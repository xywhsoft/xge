#include "xui.h"
#include "xui_test_proxy.h"

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

static int __xuiMsgBoxNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) fD = -fD;
	return fD < 0.05f;
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

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_msgbox_test_result_t tResult;
	xui_context pContext;
	xui_widget pRoot;
	xui_msgbox pBox;
	xui_surface pTarget;
	xui_surface pContentCache;
	xui_font pFont;
	xui_msgbox_desc_t tDesc;
	xui_msgbox_button_t arrCustom[2];
	xui_rect_t tIcon;
	xui_rect_t tMessage;
	xui_rect_t tBackdrop;
	xui_rect_t tButton;
	xui_rect_t tWorld;
	xui_rect_t tBeforeDrag;
	xui_rect_t tAfterDrag;
	xui_rect_t tAfterUpdate;
	int iLayer;
	int iZ;
	int iWindowZ;
	int iFailed;
	int iRet;

	memset(&tResult, 0, sizeof(tResult));
	pContext = NULL;
	pRoot = NULL;
	pBox = NULL;
	pTarget = NULL;
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

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Confirm";
	tDesc.sMessage = "This message is intentionally long enough to exercise automatic wrapping while keeping the dialog width inside the root viewport.";
	tDesc.pFont = pFont;
	tDesc.iType = XUI_MSGBOX_ICON_INFO;
	tDesc.iButtons = XUI_MSGBOX_BUTTON_OK_CANCEL;
	tDesc.bHasModal = 1;
	tDesc.bModal = 1;
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
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tBackdrop = xuiMsgBoxGetBackdropRect(pBox);
	XUI_TEST_CHECK(__xuiMsgBoxNear(tBackdrop.fW, 640.0f) && __xuiMsgBoxNear(tBackdrop.fH, 420.0f), "backdrop root size");
	iRet = xuiWidgetGetLayer(xuiMsgBoxGetWindowWidget(pBox), &iLayer, &iZ);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayer == XUI_LAYER_MODAL && iZ >= 1, "window modal layer");
	iWindowZ = iZ;
	iRet = xuiWidgetGetLayer(xuiMsgBoxGetContentWidget(pBox), &iLayer, &iZ);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayer == XUI_LAYER_MODAL && iZ == iWindowZ, "content modal layer");
	iRet = xuiWidgetGetLayer(xuiMsgBoxGetButtonWidget(pBox, 0), &iLayer, &iZ);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayer == XUI_LAYER_MODAL && iZ == iWindowZ, "button modal layer");
	iRet = xuiWidgetGetLayer(xuiMsgBoxGetBackdropWidget(pBox), &iLayer, &iZ);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayer == XUI_LAYER_MODAL && iZ == 0, "backdrop modal layer");
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
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgBoxIsOpen(pBox), "reopen for cancel button");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout cancel");
	tButton = xuiMsgBoxGetButtonRect(pBox, 1);
	tWorld = xuiWidgetGetWorldRect(xuiMsgBoxGetContentWidget(pBox));
	iRet = __xuiMsgBoxDispatchClick(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "cancel button click");
	XUI_TEST_CHECK(!xuiMsgBoxIsOpen(pBox) && xuiMsgBoxGetResult(pBox) == XUI_MSGBOX_RESULT_CANCEL, "cancel closes");
	XUI_TEST_CHECK(tResult.iCount == 2 && tResult.iLastResult == XUI_MSGBOX_RESULT_CANCEL, "cancel result callback");

	arrCustom[0].sText = "Retry";
	arrCustom[0].iResult = 10;
	arrCustom[0].iSemantic = XUI_BUTTON_SEMANTIC_PRIMARY;
	arrCustom[1].sText = "Delete";
	arrCustom[1].iResult = 20;
	arrCustom[1].iSemantic = XUI_BUTTON_SEMANTIC_DANGER;
	iRet = xuiMsgBoxSetType(pBox, XUI_MSGBOX_ICON_ERROR);
	XUI_TEST_CHECK(iRet == XUI_OK, "set type error");
	iRet = xuiMsgBoxSetCustomButtons(pBox, arrCustom, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgBoxGetButtonCount(pBox) == 2, "custom buttons");
	iRet = xuiMsgBoxSetOpen(pBox, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMsgBoxIsOpen(pBox), "reopen custom");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout custom");
	tButton = xuiMsgBoxGetButtonRect(pBox, 1);
	tWorld = xuiWidgetGetWorldRect(xuiMsgBoxGetContentWidget(pBox));
	iRet = __xuiMsgBoxDispatchClick(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "custom click");
	XUI_TEST_CHECK(!xuiMsgBoxIsOpen(pBox) && xuiMsgBoxGetResult(pBox) == 20, "custom result");
	XUI_TEST_CHECK(tResult.iCount == 3 && tResult.iLastResult == 20, "custom result callback");

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
	XUI_TEST_CHECK(tResult.iCount == 4 && tResult.iLastResult == XUI_MSGBOX_RESULT_CLOSE, "escape result callback");
	XUI_TEST_CHECK(xuiMsgBoxGetResultCount(pBox) == 4, "result count");
	XUI_TEST_CHECK(xuiMsgBoxGetChangeCount(pBox) > 0, "change count");

cleanup:
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
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
