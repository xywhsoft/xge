static int __xgeXuiPopupPlacementClamp(int iPlacement)
{
	if ( (iPlacement < XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT) || (iPlacement > XGE_XUI_OVERLAY_PLACEMENT_MANUAL) ) {
		return XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT;
	}
	return iPlacement;
}

static int __xgeXuiPopupAnchorClamp(int iAnchor)
{
	if ( (iAnchor < XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT) || (iAnchor > XGE_XUI_POPUP_ANCHOR_FIXED) ) {
		return XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT;
	}
	return iAnchor;
}

static int __xgeXuiPopupDirectionClamp(int iDirection)
{
	if ( (iDirection < XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN) || (iDirection > XGE_XUI_POPUP_DIRECTION_LEFT_UP) ) {
		return XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN;
	}
	return iDirection;
}

static float __xgeXuiPopupMaxf(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static float __xgeXuiPopupMinf(float fA, float fB)
{
	return (fA < fB) ? fA : fB;
}

static float __xgeXuiPopupClampf(float fValue, float fMin, float fMax)
{
	if ( fMax < fMin ) {
		fMax = fMin;
	}
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

static int __xgeXuiPopupRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int __xgeXuiPopupHasAnchorRect(xge_xui_popup pPopup)
{
	return (pPopup != NULL) && (pPopup->tAnchorRect.fX != 0.0f || pPopup->tAnchorRect.fY != 0.0f || pPopup->tAnchorRect.fW > 0.0f || pPopup->tAnchorRect.fH > 0.0f);
}

static int __xgeXuiPopupRectFits(xge_rect_t tRect, float fWindowW, float fWindowH)
{
	return (tRect.fX >= 0.0f) && (tRect.fY >= 0.0f) && (tRect.fX + tRect.fW <= fWindowW) && (tRect.fY + tRect.fH <= fWindowH);
}

static void __xgeXuiPopupWindowSize(xge_xui_popup pPopup, float* pW, float* pH)
{
	float fW;
	float fH;

	fW = 0.0f;
	fH = 0.0f;
	if ( (pPopup != NULL) && (pPopup->pContext != NULL) && (pPopup->pContext->pOverlayRoot != NULL) ) {
		fW = pPopup->pContext->pOverlayRoot->tRect.fW;
		fH = pPopup->pContext->pOverlayRoot->tRect.fH;
	}
	if ( fW <= 0.0f ) {
		fW = __xgeXuiHostGetViewportRect((pPopup != NULL) ? pPopup->pContext : NULL).fW;
	}
	if ( fH <= 0.0f ) {
		fH = __xgeXuiHostGetViewportRect((pPopup != NULL) ? pPopup->pContext : NULL).fH;
	}
	if ( fW <= 0.0f ) {
		fW = 1.0f;
	}
	if ( fH <= 0.0f ) {
		fH = 1.0f;
	}
	if ( pW != NULL ) {
		*pW = fW;
	}
	if ( pH != NULL ) {
		*pH = fH;
	}
}

static int __xgeXuiPopupFlipVertical(int iDirection)
{
	switch ( iDirection ) {
		case XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN: return XGE_XUI_POPUP_DIRECTION_RIGHT_UP;
		case XGE_XUI_POPUP_DIRECTION_RIGHT_UP: return XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN;
		case XGE_XUI_POPUP_DIRECTION_LEFT_DOWN: return XGE_XUI_POPUP_DIRECTION_LEFT_UP;
		case XGE_XUI_POPUP_DIRECTION_LEFT_UP: return XGE_XUI_POPUP_DIRECTION_LEFT_DOWN;
		default: return XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN;
	}
}

static int __xgeXuiPopupFlipAnchorVertical(int iAnchor)
{
	switch ( iAnchor ) {
		case XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT: return XGE_XUI_POPUP_ANCHOR_TOP_LEFT;
		case XGE_XUI_POPUP_ANCHOR_BOTTOM_RIGHT: return XGE_XUI_POPUP_ANCHOR_TOP_RIGHT;
		case XGE_XUI_POPUP_ANCHOR_TOP_LEFT: return XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT;
		case XGE_XUI_POPUP_ANCHOR_TOP_RIGHT: return XGE_XUI_POPUP_ANCHOR_BOTTOM_RIGHT;
		default: return iAnchor;
	}
}

static int __xgeXuiPopupFlipHorizontal(int iDirection)
{
	switch ( iDirection ) {
		case XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN: return XGE_XUI_POPUP_DIRECTION_LEFT_DOWN;
		case XGE_XUI_POPUP_DIRECTION_RIGHT_UP: return XGE_XUI_POPUP_DIRECTION_LEFT_UP;
		case XGE_XUI_POPUP_DIRECTION_LEFT_DOWN: return XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN;
		case XGE_XUI_POPUP_DIRECTION_LEFT_UP: return XGE_XUI_POPUP_DIRECTION_RIGHT_UP;
		default: return XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN;
	}
}

static int __xgeXuiPopupFlipAnchorHorizontal(int iAnchor)
{
	switch ( iAnchor ) {
		case XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT: return XGE_XUI_POPUP_ANCHOR_BOTTOM_RIGHT;
		case XGE_XUI_POPUP_ANCHOR_BOTTOM_RIGHT: return XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT;
		case XGE_XUI_POPUP_ANCHOR_TOP_LEFT: return XGE_XUI_POPUP_ANCHOR_TOP_RIGHT;
		case XGE_XUI_POPUP_ANCHOR_TOP_RIGHT: return XGE_XUI_POPUP_ANCHOR_TOP_LEFT;
		default: return iAnchor;
	}
}

static xge_vec2_t __xgeXuiPopupAnchorPoint(xge_rect_t tAnchor, int iAnchorPoint)
{
	switch ( iAnchorPoint ) {
		case XGE_XUI_POPUP_ANCHOR_BOTTOM_RIGHT:
			return (xge_vec2_t){ tAnchor.fX + tAnchor.fW, tAnchor.fY + tAnchor.fH };
		case XGE_XUI_POPUP_ANCHOR_TOP_LEFT:
			return (xge_vec2_t){ tAnchor.fX, tAnchor.fY };
		case XGE_XUI_POPUP_ANCHOR_TOP_RIGHT:
			return (xge_vec2_t){ tAnchor.fX + tAnchor.fW, tAnchor.fY };
		case XGE_XUI_POPUP_ANCHOR_CURSOR:
		case XGE_XUI_POPUP_ANCHOR_FIXED:
			return (xge_vec2_t){ tAnchor.fX, tAnchor.fY };
		case XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT:
		default:
			return (xge_vec2_t){ tAnchor.fX, tAnchor.fY + tAnchor.fH };
	}
}

static xge_rect_t __xgeXuiPopupRectFrom(xge_vec2_t tAnchor, float fW, float fH, float fGap, int iDirection)
{
	xge_rect_t tRect;

	tRect.fW = fW;
	tRect.fH = fH;
	switch ( iDirection ) {
		case XGE_XUI_POPUP_DIRECTION_RIGHT_UP:
			tRect.fX = tAnchor.fX;
			tRect.fY = tAnchor.fY - fGap - fH;
			break;
		case XGE_XUI_POPUP_DIRECTION_LEFT_DOWN:
			tRect.fX = tAnchor.fX - fW;
			tRect.fY = tAnchor.fY + fGap;
			break;
		case XGE_XUI_POPUP_DIRECTION_LEFT_UP:
			tRect.fX = tAnchor.fX - fW;
			tRect.fY = tAnchor.fY - fGap - fH;
			break;
		case XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN:
		default:
			tRect.fX = tAnchor.fX;
			tRect.fY = tAnchor.fY + fGap;
			break;
	}
	return tRect;
}

static int __xgeXuiPopupOwnerAvailable(xge_xui_popup pPopup)
{
	if ( (pPopup == NULL) || (pPopup->pOwner == NULL) ) {
		return 1;
	}
	return ((pPopup->pOwner->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0) && ((pPopup->pOwner->iFlags & XGE_XUI_WIDGET_ENABLED) != 0);
}

static xge_xui_widget __xgeXuiPopupFindFirstFocusable(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_xui_widget pFound;

	if ( pWidget == NULL ) {
		return NULL;
	}
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( ((pChild->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0) && ((pChild->iFlags & XGE_XUI_WIDGET_ENABLED) != 0) && ((pChild->iFlags & XGE_XUI_WIDGET_FOCUSABLE) != 0) ) {
			return pChild;
		}
		pFound = __xgeXuiPopupFindFirstFocusable(pChild);
		if ( pFound != NULL ) {
			return pFound;
		}
	}
	return NULL;
}

static void __xgeXuiPopupFocusOpenTarget(xge_xui_popup pPopup)
{
	xge_xui_widget pFocus;

	if ( (pPopup == NULL) || (pPopup->pContext == NULL) ) {
		return;
	}
	pFocus = NULL;
	switch ( pPopup->iFocusPolicy ) {
		case XGE_XUI_POPUP_FOCUS_NONE:
			return;
		case XGE_XUI_POPUP_FOCUS_FIRST_CHILD:
			pFocus = __xgeXuiPopupFindFirstFocusable(pPopup->pContentWidget);
			if ( pFocus == NULL ) {
				pFocus = __xgeXuiPopupFindFirstFocusable(pPopup->pWidget);
			}
			break;
		case XGE_XUI_POPUP_FOCUS_CUSTOM:
			pFocus = pPopup->pFocusWidget;
			break;
		case XGE_XUI_POPUP_FOCUS_POPUP:
		default:
			pFocus = pPopup->pWidget;
			break;
	}
	if ( pFocus != NULL ) {
		xgeXuiSetFocus(pPopup->pContext, pFocus);
	}
}

static void __xgeXuiPopupRestoreFocus(xge_xui_popup pPopup)
{
	xge_xui_widget pRestore;

	if ( pPopup == NULL ) {
		return;
	}
	pRestore = pPopup->pFocusRestore;
	if ( (pRestore != NULL) && (pPopup->pContext != NULL) && ((pRestore->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0) && ((pRestore->iFlags & XGE_XUI_WIDGET_ENABLED) != 0) ) {
		xgeXuiSetFocus(pPopup->pContext, pRestore);
	}
	if ( pPopup->bFocusRestoreExplicit == 0 ) {
		pPopup->pFocusRestore = NULL;
	}
}

static void __xgeXuiPopupMapPlacement(xge_xui_popup pPopup)
{
	if ( pPopup == NULL ) {
		return;
	}
	switch ( pPopup->iPlacement ) {
		case XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_RIGHT:
			pPopup->iAnchorPoint = XGE_XUI_POPUP_ANCHOR_BOTTOM_RIGHT;
			pPopup->iDirection = XGE_XUI_POPUP_DIRECTION_LEFT_DOWN;
			break;
		case XGE_XUI_OVERLAY_PLACEMENT_TOP_LEFT:
			pPopup->iAnchorPoint = XGE_XUI_POPUP_ANCHOR_TOP_LEFT;
			pPopup->iDirection = XGE_XUI_POPUP_DIRECTION_RIGHT_UP;
			break;
		case XGE_XUI_OVERLAY_PLACEMENT_TOP_RIGHT:
			pPopup->iAnchorPoint = XGE_XUI_POPUP_ANCHOR_TOP_RIGHT;
			pPopup->iDirection = XGE_XUI_POPUP_DIRECTION_LEFT_UP;
			break;
		case XGE_XUI_OVERLAY_PLACEMENT_RIGHT_TOP:
			pPopup->iAnchorPoint = XGE_XUI_POPUP_ANCHOR_TOP_RIGHT;
			pPopup->iDirection = XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN;
			break;
		case XGE_XUI_OVERLAY_PLACEMENT_LEFT_TOP:
			pPopup->iAnchorPoint = XGE_XUI_POPUP_ANCHOR_TOP_LEFT;
			pPopup->iDirection = XGE_XUI_POPUP_DIRECTION_LEFT_DOWN;
			break;
		case XGE_XUI_OVERLAY_PLACEMENT_CURSOR:
			pPopup->iAnchorPoint = XGE_XUI_POPUP_ANCHOR_CURSOR;
			pPopup->iDirection = XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN;
			break;
		case XGE_XUI_OVERLAY_PLACEMENT_CENTER:
		case XGE_XUI_OVERLAY_PLACEMENT_MANUAL:
			break;
		case XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT:
		default:
			pPopup->iAnchorPoint = XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT;
			pPopup->iDirection = XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN;
			break;
	}
}

static void __xgeXuiPopupSyncRects(xge_xui_popup pPopup)
{
	float fScrollX;
	float fScrollY;
	float fContentW;
	float fContentH;

	if ( (pPopup == NULL) || (pPopup->pScrollView == NULL) ) {
		return;
	}
	pPopup->tViewportRect = xgeXuiScrollFrameGetViewportRect(xgeXuiScrollViewGetFrame(pPopup->pScrollView));
	xgeXuiScrollViewGetOffset(pPopup->pScrollView, &fScrollX, &fScrollY);
	fContentW = (pPopup->tContentRect.fW > 0.0f) ? pPopup->tContentRect.fW : pPopup->fContentW;
	fContentH = (pPopup->tContentRect.fH > 0.0f) ? pPopup->tContentRect.fH : pPopup->fContentH;
	pPopup->tContentRect = (xge_rect_t){ pPopup->tViewportRect.fX - fScrollX, pPopup->tViewportRect.fY - fScrollY, fContentW, fContentH };
}

static void __xgeXuiPopupArrangeScrollView(xge_xui_popup pPopup)
{
	xge_rect_t tLocal;
	xge_rect_t tRect;

	if ( (pPopup == NULL) || (pPopup->pWidget == NULL) || (pPopup->pScrollWidget == NULL) || (pPopup->pScrollView == NULL) ) {
		return;
	}
	tRect = pPopup->pWidget->tContentRect;
	tLocal = (xge_rect_t){ 0.0f, 0.0f, tRect.fW, tRect.fH };
	pPopup->pScrollWidget->tLocalRect = tLocal;
	__xgeXuiWidgetArrangeRect(pPopup->pScrollWidget, tRect);
	xgeXuiScrollViewLayout(pPopup->pScrollView);
	__xgeXuiPopupSyncRects(pPopup);
}

static void __xgeXuiPopupPreserveContentChildren(xge_xui_popup pPopup)
{
	xge_xui_widget pChild;

	if ( (pPopup == NULL) || (pPopup->pWidget == NULL) || (pPopup->pContentWidget == NULL) ) {
		return;
	}
	while ( pPopup->pContentWidget->pFirstChild != NULL ) {
		pChild = pPopup->pContentWidget->pFirstChild;
		xgeXuiWidgetAddInternal(pPopup->pWidget, pChild);
	}
}

static void __xgeXuiPopupClose(xge_xui_popup pPopup)
{
	if ( (pPopup == NULL) || (pPopup->bOpen == 0) ) {
		return;
	}
	pPopup->bOpen = 0;
	pPopup->iCloseCount++;
	if ( pPopup->pContext != NULL && pPopup->pWidget != NULL ) {
		xgeXuiReleaseWidgetCapture(pPopup->pContext, pPopup->pWidget);
	}
	xgeXuiWidgetSetVisible(pPopup->pWidget, 0);
	__xgeXuiPopupRestoreFocus(pPopup);
	if ( pPopup->procClose != NULL ) {
		pPopup->procClose(pPopup->pWidget, pPopup->pUser);
	}
}

int xgeXuiPopupInit(xge_xui_popup pPopup, xge_xui_context pContext, xge_xui_widget pWidget)
{
	int iRet;

	if ( (pPopup == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pPopup, 0, sizeof(*pPopup));
	pPopup->pScrollView = (xge_xui_scroll_view)malloc(sizeof(xge_xui_scroll_view_t));
	pPopup->pScrollWidget = xgeXuiWidgetCreate();
	if ( (pPopup->pScrollView == NULL) || (pPopup->pScrollWidget == NULL) ) {
		free(pPopup->pScrollView);
		xgeXuiWidgetFree(pPopup->pScrollWidget);
		memset(pPopup, 0, sizeof(*pPopup));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	__xgeXuiOverlayWidgetInit(pWidget, 1);
	pPopup->pContext = pContext;
	pPopup->pWidget = pWidget;
	pPopup->iPlacement = XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT;
	pPopup->iAnchorPoint = XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT;
	pPopup->iDirection = XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN;
	pPopup->iOutsidePolicy = XGE_XUI_POPUP_OUTSIDE_CLOSE;
	pPopup->iOwnerPolicy = XGE_XUI_POPUP_OWNER_PASSTHROUGH;
	pPopup->iEscapePolicy = XGE_XUI_POPUP_ESCAPE_CLOSE;
	pPopup->iFocusPolicy = XGE_XUI_POPUP_FOCUS_POPUP;
	pPopup->fGap = 0.0f;
	pPopup->bCloseOnOutside = 1;
	pPopup->bCloseOnEscape = 1;
	xgeXuiWidgetSetEvent(pWidget, xgeXuiPopupEventProc, pPopup);
	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetFocusScope(pWidget, 1);
	xgeXuiWidgetSetLayer(pWidget, XGE_XUI_LAYER_POPUP);
	xgeXuiWidgetSetVisible(pWidget, 0);
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 250, 253, 255));
	pWidget->pUser = pPopup;
	xgeXuiWidgetSetName(pPopup->pScrollWidget, "popup-scrollview");
	xgeXuiWidgetSetBackground(pPopup->pScrollWidget, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeXuiWidgetSetBorder(pPopup->pScrollWidget, 0.0f, 0);
	xgeXuiWidgetSetClip(pPopup->pScrollWidget, 0);
	iRet = xgeXuiScrollViewInit(pPopup->pScrollView, pContext, pPopup->pScrollWidget);
	if ( iRet != XGE_OK ) {
		xgeXuiWidgetFree(pPopup->pScrollWidget);
		free(pPopup->pScrollView);
		memset(pPopup, 0, sizeof(*pPopup));
		return iRet;
	}
	xgeXuiScrollViewSetContentDragEnabled(pPopup->pScrollView, 0);
	xgeXuiWidgetSetBackground(pPopup->pScrollWidget, XGE_COLOR_RGBA(0, 0, 0, 0));
	pPopup->pContentWidget = xgeXuiScrollViewGetContentWidget(pPopup->pScrollView);
	if ( xgeXuiWidgetAddInternal(pWidget, pPopup->pScrollWidget) != XGE_OK ) {
		xgeXuiPopupUnit(pPopup);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	(void)xgeXuiOverlayAttach(pContext, pWidget, NULL, XGE_XUI_LAYER_POPUP);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiPopupUnit(xge_xui_popup pPopup)
{
	xge_xui_scroll_view pScrollView;
	xge_xui_widget pScrollWidget;

	if ( pPopup == NULL ) {
		return;
	}
	if ( pPopup->pWidget != NULL && pPopup->pWidget->pUser == pPopup ) {
		if ( pPopup->pContext != NULL ) {
			xgeXuiReleaseWidgetCapture(pPopup->pContext, pPopup->pWidget);
		}
		pPopup->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pPopup->pWidget, NULL, NULL);
	}
	pScrollView = pPopup->pScrollView;
	pScrollWidget = pPopup->pScrollWidget;
	__xgeXuiPopupPreserveContentChildren(pPopup);
	if ( pScrollView != NULL ) {
		xgeXuiScrollViewUnit(pScrollView);
		free(pScrollView);
	}
	if ( pScrollWidget != NULL ) {
		xgeXuiWidgetFree(pScrollWidget);
	}
	memset(pPopup, 0, sizeof(*pPopup));
}

void xgeXuiPopupSetOwner(xge_xui_popup pPopup, xge_xui_widget pOwner)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->pOwner = pOwner;
	if ( pPopup->pWidget != NULL ) {
		pPopup->pWidget->pOverlayOwner = pOwner;
	}
}

void xgeXuiPopupSetClose(xge_xui_popup pPopup, xge_xui_click_proc procClose, void* pUser)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->procClose = procClose;
	pPopup->pUser = pUser;
}

void xgeXuiPopupSetOpen(xge_xui_popup pPopup, int bOpen)
{
	if ( (pPopup == NULL) || (pPopup->pWidget == NULL) ) {
		return;
	}
	bOpen = bOpen ? 1 : 0;
	if ( pPopup->bOpen == bOpen ) {
		if ( bOpen ) {
			(void)xgeXuiOverlayAttach(pPopup->pContext, pPopup->pWidget, pPopup->pOwner, XGE_XUI_LAYER_POPUP);
			xgeXuiOverlayBringToFront(pPopup->pContext, pPopup->pWidget);
			xgeXuiPopupApplyPlacement(pPopup);
		}
		return;
	}
	if ( bOpen && __xgeXuiPopupOwnerAvailable(pPopup) == 0 ) {
		return;
	}
	pPopup->bOpen = bOpen;
	xgeXuiWidgetSetVisible(pPopup->pWidget, bOpen);
	if ( bOpen ) {
		(void)xgeXuiOverlayAttach(pPopup->pContext, pPopup->pWidget, pPopup->pOwner, XGE_XUI_LAYER_POPUP);
		xgeXuiOverlayBringToFront(pPopup->pContext, pPopup->pWidget);
		if ( pPopup->bFocusRestoreExplicit == 0 && pPopup->pContext != NULL ) {
			pPopup->pFocusRestore = pPopup->pContext->pFocus;
		}
		xgeXuiPopupSetScroll(pPopup, 0.0f, 0.0f);
		xgeXuiPopupApplyPlacement(pPopup);
		__xgeXuiPopupFocusOpenTarget(pPopup);
	} else {
		if ( pPopup->pContext != NULL ) {
			xgeXuiReleaseWidgetCapture(pPopup->pContext, pPopup->pWidget);
		}
		__xgeXuiPopupRestoreFocus(pPopup);
	}
	xgeXuiWidgetMarkPaint(pPopup->pWidget);
}

int xgeXuiPopupIsOpen(xge_xui_popup pPopup)
{
	return (pPopup != NULL) ? pPopup->bOpen : 0;
}

void xgeXuiPopupSetModal(xge_xui_popup pPopup, int bModal)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->bModal = bModal ? 1 : 0;
}

void xgeXuiPopupSetAutoClose(xge_xui_popup pPopup, int bOutside, int bEscape)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->bCloseOnOutside = bOutside ? 1 : 0;
	pPopup->bCloseOnEscape = bEscape ? 1 : 0;
	pPopup->iOutsidePolicy = pPopup->bCloseOnOutside ? XGE_XUI_POPUP_OUTSIDE_CLOSE : XGE_XUI_POPUP_OUTSIDE_IGNORE;
	pPopup->iEscapePolicy = pPopup->bCloseOnEscape ? XGE_XUI_POPUP_ESCAPE_CLOSE : XGE_XUI_POPUP_ESCAPE_IGNORE;
}

void xgeXuiPopupSetPlacement(xge_xui_popup pPopup, int iPlacement)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->iPlacement = __xgeXuiPopupPlacementClamp(iPlacement);
	__xgeXuiPopupMapPlacement(pPopup);
	xgeXuiPopupApplyPlacement(pPopup);
}

void xgeXuiPopupSetAnchorRect(xge_xui_popup pPopup, xge_rect_t tAnchor)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->tAnchorRect = tAnchor;
	xgeXuiPopupApplyPlacement(pPopup);
}

