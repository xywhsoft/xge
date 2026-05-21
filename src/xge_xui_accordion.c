static uint32_t __xgeXuiAccordionLighten(uint32_t iColor, int iDelta)
{
	int iR;
	int iG;
	int iB;

	iR = (int)XGE_COLOR_GET_R(iColor) + iDelta;
	iG = (int)XGE_COLOR_GET_G(iColor) + iDelta;
	iB = (int)XGE_COLOR_GET_B(iColor) + iDelta;
	if ( iR < 0 ) {
		iR = 0;
	} else if ( iR > 255 ) {
		iR = 255;
	}
	if ( iG < 0 ) {
		iG = 0;
	} else if ( iG > 255 ) {
		iG = 255;
	}
	if ( iB < 0 ) {
		iB = 0;
	} else if ( iB > 255 ) {
		iB = 255;
	}
	return XGE_COLOR_RGBA(iR, iG, iB, XGE_COLOR_GET_A(iColor));
}

static xge_xui_accordion_section_t* __xgeXuiAccordionSection(xge_xui_accordion pAccordion, int iIndex)
{
	if ( (pAccordion == NULL) || (iIndex < 0) || (iIndex >= pAccordion->iSectionCount) ) {
		return NULL;
	}
	return &pAccordion->arrSections[iIndex];
}

static void __xgeXuiAccordionUpdateHeaderText(xge_xui_accordion pAccordion, int iIndex)
{
	xge_xui_accordion_section_t* pSection;
	size_t n;

	pSection = __xgeXuiAccordionSection(pAccordion, iIndex);
	if ( pSection == NULL ) {
		return;
	}
	n = strlen(pSection->sTitle);
	if ( n >= sizeof(pSection->sHeaderText) ) {
		n = sizeof(pSection->sHeaderText) - 1;
	}
	memcpy(pSection->sHeaderText, pSection->sTitle, n);
	pSection->sHeaderText[n] = 0;
	xgeXuiButtonSetText(&pSection->tHeaderButton, pAccordion->pFont, pSection->sHeaderText);
}

static void __xgeXuiAccordionApplySectionStyle(xge_xui_accordion pAccordion, int iIndex)
{
	xge_xui_accordion_section_t* pSection;
	uint32_t iTextColor;

	pSection = __xgeXuiAccordionSection(pAccordion, iIndex);
	if ( pSection == NULL ) {
		return;
	}
	xgeXuiButtonSetColors(&pSection->tHeaderButton, pAccordion->iHeaderColor, pAccordion->iHoverColor, pAccordion->iExpandedColor, pAccordion->iExpandedColor, XGE_COLOR_RGBA(224, 232, 238, 255));
	xgeXuiWidgetSetBorder(pSection->pHeaderWidget, 1.0f, pAccordion->iBorderColor);
	xgeXuiWidgetSetStateBorder(pSection->pHeaderWidget, XGE_XUI_STATE_CHECKED, 1.0f, pAccordion->iExpandedColor);
	xgeXuiWidgetSetSize(pSection->pHeaderWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(pAccordion->fHeaderHeight));
	xgeXuiWidgetSetPaddingPx(pSection->pHeaderWidget, 24.0f, 0.0f, 8.0f, 0.0f);
	xgeXuiWidgetSetBackground(pSection->pClientWidget, pAccordion->iContentColor);
	xgeXuiWidgetSetBorder(pSection->pClientWidget, 1.0f, pAccordion->iBorderColor);
	xgeXuiWidgetSetPaddingPx(pSection->pClientWidget, pAccordion->fContentPadding, pAccordion->fContentPadding, pAccordion->fContentPadding, pAccordion->fContentPadding);
	xgeXuiButtonSetSelected(&pSection->tHeaderButton, pSection->bExpanded);
	xgeXuiWidgetSetVisible(pSection->pClientWidget, pSection->bExpanded);
	xgeXuiWidgetSetEnabled(pSection->pHeaderWidget, pSection->bEnabled);
	iTextColor = pSection->bEnabled ? (pSection->bExpanded ? pAccordion->iActiveTextColor : pAccordion->iTextColor) : pAccordion->iDisabledTextColor;
	xgeXuiButtonSetTextColor(&pSection->tHeaderButton, iTextColor);
	__xgeXuiAccordionUpdateHeaderText(pAccordion, iIndex);
}

static void __xgeXuiAccordionRefresh(xge_xui_accordion pAccordion)
{
	int i;

	if ( pAccordion == NULL ) {
		return;
	}
	for ( i = 0; i < pAccordion->iSectionCount; i++ ) {
		__xgeXuiAccordionApplySectionStyle(pAccordion, i);
	}
	xgeXuiWidgetMarkLayout(pAccordion->pWidget);
	xgeXuiWidgetMarkPaint(pAccordion->pWidget);
}

