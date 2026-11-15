/* The raw C provider remains externally synchronized, not newly thread-safe.
 * Quiescent standalone transfer is allowed; one active call chain belongs to
 * one thread. Language views retain their separate creator-thread contract. */
static int __xuiCodeProviderReadStatus(xui_code_provider_set providers)
{
    if(!providers)return XUI_ERROR_INVALID_ARGUMENT;
    if(providers->iMagic!=XUI_CODE_PROVIDER_MAGIC || providers->bDestroyPending || providers->bDestroying ||
       (providers->pInternalOwner && !xuiInternalWidgetIsValid(providers->pInternalOwner)))return XUI_ERROR_INVALID_STATE;
    if(providers->iActiveCalls && providers->iActiveThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    return XUI_OK;
}
/* A close request stops result publication, not quiescent graph inspection.
 * The Context still owns its resources until checked destruction consumes it. */
static int __xuiCodeProviderResultStatus(xui_code_provider_set providers)
{
    int status=__xuiCodeProviderReadStatus(providers);
    if(status==XUI_OK && providers->pInternalOwner && providers->pInternalOwner->pContext->bCloseRequested)
        status=XUI_ERROR_INVALID_STATE;
    return status;
}
static int __xuiCodeProviderDestroyNow(xui_code_provider_set providers)
{
    providers->bDestroying=true;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,XUI_OK);
    xuiInternalLifetimeInvalidateAll(&providers->pLifetimeLinks);xuiInternalCleanupAdd(&receipt,XUI_OK);
    xui_code_provider_slot_t* old[XUI_PROVIDER_ROLES]={0};xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))abort();
    for(int i=0;i<XUI_PROVIDER_ROLES;++i){old[i]=providers->slots[i];providers->slots[i]=NULL;}
    memset(&providers->callbacks,0,sizeof(providers->callbacks));
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    for(int i=0;i<XUI_PROVIDER_ROLES;++i)__xuiCodeProviderRelease(NULL,old[i],&receipt);
    providers->iMagic=0;xrtFree(providers);
    return xuiInternalCleanupEnd(NULL,&receipt);
}
static int __xuiCodeProviderDestroyOwnerChecked(xui_code_provider_set providers,xui_widget owner,bool* consumed)
{
    if(!consumed)return XUI_ERROR_INVALID_ARGUMENT;
    *consumed=false;
    if(!providers || providers->iMagic!=XUI_CODE_PROVIDER_MAGIC)return XUI_ERROR_INVALID_ARGUMENT;
    if(providers->bDestroying || (providers->iActiveCalls && providers->iActiveThread!=xrtThreadCurrentId()))return XUI_ERROR_BUSY;
    if(providers->bDestroyPending || providers->pInternalOwner!=owner)return XUI_ERROR_INVALID_STATE;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    providers->bDestroyPending=true;*consumed=true;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    return providers->iActiveCalls ? XUI_OK:__xuiCodeProviderDestroyNow(providers);
}
XUI_API int xuiCodeProviderSetDestroyOwnerChecked(xui_code_provider_set providers,bool* consumed)
{return __xuiCodeProviderDestroyOwnerChecked(providers,NULL,consumed);}
XUI_API int xuiCodeProviderSetDestroyChecked(xui_code_provider_set providers)
{bool consumed=false;return xuiCodeProviderSetDestroyOwnerChecked(providers,&consumed);}
int xuiInternalCodeProviderClaimOwner(xui_code_provider_set providers,xui_widget owner)
{
    if(!providers || !xuiInternalWidgetIsValid(owner))return XUI_ERROR_INVALID_ARGUMENT;
    int status=__xuiCodeProviderReadStatus(providers);if(status!=XUI_OK)return status;
    if(providers->iActiveCalls)return XUI_ERROR_BUSY;
    if(providers->pInternalOwner)return XUI_ERROR_INVALID_STATE;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    providers->pInternalOwner=owner;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    return XUI_OK;
}
int xuiInternalCodeProviderDestroyOwned(xui_widget owner,xui_code_provider_set providers)
{bool consumed=false;return __xuiCodeProviderDestroyOwnerChecked(providers,providers && providers->pInternalOwner ? owner:NULL,&consumed);}
XUI_API bool xuiCodeProviderSetOwnershipBusy(xui_code_provider_set providers)
{return !providers || providers->iMagic!=XUI_CODE_PROVIDER_MAGIC || providers->bDestroying || providers->iActiveCalls!=0;}
XUI_API bool xuiCodeProviderSetTraceOwnership(xui_code_provider_set providers,xrtownershipvisitor visit,void* user)
{
    if(!visit || __xuiCodeProviderReadStatus(providers)!=XUI_OK || providers->iActiveCalls)return false;
    for(int i=0;i<XUI_PROVIDER_ROLES;++i){
        const xui_code_provider_slot_t* node=providers->slots[i];
        if(node && node->owner.onHold && !visit(node->owner.tReference,user))return false;
    }
    return true;
}
XUI_API int xuiCodeProviderSetLifetimeWatch(xui_code_provider_set providers,xui_lifetime_link_t* link,xui_lifetime_invalidate_proc invalidated,void* user)
{
    int status=__xuiCodeProviderReadStatus(providers);if(status!=XUI_OK)return status;
    if(!link || !invalidated)return XUI_ERROR_INVALID_ARGUMENT;
    if(link->pNext || link->ppPrevious || link->onInvalidate || link->pUser)return XUI_ERROR_INVALID_STATE;
    link->pNext=providers->pLifetimeLinks;link->ppPrevious=&providers->pLifetimeLinks;link->onInvalidate=invalidated;link->pUser=user;
    if(link->pNext)link->pNext->ppPrevious=&link->pNext;
    providers->pLifetimeLinks=link;return XUI_OK;
}
XUI_API int xuiCodeProviderSetCallBegin(xui_code_provider_set providers,xui_code_provider_scope_t* scope)
{
    if(!scope || scope->pSelf || scope->pProviders || scope->pContext || scope->tContext.pSelf ||
       scope->_pPrevious || scope->_pNext || scope->_iGeneration)return XUI_ERROR_INVALID_ARGUMENT;
    for(int i=0;i<XUI_PROVIDER_ROLES;++i)if(scope->_pSlots[i])return XUI_ERROR_INVALID_ARGUMENT;
    int status=__xuiCodeProviderReadStatus(providers);if(status!=XUI_OK)return status;
    if(providers->iActiveCalls==SIZE_MAX || providers->nextScopeGeneration==UINT64_MAX)return XUI_ERROR_LIMIT_EXCEEDED;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    int pinned=0;
    for(;pinned<XUI_PROVIDER_ROLES;++pinned){
        status=__xuiCodeProviderRetain(providers->slots[pinned]);
        if(status!=XUI_OK)break;
        scope->_pSlots[pinned]=providers->slots[pinned];
    }
    xui_context context=providers->pInternalOwner ? providers->pInternalOwner->pContext:NULL;
    if(status==XUI_OK){
        if(context)status=xuiCallScopeBegin(context,&scope->tContext);
        else scope->tContext._pFirstError=xrtTakeError();
    }
    if(status==XUI_OK){
        ++providers->iActiveCalls;providers->iActiveThread=xrtThreadCurrentId();
        scope->pSelf=scope;scope->pProviders=providers;scope->pContext=context;
        scope->_iGeneration=++providers->nextScopeGeneration;
        scope->_pNext=providers->scopes;
        if(scope->_pNext)scope->_pNext->_pPrevious=scope;
        providers->scopes=scope;
    }else {
        /* Published slots still own a reference; this rollback cannot Drop. */
        for(int i=0;i<pinned;++i){
            xui_code_provider_slot_t* node=scope->_pSlots[i];
            if(node)--node->refs;
            scope->_pSlots[i]=NULL;
        }
    }
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    return status;
}
static int __xuiCodeProviderFinish(xui_code_provider_scope_t* scope,int primary)
{
    if(!scope || scope->pSelf!=scope || !scope->pProviders)return XUI_ERROR_INVALID_ARGUMENT;
    xui_code_provider_set providers=scope->pProviders;
    if(providers->iActiveThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,primary);
    xerror* ambient=scope->pContext ? NULL:scope->tContext._pFirstError;
    if(!scope->pContext)scope->tContext._pFirstError=NULL;
    scope->pSelf=NULL;scope->_iGeneration=0;
    if(scope->_pPrevious)scope->_pPrevious->_pNext=scope->_pNext;
    else providers->scopes=scope->_pNext;
    if(scope->_pNext)scope->_pNext->_pPrevious=scope->_pPrevious;
    scope->_pPrevious=NULL;scope->_pNext=NULL;
    for(int i=0;i<XUI_PROVIDER_ROLES;++i){
        xui_code_provider_slot_t* node=scope->_pSlots[i];scope->_pSlots[i]=NULL;
        __xuiCodeProviderRelease(scope->pContext,node,&receipt);
    }
    if(providers->bDestroyPending && receipt.status==XUI_OK)receipt.status=XUI_ERROR_INVALID_STATE;
    if(scope->pContext)receipt.status=xuiInternalCallScopeEndOutcome(&scope->tContext,receipt.status,&receipt.error);
    if(providers->bDestroyPending)xuiInternalCleanupAdd(&receipt,XUI_ERROR_INVALID_STATE);
    scope->pSelf=NULL;scope->pProviders=NULL;scope->pContext=NULL;
    if(!providers->iActiveCalls)abort();
    --providers->iActiveCalls;
    if(!providers->iActiveCalls){
        providers->iActiveThread=0;
        if(providers->bDestroyPending)xuiInternalCleanupAdd(&receipt,__xuiCodeProviderDestroyNow(providers));
    }
    /* The prior diagnostic remains first, independently of the new status. */
    if(ambient){xrtErrorFree(receipt.error);receipt.error=ambient;}
    return xuiInternalCleanupEnd(NULL,&receipt);
}
XUI_API int xuiCodeProviderSetCallEnd(xui_code_provider_scope_t* scope)
{return __xuiCodeProviderFinish(scope,XUI_OK);}
int xuiInternalCodeProviderCallEndRestored(xui_code_provider_scope_t* scope,int status,xerror* error)
{xrtSetErrorTake(error);return __xuiCodeProviderFinish(scope,status);}
static void __xuiCodeProviderCallbackBegin(xui_code_provider_scope_t* scope)
{xrtSetErrorTake(xrtErrorRef(scope->tContext._pFirstError));}
static void __xuiCodeProviderCallbackEnd(xui_code_provider_scope_t* scope)
{
    xerror* fresh=xrtTakeError();
    if(fresh==scope->tContext._pFirstError){xrtErrorFree(fresh);fresh=NULL;}
    xrtSetErrorTake(fresh);
}
