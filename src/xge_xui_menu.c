static void __xgeXuiMenuLayout(xge_xui_menu pMenu, float fX, float fY)
{
	xge_rect_t tRect;
	float fWidth;
	float fHeight;
	float fMaxHeight;
	float fWindowW;
	float fWindowH;

	if ( (pMenu == NULL) || (pMenu->pPopupWidget == NULL) || (pMenu->pListWidget == NULL) ) {
		return;
	}
	fWidth = (pMenu->fWidth > 0.0f) ? pMenu->fWidth : 136.0f;
	fMaxHeight = (pMenu->fMaxHeight > 0.0f) ? pMenu->fMaxHeight : 180.0f;
	fHeight = (float)pMenu->iItemCount * pMenu->fItemHeight;
	if ( fHeight < pMenu->fItemHeight ) {
		fHeight = pMenu->fItemHeight;
	}
	if ( fHeight > fMaxHeight ) {
		fHeight = fMaxHeight;
	}
	fWindowW = (float)xgeGetWidth();
	fWindowH = (float)xgeGetHeight();
	if ( fX + fWidth > fWindowW ) {
		fX = fWindowW - fWidth;
	}
	if ( fY + fHeight > fWindowH ) {
		fY = fWindowH - fHeight;
	}
	if ( fX < 0.0f ) {
		fX = 0.0f;
	}
	if ( fY < 0.0f ) {
		fY = 0.0f;
	}
	tRect.fX = fX;
	tRect.fY = fY;
	tRect.fW = fWidth;
	tRect.fH = fHeight;
	xgeXuiWidgetSetRect(pMenu->pPopupWidget, tRect);
	xgeXuiWidgetSetRect(pMenu->pListWidget, (xge_rect_t){ 0.0f, 0.0f, fWidth, fHeight });
}

static void __xgeXuiMenuListSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	xge_xui_menu pMenu;

	(void)pWidget;
	pMenu = (xge_xui_menu)pUser;
	if ( (pMenu == NULL) || (iIndex < 0) || (iIndex >= pMenu->iItemCount) ) {
		return;
	}
	if ( (pMenu->arrEnabled != NULL) && (iIndex < pMenu->iEnabledCount) && (pMenu->arrEnabled[iIndex] == 0) ) {
		return;
	}
	xgeXuiMenuClose(pMenu);
	if ( pMenu->procSelect != NULL ) {
		pMenu->procSelect(pMenu->pOwner, iIndex, pMenu->pUser);
	}
}

static void __xgeXuiMenuPopupClose(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_menu pMenu;

	(void)pWidget;
	pMenu = (xge_xui_menu)pUser;
	if ( (pMenu != NULL) && (pMenu->pContext != NULL) && (pMenu->pOwner != NULL) ) {
		xgeXuiSetFocus(pMenu->pContext, pMenu->pOwner);
	}
}

