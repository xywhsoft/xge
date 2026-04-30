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

static void __xgeXuiColorPickerFormatHex(xge_xui_color_picker pPicker)
{
	if ( pPicker == NULL ) {
		return;
	}
	snprintf(
		pPicker->sHex,
		sizeof(pPicker->sHex),
		"#%02X%02X%02X%02X",
		(unsigned int)XGE_COLOR_GET_R(pPicker->iColor),
		(unsigned int)XGE_COLOR_GET_G(pPicker->iColor),
		(unsigned int)XGE_COLOR_GET_B(pPicker->iColor),
		(unsigned int)XGE_COLOR_GET_A(pPicker->iColor));
	pPicker->sHex[sizeof(pPicker->sHex) - 1] = 0;
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

static void __xgeXuiColorPickerLayout(xge_xui_color_picker pPicker)
{
	xge_rect_t tRect;
	float fX;
	float fY;
	float fCell;
	float fGap;
	int i;

	if ( (pPicker == NULL) || (pPicker->pWidget == NULL) ) {
		return;
	}
	tRect = pPicker->pWidget->tContentRect;
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		tRect = pPicker->pWidget->tRect;
	}
	pPicker->tSwatchRect = (xge_rect_t){ tRect.fX + 8.0f, tRect.fY + 8.0f, 52.0f, 40.0f };
	pPicker->tHexRect = (xge_rect_t){ tRect.fX + 68.0f, tRect.fY + 8.0f, tRect.fW - 76.0f, 24.0f };
	for ( i = 0; i < 4; i++ ) {
		pPicker->arrFieldRect[i] = (xge_rect_t){ tRect.fX + 68.0f + (float)i * 44.0f, tRect.fY + 38.0f, 38.0f, 22.0f };
	}
	fCell = 22.0f;
	fGap = 6.0f;
	fX = tRect.fX + 8.0f;
	fY = tRect.fY + 72.0f;
	for ( i = 0; i < pPicker->iPaletteCount; i++ ) {
		pPicker->arrPaletteRect[i] = (xge_rect_t){ fX + (float)(i % 8) * (fCell + fGap), fY + (float)(i / 8) * (fCell + fGap), fCell, fCell };
	}
}

static void __xgeXuiColorPickerSetColorInternal(xge_xui_color_picker pPicker, uint32_t iColor, int bNotify)
{
	if ( pPicker == NULL ) {
		return;
	}
	if ( pPicker->iColor == iColor ) {
		__xgeXuiColorPickerFormatHex(pPicker);
		return;
	}
	pPicker->iColor = iColor;
	__xgeXuiColorPickerFormatHex(pPicker);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
	if ( bNotify ) {
		__xgeXuiColorPickerNotify(pPicker);
	}
}

