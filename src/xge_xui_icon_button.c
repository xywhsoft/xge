static void __xgeXuiIconButtonSetState(xge_xui_icon_button pButton, int iState)
{
	if ( pButton == NULL ) {
		return;
	}
	if ( (pButton->pWidget == NULL) || ((pButton->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pButton->pContext != NULL && pButton->pContext->pFocus == pButton->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pButton->iState != iState ) {
		pButton->iState = iState;
		xgeXuiWidgetMarkPaint(pButton->pWidget);
	}
}

static uint32_t __xgeXuiIconButtonColor(xge_xui_icon_button pButton)
{
	if ( pButton == NULL ) {
		return XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	if ( (pButton->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return pButton->iColorDisabled;
	}
	if ( (pButton->iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		return pButton->iColorActive;
	}
	if ( (pButton->iState & XGE_XUI_STATE_HOVER) != 0 ) {
		return pButton->iColorHover;
	}
	if ( (pButton->iState & XGE_XUI_STATE_FOCUS) != 0 ) {
		return pButton->iColorFocus;
	}
	return pButton->iColorNormal;
}

static xge_rect_t __xgeXuiIconButtonDestRect(xge_xui_icon_button pButton, xge_rect_t tContent)
{
	xge_rect_t tDst;
	float fSrcW;
	float fSrcH;
	float fScale;

	tDst = tContent;
	if ( (pButton == NULL) || (pButton->pTexture == NULL) ) {
		return tDst;
	}
	fSrcW = (pButton->tSrc.fW > 0.0f) ? pButton->tSrc.fW : (float)pButton->pTexture->iWidth;
	fSrcH = (pButton->tSrc.fH > 0.0f) ? pButton->tSrc.fH : (float)pButton->pTexture->iHeight;
	if ( (fSrcW <= 0.0f) || (fSrcH <= 0.0f) ) {
		return tDst;
	}
	if ( pButton->iMode == XGE_XUI_IMAGE_CENTER ) {
		tDst.fW = fSrcW;
		tDst.fH = fSrcH;
		tDst.fX = tContent.fX + (tContent.fW - tDst.fW) * 0.5f;
		tDst.fY = tContent.fY + (tContent.fH - tDst.fH) * 0.5f;
	} else if ( pButton->iMode == XGE_XUI_IMAGE_FIT ) {
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

int xgeXuiIconButtonInit(xge_xui_icon_button pButton, xge_xui_context pContext, xge_xui_widget pWidget, xge_texture pTexture)
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
	pButton->pTexture = pTexture;
	pButton->iColorNormal = pTheme->iStateNormal;
	pButton->iColorHover = pTheme->iStateHover;
	pButton->iColorActive = pTheme->iStateActive;
	pButton->iColorFocus = pTheme->iStateFocus;
	pButton->iColorDisabled = pTheme->iStateDisabled;
	pButton->iIconColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pButton->iMode = XGE_XUI_IMAGE_FIT;
	pWidget->tStyle.fRadius = pTheme->fRadius;
	xgeXuiWidgetSetEvent(pWidget, xgeXuiIconButtonEventProc, NULL);
	pWidget->procPaint = xgeXuiIconButtonPaintProc;
	pWidget->pUser = pButton;
	__xgeXuiIconButtonSetState(pButton, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiIconButtonUnit(xge_xui_icon_button pButton)
{
	if ( pButton == NULL ) {
		return;
	}
	if ( pButton->pWidget != NULL && pButton->pWidget->pUser == pButton ) {
		pButton->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pButton->pWidget, NULL, NULL);
		pButton->pWidget->procPaint = NULL;
	}
	memset(pButton, 0, sizeof(*pButton));
}

void xgeXuiIconButtonSetClick(xge_xui_icon_button pButton, xge_xui_click_proc procClick, void* pUser)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->procClick = procClick;
	pButton->pUser = pUser;
}

void xgeXuiIconButtonSetTexture(xge_xui_icon_button pButton, xge_texture pTexture)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->pTexture = pTexture;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiIconButtonSetSource(xge_xui_icon_button pButton, xge_rect_t tSrc)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->tSrc = tSrc;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiIconButtonSetIconColor(xge_xui_icon_button pButton, uint32_t iColor)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->iIconColor = iColor;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiIconButtonSetMode(xge_xui_icon_button pButton, int iMode)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->iMode = iMode;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiIconButtonSetColors(xge_xui_icon_button pButton, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled)
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

int xgeXuiIconButtonGetState(xge_xui_icon_button pButton)
{
	if ( pButton == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiIconButtonSetState(pButton, pButton->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pButton->iState;
}

int xgeXuiIconButtonEvent(xge_xui_icon_button pButton, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;

	if ( (pButton == NULL) || (pButton->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pButton->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pButton->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiIconButtonSetState(pButton, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pButton->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pButton->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			__xgeXuiIconButtonSetState(pButton, iInside ? (iState | XGE_XUI_STATE_HOVER) : (iState & ~XGE_XUI_STATE_HOVER));
			return ((pButton->iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiIconButtonSetState(pButton, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiIconButtonSetState(pButton, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiIconButtonSetState(pButton, iState);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pButton->pContext, pButton->pWidget);
			xgeXuiSetPointerCapture(pButton->pContext, pEvent->iPointerId, pButton->pWidget);
			__xgeXuiIconButtonSetState(pButton, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pButton->iState & XGE_XUI_STATE_ACTIVE) != 0);
			__xgeXuiIconButtonSetState(pButton, iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			if ( pButton->pContext != NULL && xgeXuiGetPointerCapture(pButton->pContext, pEvent->iPointerId) == pButton->pWidget ) {
				xgeXuiSetPointerCapture(pButton->pContext, pEvent->iPointerId, NULL);
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
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			__xgeXuiIconButtonSetState(pButton, XGE_XUI_STATE_NORMAL);
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
			pButton->iClickCount++;
			if ( pButton->procClick != NULL ) {
				pButton->procClick(pButton->pWidget, pButton->pUser);
			}
			return XGE_XUI_EVENT_CONSUMED;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiIconButtonEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiIconButtonEvent((xge_xui_icon_button)pUser, pEvent);
}

void xgeXuiIconButtonPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_icon_button pButton;
	xge_draw_t tDraw;
	uint32_t iColor;

	pButton = (xge_xui_icon_button)pUser;
	if ( (pWidget == NULL) || (pButton == NULL) ) {
		return;
	}
	iColor = __xgeXuiIconButtonColor(pButton);
	if ( XGE_COLOR_GET_A(iColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, iColor);
	}
	__xgeXuiHostDrawBorderRect(pWidget->tRect, 1.0f, XGE_COLOR_RGBA(127, 196, 229, 255));
	if ( (pButton->pTexture == NULL) || (XGE_COLOR_GET_A(pButton->iIconColor) == 0) ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pButton->pTexture;
	tDraw.tSrc = pButton->tSrc;
	tDraw.tDst = __xgeXuiIconButtonDestRect(pButton, pWidget->tContentRect);
	tDraw.iColor = pButton->iIconColor;
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	__xgeXuiHostDrawImage(&tDraw);
}
