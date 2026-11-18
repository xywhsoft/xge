#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#define GL_CALL WINAPI
#else
#include <dlfcn.h>
#define GL_CALL
#endif

static xge3d_renderer *renderer;
static xge3d_target *destination;

static void *load_gl(const char *name)
{
#ifdef _WIN32
    HMODULE module=GetModuleHandleW(L"opengl32.dll");
    FARPROC proc=GetProcAddress(module,name);
    if(!proc){
        typedef PROC (WINAPI *lookup)(LPCSTR);
        FARPROC raw=GetProcAddress(module,"wglGetProcAddress");
        lookup get;
        memcpy(&get,&raw,sizeof(get));
        proc=get(name);
    }
    void *address;
    memcpy(&address,&proc,sizeof(address));
    return address;
#else
    static void *module;
    if(!module)module=dlopen("libGL.so.1",RTLD_NOW|RTLD_LOCAL);
    return module?dlsym(module,name):NULL;
#endif
}

static void check_readback(const unsigned char *pixels)
{
    typedef void (GL_CALL *buffers)(int,unsigned int*);
    typedef void (GL_CALL *bind_buffer)(unsigned int,unsigned int);
    typedef void (GL_CALL *buffer_data)(unsigned int,ptrdiff_t,const void*,unsigned int);
    typedef void (GL_CALL *set_store)(unsigned int,int);
    typedef void (GL_CALL *get_integer)(unsigned int,int*);
    buffers generate=(buffers)load_gl("glGenBuffers");
    buffers remove=(buffers)load_gl("glDeleteBuffers");
    bind_buffer bind=(bind_buffer)load_gl("glBindBuffer");
    buffer_data allocate=(buffer_data)load_gl("glBufferData");
    set_store store=(set_store)load_gl("glPixelStorei");
    get_integer get=(get_integer)load_gl("glGetIntegerv");
    assert(generate&&remove&&bind&&allocate&&store&&get);

    enum { WIDTH=64, HEIGHT=32, ROW=WIDTH*4, STRIDE=ROW+4 };
    unsigned char padded[STRIDE*HEIGHT];
    memset(padded,0xa5,sizeof(padded));
    unsigned int pbo=0;
    generate(1,&pbo);
    bind(0x88EB,pbo);
    allocate(0x88EB,4096,NULL,0x88E4);
    store(0x0D05,8);
    store(0x0D02,5);
    store(0x0D03,2);
    store(0x0D04,1);
    assert(xge3dTargetReadPixels(destination,padded,sizeof(padded)-1,STRIDE)==XGE_ERROR_INVALID_ARGUMENT);
    for(size_t i=0;i<sizeof(padded);++i)assert(padded[i]==0xa5);
    assert(xge3dTargetReadPixels(destination,padded,sizeof(padded),STRIDE)==XGE_OK);
    for(size_t y=0;y<HEIGHT;++y){
        assert(!memcmp(padded+y*STRIDE,pixels+y*ROW,ROW));
        for(size_t i=ROW;i<STRIDE;++i)assert(padded[y*STRIDE+i]==0xa5);
    }
    int value=0;
    get(0x88ED,&value);assert(value==(int)pbo);
    get(0x0D05,&value);assert(value==8);
    get(0x0D02,&value);assert(value==5);
    get(0x0D03,&value);assert(value==2);
    get(0x0D04,&value);assert(value==1);
    bind(0x88EB,0);
    remove(1,&pbo);
    store(0x0D05,4);
    store(0x0D02,0);
    store(0x0D03,0);
    store(0x0D04,0);
}

