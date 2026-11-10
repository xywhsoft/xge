/* Owned request/result graph over the native decoder. Worker state remains
 * private and is joined before any request/code destruction; callbacks run
 * only on the creator and hold snapshots through their complete return. */
#include <stdatomic.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include "xge_data_lifetime_internal.h"

struct xge_async_owned_t {
    atomic_uint refs;
    bool destroyed,busy,taking,notified,hasResult,graphics;
    int kind;
    xge_async_request_t raw;
    union {xge_image_t image;xge_texture_t texture;} target;
    xerror* error;
    xge_scene_proc callback;
    void* user;
    xge_callback_node_t* owner;
    xge_callback_owner_t code;
};
static xge_data_lifetime_t* __xgeOwnedLoadRoot(xge_async_owned value)
{return value ? (xge_data_lifetime_t*)(value+1):NULL;}
static bool __xgeLoadObjectReadable(const void* data)
{return !((const struct xge_async_owned_t*)data)->destroyed;}
static bool __xgeLoadObjectTrace(const void*,xrtownershipvisitor,void*);
static int __xgeLoadObjectFinalize(void*,bool*);
static int __xgeLoadObjectFinish(void*);
static const xge_data_lifetime_type_t __xgeOwnedLoadType={
    sizeof(struct xge_async_owned_t),offsetof(struct xge_async_owned_t,refs),
    __xgeLoadObjectReadable,__xgeLoadObjectTrace,__xgeLoadObjectFinalize,__xgeLoadObjectFinish};
#define DO_FN(name) __xgeOwnedLoad##name
#define DO_PUBLIC(name) xgeAsyncLoad##name
#define DO_STATUS(name) XGE_##name
#define DO_API
#define DO_ROOT(value) __xgeOwnedLoadRoot(value)
#define DO_HANDLE xge_async_owned
#define DO_OBJECT_TYPE __xgeOwnedLoadType
#define DO_GENERATION_TYPE __xgeOwnedLoadType
#include "xge_data_lifetime.inl"
#undef DO_FN
#undef DO_PUBLIC
#undef DO_STATUS
#undef DO_API
#undef DO_ROOT
#undef DO_HANDLE
#undef DO_OBJECT_TYPE
#undef DO_GENERATION_TYPE

static bool __xgeLoadObjectTrace(const void* data,xrtownershipvisitor visit,void* user)
{
    const struct xge_async_owned_t* request=data;
    return (!request->code.onDrop || visit(request->code.tReference,user)) &&
        (!request->owner || visit(request->owner->tOwner.tReference,user)) &&
        (!request->error || visit(xrtErrorOwnership(request->error),user));
}
/* A ready OS thread has already returned from all XGE worker code. Never
 * infer that from a worker-written flag and free its code before that tail. */
