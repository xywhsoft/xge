static int __xgeXuiChildIsAfter(xge_xui_widget pChild, xge_xui_widget pAfter)
{
	xge_xui_widget pIt;

	if ( (pChild == NULL) || (pAfter == NULL) || (pChild->pParent != pAfter->pParent) ) {
		return 0;
	}
	for ( pIt = pAfter->pNextSibling; pIt != NULL; pIt = pIt->pNextSibling ) {
		if ( pIt == pChild ) {
			return 1;
		}
	}
	return 0;
}

static int __xgeXuiChildPaintBefore(xge_xui_widget pA, xge_xui_widget pB)
{
	if ( pB == NULL ) {
		return 1;
	}
	if ( pA == NULL ) {
		return 0;
	}
	if ( pA->tStyle.iZ != pB->tStyle.iZ ) {
		return pA->tStyle.iZ < pB->tStyle.iZ;
	}
	return __xgeXuiChildIsAfter(pB, pA);
}

static int __xgeXuiChildHitBefore(xge_xui_widget pA, xge_xui_widget pB)
{
	if ( pB == NULL ) {
		return 1;
	}
	if ( pA == NULL ) {
		return 0;
	}
	if ( pA->tStyle.iZ != pB->tStyle.iZ ) {
		return pA->tStyle.iZ > pB->tStyle.iZ;
	}
	return __xgeXuiChildIsAfter(pA, pB);
}

static xge_xui_widget __xgeXuiChildNextPaint(xge_xui_widget pParent, xge_xui_widget pAfter)
{
	xge_xui_widget pChild;
	xge_xui_widget pBest;

	pBest = NULL;
	if ( pParent == NULL ) {
		return NULL;
	}
	for ( pChild = pParent->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( (pAfter == NULL) || __xgeXuiChildPaintBefore(pAfter, pChild) ) {
			if ( __xgeXuiChildPaintBefore(pChild, pBest) ) {
				pBest = pChild;
			}
		}
	}
	return pBest;
}

static xge_xui_widget __xgeXuiChildNextHit(xge_xui_widget pParent, xge_xui_widget pAfter)
{
	xge_xui_widget pChild;
	xge_xui_widget pBest;

	pBest = NULL;
	if ( pParent == NULL ) {
		return NULL;
	}
	for ( pChild = pParent->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( (pAfter == NULL) || __xgeXuiChildHitBefore(pAfter, pChild) ) {
			if ( __xgeXuiChildHitBefore(pChild, pBest) ) {
				pBest = pChild;
			}
		}
	}
	return pBest;
}

static int __xgeXuiPaintWidget(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_rect_t tOldClip;
	int iCount;
	int bOldClip;
	int bUseClip;

	if ( (pWidget == NULL) || ((pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) ) {
		return 0;
	}
	bOldClip = g_xge.bClipEnabled;
	tOldClip = g_xge.tClipRect;
	bUseClip = ((pWidget->iFlags & XGE_XUI_WIDGET_CLIP) != 0);
	if ( bUseClip ) {
		__xgeXuiHostClipSet(pWidget->tContentRect);
	}
	iCount = 0;
	if ( XGE_COLOR_GET_A(pWidget->tStyle.iBackgroundColor) != 0 ) {
		if ( pWidget->tStyle.fRadius > 0.0f ) {
			__xgeXuiHostDrawRoundedRect(pWidget->tRect, pWidget->tStyle.iBackgroundColor, pWidget->tStyle.fRadius);
		} else {
			__xgeXuiHostDrawRect(pWidget->tRect, pWidget->tStyle.iBackgroundColor);
		}
		iCount++;
	}
	if ( pWidget->procPaint != NULL ) {
		pWidget->procPaint(pWidget, pWidget->pUser);
		iCount++;
	}
	pWidget->iFlags &= ~XGE_XUI_WIDGET_DIRTY_PAINT;
	for ( pChild = __xgeXuiChildNextPaint(pWidget, NULL); pChild != NULL; pChild = __xgeXuiChildNextPaint(pWidget, pChild) ) {
		iCount += __xgeXuiPaintWidget(pChild);
	}
	if ( bUseClip ) {
		if ( bOldClip ) {
			__xgeXuiHostClipSet(tOldClip);
		} else {
			__xgeXuiHostClipClear();
		}
	}
	return iCount;
}

static int __xgeXuiEventHasPoint(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_MOUSE_WHEEL:
		case XGE_EVENT_TOUCH_BEGIN:
		case XGE_EVENT_TOUCH_MOVE:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			return 1;

		default:
			return 0;
	}
}

static int __xgeXuiRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < (tRect.fX + tRect.fW)) && (fY < (tRect.fY + tRect.fH));
}

static int __xgeXuiWidgetCanFocus(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return 0;
	}
	return ((pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0) && ((pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) != 0) && ((pWidget->iFlags & XGE_XUI_WIDGET_FOCUSABLE) != 0);
}

