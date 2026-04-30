static void __xgeXuiDialogClose(xge_xui_dialog pDialog)
{
	if ( (pDialog == NULL) || (pDialog->bOpen == 0) ) {
		return;
	}
	pDialog->iCloseCount++;
	xgeXuiDialogSetOpen(pDialog, 0);
	if ( pDialog->procClose != NULL ) {
		pDialog->procClose(pDialog->pWidget, pDialog->pUser);
	}
}

int xgeXuiDialogInit(xge_xui_dialog pDialog, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pDialog == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pDialog, 0, sizeof(*pDialog));
	pTheme = xgeXuiGetTheme(pContext);
	pDialog->pContext = pContext;
	pDialog->pWidget = pWidget;
	pDialog->sTitle = "";
	pDialog->iBackdropColor = XGE_COLOR_RGBA(24, 56, 79, 72);
	pDialog->iBackgroundColor = pTheme->iPanelColor;
	pDialog->iTitleColor = pTheme->iTextColor;
	pDialog->iCloseColor = pTheme->iAccentColor;
	pDialog->bOpen = 1;
	pDialog->bModal = 1;
	pDialog->bCloseOnEscape = 1;
	pDialog->bShowClose = 1;
	pDialog->iZBase = 1300;
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetZ(pWidget, pDialog->iZBase);
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
	xge_xui_widget pRestore;

	if ( pDialog == NULL ) {
		return;
	}
	bOpen = bOpen ? 1 : 0;
	if ( pDialog->bOpen != bOpen ) {
		pDialog->bOpen = bOpen;
		xgeXuiWidgetSetVisible(pDialog->pWidget, bOpen);
		if ( bOpen ) {
			if ( pDialog->pFocusRestore == NULL && pDialog->pContext != NULL ) {
				pDialog->pFocusRestore = pDialog->pContext->pFocus;
			}
			xgeXuiSetFocus(pDialog->pContext, pDialog->pWidget);
		} else {
			pRestore = pDialog->pFocusRestore;
			if ( (pRestore != NULL) && (pDialog->pContext != NULL) && ((pRestore->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0) && ((pRestore->iFlags & XGE_XUI_WIDGET_ENABLED) != 0) ) {
				xgeXuiSetFocus(pDialog->pContext, pRestore);
			}
		}
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

void xgeXuiDialogSetModal(xge_xui_dialog pDialog, int bModal)
{
	if ( pDialog == NULL ) {
		return;
	}
	pDialog->bModal = bModal ? 1 : 0;
	xgeXuiWidgetMarkPaint(pDialog->pWidget);
}

void xgeXuiDialogSetCloseOnEscape(xge_xui_dialog pDialog, int bEnabled)
{
	if ( pDialog == NULL ) {
		return;
	}
	pDialog->bCloseOnEscape = bEnabled ? 1 : 0;
}

void xgeXuiDialogSetCloseOnOutside(xge_xui_dialog pDialog, int bEnabled)
{
	if ( pDialog == NULL ) {
		return;
	}
	pDialog->bCloseOnOutside = bEnabled ? 1 : 0;
}

void xgeXuiDialogSetFocusRestore(xge_xui_dialog pDialog, xge_xui_widget pWidget)
{
	if ( pDialog == NULL ) {
		return;
	}
	pDialog->pFocusRestore = pWidget;
}

void xgeXuiDialogSetZBase(xge_xui_dialog pDialog, int iZBase)
{
	if ( (pDialog == NULL) || (pDialog->pWidget == NULL) ) {
		return;
	}
	pDialog->iZBase = iZBase;
	xgeXuiWidgetSetZ(pDialog->pWidget, iZBase);
}

void xgeXuiDialogSetShowClose(xge_xui_dialog pDialog, int bShow)
{
	if ( pDialog == NULL ) {
		return;
	}
	pDialog->bShowClose = bShow ? 1 : 0;
	xgeXuiWidgetMarkPaint(pDialog->pWidget);
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
				if ( pDialog->bCloseOnOutside ) {
					__xgeXuiDialogClose(pDialog);
					return XGE_XUI_EVENT_CONSUMED;
				}
				return pDialog->bModal ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pDialog->pContext, pDialog->pWidget);
			if ( pDialog->bShowClose && __xgeXuiRectContains(pDialog->tCloseRect, pEvent->fX, pEvent->fY) ) {
				__xgeXuiDialogClose(pDialog);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_MOUSE_WHEEL:
		case XGE_EVENT_TOUCH_MOVE:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			return pDialog->bModal ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_KEY_DOWN:
			if ( (pEvent->iParam1 == XGE_KEY_ESCAPE) && pDialog->bCloseOnEscape ) {
				__xgeXuiDialogClose(pDialog);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return pDialog->bModal ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_KEY_UP:
		case XGE_EVENT_TEXT:
			return pDialog->bModal ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

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
	static const uint16_t arrClose8[8] = {
		0xc3, 0x66, 0x3c, 0x18, 0x18, 0x3c, 0x66, 0xc3
	};
	xge_xui_dialog pDialog;
	xge_rect_t tTitle;
	xge_rect_t tClose;
	xge_rect_t tIcon;

	pDialog = (xge_xui_dialog)pUser;
	if ( (pWidget == NULL) || (pDialog == NULL) || (pDialog->bOpen == 0) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pDialog->iBackdropColor) != 0 ) {
		__xgeXuiHostDrawRect((pDialog->pContext != NULL && pDialog->pContext->pRoot != NULL) ? pDialog->pContext->pRoot->tRect : (pWidget->pParent != NULL ? pWidget->pParent->tRect : pWidget->tRect), pDialog->iBackdropColor);
	}
	if ( XGE_COLOR_GET_A(pDialog->iBackgroundColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pDialog->iBackgroundColor);
		__xgeXuiHostDrawBorderRect(pWidget->tRect, 1.5f, XGE_COLOR_RGBA(127, 196, 229, 255));
	}
	tTitle = pWidget->tContentRect;
	tTitle.fH = 24.0f;
	__xgeXuiHostDrawRect(tTitle, XGE_COLOR_RGBA(238, 248, 255, 255));
	__xgeXuiHostDrawBorderRect((xge_rect_t){ tTitle.fX, tTitle.fY + tTitle.fH - 1.0f, tTitle.fW, 1.0f }, 1.0f, XGE_COLOR_RGBA(184, 223, 245, 255));
	memset(&tClose, 0, sizeof(tClose));
	if ( pDialog->bShowClose ) {
		tClose.fW = 18.0f;
		tClose.fH = 18.0f;
		tClose.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - tClose.fW - 2.0f;
		tClose.fY = pWidget->tContentRect.fY + 3.0f;
		pDialog->tCloseRect = tClose;
		__xgeXuiHostDrawRect(tClose, pDialog->iCloseColor);
		__xgeXuiHostDrawBorderRect(tClose, 1.0f, XGE_COLOR_RGBA(127, 196, 229, 255));
		tIcon.fW = 8.0f;
		tIcon.fH = 8.0f;
		tIcon.fX = tClose.fX + (tClose.fW - tIcon.fW) * 0.5f;
		tIcon.fY = tClose.fY + (tClose.fH - tIcon.fH) * 0.5f;
		__xgeXuiHostDrawBitmapMask(tIcon, arrClose8, 8, 8, XGE_COLOR_RGBA(255, 255, 255, 255));
	} else {
		pDialog->tCloseRect = tClose;
	}
	if ( (pDialog->pFont != NULL) && (pDialog->sTitle != NULL) && (pDialog->sTitle[0] != 0) ) {
		if ( pDialog->bShowClose ) {
			tTitle.fW -= 24.0f;
		}
		__xgeXuiHostDrawTextRect(pDialog->pFont, pDialog->sTitle, tTitle, pDialog->iTitleColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
}
