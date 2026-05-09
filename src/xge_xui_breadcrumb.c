static uint32_t __xgeXuiBreadcrumbHoverColor(uint32_t iSegment)
{
	int iR = (int)XGE_COLOR_GET_R(iSegment) + 15;
	int iG = (int)XGE_COLOR_GET_G(iSegment) + 15;
	int iB = (int)XGE_COLOR_GET_B(iSegment) + 15;
	return XGE_COLOR_RGBA(iR > 255 ? 255 : iR, iG > 255 ? 255 : iG, iB > 255 ? 255 : iB, XGE_COLOR_GET_A(iSegment));
}

static xge_rect_t __xgeXuiBreadcrumbContentRect(xge_xui_breadcrumb pBreadcrumb)
{
	xge_rect_t tRect;

	tRect = pBreadcrumb->pWidget->tContentRect;
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		tRect = pBreadcrumb->pWidget->tRect;
	}
	return tRect;
}

static float __xgeXuiBreadcrumbSegmentWidth(xge_xui_breadcrumb pBreadcrumb, const char* sText)
{
	size_t n;
	float fWidth;

	(void)pBreadcrumb;
	n = (sText != NULL) ? strlen(sText) : 0;
	fWidth = (float)n * 7.0f + pBreadcrumb->fSegmentPaddingX * 2.0f;
	return (fWidth < 28.0f) ? 28.0f : fWidth;
}

static void __xgeXuiBreadcrumbLayout(xge_xui_breadcrumb pBreadcrumb)
{
	xge_rect_t tContent;
	float fTotal;
	float fUsed;
	float fX;
	float fW;
	float fEllipsis;
	int i;
	int iFirst;

	if ( (pBreadcrumb == NULL) || (pBreadcrumb->pWidget == NULL) ) {
		return;
	}
	for ( i = 0; i < pBreadcrumb->iSegmentCount; i++ ) {
		memset(&pBreadcrumb->arrSegments[i].tRect, 0, sizeof(pBreadcrumb->arrSegments[i].tRect));
	}
	tContent = __xgeXuiBreadcrumbContentRect(pBreadcrumb);
	fTotal = 0.0f;
	for ( i = 0; i < pBreadcrumb->iSegmentCount; i++ ) {
		fTotal += __xgeXuiBreadcrumbSegmentWidth(pBreadcrumb, pBreadcrumb->arrSegments[i].sText);
		if ( i + 1 < pBreadcrumb->iSegmentCount ) {
			fTotal += pBreadcrumb->fSeparatorWidth;
		}
	}
	iFirst = 0;
	fEllipsis = 30.0f;
	if ( fTotal > tContent.fW && pBreadcrumb->iSegmentCount > 1 ) {
		for ( iFirst = 1; iFirst < pBreadcrumb->iSegmentCount; iFirst++ ) {
			fUsed = fEllipsis + pBreadcrumb->fSeparatorWidth;
			for ( i = iFirst; i < pBreadcrumb->iSegmentCount; i++ ) {
				fUsed += __xgeXuiBreadcrumbSegmentWidth(pBreadcrumb, pBreadcrumb->arrSegments[i].sText);
				if ( i + 1 < pBreadcrumb->iSegmentCount ) {
					fUsed += pBreadcrumb->fSeparatorWidth;
				}
			}
			if ( fUsed <= tContent.fW || iFirst == pBreadcrumb->iSegmentCount - 1 ) {
				break;
			}
		}
	}
	pBreadcrumb->iFirstVisible = iFirst;
	pBreadcrumb->iCollapsedCount = iFirst;
	fX = tContent.fX;
	if ( iFirst > 0 ) {
		fX += fEllipsis + pBreadcrumb->fSeparatorWidth;
	}
	for ( i = iFirst; i < pBreadcrumb->iSegmentCount; i++ ) {
		fW = __xgeXuiBreadcrumbSegmentWidth(pBreadcrumb, pBreadcrumb->arrSegments[i].sText);
		if ( fX + fW > tContent.fX + tContent.fW ) {
			fW = (tContent.fX + tContent.fW) - fX;
		}
		if ( fW < 0.0f ) {
			fW = 0.0f;
		}
		pBreadcrumb->arrSegments[i].tRect = (xge_rect_t){ fX, tContent.fY, fW, tContent.fH };
		fX += fW + pBreadcrumb->fSeparatorWidth;
	}
}

