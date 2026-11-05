#include "xui_config.h"
#if XUI_ENABLE_DOCUMENT_VIEW
#include "xui_internal.h"
#include "xui_document_image_resource_internal.h"
#include "xge.h"
#include <stdatomic.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define DOC_IMAGE_MAX_WORKERS 4u
#define DOC_IMAGE_MAX_REQUESTS 64u
#define DOC_IMAGE_MAX_COPIED_BYTES (UINT64_C(64)*1024*1024)

struct xui_doc_image_request_t {
    xui_owned_manager_t owned;
    xui_context context;
    char *name,*path;
    unsigned char* encoded;
    size_t encoded_bytes;
    uint64_t max_encoded,max_pixels;
    uint32_t base_generation,published_generation;
    atomic_int cancelled;
    xthread* worker;
    xge_image_t image;
    xerror* error;
    xui_surface surface; /* actual partial producer output, never a stack owner */
    int worker_result,final_result,terminal,started,busy,polling;
    struct xui_doc_image_request_t* next;
};
/* Non-owning admission/scheduling index. Context manager nodes uniquely own
 * storage. No allocation, thread API or external call runs under this lock. */
static xui_doc_image_request doc_image_requests;
static atomic_flag doc_image_index_lock=ATOMIC_FLAG_INIT;
static void doc_image_index_enter(void)
{while(atomic_flag_test_and_set_explicit(&doc_image_index_lock,memory_order_acquire))atomic_signal_fence(memory_order_seq_cst);}
static void doc_image_index_leave(void)
{atomic_flag_clear_explicit(&doc_image_index_lock,memory_order_release);}
static const xui_owned_manager_ops_t doc_image_job_ops;
static void doc_image_start_queued(xui_context);
static bool doc_image_context_alive(xui_context context)
{return xuiInternalContextIsValid(context) && !context->bCloseRequested && !context->bDestroyPending && !context->bDestroying;}
static xui_proxy doc_image_job_proxy(xui_context context)
{return context->pOwnedProxy ? context->pOwnedProxy:xuiInternalContextGetProxy(context);}
static bool doc_image_is_pumping(xui_context context)
{
    bool active=false;doc_image_index_enter();
    for(xui_doc_image_request job=doc_image_requests;job;job=job->next)
        if(job->context==context && (job->polling || job->busy)){active=true;break;}
    doc_image_index_leave();return active;
}
static char* doc_image_copy_string(const char* value)
{
    size_t bytes = strlen(value); char* copy;
    if (bytes == SIZE_MAX) return NULL;
    copy = (char*)xrtMalloc(bytes + 1);
    if (copy) memcpy(copy, value, bytes + 1);
    return copy;
}

static int32 doc_image_worker(ptr user)
{
    xui_doc_image_request job = (xui_doc_image_request)user;
    unsigned char* encoded = job->encoded; size_t bytes = job->encoded_bytes;
    int width = 0, height = 0, file_bytes = job->path != NULL;
    uint64_t pixel_limit = job->max_pixels < (uint64_t)INT_MAX / 4 ?
        job->max_pixels : (uint64_t)INT_MAX / 4;
    int result = XUI_OK;
    if (atomic_load_explicit(&job->cancelled, memory_order_acquire)) {
        job->worker_result = XUI_DOC_ERROR_CANCELLED; return 0;
    }
    if (file_bytes) encoded = xrtFileReadAllLimit(job->path, (size_t)job->max_encoded, &bytes);
    if (!encoded) {
        result = xrtErrorFind(xrtGetError(), "xrt.file", XFILE_ERROR_LIMIT) ?
            XUI_DOC_ERROR_LIMIT : XUI_DOC_ERROR_IO;
    } else if (atomic_load_explicit(&job->cancelled, memory_order_acquire)) {
        result = XUI_DOC_ERROR_CANCELLED;
    } else if (!bytes || xgeImageInfoMemory(encoded, (int)bytes, &width, &height) != XGE_OK) {
        result = XUI_DOC_ERROR_FORMAT;
    } else if ((uint64_t)width > pixel_limit / (uint64_t)height) {
        result = XUI_DOC_ERROR_LIMIT;
    } else if (xgeImageLoadMemoryEx(&job->image, encoded, (int)bytes, XGE_IMAGE_PREMULTIPLIED) != XGE_OK) {
        result = XUI_DOC_ERROR_FORMAT;
    } else if (job->image.iWidth != width || job->image.iHeight != height ||
        job->image.iFormat != XGE_PIXEL_RGBA8 || !job->image.pPixels) {
        result = XUI_ERROR_BACKEND_FAILED;
    }
    if (file_bytes) xrtFree(encoded);
    job->worker_result = result;job->error=xrtTakeError();
    return 0;
}


