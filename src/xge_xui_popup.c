static int __xgeXuiPopupPlacementClamp(int iPlacement)
{
	if ( (iPlacement < XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT) || (iPlacement > XGE_XUI_OVERLAY_PLACEMENT_MANUAL) ) {
		return XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT;
	}
	return iPlacement;
}

static int __xgeXuiPopupOwnerAvailable(xge_xui_popup pPopup)
{
	if ( (pPopup == NULL) || (pPopup->pOwner == NULL) ) {
		return 1;
	}
	return ((pPopup->pOwner->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0) && ((pPopup->pOwner->iFlags & XGE_XUI_WIDGET_ENABLED) != 0);
}

static void __xgeXuiPopupRestoreFocus(xge_xui_popup pPopup)
{
	xge_xui_widget pRestore;

	if ( pPopup == NULL ) {
		return;
	}
	pRestore = pPopup->pFocusRestore;
	if ( (pRestore != NULL) && (pPopup->pContext != NULL) && ((pRestore->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0) && ((pRestore->iFlags & XGE_XUI_WIDGET_ENABLED) != 0) ) {
		xgeXuiSetFocus(pPopup->pContext, pRestore);
	}
	if ( pPopup->bFocusRestoreExplicit == 0 ) {
		pPopup->pFocusRestore = NULL;
	}
}

static void __xgeXuiPopupClampRect(xge_rect_t* pRect)
{
	float fWindowW;
	float fWindowH;

	if ( pRect == NULL ) {
		return;
	}
	fWindowW = (float)xgeGetWidth();
	fWindowH = (float)xgeGetHeight();
	if ( fWindowW <= 0.0f ) {
		fWindowW = pRect->fX + pRect->fW;
	}
	if ( fWindowH <= 0.0f ) {
		fWindowH = pRect->fY + pRect->fH;
	}
	if ( pRect->fW > fWindowW ) {
		pRect->fW = fWindowW;
	}
	if ( pRect->fH > fWindowH ) {
		pRect->fH = fWindowH;
	}
	if ( pRect->fX + pRect->fW > fWindowW ) {
		pRect->fX = fWindowW - pRect->fW;
	}
	if ( pRect->fY + pRect->fH > fWindowH ) {
		pRect->fY = fWindowH - pRect->fH;
	}
	if ( pRect->fX < 0.0f ) {
		pRect->fX = 0.0f;
	}
	if ( pRect->fY < 0.0f ) {
		pRect->fY = 0.0f;
	}
}

static void __xgeXuiPopupClose(xge_xui_popup pPopup)
{
	if ( (pPopup == NULL) || (pPopup->bOpen == 0) ) {
		return;
	}
	pPopup->bOpen = 0;
	pPopup->iCloseCount++;
	xgeXuiWidgetSetVisible(pPopup->pWidget, 0);
	__xgeXuiPopupRestoreFocus(pPopup);
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
	__xgeXuiOverlayWidgetInit(pWidget, 1);
	pPopup->pContext = pContext;
	pPopup->pWidget = pWidget;
	pPopup->iPlacement = XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT;
	pPopup->bCloseOnOutside = 1;
	pPopup->bCloseOnEscape = 1;
	xgeXuiWidgetSetEvent(pWidget, xgeXuiPopupEventProc, NULL);
	pWidget->pUser = pPopup;
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetLayer(pWidget, XGE_XUI_LAYER_POPUP);
	xgeXuiWidgetSetVisible(pWidget, 0);
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(38, 46, 58, 255));
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiPopupUnit(xge_xui_popup pPopup)
{
	if ( pPopup == NULL ) {
		return;
	}
	if ( pPopup->pWidget != NULL && pPopup->pWidget->pUser == pPopup ) {
		pPopup->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pPopup->pWidget, NULL, NULL);
	}
	memset(pPopup, 0, sizeof(*pPopup));
}

void xgeXuiPopupSetOwner(xge_xui_popup pPopup, xge_xui_widget pOwner)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->pOwner = pOwner;
	if ( pPopup->pWidget != NULL ) {
		pPopup->pWidget->pOverlayOwner = pOwner;
	}
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
		if ( bOpen ) {
			xgeXuiOverlayBringToFront(pPopup->pContext, pPopup->pWidget);
		}
		return;
	}
	if ( bOpen && __xgeXuiPopupOwnerAvailable(pPopup) == 0 ) {
		return;
	}
	pPopup->bOpen = bOpen;
	xgeXuiWidgetSetVisible(pPopup->pWidget, bOpen);
	if ( bOpen ) {
		xgeXuiOverlayBringToFront(pPopup->pContext, pPopup->pWidget);
		if ( pPopup->bFocusRestoreExplicit == 0 && pPopup->pContext != NULL ) {
			pPopup->pFocusRestore = pPopup->pContext->pFocus;
		}
		xgeXuiPopupApplyPlacement(pPopup);
		xgeXuiSetFocus(pPopup->pContext, pPopup->pWidget);
	} else {
		__xgeXuiPopupRestoreFocus(pPopup);
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

void xgeXuiPopupSetModal(xge_xui_popup pPopup, int bModal)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->bModal = bModal ? 1 : 0;
}

