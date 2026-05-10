static int __xgeXuiRadioIsChecked(xge_xui_radio pRadio)
{
	return (pRadio != NULL && pRadio->pWidget != NULL && (pRadio->pWidget->iVisualState & XGE_XUI_STATE_CHECKED) != 0) ? 1 : 0;
}

static void __xgeXuiRadioSetState(xge_xui_radio pRadio, int iState)
{
	int iVisualState;

	if ( pRadio == NULL ) {
		return;
	}
	if ( (pRadio->pWidget == NULL) || ((pRadio->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pRadio->pContext != NULL && pRadio->pContext->pFocus == pRadio->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( __xgeXuiRadioIsChecked(pRadio) ) {
		iState |= XGE_XUI_STATE_CHECKED;
	}
	if ( pRadio->iState != iState ) {
		pRadio->iState = iState;
		xgeXuiWidgetMarkPaint(pRadio->pWidget);
	}
	if ( pRadio->pWidget != NULL ) {
		iVisualState = iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE | XGE_XUI_STATE_FOCUS | XGE_XUI_STATE_DISABLED | XGE_XUI_STATE_CHECKED);
		xgeXuiWidgetSetVisualState(pRadio->pWidget, iVisualState);
	}
}

static void __xgeXuiRadioCacheInvalidate(xge_xui_radio pRadio, int bLayout)
{
	if ( pRadio == NULL ) {
		return;
	}
	__xgeXuiRenderCacheInvalidate(&pRadio->tCache);
	if ( bLayout ) {
		xgeXuiWidgetMarkLayout(pRadio->pWidget);
	}
	xgeXuiWidgetMarkPaint(pRadio->pWidget);
}

static void __xgeXuiRadioGroupUnlink(xge_xui_radio pRadio)
{
	xge_xui_radio pPrev;
	xge_xui_radio pIt;

	if ( (pRadio == NULL) || (pRadio->pGroup == NULL) ) {
		return;
	}
	pPrev = NULL;
	pIt = pRadio->pGroup->pFirst;
	while ( pIt != NULL ) {
		if ( pIt == pRadio ) {
			if ( pPrev != NULL ) {
				pPrev->pNextInGroup = pIt->pNextInGroup;
			} else {
				pRadio->pGroup->pFirst = pIt->pNextInGroup;
			}
			break;
		}
		pPrev = pIt;
		pIt = pIt->pNextInGroup;
	}
	pRadio->pGroup = NULL;
	pRadio->pNextInGroup = NULL;
}

static int __xgeXuiRadioCanFocus(xge_xui_radio pRadio)
{
	if ( (pRadio == NULL) || (pRadio->pWidget == NULL) ) {
		return 0;
	}
	return ((pRadio->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0) &&
		((pRadio->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) != 0) &&
		((pRadio->pWidget->iFlags & XGE_XUI_WIDGET_FOCUSABLE) != 0);
}

static xge_xui_radio __xgeXuiRadioGroupStep(xge_xui_radio pRadio, int bBackward)
{
	xge_xui_radio pFirst;
	xge_xui_radio pPrev;
	xge_xui_radio pIt;
	xge_xui_radio pLastFocusable;
	xge_xui_radio pFirstFocusable;

	if ( (pRadio == NULL) || (pRadio->pGroup == NULL) ) {
		return NULL;
	}
	pFirst = pRadio->pGroup->pFirst;
	if ( pFirst == NULL ) {
		return NULL;
	}
	if ( !bBackward ) {
		for ( pIt = pRadio->pNextInGroup; pIt != NULL; pIt = pIt->pNextInGroup ) {
			if ( __xgeXuiRadioCanFocus(pIt) ) {
				return pIt;
			}
		}
		for ( pIt = pFirst; pIt != NULL && pIt != pRadio; pIt = pIt->pNextInGroup ) {
			if ( __xgeXuiRadioCanFocus(pIt) ) {
				return pIt;
			}
		}
		return __xgeXuiRadioCanFocus(pRadio) ? pRadio : NULL;
	}
	pPrev = NULL;
	pLastFocusable = NULL;
	pFirstFocusable = NULL;
	for ( pIt = pFirst; pIt != NULL; pIt = pIt->pNextInGroup ) {
		if ( __xgeXuiRadioCanFocus(pIt) ) {
			if ( pFirstFocusable == NULL ) {
				pFirstFocusable = pIt;
			}
			pLastFocusable = pIt;
		}
		if ( pIt == pRadio ) {
			break;
		}
		if ( __xgeXuiRadioCanFocus(pIt) ) {
			pPrev = pIt;
		}
	}
	if ( pPrev != NULL ) {
		return pPrev;
	}
	return (pFirstFocusable == pRadio) ? pLastFocusable : pFirstFocusable;
}

static void __xgeXuiRadioSetCheckedInternal(xge_xui_radio pRadio, int bChecked, int bNotify)
{
	int iState;

	if ( (pRadio == NULL) || (pRadio->pWidget == NULL) ) {
		return;
	}
	bChecked = bChecked ? 1 : 0;
	if ( __xgeXuiRadioIsChecked(pRadio) == bChecked ) {
		return;
	}
	iState = pRadio->pWidget->iVisualState;
	if ( bChecked ) {
		iState |= XGE_XUI_STATE_CHECKED;
	} else {
		iState &= ~XGE_XUI_STATE_CHECKED;
	}
	xgeXuiWidgetSetVisualState(pRadio->pWidget, iState);
	pRadio->iState = (pRadio->iState & ~XGE_XUI_STATE_CHECKED) | (bChecked ? XGE_XUI_STATE_CHECKED : 0);
	pRadio->iChangeCount++;
	__xgeXuiRenderCacheInvalidate(&pRadio->tCache);
	if ( bNotify && pRadio->procChange != NULL ) {
		pRadio->procChange(pRadio->pWidget, bChecked, pRadio->pUser);
	}
}

void xgeXuiRadioGroupInit(xge_xui_radio_group pGroup)
{
	if ( pGroup == NULL ) {
		return;
	}
	memset(pGroup, 0, sizeof(*pGroup));
	pGroup->iSelectedValue = -1;
}

void xgeXuiRadioGroupUnit(xge_xui_radio_group pGroup)
{
	xge_xui_radio pIt;
	xge_xui_radio pNext;

	if ( pGroup == NULL ) {
		return;
	}
	pIt = pGroup->pFirst;
	while ( pIt != NULL ) {
		pNext = pIt->pNextInGroup;
		pIt->pGroup = NULL;
		pIt->pNextInGroup = NULL;
		pIt = pNext;
	}
	memset(pGroup, 0, sizeof(*pGroup));
}

void xgeXuiRadioGroupSetChange(xge_xui_radio_group pGroup, xge_xui_select_proc procChange, void* pUser)
{
	if ( pGroup == NULL ) {
		return;
	}
	pGroup->procChange = procChange;
	pGroup->pUser = pUser;
}

void xgeXuiRadioGroupSetSelected(xge_xui_radio_group pGroup, int iValue)
{
	xge_xui_radio pIt;
	int bChanged;

	if ( pGroup == NULL ) {
		return;
	}
	bChanged = (pGroup->iSelectedValue != iValue);
	pGroup->iSelectedValue = iValue;
	pIt = pGroup->pFirst;
	while ( pIt != NULL ) {
		__xgeXuiRadioSetCheckedInternal(pIt, pIt->iValue == iValue, bChanged && (pIt->iValue == iValue));
		pIt = pIt->pNextInGroup;
	}
	if ( bChanged ) {
		pGroup->iChangeCount++;
		if ( pGroup->procChange != NULL ) {
			pGroup->procChange(NULL, iValue, pGroup->pUser);
		}
	}
}

int xgeXuiRadioGroupGetSelected(xge_xui_radio_group pGroup)
{
	if ( pGroup == NULL ) {
		return -1;
	}
	return pGroup->iSelectedValue;
}

int xgeXuiRadioInit(xge_xui_radio pRadio, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pRadio == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pRadio, 0, sizeof(*pRadio));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pRadio->pContext = pContext;
	pRadio->pWidget = pWidget;
	pRadio->pFont = pTheme->pFont;
	pRadio->sText = "";
	pRadio->iValue = -1;
	pRadio->iTextColor = pTheme->iTextColor;
	pRadio->iTextFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pRadio->iColorRing = pTheme->iBorderColor;
	pRadio->iColorChecked = pTheme->iAccentColor;
	pRadio->fIndicatorSize = 17.0f;
	pRadio->fGap = 6.0f;
	pRadio->iCacheMode = XGE_XUI_CACHE_AUTO;
	pRadio->iCacheState = -1;
	__xgeXuiRenderCacheInit(&pRadio->tCache);
	xgeXuiWidgetSetBackground(pWidget, 0);
	xgeXuiWidgetSetBorder(pWidget, 0.0f, 0);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiRadioEventProc, NULL);
	pWidget->procPaint = xgeXuiRadioPaintProc;
	pWidget->pUser = pRadio;
	__xgeXuiRadioSetState(pRadio, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiRadioUnit(xge_xui_radio pRadio)
{
	if ( pRadio == NULL ) {
		return;
	}
	__xgeXuiRenderCacheUnit(&pRadio->tCache);
	__xgeXuiRadioGroupUnlink(pRadio);
	if ( pRadio->pWidget != NULL && pRadio->pWidget->pUser == pRadio ) {
		pRadio->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pRadio->pWidget, NULL, NULL);
		pRadio->pWidget->procPaint = NULL;
	}
	memset(pRadio, 0, sizeof(*pRadio));
}

void xgeXuiRadioSetGroup(xge_xui_radio pRadio, xge_xui_radio_group pGroup, int iValue)
{
	xge_xui_radio pIt;

	if ( pRadio == NULL ) {
		return;
	}
	__xgeXuiRadioGroupUnlink(pRadio);
	pRadio->iValue = iValue;
	if ( pGroup != NULL ) {
		pRadio->pGroup = pGroup;
		if ( pGroup->pFirst == NULL ) {
			pGroup->pFirst = pRadio;
		} else {
			pIt = pGroup->pFirst;
			while ( pIt->pNextInGroup != NULL ) {
				pIt = pIt->pNextInGroup;
			}
			pIt->pNextInGroup = pRadio;
		}
		__xgeXuiRadioSetCheckedInternal(pRadio, pGroup->iSelectedValue == iValue, 0);
	}
}

void xgeXuiRadioSetChange(xge_xui_radio pRadio, xge_xui_checked_proc procChange, void* pUser)
{
	if ( pRadio == NULL ) {
		return;
	}
	pRadio->procChange = procChange;
	pRadio->pUser = pUser;
}

void xgeXuiRadioSetText(xge_xui_radio pRadio, xge_font pFont, const char* sText)
{
	if ( pRadio == NULL ) {
		return;
	}
	pRadio->pFont = pFont;
	pRadio->sText = (sText != NULL) ? sText : "";
	__xgeXuiRadioCacheInvalidate(pRadio, 1);
}

void xgeXuiRadioSetChecked(xge_xui_radio pRadio, int bChecked)
{
	if ( pRadio == NULL ) {
		return;
	}
	if ( pRadio->pGroup != NULL ) {
		if ( bChecked ) {
			xgeXuiRadioGroupSetSelected(pRadio->pGroup, pRadio->iValue);
		} else if ( pRadio->pGroup->iSelectedValue == pRadio->iValue ) {
			xgeXuiRadioGroupSetSelected(pRadio->pGroup, -1);
		}
	} else {
		__xgeXuiRadioSetCheckedInternal(pRadio, bChecked, 0);
	}
}

int xgeXuiRadioGetChecked(xge_xui_radio pRadio)
{
	return __xgeXuiRadioIsChecked(pRadio);
}

void xgeXuiRadioSetTextColor(xge_xui_radio pRadio, uint32_t iColor)
{
	if ( pRadio == NULL ) {
		return;
	}
	pRadio->iTextColor = iColor;
	__xgeXuiRadioCacheInvalidate(pRadio, 0);
}

void xgeXuiRadioSetColors(xge_xui_radio pRadio, uint32_t iRing, uint32_t iChecked)
{
	if ( pRadio == NULL ) {
		return;
	}
	pRadio->iColorRing = iRing;
	pRadio->iColorChecked = iChecked;
	__xgeXuiRadioCacheInvalidate(pRadio, 0);
}

void xgeXuiRadioSetTextures(xge_xui_radio pRadio, xge_texture pUncheckedTexture, xge_rect_t tUncheckedSrc, xge_texture pCheckedTexture, xge_rect_t tCheckedSrc)
{
	if ( pRadio == NULL ) {
		return;
	}
	pRadio->pUncheckedTexture = pUncheckedTexture;
	pRadio->pCheckedTexture = pCheckedTexture;
	pRadio->tUncheckedSrc = tUncheckedSrc;
	pRadio->tCheckedSrc = tCheckedSrc;
	__xgeXuiRadioCacheInvalidate(pRadio, 1);
}

void xgeXuiRadioSetIndicatorSize(xge_xui_radio pRadio, float fSize)
{
	if ( pRadio == NULL ) {
		return;
	}
	pRadio->fIndicatorSize = (fSize > 0.0f) ? fSize : 0.0f;
	__xgeXuiRadioCacheInvalidate(pRadio, 1);
}

void xgeXuiRadioSetGap(xge_xui_radio pRadio, float fGap)
{
	if ( pRadio == NULL ) {
		return;
	}
	pRadio->fGap = (fGap >= 0.0f) ? fGap : 6.0f;
	__xgeXuiRadioCacheInvalidate(pRadio, 1);
}

void xgeXuiRadioSetCacheMode(xge_xui_radio pRadio, int iMode)
{
	if ( pRadio == NULL ) {
		return;
	}
	pRadio->iCacheMode = __xgeXuiChoiceCacheModeNormalize(iMode);
	__xgeXuiRadioCacheInvalidate(pRadio, 0);
}

int xgeXuiRadioGetState(xge_xui_radio pRadio)
{
	if ( pRadio == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiRadioSetState(pRadio, pRadio->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pRadio->iState;
}

int xgeXuiRadioEvent(xge_xui_radio pRadio, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;

	if ( (pRadio == NULL) || (pRadio->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pRadio->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pRadio->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiRadioSetState(pRadio, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pRadio->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pRadio->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			__xgeXuiRadioSetState(pRadio, iInside ? (iState | XGE_XUI_STATE_HOVER) : (iState & ~XGE_XUI_STATE_HOVER));
			return ((pRadio->iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiRadioSetState(pRadio, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiRadioSetState(pRadio, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiRadioSetState(pRadio, iState);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pRadio->pContext, pRadio->pWidget);
			xgeXuiSetPointerCapture(pRadio->pContext, pEvent->iPointerId, pRadio->pWidget);
			__xgeXuiRadioSetState(pRadio, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pRadio->iState & XGE_XUI_STATE_ACTIVE) != 0);
			__xgeXuiRadioSetState(pRadio, iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			if ( pRadio->pContext != NULL && xgeXuiGetPointerCapture(pRadio->pContext, pEvent->iPointerId) == pRadio->pWidget ) {
				xgeXuiSetPointerCapture(pRadio->pContext, pEvent->iPointerId, NULL);
			}
			if ( bWasActive && iInside ) {
				xgeXuiRadioSetChecked(pRadio, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			__xgeXuiRadioSetState(pRadio, XGE_XUI_STATE_NORMAL);
			if ( pRadio->pContext != NULL && xgeXuiGetPointerCapture(pRadio->pContext, pEvent->iPointerId) == pRadio->pWidget ) {
				xgeXuiSetPointerCapture(pRadio->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( (pRadio->pContext == NULL) || (pRadio->pContext->pFocus != pRadio->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 == XGE_KEY_RIGHT) || (pEvent->iParam1 == XGE_KEY_DOWN) ||
			     (pEvent->iParam1 == XGE_KEY_LEFT) || (pEvent->iParam1 == XGE_KEY_UP) ) {
				xge_xui_radio pNext;

				pNext = __xgeXuiRadioGroupStep(pRadio, (pEvent->iParam1 == XGE_KEY_LEFT) || (pEvent->iParam1 == XGE_KEY_UP));
				if ( pNext != NULL ) {
					xgeXuiSetFocus(pNext->pContext, pNext->pWidget);
					xgeXuiRadioSetChecked(pNext, 1);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( (pEvent->iParam1 != XGE_KEY_ENTER) && (pEvent->iParam1 != XGE_KEY_SPACE) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiRadioSetChecked(pRadio, 1);
			return XGE_XUI_EVENT_CONSUMED;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiRadioEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiRadioEvent((xge_xui_radio)pUser, pEvent);
}

static float __xgeXuiRadioIndicatorSize(xge_xui_radio pRadio)
{
	xge_texture pTexture;
	xge_rect_t tSrc;

	if ( pRadio == NULL ) {
		return 17.0f;
	}
	if ( pRadio->fIndicatorSize > 0.0f ) {
		return pRadio->fIndicatorSize;
	}
	pTexture = xgeXuiRadioGetChecked(pRadio) ? pRadio->pCheckedTexture : pRadio->pUncheckedTexture;
	tSrc = xgeXuiRadioGetChecked(pRadio) ? pRadio->tCheckedSrc : pRadio->tUncheckedSrc;
	tSrc = __xgeXuiChoiceTextureSrc(pTexture, tSrc);
	if ( tSrc.fH > 0.0f ) {
		return tSrc.fH;
	}
	return 17.0f;
}

static xge_texture __xgeXuiChoiceDefaultRadioTexture(int bChecked, uint32_t iRing, uint32_t iChecked)
{
	static xge_texture_t tUnchecked;
	static xge_texture_t tChecked;
	static uint32_t iUncheckedRing = 0;
	static uint32_t iCheckedRing = 0;
	static uint32_t iCheckedDot = 0;
	static int bUncheckedReady = 0;
	static int bCheckedReady = 0;
	xge_texture pTexture;
	unsigned char arrPixels[34 * 34 * 4];
	float fX;
	float fY;
	float fDX;
	float fDY;
	float fDist;
	float fOuterR;
	float fInnerR;
	float fDotR;
	float fCoverage;
	int x;
	int y;

	if ( (!bChecked && bUncheckedReady && iUncheckedRing == iRing) || (bChecked && bCheckedReady && iCheckedRing == iRing && iCheckedDot == iChecked) ) {
		return bChecked ? &tChecked : &tUnchecked;
	}
	memset(arrPixels, 0, sizeof(arrPixels));
	fOuterR = 15.0f;
	fInnerR = 12.0f;
	fDotR = 6.3f;
	for ( y = 0; y < 34; y++ ) {
		for ( x = 0; x < 34; x++ ) {
			fX = (float)x + 0.5f;
			fY = (float)y + 0.5f;
			fDX = fX - 17.0f;
			fDY = fY - 17.0f;
			fDist = sqrtf(fDX * fDX + fDY * fDY);
			if ( fDist <= fOuterR + 0.5f ) {
				__xgeXuiChoicePutPixel(arrPixels, 34, x, y, XGE_COLOR_RGBA(255, 255, 255, 255), __xgeXuiChoiceSmoothCoverage(fDist, fOuterR));
			}
			if ( fDist >= fInnerR - 0.5f && fDist <= fOuterR + 0.5f ) {
				fCoverage = __xgeXuiChoiceSmoothCoverage(fDist - ((fOuterR + fInnerR) * 0.5f), (fOuterR - fInnerR) * 0.5f);
				__xgeXuiChoicePutPixel(arrPixels, 34, x, y, bChecked ? iChecked : iRing, fCoverage);
			}
			if ( bChecked && fDist <= fDotR + 0.5f ) {
				__xgeXuiChoicePutPixel(arrPixels, 34, x, y, iChecked, __xgeXuiChoiceSmoothCoverage(fDist, fDotR));
			}
		}
	}
	pTexture = bChecked ? &tChecked : &tUnchecked;
	xgeTextureFree(pTexture);
	if ( xgeTextureCreateRGBA(pTexture, 34, 34, arrPixels) != XGE_OK ) {
		return NULL;
	}
	if ( bChecked ) {
		bCheckedReady = 1;
		iCheckedRing = iRing;
		iCheckedDot = iChecked;
	} else {
		bUncheckedReady = 1;
		iUncheckedRing = iRing;
	}
	return pTexture;
}

static void __xgeXuiRadioDrawDirect(xge_xui_widget pWidget, xge_xui_radio pRadio, xge_rect_t tRect)
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
	if ( pRadio == NULL ) {
		return;
	}
	bChecked = xgeXuiRadioGetChecked(pRadio);
	fSize = __xgeXuiRadioIndicatorSize(pRadio);
	if ( fSize < 1.0f ) {
		fSize = 1.0f;
	}
	if ( fSize > tRect.fH ) {
		fSize = tRect.fH;
	}
	fGap = (pRadio->fGap >= 0.0f) ? pRadio->fGap : 6.0f;
	tTextSize.fX = 0.0f;
	tTextSize.fY = 0.0f;
	if ( (pRadio->pFont != NULL) && (pRadio->sText != NULL) && (pRadio->sText[0] != 0) ) {
		tTextSize = __xgeXuiHostMeasureText(pRadio->pFont, pRadio->sText);
	}
	fLineH = (tTextSize.fY > fSize) ? tTextSize.fY : fSize;
	if ( fLineH > tRect.fH ) {
		fLineH = tRect.fH;
	}
	tBox.fX = tRect.fX;
	tBox.fY = tRect.fY + (tRect.fH - fLineH) * 0.5f + (fLineH - fSize) * 0.5f;
	tBox.fW = fSize;
	tBox.fH = fSize;
	pTexture = bChecked ? pRadio->pCheckedTexture : pRadio->pUncheckedTexture;
	tSrc = bChecked ? pRadio->tCheckedSrc : pRadio->tUncheckedSrc;
	if ( pTexture != NULL ) {
		__xgeXuiChoiceDrawTexture(pTexture, tSrc, tBox);
	} else {
		pTexture = __xgeXuiChoiceDefaultRadioTexture(bChecked, pRadio->iColorRing, pRadio->iColorChecked);
		if ( pTexture != NULL ) {
			__xgeXuiChoiceDrawTexture(pTexture, (xge_rect_t){ 0.0f, 0.0f, 34.0f, 34.0f }, tBox);
		}
	}
	if ( (pRadio->pFont != NULL) && (pRadio->sText != NULL) && (pRadio->sText[0] != 0) ) {
		tText = tRect;
		tText.fX += fSize + fGap;
		tText.fW -= fSize + fGap;
		if ( tText.fW > 0.0f ) {
			__xgeXuiHostDrawTextRect(pRadio->pFont, pRadio->sText, tText, pRadio->iTextColor, pRadio->iTextFlags);
		}
	}
}

static void __xgeXuiRadioPaintCacheContent(xge_rect_t tRect, void* pUser)
{
	xge_xui_choice_cache_paint_t* pPaint;

	pPaint = (xge_xui_choice_cache_paint_t*)pUser;
	if ( pPaint == NULL ) {
		return;
	}
	__xgeXuiRadioDrawDirect(pPaint->pWidget, (xge_xui_radio)pPaint->pControl, tRect);
}

static int __xgeXuiRadioPaintCache(xge_xui_widget pWidget, xge_xui_radio pRadio)
{
	xge_xui_choice_cache_paint_t tPaint;
	xge_texture pTexture;
	xge_draw_t tDraw;
	xge_rect_t tContent;
	float fDipScale;
	int iState;

	if ( (pWidget == NULL) || (pRadio == NULL) || (pRadio->iCacheMode == XGE_XUI_CACHE_OFF) ) {
		return 0;
	}
	tContent = pWidget->tContentRect;
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		return 0;
	}
	fDipScale = (pRadio->pContext != NULL && pRadio->pContext->fDipScale > 0.0f) ? pRadio->pContext->fDipScale : 1.0f;
	iState = pWidget->iVisualState | (xgeXuiWidgetIsEnabled(pWidget) ? 0 : XGE_XUI_STATE_DISABLED);
	if ( pRadio->iCacheState != iState ) {
		pRadio->iCacheState = iState;
		__xgeXuiRenderCacheInvalidate(&pRadio->tCache);
	}
	memset(&tPaint, 0, sizeof(tPaint));
	tPaint.pWidget = pWidget;
	tPaint.pControl = pRadio;
	pTexture = __xgeXuiRenderCacheEnsure(&pRadio->tCache, tContent, fDipScale, __xgeXuiRadioPaintCacheContent, &tPaint);
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

void xgeXuiRadioPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_radio pRadio;

	pRadio = (xge_xui_radio)pUser;
	if ( (pWidget == NULL) || (pRadio == NULL) ) {
		return;
	}
	if ( __xgeXuiRadioPaintCache(pWidget, pRadio) ) {
		return;
	}
	__xgeXuiRadioDrawDirect(pWidget, pRadio, pWidget->tContentRect);
}
