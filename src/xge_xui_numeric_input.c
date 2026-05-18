static void __xgeXuiNumericInputSetValueInternal(xge_xui_numeric_input pNumeric, float fValue, int bNotify);
static void __xgeXuiNumericInputPaintAfterProc(xge_xui_widget pWidget, void* pUser);

static float __xgeXuiNumericInputClamp(xge_xui_numeric_input pNumeric, float fValue)
{
	if ( pNumeric == NULL ) {
		return fValue;
	}
	if ( fValue < pNumeric->fMin ) {
		fValue = pNumeric->fMin;
	}
	if ( fValue > pNumeric->fMax ) {
		fValue = pNumeric->fMax;
	}
	return fValue;
}

static void __xgeXuiNumericInputSyncSpinnerPadding(xge_xui_numeric_input pNumeric)
{
	xge_xui_input_decoration_desc_t tDesc;

	if ( pNumeric == NULL ) {
		return;
	}
	if ( pNumeric->bShowSpinner == 0 ) {
		if ( pNumeric->pSpinnerPaddingDecoration != NULL ) {
			xgeXuiInputDecorationRemove(&pNumeric->tInput, pNumeric->pSpinnerPaddingDecoration);
			pNumeric->pSpinnerPaddingDecoration = NULL;
		}
		return;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iKind = XGE_XUI_INPUT_DECORATION_CUSTOM_PAINT;
	tDesc.iVisibleMode = XGE_XUI_INPUT_DECORATION_VISIBLE_ALWAYS;
	tDesc.fWidth = pNumeric->fSpinnerWidth;
	tDesc.fPadding = 0.0f;
	if ( pNumeric->pSpinnerPaddingDecoration == NULL ) {
		pNumeric->pSpinnerPaddingDecoration = xgeXuiInputDecorationAdd(&pNumeric->tInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, &tDesc);
	} else {
		xgeXuiInputDecorationSet(&pNumeric->tInput, pNumeric->pSpinnerPaddingDecoration, &tDesc);
	}
}

static void __xgeXuiNumericInputFormat(xge_xui_numeric_input pNumeric)
{
	char sFmt[16];
	char sValue[64];

	if ( pNumeric == NULL ) {
		return;
	}
	if ( pNumeric->procFormat != NULL ) {
		sValue[0] = 0;
		if ( (pNumeric->procFormat(pNumeric->fValue, sValue, (int)sizeof(sValue), pNumeric->pFormatUser) == XGE_OK) && (sValue[0] != 0) ) {
			sValue[sizeof(sValue) - 1] = 0;
			xgeXuiInputSetText(&pNumeric->tInput, sValue);
			return;
		}
	}
	if ( pNumeric->bInteger ) {
		snprintf(sValue, sizeof(sValue), "%d", (int)((pNumeric->fValue >= 0.0f) ? (pNumeric->fValue + 0.5f) : (pNumeric->fValue - 0.5f)));
	} else {
		snprintf(sFmt, sizeof(sFmt), "%%.%df", pNumeric->iPrecision);
		sFmt[sizeof(sFmt) - 1] = 0;
		snprintf(sValue, sizeof(sValue), sFmt, pNumeric->fValue);
	}
	sValue[sizeof(sValue) - 1] = 0;
	xgeXuiInputSetText(&pNumeric->tInput, sValue);
}

static void __xgeXuiNumericInputSetError(xge_xui_numeric_input pNumeric, int bError)
{
	if ( pNumeric == NULL ) {
		return;
	}
	bError = (bError != 0);
	if ( pNumeric->bError != bError ) {
		pNumeric->bError = bError;
		xgeXuiInputSetError(&pNumeric->tInput, bError);
		xgeXuiWidgetMarkPaint(pNumeric->pWidget);
	} else if ( bError != 0 ) {
		xgeXuiInputSetError(&pNumeric->tInput, bError);
	}
}

static int __xgeXuiNumericInputCommitText(xge_xui_numeric_input pNumeric, int bNotify)
{
	const char* sText;
	char* sEnd;
	double fValue;

	if ( pNumeric == NULL ) {
		return 0;
	}
	sText = xgeXuiInputGetText(&pNumeric->tInput);
	fValue = strtod(sText, &sEnd);
	if ( (sText == sEnd) || (sEnd == NULL) ) {
		__xgeXuiNumericInputSetError(pNumeric, 1);
		return 0;
	}
	while ( *sEnd == ' ' || *sEnd == '\t' ) {
		sEnd++;
	}
	if ( *sEnd != 0 ) {
		__xgeXuiNumericInputSetError(pNumeric, 1);
		return 0;
	}
	__xgeXuiNumericInputSetValueInternal(pNumeric, (float)fValue, bNotify);
	return 1;
}

static void __xgeXuiNumericInputSetValueInternal(xge_xui_numeric_input pNumeric, float fValue, int bNotify)
{
	if ( pNumeric == NULL ) {
		return;
	}
	if ( pNumeric->bInteger ) {
		fValue = (float)((int)((fValue >= 0.0f) ? (fValue + 0.5f) : (fValue - 0.5f)));
	}
	fValue = __xgeXuiNumericInputClamp(pNumeric, fValue);
	__xgeXuiNumericInputSetError(pNumeric, 0);
	if ( pNumeric->fValue == fValue ) {
		__xgeXuiNumericInputFormat(pNumeric);
		xgeXuiWidgetMarkPaint(pNumeric->pWidget);
		return;
	}
	pNumeric->fValue = fValue;
	pNumeric->iChangeCount++;
	__xgeXuiNumericInputFormat(pNumeric);
	xgeXuiWidgetMarkPaint(pNumeric->pWidget);
	if ( bNotify && pNumeric->procChange != NULL ) {
		pNumeric->procChange(pNumeric->pWidget, pNumeric->fValue, pNumeric->pUser);
	}
}

static xge_rect_t __xgeXuiNumericInputSpinnerRect(xge_xui_numeric_input pNumeric)
{
	xge_rect_t tRect;
	float fRight;

	memset(&tRect, 0, sizeof(tRect));
	if ( (pNumeric == NULL) || (pNumeric->pWidget == NULL) || (pNumeric->bShowSpinner == 0) ) {
		return tRect;
	}
	tRect = pNumeric->pWidget->tPaddingRect;
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		tRect = pNumeric->pWidget->tContentRect;
	}
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		tRect = pNumeric->pWidget->tBorderRect;
	}
	fRight = tRect.fX + tRect.fW;
	tRect.fW = pNumeric->fSpinnerWidth;
	if ( tRect.fW < 12.0f ) {
		tRect.fW = 12.0f;
	}
	tRect.fX = fRight - tRect.fW;
	return tRect;
}