void xgeXuiPopupSetOffset(xge_xui_popup pPopup, float fX, float fY)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->fOffsetX = fX;
	pPopup->fOffsetY = fY;
	xgeXuiPopupApplyPlacement(pPopup);
}

void xgeXuiPopupSetFocusRestore(xge_xui_popup pPopup, xge_xui_widget pWidget)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->pFocusRestore = pWidget;
	pPopup->bFocusRestoreExplicit = (pWidget != NULL);
}

void xgeXuiPopupSetContentWidget(xge_xui_popup pPopup, xge_xui_widget pContent)
{
	if ( (pPopup == NULL) || (pPopup->pContentWidget == NULL) ) {
		return;
	}
	if ( pContent != NULL ) {
		xgeXuiWidgetAddInternal(pPopup->pContentWidget, pContent);
	}
	xgeXuiPopupApplyPlacement(pPopup);
}

void xgeXuiPopupSetContentSize(xge_xui_popup pPopup, float fW, float fH)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->fContentW = __xgeXuiPopupMaxf(0.0f, fW);
	pPopup->fContentH = __xgeXuiPopupMaxf(0.0f, fH);
	pPopup->tContentRect.fW = pPopup->fContentW;
	pPopup->tContentRect.fH = pPopup->fContentH;
	xgeXuiPopupApplyPlacement(pPopup);
}

