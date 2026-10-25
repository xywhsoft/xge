#include "xge.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"cache %d: %s\n",__LINE__,#x);return 1;}}while(0)
#if XGE3D_ENABLE_ASYNC
typedef struct gate {xmutex mutex;xcond cond;int block,entered,open,reads,frees;} gate;
static int provider(const char *uri,void **bytes,int *size,void *user)
{
    gate *g=user;if(strncmp(uri,"cache://",8))return XGE_ERROR_FILE_NOT_FOUND;
    xrtMutexLock(&g->mutex);++g->reads;g->entered=1;xrtCondSignal(&g->cond);
    while(g->block&&!g->open){xrtCondWait(&g->cond,&g->mutex);}xrtMutexUnlock(&g->mutex);
    char file[256];snprintf(file,sizeof(file),"artifacts/xge-3d/fixtures/%s",uri+8);size_t count=0;*bytes=xrtFileReadAllLimit(file,INT_MAX,&count);*size=(int)count;
    return *bytes?XGE_OK:XGE_ERROR_FILE_NOT_FOUND;
}
static void release_bytes(void *bytes,void *user){gate *g=user;xrtFree(bytes);xrtMutexLock(&g->mutex);++g->frees;xrtMutexUnlock(&g->mutex);}
static int wait_enter(gate *g)
{double end=xrtTimer()+5;xrtMutexLock(&g->mutex);while(!g->entered&&xrtTimer()<end)xrtCondWaitFor(&g->cond,&g->mutex,30);int entered=g->entered;xrtMutexUnlock(&g->mutex);return entered;}
static void gate_open(gate *g){xrtMutexLock(&g->mutex);g->open=1;xrtCondSignal(&g->cond);xrtMutexUnlock(&g->mutex);}
static int wait_ready(xge3d_cache *cache,xge3d_cache_request_t request,int failed)
{double end=xrtTimer()+5;do{xge3d_upload_budget_t budget={0};xge3d_request_info_t info;if(xge3dCachePump(cache,&budget,NULL)||xge3dCacheStatus(cache,request,&info))return 0;
    if(info.state==(failed?XGE3D_REQUEST_FAILED:XGE3D_REQUEST_READY)){return 1;}xrtSleep(1);}while(xrtTimer()<end);return 0;}
