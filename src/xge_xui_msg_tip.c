static int __xgeXuiMsgTipNormalizeType(int iType)
{
	if ( iType < XGE_XUI_MSG_TIP_ICON_NONE || iType > XGE_XUI_MSG_TIP_ICON_ERROR ) {
		return XGE_XUI_MSG_TIP_ICON_INFO;
	}
	return iType;
}

static void __xgeXuiMsgTipApplyTypeColor(xge_xui_msg_tip pTip)
{
	if ( pTip == NULL ) {
		return;
	}
	if ( pTip->iType == XGE_XUI_MSG_TIP_ICON_WAR ) {
		pTip->iIconColor = XGE_COLOR_RGBA(244, 187, 68, 255);
	} else if ( pTip->iType == XGE_XUI_MSG_TIP_ICON_ERROR ) {
		pTip->iIconColor = XGE_COLOR_RGBA(224, 92, 92, 255);
	} else if ( pTip->iType == XGE_XUI_MSG_TIP_ICON_QUEST ) {
		pTip->iIconColor = XGE_COLOR_RGBA(86, 156, 224, 255);
	} else {
		pTip->iIconColor = XGE_COLOR_RGBA(72, 190, 148, 255);
	}
}

static int __xgeXuiMsgTipHasIcon(xge_xui_msg_tip pTip)
{
	return (pTip != NULL) && ((pTip->bCustomIcon != 0 && pTip->pIconTexture != NULL) || pTip->iType != XGE_XUI_MSG_TIP_ICON_NONE);
}

static void __xgeXuiMsgTipLayout(xge_xui_msg_tip pTip)
{
	xge_rect_t tBounds;
	xge_vec2_t tText;
	float fIconBlock;
	float fTextLimit;
	float fTextW;
	float fLineH;
	float fContentW;
	float fContentH;
	float fWidth;
	float fHeight;
	int iLines;

	if ( (pTip == NULL) || (pTip->pWidget == NULL) ) {
		return;
	}
	tBounds = pTip->pWidget->tContentRect;
	if ( tBounds.fW <= 0.0f || tBounds.fH <= 0.0f ) {
		tBounds = pTip->pWidget->tRect;
	}
	if ( tBounds.fW <= 0.0f || tBounds.fH <= 0.0f ) {
		return;
	}
	pTip->bShowIcon = __xgeXuiMsgTipHasIcon(pTip);
	fIconBlock = pTip->bShowIcon ? (pTip->fIconSize + pTip->fIconGap) : 0.0f;
	fTextLimit = pTip->fMaxWidth - pTip->fPaddingX * 2.0f - fIconBlock;
	if ( fTextLimit < 32.0f ) {
		fTextLimit = 32.0f;
	}
	tText = __xgeXuiHostMeasureText(pTip->pFont, (pTip->sText != NULL) ? pTip->sText : "");
	fTextW = tText.fX;
	if ( fTextW > fTextLimit ) {
		fTextW = fTextLimit;
	}
	iLines = __xgeXuiBoxWrapMeasure(pTip->pFont, (pTip->sText != NULL) ? pTip->sText : "", fTextLimit, &fTextW);
	fLineH = __xgeXuiBoxLineHeight(pTip->pFont);
	fContentW = fIconBlock + fTextW;
	fContentH = (float)iLines * fLineH;
	if ( pTip->bShowIcon && fContentH < pTip->fIconSize ) {
		fContentH = pTip->fIconSize;
	}
	fWidth = fContentW + pTip->fPaddingX * 2.0f;
	if ( fWidth < pTip->fMinWidth ) {
		fWidth = pTip->fMinWidth;
	}
	if ( fWidth > pTip->fMaxWidth ) {
		fWidth = pTip->fMaxWidth;
	}
	if ( fWidth > tBounds.fW ) {
		fWidth = tBounds.fW;
	}
	fHeight = fContentH + pTip->fPaddingY * 2.0f;
	if ( fHeight < pTip->fMinHeight ) {
		fHeight = pTip->fMinHeight;
	}
	if ( fHeight > tBounds.fH ) {
		fHeight = tBounds.fH;
	}
	pTip->tRect = (xge_rect_t){
		tBounds.fX + (tBounds.fW - fWidth) * 0.5f,
		tBounds.fY + (tBounds.fH - fHeight) * 0.5f + pTip->fOffsetY,
		fWidth,
		fHeight
	};
	if ( pTip->tRect.fY < tBounds.fY ) {
		pTip->tRect.fY = tBounds.fY;
	}
	if ( pTip->tRect.fY + pTip->tRect.fH > tBounds.fY + tBounds.fH ) {
		pTip->tRect.fY = tBounds.fY + tBounds.fH - pTip->tRect.fH;
	}
	pTip->tIconRect = (xge_rect_t){ pTip->tRect.fX + pTip->fPaddingX, pTip->tRect.fY + (pTip->tRect.fH - pTip->fIconSize) * 0.5f, pTip->fIconSize, pTip->fIconSize };
	pTip->tTextRect = (xge_rect_t){
		pTip->tRect.fX + pTip->fPaddingX + fIconBlock,
		pTip->tRect.fY + (pTip->tRect.fH - fContentH) * 0.5f,
		pTip->tRect.fW - pTip->fPaddingX * 2.0f - fIconBlock,
		fContentH
	};
	if ( pTip->pHitWidget != NULL ) {
		xgeXuiWidgetSetRect(pTip->pHitWidget, pTip->bOpen ? pTip->tRect : (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f });
		xgeXuiWidgetSetHitTestVisible(pTip->pHitWidget, pTip->bOpen);
	}
}

