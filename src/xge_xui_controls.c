int xgeXuiButtonInit(xge_xui_button pButton, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pButton == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pButton, 0, sizeof(*pButton));
	pTheme = xgeXuiGetTheme(pContext);
	pButton->pContext = pContext;
	pButton->pWidget = pWidget;
	pButton->pFont = pTheme->pFont;
	pButton->sText = "";
	pButton->iTextColor = pTheme->iTextColor;
	pButton->iTextFlags = XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pButton->iColorNormal = pTheme->iStateNormal;
	pButton->iColorHover = pTheme->iStateHover;
	pButton->iColorActive = pTheme->iStateActive;
	pButton->iColorFocus = pTheme->iStateFocus;
	pButton->iColorDisabled = pTheme->iStateDisabled;
	pWidget->tStyle.fRadius = pTheme->fRadius;
	xgeXuiWidgetSetFocusable(pWidget, 1);
	pWidget->procEvent = xgeXuiButtonEventProc;
	pWidget->procPaint = xgeXuiButtonPaintProc;
	pWidget->pUser = pButton;
	__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiButtonUnit(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return;
	}
	if ( pButton->pWidget != NULL && pButton->pWidget->pUser == pButton ) {
		pButton->pWidget->pUser = NULL;
		pButton->pWidget->procEvent = NULL;
		pButton->pWidget->procPaint = NULL;
	}
	memset(pButton, 0, sizeof(*pButton));
}

void xgeXuiButtonSetClick(xge_xui_button pButton, xge_xui_click_proc procClick, void* pUser)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->procClick = procClick;
	pButton->pUser = pUser;
}

