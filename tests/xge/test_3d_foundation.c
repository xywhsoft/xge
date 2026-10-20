#include "xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do {++checks;if(!(x)){fprintf(stderr,"%d: %s\n",__LINE__,#x);return 1;}}while(0)
#define CLOSE(a,b) (fabsf((a)-(b))<.0001f)
static unsigned checks;
static unsigned random_state=7;
static unsigned random_value(void){random_state=random_state*1664525u+1013904223u;return random_state;}
int main(void)
{
    xge_build_info_t build={.size=sizeof(build)};
    CHECK(xgeGetBuildInfo(&build)==XGE_OK&&build.foundation_revision==XGE_FOUNDATION_API_REVISION);
    CHECK(xgeCheckBuildCompatibility(XGE_BUILD_ABI_FINGERPRINT,xgeGetBuildFeatures())==XGE_OK);
    CHECK(xgeCheckBuildCompatibility(build.abi_fingerprint^1,build.features)==XGE_ERROR_UNSUPPORTED);
    CHECK(xgeCheckBuildCompatibility(build.abi_fingerprint,build.features^1)==XGE_ERROR_UNSUPPORTED);
    xge_desc_t desc={0};desc.iRunMode=XGE_RUN_MANUAL;CHECK(xgeInit(&desc)==XGE_OK);
    xge3d_scene *s=NULL;CHECK(xge3dSceneCreate(&s)==XGE_OK);
    xge3d_vertex_t vertices[3]={0};vertices[1].position.x=1;vertices[2].position.y=1;
    for(int i=0;i<3;++i){vertices[i].normal.z=1;vertices[i].weights[0]=1;}
    xge3d_mesh_desc_t md={vertices,3,NULL,0,0};xge3d_mesh *mesh=NULL;
    CHECK(xge3dMeshCreate(&md,&mesh)==XGE_OK);
    enum{N=3000};xge3d_node_t nodes[N],parent,unrelated;
    CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&parent)==XGE_OK);
    for(int i=0;i<N;++i){CHECK(xge3dNodeCreate(s,parent,&nodes[i])==XGE_OK);
        CHECK(xge3dNodeSetMesh(s,nodes[i],mesh)==XGE_OK);
        xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position=(xge3d_vec3_t){(float)(i%100)*3-150,(float)(i/100)*3-45,(float)(i%7)};
        CHECK(xge3dNodeSetTransform(s,nodes[i],&t)==XGE_OK);
        CHECK(xge3dNodeSetLayers(s,nodes[i],1,1u<<(i%3),1)==XGE_OK);
        if(i%11==0)CHECK(xge3dNodeSetVisible(s,nodes[i],0)==XGE_OK);
    }
    CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&unrelated)==XGE_OK);
    CHECK(xge3dScenePrepareSpatial(s)==XGE_OK);
    xge3d_scene_stats_t stats={.size=sizeof(stats)};CHECK(xge3dSceneStats(s,&stats,1)==XGE_OK);
    for(int round=0;round<80;++round){
        float x=(int)(random_value()%300)-150,y=(int)(random_value()%90)-45;
        xge3d_aabb_t query={{x,y,-1},{x+12,y+12,8}};
        xge3d_query_filter_t filter={1u<<(round%3),round%2};xge3d_node_t hits[N];size_t count=0,expected=0;
        CHECK(xge3dSceneQueryAabbEx(s,&query,&filter,hits,N,&count)==XGE_OK);
        unsigned char found[N];memset(found,0,sizeof(found));
        for(size_t j=0;j<count;++j){int match=-1;for(int i=0;i<N;++i)if(nodes[i].slot==hits[j].slot){match=i;break;}
            CHECK(match>=0&&!found[match]);found[match]=1;}
        for(int i=0;i<N;++i){xge3d_aabb_t bounds;CHECK(xge3dNodeGetBounds(s,nodes[i],0,&bounds)==XGE_OK);
            int wanted=(i%3==round%3)&&(filter.include_hidden||i%11!=0)&&xge3dAabbIntersects(&bounds,&query);
            CHECK((int)found[i]==wanted);expected+=(size_t)wanted;}
        CHECK(expected==count);
    }
    CHECK(xge3dSceneStats(s,&stats,1)==XGE_OK&&stats.world_updates==0&&stats.bounds_updates==0);
    CHECK(stats.query_mesh_tests<(uint64_t)N*80/10);
    xge3d_transform_t t=XGE3D_TRANSFORM_IDENTITY;t.position.x=10;
    CHECK(xge3dNodeSetTransform(s,unrelated,&t)==XGE_OK);
    CHECK(xge3dScenePrepareSpatial(s)==XGE_OK);
    CHECK(xge3dSceneStats(s,&stats,1)==XGE_OK&&stats.bounds_updates==0&&stats.world_updates==0);
    t.position.x=1000;CHECK(xge3dNodeSetTransform(s,nodes[1],&t)==XGE_OK);
    CHECK(xge3dScenePrepareSpatial(s)==XGE_OK);
    CHECK(xge3dSceneStats(s,&stats,1)==XGE_OK&&stats.spatial_refits==1&&stats.bounds_updates==1&&stats.spatial_builds==0);
    t.position.x=20;CHECK(xge3dNodeSetTransform(s,parent,&t)==XGE_OK);
    CHECK(xge3dScenePrepareSpatial(s)==XGE_OK);
    CHECK(xge3dSceneStats(s,&stats,1)==XGE_OK&&stats.bounds_updates==N);
    vertices[1].position.x=2;CHECK(xge3dMeshUpdate(mesh,&md)==XGE_OK);
    CHECK(xge3dScenePrepareSpatial(s)==XGE_OK);
    CHECK(xge3dSceneStats(s,&stats,1)==XGE_OK&&stats.bounds_updates==N);
    xge3d_node_t joint,cloth;CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&joint)==XGE_OK);
    CHECK(xge3dNodeCreate(s,(xge3d_node_t){0},&cloth)==XGE_OK&&xge3dNodeSetMesh(s,cloth,mesh)==XGE_OK);
    xge3d_skin_binding_t binding={&joint,NULL,1};CHECK(xge3dNodeBindSkin(s,cloth,&binding)==XGE_OK);
    xge3d_aabb_t before,after;CHECK(xge3dNodeGetBounds(s,cloth,0,&before)==XGE_OK);
    t=(xge3d_transform_t)XGE3D_TRANSFORM_IDENTITY;t.position.x=7;
    CHECK(xge3dNodeSetTransform(s,joint,&t)==XGE_OK&&xge3dNodeGetBounds(s,cloth,0,&after)==XGE_OK);
    CHECK(CLOSE(after.min.x-before.min.x,7));
    xge3d_node_t invalid={UINT64_MAX,1};binding.joints=&invalid;
    CHECK(xge3dNodeBindSkin(s,cloth,&binding)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dNodeGetBounds(s,cloth,0,&after)==XGE_OK&&CLOSE(after.min.x,7));
    CHECK(xge3dNodeBindSkin(s,cloth,NULL)==XGE_OK&&xge3dNodeDestroy(s,joint)==XGE_OK);
    CHECK(xge3dNodeDestroy(s,cloth)==XGE_OK);
    xge3d_work_budget_t budget={7,0};int completed=0;size_t remaining=xge3dSceneNodeCount(s);
    CHECK(xge3dNodeDestroyStep(s,parent,&budget,&completed)==XGE_OK&&!completed);
    CHECK(remaining-xge3dSceneNodeCount(s)==7);
    CHECK(xge3dNodeSetVisible(s,nodes[200],1)==XGE_ERROR_INVALID_ARGUMENT);
    while(!completed){remaining=xge3dSceneNodeCount(s);CHECK(xge3dNodeDestroyStep(s,parent,&budget,&completed)==XGE_OK);
        CHECK(remaining-xge3dSceneNodeCount(s)<=7);}
    CHECK(xge3dSceneNodeCount(s)==1);xge3dMeshFree(mesh);
    xge3d_model *model=NULL;CHECK(xge3dModelLoad("artifacts/xge-3d/fixtures/skin.gltf",&model)==XGE_OK);
    xge3d_instance_builder *builder=NULL;CHECK(xge3dModelInstanceBegin(s,model,(xge3d_node_t){0},&builder)==XGE_OK);
    budget.nodes=1;completed=0;int iterations=0;
    while(!completed){remaining=xge3dSceneNodeCount(s);CHECK(xge3dModelInstanceStep(builder,&budget,&completed)==XGE_OK);
        CHECK(xge3dSceneNodeCount(s)-remaining<=1);CHECK(++iterations<100);}
    xge3d_node_t root;CHECK(xge3dModelInstanceTake(builder,&root)==XGE_OK);xge3dModelInstanceFree(builder);
    xge3d_animator *anim=NULL;CHECK(xge3dAnimatorCreate(s,root,&anim)==XGE_OK);
    xge3d_clip *clip=NULL;CHECK(xge3dClipLoad("artifacts/xge-3d/fixtures/motion-linear.gltf",0,&clip)==XGE_OK);
    xge3d_animation_layer_t layer=xge3dAnimationLayerDefault();layer.time=.5f;layer.loop=0;
    CHECK(xge3dAnimatorSetLayer(anim,0,clip,&layer)==XGE_OK);
    layer.weight=.25f;CHECK(xge3dAnimatorSetLayer(anim,7,clip,&layer)==XGE_OK);
    CHECK(xge3dAnimatorSetLayerMode(anim,7,XGE3D_ANIMATION_ADDITIVE)==XGE_OK);
    CHECK(xge3dAnimatorUpdate(anim,0)==XGE_OK);
    xge3d_transform_t pose;CHECK(xge3dAnimatorGetPose(anim,3,&pose)==XGE_OK&&CLOSE(pose.position.x,2.25f));
    pose.position.y=3;CHECK(xge3dAnimatorSetPose(anim,3,&pose)==XGE_OK);
    CHECK(xge3dAnimatorGetPose(anim,3,&pose)==XGE_OK&&CLOSE(pose.position.y,3));
    CHECK(xge3dAnimatorUpdate(anim,0)==XGE_OK&&xge3dAnimatorGetPose(anim,3,&pose)==XGE_OK&&CLOSE(pose.position.y,0));
    CHECK(xge3dAnimatorSetLayerMode(anim,8,0)==XGE_ERROR_INVALID_ARGUMENT);
    layer=xge3dAnimationLayerDefault();layer.loop=0;
    CHECK(xge3dAnimatorSetLayer(anim,0,clip,&layer)==XGE_OK);
    layer.weight=.25f;CHECK(xge3dAnimatorSetLayer(anim,7,clip,&layer)==XGE_OK);
    xge3d_animation_event_t mark={7,77,.25f},event;
    CHECK(xge3dAnimatorSetEvents(anim,7,&mark,1)==XGE_OK);
    CHECK(xge3dAnimatorSetRootMotion(anim,3,XGE3D_ROOT_X)==XGE_OK&&xge3dAnimatorUpdate(anim,.5f)==XGE_OK);
    xge3d_root_motion_t motion;CHECK(xge3dAnimatorTakeRootMotion(anim,&motion)==XGE_OK&&CLOSE(motion.translation.x,1.25f));
    CHECK(xge3dAnimatorNextEvent(anim,&event)==XGE_OK&&event.layer==7&&event.id==77);
    CHECK(xge3dAnimatorGetPose(anim,3,&pose)==XGE_OK&&CLOSE(pose.position.x,1));
    CHECK(xge3dAnimatorUpdate(anim,NAN)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dAnimatorGetPose(anim,3,&pose)==XGE_OK&&CLOSE(pose.position.x,1));
    xge3dClipFree(clip);xge3dAnimatorFree(anim);CHECK(xge3dNodeDestroy(s,root)==XGE_OK);
    CHECK(xge3dModelInstanceBegin(s,model,(xge3d_node_t){0},&builder)==XGE_OK);
    CHECK(xge3dModelInstanceStep(builder,&budget,&completed)==XGE_OK);xge3dModelInstanceFree(builder);
    CHECK(xge3dSceneNodeCount(s)==1);xge3dModelFree(model);
    /* Deep hierarchy traversal and deletion must not recurse on the C stack. */
    xge3d_node_t chain=unrelated;for(int i=0;i<10000;++i){xge3d_node_t child;CHECK(xge3dNodeCreate(s,chain,&child)==XGE_OK);chain=child;}
    xge3d_mat4_t world;CHECK(xge3dNodeGetWorldMatrix(s,chain,&world)==XGE_OK);
    CHECK(xge3dNodeDestroy(s,unrelated)==XGE_OK&&xge3dSceneNodeCount(s)==0);
    xge3dSceneFree(s);xgeUnit();printf("Foundation CPU: %u checks passed\n",checks);return 0;
}
