int xgeXuiWindowInit(xge_xui_window pWindow, xge_xui_context pContext, xge_xui_widget pWidget);
void xgeXuiWindowUnit(xge_xui_window pWindow);
xge_xui_widget xgeXuiWindowGetClientWidget(xge_xui_window pWindow);
void xgeXuiWindowSetTitle(xge_xui_window pWindow, xge_font pFont, const char* sTitle);
void xgeXuiWindowSetIcon(xge_xui_window pWindow, xge_texture pTexture, xge_rect_t tSrc);
void xgeXuiWindowSetClose(xge_xui_window pWindow, xge_xui_click_proc procClose, void* pUser);
void xgeXuiWindowSetOpen(xge_xui_window pWindow, int bOpen);
int xgeXuiWindowIsOpen(xge_xui_window pWindow);
void xgeXuiWindowSetShowTitleBar(xge_xui_window pWindow, int bShow);
void xgeXuiWindowSetMovable(xge_xui_window pWindow, int bEnabled);
void xgeXuiWindowSetDragAnywhere(xge_xui_window pWindow, int bEnabled);
void xgeXuiWindowSetResizable(xge_xui_window pWindow, int bEnabled);
void xgeXuiWindowSetResizeEdges(xge_xui_window pWindow, uint32_t iEdges);
void xgeXuiWindowSetShowCollapse(xge_xui_window pWindow, int bShow);
void xgeXuiWindowSetShowMaximize(xge_xui_window pWindow, int bShow);
void xgeXuiWindowSetShowClose(xge_xui_window pWindow, int bShow);
void xgeXuiWindowSetCollapsed(xge_xui_window pWindow, int bCollapsed);
int xgeXuiWindowIsCollapsed(xge_xui_window pWindow);
void xgeXuiWindowSetMaximized(xge_xui_window pWindow, int bMaximized);
int xgeXuiWindowIsMaximized(xge_xui_window pWindow);
void xgeXuiWindowSetChrome(xge_xui_window pWindow, float fTitleBarHeight, float fBorderWidth, float fResizeGrip, float fButtonSize);
void xgeXuiWindowSetColors(xge_xui_window pWindow, uint32_t iBackground, uint32_t iTitleBar, uint32_t iTitleText, uint32_t iBorder, uint32_t iButtonNormal, uint32_t iButtonHover, uint32_t iButtonActive);
int xgeXuiWindowEvent(xge_xui_window pWindow, const xge_event_t* pEvent);
int xgeXuiWindowEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
void xgeXuiWindowPaintProc(xge_xui_widget pWidget, void* pUser);
static void __xgeXuiWindowButtonPaintProc(xge_xui_widget pWidget, void* pUser);
static void __xgeXuiWindowPaintAfter(xge_xui_widget pWidget, void* pUser);

static int g_iXgeXuiWindowRaiseZ = 1500;

static float __xgeXuiWindowClampFloat(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

static void __xgeXuiWindowArrangeChild(xge_xui_widget pWidget, xge_rect_t tRect)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tLocalRect = tRect;
	__xgeXuiWidgetArrangeRect(pWidget, tRect);
}

static xge_rect_t __xgeXuiWindowParentRect(xge_xui_window pWindow)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( (pWindow == NULL) || (pWindow->pWidget == NULL) ) {
		return tRect;
	}
	if ( pWindow->pWidget->pParent != NULL ) {
		return pWindow->pWidget->pParent->tContentRect;
	}
	if ( (pWindow->pContext != NULL) && (pWindow->pContext->pRoot != NULL) ) {
		return pWindow->pContext->pRoot->tRect;
	}
	return pWindow->pWidget->tRect;
}

static float __xgeXuiWindowCollapsedHeight(xge_xui_window pWindow)
{
	float fHeight;

	if ( pWindow == NULL ) {
		return 28.0f;
	}
	fHeight = pWindow->fBorderWidth * 2.0f;
	if ( pWindow->bShowTitleBar != 0 ) {
		fHeight += pWindow->fTitleBarHeight;
		fHeight -= 1.0f;
	}
	if ( fHeight < 24.0f ) {
		fHeight = 24.0f;
	}
	return fHeight;
}

static float __xgeXuiWindowMinWidth(xge_xui_window pWindow)
{
	xge_rect_t tParent;
	float fMinWidth;
	float fButtons;

	if ( (pWindow == NULL) || (pWindow->pWidget == NULL) ) {
		return 120.0f;
	}
	tParent = (pWindow->pWidget->pParent != NULL) ? pWindow->pWidget->pParent->tContentRect : pWindow->pWidget->tRect;
	fMinWidth = __xgeXuiSizeResolve(pWindow->pWidget->tStyle.tMinWidth, tParent.fW, 0.0f, 0.0f);
	fButtons = pWindow->fBorderWidth * 2.0f + 16.0f + 16.0f;
	if ( pWindow->bShowCollapse != 0 ) {
		fButtons += pWindow->fButtonSize + 4.0f;
	}
	if ( pWindow->bShowMaximize != 0 ) {
		fButtons += pWindow->fButtonSize + 4.0f;
	}
	if ( pWindow->bShowClose != 0 ) {
		fButtons += pWindow->fButtonSize + 4.0f;
	}
	if ( fMinWidth < (fButtons + 72.0f) ) {
		fMinWidth = fButtons + 72.0f;
	}
	if ( fMinWidth < 120.0f ) {
		fMinWidth = 120.0f;
	}
	return fMinWidth;
}

static float __xgeXuiWindowMinHeight(xge_xui_window pWindow)
{
	xge_rect_t tParent;
	float fMinHeight;
	float fCollapsedHeight;

	if ( (pWindow == NULL) || (pWindow->pWidget == NULL) ) {
		return 80.0f;
	}
	tParent = (pWindow->pWidget->pParent != NULL) ? pWindow->pWidget->pParent->tContentRect : pWindow->pWidget->tRect;
	fMinHeight = __xgeXuiSizeResolve(pWindow->pWidget->tStyle.tMinHeight, tParent.fH, 0.0f, 0.0f);
	fCollapsedHeight = __xgeXuiWindowCollapsedHeight(pWindow);
	if ( fMinHeight < fCollapsedHeight ) {
		fMinHeight = fCollapsedHeight;
	}
	if ( pWindow->bCollapsed == 0 && fMinHeight < (fCollapsedHeight + 48.0f) ) {
		fMinHeight = fCollapsedHeight + 48.0f;
	}
	return fMinHeight;
}

