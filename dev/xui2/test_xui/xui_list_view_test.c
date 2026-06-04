#include "xui.h"
#include "xui_test_proxy.h"

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
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pList;
	xui_surface pTarget;
	xui_surface pCache;
	xui_list_view_desc_t tDesc;
	xui_rect_t tItem;
	xui_rect_t tListWorld;
	uint32_t iBorderColor;
	float fScroll;
	int iSelectCount;
	int iRenderCount;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pList = NULL;
	pTarget = NULL;
	pCache = NULL;
	iBorderColor = XUI_COLOR_RGBA(96, 148, 204, 255);
	iSelectCount = 0;
	iRenderCount = 0;
	iFailed = 0;
	memset(arrEnabled, 1, sizeof(arrEnabled));
	memset(arrSelected, 0, sizeof(arrSelected));
	arrEnabled[2] = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
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
	iRet = xuiWidgetAddChild(pRoot, pList);
	XUI_TEST_CHECK(iRet == XUI_OK, "add listview");
	xuiWidgetSetRect(pList, (xui_rect_t){24.0f, 20.0f, 190.0f, 120.0f});
	iRet = xuiListViewSetSelect(pList, __xuiListViewSelected, &iSelectCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");
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
	XUI_TEST_CHECK(xuiListViewGetSelected(pList) == 1, "initial selected");
	XUI_TEST_CHECK(!xuiListViewIsItemEnabled(pList, 2), "disabled item");
	XUI_TEST_CHECK(xuiListViewGetFrameWidget(pList) != NULL, "frame exists");
	XUI_TEST_CHECK(xuiListViewGetViewportWidget(pList) != NULL, "viewport exists");
	XUI_TEST_CHECK(xuiScrollFrameIsVScrollBarVisible(xuiListViewGetFrameWidget(pList)), "vertical bar visible");
	XUI_TEST_CHECK(!xuiScrollFrameIsHScrollBarVisible(xuiListViewGetFrameWidget(pList)), "horizontal bar hidden");

	tListWorld = xuiWidgetGetWorldRect(pList);
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

	iRet = xuiListViewEnsureVisible(pList, 15);
	XUI_TEST_CHECK(iRet == XUI_OK, "ensure visible");
	fScroll = xuiListViewGetScroll(pList);
	XUI_TEST_CHECK(fScroll > 0.0f, "ensure scrolls");
	tItem = xuiListViewGetItemRect(pList, 15);
	XUI_TEST_CHECK(tItem.fY + tItem.fH <= xuiListViewGetViewportRect(pList).fY + xuiListViewGetViewportRect(pList).fH + 0.01f, "item visible");

	iRet = xuiListViewSetSelectionMode(pList, XUI_SELECTION_RANGE);
	XUI_TEST_CHECK(iRet == XUI_OK, "range mode");
	iRet = xuiListViewSetSelectionBuffer(pList, arrSelected, 16);
	XUI_TEST_CHECK(iRet == XUI_OK, "external selection");
	iRet = xuiListViewSetSelected(pList, 5);
	XUI_TEST_CHECK(iRet == XUI_OK && arrSelected[5], "range base selected");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, XUI_MOD_SHIFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && arrSelected[5] && arrSelected[6], "shift range selects");

	iRet = xuiListViewSetScroll(pList, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiListViewNear(xuiListViewGetScroll(pList), 0.0f), "set scroll");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_list_view_test passed\n");
	return 0;
}
