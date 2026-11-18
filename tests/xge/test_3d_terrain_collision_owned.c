#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <assert.h>
#include <stdio.h>

static const xge3d_mat4_t identity={{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}};
static const xge3d_node_t none={0};
typedef struct thread_probe {
    xge3d_terrain *terrain;
    xge3d_query_results *results;
} thread_probe;

static int32 wrong_thread(void *raw)
{
    const thread_probe *p=raw;
    xge3d_terrain_info_t terrain_info;
    xge3d_query_results_info_t query_info;
    bool consumed=true;
    assert(xge3dTerrainInfoChecked(p->terrain,&terrain_info)==XGE_ERROR_INVALID_STATE);
    assert(xge3dTerrainReleaseOwnerChecked(p->terrain,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    assert(xge3dQueryResultsInfo(p->results,&query_info)==XGE_ERROR_INVALID_STATE);
    assert(xge3dQueryResultsRelease(p->results,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    return 0;
}

int main(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xge3d_motion_settings_t motion={0};
    xge3dMotionDefaultInto(&motion);
    assert(motion.size==sizeof(motion) && motion.iterations==8 && motion.margin==.001f && motion.max_recovery==1);
    float padded[]={0,1,99,0,1};
    xge3d_terrain_options_t options=xge3dTerrainOptionsDefault();
    options.width=2;options.depth=2;options.stride=3*sizeof(float);options.lod_count=2;
    xge3d_terrain *terrain=NULL;
    assert(xge3dTerrainCreateOwned(padded,sizeof(padded)-1,&options,NULL,&terrain)==XGE_ERROR_BUFFER_TOO_SMALL && !terrain);
    assert(!xgeGraphicsOwnedResourceCount());
    unsigned refused=0,created=0;
    for(int allow=0;allow<48;++allow){
        assert(xrtMemDebugFailAfter(allow));
        int result=xge3dTerrainCreateOwned(padded,sizeof(padded),&options,NULL,&terrain);
        assert(xrtMemDebugFailAfter(-1));
        xrtClearError();
        if(result==XGE_OK){assert(xge3dTerrainReleaseChecked(terrain)==XGE_OK);++created;}
        else{assert(result==XGE_ERROR_OUT_OF_MEMORY && !terrain);++refused;}
        assert(!xgeGraphicsOwnedResourceCount());
    }
    assert(refused>=10 && created>0);
    assert(xge3dTerrainCreateOwned(padded,sizeof(padded),&options,NULL,&terrain)==XGE_OK);
    xge3d_query_results *nodes=NULL,*contacts=NULL;
    assert(xge3dNodeResultsCreate(1,&nodes)==XGE_OK && xge3dContactResultsCreate(1,&contacts)==XGE_OK);
    thread_probe probe={terrain,nodes};
    xthread *thread=xrtThreadCreate(wrong_thread,&probe,0);
    assert(thread && xrtThreadWait(thread)==XWAIT_OK);
    xrtThreadDestroy(thread);
    padded[1]=0;
    float height;
    xge3d_vec3_t normal;
    assert(xge3dTerrainSample(terrain,0,.5f,.5f,&height,&normal)==XGE_OK && height==.5f);
    xge3d_scene *scene=NULL;
    xge3d_mesh *mesh=NULL,*shared=NULL;
    xge3d_node_t root,child;
    assert(xge3dSceneCreateOwned(&scene)==XGE_OK);
    assert(xge3dTerrainInstantiate(scene,terrain,none,&root)==XGE_OK);
    child=xge3dNodeChildAt(scene,root,0);
    assert(xge3dTerrainChunkMeshOwned(terrain,0,0,&mesh)==XGE_OK);
    assert(xge3dTerrainReleaseChecked(terrain)==XGE_OK);
    assert(xge3dMeshUpdate(mesh,NULL)==XGE_ERROR_INVALID_STATE);

    xge3d_shape_settings_t mesh_shape=xge3dShapeSettingsDefault(XGE3D_SHAPE_MESH);
    assert(xge3dNodeSetColliderSettings(scene,child,&mesh_shape,mesh)==XGE_OK);
    assert(xge3dNodeColliderMeshOwned(scene,child,&shared)==XGE_OK && shared==mesh);
    assert(xge3dMeshReleaseChecked(mesh)==XGE_OK);
    xge3d_ray_t ray={{.5f,4,.5f},{0,-1,0}};
    xge3d_collision_hit_t hit;
    xge3d_collision_filter_t filter=xge3dCollisionFilterDefault();
    assert(xge3dSceneCollisionRaycast(scene,&ray,8,&filter,&hit)==XGE_OK && hit.node.slot==child.slot);
    xge3d_collision_stats_t stats;
    assert(xge3dSceneCollisionStats(scene,&stats)==XGE_OK && stats.colliders==1);
    assert(xge3dNodeDestroy(scene,root)==XGE_OK);
    xge3d_mesh_desc_t data;
    assert(xge3dMeshGetData(shared,&data)==XGE_OK && data.index_count>0);
    assert(xgeUnitChecked()==XGE_ERROR_BUSY);
    xrtClearError();
    assert(xge3dMeshReleaseChecked(shared)==XGE_OK && !xgeGraphicsOwnedResourceCount());

    xge3d_shape_settings_t shape=xge3dShapeSettingsDefault(XGE3D_SHAPE_BOX);
    xge3d_node_t a,b;
    assert(xge3dNodeCreate(scene,none,&a)==XGE_OK && xge3dNodeCreate(scene,none,&b)==XGE_OK);
    assert(xge3dNodeSetColliderSettings(scene,a,&shape,NULL)==XGE_OK);
    assert(xge3dNodeSetColliderSettings(scene,b,&shape,NULL)==XGE_OK);
    assert(xge3dSceneOverlapInto(scene,&shape,&identity,&filter,nodes)==XGE_ERROR_BUFFER_TOO_SMALL);
    xge3d_query_results_info_t info;
    assert(xge3dQueryResultsInfo(nodes,&info)==XGE_OK && info.written==1 && info.required==2);
    assert(xrtMemDebugFailAfter(0));
    int result=xge3dQueryResultsResize(nodes,1024);
    assert(xrtMemDebugFailAfter(-1) && result==XGE_ERROR_OUT_OF_MEMORY);
    xrtClearError();
    assert(xge3dQueryResultsInfo(nodes,&info)==XGE_OK && info.capacity==1 && info.required==2);
    assert(xge3dSceneContactInto(scene,&shape,&identity,&filter,contacts)==XGE_ERROR_BUFFER_TOO_SMALL);
    xge3d_contact_t contact;
    assert(xge3dQueryResultsContactAt(contacts,0,&contact)==XGE_OK && contact.depth>0);
    assert(xge3dQueryResultsNodeAt(contacts,0,&a)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xge3dQueryResultsReleaseChecked(contacts)==XGE_OK);
    assert(xge3dQueryResultsReleaseChecked(nodes)==XGE_OK);
    assert(xge3dSceneReleaseChecked(scene)==XGE_OK);
    assert(!xrtGetError() && xgeUnitChecked()==XGE_OK);
    printf("TERRAIN_COLLISION3D_NATIVE_OK faults=%u successes=%u\n",refused,created);
    return 0;
}