static int __xgeXuiBreadcrumbSegmentAt(xge_xui_breadcrumb pBreadcrumb, float fX, float fY)
{
	int i;

	if ( pBreadcrumb == NULL ) {
		return -1;
	}
	__xgeXuiBreadcrumbLayout(pBreadcrumb);
	for ( i = pBreadcrumb->iFirstVisible; i < pBreadcrumb->iSegmentCount; i++ ) {
		if ( __xgeXuiRectContains(pBreadcrumb->arrSegments[i].tRect, fX, fY) ) {
			return i;
		}
	}
	return -1;
}

int xgeXuiBreadcrumbInit(xge_xui_breadcrumb pBreadcrumb, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pBreadcrumb == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBreadcrumb, 0, sizeof(*pBreadcrumb));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pBreadcrumb->pContext = pContext;
	pBreadcrumb->pWidget = pWidget;
	pBreadcrumb->iHover = -1;
	pBreadcrumb->iSelected = -1;
	pBreadcrumb->fSegmentPaddingX = 10.0f;
	pBreadcrumb->fSeparatorWidth = 16.0f;
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(236, 246, 253, 255));
	pBreadcrumb->iSegmentColor = XGE_COLOR_RGBA(247, 252, 255, 255);
	pBreadcrumb->iHoverColor = XGE_COLOR_RGBA(224, 241, 254, 255);
	pBreadcrumb->iSelectedColor = XGE_COLOR_RGBA(196, 224, 248, 255);
	pBreadcrumb->iBorderColor = XGE_COLOR_RGBA(143, 184, 214, 255);
	pBreadcrumb->iTextColor = XGE_COLOR_RGBA(31, 58, 82, 255);
	pBreadcrumb->iSeparatorColor = XGE_COLOR_RGBA(77, 129, 171, 255);
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiBreadcrumbEventProc, NULL);
	pWidget->procPaint = xgeXuiBreadcrumbPaintProc;
	pWidget->pUser = pBreadcrumb;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiBreadcrumbUnit(xge_xui_breadcrumb pBreadcrumb)
{
	if ( pBreadcrumb == NULL ) {
		return;
	}
	if ( pBreadcrumb->pWidget != NULL && pBreadcrumb->pWidget->pUser == pBreadcrumb ) {
		pBreadcrumb->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pBreadcrumb->pWidget, NULL, NULL);
		pBreadcrumb->pWidget->procPaint = NULL;
	}
	memset(pBreadcrumb, 0, sizeof(*pBreadcrumb));
}

void xgeXuiBreadcrumbClear(xge_xui_breadcrumb pBreadcrumb)
{
	if ( pBreadcrumb == NULL ) {
		return;
	}
	pBreadcrumb->iSegmentCount = 0;
	pBreadcrumb->iFirstVisible = 0;
	pBreadcrumb->iCollapsedCount = 0;
	pBreadcrumb->iHover = -1;
	pBreadcrumb->iSelected = -1;
	xgeXuiWidgetMarkPaint(pBreadcrumb->pWidget);
}

int xgeXuiBreadcrumbAddSegment(xge_xui_breadcrumb pBreadcrumb, const char* sText, int iId)
{
	xge_xui_breadcrumb_segment_t* pSegment;

	if ( (pBreadcrumb == NULL) || (pBreadcrumb->iSegmentCount >= XGE_XUI_BREADCRUMB_CAPACITY) ) {
		return -1;
	}
	pSegment = &pBreadcrumb->arrSegments[pBreadcrumb->iSegmentCount];
	memset(pSegment, 0, sizeof(*pSegment));
	pSegment->sText = sText;
	pSegment->iId = iId;
	pBreadcrumb->iSegmentCount++;
	__xgeXuiBreadcrumbLayout(pBreadcrumb);
	xgeXuiWidgetMarkPaint(pBreadcrumb->pWidget);
	return pBreadcrumb->iSegmentCount - 1;
}

