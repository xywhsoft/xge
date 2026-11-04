#include "xui_config.h"
#if XGE_ENABLE_XUI
#include "xui_internal.h"
#include "xui_proxy_resource.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define XUI_OWNED_RESOURCE_MAGIC UINT32_C(0x584F574E)
struct xui_proxy_capability_t {
    xui_owned_resource_t tResource;
    xui_callback_owner_t tOwner;
    xui_proxy_t tProxy;
};
static const xrtownershipops resourceOps;
static int resourceFinish(xui_owned_resource_t*);
static int resourceRelease(xui_owned_resource_t*,bool,bool,bool*);
enum {
    RESOURCE_CLAIMED=1u, RESOURCE_FINALIZING=2u, RESOURCE_FINALIZED=4u,
    RESOURCE_CLEARED=8u, RESOURCE_FINISHED=16u, RESOURCE_PINNED=32u,
    RESOURCE_FINISHING=64u, RESOURCE_DISPOSING=128u
};
/* The XRT mutation gate admits concurrent mutators. It protects graph freeze,
 * not this object's counters. Writers acquire Mutation (or exclusive Freeze)
 * before this short lock. No allocator/backend/owner hook runs under it. */
static void resourceLock(const xui_owned_resource_t* value)
{
    xatomic32* lock=(xatomic32*)&value->tLock;
    while(xrtAtomic32Exchange(lock,1,XMEMORY_ACQUIRE))
        while(xrtAtomic32Load(lock,XMEMORY_RELAXED)){}
}
static void resourceUnlock(const xui_owned_resource_t* value)
{xrtAtomic32Store((xatomic32*)&value->tLock,0,XMEMORY_RELEASE);}
static uint32_t resourceState(const xui_owned_resource_t* value)
{return xrtAtomic32Load(&value->tState,XMEMORY_ACQUIRE);}
static void resourceStateSet(xui_owned_resource_t* value,uint32_t state)
{xrtAtomic32Store(&value->tState,state,XMEMORY_RELEASE);}
static void resourceInit(xui_owned_resource_t* value,unsigned kind,uint64_t creator,uint32_t flags)
{
    xrtAtomic32Init(&value->tState,0);xrtAtomic32Init(&value->tLock,0);
    value->iMagic=XUI_OWNED_RESOURCE_MAGIC;value->iKind=kind;
    value->iReferences=value->iOwners=1;
    value->iCreatorThread=creator;value->iFlags=flags;
}
static bool affinityReady(const xui_owned_resource_t* value)
{
    uint32_t state=resourceState(value);
    /* Semantic duties are consumed. Only resident XRT storage remains. */
    if((state&(RESOURCE_FINALIZED|RESOURCE_FINISHED))==(RESOURCE_FINALIZED|RESOURCE_FINISHED) &&
       !(state&(RESOURCE_FINALIZING|RESOURCE_FINISHING)))return true;
    return (value->iFlags&XUI_PROXY_OWNED_ANYTHREAD_FINALIZE)!=0 || value->iCreatorThread==xrtThreadCurrentId();
}
static bool checkedStatus(int status,const char* operation)
{
    if(status==XUI_OK)return true;
    if(!xrtGetError()){
        xerrordesc description={0};description.Kind=XERR_STATE;description.Code=status;
        description.Domain="xui.resource";description.Operation=operation;
        description.Message="Native resource cleanup returned a failing status.";
        xrtSetErrorTake(xrtErrorBuild(&description));
    }
    return false;
}
static bool valid(const xui_owned_resource_t* value)
{return value && value->iMagic==XUI_OWNED_RESOURCE_MAGIC && value->iKind>=XUI_OWNED_PROXY && value->iKind<=XUI_OWNED_SURFACE;}
static xui_owned_resource_t* proxyResource(const xui_proxy_t* proxy)
{
    if(!proxy || proxy->iSize!=sizeof(*proxy) || proxy->iVersion!=XUI_PROXY_VERSION || !proxy->_pCapability)return NULL;
    struct xui_proxy_capability_t* capability=proxy->_pCapability;
    xui_owned_resource_t* value=&capability->tResource;
    return valid(value) && value->iKind==XUI_OWNED_PROXY ? value:NULL;
}
bool xuiProxyIsOwned(const xui_proxy_t* proxy){return proxyResource(proxy)!=NULL;}
bool xuiInternalProxyResourceManaged(const void* raw,unsigned kind)
{const xui_owned_resource_t* value=raw;return valid(value) && value->iKind==kind;}
bool xuiInternalProxyResourceReady(const void* raw,unsigned kind)
{
    const xui_owned_resource_t* value=raw;
    return xuiInternalProxyResourceManaged(raw,kind) &&
        !(resourceState(value)&(RESOURCE_CLAIMED|RESOURCE_FINALIZING|RESOURCE_FINALIZED|RESOURCE_CLEARED|RESOURCE_DISPOSING)) &&
        value->iCreatorThread==xrtThreadCurrentId();
}
static int mutationBegin(xrtownershipscope* scope)
{return xrtOwnershipMutationBegin(scope) ? XUI_OK:XUI_ERROR_INVALID_STATE;}
static void mutationEnd(xrtownershipscope* scope)
{if(!xrtOwnershipScopeEnd(scope))abort();}
static int resourceRef(xui_owned_resource_t* value,bool owner,bool pin)
{
    if(!valid(value))return XUI_ERROR_INVALID_ARGUMENT;
    xrtownershipscope mutation={0};int status=mutationBegin(&mutation);if(status!=XUI_OK)return status;
    resourceLock(value);uint32_t state=resourceState(value);
    if(!value->iReferences || (state&(RESOURCE_CLAIMED|RESOURCE_CLEARED)) ||
        ((owner || pin) && (state&(RESOURCE_FINALIZING|RESOURCE_FINALIZED))) ||
        value->iReferences==SIZE_MAX || (owner && value->iOwners==SIZE_MAX) || (pin && value->iPins==SIZE_MAX)){
        resourceUnlock(value);mutationEnd(&mutation);return XUI_ERROR_INVALID_STATE;
    }
    ++value->iReferences;if(owner)++value->iOwners;if(pin)++value->iPins;
    if(pin)resourceStateSet(value,state|RESOURCE_PINNED);
    resourceUnlock(value);mutationEnd(&mutation);return XUI_OK;
}
static void outcome(int next,int* status,xerror** first)
{
    xerror* error=xrtTakeError();
    if(next==XUI_OK && !error)return;
    if(*status==XUI_OK){*status=next==XUI_OK ? XUI_ERROR_CALLBACK_FAILED:next;*first=error;}
    else xrtErrorFree(error);
}
/* A backend finalizer consumes the payload on every result, including a
 * failing result without a diagnostic. Its one semantic duty is never retried.
 * Normal close and graph finalization share exactly this implementation. */
