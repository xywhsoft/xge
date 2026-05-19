static void __xgeXuiStyleInit(xge_xui_style_t* pStyle)
{
	if ( pStyle == NULL ) {
		return;
	}
	memset(pStyle, 0, sizeof(*pStyle));
	pStyle->iLayout = XGE_XUI_LAYOUT_ABSOLUTE;
	pStyle->iDock = XGE_XUI_DOCK_FILL;
	pStyle->tWidth = xgeXuiSizeContent();
	pStyle->tHeight = xgeXuiSizeContent();
	pStyle->tMinWidth = xgeXuiSizePx(0.0f);
	pStyle->tMinHeight = xgeXuiSizePx(0.0f);
	pStyle->tMaxWidth = xgeXuiSizePx(0.0f);
	pStyle->tMaxHeight = xgeXuiSizePx(0.0f);
	pStyle->tAnchor.tLeft = xgeXuiSizePx(0.0f);
	pStyle->tAnchor.tTop = xgeXuiSizePx(0.0f);
	pStyle->tAnchor.tRight = xgeXuiSizePx(0.0f);
	pStyle->tAnchor.tBottom = xgeXuiSizePx(0.0f);
	pStyle->iGridColumns = 1;
	pStyle->iGridColumnSpan = 1;
	pStyle->fGridRowHeight = 0.0f;
	pStyle->fGridColumnGap = 0.0f;
	pStyle->fGridRowGap = 0.0f;
	pStyle->fGap = 0.0f;
	pStyle->iAlignX = XGE_XUI_ALIGN_STRETCH;
	pStyle->iAlignY = XGE_XUI_ALIGN_STRETCH;
	pStyle->iJustify = XGE_XUI_JUSTIFY_START;
	pStyle->iLayer = XGE_XUI_LAYER_NORMAL;
	pStyle->iOverflow = XGE_XUI_OVERFLOW_VISIBLE;
	pStyle->iBackgroundColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	pStyle->iBorderColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	pStyle->iFocusRingColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	pStyle->iDisabledOverlayColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	pStyle->iDebugOutlineColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	pStyle->fBorderWidth = 0.0f;
	pStyle->fFocusRingWidth = 0.0f;
	pStyle->fDebugOutlineWidth = 0.0f;
}

void xgeXuiStyleDefault(xge_xui_style_t* pStyle)
{
	__xgeXuiStyleInit(pStyle);
}

void xgeXuiStyleFromTheme(xge_xui_style_t* pStyle, const xge_xui_theme_t* pTheme)
{
	xge_xui_theme_t tDefaultTheme;

	if ( pStyle == NULL ) {
		return;
	}
	if ( pTheme == NULL ) {
		xgeXuiThemeDefault(&tDefaultTheme);
		pTheme = &tDefaultTheme;
	}
	__xgeXuiStyleInit(pStyle);
	pStyle->iBackgroundColor = pTheme->iPanelColor;
	pStyle->fRadius = (pTheme->fRadius > 0.0f) ? pTheme->fRadius : 0.0f;
	pStyle->tPadding.tLeft = xgeXuiSizePx(pTheme->fPadding);
	pStyle->tPadding.tTop = xgeXuiSizePx(pTheme->fPadding);
	pStyle->tPadding.tRight = xgeXuiSizePx(pTheme->fPadding);
	pStyle->tPadding.tBottom = xgeXuiSizePx(pTheme->fPadding);
	pStyle->fGridColumnGap = (pTheme->fSpacing > 0.0f) ? pTheme->fSpacing : 0.0f;
	pStyle->fGridRowGap = (pTheme->fSpacing > 0.0f) ? pTheme->fSpacing : 0.0f;
	pStyle->fGap = (pTheme->fSpacing > 0.0f) ? pTheme->fSpacing : 0.0f;
}

static xge_vec2_t __xgeXuiMeasureWidget(xge_xui_widget pWidget);

static int __xgeXuiAlignClamp(int iAlign)
{
	if ( (iAlign < XGE_XUI_ALIGN_START) || (iAlign > XGE_XUI_ALIGN_STRETCH) ) {
		return XGE_XUI_ALIGN_STRETCH;
	}
	return iAlign;
}

static float __xgeXuiAlignOffset(int iAlign, float fSlot, float fSize)
{
	float fOffset;

	fOffset = fSlot - fSize;
	if ( fOffset < 0.0f ) {
		fOffset = 0.0f;
	}
	if ( iAlign == XGE_XUI_ALIGN_CENTER ) {
		return fOffset * 0.5f;
	}
	if ( iAlign == XGE_XUI_ALIGN_END ) {
		return fOffset;
	}
	return 0.0f;
}

static int __xgeXuiJustifyClamp(int iJustify)
{
	if ( (iJustify < XGE_XUI_JUSTIFY_START) || (iJustify > XGE_XUI_JUSTIFY_SPACE_BETWEEN) ) {
		return XGE_XUI_JUSTIFY_START;
	}
	return iJustify;
}

static float __xgeXuiGap(float fGap)
{
	return (fGap > 0.0f) ? fGap : 0.0f;
}

