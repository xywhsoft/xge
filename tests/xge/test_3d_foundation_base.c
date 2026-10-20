#include "xge.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"%d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void)
{
    xge_build_info_t info={.size=sizeof(info)};CHECK(xgeGetBuildInfo(&info)==XGE_OK);
    CHECK(xgeCheckBuildCompatibility(XGE_BUILD_ABI_FINGERPRINT,XGE_FEATURES)==XGE_OK);
    xge3d_scene *s=NULL;xge3d_mesh *mesh=NULL;xge3d_node_t root,child;
    xge3d_vertex_t v[3]={0};v[1].position.x=1;v[2].position.y=1;
    xge3d_mesh_desc_t md={v,3,NULL,0,0};
    CHECK(!xge3dSceneCreate(&s)&&!xge3dMeshCreate(&md,&mesh)&&!xge3dNodeCreate(s,(xge3d_node_t){0},&root));
    for(int i=0;i<256;++i)CHECK(!xge3dNodeCreate(s,root,&child)&&!xge3dNodeSetMesh(s,child,mesh));
    xge3d_aabb_t box={{-1,-1,-1},{2,2,2}};size_t count=0;
    CHECK(!xge3dSceneQueryAabb(s,&box,NULL,0,&count)&&count==256);
    CHECK(!xge3dNodeSetVisible(s,root,0)&&!xge3dSceneQueryAabb(s,&box,NULL,0,&count)&&!count);
    xge3d_query_filter_t filter={UINT32_MAX,1};CHECK(!xge3dSceneQueryAabbEx(s,&box,&filter,NULL,0,&count)&&count==256);
    CHECK(!xge3dNodeSetLayers(s,child,0,0,0)&&!xge3dSceneQueryAabbEx(s,&box,&filter,NULL,0,&count)&&count==255);
    xge3d_scene_stats_t stats={.size=sizeof(stats)};CHECK(!xge3dSceneStats(s,&stats,0)&&stats.spatial_builds==1);
    xge3d_resource_info_t resource={.size=sizeof(resource)};
    CHECK(!xge3dMeshResourceInfo(mesh,&resource)&&resource.references==257&&resource.vertices==3&&!resource.gpu_payload_bytes);
    xge3d_work_budget_t budget={0,0};int complete=0;CHECK(!xge3dNodeDestroyStep(s,root,&budget,&complete)&&!complete);
    CHECK(!xge3dSceneQueryAabbEx(s,&box,&filter,NULL,0,&count)&&!count);
    budget.nodes=3;while(!complete)CHECK(!xge3dNodeDestroyStep(s,root,&budget,&complete));
    CHECK(!xge3dSceneNodeCount(s));xge3dMeshFree(mesh);xge3dSceneFree(s);
    puts("Foundation base: SDK identity, BVH, hidden/layer filters, references and budgeted retirement passed");return 0;
}