static void __xgeXuiMsgTipDrawRoundedRect(xge_rect_t tRect, float fRadius, uint32_t iColor)
{
	xge_rect_t tRow;
	float fY;
	float fHalf;
	float fDy;
	float fInset;

	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		return;
	}
	if ( fRadius <= 0.0f ) {
		__xgeXuiHostDrawRect(tRect, iColor);
		return;
	}
	if ( fRadius > tRect.fW * 0.5f ) {
		fRadius = tRect.fW * 0.5f;
	}
	if ( fRadius > tRect.fH * 0.5f ) {
		fRadius = tRect.fH * 0.5f;
	}
	if ( fRadius < 1.0f ) {
		__xgeXuiHostDrawRect(tRect, iColor);
		return;
	}
	__xgeXuiHostDrawRect((xge_rect_t){ tRect.fX, tRect.fY + fRadius, tRect.fW, tRect.fH - fRadius * 2.0f }, iColor);
	fHalf = floorf(fRadius);
	for ( fY = 0.0f; fY < fHalf; fY += 1.0f ) {
		fDy = fRadius - fY - 0.5f;
		fInset = fRadius - sqrtf(fRadius * fRadius - fDy * fDy);
		tRow = (xge_rect_t){ tRect.fX + fInset, tRect.fY + fY, tRect.fW - fInset * 2.0f, 1.0f };
		__xgeXuiHostDrawRect(tRow, iColor);
		tRow.fY = tRect.fY + tRect.fH - fY - 1.0f;
		__xgeXuiHostDrawRect(tRow, iColor);
	}
}

