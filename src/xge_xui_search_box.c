static void __xgeXuiSearchBoxLayout(xge_xui_search_box pSearch)
{
	xge_rect_t tRect;
	float fIcon;

	if ( (pSearch == NULL) || (pSearch->pWidget == NULL) ) {
		return;
	}
	tRect = pSearch->pWidget->tRect;
	fIcon = 14.0f;
	pSearch->tSearchIconRect.fX = tRect.fX + 7.0f;
	pSearch->tSearchIconRect.fY = tRect.fY + (tRect.fH - fIcon) * 0.5f;
	pSearch->tSearchIconRect.fW = fIcon;
	pSearch->tSearchIconRect.fH = fIcon;
	pSearch->tClearRect.fX = tRect.fX + tRect.fW - 22.0f;
	pSearch->tClearRect.fY = tRect.fY + (tRect.fH - 16.0f) * 0.5f;
	pSearch->tClearRect.fW = 16.0f;
	pSearch->tClearRect.fH = 16.0f;
	xgeXuiWidgetSetPaddingPx(pSearch->pWidget, 26.0f, 4.0f, 26.0f, 4.0f);
}

static uint32_t __xgeXuiSearchBoxHoverColor(uint32_t iColor)
{
	int iR = (int)XGE_COLOR_GET_R(iColor) - 24;
	int iG = (int)XGE_COLOR_GET_G(iColor) - 24;
	int iB = (int)XGE_COLOR_GET_B(iColor) - 24;
	return XGE_COLOR_RGBA(iR < 0 ? 0 : iR, iG < 0 ? 0 : iG, iB < 0 ? 0 : iB, XGE_COLOR_GET_A(iColor));
}

