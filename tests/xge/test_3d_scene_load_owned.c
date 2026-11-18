#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <assert.h>
#include <stdio.h>

static const char model_path[]="model-triangle.gltf";
static const xge3d_node_t no_node={0};

static void check_load(void)
{
    xge3d_scene *scene=NULL;
    assert(xge3dSceneCreateOwned(&scene)==XGE_OK);
    xge3d_node_t parent,root,child;
    assert(xge3dNodeCreate(scene,no_node,&parent)==XGE_OK);
    xge3d_scene_node_desc_t rows[2]={xge3dSceneNodeDefault(),xge3dSceneNodeDefault()};
    rows[0].name="world";
    rows[1].name="child";
    rows[1].parent=0;
    rows[1].transform.position.z=-2;
    assert(rows[0].parent==xge3dSceneRootParent());
    xge3dSceneNodeTransformDefault(&rows[0].transform);

    unsigned faults=0,successes=0;
    for(int allow=0;allow<32;++allow){
        assert(xrtMemDebugFailAfter(allow));
        int status=xge3dSceneLoadOwned(scene,parent,rows,2,NULL,0,NULL,&root);
        assert(xrtMemDebugFailAfter(-1));
        xrtClearError();
        if(status==XGE_OK){
            assert(xge3dSceneNodeCount(scene)==4);
            assert(xge3dNodeDestroy(scene,root)==XGE_OK);
            ++successes;
        }else{
            assert(status==XGE_ERROR_OUT_OF_MEMORY && !root.slot);
            ++faults;
        }
        assert(xge3dSceneNodeCount(scene)==1 && !xge3dNodeChildCount(scene,parent));
    }
    assert(faults>=6 && successes>0);

    xge3d_model *provided=NULL;
    xge3d_cache *cache=NULL;
    assert(xge3dModelLoadOwned(model_path,NULL,&provided)==XGE_OK);
    assert(xge3dCacheCreateOwned(NULL,&cache)==XGE_OK);
    xge3d_model *resolved[2]={NULL,provided};
    assert(xge3dSceneLoadOwned(scene,parent,rows,2,resolved,1,cache,&root)==XGE_ERROR_INVALID_ARGUMENT);
    rows[1].model_uri=model_path;
    assert(xge3dSceneLoadOwned(scene,parent,rows,2,resolved,2,cache,&root)==XGE_ERROR_INVALID_ARGUMENT);
    rows[1].model_uri=NULL;
    rows[0].model_uri="missing-scene-model.gltf";
    assert(xge3dSceneLoadOwned(scene,parent,rows,2,resolved,2,cache,&root)!=XGE_OK && !root.slot);
    assert(xge3dSceneNodeCount(scene)==1);
    xrtClearError();

    rows[0].model_uri=model_path;
    assert(xge3dSceneLoadOwned(scene,parent,rows,2,resolved,2,cache,&root)==XGE_OK);
    assert(xge3dNodeFind(scene,root,"child",&child)==XGE_OK);
    xge3d_node_t source_node;
    assert(xge3dModelInstanceNode(scene,child,0,&source_node)==XGE_OK);
    assert(xge3dModelReleaseChecked(provided)==XGE_OK);
    bool consumed=false;
    assert(xge3dCacheReleaseOwnerChecked(cache,&consumed)==XGE_OK && consumed);
    assert(xge3dModelInstanceNode(scene,child,0,&source_node)==XGE_OK);
    assert(xge3dNodeDestroy(scene,root)==XGE_OK && xge3dSceneNodeCount(scene)==1);
    assert(xge3dSceneReleaseChecked(scene)==XGE_OK);
    printf("SCENE_LOAD3D_NATIVE_OK faults=%u successes=%u\n",faults,successes);
}

