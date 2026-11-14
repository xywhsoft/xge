/* Real unique-Document pins and owned callbacks. No business-method wrapper,
 * RC layer, callback registry or hot allocation is introduced. */
static int __xuiCodeDocumentReadStatus(xui_code_document document)
{
    if(!document)return XUI_ERROR_INVALID_ARGUMENT;
    if(document->iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    if(document->iMagic!=XUI_CODE_DOCUMENT_MAGIC || document->bDestroyPending || document->bDestroying ||
        (document->pInternalOwner && !xuiInternalWidgetIsValid(document->pInternalOwner)))return XUI_ERROR_INVALID_STATE;
    return XUI_OK;
}
XUI_API int xuiCodeDocumentCallBegin(xui_code_document document,xui_code_document_scope_t* scope)
{
    if(!scope || scope->pSelf || scope->pDocument || scope->pContext || scope->tContext.pSelf)return XUI_ERROR_INVALID_ARGUMENT;
    int status=__xuiCodeDocumentReadStatus(document);if(status!=XUI_OK)return status;
    if(document->iActiveCalls==SIZE_MAX)return XUI_ERROR_INVALID_STATE;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    xui_context context=document->pObserverContext ? document->pObserverContext:(document->pInternalOwner ? document->pInternalOwner->pContext:NULL);
    if(context)status=xuiCallScopeBegin(context,&scope->tContext);
    if(status==XUI_OK){++document->iActiveCalls;scope->pDocument=document;scope->pContext=context;scope->pSelf=scope;}
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    return status;
}
XUI_API int xuiCodeDocumentCallEnd(xui_code_document_scope_t* scope)
{
    if(!scope || scope->pSelf!=scope || !scope->pDocument)return XUI_ERROR_INVALID_ARGUMENT;
    xui_code_document document=scope->pDocument;
    if(document->iCreatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    int status=document->bDestroyPending ? XUI_ERROR_INVALID_STATE:XUI_OK;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,status);
    receipt.status=xuiInternalCallScopeEndOutcome(&scope->tContext,receipt.status,&receipt.error);
    scope->pSelf=NULL;scope->pDocument=NULL;scope->pContext=NULL;
    if(!document->iActiveCalls)abort();
    --document->iActiveCalls;
    if(document->bDestroyPending && !document->iActiveCalls)xuiInternalCleanupAdd(&receipt,__xuiCodeDocumentDestroyNow(document));
    return xuiInternalCleanupEnd(NULL,&receipt);
}
XUI_API bool xuiCodeDocumentTraceOwnership(xui_code_document document,xrtownershipvisitor visit,void* user)
{
    if(!visit || __xuiCodeDocumentReadStatus(document)!=XUI_OK || document->iActiveCalls)return false;
    return !document->tChangeOwner.onHold || visit(document->tChangeOwner.tReference,user);
}
static int __xuiCodeDocumentDropChangeOwner(const xui_callback_owner_t* owner,int status)
{
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,status);
    if(owner->onDrop){
        int next=owner->onDrop(owner->tReference.Data) ? XUI_OK:XUI_ERROR_CALLBACK_FAILED;
        xuiInternalCleanupAdd(&receipt,next);
    }
    return xuiInternalCleanupEnd(NULL,&receipt);
}
static int __xuiCodeDocumentClearChange(xui_code_document document)
{
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    xui_callback_owner_t previous=document->tChangeOwner;document->tChangeOwner=(xui_callback_owner_t){0};
    document->onChange=NULL;document->pChangeUser=NULL;if(!xrtOwnershipScopeEnd(&mutation))abort();
    return __xuiCodeDocumentDropChangeOwner(&previous,XUI_OK);
}
XUI_API int xuiCodeDocumentSetChangeCallbackOwned(xui_code_document document,xui_code_document_change_proc callback,
    void* user,const xui_callback_owner_t* incoming)
{
    if(!xuiInternalCallbackOwnerRawValid(incoming) || (!callback && incoming && incoming->onHold))return XUI_ERROR_INVALID_ARGUMENT;
    xui_callback_owner_t candidate=incoming ? *incoming:(xui_callback_owner_t){0};
    xui_code_document_scope_t scope={0};int status=xuiCodeDocumentCallBegin(document,&scope);if(status!=XUI_OK)return status;
    bool held=false;status=xuiInternalCallbackHoldFresh(&candidate,&held);
    if(status==XUI_OK)status=__xuiCodeDocumentReadStatus(document);
    if(status==XUI_OK && scope.pContext && (scope.pContext->bCloseRequested || scope.pContext->bDestroyPending || scope.pContext->bDestroying))status=XUI_ERROR_INVALID_STATE;
    xrtownershipscope mutation={0};
    if(status==XUI_OK && !xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
    xui_callback_owner_t previous={0};
    if(status==XUI_OK){
        previous=document->tChangeOwner;document->tChangeOwner=candidate;document->onChange=callback;document->pChangeUser=user;
        if(!xrtOwnershipScopeEnd(&mutation))abort();
        held=false;
    }else if(held)previous=candidate;
    status=__xuiCodeDocumentDropChangeOwner(&previous,status);
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,status);
    xuiInternalCleanupAdd(&receipt,xuiCodeDocumentCallEnd(&scope));return xuiInternalCleanupEnd(NULL,&receipt);
}
static int __xuiCodeDocumentDispatchChange(xui_code_document document,xui_code_range_t range,uint32_t version)
{
    xui_code_document_change_proc selected=document->onChange;void* user=document->pChangeUser;
    xui_callback_owner_t owner=document->tChangeOwner;bool held=false;
    int status=xuiInternalCallbackHoldIsolated(&owner,&held);
    if(status==XUI_OK)status=__xuiCodeDocumentReadStatus(document);
    if(status==XUI_OK && selected)selected(document,range,version,user);
    return __xuiCodeDocumentDropChangeOwner(held ? &owner:&(xui_callback_owner_t){0},status);
}
