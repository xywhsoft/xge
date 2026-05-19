static int __xgeXuiScrollFramePolicyNormalize(int iPolicy)
{
	if ( (iPolicy != XGE_XUI_SCROLLBAR_POLICY_AUTO) && (iPolicy != XGE_XUI_SCROLLBAR_POLICY_ALWAYS) && (iPolicy != XGE_XUI_SCROLLBAR_POLICY_HIDDEN) ) {
		return XGE_XUI_SCROLLBAR_POLICY_AUTO;
	}
	return iPolicy;
}

static int __xgeXuiScrollFrameModeNormalize(int iMode)
{
	return (iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT) ? XGE_XUI_SCROLLBAR_MODE_COMPACT : XGE_XUI_SCROLLBAR_MODE_FULL;
}

static int __xgeXuiScrollFrameWheelAxisNormalize(int iAxis)
{
	if ( (iAxis != XGE_XUI_WHEEL_AXIS_VERTICAL) && (iAxis != XGE_XUI_WHEEL_AXIS_HORIZONTAL) && (iAxis != XGE_XUI_WHEEL_AXIS_BOTH) ) {
		return XGE_XUI_WHEEL_AXIS_VERTICAL;
	}
	return iAxis;
}

static int __xgeXuiScrollFrameCornerModeNormalize(int iMode)
{
	if ( (iMode != XGE_XUI_SCROLL_FRAME_CORNER_NONE) && (iMode != XGE_XUI_SCROLL_FRAME_CORNER_AUTO) && (iMode != XGE_XUI_SCROLL_FRAME_CORNER_GRIP) ) {
		return XGE_XUI_SCROLL_FRAME_CORNER_AUTO;
	}
	return iMode;
}

static float __xgeXuiScrollFrameScrollbarSize(xge_xui_scroll_frame pFrame)
{
	if ( (pFrame != NULL) && (pFrame->fScrollbarSize > 0.0f) ) {
		return pFrame->fScrollbarSize;
	}
	return 16.0f;
}

static float __xgeXuiScrollFrameScrollbarReserveSize(xge_xui_scroll_frame pFrame)
{
	float fSize;
	float fTrack;

	fSize = __xgeXuiScrollFrameScrollbarSize(pFrame);
	if ( (pFrame != NULL) && (pFrame->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_COMPACT) ) {
		fTrack = (pFrame->tVScrollBar.fTrackSize > 0.0f) ? pFrame->tVScrollBar.fTrackSize : 8.0f;
		if ( fTrack > 8.0f ) {
			fTrack = 8.0f;
		}
		if ( fTrack < fSize ) {
			fSize = fTrack;
		}
	}
	if ( fSize < 1.0f ) {
		fSize = 1.0f;
	}
	return fSize;
}

static void __xgeXuiScrollFrameArrangeWidget(xge_xui_widget pWidget, xge_rect_t tRect)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tLocalRect = tRect;
	__xgeXuiWidgetArrangeRect(pWidget, tRect);
}

static void __xgeXuiScrollFrameNotify(xge_xui_scroll_frame pFrame)
{
	if ( (pFrame == NULL) || (pFrame->pModel == NULL) ) {
		return;
	}
	pFrame->iChangeCount++;
	if ( pFrame->procChange != NULL ) {
		pFrame->procChange(pFrame, pFrame->pModel->fScrollX, pFrame->pModel->fScrollY, pFrame->pUser);
	}
	xgeXuiWidgetMarkPaint(pFrame->pWidget);
}

static void __xgeXuiScrollFrameSyncBars(xge_xui_scroll_frame pFrame)
{
	float fMaxX;
	float fMaxY;

	if ( (pFrame == NULL) || (pFrame->pModel == NULL) ) {
		return;
	}
	xgeXuiScrollModelGetMaxOffset(pFrame->pModel, &fMaxX, &fMaxY);
	if ( pFrame->pHScrollWidget != NULL ) {
		xgeXuiWidgetSetVisible(pFrame->pHScrollWidget, pFrame->bShowHScroll);
		xgeXuiScrollBarSetRange(&pFrame->tHScrollBar, 0.0f, fMaxX, pFrame->tViewportRect.fW);
		xgeXuiScrollBarSetValue(&pFrame->tHScrollBar, pFrame->pModel->fScrollX);
	}
	if ( pFrame->pVScrollWidget != NULL ) {
		xgeXuiWidgetSetVisible(pFrame->pVScrollWidget, pFrame->bShowVScroll);
		xgeXuiScrollBarSetRange(&pFrame->tVScrollBar, 0.0f, fMaxY, pFrame->tViewportRect.fH);
		xgeXuiScrollBarSetValue(&pFrame->tVScrollBar, pFrame->pModel->fScrollY);
	}
	if ( pFrame->pCornerWidget != NULL ) {
		xgeXuiWidgetSetVisible(pFrame->pCornerWidget, pFrame->bShowCorner);
	}
}