static void __xgeXuiWindowClampRect(xge_xui_window pWindow, xge_rect_t* pRect)
{
	xge_rect_t tParent;
	float fMinWidth;
	float fMinHeight;
	float fMaxWidth;
	float fMaxHeight;

	if ( (pWindow == NULL) || (pRect == NULL) ) {
		return;
	}
	tParent = __xgeXuiWindowParentRect(pWindow);
	fMinWidth = __xgeXuiWindowMinWidth(pWindow);
	fMinHeight = __xgeXuiWindowMinHeight(pWindow);
	fMaxWidth = (tParent.fW > 0.0f) ? tParent.fW : pRect->fW;
	fMaxHeight = (tParent.fH > 0.0f) ? tParent.fH : pRect->fH;
	pRect->fW = __xgeXuiWindowClampFloat(pRect->fW, fMinWidth, (fMaxWidth > fMinWidth) ? fMaxWidth : fMinWidth);
	pRect->fH = __xgeXuiWindowClampFloat(pRect->fH, fMinHeight, (fMaxHeight > fMinHeight) ? fMaxHeight : fMinHeight);
	if ( pRect->fX < tParent.fX ) {
		pRect->fX = tParent.fX;
	}
	if ( pRect->fY < tParent.fY ) {
		pRect->fY = tParent.fY;
	}
	if ( pRect->fX + pRect->fW > tParent.fX + tParent.fW ) {
		pRect->fX = (tParent.fX + tParent.fW) - pRect->fW;
	}
	if ( pRect->fY + pRect->fH > tParent.fY + tParent.fH ) {
		pRect->fY = (tParent.fY + tParent.fH) - pRect->fH;
	}
	if ( pRect->fX < tParent.fX ) {
		pRect->fX = tParent.fX;
	}
	if ( pRect->fY < tParent.fY ) {
		pRect->fY = tParent.fY;
	}
}

static void __xgeXuiWindowApplyRect(xge_xui_window pWindow, xge_rect_t tRect)
{
	if ( (pWindow == NULL) || (pWindow->pWidget == NULL) ) {
		return;
	}
	__xgeXuiWindowClampRect(pWindow, &tRect);
	if ( pWindow->bCollapsed == 0 ) {
		pWindow->fExpandedHeight = tRect.fH;
	}
	xgeXuiWidgetSetSize(pWindow->pWidget, xgeXuiSizePx(tRect.fW), xgeXuiSizePx(tRect.fH));
	xgeXuiWidgetSetRect(pWindow->pWidget, tRect);
}

static void __xgeXuiWindowSyncButtonText(xge_xui_window pWindow)
{
	if ( pWindow == NULL ) {
		return;
	}
	xgeXuiButtonSetText(&pWindow->tCollapseButton, pWindow->pFont, "");
	xgeXuiButtonSetText(&pWindow->tMaximizeButton, pWindow->pFont, "");
	xgeXuiButtonSetText(&pWindow->tCloseButton, pWindow->pFont, "");
}

static void __xgeXuiWindowSyncButtonColors(xge_xui_window pWindow)
{
	if ( pWindow == NULL ) {
		return;
	}
	xgeXuiButtonSetColors(&pWindow->tCollapseButton,
		XGE_COLOR_RGBA(20, 32, 56, 196),
		XGE_COLOR_RGBA(42, 74, 124, 232),
		XGE_COLOR_RGBA(14, 24, 40, 236),
		XGE_COLOR_RGBA(42, 74, 124, 232),
		XGE_COLOR_RGBA(20, 32, 56, 128));
	xgeXuiButtonSetColors(&pWindow->tMaximizeButton,
		XGE_COLOR_RGBA(20, 32, 56, 196),
		XGE_COLOR_RGBA(42, 74, 124, 232),
		XGE_COLOR_RGBA(14, 24, 40, 236),
		XGE_COLOR_RGBA(42, 74, 124, 232),
		XGE_COLOR_RGBA(20, 32, 56, 128));
	xgeXuiButtonSetColors(&pWindow->tCloseButton,
		XGE_COLOR_RGBA(255, 255, 255, 0),
		XGE_COLOR_RGBA(255, 230, 230, 255),
		XGE_COLOR_RGBA(238, 198, 198, 255),
		XGE_COLOR_RGBA(255, 230, 230, 255),
		XGE_COLOR_RGBA(255, 255, 255, 0));
	xgeXuiButtonSetTextColor(&pWindow->tCollapseButton, pWindow->iTitleTextColor);
	xgeXuiButtonSetTextColor(&pWindow->tMaximizeButton, pWindow->iTitleTextColor);
	xgeXuiButtonSetTextColor(&pWindow->tCloseButton, pWindow->iTitleTextColor);
}

static void __xgeXuiWindowClose(xge_xui_window pWindow)
{
	if ( (pWindow == NULL) || (pWindow->bOpen == 0) ) {
		return;
	}
	xgeXuiWindowSetOpen(pWindow, 0);
	if ( pWindow->procClose != NULL ) {
		pWindow->procClose(pWindow->pWidget, pWindow->pUser);
	}
}

static void __xgeXuiWindowCollapseClick(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	xgeXuiWindowSetCollapsed((xge_xui_window)pUser, ((xge_xui_window)pUser)->bCollapsed == 0);
}

static void __xgeXuiWindowMaximizeClick(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	xgeXuiWindowSetMaximized((xge_xui_window)pUser, ((xge_xui_window)pUser)->bMaximized == 0);
}

static void __xgeXuiWindowCloseClick(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	__xgeXuiWindowClose((xge_xui_window)pUser);
}

