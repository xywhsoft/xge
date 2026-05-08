int xgeXuiProgressInit(xge_xui_progress pProgress, xge_xui_widget pWidget)
{
	if ( (pProgress == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pProgress, 0, sizeof(*pProgress));
	__xgeXuiControlWidgetInit(pWidget, 0);
	pProgress->pWidget = pWidget;
	pProgress->sText = "";
	pProgress->fMin = 0.0f;
	pProgress->fMax = 1.0f;
	pProgress->fValue = 0.0f;
	pProgress->iColorTrack = XGE_COLOR_RGBA(92, 100, 112, 255);
	pProgress->iColorFill = XGE_COLOR_RGBA(62, 172, 110, 255);
	pProgress->iTextColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pProgress->iTextFlags = XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pWidget->procPaint = xgeXuiProgressPaintProc;
	pWidget->pUser = pProgress;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiProgressUnit(xge_xui_progress pProgress)
{
	if ( pProgress == NULL ) {
		return;
	}
	if ( pProgress->pWidget != NULL && pProgress->pWidget->pUser == pProgress ) {
		pProgress->pWidget->pUser = NULL;
		pProgress->pWidget->procPaint = NULL;
	}
	memset(pProgress, 0, sizeof(*pProgress));
}

void xgeXuiProgressSetRange(xge_xui_progress pProgress, float fMin, float fMax)
{
	if ( pProgress == NULL ) {
		return;
	}
	if ( fMax < fMin ) {
		float fSwap;
		fSwap = fMin;
		fMin = fMax;
		fMax = fSwap;
	}
	if ( fMax == fMin ) {
		fMax = fMin + 1.0f;
	}
	pProgress->fMin = fMin;
	pProgress->fMax = fMax;
	__xgeXuiProgressSetValueInternal(pProgress, pProgress->fValue);
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetValue(xge_xui_progress pProgress, float fValue)
{
	__xgeXuiProgressSetValueInternal(pProgress, fValue);
}

float xgeXuiProgressGetValue(xge_xui_progress pProgress)
{
	if ( pProgress == NULL ) {
		return 0.0f;
	}
	return pProgress->fValue;
}

void xgeXuiProgressSetText(xge_xui_progress pProgress, xge_font pFont, const char* sText)
{
	if ( pProgress == NULL ) {
		return;
	}
	pProgress->pFont = pFont;
	pProgress->sText = (sText != NULL) ? sText : "";
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetTextColor(xge_xui_progress pProgress, uint32_t iColor)
{
	if ( pProgress == NULL ) {
		return;
	}
	pProgress->iTextColor = iColor;
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetColors(xge_xui_progress pProgress, uint32_t iTrack, uint32_t iFill)
{
	if ( pProgress == NULL ) {
		return;
	}
	pProgress->iColorTrack = iTrack;
	pProgress->iColorFill = iFill;
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_progress pProgress;
	xge_rect_t tFill;

	pProgress = (xge_xui_progress)pUser;
	if ( (pWidget == NULL) || (pProgress == NULL) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pProgress->iColorTrack) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tContentRect, pProgress->iColorTrack);
	}
	tFill = pWidget->tContentRect;
	tFill.fW *= __xgeXuiProgressRate(pProgress);
	if ( (tFill.fW > 0.0f) && (XGE_COLOR_GET_A(pProgress->iColorFill) != 0) ) {
		__xgeXuiHostDrawRect(tFill, pProgress->iColorFill);
	}
	if ( (pProgress->pFont != NULL) && (pProgress->sText != NULL) && (pProgress->sText[0] != 0) ) {
		__xgeXuiHostDrawTextRect(pProgress->pFont, pProgress->sText, pWidget->tContentRect, pProgress->iTextColor, pProgress->iTextFlags);
	}
}
