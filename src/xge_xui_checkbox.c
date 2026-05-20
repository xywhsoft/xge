typedef struct xge_xui_choice_cache_paint_t {
	xge_xui_widget pWidget;
	void* pControl;
	int iKind;
} xge_xui_choice_cache_paint_t;

static int __xgeXuiChoiceCacheModeNormalize(int iMode)
{
	if ( (iMode < XGE_XUI_CACHE_AUTO) || (iMode > XGE_XUI_CACHE_FORCE) ) {
		return XGE_XUI_CACHE_AUTO;
	}
	return iMode;
}

static int __xgeXuiChoiceIsChecked(xge_xui_widget pWidget)
{
	return (pWidget != NULL && (pWidget->iVisualState & XGE_XUI_STATE_CHECKED) != 0) ? 1 : 0;
}

static xge_rect_t __xgeXuiChoiceTextureSrc(xge_texture pTexture, xge_rect_t tSrc)
{
	if ( pTexture == NULL ) {
		return tSrc;
	}
	if ( tSrc.fW <= 0.0f ) {
		tSrc.fW = (float)pTexture->iWidth;
	}
	if ( tSrc.fH <= 0.0f ) {
		tSrc.fH = (float)pTexture->iHeight;
	}
	return tSrc;
}

static void __xgeXuiChoiceDrawTexture(xge_texture pTexture, xge_rect_t tSrc, xge_rect_t tDst, int bRenderCache)
{
	xge_draw_t tDraw;

	if ( (pTexture == NULL) || (tDst.fW <= 0.0f) || (tDst.fH <= 0.0f) ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tSrc = __xgeXuiChoiceTextureSrc(pTexture, tSrc);
	tDraw.tDst = tDst;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	if ( bRenderCache ) {
		xgeDrawEx(&tDraw);
	} else {
		__xgeXuiHostDrawImage(&tDraw);
	}
}

static void __xgeXuiChoicePutPixel(unsigned char* pPixels, int iW, int x, int y, uint32_t iColor, float fCoverage)
{
	int iOffset;
	int iA;

	if ( (pPixels == NULL) || (x < 0) || (y < 0) || (fCoverage <= 0.0f) ) {
		return;
	}
	if ( fCoverage > 1.0f ) {
		fCoverage = 1.0f;
	}
	iOffset = (y * iW + x) * 4;
	iA = (int)((float)XGE_COLOR_GET_A(iColor) * fCoverage + 0.5f);
	if ( iA < 0 ) {
		iA = 0;
	} else if ( iA > 255 ) {
		iA = 255;
	}
	pPixels[iOffset + 0] = (unsigned char)XGE_COLOR_GET_R(iColor);
	pPixels[iOffset + 1] = (unsigned char)XGE_COLOR_GET_G(iColor);
	pPixels[iOffset + 2] = (unsigned char)XGE_COLOR_GET_B(iColor);
	pPixels[iOffset + 3] = (unsigned char)iA;
}

static float __xgeXuiChoiceSmoothCoverage(float fDistance, float fHalfWidth)
{
	float fCoverage;

	fCoverage = fHalfWidth + 0.5f - fabsf(fDistance);
	if ( fCoverage < 0.0f ) {
		return 0.0f;
	}
	if ( fCoverage > 1.0f ) {
		return 1.0f;
	}
	return fCoverage;
}

static float __xgeXuiChoiceSegmentDistance(float fX, float fY, float fX0, float fY0, float fX1, float fY1)
{
	float fDX;
	float fDY;
	float fT;
	float fPX;
	float fPY;
	float fLen2;

	fDX = fX1 - fX0;
	fDY = fY1 - fY0;
	fLen2 = fDX * fDX + fDY * fDY;
	if ( fLen2 <= 0.0001f ) {
		fPX = fX - fX0;
		fPY = fY - fY0;
		return sqrtf(fPX * fPX + fPY * fPY);
	}
	fT = ((fX - fX0) * fDX + (fY - fY0) * fDY) / fLen2;
	if ( fT < 0.0f ) {
		fT = 0.0f;
	} else if ( fT > 1.0f ) {
		fT = 1.0f;
	}
	fPX = fX - (fX0 + fDX * fT);
	fPY = fY - (fY0 + fDY * fT);
	return sqrtf(fPX * fPX + fPY * fPY);
}

static xge_texture __xgeXuiChoiceDefaultCheckTexture(int bChecked, uint32_t iBox, uint32_t iChecked)
{
	static xge_texture_t tUnchecked;
	static xge_texture_t tChecked;
	static uint32_t iUncheckedBox = 0;
	static uint32_t iCheckedBox = 0;
	static uint32_t iCheckedMark = 0;
	static int bUncheckedReady = 0;
	static int bCheckedReady = 0;
	xge_texture pTexture;
	unsigned char arrPixels[32 * 32 * 4];
	float fX;
	float fY;
	float fDistEdge;
	float fDistCheck;
	float fCoverage;
	int x;
	int y;

	if ( (!bChecked && bUncheckedReady && iUncheckedBox == iBox) || (bChecked && bCheckedReady && iCheckedBox == iBox && iCheckedMark == iChecked) ) {
		return bChecked ? &tChecked : &tUnchecked;
	}
	memset(arrPixels, 0, sizeof(arrPixels));
	for ( y = 0; y < 32; y++ ) {
		for ( x = 0; x < 32; x++ ) {
			fX = (float)x + 0.5f;
			fY = (float)y + 0.5f;
			if ( (fX >= 3.0f) && (fX <= 29.0f) && (fY >= 3.0f) && (fY <= 29.0f) ) {
				__xgeXuiChoicePutPixel(arrPixels, 32, x, y, XGE_COLOR_RGBA(255, 255, 255, 255), 1.0f);
				fDistEdge = fX - 3.0f;
				if ( 29.0f - fX < fDistEdge ) {
					fDistEdge = 29.0f - fX;
				}
				if ( fY - 3.0f < fDistEdge ) {
					fDistEdge = fY - 3.0f;
				}
				if ( 29.0f - fY < fDistEdge ) {
					fDistEdge = 29.0f - fY;
				}
				fCoverage = __xgeXuiChoiceSmoothCoverage(fDistEdge - 0.65f, 0.65f);
				if ( fCoverage > 0.0f ) {
					__xgeXuiChoicePutPixel(arrPixels, 32, x, y, iBox, fCoverage);
				}
			}
			if ( bChecked ) {
				fDistCheck = __xgeXuiChoiceSegmentDistance(fX, fY, 8.2f, 16.4f, 13.1f, 21.0f);
				fCoverage = __xgeXuiChoiceSmoothCoverage(fDistCheck, 2.0f);
				if ( fCoverage > 0.0f ) {
					__xgeXuiChoicePutPixel(arrPixels, 32, x, y, iChecked, fCoverage);
				}
				fDistCheck = __xgeXuiChoiceSegmentDistance(fX, fY, 13.0f, 21.0f, 24.2f, 9.4f);
				fCoverage = __xgeXuiChoiceSmoothCoverage(fDistCheck, 2.0f);
				if ( fCoverage > 0.0f ) {
					__xgeXuiChoicePutPixel(arrPixels, 32, x, y, iChecked, fCoverage);
				}
			}
		}
	}
	pTexture = bChecked ? &tChecked : &tUnchecked;
	xgeTextureFree(pTexture);
	if ( xgeTextureCreateRGBA(pTexture, 32, 32, arrPixels) != XGE_OK ) {
		return NULL;
	}
	if ( bChecked ) {
		bCheckedReady = 1;
		iCheckedBox = iBox;
		iCheckedMark = iChecked;
	} else {
		bUncheckedReady = 1;
		iUncheckedBox = iBox;
	}
	return pTexture;
}

static float __xgeXuiCheckBoxIndicatorSize(xge_xui_checkbox pCheckBox)
{
	xge_texture pTexture;
	xge_rect_t tSrc;

	if ( pCheckBox == NULL ) {
		return 17.0f;
	}
	if ( pCheckBox->fIndicatorSize > 0.0f ) {
		return pCheckBox->fIndicatorSize;
	}
	pTexture = xgeXuiCheckBoxGetChecked(pCheckBox) ? pCheckBox->pCheckedTexture : pCheckBox->pUncheckedTexture;
	tSrc = xgeXuiCheckBoxGetChecked(pCheckBox) ? pCheckBox->tCheckedSrc : pCheckBox->tUncheckedSrc;
	tSrc = __xgeXuiChoiceTextureSrc(pTexture, tSrc);
	if ( tSrc.fH > 0.0f ) {
		return tSrc.fH;
	}
	return 17.0f;
}

static void __xgeXuiCheckBoxCacheInvalidate(xge_xui_checkbox pCheckBox, int bLayout)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	__xgeXuiRenderCacheInvalidate(&pCheckBox->tCache);
	if ( bLayout ) {
		xgeXuiWidgetMarkLayout(pCheckBox->pWidget);
	}
	xgeXuiWidgetMarkPaint(pCheckBox->pWidget);
}