static int __xgeXuiWindowResizeEdgesAt(xge_xui_window pWindow, float fX, float fY)
{
	int iEdges;
	xge_rect_t tRect;
	float fLeftGrip;
	float fRightGrip;
	float fTopGrip;
	float fBottomGrip;

	if ( (pWindow == NULL) || (pWindow->bResizable == 0) || (pWindow->bMaximized != 0) ) {
		return 0;
	}
	tRect = pWindow->pWidget->tRect;
	if ( __xgeXuiRectContains(tRect, fX, fY) == 0 ) {
		return 0;
	}
	iEdges = 0;
	fLeftGrip = pWindow->fResizeGrip;
	fRightGrip = pWindow->fResizeGrip;
	fBottomGrip = pWindow->fResizeGrip;
	fTopGrip = (pWindow->bShowTitleBar != 0) ? 3.0f : pWindow->fResizeGrip;
	if ( ((pWindow->iResizeEdges & XGE_XUI_WINDOW_EDGE_LEFT) != 0) && (fX <= (tRect.fX + fLeftGrip)) ) {
		iEdges |= XGE_XUI_WINDOW_EDGE_LEFT;
	}
	if ( ((pWindow->iResizeEdges & XGE_XUI_WINDOW_EDGE_RIGHT) != 0) && (fX >= (tRect.fX + tRect.fW - fRightGrip)) ) {
		iEdges |= XGE_XUI_WINDOW_EDGE_RIGHT;
	}
	if ( ((pWindow->iResizeEdges & XGE_XUI_WINDOW_EDGE_TOP) != 0) && (fY <= (tRect.fY + fTopGrip)) ) {
		iEdges |= XGE_XUI_WINDOW_EDGE_TOP;
	}
	if ( ((pWindow->iResizeEdges & XGE_XUI_WINDOW_EDGE_BOTTOM) != 0) && (fY >= (tRect.fY + tRect.fH - fBottomGrip)) ) {
		iEdges |= XGE_XUI_WINDOW_EDGE_BOTTOM;
	}
	return iEdges;
}

static int __xgeXuiWindowCanStartMove(xge_xui_window pWindow, xge_xui_widget pHit, float fX, float fY)
{
	xge_rect_t tTitleRect;

	if ( (pWindow == NULL) || (pWindow->bMovable == 0) || (pWindow->bMaximized != 0) ) {
		return 0;
	}
	if ( pWindow->bShowTitleBar != 0 ) {
		tTitleRect = pWindow->pWidget->tRect;
		tTitleRect.fH = pWindow->fTitleBarHeight;
		if ( __xgeXuiRectContains(tTitleRect, fX, fY) && (pHit == pWindow->pWidget) ) {
			return 1;
		}
	}
	if ( pWindow->bDragAnywhere != 0 && ((pHit == pWindow->pWidget) || (pHit == pWindow->pClientWidget)) ) {
		return 1;
	}
	return 0;
}

static void __xgeXuiWindowBuildPreviewRect(xge_xui_window pWindow, float fX, float fY)
{
	xge_rect_t tRect;
	float fDX;
	float fDY;

	if ( (pWindow == NULL) || (pWindow->iInteractionEdges == 0) ) {
		return;
	}
	fDX = fX - pWindow->fDragStartX;
	fDY = fY - pWindow->fDragStartY;
	tRect = pWindow->tDragStartRect;
	if ( pWindow->iInteractionEdges == -1 ) {
		tRect.fX += fDX;
		tRect.fY += fDY;
	} else {
		if ( (pWindow->iInteractionEdges & XGE_XUI_WINDOW_EDGE_LEFT) != 0 ) {
			tRect.fX += fDX;
			tRect.fW -= fDX;
		}
		if ( (pWindow->iInteractionEdges & XGE_XUI_WINDOW_EDGE_RIGHT) != 0 ) {
			tRect.fW += fDX;
		}
		if ( (pWindow->iInteractionEdges & XGE_XUI_WINDOW_EDGE_TOP) != 0 ) {
			tRect.fY += fDY;
			tRect.fH -= fDY;
		}
		if ( (pWindow->iInteractionEdges & XGE_XUI_WINDOW_EDGE_BOTTOM) != 0 ) {
			tRect.fH += fDY;
		}
	}
	__xgeXuiWindowClampRect(pWindow, &tRect);
	pWindow->tPreviewRect = tRect;
	pWindow->bPreviewActive = 1;
}

static void __xgeXuiWindowCommitPreview(xge_xui_window pWindow)
{
	if ( (pWindow == NULL) || (pWindow->bPreviewActive == 0) ) {
		return;
	}
	__xgeXuiWindowApplyRect(pWindow, pWindow->tPreviewRect);
	pWindow->bPreviewActive = 0;
}

static void __xgeXuiWindowCancelPreview(xge_xui_window pWindow)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->bPreviewActive = 0;
}

static xge_vec2_t __xgeXuiWindowMeasure(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_window pWindow;
	xge_vec2_t tSize;
	xge_vec2_t tClientSize;

	(void)pWidget;
	tSize.fX = 0.0f;
	tSize.fY = 0.0f;
	pWindow = (xge_xui_window)pUser;
	if ( (pWindow == NULL) || (pWindow->pClientWidget == NULL) ) {
		return tSize;
	}
	tClientSize = __xgeXuiWidgetMeasureContent(pWindow->pClientWidget);
	tSize.fX = tClientSize.fX + pWindow->fBorderWidth * 2.0f;
	tSize.fY = tClientSize.fY + pWindow->fBorderWidth * 2.0f + ((pWindow->bShowTitleBar != 0) ? pWindow->fTitleBarHeight : 0.0f);
	if ( pWindow->bCollapsed != 0 ) {
		tSize.fY = __xgeXuiWindowCollapsedHeight(pWindow);
	}
	if ( tSize.fX < __xgeXuiWindowMinWidth(pWindow) ) {
		tSize.fX = __xgeXuiWindowMinWidth(pWindow);
	}
	if ( tSize.fY < __xgeXuiWindowMinHeight(pWindow) ) {
		tSize.fY = __xgeXuiWindowMinHeight(pWindow);
	}
	return tSize;
}

