#include "xui.h"
#include "xui_test_proxy.h"

#include <float.h>
#include <math.h>
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

typedef struct xui_numeric_input_destroy_t {
	xui_widget pWidget;
	int iCount;
} xui_numeric_input_destroy_t;

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

static void __xuiNumericInputDestroyOnChange(xui_widget pWidget, float fValue, void* pUser)
{
	xui_numeric_input_destroy_t* pDestroy = (xui_numeric_input_destroy_t*)pUser;
	(void)fValue;
	if ( pDestroy != NULL ) pDestroy->iCount++;
	xuiWidgetDestroy(pWidget);
}

static void __xuiNumericInputDestroyOnError(xui_widget pWidget, int bError, void* pUser)
{
	xui_numeric_input_destroy_t* pDestroy = (xui_numeric_input_destroy_t*)pUser;
	(void)bError;
	if ( pDestroy != NULL ) pDestroy->iCount++;
	xuiWidgetDestroy(pWidget);
}

static int __xuiNumericInputDestroyOnFormat(float fValue, char* sBuffer, int iCapacity, void* pUser)
{
	xui_numeric_input_destroy_t* pDestroy = (xui_numeric_input_destroy_t*)pUser;
	(void)fValue;
	if ( sBuffer != NULL && iCapacity > 0 ) sBuffer[0] = '\0';
	if ( pDestroy != NULL ) {
		pDestroy->iCount++;
		xuiWidgetDestroy(pDestroy->pWidget);
	}
	return 1;
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

static int __xuiNumericInputDrag(xui_context pContext, float fX0, float fY0, float fX1, float fY1)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX0, fY0, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerMove(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiNumericInputLongPress(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pContext, 0.60f);
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
	xui_numeric_input_destroy_t tDestroy;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pInvalid;
	xui_widget pNumeric;
	xui_widget pNumericDefaultFont;
	xui_widget pDestroyChange;
	xui_widget pDestroyError;
	xui_widget pDestroyFormat;
	xui_widget pInput;
	xui_widget pDefaultInput;
	xui_widget pMenu;
	xui_widget pDefaultMenu;
	xui_surface pTarget;
	xui_font pFont;
	xui_numeric_input_desc_t tDesc;
	xui_numeric_input_desc_t tDefaultDesc;
	xui_accessible_node_t tAccessible;
	const xui_menu_item_t* pItem;
	xui_rect_t tWorld;
	xui_rect_t tButton;
	xui_rect_t tSpinner;
	xui_rect_t tTextRect;
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
	pInvalid = NULL;
	pNumeric = NULL;
	pNumericDefaultFont = NULL;
	pDestroyChange = NULL;
	pDestroyError = NULL;
	pDestroyFormat = NULL;
	pInput = NULL;
	pDefaultInput = NULL;
	pMenu = NULL;
	pDefaultMenu = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tChange, 0, sizeof(tChange));
	memset(&tDestroy, 0, sizeof(tDestroy));
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
	tDesc.fMin = NAN;
	pInvalid = pRoot;
	iRet = xuiNumericInputCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"invalid descriptor clears create output");
	pInvalid = pRoot;
	iRet = xuiNumericInputCreate(NULL, &pInvalid, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"invalid context clears create output");

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

	memset(&tDefaultDesc, 0, sizeof(tDefaultDesc));
	tDefaultDesc.iSize = sizeof(tDefaultDesc);
	tDefaultDesc.fMin = 0.0f;
	tDefaultDesc.fMax = 5.0f;
	tDefaultDesc.fValue = 2.0f;
	tDefaultDesc.iPrecision = 1;
	iRet = xuiNumericInputCreate(pContext, &pNumericDefaultFont, &tDefaultDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pNumericDefaultFont != NULL, "default font numeric create");
	xuiWidgetSetRect(pNumericDefaultFont, (xui_rect_t){24.0f, 64.0f, 150.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pNumericDefaultFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "add default font numeric");

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
	pDefaultInput = xuiNumericInputGetInputWidget(pNumericDefaultFont);
	XUI_TEST_CHECK(pDefaultInput != NULL, "default font input child");
	XUI_TEST_CHECK(xuiInputGetFont(pDefaultInput) == pFont, "default font propagated to input child");
	pDefaultMenu = xuiNumericInputGetMenuWidget(pNumericDefaultFont);
	XUI_TEST_CHECK(pDefaultMenu != NULL && xuiMenuGetFont(pDefaultMenu) == pFont, "default font propagated to input menu");
	XUI_TEST_CHECK(strcmp(xuiNumericInputGetText(pNumeric), "1.50") == 0, "initial text");
	XUI_TEST_CHECK(__xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 1.5f), "initial value");
	XUI_TEST_CHECK(xuiNumericInputGetStep(pNumeric) == 0.5f, "step");
	XUI_TEST_CHECK(xuiNumericInputGetPrecision(pNumeric) == 2, "precision");
	XUI_TEST_CHECK(xuiNumericInputGetSpinnerVisible(pNumeric), "spinner visible");
	iRet = xuiNumericInputGetRange(pNumeric, &fMin, &fMax);
	XUI_TEST_CHECK(iRet == XUI_OK && fMin == 0.0f && fMax == 10.0f, "range");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pNumeric, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_TEXTBOX &&
		tAccessible.tRange.fValue == 1.5 && tAccessible.tRange.fMin == 0.0 &&
		tAccessible.tRange.fMax == 10.0 &&
		(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_INCREMENT)) != 0,
		"numeric accessibility exposes range actions");
	iRet = xuiWidgetPerformAccessibleAction(pNumeric, 1, XUI_ACCESSIBLE_ACTION_INCREMENT, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 2.0f) &&
		tChange.iCount == 1, "accessible increment changes value");
	iRet = xuiNumericInputSetValue(pNumeric, 1.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset after accessible increment");
	memset(&tChange, 0, sizeof(tChange));
	fBefore = xuiNumericInputGetValue(pNumeric);
	iRet = xuiNumericInputSetRange(pNumeric, NAN, 10.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "range rejects NaN");
	iRet = xuiNumericInputGetRange(pNumeric, &fMin, &fMax);
	XUI_TEST_CHECK(iRet == XUI_OK && fMin == 0.0f && fMax == 10.0f &&
		xuiNumericInputGetValue(pNumeric) == fBefore, "invalid range preserves state");
	iRet = xuiNumericInputSetStep(pNumeric, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiNumericInputGetStep(pNumeric) == 0.5f,
		"step rejects infinity without mutation");
	iRet = xuiNumericInputSetValue(pNumeric, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiNumericInputGetValue(pNumeric) == fBefore,
		"value rejects NaN without mutation");
	iRet = xuiNumericInputSetSpinnerWidth(pNumeric, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "spinner width rejects infinity");
	iRet = xuiNumericInputSetBorderWidth(pNumeric, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "border width rejects NaN");
	iRet = xuiNumericInputSetPrecision(pNumeric, -1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiNumericInputGetPrecision(pNumeric) == 2,
		"precision rejects negative value");
	tSpinner = xuiNumericInputGetSpinnerRect(pNumeric);
	XUI_TEST_CHECK(tSpinner.fW >= 20.0f && tSpinner.fH == 30.0f, "spinner rect");
	tWorld = xuiWidgetGetRect(pInput);
	XUI_TEST_CHECK(__xuiNumericInputNear(tWorld.fX, 0.0f) &&
	               __xuiNumericInputNear(tWorld.fY, 0.0f) &&
	               __xuiNumericInputNear(tWorld.fW + tSpinner.fW, 150.0f) &&
	               __xuiNumericInputNear(tWorld.fH, 30.0f), "input child declarative slot");

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
	iRet = xuiNumericInputSetText(pNumeric, "0x10");
	XUI_TEST_CHECK(iRet == XUI_OK, "set hexadecimal text");
	iRet = xuiNumericInputCommit(pNumeric);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiNumericInputGetError(pNumeric) &&
		tChange.iErrorCount == 1, "numeric parser rejects hexadecimal extension");

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
	iRet = xuiNumericInputSetRange(pNumeric, 0.2f, 2.8f);
	XUI_TEST_CHECK(iRet == XUI_OK, "fractional integer range set");
	iRet = xuiNumericInputSetValue(pNumeric, 2.8f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiNumericInputGetValue(pNumeric) == 2.0f &&
		strcmp(xuiNumericInputGetText(pNumeric), "2") == 0 &&
		!xuiNumericInputIsButtonEnabled(pNumeric, XUI_NUMERIC_INPUT_BUTTON_UP),
		"integer value and spinner use effective integer bounds");
	iRet = xuiNumericInputSetRange(pNumeric, 2.2f, 2.8f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "integer range rejects interval without integer");
	iRet = xuiNumericInputSetRange(pNumeric, -FLT_MAX, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "large integer range set");
	iRet = xuiNumericInputSetValue(pNumeric, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK && isfinite(xuiNumericInputGetValue(pNumeric)) &&
		strlen(xuiNumericInputGetText(pNumeric)) > 10u, "large integer value formats without narrowing");
	iRet = xuiNumericInputSetRange(pNumeric, 5.0f, 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiNumericInputGetValue(pNumeric) == 5.0f &&
		!xuiNumericInputIsButtonEnabled(pNumeric, XUI_NUMERIC_INPUT_BUTTON_UP) &&
		!xuiNumericInputIsButtonEnabled(pNumeric, XUI_NUMERIC_INPUT_BUTTON_DOWN),
		"equal range remains a fixed value");

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
	fBefore = xuiNumericInputGetValue(pNumeric);
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable numeric ancestor");
	iRet = xuiNumericInputStep(pNumeric, -1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiNumericInputGetValue(pNumeric) == fBefore &&
		!xuiNumericInputIsButtonEnabled(pNumeric, XUI_NUMERIC_INPUT_BUTTON_DOWN) &&
		(xuiNumericInputGetState(pNumeric) & XUI_WIDGET_STATE_DISABLED) != 0,
		"ancestor disabled state blocks numeric changes");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "enable numeric ancestor");

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
	iRet = xuiNumericInputSetText(pNumeric, "1234");
	XUI_TEST_CHECK(iRet == XUI_OK, "set drag move text");
	iRet = xuiInputSetSelection(pInput, 1, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "select drag move range");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag move layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag move update");
	tWorld = xuiWidgetGetWorldRect(pInput);
	tTextRect = xuiInputGetTextRect(pInput);
	iRet = __xuiNumericInputDrag(pContext,
		tWorld.fX + tTextRect.fX + tTextRect.fW - 16.0f,
		tWorld.fY + tTextRect.fY + tTextRect.fH * 0.5f,
		tWorld.fX + tTextRect.fX + tTextRect.fW - 2.0f,
		tWorld.fY + tTextRect.fY + tTextRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiNumericInputGetText(pNumeric), "1423") == 0, "selection drag moves child input text");
	iRet = xuiInputGetSelection(pInput, &iSelStart, &iSelEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelStart == 2 && iSelEnd == 4, "selection follows moved child input text");
	iRet = xuiNumericInputSetValue(pNumeric, 1.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore value after drag move");

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
	iRet = xuiNumericInputSetValue(pNumeric, 1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "spinner repeat reset value");
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "spinner repeat down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "spinner repeat down dispatch");
	iRet = xuiUpdate(pContext, 0.39f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 1.0f), "spinner repeat delay");
	iRet = xuiUpdate(pContext, 0.02f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 1.5f), "spinner repeat first step");
	iRet = xuiUpdate(pContext, 0.16f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 2.5f), "spinner repeat interval");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "spinner repeat up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 2.5f), "spinner repeat release does not duplicate");
	iRet = xuiNumericInputSetValue(pNumeric, 1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "spinner outside repeat reset value");
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tButton.fX + tButton.fW * 0.5f,
		tWorld.fY + tButton.fY + tButton.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "spinner outside repeat down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "spinner outside repeat down dispatch");
	iRet = xuiInputPointerMove(pContext, tWorld.fX - 8.0f, tWorld.fY - 8.0f,
		XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "spinner outside repeat move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "spinner outside repeat move dispatch");
	iRet = xuiUpdate(pContext, 0.60f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 1.0f),
		"spinner repeat pauses outside active button");
	iRet = xuiInputPointerUp(pContext, tWorld.fX - 8.0f, tWorld.fY - 8.0f,
		XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "spinner outside repeat up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 1.0f),
		"spinner outside release does not step");

	iRet = xuiNumericInputSetValue(pNumeric, 1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi-touch spinner reset value");
	iRet = xuiInputPointerDownEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tButton.fX + tButton.fW * 0.5f),
		(int)(tWorld.fY + tButton.fY + tButton.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH) == pNumeric &&
		xuiNumericInputGetActiveButton(pNumeric) == XUI_NUMERIC_INPUT_BUTTON_UP,
		"first touch owns spinner repeat");
	iRet = xuiInputPointerDownEx(pContext, 302u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tButton.fX + tButton.fW * 0.5f),
		(int)(tWorld.fY + tButton.fY + tButton.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 302u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tButton.fX + tButton.fW * 0.5f),
		(int)(tWorld.fY + tButton.fY + tButton.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH) == pNumeric &&
		xuiGetPointerCaptureEx(pContext, 302u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		__xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 1.0f),
		"unrelated touch cannot release or step spinner");
	iRet = xuiUpdate(pContext, 0.41f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 1.5f),
		"owning touch keeps spinner repeat timer");
	iRet = xuiInputPointerUpEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tButton.fX + tButton.fW * 0.5f),
		(int)(tWorld.fY + tButton.fY + tButton.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		__xuiNumericInputNear(xuiNumericInputGetValue(pNumeric), 1.5f),
		"owning touch releases spinner without duplicate step");
	iRet = xuiInputPointerDownEx(pContext, 303u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tButton.fX + tButton.fW * 0.5f),
		(int)(tWorld.fY + tButton.fY + tButton.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiNumericInputSetReadonly(pNumeric, 1);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 303u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		xuiNumericInputGetActiveButton(pNumeric) == XUI_NUMERIC_INPUT_BUTTON_NONE,
		"readonly change releases exact spinner capture");
	iRet = xuiNumericInputSetReadonly(pNumeric, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore editable after exact capture test");
	(void)xuiInputPointerUpEx(pContext, 303u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tButton.fX + tButton.fW * 0.5f),
		(int)(tWorld.fY + tButton.fY + tButton.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	(void)xuiDispatchPendingEvents(pContext);
	iRet = xuiNumericInputSetValue(pNumeric, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore spinner value after repeat");

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
	XUI_TEST_CHECK(strcmp(xuiNumericInputGetMenuTitle(pNumeric, XUI_INPUT_MENU_COPY), "Copy") == 0, "default menu title");
	iRet = xuiSetLanguage(pContext, XUI_LANGUAGE_ZH);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiNumericInputGetMenuTitle(pNumeric, XUI_INPUT_MENU_COPY), "复制") == 0, "localized menu title");
	iRet = xuiSetLanguage(pContext, XUI_LANGUAGE_EN);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiNumericInputGetMenuTitle(pNumeric, XUI_INPUT_MENU_COPY), "Copy") == 0, "restore English menu title");
	iRet = xuiNumericInputOpenMenu(pNumeric, 60.0f, 60.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "open menu");
	XUI_TEST_CHECK(xuiMenuGetItemCount(pMenu) == 9, "numeric input menu item count");
	pItem = xuiMenuGetItem(pMenu, 4);
	iRet = xuiMenuClose(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close numeric menu");
	iRet = xuiNumericInputSetText(pNumeric, "1234");
	XUI_TEST_CHECK(iRet == XUI_OK, "set long press menu text");
	iRet = xuiInputSetSelection(pInput, 0, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "selection for long press menu");
	tWorld = xuiWidgetGetWorldRect(pInput);
	tTextRect = xuiInputGetTextRect(pInput);
	iRet = __xuiNumericInputLongPress(pContext,
		tWorld.fX + tTextRect.fX + tTextRect.fW * 0.5f,
		tWorld.fY + tTextRect.fY + tTextRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "long press opens numeric input menu");
	iRet = xuiMenuClose(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close numeric long press menu");
	XUI_TEST_CHECK(pItem != NULL && pItem->sText != NULL && strcmp(pItem->sText, "Copy") == 0, "numeric input copy item");

	iRet = xuiNumericInputSetSpinnerVisible(pNumeric, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiNumericInputGetSpinnerVisible(pNumeric), "hide spinner");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout no spinner");
	tSpinner = xuiNumericInputGetSpinnerRect(pNumeric);
	XUI_TEST_CHECK(tSpinner.fW == 0.0f, "hidden spinner rect");

	iRet = __xuiNumericInputRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "final render");

	memset(&tDefaultDesc, 0, sizeof(tDefaultDesc));
	tDefaultDesc.iSize = sizeof(tDefaultDesc);
	tDefaultDesc.fMin = 0.0f;
	tDefaultDesc.fMax = 10.0f;
	tDefaultDesc.fValue = 1.0f;
	iRet = xuiNumericInputCreate(pContext, &pDestroyChange, &tDefaultDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyChange != NULL, "destroy-on-change create");
	iRet = xuiWidgetAddChild(pRoot, pDestroyChange);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-change attach");
	memset(&tDestroy, 0, sizeof(tDestroy));
	tDestroy.pWidget = pDestroyChange;
	iRet = xuiNumericInputSetChange(pDestroyChange, __xuiNumericInputDestroyOnChange, &tDestroy);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-change callback set");
	iRet = xuiNumericInputStep(pDestroyChange, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroy.iCount == 1 &&
		xuiNumericInputGetInputWidget(pDestroyChange) == NULL, "change callback may destroy numeric input");

	iRet = xuiNumericInputCreate(pContext, &pDestroyError, &tDefaultDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyError != NULL, "destroy-on-error create");
	iRet = xuiWidgetAddChild(pRoot, pDestroyError);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-error attach");
	memset(&tDestroy, 0, sizeof(tDestroy));
	tDestroy.pWidget = pDestroyError;
	iRet = xuiNumericInputSetErrorChange(pDestroyError, __xuiNumericInputDestroyOnError, &tDestroy);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-error callback set");
	iRet = xuiNumericInputSetText(pDestroyError, "invalid");
	if ( iRet == XUI_OK ) iRet = xuiNumericInputCommit(pDestroyError);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroy.iCount == 1 &&
		xuiNumericInputGetInputWidget(pDestroyError) == NULL, "error callback may destroy numeric input");

	iRet = xuiNumericInputCreate(pContext, &pDestroyFormat, &tDefaultDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyFormat != NULL, "destroy-on-format create");
	iRet = xuiWidgetAddChild(pRoot, pDestroyFormat);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-format attach");
	memset(&tDestroy, 0, sizeof(tDestroy));
	tDestroy.pWidget = pDestroyFormat;
	iRet = xuiNumericInputSetFormatter(pDestroyFormat, __xuiNumericInputDestroyOnFormat, &tDestroy);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroy.iCount == 1 &&
		xuiNumericInputGetInputWidget(pDestroyFormat) == NULL, "formatter callback may destroy numeric input");

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
