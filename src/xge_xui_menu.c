static int __xgeXuiMenuItemEnabled(const xge_xui_menu_item_t* pItem)
{
	return (pItem != NULL) && ((pItem->iState & XGE_XUI_MENU_ITEM_ENABLED) != 0) && (pItem->iType != XGE_XUI_MENU_ITEM_SEPARATOR);
}

static void __xgeXuiMenuCommit(xge_xui_menu pMenu, int iIndex);

static void __xgeXuiMenuDefaultMetrics(xge_xui_menu_metrics_t* pMetrics)
{
	if ( pMetrics == NULL ) {
		return;
	}
	pMetrics->fItemHeight = 24.0f;
	pMetrics->fSeparatorHeight = 9.0f;
	pMetrics->fPaddingX = 1.0f;
	pMetrics->fPaddingY = 1.0f;
	pMetrics->fMarkWidth = 22.0f;
	pMetrics->fIconWidth = 2.0f;
	pMetrics->fShortcutGap = 20.0f;
	pMetrics->fArrowWidth = 16.0f;
	pMetrics->fMinWidth = 112.0f;
	pMetrics->fMaxHeight = 0.0f;
}

static void __xgeXuiMenuDefaultColors(xge_xui_menu_colors_t* pColors)
{
	if ( pColors == NULL ) {
		return;
	}
	pColors->iPanel = XGE_COLOR_RGBA(250, 252, 255, 255);
	pColors->iBorder = XGE_COLOR_RGBA(122, 164, 202, 255);
	pColors->iRow = XGE_COLOR_RGBA(250, 252, 255, 255);
	pColors->iHover = XGE_COLOR_RGBA(54, 125, 190, 255);
	pColors->iText = XGE_COLOR_RGBA(28, 60, 94, 255);
	pColors->iDisabledText = XGE_COLOR_RGBA(142, 152, 166, 210);
	pColors->iShortcutText = XGE_COLOR_RGBA(84, 111, 140, 255);
	pColors->iDangerText = XGE_COLOR_RGBA(184, 54, 54, 255);
	pColors->iMark = XGE_COLOR_RGBA(37, 94, 145, 255);
	pColors->iSeparator = XGE_COLOR_RGBA(202, 218, 232, 255);
}

static void __xgeXuiMenuInvalidate(xge_xui_menu pMenu)
{
	if ( pMenu == NULL ) {
		return;
	}
	pMenu->bLayoutDirty = 1;
	if ( pMenu->iUpdateLock == 0 ) {
		xgeXuiWidgetMarkPaint(pMenu->pContentWidget);
	}
}

static int __xgeXuiMenuValidType(int iType)
{
	if ( (iType < XGE_XUI_MENU_ITEM_NORMAL) || (iType > XGE_XUI_MENU_ITEM_SUBMENU) ) {
		return XGE_XUI_MENU_ITEM_NORMAL;
	}
	return iType;
}

static void __xgeXuiMenuNormalizeItem(xge_xui_menu_item_t* pItem)
{
	if ( pItem == NULL ) {
		return;
	}
	pItem->iType = __xgeXuiMenuValidType(pItem->iType);
	if ( pItem->iType == XGE_XUI_MENU_ITEM_SEPARATOR ) {
		pItem->iState &= ~XGE_XUI_MENU_ITEM_ENABLED;
		pItem->sText = "";
		pItem->sShortcut = "";
	}
	if ( pItem->sText == NULL ) {
		pItem->sText = "";
	}
	if ( pItem->sShortcut == NULL ) {
		pItem->sShortcut = "";
	}
}

static void __xgeXuiMenuMeasure(xge_xui_menu pMenu)
{
	xge_vec2_t tText;
	xge_vec2_t tShortcut;
	float fTextW;
	float fShortcutW;
	float fHeight;
	float fWidth;
	int i;

	if ( pMenu == NULL || pMenu->bLayoutDirty == 0 ) {
		return;
	}
	fTextW = 0.0f;
	fShortcutW = 0.0f;
	fHeight = pMenu->tMetrics.fPaddingY;
	for ( i = 0; i < pMenu->iItemCount; i++ ) {
		if ( pMenu->arrItems[i].iType == XGE_XUI_MENU_ITEM_SEPARATOR ) {
			pMenu->arrItemRect[i] = (xge_rect_t){ 0.0f, fHeight, 0.0f, pMenu->tMetrics.fSeparatorHeight };
			fHeight += pMenu->tMetrics.fSeparatorHeight;
			continue;
		}
		tText = __xgeXuiHostMeasureText(pMenu->pFont, pMenu->arrItems[i].sText);
		tShortcut = __xgeXuiHostMeasureText(pMenu->pFont, pMenu->arrItems[i].sShortcut);
		if ( tText.fX > fTextW ) {
			fTextW = tText.fX;
		}
		if ( tShortcut.fX > fShortcutW ) {
			fShortcutW = tShortcut.fX;
		}
		pMenu->arrItemRect[i] = (xge_rect_t){ 0.0f, fHeight, 0.0f, pMenu->tMetrics.fItemHeight };
		fHeight += pMenu->tMetrics.fItemHeight;
	}
	fWidth = pMenu->tMetrics.fPaddingX * 2.0f + pMenu->tMetrics.fMarkWidth + pMenu->tMetrics.fIconWidth + fTextW + pMenu->tMetrics.fArrowWidth;
	if ( fShortcutW > 0.0f ) {
		fWidth += pMenu->tMetrics.fShortcutGap + fShortcutW;
	}
	if ( fWidth < pMenu->tMetrics.fMinWidth ) {
		fWidth = pMenu->tMetrics.fMinWidth;
	}
	for ( i = 0; i < pMenu->iItemCount; i++ ) {
		pMenu->arrItemRect[i].fX = pMenu->tMetrics.fPaddingX;
		pMenu->arrItemRect[i].fW = fWidth - pMenu->tMetrics.fPaddingX * 2.0f;
	}
	fHeight += pMenu->tMetrics.fPaddingY;
	pMenu->fContentW = fWidth;
	pMenu->fContentH = (fHeight > 1.0f) ? fHeight : 1.0f;
	pMenu->fTextW = fTextW;
	pMenu->fShortcutW = fShortcutW;
	pMenu->bLayoutDirty = 0;
}