static void __xgeXuiWindowLayout(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_window pWindow;
	xge_rect_t tRect;
	xge_rect_t tClientRect;
	xge_rect_t tButtonRect;
	float fTitleHeight;
	float fBorder;
	float fButtonSize;
	float fButtonGap;
	float fButtonX;
	float fButtonY;
	float fButtonInsetRight;

	pWindow = (xge_xui_window)pUser;
	if ( (pWidget == NULL) || (pWindow == NULL) ) {
		return;
	}
	tRect = pWidget->tRect;
	fTitleHeight = (pWindow->bShowTitleBar != 0) ? pWindow->fTitleBarHeight : 0.0f;
	fBorder = pWindow->fBorderWidth;
	fButtonSize = (fTitleHeight > 0.0f) ? pWindow->fButtonSize : 0.0f;
	if ( fButtonSize > (fTitleHeight - 6.0f) ) {
		fButtonSize = fTitleHeight - 6.0f;
	}
	if ( fButtonSize < 12.0f ) {
		fButtonSize = 12.0f;
	}
	fButtonGap = 4.0f;
	fButtonInsetRight = 6.0f;
	tClientRect.fX = tRect.fX + fBorder;
	tClientRect.fY = tRect.fY + fBorder + fTitleHeight;
	tClientRect.fW = tRect.fW - fBorder * 2.0f;
	tClientRect.fH = tRect.fH - fBorder * 2.0f - fTitleHeight;
	if ( tClientRect.fW < 0.0f ) {
		tClientRect.fW = 0.0f;
	}
	if ( tClientRect.fH < 0.0f ) {
		tClientRect.fH = 0.0f;
	}
	xgeXuiWidgetSetVisible(pWindow->pClientWidget, pWindow->bCollapsed == 0);
	if ( pWindow->bCollapsed != 0 ) {
		tClientRect.fH = 0.0f;
	}
	__xgeXuiWindowArrangeChild(pWindow->pClientWidget, tClientRect);
	fButtonX = tRect.fX + tRect.fW - fBorder - fButtonInsetRight - fButtonSize;
	fButtonY = tRect.fY + fBorder + (fTitleHeight - fButtonSize) * 0.5f;
	memset(&tButtonRect, 0, sizeof(tButtonRect));
	tButtonRect.fW = fButtonSize;
	tButtonRect.fH = fButtonSize;
	tButtonRect.fY = fButtonY;
	xgeXuiWidgetSetVisible(pWindow->pCloseButtonWidget, (pWindow->bShowTitleBar != 0) && (pWindow->bShowClose != 0));
	xgeXuiWidgetSetVisible(pWindow->pMaximizeButtonWidget, (pWindow->bShowTitleBar != 0) && (pWindow->bShowMaximize != 0));
	xgeXuiWidgetSetVisible(pWindow->pCollapseButtonWidget, (pWindow->bShowTitleBar != 0) && (pWindow->bShowCollapse != 0));
	if ( (pWindow->bShowTitleBar != 0) && (pWindow->bShowClose != 0) ) {
		tButtonRect.fX = fButtonX;
		__xgeXuiWindowArrangeChild(pWindow->pCloseButtonWidget, tButtonRect);
		fButtonX -= fButtonSize + fButtonGap;
	}
	if ( (pWindow->bShowTitleBar != 0) && (pWindow->bShowMaximize != 0) ) {
		tButtonRect.fX = fButtonX;
		__xgeXuiWindowArrangeChild(pWindow->pMaximizeButtonWidget, tButtonRect);
		fButtonX -= fButtonSize + fButtonGap;
	}
	if ( (pWindow->bShowTitleBar != 0) && (pWindow->bShowCollapse != 0) ) {
		tButtonRect.fX = fButtonX;
		__xgeXuiWindowArrangeChild(pWindow->pCollapseButtonWidget, tButtonRect);
	}
}

static void __xgeXuiWindowDrawBorder(xge_rect_t tRect, uint32_t iColor, float fBorder)
{
	xge_rect_t tEdge;

	if ( (XGE_COLOR_GET_A(iColor) == 0) || (fBorder <= 0.0f) ) {
		return;
	}
	tEdge = tRect;
	tEdge.fH = fBorder;
	__xgeXuiHostDrawRect(tEdge, iColor);
	tEdge = tRect;
	tEdge.fY = tRect.fY + tRect.fH - fBorder;
	tEdge.fH = fBorder;
	__xgeXuiHostDrawRect(tEdge, iColor);
	tEdge = tRect;
	tEdge.fW = fBorder;
	__xgeXuiHostDrawRect(tEdge, iColor);
	tEdge = tRect;
	tEdge.fX = tRect.fX + tRect.fW - fBorder;
	tEdge.fW = fBorder;
	__xgeXuiHostDrawRect(tEdge, iColor);
}