int xgeXuiMenuInit(xge_xui_menu pMenu, xge_xui_context pContext, xge_xui_widget pOwner)
{
	const xge_xui_theme_t* pTheme;

	if ( (pMenu == NULL) || (pContext == NULL) || (pOwner == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pMenu, 0, sizeof(*pMenu));
	pTheme = xgeXuiGetTheme(pContext);
	pMenu->pContext = pContext;
	pMenu->pOwner = pOwner;
	pMenu->pFont = pTheme->pFont;
	pMenu->fWidth = 136.0f;
	pMenu->fMaxHeight = 180.0f;
	pMenu->fItemHeight = 24.0f;
	pMenu->iBackgroundColor = XGE_COLOR_RGBA(36, 44, 56, 245);
	pMenu->iRowColor = XGE_COLOR_RGBA(42, 52, 66, 255);
	pMenu->iSelectedColor = pTheme->iStateActive;
	pMenu->iTextColor = pTheme->iTextColor;
	pMenu->iDisabledTextColor = XGE_COLOR_RGBA(128, 138, 150, 220);
	pMenu->pPopupWidget = xgeXuiWidgetCreate();
	pMenu->pListWidget = xgeXuiWidgetCreate();
	if ( (pMenu->pPopupWidget == NULL) || (pMenu->pListWidget == NULL) ) {
		xgeXuiWidgetFree(pMenu->pPopupWidget);
		xgeXuiWidgetFree(pMenu->pListWidget);
		memset(pMenu, 0, sizeof(*pMenu));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiPopupInit(&pMenu->tPopup, pContext, pMenu->pPopupWidget);
	xgeXuiPopupSetOwner(&pMenu->tPopup, NULL);
	xgeXuiPopupSetClose(&pMenu->tPopup, __xgeXuiMenuPopupClose, pMenu);
	xgeXuiPopupSetBackground(&pMenu->tPopup, pMenu->iBackgroundColor);
	xgeXuiWidgetSetZ(pMenu->pPopupWidget, 1200);
	xgeXuiListViewInit(&pMenu->tList, pContext, pMenu->pListWidget);
	xgeXuiListViewSetFont(&pMenu->tList, pMenu->pFont);
	xgeXuiListViewSetItemHeight(&pMenu->tList, pMenu->fItemHeight);
	xgeXuiListViewSetColors(&pMenu->tList, XGE_COLOR_RGBA(0, 0, 0, 0), pMenu->iRowColor, pMenu->iSelectedColor, pMenu->iTextColor, XGE_COLOR_RGBA(64, 72, 84, 180), XGE_COLOR_RGBA(160, 172, 188, 220));
	xgeXuiListViewSetDisabledTextColor(&pMenu->tList, pMenu->iDisabledTextColor);
	xgeXuiListViewSetSelect(&pMenu->tList, __xgeXuiMenuListSelect, pMenu);
	xgeXuiWidgetAdd(pMenu->pPopupWidget, pMenu->pListWidget);
	xgeXuiWidgetAdd(xgeXuiOverlayRoot(pContext), pMenu->pPopupWidget);
	return XGE_OK;
}

void xgeXuiMenuUnit(xge_xui_menu pMenu)
{
	xge_xui_widget pPopupWidget;

	if ( pMenu == NULL ) {
		return;
	}
	pPopupWidget = pMenu->pPopupWidget;
	xgeXuiListViewUnit(&pMenu->tList);
	xgeXuiPopupUnit(&pMenu->tPopup);
	xgeXuiWidgetFree(pPopupWidget);
	memset(pMenu, 0, sizeof(*pMenu));
}

void xgeXuiMenuSetItems(xge_xui_menu pMenu, const char** arrItems, int iCount)
{
	if ( pMenu == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	pMenu->arrItems = arrItems;
	pMenu->iItemCount = iCount;
	xgeXuiListViewSetItems(&pMenu->tList, arrItems, iCount);
}

void xgeXuiMenuSetEnabledItems(xge_xui_menu pMenu, const int* arrEnabled, int iCount)
{
	if ( pMenu == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	pMenu->arrEnabled = arrEnabled;
	pMenu->iEnabledCount = iCount;
	xgeXuiListViewSetEnabledItems(&pMenu->tList, arrEnabled, iCount);
}

void xgeXuiMenuSetFont(xge_xui_menu pMenu, xge_font pFont)
{
	if ( pMenu == NULL ) {
		return;
	}
	pMenu->pFont = pFont;
	xgeXuiListViewSetFont(&pMenu->tList, pFont);
}

void xgeXuiMenuSetSelect(xge_xui_menu pMenu, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pMenu == NULL ) {
		return;
	}
	pMenu->procSelect = procSelect;
	pMenu->pUser = pUser;
}

void xgeXuiMenuSetSize(xge_xui_menu pMenu, float fWidth, float fMaxHeight)
{
	if ( pMenu == NULL ) {
		return;
	}
	pMenu->fWidth = fWidth;
	pMenu->fMaxHeight = fMaxHeight;
}

void xgeXuiMenuSetColors(xge_xui_menu pMenu, uint32_t iBackground, uint32_t iRow, uint32_t iSelected, uint32_t iText, uint32_t iDisabledText)
{
	if ( pMenu == NULL ) {
		return;
	}
	pMenu->iBackgroundColor = iBackground;
	pMenu->iRowColor = iRow;
	pMenu->iSelectedColor = iSelected;
	pMenu->iTextColor = iText;
	pMenu->iDisabledTextColor = iDisabledText;
	xgeXuiPopupSetBackground(&pMenu->tPopup, iBackground);
	xgeXuiListViewSetColors(&pMenu->tList, XGE_COLOR_RGBA(0, 0, 0, 0), iRow, iSelected, iText, XGE_COLOR_RGBA(64, 72, 84, 180), XGE_COLOR_RGBA(160, 172, 188, 220));
	xgeXuiListViewSetDisabledTextColor(&pMenu->tList, iDisabledText);
}

void xgeXuiMenuOpen(xge_xui_menu pMenu, float fX, float fY)
{
	if ( pMenu == NULL ) {
		return;
	}
	__xgeXuiMenuLayout(pMenu, fX, fY);
	xgeXuiListViewSetSelected(&pMenu->tList, -1);
	xgeXuiListViewSetScroll(&pMenu->tList, 0.0f);
	xgeXuiPopupSetOpen(&pMenu->tPopup, 1);
	xgeXuiSetFocus(pMenu->pContext, pMenu->pListWidget);
}

void xgeXuiMenuClose(xge_xui_menu pMenu)
{
	if ( pMenu == NULL ) {
		return;
	}
	xgeXuiPopupSetOpen(&pMenu->tPopup, 0);
	if ( pMenu->pContext != NULL && pMenu->pOwner != NULL ) {
		xgeXuiSetFocus(pMenu->pContext, pMenu->pOwner);
	}
}

int xgeXuiMenuIsOpen(xge_xui_menu pMenu)
{
	if ( pMenu == NULL ) {
		return 0;
	}
	return xgeXuiPopupIsOpen(&pMenu->tPopup);
}
