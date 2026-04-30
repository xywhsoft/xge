static void __xgeXuiTooltipLayout(xge_xui_tooltip pTooltip)
{
	xge_vec2_t tText;
	xge_rect_t tPopup;

	if ( (pTooltip == NULL) || (pTooltip->pOwner == NULL) || (pTooltip->pPopupWidget == NULL) || (pTooltip->pLabelWidget == NULL) ) {
		return;
	}
	tText = xgeXuiLabelMeasure(&pTooltip->tLabel);
	if ( tText.fX < 1.0f ) {
		tText.fX = 1.0f;
	}
	if ( tText.fY < 14.0f ) {
		tText.fY = 14.0f;
	}
	tPopup.fX = pTooltip->pOwner->tRect.fX;
	tPopup.fY = pTooltip->pOwner->tRect.fY + pTooltip->pOwner->tRect.fH;
	tPopup.fW = tText.fX + 12.0f;
	tPopup.fH = tText.fY + 8.0f;
	xgeXuiWidgetSetRect(pTooltip->pPopupWidget, tPopup);
	xgeXuiPopupSetAnchorRect(&pTooltip->tPopup, pTooltip->pOwner->tRect);
	xgeXuiPopupSetOffset(&pTooltip->tPopup, pTooltip->fOffsetX, pTooltip->fOffsetY);
	xgeXuiPopupApplyPlacement(&pTooltip->tPopup);
	xgeXuiWidgetSetRect(pTooltip->pLabelWidget, (xge_rect_t){ 0.0f, 0.0f, tText.fX, tText.fY });
}

