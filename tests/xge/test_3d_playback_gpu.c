#include "xge.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);goto done;}}while(0)
static int failed=1;static unsigned char actual[128*128*4],expected[sizeof(actual)];
static int compare(xge3d_animator *a,xge3d_renderer *r,xge3d_scene *s,xge3d_render_desc_t *draw,float x)
{
    xge3d_render_stats_t stats;int result=xge3dRender(r,s,draw,&stats);if(result)return 0;
    if(xge3dTargetReadPixels(draw->target,actual,sizeof(actual),512))return 0;
    xge3d_transform_t p=XGE3D_TRANSFORM_IDENTITY;p.position.x=x;
    if(xge3dAnimatorSetPose(a,3,&p)||xge3dRender(r,s,draw,&stats)||xge3dTargetReadPixels(draw->target,expected,sizeof(expected),512))return 0;
    int covered=0;for(size_t i=0;i<sizeof(actual);i+=4)if(actual[i]>200)++covered;
    return covered>100&&!memcmp(actual,expected,sizeof(actual));
}
static int frame(void *user)
{
    (void)user;xge3d_scene *s=NULL;xge3d_model *m=NULL;xge3d_renderer *r=NULL;xge3d_target *target=NULL;
    xge3d_animator *a=NULL;xge3d_clip *linear=NULL,*step=NULL,*cubic=NULL;xge3d_node_t root;
    CHECK(!xge3dSceneCreate(&s)&&!xge3dRendererCreate(&r)&&!xge3dTargetCreate(128,128,&target));
    CHECK(!xge3dModelLoad("artifacts/xge-3d/fixtures/skin.gltf",&m));
    CHECK(!xge3dModelInstantiate(s,m,(xge3d_node_t){0},&root)&&!xge3dAnimatorCreate(s,root,&a));
    CHECK(!xge3dClipLoad("artifacts/xge-3d/fixtures/motion-linear.gltf",0,&linear));
    CHECK(!xge3dClipLoad("artifacts/xge-3d/fixtures/motion-step.gltf",0,&step));
    CHECK(!xge3dClipLoad("artifacts/xge-3d/fixtures/motion-cubic.gltf",0,&cubic));
    xge3dModelFree(m);m=NULL;
    xge3d_camera_t camera;CHECK(!xge3dCameraOrthographic(&camera,6,4,.1f,10));
    CHECK(!xge3dCameraLookAt(&camera,(xge3d_vec3_t){1,0,3},(xge3d_vec3_t){1,0,0},(xge3d_vec3_t){0,1,0}));
    xge3d_render_desc_t draw={&camera,target,{0,0,0,1},3};
    xge3d_animation_layer_t d=xge3dAnimationLayerDefault();d.loop=0;d.time=.5f;d.speed=0;
    CHECK(!xge3dAnimatorPlay(a,0,linear,&d,1)&&!xge3dAnimatorUpdate(a,.5f)&&compare(a,r,s,&draw,1.5f));
    CHECK(!xge3dAnimatorUpdate(a,.5f)&&compare(a,r,s,&draw,2));
    d.time=.25f;d.speed=1;CHECK(!xge3dAnimatorSetLayer(a,0,linear,&d));d.time=0;
    CHECK(!xge3dAnimatorPlay(a,0,step,&d,1)&&!xge3dAnimatorUpdate(a,.25f)&&compare(a,r,s,&draw,1.75f));
    d.time=.5f;d.speed=0;CHECK(!xge3dAnimatorPlay(a,0,cubic,&d,.5f));
    CHECK(!xge3dAnimatorUpdate(a,0)&&compare(a,r,s,&draw,1.75f));
    CHECK(!xge3dAnimatorSetPaused(a,1)&&!xge3dAnimatorUpdate(a,10)&&compare(a,r,s,&draw,1.75f));
    CHECK(!xge3dAnimatorSetPaused(a,0)&&!xge3dAnimatorUpdate(a,.25f)&&compare(a,r,s,&draw,2));
    CHECK(!xge3dAnimatorSetLayerMode(a,7,XGE3D_ANIMATION_ADDITIVE));
    d.time=1;d.weight=.5f;CHECK(!xge3dAnimatorPlay(a,7,linear,&d,1));
    CHECK(!xge3dAnimatorUpdate(a,.25f)&&compare(a,r,s,&draw,2.5f));
    CHECK(!xge3dAnimatorSetLayerPaused(a,7,1)&&!xge3dAnimatorUpdate(a,.5f)&&compare(a,r,s,&draw,2.5f));
    CHECK(!xge3dAnimatorSetLayerPaused(a,7,0)&&!xge3dAnimatorPlay(a,7,NULL,NULL,.5f));
    xge3dClipFree(linear);linear=NULL;xge3dClipFree(cubic);cubic=NULL;xge3dClipFree(step);step=NULL;
    CHECK(!xge3dAnimatorUpdate(a,.5f)&&compare(a,r,s,&draw,2.25f));
    CHECK(!xgeImageSavePNGEx("artifacts/xge-animation-playback/skin.png",128,128,actual,512,XGE_IMAGE_PREMULTIPLIED));
    failed=0;puts("Playback GPU: 9 pixel-identical independent pose references, moving/interrupted fades, pause/resume, additive overlay and retained clip lifetime passed");
done:
    xge3dAnimatorFree(a);xge3dClipFree(linear);xge3dClipFree(step);xge3dClipFree(cubic);
    xge3dModelFree(m);xge3dSceneFree(s);xge3dTargetFree(target);xge3dRendererFree(r);return 1;
}
int main(void)
{xge_desc_t d={.iWidth=180,.iHeight=180,.sTitle="XGE playback GPU verification"};if(xgeInit(&d))return 1;
 int result=xgeRun(frame,NULL);xgeUnit();return result?1:failed;}
