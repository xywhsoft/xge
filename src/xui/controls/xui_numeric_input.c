#include "xui_config.h"
#if XUI_ENABLE_NUMERIC_INPUT
#include "xui_internal.h"

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#define XUI_NUMERIC_INPUT_MAX_TEXT	96
#define XUI_NUMERIC_INPUT_REPEAT_DELAY	0.40f
#define XUI_NUMERIC_INPUT_REPEAT_INTERVAL	0.08f

typedef struct xui_numeric_input_data_t {
	xui_widget pInput;
	xui_font pFont;
	xui_numeric_input_change_proc onChange;
	void* pChangeUser;
	xui_numeric_input_error_proc onError;
	void* pErrorUser;
	xui_numeric_input_format_proc onFormat;
	void* pFormatUser;
	float fMin;
	float fMax;
	float fValue;
	float fStep;
	float fSpinnerWidth;
	int iPrecision;
	int bInteger;
	int bReadonly;
	int bError;
	int bSpinnerVisible;
	int bPointerInside;
	int iHoverButton;
	int iActiveButton;
	int bPointerActive;
	uint64_t iActivePointerId;
	int iActivePointerType;
	int bRepeatFired;
	float fRepeatElapsed;
	int iChangeCount;
	uint32_t iTextColor;
	uint32_t iPlaceholderColor;
	uint32_t iDisabledTextColor;
	uint32_t iBackgroundColor;
	uint32_t iHoverBackgroundColor;
	uint32_t iDisabledBackgroundColor;
	uint32_t iBorderColor;
	uint32_t iHoverBorderColor;
	uint32_t iFocusBorderColor;
	uint32_t iDisabledBorderColor;
	uint32_t iErrorBackgroundColor;
	uint32_t iErrorBorderColor;
	uint32_t iSelectionColor;
	uint32_t iCursorColor;
	uint32_t iSpinnerColor;
	uint32_t iSpinnerHoverColor;
	uint32_t iSpinnerActiveColor;
	uint32_t iSpinnerBorderColor;
	uint32_t iSpinnerIconColor;
	uint32_t iSpinnerDisabledIconColor;
	uint32_t iChildStyleHash;
	float fBorderWidth;
	xui_rect_t tSpinnerRect;
	xui_rect_t tButtonUpRect;
	xui_rect_t tButtonDownRect;
} xui_numeric_input_data_t;

static int __xuiNumericInputFloatValid(float fValue)
{
	return isfinite(fValue);
}

static int __xuiNumericInputMetricValid(float fValue)
{
	return isfinite(fValue) && fValue >= 0.0f && fValue <= XUI_LAYOUT_UNBOUNDED;
}

static int __xuiNumericInputDescValid(const xui_numeric_input_desc_t* pDesc)
{
	float fMin;
	float fMax;
	float fSwap;

	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( (pDesc->iPrecision < 0) ||
	     !__xuiNumericInputFloatValid(pDesc->fMin) ||
	     !__xuiNumericInputFloatValid(pDesc->fMax) ||
	     !__xuiNumericInputFloatValid(pDesc->fValue) ||
	     !__xuiNumericInputFloatValid(pDesc->fStep) ||
	     !__xuiNumericInputMetricValid(pDesc->fSpinnerWidth) ||
	     !__xuiNumericInputMetricValid(pDesc->fBorderWidth) ||
	     (pDesc->fStep < 0.0f) ) {
		return 0;
	}
	fMin = pDesc->fMin;
	fMax = pDesc->fMax;
	if ( (fMin == 0.0f) && (fMax == 0.0f) ) fMax = 100.0f;
	if ( fMax < fMin ) {
		fSwap = fMin;
		fMin = fMax;
		fMax = fSwap;
	}
	if ( pDesc->bInteger && ceilf(fMin) > floorf(fMax) ) return 0;
	return 1;
}

static float __xuiNumericInputMinFloat(float fA, float fB)
{
	return (fA < fB) ? fA : fB;
}

static float __xuiNumericInputMaxFloat(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static float __xuiNumericInputAbsFloat(float fValue)
{
	return (fValue < 0.0f) ? -fValue : fValue;
}

static float __xuiNumericInputClampFloat(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) return fMin;
	if ( fValue > fMax ) return fMax;
	return fValue;
}

static float __xuiNumericInputEffectiveMin(const xui_numeric_input_data_t* pData)
{
	return (pData != NULL && pData->bInteger) ? ceilf(pData->fMin) : ((pData != NULL) ? pData->fMin : 0.0f);
}

static float __xuiNumericInputEffectiveMax(const xui_numeric_input_data_t* pData)
{
	return (pData != NULL && pData->bInteger) ? floorf(pData->fMax) : ((pData != NULL) ? pData->fMax : 0.0f);
}

static float __xuiNumericInputRoundFloat(float fValue)
{
	return roundf(fValue);
}

static int __xuiNumericInputAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static uint32_t __xuiNumericInputColorWithAlpha(uint32_t iColor, uint32_t iAlpha)
{
	return (iColor & 0xffffff00u) | (iAlpha & 0xffu);
}

static void __xuiNumericInputNormalizeRange(float* pMin, float* pMax)
{
	float fSwap;

	if ( (*pMax) < (*pMin) ) {
		fSwap = *pMin;
		*pMin = *pMax;
		*pMax = fSwap;
	}
}

static int __xuiNumericInputPrecision(int iPrecision)
{
	if ( iPrecision < 0 ) return 0;
	if ( iPrecision > 9 ) return 9;
	return iPrecision;
}

static void __xuiNumericInputChange(xui_numeric_input_data_t* pData)
{
	if ( pData != NULL && pData->iChangeCount < INT_MAX ) pData->iChangeCount++;
}

static int __xuiNumericInputRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) &&
	       (fY >= tRect.fY) &&
	       (fX < tRect.fX + tRect.fW) &&
	       (fY < tRect.fY + tRect.fH);
}

static int __xuiNumericInputContainsWidget(xui_widget pNumeric, xui_widget pWidget)
{
	for ( ; pWidget != NULL; pWidget = xuiWidgetGetParent(pWidget) ) {
		if ( pWidget == pNumeric ) return 1;
	}
	return 0;
}

static xui_numeric_input_data_t* __xuiNumericInputGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return NULL;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "numeric_input");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_numeric_input_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiNumericInputAlive(xui_context pContext, xui_widget pWidget,
	const xui_numeric_input_data_t* pExpected)
{
	return xuiInternalContextIsValid(pContext) && !xuiInternalContextDestroyPending(pContext) &&
		xuiInternalWidgetIsValid(pWidget) && __xuiNumericInputGetData(pWidget) == pExpected;
}

static int __xuiNumericInputPointerMatches(const xui_numeric_input_data_t* pData,
	const xui_event_t* pEvent)
{
	return (pData != NULL) && (pEvent != NULL) && pData->bPointerActive &&
		(pData->iActivePointerId == pEvent->iPointerId) &&
		(pData->iActivePointerType == pEvent->iPointerType);
}

static int __xuiNumericInputStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR) ) {
		*pColor = tProperty.tValue.iColor;
		return 1;
	}
	return 0;
}

static int __xuiNumericInputStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     isfinite(tProperty.tValue.fFloat) && (tProperty.tValue.fFloat >= 0.0f) &&
	     (tProperty.tValue.fFloat <= XUI_LAYOUT_UNBOUNDED) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static xui_font __xuiNumericInputStyleFont(xui_widget pWidget, xui_font pBaseFont)
{
	xui_style_property_t tProperty;
	xui_font pFont;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, "font.name", &tProperty);
	if ( (iRet == XUI_OK) &&
	     (tProperty.tValue.iType == XUI_STYLE_VALUE_STRING) &&
	     (tProperty.tValue.sText != NULL) ) {
		pFont = xuiFindFont(xuiWidgetGetContext(pWidget), tProperty.tValue.sText);
		if ( pFont != NULL ) {
			return pFont;
		}
	}
	return pBaseFont;
}

static void __xuiNumericInputResolve(xui_widget pWidget, const xui_numeric_input_data_t* pData, xui_numeric_input_data_t* pResolved)
{
	*pResolved = *pData;
	pResolved->pFont = (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.text.color", &pResolved->iTextColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.text.disabled_color", &pResolved->iDisabledTextColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.placeholder.color", &pResolved->iPlaceholderColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.selection.color", &pResolved->iSelectionColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.cursor.color", &pResolved->iCursorColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.background.color", &pResolved->iBackgroundColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.background.hover_color", &pResolved->iHoverBackgroundColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.background.disabled_color", &pResolved->iDisabledBackgroundColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.border.color", &pResolved->iBorderColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.border.hover_color", &pResolved->iHoverBorderColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.border.focus_color", &pResolved->iFocusBorderColor);
	pResolved->iDisabledBorderColor = (__xuiNumericInputAlpha(pResolved->iBorderColor) > 120) ?
		__xuiNumericInputColorWithAlpha(pResolved->iBorderColor, 120) : pResolved->iBorderColor;
	(void)__xuiNumericInputStyleColor(pWidget, "input.border.disabled_color", &pResolved->iDisabledBorderColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.border.disabled_color", &pResolved->iDisabledBorderColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.error.background_color", &pResolved->iErrorBackgroundColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.error.border_color", &pResolved->iErrorBorderColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.spinner.color", &pResolved->iSpinnerColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.spinner.hover_color", &pResolved->iSpinnerHoverColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.spinner.active_color", &pResolved->iSpinnerActiveColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.spinner.border_color", &pResolved->iSpinnerBorderColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.spinner.icon_color", &pResolved->iSpinnerIconColor);
	(void)__xuiNumericInputStyleColor(pWidget, "numeric_input.spinner.icon_disabled_color", &pResolved->iSpinnerDisabledIconColor);
	(void)__xuiNumericInputStyleFloat(pWidget, "numeric_input.border.width", &pResolved->fBorderWidth);
	(void)__xuiNumericInputStyleFloat(pWidget, "numeric_input.spinner.width", &pResolved->fSpinnerWidth);
	pResolved->pFont = __xuiNumericInputStyleFont(pWidget, pResolved->pFont);
	if ( pResolved->fBorderWidth < 0.0f ) pResolved->fBorderWidth = 0.0f;
	if ( pResolved->fSpinnerWidth < 0.0f ) pResolved->fSpinnerWidth = 0.0f;
}

static int __xuiNumericInputDrawRectFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( (__xuiNumericInputAlpha(iColor) == 0) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	tRect = xuiInternalSnapRect(tRect);
	return (pProxy->drawRectFill != NULL) ? pProxy->drawRectFill(pProxy, pDraw, tRect, iColor) : XUI_OK;
}

static int __xuiNumericInputDrawRectStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (fWidth <= 0.0f) || (__xuiNumericInputAlpha(iColor) == 0) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	tRect = xuiInternalStrokeCenterRectInside(tRect, fWidth, NULL);
	return (pProxy->drawRectStroke != NULL) ? pProxy->drawRectStroke(pProxy, pDraw, tRect, xuiInternalSnapSize(fWidth), iColor) : XUI_OK;
}

