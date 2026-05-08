static int __xgeXuiLabelTextSet(xge_xui_label pLabel, const char* sText)
{
	int iNeed;
	char* sNew;

	if ( pLabel == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( sText == NULL ) {
		sText = "";
	}
	iNeed = (int)strlen(sText) + 1;
	if ( iNeed > pLabel->iTextCapacity ) {
		sNew = (char*)xrtRealloc(pLabel->sTextOwned, (size_t)iNeed);
		if ( sNew == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pLabel->sTextOwned = sNew;
		pLabel->iTextCapacity = iNeed;
	}
	memcpy(pLabel->sTextOwned, sText, (size_t)iNeed);
	pLabel->sText = pLabel->sTextOwned;
	return XGE_OK;
}

int xgeXuiLabelInit(xge_xui_label pLabel, xge_xui_widget pWidget, xge_font pFont, const char* sText)
{
	if ( (pLabel == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pLabel, 0, sizeof(*pLabel));
	__xgeXuiControlWidgetInit(pWidget, 0);
	pLabel->pWidget = pWidget;
	pLabel->pFont = pFont;
	if ( __xgeXuiLabelTextSet(pLabel, sText) != XGE_OK ) {
		memset(pLabel, 0, sizeof(*pLabel));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
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
	if ( pLabel->sTextOwned != NULL ) {
		xrtFree(pLabel->sTextOwned);
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
	if ( __xgeXuiLabelTextSet(pLabel, sText) != XGE_OK ) {
		return;
	}
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
