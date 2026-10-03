#include "../xge.h"
#include <stdio.h>
#include <math.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x); return 1; } } while(0)
#define CLOSE(a,b) (fabsf((a)-(b))<.0001f)
int main(void)
{
    xge_desc_t desc={0};desc.iRunMode=XGE_RUN_MANUAL;CHECK(xgeInit(&desc)==XGE_OK);
    xge3d_model *model=NULL;xge3d_scene *scene=NULL;xge3d_node_t roots[2],tip[2];xge3d_animator *animators[2]={0};
    CHECK(xge3dModelLoad("artifacts/xge-3d/fixtures/skin.gltf",&model)==XGE_OK && xge3dSceneCreate(&scene)==XGE_OK);
    for (int i=0;i<2;++i) {
        CHECK(xge3dModelInstantiate(scene,model,(xge3d_node_t){0},&roots[i])==XGE_OK);
        CHECK(xge3dAnimatorCreate(scene,roots[i],&animators[i])==XGE_OK && xge3dAnimatorBoneNode(animators[i],"Tip",&tip[i])==XGE_OK);
    }
    xge3dModelFree(model);
    xge3d_clip *linear=NULL,*step=NULL,*cubic=NULL,*rotation=NULL,*cubic_rotation=NULL;
    CHECK(xge3dClipLoad("artifacts/xge-3d/fixtures/motion-linear.gltf",0,&linear)==XGE_OK);
    CHECK(xge3dClipLoad("artifacts/xge-3d/fixtures/motion-step.gltf",0,&step)==XGE_OK);
    CHECK(xge3dClipLoad("artifacts/xge-3d/fixtures/motion-cubic.gltf",0,&cubic)==XGE_OK);
    CHECK(xge3dClipLoad("artifacts/xge-3d/fixtures/motion-rotation.gltf",0,&rotation)==XGE_OK);
    CHECK(xge3dClipLoad("artifacts/xge-3d/fixtures/motion-cubic-rotation.gltf",0,&cubic_rotation)==XGE_OK);
    CHECK(CLOSE(xge3dClipInfo(linear).duration,1) && xge3dClipInfo(linear).track_count==1);
    xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.loop=0;
    CHECK(xge3dAnimatorSetLayer(animators[0],0,linear,&layer)==XGE_OK && xge3dAnimatorUpdate(animators[0],.5f)==XGE_OK);
    xge3d_mat4_t matrix;
    CHECK(xge3dNodeGetWorldMatrix(scene,tip[0],&matrix)==XGE_OK && CLOSE(matrix.m[12],2));
    CHECK(xge3dNodeGetWorldMatrix(scene,tip[1],&matrix)==XGE_OK && CLOSE(matrix.m[12],1));
    CHECK(xge3dAnimatorSetLayer(animators[0],0,step,&layer)==XGE_OK && xge3dAnimatorUpdate(animators[0],.5f)==XGE_OK);
    CHECK(xge3dNodeGetWorldMatrix(scene,tip[0],&matrix)==XGE_OK && CLOSE(matrix.m[12],1));
    CHECK(xge3dAnimatorUpdate(animators[0],.5f)==XGE_OK && xge3dNodeGetWorldMatrix(scene,tip[0],&matrix)==XGE_OK && CLOSE(matrix.m[12],3));
    CHECK(xge3dAnimatorSetLayer(animators[0],0,cubic,&layer)==XGE_OK && xge3dAnimatorUpdate(animators[0],.5f)==XGE_OK);
    CHECK(xge3dNodeGetWorldMatrix(scene,tip[0],&matrix)==XGE_OK && CLOSE(matrix.m[12],2.25f));
    CHECK(xge3dAnimatorSetLayer(animators[0],0,rotation,&layer)==XGE_OK && xge3dAnimatorUpdate(animators[0],.5f)==XGE_OK);
    CHECK(xge3dNodeGetWorldMatrix(scene,tip[0],&matrix)==XGE_OK && CLOSE(matrix.m[0],0) && CLOSE(matrix.m[1],1));
    xge3d_node_t socket;CHECK(xge3dNodeCreate(scene,tip[0],&socket)==XGE_OK);
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position.y=1;CHECK(xge3dNodeSetTransform(scene,socket,&t)==XGE_OK);
    CHECK(xge3dNodeGetWorldMatrix(scene,socket,&matrix)==XGE_OK && CLOSE(matrix.m[12],0) && CLOSE(matrix.m[13],0));
    CHECK(xge3dAnimatorSetLayer(animators[0],0,cubic_rotation,&layer)==XGE_OK && xge3dAnimatorUpdate(animators[0],.5f)==XGE_OK);
    CHECK(xge3dNodeGetWorldMatrix(scene,tip[0],&matrix)==XGE_OK && CLOSE(matrix.m[0],0) && CLOSE(matrix.m[1],1));
    layer.time=.5f;CHECK(xge3dAnimatorSetLayer(animators[0],0,linear,&layer)==XGE_OK);
    layer.weight=.25f;CHECK(xge3dAnimatorSetLayer(animators[0],1,step,&layer)==XGE_OK && xge3dAnimatorUpdate(animators[0],0)==XGE_OK);
    CHECK(xge3dNodeGetWorldMatrix(scene,tip[0],&matrix)==XGE_OK && CLOSE(matrix.m[12],1.75f));
    float mask[]={1,1,1,0};layer.mask=mask;layer.mask_count=4;
    CHECK(xge3dAnimatorSetLayer(animators[0],1,step,&layer)==XGE_OK && xge3dAnimatorUpdate(animators[0],0)==XGE_OK);
    CHECK(xge3dNodeGetWorldMatrix(scene,tip[0],&matrix)==XGE_OK && CLOSE(matrix.m[12],2));
    xge3d_bone_binding_t wrong={1,999};layer.bindings=&wrong;layer.binding_count=1;
    CHECK(xge3dAnimatorSetLayer(animators[0],1,step,&layer)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dAnimatorUpdate(animators[0],0)==XGE_OK && CLOSE(xge3dAnimatorLayerTime(animators[0],0),.5f));
    layer=xge3dAnimationLayerDefault();layer.time=.75f;
    CHECK(xge3dAnimatorSetLayer(animators[1],0,linear,&layer)==XGE_OK && xge3dAnimatorUpdate(animators[1],.5f)==XGE_OK);
    CHECK(CLOSE(xge3dAnimatorLayerTime(animators[1],0),.25f) && xge3dNodeGetWorldMatrix(scene,tip[1],&matrix)==XGE_OK && CLOSE(matrix.m[12],1.5f));
    layer=xge3dAnimationLayerDefault();
    CHECK(xge3dAnimatorSetLayer(animators[1],0,linear,&layer)==XGE_OK);
    xge3d_animation_event_t marks[]={{0,10,0},{0,20,.25f},{0,30,1}},event;
    CHECK(xge3dAnimatorSetEvents(animators[1],0,marks,3)==XGE_OK);
    marks[1].time=-1;CHECK(xge3dAnimatorSetEvents(animators[1],0,marks,3)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dAnimatorUpdate(animators[1],.25f)==XGE_OK && xge3dAnimatorUpdate(animators[1],1)==XGE_OK);
    unsigned expected[]={10,20,30,10,20};
    for (int i=0;i<5;++i) CHECK(xge3dAnimatorNextEvent(animators[1],&event)==XGE_OK && event.layer==0 && event.id==expected[i]);
    CHECK(xge3dAnimatorNextEvent(animators[1],&event)==XGE_ERROR_NOT_FOUND);
    CHECK(xge3dAnimatorUpdate(animators[1],2000)==XGE_ERROR_UNSUPPORTED && CLOSE(xge3dAnimatorLayerTime(animators[1],0),.25f));
    CHECK(xge3dAnimatorNextEvent(animators[1],&event)==XGE_ERROR_NOT_FOUND);
    CHECK(xge3dAnimatorUpdate(animators[1],2)==XGE_OK);
    unsigned wraps[]={30,10,20,30,10,20};
    for (int i=0;i<6;++i) CHECK(xge3dAnimatorNextEvent(animators[1],&event)==XGE_OK && event.id==wraps[i]);
    CHECK(xge3dAnimatorNextEvent(animators[1],&event)==XGE_ERROR_NOT_FOUND);
    CHECK(xge3dAnimatorSetRootMotion(animators[1],3,XGE3D_ROOT_X)==XGE_OK);
    CHECK(xge3dAnimatorUpdate(animators[1],.5f)==XGE_OK && xge3dAnimatorUpdate(animators[1],2)==XGE_OK);
    xge3d_root_motion_t motion;
    CHECK(xge3dAnimatorTakeRootMotion(animators[1],&motion)==XGE_OK && CLOSE(motion.translation.x,5));
    CHECK(xge3dNodeGetWorldMatrix(scene,tip[1],&matrix)==XGE_OK && CLOSE(matrix.m[12],1));
    CHECK(xge3dAnimatorTakeRootMotion(animators[1],&motion)==XGE_OK && CLOSE(motion.translation.x,0) && CLOSE(motion.rotation.w,1));
    layer.time=.75f;CHECK(xge3dAnimatorSetLayer(animators[1],0,rotation,&layer)==XGE_OK);
    CHECK(xge3dAnimatorSetRootMotion(animators[1],3,XGE3D_ROOT_ROTATION)==XGE_OK && xge3dAnimatorUpdate(animators[1],.5f)==XGE_OK);
    CHECK(xge3dAnimatorTakeRootMotion(animators[1],&motion)==XGE_OK && CLOSE(fabsf(motion.rotation.z),sqrtf(.5f)) && CLOSE(fabsf(motion.rotation.w),sqrtf(.5f)));
    CHECK(xge3dNodeGetWorldMatrix(scene,tip[1],&matrix)==XGE_OK && CLOSE(matrix.m[0],1) && CLOSE(matrix.m[1],0));
    layer=xge3dAnimationLayerDefault();layer.loop=0;
    CHECK(xge3dAnimatorSetLayer(animators[1],0,linear,&layer)==XGE_OK);
    layer.weight=.25f;CHECK(xge3dAnimatorSetLayer(animators[1],1,step,&layer)==XGE_OK);
    CHECK(xge3dAnimatorSetRootMotion(animators[1],3,XGE3D_ROOT_X)==XGE_OK && xge3dAnimatorUpdate(animators[1],.5f)==XGE_OK);
    CHECK(xge3dAnimatorTakeRootMotion(animators[1],&motion)==XGE_OK && CLOSE(motion.translation.x,.75f));
    CHECK(xge3dAnimatorSetRootMotion(animators[1],SIZE_MAX,0)==XGE_OK);
    const char *skins[]={"skin-eight","skin-primitives","skin-sparse"};
    for (int i=0;i<3;++i) {
        char path[256];snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/%s.gltf",skins[i]);xge3d_model *m=NULL;xge3d_node_t root,bone;
        CHECK(xge3dModelLoad(path,&m)==XGE_OK && xge3dModelInfo(m).truncated_weight_vertices==(size_t)(i==0 ? 6 : 0));
        CHECK(xge3dModelInstantiate(scene,m,(xge3d_node_t){0},&root)==XGE_OK && xge3dModelInstanceNode(scene,root,3,&bone)==XGE_OK);
        CHECK(xge3dSceneNodeCount(scene)>8);xge3dModelFree(m);CHECK(xge3dNodeDestroy(scene,root)==XGE_OK);
    }
    const char *invalid_skins[]={"skin-bad-joint","skin-bad-negative","skin-bad-zero","skin-large"};
    for (int i=0;i<4;++i) {
        char path[256];snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/%s.gltf",invalid_skins[i]);xge3d_model *m=(xge3d_model*)1;
        CHECK(xge3dModelLoad(path,&m)!=XGE_OK && !m);
    }
    xge3dClipFree(linear);xge3dClipFree(step);xge3dClipFree(cubic);xge3dClipFree(rotation);xge3dClipFree(cubic_rotation);
    CHECK(xge3dNodeDestroy(scene,roots[0])==XGE_OK && xge3dAnimatorUpdate(animators[0],0)==XGE_ERROR_INVALID_STATE);
    xge3dAnimatorFree(animators[0]);xge3dAnimatorFree(animators[1]);xge3dSceneFree(scene);xgeUnit();
    puts("3D animation CPU: independent motion/poses, interpolation, two layers/masks, sockets, events across loops and queue overflow rollback, root motion/in-place pose, failed bind and deleted-root recovery passed");return 0;
}
