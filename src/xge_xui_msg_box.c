static char* __xgeXuiBoxCopyText(const char* sText)
{
	size_t iSize;
	char* sCopy;

	if ( sText == NULL ) {
		sText = "";
	}
	iSize = strlen(sText);
	sCopy = (char*)xrtMalloc(iSize + 1);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iSize + 1);
	return sCopy;
}

static int __xgeXuiBoxUtf8Next(const char* sText, int iLen, int iPos)
{
	unsigned char c;

	if ( (sText == NULL) || (iPos >= iLen) ) {
		return iLen;
	}
	c = (unsigned char)sText[iPos];
	if ( c < 0x80 ) {
		return iPos + 1;
	}
	if ( (c & 0xE0) == 0xC0 && iPos + 1 < iLen ) {
		return iPos + 2;
	}
	if ( (c & 0xF0) == 0xE0 && iPos + 2 < iLen ) {
		return iPos + 3;
	}
	if ( (c & 0xF8) == 0xF0 && iPos + 3 < iLen ) {
		return iPos + 4;
	}
	return iPos + 1;
}

static xge_vec2_t __xgeXuiBoxMeasureSpan(xge_font pFont, const char* sText, int iStart, int iEnd)
{
	char arrBuffer[512];
	char* sBuffer;
	xge_vec2_t tSize;
	int iLen;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( (pFont == NULL) || (sText == NULL) || (iEnd <= iStart) ) {
		return tSize;
	}
	iLen = iEnd - iStart;
	sBuffer = arrBuffer;
	if ( iLen >= (int)sizeof(arrBuffer) ) {
		sBuffer = (char*)xrtMalloc((size_t)iLen + 1);
		if ( sBuffer == NULL ) {
			return tSize;
		}
	}
	memcpy(sBuffer, sText + iStart, (size_t)iLen);
	sBuffer[iLen] = 0;
	tSize = __xgeXuiHostMeasureText(pFont, sBuffer);
	if ( sBuffer != arrBuffer ) {
		xrtFree(sBuffer);
	}
	return tSize;
}

static float __xgeXuiBoxLineHeight(xge_font pFont)
{
	xge_vec2_t tSize;

	tSize = __xgeXuiHostMeasureText(pFont, "Wg");
	if ( tSize.fY <= 0.0f ) {
		return 18.0f;
	}
	return tSize.fY + 4.0f;
}

static int __xgeXuiBoxWrapNextLine(xge_font pFont, const char* sText, int iLen, int iStart, float fMaxWidth, int* pLineEnd, int* pNextStart)
{
	int iPos;
	int iNext;
	int iFitEnd;
	int iBreakEnd;
	int iBreakNext;
	int iDrawEnd;
	xge_vec2_t tSize;

	if ( pLineEnd != NULL ) {
		*pLineEnd = iStart;
	}
	if ( pNextStart != NULL ) {
		*pNextStart = iStart;
	}
	if ( (sText == NULL) || (iStart >= iLen) ) {
		return 0;
	}
	iPos = iStart;
	iFitEnd = iStart;
	iBreakEnd = -1;
	iBreakNext = -1;
	while ( iPos < iLen ) {
		if ( sText[iPos] == '\r' || sText[iPos] == '\n' ) {
			iFitEnd = iPos;
			iNext = iPos + 1;
			if ( sText[iPos] == '\r' && iNext < iLen && sText[iNext] == '\n' ) {
				iNext++;
			}
			if ( pLineEnd != NULL ) {
				*pLineEnd = iFitEnd;
			}
			if ( pNextStart != NULL ) {
				*pNextStart = iNext;
			}
			return 1;
		}
		iNext = __xgeXuiBoxUtf8Next(sText, iLen, iPos);
		if ( sText[iPos] == ' ' || sText[iPos] == '\t' ) {
			iBreakEnd = iPos;
			iBreakNext = iNext;
		}
		tSize = __xgeXuiBoxMeasureSpan(pFont, sText, iStart, iNext);
		if ( (fMaxWidth > 0.0f) && (tSize.fX > fMaxWidth) && (iFitEnd > iStart) ) {
			if ( iBreakEnd > iStart ) {
				iFitEnd = iBreakEnd;
				iNext = iBreakNext;
			} else {
				iNext = iFitEnd;
			}
			break;
		}
		iFitEnd = iNext;
		iPos = iNext;
	}
	if ( iFitEnd <= iStart ) {
		iFitEnd = __xgeXuiBoxUtf8Next(sText, iLen, iStart);
	}
	iDrawEnd = iFitEnd;
	while ( iDrawEnd > iStart && (sText[iDrawEnd - 1] == ' ' || sText[iDrawEnd - 1] == '\t') ) {
		iDrawEnd--;
	}
	if ( pLineEnd != NULL ) {
		*pLineEnd = iDrawEnd;
	}
	if ( pNextStart != NULL ) {
		*pNextStart = (iNext > iStart) ? iNext : iFitEnd;
	}
	return 1;
}

static int __xgeXuiBoxWrapMeasure(xge_font pFont, const char* sText, float fMaxWidth, float* pMaxLineWidth)
{
	int iLen;
	int iStart;
	int iLineEnd;
	int iNextStart;
	int iCount;
	xge_vec2_t tSize;
	float fMaxLineWidth;

	if ( sText == NULL ) {
		sText = "";
	}
	iLen = (int)strlen(sText);
	if ( iLen <= 0 ) {
		if ( pMaxLineWidth != NULL ) {
			*pMaxLineWidth = 0.0f;
		}
		return 1;
	}
	iStart = 0;
	iCount = 0;
	fMaxLineWidth = 0.0f;
	while ( iStart < iLen ) {
		if ( !__xgeXuiBoxWrapNextLine(pFont, sText, iLen, iStart, fMaxWidth, &iLineEnd, &iNextStart) ) {
			break;
		}
		tSize = __xgeXuiBoxMeasureSpan(pFont, sText, iStart, iLineEnd);
		if ( tSize.fX > fMaxLineWidth ) {
			fMaxLineWidth = tSize.fX;
		}
		iCount++;
		if ( iNextStart <= iStart ) {
			break;
		}
		iStart = iNextStart;
	}
	if ( iCount <= 0 ) {
		iCount = 1;
	}
	if ( pMaxLineWidth != NULL ) {
		*pMaxLineWidth = fMaxLineWidth;
	}
	return iCount;
}

