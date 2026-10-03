#include "../xge.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#define CHECK(x) do { if (!(x)) {fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);return 1;} } while(0)
static int wait_state(xge3d_loader *l,xge3d_request_t h,xge3d_request_state_t state,xge3d_request_info_t *info)
{
    uint64_t end=xrtClock()+5000000;
    do {
        if (xge3dLoaderStatus(l,h,info)!=XGE_OK) return 0;
        if (info->state==state) return 1;
        if (info->state==XGE3D_REQUEST_FAILED) return 0;
        xrtSleep(1);
    } while (xrtClock()<end);
    return 0;
}
typedef struct gate {xmutex mutex;xcond cond;int block,entered,allow,loaded,freed,calls;uint64_t thread;} gate;
static int provider(const char *uri,void **bytes,int *size,void *user)
{
    gate *g=user;if (strncmp(uri,"gate://fixture/",15)) return XGE_ERROR_FILE_NOT_FOUND;
    xrtMutexLock(&g->mutex);++g->calls;g->thread=xrtThreadCurrentId();
    if (g->block && !strcmp(uri+15,"external.gltf")) {
        g->entered=1;xrtCondSignal(&g->cond);uint64_t end=xrtClock()+5000000;
        while (!g->allow && xrtClock()<end) xrtCondWaitUntil(&g->cond,&g->mutex,end);
        if (!g->allow) {xrtMutexUnlock(&g->mutex);return XGE_ERROR_RESOURCE_FAILED;}
    }
    xrtMutexUnlock(&g->mutex);
    char path[256];snprintf(path,sizeof(path),"artifacts/xge-3d/fixtures/%s",uri+15);
    size_t length=0;*bytes=xrtFileReadAllLimit(path,INT_MAX,&length);*size=(int)length;
    if (!*bytes) return XGE_ERROR_FILE_NOT_FOUND;
    xrtMutexLock(&g->mutex);++g->loaded;xrtMutexUnlock(&g->mutex);return XGE_OK;
}
static void provider_free(void *bytes,void *user)
{gate *g=user;xrtFree(bytes);xrtMutexLock(&g->mutex);++g->freed;xrtMutexUnlock(&g->mutex);}
static int gate_enter(gate *g)
{
    xrtMutexLock(&g->mutex);uint64_t end=xrtClock()+5000000;
    while (!g->entered && xrtClock()<end) xrtCondWaitUntil(&g->cond,&g->mutex,end);
    int result=g->entered;xrtMutexUnlock(&g->mutex);return result;
}
static void gate_open(gate *g)
{xrtMutexLock(&g->mutex);g->allow=1;xrtCondSignal(&g->cond);xrtMutexUnlock(&g->mutex);}
typedef struct wrong_owner {xge3d_loader *loader;xge3d_request_t request;int status,pump,free;} wrong_owner;
static int32 wrong_thread(void *user)
{
    wrong_owner *w=user;xge3d_request_info_t info;xge3d_upload_budget_t budget={16,0,1};
    w->status=xge3dLoaderStatus(w->loader,w->request,&info);
    w->pump=xge3dLoaderPump(w->loader,&budget,NULL);w->free=xge3dLoaderFree(w->loader);return 0;
}
int main(void)
{
    /* Default IO works before xgeInit and never reads mutable provider state. */
    xge3d_loader *l=(xge3d_loader*)1;
    xge3d_loader_desc_t bad={1025,0};CHECK(xge3dLoaderCreate(&bad,&l)==XGE_ERROR_INVALID_ARGUMENT && !l);
    bad=(xge3d_loader_desc_t){2,2};CHECK(xge3dLoaderCreate(&bad,&l)==XGE_ERROR_INVALID_ARGUMENT && !l);
    CHECK(xge3dLoaderCreate(NULL,&l)==XGE_OK);
    xge3d_request_t h={1,1};xge3d_request_info_t info;
    CHECK(xge3dLoaderRequest(l,"gate://fixture/external.gltf",&h)==XGE_ERROR_UNSUPPORTED && !h.slot);
    const char *files[]={"external.gltf","embedded.glb","nonindexed.gltf","sparse.gltf"};
    for (size_t i=0;i<4;++i) {
        char path[256];snprintf(path,sizeof(path),"%sartifacts/xge-3d/fixtures/%s",i==1 ? "file://" : "",files[i]);
        CHECK(xge3dLoaderRequest(l,path,&h)==XGE_OK && wait_state(l,h,XGE3D_REQUEST_CPU_READY,&info));
        CHECK(!info.pending_cleanup && !info.uploaded_bytes && info.total_bytes>=3*sizeof(xge3d_vertex_t));
        xge3d_model *model=(xge3d_model*)1;CHECK(xge3dLoaderTake(l,h,&model)==XGE_ERROR_INVALID_STATE && !model);
        xge3d_upload_budget_t budget={16,0,1};xge3d_upload_stats_t stats;
        CHECK(xge3dLoaderPump(l,&budget,&stats)==XGE_ERROR_NOT_INITIALIZED && !stats.operations && !stats.upload_bytes);
        budget.bytes=0;CHECK(xge3dLoaderPump(l,&budget,&stats)==XGE_OK && !stats.operations);
        CHECK(xge3dLoaderStatus(l,h,&info)==XGE_OK && info.state==XGE3D_REQUEST_CPU_READY);
        CHECK(xge3dLoaderCancel(l,h)==XGE_OK && xge3dLoaderCancel(l,h)==XGE_OK);
        CHECK(xge3dLoaderStatus(l,h,&info)==XGE_OK && info.state==XGE3D_REQUEST_CANCELLED && !info.pending_cleanup);
        CHECK(xge3dLoaderRelease(l,h)==XGE_OK && xge3dLoaderStatus(l,h,&info)==XGE_ERROR_INVALID_ARGUMENT);
    }
#if XGE3D_ENABLE_ANIMATION
    CHECK(xge3dLoaderRequest(l,"artifacts/xge-3d/fixtures/motion-linear.gltf",&h)==XGE_OK && wait_state(l,h,XGE3D_REQUEST_CPU_READY,&info));
    CHECK(!info.total_bytes);
    xge3d_upload_budget_t paused={0};xge3d_model *motion=NULL;
    CHECK(xge3dLoaderPump(l,&paused,NULL)==XGE_OK && xge3dLoaderTake(l,h,&motion)==XGE_OK);
    xge3d_clip *clip=NULL;CHECK(xge3dClipFromModel(motion,0,&clip)==XGE_OK);xge3dClipFree(clip);xge3dModelFree(motion);
#endif
    CHECK(xge3dLoaderRequest(l,"artifacts/xge-3d/fixtures/embedded.glb",&h)==XGE_OK);
    wrong_owner w={l,h};xthread *thread=xrtThreadCreate(wrong_thread,&w,0);CHECK(thread);
    CHECK(xrtThreadWait(thread)==XWAIT_OK);xrtThreadDestroy(thread);
    CHECK(w.status==XGE_ERROR_INVALID_STATE && w.pump==XGE_ERROR_INVALID_STATE && w.free==XGE_ERROR_INVALID_STATE);
    CHECK(xge3dLoaderFree(l)==XGE_OK);
    /* Gate the IO itself, so queued cancellation and stale-slot reuse are
     * deterministic rather than races dependent on the machine's speed. */
    gate g={0};CHECK(xrtMutexInit(&g.mutex) && xrtCondInit(&g.cond));g.block=1;
    xge_resource_provider_t p={0};p.sScheme="gate";p.load=provider;p.free=provider_free;p.pUser=&g;
    CHECK(xgeResourceProviderAdd(&p)==XGE_OK);
    xge3d_loader_desc_t desc={3,XGE3D_LOADER_RESOURCE_PROVIDERS};CHECK(xge3dLoaderCreate(&desc,&l)==XGE_OK);
    CHECK(xge3dLoaderRequest(l,"gate://fixture/external.gltf",&h)==XGE_OK && gate_enter(&g));
    CHECK(g.thread!=xrtThreadCurrentId());
    CHECK(xge3dLoaderStatus(l,h,&info)==XGE_OK && info.state==XGE3D_REQUEST_LOADING && info.pending_cleanup);
    xge3d_request_t queued,reused,over;
    CHECK(xge3dLoaderRequest(l,"gate://fixture/embedded.glb",&queued)==XGE_OK);
    CHECK(xge3dLoaderCancel(l,queued)==XGE_OK && xge3dLoaderRelease(l,queued)==XGE_OK);
    CHECK(xge3dLoaderCancel(l,h)==XGE_OK);
    CHECK(xge3dLoaderStatus(l,h,&info)==XGE_OK && info.state==XGE3D_REQUEST_CANCELLED && info.pending_cleanup);
    CHECK(xge3dLoaderRelease(l,h)==XGE_OK);
    CHECK(xge3dLoaderRequest(l,"gate://fixture/external.gltf",&reused)==XGE_OK && reused.slot!=h.slot && reused.loader==h.loader);
    CHECK(xge3dLoaderStatus(l,h,&info)==XGE_ERROR_INVALID_ARGUMENT);
    CHECK(xge3dLoaderRequest(l,"gate://fixture/extension.gltf",&queued)==XGE_OK);
    CHECK(xge3dLoaderRequest(l,"gate://fixture/embedded.glb",&over)==XGE_ERROR_OUT_OF_MEMORY && !over.slot);
    gate_open(&g);CHECK(wait_state(l,reused,XGE3D_REQUEST_CPU_READY,&info));
    CHECK(info.total_bytes==16+3*sizeof(xge3d_vertex_t) && !info.uploaded_bytes);
    uint64_t end=xrtClock()+5000000;
    do {CHECK(xge3dLoaderStatus(l,queued,&info)==XGE_OK);if (info.state==XGE3D_REQUEST_FAILED) break;xrtSleep(1);} while (xrtClock()<end);
    CHECK(info.state==XGE3D_REQUEST_FAILED && info.result==XGE_ERROR_UNSUPPORTED);
    CHECK(xge3dLoaderRelease(l,queued)==XGE_OK && xge3dLoaderRelease(l,reused)==XGE_OK);
    xrtMutexLock(&g.mutex);CHECK(g.loaded==g.freed && g.calls==5);xrtMutexUnlock(&g.mutex);
    /* Repeated reuse, external dependencies and failed reads must all clean up. */
    for (int i=0;i<100;++i) {
        CHECK(xge3dLoaderRequest(l,"gate://fixture/external.gltf",&reused)==XGE_OK && wait_state(l,reused,XGE3D_REQUEST_CPU_READY,&info));
        CHECK(xge3dLoaderRelease(l,reused)==XGE_OK);
    }
    CHECK(xge3dLoaderRequest(l,"gate://fixture/no-such-file",&h)==XGE_OK);
    end=xrtClock()+5000000;
    do {CHECK(xge3dLoaderStatus(l,h,&info)==XGE_OK);if (info.state==XGE3D_REQUEST_FAILED) break;xrtSleep(1);} while (xrtClock()<end);
    CHECK(info.state==XGE3D_REQUEST_FAILED && info.result==XGE_ERROR_FILE_NOT_FOUND);
    /* Free also joins an active provider call and frees the source on return. */
    xrtMutexLock(&g.mutex);g.entered=0;g.allow=0;xrtMutexUnlock(&g.mutex);
    CHECK(xge3dLoaderRequest(l,"gate://fixture/external.gltf",&reused)==XGE_OK && gate_enter(&g));
    gate_open(&g);CHECK(xge3dLoaderFree(l)==XGE_OK);
    CHECK(g.loaded==g.freed);xgeResourceProviderClear();xrtCondUnit(&g.cond);xrtMutexUnit(&g.mutex);
    printf("3D async CPU: private worker, provider ownership, cancellation/stale handles, capacity, wrong thread and 100 reloads passed (%d sources freed)\n",g.freed);
    return 0;
}