static xge_xui_widget __xgeXuiHitTestWidget(xge_xui_widget pWidget, float fX, float fY)
{
	xge_xui_widget pChild;
	xge_xui_widget pHit;

	if ( (pWidget == NULL) || ((pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || ((pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		return NULL;
	}
	if ( __xgeXuiRectContains(pWidget->tRect, fX, fY) == 0 ) {
		return NULL;
	}
	for ( pChild = __xgeXuiChildNextHit(pWidget, NULL); pChild != NULL; pChild = __xgeXuiChildNextHit(pWidget, pChild) ) {
		pHit = __xgeXuiHitTestWidget(pChild, fX, fY);
		if ( pHit != NULL ) {
			return pHit;
		}
	}
	return pWidget;
}

static int __xgeXuiDispatchToWidget(xge_xui_widget pWidget, const xge_event_t* pEvent)
{
	int iResult;

	while ( pWidget != NULL ) {
		if ( pWidget->procEvent != NULL ) {
			iResult = pWidget->procEvent(pWidget, pEvent, pWidget->pUser);
			if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
		}
		pWidget = pWidget->pParent;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static int __xgeXuiDispatchCaptureToWidget(xge_xui_widget pTarget, const xge_event_t* pEvent)
{
	xge_xui_widget pStack[64];
	xge_xui_widget pWidget;
	int iCount;
	int i;
	int iResult;

	if ( (pTarget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iCount = 0;
	for ( pWidget = pTarget; pWidget != NULL && iCount < 64; pWidget = pWidget->pParent ) {
		pStack[iCount] = pWidget;
		iCount++;
	}
	for ( i = iCount - 1; i >= 0; i-- ) {
		pWidget = pStack[i];
		if ( pWidget->procCaptureEvent != NULL ) {
			iResult = pWidget->procCaptureEvent(pWidget, pEvent, pWidget->pUser);
			if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
		}
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static void __xgeXuiDispatchFocusEvent(xge_xui_widget pWidget, int iType)
{
	xge_event_t tEvent;

	if ( (pWidget == NULL) || (pWidget->procEvent == NULL) ) {
		return;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = iType;
	pWidget->procEvent(pWidget, &tEvent, pWidget->pUser);
}

static void __xgeXuiWidgetSetFlag(xge_xui_widget pWidget, uint32_t iFlag, int bEnabled)
{
	uint32_t iOldFlags;

	if ( pWidget == NULL ) {
		return;
	}
	iOldFlags = pWidget->iFlags;
	if ( bEnabled ) {
		pWidget->iFlags |= iFlag;
	} else {
		pWidget->iFlags &= ~iFlag;
	}
	if ( iOldFlags != pWidget->iFlags ) {
		xgeXuiWidgetMarkLayout(pWidget);
		xgeXuiWidgetMarkPaint(pWidget);
	}
}

static void __xgeXuiButtonSetState(xge_xui_button pButton, int iState)
{
	if ( pButton == NULL ) {
		return;
	}
	if ( (pButton->pWidget == NULL) || ((pButton->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pButton->pContext != NULL && pButton->pContext->pFocus == pButton->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pButton->iState != iState ) {
		pButton->iState = iState;
		xgeXuiWidgetMarkPaint(pButton->pWidget);
	}
}

static uint32_t __xgeXuiButtonColor(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	if ( (pButton->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return pButton->iColorDisabled;
	}
	if ( (pButton->iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		return pButton->iColorActive;
	}
	if ( (pButton->iState & XGE_XUI_STATE_HOVER) != 0 ) {
		return pButton->iColorHover;
	}
	if ( (pButton->iState & XGE_XUI_STATE_FOCUS) != 0 ) {
		return pButton->iColorFocus;
	}
	return pButton->iColorNormal;
}

static void __xgeXuiToggleSetState(xge_xui_toggle pToggle, int iState)
{
	if ( pToggle == NULL ) {
		return;
	}
	if ( (pToggle->pWidget == NULL) || ((pToggle->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pToggle->pContext != NULL && pToggle->pContext->pFocus == pToggle->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pToggle->iState != iState ) {
		pToggle->iState = iState;
		xgeXuiWidgetMarkPaint(pToggle->pWidget);
	}
}

static uint32_t __xgeXuiToggleColor(xge_xui_toggle pToggle)
{
	if ( pToggle == NULL ) {
		return XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	if ( (pToggle->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return pToggle->iColorDisabled;
	}
	if ( (pToggle->iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		return pToggle->iColorActive;
	}
	if ( (pToggle->iState & XGE_XUI_STATE_HOVER) != 0 ) {
		return pToggle->iColorHover;
	}
	if ( (pToggle->iState & XGE_XUI_STATE_FOCUS) != 0 ) {
		return pToggle->iColorFocus;
	}
	return pToggle->iColorNormal;
}

static float __xgeXuiClampFloat(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

static void __xgeXuiSliderSetState(xge_xui_slider pSlider, int iState)
{
	if ( pSlider == NULL ) {
		return;
	}
	if ( (pSlider->pWidget == NULL) || ((pSlider->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pSlider->pContext != NULL && pSlider->pContext->pFocus == pSlider->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pSlider->iState != iState ) {
		pSlider->iState = iState;
		xgeXuiWidgetMarkPaint(pSlider->pWidget);
	}
}

static float __xgeXuiSliderRate(xge_xui_slider pSlider)
{
	if ( (pSlider == NULL) || (pSlider->fMax == pSlider->fMin) ) {
		return 0.0f;
	}
	return __xgeXuiClampFloat((pSlider->fValue - pSlider->fMin) / (pSlider->fMax - pSlider->fMin), 0.0f, 1.0f);
}

static int __xgeXuiSliderSetValueInternal(xge_xui_slider pSlider, float fValue, int bNotify)
{
	if ( pSlider == NULL ) {
		return 0;
	}
	fValue = __xgeXuiClampFloat(fValue, pSlider->fMin, pSlider->fMax);
	if ( pSlider->fValue == fValue ) {
		return 0;
	}
	pSlider->fValue = fValue;
	pSlider->iChangeCount++;
	xgeXuiWidgetMarkPaint(pSlider->pWidget);
	if ( bNotify && (pSlider->procChange != NULL) ) {
		pSlider->procChange(pSlider->pWidget, pSlider->fValue, pSlider->pUser);
	}
	return 1;
}

static int __xgeXuiSliderSetValueFromPoint(xge_xui_slider pSlider, float fX, int bNotify)
{
	xge_rect_t tRect;
	float fRate;

	if ( (pSlider == NULL) || (pSlider->pWidget == NULL) ) {
		return 0;
	}
	tRect = pSlider->pWidget->tContentRect;
	if ( tRect.fW <= 0.0f ) {
		tRect = pSlider->pWidget->tRect;
	}
	if ( tRect.fW <= 0.0f ) {
		return 0;
	}
	fRate = __xgeXuiClampFloat((fX - tRect.fX) / tRect.fW, 0.0f, 1.0f);
	return __xgeXuiSliderSetValueInternal(pSlider, pSlider->fMin + (pSlider->fMax - pSlider->fMin) * fRate, bNotify);
}

static float __xgeXuiProgressRate(xge_xui_progress pProgress)
{
	if ( (pProgress == NULL) || (pProgress->fMax == pProgress->fMin) ) {
		return 0.0f;
	}
	return __xgeXuiClampFloat((pProgress->fValue - pProgress->fMin) / (pProgress->fMax - pProgress->fMin), 0.0f, 1.0f);
}

static void __xgeXuiProgressSetValueInternal(xge_xui_progress pProgress, float fValue)
{
	if ( pProgress == NULL ) {
		return;
	}
	fValue = __xgeXuiClampFloat(fValue, pProgress->fMin, pProgress->fMax);
	if ( pProgress->fValue != fValue ) {
		pProgress->fValue = fValue;
		xgeXuiWidgetMarkPaint(pProgress->pWidget);
	}
}

static void __xgeXuiScrollViewClamp(xge_xui_scroll_view pScroll)
{
	float fMaxX;
	float fMaxY;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) ) {
		return;
	}
	fMaxX = pScroll->fContentW - pScroll->pWidget->tContentRect.fW;
	fMaxY = pScroll->fContentH - pScroll->pWidget->tContentRect.fH;
	if ( fMaxX < 0.0f ) {
		fMaxX = 0.0f;
	}
	if ( fMaxY < 0.0f ) {
		fMaxY = 0.0f;
	}
	pScroll->fScrollX = __xgeXuiClampFloat(pScroll->fScrollX, 0.0f, fMaxX);
	pScroll->fScrollY = __xgeXuiClampFloat(pScroll->fScrollY, 0.0f, fMaxY);
}

static void __xgeXuiScrollViewSetOffsetInternal(xge_xui_scroll_view pScroll, float fX, float fY)
{
	float fOldX;
	float fOldY;

	if ( pScroll == NULL ) {
		return;
	}
	fOldX = pScroll->fScrollX;
	fOldY = pScroll->fScrollY;
	pScroll->fScrollX = fX;
	pScroll->fScrollY = fY;
	__xgeXuiScrollViewClamp(pScroll);
	if ( (fOldX != pScroll->fScrollX) || (fOldY != pScroll->fScrollY) ) {
		xgeXuiWidgetMarkLayout(pScroll->pWidget);
		xgeXuiWidgetMarkPaint(pScroll->pWidget);
	}
}

static float __xgeXuiListViewMaxScroll(xge_xui_list_view pList)
{
	float fMax;

	if ( (pList == NULL) || (pList->pWidget == NULL) ) {
		return 0.0f;
	}
	fMax = (float)pList->iItemCount * pList->fItemHeight - pList->pWidget->tContentRect.fH;
	return (fMax > 0.0f) ? fMax : 0.0f;
}

static void __xgeXuiListViewClamp(xge_xui_list_view pList)
{
	if ( pList == NULL ) {
		return;
	}
	pList->fScrollY = __xgeXuiClampFloat(pList->fScrollY, 0.0f, __xgeXuiListViewMaxScroll(pList));
}

static int __xgeXuiListViewIndexAt(xge_xui_list_view pList, float fY)
{
	int iIndex;

	if ( (pList == NULL) || (pList->pWidget == NULL) || (pList->fItemHeight <= 0.0f) ) {
		return -1;
	}
	iIndex = (int)((fY - pList->pWidget->tContentRect.fY + pList->fScrollY) / pList->fItemHeight);
	if ( (iIndex < 0) || (iIndex >= pList->iItemCount) ) {
		return -1;
	}
	return iIndex;
}

xge_xui_size_t xgeXuiSizePx(float fValue)
{
	xge_xui_size_t tSize;

	tSize.fValue = fValue;
	tSize.iUnit = XGE_XUI_SIZE_PX;
	return tSize;
}

xge_xui_size_t xgeXuiSizeDip(float fValue)
{
	xge_xui_size_t tSize;

	tSize.fValue = fValue;
	tSize.iUnit = XGE_XUI_SIZE_DIP;
	return tSize;
}

xge_xui_size_t xgeXuiSizePercent(float fValue)
{
	xge_xui_size_t tSize;

	tSize.fValue = fValue;
	tSize.iUnit = XGE_XUI_SIZE_PERCENT;
	return tSize;
}

xge_xui_size_t xgeXuiSizeGrow(float fValue)
{
	xge_xui_size_t tSize;

	tSize.fValue = fValue;
	tSize.iUnit = XGE_XUI_SIZE_GROW;
	return tSize;
}

xge_xui_size_t xgeXuiSizeContent(void)
{
	xge_xui_size_t tSize;

	tSize.fValue = 0.0f;
	tSize.iUnit = XGE_XUI_SIZE_CONTENT;
	return tSize;
}

void xgeXuiThemeDefault(xge_xui_theme pTheme)
{
	if ( pTheme == NULL ) {
		return;
	}
	memset(pTheme, 0, sizeof(*pTheme));
	pTheme->pFont = NULL;
	pTheme->iTextColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pTheme->iBackgroundColor = XGE_COLOR_RGBA(248, 250, 252, 255);
	pTheme->iPanelColor = XGE_COLOR_RGBA(32, 38, 46, 255);
	pTheme->iBorderColor = XGE_COLOR_RGBA(92, 100, 112, 255);
	pTheme->iAccentColor = XGE_COLOR_RGBA(62, 172, 110, 255);
	pTheme->iSelectionColor = XGE_COLOR_RGBA(80, 140, 220, 120);
	pTheme->iStateNormal = XGE_COLOR_RGBA(48, 64, 82, 255);
	pTheme->iStateHover = XGE_COLOR_RGBA(62, 82, 104, 255);
	pTheme->iStateActive = XGE_COLOR_RGBA(34, 48, 64, 255);
	pTheme->iStateFocus = XGE_COLOR_RGBA(54, 72, 96, 255);
	pTheme->iStateDisabled = XGE_COLOR_RGBA(68, 68, 68, 160);
	pTheme->fRadius = 4.0f;
	pTheme->fPadding = 4.0f;
	pTheme->fSpacing = 4.0f;
	pTheme->fBorderWidth = 1.0f;
}

void xgeXuiSetTheme(xge_xui_context pContext, const xge_xui_theme_t* pTheme)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	if ( pTheme != NULL ) {
		pContext->tTheme = *pTheme;
	} else {
		xgeXuiThemeDefault(&pContext->tTheme);
	}
	xgeXuiWidgetMarkLayout(pContext->pRoot);
	xgeXuiWidgetMarkPaint(pContext->pRoot);
}

void xgeXuiRefreshRequest(xge_xui_context pContext)
{
	const xge_xui_host_t* pHost;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	if ( pContext->bRefreshRequested != 0 ) {
		return;
	}
	pContext->bRefreshRequested = 1;
	pHost = xgeXuiGetHost(pContext);
	if ( (pHost != NULL) && (pHost->request_refresh != NULL) ) {
		pHost->request_refresh(pHost->pUser);
	}
}

int xgeXuiRefreshNeeded(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return 0;
	}
	return pContext->bRefreshRequested ? 1 : 0;
}

void xgeXuiRefreshClear(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	pContext->bRefreshRequested = 0;
}

const xge_xui_theme_t* xgeXuiGetTheme(xge_xui_context pContext)
{
	static xge_xui_theme_t tDefaultTheme;
	static int bDefaultThemeInitialized = 0;

	if ( (pContext != NULL) && (pContext->bInitialized != 0) ) {
		return &pContext->tTheme;
	}
	if ( bDefaultThemeInitialized == 0 ) {
		xgeXuiThemeDefault(&tDefaultTheme);
		bDefaultThemeInitialized = 1;
	}
	return &tDefaultTheme;
}

int xgeXuiInit(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pContext->bInitialized ) {
		return XGE_ERROR_ALREADY_INITIALIZED;
	}
	memset(pContext, 0, sizeof(*pContext));
	pContext->pRoot = __xgeXuiWidgetAlloc();
	if ( pContext->pRoot == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pContext->fDipScale = 1.0f;
	xgeXuiThemeDefault(&pContext->tTheme);
	pContext->pRoot->tRect.fW = (float)xgeGetWidth();
	pContext->pRoot->tRect.fH = (float)xgeGetHeight();
	pContext->pRoot->pInternal = pContext;
	pContext->pHost = &g_xgeXuiDefaultHost;
	pContext->bInitialized = 1;
	pContext->iDirtyLayoutCount = 1;
	pContext->iDirtyPaintCount = 1;
	return XGE_OK;
}

void xgeXuiUnit(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	__xgeXuiWidgetFreeTree(pContext->pRoot);
	memset(pContext, 0, sizeof(*pContext));
}

xge_xui_widget xgeXuiRoot(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return NULL;
	}
	return pContext->pRoot;
}

void xgeXuiSetDipScale(xge_xui_context pContext, float fScale)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	if ( fScale <= 0.0f ) {
		fScale = 1.0f;
	}
	if ( pContext->fDipScale != fScale ) {
		pContext->fDipScale = fScale;
		xgeXuiWidgetMarkLayout(pContext->pRoot);
		xgeXuiWidgetMarkPaint(pContext->pRoot);
	}
}

float xgeXuiGetDipScale(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pContext->fDipScale <= 0.0f) ) {
		return 1.0f;
	}
	return pContext->fDipScale;
}

void xgeXuiLayoutBatchBegin(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	pContext->iLayoutBatchDepth++;
}

void xgeXuiLayoutBatchEnd(xge_xui_context pContext)
{
	int bRequestRefresh;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pContext->iLayoutBatchDepth <= 0) ) {
		return;
	}
	pContext->iLayoutBatchDepth--;
	if ( pContext->iLayoutBatchDepth == 0 ) {
		bRequestRefresh = (pContext->bLayoutBatchDirtyLayout || pContext->bLayoutBatchDirtyPaint) ? 1 : 0;
		if ( pContext->bLayoutBatchDirtyLayout && pContext->pRoot != NULL ) {
			pContext->pRoot->iFlags |= XGE_XUI_WIDGET_DIRTY_LAYOUT;
		}
		if ( pContext->bLayoutBatchDirtyPaint && pContext->pRoot != NULL ) {
			pContext->pRoot->iFlags |= XGE_XUI_WIDGET_DIRTY_PAINT;
			__xgeXuiWidgetInvalidateRect(pContext->pRoot);
		}
		pContext->bLayoutBatchDirtyLayout = 0;
		pContext->bLayoutBatchDirtyPaint = 0;
		if ( bRequestRefresh ) {
			xgeXuiRefreshRequest(pContext);
		}
	}
}

void xgeXuiSetHost(xge_xui_context pContext, const xge_xui_host_t* pHost)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	pContext->pHost = (pHost != NULL) ? pHost : &g_xgeXuiDefaultHost;
	xgeXuiWidgetMarkPaint(pContext->pRoot);
}

const xge_xui_host_t* xgeXuiGetHost(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pContext->pHost == NULL) ) {
		return &g_xgeXuiDefaultHost;
	}
	return pContext->pHost;
}

xge_xui_widget xgeXuiWidgetCreate(void)
{
	return __xgeXuiWidgetAlloc();
}

void xgeXuiWidgetFree(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return;
	}
	xgeXuiWidgetRemove(pWidget);
	__xgeXuiWidgetFreeTree(pWidget);
}

int xgeXuiWidgetAdd(xge_xui_widget pParent, xge_xui_widget pChild)
{
	if ( (pParent == NULL) || (pChild == NULL) || (pParent == pChild) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiWidgetRemove(pChild);
	pChild->pParent = pParent;
	pChild->pPrevSibling = pParent->pLastChild;
	if ( pParent->pLastChild != NULL ) {
		pParent->pLastChild->pNextSibling = pChild;
	} else {
		pParent->pFirstChild = pChild;
	}
	pParent->pLastChild = pChild;
	xgeXuiWidgetMarkLayout(pParent);
	xgeXuiWidgetMarkPaint(pParent);
	return XGE_OK;
}

void xgeXuiWidgetSetId(xge_xui_widget pWidget, int iId)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->iId = iId;
}

int xgeXuiWidgetGetId(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return 0;
	}
	return pWidget->iId;
}

void xgeXuiWidgetSetName(xge_xui_widget pWidget, const char* sName)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->sName = (sName != NULL) ? sName : "";
}

const char* xgeXuiWidgetGetName(xge_xui_widget pWidget)
{
	if ( (pWidget == NULL) || (pWidget->sName == NULL) ) {
		return "";
	}
	return pWidget->sName;
}

xge_xui_widget xgeXuiWidgetFindById(xge_xui_widget pRoot, int iId)
{
	xge_xui_widget pFound;
	xge_xui_widget pChild;

	if ( pRoot == NULL ) {
		return NULL;
	}
	if ( pRoot->iId == iId ) {
		return pRoot;
	}
	for ( pChild = pRoot->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		pFound = xgeXuiWidgetFindById(pChild, iId);
		if ( pFound != NULL ) {
			return pFound;
		}
	}
	return NULL;
}

xge_xui_widget xgeXuiWidgetFindByName(xge_xui_widget pRoot, const char* sName)
{
	xge_xui_widget pFound;
	xge_xui_widget pChild;

	if ( (pRoot == NULL) || (sName == NULL) ) {
		return NULL;
	}
	if ( (pRoot->sName != NULL) && (strcmp(pRoot->sName, sName) == 0) ) {
		return pRoot;
	}
	for ( pChild = pRoot->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		pFound = xgeXuiWidgetFindByName(pChild, sName);
		if ( pFound != NULL ) {
			return pFound;
		}
	}
	return NULL;
}

void xgeXuiWidgetRemove(xge_xui_widget pWidget)
{
	xge_xui_widget pParent;

	if ( (pWidget == NULL) || (pWidget->pParent == NULL) ) {
		return;
	}
	pParent = pWidget->pParent;
	if ( pWidget->pPrevSibling != NULL ) {
		pWidget->pPrevSibling->pNextSibling = pWidget->pNextSibling;
	} else {
		pParent->pFirstChild = pWidget->pNextSibling;
	}
	if ( pWidget->pNextSibling != NULL ) {
		pWidget->pNextSibling->pPrevSibling = pWidget->pPrevSibling;
	} else {
		pParent->pLastChild = pWidget->pPrevSibling;
	}
	pWidget->pParent = NULL;
	pWidget->pPrevSibling = NULL;
	pWidget->pNextSibling = NULL;
	xgeXuiWidgetMarkLayout(pParent);
	xgeXuiWidgetMarkPaint(pParent);
}

void xgeXuiWidgetSetRect(xge_xui_widget pWidget, xge_rect_t tRect)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tRect = tRect;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

xge_rect_t xgeXuiWidgetGetRect(xge_xui_widget pWidget)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( pWidget != NULL ) {
		tRect = pWidget->tRect;
	}
	return tRect;
}

void xgeXuiWidgetSetStyle(xge_xui_widget pWidget, const xge_xui_style_t* pStyle)
{
	if ( (pWidget == NULL) || (pStyle == NULL) ) {
		return;
	}
	pWidget->tStyle = *pStyle;
	if ( pWidget->tStyle.iGridColumns <= 0 ) {
		pWidget->tStyle.iGridColumns = 1;
	}
	pWidget->tStyle.iAlignX = __xgeXuiAlignClamp(pWidget->tStyle.iAlignX);
	pWidget->tStyle.iAlignY = __xgeXuiAlignClamp(pWidget->tStyle.iAlignY);
	if ( pWidget->tStyle.fRadius < 0.0f ) {
		pWidget->tStyle.fRadius = 0.0f;
	}
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

const xge_xui_style_t* xgeXuiWidgetGetStyle(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return NULL;
	}
	return &pWidget->tStyle;
}

void xgeXuiWidgetSetLayout(xge_xui_widget pWidget, int iLayout)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.iLayout = iLayout;
	xgeXuiWidgetMarkLayout(pWidget);
}

void xgeXuiWidgetSetSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.tWidth = tWidth;
	pWidget->tStyle.tHeight = tHeight;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetMinSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.tMinWidth = tWidth;
	pWidget->tStyle.tMinHeight = tHeight;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetMaxSize(xge_xui_widget pWidget, xge_xui_size_t tWidth, xge_xui_size_t tHeight)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.tMaxWidth = tWidth;
	pWidget->tStyle.tMaxHeight = tHeight;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetGrid(xge_xui_widget pWidget, int iColumns, float fRowHeight, float fColumnGap, float fRowGap)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.iGridColumns = (iColumns > 0) ? iColumns : 1;
	pWidget->tStyle.fGridRowHeight = (fRowHeight > 0.0f) ? fRowHeight : 0.0f;
	pWidget->tStyle.fGridColumnGap = (fColumnGap > 0.0f) ? fColumnGap : 0.0f;
	pWidget->tStyle.fGridRowGap = (fRowGap > 0.0f) ? fRowGap : 0.0f;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetAlign(xge_xui_widget pWidget, int iAlignX, int iAlignY)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.iAlignX = __xgeXuiAlignClamp(iAlignX);
	pWidget->tStyle.iAlignY = __xgeXuiAlignClamp(iAlignY);
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetZ(xge_xui_widget pWidget, int iZ)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.iZ = iZ;
	xgeXuiWidgetMarkPaint((pWidget->pParent != NULL) ? pWidget->pParent : pWidget);
}

int xgeXuiWidgetGetZ(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return 0;
	}
	return pWidget->tStyle.iZ;
}

void xgeXuiWidgetSetAnchorPx(xge_xui_widget pWidget, int iAnchor, float fLeft, float fTop, float fRight, float fBottom)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.iAnchor = iAnchor & (XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_TOP | XGE_XUI_ANCHOR_RIGHT | XGE_XUI_ANCHOR_BOTTOM);
	pWidget->tStyle.tAnchor.tLeft = xgeXuiSizePx(fLeft);
	pWidget->tStyle.tAnchor.tTop = xgeXuiSizePx(fTop);
	pWidget->tStyle.tAnchor.tRight = xgeXuiSizePx(fRight);
	pWidget->tStyle.tAnchor.tBottom = xgeXuiSizePx(fBottom);
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

int xgeXuiWidgetGetAnchor(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return 0;
	}
	return pWidget->tStyle.iAnchor;
}

void xgeXuiWidgetSetMarginPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.tMargin.tLeft = xgeXuiSizePx(fLeft);
	pWidget->tStyle.tMargin.tTop = xgeXuiSizePx(fTop);
	pWidget->tStyle.tMargin.tRight = xgeXuiSizePx(fRight);
	pWidget->tStyle.tMargin.tBottom = xgeXuiSizePx(fBottom);
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetPaddingPx(xge_xui_widget pWidget, float fLeft, float fTop, float fRight, float fBottom)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.tPadding.tLeft = xgeXuiSizePx(fLeft);
	pWidget->tStyle.tPadding.tTop = xgeXuiSizePx(fTop);
	pWidget->tStyle.tPadding.tRight = xgeXuiSizePx(fRight);
	pWidget->tStyle.tPadding.tBottom = xgeXuiSizePx(fBottom);
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetBackground(xge_xui_widget pWidget, uint32_t iColor)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.iBackgroundColor = iColor;
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetRadius(xge_xui_widget pWidget, float fRadius)
{
	if ( pWidget == NULL ) {
		return;
	}
	if ( fRadius < 0.0f ) {
		fRadius = 0.0f;
	}
	pWidget->tStyle.fRadius = fRadius;
	xgeXuiWidgetMarkPaint(pWidget);
}

uint32_t xgeXuiWidgetGetFlags(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return 0;
	}
	return pWidget->iFlags;
}

void xgeXuiWidgetSetVisible(xge_xui_widget pWidget, int bVisible)
{
	__xgeXuiWidgetSetFlag(pWidget, XGE_XUI_WIDGET_VISIBLE, bVisible);
}

void xgeXuiWidgetSetEnabled(xge_xui_widget pWidget, int bEnabled)
{
	__xgeXuiWidgetSetFlag(pWidget, XGE_XUI_WIDGET_ENABLED, bEnabled);
}

void xgeXuiWidgetSetFocusable(xge_xui_widget pWidget, int bFocusable)
{
	__xgeXuiWidgetSetFlag(pWidget, XGE_XUI_WIDGET_FOCUSABLE, bFocusable);
}

void xgeXuiWidgetSetClip(xge_xui_widget pWidget, int bClip)
{
	__xgeXuiWidgetSetFlag(pWidget, XGE_XUI_WIDGET_CLIP, bClip);
}

void xgeXuiWidgetSetCaptureEvent(xge_xui_widget pWidget, xge_xui_event_proc procEvent)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procCaptureEvent = procEvent;
}