static int doc_image_admission(xui_context context,const char* name,uint64_t bytes)
{
    uint64_t copied=bytes;unsigned count=0;int status=XUI_OK;
    if(copied>DOC_IMAGE_MAX_COPIED_BYTES)return XUI_DOC_ERROR_LIMIT;
    doc_image_index_enter();
    for(xui_doc_image_request job=doc_image_requests;job;job=job->next){
        if(job->context!=context || job->terminal)continue;
        if(!job->owned.pending && !job->started && !strcmp(job->name,name))continue;
        ++count;
        if(job->encoded_bytes>DOC_IMAGE_MAX_COPIED_BYTES-copied){status=XUI_DOC_ERROR_LIMIT;break;}
        copied+=job->encoded_bytes;
    }
    doc_image_index_leave();
    return status!=XUI_OK || count>=DOC_IMAGE_MAX_REQUESTS ? XUI_DOC_ERROR_LIMIT:XUI_OK;
}
static void doc_image_start_queued(xui_context context)
{
    while(doc_image_context_alive(context)){
        unsigned running=0;xui_doc_image_request oldest=NULL;
        doc_image_index_enter();
        for(xui_doc_image_request job=doc_image_requests;job;job=job->next){
            if(job->context!=context)continue;
            if(job->worker)++running;
            else if(!job->owned.pending && !job->terminal && !job->started)oldest=job;
        }
        if(running>=DOC_IMAGE_MAX_WORKERS || !oldest){doc_image_index_leave();break;}
        oldest->started=1;doc_image_index_leave();
        /* The creator's Context scope covers selection. Enter this actual
         * manager before the thread API and retain it through publication. */
        xerror* entry=xrtTakeError();xui_manager_scope_t call={0};
        int admission=xuiInternalManagerScopeBegin(&oldest->owned,&call);
        if(admission!=XUI_OK){
            if(xuiInternalManagerAlive(&oldest->owned) && doc_image_context_alive(context)){
                oldest->terminal=1;oldest->final_result=admission;oldest->error=xrtTakeError();
            }else xrtClearError();
            xrtSetErrorTake(entry);continue;
        }
        if(atomic_load_explicit(&oldest->cancelled,memory_order_acquire)){
            oldest->terminal=1;oldest->final_result=XUI_DOC_ERROR_CANCELLED;
            xrtFree(oldest->encoded);oldest->encoded=NULL;oldest->encoded_bytes=0;
        }else{
            xerror* ambient=xrtTakeError();
            oldest->worker=xrtThreadCreate(doc_image_worker,oldest,0);
            xerror* error=xrtTakeError();
            if(!oldest->worker){oldest->terminal=1;oldest->final_result=XUI_ERROR_OUT_OF_MEMORY;oldest->error=error;error=NULL;
                xrtFree(oldest->encoded);oldest->encoded=NULL;oldest->encoded_bytes=0;}
            xrtErrorFree(error);xrtSetErrorTake(ambient);
        }
        int ended=xuiInternalManagerScopeEnd(&call,XUI_OK);xerror* error=xrtTakeError();
        if(ended<0)xuiInternalContextRecordOutcome(context,ended,error);
        xrtErrorFree(error);xrtSetErrorTake(entry);
    }
}
static int doc_image_request_launch(xui_doc_image_request job)
{
    int status=doc_image_admission(job->context,job->name,job->encoded_bytes);if(status!=XUI_OK)return status;
    xui_resource resource=xuiResourceFind(job->context,job->name);
    job->base_generation=resource ? xuiResourceGetGeneration(resource):0;
    void* retired[DOC_IMAGE_MAX_REQUESTS];size_t count=0;
    doc_image_index_enter();
    for(xui_doc_image_request prior=doc_image_requests;prior;prior=prior->next){
        if(prior->context!=job->context || prior->terminal || prior->owned.pending || strcmp(prior->name,job->name))continue;
        atomic_store_explicit(&prior->cancelled,1,memory_order_release);
        if(!prior->started){
            prior->terminal=1;prior->final_result=XUI_DOC_ERROR_CANCELLED;
            if(prior->encoded){retired[count++]=prior->encoded;prior->encoded=NULL;prior->encoded_bytes=0;}
        }
    }
    job->next=doc_image_requests;doc_image_requests=job;doc_image_index_leave();
    for(size_t i=0;i<count;++i)xrtFree(retired[i]);
    doc_image_start_queued(job->context);return XUI_OK;
}
static int doc_image_job_surface_release(xui_doc_image_request job)
{
    if(!job->surface)return XUI_OK;
    if(job->context->pOwnedProxy){
        bool consumed=false;int status=xuiSurfaceReleaseOwnerChecked(job->surface,&consumed);
        if(consumed)job->surface=NULL;
        return status;
    }
    xui_proxy proxy=doc_image_job_proxy(job->context);proxy->surfaceDestroy(proxy,job->surface);job->surface=NULL;
    return xrtGetError() ? XUI_ERROR_CALLBACK_FAILED:XUI_OK;
}
static bool doc_image_job_trace(const void* raw,xrtownershipvisitor visit,void* user)
{
    const struct xui_doc_image_request_t* job=raw;
    if(job->busy)return false;
    return !job->surface || !job->context->pOwnedProxy || visit(xuiSurfaceOwnership(job->surface),user);
}
static int doc_image_job_ready(const void* raw)
{
    const struct xui_doc_image_request_t* job=raw;
    return job->busy || job->polling || (job->worker && xrtThreadState(job->worker)!=XTHREAD_FINISHED) ? XUI_ERROR_BUSY:XUI_OK;
}
static int doc_image_job_prepare(void* raw,bool* complete)
{
    xui_doc_image_request job=raw;*complete=false;
    if(job->busy || job->polling)return XUI_STATUS_DEFERRED;
    atomic_store_explicit(&job->cancelled,1,memory_order_release);
    if(job->worker){
        if(xrtThreadState(job->worker)!=XTHREAD_FINISHED)return XUI_STATUS_DEFERRED;
        xwaitresult wait=xrtThreadWaitFor(job->worker,0);
        if(wait==XWAIT_TIMEOUT)return XUI_STATUS_DEFERRED;
        if(wait!=XWAIT_OK)return XUI_ERROR_CALLBACK_FAILED;
        xrtThreadDestroy(job->worker);job->worker=NULL;
    }
    int status=XUI_OK;xerror* first=NULL;
    xuiInternalContextCleanupOutcome(job->context,doc_image_job_surface_release(job),&status,&first);
    if(job->surface){xrtSetErrorTake(first);return status==XUI_OK ? XUI_STATUS_DEFERRED:status;}
    xgeImageFree(&job->image);xrtFree(job->encoded);job->encoded=NULL;job->encoded_bytes=0;
    job->terminal=1;*complete=true;xrtSetErrorTake(first);return status;
}
static void doc_image_job_close(void* raw)
{atomic_store_explicit(&((xui_doc_image_request)raw)->cancelled,1,memory_order_release);}
static int doc_image_job_finalize(void* raw)
{
    xui_doc_image_request job=raw;xui_context context=job->context;
    doc_image_index_enter();xui_doc_image_request* slot=&doc_image_requests;
    while(*slot && *slot!=job)slot=&(*slot)->next;
    if(*slot)*slot=job->next;
    doc_image_index_leave();
    xrtErrorFree(job->error);xrtFree(job->path);xrtFree(job->name);xrtFree(job);
    if(doc_image_context_alive(context) && !doc_image_is_pumping(context))doc_image_start_queued(context);
    return XUI_OK;
}
static const xui_owned_manager_ops_t doc_image_job_ops={doc_image_job_trace,doc_image_job_ready,doc_image_job_prepare,doc_image_job_close,doc_image_job_finalize};
static void doc_image_job_gone(void* raw){*(bool*)raw=false;}
static int doc_image_construct_end(xui_call_scope_t* scope,int status,xui_doc_image_request job,
    xui_lifetime_link_t* watch,bool* alive,xui_doc_image_request* out)
{
    xerror* first=xrtTakeError();if(status==XUI_OK && first)status=XUI_ERROR_CALLBACK_FAILED;
    if(status==XUI_OK && (!*alive || !doc_image_context_alive(job->context)))status=XUI_ERROR_INVALID_STATE;
    if(status!=XUI_OK && *alive){
        xui_cleanup_prefix_t prefix={0};xuiInternalCleanupPrefixBegin(job->context,&prefix,status,first);
        (void)xuiInternalManagerRequestClose(&job->owned);xrtClearError();xuiInternalCleanupPrefixEnd(&prefix);
    }
    status=xuiInternalCallScopeEndOutcome(scope,status,&first);xuiLifetimeUnwatch(watch);
    if(status==XUI_OK && !*alive)status=XUI_ERROR_INVALID_STATE;
    if(status==XUI_OK)*out=job;
    xrtSetErrorTake(first);return status;
}
static int doc_image_create_request(xui_context context,const char* name,const char* path,
    const void* encoded,uint64_t bytes,const xui_doc_image_load_limits_t* limits,xui_doc_image_request* out)
{
    if(out)*out=NULL;
    if(!out || !name || !*name || (path ? !*path:(!encoded || !bytes)))return XUI_ERROR_INVALID_ARGUMENT;
    uint64_t max_encoded,max_pixels;int status=doc_image_limits(limits,&max_encoded,&max_pixels);if(status!=XUI_OK)return status;
    if(bytes>max_encoded)return XUI_DOC_ERROR_LIMIT;
    xui_call_scope_t scope={0};status=xuiCallScopeBegin(context,&scope);if(status!=XUI_OK)return status;
    xui_proxy proxy=doc_image_job_proxy(context);
    if(!proxy || !proxy->surfaceCreateRGBA || !proxy->surfaceDestroy)status=XUI_ERROR_NOT_INITIALIZED;
    if(status==XUI_OK && doc_image_is_pumping(context))status=XUI_DOC_ERROR_BUSY;
    if(status==XUI_OK)status=doc_image_admission(context,name,bytes);
    if(status!=XUI_OK){xerror* first=xrtTakeError();status=xuiInternalCallScopeEndOutcome(&scope,status,&first);xrtSetErrorTake(first);return status;}
    xui_doc_image_request job=xrtCalloc(1,sizeof(*job));
    if(!job){xerror* first=xrtTakeError();status=xuiInternalCallScopeEndOutcome(&scope,XUI_ERROR_OUT_OF_MEMORY,&first);xrtSetErrorTake(first);return status;}
    job->context=context;job->max_encoded=max_encoded;job->max_pixels=max_pixels;atomic_init(&job->cancelled,0);
    status=xuiInternalManagerRegister(context,&job->owned,&doc_image_job_ops,job);
    if(status!=XUI_OK){xrtFree(job);xerror* first=xrtTakeError();status=xuiInternalCallScopeEndOutcome(&scope,status,&first);xrtSetErrorTake(first);return status;}
    bool alive=true;xui_lifetime_link_t watch={0};if(xuiInternalManagerWatch(&job->owned,&watch,doc_image_job_gone,&alive)!=XUI_OK)abort();
    job->name=doc_image_copy_string(name);
    if(path)job->path=doc_image_copy_string(path);
    else{job->encoded=xrtMalloc((size_t)bytes);if(job->encoded){memcpy(job->encoded,encoded,(size_t)bytes);job->encoded_bytes=(size_t)bytes;}}
    if(!job->name || (path ? !job->path:!job->encoded))status=XUI_ERROR_OUT_OF_MEMORY;
    if(status==XUI_OK)status=doc_image_request_launch(job);
    return doc_image_construct_end(&scope,status,job,&watch,&alive,out);
}
XUI_API int xuiDocumentImageResourceLoadFileAsync(xui_context context,const char* name,const char* path,
    const xui_doc_image_load_limits_t* limits,xui_doc_image_request* out)
{return doc_image_create_request(context,name,path,NULL,0,limits,out);}
XUI_API int xuiDocumentImageResourceLoadMemoryAsync(xui_context context,const char* name,const void* encoded,uint64_t bytes,
    const xui_doc_image_load_limits_t* limits,xui_doc_image_request* out)
{return doc_image_create_request(context,name,NULL,encoded,bytes,limits,out);}