int xgeXuiMsgTipInit(xge_xui_msg_tip pTip, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	const xge_xui_theme_t* pTheme;

	if ( (pTip == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pTip, 0, sizeof(*pTip));
	__xgeXuiOverlayWidgetInit(pWidget, 0);
	pTheme = xgeXuiGetTheme(pContext);
	pTip->pHitWidget = xgeXuiWidgetCreate();
	if ( pTip->pHitWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pTip->pContext = pContext;
	pTip->pWidget = pWidget;
	pTip->pFont = (pFont != NULL) ? pFont : ((pTheme != NULL) ? pTheme->pFont : NULL);
	pTip->iType = XGE_XUI_MSG_TIP_ICON_NONE;
	pTip->fMinWidth = 92.0f;
	pTip->fMaxWidth = 360.0f;
	pTip->fMinHeight = 40.0f;
	pTip->fPaddingX = 16.0f;
	pTip->fPaddingY = 10.0f;
	pTip->fIconSize = 20.0f;
	pTip->fIconGap = 8.0f;
	pTip->fOffsetY = -72.0f;
	pTip->iBackgroundColor = XGE_COLOR_RGBA(32, 32, 32, 205);
	pTip->iTextColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	__xgeXuiMsgTipApplyTypeColor(pTip);
	xgeXuiWidgetSetBackground(pWidget, 0);
	xgeXuiWidgetSetBorder(pWidget, 0.0f, 0);
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetInputTransparent(pWidget, 1);
	pWidget->procUpdate = xgeXuiMsgTipUpdateProc;
	pWidget->procPaint = xgeXuiMsgTipPaintProc;
	pWidget->pUser = pTip;
	xgeXuiWidgetSetBackground(pTip->pHitWidget, 0);
	xgeXuiWidgetSetBorder(pTip->pHitWidget, 0.0f, 0);
	xgeXuiWidgetSetHitTestVisible(pTip->pHitWidget, 0);
	xgeXuiWidgetSetEvent(pTip->pHitWidget, xgeXuiMsgTipEventProc, pTip);
	xgeXuiWidgetAdd(pWidget, pTip->pHitWidget);
	return XGE_OK;
}

void xgeXuiMsgTipUnit(xge_xui_msg_tip pTip)
{
	if ( pTip == NULL ) {
		return;
	}
	if ( pTip->sText != NULL ) {
		xrtFree(pTip->sText);
		pTip->sText = NULL;
	}
	if ( pTip->pWidget != NULL && pTip->pWidget->pUser == pTip ) {
		pTip->pWidget->pUser = NULL;
		pTip->pWidget->procUpdate = NULL;
		pTip->pWidget->procPaint = NULL;
		xgeXuiWidgetSetInputTransparent(pTip->pWidget, 0);
	}
	if ( pTip->pHitWidget != NULL ) {
		xgeXuiWidgetFree(pTip->pHitWidget);
		pTip->pHitWidget = NULL;
	}
	memset(pTip, 0, sizeof(*pTip));
}

int xgeXuiMsgTipShow(xge_xui_msg_tip pTip, int iType, const char* sText, float fDuration)
{
	if ( pTip == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiMsgTipSetType(pTip, iType);
	xgeXuiMsgTipSetText(pTip, sText);
	pTip->fDuration = (fDuration < 0.0f) ? 0.0f : fDuration;
	pTip->fElapsed = 0.0f;
	pTip->bOpen = 1;
	pTip->iShowCount++;
	__xgeXuiMsgTipLayout(pTip);
	xgeXuiWidgetMarkPaint(pTip->pWidget);
	return XGE_OK;
}

void xgeXuiMsgTipClose(xge_xui_msg_tip pTip)
{
	if ( (pTip == NULL) || (pTip->bOpen == 0) ) {
		return;
	}
	pTip->bOpen = 0;
	pTip->iCloseCount++;
	if ( pTip->pHitWidget != NULL ) {
		xgeXuiWidgetSetHitTestVisible(pTip->pHitWidget, 0);
		xgeXuiWidgetSetRect(pTip->pHitWidget, (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f });
	}
	xgeXuiWidgetMarkPaint(pTip->pWidget);
}

int xgeXuiMsgTipIsOpen(xge_xui_msg_tip pTip)
{
	return (pTip != NULL) ? pTip->bOpen : 0;
}

void xgeXuiMsgTipSetText(xge_xui_msg_tip pTip, const char* sText)
{
	char* sCopy;

	if ( pTip == NULL ) {
		return;
	}
	sCopy = __xgeXuiBoxCopyText(sText);
	if ( sCopy == NULL ) {
		return;
	}
	if ( pTip->sText != NULL ) {
		xrtFree(pTip->sText);
	}
	pTip->sText = sCopy;
	__xgeXuiMsgTipLayout(pTip);
	xgeXuiWidgetMarkPaint(pTip->pWidget);
}

void xgeXuiMsgTipSetType(xge_xui_msg_tip pTip, int iType)
{
	if ( pTip == NULL ) {
		return;
	}
	pTip->iType = __xgeXuiMsgTipNormalizeType(iType);
	__xgeXuiMsgTipApplyTypeColor(pTip);
	__xgeXuiMsgTipLayout(pTip);
	xgeXuiWidgetMarkPaint(pTip->pWidget);
}

void xgeXuiMsgTipSetIconTexture(xge_xui_msg_tip pTip, xge_texture pTexture, xge_rect_t tSrc)
{
	if ( pTip == NULL ) {
		return;
	}
	pTip->pIconTexture = pTexture;
	pTip->tIconSrc = tSrc;
	pTip->bCustomIcon = (pTexture != NULL);
	__xgeXuiMsgTipLayout(pTip);
	xgeXuiWidgetMarkPaint(pTip->pWidget);
}

void xgeXuiMsgTipSetMetrics(xge_xui_msg_tip pTip, float fMinWidth, float fMaxWidth, float fMinHeight, float fOffsetY)
{
	if ( pTip == NULL ) {
		return;
	}
	pTip->fMinWidth = (fMinWidth < 32.0f) ? 32.0f : fMinWidth;
	pTip->fMaxWidth = (fMaxWidth < pTip->fMinWidth) ? pTip->fMinWidth : fMaxWidth;
	pTip->fMinHeight = (fMinHeight < 24.0f) ? 24.0f : fMinHeight;
	pTip->fOffsetY = fOffsetY;
	__xgeXuiMsgTipLayout(pTip);
	xgeXuiWidgetMarkPaint(pTip->pWidget);
}

void xgeXuiMsgTipSetColors(xge_xui_msg_tip pTip, uint32_t iBackground, uint32_t iText, uint32_t iIcon)
{
	if ( pTip == NULL ) {
		return;
	}
	pTip->iBackgroundColor = iBackground;
	pTip->iTextColor = iText;
	pTip->iIconColor = iIcon;
	xgeXuiWidgetMarkPaint(pTip->pWidget);
}

int xgeXuiMsgTipEvent(xge_xui_msg_tip pTip, const xge_event_t* pEvent)
{
	if ( (pTip == NULL) || (pEvent == NULL) || (pTip->bOpen == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_DOWN || pEvent->iType == XGE_EVENT_TOUCH_BEGIN ) {
		__xgeXuiMsgTipLayout(pTip);
		if ( __xgeXuiRectContains(pTip->tRect, pEvent->fX, pEvent->fY) ) {
			xgeXuiMsgTipClose(pTip);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int xgeXuiMsgTipEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( (pUser == NULL) && (pWidget != NULL) ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiMsgTipEvent((xge_xui_msg_tip)pUser, pEvent);
}

void xgeXuiMsgTipUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser)
{
	xge_xui_msg_tip pTip;

	if ( (pUser == NULL) && (pWidget != NULL) ) {
		pUser = pWidget->pUser;
	}
	pTip = (xge_xui_msg_tip)pUser;
	if ( (pTip == NULL) || (pTip->bOpen == 0) || (pTip->fDuration <= 0.0f) || (fDelta <= 0.0f) ) {
		return;
	}
	pTip->fElapsed += fDelta;
	if ( pTip->fElapsed >= pTip->fDuration ) {
		pTip->bOpen = 0;
		pTip->iExpireCount++;
		if ( pTip->pHitWidget != NULL ) {
			xgeXuiWidgetSetHitTestVisible(pTip->pHitWidget, 0);
			xgeXuiWidgetSetRect(pTip->pHitWidget, (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f });
		}
		xgeXuiWidgetMarkPaint(pTip->pWidget);
	}
}

void xgeXuiMsgTipPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_msg_tip pTip;
	xge_draw_t tDraw;
	int iAsset;

	if ( (pUser == NULL) && (pWidget != NULL) ) {
		pUser = pWidget->pUser;
	}
	pTip = (xge_xui_msg_tip)pUser;
	if ( (pTip == NULL) || (pTip->bOpen == 0) ) {
		return;
	}
	__xgeXuiMsgTipLayout(pTip);
	__xgeXuiMsgTipDrawRoundedRect(pTip->tRect, 5.0f, pTip->iBackgroundColor);
	if ( pTip->bShowIcon ) {
		if ( pTip->bCustomIcon != 0 && pTip->pIconTexture != NULL ) {
			memset(&tDraw, 0, sizeof(tDraw));
			tDraw.pTexture = pTip->pIconTexture;
			tDraw.tSrc = pTip->tIconSrc;
			tDraw.tDst = pTip->tIconRect;
			tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
			tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
			__xgeXuiHostDrawImage(&tDraw);
		} else {
			iAsset = __xgeXuiMsgBoxBuiltinIconAsset(pTip->iType);
			if ( iAsset >= 0 ) {
				__xgeXuiBuiltinAssetDraw(pTip->tIconRect, iAsset, pTip->iIconColor);
			}
		}
	}
	__xgeXuiBoxDrawWrappedText(pTip->pFont, (pTip->sText != NULL) ? pTip->sText : "", pTip->tTextRect, pTip->iTextColor);
}
