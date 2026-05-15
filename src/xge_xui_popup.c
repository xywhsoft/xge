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

static int __xgeXuiPopupVerticalBar(xge_xui_popup pPopup, xge_rect_t* pBar, xge_rect_t* pThumb)
{
	float fTrack;
	float fThumb;
	float fRatio;

	if ( (pPopup == NULL) || (pPopup->tContentRect.fH <= pPopup->tViewportRect.fH) ) {
		return 0;
	}
	*pBar = (xge_rect_t){ pPopup->tViewportRect.fX + pPopup->tViewportRect.fW - 5.0f, pPopup->tViewportRect.fY + 2.0f, 3.0f, pPopup->tViewportRect.fH - 4.0f };
	fTrack = __xgeXuiPopupMaxf(1.0f, pBar->fH);
	fThumb = __xgeXuiPopupMaxf(18.0f, fTrack * (pPopup->tViewportRect.fH / pPopup->tContentRect.fH));
	fThumb = __xgeXuiPopupMinf(fThumb, fTrack);
	fRatio = pPopup->fScrollY / __xgeXuiPopupMaxf(1.0f, pPopup->tContentRect.fH - pPopup->tViewportRect.fH);
	*pThumb = (xge_rect_t){ pBar->fX, pBar->fY + (fTrack - fThumb) * fRatio, pBar->fW, fThumb };
	return 1;
}

static int __xgeXuiPopupHorizontalBar(xge_xui_popup pPopup, xge_rect_t* pBar, xge_rect_t* pThumb)
{
	float fTrack;
	float fThumb;
	float fRatio;

	if ( (pPopup == NULL) || (pPopup->tContentRect.fW <= pPopup->tViewportRect.fW) ) {
		return 0;
	}
	*pBar = (xge_rect_t){ pPopup->tViewportRect.fX + 2.0f, pPopup->tViewportRect.fY + pPopup->tViewportRect.fH - 5.0f, pPopup->tViewportRect.fW - 4.0f, 3.0f };
	fTrack = __xgeXuiPopupMaxf(1.0f, pBar->fW);
	fThumb = __xgeXuiPopupMaxf(18.0f, fTrack * (pPopup->tViewportRect.fW / pPopup->tContentRect.fW));
	fThumb = __xgeXuiPopupMinf(fThumb, fTrack);
	fRatio = pPopup->fScrollX / __xgeXuiPopupMaxf(1.0f, pPopup->tContentRect.fW - pPopup->tViewportRect.fW);
	*pThumb = (xge_rect_t){ pBar->fX + (fTrack - fThumb) * fRatio, pBar->fY, fThumb, pBar->fH };
	return 1;
}

static void __xgeXuiPopupScrollFromThumbDrag(xge_xui_popup pPopup, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fTrack;
	float fMovable;
	float fContentMovable;

	if ( (pPopup == NULL) || (pEvent == NULL) ) {
		return;
	}
	if ( pPopup->iScrollDragPart == 1 && __xgeXuiPopupVerticalBar(pPopup, &tBar, &tThumb) ) {
		fTrack = __xgeXuiPopupMaxf(1.0f, tBar.fH);
		fMovable = __xgeXuiPopupMaxf(1.0f, fTrack - tThumb.fH);
		fContentMovable = __xgeXuiPopupMaxf(0.0f, pPopup->tContentRect.fH - pPopup->tViewportRect.fH);
		xgeXuiPopupSetScroll(pPopup, pPopup->fDragScrollX, pPopup->fDragScrollY + ((pEvent->fY - pPopup->fDragY) / fMovable) * fContentMovable);
	} else if ( pPopup->iScrollDragPart == 2 && __xgeXuiPopupHorizontalBar(pPopup, &tBar, &tThumb) ) {
		fTrack = __xgeXuiPopupMaxf(1.0f, tBar.fW);
		fMovable = __xgeXuiPopupMaxf(1.0f, fTrack - tThumb.fW);
		fContentMovable = __xgeXuiPopupMaxf(0.0f, pPopup->tContentRect.fW - pPopup->tViewportRect.fW);
		xgeXuiPopupSetScroll(pPopup, pPopup->fDragScrollX + ((pEvent->fX - pPopup->fDragX) / fMovable) * fContentMovable, pPopup->fDragScrollY);
	}
}