static xge_rect_t __xgeXuiNumericInputButtonRect(xge_xui_numeric_input pNumeric, int iButton)
{
	xge_rect_t tRect;

	tRect = __xgeXuiNumericInputSpinnerRect(pNumeric);
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		return tRect;
	}
	tRect.fH *= 0.5f;
	if ( iButton == XGE_XUI_NUMERIC_INPUT_BUTTON_DOWN ) {
		tRect.fY += tRect.fH;
	}
	return tRect;
}

static int __xgeXuiNumericInputHitButton(xge_xui_numeric_input pNumeric, float fX, float fY)
{
	if ( pNumeric == NULL || pNumeric->bShowSpinner == 0 ) {
		return XGE_XUI_NUMERIC_INPUT_BUTTON_NONE;
	}
	if ( __xgeXuiRectContains(__xgeXuiNumericInputButtonRect(pNumeric, XGE_XUI_NUMERIC_INPUT_BUTTON_UP), fX, fY) ) {
		return XGE_XUI_NUMERIC_INPUT_BUTTON_UP;
	}
	if ( __xgeXuiRectContains(__xgeXuiNumericInputButtonRect(pNumeric, XGE_XUI_NUMERIC_INPUT_BUTTON_DOWN), fX, fY) ) {
		return XGE_XUI_NUMERIC_INPUT_BUTTON_DOWN;
	}
	return XGE_XUI_NUMERIC_INPUT_BUTTON_NONE;
}

