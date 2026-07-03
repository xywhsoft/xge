int xgeXuiSeparatorInit(xge_xui_separator pSeparator, xge_xui_widget pWidget)
{
	if ( (pSeparator == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pSeparator, 0, sizeof(*pSeparator));
	__xgeXuiControlWidgetInit(pWidget, 0);
	pSeparator->pWidget = pWidget;
	pSeparator->iColor = XGE_COLOR_RGBA(90, 104, 122, 180);
	pSeparator->fThickness = 1.0f;
	pSeparator->iOrientation = XGE_XUI_SEPARATOR_HORIZONTAL;
	pSeparator->iAlign = XGE_XUI_ALIGN_CENTER;
	pSeparator->iLineStyle = XGE_XUI_SEPARATOR_SOLID;
	pWidget->procMeasure = xgeXuiSeparatorMeasureProc;
	pWidget->procPaint = xgeXuiSeparatorPaintProc;
	pWidget->pUser = pSeparator;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiSeparatorUnit(xge_xui_separator pSeparator)
{
	if ( pSeparator == NULL ) {
		return;
	}
	if ( pSeparator->pWidget != NULL && pSeparator->pWidget->pUser == pSeparator ) {
		pSeparator->pWidget->pUser = NULL;
		pSeparator->pWidget->procMeasure = NULL;
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
	if ( fThickness < 0.0f ) {
		fThickness = 0.0f;
	}
	pSeparator->fThickness = fThickness;
	xgeXuiWidgetMarkLayout(pSeparator->pWidget);
	xgeXuiWidgetMarkPaint(pSeparator->pWidget);
}

void xgeXuiSeparatorSetOrientation(xge_xui_separator pSeparator, int iOrientation)
{
	if ( pSeparator == NULL ) {
		return;
	}
	pSeparator->iOrientation = (iOrientation == XGE_XUI_SEPARATOR_VERTICAL) ? XGE_XUI_SEPARATOR_VERTICAL : XGE_XUI_SEPARATOR_HORIZONTAL;
	xgeXuiWidgetMarkLayout(pSeparator->pWidget);
	xgeXuiWidgetMarkPaint(pSeparator->pWidget);
}

static int __xgeXuiSeparatorClampAlign(int iAlign)
{
	if ( iAlign == XGE_XUI_ALIGN_CENTER || iAlign == XGE_XUI_ALIGN_END ) {
		return iAlign;
	}
	return XGE_XUI_ALIGN_START;
}

void xgeXuiSeparatorSetAlign(xge_xui_separator pSeparator, int iAlign)
{
	if ( pSeparator == NULL ) {
		return;
	}
	pSeparator->iAlign = __xgeXuiSeparatorClampAlign(iAlign);
	xgeXuiWidgetMarkPaint(pSeparator->pWidget);
}

static int __xgeXuiSeparatorClampLineStyle(int iLineStyle)
{
	if ( iLineStyle == XGE_XUI_SEPARATOR_DOT || iLineStyle == XGE_XUI_SEPARATOR_DASH || iLineStyle == XGE_XUI_SEPARATOR_DASH_DOT ) {
		return iLineStyle;
	}
	return XGE_XUI_SEPARATOR_SOLID;
}

void xgeXuiSeparatorSetLineStyle(xge_xui_separator pSeparator, int iLineStyle)
{
	if ( pSeparator == NULL ) {
		return;
	}
	pSeparator->iLineStyle = __xgeXuiSeparatorClampLineStyle(iLineStyle);
	xgeXuiWidgetMarkPaint(pSeparator->pWidget);
}

xge_vec2_t xgeXuiSeparatorMeasureProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_separator pSeparator;
	xge_vec2_t tSize;

	(void)pWidget;
	pSeparator = (xge_xui_separator)pUser;
	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( pSeparator == NULL ) {
		return tSize;
	}
	if ( pSeparator->iOrientation == XGE_XUI_SEPARATOR_VERTICAL ) {
		tSize.fX = pSeparator->fThickness;
	} else {
		tSize.fY = pSeparator->fThickness;
	}
	return tSize;
}

static xge_rect_t __xgeXuiSeparatorLineRect(xge_xui_separator pSeparator, xge_rect_t tContent)
{
	xge_rect_t tRect;
	float fThickness;

	tRect = tContent;
	fThickness = pSeparator->fThickness;
	if ( pSeparator->iOrientation == XGE_XUI_SEPARATOR_VERTICAL ) {
		if ( fThickness > tRect.fW ) {
			fThickness = tRect.fW;
		}
		if ( pSeparator->iAlign == XGE_XUI_ALIGN_CENTER ) {
			tRect.fX += (tRect.fW - fThickness) * 0.5f;
		} else if ( pSeparator->iAlign == XGE_XUI_ALIGN_END ) {
			tRect.fX += tRect.fW - fThickness;
		}
		tRect.fW = fThickness;
	} else {
		if ( fThickness > tRect.fH ) {
			fThickness = tRect.fH;
		}
		if ( pSeparator->iAlign == XGE_XUI_ALIGN_CENTER ) {
			tRect.fY += (tRect.fH - fThickness) * 0.5f;
		} else if ( pSeparator->iAlign == XGE_XUI_ALIGN_END ) {
			tRect.fY += tRect.fH - fThickness;
		}
		tRect.fH = fThickness;
	}
	return tRect;
}

static float __xgeXuiSeparatorMaxFloat(float a, float b)
{
	return (a > b) ? a : b;
}

static void __xgeXuiSeparatorDrawSegment(xge_rect_t tLine, int iVertical, float fStart, float fLength, uint32_t iColor)
{
	xge_rect_t tPart;
	float fLimit;

	if ( fLength <= 0.0f ) {
		return;
	}
	tPart = tLine;
	if ( iVertical ) {
		fLimit = tLine.fY + tLine.fH;
		tPart.fY = tLine.fY + fStart;
		if ( tPart.fY >= fLimit ) {
			return;
		}
		tPart.fH = fLength;
		if ( (tPart.fY + tPart.fH) > fLimit ) {
			tPart.fH = fLimit - tPart.fY;
		}
	} else {
		fLimit = tLine.fX + tLine.fW;
		tPart.fX = tLine.fX + fStart;
		if ( tPart.fX >= fLimit ) {
			return;
		}
		tPart.fW = fLength;
		if ( (tPart.fX + tPart.fW) > fLimit ) {
			tPart.fW = fLimit - tPart.fX;
		}
	}
	if ( (tPart.fW > 0.0f) && (tPart.fH > 0.0f) ) {
		__xgeXuiHostDrawRect(tPart, iColor);
	}
}

static void __xgeXuiSeparatorDrawPattern(xge_xui_separator pSeparator, xge_rect_t tLine)
{
	float fThickness;
	float fCursor;
	float fMain;
	float fDash;
	float fDot;
	float fGap;
	int bVertical;

	bVertical = (pSeparator->iOrientation == XGE_XUI_SEPARATOR_VERTICAL);
	fMain = bVertical ? tLine.fH : tLine.fW;
	if ( pSeparator->iLineStyle == XGE_XUI_SEPARATOR_SOLID ) {
		__xgeXuiHostDrawRect(tLine, pSeparator->iColor);
		return;
	}
	fThickness = __xgeXuiSeparatorMaxFloat(pSeparator->fThickness, 1.0f);
	fDot = fThickness;
	fDash = fThickness * 4.0f;
	fGap = fThickness * 2.0f;
	fCursor = 0.0f;
	while ( fCursor < fMain ) {
		if ( pSeparator->iLineStyle == XGE_XUI_SEPARATOR_DOT ) {
			__xgeXuiSeparatorDrawSegment(tLine, bVertical, fCursor, fDot, pSeparator->iColor);
			fCursor += fDot + fThickness;
		} else if ( pSeparator->iLineStyle == XGE_XUI_SEPARATOR_DASH ) {
			__xgeXuiSeparatorDrawSegment(tLine, bVertical, fCursor, fDash, pSeparator->iColor);
			fCursor += fDash + fGap;
		} else {
			__xgeXuiSeparatorDrawSegment(tLine, bVertical, fCursor, fDash, pSeparator->iColor);
			fCursor += fDash + fGap;
			__xgeXuiSeparatorDrawSegment(tLine, bVertical, fCursor, fDot, pSeparator->iColor);
			fCursor += fDot + fGap;
		}
	}
}

void xgeXuiSeparatorPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_separator pSeparator;
	xge_rect_t tRect;

	pSeparator = (xge_xui_separator)pUser;
	if ( (pWidget == NULL) || (pSeparator == NULL) || (XGE_COLOR_GET_A(pSeparator->iColor) == 0) || (pSeparator->fThickness <= 0.0f) ) {
		return;
	}
	tRect = __xgeXuiSeparatorLineRect(pSeparator, pWidget->tContentRect);
	if ( (tRect.fW > 0.0f) && (tRect.fH > 0.0f) ) {
		__xgeXuiSeparatorDrawPattern(pSeparator, tRect);
	}
}
