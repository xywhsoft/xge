#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_list_view_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiListViewNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) {
		fD = -fD;
	}
	return fD < 0.01f;
}

static void __xuiListViewSelected(xui_widget pWidget, int iIndex, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)iIndex;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

typedef struct xui_list_view_event_state_t {
	int iDoubleClickCount;
	int iContextMenuCount;
	int iContextIndex;
	int iActivateCount;
	int iActivateIndex;
	int iActivateButton;
	float fContextX;
	float fContextY;
} xui_list_view_event_state_t;

static int __xuiListViewEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_list_view_event_state_t* pState = (xui_list_view_event_state_t*)pUser;
	(void)pWidget;
	if ( (pState == NULL) || (pEvent == NULL) ) return XUI_OK;
	if ( pEvent->iType == XUI_EVENT_POINTER_DOUBLE_CLICK ) pState->iDoubleClickCount++;
	return XUI_OK;
}

static int __xuiListViewContext(xui_widget pWidget, int iIndex, float fX, float fY, void* pUser)
{
	xui_list_view_event_state_t* pState = (xui_list_view_event_state_t*)pUser;
	(void)pWidget;
	if ( pState == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pState->iContextMenuCount++;
	pState->iContextIndex = iIndex;
	pState->fContextX = fX;
	pState->fContextY = fY;
	return XUI_EVENT_DISPATCH_STOP;
}

static void __xuiListViewActivate(xui_widget pWidget, int iIndex, int iButton, void* pUser)
{
	xui_list_view_event_state_t* pState = (xui_list_view_event_state_t*)pUser;
	(void)pWidget;
	if ( pState == NULL ) return;
	pState->iActivateCount++;
	pState->iActivateIndex = iIndex;
	pState->iActivateButton = iButton;
}

static int __xuiListViewRenderItem(xui_widget pWidget, int iIndex, xui_draw_context pDraw, xui_rect_t tRect, int iState, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)iIndex;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
	return 0;
}

typedef struct xui_list_view_destroy_state_t {
	xui_widget* ppWidget;
	int iCount;
} xui_list_view_destroy_state_t;

static void __xuiListViewDestroyOnSelect(xui_widget pWidget, int iIndex, void* pUser)
{
	xui_list_view_destroy_state_t* pState = (xui_list_view_destroy_state_t*)pUser;
	(void)iIndex;
	if ( pState != NULL ) {
		pState->iCount++;
		xuiWidgetDestroy(pWidget);
		if ( pState->ppWidget != NULL ) *pState->ppWidget = NULL;
	}
}

static int __xuiListViewDestroyOnRender(xui_widget pWidget, int iIndex, xui_draw_context pDraw,
	xui_rect_t tRect, int iState, void* pUser)
{
	xui_list_view_destroy_state_t* pState = (xui_list_view_destroy_state_t*)pUser;
	(void)iIndex;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	if ( pState != NULL ) {
		pState->iCount++;
		xuiWidgetDestroy(pWidget);
		if ( pState->ppWidget != NULL ) *pState->ppWidget = NULL;
	}
	return 1;
}

static const char* __xuiListViewDestroyOnText(xui_widget pWidget, int iIndex, void* pUser)
{
	xui_list_view_destroy_state_t* pState = (xui_list_view_destroy_state_t*)pUser;
	(void)iIndex;
	if ( pState != NULL ) {
		pState->iCount++;
		xuiWidgetDestroy(pWidget);
		if ( pState->ppWidget != NULL ) *pState->ppWidget = NULL;
	}
	return "destroyed";
}

static void __xuiListViewDestroyOnActivate(xui_widget pWidget, int iIndex, int iButton, void* pUser)
{
	xui_list_view_destroy_state_t* pState = (xui_list_view_destroy_state_t*)pUser;
	(void)iIndex;
	(void)iButton;
	if ( pState != NULL ) {
		pState->iCount++;
		xuiWidgetDestroy(pWidget);
		if ( pState->ppWidget != NULL ) *pState->ppWidget = NULL;
	}
}

