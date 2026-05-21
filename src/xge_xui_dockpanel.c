int xgeXuiDockLayoutInit(xge_xui_dock_layout pLayout, xge_xui_context pContext, xge_xui_widget pWidget);
void xgeXuiDockLayoutUnit(xge_xui_dock_layout pLayout);
xge_xui_widget xgeXuiDockLayoutWidget(xge_xui_dock_layout pLayout);
xge_xui_dock_pane xgeXuiDockLayoutDockWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow, int iRegion, int iSide, float fProportion);
int xgeXuiDockLayoutFloatWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow, xge_rect_t tRect);
int xgeXuiDockLayoutHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
int xgeXuiDockLayoutAutoHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
int xgeXuiDockLayoutDockAutoHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
void xgeXuiDockLayoutSetRegionPortion(xge_xui_dock_layout pLayout, int iRegion, float fPortion);
xvalue xgeXuiDockLayoutSaveState(const xge_xui_dock_layout pLayout);
int xgeXuiDockLayoutLoadState(xge_xui_dock_layout pLayout, xvalue pState);
void xgeXuiDockLayoutStateFree(xvalue pState);
int xgeXuiDockLayoutStateGetCounts(xvalue pState, int* pRegionCount, int* pWindowCount, int* pFloatingCount);
int xgeXuiDockWindowInit(xge_xui_dock_window pWindow, xge_xui_context pContext);
void xgeXuiDockWindowUnit(xge_xui_dock_window pWindow);
xge_xui_window xgeXuiDockWindowBaseWindow(xge_xui_dock_window pWindow);
xge_xui_widget xgeXuiDockWindowClientWidget(xge_xui_dock_window pWindow);
void xgeXuiDockWindowSetClientWidget(xge_xui_dock_window pWindow, xge_xui_widget pClient);
void xgeXuiDockWindowSetTitle(xge_xui_dock_window pWindow, const char* sTitle);
void xgeXuiDockWindowSetIcon(xge_xui_dock_window pWindow, xui_texture pTexture, xge_rect_t tSrc);
void xgeXuiDockWindowSetClosable(xge_xui_dock_window pWindow, int bClosable);
void xgeXuiDockWindowSetDockable(xge_xui_dock_window pWindow, int bDockable);
int xgeXuiDockWindowGetState(const xge_xui_dock_window pWindow);
int xgeXuiDockPaneGetWindowCount(const xge_xui_dock_pane pPane);
xge_xui_dock_window xgeXuiDockPaneGetWindow(const xge_xui_dock_pane pPane, int iIndex);
xge_xui_dock_window xgeXuiDockPaneGetActiveWindow(const xge_xui_dock_pane pPane);
void xgeXuiDockPaneSetActiveIndex(xge_xui_dock_pane pPane, int iIndex);
int xgeXuiDockPaneGetActiveIndex(const xge_xui_dock_pane pPane);

#define XGE_XUI_DOCK_PART_NONE			(-1)
#define XGE_XUI_DOCK_PART_CLOSE		(-2)
#define XGE_XUI_DOCK_PART_AUTO_HIDE		(-3)
#define XGE_XUI_DOCK_PART_OPTION		(-4)
#define XGE_XUI_DOCK_PART_CAPTION		(-5)
#define XGE_XUI_DOCK_PART_OVERFLOW		(-6)

#define XGE_XUI_DOCK_OPTION_FLOAT		1
#define XGE_XUI_DOCK_OPTION_DOCK		2
#define XGE_XUI_DOCK_OPTION_CLOSE		3
#define XGE_XUI_DOCK_OPTION_CLOSE_OTHERS	4
#define XGE_XUI_DOCK_OPTION_CLOSE_ALL		5

static xge_vec2_t __xgeXuiDockLayoutMeasureProc(xge_xui_widget pWidget, void* pUser);
static void __xgeXuiDockLayoutLayoutProc(xge_xui_widget pWidget, void* pUser);
static void __xgeXuiDockLayoutPaintProc(xge_xui_widget pWidget, void* pUser);
static int __xgeXuiDockLayoutEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
static void __xgeXuiDockLayoutOverlayPaintProc(xge_xui_widget pWidget, void* pUser);
static void __xgeXuiDockLayoutAutoHideOverlayPaintProc(xge_xui_widget pWidget, void* pUser);
static int __xgeXuiDockLayoutAutoHideOverlayEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
static int __xgeXuiDockWindowEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
static void __xgeXuiDockWindowAttachClientToBase(xge_xui_dock_window pWindow);
static void __xgeXuiDockWindowAttachClientToLayout(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
static void __xgeXuiDockWindowAttachClientToAutoHideOverlay(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
static void __xgeXuiDockLayoutRequestFocusWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
static void __xgeXuiDockLayoutSyncAutoHideOverlayRect(xge_xui_dock_layout pLayout);
static void __xgeXuiDockLayoutArrangeAutoHideExpand(xge_xui_dock_layout pLayout);
static int __xgeXuiDockLayoutOpenAutoHideExpand(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow);
static void __xgeXuiDockLayoutCloseAutoHideExpand(xge_xui_dock_layout pLayout);
static void __xgeXuiDockLayoutCloseOptionMenu(xge_xui_dock_layout pLayout);
static void __xgeXuiDockLayoutOptionMenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser);
static void __xgeXuiDockLayoutCloseOverflowMenu(xge_xui_dock_layout pLayout);
static void __xgeXuiDockLayoutOverflowMenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser);
static int __xgeXuiDockPaneCanAutoHide(xge_xui_dock_pane pPane);

static int __xgeXuiDockRegionValid(int iRegion)
{
	return (iRegion >= 0) && (iRegion < XGE_XUI_DOCK_REGION_COUNT);
}

static int __xgeXuiDockSideIsSplit(int iSide)
{
	return (iSide == XGE_XUI_DOCK_SIDE_LEFT) || (iSide == XGE_XUI_DOCK_SIDE_RIGHT) || (iSide == XGE_XUI_DOCK_SIDE_TOP) || (iSide == XGE_XUI_DOCK_SIDE_BOTTOM);
}

static int __xgeXuiDockSideToRegion(int iSide)
{
	switch ( iSide ) {
		case XGE_XUI_DOCK_SIDE_LEFT: return XGE_XUI_DOCK_REGION_LEFT;
		case XGE_XUI_DOCK_SIDE_RIGHT: return XGE_XUI_DOCK_REGION_RIGHT;
		case XGE_XUI_DOCK_SIDE_TOP: return XGE_XUI_DOCK_REGION_TOP;
		case XGE_XUI_DOCK_SIDE_BOTTOM: return XGE_XUI_DOCK_REGION_BOTTOM;
		default: return XGE_XUI_DOCK_REGION_DOCUMENT;
	}
}