static int __xgeXuiAccordionFindHeader(xge_xui_accordion pAccordion, xge_xui_widget pWidget)
{
	int i;

	if ( (pAccordion == NULL) || (pWidget == NULL) ) {
		return -1;
	}
	for ( i = 0; i < pAccordion->iSectionCount; i++ ) {
		if ( pAccordion->arrSections[i].pHeaderWidget == pWidget ) {
			return i;
		}
	}
	return -1;
}

static void __xgeXuiAccordionHeaderPaintAfter(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_accordion pAccordion;
	xge_xui_accordion_section_t* pSection;
	xge_rect_t tRect;
	xge_vec2_t tA;
	xge_vec2_t tB;
	xge_vec2_t tC;
	float fCX;
	float fCY;
	int iIndex;

	pAccordion = (xge_xui_accordion)pUser;
	iIndex = __xgeXuiAccordionFindHeader(pAccordion, pWidget);
	pSection = __xgeXuiAccordionSection(pAccordion, iIndex);
	if ( (pWidget == NULL) || (pSection == NULL) ) {
		return;
	}
	tRect = pWidget->tBorderRect;
	fCX = tRect.fX + 10.0f;
	fCY = tRect.fY + tRect.fH * 0.5f;
	if ( pSection->bExpanded ) {
		tA = (xge_vec2_t){ fCX - 4.0f, fCY - 2.0f };
		tB = (xge_vec2_t){ fCX + 4.0f, fCY - 2.0f };
		tC = (xge_vec2_t){ fCX, fCY + 3.0f };
	} else {
		tA = (xge_vec2_t){ fCX - 2.0f, fCY - 4.0f };
		tB = (xge_vec2_t){ fCX - 2.0f, fCY + 4.0f };
		tC = (xge_vec2_t){ fCX + 3.0f, fCY };
	}
	__xgeXuiHostDrawTriangle(tA, tB, tC, pSection->tHeaderButton.iTextColor);
}

static void __xgeXuiAccordionHeaderClick(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_accordion pAccordion;
	int iIndex;

	pAccordion = (xge_xui_accordion)pUser;
	iIndex = __xgeXuiAccordionFindHeader(pAccordion, pWidget);
	if ( iIndex < 0 ) {
		return;
	}
	if ( pAccordion->arrSections[iIndex].bEnabled == 0 ) {
		return;
	}
	pAccordion->iSelected = iIndex;
	xgeXuiAccordionSetExpanded(pAccordion, iIndex, !pAccordion->arrSections[iIndex].bExpanded);
	pAccordion->iSelectCount++;
	if ( pAccordion->procSelect != NULL ) {
		pAccordion->procSelect(pAccordion->pWidget, iIndex, pAccordion->pUser);
	}
}

int xgeXuiAccordionInit(xge_xui_accordion pAccordion, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pAccordion == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pAccordion, 0, sizeof(*pAccordion));
	pTheme = xgeXuiGetTheme(pContext);
	__xgeXuiControlWidgetInit(pWidget, 1);
	xgeXuiWidgetSetRole(pWidget, XGE_XUI_WIDGET_ROLE_CONTAINER);
	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiAccordionEventProc, pAccordion);
	pWidget->procPaint = xgeXuiAccordionPaintProc;
	pWidget->pUser = pAccordion;

	pAccordion->pContext = pContext;
	pAccordion->pWidget = pWidget;
	pAccordion->pFont = (pTheme != NULL) ? pTheme->pFont : NULL;
	pAccordion->iMode = XGE_XUI_ACCORDION_MODE_MULTIPLE;
	pAccordion->iSelected = -1;
	pAccordion->fHeaderHeight = 28.0f;
	pAccordion->fSpacing = 4.0f;
	pAccordion->fContentPadding = 8.0f;
	pAccordion->iHeaderColor = (pTheme != NULL) ? pTheme->iStateNormal : XGE_COLOR_RGBA(232, 243, 251, 255);
	pAccordion->iHoverColor = (pTheme != NULL) ? pTheme->iStateHover : XGE_COLOR_RGBA(238, 248, 255, 255);
	pAccordion->iExpandedColor = (pTheme != NULL) ? pTheme->iAccentColor : XGE_COLOR_RGBA(46, 124, 214, 255);
	pAccordion->iContentColor = XGE_COLOR_RGBA(248, 252, 255, 255);
	pAccordion->iBorderColor = (pTheme != NULL) ? pTheme->iBorderColor : XGE_COLOR_RGBA(127, 196, 229, 255);
	pAccordion->iTextColor = (pTheme != NULL) ? pTheme->iTextColor : XGE_COLOR_RGBA(31, 58, 82, 255);
	pAccordion->iActiveTextColor = XGE_COLOR_RGBA(248, 252, 255, 255);
	pAccordion->iDisabledTextColor = XGE_COLOR_RGBA(132, 148, 160, 255);
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(246, 251, 255, 255));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, pAccordion->iBorderColor);
	xgeXuiWidgetSetGap(pWidget, pAccordion->fSpacing);
	return XGE_OK;
}

