#define XRT_MODULE_ALL
#include "xge.h"
#include "extlibs/xrt/xrt.h"
#include <assert.h>
#include <stdio.h>

static const char model_path[]="artifacts/xge-3d/fixtures/embedded.gltf";

static int32 probe_owner(void *raw)
{
    xge3d_cache *cache=raw;
    bool consumed=true;
    xge3d_cache_stats_t stats={.size=sizeof(stats)};
    assert(xge3dCacheStats(cache,&stats)==XGE_ERROR_INVALID_STATE);
    assert(xge3dCacheReleaseOwnerChecked(cache,&consumed)==XGE_ERROR_INVALID_STATE && !consumed);
    return 0;
}

static void wait_ready(xge3d_cache *cache,xge3d_cache_request_t request)
{
    const xge3d_upload_budget_t budget={.bytes=4096,.operations=4};
    double deadline=xrtTimer()+5;
    do {
        xge3d_request_info_t info;
        assert(xge3dCachePump(cache,&budget,NULL)==XGE_OK);
        assert(xge3dCacheStatus(cache,request,&info)==XGE_OK);
        if(info.state==XGE3D_REQUEST_READY)return;
        assert(info.state!=XGE3D_REQUEST_FAILED);
        xrtSleep(1);
    } while(xrtTimer()<deadline);
    assert(!"cache request did not finish");
}

typedef struct cache_fixture {
    xge3d_cache *cache;
    xge3d_cache_request_t cancelled,requested;
} cache_fixture;

static int frame(void *raw)
{
    cache_fixture *fixture=raw;
    xge3d_cache *cache=fixture->cache;
    xge3d_cache_request_t a=fixture->cancelled,b=fixture->requested;
    assert(xgeBegin()==XGE_OK);
    wait_ready(cache,b);
    xge3d_model *model=NULL,*same=NULL;
    assert(xge3dCacheTake(cache,b,&model)==XGE_OK);
    assert(xge3dCacheLoad(cache,"artifacts/xge-3d/fixtures/../fixtures/embedded.gltf",&same)==XGE_OK && model==same);
    assert(xge3dModelReleaseChecked(same)==XGE_OK);
    xge3d_request_info_t info;
    assert(xge3dCacheStatus(cache,a,&info)==XGE_OK && info.state==XGE3D_REQUEST_CANCELLED);
    assert(xge3dCacheStatus(cache,b,&info)==XGE_ERROR_INVALID_ARGUMENT);
    xrtClearError();
    assert(xge3dCacheRelease(cache,a)==XGE_OK);

    xge3d_cache_stats_t stats={.size=sizeof(stats)};
    assert(xge3dCacheClear(cache)==XGE_OK);
    assert(xge3dCacheStats(cache,&stats)==XGE_OK && stats.models==1 && stats.textures==1);
    assert(stats.hits==1 && stats.coalesced==1 && !stats.requests);
    xge3d_material *material=NULL;
    xge3d_texture *texture=NULL;
    assert(xge3dModelMaterialOwned(model,0,&material)==XGE_OK && material);
    assert(xge3dMaterialMapTextureOwned(material,XGE3D_MAP_BASE_COLOR,&texture)==XGE_OK && texture);
    xge3d_material_params_t params;
    assert(xge3dMaterialGetParams(material,&params)==XGE_OK);
    assert(xge3dMaterialSetParams(material,&params)==XGE_ERROR_INVALID_STATE);
    assert(xge3dMaterialSetMap(material,XGE3D_MAP_BASE_COLOR,NULL,NULL)==XGE_ERROR_INVALID_STATE);
    assert(xge3dMaterialSetShader(material,NULL,NULL)==XGE_ERROR_INVALID_STATE);
    const unsigned char rgba[]={255,255,255,255};
    assert(xge3dTextureUpdateRGBA(texture,0,0,1,1,rgba,sizeof(rgba),0)==XGE_ERROR_INVALID_STATE);

    bool consumed=false;
    assert(xge3dCacheReleaseOwnerChecked(cache,&consumed)==XGE_OK && consumed);
    fixture->cache=NULL;
    xge3d_model_info_t model_info;
    assert(xge3dModelInfoChecked(model,&model_info)==XGE_OK && model_info.node_count>0);
    assert(xge3dModelReleaseChecked(model)==XGE_OK);
    assert(xge3dMaterialGetParams(material,&params)==XGE_OK);
    assert(xge3dMaterialReleaseChecked(material)==XGE_OK);
    assert(xge3dTextureReleaseChecked(texture)==XGE_OK);
    assert(xgeEnd()==XGE_OK);
    return 1;
}

int main(void)
{
    xge_desc_t window={.iWidth=64,.iHeight=64,.sTitle="Managed XGE model cache"};
    assert(xgeInit(&window)==XGE_OK);
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    cache_fixture fixture={0};
    assert(xrtMemDebugFailAfter(0));
    assert(xge3dCacheCreateOwned(NULL,&fixture.cache)==XGE_ERROR_OUT_OF_MEMORY && !fixture.cache);
    assert(xrtMemDebugFailAfter(-1));
    xrtClearError();
    assert(!xgeGraphicsOwnedResourceCount());

    assert(xge3dCacheCreateOwned(NULL,&fixture.cache)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==1 && xgeUnitChecked()==XGE_ERROR_BUSY);
    xrtClearError();
    xthread *thread=xrtThreadCreate(probe_owner,fixture.cache,0);
    assert(thread && xrtThreadWait(thread)==XWAIT_OK);
    xrtThreadDestroy(thread);

    assert(xge3dCacheRequest(fixture.cache,model_path,&fixture.cancelled)==XGE_OK);
    assert(xge3dCacheRequest(fixture.cache,model_path,&fixture.requested)==XGE_OK);
    assert(xge3dCacheCancel(fixture.cache,fixture.cancelled)==XGE_OK);
    int status=xgeRun(frame,&fixture);
    if(status!=XGE_OK)fprintf(stderr,"xgeRun returned %d, cache closed=%d\n",status,!fixture.cache);
    assert(status==XGE_OK && !fixture.cache);
    assert(!xgeGraphicsOwnedResourceCount() && xgeUnitChecked()==XGE_OK);
    puts("CACHE3D_NATIVE_OK");
    return 0;
}