static float __xgeXuiDockClampFloat(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

static float __xgeXuiDockLayoutTabStripHeight(xge_xui_dock_layout pLayout)
{
	return (pLayout != NULL && pLayout->fTabStripHeight > 0.0f) ? pLayout->fTabStripHeight : 25.0f;
}

static float __xgeXuiDockLayoutCaptionHeight(xge_xui_dock_layout pLayout)
{
	return (pLayout != NULL && pLayout->fCaptionHeight > 0.0f) ? pLayout->fCaptionHeight : 25.0f;
}

static float __xgeXuiDockLayoutButtonWidth(xge_xui_dock_layout pLayout)
{
	return (pLayout != NULL && pLayout->fButtonWidth > 0.0f) ? pLayout->fButtonWidth : 16.0f;
}

static float __xgeXuiDockLayoutButtonHeight(xge_xui_dock_layout pLayout)
{
	return (pLayout != NULL && pLayout->fButtonHeight > 0.0f) ? pLayout->fButtonHeight : 15.0f;
}

static float __xgeXuiDockButtonTop(float fChromeH, float fButtonH)
{
	return (fChromeH > fButtonH) ? (fChromeH - fButtonH) * 0.5f : 0.0f;
}

static xge_rect_t __xgeXuiDockRectZero(void)
{
	return (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
}

static int __xgeXuiDockRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static xge_rect_t __xgeXuiDockRectInset(xge_rect_t tRect, float fLeft, float fTop, float fRight, float fBottom)
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

static void __xgeXuiDockWidgetArrange(xge_xui_widget pWidget, xge_rect_t tRect)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tLocalRect = tRect;
	__xgeXuiWidgetArrangeRect(pWidget, tRect);
}

static int __xgeXuiDockArrayFindWindow(xarray pArr, xge_xui_dock_window pWindow)
{
	uint32 i;

	if ( (pArr == NULL) || (pWindow == NULL) ) {
		return -1;
	}
	for ( i = 0; i < pArr->Count; i++ ) {
		xge_xui_dock_window* ppItem = (xge_xui_dock_window*)xrtArrayGet_Inline(pArr, i + 1u);
		if ( (ppItem != NULL) && (*ppItem == pWindow) ) {
			return (int)i;
		}
	}
	return -1;
}

static int __xgeXuiDockArrayAppendWindowUnique(xarray pArr, xge_xui_dock_window pWindow)
{
	uint32 iIndex;
	xge_xui_dock_window* ppItem;

	if ( (pArr == NULL) || (pWindow == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiDockArrayFindWindow(pArr, pWindow) >= 0 ) {
		return XGE_OK;
	}
	iIndex = xrtArrayAppend(pArr, 1u);
	if ( iIndex == 0u ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	ppItem = (xge_xui_dock_window*)xrtArrayGet_Inline(pArr, iIndex);
	*ppItem = pWindow;
	return XGE_OK;
}

static void __xgeXuiDockArrayRemoveWindow(xarray pArr, xge_xui_dock_window pWindow)
{
	int iIndex;

	if ( (pArr == NULL) || (pWindow == NULL) ) {
		return;
	}
	iIndex = __xgeXuiDockArrayFindWindow(pArr, pWindow);
	if ( iIndex >= 0 ) {
		xrtArrayRemove(pArr, (uint32)iIndex + 1u, 1u);
	}
}

static int __xgeXuiDockArrayMoveWindow(xarray pArr, int iFrom, int iInsert)
{
	xge_xui_dock_window tWindow;
	xge_xui_dock_window* ppItem;
	uint32 iInserted;
	int iCount;

	if ( pArr == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = (int)pArr->Count;
	if ( (iFrom < 0) || (iFrom >= iCount) || (iInsert < 0) || (iInsert > iCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (iInsert == iFrom) || (iInsert == iFrom + 1) ) {
		return XGE_OK;
	}
	ppItem = (xge_xui_dock_window*)xrtArrayGet_Inline(pArr, (uint32)iFrom + 1u);
	tWindow = (ppItem != NULL) ? *ppItem : NULL;
	if ( tWindow == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iInsert > iFrom ) {
		iInsert--;
	}
	if ( !xrtArrayRemove(pArr, (uint32)iFrom + 1u, 1u) ) {
		return XGE_ERROR;
	}
	iInserted = xrtArrayInsert(pArr, (uint32)iInsert, 1u);
	if ( iInserted == 0u ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	ppItem = (xge_xui_dock_window*)xrtArrayGet_Inline(pArr, iInserted);
	*ppItem = tWindow;
	return XGE_OK;
}

static xge_xui_dock_pane __xgeXuiDockPaneCreate(xge_xui_dock_layout pLayout)
{
	xge_xui_dock_pane pPane;

	pPane = (xge_xui_dock_pane)xrtMalloc(sizeof(*pPane));
	if ( pPane == NULL ) {
		return NULL;
	}
	memset(pPane, 0, sizeof(*pPane));
	pPane->pLayout = pLayout;
	pPane->iActive = -1;
	pPane->iHoverPart = XGE_XUI_DOCK_PART_NONE;
	pPane->iActivePart = XGE_XUI_DOCK_PART_NONE;
	xrtArrayInit(&pPane->arrWindows, sizeof(xge_xui_dock_window), XRT_OBJMODE_LOCAL);
	return pPane;
}

static void __xgeXuiDockPaneDestroy(xge_xui_dock_pane pPane)
{
	uint32 i;

	if ( pPane == NULL ) {
		return;
	}
	for ( i = 0; i < pPane->arrWindows.Count; i++ ) {
		xge_xui_dock_window* ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline(&pPane->arrWindows, i + 1u);
		if ( (ppWindow != NULL) && (*ppWindow != NULL) && ((*ppWindow)->pPane == pPane) ) {
			(*ppWindow)->pPane = NULL;
		}
	}
	xrtArrayUnit(&pPane->arrWindows);
	memset(pPane, 0, sizeof(*pPane));
	xrtFree(pPane);
}

static xge_xui_dock_node __xgeXuiDockNodeCreatePane(xge_xui_dock_layout pLayout)
{
	xge_xui_dock_node pNode;
	xge_xui_dock_pane pPane;

	pPane = __xgeXuiDockPaneCreate(pLayout);
	if ( pPane == NULL ) {
		return NULL;
	}
	pNode = (xge_xui_dock_node)xrtMalloc(sizeof(*pNode));
	if ( pNode == NULL ) {
		__xgeXuiDockPaneDestroy(pPane);
		return NULL;
	}
	memset(pNode, 0, sizeof(*pNode));
	pNode->iType = XGE_XUI_DOCK_NODE_PANE;
	pNode->fRatio = 0.5f;
	pNode->fMinWidth = 148.0f;
	pNode->fMinHeight = 72.0f;
	pNode->pPane = pPane;
	pPane->pNode = pNode;
	return pNode;
}

static xge_xui_dock_node __xgeXuiDockNodeCreateSplit(xge_xui_dock_node pFirst, xge_xui_dock_node pSecond, int iAxis, float fRatio)
{
	xge_xui_dock_node pNode;

	if ( (pFirst == NULL) || (pSecond == NULL) ) {
		return NULL;
	}
	pNode = (xge_xui_dock_node)xrtMalloc(sizeof(*pNode));
	if ( pNode == NULL ) {
		return NULL;
	}
	memset(pNode, 0, sizeof(*pNode));
	pNode->iType = XGE_XUI_DOCK_NODE_SPLIT;
	pNode->iAxis = iAxis;
	pNode->fRatio = __xgeXuiDockClampFloat(fRatio, 0.10f, 0.90f);
	pNode->fMinWidth = 48.0f;
	pNode->fMinHeight = 36.0f;
	pNode->pFirst = pFirst;
	pNode->pSecond = pSecond;
	pFirst->pParent = pNode;
	pSecond->pParent = pNode;
	return pNode;
}

static void __xgeXuiDockNodeDestroy(xge_xui_dock_node pNode)
{
	if ( pNode == NULL ) {
		return;
	}
	__xgeXuiDockNodeDestroy(pNode->pFirst);
	__xgeXuiDockNodeDestroy(pNode->pSecond);
	__xgeXuiDockPaneDestroy(pNode->pPane);
	memset(pNode, 0, sizeof(*pNode));
	xrtFree(pNode);
}

static xge_xui_dock_pane __xgeXuiDockNodeFirstPane(xge_xui_dock_node pNode)
{
	xge_xui_dock_pane pPane;

	if ( pNode == NULL ) {
		return NULL;
	}
	if ( pNode->iType == XGE_XUI_DOCK_NODE_PANE ) {
		return pNode->pPane;
	}
	pPane = __xgeXuiDockNodeFirstPane(pNode->pFirst);
	return (pPane != NULL) ? pPane : __xgeXuiDockNodeFirstPane(pNode->pSecond);
}

static xge_xui_dock_pane __xgeXuiDockLayoutEnsureRegionPane(xge_xui_dock_layout pLayout, int iRegion)
{
	xge_xui_dock_region pRegion;

	if ( (pLayout == NULL) || !__xgeXuiDockRegionValid(iRegion) ) {
		return NULL;
	}
	pRegion = &pLayout->arrRegions[iRegion];
	if ( pRegion->pRoot == NULL ) {
		pRegion->pRoot = __xgeXuiDockNodeCreatePane(pLayout);
		if ( pRegion->pRoot == NULL ) {
			return NULL;
		}
		pRegion->pRoot->pParent = NULL;
		pRegion->bVisible = 1;
	}
	return __xgeXuiDockNodeFirstPane(pRegion->pRoot);
}

static xge_xui_dock_region __xgeXuiDockLayoutFindNodeRegion(xge_xui_dock_layout pLayout, xge_xui_dock_node pNode)
{
	xge_xui_dock_node pRoot;
	int i;

	if ( (pLayout == NULL) || (pNode == NULL) ) {
		return NULL;
	}
	pRoot = pNode;
	while ( pRoot->pParent != NULL ) {
		pRoot = pRoot->pParent;
	}
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		if ( pLayout->arrRegions[i].pRoot == pRoot ) {
			return &pLayout->arrRegions[i];
		}
	}
	return NULL;
}

static void __xgeXuiDockPaneCollapseIfEmpty(xge_xui_dock_pane pPane)
{
	xge_xui_dock_layout pLayout;
	xge_xui_dock_region pRegion;
	xge_xui_dock_node pNode;
	xge_xui_dock_node pSplit;
	xge_xui_dock_node pSibling;
	xge_xui_dock_node pGrand;

	if ( (pPane == NULL) || (pPane->arrWindows.Count != 0u) || (pPane->pNode == NULL) ) {
		return;
	}
	pLayout = pPane->pLayout;
	pNode = pPane->pNode;
	pRegion = __xgeXuiDockLayoutFindNodeRegion(pLayout, pNode);
	if ( pRegion == NULL ) {
		return;
	}
	if ( pNode->pParent == NULL ) {
		if ( pRegion->iKind != XGE_XUI_DOCK_REGION_DOCUMENT ) {
			__xgeXuiDockNodeDestroy(pRegion->pRoot);
			pRegion->pRoot = NULL;
			pRegion->bVisible = 0;
		}
		return;
	}
	pSplit = pNode->pParent;
	pSibling = (pSplit->pFirst == pNode) ? pSplit->pSecond : pSplit->pFirst;
	pGrand = pSplit->pParent;
	if ( pSibling == NULL ) {
		return;
	}
	pSibling->pParent = pGrand;
	if ( pGrand == NULL ) {
		pRegion->pRoot = pSibling;
	} else if ( pGrand->pFirst == pSplit ) {
		pGrand->pFirst = pSibling;
	} else if ( pGrand->pSecond == pSplit ) {
		pGrand->pSecond = pSibling;
	}
	pSplit->pFirst = NULL;
	pSplit->pSecond = NULL;
	pNode->pParent = NULL;
	__xgeXuiDockNodeDestroy(pNode);
	memset(pSplit, 0, sizeof(*pSplit));
	xrtFree(pSplit);
}

static void __xgeXuiDockPaneRemoveWindow(xge_xui_dock_pane pPane, xge_xui_dock_window pWindow)
{
	int iIndex;
	int iCount;

	if ( (pPane == NULL) || (pWindow == NULL) ) {
		return;
	}
	iIndex = __xgeXuiDockArrayFindWindow(&pPane->arrWindows, pWindow);
	if ( iIndex < 0 ) {
		return;
	}
	xrtArrayRemove(&pPane->arrWindows, (uint32)iIndex + 1u, 1u);
	iCount = (int)pPane->arrWindows.Count;
	if ( iCount <= 0 ) {
		pPane->iActive = -1;
	} else if ( pPane->iActive >= iCount ) {
		pPane->iActive = iCount - 1;
	} else if ( pPane->iActive > iIndex ) {
		pPane->iActive--;
	}
	if ( pWindow->pPane == pPane ) {
		pWindow->pPane = NULL;
	}
	__xgeXuiDockPaneCollapseIfEmpty(pPane);
}

static int __xgeXuiDockPaneAddWindow(xge_xui_dock_pane pPane, xge_xui_dock_window pWindow)
{
	int iIndex;
	int iRet;

	if ( (pPane == NULL) || (pWindow == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = __xgeXuiDockArrayFindWindow(&pPane->arrWindows, pWindow);
	if ( iIndex < 0 ) {
		iRet = __xgeXuiDockArrayAppendWindowUnique(&pPane->arrWindows, pWindow);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		iIndex = (int)pPane->arrWindows.Count - 1;
	}
	pPane->iActive = iIndex;
	pWindow->pPane = pPane;
	return XGE_OK;
}

static int __xgeXuiDockLayoutRegisterWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow)
{
	int iRet;

	if ( (pLayout == NULL) || (pWindow == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pWindow->pLayout != NULL) && (pWindow->pLayout != pLayout) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xgeXuiDockArrayAppendWindowUnique(&pLayout->arrWindows, pWindow);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pWindow->pLayout = pLayout;
	return XGE_OK;
}

static void __xgeXuiDockLayoutRemoveFloatingWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow)
{
	if ( (pLayout == NULL) || (pWindow == NULL) ) {
		return;
	}
	__xgeXuiDockArrayRemoveWindow(&pLayout->arrFloatingWindows, pWindow);
}

static void __xgeXuiDockWindowDetachFromPane(xge_xui_dock_window pWindow)
{
	if ( (pWindow == NULL) || (pWindow->pPane == NULL) ) {
		return;
	}
	__xgeXuiDockPaneRemoveWindow(pWindow->pPane, pWindow);
}

static void __xgeXuiDockLayoutUnregisterWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow)
{
	if ( (pLayout == NULL) || (pWindow == NULL) ) {
		return;
	}
	if ( pLayout->pAutoHideExpandWindow == pWindow ) {
		__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
	}
	__xgeXuiDockWindowDetachFromPane(pWindow);
	__xgeXuiDockLayoutRemoveFloatingWindow(pLayout, pWindow);
	__xgeXuiDockArrayRemoveWindow(&pLayout->arrWindows, pWindow);
	__xgeXuiDockWindowAttachClientToBase(pWindow);
	if ( pWindow->pLayout == pLayout ) {
		pWindow->pLayout = NULL;
	}
	pWindow->pPane = NULL;
}

static float __xgeXuiDockClampPortion(float fPortion)
{
	if ( fPortion <= 0.0f ) {
		return 0.22f;
	}
	if ( fPortion < 0.05f ) {
		return 0.05f;
	}
	if ( fPortion > 0.80f ) {
		return 0.80f;
	}
	return fPortion;
}

static float __xgeXuiDockClampSplitRatio(float fRatio)
{
	if ( fRatio <= 0.0f ) {
		return 0.50f;
	}
	return __xgeXuiDockClampFloat(fRatio, 0.10f, 0.90f);
}

static int __xgeXuiDockRegionActive(xge_xui_dock_region pRegion)
{
	if ( pRegion == NULL ) {
		return 0;
	}
	return (pRegion->pRoot != NULL) && (pRegion->bVisible != 0 || pRegion->iKind == XGE_XUI_DOCK_REGION_DOCUMENT);
}

static float __xgeXuiDockRegionSideSize(xge_xui_dock_region pRegion, float fAvailable, float fBasis)
{
	float fSize;
	float fMinSize;

	if ( !__xgeXuiDockRegionActive(pRegion) || (fAvailable <= 0.0f) || (fBasis <= 0.0f) ) {
		return 0.0f;
	}
	fSize = fBasis * __xgeXuiDockClampPortion(pRegion->fPortion);
	fMinSize = pRegion->fMinSize;
	if ( pRegion->pRoot != NULL ) {
		if ( (pRegion->iKind == XGE_XUI_DOCK_REGION_LEFT || pRegion->iKind == XGE_XUI_DOCK_REGION_RIGHT) && pRegion->pRoot->fMinWidth > fMinSize ) {
			fMinSize = pRegion->pRoot->fMinWidth;
		} else if ( (pRegion->iKind == XGE_XUI_DOCK_REGION_TOP || pRegion->iKind == XGE_XUI_DOCK_REGION_BOTTOM) && pRegion->pRoot->fMinHeight > fMinSize ) {
			fMinSize = pRegion->pRoot->fMinHeight;
		}
	}
	if ( (fMinSize > 0.0f) && (fSize < fMinSize) ) {
		fSize = fMinSize;
	}
	if ( (pRegion->fMaxSize > 0.0f) && (fSize > pRegion->fMaxSize) ) {
		fSize = pRegion->fMaxSize;
	}
	if ( fSize > fAvailable ) {
		fSize = fAvailable;
	}
	if ( fSize < 0.0f ) {
		fSize = 0.0f;
	}
	return fSize;
}

static void __xgeXuiDockWindowAttachClientToBase(xge_xui_dock_window pWindow)
{
	if ( (pWindow == NULL) || (pWindow->pClientWidget == NULL) || (pWindow->pWindowWidget == NULL) ) {
		return;
	}
	if ( pWindow->pClientWidget->pParent != pWindow->pWindowWidget ) {
		xgeXuiWidgetAddInternal(pWindow->pWindowWidget, pWindow->pClientWidget);
	}
	xgeXuiWidgetSetVisible(pWindow->pClientWidget, 1);
}

static void __xgeXuiDockWindowAttachClientToLayout(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow)
{
	if ( (pLayout == NULL) || (pWindow == NULL) || (pLayout->pWidget == NULL) || (pWindow->pClientWidget == NULL) ) {
		return;
	}
	if ( pWindow->pClientWidget->pParent != pLayout->pWidget ) {
		xgeXuiWidgetAddInternal(pLayout->pWidget, pWindow->pClientWidget);
	}
	xgeXuiWidgetSetVisible(pWindow->pClientWidget, 1);
}

static void __xgeXuiDockWindowAttachClientToAutoHideOverlay(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow)
{
	if ( (pLayout == NULL) || (pWindow == NULL) || (pLayout->pAutoHideOverlayWidget == NULL) || (pWindow->pClientWidget == NULL) ) {
		return;
	}
	if ( pWindow->pClientWidget->pParent != pLayout->pAutoHideOverlayWidget ) {
		xgeXuiWidgetAddInternal(pLayout->pAutoHideOverlayWidget, pWindow->pClientWidget);
	}
	xgeXuiWidgetSetVisible(pWindow->pClientWidget, 1);
}

static int __xgeXuiDockWindowFocusNow(xge_xui_dock_window pWindow)
{
	xge_xui_widget pTarget;

	if ( (pWindow == NULL) || (pWindow->pContext == NULL) ) {
		return 0;
	}
	pTarget = pWindow->pContentWidget;
	if ( (pTarget == NULL) || (xgeXuiWidgetIsFocusable(pTarget) == 0) ) {
		pTarget = pWindow->pClientWidget;
	}
	if ( (pTarget == NULL) || (xgeXuiWidgetIsFocusable(pTarget) == 0) ) {
		pTarget = pWindow->pWindowWidget;
	}
	if ( (pTarget == NULL) || (xgeXuiWidgetIsFocusable(pTarget) == 0) ) {
		return 0;
	}
	xgeXuiSetFocus(pWindow->pContext, pTarget);
	return (pWindow->pContext->pFocus == pTarget);
}

static void __xgeXuiDockLayoutRequestFocusWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow)
{
	if ( pLayout == NULL ) {
		return;
	}
	pLayout->pPendingFocusWindow = pWindow;
	if ( (pWindow != NULL) && (pWindow->iState != XGE_XUI_DOCK_WINDOW_DOCKED) && (__xgeXuiDockWindowFocusNow(pWindow) != 0) ) {
		pLayout->pPendingFocusWindow = NULL;
	}
}

static xge_rect_t __xgeXuiDockClampFloatRect(xge_xui_dock_layout pLayout, xge_rect_t tRect)
{
	xge_rect_t tRoot;

	if ( tRect.fW < 80.0f ) {
		tRect.fW = 80.0f;
	}
	if ( tRect.fH < 60.0f ) {
		tRect.fH = 60.0f;
	}
	if ( (pLayout == NULL) || (pLayout->pContext == NULL) || (pLayout->pContext->pRoot == NULL) ) {
		return tRect;
	}
	tRoot = pLayout->pContext->pRoot->tContentRect;
	if ( (tRoot.fW <= 0.0f) || (tRoot.fH <= 0.0f) ) {
		tRoot = pLayout->pContext->pRoot->tRect;
	}
	if ( tRoot.fW <= 0.0f || tRoot.fH <= 0.0f ) {
		return tRect;
	}
	if ( tRect.fW > tRoot.fW ) {
		tRect.fW = tRoot.fW;
	}
	if ( tRect.fH > tRoot.fH ) {
		tRect.fH = tRoot.fH;
	}
	tRect.fX = __xgeXuiDockClampFloat(tRect.fX, tRoot.fX, tRoot.fX + tRoot.fW - tRect.fW);
	tRect.fY = __xgeXuiDockClampFloat(tRect.fY, tRoot.fY, tRoot.fY + tRoot.fH - tRect.fH);
	return tRect;
}

static void __xgeXuiDockLayoutHideDockedClients(xge_xui_dock_layout pLayout)
{
	uint32 i;

	if ( pLayout == NULL ) {
		return;
	}
	for ( i = 0; i < pLayout->arrWindows.Count; i++ ) {
		xge_xui_dock_window* ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline(&pLayout->arrWindows, i + 1u);
		if ( (ppWindow != NULL) && (*ppWindow != NULL) && ((*ppWindow)->iState == XGE_XUI_DOCK_WINDOW_DOCKED) && ((*ppWindow)->pClientWidget != NULL) ) {
			xgeXuiWidgetSetVisible((*ppWindow)->pClientWidget, 0);
		}
	}
}

static xge_xui_dock_pane __xgeXuiDockLayoutSplitRegionRoot(xge_xui_dock_layout pLayout, int iRegion, int iSide, float fProportion)
{
	xge_xui_dock_region pRegion;
	xge_xui_dock_node pOldRoot;
	xge_xui_dock_node pNewPaneNode;
	xge_xui_dock_node pSplit;
	int iAxis;
	float fNewRatio;

	if ( (pLayout == NULL) || !__xgeXuiDockRegionValid(iRegion) || !__xgeXuiDockSideIsSplit(iSide) ) {
		return NULL;
	}
	pRegion = &pLayout->arrRegions[iRegion];
	if ( pRegion->pRoot == NULL ) {
		return __xgeXuiDockLayoutEnsureRegionPane(pLayout, iRegion);
	}
	pOldRoot = pRegion->pRoot;
	pNewPaneNode = __xgeXuiDockNodeCreatePane(pLayout);
	if ( pNewPaneNode == NULL ) {
		return NULL;
	}
	iAxis = ((iSide == XGE_XUI_DOCK_SIDE_LEFT) || (iSide == XGE_XUI_DOCK_SIDE_RIGHT)) ? XGE_XUI_ORIENTATION_VERTICAL : XGE_XUI_ORIENTATION_HORIZONTAL;
	fNewRatio = __xgeXuiDockClampSplitRatio(fProportion);
	if ( (iSide == XGE_XUI_DOCK_SIDE_LEFT) || (iSide == XGE_XUI_DOCK_SIDE_TOP) ) {
		pSplit = __xgeXuiDockNodeCreateSplit(pNewPaneNode, pOldRoot, iAxis, fNewRatio);
	} else {
		pSplit = __xgeXuiDockNodeCreateSplit(pOldRoot, pNewPaneNode, iAxis, 1.0f - fNewRatio);
	}
	if ( pSplit == NULL ) {
		__xgeXuiDockNodeDestroy(pNewPaneNode);
		return NULL;
	}
	pSplit->pParent = NULL;
	pRegion->pRoot = pSplit;
	pRegion->bVisible = 1;
	return pNewPaneNode->pPane;
}

static xge_xui_dock_pane __xgeXuiDockLayoutSplitPane(xge_xui_dock_layout pLayout, xge_xui_dock_pane pTargetPane, int iSide, float fProportion)
{
	xge_xui_dock_region pRegion;
	xge_xui_dock_node pTargetNode;
	xge_xui_dock_node pNewPaneNode;
	xge_xui_dock_node pSplit;
	xge_xui_dock_node pGrand;
	int iAxis;
	float fNewRatio;

	if ( (pLayout == NULL) || (pTargetPane == NULL) || !__xgeXuiDockSideIsSplit(iSide) ) {
		return NULL;
	}
	pTargetNode = pTargetPane->pNode;
	if ( pTargetNode == NULL ) {
		return NULL;
	}
	if ( pTargetPane->arrWindows.Count == 0u ) {
		return pTargetPane;
	}
	pRegion = __xgeXuiDockLayoutFindNodeRegion(pLayout, pTargetNode);
	if ( pRegion == NULL ) {
		return NULL;
	}
	pNewPaneNode = __xgeXuiDockNodeCreatePane(pLayout);
	if ( pNewPaneNode == NULL ) {
		return NULL;
	}
	iAxis = ((iSide == XGE_XUI_DOCK_SIDE_LEFT) || (iSide == XGE_XUI_DOCK_SIDE_RIGHT)) ? XGE_XUI_ORIENTATION_VERTICAL : XGE_XUI_ORIENTATION_HORIZONTAL;
	fNewRatio = __xgeXuiDockClampSplitRatio(fProportion);
	pGrand = pTargetNode->pParent;
	if ( (iSide == XGE_XUI_DOCK_SIDE_LEFT) || (iSide == XGE_XUI_DOCK_SIDE_TOP) ) {
		pSplit = __xgeXuiDockNodeCreateSplit(pNewPaneNode, pTargetNode, iAxis, fNewRatio);
	} else {
		pSplit = __xgeXuiDockNodeCreateSplit(pTargetNode, pNewPaneNode, iAxis, 1.0f - fNewRatio);
	}
	if ( pSplit == NULL ) {
		__xgeXuiDockNodeDestroy(pNewPaneNode);
		return NULL;
	}
	pSplit->pParent = pGrand;
	if ( pGrand == NULL ) {
		pRegion->pRoot = pSplit;
	} else if ( pGrand->pFirst == pTargetNode ) {
		pGrand->pFirst = pSplit;
	} else if ( pGrand->pSecond == pTargetNode ) {
		pGrand->pSecond = pSplit;
	} else {
		if ( pSplit->pFirst == pTargetNode ) {
			pSplit->pFirst = NULL;
		}
		if ( pSplit->pSecond == pTargetNode ) {
			pSplit->pSecond = NULL;
		}
		pTargetNode->pParent = pGrand;
		__xgeXuiDockNodeDestroy(pNewPaneNode);
		xrtFree(pSplit);
		return NULL;
	}
	pRegion->bVisible = 1;
	return pNewPaneNode->pPane;
}

static xge_xui_dock_pane __xgeXuiDockLayoutDropPane(xge_xui_dock_layout pLayout, int iRegion, int iSide, float fProportion)
{
	xge_xui_dock_pane pFirstPane;

	if ( (pLayout == NULL) || !__xgeXuiDockRegionValid(iRegion) ) {
		return NULL;
	}
	if ( __xgeXuiDockSideIsSplit(iSide) && (pLayout->arrRegions[iRegion].pRoot != NULL) ) {
		pFirstPane = __xgeXuiDockNodeFirstPane(pLayout->arrRegions[iRegion].pRoot);
		if ( (pLayout->arrRegions[iRegion].pRoot->iType == XGE_XUI_DOCK_NODE_PANE) && (pFirstPane != NULL) && (pFirstPane->arrWindows.Count == 0u) ) {
			return pFirstPane;
		}
		return __xgeXuiDockLayoutSplitRegionRoot(pLayout, iRegion, iSide, fProportion);
	}
	return __xgeXuiDockLayoutEnsureRegionPane(pLayout, iRegion);
}

static int __xgeXuiDockLayoutDockWindowToPane(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow, xge_xui_dock_pane pPane, int iSide)
{
	xge_xui_dock_region pRegion;
	int iIndex;
	int iRet;

	if ( (pLayout == NULL) || (pWindow == NULL) || (pPane == NULL) || (pPane->pNode == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pRegion = __xgeXuiDockLayoutFindNodeRegion(pLayout, pPane->pNode);
	if ( pRegion == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pWindow->pPane == pPane) && (__xgeXuiDockArrayFindWindow(&pPane->arrWindows, pWindow) >= 0) ) {
		iIndex = __xgeXuiDockArrayFindWindow(&pPane->arrWindows, pWindow);
		pPane->iActive = iIndex;
	} else {
		__xgeXuiDockWindowDetachFromPane(pWindow);
		__xgeXuiDockLayoutRemoveFloatingWindow(pLayout, pWindow);
		iRet = __xgeXuiDockPaneAddWindow(pPane, pWindow);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
	}
	pWindow->iState = XGE_XUI_DOCK_WINDOW_DOCKED;
	pWindow->bVisible = 1;
	pWindow->tAutoHideStripRect = __xgeXuiDockRectZero();
	pWindow->iLastRegion = pRegion->iKind;
	pWindow->iLastSide = __xgeXuiDockSideIsSplit(iSide) ? iSide : XGE_XUI_DOCK_SIDE_FILL;
	pWindow->iLastTabIndex = pPane->iActive;
	if ( (pPane->tRect.fW > 0.0f) && (pPane->tRect.fH > 0.0f) ) {
		pWindow->tLastDockRect = pPane->tRect;
	}
	xgeXuiWindowSetOpen(&pWindow->tWindow, 0);
	__xgeXuiDockWindowAttachClientToLayout(pLayout, pWindow);
	if ( xgeXuiDockPaneGetActiveWindow(pPane) == pWindow ) {
		__xgeXuiDockLayoutRequestFocusWindow(pLayout, pWindow);
	}
	xgeXuiWidgetMarkLayout(pLayout->pWidget);
	xgeXuiWidgetMarkPaint(pLayout->pWidget);
	return XGE_OK;
}

static float __xgeXuiDockWindowTabWidth(xge_xui_dock_window pWindow, xui_font pFont)
{
	xge_vec2_t tText;
	const char* sTitle;
	float fWidth;

	sTitle = (pWindow != NULL && pWindow->sTitle != NULL) ? pWindow->sTitle : "";
	tText = (pFont != NULL) ? __xgeXuiHostMeasureText(pFont, sTitle) : (xge_vec2_t){ 0.0f, 0.0f };
	fWidth = ((tText.fX > 0.0f) ? tText.fX : (float)strlen(sTitle) * 7.0f) + 28.0f;
	return __xgeXuiDockClampFloat(fWidth, 74.0f, 156.0f);
}

static xge_vec2_t __xgeXuiDockPaneMeasureMin(xge_xui_dock_pane pPane)
{
	xge_xui_dock_layout pLayout;
	xge_xui_dock_window pWindow;
	xui_font pFont;
	xge_vec2_t tMin;
	xge_vec2_t tDesired;
	float fTabMin;
	uint32 i;

	tMin.fX = 148.0f;
	tMin.fY = 72.0f;
	if ( pPane == NULL ) {
		return tMin;
	}
	pLayout = pPane->pLayout;
	pFont = (pLayout != NULL && pLayout->pContext != NULL) ? xgeXuiGetTheme(pLayout->pContext)->pFont : NULL;
	fTabMin = 74.0f;
	for ( i = 0; i < pPane->arrWindows.Count; i++ ) {
		pWindow = xgeXuiDockPaneGetWindow(pPane, (int)i);
		if ( pWindow == NULL ) {
			continue;
		}
		if ( __xgeXuiDockWindowTabWidth(pWindow, pFont) < fTabMin ) {
			fTabMin = __xgeXuiDockWindowTabWidth(pWindow, pFont);
		}
		if ( pWindow->pClientWidget != NULL ) {
			tDesired = xgeXuiWidgetGetDesiredSize(pWindow->pClientWidget);
			if ( tDesired.fX + 2.0f > tMin.fX ) {
				tMin.fX = tDesired.fX + 2.0f;
			}
			if ( tDesired.fY + 27.0f > tMin.fY ) {
				tMin.fY = tDesired.fY + 27.0f;
			}
		}
		if ( pWindow->pContentWidget != NULL ) {
			tDesired = xgeXuiWidgetGetDesiredSize(pWindow->pContentWidget);
			if ( tDesired.fX + 2.0f > tMin.fX ) {
				tMin.fX = tDesired.fX + 2.0f;
			}
			if ( tDesired.fY + 27.0f > tMin.fY ) {
				tMin.fY = tDesired.fY + 27.0f;
			}
		}
	}
	if ( fTabMin + 74.0f > tMin.fX ) {
		tMin.fX = fTabMin + 74.0f;
	}
	return tMin;
}

static xge_vec2_t __xgeXuiDockNodeUpdateMinSize(xge_xui_dock_layout pLayout, xge_xui_dock_node pNode)
{
	xge_vec2_t tMin;
	xge_vec2_t tFirst;
	xge_vec2_t tSecond;
	float fSplitter;

	tMin.fX = 0.0f;
	tMin.fY = 0.0f;
	if ( pNode == NULL ) {
		return tMin;
	}
	if ( pNode->iType == XGE_XUI_DOCK_NODE_PANE ) {
		tMin = __xgeXuiDockPaneMeasureMin(pNode->pPane);
		pNode->fMinWidth = tMin.fX;
		pNode->fMinHeight = tMin.fY;
		return tMin;
	}
	if ( pNode->iType != XGE_XUI_DOCK_NODE_SPLIT ) {
		return tMin;
	}
	tFirst = __xgeXuiDockNodeUpdateMinSize(pLayout, pNode->pFirst);
	tSecond = __xgeXuiDockNodeUpdateMinSize(pLayout, pNode->pSecond);
	fSplitter = (pLayout != NULL && pLayout->fSplitterSize > 0.0f) ? pLayout->fSplitterSize : 4.0f;
	if ( pNode->iAxis == XGE_XUI_ORIENTATION_HORIZONTAL ) {
		tMin.fX = (tFirst.fX > tSecond.fX) ? tFirst.fX : tSecond.fX;
		tMin.fY = tFirst.fY + fSplitter + tSecond.fY;
	} else {
		tMin.fX = tFirst.fX + fSplitter + tSecond.fX;
		tMin.fY = (tFirst.fY > tSecond.fY) ? tFirst.fY : tSecond.fY;
	}
	pNode->fMinWidth = tMin.fX;
	pNode->fMinHeight = tMin.fY;
	return tMin;
}

static float __xgeXuiDockNodeAxisMinSize(xge_xui_dock_node pNode, int iAxis)
{
	if ( pNode == NULL ) {
		return 0.0f;
	}
	return (iAxis == XGE_XUI_ORIENTATION_HORIZONTAL) ? pNode->fMinHeight : pNode->fMinWidth;
}

static float __xgeXuiDockSplitRatioForMin(xge_xui_dock_node pNode, float fAvailable, float fRatio)
{
	float fFirstMin;
	float fSecondMin;
	float fMinRatio;
	float fMaxRatio;
	float fMinSum;

	fRatio = __xgeXuiDockClampSplitRatio(fRatio);
	if ( (pNode == NULL) || (pNode->iType != XGE_XUI_DOCK_NODE_SPLIT) || (fAvailable <= 1.0f) ) {
		return fRatio;
	}
	fFirstMin = __xgeXuiDockNodeAxisMinSize(pNode->pFirst, pNode->iAxis);
	fSecondMin = __xgeXuiDockNodeAxisMinSize(pNode->pSecond, pNode->iAxis);
	if ( fFirstMin < 0.0f ) {
		fFirstMin = 0.0f;
	}
	if ( fSecondMin < 0.0f ) {
		fSecondMin = 0.0f;
	}
	fMinSum = fFirstMin + fSecondMin;
	if ( fMinSum <= 0.0f ) {
		return fRatio;
	}
	if ( fAvailable <= fMinSum ) {
		return __xgeXuiDockClampFloat(fFirstMin / fMinSum, 0.0f, 1.0f);
	}
	fMinRatio = fFirstMin / fAvailable;
	fMaxRatio = 1.0f - (fSecondMin / fAvailable);
	if ( fMaxRatio < fMinRatio ) {
		return __xgeXuiDockClampFloat(fFirstMin / fMinSum, 0.0f, 1.0f);
	}
	return __xgeXuiDockClampFloat(fRatio, fMinRatio, fMaxRatio);
}

static float __xgeXuiDockPaneTabLimitRight(xge_xui_dock_pane pPane)
{
	float fRight;

	if ( pPane == NULL ) {
		return 0.0f;
	}
	fRight = pPane->tTabStripRect.fX + pPane->tTabStripRect.fW - 3.0f;
	if ( pPane->tCloseRect.fW > 0.0f && pPane->tCloseRect.fX < fRight ) {
		fRight = pPane->tCloseRect.fX - 3.0f;
	}
	if ( pPane->tAutoHideRect.fW > 0.0f && pPane->tAutoHideRect.fX < fRight ) {
		fRight = pPane->tAutoHideRect.fX - 3.0f;
	}
	if ( pPane->tOptionRect.fW > 0.0f && pPane->tOptionRect.fX < fRight ) {
		fRight = pPane->tOptionRect.fX - 3.0f;
	}
	if ( pPane->tOverflowRect.fW > 0.0f && pPane->tOverflowRect.fX < fRight ) {
		fRight = pPane->tOverflowRect.fX - 3.0f;
	}
	return fRight;
}

static int __xgeXuiDockPaneTabsNeedOverflow(xge_xui_dock_pane pPane, float fRight)
{
	xge_xui_dock_layout pLayout;
	xui_font pFont;
	float fX;
	float fWidth;
	int i;

	if ( (pPane == NULL) || (pPane->arrWindows.Count <= 0u) ) {
		return 0;
	}
	pLayout = pPane->pLayout;
	pFont = (pLayout != NULL && pLayout->pContext != NULL) ? xgeXuiGetTheme(pLayout->pContext)->pFont : NULL;
	fX = pPane->tTabStripRect.fX + 3.0f;
	if ( fX + 8.0f > fRight ) {
		return 1;
	}
	for ( i = 0; i < (int)pPane->arrWindows.Count; i++ ) {
		fWidth = __xgeXuiDockWindowTabWidth(xgeXuiDockPaneGetWindow(pPane, i), pFont);
		if ( fX + fWidth > fRight ) {
			return 1;
		}
		fX += fWidth - 1.0f;
	}
	return 0;
}

static void __xgeXuiDockPaneDrawTabTitle(xui_font pFont, const char* sTitle, xge_rect_t tRect, uint32_t iColor)
{
	char sBuffer[128];
	xge_vec2_t tSize;
	size_t nLen;
	size_t nTry;

	if ( sTitle == NULL ) {
		sTitle = "";
	}
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		return;
	}
	tSize = (pFont != NULL) ? __xgeXuiHostMeasureText(pFont, sTitle) : (xge_vec2_t){ (float)strlen(sTitle) * 7.0f, 0.0f };
	if ( tSize.fX <= tRect.fW ) {
		__xgeXuiHostDrawTextRect(pFont, sTitle, tRect, iColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		return;
	}
	if ( tRect.fW < 18.0f ) {
		return;
	}
	nLen = strlen(sTitle);
	if ( nLen > sizeof(sBuffer) - 4u ) {
		nLen = sizeof(sBuffer) - 4u;
	}
	for ( nTry = nLen; nTry > 0u; nTry-- ) {
		memcpy(sBuffer, sTitle, nTry);
		memcpy(sBuffer + nTry, "...", 4u);
		tSize = (pFont != NULL) ? __xgeXuiHostMeasureText(pFont, sBuffer) : (xge_vec2_t){ (float)strlen(sBuffer) * 7.0f, 0.0f };
		if ( tSize.fX <= tRect.fW ) {
			__xgeXuiHostDrawTextRect(pFont, sBuffer, tRect, iColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			return;
		}
	}
	__xgeXuiHostDrawTextRect(pFont, "...", tRect, iColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
}

static xge_rect_t __xgeXuiDockPaneTabRect(xge_xui_dock_pane pPane, int iIndex)
{
	xge_xui_dock_layout pLayout;
	xui_font pFont;
	xge_rect_t tRect;
	float fX;
	float fRight;
	int i;

	tRect = __xgeXuiDockRectZero();
	if ( (pPane == NULL) || (iIndex < 0) || (iIndex >= (int)pPane->arrWindows.Count) ) {
		return tRect;
	}
	pLayout = pPane->pLayout;
	pFont = (pLayout != NULL && pLayout->pContext != NULL) ? xgeXuiGetTheme(pLayout->pContext)->pFont : NULL;
	fX = pPane->tTabStripRect.fX + 3.0f;
	fRight = __xgeXuiDockPaneTabLimitRight(pPane);
	for ( i = 0; i <= iIndex; i++ ) {
		xge_xui_dock_window pWindow = xgeXuiDockPaneGetWindow(pPane, i);
		float fWidth = __xgeXuiDockWindowTabWidth(pWindow, pFont);
		if ( i == iIndex ) {
			tRect = (xge_rect_t){ fX, pPane->tTabStripRect.fY + 3.0f, fWidth, pPane->tTabStripRect.fH - 3.0f };
			if ( tRect.fX + tRect.fW > fRight ) {
				tRect.fW = fRight - tRect.fX;
			}
			if ( tRect.fW < 8.0f ) {
				return __xgeXuiDockRectZero();
			}
			return tRect;
		}
		fX += fWidth - 1.0f;
	}
	return tRect;
}

static int __xgeXuiDockPaneTabInsertIndex(xge_xui_dock_pane pPane, float fX, float fY)
{
	xge_rect_t tTab;
	int i;
	int iCount;

	if ( (pPane == NULL) || !__xgeXuiDockRectContains(pPane->tTabStripRect, fX, fY) ) {
		return -1;
	}
	iCount = (int)pPane->arrWindows.Count;
	for ( i = 0; i < iCount; i++ ) {
		tTab = __xgeXuiDockPaneTabRect(pPane, i);
		if ( tTab.fW <= 0.0f || tTab.fH <= 0.0f ) {
			continue;
		}
		if ( fX < tTab.fX + (tTab.fW * 0.5f) ) {
			return i;
		}
		if ( fX < tTab.fX + tTab.fW ) {
			return i + 1;
		}
	}
	return iCount;
}

static xge_rect_t __xgeXuiDockPaneTabInsertRect(xge_xui_dock_pane pPane, int iInsert)
{
	xge_rect_t tTab;
	float fX;
	int iCount;

	if ( pPane == NULL ) {
		return __xgeXuiDockRectZero();
	}
	iCount = (int)pPane->arrWindows.Count;
	if ( iInsert < 0 ) {
		iInsert = 0;
	}
	if ( iInsert > iCount ) {
		iInsert = iCount;
	}
	if ( iInsert < iCount ) {
		tTab = __xgeXuiDockPaneTabRect(pPane, iInsert);
		fX = tTab.fX;
	} else if ( iCount > 0 ) {
		tTab = __xgeXuiDockPaneTabRect(pPane, iCount - 1);
		fX = tTab.fX + tTab.fW;
	} else {
		fX = pPane->tTabStripRect.fX + 4.0f;
	}
	return (xge_rect_t){ fX - 1.0f, pPane->tTabStripRect.fY + 4.0f, 2.0f, pPane->tTabStripRect.fH - 8.0f };
}

static int __xgeXuiDockPaneMoveWindow(xge_xui_dock_pane pPane, int iFrom, int iInsert)
{
	xge_xui_dock_window pWindow;
	int iRet;
	int iActive;

	if ( pPane == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pWindow = xgeXuiDockPaneGetWindow(pPane, iFrom);
	iRet = __xgeXuiDockArrayMoveWindow(&pPane->arrWindows, iFrom, iInsert);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iActive = __xgeXuiDockArrayFindWindow(&pPane->arrWindows, pWindow);
	pPane->iActive = iActive;
	if ( pWindow != NULL ) {
		pWindow->iLastTabIndex = iActive;
	}
	if ( (pPane->pLayout != NULL) && (pPane->pLayout->pWidget != NULL) ) {
		__xgeXuiDockLayoutRequestFocusWindow(pPane->pLayout, pWindow);
		xgeXuiWidgetMarkLayout(pPane->pLayout->pWidget);
		xgeXuiWidgetMarkPaint(pPane->pLayout->pWidget);
	}
	return XGE_OK;
}

static int __xgeXuiDockPaneHitPart(xge_xui_dock_pane pPane, float fX, float fY)
{
	int i;

	if ( pPane == NULL ) {
		return XGE_XUI_DOCK_PART_NONE;
	}
	if ( __xgeXuiDockRectContains(pPane->tCloseRect, fX, fY) ) {
		return XGE_XUI_DOCK_PART_CLOSE;
	}
	if ( __xgeXuiDockRectContains(pPane->tAutoHideRect, fX, fY) ) {
		return __xgeXuiDockPaneCanAutoHide(pPane) ? XGE_XUI_DOCK_PART_AUTO_HIDE : XGE_XUI_DOCK_PART_NONE;
	}
	if ( __xgeXuiDockRectContains(pPane->tOptionRect, fX, fY) ) {
		return XGE_XUI_DOCK_PART_OPTION;
	}
	if ( __xgeXuiDockRectContains(pPane->tOverflowRect, fX, fY) ) {
		return XGE_XUI_DOCK_PART_OVERFLOW;
	}
	if ( !__xgeXuiDockRectContains(pPane->tTabStripRect, fX, fY) ) {
		return XGE_XUI_DOCK_PART_NONE;
	}
	for ( i = 0; i < (int)pPane->arrWindows.Count; i++ ) {
		if ( __xgeXuiDockRectContains(__xgeXuiDockPaneTabRect(pPane, i), fX, fY) ) {
			return i;
		}
	}
	if ( pPane->iActive >= 0 && pPane->iActive < (int)pPane->arrWindows.Count ) {
		return XGE_XUI_DOCK_PART_CAPTION;
	}
	return XGE_XUI_DOCK_PART_NONE;
}

static xge_xui_dock_pane __xgeXuiDockHitPaneNode(xge_xui_dock_node pNode, float fX, float fY, int* pPart)
{
	xge_xui_dock_pane pPane;

	if ( pNode == NULL ) {
		return NULL;
	}
	if ( pNode->iType == XGE_XUI_DOCK_NODE_SPLIT ) {
		pPane = __xgeXuiDockHitPaneNode(pNode->pFirst, fX, fY, pPart);
		return (pPane != NULL) ? pPane : __xgeXuiDockHitPaneNode(pNode->pSecond, fX, fY, pPart);
	}
	if ( pNode->iType != XGE_XUI_DOCK_NODE_PANE || pNode->pPane == NULL || !__xgeXuiDockRectContains(pNode->pPane->tRect, fX, fY) ) {
		return NULL;
	}
	if ( pPart != NULL ) {
		*pPart = __xgeXuiDockPaneHitPart(pNode->pPane, fX, fY);
	}
	return (pPart == NULL || *pPart != XGE_XUI_DOCK_PART_NONE) ? pNode->pPane : NULL;
}

static xge_xui_dock_pane __xgeXuiDockLayoutHitPane(xge_xui_dock_layout pLayout, float fX, float fY, int* pPart)
{
	int i;

	if ( pPart != NULL ) {
		*pPart = XGE_XUI_DOCK_PART_NONE;
	}
	if ( pLayout == NULL ) {
		return NULL;
	}
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		xge_xui_dock_pane pPane = __xgeXuiDockHitPaneNode(pLayout->arrRegions[i].pRoot, fX, fY, pPart);
		if ( pPane != NULL ) {
			return pPane;
		}
	}
	return NULL;
}

static xge_xui_dock_pane __xgeXuiDockHitAnyPaneNode(xge_xui_dock_node pNode, float fX, float fY)
{
	xge_xui_dock_pane pPane;

	if ( pNode == NULL ) {
		return NULL;
	}
	if ( pNode->iType == XGE_XUI_DOCK_NODE_SPLIT ) {
		pPane = __xgeXuiDockHitAnyPaneNode(pNode->pFirst, fX, fY);
		return (pPane != NULL) ? pPane : __xgeXuiDockHitAnyPaneNode(pNode->pSecond, fX, fY);
	}
	if ( pNode->iType != XGE_XUI_DOCK_NODE_PANE || pNode->pPane == NULL || !__xgeXuiDockRectContains(pNode->pPane->tRect, fX, fY) ) {
		return NULL;
	}
	return pNode->pPane;
}

static xge_xui_dock_pane __xgeXuiDockLayoutHitAnyPane(xge_xui_dock_layout pLayout, float fX, float fY)
{
	int i;

	if ( pLayout == NULL ) {
		return NULL;
	}
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		xge_xui_dock_pane pPane = __xgeXuiDockHitAnyPaneNode(pLayout->arrRegions[i].pRoot, fX, fY);
		if ( pPane != NULL ) {
			return pPane;
		}
	}
	return NULL;
}

static xge_xui_dock_node __xgeXuiDockHitSplitterNode(xge_xui_dock_node pNode, float fX, float fY)
{
	xge_xui_dock_node pHit;

	if ( pNode == NULL ) {
		return NULL;
	}
	if ( pNode->iType != XGE_XUI_DOCK_NODE_SPLIT ) {
		return NULL;
	}
	if ( __xgeXuiDockRectContains(pNode->tSplitterRect, fX, fY) ) {
		return pNode;
	}
	pHit = __xgeXuiDockHitSplitterNode(pNode->pFirst, fX, fY);
	return (pHit != NULL) ? pHit : __xgeXuiDockHitSplitterNode(pNode->pSecond, fX, fY);
}

static xge_xui_dock_node __xgeXuiDockLayoutHitSplitter(xge_xui_dock_layout pLayout, float fX, float fY)
{
	int i;

	if ( pLayout == NULL ) {
		return NULL;
	}
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		xge_xui_dock_node pNode = __xgeXuiDockHitSplitterNode(pLayout->arrRegions[i].pRoot, fX, fY);
		if ( pNode != NULL ) {
			return pNode;
		}
	}
	return NULL;
}

static void __xgeXuiDockClearHoverNode(xge_xui_dock_node pNode)
{
	if ( pNode == NULL ) {
		return;
	}
	if ( pNode->pPane != NULL ) {
		pNode->pPane->iHoverPart = XGE_XUI_DOCK_PART_NONE;
	}
	__xgeXuiDockClearHoverNode(pNode->pFirst);
	__xgeXuiDockClearHoverNode(pNode->pSecond);
}

static void __xgeXuiDockLayoutClearHover(xge_xui_dock_layout pLayout)
{
	int i;

	if ( pLayout == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		__xgeXuiDockClearHoverNode(pLayout->arrRegions[i].pRoot);
	}
	pLayout->pHoverPane = NULL;
}

static xge_xui_dock_pane __xgeXuiDockFindActivePartPaneNode(xge_xui_dock_node pNode)
{
	xge_xui_dock_pane pPane;

	if ( pNode == NULL ) {
		return NULL;
	}
	if ( pNode->iType == XGE_XUI_DOCK_NODE_PANE ) {
		return (pNode->pPane != NULL && pNode->pPane->iActivePart != XGE_XUI_DOCK_PART_NONE) ? pNode->pPane : NULL;
	}
	pPane = __xgeXuiDockFindActivePartPaneNode(pNode->pFirst);
	return (pPane != NULL) ? pPane : __xgeXuiDockFindActivePartPaneNode(pNode->pSecond);
}

static xge_xui_dock_pane __xgeXuiDockLayoutFindActivePartPane(xge_xui_dock_layout pLayout)
{
	int i;

	if ( pLayout == NULL ) {
		return NULL;
	}
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		xge_xui_dock_pane pPane = __xgeXuiDockFindActivePartPaneNode(pLayout->arrRegions[i].pRoot);
		if ( pPane != NULL ) {
			return pPane;
		}
	}
	return NULL;
}

static void __xgeXuiDockClearActivePartNode(xge_xui_dock_node pNode)
{
	if ( pNode == NULL ) {
		return;
	}
	if ( pNode->pPane != NULL ) {
		pNode->pPane->iActivePart = XGE_XUI_DOCK_PART_NONE;
	}
	__xgeXuiDockClearActivePartNode(pNode->pFirst);
	__xgeXuiDockClearActivePartNode(pNode->pSecond);
}

static void __xgeXuiDockLayoutClearActivePart(xge_xui_dock_layout pLayout)
{
	int i;

	if ( pLayout == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		__xgeXuiDockClearActivePartNode(pLayout->arrRegions[i].pRoot);
	}
	pLayout->iActiveButton = 0;
}

static int __xgeXuiDockLayoutBeginSplitterDrag(xge_xui_dock_layout pLayout, xge_xui_dock_node pNode, const xge_event_t* pEvent)
{
	if ( (pLayout == NULL) || (pNode == NULL) || (pEvent == NULL) || (pNode->iType != XGE_XUI_DOCK_NODE_SPLIT) ) {
		return 0;
	}
	if ( (pNode->tSplitterRect.fW <= 0.0f) || (pNode->tSplitterRect.fH <= 0.0f) || (pLayout->iDragPhase != XGE_XUI_DOCK_DRAG_IDLE) ) {
		return 0;
	}
	__xgeXuiDockLayoutCloseOptionMenu(pLayout);
	__xgeXuiDockLayoutCloseOverflowMenu(pLayout);
	__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
	__xgeXuiDockLayoutClearActivePart(pLayout);
	__xgeXuiDockLayoutClearHover(pLayout);
	pLayout->pSplitterDragNode = pNode;
	pLayout->tSplitterDragStartMouse = (xge_vec2_t){ pEvent->fX, pEvent->fY };
	pLayout->fSplitterDragStartRatio = __xgeXuiDockClampSplitRatio(pNode->fRatio);
	if ( pLayout->pContext != NULL ) {
		xgeXuiSetPointerCapture(pLayout->pContext, pEvent->iPointerId, pLayout->pWidget);
	}
	if ( pLayout->pWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pLayout->pWidget);
	}
	return 1;
}

static void __xgeXuiDockLayoutEndSplitterDrag(xge_xui_dock_layout pLayout)
{
	if ( pLayout == NULL ) {
		return;
	}
	pLayout->pSplitterDragNode = NULL;
	pLayout->tSplitterDragStartMouse = (xge_vec2_t){ 0.0f, 0.0f };
	pLayout->fSplitterDragStartRatio = 0.0f;
	if ( pLayout->pWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pLayout->pWidget);
	}
}

static int __xgeXuiDockLayoutUpdateSplitterDrag(xge_xui_dock_layout pLayout, const xge_event_t* pEvent)
{
	xge_xui_dock_node pNode;
	float fSplitter;
	float fAvailable;
	float fDelta;
	float fRatio;

	if ( (pLayout == NULL) || (pEvent == NULL) || (pLayout->pSplitterDragNode == NULL) ) {
		return 0;
	}
	pNode = pLayout->pSplitterDragNode;
	if ( pNode->iType != XGE_XUI_DOCK_NODE_SPLIT ) {
		__xgeXuiDockLayoutEndSplitterDrag(pLayout);
		return 1;
	}
	fSplitter = (pLayout->fSplitterSize > 0.0f) ? pLayout->fSplitterSize : 4.0f;
	if ( pNode->iAxis == XGE_XUI_ORIENTATION_HORIZONTAL ) {
		fAvailable = pNode->tRect.fH - fSplitter;
		fDelta = pEvent->fY - pLayout->tSplitterDragStartMouse.fY;
	} else {
		fAvailable = pNode->tRect.fW - fSplitter;
		fDelta = pEvent->fX - pLayout->tSplitterDragStartMouse.fX;
	}
	if ( fAvailable > 1.0f ) {
		__xgeXuiDockNodeUpdateMinSize(pLayout, pNode);
		fRatio = pLayout->fSplitterDragStartRatio + (fDelta / fAvailable);
		pNode->fRatio = __xgeXuiDockSplitRatioForMin(pNode, fAvailable, fRatio);
	}
	if ( pLayout->pWidget != NULL ) {
		xgeXuiWidgetMarkLayout(pLayout->pWidget);
		xgeXuiWidgetMarkPaint(pLayout->pWidget);
	}
	return 1;
}

static void __xgeXuiDockPaneArrange(xge_xui_dock_pane pPane, xge_rect_t tRect)
{
	xge_xui_dock_layout pLayout;
	xge_xui_dock_window pActiveWindow;
	float fChromeH;
	float fButtonW;
	float fButtonH;
	float fButtonY;
	float fButtonX;
	uint32 i;

	if ( pPane == NULL ) {
		return;
	}
	pPane->tRect = tRect;
	pPane->tTabStripRect = __xgeXuiDockRectZero();
	pPane->tCaptionRect = __xgeXuiDockRectZero();
	pPane->tClientRect = __xgeXuiDockRectZero();
	pPane->tCloseRect = __xgeXuiDockRectZero();
	pPane->tAutoHideRect = __xgeXuiDockRectZero();
	pPane->tDockRect = __xgeXuiDockRectZero();
	pPane->tOptionRect = __xgeXuiDockRectZero();
	pPane->tOverflowRect = __xgeXuiDockRectZero();
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	if ( pPane->arrWindows.Count <= 0u ) {
		pPane->iActive = -1;
	} else if ( pPane->iActive < 0 || pPane->iActive >= (int)pPane->arrWindows.Count ) {
		pPane->iActive = 0;
	}
	pLayout = pPane->pLayout;
	fChromeH = __xgeXuiDockLayoutTabStripHeight(pLayout);
	if ( fChromeH > tRect.fH ) {
		fChromeH = tRect.fH;
	}
	pPane->tTabStripRect = (xge_rect_t){ tRect.fX + 1.0f, tRect.fY + 1.0f, tRect.fW - 2.0f, fChromeH };
	if ( pPane->tTabStripRect.fW < 0.0f ) {
		pPane->tTabStripRect.fW = 0.0f;
	}
	pPane->tCaptionRect = pPane->tTabStripRect;
	pPane->tClientRect = (xge_rect_t){ tRect.fX + 1.0f, tRect.fY + fChromeH + 1.0f, tRect.fW - 2.0f, tRect.fH - fChromeH - 2.0f };
	if ( pPane->tClientRect.fW < 0.0f ) {
		pPane->tClientRect.fW = 0.0f;
	}
	if ( pPane->tClientRect.fH < 0.0f ) {
		pPane->tClientRect.fH = 0.0f;
	}
	fButtonW = __xgeXuiDockLayoutButtonWidth(pLayout);
	fButtonH = __xgeXuiDockLayoutButtonHeight(pLayout);
	if ( fButtonH > fChromeH ) {
		fButtonH = fChromeH;
	}
	fButtonY = pPane->tTabStripRect.fY + __xgeXuiDockButtonTop(fChromeH, fButtonH);
	fButtonX = pPane->tTabStripRect.fX + pPane->tTabStripRect.fW - fButtonW - 4.0f;
	if ( fButtonX >= pPane->tTabStripRect.fX ) {
		pPane->tOptionRect = (xge_rect_t){ fButtonX, fButtonY, fButtonW, fButtonH };
		fButtonX -= fButtonW + 2.0f;
	}
	if ( fButtonX >= pPane->tTabStripRect.fX ) {
		pPane->tAutoHideRect = (xge_rect_t){ fButtonX, fButtonY, fButtonW, fButtonH };
		fButtonX -= fButtonW + 2.0f;
	}
	pActiveWindow = xgeXuiDockPaneGetActiveWindow(pPane);
	if ( (pActiveWindow != NULL) && (pActiveWindow->bClosable != 0) && (fButtonX >= pPane->tTabStripRect.fX) ) {
		pPane->tCloseRect = (xge_rect_t){ fButtonX, fButtonY, fButtonW, fButtonH };
		fButtonX -= fButtonW + 2.0f;
	}
	if ( (fButtonX >= pPane->tTabStripRect.fX) && __xgeXuiDockPaneTabsNeedOverflow(pPane, __xgeXuiDockPaneTabLimitRight(pPane)) ) {
		pPane->tOverflowRect = (xge_rect_t){ fButtonX, fButtonY, fButtonW, fButtonH };
	}
	for ( i = 0; i < pPane->arrWindows.Count; i++ ) {
		xge_xui_dock_window pWindow = xgeXuiDockPaneGetWindow(pPane, (int)i);
		if ( (pWindow == NULL) || (pWindow->iState != XGE_XUI_DOCK_WINDOW_DOCKED) || (pWindow->pClientWidget == NULL) ) {
			continue;
		}
		if ( (int)i == pPane->iActive ) {
			__xgeXuiDockWindowAttachClientToLayout(pPane->pLayout, pWindow);
			__xgeXuiDockWidgetArrange(pWindow->pClientWidget, pPane->tClientRect);
			xgeXuiWidgetSetVisible(pWindow->pClientWidget, 1);
			if ( (pPane->pLayout != NULL) && (pPane->pLayout->pPendingFocusWindow == pWindow) ) {
				(void)__xgeXuiDockWindowFocusNow(pWindow);
				pPane->pLayout->pPendingFocusWindow = NULL;
			}
		} else {
			xgeXuiWidgetSetVisible(pWindow->pClientWidget, 0);
		}
	}
}

static void __xgeXuiDockLayoutArrangeNode(xge_xui_dock_layout pLayout, xge_xui_dock_node pNode, xge_rect_t tRect)
{
	xge_rect_t tFirst;
	xge_rect_t tSecond;
	float fSplitter;
	float fFirstSize;

	if ( pNode == NULL ) {
		return;
	}
	pNode->tRect = tRect;
	pNode->tSplitterRect = __xgeXuiDockRectZero();
	if ( pNode->iType == XGE_XUI_DOCK_NODE_PANE ) {
		__xgeXuiDockPaneArrange(pNode->pPane, tRect);
		return;
	}
	if ( pNode->iType != XGE_XUI_DOCK_NODE_SPLIT ) {
		return;
	}
	fSplitter = (pLayout != NULL && pLayout->fSplitterSize > 0.0f) ? pLayout->fSplitterSize : 4.0f;
	if ( pNode->iAxis == XGE_XUI_ORIENTATION_HORIZONTAL ) {
		if ( fSplitter > tRect.fH ) {
			fSplitter = tRect.fH;
		}
		pNode->fRatio = __xgeXuiDockSplitRatioForMin(pNode, tRect.fH - fSplitter, pNode->fRatio);
		fFirstSize = (tRect.fH - fSplitter) * pNode->fRatio;
		tFirst = (xge_rect_t){ tRect.fX, tRect.fY, tRect.fW, fFirstSize };
		pNode->tSplitterRect = (xge_rect_t){ tRect.fX, tRect.fY + fFirstSize, tRect.fW, fSplitter };
		tSecond = (xge_rect_t){ tRect.fX, pNode->tSplitterRect.fY + pNode->tSplitterRect.fH, tRect.fW, tRect.fH - fFirstSize - fSplitter };
	} else {
		if ( fSplitter > tRect.fW ) {
			fSplitter = tRect.fW;
		}
		pNode->fRatio = __xgeXuiDockSplitRatioForMin(pNode, tRect.fW - fSplitter, pNode->fRatio);
		fFirstSize = (tRect.fW - fSplitter) * pNode->fRatio;
		tFirst = (xge_rect_t){ tRect.fX, tRect.fY, fFirstSize, tRect.fH };
		pNode->tSplitterRect = (xge_rect_t){ tRect.fX + fFirstSize, tRect.fY, fSplitter, tRect.fH };
		tSecond = (xge_rect_t){ pNode->tSplitterRect.fX + pNode->tSplitterRect.fW, tRect.fY, tRect.fW - fFirstSize - fSplitter, tRect.fH };
	}
	__xgeXuiDockLayoutArrangeNode(pLayout, pNode->pFirst, tFirst);
	__xgeXuiDockLayoutArrangeNode(pLayout, pNode->pSecond, tSecond);
}

static void __xgeXuiDockLayoutAllocateRegions(xge_xui_dock_layout pLayout, xge_rect_t tContent)
{
	xge_rect_t tRemain;
	float fSize;
	int i;

	if ( pLayout == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		pLayout->arrRegions[i].tRect = __xgeXuiDockRectZero();
	}
	tRemain = tContent;
	fSize = __xgeXuiDockRegionSideSize(&pLayout->arrRegions[XGE_XUI_DOCK_REGION_LEFT], tRemain.fW, tContent.fW);
	if ( fSize > 0.0f ) {
		pLayout->arrRegions[XGE_XUI_DOCK_REGION_LEFT].tRect = (xge_rect_t){ tRemain.fX, tRemain.fY, fSize, tRemain.fH };
		tRemain.fX += fSize;
		tRemain.fW -= fSize;
	}
	fSize = __xgeXuiDockRegionSideSize(&pLayout->arrRegions[XGE_XUI_DOCK_REGION_RIGHT], tRemain.fW, tContent.fW);
	if ( fSize > 0.0f ) {
		pLayout->arrRegions[XGE_XUI_DOCK_REGION_RIGHT].tRect = (xge_rect_t){ tRemain.fX + tRemain.fW - fSize, tRemain.fY, fSize, tRemain.fH };
		tRemain.fW -= fSize;
	}
	fSize = __xgeXuiDockRegionSideSize(&pLayout->arrRegions[XGE_XUI_DOCK_REGION_TOP], tRemain.fH, tContent.fH);
	if ( fSize > 0.0f ) {
		pLayout->arrRegions[XGE_XUI_DOCK_REGION_TOP].tRect = (xge_rect_t){ tRemain.fX, tRemain.fY, tRemain.fW, fSize };
		tRemain.fY += fSize;
		tRemain.fH -= fSize;
	}
	fSize = __xgeXuiDockRegionSideSize(&pLayout->arrRegions[XGE_XUI_DOCK_REGION_BOTTOM], tRemain.fH, tContent.fH);
	if ( fSize > 0.0f ) {
		pLayout->arrRegions[XGE_XUI_DOCK_REGION_BOTTOM].tRect = (xge_rect_t){ tRemain.fX, tRemain.fY + tRemain.fH - fSize, tRemain.fW, fSize };
		tRemain.fH -= fSize;
	}
	if ( tRemain.fW < 0.0f ) {
		tRemain.fW = 0.0f;
	}
	if ( tRemain.fH < 0.0f ) {
		tRemain.fH = 0.0f;
	}
	pLayout->arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT].tRect = tRemain;
}

static int __xgeXuiDockLayoutHasAutoHideRegion(xge_xui_dock_layout pLayout, int iRegion)
{
	xge_xui_dock_window* ppWindow;
	uint32 i;

	if ( (pLayout == NULL) || !__xgeXuiDockRegionValid(iRegion) || (iRegion == XGE_XUI_DOCK_REGION_DOCUMENT) ) {
		return 0;
	}
	for ( i = 0; i < pLayout->arrWindows.Count; i++ ) {
		ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline(&pLayout->arrWindows, i + 1u);
		if ( (ppWindow != NULL) && (*ppWindow != NULL) && ((*ppWindow)->iState == XGE_XUI_DOCK_WINDOW_AUTO_HIDE) && ((*ppWindow)->iAutoHideRegion == iRegion) ) {
			return 1;
		}
	}
	return 0;
}

static float __xgeXuiDockAutoHideItemLength(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow)
{
	xui_font pFont;
	xge_vec2_t tText;
	const char* sTitle;
	float fLength;

	sTitle = (pWindow != NULL && pWindow->sTitle != NULL) ? pWindow->sTitle : "";
	pFont = (pLayout != NULL && pLayout->pContext != NULL) ? xgeXuiGetTheme(pLayout->pContext)->pFont : NULL;
	tText = (pFont != NULL) ? __xgeXuiHostMeasureText(pFont, sTitle) : (xge_vec2_t){ (float)strlen(sTitle) * 7.0f, 16.0f };
	fLength = tText.fX + 30.0f;
	return __xgeXuiDockClampFloat(fLength, 54.0f, 140.0f);
}

static void __xgeXuiDockLayoutAssignAutoHideStrip(xge_xui_dock_layout pLayout, int iRegion, xge_rect_t tStrip)
{
	xge_xui_dock_window* ppWindow;
	xge_xui_dock_window pWindow;
	float fCursor;
	float fLength;
	uint32 i;

	if ( (pLayout == NULL) || !__xgeXuiDockRegionValid(iRegion) || (iRegion == XGE_XUI_DOCK_REGION_DOCUMENT) ) {
		return;
	}
	if ( (iRegion == XGE_XUI_DOCK_REGION_LEFT) || (iRegion == XGE_XUI_DOCK_REGION_RIGHT) ) {
		fCursor = tStrip.fY + 2.0f;
	} else {
		fCursor = tStrip.fX + 2.0f;
	}
	for ( i = 0; i < pLayout->arrWindows.Count; i++ ) {
		ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline(&pLayout->arrWindows, i + 1u);
		pWindow = (ppWindow != NULL) ? *ppWindow : NULL;
		if ( (pWindow == NULL) || (pWindow->iState != XGE_XUI_DOCK_WINDOW_AUTO_HIDE) || (pWindow->iAutoHideRegion != iRegion) ) {
			continue;
		}
		fLength = __xgeXuiDockAutoHideItemLength(pLayout, pWindow);
		if ( (iRegion == XGE_XUI_DOCK_REGION_LEFT) || (iRegion == XGE_XUI_DOCK_REGION_RIGHT) ) {
			if ( fCursor + fLength > tStrip.fY + tStrip.fH - 2.0f ) {
				pWindow->tAutoHideStripRect = __xgeXuiDockRectZero();
				continue;
			}
			pWindow->tAutoHideStripRect = (xge_rect_t){ tStrip.fX + 2.0f, fCursor, tStrip.fW - 4.0f, fLength };
			fCursor += fLength + 2.0f;
		} else {
			if ( fCursor + fLength > tStrip.fX + tStrip.fW - 2.0f ) {
				pWindow->tAutoHideStripRect = __xgeXuiDockRectZero();
				continue;
			}
			pWindow->tAutoHideStripRect = (xge_rect_t){ fCursor, tStrip.fY + 2.0f, fLength, tStrip.fH - 4.0f };
			fCursor += fLength + 2.0f;
		}
	}
}

static xge_rect_t __xgeXuiDockLayoutArrangeAutoHideStrips(xge_xui_dock_layout pLayout, xge_rect_t tContent)
{
	xge_xui_dock_window* ppWindow;
	xge_rect_t tStrip;
	float fStrip;
	uint32 i;

	if ( pLayout == NULL ) {
		return tContent;
	}
	for ( i = 0; i < pLayout->arrWindows.Count; i++ ) {
		ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline(&pLayout->arrWindows, i + 1u);
		if ( (ppWindow != NULL) && (*ppWindow != NULL) ) {
			(*ppWindow)->tAutoHideStripRect = __xgeXuiDockRectZero();
		}
	}
	fStrip = 23.0f;
	if ( __xgeXuiDockLayoutHasAutoHideRegion(pLayout, XGE_XUI_DOCK_REGION_LEFT) && (tContent.fW > fStrip) ) {
		tStrip = (xge_rect_t){ tContent.fX, tContent.fY, fStrip, tContent.fH };
		__xgeXuiDockLayoutAssignAutoHideStrip(pLayout, XGE_XUI_DOCK_REGION_LEFT, tStrip);
		tContent.fX += fStrip;
		tContent.fW -= fStrip;
	}
	if ( __xgeXuiDockLayoutHasAutoHideRegion(pLayout, XGE_XUI_DOCK_REGION_RIGHT) && (tContent.fW > fStrip) ) {
		tStrip = (xge_rect_t){ tContent.fX + tContent.fW - fStrip, tContent.fY, fStrip, tContent.fH };
		__xgeXuiDockLayoutAssignAutoHideStrip(pLayout, XGE_XUI_DOCK_REGION_RIGHT, tStrip);
		tContent.fW -= fStrip;
	}
	if ( __xgeXuiDockLayoutHasAutoHideRegion(pLayout, XGE_XUI_DOCK_REGION_TOP) && (tContent.fH > fStrip) ) {
		tStrip = (xge_rect_t){ tContent.fX, tContent.fY, tContent.fW, fStrip };
		__xgeXuiDockLayoutAssignAutoHideStrip(pLayout, XGE_XUI_DOCK_REGION_TOP, tStrip);
		tContent.fY += fStrip;
		tContent.fH -= fStrip;
	}
	if ( __xgeXuiDockLayoutHasAutoHideRegion(pLayout, XGE_XUI_DOCK_REGION_BOTTOM) && (tContent.fH > fStrip) ) {
		tStrip = (xge_rect_t){ tContent.fX, tContent.fY + tContent.fH - fStrip, tContent.fW, fStrip };
		__xgeXuiDockLayoutAssignAutoHideStrip(pLayout, XGE_XUI_DOCK_REGION_BOTTOM, tStrip);
		tContent.fH -= fStrip;
	}
	if ( tContent.fW < 0.0f ) {
		tContent.fW = 0.0f;
	}
	if ( tContent.fH < 0.0f ) {
		tContent.fH = 0.0f;
	}
	return tContent;
}

static void __xgeXuiDockLayoutArrangeRegions(xge_xui_dock_layout pLayout, xge_rect_t tContent)
{
	int i;

	if ( pLayout == NULL ) {
		return;
	}
	__xgeXuiDockLayoutHideDockedClients(pLayout);
	tContent = __xgeXuiDockLayoutArrangeAutoHideStrips(pLayout, tContent);
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		if ( pLayout->arrRegions[i].pRoot != NULL ) {
			__xgeXuiDockNodeUpdateMinSize(pLayout, pLayout->arrRegions[i].pRoot);
		}
	}
	__xgeXuiDockLayoutAllocateRegions(pLayout, tContent);
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		if ( __xgeXuiDockRegionActive(&pLayout->arrRegions[i]) ) {
			__xgeXuiDockLayoutArrangeNode(pLayout, pLayout->arrRegions[i].pRoot, pLayout->arrRegions[i].tRect);
		}
	}
}

