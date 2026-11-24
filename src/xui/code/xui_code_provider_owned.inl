/* Native ownership slots, not a language callback/output marshaller. */
static void __xuiCodeProviderStoreRaw(xui_code_provider_set providers,int role,xui_code_provider_fn_t fn,void* user)
{
    switch(role){
    case XUI_PROVIDER_COMPLETE:providers->callbacks.onComplete=fn.complete;providers->callbacks.pCompleteUser=user;break;
    case XUI_PROVIDER_HOVER:providers->callbacks.onHover=fn.hover;providers->callbacks.pHoverUser=user;break;
    case XUI_PROVIDER_SIGNATURE:providers->callbacks.onSignature=fn.signature;providers->callbacks.pSignatureUser=user;break;
    case XUI_PROVIDER_COMMAND:providers->callbacks.onCommand=fn.command;providers->callbacks.pCommandUser=user;break;
    case XUI_PROVIDER_ENABLED:providers->callbacks.onCommandEnabled=fn.enabled;providers->callbacks.pCommandEnabledUser=user;break;
    default:abort();
    }
}
static int __xuiCodeProviderSetRaw(xui_code_provider_set providers,int role,xui_code_provider_fn_t fn,void* user)
{
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    xui_code_provider_slot_t* old=NULL;xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
    else {
        old=providers->slots[role];providers->slots[role]=NULL;
        __xuiCodeProviderStoreRaw(providers,role,fn,user);
        if(!xrtOwnershipScopeEnd(&mutation))abort();
    }
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,status);
    __xuiCodeProviderRelease(scope.pContext,old,&receipt);
    xrtSetErrorTake(receipt.error);return __xuiCodeProviderFinish(&scope,receipt.status);
}
static int __xuiCodeProviderSetOwned(xui_code_provider_set providers,int role,xui_code_provider_fn_t fn,
    bool has,xui_code_provider_flavor flavor,void* user,const xui_callback_owner_t* incoming)
{
    if(!xuiInternalCallbackOwnerRawValid(incoming)||(!has&&incoming&&incoming->onHold))return XUI_ERROR_INVALID_ARGUMENT;
    xui_callback_owner_t owner={0};if(incoming)owner=*incoming;
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    xui_code_provider_slot_t* node=NULL;bool held=false;
    if(has){
        node=xrtCalloc(1,sizeof(*node));
        if(!node)status=XUI_ERROR_OUT_OF_MEMORY;
        else {node->owner=owner;node->user=user;node->fn=fn;node->flavor=flavor;status=xuiInternalCallbackHoldFresh(&owner,&held);}
    }
    if(status==XUI_OK)status=__xuiCodeProviderResultStatus(providers);
    xui_code_provider_slot_t* old=NULL;
    if(status==XUI_OK){
        xrtownershipscope mutation={0};
        if(!xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
        else {
            old=providers->slots[role];providers->slots[role]=node;
            if(node){node->refs=1;node=NULL;held=false;}
            __xuiCodeProviderStoreRaw(providers,role,(xui_code_provider_fn_t){0},NULL);
            if(!xrtOwnershipScopeEnd(&mutation))abort();
        }
    }
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,status);
    if(node){
        if(held)xuiInternalCleanupAdd(&receipt,xuiInternalCallbackOwnerDropOutcome(scope.pContext,&node->owner,receipt.status,receipt.error));
        xrtFree(node);
    }
    __xuiCodeProviderRelease(scope.pContext,old,&receipt);
    xrtSetErrorTake(receipt.error);return __xuiCodeProviderFinish(&scope,receipt.status);
}
XUI_API int xuiCodeProviderSetCompletionOwned(xui_code_provider_set p,xui_code_completion_proc fn,void* user,const xui_callback_owner_t* owner)
{return __xuiCodeProviderSetOwned(p,XUI_PROVIDER_COMPLETE,(xui_code_provider_fn_t){.complete=fn},fn!=NULL,XUI_PROVIDER_NATIVE,user,owner);}
XUI_API int xuiCodeProviderSetHoverOwned(xui_code_provider_set p,xui_code_hover_proc fn,void* user,const xui_callback_owner_t* owner)
{return __xuiCodeProviderSetOwned(p,XUI_PROVIDER_HOVER,(xui_code_provider_fn_t){.hover=fn},fn!=NULL,XUI_PROVIDER_NATIVE,user,owner);}
XUI_API int xuiCodeProviderSetSignatureOwned(xui_code_provider_set p,xui_code_signature_proc fn,void* user,const xui_callback_owner_t* owner)
{return __xuiCodeProviderSetOwned(p,XUI_PROVIDER_SIGNATURE,(xui_code_provider_fn_t){.signature=fn},fn!=NULL,XUI_PROVIDER_NATIVE,user,owner);}
XUI_API int xuiCodeProviderSetCommandOwned(xui_code_provider_set p,xui_code_command_proc fn,void* user,const xui_callback_owner_t* owner)
{return __xuiCodeProviderSetOwned(p,XUI_PROVIDER_COMMAND,(xui_code_provider_fn_t){.command=fn},fn!=NULL,XUI_PROVIDER_NATIVE,user,owner);}
XUI_API int xuiCodeProviderSetCommandEnabledOwned(xui_code_provider_set p,xui_code_command_enabled_proc fn,void* user,const xui_callback_owner_t* owner)
{return __xuiCodeProviderSetOwned(p,XUI_PROVIDER_ENABLED,(xui_code_provider_fn_t){.enabled=fn},fn!=NULL,XUI_PROVIDER_NATIVE,user,owner);}