static void __xgeXuiScrollFrameHChanged(xge_xui_widget pWidget, float fValue, void* pUser)
{
	xge_xui_scroll_frame pFrame;

	(void)pWidget;
	pFrame = (xge_xui_scroll_frame)pUser;
	if ( (pFrame == NULL) || (pFrame->pModel == NULL) ) {
		return;
	}
	if ( xgeXuiScrollModelSetOffset(pFrame->pModel, fValue, pFrame->pModel->fScrollY) ) {
		__xgeXuiScrollFrameSyncBars(pFrame);
		__xgeXuiScrollFrameNotify(pFrame);
	}
}

static void __xgeXuiScrollFrameVChanged(xge_xui_widget pWidget, float fValue, void* pUser)
{
	xge_xui_scroll_frame pFrame;

	(void)pWidget;
	pFrame = (xge_xui_scroll_frame)pUser;
	if ( (pFrame == NULL) || (pFrame->pModel == NULL) ) {
		return;
	}
	if ( xgeXuiScrollModelSetOffset(pFrame->pModel, pFrame->pModel->fScrollX, fValue) ) {
		__xgeXuiScrollFrameSyncBars(pFrame);
		__xgeXuiScrollFrameNotify(pFrame);
	}
}

static void __xgeXuiScrollFrameInitChildWidget(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeXuiWidgetSetBorder(pWidget, 0.0f, 0);
}

