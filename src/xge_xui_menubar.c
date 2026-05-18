static int __xgeXuiMenuBarItemEnabled(const xge_xui_menubar_item_t* pItem)
{
	return (pItem != NULL) && ((pItem->iState & XGE_XUI_MENUBAR_ITEM_ENABLED) != 0);
}

static int __xgeXuiMenuBarAsciiUpper(int iChar)
{
	return (iChar >= 'a' && iChar <= 'z') ? (iChar - 'a' + 'A') : iChar;
}

static int __xgeXuiMenuBarTextToDisplay(const char* sText, char* sBuffer, int iBufferSize, int* pMnemonic)
{
	int i;
	int j;
	int iMnemonic;

	if ( pMnemonic != NULL ) {
		*pMnemonic = 0;
	}
	if ( (sBuffer != NULL) && (iBufferSize > 0) ) {
		sBuffer[0] = 0;
	}
	if ( sText == NULL ) {
		return 0;
	}
	iMnemonic = 0;
	j = 0;
	for ( i = 0; sText[i] != 0; i++ ) {
		if ( sText[i] == '&' ) {
			if ( sText[i + 1] == '&' ) {
				if ( (sBuffer != NULL) && (j < iBufferSize - 1) ) {
					sBuffer[j] = '&';
				}
				j++;
				i++;
			} else if ( sText[i + 1] != 0 ) {
				iMnemonic = __xgeXuiMenuBarAsciiUpper((unsigned char)sText[i + 1]);
			}
			continue;
		}
		if ( (sBuffer != NULL) && (j < iBufferSize - 1) ) {
			sBuffer[j] = sText[i];
		}
		j++;
	}
	if ( (sBuffer != NULL) && (iBufferSize > 0) ) {
		sBuffer[(j < iBufferSize) ? j : (iBufferSize - 1)] = 0;
	}
	if ( pMnemonic != NULL ) {
		*pMnemonic = iMnemonic;
	}
	return j;
}

static void __xgeXuiMenuBarSyncOpen(xge_xui_menubar pMenuBar)
{
	xge_xui_menu pMenu;

	if ( (pMenuBar == NULL) || (pMenuBar->iOpen < 0) || (pMenuBar->iOpen >= pMenuBar->iItemCount) ) {
		return;
	}
	pMenu = pMenuBar->arrItems[pMenuBar->iOpen].pMenu;
	if ( (pMenu == NULL) || (xgeXuiMenuIsOpen(pMenu) == 0) ) {
		pMenuBar->iOpen = -1;
		pMenuBar->iActive = -1;
		xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
	}
}

static void __xgeXuiMenuBarCloseOpen(xge_xui_menubar pMenuBar)
{
	xge_xui_menu pMenu;

	if ( pMenuBar == NULL ) {
		return;
	}
	if ( (pMenuBar->iOpen >= 0) && (pMenuBar->iOpen < pMenuBar->iItemCount) ) {
		pMenu = pMenuBar->arrItems[pMenuBar->iOpen].pMenu;
		if ( pMenu != NULL ) {
			xgeXuiMenuClose(pMenu);
		}
	}
	pMenuBar->iOpen = -1;
	pMenuBar->iActive = -1;
	xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
}

static void __xgeXuiMenuBarLayout(xge_xui_menubar pMenuBar)
{
	xge_rect_t tContent;
	xge_vec2_t tSize;
	char sText[128];
	float fX;
	float fY;
	float fH;
	float fW;
	int i;

	if ( (pMenuBar == NULL) || (pMenuBar->pWidget == NULL) ) {
		return;
	}
	tContent = pMenuBar->pWidget->tContentRect;
	fX = tContent.fX + pMenuBar->tMetrics.fPaddingX;
	fY = tContent.fY + pMenuBar->tMetrics.fPaddingY;
	fH = tContent.fH - pMenuBar->tMetrics.fPaddingY * 2.0f;
	if ( fH < 1.0f ) {
		fH = 1.0f;
	}
	for ( i = 0; i < pMenuBar->iItemCount; i++ ) {
		__xgeXuiMenuBarTextToDisplay(pMenuBar->arrItems[i].sText, sText, sizeof(sText), NULL);
		tSize = __xgeXuiHostMeasureText(pMenuBar->pFont, sText);
		fW = tSize.fX + pMenuBar->tMetrics.fItemPaddingX * 2.0f;
		if ( fW < 20.0f ) {
			fW = 20.0f;
		}
		pMenuBar->arrItems[i].tRect = (xge_rect_t){ fX, fY, fW, fH };
		fX += fW + pMenuBar->tMetrics.fItemGap;
	}
}

