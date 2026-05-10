static int __xgeXuiToggleIsChecked(xge_xui_toggle pToggle)
{
	return (pToggle != NULL && pToggle->pWidget != NULL && (pToggle->pWidget->iVisualState & XGE_XUI_STATE_CHECKED) != 0) ? 1 : 0;
}

static void __xgeXuiToggleCacheInvalidate(xge_xui_toggle pToggle, int bLayout)
{
	if ( pToggle == NULL ) {
		return;
	}
	__xgeXuiRenderCacheInvalidate(&pToggle->tCache);
	if ( bLayout ) {
		xgeXuiWidgetMarkLayout(pToggle->pWidget);
	}
	xgeXuiWidgetMarkPaint(pToggle->pWidget);
}

static void __xgeXuiToggleSetState(xge_xui_toggle pToggle, int iState)
{
	int iVisualState;

	if ( pToggle == NULL ) {
		return;
	}
	if ( (pToggle->pWidget == NULL) || ((pToggle->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pToggle->pContext != NULL && pToggle->pContext->pFocus == pToggle->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( __xgeXuiToggleIsChecked(pToggle) ) {
		iState |= XGE_XUI_STATE_CHECKED;
	}
	if ( pToggle->iState != iState ) {
		pToggle->iState = iState;
		xgeXuiWidgetMarkPaint(pToggle->pWidget);
	}
	if ( pToggle->pWidget != NULL ) {
		iVisualState = iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE | XGE_XUI_STATE_FOCUS | XGE_XUI_STATE_DISABLED | XGE_XUI_STATE_CHECKED);
		xgeXuiWidgetSetVisualState(pToggle->pWidget, iVisualState);
	}
}

static void __xgeXuiToggleSetCheckedInternal(xge_xui_toggle pToggle, int bChecked, int bNotify)
{
	int iState;

	if ( (pToggle == NULL) || (pToggle->pWidget == NULL) ) {
		return;
	}
	bChecked = bChecked ? 1 : 0;
	if ( __xgeXuiToggleIsChecked(pToggle) == bChecked ) {
		return;
	}
	iState = pToggle->pWidget->iVisualState;
	if ( bChecked ) {
		iState |= XGE_XUI_STATE_CHECKED;
	} else {
		iState &= ~XGE_XUI_STATE_CHECKED;
	}
	xgeXuiWidgetSetVisualState(pToggle->pWidget, iState);
	pToggle->iState = (pToggle->iState & ~XGE_XUI_STATE_CHECKED) | (bChecked ? XGE_XUI_STATE_CHECKED : 0);
	pToggle->iChangeCount++;
	__xgeXuiRenderCacheInvalidate(&pToggle->tCache);
	if ( bNotify && pToggle->procChange != NULL ) {
		pToggle->procChange(pToggle->pWidget, bChecked, pToggle->pUser);
	}
}

static xge_rect_t __xgeXuiToggleTextureSrc(xge_texture pTexture, xge_rect_t tSrc)
{
	return __xgeXuiChoiceTextureSrc(pTexture, tSrc);
}

static int __xgeXuiToggleHasTexture(xge_xui_toggle pToggle)
{
	if ( pToggle == NULL ) {
		return 0;
	}
	return (pToggle->pCheckedTexture != NULL || pToggle->pUncheckedTexture != NULL) ? 1 : 0;
}

static int __xgeXuiToggleHasInnerText(xge_xui_toggle pToggle)
{
	if ( (pToggle == NULL) || __xgeXuiToggleHasTexture(pToggle) ) {
		return 0;
	}
	return ((pToggle->sCheckedText != NULL && pToggle->sCheckedText[0] != 0) || (pToggle->sUncheckedText != NULL && pToggle->sUncheckedText[0] != 0)) ? 1 : 0;
}

static int __xgeXuiToggleIsDisabled(xge_xui_widget pWidget)
{
	return (pWidget == NULL || ((pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0)) ? 1 : 0;
}

static xge_rect_t __xgeXuiToggleTrackRect(xge_xui_widget pWidget, xge_xui_toggle pToggle, xge_rect_t tRect)
{
	xge_rect_t tTrack;
	xge_vec2_t tChecked;
	xge_vec2_t tUnchecked;
	float fTrackH;
	float fTrackW;
	float fKnobD;
	float fTextW;
	float fPadding;
	float fGap;

	memset(&tTrack, 0, sizeof(tTrack));
	if ( (pWidget == NULL) || (pToggle == NULL) ) {
		return tTrack;
	}
	if ( __xgeXuiToggleHasTexture(pToggle) ) {
		xge_texture pTexture;
		xge_rect_t tSrc;

		pTexture = xgeXuiToggleGetChecked(pToggle) ? pToggle->pCheckedTexture : pToggle->pUncheckedTexture;
		tSrc = xgeXuiToggleGetChecked(pToggle) ? pToggle->tCheckedSrc : pToggle->tUncheckedSrc;
		tSrc = __xgeXuiToggleTextureSrc(pTexture, tSrc);
		fTrackH = (pToggle->fTrackHeight > 0.0f) ? pToggle->fTrackHeight : ((tSrc.fH > 0.0f) ? tSrc.fH : 24.0f);
		fTrackW = (pToggle->fTrackWidth > 0.0f) ? pToggle->fTrackWidth : ((tSrc.fW > 0.0f) ? tSrc.fW : fTrackH * 1.85f);
	} else {
		fTrackH = (pToggle->fTrackHeight > 0.0f) ? pToggle->fTrackHeight : 24.0f;
		fTrackW = (pToggle->fTrackWidth > 0.0f) ? pToggle->fTrackWidth : 44.0f;
		if ( __xgeXuiToggleHasInnerText(pToggle) && pToggle->pInnerFont != NULL ) {
			tChecked = __xgeXuiHostMeasureText(pToggle->pInnerFont, (pToggle->sCheckedText != NULL) ? pToggle->sCheckedText : "");
			tUnchecked = __xgeXuiHostMeasureText(pToggle->pInnerFont, (pToggle->sUncheckedText != NULL) ? pToggle->sUncheckedText : "");
			fTextW = (tChecked.fX > tUnchecked.fX) ? tChecked.fX : tUnchecked.fX;
			fPadding = (pToggle->fTextPadding > 0.0f) ? pToggle->fTextPadding : 6.0f;
			fGap = (pToggle->fTextGap > 0.0f) ? pToggle->fTextGap : 2.0f;
			fKnobD = fTrackH - ((pToggle->fKnobInset > 0.0f) ? pToggle->fKnobInset : 3.0f) * 2.0f;
			if ( fKnobD < 8.0f ) {
				fKnobD = 8.0f;
			}
			if ( fTrackW < 54.0f ) {
				fTrackW = 54.0f;
			}
			if ( fTrackW < fKnobD + fTextW + fPadding * 2.0f + fGap ) {
				fTrackW = fKnobD + fTextW + fPadding * 2.0f + fGap;
			}
		}
	}
	if ( fTrackH > tRect.fH ) {
		fTrackH = tRect.fH;
	}
	if ( fTrackW > tRect.fW ) {
		fTrackW = tRect.fW;
	}
	tTrack.fX = tRect.fX;
	tTrack.fY = tRect.fY + (tRect.fH - fTrackH) * 0.5f;
	tTrack.fW = fTrackW;
	tTrack.fH = fTrackH;
	return tTrack;
}

static void __xgeXuiToggleDrawPill(xge_rect_t tTrack, uint32_t iColor, uint32_t iBorder)
{
	xge_rect_t tMid;
	float fRadius;
	float fCenterY;

	if ( (tTrack.fW <= 0.0f) || (tTrack.fH <= 0.0f) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(iBorder) != 0 ) {
		__xgeXuiToggleDrawPill(tTrack, iBorder, 0);
		tTrack.fX += 1.0f;
		tTrack.fY += 1.0f;
		tTrack.fW -= 2.0f;
		tTrack.fH -= 2.0f;
		if ( (tTrack.fW <= 0.0f) || (tTrack.fH <= 0.0f) ) {
			return;
		}
	}
	fRadius = tTrack.fH * 0.5f;
	fCenterY = tTrack.fY + fRadius;
	tMid.fX = tTrack.fX + fRadius;
	tMid.fY = tTrack.fY;
	tMid.fW = tTrack.fW - fRadius * 2.0f;
	tMid.fH = tTrack.fH;
	if ( tMid.fW > 0.0f ) {
		__xgeXuiHostDrawRect(tMid, iColor);
	}
	xgeShapeCircleFillPx(tTrack.fX + fRadius, fCenterY, fRadius, iColor);
	xgeShapeCircleFillPx(tTrack.fX + tTrack.fW - fRadius, fCenterY, fRadius, iColor);
}

static void __xgeXuiToggleDrawDirect(xge_xui_widget pWidget, xge_xui_toggle pToggle, xge_rect_t tRect)
{
	xge_rect_t tTrack;
	xge_rect_t tText;
	xge_texture pTexture;
	xge_rect_t tSrc;
	float fKnobInset;
	float fKnobR;
	float fKnobX;
	float fCenterY;
	float fTextGap;
	float fTextPadding;
	uint32_t iTrackColor;
	uint32_t iBorder;
	uint32_t iKnobColor;
	uint32_t iTextColor;
	int bChecked;
	int bDisabled;

	if ( (pWidget == NULL) || (pToggle == NULL) ) {
		return;
	}
	bChecked = xgeXuiToggleGetChecked(pToggle);
	bDisabled = __xgeXuiToggleIsDisabled(pWidget);
	tTrack = __xgeXuiToggleTrackRect(pWidget, pToggle, tRect);
	if ( (tTrack.fW <= 0.0f) || (tTrack.fH <= 0.0f) ) {
		return;
	}
	if ( __xgeXuiToggleHasTexture(pToggle) ) {
		pTexture = bChecked ? pToggle->pCheckedTexture : pToggle->pUncheckedTexture;
		tSrc = bChecked ? pToggle->tCheckedSrc : pToggle->tUncheckedSrc;
		__xgeXuiChoiceDrawTexture(pTexture, tSrc, tTrack);
		return;
	}
	if ( bDisabled ) {
		iTrackColor = bChecked ? XGE_COLOR_RGBA(204, 212, 220, 255) : XGE_COLOR_RGBA(246, 248, 250, 255);
		iKnobColor = XGE_COLOR_RGBA(218, 222, 226, 255);
		iBorder = iKnobColor;
		iTextColor = XGE_COLOR_RGBA(142, 152, 162, 255);
	} else if ( bChecked ) {
		iTrackColor = pToggle->iColorChecked;
		iBorder = 0;
		iKnobColor = pToggle->iColorKnob;
		iTextColor = pToggle->iColorCheckedText;
	} else {
		iTrackColor = pToggle->iColorTrack;
		iKnobColor = XGE_COLOR_RGBA(214, 220, 226, 255);
		iBorder = (XGE_COLOR_GET_A(pToggle->iColorTrackBorder) != 0) ? pToggle->iColorTrackBorder : iKnobColor;
		iTextColor = pToggle->iColorUncheckedText;
	}
	__xgeXuiToggleDrawPill(tTrack, iTrackColor, iBorder);
	fKnobInset = (pToggle->fKnobInset > 0.0f) ? pToggle->fKnobInset : 3.0f;
	fKnobR = (tTrack.fH - fKnobInset * 2.0f) * 0.5f;
	fKnobR -= 1.0f;
	if ( fKnobR < 4.0f ) {
		fKnobR = 4.0f;
	}
	fCenterY = tTrack.fY + tTrack.fH * 0.5f;
	fKnobX = bChecked ? (tTrack.fX + tTrack.fW - fKnobInset - fKnobR) : (tTrack.fX + fKnobInset + fKnobR);
	if ( __xgeXuiToggleHasInnerText(pToggle) && pToggle->pInnerFont != NULL ) {
		fTextPadding = (pToggle->fTextPadding > 0.0f) ? pToggle->fTextPadding : 6.0f;
		fTextGap = (pToggle->fTextGap > 0.0f) ? pToggle->fTextGap : 2.0f;
		tText = tTrack;
		if ( bChecked ) {
			tText.fX += fTextPadding;
			tText.fW = fKnobX - fKnobR - fTextGap - tText.fX;
			if ( tText.fW > 0.0f ) {
				__xgeXuiHostDrawTextRect(pToggle->pInnerFont, (pToggle->sCheckedText != NULL) ? pToggle->sCheckedText : "", tText, iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			}
		} else {
			tText.fX = fKnobX + fKnobR + fTextGap;
			tText.fW = tTrack.fX + tTrack.fW - fTextPadding - tText.fX;
			if ( tText.fW > 0.0f ) {
				__xgeXuiHostDrawTextRect(pToggle->pInnerFont, (pToggle->sUncheckedText != NULL) ? pToggle->sUncheckedText : "", tText, iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			}
		}
	}
	xgeShapeCircleFillPx(fKnobX, fCenterY, fKnobR, iKnobColor);
}

static void __xgeXuiTogglePaintCacheContent(xge_rect_t tRect, void* pUser)
{
	xge_xui_choice_cache_paint_t* pPaint;

	pPaint = (xge_xui_choice_cache_paint_t*)pUser;
	if ( pPaint == NULL ) {
		return;
	}
	__xgeXuiToggleDrawDirect(pPaint->pWidget, (xge_xui_toggle)pPaint->pControl, tRect);
}

static int __xgeXuiTogglePaintCache(xge_xui_widget pWidget, xge_xui_toggle pToggle)
{
	xge_xui_choice_cache_paint_t tPaint;
	xge_texture pTexture;
	xge_draw_t tDraw;
	xge_rect_t tContent;
	float fDipScale;
	int iState;

	if ( (pWidget == NULL) || (pToggle == NULL) || (pToggle->iCacheMode == XGE_XUI_CACHE_OFF) ) {
		return 0;
	}
	tContent = pWidget->tContentRect;
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		return 0;
	}
	fDipScale = (pToggle->pContext != NULL && pToggle->pContext->fDipScale > 0.0f) ? pToggle->pContext->fDipScale : 1.0f;
	iState = pWidget->iVisualState | (xgeXuiWidgetIsEnabled(pWidget) ? 0 : XGE_XUI_STATE_DISABLED);
	if ( pToggle->iCacheState != iState ) {
		pToggle->iCacheState = iState;
		__xgeXuiRenderCacheInvalidate(&pToggle->tCache);
	}
	memset(&tPaint, 0, sizeof(tPaint));
	tPaint.pWidget = pWidget;
	tPaint.pControl = pToggle;
	pTexture = __xgeXuiRenderCacheEnsure(&pToggle->tCache, tContent, fDipScale, __xgeXuiTogglePaintCacheContent, &tPaint);
	if ( pTexture == NULL ) {
		return 0;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tDst = tContent;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE | XGE_DRAW_FLIP_Y;
	__xgeXuiHostDrawImage(&tDraw);
	return 1;
}

int xgeXuiToggleInit(xge_xui_toggle pToggle, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pToggle == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pToggle, 0, sizeof(*pToggle));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pToggle->pContext = pContext;
	pToggle->pWidget = pWidget;
	pToggle->pInnerFont = pTheme->pFont;
	pToggle->iColorTrack = XGE_COLOR_RGBA(246, 249, 252, 255);
	pToggle->iColorChecked = XGE_COLOR_RGBA(46, 132, 218, 255);
	pToggle->iColorKnob = XGE_COLOR_RGBA(255, 255, 255, 255);
	pToggle->iColorTrackBorder = XGE_COLOR_RGBA(214, 220, 226, 255);
	pToggle->iColorCheckedText = XGE_COLOR_RGBA(255, 255, 255, 255);
	pToggle->iColorUncheckedText = XGE_COLOR_RGBA(112, 126, 140, 255);
	pToggle->fTrackHeight = 24.0f;
	pToggle->fTrackWidth = 0.0f;
	pToggle->fKnobInset = 3.0f;
	pToggle->fTextPadding = 6.0f;
	pToggle->fTextGap = 2.0f;
	pToggle->iCacheMode = XGE_XUI_CACHE_AUTO;
	pToggle->iCacheState = -1;
	__xgeXuiRenderCacheInit(&pToggle->tCache);
	xgeXuiWidgetSetBackground(pWidget, 0);
	xgeXuiWidgetSetBorder(pWidget, 0.0f, 0);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiToggleEventProc, NULL);
	pWidget->procPaint = xgeXuiTogglePaintProc;
	pWidget->pUser = pToggle;
	__xgeXuiToggleSetState(pToggle, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiToggleUnit(xge_xui_toggle pToggle)
{
	if ( pToggle == NULL ) {
		return;
	}
	__xgeXuiRenderCacheUnit(&pToggle->tCache);
	if ( pToggle->pWidget != NULL && pToggle->pWidget->pUser == pToggle ) {
		pToggle->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pToggle->pWidget, NULL, NULL);
		pToggle->pWidget->procPaint = NULL;
	}
	memset(pToggle, 0, sizeof(*pToggle));
}

void xgeXuiToggleSetChange(xge_xui_toggle pToggle, xge_xui_checked_proc procChange, void* pUser)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->procChange = procChange;
	pToggle->pUser = pUser;
}

void xgeXuiToggleSetChecked(xge_xui_toggle pToggle, int bChecked)
{
	__xgeXuiToggleSetCheckedInternal(pToggle, bChecked, 0);
}

int xgeXuiToggleGetChecked(xge_xui_toggle pToggle)
{
	return __xgeXuiToggleIsChecked(pToggle);
}

void xgeXuiToggleSetColors(xge_xui_toggle pToggle, uint32_t iTrack, uint32_t iChecked, uint32_t iKnob, uint32_t iTrackBorder)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->iColorTrack = iTrack;
	pToggle->iColorChecked = iChecked;
	pToggle->iColorKnob = iKnob;
	pToggle->iColorTrackBorder = iTrackBorder;
	__xgeXuiToggleCacheInvalidate(pToggle, 0);
}

void xgeXuiToggleSetInnerText(xge_xui_toggle pToggle, xge_font pFont, const char* sUncheckedText, const char* sCheckedText)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->pInnerFont = pFont;
	pToggle->sUncheckedText = sUncheckedText;
	pToggle->sCheckedText = sCheckedText;
	__xgeXuiToggleCacheInvalidate(pToggle, 1);
}

void xgeXuiToggleSetInnerTextColor(xge_xui_toggle pToggle, uint32_t iUncheckedColor, uint32_t iCheckedColor)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->iColorUncheckedText = iUncheckedColor;
	pToggle->iColorCheckedText = iCheckedColor;
	__xgeXuiToggleCacheInvalidate(pToggle, 0);
}

