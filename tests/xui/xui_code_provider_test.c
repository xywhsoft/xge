#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_provider_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int xuiTestComplete(xui_widget_t* pWidget, int iOffset, const char* sPrefix, xui_code_completion_item_t* pItems, int iItemCapacity, int* pItemCount, void* pUser)
{
	int* pCalls = (int*)pUser;
	(void)pWidget;
	if ( pCalls != NULL ) (*pCalls)++;
	if ( iOffset != 4 || strcmp(sPrefix, "pr") != 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pItems != NULL && iItemCapacity > 0 ) {
		pItems[0].iSize = sizeof(pItems[0]);
		pItems[0].sLabel = "printf";
		pItems[0].sInsertText = "printf";
		pItems[0].sDetail = "int printf(...)";
		pItems[0].iKind = 1;
	}
	*pItemCount = 1;
	return XUI_OK;
}

static int xuiTestHover(xui_widget_t* pWidget, int iOffset, xui_code_hover_t* pHover, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( iOffset != 8 ) return XUI_ERROR_INVALID_ARGUMENT;
	pHover->iSize = sizeof(*pHover);
	pHover->tRange.iStart = 6;
	pHover->tRange.iEnd = 10;
	pHover->sText = "hover";
	pHover->sContentType = "text/plain";
	return XUI_OK;
}

