static void __xgeXuiPanelMarkLayout(xge_xui_panel pPanel)
{
	if ( (pPanel == NULL) || (pPanel->pWidget == NULL) ) {
		return;
	}
	xgeXuiWidgetMarkLayout(pPanel->pWidget);
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

int xgeXuiPanelInit(xge_xui_panel pPanel, xge_xui_widget pWidget)
{
	xge_xui_widget pHeader;
	xge_xui_widget pIcon;
	xge_xui_widget pTitle;
	xge_xui_widget pClient;

	if ( (pPanel == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pPanel, 0, sizeof(*pPanel));
	pHeader = xgeXuiWidgetCreate();
	pIcon = xgeXuiWidgetCreate();
	pTitle = xgeXuiWidgetCreate();
	pClient = xgeXuiWidgetCreate();
	if ( (pHeader == NULL) || (pIcon == NULL) || (pTitle == NULL) || (pClient == NULL) ) {
		xgeXuiWidgetFree(pHeader);
		xgeXuiWidgetFree(pIcon);
		xgeXuiWidgetFree(pTitle);
		xgeXuiWidgetFree(pClient);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRole(pWidget, XGE_XUI_WIDGET_ROLE_CONTAINER);
	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_COLUMN);
	pPanel->pWidget = pWidget;
	pPanel->pHeaderWidget = pHeader;
	pPanel->pIconWidget = pIcon;
	pPanel->pTitleWidget = pTitle;
	pPanel->pClientWidget = pClient;
	pPanel->pFont = NULL;
	pPanel->iTitleColor = XGE_COLOR_RGBA(54, 84, 108, 255);
	pPanel->iTitleFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pPanel->iHeaderColor = XGE_COLOR_RGBA(232, 243, 251, 255);
	pPanel->iClientColor = XGE_COLOR_RGBA(248, 252, 255, 255);
	pPanel->fHeaderHeight = 28.0f;
	pPanel->fIconSize = 16.0f;
	pPanel->fHeaderGap = 6.0f;

	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(127, 196, 229, 255));
	xgeXuiWidgetSetRadius(pWidget, 0.0f);
	xgeXuiWidgetSetPaddingPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetGap(pWidget, 0.0f);
	pWidget->pUser = pPanel;

	xgeXuiWidgetSetRole(pHeader, XGE_XUI_WIDGET_ROLE_CONTAINER);
	xgeXuiWidgetSetLayout(pHeader, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetSize(pHeader, xgeXuiSizePercent(100.0f), xgeXuiSizePx(pPanel->fHeaderHeight));
	xgeXuiWidgetSetPaddingPx(pHeader, 8.0f, 0.0f, 8.0f, 0.0f);
	xgeXuiWidgetSetGap(pHeader, pPanel->fHeaderGap);
	xgeXuiWidgetSetAlign(pHeader, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_CENTER);
	xgeXuiWidgetSetBackground(pHeader, pPanel->iHeaderColor);

	xgeXuiImageInit(&pPanel->tIconImage, pIcon, NULL);
	xgeXuiWidgetSetSize(pIcon, xgeXuiSizePx(pPanel->fIconSize), xgeXuiSizePx(pPanel->fIconSize));
	xgeXuiWidgetSetAlign(pIcon, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER);
	xgeXuiWidgetSetVisible(pIcon, 0);

	xgeXuiLabelInit(&pPanel->tTitleLabel, pTitle, NULL, "");
	xgeXuiWidgetSetSize(pTitle, xgeXuiSizeGrow(1.0f), xgeXuiSizePercent(100.0f));
	xgeXuiLabelSetColor(&pPanel->tTitleLabel, pPanel->iTitleColor);
	xgeXuiLabelSetAlign(&pPanel->tTitleLabel, pPanel->iTitleFlags);
	pPanel->sTitle = pPanel->tTitleLabel.sText;

	xgeXuiWidgetSetRole(pClient, XGE_XUI_WIDGET_ROLE_CONTAINER);
	xgeXuiWidgetSetLayout(pClient, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pClient, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetPaddingPx(pClient, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetSetGap(pClient, 6.0f);
	xgeXuiWidgetSetBackground(pClient, pPanel->iClientColor);
	xgeXuiWidgetSetClip(pClient, 1);

	if ( (xgeXuiWidgetAddInternal(pWidget, pHeader) != XGE_OK) ||
	     (xgeXuiWidgetAddInternal(pHeader, pIcon) != XGE_OK) ||
	     (xgeXuiWidgetAddInternal(pHeader, pTitle) != XGE_OK) ||
	     (xgeXuiWidgetAddInternal(pWidget, pClient) != XGE_OK) ) {
		xgeXuiPanelUnit(pPanel);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiPanelUnit(xge_xui_panel pPanel)
{
	if ( pPanel == NULL ) {
		return;
	}
	xgeXuiImageUnit(&pPanel->tIconImage);
	xgeXuiLabelUnit(&pPanel->tTitleLabel);
	xgeXuiWidgetFree(pPanel->pHeaderWidget);
	xgeXuiWidgetFree(pPanel->pClientWidget);
	if ( pPanel->pWidget != NULL && pPanel->pWidget->pUser == pPanel ) {
		pPanel->pWidget->pUser = NULL;
	}
	memset(pPanel, 0, sizeof(*pPanel));
}

xge_xui_widget xgeXuiPanelGetHeaderWidget(xge_xui_panel pPanel)
{
	return (pPanel != NULL) ? pPanel->pHeaderWidget : NULL;
}

xge_xui_widget xgeXuiPanelGetIconWidget(xge_xui_panel pPanel)
{
	return (pPanel != NULL) ? pPanel->pIconWidget : NULL;
}

xge_xui_widget xgeXuiPanelGetTitleWidget(xge_xui_panel pPanel)
{
	return (pPanel != NULL) ? pPanel->pTitleWidget : NULL;
}

xge_xui_widget xgeXuiPanelGetClientWidget(xge_xui_panel pPanel)
{
	return (pPanel != NULL) ? pPanel->pClientWidget : NULL;
}

void xgeXuiPanelSetBackground(xge_xui_panel pPanel, uint32_t iColor)
{
	if ( (pPanel == NULL) || (pPanel->pWidget == NULL) ) {
		return;
	}
	xgeXuiWidgetSetBackground(pPanel->pWidget, iColor);
}

void xgeXuiPanelSetHeaderColor(xge_xui_panel pPanel, uint32_t iColor)
{
	if ( (pPanel == NULL) || (pPanel->pHeaderWidget == NULL) ) {
		return;
	}
	pPanel->iHeaderColor = iColor;
	xgeXuiWidgetSetBackground(pPanel->pHeaderWidget, iColor);
}

void xgeXuiPanelSetClientColor(xge_xui_panel pPanel, uint32_t iColor)
{
	if ( (pPanel == NULL) || (pPanel->pClientWidget == NULL) ) {
		return;
	}
	pPanel->iClientColor = iColor;
	xgeXuiWidgetSetBackground(pPanel->pClientWidget, iColor);
}

void xgeXuiPanelSetHeaderHeight(xge_xui_panel pPanel, float fHeight)
{
	if ( (pPanel == NULL) || (pPanel->pHeaderWidget == NULL) ) {
		return;
	}
	if ( fHeight < 0.0f ) {
		fHeight = 0.0f;
	}
	pPanel->fHeaderHeight = fHeight;
	xgeXuiWidgetSetSize(pPanel->pHeaderWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(fHeight));
	xgeXuiWidgetSetVisible(pPanel->pHeaderWidget, fHeight > 0.0f);
	__xgeXuiPanelMarkLayout(pPanel);
}

void xgeXuiPanelSetIconSize(xge_xui_panel pPanel, float fSize)
{
	if ( (pPanel == NULL) || (pPanel->pIconWidget == NULL) ) {
		return;
	}
	if ( fSize < 1.0f ) {
		fSize = 1.0f;
	}
	pPanel->fIconSize = fSize;
	xgeXuiWidgetSetSize(pPanel->pIconWidget, xgeXuiSizePx(fSize), xgeXuiSizePx(fSize));
	__xgeXuiPanelMarkLayout(pPanel);
}

void xgeXuiPanelSetIcon(xge_xui_panel pPanel, xui_texture pTexture, xge_rect_t tSrc)
{
	if ( pPanel == NULL ) {
		return;
	}
	xgeXuiImageSetTexture(&pPanel->tIconImage, pTexture);
	xgeXuiImageSetSource(&pPanel->tIconImage, tSrc);
	xgeXuiWidgetSetVisible(pPanel->pIconWidget, pTexture != NULL);
	__xgeXuiPanelMarkLayout(pPanel);
}

void xgeXuiPanelSetTitle(xge_xui_panel pPanel, xui_font pFont, const char* sTitle)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->pFont = pFont;
	xgeXuiLabelSetFont(&pPanel->tTitleLabel, pFont);
	xgeXuiLabelSetText(&pPanel->tTitleLabel, (sTitle != NULL) ? sTitle : "");
	pPanel->sTitle = pPanel->tTitleLabel.sText;
	__xgeXuiPanelMarkLayout(pPanel);
}

void xgeXuiPanelSetTitleColor(xge_xui_panel pPanel, uint32_t iColor)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->iTitleColor = iColor;
	xgeXuiLabelSetColor(&pPanel->tTitleLabel, iColor);
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

void xgeXuiPanelSetTitleAlign(xge_xui_panel pPanel, uint32_t iTextFlags)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->iTitleFlags = iTextFlags | XGE_TEXT_CLIP;
	xgeXuiLabelSetAlign(&pPanel->tTitleLabel, pPanel->iTitleFlags);
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

void xgeXuiPanelSetClip(xge_xui_panel pPanel, int bClip)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->bClip = bClip ? 1 : 0;
	xgeXuiWidgetSetClip(pPanel->pClientWidget, pPanel->bClip);
}

void xgeXuiPanelPaintProc(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
}
