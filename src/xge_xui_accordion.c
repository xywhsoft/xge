static uint32_t __xgeXuiAccordionHoverColor(uint32_t iHeader)
{
	int iR = (int)XGE_COLOR_GET_R(iHeader) + 14;
	int iG = (int)XGE_COLOR_GET_G(iHeader) + 14;
	int iB = (int)XGE_COLOR_GET_B(iHeader) + 14;
	return XGE_COLOR_RGBA(iR > 255 ? 255 : iR, iG > 255 ? 255 : iG, iB > 255 ? 255 : iB, XGE_COLOR_GET_A(iHeader));
}

static xge_rect_t __xgeXuiAccordionContentRect(xge_xui_accordion pAccordion)
{
	xge_rect_t tRect;

	tRect = pAccordion->pWidget->tContentRect;
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		tRect = pAccordion->pWidget->tRect;
	}
	return tRect;
}

static void __xgeXuiAccordionLayout(xge_xui_accordion pAccordion)
{
	xge_rect_t tContent;
	float fY;
	int i;

	if ( (pAccordion == NULL) || (pAccordion->pWidget == NULL) ) {
		return;
	}
	tContent = __xgeXuiAccordionContentRect(pAccordion);
	fY = tContent.fY;
	for ( i = 0; i < pAccordion->iSectionCount; i++ ) {
		pAccordion->arrSections[i].tHeaderRect = (xge_rect_t){ tContent.fX, fY, tContent.fW, pAccordion->fHeaderHeight };
		fY += pAccordion->fHeaderHeight;
		if ( pAccordion->arrSections[i].bExpanded ) {
			pAccordion->arrSections[i].tContentRect = (xge_rect_t){ tContent.fX, fY, tContent.fW, pAccordion->arrSections[i].fContentHeight };
			fY += pAccordion->arrSections[i].fContentHeight;
		} else {
			memset(&pAccordion->arrSections[i].tContentRect, 0, sizeof(pAccordion->arrSections[i].tContentRect));
		}
		fY += pAccordion->fSpacing;
	}
}

static int __xgeXuiAccordionHeaderAt(xge_xui_accordion pAccordion, float fX, float fY)
{
	int i;

	if ( pAccordion == NULL ) {
		return -1;
	}
	__xgeXuiAccordionLayout(pAccordion);
	for ( i = 0; i < pAccordion->iSectionCount; i++ ) {
		if ( __xgeXuiRectContains(pAccordion->arrSections[i].tHeaderRect, fX, fY) ) {
			return i;
		}
	}
	return -1;
}

int xgeXuiAccordionInit(xge_xui_accordion pAccordion, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pAccordion == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pAccordion, 0, sizeof(*pAccordion));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pAccordion->pContext = pContext;
	pAccordion->pWidget = pWidget;
	pAccordion->iMode = XGE_XUI_ACCORDION_MODE_MULTIPLE;
	pAccordion->iHover = -1;
	pAccordion->iSelected = -1;
	pAccordion->fHeaderHeight = 26.0f;
	pAccordion->fSpacing = 4.0f;
	pAccordion->fContentPadding = 8.0f;
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(236, 246, 253, 255));
	pAccordion->iHeaderColor = XGE_COLOR_RGBA(224, 239, 250, 255);
	pAccordion->iHoverColor = XGE_COLOR_RGBA(238, 248, 255, 255);
	pAccordion->iExpandedColor = XGE_COLOR_RGBA(199, 226, 247, 255);
	pAccordion->iContentColor = XGE_COLOR_RGBA(247, 252, 255, 255);
	pAccordion->iBorderColor = XGE_COLOR_RGBA(129, 174, 207, 255);
	pAccordion->iTextColor = XGE_COLOR_RGBA(31, 58, 82, 255);
	pAccordion->iContentTextColor = XGE_COLOR_RGBA(62, 78, 94, 255);
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiAccordionEventProc, NULL);
	pWidget->procPaint = xgeXuiAccordionPaintProc;
	pWidget->pUser = pAccordion;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiAccordionUnit(xge_xui_accordion pAccordion)
{
	if ( pAccordion == NULL ) {
		return;
	}
	if ( pAccordion->pWidget != NULL && pAccordion->pWidget->pUser == pAccordion ) {
		pAccordion->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pAccordion->pWidget, NULL, NULL);
		pAccordion->pWidget->procPaint = NULL;
	}
	memset(pAccordion, 0, sizeof(*pAccordion));
}

