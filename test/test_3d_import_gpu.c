#include "../xge.h"
#include <stdio.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); goto done; } } while(0)
static int failed=1;
static unsigned char pixels[128*128*4];
static int frame(void *user)
{
    (void)user;
    xge3d_model *model=NULL;
    xge3d_scene *scene=NULL;
    xge3d_renderer *renderer=NULL;
    xge3d_target *target=NULL;
    xge3d_node_t roots[2];
    CHECK(xge3dModelLoad("artifacts/xge-3d/fixtures/embedded.glb",&model)==XGE_OK);
    CHECK(xge3dSceneCreate(&scene)==XGE_OK && xge3dRendererCreate(&renderer)==XGE_OK);
    CHECK(xge3dTargetCreate(128,128,&target)==XGE_OK);
    for (int i=0;i<2;++i) {
        CHECK(xge3dModelInstantiate(scene,model,(xge3d_node_t){0},&roots[i])==XGE_OK);
        xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;
        t.position=(xge3d_vec3_t){-2+(i ? .6f : -.6f),-1,0};
        CHECK(xge3dNodeSetTransform(scene,roots[i],&t)==XGE_OK);
    }
    xge3dModelFree(model); model=NULL;
    xge3d_camera_t camera;
    CHECK(xge3dCameraPerspective(&camera,1,1,.1f,10)==XGE_OK);
    CHECK(xge3dCameraLookAt(&camera,(xge3d_vec3_t){0,0,4},(xge3d_vec3_t){0},(xge3d_vec3_t){0,1,0})==XGE_OK);
    xge3d_render_desc_t desc={&camera,target,{0,0,0,0},3};
    xge3d_render_stats_t stats;
    CHECK(xge3dRender(renderer,scene,&desc,&stats)==XGE_OK && stats.draw_calls==1 && stats.instanced_draw_calls==1 && stats.instances==2 && stats.triangles==2 && stats.upload_bytes==144);
    CHECK(xge3dRender(renderer,scene,&desc,&stats)==XGE_OK && stats.upload_bytes==0);
    CHECK(xge3dTargetReadPixels(target,pixels,sizeof(pixels),512)==XGE_OK);
    int covered=0;
    for (int i=0;i<128*128;++i) if (pixels[i*4+3]>250) ++covered;
    CHECK(covered>500 && covered<128*128/2);
    CHECK(xgeImageSavePNG("artifacts/xge-3d/p2-import-gpu.png",128,128,pixels,512)==XGE_OK);
    CHECK(xge3dNodeDestroy(scene,roots[0])==XGE_OK);
    CHECK(xge3dRender(renderer,scene,&desc,&stats)==XGE_OK && stats.draw_calls==1);
    failed=0;
    printf("3D import GPU: two shared GLB instances, %d covered pixels, caller release and independent destruction passed\n",covered);
done:
    xge3dSceneFree(scene); xge3dModelFree(model);
    xge3dTargetFree(target); xge3dRendererFree(renderer);
    return 1;
}
int main(void)
{
    xge_desc_t desc={0}; desc.iWidth=200; desc.iHeight=200; desc.sTitle="XGE model GPU verification";
    if (xgeInit(&desc)!=XGE_OK) return 1;
    int result=xgeRun(frame,NULL); xgeUnit();
    return result==XGE_OK ? failed : 1;
}
