#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
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
	int bDestroyOnClose;
	xui_widget* ppWindow;
} xui_window_test_events_t;

typedef struct xui_window_focus_events_t {
	int iFocusCount;
	int iBlurCount;
} xui_window_focus_events_t;

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
		if ( pEvents->bDestroyOnClose ) {
			xuiWidgetDestroy(pWidget);
			if ( pEvents->ppWindow != NULL ) *pEvents->ppWindow = NULL;
		}
	}
}

static int __xuiWindowChildFocusEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_window_focus_events_t* pEvents = (xui_window_focus_events_t*)pUser;

	if ( pWidget == NULL || pEvent == NULL || pEvents == NULL || pEvent->pTarget != pWidget ) {
		return XUI_OK;
	}
	if ( pEvent->iType == XUI_EVENT_FOCUS ) pEvents->iFocusCount++;
	else if ( pEvent->iType == XUI_EVENT_BLUR ) pEvents->iBlurCount++;
	return XUI_OK;
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
	xui_window_focus_events_t tFocusEvents;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pInvalidWindow;
	xui_widget pMain;
	xui_widget pTop;
	xui_widget pCollapsed;
	xui_widget pFrameless;
	xui_widget pOrderWindow;
	xui_widget pDestroyWindow;
	xui_widget pOrderClient;
	xui_widget pOrderContent;
	xui_widget pOrderFooter;
	xui_widget pFocusChild;
	xui_widget pDragAdorner;
	xui_surface pTarget;
	xui_surface pCache;
	xui_font pFont;
	xui_window_desc_t tDesc;
	xui_rect_t tRect;
	xui_rect_t tWorld;
	xui_rect_t tButton;
	xui_rect_t tContentRect;
	xui_rect_t tFooterRect;
	xui_rect_t tIconBefore;
	xui_rect_t tIconAfter;
	float fTitleBarHeight;
	float fBorderWidth;
	float fResizeGrip;
	float fButtonSize;
	float fMinWidth;
	float fMinHeight;
	float fIconSize;
	char sLongTitle[XUI_WINDOW_TITLE_CAPACITY + 4];
	int iLayerMain;
	int iZMain;
	int iLayerTop;
	int iZTop;
	int iFailed;
	int iChangeBefore;
	int iCloseBefore;
	int iRet;

	memset(&tEvents, 0, sizeof(tEvents));
	memset(&tFocusEvents, 0, sizeof(tFocusEvents));
	pContext = NULL;
	pRoot = NULL;
	pInvalidWindow = NULL;
	pMain = NULL;
	pTop = NULL;
	pCollapsed = NULL;
	pFrameless = NULL;
	pOrderWindow = NULL;
	pDestroyWindow = NULL;
	pOrderClient = NULL;
	pOrderContent = NULL;
	pOrderFooter = NULL;
	pFocusChild = NULL;
	pDragAdorner = NULL;
	pTarget = NULL;
	pCache = NULL;
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

	pInvalidWindow = pRoot;
	iRet = xuiWindowCreate(NULL, &pInvalidWindow, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidWindow == NULL,
		"invalid context clears create output");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fTitleBarHeight = NAN;
	pInvalidWindow = pRoot;
	iRet = xuiWindowCreate(pContext, &pInvalidWindow, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidWindow == NULL,
		"invalid descriptor clears create output");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.tIconSrc.fX = NAN;
	pInvalidWindow = pRoot;
	iRet = xuiWindowCreate(pContext, &pInvalidWindow, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidWindow == NULL,
		"descriptor rejects NaN icon source");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Main Window";
	tDesc.pFont = pFont;
	iRet = xuiWindowCreate(pContext, &pMain, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pMain != NULL, "main create");
	(void)xuiWidgetSetRect(pMain, (xui_rect_t){80.0f, 70.0f, 260.0f, 180.0f});
	iRet = xuiWindowSetClose(pMain, __xuiWindowClosed, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "close callback");
	memset(sLongTitle, 'a', sizeof(sLongTitle));
	memcpy(sLongTitle + XUI_WINDOW_TITLE_CAPACITY - 2, "\xE4\xB8\xAD", 4);
	iRet = xuiWindowSetTitle(pMain, sLongTitle);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		strlen(xuiWindowGetTitle(pMain)) == XUI_WINDOW_TITLE_CAPACITY - 2u &&
		xuiWindowGetTitle(pMain)[XUI_WINDOW_TITLE_CAPACITY - 2] == '\0',
		"title truncation preserves UTF-8 boundary");
	memset(sLongTitle, 'a', XUI_WINDOW_TITLE_CAPACITY - 2u);
	memcpy(sLongTitle + XUI_WINDOW_TITLE_CAPACITY - 2u, "e\xCC\x81", 4u);
	iRet = xuiWindowSetTitle(pMain, sLongTitle);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		strlen(xuiWindowGetTitle(pMain)) == XUI_WINDOW_TITLE_CAPACITY - 2u,
		"title truncation preserves grapheme boundary");
	iRet = xuiWindowSetTitle(pMain, "Main Window");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore title after UTF-8 truncation");
	iRet = xuiWidgetSetEnabled(pMain, 0);
	if ( iRet == XUI_OK ) iRet = xuiWindowSetOpen(pMain, 0);
	if ( iRet == XUI_OK ) iRet = xuiWindowSetOpen(pMain, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiWidgetGetEnabled(pMain) &&
		!xuiWindowIsActive(pMain), "reopening preserves disabled state");
	iRet = xuiWidgetSetEnabled(pMain, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore main enabled state");
	iRet = xuiWindowBringToFront(pMain);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pMain, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiWindowIsActive(pMain) &&
		xuiWindowGetActive(pContext) != pMain &&
		xuiWindowBringToFront(pMain) == XUI_ERROR_INVALID_STATE,
		"disabled window leaves active state");
	iRet = xuiWidgetSetEnabled(pMain, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore enabled state after active-state test");
	iRet = __xuiWindowAddFixedChild(pContext, pMain);
	XUI_TEST_CHECK(iRet == XUI_OK, "add child to client");
	iRet = xuiWidgetCreate(pContext, &pFocusChild);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pFocusChild, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPreferredSize(pFocusChild, (xui_vec2_t){0.0f, 28.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pFocusChild, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pFocusChild, XUI_EVENT_FOCUS,
		__xuiWindowChildFocusEvent, &tFocusEvents);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pFocusChild, XUI_EVENT_BLUR,
		__xuiWindowChildFocusEvent, &tFocusEvents);
	if ( iRet == XUI_OK ) iRet = xuiWindowAddChild(pMain, pFocusChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "add focus child to client");
	iRet = xuiSetFocusWidget(pContext, pFocusChild);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pFocusChild &&
		tFocusEvents.iFocusCount == 1 && tFocusEvents.iBlurCount == 0,
		"window forwards descendant focus events");
	iRet = xuiWindowBringToFront(pMain);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pFocusChild &&
		tFocusEvents.iFocusCount == 1 && tFocusEvents.iBlurCount == 0,
		"window activation preserves descendant focus");

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

	iRet = xuiWindowGetChrome(pMain, &fTitleBarHeight, &fBorderWidth, &fResizeGrip, &fButtonSize);
	XUI_TEST_CHECK(iRet == XUI_OK, "chrome get before invalid set");
	iRet = xuiWindowSetChrome(pMain, NAN, fBorderWidth, fResizeGrip, fButtonSize);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "chrome rejects NaN");
	iRet = xuiWindowSetChrome(pMain, fTitleBarHeight, fBorderWidth, INFINITY, fButtonSize);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "chrome rejects infinity");
	{
		float fTitleAfter;
		float fBorderAfter;
		float fGripAfter;
		float fButtonAfter;
		iRet = xuiWindowGetChrome(pMain, &fTitleAfter, &fBorderAfter, &fGripAfter, &fButtonAfter);
		XUI_TEST_CHECK(iRet == XUI_OK && fTitleAfter == fTitleBarHeight && fBorderAfter == fBorderWidth &&
			fGripAfter == fResizeGrip && fButtonAfter == fButtonSize, "invalid chrome preserves state");
	}

	fIconSize = xuiWindowGetIconSize(pMain);
	iRet = xuiWindowSetIconSize(pMain, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiWindowGetIconSize(pMain) == fIconSize,
		"icon size rejects NaN without mutation");
	iRet = xuiWindowSetIconSize(pMain, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiWindowGetIconSize(pMain) == fIconSize,
		"icon size rejects infinity without mutation");

	iRet = xuiWindowGetMinSize(pMain, &fMinWidth, &fMinHeight);
	XUI_TEST_CHECK(iRet == XUI_OK, "minimum size get before invalid set");
	iRet = xuiWindowSetMinSize(pMain, NAN, fMinHeight);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "minimum size rejects NaN");
	iRet = xuiWindowSetMinSize(pMain, fMinWidth, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "minimum size rejects infinity");
	{
		float fWidthAfter;
		float fHeightAfter;
		iRet = xuiWindowGetMinSize(pMain, &fWidthAfter, &fHeightAfter);
		XUI_TEST_CHECK(iRet == XUI_OK && fWidthAfter == fMinWidth && fHeightAfter == fMinHeight,
			"invalid minimum size preserves state");
	}

	iRet = xuiWindowSetIcon(pMain, pTarget, (xui_rect_t){1, 2, 16, 16});
	XUI_TEST_CHECK(iRet == XUI_OK, "valid icon source set");
	tIconBefore = xuiWindowGetIconSource(pMain);
	iRet = xuiWindowSetIcon(pMain, pTarget, (xui_rect_t){1, 2, -1, 16});
	tIconAfter = xuiWindowGetIconSource(pMain);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		memcmp(&tIconBefore, &tIconAfter, sizeof(tIconBefore)) == 0,
		"invalid icon source preserves state");
	iRet = xuiWindowSetIcon(pMain, pTarget, (xui_rect_t){NAN, 2, 16, 16});
	tIconAfter = xuiWindowGetIconSource(pMain);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		memcmp(&tIconBefore, &tIconAfter, sizeof(tIconBefore)) == 0,
		"NaN icon source preserves state");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");
	pCache = xuiWidgetGetCacheSurface(pMain, xuiWidgetGetStateId(pMain));
	XUI_TEST_CHECK(pCache != NULL, "window cache surface");
	xuiTestSurfaceReset(pCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pCache) == 0,
		"unchanged borrowed window icon reuses cache");
	iRet = tState.tProxy.surfaceUpdateRGBA(&tState.tProxy, pTarget,
		(xui_rect_i_t){0, 0, 1, 1}, NULL, 0);
	if ( iRet == XUI_OK ) iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pCache) > 0,
		"borrowed window icon generation rebuilds cache");

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
	tRect = xuiWidgetGetRect(pMain);
	iRet = xuiWindowSetMaximized(pMain, 1);
	if ( iRet == XUI_OK ) iRet = xuiInputViewport(pContext, 700, 460);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	tWorld = xuiWidgetGetRect(pMain);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiWindowNear(tWorld.fW, 700.0f) &&
		__xuiWindowNear(tWorld.fH, 460.0f), "maximized window follows viewport growth");
	iRet = xuiInputViewport(pContext, 640, 420);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiWindowSetMaximized(pMain, 0);
	tWorld = xuiWidgetGetRect(pMain);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiWindowNear(tWorld.fX, tRect.fX) &&
		__xuiWindowNear(tWorld.fY, tRect.fY) && __xuiWindowNear(tWorld.fW, tRect.fW) &&
		__xuiWindowNear(tWorld.fH, tRect.fH), "viewport resize preserves maximize restore rect");

	/* Window Z changes must not affect Column order when only one child resyncs. */
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Layout Order";
	tDesc.pFont = pFont;
	tDesc.bClosed = 1;
	iRet = xuiWindowCreate(pContext, &pOrderWindow, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pOrderWindow != NULL, "layout order window create");
	(void)xuiWidgetSetRect(pOrderWindow, (xui_rect_t){360.0f, 40.0f, 220.0f, 180.0f});
	pOrderClient = xuiWindowGetClientWidget(pOrderWindow);
	XUI_TEST_CHECK(pOrderClient != NULL, "layout order client");
	iRet = xuiWidgetSetLayoutType(pOrderClient, XUI_LAYOUT_COLUMN);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order column");
	iRet = xuiWidgetSetGap(pOrderClient, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order gap");
	iRet = xuiWidgetCreate(pContext, &pOrderContent);
	XUI_TEST_CHECK(iRet == XUI_OK && pOrderContent != NULL, "layout order content create");
	iRet = xuiWidgetSetSizeMode(pOrderContent, XUI_SIZE_FILL, XUI_SIZE_FILL);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order content fill");
	iRet = xuiWidgetCreate(pContext, &pOrderFooter);
	XUI_TEST_CHECK(iRet == XUI_OK && pOrderFooter != NULL, "layout order footer create");
	iRet = xuiWidgetSetSizeMode(pOrderFooter, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order footer mode");
	iRet = xuiWidgetSetPreferredSize(pOrderFooter, (xui_vec2_t){0.0f, 32.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order footer preferred");
	iRet = xuiWidgetSetMinSize(pOrderFooter, (xui_vec2_t){0.0f, 32.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order footer minimum");
	iRet = xuiWindowAddChild(pOrderWindow, pOrderContent);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order add content");
	iRet = xuiWindowAddChild(pOrderWindow, pOrderFooter);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order add footer");
	iRet = xuiWindowSetOpen(pOrderWindow, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order open");
	iRet = xuiWindowBringToFront(pOrderWindow);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order bring front");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order initial layout");
	tContentRect = xuiWidgetGetRect(pOrderContent);
	tFooterRect = xuiWidgetGetRect(pOrderFooter);
	XUI_TEST_CHECK(tContentRect.fY < tFooterRect.fY && tFooterRect.fH >= 32.0f, "layout order initial child order");
	iRet = xuiWindowSetTopMost(pOrderWindow, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order topmost");
	/* This mirrors SplitLayout drag: only the fill child requests a layout sync. */
	iRet = xuiWidgetSetPreferredSize(pOrderContent, (xui_vec2_t){0.0f, 1.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order content relayout");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout order relayout");
	tContentRect = xuiWidgetGetRect(pOrderContent);
	tFooterRect = xuiWidgetGetRect(pOrderFooter);
	XUI_TEST_CHECK(tContentRect.fY < tFooterRect.fY && tFooterRect.fH >= 32.0f,
		"window Z changed Column child order");

	tWorld = xuiWidgetGetWorldRect(pMain);
	iRet = __xuiWindowDispatchDown(pContext, tWorld.fX + 60.0f, tWorld.fY + 15.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "cancel drag down");
	iRet = __xuiWindowDispatchMove(pContext, tWorld.fX + 75.0f, tWorld.fY + 30.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "cancel drag move");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	tRect = xuiWidgetGetRect(pMain);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		__xuiWindowNear(tRect.fX, 80.0f) && __xuiWindowNear(tRect.fY, 70.0f), "escape cancels deferred drag");
	(void)xuiInputPointerUp(pContext, tWorld.fX + 75.0f, tWorld.fY + 30.0f, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pContext);

	tWorld = xuiWidgetGetWorldRect(pMain);
	iRet = __xuiWindowDispatchDown(pContext, tWorld.fX + 60.0f, tWorld.fY + 15.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag down");
	iRet = __xuiWindowDispatchMove(pContext, tWorld.fX + 90.0f, tWorld.fY + 35.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag move");
	tRect = xuiWidgetGetRect(pMain);
	XUI_TEST_CHECK(__xuiWindowNear(tRect.fX, 80.0f) && __xuiWindowNear(tRect.fY, 70.0f), "drag move remains deferred");
	pDragAdorner = xuiOverlayTop(pContext);
	XUI_TEST_CHECK(pDragAdorner != NULL && pDragAdorner != pMain &&
		xuiOverlayGetOwner(pDragAdorner) == pMain &&
		xuiWidgetGetFlowMode(pDragAdorner) == XUI_FLOW_ABSOLUTE,
		"drag adorner is an absolute overlay");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout moving drag adorner");
	tRect = xuiWidgetGetWorldRect(pDragAdorner);
	XUI_TEST_CHECK(tRect.fX > 100.0f && tRect.fY > 80.0f,
		"drag adorner follows the deferred window position");
	iRet = __xuiWindowDispatchUp(pContext, tWorld.fX + 90.0f, tWorld.fY + 35.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag up");
	tRect = xuiWidgetGetRect(pMain);
	XUI_TEST_CHECK(__xuiWindowNear(tRect.fX, 110.0f) && __xuiWindowNear(tRect.fY, 90.0f), "drag updates rect");
	XUI_TEST_CHECK(xuiWindowGetActive(pContext) == pMain && xuiWindowIsActive(pMain), "drag activates");
	iRet = xuiWidgetGetLayer(pMain, &iLayerMain, &iZMain);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag layer get");
	iRet = xuiWidgetGetLayer(xuiWindowGetClientWidget(pMain), &iLayerTop, &iZTop);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayerTop == iLayerMain && iZTop == iZMain, "client follows raised window layer");

	/* A second touch must not steal, move, or release the owning drag. */
	tWorld = xuiWidgetGetWorldRect(pMain);
	tRect = xuiWidgetGetRect(pMain);
	iRet = xuiInputPointerDownEx(pContext, 101u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + 60.0f), (int)(tWorld.fY + 15.0f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 101u, XUI_POINTER_TYPE_TOUCH) == pMain,
		"first touch owns window drag");
	iRet = xuiInputPointerDownEx(pContext, 102u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + 90.0f), (int)(tWorld.fY + 15.0f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMoveEx(pContext, 102u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + 150.0f), (int)(tWorld.fY + 45.0f), XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 102u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + 150.0f), (int)(tWorld.fY + 45.0f), XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 101u, XUI_POINTER_TYPE_TOUCH) == pMain &&
		xuiGetPointerCaptureEx(pContext, 102u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		__xuiWindowNear(xuiWidgetGetRect(pMain).fX, tRect.fX),
		"second touch cannot steal window drag");
	iRet = xuiInputPointerMoveEx(pContext, 101u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + 80.0f), (int)(tWorld.fY + 25.0f), XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 101u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + 80.0f), (int)(tWorld.fY + 25.0f), XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiWindowNear(xuiWidgetGetRect(pMain).fX, tRect.fX + 20.0f) &&
		__xuiWindowNear(xuiWidgetGetRect(pMain).fY, tRect.fY + 10.0f),
		"owning touch commits window drag");

	/* Runtime interaction changes cancel a stale preview and release its owner. */
	tWorld = xuiWidgetGetWorldRect(pMain);
	tRect = xuiWidgetGetRect(pMain);
	iRet = __xuiWindowDispatchDown(pContext, tWorld.fX + 60.0f, tWorld.fY + 15.0f);
	if ( iRet == XUI_OK ) iRet = __xuiWindowDispatchMove(pContext,
		tWorld.fX + 90.0f, tWorld.fY + 35.0f, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiWindowSetMovable(pMain, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		(xuiWindowGetState(pMain) & XUI_WINDOW_STATE_DRAGGING) == 0u &&
		__xuiWindowNear(xuiWidgetGetRect(pMain).fX, tRect.fX) &&
		__xuiWindowNear(xuiWidgetGetRect(pMain).fY, tRect.fY),
		"disabling movement cancels deferred drag");
	(void)__xuiWindowDispatchUp(pContext, tWorld.fX + 90.0f, tWorld.fY + 35.0f);
	iRet = xuiWindowSetMovable(pMain, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore movable state");

	/* A click without movement is not a geometry change. */
	tWorld = xuiWidgetGetWorldRect(pMain);
	iChangeBefore = xuiWindowGetChangeCount(pMain);
	iRet = __xuiWindowDispatchDown(pContext, tWorld.fX + 100.0f, tWorld.fY + 15.0f);
	if ( iRet == XUI_OK ) iRet = __xuiWindowDispatchUp(pContext,
		tWorld.fX + 100.0f, tWorld.fY + 15.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWindowGetChangeCount(pMain) == iChangeBefore,
		"no-op drag does not increment change count");

	iRet = xuiWindowSetOpen(pTop, 0);
	if ( iRet == XUI_OK ) iRet = xuiWindowSetOpen(pOrderWindow, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "close overlapping top-most windows before resize");
	tWorld = xuiWidgetGetWorldRect(pMain);
	/* Keep the resize probe outside the overlapping top-most window. */
	iRet = __xuiWindowDispatchDown(pContext, tWorld.fX + tWorld.fW - 2.0f, tWorld.fY + 15.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "resize down");
	iRet = __xuiWindowDispatchMove(pContext, tWorld.fX + tWorld.fW + 42.0f, tWorld.fY + 15.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "resize move");
	tRect = xuiWidgetGetRect(pMain);
	XUI_TEST_CHECK(__xuiWindowNear(tRect.fW, 260.0f), "resize move remains deferred");
	pDragAdorner = xuiOverlayTop(pContext);
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiOverlayGetOwner(pDragAdorner) == pMain,
		"layout resizing drag adorner");
	tRect = xuiWidgetGetWorldRect(pDragAdorner);
	XUI_TEST_CHECK(tRect.fX > 100.0f && tRect.fY > 80.0f && tRect.fW > 295.0f,
		"resize adorner keeps position while following size");
	iRet = __xuiWindowDispatchUp(pContext, tWorld.fX + tWorld.fW + 42.0f, tWorld.fY + 15.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "resize up");
	tRect = xuiWidgetGetRect(pMain);
	XUI_TEST_CHECK(tRect.fW > 295.0f, "resize updates width");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout after deferred resize");

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

	/* Hiding a pressed caption button releases its pointer capture. */
	tWorld = xuiWidgetGetWorldRect(pMain);
	tButton = xuiWindowGetCloseButtonRect(pMain);
	iRet = __xuiWindowDispatchDown(pContext,
		tWorld.fX + tButton.fX + tButton.fW * 0.5f,
		tWorld.fY + tButton.fY + tButton.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = xuiWindowSetShowClose(pMain, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		xuiWindowGetActivePart(pMain) == XUI_WINDOW_PART_NONE,
		"hiding pressed close button cancels capture");
	if ( iRet == XUI_OK ) iRet = xuiWindowSetShowClose(pMain, 1);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	(void)__xuiWindowDispatchUp(pContext,
		tWorld.fX + tButton.fX + tButton.fW * 0.5f,
		tWorld.fY + tButton.fY + tButton.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore close button after capture cancellation");

	iCloseBefore = tEvents.iCloseCount;
	iRet = xuiWindowBringToFront(pMain);
	if ( iRet == XUI_OK ) iRet = xuiInputKeyDown(pContext, XUI_KEY_F4, XUI_MOD_ALT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiWindowIsOpen(pMain) &&
		tEvents.iCloseCount == iCloseBefore + 1 &&
		xuiWindowGetCloseCount(pMain) == iCloseBefore + 1,
		"alt-f4 closes active window");
	iRet = xuiWindowSetOpen(pMain, 1);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "reopen after alt-f4");

	tWorld = xuiWidgetGetWorldRect(pMain);
	tButton = xuiWindowGetCloseButtonRect(pMain);
	iCloseBefore = tEvents.iCloseCount;
	iRet = __xuiWindowDispatchClick(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiWindowIsOpen(pMain) &&
		tEvents.iCloseCount == iCloseBefore + 1 &&
		xuiWindowGetCloseCount(pMain) == iCloseBefore + 1, "close button");
	iRet = xuiWindowSetOpen(pMain, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWindowIsOpen(pMain), "reopen");
	XUI_TEST_CHECK(xuiWindowGetChangeCount(pMain) >= 5, "change count");

	/* Closing callback may synchronously destroy the window. */
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Destroy on close";
	tDesc.pFont = pFont;
	tDesc.bHideCollapse = 1;
	tDesc.bHideMaximize = 1;
	iRet = xuiWindowCreate(pContext, &pDestroyWindow, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyWindow != NULL, "destroy-on-close window create");
	(void)xuiWidgetSetRect(pDestroyWindow, (xui_rect_t){360.0f, 240.0f, 220.0f, 120.0f});
	tEvents.iCloseCount = 0;
	tEvents.bDestroyOnClose = 1;
	tEvents.ppWindow = &pDestroyWindow;
	iRet = xuiWindowSetClose(pDestroyWindow, __xuiWindowClosed, &tEvents);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) {
		tWorld = xuiWidgetGetWorldRect(pDestroyWindow);
		tButton = xuiWindowGetCloseButtonRect(pDestroyWindow);
		iRet = __xuiWindowDispatchClick(pContext,
			tWorld.fX + tButton.fX + tButton.fW * 0.5f,
			tWorld.fY + tButton.fY + tButton.fH * 0.5f);
	}
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyWindow == NULL && tEvents.iCloseCount == 1,
		"close callback may destroy window");

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