void xgeXuiWidgetSetMeasure(xge_xui_widget pWidget, xge_xui_measure_proc procMeasure)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procMeasure = procMeasure;
	xgeXuiWidgetMarkLayout(pWidget);
}

void xgeXuiWidgetSetPaint(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procPaint = procPaint;
	pWidget->pUser = pUser;
	xgeXuiWidgetMarkPaint(pWidget);
}

int xgeXuiWidgetIsVisible(xge_xui_widget pWidget)
{
	return (pWidget != NULL) && ((pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0);
}

int xgeXuiWidgetIsEnabled(xge_xui_widget pWidget)
{
	return (pWidget != NULL) && ((pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) != 0);
}

int xgeXuiWidgetIsFocusable(xge_xui_widget pWidget)
{
	return __xgeXuiWidgetCanFocus(pWidget);
}

void xgeXuiWidgetMarkLayout(xge_xui_widget pWidget)
{
	xge_xui_context pContext;

	pContext = __xgeXuiWidgetContext(pWidget);
	if ( (pContext != NULL) && (pContext->iLayoutBatchDepth > 0) ) {
		pContext->bLayoutBatchDirtyLayout = 1;
		return;
	}
	while ( pWidget != NULL ) {
		pWidget->iFlags |= XGE_XUI_WIDGET_DIRTY_LAYOUT;
		pWidget = pWidget->pParent;
	}
	xgeXuiRefreshRequest(pContext);
}

void xgeXuiWidgetMarkPaint(xge_xui_widget pWidget)
{
	xge_xui_context pContext;
	xge_xui_widget pDirtyWidget;

	pDirtyWidget = pWidget;
	pContext = __xgeXuiWidgetContext(pWidget);
	if ( (pContext != NULL) && (pContext->iLayoutBatchDepth > 0) ) {
		pContext->bLayoutBatchDirtyPaint = 1;
		return;
	}
	while ( pWidget != NULL ) {
		pWidget->iFlags |= XGE_XUI_WIDGET_DIRTY_PAINT;
		pWidget = pWidget->pParent;
	}
	if ( pContext != NULL ) {
		__xgeXuiWidgetInvalidateRect(pDirtyWidget);
	}
	xgeXuiRefreshRequest(pContext);
}

xge_xui_widget xgeXuiHitTest(xge_xui_context pContext, float fX, float fY)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pContext->pRoot == NULL) ) {
		return NULL;
	}
	return __xgeXuiHitTestWidget(pContext->pRoot, fX, fY);
}

