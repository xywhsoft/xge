static void __xgeXuiPopupClose(xge_xui_popup pPopup)
{
	if ( (pPopup == NULL) || (pPopup->bOpen == 0) ) {
		return;
	}
	pPopup->bOpen = 0;
	pPopup->iCloseCount++;
	xgeXuiWidgetSetVisible(pPopup->pWidget, 0);
	if ( pPopup->procClose != NULL ) {
		pPopup->procClose(pPopup->pWidget, pPopup->pUser);
	}
}

int xgeXuiPopupInit(xge_xui_popup pPopup, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pPopup == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pPopup, 0, sizeof(*pPopup));
	pPopup->pContext = pContext;
	pPopup->pWidget = pWidget;
	pPopup->iBackgroundColor = XGE_COLOR_RGBA(38, 46, 58, 255);
	pPopup->bCloseOnOutside = 1;
	pPopup->bCloseOnEscape = 1;
	pWidget->procEvent = xgeXuiPopupEventProc;
	pWidget->procPaint = xgeXuiPopupPaintProc;
	pWidget->pUser = pPopup;
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetClip(pWidget, 0);
	xgeXuiPopupSetOpen(pPopup, 0);
	return XGE_OK;
}

void xgeXuiPopupUnit(xge_xui_popup pPopup)
{
	if ( pPopup == NULL ) {
		return;
	}
	if ( pPopup->pWidget != NULL && pPopup->pWidget->pUser == pPopup ) {
		pPopup->pWidget->pUser = NULL;
		pPopup->pWidget->procEvent = NULL;
		pPopup->pWidget->procPaint = NULL;
	}
	memset(pPopup, 0, sizeof(*pPopup));
}

void xgeXuiPopupSetOwner(xge_xui_popup pPopup, xge_xui_widget pOwner)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->pOwner = pOwner;
}

void xgeXuiPopupSetClose(xge_xui_popup pPopup, xge_xui_click_proc procClose, void* pUser)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->procClose = procClose;
	pPopup->pUser = pUser;
}

void xgeXuiPopupSetOpen(xge_xui_popup pPopup, int bOpen)
{
	if ( (pPopup == NULL) || (pPopup->pWidget == NULL) ) {
		return;
	}
	bOpen = bOpen ? 1 : 0;
	if ( pPopup->bOpen == bOpen ) {
		return;
	}
	pPopup->bOpen = bOpen;
	xgeXuiWidgetSetVisible(pPopup->pWidget, bOpen);
	if ( bOpen ) {
		xgeXuiSetFocus(pPopup->pContext, pPopup->pWidget);
	}
	xgeXuiWidgetMarkPaint(pPopup->pWidget);
}

int xgeXuiPopupIsOpen(xge_xui_popup pPopup)
{
	if ( pPopup == NULL ) {
		return 0;
	}
	return pPopup->bOpen;
}

void xgeXuiPopupSetAutoClose(xge_xui_popup pPopup, int bOutside, int bEscape)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->bCloseOnOutside = bOutside ? 1 : 0;
	pPopup->bCloseOnEscape = bEscape ? 1 : 0;
}

void xgeXuiPopupSetBackground(xge_xui_popup pPopup, uint32_t iColor)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->iBackgroundColor = iColor;
	xgeXuiWidgetMarkPaint(pPopup->pWidget);
}

int xgeXuiPopupEvent(xge_xui_popup pPopup, const xge_event_t* pEvent)
{
	if ( (pPopup == NULL) || (pPopup->pWidget == NULL) || (pEvent == NULL) || (pPopup->bOpen == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( __xgeXuiRectContains(pPopup->pWidget->tRect, pEvent->fX, pEvent->fY) ) {
				xgeXuiSetFocus(pPopup->pContext, pPopup->pWidget);
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pPopup->pOwner != NULL) && __xgeXuiRectContains(pPopup->pOwner->tRect, pEvent->fX, pEvent->fY) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( pPopup->bCloseOnOutside ) {
				__xgeXuiPopupClose(pPopup);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_KEY_DOWN:
			if ( (pEvent->iParam1 == XGE_KEY_ESCAPE) && pPopup->bCloseOnEscape ) {
				__xgeXuiPopupClose(pPopup);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiPopupEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiPopupEvent((xge_xui_popup)pUser, pEvent);
}

void xgeXuiPopupPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_popup pPopup;

	pPopup = (xge_xui_popup)pUser;
	if ( (pWidget == NULL) || (pPopup == NULL) || (pPopup->bOpen == 0) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pPopup->iBackgroundColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pPopup->iBackgroundColor);
	}
}
