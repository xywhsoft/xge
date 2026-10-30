#include "xui_config.h"
#if XUI_ENABLE_COLOR_PICKER
#include "xui_internal.h"
#include "xui_pointer_press_internal.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_COLOR_PICKER_DEFAULT_POPUP_W	432.0f
#define XUI_COLOR_PICKER_DEFAULT_POPUP_H	260.0f
#define XUI_COLOR_PICKER_MIN_POPUP_W		408.0f
#define XUI_COLOR_PICKER_MIN_POPUP_H		260.0f
#define XUI_COLOR_PICKER_PANEL_PAD		14.0f
#define XUI_COLOR_PICKER_TRACK_HIT_H		18.0f

typedef struct xui_color_picker_data_t {
	xui_widget pPopup;
	xui_widget pPanel;
	xui_font pFont;
	xui_color_picker_change_proc onChange;
	void* pChangeUser;
	uint32_t iColor;
	uint32_t iOldColor;
	uint32_t arrPalette[XUI_COLOR_PICKER_PALETTE_CAPACITY];
	xui_rect_t arrPaletteRect[XUI_COLOR_PICKER_PALETTE_CAPACITY];
	int iPaletteCount;
	int iSelectedPalette;
	int bAlphaEnabled;
	int iActivePart;
	int iHoverPart;
	int iActiveChannel;
	xui_pointer_owner_t tPointerOwner;
	int iKeyboardPart;
	int iKeyboardPalette;
	int iChangeCount;
	float fHue;
	float fSaturation;
	float fValue;
	float fPopupWidth;
	float fPopupHeight;
	int iPopupPlacement;
	char sHex[16];
	char sEdit[16];
	int iEditLen;
	int bEditingHex;
	int iEditingChannel;
	int bEditSelectAll;
	int bEditError;
	xui_rect_t tSwatchRect;
	xui_rect_t tButtonRect;
	xui_rect_t tTextRect;
	xui_rect_t tSvRect;
	xui_rect_t tHueRect;
	xui_rect_t tAlphaRect;
	xui_rect_t tOldRect;
	xui_rect_t tNewRect;
	xui_rect_t tHexRect;
	xui_rect_t arrFieldRect[4];
	xui_rect_t arrSliderRect[4];
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iBackgroundColor;
	uint32_t iHoverBackgroundColor;
	uint32_t iOpenBackgroundColor;
	uint32_t iDisabledBackgroundColor;
	uint32_t iBorderColor;
	uint32_t iHoverBorderColor;
	uint32_t iFocusBorderColor;
	uint32_t iArrowColor;
	uint32_t iDisabledArrowColor;
	uint32_t iButtonColor;
	uint32_t iButtonHoverColor;
	uint32_t iButtonOpenColor;
	uint32_t iPopupPanelColor;
	uint32_t iPopupBorderColor;
	uint32_t iPopupShadowColor;
	uint32_t iPopupTextColor;
	uint32_t iPopupMutedTextColor;
	uint32_t iAccentColor;
	uint32_t iFieldColor;
	uint32_t iFieldBorderColor;
	uint32_t iSeparatorColor;
	uint32_t iErrorBorderColor;
	uint32_t iSelectionColor;
	uint32_t iSelectionTextColor;
	uint32_t iTrackColor;
	uint32_t iKnobColor;
	uint32_t iMarkerColor;
	uint32_t iMarkerBorderColor;
	uint32_t iCheckerLightColor;
	uint32_t iCheckerDarkColor;
	float fBorderWidth;
	uint32_t iChildStyleHash;
	int bChildStyleSynced;
} xui_color_picker_data_t;

static xui_color_picker_data_t* __xuiColorPickerGetData(xui_widget pWidget);
static int __xuiColorPickerApplyPopupStyle(xui_widget pWidget, xui_color_picker_data_t* pData);
static int __xuiColorPickerPlacementValid(int iPlacement);

static int __xuiColorPickerMetricValid(float fValue)
{
	return isfinite(fValue) && fValue >= 0.0f && fValue <= XUI_LAYOUT_UNBOUNDED;
}

