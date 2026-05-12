typedef struct xge_xui_slider_cache_paint_t {
	xge_xui_slider pSlider;
	xge_xui_widget pWidget;
} xge_xui_slider_cache_paint_t;

static int __xgeXuiSliderCacheModeNormalize(int iMode)
{
	if ( (iMode != XGE_XUI_CACHE_AUTO) && (iMode != XGE_XUI_CACHE_OFF) && (iMode != XGE_XUI_CACHE_FORCE) ) {
		return XGE_XUI_CACHE_AUTO;
	}
	return iMode;
}

static void __xgeXuiSliderCacheInvalidate(xge_xui_slider pSlider, int bLayout)
{
	if ( pSlider == NULL ) {
		return;
	}
	__xgeXuiRenderCacheInvalidate(&pSlider->tCache);
	if ( bLayout ) {
		xgeXuiWidgetMarkLayout(pSlider->pWidget);
	}
	xgeXuiWidgetMarkPaint(pSlider->pWidget);
}

static xge_rect_t __xgeXuiSliderTrackRect(xge_xui_widget pWidget, xge_xui_slider pSlider, xge_rect_t tContent)
{
	xge_rect_t tTrack;
	float fTrackSize;

	(void)pWidget;
	tTrack = tContent;
	fTrackSize = (pSlider != NULL && pSlider->fTrackSize > 0.0f) ? pSlider->fTrackSize : 4.0f;
	if ( pSlider != NULL && pSlider->iOrientation == XGE_XUI_SEPARATOR_VERTICAL ) {
		if ( fTrackSize > tTrack.fW ) {
			fTrackSize = tTrack.fW;
		}
		tTrack.fX += (tTrack.fW - fTrackSize) * 0.5f;
		tTrack.fW = fTrackSize;
	} else {
		if ( fTrackSize > tTrack.fH ) {
			fTrackSize = tTrack.fH;
		}
		tTrack.fX += 2.0f;
		tTrack.fW -= 4.0f;
		if ( tTrack.fW < 1.0f ) {
			tTrack.fW = 1.0f;
		}
		tTrack.fY += (tTrack.fH - fTrackSize) * 0.5f;
		tTrack.fH = fTrackSize;
	}
	return tTrack;
}

static xge_rect_t __xgeXuiSliderFillRect(xge_xui_slider pSlider, xge_rect_t tTrack, float fRate)
{
	xge_rect_t tFill;

	tFill = tTrack;
	fRate = __xgeXuiClampFloat(fRate, 0.0f, 1.0f);
	if ( pSlider != NULL && pSlider->iOrientation == XGE_XUI_SEPARATOR_VERTICAL ) {
		tFill.fH = tTrack.fH * fRate;
		tFill.fY = tTrack.fY + tTrack.fH - tFill.fH;
	} else {
		tFill.fW = tTrack.fW * fRate;
	}
	return tFill;
}

static xge_rect_t __xgeXuiSliderKnobRect(xge_xui_widget pWidget, xge_xui_slider pSlider, float fRate)
{
	xge_rect_t tRect;
	float fSize;
	float fCenter;

	tRect = pWidget->tContentRect;
	fSize = (pSlider != NULL && pSlider->fKnobSize > 0.0f) ? pSlider->fKnobSize : 14.0f;
	if ( pSlider != NULL && pSlider->iOrientation == XGE_XUI_SEPARATOR_VERTICAL ) {
		if ( fSize > pWidget->tContentRect.fW ) {
			fSize = pWidget->tContentRect.fW;
		}
		fCenter = pWidget->tContentRect.fY + pWidget->tContentRect.fH * (1.0f - __xgeXuiClampFloat(fRate, 0.0f, 1.0f));
		tRect.fX = pWidget->tContentRect.fX + (pWidget->tContentRect.fW - fSize) * 0.5f;
		tRect.fY = fCenter - fSize * 0.5f;
		tRect.fW = fSize;
		tRect.fH = fSize;
		if ( tRect.fY < pWidget->tContentRect.fY ) {
			tRect.fY = pWidget->tContentRect.fY;
		}
		if ( tRect.fY + tRect.fH > pWidget->tContentRect.fY + pWidget->tContentRect.fH ) {
			tRect.fY = pWidget->tContentRect.fY + pWidget->tContentRect.fH - tRect.fH;
		}
	} else {
		if ( fSize > pWidget->tContentRect.fH ) {
			fSize = pWidget->tContentRect.fH;
		}
		fCenter = pWidget->tContentRect.fX + pWidget->tContentRect.fW * __xgeXuiClampFloat(fRate, 0.0f, 1.0f);
		tRect.fX = fCenter - fSize * 0.5f;
		tRect.fY = pWidget->tContentRect.fY + (pWidget->tContentRect.fH - fSize) * 0.5f;
		tRect.fW = fSize;
		tRect.fH = fSize;
		if ( tRect.fX < pWidget->tContentRect.fX ) {
			tRect.fX = pWidget->tContentRect.fX;
		}
		if ( tRect.fX + tRect.fW > pWidget->tContentRect.fX + pWidget->tContentRect.fW ) {
			tRect.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - tRect.fW;
		}
	}
	return tRect;
}