void xgeXuiToggleSetTextures(xge_xui_toggle pToggle, xge_texture pUncheckedTexture, xge_rect_t tUncheckedSrc, xge_texture pCheckedTexture, xge_rect_t tCheckedSrc)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->pUncheckedTexture = pUncheckedTexture;
	pToggle->pCheckedTexture = pCheckedTexture;
	pToggle->tUncheckedSrc = tUncheckedSrc;
	pToggle->tCheckedSrc = tCheckedSrc;
	__xgeXuiToggleCacheInvalidate(pToggle, 1);
}

void xgeXuiToggleSetMetrics(xge_xui_toggle pToggle, float fTrackWidth, float fTrackHeight, float fKnobInset, float fTextPadding, float fTextGap)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->fTrackWidth = (fTrackWidth > 0.0f) ? fTrackWidth : 0.0f;
	pToggle->fTrackHeight = (fTrackHeight > 0.0f) ? fTrackHeight : 24.0f;
	pToggle->fKnobInset = (fKnobInset > 0.0f) ? fKnobInset : 2.0f;
	pToggle->fTextPadding = (fTextPadding > 0.0f) ? fTextPadding : 7.0f;
	pToggle->fTextGap = (fTextGap > 0.0f) ? fTextGap : 5.0f;
	__xgeXuiToggleCacheInvalidate(pToggle, 1);
}

