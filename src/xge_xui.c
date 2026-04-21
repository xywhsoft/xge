// XUI 孵化期基础模块：context、widget tree、dirty layout 和 paint 统计

static void __xgeXuiStyleInit(xge_xui_style_t* pStyle)
{
	if ( pStyle == NULL ) {
		return;
	}
	memset(pStyle, 0, sizeof(*pStyle));
	pStyle->iLayout = XGE_XUI_LAYOUT_ABSOLUTE;
	pStyle->tWidth = xgeXuiSizeContent();
	pStyle->tHeight = xgeXuiSizeContent();
	pStyle->tMinWidth = xgeXuiSizePx(0.0f);
	pStyle->tMinHeight = xgeXuiSizePx(0.0f);
	pStyle->tMaxWidth = xgeXuiSizePx(0.0f);
	pStyle->tMaxHeight = xgeXuiSizePx(0.0f);
	pStyle->iBackgroundColor = XGE_COLOR_RGBA(0, 0, 0, 0);
}

static float __xgeXuiSizeResolve(xge_xui_size_t tSize, float fParent, float fContent, float fFallback)
{
	switch ( tSize.iUnit ) {
		case XGE_XUI_SIZE_PX:
		case XGE_XUI_SIZE_DIP:
			return tSize.fValue;

		case XGE_XUI_SIZE_PERCENT:
			return fParent * tSize.fValue * 0.01f;

		case XGE_XUI_SIZE_CONTENT:
			return (fContent > 0.0f) ? fContent : fFallback;

		case XGE_XUI_SIZE_GROW:
		default:
			return fFallback;
	}
}

static float __xgeXuiSizeClamp(float fValue, xge_xui_size_t tMin, xge_xui_size_t tMax, float fParent)
{
	float fMin;
	float fMax;

	fMin = __xgeXuiSizeResolve(tMin, fParent, 0.0f, 0.0f);
	fMax = __xgeXuiSizeResolve(tMax, fParent, 0.0f, 0.0f);
	if ( fValue < fMin ) {
		fValue = fMin;
	}
	if ( (fMax > 0.0f) && (fValue > fMax) ) {
		fValue = fMax;
	}
	return fValue;
}

static void __xgeXuiEdgesResolve(xge_xui_edges_t* pEdges, xge_rect_t tParent, float* pLeft, float* pTop, float* pRight, float* pBottom)
{
	if ( pEdges == NULL ) {
		*pLeft = 0.0f;
		*pTop = 0.0f;
		*pRight = 0.0f;
		*pBottom = 0.0f;
		return;
	}
	*pLeft = __xgeXuiSizeResolve(pEdges->tLeft, tParent.fW, 0.0f, 0.0f);
	*pTop = __xgeXuiSizeResolve(pEdges->tTop, tParent.fH, 0.0f, 0.0f);
	*pRight = __xgeXuiSizeResolve(pEdges->tRight, tParent.fW, 0.0f, 0.0f);
	*pBottom = __xgeXuiSizeResolve(pEdges->tBottom, tParent.fH, 0.0f, 0.0f);
}

static xge_rect_t __xgeXuiContentRect(xge_xui_widget pWidget)
{
	xge_rect_t tContent;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;

	tContent = pWidget->tRect;
	__xgeXuiEdgesResolve(&pWidget->tStyle.tPadding, pWidget->tRect, &fLeft, &fTop, &fRight, &fBottom);
	tContent.fX += fLeft;
	tContent.fY += fTop;
	tContent.fW -= fLeft + fRight;
	tContent.fH -= fTop + fBottom;
	if ( tContent.fW < 0.0f ) {
		tContent.fW = 0.0f;
	}
	if ( tContent.fH < 0.0f ) {
		tContent.fH = 0.0f;
	}
	return tContent;
}

static xge_rect_t __xgeXuiChildSizeResolve(xge_xui_widget pChild, xge_rect_t tParent, float fFallbackW, float fFallbackH)
{
	xge_rect_t tRect;

	tRect = pChild->tRect;
	tRect.fW = __xgeXuiSizeResolve(pChild->tStyle.tWidth, tParent.fW, pChild->tRect.fW, fFallbackW);
	tRect.fH = __xgeXuiSizeResolve(pChild->tStyle.tHeight, tParent.fH, pChild->tRect.fH, fFallbackH);
	tRect.fW = __xgeXuiSizeClamp(tRect.fW, pChild->tStyle.tMinWidth, pChild->tStyle.tMaxWidth, tParent.fW);
	tRect.fH = __xgeXuiSizeClamp(tRect.fH, pChild->tStyle.tMinHeight, pChild->tStyle.tMaxHeight, tParent.fH);
	return tRect;
}

static xge_xui_widget __xgeXuiWidgetAlloc(void)
{
	xge_xui_widget pWidget;

	pWidget = (xge_xui_widget)xrtMalloc(sizeof(*pWidget));
	if ( pWidget == NULL ) {
		return NULL;
	}
	memset(pWidget, 0, sizeof(*pWidget));
	__xgeXuiStyleInit(&pWidget->tStyle);
	pWidget->iFlags = XGE_XUI_WIDGET_VISIBLE | XGE_XUI_WIDGET_ENABLED | XGE_XUI_WIDGET_DIRTY_LAYOUT | XGE_XUI_WIDGET_DIRTY_PAINT;
	return pWidget;
}

static void __xgeXuiWidgetFreeTree(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_xui_widget pNext;

	if ( pWidget == NULL ) {
		return;
	}
	pChild = pWidget->pFirstChild;
	while ( pChild != NULL ) {
		pNext = pChild->pNextSibling;
		__xgeXuiWidgetFreeTree(pChild);
		pChild = pNext;
	}
	xrtFree(pWidget);
}

static void __xgeXuiLayoutAbsolute(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_rect_t tParent;
	xge_rect_t tChild;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;

	if ( pWidget == NULL ) {
		return;
	}
	tParent = pWidget->tContentRect;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		tChild = __xgeXuiChildSizeResolve(pChild, tParent, pChild->tRect.fW, pChild->tRect.fH);
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		(void)fRight;
		(void)fBottom;
		tChild.fX = tParent.fX + pChild->tRect.fX + fLeft;
		tChild.fY = tParent.fY + pChild->tRect.fY + fTop;
		pChild->tRect = tChild;
	}
}

static void __xgeXuiLayoutRow(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_rect_t tParent;
	xge_rect_t tChild;
	float fFixed;
	float fGrow;
	float fRemaining;
	float fX;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;

	tParent = pWidget->tContentRect;
	fFixed = 0.0f;
	fGrow = 0.0f;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		if ( pChild->tStyle.tWidth.iUnit == XGE_XUI_SIZE_GROW ) {
			fGrow += (pChild->tStyle.tWidth.fValue > 0.0f) ? pChild->tStyle.tWidth.fValue : 1.0f;
			fFixed += fLeft + fRight;
		} else {
			tChild = __xgeXuiChildSizeResolve(pChild, tParent, pChild->tRect.fW, tParent.fH - fTop - fBottom);
			fFixed += tChild.fW + fLeft + fRight;
		}
	}
	fRemaining = tParent.fW - fFixed;
	if ( fRemaining < 0.0f ) {
		fRemaining = 0.0f;
	}
	fX = tParent.fX;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		tChild = __xgeXuiChildSizeResolve(pChild, tParent, pChild->tRect.fW, tParent.fH - fTop - fBottom);
		if ( pChild->tStyle.tWidth.iUnit == XGE_XUI_SIZE_GROW ) {
			tChild.fW = (fGrow > 0.0f) ? (fRemaining * ((pChild->tStyle.tWidth.fValue > 0.0f) ? pChild->tStyle.tWidth.fValue : 1.0f) / fGrow) : 0.0f;
		}
		tChild.fX = fX + fLeft;
		tChild.fY = tParent.fY + fTop;
		pChild->tRect = tChild;
		fX += fLeft + tChild.fW + fRight;
	}
}