void xgeXuiAccordionClear(xge_xui_accordion pAccordion)
{
	if ( pAccordion == NULL ) {
		return;
	}
	pAccordion->iSectionCount = 0;
	pAccordion->iHover = -1;
	pAccordion->iSelected = -1;
	xgeXuiWidgetMarkPaint(pAccordion->pWidget);
}

int xgeXuiAccordionAddSection(xge_xui_accordion pAccordion, const char* sTitle, const char* sText, float fContentHeight, int bExpanded, int iId)
{
	xge_xui_accordion_section_t* pSection;

	if ( (pAccordion == NULL) || (pAccordion->iSectionCount >= XGE_XUI_ACCORDION_SECTION_CAPACITY) ) {
		return -1;
	}
	pSection = &pAccordion->arrSections[pAccordion->iSectionCount];
	memset(pSection, 0, sizeof(*pSection));
	pSection->sTitle = sTitle;
	pSection->sText = sText;
	pSection->iId = iId;
	pSection->fContentHeight = (fContentHeight < 0.0f) ? 0.0f : fContentHeight;
	pSection->bExpanded = (bExpanded != 0);
	if ( pAccordion->iMode == XGE_XUI_ACCORDION_MODE_SINGLE && pSection->bExpanded ) {
		int i;
		for ( i = 0; i < pAccordion->iSectionCount; i++ ) {
			pAccordion->arrSections[i].bExpanded = 0;
		}
	}
	pAccordion->iSectionCount++;
	__xgeXuiAccordionLayout(pAccordion);
	xgeXuiWidgetMarkPaint(pAccordion->pWidget);
	return pAccordion->iSectionCount - 1;
}

int xgeXuiAccordionGetSectionCount(xge_xui_accordion pAccordion)
{
	return (pAccordion != NULL) ? pAccordion->iSectionCount : 0;
}

int xgeXuiAccordionIsExpanded(xge_xui_accordion pAccordion, int iIndex)
{
	if ( (pAccordion == NULL) || (iIndex < 0) || (iIndex >= pAccordion->iSectionCount) ) {
		return 0;
	}
	return pAccordion->arrSections[iIndex].bExpanded;
}

void xgeXuiAccordionSetExpanded(xge_xui_accordion pAccordion, int iIndex, int bExpanded)
{
	int i;

	if ( (pAccordion == NULL) || (iIndex < 0) || (iIndex >= pAccordion->iSectionCount) ) {
		return;
	}
	if ( pAccordion->iMode == XGE_XUI_ACCORDION_MODE_SINGLE && bExpanded ) {
		for ( i = 0; i < pAccordion->iSectionCount; i++ ) {
			pAccordion->arrSections[i].bExpanded = 0;
		}
	}
	pAccordion->arrSections[iIndex].bExpanded = (bExpanded != 0);
	__xgeXuiAccordionLayout(pAccordion);
	xgeXuiWidgetMarkPaint(pAccordion->pWidget);
}