static void __xgeXuiCheckBoxDrawDirect(xge_xui_widget pWidget, xge_xui_checkbox pCheckBox, xge_rect_t tRect, int bRenderCache)
{
	xge_rect_t tBox;
	xge_rect_t tText;
	xge_texture pTexture;
	xge_rect_t tSrc;
	xge_vec2_t tTextSize;
	float fSize;
	float fGap;
	float fLineH;
	int bChecked;

	(void)pWidget;
	if ( pCheckBox == NULL ) {
		return;
	}
	bChecked = xgeXuiCheckBoxGetChecked(pCheckBox);
	fSize = __xgeXuiCheckBoxIndicatorSize(pCheckBox);
	if ( fSize < 1.0f ) {
		fSize = 1.0f;
	}
	if ( fSize > tRect.fH ) {
		fSize = tRect.fH;
	}
	fGap = (pCheckBox->fGap >= 0.0f) ? pCheckBox->fGap : 6.0f;
	tTextSize.fX = 0.0f;
	tTextSize.fY = 0.0f;
	if ( (pCheckBox->pFont != NULL) && (pCheckBox->sText != NULL) && (pCheckBox->sText[0] != 0) ) {
		tTextSize = __xgeXuiHostMeasureText(pCheckBox->pFont, pCheckBox->sText);
	}
	fLineH = (tTextSize.fY > fSize) ? tTextSize.fY : fSize;
	if ( fLineH > tRect.fH ) {
		fLineH = tRect.fH;
	}
	tBox.fX = tRect.fX;
	tBox.fY = tRect.fY + (tRect.fH - fLineH) * 0.5f + (fLineH - fSize) * 0.5f;
	tBox.fW = fSize;
	tBox.fH = fSize;
	pTexture = bChecked ? pCheckBox->pCheckedTexture : pCheckBox->pUncheckedTexture;
	tSrc = bChecked ? pCheckBox->tCheckedSrc : pCheckBox->tUncheckedSrc;
	if ( pTexture != NULL ) {
		__xgeXuiChoiceDrawTexture(pTexture, tSrc, tBox, bRenderCache);
	} else {
		pTexture = __xgeXuiChoiceDefaultCheckTexture(bChecked, pCheckBox->iColorBox, pCheckBox->iColorChecked);
		if ( pTexture != NULL ) {
			__xgeXuiChoiceDrawTexture(pTexture, (xge_rect_t){ 0.0f, 0.0f, 32.0f, 32.0f }, tBox, bRenderCache);
		}
	}
	if ( (pCheckBox->pFont != NULL) && (pCheckBox->sText != NULL) && (pCheckBox->sText[0] != 0) ) {
		tText = tRect;
		tText.fX += fSize + fGap;
		tText.fW -= fSize + fGap;
		if ( tText.fW > 0.0f ) {
			if ( bRenderCache ) {
				xgeTextDrawRect(pCheckBox->pFont, pCheckBox->sText, tText, pCheckBox->iTextColor, pCheckBox->iTextFlags);
			} else {
				__xgeXuiHostDrawTextRect(pCheckBox->pFont, pCheckBox->sText, tText, pCheckBox->iTextColor, pCheckBox->iTextFlags);
			}
		}
	}
}

