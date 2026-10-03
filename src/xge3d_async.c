#include "xge3d_internal.h"
#include <limits.h>
#if XGE3D_ENABLE_ASYNC
typedef struct d3_request {
    xge3d_request_t handle;xge3d_request_state_t state;int result,running,released;
    char *uri;xge3d_model *model;d3_upload_cursor upload;xatomic32 cancelled;
    struct xge3d_loader *loader;
} d3_request;
struct xge3d_loader {
    uint64_t id,owner;size_t capacity;uint32_t flags;int stopping;
    xmutex mutex;xcond condition;xthread *worker;xslotmap slots;xarray requests;
};
static xatomic64 d3_loader_ids;
static int d3_owner(const xge3d_loader *l)
{return l && l->owner==xrtThreadCurrentId() && !l->stopping;}
static d3_request *d3_request_get(xge3d_loader *l,xge3d_request_t h)
{return h.loader==l->id ? xrtSlotMapGet(&l->slots,h.slot) : NULL;}
static void d3_request_remove(xge3d_loader *l,d3_request *r)
{
    for (size_t i=0;i<l->requests.Count;++i) if (*(d3_request**)xrtArrayGet(&l->requests,i)==r) {xrtArrayRemove(&l->requests,i,1);break;}
}
static void d3_request_free(d3_request *r)
{xge3dModelFree(r->model);xrtFree(r->uri);xrtFree(r);}
/* Only the owner changes the array. Released CPU work keeps its private job
 * alive until cleanup ends; a later Request/Pump collects the empty job. */
