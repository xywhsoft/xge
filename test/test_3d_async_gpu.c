#include "../xge.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
#define CHECK(x) do {if (!(x)) {fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);goto done;}} while(0)
static int failed=1;
static unsigned char pixels[2][128*128*4];
static int wait_cpu(xge3d_loader *l,xge3d_request_t h,xge3d_request_info_t *info)
{
    uint64_t end=xrtClock()+5000000;
    do {
        if (xge3dLoaderStatus(l,h,info)!=XGE_OK) return 0;
        if (info->state==XGE3D_REQUEST_CPU_READY) return 1;
        if (info->state==XGE3D_REQUEST_FAILED) return 0;
        xrtSleep(1);
    } while (xrtClock()<end);
    return 0;
}
/* Exercise app-owned GL interoperability without reaching into the engine. */
typedef void (APIENTRY *get_int_proc)(unsigned,int*);
typedef void (APIENTRY *pixel_store_proc)(unsigned,int);
typedef void (APIENTRY *gen_proc)(int,unsigned*);
typedef void (APIENTRY *bind_proc)(unsigned,unsigned);
typedef void (APIENTRY *buffer_proc)(unsigned,ptrdiff_t,const void*,unsigned);
typedef unsigned (APIENTRY *error_proc)(void);
static get_int_proc get_int;static pixel_store_proc pixel_store;static gen_proc gen_buffers,delete_buffers;
static bind_proc bind_buffer;static buffer_proc buffer_data;static error_proc get_error;
static void *gl_proc(const char *name)
{
    HMODULE library=GetModuleHandleA("opengl32.dll");if (!library) return NULL;
    FARPROC result=GetProcAddress(library,name);
    if (!result) {
        typedef PROC (WINAPI *wgl_proc)(LPCSTR);
        wgl_proc get=(wgl_proc)(void*)GetProcAddress(library,"wglGetProcAddress");if (get) result=get(name);
    }
    return (void*)result;
}
static int frame(void *user)
{
    (void)user;xge3d_loader *loader=NULL;xge3d_model *model=NULL,*sync=NULL;
    xge3d_scene *scene=NULL;xge3d_renderer *renderer=NULL;xge3d_target *target=NULL;
    unsigned pbo=0;int pumps=0;uint64_t uploaded=0;
    CHECK(xge3dLoaderCreate(NULL,&loader)==XGE_OK);
    xge3d_request_t h,old;xge3d_request_info_t info;xge3d_upload_stats_t stats;
    CHECK(xge3dLoaderRequest(loader,"artifacts/xge-3d/fixtures/async.gltf",&h)==XGE_OK && wait_cpu(loader,h,&info));
    uint64_t expected=64*33*4+6*sizeof(xge3d_vertex_t)+12;
    CHECK(info.total_bytes==expected && !info.uploaded_bytes);
    xge3d_upload_budget_t budget={64,0,0};
    CHECK(xge3dLoaderPump(loader,&budget,&stats)==XGE_OK && !stats.operations && !stats.upload_bytes);
    budget=(xge3d_upload_budget_t){3,0,10};
    CHECK(xge3dLoaderPump(loader,&budget,&stats)==XGE_OK && !stats.operations && stats.min_next_bytes==4);
    CHECK(xge3dLoaderTake(loader,h,&model)==XGE_ERROR_INVALID_STATE && !model);
    budget=(xge3d_upload_budget_t){64,0,2};
    CHECK(xge3dLoaderPump(loader,&budget,&stats)==XGE_OK && stats.operations==2 && stats.upload_bytes==64 && !stats.completed);
    CHECK(xge3dLoaderStatus(loader,h,&info)==XGE_OK && info.state==XGE3D_REQUEST_UPLOADING && info.uploaded_bytes==64);
    old=h;CHECK(xge3dLoaderRelease(loader,h)==XGE_OK);
    CHECK(xge3dLoaderRequest(loader,"artifacts/xge-3d/fixtures/async.gltf",&h)==XGE_OK && h.slot!=old.slot && wait_cpu(loader,h,&info));
    CHECK(xge3dLoaderStatus(loader,old,&info)==XGE_ERROR_INVALID_ARGUMENT);
    /* Finish the texture, begin VBO work, then cancel another private upload. */
    budget=(xge3d_upload_budget_t){17,0,2};
    for (int i=0;i<1500;++i) {
        CHECK(xge3dLoaderPump(loader,&budget,&stats)==XGE_OK && stats.upload_bytes<=17 && stats.operations<=2);
        CHECK(xge3dLoaderStatus(loader,h,&info)==XGE_OK);
        if (info.uploaded_bytes>64*33*4) break;
    }
    CHECK(info.uploaded_bytes>64*33*4 && info.uploaded_bytes<expected && info.state==XGE3D_REQUEST_UPLOADING);
    CHECK(xge3dLoaderCancel(loader,h)==XGE_OK && xge3dLoaderTake(loader,h,&model)==XGE_ERROR_INVALID_STATE && !model);
    CHECK(xge3dLoaderRelease(loader,h)==XGE_OK);
    get_int=(get_int_proc)gl_proc("glGetIntegerv");CHECK(get_int);
    int maximum;get_int(0x0D33,&maximum);
    if (maximum<65537) {
        CHECK(xge3dLoaderRequest(loader,"artifacts/xge-3d/fixtures/async-oversized.gltf",&h)==XGE_OK && wait_cpu(loader,h,&info));
        budget=(xge3d_upload_budget_t){1024*1024,0,100};
        CHECK(xge3dLoaderPump(loader,&budget,&stats)==XGE_OK && stats.failed==1 && !stats.completed && stats.upload_bytes==64*33*4);
        CHECK(xge3dLoaderStatus(loader,h,&info)==XGE_OK && info.state==XGE3D_REQUEST_FAILED && info.result==XGE_ERROR_UNSUPPORTED);
        CHECK(xge3dLoaderTake(loader,h,&model)==XGE_ERROR_INVALID_STATE && !model && xge3dLoaderRelease(loader,h)==XGE_OK);
    }
    /* A model loaded synchronously is the pixel reference. */
    CHECK(xge3dModelLoad("artifacts/xge-3d/fixtures/async.gltf",&sync)==XGE_OK);
    CHECK(xge3dSceneCreate(&scene)==XGE_OK && xge3dRendererCreate(&renderer)==XGE_OK && xge3dTargetCreate(128,128,&target)==XGE_OK);
    xge3d_node_t root;CHECK(xge3dModelInstantiate(scene,sync,(xge3d_node_t){0},&root)==XGE_OK);
    xge3d_transform_t transform=XGE3D_TRANSFORM_IDENTITY;transform.position=(xge3d_vec3_t){-2,-1,0};
    CHECK(xge3dNodeSetTransform(scene,root,&transform)==XGE_OK);
    xge3d_camera_t camera;CHECK(xge3dCameraPerspective(&camera,1,1,.1f,10)==XGE_OK);
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,4},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    xge3d_render_desc_t draw={&camera,target,{0,0,0,0},3};xge3d_render_stats_t render;
    CHECK(xge3dRender(renderer,scene,&draw,&render)==XGE_OK && render.draw_calls==2 && render.triangles==2);
    CHECK(xge3dTargetReadPixels(target,pixels[0],sizeof(pixels[0]),512)==XGE_OK);
    CHECK(xge3dNodeDestroy(scene,root)==XGE_OK);xge3dModelFree(sync);sync=NULL;
    get_int=(get_int_proc)gl_proc("glGetIntegerv");pixel_store=(pixel_store_proc)gl_proc("glPixelStorei");
    gen_buffers=(gen_proc)gl_proc("glGenBuffers");delete_buffers=(gen_proc)gl_proc("glDeleteBuffers");
    bind_buffer=(bind_proc)gl_proc("glBindBuffer");buffer_data=(buffer_proc)gl_proc("glBufferData");get_error=(error_proc)gl_proc("glGetError");
    CHECK(get_int && pixel_store && gen_buffers && delete_buffers && bind_buffer && buffer_data && get_error);
    gen_buffers(1,&pbo);bind_buffer(0x88EC,pbo);buffer_data(0x88EC,256,NULL,0x88E4);
    pixel_store(0x0CF5,8);pixel_store(0x0CF2,8);pixel_store(0x0CF3,3);pixel_store(0x0CF4,2);
    CHECK(get_error()==0);
    CHECK(xge3dLoaderRequest(loader,"artifacts/xge-3d/fixtures/async.gltf",&h)==XGE_OK && wait_cpu(loader,h,&info));
    /* Alternate narrow pixels, full rows and sub-byte vertex/index transfers.
     * Every pump must preserve the application's unpack/PBO state. */
    for (int i=0;i<2000;++i) {
        budget=(xge3d_upload_budget_t){i%3==0 ? 4 : i%3==1 ? 1024 : 17,0,(uint32_t)(i%3+1)};
        CHECK(xge3dLoaderPump(loader,&budget,&stats)==XGE_OK && stats.upload_bytes<=budget.bytes && stats.operations<=budget.operations && !stats.failed);
        uploaded+=stats.upload_bytes;++pumps;
        const unsigned keys[]={0x0CF5,0x0CF2,0x0CF3,0x0CF4,0x88EF};const int values[]={8,8,3,2,(int)pbo};
        for (int j=0;j<5;++j) {int value;get_int(keys[j],&value);CHECK(value==values[j]);}
        CHECK(get_error()==0 && xge3dLoaderStatus(loader,h,&info)==XGE_OK);
        if (info.state==XGE3D_REQUEST_READY) {CHECK(stats.completed==1);break;}
        CHECK(!stats.completed && xge3dLoaderTake(loader,h,&model)==XGE_ERROR_INVALID_STATE && !model);
    }
    CHECK(info.state==XGE3D_REQUEST_READY && info.uploaded_bytes==expected && uploaded==expected && pumps>20);
    bind_buffer(0x88EC,0);delete_buffers(1,&pbo);pbo=0;
    pixel_store(0x0CF5,4);pixel_store(0x0CF2,0);pixel_store(0x0CF3,0);pixel_store(0x0CF4,0);
    CHECK(xge3dLoaderTake(loader,h,&model)==XGE_OK && xge3dLoaderStatus(loader,h,&info)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dModelInstantiate(scene,model,(xge3d_node_t){0},&root)==XGE_OK && xge3dNodeSetTransform(scene,root,&transform)==XGE_OK);
    xge3dModelFree(model);model=NULL;CHECK(xge3dLoaderFree(loader)==XGE_OK);loader=NULL;
    CHECK(xge3dRender(renderer,scene,&draw,&render)==XGE_OK && render.draw_calls==2 && render.triangles==2 && !render.upload_bytes);
    CHECK(xge3dTargetReadPixels(target,pixels[1],sizeof(pixels[1]),512)==XGE_OK && !memcmp(pixels[0],pixels[1],sizeof(pixels[0])));
    CHECK(xgeImageSavePNG("artifacts/xge-3d/p5-async-upload.png",128,128,pixels[1],512)==XGE_OK);
    /* The budget is shared by all requests, rather than renewed per model. */
    CHECK(xge3dLoaderCreate(NULL,&loader)==XGE_OK);
    xge3d_request_t second;
    CHECK(xge3dLoaderRequest(loader,"artifacts/xge-3d/fixtures/async.gltf",&h)==XGE_OK);
    CHECK(xge3dLoaderRequest(loader,"artifacts/xge-3d/fixtures/async.gltf",&second)==XGE_OK && wait_cpu(loader,h,&info) && wait_cpu(loader,second,&info));
    budget=(xge3d_upload_budget_t){64,0,2};
    CHECK(xge3dLoaderPump(loader,&budget,&stats)==XGE_OK && stats.upload_bytes==64 && stats.operations==2);
    CHECK(xge3dLoaderStatus(loader,second,&info)==XGE_OK && !info.uploaded_bytes);
    budget=(xge3d_upload_budget_t){1024*1024,1,100};
    CHECK(xge3dLoaderPump(loader,&budget,&stats)==XGE_OK && stats.upload_bytes<=budget.bytes && stats.operations<=100);
    budget.microseconds=0;
    CHECK(xge3dLoaderPump(loader,&budget,&stats)==XGE_OK && stats.completed==2 && !stats.failed);
    CHECK(xge3dLoaderCancel(loader,h)==XGE_OK && xge3dLoaderRelease(loader,h)==XGE_OK);
    CHECK(xge3dLoaderTake(loader,second,&model)==XGE_OK);xge3dModelFree(model);model=NULL;
    CHECK(xge3dLoaderFree(loader)==XGE_OK);loader=NULL;
    int covered=0;for (int i=0;i<128*128;++i) if (pixels[1][i*4+3]>250) ++covered;
    CHECK(covered>500);
    printf("3D async GPU: %llu bytes in %d budgeted pumps, mip/row/pixel/VBO/IBO continuation, partial cancellations/failures, shared limits, GL unpack/PBO restored and %d exact reference pixels passed (max texture %d)\n",(unsigned long long)uploaded,pumps,covered,maximum);
    failed=0;
done:
    if (pbo) {bind_buffer(0x88EC,0);delete_buffers(1,&pbo);pixel_store(0x0CF5,4);pixel_store(0x0CF2,0);pixel_store(0x0CF3,0);pixel_store(0x0CF4,0);}
    xge3dLoaderFree(loader);xge3dModelFree(model);xge3dModelFree(sync);xge3dSceneFree(scene);xge3dTargetFree(target);xge3dRendererFree(renderer);return 1;
}
int main(void)
{
    xge_desc_t desc={0};desc.iWidth=200;desc.iHeight=200;desc.sTitle="XGE async GPU verification";
    if (xgeInit(&desc)!=XGE_OK) return 1;
    int result=xgeRun(frame,NULL);xgeUnit();return result==XGE_OK ? failed : 1;
}
