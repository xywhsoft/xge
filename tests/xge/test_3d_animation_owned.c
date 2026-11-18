#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <assert.h>
#include <stdio.h>

typedef struct animation_pair {
    xge3d_clip *clip;
    xge3d_animator *animator;
} animation_pair;

static int32 wrong_thread(void *raw)
{
    animation_pair *pair=raw;
    bool consumed=true;
    xge3d_clip_info_t info;
    assert(xge3dClipInfoChecked(pair->clip,&info)==XGE_ERROR_INVALID_STATE);
    assert(xge3dClipReleaseOwnerChecked(pair->clip,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    assert(xge3dAnimatorUpdate(pair->animator,0)==XGE_ERROR_INVALID_STATE);
    assert(xge3dAnimatorReleaseOwnerChecked(pair->animator,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    return 0;
}

int main(void)
{
    xge3d_model *rig=NULL,*motion=NULL;
    xge3d_clip *clip=NULL,*shared=NULL;
    xge3d_scene *scene=NULL;
    xge3d_animator *animator=NULL;
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    assert(xge3dModelLoadOwned("animation-rig.gltf",NULL,&rig)==XGE_OK);
    assert(xge3dModelLoadOwned("animation-motion.gltf",NULL,&motion)==XGE_OK);
    unsigned refused=0,succeeded=0;
    for(int allowance=0;allowance<12;++allowance){
        assert(xrtMemDebugFailAfter(allowance));
        int result=xge3dClipFromModelOwned(motion,0,&clip);
        assert(xrtMemDebugFailAfter(-1));
        xrtClearError();
        if(result==XGE_OK){assert(xge3dClipReleaseChecked(clip)==XGE_OK);++succeeded;}
        else{assert(result==XGE_ERROR_OUT_OF_MEMORY && !clip);++refused;}
    }
    assert(refused>=3 && succeeded>0);
    assert(xge3dClipFromModelOwned(motion,0,&clip)==XGE_OK);
    assert(xge3dClipShareOwned(clip,&shared)==XGE_OK);
    assert(xge3dClipReleaseChecked(clip)==XGE_OK);
    assert(xge3dModelReleaseChecked(motion)==XGE_OK);
    assert(xge3dSceneCreateOwned(&scene)==XGE_OK);
    xge3d_node_t root;
    assert(xge3dModelInstantiate(scene,rig,(xge3d_node_t){0},&root)==XGE_OK);
    assert(xge3dAnimatorCreateOwned(scene,root,&animator)==XGE_OK);
    assert(xge3dModelReleaseChecked(rig)==XGE_OK);
    assert(xge3dSceneReleaseChecked(scene)==XGE_OK);

    animation_pair pair={shared,animator};
    xthread *thread=xrtThreadCreate(wrong_thread,&pair,0);
    assert(thread && xrtThreadWait(thread)==XWAIT_OK);
    xrtThreadDestroy(thread);
    xge3d_clip_info_t info={0};
    assert(xge3dClipInfoChecked(shared,&info)==XGE_OK && info.track_count==1 && info.duration==1);
    float mask[]={1,1,1,1};
    xge3d_bone_binding_t mapping={1,3};
    xge3d_animation_options_t options={0,1,1,0};
    xge3d_animation_layer_t legacy=xge3dAnimationLayerDefault();
    legacy.mask_count=99;
    assert(xge3dAnimatorSetLayer(animator,0,shared,&legacy)==XGE_OK);
    assert(xge3dAnimatorPlayArrays(animator,0,shared,&options,0,NULL,1,&mapping,1)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dAnimatorPlayArrays(animator,0,shared,&options,0,mask,4,&mapping,1)==XGE_OK);
    assert(xge3dClipReleaseChecked(shared)==XGE_OK);
    xge3d_animation_event_t event={0,71,.25f};
    assert(xge3dAnimatorSetEvents(animator,0,&event,1)==XGE_OK);
    assert(xge3dAnimatorSetRootMotion(animator,3,XGE3D_ROOT_X)==XGE_OK);
    assert(xge3dAnimatorUpdate(animator,.5f)==XGE_OK);
    assert(xge3dAnimatorNextEvent(animator,&event)==XGE_OK && event.id==71);
    assert(xge3dAnimatorNextEvent(animator,&event)==XGE_ERROR_NOT_FOUND);
    xge3d_root_motion_t delta;
    assert(xge3dAnimatorTakeRootMotion(animator,&delta)==XGE_OK && delta.translation.x==1);
    assert(xge3dAnimatorTakeRootMotion(animator,&delta)==XGE_OK && delta.translation.x==0);

    xge3d_node_t skin;
    assert(xge3dAnimatorBoneNode(animator,"SkinMesh",&skin)==XGE_OK);
    xge3d_mat4_t matrix;
    uint8_t *bytes=NULL;
    uint64_t length=0;
    assert(xge3dNodeSkinMatrixAt(scene,skin,1,&matrix)==XGE_OK);
    assert(xge3dNodeSkinPaletteBytes(scene,skin,&bytes,&length)==XGE_OK);
    assert(length==sizeof(matrix)*2 && !memcmp(bytes+sizeof(matrix),&matrix,sizeof(matrix)));
    assert(xge3dAnimatorReleaseChecked(animator)==XGE_OK);
    assert(!xrtGetError() && !xgeGraphicsOwnedResourceCount());
    xrtFree(bytes);
    assert(xgeUnitChecked()==XGE_OK);
    printf("ANIMATION3D_NATIVE_OK clipOOM=%u successes=%u\n",refused,succeeded);
    return 0;
}