static void d3_collect(xge3d_loader *l)
{
    for (size_t i=0;i<l->requests.Count;) {
        d3_request *r=*(d3_request**)xrtArrayGet(&l->requests,i);
        if (r->released && !r->running) {xrtArrayRemove(&l->requests,i,1);d3_request_free(r);}
        else ++i;
    }
}
static int d3_request_cancelled(void *user)
{return xrtAtomic32Load(&((d3_request*)user)->cancelled,XMEMORY_ACQUIRE)!=0;}
static int d3_request_read(const char *uri,xge_resource_t *out,void *user)
{
    d3_request *r=user;if (d3_request_cancelled(r)) return XGE_ERROR;
    if (r->loader->flags&XGE3D_LOADER_RESOURCE_PROVIDERS) return xgeResourceLoad(uri,out);
    if (!strncmp(uri,"file://",7)) uri+=7;else if (strstr(uri,"://")) return XGE_ERROR_UNSUPPORTED;
    size_t size=0;memset(out,0,sizeof(*out));out->pData=xrtFileReadAllLimit(uri,INT_MAX,&size);
    if (!out->pData) {
        xerrkind kind=xrtErrorKind(xrtGetError());
        return kind==XERR_MEMORY ? XGE_ERROR_OUT_OF_MEMORY : kind==XERR_NOT_FOUND ? XGE_ERROR_FILE_NOT_FOUND : XGE_ERROR_RESOURCE_FAILED;
    }
    out->iSize=(int)size;return XGE_OK;
}
static int32 d3_loader_worker(void *user)
{
    xge3d_loader *l=user;
    for (;;) {
        xrtMutexLock(&l->mutex);d3_request *r=NULL;
        while (!l->stopping && !r) {
            for (size_t i=0;i<l->requests.Count;++i) {
                d3_request *p=*(d3_request**)xrtArrayGet(&l->requests,i);
                if (p->state==XGE3D_REQUEST_QUEUED) {r=p;break;}
            }
            if (!r) xrtCondWait(&l->condition,&l->mutex);
        }
        if (l->stopping) {xrtMutexUnlock(&l->mutex);break;}
        r->running=1;r->state=XGE3D_REQUEST_LOADING;xrtMutexUnlock(&l->mutex);
        d3_model_io io={d3_request_read,d3_request_cancelled,r};xge3d_model *model=NULL;
        int result=d3_model_load_io(r->uri,&io,&model);
        if (d3_request_cancelled(r)) {xge3dModelFree(model);model=NULL;}
        xrtMutexLock(&l->mutex);
        if (d3_request_cancelled(r)) {
            /* Keep running set until even a late cancellation's CPU cleanup
             * ends. Release cannot free this request underneath the worker. */
            xrtMutexUnlock(&l->mutex);xge3dModelFree(model);model=NULL;xrtMutexLock(&l->mutex);
            r->state=XGE3D_REQUEST_CANCELLED;r->result=XGE_ERROR;
        } else {
            r->result=result;r->model=model;model=NULL;
            r->state=result==XGE_OK ? XGE3D_REQUEST_CPU_READY : XGE3D_REQUEST_FAILED;
            if (r->model) r->upload.total=d3_model_upload_bytes(r->model);
        }
        r->running=0;xrtMutexUnlock(&l->mutex);
    }
    return 0;
}
int xge3dLoaderCreate(const xge3d_loader_desc_t *desc,xge3d_loader **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;xge3d_loader_desc_t d=desc ? *desc : (xge3d_loader_desc_t){0};
    if (d.max_requests>1024 || (d.flags&~XGE3D_LOADER_RESOURCE_PROVIDERS)) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_loader *l=xrtMalloc(sizeof(*l));if (!l) return XGE_ERROR_OUT_OF_MEMORY;
    memset(l,0,sizeof(*l));l->capacity=d.max_requests ? d.max_requests : 64;l->flags=d.flags;l->owner=xrtThreadCurrentId();
    l->id=xrtAtomic64FetchAdd(&d3_loader_ids,1,XMEMORY_RELAXED)+1;
    if (!l->id) {xrtFree(l);return XGE_ERROR_INVALID_STATE;}
    xrtSlotMapInit(&l->slots);xrtArrayInit(&l->requests,sizeof(d3_request*));
    if (!xrtMutexInit(&l->mutex)) {xrtFree(l);return XGE_ERROR_OUT_OF_MEMORY;}
    if (!xrtCondInit(&l->condition)) {xrtMutexUnit(&l->mutex);xrtFree(l);return XGE_ERROR_OUT_OF_MEMORY;}
    l->worker=xrtThreadCreate(d3_loader_worker,l,0);
    if (!l->worker) {xrtCondUnit(&l->condition);xrtMutexUnit(&l->mutex);xrtFree(l);return XGE_ERROR_OUT_OF_MEMORY;}
    *out=l;return XGE_OK;
}
int xge3dLoaderFree(xge3d_loader *l)
{
    if (!l) return XGE_OK;
    if (!d3_owner(l)) return XGE_ERROR_INVALID_STATE;
    xrtMutexLock(&l->mutex);l->stopping=1;
    for (size_t i=0;i<l->requests.Count;++i) xrtAtomic32Store(&(*(d3_request**)xrtArrayGet(&l->requests,i))->cancelled,1,XMEMORY_RELEASE);
    xrtCondSignal(&l->condition);xrtMutexUnlock(&l->mutex);
    xrtThreadWait(l->worker);xrtThreadDestroy(l->worker);
    for (size_t i=0;i<l->requests.Count;++i) d3_request_free(*(d3_request**)xrtArrayGet(&l->requests,i));
    xrtArrayUnit(&l->requests);xrtSlotMapUnit(&l->slots);xrtCondUnit(&l->condition);xrtMutexUnit(&l->mutex);xrtFree(l);return XGE_OK;
}
int xge3dLoaderRequest(xge3d_loader *l,const char *uri,xge3d_request_t *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=(xge3d_request_t){0};if (!d3_owner(l)) return XGE_ERROR_INVALID_STATE;
    if (!uri || !*uri) return XGE_ERROR_INVALID_ARGUMENT;
    if (!(l->flags&XGE3D_LOADER_RESOURCE_PROVIDERS) && strstr(uri,"://") && strncmp(uri,"file://",7)) return XGE_ERROR_UNSUPPORTED;
    size_t size=strlen(uri);if (size>INT_MAX) return XGE_ERROR_INVALID_ARGUMENT;
    d3_request *r=xrtMalloc(sizeof(*r));if (!r) return XGE_ERROR_OUT_OF_MEMORY;
    memset(r,0,sizeof(*r));r->uri=xrtMalloc(size+1);if (!r->uri) {xrtFree(r);return XGE_ERROR_OUT_OF_MEMORY;}
    memcpy(r->uri,uri,size+1);r->loader=l;r->state=XGE3D_REQUEST_QUEUED;xrtAtomic32Init(&r->cancelled,0);
    xrtMutexLock(&l->mutex);d3_collect(l);int result=XGE_OK;
    if (l->requests.Count>=l->capacity) result=XGE_ERROR_OUT_OF_MEMORY;
    else {
        r->handle=(xge3d_request_t){xrtSlotMapInsert(&l->slots,r),l->id};
        if (!r->handle.slot || !xrtArrayPush(&l->requests,&r)) {if (r->handle.slot) xrtSlotMapRemove(&l->slots,r->handle.slot,NULL);result=XGE_ERROR_OUT_OF_MEMORY;}
    }
    if (result==XGE_OK) {*out=r->handle;xrtCondSignal(&l->condition);}
    xrtMutexUnlock(&l->mutex);if (result!=XGE_OK) d3_request_free(r);return result;
}
int xge3dLoaderStatus(xge3d_loader *l,xge3d_request_t h,xge3d_request_info_t *out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    memset(out,0,sizeof(*out));if (!d3_owner(l)) return XGE_ERROR_INVALID_STATE;
    xrtMutexLock(&l->mutex);d3_request *r=d3_request_get(l,h);
    if (r) *out=(xge3d_request_info_t){r->state,r->result,r->running,r->upload.uploaded,r->upload.total};
    xrtMutexUnlock(&l->mutex);return r ? XGE_OK : XGE_ERROR_INVALID_ARGUMENT;
}
int xge3dLoaderCancel(xge3d_loader *l,xge3d_request_t h)
{
    if (!d3_owner(l)) return XGE_ERROR_INVALID_STATE;
    xrtMutexLock(&l->mutex);d3_request *r=d3_request_get(l,h);xge3d_model *model=NULL;
    if (r) {xrtAtomic32Store(&r->cancelled,1,XMEMORY_RELEASE);r->state=XGE3D_REQUEST_CANCELLED;r->result=XGE_ERROR;model=r->model;r->model=NULL;}
    xrtMutexUnlock(&l->mutex);xge3dModelFree(model);return r ? XGE_OK : XGE_ERROR_INVALID_ARGUMENT;
}
int xge3dLoaderRelease(xge3d_loader *l,xge3d_request_t h)
{
    int result=xge3dLoaderCancel(l,h);if (result!=XGE_OK) return result;
    xrtMutexLock(&l->mutex);d3_request *r=d3_request_get(l,h);r->released=1;xrtSlotMapRemove(&l->slots,h.slot,NULL);
    int discard=!r->running;if (discard) d3_request_remove(l,r);
    xrtMutexUnlock(&l->mutex);if (discard) d3_request_free(r);return XGE_OK;
}
int xge3dLoaderTake(xge3d_loader *l,xge3d_request_t h,xge3d_model **out)
{
    if (!out) return XGE_ERROR_INVALID_ARGUMENT;
    *out=NULL;if (!d3_owner(l)) return XGE_ERROR_INVALID_STATE;
    xrtMutexLock(&l->mutex);d3_request *r=d3_request_get(l,h);int result=XGE_OK;
    if (!r) result=XGE_ERROR_INVALID_ARGUMENT;
    else if (r->state!=XGE3D_REQUEST_READY) result=XGE_ERROR_INVALID_STATE;
    else {*out=r->model;r->model=NULL;xrtSlotMapRemove(&l->slots,h.slot,NULL);d3_request_remove(l,r);}
    xrtMutexUnlock(&l->mutex);if (result==XGE_OK) d3_request_free(r);return result;
}
int xge3dLoaderPump(xge3d_loader *l,const xge3d_upload_budget_t *budget,xge3d_upload_stats_t *out)
{
    if (out) memset(out,0,sizeof(*out));
    if (!d3_owner(l)) return XGE_ERROR_INVALID_STATE;
    if (!budget) return XGE_ERROR_INVALID_ARGUMENT;
    xge3d_upload_stats_t stats={0};uint64_t now=xrtClock(),deadline=budget->microseconds ? (UINT64_MAX-now<budget->microseconds ? UINT64_MAX : now+budget->microseconds) : 0;
    xrtMutexLock(&l->mutex);d3_collect(l);xrtMutexUnlock(&l->mutex);
    /* Only this creating thread changes the array; the worker publishes under
     * the mutex without removing jobs during a partially completed pump. */
    size_t index=0;
    for (;;) {
        xrtMutexLock(&l->mutex);d3_request *r=NULL;
        for (;index<l->requests.Count;++index) {
            d3_request *p=*(d3_request**)xrtArrayGet(&l->requests,index);
            if (p->state==XGE3D_REQUEST_CPU_READY || p->state==XGE3D_REQUEST_UPLOADING) {r=p;++index;break;}
        }
        xrtMutexUnlock(&l->mutex);if (!r) break;
        int complete=0,result=d3_model_upload_step(r->model,&r->upload,budget,&stats,deadline,&complete);
        if (result==XGE_ERROR_NOT_INITIALIZED) {if (out) *out=stats;return result;}
        xrtMutexLock(&l->mutex);
        if (result!=XGE_OK) {r->state=XGE3D_REQUEST_FAILED;r->result=result;++stats.failed;}
        else if (complete) {r->state=XGE3D_REQUEST_READY;++stats.completed;}
        else if (r->upload.context) r->state=XGE3D_REQUEST_UPLOADING;
        xge3d_model *failed=result!=XGE_OK ? r->model : NULL;if (failed) r->model=NULL;
        xrtMutexUnlock(&l->mutex);xge3dModelFree(failed);
    }
    if (out) *out=stats;
    return XGE_OK;
}
#endif