static int __xgeXuiLayoutVisible(xge_xui_widget pWidget)
{
	return (pWidget != NULL) && ((pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0);
}

static int __xgeXuiRectSame(xge_rect_t tA, xge_rect_t tB)
{
	return (tA.fX == tB.fX) && (tA.fY == tB.fY) && (tA.fW == tB.fW) && (tA.fH == tB.fH);
}

static float __xgeXuiSizeResolve(xge_xui_size_t tSize, float fParent, float fContent, float fFallback)
{
	switch ( tSize.iUnit ) {
		case XGE_XUI_SIZE_PX:
			return tSize.fValue;

		case XGE_XUI_SIZE_DIP:
			return tSize.fValue * g_fXgeXuiActiveDipScale;

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

static int __xgeXuiSizeCanStretch(xge_xui_size_t tSize)
{
	return (tSize.iUnit == XGE_XUI_SIZE_CONTENT) || (tSize.iUnit == XGE_XUI_SIZE_GROW);
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

static xge_vec2_t __xgeXuiMeasureChildren(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_vec2_t tSize;
	xge_vec2_t tChildSize;
	xge_rect_t tParent;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
	float fGap;
	int iCount;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( pWidget == NULL ) {
		return tSize;
	}
	tParent = pWidget->tContentRect;
	fGap = __xgeXuiGap(pWidget->tStyle.fGap);
	iCount = 0;
	switch ( pWidget->tStyle.iLayout ) {
		case XGE_XUI_LAYOUT_ROW:
			for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
				if ( !__xgeXuiLayoutVisible(pChild) ) {
					continue;
				}
				tChildSize = __xgeXuiMeasureWidget(pChild);
				__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
				if ( iCount > 0 ) {
					tSize.fX += fGap;
				}
				tSize.fX += tChildSize.fX + fLeft + fRight;
				if ( tSize.fY < (tChildSize.fY + fTop + fBottom) ) {
					tSize.fY = tChildSize.fY + fTop + fBottom;
				}
				iCount++;
			}
			break;

		case XGE_XUI_LAYOUT_COLUMN:
			for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
				if ( !__xgeXuiLayoutVisible(pChild) ) {
					continue;
				}
				tChildSize = __xgeXuiMeasureWidget(pChild);
				__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
				if ( iCount > 0 ) {
					tSize.fY += fGap;
				}
				tSize.fY += tChildSize.fY + fTop + fBottom;
				if ( tSize.fX < (tChildSize.fX + fLeft + fRight) ) {
					tSize.fX = tChildSize.fX + fLeft + fRight;
				}
				iCount++;
			}
			break;

		case XGE_XUI_LAYOUT_DOCK:
			for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
				if ( !__xgeXuiLayoutVisible(pChild) ) {
					continue;
				}
				tChildSize = __xgeXuiMeasureWidget(pChild);
				__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
				if ( iCount > 0 ) {
					if ( (pChild->tStyle.iDock == XGE_XUI_DOCK_TOP) || (pChild->tStyle.iDock == XGE_XUI_DOCK_BOTTOM) ) {
						tSize.fY += fGap;
					} else if ( (pChild->tStyle.iDock == XGE_XUI_DOCK_LEFT) || (pChild->tStyle.iDock == XGE_XUI_DOCK_RIGHT) ) {
						tSize.fX += fGap;
					}
				}
				if ( (pChild->tStyle.iDock == XGE_XUI_DOCK_TOP) || (pChild->tStyle.iDock == XGE_XUI_DOCK_BOTTOM) ) {
					tSize.fY += tChildSize.fY + fTop + fBottom;
					if ( tSize.fX < (tChildSize.fX + fLeft + fRight) ) {
						tSize.fX = tChildSize.fX + fLeft + fRight;
					}
				} else if ( (pChild->tStyle.iDock == XGE_XUI_DOCK_LEFT) || (pChild->tStyle.iDock == XGE_XUI_DOCK_RIGHT) ) {
					tSize.fX += tChildSize.fX + fLeft + fRight;
					if ( tSize.fY < (tChildSize.fY + fTop + fBottom) ) {
						tSize.fY = tChildSize.fY + fTop + fBottom;
					}
				} else {
					if ( tSize.fX < (tChildSize.fX + fLeft + fRight) ) {
						tSize.fX = tChildSize.fX + fLeft + fRight;
					}
					if ( tSize.fY < (tChildSize.fY + fTop + fBottom) ) {
						tSize.fY = tChildSize.fY + fTop + fBottom;
					}
				}
				iCount++;
			}
			break;

		case XGE_XUI_LAYOUT_STACK:
		case XGE_XUI_LAYOUT_GRID:
		case XGE_XUI_LAYOUT_ABSOLUTE:
		default:
			for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
				if ( !__xgeXuiLayoutVisible(pChild) ) {
					continue;
				}
				tChildSize = __xgeXuiMeasureWidget(pChild);
				__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
				if ( tSize.fX < (pChild->tLocalRect.fX + tChildSize.fX + fLeft + fRight) ) {
					tSize.fX = pChild->tLocalRect.fX + tChildSize.fX + fLeft + fRight;
				}
				if ( tSize.fY < (pChild->tLocalRect.fY + tChildSize.fY + fTop + fBottom) ) {
					tSize.fY = pChild->tLocalRect.fY + tChildSize.fY + fTop + fBottom;
				}
			}
			break;
	}
	return tSize;
}

static xge_vec2_t __xgeXuiWidgetMeasureContent(xge_xui_widget pWidget)
{
	xge_vec2_t tSize;
	xge_vec2_t tChildren;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( pWidget == NULL ) {
		return tSize;
	}
	if ( pWidget->procMeasure != NULL ) {
		tSize = pWidget->procMeasure(pWidget, ((pWidget->iCallbackFlags & XGE_XUI_WIDGET_CALLBACK_MEASURE) != 0) ? pWidget->pMeasureUser : pWidget->pUser);
	} else {
		tSize = __xgeXuiMeasureChildren(pWidget);
	}
	tChildren = __xgeXuiMeasureChildren(pWidget);
	if ( tSize.fX < tChildren.fX ) {
		tSize.fX = tChildren.fX;
	}
	if ( tSize.fY < tChildren.fY ) {
		tSize.fY = tChildren.fY;
	}
	__xgeXuiEdgesResolve(&pWidget->tStyle.tPadding, pWidget->tRect, &fLeft, &fTop, &fRight, &fBottom);
	tSize.fX += fLeft + fRight;
	tSize.fY += fTop + fBottom;
	if ( pWidget->tStyle.fBorderWidth > 0.0f ) {
		tSize.fX += pWidget->tStyle.fBorderWidth * 2.0f;
		tSize.fY += pWidget->tStyle.fBorderWidth * 2.0f;
	}
	if ( tSize.fX < 0.0f ) {
		tSize.fX = 0.0f;
	}
	if ( tSize.fY < 0.0f ) {
		tSize.fY = 0.0f;
	}
	return tSize;
}

static xge_vec2_t __xgeXuiMeasureWidget(xge_xui_widget pWidget)
{
	xge_vec2_t tSize;

	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	if ( pWidget == NULL ) {
		return tSize;
	}
	if ( !__xgeXuiLayoutVisible(pWidget) ) {
		pWidget->tDesiredSize = tSize;
		return tSize;
	}
	if ( (pWidget->iFlags & XGE_XUI_WIDGET_DIRTY_LAYOUT) == 0 ) {
		return pWidget->tDesiredSize;
	}
	tSize = __xgeXuiWidgetMeasureContent(pWidget);
	tSize.fX = __xgeXuiSizeResolve(pWidget->tStyle.tWidth, pWidget->tRect.fW, tSize.fX, tSize.fX);
	tSize.fY = __xgeXuiSizeResolve(pWidget->tStyle.tHeight, pWidget->tRect.fH, tSize.fY, tSize.fY);
	tSize.fX = __xgeXuiSizeClamp(tSize.fX, pWidget->tStyle.tMinWidth, pWidget->tStyle.tMaxWidth, pWidget->tRect.fW);
	tSize.fY = __xgeXuiSizeClamp(tSize.fY, pWidget->tStyle.tMinHeight, pWidget->tStyle.tMaxHeight, pWidget->tRect.fH);
	if ( tSize.fX < 0.0f ) {
		tSize.fX = 0.0f;
	}
	if ( tSize.fY < 0.0f ) {
		tSize.fY = 0.0f;
	}
	pWidget->tDesiredSize = tSize;
	return tSize;
}