static int __xgeXuiNumericInputCanEdit(xge_xui_numeric_input pNumeric)
{
	if ( (pNumeric == NULL) || (pNumeric->pWidget == NULL) ) {
		return 0;
	}
	if ( ((pNumeric->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) || (pNumeric->tInput.bDisabled != 0) || (pNumeric->tInput.bReadonly != 0) ) {
		return 0;
	}
	return 1;
}

static int __xgeXuiNumericInputButtonEnabled(xge_xui_numeric_input pNumeric, int iButton)
{
	if ( !__xgeXuiNumericInputCanEdit(pNumeric) ) {
		return 0;
	}
	if ( iButton == XGE_XUI_NUMERIC_INPUT_BUTTON_UP ) {
		return pNumeric->fValue < pNumeric->fMax;
	}
	if ( iButton == XGE_XUI_NUMERIC_INPUT_BUTTON_DOWN ) {
		return pNumeric->fValue > pNumeric->fMin;
	}
	return 0;
}

static int __xgeXuiNumericInputStep(xge_xui_numeric_input pNumeric, float fDirection)
{
	if ( !__xgeXuiNumericInputCanEdit(pNumeric) ) {
		return 0;
	}
	(void)__xgeXuiNumericInputCommitText(pNumeric, 0);
	if ( pNumeric->bError ) {
		return 0;
	}
	if ( (fDirection > 0.0f && pNumeric->fValue >= pNumeric->fMax) || (fDirection < 0.0f && pNumeric->fValue <= pNumeric->fMin) ) {
		return 0;
	}
	__xgeXuiNumericInputSetValueInternal(pNumeric, pNumeric->fValue + pNumeric->fStep * fDirection, 1);
	return 1;
}

static void __xgeXuiNumericInputClearButtonState(xge_xui_numeric_input pNumeric)
{
	if ( pNumeric == NULL ) {
		return;
	}
	if ( pNumeric->iHoverButton != XGE_XUI_NUMERIC_INPUT_BUTTON_NONE || pNumeric->iActiveButton != XGE_XUI_NUMERIC_INPUT_BUTTON_NONE ) {
		pNumeric->iHoverButton = XGE_XUI_NUMERIC_INPUT_BUTTON_NONE;
		pNumeric->iActiveButton = XGE_XUI_NUMERIC_INPUT_BUTTON_NONE;
		xgeXuiWidgetMarkPaint(pNumeric->pWidget);
	}
}

int xgeXuiNumericInputInit(xge_xui_numeric_input pNumeric, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	int iRet;

	if ( (pNumeric == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pNumeric, 0, sizeof(*pNumeric));
	iRet = xgeXuiInputInit(&pNumeric->tInput, pContext, pWidget, pFont);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pNumeric->pContext = pContext;
	pNumeric->pWidget = pWidget;
	pNumeric->fMin = 0.0f;
	pNumeric->fMax = 100.0f;
	pNumeric->fStep = 1.0f;
	pNumeric->fSpinnerWidth = 22.0f;
	pNumeric->bShowSpinner = 1;
	pNumeric->iPrecision = 3;
	pNumeric->iSpinnerColor = XGE_COLOR_RGBA(226, 238, 248, 255);
	pNumeric->iSpinnerHoverColor = XGE_COLOR_RGBA(206, 229, 245, 255);
	pNumeric->iSpinnerActiveColor = XGE_COLOR_RGBA(174, 211, 238, 255);
	pNumeric->iSpinnerDisabledColor = XGE_COLOR_RGBA(232, 237, 242, 255);
	pNumeric->iSpinnerIconColor = XGE_COLOR_RGBA(34, 86, 132, 255);
	pNumeric->iSpinnerDisabledIconColor = XGE_COLOR_RGBA(135, 148, 160, 255);
	xgeXuiInputSetTextAlign(&pNumeric->tInput, XGE_XUI_INPUT_TEXT_ALIGN_RIGHT);
	__xgeXuiNumericInputSyncSpinnerPadding(pNumeric);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiNumericInputEventProc, NULL);
	pWidget->procUpdate = xgeXuiNumericInputUpdateProc;
	pWidget->procPaint = xgeXuiNumericInputPaintProc;
	xgeXuiWidgetSetPaintAfter(pWidget, __xgeXuiNumericInputPaintAfterProc, pNumeric);
	pWidget->pUser = pNumeric;
	__xgeXuiNumericInputFormat(pNumeric);
	return XGE_OK;
}

void xgeXuiNumericInputUnit(xge_xui_numeric_input pNumeric)
{
	if ( pNumeric == NULL ) {
		return;
	}
	if ( pNumeric->pWidget != NULL && pNumeric->pWidget->pUser == pNumeric ) {
		pNumeric->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pNumeric->pWidget, NULL, NULL);
		pNumeric->pWidget->procUpdate = NULL;
		pNumeric->pWidget->procPaint = NULL;
		pNumeric->pWidget->procPaintAfter = NULL;
		pNumeric->pWidget->pPaintAfterUser = NULL;
		pNumeric->pWidget->iCallbackFlags &= ~XGE_XUI_WIDGET_CALLBACK_PAINT_AFTER;
	}
	xgeXuiInputUnit(&pNumeric->tInput);
	memset(pNumeric, 0, sizeof(*pNumeric));
}

