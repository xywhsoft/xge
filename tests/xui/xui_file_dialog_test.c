#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

int xuiFileDialogTestPathRootLen(const char* sPath);
int xuiFileDialogTestParentDir(const char* sPath, char* sBuffer, size_t iCapacity);
int xuiFileDialogTestPathTipOpen(xui_file_dialog pDialog);

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_file_dialog_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_file_dialog_test_result_t {
	int iCount;
	int iResult;
	char sPath[1024];
} xui_file_dialog_test_result_t;

typedef struct xui_file_dialog_destroy_result_t {
	xui_file_dialog* ppDialog;
	int iCount;
} xui_file_dialog_destroy_result_t;

typedef struct xui_file_dialog_context_result_t {
	int iCount;
	int iIndex;
	int bDirectory;
	char sPath[1024];
} xui_file_dialog_context_result_t;

static int __xuiFileDialogBackgroundClick(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	int* pCount = (int*)pUser;
	(void)pWidget;
	if ( pEvent != NULL && pEvent->iType == XUI_EVENT_POINTER_CLICK && pCount != NULL ) (*pCount)++;
	return XUI_OK;
}

static void __xuiFileDialogTestResult(xui_file_dialog pDialog, int iResult, const char* sPath, void* pUser)
{
	xui_file_dialog_test_result_t* pResult = (xui_file_dialog_test_result_t*)pUser;
	(void)pDialog;
	if ( pResult == NULL ) {
		return;
	}
	pResult->iCount++;
	pResult->iResult = iResult;
	snprintf(pResult->sPath, sizeof(pResult->sPath), "%s", sPath != NULL ? sPath : "");
}

static void __xuiFileDialogDestroyResult(xui_file_dialog pDialog, int iResult,
	const char* sPath, void* pUser)
{
	xui_file_dialog_destroy_result_t* pResult = (xui_file_dialog_destroy_result_t*)pUser;
	(void)iResult;
	(void)sPath;
	if ( pResult == NULL ) return;
	pResult->iCount++;
	xuiFileDialogDestroy(pDialog);
	if ( pResult->ppDialog != NULL ) *pResult->ppDialog = NULL;
}

static int __xuiFileDialogDestroyEditEvent(xui_widget pWidget,
	const xui_edit_event_t* pEvent, void* pUser)
{
	xui_file_dialog_destroy_result_t* pResult = (xui_file_dialog_destroy_result_t*)pUser;
	(void)pWidget;
	if ( pResult == NULL || pEvent == NULL || pEvent->iType != XUI_EDIT_EVENT_COMMIT ) {
		return XUI_OK;
	}
	pResult->iCount++;
	if ( pResult->ppDialog != NULL && *pResult->ppDialog != NULL ) {
		xuiFileDialogDestroy(*pResult->ppDialog);
		*pResult->ppDialog = NULL;
	}
	return XUI_OK;
}