static int __xuiNumericInputDrawLine(xui_proxy pProxy, xui_draw_context pDraw, float fX0, float fY0, float fX1, float fY1, uint32_t iColor)
{
	if ( (__xuiNumericInputAlpha(iColor) == 0) || (pProxy->drawLine == NULL) ) {
		return XUI_OK;
	}
	return pProxy->drawLine(pProxy, pDraw,
		xuiInternalSnapPixel(fX0), xuiInternalSnapPixel(fY0),
		xuiInternalSnapPixel(fX1), xuiInternalSnapPixel(fY1),
		1.0f, iColor);
}

static int __xuiNumericInputDrawArrow(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, int iUp, uint32_t iColor)
{
	xui_vec2_t tA;
	xui_vec2_t tB;
	xui_vec2_t tC;
	float fCX;
	float fCY;
	float fW;
	float fH;

	if ( (__xuiNumericInputAlpha(iColor) == 0) || (pProxy->drawTriangleFill == NULL) ) {
		return XUI_OK;
	}
	fCX = tRect.fX + tRect.fW * 0.5f;
	fCY = tRect.fY + tRect.fH * 0.5f;
	fW = __xuiNumericInputMinFloat(tRect.fW * 0.32f, 6.0f);
	fH = __xuiNumericInputMinFloat(tRect.fH * 0.28f, 4.0f);
	if ( fW < 3.0f ) fW = 3.0f;
	if ( fH < 2.0f ) fH = 2.0f;
	if ( iUp ) {
		tA = (xui_vec2_t){xuiInternalSnapPixel(fCX), xuiInternalSnapPixel(fCY - fH * 0.55f)};
		tB = (xui_vec2_t){xuiInternalSnapPixel(fCX - fW), xuiInternalSnapPixel(fCY + fH * 0.55f)};
		tC = (xui_vec2_t){xuiInternalSnapPixel(fCX + fW), xuiInternalSnapPixel(fCY + fH * 0.55f)};
	} else {
		tA = (xui_vec2_t){xuiInternalSnapPixel(fCX - fW), xuiInternalSnapPixel(fCY - fH * 0.55f)};
		tB = (xui_vec2_t){xuiInternalSnapPixel(fCX + fW), xuiInternalSnapPixel(fCY - fH * 0.55f)};
		tC = (xui_vec2_t){xuiInternalSnapPixel(fCX), xuiInternalSnapPixel(fCY + fH * 0.55f)};
	}
	return pProxy->drawTriangleFill(pProxy, pDraw, tA, tB, tC, iColor);
}

static int __xuiNumericInputFormatValue(xui_widget pWidget, xui_numeric_input_data_t* pData, char* sBuffer, int iCapacity)
{
	char sFormat[4];
	xui_numeric_input_format_proc onFormat;
	xui_context pContext;
	size_t iOutputSize;
	void* pFormatUser;
	float fValue;
	int bInteger;
	int iPrecision;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (sBuffer == NULL) || (iCapacity <= 0) ) {
		return 0;
	}
	sBuffer[0] = '\0';
	pContext = xuiWidgetGetContext(pWidget);
	onFormat = pData->onFormat;
	pFormatUser = pData->pFormatUser;
	fValue = pData->fValue;
	bInteger = pData->bInteger;
	iPrecision = pData->iPrecision;
	if ( onFormat != NULL ) {
		iRet = onFormat(fValue, sBuffer, iCapacity, pFormatUser);
		if ( !__xuiNumericInputAlive(pContext, pWidget, pData) ) return 0;
		if ( iRet > 0 ) {
			sBuffer[iCapacity - 1] = '\0';
			return 1;
		}
	}
	sFormat[0] = '.';
	sFormat[1] = (char)('0' + (bInteger ? 0 : iPrecision));
	sFormat[2] = 'f';
	sFormat[3] = '\0';
	iOutputSize = 0u;
	if ( !xrtNumFormatTo((double)(bInteger ? __xuiNumericInputRoundFloat(fValue) : fValue),
	     xuiXrtText(sFormat, 3u), sBuffer, (size_t)iCapacity, &iOutputSize) ) {
		sBuffer[0] = '\0';
		return 0;
	}
	return 1;
}

static int __xuiNumericInputSetInputTextFromValue(xui_widget pWidget, xui_numeric_input_data_t* pData)
{
	char sBuffer[XUI_NUMERIC_INPUT_MAX_TEXT];

	if ( (pWidget == NULL) || (pData == NULL) || (pData->pInput == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xuiNumericInputFormatValue(pWidget, pData, sBuffer, (int)sizeof(sBuffer)) ) return XUI_OK;
	return xuiInputSetText(pData->pInput, sBuffer);
}

static int __xuiNumericInputSyncInputStyle(xui_widget pWidget, xui_numeric_input_data_t* pData)
{
	xui_numeric_input_data_t tResolved;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->pInput == NULL) ) {
		return XUI_OK;
	}
	__xuiNumericInputResolve(pWidget, pData, &tResolved);
	iRet = xuiInputSetColors(pData->pInput, XUI_COLOR_RGBA(0, 0, 0, 0), tResolved.iTextColor,
		XUI_COLOR_RGBA(0, 0, 0, 0), XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( iRet == XUI_OK ) iRet = xuiInputSetErrorColors(pData->pInput,
		XUI_COLOR_RGBA(0, 0, 0, 0), XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( iRet == XUI_OK ) iRet = xuiInputSetExtendedColors(pData->pInput,
		tResolved.iPlaceholderColor,
		tResolved.iDisabledTextColor,
		XUI_COLOR_RGBA(0, 0, 0, 0),
		XUI_COLOR_RGBA(0, 0, 0, 0),
		XUI_COLOR_RGBA(0, 0, 0, 0),
		tResolved.iSelectionColor,
		tResolved.iCursorColor);
	if ( iRet == XUI_OK ) iRet = xuiInputSetBorderWidth(pData->pInput, 0.0f);
	if ( iRet == XUI_OK ) iRet = xuiInputSetReadonly(pData->pInput, tResolved.bReadonly);
	if ( iRet == XUI_OK ) iRet = xuiInputSetError(pData->pInput, tResolved.bError);
	if ( iRet == XUI_OK ) iRet = xuiInputSetTextAlign(pData->pInput, XUI_INPUT_ALIGN_RIGHT);
	if ( iRet == XUI_OK && xuiInputGetFont(pData->pInput) != tResolved.pFont ) {
		iRet = xuiInputSetFont(pData->pInput, tResolved.pFont);
	}
	return iRet;
}

static int __xuiNumericInputSetErrorInternal(xui_widget pWidget, xui_numeric_input_data_t* pData, int bError)
{
	xui_numeric_input_error_proc onError;
	void* pErrorUser;
	int iRet;

	bError = bError ? 1 : 0;
	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->bError != bError ) {
		if ( pData->pInput != NULL ) {
			iRet = xuiInputSetError(pData->pInput, bError);
			if ( iRet != XUI_OK ) return iRet;
		}
		pData->bError = bError;
		onError = pData->onError;
		pErrorUser = pData->pErrorUser;
		iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
		if ( onError != NULL ) onError(pWidget, bError, pErrorUser);
	}
	return XUI_OK;
}