static uint32_t __xgeXuiSliderTrackColor(xge_xui_slider pSlider)
{
	if ( (pSlider != NULL) && ((pSlider->iState & XGE_XUI_STATE_DISABLED) != 0) ) {
		return XGE_COLOR_RGBA(214, 222, 232, 255);
	}
	return (pSlider != NULL) ? pSlider->iColorTrack : XGE_COLOR_RGBA(214, 222, 232, 255);
}

static uint32_t __xgeXuiSliderFillColor(xge_xui_slider pSlider)
{
	if ( (pSlider != NULL) && ((pSlider->iState & XGE_XUI_STATE_DISABLED) != 0) ) {
		return pSlider->iColorDisabled;
	}
	return (pSlider != NULL) ? pSlider->iColorFill : XGE_COLOR_RGBA(46, 124, 214, 255);
}

static void __xgeXuiSliderDrawDirect(xge_xui_widget pWidget, xge_xui_slider pSlider, xge_rect_t tContent)
{
	xge_rect_t tTrack;
	xge_rect_t tFill;
	xge_rect_t tKnob;
	float fRate;
	float fTrackRadius;
	uint32_t iFillColor;
	uint32_t iKnobColor;

	if ( (pWidget == NULL) || (pSlider == NULL) || (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		return;
	}
	fRate = __xgeXuiSliderRate(pSlider);
	tTrack = __xgeXuiSliderTrackRect(pWidget, pSlider, tContent);
	tFill = __xgeXuiSliderFillRect(pSlider, tTrack, fRate);
	tKnob = __xgeXuiSliderKnobRect(pWidget, pSlider, fRate);
	fTrackRadius = (pSlider->fTrackRadius >= 0.0f) ? pSlider->fTrackRadius : ((tTrack.fW < tTrack.fH ? tTrack.fW : tTrack.fH) * 0.5f);
	iFillColor = __xgeXuiSliderFillColor(pSlider);
	iKnobColor = ((pSlider->iState & XGE_XUI_STATE_DISABLED) != 0) ? XGE_COLOR_RGBA(232, 236, 242, 255) : pSlider->iColorKnob;
	if ( (pSlider->iState & XGE_XUI_STATE_FOCUS) != 0 && XGE_COLOR_GET_A(pSlider->iColorFocus) != 0 ) {
		__xgeXuiHostDrawRoundedRect(pWidget->tRect, pSlider->iColorFocus, pWidget->tStyle.fRadius);
	}
	if ( XGE_COLOR_GET_A(__xgeXuiSliderTrackColor(pSlider)) != 0 ) {
		__xgeXuiHostDrawRoundedRect(tTrack, __xgeXuiSliderTrackColor(pSlider), fTrackRadius);
	}
	if ( XGE_COLOR_GET_A(iFillColor) != 0 && tFill.fW > 0.0f && tFill.fH > 0.0f ) {
		__xgeXuiHostDrawRoundedRect(tFill, iFillColor, fTrackRadius);
	}
	if ( XGE_COLOR_GET_A(iKnobColor) != 0 ) {
		xgeShapeCircleFillPx(tKnob.fX + tKnob.fW * 0.5f, tKnob.fY + tKnob.fH * 0.5f, (tKnob.fW < tKnob.fH ? tKnob.fW : tKnob.fH) * 0.5f, iKnobColor);
	}
	if ( XGE_COLOR_GET_A(pSlider->iColorKnobBorder) != 0 ) {
		xgeShapeCircleStrokePx(tKnob.fX + tKnob.fW * 0.5f, tKnob.fY + tKnob.fH * 0.5f, (tKnob.fW < tKnob.fH ? tKnob.fW : tKnob.fH) * 0.5f - 0.5f, 1.0f, pSlider->iColorKnobBorder);
	}
}

static void __xgeXuiSliderPaintCacheContent(xge_rect_t tRect, void* pUser)
{
	xge_xui_slider_cache_paint_t* pPaint;
	xge_xui_widget_t tWidget;
	xge_xui_slider_t tSlider;

	pPaint = (xge_xui_slider_cache_paint_t*)pUser;
	if ( pPaint == NULL ) {
		return;
	}
	tWidget = *pPaint->pWidget;
	tSlider = *pPaint->pSlider;
	tWidget.tRect = tRect;
	tWidget.tBorderRect = tRect;
	tWidget.tPaddingRect = tRect;
	tWidget.tContentRect = tRect;
	tSlider.pWidget = &tWidget;
	__xgeXuiSliderDrawDirect(&tWidget, &tSlider, tRect);
}

static int __xgeXuiSliderPaintCache(xge_xui_widget pWidget, xge_xui_slider pSlider)
{
	xge_xui_slider_cache_paint_t tPaint;
	xge_draw_t tDraw;
	xge_texture pTexture;
	xge_rect_t tContent;
	float fDipScale;
	int iState;

	if ( (pWidget == NULL) || (pSlider == NULL) || (pSlider->iCacheMode == XGE_XUI_CACHE_OFF) ) {
		return 0;
	}
	tContent = pWidget->tContentRect;
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		return 0;
	}
	iState = pWidget->iVisualState | (xgeXuiWidgetIsEnabled(pWidget) ? 0 : XGE_XUI_STATE_DISABLED);
	if ( iState != pSlider->iCacheState ) {
		pSlider->iCacheState = iState;
		__xgeXuiRenderCacheInvalidate(&pSlider->tCache);
	}
	fDipScale = (pSlider->pContext != NULL && pSlider->pContext->fDipScale > 0.0f) ? pSlider->pContext->fDipScale : 1.0f;
	memset(&tPaint, 0, sizeof(tPaint));
	tPaint.pWidget = pWidget;
	tPaint.pSlider = pSlider;
	pTexture = __xgeXuiRenderCacheEnsure(&pSlider->tCache, tContent, fDipScale, __xgeXuiSliderPaintCacheContent, &tPaint);
	if ( pTexture == NULL ) {
		return 0;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pTexture;
	tDraw.tDst = tContent;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_FLIP_Y;
	__xgeXuiHostDrawImage(&tDraw);
	return 1;
}

int xgeXuiSliderInit(xge_xui_slider pSlider, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pSlider == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pSlider, 0, sizeof(*pSlider));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pSlider->pContext = pContext;
	pSlider->pWidget = pWidget;
	pSlider->fMin = 0.0f;
	pSlider->fMax = 1.0f;
	pSlider->fValue = 0.0f;
	pSlider->fStep = 0.0f;
	pSlider->fPageStep = 0.0f;
	pSlider->fTrackSize = 4.0f;
	pSlider->fKnobSize = 14.0f;
	pSlider->fTrackRadius = -1.0f;
	pSlider->fKnobRadius = -1.0f;
	pSlider->iColorTrack = pTheme->iBorderColor;
	pSlider->iColorFill = pTheme->iAccentColor;
	pSlider->iColorKnob = XGE_COLOR_RGBA(255, 255, 255, 255);
	pSlider->iColorKnobBorder = XGE_COLOR_RGBA(154, 174, 198, 255);
	pSlider->iColorFocus = XGE_COLOR_RGBA(0, 0, 0, 0);
	pSlider->iColorDisabled = pTheme->iStateDisabled;
	pSlider->iOrientation = XGE_XUI_SEPARATOR_HORIZONTAL;
	pSlider->iCacheMode = XGE_XUI_CACHE_AUTO;
	pSlider->iCacheState = -1;
	__xgeXuiRenderCacheInit(&pSlider->tCache);
	xgeXuiWidgetSetBackground(pWidget, 0);
	xgeXuiWidgetSetBorder(pWidget, 0.0f, 0);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiSliderEventProc, pSlider);
	pWidget->procPaint = xgeXuiSliderPaintProc;
	pWidget->pUser = pSlider;
	__xgeXuiSliderSetState(pSlider, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiSliderUnit(xge_xui_slider pSlider)
{
	if ( pSlider == NULL ) {
		return;
	}
	__xgeXuiRenderCacheUnit(&pSlider->tCache);
	if ( pSlider->pWidget != NULL && pSlider->pWidget->pUser == pSlider ) {
		pSlider->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pSlider->pWidget, NULL, NULL);
		pSlider->pWidget->procPaint = NULL;
	}
	memset(pSlider, 0, sizeof(*pSlider));
}