int xgeXuiBreadcrumbGetSegmentCount(xge_xui_breadcrumb pBreadcrumb)
{
	return (pBreadcrumb != NULL) ? pBreadcrumb->iSegmentCount : 0;
}

int xgeXuiBreadcrumbGetFirstVisible(xge_xui_breadcrumb pBreadcrumb)
{
	if ( pBreadcrumb == NULL ) {
		return 0;
	}
	__xgeXuiBreadcrumbLayout(pBreadcrumb);
	return pBreadcrumb->iFirstVisible;
}

int xgeXuiBreadcrumbGetCollapsedCount(xge_xui_breadcrumb pBreadcrumb)
{
	if ( pBreadcrumb == NULL ) {
		return 0;
	}
	__xgeXuiBreadcrumbLayout(pBreadcrumb);
	return pBreadcrumb->iCollapsedCount;
}

int xgeXuiBreadcrumbGetSelected(xge_xui_breadcrumb pBreadcrumb)
{
	return (pBreadcrumb != NULL) ? pBreadcrumb->iSelected : -1;
}

void xgeXuiBreadcrumbSetSelected(xge_xui_breadcrumb pBreadcrumb, int iIndex)
{
	if ( pBreadcrumb == NULL ) {
		return;
	}
	if ( (iIndex < 0) || (iIndex >= pBreadcrumb->iSegmentCount) ) {
		iIndex = -1;
	}
	pBreadcrumb->iSelected = iIndex;
	xgeXuiWidgetMarkPaint(pBreadcrumb->pWidget);
}

void xgeXuiBreadcrumbSetFont(xge_xui_breadcrumb pBreadcrumb, xge_font pFont)
{
	if ( pBreadcrumb != NULL ) {
		pBreadcrumb->pFont = pFont;
		xgeXuiWidgetMarkPaint(pBreadcrumb->pWidget);
	}
}

void xgeXuiBreadcrumbSetMetrics(xge_xui_breadcrumb pBreadcrumb, float fSegmentPaddingX, float fSeparatorWidth)
{
	if ( pBreadcrumb == NULL ) {
		return;
	}
	pBreadcrumb->fSegmentPaddingX = (fSegmentPaddingX < 2.0f) ? 2.0f : fSegmentPaddingX;
	pBreadcrumb->fSeparatorWidth = (fSeparatorWidth < 6.0f) ? 6.0f : fSeparatorWidth;
	__xgeXuiBreadcrumbLayout(pBreadcrumb);
	xgeXuiWidgetMarkPaint(pBreadcrumb->pWidget);
}

void xgeXuiBreadcrumbSetSelect(xge_xui_breadcrumb pBreadcrumb, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pBreadcrumb != NULL ) {
		pBreadcrumb->procSelect = procSelect;
		pBreadcrumb->pUser = pUser;
	}
}

void xgeXuiBreadcrumbSetColors(xge_xui_breadcrumb pBreadcrumb, uint32_t iBackground, uint32_t iSegment, uint32_t iSelected, uint32_t iBorder, uint32_t iText, uint32_t iSeparator)
{
	if ( pBreadcrumb == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pBreadcrumb->pWidget, iBackground);
	pBreadcrumb->iSegmentColor = iSegment;
	pBreadcrumb->iHoverColor = __xgeXuiBreadcrumbHoverColor(iSegment);
	pBreadcrumb->iSelectedColor = iSelected;
	pBreadcrumb->iBorderColor = iBorder;
	pBreadcrumb->iTextColor = iText;
	pBreadcrumb->iSeparatorColor = iSeparator;
	xgeXuiWidgetMarkPaint(pBreadcrumb->pWidget);
}