static int __xgeXuiMenuIndexAt(xge_xui_menu pMenu, float fX, float fY)
{
	int i;

	if ( pMenu == NULL ) {
		return -1;
	}
	__xgeXuiMenuMeasure(pMenu);
	for ( i = 0; i < pMenu->iItemCount; i++ ) {
		if ( __xgeXuiRectContains(pMenu->arrItemRect[i], fX, fY) ) {
			return (pMenu->arrItems[i].iType == XGE_XUI_MENU_ITEM_SEPARATOR) ? -1 : i;
		}
	}
	return -1;
}

static int __xgeXuiMenuNextEnabled(xge_xui_menu pMenu, int iCurrent, int iStep)
{
	int i;
	int iIndex;

	if ( (pMenu == NULL) || (pMenu->iItemCount <= 0) ) {
		return -1;
	}
	iIndex = iCurrent;
	for ( i = 0; i < pMenu->iItemCount; i++ ) {
		iIndex += iStep;
		if ( iIndex < 0 ) {
			iIndex = pMenu->iItemCount - 1;
		} else if ( iIndex >= pMenu->iItemCount ) {
			iIndex = 0;
		}
		if ( __xgeXuiMenuItemEnabled(&pMenu->arrItems[iIndex]) ) {
			return iIndex;
		}
	}
	return -1;
}

static void __xgeXuiMenuCloseSubmenu(xge_xui_menu pMenu)
{
	if ( (pMenu == NULL) || (pMenu->pOpenSubmenu == NULL) ) {
		return;
	}
	xgeXuiMenuClose(pMenu->pOpenSubmenu);
	pMenu->pOpenSubmenu = NULL;
}

static void __xgeXuiMenuOpenSubmenu(xge_xui_menu pMenu, int iIndex)
{
	xge_xui_menu pSubmenu;
	xge_rect_t tRect;

	if ( (pMenu == NULL) || (iIndex < 0) || (iIndex >= pMenu->iItemCount) ) {
		return;
	}
	pSubmenu = pMenu->arrItems[iIndex].pSubmenu;
	if ( pMenu->arrItems[iIndex].iType != XGE_XUI_MENU_ITEM_SUBMENU || pSubmenu == NULL || !__xgeXuiMenuItemEnabled(&pMenu->arrItems[iIndex]) ) {
		__xgeXuiMenuCloseSubmenu(pMenu);
		return;
	}
	if ( pMenu->pOpenSubmenu == pSubmenu && xgeXuiMenuIsOpen(pSubmenu) ) {
		return;
	}
	__xgeXuiMenuCloseSubmenu(pMenu);
	__xgeXuiMenuMeasure(pMenu);
	tRect = pMenu->arrItemRect[iIndex];
	tRect.fX += pMenu->pContentWidget->tRect.fX + tRect.fW;
	tRect.fY += pMenu->pContentWidget->tRect.fY;
	tRect.fW = 0.0f;
	pSubmenu->pParentMenu = pMenu;
	pSubmenu->iParentItem = iIndex;
	if ( pSubmenu->procSelect == NULL ) {
		pSubmenu->procSelect = pMenu->procSelect;
		pSubmenu->pUser = pMenu->pUser;
	}
	xgeXuiPopupSetClosePolicy(&pSubmenu->tPopup, pMenu->tPopup.iOutsidePolicy, pMenu->tPopup.iOwnerPolicy, pMenu->tPopup.iEscapePolicy);
	xgeXuiMenuOpenAt(pSubmenu, pMenu->pOwner, tRect.fX, tRect.fY);
	pMenu->pOpenSubmenu = pSubmenu;
}

static void __xgeXuiMenuSetHover(xge_xui_menu pMenu, int iIndex)
{
	if ( pMenu == NULL ) {
		return;
	}
	if ( (iIndex < 0) || (iIndex >= pMenu->iItemCount) || !__xgeXuiMenuItemEnabled(&pMenu->arrItems[iIndex]) ) {
		iIndex = -1;
	}
	if ( pMenu->iHover == iIndex ) {
		return;
	}
	pMenu->iHover = iIndex;
	if ( iIndex >= 0 ) {
		__xgeXuiMenuOpenSubmenu(pMenu, iIndex);
	} else {
		__xgeXuiMenuCloseSubmenu(pMenu);
	}
	xgeXuiWidgetMarkPaint(pMenu->pContentWidget);
}

