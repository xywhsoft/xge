#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_msgbox_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_msgbox_test_result_t {
	int iCount;
	int iLastResult;
} xui_msgbox_test_result_t;

typedef struct xui_msgbox_destroy_result_t {
	xui_msgbox* ppBox;
	int iCount;
} xui_msgbox_destroy_result_t;

static int __xuiMsgBoxNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) fD = -fD;
	return fD < 0.05f;
}

static int __xuiMsgBoxRectNear(xui_rect_t tA, xui_rect_t tB)
{
	return __xuiMsgBoxNear(tA.fX, tB.fX) &&
		__xuiMsgBoxNear(tA.fY, tB.fY) &&
		__xuiMsgBoxNear(tA.fW, tB.fW) &&
		__xuiMsgBoxNear(tA.fH, tB.fH);
}

static int __xuiMsgBoxDispatchMove(xui_context pContext, float fX, float fY, uint32_t iButtons)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fX, fY, iButtons);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiMsgBoxDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiMsgBoxDispatchUp(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiMsgBoxDispatchClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = __xuiMsgBoxDispatchMove(pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMsgBoxDispatchDown(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiMsgBoxDispatchUp(pContext, fX, fY);
}

static int __xuiMsgBoxButtonUpdateOomPreserves(xui_msgbox pBox)
{
	xui_msgbox_button_t arrReplacement[2];
	static const size_t arrBudgets[] = {
		1u, 64u, 128u, 256u, 512u, 1024u, 2048u, 4096u,
		8192u, 16384u, 32768u, 65536u, 131072u, 262144u, 1048576u
	};
	xallocationbudget* pBudget;
	xallocationscope tScope;
	xui_rect_t tWindowBefore;
	xui_rect_t tButtonBefore[2];
	xui_widget pButton0;
	xui_widget pButton1;
	int iChangeBefore;
	int iBudget;
	int iRet;
	int bSawFailure;
	int bSawSuccess;

	if ( pBox == NULL ) return 0;
	arrReplacement[0] = (xui_msgbox_button_t){
		"Reconnect after refreshing the account credentials", 101,
		XUI_BUTTON_SEMANTIC_PRIMARY};
	arrReplacement[1] = (xui_msgbox_button_t){
		"Discard", 102, XUI_BUTTON_SEMANTIC_DANGER};
	bSawFailure = 0;
	bSawSuccess = 0;
	for ( iBudget = 0; iBudget < (int)(sizeof(arrBudgets) / sizeof(arrBudgets[0])); iBudget++ ) {
		if ( xuiMsgBoxSetButtons(pBox, XUI_MSGBOX_BUTTON_OK_CANCEL) != XUI_OK ) return 0;
		pButton0 = xuiMsgBoxGetButtonWidget(pBox, 0);
		pButton1 = xuiMsgBoxGetButtonWidget(pBox, 1);
		if ( pButton0 == NULL || pButton1 == NULL ) return 0;
		tWindowBefore = xuiWidgetGetRect(xuiMsgBoxGetWindowWidget(pBox));
		tButtonBefore[0] = xuiMsgBoxGetButtonRect(pBox, 0);
		tButtonBefore[1] = xuiMsgBoxGetButtonRect(pBox, 1);
		iChangeBefore = xuiMsgBoxGetChangeCount(pBox);
		pBudget = NULL;
		memset(&tScope, 0, sizeof(tScope));
		if ( !xrtAllocationBudgetCreate(arrBudgets[iBudget], &pBudget) ) return 0;
		if ( !xrtAllocationBudgetEnter(pBudget, &tScope) ) {
			xrtAllocationBudgetFree(pBudget);
			return 0;
		}
		iRet = xuiMsgBoxSetCustomButtons(pBox, arrReplacement, 2);
		(void)xrtAllocationBudgetLeave(&tScope);
		xrtAllocationBudgetFree(pBudget);
		if ( iRet == XUI_ERROR_OUT_OF_MEMORY ) {
			bSawFailure = 1;
			if ( xuiMsgBoxGetButtonCount(pBox) != 2 ||
			     strcmp(xuiButtonGetText(pButton0), "OK") != 0 ||
			     strcmp(xuiButtonGetText(pButton1), "Cancel") != 0 ||
			     xuiButtonGetSemantic(pButton0) != XUI_BUTTON_SEMANTIC_PRIMARY ||
			     xuiButtonGetSemantic(pButton1) != XUI_BUTTON_SEMANTIC_DEFAULT ||
			     xuiMsgBoxGetChangeCount(pBox) != iChangeBefore ||
			     !__xuiMsgBoxRectNear(
				     xuiWidgetGetRect(xuiMsgBoxGetWindowWidget(pBox)), tWindowBefore) ||
			     !__xuiMsgBoxRectNear(xuiMsgBoxGetButtonRect(pBox, 0), tButtonBefore[0]) ||
			     !__xuiMsgBoxRectNear(xuiMsgBoxGetButtonRect(pBox, 1), tButtonBefore[1]) ) {
				return 0;
			}
			continue;
		}
		if ( iRet != XUI_OK ) return 0;
		bSawSuccess = 1;
		if ( xuiMsgBoxSetButtons(pBox, XUI_MSGBOX_BUTTON_OK_CANCEL) != XUI_OK ) return 0;
		break;
	}
	return bSawFailure && bSawSuccess;
}

static void __xuiMsgBoxResult(xui_msgbox pBox, int iResult, void* pUser)
{
	xui_msgbox_test_result_t* pResult;

	(void)pBox;
	pResult = (xui_msgbox_test_result_t*)pUser;
	if ( pResult != NULL ) {
		pResult->iCount++;
		pResult->iLastResult = iResult;
	}
}

static void __xuiMsgBoxDestroyResult(xui_msgbox pBox, int iResult, void* pUser)
{
	xui_msgbox_destroy_result_t* pResult = (xui_msgbox_destroy_result_t*)pUser;
	(void)iResult;
	if ( pResult == NULL ) return;
	pResult->iCount++;
	xuiMsgBoxDestroy(pBox);
	if ( pResult->ppBox != NULL ) *pResult->ppBox = NULL;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_msgbox_test_result_t tResult;
	xui_msgbox_destroy_result_t tDestroyResult;
	xui_context pContext;
	xui_widget pRoot;
	xui_msgbox pBox;
	xui_msgbox pInvalidBox;
	xui_surface pCustomIcon;
	xui_surface pContentCache;
	xui_font pFont;
	xui_msgbox_desc_t tDesc;
	xui_msgbox_desc_t tInvalidDesc;
	xui_msgbox_button_t arrCustom[2];
	xui_rect_t tIcon;
	xui_rect_t tMessage;
	xui_rect_t tBackdrop;
	xui_rect_t tButton;
	xui_rect_t tTitle;
	xui_rect_t tWorld;
	xui_rect_t tBeforeDrag;
	xui_rect_t tAfterDrag;
	xui_rect_t tAfterUpdate;
	xui_rect_t tLongButton;
	xui_rect_t tShortButton;
	xui_rect_t tSmallWindow;
	xui_rect_t tSmallContent;
	xui_vec2_t tButtonContent;
	xui_thickness_t tButtonPadding;
	xui_msgbox_metrics_t tMetrics;
	xui_msgbox_metrics_t tInvalidMetrics;
	int iLayer;
	int iZ;
	int iWindowZ;
	int iChangeBefore;
	int iFailed;
	int iRet;

	memset(&tResult, 0, sizeof(tResult));
	memset(&tDestroyResult, 0, sizeof(tDestroyResult));
	pContext = NULL;
	pRoot = NULL;
	pBox = NULL;
	pCustomIcon = NULL;
	pFont = NULL;
	pContentCache = NULL;
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
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	(void)xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 640.0f, 420.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");
	iRet = xuiTestSurfaceCreate(&tState, &pCustomIcon, 16, 16,
		XUI_SURFACE_ALPHA_PREMULTIPLIED);
	XUI_TEST_CHECK(iRet == XUI_OK && pCustomIcon != NULL, "custom icon create");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Confirm";
	tDesc.sMessage = "This message is intentionally long enough to exercise automatic wrapping while keeping the dialog width inside the root viewport.";
	tDesc.pFont = pFont;
	tDesc.iType = XUI_MSGBOX_ICON_INFO;
	tDesc.iButtons = XUI_MSGBOX_BUTTON_OK_CANCEL;
	tDesc.bHasModal = 1;
	tDesc.bModal = 1;
	tInvalidDesc = tDesc;
	tInvalidDesc.tIconSrc.fW = -1;
	pInvalidBox = (xui_msgbox)&tState;
	iRet = xuiMsgBoxCreate(pContext, &pInvalidBox, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidBox == NULL, "invalid create clears output");
	tInvalidDesc = tDesc;
	tInvalidDesc.tIconSrc.fX = INFINITY;
	pInvalidBox = (xui_msgbox)&tState;
	iRet = xuiMsgBoxCreate(pContext, &pInvalidBox, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidBox == NULL,
		"nonfinite icon source is rejected at creation");
	iRet = xuiMsgBoxCreate(pContext, &pBox, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pBox != NULL, "msgbox create");
	iRet = xuiMsgBoxSetResult(pBox, __xuiMsgBoxResult, &tResult);
	XUI_TEST_CHECK(iRet == XUI_OK, "result callback set");
	XUI_TEST_CHECK(strcmp(xuiMsgBoxGetTitle(pBox), "Confirm") == 0, "title getter");
	XUI_TEST_CHECK(xuiMsgBoxGetType(pBox) == XUI_MSGBOX_ICON_INFO, "type getter");
	XUI_TEST_CHECK(xuiMsgBoxGetButtonCount(pBox) == 2, "button count");
	XUI_TEST_CHECK(xuiMsgBoxGetWindowWidget(pBox) != NULL, "window getter");
	XUI_TEST_CHECK(xuiMsgBoxGetContentWidget(pBox) != NULL, "content getter");
	XUI_TEST_CHECK(xuiMsgBoxGetBackdropWidget(pBox) != NULL, "backdrop getter");

	iRet = xuiMsgBoxSetOpen(pBox, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgBoxIsOpen(pBox), "open");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == xuiMsgBoxGetButtonWidget(pBox, 0), "default button receives focus");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tBackdrop = xuiMsgBoxGetBackdropRect(pBox);
	XUI_TEST_CHECK(__xuiMsgBoxNear(tBackdrop.fW, 640.0f) && __xuiMsgBoxNear(tBackdrop.fH, 420.0f), "backdrop root size");
	tTitle = xuiWindowGetTitleBarRect(xuiMsgBoxGetWindowWidget(pBox));
	tButton = xuiWindowGetCloseButtonRect(xuiMsgBoxGetWindowWidget(pBox));
	XUI_TEST_CHECK(tButton.fW <= 20.05f && tButton.fH <= 20.05f, "msgbox close button compact");
	XUI_TEST_CHECK(tButton.fY > tTitle.fY + 2.0f && tButton.fY + tButton.fH < tTitle.fY + tTitle.fH - 2.0f, "msgbox close button inset");
	iRet = xuiWidgetGetLayer(xuiMsgBoxGetWindowWidget(pBox), &iLayer, &iZ);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayer == XUI_LAYER_MODAL && iZ >= 1, "window modal layer");
	iWindowZ = iZ;
	iRet = xuiWidgetGetLayer(xuiMsgBoxGetContentWidget(pBox), &iLayer, &iZ);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayer == XUI_LAYER_MODAL && iZ == iWindowZ, "content modal layer");
	iRet = xuiWidgetGetLayer(xuiMsgBoxGetButtonWidget(pBox, 0), &iLayer, &iZ);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayer == XUI_LAYER_MODAL && iZ == iWindowZ, "button modal layer");
	iRet = xuiWidgetGetLayer(xuiMsgBoxGetBackdropWidget(pBox), &iLayer, &iZ);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayer == XUI_LAYER_MODAL && iZ == 0, "backdrop modal layer");
	iRet = xuiMsgBoxSetModal(pBox, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMsgBoxIsModal(pBox), "disable modal while open");
	iRet = xuiWidgetGetLayer(xuiMsgBoxGetWindowWidget(pBox), &iLayer, &iZ);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayer == XUI_LAYER_FLOATING && iZ == XUI_WINDOW_Z_TOPMOST,
		"nonmodal msgbox restores floating layer");
	XUI_TEST_CHECK(xuiWidgetGetParent(xuiMsgBoxGetBackdropWidget(pBox)) == NULL &&
		!xuiWidgetGetVisible(xuiMsgBoxGetBackdropWidget(pBox)), "nonmodal msgbox detaches backdrop");
	iRet = xuiMsgBoxSetModal(pBox, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgBoxIsModal(pBox), "restore modal while open");
	iRet = xuiWidgetGetLayer(xuiMsgBoxGetWindowWidget(pBox), &iLayer, &iZ);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayer == XUI_LAYER_MODAL, "restored msgbox uses modal layer");

	iRet = xuiMsgBoxGetMetrics(pBox, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK, "get metrics for validation");
	tInvalidMetrics = tMetrics;
	tInvalidMetrics.fMinWidth = tMetrics.fMaxWidth + 1.0f;
	iRet = xuiMsgBoxSetMetrics(pBox, &tInvalidMetrics);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject inverted width metrics");
	tInvalidMetrics = tMetrics;
	tInvalidMetrics.fFooterHeight = tMetrics.fButtonHeight - 1.0f;
	iRet = xuiMsgBoxSetMetrics(pBox, &tInvalidMetrics);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject footer shorter than button");
	iRet = xuiMsgBoxSetIconSurface(pBox, NULL, (xui_rect_t){0, 0, -1, 1});
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject negative icon source");
	iRet = xuiMsgBoxSetIconSurface(pBox, NULL, (xui_rect_t){NAN, 0, 1, 1});
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nonfinite icon source");
	iRet = xuiMsgBoxSetIconSurface(pBox, (xui_surface)&tState,
		(xui_rect_t){0, 0, 0, 16});
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT,
		"custom icon requires nonempty source");
	iChangeBefore = xuiMsgBoxGetChangeCount(pBox);
	iRet = xuiMsgBoxSetTitle(pBox, xuiMsgBoxGetTitle(pBox));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgBoxGetChangeCount(pBox) == iChangeBefore, "same title is no-op");
	tIcon = xuiMsgBoxGetIconRect(pBox);
	tMessage = xuiMsgBoxGetMessageRect(pBox);
	XUI_TEST_CHECK(tIcon.fW > 0.0f && tIcon.fH > 0.0f, "icon rect");
	XUI_TEST_CHECK(tMessage.fW > 80.0f && tMessage.fH > 20.0f, "message rect");
	XUI_TEST_CHECK(xuiMsgBoxGetWrapLineCount(pBox) >= 1, "wrap line count");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");
	pContentCache = xuiWidgetGetCacheSurface(xuiMsgBoxGetContentWidget(pBox), xuiWidgetGetStateId(xuiMsgBoxGetContentWidget(pBox)));
	XUI_TEST_CHECK(pContentCache != NULL, "content cache");
	XUI_TEST_CHECK(xuiTestSurfaceGetDrawCount(pContentCache) > 0, "builtin icon rendered");
	XUI_TEST_CHECK(xuiTestSurfaceGetTextDrawCount(pContentCache) > 0, "message rendered");
	iRet = xuiMsgBoxSetIconSurface(pBox, pCustomIcon,
		(xui_rect_t){0.0f, 0.0f, 16.0f, 16.0f});
	if ( iRet == XUI_OK ) iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "custom icon render");
	xuiTestSurfaceReset(pContentCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pContentCache) == 0,
		"unchanged borrowed msgbox icon reuses cache");
	iRet = tState.tProxy.surfaceUpdateRGBA(&tState.tProxy, pCustomIcon,
		(xui_rect_i_t){0, 0, 1, 1}, NULL, 0);
	if ( iRet == XUI_OK ) iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pContentCache) > 0,
		"borrowed msgbox icon generation rebuilds cache");
	iRet = xuiMsgBoxUseBuiltinIcon(pBox);
	if ( iRet == XUI_OK ) iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore builtin icon");
	xuiTestProxyResetTextCounters(&tState);
	iRet = xuiWidgetInvalidate(xuiMsgBoxGetContentWidget(pBox), XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "invalidate message content");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iLastTextDrawSize > 0, "message lines use bounded text lengths");

	iRet = xuiInputViewport(pContext, 760.0f, 430.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "resize viewport");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 760.0f, 430.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "resize root");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update resized msgbox");
	tBackdrop = xuiMsgBoxGetBackdropRect(pBox);
	XUI_TEST_CHECK(__xuiMsgBoxNear(tBackdrop.fW, 760.0f) && __xuiMsgBoxNear(tBackdrop.fH, 430.0f), "backdrop follows resized root");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout resized msgbox");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare resized msgbox");
	pContentCache = xuiWidgetGetCacheSurface(xuiMsgBoxGetContentWidget(pBox), xuiWidgetGetStateId(xuiMsgBoxGetContentWidget(pBox)));
	XUI_TEST_CHECK(pContentCache != NULL, "content cache resized");
	XUI_TEST_CHECK(xuiTestSurfaceGetTextDrawCount(pContentCache) > 0, "message rendered resized");

	iRet = __xuiMsgBoxDispatchClick(pContext, 8.0f, 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgBoxIsOpen(pBox), "modal backdrop consumes outside click");
	XUI_TEST_CHECK(tResult.iCount == 0, "outside click has no result");

	tWorld = xuiWidgetGetWorldRect(xuiMsgBoxGetWindowWidget(pBox));
	tBeforeDrag = xuiWidgetGetRect(xuiMsgBoxGetWindowWidget(pBox));
	iRet = __xuiMsgBoxDispatchDown(pContext, tWorld.fX + 36.0f, tWorld.fY + 15.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "msgbox title drag down");
	iRet = __xuiMsgBoxDispatchMove(pContext, tWorld.fX + 74.0f, tWorld.fY + 37.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "msgbox title drag move");
	iRet = __xuiMsgBoxDispatchUp(pContext, tWorld.fX + 74.0f, tWorld.fY + 37.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "msgbox title drag up");
	tAfterDrag = xuiWidgetGetRect(xuiMsgBoxGetWindowWidget(pBox));
	XUI_TEST_CHECK(tAfterDrag.fX > tBeforeDrag.fX + 20.0f && tAfterDrag.fY > tBeforeDrag.fY + 10.0f, "msgbox title drag moves window");
	XUI_TEST_CHECK(xuiMsgBoxIsOpen(pBox), "drag keeps msgbox open");
	iRet = xuiWidgetGetLayer(xuiMsgBoxGetWindowWidget(pBox), &iLayer, &iZ);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayer == XUI_LAYER_MODAL && iZ >= 1, "msgbox stays modal after drag");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update after drag");
	tAfterUpdate = xuiWidgetGetRect(xuiMsgBoxGetWindowWidget(pBox));
	XUI_TEST_CHECK(__xuiMsgBoxNear(tAfterUpdate.fX, tAfterDrag.fX) && __xuiMsgBoxNear(tAfterUpdate.fY, tAfterDrag.fY), "dragged msgbox position persists");

	iRet = xuiSetFocusWidget(pContext, xuiMsgBoxGetWindowWidget(pBox));
	XUI_TEST_CHECK(iRet == XUI_OK, "focus window");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ENTER, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "enter down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "enter dispatch");
	XUI_TEST_CHECK(!xuiMsgBoxIsOpen(pBox) && xuiMsgBoxGetResult(pBox) == XUI_MSGBOX_RESULT_OK, "enter closes OK");
	XUI_TEST_CHECK(tResult.iCount == 1 && tResult.iLastResult == XUI_MSGBOX_RESULT_OK, "enter result callback");

	iRet = xuiMsgBoxSetOpen(pBox, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "reopen for escape cancel");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape cancel down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape cancel dispatch");
	XUI_TEST_CHECK(!xuiMsgBoxIsOpen(pBox) && xuiMsgBoxGetResult(pBox) == XUI_MSGBOX_RESULT_CANCEL,
		"escape selects available cancel result");
	XUI_TEST_CHECK(tResult.iCount == 2 && tResult.iLastResult == XUI_MSGBOX_RESULT_CANCEL,
		"escape cancel callback");

	iRet = xuiMsgBoxSetOpen(pBox, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgBoxIsOpen(pBox), "reopen for cancel button");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout cancel");
	tButton = xuiMsgBoxGetButtonRect(pBox, 1);
	tWorld = xuiWidgetGetWorldRect(xuiMsgBoxGetContentWidget(pBox));
	iRet = __xuiMsgBoxDispatchClick(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "cancel button click");
	XUI_TEST_CHECK(!xuiMsgBoxIsOpen(pBox) && xuiMsgBoxGetResult(pBox) == XUI_MSGBOX_RESULT_CANCEL, "cancel closes");
	XUI_TEST_CHECK(tResult.iCount == 3 && tResult.iLastResult == XUI_MSGBOX_RESULT_CANCEL, "cancel result callback");
	XUI_TEST_CHECK(__xuiMsgBoxButtonUpdateOomPreserves(pBox),
		"button update allocation failures preserve the complete model and geometry");

	arrCustom[0].sText = "Retry after reconnecting to the service";
	arrCustom[0].iResult = 10;
	arrCustom[0].iSemantic = XUI_BUTTON_SEMANTIC_PRIMARY;
	arrCustom[1].sText = "Delete";
	arrCustom[1].iResult = 20;
	arrCustom[1].iSemantic = XUI_BUTTON_SEMANTIC_DANGER;
	arrCustom[1].iSemantic = 999;
	iRet = xuiMsgBoxSetCustomButtons(pBox, arrCustom, 2);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiMsgBoxGetButtonCount(pBox) == 2,
		"invalid custom semantic preserves preset buttons");
	arrCustom[1].iSemantic = XUI_BUTTON_SEMANTIC_DANGER;
	iRet = xuiMsgBoxSetType(pBox, XUI_MSGBOX_ICON_ERROR);
	XUI_TEST_CHECK(iRet == XUI_OK, "set type error");
	iRet = xuiMsgBoxSetCustomButtons(pBox, arrCustom, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgBoxGetButtonCount(pBox) == 2, "custom buttons");
	iRet = xuiMsgBoxSetOpen(pBox, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgBoxIsOpen(pBox), "reopen custom");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout custom");
	tLongButton = xuiMsgBoxGetButtonRect(pBox, 0);
	tShortButton = xuiMsgBoxGetButtonRect(pBox, 1);
	iRet = xuiMsgBoxGetMetrics(pBox, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK, "get msgbox metrics");
	iRet = xuiWidgetMeasureContent(xuiMsgBoxGetButtonWidget(pBox, 0),
		(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tButtonContent);
	XUI_TEST_CHECK(iRet == XUI_OK, "measure long button content");
	tButtonPadding = xuiWidgetGetPadding(xuiMsgBoxGetButtonWidget(pBox, 0));
	XUI_TEST_CHECK(tLongButton.fW >= tButtonContent.fX + tButtonPadding.fLeft + tButtonPadding.fRight,
		"long button fits measured text and padding");
	XUI_TEST_CHECK(tLongButton.fW > tMetrics.fButtonWidth, "long button expands past minimum width");
	XUI_TEST_CHECK(__xuiMsgBoxNear(tShortButton.fW, tMetrics.fButtonWidth), "short button keeps minimum width");
	XUI_TEST_CHECK(tShortButton.fX >= tLongButton.fX + tLongButton.fW + tMetrics.fButtonGap,
		"variable-width buttons preserve gap");
	iRet = xuiInputViewport(pContext, 240.0f, 180.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "small viewport");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0, 0, 240, 180});
	XUI_TEST_CHECK(iRet == XUI_OK, "small root");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "small msgbox update");
	tSmallWindow = xuiWidgetGetRect(xuiMsgBoxGetWindowWidget(pBox));
	tSmallContent = xuiWidgetGetRect(xuiMsgBoxGetContentWidget(pBox));
	tLongButton = xuiMsgBoxGetButtonRect(pBox, 0);
	tShortButton = xuiMsgBoxGetButtonRect(pBox, 1);
	XUI_TEST_CHECK(tSmallWindow.fX >= 0 && tSmallWindow.fY >= 0 &&
		tSmallWindow.fX + tSmallWindow.fW <= 240 &&
		tSmallWindow.fY + tSmallWindow.fH <= 180,
		"small msgbox remains inside root");
	XUI_TEST_CHECK(tLongButton.fX >= 0 && tLongButton.fY >= 0 &&
		tShortButton.fX >= tLongButton.fX + tLongButton.fW &&
		tShortButton.fX + tShortButton.fW <= tSmallContent.fW &&
		tShortButton.fY + tShortButton.fH <= tSmallContent.fH,
		"small msgbox buttons remain inside content");
	iRet = xuiInputViewport(pContext, 16.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tiny viewport");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0, 0, 16, 80});
	XUI_TEST_CHECK(iRet == XUI_OK, "tiny root");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tiny msgbox update");
	tSmallWindow = xuiWidgetGetRect(xuiMsgBoxGetWindowWidget(pBox));
	tSmallContent = xuiWidgetGetRect(xuiMsgBoxGetContentWidget(pBox));
	tLongButton = xuiMsgBoxGetButtonRect(pBox, 0);
	tShortButton = xuiMsgBoxGetButtonRect(pBox, 1);
	XUI_TEST_CHECK(tSmallWindow.fX >= 0 && tSmallWindow.fY >= 0 &&
		tSmallWindow.fX + tSmallWindow.fW <= 16 &&
		tSmallWindow.fY + tSmallWindow.fH <= 80,
		"tiny msgbox remains inside root");
	XUI_TEST_CHECK(tLongButton.fX >= 0 && tLongButton.fX <= tSmallContent.fW &&
		tShortButton.fX >= tLongButton.fX + tLongButton.fW &&
		tShortButton.fX + tShortButton.fW <= tSmallContent.fW,
		"tiny msgbox button geometry remains bounded");
	iRet = xuiInputViewport(pContext, 760.0f, 430.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore viewport after small layout");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0, 0, 760, 430});
	XUI_TEST_CHECK(iRet == XUI_OK, "restore root after small layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore msgbox layout");
	tButton = xuiMsgBoxGetButtonRect(pBox, 1);
	tWorld = xuiWidgetGetWorldRect(xuiMsgBoxGetContentWidget(pBox));
	iRet = __xuiMsgBoxDispatchClick(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "custom click");
	XUI_TEST_CHECK(!xuiMsgBoxIsOpen(pBox) && xuiMsgBoxGetResult(pBox) == 20, "custom result");
	XUI_TEST_CHECK(tResult.iCount == 4 && tResult.iLastResult == 20, "custom result callback");

	iRet = xuiMsgBoxSetButtons(pBox, XUI_MSGBOX_BUTTON_OK);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgBoxGetButtonCount(pBox) == 1, "reset buttons");
	iRet = xuiMsgBoxSetOpen(pBox, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "reopen escape");
	iRet = xuiSetFocusWidget(pContext, xuiMsgBoxGetWindowWidget(pBox));
	XUI_TEST_CHECK(iRet == XUI_OK, "focus escape");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape dispatch");
	XUI_TEST_CHECK(!xuiMsgBoxIsOpen(pBox) && xuiMsgBoxGetResult(pBox) == XUI_MSGBOX_RESULT_CLOSE, "escape closes");
	XUI_TEST_CHECK(tResult.iCount == 5 && tResult.iLastResult == XUI_MSGBOX_RESULT_CLOSE, "escape result callback");
	XUI_TEST_CHECK(xuiMsgBoxGetResultCount(pBox) == 5, "result count");
	XUI_TEST_CHECK(xuiMsgBoxGetChangeCount(pBox) > 0, "change count");

	tDestroyResult.ppBox = &pBox;
	iRet = xuiMsgBoxSetResult(pBox, __xuiMsgBoxDestroyResult, &tDestroyResult);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroying result callback set");
	iRet = xuiMsgBoxSetOpen(pBox, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "open for destroying callback");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ENTER, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroying callback enter down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pBox == NULL && tDestroyResult.iCount == 1,
		"result callback may destroy msgbox");

cleanup:
	if ( pCustomIcon != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pCustomIcon);
	}
	if ( pBox != NULL ) {
		xuiMsgBoxDestroy(pBox);
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
	printf("xui_msgbox_test passed\n");
	return 0;
}
