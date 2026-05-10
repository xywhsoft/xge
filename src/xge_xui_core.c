static xge_xui_context g_pXgeXuiProcFrameDispatchHead = NULL;

static xge_xui_context __xgeXuiProcFrameDispatchNext(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return NULL;
	}
	return (xge_xui_context)pContext->pInternal;
}

static void __xgeXuiProcFrameDispatchLink(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return;
	}
	pContext->pInternal = g_pXgeXuiProcFrameDispatchHead;
	g_pXgeXuiProcFrameDispatchHead = pContext;
}

static void __xgeXuiProcFrameDispatchUnlink(xge_xui_context pContext)
{
	xge_xui_context pPrev;
	xge_xui_context pIt;

	if ( pContext == NULL ) {
		return;
	}
	pPrev = NULL;
	for ( pIt = g_pXgeXuiProcFrameDispatchHead; pIt != NULL; pIt = __xgeXuiProcFrameDispatchNext(pIt) ) {
		if ( pIt == pContext ) {
			if ( pPrev == NULL ) {
				g_pXgeXuiProcFrameDispatchHead = __xgeXuiProcFrameDispatchNext(pIt);
			} else {
				pPrev->pInternal = pIt->pInternal;
			}
			pIt->pInternal = NULL;
			return;
		}
		pPrev = pIt;
	}
}

static void __xgeXuiDispatchWidgetEvent(xge_xui_widget pWidget, int iType);
static void __xgeXuiDispatchWidgetPointerEvent(xge_xui_widget pWidget, int iType, uint64_t iPointerId);
static void __xgeXuiWidgetDetachContextRefs(xge_xui_context pContext, xge_xui_widget pWidget);
static int __xgeXuiWidgetContainsWidget(xge_xui_widget pRoot, xge_xui_widget pWidget);
static int __xgeXuiWidgetCanAction(xge_xui_widget pWidget);

static int __xgeXuiHotKeyNormalizeModifiers(int iModifiers)
{
	return iModifiers & (XGE_KEY_MOD_SHIFT | XGE_KEY_MOD_CTRL | XGE_KEY_MOD_ALT | XGE_KEY_MOD_SUPER);
}

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

static int __xgeXuiLayerClamp(int iLayer)
{
	if ( (iLayer < XGE_XUI_LAYER_NORMAL) || (iLayer > XGE_XUI_LAYER_DEBUG) ) {
		return XGE_XUI_LAYER_NORMAL;
	}
	return iLayer;
}

static int __xgeXuiImeModeClamp(int iImeMode)
{
	if ( (iImeMode < XGE_XUI_IME_DISABLED) || (iImeMode > XGE_XUI_IME_AUTO) ) {
		return XGE_XUI_IME_DISABLED;
	}
	return iImeMode;
}

static int __xgeXuiWidgetWantsIme(xge_xui_widget pWidget)
{
	int iImeMode;

	if ( pWidget == NULL ) {
		return 0;
	}
	iImeMode = __xgeXuiImeModeClamp(pWidget->iImeMode);
	return (iImeMode == XGE_XUI_IME_ENABLED) || (iImeMode == XGE_XUI_IME_AUTO);
}

static xge_rect_t __xgeXuiZeroRect(void)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	return tRect;
}

static xge_rect_t __xgeXuiWidgetResolveImeCandidateRect(xge_xui_widget pWidget)
{
	xge_rect_t tRect;

	if ( pWidget == NULL ) {
		return __xgeXuiZeroRect();
	}
	if ( pWidget->procImeCandidateRect != NULL ) {
		return pWidget->procImeCandidateRect(pWidget, pWidget->pImeCandidateRectUser);
	}
	tRect = pWidget->tContentRect;
	if ( (tRect.fW <= 0.0f) && (tRect.fH <= 0.0f) ) {
		tRect = pWidget->tBorderRect;
	}
	if ( (tRect.fW <= 0.0f) && (tRect.fH <= 0.0f) ) {
		tRect = pWidget->tRect;
	}
	return tRect;
}

static void __xgeXuiContextUpdateImeCandidateRect(xge_xui_context pContext, int bEnabled)
{
	if ( pContext == NULL ) {
		return;
	}
	if ( bEnabled && (pContext->pFocus != NULL) ) {
		pContext->tImeCandidateRect = __xgeXuiWidgetResolveImeCandidateRect(pContext->pFocus);
		pContext->bImeCandidateRectValid = 1;
		return;
	}
	pContext->tImeCandidateRect = __xgeXuiZeroRect();
	pContext->bImeCandidateRectValid = 0;
}

static void __xgeXuiContextSyncIme(xge_xui_context pContext)
{
	int bEnabled;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	bEnabled = __xgeXuiWidgetWantsIme(pContext->pFocus);
	__xgeXuiContextUpdateImeCandidateRect(pContext, bEnabled);
	if ( pContext->bImeManaged == 0 ) {
		pContext->bImeEnabledPrev = xgeImeGetEnabled();
		pContext->bImeManaged = 1;
		pContext->bImeEnabled = bEnabled ? 1 : 0;
		(void)xgeImeSetEnabled(bEnabled);
		return;
	}
	if ( pContext->bImeEnabled != (bEnabled ? 1 : 0) ) {
		pContext->bImeEnabled = bEnabled ? 1 : 0;
		(void)xgeImeSetEnabled(bEnabled);
	}
}

static void __xgeXuiContextRestoreIme(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bImeManaged == 0) ) {
		return;
	}
	(void)xgeImeSetEnabled(pContext->bImeEnabledPrev);
	__xgeXuiContextUpdateImeCandidateRect(pContext, 0);
	pContext->bImeManaged = 0;
	pContext->bImeEnabled = 0;
}

static int __xgeXuiChildPaintBefore(xge_xui_widget pA, xge_xui_widget pB)
{
	int iLayerA;
	int iLayerB;

	if ( pB == NULL ) {
		return 1;
	}
	if ( pA == NULL ) {
		return 0;
	}
	iLayerA = __xgeXuiLayerClamp(pA->tStyle.iLayer);
	iLayerB = __xgeXuiLayerClamp(pB->tStyle.iLayer);
	if ( iLayerA != iLayerB ) {
		return iLayerA < iLayerB;
	}
	if ( pA->tStyle.iZ != pB->tStyle.iZ ) {
		return pA->tStyle.iZ < pB->tStyle.iZ;
	}
	if ( pA->iTreeOrder != pB->iTreeOrder ) {
		return pA->iTreeOrder < pB->iTreeOrder;
	}
	return __xgeXuiChildIsAfter(pB, pA);
}

static int __xgeXuiChildHitBefore(xge_xui_widget pA, xge_xui_widget pB)
{
	int iLayerA;
	int iLayerB;

	if ( pB == NULL ) {
		return 1;
	}
	if ( pA == NULL ) {
		return 0;
	}
	iLayerA = __xgeXuiLayerClamp(pA->tStyle.iLayer);
	iLayerB = __xgeXuiLayerClamp(pB->tStyle.iLayer);
	if ( iLayerA != iLayerB ) {
		return iLayerA > iLayerB;
	}
	if ( pA->tStyle.iZ != pB->tStyle.iZ ) {
		return pA->tStyle.iZ > pB->tStyle.iZ;
	}
	if ( pA->iTreeOrder != pB->iTreeOrder ) {
		return pA->iTreeOrder > pB->iTreeOrder;
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

static void* __xgeXuiWidgetEventUser(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return NULL;
	}
	return ((pWidget->iCallbackFlags & XGE_XUI_WIDGET_CALLBACK_EVENT) != 0) ? pWidget->pEventUser : pWidget->pUser;
}

static void* __xgeXuiWidgetUpdateUser(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return NULL;
	}
	return ((pWidget->iCallbackFlags & XGE_XUI_WIDGET_CALLBACK_UPDATE) != 0) ? pWidget->pUpdateUser : pWidget->pUser;
}

static void* __xgeXuiWidgetPaintUser(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return NULL;
	}
	return ((pWidget->iCallbackFlags & XGE_XUI_WIDGET_CALLBACK_PAINT) != 0) ? pWidget->pPaintUser : pWidget->pUser;
}

static void* __xgeXuiWidgetPaintBeforeUser(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return NULL;
	}
	return ((pWidget->iCallbackFlags & XGE_XUI_WIDGET_CALLBACK_PAINT_BEFORE) != 0) ? pWidget->pPaintBeforeUser : pWidget->pUser;
}

static void* __xgeXuiWidgetPaintAfterUser(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return NULL;
	}
	return ((pWidget->iCallbackFlags & XGE_XUI_WIDGET_CALLBACK_PAINT_AFTER) != 0) ? pWidget->pPaintAfterUser : pWidget->pUser;
}

static int __xgeXuiOwnerDrawModeClamp(int iMode)
{
	if ( (iMode < XGE_XUI_OWNER_DRAW_NONE) || (iMode > XGE_XUI_OWNER_DRAW_FULL) ) {
		return XGE_XUI_OWNER_DRAW_NONE;
	}
	return iMode;
}