void xgeXuiSetFocus(xge_xui_context pContext, xge_xui_widget pWidget)
{
	xge_xui_widget pOldFocus;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	if ( (pWidget != NULL) && (__xgeXuiWidgetCanFocus(pWidget) == 0) ) {
		return;
	}
	if ( pContext->pFocus != pWidget ) {
		pOldFocus = pContext->pFocus;
		__xgeXuiDispatchFocusEvent(pOldFocus, XGE_EVENT_XUI_FOCUS_OUT);
		xgeXuiWidgetMarkPaint(pOldFocus);
		pContext->pFocus = pWidget;
		__xgeXuiDispatchFocusEvent(pWidget, XGE_EVENT_XUI_FOCUS_IN);
		xgeXuiWidgetMarkPaint(pContext->pFocus);
	}
}

void xgeXuiSetCapture(xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	pContext->pCapture = pWidget;
}

int xgeXuiDispatchEvent(xge_xui_context pContext, const xge_event_t* pEvent)
{
	xge_xui_widget pTarget;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pContext->pRoot == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pContext->pCapture != NULL ) {
		pTarget = pContext->pCapture;
	} else if ( __xgeXuiEventHasPoint(pEvent) ) {
		pTarget = xgeXuiHitTest(pContext, pEvent->fX, pEvent->fY);
	} else {
		pTarget = pContext->pFocus;
		if ( pTarget == NULL ) {
			pTarget = pContext->pRoot;
		}
	}
	if ( pTarget == NULL ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( __xgeXuiDispatchCaptureToWidget(pTarget, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_XUI_EVENT_CONSUMED;
	}
	return __xgeXuiDispatchToWidget(pTarget, pEvent);
}

int xgeXuiEventPush(xge_xui_context pContext, const xge_event_t* pEvent)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pEvent == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pContext->iEventCount >= XGE_XUI_EVENT_QUEUE_CAPACITY ) {
		return XGE_ERROR;
	}
	pContext->arrEventQueue[pContext->iEventTail] = *pEvent;
	pContext->iEventTail = (pContext->iEventTail + 1) % XGE_XUI_EVENT_QUEUE_CAPACITY;
	pContext->iEventCount++;
	return XGE_OK;
}