static const char* __xuiListViewVirtualText(xui_widget pWidget, int iIndex, void* pUser)
{
	static char sText[64];
	(void)pWidget;
	(void)pUser;
	snprintf(sText, sizeof(sText), "Virtual item %d", iIndex);
	return sText;
}

static int __xuiListViewRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 420, 280};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiListViewClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

int main(void)
{
	static const char* arrItems[] = {
		"Scene", "Map", "Disabled layer", "Player", "Camera", "Light",
		"Audio", "Script", "Trigger", "Inventory", "Quest", "Save point",
		"Fog", "Weather", "Dialogue", "Debug"
	};
	int arrEnabled[16];
	int arrSelected[16];
	int arrTooSmall[15];
	int arrOversized[20];
	const char* arrExpanded[17];
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pList;
	xui_widget pInvalid;
	xui_widget pDestroyList;
	xui_widget pRenderDestroyList;
	xui_widget pTextDestroyList;
	xui_widget pActivateDestroyList;
	xui_widget pVirtualList;
	xui_surface pTarget;
	xui_surface pCache;
	xui_font pFont;
	xui_list_view_desc_t tDesc;
	xui_list_view_desc_t tInvalidDesc;
	xui_event_t tContextEvent;
	xui_event_t tPointerEvent;
	xui_rect_t tItem;
	xui_rect_t tItemNext;
	xui_rect_t tListWorld;
	xui_vec2_t tMeasured;
	xui_accessible_node_t tAccessible;
	uint32_t iBorderColor;
	uint32_t iColor;
	float fScroll;
	float fContentHeight;
	float fOldItemHeight;
	float fItemHeight;
	float fPadding;
	float fBorderWidth;
	int iSelectCount;
	int iOldSelectCount;
	int iRenderCount;
	int iOldChangeCount;
	int i;
	xui_list_view_event_state_t tEventState;
	xui_list_view_destroy_state_t tSelectDestroy;
	xui_list_view_destroy_state_t tRenderDestroy;
	xui_list_view_destroy_state_t tTextDestroy;
	xui_list_view_destroy_state_t tActivateDestroy;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pList = NULL;
	pInvalid = NULL;
	pDestroyList = NULL;
	pRenderDestroyList = NULL;
	pTextDestroyList = NULL;
	pActivateDestroyList = NULL;
	pVirtualList = NULL;
	pTarget = NULL;
	pCache = NULL;
	pFont = NULL;
	iBorderColor = XUI_COLOR_RGBA(96, 148, 204, 255);
	iSelectCount = 0;
	iRenderCount = 0;
	memset(&tEventState, 0, sizeof(tEventState));
	memset(&tSelectDestroy, 0, sizeof(tSelectDestroy));
	memset(&tRenderDestroy, 0, sizeof(tRenderDestroy));
	memset(&tTextDestroy, 0, sizeof(tTextDestroy));
	memset(&tActivateDestroy, 0, sizeof(tActivateDestroy));
	memset(&tAccessible, 0, sizeof(tAccessible));
	iFailed = 0;
	memset(arrEnabled, 1, sizeof(arrEnabled));
	memset(arrSelected, 0, sizeof(arrSelected));
	memset(arrTooSmall, 0, sizeof(arrTooSmall));
	memset(arrOversized, 0, sizeof(arrOversized));
	for ( i = 16; i < 20; i++ ) arrOversized[i] = 0x5100 + i;
	for ( i = 0; i < 17; i++ ) arrExpanded[i] = "Expanded";
	arrEnabled[2] = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "list", 4, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 420.0f, 280.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 420.0f, 280.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = arrItems;
	tDesc.arrEnabled = arrEnabled;
	tDesc.iItemCount = 16;
	tDesc.iSelected = 1;
	tDesc.fItemHeight = 22.0f;
	iRet = xuiListViewCreate(pContext, &pList, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pList != NULL, "listview create");
	tInvalidDesc = tDesc;
	tInvalidDesc.fItemHeight = INFINITY;
	pInvalid = (xui_widget)(void*)1;
	iRet = xuiListViewCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"invalid descriptor clears listview output");
	tInvalidDesc = tDesc;
	tInvalidDesc.arrSelected = arrTooSmall;
	tInvalidDesc.iSelectionCount = 15;
	pInvalid = (xui_widget)(void*)1;
	iRet = xuiListViewCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"short external selection buffer is rejected at creation");
	iRet = xuiWidgetMeasure(pList, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiListViewNear(tMeasured.fX, 180.0f) && __xuiListViewNear(tMeasured.fY, 134.0f), "list content extent is contained by six-row viewport");
	iRet = xuiWidgetAddChild(pRoot, pList);
	XUI_TEST_CHECK(iRet == XUI_OK, "add listview");
	xuiWidgetSetRect(pList, (xui_rect_t){24.0f, 20.0f, 190.0f, 120.0f});
	iRet = xuiListViewSetSelect(pList, __xuiListViewSelected, &iSelectCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");
	iRet = xuiListViewSetActivate(pList, __xuiListViewActivate, &tEventState);
	XUI_TEST_CHECK(iRet == XUI_OK, "activate callback");
	iRet = xuiWidgetSetEventCallback(pList, __xuiListViewEvent, &tEventState);
	XUI_TEST_CHECK(iRet == XUI_OK, "event callback");
	iRet = xuiListViewSetContextMenu(pList, __xuiListViewContext, &tEventState);
	XUI_TEST_CHECK(iRet == XUI_OK, "context callback");
	iRet = xuiListViewSetItemRenderer(pList, __xuiListViewRenderItem, &iRenderCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "renderer callback");
	iRet = xuiListViewSetColors(pList,
		XUI_COLOR_RGBA(247, 251, 255, 255),
		iBorderColor,
		XUI_COLOR_RGBA(0, 0, 0, 0),
		XUI_COLOR_RGBA(255, 255, 255, 0),
		XUI_COLOR_RGBA(228, 240, 252, 255),
		XUI_COLOR_RGBA(47, 128, 237, 255),
		XUI_COLOR_RGBA(31, 49, 68, 255),
		XUI_COLOR_RGBA(150, 162, 176, 180));
	XUI_TEST_CHECK(iRet == XUI_OK, "transparent focus color setup");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 420, 280, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiListViewRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && iRenderCount > 0, "render rows");

	XUI_TEST_CHECK(xuiListViewGetItemCount(pList) == 16, "item count");
	XUI_TEST_CHECK(xuiListViewGetType(pContext) != NULL, "listview type");
	XUI_TEST_CHECK(strcmp(xuiListViewGetItemText(pList, 0), "Scene") == 0, "item text");
	XUI_TEST_CHECK(xuiListViewGetSelected(pList) == 1, "initial selected");
	XUI_TEST_CHECK(!xuiListViewIsItemEnabled(pList, 2), "disabled item");
	XUI_TEST_CHECK(xuiListViewGetFrameWidget(pList) != NULL, "frame exists");
	XUI_TEST_CHECK(xuiListViewGetViewportWidget(pList) != NULL, "viewport exists");
	XUI_TEST_CHECK(xuiScrollFrameIsVScrollBarVisible(xuiListViewGetFrameWidget(pList)), "vertical bar visible");
	XUI_TEST_CHECK(!xuiScrollFrameIsHScrollBarVisible(xuiListViewGetFrameWidget(pList)), "horizontal bar hidden");
	iRet = xuiListViewSetFont(pList, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetFont(pList) == pFont, "font property");
	iRet = xuiListViewGetMetrics(pList, &fItemHeight, &fPadding, &fBorderWidth);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiListViewNear(fItemHeight, 22.0f) &&
		__xuiListViewNear(fPadding, 8.0f) && __xuiListViewNear(fBorderWidth, 1.0f), "metrics property");
	iRet = xuiListViewSetNotifyRepeatSelect(pList, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetNotifyRepeatSelect(pList), "repeat select property");
	iRet = xuiListViewSetNotifyRepeatSelect(pList, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiListViewGetNotifyRepeatSelect(pList), "repeat select restore");
	iRet = xuiListViewSetScrollbarMode(pList, XUI_SCROLLBAR_MODE_FULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetScrollbarMode(pList) == XUI_SCROLLBAR_MODE_FULL,
		"full scrollbar mode");
	iRet = xuiListViewSetScrollbarMode(pList, XUI_SCROLLBAR_MODE_COMPACT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetScrollbarMode(pList) == XUI_SCROLLBAR_MODE_COMPACT,
		"compact scrollbar mode");
	iRet = xuiListViewGetColors(pList, NULL, &iColor, NULL, NULL, NULL, NULL, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && iColor == iBorderColor, "list colors getter");
	iRet = xuiListViewGetScrollbarColors(pList, NULL, &iColor, NULL, NULL, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && iColor != 0u, "scrollbar colors getter");
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pList) == 17, "accessible node count");
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pList, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_LIST &&
		tAccessible.iRowCount == 16 && tAccessible.iColumnCount == 1 &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_MULTISELECTABLE) == 0u,
		"accessible single-selection list root");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pList, 1, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_LIST_ITEM &&
		strcmp(tAccessible.sName, "Scene") == 0 && tAccessible.iRow == 0 &&
		(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE)) != 0u,
		"accessible list item");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pList, 3, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && (tAccessible.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0u &&
		xuiWidgetPerformAccessibleAction(pList, 4u, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) == XUI_ERROR_INVALID_STATE,
		"accessible disabled item");

	tListWorld = xuiWidgetGetWorldRect(pList);
	tItem = xuiListViewGetItemRect(pList, 3);
	iRet = xuiInputPointerMove(pContext, tListWorld.fX + tItem.fX + 8.0f, tListWorld.fY + tItem.fY + 8.0f, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetHoverIndex(pList) == 3, "pointer hover item");
	XUI_TEST_CHECK(xuiListViewGetItemAt(pList, tItem.fX + 8.0f, tItem.fY + 8.0f) == 3,
		"local item hit test");
	tItem = xuiListViewGetItemRect(pList, 2);
	iRet = __xuiListViewClick(pContext, tListWorld.fX + tItem.fX + 8.0f, tListWorld.fY + tItem.fY + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "click disabled");
	XUI_TEST_CHECK(xuiListViewGetSelected(pList) == 1 && iSelectCount == 0, "disabled not selected");

	tItem = xuiListViewGetItemRect(pList, 3);
	iRet = __xuiListViewClick(pContext, tListWorld.fX + tItem.fX + 8.0f, tListWorld.fY + tItem.fY + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "click enabled");
	XUI_TEST_CHECK(xuiListViewGetSelected(pList) == 3 && iSelectCount == 1, "click selects");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pList, "click focus");
	iRet = __xuiListViewRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render focused list");
	pCache = xuiWidgetGetCacheSurface(pList, xuiWidgetGetStateId(pList));
	XUI_TEST_CHECK((pCache != NULL) && (xuiTestSurfaceGetLastColor(pCache) == iBorderColor), "focused list keeps border");

	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetSelected(pList) == 4, "keyboard down");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_HOME, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetSelected(pList) == 0, "keyboard home");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ENTER, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEventState.iActivateCount == 1 &&
		tEventState.iActivateIndex == 0 && tEventState.iActivateButton == XUI_POINTER_BUTTON_LEFT,
		"enter activates focused item");
	iRet = xuiWidgetPerformAccessibleAction(pList, 5u, XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetSelected(pList) == 3,
		"accessible item selection");
	iRet = xuiWidgetPerformAccessibleAction(pList, 5u, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && tEventState.iActivateCount == 2 && tEventState.iActivateIndex == 3,
		"accessible item activation");

	tItem = xuiListViewGetItemRect(pList, 1);
	iRet = xuiInputPointerDown(pContext, tListWorld.fX + tItem.fX + 8.0f, tListWorld.fY + tItem.fY + 8.0f, XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, tListWorld.fX + tItem.fX + 8.0f, tListWorld.fY + tItem.fY + 8.0f, XUI_POINTER_BUTTON_RIGHT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetSelected(pList) == 1, "right click selects");
	XUI_TEST_CHECK(tEventState.iContextMenuCount == 1 && tEventState.iContextIndex == 1, "right click semantic callback");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_CONTEXT_MENU, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEventState.iContextMenuCount == 2 &&
		tEventState.iContextIndex == 1 && tEventState.fContextX >= tListWorld.fX &&
		tEventState.fContextY >= tListWorld.fY, "menu key selected item anchor");

	tItem = xuiListViewGetItemRect(pList, 4);
	iRet = __xuiListViewClick(pContext, tListWorld.fX + tItem.fX + 8.0f, tListWorld.fY + tItem.fY + 8.0f);
	if ( iRet == XUI_OK ) iRet = __xuiListViewClick(pContext, tListWorld.fX + tItem.fX + 8.0f, tListWorld.fY + tItem.fY + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetSelected(pList) == 4 &&
		tEventState.iDoubleClickCount >= 1 && tEventState.iActivateCount >= 3 &&
		tEventState.iActivateIndex == 4, "double click selects and activates");

	iRet = xuiListViewEnsureVisible(pList, 15);
	XUI_TEST_CHECK(iRet == XUI_OK, "ensure visible");
	fScroll = xuiListViewGetScroll(pList);
	XUI_TEST_CHECK(fScroll > 0.0f, "ensure scrolls");
	tItem = xuiListViewGetItemRect(pList, 15);
	XUI_TEST_CHECK(tItem.fY + tItem.fH <= xuiListViewGetViewportRect(pList).fY + xuiListViewGetViewportRect(pList).fH + 0.01f, "item visible");

	iRet = xuiListViewSetSelectionMode(pList, XUI_SELECTION_RANGE);
	XUI_TEST_CHECK(iRet == XUI_OK, "range mode");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pList, 0, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_MULTISELECTABLE) != 0u,
		"accessible range-selection list root");
	iRet = xuiListViewSetSelectionBuffer(pList, arrOversized, 20);
	XUI_TEST_CHECK(iRet == XUI_OK && arrOversized[16] == 0x5110 &&
		arrOversized[17] == 0x5111 && arrOversized[18] == 0x5112 &&
		arrOversized[19] == 0x5113,
		"external selection binding preserves capacity beyond current rows");
	iRet = xuiListViewSetSelectionBuffer(pList, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "detach oversized external selection");
	iRet = xuiListViewSetSelectionBuffer(pList, arrTooSmall, 15);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT,
		"short external selection buffer is rejected at runtime");
	iRet = xuiListViewSetSelectionBuffer(pList, arrSelected, 16);
	XUI_TEST_CHECK(iRet == XUI_OK, "external selection");
	iRet = xuiListViewSetItems(pList, arrExpanded, 17);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiListViewGetItemCount(pList) == 16 &&
		strcmp(xuiListViewGetItemText(pList, 0), "Scene") == 0,
		"bound selection capacity rejects transactional item growth");
	iRet = xuiListViewSetSelected(pList, 5);
	XUI_TEST_CHECK(iRet == XUI_OK && arrSelected[5], "range base selected");
	arrSelected[5] = 0;
	iRet = xuiListViewSetSelected(pList, 5);
	XUI_TEST_CHECK(iRet == XUI_OK && arrSelected[5],
		"selected setter repairs externally cleared current row");
	tItem = xuiListViewGetItemRect(pList, 5);
	memset(&tPointerEvent, 0, sizeof(tPointerEvent));
	tPointerEvent.iSize = sizeof(tPointerEvent);
	tPointerEvent.iType = XUI_EVENT_POINTER_DOWN;
	tPointerEvent.pTarget = pList;
	tPointerEvent.iPointerType = XUI_POINTER_TYPE_MOUSE;
	tPointerEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tPointerEvent.fX = tListWorld.fX + tItem.fX + 8.0f;
	tPointerEvent.fY = tListWorld.fY + tItem.fY + 8.0f;
	iRet = xuiDispatchEvent(pContext, &tPointerEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && arrSelected[5] && !arrSelected[4],
		"plain range click collapses prior selection");
	iOldSelectCount = iSelectCount;
	iOldChangeCount = xuiListViewGetChangeCount(pList);
	iRet = xuiDispatchEvent(pContext, &tPointerEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectCount == iOldSelectCount &&
		xuiListViewGetChangeCount(pList) == iOldChangeCount,
		"repeated multi-selection click is idempotent by default");
	iRet = xuiListViewSetNotifyRepeatSelect(pList, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchEvent(pContext, &tPointerEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelectCount == iOldSelectCount + 1 &&
		xuiListViewGetChangeCount(pList) == iOldChangeCount,
		"repeat selection notification does not report a state change");
	iRet = xuiListViewSetNotifyRepeatSelect(pList, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "repeat selection restore after range test");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, XUI_MOD_SHIFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && arrSelected[5] && arrSelected[6], "shift range selects");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, XUI_MOD_CTRL);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetFocusIndex(pList) == 7 &&
		arrSelected[5] && arrSelected[6] && !arrSelected[7],
		"ctrl arrow moves focus without replacing selection");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, XUI_MOD_CTRL);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && arrSelected[7], "ctrl space toggles focused row");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, XUI_MOD_CTRL);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetFocusIndex(pList) == 8 &&
		arrSelected[5] && arrSelected[6] && arrSelected[7] && !arrSelected[8],
		"ctrl arrow advances focus independently after toggle");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !arrSelected[5] && !arrSelected[6] &&
		!arrSelected[7] && arrSelected[8],
		"plain space selects the focused row instead of the stale selected row");
	iRet = xuiInputKeyDown(pContext, 'A', XUI_MOD_CTRL);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && arrSelected[0] && arrSelected[1] && !arrSelected[2] &&
		arrSelected[15], "ctrl a selects every enabled row");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, XUI_MOD_CTRL);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !arrSelected[8] && xuiListViewGetSelected(pList) != 8,
		"ctrl space deselects focus without reporting it selected");
	iRet = xuiListViewSetSelectionBuffer(pList, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewIsItemSelected(pList, 5),
		"detaching external selection preserves current state");
	iOldChangeCount = xuiListViewGetChangeCount(pList);
	fScroll = xuiListViewGetScroll(pList);
	iRet = xuiListViewSetItems(pList, arrItems, 16);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiListViewIsItemEnabled(pList, 2) &&
		xuiListViewIsItemSelected(pList, 5) && xuiListViewIsItemSelected(pList, 6) &&
		xuiListViewGetChangeCount(pList) == iOldChangeCount &&
		__xuiListViewNear(xuiListViewGetScroll(pList), fScroll),
		"repeated owned items preserve enabled, selection, and scroll state");
	memset(arrSelected, 0, sizeof(arrSelected));
	XUI_TEST_CHECK(xuiListViewIsItemSelected(pList, 5),
		"detached selection no longer aliases external buffer");
	iRet = xuiListViewClearSelection(pList);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetSelected(pList) == -1, "clear selection");
	iOldChangeCount = xuiListViewGetChangeCount(pList);
	iRet = xuiListViewClearSelection(pList);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetChangeCount(pList) == iOldChangeCount,
		"repeated clear selection is idempotent");
	iRet = xuiListViewSetItemSelected(pList, 5, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewIsItemSelected(pList, 5), "set item selected");
	iRet = xuiListViewSetItemSelected(pList, 6, 1);
	if ( iRet == XUI_OK ) iRet = xuiListViewEnsureVisible(pList, 7);
	tItem = xuiListViewGetItemRect(pList, 7);
	memset(&tContextEvent, 0, sizeof(tContextEvent));
	tContextEvent.iSize = sizeof(tContextEvent);
	tContextEvent.iType = XUI_EVENT_CONTEXT_MENU;
	tContextEvent.pTarget = pList;
	tContextEvent.fX = tListWorld.fX + tItem.fX + 8.0f;
	tContextEvent.fY = tListWorld.fY + tItem.fY + 8.0f;
	tContextEvent.iButton = XUI_POINTER_BUTTON_RIGHT;
	tContextEvent.iModifiers = XUI_MOD_CTRL | XUI_MOD_SHIFT;
	if ( iRet == XUI_OK ) iRet = xuiDispatchEvent(pContext, &tContextEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewIsItemSelected(pList, 7) &&
		!xuiListViewIsItemSelected(pList, 5) && !xuiListViewIsItemSelected(pList, 6),
		"right click on an unselected row ignores range modifiers");
	XUI_TEST_CHECK(xuiListViewGetSelectCount(pList) > 0 && xuiListViewGetChangeCount(pList) > 0,
		"selection diagnostics");

	fOldItemHeight = xuiListViewGetItemHeight(pList);
	iRet = xuiListViewSetItemHeight(pList, XUI_LAYOUT_UNBOUNDED);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		__xuiListViewNear(xuiListViewGetItemHeight(pList), fOldItemHeight),
		"oversized row height is rejected transactionally");
	iRet = xuiListViewSetMetrics(pList, 22.5f, 9.0f, 1.0f);
	if ( iRet == XUI_OK ) iRet = xuiListViewSetScroll(pList, 0.0f);
	tItem = xuiListViewGetItemRect(pList, 1);
	tItemNext = xuiListViewGetItemRect(pList, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && tItem.fY + tItem.fH == tItemNext.fY &&
		xuiListViewGetItemAt(pList, tItemNext.fX + 1.0f, tItemNext.fY) == 2 &&
		xuiListViewGetItemAt(pList, tItemNext.fX + 1.0f, tItemNext.fY - 1.0f) == 1,
		"fractional row metrics keep rendering and hit boundaries contiguous");
	iRet = xuiListViewSetMetrics(pList, 30.0f, 9.0f, 1.0f);
	if ( iRet == XUI_OK ) iRet = xuiScrollModelGetContentSize(xuiListViewGetModel(pList), NULL, &fContentHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiListViewNear(fContentHeight, 480.0f),
		"metrics update scroll extent immediately");
	iRet = xuiListViewSetMetrics(pList, 30.0f, 0.0f, 1.0f);
	if ( iRet == XUI_OK ) iRet = xuiListViewGetMetrics(pList, NULL, &fPadding, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiListViewNear(fPadding, 0.0f), "zero list padding");

	iRet = xuiListViewSetScroll(pList, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiListViewNear(xuiListViewGetScroll(pList), 0.0f), "set scroll");

	memset(&tInvalidDesc, 0, sizeof(tInvalidDesc));
	tInvalidDesc.iSize = sizeof(tInvalidDesc);
	tInvalidDesc.arrItems = arrItems;
	tInvalidDesc.iItemCount = 2;
	iRet = xuiListViewCreate(pContext, &pDestroyList, &tInvalidDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pDestroyList);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pDestroyList, (xui_rect_t){240.0f, 10.0f, 150.0f, 70.0f});
	tSelectDestroy.ppWidget = &pDestroyList;
	if ( iRet == XUI_OK ) iRet = xuiListViewSetSelect(pDestroyList, __xuiListViewDestroyOnSelect, &tSelectDestroy);
	if ( iRet == XUI_OK ) iRet = xuiListViewSetContextMenu(pDestroyList, __xuiListViewContext, &tEventState);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "context selection destroy list prepare");
	tListWorld = xuiWidgetGetWorldRect(pDestroyList);
	tItem = xuiListViewGetItemRect(pDestroyList, 1);
	iRet = xuiInputPointerDown(pContext, tListWorld.fX + tItem.fX + 5.0f,
		tListWorld.fY + tItem.fY + 5.0f, XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, tListWorld.fX + tItem.fX + 5.0f,
		tListWorld.fY + tItem.fY + 5.0f, XUI_POINTER_BUTTON_RIGHT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyList == NULL && tSelectDestroy.iCount == 1,
		"context selection callback may destroy listview");

	iRet = xuiListViewCreate(pContext, &pRenderDestroyList, &tInvalidDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pRenderDestroyList);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pRenderDestroyList, (xui_rect_t){240.0f, 90.0f, 150.0f, 70.0f});
	tRenderDestroy.ppWidget = &pRenderDestroyList;
	if ( iRet == XUI_OK ) iRet = xuiListViewSetItemRenderer(pRenderDestroyList,
		__xuiListViewDestroyOnRender, &tRenderDestroy);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = __xuiListViewRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && pRenderDestroyList == NULL && tRenderDestroy.iCount == 1,
		"item renderer may destroy listview");

	memset(&tInvalidDesc, 0, sizeof(tInvalidDesc));
	tInvalidDesc.iSize = sizeof(tInvalidDesc);
	tInvalidDesc.pFont = pFont;
	iRet = xuiListViewCreate(pContext, &pTextDestroyList, &tInvalidDesc);
	if ( iRet == XUI_OK ) iRet = xuiListViewSetVirtualItems(pTextDestroyList, 1,
		__xuiListViewDestroyOnText, &tTextDestroy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pTextDestroyList);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pTextDestroyList, (xui_rect_t){240.0f, 170.0f, 150.0f, 70.0f});
	tTextDestroy.ppWidget = &pTextDestroyList;
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = __xuiListViewRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && pTextDestroyList == NULL && tTextDestroy.iCount == 1,
		"virtual text provider may destroy listview");

	memset(&tInvalidDesc, 0, sizeof(tInvalidDesc));
	tInvalidDesc.iSize = sizeof(tInvalidDesc);
	iRet = xuiListViewCreate(pContext, &pVirtualList, &tInvalidDesc);
	if ( iRet == XUI_OK ) iRet = xuiListViewSetVirtualItems(pVirtualList, 10000,
		__xuiListViewVirtualText, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiListViewGetItemCount(pVirtualList) == 10000 &&
		xuiWidgetGetAccessibleNodeCount(pVirtualList) == 10001 &&
		strcmp(xuiListViewGetItemText(pVirtualList, 9999), "Virtual item 9999") == 0,
		"large virtual list remains allocation-light");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pVirtualList, 10000, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tAccessible.sName, "Virtual item 9999") == 0,
		"virtual accessible item text");
	xuiWidgetDestroy(pVirtualList);
	pVirtualList = NULL;

	tInvalidDesc.arrItems = arrItems;
	tInvalidDesc.iItemCount = 1;
	iRet = xuiListViewCreate(pContext, &pActivateDestroyList, &tInvalidDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pActivateDestroyList);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pActivateDestroyList,
		(xui_rect_t){240.0f, 170.0f, 150.0f, 70.0f});
	tActivateDestroy.ppWidget = &pActivateDestroyList;
	if ( iRet == XUI_OK ) iRet = xuiListViewSetActivate(pActivateDestroyList,
		__xuiListViewDestroyOnActivate, &tActivateDestroy);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiWidgetPerformAccessibleAction(pActivateDestroyList, 2u,
		XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pActivateDestroyList == NULL && tActivateDestroy.iCount == 1,
		"activation callback may destroy listview");

cleanup:
	if ( pVirtualList != NULL ) xuiWidgetDestroy(pVirtualList);
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_list_view_test passed\n");
	return 0;
}