static void __xgeXuiBoxDrawWrappedText(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor)
{
	char arrBuffer[512];
	char* sBuffer;
	int iLen;
	int iStart;
	int iLineEnd;
	int iNextStart;
	int iSpanLen;
	float fLineH;
	xge_rect_t tLine;

	if ( (pFont == NULL) || (sText == NULL) || (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	iLen = (int)strlen(sText);
	fLineH = __xgeXuiBoxLineHeight(pFont);
	tLine = tRect;
	tLine.fH = fLineH;
	iStart = 0;
	if ( iLen <= 0 ) {
		return;
	}
	while ( iStart < iLen && tLine.fY < tRect.fY + tRect.fH ) {
		if ( !__xgeXuiBoxWrapNextLine(pFont, sText, iLen, iStart, tRect.fW, &iLineEnd, &iNextStart) ) {
			break;
		}
		iSpanLen = iLineEnd - iStart;
		if ( iSpanLen > 0 ) {
			sBuffer = arrBuffer;
			if ( iSpanLen >= (int)sizeof(arrBuffer) ) {
				sBuffer = (char*)xrtMalloc((size_t)iSpanLen + 1);
				if ( sBuffer == NULL ) {
					return;
				}
			}
			memcpy(sBuffer, sText + iStart, (size_t)iSpanLen);
			sBuffer[iSpanLen] = 0;
			__xgeXuiHostDrawTextRect(pFont, sBuffer, tLine, iColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			if ( sBuffer != arrBuffer ) {
				xrtFree(sBuffer);
			}
		}
		tLine.fY += fLineH;
		if ( iNextStart <= iStart ) {
			break;
		}
		iStart = iNextStart;
	}
}

static int __xgeXuiBoxPointEvent(int iType)
{
	return (iType == XGE_EVENT_MOUSE_DOWN) || (iType == XGE_EVENT_MOUSE_UP) || (iType == XGE_EVENT_MOUSE_MOVE) ||
		(iType == XGE_EVENT_TOUCH_BEGIN) || (iType == XGE_EVENT_TOUCH_END) || (iType == XGE_EVENT_TOUCH_MOVE);
}

static int __xgeXuiMsgBoxButtonSemanticFromText(const char* sText, int iDefault)
{
	if ( sText == NULL ) {
		return iDefault;
	}
	if ( strcmp(sText, "primary") == 0 ) {
		return XGE_XUI_BUTTON_SEMANTIC_PRIMARY;
	}
	if ( strcmp(sText, "danger") == 0 ) {
		return XGE_XUI_BUTTON_SEMANTIC_DANGER;
	}
	if ( strcmp(sText, "default") == 0 ) {
		return XGE_XUI_BUTTON_SEMANTIC_DEFAULT;
	}
	return iDefault;
}

static void __xgeXuiMsgBoxSyncButtons(xge_xui_msg_box pBox)
{
	int i;

	if ( pBox == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_MSG_BOX_BUTTON_CAPACITY; i++ ) {
		if ( pBox->arrButtonWidget[i] == NULL ) {
			continue;
		}
		xgeXuiWidgetSetVisible(pBox->arrButtonWidget[i], i < pBox->iButtonCount);
		xgeXuiWidgetSetEnabled(pBox->arrButtonWidget[i], i < pBox->iButtonCount);
		if ( i < pBox->iButtonCount ) {
			xgeXuiButtonSetText(&pBox->arrButton[i], pBox->pFont, pBox->arrButtonText[i]);
			xgeXuiButtonSetColors(&pBox->arrButton[i], pBox->iButtonColor, pBox->iButtonHoverColor, pBox->iButtonActiveColor, pBox->iButtonHoverColor, XGE_COLOR_RGBA(208, 218, 228, 255));
			xgeXuiButtonSetTextColor(&pBox->arrButton[i], pBox->iButtonTextColor);
			if ( pBox->arrButtonSemantic[i] != XGE_XUI_BUTTON_SEMANTIC_DEFAULT ) {
				xgeXuiButtonSetSemantic(&pBox->arrButton[i], pBox->arrButtonSemantic[i]);
			}
		}
	}
}

static void __xgeXuiMsgBoxSetButton(xge_xui_msg_box pBox, int iIndex, const char* sText, int iResult, int iSemantic)
{
	if ( (pBox == NULL) || (iIndex < 0) || (iIndex >= XGE_XUI_MSG_BOX_BUTTON_CAPACITY) ) {
		return;
	}
	if ( sText == NULL ) {
		sText = "";
	}
	snprintf(pBox->arrButtonTextOwned[iIndex], sizeof(pBox->arrButtonTextOwned[iIndex]), "%s", sText);
	pBox->arrButtonTextOwned[iIndex][sizeof(pBox->arrButtonTextOwned[iIndex]) - 1] = 0;
	pBox->arrButtonText[iIndex] = pBox->arrButtonTextOwned[iIndex];
	pBox->arrButtonResult[iIndex] = iResult;
	pBox->arrButtonSemantic[iIndex] = iSemantic;
}

static void __xgeXuiMsgBoxUpdateButtons(xge_xui_msg_box pBox)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->iButtonCount = 1;
	__xgeXuiMsgBoxSetButton(pBox, 0, "OK", XGE_XUI_MSG_BOX_RESULT_OK, XGE_XUI_BUTTON_SEMANTIC_PRIMARY);
	if ( pBox->iButtons == XGE_XUI_MSG_BOX_BUTTON_OK_CANCEL ) {
		pBox->iButtonCount = 2;
		__xgeXuiMsgBoxSetButton(pBox, 0, "OK", XGE_XUI_MSG_BOX_RESULT_OK, XGE_XUI_BUTTON_SEMANTIC_PRIMARY);
		__xgeXuiMsgBoxSetButton(pBox, 1, "Cancel", XGE_XUI_MSG_BOX_RESULT_CANCEL, XGE_XUI_BUTTON_SEMANTIC_DEFAULT);
	} else if ( pBox->iButtons == XGE_XUI_MSG_BOX_BUTTON_YES_NO ) {
		pBox->iButtonCount = 2;
		__xgeXuiMsgBoxSetButton(pBox, 0, "Yes", XGE_XUI_MSG_BOX_RESULT_YES, XGE_XUI_BUTTON_SEMANTIC_PRIMARY);
		__xgeXuiMsgBoxSetButton(pBox, 1, "No", XGE_XUI_MSG_BOX_RESULT_NO, XGE_XUI_BUTTON_SEMANTIC_DEFAULT);
	} else if ( pBox->iButtons == XGE_XUI_MSG_BOX_BUTTON_YES_NO_CANCEL ) {
		pBox->iButtonCount = 3;
		__xgeXuiMsgBoxSetButton(pBox, 0, "Yes", XGE_XUI_MSG_BOX_RESULT_YES, XGE_XUI_BUTTON_SEMANTIC_PRIMARY);
		__xgeXuiMsgBoxSetButton(pBox, 1, "No", XGE_XUI_MSG_BOX_RESULT_NO, XGE_XUI_BUTTON_SEMANTIC_DEFAULT);
		__xgeXuiMsgBoxSetButton(pBox, 2, "Cancel", XGE_XUI_MSG_BOX_RESULT_CANCEL, XGE_XUI_BUTTON_SEMANTIC_DEFAULT);
	}
	__xgeXuiMsgBoxSyncButtons(pBox);
	pBox->iLayoutDirty = 1;
}

static void __xgeXuiMsgBoxApplyTypeColor(xge_xui_msg_box pBox)
{
	if ( pBox == NULL ) {
		return;
	}
	if ( pBox->iType == XGE_XUI_MSG_BOX_ICON_WAR ) {
		pBox->iIconColor = XGE_COLOR_RGBA(219, 141, 36, 255);
	} else if ( pBox->iType == XGE_XUI_MSG_BOX_ICON_ERROR ) {
		pBox->iIconColor = XGE_COLOR_RGBA(210, 70, 84, 255);
	} else if ( pBox->iType == XGE_XUI_MSG_BOX_ICON_QUEST ) {
		pBox->iIconColor = XGE_COLOR_RGBA(59, 126, 204, 255);
	} else {
		pBox->iIconColor = XGE_COLOR_RGBA(35, 132, 214, 255);
	}
}

static void __xgeXuiMsgBoxCloseResult(xge_xui_msg_box pBox, int iResult)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->iResult = iResult;
	if ( xgeXuiMsgBoxIsOpen(pBox) ) {
		xgeXuiWindowSetOpen(&pBox->tWindow, 0);
	}
	if ( pBox->procResult != NULL ) {
		pBox->procResult(pBox->pWidget, iResult, pBox->pUser);
	}
}

static void __xgeXuiMsgBoxWindowClose(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	__xgeXuiMsgBoxCloseResult((xge_xui_msg_box)pUser, XGE_XUI_MSG_BOX_RESULT_CLOSE);
}

static void __xgeXuiMsgBoxButtonClick(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_msg_box pBox;
	int i;

	pBox = (xge_xui_msg_box)pUser;
	if ( (pBox == NULL) || (pWidget == NULL) ) {
		return;
	}
	for ( i = 0; i < pBox->iButtonCount; i++ ) {
		if ( pWidget == pBox->arrButtonWidget[i] ) {
			__xgeXuiMsgBoxCloseResult(pBox, pBox->arrButtonResult[i]);
			return;
		}
	}
}

static void __xgeXuiMsgBoxPaintBefore(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_msg_box pBox;

	(void)pWidget;
	pBox = (xge_xui_msg_box)pUser;
	if ( (pBox == NULL) || (pBox->bModal == 0) || !xgeXuiMsgBoxIsOpen(pBox) ) {
		return;
	}
	__xgeXuiHostDrawRect(pBox->tBackdropRect, pBox->iBackdropColor);
}

static xge_rect_t __xgeXuiBoxCurrentClientRect(xge_xui_window pWindow, xge_xui_widget pWidget)
{
	xge_xui_widget pClient;
	xge_rect_t tClient;

	memset(&tClient, 0, sizeof(tClient));
	if ( pWindow != NULL ) {
		pClient = xgeXuiWindowGetClientWidget(pWindow);
		if ( (pClient != NULL) && (pClient->tRect.fW > 0.0f) && (pClient->tRect.fH > 0.0f) ) {
			return pClient->tRect;
		}
	}
	if ( pWidget != NULL ) {
		tClient = pWidget->tRect;
		if ( pWindow != NULL ) {
			tClient.fX += pWindow->fBorderWidth;
			tClient.fY += pWindow->fBorderWidth + pWindow->fTitleBarHeight;
			tClient.fW -= pWindow->fBorderWidth * 2.0f;
			tClient.fH -= pWindow->fBorderWidth * 2.0f + pWindow->fTitleBarHeight;
			if ( tClient.fW < 0.0f ) {
				tClient.fW = 0.0f;
			}
			if ( tClient.fH < 0.0f ) {
				tClient.fH = 0.0f;
			}
		}
	}
	return tClient;
}

static void __xgeXuiMsgBoxSyncVisualRects(xge_xui_msg_box pBox)
{
	xge_rect_t tClient;
	float fIconBlock;
	float fButtonW;
	float fButtonH;
	float fButtonGap;
	float fTotalButtonW;
	float fButtonX;
	int i;

	if ( pBox == NULL ) {
		return;
	}
	tClient = __xgeXuiBoxCurrentClientRect(&pBox->tWindow, pBox->pWidget);
	if ( (tClient.fW <= 0.0f) || (tClient.fH <= 0.0f) ) {
		return;
	}
	fIconBlock = (pBox->bShowIcon != 0) ? 52.0f : 0.0f;
	fButtonW = 76.0f;
	fButtonH = 28.0f;
	fButtonGap = 8.0f;
	fTotalButtonW = (float)pBox->iButtonCount * fButtonW + (float)((pBox->iButtonCount > 0) ? (pBox->iButtonCount - 1) : 0) * fButtonGap;
	pBox->tIconRect = (xge_rect_t){ tClient.fX + 18.0f, tClient.fY + 22.0f, 32.0f, 32.0f };
	pBox->tMessageRect = (xge_rect_t){ tClient.fX + 18.0f + fIconBlock, tClient.fY + 18.0f, tClient.fW - 36.0f - fIconBlock, tClient.fH - fButtonH - 48.0f };
	if ( pBox->tMessageRect.fH < 20.0f ) {
		pBox->tMessageRect.fH = 20.0f;
	}
	fButtonX = tClient.fX + tClient.fW - fTotalButtonW - 16.0f;
	for ( i = 0; i < XGE_XUI_MSG_BOX_BUTTON_CAPACITY; i++ ) {
		pBox->arrButtonRect[i] = (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
		if ( i < pBox->iButtonCount ) {
			pBox->arrButtonRect[i] = (xge_rect_t){ fButtonX + (float)i * (fButtonW + fButtonGap), tClient.fY + tClient.fH - fButtonH - 14.0f, fButtonW, fButtonH };
		}
	}
}

static void __xgeXuiMsgBoxLayout(xge_xui_msg_box pBox)
{
	xge_rect_t tRoot;
	xge_rect_t tWin;
	xge_rect_t tClient;
	xge_vec2_t tFullText;
	float fRootW;
	float fRootH;
	float fMaxWinW;
	float fIconBlock;
	float fMessageW;
	float fMessageMaxLineW;
	float fLineH;
	float fMessageH;
	float fButtonW;
	float fButtonH;
	float fButtonGap;
	float fTotalButtonW;
	float fButtonX;
	int iLines;
	int i;

	if ( (pBox == NULL) || (pBox->pContext == NULL) || (pBox->pWidget == NULL) ) {
		return;
	}
	tRoot = (pBox->pContext->pRoot != NULL) ? pBox->pContext->pRoot->tRect : pBox->pWidget->tRect;
	fRootW = (tRoot.fW > 0.0f) ? tRoot.fW : 800.0f;
	fRootH = (tRoot.fH > 0.0f) ? tRoot.fH : 600.0f;
	fMaxWinW = fRootW * 0.8f;
	if ( fMaxWinW < 320.0f ) {
		fMaxWinW = 320.0f;
	}
	if ( fMaxWinW > 680.0f ) {
		fMaxWinW = 680.0f;
	}
	fIconBlock = (pBox->bShowIcon != 0) ? 52.0f : 0.0f;
	fLineH = __xgeXuiBoxLineHeight(pBox->pFont);
	tFullText = __xgeXuiHostMeasureText(pBox->pFont, pBox->sMessage);
	fButtonW = 76.0f;
	fButtonH = 28.0f;
	fButtonGap = 8.0f;
	fTotalButtonW = (float)pBox->iButtonCount * fButtonW + (float)((pBox->iButtonCount > 0) ? (pBox->iButtonCount - 1) : 0) * fButtonGap;
	tWin.fW = tFullText.fX + fIconBlock + 52.0f;
	if ( tWin.fW < fTotalButtonW + 48.0f ) {
		tWin.fW = fTotalButtonW + 48.0f;
	}
	if ( tWin.fW < 340.0f ) {
		tWin.fW = 340.0f;
	}
	if ( tWin.fW > fMaxWinW ) {
		tWin.fW = fMaxWinW;
	}
	fMessageW = tWin.fW - fIconBlock - 44.0f;
	if ( fMessageW < 120.0f ) {
		fMessageW = 120.0f;
	}
	iLines = __xgeXuiBoxWrapMeasure(pBox->pFont, pBox->sMessage, fMessageW, &fMessageMaxLineW);
	(void)fMessageMaxLineW;
	fMessageH = (float)iLines * fLineH;
	if ( fMessageH < 36.0f ) {
		fMessageH = 36.0f;
	}
	tWin.fH = pBox->tWindow.fTitleBarHeight + 22.0f + fMessageH + 18.0f + fButtonH + 16.0f + pBox->tWindow.fBorderWidth * 2.0f;
	if ( tWin.fH < 150.0f ) {
		tWin.fH = 150.0f;
	}
	if ( tWin.fH > fRootH * 0.85f ) {
		tWin.fH = fRootH * 0.85f;
	}
	tWin.fX = tRoot.fX + (fRootW - tWin.fW) * 0.5f;
	tWin.fY = tRoot.fY + (fRootH - tWin.fH) * 0.42f;
	xgeXuiWidgetSetSize(pBox->pWidget, xgeXuiSizePx(tWin.fW), xgeXuiSizePx(tWin.fH));
	xgeXuiWidgetSetRect(pBox->pWidget, tWin);
	__xgeXuiWindowLayout(pBox->pWidget, &pBox->tWindow);
	tClient = xgeXuiWindowGetClientWidget(&pBox->tWindow)->tRect;
	if ( tClient.fW <= 0.0f || tClient.fH <= 0.0f ) {
		tClient = pBox->pWidget->tRect;
		tClient.fX += pBox->tWindow.fBorderWidth;
		tClient.fY += pBox->tWindow.fBorderWidth + pBox->tWindow.fTitleBarHeight;
		tClient.fW -= pBox->tWindow.fBorderWidth * 2.0f;
		tClient.fH -= pBox->tWindow.fBorderWidth * 2.0f + pBox->tWindow.fTitleBarHeight;
	}
	xgeXuiWidgetSetRect(pBox->pContentWidget, (xge_rect_t){ 0.0f, 0.0f, tClient.fW, tClient.fH });
	pBox->tIconRect = (xge_rect_t){ tClient.fX + 18.0f, tClient.fY + 22.0f, 32.0f, 32.0f };
	pBox->tMessageRect = (xge_rect_t){ tClient.fX + 18.0f + fIconBlock, tClient.fY + 18.0f, tClient.fW - 36.0f - fIconBlock, tClient.fH - fButtonH - 48.0f };
	if ( pBox->tMessageRect.fH < 20.0f ) {
		pBox->tMessageRect.fH = 20.0f;
	}
	fButtonX = tClient.fX + tClient.fW - fTotalButtonW - 16.0f;
	for ( i = 0; i < XGE_XUI_MSG_BOX_BUTTON_CAPACITY; i++ ) {
		pBox->arrButtonRect[i] = (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
		if ( i < pBox->iButtonCount ) {
			pBox->arrButtonRect[i] = (xge_rect_t){ fButtonX + (float)i * (fButtonW + fButtonGap), tClient.fY + tClient.fH - fButtonH - 14.0f, fButtonW, fButtonH };
			xgeXuiWidgetSetRect(pBox->arrButtonWidget[i], (xge_rect_t){ pBox->arrButtonRect[i].fX - tClient.fX, pBox->arrButtonRect[i].fY - tClient.fY, pBox->arrButtonRect[i].fW, pBox->arrButtonRect[i].fH });
		}
	}
	__xgeXuiMsgBoxSyncVisualRects(pBox);
	pBox->tBackdropRect = tRoot;
	pBox->iLayoutDirty = 0;
}

int xgeXuiMsgBoxInit(xge_xui_msg_box pBox, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;
	const xge_xui_chrome_style_t* pChrome;
	int i;

	if ( (pBox == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBox, 0, sizeof(*pBox));
	pTheme = xgeXuiGetTheme(pContext);
	pChrome = xgeXuiGetChromeStyle(pContext);
	pBox->pContext = pContext;
	pBox->pWidget = pWidget;
	pBox->pFont = pTheme->pFont;
	pBox->sTitle = "";
	pBox->sMessage = "";
	pBox->iType = XGE_XUI_MSG_BOX_ICON_INFO;
	pBox->iButtons = XGE_XUI_MSG_BOX_BUTTON_OK;
	pBox->iResult = XGE_XUI_MSG_BOX_RESULT_CLOSE;
	pBox->bModal = 1;
	pBox->bShowIcon = 1;
	pBox->iBackdropColor = XGE_COLOR_RGBA(24, 56, 79, 90);
	pBox->iBackgroundColor = pChrome->tWindow.iFrameBackground;
	pBox->iTitleColor = pChrome->tWindow.iTitleBackground;
	pBox->iCloseColor = pChrome->tWindow.iTitleText;
	pBox->iMessageColor = pTheme->iTextColor;
	pBox->iButtonColor = pTheme->iStateNormal;
	pBox->iButtonHoverColor = pTheme->iStateHover;
	pBox->iButtonActiveColor = pTheme->iStateActive;
	pBox->iButtonTextColor = pTheme->iTextColor;
	__xgeXuiMsgBoxApplyTypeColor(pBox);
	if ( xgeXuiWindowInit(&pBox->tWindow, pContext, pWidget) != XGE_OK ) {
		memset(pBox, 0, sizeof(*pBox));
		return XGE_ERROR;
	}
	xgeXuiWindowSetShowCollapse(&pBox->tWindow, 0);
	xgeXuiWindowSetShowMaximize(&pBox->tWindow, 0);
	xgeXuiWindowSetResizable(&pBox->tWindow, 0);
	xgeXuiWindowSetClose(&pBox->tWindow, __xgeXuiMsgBoxWindowClose, pBox);
	xgeXuiWindowSetColors(&pBox->tWindow, pBox->iBackgroundColor, pBox->iTitleColor, pChrome->tWindow.iTitleText, pChrome->tWindow.iBorder, pChrome->tWindow.iButtonNormal, pChrome->tWindow.iButtonHover, pChrome->tWindow.iButtonActive);
	pBox->pContentWidget = xgeXuiWidgetCreate();
	if ( pBox->pContentWidget == NULL ) {
		xgeXuiMsgBoxUnit(pBox);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetName(pBox->pContentWidget, "msgbox-content");
	xgeXuiWidgetSetRole(pBox->pContentWidget, XGE_XUI_WIDGET_ROLE_CONTAINER);
	xgeXuiWidgetSetClip(pBox->pContentWidget, 1);
	xgeXuiWidgetSetPaint(pBox->pContentWidget, xgeXuiMsgBoxPaintProc, pBox);
	if ( xgeXuiWidgetAddInternal(xgeXuiWindowGetClientWidget(&pBox->tWindow), pBox->pContentWidget) != XGE_OK ) {
		xgeXuiMsgBoxUnit(pBox);
		return XGE_ERROR;
	}
	for ( i = 0; i < XGE_XUI_MSG_BOX_BUTTON_CAPACITY; i++ ) {
		pBox->arrButtonWidget[i] = xgeXuiWidgetCreate();
		if ( pBox->arrButtonWidget[i] == NULL ) {
			xgeXuiMsgBoxUnit(pBox);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		xgeXuiWidgetSetName(pBox->arrButtonWidget[i], "msgbox-button");
		xgeXuiWidgetSetVisible(pBox->arrButtonWidget[i], 0);
		if ( xgeXuiButtonInit(&pBox->arrButton[i], pContext, pBox->arrButtonWidget[i]) != XGE_OK ) {
			xgeXuiMsgBoxUnit(pBox);
			return XGE_ERROR;
		}
		xgeXuiButtonSetClick(&pBox->arrButton[i], __xgeXuiMsgBoxButtonClick, pBox);
		if ( xgeXuiWidgetAddInternal(xgeXuiWindowGetClientWidget(&pBox->tWindow), pBox->arrButtonWidget[i]) != XGE_OK ) {
			xgeXuiMsgBoxUnit(pBox);
			return XGE_ERROR;
		}
	}
	xgeXuiWidgetSetPaintBefore(pWidget, __xgeXuiMsgBoxPaintBefore, pBox);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiMsgBoxEventProc, pBox);
	__xgeXuiMsgBoxUpdateButtons(pBox);
	xgeXuiWindowSetOpen(&pBox->tWindow, 0);
	return XGE_OK;
}

void xgeXuiMsgBoxUnit(xge_xui_msg_box pBox)
{
	int i;

	if ( pBox == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_MSG_BOX_BUTTON_CAPACITY; i++ ) {
		xgeXuiButtonUnit(&pBox->arrButton[i]);
	}
	if ( pBox->pWidget != NULL ) {
		xgeXuiWidgetSetEvent(pBox->pWidget, NULL, NULL);
		pBox->pWidget->procPaintBefore = NULL;
		pBox->pWidget->pPaintBeforeUser = NULL;
		pBox->pWidget->iCallbackFlags &= ~XGE_XUI_WIDGET_CALLBACK_PAINT_BEFORE;
	}
	xgeXuiWindowUnit(&pBox->tWindow);
	memset(pBox, 0, sizeof(*pBox));
}

void xgeXuiMsgBoxSetText(xge_xui_msg_box pBox, xge_font pFont, const char* sTitle, const char* sMessage)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->pFont = pFont;
	pBox->sTitle = (sTitle != NULL) ? sTitle : "";
	pBox->sMessage = (sMessage != NULL) ? sMessage : "";
	xgeXuiWindowSetTitle(&pBox->tWindow, pFont, pBox->sTitle);
	__xgeXuiMsgBoxSyncButtons(pBox);
	pBox->iLayoutDirty = 1;
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

void xgeXuiMsgBoxSetType(xge_xui_msg_box pBox, int iType)
{
	if ( pBox == NULL ) {
		return;
	}
	if ( iType < XGE_XUI_MSG_BOX_ICON_NONE || iType > XGE_XUI_MSG_BOX_ICON_ERROR ) {
		iType = XGE_XUI_MSG_BOX_ICON_INFO;
	}
	pBox->iType = iType;
	pBox->bShowIcon = (iType != XGE_XUI_MSG_BOX_ICON_NONE);
	pBox->bCustomIcon = 0;
	pBox->pIconTexture = NULL;
	__xgeXuiMsgBoxApplyTypeColor(pBox);
	pBox->iLayoutDirty = 1;
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

void xgeXuiMsgBoxSetIconTexture(xge_xui_msg_box pBox, xge_texture pTexture, xge_rect_t tSrc)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->pIconTexture = pTexture;
	pBox->tIconSrc = tSrc;
	pBox->bCustomIcon = (pTexture != NULL);
	pBox->bShowIcon = (pTexture != NULL);
	pBox->iType = (pTexture != NULL) ? pBox->iType : XGE_XUI_MSG_BOX_ICON_NONE;
	pBox->iLayoutDirty = 1;
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

void xgeXuiMsgBoxSetButtons(xge_xui_msg_box pBox, int iButtons)
{
	if ( pBox == NULL ) {
		return;
	}
	if ( (iButtons < XGE_XUI_MSG_BOX_BUTTON_OK) || (iButtons > XGE_XUI_MSG_BOX_BUTTON_YES_NO_CANCEL) ) {
		iButtons = XGE_XUI_MSG_BOX_BUTTON_OK;
	}
	pBox->iButtons = iButtons;
	__xgeXuiMsgBoxUpdateButtons(pBox);
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

void xgeXuiMsgBoxSetCustomButtons(xge_xui_msg_box pBox, xvalue arrButtons)
{
	xvalue pItem;
	xvalue pVal;
	const char* sText;
	const char* sStyle;
	int iCount;
	int iId;
	int iSemantic;
	int i;

	if ( pBox == NULL ) {
		return;
	}
	if ( (arrButtons == NULL) || (xvoType(arrButtons) != XVO_DT_ARRAY) ) {
		xgeXuiMsgBoxSetButtons(pBox, XGE_XUI_MSG_BOX_BUTTON_OK);
		return;
	}
	iCount = (int)xvoArrayItemCount(arrButtons);
	if ( iCount > XGE_XUI_MSG_BOX_BUTTON_CAPACITY ) {
		iCount = XGE_XUI_MSG_BOX_BUTTON_CAPACITY;
	}
	if ( iCount <= 0 ) {
		iCount = 1;
	}
	pBox->iButtons = XGE_XUI_MSG_BOX_BUTTON_CUSTOM;
	pBox->iButtonCount = iCount;
	for ( i = 0; i < iCount; i++ ) {
		pItem = xvoArrayGetValue(arrButtons, (uint32)i);
		sText = NULL;
		sStyle = NULL;
		iId = i;
		iSemantic = XGE_XUI_BUTTON_SEMANTIC_DEFAULT;
		if ( xvoType(pItem) == XVO_DT_TEXT ) {
			sText = (const char*)xvoGetText(pItem);
		} else if ( xvoType(pItem) == XVO_DT_TABLE ) {
			pVal = xvoTableGetValue(pItem, "text", 4);
			if ( xvoType(pVal) == XVO_DT_TEXT ) {
				sText = (const char*)xvoGetText(pVal);
			}
			pVal = xvoTableGetValue(pItem, "id", 2);
			if ( pVal != NULL && xvoType(pVal) == XVO_DT_INT ) {
				iId = (int)xvoGetInt(pVal);
			} else if ( pVal != NULL && xvoType(pVal) == XVO_DT_FLOAT ) {
				iId = (int)xvoGetFloat(pVal);
			}
			pVal = xvoTableGetValue(pItem, "style", 5);
			if ( xvoType(pVal) == XVO_DT_TEXT ) {
				sStyle = (const char*)xvoGetText(pVal);
			}
			pVal = xvoTableGetValue(pItem, "semantic", 8);
			if ( xvoType(pVal) == XVO_DT_TEXT ) {
				sStyle = (const char*)xvoGetText(pVal);
			} else if ( pVal != NULL && xvoType(pVal) == XVO_DT_INT ) {
				iSemantic = (int)xvoGetInt(pVal);
			}
		}
		iSemantic = __xgeXuiMsgBoxButtonSemanticFromText(sStyle, iSemantic);
		__xgeXuiMsgBoxSetButton(pBox, i, (sText != NULL) ? sText : "Button", iId, iSemantic);
	}
	__xgeXuiMsgBoxSyncButtons(pBox);
	pBox->iLayoutDirty = 1;
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

void xgeXuiMsgBoxSetResult(xge_xui_msg_box pBox, xge_xui_select_proc procResult, void* pUser)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->procResult = procResult;
	pBox->pUser = pUser;
}

void xgeXuiMsgBoxSetModal(xge_xui_msg_box pBox, int bModal)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->bModal = bModal ? 1 : 0;
	if ( xgeXuiMsgBoxIsOpen(pBox) ) {
		(void)xgeXuiOverlayAttach(pBox->pContext, pBox->pWidget, NULL, pBox->bModal ? XGE_XUI_LAYER_MODAL : XGE_XUI_LAYER_FLOATING);
		xgeXuiOverlayBringToFront(pBox->pContext, pBox->pWidget);
	}
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

void xgeXuiMsgBoxSetOpen(xge_xui_msg_box pBox, int bOpen)
{
	if ( pBox == NULL ) {
		return;
	}
	if ( bOpen ) {
		pBox->iResult = XGE_XUI_MSG_BOX_RESULT_CLOSE;
		__xgeXuiMsgBoxLayout(pBox);
		xgeXuiWindowSetOpen(&pBox->tWindow, 1);
		(void)xgeXuiOverlayAttach(pBox->pContext, pBox->pWidget, NULL, pBox->bModal ? XGE_XUI_LAYER_MODAL : XGE_XUI_LAYER_FLOATING);
		xgeXuiWindowBringToFront(&pBox->tWindow);
		xgeXuiSetFocus(pBox->pContext, pBox->pWidget);
	} else {
		xgeXuiWindowSetOpen(&pBox->tWindow, 0);
	}
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

int xgeXuiMsgBoxIsOpen(xge_xui_msg_box pBox)
{
	return (pBox != NULL) ? xgeXuiWindowIsOpen(&pBox->tWindow) : 0;
}

int xgeXuiMsgBoxGetResult(xge_xui_msg_box pBox)
{
	return (pBox != NULL) ? pBox->iResult : XGE_XUI_MSG_BOX_RESULT_CLOSE;
}

void xgeXuiMsgBoxSetColors(xge_xui_msg_box pBox, uint32_t iBackdrop, uint32_t iBackground, uint32_t iTitle, uint32_t iClose, uint32_t iMessage, uint32_t iButton, uint32_t iButtonHover, uint32_t iButtonText)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->iBackdropColor = iBackdrop;
	pBox->iBackgroundColor = iBackground;
	pBox->iTitleColor = iTitle;
	pBox->iCloseColor = iClose;
	pBox->iMessageColor = iMessage;
	pBox->iButtonColor = iButton;
	pBox->iButtonHoverColor = iButtonHover;
	pBox->iButtonActiveColor = iButtonHover;
	pBox->iButtonTextColor = iButtonText;
	xgeXuiWindowSetColors(&pBox->tWindow, iBackground, iTitle, iClose, pBox->tWindow.iBorderColor, pBox->tWindow.iButtonColorNormal, pBox->tWindow.iButtonColorHover, pBox->tWindow.iButtonColorActive);
	__xgeXuiMsgBoxSyncButtons(pBox);
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

static int __xgeXuiMsgBoxButtonAt(xge_xui_msg_box pBox, float fX, float fY)
{
	int i;

	if ( pBox == NULL ) {
		return -1;
	}
	for ( i = 0; i < pBox->iButtonCount; i++ ) {
		if ( __xgeXuiRectContains(pBox->arrButtonRect[i], fX, fY) ) {
			return i;
		}
	}
	return -1;
}

int xgeXuiMsgBoxEvent(xge_xui_msg_box pBox, const xge_event_t* pEvent)
{
	int iRet;
	int iButton;

	if ( (pBox == NULL) || (pEvent == NULL) || !xgeXuiMsgBoxIsOpen(pBox) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pBox->iLayoutDirty != 0 ) {
		__xgeXuiMsgBoxLayout(pBox);
	}
	__xgeXuiMsgBoxSyncVisualRects(pBox);
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN ) {
		if ( pEvent->iParam1 == XGE_KEY_ENTER ) {
			__xgeXuiMsgBoxCloseResult(pBox, (pBox->iButtonCount > 0) ? pBox->arrButtonResult[0] : XGE_XUI_MSG_BOX_RESULT_OK);
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( pEvent->iParam1 == XGE_KEY_ESCAPE ) {
			__xgeXuiMsgBoxCloseResult(pBox, XGE_XUI_MSG_BOX_RESULT_CLOSE);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_UP || pEvent->iType == XGE_EVENT_TOUCH_END ) {
		iButton = __xgeXuiMsgBoxButtonAt(pBox, pEvent->fX, pEvent->fY);
		if ( iButton >= 0 ) {
			__xgeXuiMsgBoxCloseResult(pBox, pBox->arrButtonResult[iButton]);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	iRet = xgeXuiWindowEvent(&pBox->tWindow, pEvent);
	if ( iRet != XGE_XUI_EVENT_CONTINUE ) {
		return iRet;
	}
	if ( pBox->bModal != 0 ) {
		if ( __xgeXuiBoxPointEvent(pEvent->iType) && (__xgeXuiRectContains(pBox->pWidget->tRect, pEvent->fX, pEvent->fY) == 0) ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( pEvent->iType == XGE_EVENT_KEY_DOWN || pEvent->iType == XGE_EVENT_KEY_UP || pEvent->iType == XGE_EVENT_TEXT ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int xgeXuiMsgBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiMsgBoxEvent((xge_xui_msg_box)pUser, pEvent);
}

void xgeXuiMsgBoxPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_msg_box pBox;
	xge_draw_t tDraw;
	xge_rect_t tIcon;
	char sIcon[2];

	(void)pWidget;
	pBox = (xge_xui_msg_box)pUser;
	if ( (pBox == NULL) || !xgeXuiMsgBoxIsOpen(pBox) ) {
		return;
	}
	if ( pBox->iLayoutDirty != 0 ) {
		__xgeXuiMsgBoxLayout(pBox);
	}
	__xgeXuiMsgBoxSyncVisualRects(pBox);
	if ( pBox->bShowIcon != 0 ) {
		tIcon = pBox->tIconRect;
		if ( pBox->bCustomIcon != 0 && pBox->pIconTexture != NULL ) {
			memset(&tDraw, 0, sizeof(tDraw));
			tDraw.pTexture = pBox->pIconTexture;
			tDraw.tSrc = pBox->tIconSrc;
			tDraw.tDst = tIcon;
			tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
			tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
			__xgeXuiHostDrawImage(&tDraw);
		} else {
			__xgeXuiHostDrawRect(tIcon, pBox->iIconColor);
			__xgeXuiHostDrawBorderRect(tIcon, 1.0f, XGE_COLOR_RGBA(245, 251, 255, 255));
			sIcon[0] = (pBox->iType == XGE_XUI_MSG_BOX_ICON_QUEST) ? '?' : ((pBox->iType == XGE_XUI_MSG_BOX_ICON_WAR) ? '!' : ((pBox->iType == XGE_XUI_MSG_BOX_ICON_ERROR) ? 'x' : 'i'));
			sIcon[1] = 0;
			if ( pBox->pFont != NULL ) {
				__xgeXuiHostDrawTextRect(pBox->pFont, sIcon, tIcon, XGE_COLOR_RGBA(255, 255, 255, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			}
		}
	}
	__xgeXuiBoxDrawWrappedText(pBox->pFont, pBox->sMessage, pBox->tMessageRect, pBox->iMessageColor);
}

static void __xgeXuiInputBoxCloseResult(xge_xui_input_box pBox, int iResult)
{
	const char* sText;

	if ( pBox == NULL ) {
		return;
	}
	pBox->iResult = iResult;
	if ( iResult == XGE_XUI_MSG_BOX_RESULT_OK ) {
		sText = xgeXuiInputGetText(&pBox->tInput);
		if ( pBox->sResult != NULL ) {
			xrtFree(pBox->sResult);
			pBox->sResult = NULL;
		}
		pBox->sResult = __xgeXuiBoxCopyText(sText);
	}
	if ( xgeXuiInputBoxIsOpen(pBox) ) {
		xgeXuiWindowSetOpen(&pBox->tWindow, 0);
	}
	if ( (iResult == XGE_XUI_MSG_BOX_RESULT_OK) && (pBox->procResult != NULL) ) {
		pBox->procResult(pBox->pWidget, (pBox->sResult != NULL) ? pBox->sResult : "", pBox->pUser);
	}
}

static void __xgeXuiInputBoxWindowClose(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	__xgeXuiInputBoxCloseResult((xge_xui_input_box)pUser, XGE_XUI_MSG_BOX_RESULT_CLOSE);
}

static void __xgeXuiInputBoxOkClick(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	__xgeXuiInputBoxCloseResult((xge_xui_input_box)pUser, XGE_XUI_MSG_BOX_RESULT_OK);
}

static void __xgeXuiInputBoxCancelClick(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	__xgeXuiInputBoxCloseResult((xge_xui_input_box)pUser, XGE_XUI_MSG_BOX_RESULT_CANCEL);
}

static void __xgeXuiInputBoxSubmit(xge_xui_widget pWidget, const char* sText, void* pUser)
{
	(void)pWidget;
	(void)sText;
	__xgeXuiInputBoxCloseResult((xge_xui_input_box)pUser, XGE_XUI_MSG_BOX_RESULT_OK);
}

static void __xgeXuiInputBoxPaintBefore(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_input_box pBox;

	(void)pWidget;
	pBox = (xge_xui_input_box)pUser;
	if ( (pBox == NULL) || (pBox->bModal == 0) || !xgeXuiInputBoxIsOpen(pBox) ) {
		return;
	}
	__xgeXuiHostDrawRect(pBox->tBackdropRect, pBox->iBackdropColor);
}

static void __xgeXuiInputBoxSyncVisualRects(xge_xui_input_box pBox)
{
	xge_rect_t tClient;
	float fButtonW;
	float fPadX;
	float fPadY;
	float fGapX;
	float fLeftW;

	if ( pBox == NULL ) {
		return;
	}
	tClient = __xgeXuiBoxCurrentClientRect(&pBox->tWindow, pBox->pWidget);
	if ( (tClient.fW <= 0.0f) || (tClient.fH <= 0.0f) ) {
		return;
	}
	fButtonW = 82.0f;
	fPadX = 18.0f;
	fPadY = 16.0f;
	fGapX = 12.0f;
	fLeftW = tClient.fW - fPadX * 2.0f - fGapX - fButtonW;
	if ( fLeftW < 160.0f ) {
		fLeftW = 160.0f;
	}
	pBox->tPromptRect = (xge_rect_t){ tClient.fX + fPadX, tClient.fY + fPadY, fLeftW, 30.0f };
}

static void __xgeXuiInputBoxLayout(xge_xui_input_box pBox)
{
	xge_rect_t tRoot;
	xge_rect_t tWin;
	xge_rect_t tClient;
	float fRootW;
	float fRootH;
	float fButtonW;
	float fButtonH;
	float fInputH;
	float fPadX;
	float fPadY;
	float fGapX;
	float fGapY;
	float fLeftW;
	float fRightX;
	float fInputY;

	if ( (pBox == NULL) || (pBox->pContext == NULL) || (pBox->pWidget == NULL) ) {
		return;
	}
	tRoot = (pBox->pContext->pRoot != NULL) ? pBox->pContext->pRoot->tRect : pBox->pWidget->tRect;
	fRootW = (tRoot.fW > 0.0f) ? tRoot.fW : 800.0f;
	fRootH = (tRoot.fH > 0.0f) ? tRoot.fH : 600.0f;
	tWin.fW = fRootW * 0.56f;
	if ( tWin.fW < 420.0f ) {
		tWin.fW = 420.0f;
	}
	if ( tWin.fW > fRootW * 0.8f ) {
		tWin.fW = fRootW * 0.8f;
	}
	tWin.fH = 152.0f;
	tWin.fX = tRoot.fX + (fRootW - tWin.fW) * 0.5f;
	tWin.fY = tRoot.fY + (fRootH - tWin.fH) * 0.42f;
	xgeXuiWidgetSetSize(pBox->pWidget, xgeXuiSizePx(tWin.fW), xgeXuiSizePx(tWin.fH));
	xgeXuiWidgetSetRect(pBox->pWidget, tWin);
	__xgeXuiWindowLayout(pBox->pWidget, &pBox->tWindow);
	tClient = xgeXuiWindowGetClientWidget(&pBox->tWindow)->tRect;
	if ( tClient.fW <= 0.0f || tClient.fH <= 0.0f ) {
		tClient = pBox->pWidget->tRect;
		tClient.fX += pBox->tWindow.fBorderWidth;
		tClient.fY += pBox->tWindow.fBorderWidth + pBox->tWindow.fTitleBarHeight;
		tClient.fW -= pBox->tWindow.fBorderWidth * 2.0f;
		tClient.fH -= pBox->tWindow.fBorderWidth * 2.0f + pBox->tWindow.fTitleBarHeight;
	}
	xgeXuiWidgetSetRect(pBox->pContentWidget, (xge_rect_t){ 0.0f, 0.0f, tClient.fW, tClient.fH });
	fButtonW = 82.0f;
	fButtonH = 30.0f;
	fInputH = 30.0f;
	fPadX = 18.0f;
	fPadY = 16.0f;
	fGapX = 12.0f;
	fGapY = 10.0f;
	fLeftW = tClient.fW - fPadX * 2.0f - fGapX - fButtonW;
	if ( fLeftW < 160.0f ) {
		fLeftW = 160.0f;
	}
	fRightX = fPadX + fLeftW + fGapX;
	fInputY = fPadY + fButtonH + fGapY;
	pBox->tPromptRect = (xge_rect_t){ tClient.fX + fPadX, tClient.fY + fPadY, fLeftW, fButtonH };
	xgeXuiWidgetSetRect(pBox->pInputWidget, (xge_rect_t){ fPadX, fInputY, fLeftW, fInputH });
	xgeXuiWidgetSetRect(pBox->pOkWidget, (xge_rect_t){ fRightX, fPadY, fButtonW, fButtonH });
	xgeXuiWidgetSetRect(pBox->pCancelWidget, (xge_rect_t){ fRightX, fInputY, fButtonW, fInputH });
	__xgeXuiInputBoxSyncVisualRects(pBox);
	pBox->tBackdropRect = tRoot;
	pBox->iLayoutDirty = 0;
}

int xgeXuiInputBoxInit(xge_xui_input_box pBox, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	const xge_xui_theme_t* pTheme;
	const xge_xui_chrome_style_t* pChrome;

	if ( (pBox == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBox, 0, sizeof(*pBox));
	pTheme = xgeXuiGetTheme(pContext);
	pChrome = xgeXuiGetChromeStyle(pContext);
	pBox->pContext = pContext;
	pBox->pWidget = pWidget;
	pBox->pFont = (pFont != NULL) ? pFont : pTheme->pFont;
	pBox->sTitle = "";
	pBox->sPrompt = "";
	pBox->iResult = XGE_XUI_MSG_BOX_RESULT_CLOSE;
	pBox->bModal = 1;
	pBox->iBackdropColor = XGE_COLOR_RGBA(24, 56, 79, 90);
	pBox->iBackgroundColor = pChrome->tWindow.iFrameBackground;
	pBox->iTitleColor = pChrome->tWindow.iTitleBackground;
	pBox->iCloseColor = pChrome->tWindow.iTitleText;
	pBox->iPromptColor = pTheme->iTextColor;
	pBox->iButtonColor = pTheme->iStateNormal;
	pBox->iButtonHoverColor = pTheme->iStateHover;
	pBox->iButtonActiveColor = pTheme->iStateActive;
	pBox->iButtonTextColor = pTheme->iTextColor;
	if ( xgeXuiWindowInit(&pBox->tWindow, pContext, pWidget) != XGE_OK ) {
		memset(pBox, 0, sizeof(*pBox));
		return XGE_ERROR;
	}
	xgeXuiWindowSetShowCollapse(&pBox->tWindow, 0);
	xgeXuiWindowSetShowMaximize(&pBox->tWindow, 0);
	xgeXuiWindowSetResizable(&pBox->tWindow, 0);
	xgeXuiWindowSetClose(&pBox->tWindow, __xgeXuiInputBoxWindowClose, pBox);
	xgeXuiWindowSetColors(&pBox->tWindow, pBox->iBackgroundColor, pBox->iTitleColor, pBox->iCloseColor, pChrome->tWindow.iBorder, pChrome->tWindow.iButtonNormal, pChrome->tWindow.iButtonHover, pChrome->tWindow.iButtonActive);
	pBox->pContentWidget = xgeXuiWidgetCreate();
	pBox->pInputWidget = xgeXuiWidgetCreate();
	pBox->pOkWidget = xgeXuiWidgetCreate();
	pBox->pCancelWidget = xgeXuiWidgetCreate();
	if ( (pBox->pContentWidget == NULL) || (pBox->pInputWidget == NULL) || (pBox->pOkWidget == NULL) || (pBox->pCancelWidget == NULL) ) {
		xgeXuiInputBoxUnit(pBox);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetName(pBox->pContentWidget, "inputbox-content");
	xgeXuiWidgetSetClip(pBox->pContentWidget, 1);
	xgeXuiWidgetSetPaint(pBox->pContentWidget, xgeXuiInputBoxPaintProc, pBox);
	if ( xgeXuiWidgetAddInternal(xgeXuiWindowGetClientWidget(&pBox->tWindow), pBox->pContentWidget) != XGE_OK ||
		xgeXuiWidgetAddInternal(xgeXuiWindowGetClientWidget(&pBox->tWindow), pBox->pInputWidget) != XGE_OK ||
		xgeXuiWidgetAddInternal(xgeXuiWindowGetClientWidget(&pBox->tWindow), pBox->pOkWidget) != XGE_OK ||
		xgeXuiWidgetAddInternal(xgeXuiWindowGetClientWidget(&pBox->tWindow), pBox->pCancelWidget) != XGE_OK ) {
		xgeXuiInputBoxUnit(pBox);
		return XGE_ERROR;
	}
	if ( xgeXuiInputInit(&pBox->tInput, pContext, pBox->pInputWidget, pBox->pFont) != XGE_OK ||
		xgeXuiButtonInit(&pBox->tOkButton, pContext, pBox->pOkWidget) != XGE_OK ||
		xgeXuiButtonInit(&pBox->tCancelButton, pContext, pBox->pCancelWidget) != XGE_OK ) {
		xgeXuiInputBoxUnit(pBox);
		return XGE_ERROR;
	}
	xgeXuiInputSetSubmit(&pBox->tInput, __xgeXuiInputBoxSubmit, pBox);
	xgeXuiButtonSetText(&pBox->tOkButton, pBox->pFont, "OK");
	xgeXuiButtonSetText(&pBox->tCancelButton, pBox->pFont, "Cancel");
	xgeXuiButtonSetSemantic(&pBox->tOkButton, XGE_XUI_BUTTON_SEMANTIC_PRIMARY);
	xgeXuiButtonSetClick(&pBox->tOkButton, __xgeXuiInputBoxOkClick, pBox);
	xgeXuiButtonSetClick(&pBox->tCancelButton, __xgeXuiInputBoxCancelClick, pBox);
	xgeXuiWidgetSetPaintBefore(pWidget, __xgeXuiInputBoxPaintBefore, pBox);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiInputBoxEventProc, pBox);
	xgeXuiWindowSetOpen(&pBox->tWindow, 0);
	return XGE_OK;
}

void xgeXuiInputBoxUnit(xge_xui_input_box pBox)
{
	if ( pBox == NULL ) {
		return;
	}
	xgeXuiButtonUnit(&pBox->tCancelButton);
	xgeXuiButtonUnit(&pBox->tOkButton);
	xgeXuiInputUnit(&pBox->tInput);
	if ( pBox->sResult != NULL ) {
		xrtFree(pBox->sResult);
	}
	if ( pBox->pWidget != NULL ) {
		xgeXuiWidgetSetEvent(pBox->pWidget, NULL, NULL);
		pBox->pWidget->procPaintBefore = NULL;
		pBox->pWidget->pPaintBeforeUser = NULL;
		pBox->pWidget->iCallbackFlags &= ~XGE_XUI_WIDGET_CALLBACK_PAINT_BEFORE;
	}
	xgeXuiWindowUnit(&pBox->tWindow);
	memset(pBox, 0, sizeof(*pBox));
}

void xgeXuiInputBoxSetText(xge_xui_input_box pBox, xge_font pFont, const char* sTitle, const char* sPrompt, const char* sInitial)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->pFont = pFont;
	pBox->sTitle = (sTitle != NULL) ? sTitle : "";
	pBox->sPrompt = (sPrompt != NULL) ? sPrompt : "";
	xgeXuiWindowSetTitle(&pBox->tWindow, pFont, pBox->sTitle);
	xgeXuiInputSetFont(&pBox->tInput, pFont);
	xgeXuiButtonSetText(&pBox->tOkButton, pFont, "OK");
	xgeXuiButtonSetText(&pBox->tCancelButton, pFont, "Cancel");
	xgeXuiInputSetText(&pBox->tInput, (sInitial != NULL) ? sInitial : "");
	pBox->iLayoutDirty = 1;
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

void xgeXuiInputBoxSetResult(xge_xui_input_box pBox, xge_xui_text_submit_proc procResult, void* pUser)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->procResult = procResult;
	pBox->pUser = pUser;
}

void xgeXuiInputBoxSetModal(xge_xui_input_box pBox, int bModal)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->bModal = bModal ? 1 : 0;
	if ( xgeXuiInputBoxIsOpen(pBox) ) {
		(void)xgeXuiOverlayAttach(pBox->pContext, pBox->pWidget, NULL, pBox->bModal ? XGE_XUI_LAYER_MODAL : XGE_XUI_LAYER_FLOATING);
		xgeXuiOverlayBringToFront(pBox->pContext, pBox->pWidget);
	}
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

void xgeXuiInputBoxSetOpen(xge_xui_input_box pBox, int bOpen)
{
	const char* sText;

	if ( pBox == NULL ) {
		return;
	}
	if ( bOpen ) {
		pBox->iResult = XGE_XUI_MSG_BOX_RESULT_CLOSE;
		sText = xgeXuiInputGetText(&pBox->tInput);
		xgeXuiInputSetSelection(&pBox->tInput, 0, (sText != NULL) ? (int)strlen(sText) : 0);
		__xgeXuiInputBoxLayout(pBox);
		xgeXuiWindowSetOpen(&pBox->tWindow, 1);
		(void)xgeXuiOverlayAttach(pBox->pContext, pBox->pWidget, NULL, pBox->bModal ? XGE_XUI_LAYER_MODAL : XGE_XUI_LAYER_FLOATING);
		xgeXuiWindowBringToFront(&pBox->tWindow);
		xgeXuiSetFocus(pBox->pContext, pBox->pInputWidget);
	} else {
		xgeXuiWindowSetOpen(&pBox->tWindow, 0);
	}
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

int xgeXuiInputBoxIsOpen(xge_xui_input_box pBox)
{
	return (pBox != NULL) ? xgeXuiWindowIsOpen(&pBox->tWindow) : 0;
}

int xgeXuiInputBoxGetResultCode(xge_xui_input_box pBox)
{
	return (pBox != NULL) ? pBox->iResult : XGE_XUI_MSG_BOX_RESULT_CLOSE;
}

char* xgeXuiInputBoxGetResult(xge_xui_input_box pBox)
{
	if ( (pBox == NULL) || (pBox->iResult != XGE_XUI_MSG_BOX_RESULT_OK) ) {
		return NULL;
	}
	return __xgeXuiBoxCopyText((pBox->sResult != NULL) ? pBox->sResult : "");
}

void xgeXuiInputBoxSetColors(xge_xui_input_box pBox, uint32_t iBackdrop, uint32_t iBackground, uint32_t iTitle, uint32_t iClose, uint32_t iPrompt, uint32_t iButton, uint32_t iButtonHover, uint32_t iButtonText)
{
	if ( pBox == NULL ) {
		return;
	}
	pBox->iBackdropColor = iBackdrop;
	pBox->iBackgroundColor = iBackground;
	pBox->iTitleColor = iTitle;
	pBox->iCloseColor = iClose;
	pBox->iPromptColor = iPrompt;
	pBox->iButtonColor = iButton;
	pBox->iButtonHoverColor = iButtonHover;
	pBox->iButtonActiveColor = iButtonHover;
	pBox->iButtonTextColor = iButtonText;
	xgeXuiWindowSetColors(&pBox->tWindow, iBackground, iTitle, iClose, pBox->tWindow.iBorderColor, pBox->tWindow.iButtonColorNormal, pBox->tWindow.iButtonColorHover, pBox->tWindow.iButtonColorActive);
	xgeXuiButtonSetColors(&pBox->tOkButton, iButton, iButtonHover, iButtonHover, iButtonHover, XGE_COLOR_RGBA(208, 218, 228, 255));
	xgeXuiButtonSetColors(&pBox->tCancelButton, iButton, iButtonHover, iButtonHover, iButtonHover, XGE_COLOR_RGBA(208, 218, 228, 255));
	xgeXuiButtonSetTextColor(&pBox->tOkButton, iButtonText);
	xgeXuiButtonSetTextColor(&pBox->tCancelButton, iButtonText);
	xgeXuiWidgetMarkPaint(pBox->pWidget);
}

int xgeXuiInputBoxEvent(xge_xui_input_box pBox, const xge_event_t* pEvent)
{
	int iRet;

	if ( (pBox == NULL) || (pEvent == NULL) || !xgeXuiInputBoxIsOpen(pBox) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pBox->iLayoutDirty != 0 ) {
		__xgeXuiInputBoxLayout(pBox);
	}
	__xgeXuiInputBoxSyncVisualRects(pBox);
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN ) {
		if ( pEvent->iParam1 == XGE_KEY_ESCAPE ) {
			__xgeXuiInputBoxCloseResult(pBox, XGE_XUI_MSG_BOX_RESULT_CANCEL);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	iRet = xgeXuiWindowEvent(&pBox->tWindow, pEvent);
	if ( iRet != XGE_XUI_EVENT_CONTINUE ) {
		return iRet;
	}
	if ( pBox->bModal != 0 ) {
		if ( __xgeXuiBoxPointEvent(pEvent->iType) && (__xgeXuiRectContains(pBox->pWidget->tRect, pEvent->fX, pEvent->fY) == 0) ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( pEvent->iType == XGE_EVENT_KEY_DOWN || pEvent->iType == XGE_EVENT_KEY_UP || pEvent->iType == XGE_EVENT_TEXT ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int xgeXuiInputBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiInputBoxEvent((xge_xui_input_box)pUser, pEvent);
}

void xgeXuiInputBoxPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_input_box pBox;

	(void)pWidget;
	pBox = (xge_xui_input_box)pUser;
	if ( (pBox == NULL) || !xgeXuiInputBoxIsOpen(pBox) ) {
		return;
	}
	if ( pBox->iLayoutDirty != 0 ) {
		__xgeXuiInputBoxLayout(pBox);
	}
	__xgeXuiInputBoxSyncVisualRects(pBox);
	if ( pBox->pFont != NULL ) {
		__xgeXuiHostDrawTextRect(pBox->pFont, pBox->sPrompt, pBox->tPromptRect, pBox->iPromptColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
}