void xgeXuiToggleSetCacheMode(xge_xui_toggle pToggle, int iMode)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->iCacheMode = __xgeXuiChoiceCacheModeNormalize(iMode);
	__xgeXuiToggleCacheInvalidate(pToggle, 0);
}

int xgeXuiToggleGetState(xge_xui_toggle pToggle)
{
	if ( pToggle == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiToggleSetState(pToggle, pToggle->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pToggle->iState;
}

int xgeXuiToggleEvent(xge_xui_toggle pToggle, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;

	if ( (pToggle == NULL) || (pToggle->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pToggle->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pToggle->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiToggleSetState(pToggle, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pToggle->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pToggle->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			__xgeXuiToggleSetState(pToggle, iInside ? (iState | XGE_XUI_STATE_HOVER) : (iState & ~XGE_XUI_STATE_HOVER));
			return ((pToggle->iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiToggleSetState(pToggle, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiToggleSetState(pToggle, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiToggleSetState(pToggle, iState);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pToggle->pContext, pToggle->pWidget);
			xgeXuiSetPointerCapture(pToggle->pContext, pEvent->iPointerId, pToggle->pWidget);
			__xgeXuiToggleSetState(pToggle, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pToggle->iState & XGE_XUI_STATE_ACTIVE) != 0);
			__xgeXuiToggleSetState(pToggle, iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			if ( pToggle->pContext != NULL && xgeXuiGetPointerCapture(pToggle->pContext, pEvent->iPointerId) == pToggle->pWidget ) {
				xgeXuiSetPointerCapture(pToggle->pContext, pEvent->iPointerId, NULL);
			}
			if ( bWasActive && iInside ) {
				__xgeXuiToggleSetCheckedInternal(pToggle, xgeXuiToggleGetChecked(pToggle) ? 0 : 1, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			__xgeXuiToggleSetState(pToggle, XGE_XUI_STATE_NORMAL);
			if ( pToggle->pContext != NULL && xgeXuiGetPointerCapture(pToggle->pContext, pEvent->iPointerId) == pToggle->pWidget ) {
				xgeXuiSetPointerCapture(pToggle->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( (pToggle->pContext == NULL) || (pToggle->pContext->pFocus != pToggle->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 != XGE_KEY_ENTER) && (pEvent->iParam1 != XGE_KEY_SPACE) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiToggleSetCheckedInternal(pToggle, xgeXuiToggleGetChecked(pToggle) ? 0 : 1, 1);
			return XGE_XUI_EVENT_CONSUMED;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiToggleEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiToggleEvent((xge_xui_toggle)pUser, pEvent);
}

void xgeXuiTogglePaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_toggle pToggle;

	pToggle = (xge_xui_toggle)pUser;
	if ( (pWidget == NULL) || (pToggle == NULL) ) {
		return;
	}
	if ( __xgeXuiTogglePaintCache(pWidget, pToggle) ) {
		return;
	}
	__xgeXuiToggleDrawDirect(pWidget, pToggle, pWidget->tContentRect);
}