typedef struct wrong_thread {xge3d_cache *cache;int stats,free;} wrong_thread;
static int32 wrong_owner(void *user){wrong_thread *w=user;xge3d_cache_stats_t stats={.size=sizeof(stats)};w->stats=xge3dCacheStats(w->cache,&stats);w->free=xge3dCacheFree(w->cache);return 0;}
#endif
int main(void)
{
    xge3d_cache *cache=NULL;CHECK(!xge3dCacheCreate(NULL,&cache));xge3d_model *a=NULL,*same=NULL,*b=NULL;
    CHECK(xge3dCacheLoad(cache,"unknown://asset",&a)==XGE_ERROR_UNSUPPORTED&&!a);
    CHECK(!xge3dCacheLoad(cache,"artifacts/xge-3d/fixtures/embedded.gltf",&a));
    CHECK(!xge3dCacheLoad(cache,"artifacts/xge-3d/fixtures/../fixtures/embedded.gltf",&same)&&a==same);xge3dModelFree(same);
    CHECK(!xge3dCacheLoad(cache,"artifacts/xge-3d/fixtures/external.gltf",&b)&&a!=b);
    xge3d_material_desc_t ma,mb;CHECK(!xge3dMaterialGetDesc(xge3dModelMaterial(a,0),&ma)&&!xge3dMaterialGetDesc(xge3dModelMaterial(b,0),&mb)&&ma.maps[0].texture==mb.maps[0].texture);
    xge3d_cache_stats_t stats={.size=sizeof(stats)};CHECK(!xge3dCacheStats(cache,&stats)&&stats.models==2&&stats.textures==1&&stats.hits==1&&stats.shared_textures>=1);
    xge3d_asset_memory_t ai={.size=sizeof(ai)},bi={.size=sizeof(bi)};CHECK(!xge3dModelMemoryInfo(a,&ai)&&!xge3dModelMemoryInfo(b,&bi));
    CHECK(stats.gpu_required_bytes==ai.gpu_required_bytes+bi.gpu_required_bytes-20&&stats.cpu_storage_bytes<ai.cpu_storage_bytes+bi.cpu_storage_bytes);
    CHECK(!xge3dCacheClear(cache)&&!xge3dCacheStats(cache,&stats)&&stats.models==2);xge3dModelFree(a);xge3dModelFree(b);
    CHECK(!xge3dCacheClear(cache)&&!xge3dCacheStats(cache,&stats)&&!stats.models&&!stats.textures&&!stats.cpu_storage_bytes&&!stats.gpu_required_bytes);
    CHECK(!xge3dCacheFree(cache));xge3d_cache_desc_t desc=xge3dCacheDefault();desc.max_models=1;CHECK(!xge3dCacheCreate(&desc,&cache));
    CHECK(!xge3dCacheLoad(cache,"artifacts/xge-3d/fixtures/embedded.gltf",&a));xge3dModelFree(a);
    CHECK(!xge3dCacheLoad(cache,"artifacts/xge-3d/fixtures/external.gltf",&b)&&!xge3dCacheStats(cache,&stats)&&stats.models==1&&stats.evictions==1);xge3dModelFree(b);CHECK(!xge3dCacheFree(cache));
    desc=xge3dCacheDefault();desc.cpu_bytes=1;CHECK(!xge3dCacheCreate(&desc,&cache)&&!xge3dCacheLoad(cache,"artifacts/xge-3d/fixtures/embedded.gltf",&a));
    CHECK(!xge3dCacheStats(cache,&stats)&&stats.over_budget);xge3dModelFree(a);CHECK(!xge3dCacheTrim(cache)&&!xge3dCacheStats(cache,&stats)&&!stats.models&&!stats.over_budget);CHECK(!xge3dCacheFree(cache));
    xge3d_decode_budget *quota=NULL;CHECK(!xge3dDecodeBudgetCreate(1,&quota));desc=xge3dCacheDefault();desc.decode_budget=quota;
    CHECK(!xge3dCacheCreate(&desc,&cache));a=(xge3d_model*)1;CHECK(xge3dCacheLoad(cache,"artifacts/xge-3d/fixtures/embedded.gltf",&a)==XGE_ERROR_OUT_OF_MEMORY&&!a);
    CHECK(!xge3dCacheFree(cache));xge3d_decode_budget_info_t info={.size=sizeof(info)};CHECK(!xge3dDecodeBudgetInfo(quota,&info)&&!info.live_bytes);xge3dDecodeBudgetFree(quota);
    CHECK(!xge3dDecodeBudgetCreate(8*1024*1024,&quota));desc=xge3dCacheDefault();desc.decode_budget=quota;
    CHECK(!xge3dCacheCreate(&desc,&cache)&&!xge3dCacheLoad(cache,"artifacts/xge-3d/fixtures/embedded.gltf",&a)&&!xge3dCacheLoad(cache,"artifacts/xge-3d/fixtures/external.gltf",&b));
    CHECK(!xge3dCacheFree(cache)&&!xge3dDecodeBudgetInfo(quota,&info)&&info.live_bytes>0);xge3dModelFree(a);xge3dModelFree(b);
    CHECK(!xge3dDecodeBudgetInfo(quota,&info)&&!info.live_bytes&&!info.live_allocations);xge3dDecodeBudgetFree(quota);
#if XGE3D_ENABLE_ASYNC
    gate g={0};CHECK(xrtMutexInit(&g.mutex)&&xrtCondInit(&g.cond));xge_resource_provider_t p={.sScheme="cache",.load=provider,.free=release_bytes,.pUser=&g};CHECK(!xgeResourceProviderAdd(&p));desc=xge3dCacheDefault();desc.flags=XGE3D_CACHE_RESOURCE_PROVIDERS;
    CHECK(!xge3dCacheCreate(&desc,&cache));wrong_thread wrong={cache,0,0};xthread *thread=xrtThreadCreate(wrong_owner,&wrong,0);CHECK(thread&&xrtThreadWait(thread)==XWAIT_OK);xrtThreadDestroy(thread);
    CHECK(wrong.stats==XGE_ERROR_INVALID_STATE&&wrong.free==XGE_ERROR_INVALID_STATE);
    xge3d_cache_request_t r1,r2,r3;g.block=1;
    CHECK(!xge3dCacheRequest(cache,"cache://motion-linear.gltf",&r1)&&wait_enter(&g)&&!xge3dCacheRequest(cache,"cache://motion-linear.gltf",&r2));
    CHECK(!xge3dCacheCancel(cache,r1));gate_open(&g);CHECK(wait_ready(cache,r2,0)&&!xge3dCacheTake(cache,r2,&a));
    CHECK(!xge3dCacheRequest(cache,"cache://motion-linear.gltf",&r3)&&!xge3dCacheTake(cache,r3,&b)&&a==b);
    xge3d_request_info_t ri;CHECK(!xge3dCacheStatus(cache,r1,&ri)&&ri.state==XGE3D_REQUEST_CANCELLED&&xge3dCacheStatus(cache,r2,&ri)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(!xge3dCacheRelease(cache,r1)&&!xge3dCacheStats(cache,&stats)&&stats.coalesced==1&&stats.models==1&&g.reads==1);
    xge3dModelFree(a);xge3dModelFree(b);
    CHECK(!xge3dCacheRequest(cache,"cache://missing.gltf",&r1)&&wait_ready(cache,r1,1));
    CHECK(!xge3dCacheRequest(cache,"cache://missing.gltf",&r2)&&wait_ready(cache,r2,1));
    CHECK(!xge3dCacheStatus(cache,r1,&ri)&&ri.state==XGE3D_REQUEST_FAILED&&!xge3dCacheRelease(cache,r1)&&!xge3dCacheRelease(cache,r2));
    /* Last cancellation keeps cleanup observable until the provider returns. */
    xrtMutexLock(&g.mutex);g.entered=g.open=0;g.block=1;xrtMutexUnlock(&g.mutex);
    CHECK(!xge3dCacheRequest(cache,"cache://motion-step.gltf",&r1)&&wait_enter(&g)&&!xge3dCacheCancel(cache,r1));
    CHECK(!xge3dCacheStatus(cache,r1,&ri)&&ri.state==XGE3D_REQUEST_CANCELLED&&ri.pending_cleanup);
    CHECK(!xge3dCacheRequest(cache,"cache://motion-step.gltf",&r3));gate_open(&g);
    double end=xrtTimer()+5;do{CHECK(!xge3dCacheStatus(cache,r1,&ri));if(!ri.pending_cleanup)break;xrtSleep(1);}while(xrtTimer()<end);
    CHECK(!ri.pending_cleanup&&!xge3dCacheRelease(cache,r1)&&wait_ready(cache,r3,0)&&!xge3dCacheTake(cache,r3,&a));xge3dModelFree(a);
    for(int i=0;i<200;++i){CHECK(!xge3dCacheRequest(cache,"cache://motion-linear.gltf",&r1)&&!xge3dCacheRelease(cache,r1));}
    CHECK(!xge3dCacheClear(cache)&&!xge3dCacheFree(cache));CHECK(g.frees==g.reads-2);xgeResourceProviderClear();
    xrtCondUnit(&g.cond);xrtMutexUnit(&g.mutex);
#endif
    puts("Cache: canonical URI hits, model/texture sharing, unique byte accounting, pinned refs, LRU/quota, shared async cancellation/failure retry/stale handles and owner thread passed");return 0;
}