static int resourceFinalize(xui_owned_resource_t* value)
{
    xrtownershipscope mutation={0};int status=mutationBegin(&mutation);if(status!=XUI_OK)return status;
    resourceLock(value);uint32_t state=resourceState(value);
    if(state&(RESOURCE_FINALIZING|RESOURCE_PINNED)){
        resourceUnlock(value);mutationEnd(&mutation);return XUI_ERROR_BUSY;
    }
    if(state&RESOURCE_FINALIZED){resourceUnlock(value);mutationEnd(&mutation);return XUI_OK;}
    if(!affinityReady(value)){resourceUnlock(value);mutationEnd(&mutation);return XUI_ERROR_BUSY;}
    resourceStateSet(value,state|RESOURCE_FINALIZING|RESOURCE_FINALIZED);
    resourceUnlock(value);mutationEnd(&mutation);
    xerror* previous=xrtTakeError();xerror* first=NULL;
    if(value->onFinalize)outcome(value->onFinalize(value->pProxy,xuiInternalProxyResourcePayload(value)),&status,&first);
    mutation=(xrtownershipscope){0};
    if(mutationBegin(&mutation)!=XUI_OK)abort();
    resourceLock(value);resourceStateSet(value,resourceState(value)&~RESOURCE_FINALIZING);
    resourceUnlock(value);mutationEnd(&mutation);
    if(status!=XUI_OK){xrtErrorFree(previous);xrtSetErrorTake(first);}
    else xrtSetErrorTake(previous);
    return status;
}
/* Called under exclusive freeze, or while normal last release has exclusive
 * ownership of the physical record. Never calls backend/owner code. */
