#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui.h"
#include "xui_code_provider_internal.h"
#include "../core/xui_internal.h"
#include "../core/xui_callback_hold_internal.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct xui_code_provider_callbacks_t {
	xui_code_completion_proc onComplete;
	void* pCompleteUser;
	xui_code_hover_proc onHover;
	void* pHoverUser;
	xui_code_signature_proc onSignature;
	void* pSignatureUser;
	xui_code_command_proc onCommand;
	void* pCommandUser;
	xui_code_command_enabled_proc onCommandEnabled;
	void* pCommandEnabledUser;
} xui_code_provider_callbacks_t;
enum { XUI_PROVIDER_COMPLETE, XUI_PROVIDER_HOVER, XUI_PROVIDER_SIGNATURE,
    XUI_PROVIDER_COMMAND, XUI_PROVIDER_ENABLED, XUI_PROVIDER_ROLES };
typedef union xui_code_provider_fn_t {
    xui_code_completion_proc complete;
    xui_code_hover_proc hover;
    xui_code_signature_proc signature;
    xui_code_completion_reply_proc completeReply;
    xui_code_hover_reply_proc hoverReply;
    xui_code_signature_reply_proc signatureReply;
    xui_code_command_proc command;
    xui_code_command_enabled_proc enabled;
} xui_code_provider_fn_t;
typedef struct xui_code_provider_slot_t {
    size_t refs;
    xui_callback_owner_t owner;
    void* user;
    xui_code_provider_fn_t fn;
    bool reply;
} xui_code_provider_slot_t;
struct xui_code_provider_set_t {
    xui_code_provider_callbacks_t callbacks;
    xui_code_provider_slot_t* slots[XUI_PROVIDER_ROLES];
    struct xui_code_provider_reply_block_t* replies;
    xui_code_provider_scope_t* scopes;
    uint64 nextScopeGeneration;
    uint32_t iMagic;
    bool bDestroyPending,bDestroying;
    uint64_t iActiveThread;
    size_t iActiveCalls;
    xui_lifetime_link_t* pLifetimeLinks;
    xui_widget pInternalOwner;
};
#define XUI_CODE_PROVIDER_MAGIC UINT32_C(0x58435052)
static int __xuiCodeProviderRetain(xui_code_provider_slot_t* node)
{
    if(!node)return XUI_OK;
    if(!node->refs)return XUI_ERROR_INVALID_STATE;
    if(node->refs==SIZE_MAX)return XUI_ERROR_LIMIT_EXCEEDED;
    ++node->refs;return XUI_OK;
}
static void __xuiCodeProviderRelease(xui_context context,xui_code_provider_slot_t* node,xui_cleanup_receipt_t* receipt)
{
    if(!node)return;
    if(!node->refs)abort();
    if(--node->refs)return;
    xui_callback_owner_t owner=node->owner;xrtFree(node);
    xuiInternalCleanupAdd(receipt,xuiInternalCallbackOwnerDropOutcome(context,&owner,receipt->status,receipt->error));
}
#include "xui_code_provider_reply.inl"
#include "xui_code_provider_lifetime.inl"
#include "xui_code_provider_owned.inl"

static void __xuiCodeProviderCompletionItemsReset(
	xui_code_completion_item_t* pItems, int iItemCapacity)
{
	int i;

	if ( pItems == NULL || iItemCapacity <= 0 ) return;
	memset(pItems, 0, sizeof(*pItems) * (size_t)iItemCapacity);
	for ( i = 0; i < iItemCapacity; i++ ) {
		pItems[i].iSize = sizeof(pItems[i]);
	}
}

static int __xuiCodeProviderCompletionItemsValid(
	const xui_code_completion_item_t* pItems, int iItemCount)
{
	size_t iRequiredSize = offsetof(xui_code_completion_item_t, iUserData) +
		sizeof(pItems[0].iUserData);
	int i;

	for ( i = 0; i < iItemCount; i++ ) {
		if ( pItems[i].iSize < iRequiredSize || pItems[i].sLabel == NULL ||
		     pItems[i].sLabel[0] == '\0' ) {
			return 0;
		}
	}
	return 1;
}

