#include "xge.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"nodes %d: %s\n",__LINE__,#x);return 1;}}while(0)
static int near_matrix(xge3d_mat4_t a,xge3d_mat4_t b){for(int i=0;i<16;++i)if(fabsf(a.m[i]-b.m[i])>1e-4f)return 0;return 1;}
#if XGE3D_ENABLE_MODEL
static int resolve(const char *uri,xge3d_model **out,void *user){++*(int*)user;return xge3dModelLoad(uri,out);}
#endif
int main(void)
{
    xge3d_scene *s=NULL;CHECK(!xge3dSceneCreate(&s));xge3d_node_t root,a,b,copy,found,parent;
    CHECK(!xge3dNodeCreate(s,(xge3d_node_t){0},&root)&&!xge3dNodeCreate(s,root,&a)&&!xge3dNodeCreate(s,a,&b));
    char name[]="copied-name";CHECK(!xge3dNodeSetName(s,b,name));name[0]='X';CHECK(!strcmp(xge3dNodeName(s,b),"copied-name"));
    CHECK(!xge3dNodeSetName(s,b,xge3dNodeName(s,b))&&!xge3dNodeFind(s,root,"copied-name",&found)&&found.slot==b.slot);
    CHECK(!xge3dNodeGetParent(s,b,&parent)&&parent.slot==a.slot&&xge3dNodeChildCount(s,root)==1&&xge3dNodeChildAt(s,root,0).slot==a.slot);
    CHECK(xge3dNodeChildCount(s,(xge3d_node_t){0})==1&&!xge3dNodeChildAt(s,root,1).slot);
    int token=3;CHECK(!xge3dNodeSetUserData(s,b,&token)&&xge3dNodeUserData(s,b)==&token);
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY,read;xrng rng;xrtRngSeed(&rng,123,9);
    for(int i=0;i<200;++i){t.position=(xge3d_vec3_t){i*.1f,2,-3};t.rotation=(xge3d_quat_t){(float)xrtRngReal(&rng),(float)xrtRngReal(&rng),(float)xrtRngReal(&rng),(float)xrtRngReal(&rng)};
        t.scale=(xge3d_vec3_t){i%2?-2:2,.3f,4};xge3d_mat4_t expected,actual;
        CHECK(!xge3dTransformMatrix(&t,&expected)&&!xge3dNodeSetTransform(s,a,&t)&&!xge3dNodeGetTransform(s,a,&read)&&!xge3dTransformMatrix(&read,&actual)&&near_matrix(expected,actual));}
    xge3d_mat4_t shear={{1,0,0,0,.5f,1,0,0,0,0,1,0,1,2,3,1}};read=t;
    CHECK(!xge3dNodeSetMatrix(s,a,&shear)&&xge3dNodeGetTransform(s,a,&read)==XGE_ERROR_UNSUPPORTED&&!memcmp(&read,&t,sizeof(t)));
#if XGE3D_ENABLE_SHADOW
    CHECK(!xge3dNodeSetShadowFlags(s,b,XGE3D_SHADOW_RECEIVE));
#endif
    CHECK(!xge3dNodeClone(s,root,(xge3d_node_t){0},0,&copy)&&!xge3dNodeFind(s,copy,"copied-name",&found)&&found.slot!=b.slot&&!xge3dNodeUserData(s,found));
#if XGE3D_ENABLE_SHADOW
    uint32_t flags;CHECK(!xge3dNodeGetShadowFlags(s,found,&flags)&&flags==XGE3D_SHADOW_RECEIVE);
    CHECK(!xge3dNodeSetShadowFlags(s,b,3)&&!xge3dNodeGetShadowFlags(s,found,&flags)&&flags==XGE3D_SHADOW_RECEIVE);
#endif
    xge3d_mat4_t actual;CHECK(!xge3dNodeGetLocalMatrix(s,xge3dNodeChildAt(s,copy,0),&actual)&&near_matrix(shear,actual));
    CHECK(!xge3dNodeDestroy(s,copy)&&!xge3dNodeClone(s,root,(xge3d_node_t){0},XGE3D_CLONE_USER_DATA,&copy));
    CHECK(!xge3dNodeFind(s,copy,"copied-name",&found)&&xge3dNodeUserData(s,found)==&token&&!xge3dNodeDestroy(s,copy));
    size_t before=xge3dSceneNodeCount(s);xallocationbudget *quota=NULL;xallocationscope scope={0};CHECK(xrtAllocationBudgetCreate(1500,&quota)&&xrtAllocationBudgetEnter(quota,&scope));
    int result=xge3dNodeClone(s,root,(xge3d_node_t){0},0,&copy);xrtAllocationBudgetLeave(&scope);
    CHECK(result==XGE_ERROR_OUT_OF_MEMORY&&!copy.slot&&xge3dSceneNodeCount(s)==before);xallocationbudgetinfo qi={.Size=sizeof(qi)};
    CHECK(xrtAllocationBudgetInfo(quota,&qi)&&!qi.LiveBytes);xrtAllocationBudgetFree(quota);
    xge3d_scene_node_desc_t desc[2]={xge3dSceneNodeDefault(),xge3dSceneNodeDefault()};desc[0].name="group";desc[0].transform.position.x=3;
    desc[1].name="item";desc[1].parent=0;desc[1].transform.position.y=2;xge3d_scene_desc_t data={.nodes=desc,.count=2};
    CHECK(!xge3dSceneLoad(s,(xge3d_node_t){0},&data,&copy)&&!xge3dNodeFind(s,copy,"item",&found)&&!xge3dNodeGetWorldMatrix(s,found,&actual)&&actual.m[12]==3&&actual.m[13]==2);
    CHECK(!xge3dNodeDestroy(s,copy));desc[0].parent=1;CHECK(xge3dSceneLoad(s,(xge3d_node_t){0},&data,&copy)==XGE_ERROR_INVALID_ARGUMENT&&!copy.slot);desc[0].parent=SIZE_MAX;
    char *long_name=xrtMalloc(1048578);CHECK(long_name);memset(long_name,'x',1048577);long_name[1048577]=0;desc[1].name=long_name;before=xge3dSceneNodeCount(s);
    CHECK(xge3dSceneLoad(s,(xge3d_node_t){0},&data,&copy)==XGE_ERROR_INVALID_ARGUMENT&&!copy.slot&&xge3dSceneNodeCount(s)==before);xrtFree(long_name);desc[1].name="item";
#if XGE3D_ENABLE_COLLISION
    xge3d_shape_t shape=xge3dShapeDefault(XGE3D_SHAPE_SPHERE),got;CHECK(!xge3dNodeSetCollider(s,b,&shape)&&!xge3dNodeClone(s,root,(xge3d_node_t){0},0,&copy));
    CHECK(!xge3dNodeFind(s,copy,"copied-name",&found));shape.radius=2;CHECK(!xge3dNodeSetCollider(s,b,&shape)&&!xge3dNodeGetCollider(s,found,&got)&&got.radius==.5f&&!xge3dNodeDestroy(s,copy));
#endif
#if XGE3D_ENABLE_MODEL
    int loads=0;data.load_model=resolve;data.user=&loads;desc[0].model_uri="artifacts/xge-3d/fixtures/embedded.gltf";desc[1].model_uri="artifacts/xge-3d/fixtures/absent.gltf";
    before=xge3dSceneNodeCount(s);CHECK(xge3dSceneLoad(s,(xge3d_node_t){0},&data,&copy)!=XGE_OK&&!copy.slot&&loads==2&&xge3dSceneNodeCount(s)==before);
#if XGE3D_ENABLE_ANIMATION
    xge3d_model *model=NULL;CHECK(!xge3dModelLoad("artifacts/xge-3d/fixtures/skin.gltf",&model)&&!xge3dModelInstantiate(s,model,(xge3d_node_t){0},&a));xge3dModelFree(model);
    CHECK(!xge3dNodeClone(s,a,(xge3d_node_t){0},0,&copy));xge3d_animator *anim=NULL;CHECK(!xge3dAnimatorCreate(s,copy,&anim));
    xge3d_clip *clip=NULL;CHECK(!xge3dClipLoad("artifacts/xge-3d/fixtures/motion-linear.gltf",0,&clip));xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.time=.5f;layer.loop=0;
    CHECK(!xge3dAnimatorSetLayer(anim,0,clip,&layer)&&!xge3dAnimatorUpdate(anim,0));
    xge3d_node_t original_joint,new_joint,mesh;CHECK(!xge3dModelInstanceNode(s,a,3,&original_joint)&&!xge3dModelInstanceNode(s,copy,3,&new_joint));
    xge3d_mat4_t old_pose,new_pose;CHECK(!xge3dNodeGetLocalMatrix(s,original_joint,&old_pose)&&!xge3dNodeGetLocalMatrix(s,new_joint,&new_pose)&&old_pose.m[12]!=new_pose.m[12]);
    CHECK(!xge3dModelInstanceNode(s,a,1,&mesh));before=xge3dSceneNodeCount(s);CHECK(xge3dNodeClone(s,mesh,(xge3d_node_t){0},0,&found)==XGE_ERROR_UNSUPPORTED&&!found.slot&&xge3dSceneNodeCount(s)==before);
    CHECK(!xge3dNodeDestroy(s,a)&&!xge3dAnimatorUpdate(anim,.1f));xge3dAnimatorFree(anim);xge3dClipFree(clip);
#endif
#endif
    CHECK(!xge3dNodeDestroy(s,root)&&xge3dNodeFind(s,root,"copied-name",&found)==XGE_ERROR_INVALID_ARGUMENT&&!found.slot);
    xge3dSceneFree(s);puts("Nodes: affine/TRS, hierarchy/name/user data, clone rollback, independent skin/model bindings and declarative scene load passed");return 0;
}