static int frame(void *user)
{
    (void)user;
    xge3d_scene *scene=NULL;
    xge3d_target *source=NULL;
    xge3d_texture *red=NULL,*green=NULL;
    assert(xge3dRendererCreateOwned(&renderer)==XGE_OK);
    assert(xge3dSceneCreate(&scene)==XGE_OK);
    assert(xge3dTargetCreateOwned(64,32,&destination)==XGE_OK);
    assert(xge3dTargetCreateOwned(8,8,&source)==XGE_OK);
    assert(xge3dTextureFromTargetOwned(source,&green)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==3);

    int width,height;
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dTargetResize(source,16,8)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(xge3dTargetSize(source,&width,&height)==XGE_OK && width==8 && height==8);
    assert(xge3dTargetResize(source,16,8)==XGE_OK);
    assert(xge3dTargetSize(source,&width,&height)==XGE_OK && width==16 && height==8);
    assert(xgeGraphicsOwnedResourceCount()==3);

    xge3d_camera_t camera;
    const float green_clear[4]={0,1,0,1},black[4]={0,0,0,1};
    assert(xge3dCameraOrthographic(&camera,4,2,.1f,10)==XGE_OK);
    assert(xge3dRenderFrameTo(renderer,scene,&camera,source,green_clear,3,1,NULL)==XGE_OK);
    const unsigned char red_pixel[4]={255,0,0,255};
    assert(xge3dTextureCreateRGBAOwned(1,1,red_pixel,4,1,NULL,&red)==XGE_OK);
    xge3d_texture *textures[3]={red,green,NULL};
    xge3d_billboard_geometry_t geometry[3];
    xge3d_billboard_t legacy[3];
    for(int i=0;i<3;++i){
        geometry[i]=xge3dBillboardGeometryDefault();
        legacy[i]=xge3dBillboardDefault();
        geometry[i].position=legacy[i].position=(xge3d_vec3_t){i-1,0,-2};
        geometry[i].width=geometry[i].height=legacy[i].width=legacy[i].height=.8f;
        legacy[i].texture=textures[i];
    }

    unsigned char reference[64*32*4],pixels[sizeof(reference)];
    assert(xge3dBillboardsSet(renderer,legacy,3)==XGE_OK);
    xge3d_render_stats_t stats;
    assert(xge3dRenderFrameTo(renderer,scene,&camera,destination,black,3,1,&stats)==XGE_OK && stats.triangles==6);
    assert(xge3dTargetReadPixels(destination,reference,sizeof(reference),0)==XGE_OK);
    assert(reference[(16*64+16)*4]==255 && reference[(16*64+32)*4+1]==255);
    assert(reference[(16*64+48)*4]==255 && reference[(16*64+48)*4+1]==255);
    assert(xge3dBillboardsSetTexturedGeometry(renderer,geometry,textures,3)==XGE_OK);
    assert(xge3dRenderFrameTo(renderer,scene,&camera,source,black,3,1,NULL)==XGE_ERROR_INVALID_STATE);

    xge3d_billboard_batch *batch=NULL;
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dBillboardBatchCreate(geometry,3,&batch)==XGE_ERROR_OUT_OF_MEMORY && !batch);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(xge3dBillboardBatchCreate(geometry,65537,&batch)==XGE_ERROR_INVALID_ARGUMENT && !batch);
    assert(xge3dBillboardBatchCreate(geometry,3,&batch)==XGE_OK);
    assert(xge3dBillboardsSetTexturedBatch(renderer,batch,textures,3)==XGE_OK);
    assert(xge3dBillboardsSetTexturedBatch(renderer,batch,textures,2)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dBillboardsSetTexturedBatch(renderer,batch,NULL,3)==XGE_ERROR_INVALID_ARGUMENT);

    xge3d_billboard_geometry_t many[64];
    xge3d_texture *untextured[64]={0};
    for(size_t i=0;i<64;++i)many[i]=xge3dBillboardGeometryDefault();
    xge3d_billboard_batch *large=NULL;
    assert(xge3dBillboardBatchCreate(many,64,&large)==XGE_OK);
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dBillboardsSetTexturedBatch(renderer,large,untextured,64)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dBillboardsSetTexturedGeometry(renderer,many,untextured,64)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    xge3dBillboardBatchFree(large);
    geometry[2].width=-1;
    assert(xge3dBillboardBatchCreate(geometry,3,&large)==XGE_ERROR_INVALID_ARGUMENT && !large);
    assert(xge3dBillboardsSetTexturedGeometry(renderer,geometry,textures,3)==XGE_ERROR_INVALID_ARGUMENT);
    const xge3d_billboard_geometry_t *stored=NULL;
    size_t count=0;
    assert(xge3dBillboardBatchGetGeometry(batch,&stored,&count)==XGE_OK && count==3 && stored[2].width==.8f);
    xge3dBillboardBatchFree(batch);

    assert(xge3dTextureReleaseChecked(red)==XGE_OK);
    assert(xge3dTextureReleaseChecked(green)==XGE_OK);
    assert(xge3dTargetReleaseChecked(source)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==4);
    assert(xge3dRenderFrameTo(renderer,scene,&camera,destination,black,3,1,NULL)==XGE_OK);
    assert(xge3dTargetReadPixels(destination,pixels,sizeof(pixels),0)==XGE_OK);
    assert(!memcmp(reference,pixels,sizeof(pixels)));
    check_readback(pixels);
    xge3d_billboard_batch *empty=NULL;
    assert(xge3dBillboardBatchCreate(NULL,0,&empty)==XGE_OK);
    assert(xge3dBillboardBatchGetGeometry(empty,&stored,&count)==XGE_OK && !stored && !count);
    assert(xge3dBillboardsSetTexturedBatch(renderer,empty,NULL,0)==XGE_OK);
    xge3dBillboardBatchFree(empty);
    assert(xgeGraphicsOwnedResourceCount()==2);
    assert(xge3dRenderFrameTo(renderer,scene,&camera,destination,black,3,1,&stats)==XGE_OK && !stats.triangles);
    xge3dSceneFree(scene);
    return 1;
}

int main(void)
{
    xge_desc_t desc={.iWidth=128,.iHeight=64,.sTitle="XGE managed target verification"};
    assert(xgeInit(&desc)==XGE_OK);
    assert(xgeRun(frame,NULL)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==2 && xgeUnitChecked()==XGE_ERROR_BUSY);
    assert(xge3dRendererReleaseChecked(renderer)==XGE_OK && xgeGraphicsOwnedResourceCount()==1);
    assert(xge3dTargetReleaseChecked(destination)==XGE_OK && xgeGraphicsOwnedResourceCount()==0);
    assert(xgeUnitChecked()==XGE_OK);
    puts("TARGET3D_NATIVE_GPU_OK");
    return 0;
}