static void __xgeXuiCheckBoxPaintCacheContent(xge_rect_t tRect, void* pUser)
{
	xge_xui_choice_cache_paint_t* pPaint;

	pPaint = (xge_xui_choice_cache_paint_t*)pUser;
	if ( pPaint == NULL ) {
		return;
	}
	__xgeXuiCheckBoxDrawDirect(pPaint->pWidget, (xge_xui_checkbox)pPaint->pControl, tRect, 1);
}

static int __xgeXuiCheckBoxPaintCache(xge_xui_widget pWidget, xge_xui_checkbox pCheckBox)
{
	xge_xui_choice_cache_paint_t tPaint;
	xge_texture pTexture;
	xge_draw_t tDraw;
	xge_rect_t tContent;
	float fDipScale;
	int iState;

	if ( (pWidget == NULL) || (pCheckBox == NULL) || (pCheckBox->iCacheMode == XGE_XUI_CACHE_OFF) ) {
		return 0;
	}
	tContent = pWidget->tContentRect;
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		return 0;
	}
	fDipScale = (pCheckBox->pContext != NULL && pCheckBox->pContext->fDipScale > 0.0f) ? pCheckBox->pContext->fDipScale : 1.0f;
	iState = pWidget->iVisualState | (xgeXuiWidgetIsEnabled(pWidget) ? 0 : XGE_XUI_STATE_DISABLED);
	if ( pCheckBox->iCacheState != iState ) {
		pCheckBox->iCacheState = iState;
		__xgeXuiRenderCacheInvalidate(&pCheckBox->tCache);
	}
	memset(&tPaint, 0, sizeof(tPaint));
	tPaint.pWidget = pWidget;
	tPaint.pControl = pCheckBox;
	pTexture = __xgeXuiRenderCacheEnsure(&pCheckBox->tCache, tContent, fDipScale, __xgeXuiCheckBoxPaintCacheContent, &tPaint);
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