static void __xgeXuiDockDrawMaskCentered(xge_rect_t tRect, const uint16_t* arrMask, int iW, int iH, uint32_t iColor)
{
	xge_rect_t tGlyph;
	float fSize;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	fSize = (tRect.fW < tRect.fH) ? tRect.fW : tRect.fH;
	if ( fSize > 12.0f ) {
		fSize = 12.0f;
	}
	tGlyph = (xge_rect_t){ tRect.fX + (tRect.fW - fSize) * 0.5f, tRect.fY + (tRect.fH - fSize) * 0.5f, fSize, fSize };
	__xgeXuiHostDrawBitmapMask(tGlyph, arrMask, iW, iH, iColor);
}

static void __xgeXuiDockDrawArrowFallback(xge_rect_t tRect, int iSide, uint32_t iColor)
{
	static const uint16_t arrLeft[8] = { 0x10u, 0x30u, 0x70u, 0xFFu, 0xFFu, 0x70u, 0x30u, 0x10u };
	static const uint16_t arrRight[8] = { 0x08u, 0x0Cu, 0x0Eu, 0xFFu, 0xFFu, 0x0Eu, 0x0Cu, 0x08u };
	static const uint16_t arrTop[8] = { 0x18u, 0x3Cu, 0x7Eu, 0xDBu, 0x18u, 0x18u, 0x18u, 0x18u };
	static const uint16_t arrBottom[8] = { 0x18u, 0x18u, 0x18u, 0x18u, 0xDBu, 0x7Eu, 0x3Cu, 0x18u };

	switch ( iSide ) {
		case XGE_XUI_DOCK_SIDE_LEFT:
			__xgeXuiDockDrawMaskCentered(tRect, arrLeft, 8, 8, iColor);
			break;
		case XGE_XUI_DOCK_SIDE_RIGHT:
			__xgeXuiDockDrawMaskCentered(tRect, arrRight, 8, 8, iColor);
			break;
		case XGE_XUI_DOCK_SIDE_TOP:
			__xgeXuiDockDrawMaskCentered(tRect, arrTop, 8, 8, iColor);
			break;
		case XGE_XUI_DOCK_SIDE_BOTTOM:
			__xgeXuiDockDrawMaskCentered(tRect, arrBottom, 8, 8, iColor);
			break;
		default:
			__xgeXuiHostDrawBorderRect(__xgeXuiDockRectInset(tRect, 4.0f, 4.0f, 4.0f, 4.0f), 1.0f, iColor);
			break;
	}
}

static void __xgeXuiDockDrawButtonFallback(xge_rect_t tRect, int iAsset, uint32_t iColor)
{
	static const uint16_t arrClose[10] = { 0x201u, 0x102u, 0x084u, 0x048u, 0x030u, 0x030u, 0x048u, 0x084u, 0x102u, 0x201u };
	static const uint16_t arrPin[10] = { 0x0FCu, 0x084u, 0x084u, 0x0FCu, 0x030u, 0x030u, 0x030u, 0x078u, 0x030u, 0x030u };
	static const uint16_t arrDock[10] = { 0x1FEu, 0x102u, 0x102u, 0x13Eu, 0x122u, 0x122u, 0x122u, 0x13Eu, 0x100u, 0x1FEu };
	static const uint16_t arrOption[10] = { 0x000u, 0x030u, 0x030u, 0x000u, 0x030u, 0x030u, 0x000u, 0x030u, 0x030u, 0x000u };
	static const uint16_t arrOverflow[10] = { 0x000u, 0x000u, 0x000u, 0x18Cu, 0x18Cu, 0x000u, 0x000u, 0x000u, 0x000u, 0x000u };

	switch ( iAsset ) {
		case XGE_XUI_ASSET_DOCK_PANE_CLOSE:
			__xgeXuiDockDrawMaskCentered(tRect, arrClose, 10, 10, iColor);
			break;
		case XGE_XUI_ASSET_DOCK_PANE_AUTO_HIDE:
			__xgeXuiDockDrawMaskCentered(tRect, arrPin, 10, 10, iColor);
			break;
		case XGE_XUI_ASSET_DOCK_PANE_DOCK:
			__xgeXuiDockDrawMaskCentered(tRect, arrDock, 10, 10, iColor);
			break;
		case XGE_XUI_ASSET_DOCK_PANE_OPTION:
			__xgeXuiDockDrawMaskCentered(tRect, arrOption, 10, 10, iColor);
			break;
		case XGE_XUI_ASSET_DOCK_PANE_OPTION_OVERFLOW:
			__xgeXuiDockDrawMaskCentered(tRect, arrOverflow, 10, 10, iColor);
			break;
		default:
			__xgeXuiHostDrawBorderRect(__xgeXuiDockRectInset(tRect, 3.0f, 3.0f, 3.0f, 3.0f), 1.0f, iColor);
			break;
	}
}

static void __xgeXuiDockDrawPanelIndicatorFallback(xge_rect_t tRect, int iSide)
{
	__xgeXuiHostDrawRect(tRect, XGE_COLOR_RGBA(235, 247, 255, 225));
	__xgeXuiHostDrawBorderRect(tRect, 1.0f, XGE_COLOR_RGBA(57, 135, 196, 230));
	__xgeXuiDockDrawArrowFallback(tRect, iSide, XGE_COLOR_RGBA(22, 101, 163, 235));
}

static void __xgeXuiDockDrawPaneIndicatorTargetFallback(xge_rect_t tRect, int iSide, int iActive)
{
	uint32_t iBack;
	uint32_t iBorder;

	iBack = (iActive != 0) ? XGE_COLOR_RGBA(195, 227, 250, 245) : XGE_COLOR_RGBA(245, 251, 255, 235);
	iBorder = (iActive != 0) ? XGE_COLOR_RGBA(32, 126, 198, 245) : XGE_COLOR_RGBA(114, 168, 205, 235);
	__xgeXuiHostDrawRect(tRect, iBack);
	__xgeXuiHostDrawBorderRect(tRect, 1.0f, iBorder);
	__xgeXuiDockDrawArrowFallback(tRect, iSide, XGE_COLOR_RGBA(28, 99, 153, 240));
}

static void __xgeXuiDockDrawPaneIndicatorFallback(xge_rect_t tRect, int iSide)
{
	xge_rect_t tCenter;
	xge_rect_t tTarget;
	float fCell;
	float fGap;

	__xgeXuiHostDrawRect(tRect, XGE_COLOR_RGBA(255, 255, 255, 155));
	__xgeXuiHostDrawBorderRect(tRect, 1.0f, XGE_COLOR_RGBA(96, 145, 180, 210));
	fCell = (tRect.fW < tRect.fH) ? tRect.fW : tRect.fH;
	fCell = __xgeXuiDockClampFloat(fCell * 0.22f, 14.0f, 20.0f);
	fGap = 3.0f;
	tCenter = (xge_rect_t){ tRect.fX + (tRect.fW - fCell) * 0.5f, tRect.fY + (tRect.fH - fCell) * 0.5f, fCell, fCell };
	__xgeXuiDockDrawPaneIndicatorTargetFallback(tCenter, XGE_XUI_DOCK_SIDE_FILL, iSide == XGE_XUI_DOCK_SIDE_FILL);
	tTarget = (xge_rect_t){ tCenter.fX - fCell - fGap, tCenter.fY, fCell, fCell };
	__xgeXuiDockDrawPaneIndicatorTargetFallback(tTarget, XGE_XUI_DOCK_SIDE_LEFT, iSide == XGE_XUI_DOCK_SIDE_LEFT);
	tTarget = (xge_rect_t){ tCenter.fX + fCell + fGap, tCenter.fY, fCell, fCell };
	__xgeXuiDockDrawPaneIndicatorTargetFallback(tTarget, XGE_XUI_DOCK_SIDE_RIGHT, iSide == XGE_XUI_DOCK_SIDE_RIGHT);
	tTarget = (xge_rect_t){ tCenter.fX, tCenter.fY - fCell - fGap, fCell, fCell };
	__xgeXuiDockDrawPaneIndicatorTargetFallback(tTarget, XGE_XUI_DOCK_SIDE_TOP, iSide == XGE_XUI_DOCK_SIDE_TOP);
	tTarget = (xge_rect_t){ tCenter.fX, tCenter.fY + fCell + fGap, fCell, fCell };
	__xgeXuiDockDrawPaneIndicatorTargetFallback(tTarget, XGE_XUI_DOCK_SIDE_BOTTOM, iSide == XGE_XUI_DOCK_SIDE_BOTTOM);
}

static void __xgeXuiDockPanePaintButton(xge_rect_t tRect, int iAsset, int bHot, int bEnabled)
{
	uint32_t iColor;

	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	if ( (bEnabled != 0) && (bHot != 0) ) {
		__xgeXuiHostDrawRect(__xgeXuiDockRectInset(tRect, -1.0f, -1.0f, -1.0f, -1.0f), XGE_COLOR_RGBA(255, 244, 204, 255));
		__xgeXuiHostDrawBorderRect(__xgeXuiDockRectInset(tRect, -1.0f, -1.0f, -1.0f, -1.0f), 1.0f, XGE_COLOR_RGBA(229, 195, 101, 255));
	}
	iColor = (bEnabled != 0) ? XGE_COLOR_RGBA(255, 255, 255, 255) : XGE_COLOR_RGBA(150, 160, 168, 120);
	if ( __xgeXuiBuiltinAssetDraw(tRect, iAsset, iColor) == 0 ) {
		__xgeXuiDockDrawButtonFallback(tRect, iAsset, iColor);
	}
}

static void __xgeXuiDockPanePaint(xge_xui_dock_pane pPane)
{
	xge_xui_dock_layout pLayout;
	xui_font pFont;
	xge_rect_t tTab;
	xge_rect_t tText;
	xge_xui_dock_window pWindow;
	uint32_t iBack;
	uint32_t iBorder;
	uint32_t iText;
	int i;

	if ( (pPane == NULL) || (pPane->tRect.fW <= 0.0f) || (pPane->tRect.fH <= 0.0f) ) {
		return;
	}
	pLayout = pPane->pLayout;
	pFont = (pLayout != NULL && pLayout->pContext != NULL) ? xgeXuiGetTheme(pLayout->pContext)->pFont : NULL;
	iBorder = XGE_COLOR_RGBA(126, 157, 176, 255);
	iText = XGE_COLOR_RGBA(30, 50, 66, 255);
	__xgeXuiHostDrawRect(pPane->tRect, XGE_COLOR_RGBA(223, 235, 244, 255));
	__xgeXuiHostDrawRect(pPane->tClientRect, XGE_COLOR_RGBA(255, 255, 255, 255));
	__xgeXuiHostDrawRect(pPane->tTabStripRect, XGE_COLOR_RGBA(213, 228, 238, 255));
	for ( i = 0; i < (int)pPane->arrWindows.Count; i++ ) {
		pWindow = xgeXuiDockPaneGetWindow(pPane, i);
		tTab = __xgeXuiDockPaneTabRect(pPane, i);
		if ( (pWindow == NULL) || (tTab.fW <= 0.0f) || (tTab.fH <= 0.0f) ) {
			continue;
		}
		if ( i == pPane->iActive ) {
			tTab.fY -= 3.0f;
			tTab.fH += 3.0f;
			iBack = XGE_COLOR_RGBA(255, 255, 255, 255);
		} else if ( pPane->iHoverPart == i ) {
			iBack = XGE_COLOR_RGBA(244, 249, 252, 255);
		} else {
			iBack = XGE_COLOR_RGBA(232, 240, 246, 255);
		}
		__xgeXuiHostDrawRect(tTab, iBack);
		__xgeXuiHostDrawBorderRect(tTab, 1.0f, iBorder);
		tText = __xgeXuiDockRectInset(tTab, 8.0f, 0.0f, 8.0f, 0.0f);
		__xgeXuiDockPaneDrawTabTitle(pFont, pWindow->sTitle != NULL ? pWindow->sTitle : "", tText, iText);
	}
	__xgeXuiDockPanePaintButton(pPane->tOverflowRect, XGE_XUI_ASSET_DOCK_PANE_OPTION_OVERFLOW, pPane->iHoverPart == XGE_XUI_DOCK_PART_OVERFLOW || pPane->iActivePart == XGE_XUI_DOCK_PART_OVERFLOW, 1);
	__xgeXuiDockPanePaintButton(pPane->tCloseRect, XGE_XUI_ASSET_DOCK_PANE_CLOSE, pPane->iHoverPart == XGE_XUI_DOCK_PART_CLOSE || pPane->iActivePart == XGE_XUI_DOCK_PART_CLOSE, 1);
	__xgeXuiDockPanePaintButton(pPane->tAutoHideRect, XGE_XUI_ASSET_DOCK_PANE_AUTO_HIDE, pPane->iHoverPart == XGE_XUI_DOCK_PART_AUTO_HIDE || pPane->iActivePart == XGE_XUI_DOCK_PART_AUTO_HIDE, __xgeXuiDockPaneCanAutoHide(pPane));
	__xgeXuiDockPanePaintButton(pPane->tOptionRect, XGE_XUI_ASSET_DOCK_PANE_OPTION, pPane->iHoverPart == XGE_XUI_DOCK_PART_OPTION || pPane->iActivePart == XGE_XUI_DOCK_PART_OPTION, 1);
	__xgeXuiHostDrawBorderRect(pPane->tRect, 1.0f, iBorder);
}

static void __xgeXuiDockLayoutPaintNode(xge_xui_dock_node pNode)
{
	if ( pNode == NULL ) {
		return;
	}
	if ( pNode->iType == XGE_XUI_DOCK_NODE_PANE ) {
		__xgeXuiDockPanePaint(pNode->pPane);
		return;
	}
	if ( pNode->iType == XGE_XUI_DOCK_NODE_SPLIT ) {
		__xgeXuiDockLayoutPaintNode(pNode->pFirst);
		__xgeXuiDockLayoutPaintNode(pNode->pSecond);
		__xgeXuiHostDrawRect(pNode->tSplitterRect, XGE_COLOR_RGBA(196, 214, 226, 255));
		__xgeXuiHostDrawBorderRect(pNode->tSplitterRect, 1.0f, XGE_COLOR_RGBA(177, 198, 212, 255));
	}
}

