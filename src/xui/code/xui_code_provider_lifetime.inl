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
    (void)user;
    return visit && __xuiCodeProviderReadStatus(providers)==XUI_OK && providers->iActiveCalls==0;
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
    if(!scope || scope->pSelf || scope->pProviders || scope->pContext || scope->tContext.pSelf)return XUI_ERROR_INVALID_ARGUMENT;
    int status=__xuiCodeProviderReadStatus(providers);if(status!=XUI_OK)return status;
    if(providers->iActiveCalls==SIZE_MAX)return XUI_ERROR_INVALID_STATE;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    xui_context context=providers->pInternalOwner ? providers->pInternalOwner->pContext:NULL;
    if(context)status=xuiCallScopeBegin(context,&scope->tContext);
    else scope->tContext._pFirstError=xrtTakeError();
    if(status==XUI_OK){
        ++providers->iActiveCalls;providers->iActiveThread=xrtThreadCurrentId();
        scope->pSelf=scope;scope->pProviders=providers;scope->pContext=context;
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
static void __xuiCodeProviderCallbackBegin(xui_code_provider_scope_t* scope)
{xrtSetErrorTake(xrtErrorRef(scope->tContext._pFirstError));}
static void __xuiCodeProviderCallbackEnd(xui_code_provider_scope_t* scope)
{
    xerror* fresh=xrtTakeError();
    if(fresh==scope->tContext._pFirstError){xrtErrorFree(fresh);fresh=NULL;}
    xrtSetErrorTake(fresh);
}
