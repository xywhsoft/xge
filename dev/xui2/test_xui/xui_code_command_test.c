#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_command_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int xuiTestCommandProvider(xui_widget_t* pWidget, int iCommand, const void* pCommandData, int* pHandled, void* pUser)
{
	int* pSeen = (int*)pUser;
	(void)pWidget;
	if ( iCommand == XUI_CODE_COMMAND_USER_BASE + 9 ) {
		if ( pCommandData != NULL && pSeen != NULL ) *pSeen = *(const int*)pCommandData;
		*pHandled = 1;
		return XUI_OK;
	}
	*pHandled = 0;
	return XUI_OK;
}

static int xuiTestFoldProvider(xui_code_document_t* pDocument, xui_code_fold_range_t* pRanges, int iRangeCapacity, int* pRangeCount, void* pUser)
{
	(void)pDocument;
	(void)pUser;
	if ( pRangeCount == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pRangeCount = 1;
	if ( pRanges != NULL && iRangeCapacity > 0 ) {
		memset(&pRanges[0], 0, sizeof(pRanges[0]));
		pRanges[0].iSize = sizeof(pRanges[0]);
		pRanges[0].iStartLine = 0;
		pRanges[0].iEndLine = 2;
		pRanges[0].iFlags = XUI_CODE_FOLD_CUSTOM;
	}
	return XUI_OK;
}

static int xuiTestClipboardSetText(xui_proxy pProxy, const char* sText)
{
	char* sClipboard;

	if ( pProxy == NULL || pProxy->pUser == NULL || sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	sClipboard = (char*)pProxy->pUser;
	strncpy(sClipboard, sText, 4095u);
	sClipboard[4095] = '\0';
	return XUI_OK;
}

static int xuiTestClipboardGetText(xui_proxy pProxy, char* sText, int iCapacity)
{
	const char* sClipboard;
	int iLength;
	int iCopy;

	if ( pProxy == NULL || pProxy->pUser == NULL || sText == NULL || iCapacity <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	sClipboard = (const char*)pProxy->pUser;
	iLength = (int)strlen(sClipboard);
	iCopy = iLength;
	if ( iCopy >= iCapacity ) iCopy = iCapacity - 1;
	if ( iCopy > 0 ) memcpy(sText, sClipboard, (size_t)iCopy);
	sText[iCopy] = '\0';
	return iLength;
}

int main(void)
{
	xui_code_command_map pMap;
	xui_code_document pDocument;
	xui_code_selection_model pSelection;
	xui_code_fold_state pFoldState;
	xui_code_provider_set pProviders;
	xui_code_command_context_t tContext;
	xui_code_selection_t tState;
	xui_code_fold_range_t tFoldRange;
	xui_code_key_binding_t tBinding;
	xui_proxy_t tProxy;
	char sClipboard[4096];
	int iCommand;
	int iHandled;
	int iProviderSeen;
	int iProviderData;
	int iCount;
	int iLine;
	int iColumn;
	int iFailed;
	int iRet;

	pMap = NULL;
	pDocument = NULL;
	pSelection = NULL;
	pFoldState = NULL;
	pProviders = NULL;
	memset(&tProxy, 0, sizeof(tProxy));
	memset(sClipboard, 0, sizeof(sClipboard));
	tProxy.pUser = sClipboard;
	tProxy.clipboardSetText = xuiTestClipboardSetText;
	tProxy.clipboardGetText = xuiTestClipboardGetText;
	iProviderSeen = 0;
	iProviderData = 91;
	iFailed = 0;

	iRet = xuiCodeCommandMapCreate(&pMap);
	XUI_TEST_CHECK(iRet == XUI_OK && pMap != NULL, "map create");
	iCount = xuiCodeCommandMapGetCount(pMap);
	XUI_TEST_CHECK(iCount >= 20, "default binding count");

	iCommand = 0;
	iRet = xuiCodeCommandMapFind(pMap, XUI_KEY_LEFT, 0, &iCommand);
	XUI_TEST_CHECK(iRet == XUI_OK && iCommand == XUI_CODE_COMMAND_MOVE_LEFT, "left binding");
	iRet = xuiCodeCommandMapFind(pMap, XUI_KEY_LEFT, XUI_MOD_SHIFT, &iCommand);
	XUI_TEST_CHECK(iRet == XUI_OK && iCommand == XUI_CODE_COMMAND_SELECT_LEFT, "shift left binding");
	iRet = xuiCodeCommandMapFind(pMap, 'Z', XUI_MOD_CTRL, &iCommand);
	XUI_TEST_CHECK(iRet == XUI_OK && iCommand == XUI_CODE_COMMAND_UNDO, "ctrl z binding");
	iRet = xuiCodeCommandMapFind(pMap, XUI_KEY_F3, XUI_MOD_SHIFT, &iCommand);
	XUI_TEST_CHECK(iRet == XUI_OK && iCommand == XUI_CODE_COMMAND_FIND_PREVIOUS, "shift f3 binding");
	iRet = xuiCodeCommandMapFind(pMap, XUI_KEY_PAGE_DOWN, 0, &iCommand);
	XUI_TEST_CHECK(iRet == XUI_OK && iCommand == XUI_CODE_COMMAND_MOVE_PAGE_DOWN, "page down binding");
	iRet = xuiCodeCommandMapFind(pMap, XUI_KEY_PAGE_UP, XUI_MOD_SHIFT, &iCommand);
	XUI_TEST_CHECK(iRet == XUI_OK && iCommand == XUI_CODE_COMMAND_SELECT_PAGE_UP, "shift page up binding");

	iRet = xuiCodeCommandMapBind(pMap, 'K', XUI_MOD_CTRL, XUI_CODE_COMMAND_USER_BASE + 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "custom bind");
	iRet = xuiCodeCommandMapFind(pMap, 'K', XUI_MOD_CTRL, &iCommand);
	XUI_TEST_CHECK(iRet == XUI_OK && iCommand == XUI_CODE_COMMAND_USER_BASE + 1, "custom find");

	iRet = xuiCodeCommandMapBind(pMap, 'K', XUI_MOD_CTRL, XUI_CODE_COMMAND_USER_BASE + 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "custom rebind");
	XUI_TEST_CHECK(xuiCodeCommandMapGetCount(pMap) == iCount + 1, "rebind does not duplicate");
	iRet = xuiCodeCommandMapFind(pMap, 'K', XUI_MOD_CTRL, &iCommand);
	XUI_TEST_CHECK(iRet == XUI_OK && iCommand == XUI_CODE_COMMAND_USER_BASE + 2, "custom rebind find");

	memset(&tBinding, 0, sizeof(tBinding));
	iRet = xuiCodeCommandMapGetBinding(pMap, xuiCodeCommandMapGetCount(pMap) - 1, &tBinding);
	XUI_TEST_CHECK(iRet == XUI_OK && tBinding.iSize == sizeof(tBinding), "get binding");
	XUI_TEST_CHECK(tBinding.iKey == 'K' && tBinding.iCommand == XUI_CODE_COMMAND_USER_BASE + 2, "binding contents");

	iRet = xuiCodeCommandMapUnbind(pMap, 'K', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "unbind custom");
	XUI_TEST_CHECK(xuiCodeCommandMapGetCount(pMap) == iCount, "unbind count");
	iRet = xuiCodeCommandMapFind(pMap, 'K', XUI_MOD_CTRL, &iCommand);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED, "custom removed");
	iRet = xuiCodeCommandMapUnbind(pMap, 'K', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED, "unbind missing");

	iRet = xuiCodeCommandMapLoadDefaults(pMap);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeCommandMapGetCount(pMap) == iCount, "reload defaults");

	iRet = xuiCodeDocumentCreate(&pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK, "document create");
	iRet = xuiCodeDocumentSetText(pDocument, "alpha beta\n{\nbody\n}\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "document text");
	iRet = xuiCodeSelectionCreate(&pSelection);
	XUI_TEST_CHECK(iRet == XUI_OK, "selection create");
	iRet = xuiCodeFoldStateCreate(&pFoldState);
	XUI_TEST_CHECK(iRet == XUI_OK, "fold state create");
	iRet = xuiCodeFoldStateBuildFromProvider(pFoldState, pDocument, xuiTestFoldProvider, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "fold provider");
	iRet = xuiCodeProviderSetCreate(&pProviders);
	XUI_TEST_CHECK(iRet == XUI_OK, "providers create");
	iRet = xuiCodeProviderSetCommand(pProviders, xuiTestCommandProvider, &iProviderSeen);
	XUI_TEST_CHECK(iRet == XUI_OK, "set provider command");
	memset(&tContext, 0, sizeof(tContext));
	tContext.iSize = sizeof(tContext);
	tContext.pDocument = pDocument;
	tContext.pSelection = pSelection;
	tContext.pFoldState = pFoldState;
	tContext.pProviders = pProviders;
	tContext.sIndent = "  ";
	tContext.iIndentColumns = 2;
	tContext.sLineComment = "//";
	tContext.pCommandData = &iProviderData;
	tContext.iPageLineCount = 2;

	iRet = xuiCodeCommandExecute(&tContext, XUI_CODE_COMMAND_USER_BASE + 9, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && iHandled == 1 && iProviderSeen == 91, "provider command handles");
	iRet = xuiCodeDocumentSetText(pDocument, "l0\nl1\nl2\nl3\nl4\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "page document text");
	iRet = xuiCodeSelectionGotoLineColumn(pSelection, pDocument, 1, 1, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "page goto line");
	iRet = xuiCodeCommandExecute(&tContext, XUI_CODE_COMMAND_MOVE_PAGE_DOWN, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && iHandled == 1, "execute page down");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "page down state");
	iRet = xuiCodeDocumentOffsetToLineColumn(pDocument, tState.iCaretOffset, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && iLine == 3 && iColumn == 1, "page down target");
	iRet = xuiCodeCommandExecute(&tContext, XUI_CODE_COMMAND_SELECT_PAGE_UP, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && iHandled == 1, "execute select page up");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "select page state");
	iRet = xuiCodeDocumentOffsetToLineColumn(pDocument, tState.iCaretOffset, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && iLine == 1 && iColumn == 1 && tState.iAnchorOffset != tState.iCaretOffset, "select page target");
	iRet = xuiCodeDocumentSetText(pDocument, "alpha beta\n{\nbody\n}\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore document text");
	iRet = xuiCodeSelectionGotoOffset(pSelection, pDocument, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "goto start");
	iRet = xuiCodeCommandExecute(&tContext, XUI_CODE_COMMAND_MOVE_WORD_RIGHT, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && iHandled == 1, "execute word right");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iCaretOffset == 6, "word right offset");
	iRet = xuiCodeCommandExecute(&tContext, XUI_CODE_COMMAND_DELETE_WORD_FORWARD, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "alpha \n{\nbody\n}\n") == 0, "delete word forward command");
	iRet = xuiCodeCommandExecute(&tContext, XUI_CODE_COMMAND_INSERT_TAB, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && strstr(xuiCodeDocumentGetText(pDocument), "alpha   \n") != NULL, "insert tab command");
	iRet = xuiCodeSelectionGotoLineColumn(pSelection, pDocument, 0, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "goto fold line");
	iRet = xuiCodeCommandExecute(&tContext, XUI_CODE_COMMAND_FOLD_TOGGLE, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && iHandled == 1, "fold toggle command");
	iRet = xuiCodeFoldStateGetRange(pFoldState, 0, &tFoldRange);
	XUI_TEST_CHECK(iRet == XUI_OK && (tFoldRange.iFlags & XUI_CODE_FOLD_COLLAPSED) != 0, "fold collapsed");
	iRet = xuiCodeCommandExecute(&tContext, XUI_CODE_COMMAND_UNFOLD_ALL, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && iHandled == 1, "unfold all command");
	iRet = xuiCodeFoldStateGetRange(pFoldState, 0, &tFoldRange);
	XUI_TEST_CHECK(iRet == XUI_OK && (tFoldRange.iFlags & XUI_CODE_FOLD_COLLAPSED) == 0, "fold unfolded");
	iRet = xuiCodeSelectionSetRange(pSelection, pDocument, 0, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "select copy unsupported");
	iRet = xuiCodeCommandExecute(&tContext, XUI_CODE_COMMAND_COPY, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED && iHandled == 0, "copy unsupported without widget clipboard");
	tContext.pProxy = &tProxy;
	iRet = xuiCodeDocumentSetText(pDocument, "copy cut\npaste\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "clipboard document text");
	iRet = xuiCodeSelectionSetRange(pSelection, pDocument, 0, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "select copy range");
	iRet = xuiCodeCommandExecute(&tContext, XUI_CODE_COMMAND_COPY, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && iHandled == 1 && strcmp(sClipboard, "copy") == 0, "copy command");
	iRet = xuiCodeSelectionSetRange(pSelection, pDocument, 5, 8);
	XUI_TEST_CHECK(iRet == XUI_OK, "select cut range");
	iRet = xuiCodeCommandExecute(&tContext, XUI_CODE_COMMAND_CUT, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && iHandled == 1 && strcmp(sClipboard, "cut") == 0 && strcmp(xuiCodeDocumentGetText(pDocument), "copy \npaste\n") == 0, "cut command");
	iRet = xuiTestClipboardSetText(&tProxy, "X\r\nY");
	XUI_TEST_CHECK(iRet == XUI_OK, "set paste clipboard");
	iRet = xuiCodeCommandExecute(&tContext, XUI_CODE_COMMAND_PASTE, &iHandled);
	XUI_TEST_CHECK(iRet == XUI_OK && iHandled == 1 && strcmp(xuiCodeDocumentGetText(pDocument), "copy X\nY\npaste\n") == 0, "paste command");

cleanup:
	xuiCodeProviderSetDestroy(pProviders);
	xuiCodeFoldStateDestroy(pFoldState);
	xuiCodeSelectionDestroy(pSelection);
	xuiCodeDocumentDestroy(pDocument);
	xuiCodeCommandMapDestroy(pMap);
	if ( iFailed ) return 1;
	printf("xui_code_command_test passed\n");
	return 0;
}
