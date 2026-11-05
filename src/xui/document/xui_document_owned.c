#include "xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include "xui_document_owned_internal.h"
#include "xui_callback_hold_internal.h"
#include <limits.h>
#include <assert.h>

static const xrtownershipops modelOwnershipOps;
static const xrtownershipadapterv1 modelAdapter;
static atomic_uint* modelRefs(const doc_model_lifetime* root)
{
    /* Every participating concrete type proves this prefix offset. Access is
     * to its actual atomic first member, never through an aliased struct. */
    return (atomic_uint*)((char*)root - offsetof(doc_model_prefix,owned));
}
static void* modelObject(const doc_model_lifetime* root)
{return (char*)root - offsetof(doc_model_prefix,owned);}
static unsigned modelState(const doc_model_lifetime* root)
{return atomic_load_explicit(&root->state,memory_order_acquire)&~DOC_MODEL_LOCK;}
static void modelLock(doc_model_lifetime* root)
{while(atomic_fetch_or_explicit(&root->state,DOC_MODEL_LOCK,memory_order_acquire)&DOC_MODEL_LOCK){}}
static void modelUnlock(doc_model_lifetime* root)
{atomic_fetch_and_explicit(&root->state,~DOC_MODEL_LOCK,memory_order_release);}
static bool modelType(const doc_model_type* type)
{
    return type==&doc_model_document_type || type==&doc_model_snapshot_type ||
        type==&doc_model_change_type || type==&doc_model_transaction_type ||
        type==&doc_model_prepare_type || type==&doc_model_fragment_type || type==&doc_model_index_type;
}
bool doc_model_owned(const doc_model_lifetime* root)
{return root && modelType(root->ops);}
bool doc_model_mutation_begin(const doc_model_lifetime* root,xrtownershipscope* scope)
{return !doc_model_owned(root) || xrtOwnershipMutationBegin(scope);}
bool doc_allocator_mutation_begin(const doc_allocator* allocator,xrtownershipscope* scope)
{return !allocator || !allocator->managed || xrtOwnershipMutationBegin(scope);}
void doc_model_mutation_end(xrtownershipscope* scope)
{if(scope->Self && !xrtOwnershipScopeEnd(scope))abort();}
void doc_model_init(doc_model_lifetime* root,const doc_model_type* type,doc_allocator* allocator)
{
    root->ops=allocator && allocator->managed ? type:NULL;
    root->creator=allocator ? allocator->creator:0;root->claim=NULL;
    atomic_init(&root->state,0);atomic_init(&root->activity,0);
    if(root->ops && (type->flags&DOC_MODEL_ALLOCATED) && !(type->flags&DOC_MODEL_ALLOCATOR_OWNER))
        doc_allocator_retain(allocator);
}
int doc_model_admit(const doc_model_lifetime* root)
{
    if(!doc_model_owned(root))return XUI_ERROR_INVALID_ARGUMENT;
    if((root->ops->flags&DOC_MODEL_CREATOR) && root->creator!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    modelLock((doc_model_lifetime*)root);
    int status=root->claim || modelState(root) || !atomic_load_explicit(modelRefs(root),memory_order_acquire) ? XUI_ERROR_INVALID_STATE:XUI_OK;
    modelUnlock((doc_model_lifetime*)root);doc_model_mutation_end(&mutation);return status;
}
bool doc_model_readable(const doc_model_lifetime* root)
{
    if(!doc_model_owned(root))return false;
    if((root->ops->flags&DOC_MODEL_CREATOR) && root->creator!=xrtThreadCurrentId())return false;
    return atomic_load_explicit(modelRefs(root),memory_order_acquire)!=0 &&
        !(modelState(root)&(DOC_MODEL_FINALIZING|DOC_MODEL_CLEARED|DOC_MODEL_FINISHING|DOC_MODEL_FINISHED));
}
void doc_model_outcome_begin(doc_model_outcome* result,int status)
{
    result->status=status;result->error=xrtTakeError();
    if(status==XUI_OK && result->error)result->status=XUI_ERROR_CALLBACK_FAILED;
}
void doc_model_outcome_add(doc_model_outcome* result,int status)
{
    xerror* error=xrtTakeError();
    if(status==XUI_OK && !error)return;
    if(result->status==XUI_OK){result->status=status!=XUI_OK ? status:XUI_ERROR_CALLBACK_FAILED;result->error=error;}
    else xrtErrorFree(error);
}
int doc_model_outcome_end(doc_model_outcome* result)
{int status=result->status;xrtSetErrorTake(result->error);result->error=NULL;return status;}
int doc_model_owner_drop(const xui_callback_owner_t* owner)
{
    if(!owner || !owner->onDrop)return XUI_OK;
    bool success=owner->onDrop(owner->tReference.Data);
    return success && !xrtGetError() ? XUI_OK:XUI_ERROR_CALLBACK_FAILED;
}
static int modelAcquire(doc_model_lifetime* root,bool creator,bool active)
{
    if(!doc_model_owned(root))return XUI_ERROR_INVALID_ARGUMENT;
    if(creator && (root->ops->flags&DOC_MODEL_CREATOR) && root->creator!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    modelLock(root);
    int status=root->claim || modelState(root) ? XUI_ERROR_INVALID_STATE:XUI_OK;
    unsigned count=atomic_load_explicit(modelRefs(root),memory_order_relaxed);
    while(status==XUI_OK){
        if(!count || count==UINT_MAX){status=count ? XUI_DOC_ERROR_LIMIT:XUI_ERROR_INVALID_STATE;break;}
        if(atomic_compare_exchange_weak_explicit(modelRefs(root),&count,count+1,memory_order_relaxed,memory_order_relaxed)){
            if(active)atomic_fetch_add_explicit(&root->activity,1,memory_order_release);
            break;
        }
    }
    modelUnlock(root);doc_model_mutation_end(&mutation);return status;
}
int doc_model_add_ref(doc_model_lifetime* root)
{return modelAcquire(root,false,false);}
int doc_model_share_acquire(doc_model_lifetime* root,const doc_model_type* type)
{
    if(!doc_model_typed(root,type))return XUI_ERROR_INVALID_ARGUMENT;
    return modelAcquire(root,true,false);
}
xrtownershipref doc_model_reference(const doc_model_lifetime* root)
{
    return doc_model_owned(root) && atomic_load_explicit(modelRefs(root),memory_order_acquire) ?
        (xrtownershipref){root,&modelOwnershipOps}:(xrtownershipref){0};
}
static int modelReady(const doc_model_lifetime* root)
{
    if(!doc_model_owned(root))return XUI_ERROR_INVALID_ARGUMENT;
    unsigned state=modelState(root);
    if(state&DOC_MODEL_FINISHED)return XUI_OK;
    if((root->ops->flags&DOC_MODEL_CREATOR) && root->creator!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(atomic_load_explicit(&root->activity,memory_order_acquire) || (state&(DOC_MODEL_FINALIZING|DOC_MODEL_FINISHING)))return XUI_ERROR_BUSY;
    return root->ops->ready ? root->ops->ready(modelObject(root)):XUI_OK;
}
static int modelFinalize(doc_model_lifetime* root,bool* complete)
{
    *complete=false;
    if(modelState(root)&DOC_MODEL_FINALIZED){*complete=true;return XUI_OK;}
    int status=modelReady(root);if(status!=XUI_OK)return status==XUI_ERROR_BUSY ? XUI_STATUS_DEFERRED:status;
    xerror* ambient=xrtTakeError();
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation)){xrtSetErrorTake(ambient);return XUI_ERROR_INVALID_STATE;}
    atomic_fetch_or_explicit(&root->state,DOC_MODEL_FINALIZING,memory_order_release);
    doc_model_mutation_end(&mutation);
    status=root->ops->finalize ? root->ops->finalize(modelObject(root),complete):XUI_OK;
    if(!root->ops->finalize)*complete=true;
    if(*complete)atomic_fetch_or_explicit(&root->state,DOC_MODEL_FINALIZED,memory_order_release);
    atomic_fetch_and_explicit(&root->state,~DOC_MODEL_FINALIZING,memory_order_release);
    xerror* fresh=xrtTakeError();
    if((status==XUI_OK || (status==XUI_STATUS_DEFERRED && !*complete)) && fresh)status=XUI_ERROR_CALLBACK_FAILED;
    if(status==XUI_OK || (status==XUI_STATUS_DEFERRED && !*complete)){xrtErrorFree(fresh);fresh=ambient;ambient=NULL;}
    xrtErrorFree(ambient);xrtSetErrorTake(fresh);
    return status;
}
static int modelFinish(doc_model_lifetime* root)
{
    unsigned state=modelState(root);
    if(state&DOC_MODEL_FINISHED)return XUI_OK;
    if(!(state&DOC_MODEL_FINALIZED) || (state&DOC_MODEL_FINISHING))return XUI_ERROR_BUSY;
    xerror* ambient=xrtTakeError();
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation)){xrtSetErrorTake(ambient);return XUI_ERROR_INVALID_STATE;}
    atomic_fetch_or_explicit(&root->state,DOC_MODEL_FINISHING,memory_order_release);
    doc_model_mutation_end(&mutation);
    int status=root->ops->finish ? root->ops->finish(modelObject(root)):XUI_OK;
    /* Every valid finish consumes its detached payload duty even on failure.
     * No callback owner is retried because a diagnostic could not allocate. */
    atomic_fetch_or_explicit(&root->state,DOC_MODEL_FINISHED,memory_order_release);
    atomic_fetch_and_explicit(&root->state,~DOC_MODEL_FINISHING,memory_order_release);
    xerror* fresh=xrtTakeError();if(status==XUI_OK && fresh)status=XUI_ERROR_CALLBACK_FAILED;
    if(status==XUI_OK){fresh=ambient;ambient=NULL;}
    xrtErrorFree(ambient);xrtSetErrorTake(fresh);
    return status;
}
static void modelFree(doc_model_lifetime* root)
{
    const doc_model_type* type=root->ops;void* object=modelObject(root);
    if(type->flags&DOC_MODEL_ALLOCATED){
        doc_allocator* allocator=((doc_allocation*)object-1)->value.allocator;
        doc_free(object);doc_allocator_release(allocator);
    }else free(object);
}
static int modelRelease(doc_model_lifetime* root,bool uniqueOwner,bool* consumed)
{
    if(!consumed)return XUI_ERROR_INVALID_ARGUMENT;
    *consumed=false;
    if(!doc_model_owned(root))return XUI_ERROR_INVALID_ARGUMENT;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    modelLock(root);
    if(uniqueOwner && (modelState(root)&DOC_MODEL_OWNER_CONSUMED)){modelUnlock(root);doc_model_mutation_end(&mutation);return XUI_ERROR_INVALID_STATE;}
    unsigned count=atomic_load_explicit(modelRefs(root),memory_order_relaxed);
    for(;;){
        if(!count){modelUnlock(root);doc_model_mutation_end(&mutation);return XUI_ERROR_INVALID_STATE;}
        if(count==1 || uniqueOwner)break;
        if(atomic_compare_exchange_weak_explicit(modelRefs(root),&count,count-1,memory_order_acq_rel,memory_order_relaxed)){
            *consumed=true;modelUnlock(root);doc_model_mutation_end(&mutation);return XUI_OK;
        }
    }
    if((root->ops->flags&DOC_MODEL_CREATOR) && !(modelState(root)&DOC_MODEL_FINISHED) && root->creator!=xrtThreadCurrentId()){
        modelUnlock(root);doc_model_mutation_end(&mutation);return XUI_ERROR_BUSY;
    }
    int ready=modelReady(root);
    if(ready!=XUI_OK){modelUnlock(root);doc_model_mutation_end(&mutation);return ready==XUI_ERROR_BUSY ? XUI_STATUS_DEFERRED:ready;}
    /* Shared roots retire on their last true ref. The unique Txn duty retires
     * as soon as its sole owner is released: collector/call pins only preserve
     * its physical header and may not keep Document.writer installed. A real
     * active call, in contrast, was rejected by Ready without consuming it. */
    atomic_fetch_or_explicit(&root->state,DOC_MODEL_OWNER_CONSUMED,memory_order_release);
    modelUnlock(root);doc_model_mutation_end(&mutation);
    xerror* ambient=xrtTakeError();
    doc_model_outcome result={0};doc_model_outcome_begin(&result,XUI_OK);bool complete=false;
    int status=modelFinalize(root,&complete);doc_model_outcome_add(&result,status);
    if(!complete){
        atomic_fetch_and_explicit(&root->state,~DOC_MODEL_OWNER_CONSUMED,memory_order_release);
        if(result.status==XUI_OK)result.status=XUI_STATUS_DEFERRED;
        if(result.status==XUI_STATUS_DEFERRED && !result.error){result.error=ambient;ambient=NULL;}
        xrtErrorFree(ambient);
        return doc_model_outcome_end(&result);
    }
    status=modelFinish(root);doc_model_outcome_add(&result,status);
    if(!xrtOwnershipMutationBegin(&mutation))abort();
    count=atomic_fetch_sub_explicit(modelRefs(root),1,memory_order_acq_rel);assert(count);
    *consumed=true;doc_model_mutation_end(&mutation);if(count==1)modelFree(root);
    if(result.status==XUI_OK){result.error=ambient;ambient=NULL;}
    xrtErrorFree(ambient);
    return doc_model_outcome_end(&result);
}
int doc_model_release(doc_model_lifetime* root,bool* consumed)
{return modelRelease(root,doc_model_owned(root) && (root->ops->flags&DOC_MODEL_UNIQUE),consumed);}
int doc_model_call_begin(doc_model_lifetime* root,doc_model_call* call)
{
    if(!call || call->self || call->root || call->ambient)return XUI_ERROR_INVALID_ARGUMENT;
    if(!doc_model_owned(root))return XUI_ERROR_INVALID_ARGUMENT;
    int status=modelAcquire(root,true,true);if(status!=XUI_OK)return status;
    call->self=call;call->root=root;call->ambient=xrtTakeError();return XUI_OK;
}
int doc_model_call_end(doc_model_call* call,int status)
{
    if(!call || call->self!=call || !call->root)return XUI_ERROR_INVALID_STATE;
    doc_model_lifetime* root=call->root;xerror* ambient=call->ambient;call->self=NULL;call->root=NULL;call->ambient=NULL;
    doc_model_outcome result={0};doc_model_outcome_begin(&result,status);
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))abort();
    unsigned active=atomic_fetch_sub_explicit(&root->activity,1,memory_order_acq_rel);assert(active);doc_model_mutation_end(&mutation);
    bool consumed=false;doc_model_outcome_add(&result,modelRelease(root,false,&consumed));
    assert(consumed);
    if(result.status==XUI_OK){result.error=ambient;ambient=NULL;}
    xrtErrorFree(ambient);return doc_model_outcome_end(&result);
}
static bool modelCount(const void* data,size_t* count)
{
    const doc_model_lifetime* root=data;if(!doc_model_owned(root) || !count)return false;
    *count=atomic_load_explicit(modelRefs(root),memory_order_acquire);return *count!=0;
}
static bool modelTrace(const void* data,xrtownershipvisitor visit,void* user)
{
    const doc_model_lifetime* root=data;if(!doc_model_owned(root) || !visit)return false;
    unsigned state=modelState(root);if(state&(DOC_MODEL_CLEARED|DOC_MODEL_FINISHED))return true;
    if(state&(DOC_MODEL_FINALIZING|DOC_MODEL_FINISHING))return false;
    return !root->ops->trace || root->ops->trace(modelObject(root),visit,user);
}
static const xrtownershipops modelOwnershipOps={modelCount,modelTrace};
static bool modelHold(const void* data)
{return doc_model_add_ref((doc_model_lifetime*)data)==XUI_OK;}
static bool modelDropReady(const void* data)
{
    const doc_model_lifetime* root=data;
    return doc_model_owned(root) && (!(root->ops->flags&DOC_MODEL_CREATOR) ||
        (modelState(root)&DOC_MODEL_FINISHED) || root->creator==xrtThreadCurrentId());
}
static bool modelDropChecked(const void* data)
{bool consumed=false;int status=modelRelease((doc_model_lifetime*)data,false,&consumed);assert(consumed);return status==XUI_OK;}
static void modelDrop(const void* data){(void)modelDropChecked(data);}
static bool modelClaim(const void* data,const void* token)
{
    doc_model_lifetime* root=(doc_model_lifetime*)data;
    if(!token || !doc_model_owned(root) || (root->claim && root->claim!=token) ||
       (modelState(root)&(DOC_MODEL_CLEARED|DOC_MODEL_FINALIZING|DOC_MODEL_FINISHING)) || modelReady(root)!=XUI_OK)return false;
    root->claim=token;return true;
}
static void modelRestore(const void* data,const void* token)
{doc_model_lifetime* root=(doc_model_lifetime*)data;assert(token && root->claim==token && !(modelState(root)&DOC_MODEL_CLEARED));root->claim=NULL;}
static bool modelFinalizeLegacy(const void* data,const void* token)
{
    doc_model_lifetime* root=(doc_model_lifetime*)data;if(!token || root->claim!=token)return false;
    bool complete=false;int status=modelFinalize(root,&complete);return complete && status==XUI_OK;
}
static void modelClear(const void* data,const void* token)
{
    doc_model_lifetime* root=(doc_model_lifetime*)data;
    assert(token && root->claim==token && (modelState(root)&DOC_MODEL_FINALIZED));
    atomic_fetch_or_explicit(&root->state,DOC_MODEL_CLEARED,memory_order_release);
}
static bool modelFinishLegacy(const void* data,const void* token)
{
    doc_model_lifetime* root=(doc_model_lifetime*)data;
    if(!token || root->claim!=token || !(modelState(root)&DOC_MODEL_CLEARED))return false;
    return modelFinish(root)==XUI_OK;
}
static const xrtownershipadapterv1 modelAdapter={sizeof(modelAdapter),modelHold,modelDrop,modelClaim,modelRestore,modelFinalizeLegacy,modelClear,modelFinishLegacy};
static bool modelPreparationReady(const void* data){return modelReady(data)==XUI_OK;}
static xrtownershipprepareresult modelPreparation(const void* data,const void* token)
{
    const doc_model_lifetime* root=data;if(!token || root->claim!=token)return XRT_OWNERSHIP_PREPARE_FAILED;
    return modelReady(root)==XUI_OK ? XRT_OWNERSHIP_PREPARE_READY:XRT_OWNERSHIP_PREPARE_BUSY;
}
static const xrtownershippreparationv1 modelPreparationV1={sizeof(modelPreparationV1),&modelAdapter,modelPreparationReady,modelPreparation};
static bool modelBusy(const void* data){return modelReady(data)!=XUI_OK;}
static const xrtownershipquiescencev1 modelQuiescenceV1={sizeof(modelQuiescenceV1),&modelOwnershipOps,modelBusy};
static xrtownershipfinishresult modelCompletion(const void* data,const void* token)
{
    const doc_model_lifetime* root=data;if(!modelDropReady(data))return XRT_OWNERSHIP_FINISH_BUSY;
    if(!token || root->claim!=token || !(modelState(root)&DOC_MODEL_CLEARED))return XRT_OWNERSHIP_FINISH_FAILED;
    int status=modelFinish((doc_model_lifetime*)root);
    return status==XUI_OK ? XRT_OWNERSHIP_FINISH_READY:
        status==XUI_ERROR_BUSY && !(modelState(root)&DOC_MODEL_FINISHED) ? XRT_OWNERSHIP_FINISH_BUSY:XRT_OWNERSHIP_FINISH_FAILED;
}
static const xrtownershipfinishv2 modelCompletionV2={{sizeof(modelCompletionV2),&modelAdapter,modelCompletion},XRT_OWNERSHIP_FINISH_V2_TAG,modelDropChecked,modelDropReady};
static bool modelCanFinalize(const void* data,const void* token)
{const doc_model_lifetime* root=data;return token && root->claim==token && modelReady(root)==XUI_OK;}
static bool modelFinalizeStep(const void* data,const void* token,bool* complete)
{
    if(!complete)return false;
    *complete=false;
    const doc_model_lifetime* root=data;if(!token || root->claim!=token)return false;
    int status=modelFinalize((doc_model_lifetime*)root,complete);return status==XUI_OK || (status==XUI_STATUS_DEFERRED && !*complete);
}
static const xrtownershipfinalizationv1 modelFinalizationV1={sizeof(modelFinalizationV1),XRT_OWNERSHIP_FINALIZATION_V1_TAG,&modelAdapter,modelCanFinalize,modelFinalizeStep};
static bool modelReference(xrtownershipref ref)
{return ref.Ops==&modelOwnershipOps && doc_model_owned(ref.Data);}
XUI_API const xrtownershipadapterv1* xuiDocumentModelOwnershipAdapterV1(xrtownershipref ref)
{return modelReference(ref) ? &modelAdapter:NULL;}
XUI_API const xrtownershippreparationv1* xuiDocumentModelOwnershipPreparationV1(xrtownershipref ref)
{return modelReference(ref) ? &modelPreparationV1:NULL;}
XUI_API const xrtownershipquiescencev1* xuiDocumentModelOwnershipQuiescenceV1(xrtownershipref ref)
{return modelReference(ref) ? &modelQuiescenceV1:NULL;}
XUI_API const xrtownershipfinishv2* xuiDocumentModelOwnershipCompletionV2(xrtownershipref ref)
{return modelReference(ref) ? &modelCompletionV2:NULL;}
XUI_API const xrtownershipfinalizationv1* xuiDocumentModelOwnershipFinalizationV1(xrtownershipref ref)
{return modelReference(ref) ? &modelFinalizationV1:NULL;}
XUI_API bool xuiDocumentModelOwnershipReadReady(xrtownershipref ref)
{
    return modelReference(ref) && !(modelState(ref.Data)&(DOC_MODEL_FINALIZING|DOC_MODEL_FINALIZED|DOC_MODEL_CLEARED|DOC_MODEL_FINISHING|DOC_MODEL_FINISHED));
}
#endif
