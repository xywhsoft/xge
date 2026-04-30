static int __xgeXuiToastNormalizePlacement(int iPlacement)
{
	if ( iPlacement < XGE_XUI_TOAST_PLACEMENT_TOP_RIGHT || iPlacement > XGE_XUI_TOAST_PLACEMENT_BOTTOM_CENTER ) {
		return XGE_XUI_TOAST_PLACEMENT_TOP_RIGHT;
	}
	return iPlacement;
}

static uint32_t __xgeXuiToastTypeColor(xge_xui_toast pToast, int iType)
{
	if ( pToast == NULL ) {
		return 0;
	}
	switch ( iType ) {
		case XGE_XUI_TOAST_TYPE_SUCCESS:
			return pToast->iSuccessColor;
		case XGE_XUI_TOAST_TYPE_WARNING:
			return pToast->iWarningColor;
		case XGE_XUI_TOAST_TYPE_ERROR:
			return pToast->iErrorColor;
		case XGE_XUI_TOAST_TYPE_INFO:
		default:
			return pToast->iInfoColor;
	}
}

static const char* __xgeXuiToastTypeText(int iType)
{
	switch ( iType ) {
		case XGE_XUI_TOAST_TYPE_SUCCESS:
			return "OK";
		case XGE_XUI_TOAST_TYPE_WARNING:
			return "!";
		case XGE_XUI_TOAST_TYPE_ERROR:
			return "X";
		case XGE_XUI_TOAST_TYPE_INFO:
		default:
			return "i";
	}
}

static void __xgeXuiToastLayout(xge_xui_toast pToast)
{
	xge_rect_t tBounds;
	float fX;
	float fY;
	float fStackH;
	int bBottom;
	int i;

	if ( (pToast == NULL) || (pToast->pWidget == NULL) ) {
		return;
	}
	tBounds = pToast->pWidget->tContentRect;
	if ( tBounds.fW <= 0.0f || tBounds.fH <= 0.0f ) {
		tBounds = pToast->pWidget->tRect;
	}
	fStackH = (float)pToast->iItemCount * pToast->fToastHeight + (float)((pToast->iItemCount > 0) ? (pToast->iItemCount - 1) : 0) * pToast->fSpacing;
	bBottom = (pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_BOTTOM_RIGHT || pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_BOTTOM_LEFT || pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_BOTTOM_CENTER);
	for ( i = 0; i < pToast->iItemCount; i++ ) {
		if ( pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_TOP_LEFT || pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_BOTTOM_LEFT ) {
			fX = tBounds.fX;
		} else if ( pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_TOP_CENTER || pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_BOTTOM_CENTER ) {
			fX = tBounds.fX + (tBounds.fW - pToast->fToastWidth) * 0.5f;
		} else {
			fX = tBounds.fX + tBounds.fW - pToast->fToastWidth;
		}
		if ( bBottom ) {
			fY = tBounds.fY + tBounds.fH - fStackH + (float)i * (pToast->fToastHeight + pToast->fSpacing);
		} else {
			fY = tBounds.fY + (float)i * (pToast->fToastHeight + pToast->fSpacing);
		}
		if ( fX < tBounds.fX ) {
			fX = tBounds.fX;
		}
		if ( fY < tBounds.fY ) {
			fY = tBounds.fY;
		}
		pToast->arrItems[i].tRect = (xge_rect_t){ fX, fY, pToast->fToastWidth, pToast->fToastHeight };
		pToast->arrItems[i].tCloseRect = (xge_rect_t){ fX + pToast->fToastWidth - 22.0f, fY + 6.0f, 16.0f, 16.0f };
	}
}

