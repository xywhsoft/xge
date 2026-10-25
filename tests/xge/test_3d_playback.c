#include "xge.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
#define CLOSE(a,b) (fabsf((a)-(b))<.0002f)
static xge3d_playback_info_t info(xge3d_animator *a,int layer)
{xge3d_playback_info_t i={.size=sizeof(i)};if(xge3dAnimatorGetPlayback(a,layer,&i))i.size=0;return i;}
static xge3d_transform_t pose(xge3d_animator *a)
{xge3d_transform_t p=XGE3D_TRANSFORM_IDENTITY;if(xge3dAnimatorGetPose(a,3,&p))p.position.x=NAN;return p;}
int main(void)
{
    xge_desc_t init={.iRunMode=XGE_RUN_MANUAL};CHECK(!xgeInit(&init));
    xallocationbudget *lifetime=NULL;xallocationscope lifetime_scope={0};
    CHECK(xrtAllocationBudgetCreate(SIZE_MAX,&lifetime)&&xrtAllocationBudgetEnter(lifetime,&lifetime_scope));
    xge3d_scene *s=NULL;xge3d_model *m=NULL;xge3d_animator *a=NULL;xge3d_node_t root;
    CHECK(!xge3dSceneCreate(&s)&&!xge3dModelLoad("artifacts/xge-3d/fixtures/skin.gltf",&m));
    CHECK(!xge3dModelInstantiate(s,m,(xge3d_node_t){0},&root)&&!xge3dAnimatorCreate(s,root,&a));
    xge3d_clip *linear=NULL,*step=NULL,*cubic=NULL,*rotation=NULL,*scale=NULL,*instant=NULL;
    CHECK(!xge3dClipLoad("artifacts/xge-3d/fixtures/motion-linear.gltf",0,&linear));
    CHECK(!xge3dClipLoad("artifacts/xge-3d/fixtures/motion-step.gltf",0,&step));
    CHECK(!xge3dClipLoad("artifacts/xge-3d/fixtures/motion-cubic.gltf",0,&cubic));
    CHECK(!xge3dClipLoad("artifacts/xge-3d/fixtures/motion-rotation.gltf",0,&rotation));
    CHECK(!xge3dClipLoad("artifacts/xge-3d/fixtures/motion-scale.gltf",0,&scale));
    CHECK(!xge3dClipLoad("artifacts/xge-3d/fixtures/motion-instant.gltf",0,&instant));
    xge3d_animation_layer_t d=xge3dAnimationLayerDefault();d.loop=0;d.time=.5f;d.speed=0;
    CHECK(!info(a,0).active&&!info(a,0).finished);
    /* Fade clocks use real time even when clip speed is zero. */
    CHECK(!xge3dAnimatorPlay(a,0,linear,&d,1)&&!xge3dAnimatorUpdate(a,0));
    CHECK(CLOSE(pose(a).position.x,1)&&CLOSE(info(a,0).weight,0));
    CHECK(!xge3dAnimatorUpdate(a,.25f)&&CLOSE(pose(a).position.x,1.25f));
    CHECK(CLOSE(info(a,0).time,.5f)&&CLOSE(info(a,0).fade_remaining,.75f));
    CHECK(!xge3dAnimatorFadeLayer(a,0,1,.5f)&&!xge3dAnimatorUpdate(a,.25f));
    CHECK(CLOSE(info(a,0).weight,.625f)&&CLOSE(pose(a).position.x,1.625f));
    CHECK(!xge3dAnimatorSetLayerWeight(a,0,1)&&!xge3dAnimatorUpdate(a,0));
    CHECK(CLOSE(info(a,0).fade_remaining,0)&&CLOSE(pose(a).position.x,2));
    /* Outgoing clips keep moving: .25->.5, incoming STEP stays at one. */
    d.time=.25f;d.speed=1;CHECK(!xge3dAnimatorSetLayer(a,0,linear,&d));d.time=0;
    CHECK(!xge3dAnimatorPlay(a,0,step,&d,1)&&!xge3dAnimatorUpdate(a,.25f));
    CHECK(CLOSE(pose(a).position.x,1.75f)&&info(a,0).transitioning);
    d.time=.5f;d.speed=0;
    CHECK(!xge3dAnimatorPlay(a,0,cubic,&d,.5f)&&!xge3dAnimatorUpdate(a,0));
    CHECK(CLOSE(pose(a).position.x,1.75f));
    CHECK(!xge3dAnimatorUpdate(a,.25f)&&CLOSE(pose(a).position.x,2));
    for(int i=0;i<30;++i){float before=pose(a).position.x;
        CHECK(!xge3dAnimatorPlay(a,0,i%2?linear:step,&d,.5f)&&!xge3dAnimatorUpdate(a,0));
        CHECK(CLOSE(pose(a).position.x,before));CHECK(!xge3dAnimatorUpdate(a,.01f));}
    CHECK(!xge3dAnimatorUpdate(a,1)&&!info(a,0).transitioning);
    /* Missing channels and differing masks fade into the underlying pose. */
    CHECK(!xge3dAnimatorSetLayer(a,0,linear,&d));
    CHECK(!xge3dAnimatorPlay(a,0,rotation,&d,1)&&!xge3dAnimatorUpdate(a,.5f));
    CHECK(CLOSE(pose(a).position.x,1.5f)&&CLOSE(pose(a).rotation.z,sinf(.39269908f)));
    float mask[]={1,1,1,0};CHECK(!xge3dAnimatorSetLayer(a,0,linear,&d));d.mask=mask;d.mask_count=4;
    CHECK(!xge3dAnimatorPlay(a,0,step,&d,1)&&!xge3dAnimatorUpdate(a,.5f)&&CLOSE(pose(a).position.x,1.5f));
    d.mask=NULL;d.mask_count=0;
    /* Existing additive/masked layering works through automatic fades. */
    CHECK(!xge3dAnimatorSetLayer(a,0,linear,&d)&&!xge3dAnimatorSetLayerMode(a,7,XGE3D_ANIMATION_ADDITIVE));
    d.time=1;d.weight=.5f;CHECK(!xge3dAnimatorPlay(a,7,linear,&d,1)&&!xge3dAnimatorUpdate(a,.5f));
    CHECK(CLOSE(pose(a).position.x,2.5f)&&info(a,7).mode==XGE3D_ANIMATION_ADDITIVE);
    CHECK(!xge3dAnimatorPlay(a,7,NULL,NULL,.5f)&&info(a,7).stopping);
    CHECK(!xge3dAnimatorUpdate(a,.5f)&&!info(a,7).active&&CLOSE(pose(a).position.x,2));
    d.time=.5f;d.weight=1;CHECK(!xge3dAnimatorPlay(a,7,scale,&d,1)&&!xge3dAnimatorUpdate(a,.5f));
    CHECK(CLOSE(pose(a).scale.x,1.5f)&&CLOSE(pose(a).scale.y,1.5f));
    CHECK(!xge3dAnimatorSetLayerPaused(a,7,1)&&!xge3dAnimatorUpdate(a,10)&&CLOSE(info(a,7).fade_remaining,.5f));
    CHECK(!xge3dAnimatorSetLayer(a,7,rotation,&d)&&!xge3dAnimatorUpdate(a,0));
    CHECK(CLOSE(pose(a).rotation.z,sqrtf(.5f))&&CLOSE(pose(a).scale.x,1));
    CHECK(!xge3dAnimatorSetLayer(a,7,NULL,NULL));
    CHECK(!xge3dAnimatorPlay(a,0,instant,&d,.5f)&&info(a,0).finished);
    CHECK(!xge3dAnimatorUpdate(a,.5f)&&CLOSE(pose(a).position.x,2)&&!info(a,0).transitioning);
    /* Global pause preserves a procedural published pose; per-layer pause still
       allows other layers to run and does not consume a first-frame event. */
    d=xge3dAnimationLayerDefault();d.loop=0;
    CHECK(!xge3dAnimatorPlay(a,0,linear,&d,1)&&!xge3dAnimatorUpdate(a,.25f));
    xge3d_transform_t custom=pose(a);custom.position.y=5;CHECK(!xge3dAnimatorSetPose(a,3,&custom));
    xge3d_playback_info_t before=info(a,0);CHECK(!xge3dAnimatorSetPaused(a,1)&&!xge3dAnimatorUpdate(a,100));
    CHECK(CLOSE(pose(a).position.y,5)&&info(a,0).paused&&CLOSE(info(a,0).time,before.time));
    CHECK(CLOSE(info(a,0).fade_remaining,before.fade_remaining));
    CHECK(!xge3dAnimatorSetPaused(a,0)&&!xge3dAnimatorUpdate(a,.25f)&&CLOSE(info(a,0).time,.5f));
    CHECK(!xge3dAnimatorSetLayer(a,1,rotation,&d)&&!xge3dAnimatorSetLayerPaused(a,1,1));
    xge3d_animation_event_t mark={0,41,0},event;CHECK(!xge3dAnimatorSetEvents(a,1,&mark,1));
    CHECK(!xge3dAnimatorUpdate(a,.25f)&&CLOSE(info(a,0).time,.75f)&&CLOSE(info(a,1).time,0));
    CHECK(xge3dAnimatorNextEvent(a,&event)==XGE_ERROR_NOT_FOUND);
    CHECK(!xge3dAnimatorSetLayerPaused(a,1,0)&&!xge3dAnimatorUpdate(a,0));
    CHECK(!xge3dAnimatorNextEvent(a,&event)&&event.id==41);
    CHECK(!xge3dAnimatorSetLayer(a,1,NULL,NULL));
    CHECK(!xge3dAnimatorUpdate(a,.25f)&&info(a,0).finished&&CLOSE(pose(a).position.x,3));
    CHECK(!xge3dAnimatorUpdate(a,10)&&info(a,0).finished&&CLOSE(pose(a).position.x,3));
    /* During replacement only incoming marks are emitted; stopping suppresses
       further marks while preserving already queued ones. */
    CHECK(!xge3dAnimatorSetLayer(a,0,linear,&d));mark.id=11;mark.time=.5f;CHECK(!xge3dAnimatorSetEvents(a,0,&mark,1));
    CHECK(!xge3dAnimatorPlay(a,0,step,&d,1));mark.id=12;mark.time=.25f;CHECK(!xge3dAnimatorSetEvents(a,0,&mark,1));
    CHECK(!xge3dAnimatorUpdate(a,.5f)&&!xge3dAnimatorPlay(a,0,NULL,NULL,1));
    CHECK(!xge3dAnimatorUpdate(a,1)&&!xge3dAnimatorNextEvent(a,&event)&&event.id==12);
    CHECK(xge3dAnimatorNextEvent(a,&event)==XGE_ERROR_NOT_FOUND&&!info(a,0).active&&!info(a,0).finished);
    /* Linear root motion uses average fade gain, with no clip-offset jump. */
    d=xge3dAnimationLayerDefault();CHECK(!xge3dAnimatorSetRootMotion(a,3,XGE3D_ROOT_X));
    CHECK(!xge3dAnimatorPlay(a,0,linear,&d,1)&&!xge3dAnimatorUpdate(a,.5f));xge3d_root_motion_t motion;
    CHECK(!xge3dAnimatorTakeRootMotion(a,&motion)&&CLOSE(motion.translation.x,.25f));
    CHECK(!xge3dAnimatorSetLayerWeight(a,0,1)&&!xge3dAnimatorPlay(a,0,step,&d,1));
    CHECK(!xge3dAnimatorUpdate(a,.25f)&&!xge3dAnimatorTakeRootMotion(a,&motion)&&CLOSE(motion.translation.x,.4375f));
    CHECK(!xge3dAnimatorSetLayerPaused(a,0,1)&&!xge3dAnimatorUpdate(a,1));
    CHECK(!xge3dAnimatorTakeRootMotion(a,&motion)&&CLOSE(motion.translation.x,0)&&CLOSE(info(a,0).time,.25f));
    CHECK(!xge3dAnimatorSetLayerPaused(a,0,0));
    /* Queue overflow must roll back fade, both clip clocks, pose and root delta. */
    CHECK(!xge3dAnimatorSetLayer(a,0,NULL,NULL)&&!xge3dAnimatorPlay(a,0,linear,&d,10));
    xge3d_animation_event_t marks[]={{0,10,0},{0,20,.25f},{0,30,1}};
    CHECK(!xge3dAnimatorSetEvents(a,0,marks,3));before=info(a,0);custom=pose(a);
    CHECK(xge3dAnimatorUpdate(a,2000)==XGE_ERROR_UNSUPPORTED);xge3d_playback_info_t after=info(a,0);
    CHECK(!memcmp(&before,&after,sizeof(before))&&CLOSE(pose(a).position.x,custom.position.x));
    CHECK(!xge3dAnimatorTakeRootMotion(a,&motion)&&CLOSE(motion.translation.x,0));
    CHECK(xge3dAnimatorNextEvent(a,&event)==XGE_ERROR_NOT_FOUND);
    CHECK(!xge3dAnimatorUpdate(a,.25f)&&!xge3dAnimatorNextEvent(a,&event)&&event.id==10);
    CHECK(!xge3dAnimatorNextEvent(a,&event)&&event.id==20);
    CHECK(!xge3dAnimatorSetEvents(a,0,NULL,0));
    CHECK(!xge3dAnimatorSetRootMotion(a,SIZE_MAX,0));
    CHECK(xge3dAnimatorPlay(a,0,linear,&d,NAN)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dAnimatorFadeLayer(a,0,2,1)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dAnimatorSetPaused(a,2)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dAnimatorSetLayerPaused(a,8,0)==XGE_ERROR_INVALID_ARGUMENT);
    d.mask=mask;d.mask_count=3;before=info(a,0);
    CHECK(xge3dAnimatorPlay(a,0,linear,&d,1)==XGE_ERROR_INVALID_ARGUMENT);after=info(a,0);CHECK(!memcmp(&before,&after,sizeof(before)));
    /* Exercise every allocation stage of interrupted Play. A failed bind or
       snapshot must leave playback untouched; successful attempts release all. */
    int rejections=0,successes=0;d=xge3dAnimationLayerDefault();
    for(size_t limit=1;limit<=1025;limit+=16){xge3d_animator *b=NULL;CHECK(!xge3dAnimatorCreate(s,root,&b));
        CHECK(!xge3dAnimatorSetLayer(b,0,linear,&d)&&!xge3dAnimatorPlay(b,0,step,&d,1)&&!xge3dAnimatorUpdate(b,.25f));
        xallocationbudget *quota=NULL;xallocationscope scope={0};CHECK(xrtAllocationBudgetCreate(limit,&quota)&&xrtAllocationBudgetEnter(quota,&scope));
        before=info(b,0);custom=pose(b);int result=xge3dAnimatorPlay(b,0,cubic,&d,1);CHECK(xrtAllocationBudgetLeave(&scope));
        if(result){++rejections;CHECK(result==XGE_ERROR_OUT_OF_MEMORY);after=info(b,0);CHECK(!memcmp(&before,&after,sizeof(before)));
            CHECK(!xge3dAnimatorUpdate(b,0)&&CLOSE(pose(b).position.x,custom.position.x));}
        else{++successes;CHECK(!xge3dAnimatorUpdate(b,0)&&CLOSE(pose(b).position.x,custom.position.x));}
        xge3dAnimatorFree(b);xallocationbudgetinfo qi={.Size=sizeof(qi)};CHECK(xrtAllocationBudgetInfo(quota,&qi)&&!qi.LiveBytes&&!qi.LiveCount);xrtAllocationBudgetFree(quota);}
    CHECK(rejections&&successes);
    /* Warmed-up updates, including transition completion, allocate nothing. */
    CHECK(!xge3dAnimatorPlay(a,0,step,&d,1));xallocationbudget *zero=NULL;xallocationscope scope={0};
    CHECK(xrtAllocationBudgetCreate(1,&zero)&&xrtAllocationBudgetEnter(zero,&scope));
    for(int i=0;i<120;++i)CHECK(!xge3dAnimatorUpdate(a,1.f/60));
    CHECK(!scope.Rejections&&xrtAllocationBudgetLeave(&scope));xrtAllocationBudgetFree(zero);
    CHECK(!xge3dAnimatorSetPaused(a,1)&&!xge3dNodeDestroy(s,root));
    CHECK(xge3dAnimatorUpdate(a,0)==XGE_ERROR_INVALID_STATE);
    xge3dAnimatorFree(a);xge3dClipFree(linear);xge3dClipFree(step);xge3dClipFree(cubic);xge3dClipFree(rotation);
    xge3dClipFree(scale);xge3dClipFree(instant);xge3dSceneFree(s);xge3dModelFree(m);xgeUnit();
    CHECK(xrtAllocationBudgetLeave(&lifetime_scope));xallocationbudgetinfo live={.Size=sizeof(live)};
    CHECK(xrtAllocationBudgetInfo(lifetime,&live)&&!live.LiveBytes&&!live.LiveCount);xrtAllocationBudgetFree(lifetime);
    puts("Playback CPU: fades, moving/outgoing and interrupted transitions, channels/masks/additive, pause/resume/finished/stop, events/root motion, rollback and allocation failures passed");return 0;
}