static int32 wrong_thread(void *raw)
{
    bool consumed=true;
    xge3d_retirement_info_t info={.size=sizeof(info)};
    assert(xge3dModelRetireInfo(raw,&info)==XGE_ERROR_INVALID_STATE);
    assert(xge3dModelRetireReleaseOwnerChecked(raw,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    return 0;
}

static int frame(void *raw)
{
    (void)raw;
    assert(xgeBegin()==XGE_OK);
    xge3d_model *model=NULL;
    xge3d_scene *scene=NULL;
    xge3d_renderer *renderer=NULL;
    xge3d_target *target=NULL;
    assert(xge3dModelLoadOwned(model_path,NULL,&model)==XGE_OK);
    assert(xge3dSceneCreateOwned(&scene)==XGE_OK);
    assert(xge3dRendererCreateOwned(&renderer)==XGE_OK);
    assert(xge3dTargetCreateOwned(16,16,&target)==XGE_OK);
    xge3d_node_t root;
    assert(xge3dModelInstantiate(scene,model,no_node,&root)==XGE_OK);
    xge3d_transform_t transform=XGE3D_TRANSFORM_IDENTITY;
    transform.position.z=-2;
    assert(xge3dNodeSetTransform(scene,root,&transform)==XGE_OK);
    xge3d_camera_t camera;
    assert(xge3dCameraOrthographic(&camera,3,3,.1f,10)==XGE_OK);
    const float clear[4]={0,0,0,1};
    xge3d_render_stats_t stats;
    assert(xge3dRenderFrameTo(renderer,scene,&camera,target,clear,3,0,&stats)==XGE_OK && stats.triangles==1);
    xge3d_resource_info_t resources={.size=sizeof(resources)};
    assert(xge3dModelResourceInfo(model,&resources)==XGE_OK && resources.gpu_payload_bytes>0);

    xge3d_model_retirement *token=NULL;
    assert(xge3dModelRetireBeginOwned(model,&token)==XGE_ERROR_BUSY && !token);
    assert(xge3dNodeDestroy(scene,root)==XGE_OK);
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dModelRetireBeginOwned(model,&token)==XGE_ERROR_OUT_OF_MEMORY && !token);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(xge3dModelResourceInfo(model,&resources)==XGE_OK && resources.references==1);
    assert(xge3dModelRetireBeginOwned(model,&token)==XGE_OK);
    assert(xge3dModelResourceInfo(model,&resources)==XGE_ERROR_INVALID_STATE);
    assert(xge3dModelReleaseChecked(model)==XGE_OK);
    xthread *thread=xrtThreadCreate(wrong_thread,token,0);
    assert(thread && xrtThreadWait(thread)==XWAIT_OK);
    xrtThreadDestroy(thread);

    int done=0;
    xge3d_retirement_info_t progress={.size=sizeof(progress)};
    xge3d_release_budget_t budget={0,0};
    assert(xge3dModelRetireStep(token,&budget,&done)==XGE_OK && !done);
    assert(xge3dModelRetireInfo(token,&progress)==XGE_OK && !progress.operations);
    budget.operations=1;
    size_t prior=0,total=progress.total_operations;
    for(size_t i=0;i<=total && !done;++i){
        assert(xge3dModelRetireStep(token,&budget,&done)==XGE_OK);
        assert(xge3dModelRetireInfo(token,&progress)==XGE_OK && progress.operations-prior<=1);
        prior=progress.operations;
    }
    assert(done && prior==total && total>=2);
    bool consumed=false;
    assert(xge3dModelRetireReleaseOwnerChecked(token,&consumed)==XGE_OK && consumed);
    assert(xge3dTargetReleaseChecked(target)==XGE_OK);
    assert(xge3dRendererReleaseChecked(renderer)==XGE_OK);
    assert(xge3dSceneReleaseChecked(scene)==XGE_OK);
    xrtClearError();
    assert(xgeEnd()==XGE_OK);
    puts("MODEL_RETIRE3D_NATIVE_OK");
    return 1;
}

int main(void)
{
    xge_desc_t window={.iWidth=64,.iHeight=64,.sTitle="Managed scene load and retirement"};
    assert(xgeInit(&window)==XGE_OK);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    check_load();
    assert(xgeRun(frame,NULL)==XGE_OK);
    assert(!xgeGraphicsOwnedResourceCount() && xgeUnitChecked()==XGE_OK);
    return 0;
}