static bool doc_image_finish(xui_doc_image_request job)
{
    xwaitresult wait=xrtThreadWaitFor(job->worker,0);if(wait!=XWAIT_OK)return false;
    xrtThreadDestroy(job->worker);job->worker=NULL;job->busy=1;
    int result=job->worker_result;xerror* first=job->error;job->error=NULL;
    if(atomic_load_explicit(&job->cancelled,memory_order_acquire)){result=XUI_DOC_ERROR_CANCELLED;xrtErrorFree(first);first=NULL;}
    if(result==XUI_OK && first)result=XUI_ERROR_CALLBACK_FAILED;
    if(result==XUI_OK){
        xui_resource current=xuiResourceFind(job->context,job->name);
        if((current ? xuiResourceGetGeneration(current):0)!=job->base_generation)result=XUI_DOC_ERROR_STALE;
    }
    if(result==XUI_OK){
        xui_proxy proxy=doc_image_job_proxy(job->context);
        if(!proxy || !proxy->surfaceCreateRGBA)result=XUI_ERROR_NOT_INITIALIZED;
        else result=proxy->surfaceCreateRGBA(proxy,&job->surface,job->image.iWidth,job->image.iHeight,
            job->image.pPixels,job->image.iStride,XUI_SURFACE_ALPHA_PREMULTIPLIED);
        if(result==XUI_OK && (!xuiInternalManagerAlive(&job->owned) || !doc_image_context_alive(job->context)))result=XUI_ERROR_INVALID_STATE;
        if(result==XUI_OK && (!job->surface || (job->context->pOwnedProxy && xuiSurfaceGetProxy(job->surface)!=proxy)))result=XUI_ERROR_INVALID_ARGUMENT;
        if(result==XUI_OK){
            result=doc_image_resource_register_surface(job->context,job->name,job->surface,NULL);job->surface=NULL;
            if(result==XUI_OK && xuiInternalManagerAlive(&job->owned) && doc_image_context_alive(job->context)){
                xui_resource current=xuiResourceFind(job->context,job->name);job->published_generation=current ? xuiResourceGetGeneration(current):0;
            }else if(result==XUI_OK)result=XUI_ERROR_INVALID_STATE;
        }
    }
    xerror* error=xrtTakeError();
    if(result==XUI_OK && error){result=XUI_ERROR_CALLBACK_FAILED;first=error;error=NULL;}
    else if(result!=XUI_OK && !first){first=error;error=NULL;}
    xrtErrorFree(error);
    xui_cleanup_prefix_t prefix={0};xuiInternalCleanupPrefixBegin(job->context,&prefix,result,first);
    xuiInternalContextCleanupOutcome(job->context,doc_image_job_surface_release(job),&result,&first);
    xuiInternalCleanupPrefixEnd(&prefix);
    xgeImageFree(&job->image);xrtFree(job->encoded);job->encoded=NULL;job->encoded_bytes=0;
    job->terminal=1;job->final_result=result;job->error=first;job->busy=0;
    /* Ordinary decoder/producer failure belongs to this ticket, not another
     * ticket whose Poll happened to reap it. Actual cleanup already has a
     * Context receipt, including its exact failed-with-NULL bit. */
    return true;
}
static void doc_image_pump(xui_context context)
{
    for(unsigned pass=0;pass<DOC_IMAGE_MAX_REQUESTS;++pass){
        xui_doc_image_request candidates[DOC_IMAGE_MAX_WORKERS];size_t count=0;
        doc_image_index_enter();
        for(xui_doc_image_request job=doc_image_requests;job && count<DOC_IMAGE_MAX_WORKERS;job=job->next)
            if(job->context==context && job->worker && !job->owned.pending && !job->busy)candidates[count++]=job;
        doc_image_index_leave();
        bool progress=false;
        for(size_t i=0;i<count;++i){
            xui_doc_image_request job=candidates[i];
            xerror* ambient=xrtTakeError();xui_manager_scope_t scope={0};
            if(xuiInternalManagerScopeBegin(&job->owned,&scope)==XUI_OK){
                if(xrtThreadState(job->worker)==XTHREAD_FINISHED)
                    progress=doc_image_finish(job) || progress;
                (void)xuiInternalManagerScopeEnd(&scope,XUI_OK);
            }
            xrtClearError();xrtSetErrorTake(ambient);
        }
        if(!progress)break;
    }
    doc_image_start_queued(context);
}
XUI_API int xuiDocumentImageResourcePoll(xui_doc_image_request job,xui_resource* out)
{
    if(out)*out=NULL;
    if(!job)return XUI_ERROR_INVALID_ARGUMENT;
    xui_manager_scope_t scope={0};int status=xuiInternalManagerScopeBegin(&job->owned,&scope);if(status!=XUI_OK)return status;
    if(doc_image_is_pumping(job->context))return xuiInternalManagerScopeEnd(&scope,XUI_DOC_ERROR_BUSY);
    bool alive=true;xui_lifetime_link_t watch={0};if(xuiInternalManagerWatch(&job->owned,&watch,doc_image_job_gone,&alive)!=XUI_OK)abort();
    job->polling=1;doc_image_pump(job->context);
    status=!job->terminal ? XUI_DOC_ERROR_BUSY:job->final_result;
    if(!xuiInternalManagerAlive(&job->owned) || !doc_image_context_alive(job->context))status=XUI_ERROR_INVALID_STATE;
    if(status==XUI_OK && out){
        xui_resource current=xuiResourceFind(job->context,job->name);
        if(current && xuiResourceGetGeneration(current)==job->published_generation)*out=current;
    }
    if(status!=XUI_OK && job->terminal && job->final_result==status)xrtSetErrorTake(xrtErrorRef(job->error));
    job->polling=0;status=xuiInternalManagerScopeEnd(&scope,status);xuiLifetimeUnwatch(&watch);
    if(status==XUI_OK && (!alive || !doc_image_context_alive(job->context)))status=XUI_ERROR_INVALID_STATE;
    if(status!=XUI_OK && out)*out=NULL;
    return status;
}
XUI_API int xuiDocumentImageResourceCancel(xui_doc_image_request job)
{
    if(!job)return XUI_ERROR_INVALID_ARGUMENT;
    xui_manager_scope_t scope={0};int status=xuiInternalManagerScopeBegin(&job->owned,&scope);if(status!=XUI_OK)return status;
    if(job->terminal || job->busy)status=XUI_ERROR_UNSUPPORTED;
    else{
        atomic_store_explicit(&job->cancelled,1,memory_order_release);
        if(!job->started){job->terminal=1;job->final_result=XUI_DOC_ERROR_CANCELLED;xrtFree(job->encoded);job->encoded=NULL;job->encoded_bytes=0;}
    }
    return xuiInternalManagerScopeEnd(&scope,status);
}
XUI_API int xuiDocumentImageResourceReleaseOwnerChecked(xui_doc_image_request job,bool* consumed)
{
    if(!consumed)return XUI_ERROR_INVALID_ARGUMENT;
    *consumed=false;if(!job)return XUI_ERROR_INVALID_ARGUMENT;
    if(job->owned.pending)return XUI_OK;
    xui_manager_scope_t scope={0};int status=xuiInternalManagerScopeBegin(&job->owned,&scope);if(status!=XUI_OK)return status;
    status=xuiInternalManagerRequestClose(&job->owned);*consumed=status==XUI_OK;
    return xuiInternalManagerScopeEnd(&scope,status);
}
XUI_API int xuiDocumentImageResourceReleaseChecked(xui_doc_image_request job)
{bool consumed=false;return xuiDocumentImageResourceReleaseOwnerChecked(job,&consumed);}
XUI_API void xuiDocumentImageResourceRelease(xui_doc_image_request job)
{if(job)(void)xuiDocumentImageResourceReleaseChecked(job);}
XUI_API xui_context xuiDocumentImageResourceGetContext(xui_doc_image_request job)
{return job && xuiInternalManagerAlive(&job->owned) ? job->context:NULL;}
XUI_API int xuiDocumentImageResourceLifetimeWatch(xui_doc_image_request job,xui_lifetime_link_t* link,xui_lifetime_invalidate_proc invalidate,void* user)
{return job ? xuiInternalManagerWatch(&job->owned,link,invalidate,user):XUI_ERROR_INVALID_ARGUMENT;}
XUI_API int xuiDocumentImageResourceGetAsyncStats(xui_context context,xui_doc_image_async_stats_t* stats)
{
    if(!stats || stats->iSize<sizeof(*stats) || !doc_image_context_alive(context))return XUI_ERROR_INVALID_ARGUMENT;
    xui_proxy proxy=doc_image_job_proxy(context);
    if(!proxy || (context->pOwnedProxy && !xuiInternalProxyCreator(proxy)))return XUI_ERROR_BUSY;
    xui_doc_image_async_stats_t value={.iSize=sizeof(value)};
    doc_image_index_enter();
    for(xui_doc_image_request job=doc_image_requests;job;job=job->next){
        if(job->context!=context || job->terminal)continue;
        ++value.iOutstanding;if(job->worker)++value.iRunning;else if(!job->started)++value.iQueued;
        value.iCopiedBytes+=job->encoded_bytes;
    }
    doc_image_index_leave();*stats=value;return XUI_OK;
}
#endif