void xgeXuiAccordionSetMode(xge_xui_accordion pAccordion, int iMode)
{
	int i;
	int iFirstExpanded;

	if ( pAccordion == NULL ) {
		return;
	}
	pAccordion->iMode = (iMode == XGE_XUI_ACCORDION_MODE_SINGLE) ? XGE_XUI_ACCORDION_MODE_SINGLE : XGE_XUI_ACCORDION_MODE_MULTIPLE;
	if ( pAccordion->iMode == XGE_XUI_ACCORDION_MODE_SINGLE ) {
		iFirstExpanded = -1;
		for ( i = 0; i < pAccordion->iSectionCount; i++ ) {
			if ( pAccordion->arrSections[i].bExpanded ) {
				if ( iFirstExpanded < 0 ) {
					iFirstExpanded = i;
				} else {
					pAccordion->arrSections[i].bExpanded = 0;
				}
			}
		}
	}
	__xgeXuiAccordionLayout(pAccordion);
	xgeXuiWidgetMarkPaint(pAccordion->pWidget);
}

void xgeXuiAccordionSetFont(xge_xui_accordion pAccordion, xge_font pFont)
{
	if ( pAccordion != NULL ) {
		pAccordion->pFont = pFont;
		xgeXuiWidgetMarkPaint(pAccordion->pWidget);
	}
}

void xgeXuiAccordionSetMetrics(xge_xui_accordion pAccordion, float fHeaderHeight, float fSpacing, float fContentPadding)
{
	if ( pAccordion == NULL ) {
		return;
	}
	pAccordion->fHeaderHeight = (fHeaderHeight < 12.0f) ? 12.0f : fHeaderHeight;
	pAccordion->fSpacing = (fSpacing < 0.0f) ? 0.0f : fSpacing;
	pAccordion->fContentPadding = (fContentPadding < 0.0f) ? 0.0f : fContentPadding;
	__xgeXuiAccordionLayout(pAccordion);
	xgeXuiWidgetMarkPaint(pAccordion->pWidget);
}

void xgeXuiAccordionSetSelect(xge_xui_accordion pAccordion, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pAccordion != NULL ) {
		pAccordion->procSelect = procSelect;
		pAccordion->pUser = pUser;
	}
}

void xgeXuiAccordionSetColors(xge_xui_accordion pAccordion, uint32_t iBackground, uint32_t iHeader, uint32_t iExpanded, uint32_t iContent, uint32_t iBorder, uint32_t iText)
{
	if ( pAccordion == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pAccordion->pWidget, iBackground);
	pAccordion->iHeaderColor = iHeader;
	pAccordion->iHoverColor = __xgeXuiAccordionHoverColor(iHeader);
	pAccordion->iExpandedColor = iExpanded;
	pAccordion->iContentColor = iContent;
	pAccordion->iBorderColor = iBorder;
	pAccordion->iTextColor = iText;
	pAccordion->iContentTextColor = iText;
	xgeXuiWidgetMarkPaint(pAccordion->pWidget);
}

float xgeXuiAccordionGetContentHeight(xge_xui_accordion pAccordion)
{
	float fHeight;
	int i;

	if ( pAccordion == NULL ) {
		return 0.0f;
	}
	fHeight = 0.0f;
	for ( i = 0; i < pAccordion->iSectionCount; i++ ) {
		fHeight += pAccordion->fHeaderHeight + pAccordion->fSpacing;
		if ( pAccordion->arrSections[i].bExpanded ) {
			fHeight += pAccordion->arrSections[i].fContentHeight;
		}
	}
	return fHeight;
}

