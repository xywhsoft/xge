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
	pStyle->tAnchor.tLeft = xgeXuiSizePx(0.0f);
	pStyle->tAnchor.tTop = xgeXuiSizePx(0.0f);
	pStyle->tAnchor.tRight = xgeXuiSizePx(0.0f);
	pStyle->tAnchor.tBottom = xgeXuiSizePx(0.0f);
	pStyle->iGridColumns = 1;
	pStyle->fGridRowHeight = 0.0f;
	pStyle->fGridColumnGap = 0.0f;
	pStyle->fGridRowGap = 0.0f;
	pStyle->fGap = 0.0f;
	pStyle->iAlignX = XGE_XUI_ALIGN_STRETCH;
	pStyle->iAlignY = XGE_XUI_ALIGN_STRETCH;
	pStyle->iJustify = XGE_XUI_JUSTIFY_START;
	pStyle->iBackgroundColor = XGE_COLOR_RGBA(0, 0, 0, 0);
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

		case XGE_XUI_LAYOUT_STACK:
		case XGE_XUI_LAYOUT_GRID:
		case XGE_XUI_LAYOUT_ABSOLUTE:
		default:
			for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
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
		tSize = pWidget->procMeasure(pWidget, pWidget->pUser);
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
	if ( pWidget == NULL ) {
		return;
	}
	if ( __xgeXuiRectSame(pWidget->tRect, tRect) == 0 ) {
		pWidget->iFlags |= XGE_XUI_WIDGET_DIRTY_LAYOUT | XGE_XUI_WIDGET_DIRTY_PAINT;
	}
	pWidget->tRect = tRect;
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
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		if ( pChild->tStyle.tWidth.iUnit == XGE_XUI_SIZE_GROW ) {
			fGrow += (pChild->tStyle.tWidth.fValue > 0.0f) ? pChild->tStyle.tWidth.fValue : 1.0f;
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
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		tChild = __xgeXuiChildSizeResolve(pChild, tParent, pChild->tLocalRect.fW, tParent.fH - fTop - fBottom);
		if ( pChild->tStyle.tWidth.iUnit == XGE_XUI_SIZE_GROW ) {
			tChild.fW = (fGrow > 0.0f) ? (fRemaining * ((pChild->tStyle.tWidth.fValue > 0.0f) ? pChild->tStyle.tWidth.fValue : 1.0f) / fGrow) : 0.0f;
		}
		fSlotH = tParent.fH - fTop - fBottom;
		if ( fSlotH < 0.0f ) {
			fSlotH = 0.0f;
		}
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
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		if ( pChild->tStyle.tHeight.iUnit == XGE_XUI_SIZE_GROW ) {
			fGrow += (pChild->tStyle.tHeight.fValue > 0.0f) ? pChild->tStyle.tHeight.fValue : 1.0f;
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
		__xgeXuiEdgesResolve(&pChild->tStyle.tMargin, tParent, &fLeft, &fTop, &fRight, &fBottom);
		tChild = __xgeXuiChildSizeResolve(pChild, tParent, tParent.fW - fLeft - fRight, pChild->tLocalRect.fH);
		if ( pChild->tStyle.tHeight.iUnit == XGE_XUI_SIZE_GROW ) {
			tChild.fH = (fGrow > 0.0f) ? (fRemaining * ((pChild->tStyle.tHeight.fValue > 0.0f) ? pChild->tStyle.tHeight.fValue : 1.0f) / fGrow) : 0.0f;
		}
		fSlotW = tParent.fW - fLeft - fRight;
		if ( fSlotW < 0.0f ) {
			fSlotW = 0.0f;
		}
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
		tChild.fX = tParent.fX + fLeft + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignX), fSlotW, tChild.fW);
		tChild.fY = tParent.fY + fTop + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignY), fSlotH, tChild.fH);
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
		iColumn = iIndex % iColumns;
		iRow = iIndex / iColumns;
		tCell.fX = tParent.fX + ((float)iColumn * (fCellW + fGapX));
		tCell.fY = tParent.fY + ((float)iRow * (fRowH + fGapY));
		tCell.fW = fCellW;
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
		tChild.fX = tCell.fX + fLeft + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignX), fSlotW, tChild.fW);
		tChild.fY = tCell.fY + fTop + __xgeXuiAlignOffset(__xgeXuiAlignClamp(pChild->tStyle.iAlignY), fSlotH, tChild.fH);
		if ( tChild.fW < 0.0f ) {
			tChild.fW = 0.0f;
		}
		if ( tChild.fH < 0.0f ) {
			tChild.fH = 0.0f;
		}
		__xgeXuiWidgetArrangeRect(pChild, tChild);
		iIndex++;
	}
}

static void __xgeXuiLayoutWidget(xge_xui_widget pWidget, xge_rect_t tParent)
{
	xge_xui_widget pChild;
	xge_rect_t tRect;
	xge_rect_t tOldRect;
	int bDirty;

	if ( pWidget == NULL ) {
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
		return;
	}
	pWidget->tRect = tRect;
	pWidget->tContentRect = __xgeXuiContentRect(pWidget);
	(void)__xgeXuiMeasureWidget(pWidget);
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

			case XGE_XUI_LAYOUT_ABSOLUTE:
			default:
				__xgeXuiLayoutAbsolute(pWidget);
				break;
		}
	}
	pWidget->iFlags &= ~XGE_XUI_WIDGET_DIRTY_LAYOUT;

	for ( pChild = pWidget->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling ) {
		__xgeXuiLayoutWidget(pChild, pWidget->tContentRect);
	}
}
