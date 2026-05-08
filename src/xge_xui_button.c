int xgeXuiButtonInit(xge_xui_button pButton, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pButton == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pButton, 0, sizeof(*pButton));
	__xgeXuiControlWidgetInit(pWidget, 1);
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
	pButton->iColorChecked = pTheme->iAccentColor;
	pButton->iIconColor = pTheme->iTextColor;
	pButton->fIconSize = 16.0f;
	pButton->fIconGap = 6.0f;
	pButton->iIconPlacement = XGE_XUI_BUTTON_ICON_LEFT;
	pButton->iSemantic = XGE_XUI_BUTTON_SEMANTIC_DEFAULT;
	pWidget->tStyle.fRadius = pTheme->fRadius;
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
	pButton->iIconColor = iColor;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetCheckable(xge_xui_button pButton, int bCheckable)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->bCheckable = (bCheckable != 0);
	if ( pButton->bCheckable == 0 ) {
		pButton->bChecked = 0;
	}
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetChecked(xge_xui_button pButton, int bChecked)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->bCheckable = 1;
	pButton->bChecked = (bChecked != 0);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

int xgeXuiButtonGetChecked(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return 0;
	}
	return pButton->bChecked;
}

void xgeXuiButtonSetLoading(xge_xui_button pButton, int bLoading)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->bLoading = (bLoading != 0);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

int xgeXuiButtonGetLoading(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return 0;
	}
	return pButton->bLoading;
}

void xgeXuiButtonSetSemantic(xge_xui_button pButton, int iSemantic)
{
	if ( pButton == NULL ) {
		return;
	}
	if ( iSemantic == XGE_XUI_BUTTON_SEMANTIC_PRIMARY ) {
		pButton->iColorNormal = XGE_COLOR_RGBA(35, 132, 214, 255);
		pButton->iColorHover = XGE_COLOR_RGBA(56, 151, 228, 255);
		pButton->iColorActive = XGE_COLOR_RGBA(22, 104, 176, 255);
		pButton->iColorChecked = XGE_COLOR_RGBA(20, 96, 164, 255);
		pButton->iTextColor = XGE_COLOR_RGBA(248, 252, 255, 255);
		pButton->iIconColor = pButton->iTextColor;
	} else if ( iSemantic == XGE_XUI_BUTTON_SEMANTIC_DANGER ) {
		pButton->iColorNormal = XGE_COLOR_RGBA(214, 72, 86, 255);
		pButton->iColorHover = XGE_COLOR_RGBA(228, 92, 104, 255);
		pButton->iColorActive = XGE_COLOR_RGBA(178, 52, 66, 255);
		pButton->iColorChecked = XGE_COLOR_RGBA(160, 44, 58, 255);
		pButton->iTextColor = XGE_COLOR_RGBA(255, 250, 250, 255);
		pButton->iIconColor = pButton->iTextColor;
	} else {
		const xge_xui_theme_t* pTheme = xgeXuiGetTheme(pButton->pContext);
		iSemantic = XGE_XUI_BUTTON_SEMANTIC_DEFAULT;
		pButton->iColorNormal = pTheme->iStateNormal;
		pButton->iColorHover = pTheme->iStateHover;
		pButton->iColorActive = pTheme->iStateActive;
		pButton->iColorChecked = pTheme->iAccentColor;
		pButton->iTextColor = pTheme->iTextColor;
		pButton->iIconColor = pButton->iTextColor;
	}
	pButton->iSemantic = iSemantic;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

int xgeXuiButtonGetSemantic(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return XGE_XUI_BUTTON_SEMANTIC_DEFAULT;
	}
	return pButton->iSemantic;
}

