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

static uint32_t __xgeXuiLabelTextColor(xge_xui_label pLabel)
{
	if ( (pLabel == NULL) || (pLabel->pWidget == NULL) ) {
		return XGE_COLOR_RGBA(255, 255, 255, 255);
	}
	if ( xgeXuiWidgetIsEnabled(pLabel->pWidget) == 0 ) {
		return pLabel->iDisabledColor;
	}
	return pLabel->iColor;
}

static uint32_t __xgeXuiLabelTextFlags(xge_xui_label pLabel)
{
	uint32_t iFlags;

	iFlags = (pLabel != NULL) ? pLabel->iTextFlags : (XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP | XGE_TEXT_CLIP);
	iFlags |= XGE_TEXT_CLIP;
	if ( (pLabel != NULL) && pLabel->bUnderline ) {
		iFlags |= XGE_TEXT_UNDERLINE;
	}
	return iFlags;
}

static void __xgeXuiLabelInvalidate(xge_xui_label pLabel, int bLayout)
{
	if ( pLabel == NULL ) {
		return;
	}
	if ( bLayout ) {
		pLabel->tMeasuredSize = xgeXuiLabelMeasure(pLabel);
		xgeXuiWidgetMarkLayout(pLabel->pWidget);
	}
	xgeXuiWidgetMarkPaint(pLabel->pWidget);
}

int xgeXuiLabelInit(xge_xui_label pLabel, xge_xui_widget pWidget, xui_font pFont, const char* sText)
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
	pLabel->iDisabledColor = XGE_COLOR_RGBA(160, 166, 174, 255);
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
	__xgeXuiLabelInvalidate(pLabel, 1);
}

void xgeXuiLabelSetFont(xge_xui_label pLabel, xui_font pFont)
{
	if ( pLabel == NULL ) {
		return;
	}
	pLabel->pFont = pFont;
	__xgeXuiLabelInvalidate(pLabel, 1);
}

void xgeXuiLabelSetColor(xge_xui_label pLabel, uint32_t iColor)
{
	if ( pLabel == NULL ) {
		return;
	}
	pLabel->iColor = iColor;
	__xgeXuiLabelInvalidate(pLabel, 0);
}

void xgeXuiLabelSetDisabledColor(xge_xui_label pLabel, uint32_t iColor)
{
	if ( pLabel == NULL ) {
		return;
	}
	pLabel->iDisabledColor = iColor;
	__xgeXuiLabelInvalidate(pLabel, 0);
}

void xgeXuiLabelSetAlign(xge_xui_label pLabel, uint32_t iTextFlags)
{
	if ( pLabel == NULL ) {
		return;
	}
	pLabel->iTextFlags = iTextFlags | XGE_TEXT_CLIP;
	__xgeXuiLabelInvalidate(pLabel, 0);
}

void xgeXuiLabelSetUnderline(xge_xui_label pLabel, int bUnderline)
{
	if ( pLabel == NULL ) {
		return;
	}
	pLabel->bUnderline = bUnderline ? 1 : 0;
	__xgeXuiLabelInvalidate(pLabel, 0);
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

static void __xgeXuiLabelPaintTextDirect(xge_xui_widget pWidget, xge_xui_label pLabel)
{
	if ( (pWidget == NULL) || (pLabel == NULL) || (pLabel->pFont == NULL) || (pLabel->sText == NULL) ) {
		return;
	}
	__xgeXuiHostDrawTextRect(pLabel->pFont, pLabel->sText, pWidget->tContentRect, __xgeXuiLabelTextColor(pLabel), __xgeXuiLabelTextFlags(pLabel));
}

void xgeXuiLabelPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_label pLabel;

	pLabel = (xge_xui_label)pUser;
	if ( (pWidget == NULL) || (pLabel == NULL) || (pLabel->pFont == NULL) || (pLabel->sText == NULL) ) {
		return;
	}
	__xgeXuiLabelPaintTextDirect(pWidget, pLabel);
}