int xgeXuiWindowInit(xge_xui_window pWindow, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;
	xge_rect_t tZero;

	if ( (pWindow == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pWindow, 0, sizeof(*pWindow));
	memset(&tZero, 0, sizeof(tZero));
	pTheme = xgeXuiGetTheme(pContext);
	pWindow->pContext = pContext;
	pWindow->pWidget = pWidget;
	pWindow->pFont = pTheme->pFont;
	pWindow->sTitle = "";
	pWindow->iBackgroundColor = pTheme->iPanelColor;
	pWindow->iTitleBarColor = pTheme->iBackgroundColor;
	pWindow->iTitleTextColor = pTheme->iTextColor;
	pWindow->iBorderColor = pTheme->iBorderColor;
	pWindow->iButtonColorNormal = XGE_COLOR_RGBA(255, 255, 255, 0);
	pWindow->iButtonColorHover = pTheme->iStateHover;
	pWindow->iButtonColorActive = pTheme->iStateActive;
	pWindow->fTitleBarHeight = 28.0f;
	pWindow->fBorderWidth = pTheme->fBorderWidth;
	pWindow->fResizeGrip = 6.0f;
	pWindow->fButtonSize = 18.0f;
	pWindow->fIconSize = 16.0f;
	pWindow->iResizeEdges = XGE_XUI_WINDOW_EDGE_LEFT | XGE_XUI_WINDOW_EDGE_TOP | XGE_XUI_WINDOW_EDGE_RIGHT | XGE_XUI_WINDOW_EDGE_BOTTOM;
	pWindow->bOpen = 1;
	pWindow->bShowTitleBar = 1;
	pWindow->bMovable = 1;
	pWindow->bResizable = 1;
	pWindow->bShowCollapse = 1;
	pWindow->bShowMaximize = 1;
	pWindow->bShowClose = 1;
	pWindow->fExpandedHeight = 160.0f;
	pWindow->tPreviewRect = pWidget->tRect;
	pWindow->pClientWidget = xgeXuiWidgetCreate();
	pWindow->pCollapseButtonWidget = xgeXuiWidgetCreate();
	pWindow->pMaximizeButtonWidget = xgeXuiWidgetCreate();
	pWindow->pCloseButtonWidget = xgeXuiWidgetCreate();
	if ( (pWindow->pClientWidget == NULL) || (pWindow->pCollapseButtonWidget == NULL) || (pWindow->pMaximizeButtonWidget == NULL) || (pWindow->pCloseButtonWidget == NULL) ) {
		xgeXuiWindowUnit(pWindow);
		return XGE_ERROR;
	}
	xgeXuiWidgetSetLayout(pWindow->pClientWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetName(pWindow->pClientWidget, "window-client");
	xgeXuiWidgetSetName(pWindow->pCollapseButtonWidget, "window-collapse");
	xgeXuiWidgetSetName(pWindow->pMaximizeButtonWidget, "window-maximize");
	xgeXuiWidgetSetName(pWindow->pCloseButtonWidget, "window-close");
	xgeXuiWidgetSetRect(pWindow->pClientWidget, tZero);
	xgeXuiWidgetSetRect(pWindow->pCollapseButtonWidget, tZero);
	xgeXuiWidgetSetRect(pWindow->pMaximizeButtonWidget, tZero);
	xgeXuiWidgetSetRect(pWindow->pCloseButtonWidget, tZero);
	if ( xgeXuiWidgetAdd(pWidget, pWindow->pClientWidget) != XGE_OK ||
		xgeXuiWidgetAdd(pWidget, pWindow->pCollapseButtonWidget) != XGE_OK ||
		xgeXuiWidgetAdd(pWidget, pWindow->pMaximizeButtonWidget) != XGE_OK ||
		xgeXuiWidgetAdd(pWidget, pWindow->pCloseButtonWidget) != XGE_OK ) {
		xgeXuiWindowUnit(pWindow);
		return XGE_ERROR;
	}
	xgeXuiWidgetSetZ(pWindow->pCollapseButtonWidget, 10);
	xgeXuiWidgetSetZ(pWindow->pMaximizeButtonWidget, 11);
	xgeXuiWidgetSetZ(pWindow->pCloseButtonWidget, 12);
	if ( xgeXuiButtonInit(&pWindow->tCollapseButton, pContext, pWindow->pCollapseButtonWidget) != XGE_OK ||
		xgeXuiButtonInit(&pWindow->tMaximizeButton, pContext, pWindow->pMaximizeButtonWidget) != XGE_OK ||
		xgeXuiButtonInit(&pWindow->tCloseButton, pContext, pWindow->pCloseButtonWidget) != XGE_OK ) {
		xgeXuiWindowUnit(pWindow);
		return XGE_ERROR;
	}
	xgeXuiButtonSetClick(&pWindow->tCollapseButton, __xgeXuiWindowCollapseClick, pWindow);
	xgeXuiButtonSetClick(&pWindow->tMaximizeButton, __xgeXuiWindowMaximizeClick, pWindow);
	xgeXuiButtonSetClick(&pWindow->tCloseButton, __xgeXuiWindowCloseClick, pWindow);
	pWindow->pCollapseButtonWidget->procPaint = __xgeXuiWindowButtonPaintProc;
	pWindow->pMaximizeButtonWidget->procPaint = __xgeXuiWindowButtonPaintProc;
	pWindow->pCloseButtonWidget->procPaint = __xgeXuiWindowButtonPaintProc;
	__xgeXuiWindowSyncButtonText(pWindow);
	__xgeXuiWindowSyncButtonColors(pWindow);
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(pWidget->tLocalRect.fW), xgeXuiSizePx(pWidget->tLocalRect.fH));
	pWidget->procEvent = xgeXuiWindowEventProc;
	pWidget->procMeasure = __xgeXuiWindowMeasure;
	pWidget->procLayout = __xgeXuiWindowLayout;
	pWidget->pLayoutUser = pWindow;
	pWidget->procPaint = xgeXuiWindowPaintProc;
	pWidget->procPaintAfter = __xgeXuiWindowPaintAfter;
	pWidget->pUser = pWindow;
	xgeXuiWidgetSetZ(pWidget, ++g_iXgeXuiWindowRaiseZ);
	xgeXuiWidgetMarkLayout(pWidget);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiWindowUnit(xge_xui_window pWindow)
{
	if ( pWindow == NULL ) {
		return;
	}
	xgeXuiButtonUnit(&pWindow->tCloseButton);
	xgeXuiButtonUnit(&pWindow->tMaximizeButton);
	xgeXuiButtonUnit(&pWindow->tCollapseButton);
	if ( pWindow->pCloseButtonWidget != NULL ) {
		xgeXuiWidgetFree(pWindow->pCloseButtonWidget);
	}
	if ( pWindow->pMaximizeButtonWidget != NULL ) {
		xgeXuiWidgetFree(pWindow->pMaximizeButtonWidget);
	}
	if ( pWindow->pCollapseButtonWidget != NULL ) {
		xgeXuiWidgetFree(pWindow->pCollapseButtonWidget);
	}
	if ( pWindow->pClientWidget != NULL ) {
		xgeXuiWidgetFree(pWindow->pClientWidget);
	}
	if ( pWindow->pWidget != NULL && pWindow->pWidget->pUser == pWindow ) {
		pWindow->pWidget->pUser = NULL;
		pWindow->pWidget->procEvent = NULL;
		pWindow->pWidget->procMeasure = NULL;
		pWindow->pWidget->procLayout = NULL;
		pWindow->pWidget->pLayoutUser = NULL;
		pWindow->pWidget->procPaint = NULL;
		pWindow->pWidget->procPaintAfter = NULL;
	}
	memset(pWindow, 0, sizeof(*pWindow));
}

xge_xui_widget xgeXuiWindowGetClientWidget(xge_xui_window pWindow)
{
	return (pWindow != NULL) ? pWindow->pClientWidget : NULL;
}

void xgeXuiWindowSetTitle(xge_xui_window pWindow, xge_font pFont, const char* sTitle)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->pFont = pFont;
	pWindow->sTitle = (sTitle != NULL) ? sTitle : "";
	__xgeXuiWindowSyncButtonText(pWindow);
	xgeXuiWidgetMarkLayout(pWindow->pWidget);
	xgeXuiWidgetMarkPaint(pWindow->pWidget);
}

void xgeXuiWindowSetIcon(xge_xui_window pWindow, xge_texture pTexture, xge_rect_t tSrc)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->pIconTexture = pTexture;
	pWindow->tIconSrc = tSrc;
	xgeXuiWidgetMarkPaint(pWindow->pWidget);
}

