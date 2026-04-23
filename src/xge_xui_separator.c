int xgeXuiSeparatorInit(xge_xui_separator pSeparator, xge_xui_widget pWidget)
{
	if ( (pSeparator == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pSeparator, 0, sizeof(*pSeparator));
	pSeparator->pWidget = pWidget;
	pSeparator->iColor = XGE_COLOR_RGBA(90, 104, 122, 180);
	pSeparator->fThickness = 1.0f;
	pSeparator->iOrientation = XGE_XUI_SEPARATOR_HORIZONTAL;
	pWidget->procPaint = xgeXuiSeparatorPaintProc;
	pWidget->pUser = pSeparator;
	return XGE_OK;
}

void xgeXuiSeparatorUnit(xge_xui_separator pSeparator)
{
	if ( pSeparator == NULL ) {
		return;
	}
	if ( pSeparator->pWidget != NULL && pSeparator->pWidget->pUser == pSeparator ) {
		pSeparator->pWidget->pUser = NULL;
		pSeparator->pWidget->procPaint = NULL;
	}
	memset(pSeparator, 0, sizeof(*pSeparator));
}

void xgeXuiSeparatorSetColor(xge_xui_separator pSeparator, uint32_t iColor)
{
	if ( pSeparator == NULL ) {
		return;
	}
	pSeparator->iColor = iColor;
	xgeXuiWidgetMarkPaint(pSeparator->pWidget);
}

void xgeXuiSeparatorSetThickness(xge_xui_separator pSeparator, float fThickness)
{
	if ( pSeparator == NULL ) {
		return;
	}
	if ( fThickness < 1.0f ) {
		fThickness = 1.0f;
	}
	pSeparator->fThickness = fThickness;
	xgeXuiWidgetMarkPaint(pSeparator->pWidget);
}

void xgeXuiSeparatorSetOrientation(xge_xui_separator pSeparator, int iOrientation)
{
	if ( pSeparator == NULL ) {
		return;
	}
	pSeparator->iOrientation = (iOrientation == XGE_XUI_SEPARATOR_VERTICAL) ? XGE_XUI_SEPARATOR_VERTICAL : XGE_XUI_SEPARATOR_HORIZONTAL;
	xgeXuiWidgetMarkPaint(pSeparator->pWidget);
}

void xgeXuiSeparatorPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_separator pSeparator;
	xge_rect_t tRect;
	float fThickness;

	pSeparator = (xge_xui_separator)pUser;
	if ( (pWidget == NULL) || (pSeparator == NULL) || (XGE_COLOR_GET_A(pSeparator->iColor) == 0) ) {
		return;
	}
	tRect = pWidget->tContentRect;
	fThickness = pSeparator->fThickness;
	if ( pSeparator->iOrientation == XGE_XUI_SEPARATOR_VERTICAL ) {
		if ( fThickness > tRect.fW ) {
			fThickness = tRect.fW;
		}
		tRect.fX += (tRect.fW - fThickness) * 0.5f;
		tRect.fW = fThickness;
	} else {
		if ( fThickness > tRect.fH ) {
			fThickness = tRect.fH;
		}
		tRect.fY += (tRect.fH - fThickness) * 0.5f;
		tRect.fH = fThickness;
	}
	if ( (tRect.fW > 0.0f) && (tRect.fH > 0.0f) ) {
		__xgeXuiHostDrawRect(tRect, pSeparator->iColor);
	}
}