void xgeXuiSliderSetChange(xge_xui_slider pSlider, xge_xui_slider_proc procChange, void* pUser)
{
	if ( pSlider == NULL ) {
		return;
	}
	pSlider->procChange = procChange;
	pSlider->pUser = pUser;
}

void xgeXuiSliderSetRange(xge_xui_slider pSlider, float fMin, float fMax)
{
	if ( pSlider == NULL ) {
		return;
	}
	if ( fMax < fMin ) {
		float fSwap;
		fSwap = fMin;
		fMin = fMax;
		fMax = fSwap;
	}
	if ( fMax == fMin ) {
		fMax = fMin + 1.0f;
	}
	pSlider->fMin = fMin;
	pSlider->fMax = fMax;
	__xgeXuiSliderSetValueInternal(pSlider, pSlider->fValue, 0);
	__xgeXuiSliderCacheInvalidate(pSlider, 0);
}

void xgeXuiSliderSetValue(xge_xui_slider pSlider, float fValue)
{
	__xgeXuiSliderSetValueInternal(pSlider, fValue, 0);
}

float xgeXuiSliderGetValue(xge_xui_slider pSlider)
{
	if ( pSlider == NULL ) {
		return 0.0f;
	}
	return pSlider->fValue;
}

void xgeXuiSliderSetOrientation(xge_xui_slider pSlider, int iOrientation)
{
	if ( pSlider == NULL ) {
		return;
	}
	pSlider->iOrientation = (iOrientation == XGE_XUI_SEPARATOR_VERTICAL) ? XGE_XUI_SEPARATOR_VERTICAL : XGE_XUI_SEPARATOR_HORIZONTAL;
	__xgeXuiSliderCacheInvalidate(pSlider, 1);
}