void xgeXuiPopupSetAnchorPoint(xge_xui_popup pPopup, int iAnchorPoint)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->iAnchorPoint = __xgeXuiPopupAnchorClamp(iAnchorPoint);
	xgeXuiPopupApplyPlacement(pPopup);
}

void xgeXuiPopupSetDirection(xge_xui_popup pPopup, int iDirection)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->iDirection = __xgeXuiPopupDirectionClamp(iDirection);
	xgeXuiPopupApplyPlacement(pPopup);
}

void xgeXuiPopupSetGap(xge_xui_popup pPopup, float fGap)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->fGap = __xgeXuiPopupMaxf(0.0f, fGap);
	xgeXuiPopupApplyPlacement(pPopup);
}

void xgeXuiPopupSetMatchOwnerWidth(xge_xui_popup pPopup, int bEnabled)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->bMatchOwnerWidth = bEnabled ? 1 : 0;
	xgeXuiPopupApplyPlacement(pPopup);
}

void xgeXuiPopupSetConsumeInside(xge_xui_popup pPopup, int bEnabled)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->bConsumeInside = bEnabled ? 1 : 0;
}

void xgeXuiPopupSetClosePolicy(xge_xui_popup pPopup, int iOutsidePolicy, int iOwnerPolicy, int iEscapePolicy)
{
	if ( pPopup == NULL ) {
		return;
	}
	if ( (iOutsidePolicy >= XGE_XUI_POPUP_OUTSIDE_CLOSE) && (iOutsidePolicy <= XGE_XUI_POPUP_OUTSIDE_CONSUME) ) {
		pPopup->iOutsidePolicy = iOutsidePolicy;
		pPopup->bCloseOnOutside = (iOutsidePolicy == XGE_XUI_POPUP_OUTSIDE_CLOSE);
	}
	if ( (iOwnerPolicy >= XGE_XUI_POPUP_OWNER_PASSTHROUGH) && (iOwnerPolicy <= XGE_XUI_POPUP_OWNER_CONSUME) ) {
		pPopup->iOwnerPolicy = iOwnerPolicy;
	}
	if ( (iEscapePolicy >= XGE_XUI_POPUP_ESCAPE_CLOSE) && (iEscapePolicy <= XGE_XUI_POPUP_ESCAPE_IGNORE) ) {
		pPopup->iEscapePolicy = iEscapePolicy;
		pPopup->bCloseOnEscape = (iEscapePolicy == XGE_XUI_POPUP_ESCAPE_CLOSE);
	}
}