static int __xuiNumericInputSetValueInternal(xui_widget pWidget, xui_numeric_input_data_t* pData, float fValue, int bNotify)
{
	int bChanged;
	xui_numeric_input_change_proc onChange;
	xui_context pContext;
	void* pChangeUser;
	float fOldValue;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || !__xuiNumericInputFloatValid(fValue) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	if ( pData->bInteger ) {
		fValue = __xuiNumericInputRoundFloat(fValue);
		fValue = __xuiNumericInputClampFloat(fValue,
			__xuiNumericInputEffectiveMin(pData), __xuiNumericInputEffectiveMax(pData));
	} else {
		fValue = __xuiNumericInputClampFloat(fValue, pData->fMin, pData->fMax);
	}
	bChanged = (pData->fValue != fValue);
	fOldValue = pData->fValue;
	pData->fValue = fValue;
	iRet = __xuiNumericInputSetErrorInternal(pWidget, pData, 0);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiNumericInputAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = __xuiNumericInputSetInputTextFromValue(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->fValue = fOldValue;
		return iRet;
	}
	if ( !__xuiNumericInputAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( bChanged ) {
		__xuiNumericInputChange(pData);
	}
	onChange = pData->onChange;
	pChangeUser = pData->pChangeUser;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	if ( bChanged ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	if ( bChanged && bNotify && onChange != NULL ) onChange(pWidget, fValue, pChangeUser);
	return XUI_OK;
}

static int __xuiNumericInputDecimalSyntax(const char* sText)
{
	const char* p;
	int iDigits;

	if ( sText == NULL ) return 0;
	p = sText;
	while ( isspace((unsigned char)*p) ) p++;
	if ( *p == '+' || *p == '-' ) p++;
	iDigits = 0;
	while ( isdigit((unsigned char)*p) ) {
		p++;
		iDigits++;
	}
	if ( *p == '.' ) {
		p++;
		while ( isdigit((unsigned char)*p) ) {
			p++;
			iDigits++;
		}
	}
	if ( iDigits == 0 ) return 0;
	if ( *p == 'e' || *p == 'E' ) {
		int iExponentDigits = 0;
		p++;
		if ( *p == '+' || *p == '-' ) p++;
		while ( isdigit((unsigned char)*p) ) {
			p++;
			iExponentDigits++;
		}
		if ( iExponentDigits == 0 ) return 0;
	}
	while ( isspace((unsigned char)*p) ) p++;
	return *p == '\0';
}

static int __xuiNumericInputParseText(const char* sText, float* pValue)
{
	const char* sEnd;
	const char* sStart;
	double fValue;

	if ( (sText == NULL) || (pValue == NULL) ) {
		return 0;
	}
	if ( !__xuiNumericInputDecimalSyntax(sText) ) return 0;
	sStart = sText;
	while ( isspace((unsigned char)*sStart) ) sStart++;
	sEnd = sText + strlen(sText);
	while ( sEnd > sStart && isspace((unsigned char)sEnd[-1]) ) sEnd--;
	if ( sEnd == sStart || !xrtNumParse(xuiXrtText(sStart, (size_t)(sEnd - sStart)), 0, &fValue) ||
	     !isfinite(fValue) || fValue > (double)FLT_MAX || fValue < -(double)FLT_MAX ) {
		return 0;
	}
	*pValue = (float)fValue;
	return isfinite(*pValue) ? 1 : 0;
}

static int __xuiNumericInputCommitInternal(xui_widget pWidget, xui_numeric_input_data_t* pData, int bNotify)
{
	const char* sText;
	float fValue;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->pInput == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	sText = xuiInputGetText(pData->pInput);
	if ( !__xuiNumericInputParseText(sText, &fValue) ) {
		return __xuiNumericInputSetErrorInternal(pWidget, pData, 1);
	}
	return __xuiNumericInputSetValueInternal(pWidget, pData, fValue, bNotify);
}

static int __xuiNumericInputCanUserChange(xui_widget pWidget, const xui_numeric_input_data_t* pData)
{
	return (pWidget != NULL) &&
	       (pData != NULL) &&
	       xuiWidgetGetEffectiveEnabled(pWidget) &&
	       xuiWidgetGetEffectiveVisible(pWidget) &&
	       !pData->bReadonly;
}

static void __xuiNumericInputUpdateRects(xui_widget pWidget, xui_numeric_input_data_t* pData, const xui_numeric_input_data_t* pResolved, xui_rect_t* pInputRect)
{
	xui_rect_t tContent;
	xui_rect_t tInput;
	float fSpinnerW;
	float fHalf;

	if ( (pWidget == NULL) || (pData == NULL) || (pResolved == NULL) ) {
		return;
	}
	tContent = xuiWidgetGetContentRect(pWidget);
	fSpinnerW = (pResolved->bSpinnerVisible != 0) ? pResolved->fSpinnerWidth : 0.0f;
	if ( fSpinnerW < 0.0f ) fSpinnerW = 0.0f;
	if ( fSpinnerW > tContent.fW ) fSpinnerW = tContent.fW;
	pData->tSpinnerRect = (xui_rect_t){tContent.fX + tContent.fW - fSpinnerW, tContent.fY, fSpinnerW, tContent.fH};
	fHalf = pData->tSpinnerRect.fH * 0.5f;
	pData->tButtonUpRect = (xui_rect_t){pData->tSpinnerRect.fX, pData->tSpinnerRect.fY, pData->tSpinnerRect.fW, fHalf};
	pData->tButtonDownRect = (xui_rect_t){pData->tSpinnerRect.fX, pData->tSpinnerRect.fY + fHalf, pData->tSpinnerRect.fW, pData->tSpinnerRect.fH - fHalf};
	tInput = tContent;
	tInput.fW -= fSpinnerW;
	if ( tInput.fW < 0.0f ) tInput.fW = 0.0f;
	tInput = xuiInternalSnapRect(tInput);
	pData->tSpinnerRect = xuiInternalSnapRect(pData->tSpinnerRect);
	pData->tButtonUpRect = xuiInternalSnapRect(pData->tButtonUpRect);
	pData->tButtonDownRect = xuiInternalSnapRect(pData->tButtonDownRect);
	if ( pInputRect != NULL ) {
		*pInputRect = tInput;
	}
}

static int __xuiNumericInputButtonAtLocal(xui_numeric_input_data_t* pData, float fLocalX, float fLocalY)
{
	if ( (pData == NULL) || !pData->bSpinnerVisible || (pData->tSpinnerRect.fW <= 0.0f) ) {
		return XUI_NUMERIC_INPUT_BUTTON_NONE;
	}
	if ( __xuiNumericInputRectContains(pData->tButtonUpRect, fLocalX, fLocalY) ) {
		return XUI_NUMERIC_INPUT_BUTTON_UP;
	}
	if ( __xuiNumericInputRectContains(pData->tButtonDownRect, fLocalX, fLocalY) ) {
		return XUI_NUMERIC_INPUT_BUTTON_DOWN;
	}
	return XUI_NUMERIC_INPUT_BUTTON_NONE;
}

static int __xuiNumericInputEventButton(xui_widget pWidget, xui_numeric_input_data_t* pData, const xui_event_t* pEvent)
{
	xui_numeric_input_data_t tResolved;
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;

	if ( (pWidget == NULL) || (pData == NULL) || (pEvent == NULL) ) {
		return XUI_NUMERIC_INPUT_BUTTON_NONE;
	}
	__xuiNumericInputResolve(pWidget, pData, &tResolved);
	__xuiNumericInputUpdateRects(pWidget, pData, &tResolved, NULL);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	fLocalX = pEvent->fX - tWorld.fX;
	fLocalY = pEvent->fY - tWorld.fY;
	pData->bPointerInside = __xuiNumericInputRectContains(xuiWidgetGetContentRect(pWidget), fLocalX, fLocalY);
	return __xuiNumericInputButtonAtLocal(pData, fLocalX, fLocalY);
}

static int __xuiNumericInputButtonEnabledData(const xui_numeric_input_data_t* pData, int iButton)
{
	float fMin;
	float fMax;

	if ( (pData == NULL) || !pData->bSpinnerVisible ) {
		return 0;
	}
	fMin = __xuiNumericInputEffectiveMin(pData);
	fMax = __xuiNumericInputEffectiveMax(pData);
	if ( iButton == XUI_NUMERIC_INPUT_BUTTON_UP ) {
		return pData->fValue < fMax;
	}
	if ( iButton == XUI_NUMERIC_INPUT_BUTTON_DOWN ) {
		return pData->fValue > fMin;
	}
	return 0;
}

static int __xuiNumericInputStepInternal(xui_widget pWidget, xui_numeric_input_data_t* pData, int iDirection, int bNotify)
{
	xui_context pContext;
	double fNext;
	float fMin;
	float fMax;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xuiNumericInputCanUserChange(pWidget, pData) || (iDirection == 0) ) {
		return XUI_OK;
	}
	pContext = xuiWidgetGetContext(pWidget);
	iRet = __xuiNumericInputCommitInternal(pWidget, pData, 0);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiNumericInputAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( pData->bError ) {
		return XUI_OK;
	}
	fMin = __xuiNumericInputEffectiveMin(pData);
	fMax = __xuiNumericInputEffectiveMax(pData);
	fNext = (double)pData->fValue + ((iDirection > 0) ? (double)pData->fStep : -(double)pData->fStep);
	if ( fNext <= (double)fMin ) return __xuiNumericInputSetValueInternal(pWidget, pData, fMin, bNotify);
	if ( fNext >= (double)fMax ) return __xuiNumericInputSetValueInternal(pWidget, pData, fMax, bNotify);
	return __xuiNumericInputSetValueInternal(pWidget, pData, (float)fNext, bNotify);
}

static int __xuiNumericInputCancelPointer(xui_widget pWidget,
	xui_numeric_input_data_t* pData, int bInvalidate)
{
	xui_context pContext;
	uint64_t iPointerId;
	int iPointerType;
	int bPointerActive;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	iPointerId = pData->iActivePointerId;
	iPointerType = pData->iActivePointerType;
	bPointerActive = pData->bPointerActive;
	pData->iActiveButton = XUI_NUMERIC_INPUT_BUTTON_NONE;
	pData->bPointerActive = 0;
	pData->iActivePointerId = 0;
	pData->iActivePointerType = XUI_POINTER_TYPE_MOUSE;
	pData->bRepeatFired = 0;
	pData->fRepeatElapsed = 0.0f;
	if ( bPointerActive && xuiInternalContextIsValid(pContext) &&
	     xuiGetPointerCaptureEx(pContext, iPointerId, iPointerType) == pWidget ) {
		iRet = xuiReleasePointerCaptureEx(pContext, iPointerId, iPointerType, pWidget);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( bInvalidate && __xuiNumericInputAlive(pContext, pWidget, pData) ) {
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_OK;
}

static void __xuiNumericInputCancelAction(xui_widget pWidget, void* pUser)
{
	xui_numeric_input_data_t* pData;

	(void)pUser;
	pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL ) {
		return;
	}
	(void)__xuiNumericInputCancelPointer(pWidget, pData, 1);
}

static int __xuiNumericInputPointerDown(xui_widget pWidget, xui_numeric_input_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	int iButton;
	int iRet;

	if ( (pEvent->iButton != XUI_POINTER_BUTTON_LEFT) || (pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE) ) {
		return XUI_OK;
	}
	if ( pData->bPointerActive ) {
		return __xuiNumericInputPointerMatches(pData, pEvent) ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
	}
	iButton = __xuiNumericInputEventButton(pWidget, pData, pEvent);
	if ( iButton == XUI_NUMERIC_INPUT_BUTTON_NONE ) {
		pData->iHoverButton = XUI_NUMERIC_INPUT_BUTTON_NONE;
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	pContext = xuiWidgetGetContext(pWidget);
	if ( pData->pInput != NULL ) {
		iRet = xuiSetFocusWidget(pContext, pData->pInput);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiNumericInputAlive(pContext, pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
	}
	pData->iHoverButton = iButton;
	if ( __xuiNumericInputCanUserChange(pWidget, pData) && __xuiNumericInputButtonEnabledData(pData, iButton) ) {
		iRet = xuiSetPointerCaptureEx(pContext, pEvent->iPointerId, pEvent->iPointerType, pWidget);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiNumericInputAlive(pContext, pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
		pData->iActiveButton = iButton;
		pData->bPointerActive = 1;
		pData->iActivePointerId = pEvent->iPointerId;
		pData->iActivePointerType = pEvent->iPointerType;
		pData->bRepeatFired = 0;
		pData->fRepeatElapsed = 0.0f;
	}
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		(void)__xuiNumericInputCancelPointer(pWidget, pData, 0);
		return iRet;
	}
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiNumericInputPointerMove(xui_widget pWidget, xui_numeric_input_data_t* pData, const xui_event_t* pEvent)
{
	int iButton;
	int bOldInside;

	if ( pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE ) {
		return XUI_OK;
	}
	if ( pData->bPointerActive && !__xuiNumericInputPointerMatches(pData, pEvent) ) {
		return XUI_OK;
	}
	bOldInside = pData->bPointerInside;
	iButton = __xuiNumericInputEventButton(pWidget, pData, pEvent);
	if ( pData->iHoverButton != iButton || bOldInside != pData->bPointerInside ) {
		pData->iHoverButton = iButton;
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_OK;
}

static int __xuiNumericInputPointerUp(xui_widget pWidget, xui_numeric_input_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	int iButton;
	int iActive;
	int bRepeatFired;
	int iRet;

	if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) {
		return XUI_OK;
	}
	if ( !__xuiNumericInputPointerMatches(pData, pEvent) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	iActive = pData->iActiveButton;
	bRepeatFired = pData->bRepeatFired;
	iButton = __xuiNumericInputEventButton(pWidget, pData, pEvent);
	iRet = __xuiNumericInputCancelPointer(pWidget, pData, 0);
	if ( iRet != XUI_OK ) return iRet;
	if ( !bRepeatFired && (iActive != XUI_NUMERIC_INPUT_BUTTON_NONE) &&
	     (iActive == iButton) &&
	     __xuiNumericInputButtonEnabledData(pData, iActive) ) {
		iRet = __xuiNumericInputStepInternal(pWidget, pData, (iActive == XUI_NUMERIC_INPUT_BUTTON_UP) ? 1 : -1, 1);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiNumericInputAlive(pContext, pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
	}
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	return (iActive != XUI_NUMERIC_INPUT_BUTTON_NONE) ? XUI_EVENT_DISPATCH_STOP : XUI_OK;
}

static int __xuiNumericInputUpdate(xui_widget pWidget, float fDelta, void* pUser)
{
	xui_numeric_input_data_t* pData;
	xui_context pContext;
	int iDirection;
	int iSteps;
	int iRet;

	(void)pUser;
	pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !pData->bPointerActive ||
	     (pData->iActiveButton == XUI_NUMERIC_INPUT_BUTTON_NONE) ||
	     (xuiGetPointerCaptureEx(pContext, pData->iActivePointerId,
	        pData->iActivePointerType) != pWidget) ) {
		return XUI_OK;
	}
	if ( !__xuiNumericInputCanUserChange(pWidget, pData) ||
	     pData->iHoverButton != pData->iActiveButton ||
	     !__xuiNumericInputButtonEnabledData(pData, pData->iActiveButton) ) {
		pData->bRepeatFired = 0;
		pData->fRepeatElapsed = 0.0f;
		return XUI_OK;
	}
	if ( !isfinite(fDelta) || fDelta <= 0.0f ) return XUI_OK;
	pData->fRepeatElapsed += fDelta;
	if ( !pData->bRepeatFired ) {
		if ( pData->fRepeatElapsed < XUI_NUMERIC_INPUT_REPEAT_DELAY ) return XUI_OK;
		pData->fRepeatElapsed -= XUI_NUMERIC_INPUT_REPEAT_DELAY;
		pData->bRepeatFired = 1;
		iDirection = (pData->iActiveButton == XUI_NUMERIC_INPUT_BUTTON_UP) ? 1 : -1;
		iRet = __xuiNumericInputStepInternal(pWidget, pData, iDirection, 1);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiNumericInputAlive(pContext, pWidget, pData) ) return XUI_OK;
		if ( !pData->bPointerActive ||
		     !__xuiNumericInputCanUserChange(pWidget, pData) ||
		     pData->iHoverButton != pData->iActiveButton ||
		     !__xuiNumericInputButtonEnabledData(pData, pData->iActiveButton) ) {
			pData->fRepeatElapsed = 0.0f;
			return XUI_OK;
		}
	}
	if ( pData->fRepeatElapsed < XUI_NUMERIC_INPUT_REPEAT_INTERVAL ) return XUI_OK;
	iDirection = (pData->iActiveButton == XUI_NUMERIC_INPUT_BUTTON_UP) ? 1 : -1;
	iSteps = 0;
	while ( pData->fRepeatElapsed >= XUI_NUMERIC_INPUT_REPEAT_INTERVAL && iSteps < 8 ) {
		iRet = __xuiNumericInputStepInternal(pWidget, pData, iDirection, 1);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiNumericInputAlive(pContext, pWidget, pData) ) return XUI_OK;
		iSteps++;
		pData->fRepeatElapsed -= XUI_NUMERIC_INPUT_REPEAT_INTERVAL;
		if ( !pData->bPointerActive ||
		     !__xuiNumericInputCanUserChange(pWidget, pData) ||
		     pData->iHoverButton != pData->iActiveButton ||
		     !__xuiNumericInputButtonEnabledData(pData, pData->iActiveButton) ) {
			pData->fRepeatElapsed = 0.0f;
			break;
		}
	}
	if ( iSteps == 8 && pData->fRepeatElapsed >= XUI_NUMERIC_INPUT_REPEAT_INTERVAL ) {
		pData->fRepeatElapsed = fmodf(pData->fRepeatElapsed, XUI_NUMERIC_INPUT_REPEAT_INTERVAL);
	}
	return XUI_OK;
}

static int __xuiNumericInputWheel(xui_widget pWidget, xui_numeric_input_data_t* pData, const xui_event_t* pEvent)
{
	float fDelta;
	int iRet;

	if ( !__xuiNumericInputCanUserChange(pWidget, pData) || (pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE) ) {
		return XUI_OK;
	}
	fDelta = (__xuiNumericInputAbsFloat(pEvent->fWheelY) >= __xuiNumericInputAbsFloat(pEvent->fWheelX)) ? pEvent->fWheelY : pEvent->fWheelX;
	if ( !isfinite(fDelta) || fDelta == 0.0f ) {
		return XUI_OK;
	}
	iRet = __xuiNumericInputStepInternal(pWidget, pData, (fDelta > 0.0f) ? 1 : -1, 1);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiNumericInputPrepareEdit(xui_widget pWidget, xui_widget pSource,
	int iType, int* pSkipDefault, void* pUser)
{
	char sBuffer[XUI_NUMERIC_INPUT_MAX_TEXT];
	xui_numeric_input_data_t* pData;
	const xui_edit_behavior_t* pBehavior;
	xui_context pContext;
	int iRet;

	pData = (xui_numeric_input_data_t*)pUser;
	if ( pData == NULL || pSource != pData->pInput || __xuiNumericInputGetData(pWidget) != pData ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	if ( iType == XUI_EDIT_EVENT_COMMIT ) {
		return __xuiNumericInputCommitInternal(pWidget, pData, 1);
	}
	if ( iType != XUI_EDIT_EVENT_CANCEL ) return XUI_OK;
	iRet = __xuiNumericInputCancelPointer(pWidget, pData, 1);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiNumericInputAlive(pContext, pWidget, pData) ) return XUI_OK;
	pBehavior = xuiInternalEditBehavior(pSource);
	if ( pBehavior == NULL || pBehavior->iEscapeBehavior != XUI_EDIT_ESCAPE_REVERT ) return XUI_OK;
	iRet = __xuiNumericInputSetErrorInternal(pWidget, pData, 0);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiNumericInputAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( !__xuiNumericInputFormatValue(pWidget, pData, sBuffer, (int)sizeof(sBuffer)) ) return XUI_OK;
	if ( !__xuiNumericInputAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiEditSetText(pSource, sBuffer);
	if ( iRet != XUI_OK ) return iRet;
	if ( pSkipDefault != NULL ) *pSkipDefault = 1;
	return XUI_OK;
}

static int __xuiNumericInputKeyDown(xui_widget pWidget, xui_numeric_input_data_t* pData, const xui_event_t* pEvent)
{
	int iRet;

	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) {
		return XUI_OK;
	}
	switch ( pEvent->iKey ) {
	case XUI_KEY_UP:
		iRet = __xuiNumericInputStepInternal(pWidget, pData, 1, 1);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_DOWN:
		iRet = __xuiNumericInputStepInternal(pWidget, pData, -1, 1);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_ENTER:
		iRet = __xuiNumericInputCommitInternal(pWidget, pData, 1);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_ESCAPE:
		(void)__xuiNumericInputCancelPointer(pWidget, pData, 1);
		return XUI_OK;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiNumericInputEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_numeric_input_data_t* pData;
	int iRet;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_MOVE:
		return __xuiNumericInputPointerMove(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_LEAVE:
		if ( pEvent->iPhase != XUI_EVENT_PHASE_CAPTURE &&
		     !__xuiNumericInputContainsWidget(pWidget, pEvent->pRelated) ) {
			if ( pData->bPointerActive && !__xuiNumericInputPointerMatches(pData, pEvent) ) return XUI_OK;
			if ( !pData->bPointerInside && pData->iHoverButton == XUI_NUMERIC_INPUT_BUTTON_NONE ) return XUI_OK;
			pData->bPointerInside = 0;
			pData->iHoverButton = XUI_NUMERIC_INPUT_BUTTON_NONE;
			return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		break;
	case XUI_EVENT_POINTER_DOWN:
		return __xuiNumericInputPointerDown(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_UP:
		return __xuiNumericInputPointerUp(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_CLICK:
		if ( __xuiNumericInputEventButton(pWidget, pData, pEvent) != XUI_NUMERIC_INPUT_BUTTON_NONE ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_WHEEL:
		return __xuiNumericInputWheel(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( !__xuiNumericInputPointerMatches(pData, pEvent) ) return XUI_OK;
		return __xuiNumericInputCancelPointer(pWidget, pData, 1);
	case XUI_EVENT_FOCUS:
		if ( (pEvent->pTarget == pWidget || pEvent->pTarget == pData->pInput) &&
		     pEvent->iPhase != XUI_EVENT_PHASE_CAPTURE ) {
			return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		break;
	case XUI_EVENT_BLUR:
		if ( (pEvent->pTarget == pWidget || pEvent->pTarget == pData->pInput) &&
		     pEvent->iPhase != XUI_EVENT_PHASE_CAPTURE ) {
			return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		break;
	case XUI_EVENT_KEY_DOWN:
		if ( pEvent->pTarget != pWidget && pEvent->pTarget != pData->pInput ) return XUI_OK;
		return __xuiNumericInputKeyDown(pWidget, pData, pEvent);
	case XUI_EVENT_ENABLED_CHANGED:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		iRet = __xuiNumericInputSyncInputStyle(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( !xuiWidgetGetEnabled(pWidget) ) {
			iRet = __xuiNumericInputCancelPointer(pWidget, pData, 0);
			if ( iRet != XUI_OK ) return iRet;
		}
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( pEvent->pTarget != pWidget ) return XUI_OK;
		if ( !xuiWidgetGetVisible(pWidget) ) {
			iRet = __xuiNumericInputCancelPointer(pWidget, pData, 0);
			if ( iRet != XUI_OK ) return iRet;
		}
		return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiNumericInputContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_numeric_input_data_t* pData;
	xui_numeric_input_data_t tResolved;
	xui_font_metrics_t tMetrics;
	xui_proxy pProxy;

	(void)tConstraint;
	pData = (xui_numeric_input_data_t*)pUser;
	if ( (pWidget == NULL) || (pData == NULL) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiNumericInputResolve(pWidget, pData, &tResolved);
	pSize->fX = 128.0f;
	pSize->fY = 26.0f;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->fontGetMetrics != NULL) && (tResolved.pFont != NULL) &&
	     (pProxy->fontGetMetrics(pProxy, tResolved.pFont, &tMetrics) == XUI_OK) &&
	     (tMetrics.fLineHeight > 0.0f) ) {
		pSize->fY = tMetrics.fLineHeight + 8.0f;
	}
	if ( pSize->fY < 24.0f ) pSize->fY = 24.0f;
	if ( pSize->fX < tResolved.fSpinnerWidth + 72.0f ) {
		pSize->fX = tResolved.fSpinnerWidth + 72.0f;
	}
	pSize->fX = xuiInternalSnapSize(pSize->fX);
	pSize->fY = xuiInternalSnapSize(pSize->fY);
	return XUI_OK;
}

static int __xuiNumericInputLayoutPrepare(xui_widget pWidget, void* pUser)
{
	xui_numeric_input_data_t* pData;
	xui_numeric_input_data_t tResolved;
	xui_thickness_t tMargin;

	pData = (xui_numeric_input_data_t*)pUser;
	if ( (pWidget == NULL) || (pData == NULL) || (pData->pInput == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiNumericInputResolve(pWidget, pData, &tResolved);
	tMargin = (xui_thickness_t){0.0f, 0.0f,
		(tResolved.bSpinnerVisible != 0) ? tResolved.fSpinnerWidth : 0.0f, 0.0f};
	return xuiWidgetSetMargin(pData->pInput, tMargin);
}

static int __xuiNumericInputLayoutComplete(xui_widget pWidget, xui_rect_t tContentRect, void* pUser)
{
	xui_numeric_input_data_t* pData;
	xui_numeric_input_data_t tResolved;

	(void)tContentRect;
	pData = (xui_numeric_input_data_t*)pUser;
	if ( (pWidget == NULL) || (pData == NULL) || (pData->pInput == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiNumericInputResolve(pWidget, pData, &tResolved);
	__xuiNumericInputUpdateRects(pWidget, pData, &tResolved, NULL);
	return XUI_OK;
}

static uint32_t __xuiNumericInputState(xui_widget pWidget, xui_numeric_input_data_t* pData)
{
	uint32_t iState;
	xui_context pContext;
	xui_widget pFocus;

	iState = xuiWidgetGetInputState(pWidget);
	if ( pData != NULL && pData->bPointerInside ) {
		iState |= XUI_WIDGET_STATE_HOVER;
	}
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) {
		iState |= XUI_WIDGET_STATE_DISABLED;
	}
	if ( (pData != NULL) && pData->bError ) {
		iState |= XUI_NUMERIC_INPUT_STATE_ERROR;
	}
	if ( (pData != NULL) && pData->bReadonly ) {
		iState |= XUI_NUMERIC_INPUT_STATE_READONLY;
	}
	if ( pData != NULL ) {
		pContext = xuiWidgetGetContext(pWidget);
		pFocus = xuiGetFocusWidget(pContext);
		if ( (pFocus == pData->pInput) || (pFocus == pWidget) ) {
			iState |= XUI_WIDGET_STATE_FOCUS;
		}
	}
	return iState;
}

static int __xuiNumericInputDrawSpinnerButton(xui_widget pWidget, xui_draw_context pDraw, xui_proxy pProxy, xui_numeric_input_data_t* pData, const xui_numeric_input_data_t* pResolved, int iButton)
{
	xui_rect_t tRect;
	uint32_t iFill;
	uint32_t iIcon;
	int iEnabled;
	int iRet;

	(void)pWidget;
	tRect = (iButton == XUI_NUMERIC_INPUT_BUTTON_UP) ? pData->tButtonUpRect : pData->tButtonDownRect;
	iEnabled = __xuiNumericInputButtonEnabledData(pData, iButton) &&
		xuiWidgetGetEffectiveEnabled(pWidget) && !pData->bReadonly;
	iFill = pResolved->iSpinnerColor;
	if ( iEnabled && (pData->iActiveButton == iButton) && (pData->iHoverButton == iButton) ) {
		iFill = pResolved->iSpinnerActiveColor;
	} else if ( iEnabled && (pData->iHoverButton == iButton) ) {
		iFill = pResolved->iSpinnerHoverColor;
	}
	iRet = __xuiNumericInputDrawRectFill(pProxy, pDraw, tRect, iFill);
	if ( iRet != XUI_OK ) return iRet;
	iIcon = iEnabled ? pResolved->iSpinnerIconColor : pResolved->iSpinnerDisabledIconColor;
	return __xuiNumericInputDrawArrow(pProxy, pDraw, tRect, iButton == XUI_NUMERIC_INPUT_BUTTON_UP, iIcon);
}

static int __xuiNumericInputCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_numeric_input_data_t* pData;
	xui_numeric_input_data_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;
	uint32_t iState;
	uint32_t iBackground;
	uint32_t iBorder;
	int iRet;

	(void)iStateId;
	(void)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiNumericInputResolve(pWidget, pData, &tResolved);
	__xuiNumericInputUpdateRects(pWidget, pData, &tResolved, NULL);
	iState = __xuiNumericInputState(pWidget, pData);
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect = xuiInternalSnapRect(tRect);
	iBackground = tResolved.iBackgroundColor;
	iBorder = tResolved.iBorderColor;
	if ( (iState & XUI_WIDGET_STATE_DISABLED) != 0 ) {
		iBackground = tResolved.iDisabledBackgroundColor;
		iBorder = tResolved.iDisabledBorderColor;
	} else if ( (iState & XUI_NUMERIC_INPUT_STATE_ERROR) != 0 ) {
		iBackground = tResolved.iErrorBackgroundColor;
		iBorder = tResolved.iErrorBorderColor;
	} else if ( (iState & XUI_WIDGET_STATE_FOCUS) != 0 ) {
		iBorder = tResolved.iFocusBorderColor;
	} else if ( (iState & XUI_WIDGET_STATE_HOVER) != 0 ) {
		iBackground = tResolved.iHoverBackgroundColor;
		iBorder = tResolved.iHoverBorderColor;
	}
	iRet = __xuiNumericInputDrawRectFill(pProxy, pDraw, tRect, iBackground);
	if ( iRet != XUI_OK ) return iRet;
	if ( pData->bSpinnerVisible && pData->tSpinnerRect.fW > 0.0f ) {
		iRet = __xuiNumericInputDrawSpinnerButton(pWidget, pDraw, pProxy, pData, &tResolved, XUI_NUMERIC_INPUT_BUTTON_UP);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiNumericInputDrawSpinnerButton(pWidget, pDraw, pProxy, pData, &tResolved, XUI_NUMERIC_INPUT_BUTTON_DOWN);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiNumericInputDrawLine(pProxy, pDraw, pData->tSpinnerRect.fX, pData->tSpinnerRect.fY + 1.0f, pData->tSpinnerRect.fX, pData->tSpinnerRect.fY + pData->tSpinnerRect.fH - 1.0f, tResolved.iSpinnerBorderColor);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiNumericInputDrawLine(pProxy, pDraw, pData->tButtonDownRect.fX + 1.0f, pData->tButtonDownRect.fY, pData->tButtonDownRect.fX + pData->tButtonDownRect.fW - 1.0f, pData->tButtonDownRect.fY, tResolved.iSpinnerBorderColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiNumericInputDrawRectStroke(pProxy, pDraw, tRect, tResolved.fBorderWidth, iBorder);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_OK;
}

static void __xuiNumericInputDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_OVERLAY;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_CLIP;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static int __xuiNumericInputPreparePaint(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	uint32_t iHash = xuiWidgetGetStyleHash(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pInput != NULL && pData->iChildStyleHash != iHash ) {
		iRet = __xuiNumericInputSyncInputStyle(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		pData->iChildStyleHash = iHash;
	}
	return XUI_OK;
}

static void __xuiNumericInputDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiNumericInputInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetCancelAction(pWidget, __xuiNumericInputCancelAction, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_ENTER, __xuiNumericInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiNumericInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiNumericInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiNumericInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiNumericInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CLICK, __xuiNumericInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_WHEEL, __xuiNumericInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiNumericInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiNumericInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiNumericInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiNumericInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiNumericInputEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiNumericInputEvent, NULL);
	return iRet;
}

static int __xuiNumericInputCreateInputChild(xui_widget pWidget, xui_numeric_input_data_t* pData, const xui_numeric_input_desc_t* pDesc)
{
	xui_input_desc_t tInputDesc;
	int iRet;

	memset(&tInputDesc, 0, sizeof(tInputDesc));
	tInputDesc.iSize = sizeof(tInputDesc);
	tInputDesc.sPlaceholder = (pDesc != NULL) ? pDesc->sPlaceholder : "";
	tInputDesc.pFont = pData->pFont;
	tInputDesc.iMaxLength = XUI_NUMERIC_INPUT_MAX_TEXT - 1;
	tInputDesc.iTextAlign = XUI_INPUT_ALIGN_RIGHT;
	tInputDesc.bReadonly = pData->bReadonly;
	tInputDesc.bError = pData->bError;
	tInputDesc.iTextColor = pData->iTextColor;
	tInputDesc.iPlaceholderColor = pData->iPlaceholderColor;
	tInputDesc.iDisabledTextColor = pData->iDisabledTextColor;
	tInputDesc.iSelectionColor = pData->iSelectionColor;
	tInputDesc.iCursorColor = pData->iCursorColor;
	iRet = xuiInputCreate(xuiWidgetGetContext(pWidget), &pData->pInput, &tInputDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetTabStop(pData->pInput, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pData->pInput, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pWidget, pData->pInput);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pData->pInput);
		pData->pInput = NULL;
		return iRet;
	}
	iRet = xuiWidgetSetFlowMode(pData->pInput, XUI_FLOW_BLOCK);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetSizeMode(pData->pInput, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAlign(pData->pInput, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	if ( iRet == XUI_OK ) iRet = xuiInternalEditDelegate(pWidget, pData->pInput);
	if ( iRet == XUI_OK ) iRet = xuiInternalEditSetPrepare(pWidget, __xuiNumericInputPrepareEdit, pData);
	if ( iRet == XUI_OK ) {
		xui_edit_behavior_t tBehavior;
		memset(&tBehavior, 0, sizeof(tBehavior));
		tBehavior.iSize = sizeof(tBehavior);
		tBehavior.iTabBehavior = XUI_EDIT_TAB_FOCUS;
		tBehavior.iEnterBehavior = XUI_EDIT_ENTER_COMMIT;
		tBehavior.iEscapeBehavior = XUI_EDIT_ESCAPE_REVERT;
		tBehavior.bCommitOnBlur = 1;
		iRet = xuiEditSetBehavior(pWidget, &tBehavior);
	}
	if ( iRet == XUI_OK ) iRet = __xuiNumericInputSyncInputStyle(pWidget, pData);
	if ( iRet == XUI_OK ) {
		iRet = __xuiNumericInputSetInputTextFromValue(pWidget, pData);
	}
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pData->pInput);
		pData->pInput = NULL;
	}
	return iRet;
}

static int __xuiNumericInputInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_numeric_input_data_t* pData;
	const xui_numeric_input_desc_t* pDesc;
	float fMin;
	float fMax;
	float fValue;
	int iRet;

	(void)pUser;
	pData = (xui_numeric_input_data_t*)pTypeData;
	pDesc = (const xui_numeric_input_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiNumericInputDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pData, 0, sizeof(*pData));
	fMin = (pDesc != NULL) ? pDesc->fMin : 0.0f;
	fMax = (pDesc != NULL) ? pDesc->fMax : 100.0f;
	if ( (pDesc == NULL) || ((fMin == 0.0f) && (fMax == 0.0f)) ) {
		fMax = 100.0f;
	}
	__xuiNumericInputNormalizeRange(&fMin, &fMax);
	fValue = (pDesc != NULL) ? pDesc->fValue : fMin;
	pData->pFont = (pDesc != NULL) ? pDesc->pFont : NULL;
	pData->fMin = fMin;
	pData->fMax = fMax;
	pData->fValue = __xuiNumericInputClampFloat(fValue, fMin, fMax);
	pData->fStep = (pDesc != NULL && pDesc->fStep > 0.0f) ? pDesc->fStep : 1.0f;
	pData->iPrecision = (pDesc != NULL && pDesc->iPrecision > 0) ? __xuiNumericInputPrecision(pDesc->iPrecision) : 3;
	pData->bInteger = (pDesc != NULL && pDesc->bInteger != 0) ? 1 : 0;
	if ( pData->bInteger ) {
		pData->fValue = __xuiNumericInputClampFloat(__xuiNumericInputRoundFloat(pData->fValue),
			ceilf(pData->fMin), floorf(pData->fMax));
	}
	pData->bReadonly = (pDesc != NULL && pDesc->bReadonly != 0) ? 1 : 0;
	pData->bSpinnerVisible = (pDesc == NULL || pDesc->bSpinnerVisible >= 0) ? 1 : 0;
	pData->fSpinnerWidth = (pDesc != NULL && pDesc->fSpinnerWidth > 0.0f) ? pDesc->fSpinnerWidth : 22.0f;
	pData->iTextColor = (pDesc != NULL && pDesc->iTextColor != 0) ? pDesc->iTextColor : XUI_COLOR_RGBA(36, 48, 66, 255);
	pData->iPlaceholderColor = (pDesc != NULL && pDesc->iPlaceholderColor != 0) ? pDesc->iPlaceholderColor : XUI_COLOR_RGBA(135, 148, 166, 255);
	pData->iDisabledTextColor = (pDesc != NULL && pDesc->iDisabledTextColor != 0) ? pDesc->iDisabledTextColor : XUI_COLOR_RGBA(150, 160, 172, 255);
	pData->iBackgroundColor = (pDesc != NULL && pDesc->iBackgroundColor != 0) ? pDesc->iBackgroundColor : XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iHoverBackgroundColor = (pDesc != NULL && pDesc->iHoverBackgroundColor != 0) ? pDesc->iHoverBackgroundColor : XUI_COLOR_RGBA(249, 252, 255, 255);
	pData->iDisabledBackgroundColor = (pDesc != NULL && pDesc->iDisabledBackgroundColor != 0) ? pDesc->iDisabledBackgroundColor : XUI_COLOR_RGBA(242, 245, 249, 255);
	pData->iBorderColor = (pDesc != NULL && pDesc->iBorderColor != 0) ? pDesc->iBorderColor : XUI_COLOR_RGBA(176, 190, 207, 255);
	pData->iHoverBorderColor = (pDesc != NULL && pDesc->iHoverBorderColor != 0) ? pDesc->iHoverBorderColor : XUI_COLOR_RGBA(105, 166, 226, 255);
	pData->iFocusBorderColor = (pDesc != NULL && pDesc->iFocusBorderColor != 0) ? pDesc->iFocusBorderColor : XUI_COLOR_RGBA(47, 128, 237, 255);
	pData->iErrorBackgroundColor = (pDesc != NULL && pDesc->iErrorBackgroundColor != 0) ? pDesc->iErrorBackgroundColor : XUI_COLOR_RGBA(255, 248, 248, 255);
	pData->iErrorBorderColor = (pDesc != NULL && pDesc->iErrorBorderColor != 0) ? pDesc->iErrorBorderColor : XUI_COLOR_RGBA(220, 72, 72, 255);
	pData->iSelectionColor = (pDesc != NULL && pDesc->iSelectionColor != 0) ? pDesc->iSelectionColor : XUI_COLOR_RGBA(74, 144, 239, 255);
	pData->iCursorColor = (pDesc != NULL && pDesc->iCursorColor != 0) ? pDesc->iCursorColor : XUI_COLOR_RGBA(33, 94, 170, 255);
	pData->iSpinnerColor = (pDesc != NULL && pDesc->iSpinnerColor != 0) ? pDesc->iSpinnerColor : XUI_COLOR_RGBA(246, 250, 255, 255);
	pData->iSpinnerHoverColor = (pDesc != NULL && pDesc->iSpinnerHoverColor != 0) ? pDesc->iSpinnerHoverColor : XUI_COLOR_RGBA(231, 241, 252, 255);
	pData->iSpinnerActiveColor = (pDesc != NULL && pDesc->iSpinnerActiveColor != 0) ? pDesc->iSpinnerActiveColor : XUI_COLOR_RGBA(215, 232, 249, 255);
	pData->iSpinnerBorderColor = (pDesc != NULL && pDesc->iSpinnerBorderColor != 0) ? pDesc->iSpinnerBorderColor : XUI_COLOR_RGBA(205, 220, 236, 255);
	pData->iSpinnerIconColor = (pDesc != NULL && pDesc->iSpinnerIconColor != 0) ? pDesc->iSpinnerIconColor : XUI_COLOR_RGBA(68, 86, 110, 255);
	pData->iSpinnerDisabledIconColor = (pDesc != NULL && pDesc->iSpinnerDisabledIconColor != 0) ? pDesc->iSpinnerDisabledIconColor : XUI_COLOR_RGBA(157, 169, 184, 150);
	pData->fBorderWidth = (pDesc != NULL && pDesc->fBorderWidth > 0.0f) ? pDesc->fBorderWidth : 1.0f;
	pData->iHoverButton = XUI_NUMERIC_INPUT_BUTTON_NONE;
	pData->iActiveButton = XUI_NUMERIC_INPUT_BUTTON_NONE;
	pData->iActivePointerType = XUI_POINTER_TYPE_MOUSE;
	iRet = xuiWidgetSetPadding(pWidget, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 0);
	if ( iRet == XUI_OK ) iRet = __xuiNumericInputInitEvents(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiNumericInputCreateInputChild(pWidget, pData, pDesc);
}

static void __xuiNumericInputDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_numeric_input_data_t* pData;

	(void)pUser;
	pData = (xui_numeric_input_data_t*)pTypeData;
	if ( pData == NULL ) {
		return;
	}
	(void)__xuiNumericInputCancelPointer(pWidget, pData, 0);
	memset(pData, 0, sizeof(*pData));
}

static void __xuiNumericInputRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
{
	xui_style_property_info_t tInfo;

	if ( xuiStyleFindProperty(pContext, sName) != 0 ) {
		return;
	}
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = sName;
	tInfo.iValueType = iValueType;
	tInfo.iDirtyFlags = iDirtyFlags;
	tInfo.iFlags = iFlags;
	tInfo.pWidgetType = pType;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static void __xuiNumericInputRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;

	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.text.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.text.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.placeholder.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.selection.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.cursor.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, XUI_STYLE_PROPERTY_INHERITED);
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.background.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.background.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.border.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.border.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.border.focus_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.border.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.error.background_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.error.border_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.spinner.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.spinner.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.spinner.active_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.spinner.border_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.spinner.icon_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.spinner.icon_disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.border.width", XUI_STYLE_VALUE_FLOAT, iPaintDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "numeric_input.spinner.width", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiNumericInputRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, iLayoutDirty, XUI_STYLE_PROPERTY_INHERITED);
}

static int __xuiNumericInputAccessibleNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	uint32_t iMutableActions;
	int iRet;

	if ( pData == NULL || pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiInternalAccessibilityEditNode(pWidget, pNode);
	if ( iRet != XUI_OK ) return iRet;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_TEXTBOX;
	pNode->sValue = xuiNumericInputGetText(pWidget);
	pNode->tRange.iSize = sizeof(pNode->tRange);
	pNode->tRange.fValue = pData->fValue;
	pNode->tRange.fMin = __xuiNumericInputEffectiveMin(pData);
	pNode->tRange.fMax = __xuiNumericInputEffectiveMax(pData);
	pNode->tRange.fStep = pData->fStep;
	pNode->tRange.fPageStep = pData->fStep;
	iMutableActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_VALUE) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_INCREMENT) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_DECREMENT);
	if ( __xuiNumericInputCanUserChange(pWidget, pData) ) {
		pNode->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_INCREMENT) |
			XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_DECREMENT);
	} else {
		pNode->iActions &= ~iMutableActions;
	}
	return XUI_OK;
}

static int __xuiNumericInputAccessibleAction(xui_widget pWidget, int iAction, const void* pPayload)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	const xui_accessible_value_t* pValue;
	double fValue;
	float fParsed;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iAction == XUI_ACCESSIBLE_ACTION_INCREMENT || iAction == XUI_ACCESSIBLE_ACTION_DECREMENT ) {
		if ( !__xuiNumericInputCanUserChange(pWidget, pData) ) return XUI_ERROR_INVALID_STATE;
		return __xuiNumericInputStepInternal(pWidget, pData,
			iAction == XUI_ACCESSIBLE_ACTION_INCREMENT ? 1 : -1, 1);
	}
	if ( iAction == XUI_ACCESSIBLE_ACTION_SET_VALUE ) {
		if ( !__xuiNumericInputCanUserChange(pWidget, pData) ) return XUI_ERROR_INVALID_STATE;
		pValue = (const xui_accessible_value_t*)pPayload;
		if ( pValue == NULL || pValue->iSize < sizeof(*pValue) ) return XUI_ERROR_INVALID_ARGUMENT;
		if ( pValue->iType == XUI_ACCESSIBLE_VALUE_NUMBER ) {
			fValue = pValue->fNumber;
			if ( !isfinite(fValue) || fValue > (double)FLT_MAX || fValue < -(double)FLT_MAX ) {
				return XUI_ERROR_INVALID_ARGUMENT;
			}
			fParsed = (float)fValue;
		} else if ( pValue->iType == XUI_ACCESSIBLE_VALUE_TEXT && pValue->sText != NULL ) {
			if ( !__xuiNumericInputParseText(pValue->sText, &fParsed) ) return XUI_ERROR_INVALID_ARGUMENT;
		} else {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		if ( fParsed < __xuiNumericInputEffectiveMin(pData) ||
		     fParsed > __xuiNumericInputEffectiveMax(pData) ) return XUI_ERROR_INVALID_ARGUMENT;
		return __xuiNumericInputSetValueInternal(pWidget, pData, fParsed, 1);
	}
	return xuiInternalAccessibilityEditAction(pWidget, iAction, pPayload);
}

static const xui_internal_accessibility_adapter_t g_xuiNumericInputAccessible = {
	.getNode = __xuiNumericInputAccessibleNode,
	.performAction = __xuiNumericInputAccessibleAction
};

XUI_API xui_widget_type xuiNumericInputGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "numeric_input");
	if ( pType != NULL ) {
		pType->onPreparePaint = __xuiNumericInputPreparePaint;
		pType->pAccessibleAdapter = &g_xuiNumericInputAccessible;
		__xuiNumericInputRegisterStyleProperties(pContext, pType);
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "numeric_input";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_numeric_input_data_t);
	tDesc.onInit = __xuiNumericInputInit;
	tDesc.onDestroy = __xuiNumericInputDestroy;
	tDesc.onContentMeasure = __xuiNumericInputContentMeasure;
	tDesc.onLayoutPrepare = __xuiNumericInputLayoutPrepare;
	tDesc.onLayoutComplete = __xuiNumericInputLayoutComplete;
	tDesc.onCacheRender = __xuiNumericInputCacheRender;
	tDesc.onUpdate = __xuiNumericInputUpdate;
	__xuiNumericInputDefaultLayout(&tDesc.tLayout);
	__xuiNumericInputDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	pType->onPreparePaint = __xuiNumericInputPreparePaint;
	pType->pAccessibleAdapter = &g_xuiNumericInputAccessible;
	__xuiNumericInputRegisterStyleProperties(pContext, pType);
	return pType;
}

XUI_API int xuiNumericInputCreate(xui_context pContext, xui_widget* ppWidget, const xui_numeric_input_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	if ( !xuiInternalContextIsValid(pContext) || !__xuiNumericInputDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	pType = xuiNumericInputGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiNumericInputSetChange(xui_widget pWidget, xui_numeric_input_change_proc onChange, void* pUser)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onChange = onChange;
	pData->pChangeUser = pUser;
	return XUI_OK;
}

XUI_API int xuiNumericInputSetErrorChange(xui_widget pWidget, xui_numeric_input_error_proc onError, void* pUser)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onError = onError;
	pData->pErrorUser = pUser;
	return XUI_OK;
}

XUI_API int xuiNumericInputSetFormatter(xui_widget pWidget, xui_numeric_input_format_proc onFormat, void* pUser)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	xui_numeric_input_format_proc onOldFormat;
	void* pOldUser;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->onFormat == onFormat && pData->pFormatUser == pUser ) return XUI_OK;
	onOldFormat = pData->onFormat;
	pOldUser = pData->pFormatUser;
	pData->onFormat = onFormat;
	pData->pFormatUser = pUser;
	iRet = __xuiNumericInputSetInputTextFromValue(pWidget, pData);
	if ( iRet != XUI_OK && __xuiNumericInputGetData(pWidget) == pData ) {
		pData->onFormat = onOldFormat;
		pData->pFormatUser = pOldUser;
	}
	if ( iRet == XUI_OK && __xuiNumericInputGetData(pWidget) == pData ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	}
	return iRet;
}

XUI_API int xuiNumericInputSetRange(xui_widget pWidget, float fMin, float fMax)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	float fOldMin;
	float fOldMax;
	int iRet;
	if ( pData == NULL || !__xuiNumericInputFloatValid(fMin) || !__xuiNumericInputFloatValid(fMax) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiNumericInputNormalizeRange(&fMin, &fMax);
	if ( pData->bInteger && ceilf(fMin) > floorf(fMax) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fMin == fMin && pData->fMax == fMax ) return XUI_OK;
	fOldMin = pData->fMin;
	fOldMax = pData->fMax;
	pData->fMin = fMin;
	pData->fMax = fMax;
	iRet = __xuiNumericInputSetValueInternal(pWidget, pData, pData->fValue, 0);
	if ( iRet != XUI_OK && __xuiNumericInputGetData(pWidget) == pData ) {
		pData->fMin = fOldMin;
		pData->fMax = fOldMax;
	}
	if ( iRet == XUI_OK && __xuiNumericInputGetData(pWidget) == pData ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	}
	return iRet;
}

XUI_API int xuiNumericInputGetRange(xui_widget pWidget, float* pMin, float* pMax)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pMin != NULL ) *pMin = pData->fMin;
	if ( pMax != NULL ) *pMax = pData->fMax;
	return XUI_OK;
}

XUI_API int xuiNumericInputSetStep(xui_widget pWidget, float fStep)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	if ( (pData == NULL) || !__xuiNumericInputFloatValid(fStep) || (fStep < 0.0f) ) return XUI_ERROR_INVALID_ARGUMENT;
	fStep = (fStep > 0.0f) ? fStep : 1.0f;
	if ( pData->fStep == fStep ) return XUI_OK;
	pData->fStep = fStep;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return XUI_OK;
}

XUI_API float xuiNumericInputGetStep(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->fStep : 0.0f;
}

XUI_API int xuiNumericInputSetInteger(xui_widget pWidget, int bInteger)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	int bOldInteger;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bInteger = bInteger ? 1 : 0;
	if ( pData->bInteger == bInteger ) return XUI_OK;
	if ( bInteger && ceilf(pData->fMin) > floorf(pData->fMax) ) return XUI_ERROR_INVALID_ARGUMENT;
	bOldInteger = pData->bInteger;
	pData->bInteger = bInteger;
	iRet = __xuiNumericInputSetValueInternal(pWidget, pData, pData->fValue, 0);
	if ( iRet != XUI_OK && __xuiNumericInputGetData(pWidget) == pData ) pData->bInteger = bOldInteger;
	if ( iRet == XUI_OK && __xuiNumericInputGetData(pWidget) == pData ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	}
	return iRet;
}

XUI_API int xuiNumericInputIsInteger(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->bInteger : 0;
}

XUI_API int xuiNumericInputSetPrecision(xui_widget pWidget, int iPrecision)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	int iOldPrecision;
	int iRet;
	if ( pData == NULL || iPrecision < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	iPrecision = __xuiNumericInputPrecision(iPrecision);
	if ( pData->iPrecision == iPrecision ) return XUI_OK;
	iOldPrecision = pData->iPrecision;
	pData->iPrecision = iPrecision;
	iRet = __xuiNumericInputSetInputTextFromValue(pWidget, pData);
	if ( iRet != XUI_OK && __xuiNumericInputGetData(pWidget) == pData ) pData->iPrecision = iOldPrecision;
	if ( iRet == XUI_OK && __xuiNumericInputGetData(pWidget) == pData ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	}
	return iRet;
}

XUI_API int xuiNumericInputGetPrecision(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->iPrecision : 0;
}

XUI_API int xuiNumericInputSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	xui_font pOldFont;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pFont = (pFont != NULL) ? pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	if ( pData->pFont == pFont ) return XUI_OK;
	pOldFont = pData->pFont;
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->pFont = pFont;
	iRet = __xuiNumericInputSyncInputStyle(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->pFont = pOldFont;
		(void)__xuiNumericInputSyncInputStyle(pWidget, pData);
		return iRet;
	}
	return XUI_OK;
}