int xgeXuiTooltipInit(xge_xui_tooltip pTooltip, xge_xui_context pContext, xge_xui_widget pOwner)
{
	const xge_xui_theme_t* pTheme;

	if ( (pTooltip == NULL) || (pContext == NULL) || (pOwner == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pTooltip, 0, sizeof(*pTooltip));
	pTheme = xgeXuiGetTheme(pContext);
	pTooltip->pContext = pContext;
	pTooltip->pOwner = pOwner;
	pTooltip->pFont = pTheme->pFont;
	pTooltip->sText = "";
	pTooltip->iBackgroundColor = XGE_COLOR_RGBA(42, 50, 64, 245);
	pTooltip->iTextColor = pTheme->iTextColor;
	pTooltip->fOffsetX = 8.0f;
	pTooltip->fOffsetY = 6.0f;
	pTooltip->bEnabled = 1;
	pTooltip->procOldCapture = pOwner->procCaptureEvent;
	pTooltip->pOldCaptureUser = pOwner->pCaptureUser;
	pTooltip->pPopupWidget = xgeXuiWidgetCreate();
	pTooltip->pLabelWidget = xgeXuiWidgetCreate();
	if ( (pTooltip->pPopupWidget == NULL) || (pTooltip->pLabelWidget == NULL) ) {
		xgeXuiWidgetFree(pTooltip->pPopupWidget);
		xgeXuiWidgetFree(pTooltip->pLabelWidget);
		memset(pTooltip, 0, sizeof(*pTooltip));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetPaddingPx(pTooltip->pPopupWidget, 6.0f, 4.0f, 6.0f, 4.0f);
	xgeXuiPopupInit(&pTooltip->tPopup, pContext, pTooltip->pPopupWidget);
	xgeXuiPopupSetOwner(&pTooltip->tPopup, pOwner);
	xgeXuiPopupSetPlacement(&pTooltip->tPopup, XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT);
	xgeXuiPopupSetZBase(&pTooltip->tPopup, 1200);
	xgeXuiPopupSetAutoClose(&pTooltip->tPopup, 0, 0);
	xgeXuiPopupSetBackground(&pTooltip->tPopup, pTooltip->iBackgroundColor);
	xgeXuiLabelInit(&pTooltip->tLabel, pTooltip->pLabelWidget, pTooltip->pFont, pTooltip->sText);
	xgeXuiLabelSetColor(&pTooltip->tLabel, pTooltip->iTextColor);
	xgeXuiLabelSetAlign(&pTooltip->tLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	xgeXuiWidgetAdd(pTooltip->pPopupWidget, pTooltip->pLabelWidget);
	xgeXuiWidgetAdd(xgeXuiOverlayRoot(pContext), pTooltip->pPopupWidget);
	xgeXuiWidgetSetCaptureEventUser(pOwner, xgeXuiTooltipOwnerEventProc, pTooltip);
	return XGE_OK;
}

void xgeXuiTooltipUnit(xge_xui_tooltip pTooltip)
{
	xge_xui_widget pPopupWidget;

	if ( pTooltip == NULL ) {
		return;
	}
	if ( pTooltip->pOwner != NULL && pTooltip->pOwner->procCaptureEvent == xgeXuiTooltipOwnerEventProc && pTooltip->pOwner->pCaptureUser == pTooltip ) {
		xgeXuiWidgetSetCaptureEventUser(pTooltip->pOwner, pTooltip->procOldCapture, pTooltip->pOldCaptureUser);
	}
	pPopupWidget = pTooltip->pPopupWidget;
	xgeXuiLabelUnit(&pTooltip->tLabel);
	xgeXuiPopupUnit(&pTooltip->tPopup);
	xgeXuiWidgetFree(pPopupWidget);
	memset(pTooltip, 0, sizeof(*pTooltip));
}

void xgeXuiTooltipSetText(xge_xui_tooltip pTooltip, xge_font pFont, const char* sText)
{
	if ( pTooltip == NULL ) {
		return;
	}
	pTooltip->pFont = pFont;
	pTooltip->sText = (sText != NULL) ? sText : "";
	xgeXuiLabelSetFont(&pTooltip->tLabel, pFont);
	xgeXuiLabelSetText(&pTooltip->tLabel, pTooltip->sText);
	if ( xgeXuiTooltipIsOpen(pTooltip) ) {
		__xgeXuiTooltipLayout(pTooltip);
	}
}

void xgeXuiTooltipSetColors(xge_xui_tooltip pTooltip, uint32_t iBackground, uint32_t iText)
{
	if ( pTooltip == NULL ) {
		return;
	}
	pTooltip->iBackgroundColor = iBackground;
	pTooltip->iTextColor = iText;
	xgeXuiPopupSetBackground(&pTooltip->tPopup, iBackground);
	xgeXuiLabelSetColor(&pTooltip->tLabel, iText);
}

void xgeXuiTooltipSetOffset(xge_xui_tooltip pTooltip, float fX, float fY)
{
	if ( pTooltip == NULL ) {
		return;
	}
	pTooltip->fOffsetX = fX;
	pTooltip->fOffsetY = fY;
	if ( xgeXuiTooltipIsOpen(pTooltip) ) {
		__xgeXuiTooltipLayout(pTooltip);
	}
}

void xgeXuiTooltipSetEnabled(xge_xui_tooltip pTooltip, int bEnabled)
{
	if ( pTooltip == NULL ) {
		return;
	}
	pTooltip->bEnabled = bEnabled ? 1 : 0;
	if ( pTooltip->bEnabled == 0 ) {
		xgeXuiTooltipSetOpen(pTooltip, 0);
	}
}

void xgeXuiTooltipSetOpen(xge_xui_tooltip pTooltip, int bOpen)
{
	if ( (pTooltip == NULL) || (pTooltip->pPopupWidget == NULL) ) {
		return;
	}
	bOpen = (bOpen && pTooltip->bEnabled && pTooltip->sText != NULL && pTooltip->sText[0] != 0) ? 1 : 0;
	if ( bOpen ) {
		__xgeXuiTooltipLayout(pTooltip);
	}
	if ( pTooltip->tPopup.bOpen != bOpen ) {
		pTooltip->tPopup.bOpen = bOpen;
		xgeXuiWidgetSetVisible(pTooltip->pPopupWidget, bOpen);
		xgeXuiWidgetMarkPaint(pTooltip->pPopupWidget);
	}
}

int xgeXuiTooltipIsOpen(xge_xui_tooltip pTooltip)
{
	if ( pTooltip == NULL ) {
		return 0;
	}
	return pTooltip->tPopup.bOpen;
}

int xgeXuiTooltipOwnerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	xge_xui_tooltip pTooltip;
	int iResult;

	pTooltip = (xge_xui_tooltip)pUser;
	if ( (pTooltip == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType == XGE_EVENT_XUI_POINTER_ENTER) || (pEvent->iType == XGE_EVENT_MOUSE_MOVE) || (pEvent->iType == XGE_EVENT_TOUCH_MOVE) ) {
		xgeXuiTooltipSetOpen(pTooltip, 1);
	} else if ( (pEvent->iType == XGE_EVENT_XUI_POINTER_LEAVE) || (pEvent->iType == XGE_EVENT_MOUSE_DOWN) || (pEvent->iType == XGE_EVENT_TOUCH_BEGIN) ) {
		xgeXuiTooltipSetOpen(pTooltip, 0);
	}
	iResult = XGE_XUI_EVENT_CONTINUE;
	if ( pTooltip->procOldCapture != NULL ) {
		iResult = pTooltip->procOldCapture(pWidget, pEvent, pTooltip->pOldCaptureUser);
	}
	return iResult;
}
