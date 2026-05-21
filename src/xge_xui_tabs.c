static int __xgeXuiTabsItemEnabled(xge_xui_tabs pTabs, int iIndex)
{
	if ( (pTabs == NULL) || (iIndex < 0) || (iIndex >= pTabs->iItemCount) ) {
		return 0;
	}
	return pTabs->arrEnabledLocal[iIndex] != 0;
}

static int __xgeXuiTabsStepEnabled(xge_xui_tabs pTabs, int iStart, int iDelta)
{
	int i;
	int iIndex;

	if ( (pTabs == NULL) || (pTabs->iItemCount <= 0) ) {
		return -1;
	}
	iIndex = iStart;
	for ( i = 0; i < pTabs->iItemCount; i++ ) {
		iIndex += iDelta;
		if ( iIndex >= pTabs->iItemCount ) {
			iIndex = 0;
		} else if ( iIndex < 0 ) {
			iIndex = pTabs->iItemCount - 1;
		}
		if ( __xgeXuiTabsItemEnabled(pTabs, iIndex) ) {
			return iIndex;
		}
	}
	return -1;
}

static void __xgeXuiTabsSetState(xge_xui_tabs pTabs, int iState)
{
	if ( pTabs == NULL ) {
		return;
	}
	if ( (pTabs->pWidget == NULL) || ((pTabs->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pTabs->pContext != NULL && pTabs->pContext->pFocus == pTabs->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pTabs->iState != iState ) {
		pTabs->iState = iState;
		xgeXuiWidgetMarkPaint(pTabs->pWidget);
	}
}

static void __xgeXuiTabsApplyButtonStyle(xge_xui_tabs pTabs, int iIndex)
{
	xge_xui_button pButton;
	xge_xui_widget pWidget;
	uint32_t iTextColor;
	int bEnabled;

	if ( (pTabs == NULL) || (iIndex < 0) || (iIndex >= pTabs->iItemCount) ) {
		return;
	}
	pButton = &pTabs->arrButton[iIndex];
	pWidget = pTabs->arrButtonWidget[iIndex];
	if ( (pButton->pWidget == NULL) || (pWidget == NULL) ) {
		return;
	}
	bEnabled = __xgeXuiTabsItemEnabled(pTabs, iIndex);
	xgeXuiButtonSetColors(pButton, pTabs->iTabColor, pTabs->iHoverColor, pTabs->iActiveColor, pTabs->iFocusColor, pTabs->iDisabledColor);
	xgeXuiWidgetSetBorder(pWidget, 1.0f, pTabs->iBorderColor);
	xgeXuiWidgetSetStateBorder(pWidget, XGE_XUI_STATE_CHECKED, 1.0f, pTabs->iActiveColor);
	xgeXuiWidgetSetEnabled(pWidget, bEnabled);
	xgeXuiButtonSetSelected(pButton, iIndex == pTabs->iSelected);
	__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_NORMAL);
	iTextColor = (iIndex == pTabs->iSelected) ? pTabs->iActiveTextColor : pTabs->iTextColor;
	xgeXuiButtonSetTextColor(pButton, iTextColor);
	xgeXuiButtonSetBadgeVisible(pButton, pTabs->arrDirtyLocal[iIndex]);
	xgeXuiWidgetSetVisible(pWidget, iIndex < pTabs->iItemCount);
}

static void __xgeXuiTabsRefreshPages(xge_xui_tabs pTabs)
{
	int i;

	if ( pTabs == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_TABS_PAGE_CAPACITY; i++ ) {
		if ( pTabs->arrPageWidget[i] != NULL ) {
			xgeXuiWidgetSetVisible(pTabs->arrPageWidget[i], (i < pTabs->iItemCount) && (i == pTabs->iSelected));
		}
		if ( pTabs->arrButtonWidget[i] != NULL ) {
			__xgeXuiTabsApplyButtonStyle(pTabs, i);
		}
	}
	xgeXuiWidgetMarkLayout(pTabs->pWidget);
	xgeXuiWidgetMarkPaint(pTabs->pWidget);
}

static void __xgeXuiTabsSelectInternal(xge_xui_tabs pTabs, int iIndex, int bNotify)
{
	if ( pTabs == NULL ) {
		return;
	}
	if ( (iIndex < 0) || (iIndex >= pTabs->iItemCount) ) {
		iIndex = -1;
	}
	if ( (iIndex >= 0) && (__xgeXuiTabsItemEnabled(pTabs, iIndex) == 0) ) {
		return;
	}
	if ( pTabs->iSelected != iIndex ) {
		pTabs->iSelected = iIndex;
		pTabs->iChangeCount++;
		__xgeXuiTabsRefreshPages(pTabs);
		if ( bNotify && pTabs->procSelect != NULL ) {
			pTabs->procSelect(pTabs->pWidget, pTabs->iSelected, pTabs->pUser);
		}
	} else {
		__xgeXuiTabsRefreshPages(pTabs);
	}
}

static void __xgeXuiTabsButtonClick(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_tabs pTabs;
	int i;

	pTabs = (xge_xui_tabs)pUser;
	if ( (pTabs == NULL) || (pWidget == NULL) ) {
		return;
	}
	for ( i = 0; i < pTabs->iItemCount; i++ ) {
		if ( pTabs->arrButtonWidget[i] == pWidget ) {
			__xgeXuiTabsSelectInternal(pTabs, i, 1);
			return;
		}
	}
}

int xgeXuiTabsInit(xge_xui_tabs pTabs, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;
	xge_xui_widget pTabBar;
	xge_xui_widget pClient;

	if ( (pTabs == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pTabs, 0, sizeof(*pTabs));
	pTabBar = xgeXuiWidgetCreate();
	pClient = xgeXuiWidgetCreate();
	if ( (pTabBar == NULL) || (pClient == NULL) ) {
		xgeXuiWidgetFree(pTabBar);
		xgeXuiWidgetFree(pClient);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pTabs->pContext = pContext;
	pTabs->pWidget = pWidget;
	pTabs->pTabBarWidget = pTabBar;
	pTabs->pClientWidget = pClient;
	pTabs->pFont = pTheme->pFont;
	pTabs->iSelected = -1;
	pTabs->iHover = -1;
	pTabs->iActive = -1;
	pTabs->fTabWidth = 104.0f;
	pTabs->fTabHeight = 30.0f;
	pTabs->arrEnabled = pTabs->arrEnabledLocal;
	pTabs->arrDirty = pTabs->arrDirtyLocal;
	pTabs->arrIcons = pTabs->arrIconLocal;
	pTabs->arrIconSrc = pTabs->arrIconSrcLocal;
	pTabs->iTabColor = pTheme->iStateNormal;
	pTabs->iHoverColor = pTheme->iStateHover;
	pTabs->iActiveColor = pTheme->iAccentColor;
	pTabs->iFocusColor = pTheme->iStateFocus;
	pTabs->iDisabledColor = pTheme->iStateDisabled;
	pTabs->iTextColor = pTheme->iTextColor;
	pTabs->iActiveTextColor = XGE_COLOR_RGBA(248, 252, 255, 255);
	pTabs->iBorderColor = pTheme->iBorderColor;
	pTabs->iClientColor = XGE_COLOR_RGBA(248, 252, 255, 255);

	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, pTabs->iBorderColor);
	xgeXuiWidgetSetPaddingPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetGap(pWidget, 0.0f);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiTabsEventProc, pTabs);
	pWidget->pUser = pTabs;

	xgeXuiWidgetSetRole(pTabBar, XGE_XUI_WIDGET_ROLE_CONTAINER);
	xgeXuiWidgetSetLayout(pTabBar, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetSize(pTabBar, xgeXuiSizePercent(100.0f), xgeXuiSizePx(pTabs->fTabHeight));
	xgeXuiWidgetSetPaddingPx(pTabBar, 4.0f, 3.0f, 4.0f, 0.0f);
	xgeXuiWidgetSetGap(pTabBar, 2.0f);
	xgeXuiWidgetSetBackground(pTabBar, XGE_COLOR_RGBA(232, 243, 251, 255));
	xgeXuiWidgetSetClip(pTabBar, 1);

	xgeXuiWidgetSetRole(pClient, XGE_XUI_WIDGET_ROLE_CONTAINER);
	xgeXuiWidgetSetLayout(pClient, XGE_XUI_LAYOUT_STACK);
	xgeXuiWidgetSetSize(pClient, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetPaddingPx(pClient, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetSetBackground(pClient, pTabs->iClientColor);
	xgeXuiWidgetSetClip(pClient, 1);

	if ( (xgeXuiWidgetAddInternal(pWidget, pTabBar) != XGE_OK) ||
	     (xgeXuiWidgetAddInternal(pWidget, pClient) != XGE_OK) ) {
		xgeXuiTabsUnit(pTabs);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiTabsSetState(pTabs, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiTabsUnit(xge_xui_tabs pTabs)
{
	int i;

	if ( pTabs == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_TABS_PAGE_CAPACITY; i++ ) {
		xgeXuiButtonUnit(&pTabs->arrButton[i]);
	}
	xgeXuiWidgetFree(pTabs->pTabBarWidget);
	xgeXuiWidgetFree(pTabs->pClientWidget);
	if ( pTabs->pWidget != NULL && pTabs->pWidget->pUser == pTabs ) {
		pTabs->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pTabs->pWidget, NULL, NULL);
		pTabs->pWidget->procPaint = NULL;
	}
	memset(pTabs, 0, sizeof(*pTabs));
}

int xgeXuiTabsAddPage(xge_xui_tabs pTabs, const char* sTitle)
{
	xge_xui_widget pButtonWidget;
	xge_xui_widget pPageWidget;
	xge_xui_button pButton;
	int iIndex;

	if ( pTabs == NULL ) {
		return -1;
	}
	if ( pTabs->iItemCount >= XGE_XUI_TABS_PAGE_CAPACITY ) {
		return -1;
	}
	iIndex = pTabs->iItemCount;
	if ( (pTabs->arrButtonWidget[iIndex] != NULL) && (pTabs->arrPageWidget[iIndex] != NULL) ) {
		if ( sTitle == NULL ) {
			sTitle = "";
		}
		strncpy(pTabs->arrTitle[iIndex], sTitle, XGE_XUI_TABS_TITLE_CAPACITY - 1);
		pTabs->arrTitle[iIndex][XGE_XUI_TABS_TITLE_CAPACITY - 1] = 0;
		pTabs->arrEnabledLocal[iIndex] = 1;
		pTabs->arrDirtyLocal[iIndex] = 0;
		xgeXuiButtonSetText(&pTabs->arrButton[iIndex], pTabs->pFont, pTabs->arrTitle[iIndex]);
		xgeXuiWidgetSetVisible(pTabs->arrButtonWidget[iIndex], 1);
		xgeXuiWidgetSetVisible(pTabs->arrPageWidget[iIndex], 1);
		pTabs->iItemCount++;
		if ( pTabs->iSelected < 0 ) {
			pTabs->iSelected = iIndex;
		}
		__xgeXuiTabsRefreshPages(pTabs);
		return iIndex;
	}
	pButtonWidget = xgeXuiWidgetCreate();
	pPageWidget = xgeXuiWidgetCreate();
	if ( (pButtonWidget == NULL) || (pPageWidget == NULL) ) {
		xgeXuiWidgetFree(pButtonWidget);
		xgeXuiWidgetFree(pPageWidget);
		return -1;
	}
	pTabs->arrButtonWidget[iIndex] = pButtonWidget;
	pTabs->arrPageWidget[iIndex] = pPageWidget;
	pTabs->arrEnabledLocal[iIndex] = 1;
	pTabs->arrDirtyLocal[iIndex] = 0;
	if ( sTitle == NULL ) {
		sTitle = "";
	}
	strncpy(pTabs->arrTitle[iIndex], sTitle, XGE_XUI_TABS_TITLE_CAPACITY - 1);
	pTabs->arrTitle[iIndex][XGE_XUI_TABS_TITLE_CAPACITY - 1] = 0;

	pButton = &pTabs->arrButton[iIndex];
	if ( xgeXuiButtonInit(pButton, pTabs->pContext, pButtonWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pButtonWidget);
		xgeXuiWidgetFree(pPageWidget);
		pTabs->arrButtonWidget[iIndex] = NULL;
		pTabs->arrPageWidget[iIndex] = NULL;
		return -1;
	}
	xgeXuiWidgetSetSize(pButtonWidget, xgeXuiSizePx(pTabs->fTabWidth), xgeXuiSizePercent(100.0f));
	xgeXuiButtonSetText(pButton, pTabs->pFont, pTabs->arrTitle[iIndex]);
	xgeXuiButtonSetSelectable(pButton, 1);
	xgeXuiButtonSetClick(pButton, __xgeXuiTabsButtonClick, pTabs);
	xgeXuiButtonSetBadgeAnchor(pButton, XGE_XUI_BUTTON_BADGE_WIDGET_TOP_RIGHT);
	xgeXuiButtonSetBadgeOffset(pButton, -4.0f, 4.0f);
	xgeXuiButtonSetBadgeSize(pButton, 8.0f);

	xgeXuiWidgetSetRole(pPageWidget, XGE_XUI_WIDGET_ROLE_CONTAINER);
	xgeXuiWidgetSetLayout(pPageWidget, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pPageWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePercent(100.0f));
	xgeXuiWidgetSetGap(pPageWidget, 6.0f);
	xgeXuiWidgetSetClip(pPageWidget, 1);

	if ( (xgeXuiWidgetAddInternal(pTabs->pTabBarWidget, pButtonWidget) != XGE_OK) ||
	     (xgeXuiWidgetAddInternal(pTabs->pClientWidget, pPageWidget) != XGE_OK) ) {
		xgeXuiButtonUnit(pButton);
		xgeXuiWidgetFree(pButtonWidget);
		xgeXuiWidgetFree(pPageWidget);
		pTabs->arrButtonWidget[iIndex] = NULL;
		pTabs->arrPageWidget[iIndex] = NULL;
		return -1;
	}
	pTabs->iItemCount++;
	if ( pTabs->iSelected < 0 ) {
		pTabs->iSelected = iIndex;
	}
	__xgeXuiTabsRefreshPages(pTabs);
	return iIndex;
}

xge_xui_widget xgeXuiTabsGetTabBarWidget(xge_xui_tabs pTabs)
{
	return (pTabs != NULL) ? pTabs->pTabBarWidget : NULL;
}

xge_xui_widget xgeXuiTabsGetClientWidget(xge_xui_tabs pTabs)
{
	return (pTabs != NULL) ? pTabs->pClientWidget : NULL;
}

xge_xui_widget xgeXuiTabsGetPageWidget(xge_xui_tabs pTabs, int iIndex)
{
	if ( (pTabs == NULL) || (iIndex < 0) || (iIndex >= pTabs->iItemCount) ) {
		return NULL;
	}
	return pTabs->arrPageWidget[iIndex];
}

xge_xui_widget xgeXuiTabsGetButtonWidget(xge_xui_tabs pTabs, int iIndex)
{
	if ( (pTabs == NULL) || (iIndex < 0) || (iIndex >= pTabs->iItemCount) ) {
		return NULL;
	}
	return pTabs->arrButtonWidget[iIndex];
}

void xgeXuiTabsSetItems(xge_xui_tabs pTabs, const char** arrItems, int iCount)
{
	int i;

	if ( pTabs == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	if ( iCount > XGE_XUI_TABS_PAGE_CAPACITY ) {
		iCount = XGE_XUI_TABS_PAGE_CAPACITY;
	}
	pTabs->arrItems = arrItems;
	while ( pTabs->iItemCount < iCount ) {
		if ( xgeXuiTabsAddPage(pTabs, (arrItems != NULL) ? arrItems[pTabs->iItemCount] : "") < 0 ) {
			break;
		}
	}
	for ( i = 0; i < iCount; i++ ) {
		strncpy(pTabs->arrTitle[i], (arrItems != NULL && arrItems[i] != NULL) ? arrItems[i] : "", XGE_XUI_TABS_TITLE_CAPACITY - 1);
		pTabs->arrTitle[i][XGE_XUI_TABS_TITLE_CAPACITY - 1] = 0;
		xgeXuiButtonSetText(&pTabs->arrButton[i], pTabs->pFont, pTabs->arrTitle[i]);
		xgeXuiWidgetSetVisible(pTabs->arrButtonWidget[i], 1);
		xgeXuiWidgetSetVisible(pTabs->arrPageWidget[i], 1);
	}
	for ( i = iCount; i < pTabs->iItemCount; i++ ) {
		xgeXuiWidgetSetVisible(pTabs->arrButtonWidget[i], 0);
		xgeXuiWidgetSetVisible(pTabs->arrPageWidget[i], 0);
	}
	pTabs->iItemCount = iCount;
	if ( (pTabs->iSelected < 0) || (pTabs->iSelected >= iCount) || !__xgeXuiTabsItemEnabled(pTabs, pTabs->iSelected) ) {
		pTabs->iSelected = (iCount > 0) ? __xgeXuiTabsStepEnabled(pTabs, -1, 1) : -1;
	}
	__xgeXuiTabsRefreshPages(pTabs);
}

void xgeXuiTabsSetEnabledItems(xge_xui_tabs pTabs, const int* arrEnabled, int iCount)
{
	int i;

	if ( pTabs == NULL ) {
		return;
	}
	pTabs->arrEnabled = pTabs->arrEnabledLocal;
	pTabs->iEnabledCount = (iCount > 0) ? iCount : 0;
	for ( i = 0; i < pTabs->iItemCount; i++ ) {
		pTabs->arrEnabledLocal[i] = ((arrEnabled == NULL) || (i >= iCount)) ? 1 : (arrEnabled[i] != 0);
	}
	if ( (pTabs->iSelected >= 0) && !__xgeXuiTabsItemEnabled(pTabs, pTabs->iSelected) ) {
		pTabs->iSelected = __xgeXuiTabsStepEnabled(pTabs, pTabs->iSelected, 1);
	}
	__xgeXuiTabsRefreshPages(pTabs);
}

void xgeXuiTabsSetDirtyItems(xge_xui_tabs pTabs, const int* arrDirty, int iCount)
{
	int i;

	if ( pTabs == NULL ) {
		return;
	}
	pTabs->arrDirty = pTabs->arrDirtyLocal;
	pTabs->iDirtyCount = (iCount > 0) ? iCount : 0;
	for ( i = 0; i < pTabs->iItemCount; i++ ) {
		pTabs->arrDirtyLocal[i] = ((arrDirty != NULL) && (i < iCount) && (arrDirty[i] != 0));
	}
	__xgeXuiTabsRefreshPages(pTabs);
}

void xgeXuiTabsSetIcons(xge_xui_tabs pTabs, const xui_texture* arrIcons, const xge_rect_t* arrSrc, int iCount)
{
	int i;
	xge_rect_t tSrc;

	if ( pTabs == NULL ) {
		return;
	}
	pTabs->arrIcons = pTabs->arrIconLocal;
	pTabs->arrIconSrc = pTabs->arrIconSrcLocal;
	pTabs->iIconCount = (iCount > 0) ? iCount : 0;
	for ( i = 0; i < pTabs->iItemCount; i++ ) {
		pTabs->arrIconLocal[i] = ((arrIcons != NULL) && (i < iCount)) ? arrIcons[i] : NULL;
		tSrc = ((arrSrc != NULL) && (i < iCount)) ? arrSrc[i] : (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
		pTabs->arrIconSrcLocal[i] = tSrc;
		xgeXuiButtonSetIcon(&pTabs->arrButton[i], pTabs->arrIconLocal[i], tSrc);
		xgeXuiButtonSetIconLayout(&pTabs->arrButton[i], XGE_XUI_BUTTON_ICON_LEFT, 14.0f, 5.0f);
	}
}

void xgeXuiTabsSetFont(xge_xui_tabs pTabs, xui_font pFont)
{
	int i;

	if ( pTabs == NULL ) {
		return;
	}
	pTabs->pFont = pFont;
	for ( i = 0; i < pTabs->iItemCount; i++ ) {
		xgeXuiButtonSetText(&pTabs->arrButton[i], pFont, pTabs->arrTitle[i]);
	}
}

void xgeXuiTabsSetSelect(xge_xui_tabs pTabs, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pTabs == NULL ) {
		return;
	}
	pTabs->procSelect = procSelect;
	pTabs->pUser = pUser;
}

void xgeXuiTabsSetClose(xge_xui_tabs pTabs, xge_xui_select_proc procClose, int bCloseButtons, void* pUser)
{
	if ( pTabs == NULL ) {
		return;
	}
	pTabs->procClose = procClose;
	pTabs->bCloseButtons = (bCloseButtons != 0);
	pTabs->pUser = pUser;
}

void xgeXuiTabsSetSelected(xge_xui_tabs pTabs, int iIndex)
{
	__xgeXuiTabsSelectInternal(pTabs, iIndex, 0);
}

int xgeXuiTabsGetSelected(xge_xui_tabs pTabs)
{
	return (pTabs != NULL) ? pTabs->iSelected : -1;
}

void xgeXuiTabsSetTabSize(xge_xui_tabs pTabs, float fWidth, float fHeight)
{
	int i;

	if ( pTabs == NULL ) {
		return;
	}
	if ( fWidth < 24.0f ) {
		fWidth = 24.0f;
	}
	if ( fHeight < 18.0f ) {
		fHeight = 18.0f;
	}
	pTabs->fTabWidth = fWidth;
	pTabs->fTabHeight = fHeight;
	xgeXuiWidgetSetSize(pTabs->pTabBarWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(fHeight));
	for ( i = 0; i < pTabs->iItemCount; i++ ) {
		xgeXuiWidgetSetSize(pTabs->arrButtonWidget[i], xgeXuiSizePx(fWidth), xgeXuiSizePercent(100.0f));
	}
	xgeXuiWidgetMarkLayout(pTabs->pWidget);
}

void xgeXuiTabsSetScrollable(xge_xui_tabs pTabs, int bScrollable)
{
	if ( pTabs == NULL ) {
		return;
	}
	pTabs->bScrollable = (bScrollable != 0);
}

void xgeXuiTabsSetScroll(xge_xui_tabs pTabs, float fScrollX)
{
	if ( pTabs == NULL ) {
		return;
	}
	if ( fScrollX < 0.0f ) {
		fScrollX = 0.0f;
	}
	pTabs->fScrollX = fScrollX;
}

float xgeXuiTabsGetScroll(xge_xui_tabs pTabs)
{
	return (pTabs != NULL) ? pTabs->fScrollX : 0.0f;
}

void xgeXuiTabsSetColors(xge_xui_tabs pTabs, uint32_t iBackground, uint32_t iTab, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iText, uint32_t iActiveText)
{
	int i;

	if ( pTabs == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pTabs->pWidget, iBackground);
	pTabs->iTabColor = iTab;
	pTabs->iHoverColor = iHover;
	pTabs->iActiveColor = iActive;
	pTabs->iFocusColor = iFocus;
	pTabs->iDisabledColor = iDisabled;
	pTabs->iTextColor = iText;
	pTabs->iActiveTextColor = iActiveText;
	for ( i = 0; i < pTabs->iItemCount; i++ ) {
		__xgeXuiTabsApplyButtonStyle(pTabs, i);
	}
}

int xgeXuiTabsGetState(xge_xui_tabs pTabs)
{
	if ( pTabs == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiTabsSetState(pTabs, pTabs->iState & XGE_XUI_STATE_FOCUS);
	return pTabs->iState;
}

int xgeXuiTabsEvent(xge_xui_tabs pTabs, const xge_event_t* pEvent)
{
	int iIndex;

	if ( (pTabs == NULL) || (pTabs->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( !xgeXuiWidgetIsEnabled(pTabs->pWidget) ) {
		__xgeXuiTabsSetState(pTabs, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN ) {
		if ( (pTabs->pContext == NULL) || (pTabs->pContext->pFocus != pTabs->pWidget) || (pTabs->iItemCount <= 0) ) {
			return XGE_XUI_EVENT_CONTINUE;
		}
		if ( pEvent->iParam1 == XGE_KEY_RIGHT ) {
			iIndex = __xgeXuiTabsStepEnabled(pTabs, pTabs->iSelected, 1);
			__xgeXuiTabsSelectInternal(pTabs, iIndex, 1);
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( pEvent->iParam1 == XGE_KEY_LEFT ) {
			iIndex = __xgeXuiTabsStepEnabled(pTabs, pTabs->iSelected, -1);
			__xgeXuiTabsSelectInternal(pTabs, iIndex, 1);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int xgeXuiTabsEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiTabsEvent((xge_xui_tabs)pUser, pEvent);
}

void xgeXuiTabsPaintProc(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
}
