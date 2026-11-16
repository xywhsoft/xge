/* Immutable typed descriptor and one real payload/code reference. No loop,
 * graphics context, callback invocation or global backend selection here. */
struct BR_BODY {
    atomic_uint refs;
    BR_DESCRIPTOR descriptor;
    xge_callback_owner_t owner;
};
static xge_data_lifetime_t* BR_FN(Root)(BR_HANDLE backend)
{return backend ? (xge_data_lifetime_t*)(backend+1):NULL;}
static bool BR_FN(Trace)(const void* raw,xrtownershipvisitor visit,void* user);
static int BR_FN(Finalize)(void* raw,bool* complete);
static const xge_data_lifetime_type_t BR_FN(Type)={
    sizeof(struct BR_BODY),offsetof(struct BR_BODY,refs),NULL,BR_FN(Trace),BR_FN(Finalize),NULL};
#define DO_FN(name) BR_FN(Lifetime##name)
#define DO_PUBLIC(name) BR_PUBLIC(name)
#define DO_STATUS(name) XGE_##name
#define DO_API
#define DO_ROOT(backend) BR_FN(Root)(backend)
#define DO_HANDLE BR_HANDLE
#define DO_OBJECT_TYPE BR_FN(Type)
#define DO_GENERATION_TYPE BR_FN(Type)
#define DO_READ_READY(root) (BR_FN(LifetimeAdmit)((const xge_data_lifetime_t*)(root))==XGE_OK)
#include "xge_data_lifetime.inl"
#undef DO_FN
#undef DO_PUBLIC
#undef DO_STATUS
#undef DO_API
#undef DO_ROOT
#undef DO_HANDLE
#undef DO_OBJECT_TYPE
#undef DO_GENERATION_TYPE
#undef DO_READ_READY

static bool BR_FN(Trace)(const void* raw,xrtownershipvisitor visit,void* user)
{
    const struct BR_BODY* backend=raw;
    return !backend->owner.onDrop || visit(backend->owner.tReference,user);
}
static int BR_FN(Finalize)(void* raw,bool* complete)
{
    struct BR_BODY* backend=raw;*complete=false;xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))return XGE_ERROR_INVALID_STATE;
    xge_callback_owner_t owner=backend->owner;backend->owner=(xge_callback_owner_t){0};
    BR_FN(LifetimeMutationEnd)(&mutation);
    /* Drop consumes exactly once, even on failure. The owner must keep the
     * payload/code and complete native return tail valid, as in other owners. */
    *complete=true;
    return !owner.onDrop || owner.onDrop(owner.tReference.Data) ? XGE_OK:XGE_ERROR_CALLBACK_FAILED;
}
int BR_PUBLIC(CreateOwned)(BR_HANDLE* output,const BR_DESCRIPTOR* descriptor,
                           const xge_callback_owner_t* owner)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    *output=NULL;
    if(!descriptor || (owner && !__xgeBackendOwnerValid(owner)))return XGE_ERROR_INVALID_ARGUMENT;
    BR_DESCRIPTOR copy=*descriptor;
    size_t bytes=copy.sName ? strlen(copy.sName)+1u:0u;
    if(bytes>SIZE_MAX-sizeof(struct BR_BODY)-sizeof(xge_data_lifetime_t))return XGE_ERROR_OUT_OF_MEMORY;
    xerror* ambient=xrtTakeError();
    BR_HANDLE backend=xrtCalloc(1,sizeof(*backend)+sizeof(xge_data_lifetime_t)+bytes);
    if(!backend){
        xge_data_outcome_t result={0};BR_FN(LifetimeOutcomeBegin)(&result,XGE_ERROR_OUT_OF_MEMORY);
        return BR_FN(LifetimeOutcomeEnd)(&result,ambient);
    }
    atomic_init(&backend->refs,1u);backend->descriptor=copy;
    BR_FN(LifetimeInit)(BR_FN(Root)(backend),&BR_FN(Type),xrtThreadCurrentId());
    if(copy.sName){
        char* name=(char*)(BR_FN(Root)(backend)+1);
        memcpy(name,copy.sName,bytes);backend->descriptor.sName=name;
    }
    xge_callback_owner_t retained=owner ? *owner:(xge_callback_owner_t){0};
    bool held=false;
    if(retained.onHold)held=retained.onHold(retained.tReference.Data);
    if(held)backend->owner=retained;
    int status=(!retained.onHold || held) && !xrtGetError() ? XGE_OK:XGE_ERROR_CALLBACK_FAILED;
    xge_data_outcome_t result={0};BR_FN(LifetimeOutcomeBegin)(&result,status);
    if(result.iStatus==XGE_OK)*output=backend;
    else{
        bool consumed=false;
        BR_FN(LifetimeOutcomeAdd)(&result,BR_FN(LifetimeRelease)(BR_FN(Root)(backend),&consumed));
        if(!consumed)*output=backend;
    }
    return BR_FN(LifetimeOutcomeEnd)(&result,ambient);
}
int BR_PUBLIC(ShareOwned)(BR_HANDLE backend,BR_HANDLE* output)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    *output=NULL;int status=BR_FN(LifetimeAcquire)(BR_FN(Root)(backend),true,false);
    if(status==XGE_OK)*output=backend;
    return status;
}
int BR_PUBLIC(Describe)(BR_HANDLE backend,const BR_DESCRIPTOR** output)
{
    if(!output)return XGE_ERROR_INVALID_ARGUMENT;
    *output=NULL;int status=BR_FN(LifetimeAdmit)(BR_FN(Root)(backend));
    if(status==XGE_OK)*output=&backend->descriptor;
    return status;
}
int BR_PUBLIC(ReleaseOwnerChecked)(BR_HANDLE backend,bool* consumed)
{return BR_FN(LifetimeRelease)(BR_FN(Root)(backend),consumed);}
int BR_PUBLIC(ReleaseChecked)(BR_HANDLE backend)
{bool consumed=false;return BR_PUBLIC(ReleaseOwnerChecked)(backend,&consumed);}