static xge_rect_t __xgeXuiRectInset(xge_rect_t tRect, float fLeft, float fTop, float fRight, float fBottom)
{
	tRect.fX += fLeft;
	tRect.fY += fTop;
	tRect.fW -= fLeft + fRight;
	tRect.fH -= fTop + fBottom;
	if ( tRect.fW < 0.0f ) {
		tRect.fW = 0.0f;
	}
	if ( tRect.fH < 0.0f ) {
		tRect.fH = 0.0f;
	}
	return tRect;
}

static xge_rect_t __xgeXuiRectOutset(xge_rect_t tRect, float fLeft, float fTop, float fRight, float fBottom)
{
	tRect.fX -= fLeft;
	tRect.fY -= fTop;
	tRect.fW += fLeft + fRight;
	tRect.fH += fTop + fBottom;
	if ( tRect.fW < 0.0f ) {
		tRect.fW = 0.0f;
	}
	if ( tRect.fH < 0.0f ) {
		tRect.fH = 0.0f;
	}
	return tRect;
}

static xge_rect_t __xgeXuiWidgetBoxParentRect(xge_xui_widget pWidget, xge_rect_t tFallback)
{
	if ( (pWidget != NULL) && (pWidget->pParent != NULL) ) {
		if ( (pWidget->pParent->tContentRect.fW > 0.0f) || (pWidget->pParent->tContentRect.fH > 0.0f) ) {
			return pWidget->pParent->tContentRect;
		}
		if ( (pWidget->pParent->tRect.fW > 0.0f) || (pWidget->pParent->tRect.fH > 0.0f) ) {
			return pWidget->pParent->tRect;
		}
	}
	return tFallback;
}

static void __xgeXuiWidgetBoxUpdate(xge_xui_widget pWidget, xge_rect_t tParent)
{
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;

	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tBorderRect = pWidget->tRect;
	if ( pWidget->tStyle.fBorderWidth > 0.0f ) {
		pWidget->tPaddingRect = __xgeXuiRectInset(pWidget->tBorderRect, pWidget->tStyle.fBorderWidth, pWidget->tStyle.fBorderWidth, pWidget->tStyle.fBorderWidth, pWidget->tStyle.fBorderWidth);
	} else {
		pWidget->tPaddingRect = pWidget->tBorderRect;
	}
	__xgeXuiEdgesResolve(&pWidget->tStyle.tPadding, pWidget->tPaddingRect, &fLeft, &fTop, &fRight, &fBottom);
	pWidget->tContentRect = __xgeXuiRectInset(pWidget->tPaddingRect, fLeft, fTop, fRight, fBottom);
	__xgeXuiEdgesResolve(&pWidget->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
	pWidget->tOuterRect = __xgeXuiRectOutset(pWidget->tBorderRect, fLeft, fTop, fRight, fBottom);
}

static xge_rect_t __xgeXuiChildSizeResolve(xge_xui_widget pChild, xge_rect_t tParent, float fFallbackW, float fFallbackH)
{
	xge_rect_t tRect;
	xge_vec2_t tMeasure;
	int iAlignX;
	int iAlignY;

	tRect = pChild->tRect;
	tMeasure = __xgeXuiMeasureWidget(pChild);
	iAlignX = __xgeXuiAlignClamp(pChild->tStyle.iAlignX);
	iAlignY = __xgeXuiAlignClamp(pChild->tStyle.iAlignY);
	tRect.fW = __xgeXuiSizeResolve(pChild->tStyle.tWidth, tParent.fW, tMeasure.fX, fFallbackW);
	tRect.fH = __xgeXuiSizeResolve(pChild->tStyle.tHeight, tParent.fH, tMeasure.fY, fFallbackH);
	if ( (pChild->procMeasure == NULL) && (pChild->tStyle.tWidth.iUnit == XGE_XUI_SIZE_CONTENT) && (pChild->tLocalRect.fW > 0.0f) ) {
		tRect.fW = fFallbackW;
	}
	if ( (pChild->procMeasure == NULL) && (pChild->tStyle.tHeight.iUnit == XGE_XUI_SIZE_CONTENT) && (pChild->tLocalRect.fH > 0.0f) ) {
		tRect.fH = fFallbackH;
	}
	tRect.fW = __xgeXuiSizeClamp(tRect.fW, pChild->tStyle.tMinWidth, pChild->tStyle.tMaxWidth, tParent.fW);
	tRect.fH = __xgeXuiSizeClamp(tRect.fH, pChild->tStyle.tMinHeight, pChild->tStyle.tMaxHeight, tParent.fH);
	if ( (iAlignX == XGE_XUI_ALIGN_STRETCH) && (pChild->tStyle.tWidth.iUnit == XGE_XUI_SIZE_GROW) ) {
		tRect.fW = fFallbackW;
	}
	if ( (iAlignY == XGE_XUI_ALIGN_STRETCH) && (pChild->tStyle.tHeight.iUnit == XGE_XUI_SIZE_GROW) ) {
		tRect.fH = fFallbackH;
	}
	return tRect;
}

static void __xgeXuiChildSizeClampResolved(xge_xui_widget pChild, xge_rect_t tParent, xge_rect_t* pRect)
{
	if ( (pChild == NULL) || (pRect == NULL) ) {
		return;
	}
	pRect->fW = __xgeXuiSizeClamp(pRect->fW, pChild->tStyle.tMinWidth, pChild->tStyle.tMaxWidth, tParent.fW);
	pRect->fH = __xgeXuiSizeClamp(pRect->fH, pChild->tStyle.tMinHeight, pChild->tStyle.tMaxHeight, tParent.fH);
}

static float __xgeXuiGrowWeight(xge_xui_size_t tSize)
{
	return (tSize.fValue > 0.0f) ? tSize.fValue : 1.0f;
}

static float __xgeXuiGrowClampWidth(xge_xui_widget pChild, float fValue, float fParent)
{
	return __xgeXuiSizeClamp(fValue, pChild->tStyle.tMinWidth, pChild->tStyle.tMaxWidth, fParent);
}

static float __xgeXuiGrowClampHeight(xge_xui_widget pChild, float fValue, float fParent)
{
	return __xgeXuiSizeClamp(fValue, pChild->tStyle.tMinHeight, pChild->tStyle.tMaxHeight, fParent);
}

static float __xgeXuiGrowWidth(xge_xui_widget pWidget, xge_xui_widget pTarget, float fRemaining, float fGrow)
{
	xge_xui_widget pChild;
	xge_xui_widget arrChild[256];
	float arrWeight[256];
	float arrSize[256];
	int arrFrozen[256];
	float fActiveRemaining;
	float fActiveGrow;
	float fWeight;
	float fSize;
	float fClamped;
	int iCount;
	int i;
	int iChanged;

	if ( (pTarget == NULL) || (fRemaining <= 0.0f) || (fGrow <= 0.0f) ) {
		return 0.0f;
	}
	iCount = 0;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( !__xgeXuiLayoutVisible(pChild) ) {
			continue;
		}
		if ( pChild->tStyle.tWidth.iUnit != XGE_XUI_SIZE_GROW ) {
			continue;
		}
		if ( iCount >= (int)(sizeof(arrChild) / sizeof(arrChild[0])) ) {
			fWeight = __xgeXuiGrowWeight(pTarget->tStyle.tWidth);
			fSize = (fGrow > 0.0f) ? (fRemaining * fWeight / fGrow) : 0.0f;
			return __xgeXuiGrowClampWidth(pTarget, fSize, pWidget->tContentRect.fW);
		}
		arrChild[iCount] = pChild;
		arrWeight[iCount] = __xgeXuiGrowWeight(pChild->tStyle.tWidth);
		arrSize[iCount] = 0.0f;
		arrFrozen[iCount] = 0;
		iCount++;
	}
	fActiveRemaining = fRemaining;
	fActiveGrow = fGrow;
	do {
		iChanged = 0;
		for ( i = 0; i < iCount; i++ ) {
			if ( arrFrozen[i] ) {
				continue;
			}
			pChild = arrChild[i];
			fWeight = arrWeight[i];
			fSize = (fActiveGrow > 0.0f) ? (fActiveRemaining * fWeight / fActiveGrow) : 0.0f;
			fClamped = __xgeXuiGrowClampWidth(pChild, fSize, pWidget->tContentRect.fW);
			if ( fClamped != fSize ) {
				arrSize[i] = fClamped;
				arrFrozen[i] = 1;
				fActiveRemaining -= fClamped;
				if ( fActiveRemaining < 0.0f ) {
					fActiveRemaining = 0.0f;
				}
				fActiveGrow -= fWeight;
				if ( fActiveGrow < 0.0f ) {
					fActiveGrow = 0.0f;
				}
				iChanged = 1;
			}
		}
	} while ( iChanged && (fActiveGrow > 0.0f) );
	for ( i = 0; i < iCount; i++ ) {
		if ( arrFrozen[i] == 0 ) {
			arrSize[i] = (fActiveGrow > 0.0f) ? (fActiveRemaining * arrWeight[i] / fActiveGrow) : 0.0f;
			arrSize[i] = __xgeXuiGrowClampWidth(arrChild[i], arrSize[i], pWidget->tContentRect.fW);
		}
		if ( arrChild[i] == pTarget ) {
			return arrSize[i];
		}
	}
	return 0.0f;
}