static void __xgeXuiMenuApplyCheckState(xge_xui_menu pMenu, int iIndex)
{
	int i;

	if ( (pMenu == NULL) || (iIndex < 0) || (iIndex >= pMenu->iItemCount) ) {
		return;
	}
	if ( pMenu->arrItems[iIndex].iType == XGE_XUI_MENU_ITEM_CHECK ) {
		pMenu->arrItems[iIndex].iState ^= XGE_XUI_MENU_ITEM_CHECKED;
		return;
	}
	if ( pMenu->arrItems[iIndex].iType != XGE_XUI_MENU_ITEM_RADIO ) {
		return;
	}
	for ( i = iIndex - 1; i >= 0 && pMenu->arrItems[i].iType == XGE_XUI_MENU_ITEM_RADIO; i-- ) {
		pMenu->arrItems[i].iState &= ~XGE_XUI_MENU_ITEM_CHECKED;
	}
	for ( i = iIndex + 1; i < pMenu->iItemCount && pMenu->arrItems[i].iType == XGE_XUI_MENU_ITEM_RADIO; i++ ) {
		pMenu->arrItems[i].iState &= ~XGE_XUI_MENU_ITEM_CHECKED;
	}
	pMenu->arrItems[iIndex].iState |= XGE_XUI_MENU_ITEM_CHECKED;
}

static int __xgeXuiMenuAsciiUpper(int iChar)
{
	return (iChar >= 'a' && iChar <= 'z') ? (iChar - 'a' + 'A') : iChar;
}

static int __xgeXuiMenuTextEqualFoldN(const char* sText, int iLen, const char* sCompare)
{
	int i;

	if ( (sText == NULL) || (sCompare == NULL) || (iLen < 0) ) {
		return 0;
	}
	for ( i = 0; i < iLen; i++ ) {
		if ( sCompare[i] == 0 || __xgeXuiMenuAsciiUpper((unsigned char)sText[i]) != __xgeXuiMenuAsciiUpper((unsigned char)sCompare[i]) ) {
			return 0;
		}
	}
	return sCompare[iLen] == 0;
}

static int __xgeXuiMenuParseFunctionKey(const char* sText, int iLen)
{
	int iValue;
	int i;

	if ( (sText == NULL) || (iLen < 2) || (__xgeXuiMenuAsciiUpper((unsigned char)sText[0]) != 'F') ) {
		return 0;
	}
	iValue = 0;
	for ( i = 1; i < iLen; i++ ) {
		if ( sText[i] < '0' || sText[i] > '9' ) {
			return 0;
		}
		iValue = iValue * 10 + (sText[i] - '0');
	}
	if ( iValue < 1 || iValue > 24 ) {
		return 0;
	}
	return XGE_KEY_F10 + (iValue - 10);
}

static int __xgeXuiMenuShortcutKeyFromToken(const char* sText, int iLen)
{
	if ( (sText == NULL) || (iLen <= 0) ) {
		return 0;
	}
	if ( iLen == 1 ) {
		return __xgeXuiMenuAsciiUpper((unsigned char)sText[0]);
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "Del") || __xgeXuiMenuTextEqualFoldN(sText, iLen, "Delete") ) {
		return XGE_KEY_DELETE;
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "Enter") || __xgeXuiMenuTextEqualFoldN(sText, iLen, "Return") ) {
		return XGE_KEY_ENTER;
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "Esc") || __xgeXuiMenuTextEqualFoldN(sText, iLen, "Escape") ) {
		return XGE_KEY_ESCAPE;
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "Space") ) {
		return XGE_KEY_SPACE;
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "Tab") ) {
		return XGE_KEY_TAB;
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "Backspace") ) {
		return XGE_KEY_BACKSPACE;
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "Left") ) {
		return XGE_KEY_LEFT;
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "Right") ) {
		return XGE_KEY_RIGHT;
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "Up") ) {
		return XGE_KEY_UP;
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "Down") ) {
		return XGE_KEY_DOWN;
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "PageUp") || __xgeXuiMenuTextEqualFoldN(sText, iLen, "PgUp") ) {
		return XGE_KEY_PAGE_UP;
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "PageDown") || __xgeXuiMenuTextEqualFoldN(sText, iLen, "PgDn") ) {
		return XGE_KEY_PAGE_DOWN;
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "Home") ) {
		return XGE_KEY_HOME;
	}
	if ( __xgeXuiMenuTextEqualFoldN(sText, iLen, "End") ) {
		return XGE_KEY_END;
	}
	return __xgeXuiMenuParseFunctionKey(sText, iLen);
}