static int __xuiFileDialogTestContext(xui_file_dialog pDialog, int iIndex,
	const char* sPath, int bDirectory, float fX, float fY, void* pUser)
{
	xui_file_dialog_context_result_t* pResult = (xui_file_dialog_context_result_t*)pUser;
	(void)pDialog;
	(void)fX;
	(void)fY;
	if ( pResult == NULL ) return XUI_OK;
	pResult->iCount++;
	pResult->iIndex = iIndex;
	pResult->bDirectory = bDirectory;
	snprintf(pResult->sPath, sizeof(pResult->sPath), "%s", sPath != NULL ? sPath : "");
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiFileDialogFindEntry(xui_file_dialog pDialog, const char* sName)
{
	int i;
	int iCount;

	iCount = xuiFileDialogGetEntryCount(pDialog);
	for ( i = 0; i < iCount; i++ ) {
		if ( strcmp(xuiFileDialogGetEntryName(pDialog, i), sName) == 0 ) {
			return i;
		}
	}
	return -1;
}

static int __xuiFileDialogPathHelpersValid(void)
{
	static const struct {
		const char* sPath;
		const char* sRoot;
	} arrRoots[] = {
		{"/tmp", "/"},
		{"C:", "C:"},
		{"C:\\folder", "C:\\"},
		{"\\\\server\\share", "\\\\server\\share"},
		{"\\\\server\\share\\folder", "\\\\server\\share\\"},
		{"\\\\?\\C:\\folder", "\\\\?\\C:\\"},
		{"\\\\?\\UNC\\server\\share", "\\\\?\\UNC\\server\\share"},
		{"\\\\?\\UNC\\server\\share\\folder", "\\\\?\\UNC\\server\\share\\"}
	};
	static const struct {
		const char* sPath;
		const char* sParent;
	} arrParents[] = {
		{"/", ""},
		{"/tmp", "/"},
		{"/tmp/child", "/tmp"},
		{"C:\\", ""},
		{"C:\\folder", "C:\\"},
		{"C:\\folder\\child", "C:\\folder"},
		{"\\\\server\\share", ""},
		{"\\\\server\\share\\folder", "\\\\server\\share\\"},
		{"\\\\?\\C:\\folder", "\\\\?\\C:\\"},
		{"\\\\?\\UNC\\server\\share", ""},
		{"\\\\?\\UNC\\server\\share\\folder", "\\\\?\\UNC\\server\\share\\"}
	};
	char sParent[256];
	size_t i;

	for ( i = 0u; i < sizeof(arrRoots) / sizeof(arrRoots[0]); i++ ) {
		if ( xuiFileDialogTestPathRootLen(arrRoots[i].sPath) !=
		     (int)strlen(arrRoots[i].sRoot) ) return 0;
	}
	for ( i = 0u; i < sizeof(arrParents) / sizeof(arrParents[0]); i++ ) {
		if ( xuiFileDialogTestParentDir(arrParents[i].sPath, sParent,
		     sizeof(sParent)) != XUI_OK || strcmp(sParent, arrParents[i].sParent) != 0 ) return 0;
	}
	return 1;
}

static int __xuiFileDialogClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiFileDialogDoubleClickEntry(xui_context pContext, xui_file_dialog pDialog, int iIndex)
{
	xui_widget pList;
	xui_rect_t tListWorld;
	xui_rect_t tItem;
	float fX;
	float fY;
	int iRet;

	pList = xuiFileDialogGetFileListWidget(pDialog);
	if ( pList == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiLayout(pContext);
	if ( iRet != XUI_OK ) return iRet;
	tListWorld = xuiWidgetGetWorldRect(pList);
	tItem = xuiListViewGetItemRect(pList, iIndex);
	fX = tListWorld.fX + tItem.fX + tItem.fW * 0.5f;
	fY = tListWorld.fY + tItem.fY + tItem.fH * 0.5f;
	iRet = __xuiFileDialogClick(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiFileDialogClick(pContext, fX, fY);
}

static int __xuiFileDialogRightClickEntry(xui_context pContext, xui_file_dialog pDialog, int iIndex)
{
	xui_widget pList;
	xui_rect_t tListWorld;
	xui_rect_t tItem;
	float fX;
	float fY;
	int iRet;

	pList = xuiFileDialogGetFileListWidget(pDialog);
	if ( pList == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiLayout(pContext);
	if ( iRet != XUI_OK ) return iRet;
	tListWorld = xuiWidgetGetWorldRect(pList);
	tItem = xuiListViewGetItemRect(pList, iIndex);
	fX = tListWorld.fX + tItem.fX + 8.0f;
	fY = tListWorld.fY + tItem.fY + tItem.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY,
		XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

static int __xuiFileDialogClickWidget(xui_context pContext, xui_widget pWidget)
{
	xui_rect_t tWorld;
	int iRet;

	if ( pWidget == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiLayout(pContext);
	if ( iRet != XUI_OK ) return iRet;
	tWorld = xuiWidgetGetWorldRect(pWidget);
	return __xuiFileDialogClick(pContext, tWorld.fX + tWorld.fW * 0.5f, tWorld.fY + tWorld.fH * 0.5f);
}

#if defined(_WIN32)
static int __xuiFileDialogFindTreeText(xui_widget pTree, const char* sText)
{
	int i;
	for ( i = 0; i < xuiTreeViewGetVisibleCount(pTree); ++i ) {
		const xui_tree_view_node_t* pNode = xuiTreeViewGetVisibleNode(pTree, i);
		if ( pNode != NULL && pNode->sText != NULL && strcmp(pNode->sText, sText) == 0 ) return i;
	}
	return -1;
}

static int __xuiFileDialogClickTreeItem(xui_context pContext, xui_widget pTree, int iVisible)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;
	int iRet;
	if ( pTree == NULL || iVisible < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiLayout(pContext);
	if ( iRet != XUI_OK ) return iRet;
	tWorld = xuiWidgetGetWorldRect(pTree);
	tItem = xuiTreeViewGetItemRect(pTree, iVisible);
	return __xuiFileDialogClick(pContext, tWorld.fX + tItem.fX + tItem.fW * 0.75f,
		tWorld.fY + tItem.fY + tItem.fH * 0.5f);
}
#endif

static int __xuiFileDialogSendKey(xui_context pContext, int iKey)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiFileDialogSendText(xui_context pContext, uint32_t iCodepoint)
{
	int iRet;

	iRet = xuiInputText(pContext, iCodepoint);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiFileDialogRefreshOomPreserves(xui_file_dialog pDialog,
	const char* sPendingEntry)
{
	xallocationbudget* pBudget = NULL;
	xallocationscope tScope;
	char sDirectory[1024];
	char sFirstEntry[1024];
	xui_widget pList;
	int iEntryCount;
	int iSelected;
	int iAttempt;
	int iBudget;
	int iRet;
	int bSawOom = 0;

	if ( pDialog == NULL || sPendingEntry == NULL ||
	     xuiFileDialogGetEntryCount(pDialog) <= 0 ) {
		return 0;
	}
	pList = xuiFileDialogGetFileListWidget(pDialog);
	if ( pList == NULL ) {
		return 0;
	}
	snprintf(sDirectory, sizeof(sDirectory), "%s", xuiFileDialogGetDirectory(pDialog));
	snprintf(sFirstEntry, sizeof(sFirstEntry), "%s", xuiFileDialogGetEntryName(pDialog, 0));
	iEntryCount = xuiFileDialogGetEntryCount(pDialog);
	iSelected = xuiListViewGetSelected(pList);
	for ( iAttempt = 0; iAttempt <= 1024; iAttempt++ ) {
		iBudget = (iAttempt == 0) ? 1 : iAttempt * 256;
		memset(&tScope, 0, sizeof(tScope));
		if ( !xrtAllocationBudgetCreate((size_t)iBudget, &pBudget) ) {
			return 0;
		}
		if ( !xrtAllocationBudgetEnter(pBudget, &tScope) ) {
			xrtAllocationBudgetFree(pBudget);
			return 0;
		}
		iRet = xuiFileDialogRefresh(pDialog);
		xrtAllocationBudgetLeave(&tScope);
		xrtAllocationBudgetFree(pBudget);
		pBudget = NULL;
		if ( xuiListViewGetItemCount(pList) != xuiFileDialogGetEntryCount(pDialog) ) {
			return 0;
		}
		if ( iRet == XUI_ERROR_OUT_OF_MEMORY ) {
			bSawOom = 1;
			if ( xuiFileDialogGetEntryCount(pDialog) != iEntryCount ||
			     strcmp(xuiFileDialogGetDirectory(pDialog), sDirectory) != 0 ||
			     strcmp(xuiFileDialogGetEntryName(pDialog, 0), sFirstEntry) != 0 ||
			     xuiListViewGetSelected(pList) != iSelected ||
			     __xuiFileDialogFindEntry(pDialog, sPendingEntry) >= 0 ) {
				return 0;
			}
			continue;
		}
		if ( iRet != XUI_OK ) {
			return 0;
		}
		return bSawOom && xuiFileDialogGetEntryCount(pDialog) == iEntryCount + 1 &&
			xuiListViewGetItemCount(pList) == iEntryCount + 1 &&
			xuiListViewGetSelected(pList) == -1 &&
			__xuiFileDialogFindEntry(pDialog, sPendingEntry) >= 0;
	}
	return 0;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext = NULL;
	xui_font pFont = NULL;
	xui_widget pRoot = NULL;
	xui_widget pRootTree = NULL;
	xui_widget pRootViewport = NULL;
	xui_widget pBreadcrumb = NULL;
	xui_file_dialog pDialog = NULL;
	xui_surface pTarget = NULL;
	xui_surface pRootCache = NULL;
	xui_file_dialog_desc_t tDesc;
	xui_file_dialog_test_result_t tResult;
	xui_file_dialog_destroy_result_t tDestroyResult;
	xui_file_dialog_context_result_t tContextResult;
	xui_rect_i_t tFullRect;
	xui_rect_t tRect;
	xui_rect_t tWorld;
	str sFixture = NULL;
	str sSub = NULL;
	str sSubTxt = NULL;
	str sSubSave = NULL;
	str sTxt = NULL;
	str sBin = NULL;
	str sLate = NULL;
	str sSave = NULL;
	str sMissingDir = NULL;
	int iFailed = 0;
	int iRet;
	int iIndex;
	int iBulk;
	int iBackgroundClicks = 0;
	float fRootItemHeight;
#if defined(_WIN32)
	int iUser;
	int iThisPc;
	int iDrive;
	int iUserChildren;
#endif
	char sBulkName[64];
	str sBulkPath;

	xuiTestProxyInit(&tState);
	XUI_TEST_CHECK(__xuiFileDialogPathHelpersValid(),
		"drive, UNC, and extended path roots preserve parent boundaries");
	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy");
	iRet = xuiInputViewport(pContext, 800.0f, 520.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "body", 4, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	(void)xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 800.0f, 520.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");
	iRet = xuiWidgetSetEventHandler(pRoot, XUI_EVENT_POINTER_CLICK,
		__xuiFileDialogBackgroundClick, &iBackgroundClicks);
	XUI_TEST_CHECK(iRet == XUI_OK, "background click handler");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fWidth = NAN;
	pDialog = (xui_file_dialog)(size_t)1;
	iRet = xuiFileDialogCreate(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pDialog == NULL,
		"nan width rejected and output cleared");
	tDesc.fWidth = 0.0f;
	tDesc.fHeight = INFINITY;
	pDialog = (xui_file_dialog)(size_t)1;
	iRet = xuiFileDialogCreate(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pDialog == NULL,
		"infinite height rejected and output cleared");
	tDesc.fHeight = 0.0f;
	tDesc.iSize = sizeof(tDesc) - 1u;
	pDialog = (xui_file_dialog)(size_t)1;
	iRet = xuiOpenFileDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pDialog == NULL,
		"short convenience descriptor rejected before copy");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = ".";
	tDesc.fWidth = 100.0f;
	tDesc.fHeight = 100.0f;
	iRet = xuiFileDialogCreate(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL, "undersized dialog create");
	tRect = xuiWidgetGetRect(xuiFileDialogGetWindowWidget(pDialog));
	XUI_TEST_CHECK(tRect.fW >= 560.0f && tRect.fH >= 430.0f,
		"initial dialog size respects window minimum");
	xuiFileDialogDestroy(pDialog);
	pDialog = NULL;

	sFixture = xrtPathJoin("build", "file_dialog_fixture");
	sSub = xrtPathJoin(sFixture, "sub");
	sSubTxt = xrtPathJoin(sSub, "inside.txt");
	sSubSave = xrtPathJoin(sSub, "new.txt");
	sTxt = xrtPathJoin(sFixture, "alpha.txt");
	sBin = xrtPathJoin(sFixture, "beta.bin");
	sLate = xrtPathJoin(sFixture, "late.txt");
	sSave = xrtPathJoin(sFixture, "new.txt");
	sMissingDir = xrtPathJoin(sFixture, "missing_dir");
	XUI_TEST_CHECK(sFixture != NULL && sSub != NULL && sSubTxt != NULL && sSubSave != NULL && sTxt != NULL && sBin != NULL && sLate != NULL && sSave != NULL && sMissingDir != NULL, "fixture paths");
	(void)xrtDirRemoveAll(sFixture);
	XUI_TEST_CHECK(xrtDirCreateAll(sSub) == TRUE, "fixture dirs");
	XUI_TEST_CHECK(xrtFileWriteAll(sSubTxt, (xbytesview){ (const unsigned char*)"inside", 6 }), "fixture sub txt");
	XUI_TEST_CHECK(xrtFileWriteAll(sTxt, (xbytesview){ (const unsigned char*)"alpha", 5 }), "fixture txt");
	XUI_TEST_CHECK(xrtFileWriteAll(sBin, (xbytesview){ (const unsigned char*)"beta", 4 }), "fixture bin");
	for ( iBulk = 0; iBulk < 48; iBulk++ ) {
		snprintf(sBulkName, sizeof(sBulkName), "zbulk_%02d.txt", iBulk);
		sBulkPath = xrtPathJoin(sFixture, sBulkName);
		XUI_TEST_CHECK(sBulkPath != NULL, "bulk fixture path");
		XUI_TEST_CHECK(xrtFileWriteAll(sBulkPath, (xbytesview){ (const unsigned char*)"bulk", 4 }), "bulk fixture file");
		xrtFree(sBulkPath);
	}
	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 800, 520, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	tFullRect = (xui_rect_i_t){0, 0, 800, 520};

	memset(&tResult, 0, sizeof(tResult));
	memset(&tContextResult, 0, sizeof(tContextResult));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = (const char*)sFixture;
	tDesc.sFilter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*";
	tDesc.onResult = __xuiFileDialogTestResult;
	tDesc.pResultUser = &tResult;
	iRet = xuiOpenFileDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL && xuiFileDialogIsOpen(pDialog), "open dialog");
	iRet = xuiFileDialogSetContextMenu(pDialog, __xuiFileDialogTestContext, &tContextResult);
	XUI_TEST_CHECK(iRet == XUI_OK, "file dialog context callback");
	XUI_TEST_CHECK(xuiFileDialogIsModal(pDialog), "convenience file dialog is modal");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout modal dialog");
	iRet = __xuiFileDialogClick(pContext, 4.0f, 4.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && iBackgroundClicks == 0, "modal dialog blocks background input");
	iRet = xuiFileDialogSetModal(pDialog, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiFileDialogIsModal(pDialog), "dialog switches to modeless");
	iRet = __xuiFileDialogClick(pContext, 4.0f, 4.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && iBackgroundClicks == 1, "modeless dialog permits background input");
	iRet = xuiFileDialogSetModal(pDialog, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFileDialogIsModal(pDialog), "dialog restores modal state");
	iRet = __xuiFileDialogClick(pContext, 4.0f, 4.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && iBackgroundClicks == 1, "restored modal dialog blocks background input");
	XUI_TEST_CHECK(xuiFileDialogGetWindowWidget(pDialog) != NULL, "window getter");
	XUI_TEST_CHECK(xuiFileDialogGetRootListWidget(pDialog) != NULL, "root list getter");
	pRootTree = xuiFileDialogGetRootListWidget(pDialog);
	XUI_TEST_CHECK(xuiWidgetIsType(pRootTree, xuiTreeViewGetType(pContext)), "root navigation uses tree view");
	XUI_TEST_CHECK(xuiTreeViewGetVisibleCount(pRootTree) > 0, "root tree has visible nodes");
#if defined(_WIN32)
	iUser = __xuiFileDialogFindTreeText(pRootTree, xuiTranslate(pContext, XUI_TR_FILE_USER));
	iThisPc = __xuiFileDialogFindTreeText(pRootTree, xuiTranslate(pContext, XUI_TR_FILE_THIS_PC));
	XUI_TEST_CHECK(iUser >= 0 && iThisPc >= 0, "windows user and this pc groups");
	{
		const xui_tree_view_node_t* pUserNode = xuiTreeViewGetVisibleNode(pRootTree, iUser);
		const xui_tree_view_node_t* pComputerNode = xuiTreeViewGetVisibleNode(pRootTree, iThisPc);
		iDrive = -1;
		iUserChildren = 0;
		for ( iIndex = 0; iIndex < xuiTreeViewGetVisibleCount(pRootTree); ++iIndex ) {
			const xui_tree_view_node_t* pNode = xuiTreeViewGetVisibleNode(pRootTree, iIndex);
			if ( pNode != NULL && pUserNode != NULL && pNode->iParent == pUserNode->iId ) iUserChildren++;
			if ( pNode != NULL && pComputerNode != NULL && pNode->iParent == pComputerNode->iId && iDrive < 0 ) iDrive = iIndex;
		}
		XUI_TEST_CHECK(iUserChildren == 6, "windows user group has six known folders");
		XUI_TEST_CHECK(iDrive >= 0, "this pc group has a drive");
	}
	iRet = __xuiFileDialogClickTreeItem(pContext, pRootTree, iThisPc);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sFixture) == 0,
		"this pc group is not navigable");
	{
		const xui_tree_view_node_t* pDriveNode = xuiTreeViewGetVisibleNode(pRootTree, iDrive);
		char sDrive[32];
		XUI_TEST_CHECK(pDriveNode != NULL && pDriveNode->sText != NULL, "drive node text");
		snprintf(sDrive, sizeof(sDrive), "%s", pDriveNode->sText);
		iRet = __xuiFileDialogClickTreeItem(pContext, pRootTree, iDrive);
		XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), sDrive) == 0,
			"drive node navigates to root");
		iRet = xuiFileDialogSetDirectory(pDialog, (const char*)sFixture);
		XUI_TEST_CHECK(iRet == XUI_OK, "restore fixture after drive navigation");
	}
#endif
	XUI_TEST_CHECK(xuiFileDialogGetFileListWidget(pDialog) != NULL, "file list getter");
	XUI_TEST_CHECK(xuiFileDialogGetPathBreadcrumbWidget(pDialog) != NULL, "path breadcrumb getter");
	XUI_TEST_CHECK(xuiFileDialogGetFilterComboWidget(pDialog) != NULL, "filter combo getter");
	XUI_TEST_CHECK(xuiFileDialogGetOkButtonWidget(pDialog) != NULL, "ok button getter");
	XUI_TEST_CHECK(xuiFileDialogGetCancelButtonWidget(pDialog) != NULL, "cancel button getter");
	XUI_TEST_CHECK(xuiFileDialogGetFilterCount(pDialog) == 2, "filter count");
	XUI_TEST_CHECK(strcmp(xuiFileDialogGetFilterName(pDialog, 0), "Text Files (*.txt)") == 0, "filter name");
	XUI_TEST_CHECK(strcmp(xuiFileDialogGetFilterPattern(pDialog, 1), "*.*") == 0, "filter pattern");
	XUI_TEST_CHECK(strcmp(xuiFileDialogGetFilter(pDialog), "*.txt") == 0, "active filter");
	XUI_TEST_CHECK(xuiFileDialogGetEntryCount(pDialog) >= 2 &&
		xuiFileDialogEntryIsDir(pDialog, 0) &&
		strcmp(xuiFileDialogGetEntryName(pDialog, 0), "sub") == 0 &&
		strcmp(xuiFileDialogGetEntryName(pDialog, 1), "alpha.txt") == 0,
		"entries sort directories first then names");
	XUI_TEST_CHECK(strcmp(xuiListViewGetItemText(xuiFileDialogGetFileListWidget(pDialog), 0),
		xuiFileDialogGetEntryName(pDialog, 0)) == 0,
		"virtual file list reads canonical entry text");
	iIndex = __xuiFileDialogFindEntry(pDialog, "alpha.txt");
	iRet = __xuiFileDialogRightClickEntry(pContext, pDialog, iIndex);
	XUI_TEST_CHECK(iRet == XUI_OK && tContextResult.iCount == 1 &&
		tContextResult.iIndex == iIndex && !tContextResult.bDirectory &&
		strcmp(tContextResult.sPath, (const char*)sTxt) == 0,
		"file list context callback receives selected path");
	iRet = xuiInputSetText(xuiFileDialogGetNameInputWidget(pDialog), "preserve.txt");
	XUI_TEST_CHECK(iRet == XUI_OK, "name before failed directory change");
	iIndex = xuiFileDialogGetEntryCount(pDialog);
	iRet = xuiFileDialogSetDirectory(pDialog, (const char*)sMissingDir);
	XUI_TEST_CHECK(iRet == XUI_ERROR_FILE_NOT_FOUND &&
		strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sFixture) == 0 &&
		xuiFileDialogGetEntryCount(pDialog) == iIndex &&
		strcmp(xuiInputGetText(xuiFileDialogGetNameInputWidget(pDialog)), "preserve.txt") == 0,
		"missing directory preserves current state");
	(void)xuiInputSetText(xuiFileDialogGetNameInputWidget(pDialog), "");
	XUI_TEST_CHECK(xuiWidgetGetVisible(xuiFileDialogGetPathBreadcrumbWidget(pDialog)) && !xuiWidgetGetVisible(xuiFileDialogGetPathInputWidget(pDialog)), "path starts as breadcrumb");
	XUI_TEST_CHECK(xuiFileDialogGetUpButtonWidget(pDialog) != NULL, "up button getter");
	XUI_TEST_CHECK(xuiFileDialogGetRefreshButtonWidget(pDialog) != NULL, "refresh button getter");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dialog layout");
	tRect = xuiWidgetGetWorldRect(xuiFileDialogGetNameInputWidget(pDialog));
	tWorld = xuiWidgetGetWorldRect(xuiFileDialogGetFilterComboWidget(pDialog));
	XUI_TEST_CHECK(tRect.fY == tWorld.fY && tRect.fH == tWorld.fH && tRect.fH >= 28.0f, "name and filter fixed row height");
	XUI_TEST_CHECK(xuiWidgetGetWorldRect(xuiWidgetGetParent(xuiFileDialogGetOkButtonWidget(pDialog))).fH >= 28.0f, "button fixed row height");
	XUI_TEST_CHECK(xuiWidgetGetWorldRect(xuiFileDialogGetOkButtonWidget(pDialog)).fH >= 24.0f, "button control height");
	XUI_TEST_CHECK(xuiWidgetGetWorldRect(xuiFileDialogGetOkButtonWidget(pDialog)).fY > tRect.fY, "buttons below fields");
	tRect = xuiWidgetGetWorldRect(xuiFileDialogGetOkButtonWidget(pDialog));
	tWorld = xuiWidgetGetWorldRect(xuiFileDialogGetWindowWidget(pDialog));
	XUI_TEST_CHECK(tRect.fY + tRect.fH <= tWorld.fY + tWorld.fH, "buttons remain inside dialog");
	tRect = xuiWidgetGetWorldRect(xuiFileDialogGetRootListWidget(pDialog));
	tWorld = xuiWidgetGetWorldRect(xuiFileDialogGetFileListWidget(pDialog));
	XUI_TEST_CHECK(tRect.fH == tWorld.fH, "root list fills list row");
	tRect = xuiWidgetGetWorldRect(xuiFileDialogGetWindowWidget(pDialog));
	XUI_TEST_CHECK(tRect.fX >= 0.0f && tRect.fY >= 0.0f && (tRect.fY + tRect.fH) <= 520.0f, "dialog initially visible");
	(void)xuiWidgetSetRect(xuiFileDialogGetWindowWidget(pDialog), (xui_rect_t){72.0f, 48.0f, 620.0f, 430.0f});
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dialog relayout after resize");
	tRect = xuiWidgetGetWorldRect(xuiFileDialogGetNameInputWidget(pDialog));
	tWorld = xuiWidgetGetWorldRect(xuiFileDialogGetFilterComboWidget(pDialog));
	XUI_TEST_CHECK(tRect.fY == tWorld.fY && tRect.fH >= 28.0f && tRect.fW >= 120.0f && tWorld.fW >= 176.0f, "resized name filter row");
	XUI_TEST_CHECK(xuiWidgetGetWorldRect(xuiWidgetGetParent(xuiFileDialogGetOkButtonWidget(pDialog))).fH >= 28.0f, "resized button row height");
	XUI_TEST_CHECK(xuiWidgetGetWorldRect(xuiFileDialogGetOkButtonWidget(pDialog)).fY > tRect.fY, "resized buttons below fields");
	tRect = xuiWidgetGetWorldRect(xuiFileDialogGetOkButtonWidget(pDialog));
	tWorld = xuiWidgetGetWorldRect(xuiFileDialogGetWindowWidget(pDialog));
	XUI_TEST_CHECK(tRect.fY + tRect.fH <= tWorld.fY + tWorld.fH, "resized buttons remain inside dialog");
	XUI_TEST_CHECK(xuiWidgetGetWorldRect(xuiFileDialogGetFileListWidget(pDialog)).fH >= 72.0f, "resized file list height");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "dialog render");
	pRootViewport = xuiTreeViewGetViewportWidget(pRootTree);
	XUI_TEST_CHECK(pRootViewport != NULL, "root tree viewport");
	pRootCache = xuiWidgetGetCacheSurface(pRootViewport, xuiWidgetGetStateId(pRootViewport));
	XUI_TEST_CHECK(pRootCache != NULL, "root tree viewport cache");
	fRootItemHeight = 0.0f;
	XUI_TEST_CHECK(xuiTreeViewGetMetrics(pRootTree, &fRootItemHeight, NULL, NULL, NULL) == XUI_OK &&
		fRootItemHeight > 0.0f, "root tree item height");
	XUI_TEST_CHECK(xuiTestSurfaceGetTextDrawCount(pRootCache) > 0 &&
		xuiTestSurfaceGetTextDrawCount(pRootCache) <=
			(int)(xuiWidgetGetWorldRect(pRootViewport).fH / fRootItemHeight) + 2,
		"root custom renderer must not fall through to default rendering");
	pBreadcrumb = xuiFileDialogGetPathBreadcrumbWidget(pDialog);
	XUI_TEST_CHECK(xuiBreadcrumbGetItemCount(pBreadcrumb) >= 2, "breadcrumb items");
	tWorld = xuiWidgetGetWorldRect(pBreadcrumb);
	iRet = __xuiFileDialogClick(pContext, tWorld.fX + tWorld.fW - 4.0f, tWorld.fY + tWorld.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetVisible(xuiFileDialogGetPathInputWidget(pDialog)) && xuiGetFocusWidget(pContext) == xuiFileDialogGetPathInputWidget(pDialog), "breadcrumb empty click edits path");
	iRet = xuiInputSetText(xuiFileDialogGetPathInputWidget(pDialog), (const char*)sSub);
	XUI_TEST_CHECK(iRet == XUI_OK, "path input edit set");
	iRet = __xuiFileDialogSendKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sSub) == 0 && !xuiWidgetGetVisible(xuiFileDialogGetPathInputWidget(pDialog)), "path enter navigates and returns to breadcrumb");
	iRet = xuiFileDialogGoUp(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sFixture) == 0, "up after breadcrumb edit");
	iIndex = __xuiFileDialogFindEntry(pDialog, "sub");
	XUI_TEST_CHECK(iIndex >= 0, "directory visible");
	iRet = xuiFileDialogSelectIndex(pDialog, iIndex);
	XUI_TEST_CHECK(iRet == XUI_OK, "first programmatic folder select");
	iRet = xuiFileDialogSelectIndex(pDialog, iIndex);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sFixture) == 0,
		"repeat selection does not impersonate double click");
	iRet = __xuiFileDialogDoubleClickEntry(pContext, pDialog, iIndex);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sSub) == 0, "double click enters directory");
	iRet = __xuiFileDialogClickWidget(pContext, xuiFileDialogGetUpButtonWidget(pDialog));
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sFixture) == 0, "up button enters parent directory");
	iIndex = __xuiFileDialogFindEntry(pDialog, "sub");
	iRet = xuiFileDialogSelectIndex(pDialog, iIndex);
	if ( iRet == XUI_OK ) iRet = xuiSetFocusWidget(pContext, xuiFileDialogGetFileListWidget(pDialog));
	if ( iRet == XUI_OK ) iRet = __xuiFileDialogSendKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sSub) == 0,
		"enter activates selected directory");
	iRet = xuiFileDialogGoUp(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sFixture) == 0,
		"up after keyboard activation");
	iIndex = __xuiFileDialogFindEntry(pDialog, "alpha.txt");
	XUI_TEST_CHECK(iIndex >= 0 && xuiFileDialogSelectIndex(pDialog, iIndex) == XUI_OK,
		"select entry before transactional refresh");
	XUI_TEST_CHECK(xrtFileWriteAll(sLate, (xbytesview){ (const unsigned char*)"late", 4 }), "late txt");
	XUI_TEST_CHECK(__xuiFileDialogFindEntry(pDialog, "late.txt") < 0,
		"late file remains absent before refresh");
	XUI_TEST_CHECK(__xuiFileDialogRefreshOomPreserves(pDialog, "late.txt"),
		"refresh allocation budget sweep preserves entries and selection atomically");
	iRet = __xuiFileDialogClickWidget(pContext, xuiFileDialogGetRefreshButtonWidget(pDialog));
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiFileDialogFindEntry(pDialog, "late.txt") >= 0, "refresh button reloads entries");
	(void)xuiInputSetText(xuiFileDialogGetNameInputWidget(pDialog), "");
	iRet = xuiFileDialogSetDirectory(pDialog, (const char*)sSub);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sSub) == 0, "enter sub before breadcrumb segment");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout before breadcrumb click");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before breadcrumb click");
	pBreadcrumb = xuiFileDialogGetPathBreadcrumbWidget(pDialog);
	tRect = xuiBreadcrumbGetItemRect(pBreadcrumb, xuiBreadcrumbGetItemCount(pBreadcrumb) - 2);
	tWorld = xuiWidgetGetWorldRect(pBreadcrumb);
	iRet = __xuiFileDialogClick(pContext, tWorld.fX + tRect.fX + tRect.fW * 0.5f, tWorld.fY + tRect.fY + tRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sFixture) == 0, "breadcrumb segment navigates");
	tWorld = xuiWidgetGetWorldRect(pBreadcrumb);
	iRet = __xuiFileDialogClick(pContext, tWorld.fX + tWorld.fW - 4.0f, tWorld.fY + tWorld.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetVisible(xuiFileDialogGetPathInputWidget(pDialog)), "path edit opens again");
	iRet = xuiInputSetText(xuiFileDialogGetPathInputWidget(pDialog), (const char*)sMissingDir);
	XUI_TEST_CHECK(iRet == XUI_OK, "path input missing set");
	iRet = __xuiFileDialogSendKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sFixture) == 0 && xuiInputGetError(xuiFileDialogGetPathInputWidget(pDialog)), "missing path shows error");
	XUI_TEST_CHECK(xuiFileDialogTestPathTipOpen(pDialog),
		"missing path opens error tip");
	XUI_TEST_CHECK(xuiInputSetError(xuiFileDialogGetNameInputWidget(pDialog), 1) == XUI_OK,
		"set transient name error before close");
	iRet = xuiFileDialogSetOpen(pDialog, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiFileDialogIsOpen(pDialog) &&
		!xuiWidgetGetVisible(xuiFileDialogGetPathInputWidget(pDialog)) &&
		xuiWidgetGetVisible(xuiFileDialogGetPathBreadcrumbWidget(pDialog)) &&
		!xuiInputGetError(xuiFileDialogGetPathInputWidget(pDialog)) &&
		!xuiInputGetError(xuiFileDialogGetNameInputWidget(pDialog)) &&
		!xuiFileDialogTestPathTipOpen(pDialog),
		"close clears transient path session state");
	iRet = xuiFileDialogSetOpen(pDialog, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFileDialogIsOpen(pDialog) &&
		!xuiWidgetGetVisible(xuiFileDialogGetPathInputWidget(pDialog)) &&
		xuiWidgetGetVisible(xuiFileDialogGetPathBreadcrumbWidget(pDialog)),
		"reopen starts in breadcrumb mode");
	pBreadcrumb = xuiFileDialogGetPathBreadcrumbWidget(pDialog);
	tWorld = xuiWidgetGetWorldRect(pBreadcrumb);
	iRet = __xuiFileDialogClick(pContext,
		tWorld.fX + tWorld.fW - 4.0f, tWorld.fY + tWorld.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiWidgetGetVisible(xuiFileDialogGetPathInputWidget(pDialog)) &&
		strcmp(xuiInputGetText(xuiFileDialogGetPathInputWidget(pDialog)),
			xuiFileDialogGetDirectory(pDialog)) == 0,
		"reopened path edit restores current directory");
	iRet = xuiSetFocusWidget(pContext, xuiFileDialogGetPathInputWidget(pDialog));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == xuiFileDialogGetPathInputWidget(pDialog), "path focus before blur");
	iRet = xuiInputSetText(xuiFileDialogGetPathInputWidget(pDialog), (const char*)sSub);
	XUI_TEST_CHECK(iRet == XUI_OK, "path input valid for blur");
	iRet = __xuiFileDialogClickWidget(pContext, xuiFileDialogGetNameInputWidget(pDialog));
	XUI_TEST_CHECK(iRet == XUI_OK, "click away for blur");
	XUI_TEST_CHECK(strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sSub) == 0, "path blur directory");
	XUI_TEST_CHECK(!xuiWidgetGetVisible(xuiFileDialogGetPathInputWidget(pDialog)), "path blur returns breadcrumb");
	iRet = xuiFileDialogGoUp(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sFixture) == 0, "up after path blur");
	iRet = xuiInputSetText(xuiFileDialogGetNameInputWidget(pDialog), "sub");
	XUI_TEST_CHECK(iRet == XUI_OK, "name input set directory");
	(void)xuiSetFocusWidget(pContext, xuiFileDialogGetNameInputWidget(pDialog));
	iRet = __xuiFileDialogSendKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sSub) == 0, "name enter navigates directory");
	iRet = xuiFileDialogGoUp(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sFixture) == 0, "up after name enter");
	iRet = xuiInputSetText(xuiFileDialogGetNameInputWidget(pDialog), "missing.txt");
	XUI_TEST_CHECK(iRet == XUI_OK, "name input set missing");
	iRet = __xuiFileDialogSendKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFileDialogIsOpen(pDialog) && xuiInputGetError(xuiFileDialogGetNameInputWidget(pDialog)), "open missing file stays open");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_ERROR_FILE_NOT_FOUND && xuiFileDialogIsOpen(pDialog) &&
		tResult.iCount == 0 && xuiInputGetError(xuiFileDialogGetNameInputWidget(pDialog)),
		"public commit rejects missing open file");
	iIndex = __xuiFileDialogFindEntry(pDialog, "alpha.txt");
	XUI_TEST_CHECK(iIndex >= 0, "txt visible");
	XUI_TEST_CHECK(__xuiFileDialogFindEntry(pDialog, "beta.bin") < 0, "filter hides bin");
	iRet = xuiFileDialogSetFilter(pDialog, "Text Files (*.txt)|*.txt|All Files (*.*)|*.*");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetFilter(pDialog), "*.txt") == 0, "reset filter");
	iRet = xuiFileDialogSetFilter(pDialog, "All Files (*.*)|*.*");
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiFileDialogFindEntry(pDialog, "beta.bin") >= 0, "all filter shows bin");
	iRet = xuiFileDialogSetFilter(pDialog, "Text Files (*.txt)|*.txt|All Files (*.*)|*.*");
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiFileDialogFindEntry(pDialog, "beta.bin") < 0, "text filter hides bin again");
	iIndex = __xuiFileDialogFindEntry(pDialog, "alpha.txt");
	XUI_TEST_CHECK(iIndex >= 0, "txt visible after filter reset");
	iRet = xuiFileDialogSelectIndex(pDialog, iIndex);
	XUI_TEST_CHECK(iRet == XUI_OK, "select file");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiFileDialogIsOpen(pDialog), "commit open");
	XUI_TEST_CHECK(tResult.iCount == 1 && tResult.iResult == XUI_FILE_DIALOG_RESULT_OK, "open result callback");
	XUI_TEST_CHECK(strcmp(xuiFileDialogGetResultPath(pDialog), (const char*)sTxt) == 0, "open result path");
	iRet = xuiFileDialogSetOpen(pDialog, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFileDialogIsOpen(pDialog), "reuse dialog open");
	XUI_TEST_CHECK(xuiFileDialogGetResult(pDialog) == XUI_FILE_DIALOG_RESULT_NONE &&
		xuiFileDialogGetResultPath(pDialog)[0] == 0, "reuse dialog resets stale result");
	iRet = xuiFileDialogCancel(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFileDialogGetResult(pDialog) == XUI_FILE_DIALOG_RESULT_CANCEL &&
		xuiFileDialogGetResultPath(pDialog)[0] == 0, "reuse dialog cancel keeps result path empty");
	xuiFileDialogDestroy(pDialog);
	pDialog = NULL;

	memset(&tResult, 0, sizeof(tResult));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = (const char*)sFixture;
	tDesc.sFilter = "Text Files (*.txt)|*.txt";
	tDesc.onResult = __xuiFileDialogTestResult;
	tDesc.pResultUser = &tResult;
	iRet = xuiOpenFileDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL, "directory switch open dialog");
	iIndex = __xuiFileDialogFindEntry(pDialog, "sub");
	XUI_TEST_CHECK(iIndex >= 0, "directory switch open entry");
	iRet = xuiFileDialogSelectIndex(pDialog, iIndex);
	XUI_TEST_CHECK(iRet == XUI_OK, "directory switch select folder");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sSub) == 0, "directory switch enters folder");
	XUI_TEST_CHECK(strcmp(xuiInputGetText(xuiFileDialogGetNameInputWidget(pDialog)), "") == 0, "directory switch clears open name");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiFileDialogIsOpen(pDialog) && tResult.iCount == 0, "directory switch clears stale selection");
	iIndex = __xuiFileDialogFindEntry(pDialog, "inside.txt");
	XUI_TEST_CHECK(iIndex >= 0, "directory switch target visible");
	iRet = xuiFileDialogSelectIndex(pDialog, iIndex);
	XUI_TEST_CHECK(iRet == XUI_OK, "directory switch target select");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetResultPath(pDialog), (const char*)sSubTxt) == 0, "directory switch open result path");
	xuiFileDialogDestroy(pDialog);
	pDialog = NULL;

	memset(&tResult, 0, sizeof(tResult));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = (const char*)sFixture;
	tDesc.sFilter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*";
	tDesc.sFileName = "new";
	tDesc.onResult = __xuiFileDialogTestResult;
	tDesc.pResultUser = &tResult;
	iRet = xuiSaveFileDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL, "save dialog");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == xuiFileDialogGetNameInputWidget(pDialog), "save name input default focus");
	XUI_TEST_CHECK(xuiInputHasSelection(xuiFileDialogGetNameInputWidget(pDialog)), "save name input selected");
	iRet = __xuiFileDialogClickWidget(pContext, xuiFileDialogGetNameInputWidget(pDialog));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == xuiFileDialogGetNameInputWidget(pDialog), "save name input focus by click");
	iRet = __xuiFileDialogSendText(pContext, '2');
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(xuiFileDialogGetNameInputWidget(pDialog)), "new2") == 0, "save name input accepts text");
	iRet = xuiInputSetText(xuiFileDialogGetNameInputWidget(pDialog), "new");
	XUI_TEST_CHECK(iRet == XUI_OK, "save name reset after typing");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetResultPath(pDialog), (const char*)sSave) == 0, "save appends default extension");
	XUI_TEST_CHECK(tResult.iCount == 1 && strcmp(tResult.sPath, (const char*)sSave) == 0, "save callback path");
	xuiFileDialogDestroy(pDialog);
	pDialog = NULL;

	memset(&tResult, 0, sizeof(tResult));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = (const char*)sFixture;
	tDesc.sFilter = "Text Files (*.txt)|*.txt";
	tDesc.sFileName = "new";
	tDesc.onResult = __xuiFileDialogTestResult;
	tDesc.pResultUser = &tResult;
	iRet = xuiSaveFileDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL, "directory switch save dialog");
	iIndex = __xuiFileDialogFindEntry(pDialog, "sub");
	XUI_TEST_CHECK(iIndex >= 0, "directory switch save entry");
	iRet = xuiFileDialogSelectIndex(pDialog, iIndex);
	XUI_TEST_CHECK(iRet == XUI_OK, "directory switch save folder select");
	XUI_TEST_CHECK(strcmp(xuiInputGetText(xuiFileDialogGetNameInputWidget(pDialog)), "new") == 0, "save folder selection preserves name");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sSub) == 0, "directory switch save enters folder");
	XUI_TEST_CHECK(strcmp(xuiInputGetText(xuiFileDialogGetNameInputWidget(pDialog)), "new") == 0, "save directory switch preserves name");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetResultPath(pDialog), (const char*)sSubSave) == 0, "directory switch save result path");
	xuiFileDialogDestroy(pDialog);
	pDialog = NULL;

	memset(&tResult, 0, sizeof(tResult));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = (const char*)sFixture;
	tDesc.sFilter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*";
	tDesc.sFileName = "alpha";
	tDesc.onResult = __xuiFileDialogTestResult;
	tDesc.pResultUser = &tResult;
	iRet = xuiSaveFileDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL, "save overwrite dialog");
	XUI_TEST_CHECK(xrtFileExists(sTxt), "save overwrite fixture exists");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK, "save overwrite commit");
	XUI_TEST_CHECK(xuiFileDialogIsOpen(pDialog), "save overwrite keeps dialog open");
	XUI_TEST_CHECK(xuiFileDialogGetOverwriteMsgBox(pDialog) != NULL,
		"save overwrite creates confirmation");
	XUI_TEST_CHECK(xuiMsgBoxIsOpen(xuiFileDialogGetOverwriteMsgBox(pDialog)),
		"save overwrite opens confirmation");
	{
		int iDialogLayer = 0;
		int iDialogZ = 0;
		int iBackdropLayer = 0;
		int iBackdropZ = 0;
		xui_msgbox pOverwrite = xuiFileDialogGetOverwriteMsgBox(pDialog);
		iRet = xuiWidgetGetLayer(xuiFileDialogGetWindowWidget(pDialog), &iDialogLayer, &iDialogZ);
		if ( iRet == XUI_OK ) iRet = xuiWidgetGetLayer(xuiMsgBoxGetBackdropWidget(pOverwrite),
			&iBackdropLayer, &iBackdropZ);
		XUI_TEST_CHECK(iRet == XUI_OK && iDialogLayer == XUI_LAYER_MODAL &&
			iBackdropLayer == XUI_LAYER_MODAL && iBackdropZ > iDialogZ,
			"overwrite confirmation backdrop stacks above file dialog");
	}
	iRet = __xuiFileDialogClickWidget(pContext, xuiFileDialogGetCancelButtonWidget(pDialog));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFileDialogIsOpen(pDialog) &&
		xuiMsgBoxIsOpen(xuiFileDialogGetOverwriteMsgBox(pDialog)),
		"overwrite confirmation blocks file dialog input");
	iRet = __xuiFileDialogClickWidget(pContext, xuiMsgBoxGetButtonWidget(xuiFileDialogGetOverwriteMsgBox(pDialog), 1));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFileDialogIsOpen(pDialog) && tResult.iCount == 0, "save overwrite no keeps open");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgBoxIsOpen(xuiFileDialogGetOverwriteMsgBox(pDialog)), "save overwrite asks again");
	iRet = __xuiFileDialogClickWidget(pContext, xuiMsgBoxGetButtonWidget(xuiFileDialogGetOverwriteMsgBox(pDialog), 0));
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiFileDialogIsOpen(pDialog), "save overwrite yes closes");
	XUI_TEST_CHECK(tResult.iCount == 1 && strcmp(tResult.sPath, (const char*)sTxt) == 0, "save overwrite callback path");
	xuiFileDialogDestroy(pDialog);
	pDialog = NULL;

	memset(&tResult, 0, sizeof(tResult));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = (const char*)sFixture;
	tDesc.onResult = __xuiFileDialogTestResult;
	tDesc.pResultUser = &tResult;
	iRet = xuiSelectFolderDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL, "folder dialog");
	XUI_TEST_CHECK(xuiInputIsReadonly(xuiFileDialogGetNameInputWidget(pDialog)), "folder name readonly");
	XUI_TEST_CHECK(__xuiFileDialogFindEntry(pDialog, "alpha.txt") < 0, "folder hides files");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetResultPath(pDialog), (const char*)sFixture) == 0, "folder current result path");
	xuiFileDialogDestroy(pDialog);
	pDialog = NULL;

	memset(&tResult, 0, sizeof(tResult));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = (const char*)sFixture;
	tDesc.onResult = __xuiFileDialogTestResult;
	tDesc.pResultUser = &tResult;
	iRet = xuiSelectFolderDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL, "folder selected dialog");
	iIndex = __xuiFileDialogFindEntry(pDialog, "sub");
	XUI_TEST_CHECK(iIndex >= 0 && xuiFileDialogEntryIsDir(pDialog, iIndex), "folder entry");
	iRet = xuiFileDialogSelectIndex(pDialog, iIndex);
	XUI_TEST_CHECK(iRet == XUI_OK, "select folder");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetResultPath(pDialog), (const char*)sSub) == 0, "folder result path");
	xuiFileDialogDestroy(pDialog);
	pDialog = NULL;

	memset(&tResult, 0, sizeof(tResult));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = (const char*)sFixture;
	tDesc.onResult = __xuiFileDialogTestResult;
	tDesc.pResultUser = &tResult;
	iRet = xuiSelectFolderDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL, "folder double dialog");
	iIndex = __xuiFileDialogFindEntry(pDialog, "sub");
	XUI_TEST_CHECK(iIndex >= 0, "folder double entry");
	iRet = __xuiFileDialogDoubleClickEntry(pContext, pDialog, iIndex);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFileDialogIsOpen(pDialog), "folder double keeps open");
	XUI_TEST_CHECK(strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sSub) == 0, "folder double enters");
	xuiFileDialogDestroy(pDialog);
	pDialog = NULL;

	memset(&tResult, 0, sizeof(tResult));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = "";
	tDesc.sFileName = "new-file.txt";
	tDesc.onResult = __xuiFileDialogTestResult;
	tDesc.pResultUser = &tResult;
	iRet = xuiSelectFolderDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL, "folder root dialog");
	XUI_TEST_CHECK(strcmp(xuiInputGetText(xuiFileDialogGetNameInputWidget(pDialog)), "") == 0, "folder ignores file name");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFileDialogIsOpen(pDialog) && tResult.iCount == 0, "folder root requires selection");
	xuiFileDialogDestroy(pDialog);
	pDialog = NULL;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = "";
	iRet = xuiOpenFileDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFileDialogGetEntryCount(pDialog) > 0, "virtual roots");
	(void)xuiSetFocusWidget(pContext, xuiFileDialogGetNameInputWidget(pDialog));
	iRet = __xuiFileDialogSendKey(pContext, XUI_KEY_ESCAPE);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiFileDialogIsOpen(pDialog) && xuiFileDialogGetResult(pDialog) == XUI_FILE_DIALOG_RESULT_CANCEL, "escape cancels");
	xuiFileDialogDestroy(pDialog);
	pDialog = NULL;

	memset(&tDestroyResult, 0, sizeof(tDestroyResult));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = (const char*)sFixture;
	tDesc.sFilter = "Text Files (*.txt)|*.txt";
	tDesc.onResult = __xuiFileDialogDestroyResult;
	tDesc.pResultUser = &tDestroyResult;
	tDestroyResult.ppDialog = &pDialog;
	iRet = xuiOpenFileDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL, "callback destroy dialog open");
	iIndex = __xuiFileDialogFindEntry(pDialog, "alpha.txt");
	XUI_TEST_CHECK(iIndex >= 0 && xuiFileDialogSelectIndex(pDialog, iIndex) == XUI_OK,
		"callback destroy dialog select");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog == NULL && tDestroyResult.iCount == 1,
		"result callback may destroy dialog");

	memset(&tDestroyResult, 0, sizeof(tDestroyResult));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = (const char*)sFixture;
	tDestroyResult.ppDialog = &pDialog;
	iRet = xuiOpenFileDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL, "blur callback destroy dialog open");
	{
		xui_widget pPathInput = xuiFileDialogGetPathInputWidget(pDialog);
		xui_widget pNameInput = xuiFileDialogGetNameInputWidget(pDialog);
		xui_edit_behavior_t tBehavior;
		memset(&tBehavior, 0, sizeof(tBehavior));
		iRet = xuiEditGetBehavior(pPathInput, &tBehavior);
		if ( iRet == XUI_OK ) {
			tBehavior.bCommitOnBlur = 1;
			iRet = xuiEditSetBehavior(pPathInput, &tBehavior);
		}
		if ( iRet == XUI_OK ) iRet = xuiEditSetEvent(pPathInput,
			__xuiFileDialogDestroyEditEvent, &tDestroyResult);
		if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(
			xuiFileDialogGetPathBreadcrumbWidget(pDialog), 0);
		if ( iRet == XUI_OK ) iRet = xuiWidgetSetVisible(pPathInput, 1);
		if ( iRet == XUI_OK ) iRet = xuiSetFocusWidget(pContext, pPathInput);
		if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
		if ( iRet == XUI_OK && xuiGetFocusWidget(pContext) != pPathInput ) {
			iRet = XUI_ERROR_INVALID_STATE;
		}
		if ( iRet == XUI_OK ) iRet = xuiSetFocusWidget(pContext, pNameInput);
		if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	}
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog == NULL && tDestroyResult.iCount == 1,
		"path blur commit callback may destroy dialog without stale access");

cleanup:
	if ( pDialog != NULL ) xuiFileDialogDestroy(pDialog);
	if ( sFixture != NULL ) {
		xrtDirRemoveAll(sFixture);
	}
	if ( sFixture != NULL ) xrtFree(sFixture);
	if ( sSub != NULL ) xrtFree(sSub);
	if ( sSubTxt != NULL ) xrtFree(sSubTxt);
	if ( sSubSave != NULL ) xrtFree(sSubSave);
	if ( sTxt != NULL ) xrtFree(sTxt);
	if ( sBin != NULL ) xrtFree(sBin);
	if ( sLate != NULL ) xrtFree(sLate);
	if ( sSave != NULL ) xrtFree(sSave);
	if ( sMissingDir != NULL ) xrtFree(sMissingDir);
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_file_dialog_test passed\n");
	return 0;
}