void xgeXuiSliderSetStep(xge_xui_slider pSlider, float fStep, float fPageStep)
{
	if ( pSlider == NULL ) {
		return;
	}
	pSlider->fStep = (fStep > 0.0f) ? fStep : 0.0f;
	pSlider->fPageStep = (fPageStep > 0.0f) ? fPageStep : 0.0f;
	__xgeXuiSliderSetValueInternal(pSlider, pSlider->fValue, 0);
	__xgeXuiSliderCacheInvalidate(pSlider, 0);
}

void xgeXuiSliderSetMetrics(xge_xui_slider pSlider, float fTrackSize, float fKnobSize, float fTrackRadius, float fKnobRadius)
{
	if ( pSlider == NULL ) {
		return;
	}
	pSlider->fTrackSize = (fTrackSize > 0.0f) ? fTrackSize : 4.0f;
	pSlider->fKnobSize = (fKnobSize > 0.0f) ? fKnobSize : 14.0f;
	pSlider->fTrackRadius = (fTrackRadius >= 0.0f) ? fTrackRadius : -1.0f;
	pSlider->fKnobRadius = (fKnobRadius >= 0.0f) ? fKnobRadius : -1.0f;
	__xgeXuiSliderCacheInvalidate(pSlider, 1);
}

void xgeXuiSliderSetColors(xge_xui_slider pSlider, uint32_t iTrack, uint32_t iFill, uint32_t iKnob, uint32_t iFocus, uint32_t iDisabled)
{
	if ( pSlider == NULL ) {
		return;
	}
	pSlider->iColorTrack = iTrack;
	pSlider->iColorFill = iFill;
	pSlider->iColorKnob = iKnob;
	pSlider->iColorFocus = iFocus;
	pSlider->iColorDisabled = iDisabled;
	__xgeXuiSliderCacheInvalidate(pSlider, 0);
}

void xgeXuiSliderSetKnobBorderColor(xge_xui_slider pSlider, uint32_t iColor)
{
	if ( pSlider == NULL ) {
		return;
	}
	pSlider->iColorKnobBorder = iColor;
	__xgeXuiSliderCacheInvalidate(pSlider, 0);
}

void xgeXuiSliderSetCacheMode(xge_xui_slider pSlider, int iMode)
{
	if ( pSlider == NULL ) {
		return;
	}
	pSlider->iCacheMode = __xgeXuiSliderCacheModeNormalize(iMode);
	__xgeXuiSliderCacheInvalidate(pSlider, 0);
}

