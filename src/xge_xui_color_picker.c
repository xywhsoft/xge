#define XGE_XUI_COLOR_PICKER_AREA_NONE 0
#define XGE_XUI_COLOR_PICKER_AREA_SV 1
#define XGE_XUI_COLOR_PICKER_AREA_HUE 2
#define XGE_XUI_COLOR_PICKER_AREA_RGB_SLIDER 10

int xgeXuiColorPickerPopupEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
void xgeXuiColorPickerPopupPaintProc(xge_xui_widget pWidget, void* pUser);

static int __xgeXuiColorPickerClampByte(int iValue)
{
	if ( iValue < 0 ) {
		return 0;
	}
	if ( iValue > 255 ) {
		return 255;
	}
	return iValue;
}

static float __xgeXuiColorPickerClamp01(float fValue)
{
	if ( fValue < 0.0f ) {
		return 0.0f;
	}
	if ( fValue > 1.0f ) {
		return 1.0f;
	}
	return fValue;
}

static int __xgeXuiColorPickerHexNibble(char c)
{
	if ( c >= '0' && c <= '9' ) {
		return c - '0';
	}
	if ( c >= 'a' && c <= 'f' ) {
		return c - 'a' + 10;
	}
	if ( c >= 'A' && c <= 'F' ) {
		return c - 'A' + 10;
	}
	return -1;
}

static void __xgeXuiColorPickerFormatHex(xge_xui_color_picker pPicker)
{
	if ( pPicker == NULL ) {
		return;
	}
	if ( pPicker->bAlphaEnabled ) {
		snprintf(
			pPicker->sHex,
			sizeof(pPicker->sHex),
			"#%02X%02X%02X%02X",
			(unsigned int)XGE_COLOR_GET_R(pPicker->iColor),
			(unsigned int)XGE_COLOR_GET_G(pPicker->iColor),
			(unsigned int)XGE_COLOR_GET_B(pPicker->iColor),
			(unsigned int)XGE_COLOR_GET_A(pPicker->iColor));
	} else {
		snprintf(
			pPicker->sHex,
			sizeof(pPicker->sHex),
			"#%02X%02X%02X",
			(unsigned int)XGE_COLOR_GET_R(pPicker->iColor),
			(unsigned int)XGE_COLOR_GET_G(pPicker->iColor),
			(unsigned int)XGE_COLOR_GET_B(pPicker->iColor));
	}
	pPicker->sHex[sizeof(pPicker->sHex) - 1] = 0;
}