XUI_API xui_font xuiNumericInputGetFont(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API int xuiNumericInputSetReadonly(xui_widget pWidget, int bReadonly)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	int bOldReadonly;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bReadonly = bReadonly ? 1 : 0;
	if ( pData->bReadonly == bReadonly ) return XUI_OK;
	bOldReadonly = pData->bReadonly;
	if ( bReadonly ) {
		iRet = __xuiNumericInputCancelPointer(pWidget, pData, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->bReadonly = bReadonly;
	iRet = __xuiNumericInputSyncInputStyle(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->bReadonly = bOldReadonly;
		(void)__xuiNumericInputSyncInputStyle(pWidget, pData);
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiNumericInputIsReadonly(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->bReadonly : 0;
}

XUI_API int xuiNumericInputSetSpinnerVisible(xui_widget pWidget, int bVisible)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bVisible = bVisible ? 1 : 0;
	if ( pData->bSpinnerVisible == bVisible ) return XUI_OK;
	if ( !bVisible ) {
		iRet = __xuiNumericInputCancelPointer(pWidget, pData, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->bSpinnerVisible = bVisible;
	pData->iHoverButton = XUI_NUMERIC_INPUT_BUTTON_NONE;
	return XUI_OK;
}

XUI_API int xuiNumericInputGetSpinnerVisible(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->bSpinnerVisible : 0;
}

XUI_API int xuiNumericInputSetSpinnerWidth(xui_widget pWidget, float fWidth)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	int iRet;
	if ( (pData == NULL) || !__xuiNumericInputMetricValid(fWidth) ) return XUI_ERROR_INVALID_ARGUMENT;
	fWidth = (fWidth > 0.0f) ? fWidth : 22.0f;
	if ( pData->fSpinnerWidth == fWidth ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->fSpinnerWidth = fWidth;
	return XUI_OK;
}

XUI_API float xuiNumericInputGetSpinnerWidth(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->fSpinnerWidth : 0.0f;
}

XUI_API int xuiNumericInputSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iText, uint32_t iBorder, uint32_t iFocus)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	xui_numeric_input_data_t tOld;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iBackgroundColor == iBackground && pData->iTextColor == iText &&
	     pData->iBorderColor == iBorder && pData->iFocusBorderColor == iFocus ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	tOld = *pData;
	pData->iBackgroundColor = iBackground;
	pData->iTextColor = iText;
	pData->iBorderColor = iBorder;
	pData->iFocusBorderColor = iFocus;
	iRet = __xuiNumericInputSyncInputStyle(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->iBackgroundColor = tOld.iBackgroundColor;
		pData->iTextColor = tOld.iTextColor;
		pData->iBorderColor = tOld.iBorderColor;
		pData->iFocusBorderColor = tOld.iFocusBorderColor;
		(void)__xuiNumericInputSyncInputStyle(pWidget, pData);
	}
	return iRet;
}

XUI_API int xuiNumericInputGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pText, uint32_t* pBorder, uint32_t* pFocus)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBackground != NULL ) *pBackground = pData->iBackgroundColor;
	if ( pText != NULL ) *pText = pData->iTextColor;
	if ( pBorder != NULL ) *pBorder = pData->iBorderColor;
	if ( pFocus != NULL ) *pFocus = pData->iFocusBorderColor;
	return XUI_OK;
}

