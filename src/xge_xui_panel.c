int xgeXuiPanelInit(xge_xui_panel pPanel, xge_xui_widget pWidget)
{
	if ( (pPanel == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pPanel, 0, sizeof(*pPanel));
	xgeXuiWidgetSetRole(pWidget, XGE_XUI_WIDGET_ROLE_CONTAINER);
	pPanel->pWidget = pWidget;
	pPanel->sTitle = "";
	pPanel->iTitleColor = XGE_COLOR_RGBA(24, 56, 79, 255);
	pPanel->iTitleFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP | XGE_TEXT_CLIP;
	pWidget->procPaint = xgeXuiPanelPaintProc;
	pWidget->pUser = pPanel;
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(249, 253, 255, 255));
	xgeXuiWidgetSetBorder(pWidget, 1.5f, XGE_COLOR_RGBA(127, 196, 229, 255));
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiPanelUnit(xge_xui_panel pPanel)
{
	if ( pPanel == NULL ) {
		return;
	}
	if ( pPanel->pWidget != NULL && pPanel->pWidget->pUser == pPanel ) {
		pPanel->pWidget->pUser = NULL;
		pPanel->pWidget->procPaint = NULL;
	}
	memset(pPanel, 0, sizeof(*pPanel));
}

void xgeXuiPanelSetBackground(xge_xui_panel pPanel, uint32_t iColor)
{
	if ( pPanel == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pPanel->pWidget, iColor);
}

void xgeXuiPanelSetTitle(xge_xui_panel pPanel, xge_font pFont, const char* sTitle)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->pFont = pFont;
	pPanel->sTitle = (sTitle != NULL) ? sTitle : "";
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

void xgeXuiPanelSetTitleColor(xge_xui_panel pPanel, uint32_t iColor)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->iTitleColor = iColor;
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

void xgeXuiPanelSetTitleAlign(xge_xui_panel pPanel, uint32_t iTextFlags)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->iTitleFlags = iTextFlags | XGE_TEXT_CLIP;
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

void xgeXuiPanelSetClip(xge_xui_panel pPanel, int bClip)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->bClip = bClip ? 1 : 0;
	xgeXuiWidgetSetClip(pPanel->pWidget, pPanel->bClip);
}

void xgeXuiPanelPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_panel pPanel;

	pPanel = (xge_xui_panel)pUser;
	if ( (pWidget == NULL) || (pPanel == NULL) ) {
		return;
	}
	if ( (pPanel->pFont != NULL) && (pPanel->sTitle != NULL) && (pPanel->sTitle[0] != 0) ) {
		__xgeXuiHostDrawTextRect(pPanel->pFont, pPanel->sTitle, pWidget->tContentRect, pPanel->iTitleColor, pPanel->iTitleFlags);
	}
}