int xgeXuiEventPop(xge_xui_context pContext, xge_event_t* pEvent)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pEvent == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pContext->iEventCount <= 0 ) {
		return XGE_ERROR;
	}
	*pEvent = pContext->arrEventQueue[pContext->iEventHead];
	pContext->iEventHead = (pContext->iEventHead + 1) % XGE_XUI_EVENT_QUEUE_CAPACITY;
	pContext->iEventCount--;
	return XGE_OK;
}

int xgeXuiEventCount(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return 0;
	}
	return pContext->iEventCount;
}

int xgeXuiDispatchQueuedEvents(xge_xui_context pContext)
{
	xge_event_t tEvent;
	int iLimit;
	int iProcessed;
	int i;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iLimit = pContext->iEventCount;
	iProcessed = 0;
	for ( i = 0; i < iLimit; i++ ) {
		if ( xgeXuiEventPop(pContext, &tEvent) != XGE_OK ) {
			break;
		}
		xgeXuiDispatchEvent(pContext, &tEvent);
		iProcessed++;
	}
	return iProcessed;
}

int xgeXuiUpdate(xge_xui_context pContext, float fDelta)
{
	xge_rect_t tRootRect;
	const xge_xui_host_t* pOldHost;
	float fOldDipScale;

	(void)fDelta;
	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pContext->pRoot == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	tRootRect = pContext->pRoot->tRect;
	if ( tRootRect.fW <= 0.0f ) {
		tRootRect.fW = (float)xgeGetWidth();
	}
	if ( tRootRect.fH <= 0.0f ) {
		tRootRect.fH = (float)xgeGetHeight();
	}
	pOldHost = g_xgeXuiActiveHost;
	fOldDipScale = g_fXgeXuiActiveDipScale;
	g_xgeXuiActiveHost = xgeXuiGetHost(pContext);
	g_fXgeXuiActiveDipScale = xgeXuiGetDipScale(pContext);
	__xgeXuiLayoutWidget(pContext->pRoot, tRootRect);
	g_fXgeXuiActiveDipScale = fOldDipScale;
	g_xgeXuiActiveHost = pOldHost;
	pContext->iDirtyLayoutCount = 0;
	return XGE_OK;
}

