static void __xgeXuiNumericInputSetValueInternal(xge_xui_numeric_input pNumeric, float fValue, int bNotify);

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

static void __xgeXuiNumericInputFormat(xge_xui_numeric_input pNumeric)
{
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
		snprintf(sValue, sizeof(sValue), "%.3f", pNumeric->fValue);
	}
	sValue[sizeof(sValue) - 1] = 0;
	xgeXuiInputSetText(&pNumeric->tInput, sValue);
}

static void __xgeXuiNumericInputSetError(xge_xui_numeric_input pNumeric, int bError)
{
	if ( pNumeric == NULL ) {
		return;
	}
	if ( pNumeric->bError != (bError != 0) ) {
		pNumeric->bError = (bError != 0);
		xgeXuiWidgetMarkPaint(pNumeric->pWidget);
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

static xge_rect_t __xgeXuiNumericInputSpinnerRect(xge_xui_numeric_input pNumeric, int bUp)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( (pNumeric == NULL) || (pNumeric->pWidget == NULL) || (pNumeric->bShowSpinner == 0) ) {
		return tRect;
	}
	tRect = pNumeric->pWidget->tContentRect;
	tRect.fW = (tRect.fH > 18.0f) ? 18.0f : tRect.fH;
	if ( tRect.fW < 12.0f ) {
		tRect.fW = 12.0f;
	}
	tRect.fX = pNumeric->pWidget->tContentRect.fX + pNumeric->pWidget->tContentRect.fW - tRect.fW;
	tRect.fH *= 0.5f;
	if ( bUp == 0 ) {
		tRect.fY += tRect.fH;
	}
	return tRect;
}

static void __xgeXuiNumericInputStep(xge_xui_numeric_input pNumeric, float fDirection)
{
	if ( pNumeric == NULL ) {
		return;
	}
	(void)__xgeXuiNumericInputCommitText(pNumeric, 0);
	if ( pNumeric->bError ) {
		return;
	}
	__xgeXuiNumericInputSetValueInternal(pNumeric, pNumeric->fValue + pNumeric->fStep * fDirection, 1);
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
	pNumeric->bShowSpinner = 1;
	xgeXuiWidgetSetEvent(pWidget, xgeXuiNumericInputEventProc, NULL);
	pWidget->procUpdate = xgeXuiNumericInputUpdateProc;
	pWidget->procPaint = xgeXuiNumericInputPaintProc;
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

void xgeXuiNumericInputSetSpinnerVisible(xge_xui_numeric_input pNumeric, int bVisible)
{
	if ( pNumeric == NULL ) {
		return;
	}
	pNumeric->bShowSpinner = (bVisible != 0);
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
	xge_rect_t tUp;
	xge_rect_t tDown;
	int iRet;

	if ( (pNumeric == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			tUp = __xgeXuiNumericInputSpinnerRect(pNumeric, 1);
			tDown = __xgeXuiNumericInputSpinnerRect(pNumeric, 0);
			if ( __xgeXuiRectContains(tUp, pEvent->fX, pEvent->fY) ) {
				xgeXuiSetFocus(pNumeric->pContext, pNumeric->pWidget);
				__xgeXuiNumericInputStep(pNumeric, 1.0f);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( __xgeXuiRectContains(tDown, pEvent->fX, pEvent->fY) ) {
				xgeXuiSetFocus(pNumeric->pContext, pNumeric->pWidget);
				__xgeXuiNumericInputStep(pNumeric, -1.0f);
				return XGE_XUI_EVENT_CONSUMED;
			}
			break;

		case XGE_EVENT_KEY_DOWN:
			if ( pEvent->iParam1 == XGE_KEY_UP ) {
				__xgeXuiNumericInputStep(pNumeric, 1.0f);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_DOWN ) {
				__xgeXuiNumericInputStep(pNumeric, -1.0f);
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

void xgeXuiNumericInputPaintProc(xge_xui_widget pWidget, void* pUser)
{
	static const uint16_t arrTriangleUp8[8] = {
		0x00, 0x18, 0x3c, 0x7e, 0xff, 0x00, 0x00, 0x00
	};
	static const uint16_t arrTriangleDown8[8] = {
		0x00, 0x00, 0x00, 0xff, 0x7e, 0x3c, 0x18, 0x00
	};
	xge_xui_numeric_input pNumeric;
	xge_rect_t tUp;
	xge_rect_t tDown;
	xge_rect_t tIcon;
	uint32_t iBorder;

	pNumeric = (xge_xui_numeric_input)pUser;
	if ( (pWidget == NULL) || (pNumeric == NULL) ) {
		return;
	}
	xgeXuiInputPaintProc(pWidget, &pNumeric->tInput);
	if ( pNumeric->bShowSpinner == 0 ) {
		return;
	}
	iBorder = XGE_COLOR_RGBA(184, 223, 245, 255);
	if ( pNumeric->bError ) {
		__xgeXuiHostDrawBorderRect(pWidget->tRect, 1.5f, XGE_COLOR_RGBA(220, 74, 84, 255));
	}
	tUp = __xgeXuiNumericInputSpinnerRect(pNumeric, 1);
	tDown = __xgeXuiNumericInputSpinnerRect(pNumeric, 0);
	__xgeXuiHostDrawRect(tUp, XGE_COLOR_RGBA(255, 255, 255, 255));
	__xgeXuiHostDrawBorderRect(tUp, 1.0f, iBorder);
	__xgeXuiHostDrawRect(tDown, XGE_COLOR_RGBA(255, 255, 255, 255));
	__xgeXuiHostDrawBorderRect(tDown, 1.0f, iBorder);
	tIcon.fW = 8.0f;
	tIcon.fH = 8.0f;
	tIcon.fX = tUp.fX + (tUp.fW - tIcon.fW) * 0.5f;
	tIcon.fY = tUp.fY + (tUp.fH - tIcon.fH) * 0.5f;
	__xgeXuiHostDrawBitmapMask(tIcon, arrTriangleUp8, 8, 8, pNumeric->tInput.iTextColor);
	tIcon.fX = tDown.fX + (tDown.fW - tIcon.fW) * 0.5f;
	tIcon.fY = tDown.fY + (tDown.fH - tIcon.fH) * 0.5f;
	__xgeXuiHostDrawBitmapMask(tIcon, arrTriangleDown8, 8, 8, pNumeric->tInput.iTextColor);
}