static float __xgeXuiGrowHeight(xge_xui_widget pWidget, xge_xui_widget pTarget, float fRemaining, float fGrow)
{
	xge_xui_widget pChild;
	xge_xui_widget arrChild[256];
	float arrWeight[256];
	float arrSize[256];
	int arrFrozen[256];
	float fActiveRemaining;
	float fActiveGrow;
	float fWeight;
	float fSize;
	float fClamped;
	int iCount;
	int i;
	int iChanged;

	if ( (pTarget == NULL) || (fRemaining <= 0.0f) || (fGrow <= 0.0f) ) {
		return 0.0f;
	}
	iCount = 0;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( !__xgeXuiLayoutVisible(pChild) ) {
			continue;
		}
		if ( pChild->tStyle.tHeight.iUnit != XGE_XUI_SIZE_GROW ) {
			continue;
		}
		if ( iCount >= (int)(sizeof(arrChild) / sizeof(arrChild[0])) ) {
			fWeight = __xgeXuiGrowWeight(pTarget->tStyle.tHeight);
			fSize = (fGrow > 0.0f) ? (fRemaining * fWeight / fGrow) : 0.0f;
			return __xgeXuiGrowClampHeight(pTarget, fSize, pWidget->tContentRect.fH);
		}
		arrChild[iCount] = pChild;
		arrWeight[iCount] = __xgeXuiGrowWeight(pChild->tStyle.tHeight);
		arrSize[iCount] = 0.0f;
		arrFrozen[iCount] = 0;
		iCount++;
	}
	fActiveRemaining = fRemaining;
	fActiveGrow = fGrow;
	do {
		iChanged = 0;
		for ( i = 0; i < iCount; i++ ) {
			if ( arrFrozen[i] ) {
				continue;
			}
			pChild = arrChild[i];
			fWeight = arrWeight[i];
			fSize = (fActiveGrow > 0.0f) ? (fActiveRemaining * fWeight / fActiveGrow) : 0.0f;
			fClamped = __xgeXuiGrowClampHeight(pChild, fSize, pWidget->tContentRect.fH);
			if ( fClamped != fSize ) {
				arrSize[i] = fClamped;
				arrFrozen[i] = 1;
				fActiveRemaining -= fClamped;
				if ( fActiveRemaining < 0.0f ) {
					fActiveRemaining = 0.0f;
				}
				fActiveGrow -= fWeight;
				if ( fActiveGrow < 0.0f ) {
					fActiveGrow = 0.0f;
				}
				iChanged = 1;
			}
		}
	} while ( iChanged && (fActiveGrow > 0.0f) );
	for ( i = 0; i < iCount; i++ ) {
		if ( arrFrozen[i] == 0 ) {
			arrSize[i] = (fActiveGrow > 0.0f) ? (fActiveRemaining * arrWeight[i] / fActiveGrow) : 0.0f;
			arrSize[i] = __xgeXuiGrowClampHeight(arrChild[i], arrSize[i], pWidget->tContentRect.fH);
		}
		if ( arrChild[i] == pTarget ) {
			return arrSize[i];
		}
	}
	return 0.0f;
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
	pWidget->iRole = XGE_XUI_WIDGET_ROLE_CONTAINER;
	pWidget->iFlags = XGE_XUI_WIDGET_VISIBLE | XGE_XUI_WIDGET_ENABLED | XGE_XUI_WIDGET_HIT_TEST_VISIBLE | XGE_XUI_WIDGET_DIRTY_LAYOUT | XGE_XUI_WIDGET_DIRTY_PAINT;
	return pWidget;
}