void xgeXuiButtonSetIcon(xge_xui_button pButton, xge_texture pTexture, xge_rect_t tSrc)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->pIconTexture = pTexture;
	pButton->tIconSrc = tSrc;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetIconColor(xge_xui_button pButton, uint32_t iColor)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->iIconColor = iColor;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetIconLayout(xge_xui_button pButton, int iPlacement, float fIconSize, float fGap)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->iIconPlacement = (iPlacement == XGE_XUI_BUTTON_ICON_RIGHT) ? XGE_XUI_BUTTON_ICON_RIGHT : XGE_XUI_BUTTON_ICON_LEFT;
	pButton->fIconSize = (fIconSize > 0.0f) ? fIconSize : 16.0f;
	pButton->fIconGap = (fGap > 0.0f) ? fGap : 0.0f;
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
	pButton->iColorChecked = iActive;
	pButton->iSemantic = XGE_XUI_BUTTON_SEMANTIC_DEFAULT;
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
	if ( pButton->bLoading != 0 ) {
		if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) || (pEvent->iType == XGE_EVENT_MOUSE_UP) || (pEvent->iType == XGE_EVENT_TOUCH_BEGIN) || (pEvent->iType == XGE_EVENT_TOUCH_END) || (pEvent->iType == XGE_EVENT_KEY_DOWN) ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
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

		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiButtonSetState(pButton, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiButtonSetState(pButton, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiButtonSetState(pButton, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pButton->pContext, pButton->pWidget);
			xgeXuiSetPointerCapture(pButton->pContext, pEvent->iPointerId, pButton->pWidget);
			__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pButton->iState & XGE_XUI_STATE_ACTIVE) != 0);
			iState = iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
			__xgeXuiButtonSetState(pButton, iState);
			if ( pButton->pContext != NULL && xgeXuiGetPointerCapture(pButton->pContext, pEvent->iPointerId) == pButton->pWidget ) {
				xgeXuiSetPointerCapture(pButton->pContext, pEvent->iPointerId, NULL);
			}
			if ( bWasActive && iInside ) {
				if ( pButton->bCheckable != 0 ) {
					pButton->bChecked = (pButton->bChecked == 0);
					xgeXuiWidgetMarkPaint(pButton->pWidget);
				}
				pButton->iClickCount++;
				if ( pButton->procClick != NULL ) {
					pButton->procClick(pButton->pWidget, pButton->pUser);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_NORMAL);
			if ( pButton->pContext != NULL && xgeXuiGetPointerCapture(pButton->pContext, pEvent->iPointerId) == pButton->pWidget ) {
				xgeXuiSetPointerCapture(pButton->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_KEY_DOWN:
			if ( (pButton->pContext == NULL) || (pButton->pContext->pFocus != pButton->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 != XGE_KEY_ENTER) && (pEvent->iParam1 != XGE_KEY_SPACE) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( pButton->bCheckable != 0 ) {
				pButton->bChecked = (pButton->bChecked == 0);
				xgeXuiWidgetMarkPaint(pButton->pWidget);
			}
			pButton->iClickCount++;
			if ( pButton->procClick != NULL ) {
				pButton->procClick(pButton->pWidget, pButton->pUser);
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

static void __xgeXuiButtonLayoutContent(xge_xui_button pButton, xge_rect_t tContent)
{
	xge_vec2_t tTextSize;
	float fIconSize;
	float fGap;
	float fTextW;
	float fTotalW;
	float fStartX;
	int bHasText;
	int bHasIcon;

	pButton->tIconRect = (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	pButton->tTextRect = tContent;
	bHasText = (pButton->sText != NULL) && (pButton->sText[0] != 0);
	bHasIcon = (pButton->pIconTexture != NULL) && (pButton->fIconSize > 0.0f);
	if ( bHasIcon == 0 ) {
		return;
	}
	fIconSize = pButton->fIconSize;
	if ( fIconSize > tContent.fH ) {
		fIconSize = tContent.fH;
	}
	if ( fIconSize > tContent.fW ) {
		fIconSize = tContent.fW;
	}
	fGap = bHasText ? pButton->fIconGap : 0.0f;
	fTextW = 0.0f;
	if ( bHasText ) {
		if ( pButton->pFont != NULL ) {
			tTextSize = __xgeXuiHostMeasureText(pButton->pFont, pButton->sText);
			fTextW = tTextSize.fX;
		}
		if ( fTextW <= 0.0f ) {
			fTextW = (float)strlen(pButton->sText) * 6.0f;
		}
		if ( fTextW > tContent.fW - fIconSize - fGap ) {
			fTextW = tContent.fW - fIconSize - fGap;
		}
		if ( fTextW < 0.0f ) {
			fTextW = 0.0f;
		}
	}
	fTotalW = fIconSize + fGap + fTextW;
	if ( fTotalW > tContent.fW ) {
		fTotalW = tContent.fW;
	}
	fStartX = tContent.fX + (tContent.fW - fTotalW) * 0.5f;
	if ( pButton->iIconPlacement == XGE_XUI_BUTTON_ICON_RIGHT ) {
		pButton->tTextRect = (xge_rect_t){ fStartX, tContent.fY, fTextW, tContent.fH };
		pButton->tIconRect = (xge_rect_t){ fStartX + fTextW + fGap, tContent.fY + (tContent.fH - fIconSize) * 0.5f, fIconSize, fIconSize };
	} else {
		pButton->tIconRect = (xge_rect_t){ fStartX, tContent.fY + (tContent.fH - fIconSize) * 0.5f, fIconSize, fIconSize };
		pButton->tTextRect = (xge_rect_t){ fStartX + fIconSize + fGap, tContent.fY, fTextW, tContent.fH };
	}
}

void xgeXuiButtonPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_button pButton;
	xge_draw_t tDraw;
	xge_rect_t tContent;
	xge_rect_t tDot;
	uint32_t iColor;
	uint32_t iDotColor;
	int bHasIcon;
	int bHasText;

	pButton = (xge_xui_button)pUser;
	if ( (pWidget == NULL) || (pButton == NULL) ) {
		return;
	}
	tContent = pWidget->tContentRect;
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		tContent = pWidget->tRect;
	}
	iColor = __xgeXuiButtonColor(pButton);
	if ( XGE_COLOR_GET_A(iColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, iColor);
	}
	__xgeXuiHostDrawBorderRect(pWidget->tRect, 1.0f, XGE_COLOR_RGBA(127, 196, 229, 255));
	bHasIcon = (pButton->pIconTexture != NULL) && (pButton->fIconSize > 0.0f);
	bHasText = (pButton->pFont != NULL) && (pButton->sText != NULL) && (pButton->sText[0] != 0);
	__xgeXuiButtonLayoutContent(pButton, tContent);
	if ( bHasIcon && (XGE_COLOR_GET_A(pButton->iIconColor) != 0) ) {
		memset(&tDraw, 0, sizeof(tDraw));
		tDraw.pTexture = pButton->pIconTexture;
		tDraw.tSrc = pButton->tIconSrc;
		tDraw.tDst = pButton->tIconRect;
		tDraw.iColor = pButton->iIconColor;
		tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
		__xgeXuiHostDrawImage(&tDraw);
	}
	if ( bHasText ) {
		if ( bHasIcon ) {
			__xgeXuiHostDrawTextRect(pButton->pFont, pButton->sText, pButton->tTextRect, pButton->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		} else {
			__xgeXuiHostDrawTextRect(pButton->pFont, pButton->sText, tContent, pButton->iTextColor, pButton->iTextFlags);
		}
	}
	if ( pButton->bLoading != 0 ) {
		iDotColor = pButton->iTextColor;
		tDot.fW = 2.0f;
		tDot.fH = 2.0f;
		tDot.fX = tContent.fX + tContent.fW - 16.0f;
		tDot.fY = tContent.fY + (tContent.fH - 2.0f) * 0.5f;
		__xgeXuiHostDrawRect(tDot, iDotColor);
		tDot.fX += 5.0f;
		__xgeXuiHostDrawRect(tDot, iDotColor);
		tDot.fX += 5.0f;
		__xgeXuiHostDrawRect(tDot, iDotColor);
	}
}
