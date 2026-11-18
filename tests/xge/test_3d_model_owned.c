#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static xge3d_scene *scene;
static const char *path="model-triangle.gltf";

typedef struct fixture_provider {
    const xge_resource_t *source;
    int loaded,freed;
} fixture_provider;

static int provide(const char *uri,void **out,int *bytes,void *user)
{
    fixture_provider *fixture=user;
    assert(!strcmp(uri,"model-fixture://triangle.gltf"));
    assert(xgeUnitChecked()==XGE_ERROR_BUSY);
    void *copy=malloc((size_t)fixture->source->iSize);
    if(!copy)return XGE_ERROR_OUT_OF_MEMORY;
    memcpy(copy,fixture->source->pData,(size_t)fixture->source->iSize);
    *out=copy;
    *bytes=fixture->source->iSize;
    ++fixture->loaded;
    return XGE_OK;
}
static void release_source(void *data,void *user)
{
    fixture_provider *fixture=user;
    ++fixture->freed;
    free(data);
}
static int wrong_thread(void *raw)
{
    xge3d_model *model=raw,*alias=model;
    bool consumed=true;
    xge3d_model_info_t info;
    const char *name="stale";
    assert(xge3dModelInfoChecked(model,&info)==XGE_ERROR_INVALID_STATE);
    assert(xge3dModelNodeNameChecked(model,0,&name)==XGE_ERROR_INVALID_STATE && !name);
    assert(xge3dModelShareOwned(model,&alias)==XGE_ERROR_INVALID_STATE && !alias);
    assert(xge3dModelReleaseOwnerChecked(model,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    return 0;
}
static xge3d_decode_budget_info_t quota(xge3d_decode_budget *budget)
{
    xge3d_decode_budget_info_t info={.size=sizeof(info)};
    assert(xge3dDecodeBudgetInfo(budget,&info)==XGE_OK);
    return info;
}
static int frame(void *unused)
{
    (void)unused;
    xge3d_renderer *renderer=NULL;
    xge3d_target *target=NULL;
    xge3d_camera_t camera;
    assert(xge3dRendererCreateOwned(&renderer)==XGE_OK);
    assert(xge3dTargetCreateOwned(32,32,&target)==XGE_OK);
    assert(xge3dCameraOrthographic(&camera,2,2,.1f,10)==XGE_OK);
    float clear[4]={0,0,0,1};
    xge3d_render_stats_t stats;
    assert(xge3dRenderFrameTo(renderer,scene,&camera,target,clear,3,1,&stats)==XGE_OK && stats.triangles==1);
    unsigned char pixels[32*32*4];
    assert(xge3dTargetReadPixels(target,pixels,sizeof(pixels),0)==XGE_OK);
    size_t center=(16*32+16)*4;
    assert(pixels[center]>220 && pixels[center+1]<16 && pixels[center+2]<16);
    assert(xge3dTargetReleaseChecked(target)==XGE_OK);
    assert(xge3dRendererReleaseChecked(renderer)==XGE_OK);
    return 1;
}

int main(void)
{
    xge_desc_t window={.iWidth=64,.iHeight=64,.sTitle="Managed XGE 3D model"};
    assert(xgeInit(&window)==XGE_OK);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xge_resource_t source={0};
    assert(xgeResourceLoad(path,&source)==XGE_OK && source.iSize>0);
    xge3d_model *model=NULL;
    assert(xge3dModelLoadOwned(path,NULL,&model)==XGE_OK);
    assert(xge3dModelReleaseChecked(model)==XGE_OK);
    fixture_provider fixture={.source=&source};
    xge_resource_provider_t provider={.sScheme="model-fixture",.load=provide,.free=release_source,.pUser=&fixture};
    assert(xgeResourceProviderAdd(&provider)==XGE_OK);
    assert(xge3dModelLoadOwned("model-fixture://triangle.gltf",NULL,&model)==XGE_OK);
    assert(fixture.loaded==1 && fixture.freed==1 && xgeResourceProviderClearChecked()==XGE_OK);
    assert(xge3dModelReleaseChecked(model)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==0);

    xge3d_decode_budget *budget=NULL;
    assert(xge3dDecodeBudgetCreate(1,&budget)==XGE_OK);
    assert(xge3dModelLoadMemoryOwned(source.pData,(size_t)source.iSize,NULL,budget,&model)==XGE_ERROR_OUT_OF_MEMORY && !model);
    assert(quota(budget).live_bytes==0 && quota(budget).rejections>0 && xgeGraphicsOwnedResourceCount()==0);
    xrtClearError();
    assert(xge3dDecodeBudgetSetLimit(budget,1024*1024)==XGE_OK);
    assert(xge3dModelLoadMemoryOwned(source.pData,(size_t)source.iSize,NULL,budget,&model)==XGE_OK);
    xge3d_model_info_t info;
    const char *name=NULL;
    assert(xge3dModelInfoChecked(model,&info)==XGE_OK && info.node_count==2 && info.primitive_count==1);
    assert(xge3dModelNodeNameChecked(model,0,&name)==XGE_OK && !strcmp(name,"Triangle"));
    assert(xge3dModelNodeNameChecked(model,10,&name)==XGE_OK && !name);
    xge3d_asset_memory_t memory={.size=sizeof(memory)};
    assert(xge3dModelMemoryInfo(model,&memory)==XGE_OK && memory.source_bytes==(uint64_t)source.iSize);
    xge3d_resource_info_t resources={.size=sizeof(resources)};
    assert(xge3dModelResourceInfo(model,&resources)==XGE_OK && resources.vertices==3 && resources.references==1);
    xgeResourceFree(&source);
    xthread *worker=xrtThreadCreate(wrong_thread,model,0);
    assert(worker && xrtThreadWait(worker)==XWAIT_OK);
    xrtThreadDestroy(worker);

    xge3d_material *material=NULL;
    assert(xge3dModelMaterialOwned(model,0,&material)==XGE_OK && material);
    assert(xge3dMaterialReleaseChecked(material)==XGE_OK);
    assert(xge3dSceneCreateOwned(&scene)==XGE_OK);
    for(int allowance=0;allowance<12;++allowance){
        xge3d_node_t instance={0};
        assert(xrtMemDebugFailAfter(allowance));
        int status=xge3dModelInstantiate(scene,model,(xge3d_node_t){0},&instance);
        assert(xrtMemDebugFailAfter(-1));
        xrtClearError();
        if(status==XGE_OK)assert(xge3dNodeDestroy(scene,instance)==XGE_OK);
        else assert(!instance.slot);
        assert(xge3dSceneNodeCount(scene)==0);
    }
    xge3d_node_t root,child,outside;
    assert(xge3dModelInstantiate(scene,model,(xge3d_node_t){0},&root)==XGE_OK);
    assert(xge3dModelInstanceNode(scene,root,0,&child)==XGE_OK);
    assert(xge3dModelInstanceNode(scene,root,1,&outside)==XGE_ERROR_NOT_FOUND && !outside.slot);
    assert(xge3dModelReleaseChecked(model)==XGE_OK);
    assert(xge3dNodeSetParent(scene,child,(xge3d_node_t){0})==XGE_OK);
    assert(xge3dNodeDestroy(scene,root)==XGE_OK);
    assert(xge3dSceneNodeCount(scene)==1);
    assert(xgeRun(frame,NULL)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==2 && xgeUnitChecked()==XGE_ERROR_BUSY);
    assert(quota(budget).live_bytes>0);
    assert(xge3dSceneReleaseChecked(scene)==XGE_OK);
    assert(quota(budget).live_bytes==0);
    xge3dDecodeBudgetFree(budget);
    assert(xgeGraphicsOwnedResourceCount()==0 && xgeUnitChecked()==XGE_OK);
    puts("MODEL3D_NATIVE_GPU_OK");
    return 0;
}