static int __xgeXuiMenuShortcutMatches(const char* sShortcut, const xge_event_t* pEvent)
{
	const char* p;
	const char* pToken;
	int iLen;
	int iModifiers;
	int iKey;
	int iEventKey;
	int iEventModifiers;

	if ( (sShortcut == NULL) || (sShortcut[0] == 0) || (pEvent == NULL) || (pEvent->iType != XGE_EVENT_KEY_DOWN) ) {
		return 0;
	}
	p = sShortcut;
	iModifiers = 0;
	iKey = 0;
	while ( *p != 0 ) {
		while ( *p == ' ' || *p == '\t' || *p == '+' ) {
			p++;
		}
		pToken = p;
		while ( *p != 0 && *p != '+' ) {
			p++;
		}
		iLen = (int)(p - pToken);
		while ( iLen > 0 && (pToken[iLen - 1] == ' ' || pToken[iLen - 1] == '\t') ) {
			iLen--;
		}
		if ( iLen <= 0 ) {
			continue;
		}
		if ( __xgeXuiMenuTextEqualFoldN(pToken, iLen, "Ctrl") || __xgeXuiMenuTextEqualFoldN(pToken, iLen, "Control") ) {
			iModifiers |= XGE_KEY_MOD_CTRL;
		} else if ( __xgeXuiMenuTextEqualFoldN(pToken, iLen, "Alt") ) {
			iModifiers |= XGE_KEY_MOD_ALT;
		} else if ( __xgeXuiMenuTextEqualFoldN(pToken, iLen, "Shift") ) {
			iModifiers |= XGE_KEY_MOD_SHIFT;
		} else if ( __xgeXuiMenuTextEqualFoldN(pToken, iLen, "Super") || __xgeXuiMenuTextEqualFoldN(pToken, iLen, "Win") || __xgeXuiMenuTextEqualFoldN(pToken, iLen, "Cmd") ) {
			iModifiers |= XGE_KEY_MOD_SUPER;
		} else {
			iKey = __xgeXuiMenuShortcutKeyFromToken(pToken, iLen);
			if ( iKey == 0 ) {
				return 0;
			}
		}
	}
	if ( iKey == 0 ) {
		return 0;
	}
	iEventKey = __xgeXuiMenuAsciiUpper(pEvent->iParam1);
	iEventModifiers = pEvent->iParam2 & (XGE_KEY_MOD_SHIFT | XGE_KEY_MOD_CTRL | XGE_KEY_MOD_ALT | XGE_KEY_MOD_SUPER);
	return (iEventKey == iKey) && (iEventModifiers == iModifiers);
}

static int __xgeXuiMenuCommitShortcut(xge_xui_menu pMenu, const xge_event_t* pEvent)
{
	int i;

	if ( (pMenu == NULL) || (pEvent == NULL) ) {
		return 0;
	}
	if ( (pMenu->pOpenSubmenu != NULL) && xgeXuiMenuIsOpen(pMenu->pOpenSubmenu) && __xgeXuiMenuCommitShortcut(pMenu->pOpenSubmenu, pEvent) ) {
		return 1;
	}
	for ( i = 0; i < pMenu->iItemCount; i++ ) {
		if ( !__xgeXuiMenuItemEnabled(&pMenu->arrItems[i]) || pMenu->arrItems[i].iType == XGE_XUI_MENU_ITEM_SUBMENU ) {
			continue;
		}
		if ( __xgeXuiMenuShortcutMatches(pMenu->arrItems[i].sShortcut, pEvent) ) {
			__xgeXuiMenuSetHover(pMenu, i);
			__xgeXuiMenuCommit(pMenu, i);
			return 1;
		}
	}
	return 0;
}

static void __xgeXuiMenuCommit(xge_xui_menu pMenu, int iIndex)
{
	xge_xui_menu pRoot;
	xge_xui_menu_item_t* pItem;

	if ( (pMenu == NULL) || (iIndex < 0) || (iIndex >= pMenu->iItemCount) ) {
		return;
	}
	pItem = &pMenu->arrItems[iIndex];
	if ( !__xgeXuiMenuItemEnabled(pItem) || pItem->iType == XGE_XUI_MENU_ITEM_SUBMENU ) {
		if ( pItem->iType == XGE_XUI_MENU_ITEM_SUBMENU ) {
			__xgeXuiMenuOpenSubmenu(pMenu, iIndex);
		}
		return;
	}
	__xgeXuiMenuApplyCheckState(pMenu, iIndex);
	pMenu->iSelectCount++;
	pRoot = pMenu;
	while ( pRoot->pParentMenu != NULL ) {
		pRoot = pRoot->pParentMenu;
	}
	xgeXuiMenuClose(pRoot);
	if ( pMenu->procSelect != NULL ) {
		pMenu->procSelect(pMenu->pOwner, iIndex, pItem->iValue, pMenu->pUser);
	}
	if ( (pRoot->pContext != NULL) && (pRoot->pOwner != NULL) ) {
		xgeXuiSetFocus(pRoot->pContext, pRoot->pOwner);
	}
}

static void __xgeXuiMenuPopupClose(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_menu pMenu;

	(void)pWidget;
	pMenu = (xge_xui_menu)pUser;
	if ( pMenu == NULL ) {
		return;
	}
	__xgeXuiMenuCloseSubmenu(pMenu);
	pMenu->iHover = -1;
	if ( (pMenu->pContext != NULL) && (pMenu->pOwner != NULL) && (pMenu->pParentMenu == NULL) ) {
		xgeXuiSetFocus(pMenu->pContext, pMenu->pOwner);
	}
}