void xgeXuiAccordionUnit(xge_xui_accordion pAccordion)
{
	if ( pAccordion == NULL ) {
		return;
	}
	xgeXuiAccordionClear(pAccordion);
	if ( pAccordion->pWidget != NULL && pAccordion->pWidget->pUser == pAccordion ) {
		pAccordion->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pAccordion->pWidget, NULL, NULL);
		pAccordion->pWidget->procPaint = NULL;
	}
	memset(pAccordion, 0, sizeof(*pAccordion));
}

void xgeXuiAccordionClear(xge_xui_accordion pAccordion)
{
	int i;

	if ( pAccordion == NULL ) {
		return;
	}
	for ( i = 0; i < pAccordion->iSectionCount; i++ ) {
		xgeXuiButtonUnit(&pAccordion->arrSections[i].tHeaderButton);
		xgeXuiWidgetFree(pAccordion->arrSections[i].pSectionWidget);
	}
	pAccordion->iSectionCount = 0;
	pAccordion->iSelected = -1;
	xgeXuiWidgetMarkLayout(pAccordion->pWidget);
	xgeXuiWidgetMarkPaint(pAccordion->pWidget);
}

int xgeXuiAccordionAddSection(xge_xui_accordion pAccordion, const char* sTitle, int bExpanded, int iId)
{
	xge_xui_accordion_section_t* pSection;
	xge_xui_widget pSectionWidget;
	xge_xui_widget pHeaderWidget;
	xge_xui_widget pClientWidget;
	int iIndex;
	int i;

	if ( (pAccordion == NULL) || (pAccordion->pWidget == NULL) || (pAccordion->iSectionCount >= XGE_XUI_ACCORDION_SECTION_CAPACITY) ) {
		return -1;
	}
	pSectionWidget = xgeXuiWidgetCreate();
	pHeaderWidget = xgeXuiWidgetCreate();
	pClientWidget = xgeXuiWidgetCreate();
	if ( (pSectionWidget == NULL) || (pHeaderWidget == NULL) || (pClientWidget == NULL) ) {
		xgeXuiWidgetFree(pSectionWidget);
		xgeXuiWidgetFree(pHeaderWidget);
		xgeXuiWidgetFree(pClientWidget);
		return -1;
	}
	iIndex = pAccordion->iSectionCount;
	pSection = &pAccordion->arrSections[iIndex];
	memset(pSection, 0, sizeof(*pSection));
	strncpy(pSection->sTitle, (sTitle != NULL) ? sTitle : "", XGE_XUI_ACCORDION_TITLE_CAPACITY - 1);
	pSection->sTitle[XGE_XUI_ACCORDION_TITLE_CAPACITY - 1] = 0;
	pSection->iId = iId;
	pSection->bExpanded = (bExpanded != 0);
	pSection->bEnabled = 1;
	pSection->pSectionWidget = pSectionWidget;
	pSection->pHeaderWidget = pHeaderWidget;
	pSection->pClientWidget = pClientWidget;

	xgeXuiWidgetSetRole(pSectionWidget, XGE_XUI_WIDGET_ROLE_CONTAINER);
	xgeXuiWidgetSetLayout(pSectionWidget, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pSectionWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeContent());
	xgeXuiWidgetSetPaddingPx(pSectionWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetGap(pSectionWidget, 0.0f);
	xgeXuiWidgetSetBackground(pSectionWidget, XGE_COLOR_RGBA(0, 0, 0, 0));

	if ( xgeXuiButtonInit(&pSection->tHeaderButton, pAccordion->pContext, pHeaderWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pSectionWidget);
		xgeXuiWidgetFree(pHeaderWidget);
		xgeXuiWidgetFree(pClientWidget);
		memset(pSection, 0, sizeof(*pSection));
		return -1;
	}
	pSection->tHeaderButton.iTextFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	xgeXuiButtonSetSelectable(&pSection->tHeaderButton, 1);
	xgeXuiButtonSetClick(&pSection->tHeaderButton, __xgeXuiAccordionHeaderClick, pAccordion);
	xgeXuiWidgetSetPaintAfter(pHeaderWidget, __xgeXuiAccordionHeaderPaintAfter, pAccordion);

	xgeXuiWidgetSetRole(pClientWidget, XGE_XUI_WIDGET_ROLE_CONTAINER);
	xgeXuiWidgetSetLayout(pClientWidget, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pClientWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeContent());
	xgeXuiWidgetSetGap(pClientWidget, 6.0f);
	xgeXuiWidgetSetClip(pClientWidget, 1);

	if ( xgeXuiWidgetAddInternal(pSectionWidget, pHeaderWidget) != XGE_OK ) {
		xgeXuiButtonUnit(&pSection->tHeaderButton);
		xgeXuiWidgetFree(pSectionWidget);
		xgeXuiWidgetFree(pHeaderWidget);
		xgeXuiWidgetFree(pClientWidget);
		memset(pSection, 0, sizeof(*pSection));
		return -1;
	}
	if ( xgeXuiWidgetAddInternal(pSectionWidget, pClientWidget) != XGE_OK ) {
		xgeXuiButtonUnit(&pSection->tHeaderButton);
		xgeXuiWidgetFree(pSectionWidget);
		xgeXuiWidgetFree(pClientWidget);
		memset(pSection, 0, sizeof(*pSection));
		return -1;
	}
	if ( xgeXuiWidgetAddInternal(pAccordion->pWidget, pSectionWidget) != XGE_OK ) {
		xgeXuiButtonUnit(&pSection->tHeaderButton);
		xgeXuiWidgetFree(pSectionWidget);
		memset(pSection, 0, sizeof(*pSection));
		return -1;
	}
	pAccordion->iSectionCount++;
	if ( pAccordion->iMode == XGE_XUI_ACCORDION_MODE_SINGLE && pSection->bExpanded ) {
		for ( i = 0; i < iIndex; i++ ) {
			pAccordion->arrSections[i].bExpanded = 0;
		}
	}
	if ( pAccordion->iSelected < 0 && pSection->bExpanded ) {
		pAccordion->iSelected = iIndex;
	}
	__xgeXuiAccordionRefresh(pAccordion);
	return iIndex;
}

int xgeXuiAccordionGetSectionCount(xge_xui_accordion pAccordion)
{
	return (pAccordion != NULL) ? pAccordion->iSectionCount : 0;
}

xge_xui_widget xgeXuiAccordionGetSectionWidget(xge_xui_accordion pAccordion, int iIndex)
{
	xge_xui_accordion_section_t* pSection;

	pSection = __xgeXuiAccordionSection(pAccordion, iIndex);
	return (pSection != NULL) ? pSection->pSectionWidget : NULL;
}

xge_xui_widget xgeXuiAccordionGetHeaderWidget(xge_xui_accordion pAccordion, int iIndex)
{
	xge_xui_accordion_section_t* pSection;

	pSection = __xgeXuiAccordionSection(pAccordion, iIndex);
	return (pSection != NULL) ? pSection->pHeaderWidget : NULL;
}

xge_xui_widget xgeXuiAccordionGetButtonWidget(xge_xui_accordion pAccordion, int iIndex)
{
	return xgeXuiAccordionGetHeaderWidget(pAccordion, iIndex);
}

xge_xui_widget xgeXuiAccordionGetClientWidget(xge_xui_accordion pAccordion, int iIndex)
{
	xge_xui_accordion_section_t* pSection;

	pSection = __xgeXuiAccordionSection(pAccordion, iIndex);
	return (pSection != NULL) ? pSection->pClientWidget : NULL;
}

int xgeXuiAccordionIsExpanded(xge_xui_accordion pAccordion, int iIndex)
{
	xge_xui_accordion_section_t* pSection;

	pSection = __xgeXuiAccordionSection(pAccordion, iIndex);
	return (pSection != NULL) ? pSection->bExpanded : 0;
}

void xgeXuiAccordionSetExpanded(xge_xui_accordion pAccordion, int iIndex, int bExpanded)
{
	int i;

	if ( __xgeXuiAccordionSection(pAccordion, iIndex) == NULL ) {
		return;
	}
	xgeXuiLayoutBatchBegin(pAccordion->pContext);
	if ( pAccordion->iMode == XGE_XUI_ACCORDION_MODE_SINGLE && bExpanded ) {
		for ( i = 0; i < pAccordion->iSectionCount; i++ ) {
			if ( i != iIndex ) {
				pAccordion->arrSections[i].bExpanded = 0;
				__xgeXuiAccordionApplySectionStyle(pAccordion, i);
			}
		}
	}
	pAccordion->arrSections[iIndex].bExpanded = (bExpanded != 0);
	if ( bExpanded != 0 ) {
		pAccordion->iSelected = iIndex;
	} else if ( pAccordion->iSelected == iIndex ) {
		pAccordion->iSelected = -1;
	}
	__xgeXuiAccordionApplySectionStyle(pAccordion, iIndex);
	xgeXuiLayoutBatchEnd(pAccordion->pContext);
	xgeXuiWidgetMarkLayout(pAccordion->pWidget);
	xgeXuiWidgetMarkPaint(pAccordion->pWidget);
}

void xgeXuiAccordionSetSectionEnabled(xge_xui_accordion pAccordion, int iIndex, int bEnabled)
{
	xge_xui_accordion_section_t* pSection;

	pSection = __xgeXuiAccordionSection(pAccordion, iIndex);
	if ( pSection == NULL ) {
		return;
	}
	pSection->bEnabled = (bEnabled != 0);
	__xgeXuiAccordionApplySectionStyle(pAccordion, iIndex);
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
	__xgeXuiAccordionRefresh(pAccordion);
}

void xgeXuiAccordionSetFont(xge_xui_accordion pAccordion, xui_font pFont)
{
	int i;

	if ( pAccordion == NULL ) {
		return;
	}
	pAccordion->pFont = pFont;
	for ( i = 0; i < pAccordion->iSectionCount; i++ ) {
		xgeXuiButtonSetText(&pAccordion->arrSections[i].tHeaderButton, pFont, pAccordion->arrSections[i].sHeaderText);
	}
}

void xgeXuiAccordionSetMetrics(xge_xui_accordion pAccordion, float fHeaderHeight, float fSpacing, float fContentPadding)
{
	if ( pAccordion == NULL ) {
		return;
	}
	pAccordion->fHeaderHeight = (fHeaderHeight < 18.0f) ? 18.0f : fHeaderHeight;
	pAccordion->fSpacing = (fSpacing < 0.0f) ? 0.0f : fSpacing;
	pAccordion->fContentPadding = (fContentPadding < 0.0f) ? 0.0f : fContentPadding;
	xgeXuiWidgetSetGap(pAccordion->pWidget, pAccordion->fSpacing);
	__xgeXuiAccordionRefresh(pAccordion);
}

void xgeXuiAccordionSetSelect(xge_xui_accordion pAccordion, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pAccordion != NULL ) {
		pAccordion->procSelect = procSelect;
		pAccordion->pUser = pUser;
	}
}

