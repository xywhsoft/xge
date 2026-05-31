#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_numeric_input_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_numeric_input_change_t {
	int iCount;
	float fValue;
	int iErrorCount;
	int bError;
} xui_numeric_input_change_t;

static int __xuiNumericInputNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) fD = -fD;
	return fD < 0.02f;
}

static void __xuiNumericInputChanged(xui_widget pWidget, float fValue, void* pUser)
{
	xui_numeric_input_change_t* pChange;

	(void)pWidget;
	pChange = (xui_numeric_input_change_t*)pUser;
	if ( pChange == NULL ) return;
	pChange->iCount++;
	pChange->fValue = fValue;
}

static void __xuiNumericInputErrorChanged(xui_widget pWidget, int bError, void* pUser)
{
	xui_numeric_input_change_t* pChange;

	(void)pWidget;
	pChange = (xui_numeric_input_change_t*)pUser;
	if ( pChange == NULL ) return;
	pChange->iErrorCount++;
	pChange->bError = bError;
}

static int __xuiNumericInputRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 360, 220};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiNumericInputClick(xui_context pContext, float fX, float fY)
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

static int __xuiNumericInputDispatchKey(xui_context pContext, int iKey, uint32_t iModifiers)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_numeric_input_change_t tChange;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pNumeric;
	xui_widget pInput;
	xui_widget pMenu;
	xui_surface pTarget;
	xui_font pFont;
	xui_numeric_input_desc_t tDesc;
	xui_rect_t tWorld;
	xui_rect_t tButton;
	xui_rect_t tSpinner;
	float fMin;
	float fMax;
	float fBefore;
	int iStartChange;
	int iSelStart;
	int iSelEnd;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pNumeric = NULL;
	pInput = NULL;
	pMenu = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tChange, 0, sizeof(tChange));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadFile(&tState.tProxy, &pFont, "test.ttf", 14.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 360.0f, 220.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 360.0f, 220.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sPlaceholder = "number";
	tDesc.fMin = 0.0f;
	tDesc.fMax = 10.0f;
	tDesc.fStep = 0.5f;
	tDesc.fValue = 1.5f;
	tDesc.iPrecision = 2;
	iRet = xuiNumericInputCreate(pContext, &pNumeric, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pNumeric != NULL, "numeric create");
	xuiWidgetSetRect(pNumeric, (xui_rect_t){24.0f, 24.0f, 150.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pNumeric);
	XUI_TEST_CHECK(iRet == XUI_OK, "add numeric");
	iRet = xuiNumericInputSetChange(pNumeric, __xuiNumericInputChanged, &tChange);
	XUI_TEST_CHECK(iRet == XUI_OK, "change callback");
	iRet = xuiNumericInputSetErrorChange(pNumeric, __xuiNumericInputErrorChanged, &tChange);
	XUI_TEST_CHECK(iRet == XUI_OK, "error callback");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 360, 220, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiNumericInputRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render initial");

	pInput = xuiNumericInputGetInputWidget(pNumeric);
	XUI_TEST_CHECK(pInput != NULL, "input child");
	XUI_TEST_CHECK(xuiInputGetTextAlign(pInput) == XUI_INPUT_ALIGN_RIGHT, "input child right align");
	XUI_TEST_CHECK(strcmp(xuiNumericInputGetText(pNumeric), "1.50") == 0, "initial text");
	XUI_TEST_CHECK(__xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 1.5f), "initial value");
	XUI_TEST_CHECK(xuiNumericInputGetStep(pNumeric) == 0.5f, "step");
	XUI_TEST_CHECK(xuiNumericInputGetPrecision(pNumeric) == 2, "precision");
	XUI_TEST_CHECK(xuiNumericInputGetSpinnerVisible(pNumeric), "spinner visible");
	iRet = xuiNumericInputGetRange(pNumeric, &fMin, &fMax);
	XUI_TEST_CHECK(iRet == XUI_OK && fMin == 0.0f && fMax == 10.0f, "range");
	tSpinner = xuiNumericInputGetSpinnerRect(pNumeric);
	XUI_TEST_CHECK(tSpinner.fW >= 20.0f && tSpinner.fH == 30.0f, "spinner rect");

	iRet = xuiNumericInputStep(pNumeric, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 2.0f), "step up");
	XUI_TEST_CHECK(tChange.iCount == 1 && __xuiNumericInputNear(tChange.fValue, 2.0f), "step change callback");
	XUI_TEST_CHECK(strcmp(xuiNumericInputGetText(pNumeric), "2.00") == 0, "step text");

	iRet = xuiNumericInputSetText(pNumeric, "bad");
	XUI_TEST_CHECK(iRet == XUI_OK, "set bad text");
	iRet = xuiNumericInputCommit(pNumeric);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiNumericInputGetError(pNumeric), "bad commit error");
	XUI_TEST_CHECK(strcmp(xuiNumericInputGetText(pNumeric), "bad") == 0, "bad text preserved");
	XUI_TEST_CHECK(tChange.iErrorCount == 1 && tChange.bError == 1, "error callback on");

	iRet = xuiNumericInputSetText(pNumeric, "9.75");
	XUI_TEST_CHECK(iRet == XUI_OK, "set valid text");
	iRet = xuiNumericInputCommit(pNumeric);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiNumericInputGetError(pNumeric), "valid commit clears error");
	XUI_TEST_CHECK(__xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 9.75f), "valid commit value");
	XUI_TEST_CHECK(strcmp(xuiNumericInputGetText(pNumeric), "9.75") == 0, "valid commit text");
	XUI_TEST_CHECK(tChange.iErrorCount == 2 && tChange.bError == 0, "error callback off");

	iRet = xuiNumericInputSetInteger(pNumeric, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiNumericInputIsInteger(pNumeric), "integer set");
	iRet = xuiNumericInputSetValue(pNumeric, 2.6f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 3.0f), "integer value round");
	XUI_TEST_CHECK(strcmp(xuiNumericInputGetText(pNumeric), "3") == 0, "integer text");

	iRet = xuiNumericInputSetRange(pNumeric, 0.0f, 3.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set range");
	iRet = xuiNumericInputSetValue(pNumeric, 3.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set max value");
	XUI_TEST_CHECK(!xuiNumericInputIsButtonEnabled(pNumeric, XUI_NUMERIC_INPUT_BUTTON_UP), "up disabled at max");
	XUI_TEST_CHECK(xuiNumericInputIsButtonEnabled(pNumeric, XUI_NUMERIC_INPUT_BUTTON_DOWN), "down enabled at max");

	iRet = xuiNumericInputSetReadonly(pNumeric, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiNumericInputIsReadonly(pNumeric), "readonly set");
	fBefore = xuiNumericInputGetValue(pNumeric);
	iRet = xuiNumericInputStep(pNumeric, -1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiNumericInputGetValue(pNumeric) == fBefore, "readonly blocks step");
	iRet = xuiNumericInputSetReadonly(pNumeric, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "readonly clear");

	iRet = xuiNumericInputSetInteger(pNumeric, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "integer clear");
	iRet = xuiNumericInputSetPrecision(pNumeric, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "precision reset");
	iRet = xuiNumericInputSetStep(pNumeric, 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "step reset");
	iRet = xuiNumericInputSetRange(pNumeric, 0.0f, 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "range reset");
	iRet = xuiNumericInputSetValue(pNumeric, 1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "key value reset");
	iRet = xuiSetFocusWidget(pContext, pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pInput, "focus input");
	iRet = xuiNumericInputSetText(pNumeric, "12.50");
	XUI_TEST_CHECK(iRet == XUI_OK, "set hotkey text");
	iRet = __xuiNumericInputDispatchKey(pContext, 'A', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "ctrl a input");
	iRet = xuiInputGetSelection(pInput, &iSelStart, &iSelEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelStart == 0 && iSelEnd == 5, "ctrl a selects child input");
	iRet = __xuiNumericInputDispatchKey(pContext, 'C', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTestProxyGetClipboardText(&tState), "12.50") == 0, "ctrl c copies child input");
	iRet = xuiTestProxySetClipboardText(&tState, "2.25");
	XUI_TEST_CHECK(iRet == XUI_OK, "set numeric clipboard");
	iRet = __xuiNumericInputDispatchKey(pContext, 'V', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiNumericInputGetText(pNumeric), "2.25") == 0, "ctrl v pastes child input");
	iRet = xuiNumericInputSetValue(pNumeric, 1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "key value reset");
	iRet = __xuiNumericInputDispatchKey(pContext, XUI_KEY_UP, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 1.5f), "key up step");
	iRet = __xuiNumericInputDispatchKey(pContext, XUI_KEY_ENTER, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiNumericInputGetError(pNumeric), "enter commit");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout before spinner");
	tWorld = xuiWidgetGetWorldRect(pNumeric);
	tButton = xuiNumericInputGetButtonRect(pNumeric, XUI_NUMERIC_INPUT_BUTTON_UP);
	iStartChange = tChange.iCount;
	iRet = xuiInputPointerMove(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "spinner move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiNumericInputGetHoverButton(pNumeric) == XUI_NUMERIC_INPUT_BUTTON_UP, "spinner hover");
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "spinner down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiNumericInputGetActiveButton(pNumeric) == XUI_NUMERIC_INPUT_BUTTON_UP, "spinner active");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "spinner up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 2.0f), "spinner step");
	XUI_TEST_CHECK(tChange.iCount > iStartChange, "spinner change");

	iRet = xuiInputPointerWheel(pContext, tWorld.fX + 12.0f, tWorld.fY + 15.0f, 0.0f, -1.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 1.5f), "wheel step");

	iRet = xuiWidgetSetEnabled(pNumeric, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	fBefore = xuiNumericInputGetValue(pNumeric);
	iRet = xuiNumericInputStep(pNumeric, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiNumericInputGetValue(pNumeric) == fBefore, "disabled blocks step");
	iRet = xuiWidgetSetEnabled(pNumeric, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "enable");

	pMenu = xuiNumericInputGetMenuWidget(pNumeric);
	XUI_TEST_CHECK(pMenu != NULL, "menu widget");
	iRet = xuiNumericInputSetMenuTitle(pNumeric, XUI_INPUT_MENU_COPY, "Copy Number");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiNumericInputGetMenuTitle(pNumeric, XUI_INPUT_MENU_COPY), "Copy Number") == 0, "menu title");
	iRet = xuiNumericInputOpenMenu(pNumeric, 60.0f, 60.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "open menu");

	iRet = xuiNumericInputSetSpinnerVisible(pNumeric, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiNumericInputGetSpinnerVisible(pNumeric), "hide spinner");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout no spinner");
	tSpinner = xuiNumericInputGetSpinnerRect(pNumeric);
	XUI_TEST_CHECK(tSpinner.fW == 0.0f, "hidden spinner rect");

	iRet = __xuiNumericInputRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "final render");

cleanup:
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_numeric_input_test passed\n");
	return 0;
}