static void __xgeXuiMenuDrawCheck(xge_rect_t tRect, uint32_t iColor, int bRadio)
{
	xge_rect_t tMark;
	uint32_t iBox;

	iBox = XGE_COLOR_RGBA(104, 143, 176, 255);
	tMark = (xge_rect_t){ tRect.fX + (tRect.fW - 16.0f) * 0.5f, tRect.fY + (tRect.fH - 16.0f) * 0.5f, 16.0f, 16.0f };
	if ( bRadio ) {
		__xgeXuiHostDrawCircleStroke(tMark.fX + 8.0f, tMark.fY + 8.0f, 7.0f, 1.5f, iBox);
		__xgeXuiHostDrawCircle(tMark.fX + 8.0f, tMark.fY + 8.0f, 4.0f, iColor);
	} else {
		__xgeXuiHostDrawBorderRect(tMark, 1.0f, iBox);
		__xgeXuiHostDrawLine(tMark.fX + 4.0f, tMark.fY + 8.0f, tMark.fX + 7.0f, tMark.fY + 11.0f, 2.0f, iColor);
		__xgeXuiHostDrawLine(tMark.fX + 7.0f, tMark.fY + 11.0f, tMark.fX + 13.0f, tMark.fY + 5.0f, 2.0f, iColor);
	}
}