int xgeXuiPaint(xge_xui_context pContext)
{
	const xge_xui_host_t* pOldHost;
	xge_xui_context pOldContext;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pContext->pRoot == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pContext->pRoot->iFlags & XGE_XUI_WIDGET_DIRTY_PAINT) == 0 ) {
		pContext->iPaintCommandCount = 0;
		pContext->iPaintFlushCount = 0;
		pContext->iDirtyPaintCount = 0;
		pContext->bRefreshRequested = 0;
		return 0;
	}
	pOldHost = g_xgeXuiActiveHost;
	pOldContext = g_xgeXuiActiveContext;
	g_xgeXuiActiveHost = xgeXuiGetHost(pContext);
	g_xgeXuiActiveContext = pContext;
	pContext->iPaintFlushCount = 0;
	pContext->iPaintCommandCount = __xgeXuiPaintWidget(pContext->pRoot);
	if ( pContext->iPaintFlushCount < pContext->iPaintCommandCount ) {
		pContext->iPaintFlushCount = pContext->iPaintCommandCount;
	}
	g_xgeXuiActiveHost = pOldHost;
	g_xgeXuiActiveContext = pOldContext;
	pContext->iDirtyPaintCount = 0;
	pContext->bRefreshRequested = 0;
	return pContext->iPaintCommandCount;
}
