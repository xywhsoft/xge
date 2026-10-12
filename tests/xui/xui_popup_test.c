#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_popup_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiPopupNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) {
		fD = -fD;
	}
	return fD < 0.01f;
}

static void __xuiPopupChanged(xui_widget pWidget, int bOpen, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)bOpen;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

static int __xuiPopupRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 360, 260};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiPopupDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pOwner;
	xui_widget pPopup;
	xui_widget pPanel;
	xui_widget pScrollView;
	xui_widget pFrame;
	xui_widget pContent;
	xui_widget pChild;
	xui_surface pTarget;
	xui_popup_desc_t tDesc;
	xui_rect_t tPopup;
	xui_rect_t tViewport;
	xui_rect_t tOwner;
	float fOffsetX;
	float fOffsetY;
	int iChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pOwner = NULL;
	pPopup = NULL;
	pPanel = NULL;
	pScrollView = NULL;
	pFrame = NULL;
	pContent = NULL;
	pChild = NULL;
	pTarget = NULL;
	iChanged = 0;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 360.0f, 260.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 360.0f, 260.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	iRet = xuiWidgetCreate(pContext, &pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK && pOwner != NULL, "owner create");
	xuiWidgetSetRect(pOwner, (xui_rect_t){40.0f, 30.0f, 92.0f, 28.0f});
	xuiWidgetSetFocusable(pOwner, 1);
	iRet = xuiWidgetAddChild(pRoot, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK, "add owner");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pOwner;
	tDesc.fContentWidth = 120.0f;
	tDesc.fContentHeight = 260.0f;
	tDesc.fMaxWidth = 180.0f;
	tDesc.fMaxHeight = 140.0f;
	tDesc.fGap = 5.0f;
	tDesc.fMargin = 4.0f;
	tDesc.fPadding = 4.0f;
	tDesc.fBorderWidth = 1.0f;
	tDesc.iAnchor = XUI_POPUP_ANCHOR_BOTTOM_LEFT;
	tDesc.iDirection = XUI_POPUP_DIRECTION_RIGHT_DOWN;
	tDesc.iOutsidePolicy = XUI_POPUP_OUTSIDE_CLOSE;
	tDesc.iOwnerPolicy = XUI_POPUP_OWNER_PASSTHROUGH;
	tDesc.iEscapePolicy = XUI_POPUP_ESCAPE_CLOSE;
	tDesc.iFocusPolicy = XUI_POPUP_FOCUS_POPUP;
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	tDesc.fScrollbarSize = 8.0f;
	iRet = xuiPopupCreate(pContext, &pPopup, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pPopup != NULL, "popup create");
	iRet = xuiPopupSetChange(pPopup, __xuiPopupChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "change callback");

	pPanel = xuiPopupGetPanelWidget(pPopup);
	pScrollView = xuiPopupGetScrollViewWidget(pPopup);
	pFrame = xuiPopupGetFrameWidget(pPopup);
	pContent = xuiPopupGetContentWidget(pPopup);
	XUI_TEST_CHECK(pPanel != NULL && pScrollView != NULL && pFrame != NULL && pContent != NULL, "popup internals");
	XUI_TEST_CHECK(xuiWidgetGetParent(pScrollView) == pPanel && xuiWidgetGetParent(pContent) == xuiPopupGetViewportWidget(pPopup), "popup tree");

	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "content child create");
	xuiWidgetSetRect(pChild, (xui_rect_t){8.0f, 210.0f, 80.0f, 30.0f});
	iRet = xuiWidgetAddChild(pContent, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "add popup child");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 360, 260, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial layout");
	iRet = xuiSetFocusWidget(pContext, pOwner);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pOwner, "focus owner");
	iRet = xuiPopupSetOpen(pPopup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPopupIsOpen(pPopup), "open popup");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "open layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiPopupRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");

	tOwner = xuiWidgetGetWorldRect(pOwner);
	tPopup = xuiPopupGetPopupRect(pPopup);
	XUI_TEST_CHECK(__xuiPopupNear(tPopup.fX, tOwner.fX) && __xuiPopupNear(tPopup.fY, tOwner.fY + tOwner.fH + 5.0f), "anchored below owner");
	XUI_TEST_CHECK(tPopup.fW > 120.0f && tPopup.fW < 150.0f && __xuiPopupNear(tPopup.fH, 140.0f), "outer size with vertical reserve");
	tViewport = xuiPopupGetViewportRect(pPopup);
	XUI_TEST_CHECK(__xuiPopupNear(tViewport.fW, 120.0f), "viewport width preserved");
	XUI_TEST_CHECK(xuiScrollFrameIsVScrollBarVisible(pFrame), "vertical bar visible");
	XUI_TEST_CHECK(!xuiScrollFrameIsHScrollBarVisible(pFrame), "horizontal bar not chained");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pPopup, "popup focused");

	iRet = xuiInputPointerWheel(pContext, tPopup.fX + 20.0f, tPopup.fY + 20.0f, 0.0f, -1.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel dispatch");
	iRet = xuiPopupGetScroll(pPopup, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiPopupNear(fOffsetX, 0.0f) && fOffsetY > 0.0f, "popup scroll");

	iRet = __xuiPopupDispatchDown(pContext, 320.0f, 220.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiPopupIsOpen(pPopup), "outside closes popup");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pOwner, "focus restored");
	XUI_TEST_CHECK(iChanged >= 2, "change callback open close");

	iRet = xuiPopupSetOpen(pPopup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPopupIsOpen(pPopup), "reopen popup");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiPopupIsOpen(pPopup), "escape closes popup");

	iRet = xuiPopupSetClosePolicy(pPopup, XUI_POPUP_OUTSIDE_IGNORE, XUI_POPUP_OWNER_CLOSE, XUI_POPUP_ESCAPE_CLOSE);
	XUI_TEST_CHECK(iRet == XUI_OK, "owner close policy");
	iRet = xuiPopupSetOpen(pPopup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPopupIsOpen(pPopup), "reopen owner policy");
	tOwner = xuiWidgetGetWorldRect(pOwner);
	iRet = __xuiPopupDispatchDown(pContext, tOwner.fX + 5.0f, tOwner.fY + 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiPopupIsOpen(pPopup), "owner closes popup");

	iRet = xuiPopupSetClosePolicy(pPopup, XUI_POPUP_OUTSIDE_CLOSE, XUI_POPUP_OWNER_PASSTHROUGH, XUI_POPUP_ESCAPE_CLOSE);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore close policy");
	iRet = xuiPopupSetContentSize(pPopup, 100.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "small content size");
	iRet = xuiPopupSetMaxSize(pPopup, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear max size");
	iRet = xuiPopupSetAnchorRect(pPopup, (xui_rect_t){50.0f, 220.0f, 80.0f, 30.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "fallback anchor");
	iRet = xuiPopupSetDirection(pPopup, XUI_POPUP_DIRECTION_RIGHT_DOWN);
	XUI_TEST_CHECK(iRet == XUI_OK, "fallback direction");
	iRet = xuiPopupSetOpen(pPopup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPopupIsOpen(pPopup), "open fallback popup");
	tPopup = xuiPopupGetPopupRect(pPopup);
	XUI_TEST_CHECK(tPopup.fY < 220.0f, "vertical fallback");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_popup_test passed\n");
	return 0;
}