void xgeXuiWindowSetClose(xge_xui_window pWindow, xge_xui_click_proc procClose, void* pUser)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->procClose = procClose;
	pWindow->pUser = pUser;
}

void xgeXuiWindowSetOpen(xge_xui_window pWindow, int bOpen)
{
	if ( pWindow == NULL ) {
		return;
	}
	bOpen = bOpen ? 1 : 0;
	if ( pWindow->bOpen == bOpen ) {
		return;
	}
	pWindow->bOpen = bOpen;
	xgeXuiWidgetSetVisible(pWindow->pWidget, bOpen);
	if ( bOpen != 0 ) {
		xgeXuiSetFocus(pWindow->pContext, pWindow->pWidget);
	}
	xgeXuiWidgetMarkPaint(pWindow->pWidget);
}

int xgeXuiWindowIsOpen(xge_xui_window pWindow)
{
	return (pWindow != NULL) ? pWindow->bOpen : 0;
}

void xgeXuiWindowSetShowTitleBar(xge_xui_window pWindow, int bShow)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->bShowTitleBar = bShow ? 1 : 0;
	xgeXuiWidgetMarkLayout(pWindow->pWidget);
	xgeXuiWidgetMarkPaint(pWindow->pWidget);
}

void xgeXuiWindowSetMovable(xge_xui_window pWindow, int bEnabled)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->bMovable = bEnabled ? 1 : 0;
}

void xgeXuiWindowSetDragAnywhere(xge_xui_window pWindow, int bEnabled)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->bDragAnywhere = bEnabled ? 1 : 0;
}

void xgeXuiWindowSetResizable(xge_xui_window pWindow, int bEnabled)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->bResizable = bEnabled ? 1 : 0;
}

void xgeXuiWindowSetResizeEdges(xge_xui_window pWindow, uint32_t iEdges)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->iResizeEdges = iEdges & (XGE_XUI_WINDOW_EDGE_LEFT | XGE_XUI_WINDOW_EDGE_TOP | XGE_XUI_WINDOW_EDGE_RIGHT | XGE_XUI_WINDOW_EDGE_BOTTOM);
}

void xgeXuiWindowSetShowCollapse(xge_xui_window pWindow, int bShow)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->bShowCollapse = bShow ? 1 : 0;
	xgeXuiWidgetMarkLayout(pWindow->pWidget);
	xgeXuiWidgetMarkPaint(pWindow->pWidget);
}

void xgeXuiWindowSetShowMaximize(xge_xui_window pWindow, int bShow)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->bShowMaximize = bShow ? 1 : 0;
	xgeXuiWidgetMarkLayout(pWindow->pWidget);
	xgeXuiWidgetMarkPaint(pWindow->pWidget);
}

void xgeXuiWindowSetShowClose(xge_xui_window pWindow, int bShow)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->bShowClose = bShow ? 1 : 0;
	xgeXuiWidgetMarkLayout(pWindow->pWidget);
	xgeXuiWidgetMarkPaint(pWindow->pWidget);
}

void xgeXuiWindowSetCollapsed(xge_xui_window pWindow, int bCollapsed)
{
	xge_rect_t tRect;
	float fCollapsedHeight;

	if ( pWindow == NULL ) {
		return;
	}
	bCollapsed = bCollapsed ? 1 : 0;
	if ( pWindow->bCollapsed == bCollapsed ) {
		return;
	}
	tRect = pWindow->pWidget->tRect;
	fCollapsedHeight = __xgeXuiWindowCollapsedHeight(pWindow);
	if ( bCollapsed != 0 ) {
		if ( tRect.fH > fCollapsedHeight ) {
			pWindow->fExpandedHeight = tRect.fH;
		}
		tRect.fH = fCollapsedHeight;
	} else {
		if ( pWindow->fExpandedHeight < (fCollapsedHeight + 48.0f) ) {
			pWindow->fExpandedHeight = fCollapsedHeight + 96.0f;
		}
		tRect.fH = pWindow->fExpandedHeight;
	}
	pWindow->bCollapsed = bCollapsed;
	__xgeXuiWindowSyncButtonText(pWindow);
	__xgeXuiWindowApplyRect(pWindow, tRect);
	xgeXuiWidgetMarkLayout(pWindow->pWidget);
	xgeXuiWidgetMarkPaint(pWindow->pWidget);
}

int xgeXuiWindowIsCollapsed(xge_xui_window pWindow)
{
	return (pWindow != NULL) ? pWindow->bCollapsed : 0;
}

void xgeXuiWindowSetMaximized(xge_xui_window pWindow, int bMaximized)
{
	xge_rect_t tRect;

	if ( pWindow == NULL ) {
		return;
	}
	bMaximized = bMaximized ? 1 : 0;
	if ( pWindow->bMaximized == bMaximized ) {
		return;
	}
	if ( bMaximized != 0 ) {
		pWindow->tRestoreRect = pWindow->pWidget->tRect;
		pWindow->bMaximized = 1;
		tRect = __xgeXuiWindowParentRect(pWindow);
		__xgeXuiWindowSyncButtonText(pWindow);
		__xgeXuiWindowApplyRect(pWindow, tRect);
	} else {
		pWindow->bMaximized = 0;
		__xgeXuiWindowSyncButtonText(pWindow);
		if ( pWindow->tRestoreRect.fW > 0.0f && pWindow->tRestoreRect.fH > 0.0f ) {
			__xgeXuiWindowApplyRect(pWindow, pWindow->tRestoreRect);
		}
	}
	xgeXuiWidgetMarkLayout(pWindow->pWidget);
	xgeXuiWidgetMarkPaint(pWindow->pWidget);
}