static void resourceClear(xui_owned_resource_t* value)
{
    /* Like Cell's native quarantine, the cleared bit hides these owning
     * slots from the committed graph. Their physical references remain in
     * the same storage until Finish, with no duplicate retired-slot fields. */
    resourceStateSet(value,resourceState(value)|RESOURCE_CLEARED);
}
static int resourceFinish(xui_owned_resource_t* value)
{
    xrtownershipscope mutation={0};int status=mutationBegin(&mutation);if(status!=XUI_OK)return status;
    resourceLock(value);uint32_t state=resourceState(value);
    if(state&RESOURCE_FINISHING){resourceUnlock(value);mutationEnd(&mutation);return XUI_ERROR_BUSY;}
    if(state&RESOURCE_FINISHED){resourceUnlock(value);mutationEnd(&mutation);return XUI_OK;}
    if(!affinityReady(value)){resourceUnlock(value);mutationEnd(&mutation);return XUI_ERROR_BUSY;}
    xui_proxy retired=value->pProxy;value->pProxy=NULL;
    xui_callback_owner_t owner={0};
    if(value->iKind==XUI_OWNED_PROXY){
        struct xui_proxy_capability_t* capability=(void*)value;
        owner=capability->tOwner;capability->tOwner=(xui_callback_owner_t){0};
    }
    resourceStateSet(value,state|RESOURCE_FINISHED|RESOURCE_FINISHING);
    resourceUnlock(value);mutationEnd(&mutation);
    xerror* first=NULL;
    if(retired){
        bool consumed=false;outcome(xuiProxyReleaseOwnerChecked(retired,&consumed),&status,&first);
        if(!consumed){
            /* Infrastructure refusal consumed no edge. Keep it retryable;
             * completed owner Drop duties are never installed a second time. */
            mutation=(xrtownershipscope){0};if(mutationBegin(&mutation)!=XUI_OK)abort();
            resourceLock(value);value->pProxy=retired;
            resourceStateSet(value,resourceState(value)&~RESOURCE_FINISHED);
            resourceUnlock(value);mutationEnd(&mutation);
            if(status==XUI_OK)status=XUI_ERROR_INVALID_STATE;
        }
    }
    if(owner.onDrop)outcome(owner.onDrop(owner.tReference.Data) ? XUI_OK:XUI_ERROR_CALLBACK_FAILED,&status,&first);
    mutation=(xrtownershipscope){0};if(mutationBegin(&mutation)!=XUI_OK)abort();
    resourceLock(value);resourceStateSet(value,resourceState(value)&~RESOURCE_FINISHING);
    resourceUnlock(value);mutationEnd(&mutation);
    if(status!=XUI_OK)xrtSetErrorTake(first);
    return status;
}
static int resourceRelease(xui_owned_resource_t* value,bool owner,bool pin,bool* consumed)
{
    if(consumed)*consumed=false;
    if(!valid(value) || (owner && !consumed))return XUI_ERROR_INVALID_ARGUMENT;
    xrtownershipscope mutation={0};int status=mutationBegin(&mutation);if(status!=XUI_OK)return status;
    resourceLock(value);
    if(!value->iReferences || (owner && !value->iOwners) || (pin && !value->iPins)){
        resourceUnlock(value);mutationEnd(&mutation);return XUI_ERROR_INVALID_STATE;
    }
    if(value->iReferences==1 && !affinityReady(value)){
        resourceUnlock(value);mutationEnd(&mutation);return XUI_ERROR_BUSY;
    }
    if(owner){--value->iOwners;*consumed=true;}
    if(pin && --value->iPins==0)resourceStateSet(value,resourceState(value)&~RESOURCE_PINNED);
    bool last=--value->iReferences==0;
    if(last)resourceStateSet(value,resourceState(value)|RESOURCE_DISPOSING);
    resourceUnlock(value);mutationEnd(&mutation);
    if(!last)return XUI_OK;
    xerror* previous=xrtTakeError();xerror* first=NULL;
    outcome(resourceFinalize(value),&status,&first);
    if(!(resourceState(value)&RESOURCE_CLEARED)){
        mutation=(xrtownershipscope){0};if(mutationBegin(&mutation)!=XUI_OK)abort();
        resourceLock(value);resourceClear(value);resourceUnlock(value);mutationEnd(&mutation);
    }
    outcome(resourceFinish(value),&status,&first);
    value->iMagic=0;xrtFree(value);
    if(status!=XUI_OK){xrtErrorFree(previous);xrtSetErrorTake(first);}
    else xrtSetErrorTake(previous);
    return status;
}
int xuiProxyAddRef(xui_proxy proxy)
{return resourceRef(proxyResource(proxy),true,false);}
int xuiProxyReleaseOwnerChecked(xui_proxy proxy,bool* consumed)
{return resourceRelease(proxyResource(proxy),true,false,consumed);}
int xuiProxyReleaseChecked(xui_proxy proxy)
{bool consumed=false;return xuiProxyReleaseOwnerChecked(proxy,&consumed);}
int xuiFontAddRef(xui_font font)
{return xuiInternalProxyResourceManaged(font,XUI_OWNED_FONT) ? resourceRef((void*)font,true,false):XUI_ERROR_INVALID_ARGUMENT;}
int xuiSurfaceAddRef(xui_surface surface)
{return xuiInternalProxyResourceManaged(surface,XUI_OWNED_SURFACE) ? resourceRef((void*)surface,true,false):XUI_ERROR_INVALID_ARGUMENT;}
int xuiFontReleaseOwnerChecked(xui_font font,bool* consumed)
{if(consumed)*consumed=false;return xuiInternalProxyResourceManaged(font,XUI_OWNED_FONT) ? resourceRelease((void*)font,true,false,consumed):XUI_ERROR_INVALID_ARGUMENT;}
int xuiSurfaceReleaseOwnerChecked(xui_surface surface,bool* consumed)
{if(consumed)*consumed=false;return xuiInternalProxyResourceManaged(surface,XUI_OWNED_SURFACE) ? resourceRelease((void*)surface,true,false,consumed):XUI_ERROR_INVALID_ARGUMENT;}
int xuiFontReleaseChecked(xui_font font)
{bool consumed=false;return xuiFontReleaseOwnerChecked(font,&consumed);}
int xuiSurfaceReleaseChecked(xui_surface surface)
{bool consumed=false;return xuiSurfaceReleaseOwnerChecked(surface,&consumed);}
xui_proxy xuiFontGetProxy(xui_font font)
{
    const xui_owned_resource_t* value=(const void*)font;
    if(!xuiInternalProxyResourceManaged(font,XUI_OWNED_FONT))return NULL;
    resourceLock(value);xui_proxy proxy=!(resourceState(value)&RESOURCE_CLEARED) ? value->pProxy:NULL;resourceUnlock(value);return proxy;
}
xui_proxy xuiSurfaceGetProxy(xui_surface surface)
{
    const xui_owned_resource_t* value=(const void*)surface;
    if(!xuiInternalProxyResourceManaged(surface,XUI_OWNED_SURFACE))return NULL;
    resourceLock(value);xui_proxy proxy=!(resourceState(value)&RESOURCE_CLEARED) ? value->pProxy:NULL;resourceUnlock(value);return proxy;
}
int xuiInternalProxyResourcePin(void* raw,unsigned kind)
{return xuiInternalProxyResourceManaged(raw,kind) ? resourceRef(raw,false,true):XUI_ERROR_INVALID_ARGUMENT;}
int xuiInternalProxyResourceUnpin(void* raw,unsigned kind)
{
    if(!xuiInternalProxyResourceManaged(raw,kind))return XUI_ERROR_INVALID_ARGUMENT;
    return resourceRelease(raw,false,true,NULL);
}
static bool ownerValid(const xui_callback_owner_t* owner)
{return owner && owner->tReference.Data && owner->tReference.Ops && owner->tReference.Ops->Count && owner->tReference.Ops->Trace && owner->onHold && owner->onDrop;}
int xuiInternalProxyCreateOwned(const xui_proxy_t* source,const xui_callback_owner_t* owner,bool trusted,uint32_t flags,xui_proxy* output)
{
    if(!source || !output || source->iSize!=sizeof(*source) || source->iVersion!=XUI_PROXY_VERSION ||
        source->_pCapability || (flags&~XUI_PROXY_OWNED_ANYTHREAD_FINALIZE) || (!ownerValid(owner) && !(trusted && !owner)))return XUI_ERROR_INVALID_ARGUMENT;
    struct xui_proxy_capability_t* capability=xrtCalloc(1,sizeof(*capability));if(!capability)return XUI_ERROR_OUT_OF_MEMORY;
    /* Snapshot caller metadata before an arbitrary Hold can reenter and edit
     * the input record. No pointer to caller storage survives registration. */
    capability->tProxy=*source;
    if(owner)capability->tOwner=*owner;
    if(owner){
        xui_callback_owner_t incoming=capability->tOwner;
        xerror* previous=xrtTakeError();bool held=incoming.onHold(incoming.tReference.Data);
        xerror* first=xrtTakeError();
        if(!held || first){
            if(held){(void)incoming.onDrop(incoming.tReference.Data);xrtClearError();}
            xrtErrorFree(previous);xrtSetErrorTake(first);xrtFree(capability);return XUI_ERROR_CALLBACK_FAILED;
        }
        xrtSetErrorTake(previous);
    }
    resourceInit(&capability->tResource,XUI_OWNED_PROXY,xrtThreadCurrentId(),flags);
    capability->tProxy._pCapability=capability;
    *output=&capability->tProxy;return XUI_OK;
}
int xuiProxyCreateOwned(const xui_proxy_t* source,const xui_callback_owner_t* owner,xui_proxy* output)
{return xuiInternalProxyCreateOwned(source,owner,false,0,output);}
int xuiProxyCreateOwnedEx(const xui_proxy_t* source,const xui_callback_owner_t* owner,uint32_t flags,xui_proxy* output)
{return xuiInternalProxyCreateOwned(source,owner,false,flags,output);}
static int resourceCreate(xui_proxy proxy,size_t payload,xui_owned_payload_finalize_proc finalize,unsigned kind,void** output,void** data)
{
    if(!output || !data || !finalize || !proxyResource(proxy) || payload>SIZE_MAX-sizeof(xui_owned_resource_t))return XUI_ERROR_INVALID_ARGUMENT;
    xui_owned_resource_t* value=xrtCalloc(1,sizeof(*value)+payload);if(!value)return XUI_ERROR_OUT_OF_MEMORY;
    int status=xuiProxyAddRef(proxy);if(status!=XUI_OK){xrtFree(value);return status;}
    const xui_owned_resource_t* parent=proxyResource(proxy);
    resourceInit(value,kind,parent->iCreatorThread,parent->iFlags);
    value->pProxy=&proxy->_pCapability->tProxy;value->onFinalize=finalize;
    *output=value;*data=xuiInternalProxyResourcePayload(value);return XUI_OK;
}
int xuiFontCreateOwned(xui_proxy proxy,size_t payload,xui_proxy_payload_finalize_proc finalize,xui_font* output,void** data)
{void* raw=NULL;if(!output)return XUI_ERROR_INVALID_ARGUMENT;int status=resourceCreate(proxy,payload,finalize,XUI_OWNED_FONT,&raw,data);if(status==XUI_OK)*output=raw;return status;}
int xuiSurfaceCreateOwned(xui_proxy proxy,size_t payload,xui_proxy_payload_finalize_proc finalize,xui_surface* output,void** data)
{void* raw=NULL;if(!output)return XUI_ERROR_INVALID_ARGUMENT;int status=resourceCreate(proxy,payload,finalize,XUI_OWNED_SURFACE,&raw,data);if(status==XUI_OK)*output=raw;return status;}
static bool resourceCount(const void* raw,size_t* count)
{const xui_owned_resource_t* value=raw;if(!valid(value) || !count || !value->iReferences)return false;*count=value->iReferences;return true;}
static bool resourceTrace(const void* raw,xrtownershipvisitor visit,void* context)
{
    const xui_owned_resource_t* value=raw;if(!valid(value) || !visit)return false;
    if(resourceState(value)&RESOURCE_CLEARED)return true;
    if(value->iKind==XUI_OWNED_PROXY){
        const struct xui_proxy_capability_t* capability=raw;
        return !capability->tOwner.tReference.Data || visit(capability->tOwner.tReference,context);
    }
    return !value->pProxy || visit(xuiProxyOwnership(value->pProxy),context);
}
static const xrtownershipops resourceOps={resourceCount,resourceTrace};
static xrtownershipref ownership(const void* raw,unsigned kind)
{return xuiInternalProxyResourceManaged(raw,kind) ? (xrtownershipref){raw,&resourceOps}:(xrtownershipref){0};}
xrtownershipref xuiProxyOwnership(xui_proxy proxy)
{return ownership(proxyResource(proxy),XUI_OWNED_PROXY);}
xrtownershipref xuiFontOwnership(xui_font font){return ownership(font,XUI_OWNED_FONT);}
xrtownershipref xuiSurfaceOwnership(xui_surface surface){return ownership(surface,XUI_OWNED_SURFACE);}
static bool adapterHold(const void* raw){return resourceRef((void*)raw,false,false)==XUI_OK;}
/* Both paths consume one physical snapshot reference exactly once. V2
 * preserves an explicit failed result even without an allocated diagnostic. */
