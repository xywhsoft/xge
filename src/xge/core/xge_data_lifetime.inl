/* Instantiated once by each exact typed DataObject family. Only cold native
 * lifetime/ownership mechanics are shared; there is no operation dispatcher or
 * incompatible callback cast. DO_FN/DO_PUBLIC/DO_STATUS/DO_API/DO_ROOT are
 * provided by that family and are undefined by its including source. */
static const xrtownershipops DO_FN(Ops);
static const xrtownershipadapterv1 DO_FN(Adapter);
static bool DO_FN(Known)(const xge_data_lifetime_t* root)
{return root && (root->pType==&DO_OBJECT_TYPE || root->pType==&DO_GENERATION_TYPE);}
static unsigned DO_FN(State)(const xge_data_lifetime_t* root)
{return atomic_load_explicit(&root->iState,memory_order_acquire)&~XGE_DATA_LOCK;}
static void DO_FN(Lock)(xge_data_lifetime_t* root)
{while(atomic_fetch_or_explicit(&root->iState,XGE_DATA_LOCK,memory_order_acquire)&XGE_DATA_LOCK){}}
static void DO_FN(Unlock)(xge_data_lifetime_t* root)
{atomic_fetch_and_explicit(&root->iState,~XGE_DATA_LOCK,memory_order_release);}
static void* DO_FN(Body)(const xge_data_lifetime_t* root)
{return (char*)root-root->pType->iBodyBytes;}
static atomic_uint* DO_FN(Refs)(const xge_data_lifetime_t* root)
{return (atomic_uint*)((char*)DO_FN(Body)(root)+root->pType->iRefsOffset);}
static void DO_FN(MutationEnd)(xrtownershipscope* scope)
{if(!xrtOwnershipScopeEnd(scope))abort();}
static void DO_FN(Init)(xge_data_lifetime_t* root,const xge_data_lifetime_type_t* type,uint64_t creator)
{
    root->pType=type;root->iCreator=creator;root->pClaim=NULL;
    atomic_init(&root->iState,0u);atomic_init(&root->iActivity,0u);
}
static void DO_FN(OutcomeBegin)(xge_data_outcome_t* result,int status)
{
    result->iStatus=status;result->pError=xrtTakeError();
    if((status==DO_STATUS(OK) || status==DO_STATUS(STATUS_DEFERRED)) && result->pError)result->iStatus=DO_STATUS(ERROR_CALLBACK_FAILED);
}
static void DO_FN(OutcomeAdd)(xge_data_outcome_t* result,int status)
{
    xerror* error=xrtTakeError();if(status==DO_STATUS(OK) && !error)return;
    if(result->iStatus==DO_STATUS(OK) || (result->iStatus==DO_STATUS(STATUS_DEFERRED) && !result->pError)){
        result->iStatus=status!=DO_STATUS(OK) ? status:DO_STATUS(ERROR_CALLBACK_FAILED);
        if(result->iStatus==DO_STATUS(STATUS_DEFERRED) && error)result->iStatus=DO_STATUS(ERROR_CALLBACK_FAILED);
        result->pError=error;
    }
    else xrtErrorFree(error);
}
static int DO_FN(OutcomeEnd)(xge_data_outcome_t* result,xerror* ambient)
{
    if(result->iStatus==DO_STATUS(OK) || (result->iStatus==DO_STATUS(STATUS_DEFERRED) && !result->pError)){
        xrtErrorFree(result->pError);result->pError=ambient;ambient=NULL;
    }
    xrtErrorFree(ambient);int status=result->iStatus;xrtSetErrorTake(result->pError);result->pError=NULL;return status;
}
static bool DO_FN(Readable)(const xge_data_lifetime_t* root)
{
    return DO_FN(Known)(root) && root->iCreator==xrtThreadCurrentId() &&
        atomic_load_explicit(DO_FN(Refs)(root),memory_order_acquire)!=0u &&
        !(DO_FN(State)(root)&(XGE_DATA_FINALIZING|XGE_DATA_FINALIZED|XGE_DATA_CLEARED|XGE_DATA_FINISHING|XGE_DATA_FINISHED|XGE_DATA_CONSUMING)) &&
        (!root->pType->onReadable || root->pType->onReadable(DO_FN(Body)(root)));
}
static int DO_FN(Admit)(const xge_data_lifetime_t* root)
{
    if(!DO_FN(Known)(root))return DO_STATUS(ERROR_INVALID_ARGUMENT);
    if(root->iCreator!=xrtThreadCurrentId())return DO_STATUS(ERROR_BUSY);
    if(root->pClaim || DO_FN(State)(root) || (root->pType->onReadable && !root->pType->onReadable(DO_FN(Body)(root))))return DO_STATUS(ERROR_INVALID_STATE);
    return DO_STATUS(OK);
}
static int DO_FN(Acquire)(xge_data_lifetime_t* root,bool creator,bool activity)
{
    if(!DO_FN(Known)(root))return DO_STATUS(ERROR_INVALID_ARGUMENT);
    if(creator && root->iCreator!=xrtThreadCurrentId())return DO_STATUS(ERROR_BUSY);
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return DO_STATUS(ERROR_INVALID_STATE);
    DO_FN(Lock)(root);int status=root->pClaim || DO_FN(State)(root) ||
        (root->pType->onReadable && !root->pType->onReadable(DO_FN(Body)(root))) ? DO_STATUS(ERROR_INVALID_STATE):DO_STATUS(OK);
    unsigned refs=atomic_load_explicit(DO_FN(Refs)(root),memory_order_relaxed);
    while(status==DO_STATUS(OK)){
        if(!refs || refs>=INT_MAX){status=DO_STATUS(ERROR_INVALID_STATE);break;}
        if(atomic_compare_exchange_weak_explicit(DO_FN(Refs)(root),&refs,refs+1u,memory_order_relaxed,memory_order_relaxed)){
            if(activity)atomic_fetch_add_explicit(&root->iActivity,1u,memory_order_release);
            break;
        }
    }
    DO_FN(Unlock)(root);DO_FN(MutationEnd)(&mutation);return status;
}
static int DO_FN(Ready)(const xge_data_lifetime_t* root)
{
    if(!DO_FN(Known)(root))return DO_STATUS(ERROR_INVALID_ARGUMENT);
    unsigned state=DO_FN(State)(root);if(state&XGE_DATA_FINISHED)return DO_STATUS(OK);
    if(root->iCreator!=xrtThreadCurrentId())return DO_STATUS(ERROR_BUSY);
    if(atomic_load_explicit(&root->iActivity,memory_order_acquire) || (state&(XGE_DATA_FINALIZING|XGE_DATA_FINISHING)))return DO_STATUS(ERROR_BUSY);
    return DO_STATUS(OK);
}
static int DO_FN(Finalize)(xge_data_lifetime_t* root,bool* complete)
{
    *complete=false;
    if(DO_FN(State)(root)&XGE_DATA_FINALIZED){*complete=true;return DO_STATUS(OK);}
    int status=DO_FN(Ready)(root);if(status!=DO_STATUS(OK))return status==DO_STATUS(ERROR_BUSY) ? DO_STATUS(STATUS_DEFERRED):status;
    xerror* ambient=xrtTakeError();xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation)){xrtSetErrorTake(ambient);return DO_STATUS(ERROR_INVALID_STATE);}
    atomic_fetch_or_explicit(&root->iState,XGE_DATA_FINALIZING,memory_order_release);DO_FN(MutationEnd)(&mutation);
    status=root->pType->onFinalize ? root->pType->onFinalize(DO_FN(Body)(root),complete):DO_STATUS(OK);
    if(!root->pType->onFinalize)*complete=true;
    if(*complete)atomic_fetch_or_explicit(&root->iState,XGE_DATA_FINALIZED,memory_order_release);
    atomic_fetch_and_explicit(&root->iState,~XGE_DATA_FINALIZING,memory_order_release);
    xge_data_outcome_t result={0};DO_FN(OutcomeBegin)(&result,status);return DO_FN(OutcomeEnd)(&result,ambient);
}
static int DO_FN(Finish)(xge_data_lifetime_t* root)
{
    unsigned state=DO_FN(State)(root);if(state&XGE_DATA_FINISHED)return DO_STATUS(OK);
    if(!(state&XGE_DATA_FINALIZED) || (state&XGE_DATA_FINISHING))return DO_STATUS(ERROR_BUSY);
    if(root->iCreator!=xrtThreadCurrentId())return DO_STATUS(ERROR_BUSY);
    xerror* ambient=xrtTakeError();xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation)){xrtSetErrorTake(ambient);return DO_STATUS(ERROR_INVALID_STATE);}
    atomic_fetch_or_explicit(&root->iState,XGE_DATA_FINISHING,memory_order_release);DO_FN(MutationEnd)(&mutation);
    int status=root->pType->onFinish ? root->pType->onFinish(DO_FN(Body)(root)):DO_STATUS(OK);
    atomic_fetch_or_explicit(&root->iState,XGE_DATA_FINISHED,memory_order_release);
    atomic_fetch_and_explicit(&root->iState,~XGE_DATA_FINISHING,memory_order_release);
    xge_data_outcome_t result={0};DO_FN(OutcomeBegin)(&result,status);return DO_FN(OutcomeEnd)(&result,ambient);
}
static int DO_FN(Release)(xge_data_lifetime_t* root,bool* consumed)
{
    if(!consumed)return DO_STATUS(ERROR_INVALID_ARGUMENT);
    *consumed=false;if(!DO_FN(Known)(root))return DO_STATUS(ERROR_INVALID_ARGUMENT);
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return DO_STATUS(ERROR_INVALID_STATE);
    DO_FN(Lock)(root);unsigned refs=atomic_load_explicit(DO_FN(Refs)(root),memory_order_relaxed);
    for(;;){
        if(!refs){DO_FN(Unlock)(root);DO_FN(MutationEnd)(&mutation);return DO_STATUS(ERROR_INVALID_STATE);}
        if(refs==1u)break;
        if(atomic_compare_exchange_weak_explicit(DO_FN(Refs)(root),&refs,refs-1u,memory_order_acq_rel,memory_order_relaxed)){
            *consumed=true;DO_FN(Unlock)(root);DO_FN(MutationEnd)(&mutation);return DO_STATUS(OK);
        }
    }
    unsigned state=DO_FN(State)(root);
    if((state&XGE_DATA_CONSUMING) && !(state&XGE_DATA_FINISHED)){
        DO_FN(Unlock)(root);DO_FN(MutationEnd)(&mutation);return DO_STATUS(ERROR_INVALID_STATE);
    }
    if(!(state&XGE_DATA_FINISHED) && root->iCreator!=xrtThreadCurrentId()){
        DO_FN(Unlock)(root);DO_FN(MutationEnd)(&mutation);return DO_STATUS(ERROR_BUSY);
    }
    if(DO_FN(Ready)(root)!=DO_STATUS(OK)){
        DO_FN(Unlock)(root);DO_FN(MutationEnd)(&mutation);return DO_STATUS(STATUS_DEFERRED);
    }
    atomic_fetch_or_explicit(&root->iState,XGE_DATA_CONSUMING,memory_order_release);
    DO_FN(Unlock)(root);DO_FN(MutationEnd)(&mutation);
    xerror* ambient=xrtTakeError();xge_data_outcome_t result={0};DO_FN(OutcomeBegin)(&result,DO_STATUS(OK));bool complete=false;
    DO_FN(OutcomeAdd)(&result,DO_FN(Finalize)(root,&complete));
    if(!complete){
        atomic_fetch_and_explicit(&root->iState,~XGE_DATA_CONSUMING,memory_order_release);
        if(result.iStatus==DO_STATUS(OK))result.iStatus=DO_STATUS(STATUS_DEFERRED);
        return DO_FN(OutcomeEnd)(&result,ambient);
    }
    DO_FN(OutcomeAdd)(&result,DO_FN(Finish)(root));
    if(!xrtOwnershipMutationBegin(&mutation))abort();
    refs=atomic_fetch_sub_explicit(DO_FN(Refs)(root),1u,memory_order_acq_rel);assert(refs);
    *consumed=true;DO_FN(MutationEnd)(&mutation);if(refs==1u)xrtFree(DO_FN(Body)(root));
    return DO_FN(OutcomeEnd)(&result,ambient);
}
static int DO_FN(ActivityEnd)(xge_data_lifetime_t* root)
{
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))abort();
    unsigned activity=atomic_fetch_sub_explicit(&root->iActivity,1u,memory_order_release);if(!activity)abort();
    DO_FN(MutationEnd)(&mutation);bool consumed=false;return DO_FN(Release)(root,&consumed);
}
static xrtownershipref DO_FN(Reference)(const xge_data_lifetime_t* root)
{return DO_FN(Known)(root) && atomic_load_explicit(DO_FN(Refs)(root),memory_order_acquire) ? (xrtownershipref){root,&DO_FN(Ops)}:(xrtownershipref){0};}
static bool DO_FN(Count)(const void* data,size_t* count)
{
    const xge_data_lifetime_t* root=data;if(!DO_FN(Known)(root) || !count)return false;
    *count=atomic_load_explicit(DO_FN(Refs)(root),memory_order_acquire);return true;
}
static bool DO_FN(Trace)(const void* data,xrtownershipvisitor visitor,void* user)
{
    const xge_data_lifetime_t* root=data;if(!DO_FN(Known)(root) || !visitor)return false;
    unsigned state=DO_FN(State)(root);if(state&(XGE_DATA_FINALIZING|XGE_DATA_FINISHING))return false;
    return (state&(XGE_DATA_CLEARED|XGE_DATA_FINISHED)) || !root->pType->onTrace || root->pType->onTrace(DO_FN(Body)(root),visitor,user);
}
static const xrtownershipops DO_FN(Ops)={DO_FN(Count),DO_FN(Trace)};
static bool DO_FN(Hold)(const void* data)
{
    xge_data_lifetime_t* root=(xge_data_lifetime_t*)data;
    if(!DO_FN(Known)(root) || root->pClaim || (DO_FN(State)(root)&(XGE_DATA_FINALIZING|XGE_DATA_FINISHING|XGE_DATA_CLEARED)))return false;
    unsigned refs=atomic_load_explicit(DO_FN(Refs)(root),memory_order_relaxed);
    do{if(!refs || refs>=INT_MAX)return false;}
    while(!atomic_compare_exchange_weak_explicit(DO_FN(Refs)(root),&refs,refs+1u,memory_order_relaxed,memory_order_relaxed));
    return true;
}
static bool DO_FN(DropChecked)(const void* data)
{bool consumed=false;return DO_FN(Release)((xge_data_lifetime_t*)data,&consumed)==DO_STATUS(OK);}
static void DO_FN(Drop)(const void* data){(void)DO_FN(DropChecked)(data);}
static bool DO_FN(Claim)(const void* data,const void* token)
{
    xge_data_lifetime_t* root=(xge_data_lifetime_t*)data;
    if(!token || !DO_FN(Known)(root) || root->pClaim || DO_FN(State)(root) || DO_FN(Ready)(root)!=DO_STATUS(OK))return false;
    root->pClaim=token;return true;
}
static void DO_FN(Restore)(const void* data,const void* token)
{xge_data_lifetime_t* root=(xge_data_lifetime_t*)data;assert(token && root->pClaim==token && !(DO_FN(State)(root)&XGE_DATA_CLEARED));root->pClaim=NULL;}
static bool DO_FN(FinalizeLegacy)(const void* data,const void* token)
{xge_data_lifetime_t* root=(xge_data_lifetime_t*)data;if(!token || root->pClaim!=token)return false;bool complete=false;return DO_FN(Finalize)(root,&complete)==DO_STATUS(OK) && complete;}
static void DO_FN(Clear)(const void* data,const void* token)
{xge_data_lifetime_t* root=(xge_data_lifetime_t*)data;assert(token && root->pClaim==token && (DO_FN(State)(root)&XGE_DATA_FINALIZED));atomic_fetch_or_explicit(&root->iState,XGE_DATA_CLEARED,memory_order_release);}
static bool DO_FN(FinishLegacy)(const void* data,const void* token)
{xge_data_lifetime_t* root=(xge_data_lifetime_t*)data;return token && root->pClaim==token && (DO_FN(State)(root)&XGE_DATA_CLEARED) && DO_FN(Finish)(root)==DO_STATUS(OK);}
static const xrtownershipadapterv1 DO_FN(Adapter)={sizeof(DO_FN(Adapter)),DO_FN(Hold),DO_FN(Drop),DO_FN(Claim),DO_FN(Restore),DO_FN(FinalizeLegacy),DO_FN(Clear),DO_FN(FinishLegacy)};
static bool DO_FN(PreparationReady)(const void* data){return DO_FN(Ready)(data)==DO_STATUS(OK);}
static xrtownershipprepareresult DO_FN(Prepare)(const void* data,const void* token)
{const xge_data_lifetime_t* root=data;if(!token || root->pClaim!=token)return XRT_OWNERSHIP_PREPARE_FAILED;return DO_FN(Ready)(root)==DO_STATUS(OK) ? XRT_OWNERSHIP_PREPARE_READY:XRT_OWNERSHIP_PREPARE_BUSY;}
static const xrtownershippreparationv1 DO_FN(Preparation)={sizeof(DO_FN(Preparation)),&DO_FN(Adapter),DO_FN(PreparationReady),DO_FN(Prepare)};
static bool DO_FN(Busy)(const void* data){return DO_FN(Ready)(data)!=DO_STATUS(OK);}
static const xrtownershipquiescencev1 DO_FN(Quiescence)={sizeof(DO_FN(Quiescence)),&DO_FN(Ops),DO_FN(Busy)};
static bool DO_FN(DropReady)(const void* data){return DO_FN(Ready)(data)==DO_STATUS(OK);}
static xrtownershipfinishresult DO_FN(CompletionStep)(const void* data,const void* token)
{
    const xge_data_lifetime_t* root=data;if(!DO_FN(DropReady)(data))return XRT_OWNERSHIP_FINISH_BUSY;
    if(!token || root->pClaim!=token || !(DO_FN(State)(root)&XGE_DATA_CLEARED))return XRT_OWNERSHIP_FINISH_FAILED;
    return DO_FN(Finish)((xge_data_lifetime_t*)root)==DO_STATUS(OK) ? XRT_OWNERSHIP_FINISH_READY:XRT_OWNERSHIP_FINISH_FAILED;
}
static const xrtownershipfinishv2 DO_FN(Completion)={{sizeof(DO_FN(Completion)),&DO_FN(Adapter),DO_FN(CompletionStep)},XRT_OWNERSHIP_FINISH_V2_TAG,DO_FN(DropChecked),DO_FN(DropReady)};
static bool DO_FN(CanFinalize)(const void* data,const void* token)
{const xge_data_lifetime_t* root=data;return token && root->pClaim==token && DO_FN(Ready)(root)==DO_STATUS(OK);}
static bool DO_FN(FinalizeStep)(const void* data,const void* token,bool* complete)
{
    if(!complete)return false;
    *complete=false;const xge_data_lifetime_t* root=data;if(!token || root->pClaim!=token)return false;
    int status=DO_FN(Finalize)((xge_data_lifetime_t*)root,complete);return status==DO_STATUS(OK) || (status==DO_STATUS(STATUS_DEFERRED) && !*complete);
}
static const xrtownershipfinalizationv1 DO_FN(Finalization)={sizeof(DO_FN(Finalization)),XRT_OWNERSHIP_FINALIZATION_V1_TAG,&DO_FN(Adapter),DO_FN(CanFinalize),DO_FN(FinalizeStep)};
static bool DO_FN(RefKnown)(xrtownershipref ref){return ref.Ops==&DO_FN(Ops) && DO_FN(Known)(ref.Data);}
DO_API xrtownershipref DO_PUBLIC(Ownership)(DO_HANDLE object){return DO_FN(Reference)(DO_ROOT(object));}
DO_API const xrtownershipadapterv1* DO_PUBLIC(OwnershipAdapterV1)(xrtownershipref ref){return DO_FN(RefKnown)(ref) ? &DO_FN(Adapter):NULL;}
DO_API const xrtownershippreparationv1* DO_PUBLIC(OwnershipPreparationV1)(xrtownershipref ref){return DO_FN(RefKnown)(ref) ? &DO_FN(Preparation):NULL;}
DO_API const xrtownershipquiescencev1* DO_PUBLIC(OwnershipQuiescenceV1)(xrtownershipref ref){return DO_FN(RefKnown)(ref) ? &DO_FN(Quiescence):NULL;}
DO_API const xrtownershipfinishv2* DO_PUBLIC(OwnershipCompletionV2)(xrtownershipref ref){return DO_FN(RefKnown)(ref) ? &DO_FN(Completion):NULL;}
DO_API const xrtownershipfinalizationv1* DO_PUBLIC(OwnershipFinalizationV1)(xrtownershipref ref){return DO_FN(RefKnown)(ref) ? &DO_FN(Finalization):NULL;}
DO_API bool DO_PUBLIC(OwnershipReadReady)(xrtownershipref ref)
{return DO_FN(RefKnown)(ref) && !(DO_FN(State)(ref.Data)&(XGE_DATA_FINALIZING|XGE_DATA_FINALIZED|XGE_DATA_CLEARED|XGE_DATA_FINISHING|XGE_DATA_FINISHED));}