static void __xgeXuiDockLayoutPaintAutoHideStrips(xge_xui_dock_layout pLayout)
{
	xge_xui_dock_window* ppWindow;
	xge_xui_dock_window pWindow;
	xui_font pFont;
	xge_rect_t tRect;
	xge_rect_t tIcon;
	xge_rect_t tText;
	uint32_t iBack;
	uint32_t iBorder;
	uint32 i;

	if ( pLayout == NULL ) {
		return;
	}
	pFont = (pLayout->pContext != NULL) ? xgeXuiGetTheme(pLayout->pContext)->pFont : NULL;
	for ( i = 0; i < pLayout->arrWindows.Count; i++ ) {
		ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline(&pLayout->arrWindows, i + 1u);
		pWindow = (ppWindow != NULL) ? *ppWindow : NULL;
		if ( (pWindow == NULL) || (pWindow->iState != XGE_XUI_DOCK_WINDOW_AUTO_HIDE) || (pWindow->tAutoHideStripRect.fW <= 0.0f) || (pWindow->tAutoHideStripRect.fH <= 0.0f) ) {
			continue;
		}
		tRect = pWindow->tAutoHideStripRect;
		iBack = (pLayout->pAutoHideHoverWindow == pWindow || pLayout->pAutoHideActiveWindow == pWindow) ? XGE_COLOR_RGBA(255, 244, 204, 255) : XGE_COLOR_RGBA(218, 232, 242, 255);
		iBorder = (pLayout->pAutoHideHoverWindow == pWindow || pLayout->pAutoHideActiveWindow == pWindow) ? XGE_COLOR_RGBA(229, 195, 101, 255) : XGE_COLOR_RGBA(141, 174, 194, 255);
		__xgeXuiHostDrawRect(tRect, iBack);
		__xgeXuiHostDrawBorderRect(tRect, 1.0f, iBorder);
		tIcon = (xge_rect_t){ tRect.fX + 3.0f, tRect.fY + 3.0f, 16.0f, 15.0f };
		if ( tIcon.fX + tIcon.fW <= tRect.fX + tRect.fW && tIcon.fY + tIcon.fH <= tRect.fY + tRect.fH ) {
		if ( __xgeXuiBuiltinAssetDraw(tIcon, XGE_XUI_ASSET_DOCK_PANE_DOCK, XGE_COLOR_RGBA(255, 255, 255, 255)) == 0 ) {
			__xgeXuiDockDrawButtonFallback(tIcon, XGE_XUI_ASSET_DOCK_PANE_DOCK, XGE_COLOR_RGBA(255, 255, 255, 255));
		}
		}
		tText = __xgeXuiDockRectInset(tRect, 22.0f, 0.0f, 4.0f, 0.0f);
		if ( tText.fW > 8.0f ) {
			__xgeXuiDockPaneDrawTabTitle(pFont, pWindow->sTitle != NULL ? pWindow->sTitle : "", tText, XGE_COLOR_RGBA(31, 57, 72, 255));
		}
	}
}

static xge_xui_dock_window __xgeXuiDockLayoutHitAutoHideWindow(xge_xui_dock_layout pLayout, float fX, float fY)
{
	xge_xui_dock_window* ppWindow;
	uint32 i;

	if ( pLayout == NULL ) {
		return NULL;
	}
	for ( i = 0; i < pLayout->arrWindows.Count; i++ ) {
		ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline(&pLayout->arrWindows, i + 1u);
		if ( (ppWindow != NULL) && (*ppWindow != NULL) && ((*ppWindow)->iState == XGE_XUI_DOCK_WINDOW_AUTO_HIDE) && __xgeXuiDockRectContains((*ppWindow)->tAutoHideStripRect, fX, fY) ) {
			return *ppWindow;
		}
	}
	return NULL;
}

static xge_vec2_t __xgeXuiDockLayoutMeasureProc(xge_xui_widget pWidget, void* pUser)
{
	xge_vec2_t tSize;

	(void)pUser;
	tSize.fX = (pWidget != NULL && pWidget->tLocalRect.fW > 0.0f) ? pWidget->tLocalRect.fW : 320.0f;
	tSize.fY = (pWidget != NULL && pWidget->tLocalRect.fH > 0.0f) ? pWidget->tLocalRect.fH : 240.0f;
	return tSize;
}

static void __xgeXuiDockLayoutLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_dock_layout pLayout;

	pLayout = (xge_xui_dock_layout)pUser;
	if ( (pWidget == NULL) || (pLayout == NULL) ) {
		return;
	}
	__xgeXuiDockLayoutArrangeRegions(pLayout, pWidget->tContentRect);
	__xgeXuiDockLayoutSyncAutoHideOverlayRect(pLayout);
	__xgeXuiDockLayoutArrangeAutoHideExpand(pLayout);
}

static void __xgeXuiDockLayoutPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_dock_layout pLayout;
	int i;

	(void)pWidget;
	pLayout = (xge_xui_dock_layout)pUser;
	if ( pLayout == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		if ( __xgeXuiDockRegionActive(&pLayout->arrRegions[i]) ) {
			__xgeXuiDockLayoutPaintNode(pLayout->arrRegions[i].pRoot);
		}
	}
	__xgeXuiDockLayoutPaintAutoHideStrips(pLayout);
}

static xge_rect_t __xgeXuiDockLayoutOverlayRect(xge_xui_dock_layout pLayout)
{
	if ( (pLayout != NULL) && (pLayout->pContext != NULL) && (pLayout->pContext->pRoot != NULL) ) {
		return pLayout->pContext->pRoot->tRect;
	}
	if ( (pLayout != NULL) && (pLayout->pWidget != NULL) ) {
		return pLayout->pWidget->tRect;
	}
	return __xgeXuiDockRectZero();
}

static void __xgeXuiDockLayoutSyncDragOverlayRect(xge_xui_dock_layout pLayout)
{
	if ( (pLayout == NULL) || (pLayout->pDragOverlayWidget == NULL) ) {
		return;
	}
	xgeXuiWidgetSetRect(pLayout->pDragOverlayWidget, __xgeXuiDockLayoutOverlayRect(pLayout));
}

