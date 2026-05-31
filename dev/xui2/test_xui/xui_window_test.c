#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_window_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_window_test_events_t {
	int iCloseCount;
} xui_window_test_events_t;

static int __xuiWindowNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) fD = -fD;
	return fD < 0.05f;
}

static int __xuiWindowDispatchMove(xui_context pContext, float fX, float fY, uint32_t iButtons)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fX, fY, iButtons);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiWindowDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiWindowDispatchUp(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiWindowDispatchClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = __xuiWindowDispatchMove(pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiWindowDispatchDown(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiWindowDispatchUp(pContext, fX, fY);
}

static void __xuiWindowClosed(xui_widget pWidget, void* pUser)
{
	xui_window_test_events_t* pEvents;

	(void)pWidget;
	pEvents = (xui_window_test_events_t*)pUser;
	if ( pEvents != NULL ) {
		pEvents->iCloseCount++;
	}
}

static int __xuiWindowAddFixedChild(xui_context pContext, xui_widget pWindow)
{
	xui_widget pChild;
	int iRet;

	iRet = xuiWidgetCreate(pContext, &pChild);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetSizeMode(pChild, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	(void)xuiWidgetSetPreferredSize(pChild, (xui_vec2_t){0.0f, 28.0f});
	(void)xuiWidgetSetAlign(pChild, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	iRet = xuiWindowAddChild(pWindow, pChild);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pChild);
		return iRet;
	}
	return xuiWidgetGetParent(pChild) == xuiWindowGetClientWidget(pWindow) ? XUI_OK : XUI_ERROR;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_window_test_events_t tEvents;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pMain;
	xui_widget pTop;
	xui_widget pCollapsed;
	xui_widget pFrameless;
	xui_surface pTarget;
	xui_font pFont;
	xui_window_desc_t tDesc;
	xui_rect_t tRect;
	xui_rect_t tWorld;
	xui_rect_t tButton;
	int iLayerMain;
	int iZMain;
	int iLayerTop;
	int iZTop;
	int iFailed;
	int iRet;

	memset(&tEvents, 0, sizeof(tEvents));
	pContext = NULL;
	pRoot = NULL;
	pMain = NULL;
	pTop = NULL;
	pCollapsed = NULL;
	pFrameless = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 640.0f, 420.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "body", 4, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "font default");
	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	(void)xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 640.0f, 420.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Main Window";
	tDesc.pFont = pFont;
	iRet = xuiWindowCreate(pContext, &pMain, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pMain != NULL, "main create");
	(void)xuiWidgetSetRect(pMain, (xui_rect_t){80.0f, 70.0f, 260.0f, 180.0f});
	iRet = xuiWindowSetClose(pMain, __xuiWindowClosed, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "close callback");
	iRet = __xuiWindowAddFixedChild(pContext, pMain);
	XUI_TEST_CHECK(iRet == XUI_OK, "add child to client");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Top Most";
	tDesc.pFont = pFont;
	tDesc.bTopMost = 1;
	iRet = xuiWindowCreate(pContext, &pTop, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTop != NULL, "top create");
	(void)xuiWidgetSetRect(pTop, (xui_rect_t){220.0f, 110.0f, 220.0f, 150.0f});

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Collapsed";
	tDesc.pFont = pFont;
	tDesc.bCollapsed = 1;
	iRet = xuiWindowCreate(pContext, &pCollapsed, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pCollapsed != NULL, "collapsed create");
	(void)xuiWidgetSetRect(pCollapsed, (xui_rect_t){70.0f, 280.0f, 240.0f, 64.0f});

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Frameless";
	tDesc.pFont = pFont;
	tDesc.bNoTitleBar = 1;
	tDesc.bDragAnywhere = 1;
	tDesc.bHideCollapse = 1;
	tDesc.bHideMaximize = 1;
	tDesc.bHideClose = 1;
	iRet = xuiWindowCreate(pContext, &pFrameless, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pFrameless != NULL, "frameless create");
	(void)xuiWidgetSetRect(pFrameless, (xui_rect_t){390.0f, 275.0f, 160.0f, 96.0f});

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 640, 420, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");

	XUI_TEST_CHECK(xuiWindowGetClientWidget(pMain) != NULL, "client widget");
	XUI_TEST_CHECK(xuiWindowGetCloseButtonWidget(pMain) != NULL, "close button");
	XUI_TEST_CHECK(strcmp(xuiWindowGetTitle(pMain), "Main Window") == 0, "title getter");
	XUI_TEST_CHECK(xuiWidgetGetParent(pMain) == xuiOverlayRoot(pContext), "overlay parent");
	iRet = xuiWidgetGetLayer(pMain, &iLayerMain, &iZMain);
	XUI_TEST_CHECK(iRet == XUI_OK, "main layer get");
	iRet = xuiWidgetGetLayer(pTop, &iLayerTop, &iZTop);
	XUI_TEST_CHECK(iRet == XUI_OK, "top layer get");
	XUI_TEST_CHECK(iLayerMain == XUI_LAYER_FLOATING && iLayerTop == XUI_LAYER_FLOATING && iZTop > iZMain, "floating z groups");
	iRet = xuiWidgetGetLayer(xuiWindowGetClientWidget(pMain), &iLayerTop, &iZTop);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayerTop == iLayerMain && iZTop == iZMain, "client follows window layer");
	XUI_TEST_CHECK(xuiWindowIsTopMost(pTop) && xuiWindowGetActive(pContext) == pFrameless, "active latest");
	XUI_TEST_CHECK(xuiWindowIsCollapsed(pCollapsed) && !xuiWidgetGetVisible(xuiWindowGetClientWidget(pCollapsed)), "collapsed hides client");
	XUI_TEST_CHECK(xuiWindowGetState(pTop) & XUI_WINDOW_STATE_TOPMOST, "state topmost");

	tWorld = xuiWidgetGetWorldRect(pMain);
	iRet = __xuiWindowDispatchDown(pContext, tWorld.fX + 60.0f, tWorld.fY + 15.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag down");
	iRet = __xuiWindowDispatchMove(pContext, tWorld.fX + 90.0f, tWorld.fY + 35.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag move");
	iRet = __xuiWindowDispatchUp(pContext, tWorld.fX + 90.0f, tWorld.fY + 35.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag up");
	tRect = xuiWidgetGetRect(pMain);
	XUI_TEST_CHECK(__xuiWindowNear(tRect.fX, 110.0f) && __xuiWindowNear(tRect.fY, 90.0f), "drag updates rect");
	XUI_TEST_CHECK(xuiWindowGetActive(pContext) == pMain && xuiWindowIsActive(pMain), "drag activates");
	iRet = xuiWidgetGetLayer(pMain, &iLayerMain, &iZMain);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag layer get");
	iRet = xuiWidgetGetLayer(xuiWindowGetClientWidget(pMain), &iLayerTop, &iZTop);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayerTop == iLayerMain && iZTop == iZMain, "client follows raised window layer");

	tWorld = xuiWidgetGetWorldRect(pMain);
	iRet = __xuiWindowDispatchDown(pContext, tWorld.fX + tWorld.fW - 2.0f, tWorld.fY + 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "resize down");
	iRet = __xuiWindowDispatchMove(pContext, tWorld.fX + tWorld.fW + 42.0f, tWorld.fY + 80.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "resize move");
	iRet = __xuiWindowDispatchUp(pContext, tWorld.fX + tWorld.fW + 42.0f, tWorld.fY + 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "resize up");
	tRect = xuiWidgetGetRect(pMain);
	XUI_TEST_CHECK(tRect.fW > 295.0f, "resize updates width");

	tWorld = xuiWidgetGetWorldRect(pMain);
	tButton = xuiWindowGetCollapseButtonRect(pMain);
	iRet = __xuiWindowDispatchClick(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWindowIsCollapsed(pMain), "collapse button");
	iRet = __xuiWindowDispatchClick(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiWindowIsCollapsed(pMain), "expand button");

	iRet = xuiWindowSetMaximized(pMain, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWindowIsMaximized(pMain), "maximize set");
	tRect = xuiWidgetGetRect(pMain);
	XUI_TEST_CHECK(tRect.fW >= 639.0f && tRect.fH >= 419.0f, "maximized rect");
	iRet = xuiWindowSetMaximized(pMain, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiWindowIsMaximized(pMain), "restore set");

	tWorld = xuiWidgetGetWorldRect(pFrameless);
	XUI_TEST_CHECK((xuiWindowGetResizeEdges(pFrameless) & XUI_WINDOW_EDGE_TOP) == 0u, "frameless drag-anywhere omits top resize edge");
	XUI_TEST_CHECK(xuiWindowGetResizeEdgesAt(pFrameless, tWorld.fX + tWorld.fW * 0.5f, tWorld.fY + 2.0f) == 0u, "frameless top strip is not resize");
	iRet = __xuiWindowDispatchDown(pContext, tWorld.fX + tWorld.fW * 0.5f, tWorld.fY + 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "frameless top down");
	iRet = __xuiWindowDispatchMove(pContext, tWorld.fX + tWorld.fW * 0.5f + 15.0f, tWorld.fY + 18.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "frameless top drag move");
	iRet = __xuiWindowDispatchUp(pContext, tWorld.fX + tWorld.fW * 0.5f + 15.0f, tWorld.fY + 18.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "frameless top up");
	tRect = xuiWidgetGetRect(pFrameless);
	XUI_TEST_CHECK(__xuiWindowNear(tRect.fX, 405.0f) && __xuiWindowNear(tRect.fY, 291.0f) &&
		__xuiWindowNear(tRect.fW, 160.0f) && __xuiWindowNear(tRect.fH, 96.0f), "frameless top drag moves without resize");

	tWorld = xuiWidgetGetWorldRect(pFrameless);
	iRet = __xuiWindowDispatchDown(pContext, tWorld.fX + 20.0f, tWorld.fY + 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "frameless down");
	iRet = __xuiWindowDispatchMove(pContext, tWorld.fX + 45.0f, tWorld.fY + 34.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "frameless drag move");
	iRet = __xuiWindowDispatchUp(pContext, tWorld.fX + 45.0f, tWorld.fY + 34.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "frameless up");
	tRect = xuiWidgetGetRect(pFrameless);
	XUI_TEST_CHECK(tRect.fX > 410.0f && tRect.fY > 285.0f, "frameless drag anywhere");

	tWorld = xuiWidgetGetWorldRect(pMain);
	tButton = xuiWindowGetCloseButtonRect(pMain);
	iRet = __xuiWindowDispatchClick(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiWindowIsOpen(pMain) && tEvents.iCloseCount == 1 && xuiWindowGetCloseCount(pMain) == 1, "close button");
	iRet = xuiWindowSetOpen(pMain, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWindowIsOpen(pMain), "reopen");
	XUI_TEST_CHECK(xuiWindowGetChangeCount(pMain) >= 5, "change count");

cleanup:
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_window_test passed\n");
	return 0;
}
