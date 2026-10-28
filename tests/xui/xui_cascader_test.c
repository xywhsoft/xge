#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_cascader_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_cascader_test_change_t {
	int iCount;
	int iLeaf;
	int iDepth;
	int arrValues[XUI_CASCADER_PATH_CAPACITY];
	int bOpenAtCallback;
	int bDestroyOnChange;
} xui_cascader_test_change_t;

typedef struct xui_cascader_test_event_destroy_t {
	xui_widget* ppWidget;
	int iEventType;
	int iCount;
} xui_cascader_test_event_destroy_t;

static void __xuiCascaderChanged(xui_widget pWidget, int iLeafIndex, const int* arrValues, int iDepth, void* pUser)
{
	xui_cascader_test_change_t* pChange;
	int i;

	(void)pWidget;
	pChange = (xui_cascader_test_change_t*)pUser;
	pChange->iCount++;
	pChange->iLeaf = iLeafIndex;
	pChange->iDepth = iDepth;
	pChange->bOpenAtCallback = xuiCascaderIsOpen(pWidget);
	for ( i = 0; i < XUI_CASCADER_PATH_CAPACITY; i++ ) {
		pChange->arrValues[i] = (arrValues != NULL && i < iDepth) ? arrValues[i] : -1;
	}
	if ( pChange->bDestroyOnChange ) xuiWidgetDestroy(pWidget);
}

static int __xuiCascaderDestroyVictimOnEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_cascader_test_event_destroy_t* pDestroy = (xui_cascader_test_event_destroy_t*)pUser;
	xui_widget pVictim;

	(void)pWidget;
	if ( (pDestroy == NULL) || (pEvent == NULL) || (pEvent->iType != pDestroy->iEventType) ||
	     (pDestroy->ppWidget == NULL) || (*pDestroy->ppWidget == NULL) ) return XUI_OK;
	pVictim = *pDestroy->ppWidget;
	*pDestroy->ppWidget = NULL;
	pDestroy->iCount++;
	xuiWidgetDestroy(pVictim);
	return XUI_OK;
}