static int __xgeXuiWidgetPaintState(xge_xui_context pContext, xge_xui_widget pWidget)
{
	int iState;

	iState = XGE_XUI_STATE_NORMAL;
	if ( (pWidget == NULL) || ((pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( (pContext != NULL) && (pWidget != NULL) ) {
		if ( pContext->pFocus == pWidget ) {
			iState |= XGE_XUI_STATE_FOCUS;
		}
		if ( (pContext->pHover == pWidget) || __xgeXuiWidgetContainsWidget(pWidget, pContext->pHover) ) {
			iState |= XGE_XUI_STATE_HOVER;
		}
		if ( __xgeXuiWidgetContainsWidget(pWidget, pContext->pCapture) ) {
			iState |= XGE_XUI_STATE_ACTIVE;
		}
	}
	return iState;
}

static int __xgeXuiWidgetStateStyleIndex(int iState)
{
	if ( (iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return 4;
	}
	if ( (iState & XGE_XUI_STATE_FOCUS) != 0 ) {
		return 3;
	}
	if ( (iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		return 2;
	}
	if ( (iState & XGE_XUI_STATE_HOVER) != 0 ) {
		return 1;
	}
	if ( (iState & XGE_XUI_STATE_CHECKED) != 0 ) {
		return 5;
	}
	return 0;
}

static void __xgeXuiWidgetApplyStateStyle(xge_xui_style_t* pDst, const xge_xui_state_style_t* pSrc)
{
	if ( (pDst == NULL) || (pSrc == NULL) || (pSrc->iMask == 0) ) {
		return;
	}
	if ( (pSrc->iMask & XGE_XUI_STATE_STYLE_BACKGROUND) != 0 ) {
		pDst->iBackgroundColor = pSrc->iBackgroundColor;
	}
	if ( (pSrc->iMask & XGE_XUI_STATE_STYLE_BORDER_COLOR) != 0 ) {
		pDst->iBorderColor = pSrc->iBorderColor;
	}
	if ( (pSrc->iMask & XGE_XUI_STATE_STYLE_BORDER_WIDTH) != 0 ) {
		pDst->fBorderWidth = (pSrc->fBorderWidth > 0.0f) ? pSrc->fBorderWidth : 0.0f;
	}
	if ( (pSrc->iMask & XGE_XUI_STATE_STYLE_FOCUS_RING_COLOR) != 0 ) {
		pDst->iFocusRingColor = pSrc->iFocusRingColor;
	}
	if ( (pSrc->iMask & XGE_XUI_STATE_STYLE_FOCUS_RING_WIDTH) != 0 ) {
		pDst->fFocusRingWidth = (pSrc->fFocusRingWidth > 0.0f) ? pSrc->fFocusRingWidth : 0.0f;
	}
	if ( (pSrc->iMask & XGE_XUI_STATE_STYLE_DISABLED_OVERLAY) != 0 ) {
		pDst->iDisabledOverlayColor = pSrc->iDisabledOverlayColor;
	}
}

static xge_xui_style_t __xgeXuiWidgetEffectivePaintStyle(xge_xui_context pContext, xge_xui_widget pWidget)
{
	xge_xui_style_t tStyle;
	int iState;

	memset(&tStyle, 0, sizeof(tStyle));
	if ( pWidget == NULL ) {
		return tStyle;
	}
	tStyle = pWidget->tStyle;
	iState = pWidget->iVisualState;
	if ( (pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( (pContext != NULL) && (pContext->pFocus == pWidget) ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	__xgeXuiWidgetApplyStateStyle(&tStyle, &pWidget->arrStateStyle[0]);
	if ( (iState & XGE_XUI_STATE_CHECKED) != 0 ) {
		__xgeXuiWidgetApplyStateStyle(&tStyle, &pWidget->arrStateStyle[5]);
	}
	if ( (iState & XGE_XUI_STATE_HOVER) != 0 ) {
		__xgeXuiWidgetApplyStateStyle(&tStyle, &pWidget->arrStateStyle[1]);
	}
	if ( (iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		__xgeXuiWidgetApplyStateStyle(&tStyle, &pWidget->arrStateStyle[2]);
	}
	if ( (iState & XGE_XUI_STATE_FOCUS) != 0 ) {
		__xgeXuiWidgetApplyStateStyle(&tStyle, &pWidget->arrStateStyle[3]);
	}
	if ( (iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		__xgeXuiWidgetApplyStateStyle(&tStyle, &pWidget->arrStateStyle[4]);
	}
	return tStyle;
}

static xge_xui_paint_info_t __xgeXuiPaintInfo(xge_xui_context pContext, xge_xui_widget pWidget, int iMode, int iPart)
{
	xge_xui_paint_info_t tInfo;

	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.pContext = pContext;
	tInfo.pWidget = pWidget;
	tInfo.iRole = (pWidget != NULL) ? xgeXuiWidgetGetRole(pWidget) : XGE_XUI_WIDGET_ROLE_CONTAINER;
	tInfo.iState = __xgeXuiWidgetPaintState(pContext, pWidget);
	tInfo.iOwnerDrawMode = iMode;
	tInfo.iPart = iPart;
	if ( pWidget != NULL ) {
		tInfo.tOuterRect = pWidget->tOuterRect;
		tInfo.tBorderRect = pWidget->tBorderRect;
		tInfo.tPaddingRect = pWidget->tPaddingRect;
		tInfo.tContentRect = pWidget->tContentRect;
		tInfo.pStyle = &pWidget->tStyle;
		tInfo.pControl = pWidget->pOwnerDrawControl;
	}
	tInfo.fDipScale = (pContext != NULL) ? xgeXuiGetDipScale(pContext) : g_fXgeXuiActiveDipScale;
	tInfo.iItemIndex = -1;
	tInfo.iRow = -1;
	tInfo.iColumn = -1;
	return tInfo;
}

static int __xgeXuiWidgetOwnerDrawEnabled(xge_xui_widget pWidget)
{
	return (pWidget != NULL) && (pWidget->procOwnerDraw != NULL) && (__xgeXuiOwnerDrawModeClamp(pWidget->iOwnerDrawMode) != XGE_XUI_OWNER_DRAW_NONE);
}

static int __xgeXuiWidgetOwnerDraw(xge_xui_context pContext, xge_xui_widget pWidget, int iPart)
{
	xge_xui_paint_info_t tInfo;
	xge_xui_widget pOldPaintWidget;
	int iMode;

	if ( !__xgeXuiWidgetOwnerDrawEnabled(pWidget) ) {
		return 0;
	}
	iMode = __xgeXuiOwnerDrawModeClamp(pWidget->iOwnerDrawMode);
	tInfo = __xgeXuiPaintInfo(pContext, pWidget, iMode, iPart);
	pOldPaintWidget = g_pXgeXuiActivePaintWidget;
	g_pXgeXuiActivePaintWidget = pWidget;
	pWidget->procOwnerDraw(&tInfo, pWidget->pOwnerDrawUser);
	g_pXgeXuiActivePaintWidget = pOldPaintWidget;
	return 1;
}

static void __xgeXuiWidgetClearPaintDirtySubtree(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;

	if ( pWidget == NULL ) {
		return;
	}
	pWidget->iFlags &= ~XGE_XUI_WIDGET_DIRTY_PAINT;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xgeXuiWidgetClearPaintDirtySubtree(pChild);
	}
}

static xge_rect_t __xgeXuiRectIntersection(xge_rect_t tA, xge_rect_t tB)
{
	xge_rect_t tRect;
	float fA2;
	float fB2;
	float fRight;
	float fBottom;

	tRect.fX = (tA.fX > tB.fX) ? tA.fX : tB.fX;
	tRect.fY = (tA.fY > tB.fY) ? tA.fY : tB.fY;
	fA2 = tA.fX + tA.fW;
	fB2 = tB.fX + tB.fW;
	fRight = (fA2 < fB2) ? fA2 : fB2;
	fA2 = tA.fY + tA.fH;
	fB2 = tB.fY + tB.fH;
	fBottom = (fA2 < fB2) ? fA2 : fB2;
	tRect.fW = fRight - tRect.fX;
	tRect.fH = fBottom - tRect.fY;
	if ( tRect.fW < 0.0f ) {
		tRect.fW = 0.0f;
	}
	if ( tRect.fH < 0.0f ) {
		tRect.fH = 0.0f;
	}
	return tRect;
}

static void __xgeXuiPaintClipRestoreBase(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return;
	}
	(void)xgeFlush();
	if ( pContext->bPaintClipBaseEnabled ) {
		__xgeXuiHostClipSet(pContext->tPaintClipBaseRect);
	} else {
		__xgeXuiHostClipClear();
	}
}

static void __xgeXuiPaintClipBegin(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return;
	}
	pContext->iPaintClipStackCount = 0;
	pContext->bPaintClipBaseEnabled = g_xge.bClipEnabled;
	pContext->tPaintClipBaseRect = g_xge.tClipRect;
}

static void __xgeXuiPaintClipEnd(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return;
	}
	if ( pContext->iPaintClipStackCount > 0 ) {
		pContext->iPaintClipStackCount = 0;
		__xgeXuiPaintClipRestoreBase(pContext);
	}
}

static int __xgeXuiPaintClipPush(xge_xui_context pContext, xge_rect_t tRect)
{
	xge_rect_t tClip;

	if ( pContext == NULL ) {
		return 0;
	}
	if ( pContext->iPaintClipStackCount >= XGE_XUI_CLIP_STACK_MAX ) {
		return 0;
	}
	tClip = tRect;
	if ( pContext->iPaintClipStackCount > 0 ) {
		tClip = __xgeXuiRectIntersection(pContext->arrPaintClipStack[pContext->iPaintClipStackCount - 1], tClip);
	} else if ( pContext->bPaintClipBaseEnabled ) {
		tClip = __xgeXuiRectIntersection(pContext->tPaintClipBaseRect, tClip);
	}
	pContext->arrPaintClipStack[pContext->iPaintClipStackCount] = tClip;
	pContext->iPaintClipStackCount++;
	(void)xgeFlush();
	__xgeXuiHostClipSet(tClip);
	return 1;
}

static void __xgeXuiPaintClipPop(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->iPaintClipStackCount <= 0) ) {
		return;
	}
	pContext->iPaintClipStackCount--;
	(void)xgeFlush();
	if ( pContext->iPaintClipStackCount > 0 ) {
		__xgeXuiHostClipSet(pContext->arrPaintClipStack[pContext->iPaintClipStackCount - 1]);
	} else if ( pContext->bPaintClipBaseEnabled ) {
		__xgeXuiHostClipSet(pContext->tPaintClipBaseRect);
	} else {
		__xgeXuiHostClipClear();
	}
}

static int __xgeXuiPaintWidget(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_xui_widget pOldPaintWidget;
	xge_xui_context pContext;
	int iCount;
	int bUseClip;
	int bClipPushed;
	int iOwnerMode;
	int bOwnerDraw;
	int bFullOwnerDraw;
	xge_xui_style_t tPaintStyle;

	if ( (pWidget == NULL) || ((pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) ) {
		return 0;
	}
	pContext = g_xgeXuiActiveContext;
	iCount = 0;
	iOwnerMode = __xgeXuiOwnerDrawModeClamp(pWidget->iOwnerDrawMode);
	bOwnerDraw = __xgeXuiWidgetOwnerDrawEnabled(pWidget);
	bFullOwnerDraw = bOwnerDraw && (iOwnerMode == XGE_XUI_OWNER_DRAW_FULL);
	tPaintStyle = __xgeXuiWidgetEffectivePaintStyle(pContext, pWidget);
	if ( bFullOwnerDraw ) {
		iCount += __xgeXuiWidgetOwnerDraw(pContext, pWidget, XGE_XUI_PAINT_PART_WIDGET);
		__xgeXuiWidgetClearPaintDirtySubtree(pWidget);
		return iCount;
	}
	if ( pWidget->procPaintBefore != NULL ) {
		pOldPaintWidget = g_pXgeXuiActivePaintWidget;
		g_pXgeXuiActivePaintWidget = pWidget;
		pWidget->procPaintBefore(pWidget, __xgeXuiWidgetPaintBeforeUser(pWidget));
		g_pXgeXuiActivePaintWidget = pOldPaintWidget;
		iCount++;
	}
	if ( XGE_COLOR_GET_A(tPaintStyle.iBackgroundColor) != 0 ) {
		if ( tPaintStyle.fRadius > 0.0f ) {
			__xgeXuiHostDrawRoundedRect(pWidget->tBorderRect, tPaintStyle.iBackgroundColor, tPaintStyle.fRadius);
		} else {
			__xgeXuiHostDrawRect(pWidget->tBorderRect, tPaintStyle.iBackgroundColor);
		}
		iCount++;
	}
	bUseClip = ((pWidget->iFlags & XGE_XUI_WIDGET_CLIP) != 0);
	bClipPushed = bUseClip ? __xgeXuiPaintClipPush(pContext, pWidget->tContentRect) : 0;
	if ( bOwnerDraw ) {
		iCount += __xgeXuiWidgetOwnerDraw(pContext, pWidget, XGE_XUI_PAINT_PART_CONTENT);
	} else if ( pWidget->procPaint != NULL ) {
		pOldPaintWidget = g_pXgeXuiActivePaintWidget;
		g_pXgeXuiActivePaintWidget = pWidget;
		pWidget->procPaint(pWidget, __xgeXuiWidgetPaintUser(pWidget));
		g_pXgeXuiActivePaintWidget = pOldPaintWidget;
		iCount++;
	}
	pWidget->iFlags &= ~XGE_XUI_WIDGET_DIRTY_PAINT;
	if ( (!bOwnerDraw) || (iOwnerMode != XGE_XUI_OWNER_DRAW_CONTENT_AND_CHILDREN) ) {
		for ( pChild = __xgeXuiChildNextPaint(pWidget, NULL); pChild != NULL; pChild = __xgeXuiChildNextPaint(pWidget, pChild) ) {
			iCount += __xgeXuiPaintWidget(pChild);
		}
	} else {
		for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
			__xgeXuiWidgetClearPaintDirtySubtree(pChild);
		}
	}
	if ( bClipPushed ) {
		__xgeXuiPaintClipPop(pContext);
	}
	if ( ((pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) && (XGE_COLOR_GET_A(tPaintStyle.iDisabledOverlayColor) != 0) ) {
		__xgeXuiHostDrawRect(pWidget->tBorderRect, tPaintStyle.iDisabledOverlayColor);
		iCount++;
	}
	if ( (tPaintStyle.fBorderWidth > 0.0f) && (XGE_COLOR_GET_A(tPaintStyle.iBorderColor) != 0) ) {
		__xgeXuiHostDrawBorderRect(pWidget->tBorderRect, tPaintStyle.fBorderWidth, tPaintStyle.iBorderColor);
		iCount++;
	}
	if ( (pContext != NULL) && (pContext->pFocus == pWidget) && (tPaintStyle.fFocusRingWidth > 0.0f) && (XGE_COLOR_GET_A(tPaintStyle.iFocusRingColor) != 0) ) {
		__xgeXuiHostDrawBorderRect(pWidget->tBorderRect, tPaintStyle.fFocusRingWidth, tPaintStyle.iFocusRingColor);
		iCount++;
	}
	if ( (pWidget->tStyle.fDebugOutlineWidth > 0.0f) && (XGE_COLOR_GET_A(pWidget->tStyle.iDebugOutlineColor) != 0) ) {
		__xgeXuiHostDrawBorderRect(pWidget->tBorderRect, pWidget->tStyle.fDebugOutlineWidth, pWidget->tStyle.iDebugOutlineColor);
		iCount++;
	}
	return iCount;
}

static int __xgeXuiPaintWidgetAfterAll(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_xui_widget pOldPaintWidget;
	int iCount;

	if ( (pWidget == NULL) || ((pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) ) {
		return 0;
	}
	if ( __xgeXuiWidgetOwnerDrawEnabled(pWidget) && (__xgeXuiOwnerDrawModeClamp(pWidget->iOwnerDrawMode) == XGE_XUI_OWNER_DRAW_FULL) ) {
		return 0;
	}
	iCount = 0;
	if ( (!__xgeXuiWidgetOwnerDrawEnabled(pWidget)) || (__xgeXuiOwnerDrawModeClamp(pWidget->iOwnerDrawMode) != XGE_XUI_OWNER_DRAW_CONTENT_AND_CHILDREN) ) {
		for ( pChild = __xgeXuiChildNextPaint(pWidget, NULL); pChild != NULL; pChild = __xgeXuiChildNextPaint(pWidget, pChild) ) {
			iCount += __xgeXuiPaintWidgetAfterAll(pChild);
		}
	}
	if ( pWidget->procPaintAfter != NULL ) {
		pOldPaintWidget = g_pXgeXuiActivePaintWidget;
		g_pXgeXuiActivePaintWidget = pWidget;
		pWidget->procPaintAfter(pWidget, __xgeXuiWidgetPaintAfterUser(pWidget));
		g_pXgeXuiActivePaintWidget = pOldPaintWidget;
		iCount++;
	}
	return iCount;
}

static void __xgeXuiUpdateWidget(xge_xui_widget pWidget, float fDelta)
{
	xge_xui_widget pChild;

	if ( (pWidget == NULL) || ((pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) ) {
		return;
	}
	if ( pWidget->procUpdate != NULL ) {
		pWidget->procUpdate(pWidget, fDelta, __xgeXuiWidgetUpdateUser(pWidget));
	}
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xgeXuiUpdateWidget(pChild, fDelta);
	}
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

static int __xgeXuiContextPressIsPrimaryDown(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return (pEvent->iType == XGE_EVENT_TOUCH_BEGIN) || ((pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 == XGE_MOUSE_LEFT));
}

static int __xgeXuiContextPressIsRelease(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return (pEvent->iType == XGE_EVENT_TOUCH_END) || ((pEvent->iType == XGE_EVENT_MOUSE_UP) && (pEvent->iParam1 == XGE_MOUSE_LEFT));
}

static int __xgeXuiContextPressIsCancel(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return pEvent->iType == XGE_EVENT_TOUCH_CANCEL;
}

static int __xgeXuiContextPressIsMove(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return (pEvent->iType == XGE_EVENT_TOUCH_MOVE) || (pEvent->iType == XGE_EVENT_MOUSE_MOVE);
}

static void __xgeXuiContextPressCancel(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return;
	}
	pContext->bContextPressActive = 0;
	pContext->bContextPressMoved = 0;
	pContext->bContextPressFired = 0;
	pContext->bContextRightActive = 0;
	pContext->fContextPressTime = 0.0f;
	pContext->iContextPressPointerId = 0;
	pContext->pContextPressTarget = NULL;
}

static void __xgeXuiClickPressCancel(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return;
	}
	pContext->bClickPressActive = 0;
	pContext->bClickPressMoved = 0;
	pContext->iClickPressButton = 0;
	pContext->iClickPressPointerId = 0;
	pContext->pClickPressTarget = NULL;
}

static void __xgeXuiDragCancelState(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return;
	}
	pContext->bDragPressActive = 0;
	pContext->bDragActive = 0;
	pContext->iDragPointerId = 0;
	pContext->pDragTarget = NULL;
}

static int __xgeXuiWidgetCanFocus(xge_xui_widget pWidget)
{
	xge_xui_widget pIt;

	if ( pWidget == NULL ) {
		return 0;
	}
	if ( (pWidget->iFlags & XGE_XUI_WIDGET_FOCUSABLE) == 0 ) {
		return 0;
	}
	for ( pIt = pWidget; pIt != NULL; pIt = pIt->pParent ) {
		if ( ((pIt->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || ((pIt->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
			return 0;
		}
	}
	return 1;
}

static int __xgeXuiWidgetCanTabFocus(xge_xui_widget pWidget)
{
	return __xgeXuiWidgetCanFocus(pWidget) && ((pWidget->iFlags & XGE_XUI_WIDGET_TAB_STOP) != 0);
}

static int __xgeXuiWidgetCanAction(xge_xui_widget pWidget)
{
	xge_xui_widget pIt;

	if ( pWidget == NULL ) {
		return 0;
	}
	for ( pIt = pWidget; pIt != NULL; pIt = pIt->pParent ) {
		if ( ((pIt->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || ((pIt->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
			return 0;
		}
	}
	return 1;
}

static int __xgeXuiWidgetCanFocusScope(xge_xui_widget pWidget)
{
	xge_xui_widget pIt;

	if ( (pWidget == NULL) || ((pWidget->iFlags & XGE_XUI_WIDGET_FOCUS_SCOPE) == 0) ) {
		return 0;
	}
	for ( pIt = pWidget; pIt != NULL; pIt = pIt->pParent ) {
		if ( ((pIt->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || ((pIt->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
			return 0;
		}
	}
	return 1;
}

static int __xgeXuiWidgetCanHitTest(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return 0;
	}
	return ((pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0) && ((pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) != 0) && ((pWidget->iFlags & XGE_XUI_WIDGET_HIT_TEST_VISIBLE) != 0);
}

static int __xgeXuiOverflowClamp(int iOverflow)
{
	if ( (iOverflow < XGE_XUI_OVERFLOW_VISIBLE) || (iOverflow > XGE_XUI_OVERFLOW_SCROLL) ) {
		return XGE_XUI_OVERFLOW_VISIBLE;
	}
	return iOverflow;
}

static int __xgeXuiOverflowClips(int iOverflow)
{
	iOverflow = __xgeXuiOverflowClamp(iOverflow);
	return (iOverflow == XGE_XUI_OVERFLOW_CLIP) || (iOverflow == XGE_XUI_OVERFLOW_HIDDEN) || (iOverflow == XGE_XUI_OVERFLOW_SCROLL);
}

static void __xgeXuiWidgetSyncClipFlag(xge_xui_widget pWidget)
{
	uint32_t iOldFlags;
	int bClip;

	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.iOverflow = __xgeXuiOverflowClamp(pWidget->tStyle.iOverflow);
	bClip = (pWidget->tStyle.iClip != 0) || __xgeXuiOverflowClips(pWidget->tStyle.iOverflow);
	iOldFlags = pWidget->iFlags;
	if ( bClip ) {
		pWidget->iFlags |= XGE_XUI_WIDGET_CLIP;
	} else {
		pWidget->iFlags &= ~XGE_XUI_WIDGET_CLIP;
	}
	if ( iOldFlags != pWidget->iFlags ) {
		xgeXuiWidgetMarkLayout(pWidget);
		xgeXuiWidgetMarkPaint(pWidget);
	}
}

static int __xgeXuiWidgetInterceptsHit(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return 0;
	}
	return (pWidget->procEvent != NULL) || (pWidget->procCaptureEvent != NULL) || (pWidget->iEventMask != 0) || __xgeXuiWidgetCanFocus(pWidget);
}

static uint32_t __xgeXuiEventMaskFromType(int iType)
{
	switch ( iType ) {
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			return XGE_XUI_EVENT_MASK_MOUSE_DOWN;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			return XGE_XUI_EVENT_MASK_MOUSE_UP;
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			return XGE_XUI_EVENT_MASK_MOUSE_MOVE;
		case XGE_EVENT_MOUSE_WHEEL:
			return XGE_XUI_EVENT_MASK_MOUSE_WHEEL;
		case XGE_EVENT_XUI_POINTER_ENTER:
			return XGE_XUI_EVENT_MASK_MOUSE_ENTER;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			return XGE_XUI_EVENT_MASK_MOUSE_LEAVE;
		case XGE_EVENT_XUI_CLICK:
			return XGE_XUI_EVENT_MASK_CLICK;
		case XGE_EVENT_XUI_DOUBLE_CLICK:
			return XGE_XUI_EVENT_MASK_DOUBLE_CLICK;
		case XGE_EVENT_XUI_CONTEXT_BEGIN:
		case XGE_EVENT_XUI_CONTEXT_UPDATE:
		case XGE_EVENT_XUI_CONTEXT_END:
		case XGE_EVENT_XUI_CONTEXT_CANCEL:
		case XGE_EVENT_XUI_CONTEXT_MENU:
			return XGE_XUI_EVENT_MASK_CONTEXT_MENU;
		case XGE_EVENT_KEY_DOWN:
			return XGE_XUI_EVENT_MASK_KEY_DOWN;
		case XGE_EVENT_KEY_UP:
			return XGE_XUI_EVENT_MASK_KEY_UP;
		case XGE_EVENT_TEXT:
		case XGE_EVENT_IME_END:
			return XGE_XUI_EVENT_MASK_TEXT_INPUT;
		case XGE_EVENT_XUI_HOTKEY:
			return XGE_XUI_EVENT_MASK_HOTKEY;
		case XGE_EVENT_XUI_COMMAND:
			return XGE_XUI_EVENT_MASK_COMMAND;
		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			return XGE_XUI_EVENT_MASK_FOCUS;
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			return XGE_XUI_EVENT_MASK_CAPTURE;
		case XGE_EVENT_XUI_DRAG_BEGIN:
		case XGE_EVENT_XUI_DRAG_MOVE:
		case XGE_EVENT_XUI_DRAG_END:
		case XGE_EVENT_XUI_DRAG_CANCEL:
			return XGE_XUI_EVENT_MASK_DRAG;
		case XGE_EVENT_XUI_BOUNDS_CHANGED:
		case XGE_EVENT_XUI_VISIBLE_CHANGED:
		case XGE_EVENT_XUI_ENABLED_CHANGED:
			return XGE_XUI_EVENT_MASK_STATE;
		default:
			return 0;
	}
}

static uint32_t __xgeXuiWidgetComputeSubtreeEventMask(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	uint32_t iMask;

	if ( pWidget == NULL ) {
		return 0;
	}
	iMask = pWidget->iEventMask;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		iMask |= pChild->iSubtreeEventMask;
	}
	return iMask;
}

static void __xgeXuiWidgetRefreshEventMasksUp(xge_xui_widget pWidget)
{
	uint32_t iMask;

	while ( pWidget != NULL ) {
		iMask = __xgeXuiWidgetComputeSubtreeEventMask(pWidget);
		if ( pWidget->iSubtreeEventMask == iMask ) {
			break;
		}
		pWidget->iSubtreeEventMask = iMask;
		pWidget = pWidget->pParent;
	}
}

static void __xgeXuiWidgetSetEventInterest(xge_xui_widget pWidget, uint32_t iMask, int bEnabled)
{
	uint32_t iOldMask;

	if ( (pWidget == NULL) || (iMask == 0) ) {
		return;
	}
	iOldMask = pWidget->iEventMask;
	if ( bEnabled ) {
		pWidget->iEventMask |= iMask;
	} else {
		pWidget->iEventMask &= ~iMask;
	}
	if ( pWidget->iEventMask != iOldMask ) {
		__xgeXuiWidgetRefreshEventMasksUp(pWidget);
	}
}

static void __xgeXuiWidgetRefreshRawEventInterest(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return;
	}
	__xgeXuiWidgetSetEventInterest(pWidget, XGE_XUI_EVENT_MASK_RAW, (pWidget->procEvent != NULL) || (pWidget->procCaptureEvent != NULL));
}

static int __xgeXuiHotKeyWidgetHasRegistration(xge_xui_context pContext, xge_xui_widget pWidget)
{
	int i;

	if ( (pContext == NULL) || (pWidget == NULL) ) {
		return 0;
	}
	for ( i = 0; i < pContext->iHotKeyCount; i++ ) {
		if ( pContext->arrHotKey[i].pWidget == pWidget ) {
			return 1;
		}
	}
	return 0;
}

static void __xgeXuiHotKeyRefreshWidgetInterest(xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return;
	}
	if ( __xgeXuiHotKeyWidgetHasRegistration(pContext, pWidget) || (pWidget->arrEventProc[XGE_EVENT_XUI_HOTKEY] != NULL) ) {
		__xgeXuiWidgetSetEventInterest(pWidget, XGE_XUI_EVENT_MASK_HOTKEY, 1);
	} else {
		__xgeXuiWidgetSetEventInterest(pWidget, XGE_XUI_EVENT_MASK_HOTKEY, 0);
	}
}

static int __xgeXuiHotKeyWidgetHasCommandRegistration(xge_xui_context pContext, xge_xui_widget pWidget)
{
	int i;

	if ( (pContext == NULL) || (pWidget == NULL) ) {
		return 0;
	}
	for ( i = 0; i < pContext->iHotKeyCount; i++ ) {
		if ( (pContext->arrHotKey[i].pWidget == pWidget) && (pContext->arrHotKey[i].procEvent == NULL) ) {
			return 1;
		}
	}
	return 0;
}

static void __xgeXuiCommandRefreshWidgetInterest(xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return;
	}
	if ( __xgeXuiHotKeyWidgetHasCommandRegistration(pContext, pWidget) || (pWidget->arrEventProc[XGE_EVENT_XUI_COMMAND] != NULL) ) {
		__xgeXuiWidgetSetEventInterest(pWidget, XGE_XUI_EVENT_MASK_COMMAND, 1);
	} else {
		__xgeXuiWidgetSetEventInterest(pWidget, XGE_XUI_EVENT_MASK_COMMAND, 0);
	}
}

static int __xgeXuiWidgetCanCommand(xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pContext == NULL) || (__xgeXuiWidgetCanAction(pWidget) == 0) ) {
		return 0;
	}
	return __xgeXuiWidgetContainsWidget(pContext->pRoot, pWidget) || __xgeXuiWidgetContainsWidget(pContext->pOverlayRoot, pWidget);
}

static int __xgeXuiWidgetCanDrag(xge_xui_widget pWidget)
{
	return __xgeXuiWidgetCanAction(pWidget) && (((pWidget->iFlags & XGE_XUI_WIDGET_DRAG_ENABLED) != 0) || ((pWidget->iEventMask & XGE_XUI_EVENT_MASK_DRAG) != 0));
}

static int __xgeXuiWidgetHasDragHandler(xge_xui_widget pWidget)
{
	return (pWidget != NULL) && ((pWidget->arrEventProc[XGE_EVENT_XUI_DRAG_BEGIN] != NULL) || (pWidget->arrEventProc[XGE_EVENT_XUI_DRAG_MOVE] != NULL) || (pWidget->arrEventProc[XGE_EVENT_XUI_DRAG_END] != NULL) || (pWidget->arrEventProc[XGE_EVENT_XUI_DRAG_CANCEL] != NULL));
}

static int __xgeXuiEventIsKeyboardContextMenu(const xge_event_t* pEvent)
{
	int iModifiers;

	if ( (pEvent == NULL) || (pEvent->iType != XGE_EVENT_KEY_DOWN) ) {
		return 0;
	}
	iModifiers = __xgeXuiHotKeyNormalizeModifiers(pEvent->iParam2);
	if ( (pEvent->iParam1 == XGE_KEY_MENU) && (iModifiers == 0) ) {
		return 1;
	}
	return (pEvent->iParam1 == XGE_KEY_F10) && (iModifiers == XGE_KEY_MOD_SHIFT);
}

static xge_event_t __xgeXuiEventRoute(const xge_event_t* pEvent, int iPhase, xge_xui_widget pOriginalTarget, xge_xui_widget pCurrentTarget, xge_xui_widget pCapture)
{
	xge_event_t tEvent;

	if ( pEvent != NULL ) {
		tEvent = *pEvent;
	} else {
		memset(&tEvent, 0, sizeof(tEvent));
	}
	if ( pOriginalTarget == NULL ) {
		pOriginalTarget = pCurrentTarget;
	}
	if ( tEvent.fTime <= 0.0 ) {
		tEvent.fTime = xgeTimer();
	}
	tEvent.iXuiPhase = iPhase;
	tEvent.bXuiCaptured = (pCapture != NULL);
	tEvent.pXuiOriginalTarget = pOriginalTarget;
	tEvent.pXuiCurrentTarget = pCurrentTarget;
	tEvent.pXuiCapture = pCapture;
	return tEvent;
}

static int __xgeXuiEventResultNormalize(int iResult)
{
	if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( iResult == XGE_XUI_EVENT_HANDLED ) {
		return XGE_XUI_EVENT_HANDLED;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static int __xgeXuiEventResultMerge(int iCurrent, int iNext)
{
	iCurrent = __xgeXuiEventResultNormalize(iCurrent);
	iNext = __xgeXuiEventResultNormalize(iNext);
	if ( (iCurrent == XGE_XUI_EVENT_CONSUMED) || (iNext == XGE_XUI_EVENT_CONSUMED) ) {
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (iCurrent == XGE_XUI_EVENT_HANDLED) || (iNext == XGE_XUI_EVENT_HANDLED) ) {
		return XGE_XUI_EVENT_HANDLED;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static uint64_t __xgeXuiPointerIdNormalize(uint64_t iPointerId)
{
	return iPointerId;
}

static uint64_t __xgeXuiEventPointerId(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return __xgeXuiPointerIdNormalize(pEvent->iPointerId);
}

static int __xgeXuiPointerCaptureSlot(xge_xui_context pContext, uint64_t iPointerId)
{
	int i;

	if ( pContext == NULL ) {
		return -1;
	}
	iPointerId = __xgeXuiPointerIdNormalize(iPointerId);
	if ( iPointerId == 0 ) {
		return 0;
	}
	for ( i = 1; i < XGE_XUI_POINTER_CAPTURE_CAPACITY; i++ ) {
		if ( (pContext->arrPointerCaptureWidget[i] != NULL) && (pContext->arrPointerCaptureId[i] == iPointerId) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeXuiPointerCaptureEmptySlot(xge_xui_context pContext)
{
	int i;

	if ( pContext == NULL ) {
		return -1;
	}
	for ( i = 1; i < XGE_XUI_POINTER_CAPTURE_CAPACITY; i++ ) {
		if ( pContext->arrPointerCaptureWidget[i] == NULL ) {
			return i;
		}
	}
	return -1;
}

static void __xgeXuiPointerCaptureClearSlot(xge_xui_context pContext, int iSlot)
{
	if ( (pContext == NULL) || (iSlot < 0) || (iSlot >= XGE_XUI_POINTER_CAPTURE_CAPACITY) ) {
		return;
	}
	pContext->arrPointerCaptureId[iSlot] = 0;
	pContext->arrPointerCaptureWidget[iSlot] = NULL;
	if ( iSlot == 0 ) {
		pContext->pCapture = NULL;
	}
}

static int __xgeXuiReleasePointerCapture(xge_xui_context pContext, uint64_t iPointerId, int bNotifyLost)
{
	xge_xui_widget pOldCapture;
	int iSlot;

	if ( pContext == NULL ) {
		return 0;
	}
	iPointerId = __xgeXuiPointerIdNormalize(iPointerId);
	iSlot = __xgeXuiPointerCaptureSlot(pContext, iPointerId);
	if ( iPointerId == 0 ) {
		pOldCapture = pContext->pCapture;
		pContext->pCapture = NULL;
		pContext->arrPointerCaptureId[0] = 0;
		pContext->arrPointerCaptureWidget[0] = NULL;
	} else {
		if ( iSlot < 0 ) {
			return 0;
		}
		pOldCapture = pContext->arrPointerCaptureWidget[iSlot];
		__xgeXuiPointerCaptureClearSlot(pContext, iSlot);
	}
	if ( pOldCapture == NULL ) {
		return 0;
	}
	if ( bNotifyLost ) {
		__xgeXuiDispatchWidgetPointerEvent(pOldCapture, XGE_EVENT_XUI_CAPTURE_LOST, iPointerId);
		__xgeXuiDispatchWidgetPointerEvent(pOldCapture, XGE_EVENT_XUI_CAPTURE_CANCEL, iPointerId);
	}
	return 1;
}

static int __xgeXuiReleaseAllCaptures(xge_xui_context pContext, int bNotifyLost)
{
	xge_xui_widget pOldCapture;
	uint64_t arrPointerId[XGE_XUI_POINTER_CAPTURE_CAPACITY];
	xge_xui_widget arrCapture[XGE_XUI_POINTER_CAPTURE_CAPACITY];
	int iCount;
	int i;

	if ( pContext == NULL ) {
		return 0;
	}
	iCount = 0;
	if ( pContext->pCapture != NULL ) {
		arrPointerId[iCount] = 0;
		arrCapture[iCount] = pContext->pCapture;
		iCount++;
	}
	for ( i = 1; i < XGE_XUI_POINTER_CAPTURE_CAPACITY; i++ ) {
		if ( pContext->arrPointerCaptureWidget[i] != NULL ) {
			arrPointerId[iCount] = pContext->arrPointerCaptureId[i];
			arrCapture[iCount] = pContext->arrPointerCaptureWidget[i];
			iCount++;
		}
	}
	pContext->pCapture = NULL;
	for ( i = 0; i < XGE_XUI_POINTER_CAPTURE_CAPACITY; i++ ) {
		__xgeXuiPointerCaptureClearSlot(pContext, i);
	}
	if ( bNotifyLost ) {
		for ( i = 0; i < iCount; i++ ) {
			pOldCapture = arrCapture[i];
			if ( pOldCapture != NULL ) {
				__xgeXuiDispatchWidgetPointerEvent(pOldCapture, XGE_EVENT_XUI_CAPTURE_LOST, arrPointerId[i]);
				__xgeXuiDispatchWidgetPointerEvent(pOldCapture, XGE_EVENT_XUI_CAPTURE_CANCEL, arrPointerId[i]);
			}
		}
	}
	return iCount;
}

static int __xgeXuiReleaseWidgetCaptures(xge_xui_context pContext, xge_xui_widget pWidget, int bNotifyLost)
{
	uint64_t iPointerId;
	int iCount;
	int i;

	if ( (pContext == NULL) || (pWidget == NULL) ) {
		return 0;
	}
	iCount = 0;
	if ( pContext->pCapture == pWidget ) {
		iCount += __xgeXuiReleasePointerCapture(pContext, 0, bNotifyLost);
	}
	for ( i = 1; i < XGE_XUI_POINTER_CAPTURE_CAPACITY; i++ ) {
		if ( pContext->arrPointerCaptureWidget[i] == pWidget ) {
			iPointerId = pContext->arrPointerCaptureId[i];
			iCount += __xgeXuiReleasePointerCapture(pContext, iPointerId, bNotifyLost);
		}
	}
	return iCount;
}

static xge_xui_widget __xgeXuiHitTestWidget(xge_xui_widget pWidget, float fX, float fY)
{
	xge_xui_widget pChild;
	xge_xui_widget pHit;
	int bInsideBorder;
	int bClip;

	if ( __xgeXuiWidgetCanHitTest(pWidget) == 0 ) {
		return NULL;
	}
	bInsideBorder = __xgeXuiRectContains(pWidget->tBorderRect, fX, fY);
	bClip = ((pWidget->iFlags & XGE_XUI_WIDGET_CLIP) != 0);
	if ( (bInsideBorder == 0) && bClip ) {
		return NULL;
	}
	if ( bInsideBorder && bClip && (__xgeXuiRectContains(pWidget->tContentRect, fX, fY) == 0) ) {
		return ((pWidget->iFlags & XGE_XUI_WIDGET_INPUT_TRANSPARENT) == 0) ? pWidget : NULL;
	}
	for ( pChild = __xgeXuiChildNextHit(pWidget, NULL); pChild != NULL; pChild = __xgeXuiChildNextHit(pWidget, pChild) ) {
		pHit = __xgeXuiHitTestWidget(pChild, fX, fY);
		if ( pHit != NULL ) {
			return pHit;
		}
	}
	return (bInsideBorder && ((pWidget->iFlags & XGE_XUI_WIDGET_INPUT_TRANSPARENT) == 0)) ? pWidget : NULL;
}

static int __xgeXuiDispatchToWidget(xge_xui_widget pWidget, xge_xui_widget pOriginalTarget, xge_xui_widget pCapture, const xge_event_t* pEvent)
{
	int iResult;
	int iRouteResult;
	xge_xui_widget pTarget;
	xge_event_t tRouteEvent;
	xge_xui_event_proc procTyped;

	iRouteResult = XGE_XUI_EVENT_CONTINUE;
	pTarget = pWidget;
	while ( pWidget != NULL ) {
		if ( (pEvent != NULL) && (pEvent->iType > XGE_EVENT_NONE) && (pEvent->iType < XGE_XUI_WIDGET_EVENT_SLOT_COUNT) ) {
			procTyped = pWidget->arrEventProc[pEvent->iType];
			if ( procTyped != NULL ) {
				tRouteEvent = __xgeXuiEventRoute(pEvent, (pWidget == pTarget) ? XGE_XUI_EVENT_PHASE_TARGET : XGE_XUI_EVENT_PHASE_BUBBLE, pOriginalTarget, pWidget, pCapture);
				iResult = __xgeXuiEventResultNormalize(procTyped(pWidget, &tRouteEvent, pWidget->arrEventUser[pEvent->iType]));
				if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
					return XGE_XUI_EVENT_CONSUMED;
				}
				iRouteResult = __xgeXuiEventResultMerge(iRouteResult, iResult);
			}
		}
		if ( pWidget->procEvent != NULL ) {
			tRouteEvent = __xgeXuiEventRoute(pEvent, (pWidget == pTarget) ? XGE_XUI_EVENT_PHASE_TARGET : XGE_XUI_EVENT_PHASE_BUBBLE, pOriginalTarget, pWidget, pCapture);
			iResult = __xgeXuiEventResultNormalize(pWidget->procEvent(pWidget, &tRouteEvent, __xgeXuiWidgetEventUser(pWidget)));
			if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			iRouteResult = __xgeXuiEventResultMerge(iRouteResult, iResult);
		}
		pWidget = pWidget->pParent;
	}
	return iRouteResult;
}

static int __xgeXuiDispatchCaptureToWidget(xge_xui_widget pTarget, xge_xui_widget pOriginalTarget, xge_xui_widget pCapture, const xge_event_t* pEvent)
{
	xge_xui_widget pStack[64];
	xge_xui_widget pWidget;
	xge_event_t tRouteEvent;
	int iCount;
	int i;
	int iResult;
	int iRouteResult;
	xge_xui_event_proc procTyped;

	if ( (pTarget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iRouteResult = XGE_XUI_EVENT_CONTINUE;
	iCount = 0;
	for ( pWidget = pTarget; pWidget != NULL && iCount < 64; pWidget = pWidget->pParent ) {
		pStack[iCount] = pWidget;
		iCount++;
	}
	for ( i = iCount - 1; i >= 0; i-- ) {
		pWidget = pStack[i];
		if ( (pEvent->iType > XGE_EVENT_NONE) && (pEvent->iType < XGE_XUI_WIDGET_EVENT_SLOT_COUNT) ) {
			procTyped = pWidget->arrEventProc[pEvent->iType];
			if ( procTyped != NULL ) {
				tRouteEvent = __xgeXuiEventRoute(pEvent, XGE_XUI_EVENT_PHASE_TUNNEL, pOriginalTarget, pWidget, pCapture);
				iResult = __xgeXuiEventResultNormalize(procTyped(pWidget, &tRouteEvent, pWidget->arrEventUser[pEvent->iType]));
				if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
					return XGE_XUI_EVENT_CONSUMED;
				}
				iRouteResult = __xgeXuiEventResultMerge(iRouteResult, iResult);
			}
		}
		if ( pWidget->procCaptureEvent != NULL ) {
			tRouteEvent = __xgeXuiEventRoute(pEvent, XGE_XUI_EVENT_PHASE_TUNNEL, pOriginalTarget, pWidget, pCapture);
			iResult = __xgeXuiEventResultNormalize(pWidget->procCaptureEvent(pWidget, &tRouteEvent, (pWidget->pCaptureUser != NULL) ? pWidget->pCaptureUser : pWidget->pUser));
			if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			iRouteResult = __xgeXuiEventResultMerge(iRouteResult, iResult);
		}
	}
	return iRouteResult;
}

static int __xgeXuiDispatchOverlayPoint(xge_xui_context pContext, const xge_event_t* pEvent, xge_xui_widget pHit)
{
	xge_xui_widget pChild;
	int iResult;
	int iRouteResult;

	if ( (pContext == NULL) || (pContext->pOverlayRoot == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iRouteResult = XGE_XUI_EVENT_CONTINUE;
	for ( pChild = __xgeXuiChildNextHit(pContext->pOverlayRoot, NULL); pChild != NULL; pChild = __xgeXuiChildNextHit(pContext->pOverlayRoot, pChild) ) {
		xge_event_t tRouteEvent;
		xge_xui_event_proc procTyped;

		if ( (__xgeXuiWidgetCanHitTest(pChild) == 0) || ((pChild->iFlags & XGE_XUI_WIDGET_INPUT_TRANSPARENT) != 0) ) {
			continue;
		}
		if ( (pHit != NULL) && __xgeXuiWidgetContainsWidget(pChild, pHit) ) {
			continue;
		}
		if ( (pEvent->iType > XGE_EVENT_NONE) && (pEvent->iType < XGE_XUI_WIDGET_EVENT_SLOT_COUNT) ) {
			procTyped = pChild->arrEventProc[pEvent->iType];
			if ( procTyped != NULL ) {
				tRouteEvent = __xgeXuiEventRoute(pEvent, XGE_XUI_EVENT_PHASE_TARGET, pChild, pChild, NULL);
				iResult = __xgeXuiEventResultNormalize(procTyped(pChild, &tRouteEvent, pChild->arrEventUser[pEvent->iType]));
				if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
					return XGE_XUI_EVENT_CONSUMED;
				}
				iRouteResult = __xgeXuiEventResultMerge(iRouteResult, iResult);
			}
		}
		if ( pChild->procEvent != NULL ) {
			tRouteEvent = __xgeXuiEventRoute(pEvent, XGE_XUI_EVENT_PHASE_TARGET, pChild, pChild, NULL);
			iResult = __xgeXuiEventResultNormalize(pChild->procEvent(pChild, &tRouteEvent, __xgeXuiWidgetEventUser(pChild)));
			if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			iRouteResult = __xgeXuiEventResultMerge(iRouteResult, iResult);
		}
	}
	return iRouteResult;
}

static int __xgeXuiDispatchContextEvent(xge_xui_context pContext, xge_xui_widget pTarget, int iType, float fX, float fY, uint64_t iPointerId)
{
	xge_event_t tEvent;
	xge_xui_widget pCapture;
	int iResult;
	int iCaptureResult;

	if ( (pContext == NULL) || (pTarget == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = iType;
	tEvent.fX = fX;
	tEvent.fY = fY;
	tEvent.iPointerId = __xgeXuiPointerIdNormalize(iPointerId);
	pCapture = xgeXuiGetPointerCapture(pContext, tEvent.iPointerId);
	iCaptureResult = __xgeXuiDispatchCaptureToWidget(pTarget, pTarget, pCapture, &tEvent);
	if ( iCaptureResult == XGE_XUI_EVENT_CONSUMED ) {
		iResult = XGE_XUI_EVENT_CONSUMED;
	} else {
		iResult = __xgeXuiEventResultMerge(iCaptureResult, __xgeXuiDispatchToWidget(pTarget, pTarget, pCapture, &tEvent));
	}
	if ( (iType == XGE_EVENT_XUI_CONTEXT_BEGIN) && (iResult != XGE_XUI_EVENT_CONTINUE) && (pCapture != NULL) ) {
		__xgeXuiReleasePointerCapture(pContext, tEvent.iPointerId, 1);
	}
	return iResult;
}

static int __xgeXuiDispatchSyntheticEvent(xge_xui_context pContext, xge_xui_widget pTarget, xge_xui_widget pOriginalTarget, xge_xui_widget pCapture, const xge_event_t* pSource, int iType)
{
	xge_event_t tEvent;
	int iResult;
	int iCaptureResult;

	if ( (pContext == NULL) || (pTarget == NULL) || (pSource == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	tEvent = *pSource;
	tEvent.iType = iType;
	tEvent.iPointerId = __xgeXuiEventPointerId(pSource);
	iCaptureResult = __xgeXuiDispatchCaptureToWidget(pTarget, (pOriginalTarget != NULL) ? pOriginalTarget : pTarget, pCapture, &tEvent);
	if ( iCaptureResult == XGE_XUI_EVENT_CONSUMED ) {
		iResult = XGE_XUI_EVENT_CONSUMED;
	} else {
		iResult = __xgeXuiEventResultMerge(iCaptureResult, __xgeXuiDispatchToWidget(pTarget, (pOriginalTarget != NULL) ? pOriginalTarget : pTarget, pCapture, &tEvent));
	}
	return iResult;
}

static xge_xui_widget __xgeXuiDragCandidate(xge_xui_widget pHit)
{
	xge_xui_widget pIt;

	for ( pIt = pHit; pIt != NULL; pIt = pIt->pParent ) {
		if ( __xgeXuiWidgetCanDrag(pIt) ) {
			return pIt;
		}
	}
	return NULL;
}

static int __xgeXuiDispatchDragEvent(xge_xui_context pContext, const xge_event_t* pSource, int iType)
{
	xge_event_t tEvent;
	xge_xui_widget pCapture;
	int iResult;
	int iCaptureResult;

	if ( (pContext == NULL) || (pSource == NULL) || (pContext->pDragTarget == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	tEvent = *pSource;
	tEvent.iType = iType;
	tEvent.iPointerId = pContext->iDragPointerId;
	tEvent.fDX = pSource->fX - pContext->fDragStartX;
	tEvent.fDY = pSource->fY - pContext->fDragStartY;
	tEvent.pData = NULL;
	pCapture = xgeXuiGetPointerCapture(pContext, pContext->iDragPointerId);
	iCaptureResult = __xgeXuiDispatchCaptureToWidget(pContext->pDragTarget, pContext->pDragTarget, pCapture, &tEvent);
	if ( iCaptureResult == XGE_XUI_EVENT_CONSUMED ) {
		iResult = XGE_XUI_EVENT_CONSUMED;
	} else {
		iResult = __xgeXuiEventResultMerge(iCaptureResult, __xgeXuiDispatchToWidget(pContext->pDragTarget, pContext->pDragTarget, pCapture, &tEvent));
	}
	pContext->fDragLastX = pSource->fX;
	pContext->fDragLastY = pSource->fY;
	return iResult;
}

static int __xgeXuiClickReleaseMatches(xge_xui_context pContext, xge_xui_widget pHit, const xge_event_t* pEvent)
{
	float fDX;
	float fDY;

	if ( (pContext == NULL) || (pEvent == NULL) || (pContext->bClickPressActive == 0) || (pContext->pClickPressTarget == NULL) ) {
		return 0;
	}
	if ( (pEvent->iType != XGE_EVENT_MOUSE_UP) || (pEvent->iParam1 != pContext->iClickPressButton) ) {
		return 0;
	}
	if ( __xgeXuiEventPointerId(pEvent) != pContext->iClickPressPointerId ) {
		return 0;
	}
	if ( pContext->bClickPressMoved != 0 ) {
		return 0;
	}
	if ( (pHit != pContext->pClickPressTarget) && (__xgeXuiWidgetContainsWidget(pContext->pClickPressTarget, pHit) == 0) ) {
		return 0;
	}
	fDX = pEvent->fX - pContext->fClickPressX;
	fDY = pEvent->fY - pContext->fClickPressY;
	return (fDX * fDX + fDY * fDY) <= 36.0f;
}

static int __xgeXuiClickIsDouble(xge_xui_context pContext, const xge_event_t* pEvent)
{
	double fNow;
	float fDX;
	float fDY;

	if ( (pContext == NULL) || (pEvent == NULL) || (pContext->pLastClickTarget == NULL) || (pContext->pLastClickTarget != pContext->pClickPressTarget) ) {
		return 0;
	}
	if ( (pContext->iLastClickButton != pContext->iClickPressButton) || (pContext->iLastClickPointerId != pContext->iClickPressPointerId) ) {
		return 0;
	}
	fNow = xgeTimer();
	if ( (fNow - pContext->fLastClickTime) > 0.50 ) {
		return 0;
	}
	fDX = pEvent->fX - pContext->fLastClickX;
	fDY = pEvent->fY - pContext->fLastClickY;
	return (fDX * fDX + fDY * fDY) <= 36.0f;
}

static void __xgeXuiDispatchStateEvent(xge_xui_widget pWidget, int iType, int iNewValue, int iOldValue, xge_rect_t tNewRect, xge_rect_t tOldRect)
{
	xge_event_t tEvent;
	xge_event_t tRouteEvent;
	xge_xui_event_proc procTyped;

	if ( (pWidget == NULL) || (iType <= XGE_EVENT_NONE) || (iType >= XGE_XUI_WIDGET_EVENT_SLOT_COUNT) ) {
		return;
	}
	if ( (pWidget->iEventMask & __xgeXuiEventMaskFromType(iType)) == 0 ) {
		return;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = iType;
	tEvent.iParam1 = iNewValue;
	tEvent.iParam2 = iOldValue;
	tEvent.fX = tNewRect.fX;
	tEvent.fY = tNewRect.fY;
	tEvent.fDX = tNewRect.fW - tOldRect.fW;
	tEvent.fDY = tNewRect.fH - tOldRect.fH;
	procTyped = pWidget->arrEventProc[iType];
	if ( procTyped != NULL ) {
		tRouteEvent = __xgeXuiEventRoute(&tEvent, XGE_XUI_EVENT_PHASE_TARGET, pWidget, pWidget, NULL);
		procTyped(pWidget, &tRouteEvent, pWidget->arrEventUser[iType]);
	}
	if ( pWidget->procEvent != NULL ) {
		tRouteEvent = __xgeXuiEventRoute(&tEvent, XGE_XUI_EVENT_PHASE_TARGET, pWidget, pWidget, NULL);
		pWidget->procEvent(pWidget, &tRouteEvent, __xgeXuiWidgetEventUser(pWidget));
	}
}

static void __xgeXuiDispatchFocusEvent(xge_xui_widget pWidget, int iType)
{
	xge_event_t tEvent;
	xge_event_t tRouteEvent;
	xge_xui_event_proc procTyped;

	if ( pWidget == NULL ) {
		return;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = iType;
	if ( (iType > XGE_EVENT_NONE) && (iType < XGE_XUI_WIDGET_EVENT_SLOT_COUNT) ) {
		procTyped = pWidget->arrEventProc[iType];
		if ( procTyped != NULL ) {
			tRouteEvent = __xgeXuiEventRoute(&tEvent, XGE_XUI_EVENT_PHASE_TARGET, pWidget, pWidget, NULL);
			procTyped(pWidget, &tRouteEvent, pWidget->arrEventUser[iType]);
		}
	}
	if ( pWidget->procEvent != NULL ) {
		tRouteEvent = __xgeXuiEventRoute(&tEvent, XGE_XUI_EVENT_PHASE_TARGET, pWidget, pWidget, NULL);
		pWidget->procEvent(pWidget, &tRouteEvent, __xgeXuiWidgetEventUser(pWidget));
	}
}

static void __xgeXuiDispatchWidgetPointerEvent(xge_xui_widget pWidget, int iType, uint64_t iPointerId)
{
	xge_event_t tEvent;
	xge_event_t tRouteEvent;
	xge_xui_widget pCapture;
	xge_xui_event_proc procTyped;

	if ( pWidget == NULL ) {
		return;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = iType;
	tEvent.iPointerId = __xgeXuiPointerIdNormalize(iPointerId);
	pCapture = ((iType == XGE_EVENT_XUI_CAPTURE_LOST) || (iType == XGE_EVENT_XUI_CAPTURE_CANCEL)) ? pWidget : NULL;
	if ( (iType > XGE_EVENT_NONE) && (iType < XGE_XUI_WIDGET_EVENT_SLOT_COUNT) ) {
		procTyped = pWidget->arrEventProc[iType];
		if ( procTyped != NULL ) {
			tRouteEvent = __xgeXuiEventRoute(&tEvent, XGE_XUI_EVENT_PHASE_TARGET, pWidget, pWidget, pCapture);
			procTyped(pWidget, &tRouteEvent, pWidget->arrEventUser[iType]);
		}
	}
	if ( pWidget->procCaptureEvent != NULL ) {
		tRouteEvent = __xgeXuiEventRoute(&tEvent, XGE_XUI_EVENT_PHASE_TARGET, pWidget, pWidget, pCapture);
		pWidget->procCaptureEvent(pWidget, &tRouteEvent, (pWidget->pCaptureUser != NULL) ? pWidget->pCaptureUser : pWidget->pUser);
	}
	if ( pWidget->procEvent != NULL ) {
		tRouteEvent = __xgeXuiEventRoute(&tEvent, XGE_XUI_EVENT_PHASE_TARGET, pWidget, pWidget, pCapture);
		pWidget->procEvent(pWidget, &tRouteEvent, __xgeXuiWidgetEventUser(pWidget));
	}
}

static void __xgeXuiDispatchWidgetEvent(xge_xui_widget pWidget, int iType)
{
	__xgeXuiDispatchWidgetPointerEvent(pWidget, iType, 0);
}

static int __xgeXuiWidgetBuildPath(xge_xui_widget pWidget, xge_xui_widget* pPath, int iCapacity)
{
	xge_xui_widget pStack[64];
	int iCount;
	int i;

	if ( (pPath == NULL) || (iCapacity <= 0) ) {
		return 0;
	}
	iCount = 0;
	for ( ; pWidget != NULL && iCount < 64; pWidget = pWidget->pParent ) {
		pStack[iCount++] = pWidget;
	}
	if ( iCount > iCapacity ) {
		iCount = iCapacity;
	}
	for ( i = 0; i < iCount; i++ ) {
		pPath[i] = pStack[iCount - 1 - i];
	}
	return iCount;
}

static void __xgeXuiHoverPathUpdate(xge_xui_context pContext, xge_xui_widget pHit, uint64_t iPointerId)
{
	xge_xui_widget arrNewPath[64];
	int iNewCount;
	int iCommon;
	int i;

	if ( pContext == NULL ) {
		return;
	}
	iNewCount = __xgeXuiWidgetBuildPath(pHit, arrNewPath, 64);
	iCommon = 0;
	while ( (iCommon < pContext->iHoverPathCount) && (iCommon < iNewCount) && (pContext->arrHoverPath[iCommon] == arrNewPath[iCommon]) ) {
		iCommon++;
	}
	for ( i = pContext->iHoverPathCount - 1; i >= iCommon; i-- ) {
		__xgeXuiDispatchWidgetPointerEvent(pContext->arrHoverPath[i], XGE_EVENT_XUI_POINTER_LEAVE, iPointerId);
	}
	for ( i = iCommon; i < iNewCount; i++ ) {
		__xgeXuiDispatchWidgetPointerEvent(arrNewPath[i], XGE_EVENT_XUI_POINTER_ENTER, iPointerId);
	}
	memset(pContext->arrHoverPath, 0, sizeof(pContext->arrHoverPath));
	for ( i = 0; i < iNewCount; i++ ) {
		pContext->arrHoverPath[i] = arrNewPath[i];
	}
	pContext->iHoverPathCount = iNewCount;
	pContext->pHover = pHit;
}

static int __xgeXuiWidgetTabBefore(xge_xui_widget pA, xge_xui_widget pB)
{
	if ( pB == NULL ) {
		return 1;
	}
	if ( pA == NULL ) {
		return 0;
	}
	if ( pA->iTabIndex != pB->iTabIndex ) {
		return pA->iTabIndex < pB->iTabIndex;
	}
	if ( pA->iTreeOrder != pB->iTreeOrder ) {
		return pA->iTreeOrder < pB->iTreeOrder;
	}
	return __xgeXuiChildIsAfter(pB, pA);
}

static int __xgeXuiFocusCandidateInRange(xge_xui_widget pCandidate, xge_xui_widget pCurrent, int bBackward)
{
	if ( (pCurrent == NULL) || (__xgeXuiWidgetCanTabFocus(pCurrent) == 0) ) {
		return 1;
	}
	if ( bBackward ) {
		return __xgeXuiWidgetTabBefore(pCandidate, pCurrent);
	}
	return __xgeXuiWidgetTabBefore(pCurrent, pCandidate);
}

static int __xgeXuiFocusCandidateBetter(xge_xui_widget pCandidate, xge_xui_widget pBest, int bBackward)
{
	if ( pBest == NULL ) {
		return 1;
	}
	if ( bBackward ) {
		return __xgeXuiWidgetTabBefore(pBest, pCandidate);
	}
	return __xgeXuiWidgetTabBefore(pCandidate, pBest);
}

static void __xgeXuiFocusFindBest(xge_xui_widget pWidget, xge_xui_widget pCurrent, int bBackward, int bWrap, xge_xui_widget* ppBest)
{
	xge_xui_widget pChild;

	if ( (pWidget == NULL) || (ppBest == NULL) ) {
		return;
	}
	if ( __xgeXuiWidgetCanTabFocus(pWidget) && (bWrap || __xgeXuiFocusCandidateInRange(pWidget, pCurrent, bBackward)) && __xgeXuiFocusCandidateBetter(pWidget, *ppBest, bBackward) ) {
		*ppBest = pWidget;
	}
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xgeXuiFocusFindBest(pChild, pCurrent, bBackward, bWrap, ppBest);
	}
}

static xge_xui_widget __xgeXuiOverlayTopLayer(xge_xui_context pContext, int iLayer)
{
	xge_xui_widget pChild;
	xge_xui_widget pTop;

	if ( (pContext == NULL) || (pContext->pOverlayRoot == NULL) ) {
		return NULL;
	}
	iLayer = __xgeXuiLayerClamp(iLayer);
	pTop = NULL;
	for ( pChild = pContext->pOverlayRoot->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( ((pChild->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || (__xgeXuiLayerClamp(pChild->tStyle.iLayer) != iLayer) ) {
			continue;
		}
		if ( (pTop == NULL) || __xgeXuiChildHitBefore(pChild, pTop) ) {
			pTop = pChild;
		}
	}
	return pTop;
}

static xge_xui_widget __xgeXuiOverlayTopContainingFocus(xge_xui_context pContext)
{
	xge_xui_widget pChild;
	xge_xui_widget pTop;

	if ( (pContext == NULL) || (pContext->pOverlayRoot == NULL) || (pContext->pFocus == NULL) ) {
		return NULL;
	}
	pTop = NULL;
	for ( pChild = pContext->pOverlayRoot->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( ((pChild->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || (__xgeXuiWidgetContainsWidget(pChild, pContext->pFocus) == 0) ) {
			continue;
		}
		if ( (pTop == NULL) || __xgeXuiChildHitBefore(pChild, pTop) ) {
			pTop = pChild;
		}
	}
	return pTop;
}

static xge_xui_widget __xgeXuiFocusScope(xge_xui_context pContext)
{
	xge_xui_widget pModal;
	xge_xui_widget pOverlay;
	xge_xui_widget pNamed;

	if ( (pContext == NULL) || (pContext->pRoot == NULL) ) {
		return NULL;
	}
	pModal = __xgeXuiOverlayTopLayer(pContext, XGE_XUI_LAYER_MODAL);
	if ( pModal != NULL ) {
		return pModal;
	}
	pOverlay = __xgeXuiOverlayTopContainingFocus(pContext);
	if ( pOverlay != NULL ) {
		return pOverlay;
	}
	for ( pNamed = pContext->pFocus; pNamed != NULL; pNamed = pNamed->pParent ) {
		if ( __xgeXuiWidgetCanFocusScope(pNamed) ) {
			return pNamed;
		}
		if ( pNamed == pContext->pRoot ) {
			break;
		}
	}
	return pContext->pRoot;
}

static int __xgeXuiWidgetHasAction(xge_xui_widget pWidget, int bCancel)
{
	if ( __xgeXuiWidgetCanAction(pWidget) == 0 ) {
		return 0;
	}
	return bCancel ? (pWidget->procCancelAction != NULL) : (pWidget->procDefaultAction != NULL);
}

static int __xgeXuiFocusActionCandidateBetter(xge_xui_widget pCandidate, xge_xui_widget pBest)
{
	return __xgeXuiWidgetTabBefore(pCandidate, pBest);
}

static void __xgeXuiFocusFindAction(xge_xui_widget pWidget, int bCancel, xge_xui_widget* ppBest)
{
	xge_xui_widget pChild;

	if ( (pWidget == NULL) || (ppBest == NULL) ) {
		return;
	}
	if ( __xgeXuiWidgetHasAction(pWidget, bCancel) && __xgeXuiFocusActionCandidateBetter(pWidget, *ppBest) ) {
		*ppBest = pWidget;
	}
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xgeXuiFocusFindAction(pChild, bCancel, ppBest);
	}
}

static int __xgeXuiFocusInvokeAction(xge_xui_context pContext, int bCancel)
{
	xge_xui_widget pScope;
	xge_xui_widget pAction;
	xge_xui_click_proc procAction;
	void* pUser;

	pScope = __xgeXuiFocusScope(pContext);
	if ( pScope == NULL ) {
		return 0;
	}
	pAction = NULL;
	__xgeXuiFocusFindAction(pScope, bCancel, &pAction);
	if ( pAction == NULL ) {
		return 0;
	}
	procAction = bCancel ? pAction->procCancelAction : pAction->procDefaultAction;
	pUser = bCancel ? pAction->pCancelActionUser : pAction->pDefaultActionUser;
	if ( procAction == NULL ) {
		return 0;
	}
	procAction(pAction, pUser);
	return 1;
}

static int __xgeXuiFocusInvokeKeyAction(xge_xui_context pContext, const xge_event_t* pEvent)
{
	if ( (pContext == NULL) || (pEvent == NULL) || (pEvent->iType != XGE_EVENT_KEY_DOWN) ) {
		return 0;
	}
	if ( pEvent->iParam1 == XGE_KEY_ENTER ) {
		return __xgeXuiFocusInvokeAction(pContext, 0);
	}
	if ( pEvent->iParam1 == XGE_KEY_ESCAPE ) {
		return __xgeXuiFocusInvokeAction(pContext, 1);
	}
	return 0;
}

static xge_xui_widget __xgeXuiFocusEventTarget(xge_xui_context pContext)
{
	xge_xui_widget pScope;
	xge_xui_widget pTarget;

	if ( pContext == NULL ) {
		return NULL;
	}
	pScope = __xgeXuiFocusScope(pContext);
	pTarget = pContext->pFocus;
	if ( (pScope != NULL) && (pScope != pContext->pRoot) && (__xgeXuiWidgetContainsWidget(pScope, pTarget) == 0) ) {
		pTarget = pScope;
	}
	if ( pTarget != NULL ) {
		return pTarget;
	}
	return (pScope != NULL) ? pScope : pContext->pRoot;
}

static int __xgeXuiDispatchKeyboardContextMenu(xge_xui_context pContext, const xge_event_t* pEvent)
{
	xge_event_t tEvent;
	xge_xui_widget pTarget;
	xge_rect_t tRect;

	if ( (pContext == NULL) || (pEvent == NULL) || (__xgeXuiEventIsKeyboardContextMenu(pEvent) == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	pTarget = __xgeXuiFocusEventTarget(pContext);
	if ( __xgeXuiWidgetCanAction(pTarget) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	tRect = pTarget->tContentRect;
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		tRect = pTarget->tRect;
	}
	tEvent = *pEvent;
	tEvent.fX = tRect.fX + (tRect.fW * 0.5f);
	tEvent.fY = tRect.fY + (tRect.fH * 0.5f);
	tEvent.fDX = 0.0f;
	tEvent.fDY = 0.0f;
	tEvent.iPointerId = 0;
	return __xgeXuiDispatchSyntheticEvent(pContext, pTarget, pTarget, NULL, &tEvent, XGE_EVENT_XUI_CONTEXT_MENU);
}

static int __xgeXuiHotKeyDispatch(xge_xui_context pContext, const xge_event_t* pEvent)
{
	xge_xui_widget pScope;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	xge_xui_hotkey pHotKey;
	int iModifiers;
	int iResult;
	int iMergedResult;
	int i;

	if ( (pContext == NULL) || (pEvent == NULL) || (pEvent->iType != XGE_EVENT_KEY_DOWN) || (pContext->iHotKeyCount <= 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	pScope = __xgeXuiFocusScope(pContext);
	if ( pScope == NULL ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iModifiers = __xgeXuiHotKeyNormalizeModifiers(pEvent->iParam2);
	iMergedResult = XGE_XUI_EVENT_CONTINUE;
	for ( i = pContext->iHotKeyCount - 1; i >= 0; i-- ) {
		pHotKey = &pContext->arrHotKey[i];
		pWidget = pHotKey->pWidget;
		if ( (pHotKey->iKey != pEvent->iParam1) || (pHotKey->iModifiers != iModifiers) ) {
			continue;
		}
		if ( (__xgeXuiWidgetCanCommand(pContext, pWidget) == 0) || (__xgeXuiWidgetContainsWidget(pScope, pWidget) == 0) ) {
			continue;
		}
		if ( pHotKey->procEvent != NULL ) {
			tEvent = __xgeXuiEventRoute(pEvent, XGE_XUI_EVENT_PHASE_TARGET, pWidget, pWidget, NULL);
			tEvent.iType = XGE_EVENT_XUI_HOTKEY;
			tEvent.iParam1 = pHotKey->iKey;
			tEvent.iParam2 = pHotKey->iModifiers;
			iResult = __xgeXuiEventResultNormalize(pHotKey->procEvent(pWidget, &tEvent, pHotKey->pUser));
		} else {
			iResult = xgeXuiCommandDispatch(pContext, pWidget, pWidget, pHotKey->iCommand, pHotKey->sCommand, pHotKey->pCommandData);
		}
		iMergedResult = __xgeXuiEventResultMerge(iMergedResult, iResult);
		if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	return iMergedResult;
}

static int __xgeXuiFocusStep(xge_xui_context pContext, int bBackward)
{
	xge_xui_widget pScope;
	xge_xui_widget pCurrent;
	xge_xui_widget pTarget;

	if ( (pContext == NULL) || (pContext->pRoot == NULL) ) {
		return 0;
	}
	pScope = __xgeXuiFocusScope(pContext);
	if ( pScope == NULL ) {
		return 0;
	}
	pCurrent = __xgeXuiWidgetContainsWidget(pScope, pContext->pFocus) ? pContext->pFocus : NULL;
	pTarget = NULL;
	__xgeXuiFocusFindBest(pScope, pCurrent, bBackward, 0, &pTarget);
	if ( pTarget == NULL ) {
		__xgeXuiFocusFindBest(pScope, NULL, bBackward, 1, &pTarget);
	}
	if ( pTarget == NULL ) {
		return 0;
	}
	xgeXuiSetFocus(pContext, pTarget);
	return 1;
}

static void __xgeXuiWidgetSetFlag(xge_xui_widget pWidget, uint32_t iFlag, int bEnabled)
{
	xge_xui_context pContext;
	uint32_t iOldFlags;
	int iStateEvent;
	int bOldEnabled;
	int bNewEnabled;

	if ( pWidget == NULL ) {
		return;
	}
	iOldFlags = pWidget->iFlags;
	if ( (bEnabled == 0) && ((iFlag == XGE_XUI_WIDGET_VISIBLE) || (iFlag == XGE_XUI_WIDGET_ENABLED)) && ((iOldFlags & iFlag) != 0) ) {
		pContext = __xgeXuiWidgetContext(pWidget);
		__xgeXuiWidgetDetachContextRefs(pContext, pWidget);
	}
	if ( bEnabled ) {
		pWidget->iFlags |= iFlag;
	} else {
		pWidget->iFlags &= ~iFlag;
	}
	if ( iOldFlags != pWidget->iFlags ) {
		xgeXuiWidgetMarkLayout(pWidget);
		xgeXuiWidgetMarkPaint(pWidget);
		iStateEvent = 0;
		if ( iFlag == XGE_XUI_WIDGET_VISIBLE ) {
			iStateEvent = XGE_EVENT_XUI_VISIBLE_CHANGED;
		} else if ( iFlag == XGE_XUI_WIDGET_ENABLED ) {
			iStateEvent = XGE_EVENT_XUI_ENABLED_CHANGED;
		}
		if ( iStateEvent != 0 ) {
			bOldEnabled = ((iOldFlags & iFlag) != 0) ? 1 : 0;
			bNewEnabled = ((pWidget->iFlags & iFlag) != 0) ? 1 : 0;
			__xgeXuiDispatchStateEvent(pWidget, iStateEvent, bNewEnabled, bOldEnabled, pWidget->tRect, pWidget->tRect);
		}
	}
}

static void __xgeXuiWidgetSetInputFlag(xge_xui_widget pWidget, uint32_t iFlag, int bEnabled)
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
		xgeXuiWidgetMarkPaint(pWidget);
	}
}

static void __xgeXuiWidgetMarkLayoutTree(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;

	if ( pWidget == NULL ) {
		return;
	}
	pWidget->iFlags |= XGE_XUI_WIDGET_DIRTY_LAYOUT;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xgeXuiWidgetMarkLayoutTree(pChild);
	}
}

static void __xgeXuiButtonSetState(xge_xui_button pButton, int iState)
{
	int iVisualState;

	if ( pButton == NULL ) {
		return;
	}
	if ( (pButton->pWidget == NULL) || ((pButton->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pButton->pContext != NULL && pButton->pContext->pFocus == pButton->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pButton->bSelected != 0 ) {
		iState |= XGE_XUI_STATE_CHECKED;
	}
	if ( pButton->iState != iState ) {
		pButton->iState = iState;
		xgeXuiWidgetMarkPaint(pButton->pWidget);
	}
	iVisualState = iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE | XGE_XUI_STATE_FOCUS | XGE_XUI_STATE_DISABLED | XGE_XUI_STATE_CHECKED);
	xgeXuiWidgetSetVisualState(pButton->pWidget, iVisualState);
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
	if ( pButton->bSelected != 0 ) {
		return pButton->iColorChecked;
	}
	return pButton->iColorNormal;
}

static void __xgeXuiCheckBoxSetState(xge_xui_checkbox pCheckBox, int iState)
{
	int iVisualState;

	if ( pCheckBox == NULL ) {
		return;
	}
	if ( (pCheckBox->pWidget == NULL) || ((pCheckBox->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pCheckBox->pContext != NULL && pCheckBox->pContext->pFocus == pCheckBox->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( xgeXuiCheckBoxGetChecked(pCheckBox) ) {
		iState |= XGE_XUI_STATE_CHECKED;
	}
	if ( pCheckBox->iState != iState ) {
		pCheckBox->iState = iState;
		xgeXuiWidgetMarkPaint(pCheckBox->pWidget);
	}
	if ( pCheckBox->pWidget != NULL ) {
		iVisualState = iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE | XGE_XUI_STATE_FOCUS | XGE_XUI_STATE_DISABLED | XGE_XUI_STATE_CHECKED);
		xgeXuiWidgetSetVisualState(pCheckBox->pWidget, iVisualState);
	}
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

	if ( (pList == NULL) || (pList->tBase.pWidget == NULL) ) {
		return 0.0f;
	}
	fMax = (float)pList->tBase.iItemCount * pList->tBase.fItemHeight - pList->tBase.pWidget->tContentRect.fH;
	return (fMax > 0.0f) ? fMax : 0.0f;
}

static void __xgeXuiListViewClamp(xge_xui_list_view pList)
{
	if ( pList == NULL ) {
		return;
	}
	pList->tBase.fScrollY = __xgeXuiClampFloat(pList->tBase.fScrollY, 0.0f, __xgeXuiListViewMaxScroll(pList));
}

static int __xgeXuiListViewIndexAt(xge_xui_list_view pList, float fY)
{
	int iIndex;

	if ( (pList == NULL) || (pList->tBase.pWidget == NULL) || (pList->tBase.fItemHeight <= 0.0f) ) {
		return -1;
	}
	iIndex = (int)((fY - pList->tBase.pWidget->tContentRect.fY + pList->tBase.fScrollY) / pList->tBase.fItemHeight);
	if ( (iIndex < 0) || (iIndex >= pList->tBase.iItemCount) ) {
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
	pTheme->iTextColor = XGE_COLOR_RGBA(24, 56, 79, 255);
	pTheme->iBackgroundColor = XGE_COLOR_RGBA(238, 248, 255, 255);
	pTheme->iPanelColor = XGE_COLOR_RGBA(249, 253, 255, 255);
	pTheme->iBorderColor = XGE_COLOR_RGBA(127, 196, 229, 255);
	pTheme->iAccentColor = XGE_COLOR_RGBA(53, 174, 234, 255);
	pTheme->iSelectionColor = XGE_COLOR_RGBA(223, 243, 255, 255);
	pTheme->iStateNormal = XGE_COLOR_RGBA(255, 255, 255, 255);
	pTheme->iStateHover = XGE_COLOR_RGBA(223, 243, 255, 255);
	pTheme->iStateActive = XGE_COLOR_RGBA(190, 231, 252, 255);
	pTheme->iStateFocus = XGE_COLOR_RGBA(53, 174, 234, 80);
	pTheme->iStateDisabled = XGE_COLOR_RGBA(237, 245, 250, 210);
	pTheme->fRadius = 5.0f;
	pTheme->fPadding = 6.0f;
	pTheme->fSpacing = 6.0f;
	pTheme->fBorderWidth = 1.5f;
}

static int __xgeXuiLoadDefaultUIFont(xge_font pFont)
{
	static const char* const arrFontPaths[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/SimSun.ttc",
		"C:/Windows/Fonts/simsun.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/simhei.ttf",
		"C:/Windows/Fonts/arial.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
		"/System/Library/Fonts/PingFang.ttc"
	};
	int i;

	if ( pFont == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < (int)(sizeof(arrFontPaths) / sizeof(arrFontPaths[0])); i++ ) {
		if ( xgeFontLoad(pFont, arrFontPaths[i], 12.0f) == XGE_OK ) {
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void __xgeXuiUseContextDefaultFont(xge_xui_context pContext)
{
	if ( (pContext != NULL) && (pContext->bDefaultFontReady != 0) && (pContext->tTheme.pFont == NULL) ) {
		pContext->tTheme.pFont = &pContext->tDefaultFont;
	}
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
		__xgeXuiUseContextDefaultFont(pContext);
	}
	pContext->iThemeVersion++;
	if ( pContext->iThemeVersion == 0 ) {
		pContext->iThemeVersion = 1;
	}
	xgeXuiWidgetMarkLayout(pContext->pRoot);
	xgeXuiWidgetMarkPaint(pContext->pRoot);
}

uint32_t xgeXuiGetThemeVersion(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return 0;
	}
	return pContext->iThemeVersion;
}

static void __xgeXuiThemeVersionBump(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	pContext->iThemeVersion++;
	if ( pContext->iThemeVersion == 0 ) {
		pContext->iThemeVersion = 1;
	}
}

static xvalue __xgeXuiTokenSection(xge_xui_context pContext, const char* sSection)
{
	xvalue pTokens;
	xvalue pSection;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (sSection == NULL) ) {
		return NULL;
	}
	if ( pContext->pRegisteredTokens == NULL ) {
		pContext->pRegisteredTokens = xvoCreateTable();
		if ( pContext->pRegisteredTokens == NULL ) {
			return NULL;
		}
	}
	pTokens = (xvalue)pContext->pRegisteredTokens;
	pSection = xvoTableGetValue(pTokens, sSection, (uint32)strlen(sSection));
	if ( (pSection != NULL) && (xvoType(pSection) == XVO_DT_TABLE) ) {
		return pSection;
	}
	if ( !xvoTableSetTable(pTokens, sSection, (uint32)strlen(sSection)) ) {
		return NULL;
	}
	pSection = xvoTableGetValue(pTokens, sSection, (uint32)strlen(sSection));
	if ( (pSection == NULL) || (xvoType(pSection) != XVO_DT_TABLE) ) {
		return NULL;
	}
	return pSection;
}

static void __xgeXuiTokenChanged(xge_xui_context pContext)
{
	__xgeXuiThemeVersionBump(pContext);
	if ( (pContext != NULL) && (pContext->pRoot != NULL) ) {
		xgeXuiWidgetMarkStyle(pContext->pRoot);
	}
}

int xgeXuiTokenSetColor(xge_xui_context pContext, const char* sName, uint32_t iColor)
{
	xvalue pSection;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (sName == NULL) || (sName[0] == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSection = __xgeXuiTokenSection(pContext, "colors");
	if ( pSection == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( !xvoTableSetInt(pSection, sName, (uint32)strlen(sName), (int64)iColor) ) {
		return XGE_ERROR;
	}
	__xgeXuiTokenChanged(pContext);
	return XGE_OK;
}

int xgeXuiTokenSetSpacing(xge_xui_context pContext, const char* sName, float fValue)
{
	xvalue pSection;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (sName == NULL) || (sName[0] == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSection = __xgeXuiTokenSection(pContext, "spacing");
	if ( pSection == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( !xvoTableSetFloat(pSection, sName, (uint32)strlen(sName), (double)fValue) ) {
		return XGE_ERROR;
	}
	__xgeXuiTokenChanged(pContext);
	return XGE_OK;
}

int xgeXuiTokenSetFont(xge_xui_context pContext, const char* sName, xge_font pFont)
{
	xvalue pSection;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (sName == NULL) || (sName[0] == 0) || (pFont == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSection = __xgeXuiTokenSection(pContext, "fonts");
	if ( pSection == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( !xvoTableSetPoint(pSection, sName, (uint32)strlen(sName), pFont) ) {
		return XGE_ERROR;
	}
	__xgeXuiTokenChanged(pContext);
	return XGE_OK;
}

int xgeXuiTokenSetTexture(xge_xui_context pContext, const char* sName, xge_texture pTexture)
{
	xvalue pSection;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (sName == NULL) || (sName[0] == 0) || (pTexture == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSection = __xgeXuiTokenSection(pContext, "textures");
	if ( pSection == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( !xvoTableSetPoint(pSection, sName, (uint32)strlen(sName), pTexture) ) {
		return XGE_ERROR;
	}
	__xgeXuiTokenChanged(pContext);
	return XGE_OK;
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
	pContext->pOverlayRoot = __xgeXuiWidgetAlloc();
	pContext->pTooltipPopupWidget = __xgeXuiWidgetAlloc();
	if ( (pContext->pRoot == NULL) || (pContext->pOverlayRoot == NULL) || (pContext->pTooltipPopupWidget == NULL) ) {
		__xgeXuiWidgetFreeTree(pContext->pRoot);
		__xgeXuiWidgetFreeTree(pContext->pOverlayRoot);
		__xgeXuiWidgetFreeTree(pContext->pTooltipPopupWidget);
		memset(pContext, 0, sizeof(*pContext));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pContext->fDipScale = 1.0f;
	pContext->iNextTreeOrder = 1;
	xgeXuiThemeDefault(&pContext->tTheme);
	if ( __xgeXuiLoadDefaultUIFont(&pContext->tDefaultFont) == XGE_OK ) {
		pContext->bDefaultFontReady = 1;
		__xgeXuiUseContextDefaultFont(pContext);
	}
	pContext->iThemeVersion = 1;
	pContext->pRoot->tRect.fW = (float)xgeGetWidth();
	pContext->pRoot->tRect.fH = (float)xgeGetHeight();
	pContext->pRoot->tLocalRect = pContext->pRoot->tRect;
	__xgeXuiWidgetBoxUpdate(pContext->pRoot, pContext->pRoot->tRect);
	pContext->pRoot->pInternal = pContext;
	pContext->pRoot->iRole = XGE_XUI_WIDGET_ROLE_CONTAINER;
	pContext->pRoot->iTreeOrder = pContext->iNextTreeOrder++;
	pContext->pOverlayRoot->tRect = pContext->pRoot->tRect;
	pContext->pOverlayRoot->tLocalRect = pContext->pOverlayRoot->tRect;
	__xgeXuiWidgetBoxUpdate(pContext->pOverlayRoot, pContext->pOverlayRoot->tRect);
	pContext->pOverlayRoot->pInternal = pContext;
	pContext->pOverlayRoot->iRole = XGE_XUI_WIDGET_ROLE_OVERLAY;
	pContext->pOverlayRoot->iTreeOrder = pContext->iNextTreeOrder++;
	pContext->pOverlayRoot->tStyle.iLayer = XGE_XUI_LAYER_POPUP;
	pContext->pOverlayRoot->tStyle.iLayout = XGE_XUI_LAYOUT_ABSOLUTE;
	xgeXuiWidgetSetBackground(pContext->pOverlayRoot, XGE_COLOR_RGBA(0, 0, 0, 0));
	pContext->pTooltipPopupWidget->procPaint = xgeXuiTooltipPaintProc;
	pContext->pTooltipPopupWidget->pUser = pContext;
	xgeXuiWidgetSetVisible(pContext->pTooltipPopupWidget, 0);
	xgeXuiWidgetSetEnabled(pContext->pTooltipPopupWidget, 0);
	xgeXuiWidgetSetClip(pContext->pTooltipPopupWidget, 0);
	(void)xgeXuiOverlayAttach(pContext, pContext->pTooltipPopupWidget, NULL, XGE_XUI_LAYER_TOOLTIP);
	pContext->pHost = &g_xgeXuiDefaultHost;
	pContext->bAutoDispatchProcFrameEvents = 1;
	pContext->bInitialized = 1;
#if XGE_HAS_DEBUGMODE
	pContext->iDirtyLayoutCount = 1;
	pContext->iDirtyPaintCount = 1;
#endif
	__xgeXuiProcFrameDispatchLink(pContext);
	return XGE_OK;
}

void xgeXuiUnit(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	__xgeXuiContextRestoreIme(pContext);
	__xgeXuiProcFrameDispatchUnlink(pContext);
	__xgeXuiWidgetFreeTree(pContext->pRoot);
	__xgeXuiWidgetFreeTree(pContext->pOverlayRoot);
	if ( pContext->pRegisteredTokens != NULL ) {
		xvoUnref((xvalue)pContext->pRegisteredTokens);
	}
	if ( pContext->bDefaultFontReady != 0 ) {
		xgeFontFree(&pContext->tDefaultFont);
	}
	memset(pContext, 0, sizeof(*pContext));
}

void xgeXuiSetProcFrameEventDispatch(xge_xui_context pContext, int bEnabled)
{
	if ( pContext == NULL ) {
		return;
	}
	pContext->bAutoDispatchProcFrameEvents = (bEnabled != 0);
}

xge_xui_widget xgeXuiRoot(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return NULL;
	}
	return pContext->pRoot;
}

xge_xui_widget xgeXuiOverlayRoot(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return NULL;
	}
	return pContext->pOverlayRoot;
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
		__xgeXuiWidgetMarkLayoutTree(pContext->pRoot);
		xgeXuiRefreshRequest(pContext);
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

void xgeXuiSetSafeAreaPx(xge_xui_context pContext, float fLeft, float fTop, float fRight, float fBottom)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pContext->pRoot == NULL) ) {
		return;
	}
	xgeXuiWidgetSetPaddingPx(pContext->pRoot, fLeft, fTop, fRight, fBottom);
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
			__xgeXuiWidgetMarkLayoutTree(pContext->pRoot);
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

static int __xgeXuiWidgetContainsWidget(xge_xui_widget pRoot, xge_xui_widget pWidget)
{
	xge_xui_widget pChild;

	if ( (pRoot == NULL) || (pWidget == NULL) ) {
		return 0;
	}
	if ( pRoot == pWidget ) {
		return 1;
	}
	for ( pChild = pRoot->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( __xgeXuiWidgetContainsWidget(pChild, pWidget) ) {
			return 1;
		}
	}
	return 0;
}

static void __xgeXuiOverlayClearOwnerRefs(xge_xui_context pContext, xge_xui_widget pWidget)
{
	xge_xui_widget pChild;

	if ( (pContext == NULL) || (pContext->pOverlayRoot == NULL) || (pWidget == NULL) ) {
		return;
	}
	for ( pChild = pContext->pOverlayRoot->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( __xgeXuiWidgetContainsWidget(pWidget, pChild->pOverlayOwner) ) {
			pChild->pOverlayOwner = NULL;
		}
	}
}

static void __xgeXuiWidgetDetachContextRefs(xge_xui_context pContext, xge_xui_widget pWidget)
{
	xge_xui_widget pOld;
	int i;

	if ( (pContext == NULL) || (pWidget == NULL) ) {
		return;
	}
	__xgeXuiOverlayClearOwnerRefs(pContext, pWidget);
	if ( __xgeXuiWidgetContainsWidget(pWidget, pContext->pCapture) ) {
		(void)__xgeXuiReleasePointerCapture(pContext, 0, 1);
	}
	for ( i = 1; i < XGE_XUI_POINTER_CAPTURE_CAPACITY; i++ ) {
		if ( __xgeXuiWidgetContainsWidget(pWidget, pContext->arrPointerCaptureWidget[i]) ) {
			(void)__xgeXuiReleasePointerCapture(pContext, pContext->arrPointerCaptureId[i], 1);
		}
	}
	if ( __xgeXuiWidgetContainsWidget(pWidget, pContext->pFocus) ) {
		pOld = pContext->pFocus;
		pContext->pFocus = NULL;
		__xgeXuiContextSyncIme(pContext);
		__xgeXuiDispatchFocusEvent(pOld, XGE_EVENT_XUI_FOCUS_OUT);
	}
	if ( __xgeXuiWidgetContainsWidget(pWidget, pContext->pHover) ) {
		pOld = pContext->pHover;
		(void)pOld;
		__xgeXuiHoverPathUpdate(pContext, NULL, 0);
	}
	if ( __xgeXuiWidgetContainsWidget(pWidget, pContext->pTooltipOwner) ) {
		pContext->pTooltipOwner = NULL;
		pContext->bTooltipOpen = 0;
		pContext->fTooltipHoverTime = 0.0f;
		memset(&pContext->tActiveTooltip, 0, sizeof(pContext->tActiveTooltip));
		if ( pContext->pTooltipPopupWidget != NULL ) {
			xgeXuiWidgetSetVisible(pContext->pTooltipPopupWidget, 0);
			pContext->pTooltipPopupWidget->pOverlayOwner = NULL;
			xgeXuiWidgetMarkPaint(pContext->pTooltipPopupWidget);
		}
	}
	if ( __xgeXuiWidgetContainsWidget(pWidget, pContext->pContextPressTarget) ) {
		__xgeXuiContextPressCancel(pContext);
	}
	if ( __xgeXuiWidgetContainsWidget(pWidget, pContext->pClickPressTarget) ) {
		__xgeXuiClickPressCancel(pContext);
	}
	if ( __xgeXuiWidgetContainsWidget(pWidget, pContext->pDragTarget) ) {
		__xgeXuiDragCancelState(pContext);
	}
	if ( __xgeXuiWidgetContainsWidget(pWidget, pContext->pLastClickTarget) ) {
		pContext->pLastClickTarget = NULL;
		pContext->iLastClickButton = 0;
		pContext->iLastClickPointerId = 0;
		pContext->fLastClickTime = 0.0;
	}
}

void xgeXuiWidgetFree(xge_xui_widget pWidget)
{
	xge_xui_context pContext;

	if ( pWidget == NULL ) {
		return;
	}
	pContext = __xgeXuiWidgetContext(pWidget);
	__xgeXuiWidgetDetachContextRefs(pContext, pWidget);
	xgeXuiHotKeyClearWidget(pContext, pWidget);
	xgeXuiWidgetRemove(pWidget);
	__xgeXuiWidgetFreeTree(pWidget);
}

static uint32_t __xgeXuiWidgetNextTreeOrder(xge_xui_widget pParent)
{
	xge_xui_context pContext;
	xge_xui_widget pChild;
	uint32_t iMax;

	if ( pParent == NULL ) {
		return 0;
	}
	pContext = __xgeXuiWidgetContext(pParent);
	if ( pContext != NULL ) {
		if ( pContext->iNextTreeOrder == 0 ) {
			pContext->iNextTreeOrder = 1;
		}
		return pContext->iNextTreeOrder++;
	}
	iMax = 0;
	for ( pChild = pParent->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( pChild->iTreeOrder > iMax ) {
			iMax = pChild->iTreeOrder;
		}
	}
	return iMax + 1;
}

static int __xgeXuiWidgetAdd(xge_xui_widget pParent, xge_xui_widget pChild, int bInternal)
{
	xge_xui_widget pIt;

	if ( (pParent == NULL) || (pChild == NULL) || (pParent == pChild) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (bInternal == 0) && (pParent->iRole == XGE_XUI_WIDGET_ROLE_CONTROL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( pIt = pParent; pIt != NULL; pIt = pIt->pParent ) {
		if ( pIt == pChild ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
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
	pChild->iTreeOrder = __xgeXuiWidgetNextTreeOrder(pParent);
	__xgeXuiWidgetRefreshEventMasksUp(pParent);
	xgeXuiWidgetMarkLayout(pParent);
	xgeXuiWidgetMarkPaint(pParent);
	return XGE_OK;
}

int xgeXuiWidgetAdd(xge_xui_widget pParent, xge_xui_widget pChild)
{
	return __xgeXuiWidgetAdd(pParent, pChild, 0);
}

int xgeXuiWidgetAddInternal(xge_xui_widget pParent, xge_xui_widget pChild)
{
	return __xgeXuiWidgetAdd(pParent, pChild, 1);
}

int xgeXuiOverlayAttach(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_widget pOwner, int iLayer)
{
	xge_xui_context pOwnerContext;
	xge_xui_context pWidgetContext;

	if ( (pContext == NULL) || (pContext->pOverlayRoot == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pOwnerContext = (pOwner != NULL) ? __xgeXuiWidgetContext(pOwner) : NULL;
	if ( (pOwnerContext != NULL) && (pOwnerContext != pContext) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pWidgetContext = __xgeXuiWidgetContext(pWidget);
	if ( (pWidgetContext != NULL) && (pWidgetContext != pContext) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiWidgetSetRole(pWidget, XGE_XUI_WIDGET_ROLE_OVERLAY);
	xgeXuiWidgetSetLayer(pWidget, iLayer);
	pWidget->pOverlayOwner = pOwner;
	if ( pWidget->pParent != pContext->pOverlayRoot ) {
		return xgeXuiWidgetAddInternal(pContext->pOverlayRoot, pWidget);
	}
	xgeXuiWidgetMarkLayout(pContext->pOverlayRoot);
	xgeXuiWidgetMarkPaint(pContext->pOverlayRoot);
	return XGE_OK;
}

void xgeXuiOverlayDetach(xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pContext == NULL) || (pContext->pOverlayRoot == NULL) || (pWidget == NULL) ) {
		return;
	}
	if ( pWidget->pParent == pContext->pOverlayRoot ) {
		xgeXuiWidgetRemove(pWidget);
	}
	pWidget->pOverlayOwner = NULL;
}

void xgeXuiOverlayBringToFront(xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pContext == NULL) || (pContext->pOverlayRoot == NULL) || (pWidget == NULL) || (pWidget->pParent != pContext->pOverlayRoot) ) {
		return;
	}
	pWidget->iTreeOrder = __xgeXuiWidgetNextTreeOrder(pContext->pOverlayRoot);
	xgeXuiWidgetMarkPaint(pContext->pOverlayRoot);
}

xge_xui_widget xgeXuiOverlayGetOwner(xge_xui_widget pWidget)
{
	return (pWidget != NULL) ? pWidget->pOverlayOwner : NULL;
}

xge_xui_widget xgeXuiOverlayTop(xge_xui_context pContext)
{
	xge_xui_widget pChild;
	xge_xui_widget pTop;

	if ( (pContext == NULL) || (pContext->pOverlayRoot == NULL) ) {
		return NULL;
	}
	pTop = NULL;
	for ( pChild = pContext->pOverlayRoot->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( (pChild->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 ) {
			continue;
		}
		if ( (pTop == NULL) || __xgeXuiChildHitBefore(pChild, pTop) ) {
			pTop = pChild;
		}
	}
	return pTop;
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

static int __xgeXuiWidgetRoleClamp(int iRole)
{
	if ( (iRole < XGE_XUI_WIDGET_ROLE_CONTROL) || (iRole > XGE_XUI_WIDGET_ROLE_OVERLAY) ) {
		return XGE_XUI_WIDGET_ROLE_CONTAINER;
	}
	return iRole;
}

void xgeXuiWidgetSetRole(xge_xui_widget pWidget, int iRole)
{
	if ( pWidget == NULL ) {
		return;
	}
	iRole = __xgeXuiWidgetRoleClamp(iRole);
	if ( pWidget->iRole == iRole ) {
		return;
	}
	pWidget->iRole = iRole;
	if ( (iRole == XGE_XUI_WIDGET_ROLE_CONTROL) && (pWidget->tStyle.iOverflow == XGE_XUI_OVERFLOW_VISIBLE) && (pWidget->tStyle.iClip == 0) ) {
		pWidget->tStyle.iOverflow = XGE_XUI_OVERFLOW_CLIP;
		pWidget->tStyle.iClip = 1;
		__xgeXuiWidgetSyncClipFlag(pWidget);
	}
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

int xgeXuiWidgetGetRole(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return XGE_XUI_WIDGET_ROLE_CONTAINER;
	}
	return __xgeXuiWidgetRoleClamp(pWidget->iRole);
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
	xge_xui_context pContext;
	xge_xui_widget pParent;

	if ( (pWidget == NULL) || (pWidget->pParent == NULL) ) {
		return;
	}
	pContext = __xgeXuiWidgetContext(pWidget);
	__xgeXuiWidgetDetachContextRefs(pContext, pWidget);
	xgeXuiHotKeyClearWidget(pContext, pWidget);
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
	if ( (pContext != NULL) && (pParent == pContext->pOverlayRoot) ) {
		pWidget->pOverlayOwner = NULL;
	}
	__xgeXuiWidgetRefreshEventMasksUp(pParent);
	xgeXuiWidgetMarkLayout(pParent);
	xgeXuiWidgetMarkPaint(pParent);
}

void xgeXuiWidgetSetRect(xge_xui_widget pWidget, xge_rect_t tRect)
{
	xge_rect_t tOldRect;

	if ( pWidget == NULL ) {
		return;
	}
	if ( __xgeXuiRectSame(pWidget->tRect, tRect) && __xgeXuiRectSame(pWidget->tLocalRect, tRect) ) {
		return;
	}
	tOldRect = pWidget->tRect;
	pWidget->tLocalRect = tRect;
	pWidget->tRect = tRect;
	__xgeXuiWidgetBoxUpdate(pWidget, __xgeXuiWidgetBoxParentRect(pWidget, tRect));
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	__xgeXuiDispatchStateEvent(pWidget, XGE_EVENT_XUI_BOUNDS_CHANGED, 0, 0, pWidget->tRect, tOldRect);
}

xge_rect_t xgeXuiWidgetGetRect(xge_xui_widget pWidget)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( pWidget != NULL ) {
		tRect = pWidget->tBorderRect;
		if ( (tRect.fW <= 0.0f) && (tRect.fH <= 0.0f) ) {
			tRect = pWidget->tRect;
		}
	}
	return tRect;
}

xge_rect_t xgeXuiWidgetGetOuterRect(xge_xui_widget pWidget)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( pWidget != NULL ) {
		tRect = pWidget->tOuterRect;
		if ( (tRect.fW <= 0.0f) && (tRect.fH <= 0.0f) ) {
			tRect = pWidget->tRect;
		}
	}
	return tRect;
}

xge_rect_t xgeXuiWidgetGetBorderRect(xge_xui_widget pWidget)
{
	return xgeXuiWidgetGetRect(pWidget);
}

xge_rect_t xgeXuiWidgetGetPaddingRect(xge_xui_widget pWidget)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( pWidget != NULL ) {
		tRect = pWidget->tPaddingRect;
		if ( (tRect.fW <= 0.0f) && (tRect.fH <= 0.0f) ) {
			tRect = pWidget->tRect;
		}
	}
	return tRect;
}

xge_rect_t xgeXuiWidgetGetContentRect(xge_xui_widget pWidget)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( pWidget != NULL ) {
		tRect = pWidget->tContentRect;
		if ( (tRect.fW <= 0.0f) && (tRect.fH <= 0.0f) ) {
			tRect = pWidget->tRect;
		}
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
	pWidget->tStyle.iJustify = __xgeXuiJustifyClamp(pWidget->tStyle.iJustify);
	pWidget->tStyle.iLayer = __xgeXuiLayerClamp(pWidget->tStyle.iLayer);
	if ( (pWidget->tStyle.iDock < XGE_XUI_DOCK_FILL) || (pWidget->tStyle.iDock > XGE_XUI_DOCK_BOTTOM) ) {
		pWidget->tStyle.iDock = XGE_XUI_DOCK_FILL;
	}
	pWidget->tStyle.iOverflow = __xgeXuiOverflowClamp(pWidget->tStyle.iOverflow);
	if ( (pWidget->tStyle.iClip != 0) && (pWidget->tStyle.iOverflow == XGE_XUI_OVERFLOW_VISIBLE) ) {
		pWidget->tStyle.iOverflow = XGE_XUI_OVERFLOW_CLIP;
	}
	if ( __xgeXuiOverflowClips(pWidget->tStyle.iOverflow) ) {
		pWidget->tStyle.iClip = 1;
	}
	pWidget->tStyle.fGap = __xgeXuiGap(pWidget->tStyle.fGap);
	if ( pWidget->tStyle.fRadius < 0.0f ) {
		pWidget->tStyle.fRadius = 0.0f;
	}
	if ( pWidget->tStyle.fBorderWidth < 0.0f ) {
		pWidget->tStyle.fBorderWidth = 0.0f;
	}
	if ( pWidget->tStyle.fFocusRingWidth < 0.0f ) {
		pWidget->tStyle.fFocusRingWidth = 0.0f;
	}
	if ( pWidget->tStyle.fDebugOutlineWidth < 0.0f ) {
		pWidget->tStyle.fDebugOutlineWidth = 0.0f;
	}
	__xgeXuiWidgetSyncClipFlag(pWidget);
	__xgeXuiWidgetBoxUpdate(pWidget, __xgeXuiWidgetBoxParentRect(pWidget, pWidget->tRect));
	xgeXuiWidgetMarkStyle(pWidget);
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

void xgeXuiWidgetSetDock(xge_xui_widget pWidget, int iDock)
{
	if ( pWidget == NULL ) {
		return;
	}
	if ( (iDock < XGE_XUI_DOCK_FILL) || (iDock > XGE_XUI_DOCK_BOTTOM) ) {
		iDock = XGE_XUI_DOCK_FILL;
	}
	pWidget->tStyle.iDock = iDock;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

int xgeXuiWidgetGetDock(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return XGE_XUI_DOCK_FILL;
	}
	return pWidget->tStyle.iDock;
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

void xgeXuiWidgetSetGridColumnSpan(xge_xui_widget pWidget, int iColumnSpan)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.iGridColumnSpan = (iColumnSpan > 1) ? iColumnSpan : 1;
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetGap(xge_xui_widget pWidget, float fGap)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.fGap = __xgeXuiGap(fGap);
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

void xgeXuiWidgetSetJustify(xge_xui_widget pWidget, int iJustify)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.iJustify = __xgeXuiJustifyClamp(iJustify);
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

void xgeXuiWidgetSetLayer(xge_xui_widget pWidget, int iLayer)
{
	if ( pWidget == NULL ) {
		return;
	}
	iLayer = __xgeXuiLayerClamp(iLayer);
	if ( pWidget->tStyle.iLayer == iLayer ) {
		return;
	}
	pWidget->tStyle.iLayer = iLayer;
	xgeXuiWidgetMarkPaint((pWidget->pParent != NULL) ? pWidget->pParent : pWidget);
}

int xgeXuiWidgetGetLayer(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return XGE_XUI_LAYER_NORMAL;
	}
	return __xgeXuiLayerClamp(pWidget->tStyle.iLayer);
}

uint32_t xgeXuiWidgetGetTreeOrder(xge_xui_widget pWidget)
{
	return (pWidget != NULL) ? pWidget->iTreeOrder : 0;
}

void xgeXuiWidgetSetOverflow(xge_xui_widget pWidget, int iOverflow)
{
	if ( pWidget == NULL ) {
		return;
	}
	iOverflow = __xgeXuiOverflowClamp(iOverflow);
	if ( pWidget->tStyle.iOverflow == iOverflow && pWidget->tStyle.iClip == __xgeXuiOverflowClips(iOverflow) ) {
		__xgeXuiWidgetSyncClipFlag(pWidget);
		return;
	}
	pWidget->tStyle.iOverflow = iOverflow;
	pWidget->tStyle.iClip = __xgeXuiOverflowClips(iOverflow);
	__xgeXuiWidgetSyncClipFlag(pWidget);
	xgeXuiWidgetMarkStyle(pWidget);
}

int xgeXuiWidgetGetOverflow(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return XGE_XUI_OVERFLOW_VISIBLE;
	}
	return __xgeXuiOverflowClamp(pWidget->tStyle.iOverflow);
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
	__xgeXuiWidgetBoxUpdate(pWidget, __xgeXuiWidgetBoxParentRect(pWidget, pWidget->tRect));
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
	__xgeXuiWidgetBoxUpdate(pWidget, __xgeXuiWidgetBoxParentRect(pWidget, pWidget->tRect));
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

void xgeXuiWidgetSetBorder(xge_xui_widget pWidget, float fWidth, uint32_t iColor)
{
	if ( pWidget == NULL ) {
		return;
	}
	if ( fWidth < 0.0f ) {
		fWidth = 0.0f;
	}
	pWidget->tStyle.fBorderWidth = fWidth;
	pWidget->tStyle.iBorderColor = iColor;
	__xgeXuiWidgetBoxUpdate(pWidget, __xgeXuiWidgetBoxParentRect(pWidget, pWidget->tRect));
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetFocusRing(xge_xui_widget pWidget, float fWidth, uint32_t iColor)
{
	if ( pWidget == NULL ) {
		return;
	}
	if ( fWidth < 0.0f ) {
		fWidth = 0.0f;
	}
	pWidget->tStyle.fFocusRingWidth = fWidth;
	pWidget->tStyle.iFocusRingColor = iColor;
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetDisabledOverlay(xge_xui_widget pWidget, uint32_t iColor)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.iDisabledOverlayColor = iColor;
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetDebugOutline(xge_xui_widget pWidget, float fWidth, uint32_t iColor)
{
	if ( pWidget == NULL ) {
		return;
	}
	if ( fWidth < 0.0f ) {
		fWidth = 0.0f;
	}
	pWidget->tStyle.fDebugOutlineWidth = fWidth;
	pWidget->tStyle.iDebugOutlineColor = iColor;
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetStateStyle(xge_xui_widget pWidget, int iState, const xge_xui_state_style_t* pStyle)
{
	int iIndex;

	if ( (pWidget == NULL) || (pStyle == NULL) ) {
		return;
	}
	iIndex = __xgeXuiWidgetStateStyleIndex(iState);
	pWidget->arrStateStyle[iIndex] = *pStyle;
	if ( pWidget->arrStateStyle[iIndex].fBorderWidth < 0.0f ) {
		pWidget->arrStateStyle[iIndex].fBorderWidth = 0.0f;
	}
	if ( pWidget->arrStateStyle[iIndex].fFocusRingWidth < 0.0f ) {
		pWidget->arrStateStyle[iIndex].fFocusRingWidth = 0.0f;
	}
	xgeXuiWidgetMarkPaint(pWidget);
}

const xge_xui_state_style_t* xgeXuiWidgetGetStateStyle(xge_xui_widget pWidget, int iState)
{
	if ( pWidget == NULL ) {
		return NULL;
	}
	return &pWidget->arrStateStyle[__xgeXuiWidgetStateStyleIndex(iState)];
}

void xgeXuiWidgetClearStateStyle(xge_xui_widget pWidget, int iState)
{
	if ( pWidget == NULL ) {
		return;
	}
	memset(&pWidget->arrStateStyle[__xgeXuiWidgetStateStyleIndex(iState)], 0, sizeof(pWidget->arrStateStyle[0]));
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetStateBackground(xge_xui_widget pWidget, int iState, uint32_t iColor)
{
	xge_xui_state_style_t tStyle;

	if ( pWidget == NULL ) {
		return;
	}
	tStyle = pWidget->arrStateStyle[__xgeXuiWidgetStateStyleIndex(iState)];
	tStyle.iMask |= XGE_XUI_STATE_STYLE_BACKGROUND;
	tStyle.iBackgroundColor = iColor;
	xgeXuiWidgetSetStateStyle(pWidget, iState, &tStyle);
}

void xgeXuiWidgetSetStateBorder(xge_xui_widget pWidget, int iState, float fWidth, uint32_t iColor)
{
	xge_xui_state_style_t tStyle;

	if ( pWidget == NULL ) {
		return;
	}
	if ( fWidth < 0.0f ) {
		fWidth = 0.0f;
	}
	tStyle = pWidget->arrStateStyle[__xgeXuiWidgetStateStyleIndex(iState)];
	tStyle.iMask |= XGE_XUI_STATE_STYLE_BORDER_WIDTH | XGE_XUI_STATE_STYLE_BORDER_COLOR;
	tStyle.fBorderWidth = fWidth;
	tStyle.iBorderColor = iColor;
	xgeXuiWidgetSetStateStyle(pWidget, iState, &tStyle);
}

void xgeXuiWidgetSetStateFocusRing(xge_xui_widget pWidget, int iState, float fWidth, uint32_t iColor)
{
	xge_xui_state_style_t tStyle;

	if ( pWidget == NULL ) {
		return;
	}
	if ( fWidth < 0.0f ) {
		fWidth = 0.0f;
	}
	tStyle = pWidget->arrStateStyle[__xgeXuiWidgetStateStyleIndex(iState)];
	tStyle.iMask |= XGE_XUI_STATE_STYLE_FOCUS_RING_WIDTH | XGE_XUI_STATE_STYLE_FOCUS_RING_COLOR;
	tStyle.fFocusRingWidth = fWidth;
	tStyle.iFocusRingColor = iColor;
	xgeXuiWidgetSetStateStyle(pWidget, iState, &tStyle);
}

void xgeXuiWidgetSetStateDisabledOverlay(xge_xui_widget pWidget, int iState, uint32_t iColor)
{
	xge_xui_state_style_t tStyle;

	if ( pWidget == NULL ) {
		return;
	}
	tStyle = pWidget->arrStateStyle[__xgeXuiWidgetStateStyleIndex(iState)];
	tStyle.iMask |= XGE_XUI_STATE_STYLE_DISABLED_OVERLAY;
	tStyle.iDisabledOverlayColor = iColor;
	xgeXuiWidgetSetStateStyle(pWidget, iState, &tStyle);
}

void xgeXuiWidgetSetVisualState(xge_xui_widget pWidget, int iState)
{
	if ( pWidget == NULL ) {
		return;
	}
	iState &= (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE | XGE_XUI_STATE_FOCUS | XGE_XUI_STATE_DISABLED | XGE_XUI_STATE_CHECKED);
	if ( pWidget->iVisualState == iState ) {
		return;
	}
	pWidget->iVisualState = iState;
	xgeXuiWidgetMarkPaint(pWidget);
}

int xgeXuiWidgetGetVisualState(xge_xui_widget pWidget)
{
	return (pWidget != NULL) ? pWidget->iVisualState : XGE_XUI_STATE_DISABLED;
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
	xge_xui_context pContext;

	if ( pWidget == NULL ) {
		return;
	}
	if ( bFocusable == 0 ) {
		pContext = __xgeXuiWidgetContext(pWidget);
		if ( (pContext != NULL) && (pContext->pFocus == pWidget) ) {
			pContext->pFocus = NULL;
			__xgeXuiContextSyncIme(pContext);
			__xgeXuiDispatchFocusEvent(pWidget, XGE_EVENT_XUI_FOCUS_OUT);
			xgeXuiWidgetMarkPaint(pWidget);
		}
	}
	__xgeXuiWidgetSetFlag(pWidget, XGE_XUI_WIDGET_FOCUSABLE, bFocusable);
	__xgeXuiWidgetSetFlag(pWidget, XGE_XUI_WIDGET_TAB_STOP, bFocusable);
}

void xgeXuiWidgetSetTabStop(xge_xui_widget pWidget, int bTabStop)
{
	__xgeXuiWidgetSetFlag(pWidget, XGE_XUI_WIDGET_TAB_STOP, bTabStop);
}

int xgeXuiWidgetIsTabStop(xge_xui_widget pWidget)
{
	return __xgeXuiWidgetCanTabFocus(pWidget);
}

void xgeXuiWidgetSetTabIndex(xge_xui_widget pWidget, int iTabIndex)
{
	if ( pWidget == NULL ) {
		return;
	}
	if ( pWidget->iTabIndex == iTabIndex ) {
		return;
	}
	pWidget->iTabIndex = iTabIndex;
	xgeXuiWidgetMarkPaint(pWidget);
}

int xgeXuiWidgetGetTabIndex(xge_xui_widget pWidget)
{
	return (pWidget != NULL) ? pWidget->iTabIndex : 0;
}

void xgeXuiWidgetSetFocusScope(xge_xui_widget pWidget, int bFocusScope)
{
	__xgeXuiWidgetSetFlag(pWidget, XGE_XUI_WIDGET_FOCUS_SCOPE, bFocusScope);
}

int xgeXuiWidgetIsFocusScope(xge_xui_widget pWidget)
{
	return (pWidget != NULL) && ((pWidget->iFlags & XGE_XUI_WIDGET_FOCUS_SCOPE) != 0);
}

void xgeXuiWidgetSetDefaultAction(xge_xui_widget pWidget, xge_xui_click_proc procAction, void* pUser)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procDefaultAction = procAction;
	pWidget->pDefaultActionUser = pUser;
}

void xgeXuiWidgetSetCancelAction(xge_xui_widget pWidget, xge_xui_click_proc procAction, void* pUser)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procCancelAction = procAction;
	pWidget->pCancelActionUser = pUser;
}

void xgeXuiWidgetSetImeMode(xge_xui_widget pWidget, int iImeMode)
{
	xge_xui_context pContext;

	if ( pWidget == NULL ) {
		return;
	}
	iImeMode = __xgeXuiImeModeClamp(iImeMode);
	if ( pWidget->iImeMode == iImeMode ) {
		return;
	}
	pWidget->iImeMode = iImeMode;
	pContext = __xgeXuiWidgetContext(pWidget);
	if ( (pContext != NULL) && (pContext->pFocus == pWidget) ) {
		__xgeXuiContextSyncIme(pContext);
	}
}

int xgeXuiWidgetGetImeMode(xge_xui_widget pWidget)
{
	return (pWidget != NULL) ? __xgeXuiImeModeClamp(pWidget->iImeMode) : XGE_XUI_IME_DISABLED;
}

void xgeXuiWidgetSetImeCandidateRect(xge_xui_widget pWidget, xge_xui_ime_candidate_rect_proc procRect, void* pUser)
{
	xge_xui_context pContext;

	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procImeCandidateRect = procRect;
	pWidget->pImeCandidateRectUser = (procRect != NULL) ? pUser : NULL;
	pContext = __xgeXuiWidgetContext(pWidget);
	if ( (pContext != NULL) && (pContext->pFocus == pWidget) ) {
		__xgeXuiContextSyncIme(pContext);
	}
}

xge_rect_t xgeXuiWidgetGetImeCandidateRect(xge_xui_widget pWidget)
{
	return __xgeXuiWidgetResolveImeCandidateRect(pWidget);
}

int xgeXuiHasImeCandidateRect(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return 0;
	}
	__xgeXuiContextUpdateImeCandidateRect(pContext, __xgeXuiWidgetWantsIme(pContext->pFocus));
	return pContext->bImeCandidateRectValid;
}

xge_rect_t xgeXuiGetImeCandidateRect(xge_xui_context pContext)
{
	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return __xgeXuiZeroRect();
	}
	__xgeXuiContextUpdateImeCandidateRect(pContext, __xgeXuiWidgetWantsIme(pContext->pFocus));
	if ( pContext->bImeCandidateRectValid == 0 ) {
		return __xgeXuiZeroRect();
	}
	return pContext->tImeCandidateRect;
}

void xgeXuiWidgetSetHitTestVisible(xge_xui_widget pWidget, int bVisible)
{
	__xgeXuiWidgetSetInputFlag(pWidget, XGE_XUI_WIDGET_HIT_TEST_VISIBLE, bVisible);
}

int xgeXuiWidgetIsHitTestVisible(xge_xui_widget pWidget)
{
	return (pWidget != NULL) && ((pWidget->iFlags & XGE_XUI_WIDGET_HIT_TEST_VISIBLE) != 0);
}

void xgeXuiWidgetSetInputTransparent(xge_xui_widget pWidget, int bTransparent)
{
	__xgeXuiWidgetSetInputFlag(pWidget, XGE_XUI_WIDGET_INPUT_TRANSPARENT, bTransparent);
}

int xgeXuiWidgetIsInputTransparent(xge_xui_widget pWidget)
{
	return (pWidget != NULL) && ((pWidget->iFlags & XGE_XUI_WIDGET_INPUT_TRANSPARENT) != 0);
}

void xgeXuiWidgetSetClip(xge_xui_widget pWidget, int bClip)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tStyle.iClip = bClip ? 1 : 0;
	pWidget->tStyle.iOverflow = bClip ? XGE_XUI_OVERFLOW_CLIP : XGE_XUI_OVERFLOW_VISIBLE;
	__xgeXuiWidgetSyncClipFlag(pWidget);
	xgeXuiWidgetMarkStyle(pWidget);
}

void xgeXuiWidgetSetDragEnabled(xge_xui_widget pWidget, int bEnabled)
{
	if ( pWidget == NULL ) {
		return;
	}
	if ( bEnabled ) {
		pWidget->iFlags |= XGE_XUI_WIDGET_DRAG_ENABLED;
	} else {
		pWidget->iFlags &= ~XGE_XUI_WIDGET_DRAG_ENABLED;
	}
	__xgeXuiWidgetSetEventInterest(pWidget, XGE_XUI_EVENT_MASK_DRAG, bEnabled || __xgeXuiWidgetHasDragHandler(pWidget));
}

int xgeXuiWidgetIsDragEnabled(xge_xui_widget pWidget)
{
	return (pWidget != NULL) && ((pWidget->iFlags & XGE_XUI_WIDGET_DRAG_ENABLED) != 0);
}

void xgeXuiWidgetSetEvent(xge_xui_widget pWidget, xge_xui_event_proc procEvent, void* pUser)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procEvent = procEvent;
	pWidget->pEventUser = pUser;
	if ( (procEvent != NULL) && (pUser != NULL) ) {
		pWidget->iCallbackFlags |= XGE_XUI_WIDGET_CALLBACK_EVENT;
	} else {
		pWidget->iCallbackFlags &= ~XGE_XUI_WIDGET_CALLBACK_EVENT;
	}
	__xgeXuiWidgetRefreshRawEventInterest(pWidget);
}

void xgeXuiWidgetSetCaptureEvent(xge_xui_widget pWidget, xge_xui_event_proc procEvent)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procCaptureEvent = procEvent;
	pWidget->pCaptureUser = NULL;
	__xgeXuiWidgetRefreshRawEventInterest(pWidget);
}

void xgeXuiWidgetSetCaptureEventUser(xge_xui_widget pWidget, xge_xui_event_proc procEvent, void* pUser)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procCaptureEvent = procEvent;
	pWidget->pCaptureUser = pUser;
	__xgeXuiWidgetRefreshRawEventInterest(pWidget);
}

void xgeXuiWidgetSetEventHandler(xge_xui_widget pWidget, int iEventType, xge_xui_event_proc procEvent, void* pUser)
{
	uint32_t iMask;

	if ( (pWidget == NULL) || (iEventType <= XGE_EVENT_NONE) || (iEventType >= XGE_XUI_WIDGET_EVENT_SLOT_COUNT) ) {
		return;
	}
	pWidget->arrEventProc[iEventType] = procEvent;
	pWidget->arrEventUser[iEventType] = pUser;
	iMask = __xgeXuiEventMaskFromType(iEventType);
	if ( iEventType == XGE_EVENT_XUI_HOTKEY ) {
		__xgeXuiHotKeyRefreshWidgetInterest(__xgeXuiWidgetContext(pWidget), pWidget);
	} else if ( iEventType == XGE_EVENT_XUI_COMMAND ) {
		__xgeXuiCommandRefreshWidgetInterest(__xgeXuiWidgetContext(pWidget), pWidget);
	} else if ( iMask == XGE_XUI_EVENT_MASK_DRAG ) {
		__xgeXuiWidgetSetEventInterest(pWidget, iMask, ((pWidget->iFlags & XGE_XUI_WIDGET_DRAG_ENABLED) != 0) || __xgeXuiWidgetHasDragHandler(pWidget));
	} else if ( iMask != 0 ) {
		__xgeXuiWidgetSetEventInterest(pWidget, iMask, procEvent != NULL);
	}
}

void xgeXuiWidgetSetEventInterest(xge_xui_widget pWidget, uint32_t iEventMask, int bEnabled)
{
	__xgeXuiWidgetSetEventInterest(pWidget, iEventMask, bEnabled);
}

uint32_t xgeXuiWidgetGetEventMask(xge_xui_widget pWidget)
{
	return (pWidget != NULL) ? pWidget->iEventMask : 0;
}

uint32_t xgeXuiWidgetGetSubtreeEventMask(xge_xui_widget pWidget)
{
	return (pWidget != NULL) ? pWidget->iSubtreeEventMask : 0;
}

int xgeXuiHotKeyRegister(xge_xui_context pContext, xge_xui_widget pWidget, int iKey, int iModifiers, xge_xui_event_proc procEvent, void* pUser)
{
	xge_xui_hotkey pHotKey;
	int i;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pWidget == NULL) || (procEvent == NULL) || (__xgeXuiWidgetContext(pWidget) != pContext) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iModifiers = __xgeXuiHotKeyNormalizeModifiers(iModifiers);
	for ( i = 0; i < pContext->iHotKeyCount; i++ ) {
		pHotKey = &pContext->arrHotKey[i];
		if ( (pHotKey->pWidget == pWidget) && (pHotKey->iKey == iKey) && (pHotKey->iModifiers == iModifiers) ) {
			pHotKey->procEvent = procEvent;
			pHotKey->pUser = pUser;
			pHotKey->iCommand = 0;
			pHotKey->sCommand = NULL;
			pHotKey->pCommandData = NULL;
			__xgeXuiHotKeyRefreshWidgetInterest(pContext, pWidget);
			__xgeXuiCommandRefreshWidgetInterest(pContext, pWidget);
			return XGE_OK;
		}
	}
	if ( pContext->iHotKeyCount >= XGE_XUI_HOTKEY_CAPACITY ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pHotKey = &pContext->arrHotKey[pContext->iHotKeyCount++];
	pHotKey->pWidget = pWidget;
	pHotKey->iKey = iKey;
	pHotKey->iModifiers = iModifiers;
	pHotKey->procEvent = procEvent;
	pHotKey->pUser = pUser;
	pHotKey->iCommand = 0;
	pHotKey->sCommand = NULL;
	pHotKey->pCommandData = NULL;
	__xgeXuiHotKeyRefreshWidgetInterest(pContext, pWidget);
	__xgeXuiCommandRefreshWidgetInterest(pContext, pWidget);
	return XGE_OK;
}

int xgeXuiHotKeyRegisterCommand(xge_xui_context pContext, xge_xui_widget pWidget, int iKey, int iModifiers, int iCommand, const char* sCommand, void* pData)
{
	xge_xui_hotkey pHotKey;
	int i;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pWidget == NULL) || (__xgeXuiWidgetContext(pWidget) != pContext) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iModifiers = __xgeXuiHotKeyNormalizeModifiers(iModifiers);
	for ( i = 0; i < pContext->iHotKeyCount; i++ ) {
		pHotKey = &pContext->arrHotKey[i];
		if ( (pHotKey->pWidget == pWidget) && (pHotKey->iKey == iKey) && (pHotKey->iModifiers == iModifiers) ) {
			pHotKey->procEvent = NULL;
			pHotKey->pUser = NULL;
			pHotKey->iCommand = iCommand;
			pHotKey->sCommand = sCommand;
			pHotKey->pCommandData = pData;
			__xgeXuiHotKeyRefreshWidgetInterest(pContext, pWidget);
			__xgeXuiCommandRefreshWidgetInterest(pContext, pWidget);
			return XGE_OK;
		}
	}
	if ( pContext->iHotKeyCount >= XGE_XUI_HOTKEY_CAPACITY ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pHotKey = &pContext->arrHotKey[pContext->iHotKeyCount++];
	pHotKey->pWidget = pWidget;
	pHotKey->iKey = iKey;
	pHotKey->iModifiers = iModifiers;
	pHotKey->procEvent = NULL;
	pHotKey->pUser = NULL;
	pHotKey->iCommand = iCommand;
	pHotKey->sCommand = sCommand;
	pHotKey->pCommandData = pData;
	__xgeXuiHotKeyRefreshWidgetInterest(pContext, pWidget);
	__xgeXuiCommandRefreshWidgetInterest(pContext, pWidget);
	return XGE_OK;
}

void xgeXuiHotKeyUnregister(xge_xui_context pContext, xge_xui_widget pWidget, int iKey, int iModifiers)
{
	int i;

	if ( (pContext == NULL) || (pWidget == NULL) ) {
		return;
	}
	iModifiers = __xgeXuiHotKeyNormalizeModifiers(iModifiers);
	for ( i = 0; i < pContext->iHotKeyCount; i++ ) {
		if ( (pContext->arrHotKey[i].pWidget == pWidget) && (pContext->arrHotKey[i].iKey == iKey) && (pContext->arrHotKey[i].iModifiers == iModifiers) ) {
			if ( i + 1 < pContext->iHotKeyCount ) {
				memmove(&pContext->arrHotKey[i], &pContext->arrHotKey[i + 1], (size_t)(pContext->iHotKeyCount - i - 1) * sizeof(pContext->arrHotKey[0]));
			}
			pContext->iHotKeyCount--;
			memset(&pContext->arrHotKey[pContext->iHotKeyCount], 0, sizeof(pContext->arrHotKey[pContext->iHotKeyCount]));
			i--;
		}
	}
	__xgeXuiHotKeyRefreshWidgetInterest(pContext, pWidget);
	__xgeXuiCommandRefreshWidgetInterest(pContext, pWidget);
}

void xgeXuiHotKeyClearWidget(xge_xui_context pContext, xge_xui_widget pWidget)
{
	int i;

	if ( (pContext == NULL) || (pWidget == NULL) ) {
		return;
	}
	for ( i = 0; i < pContext->iHotKeyCount; i++ ) {
		if ( __xgeXuiWidgetContainsWidget(pWidget, pContext->arrHotKey[i].pWidget) ) {
			if ( i + 1 < pContext->iHotKeyCount ) {
				memmove(&pContext->arrHotKey[i], &pContext->arrHotKey[i + 1], (size_t)(pContext->iHotKeyCount - i - 1) * sizeof(pContext->arrHotKey[0]));
			}
			pContext->iHotKeyCount--;
			memset(&pContext->arrHotKey[pContext->iHotKeyCount], 0, sizeof(pContext->arrHotKey[pContext->iHotKeyCount]));
			i--;
		}
	}
	__xgeXuiHotKeyRefreshWidgetInterest(pContext, pWidget);
	__xgeXuiCommandRefreshWidgetInterest(pContext, pWidget);
}

int xgeXuiCommandDispatch(xge_xui_context pContext, xge_xui_widget pTarget, xge_xui_widget pSource, int iCommand, const char* sCommand, void* pData)
{
	xge_xui_command_t tCommand;
	xge_event_t tEvent;
	int iResult;
	int iCaptureResult;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (__xgeXuiWidgetCanCommand(pContext, pTarget) == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pSource != NULL) && (__xgeXuiWidgetContext(pSource) != pContext) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	memset(&tCommand, 0, sizeof(tCommand));
	tCommand.iCommand = iCommand;
	tCommand.sCommand = sCommand;
	tCommand.pSource = (pSource != NULL) ? pSource : pTarget;
	tCommand.pData = pData;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_XUI_COMMAND;
	tEvent.iParam1 = iCommand;
	tEvent.pData = &tCommand;
	iCaptureResult = __xgeXuiDispatchCaptureToWidget(pTarget, pTarget, NULL, &tEvent);
	if ( iCaptureResult == XGE_XUI_EVENT_CONSUMED ) {
		iResult = XGE_XUI_EVENT_CONSUMED;
	} else {
		iResult = __xgeXuiEventResultMerge(iCaptureResult, __xgeXuiDispatchToWidget(pTarget, pTarget, NULL, &tEvent));
	}
	return iResult;
}

void xgeXuiWidgetSetUpdate(xge_xui_widget pWidget, xge_xui_update_proc procUpdate, void* pUser)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procUpdate = procUpdate;
	pWidget->pUpdateUser = pUser;
	pWidget->iCallbackFlags |= XGE_XUI_WIDGET_CALLBACK_UPDATE;
}

void xgeXuiWidgetSetMeasure(xge_xui_widget pWidget, xge_xui_measure_proc procMeasure)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procMeasure = procMeasure;
	pWidget->iCallbackFlags &= ~XGE_XUI_WIDGET_CALLBACK_MEASURE;
	xgeXuiWidgetMarkLayout(pWidget);
}

void xgeXuiWidgetSetMeasureUser(xge_xui_widget pWidget, xge_xui_measure_proc procMeasure, void* pUser)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procMeasure = procMeasure;
	pWidget->pMeasureUser = pUser;
	pWidget->iCallbackFlags |= XGE_XUI_WIDGET_CALLBACK_MEASURE;
	xgeXuiWidgetMarkLayout(pWidget);
}

void xgeXuiWidgetSetLayoutProc(xge_xui_widget pWidget, xge_xui_layout_proc procLayout, void* pUser)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procLayout = procLayout;
	pWidget->pLayoutUser = pUser;
	xgeXuiWidgetMarkLayout(pWidget);
}

void xgeXuiWidgetSetPaintBefore(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procPaintBefore = procPaint;
	pWidget->pPaintBeforeUser = pUser;
	pWidget->iCallbackFlags |= XGE_XUI_WIDGET_CALLBACK_PAINT_BEFORE;
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetPaint(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procPaint = procPaint;
	pWidget->pPaintUser = pUser;
	pWidget->iCallbackFlags |= XGE_XUI_WIDGET_CALLBACK_PAINT;
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetPaintAfter(xge_xui_widget pWidget, xge_xui_paint_proc procPaint, void* pUser)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->procPaintAfter = procPaint;
	pWidget->pPaintAfterUser = pUser;
	pWidget->iCallbackFlags |= XGE_XUI_WIDGET_CALLBACK_PAINT_AFTER;
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetOwnerDraw(xge_xui_widget pWidget, int iMode, xge_xui_owner_draw_proc procDraw, void* pUser)
{
	if ( pWidget == NULL ) {
		return;
	}
	iMode = __xgeXuiOwnerDrawModeClamp(iMode);
	if ( procDraw == NULL ) {
		iMode = XGE_XUI_OWNER_DRAW_NONE;
	}
	pWidget->procOwnerDraw = procDraw;
	pWidget->pOwnerDrawUser = pUser;
	pWidget->iOwnerDrawMode = iMode;
	xgeXuiWidgetMarkPaint(pWidget);
}

void xgeXuiWidgetSetOwnerDrawControl(xge_xui_widget pWidget, void* pControl)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->pOwnerDrawControl = pControl;
	xgeXuiWidgetMarkPaint(pWidget);
}

int xgeXuiWidgetGetOwnerDrawMode(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return XGE_XUI_OWNER_DRAW_NONE;
	}
	return __xgeXuiOwnerDrawModeClamp(pWidget->iOwnerDrawMode);
}

xge_vec2_t xgeXuiWidgetGetDesiredSize(xge_xui_widget pWidget)
{
	xge_vec2_t tSize;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( pWidget == NULL ) {
		return tSize;
	}
	return pWidget->tDesiredSize;
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

void xgeXuiWidgetMarkStyle(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->iFlags |= XGE_XUI_WIDGET_DIRTY_STYLE;
	pWidget->iStyleVersion++;
	if ( pWidget->iStyleVersion == 0 ) {
		pWidget->iStyleVersion = 1;
	}
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
}

xge_xui_widget xgeXuiHitTest(xge_xui_context pContext, float fX, float fY)
{
	xge_xui_widget pHit;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pContext->pRoot == NULL) ) {
		return NULL;
	}
	pHit = __xgeXuiHitTestWidget(pContext->pOverlayRoot, fX, fY);
	if ( (pHit != NULL) && (pHit != pContext->pOverlayRoot) ) {
		return pHit;
	}
	return __xgeXuiHitTestWidget(pContext->pRoot, fX, fY);
}

void xgeXuiSetFocus(xge_xui_context pContext, xge_xui_widget pWidget)
{
	xge_xui_widget pOldFocus;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	if ( (pWidget != NULL) && ((__xgeXuiWidgetContext(pWidget) != pContext) || (__xgeXuiWidgetCanFocus(pWidget) == 0)) ) {
		return;
	}
	if ( pContext->pFocus != pWidget ) {
		pOldFocus = pContext->pFocus;
		pContext->pFocus = NULL;
		if ( (pOldFocus != NULL) || (pWidget == NULL) ) {
			__xgeXuiContextSyncIme(pContext);
		}
		__xgeXuiDispatchFocusEvent(pOldFocus, XGE_EVENT_XUI_FOCUS_OUT);
		xgeXuiWidgetMarkPaint(pOldFocus);
		pContext->pFocus = pWidget;
		__xgeXuiContextSyncIme(pContext);
		__xgeXuiDispatchFocusEvent(pWidget, XGE_EVENT_XUI_FOCUS_IN);
		xgeXuiWidgetMarkPaint(pContext->pFocus);
	}
}

void xgeXuiSetCapture(xge_xui_context pContext, xge_xui_widget pWidget)
{
	xgeXuiSetPointerCapture(pContext, 0, pWidget);
}

void xgeXuiSetPointerCapture(xge_xui_context pContext, uint64_t iPointerId, xge_xui_widget pWidget)
{
	xge_xui_widget pOldCapture;
	int iSlot;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return;
	}
	if ( (pWidget != NULL) && ((__xgeXuiWidgetContext(pWidget) != pContext) || ((pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || ((pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0)) ) {
		return;
	}
	iPointerId = __xgeXuiPointerIdNormalize(iPointerId);
	if ( iPointerId == 0 ) {
		pOldCapture = pContext->pCapture;
		if ( pOldCapture == pWidget ) {
			return;
		}
		pContext->pCapture = pWidget;
		pContext->arrPointerCaptureId[0] = 0;
		pContext->arrPointerCaptureWidget[0] = pWidget;
		if ( (pOldCapture != NULL) && (pWidget != NULL) ) {
			__xgeXuiDispatchWidgetPointerEvent(pOldCapture, XGE_EVENT_XUI_CAPTURE_LOST, 0);
			__xgeXuiDispatchWidgetPointerEvent(pOldCapture, XGE_EVENT_XUI_CAPTURE_CANCEL, 0);
		}
		return;
	}
	iSlot = __xgeXuiPointerCaptureSlot(pContext, iPointerId);
	pOldCapture = (iSlot >= 0) ? pContext->arrPointerCaptureWidget[iSlot] : NULL;
	if ( pOldCapture == pWidget ) {
		return;
	}
	if ( pWidget == NULL ) {
		if ( iSlot >= 0 ) {
			__xgeXuiPointerCaptureClearSlot(pContext, iSlot);
		}
		return;
	}
	if ( iSlot < 0 ) {
		iSlot = __xgeXuiPointerCaptureEmptySlot(pContext);
		if ( iSlot < 0 ) {
			return;
		}
	}
	pContext->arrPointerCaptureId[iSlot] = iPointerId;
	pContext->arrPointerCaptureWidget[iSlot] = pWidget;
	if ( (pOldCapture != NULL) && (pWidget != NULL) ) {
		__xgeXuiDispatchWidgetPointerEvent(pOldCapture, XGE_EVENT_XUI_CAPTURE_LOST, iPointerId);
		__xgeXuiDispatchWidgetPointerEvent(pOldCapture, XGE_EVENT_XUI_CAPTURE_CANCEL, iPointerId);
	}
}

xge_xui_widget xgeXuiGetPointerCapture(xge_xui_context pContext, uint64_t iPointerId)
{
	int iSlot;

	if ( pContext == NULL ) {
		return NULL;
	}
	iPointerId = __xgeXuiPointerIdNormalize(iPointerId);
	if ( iPointerId == 0 ) {
		return pContext->pCapture;
	}
	iSlot = __xgeXuiPointerCaptureSlot(pContext, iPointerId);
	return (iSlot >= 0) ? pContext->arrPointerCaptureWidget[iSlot] : NULL;
}

int xgeXuiHasCapture(xge_xui_context pContext)
{
	int i;

	if ( pContext == NULL ) {
		return 0;
	}
	if ( pContext->pCapture != NULL ) {
		return 1;
	}
	for ( i = 1; i < XGE_XUI_POINTER_CAPTURE_CAPACITY; i++ ) {
		if ( pContext->arrPointerCaptureWidget[i] != NULL ) {
			return 1;
		}
	}
	return 0;
}

int xgeXuiWidgetHasCapture(xge_xui_context pContext, xge_xui_widget pWidget)
{
	int i;

	if ( (pContext == NULL) || (pWidget == NULL) ) {
		return 0;
	}
	if ( pContext->pCapture == pWidget ) {
		return 1;
	}
	for ( i = 1; i < XGE_XUI_POINTER_CAPTURE_CAPACITY; i++ ) {
		if ( pContext->arrPointerCaptureWidget[i] == pWidget ) {
			return 1;
		}
	}
	return 0;
}

void xgeXuiReleaseWidgetCapture(xge_xui_context pContext, xge_xui_widget pWidget)
{
	(void)__xgeXuiReleaseWidgetCaptures(pContext, pWidget, 0);
}

int xgeXuiDispatchEvent(xge_xui_context pContext, const xge_event_t* pEvent)
{
	xge_xui_widget pTarget;
	xge_xui_widget pHit;
	xge_xui_widget pOriginalTarget;
	xge_xui_widget pEventCapture;
	uint32_t iDispatchMask;
	float fDX;
	float fDY;
	int iResult;
	int iCaptureResult;
	int iOverlayResult;
	int iSyntheticResult;
	int iDragResult;
	uint64_t iPointerId;
	int bPointEvent;
	int bMoveEvent;
	int bHitRequired;
	int bClickReleaseMatches;
	int bDoubleClick;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pContext->pRoot == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_TAB) ) {
		return __xgeXuiFocusStep(pContext, (pEvent->iParam2 & XGE_KEY_MOD_SHIFT) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) && xgeXuiHasCapture(pContext) ) {
		if ( pContext->bDragActive != 0 ) {
			xge_event_t tDragEvent;
			tDragEvent = *pEvent;
			tDragEvent.fX = pContext->fDragLastX;
			tDragEvent.fY = pContext->fDragLastY;
			(void)__xgeXuiDispatchDragEvent(pContext, &tDragEvent, XGE_EVENT_XUI_DRAG_CANCEL);
			__xgeXuiDragCancelState(pContext);
		}
		__xgeXuiReleaseAllCaptures(pContext, 1);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN ) {
		iResult = __xgeXuiHotKeyDispatch(pContext, pEvent);
		if ( iResult != XGE_XUI_EVENT_CONTINUE ) {
			return iResult;
		}
	}
	iPointerId = __xgeXuiEventPointerId(pEvent);
	bPointEvent = __xgeXuiEventHasPoint(pEvent);
	bMoveEvent = (pEvent->iType == XGE_EVENT_MOUSE_MOVE) || (pEvent->iType == XGE_EVENT_TOUCH_MOVE);
	pEventCapture = bPointEvent ? xgeXuiGetPointerCapture(pContext, iPointerId) : pContext->pCapture;
	iDispatchMask = 0;
	if ( pContext->pRoot != NULL ) {
		iDispatchMask |= pContext->pRoot->iSubtreeEventMask;
	}
	if ( pContext->pOverlayRoot != NULL ) {
		iDispatchMask |= pContext->pOverlayRoot->iSubtreeEventMask;
	}
	if ( ((iDispatchMask & XGE_XUI_EVENT_MASK_CONTEXT_MENU) != 0) && __xgeXuiEventIsKeyboardContextMenu(pEvent) ) {
		iResult = __xgeXuiDispatchKeyboardContextMenu(pContext, pEvent);
		if ( iResult != XGE_XUI_EVENT_CONTINUE ) {
			return iResult;
		}
	}
	bHitRequired = bPointEvent && ((bMoveEvent == 0) || (pEventCapture != NULL) || ((iDispatchMask & (XGE_XUI_EVENT_MASK_RAW | XGE_XUI_EVENT_MASK_MOUSE_MOVE | XGE_XUI_EVENT_MASK_MOUSE_ENTER | XGE_XUI_EVENT_MASK_MOUSE_LEAVE | XGE_XUI_EVENT_MASK_TOOLTIP | XGE_XUI_EVENT_MASK_DRAG)) != 0));
	pHit = bHitRequired ? xgeXuiHitTest(pContext, pEvent->fX, pEvent->fY) : NULL;
	if ( bHitRequired ) {
		xgeXuiTooltipHandleEvent(pContext, pHit, pEvent);
	}
	if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 == XGE_MOUSE_RIGHT) ) {
		pContext->pContextPressTarget = pHit;
		pContext->iContextPressPointerId = iPointerId;
		iResult = __xgeXuiDispatchContextEvent(pContext, pHit, XGE_EVENT_XUI_CONTEXT_BEGIN, pEvent->fX, pEvent->fY, iPointerId);
		if ( iResult == XGE_XUI_EVENT_CONSUMED ) {
			pContext->bContextRightActive = 1;
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( ((iDispatchMask & XGE_XUI_EVENT_MASK_CONTEXT_MENU) != 0) && (pHit != NULL) ) {
			pContext->bContextRightActive = 1;
		} else {
			pContext->bContextRightActive = 0;
			pContext->pContextPressTarget = NULL;
		}
	}
	if ( (pEvent->iType == XGE_EVENT_MOUSE_UP) && (pEvent->iParam1 == XGE_MOUSE_RIGHT) && (pContext->bContextRightActive != 0) ) {
		pTarget = (pContext->pContextPressTarget != NULL) ? pContext->pContextPressTarget : pHit;
		iResult = __xgeXuiDispatchContextEvent(pContext, pTarget, XGE_EVENT_XUI_CONTEXT_END, pEvent->fX, pEvent->fY, pContext->iContextPressPointerId);
		if ( (iDispatchMask & XGE_XUI_EVENT_MASK_CONTEXT_MENU) != 0 ) {
			iSyntheticResult = __xgeXuiDispatchContextEvent(pContext, pTarget, XGE_EVENT_XUI_CONTEXT_MENU, pEvent->fX, pEvent->fY, pContext->iContextPressPointerId);
			iResult = __xgeXuiEventResultMerge(iResult, iSyntheticResult);
		}
		pContext->pContextPressTarget = NULL;
		pContext->bContextRightActive = 0;
		return iResult;
	}
	if ( __xgeXuiContextPressIsPrimaryDown(pEvent) ) {
		pContext->bContextPressActive = 1;
		pContext->bContextPressMoved = 0;
		pContext->bContextPressFired = 0;
		pContext->fContextPressTime = 0.0f;
		pContext->fContextPressStartX = pEvent->fX;
		pContext->fContextPressStartY = pEvent->fY;
		pContext->fContextPressLastX = pEvent->fX;
		pContext->fContextPressLastY = pEvent->fY;
		pContext->iContextPressPointerId = iPointerId;
		pContext->pContextPressTarget = pHit;
	} else if ( pContext->bContextPressActive != 0 && __xgeXuiContextPressIsMove(pEvent) ) {
		pContext->fContextPressLastX = pEvent->fX;
		pContext->fContextPressLastY = pEvent->fY;
		if ( pContext->bContextPressFired != 0 ) {
			(void)__xgeXuiDispatchContextEvent(pContext, pContext->pContextPressTarget, XGE_EVENT_XUI_CONTEXT_UPDATE, pEvent->fX, pEvent->fY, pContext->iContextPressPointerId);
			return XGE_XUI_EVENT_CONSUMED;
		}
		fDX = pEvent->fX - pContext->fContextPressStartX;
		fDY = pEvent->fY - pContext->fContextPressStartY;
		if ( (fDX * fDX + fDY * fDY) > 36.0f ) {
			pContext->bContextPressMoved = 1;
		}
	} else if ( pContext->bContextPressActive != 0 && __xgeXuiContextPressIsRelease(pEvent) ) {
		if ( pContext->bContextPressFired != 0 ) {
			iResult = __xgeXuiDispatchContextEvent(pContext, pContext->pContextPressTarget, XGE_EVENT_XUI_CONTEXT_END, pEvent->fX, pEvent->fY, pContext->iContextPressPointerId);
			__xgeXuiContextPressCancel(pContext);
			return iResult;
		}
	} else if ( pContext->bContextPressActive != 0 && __xgeXuiContextPressIsCancel(pEvent) ) {
		if ( pContext->bContextPressFired != 0 ) {
			(void)__xgeXuiDispatchContextEvent(pContext, pContext->pContextPressTarget, XGE_EVENT_XUI_CONTEXT_CANCEL, pEvent->fX, pEvent->fY, pContext->iContextPressPointerId);
		}
		__xgeXuiContextPressCancel(pContext);
	}
	if ( (pContext->bClickPressActive != 0) && (pEvent->iType == XGE_EVENT_MOUSE_MOVE) ) {
		fDX = pEvent->fX - pContext->fClickPressX;
		fDY = pEvent->fY - pContext->fClickPressY;
		if ( (fDX * fDX + fDY * fDY) > 36.0f ) {
			pContext->bClickPressMoved = 1;
		}
	}
	if ( bHitRequired && (pEventCapture == NULL) && (pContext->pOverlayRoot != NULL) ) {
		if ( (pHit == NULL) || (pHit->pParent != pContext->pOverlayRoot) ) {
			iOverlayResult = __xgeXuiDispatchOverlayPoint(pContext, pEvent, pHit);
			if ( iOverlayResult != XGE_XUI_EVENT_CONTINUE ) {
				return iOverlayResult;
			}
		}
	}
	if ( bMoveEvent && (pEventCapture == NULL) ) {
		if ( bHitRequired ) {
			__xgeXuiHoverPathUpdate(pContext, pHit, iPointerId);
		} else if ( pContext->iHoverPathCount != 0 ) {
			__xgeXuiHoverPathUpdate(pContext, NULL, iPointerId);
		}
	}
	if ( __xgeXuiContextPressIsPrimaryDown(pEvent) && ((iDispatchMask & XGE_XUI_EVENT_MASK_DRAG) != 0) ) {
		pContext->pDragTarget = __xgeXuiDragCandidate(pHit);
		if ( pContext->pDragTarget != NULL ) {
			pContext->bDragPressActive = 1;
			pContext->bDragActive = 0;
			pContext->iDragPointerId = iPointerId;
			pContext->fDragStartX = pEvent->fX;
			pContext->fDragStartY = pEvent->fY;
			pContext->fDragLastX = pEvent->fX;
			pContext->fDragLastY = pEvent->fY;
		}
	}
	if ( (pContext->bDragPressActive != 0) && (pContext->iDragPointerId == iPointerId) && __xgeXuiContextPressIsMove(pEvent) ) {
		fDX = pEvent->fX - pContext->fDragStartX;
		fDY = pEvent->fY - pContext->fDragStartY;
		if ( (pContext->bDragActive == 0) && ((fDX * fDX + fDY * fDY) > 36.0f) ) {
			pContext->bDragActive = 1;
			xgeXuiSetPointerCapture(pContext, iPointerId, pContext->pDragTarget);
			iDragResult = __xgeXuiDispatchDragEvent(pContext, pEvent, XGE_EVENT_XUI_DRAG_BEGIN);
			if ( iDragResult == XGE_XUI_EVENT_CONSUMED ) {
				__xgeXuiClickPressCancel(pContext);
				return XGE_XUI_EVENT_CONSUMED;
			}
		}
		if ( pContext->bDragActive != 0 ) {
			iDragResult = __xgeXuiDispatchDragEvent(pContext, pEvent, XGE_EVENT_XUI_DRAG_MOVE);
			__xgeXuiClickPressCancel(pContext);
			if ( iDragResult != XGE_XUI_EVENT_CONTINUE ) {
				return iDragResult;
			}
		}
	}
	if ( (pContext->bDragPressActive != 0) && (pContext->iDragPointerId == iPointerId) && __xgeXuiContextPressIsRelease(pEvent) ) {
		if ( pContext->bDragActive != 0 ) {
			iDragResult = __xgeXuiDispatchDragEvent(pContext, pEvent, XGE_EVENT_XUI_DRAG_END);
			xgeXuiSetPointerCapture(pContext, iPointerId, NULL);
			__xgeXuiDragCancelState(pContext);
			return iDragResult;
		}
		__xgeXuiDragCancelState(pContext);
	} else if ( (pContext->bDragPressActive != 0) && (pContext->iDragPointerId == iPointerId) && __xgeXuiContextPressIsCancel(pEvent) ) {
		if ( pContext->bDragActive != 0 ) {
			iDragResult = __xgeXuiDispatchDragEvent(pContext, pEvent, XGE_EVENT_XUI_DRAG_CANCEL);
			xgeXuiSetPointerCapture(pContext, iPointerId, NULL);
			__xgeXuiDragCancelState(pContext);
			return iDragResult;
		}
		__xgeXuiDragCancelState(pContext);
	}
	if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 == XGE_MOUSE_LEFT) && ((iDispatchMask & (XGE_XUI_EVENT_MASK_CLICK | XGE_XUI_EVENT_MASK_DOUBLE_CLICK)) != 0) && (pHit != NULL) ) {
		pContext->bClickPressActive = 1;
		pContext->bClickPressMoved = 0;
		pContext->iClickPressButton = XGE_MOUSE_LEFT;
		pContext->iClickPressPointerId = iPointerId;
		pContext->fClickPressX = pEvent->fX;
		pContext->fClickPressY = pEvent->fY;
		pContext->pClickPressTarget = pHit;
	}
	bClickReleaseMatches = __xgeXuiClickReleaseMatches(pContext, pHit, pEvent);
	bDoubleClick = bClickReleaseMatches ? __xgeXuiClickIsDouble(pContext, pEvent) : 0;
	if ( pEventCapture != NULL ) {
		pTarget = pEventCapture;
		pOriginalTarget = (pHit != NULL) ? pHit : pTarget;
	} else if ( bPointEvent ) {
		pTarget = pHit;
		pOriginalTarget = pHit;
	} else {
		pTarget = __xgeXuiFocusEventTarget(pContext);
		pOriginalTarget = pTarget;
	}
	if ( pTarget == NULL ) {
		if ( (pEvent->iType == XGE_EVENT_MOUSE_UP) && (pEvent->iParam1 == XGE_MOUSE_LEFT) ) {
			__xgeXuiClickPressCancel(pContext);
		}
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( ((pEvent->iType == XGE_EVENT_IME_START) || (pEvent->iType == XGE_EVENT_IME_UPDATE) || (pEvent->iType == XGE_EVENT_IME_END)) && (__xgeXuiWidgetWantsIme(pTarget) == 0) ) {
		return XGE_XUI_EVENT_CONSUMED;
	}
	iCaptureResult = __xgeXuiDispatchCaptureToWidget(pTarget, pOriginalTarget, pEventCapture, pEvent);
	if ( iCaptureResult == XGE_XUI_EVENT_CONSUMED ) {
		iResult = XGE_XUI_EVENT_CONSUMED;
	} else {
		iResult = __xgeXuiEventResultMerge(iCaptureResult, __xgeXuiDispatchToWidget(pTarget, pOriginalTarget, pEventCapture, pEvent));
	}
	if ( __xgeXuiContextPressIsRelease(pEvent) || __xgeXuiContextPressIsCancel(pEvent) ) {
		__xgeXuiContextPressCancel(pContext);
	}
	if ( (pEvent->iType == XGE_EVENT_MOUSE_UP) && (pEvent->iParam1 == XGE_MOUSE_LEFT) ) {
		if ( bClickReleaseMatches ) {
			if ( (iDispatchMask & XGE_XUI_EVENT_MASK_CLICK) != 0 ) {
				iSyntheticResult = __xgeXuiDispatchSyntheticEvent(pContext, pContext->pClickPressTarget, pContext->pClickPressTarget, pEventCapture, pEvent, XGE_EVENT_XUI_CLICK);
				iResult = __xgeXuiEventResultMerge(iResult, iSyntheticResult);
			}
			if ( bDoubleClick && ((iDispatchMask & XGE_XUI_EVENT_MASK_DOUBLE_CLICK) != 0) ) {
				iSyntheticResult = __xgeXuiDispatchSyntheticEvent(pContext, pContext->pClickPressTarget, pContext->pClickPressTarget, pEventCapture, pEvent, XGE_EVENT_XUI_DOUBLE_CLICK);
				iResult = __xgeXuiEventResultMerge(iResult, iSyntheticResult);
			}
			pContext->pLastClickTarget = pContext->pClickPressTarget;
			pContext->iLastClickButton = pContext->iClickPressButton;
			pContext->iLastClickPointerId = pContext->iClickPressPointerId;
			pContext->fLastClickTime = xgeTimer();
			pContext->fLastClickX = pEvent->fX;
			pContext->fLastClickY = pEvent->fY;
		}
		__xgeXuiClickPressCancel(pContext);
	}
	if ( (iResult == XGE_XUI_EVENT_CONTINUE) && (xgeXuiHasCapture(pContext) == 0) && __xgeXuiFocusInvokeKeyAction(pContext, pEvent) ) {
		iResult = XGE_XUI_EVENT_CONSUMED;
	}
	return iResult;
}

int xgeXuiDispatchProcFrameEventAll(const xge_event_t* pEvent)
{
	xge_xui_context pContext;
	xge_xui_context pNext;
	int iResult;
	int iMergedResult;

	iMergedResult = XGE_XUI_EVENT_CONTINUE;
	for ( pContext = g_pXgeXuiProcFrameDispatchHead; pContext != NULL; pContext = pNext ) {
		pNext = __xgeXuiProcFrameDispatchNext(pContext);
		if ( (pContext->bInitialized == 0) || (pContext->bAutoDispatchProcFrameEvents == 0) ) {
			continue;
		}
		iResult = xgeXuiDispatchEvent(pContext, pEvent);
		iMergedResult = __xgeXuiEventResultMerge(iMergedResult, iResult);
		if ( iMergedResult == XGE_XUI_EVENT_CONSUMED ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	return iMergedResult;
}

int xgeXuiEventPush(xge_xui_context pContext, const xge_event_t* pEvent)
{
	xge_event_t* pTailEvent;
	int iTailPrev;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pEvent == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_MOVE) || (pEvent->iType == XGE_EVENT_TOUCH_MOVE)) && (pContext->iEventCount > 0) ) {
		iTailPrev = (pContext->iEventTail + XGE_XUI_EVENT_QUEUE_CAPACITY - 1) % XGE_XUI_EVENT_QUEUE_CAPACITY;
		pTailEvent = &pContext->arrEventQueue[iTailPrev];
		if ( (pTailEvent->iType == pEvent->iType) && (__xgeXuiEventPointerId(pTailEvent) == __xgeXuiEventPointerId(pEvent)) && (pTailEvent->iParam1 == pEvent->iParam1) && (pTailEvent->iParam2 == pEvent->iParam2) ) {
			pTailEvent->fX = pEvent->fX;
			pTailEvent->fY = pEvent->fY;
			pTailEvent->fDX += pEvent->fDX;
			pTailEvent->fDY += pEvent->fDY;
			pTailEvent->iPointerId = __xgeXuiEventPointerId(pEvent);
			pTailEvent->iCodepoint = pEvent->iCodepoint;
			pTailEvent->pData = pEvent->pData;
			return XGE_OK;
		}
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
	xge_rect_t tOverlayRect;
	const xge_xui_host_t* pOldHost;
	float fOldDipScale;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pContext->pRoot == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( fDelta < 0.0f ) {
		fDelta = 0.0f;
	}
	if ( pContext->bContextPressActive != 0 && pContext->bContextPressMoved == 0 ) {
		pContext->fContextPressTime += fDelta;
		if ( pContext->bContextPressFired == 0 && pContext->fContextPressTime >= 0.55f ) {
			int iBeginResult;
			int iMenuResult;
			uint32_t iContextMask;

			iContextMask = 0;
			if ( pContext->pRoot != NULL ) {
				iContextMask |= pContext->pRoot->iSubtreeEventMask;
			}
			if ( pContext->pOverlayRoot != NULL ) {
				iContextMask |= pContext->pOverlayRoot->iSubtreeEventMask;
			}
			iBeginResult = __xgeXuiDispatchContextEvent(pContext, pContext->pContextPressTarget, XGE_EVENT_XUI_CONTEXT_BEGIN, pContext->fContextPressLastX, pContext->fContextPressLastY, pContext->iContextPressPointerId);
			iMenuResult = XGE_XUI_EVENT_CONTINUE;
			if ( (iContextMask & XGE_XUI_EVENT_MASK_CONTEXT_MENU) != 0 ) {
				iMenuResult = __xgeXuiDispatchContextEvent(pContext, pContext->pContextPressTarget, XGE_EVENT_XUI_CONTEXT_MENU, pContext->fContextPressLastX, pContext->fContextPressLastY, pContext->iContextPressPointerId);
			}
			if ( (iBeginResult != XGE_XUI_EVENT_CONTINUE) || (iMenuResult != XGE_XUI_EVENT_CONTINUE) || ((iContextMask & XGE_XUI_EVENT_MASK_CONTEXT_MENU) != 0) ) {
				pContext->bContextPressFired = 1;
			}
		}
	}
	tRootRect = (xge_rect_t){ 0.0f, 0.0f, (float)xgeGetWidth(), (float)xgeGetHeight() };
	if ( tRootRect.fW <= 0.0f ) {
		tRootRect.fW = pContext->pRoot->tRect.fW;
	}
	if ( tRootRect.fH <= 0.0f ) {
		tRootRect.fH = pContext->pRoot->tRect.fH;
	}
	if ( __xgeXuiRectSame(pContext->pRoot->tRect, tRootRect) == 0 ) {
		pContext->pRoot->tRect = tRootRect;
		pContext->pRoot->tLocalRect = tRootRect;
		__xgeXuiWidgetBoxUpdate(pContext->pRoot, tRootRect);
		pContext->pRoot->iFlags |= XGE_XUI_WIDGET_DIRTY_LAYOUT;
	}
	pOldHost = g_xgeXuiActiveHost;
	fOldDipScale = g_fXgeXuiActiveDipScale;
	g_xgeXuiActiveHost = xgeXuiGetHost(pContext);
	g_fXgeXuiActiveDipScale = xgeXuiGetDipScale(pContext);
	__xgeXuiUpdateWidget(pContext->pRoot, fDelta);
	__xgeXuiUpdateWidget(pContext->pOverlayRoot, fDelta);
	__xgeXuiLayoutWidget(pContext->pRoot, tRootRect);
	if ( pContext->pOverlayRoot != NULL ) {
		tOverlayRect = tRootRect;
		if ( __xgeXuiRectSame(pContext->pOverlayRoot->tRect, tOverlayRect) == 0 ) {
			pContext->pOverlayRoot->tRect = tOverlayRect;
			pContext->pOverlayRoot->tLocalRect = tOverlayRect;
			__xgeXuiWidgetBoxUpdate(pContext->pOverlayRoot, tOverlayRect);
			pContext->pOverlayRoot->iFlags |= XGE_XUI_WIDGET_DIRTY_LAYOUT;
		}
		__xgeXuiLayoutWidget(pContext->pOverlayRoot, tOverlayRect);
		xgeXuiTooltipUpdate(pContext, fDelta);
	}
	g_fXgeXuiActiveDipScale = fOldDipScale;
	g_xgeXuiActiveHost = pOldHost;
#if XGE_HAS_DEBUGMODE
	pContext->iDirtyLayoutCount = 0;
#endif
	return XGE_OK;
}

int xgeXuiPaint(xge_xui_context pContext)
{
	const xge_xui_host_t* pOldHost;
	xge_xui_context pOldContext;
	int bExternalDirty;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (pContext->pRoot == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	bExternalDirty = (__xgeSurfaceDirtyGenerationGet() != pContext->iPaintSurfaceDirtyGeneration);
	if ( (bExternalDirty == 0) && ((pContext->pRoot->iFlags & XGE_XUI_WIDGET_DIRTY_PAINT) == 0) && ((pContext->pOverlayRoot == NULL) || ((pContext->pOverlayRoot->iFlags & XGE_XUI_WIDGET_DIRTY_PAINT) == 0)) ) {
		pContext->iPaintCommandCount = 0;
		pContext->iPaintFlushCount = 0;
#if XGE_HAS_DEBUGMODE
		pContext->iDirtyPaintCount = 0;
#endif
		pContext->bRefreshRequested = 0;
		return 0;
	}
	pOldHost = g_xgeXuiActiveHost;
	pOldContext = g_xgeXuiActiveContext;
	g_xgeXuiActiveHost = xgeXuiGetHost(pContext);
	g_xgeXuiActiveContext = pContext;
	__xgeXuiPaintClipBegin(pContext);
	pContext->iPaintFlushCount = 0;
	pContext->iPaintCommandCount = __xgeXuiPaintWidget(pContext->pRoot);
	(void)xgeFlush();
	pContext->iPaintCommandCount += __xgeXuiPaintWidget(pContext->pOverlayRoot);
	(void)xgeFlush();
	pContext->iPaintCommandCount += __xgeXuiPaintWidgetAfterAll(pContext->pRoot);
	pContext->iPaintCommandCount += __xgeXuiPaintWidgetAfterAll(pContext->pOverlayRoot);
	__xgeXuiPaintClipEnd(pContext);
	if ( pContext->iPaintFlushCount < pContext->iPaintCommandCount ) {
		pContext->iPaintFlushCount = pContext->iPaintCommandCount;
	}
	g_xgeXuiActiveHost = pOldHost;
	g_xgeXuiActiveContext = pOldContext;
#if XGE_HAS_DEBUGMODE
	pContext->iDirtyPaintCount = 0;
#endif
	pContext->bRefreshRequested = 0;
	pContext->iPaintSurfaceDirtyGeneration = __xgeSurfaceDirtyGenerationGet();
	return pContext->iPaintCommandCount;
}
