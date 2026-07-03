#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

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

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext = NULL;
	xui_font pFont = NULL;
	xui_widget pRoot = NULL;
	xui_widget pBreadcrumb = NULL;
	xui_file_dialog pDialog = NULL;
	xui_surface pTarget = NULL;
	xui_file_dialog_desc_t tDesc;
	xui_file_dialog_test_result_t tResult;
	xui_rect_i_t tFullRect;
	xui_rect_t tRect;
	xui_rect_t tWorld;
	str sFixture = NULL;
	str sSub = NULL;
	str sTxt = NULL;
	str sBin = NULL;
	str sLate = NULL;
	str sSave = NULL;
	str sMissingDir = NULL;
	int iFailed = 0;
	int iRet;
	int iIndex;

	xuiTestProxyInit(&tState);
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

	sFixture = xrtPathJoin(2, "build", "file_dialog_fixture");
	sSub = xrtPathJoin(2, sFixture, "sub");
	sTxt = xrtPathJoin(2, sFixture, "alpha.txt");
	sBin = xrtPathJoin(2, sFixture, "beta.bin");
	sLate = xrtPathJoin(2, sFixture, "late.txt");
	sSave = xrtPathJoin(2, sFixture, "new.txt");
	sMissingDir = xrtPathJoin(2, sFixture, "missing_dir");
	XUI_TEST_CHECK(sFixture != NULL && sSub != NULL && sTxt != NULL && sBin != NULL && sLate != NULL && sSave != NULL && sMissingDir != NULL, "fixture paths");
	(void)xrtDirDelete(sFixture);
	XUI_TEST_CHECK(xrtDirCreateAll(sSub) == TRUE, "fixture dirs");
	XUI_TEST_CHECK(xrtFilePutAll(sTxt, "alpha", 5) == 5, "fixture txt");
	XUI_TEST_CHECK(xrtFilePutAll(sBin, "beta", 4) == 4, "fixture bin");
	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 800, 520, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	tFullRect = (xui_rect_i_t){0, 0, 800, 520};

	memset(&tResult, 0, sizeof(tResult));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sInitialDir = (const char*)sFixture;
	tDesc.sFilter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*";
	tDesc.onResult = __xuiFileDialogTestResult;
	tDesc.pResultUser = &tResult;
	iRet = xuiOpenFileDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL && xuiFileDialogIsOpen(pDialog), "open dialog");
	XUI_TEST_CHECK(xuiFileDialogGetWindowWidget(pDialog) != NULL, "window getter");
	XUI_TEST_CHECK(xuiFileDialogGetRootListWidget(pDialog) != NULL, "root list getter");
	XUI_TEST_CHECK(xuiFileDialogGetFileListWidget(pDialog) != NULL, "file list getter");
	XUI_TEST_CHECK(xuiFileDialogGetPathBreadcrumbWidget(pDialog) != NULL, "path breadcrumb getter");
	XUI_TEST_CHECK(xuiFileDialogGetFilterComboWidget(pDialog) != NULL, "filter combo getter");
	XUI_TEST_CHECK(xuiFileDialogGetOkButtonWidget(pDialog) != NULL, "ok button getter");
	XUI_TEST_CHECK(xuiFileDialogGetCancelButtonWidget(pDialog) != NULL, "cancel button getter");
	XUI_TEST_CHECK(xuiFileDialogGetFilterCount(pDialog) == 2, "filter count");
	XUI_TEST_CHECK(strcmp(xuiFileDialogGetFilterName(pDialog, 0), "Text Files (*.txt)") == 0, "filter name");
	XUI_TEST_CHECK(strcmp(xuiFileDialogGetFilterPattern(pDialog, 1), "*.*") == 0, "filter pattern");
	XUI_TEST_CHECK(strcmp(xuiFileDialogGetFilter(pDialog), "*.txt") == 0, "active filter");
	XUI_TEST_CHECK(xuiWidgetGetVisible(xuiFileDialogGetPathBreadcrumbWidget(pDialog)) && !xuiWidgetGetVisible(xuiFileDialogGetPathInputWidget(pDialog)), "path starts as breadcrumb");
	XUI_TEST_CHECK(xuiFileDialogGetUpButtonWidget(pDialog) != NULL, "up button getter");
	XUI_TEST_CHECK(xuiFileDialogGetRefreshButtonWidget(pDialog) != NULL, "refresh button getter");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dialog layout");
	tRect = xuiWidgetGetWorldRect(xuiFileDialogGetNameInputWidget(pDialog));
	tWorld = xuiWidgetGetWorldRect(xuiFileDialogGetFilterComboWidget(pDialog));
	XUI_TEST_CHECK(tRect.fY == tWorld.fY && tRect.fH == tWorld.fH, "name and filter share row");
	XUI_TEST_CHECK(xuiWidgetGetWorldRect(xuiFileDialogGetOkButtonWidget(pDialog)).fY > tRect.fY, "buttons below fields");
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
	XUI_TEST_CHECK(tRect.fY == tWorld.fY && tRect.fW >= 120.0f && tWorld.fW >= 176.0f, "resized name filter row");
	XUI_TEST_CHECK(xuiWidgetGetWorldRect(xuiFileDialogGetOkButtonWidget(pDialog)).fY > tRect.fY, "resized buttons below fields");
	XUI_TEST_CHECK(xuiWidgetGetWorldRect(xuiFileDialogGetFileListWidget(pDialog)).fH >= 72.0f, "resized file list height");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "dialog render");
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
	iRet = __xuiFileDialogDoubleClickEntry(pContext, pDialog, iIndex);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sSub) == 0, "double click enters directory");
	iRet = __xuiFileDialogClickWidget(pContext, xuiFileDialogGetUpButtonWidget(pDialog));
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiFileDialogGetDirectory(pDialog), (const char*)sFixture) == 0, "up button enters parent directory");
	XUI_TEST_CHECK(xrtFilePutAll(sLate, "late", 4) == 4, "late txt");
	XUI_TEST_CHECK(__xuiFileDialogFindEntry(pDialog, "late.txt") < 0, "late file absent before refresh");
	iRet = __xuiFileDialogClickWidget(pContext, xuiFileDialogGetRefreshButtonWidget(pDialog));
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiFileDialogFindEntry(pDialog, "late.txt") >= 0, "refresh button reloads entries");
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
	tDesc.sFilter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*";
	tDesc.sFileName = "alpha";
	tDesc.onResult = __xuiFileDialogTestResult;
	tDesc.pResultUser = &tResult;
	iRet = xuiSaveFileDialog(pContext, &pDialog, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDialog != NULL, "save overwrite dialog");
	iRet = xuiFileDialogCommit(pDialog);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFileDialogIsOpen(pDialog) && xuiMsgBoxIsOpen(xuiFileDialogGetOverwriteMsgBox(pDialog)), "save overwrite asks");
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

cleanup:
	if ( pDialog != NULL ) xuiFileDialogDestroy(pDialog);
	if ( sFixture != NULL ) {
		xrtDirDelete(sFixture);
	}
	if ( sFixture != NULL ) xrtFree(sFixture);
	if ( sSub != NULL ) xrtFree(sSub);
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