XUI_API int xuiNumericInputSetExtendedColors(xui_widget pWidget, uint32_t iPlaceholder, uint32_t iDisabledText, uint32_t iHoverBackground, uint32_t iDisabledBackground, uint32_t iHoverBorder, uint32_t iErrorBackground, uint32_t iErrorBorder, uint32_t iSelection, uint32_t iCursor)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	xui_numeric_input_data_t tOld;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iPlaceholderColor == iPlaceholder &&
	     pData->iDisabledTextColor == iDisabledText &&
	     pData->iHoverBackgroundColor == iHoverBackground &&
	     pData->iDisabledBackgroundColor == iDisabledBackground &&
	     pData->iHoverBorderColor == iHoverBorder &&
	     pData->iErrorBackgroundColor == iErrorBackground &&
	     pData->iErrorBorderColor == iErrorBorder &&
	     pData->iSelectionColor == iSelection && pData->iCursorColor == iCursor ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	tOld = *pData;
	pData->iPlaceholderColor = iPlaceholder;
	pData->iDisabledTextColor = iDisabledText;
	pData->iHoverBackgroundColor = iHoverBackground;
	pData->iDisabledBackgroundColor = iDisabledBackground;
	pData->iHoverBorderColor = iHoverBorder;
	pData->iErrorBackgroundColor = iErrorBackground;
	pData->iErrorBorderColor = iErrorBorder;
	pData->iSelectionColor = iSelection;
	pData->iCursorColor = iCursor;
	iRet = __xuiNumericInputSyncInputStyle(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->iPlaceholderColor = tOld.iPlaceholderColor;
		pData->iDisabledTextColor = tOld.iDisabledTextColor;
		pData->iHoverBackgroundColor = tOld.iHoverBackgroundColor;
		pData->iDisabledBackgroundColor = tOld.iDisabledBackgroundColor;
		pData->iHoverBorderColor = tOld.iHoverBorderColor;
		pData->iErrorBackgroundColor = tOld.iErrorBackgroundColor;
		pData->iErrorBorderColor = tOld.iErrorBorderColor;
		pData->iSelectionColor = tOld.iSelectionColor;
		pData->iCursorColor = tOld.iCursorColor;
		(void)__xuiNumericInputSyncInputStyle(pWidget, pData);
	}
	return iRet;
}