int xgeXuiWindowIsMaximized(xge_xui_window pWindow)
{
	return (pWindow != NULL) ? pWindow->bMaximized : 0;
}

void xgeXuiWindowSetChrome(xge_xui_window pWindow, float fTitleBarHeight, float fBorderWidth, float fResizeGrip, float fButtonSize)
{
	if ( pWindow == NULL ) {
		return;
	}
	if ( fTitleBarHeight >= 20.0f ) {
		pWindow->fTitleBarHeight = fTitleBarHeight;
	}
	if ( fBorderWidth >= 0.0f ) {
		pWindow->fBorderWidth = fBorderWidth;
	}
	if ( fResizeGrip >= 2.0f ) {
		pWindow->fResizeGrip = fResizeGrip;
	}
	if ( fButtonSize >= 12.0f ) {
		pWindow->fButtonSize = fButtonSize;
	}
	xgeXuiWidgetMarkLayout(pWindow->pWidget);
	xgeXuiWidgetMarkPaint(pWindow->pWidget);
}

void xgeXuiWindowSetColors(xge_xui_window pWindow, uint32_t iBackground, uint32_t iTitleBar, uint32_t iTitleText, uint32_t iBorder, uint32_t iButtonNormal, uint32_t iButtonHover, uint32_t iButtonActive)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->iBackgroundColor = iBackground;
	pWindow->iTitleBarColor = iTitleBar;
	pWindow->iTitleTextColor = iTitleText;
	pWindow->iBorderColor = iBorder;
	pWindow->iButtonColorNormal = iButtonNormal;
	pWindow->iButtonColorHover = iButtonHover;
	pWindow->iButtonColorActive = iButtonActive;
	__xgeXuiWindowSyncButtonColors(pWindow);
	xgeXuiWidgetMarkPaint(pWindow->pWidget);
}