void xgeXuiNumericInputSetChange(xge_xui_numeric_input pNumeric, xge_xui_slider_proc procChange, void* pUser)
{
	if ( pNumeric == NULL ) {
		return;
	}
	pNumeric->procChange = procChange;
	pNumeric->pUser = pUser;
}

void xgeXuiNumericInputSetErrorChange(xge_xui_numeric_input pNumeric, xge_xui_input_error_proc procError, void* pUser)
{
	if ( pNumeric == NULL ) {
		return;
	}
	xgeXuiInputSetErrorChange(&pNumeric->tInput, procError, pUser);
}

void xgeXuiNumericInputSetFormatter(xge_xui_numeric_input pNumeric, xge_xui_numeric_format_proc procFormat, void* pUser)
{
	if ( pNumeric == NULL ) {
		return;
	}
	pNumeric->procFormat = procFormat;
	pNumeric->pFormatUser = pUser;
	__xgeXuiNumericInputFormat(pNumeric);
	xgeXuiWidgetMarkPaint(pNumeric->pWidget);
}

void xgeXuiNumericInputSetRange(xge_xui_numeric_input pNumeric, float fMin, float fMax)
{
	float fSwap;

	if ( pNumeric == NULL ) {
		return;
	}
	if ( fMax < fMin ) {
		fSwap = fMin;
		fMin = fMax;
		fMax = fSwap;
	}
	if ( fMax == fMin ) {
		fMax = fMin + 1.0f;
	}
	pNumeric->fMin = fMin;
	pNumeric->fMax = fMax;
	__xgeXuiNumericInputSetValueInternal(pNumeric, pNumeric->fValue, 0);
}

void xgeXuiNumericInputSetStep(xge_xui_numeric_input pNumeric, float fStep)
{
	if ( pNumeric == NULL ) {
		return;
	}
	if ( fStep <= 0.0f ) {
		fStep = 1.0f;
	}
	pNumeric->fStep = fStep;
}

void xgeXuiNumericInputSetInteger(xge_xui_numeric_input pNumeric, int bInteger)
{
	if ( pNumeric == NULL ) {
		return;
	}
	pNumeric->bInteger = (bInteger != 0);
	__xgeXuiNumericInputSetValueInternal(pNumeric, pNumeric->fValue, 0);
}