XUI_API int xuiNumericInputGetExtendedColors(xui_widget pWidget, uint32_t* pPlaceholder, uint32_t* pDisabledText, uint32_t* pHoverBackground, uint32_t* pDisabledBackground, uint32_t* pHoverBorder, uint32_t* pErrorBackground, uint32_t* pErrorBorder, uint32_t* pSelection, uint32_t* pCursor)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pPlaceholder != NULL ) *pPlaceholder = pData->iPlaceholderColor;
	if ( pDisabledText != NULL ) *pDisabledText = pData->iDisabledTextColor;
	if ( pHoverBackground != NULL ) *pHoverBackground = pData->iHoverBackgroundColor;
	if ( pDisabledBackground != NULL ) *pDisabledBackground = pData->iDisabledBackgroundColor;
	if ( pHoverBorder != NULL ) *pHoverBorder = pData->iHoverBorderColor;
	if ( pErrorBackground != NULL ) *pErrorBackground = pData->iErrorBackgroundColor;
	if ( pErrorBorder != NULL ) *pErrorBorder = pData->iErrorBorderColor;
	if ( pSelection != NULL ) *pSelection = pData->iSelectionColor;
	if ( pCursor != NULL ) *pCursor = pData->iCursorColor;
	return XUI_OK;
}

