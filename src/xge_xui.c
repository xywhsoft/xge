int xgeXuiDialogInit(xge_xui_dialog pDialog, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pDialog == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pDialog, 0, sizeof(*pDialog));
	pDialog->pContext = pContext;
	pDialog->pWidget = pWidget;
	pDialog->sTitle = "";
	pDialog->iBackdropColor = XGE_COLOR_RGBA(0, 0, 0, 120);
	pDialog->iBackgroundColor = XGE_COLOR_RGBA(40, 46, 56, 255);
	pDialog->iTitleColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pDialog->iCloseColor = XGE_COLOR_RGBA(210, 86, 86, 255);
	pDialog->bOpen = 1;
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetClip(pWidget, 1);
	pWidget->procEvent = xgeXuiDialogEventProc;
	pWidget->procPaint = xgeXuiDialogPaintProc;
	pWidget->pUser = pDialog;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiDialogUnit(xge_xui_dialog pDialog)
{
	if ( pDialog == NULL ) {
		return;
	}
	if ( pDialog->pWidget != NULL && pDialog->pWidget->pUser == pDialog ) {
		pDialog->pWidget->pUser = NULL;
		pDialog->pWidget->procEvent = NULL;
		pDialog->pWidget->procPaint = NULL;
	}
	memset(pDialog, 0, sizeof(*pDialog));
}

void xgeXuiDialogSetTitle(xge_xui_dialog pDialog, xge_font pFont, const char* sTitle)
{
	if ( pDialog == NULL ) {
		return;
	}
	pDialog->pFont = pFont;
	pDialog->sTitle = (sTitle != NULL) ? sTitle : "";
	xgeXuiWidgetMarkPaint(pDialog->pWidget);
}

void xgeXuiDialogSetClose(xge_xui_dialog pDialog, xge_xui_click_proc procClose, void* pUser)
{
	if ( pDialog == NULL ) {
		return;
	}
	pDialog->procClose = procClose;
	pDialog->pUser = pUser;
}

void xgeXuiDialogSetOpen(xge_xui_dialog pDialog, int bOpen)
{
	if ( pDialog == NULL ) {
		return;
	}
	bOpen = bOpen ? 1 : 0;
	if ( pDialog->bOpen != bOpen ) {
		pDialog->bOpen = bOpen;
		xgeXuiWidgetSetVisible(pDialog->pWidget, bOpen);
		xgeXuiWidgetMarkPaint(pDialog->pWidget);
	}
}

int xgeXuiDialogIsOpen(xge_xui_dialog pDialog)
{
	if ( pDialog == NULL ) {
		return 0;
	}
	return pDialog->bOpen;
}

void xgeXuiDialogSetColors(xge_xui_dialog pDialog, uint32_t iBackdrop, uint32_t iBackground, uint32_t iTitle, uint32_t iClose)
{
	if ( pDialog == NULL ) {
		return;
	}
	pDialog->iBackdropColor = iBackdrop;
	pDialog->iBackgroundColor = iBackground;
	pDialog->iTitleColor = iTitle;
	pDialog->iCloseColor = iClose;
	xgeXuiWidgetMarkPaint(pDialog->pWidget);
}

int xgeXuiDialogEvent(xge_xui_dialog pDialog, const xge_event_t* pEvent)
{
	if ( (pDialog == NULL) || (pDialog->pWidget == NULL) || (pEvent == NULL) || (pDialog->bOpen == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( __xgeXuiRectContains(pDialog->pWidget->tRect, pEvent->fX, pEvent->fY) == 0 ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			xgeXuiSetFocus(pDialog->pContext, pDialog->pWidget);
			if ( __xgeXuiRectContains(pDialog->tCloseRect, pEvent->fX, pEvent->fY) ) {
				pDialog->iCloseCount++;
				xgeXuiDialogSetOpen(pDialog, 0);
				if ( pDialog->procClose != NULL ) {
					pDialog->procClose(pDialog->pWidget, pDialog->pUser);
				}
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_MOUSE_WHEEL:
		case XGE_EVENT_TOUCH_MOVE:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_KEY_DOWN:
		case XGE_EVENT_KEY_UP:
		case XGE_EVENT_TEXT:
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiDialogEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiDialogEvent((xge_xui_dialog)pUser, pEvent);
}

void xgeXuiDialogPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_dialog pDialog;
	xge_rect_t tTitle;
	xge_rect_t tClose;

	pDialog = (xge_xui_dialog)pUser;
	if ( (pWidget == NULL) || (pDialog == NULL) || (pDialog->bOpen == 0) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pDialog->iBackdropColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->pParent != NULL ? pWidget->pParent->tRect : pWidget->tRect, pDialog->iBackdropColor);
	}
	if ( XGE_COLOR_GET_A(pDialog->iBackgroundColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pDialog->iBackgroundColor);
	}
	tTitle = pWidget->tContentRect;
	tTitle.fH = 24.0f;
	tClose.fW = 18.0f;
	tClose.fH = 18.0f;
	tClose.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - tClose.fW - 2.0f;
	tClose.fY = pWidget->tContentRect.fY + 3.0f;
	pDialog->tCloseRect = tClose;
	__xgeXuiHostDrawRect(tClose, pDialog->iCloseColor);
	if ( (pDialog->pFont != NULL) && (pDialog->sTitle != NULL) && (pDialog->sTitle[0] != 0) ) {
		tTitle.fW -= 24.0f;
		__xgeXuiHostDrawTextRect(pDialog->pFont, pDialog->sTitle, tTitle, pDialog->iTitleColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
}
