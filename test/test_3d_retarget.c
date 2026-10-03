#include "../xge.h"
#include <stdio.h>
#include <math.h>
#define CHECK(x) do {if (!(x)) {fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);return 1;}} while(0)
#define CLOSE(a,b) (fabsf((a)-(b))<.0003f)
static int probe(void)
{
    xge3d_scene *s=NULL;xge3d_model *model=NULL;xge3d_node_t root,hand,hips,lower,upper;xge3d_animator *anim=NULL;xge3d_clip *clip=NULL;
    CHECK(xge3dModelLoad("artifacts/xge-3d/fixtures/rig-b.gltf",&model)==XGE_OK);
    CHECK(xge3dSceneCreate(&s)==XGE_OK && xge3dModelInstantiate(s,model,(xge3d_node_t){0},&root)==XGE_OK);
    CHECK(xge3dAnimatorCreate(s,root,&anim)==XGE_OK && xge3dAnimatorBoneNode(anim,"B_left_hand",&hand)==XGE_OK);
    CHECK(xge3dAnimatorBoneNode(anim,"B_left_lower_arm",&lower)==XGE_OK && xge3dAnimatorBoneNode(anim,"B_left_upper_arm",&upper)==XGE_OK && xge3dAnimatorBoneNode(anim,"B_hips",&hips)==XGE_OK);
    xge3d_mat4_t matrix;CHECK(xge3dNodeGetWorldMatrix(s,hand,&matrix)==XGE_OK && matrix.m[13]<1.4f); /* Actual A bind pose. */
    CHECK(xge3dClipLoad("artifacts/xge-3d/fixtures/probe-b.gltf",0,&clip)==XGE_OK);
    xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.loop=0;
    CHECK(xge3dAnimatorSetLayer(anim,0,clip,&layer)==XGE_OK && xge3dAnimatorUpdate(anim,0)==XGE_OK);
    CHECK(xge3dNodeGetWorldMatrix(s,hand,&matrix)==XGE_OK && CLOSE(matrix.m[12],1.35f) && CLOSE(matrix.m[13],1.95f));
    CHECK(xge3dAnimatorUpdate(anim,1)==XGE_OK && xge3dNodeGetWorldMatrix(s,hand,&matrix)==XGE_OK);
    float ratio=(2.35f-.06f)/(1.9f-.06f);
    CHECK(CLOSE(matrix.m[12],.32f+1.03f*.5f) && CLOSE(matrix.m[13],1.95f+.1f*ratio-1.03f*sqrtf(.75f)) && CLOSE(matrix.m[14],ratio));
    CHECK(xge3dNodeGetWorldMatrix(s,hips,&matrix)==XGE_OK && CLOSE(matrix.m[13],1.2f+.1f*ratio));
    xge3d_mat4_t a,b,c;CHECK(xge3dNodeGetWorldMatrix(s,upper,&a)==XGE_OK && xge3dNodeGetWorldMatrix(s,lower,&b)==XGE_OK && xge3dNodeGetWorldMatrix(s,hand,&c)==XGE_OK);
    float length1=0,length2=0;for (int k=12;k<15;++k) {length1+=(a.m[k]-b.m[k])*(a.m[k]-b.m[k]);length2+=(b.m[k]-c.m[k])*(b.m[k]-c.m[k]);}
    CHECK(CLOSE(sqrtf(length1),.55f) && CLOSE(sqrtf(length2),.48f));
    xge3dAnimatorFree(anim);xge3dClipFree(clip);xge3dSceneFree(s);xge3dModelFree(model);return 0;
}
int main(void)
{
    xge_desc_t d={0};d.iRunMode=XGE_RUN_MANUAL;CHECK(xgeInit(&d)==XGE_OK && probe()==0);
    const char *actions[]={"idle","walk","run","jump","slash","gather"};
    for (int rig=0;rig<2;++rig) for (int action=0;action<6;++action) {
        char path[256];snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/rig-%c.gltf",'a'+rig);
        xge3d_model *model=NULL;xge3d_clip *clip=NULL;xge3d_scene *s=NULL;xge3d_node_t root,upper,lower,hand,mesh;xge3d_animator *anim=NULL;
        CHECK(xge3dModelLoad(path,&model)==XGE_OK && xge3dSceneCreate(&s)==XGE_OK && xge3dModelInstantiate(s,model,(xge3d_node_t){0},&root)==XGE_OK);
        snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/retarget-%c-%s.gltf",'a'+rig,actions[action]);CHECK(xge3dClipLoad(path,0,&clip)==XGE_OK);
        CHECK(xge3dAnimatorCreate(s,root,&anim)==XGE_OK);xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.loop=0;
        CHECK(xge3dAnimatorSetLayer(anim,0,clip,&layer)==XGE_OK && xge3dModelInstanceNode(s,root,17,&mesh)==XGE_OK);
        char name[64];snprintf(name,sizeof(name),"%c_left_upper_arm",'A'+rig);CHECK(xge3dAnimatorBoneNode(anim,name,&upper)==XGE_OK);
        snprintf(name,sizeof(name),"%c_left_lower_arm",'A'+rig);CHECK(xge3dAnimatorBoneNode(anim,name,&lower)==XGE_OK);
        snprintf(name,sizeof(name),"%c_left_hand",'A'+rig);CHECK(xge3dAnimatorBoneNode(anim,name,&hand)==XGE_OK);
        float delta=xge3dClipInfo(clip).duration/60;
        xge3dClipFree(clip);clip=NULL;xge3dModelFree(model);model=NULL;
        for (int frame=0;frame<60;++frame) {
            CHECK(xge3dAnimatorUpdate(anim,delta)==XGE_OK);const xge3d_mat4_t *palette;size_t count;
            CHECK(xge3dNodeSkinMatrices(s,mesh,&palette,&count)==XGE_OK && count==17);
            for (size_t j=0;j<count;++j) for (int k=0;k<16;++k) CHECK(isfinite(palette[j].m[k]));
            xge3d_mat4_t a,b,c;CHECK(xge3dNodeGetWorldMatrix(s,upper,&a)==XGE_OK && xge3dNodeGetWorldMatrix(s,lower,&b)==XGE_OK && xge3dNodeGetWorldMatrix(s,hand,&c)==XGE_OK);
            float x=0,y=0;for (int k=12;k<15;++k) {x+=(a.m[k]-b.m[k])*(a.m[k]-b.m[k]);y+=(b.m[k]-c.m[k])*(b.m[k]-c.m[k]);}
            CHECK(CLOSE(sqrtf(x),rig ? .55f : .38f) && CLOSE(sqrtf(y),rig ? .48f : .32f));
        }
        xge3dAnimatorFree(anim);xge3dSceneFree(s);
    }
    xgeUnit();puts("3D retarget CPU: reference T/A pose and different local axes, known world rotation/root/pelvis scaling, preserved target arm lengths, 6 external actions on 2 distinct rigs passed");return 0;
}