XUI_API int xuiNumericInputSetBorderWidth(xui_widget pWidget, float fBorderWidth)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	int iRet;

	if ( (pData == NULL) || !__xuiNumericInputMetricValid(fBorderWidth) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fBorderWidth == fBorderWidth ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->fBorderWidth = fBorderWidth;
	return XUI_OK;
}

XUI_API float xuiNumericInputGetBorderWidth(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->fBorderWidth : 0.0f;
}

XUI_API int xuiNumericInputSetSpinnerColors(xui_widget pWidget, uint32_t iColor, uint32_t iHover, uint32_t iActive, uint32_t iBorder, uint32_t iIcon, uint32_t iDisabledIcon)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iSpinnerColor == iColor && pData->iSpinnerHoverColor == iHover &&
	     pData->iSpinnerActiveColor == iActive && pData->iSpinnerBorderColor == iBorder &&
	     pData->iSpinnerIconColor == iIcon &&
	     pData->iSpinnerDisabledIconColor == iDisabledIcon ) return XUI_OK;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	pData->iSpinnerColor = iColor;
	pData->iSpinnerHoverColor = iHover;
	pData->iSpinnerActiveColor = iActive;
	pData->iSpinnerBorderColor = iBorder;
	pData->iSpinnerIconColor = iIcon;
	pData->iSpinnerDisabledIconColor = iDisabledIcon;
	return XUI_OK;
}