int xgeXuiCheckBoxInit(xge_xui_checkbox pCheckBox, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pCheckBox == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pCheckBox, 0, sizeof(*pCheckBox));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pCheckBox->pContext = pContext;
	pCheckBox->pWidget = pWidget;
	pCheckBox->pFont = pTheme->pFont;
	pCheckBox->sText = "";
	pCheckBox->iTextColor = pTheme->iTextColor;
	pCheckBox->iTextFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pCheckBox->iColorBox = pTheme->iBorderColor;
	pCheckBox->iColorChecked = pTheme->iAccentColor;
	pCheckBox->fIndicatorSize = 17.0f;
	pCheckBox->fGap = 6.0f;
	pCheckBox->iCacheMode = XGE_XUI_CACHE_AUTO;
	pCheckBox->iCacheState = -1;
	__xgeXuiRenderCacheInit(&pCheckBox->tCache);
	xgeXuiWidgetSetBackground(pWidget, 0);
	xgeXuiWidgetSetBorder(pWidget, 0.0f, 0);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiCheckBoxEventProc, NULL);
	pWidget->procPaint = xgeXuiCheckBoxPaintProc;
	pWidget->pUser = pCheckBox;
	__xgeXuiCheckBoxSetState(pCheckBox, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiCheckBoxUnit(xge_xui_checkbox pCheckBox)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	__xgeXuiRenderCacheUnit(&pCheckBox->tCache);
	if ( pCheckBox->pWidget != NULL && pCheckBox->pWidget->pUser == pCheckBox ) {
		pCheckBox->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pCheckBox->pWidget, NULL, NULL);
		pCheckBox->pWidget->procPaint = NULL;
	}
	memset(pCheckBox, 0, sizeof(*pCheckBox));
}