static void __xgeXuiToastRemoveAt(xge_xui_toast pToast, int iIndex)
{
	int i;

	if ( (pToast == NULL) || (iIndex < 0) || (iIndex >= pToast->iItemCount) ) {
		return;
	}
	for ( i = iIndex; i + 1 < pToast->iItemCount; i++ ) {
		pToast->arrItems[i] = pToast->arrItems[i + 1];
	}
	pToast->iItemCount--;
	if ( pToast->iHoverClose >= pToast->iItemCount ) {
		pToast->iHoverClose = -1;
	}
	__xgeXuiToastLayout(pToast);
	xgeXuiWidgetMarkPaint(pToast->pWidget);
}

int xgeXuiToastInit(xge_xui_toast pToast, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	const xge_xui_theme_t* pTheme;

	if ( (pToast == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pToast, 0, sizeof(*pToast));
	pTheme = xgeXuiGetTheme(pContext);
	pToast->pContext = pContext;
	pToast->pWidget = pWidget;
	pToast->pFont = (pFont != NULL) ? pFont : pTheme->pFont;
	pToast->iPlacement = XGE_XUI_TOAST_PLACEMENT_TOP_RIGHT;
	pToast->fToastWidth = 260.0f;
	pToast->fToastHeight = 58.0f;
	pToast->fSpacing = 8.0f;
	pToast->iHoverClose = -1;
	pToast->iBackgroundColor = XGE_COLOR_RGBA(247, 252, 255, 245);
	pToast->iBorderColor = XGE_COLOR_RGBA(129, 174, 207, 255);
	pToast->iTextColor = XGE_COLOR_RGBA(31, 58, 82, 255);
	pToast->iMutedTextColor = XGE_COLOR_RGBA(92, 112, 130, 255);
	pToast->iInfoColor = XGE_COLOR_RGBA(78, 159, 220, 255);
	pToast->iSuccessColor = XGE_COLOR_RGBA(43, 184, 150, 255);
	pToast->iWarningColor = XGE_COLOR_RGBA(244, 187, 68, 255);
	pToast->iErrorColor = XGE_COLOR_RGBA(224, 92, 92, 255);
	pToast->iCloseColor = XGE_COLOR_RGBA(96, 126, 148, 255);
	pToast->iCloseHoverColor = XGE_COLOR_RGBA(31, 58, 82, 255);
	xgeXuiWidgetSetClip(pWidget, 1);
	pWidget->procEvent = xgeXuiToastEventProc;
	pWidget->procUpdate = xgeXuiToastUpdateProc;
	pWidget->procPaint = xgeXuiToastPaintProc;
	pWidget->pUser = pToast;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiToastUnit(xge_xui_toast pToast)
{
	if ( pToast == NULL ) {
		return;
	}
	if ( pToast->pWidget != NULL && pToast->pWidget->pUser == pToast ) {
		pToast->pWidget->pUser = NULL;
		pToast->pWidget->procEvent = NULL;
		pToast->pWidget->procUpdate = NULL;
		pToast->pWidget->procPaint = NULL;
	}
	memset(pToast, 0, sizeof(*pToast));
}

void xgeXuiToastClear(xge_xui_toast pToast)
{
	if ( pToast == NULL ) {
		return;
	}
	pToast->iItemCount = 0;
	pToast->iHoverClose = -1;
	xgeXuiWidgetMarkPaint(pToast->pWidget);
}

int xgeXuiToastShow(xge_xui_toast pToast, int iType, const char* sTitle, const char* sMessage, float fDuration)
{
	xge_xui_toast_item_t* pItem;
	int i;

	if ( pToast == NULL ) {
		return -1;
	}
	if ( pToast->iItemCount >= XGE_XUI_TOAST_CAPACITY ) {
		for ( i = 0; i + 1 < pToast->iItemCount; i++ ) {
			pToast->arrItems[i] = pToast->arrItems[i + 1];
		}
		pToast->iItemCount--;
	}
	pItem = &pToast->arrItems[pToast->iItemCount];
	memset(pItem, 0, sizeof(*pItem));
	pItem->iType = (iType < XGE_XUI_TOAST_TYPE_INFO || iType > XGE_XUI_TOAST_TYPE_ERROR) ? XGE_XUI_TOAST_TYPE_INFO : iType;
	pItem->sTitle = (sTitle != NULL) ? sTitle : "";
	pItem->sMessage = (sMessage != NULL) ? sMessage : "";
	pItem->fDuration = (fDuration <= 0.0f) ? 3.0f : fDuration;
	pToast->iItemCount++;
	pToast->iShowCount++;
	__xgeXuiToastLayout(pToast);
	xgeXuiWidgetMarkPaint(pToast->pWidget);
	return pToast->iItemCount - 1;
}

int xgeXuiToastClose(xge_xui_toast pToast, int iIndex)
{
	if ( (pToast == NULL) || (iIndex < 0) || (iIndex >= pToast->iItemCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiToastRemoveAt(pToast, iIndex);
	pToast->iCloseCount++;
	return XGE_OK;
}

int xgeXuiToastGetCount(xge_xui_toast pToast)
{
	return (pToast != NULL) ? pToast->iItemCount : 0;
}

void xgeXuiToastSetPlacement(xge_xui_toast pToast, int iPlacement)
{
	if ( pToast == NULL ) {
		return;
	}
	pToast->iPlacement = __xgeXuiToastNormalizePlacement(iPlacement);
	__xgeXuiToastLayout(pToast);
	xgeXuiWidgetMarkPaint(pToast->pWidget);
}

void xgeXuiToastSetMetrics(xge_xui_toast pToast, float fToastWidth, float fToastHeight, float fSpacing)
{
	if ( pToast == NULL ) {
		return;
	}
	pToast->fToastWidth = (fToastWidth < 80.0f) ? 80.0f : fToastWidth;
	pToast->fToastHeight = (fToastHeight < 32.0f) ? 32.0f : fToastHeight;
	pToast->fSpacing = (fSpacing < 0.0f) ? 0.0f : fSpacing;
	__xgeXuiToastLayout(pToast);
	xgeXuiWidgetMarkPaint(pToast->pWidget);
}

void xgeXuiToastSetColors(xge_xui_toast pToast, uint32_t iBackground, uint32_t iBorder, uint32_t iText, uint32_t iMutedText, uint32_t iInfo, uint32_t iSuccess, uint32_t iWarning, uint32_t iError)
{
	if ( pToast == NULL ) {
		return;
	}
	pToast->iBackgroundColor = iBackground;
	pToast->iBorderColor = iBorder;
	pToast->iTextColor = iText;
	pToast->iMutedTextColor = iMutedText;
	pToast->iInfoColor = iInfo;
	pToast->iSuccessColor = iSuccess;
	pToast->iWarningColor = iWarning;
	pToast->iErrorColor = iError;
	xgeXuiWidgetMarkPaint(pToast->pWidget);
}

int xgeXuiToastEvent(xge_xui_toast pToast, const xge_event_t* pEvent)
{
	int i;
	int iHover;

	if ( (pToast == NULL) || (pToast->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	__xgeXuiToastLayout(pToast);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
			iHover = -1;
			for ( i = 0; i < pToast->iItemCount; i++ ) {
				if ( __xgeXuiRectContains(pToast->arrItems[i].tCloseRect, pEvent->fX, pEvent->fY) ) {
					iHover = i;
					break;
				}
			}
			if ( pToast->iHoverClose != iHover ) {
				pToast->iHoverClose = iHover;
				xgeXuiWidgetMarkPaint(pToast->pWidget);
			}
			return (iHover >= 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
			for ( i = 0; i < pToast->iItemCount; i++ ) {
				if ( __xgeXuiRectContains(pToast->arrItems[i].tCloseRect, pEvent->fX, pEvent->fY) ) {
					return xgeXuiToastClose(pToast, i) == XGE_OK ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
				}
			}
			return XGE_XUI_EVENT_CONTINUE;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiToastEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiToastEvent((xge_xui_toast)pUser, pEvent);
}

void xgeXuiToastUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser)
{
	xge_xui_toast pToast;
	int i;

	(void)pWidget;
	pToast = (xge_xui_toast)pUser;
	if ( (pToast == NULL) || (fDelta <= 0.0f) ) {
		return;
	}
	i = 0;
	while ( i < pToast->iItemCount ) {
		pToast->arrItems[i].fElapsed += fDelta;
		if ( pToast->arrItems[i].fElapsed >= pToast->arrItems[i].fDuration ) {
			__xgeXuiToastRemoveAt(pToast, i);
			pToast->iExpireCount++;
			continue;
		}
		i++;
	}
}

void xgeXuiToastPaintProc(xge_xui_widget pWidget, void* pUser)
{
	static const uint16_t arrClose10[10] = {
		0x201, 0x102, 0x084, 0x048, 0x030,
		0x030, 0x048, 0x084, 0x102, 0x201
	};
	xge_xui_toast pToast;
	xge_xui_toast_item_t* pItem;
	xge_rect_t tRect;
	xge_rect_t tBand;
	xge_rect_t tIcon;
	xge_rect_t tTitle;
	xge_rect_t tMessage;
	xge_rect_t tCloseIcon;
	uint32_t iTypeColor;
	uint32_t iCloseColor;
	int i;

	pToast = (xge_xui_toast)pUser;
	if ( (pWidget == NULL) || (pToast == NULL) ) {
		return;
	}
	__xgeXuiToastLayout(pToast);
	for ( i = 0; i < pToast->iItemCount; i++ ) {
		pItem = &pToast->arrItems[i];
		tRect = pItem->tRect;
		iTypeColor = __xgeXuiToastTypeColor(pToast, pItem->iType);
		__xgeXuiHostDrawRect(tRect, pToast->iBackgroundColor);
		__xgeXuiHostDrawBorderRect(tRect, 1.0f, pToast->iBorderColor);
		tBand = tRect;
		tBand.fW = 4.0f;
		__xgeXuiHostDrawRect(tBand, iTypeColor);
		tIcon = (xge_rect_t){ tRect.fX + 12.0f, tRect.fY + 12.0f, 24.0f, 24.0f };
		__xgeXuiHostDrawRect(tIcon, XGE_COLOR_RGBA(XGE_COLOR_GET_R(iTypeColor), XGE_COLOR_GET_G(iTypeColor), XGE_COLOR_GET_B(iTypeColor), 42));
		__xgeXuiHostDrawBorderRect(tIcon, 1.0f, iTypeColor);
		if ( pToast->pFont != NULL ) {
			__xgeXuiHostDrawTextRect(pToast->pFont, __xgeXuiToastTypeText(pItem->iType), tIcon, iTypeColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			tTitle = (xge_rect_t){ tRect.fX + 44.0f, tRect.fY + 8.0f, tRect.fW - 74.0f, 20.0f };
			tMessage = (xge_rect_t){ tRect.fX + 44.0f, tRect.fY + 30.0f, tRect.fW - 54.0f, 18.0f };
			__xgeXuiHostDrawTextRect(pToast->pFont, pItem->sTitle != NULL ? pItem->sTitle : "", tTitle, pToast->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			__xgeXuiHostDrawTextRect(pToast->pFont, pItem->sMessage != NULL ? pItem->sMessage : "", tMessage, pToast->iMutedTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		iCloseColor = (i == pToast->iHoverClose) ? pToast->iCloseHoverColor : pToast->iCloseColor;
		tCloseIcon = (xge_rect_t){ pItem->tCloseRect.fX + 3.0f, pItem->tCloseRect.fY + 3.0f, 10.0f, 10.0f };
		__xgeXuiHostDrawBitmapMask(tCloseIcon, arrClose10, 10, 10, iCloseColor);
	}
}