int xgeXuiSearchBoxInit(xge_xui_search_box pSearch, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	const xge_xui_theme_t* pTheme;
	int iRet;

	if ( (pSearch == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pSearch, 0, sizeof(*pSearch));
	pTheme = xgeXuiGetTheme(pContext);
	pSearch->pContext = pContext;
	pSearch->pWidget = pWidget;
	pSearch->bSuggestionsReserved = 1;
	pSearch->iIconColor = XGE_COLOR_RGBA(68, 126, 166, 255);
	pSearch->iClearColor = XGE_COLOR_RGBA(96, 126, 148, 255);
	pSearch->iClearHoverColor = __xgeXuiSearchBoxHoverColor(pSearch->iClearColor);
	__xgeXuiSearchBoxLayout(pSearch);
	iRet = xgeXuiInputInit(&pSearch->tInput, pContext, pWidget, (pFont != NULL) ? pFont : pTheme->pFont);
	if ( iRet != XGE_OK ) {
		memset(pSearch, 0, sizeof(*pSearch));
		return iRet;
	}
	xgeXuiInputSetPlaceholder(&pSearch->tInput, "Search");
	xgeXuiWidgetSetEvent(pWidget, xgeXuiSearchBoxEventProc, NULL);
	pWidget->procUpdate = xgeXuiSearchBoxUpdateProc;
	pWidget->procPaint = xgeXuiSearchBoxPaintProc;
	pWidget->pUser = pSearch;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiSearchBoxUnit(xge_xui_search_box pSearch)
{
	if ( pSearch == NULL ) {
		return;
	}
	xgeXuiInputUnit(&pSearch->tInput);
	if ( pSearch->pWidget != NULL && pSearch->pWidget->pUser == pSearch ) {
		pSearch->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pSearch->pWidget, NULL, NULL);
		pSearch->pWidget->procUpdate = NULL;
		pSearch->pWidget->procPaint = NULL;
	}
	memset(pSearch, 0, sizeof(*pSearch));
}

void xgeXuiSearchBoxSetText(xge_xui_search_box pSearch, const char* sText)
{
	if ( pSearch != NULL ) {
		xgeXuiInputSetText(&pSearch->tInput, sText);
	}
}

const char* xgeXuiSearchBoxGetText(xge_xui_search_box pSearch)
{
	return (pSearch != NULL) ? xgeXuiInputGetText(&pSearch->tInput) : "";
}

void xgeXuiSearchBoxSetPlaceholder(xge_xui_search_box pSearch, const char* sText)
{
	if ( pSearch != NULL ) {
		xgeXuiInputSetPlaceholder(&pSearch->tInput, sText);
	}
}

void xgeXuiSearchBoxSetSubmit(xge_xui_search_box pSearch, xge_xui_text_submit_proc procSubmit, void* pUser)
{
	if ( pSearch != NULL ) {
		pSearch->procSubmit = procSubmit;
		pSearch->pSubmitUser = pUser;
	}
}

void xgeXuiSearchBoxSetClear(xge_xui_search_box pSearch, xge_xui_text_submit_proc procClear, void* pUser)
{
	if ( pSearch != NULL ) {
		pSearch->procClear = procClear;
		pSearch->pClearUser = pUser;
	}
}

void xgeXuiSearchBoxSetSuggestionsReserved(xge_xui_search_box pSearch, int bReserved)
{
	if ( pSearch != NULL ) {
		pSearch->bSuggestionsReserved = (bReserved != 0);
	}
}

void xgeXuiSearchBoxSetColors(xge_xui_search_box pSearch, uint32_t iText, uint32_t iBackground, uint32_t iFocus, uint32_t iCursor, uint32_t iIcon, uint32_t iClear)
{
	if ( pSearch == NULL ) {
		return;
	}
	xgeXuiInputSetColors(&pSearch->tInput, iText, iBackground, iFocus, iCursor);
	pSearch->iIconColor = iIcon;
	pSearch->iClearColor = iClear;
	pSearch->iClearHoverColor = __xgeXuiSearchBoxHoverColor(iClear);
	xgeXuiWidgetMarkPaint(pSearch->pWidget);
}

int xgeXuiSearchBoxEvent(xge_xui_search_box pSearch, const xge_event_t* pEvent)
{
	int iRet;

	if ( (pSearch == NULL) || (pSearch->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	__xgeXuiSearchBoxLayout(pSearch);
	if ( pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
		pSearch->bClearHover = __xgeXuiRectContains(pSearch->tClearRect, pEvent->fX, pEvent->fY) && xgeXuiSearchBoxGetText(pSearch)[0] != 0;
		xgeXuiWidgetMarkPaint(pSearch->pWidget);
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_DOWN && __xgeXuiRectContains(pSearch->tClearRect, pEvent->fX, pEvent->fY) && xgeXuiSearchBoxGetText(pSearch)[0] != 0 ) {
		xgeXuiInputSetText(&pSearch->tInput, "");
		pSearch->iClearCount++;
		if ( pSearch->procClear != NULL ) {
			pSearch->procClear(pSearch->pWidget, "", pSearch->pClearUser);
		}
		xgeXuiSetFocus(pSearch->pContext, pSearch->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN && pEvent->iParam1 == XGE_KEY_ENTER && pSearch->pContext != NULL && pSearch->pContext->pFocus == pSearch->pWidget ) {
		pSearch->iSubmitCount++;
		if ( pSearch->procSubmit != NULL ) {
			pSearch->procSubmit(pSearch->pWidget, xgeXuiSearchBoxGetText(pSearch), pSearch->pSubmitUser);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	iRet = xgeXuiInputEvent(&pSearch->tInput, pEvent);
	pSearch->pWidget->pUser = pSearch;
	xgeXuiWidgetSetEvent(pSearch->pWidget, xgeXuiSearchBoxEventProc, NULL);
	pSearch->pWidget->procUpdate = xgeXuiSearchBoxUpdateProc;
	pSearch->pWidget->procPaint = xgeXuiSearchBoxPaintProc;
	return iRet;
}

int xgeXuiSearchBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiSearchBoxEvent((xge_xui_search_box)pUser, pEvent);
}

void xgeXuiSearchBoxUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser)
{
	xge_xui_search_box pSearch;

	(void)pWidget;
	pSearch = (xge_xui_search_box)pUser;
	if ( pSearch != NULL ) {
		xgeXuiInputUpdateProc(pSearch->pWidget, fDelta, &pSearch->tInput);
	}
}

void xgeXuiSearchBoxPaintProc(xge_xui_widget pWidget, void* pUser)
{
	static const uint16_t arrSearch12[12] = {
		0x000, 0x1e0, 0x210, 0x408,
		0x408, 0x408, 0x210, 0x1e0,
		0x060, 0x030, 0x018, 0x000
	};
	static const uint16_t arrClear10[10] = {
		0x201, 0x102, 0x084, 0x048, 0x030,
		0x030, 0x048, 0x084, 0x102, 0x201
	};
	xge_xui_search_box pSearch;
	uint32_t iClear;

	pSearch = (xge_xui_search_box)pUser;
	if ( (pWidget == NULL) || (pSearch == NULL) ) {
		return;
	}
	__xgeXuiSearchBoxLayout(pSearch);
	xgeXuiInputPaintProc(pWidget, &pSearch->tInput);
	__xgeXuiHostDrawBitmapMask(pSearch->tSearchIconRect, arrSearch12, 12, 12, pSearch->iIconColor);
	if ( xgeXuiSearchBoxGetText(pSearch)[0] != 0 ) {
		iClear = pSearch->bClearHover ? pSearch->iClearHoverColor : pSearch->iClearColor;
		__xgeXuiHostDrawBitmapMask((xge_rect_t){ pSearch->tClearRect.fX + 3.0f, pSearch->tClearRect.fY + 3.0f, 10.0f, 10.0f }, arrClear10, 10, 10, iClear);
	}
}
