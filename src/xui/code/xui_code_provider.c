#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui.h"
#include "xui_code_provider_internal.h"

#include <stddef.h>
#include <string.h>

struct xui_code_provider_set_t {
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
};

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
	xui_code_provider_set pProviders;

	if ( ppProviders == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppProviders = NULL;
	pProviders = (xui_code_provider_set)xrtMalloc(sizeof(*pProviders));
	if ( pProviders == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pProviders, 0, sizeof(*pProviders));
	*ppProviders = pProviders;
	return XUI_OK;
}

XUI_API void xuiCodeProviderSetDestroy(xui_code_provider_set pProviders)
{
	xrtFree(pProviders);
}

XUI_API void xuiCodeProviderSetClear(xui_code_provider_set pProviders)
{
	if ( pProviders == NULL ) return;
	memset(pProviders, 0, sizeof(*pProviders));
}

XUI_API int xuiCodeProviderSetCompletion(xui_code_provider_set pProviders, xui_code_completion_proc onComplete, void* pUser)
{
	if ( pProviders == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pProviders->onComplete = onComplete;
	pProviders->pCompleteUser = pUser;
	return XUI_OK;
}

XUI_API int xuiCodeProviderSetHover(xui_code_provider_set pProviders, xui_code_hover_proc onHover, void* pUser)
{
	if ( pProviders == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pProviders->onHover = onHover;
	pProviders->pHoverUser = pUser;
	return XUI_OK;
}

XUI_API int xuiCodeProviderSetSignature(xui_code_provider_set pProviders, xui_code_signature_proc onSignature, void* pUser)
{
	if ( pProviders == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pProviders->onSignature = onSignature;
	pProviders->pSignatureUser = pUser;
	return XUI_OK;
}

XUI_API int xuiCodeProviderSetCommand(xui_code_provider_set pProviders, xui_code_command_proc onCommand, void* pUser)
{
	if ( pProviders == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pProviders->onCommand = onCommand;
	pProviders->pCommandUser = pUser;
	return XUI_OK;
}

XUI_API int xuiCodeProviderSetCommandEnabled(xui_code_provider_set pProviders, xui_code_command_enabled_proc onEnabled, void* pUser)
{
	if ( pProviders == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pProviders->onCommandEnabled = onEnabled;
	pProviders->pCommandEnabledUser = pUser;
	return XUI_OK;
}

XUI_API int xuiCodeProviderRequestCompletion(xui_code_provider_set pProviders, xui_widget pWidget, int iOffset, const char* sPrefix, xui_code_completion_item_t* pItems, int iItemCapacity, int* pItemCount)
{
	int iRet;

	if ( (pProviders == NULL) || (pItemCount == NULL) || (iOffset < 0) ||
	     (iItemCapacity < 0) || (pItems == NULL && iItemCapacity > 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pItemCount = 0;
	__xuiCodeProviderCompletionItemsReset(pItems, iItemCapacity);
	if ( pProviders->onComplete == NULL ) return XUI_ERROR_UNSUPPORTED;
	iRet = pProviders->onComplete((xui_widget_t*)pWidget, iOffset,
		(sPrefix != NULL) ? sPrefix : "", pItems, iItemCapacity,
		pItemCount, pProviders->pCompleteUser);
	if ( iRet != XUI_OK ) {
		*pItemCount = 0;
		__xuiCodeProviderCompletionItemsReset(pItems, iItemCapacity);
		return iRet;
	}
	if ( *pItemCount < 0 || *pItemCount > iItemCapacity ||
	     !__xuiCodeProviderCompletionItemsValid(pItems, *pItemCount) ) {
		*pItemCount = 0;
		__xuiCodeProviderCompletionItemsReset(pItems, iItemCapacity);
		return XUI_ERROR_INVALID_STATE;
	}
	return XUI_OK;
}

XUI_API int xuiCodeProviderRequestHover(xui_code_provider_set pProviders, xui_widget pWidget, int iOffset, xui_code_hover_t* pHover)
{
	xui_code_hover_t tHover;
	int iRet;

	if ( (pProviders == NULL) || (pHover == NULL) || (iOffset < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pHover, 0, sizeof(*pHover));
	pHover->iSize = sizeof(*pHover);
	if ( pProviders->onHover == NULL ) return XUI_ERROR_UNSUPPORTED;
	memset(&tHover, 0, sizeof(tHover));
	tHover.iSize = sizeof(tHover);
	iRet = pProviders->onHover((xui_widget_t*)pWidget, iOffset,
		&tHover, pProviders->pHoverUser);
	if ( iRet != XUI_OK ) return iRet;
	if ( !xuiInternalCodeProviderHoverValid(&tHover) ) return XUI_ERROR_INVALID_STATE;
	tHover.iSize = sizeof(tHover);
	*pHover = tHover;
	return XUI_OK;
}

XUI_API int xuiCodeProviderRequestSignature(xui_code_provider_set pProviders, xui_widget pWidget, int iOffset, xui_code_signature_help_t* pHelp)
{
	xui_code_signature_help_t tHelp;
	int iRet;

	if ( (pProviders == NULL) || (pHelp == NULL) || (iOffset < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pHelp, 0, sizeof(*pHelp));
	pHelp->iSize = sizeof(*pHelp);
	pHelp->iActiveParameter = -1;
	if ( pProviders->onSignature == NULL ) return XUI_ERROR_UNSUPPORTED;
	memset(&tHelp, 0, sizeof(tHelp));
	tHelp.iSize = sizeof(tHelp);
	tHelp.iActiveParameter = -1;
	iRet = pProviders->onSignature((xui_widget_t*)pWidget, iOffset,
		&tHelp, pProviders->pSignatureUser);
	if ( iRet != XUI_OK ) return iRet;
	if ( !xuiInternalCodeProviderSignatureValid(&tHelp) ) return XUI_ERROR_INVALID_STATE;
	tHelp.iSize = sizeof(tHelp);
	*pHelp = tHelp;
	return XUI_OK;
}

XUI_API int xuiCodeProviderExecuteCommand(xui_code_provider_set pProviders, xui_widget pWidget, int iCommand, const void* pCommandData, int* pHandled)
{
	int bHandled;
	int iRet;

	if ( (pProviders == NULL) || (pHandled == NULL) || (iCommand <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	*pHandled = 0;
	if ( pProviders->onCommand == NULL ) return XUI_ERROR_UNSUPPORTED;
	bHandled = 0;
	iRet = pProviders->onCommand((xui_widget_t*)pWidget, iCommand,
		pCommandData, &bHandled, pProviders->pCommandUser);
	if ( iRet == XUI_OK ) *pHandled = bHandled ? 1 : 0;
	return iRet;
}

XUI_API int xuiCodeProviderCanExecuteCommand(xui_code_provider_set pProviders, xui_widget pWidget, int iCommand, int* pEnabled)
{
	int bEnabled;
	int iRet;

	if ( (pProviders == NULL) || (pEnabled == NULL) || (iCommand <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	*pEnabled = 0;
	if ( pProviders->onCommandEnabled == NULL ) return XUI_ERROR_UNSUPPORTED;
	bEnabled = 0;
	iRet = pProviders->onCommandEnabled((xui_widget_t*)pWidget, iCommand,
		&bEnabled, pProviders->pCommandEnabledUser);
	if ( iRet == XUI_OK ) *pEnabled = bEnabled ? 1 : 0;
	return iRet;
}

#endif