int xgeXuiBreadcrumbEvent(xge_xui_breadcrumb pBreadcrumb, const xge_event_t* pEvent)
{
	int iInside;
	int iSegment;

	if ( (pBreadcrumb == NULL) || (pBreadcrumb->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pBreadcrumb->pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
			iSegment = iInside ? __xgeXuiBreadcrumbSegmentAt(pBreadcrumb, pEvent->fX, pEvent->fY) : -1;
			if ( pBreadcrumb->iHover != iSegment ) {
				pBreadcrumb->iHover = iSegment;
				pBreadcrumb->iState = (iSegment >= 0) ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
				xgeXuiWidgetMarkPaint(pBreadcrumb->pWidget);
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pBreadcrumb->pContext, pBreadcrumb->pWidget);
			iSegment = __xgeXuiBreadcrumbSegmentAt(pBreadcrumb, pEvent->fX, pEvent->fY);
			if ( iSegment < 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiBreadcrumbSetSelected(pBreadcrumb, iSegment);
			pBreadcrumb->iSelectCount++;
			if ( pBreadcrumb->procSelect != NULL ) {
				pBreadcrumb->procSelect(pBreadcrumb->pWidget, iSegment, pBreadcrumb->pUser);
			}
			return XGE_XUI_EVENT_CONSUMED;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiBreadcrumbEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiBreadcrumbEvent((xge_xui_breadcrumb)pUser, pEvent);
}

void xgeXuiBreadcrumbPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_breadcrumb pBreadcrumb;
	xge_rect_t tContent;
	xge_rect_t tSegment;
	xge_rect_t tSeparator;
	uint32_t iColor;
	int i;

	pBreadcrumb = (xge_xui_breadcrumb)pUser;
	if ( (pWidget == NULL) || (pBreadcrumb == NULL) ) {
		return;
	}
	__xgeXuiBreadcrumbLayout(pBreadcrumb);
	tContent = __xgeXuiBreadcrumbContentRect(pBreadcrumb);
	if ( pBreadcrumb->iCollapsedCount > 0 ) {
		tSegment = (xge_rect_t){ tContent.fX, tContent.fY, 30.0f, tContent.fH };
		__xgeXuiHostDrawRect(tSegment, pBreadcrumb->iSegmentColor);
		__xgeXuiHostDrawBorderRect(tSegment, 1.0f, pBreadcrumb->iBorderColor);
		if ( pBreadcrumb->pFont != NULL ) {
			__xgeXuiHostDrawTextRect(pBreadcrumb->pFont, "...", tSegment, pBreadcrumb->iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		tSeparator = (xge_rect_t){ tSegment.fX + tSegment.fW, tContent.fY, pBreadcrumb->fSeparatorWidth, tContent.fH };
		if ( pBreadcrumb->pFont != NULL ) {
			__xgeXuiHostDrawTextRect(pBreadcrumb->pFont, ">", tSeparator, pBreadcrumb->iSeparatorColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	for ( i = pBreadcrumb->iFirstVisible; i < pBreadcrumb->iSegmentCount; i++ ) {
		tSegment = pBreadcrumb->arrSegments[i].tRect;
		if ( tSegment.fW <= 0.0f ) {
			continue;
		}
		iColor = pBreadcrumb->iSegmentColor;
		if ( i == pBreadcrumb->iHover ) {
			iColor = pBreadcrumb->iHoverColor;
		}
		if ( i == pBreadcrumb->iSelected ) {
			iColor = pBreadcrumb->iSelectedColor;
		}
		__xgeXuiHostDrawRect(tSegment, iColor);
		__xgeXuiHostDrawBorderRect(tSegment, 1.0f, pBreadcrumb->iBorderColor);
		if ( pBreadcrumb->pFont != NULL ) {
			__xgeXuiHostDrawTextRect(pBreadcrumb->pFont, pBreadcrumb->arrSegments[i].sText != NULL ? pBreadcrumb->arrSegments[i].sText : "", tSegment, pBreadcrumb->iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		if ( i + 1 < pBreadcrumb->iSegmentCount ) {
			tSeparator = (xge_rect_t){ tSegment.fX + tSegment.fW, tContent.fY, pBreadcrumb->fSeparatorWidth, tContent.fH };
			if ( pBreadcrumb->pFont != NULL ) {
				__xgeXuiHostDrawTextRect(pBreadcrumb->pFont, ">", tSeparator, pBreadcrumb->iSeparatorColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			}
		}
	}
}