static bool adapterDropChecked(const void* raw){return checkedStatus(resourceRelease((void*)raw,false,false,NULL),"ownership.drop");}
static void adapterDrop(const void* raw){(void)adapterDropChecked(raw);}
static bool busy(const void* raw)
{const xui_owned_resource_t* value=raw;return !valid(value) || (resourceState(value)&(RESOURCE_PINNED|RESOURCE_FINALIZING|RESOURCE_FINISHING))!=0 || !affinityReady(value);}
static bool adapterClaim(const void* raw,const void* token)
{
    xui_owned_resource_t* value=(void*)raw;
    if(!token || !valid(value))return false;
    resourceLock(value);uint32_t state=resourceState(value);
    if((state&(RESOURCE_PINNED|RESOURCE_FINALIZING|RESOURCE_FINISHING|RESOURCE_CLEARED)) || !affinityReady(value) || (value->pClaim && value->pClaim!=token)){
        resourceUnlock(value);return false;
    }
    value->pClaim=token;resourceStateSet(value,state|RESOURCE_CLAIMED);resourceUnlock(value);return true;
}
static void adapterRestore(const void* raw,const void* token)
{xui_owned_resource_t* value=(void*)raw;resourceLock(value);if(!token || value->pClaim!=token || (resourceState(value)&RESOURCE_CLEARED))abort();value->pClaim=NULL;resourceStateSet(value,resourceState(value)&~RESOURCE_CLAIMED);resourceUnlock(value);}
static bool adapterFinalize(const void* raw,const void* token)
{
    xui_owned_resource_t* value=(void*)raw;resourceLock(value);bool claimed=token && value->pClaim==token;resourceUnlock(value);
    return claimed && checkedStatus(resourceFinalize(value),"ownership.finalize");
}
static void adapterClear(const void* raw,const void* token)
{xui_owned_resource_t* value=(void*)raw;resourceLock(value);if(!token || value->pClaim!=token || (resourceState(value)&RESOURCE_CLEARED))abort();resourceClear(value);resourceUnlock(value);}
static int adapterFinishStatus(const void* raw,const void* token)
{
    xui_owned_resource_t* value=(void*)raw;resourceLock(value);bool committed=token && value->pClaim==token && (resourceState(value)&RESOURCE_CLEARED);resourceUnlock(value);
    return committed ? resourceFinish(value):XUI_ERROR_INVALID_STATE;
}
static bool adapterFinish(const void* raw,const void* token)
{return checkedStatus(adapterFinishStatus(raw,token),"ownership.finish");}
static const xrtownershipadapterv1 resourceAdapter={sizeof(resourceAdapter),adapterHold,adapterDrop,adapterClaim,adapterRestore,adapterFinalize,adapterClear,adapterFinish};
static bool preparationReady(const void* raw){return !busy(raw);}
static xrtownershipprepareresult prepare(const void* raw,const void* token)
{
    const xui_owned_resource_t* value=raw;if(!token || !valid(value))return XRT_OWNERSHIP_PREPARE_FAILED;
    resourceLock(value);bool claimed=value->pClaim==token && !(resourceState(value)&RESOURCE_CLEARED);resourceUnlock(value);
    return !claimed ? XRT_OWNERSHIP_PREPARE_FAILED:busy(raw) ? XRT_OWNERSHIP_PREPARE_BUSY:XRT_OWNERSHIP_PREPARE_READY;
}
static const xrtownershippreparationv1 resourcePreparation={sizeof(resourcePreparation),&resourceAdapter,preparationReady,prepare};
static xrtownershipfinishresult finish(const void* raw,const void* token)
{
    int status=adapterFinishStatus(raw,token);
    if(status==XUI_ERROR_BUSY)return XRT_OWNERSHIP_FINISH_BUSY;
    return checkedStatus(status,"ownership.finish") ? XRT_OWNERSHIP_FINISH_READY:XRT_OWNERSHIP_FINISH_FAILED;
}
static const xrtownershipfinishv1 resourceCompletion={sizeof(resourceCompletion),&resourceAdapter,finish};
static const xrtownershipfinishv2 resourceCompletionV2={
    {sizeof(resourceCompletionV2),&resourceAdapter,finish},
    XRT_OWNERSHIP_FINISH_V2_TAG,adapterDropChecked,preparationReady
};
static const xrtownershipquiescencev1 resourceQuiescence={sizeof(resourceQuiescence),&resourceOps,busy};
static bool exact(xrtownershipref ref){return ref.Ops==&resourceOps && valid(ref.Data);}
const xrtownershipadapterv1* xuiProxyResourceOwnershipAdapterV1(xrtownershipref ref){return exact(ref) ? &resourceAdapter:NULL;}
const xrtownershippreparationv1* xuiProxyResourceOwnershipPreparationV1(xrtownershipref ref){return exact(ref) ? &resourcePreparation:NULL;}
const xrtownershipfinishv1* xuiProxyResourceOwnershipCompletionV1(xrtownershipref ref){return exact(ref) ? &resourceCompletion:NULL;}
const xrtownershipfinishv2* xuiProxyResourceOwnershipCompletionV2(xrtownershipref ref){return exact(ref) ? &resourceCompletionV2:NULL;}
const xrtownershipquiescencev1* xuiProxyResourceOwnershipQuiescenceV1(xrtownershipref ref){return exact(ref) ? &resourceQuiescence:NULL;}
bool xuiProxyResourceOwnershipBusy(xrtownershipref ref){return !exact(ref) || busy(ref.Data);}
#endif