static int __xgeXuiMenuBarIndexAt(xge_xui_menubar pMenuBar, float fX, float fY)
{
	int i;

	if ( pMenuBar == NULL ) {
		return -1;
	}
	__xgeXuiMenuBarLayout(pMenuBar);
	for ( i = 0; i < pMenuBar->iItemCount; i++ ) {
		if ( __xgeXuiMenuBarItemEnabled(&pMenuBar->arrItems[i]) && __xgeXuiRectContains(pMenuBar->arrItems[i].tRect, fX, fY) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeXuiMenuBarNextEnabled(xge_xui_menubar pMenuBar, int iCurrent, int iStep)
{
	int i;
	int iIndex;

	if ( (pMenuBar == NULL) || (pMenuBar->iItemCount <= 0) ) {
		return -1;
	}
	iIndex = iCurrent;
	for ( i = 0; i < pMenuBar->iItemCount; i++ ) {
		iIndex += iStep;
		if ( iIndex < 0 ) {
			iIndex = pMenuBar->iItemCount - 1;
		} else if ( iIndex >= pMenuBar->iItemCount ) {
			iIndex = 0;
		}
		if ( __xgeXuiMenuBarItemEnabled(&pMenuBar->arrItems[iIndex]) ) {
			return iIndex;
		}
	}
	return -1;
}

static int __xgeXuiMenuBarMnemonicIndex(xge_xui_menubar pMenuBar, int iKey)
{
	int iMnemonic;
	int i;

	if ( pMenuBar == NULL ) {
		return -1;
	}
	iKey = __xgeXuiMenuBarAsciiUpper(iKey);
	for ( i = 0; i < pMenuBar->iItemCount; i++ ) {
		iMnemonic = pMenuBar->arrItems[i].iMnemonic;
		if ( iMnemonic == 0 ) {
			__xgeXuiMenuBarTextToDisplay(pMenuBar->arrItems[i].sText, NULL, 0, &iMnemonic);
		}
		if ( __xgeXuiMenuBarItemEnabled(&pMenuBar->arrItems[i]) && (iMnemonic != 0) && (iMnemonic == iKey) ) {
			return i;
		}
	}
	return -1;
}

static void __xgeXuiMenuBarOpenItem(xge_xui_menubar pMenuBar, int iIndex)
{
	xge_xui_menu pMenu;
	xge_rect_t tRect;

	if ( (pMenuBar == NULL) || (iIndex < 0) || (iIndex >= pMenuBar->iItemCount) || !__xgeXuiMenuBarItemEnabled(&pMenuBar->arrItems[iIndex]) ) {
		return;
	}
	__xgeXuiMenuBarLayout(pMenuBar);
	if ( (pMenuBar->iOpen >= 0) && (pMenuBar->iOpen < pMenuBar->iItemCount) && (pMenuBar->iOpen != iIndex) ) {
		pMenu = pMenuBar->arrItems[pMenuBar->iOpen].pMenu;
		if ( pMenu != NULL ) {
			xgeXuiMenuClose(pMenu);
		}
	}
	pMenu = pMenuBar->arrItems[iIndex].pMenu;
	pMenuBar->iHover = iIndex;
	pMenuBar->iActive = iIndex;
	pMenuBar->iOpen = iIndex;
	if ( pMenu != NULL ) {
		if ( pMenuBar->procSelect != NULL ) {
			xgeXuiMenuSetSelect(pMenu, pMenuBar->procSelect, pMenuBar->pUser);
		}
		tRect = pMenuBar->arrItems[iIndex].tRect;
		xgeXuiMenuOpenAt(pMenu, pMenuBar->pWidget, tRect.fX, pMenuBar->pWidget->tRect.fY + pMenuBar->pWidget->tRect.fH);
	}
	xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
}

int xgeXuiMenuBarInit(xge_xui_menubar pMenuBar, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;
	const xge_xui_chrome_style_t* pChrome;

	if ( (pMenuBar == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pMenuBar, 0, sizeof(*pMenuBar));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pChrome = xgeXuiGetChromeStyle(pContext);
	pMenuBar->pContext = pContext;
	pMenuBar->pWidget = pWidget;
	pMenuBar->pFont = pTheme->pFont;
	pMenuBar->iHover = -1;
	pMenuBar->iActive = -1;
	pMenuBar->iOpen = -1;
	pMenuBar->tMetrics = pChrome->tMenuBarMetrics;
	pMenuBar->tColors = pChrome->tBarColors;
	xgeXuiWidgetSetBackground(pWidget, pMenuBar->tColors.iBackground);
	xgeXuiWidgetSetBorder(pWidget, 1.0f, pMenuBar->tColors.iBorder);
	pWidget->tStyle.fRadius = 0.0f;
	xgeXuiWidgetSetEvent(pWidget, xgeXuiMenuBarEventProc, NULL);
	pWidget->procPaint = xgeXuiMenuBarPaintProc;
	pWidget->pUser = pMenuBar;
	return XGE_OK;
}

void xgeXuiMenuBarUnit(xge_xui_menubar pMenuBar)
{
	if ( pMenuBar == NULL ) {
		return;
	}
	if ( pMenuBar->pWidget != NULL && pMenuBar->pWidget->pUser == pMenuBar ) {
		pMenuBar->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pMenuBar->pWidget, NULL, NULL);
		pMenuBar->pWidget->procPaint = NULL;
	}
	memset(pMenuBar, 0, sizeof(*pMenuBar));
}

void xgeXuiMenuBarSetItems(xge_xui_menubar pMenuBar, const xge_xui_menubar_item_t* arrItems, int iCount)
{
	int i;

	if ( pMenuBar == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	if ( iCount > XGE_XUI_MENUBAR_CAPACITY ) {
		iCount = XGE_XUI_MENUBAR_CAPACITY;
	}
	memset(pMenuBar->arrItems, 0, sizeof(pMenuBar->arrItems));
	for ( i = 0; i < iCount; i++ ) {
		pMenuBar->arrItems[i] = arrItems[i];
		if ( pMenuBar->arrItems[i].sText == NULL ) {
			pMenuBar->arrItems[i].sText = "";
		}
		if ( pMenuBar->arrItems[i].iState == 0 ) {
			pMenuBar->arrItems[i].iState = XGE_XUI_MENUBAR_ITEM_ENABLED;
		}
		if ( pMenuBar->arrItems[i].iMnemonic != 0 ) {
			pMenuBar->arrItems[i].iMnemonic = __xgeXuiMenuBarAsciiUpper(pMenuBar->arrItems[i].iMnemonic);
		}
	}
	pMenuBar->iItemCount = iCount;
	pMenuBar->iHover = -1;
	pMenuBar->iActive = -1;
	pMenuBar->iOpen = -1;
	xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
}

int xgeXuiMenuBarAddItem(xge_xui_menubar pMenuBar, const char* sText, xge_xui_menu pMenu, int iValue)
{
	xge_xui_menubar_item_t* pItem;
	int iIndex;

	if ( pMenuBar == NULL || pMenuBar->iItemCount >= XGE_XUI_MENUBAR_CAPACITY ) {
		return -1;
	}
	iIndex = pMenuBar->iItemCount++;
	pItem = &pMenuBar->arrItems[iIndex];
	memset(pItem, 0, sizeof(*pItem));
	pItem->sText = (sText != NULL) ? sText : "";
	pItem->iState = XGE_XUI_MENUBAR_ITEM_ENABLED;
	pItem->iValue = iValue;
	pItem->pMenu = pMenu;
	__xgeXuiMenuBarTextToDisplay(pItem->sText, NULL, 0, &pItem->iMnemonic);
	xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
	return iIndex;
}

void xgeXuiMenuBarSetItemMenu(xge_xui_menubar pMenuBar, int iIndex, xge_xui_menu pMenu)
{
	if ( (pMenuBar == NULL) || (iIndex < 0) || (iIndex >= pMenuBar->iItemCount) ) {
		return;
	}
	pMenuBar->arrItems[iIndex].pMenu = pMenu;
	xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
}

void xgeXuiMenuBarSetItemEnabled(xge_xui_menubar pMenuBar, int iIndex, int bEnabled)
{
	if ( (pMenuBar == NULL) || (iIndex < 0) || (iIndex >= pMenuBar->iItemCount) ) {
		return;
	}
	if ( bEnabled ) {
		pMenuBar->arrItems[iIndex].iState |= XGE_XUI_MENUBAR_ITEM_ENABLED;
	} else {
		pMenuBar->arrItems[iIndex].iState &= ~XGE_XUI_MENUBAR_ITEM_ENABLED;
		if ( pMenuBar->iHover == iIndex ) {
			pMenuBar->iHover = -1;
		}
		if ( pMenuBar->iOpen == iIndex ) {
			__xgeXuiMenuBarCloseOpen(pMenuBar);
		}
	}
	xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
}

void xgeXuiMenuBarSetFont(xge_xui_menubar pMenuBar, xge_font pFont)
{
	if ( pMenuBar == NULL ) {
		return;
	}
	pMenuBar->pFont = pFont;
	xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
}

void xgeXuiMenuBarSetSelect(xge_xui_menubar pMenuBar, xge_xui_menu_select_proc procSelect, void* pUser)
{
	if ( pMenuBar == NULL ) {
		return;
	}
	pMenuBar->procSelect = procSelect;
	pMenuBar->pUser = pUser;
}

void xgeXuiMenuBarSetMetrics(xge_xui_menubar pMenuBar, const xge_xui_bar_metrics_t* pMetrics)
{
	if ( (pMenuBar == NULL) || (pMetrics == NULL) ) {
		return;
	}
	pMenuBar->tMetrics = *pMetrics;
	xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
}

void xgeXuiMenuBarSetColors(xge_xui_menubar pMenuBar, const xge_xui_bar_colors_t* pColors)
{
	if ( (pMenuBar == NULL) || (pColors == NULL) ) {
		return;
	}
	pMenuBar->tColors = *pColors;
	xgeXuiWidgetSetBackground(pMenuBar->pWidget, pColors->iBackground);
	xgeXuiWidgetSetBorder(pMenuBar->pWidget, 1.0f, pColors->iBorder);
	xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
}

int xgeXuiMenuBarEvent(xge_xui_menubar pMenuBar, const xge_event_t* pEvent)
{
	int iIndex;
	int iNext;

	if ( (pMenuBar == NULL) || (pMenuBar->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pMenuBar->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pMenuBar->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	__xgeXuiMenuBarSyncOpen(pMenuBar);
	iIndex = __xgeXuiMenuBarIndexAt(pMenuBar, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			pMenuBar->iHover = iIndex;
			if ( (iIndex >= 0) && (pMenuBar->iOpen >= 0) && (iIndex != pMenuBar->iOpen) ) {
				__xgeXuiMenuBarOpenItem(pMenuBar, iIndex);
			}
			xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
			return (pMenuBar->iOpen >= 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			pMenuBar->iHover = -1;
			xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iIndex < 0 ) {
				if ( pMenuBar->iOpen >= 0 ) {
					pMenuBar->iHover = -1;
					__xgeXuiMenuBarCloseOpen(pMenuBar);
					return XGE_XUI_EVENT_CONSUMED;
				}
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pMenuBar->pContext, pMenuBar->pWidget);
			pMenuBar->iHover = iIndex;
			if ( (pMenuBar->iOpen == iIndex) && (pMenuBar->arrItems[iIndex].pMenu != NULL) && xgeXuiMenuIsOpen(pMenuBar->arrItems[iIndex].pMenu) ) {
				__xgeXuiMenuBarCloseOpen(pMenuBar);
				return XGE_XUI_EVENT_CONSUMED;
			}
			pMenuBar->iActive = iIndex;
			__xgeXuiMenuBarOpenItem(pMenuBar, iIndex);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			return (pMenuBar->iActive >= 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_KEY_DOWN:
			if ( (pEvent->iParam2 & XGE_KEY_MOD_ALT) != 0 ) {
				iIndex = __xgeXuiMenuBarMnemonicIndex(pMenuBar, pEvent->iParam1);
				if ( iIndex >= 0 ) {
					xgeXuiSetFocus(pMenuBar->pContext, pMenuBar->pWidget);
					__xgeXuiMenuBarOpenItem(pMenuBar, iIndex);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			if ( (pMenuBar->pContext == NULL) || (pMenuBar->pContext->pFocus != pMenuBar->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( pEvent->iParam1 == XGE_KEY_F10 || pEvent->iParam1 == XGE_KEY_MENU ) {
				iIndex = __xgeXuiMenuBarNextEnabled(pMenuBar, -1, 1);
				if ( iIndex >= 0 ) {
					pMenuBar->iHover = iIndex;
					xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			if ( pEvent->iParam1 == XGE_KEY_LEFT || pEvent->iParam1 == XGE_KEY_RIGHT ) {
				iNext = __xgeXuiMenuBarNextEnabled(pMenuBar, (pMenuBar->iHover >= 0) ? pMenuBar->iHover : pMenuBar->iOpen, (pEvent->iParam1 == XGE_KEY_RIGHT) ? 1 : -1);
				if ( iNext >= 0 ) {
					pMenuBar->iHover = iNext;
					if ( pMenuBar->iOpen >= 0 ) {
						__xgeXuiMenuBarOpenItem(pMenuBar, iNext);
					}
					xgeXuiWidgetMarkPaint(pMenuBar->pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			if ( pEvent->iParam1 == XGE_KEY_DOWN || pEvent->iParam1 == XGE_KEY_ENTER || pEvent->iParam1 == XGE_KEY_SPACE ) {
				iIndex = (pMenuBar->iHover >= 0) ? pMenuBar->iHover : __xgeXuiMenuBarNextEnabled(pMenuBar, -1, 1);
				if ( iIndex >= 0 ) {
					__xgeXuiMenuBarOpenItem(pMenuBar, iIndex);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			if ( pEvent->iParam1 == XGE_KEY_ESCAPE ) {
				pMenuBar->iHover = -1;
				__xgeXuiMenuBarCloseOpen(pMenuBar);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiMenuBarEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiMenuBarEvent((xge_xui_menubar)pUser, pEvent);
}

void xgeXuiMenuBarPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_menubar pMenuBar;
	xge_xui_menubar_item_t* pItem;
	xge_rect_t tRect;
	xge_rect_t tText;
	uint32_t iBack;
	uint32_t iText;
	char sText[128];
	int i;
	int bHot;

	pMenuBar = (xge_xui_menubar)pUser;
	if ( (pWidget == NULL) || (pMenuBar == NULL) ) {
		return;
	}
	__xgeXuiMenuBarSyncOpen(pMenuBar);
	__xgeXuiMenuBarLayout(pMenuBar);
	for ( i = 0; i < pMenuBar->iItemCount; i++ ) {
		pItem = &pMenuBar->arrItems[i];
		tRect = pItem->tRect;
		if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
			continue;
		}
		bHot = (i == pMenuBar->iHover) || (i == pMenuBar->iOpen) || (i == pMenuBar->iActive);
		iBack = pMenuBar->tColors.iItem;
		iText = pMenuBar->tColors.iText;
		if ( !__xgeXuiMenuBarItemEnabled(pItem) ) {
			iBack = XGE_COLOR_RGBA(255, 255, 255, 0);
			iText = pMenuBar->tColors.iDisabledText;
		} else if ( i == pMenuBar->iOpen || i == pMenuBar->iActive ) {
			iBack = pMenuBar->tColors.iActive;
		} else if ( bHot ) {
			iBack = pMenuBar->tColors.iHover;
		}
		if ( XGE_COLOR_GET_A(iBack) != 0 ) {
			__xgeXuiHostDrawRect(tRect, iBack);
		}
		tText = tRect;
		tText.fX += pMenuBar->tMetrics.fItemPaddingX;
		tText.fW -= pMenuBar->tMetrics.fItemPaddingX * 2.0f;
		__xgeXuiMenuBarTextToDisplay(pItem->sText, sText, sizeof(sText), NULL);
		if ( tText.fW > 0.0f ) {
			__xgeXuiHostDrawTextRect(pMenuBar->pFont, sText, tText, iText, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
}
