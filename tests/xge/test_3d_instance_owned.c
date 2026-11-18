#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <assert.h>
#include <stdio.h>

static const xge3d_work_budget_t one={.nodes=1};
static const xge3d_work_budget_t all={.nodes=SIZE_MAX};
static const xge3d_node_t none={0};

static int32 wrong_thread(void *raw)
{
    xge3d_instance_builder *builder=raw;
    bool consumed=true;
    int completed=1;
    xge3d_node_t root={1,1};
    assert(xge3dModelInstanceStep(builder,&one,&completed)==XGE_ERROR_INVALID_STATE && !completed);
    assert(xge3dModelInstanceTake(builder,&root)==XGE_ERROR_INVALID_STATE && !root.slot);
    assert(xge3dModelInstanceReleaseOwnerChecked(builder,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    return 0;
}

static void finish(xge3d_instance_builder *builder)
{
    int completed=0;
    for(size_t i=0;i<64 && !completed;++i)
        assert(xge3dModelInstanceStep(builder,&one,&completed)==XGE_OK);
    assert(completed);
}

static void fault_paths(xge3d_model *model)
{
    int rejected=0,succeeded=0;
    for(int budget=0;budget<24;++budget){
        xge3d_scene *scene=NULL;
        xge3d_instance_builder *builder=NULL;
        int completed=0;
        assert(xge3dSceneCreateOwned(&scene)==XGE_OK);
        assert(xrtMemDebugFailAfter(budget));
        int status=xge3dModelInstanceBeginOwned(scene,model,none,&builder);
        if(status==XGE_OK)status=xge3dModelInstanceStep(builder,&all,&completed);
        assert(xrtMemDebugFailAfter(-1));
        xrtClearError();
        assert(xge3dSceneNodeCount(scene)==0);
        if(status==XGE_OK){assert(completed);++succeeded;}
        else{assert(status==XGE_ERROR_OUT_OF_MEMORY && !completed);++rejected;}
        if(builder)assert(xge3dModelInstanceReleaseChecked(builder)==XGE_OK);
        assert(xge3dSceneNodeCount(scene)==0);
        assert(xge3dSceneReleaseChecked(scene)==XGE_OK);
    }
    assert(rejected>=6 && succeeded>0);
    printf("INSTANCE3D_FAULT_COUNTS %d %d\n",rejected,succeeded);
}

static void skin_cleanup(void)
{
    xge3d_model *model=NULL;
    xge3d_scene *scene=NULL;
    assert(xge3dModelLoadOwned("artifacts/xge-3d/fixtures/skin.gltf",NULL,&model)==XGE_OK);
    assert(xge3dSceneCreateOwned(&scene)==XGE_OK);
    for(int publish=0;publish<=1;++publish){
        xge3d_instance_builder *builder=NULL;
        assert(xge3dModelInstanceBeginOwned(scene,model,none,&builder)==XGE_OK);
        finish(builder);
        assert(xge3dSceneNodeCount(scene)==0);
        if(publish){
            xge3d_node_t root;
            assert(xge3dModelInstanceTake(builder,&root)==XGE_OK);
            assert(xge3dNodeDestroy(scene,root)==XGE_OK);
        }
        assert(xge3dModelInstanceReleaseChecked(builder)==XGE_OK);
        assert(!xrtGetError() && xge3dSceneNodeCount(scene)==0);
    }
    assert(xge3dSceneReleaseChecked(scene)==XGE_OK);
    assert(xge3dModelReleaseChecked(model)==XGE_OK);
}

int main(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xge3d_model *model=NULL;
    assert(xge3dModelLoadOwned("model-triangle.gltf",NULL,&model)==XGE_OK);
    fault_paths(model);
    skin_cleanup();

    xge3d_scene *scene=NULL,*again=NULL;
    xge3d_instance_builder *builder=NULL,*discard=NULL;
    assert(xge3dSceneCreateOwned(&scene)==XGE_OK);
    assert(xge3dModelInstanceBeginOwned(scene,model,none,&builder)==XGE_OK);
    assert(xge3dModelInstanceBeginOwned(scene,model,none,&discard)==XGE_OK);
    assert(xge3dModelReleaseChecked(model)==XGE_OK);
    assert(xge3dSceneReleaseChecked(scene)==XGE_OK);
    /* At this point only the builders retain the scene. */
    assert(xge3dSceneShareOwned(scene,&again)==XGE_OK && again==scene);
    xthread *thread=xrtThreadCreate(wrong_thread,builder,0);
    assert(thread && xrtThreadWait(thread)==XWAIT_OK);
    xrtThreadDestroy(thread);

    int completed=1;
    xge3d_work_budget_t zero={0};
    assert(xge3dModelInstanceStep(builder,&zero,&completed)==XGE_OK && !completed);
    assert(xge3dModelInstanceStep(discard,&one,&completed)==XGE_OK && !completed);
    assert(xge3dModelInstanceReleaseChecked(discard)==XGE_OK);
    finish(builder);
    assert(xge3dSceneNodeCount(again)==0 && !xge3dSceneNodeAt(again,0).slot);
    assert(xge3dSceneSetOrigin(again,(xge3d_dvec3_t){123,0,0})==XGE_OK);
    xge3d_node_t root={1,1};
    assert(xrtMemDebugFailAfter(0));
    int status=xge3dModelInstanceTake(builder,&root);
    assert(xrtMemDebugFailAfter(-1));
    assert(status==XGE_ERROR_OUT_OF_MEMORY && !root.slot && xge3dSceneNodeCount(again)==0);
    xrtClearError();

    xge3d_scene_stats_t stats={.size=sizeof(stats)};
    assert(xge3dSceneStats(again,&stats,0)==XGE_OK && !stats.nodes && !stats.destroyed_nodes);
    assert(xge3dModelInstanceTake(builder,&root)==XGE_OK && root.slot);
    assert(xge3dSceneNodeCount(again)==2 && xge3dNodeChildCount(again,root)==1);
    xge3d_node_t selected,outside;
    assert(xge3dModelInstanceNode(again,root,0,&selected)==XGE_OK);
    assert(xge3dModelInstanceNode(again,root,1,&outside)==XGE_ERROR_NOT_FOUND && !outside.slot);
    assert(xge3dSceneSetOrigin(again,(xge3d_dvec3_t){0,0,0})==XGE_OK);
    xge3d_mat4_t matrix;
    assert(xge3dNodeGetWorldMatrix(again,selected,&matrix)==XGE_OK && matrix.m[12]==123);
    assert(xge3dModelInstanceReleaseChecked(builder)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()>0 && xgeUnitChecked()==XGE_ERROR_BUSY);
    xrtClearError();
    assert(xge3dSceneReleaseChecked(again)==XGE_OK);
    assert(!xgeGraphicsOwnedResourceCount() && xgeUnitChecked()==XGE_OK);
    puts("MODEL_INSTANCE3D_NATIVE_OK");
    return 0;
}