static int xuiTestSignature(xui_widget_t* pWidget, int iOffset, xui_code_signature_help_t* pHelp, void* pUser)
{
	static xui_code_signature_parameter_t arrParams[2];
	(void)pWidget;
	(void)pUser;
	if ( iOffset != 12 ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(arrParams, 0, sizeof(arrParams));
	arrParams[0].iSize = sizeof(arrParams[0]);
	arrParams[0].sLabel = "format";
	arrParams[1].iSize = sizeof(arrParams[1]);
	arrParams[1].sLabel = "...";
	pHelp->iSize = sizeof(*pHelp);
	pHelp->sLabel = "printf(format, ...)";
	pHelp->pParameters = arrParams;
	pHelp->iParameterCount = 2;
	pHelp->iActiveParameter = 1;
	return XUI_OK;
}

static int xuiTestCommand(xui_widget_t* pWidget, int iCommand, const void* pCommandData, int* pHandled, void* pUser)
{
	int* pSeen = (int*)pUser;
	(void)pWidget;
	if ( iCommand != XUI_CODE_COMMAND_USER_BASE + 3 || pCommandData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pSeen != NULL ) *pSeen = *(const int*)pCommandData;
	*pHandled = 1;
	return XUI_OK;
}

static int xuiTestCommandEnabled(xui_widget_t* pWidget, int iCommand, int* pEnabled, void* pUser)
{
	int* pCalls = (int*)pUser;
	(void)pWidget;
	if ( pEnabled == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pCalls != NULL ) (*pCalls)++;
	if ( iCommand != XUI_CODE_COMMAND_USER_BASE + 3 ) return XUI_ERROR_UNSUPPORTED;
	*pEnabled = 1;
	return XUI_OK;
}

static int xuiTestInvalidCompletion(xui_widget_t* pWidget, int iOffset,
	const char* sPrefix, xui_code_completion_item_t* pItems, int iItemCapacity,
	int* pItemCount, void* pUser)
{
	(void)pWidget;
	(void)iOffset;
	(void)sPrefix;
	(void)pItems;
	(void)iItemCapacity;
	(void)pUser;
	*pItemCount = -1;
	return XUI_OK;
}

static int xuiTestOverflowCompletion(xui_widget_t* pWidget, int iOffset,
	const char* sPrefix, xui_code_completion_item_t* pItems, int iItemCapacity,
	int* pItemCount, void* pUser)
{
	(void)pWidget;
	(void)iOffset;
	(void)sPrefix;
	(void)pUser;
	if ( pItems != NULL && iItemCapacity > 0 ) pItems[0].sLabel = "overflow";
	*pItemCount = iItemCapacity + 1;
	return XUI_OK;
}

static int xuiTestShortCompletion(xui_widget_t* pWidget, int iOffset,
	const char* sPrefix, xui_code_completion_item_t* pItems, int iItemCapacity,
	int* pItemCount, void* pUser)
{
	(void)pWidget;
	(void)iOffset;
	(void)sPrefix;
	(void)pUser;
	if ( pItems != NULL && iItemCapacity > 0 ) {
		pItems[0].iSize = sizeof(uint32_t);
		pItems[0].sLabel = "short";
	}
	*pItemCount = 1;
	return XUI_OK;
}

static int xuiTestPartialCompletion(xui_widget_t* pWidget, int iOffset,
	const char* sPrefix, xui_code_completion_item_t* pItems, int iItemCapacity,
	int* pItemCount, void* pUser)
{
	(void)pWidget;
	(void)iOffset;
	(void)sPrefix;
	(void)pUser;
	if ( pItems == NULL || iItemCapacity <= 0 ||
	     pItems[0].iSize != sizeof(pItems[0]) ) {
		return XUI_ERROR_INVALID_STATE;
	}
	pItems[0].sLabel = "partial";
	*pItemCount = 1;
	return XUI_OK;
}

static int xuiTestFailingCompletion(xui_widget_t* pWidget, int iOffset,
	const char* sPrefix, xui_code_completion_item_t* pItems, int iItemCapacity,
	int* pItemCount, void* pUser)
{
	(void)pWidget;
	(void)iOffset;
	(void)sPrefix;
	(void)pItems;
	(void)iItemCapacity;
	(void)pUser;
	*pItemCount = 9;
	return XUI_ERROR_RESOURCE_FAILED;
}

static int xuiTestInvalidHover(xui_widget_t* pWidget, int iOffset,
	xui_code_hover_t* pHover, void* pUser)
{
	(void)pWidget;
	(void)iOffset;
	(void)pUser;
	pHover->tRange.iStart = -1;
	pHover->tRange.iEnd = 2;
	pHover->sText = "invalid";
	return XUI_OK;
}

static int xuiTestShortHover(xui_widget_t* pWidget, int iOffset,
	xui_code_hover_t* pHover, void* pUser)
{
	(void)pWidget;
	(void)iOffset;
	(void)pUser;
	pHover->iSize = sizeof(uint32_t);
	pHover->tRange = (xui_code_range_t){0, 1};
	pHover->sText = "short";
	return XUI_OK;
}

static int xuiTestMissingTextHover(xui_widget_t* pWidget, int iOffset,
	xui_code_hover_t* pHover, void* pUser)
{
	(void)pWidget;
	(void)iOffset;
	(void)pUser;
	pHover->tRange = (xui_code_range_t){0, 1};
	return XUI_OK;
}

static int xuiTestInvalidSignature(xui_widget_t* pWidget, int iOffset,
	xui_code_signature_help_t* pHelp, void* pUser)
{
	(void)pWidget;
	(void)iOffset;
	(void)pUser;
	pHelp->sLabel = "invalid";
	pHelp->iParameterCount = 1;
	pHelp->pParameters = NULL;
	return XUI_OK;
}

static int xuiTestInvalidActiveSignature(xui_widget_t* pWidget, int iOffset,
	xui_code_signature_help_t* pHelp, void* pUser)
{
	static xui_code_signature_parameter_t tParameter;
	(void)pWidget;
	(void)iOffset;
	(void)pUser;
	memset(&tParameter, 0, sizeof(tParameter));
	tParameter.iSize = sizeof(tParameter);
	pHelp->sLabel = "call(value)";
	pHelp->pParameters = &tParameter;
	pHelp->iParameterCount = 1;
	pHelp->iActiveParameter = 1;
	return XUI_OK;
}

static int xuiTestInvalidParameterSignature(xui_widget_t* pWidget, int iOffset,
	xui_code_signature_help_t* pHelp, void* pUser)
{
	static xui_code_signature_parameter_t tParameter;
	(void)pWidget;
	(void)iOffset;
	(void)pUser;
	memset(&tParameter, 0, sizeof(tParameter));
	tParameter.iSize = sizeof(uint32_t);
	tParameter.tLabelRange = (xui_code_range_t){5, 10};
	pHelp->sLabel = "call(value)";
	pHelp->pParameters = &tParameter;
	pHelp->iParameterCount = 1;
	pHelp->iActiveParameter = 0;
	return XUI_OK;
}

static int xuiTestOutOfRangeParameterSignature(xui_widget_t* pWidget, int iOffset,
	xui_code_signature_help_t* pHelp, void* pUser)
{
	static xui_code_signature_parameter_t tParameter;
	(void)pWidget;
	(void)iOffset;
	(void)pUser;
	memset(&tParameter, 0, sizeof(tParameter));
	tParameter.iSize = sizeof(tParameter);
	tParameter.tLabelRange = (xui_code_range_t){5, 50};
	pHelp->sLabel = "call(value)";
	pHelp->pParameters = &tParameter;
	pHelp->iParameterCount = 1;
	pHelp->iActiveParameter = 0;
	return XUI_OK;
}

static int xuiTestEmptySignature(xui_widget_t* pWidget, int iOffset,
	xui_code_signature_help_t* pHelp, void* pUser)
{
	(void)pWidget;
	(void)iOffset;
	(void)pUser;
	pHelp->sLabel = "call()";
	return XUI_OK;
}

static int xuiTestFailingCommand(xui_widget_t* pWidget, int iCommand,
	const void* pCommandData, int* pHandled, void* pUser)
{
	(void)pWidget;
	(void)iCommand;
	(void)pCommandData;
	(void)pUser;
	*pHandled = 1;
	return XUI_ERROR_RESOURCE_FAILED;
}

static int xuiTestTruthyCommandEnabled(xui_widget_t* pWidget, int iCommand,
	int* pEnabled, void* pUser)
{
	(void)pWidget;
	(void)iCommand;
	(void)pUser;
	*pEnabled = 7;
	return XUI_OK;
}

int main(void)
{
	xui_code_provider_set pProviders;
	xui_code_completion_item_t arrItems[2];
	xui_code_hover_t tHover;
	xui_code_signature_help_t tSignature;
	int iCount;
	int iHandled;
	int iCalls;
	int iCommandData;
	int iSeen;
	int iFailed;
	int iRet;

	pProviders = NULL;
	iCalls = 0;
	iSeen = 0;
	iFailed = 0;
	iCommandData = 77;

	iRet = xuiCodeProviderSetCreate(&pProviders);
	XUI_TEST_CHECK(iRet == XUI_OK && pProviders != NULL, "provider create");
	iRet = xuiCodeProviderRequestCompletion(pProviders, NULL, 4, "pr", arrItems, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED && iCount == 0, "missing completion");
	iRet = xuiCodeProviderSetCompletion(pProviders, xuiTestComplete, &iCalls);
	XUI_TEST_CHECK(iRet == XUI_OK, "set completion");
	memset(arrItems, 0, sizeof(arrItems));
	iRet = xuiCodeProviderRequestCompletion(pProviders, NULL, 4, "pr", arrItems, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 && iCalls == 1, "request completion");
	XUI_TEST_CHECK(strcmp(arrItems[0].sLabel, "printf") == 0 && arrItems[0].iKind == 1, "completion item");

	iRet = xuiCodeProviderSetHover(pProviders, xuiTestHover, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set hover");
	iRet = xuiCodeProviderRequestHover(pProviders, NULL, 8, &tHover);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tHover.sText, "hover") == 0, "request hover");
	XUI_TEST_CHECK(tHover.tRange.iStart == 6 && tHover.tRange.iEnd == 10, "hover range");

	iRet = xuiCodeProviderSetSignature(pProviders, xuiTestSignature, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set signature");
	iRet = xuiCodeProviderRequestSignature(pProviders, NULL, 12, &tSignature);
	XUI_TEST_CHECK(iRet == XUI_OK && tSignature.iParameterCount == 2 && tSignature.iActiveParameter == 1, "request signature");
	XUI_TEST_CHECK(strcmp(tSignature.pParameters[0].sLabel, "format") == 0, "signature parameter");

	iRet = xuiCodeProviderSetCommand(pProviders, xuiTestCommand, &iSeen);
	XUI_TEST_CHECK(iRet == XUI_OK, "set command");
	iRet = xuiCodeProviderCanExecuteCommand(pProviders, NULL, XUI_CODE_COMMAND_USER_BASE + 3, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED && iHandled == 0, "missing command enabled");
	iRet = xuiCodeProviderSetCommandEnabled(pProviders, xuiTestCommandEnabled, &iCalls);
	XUI_TEST_CHECK(iRet == XUI_OK, "set command enabled");
	iRet = xuiCodeProviderCanExecuteCommand(pProviders, NULL, XUI_CODE_COMMAND_USER_BASE + 3, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && iHandled == 1, "command enabled query");
	iRet = xuiCodeProviderExecuteCommand(pProviders, NULL, XUI_CODE_COMMAND_USER_BASE + 3, &iCommandData, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && iHandled == 1 && iSeen == 77, "execute command");

	iRet = xuiCodeProviderRequestCompletion(pProviders, NULL, 4, "pr", NULL, 1, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT,
		"missing completion buffer rejected");
	iRet = xuiCodeProviderSetCompletion(pProviders, xuiTestInvalidCompletion, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set invalid completion");
	iRet = xuiCodeProviderRequestCompletion(pProviders, NULL, 0, NULL, NULL, 0, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && iCount == 0,
		"negative completion count rejected");
	iRet = xuiCodeProviderSetCompletion(pProviders, xuiTestOverflowCompletion, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set overflow completion");
	arrItems[0].sDetail = "stale";
	iRet = xuiCodeProviderRequestCompletion(pProviders, NULL, 0, NULL,
		arrItems, 1, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && iCount == 0 &&
		arrItems[0].sLabel == NULL && arrItems[0].sDetail == NULL &&
		arrItems[0].iSize == sizeof(arrItems[0]),
		"overflow completion rejected and output reset");
	iRet = xuiCodeProviderSetCompletion(pProviders, xuiTestShortCompletion, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set short completion");
	iRet = xuiCodeProviderRequestCompletion(pProviders, NULL, 0, NULL,
		arrItems, 1, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && iCount == 0,
		"short completion descriptor rejected");
	iRet = xuiCodeProviderSetCompletion(pProviders, xuiTestPartialCompletion, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set partial completion");
	arrItems[0].sDetail = "stale";
	iRet = xuiCodeProviderRequestCompletion(pProviders, NULL, 0, NULL,
		arrItems, 1, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		strcmp(arrItems[0].sLabel, "partial") == 0 && arrItems[0].sDetail == NULL,
		"completion slots initialized before callback");
	iRet = xuiCodeProviderSetCompletion(pProviders, xuiTestFailingCompletion, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set failing completion");
	iRet = xuiCodeProviderRequestCompletion(pProviders, NULL, 0, NULL, NULL, 0, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_RESOURCE_FAILED && iCount == 0,
		"failed completion clears count");

	iRet = xuiCodeProviderSetHover(pProviders, xuiTestInvalidHover, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set invalid hover");
	iRet = xuiCodeProviderRequestHover(pProviders, NULL, 0, &tHover);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && tHover.sText == NULL,
		"invalid hover output rejected");
	iRet = xuiCodeProviderSetHover(pProviders, xuiTestShortHover, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set short hover");
	iRet = xuiCodeProviderRequestHover(pProviders, NULL, 0, &tHover);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && tHover.sText == NULL,
		"short hover descriptor rejected");
	iRet = xuiCodeProviderSetHover(pProviders, xuiTestMissingTextHover, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set missing text hover");
	iRet = xuiCodeProviderRequestHover(pProviders, NULL, 0, &tHover);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && tHover.sText == NULL,
		"missing hover text rejected");
	iRet = xuiCodeProviderSetSignature(pProviders, xuiTestInvalidSignature, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set invalid signature");
	iRet = xuiCodeProviderRequestSignature(pProviders, NULL, 0, &tSignature);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && tSignature.sLabel == NULL,
		"invalid signature output rejected");
	iRet = xuiCodeProviderSetSignature(pProviders, xuiTestInvalidActiveSignature, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set invalid active signature");
	iRet = xuiCodeProviderRequestSignature(pProviders, NULL, 0, &tSignature);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && tSignature.sLabel == NULL &&
		tSignature.iActiveParameter == -1,
		"invalid active signature rejected");
	iRet = xuiCodeProviderSetSignature(pProviders, xuiTestInvalidParameterSignature, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set invalid parameter signature");
	iRet = xuiCodeProviderRequestSignature(pProviders, NULL, 0, &tSignature);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && tSignature.sLabel == NULL,
		"short signature parameter rejected");
	iRet = xuiCodeProviderSetSignature(pProviders,
		xuiTestOutOfRangeParameterSignature, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set out-of-range parameter signature");
	iRet = xuiCodeProviderRequestSignature(pProviders, NULL, 0, &tSignature);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && tSignature.sLabel == NULL,
		"out-of-range signature parameter rejected");
	iRet = xuiCodeProviderSetSignature(pProviders, xuiTestEmptySignature, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set empty signature");
	iRet = xuiCodeProviderRequestSignature(pProviders, NULL, 0, &tSignature);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tSignature.sLabel, "call()") == 0 &&
		tSignature.iParameterCount == 0 && tSignature.iActiveParameter == -1,
		"empty signature receives no-active default");

	iRet = xuiCodeProviderSetCommand(pProviders, xuiTestFailingCommand, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set failing command");
	iRet = xuiCodeProviderExecuteCommand(pProviders, NULL,
		XUI_CODE_COMMAND_USER_BASE + 3, NULL, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_ERROR_RESOURCE_FAILED && iHandled == 0,
		"failed command clears handled");
	iRet = xuiCodeProviderSetCommandEnabled(pProviders,
		xuiTestTruthyCommandEnabled, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set truthy command enabled");
	iRet = xuiCodeProviderCanExecuteCommand(pProviders, NULL,
		XUI_CODE_COMMAND_USER_BASE + 3, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && iHandled == 1,
		"command enabled normalized");

	xuiCodeProviderSetClear(pProviders);
	iRet = xuiCodeProviderCanExecuteCommand(pProviders, NULL, XUI_CODE_COMMAND_USER_BASE + 3, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED && iHandled == 0, "clear command enabled");
	iRet = xuiCodeProviderExecuteCommand(pProviders, NULL, XUI_CODE_COMMAND_USER_BASE + 3, &iCommandData, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED && iHandled == 0, "clear providers");

cleanup:
	xuiCodeProviderSetDestroy(pProviders);
	if ( iFailed ) return 1;
	printf("xui_code_provider_test passed\n");
	return 0;
}