int xgeXuiAccordionEvent(xge_xui_accordion pAccordion, const xge_event_t* pEvent)
{
	int iInside;
	int iSection;

	if ( (pAccordion == NULL) || (pAccordion->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pAccordion->pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
			iSection = iInside ? __xgeXuiAccordionHeaderAt(pAccordion, pEvent->fX, pEvent->fY) : -1;
			if ( pAccordion->iHover != iSection ) {
				pAccordion->iHover = iSection;
				pAccordion->iState = (iSection >= 0) ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
				xgeXuiWidgetMarkPaint(pAccordion->pWidget);
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pAccordion->pContext, pAccordion->pWidget);
			iSection = __xgeXuiAccordionHeaderAt(pAccordion, pEvent->fX, pEvent->fY);
			if ( iSection < 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pAccordion->iSelected = iSection;
			xgeXuiAccordionSetExpanded(pAccordion, iSection, !pAccordion->arrSections[iSection].bExpanded);
			pAccordion->iSelectCount++;
			if ( pAccordion->procSelect != NULL ) {
				pAccordion->procSelect(pAccordion->pWidget, iSection, pAccordion->pUser);
			}
			return XGE_XUI_EVENT_CONSUMED;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiAccordionEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiAccordionEvent((xge_xui_accordion)pUser, pEvent);
}

void xgeXuiAccordionPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_accordion pAccordion;
	xge_xui_accordion_section_t* pSection;
	xge_rect_t tText;
	xge_rect_t tChevron;
	xge_rect_t tBorder;
	uint32_t iHeaderColor;
	float fMidX;
	float fMidY;
	int i;

	pAccordion = (xge_xui_accordion)pUser;
	if ( (pWidget == NULL) || (pAccordion == NULL) ) {
		return;
	}
	__xgeXuiAccordionLayout(pAccordion);
	for ( i = 0; i < pAccordion->iSectionCount; i++ ) {
		pSection = &pAccordion->arrSections[i];
		iHeaderColor = pSection->bExpanded ? pAccordion->iExpandedColor : pAccordion->iHeaderColor;
		if ( i == pAccordion->iHover ) {
			iHeaderColor = pAccordion->iHoverColor;
		}
		__xgeXuiHostDrawRect(pSection->tHeaderRect, iHeaderColor);
		tBorder = pSection->tHeaderRect;
		if ( pSection->bExpanded && pSection->tContentRect.fH > 0.0f ) {
			tBorder.fH += pSection->tContentRect.fH;
		}
		if ( pAccordion->pFont != NULL ) {
			tChevron = pSection->tHeaderRect;
			tChevron.fW = 22.0f;
			fMidX = tChevron.fX + tChevron.fW * 0.5f;
			fMidY = tChevron.fY + tChevron.fH * 0.5f;
			if ( pSection->bExpanded ) {
				xgeShapeLinePx(fMidX - 3.0f, fMidY - 1.5f, fMidX, fMidY + 2.0f, 1.0f, pAccordion->iTextColor);
				xgeShapeLinePx(fMidX, fMidY + 2.0f, fMidX + 3.0f, fMidY - 1.5f, 1.0f, pAccordion->iTextColor);
			} else {
				xgeShapeLinePx(fMidX - 1.5f, fMidY - 3.0f, fMidX + 2.0f, fMidY, 1.0f, pAccordion->iTextColor);
				xgeShapeLinePx(fMidX + 2.0f, fMidY, fMidX - 1.5f, fMidY + 3.0f, 1.0f, pAccordion->iTextColor);
			}
			tText = pSection->tHeaderRect;
			tText.fX += 24.0f;
			tText.fW -= 30.0f;
			__xgeXuiHostDrawTextRect(pAccordion->pFont, pSection->sTitle != NULL ? pSection->sTitle : "", tText, pAccordion->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		if ( pSection->bExpanded && pSection->tContentRect.fH > 0.0f ) {
			__xgeXuiHostDrawRect(pSection->tContentRect, pAccordion->iContentColor);
			__xgeXuiHostDrawBorderRect(tBorder, 1.0f, pAccordion->iBorderColor);
			if ( pAccordion->pFont != NULL ) {
				tText = pSection->tContentRect;
				tText.fX += pAccordion->fContentPadding;
				tText.fY += pAccordion->fContentPadding;
				tText.fW -= pAccordion->fContentPadding * 2.0f;
				tText.fH -= pAccordion->fContentPadding * 2.0f;
				__xgeXuiHostDrawTextRect(pAccordion->pFont, pSection->sText != NULL ? pSection->sText : "", tText, pAccordion->iContentTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP | XGE_TEXT_CLIP);
			}
		} else {
			__xgeXuiHostDrawBorderRect(tBorder, 1.0f, pAccordion->iBorderColor);
		}
	}
}