void xgeXuiPopupSetAutoClose(xge_xui_popup pPopup, int bOutside, int bEscape)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->bCloseOnOutside = bOutside ? 1 : 0;
	pPopup->bCloseOnEscape = bEscape ? 1 : 0;
}

void xgeXuiPopupSetPlacement(xge_xui_popup pPopup, int iPlacement)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->iPlacement = __xgeXuiPopupPlacementClamp(iPlacement);
	xgeXuiPopupApplyPlacement(pPopup);
}

void xgeXuiPopupSetAnchorRect(xge_xui_popup pPopup, xge_rect_t tAnchor)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->tAnchorRect = tAnchor;
	xgeXuiPopupApplyPlacement(pPopup);
}

void xgeXuiPopupSetOffset(xge_xui_popup pPopup, float fX, float fY)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->fOffsetX = fX;
	pPopup->fOffsetY = fY;
	xgeXuiPopupApplyPlacement(pPopup);
}

void xgeXuiPopupSetFocusRestore(xge_xui_popup pPopup, xge_xui_widget pWidget)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->pFocusRestore = pWidget;
	pPopup->bFocusRestoreExplicit = (pWidget != NULL);
}

void xgeXuiPopupApplyPlacement(xge_xui_popup pPopup)
{
	xge_rect_t tRect;
	xge_rect_t tAnchor;
	float fWindowW;
	float fWindowH;

	if ( (pPopup == NULL) || (pPopup->pWidget == NULL) || (pPopup->iPlacement == XGE_XUI_OVERLAY_PLACEMENT_MANUAL) ) {
		return;
	}
	tRect = pPopup->pWidget->tRect;
	tAnchor = pPopup->tAnchorRect;
	if ( tAnchor.fX == 0.0f && tAnchor.fY == 0.0f && tAnchor.fW <= 0.0f && tAnchor.fH <= 0.0f && pPopup->pOwner != NULL ) {
		tAnchor = pPopup->pOwner->tRect;
	}
	fWindowW = (float)xgeGetWidth();
	fWindowH = (float)xgeGetHeight();
	switch ( pPopup->iPlacement ) {
		case XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_RIGHT:
			tRect.fX = tAnchor.fX + tAnchor.fW - tRect.fW;
			tRect.fY = tAnchor.fY + tAnchor.fH;
			break;
		case XGE_XUI_OVERLAY_PLACEMENT_TOP_LEFT:
			tRect.fX = tAnchor.fX;
			tRect.fY = tAnchor.fY - tRect.fH;
			break;
		case XGE_XUI_OVERLAY_PLACEMENT_TOP_RIGHT:
			tRect.fX = tAnchor.fX + tAnchor.fW - tRect.fW;
			tRect.fY = tAnchor.fY - tRect.fH;
			break;
		case XGE_XUI_OVERLAY_PLACEMENT_RIGHT_TOP:
			tRect.fX = tAnchor.fX + tAnchor.fW;
			tRect.fY = tAnchor.fY;
			break;
		case XGE_XUI_OVERLAY_PLACEMENT_LEFT_TOP:
			tRect.fX = tAnchor.fX - tRect.fW;
			tRect.fY = tAnchor.fY;
			break;
		case XGE_XUI_OVERLAY_PLACEMENT_CENTER:
			tRect.fX = (fWindowW - tRect.fW) * 0.5f;
			tRect.fY = (fWindowH - tRect.fH) * 0.5f;
			break;
		case XGE_XUI_OVERLAY_PLACEMENT_CURSOR:
		case XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT:
		default:
			tRect.fX = tAnchor.fX;
			tRect.fY = tAnchor.fY + tAnchor.fH;
			break;
	}
	tRect.fX += pPopup->fOffsetX;
	tRect.fY += pPopup->fOffsetY;
	__xgeXuiPopupClampRect(&tRect);
	xgeXuiWidgetSetRect(pPopup->pWidget, tRect);
}

void xgeXuiPopupSetBackground(xge_xui_popup pPopup, uint32_t iColor)
{
	if ( pPopup == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pPopup->pWidget, iColor);
}

void xgeXuiPopupSetBorder(xge_xui_popup pPopup, uint32_t iColor)
{
	if ( pPopup == NULL ) {
		return;
	}
	xgeXuiWidgetSetBorder(pPopup->pWidget, (XGE_COLOR_GET_A(iColor) != 0) ? 1.0f : 0.0f, iColor);
}

int xgeXuiPopupEvent(xge_xui_popup pPopup, const xge_event_t* pEvent)
{
	if ( (pPopup == NULL) || (pPopup->pWidget == NULL) || (pEvent == NULL) || (pPopup->bOpen == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( __xgeXuiPopupOwnerAvailable(pPopup) == 0 ) {
		__xgeXuiPopupClose(pPopup);
		return XGE_XUI_EVENT_CONSUMED;
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
			return pPopup->bModal ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

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
	(void)pWidget;
	(void)pUser;
}