void xgeXuiCheckBoxSetChange(xge_xui_checkbox pCheckBox, xge_xui_checked_proc procChange, void* pUser)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	pCheckBox->procChange = procChange;
	pCheckBox->pUser = pUser;
}

void xgeXuiCheckBoxSetText(xge_xui_checkbox pCheckBox, xge_font pFont, const char* sText)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	pCheckBox->pFont = pFont;
	pCheckBox->sText = (sText != NULL) ? sText : "";
	__xgeXuiCheckBoxCacheInvalidate(pCheckBox, 1);
}

void xgeXuiCheckBoxSetChecked(xge_xui_checkbox pCheckBox, int bChecked)
{
	int iState;

	if ( (pCheckBox == NULL) || (pCheckBox->pWidget == NULL) ) {
		return;
	}
	iState = pCheckBox->pWidget->iVisualState;
	if ( bChecked ) {
		iState |= XGE_XUI_STATE_CHECKED;
	} else {
		iState &= ~XGE_XUI_STATE_CHECKED;
	}
	if ( iState != pCheckBox->pWidget->iVisualState ) {
		xgeXuiWidgetSetVisualState(pCheckBox->pWidget, iState);
		pCheckBox->iState = iState;
		__xgeXuiRenderCacheInvalidate(&pCheckBox->tCache);
	}
}

int xgeXuiCheckBoxGetChecked(xge_xui_checkbox pCheckBox)
{
	if ( (pCheckBox == NULL) || (pCheckBox->pWidget == NULL) ) {
		return 0;
	}
	return __xgeXuiChoiceIsChecked(pCheckBox->pWidget);
}

void xgeXuiCheckBoxSetTextColor(xge_xui_checkbox pCheckBox, uint32_t iColor)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	pCheckBox->iTextColor = iColor;
	__xgeXuiCheckBoxCacheInvalidate(pCheckBox, 0);
}

void xgeXuiCheckBoxSetColors(xge_xui_checkbox pCheckBox, uint32_t iBox, uint32_t iChecked)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	pCheckBox->iColorBox = iBox;
	pCheckBox->iColorChecked = iChecked;
	__xgeXuiCheckBoxCacheInvalidate(pCheckBox, 0);
}

void xgeXuiCheckBoxSetTextures(xge_xui_checkbox pCheckBox, xge_texture pUncheckedTexture, xge_rect_t tUncheckedSrc, xge_texture pCheckedTexture, xge_rect_t tCheckedSrc)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	pCheckBox->pUncheckedTexture = pUncheckedTexture;
	pCheckBox->pCheckedTexture = pCheckedTexture;
	pCheckBox->tUncheckedSrc = tUncheckedSrc;
	pCheckBox->tCheckedSrc = tCheckedSrc;
	__xgeXuiCheckBoxCacheInvalidate(pCheckBox, 1);
}

void xgeXuiCheckBoxSetIndicatorSize(xge_xui_checkbox pCheckBox, float fSize)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	pCheckBox->fIndicatorSize = (fSize > 0.0f) ? fSize : 0.0f;
	__xgeXuiCheckBoxCacheInvalidate(pCheckBox, 1);
}

void xgeXuiCheckBoxSetGap(xge_xui_checkbox pCheckBox, float fGap)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	pCheckBox->fGap = (fGap >= 0.0f) ? fGap : 6.0f;
	__xgeXuiCheckBoxCacheInvalidate(pCheckBox, 1);
}