int xgeXuiWindowEvent(xge_xui_window pWindow, const xge_event_t* pEvent)
{
	xge_xui_widget pHit;
	int iResizeEdges;

	if ( (pWindow == NULL) || (pWindow->pWidget == NULL) || (pEvent == NULL) || (pWindow->bOpen == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( __xgeXuiRectContains(pWindow->pWidget->tRect, pEvent->fX, pEvent->fY) == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiWidgetSetZ(pWindow->pWidget, ++g_iXgeXuiWindowRaiseZ);
			pHit = xgeXuiHitTest(pWindow->pContext, pEvent->fX, pEvent->fY);
			iResizeEdges = __xgeXuiWindowResizeEdgesAt(pWindow, pEvent->fX, pEvent->fY);
			if ( iResizeEdges != 0 ) {
				pWindow->iInteractionEdges = iResizeEdges;
			} else if ( __xgeXuiWindowCanStartMove(pWindow, pHit, pEvent->fX, pEvent->fY) ) {
				pWindow->iInteractionEdges = -1;
			}
			if ( pWindow->iInteractionEdges == 0 ) {
				if ( pHit == pWindow->pWidget ) {
					xgeXuiSetFocus(pWindow->pContext, pWindow->pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
				return XGE_XUI_EVENT_CONTINUE;
			}
			pWindow->fDragStartX = pEvent->fX;
			pWindow->fDragStartY = pEvent->fY;
			pWindow->tDragStartRect = pWindow->pWidget->tRect;
			pWindow->tPreviewRect = pWindow->tDragStartRect;
			pWindow->bPreviewActive = 1;
			xgeXuiSetFocus(pWindow->pContext, pWindow->pWidget);
			xgeXuiSetCapture(pWindow->pContext, pWindow->pWidget);
			xgeXuiRefreshRequest(pWindow->pContext);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( (pWindow->pContext != NULL) && (pWindow->pContext->pCapture == pWindow->pWidget) && (pWindow->iInteractionEdges != 0) ) {
				__xgeXuiWindowBuildPreviewRect(pWindow, pEvent->fX, pEvent->fY);
				xgeXuiRefreshRequest(pWindow->pContext);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			if ( pWindow->iInteractionEdges != 0 ) {
				__xgeXuiWindowCommitPreview(pWindow);
				pWindow->iInteractionEdges = 0;
				if ( (pWindow->pContext != NULL) && (pWindow->pContext->pCapture == pWindow->pWidget) ) {
					xgeXuiSetCapture(pWindow->pContext, NULL);
				}
				xgeXuiRefreshRequest(pWindow->pContext);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
			if ( pWindow->iInteractionEdges != 0 ) {
				__xgeXuiWindowCancelPreview(pWindow);
				pWindow->iInteractionEdges = 0;
				if ( (pWindow->pContext != NULL) && (pWindow->pContext->pCapture == pWindow->pWidget) ) {
					xgeXuiSetCapture(pWindow->pContext, NULL);
				}
				xgeXuiRefreshRequest(pWindow->pContext);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiWindowEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiWindowEvent((xge_xui_window)pUser, pEvent);
}

void xgeXuiWindowPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_window pWindow;
	xge_rect_t tOuter;
	xge_rect_t tInner;
	xge_rect_t tTitle;
	xge_rect_t tIconRect;
	xge_rect_t tTextRect;
	xge_draw_t tDraw;

	pWindow = (xge_xui_window)pUser;
	if ( (pWidget == NULL) || (pWindow == NULL) || (pWindow->bOpen == 0) ) {
		return;
	}
	tOuter = pWidget->tRect;
	tInner = tOuter;
	if ( pWindow->fBorderWidth > 0.0f ) {
		__xgeXuiWindowDrawBorder(tOuter, pWindow->iBorderColor, pWindow->fBorderWidth);
		tInner.fX += pWindow->fBorderWidth;
		tInner.fY += pWindow->fBorderWidth;
		tInner.fW -= pWindow->fBorderWidth * 2.0f;
		tInner.fH -= pWindow->fBorderWidth * 2.0f;
	}
	if ( tInner.fW < 0.0f ) {
		tInner.fW = 0.0f;
	}
	if ( tInner.fH < 0.0f ) {
		tInner.fH = 0.0f;
	}
	__xgeXuiHostDrawRect(tInner, pWindow->iBackgroundColor);
	if ( pWindow->bShowTitleBar != 0 ) {
		tTitle = tInner;
		tTitle.fH = pWindow->fTitleBarHeight;
		__xgeXuiHostDrawRect(tTitle, pWindow->iTitleBarColor);
		__xgeXuiHostDrawRect((xge_rect_t){ tTitle.fX, tTitle.fY + tTitle.fH - 1.0f, tTitle.fW, 1.0f }, pWindow->iBorderColor);
		tTextRect = tTitle;
		tTextRect.fX += 8.0f;
		tTextRect.fW -= 16.0f;
		if ( pWindow->pIconTexture != NULL ) {
			memset(&tDraw, 0, sizeof(tDraw));
			tIconRect.fW = pWindow->fIconSize;
			tIconRect.fH = pWindow->fIconSize;
			tIconRect.fX = tTitle.fX + 8.0f;
			tIconRect.fY = tTitle.fY + (tTitle.fH - tIconRect.fH) * 0.5f;
			tDraw.pTexture = pWindow->pIconTexture;
			tDraw.tSrc = pWindow->tIconSrc;
			tDraw.tDst = tIconRect;
			tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
			tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
			__xgeXuiHostDrawImage(&tDraw);
			tTextRect.fX = tIconRect.fX + tIconRect.fW + 8.0f;
			tTextRect.fW -= pWindow->fIconSize + 8.0f;
		}
		if ( (pWindow->pFont != NULL) && (pWindow->sTitle != NULL) && (pWindow->sTitle[0] != 0) ) {
			__xgeXuiHostDrawTextRect(pWindow->pFont, pWindow->sTitle, tTextRect, pWindow->iTitleTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
}

static void __xgeXuiWindowButtonPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_button pButton;
	xge_xui_window pWindow;
	xge_rect_t tRect;
	xge_rect_t tIcon;
	uint32_t iBack;
	uint32_t iIcon;
	int iKind;

	pButton = (xge_xui_button)pUser;
	if ( (pWidget == NULL) || (pButton == NULL) ) {
		return;
	}
	pWindow = (xge_xui_window)pButton->pUser;
	if ( pWindow == NULL ) {
		return;
	}
	iKind = 0;
	if ( pWidget == pWindow->pMaximizeButtonWidget ) {
		iKind = 1;
	} else if ( pWidget == pWindow->pCloseButtonWidget ) {
		iKind = 2;
	}
	tRect = pWidget->tRect;
	iBack = (iKind == 2) ? pWindow->iButtonColorNormal : XGE_COLOR_RGBA(255, 255, 255, 0);
	iIcon = pWindow->iTitleTextColor;
	if ( (pButton->iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		iBack = (iKind == 2) ? pWindow->iButtonColorActive : XGE_COLOR_RGBA(184, 223, 245, 220);
	} else if ( (pButton->iState & XGE_XUI_STATE_HOVER) != 0 ) {
		iBack = (iKind == 2) ? pWindow->iButtonColorHover : XGE_COLOR_RGBA(218, 239, 252, 220);
	}
	if ( XGE_COLOR_GET_A(iBack) != 0 ) {
		__xgeXuiHostDrawRect(tRect, iBack);
		__xgeXuiHostDrawBorderRect(tRect, 1.0f, XGE_COLOR_RGBA(127, 196, 229, 255));
	}
	if ( iKind == 2 ) {
		iIcon = XGE_COLOR_RGBA(170, 72, 76, 255);
	}
	tIcon.fW = 10.0f;
	tIcon.fH = 10.0f;
	tIcon.fX = tRect.fX + (tRect.fW - tIcon.fW) * 0.5f;
	tIcon.fY = tRect.fY + (tRect.fH - tIcon.fH) * 0.5f;
	if ( iKind == 0 ) {
		if ( pWindow->bCollapsed != 0 ) {
			xgeShapeLinePx(tIcon.fX + 1.0f, tIcon.fY + 6.0f, tIcon.fX + 5.0f, tIcon.fY + 2.0f, 1.2f, iIcon);
			xgeShapeLinePx(tIcon.fX + 5.0f, tIcon.fY + 2.0f, tIcon.fX + 9.0f, tIcon.fY + 6.0f, 1.2f, iIcon);
		} else {
			xgeShapeLinePx(tIcon.fX + 1.0f, tIcon.fY + 4.0f, tIcon.fX + 5.0f, tIcon.fY + 8.0f, 1.2f, iIcon);
			xgeShapeLinePx(tIcon.fX + 5.0f, tIcon.fY + 8.0f, tIcon.fX + 9.0f, tIcon.fY + 4.0f, 1.2f, iIcon);
		}
	} else if ( iKind == 1 ) {
		if ( pWindow->bMaximized != 0 ) {
			xgeShapeRectStrokePx((xge_rect_t){ tIcon.fX + 1.0f, tIcon.fY + 4.0f, 6.0f, 5.0f }, 1.0f, iIcon);
			xgeShapeRectStrokePx((xge_rect_t){ tIcon.fX + 3.0f, tIcon.fY + 1.0f, 6.0f, 5.0f }, 1.0f, iIcon);
		} else {
			xgeShapeRectStrokePx((xge_rect_t){ tIcon.fX + 1.0f, tIcon.fY + 1.0f, 8.0f, 8.0f }, 1.0f, iIcon);
		}
	} else {
		xgeShapeLinePx(tIcon.fX + 1.0f, tIcon.fY + 1.0f, tIcon.fX + 9.0f, tIcon.fY + 9.0f, 1.3f, iIcon);
		xgeShapeLinePx(tIcon.fX + 9.0f, tIcon.fY + 1.0f, tIcon.fX + 1.0f, tIcon.fY + 9.0f, 1.3f, iIcon);
	}
}

static void __xgeXuiWindowPaintAfter(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_window pWindow;

	(void)pWidget;
	pWindow = (xge_xui_window)pUser;
	if ( (pWindow == NULL) || (pWindow->bOpen == 0) || (pWindow->bPreviewActive == 0) ) {
		return;
	}
	__xgeXuiWindowDrawBorder(pWindow->tPreviewRect, XGE_COLOR_RGBA(116, 184, 255, 220), 2.0f);
}