static int __xuiCascaderRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 520, 360};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiCascaderDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiCascaderClickItem(xui_context pContext, xui_widget pCascader, int iColumn, int iItem)
{
	xui_widget pPanel;
	xui_rect_t tWorld;
	xui_rect_t tItem;
	float x;
	float y;
	int iRet;

	pPanel = xuiCascaderGetPanelWidget(pCascader);
	if ( pPanel == NULL ) return XUI_ERROR;
	tWorld = xuiWidgetGetWorldRect(pPanel);
	tItem = xuiCascaderGetItemRect(pCascader, iColumn, iItem);
	x = tWorld.fX + tItem.fX + tItem.fW * 0.5f;
	y = tWorld.fY + tItem.fY + tItem.fH * 0.5f;
	iRet = __xuiCascaderDispatchDown(pContext, x, y);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, x, y, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiCascaderSetItemsOomPreserves(xui_widget pCascader)
{
	xallocationbudget* pBudget;
	xallocationscope tScope;
	xui_cascader_item_t arrReplacement[2];
	int iRet;

	pBudget = NULL;
	memset(&tScope, 0, sizeof(tScope));
	arrReplacement[0] = (xui_cascader_item_t){"Replacement", 701, -1, XUI_CASCADER_ITEM_LEAF, NULL};
	arrReplacement[1] = (xui_cascader_item_t){"Other", 702, -1, XUI_CASCADER_ITEM_LEAF, NULL};
	if ( !xrtAllocationBudgetCreate(1u, &pBudget) ) return 0;
	if ( !xrtAllocationBudgetEnter(pBudget, &tScope) ) {
		xrtAllocationBudgetFree(pBudget);
		return 0;
	}
	iRet = xuiCascaderSetItems(pCascader, arrReplacement, 2);
	(void)xrtAllocationBudgetLeave(&tScope);
	xrtAllocationBudgetFree(pBudget);
	return iRet == XUI_ERROR_OUT_OF_MEMORY &&
		xuiCascaderGetItemCount(pCascader) == 11 &&
		xuiCascaderGetSelectedLeaf(pCascader) == 5 &&
		strcmp(xuiCascaderGetItem(pCascader, 0)->sText, "Guide") == 0;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_cascader_test_change_t tChange;
	xui_cascader_test_event_destroy_t tFocusDestroy;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pCascader;
	xui_widget pPanel;
	xui_widget pPopup;
	xui_widget pFocusDestroyCascader;
	xui_widget pFocusDestroyPanel;
	xui_surface pTarget;
	xui_font pFont;
	xui_cascader_desc_t tDesc;
	xui_cascader_desc_t tInvalidDesc;
	xui_cascader_item_t arrItems[11];
	xui_cascader_item_t arrInvalid[9];
	xui_accessible_node_t tAccessible;
	xui_accessible_value_t tAccessibleValue;
	int arrSelected[3];
	int arrPath[3];
	char sLongUtf8[132];
	char sOwnedText[32];
	char sSetText[32];
	xui_rect_t tButton;
	xui_rect_t tClear;
	xui_rect_t tPopupRect;
	xui_rect_t tPanelWorld;
	xui_rect_t tItemRect;
	uint32_t iState;
	int iChangeBefore;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pCascader = NULL;
	pPanel = NULL;
	pPopup = NULL;
	pFocusDestroyCascader = NULL;
	pFocusDestroyPanel = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tChange, 0, sizeof(tChange));
	memset(&tFocusDestroy, 0, sizeof(tFocusDestroy));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "cascader", 8, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 520.0f, 360.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 520.0f, 360.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");
	memset(&tInvalidDesc, 0, sizeof(tInvalidDesc));
	tInvalidDesc.iSize = sizeof(tInvalidDesc);
	tInvalidDesc.fItemHeight = NAN;
	pCascader = pRoot;
	iRet = xuiCascaderCreate(pContext, &pCascader, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pCascader == NULL, "reject NaN descriptor metric");
	tInvalidDesc.fItemHeight = 0.0f;
	tInvalidDesc.iSelectedDepth = 1;
	iRet = xuiCascaderCreate(pContext, &pCascader, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pCascader == NULL, "reject descriptor selection without values");

	memset(arrItems, 0, sizeof(arrItems));
	strcpy(sOwnedText, "Guide");
	arrItems[0] = (xui_cascader_item_t){sOwnedText, 1, -1, 0u, NULL};
	arrItems[1] = (xui_cascader_item_t){"Components", 2, -1, 0u, NULL};
	arrItems[2] = (xui_cascader_item_t){"Resources", 3, -1, 0u, NULL};
	arrItems[3] = (xui_cascader_item_t){"Design", 11, 0, XUI_CASCADER_ITEM_LEAF, NULL};
	arrItems[4] = (xui_cascader_item_t){"Navigation", 12, 0, 0u, NULL};
	arrItems[5] = (xui_cascader_item_t){"Side nav", 121, 4, XUI_CASCADER_ITEM_LEAF, NULL};
	arrItems[6] = (xui_cascader_item_t){"Top nav", 122, 4, XUI_CASCADER_ITEM_LEAF, NULL};
	arrItems[7] = (xui_cascader_item_t){"Disabled leaf", 123, 4, XUI_CASCADER_ITEM_DISABLED | XUI_CASCADER_ITEM_LEAF, NULL};
	arrItems[8] = (xui_cascader_item_t){"Forms", 21, 1, 0u, NULL};
	arrItems[9] = (xui_cascader_item_t){"Input", 211, 8, XUI_CASCADER_ITEM_LEAF, NULL};
	arrItems[10] = (xui_cascader_item_t){"DatePicker", 212, 8, XUI_CASCADER_ITEM_LEAF, NULL};
	arrSelected[0] = 1;
	arrSelected[1] = 12;
	arrSelected[2] = 121;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = arrItems;
	tDesc.iItemCount = 11;
	tDesc.arrSelectedValues = arrSelected;
	tDesc.iSelectedDepth = 3;
	tDesc.pFont = pFont;
	tDesc.bClearable = 1;
	tDesc.fColumnWidth = 128.0f;
	tDesc.fPopupMaxHeight = 160.0f;
	iRet = xuiCascaderCreate(pContext, &pCascader, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pCascader != NULL, "cascader create");
	strcpy(sOwnedText, "Changed after create");
	XUI_TEST_CHECK(strcmp(xuiCascaderGetItem(pCascader, 0)->sText, "Guide") == 0, "item text deep copy");
	iRet = xuiCascaderSetChange(pCascader, __xuiCascaderChanged, &tChange);
	XUI_TEST_CHECK(iRet == XUI_OK, "change callback");
	xuiWidgetSetRect(pCascader, (xui_rect_t){40.0f, 32.0f, 220.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pCascader);
	XUI_TEST_CHECK(iRet == XUI_OK, "add cascader");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 520, 360, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiCascaderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");

	XUI_TEST_CHECK(xuiCascaderGetItemCount(pCascader) == 11, "item count");
	XUI_TEST_CHECK(xuiCascaderGetSelectedLeaf(pCascader) == 5, "initial leaf");
	XUI_TEST_CHECK(xuiCascaderGetSelectedDepth(pCascader) == 3, "initial depth");
	XUI_TEST_CHECK(strcmp(xuiCascaderGetPlaceholder(pCascader), "Select") == 0, "default placeholder");
	memset(sLongUtf8, 'a', 126);
	sLongUtf8[126] = (char)0xe4;
	sLongUtf8[127] = (char)0xb8;
	sLongUtf8[128] = (char)0xad;
	sLongUtf8[129] = '\0';
	XUI_TEST_CHECK(xuiCascaderSetPlaceholder(pCascader, sLongUtf8) == XUI_OK, "set long utf8 placeholder");
	XUI_TEST_CHECK(strlen(xuiCascaderGetPlaceholder(pCascader)) == 126u, "placeholder truncation keeps whole utf8 codepoints");
	XUI_TEST_CHECK(xuiCascaderSetPlaceholder(pCascader, "Select") == XUI_OK, "restore placeholder");
	XUI_TEST_CHECK(strcmp(xuiCascaderGetSelectedText(pCascader), "Guide / Navigation / Side nav") == 0, "full display");
	XUI_TEST_CHECK(xuiCascaderGetSelectedPath(pCascader, arrPath, 3) == 3 && arrPath[0] == 1 && arrPath[1] == 12 && arrPath[2] == 121, "selected values");
	arrPath[0] = -1;
	XUI_TEST_CHECK(xuiCascaderGetSelectedPath(pCascader, NULL, 0) == 3 &&
	               xuiCascaderGetSelectedPath(pCascader, arrPath, 1) == 3 && arrPath[0] == 1,
	               "selected path supports size query and partial output");
	XUI_TEST_CHECK(xuiCascaderGetSelectedPath(pCascader, arrPath, -1) == XUI_ERROR_INVALID_ARGUMENT,
	               "selected path rejects negative capacity");
	memset(arrInvalid, 0, sizeof(arrInvalid));
	arrInvalid[0] = (xui_cascader_item_t){"Self", 1, 0, 0u, NULL};
	XUI_TEST_CHECK(xuiCascaderSetItems(pCascader, arrInvalid, 1) == XUI_ERROR_INVALID_ARGUMENT, "reject self parent");
	arrInvalid[0] = (xui_cascader_item_t){"Cycle A", 1, 1, 0u, NULL};
	arrInvalid[1] = (xui_cascader_item_t){"Cycle B", 2, 0, 0u, NULL};
	XUI_TEST_CHECK(xuiCascaderSetItems(pCascader, arrInvalid, 2) == XUI_ERROR_INVALID_ARGUMENT, "reject parent cycle");
	arrInvalid[0] = (xui_cascader_item_t){"Leaf parent", 1, -1, XUI_CASCADER_ITEM_LEAF, NULL};
	arrInvalid[1] = (xui_cascader_item_t){"Child", 2, 0, 0u, NULL};
	XUI_TEST_CHECK(xuiCascaderSetItems(pCascader, arrInvalid, 2) == XUI_ERROR_INVALID_ARGUMENT, "reject child of leaf");
	arrInvalid[0] = (xui_cascader_item_t){"Duplicate A", 1, -1, 0u, NULL};
	arrInvalid[1] = (xui_cascader_item_t){"Duplicate B", 1, -1, 0u, NULL};
	XUI_TEST_CHECK(xuiCascaderSetItems(pCascader, arrInvalid, 2) == XUI_ERROR_INVALID_ARGUMENT, "reject duplicate sibling value");
	for ( iRet = 0; iRet < 9; iRet++ ) {
		arrInvalid[iRet] = (xui_cascader_item_t){"Deep", iRet, iRet - 1, 0u, NULL};
	}
	XUI_TEST_CHECK(xuiCascaderSetItems(pCascader, arrInvalid, 9) == XUI_ERROR_INVALID_ARGUMENT, "reject excessive hierarchy depth");
	XUI_TEST_CHECK(xuiCascaderSetItems(pCascader, arrInvalid, XUI_CASCADER_ITEM_CAPACITY + 1) == XUI_ERROR_INVALID_ARGUMENT, "reject excessive item count");
	XUI_TEST_CHECK(xuiCascaderGetItemCount(pCascader) == 11 && xuiCascaderGetSelectedLeaf(pCascader) == 5, "invalid items preserve data");
	strcpy(sSetText, "Temporary");
	arrInvalid[0] = (xui_cascader_item_t){sSetText, 900, -1, XUI_CASCADER_ITEM_LEAF, NULL};
	XUI_TEST_CHECK(xuiCascaderSetItems(pCascader, arrInvalid, 1) == XUI_OK, "replace items");
	strcpy(sSetText, "Mutated");
	XUI_TEST_CHECK(strcmp(xuiCascaderGetItem(pCascader, 0)->sText, "Temporary") == 0,
	               "set items deep copies text");
	strcpy(sOwnedText, "Guide");
	XUI_TEST_CHECK(xuiCascaderSetItems(pCascader, arrItems, 11) == XUI_OK &&
	               xuiCascaderSetSelectedPath(pCascader, arrSelected, 3) == XUI_OK,
	               "restore items and selection");
	XUI_TEST_CHECK(__xuiCascaderSetItemsOomPreserves(pCascader),
	               "item replacement allocation failure preserves data and selection");
	XUI_TEST_CHECK(xuiCascaderSetPopupSize(pCascader, NAN, 0.0f, 160.0f) == XUI_ERROR_INVALID_ARGUMENT, "reject NaN popup size");
	XUI_TEST_CHECK(xuiCascaderSetMetrics(pCascader, 32.0f, INFINITY) == XUI_ERROR_INVALID_ARGUMENT, "reject infinite metric");
	iRet = xuiCascaderSetShowAllLevels(pCascader, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCascaderGetSelectedText(pCascader), "Side nav") == 0, "leaf display");
	iRet = xuiCascaderSetShowAllLevels(pCascader, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore full display");
	tButton = xuiCascaderGetButtonRect(pCascader);
	tClear = xuiCascaderGetClearRect(pCascader);
	XUI_TEST_CHECK(tButton.fW > 20.0f && tButton.fH > 20.0f && tClear.fW > 12.0f, "owner rects");

	iRet = xuiSetFocusWidget(pContext, pCascader);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus owner before popup shortcuts");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_F4, 0) == XUI_OK &&
	               xuiDispatchPendingEvents(pContext) == XUI_OK && xuiCascaderIsOpen(pCascader),
	               "F4 opens popup");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_F4, XUI_MOD_ALT) == XUI_OK &&
	               xuiDispatchPendingEvents(pContext) == XUI_OK && xuiCascaderIsOpen(pCascader),
	               "Alt+F4 is not consumed as a popup shortcut");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_F4, 0) == XUI_OK &&
	               xuiDispatchPendingEvents(pContext) == XUI_OK && !xuiCascaderIsOpen(pCascader),
	               "F4 closes popup");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_DOWN, XUI_MOD_ALT) == XUI_OK &&
	               xuiDispatchPendingEvents(pContext) == XUI_OK && xuiCascaderIsOpen(pCascader),
	               "Alt+Down opens popup");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_UP, XUI_MOD_ALT) == XUI_OK &&
	               xuiDispatchPendingEvents(pContext) == XUI_OK && !xuiCascaderIsOpen(pCascader),
	               "Alt+Up closes popup");

	iRet = xuiCascaderOpen(pCascader);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCascaderIsOpen(pCascader), "open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "open layout");
	iRet = __xuiCascaderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "open render");
	pPanel = xuiCascaderGetPanelWidget(pCascader);
	pPopup = xuiCascaderGetPopupWidget(pCascader);
	XUI_TEST_CHECK(pPanel != NULL && pPopup != NULL && xuiGetFocusWidget(pContext) == pPanel, "panel popup focus");
	XUI_TEST_CHECK(xuiCascaderGetColumnCount(pCascader) == 3, "column count");
	iState = xuiCascaderGetState(pCascader);
	XUI_TEST_CHECK((iState & XUI_CASCADER_STATE_OPEN) != 0u, "open state");
	tPopupRect = xuiPopupGetPopupRect(pPopup);
	XUI_TEST_CHECK(tPopupRect.fY >= 60.0f && tPopupRect.fW >= 380.0f, "popup rect");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_TAB, 0) == XUI_OK &&
	               xuiDispatchPendingEvents(pContext) == XUI_OK && !xuiCascaderIsOpen(pCascader),
	               "Tab closes popup without trapping focus");
	XUI_TEST_CHECK(xuiCascaderOpen(pCascader) == XUI_OK && xuiLayout(pContext) == XUI_OK &&
	               __xuiCascaderRender(pContext, pTarget) == XUI_OK,
	               "reopen after Tab");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_END, 0) == XUI_OK &&
	               xuiDispatchPendingEvents(pContext) == XUI_OK &&
	               xuiInputKeyDown(pContext, XUI_KEY_HOME, 0) == XUI_OK &&
	               xuiDispatchPendingEvents(pContext) == XUI_OK &&
	               xuiInputKeyDown(pContext, XUI_KEY_PAGE_DOWN, 0) == XUI_OK &&
	               xuiDispatchPendingEvents(pContext) == XUI_OK &&
	               xuiInputKeyDown(pContext, XUI_KEY_PAGE_UP, 0) == XUI_OK &&
	               xuiDispatchPendingEvents(pContext) == XUI_OK &&
	               xuiCascaderIsOpen(pCascader) && xuiCascaderGetSelectedLeaf(pCascader) == 5,
	               "Home End and page navigation do not commit selection");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK, "keyboard next item");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK, "keyboard skips disabled item");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ENTER, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK, "keyboard commit after disabled item");
	XUI_TEST_CHECK(tChange.iCount == 0, "reselecting the current item does not emit a change");
	memset(&tChange, 0, sizeof(tChange));
	XUI_TEST_CHECK(xuiCascaderOpen(pCascader) == XUI_OK && xuiLayout(pContext) == XUI_OK, "reopen after keyboard navigation");
	XUI_TEST_CHECK(__xuiCascaderRender(pContext, pTarget) == XUI_OK, "render reopened cascader");
	pPanel = xuiCascaderGetPanelWidget(pCascader);
	tPanelWorld = xuiWidgetGetWorldRect(pPanel);
	tItemRect = xuiCascaderGetItemRect(pCascader, 2, 6);
	iRet = __xuiCascaderDispatchDown(pContext,
		tPanelWorld.fX + tItemRect.fX + tItemRect.fW * 0.5f,
		tPanelWorld.fY + tItemRect.fY + tItemRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pPanel &&
	               xuiCascaderIsOpen(pCascader) && tChange.iCount == 0,
	               "item press captures without committing");
	XUI_TEST_CHECK(xuiInputPointerMove(pContext,
	               (int)(tPanelWorld.fX + tItemRect.fX + tItemRect.fW * 0.5f),
	               (int)(tPanelWorld.fY + tPanelWorld.fH + 20.0f), XUI_POINTER_BUTTON_LEFT) == XUI_OK &&
	               xuiDispatchPendingEvents(pContext) == XUI_OK,
	               "drag pressed item outside panel");
	XUI_TEST_CHECK(xuiInputPointerUp(pContext,
	               (int)(tPanelWorld.fX + tItemRect.fX + tItemRect.fW * 0.5f),
	               (int)(tPanelWorld.fY + tPanelWorld.fH + 20.0f), XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK &&
	               xuiDispatchPendingEvents(pContext) == XUI_OK &&
	               xuiGetPointerCapture(pContext) == NULL && xuiCascaderIsOpen(pCascader) &&
	               tChange.iCount == 0 && xuiCascaderGetSelectedLeaf(pCascader) == 5,
	               "release outside cancels item activation and capture");

	iRet = __xuiCascaderClickItem(pContext, pCascader, 2, 7);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCascaderIsOpen(pCascader), "disabled click keeps open");
	XUI_TEST_CHECK(tChange.iCount == 0 && xuiCascaderGetSelectedLeaf(pCascader) == 5, "disabled no change");
	iRet = __xuiCascaderClickItem(pContext, pCascader, 2, 6);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiCascaderIsOpen(pCascader), "leaf click closes");
	XUI_TEST_CHECK(tChange.iCount == 1 && tChange.iLeaf == 6 && tChange.iDepth == 3 && !tChange.bOpenAtCallback, "leaf callback after close");
	XUI_TEST_CHECK(tChange.arrValues[0] == 1 && tChange.arrValues[1] == 12 && tChange.arrValues[2] == 122, "callback values");
	XUI_TEST_CHECK(xuiCascaderGetSelectedLeaf(pCascader) == 6 && strcmp(xuiCascaderGetSelectedText(pCascader), "Guide / Navigation / Top nav") == 0, "selected top nav");

	iRet = xuiSetFocusWidget(pContext, pCascader);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus owner");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DELETE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "delete input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tChange.iCount == 2 && xuiCascaderGetSelectedDepth(pCascader) == 0, "keyboard clear");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCascaderIsOpen(pCascader), "key down opens");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key right input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCascaderGetColumnCount(pCascader) >= 2, "key right opens child column");

	iRet = xuiWidgetSetEnabled(pCascader, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable cascader");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable dispatch");
	iRet = xuiCascaderOpen(pCascader);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiCascaderIsOpen(pCascader), "disabled does not open");
	iState = xuiCascaderGetState(pCascader);
	XUI_TEST_CHECK((iState & XUI_WIDGET_STATE_DISABLED) != 0u, "disabled state");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pCascader, 1) == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK, "reenable cascader");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pCascader, 0, &tAccessible) == XUI_OK &&
	               tAccessible.iRole == XUI_ACCESSIBLE_ROLE_COMBOBOX &&
	               (tAccessible.iState & XUI_ACCESSIBLE_STATE_COLLAPSED) != 0u,
	               "combobox accessibility node");
	memset(&tAccessibleValue, 0, sizeof(tAccessibleValue));
	tAccessibleValue.iSize = sizeof(tAccessibleValue);
	tAccessibleValue.iType = XUI_ACCESSIBLE_VALUE_NUMBER;
	tAccessibleValue.fNumber = 6.0;
	iChangeBefore = tChange.iCount;
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pCascader, tAccessible.iId,
	               XUI_ACCESSIBLE_ACTION_SET_VALUE, &tAccessibleValue) == XUI_OK &&
	               xuiCascaderGetSelectedLeaf(pCascader) == 6 && tChange.iCount == iChangeBefore + 1,
	               "accessibility selects a leaf");
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pCascader, tAccessible.iId,
	               XUI_ACCESSIBLE_ACTION_SET_VALUE, &tAccessibleValue) == XUI_OK &&
	               tChange.iCount == iChangeBefore + 1,
	               "accessibility reselect is idempotent");
	XUI_TEST_CHECK(xuiCascaderSetSelectedPath(pCascader, arrSelected, 3) == XUI_OK, "restore before destroying callback");
	XUI_TEST_CHECK(xuiCascaderOpen(pCascader) == XUI_OK && xuiLayout(pContext) == XUI_OK, "open before destroying callback");
	XUI_TEST_CHECK(__xuiCascaderRender(pContext, pTarget) == XUI_OK, "render before destroying callback");
	iChangeBefore = tChange.iCount;
	tChange.bDestroyOnChange = 1;
	iRet = __xuiCascaderClickItem(pContext, pCascader, 2, 6);
	XUI_TEST_CHECK(iRet == XUI_OK && tChange.iCount == iChangeBefore + 1 && !tChange.bOpenAtCallback, "destroying callback completes safely");
	pCascader = NULL;

	iRet = xuiCascaderCreate(pContext, &pFocusDestroyCascader, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pFocusDestroyCascader != NULL, "focus destroy cascader create");
	XUI_TEST_CHECK(xuiWidgetSetRect(pFocusDestroyCascader, (xui_rect_t){280.0f, 32.0f, 200.0f, 30.0f}) == XUI_OK &&
	               xuiWidgetAddChild(pRoot, pFocusDestroyCascader) == XUI_OK,
	               "focus destroy cascader attach");
	pFocusDestroyPanel = xuiCascaderGetPanelWidget(pFocusDestroyCascader);
	tFocusDestroy.ppWidget = &pFocusDestroyCascader;
	tFocusDestroy.iEventType = XUI_EVENT_FOCUS;
	XUI_TEST_CHECK(pFocusDestroyPanel != NULL &&
	               xuiWidgetSetEventHandler(pFocusDestroyPanel, XUI_EVENT_FOCUS,
	                   __xuiCascaderDestroyVictimOnEvent, &tFocusDestroy) == XUI_OK,
	               "focus destroy handler");
	XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK, "focus destroy cascader layout");
	tPopupRect = xuiWidgetGetWorldRect(pFocusDestroyCascader);
	iRet = __xuiCascaderDispatchDown(pContext, tPopupRect.fX + 12.0f,
	                                tPopupRect.fY + tPopupRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tFocusDestroy.iCount == 1 && pFocusDestroyCascader == NULL,
	               "focus callback may destroy cascader safely");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_cascader_test passed\n");
	return 0;
}