void xgeXuiCheckBoxSetCacheMode(xge_xui_checkbox pCheckBox, int iMode)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	pCheckBox->iCacheMode = __xgeXuiChoiceCacheModeNormalize(iMode);
	__xgeXuiCheckBoxCacheInvalidate(pCheckBox, 0);
}

int xgeXuiCheckBoxGetState(xge_xui_checkbox pCheckBox)
{
	if ( pCheckBox == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiCheckBoxSetState(pCheckBox, pCheckBox->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pCheckBox->iState;
}

static void __xgeXuiCheckBoxToggle(xge_xui_checkbox pCheckBox)
{
	int bChecked;

	if ( pCheckBox == NULL ) {
		return;
	}
	bChecked = xgeXuiCheckBoxGetChecked(pCheckBox) ? 0 : 1;
	xgeXuiCheckBoxSetChecked(pCheckBox, bChecked);
	pCheckBox->iChangeCount++;
	if ( pCheckBox->procChange != NULL ) {
		pCheckBox->procChange(pCheckBox->pWidget, bChecked, pCheckBox->pUser);
	}
}

int xgeXuiCheckBoxEvent(xge_xui_checkbox pCheckBox, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;

	if ( (pCheckBox == NULL) || (pCheckBox->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pCheckBox->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pCheckBox->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiCheckBoxSetState(pCheckBox, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pCheckBox->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pCheckBox->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			__xgeXuiCheckBoxSetState(pCheckBox, iInside ? (iState | XGE_XUI_STATE_HOVER) : (iState & ~XGE_XUI_STATE_HOVER));
			return ((pCheckBox->iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiCheckBoxSetState(pCheckBox, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiCheckBoxSetState(pCheckBox, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiCheckBoxSetState(pCheckBox, iState);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pCheckBox->pContext, pCheckBox->pWidget);
			xgeXuiSetPointerCapture(pCheckBox->pContext, pEvent->iPointerId, pCheckBox->pWidget);
			__xgeXuiCheckBoxSetState(pCheckBox, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pCheckBox->iState & XGE_XUI_STATE_ACTIVE) != 0);
			__xgeXuiCheckBoxSetState(pCheckBox, iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			if ( pCheckBox->pContext != NULL && xgeXuiGetPointerCapture(pCheckBox->pContext, pEvent->iPointerId) == pCheckBox->pWidget ) {
				xgeXuiSetPointerCapture(pCheckBox->pContext, pEvent->iPointerId, NULL);
			}
			if ( bWasActive && iInside ) {
				__xgeXuiCheckBoxToggle(pCheckBox);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			__xgeXuiCheckBoxSetState(pCheckBox, XGE_XUI_STATE_NORMAL);
			if ( pCheckBox->pContext != NULL && xgeXuiGetPointerCapture(pCheckBox->pContext, pEvent->iPointerId) == pCheckBox->pWidget ) {
				xgeXuiSetPointerCapture(pCheckBox->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( (pCheckBox->pContext == NULL) || (pCheckBox->pContext->pFocus != pCheckBox->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 != XGE_KEY_ENTER) && (pEvent->iParam1 != XGE_KEY_SPACE) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiCheckBoxToggle(pCheckBox);
			return XGE_XUI_EVENT_CONSUMED;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiCheckBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiCheckBoxEvent((xge_xui_checkbox)pUser, pEvent);
}

void xgeXuiCheckBoxPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_checkbox pCheckBox;

	pCheckBox = (xge_xui_checkbox)pUser;
	if ( (pWidget == NULL) || (pCheckBox == NULL) ) {
		return;
	}
	if ( __xgeXuiCheckBoxPaintCache(pWidget, pCheckBox) ) {
		return;
	}
	__xgeXuiCheckBoxDrawDirect(pWidget, pCheckBox, pWidget->tContentRect, 0);
}