static int __xuiColorPickerAlpha(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static int __xuiColorPickerR(uint32_t iColor)
{
	return (int)((iColor >> 24) & 0xffu);
}

static int __xuiColorPickerG(uint32_t iColor)
{
	return (int)((iColor >> 16) & 0xffu);
}

static int __xuiColorPickerB(uint32_t iColor)
{
	return (int)((iColor >> 8) & 0xffu);
}

static int __xuiColorPickerA(uint32_t iColor)
{
	return (int)(iColor & 0xffu);
}

static uint32_t __xuiColorPickerColorWithAlpha(uint32_t iColor, uint32_t iAlpha)
{
	if ( (iColor & 0xffu) == 0 ) return iColor;
	return (iColor & 0xffffff00u) | (iAlpha & 0xffu);
}

static int __xuiColorPickerClampByte(int iValue)
{
	if ( iValue < 0 ) return 0;
	if ( iValue > 255 ) return 255;
	return iValue;
}

static float __xuiColorPickerClamp01(float fValue)
{
	if ( fValue < 0.0f ) return 0.0f;
	if ( fValue > 1.0f ) return 1.0f;
	return fValue;
}

static float __xuiColorPickerMax(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static float __xuiColorPickerMin(float fA, float fB)
{
	return (fA < fB) ? fA : fB;
}

static uint32_t __xuiColorPickerOpaque(uint32_t iColor)
{
	return XUI_COLOR_RGBA(__xuiColorPickerR(iColor), __xuiColorPickerG(iColor), __xuiColorPickerB(iColor), 255);
}

static uint32_t __xuiColorPickerNormalizeColor(const xui_color_picker_data_t* pData, uint32_t iColor)
{
	if ( (pData == NULL) || !pData->bAlphaEnabled ) {
		return __xuiColorPickerOpaque(iColor);
	}
	return iColor;
}

static int __xuiColorPickerHexNibble(char c)
{
	if ( c >= '0' && c <= '9' ) return c - '0';
	if ( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
	if ( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
	return -1;
}

static int __xuiColorPickerHexChar(uint32_t iCodepoint)
{
	return ((iCodepoint >= '0' && iCodepoint <= '9') ||
	        (iCodepoint >= 'a' && iCodepoint <= 'f') ||
	        (iCodepoint >= 'A' && iCodepoint <= 'F'));
}

static void __xuiColorPickerFormatHex(xui_color_picker_data_t* pData)
{
	if ( pData == NULL ) {
		return;
	}
	if ( pData->bAlphaEnabled ) {
		snprintf(pData->sHex, sizeof(pData->sHex), "#%02X%02X%02X%02X",
			__xuiColorPickerR(pData->iColor),
			__xuiColorPickerG(pData->iColor),
			__xuiColorPickerB(pData->iColor),
			__xuiColorPickerA(pData->iColor));
	} else {
		snprintf(pData->sHex, sizeof(pData->sHex), "#%02X%02X%02X",
			__xuiColorPickerR(pData->iColor),
			__xuiColorPickerG(pData->iColor),
			__xuiColorPickerB(pData->iColor));
	}
	pData->sHex[sizeof(pData->sHex) - 1] = '\0';
}

static int __xuiColorPickerParseHex(const char* sHex, uint32_t* pColor)
{
	const char* s;
	unsigned int arr[4];
	int iLen;
	int i;
	int n0;
	int n1;

	if ( (sHex == NULL) || (pColor == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	s = (sHex[0] == '#') ? (sHex + 1) : sHex;
	iLen = (int)strlen(s);
	if ( (iLen != 6) && (iLen != 8) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(arr, 0, sizeof(arr));
	for ( i = 0; i < iLen / 2; i++ ) {
		n0 = __xuiColorPickerHexNibble(s[i * 2]);
		n1 = __xuiColorPickerHexNibble(s[i * 2 + 1]);
		if ( (n0 < 0) || (n1 < 0) ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		arr[i] = (unsigned int)((n0 << 4) | n1);
	}
	if ( iLen == 6 ) {
		arr[3] = 255;
	}
	*pColor = XUI_COLOR_RGBA(arr[0], arr[1], arr[2], arr[3]);
	return XUI_OK;
}

static uint32_t __xuiColorPickerMix(uint32_t iA, uint32_t iB, float fT)
{
	int iR;
	int iG;
	int iBlue;
	int iAlpha;

	fT = __xuiColorPickerClamp01(fT);
	iR = (int)((float)__xuiColorPickerR(iA) + ((float)__xuiColorPickerR(iB) - (float)__xuiColorPickerR(iA)) * fT + 0.5f);
	iG = (int)((float)__xuiColorPickerG(iA) + ((float)__xuiColorPickerG(iB) - (float)__xuiColorPickerG(iA)) * fT + 0.5f);
	iBlue = (int)((float)__xuiColorPickerB(iA) + ((float)__xuiColorPickerB(iB) - (float)__xuiColorPickerB(iA)) * fT + 0.5f);
	iAlpha = (int)((float)__xuiColorPickerA(iA) + ((float)__xuiColorPickerA(iB) - (float)__xuiColorPickerA(iA)) * fT + 0.5f);
	return XUI_COLOR_RGBA(iR, iG, iBlue, iAlpha);
}

static uint32_t __xuiColorPickerHsvToRgb(float fHue, float fSat, float fVal)
{
	float fC;
	float fX;
	float fM;
	float fR;
	float fG;
	float fB;
	int iSector;

	fHue = fHue - floorf(fHue);
	fSat = __xuiColorPickerClamp01(fSat);
	fVal = __xuiColorPickerClamp01(fVal);
	fC = fVal * fSat;
	iSector = (int)floorf(fHue * 6.0f);
	fX = fC * (1.0f - fabsf(fmodf(fHue * 6.0f, 2.0f) - 1.0f));
	fM = fVal - fC;
	fR = 0.0f;
	fG = 0.0f;
	fB = 0.0f;
	switch ( iSector % 6 ) {
	case 0: fR = fC; fG = fX; break;
	case 1: fR = fX; fG = fC; break;
	case 2: fG = fC; fB = fX; break;
	case 3: fG = fX; fB = fC; break;
	case 4: fR = fX; fB = fC; break;
	default: fR = fC; fB = fX; break;
	}
	return XUI_COLOR_RGBA(
		__xuiColorPickerClampByte((int)((fR + fM) * 255.0f + 0.5f)),
		__xuiColorPickerClampByte((int)((fG + fM) * 255.0f + 0.5f)),
		__xuiColorPickerClampByte((int)((fB + fM) * 255.0f + 0.5f)),
		255);
}

static void __xuiColorPickerRgbToHsv(uint32_t iColor, float* pHue, float* pSat, float* pVal)
{
	float fR;
	float fG;
	float fB;
	float fMax;
	float fMin;
	float fDelta;
	float fHue;

	fR = (float)__xuiColorPickerR(iColor) / 255.0f;
	fG = (float)__xuiColorPickerG(iColor) / 255.0f;
	fB = (float)__xuiColorPickerB(iColor) / 255.0f;
	fMax = fmaxf(fR, fmaxf(fG, fB));
	fMin = fminf(fR, fminf(fG, fB));
	fDelta = fMax - fMin;
	fHue = 0.0f;
	if ( fDelta > 0.0001f ) {
		if ( fMax == fR ) {
			fHue = fmodf(((fG - fB) / fDelta), 6.0f) / 6.0f;
		} else if ( fMax == fG ) {
			fHue = (((fB - fR) / fDelta) + 2.0f) / 6.0f;
		} else {
			fHue = (((fR - fG) / fDelta) + 4.0f) / 6.0f;
		}
		if ( fHue < 0.0f ) {
			fHue += 1.0f;
		}
	}
	if ( pHue != NULL ) *pHue = fHue;
	if ( pSat != NULL ) *pSat = (fMax <= 0.0001f) ? 0.0f : (fDelta / fMax);
	if ( pVal != NULL ) *pVal = fMax;
}

static uint32_t __xuiColorPickerTextOnColor(uint32_t iColor)
{
	int iLuma;

	iLuma = __xuiColorPickerR(iColor) * 299 + __xuiColorPickerG(iColor) * 587 + __xuiColorPickerB(iColor) * 114;
	return (iLuma > 145000) ? XUI_COLOR_RGBA(22, 34, 44, 255) : XUI_COLOR_RGBA(255, 255, 255, 255);
}

static int __xuiColorPickerRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (tRect.fW > 0) && (tRect.fH > 0) &&
	       (fX >= tRect.fX) &&
	       (fY >= tRect.fY) &&
	       (fX < (tRect.fX + tRect.fW)) &&
	       (fY < (tRect.fY + tRect.fH));
}

static xui_rect_t __xuiColorPickerTrackHitRect(xui_rect_t tTrack)
{
	float fHeight;
	float fCenterY;

	fHeight = (float)tTrack.fH;
	if ( fHeight < XUI_COLOR_PICKER_TRACK_HIT_H ) fHeight = XUI_COLOR_PICKER_TRACK_HIT_H;
	fCenterY = (float)tTrack.fY + (float)tTrack.fH * 0.5f;
	return xuiInternalRectFromFloatNearest((float)tTrack.fX,
		fCenterY - fHeight * 0.5f, (float)tTrack.fW, fHeight);
}

static int __xuiColorPickerChannelPart(int iChannel)
{
	if ( iChannel == 0 ) return XUI_COLOR_PICKER_PART_RED;
	if ( iChannel == 1 ) return XUI_COLOR_PICKER_PART_GREEN;
	if ( iChannel == 2 ) return XUI_COLOR_PICKER_PART_BLUE;
	if ( iChannel == 3 ) return XUI_COLOR_PICKER_PART_ALPHA;
	return XUI_COLOR_PICKER_PART_NONE;
}

static int __xuiColorPickerPartChannel(int iPart)
{
	if ( iPart == XUI_COLOR_PICKER_PART_RED ) return 0;
	if ( iPart == XUI_COLOR_PICKER_PART_GREEN ) return 1;
	if ( iPart == XUI_COLOR_PICKER_PART_BLUE ) return 2;
	if ( iPart == XUI_COLOR_PICKER_PART_ALPHA ) return 3;
	return -1;
}

static int __xuiColorPickerAlive(xui_context pContext, xui_widget pWidget,
	const xui_color_picker_data_t* pExpected)
{
	return xuiInternalContextIsValid(pContext) && !xuiInternalContextDestroyPending(pContext) &&
		xuiInternalWidgetIsValid(pWidget) && __xuiColorPickerGetData(pWidget) == pExpected;
}

static int __xuiColorPickerKeyboardParts(const xui_color_picker_data_t* pData, int* arrParts, int iCapacity)
{
	int iCount;

	if ( pData == NULL || arrParts == NULL || iCapacity < 9 ) return 0;
	iCount = 0;
	arrParts[iCount++] = XUI_COLOR_PICKER_PART_SV;
	arrParts[iCount++] = XUI_COLOR_PICKER_PART_HUE;
	arrParts[iCount++] = XUI_COLOR_PICKER_PART_OLD;
	arrParts[iCount++] = XUI_COLOR_PICKER_PART_RED;
	arrParts[iCount++] = XUI_COLOR_PICKER_PART_GREEN;
	arrParts[iCount++] = XUI_COLOR_PICKER_PART_BLUE;
	if ( pData->bAlphaEnabled ) arrParts[iCount++] = XUI_COLOR_PICKER_PART_ALPHA;
	arrParts[iCount++] = XUI_COLOR_PICKER_PART_HEX;
	if ( pData->iPaletteCount > 0 ) arrParts[iCount++] = XUI_COLOR_PICKER_PART_PALETTE;
	return iCount;
}

static int __xuiColorPickerDescValid(const xui_color_picker_desc_t* pDesc)
{
	if ( pDesc == NULL ) {
		return 1;
	}
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) {
		return 0;
	}
	if ( (pDesc->iPaletteCount < 0) ||
	     (pDesc->iPaletteCount > XUI_COLOR_PICKER_PALETTE_CAPACITY) ||
	     !__xuiColorPickerMetricValid(pDesc->fPopupWidth) ||
	     !__xuiColorPickerMetricValid(pDesc->fPopupHeight) ||
	     !__xuiColorPickerMetricValid(pDesc->fBorderWidth) ||
	     ((pDesc->fPopupWidth > 0.0f) && (pDesc->fPopupWidth < XUI_COLOR_PICKER_MIN_POPUP_W)) ||
	     ((pDesc->fPopupHeight > 0.0f) && (pDesc->fPopupHeight < XUI_COLOR_PICKER_MIN_POPUP_H)) ||
	     !__xuiColorPickerPlacementValid(pDesc->iPopupPlacement) ) {
		return 0;
	}
	if ( (pDesc->iPaletteCount > 0) && (pDesc->arrPalette == NULL) ) {
		return 0;
	}
	return 1;
}

static int __xuiColorPickerPlacementValid(int iPlacement)
{
	return (iPlacement == XUI_COLOR_PICKER_POPUP_AUTO) ||
	       (iPlacement == XUI_COLOR_PICKER_POPUP_BOTTOM) ||
	       (iPlacement == XUI_COLOR_PICKER_POPUP_TOP);
}

static int __xuiColorPickerStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
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

static int __xuiColorPickerStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) &&
	     (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     __xuiColorPickerMetricValid(tProperty.tValue.fFloat) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static xui_font __xuiColorPickerStyleFont(xui_widget pWidget, xui_font pBaseFont)
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

static void __xuiColorPickerDefaultPalette(xui_color_picker_data_t* pData)
{
	static const uint32_t arrDefault[] = {
		XUI_COLOR_RGBA(0, 0, 0, 255),
		XUI_COLOR_RGBA(255, 255, 255, 255),
		XUI_COLOR_RGBA(148, 163, 184, 255),
		XUI_COLOR_RGBA(239, 68, 68, 255),
		XUI_COLOR_RGBA(249, 115, 22, 255),
		XUI_COLOR_RGBA(245, 158, 11, 255),
		XUI_COLOR_RGBA(234, 179, 8, 255),
		XUI_COLOR_RGBA(132, 204, 22, 255),
		XUI_COLOR_RGBA(34, 197, 94, 255),
		XUI_COLOR_RGBA(20, 184, 166, 255),
		XUI_COLOR_RGBA(6, 182, 212, 255),
		XUI_COLOR_RGBA(59, 130, 246, 255),
		XUI_COLOR_RGBA(99, 102, 241, 255),
		XUI_COLOR_RGBA(139, 92, 246, 255),
		XUI_COLOR_RGBA(217, 70, 239, 255),
		XUI_COLOR_RGBA(236, 72, 153, 255)
	};
	int i;

	pData->iPaletteCount = (int)(sizeof(arrDefault) / sizeof(arrDefault[0]));
	for ( i = 0; i < pData->iPaletteCount; i++ ) {
		pData->arrPalette[i] = arrDefault[i];
	}
}

static void __xuiColorPickerDefaults(xui_color_picker_data_t* pData)
{
	memset(pData, 0, sizeof(*pData));
	pData->iColor = XUI_COLOR_RGBA(46, 124, 214, 255);
	pData->iOldColor = pData->iColor;
	pData->iSelectedPalette = -1;
	pData->iActivePart = XUI_COLOR_PICKER_PART_NONE;
	pData->iHoverPart = XUI_COLOR_PICKER_PART_NONE;
	pData->iActiveChannel = -1;
	pData->iKeyboardPart = XUI_COLOR_PICKER_PART_NONE;
	pData->iKeyboardPalette = -1;
	pData->iEditingChannel = -1;
	pData->fPopupWidth = XUI_COLOR_PICKER_DEFAULT_POPUP_W;
	pData->fPopupHeight = XUI_COLOR_PICKER_DEFAULT_POPUP_H;
	pData->iPopupPlacement = XUI_COLOR_PICKER_POPUP_AUTO;
	pData->iTextColor = XUI_COLOR_RGBA(33, 55, 79, 255);
	pData->iDisabledTextColor = XUI_COLOR_RGBA(132, 146, 162, 210);
	pData->iBackgroundColor = XUI_COLOR_RGBA(248, 252, 255, 255);
	pData->iHoverBackgroundColor = XUI_COLOR_RGBA(232, 244, 252, 255);
	pData->iOpenBackgroundColor = XUI_COLOR_RGBA(222, 239, 252, 255);
	pData->iDisabledBackgroundColor = XUI_COLOR_RGBA(235, 240, 245, 255);
	pData->iBorderColor = XUI_COLOR_RGBA(132, 174, 214, 255);
	pData->iHoverBorderColor = XUI_COLOR_RGBA(78, 148, 208, 255);
	pData->iFocusBorderColor = XUI_COLOR_RGBA(42, 126, 205, 255);
	pData->iArrowColor = XUI_COLOR_RGBA(42, 92, 136, 255);
	pData->iDisabledArrowColor = XUI_COLOR_RGBA(134, 148, 164, 180);
	pData->iButtonColor = XUI_COLOR_RGBA(236, 246, 253, 255);
	pData->iButtonHoverColor = XUI_COLOR_RGBA(220, 238, 251, 255);
	pData->iButtonOpenColor = XUI_COLOR_RGBA(207, 229, 247, 255);
	pData->iPopupPanelColor = XUI_COLOR_RGBA(250, 253, 255, 255);
	pData->iPopupBorderColor = XUI_COLOR_RGBA(122, 164, 202, 255);
	pData->iPopupShadowColor = XUI_COLOR_RGBA(44, 70, 96, 46);
	pData->iPopupTextColor = XUI_COLOR_RGBA(28, 60, 94, 255);
	pData->iPopupMutedTextColor = XUI_COLOR_RGBA(92, 116, 142, 255);
	pData->iAccentColor = XUI_COLOR_RGBA(42, 126, 205, 255);
	pData->iFieldColor = XUI_COLOR_RGBA(244, 249, 253, 255);
	pData->iFieldBorderColor = XUI_COLOR_RGBA(142, 176, 208, 255);
	pData->iSeparatorColor = XUI_COLOR_RGBA(206, 222, 236, 255);
	pData->iErrorBorderColor = XUI_COLOR_RGBA(218, 82, 82, 255);
	pData->iSelectionColor = XUI_COLOR_RGBA(42, 126, 205, 180);
	pData->iSelectionTextColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iTrackColor = XUI_COLOR_RGBA(210, 225, 238, 255);
	pData->iKnobColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iMarkerColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iMarkerBorderColor = XUI_COLOR_RGBA(32, 44, 56, 210);
	pData->iCheckerLightColor = XUI_COLOR_RGBA(242, 247, 252, 255);
	pData->iCheckerDarkColor = XUI_COLOR_RGBA(204, 216, 228, 255);
	pData->fBorderWidth = 1.0f;
	__xuiColorPickerDefaultPalette(pData);
	__xuiColorPickerRgbToHsv(pData->iColor, &pData->fHue, &pData->fSaturation, &pData->fValue);
	__xuiColorPickerFormatHex(pData);
}

static void __xuiColorPickerApplyDesc(xui_color_picker_data_t* pData, const xui_color_picker_desc_t* pDesc)
{
	int i;

	if ( (pData == NULL) || (pDesc == NULL) ) {
		return;
	}
	pData->pFont = pDesc->pFont;
	pData->bAlphaEnabled = pDesc->bAlphaEnabled ? 1 : 0;
	pData->iColor = pDesc->iColor;
	pData->iColor = __xuiColorPickerNormalizeColor(pData, pData->iColor);
	if ( (pDesc->arrPalette != NULL) && (pDesc->iPaletteCount > 0) ) {
		pData->iPaletteCount = pDesc->iPaletteCount;
		for ( i = 0; i < pData->iPaletteCount; i++ ) {
			pData->arrPalette[i] = pDesc->arrPalette[i];
		}
	}
	if ( pDesc->fPopupWidth > 0.0f ) pData->fPopupWidth = pDesc->fPopupWidth;
	if ( pDesc->fPopupHeight > 0.0f ) pData->fPopupHeight = pDesc->fPopupHeight;
	if ( __xuiColorPickerPlacementValid(pDesc->iPopupPlacement) ) pData->iPopupPlacement = pDesc->iPopupPlacement;
	if ( __xuiColorPickerAlpha(pDesc->iTextColor) != 0 ) pData->iTextColor = pDesc->iTextColor;
	if ( __xuiColorPickerAlpha(pDesc->iDisabledTextColor) != 0 ) pData->iDisabledTextColor = pDesc->iDisabledTextColor;
	if ( __xuiColorPickerAlpha(pDesc->iBackgroundColor) != 0 ) pData->iBackgroundColor = pDesc->iBackgroundColor;
	if ( __xuiColorPickerAlpha(pDesc->iHoverBackgroundColor) != 0 ) pData->iHoverBackgroundColor = pDesc->iHoverBackgroundColor;
	if ( __xuiColorPickerAlpha(pDesc->iOpenBackgroundColor) != 0 ) pData->iOpenBackgroundColor = pDesc->iOpenBackgroundColor;
	if ( __xuiColorPickerAlpha(pDesc->iDisabledBackgroundColor) != 0 ) pData->iDisabledBackgroundColor = pDesc->iDisabledBackgroundColor;
	if ( __xuiColorPickerAlpha(pDesc->iBorderColor) != 0 ) pData->iBorderColor = pDesc->iBorderColor;
	if ( __xuiColorPickerAlpha(pDesc->iHoverBorderColor) != 0 ) pData->iHoverBorderColor = pDesc->iHoverBorderColor;
	if ( __xuiColorPickerAlpha(pDesc->iFocusBorderColor) != 0 ) pData->iFocusBorderColor = pDesc->iFocusBorderColor;
	if ( __xuiColorPickerAlpha(pDesc->iArrowColor) != 0 ) pData->iArrowColor = pDesc->iArrowColor;
	if ( __xuiColorPickerAlpha(pDesc->iDisabledArrowColor) != 0 ) pData->iDisabledArrowColor = pDesc->iDisabledArrowColor;
	if ( __xuiColorPickerAlpha(pDesc->iButtonColor) != 0 ) pData->iButtonColor = pDesc->iButtonColor;
	if ( __xuiColorPickerAlpha(pDesc->iButtonHoverColor) != 0 ) pData->iButtonHoverColor = pDesc->iButtonHoverColor;
	if ( __xuiColorPickerAlpha(pDesc->iButtonOpenColor) != 0 ) pData->iButtonOpenColor = pDesc->iButtonOpenColor;
	if ( __xuiColorPickerAlpha(pDesc->iPopupPanelColor) != 0 ) pData->iPopupPanelColor = pDesc->iPopupPanelColor;
	if ( __xuiColorPickerAlpha(pDesc->iPopupBorderColor) != 0 ) pData->iPopupBorderColor = pDesc->iPopupBorderColor;
	if ( pDesc->iPopupShadowColor != 0u ) pData->iPopupShadowColor = pDesc->iPopupShadowColor;
	if ( __xuiColorPickerAlpha(pDesc->iPopupTextColor) != 0 ) pData->iPopupTextColor = pDesc->iPopupTextColor;
	if ( __xuiColorPickerAlpha(pDesc->iPopupMutedTextColor) != 0 ) pData->iPopupMutedTextColor = pDesc->iPopupMutedTextColor;
	if ( __xuiColorPickerAlpha(pDesc->iAccentColor) != 0 ) pData->iAccentColor = pDesc->iAccentColor;
	if ( __xuiColorPickerAlpha(pDesc->iFieldColor) != 0 ) pData->iFieldColor = pDesc->iFieldColor;
	if ( __xuiColorPickerAlpha(pDesc->iFieldBorderColor) != 0 ) pData->iFieldBorderColor = pDesc->iFieldBorderColor;
	if ( __xuiColorPickerAlpha(pDesc->iSeparatorColor) != 0 ) pData->iSeparatorColor = pDesc->iSeparatorColor;
	if ( pDesc->fBorderWidth > 0.0f ) pData->fBorderWidth = pDesc->fBorderWidth;
	pData->iOldColor = pData->iColor;
	__xuiColorPickerRgbToHsv(pData->iColor, &pData->fHue, &pData->fSaturation, &pData->fValue);
	__xuiColorPickerFormatHex(pData);
}

static void __xuiColorPickerResolve(xui_widget pWidget, xui_color_picker_data_t* pData, xui_color_picker_data_t* pResolved)
{
	*pResolved = *pData;
	pResolved->pFont = (pData->pFont != NULL) ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	pResolved->pFont = __xuiColorPickerStyleFont(pWidget, pResolved->pFont);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.text.color", &pResolved->iTextColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.text.disabled_color", &pResolved->iDisabledTextColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.background.color", &pResolved->iBackgroundColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.background.hover_color", &pResolved->iHoverBackgroundColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.background.open_color", &pResolved->iOpenBackgroundColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.background.disabled_color", &pResolved->iDisabledBackgroundColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.border.color", &pResolved->iBorderColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.border.hover_color", &pResolved->iHoverBorderColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.border.focus_color", &pResolved->iFocusBorderColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.arrow.color", &pResolved->iArrowColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.arrow.disabled_color", &pResolved->iDisabledArrowColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.button.color", &pResolved->iButtonColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.button.hover_color", &pResolved->iButtonHoverColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.button.open_color", &pResolved->iButtonOpenColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.popup.panel_color", &pResolved->iPopupPanelColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.popup.border_color", &pResolved->iPopupBorderColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.popup.shadow_color", &pResolved->iPopupShadowColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.popup.text_color", &pResolved->iPopupTextColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.popup.muted_text_color", &pResolved->iPopupMutedTextColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.accent.color", &pResolved->iAccentColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.field.color", &pResolved->iFieldColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.field.border_color", &pResolved->iFieldBorderColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.separator.color", &pResolved->iSeparatorColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.field.error_border_color", &pResolved->iErrorBorderColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.selection.color", &pResolved->iSelectionColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.selection.text_color", &pResolved->iSelectionTextColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.track.color", &pResolved->iTrackColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.knob.color", &pResolved->iKnobColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.marker.color", &pResolved->iMarkerColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.marker.border_color", &pResolved->iMarkerBorderColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.checker.light_color", &pResolved->iCheckerLightColor);
	(void)__xuiColorPickerStyleColor(pWidget, "colorpicker.checker.dark_color", &pResolved->iCheckerDarkColor);
	(void)__xuiColorPickerStyleFloat(pWidget, "colorpicker.border.width", &pResolved->fBorderWidth);
}

static int __xuiColorPickerFindPaletteColor(xui_color_picker_data_t* pData, uint32_t iColor)
{
	uint32_t iTarget;
	int i;

	if ( pData == NULL ) {
		return -1;
	}
	iTarget = pData->bAlphaEnabled ? iColor : __xuiColorPickerOpaque(iColor);
	for ( i = 0; i < pData->iPaletteCount; i++ ) {
		if ( (pData->bAlphaEnabled ? pData->arrPalette[i] : __xuiColorPickerOpaque(pData->arrPalette[i])) == iTarget ) {
			return i;
		}
	}
	return -1;
}

static void __xuiColorPickerSyncSelectedPalette(xui_color_picker_data_t* pData)
{
	if ( pData != NULL ) {
		pData->iSelectedPalette = __xuiColorPickerFindPaletteColor(pData, pData->iColor);
	}
}

static int __xuiColorPickerInvalidateAll(xui_widget pWidget, xui_color_picker_data_t* pData)
{
	int iRet;
	int iPanelRet;

	iRet = XUI_OK;
	if ( pWidget != NULL ) {
		iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	iPanelRet = XUI_OK;
	if ( (pData != NULL) && (pData->pPanel != NULL) ) {
		iPanelRet = xuiWidgetInvalidate(pData->pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return (iRet != XUI_OK) ? iRet : iPanelRet;
}

static void __xuiColorPickerNotify(xui_widget pWidget, xui_color_picker_data_t* pData)
{
	xui_color_picker_change_proc onChange;
	void* pUser;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return;
	}
	if ( pData->iChangeCount < INT_MAX ) pData->iChangeCount++;
	onChange = pData->onChange;
	pUser = pData->pChangeUser;
	if ( onChange != NULL ) {
		onChange(pWidget, pData->iColor, pUser);
	}
}

static int __xuiColorPickerSetColorInternal(xui_widget pWidget, xui_color_picker_data_t* pData, uint32_t iColor, int bNotify)
{
	uint32_t iOldColor;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iColor = __xuiColorPickerNormalizeColor(pData, iColor);
	if ( pData->iColor == iColor ) {
		return XUI_OK;
	}
	iOldColor = pData->iColor;
	pData->iColor = iColor;
	if ( (iOldColor & 0xffffff00u) != (iColor & 0xffffff00u) ) {
		__xuiColorPickerRgbToHsv(iColor, &pData->fHue, &pData->fSaturation, &pData->fValue);
	}
	__xuiColorPickerFormatHex(pData);
	__xuiColorPickerSyncSelectedPalette(pData);
	iRet = __xuiColorPickerInvalidateAll(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	if ( bNotify ) {
		__xuiColorPickerNotify(pWidget, pData);
	}
	return XUI_OK;
}

static int __xuiColorPickerSetHsvInternal(xui_widget pWidget, xui_color_picker_data_t* pData, float fHue, float fSat, float fVal, int bNotify)
{
	uint32_t iColor;
	uint32_t iRgb;
	float fNewHue;
	float fNewSaturation;
	float fNewValue;
	int bHsvChanged;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || !isfinite(fHue) || !isfinite(fSat) || !isfinite(fVal) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	fNewHue = __xuiColorPickerClamp01(fHue);
	fNewSaturation = __xuiColorPickerClamp01(fSat);
	fNewValue = __xuiColorPickerClamp01(fVal);
	bHsvChanged = pData->fHue != fNewHue ||
		pData->fSaturation != fNewSaturation || pData->fValue != fNewValue;
	iRgb = __xuiColorPickerHsvToRgb(fNewHue, fNewSaturation, fNewValue);
	iColor = XUI_COLOR_RGBA(__xuiColorPickerR(iRgb), __xuiColorPickerG(iRgb),
		__xuiColorPickerB(iRgb), __xuiColorPickerA(pData->iColor));
	pData->fHue = fNewHue;
	pData->fSaturation = fNewSaturation;
	pData->fValue = fNewValue;
	if ( pData->iColor == iColor ) {
		return bHsvChanged ? __xuiColorPickerInvalidateAll(pWidget, pData) : XUI_OK;
	}
	pData->iColor = iColor;
	__xuiColorPickerFormatHex(pData);
	__xuiColorPickerSyncSelectedPalette(pData);
	iRet = __xuiColorPickerInvalidateAll(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	if ( bNotify ) __xuiColorPickerNotify(pWidget, pData);
	return XUI_OK;
}

static int __xuiColorPickerSetChannelInternal(xui_widget pWidget, xui_color_picker_data_t* pData, int iChannel, int iValue, int bNotify)
{
	int iR;
	int iG;
	int iB;
	int iA;

	if ( (pWidget == NULL) || (pData == NULL) || (iChannel < 0) || (iChannel > 3) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (iChannel == 3) && !pData->bAlphaEnabled ) {
		return XUI_OK;
	}
	iR = __xuiColorPickerR(pData->iColor);
	iG = __xuiColorPickerG(pData->iColor);
	iB = __xuiColorPickerB(pData->iColor);
	iA = __xuiColorPickerA(pData->iColor);
	iValue = __xuiColorPickerClampByte(iValue);
	if ( iChannel == 0 ) iR = iValue;
	else if ( iChannel == 1 ) iG = iValue;
	else if ( iChannel == 2 ) iB = iValue;
	else iA = iValue;
	return __xuiColorPickerSetColorInternal(pWidget, pData, XUI_COLOR_RGBA(iR, iG, iB, iA), bNotify);
}

static uint32_t __xuiColorPickerState(xui_widget pWidget, xui_color_picker_data_t* pData)
{
	xui_context pContext;
	xui_widget pFocus;
	uint32_t iState;

	if ( pWidget == NULL ) {
		return 0u;
	}
	iState = xuiWidgetGetInputState(pWidget);
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) {
		iState |= XUI_WIDGET_STATE_DISABLED;
	}
	if ( (pData != NULL) && (pData->pPopup != NULL) && xuiPopupIsOpen(pData->pPopup) ) {
		iState |= XUI_COLOR_PICKER_STATE_OPEN;
	}
	if ( pData != NULL ) {
		pContext = xuiWidgetGetContext(pWidget);
		pFocus = xuiGetFocusWidget(pContext);
		if ( (pFocus == pWidget) || (pFocus == pData->pPanel) || (pFocus == pData->pPopup) ) {
			iState |= XUI_WIDGET_STATE_FOCUS;
		}
	}
	return iState;
}

static int __xuiColorPickerSyncState(xui_widget pWidget, xui_color_picker_data_t* pData)
{
	xui_context pContext;
	uint32_t iState;
	int iRet;

	pContext = xuiWidgetGetContext(pWidget);
	iState = __xuiColorPickerState(pWidget, pData);
	if ( xuiWidgetGetStateId(pWidget) == iState ) return XUI_OK;
	iRet = xuiWidgetSetStateId(pWidget, iState);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void __xuiColorPickerUpdateOwnerRects(xui_widget pWidget, xui_color_picker_data_t* pData)
{
	xui_rect_t tRect;
	float fContentW;
	float fContentH;
	float fButtonW;
	float fLeadingW;
	float fSwatch;
	float fSwatchX;
	float fTextX;
	float fTextRight;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return;
	}
	tRect = xuiWidgetGetRect(pWidget);
	fContentW = tRect.fW > 0 ? (float)tRect.fW : 0.0f;
	fContentH = tRect.fH > 0 ? (float)tRect.fH : 0.0f;
	fButtonW = fContentH;
	if ( fButtonW < 24.0f ) fButtonW = 24.0f;
	if ( fButtonW > 36.0f ) fButtonW = 36.0f;
	if ( fButtonW > fContentW ) fButtonW = fContentW;
	fLeadingW = fContentW - fButtonW;
	fSwatch = fContentH - 10.0f;
	if ( fSwatch < 12.0f ) fSwatch = 12.0f;
	if ( fSwatch > 22.0f ) fSwatch = 22.0f;
	if ( fSwatch > fContentH ) fSwatch = fContentH;
	fSwatchX = fLeadingW > 7.0f ? 7.0f : fLeadingW;
	if ( fSwatch > fLeadingW - fSwatchX ) fSwatch = fLeadingW - fSwatchX;
	if ( fSwatch < 0.0f ) fSwatch = 0.0f;
	fTextX = fSwatchX + fSwatch + (fSwatch > 0.0f ? 8.0f : 0.0f);
	if ( fTextX > fLeadingW ) fTextX = fLeadingW;
	fTextRight = fLeadingW - 7.0f;
	if ( fTextRight < fTextX ) fTextRight = fTextX;
	pData->tButtonRect = xuiInternalRectFromFloatNearest(fContentW - fButtonW, 0.0f,
		fButtonW, fContentH);
	pData->tSwatchRect = xuiInternalRectFromFloatNearest(fSwatchX,
		(fContentH - fSwatch) * 0.5f, fSwatch, fSwatch);
	pData->tTextRect = xuiInternalRectFromFloatNearest(fTextX, 0.0f,
		fTextRight - fTextX, fContentH);
}

static void __xuiColorPickerUpdatePanelRects(xui_color_picker_data_t* pData)
{
	xui_rect_t tRect;
	float fW;
	float fH;
	float fRightX;
	float fControlX;
	float fControlRight;
	float fControlW;
	float fRowY;
	float fRowStep;
	float fPaletteCell;
	float fPaletteGap;
	float fPaletteWidth;
	float fPaletteX;
	float fSwatchGap;
	float fSwatchW;
	int i;

	if ( pData == NULL ) {
		return;
	}
	fW = pData->fPopupWidth;
	fH = pData->fPopupHeight;
	if ( pData->pPanel != NULL ) {
		tRect = xuiWidgetGetRect(pData->pPanel);
		if ( tRect.fW > 0.0f ) fW = tRect.fW;
		if ( tRect.fH > 0.0f ) fH = tRect.fH;
	}
	if ( fW < 320.0f ) fW = 320.0f;
	if ( fH < 260.0f ) fH = 260.0f;
	pData->tSvRect = xuiInternalSnapRect((xui_rect_t){14.0f, 16.0f, 190.0f, 172.0f});
	pData->tHueRect = xuiInternalSnapRect((xui_rect_t){214.0f, 16.0f, 22.0f, 172.0f});
	fRightX = 254.0f;
	fControlX = fRightX + 22.0f;
	fControlRight = fW - XUI_COLOR_PICKER_PANEL_PAD;
	fControlW = __xuiColorPickerMax(84.0f, fControlRight - fControlX);
	fSwatchGap = 14.0f;
	fSwatchW = (fControlW - fSwatchGap) * 0.5f;
	pData->tOldRect = xuiInternalSnapRect((xui_rect_t){fControlX, 30.0f, fSwatchW, 28.0f});
	pData->tNewRect = xuiInternalSnapRect((xui_rect_t){fControlX + fSwatchW + fSwatchGap, 30.0f, fSwatchW, 28.0f});
	fRowY = pData->bAlphaEnabled ? 64.0f : 72.0f;
	fRowStep = pData->bAlphaEnabled ? 25.0f : 32.0f;
	for ( i = 0; i < 4; i++ ) {
		pData->arrFieldRect[i] = xuiInternalSnapRect((xui_rect_t){fControlX, fRowY + (float)i * fRowStep, 42.0f, 22.0f});
		pData->arrSliderRect[i] = xuiInternalSnapRect((xui_rect_t){fControlX + 56.0f, fRowY + (float)i * fRowStep + 8.0f, __xuiColorPickerMax(36.0f, fControlRight - (fControlX + 56.0f)), 6.0f});
	}
	pData->tAlphaRect = pData->arrSliderRect[3];
	pData->tHexRect = xuiInternalSnapRect((xui_rect_t){fControlX, pData->tSvRect.fY + pData->tSvRect.fH - 24.0f, fControlW, 24.0f});
	fPaletteCell = 20.0f;
	fPaletteGap = (pData->iPaletteCount > 1) ? ((fW - 28.0f - (float)pData->iPaletteCount * fPaletteCell) / (float)(pData->iPaletteCount - 1)) : 0.0f;
	if ( fPaletteGap < 4.0f ) fPaletteGap = 4.0f;
	if ( fPaletteGap > 16.0f ) fPaletteGap = 16.0f;
	fPaletteWidth = (float)pData->iPaletteCount * fPaletteCell + ((pData->iPaletteCount > 1) ? (float)(pData->iPaletteCount - 1) * fPaletteGap : 0.0f);
	fPaletteX = (fW - fPaletteWidth) * 0.5f;
	for ( i = 0; i < pData->iPaletteCount; i++ ) {
		pData->arrPaletteRect[i] = xuiInternalSnapRect((xui_rect_t){fPaletteX + (float)i * (fPaletteCell + fPaletteGap), fH - 38.0f, fPaletteCell, fPaletteCell});
	}
}

static int __xuiColorPickerDrawFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( __xuiColorPickerAlpha(iColor) == 0 ) {
		return XUI_OK;
	}
	return (pProxy->drawRectFill != NULL) ? pProxy->drawRectFill(pProxy, pDraw, tRect, iColor) : XUI_OK;
}

static int __xuiColorPickerDrawStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (fWidth <= 0.0f) || (__xuiColorPickerAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return (pProxy->drawRectStroke != NULL) ? pProxy->drawRectStroke(pProxy, pDraw, tRect, fWidth, iColor) : XUI_OK;
}

static int __xuiColorPickerDrawChecker(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iOverlay, const xui_color_picker_data_t* pResolved)
{
	xui_rect_t tCell;
	float fCellSize;
	float fRight;
	float fBottom;
	float fX;
	float fY;
	int i;
	int j;
	uint32_t iA;
	uint32_t iB;

	if ( pProxy->drawRectFill == NULL ) {
		return XUI_OK;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	iA = pResolved->iCheckerLightColor;
	iB = pResolved->iCheckerDarkColor;
	fCellSize = 6.0f;
	fRight = tRect.fX + tRect.fW;
	fBottom = tRect.fY + tRect.fH;
	for ( fY = tRect.fY, j = 0; fY < fBottom; fY += fCellSize, j++ ) {
		for ( fX = tRect.fX, i = 0; fX < fRight; fX += fCellSize, i++ ) {
			tCell.fX = fX;
			tCell.fY = fY;
			tCell.fW = __xuiColorPickerMin(fCellSize, fRight - fX);
			tCell.fH = __xuiColorPickerMin(fCellSize, fBottom - fY);
			if ( (tCell.fW <= 0.0f) || (tCell.fH <= 0.0f) ) {
				continue;
			}
			{
				int iRet = __xuiColorPickerDrawFill(pProxy, pDraw, xuiInternalSnapRect(tCell), (((i + j) & 1) != 0) ? iA : iB);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	/* Composite the content separately so transparent checker styles stay transparent. */
	return __xuiColorPickerDrawFill(pProxy, pDraw, tRect, iOverlay);
}

static int __xuiColorPickerDrawSwatch(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor, uint32_t iBorder, const xui_color_picker_data_t* pResolved)
{
	int iRet;

	if ( __xuiColorPickerAlpha(iColor) < 255 ) {
		iRet = __xuiColorPickerDrawChecker(pProxy, pDraw, tRect, iColor, pResolved);
		if ( iRet != XUI_OK ) return iRet;
	} else {
		iRet = __xuiColorPickerDrawFill(pProxy, pDraw, tRect, iColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	return __xuiColorPickerDrawStroke(pProxy, pDraw, tRect, 1.0f, iBorder);
}

static int __xuiColorPickerDrawChevron(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, int bOpen, uint32_t iColor)
{
	float fCx;
	float fCy;
	float fLeft;
	float fRight;
	float fTop;
	float fBottom;
	int iRet;

	if ( pProxy->drawLine == NULL ) {
		return XUI_OK;
	}
	fCx = tRect.fX + tRect.fW * 0.5f;
	fCy = tRect.fY + tRect.fH * 0.5f;
	fLeft = fCx - 4.0f;
	fRight = fCx + 4.0f;
	fTop = fCy - 2.0f;
	fBottom = fCy + 2.0f;
	if ( bOpen ) {
		iRet = pProxy->drawLine(pProxy, pDraw, fLeft, fBottom, fCx, fTop, 1.7f, iColor);
		if ( iRet != XUI_OK ) return iRet;
		return pProxy->drawLine(pProxy, pDraw, fCx, fTop, fRight, fBottom, 1.7f, iColor);
	}
	iRet = pProxy->drawLine(pProxy, pDraw, fLeft, fTop, fCx, fBottom, 1.7f, iColor);
	if ( iRet != XUI_OK ) return iRet;
	return pProxy->drawLine(pProxy, pDraw, fCx, fBottom, fRight, fTop, 1.7f, iColor);
}

static int __xuiColorPickerDrawGradientQuad(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect,
	uint32_t iTopLeft, uint32_t iTopRight, uint32_t iBottomRight, uint32_t iBottomLeft)
{
	static const uint32_t arrIndices[6] = {0u, 1u, 2u, 0u, 2u, 3u};
	xui_mesh_vertex_t arrVertices[4];

	if ( (pProxy == NULL) || (pProxy->drawMeshTriangles == NULL) ||
	     (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	arrVertices[0] = (xui_mesh_vertex_t){tRect.fX, tRect.fY, iTopLeft};
	arrVertices[1] = (xui_mesh_vertex_t){tRect.fX + tRect.fW, tRect.fY, iTopRight};
	arrVertices[2] = (xui_mesh_vertex_t){tRect.fX + tRect.fW, tRect.fY + tRect.fH, iBottomRight};
	arrVertices[3] = (xui_mesh_vertex_t){tRect.fX, tRect.fY + tRect.fH, iBottomLeft};
	return pProxy->drawMeshTriangles(pProxy, pDraw, arrVertices, 4, arrIndices, 6, 0);
}

static int __xuiColorPickerDrawHue(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect)
{
	xui_mesh_vertex_t arrVertices[14];
	uint32_t arrIndices[36];
	float fY;
	uint32_t iColor;
	int i;

	if ( (pProxy == NULL) || (pProxy->drawMeshTriangles == NULL) ) {
		return XUI_OK;
	}
	tRect.fX += 1.0f;
	tRect.fY += 1.0f;
	tRect.fW -= 2.0f;
	tRect.fH -= 2.0f;
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	for ( i = 0; i <= 6; i++ ) {
		fY = tRect.fY + tRect.fH * (float)i / 6.0f;
		iColor = __xuiColorPickerHsvToRgb((float)i / 6.0f, 1.0f, 1.0f);
		arrVertices[i * 2] = (xui_mesh_vertex_t){tRect.fX, fY, iColor};
		arrVertices[i * 2 + 1] = (xui_mesh_vertex_t){tRect.fX + tRect.fW, fY, iColor};
		if ( i < 6 ) {
			int iVertex = i * 2;
			int iIndex = i * 6;
			arrIndices[iIndex] = (uint32_t)iVertex;
			arrIndices[iIndex + 1] = (uint32_t)(iVertex + 1);
			arrIndices[iIndex + 2] = (uint32_t)(iVertex + 3);
			arrIndices[iIndex + 3] = (uint32_t)iVertex;
			arrIndices[iIndex + 4] = (uint32_t)(iVertex + 3);
			arrIndices[iIndex + 5] = (uint32_t)(iVertex + 2);
		}
	}
	return pProxy->drawMeshTriangles(pProxy, pDraw, arrVertices, 14, arrIndices, 36, 0);
}

static int __xuiColorPickerDrawSv(xui_proxy pProxy, xui_draw_context pDraw, xui_color_picker_data_t* pData)
{
	xui_rect_t tRect;
	uint32_t iHue;
	int iRet;

	if ( (pProxy == NULL) || (pProxy->drawMeshTriangles == NULL) || (pData == NULL) ) {
		return XUI_OK;
	}
	tRect = pData->tSvRect;
	tRect.fX += 1.0f;
	tRect.fY += 1.0f;
	tRect.fW -= 2.0f;
	tRect.fH -= 2.0f;
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return XUI_OK;
	}
	iHue = __xuiColorPickerHsvToRgb(pData->fHue, 1.0f, 1.0f);
	iRet = __xuiColorPickerDrawGradientQuad(pProxy, pDraw, tRect,
		XUI_COLOR_RGBA(255, 255, 255, 255), iHue, iHue, XUI_COLOR_RGBA(255, 255, 255, 255));
	if ( iRet != XUI_OK ) return iRet;
	return __xuiColorPickerDrawGradientQuad(pProxy, pDraw, tRect,
		XUI_COLOR_RGBA(0, 0, 0, 0), XUI_COLOR_RGBA(0, 0, 0, 0),
		XUI_COLOR_RGBA(0, 0, 0, 255), XUI_COLOR_RGBA(0, 0, 0, 255));
}

static int __xuiColorPickerDrawTrack(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tTrack, float fRate, const xui_color_picker_data_t* pResolved)
{
	xui_rect_t tFill;
	float fKnobX;
	int iRet;

	fRate = __xuiColorPickerClamp01(fRate);
	iRet = __xuiColorPickerDrawFill(pProxy, pDraw, tTrack, pResolved->iTrackColor);
	if ( iRet != XUI_OK ) return iRet;
	tFill = tTrack;
	tFill.fW *= fRate;
	iRet = __xuiColorPickerDrawFill(pProxy, pDraw, tFill, pResolved->iAccentColor);
	if ( iRet != XUI_OK ) return iRet;
	fKnobX = tTrack.fX + tTrack.fW * fRate;
	if ( pProxy->drawCircleFill != NULL ) {
		iRet = pProxy->drawCircleFill(pProxy, pDraw, fKnobX, tTrack.fY + tTrack.fH * 0.5f, 5.5f, pResolved->iKnobColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pProxy->drawCircleStroke != NULL ) {
		return pProxy->drawCircleStroke(pProxy, pDraw, fKnobX, tTrack.fY + tTrack.fH * 0.5f, 5.5f, 1.0f, pResolved->iFieldBorderColor);
	}
	return XUI_OK;
}

static int __xuiColorPickerSyncStyle(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	xui_color_picker_data_t tResolved;
	xui_context pContext;
	uint32_t iHash = xuiWidgetGetStyleHash(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	if ( pData->bChildStyleSynced && pData->iChildStyleHash == iHash ) return XUI_OK;
	__xuiColorPickerResolve(pWidget, pData, &tResolved);
	if ( pData->pPopup != NULL ) {
		iRet = xuiPopupSetColors(pData->pPopup, tResolved.iPopupPanelColor,
			tResolved.iPopupBorderColor, tResolved.iPopupShadowColor, 0u);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	}
	if ( pData->pPanel != NULL )
	{
		iRet = xuiWidgetInvalidate(pData->pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
	}
	pData->iChildStyleHash = iHash;
	pData->bChildStyleSynced = 1;
	return XUI_OK;
}

static int __xuiColorPickerCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_color_picker_data_t* pData;
	xui_color_picker_data_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;
	uint32_t iState;
	uint32_t iBackground;
	uint32_t iBorder;
	uint32_t iText;
	uint32_t iArrow;
	uint32_t iButton;
	int iRet;

	(void)iStateId;
	(void)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiColorPickerResolve(pWidget, pData, &tResolved);
	__xuiColorPickerUpdateOwnerRects(pWidget, pData);
	iState = __xuiColorPickerState(pWidget, pData);
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect = xuiInternalSnapRect(tRect);
	iBackground = tResolved.iBackgroundColor;
	iBorder = tResolved.iBorderColor;
	iText = tResolved.iTextColor;
	iArrow = tResolved.iArrowColor;
	iButton = tResolved.iButtonColor;
	if ( (iState & XUI_WIDGET_STATE_DISABLED) != 0u ) {
		iBackground = tResolved.iDisabledBackgroundColor;
		iBorder = __xuiColorPickerColorWithAlpha(tResolved.iBorderColor, 120);
		iText = tResolved.iDisabledTextColor;
		iArrow = tResolved.iDisabledArrowColor;
	} else if ( (iState & XUI_COLOR_PICKER_STATE_OPEN) != 0u ) {
		iBackground = tResolved.iOpenBackgroundColor;
		iBorder = tResolved.iFocusBorderColor;
		iButton = tResolved.iButtonOpenColor;
	} else if ( (iState & XUI_WIDGET_STATE_FOCUS) != 0u ) {
		iBorder = tResolved.iFocusBorderColor;
	} else if ( (iState & XUI_WIDGET_STATE_HOVER) != 0u ) {
		iBackground = tResolved.iHoverBackgroundColor;
		iBorder = tResolved.iHoverBorderColor;
		iButton = tResolved.iButtonHoverColor;
	}
	iRet = __xuiColorPickerDrawFill(pProxy, pDraw, tRect, iBackground);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiColorPickerDrawFill(pProxy, pDraw, pData->tButtonRect, iButton);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiColorPickerDrawSwatch(pProxy, pDraw, pData->tSwatchRect, pData->iColor, __xuiColorPickerColorWithAlpha(iBorder, 190), &tResolved);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pProxy->drawText != NULL) && (__xuiColorPickerAlpha(iText) != 0) ) {
		iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=tResolved.pFont, .sText=pData->sHex, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, pData->tTextRect, iText, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiColorPickerDrawChevron(pProxy, pDraw, pData->tButtonRect, (iState & XUI_COLOR_PICKER_STATE_OPEN) != 0u, iArrow);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiColorPickerDrawStroke(pProxy, pDraw, tRect, tResolved.fBorderWidth, iBorder);
}

static int __xuiColorPickerPanelRender(xui_widget pPanel, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_widget pOwner;
	xui_color_picker_data_t* pData;
	xui_color_picker_data_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tLabel;
	xui_rect_t tLine;
	xui_vec2_t tA;
	xui_vec2_t tB;
	xui_vec2_t tC;
	const char* arrName[4] = {"R", "G", "B", "A"};
	char sText[32];
	int arrValue[4];
	int i;
	int iCount;
	int bKeyboardFocus;
	float fX;
	float fY;
	float fRate;
	int iRet;

	(void)iStateId;
	pOwner = (xui_widget)pUser;
	if ( (pPanel == NULL) || (pDraw == NULL) || (pOwner == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiColorPickerGetData(pOwner);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pOwner));
	if ( pProxy == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	__xuiColorPickerResolve(pOwner, pData, &tResolved);
	__xuiColorPickerUpdatePanelRects(pData);
	bKeyboardFocus = pData->iKeyboardPart != XUI_COLOR_PICKER_PART_NONE &&
		xuiGetFocusWidget(xuiWidgetGetContext(pPanel)) == pPanel;

	iRet = __xuiColorPickerDrawSv(pProxy, pDraw, pData);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiColorPickerDrawStroke(pProxy, pDraw, pData->tSvRect,
		(bKeyboardFocus && pData->iKeyboardPart == XUI_COLOR_PICKER_PART_SV) ? 2.0f : 1.0f,
		(bKeyboardFocus && pData->iKeyboardPart == XUI_COLOR_PICKER_PART_SV) ? tResolved.iAccentColor : tResolved.iFieldBorderColor);
	if ( iRet != XUI_OK ) return iRet;
	fX = pData->tSvRect.fX + pData->tSvRect.fW * pData->fSaturation;
	fY = pData->tSvRect.fY + pData->tSvRect.fH * (1.0f - pData->fValue);
	if ( pProxy->drawCircleStroke != NULL ) {
		iRet = pProxy->drawCircleStroke(pProxy, pDraw, fX, fY, 7.0f, 2.0f, tResolved.iMarkerColor);
		if ( iRet != XUI_OK ) return iRet;
		iRet = pProxy->drawCircleStroke(pProxy, pDraw, fX, fY, 8.0f, 1.0f, tResolved.iMarkerBorderColor);
		if ( iRet != XUI_OK ) return iRet;
	}

	iRet = __xuiColorPickerDrawHue(pProxy, pDraw, pData->tHueRect);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiColorPickerDrawStroke(pProxy, pDraw, pData->tHueRect,
		(bKeyboardFocus && pData->iKeyboardPart == XUI_COLOR_PICKER_PART_HUE) ? 2.0f : 1.0f,
		(bKeyboardFocus && pData->iKeyboardPart == XUI_COLOR_PICKER_PART_HUE) ? tResolved.iAccentColor : tResolved.iFieldBorderColor);
	if ( iRet != XUI_OK ) return iRet;
	fY = pData->tHueRect.fY + pData->tHueRect.fH * pData->fHue;
	if ( pProxy->drawTriangleFill != NULL ) {
		tA = (xui_vec2_t){pData->tHueRect.fX - 7.0f, fY - 5.0f};
		tB = (xui_vec2_t){pData->tHueRect.fX - 1.0f, fY};
		tC = (xui_vec2_t){pData->tHueRect.fX - 7.0f, fY + 5.0f};
		iRet = pProxy->drawTriangleFill(pProxy, pDraw, tA, tB, tC, tResolved.iMarkerColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pProxy->drawRectFill != NULL ) {
		tLine = (xui_rect_t){pData->tHueRect.fX - 1.0f, fY - 1.0f, pData->tHueRect.fW + 8.0f, 2.0f};
		iRet = pProxy->drawRectFill(pProxy, pDraw, tLine, tResolved.iMarkerColor);
		if ( iRet != XUI_OK ) return iRet;
	}

	if ( pProxy->drawText != NULL ) {
		tLabel = (xui_rect_t){pData->tOldRect.fX, pData->tOldRect.fY - 16.0f, pData->tOldRect.fW, 14.0f};
		iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=tResolved.pFont, .sText="Old", .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tLabel, tResolved.iPopupMutedTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
		tLabel.fX = pData->tNewRect.fX;
		iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=tResolved.pFont, .sText="New", .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tLabel, tResolved.iPopupMutedTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiColorPickerDrawSwatch(pProxy, pDraw, pData->tOldRect, pData->iOldColor,
		(bKeyboardFocus && pData->iKeyboardPart == XUI_COLOR_PICKER_PART_OLD) ? tResolved.iAccentColor : tResolved.iFieldBorderColor,
		&tResolved);
	if ( iRet != XUI_OK ) return iRet;
	if ( bKeyboardFocus && pData->iKeyboardPart == XUI_COLOR_PICKER_PART_OLD ) {
		iRet = __xuiColorPickerDrawStroke(pProxy, pDraw, pData->tOldRect, 2.0f, tResolved.iAccentColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiColorPickerDrawSwatch(pProxy, pDraw, pData->tNewRect, pData->iColor, tResolved.iFieldBorderColor, &tResolved);
	if ( iRet != XUI_OK ) return iRet;

	arrValue[0] = __xuiColorPickerR(pData->iColor);
	arrValue[1] = __xuiColorPickerG(pData->iColor);
	arrValue[2] = __xuiColorPickerB(pData->iColor);
	arrValue[3] = __xuiColorPickerA(pData->iColor);
	iCount = pData->bAlphaEnabled ? 4 : 3;
	for ( i = 0; i < iCount; i++ ) {
		if ( pProxy->drawText != NULL ) {
			tLabel = pData->arrFieldRect[i];
			tLabel.fX -= 22.0f;
			tLabel.fW = 18.0f;
			iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=tResolved.pFont, .sText=arrName[i], .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tLabel, tResolved.iPopupMutedTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
		iRet = __xuiColorPickerDrawFill(pProxy, pDraw, pData->arrFieldRect[i], tResolved.iFieldColor);
		if ( iRet != XUI_OK ) return iRet;
		if ( pData->iEditingChannel == i && pData->bEditSelectAll && pProxy->drawRectFill != NULL ) {
			tLine = pData->arrFieldRect[i];
			tLine.fX += 3.0f;
			tLine.fY += 3.0f;
			tLine.fW -= 6.0f;
			tLine.fH -= 6.0f;
			iRet = pProxy->drawRectFill(pProxy, pDraw, xuiInternalSnapRect(tLine), tResolved.iSelectionColor);
			if ( iRet != XUI_OK ) return iRet;
		}
		iRet = __xuiColorPickerDrawStroke(pProxy, pDraw, pData->arrFieldRect[i], 1.0f,
			(pData->iEditingChannel == i && pData->bEditError) ? tResolved.iErrorBorderColor :
			((pData->iActiveChannel == i || pData->iEditingChannel == i ||
			  (bKeyboardFocus && pData->iKeyboardPart == __xuiColorPickerChannelPart(i))) ?
			 tResolved.iAccentColor : tResolved.iFieldBorderColor));
		if ( iRet != XUI_OK ) return iRet;
		if ( pProxy->drawText != NULL ) {
			if ( pData->iEditingChannel == i ) {
				snprintf(sText, sizeof(sText), "%s", pData->sEdit);
			} else {
				snprintf(sText, sizeof(sText), "%d", arrValue[i]);
			}
			sText[sizeof(sText) - 1] = '\0';
			iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=tResolved.pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, pData->arrFieldRect[i], (pData->iEditingChannel == i && pData->bEditSelectAll) ? tResolved.iSelectionTextColor : tResolved.iPopupTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
		fRate = (float)arrValue[i] / 255.0f;
		iRet = __xuiColorPickerDrawTrack(pProxy, pDraw, pData->arrSliderRect[i], fRate, &tResolved);
		if ( iRet != XUI_OK ) return iRet;
	}

	if ( pProxy->drawText != NULL ) {
		tLabel = pData->tHexRect;
		tLabel.fX -= 22.0f;
		tLabel.fW = 18.0f;
		iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=tResolved.pFont, .sText="#", .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tLabel, tResolved.iPopupMutedTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiColorPickerDrawFill(pProxy, pDraw, pData->tHexRect, tResolved.iFieldColor);
	if ( iRet != XUI_OK ) return iRet;
	if ( pData->bEditingHex && pData->bEditSelectAll && pProxy->drawRectFill != NULL ) {
		tLine = pData->tHexRect;
		tLine.fX += 3.0f;
		tLine.fY += 3.0f;
		tLine.fW -= 6.0f;
		tLine.fH -= 6.0f;
		iRet = pProxy->drawRectFill(pProxy, pDraw, xuiInternalSnapRect(tLine), tResolved.iSelectionColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiColorPickerDrawStroke(pProxy, pDraw, pData->tHexRect, 1.0f,
		(pData->bEditingHex && pData->bEditError) ? tResolved.iErrorBorderColor :
		((pData->bEditingHex || (bKeyboardFocus && pData->iKeyboardPart == XUI_COLOR_PICKER_PART_HEX)) ?
		 tResolved.iAccentColor : tResolved.iFieldBorderColor));
	if ( iRet != XUI_OK ) return iRet;
	if ( pProxy->drawText != NULL ) {
		iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=tResolved.pFont, .sText=pData->bEditingHex ? pData->sEdit : (pData->sHex + 1), .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, pData->tHexRect, (pData->bEditingHex && pData->bEditSelectAll) ? tResolved.iSelectionTextColor : tResolved.iPopupTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}

	if ( pProxy->drawRectFill != NULL && pData->iPaletteCount > 0 ) {
		tLine = (xui_rect_t){XUI_COLOR_PICKER_PANEL_PAD, pData->arrPaletteRect[0].fY - 14.0f, xuiWidgetGetRect(pPanel).fW - XUI_COLOR_PICKER_PANEL_PAD * 2.0f, 1.0f};
		iRet = pProxy->drawRectFill(pProxy, pDraw, tLine, tResolved.iSeparatorColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( i = 0; i < pData->iPaletteCount; i++ ) {
		iRet = __xuiColorPickerDrawSwatch(pProxy, pDraw, pData->arrPaletteRect[i], pData->arrPalette[i], (i == pData->iSelectedPalette) ? tResolved.iAccentColor : tResolved.iFieldBorderColor, &tResolved);
		if ( iRet != XUI_OK ) return iRet;
		if ( i == pData->iSelectedPalette ||
		     (bKeyboardFocus && pData->iKeyboardPart == XUI_COLOR_PICKER_PART_PALETTE && i == pData->iKeyboardPalette) ) {
			iRet = __xuiColorPickerDrawStroke(pProxy, pDraw, pData->arrPaletteRect[i], 2.0f, tResolved.iAccentColor);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiColorPickerHitPanel(xui_color_picker_data_t* pData, float fX, float fY)
{
	int i;
	int iCount;

	if ( pData == NULL ) {
		return XUI_COLOR_PICKER_PART_NONE;
	}
	if ( __xuiColorPickerRectContains(pData->tSvRect, fX, fY) ) return XUI_COLOR_PICKER_PART_SV;
	if ( __xuiColorPickerRectContains(pData->tHueRect, fX, fY) ) return XUI_COLOR_PICKER_PART_HUE;
	if ( __xuiColorPickerRectContains(pData->tOldRect, fX, fY) ) return XUI_COLOR_PICKER_PART_OLD;
	if ( __xuiColorPickerRectContains(pData->tNewRect, fX, fY) ) return XUI_COLOR_PICKER_PART_NEW;
	iCount = pData->bAlphaEnabled ? 4 : 3;
	for ( i = 0; i < iCount; i++ ) {
		if ( __xuiColorPickerRectContains(pData->arrFieldRect[i], fX, fY) ||
		     __xuiColorPickerRectContains(__xuiColorPickerTrackHitRect(pData->arrSliderRect[i]), fX, fY) ) {
			return __xuiColorPickerChannelPart(i);
		}
	}
	if ( __xuiColorPickerRectContains(pData->tHexRect, fX, fY) ) return XUI_COLOR_PICKER_PART_HEX;
	for ( i = 0; i < pData->iPaletteCount; i++ ) {
		if ( __xuiColorPickerRectContains(pData->arrPaletteRect[i], fX, fY) ) {
			return XUI_COLOR_PICKER_PART_PALETTE;
		}
	}
	return XUI_COLOR_PICKER_PART_NONE;
}

static int __xuiColorPickerPickSv(xui_widget pOwner, xui_color_picker_data_t* pData, float fX, float fY)
{
	float fS;
	float fV;

	fS = (fX - pData->tSvRect.fX) / pData->tSvRect.fW;
	fV = 1.0f - ((fY - pData->tSvRect.fY) / pData->tSvRect.fH);
	return __xuiColorPickerSetHsvInternal(pOwner, pData, pData->fHue, fS, fV, 1);
}

static int __xuiColorPickerPickHue(xui_widget pOwner, xui_color_picker_data_t* pData, float fY)
{
	float fHue;

	fHue = (fY - pData->tHueRect.fY) / pData->tHueRect.fH;
	return __xuiColorPickerSetHsvInternal(pOwner, pData, fHue, pData->fSaturation, pData->fValue, 1);
}

static int __xuiColorPickerPickChannel(xui_widget pOwner, xui_color_picker_data_t* pData, int iChannel, float fX)
{
	xui_rect_t tTrack;
	float fRate;

	if ( (pData == NULL) || (iChannel < 0) || (iChannel > 3) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tTrack = pData->arrSliderRect[iChannel];
	fRate = (tTrack.fW <= 0.0f) ? 0.0f : ((fX - tTrack.fX) / tTrack.fW);
	return __xuiColorPickerSetChannelInternal(pOwner, pData, iChannel, (int)(__xuiColorPickerClamp01(fRate) * 255.0f + 0.5f), 1);
}

static int __xuiColorPickerBeginHexEdit(xui_widget pOwner, xui_color_picker_data_t* pData)
{
	if ( (pOwner == NULL) || (pData == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	snprintf(pData->sEdit, sizeof(pData->sEdit), "%s", pData->sHex + 1);
	pData->sEdit[sizeof(pData->sEdit) - 1] = '\0';
	pData->iEditLen = (int)strlen(pData->sEdit);
	pData->bEditingHex = 1;
	pData->iEditingChannel = -1;
	pData->bEditSelectAll = 1;
	pData->bEditError = 0;
	return (pData->pPanel != NULL) ? xuiWidgetInvalidate(pData->pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) : XUI_OK;
}

static int __xuiColorPickerBeginChannelEdit(xui_color_picker_data_t* pData, int iChannel)
{
	int iValue;

	if ( (pData == NULL) || (iChannel < 0) || (iChannel > 3) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iValue = (iChannel == 0) ? __xuiColorPickerR(pData->iColor) :
		((iChannel == 1) ? __xuiColorPickerG(pData->iColor) :
		((iChannel == 2) ? __xuiColorPickerB(pData->iColor) : __xuiColorPickerA(pData->iColor)));
	snprintf(pData->sEdit, sizeof(pData->sEdit), "%d", iValue);
	pData->sEdit[sizeof(pData->sEdit) - 1] = '\0';
	pData->iEditLen = (int)strlen(pData->sEdit);
	pData->bEditingHex = 0;
	pData->iEditingChannel = iChannel;
	pData->bEditSelectAll = 1;
	pData->bEditError = 0;
	return (pData->pPanel != NULL) ? xuiWidgetInvalidate(pData->pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) : XUI_OK;
}

static int __xuiColorPickerCancelEdit(xui_color_picker_data_t* pData)
{
	int bChanged;

	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	bChanged = pData->bEditingHex || (pData->iEditingChannel >= 0) ||
		pData->bEditSelectAll || pData->bEditError || (pData->iEditLen > 0);
	pData->bEditingHex = 0;
	pData->iEditingChannel = -1;
	pData->bEditSelectAll = 0;
	pData->bEditError = 0;
	pData->iEditLen = 0;
	pData->sEdit[0] = '\0';
	return (bChanged && pData->pPanel != NULL) ? xuiWidgetInvalidate(pData->pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) : XUI_OK;
}

static int __xuiColorPickerEditing(const xui_color_picker_data_t* pData)
{
	return (pData != NULL) && (pData->bEditingHex || pData->iEditingChannel >= 0);
}

static int __xuiColorPickerSetKeyboardPart(xui_widget pPanel, xui_color_picker_data_t* pData, int iPart)
{
	int bChanged;

	if ( pPanel == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bChanged = pData->iKeyboardPart != iPart;
	pData->iKeyboardPart = iPart;
	if ( iPart == XUI_COLOR_PICKER_PART_PALETTE &&
	     (pData->iKeyboardPalette < 0 || pData->iKeyboardPalette >= pData->iPaletteCount) ) {
		pData->iKeyboardPalette = pData->iSelectedPalette >= 0 ? pData->iSelectedPalette : 0;
		bChanged = 1;
	}
	return bChanged ? xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) : XUI_OK;
}

static int __xuiColorPickerMoveKeyboardPart(xui_widget pPanel, xui_color_picker_data_t* pData, int iDirection)
{
	int arrParts[9];
	int iCount;
	int i;
	int iCurrent;

	if ( pPanel == NULL || pData == NULL || iDirection == 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	iCount = __xuiColorPickerKeyboardParts(pData, arrParts, (int)(sizeof(arrParts) / sizeof(arrParts[0])));
	if ( iCount <= 0 ) return XUI_OK;
	iCurrent = -1;
	for ( i = 0; i < iCount; i++ ) {
		if ( arrParts[i] == pData->iKeyboardPart ) {
			iCurrent = i;
			break;
		}
	}
	if ( iCurrent < 0 ) iCurrent = iDirection > 0 ? 0 : (iCount - 1);
	else iCurrent = (iCurrent + (iDirection > 0 ? 1 : iCount - 1)) % iCount;
	return __xuiColorPickerSetKeyboardPart(pPanel, pData, arrParts[iCurrent]);
}

static int __xuiColorPickerCommitHexEdit(xui_widget pOwner, xui_color_picker_data_t* pData)
{
	uint32_t iColor;
	int iRet;

	if ( (pOwner == NULL) || (pData == NULL) || !pData->bEditingHex ) {
		return XUI_OK;
	}
	if ( __xuiColorPickerParseHex(pData->sEdit, &iColor) != XUI_OK ) {
		pData->bEditError = 1;
		if ( pData->pPanel != NULL ) {
			iRet = xuiWidgetInvalidate(pData->pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiColorPickerCancelEdit(pData);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiColorPickerSetColorInternal(pOwner, pData, iColor, 1);
}

static int __xuiColorPickerCommitChannelEdit(xui_widget pOwner, xui_color_picker_data_t* pData)
{
	int iValue;
	int i;
	int iChannel;
	int iRet;

	if ( (pOwner == NULL) || (pData == NULL) || (pData->iEditingChannel < 0) ) {
		return XUI_OK;
	}
	if ( pData->iEditLen <= 0 ) {
		pData->bEditError = 1;
		if ( pData->pPanel != NULL ) {
			iRet = xuiWidgetInvalidate(pData->pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iValue = 0;
	for ( i = 0; i < pData->iEditLen; i++ ) {
		if ( pData->sEdit[i] < '0' || pData->sEdit[i] > '9' ) {
			pData->bEditError = 1;
			if ( pData->pPanel != NULL ) {
				iRet = xuiWidgetInvalidate(pData->pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
				if ( iRet != XUI_OK ) return iRet;
			}
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		iValue = iValue * 10 + (pData->sEdit[i] - '0');
	}
	iChannel = pData->iEditingChannel;
	iRet = __xuiColorPickerCancelEdit(pData);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiColorPickerSetChannelInternal(pOwner, pData, iChannel, __xuiColorPickerClampByte(iValue), 1);
}

static int __xuiColorPickerCommitEdit(xui_widget pOwner, xui_color_picker_data_t* pData)
{
	if ( (pData == NULL) || !__xuiColorPickerEditing(pData) ) {
		return XUI_OK;
	}
	return pData->bEditingHex ? __xuiColorPickerCommitHexEdit(pOwner, pData) : __xuiColorPickerCommitChannelEdit(pOwner, pData);
}

int xuiInternalColorPickerCommitPendingEdit(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiColorPickerCommitEdit(pWidget, pData);
}

static int __xuiColorPickerCancelPointer(xui_widget pPanel, xui_color_picker_data_t* pData)
{
	xui_context pContext;
	int bChanged;
	int iRet;

	if ( (pPanel == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pPanel);
	bChanged = (pData->iActivePart != XUI_COLOR_PICKER_PART_NONE) || (pData->iActiveChannel >= 0);
	if ( pData->tPointerOwner.bActive ) bChanged = 1;
	pData->iActivePart = XUI_COLOR_PICKER_PART_NONE;
	pData->iActiveChannel = -1;
	iRet = xuiInternalPointerOwnerCancel(pContext, pPanel, &pData->tPointerOwner);
	if ( iRet != XUI_OK ) return iRet;
	if ( !xuiInternalContextIsValid(pContext) || xuiInternalContextDestroyPending(pContext) ||
	     !xuiInternalWidgetIsValid(pPanel) ) return XUI_OK;
	return bChanged ? xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) : XUI_OK;
}

static int __xuiColorPickerPanelPointerDown(xui_widget pPanel, xui_widget pOwner, xui_color_picker_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	xui_rect_t tWorld;
	float fX;
	float fY;
	int i;
	int iCount;
	int iHit;
	int iFieldChannel;
	int iSliderChannel;
	int bSameEditTarget;
	int iRet;

	if ( (pEvent->iButton != 0) && (pEvent->iButton != XUI_POINTER_BUTTON_LEFT) ) {
		return XUI_OK;
	}
	if ( pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE ) {
		return XUI_OK;
	}
	if ( pData->tPointerOwner.bActive ) {
		return xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ?
			XUI_EVENT_DISPATCH_STOP : XUI_OK;
	}
	tWorld = xuiWidgetGetWorldRect(pPanel);
	fX = pEvent->fX - tWorld.fX;
	fY = pEvent->fY - tWorld.fY;
	__xuiColorPickerUpdatePanelRects(pData);
	iHit = __xuiColorPickerHitPanel(pData, fX, fY);
	iCount = pData->bAlphaEnabled ? 4 : 3;
	iFieldChannel = -1;
	iSliderChannel = -1;
	for ( i = 0; i < iCount; i++ ) {
		if ( __xuiColorPickerRectContains(pData->arrFieldRect[i], fX, fY) ) {
			iFieldChannel = i;
			break;
		}
		if ( __xuiColorPickerRectContains(__xuiColorPickerTrackHitRect(pData->arrSliderRect[i]), fX, fY) ) {
			iSliderChannel = i;
			break;
		}
	}
	pContext = xuiWidgetGetContext(pOwner);
	iRet = xuiSetFocusWidget(pContext, pPanel);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pOwner, pData) ) return XUI_EVENT_DISPATCH_STOP;
	pData->iKeyboardPart = XUI_COLOR_PICKER_PART_NONE;
	bSameEditTarget = (pData->bEditingHex && iHit == XUI_COLOR_PICKER_PART_HEX) ||
	                  (pData->iEditingChannel >= 0 && iFieldChannel == pData->iEditingChannel);
	if ( __xuiColorPickerEditing(pData) && !bSameEditTarget ) {
		iRet = __xuiColorPickerCommitEdit(pOwner, pData);
		if ( iRet != XUI_OK ) {
			iRet = __xuiColorPickerCancelEdit(pData);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( !__xuiColorPickerAlive(pContext, pOwner, pData) ) return XUI_EVENT_DISPATCH_STOP;
	}
	pData->iActivePart = iHit;
	pData->iActiveChannel = -1;
	if ( iFieldChannel >= 0 ) {
		pData->iActivePart = __xuiColorPickerChannelPart(iFieldChannel);
		if ( bSameEditTarget ) {
			pData->bEditSelectAll = 1;
			iRet = xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		iRet = __xuiColorPickerBeginChannelEdit(pData, iFieldChannel);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit == XUI_COLOR_PICKER_PART_SV ) {
		iRet = xuiInternalPointerOwnerBegin(xuiWidgetGetContext(pPanel), pPanel,
			pEvent, &pData->tPointerOwner);
		if ( iRet != XUI_OK ) {
			pData->iActivePart = XUI_COLOR_PICKER_PART_NONE;
			return iRet;
		}
		iRet = __xuiColorPickerPickSv(pOwner, pData, fX, fY);
		if ( iRet != XUI_OK ) (void)__xuiColorPickerCancelPointer(pPanel, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit == XUI_COLOR_PICKER_PART_HUE ) {
		iRet = xuiInternalPointerOwnerBegin(xuiWidgetGetContext(pPanel), pPanel,
			pEvent, &pData->tPointerOwner);
		if ( iRet != XUI_OK ) {
			pData->iActivePart = XUI_COLOR_PICKER_PART_NONE;
			return iRet;
		}
		iRet = __xuiColorPickerPickHue(pOwner, pData, fY);
		if ( iRet != XUI_OK ) (void)__xuiColorPickerCancelPointer(pPanel, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iSliderChannel >= 0 ) {
		pData->iActiveChannel = iSliderChannel;
		pData->iActivePart = __xuiColorPickerChannelPart(iSliderChannel);
		iRet = xuiInternalPointerOwnerBegin(xuiWidgetGetContext(pPanel), pPanel,
			pEvent, &pData->tPointerOwner);
		if ( iRet != XUI_OK ) {
			pData->iActivePart = XUI_COLOR_PICKER_PART_NONE;
			pData->iActiveChannel = -1;
			return iRet;
		}
		iRet = __xuiColorPickerPickChannel(pOwner, pData, pData->iActiveChannel, fX);
		if ( iRet != XUI_OK ) (void)__xuiColorPickerCancelPointer(pPanel, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit == XUI_COLOR_PICKER_PART_OLD ) {
		iRet = __xuiColorPickerSetColorInternal(pOwner, pData, pData->iOldColor, 1);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit == XUI_COLOR_PICKER_PART_HEX ) {
		if ( bSameEditTarget ) {
			pData->bEditSelectAll = 1;
			iRet = xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		iRet = __xuiColorPickerBeginHexEdit(pOwner, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit == XUI_COLOR_PICKER_PART_PALETTE ) {
		for ( i = 0; i < pData->iPaletteCount; i++ ) {
			if ( __xuiColorPickerRectContains(pData->arrPaletteRect[i], fX, fY) ) {
				pData->iSelectedPalette = i;
				iRet = __xuiColorPickerSetColorInternal(pOwner, pData, pData->arrPalette[i], 1);
				return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
			}
		}
	}
	iRet = xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
}

static int __xuiColorPickerPanelPointerMove(xui_widget pPanel, xui_widget pOwner, xui_color_picker_data_t* pData, const xui_event_t* pEvent)
{
	xui_rect_t tWorld;
	float fX;
	float fY;
	int iHit;

	if ( pData->tPointerOwner.bActive &&
	     !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
		return XUI_OK;
	}
	tWorld = xuiWidgetGetWorldRect(pPanel);
	fX = pEvent->fX - tWorld.fX;
	fY = pEvent->fY - tWorld.fY;
	__xuiColorPickerUpdatePanelRects(pData);
	if ( pData->iActivePart == XUI_COLOR_PICKER_PART_SV ) {
		iHit = __xuiColorPickerPickSv(pOwner, pData, fX, fY);
		if ( iHit != XUI_OK ) (void)__xuiColorPickerCancelPointer(pPanel, pData);
		return (iHit == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iHit;
	}
	if ( pData->iActivePart == XUI_COLOR_PICKER_PART_HUE ) {
		iHit = __xuiColorPickerPickHue(pOwner, pData, fY);
		if ( iHit != XUI_OK ) (void)__xuiColorPickerCancelPointer(pPanel, pData);
		return (iHit == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iHit;
	}
	if ( pData->iActiveChannel >= 0 ) {
		iHit = __xuiColorPickerPickChannel(pOwner, pData, pData->iActiveChannel, fX);
		if ( iHit != XUI_OK ) (void)__xuiColorPickerCancelPointer(pPanel, pData);
		return (iHit == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iHit;
	}
	iHit = __xuiColorPickerHitPanel(pData, fX, fY);
	if ( pData->iHoverPart != iHit ) {
		pData->iHoverPart = iHit;
		return xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_OK;
}

static int __xuiColorPickerPanelKeyboardActivate(xui_widget pOwner, xui_color_picker_data_t* pData)
{
	int iChannel;

	if ( pOwner == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iKeyboardPart == XUI_COLOR_PICKER_PART_OLD ) {
		return __xuiColorPickerSetColorInternal(pOwner, pData, pData->iOldColor, 1);
	}
	iChannel = __xuiColorPickerPartChannel(pData->iKeyboardPart);
	if ( iChannel >= 0 ) return __xuiColorPickerBeginChannelEdit(pData, iChannel);
	if ( pData->iKeyboardPart == XUI_COLOR_PICKER_PART_HEX ) {
		return __xuiColorPickerBeginHexEdit(pOwner, pData);
	}
	if ( pData->iKeyboardPart == XUI_COLOR_PICKER_PART_PALETTE &&
	     pData->iKeyboardPalette >= 0 && pData->iKeyboardPalette < pData->iPaletteCount ) {
		return __xuiColorPickerSetColorInternal(pOwner, pData,
			pData->arrPalette[pData->iKeyboardPalette], 1);
	}
	return XUI_OK;
}

static int __xuiColorPickerPanelKeyboardAdjust(xui_widget pPanel, xui_widget pOwner,
	xui_color_picker_data_t* pData, const xui_event_t* pEvent)
{
	float fStep;
	float fHueStep;
	float fSaturation;
	float fValue;
	int iChannel;
	int iValue;
	int iStep;
	int iTarget;

	if ( pPanel == NULL || pOwner == NULL || pData == NULL || pEvent == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	fStep = (pEvent->iModifiers & XUI_MOD_SHIFT) != 0u ? 0.05f : 0.01f;
	fHueStep = (pEvent->iModifiers & XUI_MOD_SHIFT) != 0u ? (10.0f / 360.0f) : (1.0f / 360.0f);
	iStep = (pEvent->iModifiers & XUI_MOD_SHIFT) != 0u ? 10 : 1;
	if ( pEvent->iKey == XUI_KEY_PAGE_UP || pEvent->iKey == XUI_KEY_PAGE_DOWN ) iStep = 10;

	if ( pData->iKeyboardPart == XUI_COLOR_PICKER_PART_SV ) {
		fSaturation = pData->fSaturation;
		fValue = pData->fValue;
		if ( pEvent->iKey == XUI_KEY_LEFT ) fSaturation -= fStep;
		else if ( pEvent->iKey == XUI_KEY_RIGHT ) fSaturation += fStep;
		else if ( pEvent->iKey == XUI_KEY_UP ) fValue += fStep;
		else if ( pEvent->iKey == XUI_KEY_DOWN ) fValue -= fStep;
		else return XUI_OK;
		return __xuiColorPickerSetHsvInternal(pOwner, pData, pData->fHue, fSaturation, fValue, 1);
	}
	if ( pData->iKeyboardPart == XUI_COLOR_PICKER_PART_HUE ) {
		if ( pEvent->iKey == XUI_KEY_LEFT || pEvent->iKey == XUI_KEY_UP ) fHueStep = -fHueStep;
		else if ( pEvent->iKey != XUI_KEY_RIGHT && pEvent->iKey != XUI_KEY_DOWN ) return XUI_OK;
		return __xuiColorPickerSetHsvInternal(pOwner, pData, pData->fHue + fHueStep,
			pData->fSaturation, pData->fValue, 1);
	}
	iChannel = __xuiColorPickerPartChannel(pData->iKeyboardPart);
	if ( iChannel >= 0 ) {
		iValue = iChannel == 0 ? __xuiColorPickerR(pData->iColor) :
			(iChannel == 1 ? __xuiColorPickerG(pData->iColor) :
			 (iChannel == 2 ? __xuiColorPickerB(pData->iColor) : __xuiColorPickerA(pData->iColor)));
		if ( pEvent->iKey == XUI_KEY_HOME ) iValue = 0;
		else if ( pEvent->iKey == XUI_KEY_END ) iValue = 255;
		else if ( pEvent->iKey == XUI_KEY_LEFT || pEvent->iKey == XUI_KEY_DOWN || pEvent->iKey == XUI_KEY_PAGE_DOWN ) iValue -= iStep;
		else if ( pEvent->iKey == XUI_KEY_RIGHT || pEvent->iKey == XUI_KEY_UP || pEvent->iKey == XUI_KEY_PAGE_UP ) iValue += iStep;
		else return XUI_OK;
		return __xuiColorPickerSetChannelInternal(pOwner, pData, iChannel, iValue, 1);
	}
	if ( pData->iKeyboardPart == XUI_COLOR_PICKER_PART_PALETTE && pData->iPaletteCount > 0 ) {
		iTarget = pData->iKeyboardPalette;
		if ( iTarget < 0 || iTarget >= pData->iPaletteCount ) iTarget = 0;
		if ( pEvent->iKey == XUI_KEY_HOME ) iTarget = 0;
		else if ( pEvent->iKey == XUI_KEY_END ) iTarget = pData->iPaletteCount - 1;
		else if ( pEvent->iKey == XUI_KEY_LEFT || pEvent->iKey == XUI_KEY_UP ) iTarget--;
		else if ( pEvent->iKey == XUI_KEY_RIGHT || pEvent->iKey == XUI_KEY_DOWN ) iTarget++;
		else return XUI_OK;
		if ( iTarget < 0 ) iTarget = 0;
		if ( iTarget >= pData->iPaletteCount ) iTarget = pData->iPaletteCount - 1;
		if ( iTarget == pData->iKeyboardPalette ) return XUI_OK;
		pData->iKeyboardPalette = iTarget;
		return xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_OK;
}

static int __xuiColorPickerPanelKeyDown(xui_widget pPanel, xui_widget pOwner, xui_color_picker_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	int iRet;

	pContext = xuiWidgetGetContext(pOwner);
	if ( pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE ) {
		return XUI_OK;
	}
	if ( __xuiColorPickerEditing(pData) ) {
		if ( (pEvent->iKey == 'A' || pEvent->iKey == 'a') && ((pEvent->iModifiers & XUI_MOD_CTRL) != 0u) ) {
			pData->bEditSelectAll = 1;
			iRet = xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( pEvent->iKey == XUI_KEY_ENTER ) {
			iRet = __xuiColorPickerCommitEdit(pOwner, pData);
			if ( iRet == XUI_ERROR_INVALID_ARGUMENT ) return XUI_EVENT_DISPATCH_STOP;
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( pEvent->iKey == XUI_KEY_TAB ) {
			iRet = __xuiColorPickerCommitEdit(pOwner, pData);
			if ( iRet == XUI_ERROR_INVALID_ARGUMENT ) return XUI_EVENT_DISPATCH_STOP;
			if ( iRet != XUI_OK ) return iRet;
			if ( !__xuiColorPickerAlive(pContext, pOwner, pData) ) return XUI_EVENT_DISPATCH_STOP;
			iRet = __xuiColorPickerMoveKeyboardPart(pPanel, pData,
				(pEvent->iModifiers & XUI_MOD_SHIFT) != 0u ? -1 : 1);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( pEvent->iKey == XUI_KEY_ESCAPE ) {
			iRet = __xuiColorPickerCancelEdit(pData);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( pEvent->iKey == XUI_KEY_BACKSPACE ) {
			if ( pData->bEditSelectAll ) {
				pData->iEditLen = 0;
				pData->sEdit[0] = '\0';
				pData->bEditSelectAll = 0;
			} else if ( pData->iEditLen > 0 ) {
				pData->sEdit[--pData->iEditLen] = '\0';
			}
			pData->bEditError = 0;
			iRet = xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( pEvent->iKey == XUI_KEY_DELETE ) {
			pData->iEditLen = 0;
			pData->sEdit[0] = '\0';
			pData->bEditError = 0;
			iRet = xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iKey == XUI_KEY_TAB ) {
		iRet = __xuiColorPickerMoveKeyboardPart(pPanel, pData,
			(pEvent->iModifiers & XUI_MOD_SHIFT) != 0u ? -1 : 1);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( pEvent->iKey == XUI_KEY_ESCAPE ) {
		iRet = xuiColorPickerClose(pOwner);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( pData->iKeyboardPart == XUI_COLOR_PICKER_PART_NONE &&
	     (pEvent->iKey == XUI_KEY_LEFT || pEvent->iKey == XUI_KEY_RIGHT ||
	      pEvent->iKey == XUI_KEY_UP || pEvent->iKey == XUI_KEY_DOWN ||
	      pEvent->iKey == XUI_KEY_HOME || pEvent->iKey == XUI_KEY_END ||
	      pEvent->iKey == XUI_KEY_PAGE_UP || pEvent->iKey == XUI_KEY_PAGE_DOWN ||
	      pEvent->iKey == XUI_KEY_ENTER || pEvent->iKey == XUI_KEY_SPACE) ) {
		iRet = __xuiColorPickerSetKeyboardPart(pPanel, pData, XUI_COLOR_PICKER_PART_SV);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pEvent->iKey == XUI_KEY_ENTER || pEvent->iKey == XUI_KEY_SPACE ) {
		iRet = __xuiColorPickerPanelKeyboardActivate(pOwner, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	iRet = __xuiColorPickerPanelKeyboardAdjust(pPanel, pOwner, pData, pEvent);
	if ( iRet != XUI_OK ) return iRet;
	if ( pEvent->iKey == XUI_KEY_LEFT || pEvent->iKey == XUI_KEY_RIGHT ||
	     pEvent->iKey == XUI_KEY_UP || pEvent->iKey == XUI_KEY_DOWN ||
	     pEvent->iKey == XUI_KEY_HOME || pEvent->iKey == XUI_KEY_END ||
	     pEvent->iKey == XUI_KEY_PAGE_UP || pEvent->iKey == XUI_KEY_PAGE_DOWN ) {
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static int __xuiColorPickerPanelText(xui_widget pPanel, xui_color_picker_data_t* pData, const xui_event_t* pEvent)
{
	uint32_t iCodepoint;
	int iLimit;
	int iRet;

	if ( (pData == NULL) || !__xuiColorPickerEditing(pData) || (pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE) ) {
		return XUI_OK;
	}
	iCodepoint = pEvent->iCodepoint;
	if ( iCodepoint == 0 && pEvent->iTextSize > 0 ) {
		iCodepoint = (uint32_t)(unsigned char)pEvent->sText[0];
	}
	iLimit = pData->bEditingHex ? (pData->bAlphaEnabled ? 8 : 6) : 3;
	if ( pData->bEditingHex && __xuiColorPickerHexChar(iCodepoint) &&
	     (pData->bEditSelectAll || pData->iEditLen < iLimit) ) {
		if ( pData->bEditSelectAll ) {
			pData->iEditLen = 0;
			pData->sEdit[0] = '\0';
			pData->bEditSelectAll = 0;
		}
		if ( iCodepoint >= 'a' && iCodepoint <= 'f' ) {
			iCodepoint = iCodepoint - 'a' + 'A';
		}
		pData->sEdit[pData->iEditLen++] = (char)iCodepoint;
		pData->sEdit[pData->iEditLen] = '\0';
		pData->bEditError = 0;
		iRet = xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	} else if ( !pData->bEditingHex && iCodepoint >= '0' && iCodepoint <= '9' &&
	            (pData->bEditSelectAll || pData->iEditLen < iLimit) ) {
		if ( pData->bEditSelectAll ) {
			pData->iEditLen = 0;
			pData->sEdit[0] = '\0';
			pData->bEditSelectAll = 0;
		}
		pData->sEdit[pData->iEditLen++] = (char)iCodepoint;
		pData->sEdit[pData->iEditLen] = '\0';
		pData->bEditError = 0;
		iRet = xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiColorPickerPanelWheel(xui_widget pPanel, xui_widget pOwner, xui_color_picker_data_t* pData, const xui_event_t* pEvent)
{
	xui_rect_t tWorld;
	float fX;
	float fY;
	int i;
	int iCount;
	int iDirection;
	int iValue;
	int iRet;

	if ( pEvent->fWheelY == 0.0f ) return XUI_OK;
	iDirection = (pEvent->fWheelY > 0.0f) ? 1 : -1;
	tWorld = xuiWidgetGetWorldRect(pPanel);
	fX = pEvent->fX - tWorld.fX;
	fY = pEvent->fY - tWorld.fY;
	__xuiColorPickerUpdatePanelRects(pData);
	if ( __xuiColorPickerRectContains(pData->tHueRect, fX, fY) ) {
		iRet = __xuiColorPickerSetHsvInternal(pOwner, pData,
			pData->fHue - (float)iDirection * 0.01f,
			pData->fSaturation, pData->fValue, 1);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	iCount = pData->bAlphaEnabled ? 4 : 3;
	for ( i = 0; i < iCount; i++ ) {
		if ( __xuiColorPickerRectContains(pData->arrFieldRect[i], fX, fY) ||
		     __xuiColorPickerRectContains(__xuiColorPickerTrackHitRect(pData->arrSliderRect[i]), fX, fY) ) {
			iValue = (i == 0) ? __xuiColorPickerR(pData->iColor) :
				((i == 1) ? __xuiColorPickerG(pData->iColor) :
				((i == 2) ? __xuiColorPickerB(pData->iColor) : __xuiColorPickerA(pData->iColor)));
			iValue = __xuiColorPickerClampByte(iValue + iDirection);
			iRet = __xuiColorPickerSetChannelInternal(pOwner, pData, i, iValue, 1);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
	}
	return XUI_OK;
}

static int __xuiColorPickerPanelAcceptsInput(xui_widget pPanel, xui_widget pOwner, xui_color_picker_data_t* pData)
{
	return (pPanel != NULL) && (pOwner != NULL) && (pData != NULL) &&
	       (pData->pPanel == pPanel) && (pData->pPopup != NULL) &&
	       xuiPopupIsOpen(pData->pPopup) &&
	       xuiWidgetGetEffectiveEnabled(pOwner) && xuiWidgetGetEffectiveVisible(pOwner) &&
	       xuiWidgetGetEffectiveEnabled(pPanel) && xuiWidgetGetEffectiveVisible(pPanel);
}

static int __xuiColorPickerPanelEvent(xui_widget pPanel, const xui_event_t* pEvent, void* pUser)
{
	xui_widget pOwner;
	xui_color_picker_data_t* pData;

	pOwner = (xui_widget)pUser;
	if ( (pPanel == NULL) || (pOwner == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiColorPickerGetData(pOwner);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_DOWN:
	case XUI_EVENT_POINTER_MOVE:
	case XUI_EVENT_POINTER_UP:
	case XUI_EVENT_POINTER_WHEEL:
	case XUI_EVENT_KEY_DOWN:
	case XUI_EVENT_TEXT:
		if ( !__xuiColorPickerPanelAcceptsInput(pPanel, pOwner, pData) ) return XUI_OK;
		break;
	default:
		break;
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_DOWN:
		return __xuiColorPickerPanelPointerDown(pPanel, pOwner, pData, pEvent);
	case XUI_EVENT_POINTER_MOVE:
		return __xuiColorPickerPanelPointerMove(pPanel, pOwner, pData, pEvent);
	case XUI_EVENT_POINTER_LEAVE:
		if ( pData->iHoverPart != XUI_COLOR_PICKER_PART_NONE ) {
			pData->iHoverPart = XUI_COLOR_PICKER_PART_NONE;
			return xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		}
		return XUI_OK;
	case XUI_EVENT_POINTER_UP:
		if ( pData->tPointerOwner.bActive &&
		     !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		{
			int iRet = __xuiColorPickerCancelPointer(pPanel, pData);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		if ( !xuiInternalPointerOwnerCaptureLost(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		{
			int iRet = __xuiColorPickerCancelPointer(pPanel, pData);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
	case XUI_EVENT_POINTER_WHEEL:
		return __xuiColorPickerPanelWheel(pPanel, pOwner, pData, pEvent);
	case XUI_EVENT_KEY_DOWN:
		return __xuiColorPickerPanelKeyDown(pPanel, pOwner, pData, pEvent);
	case XUI_EVENT_TEXT:
		return __xuiColorPickerPanelText(pPanel, pData, pEvent);
	case XUI_EVENT_FOCUS:
	case XUI_EVENT_BLUR:
		return __xuiColorPickerSyncState(pOwner, pData);
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiColorPickerOpenInternal(xui_widget pWidget, xui_color_picker_data_t* pData)
{
	xui_context pContext;
	xui_widget pPopup;
	xui_widget pPanel;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->pPopup == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) {
		if ( xuiPopupIsOpen(pData->pPopup) ) return xuiColorPickerClose(pWidget);
		return XUI_OK;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pPopup = pData->pPopup;
	pPanel = pData->pPanel;
	iRet = __xuiColorPickerApplyPopupStyle(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	pData->iOldColor = pData->iColor;
	pData->iActivePart = XUI_COLOR_PICKER_PART_NONE;
	pData->iActiveChannel = -1;
	pData->iHoverPart = XUI_COLOR_PICKER_PART_NONE;
	pData->iKeyboardPart = XUI_COLOR_PICKER_PART_NONE;
	iRet = __xuiColorPickerCancelEdit(pData);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiPopupSetOpen(pPopup, 1);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( pPanel != NULL ) {
		iRet = xuiSetFocusWidget(pContext, pPanel);
		if ( iRet != XUI_OK ) {
			if ( __xuiColorPickerAlive(pContext, pWidget, pData) ) (void)xuiPopupSetOpen(pPopup, 0);
			return iRet;
		}
		if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	}
	return __xuiColorPickerSyncState(pWidget, pData);
}

static int __xuiColorPickerPointerDown(xui_widget pWidget, xui_color_picker_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	int iRet;

	if ( (pEvent->iButton != XUI_POINTER_BUTTON_LEFT) || (pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE) ) {
		return XUI_OK;
	}
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) {
		return XUI_OK;
	}
	pContext = xuiWidgetGetContext(pWidget);
	iRet = xuiSetFocusWidget(pContext, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
	if ( (pData->pPopup != NULL) && xuiPopupIsOpen(pData->pPopup) ) {
		iRet = xuiColorPickerClose(pWidget);
	} else {
		iRet = __xuiColorPickerOpenInternal(pWidget, pData);
	}
	return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
}

static int __xuiColorPickerKeyDown(xui_widget pWidget, xui_color_picker_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	int iRet;

	if ( pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE || !xuiWidgetGetEffectiveEnabled(pWidget) ) {
		return XUI_OK;
	}
	pContext = xuiWidgetGetContext(pWidget);
	switch ( pEvent->iKey ) {
	case XUI_KEY_ENTER:
	case XUI_KEY_SPACE:
		iRet = xuiColorPickerToggle(pWidget);
		if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
		if ( iRet == XUI_OK && pData->pPanel != NULL && xuiColorPickerIsOpen(pWidget) ) {
			iRet = __xuiColorPickerSetKeyboardPart(pData->pPanel, pData, XUI_COLOR_PICKER_PART_SV);
		}
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	case XUI_KEY_DOWN:
		iRet = __xuiColorPickerOpenInternal(pWidget, pData);
		if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
		if ( iRet == XUI_OK && pData->pPanel != NULL ) {
			iRet = __xuiColorPickerSetKeyboardPart(pData->pPanel, pData, XUI_COLOR_PICKER_PART_SV);
		}
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	case XUI_KEY_ESCAPE:
		if ( (pData->pPopup != NULL) && xuiPopupIsOpen(pData->pPopup) ) {
			iRet = xuiColorPickerClose(pWidget);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		break;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiColorPickerEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_color_picker_data_t* pData;

	(void)pWidget;
	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_DOWN:
		return __xuiColorPickerPointerDown(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_CLICK:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_KEY_DOWN:
		return __xuiColorPickerKeyDown(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_LEAVE:
	case XUI_EVENT_POINTER_MOVE:
	case XUI_EVENT_FOCUS:
	case XUI_EVENT_BLUR:
		return __xuiColorPickerSyncState(pWidget, pData);
	case XUI_EVENT_BOUNDS_CHANGED:
		if ( (pData->pPopup != NULL) && xuiPopupIsOpen(pData->pPopup) ) {
			xui_context pContext = xuiWidgetGetContext(pWidget);
			int iRet = __xuiColorPickerApplyPopupStyle(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
			return xuiPopupApplyPlacement(pData->pPopup);
		}
		return XUI_OK;
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) {
			xui_context pContext = xuiWidgetGetContext(pWidget);
			int iRet = xuiColorPickerClose(pWidget);
			if ( iRet != XUI_OK ) return iRet;
			if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
		}
		return __xuiColorPickerSyncState(pWidget, pData);
	default:
		break;
	}
	return XUI_OK;
}

static void __xuiColorPickerDefaultAction(xui_widget pWidget, void* pUser)
{
	(void)pUser;
	(void)xuiColorPickerToggle(pWidget);
}

static void __xuiColorPickerCancelAction(xui_widget pWidget, void* pUser)
{
	(void)pUser;
	(void)xuiColorPickerClose(pWidget);
}

static void __xuiColorPickerPopupChanged(xui_widget pPopup, int bOpen, void* pUser)
{
	xui_widget pOwner;
	xui_color_picker_data_t* pData;
	xui_context pContext;

	(void)pPopup;
	pOwner = (xui_widget)pUser;
	pData = __xuiColorPickerGetData(pOwner);
	if ( pData == NULL ) {
		return;
	}
	pContext = xuiWidgetGetContext(pOwner);
	if ( !bOpen ) {
		if ( pData->pPanel != NULL ) (void)__xuiColorPickerCancelPointer(pData->pPanel, pData);
		if ( !__xuiColorPickerAlive(pContext, pOwner, pData) ) return;
		pData->iActivePart = XUI_COLOR_PICKER_PART_NONE;
		pData->iActiveChannel = -1;
		pData->iHoverPart = XUI_COLOR_PICKER_PART_NONE;
		pData->iKeyboardPart = XUI_COLOR_PICKER_PART_NONE;
		(void)__xuiColorPickerCancelEdit(pData);
	}
	(void)__xuiColorPickerSyncState(pOwner, pData);
}

static int __xuiColorPickerContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_color_picker_data_t* pData;
	xui_color_picker_data_t tResolved;
	xui_font_metrics_t tMetrics;
	xui_proxy pProxy;

	(void)pUser;
	if ( (pWidget == NULL) || (pSize == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiColorPickerResolve(pWidget, pData, &tResolved);
	pSize->fX = 176.0f;
	pSize->fY = 30.0f;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->fontGetMetrics != NULL) && (tResolved.pFont != NULL) &&
	     (pProxy->fontGetMetrics(pProxy, tResolved.pFont, &tMetrics) == XUI_OK) &&
	     (tMetrics.fLineHeight > 0.0f) ) {
		pSize->fY = __xuiColorPickerMax(30.0f, tMetrics.fLineHeight + 12.0f);
	}
	pSize->fX = xuiInternalSnapSize(pSize->fX);
	pSize->fY = xuiInternalSnapSize(pSize->fY);
	if ( (tConstraint.fX >= 0.0f) && (tConstraint.fX < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fX > tConstraint.fX) ) pSize->fX = tConstraint.fX;
	if ( (tConstraint.fY >= 0.0f) && (tConstraint.fY < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fY > tConstraint.fY) ) pSize->fY = tConstraint.fY;
	return XUI_OK;
}

static void __xuiColorPickerDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_VISIBLE;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiColorPickerDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiColorPickerInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetDefaultAction(pWidget, __xuiColorPickerDefaultAction, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCancelAction(pWidget, __xuiColorPickerCancelAction, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_ENTER, __xuiColorPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiColorPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiColorPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiColorPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CLICK, __xuiColorPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiColorPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiColorPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiColorPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BOUNDS_CHANGED, __xuiColorPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiColorPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiColorPickerEvent, NULL);
	return iRet;
}

static int __xuiColorPickerInitPanelEvents(xui_widget pPanel, xui_widget pOwner)
{
	int iRet;

	iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_POINTER_DOWN, __xuiColorPickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_POINTER_MOVE, __xuiColorPickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_POINTER_LEAVE, __xuiColorPickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_POINTER_UP, __xuiColorPickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiColorPickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_POINTER_WHEEL, __xuiColorPickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_KEY_DOWN, __xuiColorPickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_TEXT, __xuiColorPickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_FOCUS, __xuiColorPickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_BLUR, __xuiColorPickerPanelEvent, pOwner);
	return iRet;
}

static int __xuiColorPickerCreatePopup(xui_widget pWidget, xui_color_picker_data_t* pData)
{
	xui_popup_desc_t tDesc;
	xui_cache_policy_t tPolicy;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pWidget;
	tDesc.fContentWidth = pData->fPopupWidth;
	tDesc.fContentHeight = pData->fPopupHeight;
	tDesc.fPadding = 3.0f;
	tDesc.fBorderWidth = 1.0f;
	tDesc.fShadowSize = 5.0f;
	tDesc.fGap = 2.0f;
	tDesc.iAnchor = XUI_POPUP_ANCHOR_BOTTOM_LEFT;
	tDesc.iDirection = XUI_POPUP_DIRECTION_RIGHT_DOWN;
	tDesc.iOutsidePolicy = XUI_POPUP_OUTSIDE_CLOSE;
	tDesc.iOwnerPolicy = XUI_POPUP_OWNER_PASSTHROUGH;
	tDesc.iEscapePolicy = XUI_POPUP_ESCAPE_CLOSE;
	tDesc.iFocusPolicy = XUI_POPUP_FOCUS_CUSTOM;
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	tDesc.iPanelColor = pData->iPopupPanelColor;
	tDesc.iBorderColor = pData->iPopupBorderColor;
	tDesc.iShadowColor = pData->iPopupShadowColor;
	tDesc.iBackdropColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	iRet = xuiPopupCreate(xuiWidgetGetContext(pWidget), &pData->pPopup, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	pData->pPanel = xuiPopupGetContentWidget(pData->pPopup);
	if ( pData->pPanel == NULL ) {
		iRet = XUI_ERROR_NOT_INITIALIZED;
		goto fail;
	}
	__xuiColorPickerDefaultCachePolicy(&tPolicy);
	iRet = xuiWidgetSetLayoutType(pData->pPanel, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pData->pPanel, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pData->pPanel, XUI_OVERFLOW_VISIBLE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pData->pPanel, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pData->pPanel, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetImeMode(pData->pPanel, XUI_IME_DISABLED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCachePolicy(pData->pPanel, &tPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCacheRenderCallback(pData->pPanel, __xuiColorPickerPanelRender, pWidget);
	if ( iRet == XUI_OK ) iRet = __xuiColorPickerInitPanelEvents(pData->pPanel, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetChange(pData->pPopup, __xuiColorPickerPopupChanged, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetFocusPolicy(pData->pPopup, XUI_POPUP_FOCUS_CUSTOM, pData->pPanel);
	if ( iRet == XUI_OK ) iRet = __xuiColorPickerApplyPopupStyle(pWidget, pData);
	if ( iRet == XUI_OK ) return XUI_OK;

fail:
	xuiWidgetDestroy(pData->pPopup);
	pData->pPopup = NULL;
	pData->pPanel = NULL;
	return iRet;
}

static int __xuiColorPickerApplyPopupStyle(xui_widget pWidget, xui_color_picker_data_t* pData)
{
	xui_color_picker_data_t tResolved;
	xui_context pContext;
	xui_widget pPopup;
	xui_widget pPanel;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->pPopup == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pPopup = pData->pPopup;
	pPanel = pData->pPanel;
	__xuiColorPickerResolve(pWidget, pData, &tResolved);
	iRet = xuiPopupSetOwner(pPopup, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetFocusRestore(pPopup, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetContentSize(pPopup, pData->fPopupWidth, pData->fPopupHeight);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetColors(pPopup, tResolved.iPopupPanelColor, tResolved.iPopupBorderColor,
		tResolved.iPopupShadowColor, XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetMetrics(pPopup, 3.0f, 1.0f, 5.0f);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetClosePolicy(pPopup, XUI_POPUP_OUTSIDE_CLOSE,
		XUI_POPUP_OWNER_PASSTHROUGH, XUI_POPUP_ESCAPE_CLOSE);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetFocusPolicy(pPopup, XUI_POPUP_FOCUS_CUSTOM, pPanel);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetGap(pPopup, 2.0f);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupClearAnchorRect(pPopup);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( pData->iPopupPlacement == XUI_COLOR_PICKER_POPUP_TOP ) {
		iRet = xuiPopupSetAnchor(pPopup, XUI_POPUP_ANCHOR_TOP_LEFT);
		if ( iRet == XUI_OK && __xuiColorPickerAlive(pContext, pWidget, pData) )
			iRet = xuiPopupSetDirection(pPopup, XUI_POPUP_DIRECTION_RIGHT_UP);
	} else {
		iRet = xuiPopupSetAnchor(pPopup, XUI_POPUP_ANCHOR_BOTTOM_LEFT);
		if ( iRet == XUI_OK && __xuiColorPickerAlive(pContext, pWidget, pData) )
			iRet = xuiPopupSetDirection(pPopup, XUI_POPUP_DIRECTION_RIGHT_DOWN);
	}
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( pPanel != NULL ) {
		iRet = xuiWidgetSetRect(pPanel, (xui_rect_t){0.0f, 0.0f, pData->fPopupWidth, pData->fPopupHeight});
		if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
		if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pPanel,
			XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiColorPickerInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_color_picker_data_t* pData;
	const xui_color_picker_desc_t* pDesc;
	int iRet;

	(void)pUser;
	pData = (xui_color_picker_data_t*)pTypeData;
	pDesc = (const xui_color_picker_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiColorPickerDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiColorPickerDefaults(pData);
	__xuiColorPickerApplyDesc(pData, pDesc);
	__xuiColorPickerSyncSelectedPalette(pData);
	iRet = xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pWidget, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_VISIBLE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPadding(pWidget, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	if ( iRet == XUI_OK ) iRet = __xuiColorPickerInitEvents(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiColorPickerCreatePopup(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiColorPickerSyncState(pWidget, pData);
	if ( iRet == XUI_OK ) return XUI_OK;
	xuiWidgetDestroy(pData->pPopup);
	pData->pPopup = NULL;
	pData->pPanel = NULL;
	return iRet;
}

static void __xuiColorPickerDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_color_picker_data_t* pData;

	(void)pUser;
	pData = (xui_color_picker_data_t*)pTypeData;
	if ( pData != NULL ) {
		if ( pData->pPanel != NULL ) {
			(void)__xuiColorPickerCancelPointer(pData->pPanel, pData);
		}
		if ( pData->pPopup != NULL ) {
			xuiWidgetDestroy(pData->pPopup);
		}
		memset(pData, 0, sizeof(*pData));
	}
}

static void __xuiColorPickerRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
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

static void __xuiColorPickerRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;

	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.text.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.text.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.background.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.background.open_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.background.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.border.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.border.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.border.focus_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.arrow.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.arrow.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.button.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.button.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.button.open_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.popup.panel_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.popup.border_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.popup.shadow_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.popup.text_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.popup.muted_text_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.accent.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.field.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.field.border_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.separator.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.field.error_border_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.selection.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.selection.text_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.track.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.knob.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.marker.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.marker.border_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.checker.light_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.checker.dark_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "colorpicker.border.width", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiColorPickerRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, iLayoutDirty, XUI_STYLE_PROPERTY_INHERITED);
}

static xui_color_picker_data_t* __xuiColorPickerGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) {
		return NULL;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "colorpicker");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) {
		return NULL;
	}
	return (xui_color_picker_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiColorPickerAccessibleNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);

	if ( (pData == NULL) || (pNode == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_COMBOBOX;
	pNode->iState |= XUI_ACCESSIBLE_STATE_READONLY;
	pNode->iState |= xuiColorPickerIsOpen(pWidget) ?
		XUI_ACCESSIBLE_STATE_EXPANDED : XUI_ACCESSIBLE_STATE_COLLAPSED;
	pNode->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_EXPAND) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_COLLAPSE) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_VALUE);
	pNode->sName = "Color picker";
	pNode->sValue = pData->sHex;
	return XUI_OK;
}

static int __xuiColorPickerAccessibleAction(xui_widget pWidget, int iAction, const void* pPayload)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	const xui_accessible_value_t* pValue;
	uint32_t iColor;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iAction == XUI_ACCESSIBLE_ACTION_EXPAND ) return xuiColorPickerOpen(pWidget);
	if ( iAction == XUI_ACCESSIBLE_ACTION_COLLAPSE ) return xuiColorPickerClose(pWidget);
	if ( iAction != XUI_ACCESSIBLE_ACTION_SET_VALUE ) return XUI_ERROR_UNSUPPORTED;
	pValue = (const xui_accessible_value_t*)pPayload;
	if ( (pValue == NULL) || (pValue->iSize < sizeof(*pValue)) ||
	     (pValue->iType != XUI_ACCESSIBLE_VALUE_TEXT) || (pValue->sText == NULL) ||
	     (__xuiColorPickerParseHex(pValue->sText, &iColor) != XUI_OK) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiColorPickerSetColorInternal(pWidget, pData, iColor, 1);
}

static const xui_internal_accessibility_adapter_t g_xuiColorPickerAccessible = {
	.getNode = __xuiColorPickerAccessibleNode,
	.performAction = __xuiColorPickerAccessibleAction
};

XUI_API xui_widget_type xuiColorPickerGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) {
		return NULL;
	}
	pType = xuiWidgetFindType(pContext, "colorpicker");
	if ( pType != NULL ) {
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "colorpicker";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_color_picker_data_t);
	tDesc.onInit = __xuiColorPickerInit;
	tDesc.onDestroy = __xuiColorPickerDestroy;
	tDesc.onContentMeasure = __xuiColorPickerContentMeasure;
	tDesc.onCacheRender = __xuiColorPickerCacheRender;
	__xuiColorPickerDefaultLayout(&tDesc.tLayout);
	__xuiColorPickerDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) {
		return NULL;
	}
	__xuiColorPickerRegisterStyleProperties(pContext, pType);
	pType->onPreparePaint = __xuiColorPickerSyncStyle;
	pType->pAccessibleAdapter = &g_xuiColorPickerAccessible;
	return pType;
}

XUI_API int xuiColorPickerCreate(xui_context pContext, xui_widget* ppWidget, const xui_color_picker_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	if ( !__xuiColorPickerDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	pType = xuiColorPickerGetType(pContext);
	if ( pType == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiColorPickerSetChange(xui_widget pWidget, xui_color_picker_change_proc onChange, void* pUser)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onChange = onChange;
	pData->pChangeUser = pUser;
	return XUI_OK;
}

XUI_API int xuiColorPickerSetColor(xui_widget pWidget, uint32_t iColor)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiColorPickerSetColorInternal(pWidget, pData, iColor, 0);
}

XUI_API uint32_t xuiColorPickerGetColor(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	return (pData != NULL) ? pData->iColor : 0u;
}

XUI_API int xuiColorPickerSetRGBA(xui_widget pWidget, int iR, int iG, int iB, int iA)
{
	return xuiColorPickerSetColor(pWidget, XUI_COLOR_RGBA(
		__xuiColorPickerClampByte(iR),
		__xuiColorPickerClampByte(iG),
		__xuiColorPickerClampByte(iB),
		__xuiColorPickerClampByte(iA)));
}

XUI_API int xuiColorPickerGetRGBA(xui_widget pWidget, int* pR, int* pG, int* pB, int* pA)
{
	uint32_t iColor;

	if ( __xuiColorPickerGetData(pWidget) == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iColor = xuiColorPickerGetColor(pWidget);
	if ( pR != NULL ) *pR = __xuiColorPickerR(iColor);
	if ( pG != NULL ) *pG = __xuiColorPickerG(iColor);
	if ( pB != NULL ) *pB = __xuiColorPickerB(iColor);
	if ( pA != NULL ) *pA = __xuiColorPickerA(iColor);
	return XUI_OK;
}

XUI_API int xuiColorPickerSetAlphaEnabled(xui_widget pWidget, int bEnabled)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	xui_context pContext;
	uint32_t iOldColor;
	int iEnabled;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	iEnabled = bEnabled ? 1 : 0;
	if ( pData->bAlphaEnabled == iEnabled ) return XUI_OK;
	if ( pData->pPanel != NULL ) {
		iRet = __xuiColorPickerCancelPointer(pData->pPanel, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	}
	iRet = __xuiColorPickerCancelEdit(pData);
	if ( iRet != XUI_OK ) return iRet;
	iOldColor = pData->iColor;
	pData->bAlphaEnabled = iEnabled;
	if ( !iEnabled && pData->iKeyboardPart == XUI_COLOR_PICKER_PART_ALPHA ) {
		pData->iKeyboardPart = XUI_COLOR_PICKER_PART_HEX;
	}
	pData->iColor = __xuiColorPickerNormalizeColor(pData, pData->iColor);
	__xuiColorPickerFormatHex(pData);
	__xuiColorPickerSyncSelectedPalette(pData);
	iRet = __xuiColorPickerInvalidateAll(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	if ( pData->iColor != iOldColor )
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiColorPickerGetAlphaEnabled(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	return (pData != NULL) ? pData->bAlphaEnabled : 0;
}

XUI_API int xuiColorPickerSetHex(xui_widget pWidget, const char* sHex)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	uint32_t iColor;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiColorPickerParseHex(sHex, &iColor);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiColorPickerSetColorInternal(pWidget, pData, iColor, 0);
}

XUI_API const char* xuiColorPickerGetHex(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	return (pData != NULL) ? pData->sHex : "";
}

XUI_API int xuiColorPickerSetPalette(xui_widget pWidget, const uint32_t* arrColors, int iCount)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	uint32_t arrCopy[XUI_COLOR_PICKER_PALETTE_CAPACITY];

	if ( (pData == NULL) || (iCount < 0) || (iCount > XUI_COLOR_PICKER_PALETTE_CAPACITY) ||
	     ((iCount > 0) && (arrColors == NULL)) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCount > 0 ) memcpy(arrCopy, arrColors, (size_t)iCount * sizeof(arrCopy[0]));
	if ( pData->iPaletteCount == iCount &&
	     (iCount == 0 || memcmp(pData->arrPalette, arrCopy, (size_t)iCount * sizeof(arrCopy[0])) == 0) ) {
		return XUI_OK;
	}
	memset(pData->arrPalette, 0, sizeof(pData->arrPalette));
	pData->iPaletteCount = iCount;
	if ( iCount > 0 ) memcpy(pData->arrPalette, arrCopy, (size_t)iCount * sizeof(arrCopy[0]));
	if ( iCount <= 0 ) {
		pData->iKeyboardPalette = -1;
		if ( pData->iKeyboardPart == XUI_COLOR_PICKER_PART_PALETTE ) {
			pData->iKeyboardPart = XUI_COLOR_PICKER_PART_HEX;
		}
	} else if ( pData->iKeyboardPalette >= iCount ) {
		pData->iKeyboardPalette = iCount - 1;
	}
	__xuiColorPickerSyncSelectedPalette(pData);
	{
		int iRet = __xuiColorPickerInvalidateAll(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return XUI_OK;
}

XUI_API int xuiColorPickerGetPaletteCount(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	return (pData != NULL) ? pData->iPaletteCount : 0;
}

XUI_API uint32_t xuiColorPickerGetPaletteColor(xui_widget pWidget, int iIndex)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPaletteCount) ) return 0u;
	return pData->arrPalette[iIndex];
}

XUI_API int xuiColorPickerOpen(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiColorPickerOpenInternal(pWidget, pData);
}

XUI_API int xuiColorPickerClose(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	xui_context pContext;
	xui_widget pPopup;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	pPopup = pData->pPopup;
	if ( pData->pPanel != NULL ) {
		iRet = __xuiColorPickerCancelPointer(pData->pPanel, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	}
	if ( pPopup != NULL ) {
		iRet = xuiPopupSetOpen(pPopup, 0);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	}
	return __xuiColorPickerSyncState(pWidget, pData);
}

XUI_API int xuiColorPickerToggle(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return (pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup)) ? xuiColorPickerClose(pWidget) : xuiColorPickerOpen(pWidget);
}

XUI_API int xuiColorPickerIsOpen(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	return (pData != NULL && pData->pPopup != NULL) ? xuiPopupIsOpen(pData->pPopup) : 0;
}

XUI_API int xuiColorPickerSetPopupSize(xui_widget pWidget, float fWidth, float fHeight)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	xui_context pContext;
	float fOldWidth;
	float fOldHeight;
	float fNewWidth;
	float fNewHeight;
	int iRet;

	if ( (pData == NULL) || !__xuiColorPickerMetricValid(fWidth) || !__xuiColorPickerMetricValid(fHeight) ||
	     ((fWidth > 0.0f) && (fWidth < XUI_COLOR_PICKER_MIN_POPUP_W)) ||
	     ((fHeight > 0.0f) && (fHeight < XUI_COLOR_PICKER_MIN_POPUP_H)) ) return XUI_ERROR_INVALID_ARGUMENT;
	fNewWidth = (fWidth > 0.0f) ? fWidth : pData->fPopupWidth;
	fNewHeight = (fHeight > 0.0f) ? fHeight : pData->fPopupHeight;
	if ( (pData->fPopupWidth == fNewWidth) && (pData->fPopupHeight == fNewHeight) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	fOldWidth = pData->fPopupWidth;
	fOldHeight = pData->fPopupHeight;
	pData->fPopupWidth = fNewWidth;
	pData->fPopupHeight = fNewHeight;
	iRet = (pData->pPopup != NULL) ? __xuiColorPickerApplyPopupStyle(pWidget, pData) : XUI_OK;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( iRet == XUI_OK ) iRet = __xuiColorPickerInvalidateAll(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->fPopupWidth = fOldWidth;
		pData->fPopupHeight = fOldHeight;
		if ( pData->pPopup != NULL ) (void)__xuiColorPickerApplyPopupStyle(pWidget, pData);
	}
	return iRet;
}

XUI_API int xuiColorPickerGetPopupSize(xui_widget pWidget, float* pWidth, float* pHeight)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pWidth != NULL ) *pWidth = pData->fPopupWidth;
	if ( pHeight != NULL ) *pHeight = pData->fPopupHeight;
	return XUI_OK;
}

XUI_API int xuiColorPickerSetPopupPlacement(xui_widget pWidget, int iPlacement)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	xui_context pContext;
	int iOldPlacement;
	int iRet;
	if ( (pData == NULL) || !__xuiColorPickerPlacementValid(iPlacement) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iPopupPlacement == iPlacement ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	iOldPlacement = pData->iPopupPlacement;
	pData->iPopupPlacement = iPlacement;
	iRet = (pData->pPopup != NULL) ? __xuiColorPickerApplyPopupStyle(pWidget, pData) : XUI_OK;
	if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( iRet != XUI_OK ) {
		pData->iPopupPlacement = iOldPlacement;
		if ( pData->pPopup != NULL ) (void)__xuiColorPickerApplyPopupStyle(pWidget, pData);
	}
	return iRet;
}

XUI_API int xuiColorPickerGetPopupPlacement(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	return (pData != NULL) ? pData->iPopupPlacement : XUI_COLOR_PICKER_POPUP_AUTO;
}

XUI_API int xuiColorPickerSetMetrics(xui_widget pWidget, float fBorderWidth)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( (pData == NULL) || !__xuiColorPickerMetricValid(fBorderWidth) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fBorderWidth == fBorderWidth ) return XUI_OK;
	pData->fBorderWidth = fBorderWidth;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiColorPickerGetMetrics(xui_widget pWidget, float* pBorderWidth)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBorderWidth != NULL ) *pBorderWidth = pData->fBorderWidth;
	return XUI_OK;
}

XUI_API int xuiColorPickerSetColors(xui_widget pWidget, uint32_t iText, uint32_t iDisabledText, uint32_t iBackground, uint32_t iHoverBackground, uint32_t iOpenBackground, uint32_t iDisabledBackground)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pData->iTextColor == iText) && (pData->iDisabledTextColor == iDisabledText) &&
	     (pData->iBackgroundColor == iBackground) && (pData->iHoverBackgroundColor == iHoverBackground) &&
	     (pData->iOpenBackgroundColor == iOpenBackground) &&
	     (pData->iDisabledBackgroundColor == iDisabledBackground) ) return XUI_OK;
	pData->iTextColor = iText;
	pData->iDisabledTextColor = iDisabledText;
	pData->iBackgroundColor = iBackground;
	pData->iHoverBackgroundColor = iHoverBackground;
	pData->iOpenBackgroundColor = iOpenBackground;
	pData->iDisabledBackgroundColor = iDisabledBackground;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiColorPickerGetColors(xui_widget pWidget, uint32_t* pText, uint32_t* pDisabledText, uint32_t* pBackground, uint32_t* pHoverBackground, uint32_t* pOpenBackground, uint32_t* pDisabledBackground)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pText != NULL ) *pText = pData->iTextColor;
	if ( pDisabledText != NULL ) *pDisabledText = pData->iDisabledTextColor;
	if ( pBackground != NULL ) *pBackground = pData->iBackgroundColor;
	if ( pHoverBackground != NULL ) *pHoverBackground = pData->iHoverBackgroundColor;
	if ( pOpenBackground != NULL ) *pOpenBackground = pData->iOpenBackgroundColor;
	if ( pDisabledBackground != NULL ) *pDisabledBackground = pData->iDisabledBackgroundColor;
	return XUI_OK;
}

XUI_API int xuiColorPickerSetBorderColors(xui_widget pWidget, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iFocusBorder)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pData->iBorderColor == iBorder) && (pData->iHoverBorderColor == iHoverBorder) &&
	     (pData->iFocusBorderColor == iFocusBorder) ) return XUI_OK;
	pData->iBorderColor = iBorder;
	pData->iHoverBorderColor = iHoverBorder;
	pData->iFocusBorderColor = iFocusBorder;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiColorPickerGetBorderColors(xui_widget pWidget, uint32_t* pBorder, uint32_t* pHoverBorder, uint32_t* pFocusBorder)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBorder != NULL ) *pBorder = pData->iBorderColor;
	if ( pHoverBorder != NULL ) *pHoverBorder = pData->iHoverBorderColor;
	if ( pFocusBorder != NULL ) *pFocusBorder = pData->iFocusBorderColor;
	return XUI_OK;
}

XUI_API int xuiColorPickerSetArrowColors(xui_widget pWidget, uint32_t iArrow, uint32_t iDisabledArrow)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pData->iArrowColor == iArrow) && (pData->iDisabledArrowColor == iDisabledArrow) ) return XUI_OK;
	pData->iArrowColor = iArrow;
	pData->iDisabledArrowColor = iDisabledArrow;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiColorPickerGetArrowColors(xui_widget pWidget, uint32_t* pArrow, uint32_t* pDisabledArrow)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pArrow != NULL ) *pArrow = pData->iArrowColor;
	if ( pDisabledArrow != NULL ) *pDisabledArrow = pData->iDisabledArrowColor;
	return XUI_OK;
}

XUI_API int xuiColorPickerSetButtonColors(xui_widget pWidget, uint32_t iButton, uint32_t iButtonHover, uint32_t iButtonOpen)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pData->iButtonColor == iButton) && (pData->iButtonHoverColor == iButtonHover) &&
	     (pData->iButtonOpenColor == iButtonOpen) ) return XUI_OK;
	pData->iButtonColor = iButton;
	pData->iButtonHoverColor = iButtonHover;
	pData->iButtonOpenColor = iButtonOpen;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiColorPickerGetButtonColors(xui_widget pWidget, uint32_t* pButton, uint32_t* pButtonHover, uint32_t* pButtonOpen)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pButton != NULL ) *pButton = pData->iButtonColor;
	if ( pButtonHover != NULL ) *pButtonHover = pData->iButtonHoverColor;
	if ( pButtonOpen != NULL ) *pButtonOpen = pData->iButtonOpenColor;
	return XUI_OK;
}

XUI_API int xuiColorPickerSetPopupColors(xui_widget pWidget, uint32_t iPanel, uint32_t iBorder, uint32_t iShadow, uint32_t iText, uint32_t iMutedText, uint32_t iAccent, uint32_t iField, uint32_t iFieldBorder, uint32_t iSeparator)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	xui_context pContext;
	uint32_t arrOld[9];
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pData->iPopupPanelColor == iPanel) && (pData->iPopupBorderColor == iBorder) &&
	     (pData->iPopupShadowColor == iShadow) && (pData->iPopupTextColor == iText) &&
	     (pData->iPopupMutedTextColor == iMutedText) && (pData->iAccentColor == iAccent) &&
	     (pData->iFieldColor == iField) && (pData->iFieldBorderColor == iFieldBorder) &&
	     (pData->iSeparatorColor == iSeparator) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	arrOld[0] = pData->iPopupPanelColor;
	arrOld[1] = pData->iPopupBorderColor;
	arrOld[2] = pData->iPopupShadowColor;
	arrOld[3] = pData->iPopupTextColor;
	arrOld[4] = pData->iPopupMutedTextColor;
	arrOld[5] = pData->iAccentColor;
	arrOld[6] = pData->iFieldColor;
	arrOld[7] = pData->iFieldBorderColor;
	arrOld[8] = pData->iSeparatorColor;
	pData->iPopupPanelColor = iPanel;
	pData->iPopupBorderColor = iBorder;
	pData->iPopupShadowColor = iShadow;
	pData->iPopupTextColor = iText;
	pData->iPopupMutedTextColor = iMutedText;
	pData->iAccentColor = iAccent;
	pData->iFieldColor = iField;
	pData->iFieldBorderColor = iFieldBorder;
	pData->iSeparatorColor = iSeparator;
	if ( pData->pPopup != NULL ) {
		iRet = __xuiColorPickerApplyPopupStyle(pWidget, pData);
		if ( !__xuiColorPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
		if ( iRet != XUI_OK ) {
			pData->iPopupPanelColor = arrOld[0];
			pData->iPopupBorderColor = arrOld[1];
			pData->iPopupShadowColor = arrOld[2];
			pData->iPopupTextColor = arrOld[3];
			pData->iPopupMutedTextColor = arrOld[4];
			pData->iAccentColor = arrOld[5];
			pData->iFieldColor = arrOld[6];
			pData->iFieldBorderColor = arrOld[7];
			pData->iSeparatorColor = arrOld[8];
			(void)__xuiColorPickerApplyPopupStyle(pWidget, pData);
			return iRet;
		}
	}
	return __xuiColorPickerInvalidateAll(pWidget, pData);
}

XUI_API int xuiColorPickerGetPopupColors(xui_widget pWidget, uint32_t* pPanel, uint32_t* pBorder, uint32_t* pShadow, uint32_t* pText, uint32_t* pMutedText, uint32_t* pAccent, uint32_t* pField, uint32_t* pFieldBorder, uint32_t* pSeparator)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pPanel != NULL ) *pPanel = pData->iPopupPanelColor;
	if ( pBorder != NULL ) *pBorder = pData->iPopupBorderColor;
	if ( pShadow != NULL ) *pShadow = pData->iPopupShadowColor;
	if ( pText != NULL ) *pText = pData->iPopupTextColor;
	if ( pMutedText != NULL ) *pMutedText = pData->iPopupMutedTextColor;
	if ( pAccent != NULL ) *pAccent = pData->iAccentColor;
	if ( pField != NULL ) *pField = pData->iFieldColor;
	if ( pFieldBorder != NULL ) *pFieldBorder = pData->iFieldBorderColor;
	if ( pSeparator != NULL ) *pSeparator = pData->iSeparatorColor;
	return XUI_OK;
}

XUI_API int xuiColorPickerSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pFont == pFont ) return XUI_OK;
	pData->pFont = pFont;
	iRet = (pData->pPanel != NULL) ? xuiWidgetInvalidate(pData->pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) : XUI_OK;
	if ( iRet != XUI_OK ) return iRet;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API xui_font xuiColorPickerGetFont(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API xui_widget xuiColorPickerGetPopupWidget(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	return (pData != NULL) ? pData->pPopup : NULL;
}

XUI_API xui_widget xuiColorPickerGetPanelWidget(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	return (pData != NULL) ? pData->pPanel : NULL;
}

XUI_API xui_rect_t xuiColorPickerGetSwatchRect(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	__xuiColorPickerUpdateOwnerRects(pWidget, pData);
	return pData->tSwatchRect;
}

XUI_API xui_rect_t xuiColorPickerGetButtonRect(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	__xuiColorPickerUpdateOwnerRects(pWidget, pData);
	return pData->tButtonRect;
}

XUI_API xui_rect_t xuiColorPickerGetTextRect(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	__xuiColorPickerUpdateOwnerRects(pWidget, pData);
	return pData->tTextRect;
}

XUI_API xui_rect_t xuiColorPickerGetSvRect(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	__xuiColorPickerUpdatePanelRects(pData);
	return pData->tSvRect;
}

XUI_API xui_rect_t xuiColorPickerGetHueRect(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	__xuiColorPickerUpdatePanelRects(pData);
	return pData->tHueRect;
}

XUI_API xui_rect_t xuiColorPickerGetAlphaRect(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	__xuiColorPickerUpdatePanelRects(pData);
	return pData->tAlphaRect;
}

XUI_API xui_rect_t xuiColorPickerGetHexRect(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	__xuiColorPickerUpdatePanelRects(pData);
	return pData->tHexRect;
}

XUI_API xui_rect_t xuiColorPickerGetOldRect(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	__xuiColorPickerUpdatePanelRects(pData);
	return pData->tOldRect;
}

XUI_API xui_rect_t xuiColorPickerGetNewRect(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	__xuiColorPickerUpdatePanelRects(pData);
	return pData->tNewRect;
}

XUI_API xui_rect_t xuiColorPickerGetPaletteRect(xui_widget pWidget, int iIndex)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iPaletteCount) ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	__xuiColorPickerUpdatePanelRects(pData);
	return pData->arrPaletteRect[iIndex];
}

XUI_API int xuiColorPickerGetHoverPart(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	return (pData != NULL) ? pData->iHoverPart : XUI_COLOR_PICKER_PART_NONE;
}

XUI_API int xuiColorPickerGetActivePart(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	return (pData != NULL) ? pData->iActivePart : XUI_COLOR_PICKER_PART_NONE;
}

XUI_API uint32_t xuiColorPickerGetState(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	return __xuiColorPickerState(pWidget, pData);
}

XUI_API int xuiColorPickerGetChangeCount(xui_widget pWidget)
{
	xui_color_picker_data_t* pData = __xuiColorPickerGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