static xge_xui_context __xgeXuiWidgetContext(xge_xui_widget pWidget)
{
	while ( pWidget != NULL ) {
		if ( pWidget->pParent == NULL ) {
			break;
		}
		pWidget = pWidget->pParent;
	}
	return (pWidget != NULL) ? (xge_xui_context)pWidget->pInternal : NULL;
}

static void __xgeXuiWidgetInvalidateRect(xge_xui_widget pWidget)
{
	xge_rect_t tRect;

	if ( pWidget == NULL ) {
		return;
	}
	tRect = pWidget->tRect;
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		tRect = pWidget->tContentRect;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	xgeInvalidateRect(tRect);
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

static void __xgeXuiWidgetArrangeRect(xge_xui_widget pWidget, xge_rect_t tRect)
{
	xge_rect_t tParent;

	if ( pWidget == NULL ) {
		return;
	}
	tParent = __xgeXuiWidgetBoxParentRect(pWidget, tRect);
	if ( __xgeXuiRectSame(pWidget->tRect, tRect) == 0 ) {
		pWidget->iFlags |= XGE_XUI_WIDGET_DIRTY_LAYOUT | XGE_XUI_WIDGET_DIRTY_PAINT;
		xgeXuiWidgetMarkPaint(pWidget);
		pWidget->tRect = tRect;
		__xgeXuiWidgetBoxUpdate(pWidget, tParent);
		__xgeXuiWidgetInvalidateRect(pWidget);
		return;
	}
	pWidget->tRect = tRect;
	__xgeXuiWidgetBoxUpdate(pWidget, tParent);
}

static void __xgeXuiLayoutAbsolute(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_rect_t tParent;
	xge_rect_t tLocal;
	xge_rect_t tChild;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
	float fAnchorLeft;
	float fAnchorTop;
	float fAnchorRight;
	float fAnchorBottom;
	xge_rect_t tSlot;
	int bLeft;
	int bTop;
	int bRight;
	int bBottom;

	if ( pWidget == NULL ) {
		return;
	}
	tParent = pWidget->tContentRect;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( !__xgeXuiLayoutVisible(pChild) ) {
			continue;
		}
		tLocal = pChild->tLocalRect;
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		__xgeXuiEdgesResolve(&pChild->tStyle.tAnchor, tParent, &fAnchorLeft, &fAnchorTop, &fAnchorRight, &fAnchorBottom);
		bLeft = ((pChild->tStyle.iAnchor & XGE_XUI_ANCHOR_LEFT) != 0);
		bTop = ((pChild->tStyle.iAnchor & XGE_XUI_ANCHOR_TOP) != 0);
		bRight = ((pChild->tStyle.iAnchor & XGE_XUI_ANCHOR_RIGHT) != 0);
		bBottom = ((pChild->tStyle.iAnchor & XGE_XUI_ANCHOR_BOTTOM) != 0);
		tSlot.fW = tLocal.fW - fLeft - fRight;
		tSlot.fH = tLocal.fH - fTop - fBottom;
		if ( bLeft && bRight ) {
			tSlot.fX = tParent.fX + fAnchorLeft + fLeft;
			tSlot.fW = tParent.fW - fAnchorLeft - fAnchorRight - fLeft - fRight;
		} else if ( bLeft ) {
			tSlot.fX = tParent.fX + fAnchorLeft + fLeft;
		} else if ( bRight ) {
			tSlot.fX = tParent.fX + tParent.fW - fAnchorRight - tLocal.fW + fLeft;
		} else {
			tSlot.fX = tParent.fX + tLocal.fX + fLeft;
		}
		if ( bTop && bBottom ) {
			tSlot.fY = tParent.fY + fAnchorTop + fTop;
			tSlot.fH = tParent.fH - fAnchorTop - fAnchorBottom - fTop - fBottom;
		} else if ( bTop ) {
			tSlot.fY = tParent.fY + fAnchorTop + fTop;
		} else if ( bBottom ) {
			tSlot.fY = tParent.fY + tParent.fH - fAnchorBottom - tLocal.fH + fTop;
		} else {
			tSlot.fY = tParent.fY + tLocal.fY + fTop;
		}
		if ( tSlot.fW < 0.0f ) {
			tSlot.fW = 0.0f;
		}
		if ( tSlot.fH < 0.0f ) {
			tSlot.fH = 0.0f;
		}
		tChild = __xgeXuiChildSizeResolve(pChild, tSlot, tSlot.fW, tSlot.fH);
		if ( bLeft && bRight ) {
			tChild.fW = tSlot.fW;
		}
		if ( bTop && bBottom ) {
			tChild.fH = tSlot.fH;
		}
		if ( bRight && (bLeft == 0) ) {
			tSlot.fX = tParent.fX + tParent.fW - fAnchorRight - fRight - tChild.fW;
			tSlot.fW = tChild.fW;
		}
		if ( bBottom && (bTop == 0) ) {
			tSlot.fY = tParent.fY + tParent.fH - fAnchorBottom - fBottom - tChild.fH;
			tSlot.fH = tChild.fH;
		}
		tChild.fX = tSlot.fX + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignX), tSlot.fW, tChild.fW);
		tChild.fY = tSlot.fY + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignY), tSlot.fH, tChild.fH);
		__xgeXuiWidgetArrangeRect(pChild, tChild);
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
	float fSlotH;
	float fGap;
	float fBetween;
	int iCount;
	int iJustify;

	tParent = pWidget->tContentRect;
	fFixed = 0.0f;
	fGrow = 0.0f;
	fGap = __xgeXuiGap(pWidget->tStyle.fGap);
	iCount = 0;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( !__xgeXuiLayoutVisible(pChild) ) {
			continue;
		}
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		if ( pChild->tStyle.tWidth.iUnit == XGE_XUI_SIZE_GROW ) {
			fGrow += __xgeXuiGrowWeight(pChild->tStyle.tWidth);
			fFixed += fLeft + fRight;
		} else {
			tChild = __xgeXuiChildSizeResolve(pChild, tParent, pChild->tLocalRect.fW, tParent.fH - fTop - fBottom);
			fFixed += tChild.fW + fLeft + fRight;
		}
		iCount++;
	}
	if ( iCount > 1 ) {
		fFixed += fGap * (float)(iCount - 1);
	}
	fRemaining = tParent.fW - fFixed;
	if ( fRemaining < 0.0f ) {
		fRemaining = 0.0f;
	}
	fX = tParent.fX;
	fBetween = fGap;
	iJustify = __xgeXuiJustifyClamp(pWidget->tStyle.iJustify);
	if ( fGrow <= 0.0f ) {
		if ( iJustify == XGE_XUI_JUSTIFY_CENTER ) {
			fX += fRemaining * 0.5f;
		} else if ( iJustify == XGE_XUI_JUSTIFY_END ) {
			fX += fRemaining;
		} else if ( (iJustify == XGE_XUI_JUSTIFY_SPACE_BETWEEN) && (iCount > 1) ) {
			fBetween += fRemaining / (float)(iCount - 1);
		}
	}
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( !__xgeXuiLayoutVisible(pChild) ) {
			continue;
		}
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		tChild = __xgeXuiChildSizeResolve(pChild, tParent, pChild->tLocalRect.fW, tParent.fH - fTop - fBottom);
		if ( pChild->tStyle.tWidth.iUnit == XGE_XUI_SIZE_GROW ) {
			tChild.fW = __xgeXuiGrowWidth(pWidget, pChild, fRemaining, fGrow);
		}
		fSlotH = tParent.fH - fTop - fBottom;
		if ( fSlotH < 0.0f ) {
			fSlotH = 0.0f;
		}
		if ( (__xgeXuiAlignClamp(pChild->tStyle.iAlignY) == XGE_XUI_ALIGN_STRETCH) && __xgeXuiSizeCanStretch(pChild->tStyle.tHeight) ) {
			tChild.fH = fSlotH;
		}
		__xgeXuiChildSizeClampResolved(pChild, tParent, &tChild);
		tChild.fX = fX + fLeft;
		tChild.fY = tParent.fY + fTop + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignY), fSlotH, tChild.fH);
		__xgeXuiWidgetArrangeRect(pChild, tChild);
		fX += fLeft + tChild.fW + fRight + fBetween;
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
	float fSlotW;
	float fGap;
	float fBetween;
	int iCount;
	int iJustify;

	tParent = pWidget->tContentRect;
	fFixed = 0.0f;
	fGrow = 0.0f;
	fGap = __xgeXuiGap(pWidget->tStyle.fGap);
	iCount = 0;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( !__xgeXuiLayoutVisible(pChild) ) {
			continue;
		}
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		if ( pChild->tStyle.tHeight.iUnit == XGE_XUI_SIZE_GROW ) {
			fGrow += __xgeXuiGrowWeight(pChild->tStyle.tHeight);
			fFixed += fTop + fBottom;
		} else {
			tChild = __xgeXuiChildSizeResolve(pChild, tParent, tParent.fW - fLeft - fRight, pChild->tLocalRect.fH);
			fFixed += tChild.fH + fTop + fBottom;
		}
		iCount++;
	}
	if ( iCount > 1 ) {
		fFixed += fGap * (float)(iCount - 1);
	}
	fRemaining = tParent.fH - fFixed;
	if ( fRemaining < 0.0f ) {
		fRemaining = 0.0f;
	}
	fY = tParent.fY;
	fBetween = fGap;
	iJustify = __xgeXuiJustifyClamp(pWidget->tStyle.iJustify);
	if ( fGrow <= 0.0f ) {
		if ( iJustify == XGE_XUI_JUSTIFY_CENTER ) {
			fY += fRemaining * 0.5f;
		} else if ( iJustify == XGE_XUI_JUSTIFY_END ) {
			fY += fRemaining;
		} else if ( (iJustify == XGE_XUI_JUSTIFY_SPACE_BETWEEN) && (iCount > 1) ) {
			fBetween += fRemaining / (float)(iCount - 1);
		}
	}
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( !__xgeXuiLayoutVisible(pChild) ) {
			continue;
		}
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		tChild = __xgeXuiChildSizeResolve(pChild, tParent, tParent.fW - fLeft - fRight, pChild->tLocalRect.fH);
		if ( pChild->tStyle.tHeight.iUnit == XGE_XUI_SIZE_GROW ) {
			tChild.fH = __xgeXuiGrowHeight(pWidget, pChild, fRemaining, fGrow);
		}
		fSlotW = tParent.fW - fLeft - fRight;
		if ( fSlotW < 0.0f ) {
			fSlotW = 0.0f;
		}
		if ( (__xgeXuiAlignClamp(pChild->tStyle.iAlignX) == XGE_XUI_ALIGN_STRETCH) && __xgeXuiSizeCanStretch(pChild->tStyle.tWidth) ) {
			tChild.fW = fSlotW;
		}
		__xgeXuiChildSizeClampResolved(pChild, tParent, &tChild);
		tChild.fX = tParent.fX + fLeft + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignX), fSlotW, tChild.fW);
		tChild.fY = fY + fTop;
		__xgeXuiWidgetArrangeRect(pChild, tChild);
		fY += fTop + tChild.fH + fBottom + fBetween;
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
	float fSlotW;
	float fSlotH;

	tParent = pWidget->tContentRect;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( !__xgeXuiLayoutVisible(pChild) ) {
			continue;
		}
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		fSlotW = tParent.fW - fLeft - fRight;
		fSlotH = tParent.fH - fTop - fBottom;
		if ( fSlotW < 0.0f ) {
			fSlotW = 0.0f;
		}
		if ( fSlotH < 0.0f ) {
			fSlotH = 0.0f;
		}
		tChild = __xgeXuiChildSizeResolve(pChild, tParent, fSlotW, fSlotH);
		if ( (__xgeXuiAlignClamp(pChild->tStyle.iAlignX) == XGE_XUI_ALIGN_STRETCH) && __xgeXuiSizeCanStretch(pChild->tStyle.tWidth) ) {
			tChild.fW = fSlotW;
		}
		if ( (__xgeXuiAlignClamp(pChild->tStyle.iAlignY) == XGE_XUI_ALIGN_STRETCH) && __xgeXuiSizeCanStretch(pChild->tStyle.tHeight) ) {
			tChild.fH = fSlotH;
		}
		__xgeXuiChildSizeClampResolved(pChild, tParent, &tChild);
		tChild.fX = tParent.fX + fLeft + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignX), fSlotW, tChild.fW);
		tChild.fY = tParent.fY + fTop + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignY), fSlotH, tChild.fH);
		__xgeXuiWidgetArrangeRect(pChild, tChild);
	}
}