static int __xgeLoadWorkerReady(xge_async_owned request)
{
    if(!request->raw.pThread)return XGE_OK;
    xwaitresult status=xrtThreadWaitFor((xthread*)request->raw.pThread,0);
    return status==XWAIT_OK ? XGE_OK:status==XWAIT_TIMEOUT ? XGE_ERROR_BUSY:XGE_ERROR_INVALID_STATE;
}
static int __xgeLoadDiscard(xge_async_owned request)
{
    if(request->kind==XGE_ASYNC_IMAGE)xgeImageFree(&request->target.image);
    else xgeTextureFree(&request->target.texture);
    request->hasResult=false;
    if(!request->graphics)return XGE_OK;
    request->graphics=false;return __xgeGraphicsOwnerRelease(&g_xgeGraphicsLifetime);
}
static int __xgeLoadObjectFinalize(void* data,bool* complete)
{
    xge_async_owned request=data;request->destroyed=true;request->raw.bCancel=1;
    int status=__xgeLoadWorkerReady(request);
    if(status!=XGE_OK){*complete=false;return status==XGE_ERROR_BUSY ? XGE_STATUS_DEFERRED:status;}
    xgeAsyncRequestFree(&request->raw);
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XGE_ERROR_INVALID_STATE;
    xge_callback_node_t* owner=request->owner;request->owner=NULL;
    request->callback=NULL;request->user=NULL;xerror* error=request->error;request->error=NULL;
    __xgeOwnedLoadMutationEnd(&mutation);
    xge_data_outcome_t result={0};__xgeOwnedLoadOutcomeBegin(&result,__xgeLoadDiscard(request));
    xrtErrorFree(error);__xgeOwnedLoadOutcomeAdd(&result,__xgeCallbackNodeRelease(owner));
    *complete=true;xrtSetErrorTake(result.pError);return result.iStatus;
}
static int __xgeLoadObjectFinish(void* data)
{
    xge_async_owned request=data;xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))return XGE_ERROR_INVALID_STATE;
    xge_callback_owner_t code=request->code;request->code=(xge_callback_owner_t){0};
    __xgeOwnedLoadMutationEnd(&mutation);return __xgeCallbackOwnerDrop(code);
}
static int __xgeLoadCreate(xge_async_owned* output,int kind,const xge_callback_owner_t* codeOwner)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    *output=NULL;if(!__xgeCallbackOwnerValid(codeOwner))return XGE_ERROR_INVALID_ARGUMENT;
    xge_callback_owner_t code=codeOwner ? *codeOwner:(xge_callback_owner_t){0};bool held=false;
    int status=__xgeCallbackOwnerHold(code,&held);
    if(status!=XGE_OK){
        xerror* error=xrtTakeError();if(held)__xgeCallbackOutcome(__xgeCallbackOwnerDrop(code),&status,&error);
        xrtSetErrorTake(error);return status;
    }
    xge_async_owned request=xrtCalloc(1,sizeof(*request)+sizeof(xge_data_lifetime_t));
    if(!request){
        xerror* error=xrtTakeError();status=XGE_ERROR_OUT_OF_MEMORY;
        if(held)__xgeCallbackOutcome(__xgeCallbackOwnerDrop(code),&status,&error);
        xrtSetErrorTake(error);return status;
    }
    atomic_init(&request->refs,1u);request->kind=kind;request->code=code;
    xgeAsyncRequestInit(&request->raw);
    __xgeOwnedLoadInit(__xgeOwnedLoadRoot(request),&__xgeOwnedLoadType,xrtThreadCurrentId());
    *output=request;return XGE_OK;
}
int xgeAsyncImageCreateOwned(xge_async_owned* output,const xge_callback_owner_t* owner)
{return __xgeLoadCreate(output,XGE_ASYNC_IMAGE,owner);}
int xgeAsyncTextureCreateOwned(xge_async_owned* output,const xge_callback_owner_t* owner)
{return __xgeLoadCreate(output,XGE_ASYNC_TEXTURE,owner);}
int xgeAsyncLoadShareOwned(xge_async_owned request,xge_async_owned* output)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    *output=NULL;int status=__xgeOwnedLoadAcquire(__xgeOwnedLoadRoot(request),true,false);
    if(status==XGE_OK)*output=request;
    return status;
}
int xgeAsyncLoadReleaseOwnerChecked(xge_async_owned request,bool* consumed)
{return __xgeOwnedLoadRelease(__xgeOwnedLoadRoot(request),consumed);}
int xgeAsyncLoadReleaseChecked(xge_async_owned request)
{bool consumed=false;return xgeAsyncLoadReleaseOwnerChecked(request,&consumed);}
int xgeAsyncLoadInfoGet(xge_async_owned request,xge_async_load_info_t* output)
{
    if(!output || output->size!=sizeof(*output))return XGE_ERROR_INVALID_ARGUMENT;
    int status=__xgeOwnedLoadAdmit(__xgeOwnedLoadRoot(request));if(status!=XGE_OK)return status;
    xge_async_load_info_t info={0};info.size=sizeof(info);info.kind=request->kind;
    info.status=request->raw.iStatus;info.result=request->raw.iResult;info.progress=request->raw.fProgress;
    info.hasResult=request->hasResult;info.cancelRequested=request->raw.bCancel!=0;info.threaded=request->raw.bThreaded!=0;
    *output=info;return XGE_OK;
}
static int __xgeLoadAdmissionEnd(int status,xerror* ambient)
{xge_data_outcome_t result={0};__xgeOwnedLoadOutcomeBegin(&result,status);return __xgeOwnedLoadOutcomeEnd(&result,ambient);}
static int __xgeLoadOperationBegin(xge_async_owned request,xerror** ambient)
{
    *ambient=xrtTakeError();int status=__xgeOwnedLoadAcquire(__xgeOwnedLoadRoot(request),true,true);
    if(status!=XGE_OK)return status;
    if(request->busy || request->taking)status=XGE_ERROR_BUSY;
    else if(!__xgeCallbackActivityEnter(false))status=XGE_ERROR_INVALID_STATE;
    if(status==XGE_OK){request->busy=true;return XGE_OK;}
    xge_data_outcome_t result={0};__xgeOwnedLoadOutcomeBegin(&result,status);
    __xgeOwnedLoadOutcomeAdd(&result,__xgeOwnedLoadActivityEnd(__xgeOwnedLoadRoot(request)));
    xrtSetErrorTake(result.pError);return result.iStatus;
}
static int __xgeLoadOperationEnd(xge_async_owned request,int status,xerror* ambient)
{
    request->busy=false;xge_data_outcome_t result={0};__xgeOwnedLoadOutcomeBegin(&result,status);
    __xgeOwnedLoadOutcomeAdd(&result,__xgeOwnedLoadActivityEnd(__xgeOwnedLoadRoot(request)));
    /* ActivityEnd may have physically released request. */
    __xgeCallbackActivityLeave();return __xgeOwnedLoadOutcomeEnd(&result,ambient);
}
static int __xgeLoadNotify(xge_async_owned request)
{
    if(request->notified || request->raw.iStatus==XGE_ASYNC_PENDING || request->raw.iStatus==XGE_ASYNC_LOADING)return XGE_OK;
    request->notified=true;if(request->raw.bCancel || !request->callback)return XGE_OK;
    xge_scene_proc callback=request->callback;void* user=request->user;xge_callback_node_t* node=request->owner;
    if(!__xgeCallbackNodePin(node))return XGE_ERROR_INVALID_STATE;
    xge_callback_snapshot_t snapshot={0};int status=__xgeCallbackSnapshotBegin(node,true,&snapshot);
    if(status==XGE_OK)status=callback(user);
    if(status>0)status=XGE_ERROR_CALLBACK_FAILED;
    xge_data_outcome_t result={0};__xgeOwnedLoadOutcomeBegin(&result,status);
    __xgeOwnedLoadOutcomeAdd(&result,__xgeCallbackSnapshotEnd(&snapshot));
    xrtSetErrorTake(result.pError);return result.iStatus;
}
static int __xgeLoadSettle(xge_async_owned request,int status,xerror* workerError)
{
    bool callbackFailure=status==XGE_ERROR_CALLBACK_FAILED;
    xerror* error=xrtTakeError();if(error)xrtErrorFree(workerError);else error=workerError;
    if(request->raw.bCancel && !callbackFailure){xrtErrorFree(error);error=NULL;request->raw.iStatus=XGE_ASYNC_CANCELLED;status=XGE_ERROR;}
    else if(status!=XGE_OK || error){request->raw.iStatus=XGE_ASYNC_FAILED;if(status==XGE_OK)status=XGE_ERROR_RESOURCE_FAILED;}
    else request->raw.iStatus=XGE_ASYNC_READY;
    request->raw.iResult=status;request->raw.fProgress=1.0f;
    request->hasResult=request->raw.iStatus==XGE_ASYNC_READY;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation)){xrtSetErrorTake(error);return XGE_ERROR_INVALID_STATE;}
    xerror* old=request->error;request->error=error;__xgeOwnedLoadMutationEnd(&mutation);xrtErrorFree(old);
    if(!request->hasResult){
        int cleanup=__xgeLoadDiscard(request);
        if(cleanup!=XGE_OK)return cleanup;
    }
    /* An invalid/throwing source callback is a checked invocation failure,
     * rather than an ordinary IO result. Do not enter another language
     * callback while its caller's failure boundary is already latched. */
    if(callbackFailure){
        request->notified=true;xrtSetErrorTake(xrtErrorRef(request->error));return XGE_ERROR_CALLBACK_FAILED;
    }
    return __xgeLoadNotify(request);
}
static int __xgeLoadStart(xge_async_owned request,int kind,const char* path,uint32_t flags)
{
    xerror* ambient=NULL;int status=__xgeLoadOperationBegin(request,&ambient);
    if(status!=XGE_OK)return __xgeLoadAdmissionEnd(status,ambient);
    if(!path || request->kind!=kind || request->raw.iStatus!=XGE_ASYNC_PENDING)status=XGE_ERROR_INVALID_ARGUMENT;
    if(status==XGE_OK && kind==XGE_ASYNC_TEXTURE){status=__xgeGraphicsOwnerAcquire(&g_xgeGraphicsLifetime);request->graphics=status==XGE_OK;}
    if(status==XGE_OK){
        void* target=kind==XGE_ASYNC_IMAGE ? (void*)&request->target.image:(void*)&request->target.texture;
        status=__xgeAsyncBegin(&request->raw,kind,target,path,NULL,NULL);
        request->raw.iFlags=flags;
        if(status==XGE_OK){
            int threadStatus=g_xge.bAsyncThreadingEnabled ? __xgeAsyncStartThread(&request->raw):XGE_ERROR_UNSUPPORTED;
            if(threadStatus==XGE_OK)return __xgeLoadOperationEnd(request,XGE_OK,ambient);
            if(threadStatus==XGE_ERROR_UNSUPPORTED){
                status=kind==XGE_ASYNC_IMAGE ? xgeImageLoadEx(&request->target.image,path,flags):xgeTextureLoadEx(&request->target.texture,path,flags);
            }else status=threadStatus; /* Resource failure never falls back to blocking decode. */
            status=__xgeLoadSettle(request,status,NULL);
        }else if(request->graphics){
            request->graphics=false;(void)__xgeGraphicsOwnerRelease(&g_xgeGraphicsLifetime);
        }
    }
    return __xgeLoadOperationEnd(request,status,ambient);
}
int xgeAsyncImageStartOwned(xge_async_owned request,const char* path,uint32_t flags)
{return __xgeLoadStart(request,XGE_ASYNC_IMAGE,path,flags);}
int xgeAsyncTextureStartOwned(xge_async_owned request,const char* path,uint32_t flags)
{return __xgeLoadStart(request,XGE_ASYNC_TEXTURE,path,flags);}
int xgeAsyncLoadPollOwned(xge_async_owned request,int* output)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    xerror* ambient=NULL;int status=__xgeLoadOperationBegin(request,&ambient);
    if(status!=XGE_OK)return __xgeLoadAdmissionEnd(status,ambient);
    int ready=__xgeLoadWorkerReady(request);
    if(ready==XGE_ERROR_BUSY){*output=request->raw.iStatus;return __xgeLoadOperationEnd(request,XGE_OK,ambient);}
    if(ready!=XGE_OK)return __xgeLoadOperationEnd(request,ready,ambient);
    if(request->raw.bThreaded){
        __xge_async_payload_t* payload=request->raw.pPayload;
        xerror* error=payload ? xrtErrorRef(payload->pError):NULL;
        (void)xgeAsyncPoll(&request->raw);
        status=__xgeLoadSettle(request,request->raw.iResult,error);
    }else status=__xgeLoadNotify(request);
    if(status==XGE_OK)*output=request->raw.iStatus;
    return __xgeLoadOperationEnd(request,status,ambient);
}
int xgeAsyncLoadCancelOwned(xge_async_owned request)
{
    int status=__xgeOwnedLoadAdmit(__xgeOwnedLoadRoot(request));if(status!=XGE_OK)return status;
    return request->raw.iStatus==XGE_ASYNC_CANCELLED ? XGE_OK:xgeAsyncRequestCancel(&request->raw);
}
int xgeAsyncLoadResetOwned(xge_async_owned request)
{
    xerror* ambient=NULL;int status=__xgeLoadOperationBegin(request,&ambient);
    if(status!=XGE_OK)return __xgeLoadAdmissionEnd(status,ambient);
    status=__xgeLoadWorkerReady(request);
    if(status==XGE_OK){
        request->taking=true;
        xgeAsyncRequestFree(&request->raw);status=__xgeLoadDiscard(request);
        xrtownershipscope mutation={0};
        if(!xrtOwnershipMutationBegin(&mutation))status=XGE_ERROR_INVALID_STATE;
        else{ xerror* error=request->error;request->error=NULL;__xgeOwnedLoadMutationEnd(&mutation);xrtErrorFree(error); }
        request->notified=false;
        request->taking=false;
    }
    return __xgeLoadOperationEnd(request,status,ambient);
}
int xgeAsyncLoadThrowIfFailed(xge_async_owned request)
{
    int status=__xgeOwnedLoadAdmit(__xgeOwnedLoadRoot(request));if(status!=XGE_OK)return status;
    if(request->raw.iStatus==XGE_ASYNC_CANCELLED){xrtSetErrorKind(XERR_CANCELLED);return XGE_ERROR_INVALID_STATE;}
    if(request->raw.iStatus!=XGE_ASYNC_FAILED)return XGE_OK;
    if(request->error)xrtSetErrorTake(xrtErrorRef(request->error));
    return request->raw.iResult!=XGE_OK ? request->raw.iResult:XGE_ERROR_RESOURCE_FAILED;
}
int xgeAsyncLoadCallbackSetOwned(xge_async_owned request,xge_scene_proc callback,void* user,const xge_callback_owner_t* owner)
{
    xerror* ambient=xrtTakeError();int status=__xgeOwnedLoadAcquire(__xgeOwnedLoadRoot(request),true,true);
    if(status!=XGE_OK)return __xgeLoadAdmissionEnd(status,ambient);
    xge_callback_node_t* next=NULL;status=__xgeCallbackNodePrepare(callback!=NULL,false,owner,&next);
    xge_data_outcome_t result={0};__xgeOwnedLoadOutcomeBegin(&result,status);
    if(status==XGE_OK){
        xrtownershipscope mutation={0};
        if(!xrtOwnershipMutationBegin(&mutation)){
            __xgeOwnedLoadOutcomeAdd(&result,XGE_ERROR_INVALID_STATE);
            __xgeOwnedLoadOutcomeAdd(&result,__xgeCallbackNodeRelease(next));
        }else{
            xge_callback_node_t* old=request->owner;request->owner=next;
            request->callback=callback;request->user=callback ? user:NULL;
            __xgeOwnedLoadMutationEnd(&mutation);__xgeOwnedLoadOutcomeAdd(&result,__xgeCallbackNodeRelease(old));
        }
    }
    __xgeOwnedLoadOutcomeAdd(&result,__xgeOwnedLoadActivityEnd(__xgeOwnedLoadRoot(request)));
    return __xgeOwnedLoadOutcomeEnd(&result,ambient);
}
int xgeAsyncImageTakeOwned(xge_async_owned request,xge_image output)
{
    if(!output || output->pPixels)return XGE_ERROR_INVALID_ARGUMENT;
    int status=__xgeOwnedLoadAdmit(__xgeOwnedLoadRoot(request));if(status!=XGE_OK)return status;
    if(request->taking || request->kind!=XGE_ASYNC_IMAGE || !request->hasResult)return XGE_ERROR_INVALID_STATE;
    *output=request->target.image;memset(&request->target.image,0,sizeof(request->target.image));
    request->hasResult=false;return XGE_OK;
}
int xgeAsyncTextureTakeOwned(xge_async_owned request,xge_texture* output)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    *output=NULL;int status=__xgeOwnedLoadAcquire(__xgeOwnedLoadRoot(request),true,true);if(status!=XGE_OK)return status;
    xerror* ambient=xrtTakeError();
    if(request->taking || request->kind!=XGE_ASYNC_TEXTURE || !request->hasResult)status=XGE_ERROR_INVALID_STATE;
    xge_texture texture=NULL;bool taking=status==XGE_OK;
    if(taking)request->taking=true;
    if(status==XGE_OK){texture=xrtMalloc(sizeof(*texture));if(!texture)status=XGE_ERROR_OUT_OF_MEMORY;}
    if(status==XGE_OK){*texture=request->target.texture;status=xgeTextureAdoptStorage(texture);}
    if(status==XGE_OK){
        memset(&request->target.texture,0,sizeof(request->target.texture));request->hasResult=false;*output=texture;
        if(request->graphics){request->graphics=false;status=__xgeGraphicsOwnerRelease(&g_xgeGraphicsLifetime);}
    }else xrtFree(texture);
    if(taking)request->taking=false;
    xge_data_outcome_t result={0};__xgeOwnedLoadOutcomeBegin(&result,status);
    __xgeOwnedLoadOutcomeAdd(&result,__xgeOwnedLoadActivityEnd(__xgeOwnedLoadRoot(request)));
    return __xgeOwnedLoadOutcomeEnd(&result,ambient);
}