XUI_API int xuiNumericInputGetSpinnerColors(xui_widget pWidget, uint32_t* pColor, uint32_t* pHover, uint32_t* pActive, uint32_t* pBorder, uint32_t* pIcon, uint32_t* pDisabledIcon)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pColor != NULL ) *pColor = pData->iSpinnerColor;
	if ( pHover != NULL ) *pHover = pData->iSpinnerHoverColor;
	if ( pActive != NULL ) *pActive = pData->iSpinnerActiveColor;
	if ( pBorder != NULL ) *pBorder = pData->iSpinnerBorderColor;
	if ( pIcon != NULL ) *pIcon = pData->iSpinnerIconColor;
	if ( pDisabledIcon != NULL ) *pDisabledIcon = pData->iSpinnerDisabledIconColor;
	return XUI_OK;
}

XUI_API int xuiNumericInputSetValue(xui_widget pWidget, float fValue)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL || !__xuiNumericInputFloatValid(fValue) ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiNumericInputSetValueInternal(pWidget, pData, fValue, 0);
}

XUI_API float xuiNumericInputGetValue(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->fValue : 0.0f;
}

XUI_API int xuiNumericInputCommit(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiNumericInputCommitInternal(pWidget, pData, 1);
}

XUI_API int xuiNumericInputStep(xui_widget pWidget, int iDirection)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiNumericInputStepInternal(pWidget, pData, iDirection, 1);
}

XUI_API int xuiNumericInputSetText(xui_widget pWidget, const char* sText)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	if ( (pData == NULL) || (pData->pInput == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiInputSetText(pData->pInput, sText);
}

XUI_API const char* xuiNumericInputGetText(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL && pData->pInput != NULL) ? xuiInputGetText(pData->pInput) : "";
}

XUI_API int xuiNumericInputSetMenuTitle(xui_widget pWidget, int iCommand, const char* sTitle)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	if ( (pData == NULL) || (pData->pInput == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiInputSetMenuTitle(pData->pInput, iCommand, sTitle);
}

XUI_API const char* xuiNumericInputGetMenuTitle(xui_widget pWidget, int iCommand)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL && pData->pInput != NULL) ? xuiInputGetMenuTitle(pData->pInput, iCommand) : "";
}

XUI_API int xuiNumericInputOpenMenu(xui_widget pWidget, float fX, float fY)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	int iRet;
	if ( (pData == NULL) || (pData->pInput == NULL) || !isfinite(fX) || !isfinite(fY) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiNumericInputSyncInputStyle(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	return xuiInputOpenMenu(pData->pInput, fX, fY);
}

XUI_API xui_widget xuiNumericInputGetMenuWidget(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL && pData->pInput != NULL) ? xuiInputGetMenuWidget(pData->pInput) : NULL;
}

XUI_API xui_widget xuiNumericInputGetInputWidget(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->pInput : NULL;
}

XUI_API xui_rect_t xuiNumericInputGetSpinnerRect(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	xui_numeric_input_data_t tResolved;
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	__xuiNumericInputResolve(pWidget, pData, &tResolved);
	__xuiNumericInputUpdateRects(pWidget, pData, &tResolved, NULL);
	return pData->tSpinnerRect;
}

XUI_API xui_rect_t xuiNumericInputGetButtonRect(xui_widget pWidget, int iButton)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	xui_numeric_input_data_t tResolved;
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	__xuiNumericInputResolve(pWidget, pData, &tResolved);
	__xuiNumericInputUpdateRects(pWidget, pData, &tResolved, NULL);
	if ( iButton == XUI_NUMERIC_INPUT_BUTTON_UP ) return pData->tButtonUpRect;
	if ( iButton == XUI_NUMERIC_INPUT_BUTTON_DOWN ) return pData->tButtonDownRect;
	return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API int xuiNumericInputGetHoverButton(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->iHoverButton : XUI_NUMERIC_INPUT_BUTTON_NONE;
}

XUI_API int xuiNumericInputGetActiveButton(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->iActiveButton : XUI_NUMERIC_INPUT_BUTTON_NONE;
}

XUI_API int xuiNumericInputIsButtonEnabled(xui_widget pWidget, int iButton)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	if ( pData == NULL ) return 0;
	return xuiWidgetGetEffectiveEnabled(pWidget) && xuiWidgetGetEffectiveVisible(pWidget) &&
		!pData->bReadonly && __xuiNumericInputButtonEnabledData(pData, iButton);
}

XUI_API int xuiNumericInputGetError(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->bError : 0;
}

XUI_API uint32_t xuiNumericInputGetState(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? __xuiNumericInputState(pWidget, pData) : 0u;
}

XUI_API int xuiNumericInputGetChangeCount(xui_widget pWidget)
{
	xui_numeric_input_data_t* pData = __xuiNumericInputGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