static void __xgeXuiRectClampPositive(xge_rect_t* pRect)
{
	if ( pRect == NULL ) {
		return;
	}
	if ( pRect->fW < 0.0f ) {
		pRect->fW = 0.0f;
	}
	if ( pRect->fH < 0.0f ) {
		pRect->fH = 0.0f;
	}
}

static void __xgeXuiLayoutDock(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_rect_t tRemain;
	xge_rect_t tSlot;
	xge_rect_t tChild;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
	float fSlotW;
	float fSlotH;
	float fGap;
	int iDock;

	if ( pWidget == NULL ) {
		return;
	}
	tRemain = pWidget->tContentRect;
	fGap = __xgeXuiGap(pWidget->tStyle.fGap);
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( !__xgeXuiLayoutVisible(pChild) ) {
			continue;
		}
		iDock = pChild->tStyle.iDock;
		if ( (iDock < XGE_XUI_DOCK_FILL) || (iDock > XGE_XUI_DOCK_BOTTOM) ) {
			iDock = XGE_XUI_DOCK_FILL;
		}
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tRemain, &fLeft, &fTop, &fRight, &fBottom);
		tSlot = tRemain;
		tSlot.fX += fLeft;
		tSlot.fY += fTop;
		tSlot.fW -= fLeft + fRight;
		tSlot.fH -= fTop + fBottom;
		__xgeXuiRectClampPositive(&tSlot);
		fSlotW = tSlot.fW;
		fSlotH = tSlot.fH;
		tChild = __xgeXuiChildSizeResolve(pChild, tRemain, fSlotW, fSlotH);

		if ( (iDock == XGE_XUI_DOCK_TOP) || (iDock == XGE_XUI_DOCK_BOTTOM) ) {
			if ( (__xgeXuiAlignClamp(pChild->tStyle.iAlignX) == XGE_XUI_ALIGN_STRETCH) && __xgeXuiSizeCanStretch(pChild->tStyle.tWidth) ) {
				tChild.fW = fSlotW;
			}
			__xgeXuiChildSizeClampResolved(pChild, tRemain, &tChild);
			tChild.fX = tSlot.fX + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignX), fSlotW, tChild.fW);
			if ( iDock == XGE_XUI_DOCK_TOP ) {
				tChild.fY = tSlot.fY + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignY), fSlotH, tChild.fH);
				tRemain.fY += fTop + tChild.fH + fBottom + fGap;
				tRemain.fH -= fTop + tChild.fH + fBottom + fGap;
			} else {
				tChild.fY = tSlot.fY + fSlotH - tChild.fH - __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignY), fSlotH, tChild.fH);
				tRemain.fH -= fTop + tChild.fH + fBottom + fGap;
			}
		} else if ( (iDock == XGE_XUI_DOCK_LEFT) || (iDock == XGE_XUI_DOCK_RIGHT) ) {
			if ( (__xgeXuiAlignClamp(pChild->tStyle.iAlignY) == XGE_XUI_ALIGN_STRETCH) && __xgeXuiSizeCanStretch(pChild->tStyle.tHeight) ) {
				tChild.fH = fSlotH;
			}
			__xgeXuiChildSizeClampResolved(pChild, tRemain, &tChild);
			if ( iDock == XGE_XUI_DOCK_LEFT ) {
				tChild.fX = tSlot.fX + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignX), fSlotW, tChild.fW);
				tRemain.fX += fLeft + tChild.fW + fRight + fGap;
				tRemain.fW -= fLeft + tChild.fW + fRight + fGap;
			} else {
				tChild.fX = tSlot.fX + fSlotW - tChild.fW - __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignX), fSlotW, tChild.fW);
				tRemain.fW -= fLeft + tChild.fW + fRight + fGap;
			}
			tChild.fY = tSlot.fY + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignY), fSlotH, tChild.fH);
		} else {
			if ( (__xgeXuiAlignClamp(pChild->tStyle.iAlignX) == XGE_XUI_ALIGN_STRETCH) && __xgeXuiSizeCanStretch(pChild->tStyle.tWidth) ) {
				tChild.fW = fSlotW;
			}
			if ( (__xgeXuiAlignClamp(pChild->tStyle.iAlignY) == XGE_XUI_ALIGN_STRETCH) && __xgeXuiSizeCanStretch(pChild->tStyle.tHeight) ) {
				tChild.fH = fSlotH;
			}
			__xgeXuiChildSizeClampResolved(pChild, tRemain, &tChild);
			tChild.fX = tSlot.fX + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignX), fSlotW, tChild.fW);
			tChild.fY = tSlot.fY + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignY), fSlotH, tChild.fH);
		}
		__xgeXuiRectClampPositive(&tRemain);
		__xgeXuiWidgetArrangeRect(pChild, tChild);
	}
}