void xgeXuiButtonSetText(xge_xui_button pButton, xge_font pFont, const char* sText)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->pFont = pFont;
	pButton->sText = (sText != NULL) ? sText : "";
	xgeXuiWidgetMarkLayout(pButton->pWidget);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetTextColor(xge_xui_button pButton, uint32_t iColor)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->iTextColor = iColor;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetColors(xge_xui_button pButton, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->iColorNormal = iNormal;
	pButton->iColorHover = iHover;
	pButton->iColorActive = iActive;
	pButton->iColorFocus = iFocus;
	pButton->iColorDisabled = iDisabled;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

int xgeXuiButtonGetState(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiButtonSetState(pButton, pButton->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pButton->iState;
}

int xgeXuiButtonEvent(xge_xui_button pButton, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;

	if ( (pButton == NULL) || (pButton->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pButton->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pButton->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pButton->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pButton->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);

	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( iInside ) {
				iState |= XGE_XUI_STATE_HOVER;
			} else {
				iState &= ~XGE_XUI_STATE_HOVER;
			}
			__xgeXuiButtonSetState(pButton, iState);
			return ((iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pButton->pContext, pButton->pWidget);
			xgeXuiSetCapture(pButton->pContext, pButton->pWidget);
			__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pButton->iState & XGE_XUI_STATE_ACTIVE) != 0);
			iState = iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
			__xgeXuiButtonSetState(pButton, iState);
			if ( pButton->pContext != NULL && pButton->pContext->pCapture == pButton->pWidget ) {
				xgeXuiSetCapture(pButton->pContext, NULL);
			}
			if ( bWasActive && iInside ) {
				pButton->iClickCount++;
				if ( pButton->procClick != NULL ) {
					pButton->procClick(pButton->pWidget, pButton->pUser);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
			__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_NORMAL);
			if ( pButton->pContext != NULL && pButton->pContext->pCapture == pButton->pWidget ) {
				xgeXuiSetCapture(pButton->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiButtonEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiButtonEvent((xge_xui_button)pUser, pEvent);
}

void xgeXuiButtonPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_button pButton;
	uint32_t iColor;

	pButton = (xge_xui_button)pUser;
	if ( (pWidget == NULL) || (pButton == NULL) ) {
		return;
	}
	iColor = __xgeXuiButtonColor(pButton);
	if ( XGE_COLOR_GET_A(iColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, iColor);
	}
	if ( (pButton->pFont != NULL) && (pButton->sText != NULL) && (pButton->sText[0] != 0) ) {
		__xgeXuiHostDrawTextRect(pButton->pFont, pButton->sText, pWidget->tContentRect, pButton->iTextColor, pButton->iTextFlags);
	}
}

int xgeXuiLabelInit(xge_xui_label pLabel, xge_xui_widget pWidget, xge_font pFont, const char* sText)
{
	if ( (pLabel == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pLabel, 0, sizeof(*pLabel));
	pLabel->pWidget = pWidget;
	pLabel->pFont = pFont;
	pLabel->sText = (sText != NULL) ? sText : "";
	pLabel->iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pLabel->iTextFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP | XGE_TEXT_CLIP;
	pWidget->procMeasure = xgeXuiLabelMeasureProc;
	pWidget->procPaint = xgeXuiLabelPaintProc;
	pWidget->pUser = pLabel;
	pLabel->tMeasuredSize = xgeXuiLabelMeasure(pLabel);
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiLabelUnit(xge_xui_label pLabel)
{
	if ( pLabel == NULL ) {
		return;
	}
	if ( pLabel->pWidget != NULL && pLabel->pWidget->pUser == pLabel ) {
		pLabel->pWidget->pUser = NULL;
		pLabel->pWidget->procMeasure = NULL;
		pLabel->pWidget->procPaint = NULL;
	}
	memset(pLabel, 0, sizeof(*pLabel));
}

void xgeXuiLabelSetText(xge_xui_label pLabel, const char* sText)
{
	if ( pLabel == NULL ) {
		return;
	}
	pLabel->sText = (sText != NULL) ? sText : "";
	pLabel->tMeasuredSize = xgeXuiLabelMeasure(pLabel);
	xgeXuiWidgetMarkLayout(pLabel->pWidget);
	xgeXuiWidgetMarkPaint(pLabel->pWidget);
}

void xgeXuiLabelSetFont(xge_xui_label pLabel, xge_font pFont)
{
	if ( pLabel == NULL ) {
		return;
	}
	pLabel->pFont = pFont;
	pLabel->tMeasuredSize = xgeXuiLabelMeasure(pLabel);
	xgeXuiWidgetMarkLayout(pLabel->pWidget);
	xgeXuiWidgetMarkPaint(pLabel->pWidget);
}

void xgeXuiLabelSetColor(xge_xui_label pLabel, uint32_t iColor)
{
	if ( pLabel == NULL ) {
		return;
	}
	pLabel->iColor = iColor;
	xgeXuiWidgetMarkPaint(pLabel->pWidget);
}

void xgeXuiLabelSetAlign(xge_xui_label pLabel, uint32_t iTextFlags)
{
	if ( pLabel == NULL ) {
		return;
	}
	pLabel->iTextFlags = iTextFlags | XGE_TEXT_CLIP;
	xgeXuiWidgetMarkPaint(pLabel->pWidget);
}

xge_vec2_t xgeXuiLabelMeasure(xge_xui_label pLabel)
{
	xge_vec2_t tSize;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( (pLabel == NULL) || (pLabel->pFont == NULL) || (pLabel->sText == NULL) ) {
		return tSize;
	}
	tSize = __xgeXuiHostMeasureText(pLabel->pFont, pLabel->sText);
	return tSize;
}

xge_vec2_t xgeXuiLabelMeasureProc(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	return xgeXuiLabelMeasure((xge_xui_label)pUser);
}

void xgeXuiLabelPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_label pLabel;

	pLabel = (xge_xui_label)pUser;
	if ( (pWidget == NULL) || (pLabel == NULL) || (pLabel->pFont == NULL) || (pLabel->sText == NULL) ) {
		return;
	}
	__xgeXuiHostDrawTextRect(pLabel->pFont, pLabel->sText, pWidget->tContentRect, pLabel->iColor, pLabel->iTextFlags);
}

int xgeXuiImageInit(xge_xui_image pImage, xge_xui_widget pWidget, xge_texture pTexture)
{
	if ( (pImage == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pImage, 0, sizeof(*pImage));
	pImage->pWidget = pWidget;
	pImage->pTexture = pTexture;
	pImage->iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pImage->iMode = XGE_XUI_IMAGE_STRETCH;
	pWidget->procMeasure = xgeXuiImageMeasureProc;
	pWidget->procPaint = xgeXuiImagePaintProc;
	pWidget->pUser = pImage;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiImageUnit(xge_xui_image pImage)
{
	if ( pImage == NULL ) {
		return;
	}
	if ( pImage->pWidget != NULL && pImage->pWidget->pUser == pImage ) {
		pImage->pWidget->pUser = NULL;
		pImage->pWidget->procMeasure = NULL;
		pImage->pWidget->procPaint = NULL;
	}
	memset(pImage, 0, sizeof(*pImage));
}

void xgeXuiImageSetTexture(xge_xui_image pImage, xge_texture pTexture)
{
	if ( pImage == NULL ) {
		return;
	}
	pImage->pTexture = pTexture;
	xgeXuiWidgetMarkLayout(pImage->pWidget);
	xgeXuiWidgetMarkPaint(pImage->pWidget);
}

void xgeXuiImageSetSource(xge_xui_image pImage, xge_rect_t tSrc)
{
	if ( pImage == NULL ) {
		return;
	}
	pImage->tSrc = tSrc;
	xgeXuiWidgetMarkLayout(pImage->pWidget);
	xgeXuiWidgetMarkPaint(pImage->pWidget);
}

void xgeXuiImageSetColor(xge_xui_image pImage, uint32_t iColor)
{
	if ( pImage == NULL ) {
		return;
	}
	pImage->iColor = iColor;
	xgeXuiWidgetMarkPaint(pImage->pWidget);
}

void xgeXuiImageSetMode(xge_xui_image pImage, int iMode)
{
	if ( pImage == NULL ) {
		return;
	}
	pImage->iMode = iMode;
	xgeXuiWidgetMarkPaint(pImage->pWidget);
}

xge_vec2_t xgeXuiImageMeasureProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_image pImage;
	xge_vec2_t tSize;

	(void)pWidget;
	pImage = (xge_xui_image)pUser;
	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( pImage == NULL ) {
		return tSize;
	}
	if ( pImage->tSrc.fW > 0.0f && pImage->tSrc.fH > 0.0f ) {
		tSize.fX = pImage->tSrc.fW;
		tSize.fY = pImage->tSrc.fH;
		return tSize;
	}
	if ( pImage->pTexture != NULL ) {
		tSize.fX = (float)pImage->pTexture->iWidth;
		tSize.fY = (float)pImage->pTexture->iHeight;
	}
	return tSize;
}

static xge_rect_t __xgeXuiImageDestRect(xge_xui_image pImage, xge_rect_t tContent)
{
	xge_rect_t tDst;
	float fSrcW;
	float fSrcH;
	float fScale;

	tDst = tContent;
	if ( (pImage == NULL) || (pImage->pTexture == NULL) ) {
		return tDst;
	}
	fSrcW = (pImage->tSrc.fW > 0.0f) ? pImage->tSrc.fW : (float)pImage->pTexture->iWidth;
	fSrcH = (pImage->tSrc.fH > 0.0f) ? pImage->tSrc.fH : (float)pImage->pTexture->iHeight;
	if ( (fSrcW <= 0.0f) || (fSrcH <= 0.0f) ) {
		return tDst;
	}
	if ( pImage->iMode == XGE_XUI_IMAGE_CENTER ) {
		tDst.fW = fSrcW;
		tDst.fH = fSrcH;
		tDst.fX = tContent.fX + (tContent.fW - tDst.fW) * 0.5f;
		tDst.fY = tContent.fY + (tContent.fH - tDst.fH) * 0.5f;
	} else if ( pImage->iMode == XGE_XUI_IMAGE_FIT ) {
		fScale = tContent.fW / fSrcW;
		if ( (fSrcH * fScale) > tContent.fH ) {
			fScale = tContent.fH / fSrcH;
		}
		tDst.fW = fSrcW * fScale;
		tDst.fH = fSrcH * fScale;
		tDst.fX = tContent.fX + (tContent.fW - tDst.fW) * 0.5f;
		tDst.fY = tContent.fY + (tContent.fH - tDst.fH) * 0.5f;
	}
	return tDst;
}

void xgeXuiImagePaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_image pImage;
	xge_draw_t tDraw;

	pImage = (xge_xui_image)pUser;
	if ( (pWidget == NULL) || (pImage == NULL) || (pImage->pTexture == NULL) || (XGE_COLOR_GET_A(pImage->iColor) == 0) ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pImage->pTexture;
	tDraw.tSrc = pImage->tSrc;
	tDraw.tDst = __xgeXuiImageDestRect(pImage, pWidget->tContentRect);
	tDraw.iColor = pImage->iColor;
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	__xgeXuiHostDrawImage(&tDraw);
}

int xgeXuiInputInit(xge_xui_input pInput, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	const xge_xui_theme_t* pTheme;
	int iRet;

	if ( (pInput == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pInput, 0, sizeof(*pInput));
	iRet = xgeXuiTextInit(&pInput->tText);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pTheme = xgeXuiGetTheme(pContext);
	pInput->pContext = pContext;
	pInput->pWidget = pWidget;
	pInput->pFont = (pFont != NULL) ? pFont : pTheme->pFont;
	pInput->iTextColor = pTheme->iTextColor;
	pInput->iBackgroundColor = pTheme->iBackgroundColor;
	pInput->iFocusColor = pTheme->iStateFocus;
	pInput->iCursorColor = pTheme->iTextColor;
	pInput->iSelectionColor = pTheme->iSelectionColor;
	pInput->bInitialized = 1;
	pWidget->tStyle.fRadius = pTheme->fRadius;
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetClip(pWidget, 1);
	pWidget->procEvent = xgeXuiInputEventProc;
	pWidget->procPaint = xgeXuiInputPaintProc;
	pWidget->pUser = pInput;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiInputUnit(xge_xui_input pInput)
{
	if ( pInput == NULL ) {
		return;
	}
	if ( pInput->pWidget != NULL && pInput->pWidget->pUser == pInput ) {
		pInput->pWidget->pUser = NULL;
		pInput->pWidget->procEvent = NULL;
		pInput->pWidget->procPaint = NULL;
	}
	xgeXuiTextUnit(&pInput->tText);
	memset(pInput, 0, sizeof(*pInput));
}

void xgeXuiInputSetText(xge_xui_input pInput, const char* sText)
{
	if ( (pInput == NULL) || (pInput->bInitialized == 0) ) {
		return;
	}
	if ( xgeXuiTextSet(&pInput->tText, sText) == XGE_OK ) {
		xgeXuiWidgetMarkLayout(pInput->pWidget);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
	}
}

const char* xgeXuiInputGetText(xge_xui_input pInput)
{
	if ( (pInput == NULL) || (pInput->tText.sText == NULL) ) {
		return "";
	}
	return pInput->tText.sText;
}

void xgeXuiInputSetFont(xge_xui_input pInput, xge_font pFont)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->pFont = pFont;
	xgeXuiWidgetMarkLayout(pInput->pWidget);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputSetColors(xge_xui_input pInput, uint32_t iText, uint32_t iBackground, uint32_t iFocus, uint32_t iCursor)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->iTextColor = iText;
	pInput->iBackgroundColor = iBackground;
	pInput->iFocusColor = iFocus;
	pInput->iCursorColor = iCursor;
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputSetSelection(xge_xui_input pInput, int iStart, int iEnd)
{
	if ( (pInput == NULL) || (pInput->bInitialized == 0) ) {
		return;
	}
	xgeXuiTextSetSelection(&pInput->tText, iStart, iEnd);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputGetSelection(xge_xui_input pInput, int* pStart, int* pEnd)
{
	if ( pInput == NULL ) {
		if ( pStart != NULL ) {
			*pStart = 0;
		}
		if ( pEnd != NULL ) {
			*pEnd = 0;
		}
		return;
	}
	xgeXuiTextGetSelection(&pInput->tText, pStart, pEnd);
}

xge_rect_t xgeXuiInputGetCandidateRect(xge_xui_input pInput)
{
	xge_rect_t tRect;
	float fCursorX;

	memset(&tRect, 0, sizeof(tRect));
	if ( (pInput == NULL) || (pInput->pWidget == NULL) ) {
		return tRect;
	}
	fCursorX = pInput->pWidget->tContentRect.fX + __xgeXuiTextPrefixWidth(pInput->pFont, xgeXuiInputGetText(pInput), xgeXuiTextGetCursor(&pInput->tText));
	if ( fCursorX > (pInput->pWidget->tContentRect.fX + pInput->pWidget->tContentRect.fW) ) {
		fCursorX = pInput->pWidget->tContentRect.fX + pInput->pWidget->tContentRect.fW;
	}
	tRect.fX = fCursorX;
	tRect.fY = pInput->pWidget->tContentRect.fY;
	tRect.fW = 1.0f;
	tRect.fH = pInput->pWidget->tContentRect.fH;
	return tRect;
}

int xgeXuiInputEvent(xge_xui_input pInput, const xge_event_t* pEvent)
{
	int iResult;
	int iCursor;
	int iStart;
	int iEnd;

	if ( (pInput == NULL) || (pInput->bInitialized == 0) || (pInput->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( ((pInput->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || ((pInput->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_DOWN) || (pEvent->iType == XGE_EVENT_TOUCH_BEGIN)) && __xgeXuiRectContains(pInput->pWidget->tRect, pEvent->fX, pEvent->fY) ) {
		xgeXuiSetFocus(pInput->pContext, pInput->pWidget);
		iCursor = __xgeXuiInputCursorFromX(pInput, pEvent->fX);
		xgeXuiTextSetCursor(&pInput->tText, iCursor);
		pInput->bSelecting = 1;
		xgeXuiSetCapture(pInput->pContext, pInput->pWidget);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_MOVE) || (pEvent->iType == XGE_EVENT_TOUCH_MOVE)) && (pInput->bSelecting != 0) ) {
		iCursor = __xgeXuiInputCursorFromX(pInput, pEvent->fX);
		xgeXuiTextSetSelection(&pInput->tText, pInput->tText.iSelectStart, iCursor);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_UP) || (pEvent->iType == XGE_EVENT_TOUCH_END) || (pEvent->iType == XGE_EVENT_TOUCH_CANCEL)) && (pInput->bSelecting != 0) ) {
		pInput->bSelecting = 0;
		if ( pInput->pContext != NULL && pInput->pContext->pCapture == pInput->pWidget ) {
			xgeXuiSetCapture(pInput->pContext, NULL);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pInput->pContext != NULL) && (pInput->pContext->pFocus != pInput->pWidget) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType == XGE_EVENT_TEXT) || (pEvent->iType == XGE_EVENT_IME_START) || (pEvent->iType == XGE_EVENT_IME_UPDATE) || (pEvent->iType == XGE_EVENT_IME_END) ) {
		iResult = xgeXuiTextInputEvent(&pInput->tText, pEvent);
		if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
			xgeXuiWidgetMarkPaint(pInput->pWidget);
		}
		return iResult;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_BACKSPACE) ) {
		if ( xgeXuiTextDeleteBack(&pInput->tText) == XGE_OK ) {
			xgeXuiWidgetMarkPaint(pInput->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_DELETE) ) {
		if ( xgeXuiTextDeleteForward(&pInput->tText) == XGE_OK ) {
			xgeXuiWidgetMarkPaint(pInput->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_LEFT) ) {
		xgeXuiTextGetSelection(&pInput->tText, &iStart, &iEnd);
		if ( iStart != iEnd ) {
			xgeXuiTextSetCursor(&pInput->tText, iStart);
		} else {
			xgeXuiTextSetCursor(&pInput->tText, __xgeXuiTextPrevCursor(&pInput->tText, pInput->tText.iCursor));
		}
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_RIGHT) ) {
		xgeXuiTextGetSelection(&pInput->tText, &iStart, &iEnd);
		if ( iStart != iEnd ) {
			xgeXuiTextSetCursor(&pInput->tText, iEnd);
		} else {
			xgeXuiTextSetCursor(&pInput->tText, __xgeXuiTextNextCursor(&pInput->tText, pInput->tText.iCursor));
		}
		xgeXuiWidgetMarkPaint(pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int xgeXuiInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiInputEvent((xge_xui_input)pUser, pEvent);
}

void xgeXuiInputPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_input pInput;
	xge_rect_t tCursor;
	xge_rect_t tSelection;
	xge_rect_t tComposition;
	xge_vec2_t tSize;
	uint32_t iBackground;
	float fStartX;
	float fEndX;
	int iStart;
	int iEnd;

	pInput = (xge_xui_input)pUser;
	if ( (pWidget == NULL) || (pInput == NULL) ) {
		return;
	}
	iBackground = (pInput->pContext != NULL && pInput->pContext->pFocus == pWidget) ? pInput->iFocusColor : pInput->iBackgroundColor;
	if ( XGE_COLOR_GET_A(iBackground) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, iBackground);
	}
	xgeXuiTextGetSelection(&pInput->tText, &iStart, &iEnd);
	if ( (iStart != iEnd) && (XGE_COLOR_GET_A(pInput->iSelectionColor) != 0) ) {
		fStartX = pWidget->tContentRect.fX + __xgeXuiTextPrefixWidth(pInput->pFont, pInput->tText.sText, iStart);
		fEndX = pWidget->tContentRect.fX + __xgeXuiTextPrefixWidth(pInput->pFont, pInput->tText.sText, iEnd);
		if ( fStartX < pWidget->tContentRect.fX ) {
			fStartX = pWidget->tContentRect.fX;
		}
		if ( fEndX > (pWidget->tContentRect.fX + pWidget->tContentRect.fW) ) {
			fEndX = pWidget->tContentRect.fX + pWidget->tContentRect.fW;
		}
		if ( fEndX > fStartX ) {
			tSelection.fX = fStartX;
			tSelection.fY = pWidget->tContentRect.fY + 2.0f;
			tSelection.fW = fEndX - fStartX;
			tSelection.fH = pWidget->tContentRect.fH - 4.0f;
			if ( tSelection.fH < 1.0f ) {
				tSelection.fH = 1.0f;
			}
			__xgeXuiHostDrawRect(tSelection, pInput->iSelectionColor);
		}
	}
	if ( (pInput->pFont != NULL) && (pInput->tText.sText != NULL) ) {
		__xgeXuiHostDrawTextRect(pInput->pFont, pInput->tText.sText, pWidget->tContentRect, pInput->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	if ( (pInput->pFont != NULL) && (pInput->tText.sComposition != NULL) && (pInput->tText.sComposition[0] != 0) ) {
		tComposition = xgeXuiInputGetCandidateRect(pInput);
		tComposition.fX += 1.0f;
		tComposition.fW = (pWidget->tContentRect.fX + pWidget->tContentRect.fW) - tComposition.fX;
		if ( tComposition.fW > 0.0f ) {
			__xgeXuiHostDrawTextRect(pInput->pFont, pInput->tText.sComposition, tComposition, pInput->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	if ( (pInput->pContext != NULL) && (pInput->pContext->pFocus == pWidget) && (XGE_COLOR_GET_A(pInput->iCursorColor) != 0) ) {
		tSize.fX = __xgeXuiTextPrefixWidth(pInput->pFont, xgeXuiInputGetText(pInput), xgeXuiTextGetCursor(&pInput->tText));
		if ( (pInput->tText.sComposition != NULL) && (pInput->tText.sComposition[0] != 0) ) {
			tSize.fX += __xgeXuiTextPrefixWidth(pInput->pFont, pInput->tText.sComposition, pInput->tText.iCompositionSize);
		}
		tSize.fY = 0.0f;
		tCursor.fX = pWidget->tContentRect.fX + tSize.fX + 1.0f;
		if ( tCursor.fX > (pWidget->tContentRect.fX + pWidget->tContentRect.fW - 1.0f) ) {
			tCursor.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - 1.0f;
		}
		tCursor.fY = pWidget->tContentRect.fY + 2.0f;
		tCursor.fW = 1.0f;
		tCursor.fH = pWidget->tContentRect.fH - 4.0f;
		if ( tCursor.fH < 1.0f ) {
			tCursor.fH = 1.0f;
		}
		__xgeXuiHostDrawRect(tCursor, pInput->iCursorColor);
	}
}