static int __xgeXuiDockLayoutEnsureDragOverlay(xge_xui_dock_layout pLayout)
{
	xge_xui_widget pOverlay;

	if ( (pLayout == NULL) || (pLayout->pContext == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pLayout->pDragOverlayWidget != NULL ) {
		__xgeXuiDockLayoutSyncDragOverlayRect(pLayout);
		return XGE_OK;
	}
	pOverlay = xgeXuiWidgetCreate();
	if ( pOverlay == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetLayout(pOverlay, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pOverlay, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeXuiWidgetSetInputTransparent(pOverlay, 1);
	xgeXuiWidgetSetPaint(pOverlay, __xgeXuiDockLayoutOverlayPaintProc, pLayout);
	xgeXuiWidgetSetVisible(pOverlay, 0);
	pLayout->pDragOverlayWidget = pOverlay;
	__xgeXuiDockLayoutSyncDragOverlayRect(pLayout);
	if ( xgeXuiOverlayAttach(pLayout->pContext, pOverlay, pLayout->pWidget, XGE_XUI_LAYER_DRAG_ADORNER) != XGE_OK ) {
		xgeXuiWidgetFree(pOverlay);
		pLayout->pDragOverlayWidget = NULL;
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void __xgeXuiDockLayoutSetDragOverlayVisible(xge_xui_dock_layout pLayout, int bVisible)
{
	if ( (pLayout == NULL) || (pLayout->pDragOverlayWidget == NULL) ) {
		return;
	}
	xgeXuiWidgetSetVisible(pLayout->pDragOverlayWidget, bVisible);
	xgeXuiWidgetMarkPaint(pLayout->pDragOverlayWidget);
	if ( pLayout->pContext != NULL && pLayout->pContext->pOverlayRoot != NULL ) {
		xgeXuiWidgetMarkPaint(pLayout->pContext->pOverlayRoot);
	}
}

static xge_rect_t __xgeXuiDockLayoutAutoHideExpandResolveRect(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow)
{
	xge_rect_t tContent;
	xge_rect_t tStrip;
	xge_rect_t tRect;
	float fSize;
	float fMinSize;
	float fMaxSize;

	if ( (pLayout == NULL) || (pLayout->pWidget == NULL) || (pWindow == NULL) ) {
		return __xgeXuiDockRectZero();
	}
	tContent = pLayout->pWidget->tContentRect;
	tStrip = pWindow->tAutoHideStripRect;
	if ( (tStrip.fW <= 0.0f) || (tStrip.fH <= 0.0f) ) {
		return __xgeXuiDockRectZero();
	}
	tRect = __xgeXuiDockRectZero();
	if ( (pWindow->iAutoHideRegion == XGE_XUI_DOCK_REGION_LEFT) || (pWindow->iAutoHideRegion == XGE_XUI_DOCK_REGION_RIGHT) ) {
		fMinSize = 160.0f;
		fMaxSize = __xgeXuiDockClampFloat(tContent.fW - tStrip.fW - 8.0f, fMinSize, 420.0f);
		fSize = (pWindow->tLastDockRect.fW > 0.0f) ? pWindow->tLastDockRect.fW : tContent.fW * 0.33f;
		fSize = __xgeXuiDockClampFloat(fSize, fMinSize, fMaxSize);
		if ( pWindow->iAutoHideRegion == XGE_XUI_DOCK_REGION_LEFT ) {
			tRect = (xge_rect_t){ tStrip.fX + tStrip.fW, tContent.fY, fSize, tContent.fH };
			if ( tRect.fX + tRect.fW > tContent.fX + tContent.fW ) {
				tRect.fW = tContent.fX + tContent.fW - tRect.fX;
			}
		} else {
			tRect = (xge_rect_t){ tStrip.fX - fSize, tContent.fY, fSize, tContent.fH };
			if ( tRect.fX < tContent.fX ) {
				tRect.fW -= tContent.fX - tRect.fX;
				tRect.fX = tContent.fX;
			}
		}
	} else if ( (pWindow->iAutoHideRegion == XGE_XUI_DOCK_REGION_TOP) || (pWindow->iAutoHideRegion == XGE_XUI_DOCK_REGION_BOTTOM) ) {
		fMinSize = 120.0f;
		fMaxSize = __xgeXuiDockClampFloat(tContent.fH - tStrip.fH - 8.0f, fMinSize, 320.0f);
		fSize = (pWindow->tLastDockRect.fH > 0.0f) ? pWindow->tLastDockRect.fH : tContent.fH * 0.36f;
		fSize = __xgeXuiDockClampFloat(fSize, fMinSize, fMaxSize);
		if ( pWindow->iAutoHideRegion == XGE_XUI_DOCK_REGION_TOP ) {
			tRect = (xge_rect_t){ tContent.fX, tStrip.fY + tStrip.fH, tContent.fW, fSize };
			if ( tRect.fY + tRect.fH > tContent.fY + tContent.fH ) {
				tRect.fH = tContent.fY + tContent.fH - tRect.fY;
			}
		} else {
			tRect = (xge_rect_t){ tContent.fX, tStrip.fY - fSize, tContent.fW, fSize };
			if ( tRect.fY < tContent.fY ) {
				tRect.fH -= tContent.fY - tRect.fY;
				tRect.fY = tContent.fY;
			}
		}
	}
	if ( (tRect.fW < 48.0f) || (tRect.fH < 48.0f) ) {
		return __xgeXuiDockRectZero();
	}
	return tRect;
}

static void __xgeXuiDockLayoutArrangeAutoHideExpand(xge_xui_dock_layout pLayout)
{
	xge_xui_dock_window pWindow;
	xge_rect_t tRect;
	float fCaptionH;
	float fButtonW;
	float fButtonH;
	float fButtonY;
	float fButtonX;

	if ( pLayout == NULL ) {
		return;
	}
	pWindow = pLayout->pAutoHideExpandWindow;
	if ( (pWindow == NULL) || (pWindow->pLayout != pLayout) || (pWindow->iState != XGE_XUI_DOCK_WINDOW_AUTO_HIDE) ) {
		pLayout->tAutoHideExpandRect = __xgeXuiDockRectZero();
		pLayout->tAutoHideExpandCaptionRect = __xgeXuiDockRectZero();
		pLayout->tAutoHideExpandClientRect = __xgeXuiDockRectZero();
		pLayout->tAutoHideExpandDockRect = __xgeXuiDockRectZero();
		pLayout->tAutoHideExpandCloseRect = __xgeXuiDockRectZero();
		return;
	}
	tRect = __xgeXuiDockLayoutAutoHideExpandResolveRect(pLayout, pWindow);
	pLayout->tAutoHideExpandRect = tRect;
	pLayout->tAutoHideExpandCaptionRect = __xgeXuiDockRectZero();
	pLayout->tAutoHideExpandClientRect = __xgeXuiDockRectZero();
	pLayout->tAutoHideExpandDockRect = __xgeXuiDockRectZero();
	pLayout->tAutoHideExpandCloseRect = __xgeXuiDockRectZero();
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	fCaptionH = __xgeXuiDockLayoutCaptionHeight(pLayout);
	if ( fCaptionH > tRect.fH ) {
		fCaptionH = tRect.fH;
	}
	pLayout->tAutoHideExpandCaptionRect = (xge_rect_t){ tRect.fX + 1.0f, tRect.fY + 1.0f, tRect.fW - 2.0f, fCaptionH };
	pLayout->tAutoHideExpandClientRect = (xge_rect_t){ tRect.fX + 1.0f, pLayout->tAutoHideExpandCaptionRect.fY + pLayout->tAutoHideExpandCaptionRect.fH, tRect.fW - 2.0f, tRect.fH - pLayout->tAutoHideExpandCaptionRect.fH - 2.0f };
	if ( pLayout->tAutoHideExpandClientRect.fW < 0.0f ) {
		pLayout->tAutoHideExpandClientRect.fW = 0.0f;
	}
	if ( pLayout->tAutoHideExpandClientRect.fH < 0.0f ) {
		pLayout->tAutoHideExpandClientRect.fH = 0.0f;
	}
	fButtonW = __xgeXuiDockLayoutButtonWidth(pLayout);
	fButtonH = __xgeXuiDockLayoutButtonHeight(pLayout);
	if ( fButtonH > fCaptionH ) {
		fButtonH = fCaptionH;
	}
	fButtonY = pLayout->tAutoHideExpandCaptionRect.fY + __xgeXuiDockButtonTop(fCaptionH, fButtonH);
	fButtonX = pLayout->tAutoHideExpandCaptionRect.fX + pLayout->tAutoHideExpandCaptionRect.fW - fButtonW - 4.0f;
	if ( fButtonX >= pLayout->tAutoHideExpandCaptionRect.fX ) {
		pLayout->tAutoHideExpandCloseRect = (xge_rect_t){ fButtonX, fButtonY, fButtonW, fButtonH };
		fButtonX -= fButtonW + 2.0f;
	}
	if ( fButtonX >= pLayout->tAutoHideExpandCaptionRect.fX ) {
		pLayout->tAutoHideExpandDockRect = (xge_rect_t){ fButtonX, fButtonY, fButtonW, fButtonH };
	}
	if ( (pWindow->pClientWidget != NULL) && (pLayout->pAutoHideOverlayWidget != NULL) ) {
		__xgeXuiDockWindowAttachClientToAutoHideOverlay(pLayout, pWindow);
		__xgeXuiDockWidgetArrange(pWindow->pClientWidget, pLayout->tAutoHideExpandClientRect);
		xgeXuiWidgetSetVisible(pWindow->pClientWidget, 1);
	}
}

static void __xgeXuiDockLayoutSyncAutoHideOverlayRect(xge_xui_dock_layout pLayout)
{
	if ( (pLayout == NULL) || (pLayout->pAutoHideOverlayWidget == NULL) ) {
		return;
	}
	xgeXuiWidgetSetRect(pLayout->pAutoHideOverlayWidget, __xgeXuiDockLayoutOverlayRect(pLayout));
}

static int __xgeXuiDockLayoutEnsureAutoHideOverlay(xge_xui_dock_layout pLayout)
{
	xge_xui_widget pOverlay;

	if ( (pLayout == NULL) || (pLayout->pContext == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pLayout->pAutoHideOverlayWidget != NULL ) {
		__xgeXuiDockLayoutSyncAutoHideOverlayRect(pLayout);
		return XGE_OK;
	}
	pOverlay = xgeXuiWidgetCreate();
	if ( pOverlay == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetLayout(pOverlay, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pOverlay, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeXuiWidgetSetClip(pOverlay, 0);
	xgeXuiWidgetSetPaint(pOverlay, __xgeXuiDockLayoutAutoHideOverlayPaintProc, pLayout);
	xgeXuiWidgetSetEvent(pOverlay, __xgeXuiDockLayoutAutoHideOverlayEventProc, pLayout);
	xgeXuiWidgetSetVisible(pOverlay, 0);
	pLayout->pAutoHideOverlayWidget = pOverlay;
	__xgeXuiDockLayoutSyncAutoHideOverlayRect(pLayout);
	if ( xgeXuiOverlayAttach(pLayout->pContext, pOverlay, pLayout->pWidget, XGE_XUI_LAYER_DRAG_ADORNER) != XGE_OK ) {
		xgeXuiWidgetFree(pOverlay);
		pLayout->pAutoHideOverlayWidget = NULL;
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void __xgeXuiDockLayoutSetAutoHideOverlayVisible(xge_xui_dock_layout pLayout, int bVisible)
{
	if ( (pLayout == NULL) || (pLayout->pAutoHideOverlayWidget == NULL) ) {
		return;
	}
	xgeXuiWidgetSetVisible(pLayout->pAutoHideOverlayWidget, bVisible);
	xgeXuiWidgetMarkLayout(pLayout->pAutoHideOverlayWidget);
	xgeXuiWidgetMarkPaint(pLayout->pAutoHideOverlayWidget);
	if ( pLayout->pContext != NULL && pLayout->pContext->pOverlayRoot != NULL ) {
		xgeXuiWidgetMarkPaint(pLayout->pContext->pOverlayRoot);
	}
}

static int __xgeXuiDockLayoutOpenAutoHideExpand(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow)
{
	if ( (pLayout == NULL) || (pWindow == NULL) || (pWindow->pLayout != pLayout) || (pWindow->iState != XGE_XUI_DOCK_WINDOW_AUTO_HIDE) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( __xgeXuiDockLayoutEnsureAutoHideOverlay(pLayout) != XGE_OK ) {
		return XGE_ERROR;
	}
	__xgeXuiDockLayoutCloseOptionMenu(pLayout);
	__xgeXuiDockLayoutCloseOverflowMenu(pLayout);
	if ( pLayout->pAutoHideExpandWindow != pWindow ) {
		__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
	}
	pLayout->pAutoHideExpandWindow = pWindow;
	pLayout->iAutoHideExpandHoverPart = XGE_XUI_DOCK_PART_NONE;
	pLayout->iAutoHideExpandActivePart = XGE_XUI_DOCK_PART_NONE;
	__xgeXuiDockLayoutSyncAutoHideOverlayRect(pLayout);
	__xgeXuiDockLayoutArrangeAutoHideExpand(pLayout);
	if ( (pLayout->tAutoHideExpandRect.fW <= 0.0f) || (pLayout->tAutoHideExpandRect.fH <= 0.0f) ) {
		__xgeXuiDockWindowAttachClientToBase(pWindow);
		pLayout->pAutoHideExpandWindow = NULL;
		return XGE_ERROR;
	}
	xgeXuiOverlayBringToFront(pLayout->pContext, pLayout->pAutoHideOverlayWidget);
	__xgeXuiDockLayoutSetAutoHideOverlayVisible(pLayout, 1);
	(void)__xgeXuiDockWindowFocusNow(pWindow);
	xgeXuiWidgetMarkPaint(pLayout->pWidget);
	return XGE_OK;
}

static void __xgeXuiDockLayoutCloseAutoHideExpand(xge_xui_dock_layout pLayout)
{
	xge_xui_dock_window pWindow;

	if ( pLayout == NULL ) {
		return;
	}
	pWindow = pLayout->pAutoHideExpandWindow;
	if ( (pWindow != NULL) && (pWindow->iState == XGE_XUI_DOCK_WINDOW_AUTO_HIDE) ) {
		__xgeXuiDockWindowAttachClientToBase(pWindow);
	}
	pLayout->pAutoHideExpandWindow = NULL;
	pLayout->iAutoHideExpandHoverPart = XGE_XUI_DOCK_PART_NONE;
	pLayout->iAutoHideExpandActivePart = XGE_XUI_DOCK_PART_NONE;
	pLayout->tAutoHideExpandRect = __xgeXuiDockRectZero();
	pLayout->tAutoHideExpandCaptionRect = __xgeXuiDockRectZero();
	pLayout->tAutoHideExpandClientRect = __xgeXuiDockRectZero();
	pLayout->tAutoHideExpandDockRect = __xgeXuiDockRectZero();
	pLayout->tAutoHideExpandCloseRect = __xgeXuiDockRectZero();
	__xgeXuiDockLayoutSetAutoHideOverlayVisible(pLayout, 0);
}

static int __xgeXuiDockLayoutAutoHideExpandHitPart(xge_xui_dock_layout pLayout, float fX, float fY)
{
	if ( (pLayout == NULL) || (pLayout->pAutoHideExpandWindow == NULL) ) {
		return XGE_XUI_DOCK_PART_NONE;
	}
	if ( __xgeXuiDockRectContains(pLayout->tAutoHideExpandCloseRect, fX, fY) ) {
		return XGE_XUI_DOCK_PART_CLOSE;
	}
	if ( __xgeXuiDockRectContains(pLayout->tAutoHideExpandDockRect, fX, fY) ) {
		return XGE_XUI_DOCK_PART_AUTO_HIDE;
	}
	if ( __xgeXuiDockRectContains(pLayout->tAutoHideExpandCaptionRect, fX, fY) ) {
		return XGE_XUI_DOCK_PART_CAPTION;
	}
	if ( __xgeXuiDockRectContains(pLayout->tAutoHideExpandRect, fX, fY) ) {
		return XGE_XUI_DOCK_SIDE_FILL;
	}
	return XGE_XUI_DOCK_PART_NONE;
}

static void __xgeXuiDockLayoutAutoHideOverlayPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_dock_layout pLayout;
	xge_xui_dock_window pWindow;
	xui_font pFont;
	xge_rect_t tText;

	(void)pWidget;
	pLayout = (xge_xui_dock_layout)pUser;
	if ( (pLayout == NULL) || (pLayout->pAutoHideExpandWindow == NULL) || (pLayout->tAutoHideExpandRect.fW <= 0.0f) || (pLayout->tAutoHideExpandRect.fH <= 0.0f) ) {
		return;
	}
	pWindow = pLayout->pAutoHideExpandWindow;
	pFont = (pLayout->pContext != NULL) ? xgeXuiGetTheme(pLayout->pContext)->pFont : NULL;
	__xgeXuiHostDrawRect((xge_rect_t){ pLayout->tAutoHideExpandRect.fX + 3.0f, pLayout->tAutoHideExpandRect.fY + 3.0f, pLayout->tAutoHideExpandRect.fW, pLayout->tAutoHideExpandRect.fH }, XGE_COLOR_RGBA(70, 94, 112, 54));
	__xgeXuiHostDrawRect(pLayout->tAutoHideExpandRect, XGE_COLOR_RGBA(223, 235, 244, 255));
	__xgeXuiHostDrawRect(pLayout->tAutoHideExpandClientRect, XGE_COLOR_RGBA(255, 255, 255, 255));
	__xgeXuiHostDrawRect(pLayout->tAutoHideExpandCaptionRect, XGE_COLOR_RGBA(213, 228, 238, 255));
	tText = __xgeXuiDockRectInset(pLayout->tAutoHideExpandCaptionRect, 8.0f, 0.0f, 44.0f, 0.0f);
	__xgeXuiDockPaneDrawTabTitle(pFont, (pWindow->sTitle != NULL) ? pWindow->sTitle : "", tText, XGE_COLOR_RGBA(31, 57, 72, 255));
	__xgeXuiDockPanePaintButton(pLayout->tAutoHideExpandDockRect, XGE_XUI_ASSET_DOCK_PANE_DOCK, (pLayout->iAutoHideExpandHoverPart == XGE_XUI_DOCK_PART_AUTO_HIDE) || (pLayout->iAutoHideExpandActivePart == XGE_XUI_DOCK_PART_AUTO_HIDE), 1);
	__xgeXuiDockPanePaintButton(pLayout->tAutoHideExpandCloseRect, XGE_XUI_ASSET_DOCK_PANE_CLOSE, (pLayout->iAutoHideExpandHoverPart == XGE_XUI_DOCK_PART_CLOSE) || (pLayout->iAutoHideExpandActivePart == XGE_XUI_DOCK_PART_CLOSE), 1);
	__xgeXuiHostDrawBorderRect(pLayout->tAutoHideExpandRect, 1.0f, XGE_COLOR_RGBA(103, 152, 179, 255));
}

static int __xgeXuiDockLayoutAutoHideOverlayEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	xge_xui_dock_layout pLayout;
	xge_xui_dock_window pWindow;
	int iPart;
	int iActivePart;

	(void)pWidget;
	pLayout = (xge_xui_dock_layout)pUser;
	if ( (pLayout == NULL) || (pEvent == NULL) || (pLayout->pAutoHideExpandWindow == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	pWindow = pLayout->pAutoHideExpandWindow;
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			iPart = __xgeXuiDockLayoutAutoHideExpandHitPart(pLayout, pEvent->fX, pEvent->fY);
			if ( iPart != pLayout->iAutoHideExpandHoverPart ) {
				pLayout->iAutoHideExpandHoverPart = iPart;
				if ( pLayout->pAutoHideOverlayWidget != NULL ) {
					xgeXuiWidgetMarkPaint(pLayout->pAutoHideOverlayWidget);
				}
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 != XGE_MOUSE_LEFT) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iPart = __xgeXuiDockLayoutAutoHideExpandHitPart(pLayout, pEvent->fX, pEvent->fY);
			if ( iPart == XGE_XUI_DOCK_PART_NONE ) {
				__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( (iPart == XGE_XUI_DOCK_PART_CLOSE) || (iPart == XGE_XUI_DOCK_PART_AUTO_HIDE) ) {
				pLayout->iAutoHideExpandActivePart = iPart;
				pLayout->iAutoHideExpandHoverPart = iPart;
				if ( pLayout->pContext != NULL && pLayout->pAutoHideOverlayWidget != NULL ) {
					xgeXuiSetPointerCapture(pLayout->pContext, pEvent->iPointerId, pLayout->pAutoHideOverlayWidget);
				}
				if ( pLayout->pAutoHideOverlayWidget != NULL ) {
					xgeXuiWidgetMarkPaint(pLayout->pAutoHideOverlayWidget);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return (iPart == XGE_XUI_DOCK_SIDE_FILL) ? XGE_XUI_EVENT_CONTINUE : XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			iActivePart = pLayout->iAutoHideExpandActivePart;
			if ( iActivePart != XGE_XUI_DOCK_PART_NONE ) {
				iPart = __xgeXuiDockLayoutAutoHideExpandHitPart(pLayout, pEvent->fX, pEvent->fY);
				if ( pLayout->pContext != NULL && pLayout->pAutoHideOverlayWidget != NULL && xgeXuiGetPointerCapture(pLayout->pContext, pEvent->iPointerId) == pLayout->pAutoHideOverlayWidget ) {
					xgeXuiSetPointerCapture(pLayout->pContext, pEvent->iPointerId, NULL);
				}
				pLayout->iAutoHideExpandActivePart = XGE_XUI_DOCK_PART_NONE;
				pLayout->iAutoHideExpandHoverPart = iPart;
				if ( iPart == iActivePart ) {
					if ( iActivePart == XGE_XUI_DOCK_PART_AUTO_HIDE ) {
						__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
						(void)xgeXuiDockLayoutDockAutoHideWindow(pLayout, pWindow);
					} else if ( iActivePart == XGE_XUI_DOCK_PART_CLOSE ) {
						__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
					}
				}
				if ( pLayout->pWidget != NULL ) {
					xgeXuiWidgetMarkLayout(pLayout->pWidget);
					xgeXuiWidgetMarkPaint(pLayout->pWidget);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pLayout->iAutoHideExpandActivePart = XGE_XUI_DOCK_PART_NONE;
			if ( pLayout->pAutoHideOverlayWidget != NULL ) {
				xgeXuiWidgetMarkPaint(pLayout->pAutoHideOverlayWidget);
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( pEvent->iParam1 == XGE_KEY_ESCAPE ) {
				__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		default:
			break;
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static float __xgeXuiDockEdgeSize(float fSize)
{
	if ( fSize < 120.0f ) {
		return fSize * 0.50f;
	}
	if ( fSize > 360.0f ) {
		return 90.0f;
	}
	return fSize * 0.25f;
}

static int __xgeXuiDockDropSideFromRect(xge_rect_t tRect, float fX, float fY)
{
	float fEdgeX;
	float fEdgeY;

	if ( !__xgeXuiDockRectContains(tRect, fX, fY) ) {
		return XGE_XUI_DOCK_SIDE_NONE;
	}
	fEdgeX = __xgeXuiDockEdgeSize(tRect.fW);
	fEdgeY = __xgeXuiDockEdgeSize(tRect.fH);
	if ( fX < tRect.fX + fEdgeX ) {
		return XGE_XUI_DOCK_SIDE_LEFT;
	}
	if ( fX >= tRect.fX + tRect.fW - fEdgeX ) {
		return XGE_XUI_DOCK_SIDE_RIGHT;
	}
	if ( fY < tRect.fY + fEdgeY ) {
		return XGE_XUI_DOCK_SIDE_TOP;
	}
	if ( fY >= tRect.fY + tRect.fH - fEdgeY ) {
		return XGE_XUI_DOCK_SIDE_BOTTOM;
	}
	return XGE_XUI_DOCK_SIDE_FILL;
}

static int __xgeXuiDockGlobalSideFromRect(xge_rect_t tRect, float fX, float fY)
{
	float fEdgeX;
	float fEdgeY;

	if ( !__xgeXuiDockRectContains(tRect, fX, fY) ) {
		return XGE_XUI_DOCK_SIDE_NONE;
	}
	fEdgeX = __xgeXuiDockEdgeSize(tRect.fW);
	fEdgeY = __xgeXuiDockEdgeSize(tRect.fH);
	if ( fX < tRect.fX + fEdgeX ) {
		return XGE_XUI_DOCK_SIDE_LEFT;
	}
	if ( fX >= tRect.fX + tRect.fW - fEdgeX ) {
		return XGE_XUI_DOCK_SIDE_RIGHT;
	}
	if ( fY < tRect.fY + fEdgeY ) {
		return XGE_XUI_DOCK_SIDE_TOP;
	}
	if ( fY >= tRect.fY + tRect.fH - fEdgeY ) {
		return XGE_XUI_DOCK_SIDE_BOTTOM;
	}
	return XGE_XUI_DOCK_SIDE_NONE;
}

static int __xgeXuiDockRegionFromGlobalSide(int iSide)
{
	switch ( iSide ) {
		case XGE_XUI_DOCK_SIDE_LEFT: return XGE_XUI_DOCK_REGION_LEFT;
		case XGE_XUI_DOCK_SIDE_RIGHT: return XGE_XUI_DOCK_REGION_RIGHT;
		case XGE_XUI_DOCK_SIDE_TOP: return XGE_XUI_DOCK_REGION_TOP;
		case XGE_XUI_DOCK_SIDE_BOTTOM: return XGE_XUI_DOCK_REGION_BOTTOM;
		default: return XGE_XUI_DOCK_REGION_DOCUMENT;
	}
}

static xge_rect_t __xgeXuiDockPreviewRectForSide(xge_rect_t tBase, int iSide)
{
	float fSize;

	if ( iSide == XGE_XUI_DOCK_SIDE_FILL ) {
		return __xgeXuiDockRectInset(tBase, 4.0f, 4.0f, 4.0f, 4.0f);
	}
	if ( iSide == XGE_XUI_DOCK_SIDE_LEFT || iSide == XGE_XUI_DOCK_SIDE_RIGHT ) {
		fSize = __xgeXuiDockClampFloat(tBase.fW * 0.30f, 72.0f, tBase.fW - 24.0f);
		if ( iSide == XGE_XUI_DOCK_SIDE_RIGHT ) {
			tBase.fX += tBase.fW - fSize;
		}
		tBase.fW = fSize;
		return tBase;
	}
	if ( iSide == XGE_XUI_DOCK_SIDE_TOP || iSide == XGE_XUI_DOCK_SIDE_BOTTOM ) {
		fSize = __xgeXuiDockClampFloat(tBase.fH * 0.30f, 48.0f, tBase.fH - 24.0f);
		if ( iSide == XGE_XUI_DOCK_SIDE_BOTTOM ) {
			tBase.fY += tBase.fH - fSize;
		}
		tBase.fH = fSize;
		return tBase;
	}
	return __xgeXuiDockRectZero();
}

static xge_rect_t __xgeXuiDockIndicatorRect(xge_rect_t tBase, float fW, float fH)
{
	return (xge_rect_t){ tBase.fX + (tBase.fW - fW) * 0.5f, tBase.fY + (tBase.fH - fH) * 0.5f, fW, fH };
}

static xge_rect_t __xgeXuiDockPanelIndicatorRect(xge_rect_t tBase, int iSide)
{
	xge_rect_t tRect;
	float fSize;
	float fMargin;

	fSize = 31.0f;
	fMargin = 18.0f;
	tRect = __xgeXuiDockIndicatorRect(tBase, fSize, fSize);
	switch ( iSide ) {
		case XGE_XUI_DOCK_SIDE_LEFT:
			tRect.fX = tBase.fX + fMargin;
			break;
		case XGE_XUI_DOCK_SIDE_RIGHT:
			tRect.fX = tBase.fX + tBase.fW - fSize - fMargin;
			break;
		case XGE_XUI_DOCK_SIDE_TOP:
			tRect.fY = tBase.fY + fMargin;
			break;
		case XGE_XUI_DOCK_SIDE_BOTTOM:
			tRect.fY = tBase.fY + tBase.fH - fSize - fMargin;
			break;
		default:
			break;
	}
	return tRect;
}

static int __xgeXuiDockPaneIndicatorAsset(int iSide)
{
	switch ( iSide ) {
		case XGE_XUI_DOCK_SIDE_LEFT: return XGE_XUI_ASSET_DOCK_INDICATOR_PANE_DIAMOND_LEFT;
		case XGE_XUI_DOCK_SIDE_RIGHT: return XGE_XUI_ASSET_DOCK_INDICATOR_PANE_DIAMOND_RIGHT;
		case XGE_XUI_DOCK_SIDE_TOP: return XGE_XUI_ASSET_DOCK_INDICATOR_PANE_DIAMOND_TOP;
		case XGE_XUI_DOCK_SIDE_BOTTOM: return XGE_XUI_ASSET_DOCK_INDICATOR_PANE_DIAMOND_BOTTOM;
		case XGE_XUI_DOCK_SIDE_FILL: return XGE_XUI_ASSET_DOCK_INDICATOR_PANE_DIAMOND_FILL;
		default: return XGE_XUI_ASSET_DOCK_INDICATOR_PANE_DIAMOND;
	}
}

static int __xgeXuiDockPanelIndicatorAsset(int iSide)
{
	switch ( iSide ) {
		case XGE_XUI_DOCK_SIDE_LEFT: return XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_LEFT_ACTIVE;
		case XGE_XUI_DOCK_SIDE_RIGHT: return XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_RIGHT_ACTIVE;
		case XGE_XUI_DOCK_SIDE_TOP: return XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_TOP_ACTIVE;
		case XGE_XUI_DOCK_SIDE_BOTTOM: return XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_BOTTOM_ACTIVE;
		case XGE_XUI_DOCK_SIDE_FILL: return XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_FILL_ACTIVE;
		default: return XGE_XUI_ASSET_DOCK_INDICATOR_PANEL_FILL;
	}
}

static xge_xui_dock_window __xgeXuiDockPaneWindowAtPart(xge_xui_dock_pane pPane, int iPart)
{
	if ( (pPane != NULL) && (iPart == XGE_XUI_DOCK_PART_CAPTION) ) {
		iPart = pPane->iActive;
	}
	return (iPart >= 0) ? xgeXuiDockPaneGetWindow(pPane, iPart) : NULL;
}

static int __xgeXuiDockLayoutBeginPendingDragWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow, xge_xui_dock_pane pSourcePane, int iSourceIndex, const xge_event_t* pEvent)
{
	if ( (pLayout == NULL) || (pWindow == NULL) || (pEvent == NULL) ) {
		return 0;
	}
	if ( (pWindow == NULL) || (pWindow->bDockable == 0) ) {
		return 0;
	}
	__xgeXuiDockLayoutCloseOptionMenu(pLayout);
	__xgeXuiDockLayoutCloseOverflowMenu(pLayout);
	__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
	pLayout->iDragPhase = XGE_XUI_DOCK_DRAG_PENDING;
	pLayout->pDragWindow = pWindow;
	pLayout->pDragSourcePane = pSourcePane;
	pLayout->iDragSourceIndex = iSourceIndex;
	pLayout->tDragStartMouse = (xge_vec2_t){ pEvent->fX, pEvent->fY };
	pLayout->tDragLastMouse = pLayout->tDragStartMouse;
	pLayout->iHoverSide = XGE_XUI_DOCK_SIDE_NONE;
	pLayout->iHoverTabIndex = -1;
	pLayout->pHoverRegion = NULL;
	pLayout->pHoverPane = NULL;
	if ( (pWindow->iState == XGE_XUI_DOCK_WINDOW_FLOATING) && (pWindow->pWindowWidget != NULL) ) {
		pLayout->tPreviewRect = pWindow->pWindowWidget->tRect;
	}
	if ( pLayout->pContext != NULL ) {
		xgeXuiSetPointerCapture(pLayout->pContext, pEvent->iPointerId, pLayout->pWidget);
	}
	return 1;
}

static int __xgeXuiDockLayoutBeginPendingDrag(xge_xui_dock_layout pLayout, xge_xui_dock_pane pPane, int iPart, const xge_event_t* pEvent)
{
	xge_xui_dock_window pWindow;
	int iSourceIndex;

	if ( (pLayout == NULL) || (pPane == NULL) || (pEvent == NULL) ) {
		return 0;
	}
	iSourceIndex = (iPart == XGE_XUI_DOCK_PART_CAPTION) ? pPane->iActive : iPart;
	if ( iSourceIndex < 0 ) {
		return 0;
	}
	pWindow = __xgeXuiDockPaneWindowAtPart(pPane, iPart);
	return __xgeXuiDockLayoutBeginPendingDragWindow(pLayout, pWindow, pPane, iSourceIndex, pEvent);
}

static void __xgeXuiDockLayoutUpdateDragHover(xge_xui_dock_layout pLayout, float fX, float fY, int bDockingSuppressed)
{
	xge_xui_dock_pane pPane;
	xge_rect_t tBase;
	int iSide;
	int iRegion;

	if ( pLayout == NULL ) {
		return;
	}
	(void)__xgeXuiDockLayoutEnsureDragOverlay(pLayout);
	__xgeXuiDockLayoutSyncDragOverlayRect(pLayout);
	pLayout->tDragLastMouse = (xge_vec2_t){ fX, fY };
	pLayout->pHoverPane = NULL;
	pLayout->pHoverRegion = NULL;
	pLayout->iHoverSide = XGE_XUI_DOCK_SIDE_NONE;
	pLayout->iHoverTabIndex = -1;
	tBase = (pLayout->pWidget != NULL) ? pLayout->pWidget->tContentRect : __xgeXuiDockRectZero();
	if ( bDockingSuppressed != 0 ) {
		pLayout->tPreviewRect = __xgeXuiDockClampFloatRect(pLayout, (xge_rect_t){ fX - 160.0f, fY - 18.0f, 320.0f, 220.0f });
		pLayout->tIndicatorRect = __xgeXuiDockRectZero();
		__xgeXuiDockLayoutSetDragOverlayVisible(pLayout, 1);
		return;
	}
	if ( (pLayout->pDragSourcePane != NULL) && (pLayout->pDragSourcePane->arrWindows.Count > 1u) ) {
		int iTabIndex = __xgeXuiDockPaneTabInsertIndex(pLayout->pDragSourcePane, fX, fY);
		if ( iTabIndex >= 0 ) {
			pLayout->pHoverPane = pLayout->pDragSourcePane;
			pLayout->iHoverSide = XGE_XUI_DOCK_SIDE_FILL;
			pLayout->iHoverTabIndex = iTabIndex;
			pLayout->tPreviewRect = __xgeXuiDockPaneTabInsertRect(pLayout->pDragSourcePane, iTabIndex);
			pLayout->tIndicatorRect = __xgeXuiDockRectZero();
			__xgeXuiDockLayoutSetDragOverlayVisible(pLayout, 1);
			return;
		}
	}
	iSide = __xgeXuiDockGlobalSideFromRect(tBase, fX, fY);
	if ( iSide != XGE_XUI_DOCK_SIDE_NONE ) {
		iRegion = __xgeXuiDockRegionFromGlobalSide(iSide);
		pLayout->pHoverRegion = &pLayout->arrRegions[iRegion];
		pLayout->iHoverSide = iSide;
		pLayout->tPreviewRect = __xgeXuiDockPreviewRectForSide(tBase, iSide);
		pLayout->tIndicatorRect = __xgeXuiDockPanelIndicatorRect(tBase, iSide);
		__xgeXuiDockLayoutSetDragOverlayVisible(pLayout, 1);
		return;
	}
	pPane = __xgeXuiDockLayoutHitAnyPane(pLayout, fX, fY);
	if ( pPane != NULL ) {
		iSide = __xgeXuiDockDropSideFromRect(pPane->tRect, fX, fY);
		pLayout->pHoverPane = pPane;
		pLayout->iHoverSide = iSide;
		pLayout->tPreviewRect = __xgeXuiDockPreviewRectForSide(pPane->tRect, iSide);
		pLayout->tIndicatorRect = __xgeXuiDockIndicatorRect(pPane->tRect, 88.0f, 88.0f);
		__xgeXuiDockLayoutSetDragOverlayVisible(pLayout, 1);
		return;
	}
	if ( __xgeXuiDockRectContains(tBase, fX, fY) ) {
		iSide = XGE_XUI_DOCK_SIDE_FILL;
		pLayout->pHoverRegion = &pLayout->arrRegions[XGE_XUI_DOCK_REGION_DOCUMENT];
		pLayout->iHoverSide = iSide;
		pLayout->tPreviewRect = __xgeXuiDockPreviewRectForSide(tBase, iSide);
		pLayout->tIndicatorRect = __xgeXuiDockPanelIndicatorRect(tBase, iSide);
		__xgeXuiDockLayoutSetDragOverlayVisible(pLayout, 1);
		return;
	}
	pLayout->tPreviewRect = __xgeXuiDockClampFloatRect(pLayout, (xge_rect_t){ fX - 160.0f, fY - 18.0f, 320.0f, 220.0f });
	pLayout->tIndicatorRect = __xgeXuiDockRectZero();
	__xgeXuiDockLayoutSetDragOverlayVisible(pLayout, 1);
}

static void __xgeXuiDockLayoutCancelDrag(xge_xui_dock_layout pLayout)
{
	if ( pLayout == NULL ) {
		return;
	}
	pLayout->iDragPhase = XGE_XUI_DOCK_DRAG_IDLE;
	pLayout->pDragWindow = NULL;
	pLayout->pDragSourcePane = NULL;
	pLayout->pHoverRegion = NULL;
	pLayout->iHoverSide = XGE_XUI_DOCK_SIDE_NONE;
	pLayout->iHoverTabIndex = -1;
	pLayout->iDragSourceIndex = -1;
	pLayout->tPreviewRect = __xgeXuiDockRectZero();
	pLayout->tIndicatorRect = __xgeXuiDockRectZero();
	__xgeXuiDockLayoutSetDragOverlayVisible(pLayout, 0);
	if ( pLayout->pWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pLayout->pWidget);
	}
}

static int __xgeXuiDockLayoutCommitDrag(xge_xui_dock_layout pLayout)
{
	xge_xui_dock_window pWindow;
	xge_xui_dock_pane pPane;
	int iRegion;
	int iSide;
	int iRet;

	if ( pLayout == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pWindow = pLayout->pDragWindow;
	iSide = pLayout->iHoverSide;
	if ( pWindow == NULL ) {
		__xgeXuiDockLayoutCancelDrag(pLayout);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pWindow->bDockable == 0 ) {
		__xgeXuiDockLayoutCancelDrag(pLayout);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pLayout->pHoverPane != NULL) && (pLayout->pHoverPane == pLayout->pDragSourcePane) && (pLayout->iHoverTabIndex >= 0) ) {
		iRet = __xgeXuiDockPaneMoveWindow(pLayout->pHoverPane, pLayout->iDragSourceIndex, pLayout->iHoverTabIndex);
	} else if ( iSide == XGE_XUI_DOCK_SIDE_NONE ) {
		iRet = xgeXuiDockLayoutFloatWindow(pLayout, pWindow, pLayout->tPreviewRect);
	} else if ( pLayout->pHoverPane != NULL ) {
		if ( iSide == XGE_XUI_DOCK_SIDE_FILL ) {
			pPane = pLayout->pHoverPane;
		} else {
			pPane = __xgeXuiDockLayoutSplitPane(pLayout, pLayout->pHoverPane, iSide, pLayout->fSidePortionDefault);
		}
		iRet = (pPane != NULL) ? __xgeXuiDockLayoutDockWindowToPane(pLayout, pWindow, pPane, iSide) : XGE_ERROR;
	} else if ( pLayout->pHoverRegion != NULL ) {
		iRegion = pLayout->pHoverRegion->iKind;
		if ( iRegion != XGE_XUI_DOCK_REGION_DOCUMENT ) {
			pLayout->arrRegions[iRegion].fPortion = __xgeXuiDockClampPortion(pLayout->fSidePortionDefault);
			pPane = __xgeXuiDockLayoutDropPane(pLayout, iRegion, XGE_XUI_DOCK_SIDE_FILL, pLayout->fSidePortionDefault);
		} else {
			pPane = __xgeXuiDockLayoutDropPane(pLayout, iRegion, XGE_XUI_DOCK_SIDE_FILL, pLayout->fSidePortionDefault);
		}
		iRet = (pPane != NULL) ? __xgeXuiDockLayoutDockWindowToPane(pLayout, pWindow, pPane, iSide) : XGE_ERROR;
	} else {
		iRet = (xgeXuiDockLayoutDockWindow(pLayout, pWindow, XGE_XUI_DOCK_REGION_DOCUMENT, iSide, pLayout->fSidePortionDefault) != NULL) ? XGE_OK : XGE_ERROR;
	}
	__xgeXuiDockLayoutCancelDrag(pLayout);
	return iRet;
}

static int __xgeXuiDockLayoutUpdateDragEvent(xge_xui_dock_layout pLayout, const xge_event_t* pEvent)
{
	float fDX;
	float fDY;

	if ( (pLayout == NULL) || (pEvent == NULL) ) {
		return 0;
	}
	if ( pLayout->iDragPhase == XGE_XUI_DOCK_DRAG_PENDING ) {
		fDX = pEvent->fX - pLayout->tDragStartMouse.fX;
		fDY = pEvent->fY - pLayout->tDragStartMouse.fY;
		if ( (fDX * fDX + fDY * fDY) <= 36.0f ) {
			return 0;
		}
		pLayout->iDragPhase = XGE_XUI_DOCK_DRAG_DRAGGING;
		__xgeXuiDockLayoutClearActivePart(pLayout);
		__xgeXuiDockLayoutClearHover(pLayout);
	}
	if ( pLayout->iDragPhase == XGE_XUI_DOCK_DRAG_DRAGGING ) {
		__xgeXuiDockLayoutUpdateDragHover(pLayout, pEvent->fX, pEvent->fY, (pEvent->iParam2 & XGE_KEY_MOD_CTRL) != 0);
		return 1;
	}
	return 0;
}

static void __xgeXuiDockLayoutOverlayPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_dock_layout pLayout;
	xge_rect_t tIndicator;
	int iAsset;

	(void)pWidget;
	pLayout = (xge_xui_dock_layout)pUser;
	if ( (pLayout == NULL) || (pLayout->iDragPhase != XGE_XUI_DOCK_DRAG_DRAGGING) ) {
		return;
	}
	if ( (pLayout->tPreviewRect.fW > 0.0f) && (pLayout->tPreviewRect.fH > 0.0f) ) {
		__xgeXuiHostDrawRect(pLayout->tPreviewRect, XGE_COLOR_RGBA(83, 158, 219, 64));
		__xgeXuiHostDrawBorderRect(pLayout->tPreviewRect, 2.0f, XGE_COLOR_RGBA(19, 122, 204, 210));
	}
	if ( (pLayout->tIndicatorRect.fW <= 0.0f) || (pLayout->tIndicatorRect.fH <= 0.0f) ) {
		return;
	}
	if ( pLayout->pHoverPane != NULL ) {
		iAsset = __xgeXuiDockPaneIndicatorAsset(pLayout->iHoverSide);
		if ( __xgeXuiBuiltinAssetDraw(pLayout->tIndicatorRect, iAsset, XGE_COLOR_RGBA(255, 255, 255, 255)) == 0 ) {
			__xgeXuiDockDrawPaneIndicatorFallback(pLayout->tIndicatorRect, pLayout->iHoverSide);
		}
	} else {
		iAsset = __xgeXuiDockPanelIndicatorAsset(pLayout->iHoverSide);
		tIndicator = pLayout->tIndicatorRect;
		if ( __xgeXuiBuiltinAssetDraw(tIndicator, iAsset, XGE_COLOR_RGBA(255, 255, 255, 255)) == 0 ) {
			__xgeXuiDockDrawPanelIndicatorFallback(tIndicator, pLayout->iHoverSide);
		}
	}
}

static int __xgeXuiDockLayoutTooltipResolve(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_tooltip_desc pDesc, void* pUser)
{
	xge_xui_dock_layout pLayout;
	xge_xui_dock_pane pPane;
	xge_xui_dock_window pWindow;
	const char* sText;
	int iPart;

	(void)pWidget;
	pLayout = (xge_xui_dock_layout)pUser;
	if ( (pContext == NULL) || (pLayout == NULL) || (pDesc == NULL) || (pLayout->iDragPhase != XGE_XUI_DOCK_DRAG_IDLE) || (pLayout->pSplitterDragNode != NULL) ) {
		return 0;
	}
	iPart = XGE_XUI_DOCK_PART_NONE;
	pPane = __xgeXuiDockLayoutHitPane(pLayout, pContext->fTooltipMouseX, pContext->fTooltipMouseY, &iPart);
	if ( pPane == NULL ) {
		return 0;
	}
	sText = NULL;
	if ( iPart == XGE_XUI_DOCK_PART_CLOSE ) {
		sText = "Close";
	} else if ( iPart == XGE_XUI_DOCK_PART_AUTO_HIDE ) {
		sText = "Auto Hide";
	} else if ( iPart == XGE_XUI_DOCK_PART_OPTION ) {
		sText = "Options";
	} else if ( iPart == XGE_XUI_DOCK_PART_OVERFLOW ) {
		sText = "Tab List";
	} else if ( iPart == XGE_XUI_DOCK_PART_CAPTION ) {
		pWindow = xgeXuiDockPaneGetActiveWindow(pPane);
		sText = (pWindow != NULL) ? pWindow->sTitle : NULL;
	} else if ( iPart >= 0 ) {
		pWindow = xgeXuiDockPaneGetWindow(pPane, iPart);
		sText = (pWindow != NULL) ? pWindow->sTitle : NULL;
	}
	if ( (sText == NULL) || (sText[0] == 0) ) {
		return 0;
	}
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iType = XGE_XUI_TOOLTIP_TEXT;
	pDesc->sText = sText;
	pDesc->iAnchor = XGE_XUI_TOOLTIP_ANCHOR_CURSOR;
	pDesc->fDelay = 0.35f;
	pDesc->bFollowCursor = 1;
	return 1;
}

static int __xgeXuiDockPaneClosableCount(xge_xui_dock_pane pPane, xge_xui_dock_window pExcept)
{
	xge_xui_dock_window pWindow;
	int iCount;
	uint32 i;

	iCount = 0;
	if ( pPane == NULL ) {
		return 0;
	}
	for ( i = 0; i < pPane->arrWindows.Count; i++ ) {
		pWindow = xgeXuiDockPaneGetWindow(pPane, (int)i);
		if ( (pWindow != NULL) && (pWindow != pExcept) && (pWindow->bClosable != 0) ) {
			iCount++;
		}
	}
	return iCount;
}

static xge_rect_t __xgeXuiDockWindowDefaultFloatRect(xge_xui_dock_layout pLayout, xge_xui_dock_pane pPane, xge_xui_dock_window pWindow)
{
	xge_rect_t tRect;

	if ( (pWindow != NULL) && (pWindow->tLastFloatRect.fW > 0.0f) && (pWindow->tLastFloatRect.fH > 0.0f) ) {
		return pWindow->tLastFloatRect;
	}
	tRect = (pPane != NULL) ? pPane->tRect : __xgeXuiDockRectZero();
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		tRect = (pLayout != NULL && pLayout->pWidget != NULL) ? pLayout->pWidget->tContentRect : (xge_rect_t){ 32.0f, 32.0f, 360.0f, 240.0f };
	}
	tRect.fX += 24.0f;
	tRect.fY += 24.0f;
	if ( tRect.fW < 280.0f ) {
		tRect.fW = 280.0f;
	}
	if ( tRect.fH < 180.0f ) {
		tRect.fH = 180.0f;
	}
	if ( tRect.fW > 520.0f ) {
		tRect.fW = 520.0f;
	}
	if ( tRect.fH > 360.0f ) {
		tRect.fH = 360.0f;
	}
	return tRect;
}

static int __xgeXuiDockWindowAutoHideStripRegion(xge_xui_dock_window pWindow)
{
	xge_xui_dock_region pRegion;

	if ( (pWindow == NULL) || (pWindow->pPane == NULL) || (pWindow->pPane->pNode == NULL) || (pWindow->pLayout == NULL) ) {
		return -1;
	}
	pRegion = __xgeXuiDockLayoutFindNodeRegion(pWindow->pLayout, pWindow->pPane->pNode);
	if ( pRegion == NULL ) {
		return -1;
	}
	if ( pRegion->iKind != XGE_XUI_DOCK_REGION_DOCUMENT ) {
		return pRegion->iKind;
	}
	if ( __xgeXuiDockSideIsSplit(pWindow->iLastSide) ) {
		return __xgeXuiDockSideToRegion(pWindow->iLastSide);
	}
	return -1;
}

static int __xgeXuiDockPaneCanAutoHide(xge_xui_dock_pane pPane)
{
	xge_xui_dock_window pWindow;

	pWindow = xgeXuiDockPaneGetActiveWindow(pPane);
	return (pWindow != NULL) && (pWindow->iState == XGE_XUI_DOCK_WINDOW_DOCKED) && (pWindow->bDockable != 0) && (__xgeXuiDockWindowAutoHideStripRegion(pWindow) >= 0);
}

static void __xgeXuiDockLayoutCloseOptionMenu(xge_xui_dock_layout pLayout)
{
	if ( (pLayout == NULL) || (pLayout->bOptionMenuInit == 0) ) {
		return;
	}
	xgeXuiMenuClose(&pLayout->tOptionMenu);
	pLayout->pOptionMenuPane = NULL;
}

static void __xgeXuiDockLayoutCloseOverflowMenu(xge_xui_dock_layout pLayout)
{
	if ( (pLayout == NULL) || (pLayout->bOverflowMenuInit == 0) ) {
		return;
	}
	xgeXuiMenuClose(&pLayout->tOverflowMenu);
	pLayout->pOverflowMenuPane = NULL;
}

static void __xgeXuiDockLayoutOpenOptionMenu(xge_xui_dock_layout pLayout, xge_xui_dock_pane pPane)
{
	xge_xui_menu_item_t arrItems[6];
	xge_xui_dock_window pActive;
	int iCount;
	int iClosableOthers;
	int iActiveClosable;
	int iActiveDockable;

	if ( (pLayout == NULL) || (pPane == NULL) || (pLayout->bOptionMenuInit == 0) ) {
		return;
	}
	__xgeXuiDockLayoutCloseOverflowMenu(pLayout);
	pActive = xgeXuiDockPaneGetActiveWindow(pPane);
	iActiveClosable = (pActive != NULL) && (pActive->bClosable != 0);
	iActiveDockable = (pActive != NULL) && (pActive->bDockable != 0);
	iClosableOthers = __xgeXuiDockPaneClosableCount(pPane, pActive);
	memset(arrItems, 0, sizeof(arrItems));
	iCount = 0;
	arrItems[iCount++] = (xge_xui_menu_item_t){ "Float", NULL, XGE_XUI_MENU_ITEM_NORMAL, iActiveDockable ? XGE_XUI_MENU_ITEM_ENABLED : 0, XGE_XUI_DOCK_OPTION_FLOAT, 0, NULL, NULL };
	arrItems[iCount++] = (xge_xui_menu_item_t){ "Dock", NULL, XGE_XUI_MENU_ITEM_NORMAL, 0, XGE_XUI_DOCK_OPTION_DOCK, 0, NULL, NULL };
	arrItems[iCount++] = (xge_xui_menu_item_t){ NULL, NULL, XGE_XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL };
	arrItems[iCount++] = (xge_xui_menu_item_t){ "Close", NULL, XGE_XUI_MENU_ITEM_NORMAL, iActiveClosable ? (XGE_XUI_MENU_ITEM_ENABLED | XGE_XUI_MENU_ITEM_DANGER) : 0, XGE_XUI_DOCK_OPTION_CLOSE, 0, NULL, NULL };
	arrItems[iCount++] = (xge_xui_menu_item_t){ "Close Others", NULL, XGE_XUI_MENU_ITEM_NORMAL, (iClosableOthers > 0) ? XGE_XUI_MENU_ITEM_ENABLED : 0, XGE_XUI_DOCK_OPTION_CLOSE_OTHERS, 0, NULL, NULL };
	arrItems[iCount++] = (xge_xui_menu_item_t){ "Close All", NULL, XGE_XUI_MENU_ITEM_NORMAL, ((iActiveClosable || iClosableOthers > 0) ? (XGE_XUI_MENU_ITEM_ENABLED | XGE_XUI_MENU_ITEM_DANGER) : 0), XGE_XUI_DOCK_OPTION_CLOSE_ALL, 0, NULL, NULL };
	xgeXuiMenuSetItems(&pLayout->tOptionMenu, arrItems, iCount);
	xgeXuiMenuSetSelect(&pLayout->tOptionMenu, __xgeXuiDockLayoutOptionMenuSelect, pLayout);
	pLayout->pOptionMenuPane = pPane;
	xgeXuiMenuOpenAt(&pLayout->tOptionMenu, pLayout->pWidget, pPane->tOptionRect.fX, pPane->tOptionRect.fY + pPane->tOptionRect.fH);
}

static void __xgeXuiDockLayoutClosePaneWindows(xge_xui_dock_layout pLayout, xge_xui_dock_pane pPane, xge_xui_dock_window pExcept)
{
	xge_xui_dock_window pWindow;
	int i;

	if ( (pLayout == NULL) || (pPane == NULL) ) {
		return;
	}
	for ( i = (int)pPane->arrWindows.Count - 1; i >= 0; i-- ) {
		pWindow = xgeXuiDockPaneGetWindow(pPane, i);
		if ( (pWindow != NULL) && (pWindow != pExcept) && (pWindow->bClosable != 0) ) {
			(void)xgeXuiDockLayoutHideWindow(pLayout, pWindow);
		}
	}
}

static void __xgeXuiDockLayoutOptionMenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xge_xui_dock_layout pLayout;
	xge_xui_dock_pane pPane;
	xge_xui_dock_window pActive;

	(void)pWidget;
	(void)iIndex;
	pLayout = (xge_xui_dock_layout)pUser;
	if ( pLayout == NULL ) {
		return;
	}
	pPane = pLayout->pOptionMenuPane;
	pActive = xgeXuiDockPaneGetActiveWindow(pPane);
	pLayout->pOptionMenuPane = NULL;
	if ( pPane == NULL ) {
		return;
	}
	switch ( iValue ) {
		case XGE_XUI_DOCK_OPTION_FLOAT:
			if ( (pActive != NULL) && (pActive->bDockable != 0) ) {
				(void)xgeXuiDockLayoutFloatWindow(pLayout, pActive, __xgeXuiDockWindowDefaultFloatRect(pLayout, pPane, pActive));
			}
			break;
		case XGE_XUI_DOCK_OPTION_CLOSE:
			if ( (pActive != NULL) && (pActive->bClosable != 0) ) {
				(void)xgeXuiDockLayoutHideWindow(pLayout, pActive);
			}
			break;
		case XGE_XUI_DOCK_OPTION_CLOSE_OTHERS:
			__xgeXuiDockLayoutClosePaneWindows(pLayout, pPane, pActive);
			break;
		case XGE_XUI_DOCK_OPTION_CLOSE_ALL:
			__xgeXuiDockLayoutClosePaneWindows(pLayout, pPane, NULL);
			break;
		default:
			break;
	}
	if ( pLayout->pWidget != NULL ) {
		xgeXuiWidgetMarkLayout(pLayout->pWidget);
		xgeXuiWidgetMarkPaint(pLayout->pWidget);
	}
}

static void __xgeXuiDockLayoutOpenOverflowMenu(xge_xui_dock_layout pLayout, xge_xui_dock_pane pPane)
{
	xge_xui_menu_item_t arrItems[XGE_XUI_MENU_ITEM_CAPACITY];
	xge_xui_dock_window pWindow;
	int iCount;
	int iLimit;
	int iState;
	int i;

	if ( (pLayout == NULL) || (pPane == NULL) || (pLayout->bOverflowMenuInit == 0) || (pPane->tOverflowRect.fW <= 0.0f) ) {
		return;
	}
	__xgeXuiDockLayoutCloseOptionMenu(pLayout);
	memset(arrItems, 0, sizeof(arrItems));
	iLimit = (int)pPane->arrWindows.Count;
	if ( iLimit > XGE_XUI_MENU_ITEM_CAPACITY ) {
		iLimit = XGE_XUI_MENU_ITEM_CAPACITY;
	}
	iCount = 0;
	for ( i = 0; i < iLimit; i++ ) {
		pWindow = xgeXuiDockPaneGetWindow(pPane, i);
		if ( pWindow == NULL ) {
			continue;
		}
		iState = XGE_XUI_MENU_ITEM_ENABLED;
		if ( i == pPane->iActive ) {
			iState |= XGE_XUI_MENU_ITEM_CHECKED;
		}
		arrItems[iCount++] = (xge_xui_menu_item_t){ pWindow->sTitle != NULL ? pWindow->sTitle : "", NULL, XGE_XUI_MENU_ITEM_RADIO, iState, i, 0, NULL, NULL };
	}
	if ( iCount <= 0 ) {
		return;
	}
	xgeXuiMenuSetItems(&pLayout->tOverflowMenu, arrItems, iCount);
	xgeXuiMenuSetSelect(&pLayout->tOverflowMenu, __xgeXuiDockLayoutOverflowMenuSelect, pLayout);
	pLayout->pOverflowMenuPane = pPane;
	xgeXuiMenuOpenAt(&pLayout->tOverflowMenu, pLayout->pWidget, pPane->tOverflowRect.fX, pPane->tOverflowRect.fY + pPane->tOverflowRect.fH);
}

static void __xgeXuiDockLayoutOverflowMenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xge_xui_dock_layout pLayout;
	xge_xui_dock_pane pPane;

	(void)pWidget;
	(void)iIndex;
	pLayout = (xge_xui_dock_layout)pUser;
	if ( pLayout == NULL ) {
		return;
	}
	pPane = pLayout->pOverflowMenuPane;
	pLayout->pOverflowMenuPane = NULL;
	if ( (pPane == NULL) || (iValue < 0) || (iValue >= (int)pPane->arrWindows.Count) ) {
		return;
	}
	xgeXuiDockPaneSetActiveIndex(pPane, iValue);
	if ( pLayout->pWidget != NULL ) {
		xgeXuiWidgetMarkLayout(pLayout->pWidget);
		xgeXuiWidgetMarkPaint(pLayout->pWidget);
	}
}

static int __xgeXuiDockLayoutEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	xge_xui_dock_layout pLayout;
	xge_xui_dock_pane pPane;
	xge_xui_dock_pane pPressedPane;
	xge_xui_dock_node pSplitter;
	int iPart;
	int iHitPart;
	int iActiveButton;
	int bLeftPress;
	int bMiddlePress;

	(void)pWidget;
	pLayout = (xge_xui_dock_layout)pUser;
	if ( (pLayout == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( __xgeXuiDockLayoutUpdateSplitterDrag(pLayout, pEvent) ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( __xgeXuiDockLayoutUpdateDragEvent(pLayout, pEvent) ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pLayout->iDragPhase == XGE_XUI_DOCK_DRAG_PENDING ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			{
				xge_xui_dock_window pAutoHide = __xgeXuiDockLayoutHitAutoHideWindow(pLayout, pEvent->fX, pEvent->fY);
				if ( pAutoHide != pLayout->pAutoHideHoverWindow ) {
					pLayout->pAutoHideHoverWindow = pAutoHide;
					xgeXuiWidgetMarkPaint(pLayout->pWidget);
				}
			}
			iPart = XGE_XUI_DOCK_PART_NONE;
			pPane = __xgeXuiDockLayoutHitPane(pLayout, pEvent->fX, pEvent->fY, &iPart);
			if ( (pPane != pLayout->pHoverPane) || (pPane != NULL && pPane->iHoverPart != iPart) ) {
				__xgeXuiDockLayoutClearHover(pLayout);
				if ( pPane != NULL ) {
					pPane->iHoverPart = iPart;
				}
				pLayout->pHoverPane = pPane;
				xgeXuiWidgetMarkPaint(pLayout->pWidget);
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			bLeftPress = (pEvent->iType == XGE_EVENT_TOUCH_BEGIN) || ((pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 == XGE_MOUSE_LEFT));
			bMiddlePress = (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 == XGE_MOUSE_MIDDLE);
			if ( !bLeftPress && !bMiddlePress ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( bLeftPress ) {
				xge_xui_dock_window pAutoHide = __xgeXuiDockLayoutHitAutoHideWindow(pLayout, pEvent->fX, pEvent->fY);
				if ( pAutoHide != NULL ) {
					__xgeXuiDockLayoutClearActivePart(pLayout);
					pLayout->pAutoHideActiveWindow = pAutoHide;
					pLayout->pAutoHideHoverWindow = pAutoHide;
					if ( pLayout->pContext != NULL ) {
						xgeXuiSetPointerCapture(pLayout->pContext, pEvent->iPointerId, pLayout->pWidget);
					}
					xgeXuiWidgetMarkPaint(pLayout->pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
				pSplitter = __xgeXuiDockLayoutHitSplitter(pLayout, pEvent->fX, pEvent->fY);
				if ( __xgeXuiDockLayoutBeginSplitterDrag(pLayout, pSplitter, pEvent) ) {
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			iPart = XGE_XUI_DOCK_PART_NONE;
			pPane = __xgeXuiDockLayoutHitPane(pLayout, pEvent->fX, pEvent->fY, &iPart);
			if ( (pPane == NULL) || (iPart == XGE_XUI_DOCK_PART_NONE) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( bMiddlePress && iPart < 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiDockLayoutClearActivePart(pLayout);
			pPane->iActivePart = iPart;
			pPane->iHoverPart = iPart;
			pLayout->pHoverPane = pPane;
			pLayout->iActiveButton = bMiddlePress ? XGE_MOUSE_MIDDLE : XGE_MOUSE_LEFT;
			if ( bLeftPress && ((iPart >= 0) || (iPart == XGE_XUI_DOCK_PART_CAPTION)) ) {
				(void)__xgeXuiDockLayoutBeginPendingDrag(pLayout, pPane, iPart, pEvent);
			}
			if ( pLayout->pContext != NULL ) {
				xgeXuiSetPointerCapture(pLayout->pContext, pEvent->iPointerId, pLayout->pWidget);
			}
			xgeXuiWidgetMarkPaint(pLayout->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			if ( pLayout->pAutoHideActiveWindow != NULL ) {
				xge_xui_dock_window pActiveAutoHide = pLayout->pAutoHideActiveWindow;
				xge_xui_dock_window pHitAutoHide = __xgeXuiDockLayoutHitAutoHideWindow(pLayout, pEvent->fX, pEvent->fY);
				if ( pLayout->pContext != NULL && xgeXuiGetPointerCapture(pLayout->pContext, pEvent->iPointerId) == pLayout->pWidget ) {
					xgeXuiSetPointerCapture(pLayout->pContext, pEvent->iPointerId, NULL);
				}
				pLayout->pAutoHideActiveWindow = NULL;
				if ( pHitAutoHide == pActiveAutoHide ) {
					(void)__xgeXuiDockLayoutOpenAutoHideExpand(pLayout, pActiveAutoHide);
				}
				pLayout->pAutoHideHoverWindow = NULL;
				xgeXuiWidgetMarkLayout(pLayout->pWidget);
				xgeXuiWidgetMarkPaint(pLayout->pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pLayout->pSplitterDragNode != NULL ) {
				if ( pLayout->pContext != NULL && xgeXuiGetPointerCapture(pLayout->pContext, pEvent->iPointerId) == pLayout->pWidget ) {
					xgeXuiSetPointerCapture(pLayout->pContext, pEvent->iPointerId, NULL);
				}
				__xgeXuiDockLayoutEndSplitterDrag(pLayout);
				xgeXuiWidgetMarkLayout(pLayout->pWidget);
				xgeXuiWidgetMarkPaint(pLayout->pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pLayout->iDragPhase == XGE_XUI_DOCK_DRAG_DRAGGING ) {
				if ( pLayout->pContext != NULL && xgeXuiGetPointerCapture(pLayout->pContext, pEvent->iPointerId) == pLayout->pWidget ) {
					xgeXuiSetPointerCapture(pLayout->pContext, pEvent->iPointerId, NULL);
				}
				(void)__xgeXuiDockLayoutCommitDrag(pLayout);
				xgeXuiWidgetMarkLayout(pLayout->pWidget);
				xgeXuiWidgetMarkPaint(pLayout->pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			pPressedPane = __xgeXuiDockLayoutFindActivePartPane(pLayout);
			if ( pPressedPane == NULL ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iPart = pPressedPane->iActivePart;
			iActiveButton = pLayout->iActiveButton;
			iHitPart = XGE_XUI_DOCK_PART_NONE;
			pPane = __xgeXuiDockLayoutHitPane(pLayout, pEvent->fX, pEvent->fY, &iHitPart);
			__xgeXuiDockLayoutClearActivePart(pLayout);
			if ( pLayout->pContext != NULL && xgeXuiGetPointerCapture(pLayout->pContext, pEvent->iPointerId) == pLayout->pWidget ) {
				xgeXuiSetPointerCapture(pLayout->pContext, pEvent->iPointerId, NULL);
			}
			if ( pLayout->iDragPhase == XGE_XUI_DOCK_DRAG_PENDING ) {
				__xgeXuiDockLayoutCancelDrag(pLayout);
			}
			if ( (pPane == pPressedPane) && (iPart == iHitPart) ) {
				if ( iPart >= 0 ) {
					if ( (iActiveButton == XGE_MOUSE_MIDDLE) && (pEvent->iType == XGE_EVENT_MOUSE_UP) && (pEvent->iParam1 == XGE_MOUSE_MIDDLE) ) {
						xge_xui_dock_window pWindow = xgeXuiDockPaneGetWindow(pPressedPane, iPart);
						if ( (pWindow != NULL) && (pWindow->bClosable != 0) ) {
							xgeXuiDockLayoutHideWindow(pLayout, pWindow);
						}
					} else if ( iActiveButton == XGE_MOUSE_LEFT ) {
						xgeXuiDockPaneSetActiveIndex(pPressedPane, iPart);
					}
				} else if ( iPart == XGE_XUI_DOCK_PART_CLOSE ) {
					xge_xui_dock_window pWindow = xgeXuiDockPaneGetActiveWindow(pPressedPane);
					if ( (pWindow != NULL) && (pWindow->bClosable != 0) ) {
						xgeXuiDockLayoutHideWindow(pLayout, pWindow);
					}
				} else if ( iPart == XGE_XUI_DOCK_PART_AUTO_HIDE ) {
					xge_xui_dock_window pWindow = xgeXuiDockPaneGetActiveWindow(pPressedPane);
					if ( (pWindow != NULL) && (__xgeXuiDockPaneCanAutoHide(pPressedPane) != 0) ) {
						(void)xgeXuiDockLayoutAutoHideWindow(pLayout, pWindow);
					}
				} else if ( iPart == XGE_XUI_DOCK_PART_OPTION ) {
					__xgeXuiDockLayoutOpenOptionMenu(pLayout, pPressedPane);
				} else if ( iPart == XGE_XUI_DOCK_PART_OVERFLOW ) {
					__xgeXuiDockLayoutOpenOverflowMenu(pLayout, pPressedPane);
				}
				xgeXuiWidgetMarkLayout(pLayout->pWidget);
				xgeXuiWidgetMarkPaint(pLayout->pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			xgeXuiWidgetMarkPaint(pLayout->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			__xgeXuiDockLayoutEndSplitterDrag(pLayout);
			__xgeXuiDockLayoutCancelDrag(pLayout);
			__xgeXuiDockLayoutClearActivePart(pLayout);
			pLayout->pAutoHideActiveWindow = NULL;
			if ( pLayout->pContext != NULL && xgeXuiGetPointerCapture(pLayout->pContext, pEvent->iPointerId) == pLayout->pWidget ) {
				xgeXuiSetPointerCapture(pLayout->pContext, pEvent->iPointerId, NULL);
			}
			xgeXuiWidgetMarkPaint(pLayout->pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( (pLayout->iDragPhase != XGE_XUI_DOCK_DRAG_IDLE) || (pLayout->pSplitterDragNode != NULL) ) {
				if ( pEvent->iParam1 == XGE_KEY_ESCAPE ) {
					__xgeXuiDockLayoutEndSplitterDrag(pLayout);
					__xgeXuiDockLayoutCancelDrag(pLayout);
					__xgeXuiDockLayoutClearActivePart(pLayout);
					pLayout->pAutoHideActiveWindow = NULL;
					if ( pLayout->pContext != NULL ) {
						xgeXuiReleaseWidgetCapture(pLayout->pContext, pLayout->pWidget);
					}
					xgeXuiWidgetMarkLayout(pLayout->pWidget);
					xgeXuiWidgetMarkPaint(pLayout->pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( pEvent->iParam1 == XGE_KEY_TAB ) {
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			return XGE_XUI_EVENT_CONTINUE;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

static int __xgeXuiDockWindowButtonHit(xge_xui_widget pButton, float fX, float fY)
{
	return (pButton != NULL) && xgeXuiWidgetIsVisible(pButton) && __xgeXuiDockRectContains(pButton->tRect, fX, fY);
}

static int __xgeXuiDockWindowResizeGripHit(xge_xui_dock_window pWindow, float fX, float fY)
{
	xge_xui_window pBase;
	xge_rect_t tRect;
	float fGrip;

	if ( (pWindow == NULL) || (pWindow->pWindowWidget == NULL) ) {
		return 0;
	}
	pBase = &pWindow->tWindow;
	if ( (pBase->bResizable == 0) || (pBase->bMaximized != 0) ) {
		return 0;
	}
	tRect = pWindow->pWindowWidget->tRect;
	if ( !__xgeXuiDockRectContains(tRect, fX, fY) ) {
		return 0;
	}
	fGrip = pBase->fResizeGrip;
	if ( ((pBase->iResizeEdges & XGE_XUI_WINDOW_EDGE_LEFT) != 0) && (fX <= tRect.fX + fGrip) ) {
		return 1;
	}
	if ( ((pBase->iResizeEdges & XGE_XUI_WINDOW_EDGE_RIGHT) != 0) && (fX >= tRect.fX + tRect.fW - fGrip) ) {
		return 1;
	}
	if ( ((pBase->iResizeEdges & XGE_XUI_WINDOW_EDGE_TOP) != 0) && (fY <= tRect.fY + 3.0f) ) {
		return 1;
	}
	if ( ((pBase->iResizeEdges & XGE_XUI_WINDOW_EDGE_BOTTOM) != 0) && (fY >= tRect.fY + tRect.fH - fGrip) ) {
		return 1;
	}
	return 0;
}

static int __xgeXuiDockWindowCanStartFloatingDockDrag(xge_xui_dock_window pWindow, const xge_event_t* pEvent)
{
	xge_xui_window pBase;
	xge_rect_t tTitleRect;

	if ( (pWindow == NULL) || (pEvent == NULL) || (pWindow->pLayout == NULL) || (pWindow->pWindowWidget == NULL) ) {
		return 0;
	}
	if ( (pWindow->iState != XGE_XUI_DOCK_WINDOW_FLOATING) || (pWindow->bDockable == 0) ) {
		return 0;
	}
	if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 != XGE_MOUSE_LEFT) ) {
		return 0;
	}
	pBase = &pWindow->tWindow;
	if ( (pBase->bShowTitleBar == 0) || (pBase->bMovable == 0) || (pBase->bMaximized != 0) ) {
		return 0;
	}
	if ( __xgeXuiDockWindowButtonHit(pBase->pCollapseButtonWidget, pEvent->fX, pEvent->fY) ||
		__xgeXuiDockWindowButtonHit(pBase->pMaximizeButtonWidget, pEvent->fX, pEvent->fY) ||
		__xgeXuiDockWindowButtonHit(pBase->pCloseButtonWidget, pEvent->fX, pEvent->fY) ) {
		return 0;
	}
	if ( __xgeXuiDockWindowResizeGripHit(pWindow, pEvent->fX, pEvent->fY) ) {
		return 0;
	}
	tTitleRect = pWindow->pWindowWidget->tRect;
	tTitleRect.fH = pBase->fTitleBarHeight;
	return __xgeXuiDockRectContains(tTitleRect, pEvent->fX, pEvent->fY);
}

static int __xgeXuiDockWindowEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	xge_xui_dock_window pWindow;
	xge_xui_dock_layout pLayout;

	(void)pWidget;
	pWindow = (xge_xui_dock_window)pUser;
	if ( (pWindow == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) || (pEvent->iType == XGE_EVENT_TOUCH_BEGIN) ) {
		pLayout = pWindow->pLayout;
		if ( (pLayout != NULL) && (pLayout->iDragPhase == XGE_XUI_DOCK_DRAG_IDLE) && __xgeXuiDockWindowCanStartFloatingDockDrag(pWindow, pEvent) ) {
			xgeXuiWindowBringToFront(&pWindow->tWindow);
			xgeXuiSetFocus(pWindow->pContext, pWindow->pWindowWidget);
			if ( __xgeXuiDockLayoutBeginPendingDragWindow(pLayout, pWindow, NULL, -1, pEvent) ) {
				xgeXuiWidgetMarkPaint(pLayout->pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
		}
	}
	return xgeXuiWindowEvent(&pWindow->tWindow, pEvent);
}

static const char* __xgeXuiDockRegionStateName(int iRegion)
{
	switch ( iRegion ) {
		case XGE_XUI_DOCK_REGION_DOCUMENT: return "document";
		case XGE_XUI_DOCK_REGION_LEFT: return "left";
		case XGE_XUI_DOCK_REGION_RIGHT: return "right";
		case XGE_XUI_DOCK_REGION_TOP: return "top";
		case XGE_XUI_DOCK_REGION_BOTTOM: return "bottom";
		default: return "unknown";
	}
}

static const char* __xgeXuiDockSideStateName(int iSide)
{
	switch ( iSide ) {
		case XGE_XUI_DOCK_SIDE_NONE: return "none";
		case XGE_XUI_DOCK_SIDE_LEFT: return "left";
		case XGE_XUI_DOCK_SIDE_RIGHT: return "right";
		case XGE_XUI_DOCK_SIDE_TOP: return "top";
		case XGE_XUI_DOCK_SIDE_BOTTOM: return "bottom";
		case XGE_XUI_DOCK_SIDE_FILL: return "fill";
		default: return "unknown";
	}
}

static const char* __xgeXuiDockWindowStateName(int iState)
{
	switch ( iState ) {
		case XGE_XUI_DOCK_WINDOW_FLOATING: return "floating";
		case XGE_XUI_DOCK_WINDOW_DOCKED: return "docked";
		case XGE_XUI_DOCK_WINDOW_HIDDEN: return "hidden";
		case XGE_XUI_DOCK_WINDOW_AUTO_HIDE: return "autoHide";
		default: return "unknown";
	}
}

static const char* __xgeXuiDockWindowSaveId(const xge_xui_dock_window pWindow)
{
	const char* sName;

	if ( pWindow == NULL ) {
		return "";
	}
	sName = (pWindow->pWindowWidget != NULL) ? xgeXuiWidgetGetName(pWindow->pWindowWidget) : NULL;
	if ( (sName != NULL) && (sName[0] != 0) ) {
		return sName;
	}
	return (pWindow->sTitle != NULL) ? pWindow->sTitle : "";
}

static int __xgeXuiDockStateSetText(xvalue pTable, const char* sKey, const char* sText)
{
	xvalue pText;

	if ( (pTable == NULL) || (sKey == NULL) || (sText == NULL) ) {
		return 0;
	}
	pText = xvoCreateText((void*)sText, (uint32)strlen(sText), FALSE);
	if ( pText == NULL ) {
		return 0;
	}
	if ( !xvoTableSetValue(pTable, sKey, (uint32)strlen(sKey), pText, TRUE) ) {
		xvoUnref(pText);
		return 0;
	}
	return 1;
}

static int __xgeXuiDockStateAppendText(xvalue pArray, const char* sText)
{
	xvalue pText;

	if ( (pArray == NULL) || (sText == NULL) ) {
		return 0;
	}
	pText = xvoCreateText((void*)sText, (uint32)strlen(sText), FALSE);
	if ( pText == NULL ) {
		return 0;
	}
	if ( !xvoArrayAppendValue(pArray, pText, TRUE) ) {
		xvoUnref(pText);
		return 0;
	}
	return 1;
}

static xvalue __xgeXuiDockStateCreateRect(xge_rect_t tRect)
{
	xvalue pRect;

	pRect = xvoCreateArray();
	if ( pRect == NULL ) {
		return NULL;
	}
	if ( !xvoArrayAppendFloat(pRect, tRect.fX) ||
	     !xvoArrayAppendFloat(pRect, tRect.fY) ||
	     !xvoArrayAppendFloat(pRect, tRect.fW) ||
	     !xvoArrayAppendFloat(pRect, tRect.fH) ) {
		xvoUnref(pRect);
		return NULL;
	}
	return pRect;
}

static int __xgeXuiDockStateSetRect(xvalue pTable, const char* sKey, xge_rect_t tRect)
{
	xvalue pRect;

	pRect = __xgeXuiDockStateCreateRect(tRect);
	if ( pRect == NULL ) {
		return 0;
	}
	if ( !xvoTableSetValue(pTable, sKey, (uint32)strlen(sKey), pRect, TRUE) ) {
		xvoUnref(pRect);
		return 0;
	}
	return 1;
}

static xvalue __xgeXuiDockStateAppendTable(xvalue pArray)
{
	uint32 iCount;

	if ( (pArray == NULL) || !xvoArrayAppendTable(pArray) ) {
		return NULL;
	}
	iCount = xvoArrayItemCount(pArray);
	if ( iCount == 0u ) {
		return NULL;
	}
	return xvoArrayGetValue(pArray, iCount - 1u);
}

static xvalue __xgeXuiDockStateCreateNode(const xge_xui_dock_node pNode)
{
	xvalue pNodeState;
	xvalue pTabs;
	xvalue pChild;
	xge_xui_dock_window pWindow;
	uint32 i;

	if ( pNode == NULL ) {
		return xvoCreateNull();
	}
	pNodeState = xvoCreateTable();
	if ( pNodeState == NULL ) {
		return NULL;
	}
	if ( pNode->iType == XGE_XUI_DOCK_NODE_SPLIT ) {
		if ( !__xgeXuiDockStateSetText(pNodeState, "type", "split") ||
		     !__xgeXuiDockStateSetText(pNodeState, "axis", (pNode->iAxis == XGE_XUI_ORIENTATION_HORIZONTAL) ? "horizontal" : "vertical") ||
		     !xvoTableSetFloat(pNodeState, "ratio", 5, pNode->fRatio) ) {
			xvoUnref(pNodeState);
			return NULL;
		}
		pChild = __xgeXuiDockStateCreateNode(pNode->pFirst);
		if ( (pChild == NULL) || !xvoTableSetValue(pNodeState, "first", 5, pChild, TRUE) ) {
			xvoUnref(pChild);
			xvoUnref(pNodeState);
			return NULL;
		}
		pChild = __xgeXuiDockStateCreateNode(pNode->pSecond);
		if ( (pChild == NULL) || !xvoTableSetValue(pNodeState, "second", 6, pChild, TRUE) ) {
			xvoUnref(pChild);
			xvoUnref(pNodeState);
			return NULL;
		}
		return pNodeState;
	}
	if ( !__xgeXuiDockStateSetText(pNodeState, "type", "pane") ) {
		xvoUnref(pNodeState);
		return NULL;
	}
	if ( (pNode->pPane != NULL) && !xvoTableSetInt(pNodeState, "active", 6, pNode->pPane->iActive) ) {
		xvoUnref(pNodeState);
		return NULL;
	}
	pTabs = xvoCreateArray();
	if ( pTabs == NULL ) {
		xvoUnref(pNodeState);
		return NULL;
	}
	if ( pNode->pPane != NULL ) {
		for ( i = 0; i < pNode->pPane->arrWindows.Count; i++ ) {
			pWindow = xgeXuiDockPaneGetWindow(pNode->pPane, (int)i);
			if ( !__xgeXuiDockStateAppendText(pTabs, __xgeXuiDockWindowSaveId(pWindow)) ) {
				xvoUnref(pTabs);
				xvoUnref(pNodeState);
				return NULL;
			}
		}
	}
	if ( !xvoTableSetValue(pNodeState, "tabs", 4, pTabs, TRUE) ) {
		xvoUnref(pTabs);
		xvoUnref(pNodeState);
		return NULL;
	}
	return pNodeState;
}

static int __xgeXuiDockStateAppendRegion(xvalue pRegions, const xge_xui_dock_region pRegion)
{
	xvalue pRegionState;
	xvalue pRootState;

	pRegionState = __xgeXuiDockStateAppendTable(pRegions);
	if ( (pRegionState == NULL) || (pRegion == NULL) ) {
		return 0;
	}
	if ( !__xgeXuiDockStateSetText(pRegionState, "kind", __xgeXuiDockRegionStateName(pRegion->iKind)) ||
	     !xvoTableSetFloat(pRegionState, "portion", 7, pRegion->fPortion) ||
	     !xvoTableSetBool(pRegionState, "visible", 7, pRegion->bVisible != 0) ) {
		return 0;
	}
	pRootState = __xgeXuiDockStateCreateNode(pRegion->pRoot);
	if ( (pRootState == NULL) || !xvoTableSetValue(pRegionState, "root", 4, pRootState, TRUE) ) {
		xvoUnref(pRootState);
		return 0;
	}
	return 1;
}

static int __xgeXuiDockStateAppendWindow(xvalue pWindows, const xge_xui_dock_window pWindow)
{
	xvalue pWindowState;
	const char* sId;

	pWindowState = __xgeXuiDockStateAppendTable(pWindows);
	if ( (pWindowState == NULL) || (pWindow == NULL) ) {
		return 0;
	}
	sId = __xgeXuiDockWindowSaveId(pWindow);
	if ( !__xgeXuiDockStateSetText(pWindowState, "id", sId) ||
	     !__xgeXuiDockStateSetText(pWindowState, "title", pWindow->sTitle != NULL ? pWindow->sTitle : "") ||
	     !__xgeXuiDockStateSetText(pWindowState, "state", __xgeXuiDockWindowStateName(pWindow->iState)) ||
	     !__xgeXuiDockStateSetText(pWindowState, "lastRegion", __xgeXuiDockRegionStateName(pWindow->iLastRegion)) ||
	     !__xgeXuiDockStateSetText(pWindowState, "lastSide", __xgeXuiDockSideStateName(pWindow->iLastSide)) ||
	     !__xgeXuiDockStateSetText(pWindowState, "autoHideRegion", __xgeXuiDockRegionStateName(pWindow->iAutoHideRegion)) ||
	     !xvoTableSetInt(pWindowState, "lastTabIndex", 12, pWindow->iLastTabIndex) ||
	     !xvoTableSetBool(pWindowState, "visible", 7, pWindow->bVisible != 0) ||
	     !xvoTableSetBool(pWindowState, "closable", 8, pWindow->bClosable != 0) ||
	     !xvoTableSetBool(pWindowState, "dockable", 8, pWindow->bDockable != 0) ||
	     !__xgeXuiDockStateSetRect(pWindowState, "lastFloatRect", pWindow->tLastFloatRect) ) {
		return 0;
	}
	return 1;
}

static int __xgeXuiDockStateAppendFloating(xvalue pFloating, const xge_xui_dock_window pWindow, int iZ)
{
	xvalue pFloatingState;
	const char* sId;

	pFloatingState = __xgeXuiDockStateAppendTable(pFloating);
	if ( (pFloatingState == NULL) || (pWindow == NULL) ) {
		return 0;
	}
	sId = __xgeXuiDockWindowSaveId(pWindow);
	if ( !__xgeXuiDockStateSetText(pFloatingState, "id", sId) ||
	     !xvoTableSetInt(pFloatingState, "z", 1, iZ) ||
	     !__xgeXuiDockStateSetRect(pFloatingState, "rect", pWindow->tLastFloatRect) ) {
		return 0;
	}
	return 1;
}

typedef struct xge_xui_dock_load_window_t {
	xge_xui_dock_window pWindow;
	xge_xui_dock_pane pTreePane;
	xge_rect_t tLastFloatRect;
	xge_rect_t tFloatRect;
	int iState;
	int iLastRegion;
	int iLastSide;
	int iLastTabIndex;
	int iAutoHideRegion;
	int iTreeRegion;
	int iTreeTabIndex;
	int bInState;
	int bInTree;
	int bInFloating;
	int bClosable;
	int bDockable;
	int bVisible;
} xge_xui_dock_load_window_t;

typedef struct xge_xui_dock_load_index_t {
	xdict_struct tById;
	int bInit;
} xge_xui_dock_load_index_t;

static int __xgeXuiDockRegionStateValue(const char* sText, int* pRegion)
{
	if ( (sText == NULL) || (pRegion == NULL) ) {
		return 0;
	}
	if ( (strcmp(sText, "document") == 0) || (strcmp(sText, "doc") == 0) || (strcmp(sText, "center") == 0) || (strcmp(sText, "fill") == 0) ) {
		*pRegion = XGE_XUI_DOCK_REGION_DOCUMENT;
		return 1;
	}
	if ( strcmp(sText, "left") == 0 ) {
		*pRegion = XGE_XUI_DOCK_REGION_LEFT;
		return 1;
	}
	if ( strcmp(sText, "right") == 0 ) {
		*pRegion = XGE_XUI_DOCK_REGION_RIGHT;
		return 1;
	}
	if ( strcmp(sText, "top") == 0 ) {
		*pRegion = XGE_XUI_DOCK_REGION_TOP;
		return 1;
	}
	if ( strcmp(sText, "bottom") == 0 ) {
		*pRegion = XGE_XUI_DOCK_REGION_BOTTOM;
		return 1;
	}
	return 0;
}

static int __xgeXuiDockSideStateValue(const char* sText, int* pSide)
{
	if ( (sText == NULL) || (pSide == NULL) ) {
		return 0;
	}
	if ( strcmp(sText, "none") == 0 ) {
		*pSide = XGE_XUI_DOCK_SIDE_NONE;
		return 1;
	}
	if ( strcmp(sText, "left") == 0 ) {
		*pSide = XGE_XUI_DOCK_SIDE_LEFT;
		return 1;
	}
	if ( strcmp(sText, "right") == 0 ) {
		*pSide = XGE_XUI_DOCK_SIDE_RIGHT;
		return 1;
	}
	if ( strcmp(sText, "top") == 0 ) {
		*pSide = XGE_XUI_DOCK_SIDE_TOP;
		return 1;
	}
	if ( strcmp(sText, "bottom") == 0 ) {
		*pSide = XGE_XUI_DOCK_SIDE_BOTTOM;
		return 1;
	}
	if ( (strcmp(sText, "fill") == 0) || (strcmp(sText, "center") == 0) || (strcmp(sText, "tab") == 0) ) {
		*pSide = XGE_XUI_DOCK_SIDE_FILL;
		return 1;
	}
	return 0;
}

static int __xgeXuiDockWindowStateValue(const char* sText, int* pState)
{
	if ( (sText == NULL) || (pState == NULL) ) {
		return 0;
	}
	if ( (strcmp(sText, "floating") == 0) || (strcmp(sText, "float") == 0) ) {
		*pState = XGE_XUI_DOCK_WINDOW_FLOATING;
		return 1;
	}
	if ( (strcmp(sText, "docked") == 0) || (strcmp(sText, "dock") == 0) ) {
		*pState = XGE_XUI_DOCK_WINDOW_DOCKED;
		return 1;
	}
	if ( (strcmp(sText, "hidden") == 0) || (strcmp(sText, "hide") == 0) ) {
		*pState = XGE_XUI_DOCK_WINDOW_HIDDEN;
		return 1;
	}
	if ( (strcmp(sText, "autoHide") == 0) || (strcmp(sText, "auto-hide") == 0) || (strcmp(sText, "autohide") == 0) ) {
		*pState = XGE_XUI_DOCK_WINDOW_AUTO_HIDE;
		return 1;
	}
	return 0;
}

static int __xgeXuiDockAxisStateValue(const char* sText, int* pAxis)
{
	if ( (sText == NULL) || (pAxis == NULL) ) {
		return 0;
	}
	if ( strcmp(sText, "horizontal") == 0 ) {
		*pAxis = XGE_XUI_ORIENTATION_HORIZONTAL;
		return 1;
	}
	if ( strcmp(sText, "vertical") == 0 ) {
		*pAxis = XGE_XUI_ORIENTATION_VERTICAL;
		return 1;
	}
	return 0;
}

static xvalue __xgeXuiDockLoadTableGet(xvalue pTable, const char* sKey)
{
	if ( (pTable == NULL) || (sKey == NULL) || (xvoType(pTable) != XVO_DT_TABLE) ) {
		return NULL;
	}
	return xvoTableGetValue(pTable, sKey, (uint32)strlen(sKey));
}

static int __xgeXuiDockLoadValueToFloat(xvalue pValue, float* pOut)
{
	int iType;

	if ( pOut == NULL ) {
		return 0;
	}
	iType = xvoType(pValue);
	if ( iType == XVO_DT_INT ) {
		*pOut = (float)xvoGetInt(pValue);
		return 1;
	}
	if ( iType == XVO_DT_FLOAT ) {
		*pOut = (float)xvoGetFloat(pValue);
		return 1;
	}
	return 0;
}

static int __xgeXuiDockLoadValueToInt(xvalue pValue, int* pOut)
{
	int iType;

	if ( pOut == NULL ) {
		return 0;
	}
	iType = xvoType(pValue);
	if ( iType == XVO_DT_INT ) {
		*pOut = (int)xvoGetInt(pValue);
		return 1;
	}
	if ( iType == XVO_DT_FLOAT ) {
		*pOut = (int)xvoGetFloat(pValue);
		return 1;
	}
	return 0;
}

static int __xgeXuiDockLoadValueToBool(xvalue pValue, int* pOut)
{
	int iType;

	if ( pOut == NULL ) {
		return 0;
	}
	iType = xvoType(pValue);
	if ( iType == XVO_DT_BOOL ) {
		*pOut = xvoGetBool(pValue) ? 1 : 0;
		return 1;
	}
	if ( iType == XVO_DT_INT ) {
		*pOut = (xvoGetInt(pValue) != 0) ? 1 : 0;
		return 1;
	}
	return 0;
}

static int __xgeXuiDockLoadReadRect(xvalue pValue, xge_rect_t* pRect)
{
	if ( (pRect == NULL) || (xvoType(pValue) != XVO_DT_ARRAY) || (xvoArrayItemCount(pValue) != 4u) ) {
		return 0;
	}
	return __xgeXuiDockLoadValueToFloat(xvoArrayGetValue(pValue, 0), &pRect->fX) &&
	       __xgeXuiDockLoadValueToFloat(xvoArrayGetValue(pValue, 1), &pRect->fY) &&
	       __xgeXuiDockLoadValueToFloat(xvoArrayGetValue(pValue, 2), &pRect->fW) &&
	       __xgeXuiDockLoadValueToFloat(xvoArrayGetValue(pValue, 3), &pRect->fH);
}

static int __xgeXuiDockLoadFindInfoByWindow(xge_xui_dock_load_window_t* pInfos, int iCount, xge_xui_dock_window pWindow)
{
	int i;

	if ( (pInfos == NULL) || (pWindow == NULL) ) {
		return -1;
	}
	for ( i = 0; i < iCount; i++ ) {
		if ( pInfos[i].pWindow == pWindow ) {
			return i;
		}
	}
	return -1;
}

static void __xgeXuiDockLoadIndexUnit(xge_xui_dock_load_index_t* pIndex)
{
	if ( (pIndex == NULL) || (pIndex->bInit == 0) ) {
		return;
	}
	xrtDictUnit(&pIndex->tById);
	pIndex->bInit = 0;
}

static int __xgeXuiDockLoadIndexInit(xge_xui_dock_load_index_t* pIndex, xge_xui_dock_load_window_t* pInfos, int iCount)
{
	const char* sId;
	int* pSlot;
	bool bNew;
	int i;

	if ( pIndex == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pIndex, 0, sizeof(*pIndex));
	xrtDictInit(&pIndex->tById, sizeof(int), XRT_OBJMODE_LOCAL);
	pIndex->bInit = 1;
	if ( pInfos == NULL ) {
		return XGE_OK;
	}
	for ( i = 0; i < iCount; i++ ) {
		sId = __xgeXuiDockWindowSaveId(pInfos[i].pWindow);
		if ( (sId == NULL) || (sId[0] == 0) ) {
			continue;
		}
		bNew = false;
		pSlot = (int*)xrtDictSet(&pIndex->tById, (ptr)sId, (uint32)strlen(sId), &bNew);
		if ( pSlot == NULL ) {
			__xgeXuiDockLoadIndexUnit(pIndex);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		if ( !bNew ) {
			__xgeXuiDockLoadIndexUnit(pIndex);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		*pSlot = i;
	}
	return XGE_OK;
}

static int __xgeXuiDockLoadIndexFindInfoById(const xge_xui_dock_load_index_t* pIndex, const char* sId)
{
	int* pSlot;

	if ( (pIndex == NULL) || (pIndex->bInit == 0) || (sId == NULL) || (sId[0] == 0) ) {
		return -1;
	}
	pSlot = (int*)xrtDictGet((xdict)&pIndex->tById, (ptr)sId, (uint32)strlen(sId));
	return (pSlot != NULL) ? *pSlot : -1;
}

static int __xgeXuiDockLoadPrepareWindows(xge_xui_dock_layout pLayout, xvalue pState, xge_xui_dock_load_window_t** ppInfos, int* pCount)
{
	xvalue pWindows;
	xvalue pItem;
	xvalue pValue;
	const char* sText;
	xge_xui_dock_load_window_t* pInfos;
	xge_xui_dock_load_index_t tIndex;
	xge_xui_dock_window* ppWindow;
	uint32 i;
	int iCount;
	int iIndex;
	int iRet;

	if ( (pLayout == NULL) || (ppInfos == NULL) || (pCount == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppInfos = NULL;
	*pCount = 0;
	pWindows = __xgeXuiDockLoadTableGet(pState, "windows");
	if ( xvoType(pWindows) != XVO_DT_ARRAY ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iCount = (int)pLayout->arrWindows.Count;
	pInfos = NULL;
	memset(&tIndex, 0, sizeof(tIndex));
	if ( iCount > 0 ) {
		pInfos = (xge_xui_dock_load_window_t*)xrtMalloc(sizeof(*pInfos) * (size_t)iCount);
		if ( pInfos == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		memset(pInfos, 0, sizeof(*pInfos) * (size_t)iCount);
		for ( i = 0; i < (uint32)iCount; i++ ) {
			ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline(&pLayout->arrWindows, i + 1u);
			pInfos[i].pWindow = (ppWindow != NULL) ? *ppWindow : NULL;
			pInfos[i].tLastFloatRect = (pInfos[i].pWindow != NULL) ? pInfos[i].pWindow->tLastFloatRect : __xgeXuiDockRectZero();
			pInfos[i].tFloatRect = pInfos[i].tLastFloatRect;
			pInfos[i].iState = (pInfos[i].pWindow != NULL) ? pInfos[i].pWindow->iState : XGE_XUI_DOCK_WINDOW_HIDDEN;
			pInfos[i].iLastRegion = (pInfos[i].pWindow != NULL) ? pInfos[i].pWindow->iLastRegion : XGE_XUI_DOCK_REGION_DOCUMENT;
			pInfos[i].iLastSide = (pInfos[i].pWindow != NULL) ? pInfos[i].pWindow->iLastSide : XGE_XUI_DOCK_SIDE_FILL;
			pInfos[i].iLastTabIndex = (pInfos[i].pWindow != NULL) ? pInfos[i].pWindow->iLastTabIndex : -1;
			pInfos[i].iAutoHideRegion = (pInfos[i].pWindow != NULL) ? pInfos[i].pWindow->iAutoHideRegion : XGE_XUI_DOCK_REGION_LEFT;
			pInfos[i].iTreeRegion = XGE_XUI_DOCK_REGION_DOCUMENT;
			pInfos[i].iTreeTabIndex = -1;
			pInfos[i].bClosable = (pInfos[i].pWindow != NULL) ? pInfos[i].pWindow->bClosable : 1;
			pInfos[i].bDockable = (pInfos[i].pWindow != NULL) ? pInfos[i].pWindow->bDockable : 1;
			pInfos[i].bVisible = (pInfos[i].pWindow != NULL) ? pInfos[i].pWindow->bVisible : 0;
		}
	}
	iRet = __xgeXuiDockLoadIndexInit(&tIndex, pInfos, iCount);
	if ( iRet != XGE_OK ) {
		goto cleanup;
	}
	for ( i = 0; i < xvoArrayItemCount(pWindows); i++ ) {
		pItem = xvoArrayGetValue(pWindows, i);
		if ( xvoType(pItem) != XVO_DT_TABLE ) {
			iRet = XGE_ERROR_INVALID_ARGUMENT;
			goto cleanup;
		}
		pValue = __xgeXuiDockLoadTableGet(pItem, "id");
		if ( xvoType(pValue) != XVO_DT_TEXT ) {
			iRet = XGE_ERROR_INVALID_ARGUMENT;
			goto cleanup;
		}
		iIndex = __xgeXuiDockLoadIndexFindInfoById(&tIndex, (const char*)xvoGetText(pValue));
		if ( (iIndex < 0) || (pInfos[iIndex].bInState != 0) ) {
			iRet = XGE_ERROR_INVALID_ARGUMENT;
			goto cleanup;
		}
		pInfos[iIndex].bInState = 1;
		pValue = __xgeXuiDockLoadTableGet(pItem, "state");
		if ( xvoType(pValue) == XVO_DT_TEXT ) {
			sText = (const char*)xvoGetText(pValue);
			if ( !__xgeXuiDockWindowStateValue(sText, &pInfos[iIndex].iState) ) {
				iRet = XGE_ERROR_INVALID_ARGUMENT;
				goto cleanup;
			}
		}
		pValue = __xgeXuiDockLoadTableGet(pItem, "lastRegion");
		if ( xvoType(pValue) == XVO_DT_TEXT ) {
			if ( !__xgeXuiDockRegionStateValue((const char*)xvoGetText(pValue), &pInfos[iIndex].iLastRegion) ) {
				iRet = XGE_ERROR_INVALID_ARGUMENT;
				goto cleanup;
			}
		}
		pValue = __xgeXuiDockLoadTableGet(pItem, "lastSide");
		if ( xvoType(pValue) == XVO_DT_TEXT ) {
			if ( !__xgeXuiDockSideStateValue((const char*)xvoGetText(pValue), &pInfos[iIndex].iLastSide) ) {
				iRet = XGE_ERROR_INVALID_ARGUMENT;
				goto cleanup;
			}
		}
		pValue = __xgeXuiDockLoadTableGet(pItem, "autoHideRegion");
		if ( xvoType(pValue) == XVO_DT_TEXT ) {
			if ( !__xgeXuiDockRegionStateValue((const char*)xvoGetText(pValue), &pInfos[iIndex].iAutoHideRegion) || (pInfos[iIndex].iAutoHideRegion == XGE_XUI_DOCK_REGION_DOCUMENT) ) {
				iRet = XGE_ERROR_INVALID_ARGUMENT;
				goto cleanup;
			}
		}
		pValue = __xgeXuiDockLoadTableGet(pItem, "lastTabIndex");
		if ( xvoType(pValue) != XVO_DT_NULL && !__xgeXuiDockLoadValueToInt(pValue, &pInfos[iIndex].iLastTabIndex) ) {
			iRet = XGE_ERROR_INVALID_ARGUMENT;
			goto cleanup;
		}
		pValue = __xgeXuiDockLoadTableGet(pItem, "visible");
		if ( xvoType(pValue) != XVO_DT_NULL && !__xgeXuiDockLoadValueToBool(pValue, &pInfos[iIndex].bVisible) ) {
			iRet = XGE_ERROR_INVALID_ARGUMENT;
			goto cleanup;
		}
		pValue = __xgeXuiDockLoadTableGet(pItem, "closable");
		if ( xvoType(pValue) != XVO_DT_NULL && !__xgeXuiDockLoadValueToBool(pValue, &pInfos[iIndex].bClosable) ) {
			iRet = XGE_ERROR_INVALID_ARGUMENT;
			goto cleanup;
		}
		pValue = __xgeXuiDockLoadTableGet(pItem, "dockable");
		if ( xvoType(pValue) != XVO_DT_NULL && !__xgeXuiDockLoadValueToBool(pValue, &pInfos[iIndex].bDockable) ) {
			iRet = XGE_ERROR_INVALID_ARGUMENT;
			goto cleanup;
		}
		pValue = __xgeXuiDockLoadTableGet(pItem, "lastFloatRect");
		if ( xvoType(pValue) != XVO_DT_NULL && !__xgeXuiDockLoadReadRect(pValue, &pInfos[iIndex].tLastFloatRect) ) {
			iRet = XGE_ERROR_INVALID_ARGUMENT;
			goto cleanup;
		}
		pInfos[iIndex].tFloatRect = pInfos[iIndex].tLastFloatRect;
	}
	*ppInfos = pInfos;
	*pCount = iCount;
	pInfos = NULL;
	iRet = XGE_OK;

cleanup:
	__xgeXuiDockLoadIndexUnit(&tIndex);
	xrtFree(pInfos);
	return iRet;
}

static int __xgeXuiDockLoadAppendPaneWindow(xge_xui_dock_pane pPane, xge_xui_dock_window pWindow)
{
	uint32 iIndex;
	xge_xui_dock_window* ppItem;

	if ( (pPane == NULL) || (pWindow == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = xrtArrayAppend(&pPane->arrWindows, 1u);
	if ( iIndex == 0u ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	ppItem = (xge_xui_dock_window*)xrtArrayGet_Inline(&pPane->arrWindows, iIndex);
	if ( ppItem == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	*ppItem = pWindow;
	return XGE_OK;
}

static int __xgeXuiDockLoadBuildNode(xge_xui_dock_layout pLayout, xvalue pNodeState, int iRegion, const xge_xui_dock_load_index_t* pIndex, xge_xui_dock_load_window_t* pInfos, int iInfoCount, xge_xui_dock_node* ppNode)
{
	xge_xui_dock_node pNode;
	xge_xui_dock_node pFirst;
	xge_xui_dock_node pSecond;
	xvalue pValue;
	xvalue pTabs;
	const char* sText;
	float fRatio;
	int iAxis;
	int iActive;
	int iIndex;
	uint32 i;
	int iRet;

	if ( ppNode == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	*ppNode = NULL;
	if ( xvoType(pNodeState) == XVO_DT_NULL ) {
		return XGE_OK;
	}
	if ( xvoType(pNodeState) != XVO_DT_TABLE ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pValue = __xgeXuiDockLoadTableGet(pNodeState, "type");
	if ( xvoType(pValue) != XVO_DT_TEXT ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sText = (const char*)xvoGetText(pValue);
	if ( strcmp(sText, "split") == 0 ) {
		iAxis = XGE_XUI_ORIENTATION_VERTICAL;
		pValue = __xgeXuiDockLoadTableGet(pNodeState, "axis");
		if ( (xvoType(pValue) != XVO_DT_TEXT) || !__xgeXuiDockAxisStateValue((const char*)xvoGetText(pValue), &iAxis) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		fRatio = 0.50f;
		pValue = __xgeXuiDockLoadTableGet(pNodeState, "ratio");
		if ( (xvoType(pValue) != XVO_DT_NULL) && !__xgeXuiDockLoadValueToFloat(pValue, &fRatio) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		pFirst = NULL;
		pSecond = NULL;
		iRet = __xgeXuiDockLoadBuildNode(pLayout, __xgeXuiDockLoadTableGet(pNodeState, "first"), iRegion, pIndex, pInfos, iInfoCount, &pFirst);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		iRet = __xgeXuiDockLoadBuildNode(pLayout, __xgeXuiDockLoadTableGet(pNodeState, "second"), iRegion, pIndex, pInfos, iInfoCount, &pSecond);
		if ( iRet != XGE_OK ) {
			__xgeXuiDockNodeDestroy(pFirst);
			return iRet;
		}
		if ( (pFirst == NULL) || (pSecond == NULL) ) {
			__xgeXuiDockNodeDestroy(pFirst);
			__xgeXuiDockNodeDestroy(pSecond);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		pNode = __xgeXuiDockNodeCreateSplit(pFirst, pSecond, iAxis, fRatio);
		if ( pNode == NULL ) {
			__xgeXuiDockNodeDestroy(pFirst);
			__xgeXuiDockNodeDestroy(pSecond);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		*ppNode = pNode;
		return XGE_OK;
	}
	if ( strcmp(sText, "pane") != 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pNode = __xgeXuiDockNodeCreatePane(pLayout);
	if ( pNode == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iActive = -1;
	pValue = __xgeXuiDockLoadTableGet(pNodeState, "active");
	if ( (xvoType(pValue) != XVO_DT_NULL) && !__xgeXuiDockLoadValueToInt(pValue, &iActive) ) {
		__xgeXuiDockNodeDestroy(pNode);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pTabs = __xgeXuiDockLoadTableGet(pNodeState, "tabs");
	if ( xvoType(pTabs) != XVO_DT_ARRAY ) {
		__xgeXuiDockNodeDestroy(pNode);
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < xvoArrayItemCount(pTabs); i++ ) {
		pValue = xvoArrayGetValue(pTabs, i);
		if ( xvoType(pValue) != XVO_DT_TEXT ) {
			__xgeXuiDockNodeDestroy(pNode);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iIndex = __xgeXuiDockLoadIndexFindInfoById(pIndex, (const char*)xvoGetText(pValue));
		if ( (iIndex < 0) || (iIndex >= iInfoCount) || (pInfos[iIndex].bInState == 0) || (pInfos[iIndex].bInTree != 0) || (pInfos[iIndex].bInFloating != 0) ) {
			__xgeXuiDockNodeDestroy(pNode);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iRet = __xgeXuiDockLoadAppendPaneWindow(pNode->pPane, pInfos[iIndex].pWindow);
		if ( iRet != XGE_OK ) {
			__xgeXuiDockNodeDestroy(pNode);
			return iRet;
		}
		pInfos[iIndex].bInTree = 1;
		pInfos[iIndex].pTreePane = pNode->pPane;
		pInfos[iIndex].iTreeRegion = iRegion;
		pInfos[iIndex].iTreeTabIndex = (int)i;
	}
	if ( pNode->pPane != NULL ) {
		if ( pNode->pPane->arrWindows.Count == 0u ) {
			pNode->pPane->iActive = -1;
		} else {
			if ( iActive < 0 ) {
				iActive = 0;
			}
			if ( iActive >= (int)pNode->pPane->arrWindows.Count ) {
				iActive = (int)pNode->pPane->arrWindows.Count - 1;
			}
			pNode->pPane->iActive = iActive;
		}
	}
	*ppNode = pNode;
	return XGE_OK;
}

static int __xgeXuiDockLoadBuildRegions(xge_xui_dock_layout pLayout, xvalue pState, const xge_xui_dock_load_index_t* pIndex, xge_xui_dock_load_window_t* pInfos, int iInfoCount, xge_xui_dock_node* arrRoots, float* arrPortions, int* arrVisible)
{
	xvalue pRegions;
	xvalue pItem;
	xvalue pValue;
	int arrSeen[XGE_XUI_DOCK_REGION_COUNT];
	float fPortion;
	int iVisible;
	int iRegion;
	uint32 i;
	int iRet;

	if ( (pLayout == NULL) || (arrRoots == NULL) || (arrPortions == NULL) || (arrVisible == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		arrRoots[i] = NULL;
		arrPortions[i] = pLayout->arrRegions[i].fPortion;
		arrVisible[i] = (i == XGE_XUI_DOCK_REGION_DOCUMENT) ? 1 : 0;
		arrSeen[i] = 0;
	}
	pRegions = __xgeXuiDockLoadTableGet(pState, "regions");
	if ( (xvoType(pRegions) != XVO_DT_ARRAY) || (xvoArrayItemCount(pRegions) != XGE_XUI_DOCK_REGION_COUNT) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < xvoArrayItemCount(pRegions); i++ ) {
		pItem = xvoArrayGetValue(pRegions, i);
		if ( xvoType(pItem) != XVO_DT_TABLE ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iRegion = (int)i;
		pValue = __xgeXuiDockLoadTableGet(pItem, "kind");
		if ( xvoType(pValue) == XVO_DT_TEXT ) {
			if ( !__xgeXuiDockRegionStateValue((const char*)xvoGetText(pValue), &iRegion) ) {
				return XGE_ERROR_INVALID_ARGUMENT;
			}
		} else if ( xvoType(pValue) != XVO_DT_NULL ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( !__xgeXuiDockRegionValid(iRegion) || arrSeen[iRegion] ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		arrSeen[iRegion] = 1;
		fPortion = arrPortions[iRegion];
		pValue = __xgeXuiDockLoadTableGet(pItem, "portion");
		if ( (xvoType(pValue) != XVO_DT_NULL) && !__xgeXuiDockLoadValueToFloat(pValue, &fPortion) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iVisible = arrVisible[iRegion];
		pValue = __xgeXuiDockLoadTableGet(pItem, "visible");
		if ( (xvoType(pValue) != XVO_DT_NULL) && !__xgeXuiDockLoadValueToBool(pValue, &iVisible) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		arrPortions[iRegion] = __xgeXuiDockClampPortion(fPortion);
		arrVisible[iRegion] = iVisible ? 1 : 0;
		iRet = __xgeXuiDockLoadBuildNode(pLayout, __xgeXuiDockLoadTableGet(pItem, "root"), iRegion, pIndex, pInfos, iInfoCount, &arrRoots[iRegion]);
		if ( iRet != XGE_OK ) {
			return iRet;
		}
		if ( (iRegion != XGE_XUI_DOCK_REGION_DOCUMENT) && (arrRoots[iRegion] == NULL) ) {
			arrVisible[iRegion] = 0;
		}
	}
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		if ( !arrSeen[i] ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static int __xgeXuiDockLoadBuildFloating(xge_xui_dock_layout pLayout, xvalue pState, const xge_xui_dock_load_index_t* pIndex, xge_xui_dock_load_window_t* pInfos, int iInfoCount, xarray pFloating)
{
	xvalue pFloatingState;
	xvalue pItem;
	xvalue pValue;
	xge_xui_dock_window* ppItem;
	xge_rect_t tRect;
	int iIndex;
	uint32 i;
	uint32 iSlot;

	(void)pLayout;
	if ( pFloating == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFloatingState = __xgeXuiDockLoadTableGet(pState, "floating");
	if ( xvoType(pFloatingState) != XVO_DT_ARRAY ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < xvoArrayItemCount(pFloatingState); i++ ) {
		pItem = xvoArrayGetValue(pFloatingState, i);
		if ( xvoType(pItem) != XVO_DT_TABLE ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		pValue = __xgeXuiDockLoadTableGet(pItem, "id");
		if ( xvoType(pValue) != XVO_DT_TEXT ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iIndex = __xgeXuiDockLoadIndexFindInfoById(pIndex, (const char*)xvoGetText(pValue));
		if ( (iIndex < 0) || (iIndex >= iInfoCount) || (pInfos[iIndex].bInState == 0) || (pInfos[iIndex].bInTree != 0) || (pInfos[iIndex].bInFloating != 0) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		tRect = pInfos[iIndex].tLastFloatRect;
		pValue = __xgeXuiDockLoadTableGet(pItem, "rect");
		if ( (xvoType(pValue) != XVO_DT_NULL) && !__xgeXuiDockLoadReadRect(pValue, &tRect) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		iSlot = xrtArrayAppend(pFloating, 1u);
		if ( iSlot == 0u ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		ppItem = (xge_xui_dock_window*)xrtArrayGet_Inline(pFloating, iSlot);
		if ( ppItem == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		*ppItem = pInfos[iIndex].pWindow;
		pInfos[iIndex].bInFloating = 1;
		pInfos[iIndex].tFloatRect = tRect;
	}
	return XGE_OK;
}

static int __xgeXuiDockLoadValidateWindowUsage(xge_xui_dock_load_window_t* pInfos, int iInfoCount)
{
	int i;

	for ( i = 0; i < iInfoCount; i++ ) {
		if ( pInfos[i].bInState == 0 ) {
			continue;
		}
		if ( pInfos[i].bInTree && (pInfos[i].iState != XGE_XUI_DOCK_WINDOW_DOCKED) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( pInfos[i].bInFloating && (pInfos[i].iState != XGE_XUI_DOCK_WINDOW_FLOATING) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
		if ( !pInfos[i].bInTree && !pInfos[i].bInFloating && (pInfos[i].iState != XGE_XUI_DOCK_WINDOW_HIDDEN) && (pInfos[i].iState != XGE_XUI_DOCK_WINDOW_AUTO_HIDE) ) {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static void __xgeXuiDockLoadApplyNode(xge_xui_dock_layout pLayout, xge_xui_dock_node pNode, int iRegion, xge_xui_dock_load_window_t* pInfos, int iInfoCount)
{
	xge_xui_dock_pane pPane;
	xge_xui_dock_window pWindow;
	xge_xui_dock_window* ppWindow;
	int i;
	int iIndex;

	if ( pNode == NULL ) {
		return;
	}
	if ( pNode->iType == XGE_XUI_DOCK_NODE_SPLIT ) {
		__xgeXuiDockLoadApplyNode(pLayout, pNode->pFirst, iRegion, pInfos, iInfoCount);
		__xgeXuiDockLoadApplyNode(pLayout, pNode->pSecond, iRegion, pInfos, iInfoCount);
		return;
	}
	if ( pNode->iType != XGE_XUI_DOCK_NODE_PANE ) {
		return;
	}
	pPane = pNode->pPane;
	if ( pPane == NULL ) {
		return;
	}
	pPane->pLayout = pLayout;
	pPane->pNode = pNode;
	if ( pPane->arrWindows.Count == 0u ) {
		pPane->iActive = -1;
		return;
	}
	if ( pPane->iActive < 0 ) {
		pPane->iActive = 0;
	}
	if ( pPane->iActive >= (int)pPane->arrWindows.Count ) {
		pPane->iActive = (int)pPane->arrWindows.Count - 1;
	}
	for ( i = 0; i < (int)pPane->arrWindows.Count; i++ ) {
		ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline(&pPane->arrWindows, (uint32)i + 1u);
		pWindow = (ppWindow != NULL) ? *ppWindow : NULL;
		iIndex = __xgeXuiDockLoadFindInfoByWindow(pInfos, iInfoCount, pWindow);
		if ( (pWindow == NULL) || (iIndex < 0) ) {
			continue;
		}
		pWindow->pLayout = pLayout;
		pWindow->pPane = pPane;
		pWindow->iState = XGE_XUI_DOCK_WINDOW_DOCKED;
		pWindow->bVisible = 1;
		pWindow->tAutoHideStripRect = __xgeXuiDockRectZero();
		pWindow->iLastRegion = iRegion;
		pWindow->iLastSide = (pInfos[iIndex].bInState && __xgeXuiDockSideIsSplit(pInfos[iIndex].iLastSide)) ? pInfos[iIndex].iLastSide : XGE_XUI_DOCK_SIDE_FILL;
		pWindow->iLastTabIndex = i;
		xgeXuiWindowSetOpen(&pWindow->tWindow, 0);
		__xgeXuiDockWindowAttachClientToLayout(pLayout, pWindow);
		if ( i == pPane->iActive ) {
			__xgeXuiDockLayoutRequestFocusWindow(pLayout, pWindow);
		}
	}
}

static void __xgeXuiDockLoadCommit(xge_xui_dock_layout pLayout, xge_xui_dock_node* arrRoots, const float* arrPortions, const int* arrVisible, xarray pFloating, xge_xui_dock_load_window_t* pInfos, int iInfoCount)
{
	xge_xui_dock_window pWindow;
	xge_xui_dock_window* ppWindow;
	xge_rect_t tRect;
	uint32 i;
	int r;

	__xgeXuiDockLayoutCancelDrag(pLayout);
	__xgeXuiDockLayoutEndSplitterDrag(pLayout);
	__xgeXuiDockLayoutCloseOptionMenu(pLayout);
	__xgeXuiDockLayoutCloseOverflowMenu(pLayout);
	__xgeXuiDockLayoutClearActivePart(pLayout);
	if ( (pLayout->pContext != NULL) && (pLayout->pWidget != NULL) ) {
		xgeXuiReleaseWidgetCapture(pLayout->pContext, pLayout->pWidget);
	}
	for ( i = 0; i < pLayout->arrWindows.Count; i++ ) {
		ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline(&pLayout->arrWindows, i + 1u);
		pWindow = (ppWindow != NULL) ? *ppWindow : NULL;
		if ( pWindow != NULL ) {
			pWindow->pLayout = pLayout;
			pWindow->pPane = NULL;
		}
	}
	for ( r = 0; r < XGE_XUI_DOCK_REGION_COUNT; r++ ) {
		__xgeXuiDockNodeDestroy(pLayout->arrRegions[r].pRoot);
		pLayout->arrRegions[r].pRoot = arrRoots[r];
		arrRoots[r] = NULL;
		pLayout->arrRegions[r].fPortion = arrPortions[r];
		pLayout->arrRegions[r].bVisible = (pLayout->arrRegions[r].pRoot != NULL || r == XGE_XUI_DOCK_REGION_DOCUMENT) ? (arrVisible[r] ? 1 : 0) : 0;
		if ( r == XGE_XUI_DOCK_REGION_DOCUMENT ) {
			pLayout->arrRegions[r].bVisible = 1;
		}
	}
	xrtArrayUnit(&pLayout->arrFloatingWindows);
	pLayout->arrFloatingWindows = *pFloating;
	memset(pFloating, 0, sizeof(*pFloating));
	pLayout->pPendingFocusWindow = NULL;
	for ( i = 0; i < (uint32)iInfoCount; i++ ) {
		pWindow = pInfos[i].pWindow;
		if ( pWindow == NULL ) {
			continue;
		}
		if ( pInfos[i].bInState ) {
			pWindow->iLastRegion = pInfos[i].iLastRegion;
			pWindow->iLastSide = pInfos[i].iLastSide;
			pWindow->iLastTabIndex = pInfos[i].iLastTabIndex;
			pWindow->iAutoHideRegion = pInfos[i].iAutoHideRegion;
			pWindow->bClosable = pInfos[i].bClosable;
			pWindow->bDockable = pInfos[i].bDockable;
			pWindow->bVisible = pInfos[i].bVisible;
			pWindow->tLastFloatRect = pInfos[i].tLastFloatRect;
		}
		if ( pInfos[i].bInTree ) {
			continue;
		}
		if ( pInfos[i].bInFloating ) {
			tRect = __xgeXuiDockClampFloatRect(pLayout, pInfos[i].tFloatRect);
			pWindow->pLayout = pLayout;
			pWindow->pPane = NULL;
			pWindow->iState = XGE_XUI_DOCK_WINDOW_FLOATING;
			pWindow->bVisible = 1;
			pWindow->tAutoHideStripRect = __xgeXuiDockRectZero();
			pWindow->tLastFloatRect = tRect;
			__xgeXuiDockWindowAttachClientToBase(pWindow);
			xgeXuiWindowSetOpen(&pWindow->tWindow, 1);
			if ( pWindow->pWindowWidget != NULL ) {
				xgeXuiWidgetSetRect(pWindow->pWindowWidget, tRect);
			}
			continue;
		}
		if ( pInfos[i].bInState && (pInfos[i].iState == XGE_XUI_DOCK_WINDOW_AUTO_HIDE) ) {
			pWindow->pLayout = pLayout;
			pWindow->pPane = NULL;
			pWindow->iState = XGE_XUI_DOCK_WINDOW_AUTO_HIDE;
			pWindow->bVisible = 0;
			pWindow->tAutoHideStripRect = __xgeXuiDockRectZero();
			__xgeXuiDockWindowAttachClientToBase(pWindow);
			xgeXuiWindowSetOpen(&pWindow->tWindow, 0);
			continue;
		}
		pWindow->pLayout = pLayout;
		pWindow->pPane = NULL;
		pWindow->iState = XGE_XUI_DOCK_WINDOW_HIDDEN;
		pWindow->bVisible = 0;
		pWindow->tAutoHideStripRect = __xgeXuiDockRectZero();
		__xgeXuiDockWindowAttachClientToBase(pWindow);
		xgeXuiWindowSetOpen(&pWindow->tWindow, 0);
	}
	for ( r = 0; r < XGE_XUI_DOCK_REGION_COUNT; r++ ) {
		__xgeXuiDockLoadApplyNode(pLayout, pLayout->arrRegions[r].pRoot, r, pInfos, iInfoCount);
	}
	for ( i = 0; i < pLayout->arrFloatingWindows.Count; i++ ) {
		ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline(&pLayout->arrFloatingWindows, i + 1u);
		if ( (ppWindow != NULL) && (*ppWindow != NULL) ) {
			xgeXuiWindowBringToFront(&(*ppWindow)->tWindow);
		}
	}
	xgeXuiWidgetMarkLayout(pLayout->pWidget);
	xgeXuiWidgetMarkPaint(pLayout->pWidget);
}

int xgeXuiDockLayoutInit(xge_xui_dock_layout pLayout, xge_xui_context pContext, xge_xui_widget pWidget)
{
	int i;

	if ( (pLayout == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->pContext = pContext;
	pLayout->pWidget = pWidget;
	pLayout->fSidePortionDefault = 0.22f;
	pLayout->fSplitterSize = 4.0f;
	pLayout->fTabStripHeight = 25.0f;
	pLayout->fCaptionHeight = 25.0f;
	pLayout->fButtonWidth = 16.0f;
	pLayout->fButtonHeight = 15.0f;
	pLayout->iDragPhase = XGE_XUI_DOCK_DRAG_IDLE;
	pLayout->iDragSourceIndex = -1;
	xrtArrayInit(&pLayout->arrWindows, sizeof(xge_xui_dock_window), XRT_OBJMODE_LOCAL);
	xrtArrayInit(&pLayout->arrFloatingWindows, sizeof(xge_xui_dock_window), XRT_OBJMODE_LOCAL);
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		pLayout->arrRegions[i].pLayout = pLayout;
		pLayout->arrRegions[i].iKind = i;
		pLayout->arrRegions[i].fPortion = (i == XGE_XUI_DOCK_REGION_DOCUMENT) ? 1.0f : pLayout->fSidePortionDefault;
		pLayout->arrRegions[i].fMinSize = (i == XGE_XUI_DOCK_REGION_DOCUMENT) ? 0.0f : 80.0f;
		pLayout->arrRegions[i].fMaxSize = 0.0f;
		pLayout->arrRegions[i].bVisible = (i == XGE_XUI_DOCK_REGION_DOCUMENT) ? 1 : 0;
	}
	xgeXuiWidgetSetRole(pWidget, XGE_XUI_WIDGET_ROLE_CONTAINER);
	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(245, 250, 253, 255));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(147, 188, 210, 255));
	xgeXuiWidgetSetMeasureUser(pWidget, __xgeXuiDockLayoutMeasureProc, pLayout);
	xgeXuiWidgetSetLayoutProc(pWidget, __xgeXuiDockLayoutLayoutProc, pLayout);
	xgeXuiWidgetSetPaint(pWidget, __xgeXuiDockLayoutPaintProc, pLayout);
	xgeXuiWidgetSetEvent(pWidget, __xgeXuiDockLayoutEventProc, pLayout);
	xgeXuiWidgetSetTooltipResolver(pWidget, __xgeXuiDockLayoutTooltipResolve, pLayout);
	if ( xgeXuiMenuInit(&pLayout->tOptionMenu, pContext) == XGE_OK ) {
		pLayout->bOptionMenuInit = 1;
		xgeXuiMenuSetSelect(&pLayout->tOptionMenu, __xgeXuiDockLayoutOptionMenuSelect, pLayout);
	}
	if ( xgeXuiMenuInit(&pLayout->tOverflowMenu, pContext) == XGE_OK ) {
		pLayout->bOverflowMenuInit = 1;
		xgeXuiMenuSetSelect(&pLayout->tOverflowMenu, __xgeXuiDockLayoutOverflowMenuSelect, pLayout);
	}
	pWidget->pUser = pLayout;
	return XGE_OK;
}

void xgeXuiDockLayoutUnit(xge_xui_dock_layout pLayout)
{
	uint32 i;
	int r;

	if ( pLayout == NULL ) {
		return;
	}
	__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
	__xgeXuiDockLayoutCancelDrag(pLayout);
	__xgeXuiDockLayoutEndSplitterDrag(pLayout);
	if ( (pLayout->pContext != NULL) && (pLayout->pWidget != NULL) ) {
		xgeXuiReleaseWidgetCapture(pLayout->pContext, pLayout->pWidget);
	}
	if ( pLayout->pAutoHideOverlayWidget != NULL ) {
		xgeXuiOverlayDetach(pLayout->pContext, pLayout->pAutoHideOverlayWidget);
		xgeXuiWidgetSetPaint(pLayout->pAutoHideOverlayWidget, NULL, NULL);
		xgeXuiWidgetSetEvent(pLayout->pAutoHideOverlayWidget, NULL, NULL);
		xgeXuiWidgetFree(pLayout->pAutoHideOverlayWidget);
		pLayout->pAutoHideOverlayWidget = NULL;
	}
	if ( pLayout->pDragOverlayWidget != NULL ) {
		xgeXuiOverlayDetach(pLayout->pContext, pLayout->pDragOverlayWidget);
		xgeXuiWidgetSetPaint(pLayout->pDragOverlayWidget, NULL, NULL);
		xgeXuiWidgetFree(pLayout->pDragOverlayWidget);
		pLayout->pDragOverlayWidget = NULL;
	}
	if ( pLayout->bOptionMenuInit != 0 ) {
		xgeXuiMenuUnit(&pLayout->tOptionMenu);
		pLayout->bOptionMenuInit = 0;
		pLayout->pOptionMenuPane = NULL;
	}
	if ( pLayout->bOverflowMenuInit != 0 ) {
		xgeXuiMenuUnit(&pLayout->tOverflowMenu);
		pLayout->bOverflowMenuInit = 0;
		pLayout->pOverflowMenuPane = NULL;
	}
	for ( i = 0; i < pLayout->arrWindows.Count; i++ ) {
		xge_xui_dock_window* ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline(&pLayout->arrWindows, i + 1u);
		if ( (ppWindow != NULL) && (*ppWindow != NULL) && ((*ppWindow)->pLayout == pLayout) ) {
			__xgeXuiDockWindowAttachClientToBase(*ppWindow);
			(*ppWindow)->pLayout = NULL;
			(*ppWindow)->pPane = NULL;
			(*ppWindow)->iState = XGE_XUI_DOCK_WINDOW_HIDDEN;
			if ( (*ppWindow)->pWindowWidget != NULL ) {
				xgeXuiWindowSetOpen(&(*ppWindow)->tWindow, 0);
			}
		}
	}
	for ( r = 0; r < XGE_XUI_DOCK_REGION_COUNT; r++ ) {
		__xgeXuiDockNodeDestroy(pLayout->arrRegions[r].pRoot);
		pLayout->arrRegions[r].pRoot = NULL;
	}
	xrtArrayUnit(&pLayout->arrFloatingWindows);
	xrtArrayUnit(&pLayout->arrWindows);
	if ( (pLayout->pWidget != NULL) && (pLayout->pWidget->pUser == pLayout) ) {
		xgeXuiWidgetSetMeasure(pLayout->pWidget, NULL);
		xgeXuiWidgetSetLayoutProc(pLayout->pWidget, NULL, NULL);
		xgeXuiWidgetSetPaint(pLayout->pWidget, NULL, NULL);
		xgeXuiWidgetSetEvent(pLayout->pWidget, NULL, NULL);
		xgeXuiWidgetSetTooltipResolver(pLayout->pWidget, NULL, NULL);
		pLayout->pWidget->pUser = NULL;
	}
	memset(pLayout, 0, sizeof(*pLayout));
}

xge_xui_widget xgeXuiDockLayoutWidget(xge_xui_dock_layout pLayout)
{
	return (pLayout != NULL) ? pLayout->pWidget : NULL;
}

xge_xui_dock_pane xgeXuiDockLayoutDockWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow, int iRegion, int iSide, float fProportion)
{
	xge_xui_dock_pane pPane;
	int iDropSide;

	if ( (pLayout == NULL) || (pWindow == NULL) || !__xgeXuiDockRegionValid(iRegion) ) {
		return NULL;
	}
	if ( pWindow->bDockable == 0 ) {
		return NULL;
	}
	__xgeXuiDockLayoutCloseOptionMenu(pLayout);
	__xgeXuiDockLayoutCloseOverflowMenu(pLayout);
	__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
	if ( __xgeXuiDockLayoutRegisterWindow(pLayout, pWindow) != XGE_OK ) {
		return NULL;
	}
	if ( fProportion > 0.0f ) {
		xgeXuiDockLayoutSetRegionPortion(pLayout, iRegion, fProportion);
	}
	iDropSide = __xgeXuiDockSideIsSplit(iSide) ? iSide : XGE_XUI_DOCK_SIDE_FILL;
	pPane = __xgeXuiDockLayoutDropPane(pLayout, iRegion, iDropSide, fProportion);
	if ( pPane == NULL ) {
		return NULL;
	}
	if ( __xgeXuiDockLayoutDockWindowToPane(pLayout, pWindow, pPane, iDropSide) != XGE_OK ) {
		return NULL;
	}
	return pPane;
}

int xgeXuiDockLayoutFloatWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow, xge_rect_t tRect)
{
	int iRet;

	if ( (pLayout == NULL) || (pWindow == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiDockLayoutCloseOptionMenu(pLayout);
	__xgeXuiDockLayoutCloseOverflowMenu(pLayout);
	__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
	iRet = __xgeXuiDockLayoutRegisterWindow(pLayout, pWindow);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	__xgeXuiDockWindowDetachFromPane(pWindow);
	__xgeXuiDockLayoutRemoveFloatingWindow(pLayout, pWindow);
	iRet = __xgeXuiDockArrayAppendWindowUnique(&pLayout->arrFloatingWindows, pWindow);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	tRect = __xgeXuiDockClampFloatRect(pLayout, tRect);
	pWindow->iState = XGE_XUI_DOCK_WINDOW_FLOATING;
	pWindow->bVisible = 1;
	pWindow->tAutoHideStripRect = __xgeXuiDockRectZero();
	pWindow->tLastFloatRect = tRect;
	__xgeXuiDockWindowAttachClientToBase(pWindow);
	xgeXuiWindowSetOpen(&pWindow->tWindow, 1);
	if ( pWindow->pWindowWidget != NULL ) {
		xgeXuiWidgetSetRect(pWindow->pWindowWidget, tRect);
	}
	xgeXuiWindowBringToFront(&pWindow->tWindow);
	__xgeXuiDockLayoutRequestFocusWindow(pLayout, pWindow);
	xgeXuiWidgetMarkLayout(pLayout->pWidget);
	xgeXuiWidgetMarkPaint(pLayout->pWidget);
	return XGE_OK;
}

int xgeXuiDockLayoutHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow)
{
	xge_xui_dock_pane pSourcePane;
	int iRet;
	int bFocusFallback;

	if ( (pLayout == NULL) || (pWindow == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiDockLayoutCloseOptionMenu(pLayout);
	__xgeXuiDockLayoutCloseOverflowMenu(pLayout);
	__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
	iRet = __xgeXuiDockLayoutRegisterWindow(pLayout, pWindow);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pSourcePane = pWindow->pPane;
	bFocusFallback = (pSourcePane != NULL) && (pSourcePane->arrWindows.Count > 1u);
	if ( (pSourcePane != NULL) && (pSourcePane->tRect.fW > 0.0f) && (pSourcePane->tRect.fH > 0.0f) ) {
		pWindow->tLastDockRect = pSourcePane->tRect;
	}
	__xgeXuiDockWindowDetachFromPane(pWindow);
	__xgeXuiDockLayoutRemoveFloatingWindow(pLayout, pWindow);
	pWindow->iState = XGE_XUI_DOCK_WINDOW_HIDDEN;
	pWindow->bVisible = 0;
	pWindow->tAutoHideStripRect = __xgeXuiDockRectZero();
	__xgeXuiDockWindowAttachClientToBase(pWindow);
	xgeXuiWindowSetOpen(&pWindow->tWindow, 0);
	if ( bFocusFallback && (pSourcePane->pLayout == pLayout) && (pSourcePane->arrWindows.Count > 0u) ) {
		__xgeXuiDockLayoutRequestFocusWindow(pLayout, xgeXuiDockPaneGetActiveWindow(pSourcePane));
	} else if ( pLayout->pPendingFocusWindow == pWindow ) {
		pLayout->pPendingFocusWindow = NULL;
	}
	xgeXuiWidgetMarkLayout(pLayout->pWidget);
	xgeXuiWidgetMarkPaint(pLayout->pWidget);
	return XGE_OK;
}

int xgeXuiDockLayoutAutoHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow)
{
	xge_xui_dock_pane pSourcePane;
	int iRet;
	int iStripRegion;
	int bFocusFallback;

	if ( (pLayout == NULL) || (pWindow == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pWindow->pLayout != pLayout) || (pWindow->iState != XGE_XUI_DOCK_WINDOW_DOCKED) || (pWindow->bDockable == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iStripRegion = __xgeXuiDockWindowAutoHideStripRegion(pWindow);
	if ( iStripRegion < 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiDockLayoutCloseOptionMenu(pLayout);
	__xgeXuiDockLayoutCloseOverflowMenu(pLayout);
	__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
	iRet = __xgeXuiDockLayoutRegisterWindow(pLayout, pWindow);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	pSourcePane = pWindow->pPane;
	bFocusFallback = (pSourcePane != NULL) && (pSourcePane->arrWindows.Count > 1u);
	if ( (pSourcePane != NULL) && (pSourcePane->tRect.fW > 0.0f) && (pSourcePane->tRect.fH > 0.0f) ) {
		pWindow->tLastDockRect = pSourcePane->tRect;
	}
	__xgeXuiDockWindowDetachFromPane(pWindow);
	__xgeXuiDockLayoutRemoveFloatingWindow(pLayout, pWindow);
	pWindow->iState = XGE_XUI_DOCK_WINDOW_AUTO_HIDE;
	pWindow->bVisible = 0;
	pWindow->iAutoHideRegion = iStripRegion;
	pWindow->tAutoHideStripRect = __xgeXuiDockRectZero();
	__xgeXuiDockWindowAttachClientToBase(pWindow);
	xgeXuiWindowSetOpen(&pWindow->tWindow, 0);
	if ( bFocusFallback && (pSourcePane->pLayout == pLayout) && (pSourcePane->arrWindows.Count > 0u) ) {
		__xgeXuiDockLayoutRequestFocusWindow(pLayout, xgeXuiDockPaneGetActiveWindow(pSourcePane));
	} else if ( pLayout->pPendingFocusWindow == pWindow ) {
		pLayout->pPendingFocusWindow = NULL;
	}
	xgeXuiWidgetMarkLayout(pLayout->pWidget);
	xgeXuiWidgetMarkPaint(pLayout->pWidget);
	return XGE_OK;
}

int xgeXuiDockLayoutDockAutoHideWindow(xge_xui_dock_layout pLayout, xge_xui_dock_window pWindow)
{
	int iRegion;
	int iSide;

	if ( (pLayout == NULL) || (pWindow == NULL) || (pWindow->pLayout != pLayout) || (pWindow->iState != XGE_XUI_DOCK_WINDOW_AUTO_HIDE) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
	iRegion = __xgeXuiDockRegionValid(pWindow->iLastRegion) ? pWindow->iLastRegion : XGE_XUI_DOCK_REGION_DOCUMENT;
	iSide = __xgeXuiDockSideIsSplit(pWindow->iLastSide) ? pWindow->iLastSide : XGE_XUI_DOCK_SIDE_FILL;
	pWindow->tAutoHideStripRect = __xgeXuiDockRectZero();
	if ( xgeXuiDockLayoutDockWindow(pLayout, pWindow, iRegion, iSide, 0.0f) == NULL ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

void xgeXuiDockLayoutSetRegionPortion(xge_xui_dock_layout pLayout, int iRegion, float fPortion)
{
	if ( (pLayout == NULL) || !__xgeXuiDockRegionValid(iRegion) ) {
		return;
	}
	pLayout->arrRegions[iRegion].fPortion = __xgeXuiDockClampPortion(fPortion);
	xgeXuiWidgetMarkLayout(pLayout->pWidget);
	xgeXuiWidgetMarkPaint(pLayout->pWidget);
}

xvalue xgeXuiDockLayoutSaveState(const xge_xui_dock_layout pLayout)
{
	xvalue pState;
	xvalue pRegions;
	xvalue pWindows;
	xvalue pFloating;
	xge_xui_dock_window* ppWindow;
	uint32 i;

	if ( pLayout == NULL ) {
		return NULL;
	}
	pState = xvoCreateTable();
	if ( pState == NULL ) {
		return NULL;
	}
	if ( !xvoTableSetInt(pState, "version", 7, 1) ) {
		xvoUnref(pState);
		return NULL;
	}
	if ( !xvoTableSetArray(pState, "regions", 7) ) {
		xvoUnref(pState);
		return NULL;
	}
	pRegions = xvoTableGetValue(pState, "regions", 7);
	if ( pRegions == NULL ) {
		xvoUnref(pState);
		return NULL;
	}
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		if ( !__xgeXuiDockStateAppendRegion(pRegions, &pLayout->arrRegions[i]) ) {
			xvoUnref(pState);
			return NULL;
		}
	}
	if ( !xvoTableSetArray(pState, "windows", 7) ) {
		xvoUnref(pState);
		return NULL;
	}
	pWindows = xvoTableGetValue(pState, "windows", 7);
	if ( pWindows == NULL ) {
		xvoUnref(pState);
		return NULL;
	}
	for ( i = 0; i < pLayout->arrWindows.Count; i++ ) {
		ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline((xarray)&pLayout->arrWindows, i + 1u);
		if ( (ppWindow != NULL) && (*ppWindow != NULL) && !__xgeXuiDockStateAppendWindow(pWindows, *ppWindow) ) {
			xvoUnref(pState);
			return NULL;
		}
	}
	if ( !xvoTableSetArray(pState, "floating", 8) ) {
		xvoUnref(pState);
		return NULL;
	}
	pFloating = xvoTableGetValue(pState, "floating", 8);
	if ( pFloating == NULL ) {
		xvoUnref(pState);
		return NULL;
	}
	for ( i = 0; i < pLayout->arrFloatingWindows.Count; i++ ) {
		ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline((xarray)&pLayout->arrFloatingWindows, i + 1u);
		if ( (ppWindow != NULL) && (*ppWindow != NULL) && !__xgeXuiDockStateAppendFloating(pFloating, *ppWindow, (int)i) ) {
			xvoUnref(pState);
			return NULL;
		}
	}
	return pState;
}

int xgeXuiDockLayoutLoadState(xge_xui_dock_layout pLayout, xvalue pState)
{
	xge_xui_dock_load_window_t* pInfos;
	xge_xui_dock_load_index_t tIndex;
	xge_xui_dock_node arrRoots[XGE_XUI_DOCK_REGION_COUNT];
	float arrPortions[XGE_XUI_DOCK_REGION_COUNT];
	int arrVisible[XGE_XUI_DOCK_REGION_COUNT];
	xarray_struct tFloating;
	xvalue pVersion;
	int iInfoCount;
	int bFloatingMoved;
	int iRet;
	int i;

	if ( (pLayout == NULL) || (xvoType(pState) != XVO_DT_TABLE) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pVersion = __xgeXuiDockLoadTableGet(pState, "version");
	if ( (xvoType(pVersion) != XVO_DT_INT) || (xvoGetInt(pVersion) != 1) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pInfos = NULL;
	iInfoCount = 0;
	bFloatingMoved = 0;
	memset(&tIndex, 0, sizeof(tIndex));
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		arrRoots[i] = NULL;
		arrPortions[i] = pLayout->arrRegions[i].fPortion;
		arrVisible[i] = pLayout->arrRegions[i].bVisible;
	}
	xrtArrayInit(&tFloating, sizeof(xge_xui_dock_window), XRT_OBJMODE_LOCAL);
	iRet = __xgeXuiDockLoadPrepareWindows(pLayout, pState, &pInfos, &iInfoCount);
	if ( iRet != XGE_OK ) {
		goto cleanup;
	}
	iRet = __xgeXuiDockLoadIndexInit(&tIndex, pInfos, iInfoCount);
	if ( iRet != XGE_OK ) {
		goto cleanup;
	}
	iRet = __xgeXuiDockLoadBuildRegions(pLayout, pState, &tIndex, pInfos, iInfoCount, arrRoots, arrPortions, arrVisible);
	if ( iRet != XGE_OK ) {
		goto cleanup;
	}
	iRet = __xgeXuiDockLoadBuildFloating(pLayout, pState, &tIndex, pInfos, iInfoCount, &tFloating);
	if ( iRet != XGE_OK ) {
		goto cleanup;
	}
	iRet = __xgeXuiDockLoadValidateWindowUsage(pInfos, iInfoCount);
	if ( iRet != XGE_OK ) {
		goto cleanup;
	}
	__xgeXuiDockLayoutCloseAutoHideExpand(pLayout);
	__xgeXuiDockLoadCommit(pLayout, arrRoots, arrPortions, arrVisible, &tFloating, pInfos, iInfoCount);
	bFloatingMoved = 1;

cleanup:
	for ( i = 0; i < XGE_XUI_DOCK_REGION_COUNT; i++ ) {
		__xgeXuiDockNodeDestroy(arrRoots[i]);
		arrRoots[i] = NULL;
	}
	if ( !bFloatingMoved ) {
		xrtArrayUnit(&tFloating);
	}
	__xgeXuiDockLoadIndexUnit(&tIndex);
	xrtFree(pInfos);
	return iRet;
}

void xgeXuiDockLayoutStateFree(xvalue pState)
{
	if ( pState != NULL ) {
		xvoUnref(pState);
	}
}

int xgeXuiDockLayoutStateGetCounts(xvalue pState, int* pRegionCount, int* pWindowCount, int* pFloatingCount)
{
	xvalue pRegions;
	xvalue pWindows;
	xvalue pFloating;

	if ( xvoType(pState) != XVO_DT_TABLE ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pRegions = xvoTableGetValue(pState, "regions", 7);
	pWindows = xvoTableGetValue(pState, "windows", 7);
	pFloating = xvoTableGetValue(pState, "floating", 8);
	if ( pRegionCount != NULL ) {
		*pRegionCount = (xvoType(pRegions) == XVO_DT_ARRAY) ? (int)xvoArrayItemCount(pRegions) : -1;
	}
	if ( pWindowCount != NULL ) {
		*pWindowCount = (xvoType(pWindows) == XVO_DT_ARRAY) ? (int)xvoArrayItemCount(pWindows) : -1;
	}
	if ( pFloatingCount != NULL ) {
		*pFloatingCount = (xvoType(pFloating) == XVO_DT_ARRAY) ? (int)xvoArrayItemCount(pFloating) : -1;
	}
	return XGE_OK;
}

int xgeXuiDockWindowInit(xge_xui_dock_window pWindow, xge_xui_context pContext)
{
	xge_xui_widget pWidget;
	xge_rect_t tZero;

	if ( (pWindow == NULL) || (pContext == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pWindow, 0, sizeof(*pWindow));
	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(&tZero, 0, sizeof(tZero));
	pWindow->pContext = pContext;
	pWindow->pWindowWidget = pWidget;
	pWindow->sTitle = "";
	pWindow->iState = XGE_XUI_DOCK_WINDOW_HIDDEN;
	pWindow->iLastRegion = XGE_XUI_DOCK_REGION_DOCUMENT;
	pWindow->iLastSide = XGE_XUI_DOCK_SIDE_FILL;
	pWindow->iLastTabIndex = -1;
	pWindow->iAutoHideRegion = XGE_XUI_DOCK_REGION_LEFT;
	pWindow->bClosable = 1;
	pWindow->bDockable = 1;
	pWindow->bVisible = 0;
	pWindow->tLastFloatRect = (xge_rect_t){ 80.0f, 80.0f, 320.0f, 220.0f };
	pWindow->tAutoHideStripRect = __xgeXuiDockRectZero();
	if ( xgeXuiWindowInit(&pWindow->tWindow, pContext, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		memset(pWindow, 0, sizeof(*pWindow));
		return XGE_ERROR;
	}
	pWindow->pClientWidget = xgeXuiWindowGetClientWidget(&pWindow->tWindow);
	xgeXuiWindowSetShowCollapse(&pWindow->tWindow, 0);
	xgeXuiWindowSetShowMaximize(&pWindow->tWindow, 0);
	xgeXuiWindowSetTitle(&pWindow->tWindow, xgeXuiGetTheme(pContext)->pFont, "");
	xgeXuiWindowSetOpen(&pWindow->tWindow, 0);
	xgeXuiWidgetSetEvent(pWidget, __xgeXuiDockWindowEventProc, pWindow);
	pWindow->tIconSrc = tZero;
	return XGE_OK;
}

void xgeXuiDockWindowUnit(xge_xui_dock_window pWindow)
{
	xge_xui_widget pWidget;

	if ( pWindow == NULL ) {
		return;
	}
	if ( pWindow->pLayout != NULL ) {
		__xgeXuiDockLayoutUnregisterWindow(pWindow->pLayout, pWindow);
	}
	if ( pWindow->pContentWidget != NULL ) {
		xgeXuiWidgetRemove(pWindow->pContentWidget);
		pWindow->pContentWidget = NULL;
	}
	pWidget = pWindow->pWindowWidget;
	xgeXuiWindowUnit(&pWindow->tWindow);
	if ( pWidget != NULL ) {
		xgeXuiWidgetFree(pWidget);
	}
	memset(pWindow, 0, sizeof(*pWindow));
}

xge_xui_window xgeXuiDockWindowBaseWindow(xge_xui_dock_window pWindow)
{
	return (pWindow != NULL) ? &pWindow->tWindow : NULL;
}

xge_xui_widget xgeXuiDockWindowClientWidget(xge_xui_dock_window pWindow)
{
	return (pWindow != NULL) ? pWindow->pClientWidget : NULL;
}

void xgeXuiDockWindowSetClientWidget(xge_xui_dock_window pWindow, xge_xui_widget pClient)
{
	if ( pWindow == NULL ) {
		return;
	}
	if ( pWindow->pContentWidget == pClient ) {
		return;
	}
	if ( pWindow->pContentWidget != NULL ) {
		xgeXuiWidgetRemove(pWindow->pContentWidget);
	}
	pWindow->pContentWidget = pClient;
	if ( (pClient != NULL) && (pWindow->pClientWidget != NULL) ) {
		xgeXuiWidgetSetRect(pClient, (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f });
		xgeXuiWidgetSetAnchorPx(pClient, XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_TOP | XGE_XUI_ANCHOR_RIGHT | XGE_XUI_ANCHOR_BOTTOM, 0.0f, 0.0f, 0.0f, 0.0f);
		xgeXuiWidgetAddInternal(pWindow->pClientWidget, pClient);
	}
	xgeXuiWidgetMarkLayout(pWindow->pClientWidget);
	xgeXuiWidgetMarkPaint(pWindow->pClientWidget);
}

void xgeXuiDockWindowSetTitle(xge_xui_dock_window pWindow, const char* sTitle)
{
	xui_font pFont;

	if ( pWindow == NULL ) {
		return;
	}
	pWindow->sTitle = (sTitle != NULL) ? sTitle : "";
	pFont = (pWindow->pContext != NULL) ? xgeXuiGetTheme(pWindow->pContext)->pFont : NULL;
	xgeXuiWindowSetTitle(&pWindow->tWindow, pFont, pWindow->sTitle);
	if ( (pWindow->pLayout != NULL) && (pWindow->pLayout->pWidget != NULL) ) {
		xgeXuiWidgetMarkLayout(pWindow->pLayout->pWidget);
		xgeXuiWidgetMarkPaint(pWindow->pLayout->pWidget);
	}
}

void xgeXuiDockWindowSetIcon(xge_xui_dock_window pWindow, xui_texture pTexture, xge_rect_t tSrc)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->pIconTexture = pTexture;
	pWindow->tIconSrc = tSrc;
	xgeXuiWindowSetIcon(&pWindow->tWindow, pTexture, tSrc);
	if ( (pWindow->pLayout != NULL) && (pWindow->pLayout->pWidget != NULL) ) {
		xgeXuiWidgetMarkPaint(pWindow->pLayout->pWidget);
	}
}

void xgeXuiDockWindowSetClosable(xge_xui_dock_window pWindow, int bClosable)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->bClosable = bClosable ? 1 : 0;
	xgeXuiWindowSetShowClose(&pWindow->tWindow, pWindow->bClosable);
	if ( (pWindow->pLayout != NULL) && (pWindow->pLayout->pWidget != NULL) ) {
		xgeXuiWidgetMarkPaint(pWindow->pLayout->pWidget);
	}
}

void xgeXuiDockWindowSetDockable(xge_xui_dock_window pWindow, int bDockable)
{
	if ( pWindow == NULL ) {
		return;
	}
	pWindow->bDockable = bDockable ? 1 : 0;
}

int xgeXuiDockWindowGetState(const xge_xui_dock_window pWindow)
{
	return (pWindow != NULL) ? pWindow->iState : XGE_XUI_DOCK_WINDOW_HIDDEN;
}

int xgeXuiDockPaneGetWindowCount(const xge_xui_dock_pane pPane)
{
	return (pPane != NULL) ? (int)pPane->arrWindows.Count : 0;
}

xge_xui_dock_window xgeXuiDockPaneGetWindow(const xge_xui_dock_pane pPane, int iIndex)
{
	xge_xui_dock_window* ppWindow;

	if ( (pPane == NULL) || (iIndex < 0) || (iIndex >= (int)pPane->arrWindows.Count) ) {
		return NULL;
	}
	ppWindow = (xge_xui_dock_window*)xrtArrayGet_Inline((xarray)&pPane->arrWindows, (uint32)iIndex + 1u);
	return (ppWindow != NULL) ? *ppWindow : NULL;
}

xge_xui_dock_window xgeXuiDockPaneGetActiveWindow(const xge_xui_dock_pane pPane)
{
	return (pPane != NULL) ? xgeXuiDockPaneGetWindow(pPane, pPane->iActive) : NULL;
}

void xgeXuiDockPaneSetActiveIndex(xge_xui_dock_pane pPane, int iIndex)
{
	if ( (pPane == NULL) || (iIndex < 0) || (iIndex >= (int)pPane->arrWindows.Count) ) {
		return;
	}
	pPane->iActive = iIndex;
	if ( (pPane->pLayout != NULL) && (pPane->pLayout->pWidget != NULL) ) {
		__xgeXuiDockLayoutRequestFocusWindow(pPane->pLayout, xgeXuiDockPaneGetActiveWindow(pPane));
		xgeXuiWidgetMarkLayout(pPane->pLayout->pWidget);
		xgeXuiWidgetMarkPaint(pPane->pLayout->pWidget);
	}
}

int xgeXuiDockPaneGetActiveIndex(const xge_xui_dock_pane pPane)
{
	return (pPane != NULL) ? pPane->iActive : -1;
}