void xgeXuiPopupSetFocusPolicy(xge_xui_popup pPopup, int iFocusPolicy, xge_xui_widget pCustomFocus)
{
	if ( pPopup == NULL ) {
		return;
	}
	if ( (iFocusPolicy < XGE_XUI_POPUP_FOCUS_NONE) || (iFocusPolicy > XGE_XUI_POPUP_FOCUS_CUSTOM) ) {
		iFocusPolicy = XGE_XUI_POPUP_FOCUS_POPUP;
	}
	pPopup->iFocusPolicy = iFocusPolicy;
	pPopup->pFocusWidget = pCustomFocus;
}

xge_rect_t xgeXuiPopupGetViewportRect(xge_xui_popup pPopup)
{
	if ( pPopup == NULL ) {
		return (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	}
	__xgeXuiPopupSyncRects(pPopup);
	return pPopup->tViewportRect;
}

xge_rect_t xgeXuiPopupGetContentRect(xge_xui_popup pPopup)
{
	if ( pPopup == NULL ) {
		return (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	}
	__xgeXuiPopupSyncRects(pPopup);
	return pPopup->tContentRect;
}

void xgeXuiPopupSetScroll(xge_xui_popup pPopup, float fX, float fY)
{
	if ( (pPopup == NULL) || (pPopup->pScrollView == NULL) ) {
		return;
	}
	xgeXuiScrollViewSetOffset(pPopup->pScrollView, fX, fY);
	__xgeXuiPopupSyncRects(pPopup);
}

void xgeXuiPopupGetScroll(xge_xui_popup pPopup, float* pX, float* pY)
{
	if ( pPopup != NULL && pPopup->pScrollView != NULL ) {
		xgeXuiScrollViewGetOffset(pPopup->pScrollView, pX, pY);
		return;
	}
	if ( pX != NULL ) {
		*pX = 0.0f;
	}
	if ( pY != NULL ) {
		*pY = 0.0f;
	}
}

static void __xgeXuiPopupResolveScrollFrameSize(xge_xui_popup pPopup, float fWindowW, float fWindowH, float fFrameW, float fFrameH, float fContentW, float fContentH, float* pOuterW, float* pOuterH)
{
	xge_xui_scroll_frame pFrame;
	float fBar;
	float fMaxFrameW;
	float fMaxFrameH;
	float fViewportW;
	float fViewportH;
	int bNeedH;
	int bNeedV;
	int bNextH;
	int bNextV;
	int i;

	pFrame = (pPopup != NULL && pPopup->pScrollView != NULL) ? xgeXuiScrollViewGetFrame(pPopup->pScrollView) : NULL;
	fBar = __xgeXuiScrollFrameScrollbarReserveSize(pFrame);
	fMaxFrameW = __xgeXuiPopupMaxf(1.0f, fWindowW - fFrameW);
	fMaxFrameH = __xgeXuiPopupMaxf(1.0f, fWindowH - fFrameH);
	bNeedH = 0;
	bNeedV = 0;
	for ( i = 0; i < 3; i++ ) {
		fViewportW = fMaxFrameW - (bNeedV ? fBar : 0.0f);
		fViewportH = fMaxFrameH - (bNeedH ? fBar : 0.0f);
		if ( fViewportW < 1.0f ) {
			fViewportW = 1.0f;
		}
		if ( fViewportH < 1.0f ) {
			fViewportH = 1.0f;
		}
		bNextH = (fContentW > fViewportW);
		bNextV = (fContentH > fViewportH);
		if ( (bNextH == bNeedH) && (bNextV == bNeedV) ) {
			break;
		}
		bNeedH = bNextH;
		bNeedV = bNextV;
	}
	fViewportW = fMaxFrameW - (bNeedV ? fBar : 0.0f);
	fViewportH = fMaxFrameH - (bNeedH ? fBar : 0.0f);
	if ( fViewportW < 1.0f ) {
		fViewportW = 1.0f;
	}
	if ( fViewportH < 1.0f ) {
		fViewportH = 1.0f;
	}
	fViewportW = __xgeXuiPopupMinf(fContentW, fViewportW);
	fViewportH = __xgeXuiPopupMinf(fContentH, fViewportH);
	if ( pOuterW != NULL ) {
		*pOuterW = fViewportW + (bNeedV ? fBar : 0.0f) + fFrameW;
	}
	if ( pOuterH != NULL ) {
		*pOuterH = fViewportH + (bNeedH ? fBar : 0.0f) + fFrameH;
	}
}

void xgeXuiPopupApplyPlacement(xge_xui_popup pPopup)
{
	xge_rect_t tAnchor;
	xge_rect_t tRect;
	xge_rect_t arrCandidate[4];
	xge_vec2_t tPoint;
	float fWindowW;
	float fWindowH;
	float fContentW;
	float fContentH;
	float fFrameW;
	float fFrameH;
	float fOuterW;
	float fOuterH;
	int arrDirection[4];
	int arrAnchorPoint[4];
	int i;

	if ( (pPopup == NULL) || (pPopup->pWidget == NULL) || (pPopup->pScrollView == NULL) ) {
		return;
	}
	__xgeXuiPopupWindowSize(pPopup, &fWindowW, &fWindowH);
	fFrameW = (pPopup->pWidget->tStyle.fBorderWidth > 0.0f) ? pPopup->pWidget->tStyle.fBorderWidth * 2.0f : 0.0f;
	fFrameH = fFrameW;
	tAnchor = pPopup->tAnchorRect;
	if ( !__xgeXuiPopupHasAnchorRect(pPopup) && pPopup->pOwner != NULL ) {
		tAnchor = pPopup->pOwner->tRect;
	}
	fContentW = pPopup->fContentW;
	fContentH = pPopup->fContentH;
	if ( fContentW <= 0.0f ) {
		fContentW = (pPopup->pContentWidget != NULL && pPopup->pContentWidget->tLocalRect.fW > 0.0f) ? pPopup->pContentWidget->tLocalRect.fW : pPopup->pWidget->tRect.fW - fFrameW;
	}
	if ( fContentH <= 0.0f ) {
		fContentH = (pPopup->pContentWidget != NULL && pPopup->pContentWidget->tLocalRect.fH > 0.0f) ? pPopup->pContentWidget->tLocalRect.fH : pPopup->pWidget->tRect.fH - fFrameH;
	}
	if ( pPopup->bMatchOwnerWidth && tAnchor.fW > 0.0f ) {
		fContentW = tAnchor.fW - fFrameW;
	}
	fContentW = __xgeXuiPopupMaxf(1.0f, fContentW);
	fContentH = __xgeXuiPopupMaxf(1.0f, fContentH);
	__xgeXuiPopupResolveScrollFrameSize(pPopup, fWindowW, fWindowH, fFrameW, fFrameH, fContentW, fContentH, &fOuterW, &fOuterH);
	pPopup->tContentRect.fW = fContentW;
	pPopup->tContentRect.fH = fContentH;
	xgeXuiScrollViewSetContentSize(pPopup->pScrollView, fContentW, fContentH);
	if ( pPopup->iPlacement == XGE_XUI_OVERLAY_PLACEMENT_MANUAL ) {
		__xgeXuiPopupArrangeScrollView(pPopup);
		return;
	}
	if ( pPopup->iPlacement == XGE_XUI_OVERLAY_PLACEMENT_CENTER ) {
		tRect = (xge_rect_t){ (fWindowW - fOuterW) * 0.5f, (fWindowH - fOuterH) * 0.5f, fOuterW, fOuterH };
	} else {
		arrDirection[0] = pPopup->iDirection;
		arrDirection[1] = __xgeXuiPopupFlipVertical(pPopup->iDirection);
		arrDirection[2] = __xgeXuiPopupFlipHorizontal(pPopup->iDirection);
		arrDirection[3] = __xgeXuiPopupFlipHorizontal(arrDirection[1]);
		arrAnchorPoint[0] = pPopup->iAnchorPoint;
		arrAnchorPoint[1] = __xgeXuiPopupFlipAnchorVertical(pPopup->iAnchorPoint);
		arrAnchorPoint[2] = __xgeXuiPopupFlipAnchorHorizontal(pPopup->iAnchorPoint);
		arrAnchorPoint[3] = __xgeXuiPopupFlipAnchorHorizontal(arrAnchorPoint[1]);
		for ( i = 0; i < 4; i++ ) {
			tPoint = __xgeXuiPopupAnchorPoint(tAnchor, arrAnchorPoint[i]);
			arrCandidate[i] = __xgeXuiPopupRectFrom(tPoint, fOuterW, fOuterH, pPopup->fGap, arrDirection[i]);
			arrCandidate[i].fX += pPopup->fOffsetX;
			arrCandidate[i].fY += pPopup->fOffsetY;
			if ( __xgeXuiPopupRectFits(arrCandidate[i], fWindowW, fWindowH) ) {
				break;
			}
		}
		if ( i >= 4 ) {
			i = 0;
			arrCandidate[0].fX = __xgeXuiPopupClampf(arrCandidate[0].fX, 0.0f, fWindowW - fOuterW);
			arrCandidate[0].fY = __xgeXuiPopupClampf(arrCandidate[0].fY, 0.0f, fWindowH - fOuterH);
		}
		tRect = arrCandidate[i];
	}
	xgeXuiWidgetSetRect(pPopup->pWidget, tRect);
	__xgeXuiPopupArrangeScrollView(pPopup);
}

void xgeXuiPopupSetBackground(xge_xui_popup pPopup, uint32_t iColor)
{
	if ( pPopup == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pPopup->pWidget, iColor);
}

void xgeXuiPopupSetBorder(xge_xui_popup pPopup, uint32_t iColor)
{
	if ( pPopup == NULL ) {
		return;
	}
	xgeXuiWidgetSetBorder(pPopup->pWidget, (XGE_COLOR_GET_A(iColor) != 0) ? 1.0f : 0.0f, iColor);
	xgeXuiPopupApplyPlacement(pPopup);
}

int xgeXuiPopupEvent(xge_xui_popup pPopup, const xge_event_t* pEvent)
{
	int bInside;
	int bOwner;

	if ( (pPopup == NULL) || (pPopup->pWidget == NULL) || (pEvent == NULL) || (pPopup->bOpen == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( __xgeXuiPopupOwnerAvailable(pPopup) == 0 ) {
		__xgeXuiPopupClose(pPopup);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN ) {
		if ( (pEvent->iParam1 == XGE_KEY_ESCAPE) && (pPopup->iEscapePolicy == XGE_XUI_POPUP_ESCAPE_CLOSE) ) {
			__xgeXuiPopupClose(pPopup);
			return XGE_XUI_EVENT_CONSUMED;
		}
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType != XGE_EVENT_MOUSE_DOWN) && (pEvent->iType != XGE_EVENT_TOUCH_BEGIN) &&
	     (pEvent->iType != XGE_EVENT_MOUSE_UP) && (pEvent->iType != XGE_EVENT_TOUCH_END) &&
	     (pEvent->iType != XGE_EVENT_TOUCH_CANCEL) && (pEvent->iType != XGE_EVENT_MOUSE_WHEEL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	bInside = __xgeXuiPopupRectContains(pPopup->pWidget->tRect, pEvent->fX, pEvent->fY);
	if ( bInside ) {
		__xgeXuiPopupFocusOpenTarget(pPopup);
		return pPopup->bConsumeInside ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
	}
	bOwner = (pPopup->pOwner != NULL) && __xgeXuiPopupRectContains(pPopup->pOwner->tRect, pEvent->fX, pEvent->fY);
	if ( bOwner ) {
		if ( pPopup->iOwnerPolicy == XGE_XUI_POPUP_OWNER_CLOSE ) {
			__xgeXuiPopupClose(pPopup);
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( pPopup->iOwnerPolicy == XGE_XUI_POPUP_OWNER_TOGGLE ) {
			__xgeXuiPopupClose(pPopup);
			return XGE_XUI_EVENT_CONTINUE;
		}
		return (pPopup->iOwnerPolicy == XGE_XUI_POPUP_OWNER_CONSUME) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) || (pEvent->iType == XGE_EVENT_TOUCH_BEGIN) || (pEvent->iType == XGE_EVENT_MOUSE_WHEEL) ) {
		if ( pPopup->iOutsidePolicy == XGE_XUI_POPUP_OUTSIDE_CLOSE ) {
			__xgeXuiPopupClose(pPopup);
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( pPopup->iOutsidePolicy == XGE_XUI_POPUP_OUTSIDE_CONSUME ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		return pPopup->bModal ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int xgeXuiPopupEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiPopupEvent((xge_xui_popup)pUser, pEvent);
}

void xgeXuiPopupPaintProc(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
}