int xgeXuiScrollFrameInit(xge_xui_scroll_frame pFrame, xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_scroll_model pModel)
{
	const xge_xui_theme_t* pTheme;

	if ( (pFrame == NULL) || (pContext == NULL) || (pWidget == NULL) || (pModel == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pFrame, 0, sizeof(*pFrame));
	pFrame->pViewportWidget = xgeXuiWidgetCreate();
	pFrame->pHScrollWidget = xgeXuiWidgetCreate();
	pFrame->pVScrollWidget = xgeXuiWidgetCreate();
	pFrame->pCornerWidget = xgeXuiWidgetCreate();
	if ( (pFrame->pViewportWidget == NULL) || (pFrame->pHScrollWidget == NULL) || (pFrame->pVScrollWidget == NULL) || (pFrame->pCornerWidget == NULL) ) {
		xgeXuiWidgetFree(pFrame->pViewportWidget);
		xgeXuiWidgetFree(pFrame->pHScrollWidget);
		xgeXuiWidgetFree(pFrame->pVScrollWidget);
		xgeXuiWidgetFree(pFrame->pCornerWidget);
		memset(pFrame, 0, sizeof(*pFrame));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pFrame->pContext = pContext;
	pFrame->pWidget = pWidget;
	pFrame->pModel = pModel;
	pFrame->iScrollbarPolicyX = XGE_XUI_SCROLLBAR_POLICY_AUTO;
	pFrame->iScrollbarPolicyY = XGE_XUI_SCROLLBAR_POLICY_AUTO;
	pFrame->iScrollbarMode = XGE_XUI_SCROLLBAR_MODE_COMPACT;
	pFrame->iWheelAxis = XGE_XUI_WHEEL_AXIS_VERTICAL;
	pFrame->iCornerMode = XGE_XUI_SCROLL_FRAME_CORNER_AUTO;
	pFrame->fScrollbarSize = 16.0f;
	pFrame->fWheelStep = 48.0f;
	pTheme = xgeXuiGetTheme(pContext);
	pFrame->iCornerColor = XGE_COLOR_RGBA(248, 251, 255, 255);
	pFrame->iCornerGripColor = (pTheme != NULL) ? pTheme->iTextColor : XGE_COLOR_RGBA(104, 132, 158, 255);
	__xgeXuiViewportWidgetInit(pFrame->pViewportWidget, 0);
	xgeXuiWidgetSetClip(pFrame->pViewportWidget, 1);
	__xgeXuiScrollFrameInitChildWidget(pFrame->pViewportWidget);
	if ( xgeXuiScrollBarInit(&pFrame->tHScrollBar, pContext, pFrame->pHScrollWidget) != XGE_OK ||
	     xgeXuiScrollBarInit(&pFrame->tVScrollBar, pContext, pFrame->pVScrollWidget) != XGE_OK ) {
		xgeXuiScrollFrameUnit(pFrame);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiScrollBarSetMode(&pFrame->tHScrollBar, pFrame->iScrollbarMode);
	xgeXuiScrollBarSetMode(&pFrame->tVScrollBar, pFrame->iScrollbarMode);
	xgeXuiScrollBarSetOrientation(&pFrame->tHScrollBar, XGE_XUI_SEPARATOR_HORIZONTAL);
	xgeXuiScrollBarSetOrientation(&pFrame->tVScrollBar, XGE_XUI_SEPARATOR_VERTICAL);
	xgeXuiScrollBarSetChange(&pFrame->tHScrollBar, __xgeXuiScrollFrameHChanged, pFrame);
	xgeXuiScrollBarSetChange(&pFrame->tVScrollBar, __xgeXuiScrollFrameVChanged, pFrame);
	__xgeXuiControlWidgetInit(pFrame->pCornerWidget, 0);
	xgeXuiWidgetSetHitTestVisible(pFrame->pCornerWidget, 0);
	__xgeXuiScrollFrameInitChildWidget(pFrame->pCornerWidget);
	xgeXuiWidgetSetPaint(pFrame->pCornerWidget, xgeXuiScrollFramePaintProc, pFrame);
	if ( xgeXuiWidgetAddInternal(pWidget, pFrame->pViewportWidget) != XGE_OK ||
	     xgeXuiWidgetAddInternal(pWidget, pFrame->pHScrollWidget) != XGE_OK ||
	     xgeXuiWidgetAddInternal(pWidget, pFrame->pVScrollWidget) != XGE_OK ||
	     xgeXuiWidgetAddInternal(pWidget, pFrame->pCornerWidget) != XGE_OK ) {
		xgeXuiScrollFrameUnit(pFrame);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiScrollFrameLayout(pFrame);
	return XGE_OK;
}

void xgeXuiScrollFrameUnit(xge_xui_scroll_frame pFrame)
{
	if ( pFrame == NULL ) {
		return;
	}
	if ( (pFrame->pContext != NULL) && (pFrame->pWidget != NULL) && (xgeXuiGetPointerCapture(pFrame->pContext, 0) == pFrame->pWidget) ) {
		xgeXuiSetPointerCapture(pFrame->pContext, 0, NULL);
	}
	xgeXuiScrollBarUnit(&pFrame->tHScrollBar);
	xgeXuiScrollBarUnit(&pFrame->tVScrollBar);
	xgeXuiWidgetFree(pFrame->pCornerWidget);
	xgeXuiWidgetFree(pFrame->pVScrollWidget);
	xgeXuiWidgetFree(pFrame->pHScrollWidget);
	xgeXuiWidgetFree(pFrame->pViewportWidget);
	memset(pFrame, 0, sizeof(*pFrame));
}

xge_xui_widget xgeXuiScrollFrameGetViewportWidget(xge_xui_scroll_frame pFrame)
{
	return (pFrame != NULL) ? pFrame->pViewportWidget : NULL;
}

xge_xui_widget xgeXuiScrollFrameGetHScrollBarWidget(xge_xui_scroll_frame pFrame)
{
	return (pFrame != NULL) ? pFrame->pHScrollWidget : NULL;
}

xge_xui_widget xgeXuiScrollFrameGetVScrollBarWidget(xge_xui_scroll_frame pFrame)
{
	return (pFrame != NULL) ? pFrame->pVScrollWidget : NULL;
}

xge_xui_widget xgeXuiScrollFrameGetCornerWidget(xge_xui_scroll_frame pFrame)
{
	return (pFrame != NULL) ? pFrame->pCornerWidget : NULL;
}

xge_rect_t xgeXuiScrollFrameGetViewportRect(xge_xui_scroll_frame pFrame)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( pFrame != NULL ) {
		tRect = pFrame->tViewportRect;
	}
	return tRect;
}

void xgeXuiScrollFrameSetChange(xge_xui_scroll_frame pFrame, xge_xui_scroll_frame_change_proc procChange, void* pUser)
{
	if ( pFrame == NULL ) {
		return;
	}
	pFrame->procChange = procChange;
	pFrame->pUser = pUser;
}

int xgeXuiScrollFrameSetContentSize(xge_xui_scroll_frame pFrame, float fWidth, float fHeight)
{
	int bChanged;

	if ( (pFrame == NULL) || (pFrame->pModel == NULL) ) {
		return 0;
	}
	bChanged = xgeXuiScrollModelSetContentSize(pFrame->pModel, fWidth, fHeight);
	xgeXuiScrollFrameLayout(pFrame);
	if ( bChanged ) {
		__xgeXuiScrollFrameNotify(pFrame);
	}
	return bChanged;
}

int xgeXuiScrollFrameSetOffset(xge_xui_scroll_frame pFrame, float fX, float fY)
{
	int bChanged;

	if ( (pFrame == NULL) || (pFrame->pModel == NULL) ) {
		return 0;
	}
	bChanged = xgeXuiScrollModelSetOffset(pFrame->pModel, fX, fY);
	__xgeXuiScrollFrameSyncBars(pFrame);
	if ( bChanged ) {
		__xgeXuiScrollFrameNotify(pFrame);
	}
	return bChanged;
}

int xgeXuiScrollFrameScrollBy(xge_xui_scroll_frame pFrame, float fDX, float fDY)
{
	if ( (pFrame == NULL) || (pFrame->pModel == NULL) ) {
		return 0;
	}
	return xgeXuiScrollFrameSetOffset(pFrame, pFrame->pModel->fScrollX + fDX, pFrame->pModel->fScrollY + fDY);
}

void xgeXuiScrollFrameGetOffset(xge_xui_scroll_frame pFrame, float* pX, float* pY)
{
	xgeXuiScrollModelGetOffset((pFrame != NULL) ? pFrame->pModel : NULL, pX, pY);
}

void xgeXuiScrollFrameSetScrollbarPolicy(xge_xui_scroll_frame pFrame, int iPolicyX, int iPolicyY)
{
	if ( pFrame == NULL ) {
		return;
	}
	pFrame->iScrollbarPolicyX = __xgeXuiScrollFramePolicyNormalize(iPolicyX);
	pFrame->iScrollbarPolicyY = __xgeXuiScrollFramePolicyNormalize(iPolicyY);
	xgeXuiScrollFrameLayout(pFrame);
}

void xgeXuiScrollFrameSetScrollbarMode(xge_xui_scroll_frame pFrame, int iMode)
{
	if ( pFrame == NULL ) {
		return;
	}
	pFrame->iScrollbarMode = __xgeXuiScrollFrameModeNormalize(iMode);
	xgeXuiScrollBarSetMode(&pFrame->tHScrollBar, pFrame->iScrollbarMode);
	xgeXuiScrollBarSetMode(&pFrame->tVScrollBar, pFrame->iScrollbarMode);
	xgeXuiScrollFrameLayout(pFrame);
}

int xgeXuiScrollFrameGetScrollbarMode(xge_xui_scroll_frame pFrame)
{
	return (pFrame != NULL) ? pFrame->iScrollbarMode : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

void xgeXuiScrollFrameSetWheelAxis(xge_xui_scroll_frame pFrame, int iAxis)
{
	if ( pFrame == NULL ) {
		return;
	}
	pFrame->iWheelAxis = __xgeXuiScrollFrameWheelAxisNormalize(iAxis);
}

int xgeXuiScrollFrameGetWheelAxis(xge_xui_scroll_frame pFrame)
{
	return (pFrame != NULL) ? pFrame->iWheelAxis : XGE_XUI_WHEEL_AXIS_VERTICAL;
}

void xgeXuiScrollFrameSetWheelStep(xge_xui_scroll_frame pFrame, float fStep)
{
	if ( pFrame == NULL ) {
		return;
	}
	pFrame->fWheelStep = (fStep > 0.0f) ? fStep : 48.0f;
}

void xgeXuiScrollFrameSetContentDragEnabled(xge_xui_scroll_frame pFrame, int bEnabled)
{
	if ( pFrame == NULL ) {
		return;
	}
	pFrame->bContentDragEnabled = bEnabled ? 1 : 0;
	if ( pFrame->bContentDragEnabled == 0 ) {
		pFrame->bDraggingContent = 0;
	}
}

int xgeXuiScrollFrameIsContentDragEnabled(xge_xui_scroll_frame pFrame)
{
	return (pFrame != NULL) ? pFrame->bContentDragEnabled : 0;
}

void xgeXuiScrollFrameSetCornerMode(xge_xui_scroll_frame pFrame, int iMode)
{
	if ( pFrame == NULL ) {
		return;
	}
	pFrame->iCornerMode = __xgeXuiScrollFrameCornerModeNormalize(iMode);
	xgeXuiScrollFrameLayout(pFrame);
}

void xgeXuiScrollFrameSetMetrics(xge_xui_scroll_frame pFrame, float fScrollbarSize, float fMinThumbSize, float fThumbRadius, float fButtonSize)
{
	if ( pFrame == NULL ) {
		return;
	}
	pFrame->fScrollbarSize = (fScrollbarSize > 0.0f) ? fScrollbarSize : 16.0f;
	xgeXuiScrollBarSetMetrics(&pFrame->tHScrollBar, pFrame->fScrollbarSize, fMinThumbSize, fThumbRadius, fButtonSize);
	xgeXuiScrollBarSetMetrics(&pFrame->tVScrollBar, pFrame->fScrollbarSize, fMinThumbSize, fThumbRadius, fButtonSize);
	xgeXuiScrollFrameLayout(pFrame);
}

void xgeXuiScrollFrameSetColors(xge_xui_scroll_frame pFrame, uint32_t iTrack, uint32_t iThumb, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled)
{
	if ( pFrame == NULL ) {
		return;
	}
	xgeXuiScrollBarSetColors(&pFrame->tHScrollBar, iTrack, iThumb, iHover, iActive, iFocus, iDisabled);
	xgeXuiScrollBarSetColors(&pFrame->tVScrollBar, iTrack, iThumb, iHover, iActive, iFocus, iDisabled);
}

void xgeXuiScrollFrameSetButtonColors(xge_xui_scroll_frame pFrame, uint32_t iButton, uint32_t iIcon)
{
	if ( pFrame == NULL ) {
		return;
	}
	xgeXuiScrollBarSetButtonColors(&pFrame->tHScrollBar, iButton, iIcon);
	xgeXuiScrollBarSetButtonColors(&pFrame->tVScrollBar, iButton, iIcon);
}

void xgeXuiScrollFrameSetCornerColors(xge_xui_scroll_frame pFrame, uint32_t iCorner, uint32_t iGrip)
{
	if ( pFrame == NULL ) {
		return;
	}
	pFrame->iCornerColor = iCorner;
	pFrame->iCornerGripColor = iGrip;
	xgeXuiWidgetMarkPaint(pFrame->pCornerWidget);
}

void xgeXuiScrollFrameLayout(xge_xui_scroll_frame pFrame)
{
	xge_rect_t tFrame;
	xge_rect_t tViewport;
	xge_rect_t tHBar;
	xge_rect_t tVBar;
	xge_rect_t tCorner;
	float fBar;
	float fBorderOverlap;
	float fOuterExpand;
	int bNeedH;
	int bNeedV;
	int bNextH;
	int bNextV;
	int i;
	int bOffsetChanged;

	if ( (pFrame == NULL) || (pFrame->pWidget == NULL) || (pFrame->pModel == NULL) ) {
		return;
	}
	tFrame = pFrame->pWidget->tContentRect;
	if ( (tFrame.fW <= 0.0f) && (tFrame.fH <= 0.0f) ) {
		tFrame = pFrame->pWidget->tRect;
	}
	if ( tFrame.fW < 0.0f ) {
		tFrame.fW = 0.0f;
	}
	if ( tFrame.fH < 0.0f ) {
		tFrame.fH = 0.0f;
	}
	fBar = __xgeXuiScrollFrameScrollbarReserveSize(pFrame);
	fBorderOverlap = 1.0f;
	fOuterExpand = (pFrame->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? fBorderOverlap : 0.0f;
	bNeedH = (pFrame->iScrollbarPolicyX == XGE_XUI_SCROLLBAR_POLICY_ALWAYS);
	bNeedV = (pFrame->iScrollbarPolicyY == XGE_XUI_SCROLLBAR_POLICY_ALWAYS);
	for ( i = 0; i < 3; i++ ) {
		tViewport = tFrame;
		if ( bNeedV ) {
			tViewport.fW -= fBar;
		}
		if ( bNeedH ) {
			tViewport.fH -= fBar;
		}
		if ( tViewport.fW < 0.0f ) {
			tViewport.fW = 0.0f;
		}
		if ( tViewport.fH < 0.0f ) {
			tViewport.fH = 0.0f;
		}
		bNextH = (pFrame->iScrollbarPolicyX == XGE_XUI_SCROLLBAR_POLICY_ALWAYS) ||
		         ((pFrame->iScrollbarPolicyX == XGE_XUI_SCROLLBAR_POLICY_AUTO) && (pFrame->pModel->fContentW > tViewport.fW));
		bNextV = (pFrame->iScrollbarPolicyY == XGE_XUI_SCROLLBAR_POLICY_ALWAYS) ||
		         ((pFrame->iScrollbarPolicyY == XGE_XUI_SCROLLBAR_POLICY_AUTO) && (pFrame->pModel->fContentH > tViewport.fH));
		if ( (bNextH == bNeedH) && (bNextV == bNeedV) ) {
			break;
		}
		bNeedH = bNextH;
		bNeedV = bNextV;
	}
	tViewport = tFrame;
	if ( bNeedV ) {
		tViewport.fW -= fBar;
	}
	if ( bNeedH ) {
		tViewport.fH -= fBar;
	}
	if ( tViewport.fW < 0.0f ) {
		tViewport.fW = 0.0f;
	}
	if ( tViewport.fH < 0.0f ) {
		tViewport.fH = 0.0f;
	}
	tHBar = (xge_rect_t){ tViewport.fX - (bNeedH ? fBorderOverlap : 0.0f), tViewport.fY + tViewport.fH, tViewport.fW + (bNeedH ? fBorderOverlap * 2.0f : 0.0f), bNeedH ? (fBar + fOuterExpand) : 0.0f };
	tVBar = (xge_rect_t){ tViewport.fX + tViewport.fW, tViewport.fY - (bNeedV ? fBorderOverlap : 0.0f), bNeedV ? (fBar + fOuterExpand) : 0.0f, tViewport.fH + (bNeedV ? fBorderOverlap * 2.0f : 0.0f) };
	tCorner = (xge_rect_t){ tViewport.fX + tViewport.fW, tViewport.fY + tViewport.fH, bNeedV ? (fBar + fOuterExpand) : 0.0f, bNeedH ? (fBar + fOuterExpand) : 0.0f };
	pFrame->tFrameRect = tFrame;
	pFrame->tViewportRect = tViewport;
	pFrame->tHScrollRect = tHBar;
	pFrame->tVScrollRect = tVBar;
	pFrame->tCornerRect = tCorner;
	pFrame->bShowHScroll = bNeedH;
	pFrame->bShowVScroll = bNeedV;
	pFrame->bShowCorner = bNeedH && bNeedV && (pFrame->iCornerMode != XGE_XUI_SCROLL_FRAME_CORNER_NONE);
	__xgeXuiScrollFrameArrangeWidget(pFrame->pViewportWidget, tViewport);
	__xgeXuiScrollFrameArrangeWidget(pFrame->pHScrollWidget, tHBar);
	__xgeXuiScrollFrameArrangeWidget(pFrame->pVScrollWidget, tVBar);
	__xgeXuiScrollFrameArrangeWidget(pFrame->pCornerWidget, tCorner);
	bOffsetChanged = xgeXuiScrollModelSetViewport(pFrame->pModel, tViewport);
	__xgeXuiScrollFrameSyncBars(pFrame);
	if ( bOffsetChanged ) {
		__xgeXuiScrollFrameNotify(pFrame);
	}
}

int xgeXuiScrollFrameEvent(xge_xui_scroll_frame pFrame, const xge_event_t* pEvent)
{
	float fDX;
	float fDY;
	int bInsideViewport;
	int bCaptured;

	if ( (pFrame == NULL) || (pFrame->pWidget == NULL) || (pFrame->pModel == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( ((pFrame->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || ((pFrame->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	bInsideViewport = __xgeXuiRectContains(pFrame->tViewportRect, pEvent->fX, pEvent->fY);
	bCaptured = (pFrame->pContext != NULL) && (xgeXuiGetPointerCapture(pFrame->pContext, pEvent->iPointerId) == pFrame->pWidget);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			if ( bInsideViewport == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			fDX = 0.0f;
			fDY = 0.0f;
			if ( pFrame->iWheelAxis == XGE_XUI_WHEEL_AXIS_HORIZONTAL ) {
				fDX = (pEvent->fDX != 0.0f) ? (-pEvent->fDX * pFrame->fWheelStep) : (-pEvent->fDY * pFrame->fWheelStep);
			} else if ( pFrame->iWheelAxis == XGE_XUI_WHEEL_AXIS_BOTH ) {
				fDX = -pEvent->fDX * pFrame->fWheelStep;
				fDY = -pEvent->fDY * pFrame->fWheelStep;
			} else {
				fDY = -pEvent->fDY * pFrame->fWheelStep;
			}
			return xgeXuiScrollFrameScrollBy(pFrame, fDX, fDY) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( (pFrame->bContentDragEnabled == 0) || (bInsideViewport == 0) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 != XGE_MOUSE_LEFT) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pFrame->bDraggingContent = 1;
			pFrame->fDragX = pEvent->fX;
			pFrame->fDragY = pEvent->fY;
			pFrame->fDragScrollX = pFrame->pModel->fScrollX;
			pFrame->fDragScrollY = pFrame->pModel->fScrollY;
			xgeXuiSetPointerCapture(pFrame->pContext, pEvent->iPointerId, pFrame->pWidget);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( (pFrame->bDraggingContent == 0) || (bCaptured == 0) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiScrollFrameSetOffset(pFrame, pFrame->fDragScrollX - (pEvent->fX - pFrame->fDragX), pFrame->fDragScrollY - (pEvent->fY - pFrame->fDragY));
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			if ( pFrame->bDraggingContent == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pFrame->bDraggingContent = 0;
			if ( bCaptured ) {
				xgeXuiSetPointerCapture(pFrame->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiScrollFrameEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiScrollFrameEvent((xge_xui_scroll_frame)pUser, pEvent);
}

void xgeXuiScrollFrameLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	xgeXuiScrollFrameLayout((xge_xui_scroll_frame)pUser);
}

void xgeXuiScrollFramePaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_scroll_frame pFrame;
	xge_rect_t tRect;
	xge_rect_t tGrip;

	pFrame = (xge_xui_scroll_frame)pUser;
	if ( (pFrame == NULL) || (pWidget == NULL) || (pFrame->bShowCorner == 0) ) {
		return;
	}
	tRect = pWidget->tBorderRect;
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		tRect = pFrame->tCornerRect;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pFrame->iCornerColor) != 0 ) {
		__xgeXuiHostDrawRect(tRect, pFrame->iCornerColor);
	}
	if ( (pFrame->iCornerMode == XGE_XUI_SCROLL_FRAME_CORNER_GRIP) && (XGE_COLOR_GET_A(pFrame->iCornerGripColor) != 0) ) {
		tGrip = tRect;
		tGrip.fX = tRect.fX + tRect.fW - 5.0f;
		tGrip.fY = tRect.fY + tRect.fH - 5.0f;
		tGrip.fW = 2.0f;
		tGrip.fH = 2.0f;
		__xgeXuiHostDrawRect(tGrip, pFrame->iCornerGripColor);
		tGrip.fX -= 4.0f;
		tGrip.fY += 2.0f;
		__xgeXuiHostDrawRect(tGrip, pFrame->iCornerGripColor);
		tGrip.fX += 4.0f;
		tGrip.fY -= 4.0f;
		__xgeXuiHostDrawRect(tGrip, pFrame->iCornerGripColor);
	}
}