static void __xgeXuiMenuPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_menu pMenu;
	xge_xui_menu_item_t* pItem;
	xge_rect_t tRect;
	xge_rect_t tMark;
	xge_rect_t tText;
	xge_rect_t tShortcut;
	xge_rect_t tSep;
	uint32_t iText;
	int i;
	int bHover;

	(void)pWidget;
	pMenu = (xge_xui_menu)pUser;
	if ( pMenu == NULL ) {
		return;
	}
	__xgeXuiMenuMeasure(pMenu);
	__xgeXuiHostDrawRect(pMenu->pContentWidget->tRect, pMenu->tColors.iPanel);
	for ( i = 0; i < pMenu->iItemCount; i++ ) {
		pItem = &pMenu->arrItems[i];
		tRect = pMenu->arrItemRect[i];
		tRect.fX += pMenu->pContentWidget->tRect.fX;
		tRect.fY += pMenu->pContentWidget->tRect.fY;
		if ( pItem->iType == XGE_XUI_MENU_ITEM_SEPARATOR ) {
			tSep = (xge_rect_t){ tRect.fX + 4.0f, tRect.fY + tRect.fH * 0.5f, tRect.fW - 8.0f, 1.0f };
			__xgeXuiHostDrawRect(tSep, pMenu->tColors.iSeparator);
			continue;
		}
		bHover = (i == pMenu->iHover) && __xgeXuiMenuItemEnabled(pItem);
		if ( bHover ) {
			__xgeXuiHostDrawRect(tRect, pMenu->tColors.iHover);
		} else {
			__xgeXuiHostDrawRect(tRect, pMenu->tColors.iRow);
		}
		tMark = tRect;
		tMark.fW = pMenu->tMetrics.fMarkWidth;
		if ( (pItem->iState & XGE_XUI_MENU_ITEM_CHECKED) != 0 ) {
			__xgeXuiMenuDrawCheck(tMark, pMenu->tColors.iMark, pItem->iType == XGE_XUI_MENU_ITEM_RADIO);
		}
		if ( pItem->iIcon != 0 ) {
			tSep = (xge_rect_t){ tRect.fX + 3.0f, tRect.fY + (tRect.fH - 16.0f) * 0.5f, 16.0f, 16.0f };
			__xgeXuiBuiltinAssetDraw(tSep, XGE_XUI_ASSET_MENU_ICON, bHover ? XGE_COLOR_RGBA(235, 246, 255, 255) : pMenu->tColors.iMark);
		}
		iText = bHover ? XGE_COLOR_RGBA(255, 255, 255, 255) : ((pItem->iState & XGE_XUI_MENU_ITEM_DANGER) ? pMenu->tColors.iDangerText : pMenu->tColors.iText);
		if ( !__xgeXuiMenuItemEnabled(pItem) ) {
			iText = pMenu->tColors.iDisabledText;
		}
		tText = tRect;
		tText.fX += pMenu->tMetrics.fMarkWidth + pMenu->tMetrics.fIconWidth;
		tText.fW = tRect.fX + tRect.fW - pMenu->tMetrics.fArrowWidth - tText.fX;
		if ( pItem->sShortcut != NULL && pItem->sShortcut[0] != 0 ) {
			tText.fW -= pMenu->tMetrics.fShortcutGap + pMenu->fShortcutW;
		}
		__xgeXuiHostDrawTextRect(pMenu->pFont, pItem->sText, tText, iText, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		if ( pItem->sShortcut != NULL && pItem->sShortcut[0] != 0 ) {
			tShortcut = tRect;
			tShortcut.fX = tRect.fX + tRect.fW - pMenu->tMetrics.fArrowWidth - pMenu->fShortcutW;
			tShortcut.fW = pMenu->fShortcutW;
			__xgeXuiHostDrawTextRect(pMenu->pFont, pItem->sShortcut, tShortcut, bHover ? XGE_COLOR_RGBA(235, 246, 255, 255) : pMenu->tColors.iShortcutText, XGE_TEXT_ALIGN_RIGHT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		if ( pItem->iType == XGE_XUI_MENU_ITEM_SUBMENU ) {
			tSep = (xge_rect_t){ tRect.fX + tRect.fW - pMenu->tMetrics.fArrowWidth + 5.0f, tRect.fY + tRect.fH * 0.5f - 4.0f, 0.0f, 0.0f };
			__xgeXuiHostDrawRect((xge_rect_t){ tSep.fX, tSep.fY, 2.0f, 2.0f }, iText);
			__xgeXuiHostDrawRect((xge_rect_t){ tSep.fX + 2.0f, tSep.fY + 2.0f, 2.0f, 2.0f }, iText);
			__xgeXuiHostDrawRect((xge_rect_t){ tSep.fX, tSep.fY + 4.0f, 2.0f, 2.0f }, iText);
		}
	}
}

static int __xgeXuiMenuEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	xge_xui_menu pMenu;
	float fX;
	float fY;
	int iIndex;

	(void)pWidget;
	pMenu = (xge_xui_menu)pUser;
	if ( (pMenu == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
			fX = pEvent->fX - pMenu->pContentWidget->tRect.fX;
			fY = pEvent->fY - pMenu->pContentWidget->tRect.fY;
			__xgeXuiMenuSetHover(pMenu, __xgeXuiMenuIndexAt(pMenu, fX, fY));
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_DOWN:
			fX = pEvent->fX - pMenu->pContentWidget->tRect.fX;
			fY = pEvent->fY - pMenu->pContentWidget->tRect.fY;
			iIndex = __xgeXuiMenuIndexAt(pMenu, fX, fY);
			__xgeXuiMenuSetHover(pMenu, iIndex);
			if ( iIndex >= 0 ) {
				__xgeXuiMenuCommit(pMenu, iIndex);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return (iIndex >= 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_UP:
			fX = pEvent->fX - pMenu->pContentWidget->tRect.fX;
			fY = pEvent->fY - pMenu->pContentWidget->tRect.fY;
			iIndex = __xgeXuiMenuIndexAt(pMenu, fX, fY);
			if ( iIndex >= 0 ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_KEY_DOWN:
			if ( __xgeXuiMenuCommitShortcut(pMenu, pEvent) ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_DOWN ) {
				__xgeXuiMenuSetHover(pMenu, __xgeXuiMenuNextEnabled(pMenu, pMenu->iHover, 1));
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_UP ) {
				__xgeXuiMenuSetHover(pMenu, __xgeXuiMenuNextEnabled(pMenu, pMenu->iHover, -1));
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_RIGHT && pMenu->iHover >= 0 ) {
				__xgeXuiMenuOpenSubmenu(pMenu, pMenu->iHover);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_LEFT && pMenu->pParentMenu != NULL ) {
				xgeXuiMenuClose(pMenu);
				xgeXuiSetFocus(pMenu->pParentMenu->pContext, pMenu->pParentMenu->pContentWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_ENTER || pEvent->iParam1 == XGE_KEY_SPACE ) {
				__xgeXuiMenuCommit(pMenu, pMenu->iHover);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_ESCAPE ) {
				xgeXuiMenuClose(pMenu);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiMenuInit(xge_xui_menu pMenu, xge_xui_context pContext)
{
	const xge_xui_theme_t* pTheme;

	if ( (pMenu == NULL) || (pContext == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pMenu, 0, sizeof(*pMenu));
	pTheme = xgeXuiGetTheme(pContext);
	pMenu->pContext = pContext;
	pMenu->pFont = pTheme->pFont;
	pMenu->iHover = -1;
	pMenu->iParentItem = -1;
	__xgeXuiMenuDefaultMetrics(&pMenu->tMetrics);
	__xgeXuiMenuDefaultColors(&pMenu->tColors);
	if ( xgeXuiGetChromeStyle(pContext) != NULL ) {
		pMenu->tMetrics = xgeXuiGetChromeStyle(pContext)->tMenuMetrics;
		pMenu->tColors = xgeXuiGetChromeStyle(pContext)->tMenuColors;
	}
	pMenu->pPopupWidget = xgeXuiWidgetCreate();
	pMenu->pContentWidget = xgeXuiWidgetCreate();
	if ( (pMenu->pPopupWidget == NULL) || (pMenu->pContentWidget == NULL) ) {
		xgeXuiWidgetFree(pMenu->pPopupWidget);
		xgeXuiWidgetFree(pMenu->pContentWidget);
		memset(pMenu, 0, sizeof(*pMenu));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	__xgeXuiControlWidgetInit(pMenu->pContentWidget, 1);
	xgeXuiWidgetSetEvent(pMenu->pContentWidget, __xgeXuiMenuEventProc, pMenu);
	pMenu->pContentWidget->procPaint = __xgeXuiMenuPaintProc;
	pMenu->pContentWidget->pUser = pMenu;
	xgeXuiPopupInit(&pMenu->tPopup, pContext, pMenu->pPopupWidget);
	xgeXuiPopupSetContentWidget(&pMenu->tPopup, pMenu->pContentWidget);
	xgeXuiPopupSetClose(&pMenu->tPopup, __xgeXuiMenuPopupClose, pMenu);
	xgeXuiPopupSetClosePolicy(&pMenu->tPopup, XGE_XUI_POPUP_OUTSIDE_CLOSE, XGE_XUI_POPUP_OWNER_CLOSE, XGE_XUI_POPUP_ESCAPE_CLOSE);
	xgeXuiPopupSetConsumeInside(&pMenu->tPopup, 1);
	xgeXuiPopupSetFocusPolicy(&pMenu->tPopup, XGE_XUI_POPUP_FOCUS_CUSTOM, pMenu->pContentWidget);
	xgeXuiPopupSetBackground(&pMenu->tPopup, pMenu->tColors.iPanel);
	xgeXuiPopupSetBorder(&pMenu->tPopup, pMenu->tColors.iBorder);
	if ( xgeXuiOverlayAttach(pContext, pMenu->pPopupWidget, NULL, XGE_XUI_LAYER_POPUP) != XGE_OK ) {
		xgeXuiPopupUnit(&pMenu->tPopup);
		xgeXuiWidgetFree(pMenu->pPopupWidget);
		xgeXuiWidgetFree(pMenu->pContentWidget);
		memset(pMenu, 0, sizeof(*pMenu));
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pMenu->bLayoutDirty = 1;
	return XGE_OK;
}

void xgeXuiMenuUnit(xge_xui_menu pMenu)
{
	xge_xui_widget pPopupWidget;

	if ( pMenu == NULL ) {
		return;
	}
	pPopupWidget = pMenu->pPopupWidget;
	xgeXuiMenuClose(pMenu);
	xgeXuiPopupUnit(&pMenu->tPopup);
	xgeXuiWidgetFree(pPopupWidget);
	memset(pMenu, 0, sizeof(*pMenu));
}

void xgeXuiMenuBeginUpdate(xge_xui_menu pMenu)
{
	if ( pMenu != NULL ) {
		pMenu->iUpdateLock++;
	}
}

void xgeXuiMenuEndUpdate(xge_xui_menu pMenu)
{
	if ( pMenu == NULL ) {
		return;
	}
	if ( pMenu->iUpdateLock > 0 ) {
		pMenu->iUpdateLock--;
	}
	if ( pMenu->iUpdateLock == 0 ) {
		__xgeXuiMenuMeasure(pMenu);
		xgeXuiWidgetMarkPaint(pMenu->pContentWidget);
	}
}

void xgeXuiMenuClear(xge_xui_menu pMenu)
{
	if ( pMenu == NULL ) {
		return;
	}
	pMenu->iItemCount = 0;
	pMenu->iHover = -1;
	__xgeXuiMenuCloseSubmenu(pMenu);
	__xgeXuiMenuInvalidate(pMenu);
}

int xgeXuiMenuAddItem(xge_xui_menu pMenu, const xge_xui_menu_item_t* pItem)
{
	if ( (pMenu == NULL) || (pItem == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pMenu->iItemCount >= XGE_XUI_MENU_ITEM_CAPACITY ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pMenu->arrItems[pMenu->iItemCount] = *pItem;
	__xgeXuiMenuNormalizeItem(&pMenu->arrItems[pMenu->iItemCount]);
	pMenu->iItemCount++;
	__xgeXuiMenuInvalidate(pMenu);
	if ( pMenu->iUpdateLock == 0 ) {
		__xgeXuiMenuMeasure(pMenu);
	}
	return XGE_OK;
}

int xgeXuiMenuAddSeparator(xge_xui_menu pMenu)
{
	xge_xui_menu_item_t tItem;

	memset(&tItem, 0, sizeof(tItem));
	tItem.iType = XGE_XUI_MENU_ITEM_SEPARATOR;
	return xgeXuiMenuAddItem(pMenu, &tItem);
}

void xgeXuiMenuSetItems(xge_xui_menu pMenu, const xge_xui_menu_item_t* arrItems, int iCount)
{
	int i;

	if ( pMenu == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	if ( arrItems == NULL ) {
		iCount = 0;
	}
	if ( iCount > XGE_XUI_MENU_ITEM_CAPACITY ) {
		iCount = XGE_XUI_MENU_ITEM_CAPACITY;
	}
	pMenu->iItemCount = iCount;
	for ( i = 0; i < iCount; i++ ) {
		pMenu->arrItems[i] = arrItems[i];
		__xgeXuiMenuNormalizeItem(&pMenu->arrItems[i]);
	}
	pMenu->iHover = -1;
	__xgeXuiMenuCloseSubmenu(pMenu);
	__xgeXuiMenuInvalidate(pMenu);
	if ( pMenu->iUpdateLock == 0 ) {
		__xgeXuiMenuMeasure(pMenu);
	}
}

void xgeXuiMenuSetItemState(xge_xui_menu pMenu, int iIndex, int iState)
{
	if ( (pMenu == NULL) || (iIndex < 0) || (iIndex >= pMenu->iItemCount) ) {
		return;
	}
	pMenu->arrItems[iIndex].iState = iState;
	__xgeXuiMenuNormalizeItem(&pMenu->arrItems[iIndex]);
	if ( pMenu->iHover == iIndex && !__xgeXuiMenuItemEnabled(&pMenu->arrItems[iIndex]) ) {
		pMenu->iHover = -1;
	}
	xgeXuiWidgetMarkPaint(pMenu->pContentWidget);
}

void xgeXuiMenuSetFont(xge_xui_menu pMenu, xui_font pFont)
{
	if ( pMenu == NULL ) {
		return;
	}
	pMenu->pFont = pFont;
	__xgeXuiMenuInvalidate(pMenu);
	if ( pMenu->iUpdateLock == 0 ) {
		__xgeXuiMenuMeasure(pMenu);
	}
}

void xgeXuiMenuSetSelect(xge_xui_menu pMenu, xge_xui_menu_select_proc procSelect, void* pUser)
{
	if ( pMenu == NULL ) {
		return;
	}
	pMenu->procSelect = procSelect;
	pMenu->pUser = pUser;
}

void xgeXuiMenuSetMetrics(xge_xui_menu pMenu, const xge_xui_menu_metrics_t* pMetrics)
{
	if ( (pMenu == NULL) || (pMetrics == NULL) ) {
		return;
	}
	pMenu->tMetrics = *pMetrics;
	__xgeXuiMenuInvalidate(pMenu);
	if ( pMenu->iUpdateLock == 0 ) {
		__xgeXuiMenuMeasure(pMenu);
	}
}

void xgeXuiMenuSetColors(xge_xui_menu pMenu, const xge_xui_menu_colors_t* pColors)
{
	if ( (pMenu == NULL) || (pColors == NULL) ) {
		return;
	}
	pMenu->tColors = *pColors;
	xgeXuiPopupSetBackground(&pMenu->tPopup, pMenu->tColors.iPanel);
	xgeXuiPopupSetBorder(&pMenu->tPopup, pMenu->tColors.iBorder);
	xgeXuiWidgetMarkPaint(pMenu->pContentWidget);
}

void xgeXuiMenuOpenAt(xge_xui_menu pMenu, xge_xui_widget pOwner, float fX, float fY)
{
	if ( pMenu == NULL ) {
		return;
	}
	__xgeXuiMenuMeasure(pMenu);
	pMenu->pOwner = pOwner;
	xgeXuiPopupSetOwner(&pMenu->tPopup, pOwner);
	xgeXuiPopupSetFocusRestore(&pMenu->tPopup, pOwner);
	xgeXuiPopupSetAnchorRect(&pMenu->tPopup, (xge_rect_t){ fX, fY, 0.0f, 0.0f });
	xgeXuiPopupSetAnchorPoint(&pMenu->tPopup, XGE_XUI_POPUP_ANCHOR_FIXED);
	xgeXuiPopupSetDirection(&pMenu->tPopup, XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN);
	xgeXuiPopupSetGap(&pMenu->tPopup, 0.0f);
	xgeXuiPopupSetContentSize(&pMenu->tPopup, pMenu->fContentW, pMenu->fContentH);
	xgeXuiPopupSetScroll(&pMenu->tPopup, 0.0f, 0.0f);
	pMenu->iHover = __xgeXuiMenuNextEnabled(pMenu, -1, 1);
	xgeXuiPopupSetOpen(&pMenu->tPopup, 1);
	xgeXuiSetFocus(pMenu->pContext, pMenu->pContentWidget);
	xgeXuiWidgetMarkPaint(pMenu->pContentWidget);
}

void xgeXuiMenuOpenForOwner(xge_xui_menu pMenu, xge_xui_widget pOwner)
{
	if ( (pMenu == NULL) || (pOwner == NULL) ) {
		return;
	}
	__xgeXuiMenuMeasure(pMenu);
	pMenu->pOwner = pOwner;
	xgeXuiPopupSetOwner(&pMenu->tPopup, pOwner);
	xgeXuiPopupSetFocusRestore(&pMenu->tPopup, pOwner);
	pMenu->tPopup.tAnchorRect = (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	pMenu->tPopup.bAnchorRectSet = 0;
	xgeXuiPopupSetAnchorPoint(&pMenu->tPopup, XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT);
	xgeXuiPopupSetDirection(&pMenu->tPopup, XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN);
	xgeXuiPopupSetGap(&pMenu->tPopup, 0.0f);
	xgeXuiPopupSetContentSize(&pMenu->tPopup, pMenu->fContentW, pMenu->fContentH);
	xgeXuiPopupSetScroll(&pMenu->tPopup, 0.0f, 0.0f);
	pMenu->iHover = __xgeXuiMenuNextEnabled(pMenu, -1, 1);
	xgeXuiPopupSetOpen(&pMenu->tPopup, 1);
	xgeXuiSetFocus(pMenu->pContext, pMenu->pContentWidget);
	xgeXuiWidgetMarkPaint(pMenu->pContentWidget);
}

void xgeXuiMenuClose(xge_xui_menu pMenu)
{
	if ( pMenu == NULL ) {
		return;
	}
	__xgeXuiMenuCloseSubmenu(pMenu);
	xgeXuiPopupSetOpen(&pMenu->tPopup, 0);
	pMenu->iHover = -1;
	if ( pMenu->pContext != NULL && pMenu->pOwner != NULL && pMenu->pParentMenu == NULL ) {
		xgeXuiSetFocus(pMenu->pContext, pMenu->pOwner);
	}
}

int xgeXuiMenuIsOpen(xge_xui_menu pMenu)
{
	return (pMenu != NULL) ? xgeXuiPopupIsOpen(&pMenu->tPopup) : 0;
}
