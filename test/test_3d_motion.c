#include "../xge.h"
#include <stdio.h>
#include <math.h>
#define CHECK(x) do {if (!(x)) {fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);return 1;}} while(0)
#define CLOSE(a,b) (fabsf((a)-(b))<.0001f)
static int verify_bvh(const char *path,int z_up)
{
    xge3d_model *model=NULL;xge3d_scene *scene=NULL;xge3d_node_t root,tip;xge3d_clip *clip=NULL;xge3d_animator *anim=NULL;
    CHECK(xge3dModelLoad(path,&model)==XGE_OK && xge3dModelInfo(model).node_count==3);
    CHECK(xge3dClipLoad(path,0,&clip)==XGE_OK && xge3dClipInfo(clip).track_count==9 && CLOSE(xge3dClipInfo(clip).duration,1));
    CHECK(xge3dSceneCreate(&scene)==XGE_OK && xge3dModelInstantiate(scene,model,(xge3d_node_t){0},&root)==XGE_OK);
    CHECK(xge3dAnimatorCreate(scene,root,&anim)==XGE_OK && xge3dAnimatorBoneNode(anim,"Tip",&tip)==XGE_OK);
    xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.loop=0;
    CHECK(xge3dAnimatorSetLayer(anim,0,clip,&layer)==XGE_OK && xge3dAnimatorUpdate(anim,.5f)==XGE_OK);
    xge3d_mat4_t matrix;CHECK(xge3dNodeGetWorldMatrix(scene,tip,&matrix)==XGE_OK);
    CHECK(CLOSE(matrix.m[12],1) && CLOSE(matrix.m[z_up ? 14 : 13],z_up ? -2 : 2) && CLOSE(matrix.m[0],-1));
    xge3dAnimatorFree(anim);xge3dClipFree(clip);xge3dModelFree(model);xge3dSceneFree(scene);return 0;
}
int main(void)
{
    xge_desc_t d={0};d.iRunMode=XGE_RUN_MANUAL;CHECK(xgeInit(&d)==XGE_OK);
    CHECK(verify_bvh("artifacts/xge-3d/fixtures/bvh-y.gltf",0)==0);
    CHECK(verify_bvh("artifacts/xge-3d/fixtures/bvh-z.gltf",1)==0);
    const char *actions[]={"idle","walk","run","jump","slash","gather"};
    for (int i=0;i<6;++i) {
        char path[256];snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/kaykit-%s.gltf",actions[i]);
        xge3d_model *model=NULL;xge3d_clip *clip=NULL;xge3d_scene *scene=NULL;xge3d_node_t root,bone;xge3d_animator *anim=NULL;
        CHECK(xge3dModelLoad(path,&model)==XGE_OK && xge3dClipLoad(path,0,&clip)==XGE_OK);
        CHECK(xge3dClipInfo(clip).track_count==xge3dModelInfo(model).node_count*3 && xge3dClipInfo(clip).duration>.5f);
        CHECK(xge3dSceneCreate(&scene)==XGE_OK && xge3dModelInstantiate(scene,model,(xge3d_node_t){0},&root)==XGE_OK);
        CHECK(xge3dAnimatorCreate(scene,root,&anim)==XGE_OK && xge3dAnimatorBoneNode(anim,"hips",&bone)==XGE_OK);
        xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.loop=0;
        CHECK(xge3dAnimatorSetLayer(anim,0,clip,&layer)==XGE_OK);
        xge3d_mat4_t before,after;CHECK(xge3dAnimatorUpdate(anim,0)==XGE_OK && xge3dNodeGetWorldMatrix(scene,bone,&before)==XGE_OK);
        CHECK(xge3dAnimatorUpdate(anim,xge3dClipInfo(clip).duration*.5f)==XGE_OK && xge3dNodeGetWorldMatrix(scene,bone,&after)==XGE_OK);
        CHECK(isfinite(after.m[13]) && fabsf(after.m[13])<10);
        printf("external %s: %.6g sec, hips %.6g -> %.6g m\n",actions[i],xge3dClipInfo(clip).duration,before.m[13],after.m[13]);
        xge3dAnimatorFree(anim);xge3dClipFree(clip);xge3dModelFree(model);xge3dSceneFree(scene);
    }
    xgeUnit();puts("3D offline motion: BVH units/ordered rotations/axes and six independent external FBX actions load and play through public C API passed");return 0;
}