XUI_API int xuiCodeProviderSetCreate(xui_code_provider_set* ppProviders)
{
    if(!ppProviders)return XUI_ERROR_INVALID_ARGUMENT;
    *ppProviders=NULL;
    xui_code_provider_set providers=xrtCalloc(1,sizeof(*providers));
    if(!providers)return XUI_ERROR_OUT_OF_MEMORY;
    providers->iMagic=XUI_CODE_PROVIDER_MAGIC;*ppProviders=providers;return XUI_OK;
}
XUI_API void xuiCodeProviderSetDestroy(xui_code_provider_set providers)
{if(providers)(void)xuiCodeProviderSetDestroyChecked(providers);}
XUI_API int xuiCodeProviderSetClearChecked(xui_code_provider_set providers)
{
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    xui_code_provider_slot_t* old[XUI_PROVIDER_ROLES]={0};xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
    else {
        memset(&providers->callbacks,0,sizeof(providers->callbacks));
        for(int i=0;i<XUI_PROVIDER_ROLES;++i){old[i]=providers->slots[i];providers->slots[i]=NULL;}
        if(!xrtOwnershipScopeEnd(&mutation))abort();
    }
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,status);
    for(int i=0;i<XUI_PROVIDER_ROLES;++i)__xuiCodeProviderRelease(scope.pContext,old[i],&receipt);
    xrtSetErrorTake(receipt.error);return __xuiCodeProviderFinish(&scope,receipt.status);
}
XUI_API void xuiCodeProviderSetClear(xui_code_provider_set providers)
{if(providers)(void)xuiCodeProviderSetClearChecked(providers);}
XUI_API int xuiCodeProviderSetCompletion(xui_code_provider_set providers,xui_code_completion_proc callback,void* user)
{return __xuiCodeProviderSetRaw(providers,XUI_PROVIDER_COMPLETE,(xui_code_provider_fn_t){.complete=callback},user);}
XUI_API int xuiCodeProviderSetHover(xui_code_provider_set providers,xui_code_hover_proc callback,void* user)
{return __xuiCodeProviderSetRaw(providers,XUI_PROVIDER_HOVER,(xui_code_provider_fn_t){.hover=callback},user);}
XUI_API int xuiCodeProviderSetSignature(xui_code_provider_set providers,xui_code_signature_proc callback,void* user)
{return __xuiCodeProviderSetRaw(providers,XUI_PROVIDER_SIGNATURE,(xui_code_provider_fn_t){.signature=callback},user);}
XUI_API int xuiCodeProviderSetCommand(xui_code_provider_set providers,xui_code_command_proc callback,void* user)
{return __xuiCodeProviderSetRaw(providers,XUI_PROVIDER_COMMAND,(xui_code_provider_fn_t){.command=callback},user);}
XUI_API int xuiCodeProviderSetCommandEnabled(xui_code_provider_set providers,xui_code_command_enabled_proc callback,void* user)
{return __xuiCodeProviderSetRaw(providers,XUI_PROVIDER_ENABLED,(xui_code_provider_fn_t){.enabled=callback},user);}
XUI_API int xuiCodeProviderRequestCompletion(xui_code_provider_set providers,xui_widget widget,int offset,const char* prefix,xui_code_completion_item_t* items,int capacity,int* count)
{
    if(!providers || !count || offset<0 || capacity<0 || (!items && capacity))return XUI_ERROR_INVALID_ARGUMENT;
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    *count=0;__xuiCodeProviderCompletionItemsReset(items,capacity);
    xui_code_completion_proc callback=providers->callbacks.onComplete;void* user=providers->callbacks.pCompleteUser;
    xui_code_provider_slot_t* selected=providers->slots[XUI_PROVIDER_COMPLETE];
    bool reply = selected && selected->reply;
    if(selected){callback=reply ? NULL : selected->fn.complete;user=selected->user;}
    xui_code_provider_borrow_t borrow={0};
    if(selected)status=__xuiCodeProviderBorrow(&scope,XUI_PROVIDER_COMPLETE,selected,&borrow);
    if(status==XUI_OK)status=__xuiCodeProviderResultStatus(providers);
    if(status==XUI_OK && selected && !__xuiCodeProviderBorrowValid(&borrow,providers,XUI_PROVIDER_COMPLETE,selected))status=XUI_ERROR_INVALID_STATE;
    if(status==XUI_OK && !callback && !reply)status=XUI_ERROR_UNSUPPORTED;
    xui_code_completion_reply_t response={.items=items,.prefix=prefix ? prefix : "",.capacity=capacity};
    if(status==XUI_OK){
        __xuiCodeProviderCallbackBegin(&scope);
        if (reply) {
            status=selected->fn.completeReply(widget,offset,&response,user);
            *count=response.count;
        } else {
            status=callback(widget,offset,prefix?prefix:"",items,capacity,count,user);
        }
        __xuiCodeProviderCallbackEnd(&scope);
        if(status==XUI_OK)status=__xuiCodeProviderResultStatus(providers);
        if(status==XUI_OK && (*count<0 || *count>capacity || !__xuiCodeProviderCompletionItemsValid(items,*count)))status=XUI_ERROR_INVALID_STATE;
    }
    if(status==XUI_OK && selected && !__xuiCodeProviderBorrowValid(&borrow,providers,XUI_PROVIDER_COMPLETE,selected))status=XUI_ERROR_INVALID_STATE;
    status=__xuiCodeProviderFinish(&scope,status);
    if(status==XUI_OK && selected && !__xuiCodeProviderBorrowValid(&borrow,providers,XUI_PROVIDER_COMPLETE,selected))status=XUI_ERROR_INVALID_STATE;
    if (reply && status==XUI_OK)
        __xuiCodeProviderSaveReply(providers,borrow.caller,XUI_PROVIDER_COMPLETE,response.blocks);
    else
        __xuiCodeReplyFree(response.blocks);
    if(status!=XUI_OK){*count=0;__xuiCodeProviderCompletionItemsReset(items,capacity);}
    return status;
}
XUI_API int xuiCodeProviderRequestHover(xui_code_provider_set providers,xui_widget widget,int offset,xui_code_hover_t* output)
{
    if(!providers || !output || offset<0)return XUI_ERROR_INVALID_ARGUMENT;
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    memset(output,0,sizeof(*output));output->iSize=sizeof(*output);
    xui_code_hover_t value={0};value.iSize=sizeof(value);
    xui_code_hover_proc callback=providers->callbacks.onHover;void* user=providers->callbacks.pHoverUser;
    xui_code_provider_slot_t* selected=providers->slots[XUI_PROVIDER_HOVER];
    bool reply = selected && selected->reply;
    if(selected){callback=reply ? NULL : selected->fn.hover;user=selected->user;}
    xui_code_provider_borrow_t borrow={0};
    if(selected)status=__xuiCodeProviderBorrow(&scope,XUI_PROVIDER_HOVER,selected,&borrow);
    if(status==XUI_OK)status=__xuiCodeProviderResultStatus(providers);
    if(status==XUI_OK && selected && !__xuiCodeProviderBorrowValid(&borrow,providers,XUI_PROVIDER_HOVER,selected))status=XUI_ERROR_INVALID_STATE;
    if(status==XUI_OK && !callback && !reply)status=XUI_ERROR_UNSUPPORTED;
    xui_code_hover_reply_t response={0};
    if(status==XUI_OK){
        __xuiCodeProviderCallbackBegin(&scope);
        if (reply) {
            status=selected->fn.hoverReply(widget,offset,&response,user);
            value=response.value;
        } else {
            status=callback(widget,offset,&value,user);
        }
        __xuiCodeProviderCallbackEnd(&scope);
        if(status==XUI_OK)status=__xuiCodeProviderResultStatus(providers);
        if(status==XUI_OK && !xuiInternalCodeProviderHoverValid(&value))status=XUI_ERROR_INVALID_STATE;
    }
    if(status==XUI_OK && selected && !__xuiCodeProviderBorrowValid(&borrow,providers,XUI_PROVIDER_HOVER,selected))status=XUI_ERROR_INVALID_STATE;
    status=__xuiCodeProviderFinish(&scope,status);
    if(status==XUI_OK && selected && !__xuiCodeProviderBorrowValid(&borrow,providers,XUI_PROVIDER_HOVER,selected))status=XUI_ERROR_INVALID_STATE;
    if (reply && status==XUI_OK)
        __xuiCodeProviderSaveReply(providers,borrow.caller,XUI_PROVIDER_HOVER,response.block);
    else
        __xuiCodeReplyFree(response.block);
    if(status==XUI_OK){value.iSize=sizeof(value);*output=value;}
    return status;
}
XUI_API int xuiCodeProviderRequestSignature(xui_code_provider_set providers,xui_widget widget,int offset,xui_code_signature_help_t* output)
{
    if(!providers || !output || offset<0)return XUI_ERROR_INVALID_ARGUMENT;
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    memset(output,0,sizeof(*output));output->iSize=sizeof(*output);output->iActiveParameter=-1;
    xui_code_signature_help_t value={0};value.iSize=sizeof(value);value.iActiveParameter=-1;
    xui_code_signature_proc callback=providers->callbacks.onSignature;void* user=providers->callbacks.pSignatureUser;
    xui_code_provider_slot_t* selected=providers->slots[XUI_PROVIDER_SIGNATURE];
    bool reply = selected && selected->reply;
    if(selected){callback=reply ? NULL : selected->fn.signature;user=selected->user;}
    xui_code_provider_borrow_t borrow={0};
    if(selected)status=__xuiCodeProviderBorrow(&scope,XUI_PROVIDER_SIGNATURE,selected,&borrow);
    if(status==XUI_OK)status=__xuiCodeProviderResultStatus(providers);
    if(status==XUI_OK && selected && !__xuiCodeProviderBorrowValid(&borrow,providers,XUI_PROVIDER_SIGNATURE,selected))status=XUI_ERROR_INVALID_STATE;
    if(status==XUI_OK && !callback && !reply)status=XUI_ERROR_UNSUPPORTED;
    xui_code_signature_reply_t response={0};
    if(status==XUI_OK){
        __xuiCodeProviderCallbackBegin(&scope);
        if (reply) {
            status=selected->fn.signatureReply(widget,offset,&response,user);
            value=response.value;
        } else {
            status=callback(widget,offset,&value,user);
        }
        __xuiCodeProviderCallbackEnd(&scope);
        if(status==XUI_OK)status=__xuiCodeProviderResultStatus(providers);
        if(status==XUI_OK && !xuiInternalCodeProviderSignatureValid(&value))status=XUI_ERROR_INVALID_STATE;
    }
    if(status==XUI_OK && selected && !__xuiCodeProviderBorrowValid(&borrow,providers,XUI_PROVIDER_SIGNATURE,selected))status=XUI_ERROR_INVALID_STATE;
    status=__xuiCodeProviderFinish(&scope,status);
    if(status==XUI_OK && selected && !__xuiCodeProviderBorrowValid(&borrow,providers,XUI_PROVIDER_SIGNATURE,selected))status=XUI_ERROR_INVALID_STATE;
    if (reply && status==XUI_OK)
        __xuiCodeProviderSaveReply(providers,borrow.caller,XUI_PROVIDER_SIGNATURE,response.block);
    else
        __xuiCodeReplyFree(response.block);
    if(status==XUI_OK){value.iSize=sizeof(value);*output=value;}
    return status;
}
XUI_API int xuiCodeProviderExecuteCommand(xui_code_provider_set providers,xui_widget widget,int command,const void* data,int* handled)
{
    if(!providers || !handled || command<=0)return XUI_ERROR_INVALID_ARGUMENT;
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    *handled=0;int value=0;
    xui_code_command_proc callback=providers->callbacks.onCommand;void* user=providers->callbacks.pCommandUser;
    xui_code_provider_slot_t* selected=providers->slots[XUI_PROVIDER_COMMAND];
    if(selected){callback=selected->fn.command;user=selected->user;}
    if(status==XUI_OK && !callback)status=XUI_ERROR_UNSUPPORTED;
    if(status==XUI_OK){
        __xuiCodeProviderCallbackBegin(&scope);status=callback(widget,command,data,&value,user);__xuiCodeProviderCallbackEnd(&scope);
        if(status==XUI_OK)status=__xuiCodeProviderResultStatus(providers);
    }
    status=__xuiCodeProviderFinish(&scope,status);if(status==XUI_OK)*handled=value?1:0;return status;
}
XUI_API int xuiCodeProviderCanExecuteCommand(xui_code_provider_set providers,xui_widget widget,int command,int* enabled)
{
    if(!providers || !enabled || command<=0)return XUI_ERROR_INVALID_ARGUMENT;
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    *enabled=0;int value=0;
    xui_code_command_enabled_proc callback=providers->callbacks.onCommandEnabled;void* user=providers->callbacks.pCommandEnabledUser;
    xui_code_provider_slot_t* selected=providers->slots[XUI_PROVIDER_ENABLED];
    if(selected){callback=selected->fn.enabled;user=selected->user;}
    if(status==XUI_OK && !callback)status=XUI_ERROR_UNSUPPORTED;
    if(status==XUI_OK){
        __xuiCodeProviderCallbackBegin(&scope);status=callback(widget,command,&value,user);__xuiCodeProviderCallbackEnd(&scope);
        if(status==XUI_OK)status=__xuiCodeProviderResultStatus(providers);
    }
    status=__xuiCodeProviderFinish(&scope,status);if(status==XUI_OK)*enabled=value?1:0;return status;
}
#endif