XUI_API int xuiCodeProviderSetCompletionReplyOwned(xui_code_provider_set providers,
    xui_code_completion_reply_proc callback, void* user, const xui_callback_owner_t* owner)
{
    return __xuiCodeProviderSetOwned(providers, XUI_PROVIDER_COMPLETE,
        (xui_code_provider_fn_t){.completeReply = callback}, callback != NULL, XUI_PROVIDER_TYPED, user, owner);
}
XUI_API int xuiCodeProviderSetHoverReplyOwned(xui_code_provider_set providers,
    xui_code_hover_reply_proc callback, void* user, const xui_callback_owner_t* owner)
{
    return __xuiCodeProviderSetOwned(providers, XUI_PROVIDER_HOVER,
        (xui_code_provider_fn_t){.hoverReply = callback}, callback != NULL, XUI_PROVIDER_TYPED, user, owner);
}
XUI_API int xuiCodeProviderSetSignatureReplyOwned(xui_code_provider_set providers,
    xui_code_signature_reply_proc callback, void* user, const xui_callback_owner_t* owner)
{
    return __xuiCodeProviderSetOwned(providers, XUI_PROVIDER_SIGNATURE,
        (xui_code_provider_fn_t){.signatureReply = callback}, callback != NULL, XUI_PROVIDER_TYPED, user, owner);
}

XUI_API int xuiCodeProviderSetCommandHandlerOwned(xui_code_provider_set providers,
    xui_code_editor_command_proc callback, void* user, const xui_callback_owner_t* owner)
{
    return __xuiCodeProviderSetOwned(providers, XUI_PROVIDER_COMMAND,
        (xui_code_provider_fn_t){.editorCommand=callback}, callback != NULL, XUI_PROVIDER_TYPED, user, owner);
}

XUI_API int xuiCodeProviderSetValueCommandOwned(xui_code_provider_set providers,
    xui_code_value_command_proc callback, void* user, const xui_callback_owner_t* owner)
{
    return __xuiCodeProviderSetOwned(providers, XUI_PROVIDER_COMMAND,
        (xui_code_provider_fn_t){.valueCommand=callback}, callback != NULL, XUI_PROVIDER_VALUE_COMMAND, user, owner);
}

int xuiInternalCodeProviderSetCommandGuard(xui_code_provider_set providers,
    xui_code_editor_command_proc callback, void* user, const xui_callback_owner_t* owner)
{
    return __xuiCodeProviderSetOwned(providers, XUI_PROVIDER_ENABLED,
        (xui_code_provider_fn_t){.enabled=callback}, callback != NULL, XUI_PROVIDER_TYPED, user, owner);
}

bool xuiInternalCodeProviderHasCommandGuard(xui_code_provider_set providers)
{
    if (!providers) return false;
    xui_code_provider_slot_t* slot=providers->slots[XUI_PROVIDER_ENABLED];
    return slot && slot->flavor == XUI_PROVIDER_TYPED;
}

bool xuiInternalCodeProviderHasHover(xui_code_provider_set providers)
{
    return providers && (providers->slots[XUI_PROVIDER_HOVER] || providers->callbacks.onHover);
}

typedef struct xui_code_provider_borrow_t {
    xui_code_provider_scope_t* caller;
    uint64 generation;
} xui_code_provider_borrow_t;
static int __xuiCodeProviderBorrow(xui_code_provider_scope_t* request,int role,xui_code_provider_slot_t* selected,xui_code_provider_borrow_t* borrow)
{
    xui_code_provider_scope_t* caller=request->_pNext;
    if(!caller || caller->pSelf!=caller)return XUI_ERROR_INVALID_STATE;
    borrow->caller=caller;borrow->generation=caller->_iGeneration;
    xui_code_provider_slot_t* old=caller->_pSlots[role];
    if(old==selected)return XUI_OK;
    int status=__xuiCodeProviderRetain(selected);if(status!=XUI_OK)return status;
    xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation)){--selected->refs;return XUI_ERROR_INVALID_STATE;}
    caller->_pSlots[role]=selected;
    if(!xrtOwnershipScopeEnd(&mutation))abort();
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,XUI_OK);
    __xuiCodeProviderRelease(request->pContext,old,&receipt);
    return xuiInternalCleanupEnd(NULL,&receipt);
}
static bool __xuiCodeProviderBorrowValid(const xui_code_provider_borrow_t* borrow,xui_code_provider_set providers,int role,xui_code_provider_slot_t* selected)
{
    const xui_code_provider_scope_t* caller=borrow->caller;
    return caller && caller->pSelf==caller && caller->pProviders==providers &&
        caller->_iGeneration==borrow->generation && caller->_pSlots[role]==selected;
}