static int __xgeXuiPopupOwnerAvailable(xge_xui_popup pPopup)
{
	if ( (pPopup == NULL) || (pPopup->pOwner == NULL) ) {
		return 1;
	}
	return ((pPopup->pOwner->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0) && ((pPopup->pOwner->iFlags & XGE_XUI_WIDGET_ENABLED) != 0);
}

static int __xgeXuiPopupHasAnchorRect(xge_xui_popup pPopup)
{
	return (pPopup != NULL) && (pPopup->tAnchorRect.fX != 0.0f || pPopup->tAnchorRect.fY != 0.0f || pPopup->tAnchorRect.fW > 0.0f || pPopup->tAnchorRect.fH > 0.0f);
}

static int __xgeXuiPopupRectFits(xge_rect_t tRect, float fWindowW, float fWindowH)
{
	return (tRect.fX >= 0.0f) && (tRect.fY >= 0.0f) && (tRect.fX + tRect.fW <= fWindowW) && (tRect.fY + tRect.fH <= fWindowH);
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
			pFocus = __xgeXuiPopupFindFirstFocusable(pPopup->pWidget);
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

static void __xgeXuiPopupAutoContent(xge_xui_popup pPopup)
{
	if ( (pPopup == NULL) || (pPopup->pWidget == NULL) || (pPopup->pContentWidget != NULL) ) {
		return;
	}
	if ( (pPopup->fContentW > 0.0f || pPopup->fContentH > 0.0f) && pPopup->pWidget->pFirstChild != NULL ) {
		pPopup->pContentWidget = pPopup->pWidget->pFirstChild;
	}
}

static void __xgeXuiPopupLayoutContent(xge_xui_popup pPopup)
{
	float fMaxX;
	float fMaxY;
	xge_rect_t tContent;

	if ( (pPopup == NULL) || (pPopup->pWidget == NULL) ) {
		return;
	}
	__xgeXuiPopupAutoContent(pPopup);
	fMaxX = __xgeXuiPopupMaxf(0.0f, pPopup->tContentRect.fW - pPopup->tViewportRect.fW);
	fMaxY = __xgeXuiPopupMaxf(0.0f, pPopup->tContentRect.fH - pPopup->tViewportRect.fH);
	pPopup->fScrollX = __xgeXuiPopupClampf(pPopup->fScrollX, 0.0f, fMaxX);
	pPopup->fScrollY = __xgeXuiPopupClampf(pPopup->fScrollY, 0.0f, fMaxY);
	if ( pPopup->pContentWidget != NULL ) {
		tContent = (xge_rect_t){ -pPopup->fScrollX, -pPopup->fScrollY, pPopup->tContentRect.fW, pPopup->tContentRect.fH };
		xgeXuiWidgetSetRect(pPopup->pContentWidget, tContent);
	}
	xgeXuiWidgetMarkPaint(pPopup->pWidget);
}

static void __xgeXuiPopupClearInteraction(xge_xui_popup pPopup)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->iScrollDragPart = 0;
	xgeXuiReleaseWidgetCapture(pPopup->pContext, pPopup->pWidget);
}

static void __xgeXuiPopupClose(xge_xui_popup pPopup)
{
	if ( (pPopup == NULL) || (pPopup->bOpen == 0) ) {
		return;
	}
	pPopup->bOpen = 0;
	pPopup->iCloseCount++;
	__xgeXuiPopupClearInteraction(pPopup);
	xgeXuiWidgetSetVisible(pPopup->pWidget, 0);
	__xgeXuiPopupRestoreFocus(pPopup);
	if ( pPopup->procClose != NULL ) {
		pPopup->procClose(pPopup->pWidget, pPopup->pUser);
	}
}

int xgeXuiPopupInit(xge_xui_popup pPopup, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pPopup == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pPopup, 0, sizeof(*pPopup));
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
	xgeXuiWidgetSetEvent(pWidget, xgeXuiPopupEventProc, NULL);
	xgeXuiWidgetSetPaintAfter(pWidget, xgeXuiPopupPaintProc, pPopup);
	pWidget->pUser = pPopup;
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetLayer(pWidget, XGE_XUI_LAYER_POPUP);
	xgeXuiWidgetSetVisible(pWidget, 0);
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 250, 253, 255));
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiPopupUnit(xge_xui_popup pPopup)
{
	if ( pPopup == NULL ) {
		return;
	}
	if ( pPopup->pWidget != NULL && pPopup->pWidget->pUser == pPopup ) {
		__xgeXuiPopupClearInteraction(pPopup);
		pPopup->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pPopup->pWidget, NULL, NULL);
		xgeXuiWidgetSetPaintAfter(pPopup->pWidget, NULL, NULL);
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
		xgeXuiOverlayBringToFront(pPopup->pContext, pPopup->pWidget);
		if ( pPopup->bFocusRestoreExplicit == 0 && pPopup->pContext != NULL ) {
			pPopup->pFocusRestore = pPopup->pContext->pFocus;
		}
		xgeXuiPopupApplyPlacement(pPopup);
		__xgeXuiPopupFocusOpenTarget(pPopup);
	} else {
		__xgeXuiPopupClearInteraction(pPopup);
		__xgeXuiPopupRestoreFocus(pPopup);
	}
	xgeXuiWidgetMarkPaint(pPopup->pWidget);
}