static void __xgeXuiLayoutGrid(xge_xui_widget pWidget)
{
	xge_xui_widget pChild;
	xge_rect_t tParent;
	xge_rect_t tCell;
	xge_rect_t tChild;
	float fCellW;
	float fRowH;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
	float fGapX;
	float fGapY;
	float fSlotW;
	float fSlotH;
	int iColumns;
	int iIndex;
	int iColumn;
	int iRow;
	int iSpan;

	if ( pWidget == NULL ) {
		return;
	}
	tParent = pWidget->tContentRect;
	iColumns = (pWidget->tStyle.iGridColumns > 0) ? pWidget->tStyle.iGridColumns : 1;
	fGapX = (pWidget->tStyle.fGridColumnGap > 0.0f) ? pWidget->tStyle.fGridColumnGap : 0.0f;
	fGapY = (pWidget->tStyle.fGridRowGap > 0.0f) ? pWidget->tStyle.fGridRowGap : 0.0f;
	fCellW = (tParent.fW - (fGapX * (float)(iColumns - 1))) / (float)iColumns;
	if ( fCellW < 0.0f ) {
		fCellW = 0.0f;
	}
	fRowH = (pWidget->tStyle.fGridRowHeight > 0.0f) ? pWidget->tStyle.fGridRowHeight : fCellW;
	iIndex = 0;
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( !__xgeXuiLayoutVisible(pChild) ) {
			continue;
		}
		iSpan = (pChild->tStyle.iGridColumnSpan > 0) ? pChild->tStyle.iGridColumnSpan : 1;
		if ( iSpan > iColumns ) {
			iSpan = iColumns;
		}
		iColumn = iIndex % iColumns;
		if ( (iColumn + iSpan) > iColumns ) {
			iIndex += iColumns - iColumn;
			iColumn = 0;
		}
		iRow = iIndex / iColumns;
		tCell.fX = tParent.fX + ((float)iColumn * (fCellW + fGapX));
		tCell.fY = tParent.fY + ((float)iRow * (fRowH + fGapY));
		tCell.fW = (fCellW * (float)iSpan) + (fGapX * (float)(iSpan - 1));
		tCell.fH = fRowH;
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tCell, &fLeft, &fTop, &fRight, &fBottom);
		fSlotW = tCell.fW - fLeft - fRight;
		fSlotH = tCell.fH - fTop - fBottom;
		if ( fSlotW < 0.0f ) {
			fSlotW = 0.0f;
		}
		if ( fSlotH < 0.0f ) {
			fSlotH = 0.0f;
		}
		tChild = __xgeXuiChildSizeResolve(pChild, tCell, fSlotW, fSlotH);
		if ( (__xgeXuiAlignClamp(pChild->tStyle.iAlignX) == XGE_XUI_ALIGN_STRETCH) && __xgeXuiSizeCanStretch(pChild->tStyle.tWidth) ) {
			tChild.fW = fSlotW;
		}
		if ( (__xgeXuiAlignClamp(pChild->tStyle.iAlignY) == XGE_XUI_ALIGN_STRETCH) && __xgeXuiSizeCanStretch(pChild->tStyle.tHeight) ) {
			tChild.fH = fSlotH;
		}
		__xgeXuiChildSizeClampResolved(pChild, tCell, &tChild);
		tChild.fX = tCell.fX + fLeft + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignX), fSlotW, tChild.fW);
		tChild.fY = tCell.fY + fTop + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignY), fSlotH, tChild.fH);
		if ( tChild.fW < 0.0f ) {
			tChild.fW = 0.0f;
		}
		if ( tChild.fH < 0.0f ) {
			tChild.fH = 0.0f;
		}
		__xgeXuiWidgetArrangeRect(pChild, tChild);
		iIndex += iSpan;
	}
}