void xgeXuiAccordionSetColors(xge_xui_accordion pAccordion, uint32_t iBackground, uint32_t iHeader, uint32_t iHover, uint32_t iExpanded, uint32_t iContent, uint32_t iBorder, uint32_t iText)
{
	if ( pAccordion == NULL ) {
		return;
	}
	pAccordion->iHeaderColor = iHeader;
	pAccordion->iHoverColor = (XGE_COLOR_GET_A(iHover) != 0) ? iHover : __xgeXuiAccordionLighten(iHeader, 12);
	pAccordion->iExpandedColor = iExpanded;
	pAccordion->iContentColor = iContent;
	pAccordion->iBorderColor = iBorder;
	pAccordion->iTextColor = iText;
	pAccordion->iActiveTextColor = XGE_COLOR_RGBA(248, 252, 255, 255);
	xgeXuiWidgetSetBackground(pAccordion->pWidget, iBackground);
	xgeXuiWidgetSetBorder(pAccordion->pWidget, 1.0f, iBorder);
	__xgeXuiAccordionRefresh(pAccordion);
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
		if ( pAccordion->arrSections[i].pSectionWidget != NULL ) {
			fHeight += pAccordion->arrSections[i].pSectionWidget->tDesiredSize.fY;
		}
		if ( i > 0 ) {
			fHeight += pAccordion->fSpacing;
		}
	}
	return fHeight;
}

int xgeXuiAccordionEvent(xge_xui_accordion pAccordion, const xge_event_t* pEvent)
{
	if ( (pAccordion == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( !xgeXuiWidgetIsEnabled(pAccordion->pWidget) ) {
		pAccordion->iState = XGE_XUI_STATE_DISABLED;
		return XGE_XUI_EVENT_CONTINUE;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int xgeXuiAccordionEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiAccordionEvent((xge_xui_accordion)pUser, pEvent);
}

void xgeXuiAccordionPaintProc(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
}