static int __xgeXuiColorPickerParseHex(const char* sHex, uint32_t* pColor)
{
	const char* s;
	int iLen;
	int i;
	int n0;
	int n1;
	unsigned int arr[4];

	if ( (sHex == NULL) || (pColor == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	s = (sHex[0] == '#') ? (sHex + 1) : sHex;
	iLen = (int)strlen(s);
	if ( iLen != 6 && iLen != 8 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iLen / 2; i++ ) {
		n0 = __xgeXuiColorPickerHexNibble(s[i * 2]);
		n1 = __xgeXuiColorPickerHexNibble(s[i * 2 + 1]);
		if ( n0 < 0 || n1 < 0 ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		arr[i] = (unsigned int)((n0 << 4) | n1);
	}
	if ( iLen == 6 ) {
		arr[3] = 255;
	}
	*pColor = XGE_COLOR_RGBA(arr[0], arr[1], arr[2], arr[3]);
	return XGE_OK;
}

static uint32_t __xgeXuiColorPickerOpaque(uint32_t iColor)
{
	return XGE_COLOR_RGBA(XGE_COLOR_GET_R(iColor), XGE_COLOR_GET_G(iColor), XGE_COLOR_GET_B(iColor), 255);
}

static uint32_t __xgeXuiColorPickerNormalizeColor(xge_xui_color_picker pPicker, uint32_t iColor)
{
	if ( (pPicker == NULL) || (pPicker->bAlphaEnabled == 0) ) {
		return __xgeXuiColorPickerOpaque(iColor);
	}
	return iColor;
}

static uint32_t __xgeXuiColorPickerTextOnColor(uint32_t iColor)
{
	int iR;
	int iG;
	int iB;
	int iLuma;

	iR = (int)XGE_COLOR_GET_R(iColor);
	iG = (int)XGE_COLOR_GET_G(iColor);
	iB = (int)XGE_COLOR_GET_B(iColor);
	iLuma = iR * 299 + iG * 587 + iB * 114;
	return (iLuma > 145000) ? XGE_COLOR_RGBA(22, 34, 44, 255) : XGE_COLOR_RGBA(255, 255, 255, 255);
}

static uint32_t __xgeXuiColorPickerMix(uint32_t iA, uint32_t iB, float fT)
{
	int iR;
	int iG;
	int iBlue;

	fT = __xgeXuiColorPickerClamp01(fT);
	iR = (int)((float)XGE_COLOR_GET_R(iA) + ((float)XGE_COLOR_GET_R(iB) - (float)XGE_COLOR_GET_R(iA)) * fT + 0.5f);
	iG = (int)((float)XGE_COLOR_GET_G(iA) + ((float)XGE_COLOR_GET_G(iB) - (float)XGE_COLOR_GET_G(iA)) * fT + 0.5f);
	iBlue = (int)((float)XGE_COLOR_GET_B(iA) + ((float)XGE_COLOR_GET_B(iB) - (float)XGE_COLOR_GET_B(iA)) * fT + 0.5f);
	return XGE_COLOR_RGBA(iR, iG, iBlue, 255);
}

static uint32_t __xgeXuiColorPickerHsvToRgb(float fHue, float fSat, float fVal)
{
	float fC;
	float fX;
	float fM;
	float fR;
	float fG;
	float fB;
	int iSector;

	fHue = fHue - floorf(fHue);
	fSat = __xgeXuiColorPickerClamp01(fSat);
	fVal = __xgeXuiColorPickerClamp01(fVal);
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
	return XGE_COLOR_RGBA((int)((fR + fM) * 255.0f + 0.5f), (int)((fG + fM) * 255.0f + 0.5f), (int)((fB + fM) * 255.0f + 0.5f), 255);
}

static void __xgeXuiColorPickerRgbToHsv(uint32_t iColor, float* pHue, float* pSat, float* pVal)
{
	float fR;
	float fG;
	float fB;
	float fMax;
	float fMin;
	float fDelta;
	float fHue;

	fR = (float)XGE_COLOR_GET_R(iColor) / 255.0f;
	fG = (float)XGE_COLOR_GET_G(iColor) / 255.0f;
	fB = (float)XGE_COLOR_GET_B(iColor) / 255.0f;
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
	if ( pHue != NULL ) {
		*pHue = fHue;
	}
	if ( pSat != NULL ) {
		*pSat = (fMax <= 0.0001f) ? 0.0f : (fDelta / fMax);
	}
	if ( pVal != NULL ) {
		*pVal = fMax;
	}
}

static int __xgeXuiColorPickerFindPaletteColor(xge_xui_color_picker pPicker, uint32_t iColor)
{
	int i;
	uint32_t iTarget;

	if ( pPicker == NULL ) {
		return -1;
	}
	iTarget = (pPicker->bAlphaEnabled != 0) ? iColor : __xgeXuiColorPickerOpaque(iColor);
	for ( i = 0; i < pPicker->iPaletteCount; i++ ) {
		if ( ((pPicker->bAlphaEnabled != 0) ? pPicker->arrPalette[i] : __xgeXuiColorPickerOpaque(pPicker->arrPalette[i])) == iTarget ) {
			return i;
		}
	}
	return -1;
}

static void __xgeXuiColorPickerSyncSelectedPalette(xge_xui_color_picker pPicker)
{
	if ( pPicker == NULL ) {
		return;
	}
	pPicker->iSelectedPalette = __xgeXuiColorPickerFindPaletteColor(pPicker, pPicker->iColor);
}

static void __xgeXuiColorPickerNotify(xge_xui_color_picker pPicker)
{
	if ( pPicker == NULL ) {
		return;
	}
	pPicker->iChangeCount++;
	if ( pPicker->procChange != NULL ) {
		pPicker->procChange(pPicker->pWidget, pPicker->iColor, pPicker->pUser);
	}
}

static void __xgeXuiColorPickerMarkPaint(xge_xui_color_picker pPicker)
{
	if ( pPicker == NULL ) {
		return;
	}
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
	xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
}

static void __xgeXuiColorPickerSetColorInternal(xge_xui_color_picker pPicker, uint32_t iColor, int bNotify)
{
	if ( pPicker == NULL ) {
		return;
	}
	iColor = __xgeXuiColorPickerNormalizeColor(pPicker, iColor);
	if ( pPicker->iColor == iColor ) {
		__xgeXuiColorPickerFormatHex(pPicker);
		return;
	}
	pPicker->iColor = iColor;
	__xgeXuiColorPickerFormatHex(pPicker);
	__xgeXuiColorPickerRgbToHsv(iColor, &pPicker->fHue, &pPicker->fSaturation, &pPicker->fValue);
	__xgeXuiColorPickerSyncSelectedPalette(pPicker);
	__xgeXuiColorPickerMarkPaint(pPicker);
	if ( bNotify ) {
		__xgeXuiColorPickerNotify(pPicker);
	}
}

static void __xgeXuiColorPickerSetHsvInternal(xge_xui_color_picker pPicker, float fHue, float fSat, float fVal, int bNotify)
{
	if ( pPicker == NULL ) {
		return;
	}
	pPicker->fHue = __xgeXuiColorPickerClamp01(fHue);
	pPicker->fSaturation = __xgeXuiColorPickerClamp01(fSat);
	pPicker->fValue = __xgeXuiColorPickerClamp01(fVal);
	__xgeXuiColorPickerSetColorInternal(pPicker, XGE_COLOR_RGBA(XGE_COLOR_GET_R(__xgeXuiColorPickerHsvToRgb(pPicker->fHue, pPicker->fSaturation, pPicker->fValue)), XGE_COLOR_GET_G(__xgeXuiColorPickerHsvToRgb(pPicker->fHue, pPicker->fSaturation, pPicker->fValue)), XGE_COLOR_GET_B(__xgeXuiColorPickerHsvToRgb(pPicker->fHue, pPicker->fSaturation, pPicker->fValue)), XGE_COLOR_GET_A(pPicker->iColor)), bNotify);
}

static void __xgeXuiColorPickerLayoutMain(xge_xui_color_picker pPicker)
{
	xge_rect_t tRect;
	float fInset;

	if ( (pPicker == NULL) || (pPicker->pWidget == NULL) ) {
		return;
	}
	tRect = pPicker->pWidget->tBorderRect;
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		tRect = pPicker->pWidget->tRect;
	}
	pPicker->tButtonRect = tRect;
	pPicker->tButtonRect.fX = tRect.fX + tRect.fW - tRect.fH;
	pPicker->tButtonRect.fW = tRect.fH;
	fInset = 2.0f;
	pPicker->tSwatchRect.fX = tRect.fX + fInset;
	pPicker->tSwatchRect.fY = tRect.fY + fInset;
	pPicker->tSwatchRect.fW = pPicker->tButtonRect.fX - tRect.fX - fInset * 2.0f;
	pPicker->tSwatchRect.fH = tRect.fH - fInset * 2.0f;
	if ( pPicker->tSwatchRect.fW < 1.0f ) {
		pPicker->tSwatchRect.fW = 1.0f;
	}
}

static void __xgeXuiColorPickerLayoutPopup(xge_xui_color_picker pPicker)
{
	xge_rect_t tRect;
	float fX;
	float fY;
	float fGap;
	float fHueBaseX;
	float fCell;
	float fFieldX;
	float fControlRight;
	float fPaletteGap;
	int i;

	if ( (pPicker == NULL) || (pPicker->pPopupWidget == NULL) ) {
		return;
	}
	tRect = pPicker->pPopupWidget->tContentRect;
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		tRect = pPicker->pPopupWidget->tRect;
	}
	fGap = 10.0f;
	pPicker->tSvRect = (xge_rect_t){ tRect.fX + 14.0f, tRect.fY + 18.0f, 272.0f, 252.0f };
	fHueBaseX = pPicker->tSvRect.fX + pPicker->tSvRect.fW + fGap;
	pPicker->tHueRect = (xge_rect_t){ fHueBaseX + 12.0f, pPicker->tSvRect.fY, 28.0f, pPicker->tSvRect.fH };
	fFieldX = fHueBaseX + 28.0f + 30.0f;
	pPicker->tOldRect = (xge_rect_t){ fFieldX + 38.0f, tRect.fY + 30.0f, 72.0f, 38.0f };
	pPicker->tNewRect = (xge_rect_t){ pPicker->tOldRect.fX + pPicker->tOldRect.fW + 18.0f, pPicker->tOldRect.fY, 72.0f, 38.0f };
	fControlRight = tRect.fX + tRect.fW - 18.0f;
	for ( i = 0; i < 4; i++ ) {
		pPicker->arrFieldRect[i] = (xge_rect_t){ fFieldX + 30.0f, tRect.fY + 92.0f + (float)i * 34.0f, 72.0f, 26.0f };
		pPicker->arrSliderRect[i] = (xge_rect_t){ fFieldX + 122.0f, pPicker->arrFieldRect[i].fY + 10.5f, fControlRight - (fFieldX + 122.0f), 5.0f };
	}
	pPicker->tHexCopyRect = (xge_rect_t){ fControlRight - 28.0f, tRect.fY + 244.0f, 28.0f, 26.0f };
	pPicker->tHexRect = (xge_rect_t){ fFieldX + 30.0f, pPicker->tHexCopyRect.fY, pPicker->tHexCopyRect.fX - (fFieldX + 40.0f), 26.0f };
	pPicker->tSvRect.fH = (pPicker->tHexRect.fY + pPicker->tHexRect.fH) - pPicker->tSvRect.fY;
	pPicker->tHueRect.fH = pPicker->tSvRect.fH;
	fCell = 40.0f;
	fX = tRect.fX + 14.0f;
	fY = tRect.fY + tRect.fH - 54.0f;
	fPaletteGap = (pPicker->iPaletteCount > 1) ? ((tRect.fW - 28.0f - (float)pPicker->iPaletteCount * fCell) / (float)(pPicker->iPaletteCount - 1)) : 0.0f;
	if ( fPaletteGap < 0.0f ) {
		fPaletteGap = 0.0f;
	}
	for ( i = 0; i < pPicker->iPaletteCount; i++ ) {
		pPicker->arrPaletteRect[i] = (xge_rect_t){ fX + (float)i * (fCell + fPaletteGap), fY, fCell, fCell };
	}
}

static void __xgeXuiColorPickerOpenPopup(xge_xui_color_picker pPicker, int bOpen)
{
	xge_rect_t tRect;
	float fWindowW;
	float fWindowH;

	if ( (pPicker == NULL) || (pPicker->pPopupWidget == NULL) ) {
		return;
	}
	bOpen = bOpen ? 1 : 0;
	if ( bOpen ) {
		__xgeXuiColorPickerLayoutMain(pPicker);
		pPicker->iOldColor = pPicker->iColor;
		tRect = (xge_rect_t){ pPicker->pWidget->tBorderRect.fX, pPicker->pWidget->tBorderRect.fY + pPicker->pWidget->tBorderRect.fH + 1.0f, 620.0f, 368.0f };
		fWindowW = (float)xgeGetWidth();
		fWindowH = (float)xgeGetHeight();
		if ( tRect.fX + tRect.fW > fWindowW - 8.0f ) {
			tRect.fX = fWindowW - tRect.fW - 8.0f;
		}
		if ( tRect.fX < 8.0f ) {
			tRect.fX = 8.0f;
		}
		if ( tRect.fY + tRect.fH > fWindowH - 8.0f ) {
			tRect.fY = pPicker->pWidget->tBorderRect.fY - tRect.fH - 1.0f;
		}
		if ( tRect.fY < 8.0f ) {
			tRect.fY = 8.0f;
		}
		xgeXuiWidgetSetRect(pPicker->pPopupWidget, tRect);
		__xgeXuiColorPickerLayoutPopup(pPicker);
	}
	xgeXuiPopupSetOpen(pPicker->pPopup, bOpen);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

static void __xgeXuiColorPickerPopupClose(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_color_picker pPicker;

	(void)pWidget;
	pPicker = (xge_xui_color_picker)pUser;
	if ( pPicker == NULL ) {
		return;
	}
	pPicker->iActiveArea = XGE_XUI_COLOR_PICKER_AREA_NONE;
	pPicker->iActiveField = -1;
	pPicker->bCopyHover = 0;
	pPicker->bCopyActive = 0;
	pPicker->bEditError = 0;
	xgeXuiTextSet(&pPicker->tEditText, "");
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

static void __xgeXuiColorPickerBeginEdit(xge_xui_color_picker pPicker, int iField)
{
	char sValue[16];
	int iValue;

	if ( (pPicker == NULL) || (iField < 0) || (iField > 4) || ((iField == 3) && (pPicker->bAlphaEnabled == 0)) ) {
		return;
	}
	pPicker->iActiveField = iField;
	pPicker->bEditError = 0;
	if ( iField == 4 ) {
		xgeXuiTextSet(&pPicker->tEditText, xgeXuiColorPickerGetHex(pPicker) + 1);
	} else {
		iValue = (iField == 0) ? (int)XGE_COLOR_GET_R(pPicker->iColor) : ((iField == 1) ? (int)XGE_COLOR_GET_G(pPicker->iColor) : ((iField == 2) ? (int)XGE_COLOR_GET_B(pPicker->iColor) : (int)XGE_COLOR_GET_A(pPicker->iColor)));
		snprintf(sValue, sizeof(sValue), "%d", iValue);
		sValue[sizeof(sValue) - 1] = 0;
		xgeXuiTextSet(&pPicker->tEditText, sValue);
	}
	xgeXuiTextSetSelection(&pPicker->tEditText, 0, pPicker->tEditText.iSize);
	xgeXuiWidgetSetImeMode(pPicker->pPopupWidget, XGE_XUI_IME_ENABLED);
	xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
}

static void __xgeXuiColorPickerCancelEdit(xge_xui_color_picker pPicker)
{
	if ( pPicker == NULL ) {
		return;
	}
	pPicker->iActiveField = -1;
	pPicker->bEditError = 0;
	xgeXuiTextSet(&pPicker->tEditText, "");
	xgeXuiWidgetSetImeMode(pPicker->pPopupWidget, XGE_XUI_IME_DISABLED);
	xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
}

static int __xgeXuiColorPickerParseByteText(const char* sText, int* pValue)
{
	const char* p;
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) || (pValue == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iValue = 0;
	for ( p = sText; *p != 0; p++ ) {
		if ( (*p < '0') || (*p > '9') ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iValue = iValue * 10 + (*p - '0');
	}
	*pValue = __xgeXuiColorPickerClampByte(iValue);
	return XGE_OK;
}

static int __xgeXuiColorPickerCommitEdit(xge_xui_color_picker pPicker, int bNotify)
{
	const char* sText;
	uint32_t iColor;
	int iR;
	int iG;
	int iB;
	int iA;
	int iValue;

	if ( (pPicker == NULL) || (pPicker->iActiveField < 0) ) {
		return XGE_OK;
	}
	sText = (pPicker->tEditText.sText != NULL) ? pPicker->tEditText.sText : "";
	if ( pPicker->iActiveField == 4 ) {
		if ( __xgeXuiColorPickerParseHex(sText, &iColor) != XGE_OK ) {
			pPicker->bEditError = 1;
			xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	} else {
		if ( __xgeXuiColorPickerParseByteText(sText, &iValue) != XGE_OK ) {
			pPicker->bEditError = 1;
			xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iR = (int)XGE_COLOR_GET_R(pPicker->iColor);
		iG = (int)XGE_COLOR_GET_G(pPicker->iColor);
		iB = (int)XGE_COLOR_GET_B(pPicker->iColor);
		iA = (int)XGE_COLOR_GET_A(pPicker->iColor);
		if ( pPicker->iActiveField == 0 ) {
			iR = iValue;
		} else if ( pPicker->iActiveField == 1 ) {
			iG = iValue;
		} else if ( pPicker->iActiveField == 2 ) {
			iB = iValue;
		} else {
			iA = iValue;
		}
		iColor = XGE_COLOR_RGBA(iR, iG, iB, iA);
	}
	__xgeXuiColorPickerCancelEdit(pPicker);
	__xgeXuiColorPickerSetColorInternal(pPicker, iColor, bNotify);
	return XGE_OK;
}

static int __xgeXuiColorPickerTextCanAppend(xge_xui_color_picker pPicker, uint32_t iCodepoint)
{
	const char* sText;
	int iLen;
	int iStart;
	int iEnd;
	int iLimit;
	int bHasHash;

	if ( (pPicker == NULL) || (pPicker->iActiveField < 0) || (iCodepoint > 127u) ) {
		return 0;
	}
	sText = (pPicker->tEditText.sText != NULL) ? pPicker->tEditText.sText : "";
	iLen = (int)strlen(sText);
	xgeXuiTextGetSelection(&pPicker->tEditText, &iStart, &iEnd);
	if ( iStart > iEnd ) {
		int iTemp = iStart;
		iStart = iEnd;
		iEnd = iTemp;
	}
	if ( iStart < 0 ) {
		iStart = 0;
	}
	if ( iEnd > iLen ) {
		iEnd = iLen;
	}
	iLen -= iEnd - iStart;
	if ( pPicker->iActiveField == 4 ) {
		bHasHash = (sText[0] == '#' && !(iStart == 0 && iEnd > 0)) ? 1 : 0;
		if ( iCodepoint == '#' ) {
			return (iStart == 0) && (bHasHash == 0);
		}
		if ( __xgeXuiColorPickerHexNibble((char)iCodepoint) < 0 ) {
			return 0;
		}
		iLimit = bHasHash ? (pPicker->bAlphaEnabled ? 9 : 7) : (pPicker->bAlphaEnabled ? 8 : 6);
		return iLen < iLimit;
	}
	return (iCodepoint >= '0') && (iCodepoint <= '9') && (iLen < 3);
}

static void __xgeXuiColorPickerPickSv(xge_xui_color_picker pPicker, float fX, float fY, int bNotify)
{
	float fS;
	float fV;

	if ( pPicker == NULL ) {
		return;
	}
	fS = (fX - pPicker->tSvRect.fX) / pPicker->tSvRect.fW;
	fV = 1.0f - ((fY - pPicker->tSvRect.fY) / pPicker->tSvRect.fH);
	__xgeXuiColorPickerSetHsvInternal(pPicker, pPicker->fHue, fS, fV, bNotify);
}

static void __xgeXuiColorPickerPickHue(xge_xui_color_picker pPicker, float fY, int bNotify)
{
	float fHue;

	if ( pPicker == NULL ) {
		return;
	}
	fHue = (fY - pPicker->tHueRect.fY) / pPicker->tHueRect.fH;
	__xgeXuiColorPickerSetHsvInternal(pPicker, fHue, pPicker->fSaturation, pPicker->fValue, bNotify);
}

static void __xgeXuiColorPickerPickRgbSlider(xge_xui_color_picker pPicker, int iIndex, float fX, int bNotify)
{
	xge_rect_t tRect;
	float fRate;
	int iR;
	int iG;
	int iB;
	int iA;

	if ( (pPicker == NULL) || (iIndex < 0) || (iIndex > 3) || ((iIndex == 3) && (pPicker->bAlphaEnabled == 0)) ) {
		return;
	}
	tRect = pPicker->arrSliderRect[iIndex];
	fRate = __xgeXuiColorPickerClamp01((fX - tRect.fX) / tRect.fW);
	iR = (int)XGE_COLOR_GET_R(pPicker->iColor);
	iG = (int)XGE_COLOR_GET_G(pPicker->iColor);
	iB = (int)XGE_COLOR_GET_B(pPicker->iColor);
	iA = (int)XGE_COLOR_GET_A(pPicker->iColor);
	if ( iIndex == 0 ) {
		iR = (int)(fRate * 255.0f + 0.5f);
	} else if ( iIndex == 1 ) {
		iG = (int)(fRate * 255.0f + 0.5f);
	} else if ( iIndex == 2 ) {
		iB = (int)(fRate * 255.0f + 0.5f);
	} else {
		iA = (int)(fRate * 255.0f + 0.5f);
	}
	__xgeXuiColorPickerSetColorInternal(pPicker, XGE_COLOR_RGBA(iR, iG, iB, iA), bNotify);
}

static void __xgeXuiColorPickerDrawGradientHorizontal(xge_rect_t tRect, uint32_t iLeft, uint32_t iRight, int iSteps)
{
	int i;
	xge_rect_t tStep;

	if ( iSteps <= 0 ) {
		return;
	}
	tStep = tRect;
	tStep.fW = tRect.fW / (float)iSteps + 1.0f;
	for ( i = 0; i < iSteps; i++ ) {
		tStep.fX = tRect.fX + (float)i * tRect.fW / (float)iSteps;
		__xgeXuiHostDrawRect(tStep, __xgeXuiColorPickerMix(iLeft, iRight, (float)i / (float)(iSteps - 1)));
	}
}

static void __xgeXuiColorPickerDrawHue(xge_rect_t tRect)
{
	int i;
	xge_rect_t tStep;
	uint32_t iA;
	uint32_t iB;
	float f0;
	float f1;
	int iSteps;

	iSteps = 96;
	tRect.fX += 1.0f;
	tRect.fY += 1.0f;
	tRect.fW -= 2.0f;
	tRect.fH -= 2.0f;
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		return;
	}
	tStep = tRect;
	tStep.fH = tRect.fH / (float)iSteps;
	for ( i = 0; i < iSteps; i++ ) {
		f0 = (float)i / (float)iSteps;
		f1 = (float)(i + 1) / (float)iSteps;
		iA = __xgeXuiColorPickerHsvToRgb(f0, 1.0f, 1.0f);
		iB = __xgeXuiColorPickerHsvToRgb(f1, 1.0f, 1.0f);
		tStep.fY = tRect.fY + (float)i * tRect.fH / (float)iSteps;
		__xgeXuiHostDrawRect(tStep, __xgeXuiColorPickerMix(iA, iB, 0.5f));
	}
}

static void __xgeXuiColorPickerDrawSv(xge_xui_color_picker pPicker)
{
	int i;
	int j;
	int iCols;
	int iRows;
	xge_rect_t tCell;
	xge_rect_t tRect;
	uint32_t iHue;
	uint32_t iSat;
	uint32_t iColor;
	float fS;
	float fV;

	if ( pPicker == NULL ) {
		return;
	}
	tRect = pPicker->tSvRect;
	tRect.fX += 1.0f;
	tRect.fY += 1.0f;
	tRect.fW -= 2.0f;
	tRect.fH -= 2.0f;
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		return;
	}
	iCols = 40;
	iRows = 32;
	iHue = __xgeXuiColorPickerHsvToRgb(pPicker->fHue, 1.0f, 1.0f);
	tCell.fW = tRect.fW / (float)iCols;
	tCell.fH = tRect.fH / (float)iRows;
	for ( j = 0; j < iRows; j++ ) {
		fV = 1.0f - ((float)j / (float)(iRows - 1));
		for ( i = 0; i < iCols; i++ ) {
			fS = (float)i / (float)(iCols - 1);
			iSat = __xgeXuiColorPickerMix(XGE_COLOR_RGBA(255, 255, 255, 255), iHue, fS);
			iColor = __xgeXuiColorPickerMix(XGE_COLOR_RGBA(0, 0, 0, 255), iSat, fV);
			tCell.fX = tRect.fX + (float)i * tRect.fW / (float)iCols;
			tCell.fY = tRect.fY + (float)j * tRect.fH / (float)iRows;
			__xgeXuiHostDrawRect(tCell, iColor);
		}
	}
}

static void __xgeXuiColorPickerDrawChevron(xge_rect_t tButton, uint32_t iColor, int bOpen)
{
	static const uint16_t arrChevronUp8[8] = {
		0x00, 0x00, 0x42, 0x66, 0x3c, 0x18, 0x00, 0x00
	};
	static const uint16_t arrChevronDown8[8] = {
		0x00, 0x00, 0x18, 0x3c, 0x66, 0x42, 0x00, 0x00
	};
	xge_rect_t tIcon;

	tIcon.fW = 8.0f;
	tIcon.fH = 8.0f;
	tIcon.fX = tButton.fX + (tButton.fW - tIcon.fW) * 0.5f;
	tIcon.fY = tButton.fY + (tButton.fH - tIcon.fH) * 0.5f;
	__xgeXuiHostDrawBitmapMask(tIcon, bOpen ? arrChevronDown8 : arrChevronUp8, 8, 8, iColor);
}

static void __xgeXuiColorPickerDrawCopyIcon(xge_rect_t tRect, uint32_t iColor)
{
	xge_rect_t tBack;
	xge_rect_t tFront;

	tBack = (xge_rect_t){ tRect.fX + (tRect.fW - 12.0f) * 0.5f + 2.0f, tRect.fY + (tRect.fH - 12.0f) * 0.5f, 8.0f, 8.0f };
	tFront = (xge_rect_t){ tBack.fX - 3.0f, tBack.fY + 3.0f, 8.0f, 8.0f };
	__xgeXuiHostDrawBorderRect(tBack, 1.0f, iColor);
	__xgeXuiHostDrawBorderRect(tFront, 1.0f, iColor);
}

static void __xgeXuiColorPickerDrawEditText(xge_xui_color_picker pPicker, xge_rect_t tRect, const char* sText, int bActive)
{
	xge_rect_t tSelection;
	xge_vec2_t tTextSize;
	int iStart;
	int iEnd;

	if ( (pPicker == NULL) || (pPicker->pFont == NULL) || (sText == NULL) ) {
		return;
	}
	if ( bActive ) {
		xgeXuiTextGetSelection(&pPicker->tEditText, &iStart, &iEnd);
		if ( iStart != iEnd ) {
			tTextSize = __xgeXuiHostMeasureText(pPicker->pFont, sText);
			tSelection.fW = tTextSize.fX + 8.0f;
			if ( tSelection.fW > tRect.fW - 6.0f ) {
				tSelection.fW = tRect.fW - 6.0f;
			}
			tSelection.fH = tTextSize.fY + 4.0f;
			if ( tSelection.fH > tRect.fH - 6.0f ) {
				tSelection.fH = tRect.fH - 6.0f;
			}
			tSelection.fX = tRect.fX + (tRect.fW - tSelection.fW) * 0.5f;
			tSelection.fY = tRect.fY + (tRect.fH - tSelection.fH) * 0.5f;
			if ( tSelection.fW > 0.0f && tSelection.fH > 0.0f ) {
				__xgeXuiHostDrawRect(tSelection, XGE_COLOR_RGBA(46, 132, 220, 255));
			}
			__xgeXuiHostDrawTextRect(pPicker->pFont, sText, tRect, XGE_COLOR_RGBA(255, 255, 255, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			return;
		}
	}
	__xgeXuiHostDrawTextRect(pPicker->pFont, sText, tRect, XGE_COLOR_RGBA(220, 235, 245, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
}

int xgeXuiColorPickerInit(xge_xui_color_picker pPicker, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	const xge_xui_theme_t* pTheme;
	static const uint32_t arrDefaultPalette[11] = {
		XGE_COLOR_RGBA(0, 0, 0, 0),
		XGE_COLOR_RGBA(0, 0, 0, 255),
		XGE_COLOR_RGBA(255, 255, 255, 255),
		XGE_COLOR_RGBA(148, 158, 168, 255),
		XGE_COLOR_RGBA(225, 58, 70, 255),
		XGE_COLOR_RGBA(230, 126, 34, 255),
		XGE_COLOR_RGBA(244, 201, 54, 255),
		XGE_COLOR_RGBA(74, 165, 91, 255),
		XGE_COLOR_RGBA(43, 184, 203, 255),
		XGE_COLOR_RGBA(46, 124, 214, 255),
		XGE_COLOR_RGBA(132, 86, 209, 255)
	};

	if ( (pPicker == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pPicker, 0, sizeof(*pPicker));
	if ( xgeXuiTextInit(&pPicker->tEditText) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pPicker->pPopupWidget = xgeXuiWidgetCreate();
	pPicker->pPopup = (xge_xui_popup)xrtMalloc(sizeof(xge_xui_popup_t));
	if ( (pPicker->pPopupWidget == NULL) || (pPicker->pPopup == NULL) ) {
		xgeXuiWidgetFree(pPicker->pPopupWidget);
		xrtFree(pPicker->pPopup);
		xgeXuiTextUnit(&pPicker->tEditText);
		memset(pPicker, 0, sizeof(*pPicker));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pPicker->pPopup, 0, sizeof(xge_xui_popup_t));
	pPicker->pContext = pContext;
	pPicker->pWidget = pWidget;
	pPicker->pFont = (pFont != NULL) ? pFont : pTheme->pFont;
	pPicker->iHoverPalette = -1;
	pPicker->iSelectedPalette = -1;
	pPicker->iActiveField = -1;
	pPicker->iActiveArea = XGE_XUI_COLOR_PICKER_AREA_NONE;
	pPicker->iPanelColor = XGE_COLOR_RGBA(31, 44, 56, 255);
	pPicker->iBorderColor = XGE_COLOR_RGBA(126, 166, 200, 255);
	pPicker->iTextColor = pTheme->iTextColor;
	pPicker->iMutedTextColor = XGE_COLOR_RGBA(176, 192, 207, 255);
	pPicker->iAccentColor = pTheme->iAccentColor;
	pPicker->iFieldColor = XGE_COLOR_RGBA(246, 251, 255, 255);
	pPicker->iHoverColor = XGE_COLOR_RGBA(228, 242, 252, 255);
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, pPicker->iBorderColor);
	xgeXuiWidgetSetStateBackground(pWidget, XGE_XUI_STATE_HOVER, XGE_COLOR_RGBA(232, 244, 252, 255));
	xgeXuiWidgetSetStateBackground(pWidget, XGE_XUI_STATE_DISABLED, XGE_COLOR_RGBA(235, 240, 245, 255));
	xgeXuiWidgetSetStateBorder(pWidget, XGE_XUI_STATE_FOCUS, 1.0f, pPicker->iAccentColor);
	xgeXuiWidgetSetClip(pWidget, 0);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiColorPickerEventProc, NULL);
	pWidget->procPaint = xgeXuiColorPickerPaintProc;
	pWidget->pUser = pPicker;
	xgeXuiPopupInit(pPicker->pPopup, pContext, pPicker->pPopupWidget);
	xgeXuiPopupSetOwner(pPicker->pPopup, pWidget);
	xgeXuiPopupSetFocusRestore(pPicker->pPopup, pWidget);
	xgeXuiPopupSetPlacement(pPicker->pPopup, XGE_XUI_OVERLAY_PLACEMENT_MANUAL);
	xgeXuiPopupSetClose(pPicker->pPopup, __xgeXuiColorPickerPopupClose, pPicker);
	xgeXuiPopupSetBackground(pPicker->pPopup, pPicker->iPanelColor);
	xgeXuiPopupSetBorder(pPicker->pPopup, XGE_COLOR_RGBA(94, 128, 158, 255));
	xgeXuiWidgetSetPaddingPx(pPicker->pPopupWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetEvent(pPicker->pPopupWidget, xgeXuiColorPickerPopupEventProc, NULL);
	pPicker->pPopupWidget->procPaint = xgeXuiColorPickerPopupPaintProc;
	pPicker->pPopupWidget->pUser = pPicker;
	if ( xgeXuiOverlayAttach(pContext, pPicker->pPopupWidget, pWidget, XGE_XUI_LAYER_POPUP) != XGE_OK ) {
		xgeXuiPopupUnit(pPicker->pPopup);
		xgeXuiWidgetFree(pPicker->pPopupWidget);
		xrtFree(pPicker->pPopup);
		xgeXuiTextUnit(&pPicker->tEditText);
		memset(pPicker, 0, sizeof(*pPicker));
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiColorPickerSetPalette(pPicker, arrDefaultPalette, 11);
	__xgeXuiColorPickerSetColorInternal(pPicker, XGE_COLOR_RGBA(130, 183, 55, 255), 0);
	return XGE_OK;
}

void xgeXuiColorPickerUnit(xge_xui_color_picker pPicker)
{
	xge_xui_widget pPopupWidget;

	if ( pPicker == NULL ) {
		return;
	}
	if ( pPicker->pWidget != NULL && pPicker->pWidget->pUser == pPicker ) {
		pPicker->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pPicker->pWidget, NULL, NULL);
		pPicker->pWidget->procPaint = NULL;
	}
	pPopupWidget = pPicker->pPopupWidget;
	xgeXuiPopupUnit(pPicker->pPopup);
	xgeXuiWidgetFree(pPopupWidget);
	xrtFree(pPicker->pPopup);
	xgeXuiTextUnit(&pPicker->tEditText);
	memset(pPicker, 0, sizeof(*pPicker));
}

void xgeXuiColorPickerSetChange(xge_xui_color_picker pPicker, xge_xui_color_proc procChange, void* pUser)
{
	if ( pPicker != NULL ) {
		pPicker->procChange = procChange;
		pPicker->pUser = pUser;
	}
}

void xgeXuiColorPickerSetColor(xge_xui_color_picker pPicker, uint32_t iColor)
{
	__xgeXuiColorPickerSetColorInternal(pPicker, iColor, 0);
}

uint32_t xgeXuiColorPickerGetColor(xge_xui_color_picker pPicker)
{
	return (pPicker != NULL) ? pPicker->iColor : 0;
}

void xgeXuiColorPickerSetRGBA(xge_xui_color_picker pPicker, int iR, int iG, int iB, int iA)
{
	__xgeXuiColorPickerSetColorInternal(pPicker, XGE_COLOR_RGBA(__xgeXuiColorPickerClampByte(iR), __xgeXuiColorPickerClampByte(iG), __xgeXuiColorPickerClampByte(iB), __xgeXuiColorPickerClampByte(iA)), 0);
}

static void __xgeXuiColorPickerDrawSwatch(xge_rect_t tRect, uint32_t iColor)
{
	xge_rect_t tCell;
	float fRight;
	float fBottom;
	float fCellRight;
	float fCellBottom;
	int i;
	int j;
	int iCols;
	int iRows;

	if ( XGE_COLOR_GET_A(iColor) < 255 ) {
		iCols = (int)(tRect.fW / 8.0f) + 1;
		iRows = (int)(tRect.fH / 8.0f) + 1;
		fRight = tRect.fX + tRect.fW;
		fBottom = tRect.fY + tRect.fH;
		tCell.fW = 8.0f;
		tCell.fH = 8.0f;
		for ( j = 0; j < iRows; j++ ) {
			for ( i = 0; i < iCols; i++ ) {
				tCell.fX = tRect.fX + (float)i * 8.0f;
				tCell.fY = tRect.fY + (float)j * 8.0f;
				fCellRight = tCell.fX + 8.0f;
				fCellBottom = tCell.fY + 8.0f;
				if ( fCellRight > fRight ) {
					tCell.fW = fRight - tCell.fX;
				} else {
					tCell.fW = 8.0f;
				}
				if ( fCellBottom > fBottom ) {
					tCell.fH = fBottom - tCell.fY;
				} else {
					tCell.fH = 8.0f;
				}
				if ( tCell.fW <= 0.0f || tCell.fH <= 0.0f ) {
					continue;
				}
				__xgeXuiHostDrawRect(tCell, ((i + j) & 1) ? XGE_COLOR_RGBA(230, 236, 242, 255) : XGE_COLOR_RGBA(255, 255, 255, 255));
			}
		}
	}
	__xgeXuiHostDrawRect(tRect, iColor);
}

void xgeXuiColorPickerGetRGBA(xge_xui_color_picker pPicker, int* pR, int* pG, int* pB, int* pA)
{
	uint32_t iColor;

	iColor = (pPicker != NULL) ? pPicker->iColor : 0;
	if ( pR != NULL ) {
		*pR = (int)XGE_COLOR_GET_R(iColor);
	}
	if ( pG != NULL ) {
		*pG = (int)XGE_COLOR_GET_G(iColor);
	}
	if ( pB != NULL ) {
		*pB = (int)XGE_COLOR_GET_B(iColor);
	}
	if ( pA != NULL ) {
		*pA = (int)XGE_COLOR_GET_A(iColor);
	}
}

void xgeXuiColorPickerSetAlphaEnabled(xge_xui_color_picker pPicker, int bEnabled)
{
	if ( pPicker == NULL ) {
		return;
	}
	bEnabled = bEnabled ? 1 : 0;
	if ( pPicker->bAlphaEnabled == bEnabled ) {
		return;
	}
	pPicker->bAlphaEnabled = bEnabled;
	if ( bEnabled == 0 ) {
		pPicker->iColor = __xgeXuiColorPickerOpaque(pPicker->iColor);
	}
	__xgeXuiColorPickerFormatHex(pPicker);
	__xgeXuiColorPickerSyncSelectedPalette(pPicker);
	__xgeXuiColorPickerMarkPaint(pPicker);
}

int xgeXuiColorPickerGetAlphaEnabled(xge_xui_color_picker pPicker)
{
	return (pPicker != NULL) ? pPicker->bAlphaEnabled : 0;
}

int xgeXuiColorPickerSetHex(xge_xui_color_picker pPicker, const char* sHex)
{
	uint32_t iColor;

	if ( pPicker == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiColorPickerParseHex(sHex, &iColor) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiColorPickerSetColorInternal(pPicker, iColor, 0);
	return XGE_OK;
}

const char* xgeXuiColorPickerGetHex(xge_xui_color_picker pPicker)
{
	if ( pPicker == NULL ) {
		return "";
	}
	__xgeXuiColorPickerFormatHex(pPicker);
	return pPicker->sHex;
}

void xgeXuiColorPickerSetPalette(xge_xui_color_picker pPicker, const uint32_t* pColors, int iCount)
{
	int i;

	if ( pPicker == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	if ( iCount > XGE_XUI_COLOR_PICKER_PALETTE_CAPACITY ) {
		iCount = XGE_XUI_COLOR_PICKER_PALETTE_CAPACITY;
	}
	for ( i = 0; i < iCount; i++ ) {
		pPicker->arrPalette[i] = (pColors != NULL) ? pColors[i] : 0;
	}
	pPicker->iPaletteCount = iCount;
	__xgeXuiColorPickerSyncSelectedPalette(pPicker);
	__xgeXuiColorPickerLayoutPopup(pPicker);
	__xgeXuiColorPickerMarkPaint(pPicker);
}

int xgeXuiColorPickerGetPaletteCount(xge_xui_color_picker pPicker)
{
	return (pPicker != NULL) ? pPicker->iPaletteCount : 0;
}

void xgeXuiColorPickerSetColors(xge_xui_color_picker pPicker, uint32_t iBackground, uint32_t iPanel, uint32_t iBorder, uint32_t iText, uint32_t iAccent, uint32_t iField)
{
	if ( pPicker == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pPicker->pWidget, iBackground);
	xgeXuiWidgetSetBorder(pPicker->pWidget, 1.0f, iBorder);
	xgeXuiWidgetSetStateBorder(pPicker->pWidget, XGE_XUI_STATE_FOCUS, 1.0f, iAccent);
	xgeXuiPopupSetBackground(pPicker->pPopup, iPanel);
	xgeXuiPopupSetBorder(pPicker->pPopup, iBorder);
	pPicker->iPanelColor = iPanel;
	pPicker->iBorderColor = iBorder;
	pPicker->iTextColor = iText;
	pPicker->iAccentColor = iAccent;
	pPicker->iFieldColor = iField;
	__xgeXuiColorPickerMarkPaint(pPicker);
}

int xgeXuiColorPickerEvent(xge_xui_color_picker pPicker, const xge_event_t* pEvent)
{
	int iInside;

	if ( (pPicker == NULL) || (pPicker->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	__xgeXuiColorPickerLayoutMain(pPicker);
	if ( (pPicker->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pPicker->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		xgeXuiWidgetSetVisualState(pPicker->pWidget, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pPicker->pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
			xgeXuiWidgetSetVisualState(pPicker->pWidget, iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_ENTER:
			xgeXuiWidgetSetVisualState(pPicker->pWidget, XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			xgeXuiWidgetSetVisualState(pPicker->pWidget, XGE_XUI_STATE_NORMAL);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			xgeXuiWidgetMarkPaint(pPicker->pWidget);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pPicker->pContext, pPicker->pWidget);
			__xgeXuiColorPickerOpenPopup(pPicker, xgeXuiPopupIsOpen(pPicker->pPopup) == 0);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( (pPicker->pContext == NULL) || (pPicker->pContext->pFocus != pPicker->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE) || (pEvent->iParam1 == XGE_KEY_DOWN) ) {
				__xgeXuiColorPickerOpenPopup(pPicker, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiColorPickerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiColorPickerEvent((xge_xui_color_picker)pUser, pEvent);
}

int xgeXuiColorPickerPopupEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	xge_xui_color_picker pPicker;
	int i;
	int iRet;
	int iValue;

	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	pPicker = (xge_xui_color_picker)pUser;
	if ( (pPicker == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iRet = xgeXuiPopupEvent(pPicker->pPopup, pEvent);
	if ( iRet == XGE_XUI_EVENT_CONSUMED || xgeXuiPopupIsOpen(pPicker->pPopup) == 0 ) {
		return iRet;
	}
	__xgeXuiColorPickerLayoutPopup(pPicker);
	switch ( pEvent->iType ) {
		case XGE_EVENT_TEXT:
			if ( pPicker->iActiveField >= 0 ) {
				if ( __xgeXuiColorPickerTextCanAppend(pPicker, pEvent->iCodepoint) ) {
					xgeXuiTextInsertCodepoint(&pPicker->tEditText, pEvent->iCodepoint);
					pPicker->bEditError = 0;
					xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_KEY_DOWN:
			if ( pPicker->iActiveField >= 0 ) {
				if ( pEvent->iParam1 == XGE_KEY_ENTER ) {
					__xgeXuiColorPickerCommitEdit(pPicker, 1);
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( pEvent->iParam1 == XGE_KEY_ESCAPE ) {
					__xgeXuiColorPickerCancelEdit(pPicker);
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( ((pEvent->iParam2 & XGE_KEY_MOD_CTRL) != 0) && (pEvent->iParam1 == 'A' || pEvent->iParam1 == 'a') ) {
					xgeXuiTextSetSelection(&pPicker->tEditText, 0, pPicker->tEditText.iSize);
					xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( ((pEvent->iParam2 & XGE_KEY_MOD_CTRL) != 0) && (pEvent->iParam1 == 'C' || pEvent->iParam1 == 'c') ) {
					if ( pPicker->iActiveField == 4 ) {
						xgeClipboardSetText(((pPicker->tEditText.sText != NULL) && (pPicker->tEditText.sText[0] == '#')) ? (pPicker->tEditText.sText + 1) : ((pPicker->tEditText.sText != NULL) ? pPicker->tEditText.sText : ""));
					} else {
						xgeClipboardSetText((pPicker->tEditText.sText != NULL) ? pPicker->tEditText.sText : "");
					}
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( pEvent->iParam1 == XGE_KEY_BACKSPACE ) {
					xgeXuiTextDeleteBack(&pPicker->tEditText);
					pPicker->bEditError = 0;
					xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( pEvent->iParam1 == XGE_KEY_DELETE ) {
					xgeXuiTextDeleteForward(&pPicker->tEditText);
					pPicker->bEditError = 0;
					xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
				iValue = __xgeXuiRectContains(pPicker->tHexCopyRect, pEvent->fX, pEvent->fY) ? 1 : 0;
				if ( pPicker->bCopyHover != iValue ) {
					pPicker->bCopyHover = iValue;
					xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
				}
			}
			if ( pPicker->iActiveArea == XGE_XUI_COLOR_PICKER_AREA_SV ) {
				__xgeXuiColorPickerPickSv(pPicker, pEvent->fX, pEvent->fY, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pPicker->iActiveArea == XGE_XUI_COLOR_PICKER_AREA_HUE ) {
				__xgeXuiColorPickerPickHue(pPicker, pEvent->fY, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pPicker->iActiveArea >= XGE_XUI_COLOR_PICKER_AREA_RGB_SLIDER ) {
				__xgeXuiColorPickerPickRgbSlider(pPicker, pPicker->iActiveArea - XGE_XUI_COLOR_PICKER_AREA_RGB_SLIDER, pEvent->fX, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( !__xgeXuiRectContains(pPicker->pPopupWidget->tRect, pEvent->fX, pEvent->fY) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pPicker->pContext, pPicker->pPopupWidget);
			if ( pPicker->iActiveField >= 0 ) {
				__xgeXuiColorPickerCommitEdit(pPicker, 1);
			}
			if ( __xgeXuiRectContains(pPicker->tSvRect, pEvent->fX, pEvent->fY) ) {
				pPicker->bCopyActive = 0;
				pPicker->iActiveArea = XGE_XUI_COLOR_PICKER_AREA_SV;
				xgeXuiSetPointerCapture(pPicker->pContext, pEvent->iPointerId, pPicker->pPopupWidget);
				__xgeXuiColorPickerPickSv(pPicker, pEvent->fX, pEvent->fY, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( __xgeXuiRectContains(pPicker->tHueRect, pEvent->fX, pEvent->fY) ) {
				pPicker->bCopyActive = 0;
				pPicker->iActiveArea = XGE_XUI_COLOR_PICKER_AREA_HUE;
				xgeXuiSetPointerCapture(pPicker->pContext, pEvent->iPointerId, pPicker->pPopupWidget);
				__xgeXuiColorPickerPickHue(pPicker, pEvent->fY, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			for ( i = 0; i < (pPicker->bAlphaEnabled ? 4 : 3); i++ ) {
				if ( __xgeXuiRectContains(pPicker->arrFieldRect[i], pEvent->fX, pEvent->fY) ) {
					pPicker->bCopyActive = 0;
					__xgeXuiColorPickerBeginEdit(pPicker, i);
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( __xgeXuiRectContains(pPicker->arrSliderRect[i], pEvent->fX, pEvent->fY) ) {
					pPicker->bCopyActive = 0;
					pPicker->iActiveArea = XGE_XUI_COLOR_PICKER_AREA_RGB_SLIDER + i;
					xgeXuiSetPointerCapture(pPicker->pContext, pEvent->iPointerId, pPicker->pPopupWidget);
					__xgeXuiColorPickerPickRgbSlider(pPicker, i, pEvent->fX, 1);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			if ( __xgeXuiRectContains(pPicker->tHexRect, pEvent->fX, pEvent->fY) ) {
				pPicker->bCopyActive = 0;
				__xgeXuiColorPickerBeginEdit(pPicker, 4);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( __xgeXuiRectContains(pPicker->tHexCopyRect, pEvent->fX, pEvent->fY) ) {
				pPicker->bCopyHover = 1;
				pPicker->bCopyActive = 1;
				xgeClipboardSetText(xgeXuiColorPickerGetHex(pPicker) + 1);
				xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( __xgeXuiRectContains(pPicker->tOldRect, pEvent->fX, pEvent->fY) ) {
				pPicker->bCopyActive = 0;
				__xgeXuiColorPickerSetColorInternal(pPicker, pPicker->iOldColor, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			for ( i = 0; i < pPicker->iPaletteCount; i++ ) {
				if ( __xgeXuiRectContains(pPicker->arrPaletteRect[i], pEvent->fX, pEvent->fY) ) {
					pPicker->iSelectedPalette = i;
					__xgeXuiColorPickerSetColorInternal(pPicker, pPicker->arrPalette[i], 1);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pPicker->iActiveArea = XGE_XUI_COLOR_PICKER_AREA_NONE;
			if ( pPicker->bCopyActive != 0 ) {
				pPicker->bCopyActive = 0;
				xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
			}
			if ( pPicker->pContext != NULL && xgeXuiGetPointerCapture(pPicker->pContext, pEvent->iPointerId) == pPicker->pPopupWidget ) {
				xgeXuiSetPointerCapture(pPicker->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_WHEEL:
			if ( __xgeXuiRectContains(pPicker->tHueRect, pEvent->fX, pEvent->fY) ) {
				__xgeXuiColorPickerSetHsvInternal(pPicker, pPicker->fHue - (float)pEvent->iParam1 * 0.01f, pPicker->fSaturation, pPicker->fValue, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			for ( i = 0; i < (pPicker->bAlphaEnabled ? 4 : 3); i++ ) {
				if ( __xgeXuiRectContains(pPicker->arrFieldRect[i], pEvent->fX, pEvent->fY) ) {
					iValue = (i == 0) ? (int)XGE_COLOR_GET_R(pPicker->iColor) : ((i == 1) ? (int)XGE_COLOR_GET_G(pPicker->iColor) : ((i == 2) ? (int)XGE_COLOR_GET_B(pPicker->iColor) : (int)XGE_COLOR_GET_A(pPicker->iColor)));
					iValue = __xgeXuiColorPickerClampByte(iValue + pEvent->iParam1);
					if ( i == 0 ) {
						__xgeXuiColorPickerSetColorInternal(pPicker, XGE_COLOR_RGBA(iValue, XGE_COLOR_GET_G(pPicker->iColor), XGE_COLOR_GET_B(pPicker->iColor), XGE_COLOR_GET_A(pPicker->iColor)), 1);
					} else if ( i == 1 ) {
						__xgeXuiColorPickerSetColorInternal(pPicker, XGE_COLOR_RGBA(XGE_COLOR_GET_R(pPicker->iColor), iValue, XGE_COLOR_GET_B(pPicker->iColor), XGE_COLOR_GET_A(pPicker->iColor)), 1);
					} else if ( i == 2 ) {
						__xgeXuiColorPickerSetColorInternal(pPicker, XGE_COLOR_RGBA(XGE_COLOR_GET_R(pPicker->iColor), XGE_COLOR_GET_G(pPicker->iColor), iValue, XGE_COLOR_GET_A(pPicker->iColor)), 1);
					} else {
						__xgeXuiColorPickerSetColorInternal(pPicker, XGE_COLOR_RGBA(XGE_COLOR_GET_R(pPicker->iColor), XGE_COLOR_GET_G(pPicker->iColor), XGE_COLOR_GET_B(pPicker->iColor), iValue), 1);
					}
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			return XGE_XUI_EVENT_CONTINUE;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

void xgeXuiColorPickerPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_color_picker pPicker;
	uint32_t iText;
	uint32_t iArrow;

	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	pPicker = (xge_xui_color_picker)pUser;
	if ( (pWidget == NULL) || (pPicker == NULL) ) {
		return;
	}
	__xgeXuiColorPickerLayoutMain(pPicker);
	__xgeXuiColorPickerDrawSwatch(pPicker->tSwatchRect, pPicker->iColor);
	if ( pPicker->pFont != NULL ) {
		iText = __xgeXuiColorPickerTextOnColor(pPicker->iColor);
		__xgeXuiHostDrawTextRect(pPicker->pFont, xgeXuiColorPickerGetHex(pPicker), pPicker->tSwatchRect, iText, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	iArrow = ((pWidget->iVisualState & XGE_XUI_STATE_DISABLED) != 0) ? XGE_COLOR_RGBA(142, 152, 166, 210) : XGE_COLOR_RGBA(45, 92, 132, 255);
	__xgeXuiColorPickerDrawChevron(pPicker->tButtonRect, iArrow, xgeXuiPopupIsOpen(pPicker->pPopup));
}

void xgeXuiColorPickerPopupPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_color_picker pPicker;
	xge_rect_t tRect;
	xge_rect_t tLabel;
	xge_rect_t tIndicator;
	xge_rect_t tLine;
	xge_rect_t tTrack;
	xge_rect_t tKnob;
	xge_vec2_t arrTri[3];
	char sText[32];
	const char* arrNames[4] = { "R", "G", "B", "A" };
	int arrRgb[4];
	int iFieldCount;
	int i;
	float fX;
	float fY;
	uint32_t iFieldBorder;

	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	pPicker = (xge_xui_color_picker)pUser;
	if ( (pWidget == NULL) || (pPicker == NULL) ) {
		return;
	}
	__xgeXuiColorPickerLayoutPopup(pPicker);
	__xgeXuiColorPickerDrawSv(pPicker);
	__xgeXuiHostDrawBorderRect(pPicker->tSvRect, 1.0f, XGE_COLOR_RGBA(110, 148, 180, 255));
	fX = pPicker->tSvRect.fX + pPicker->tSvRect.fW * pPicker->fSaturation;
	fY = pPicker->tSvRect.fY + pPicker->tSvRect.fH * (1.0f - pPicker->fValue);
	xgeShapeCircleStrokePx(fX, fY, 8.0f, 2.0f, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeShapeCircleStrokePx(fX, fY, 9.0f, 1.0f, XGE_COLOR_RGBA(32, 44, 56, 210));
	__xgeXuiColorPickerDrawHue(pPicker->tHueRect);
	__xgeXuiHostDrawBorderRect(pPicker->tHueRect, 1.0f, XGE_COLOR_RGBA(110, 148, 180, 255));
	fY = pPicker->tHueRect.fY + pPicker->tHueRect.fH * pPicker->fHue;
	arrTri[0] = (xge_vec2_t){ pPicker->tHueRect.fX - 9.0f, fY - 6.0f };
	arrTri[1] = (xge_vec2_t){ pPicker->tHueRect.fX - 1.0f, fY };
	arrTri[2] = (xge_vec2_t){ pPicker->tHueRect.fX - 9.0f, fY + 6.0f };
	xgeShapeTriangleFillPx(arrTri[0], arrTri[1], arrTri[2], XGE_COLOR_RGBA(255, 255, 255, 255));
	tLine = (xge_rect_t){ pPicker->tHueRect.fX - 1.0f, fY - 1.0f, pPicker->tHueRect.fW + 10.0f, 2.0f };
	__xgeXuiHostDrawRect(tLine, XGE_COLOR_RGBA(255, 255, 255, 255));
	if ( pPicker->pFont != NULL ) {
		tLabel = (xge_rect_t){ pPicker->tOldRect.fX, pPicker->tOldRect.fY - 18.0f, pPicker->tOldRect.fW, 14.0f };
		__xgeXuiHostDrawTextRect(pPicker->pFont, "Old", tLabel, pPicker->iMutedTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		tLabel.fX = pPicker->tNewRect.fX;
		__xgeXuiHostDrawTextRect(pPicker->pFont, "New", tLabel, pPicker->iMutedTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	__xgeXuiColorPickerDrawSwatch(pPicker->tOldRect, pPicker->iOldColor);
	__xgeXuiHostDrawBorderRect(pPicker->tOldRect, 1.0f, XGE_COLOR_RGBA(95, 142, 176, 255));
	__xgeXuiColorPickerDrawSwatch(pPicker->tNewRect, pPicker->iColor);
	__xgeXuiHostDrawBorderRect(pPicker->tNewRect, 1.0f, XGE_COLOR_RGBA(95, 142, 176, 255));
	arrRgb[0] = (int)XGE_COLOR_GET_R(pPicker->iColor);
	arrRgb[1] = (int)XGE_COLOR_GET_G(pPicker->iColor);
	arrRgb[2] = (int)XGE_COLOR_GET_B(pPicker->iColor);
	arrRgb[3] = (int)XGE_COLOR_GET_A(pPicker->iColor);
	iFieldCount = pPicker->bAlphaEnabled ? 4 : 3;
	for ( i = 0; i < iFieldCount; i++ ) {
		if ( pPicker->pFont != NULL ) {
			tLabel = pPicker->arrFieldRect[i];
			tLabel.fX -= 30.0f;
			tLabel.fW = 24.0f;
			__xgeXuiHostDrawTextRect(pPicker->pFont, arrNames[i], tLabel, pPicker->iMutedTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		__xgeXuiHostDrawRect(pPicker->arrFieldRect[i], XGE_COLOR_RGBA(25, 38, 50, 255));
		iFieldBorder = (pPicker->iActiveField == i && pPicker->bEditError) ? XGE_COLOR_RGBA(224, 80, 80, 255) : ((pPicker->iActiveField == i) ? pPicker->iAccentColor : XGE_COLOR_RGBA(86, 120, 150, 255));
		__xgeXuiHostDrawBorderRect(pPicker->arrFieldRect[i], 1.0f, iFieldBorder);
		if ( pPicker->pFont != NULL ) {
			if ( pPicker->iActiveField == i ) {
				snprintf(sText, sizeof(sText), "%s", (pPicker->tEditText.sText != NULL) ? pPicker->tEditText.sText : "");
			} else {
				snprintf(sText, sizeof(sText), "%d", arrRgb[i]);
			}
			sText[sizeof(sText) - 1] = 0;
			__xgeXuiColorPickerDrawEditText(pPicker, pPicker->arrFieldRect[i], sText, pPicker->iActiveField == i);
		}
		tTrack = pPicker->arrSliderRect[i];
		__xgeXuiHostDrawRect(tTrack, XGE_COLOR_RGBA(76, 98, 118, 255));
		tIndicator = tTrack;
		tIndicator.fW = tTrack.fW * ((float)arrRgb[i] / 255.0f);
		__xgeXuiHostDrawRect(tIndicator, pPicker->iAccentColor);
		tKnob = (xge_rect_t){ tTrack.fX + tTrack.fW * ((float)arrRgb[i] / 255.0f) - 7.0f, tTrack.fY + tTrack.fH * 0.5f - 7.0f, 14.0f, 14.0f };
		xgeShapeCircleFillPx(tKnob.fX + 7.0f, tKnob.fY + 7.0f, 7.0f, XGE_COLOR_RGBA(246, 250, 253, 255));
		xgeShapeCircleStrokePx(tKnob.fX + 7.0f, tKnob.fY + 7.0f, 7.0f, 1.0f, XGE_COLOR_RGBA(38, 70, 94, 180));
	}
	if ( pPicker->pFont != NULL ) {
		tLabel = pPicker->tHexRect;
		tLabel.fX -= 30.0f;
		tLabel.fW = 24.0f;
		__xgeXuiHostDrawTextRect(pPicker->pFont, "#", tLabel, pPicker->iMutedTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	__xgeXuiHostDrawRect(pPicker->tHexRect, XGE_COLOR_RGBA(25, 38, 50, 255));
	iFieldBorder = (pPicker->iActiveField == 4 && pPicker->bEditError) ? XGE_COLOR_RGBA(224, 80, 80, 255) : ((pPicker->iActiveField == 4) ? pPicker->iAccentColor : XGE_COLOR_RGBA(86, 120, 150, 255));
	__xgeXuiHostDrawBorderRect(pPicker->tHexRect, 1.0f, iFieldBorder);
	if ( pPicker->pFont != NULL ) {
		__xgeXuiColorPickerDrawEditText(pPicker, pPicker->tHexRect, (pPicker->iActiveField == 4 && pPicker->tEditText.sText != NULL) ? pPicker->tEditText.sText : (xgeXuiColorPickerGetHex(pPicker) + 1), pPicker->iActiveField == 4);
	}
	__xgeXuiHostDrawRect(pPicker->tHexCopyRect, pPicker->bCopyActive ? XGE_COLOR_RGBA(42, 96, 136, 255) : (pPicker->bCopyHover ? XGE_COLOR_RGBA(38, 70, 92, 255) : XGE_COLOR_RGBA(33, 50, 64, 255)));
	__xgeXuiHostDrawBorderRect(pPicker->tHexCopyRect, 1.0f, pPicker->bCopyActive ? pPicker->iAccentColor : (pPicker->bCopyHover ? XGE_COLOR_RGBA(120, 166, 198, 255) : XGE_COLOR_RGBA(86, 120, 150, 255)));
	__xgeXuiColorPickerDrawCopyIcon(pPicker->tHexCopyRect, pPicker->bCopyActive ? XGE_COLOR_RGBA(255, 255, 255, 255) : XGE_COLOR_RGBA(205, 224, 238, 255));
	tRect = (xge_rect_t){ pWidget->tContentRect.fX + 14.0f, pPicker->arrPaletteRect[0].fY - 20.0f, pWidget->tContentRect.fW - 28.0f, 1.0f };
	__xgeXuiHostDrawRect(tRect, XGE_COLOR_RGBA(74, 98, 118, 255));
	for ( i = 0; i < pPicker->iPaletteCount; i++ ) {
		__xgeXuiColorPickerDrawSwatch(pPicker->arrPaletteRect[i], pPicker->arrPalette[i]);
		__xgeXuiHostDrawBorderRect(pPicker->arrPaletteRect[i], (i == pPicker->iSelectedPalette) ? 2.0f : 1.0f, (i == pPicker->iSelectedPalette) ? pPicker->iAccentColor : XGE_COLOR_RGBA(96, 132, 162, 255));
	}
}