int xgeXuiSliderGetState(xge_xui_slider pSlider)
{
	if ( pSlider == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiSliderSetState(pSlider, pSlider->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pSlider->iState;
}

int xgeXuiSliderEvent(xge_xui_slider pSlider, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;
	float fSmallStep;
	float fPageStep;

	if ( (pSlider == NULL) || (pSlider->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pSlider->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pSlider->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiSliderSetState(pSlider, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pSlider->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pSlider->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);

	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			__xgeXuiSliderSetState(pSlider, iInside ? (iState | XGE_XUI_STATE_HOVER) : (iState & ~XGE_XUI_STATE_HOVER));
			if ( (pSlider->pContext != NULL) && (xgeXuiGetPointerCapture(pSlider->pContext, pEvent->iPointerId) == pSlider->pWidget) ) {
				__xgeXuiSliderSetValueFromPoint(pSlider, pEvent->fX, pEvent->fY, 1);
				__xgeXuiSliderSetState(pSlider, pSlider->iState | XGE_XUI_STATE_ACTIVE);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiSliderSetState(pSlider, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiSliderSetState(pSlider, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiSliderSetState(pSlider, iState);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pSlider->pContext, pSlider->pWidget);
			xgeXuiSetPointerCapture(pSlider->pContext, pEvent->iPointerId, pSlider->pWidget);
			__xgeXuiSliderSetValueFromPoint(pSlider, pEvent->fX, pEvent->fY, 1);
			__xgeXuiSliderSetState(pSlider, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pSlider->iState & XGE_XUI_STATE_ACTIVE) != 0);
			if ( bWasActive ) {
				__xgeXuiSliderSetValueFromPoint(pSlider, pEvent->fX, pEvent->fY, 1);
			}
			__xgeXuiSliderSetState(pSlider, iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			if ( pSlider->pContext != NULL && xgeXuiGetPointerCapture(pSlider->pContext, pEvent->iPointerId) == pSlider->pWidget ) {
				xgeXuiSetPointerCapture(pSlider->pContext, pEvent->iPointerId, NULL);
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			__xgeXuiSliderSetState(pSlider, XGE_XUI_STATE_NORMAL);
			if ( pSlider->pContext != NULL && xgeXuiGetPointerCapture(pSlider->pContext, pEvent->iPointerId) == pSlider->pWidget ) {
				xgeXuiSetPointerCapture(pSlider->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( (pSlider->pContext == NULL) || (pSlider->pContext->pFocus != pSlider->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			fSmallStep = (pSlider->fStep > 0.0f) ? pSlider->fStep : (pSlider->fMax - pSlider->fMin) * 0.01f;
			fPageStep = (pSlider->fPageStep > 0.0f) ? pSlider->fPageStep : (pSlider->fMax - pSlider->fMin) * 0.10f;
			if ( (pEvent->iParam1 == XGE_KEY_LEFT) || (pEvent->iParam1 == XGE_KEY_DOWN) ) {
				__xgeXuiSliderSetValueInternal(pSlider, pSlider->fValue - fSmallStep, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( (pEvent->iParam1 == XGE_KEY_RIGHT) || (pEvent->iParam1 == XGE_KEY_UP) ) {
				__xgeXuiSliderSetValueInternal(pSlider, pSlider->fValue + fSmallStep, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_PAGE_DOWN ) {
				__xgeXuiSliderSetValueInternal(pSlider, pSlider->fValue - fPageStep, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_PAGE_UP ) {
				__xgeXuiSliderSetValueInternal(pSlider, pSlider->fValue + fPageStep, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_HOME ) {
				__xgeXuiSliderSetValueInternal(pSlider, pSlider->fMin, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_END ) {
				__xgeXuiSliderSetValueInternal(pSlider, pSlider->fMax, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiSliderEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiSliderEvent((xge_xui_slider)pUser, pEvent);
}

void xgeXuiSliderPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_slider pSlider;

	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	pSlider = (xge_xui_slider)pUser;
	if ( (pWidget == NULL) || (pSlider == NULL) ) {
		return;
	}
	if ( __xgeXuiSliderPaintCache(pWidget, pSlider) ) {
		return;
	}
	__xgeXuiSliderDrawDirect(pWidget, pSlider, pWidget->tContentRect);
}
