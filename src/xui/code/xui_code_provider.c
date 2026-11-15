#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui.h"
#include "xui_code_provider_internal.h"
#include "../core/xui_internal.h"

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
struct xui_code_provider_set_t {
    xui_code_provider_callbacks_t callbacks;
    uint32_t iMagic;
    bool bDestroyPending,bDestroying;
    uint64_t iActiveThread;
    size_t iActiveCalls;
    xui_lifetime_link_t* pLifetimeLinks;
    xui_widget pInternalOwner;
};
#define XUI_CODE_PROVIDER_MAGIC UINT32_C(0x58435052)
#include "xui_code_provider_lifetime.inl"

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
    xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
    else {memset(&providers->callbacks,0,sizeof(providers->callbacks));if(!xrtOwnershipScopeEnd(&mutation))abort();}
    return __xuiCodeProviderFinish(&scope,status);
}
XUI_API void xuiCodeProviderSetClear(xui_code_provider_set providers)
{if(providers)(void)xuiCodeProviderSetClearChecked(providers);}
XUI_API int xuiCodeProviderSetCompletion(xui_code_provider_set providers,xui_code_completion_proc callback,void* user)
{
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
    else {providers->callbacks.onComplete=callback;providers->callbacks.pCompleteUser=user;if(!xrtOwnershipScopeEnd(&mutation))abort();}
    return __xuiCodeProviderFinish(&scope,status);
}
XUI_API int xuiCodeProviderSetHover(xui_code_provider_set providers,xui_code_hover_proc callback,void* user)
{
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
    else {providers->callbacks.onHover=callback;providers->callbacks.pHoverUser=user;if(!xrtOwnershipScopeEnd(&mutation))abort();}
    return __xuiCodeProviderFinish(&scope,status);
}
XUI_API int xuiCodeProviderSetSignature(xui_code_provider_set providers,xui_code_signature_proc callback,void* user)
{
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
    else {providers->callbacks.onSignature=callback;providers->callbacks.pSignatureUser=user;if(!xrtOwnershipScopeEnd(&mutation))abort();}
    return __xuiCodeProviderFinish(&scope,status);
}
XUI_API int xuiCodeProviderSetCommand(xui_code_provider_set providers,xui_code_command_proc callback,void* user)
{
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
    else {providers->callbacks.onCommand=callback;providers->callbacks.pCommandUser=user;if(!xrtOwnershipScopeEnd(&mutation))abort();}
    return __xuiCodeProviderFinish(&scope,status);
}
XUI_API int xuiCodeProviderSetCommandEnabled(xui_code_provider_set providers,xui_code_command_enabled_proc callback,void* user)
{
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    xrtownershipscope mutation={0};
    if(!xrtOwnershipMutationBegin(&mutation))status=XUI_ERROR_INVALID_STATE;
    else {providers->callbacks.onCommandEnabled=callback;providers->callbacks.pCommandEnabledUser=user;if(!xrtOwnershipScopeEnd(&mutation))abort();}
    return __xuiCodeProviderFinish(&scope,status);
}
XUI_API int xuiCodeProviderRequestCompletion(xui_code_provider_set providers,xui_widget widget,int offset,const char* prefix,xui_code_completion_item_t* items,int capacity,int* count)
{
    if(!providers || !count || offset<0 || capacity<0 || (!items && capacity))return XUI_ERROR_INVALID_ARGUMENT;
    xui_code_provider_scope_t scope={0};int status=xuiCodeProviderSetCallBegin(providers,&scope);
    if(status!=XUI_OK)return status;
    *count=0;__xuiCodeProviderCompletionItemsReset(items,capacity);
    xui_code_completion_proc callback=providers->callbacks.onComplete;void* user=providers->callbacks.pCompleteUser;
    if(!callback)status=XUI_ERROR_UNSUPPORTED;
    else {
        __xuiCodeProviderCallbackBegin(&scope);
        status=callback(widget,offset,prefix?prefix:"",items,capacity,count,user);
        __xuiCodeProviderCallbackEnd(&scope);
        if(status==XUI_OK)status=__xuiCodeProviderResultStatus(providers);
        if(status==XUI_OK && (*count<0 || *count>capacity || !__xuiCodeProviderCompletionItemsValid(items,*count)))status=XUI_ERROR_INVALID_STATE;
    }
    status=__xuiCodeProviderFinish(&scope,status);
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
    if(!callback)status=XUI_ERROR_UNSUPPORTED;
    else {
        __xuiCodeProviderCallbackBegin(&scope);status=callback(widget,offset,&value,user);__xuiCodeProviderCallbackEnd(&scope);
        if(status==XUI_OK)status=__xuiCodeProviderResultStatus(providers);
        if(status==XUI_OK && !xuiInternalCodeProviderHoverValid(&value))status=XUI_ERROR_INVALID_STATE;
    }
    status=__xuiCodeProviderFinish(&scope,status);
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
    if(!callback)status=XUI_ERROR_UNSUPPORTED;
    else {
        __xuiCodeProviderCallbackBegin(&scope);status=callback(widget,offset,&value,user);__xuiCodeProviderCallbackEnd(&scope);
        if(status==XUI_OK)status=__xuiCodeProviderResultStatus(providers);
        if(status==XUI_OK && !xuiInternalCodeProviderSignatureValid(&value))status=XUI_ERROR_INVALID_STATE;
    }
    status=__xuiCodeProviderFinish(&scope,status);
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
    if(!callback)status=XUI_ERROR_UNSUPPORTED;
    else {
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
    if(!callback)status=XUI_ERROR_UNSUPPORTED;
    else {
        __xuiCodeProviderCallbackBegin(&scope);status=callback(widget,command,&value,user);__xuiCodeProviderCallbackEnd(&scope);
        if(status==XUI_OK)status=__xuiCodeProviderResultStatus(providers);
    }
    status=__xuiCodeProviderFinish(&scope,status);if(status==XUI_OK)*enabled=value?1:0;return status;
}
#endif