int xgeXuiPopupIsOpen(xge_xui_popup pPopup)
{
	if ( pPopup == NULL ) {
		return 0;
	}
	return pPopup->bOpen;
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
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->pContentWidget = pContent;
	if ( (pPopup->pWidget != NULL) && (pContent != NULL) && (pContent->pParent != pPopup->pWidget) ) {
		xgeXuiWidgetAddInternal(pPopup->pWidget, pContent);
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
	return pPopup->tViewportRect;
}

xge_rect_t xgeXuiPopupGetContentRect(xge_xui_popup pPopup)
{
	if ( pPopup == NULL ) {
		return (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	}
	return pPopup->tContentRect;
}

void xgeXuiPopupSetScroll(xge_xui_popup pPopup, float fX, float fY)
{
	if ( pPopup == NULL ) {
		return;
	}
	pPopup->fScrollX = fX;
	pPopup->fScrollY = fY;
	__xgeXuiPopupLayoutContent(pPopup);
}

void xgeXuiPopupGetScroll(xge_xui_popup pPopup, float* pX, float* pY)
{
	if ( pX != NULL ) {
		*pX = (pPopup != NULL) ? pPopup->fScrollX : 0.0f;
	}
	if ( pY != NULL ) {
		*pY = (pPopup != NULL) ? pPopup->fScrollY : 0.0f;
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
	float fViewportW;
	float fViewportH;
	int arrDirection[4];
	int arrAnchorPoint[4];
	int i;

	if ( (pPopup == NULL) || (pPopup->pWidget == NULL) ) {
		return;
	}
	fWindowW = (float)xgeGetWidth();
	fWindowH = (float)xgeGetHeight();
	if ( fWindowW <= 0.0f ) {
		fWindowW = __xgeXuiPopupMaxf(1.0f, pPopup->pWidget->tRect.fX + pPopup->pWidget->tRect.fW);
	}
	if ( fWindowH <= 0.0f ) {
		fWindowH = __xgeXuiPopupMaxf(1.0f, pPopup->pWidget->tRect.fY + pPopup->pWidget->tRect.fH);
	}
	fContentW = pPopup->fContentW;
	fContentH = pPopup->fContentH;
	if ( fContentW <= 0.0f ) {
		fContentW = (pPopup->pContentWidget != NULL && pPopup->pContentWidget->tRect.fW > 0.0f) ? pPopup->pContentWidget->tRect.fW : pPopup->pWidget->tRect.fW;
	}
	if ( fContentH <= 0.0f ) {
		fContentH = (pPopup->pContentWidget != NULL && pPopup->pContentWidget->tRect.fH > 0.0f) ? pPopup->pContentWidget->tRect.fH : pPopup->pWidget->tRect.fH;
	}
	tAnchor = pPopup->tAnchorRect;
	if ( !__xgeXuiPopupHasAnchorRect(pPopup) && pPopup->pOwner != NULL ) {
		tAnchor = pPopup->pOwner->tRect;
	}
	if ( pPopup->bMatchOwnerWidth && tAnchor.fW > 0.0f ) {
		fContentW = tAnchor.fW;
	}
	fContentW = __xgeXuiPopupMaxf(1.0f, fContentW);
	fContentH = __xgeXuiPopupMaxf(1.0f, fContentH);
	fViewportW = __xgeXuiPopupMinf(fContentW, fWindowW);
	fViewportH = __xgeXuiPopupMinf(fContentH, fWindowH);
	pPopup->tContentRect = (xge_rect_t){ 0.0f, 0.0f, fContentW, fContentH };
	pPopup->bScrollEnabled = (fViewportW < fContentW || fViewportH < fContentH);
	if ( pPopup->iPlacement == XGE_XUI_OVERLAY_PLACEMENT_MANUAL ) {
		pPopup->tViewportRect = pPopup->pWidget->tRect;
		__xgeXuiPopupLayoutContent(pPopup);
		return;
	}
	if ( pPopup->iPlacement == XGE_XUI_OVERLAY_PLACEMENT_CENTER ) {
		tRect = (xge_rect_t){ (fWindowW - fViewportW) * 0.5f, (fWindowH - fViewportH) * 0.5f, fViewportW, fViewportH };
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
			arrCandidate[i] = __xgeXuiPopupRectFrom(tPoint, fViewportW, fViewportH, pPopup->fGap, arrDirection[i]);
			arrCandidate[i].fX += pPopup->fOffsetX;
			arrCandidate[i].fY += pPopup->fOffsetY;
			if ( __xgeXuiPopupRectFits(arrCandidate[i], fWindowW, fWindowH) ) {
				break;
			}
		}
		if ( i >= 4 ) {
			i = 0;
			arrCandidate[0].fX = __xgeXuiPopupClampf(arrCandidate[0].fX, 0.0f, fWindowW - fViewportW);
			arrCandidate[0].fY = __xgeXuiPopupClampf(arrCandidate[0].fY, 0.0f, fWindowH - fViewportH);
		}
		tRect = arrCandidate[i];
	}
	pPopup->tViewportRect = tRect;
	xgeXuiWidgetSetRect(pPopup->pWidget, tRect);
	__xgeXuiPopupLayoutContent(pPopup);
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
}

int xgeXuiPopupEvent(xge_xui_popup pPopup, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;

	if ( (pPopup == NULL) || (pPopup->pWidget == NULL) || (pEvent == NULL) || (pPopup->bOpen == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( __xgeXuiPopupOwnerAvailable(pPopup) == 0 ) {
		__xgeXuiPopupClose(pPopup);
		return XGE_XUI_EVENT_CONSUMED;
	}
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pPopup->iScrollDragPart != 0 ) {
				if ( xgeXuiGetPointerCapture(pPopup->pContext, pEvent->iPointerId) != pPopup->pWidget ) {
					return XGE_XUI_EVENT_CONTINUE;
				}
				__xgeXuiPopupScrollFromThumbDrag(pPopup, pEvent);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_WHEEL:
			if ( pPopup->bScrollEnabled && __xgeXuiPopupRectContains(pPopup->pWidget->tRect, pEvent->fX, pEvent->fY) ) {
				xgeXuiPopupSetScroll(pPopup, pPopup->fScrollX - pEvent->fDX * 32.0f, pPopup->fScrollY - pEvent->fDY * 32.0f);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( __xgeXuiPopupRectContains(pPopup->pWidget->tRect, pEvent->fX, pEvent->fY) ) {
				__xgeXuiPopupFocusOpenTarget(pPopup);
				if ( pPopup->bScrollEnabled && __xgeXuiPopupVerticalBar(pPopup, &tBar, &tThumb) && __xgeXuiPopupRectContains(tBar, pEvent->fX, pEvent->fY) ) {
					if ( __xgeXuiPopupRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
						pPopup->iScrollDragPart = 1;
						pPopup->fDragX = pEvent->fX;
						pPopup->fDragY = pEvent->fY;
						pPopup->fDragScrollX = pPopup->fScrollX;
						pPopup->fDragScrollY = pPopup->fScrollY;
						xgeXuiSetPointerCapture(pPopup->pContext, pEvent->iPointerId, pPopup->pWidget);
					} else {
						xgeXuiPopupSetScroll(pPopup, pPopup->fScrollX, pPopup->fScrollY + ((pEvent->fY < tThumb.fY) ? -pPopup->tViewportRect.fH : pPopup->tViewportRect.fH));
					}
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( pPopup->bScrollEnabled && __xgeXuiPopupHorizontalBar(pPopup, &tBar, &tThumb) && __xgeXuiPopupRectContains(tBar, pEvent->fX, pEvent->fY) ) {
					if ( __xgeXuiPopupRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
						pPopup->iScrollDragPart = 2;
						pPopup->fDragX = pEvent->fX;
						pPopup->fDragY = pEvent->fY;
						pPopup->fDragScrollX = pPopup->fScrollX;
						pPopup->fDragScrollY = pPopup->fScrollY;
						xgeXuiSetPointerCapture(pPopup->pContext, pEvent->iPointerId, pPopup->pWidget);
					} else {
						xgeXuiPopupSetScroll(pPopup, pPopup->fScrollX + ((pEvent->fX < tThumb.fX) ? -pPopup->tViewportRect.fW : pPopup->tViewportRect.fW), pPopup->fScrollY);
					}
					return XGE_XUI_EVENT_CONSUMED;
				}
				return pPopup->bConsumeInside ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pPopup->pOwner != NULL) && __xgeXuiPopupRectContains(pPopup->pOwner->tRect, pEvent->fX, pEvent->fY) ) {
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
			if ( pPopup->iOutsidePolicy == XGE_XUI_POPUP_OUTSIDE_CLOSE ) {
				__xgeXuiPopupClose(pPopup);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pPopup->iOutsidePolicy == XGE_XUI_POPUP_OUTSIDE_CONSUME ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			return pPopup->bModal ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			if ( pPopup->iScrollDragPart == 0 ) {
				if ( pPopup->bConsumeInside && __xgeXuiPopupRectContains(pPopup->pWidget->tRect, pEvent->fX, pEvent->fY) ) {
					return XGE_XUI_EVENT_CONSUMED;
				}
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( xgeXuiGetPointerCapture(pPopup->pContext, pEvent->iPointerId) == pPopup->pWidget ) {
				__xgeXuiPopupScrollFromThumbDrag(pPopup, pEvent);
				xgeXuiSetPointerCapture(pPopup->pContext, pEvent->iPointerId, NULL);
			}
			pPopup->iScrollDragPart = 0;
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			if ( pPopup->iScrollDragPart == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pPopup->iScrollDragPart = 0;
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_KEY_DOWN:
			if ( (pEvent->iParam1 == XGE_KEY_ESCAPE) && (pPopup->iEscapePolicy == XGE_XUI_POPUP_ESCAPE_CLOSE) ) {
				__xgeXuiPopupClose(pPopup);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiPopupEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiPopupEvent((xge_xui_popup)pUser, pEvent);
}

void xgeXuiPopupPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_popup pPopup;
	xge_rect_t tBar;
	xge_rect_t tThumb;

	(void)pWidget;
	pPopup = (xge_xui_popup)pUser;
	if ( (pPopup == NULL) || (pPopup->bScrollEnabled == 0) ) {
		return;
	}
	if ( __xgeXuiPopupVerticalBar(pPopup, &tBar, &tThumb) ) {
		__xgeXuiHostDrawRect(tBar, XGE_COLOR_RGBA(215, 226, 235, 255));
		__xgeXuiHostDrawRect(tThumb, XGE_COLOR_RGBA(96, 136, 174, 255));
	}
	if ( __xgeXuiPopupHorizontalBar(pPopup, &tBar, &tThumb) ) {
		__xgeXuiHostDrawRect(tBar, XGE_COLOR_RGBA(215, 226, 235, 255));
		__xgeXuiHostDrawRect(tThumb, XGE_COLOR_RGBA(96, 136, 174, 255));
	}
}