static void __xgeXuiLayoutOffsetSubtree(xge_xui_widget pWidget, float fDX, float fDY)
{
	xge_xui_widget pChild;
	xge_rect_t tRect;
	xge_rect_t tOuter;
	xge_rect_t tBorder;
	xge_rect_t tPadding;
	xge_rect_t tContent;

	if ( pWidget == NULL ) {
		return;
	}
	tRect = pWidget->tRect;
	tOuter = pWidget->tOuterRect;
	tBorder = pWidget->tBorderRect;
	tPadding = pWidget->tPaddingRect;
	tContent = pWidget->tContentRect;
	pWidget->tRect.fX += fDX;
	pWidget->tRect.fY += fDY;
	pWidget->tOuterRect.fX += fDX;
	pWidget->tOuterRect.fY += fDY;
	pWidget->tBorderRect.fX += fDX;
	pWidget->tBorderRect.fY += fDY;
	pWidget->tPaddingRect.fX += fDX;
	pWidget->tPaddingRect.fY += fDY;
	pWidget->tContentRect.fX += fDX;
	pWidget->tContentRect.fY += fDY;
	if ( !__xgeXuiRectSame(tRect, pWidget->tRect) || !__xgeXuiRectSame(tOuter, pWidget->tOuterRect) || !__xgeXuiRectSame(tBorder, pWidget->tBorderRect) || !__xgeXuiRectSame(tPadding, pWidget->tPaddingRect) || !__xgeXuiRectSame(tContent, pWidget->tContentRect) ) {
		xgeXuiWidgetMarkLayout(pWidget);
		xgeXuiWidgetMarkPaint(pWidget);
	}
	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( !__xgeXuiLayoutVisible(pChild) ) {
			continue;
		}
		__xgeXuiLayoutOffsetSubtree(pChild, fDX, fDY);
	}
}

static xge_xui_scroll_model __xgeXuiLayoutScrollModel(xge_xui_widget pWidget)
{
	(void)pWidget;
	return NULL;
}

static int __xgeXuiLayoutScrollRects(xge_xui_widget pWidget, xge_rect_t* pViewport, xge_rect_t* pVirtual)
{
	(void)pWidget;
	(void)pViewport;
	(void)pVirtual;
	return 0;
}

static void __xgeXuiLayoutWidget(xge_xui_widget pWidget, xge_rect_t tParent)
{
	xge_xui_widget pChild;
	xge_rect_t tRect;
	xge_rect_t tOldRect;
	xge_rect_t tViewportRect;
	xge_rect_t tVirtualRect;
	xge_rect_t tChildParentRect;
	int bDirty;
	int bScrollLayout;

	if ( pWidget == NULL ) {
		return;
	}
	if ( !__xgeXuiLayoutVisible(pWidget) ) {
		pWidget->iFlags &= ~XGE_XUI_WIDGET_DIRTY_LAYOUT;
		return;
	}

	bDirty = ((pWidget->iFlags & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0);
	tOldRect = pWidget->tRect;
	tRect = pWidget->tRect;
	if ( tRect.fW <= 0.0f ) {
		tRect.fW = tParent.fW;
	}
	if ( tRect.fH <= 0.0f ) {
		tRect.fH = tParent.fH;
	}
	if ( (bDirty == 0) && __xgeXuiRectSame(tOldRect, tRect) ) {
		tChildParentRect = pWidget->tContentRect;
		if ( __xgeXuiLayoutScrollRects(pWidget, &tViewportRect, &tVirtualRect) ) {
			pWidget->tContentRect = tViewportRect;
			tChildParentRect = tVirtualRect;
		}
		for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
			if ( !__xgeXuiLayoutVisible(pChild) ) {
				continue;
			}
			__xgeXuiLayoutWidget(pChild, tChildParentRect);
		}
		return;
	}
	pWidget->tRect = tRect;
	__xgeXuiWidgetBoxUpdate(pWidget, tParent);
	(void)__xgeXuiMeasureWidget(pWidget);
	bScrollLayout = __xgeXuiLayoutScrollRects(pWidget, &tViewportRect, &tVirtualRect);
	if ( bScrollLayout ) {
		pWidget->tContentRect = tVirtualRect;
	}
	if ( pWidget->procLayout != NULL ) {
		pWidget->procLayout(pWidget, pWidget->pLayoutUser);
	} else {
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
				__xgeXuiLayoutGrid(pWidget);
				break;

			case XGE_XUI_LAYOUT_DOCK:
				__xgeXuiLayoutDock(pWidget);
				break;

			case XGE_XUI_LAYOUT_ABSOLUTE:
			default:
				__xgeXuiLayoutAbsolute(pWidget);
				break;
		}
	}
	tChildParentRect = pWidget->tContentRect;
	if ( bScrollLayout ) {
		tChildParentRect = tVirtualRect;
		pWidget->tContentRect = tViewportRect;
	}
	pWidget->iFlags &= ~XGE_XUI_WIDGET_DIRTY_LAYOUT;

	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		if ( !__xgeXuiLayoutVisible(pChild) ) {
			continue;
		}
		__xgeXuiLayoutWidget(pChild, tChildParentRect);
	}
}