void xgeXuiNumericInputSetPrecision(xge_xui_numeric_input pNumeric, int iPrecision)
{
	if ( pNumeric == NULL ) {
		return;
	}
	if ( iPrecision < 0 ) {
		iPrecision = 0;
	}
	if ( iPrecision > 9 ) {
		iPrecision = 9;
	}
	pNumeric->iPrecision = iPrecision;
	if ( pNumeric->bInteger == 0 ) {
		__xgeXuiNumericInputFormat(pNumeric);
		xgeXuiWidgetMarkPaint(pNumeric->pWidget);
	}
}

void xgeXuiNumericInputSetSpinnerVisible(xge_xui_numeric_input pNumeric, int bVisible)
{
	if ( pNumeric == NULL ) {
		return;
	}
	pNumeric->bShowSpinner = (bVisible != 0);
	__xgeXuiNumericInputSyncSpinnerPadding(pNumeric);
	__xgeXuiNumericInputClearButtonState(pNumeric);
	xgeXuiWidgetMarkPaint(pNumeric->pWidget);
}

void xgeXuiNumericInputSetSpinnerWidth(xge_xui_numeric_input pNumeric, float fWidth)
{
	if ( pNumeric == NULL ) {
		return;
	}
	if ( fWidth < 16.0f ) {
		fWidth = 16.0f;
	}
	pNumeric->fSpinnerWidth = fWidth;
	__xgeXuiNumericInputSyncSpinnerPadding(pNumeric);
	xgeXuiWidgetMarkPaint(pNumeric->pWidget);
}

void xgeXuiNumericInputSetSpinnerColors(xge_xui_numeric_input pNumeric, uint32_t iColor, uint32_t iHoverColor, uint32_t iActiveColor, uint32_t iDisabledColor, uint32_t iIconColor, uint32_t iDisabledIconColor)
{
	if ( pNumeric == NULL ) {
		return;
	}
	pNumeric->iSpinnerColor = iColor;
	pNumeric->iSpinnerHoverColor = iHoverColor;
	pNumeric->iSpinnerActiveColor = iActiveColor;
	pNumeric->iSpinnerDisabledColor = iDisabledColor;
	pNumeric->iSpinnerIconColor = iIconColor;
	pNumeric->iSpinnerDisabledIconColor = iDisabledIconColor;
	xgeXuiWidgetMarkPaint(pNumeric->pWidget);
}

void xgeXuiNumericInputSetValue(xge_xui_numeric_input pNumeric, float fValue)
{
	__xgeXuiNumericInputSetValueInternal(pNumeric, fValue, 0);
}

float xgeXuiNumericInputGetValue(xge_xui_numeric_input pNumeric)
{
	return (pNumeric != NULL) ? pNumeric->fValue : 0.0f;
}