static void __xgeXuiLayoutColumn(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_rect_t tParent;
	xge_rect_t tChild;
	float fFixed;
	float fGrow;
	float fRemaining;
	float fY;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;

	tParent = pWidget->tContentRect;
	fFixed = 0.0f;
	fGrow = 0.0f;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		if ( pChild->tStyle.tHeight.iUnit == XGE_XUI_SIZE_GROW ) {
			fGrow += (pChild->tStyle.tHeight.fValue > 0.0f) ? pChild->tStyle.tHeight.fValue : 1.0f;
			fFixed += fTop + fBottom;
		} else {
			tChild = __xgeXuiChildSizeResolve(pChild, tParent, tParent.fW - fLeft - fRight, pChild->tRect.fH);
			fFixed += tChild.fH + fTop + fBottom;
		}
	}
	fRemaining = tParent.fH - fFixed;
	if ( fRemaining < 0.0f ) {
		fRemaining = 0.0f;
	}
	fY = tParent.fY;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		tChild = __xgeXuiChildSizeResolve(pChild, tParent, tParent.fW - fLeft - fRight, pChild->tRect.fH);
		if ( pChild->tStyle.tHeight.iUnit == XGE_XUI_SIZE_GROW ) {
			tChild.fH = (fGrow > 0.0f) ? (fRemaining * ((pChild->tStyle.tHeight.fValue > 0.0f) ? pChild->tStyle.tHeight.fValue : 1.0f) / fGrow) : 0.0f;
		}
		tChild.fX = tParent.fX + fLeft;
		tChild.fY = fY + fTop;
		pChild->tRect = tChild;
		fY += fTop + tChild.fH + fBottom;
	}
}

static void __xgeXuiLayoutStack(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_rect_t tParent;
	xge_rect_t tChild;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;

	tParent = pWidget->tContentRect;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		tChild = __xgeXuiChildSizeResolve(pChild, tParent, tParent.fW - fLeft - fRight, tParent.fH - fTop - fBottom);
		tChild.fX = tParent.fX + fLeft;
		tChild.fY = tParent.fY + fTop;
		pChild->tRect = tChild;
	}
}