int xgeXuiColorPickerInit(xge_xui_color_picker pPicker, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	const xge_xui_theme_t* pTheme;
	static const uint32_t arrDefaultPalette[8] = {
		XGE_COLOR_RGBA(255, 255, 255, 255),
		XGE_COLOR_RGBA(34, 48, 64, 255),
		XGE_COLOR_RGBA(78, 159, 220, 255),
		XGE_COLOR_RGBA(43, 184, 150, 255),
		XGE_COLOR_RGBA(244, 187, 68, 255),
		XGE_COLOR_RGBA(224, 92, 92, 255),
		XGE_COLOR_RGBA(142, 116, 220, 255),
		XGE_COLOR_RGBA(96, 126, 148, 255)
	};

	if ( (pPicker == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pPicker, 0, sizeof(*pPicker));
	pTheme = xgeXuiGetTheme(pContext);
	pPicker->pContext = pContext;
	pPicker->pWidget = pWidget;
	pPicker->pFont = (pFont != NULL) ? pFont : pTheme->pFont;
	pPicker->iHoverPalette = -1;
	pPicker->iSelectedPalette = -1;
	pPicker->iActiveField = -1;
	pPicker->iBackgroundColor = XGE_COLOR_RGBA(236, 246, 253, 255);
	pPicker->iPanelColor = XGE_COLOR_RGBA(247, 252, 255, 255);
	pPicker->iBorderColor = XGE_COLOR_RGBA(129, 174, 207, 255);
	pPicker->iTextColor = XGE_COLOR_RGBA(31, 58, 82, 255);
	pPicker->iAccentColor = XGE_COLOR_RGBA(53, 154, 214, 255);
	pPicker->iFieldColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pPicker->iHoverColor = XGE_COLOR_RGBA(222, 239, 254, 255);
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetClip(pWidget, 1);
	pWidget->procEvent = xgeXuiColorPickerEventProc;
	pWidget->procPaint = xgeXuiColorPickerPaintProc;
	pWidget->pUser = pPicker;
	xgeXuiColorPickerSetPalette(pPicker, arrDefaultPalette, 8);
	__xgeXuiColorPickerSetColorInternal(pPicker, XGE_COLOR_RGBA(78, 159, 220, 255), 0);
	__xgeXuiColorPickerLayout(pPicker);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiColorPickerUnit(xge_xui_color_picker pPicker)
{
	if ( pPicker == NULL ) {
		return;
	}
	if ( pPicker->pWidget != NULL && pPicker->pWidget->pUser == pPicker ) {
		pPicker->pWidget->pUser = NULL;
		pPicker->pWidget->procEvent = NULL;
		pPicker->pWidget->procPaint = NULL;
	}
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
	__xgeXuiColorPickerLayout(pPicker);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
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
	pPicker->iBackgroundColor = iBackground;
	pPicker->iPanelColor = iPanel;
	pPicker->iBorderColor = iBorder;
	pPicker->iTextColor = iText;
	pPicker->iAccentColor = iAccent;
	pPicker->iFieldColor = iField;
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

int xgeXuiColorPickerEvent(xge_xui_color_picker pPicker, const xge_event_t* pEvent)
{
	int i;
	int iHover;

	if ( (pPicker == NULL) || (pPicker->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	__xgeXuiColorPickerLayout(pPicker);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
			iHover = -1;
			for ( i = 0; i < pPicker->iPaletteCount; i++ ) {
				if ( __xgeXuiRectContains(pPicker->arrPaletteRect[i], pEvent->fX, pEvent->fY) ) {
					iHover = i;
					break;
				}
			}
			if ( pPicker->iHoverPalette != iHover ) {
				pPicker->iHoverPalette = iHover;
				xgeXuiWidgetMarkPaint(pPicker->pWidget);
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
			if ( !__xgeXuiRectContains(pPicker->pWidget->tRect, pEvent->fX, pEvent->fY) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pPicker->pContext, pPicker->pWidget);
			for ( i = 0; i < pPicker->iPaletteCount; i++ ) {
				if ( __xgeXuiRectContains(pPicker->arrPaletteRect[i], pEvent->fX, pEvent->fY) ) {
					pPicker->iSelectedPalette = i;
					__xgeXuiColorPickerSetColorInternal(pPicker, pPicker->arrPalette[i], 1);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			pPicker->iActiveField = -1;
			for ( i = 0; i < 4; i++ ) {
				if ( __xgeXuiRectContains(pPicker->arrFieldRect[i], pEvent->fX, pEvent->fY) ) {
					pPicker->iActiveField = i;
					xgeXuiWidgetMarkPaint(pPicker->pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			if ( __xgeXuiRectContains(pPicker->tHexRect, pEvent->fX, pEvent->fY) ) {
				pPicker->iActiveField = 4;
				xgeXuiWidgetMarkPaint(pPicker->pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONSUMED;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiColorPickerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiColorPickerEvent((xge_xui_color_picker)pUser, pEvent);
}

void xgeXuiColorPickerPaintProc(xge_xui_widget pWidget, void* pUser)
{
	static const char* arrNames[4] = { "R", "G", "B", "A" };
	xge_xui_color_picker pPicker;
	xge_rect_t tText;
	char sText[32];
	int arrRGBA[4];
	int i;
	uint32_t iBorder;

	pPicker = (xge_xui_color_picker)pUser;
	if ( (pWidget == NULL) || (pPicker == NULL) ) {
		return;
	}
	__xgeXuiColorPickerLayout(pPicker);
	__xgeXuiHostDrawRect(pWidget->tRect, pPicker->iBackgroundColor);
	__xgeXuiHostDrawBorderRect(pWidget->tRect, 1.0f, pPicker->iBorderColor);
	__xgeXuiHostDrawRect(pPicker->tSwatchRect, pPicker->iColor);
	__xgeXuiHostDrawBorderRect(pPicker->tSwatchRect, 1.5f, pPicker->iBorderColor);
	__xgeXuiHostDrawRect(pPicker->tHexRect, pPicker->iFieldColor);
	__xgeXuiHostDrawBorderRect(pPicker->tHexRect, (pPicker->iActiveField == 4) ? 1.5f : 1.0f, (pPicker->iActiveField == 4) ? pPicker->iAccentColor : pPicker->iBorderColor);
	if ( pPicker->pFont != NULL ) {
		__xgeXuiHostDrawTextRect(pPicker->pFont, xgeXuiColorPickerGetHex(pPicker), pPicker->tHexRect, pPicker->iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	xgeXuiColorPickerGetRGBA(pPicker, &arrRGBA[0], &arrRGBA[1], &arrRGBA[2], &arrRGBA[3]);
	for ( i = 0; i < 4; i++ ) {
		__xgeXuiHostDrawRect(pPicker->arrFieldRect[i], pPicker->iFieldColor);
		__xgeXuiHostDrawBorderRect(pPicker->arrFieldRect[i], (pPicker->iActiveField == i) ? 1.5f : 1.0f, (pPicker->iActiveField == i) ? pPicker->iAccentColor : pPicker->iBorderColor);
		if ( pPicker->pFont != NULL ) {
			snprintf(sText, sizeof(sText), "%s%d", arrNames[i], arrRGBA[i]);
			sText[sizeof(sText) - 1] = 0;
			__xgeXuiHostDrawTextRect(pPicker->pFont, sText, pPicker->arrFieldRect[i], pPicker->iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	for ( i = 0; i < pPicker->iPaletteCount; i++ ) {
		__xgeXuiHostDrawRect(pPicker->arrPaletteRect[i], pPicker->arrPalette[i]);
		iBorder = (i == pPicker->iSelectedPalette || i == pPicker->iHoverPalette) ? pPicker->iAccentColor : pPicker->iBorderColor;
		__xgeXuiHostDrawBorderRect(pPicker->arrPaletteRect[i], (i == pPicker->iSelectedPalette) ? 2.0f : 1.0f, iBorder);
	}
	if ( pPicker->pFont != NULL ) {
		tText = pWidget->tRect;
		tText.fX += 8.0f;
		tText.fY = pPicker->tSwatchRect.fY + pPicker->tSwatchRect.fH + 8.0f;
		tText.fW -= 16.0f;
		tText.fH = 14.0f;
		__xgeXuiHostDrawTextRect(pPicker->pFont, "Palette", tText, pPicker->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
}