int xgeXuiNumericInputGetState(xge_xui_numeric_input pNumeric)
{
	if ( pNumeric == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	if ( (pNumeric->pWidget == NULL) || ((pNumeric->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		return XGE_XUI_STATE_DISABLED;
	}
	return pNumeric->iState;
}

int xgeXuiNumericInputEvent(xge_xui_numeric_input pNumeric, const xge_event_t* pEvent)
{
	int iButton;
	int iRet;

	if ( (pNumeric == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	switch ( pEvent->iType ) {
		case XGE_EVENT_XUI_POINTER_LEAVE:
			if ( pNumeric->iActiveButton == XGE_XUI_NUMERIC_INPUT_BUTTON_NONE ) {
				__xgeXuiNumericInputClearButtonState(pNumeric);
			}
			break;

		case XGE_EVENT_MOUSE_MOVE:
			iButton = __xgeXuiNumericInputHitButton(pNumeric, pEvent->fX, pEvent->fY);
			if ( pNumeric->iHoverButton != iButton ) {
				pNumeric->iHoverButton = iButton;
				xgeXuiWidgetMarkPaint(pNumeric->pWidget);
			}
			break;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( pNumeric->iActiveButton == XGE_XUI_NUMERIC_INPUT_BUTTON_NONE ) {
				iButton = __xgeXuiNumericInputHitButton(pNumeric, pEvent->fX, pEvent->fY);
				if ( __xgeXuiNumericInputButtonEnabled(pNumeric, iButton) ) {
					pNumeric->iActiveButton = iButton;
					pNumeric->iHoverButton = iButton;
					xgeXuiSetFocus(pNumeric->pContext, pNumeric->pWidget);
					xgeXuiSetPointerCapture(pNumeric->pContext, pEvent->iPointerId, pNumeric->pWidget);
					xgeXuiWidgetMarkPaint(pNumeric->pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			break;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			if ( pNumeric->iActiveButton != XGE_XUI_NUMERIC_INPUT_BUTTON_NONE ) {
				if ( xgeXuiGetPointerCapture(pNumeric->pContext, pEvent->iPointerId) != pNumeric->pWidget ) {
					return XGE_XUI_EVENT_CONTINUE;
				}
				iButton = __xgeXuiNumericInputHitButton(pNumeric, pEvent->fX, pEvent->fY);
				if ( iButton == pNumeric->iActiveButton ) {
					(void)__xgeXuiNumericInputStep(pNumeric, (iButton == XGE_XUI_NUMERIC_INPUT_BUTTON_UP) ? 1.0f : -1.0f);
				}
				pNumeric->iActiveButton = XGE_XUI_NUMERIC_INPUT_BUTTON_NONE;
				pNumeric->iHoverButton = iButton;
				xgeXuiSetPointerCapture(pNumeric->pContext, pEvent->iPointerId, NULL);
				xgeXuiWidgetMarkPaint(pNumeric->pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			break;

		case XGE_EVENT_TOUCH_CANCEL:
			if ( pNumeric->iActiveButton != XGE_XUI_NUMERIC_INPUT_BUTTON_NONE ) {
				pNumeric->iActiveButton = XGE_XUI_NUMERIC_INPUT_BUTTON_NONE;
				xgeXuiSetPointerCapture(pNumeric->pContext, pEvent->iPointerId, NULL);
				xgeXuiWidgetMarkPaint(pNumeric->pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			break;

		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pNumeric->iActiveButton = XGE_XUI_NUMERIC_INPUT_BUTTON_NONE;
			xgeXuiWidgetMarkPaint(pNumeric->pWidget);
			break;

		case XGE_EVENT_MOUSE_WHEEL:
			if ( (pEvent->fDY != 0.0f) && (pNumeric->iActiveButton == XGE_XUI_NUMERIC_INPUT_BUTTON_NONE) && __xgeXuiRectContains(pNumeric->pWidget->tRect, pEvent->fX, pEvent->fY) && __xgeXuiNumericInputCanEdit(pNumeric) ) {
				xgeXuiSetFocus(pNumeric->pContext, pNumeric->pWidget);
				(void)__xgeXuiNumericInputStep(pNumeric, (pEvent->fDY > 0.0f) ? 1.0f : -1.0f);
				return XGE_XUI_EVENT_CONSUMED;
			}
			break;

		case XGE_EVENT_KEY_DOWN:
			if ( pEvent->iParam1 == XGE_KEY_UP ) {
				(void)__xgeXuiNumericInputStep(pNumeric, 1.0f);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_DOWN ) {
				(void)__xgeXuiNumericInputStep(pNumeric, -1.0f);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_ENTER ) {
				(void)__xgeXuiNumericInputCommitText(pNumeric, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			break;

		case XGE_EVENT_XUI_FOCUS_OUT:
			(void)__xgeXuiNumericInputCommitText(pNumeric, 1);
			break;
	}
	iRet = xgeXuiInputEvent(&pNumeric->tInput, pEvent);
	if ( (pNumeric->pContext != NULL) && (pNumeric->pContext->pFocus == pNumeric->pWidget) ) {
		pNumeric->iState |= XGE_XUI_STATE_FOCUS;
	} else {
		pNumeric->iState &= ~XGE_XUI_STATE_FOCUS;
	}
	return iRet;
}

int xgeXuiNumericInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiNumericInputEvent((xge_xui_numeric_input)pUser, pEvent);
}

void xgeXuiNumericInputUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser)
{
	xge_xui_numeric_input pNumeric;

	(void)pWidget;
	pNumeric = (xge_xui_numeric_input)pUser;
	if ( pNumeric != NULL ) {
		xgeXuiInputUpdateProc(pNumeric->pWidget, fDelta, &pNumeric->tInput);
	}
}

static void __xgeXuiNumericInputPaintButton(xge_xui_numeric_input pNumeric, int iButton, const uint16_t* arrIcon)
{
	xge_rect_t tRect;
	xge_rect_t tIcon;
	uint32_t iColor;
	uint32_t iIcon;
	int bEnabled;
	int bActive;

	tRect = __xgeXuiNumericInputButtonRect(pNumeric, iButton);
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		return;
	}
	bEnabled = __xgeXuiNumericInputButtonEnabled(pNumeric, iButton);
	bActive = (pNumeric->iActiveButton == iButton);
	if ( !bEnabled ) {
		iColor = pNumeric->iSpinnerDisabledColor;
		iIcon = pNumeric->iSpinnerDisabledIconColor;
	} else if ( bActive ) {
		iColor = pNumeric->iSpinnerActiveColor;
		iIcon = pNumeric->iSpinnerIconColor;
	} else if ( pNumeric->iHoverButton == iButton ) {
		iColor = pNumeric->iSpinnerHoverColor;
		iIcon = pNumeric->iSpinnerIconColor;
	} else {
		iColor = pNumeric->iSpinnerColor;
		iIcon = pNumeric->iSpinnerIconColor;
	}
	__xgeXuiHostDrawRect(tRect, iColor);
	if ( bActive ) {
		tRect.fY += 1.0f;
	}
	tIcon.fW = 8.0f;
	tIcon.fH = 8.0f;
	tIcon.fX = tRect.fX + (tRect.fW - tIcon.fW) * 0.5f;
	tIcon.fY = tRect.fY + (tRect.fH - tIcon.fH) * 0.5f;
	__xgeXuiHostDrawBitmapMask(tIcon, arrIcon, 8, 8, iIcon);
}

void xgeXuiNumericInputPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_numeric_input pNumeric;

	pNumeric = (xge_xui_numeric_input)pUser;
	if ( (pWidget == NULL) || (pNumeric == NULL) ) {
		return;
	}
	xgeXuiInputPaintProc(pWidget, &pNumeric->tInput);
}

static void __xgeXuiNumericInputPaintAfterProc(xge_xui_widget pWidget, void* pUser)
{
	static const uint16_t arrTriangleUp8[8] = {
		0x00, 0x18, 0x3c, 0x7e, 0xff, 0x00, 0x00, 0x00
	};
	static const uint16_t arrTriangleDown8[8] = {
		0x00, 0x00, 0x00, 0xff, 0x7e, 0x3c, 0x18, 0x00
	};
	xge_xui_numeric_input pNumeric;
	int bClipPushed;

	pNumeric = (xge_xui_numeric_input)pUser;
	if ( pNumeric == NULL && pWidget != NULL ) {
		pNumeric = (xge_xui_numeric_input)pWidget->pUser;
	}
	if ( (pWidget == NULL) || (pNumeric == NULL) ) {
		return;
	}
	if ( pNumeric->bShowSpinner == 0 ) {
		return;
	}
	bClipPushed = __xgeXuiPaintClipPush(pNumeric->pContext, pWidget->tBorderRect);
	__xgeXuiNumericInputPaintButton(pNumeric, XGE_XUI_NUMERIC_INPUT_BUTTON_UP, arrTriangleUp8);
	__xgeXuiNumericInputPaintButton(pNumeric, XGE_XUI_NUMERIC_INPUT_BUTTON_DOWN, arrTriangleDown8);
	if ( bClipPushed ) {
		__xgeXuiPaintClipPop(pNumeric->pContext);
	}
}