static void __xgeXuiLayoutWidget(xge_xui_widget pWidget, xge_rect_t tParent)
{
	xge_xui_widget pChild;
	xge_rect_t tRect;

	if ( pWidget == NULL ) {
		return;
	}

	tRect = pWidget->tRect;
	if ( tRect.fW <= 0.0f ) {
		tRect.fW = tParent.fW;
	}
	if ( tRect.fH <= 0.0f ) {
		tRect.fH = tParent.fH;
	}
	pWidget->tRect = tRect;
	pWidget->tContentRect = __xgeXuiContentRect(pWidget);
	switch ( pWidget->tStyle.iLayout ) {
		case XGE_XUI_LAYOUT_ROW:
			__xgeXuiLayoutRow(pWidget);
			break;

		case XGE_XUI_LAYOUT_COLUMN:
			__xgeXuiLayoutColumn(pWidget);
			break;

		case XGE_XUI_LAYOUT_STACK:
			__xgeXuiLayoutStack(pWidget);
			break;

		case XGE_XUI_LAYOUT_GRID:
		case XGE_XUI_LAYOUT_ABSOLUTE:
		default:
			__xgeXuiLayoutAbsolute(pWidget);
			break;
	}
	pWidget->iFlags &= ~XGE_XUI_WIDGET_DIRTY_LAYOUT;

	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xgeXuiLayoutWidget(pChild, pWidget->tContentRect);
	}
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
		xgeClipSet(pWidget->tContentRect);
	}
	iCount = 0;
	if ( XGE_COLOR_GET_A(pWidget->tStyle.iBackgroundColor) != 0 ) {
		xgeShapeRectFillPx(pWidget->tRect, pWidget->tStyle.iBackgroundColor);
		iCount++;
	}
	if ( pWidget->procPaint != NULL ) {
		pWidget->procPaint(pWidget, pWidget->pUser);
		iCount++;
	}
	pWidget->iFlags &= ~XGE_XUI_WIDGET_DIRTY_PAINT;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		iCount += __xgeXuiPaintWidget(pChild);
	}
	if ( bUseClip ) {
		if ( bOldClip ) {
			xgeClipSet(tOldClip);
		} else {
			xgeClipClear();
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
	for ( pChild = pWidget->pLastChild; pChild != NULL; pChild = pChild->pPrevSibling ) {
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

xge_xui_size_t xgeXuiSizePx(float fValue)
{
	xge_xui_size_t tSize;

	tSize.fValue = fValue;
	tSize.iUnit = XGE_XUI_SIZE_PX;
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
	pContext->pRoot->tRect.fW = (float)xgeGetWidth();
	pContext->pRoot->tRect.fH = (float)xgeGetHeight();
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
	while ( pWidget != NULL ) {
		pWidget->iFlags |= XGE_XUI_WIDGET_DIRTY_LAYOUT;
		pWidget = pWidget->pParent;
	}
}

void xgeXuiWidgetMarkPaint(xge_xui_widget pWidget)
{
	while ( pWidget != NULL ) {
		pWidget->iFlags |= XGE_XUI_WIDGET_DIRTY_PAINT;
		pWidget = pWidget->pParent;
	}
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
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	if ( (pWidget != NULL) && (__xgeXuiWidgetCanFocus(pWidget) == 0) ) {
		return;
	}
	if ( pContext->pFocus != pWidget ) {
		xgeXuiWidgetMarkPaint(pContext->pFocus);
		pContext->pFocus = pWidget;
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
	return __xgeXuiDispatchToWidget(pTarget, pEvent);
}

int xgeXuiUpdate(xge_xui_context pContext, float fDelta)
{
	xge_rect_t tRootRect;

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
	__xgeXuiLayoutWidget(pContext->pRoot, tRootRect);
	pContext->iDirtyLayoutCount = 0;
	return XGE_OK;
}

int xgeXuiPaint(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pContext->pRoot == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pContext->iPaintCommandCount = __xgeXuiPaintWidget(pContext->pRoot);
	pContext->iDirtyPaintCount = 0;
	return pContext->iPaintCommandCount;
}

static int __xgeXuiTextEnsure(xge_xui_text pText, int iNeed)
{
	char* sNew;
	int iCapacity;

	if ( pText == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iNeed <= pText->iCapacity ) {
		return XGE_OK;
	}
	iCapacity = (pText->iCapacity > 0) ? pText->iCapacity : 32;
	while ( iCapacity < iNeed ) {
		if ( iCapacity > (INT32_MAX / 2) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		iCapacity *= 2;
	}
	sNew = (char*)xrtRealloc(pText->sText, (size_t)iCapacity);
	if ( sNew == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pText->sText = sNew;
	pText->iCapacity = iCapacity;
	return XGE_OK;
}

static int __xgeXuiTextUTF8Encode(uint32_t iCodepoint, char* sOut)
{
	if ( sOut == NULL ) {
		return 0;
	}
	if ( iCodepoint < 0x80 ) {
		sOut[0] = (char)iCodepoint;
		return 1;
	}
	if ( iCodepoint < 0x800 ) {
		sOut[0] = (char)(0xC0 | ((iCodepoint >> 6) & 0x1F));
		sOut[1] = (char)(0x80 | (iCodepoint & 0x3F));
		return 2;
	}
	if ( (iCodepoint < 0x10000) && ((iCodepoint < 0xD800) || (iCodepoint > 0xDFFF)) ) {
		sOut[0] = (char)(0xE0 | ((iCodepoint >> 12) & 0x0F));
		sOut[1] = (char)(0x80 | ((iCodepoint >> 6) & 0x3F));
		sOut[2] = (char)(0x80 | (iCodepoint & 0x3F));
		return 3;
	}
	return 0;
}

static int __xgeXuiTextSelectionDelete(xge_xui_text pText)
{
	int iStart;
	int iEnd;
	int iRemain;

	if ( pText == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iStart = pText->iSelectStart;
	iEnd = pText->iSelectEnd;
	if ( iStart > iEnd ) {
		int iTmp = iStart;
		iStart = iEnd;
		iEnd = iTmp;
	}
	if ( (iStart < 0) || (iEnd > pText->iSize) || (iStart == iEnd) ) {
		pText->iSelectStart = pText->iCursor;
		pText->iSelectEnd = pText->iCursor;
		return XGE_OK;
	}
	iRemain = pText->iSize - iEnd;
	memmove(pText->sText + iStart, pText->sText + iEnd, (size_t)iRemain + 1);
	pText->iSize -= iEnd - iStart;
	pText->iCursor = iStart;
	pText->iSelectStart = iStart;
	pText->iSelectEnd = iStart;
	return XGE_OK;
}

int xgeXuiTextInit(xge_xui_text pText)
{
	int iRet;

	if ( pText == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pText, 0, sizeof(*pText));
	iRet = __xgeXuiTextEnsure(pText, 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pText->sText[0] = 0;
	return XGE_OK;
}

void xgeXuiTextUnit(xge_xui_text pText)
{
	if ( pText == NULL ) {
		return;
	}
	if ( pText->sText != NULL ) {
		xrtFree(pText->sText);
	}
	memset(pText, 0, sizeof(*pText));
}

int xgeXuiTextSet(xge_xui_text pText, const char* sText)
{
	int iSize;
	int iRet;

	if ( pText == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( sText == NULL ) {
		sText = "";
	}
	iSize = (int)strlen(sText);
	iRet = __xgeXuiTextEnsure(pText, iSize + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	memcpy(pText->sText, sText, (size_t)iSize + 1);
	pText->iSize = iSize;
	pText->iCursor = iSize;
	pText->iSelectStart = iSize;
	pText->iSelectEnd = iSize;
	return XGE_OK;
}

int xgeXuiTextInsert(xge_xui_text pText, const char* sText)
{
	int iSize;
	int iRet;

	if ( (pText == NULL) || (sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pText->sText == NULL ) {
		iRet = xgeXuiTextInit(pText);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	iRet = __xgeXuiTextSelectionDelete(pText);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iSize = (int)strlen(sText);
	iRet = __xgeXuiTextEnsure(pText, pText->iSize + iSize + 1);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	if ( pText->iCursor < 0 ) {
		pText->iCursor = 0;
	}
	if ( pText->iCursor > pText->iSize ) {
		pText->iCursor = pText->iSize;
	}
	memmove(pText->sText + pText->iCursor + iSize, pText->sText + pText->iCursor, (size_t)(pText->iSize - pText->iCursor) + 1);
	memcpy(pText->sText + pText->iCursor, sText, (size_t)iSize);
	pText->iCursor += iSize;
	pText->iSize += iSize;
	pText->iSelectStart = pText->iCursor;
	pText->iSelectEnd = pText->iCursor;
	return XGE_OK;
}

int xgeXuiTextInsertCodepoint(xge_xui_text pText, uint32_t iCodepoint)
{
	char arrText[4];
	int iSize;

	iSize = __xgeXuiTextUTF8Encode(iCodepoint, arrText);
	if ( iSize <= 0 ) {
		return XGE_ERROR_UNSUPPORTED;
	}
	arrText[iSize] = 0;
	return xgeXuiTextInsert(pText, arrText);
}

int xgeXuiTextDeleteBack(xge_xui_text pText)
{
	int iStart;

	if ( (pText == NULL) || (pText->sText == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pText->iSelectStart != pText->iSelectEnd ) {
		return __xgeXuiTextSelectionDelete(pText);
	}
	if ( pText->iCursor <= 0 ) {
		return XGE_OK;
	}
	if ( pText->iCursor > pText->iSize ) {
		pText->iCursor = pText->iSize;
	}
	iStart = pText->iCursor - 1;
	while ( (iStart > 0) && (((unsigned char)pText->sText[iStart] & 0xC0) == 0x80) ) {
		iStart--;
	}
	memmove(pText->sText + iStart, pText->sText + pText->iCursor, (size_t)(pText->iSize - pText->iCursor) + 1);
	pText->iSize -= pText->iCursor - iStart;
	pText->iCursor = iStart;
	pText->iSelectStart = iStart;
	pText->iSelectEnd = iStart;
	return XGE_OK;
}

int xgeXuiTextInputEvent(xge_xui_text pText, const xge_event_t* pEvent)
{
	if ( (pText == NULL) || (pEvent == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pEvent->iType != XGE_EVENT_TEXT ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iCodepoint == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( xgeXuiTextInsertCodepoint(pText, pEvent->iCodepoint) != XGE_OK ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	return XGE_XUI_EVENT_CONSUMED;
}

int xgeXuiButtonInit(xge_xui_button pButton, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pButton == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pButton, 0, sizeof(*pButton));
	pButton->pContext = pContext;
	pButton->pWidget = pWidget;
	pButton->sText = "";
	pButton->iTextColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pButton->iTextFlags = XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pButton->iColorNormal = XGE_COLOR_RGBA(48, 64, 82, 255);
	pButton->iColorHover = XGE_COLOR_RGBA(62, 82, 104, 255);
	pButton->iColorActive = XGE_COLOR_RGBA(34, 48, 64, 255);
	pButton->iColorFocus = XGE_COLOR_RGBA(54, 72, 96, 255);
	pButton->iColorDisabled = XGE_COLOR_RGBA(68, 68, 68, 160);
	xgeXuiWidgetSetFocusable(pWidget, 1);
	pWidget->procEvent = xgeXuiButtonEventProc;
	pWidget->procPaint = xgeXuiButtonPaintProc;
	pWidget->pUser = pButton;
	__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiButtonUnit(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return;
	}
	if ( pButton->pWidget != NULL && pButton->pWidget->pUser == pButton ) {
		pButton->pWidget->pUser = NULL;
		pButton->pWidget->procEvent = NULL;
		pButton->pWidget->procPaint = NULL;
	}
	memset(pButton, 0, sizeof(*pButton));
}

void xgeXuiButtonSetClick(xge_xui_button pButton, xge_xui_click_proc procClick, void* pUser)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->procClick = procClick;
	pButton->pUser = pUser;
}

void xgeXuiButtonSetText(xge_xui_button pButton, xge_font pFont, const char* sText)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->pFont = pFont;
	pButton->sText = (sText != NULL) ? sText : "";
	xgeXuiWidgetMarkLayout(pButton->pWidget);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetTextColor(xge_xui_button pButton, uint32_t iColor)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->iTextColor = iColor;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetColors(xge_xui_button pButton, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->iColorNormal = iNormal;
	pButton->iColorHover = iHover;
	pButton->iColorActive = iActive;
	pButton->iColorFocus = iFocus;
	pButton->iColorDisabled = iDisabled;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

int xgeXuiButtonGetState(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiButtonSetState(pButton, pButton->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pButton->iState;
}

int xgeXuiButtonEvent(xge_xui_button pButton, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;

	if ( (pButton == NULL) || (pButton->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pButton->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pButton->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pButton->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pButton->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);

	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( iInside ) {
				iState |= XGE_XUI_STATE_HOVER;
			} else {
				iState &= ~XGE_XUI_STATE_HOVER;
			}
			__xgeXuiButtonSetState(pButton, iState);
			return ((iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pButton->pContext, pButton->pWidget);
			xgeXuiSetCapture(pButton->pContext, pButton->pWidget);
			__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pButton->iState & XGE_XUI_STATE_ACTIVE) != 0);
			iState = iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
			__xgeXuiButtonSetState(pButton, iState);
			if ( pButton->pContext != NULL && pButton->pContext->pCapture == pButton->pWidget ) {
				xgeXuiSetCapture(pButton->pContext, NULL);
			}
			if ( bWasActive && iInside ) {
				pButton->iClickCount++;
				if ( pButton->procClick != NULL ) {
					pButton->procClick(pButton->pWidget, pButton->pUser);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
			__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_NORMAL);
			if ( pButton->pContext != NULL && pButton->pContext->pCapture == pButton->pWidget ) {
				xgeXuiSetCapture(pButton->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiButtonEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiButtonEvent((xge_xui_button)pUser, pEvent);
}

void xgeXuiButtonPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_button pButton;
	uint32_t iColor;

	pButton = (xge_xui_button)pUser;
	if ( (pWidget == NULL) || (pButton == NULL) ) {
		return;
	}
	iColor = __xgeXuiButtonColor(pButton);
	if ( XGE_COLOR_GET_A(iColor) != 0 ) {
		xgeShapeRectFillPx(pWidget->tRect, iColor);
	}
	if ( (pButton->pFont != NULL) && (pButton->sText != NULL) && (pButton->sText[0] != 0) ) {
		xgeTextDrawRect(pButton->pFont, pButton->sText, pWidget->tContentRect, pButton->iTextColor, pButton->iTextFlags);
	}
}

int xgeXuiLabelInit(xge_xui_label pLabel, xge_xui_widget pWidget, xge_font pFont, const char* sText)
{
	if ( (pLabel == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pLabel, 0, sizeof(*pLabel));
	pLabel->pWidget = pWidget;
	pLabel->pFont = pFont;
	pLabel->sText = (sText != NULL) ? sText : "";
	pLabel->iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pLabel->iTextFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP | XGE_TEXT_CLIP;
	pWidget->procPaint = xgeXuiLabelPaintProc;
	pWidget->pUser = pLabel;
	pLabel->tMeasuredSize = xgeXuiLabelMeasure(pLabel);
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiLabelUnit(xge_xui_label pLabel)
{
	if ( pLabel == NULL ) {
		return;
	}
	if ( pLabel->pWidget != NULL && pLabel->pWidget->pUser == pLabel ) {
		pLabel->pWidget->pUser = NULL;
		pLabel->pWidget->procPaint = NULL;
	}
	memset(pLabel, 0, sizeof(*pLabel));
}

void xgeXuiLabelSetText(xge_xui_label pLabel, const char* sText)
{
	if ( pLabel == NULL ) {
		return;
	}
	pLabel->sText = (sText != NULL) ? sText : "";
	pLabel->tMeasuredSize = xgeXuiLabelMeasure(pLabel);
	xgeXuiWidgetMarkLayout(pLabel->pWidget);
	xgeXuiWidgetMarkPaint(pLabel->pWidget);
}

void xgeXuiLabelSetFont(xge_xui_label pLabel, xge_font pFont)
{
	if ( pLabel == NULL ) {
		return;
	}
	pLabel->pFont = pFont;
	pLabel->tMeasuredSize = xgeXuiLabelMeasure(pLabel);
	xgeXuiWidgetMarkLayout(pLabel->pWidget);
	xgeXuiWidgetMarkPaint(pLabel->pWidget);
}

void xgeXuiLabelSetColor(xge_xui_label pLabel, uint32_t iColor)
{
	if ( pLabel == NULL ) {
		return;
	}
	pLabel->iColor = iColor;
	xgeXuiWidgetMarkPaint(pLabel->pWidget);
}

void xgeXuiLabelSetAlign(xge_xui_label pLabel, uint32_t iTextFlags)
{
	if ( pLabel == NULL ) {
		return;
	}
	pLabel->iTextFlags = iTextFlags | XGE_TEXT_CLIP;
	xgeXuiWidgetMarkPaint(pLabel->pWidget);
}

xge_vec2_t xgeXuiLabelMeasure(xge_xui_label pLabel)
{
	xge_vec2_t tSize;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( (pLabel == NULL) || (pLabel->pFont == NULL) || (pLabel->sText == NULL) ) {
		return tSize;
	}
	tSize = xgeTextMeasure(pLabel->pFont, pLabel->sText);
	return tSize;
}

void xgeXuiLabelPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_label pLabel;

	pLabel = (xge_xui_label)pUser;
	if ( (pWidget == NULL) || (pLabel == NULL) || (pLabel->pFont == NULL) || (pLabel->sText == NULL) ) {
		return;
	}
	xgeTextDrawRect(pLabel->pFont, pLabel->sText, pWidget->tContentRect, pLabel->iColor, pLabel->iTextFlags);
}

int xgeXuiImageInit(xge_xui_image pImage, xge_xui_widget pWidget, xge_texture pTexture)
{
	if ( (pImage == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pImage, 0, sizeof(*pImage));
	pImage->pWidget = pWidget;
	pImage->pTexture = pTexture;
	pImage->iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pImage->iMode = XGE_XUI_IMAGE_STRETCH;
	pWidget->procPaint = xgeXuiImagePaintProc;
	pWidget->pUser = pImage;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiImageUnit(xge_xui_image pImage)
{
	if ( pImage == NULL ) {
		return;
	}
	if ( pImage->pWidget != NULL && pImage->pWidget->pUser == pImage ) {
		pImage->pWidget->pUser = NULL;
		pImage->pWidget->procPaint = NULL;
	}
	memset(pImage, 0, sizeof(*pImage));
}

void xgeXuiImageSetTexture(xge_xui_image pImage, xge_texture pTexture)
{
	if ( pImage == NULL ) {
		return;
	}
	pImage->pTexture = pTexture;
	xgeXuiWidgetMarkPaint(pImage->pWidget);
}

void xgeXuiImageSetSource(xge_xui_image pImage, xge_rect_t tSrc)
{
	if ( pImage == NULL ) {
		return;
	}
	pImage->tSrc = tSrc;
	xgeXuiWidgetMarkPaint(pImage->pWidget);
}

void xgeXuiImageSetColor(xge_xui_image pImage, uint32_t iColor)
{
	if ( pImage == NULL ) {
		return;
	}
	pImage->iColor = iColor;
	xgeXuiWidgetMarkPaint(pImage->pWidget);
}

void xgeXuiImageSetMode(xge_xui_image pImage, int iMode)
{
	if ( pImage == NULL ) {
		return;
	}
	pImage->iMode = iMode;
	xgeXuiWidgetMarkPaint(pImage->pWidget);
}

static xge_rect_t __xgeXuiImageDestRect(xge_xui_image pImage, xge_rect_t tContent)
{
	xge_rect_t tDst;
	float fSrcW;
	float fSrcH;
	float fScale;

	tDst = tContent;
	if ( (pImage == NULL) || (pImage->pTexture == NULL) ) {
		return tDst;
	}
	fSrcW = (pImage->tSrc.fW > 0.0f) ? pImage->tSrc.fW : (float)pImage->pTexture->iWidth;
	fSrcH = (pImage->tSrc.fH > 0.0f) ? pImage->tSrc.fH : (float)pImage->pTexture->iHeight;
	if ( (fSrcW <= 0.0f) || (fSrcH <= 0.0f) ) {
		return tDst;
	}
	if ( pImage->iMode == XGE_XUI_IMAGE_CENTER ) {
		tDst.fW = fSrcW;
		tDst.fH = fSrcH;
		tDst.fX = tContent.fX + (tContent.fW - tDst.fW) * 0.5f;
		tDst.fY = tContent.fY + (tContent.fH - tDst.fH) * 0.5f;
	} else if ( pImage->iMode == XGE_XUI_IMAGE_FIT ) {
		fScale = tContent.fW / fSrcW;
		if ( (fSrcH * fScale) > tContent.fH ) {
			fScale = tContent.fH / fSrcH;
		}
		tDst.fW = fSrcW * fScale;
		tDst.fH = fSrcH * fScale;
		tDst.fX = tContent.fX + (tContent.fW - tDst.fW) * 0.5f;
		tDst.fY = tContent.fY + (tContent.fH - tDst.fH) * 0.5f;
	}
	return tDst;
}

void xgeXuiImagePaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_image pImage;
	xge_draw_t tDraw;

	pImage = (xge_xui_image)pUser;
	if ( (pWidget == NULL) || (pImage == NULL) || (pImage->pTexture == NULL) || (XGE_COLOR_GET_A(pImage->iColor) == 0) ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = pImage->pTexture;
	tDraw.tSrc = pImage->tSrc;
	tDraw.tDst = __xgeXuiImageDestRect(pImage, pWidget->tContentRect);
	tDraw.iColor = pImage->iColor;
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
}

int xgeXuiInputInit(xge_xui_input pInput, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	int iRet;

	if ( (pInput == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pInput, 0, sizeof(*pInput));
	iRet = xgeXuiTextInit(&pInput->tText);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pInput->pContext = pContext;
	pInput->pWidget = pWidget;
	pInput->pFont = pFont;
	pInput->iTextColor = XGE_COLOR_RGBA(24, 28, 34, 255);
	pInput->iBackgroundColor = XGE_COLOR_RGBA(248, 250, 252, 255);
	pInput->iFocusColor = XGE_COLOR_RGBA(220, 235, 255, 255);
	pInput->iCursorColor = XGE_COLOR_RGBA(24, 28, 34, 255);
	pInput->bInitialized = 1;
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetClip(pWidget, 1);
	pWidget->procEvent = xgeXuiInputEventProc;
	pWidget->procPaint = xgeXuiInputPaintProc;
	pWidget->pUser = pInput;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiInputUnit(xge_xui_input pInput)
{
	if ( pInput == NULL ) {
		return;
	}
	if ( pInput->pWidget != NULL && pInput->pWidget->pUser == pInput ) {
		pInput->pWidget->pUser = NULL;
		pInput->pWidget->procEvent = NULL;
		pInput->pWidget->procPaint = NULL;
	}
	xgeXuiTextUnit(&pInput->tText);
	memset(pInput, 0, sizeof(*pInput));
}

void xgeXuiInputSetText(xge_xui_input pInput, const char* sText)
{
	if ( (pInput == NULL) || (pInput->bInitialized == 0) ) {
		return;
	}
	if ( xgeXuiTextSet(&pInput->tText, sText) == XGE_OK ) {
		xgeXuiWidgetMarkLayout(pInput->pWidget);
		xgeXuiWidgetMarkPaint(pInput->pWidget);
	}
}

const char* xgeXuiInputGetText(xge_xui_input pInput)
{
	if ( (pInput == NULL) || (pInput->tText.sText == NULL) ) {
		return "";
	}
	return pInput->tText.sText;
}

void xgeXuiInputSetFont(xge_xui_input pInput, xge_font pFont)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->pFont = pFont;
	xgeXuiWidgetMarkLayout(pInput->pWidget);
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

void xgeXuiInputSetColors(xge_xui_input pInput, uint32_t iText, uint32_t iBackground, uint32_t iFocus, uint32_t iCursor)
{
	if ( pInput == NULL ) {
		return;
	}
	pInput->iTextColor = iText;
	pInput->iBackgroundColor = iBackground;
	pInput->iFocusColor = iFocus;
	pInput->iCursorColor = iCursor;
	xgeXuiWidgetMarkPaint(pInput->pWidget);
}

int xgeXuiInputEvent(xge_xui_input pInput, const xge_event_t* pEvent)
{
	int iResult;

	if ( (pInput == NULL) || (pInput->bInitialized == 0) || (pInput->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( ((pInput->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || ((pInput->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_DOWN) || (pEvent->iType == XGE_EVENT_TOUCH_BEGIN)) && __xgeXuiRectContains(pInput->pWidget->tRect, pEvent->fX, pEvent->fY) ) {
		xgeXuiSetFocus(pInput->pContext, pInput->pWidget);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pInput->pContext != NULL) && (pInput->pContext->pFocus != pInput->pWidget) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_TEXT ) {
		iResult = xgeXuiTextInputEvent(&pInput->tText, pEvent);
		if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
			xgeXuiWidgetMarkPaint(pInput->pWidget);
		}
		return iResult;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_BACKSPACE) ) {
		if ( xgeXuiTextDeleteBack(&pInput->tText) == XGE_OK ) {
			xgeXuiWidgetMarkPaint(pInput->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int xgeXuiInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiInputEvent((xge_xui_input)pUser, pEvent);
}

void xgeXuiInputPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_input pInput;
	xge_rect_t tCursor;
	xge_vec2_t tSize;
	uint32_t iBackground;

	pInput = (xge_xui_input)pUser;
	if ( (pWidget == NULL) || (pInput == NULL) ) {
		return;
	}
	iBackground = (pInput->pContext != NULL && pInput->pContext->pFocus == pWidget) ? pInput->iFocusColor : pInput->iBackgroundColor;
	if ( XGE_COLOR_GET_A(iBackground) != 0 ) {
		xgeShapeRectFillPx(pWidget->tRect, iBackground);
	}
	if ( (pInput->pFont != NULL) && (pInput->tText.sText != NULL) ) {
		xgeTextDrawRect(pInput->pFont, pInput->tText.sText, pWidget->tContentRect, pInput->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	if ( (pInput->pContext != NULL) && (pInput->pContext->pFocus == pWidget) && (XGE_COLOR_GET_A(pInput->iCursorColor) != 0) ) {
		tSize = xgeTextMeasure(pInput->pFont, xgeXuiInputGetText(pInput));
		tCursor.fX = pWidget->tContentRect.fX + tSize.fX + 1.0f;
		if ( tCursor.fX > (pWidget->tContentRect.fX + pWidget->tContentRect.fW - 1.0f) ) {
			tCursor.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - 1.0f;
		}
		tCursor.fY = pWidget->tContentRect.fY + 2.0f;
		tCursor.fW = 1.0f;
		tCursor.fH = pWidget->tContentRect.fH - 4.0f;
		if ( tCursor.fH < 1.0f ) {
			tCursor.fH = 1.0f;
		}
		xgeShapeRectFillPx(tCursor, pInput->iCursorColor);
	}
}

int xgeXuiToggleInit(xge_xui_toggle pToggle, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pToggle == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pToggle, 0, sizeof(*pToggle));
	pToggle->pContext = pContext;
	pToggle->pWidget = pWidget;
	pToggle->sText = "";
	pToggle->iTextColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pToggle->iTextFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pToggle->iColorNormal = XGE_COLOR_RGBA(54, 60, 70, 255);
	pToggle->iColorHover = XGE_COLOR_RGBA(68, 76, 88, 255);
	pToggle->iColorActive = XGE_COLOR_RGBA(40, 46, 56, 255);
	pToggle->iColorFocus = XGE_COLOR_RGBA(62, 74, 94, 255);
	pToggle->iColorDisabled = XGE_COLOR_RGBA(68, 68, 68, 160);
	pToggle->iColorChecked = XGE_COLOR_RGBA(62, 172, 110, 255);
	xgeXuiWidgetSetFocusable(pWidget, 1);
	pWidget->procEvent = xgeXuiToggleEventProc;
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
	if ( pToggle->pWidget != NULL && pToggle->pWidget->pUser == pToggle ) {
		pToggle->pWidget->pUser = NULL;
		pToggle->pWidget->procEvent = NULL;
		pToggle->pWidget->procPaint = NULL;
	}
	memset(pToggle, 0, sizeof(*pToggle));
}

void xgeXuiToggleSetChange(xge_xui_toggle pToggle, xge_xui_toggle_proc procChange, void* pUser)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->procChange = procChange;
	pToggle->pUser = pUser;
}

void xgeXuiToggleSetText(xge_xui_toggle pToggle, xge_font pFont, const char* sText)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->pFont = pFont;
	pToggle->sText = (sText != NULL) ? sText : "";
	xgeXuiWidgetMarkLayout(pToggle->pWidget);
	xgeXuiWidgetMarkPaint(pToggle->pWidget);
}

void xgeXuiToggleSetChecked(xge_xui_toggle pToggle, int bChecked)
{
	if ( pToggle == NULL ) {
		return;
	}
	bChecked = bChecked ? 1 : 0;
	if ( pToggle->bChecked != bChecked ) {
		pToggle->bChecked = bChecked;
		xgeXuiWidgetMarkPaint(pToggle->pWidget);
	}
}

int xgeXuiToggleGetChecked(xge_xui_toggle pToggle)
{
	if ( pToggle == NULL ) {
		return 0;
	}
	return pToggle->bChecked;
}

void xgeXuiToggleSetTextColor(xge_xui_toggle pToggle, uint32_t iColor)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->iTextColor = iColor;
	xgeXuiWidgetMarkPaint(pToggle->pWidget);
}

void xgeXuiToggleSetColors(xge_xui_toggle pToggle, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iChecked)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->iColorNormal = iNormal;
	pToggle->iColorHover = iHover;
	pToggle->iColorActive = iActive;
	pToggle->iColorFocus = iFocus;
	pToggle->iColorDisabled = iDisabled;
	pToggle->iColorChecked = iChecked;
	xgeXuiWidgetMarkPaint(pToggle->pWidget);
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
			if ( iInside ) {
				iState |= XGE_XUI_STATE_HOVER;
			} else {
				iState &= ~XGE_XUI_STATE_HOVER;
			}
			__xgeXuiToggleSetState(pToggle, iState);
			return ((iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pToggle->pContext, pToggle->pWidget);
			xgeXuiSetCapture(pToggle->pContext, pToggle->pWidget);
			__xgeXuiToggleSetState(pToggle, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pToggle->iState & XGE_XUI_STATE_ACTIVE) != 0);
			iState = iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
			__xgeXuiToggleSetState(pToggle, iState);
			if ( pToggle->pContext != NULL && pToggle->pContext->pCapture == pToggle->pWidget ) {
				xgeXuiSetCapture(pToggle->pContext, NULL);
			}
			if ( bWasActive && iInside ) {
				pToggle->bChecked = pToggle->bChecked ? 0 : 1;
				pToggle->iChangeCount++;
				xgeXuiWidgetMarkPaint(pToggle->pWidget);
				if ( pToggle->procChange != NULL ) {
					pToggle->procChange(pToggle->pWidget, pToggle->bChecked, pToggle->pUser);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
			__xgeXuiToggleSetState(pToggle, XGE_XUI_STATE_NORMAL);
			if ( pToggle->pContext != NULL && pToggle->pContext->pCapture == pToggle->pWidget ) {
				xgeXuiSetCapture(pToggle->pContext, NULL);
			}
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
	xge_rect_t tBox;
	xge_rect_t tText;
	float fBoxSize;
	uint32_t iColor;

	pToggle = (xge_xui_toggle)pUser;
	if ( (pWidget == NULL) || (pToggle == NULL) ) {
		return;
	}
	iColor = __xgeXuiToggleColor(pToggle);
	if ( XGE_COLOR_GET_A(iColor) != 0 ) {
		xgeShapeRectFillPx(pWidget->tRect, iColor);
	}
	fBoxSize = pWidget->tContentRect.fH;
	if ( fBoxSize > 18.0f ) {
		fBoxSize = 18.0f;
	}
	if ( fBoxSize < 1.0f ) {
		fBoxSize = 1.0f;
	}
	tBox.fX = pWidget->tContentRect.fX;
	tBox.fY = pWidget->tContentRect.fY + (pWidget->tContentRect.fH - fBoxSize) * 0.5f;
	tBox.fW = fBoxSize;
	tBox.fH = fBoxSize;
	xgeShapeRectFillPx(tBox, pToggle->bChecked ? pToggle->iColorChecked : XGE_COLOR_RGBA(180, 186, 196, 255));
	if ( pToggle->bChecked ) {
		tBox.fX += 4.0f;
		tBox.fY += 4.0f;
		tBox.fW -= 8.0f;
		tBox.fH -= 8.0f;
		if ( (tBox.fW > 0.0f) && (tBox.fH > 0.0f) ) {
			xgeShapeRectFillPx(tBox, XGE_COLOR_RGBA(255, 255, 255, 255));
		}
	}
	if ( (pToggle->pFont != NULL) && (pToggle->sText != NULL) && (pToggle->sText[0] != 0) ) {
		tText = pWidget->tContentRect;
		tText.fX += fBoxSize + 6.0f;
		tText.fW -= fBoxSize + 6.0f;
		if ( tText.fW > 0.0f ) {
			xgeTextDrawRect(pToggle->pFont, pToggle->sText, tText, pToggle->iTextColor, pToggle->iTextFlags);
		}
	}
}

int xgeXuiSliderInit(xge_xui_slider pSlider, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pSlider == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pSlider, 0, sizeof(*pSlider));
	pSlider->pContext = pContext;
	pSlider->pWidget = pWidget;
	pSlider->fMin = 0.0f;
	pSlider->fMax = 1.0f;
	pSlider->fValue = 0.0f;
	pSlider->iColorTrack = XGE_COLOR_RGBA(92, 100, 112, 255);
	pSlider->iColorFill = XGE_COLOR_RGBA(62, 172, 110, 255);
	pSlider->iColorKnob = XGE_COLOR_RGBA(248, 250, 252, 255);
	pSlider->iColorFocus = XGE_COLOR_RGBA(70, 92, 128, 255);
	pSlider->iColorDisabled = XGE_COLOR_RGBA(96, 96, 96, 160);
	xgeXuiWidgetSetFocusable(pWidget, 1);
	pWidget->procEvent = xgeXuiSliderEventProc;
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
	if ( pSlider->pWidget != NULL && pSlider->pWidget->pUser == pSlider ) {
		pSlider->pWidget->pUser = NULL;
		pSlider->pWidget->procEvent = NULL;
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
	xgeXuiWidgetMarkPaint(pSlider->pWidget);
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
	xgeXuiWidgetMarkPaint(pSlider->pWidget);
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
			if ( iInside ) {
				iState |= XGE_XUI_STATE_HOVER;
			} else {
				iState &= ~XGE_XUI_STATE_HOVER;
			}
			if ( (pSlider->pContext != NULL) && (pSlider->pContext->pCapture == pSlider->pWidget) ) {
				iState |= XGE_XUI_STATE_ACTIVE;
				__xgeXuiSliderSetValueFromPoint(pSlider, pEvent->fX, 1);
				__xgeXuiSliderSetState(pSlider, iState);
				return XGE_XUI_EVENT_CONSUMED;
			}
			__xgeXuiSliderSetState(pSlider, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pSlider->pContext, pSlider->pWidget);
			xgeXuiSetCapture(pSlider->pContext, pSlider->pWidget);
			__xgeXuiSliderSetValueFromPoint(pSlider, pEvent->fX, 1);
			__xgeXuiSliderSetState(pSlider, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pSlider->iState & XGE_XUI_STATE_ACTIVE) != 0);
			if ( bWasActive ) {
				__xgeXuiSliderSetValueFromPoint(pSlider, pEvent->fX, 1);
			}
			iState = iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
			__xgeXuiSliderSetState(pSlider, iState);
			if ( pSlider->pContext != NULL && pSlider->pContext->pCapture == pSlider->pWidget ) {
				xgeXuiSetCapture(pSlider->pContext, NULL);
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
			__xgeXuiSliderSetState(pSlider, XGE_XUI_STATE_NORMAL);
			if ( pSlider->pContext != NULL && pSlider->pContext->pCapture == pSlider->pWidget ) {
				xgeXuiSetCapture(pSlider->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiSliderEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiSliderEvent((xge_xui_slider)pUser, pEvent);
}

void xgeXuiSliderPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_slider pSlider;
	xge_rect_t tTrack;
	xge_rect_t tFill;
	xge_rect_t tKnob;
	float fRate;
	float fKnobSize;

	pSlider = (xge_xui_slider)pUser;
	if ( (pWidget == NULL) || (pSlider == NULL) ) {
		return;
	}
	tTrack = pWidget->tContentRect;
	if ( tTrack.fH > 6.0f ) {
		tTrack.fY += (tTrack.fH - 6.0f) * 0.5f;
		tTrack.fH = 6.0f;
	}
	if ( (pSlider->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		xgeShapeRectFillPx(tTrack, pSlider->iColorDisabled);
		return;
	}
	if ( (pSlider->iState & XGE_XUI_STATE_FOCUS) != 0 ) {
		xgeShapeRectFillPx(pWidget->tRect, pSlider->iColorFocus);
	}
	xgeShapeRectFillPx(tTrack, pSlider->iColorTrack);
	fRate = __xgeXuiSliderRate(pSlider);
	tFill = tTrack;
	tFill.fW *= fRate;
	if ( tFill.fW > 0.0f ) {
		xgeShapeRectFillPx(tFill, pSlider->iColorFill);
	}
	fKnobSize = pWidget->tContentRect.fH;
	if ( fKnobSize > 18.0f ) {
		fKnobSize = 18.0f;
	}
	if ( fKnobSize < 6.0f ) {
		fKnobSize = 6.0f;
	}
	tKnob.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW * fRate - fKnobSize * 0.5f;
	tKnob.fY = pWidget->tContentRect.fY + (pWidget->tContentRect.fH - fKnobSize) * 0.5f;
	tKnob.fW = fKnobSize;
	tKnob.fH = fKnobSize;
	if ( tKnob.fX < pWidget->tContentRect.fX ) {
		tKnob.fX = pWidget->tContentRect.fX;
	}
	if ( (tKnob.fX + tKnob.fW) > (pWidget->tContentRect.fX + pWidget->tContentRect.fW) ) {
		tKnob.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - tKnob.fW;
	}
	xgeShapeRectFillPx(tKnob, pSlider->iColorKnob);
}

int xgeXuiProgressInit(xge_xui_progress pProgress, xge_xui_widget pWidget)
{
	if ( (pProgress == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pProgress, 0, sizeof(*pProgress));
	pProgress->pWidget = pWidget;
	pProgress->sText = "";
	pProgress->fMin = 0.0f;
	pProgress->fMax = 1.0f;
	pProgress->fValue = 0.0f;
	pProgress->iColorTrack = XGE_COLOR_RGBA(92, 100, 112, 255);
	pProgress->iColorFill = XGE_COLOR_RGBA(62, 172, 110, 255);
	pProgress->iTextColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pProgress->iTextFlags = XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pWidget->procPaint = xgeXuiProgressPaintProc;
	pWidget->pUser = pProgress;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiProgressUnit(xge_xui_progress pProgress)
{
	if ( pProgress == NULL ) {
		return;
	}
	if ( pProgress->pWidget != NULL && pProgress->pWidget->pUser == pProgress ) {
		pProgress->pWidget->pUser = NULL;
		pProgress->pWidget->procPaint = NULL;
	}
	memset(pProgress, 0, sizeof(*pProgress));
}

void xgeXuiProgressSetRange(xge_xui_progress pProgress, float fMin, float fMax)
{
	if ( pProgress == NULL ) {
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
	pProgress->fMin = fMin;
	pProgress->fMax = fMax;
	__xgeXuiProgressSetValueInternal(pProgress, pProgress->fValue);
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetValue(xge_xui_progress pProgress, float fValue)
{
	__xgeXuiProgressSetValueInternal(pProgress, fValue);
}

float xgeXuiProgressGetValue(xge_xui_progress pProgress)
{
	if ( pProgress == NULL ) {
		return 0.0f;
	}
	return pProgress->fValue;
}

void xgeXuiProgressSetText(xge_xui_progress pProgress, xge_font pFont, const char* sText)
{
	if ( pProgress == NULL ) {
		return;
	}
	pProgress->pFont = pFont;
	pProgress->sText = (sText != NULL) ? sText : "";
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetTextColor(xge_xui_progress pProgress, uint32_t iColor)
{
	if ( pProgress == NULL ) {
		return;
	}
	pProgress->iTextColor = iColor;
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetColors(xge_xui_progress pProgress, uint32_t iTrack, uint32_t iFill)
{
	if ( pProgress == NULL ) {
		return;
	}
	pProgress->iColorTrack = iTrack;
	pProgress->iColorFill = iFill;
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_progress pProgress;
	xge_rect_t tFill;

	pProgress = (xge_xui_progress)pUser;
	if ( (pWidget == NULL) || (pProgress == NULL) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pProgress->iColorTrack) != 0 ) {
		xgeShapeRectFillPx(pWidget->tContentRect, pProgress->iColorTrack);
	}
	tFill = pWidget->tContentRect;
	tFill.fW *= __xgeXuiProgressRate(pProgress);
	if ( (tFill.fW > 0.0f) && (XGE_COLOR_GET_A(pProgress->iColorFill) != 0) ) {
		xgeShapeRectFillPx(tFill, pProgress->iColorFill);
	}
	if ( (pProgress->pFont != NULL) && (pProgress->sText != NULL) && (pProgress->sText[0] != 0) ) {
		xgeTextDrawRect(pProgress->pFont, pProgress->sText, pWidget->tContentRect, pProgress->iTextColor, pProgress->iTextFlags);
	}
}

int xgeXuiPanelInit(xge_xui_panel pPanel, xge_xui_widget pWidget)
{
	if ( (pPanel == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pPanel, 0, sizeof(*pPanel));
	pPanel->pWidget = pWidget;
	pPanel->sTitle = "";
	pPanel->iBackgroundColor = XGE_COLOR_RGBA(32, 38, 46, 255);
	pPanel->iTitleColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pPanel->iTitleFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP | XGE_TEXT_CLIP;
	pWidget->procPaint = xgeXuiPanelPaintProc;
	pWidget->pUser = pPanel;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiPanelUnit(xge_xui_panel pPanel)
{
	if ( pPanel == NULL ) {
		return;
	}
	if ( pPanel->pWidget != NULL && pPanel->pWidget->pUser == pPanel ) {
		pPanel->pWidget->pUser = NULL;
		pPanel->pWidget->procPaint = NULL;
	}
	memset(pPanel, 0, sizeof(*pPanel));
}

void xgeXuiPanelSetBackground(xge_xui_panel pPanel, uint32_t iColor)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->iBackgroundColor = iColor;
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

void xgeXuiPanelSetTitle(xge_xui_panel pPanel, xge_font pFont, const char* sTitle)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->pFont = pFont;
	pPanel->sTitle = (sTitle != NULL) ? sTitle : "";
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

void xgeXuiPanelSetTitleColor(xge_xui_panel pPanel, uint32_t iColor)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->iTitleColor = iColor;
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

void xgeXuiPanelSetTitleAlign(xge_xui_panel pPanel, uint32_t iTextFlags)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->iTitleFlags = iTextFlags | XGE_TEXT_CLIP;
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

void xgeXuiPanelSetClip(xge_xui_panel pPanel, int bClip)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->bClip = bClip ? 1 : 0;
	xgeXuiWidgetSetClip(pPanel->pWidget, pPanel->bClip);
}

void xgeXuiPanelPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_panel pPanel;

	pPanel = (xge_xui_panel)pUser;
	if ( (pWidget == NULL) || (pPanel == NULL) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pPanel->iBackgroundColor) != 0 ) {
		xgeShapeRectFillPx(pWidget->tRect, pPanel->iBackgroundColor);
	}
	if ( (pPanel->pFont != NULL) && (pPanel->sTitle != NULL) && (pPanel->sTitle[0] != 0) ) {
		xgeTextDrawRect(pPanel->pFont, pPanel->sTitle, pWidget->tContentRect, pPanel->iTitleColor, pPanel->iTitleFlags);
	}
}

int xgeXuiScrollViewInit(xge_xui_scroll_view pScroll, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pScroll == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pScroll, 0, sizeof(*pScroll));
	pScroll->pContext = pContext;
	pScroll->pWidget = pWidget;
	pScroll->fContentW = pWidget->tContentRect.fW;
	pScroll->fContentH = pWidget->tContentRect.fH;
	pScroll->iBackgroundColor = XGE_COLOR_RGBA(24, 28, 34, 255);
	pScroll->iBarColor = XGE_COLOR_RGBA(64, 72, 84, 180);
	pScroll->iThumbColor = XGE_COLOR_RGBA(160, 172, 188, 220);
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetClip(pWidget, 1);
	pWidget->procEvent = xgeXuiScrollViewEventProc;
	pWidget->procPaint = xgeXuiScrollViewPaintProc;
	pWidget->pUser = pScroll;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiScrollViewUnit(xge_xui_scroll_view pScroll)
{
	if ( pScroll == NULL ) {
		return;
	}
	if ( pScroll->pWidget != NULL && pScroll->pWidget->pUser == pScroll ) {
		pScroll->pWidget->pUser = NULL;
		pScroll->pWidget->procEvent = NULL;
		pScroll->pWidget->procPaint = NULL;
	}
	memset(pScroll, 0, sizeof(*pScroll));
}

void xgeXuiScrollViewSetContentSize(xge_xui_scroll_view pScroll, float fWidth, float fHeight)
{
	if ( pScroll == NULL ) {
		return;
	}
	if ( fWidth < 0.0f ) {
		fWidth = 0.0f;
	}
	if ( fHeight < 0.0f ) {
		fHeight = 0.0f;
	}
	pScroll->fContentW = fWidth;
	pScroll->fContentH = fHeight;
	__xgeXuiScrollViewClamp(pScroll);
	xgeXuiWidgetMarkPaint(pScroll->pWidget);
}

void xgeXuiScrollViewSetOffset(xge_xui_scroll_view pScroll, float fX, float fY)
{
	__xgeXuiScrollViewSetOffsetInternal(pScroll, fX, fY);
}

void xgeXuiScrollViewGetOffset(xge_xui_scroll_view pScroll, float* pX, float* pY)
{
	if ( pX != NULL ) {
		*pX = (pScroll != NULL) ? pScroll->fScrollX : 0.0f;
	}
	if ( pY != NULL ) {
		*pY = (pScroll != NULL) ? pScroll->fScrollY : 0.0f;
	}
}

void xgeXuiScrollViewSetColors(xge_xui_scroll_view pScroll, uint32_t iBackground, uint32_t iBar, uint32_t iThumb)
{
	if ( pScroll == NULL ) {
		return;
	}
	pScroll->iBackgroundColor = iBackground;
	pScroll->iBarColor = iBar;
	pScroll->iThumbColor = iThumb;
	xgeXuiWidgetMarkPaint(pScroll->pWidget);
}

int xgeXuiScrollViewEvent(xge_xui_scroll_view pScroll, const xge_event_t* pEvent)
{
	int iInside;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pScroll->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pScroll->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pScroll->pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiScrollViewSetOffsetInternal(pScroll, pScroll->fScrollX - pEvent->fDX * 32.0f, pScroll->fScrollY - pEvent->fDY * 32.0f);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pScroll->bDragging = 1;
			pScroll->fDragX = pEvent->fX;
			pScroll->fDragY = pEvent->fY;
			xgeXuiSetFocus(pScroll->pContext, pScroll->pWidget);
			xgeXuiSetCapture(pScroll->pContext, pScroll->pWidget);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pScroll->bDragging == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiScrollViewSetOffsetInternal(pScroll, pScroll->fScrollX + (pScroll->fDragX - pEvent->fX), pScroll->fScrollY + (pScroll->fDragY - pEvent->fY));
			pScroll->fDragX = pEvent->fX;
			pScroll->fDragY = pEvent->fY;
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			if ( pScroll->bDragging == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pScroll->bDragging = 0;
			if ( pScroll->pContext != NULL && pScroll->pContext->pCapture == pScroll->pWidget ) {
				xgeXuiSetCapture(pScroll->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiScrollViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiScrollViewEvent((xge_xui_scroll_view)pUser, pEvent);
}

void xgeXuiScrollViewPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_scroll_view pScroll;
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fVisible;
	float fContent;
	float fMaxScroll;

	pScroll = (xge_xui_scroll_view)pUser;
	if ( (pWidget == NULL) || (pScroll == NULL) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pScroll->iBackgroundColor) != 0 ) {
		xgeShapeRectFillPx(pWidget->tRect, pScroll->iBackgroundColor);
	}
	fVisible = pWidget->tContentRect.fH;
	fContent = pScroll->fContentH;
	if ( fContent > fVisible && fVisible > 0.0f ) {
		tBar.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - 4.0f;
		tBar.fY = pWidget->tContentRect.fY;
		tBar.fW = 4.0f;
		tBar.fH = pWidget->tContentRect.fH;
		xgeShapeRectFillPx(tBar, pScroll->iBarColor);
		tThumb = tBar;
		tThumb.fH = tBar.fH * (fVisible / fContent);
		if ( tThumb.fH < 8.0f ) {
			tThumb.fH = 8.0f;
		}
		if ( tThumb.fH > tBar.fH ) {
			tThumb.fH = tBar.fH;
		}
		fMaxScroll = fContent - fVisible;
		tThumb.fY = tBar.fY;
		if ( fMaxScroll > 0.0f && tBar.fH > tThumb.fH ) {
			tThumb.fY += (tBar.fH - tThumb.fH) * (pScroll->fScrollY / fMaxScroll);
		}
		xgeShapeRectFillPx(tThumb, pScroll->iThumbColor);
	}
	fVisible = pWidget->tContentRect.fW;
	fContent = pScroll->fContentW;
	if ( fContent > fVisible && fVisible > 0.0f ) {
		tBar.fX = pWidget->tContentRect.fX;
		tBar.fY = pWidget->tContentRect.fY + pWidget->tContentRect.fH - 4.0f;
		tBar.fW = pWidget->tContentRect.fW;
		tBar.fH = 4.0f;
		xgeShapeRectFillPx(tBar, pScroll->iBarColor);
		tThumb = tBar;
		tThumb.fW = tBar.fW * (fVisible / fContent);
		if ( tThumb.fW < 8.0f ) {
			tThumb.fW = 8.0f;
		}
		if ( tThumb.fW > tBar.fW ) {
			tThumb.fW = tBar.fW;
		}
		fMaxScroll = fContent - fVisible;
		tThumb.fX = tBar.fX;
		if ( fMaxScroll > 0.0f && tBar.fW > tThumb.fW ) {
			tThumb.fX += (tBar.fW - tThumb.fW) * (pScroll->fScrollX / fMaxScroll);
		}
		xgeShapeRectFillPx(tThumb, pScroll->iThumbColor);
	}
}
