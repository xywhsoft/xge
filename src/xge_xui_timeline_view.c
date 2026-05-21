static float __xgeXuiTimelineClampFloat(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

static int __xgeXuiTimelineClampInt(int iValue, int iMin, int iMax)
{
	if ( iValue < iMin ) {
		return iMin;
	}
	if ( iValue > iMax ) {
		return iMax;
	}
	return iValue;
}

static int __xgeXuiTimelineRectIntersects(xge_rect_t tA, xge_rect_t tB)
{
	return (tA.fX < tB.fX + tB.fW) && (tA.fX + tA.fW > tB.fX) && (tA.fY < tB.fY + tB.fH) && (tA.fY + tA.fH > tB.fY);
}

static xge_rect_t __xgeXuiTimelineRectClip(xge_rect_t tRect, xge_rect_t tClip)
{
	float fX1;
	float fY1;
	float fX2;
	float fY2;

	fX1 = (tRect.fX > tClip.fX) ? tRect.fX : tClip.fX;
	fY1 = (tRect.fY > tClip.fY) ? tRect.fY : tClip.fY;
	fX2 = (tRect.fX + tRect.fW < tClip.fX + tClip.fW) ? tRect.fX + tRect.fW : tClip.fX + tClip.fW;
	fY2 = (tRect.fY + tRect.fH < tClip.fY + tClip.fH) ? tRect.fY + tRect.fH : tClip.fY + tClip.fH;
	if ( fX2 < fX1 ) {
		fX2 = fX1;
	}
	if ( fY2 < fY1 ) {
		fY2 = fY1;
	}
	return (xge_rect_t){ fX1, fY1, fX2 - fX1, fY2 - fY1 };
}

static char* __xgeXuiTimelineCopyText(const char* sText)
{
	char* sCopy;
	size_t iSize;

	if ( sText == NULL ) {
		sText = "";
	}
	iSize = strlen(sText) + 1u;
	sCopy = (char*)xrtMalloc(iSize);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iSize);
	return sCopy;
}

static int __xgeXuiTimelineFrameTypeNormalize(int iType)
{
	if ( (iType < XGE_XUI_TIMELINE_FRAME_EMPTY) || (iType > XGE_XUI_TIMELINE_FRAME_BLANK_KEY) ) {
		return XGE_XUI_TIMELINE_FRAME_EMPTY;
	}
	return iType;
}

static int __xgeXuiTimelineSpanTypeNormalize(int iType)
{
	if ( (iType < XGE_XUI_TIMELINE_SPAN_CUSTOM) || (iType > XGE_XUI_TIMELINE_SPAN_HOLD) ) {
		return XGE_XUI_TIMELINE_SPAN_CUSTOM;
	}
	return iType;
}

static xge_xui_widget __xgeXuiTimelineViewportWidget(xge_xui_timeline_view pTimeline)
{
	return (pTimeline != NULL) ? xgeXuiScrollFrameGetViewportWidget(pTimeline->pFrame) : NULL;
}

static void __xgeXuiTimelineMenuSelect(xge_xui_widget pOwner, int iIndex, int iValue, void* pUser);
static void __xgeXuiTimelineRenameSubmit(xge_xui_widget pWidget, const char* sText, void* pUser);
static int __xgeXuiTimelineTooltipResolve(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_tooltip_desc pDesc, void* pUser);

static void __xgeXuiTimelineInvalidate(xge_xui_timeline_view pTimeline)
{
	if ( pTimeline == NULL ) {
		return;
	}
	xgeXuiWidgetMarkPaint(pTimeline->pWidget);
	xgeXuiWidgetMarkPaint(__xgeXuiTimelineViewportWidget(pTimeline));
}

static xge_xui_timeline_layer_t* __xgeXuiTimelineLayerAt(xge_xui_timeline_view pTimeline, int iLayer)
{
	if ( (pTimeline == NULL) || (iLayer < 0) || ((uint32)iLayer >= pTimeline->arrLayers.Count) ) {
		return NULL;
	}
	return (xge_xui_timeline_layer_t*)xrtPtrArrayGet_Inline(&pTimeline->arrLayers, (uint32)iLayer + 1u);
}

static int __xgeXuiTimelineLayerIndexById(xge_xui_timeline_view pTimeline, int iId)
{
	xge_xui_timeline_layer_t* pLayer;
	uint32 i;

	if ( pTimeline == NULL ) {
		return -1;
	}
	for ( i = 0; i < pTimeline->arrLayers.Count; i++ ) {
		pLayer = __xgeXuiTimelineLayerAt(pTimeline, (int)i);
		if ( (pLayer != NULL) && (pLayer->iId == iId) ) {
			return (int)i;
		}
	}
	return -1;
}

static float __xgeXuiTimelineLayerHeight(xge_xui_timeline_view pTimeline, int iLayer)
{
	xge_xui_timeline_layer_t* pLayer;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( (pLayer != NULL) && (pLayer->fHeight > 0.0f) ) {
		return pLayer->fHeight;
	}
	return (pTimeline != NULL && pTimeline->fRowHeight > 0.0f) ? pTimeline->fRowHeight : 22.0f;
}

static float __xgeXuiTimelineLayerTop(xge_xui_timeline_view pTimeline, int iLayer)
{
	float fY;
	int i;

	fY = 0.0f;
	if ( pTimeline == NULL ) {
		return fY;
	}
	for ( i = 0; i < iLayer && i < (int)pTimeline->arrLayers.Count; i++ ) {
		fY += __xgeXuiTimelineLayerHeight(pTimeline, i);
	}
	return fY;
}

static float __xgeXuiTimelineContentHeight(xge_xui_timeline_view pTimeline)
{
	float fHeight;
	uint32 i;

	fHeight = 0.0f;
	if ( pTimeline == NULL ) {
		return fHeight;
	}
	for ( i = 0; i < pTimeline->arrLayers.Count; i++ ) {
		fHeight += __xgeXuiTimelineLayerHeight(pTimeline, (int)i);
	}
	return fHeight;
}

static void __xgeXuiTimelineSyncFrame(xge_xui_timeline_view pTimeline)
{
	float fContentW;
	float fContentH;

	if ( pTimeline == NULL ) {
		return;
	}
	fContentW = pTimeline->fLayerHeaderWidth + (float)pTimeline->iFrameCount * pTimeline->fFrameWidth;
	fContentH = pTimeline->fRulerHeight + __xgeXuiTimelineContentHeight(pTimeline);
	xgeXuiScrollFrameSetContentSize(pTimeline->pFrame, fContentW, fContentH);
	xgeXuiScrollFrameSetWheelStep(pTimeline->pFrame, ((pTimeline->fRowHeight > 0.0f) ? pTimeline->fRowHeight : 22.0f) * 3.0f);
	__xgeXuiTimelineInvalidate(pTimeline);
}

static void __xgeXuiTimelineRects(xge_xui_timeline_view pTimeline, xge_rect_t* pViewport, xge_rect_t* pCorner, xge_rect_t* pRuler, xge_rect_t* pLayerList, xge_rect_t* pGrid)
{
	xge_rect_t tViewport;
	xge_rect_t tCorner;
	xge_rect_t tRuler;
	xge_rect_t tLayer;
	xge_rect_t tGrid;
	float fHeaderW;
	float fRulerH;

	memset(&tViewport, 0, sizeof(tViewport));
	if ( pTimeline != NULL ) {
		tViewport = xgeXuiScrollFrameGetViewportRect(pTimeline->pFrame);
	}
	fHeaderW = (pTimeline != NULL) ? pTimeline->fLayerHeaderWidth : 0.0f;
	fRulerH = (pTimeline != NULL) ? pTimeline->fRulerHeight : 0.0f;
	if ( fHeaderW > tViewport.fW ) {
		fHeaderW = tViewport.fW;
	}
	if ( fRulerH > tViewport.fH ) {
		fRulerH = tViewport.fH;
	}
	tCorner = (xge_rect_t){ tViewport.fX, tViewport.fY, fHeaderW, fRulerH };
	tRuler = (xge_rect_t){ tViewport.fX + fHeaderW, tViewport.fY, tViewport.fW - fHeaderW, fRulerH };
	tLayer = (xge_rect_t){ tViewport.fX, tViewport.fY + fRulerH, fHeaderW, tViewport.fH - fRulerH };
	tGrid = (xge_rect_t){ tViewport.fX + fHeaderW, tViewport.fY + fRulerH, tViewport.fW - fHeaderW, tViewport.fH - fRulerH };
	if ( pViewport != NULL ) {
		*pViewport = tViewport;
	}
	if ( pCorner != NULL ) {
		*pCorner = tCorner;
	}
	if ( pRuler != NULL ) {
		*pRuler = tRuler;
	}
	if ( pLayerList != NULL ) {
		*pLayerList = tLayer;
	}
	if ( pGrid != NULL ) {
		*pGrid = tGrid;
	}
}

static int __xgeXuiTimelineLayerAtY(xge_xui_timeline_view pTimeline, float fY)
{
	xge_rect_t tLayer;
	float fContentY;
	float fTop;
	float fBottom;
	uint32 i;

	if ( pTimeline == NULL ) {
		return -1;
	}
	__xgeXuiTimelineRects(pTimeline, NULL, NULL, NULL, &tLayer, NULL);
	if ( __xgeXuiRectContains(tLayer, 0.0f, fY) == 0 && (fY < tLayer.fY || fY > tLayer.fY + tLayer.fH) ) {
		return -1;
	}
	fContentY = fY - tLayer.fY + pTimeline->tScroll.fScrollY;
	fTop = 0.0f;
	for ( i = 0; i < pTimeline->arrLayers.Count; i++ ) {
		fBottom = fTop + __xgeXuiTimelineLayerHeight(pTimeline, (int)i);
		if ( (fContentY >= fTop) && (fContentY < fBottom) ) {
			return (int)i;
		}
		fTop = fBottom;
	}
	return -1;
}

static int __xgeXuiTimelineFrameAtX(xge_xui_timeline_view pTimeline, float fX)
{
	xge_rect_t tGrid;
	int iFrame;

	if ( (pTimeline == NULL) || (pTimeline->fFrameWidth <= 0.0f) ) {
		return -1;
	}
	__xgeXuiTimelineRects(pTimeline, NULL, NULL, NULL, NULL, &tGrid);
	if ( (fX < tGrid.fX) || (fX >= tGrid.fX + tGrid.fW) ) {
		return -1;
	}
	iFrame = (int)((fX - tGrid.fX + pTimeline->tScroll.fScrollX) / pTimeline->fFrameWidth);
	if ( (iFrame < 0) || (iFrame >= pTimeline->iFrameCount) ) {
		return -1;
	}
	return iFrame;
}

static int __xgeXuiTimelineHitFrame(xge_xui_timeline_view pTimeline, float fX, float fY, int* pLayer, int* pFrame)
{
	xge_rect_t tGrid;
	int iLayer;
	int iFrame;

	if ( pLayer != NULL ) {
		*pLayer = -1;
	}
	if ( pFrame != NULL ) {
		*pFrame = -1;
	}
	if ( pTimeline == NULL ) {
		return 0;
	}
	__xgeXuiTimelineRects(pTimeline, NULL, NULL, NULL, NULL, &tGrid);
	if ( __xgeXuiRectContains(tGrid, fX, fY) == 0 ) {
		return 0;
	}
	iLayer = __xgeXuiTimelineLayerAtY(pTimeline, fY);
	iFrame = __xgeXuiTimelineFrameAtX(pTimeline, fX);
	if ( (iLayer < 0) || (iFrame < 0) ) {
		return 0;
	}
	if ( pLayer != NULL ) {
		*pLayer = iLayer;
	}
	if ( pFrame != NULL ) {
		*pFrame = iFrame;
	}
	return 1;
}

static int64 __xgeXuiTimelineSelectionKey(int iLayer, int iFrame)
{
	return (((int64)iLayer) << 32) | ((uint32)iFrame);
}

static void __xgeXuiTimelineNotifySelection(xge_xui_timeline_view pTimeline)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->iSelectionChangeCount++;
	if ( pTimeline->procSelection != NULL ) {
		pTimeline->procSelection(pTimeline->pWidget, (int)xrtListCount(&pTimeline->tSelection), pTimeline->pSelectionUser);
	}
	__xgeXuiTimelineInvalidate(pTimeline);
}

static void __xgeXuiTimelineSelectionClearRaw(xge_xui_timeline_view pTimeline)
{
	if ( pTimeline == NULL ) {
		return;
	}
	xrtListUnit(&pTimeline->tSelection);
	xrtListInit(&pTimeline->tSelection, sizeof(xge_xui_timeline_selection_t), XRT_OBJMODE_LOCAL);
}

static int __xgeXuiTimelineSelectFrameRaw(xge_xui_timeline_view pTimeline, int iLayer, int iFrame, int bSelected)
{
	xge_xui_timeline_selection_t* pSelection;
	int64 iKey;
	bool bNew;

	if ( (pTimeline == NULL) || (iLayer < 0) || ((uint32)iLayer >= pTimeline->arrLayers.Count) || (iFrame < 0) || (iFrame >= pTimeline->iFrameCount) ) {
		return 0;
	}
	iKey = __xgeXuiTimelineSelectionKey(iLayer, iFrame);
	if ( bSelected == 0 ) {
		return xrtListRemove(&pTimeline->tSelection, iKey) ? 1 : 0;
	}
	pSelection = (xge_xui_timeline_selection_t*)xrtListSet(&pTimeline->tSelection, iKey, &bNew);
	if ( pSelection == NULL ) {
		return 0;
	}
	pSelection->iLayer = iLayer;
	pSelection->iFrame = iFrame;
	return bNew ? 1 : 0;
}

static void __xgeXuiTimelineSelectRangeRaw(xge_xui_timeline_view pTimeline, int iLayerA, int iFrameA, int iLayerB, int iFrameB)
{
	int iLayerMin;
	int iLayerMax;
	int iFrameMin;
	int iFrameMax;
	int iLayer;
	int iFrame;

	if ( pTimeline == NULL ) {
		return;
	}
	iLayerMin = (iLayerA < iLayerB) ? iLayerA : iLayerB;
	iLayerMax = (iLayerA > iLayerB) ? iLayerA : iLayerB;
	iFrameMin = (iFrameA < iFrameB) ? iFrameA : iFrameB;
	iFrameMax = (iFrameA > iFrameB) ? iFrameA : iFrameB;
	iLayerMin = __xgeXuiTimelineClampInt(iLayerMin, 0, (int)pTimeline->arrLayers.Count - 1);
	iLayerMax = __xgeXuiTimelineClampInt(iLayerMax, 0, (int)pTimeline->arrLayers.Count - 1);
	iFrameMin = __xgeXuiTimelineClampInt(iFrameMin, 0, pTimeline->iFrameCount - 1);
	iFrameMax = __xgeXuiTimelineClampInt(iFrameMax, 0, pTimeline->iFrameCount - 1);
	for ( iLayer = iLayerMin; iLayer <= iLayerMax; iLayer++ ) {
		for ( iFrame = iFrameMin; iFrame <= iFrameMax; iFrame++ ) {
			__xgeXuiTimelineSelectFrameRaw(pTimeline, iLayer, iFrame, 1);
		}
	}
}

typedef struct xge_xui_timeline_selection_range_ctx_t {
	int iLayer;
	int iMinFrame;
	int iMaxFrame;
	int iCount;
} xge_xui_timeline_selection_range_ctx_t;

static bool __xgeXuiTimelineSelectionRangeProc(int64 iKey, ptr pVal, ptr pArg)
{
	xge_xui_timeline_selection_range_ctx_t* pCtx;
	xge_xui_timeline_selection_t* pSelection;

	(void)iKey;
	pCtx = (xge_xui_timeline_selection_range_ctx_t*)pArg;
	pSelection = (xge_xui_timeline_selection_t*)pVal;
	if ( (pCtx == NULL) || (pSelection == NULL) || (pSelection->iLayer != pCtx->iLayer) ) {
		return FALSE;
	}
	if ( (pCtx->iCount == 0) || (pSelection->iFrame < pCtx->iMinFrame) ) {
		pCtx->iMinFrame = pSelection->iFrame;
	}
	if ( (pCtx->iCount == 0) || (pSelection->iFrame > pCtx->iMaxFrame) ) {
		pCtx->iMaxFrame = pSelection->iFrame;
	}
	pCtx->iCount++;
	return FALSE;
}

static int __xgeXuiTimelineSelectionRangeForLayer(xge_xui_timeline_view pTimeline, int iLayer, int* pStartFrame, int* pEndFrame)
{
	xge_xui_timeline_selection_range_ctx_t tCtx;
	int iFrame;

	if ( (pTimeline == NULL) || (iLayer < 0) || ((uint32)iLayer >= pTimeline->arrLayers.Count) ) {
		return 0;
	}
	memset(&tCtx, 0, sizeof(tCtx));
	tCtx.iLayer = iLayer;
	xrtListWalk(&pTimeline->tSelection, __xgeXuiTimelineSelectionRangeProc, &tCtx);
	if ( (tCtx.iCount <= 0) || (tCtx.iMinFrame < 0) || (tCtx.iMaxFrame < tCtx.iMinFrame) ) {
		return 0;
	}
	if ( tCtx.iCount != (tCtx.iMaxFrame - tCtx.iMinFrame + 1) ) {
		return 0;
	}
	for ( iFrame = tCtx.iMinFrame; iFrame <= tCtx.iMaxFrame; iFrame++ ) {
		if ( xrtListExists(&pTimeline->tSelection, __xgeXuiTimelineSelectionKey(iLayer, iFrame)) == 0 ) {
			return 0;
		}
	}
	if ( pStartFrame != NULL ) {
		*pStartFrame = tCtx.iMinFrame;
	}
	if ( pEndFrame != NULL ) {
		*pEndFrame = tCtx.iMaxFrame;
	}
	return 1;
}

static int __xgeXuiTimelineZoomAt(xge_xui_timeline_view pTimeline, float fX, float fY, float fWheel)
{
	xge_rect_t tRuler;
	xge_rect_t tGrid;
	float fOldWidth;
	float fNewWidth;
	float fFactor;
	float fLocalX;
	float fContentX;
	float fScrollX;

	if ( (pTimeline == NULL) || (pTimeline->pFrame == NULL) || (pTimeline->fFrameWidth <= 0.0f) || (fWheel == 0.0f) ) {
		return 0;
	}
	__xgeXuiTimelineRects(pTimeline, NULL, NULL, &tRuler, NULL, &tGrid);
	if ( (__xgeXuiRectContains(tRuler, fX, fY) == 0) && (__xgeXuiRectContains(tGrid, fX, fY) == 0) ) {
		return 0;
	}
	fOldWidth = pTimeline->fFrameWidth;
	fFactor = (fWheel > 0.0f) ? 1.12f : (1.0f / 1.12f);
	fNewWidth = __xgeXuiTimelineClampFloat(fOldWidth * fFactor, pTimeline->fMinFrameWidth, pTimeline->fMaxFrameWidth);
	if ( fabsf(fNewWidth - fOldWidth) < 0.001f ) {
		return 1;
	}
	fLocalX = __xgeXuiTimelineClampFloat(fX - tGrid.fX, 0.0f, tGrid.fW);
	fContentX = pTimeline->tScroll.fScrollX + fLocalX;
	xgeXuiTimelineViewSetFrameWidth(pTimeline, fNewWidth);
	fScrollX = (fContentX / fOldWidth) * pTimeline->fFrameWidth - fLocalX;
	xgeXuiScrollFrameSetOffset(pTimeline->pFrame, fScrollX, pTimeline->tScroll.fScrollY);
	__xgeXuiTimelineInvalidate(pTimeline);
	return 1;
}

static int __xgeXuiTimelineLayerNameSetRaw(xge_xui_timeline_layer_t* pLayer, const char* sName)
{
	char* sCopy;

	if ( pLayer == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sCopy = __xgeXuiTimelineCopyText(sName);
	if ( sCopy == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( pLayer->sName != NULL ) {
		xrtFree(pLayer->sName);
	}
	pLayer->sName = sCopy;
	return XGE_OK;
}

static bool __xgeXuiTimelineSpanFreeProc(int64 iKey, ptr pVal, ptr pArg)
{
	xge_xui_timeline_span_t* pSpan;

	(void)iKey;
	(void)pArg;
	pSpan = (xge_xui_timeline_span_t*)pVal;
	if ( pSpan != NULL ) {
		if ( pSpan->sCustomType != NULL ) {
			xrtFree(pSpan->sCustomType);
			pSpan->sCustomType = NULL;
		}
		if ( pSpan->sLabel != NULL ) {
			xrtFree(pSpan->sLabel);
			pSpan->sLabel = NULL;
		}
	}
	return FALSE;
}

static void __xgeXuiTimelineLayerFree(xge_xui_timeline_layer_t* pLayer)
{
	if ( pLayer == NULL ) {
		return;
	}
	if ( pLayer->sName != NULL ) {
		xrtFree(pLayer->sName);
	}
	xrtListWalk(&pLayer->tSpans, __xgeXuiTimelineSpanFreeProc, NULL);
	xrtListUnit(&pLayer->tSpans);
	xrtListUnit(&pLayer->tFrames);
	xrtFree(pLayer);
}

static void __xgeXuiTimelineUnitInteraction(xge_xui_timeline_view pTimeline)
{
	if ( pTimeline == NULL ) {
		return;
	}
	if ( pTimeline->pLayerMenu != NULL ) {
		if ( pTimeline->pLayerMenu->pContext != NULL ) {
			xgeXuiMenuUnit(pTimeline->pLayerMenu);
		}
		xrtFree(pTimeline->pLayerMenu);
		pTimeline->pLayerMenu = NULL;
	}
	if ( pTimeline->pFrameMenu != NULL ) {
		if ( pTimeline->pFrameMenu->pContext != NULL ) {
			xgeXuiMenuUnit(pTimeline->pFrameMenu);
		}
		xrtFree(pTimeline->pFrameMenu);
		pTimeline->pFrameMenu = NULL;
	}
	if ( pTimeline->pRenameBox != NULL ) {
		if ( pTimeline->pRenameBox->pContext != NULL ) {
			xgeXuiInputBoxUnit(pTimeline->pRenameBox);
		}
		xrtFree(pTimeline->pRenameBox);
		pTimeline->pRenameBox = NULL;
	}
	if ( pTimeline->pRenameWidget != NULL ) {
		xgeXuiWidgetFree(pTimeline->pRenameWidget);
		pTimeline->pRenameWidget = NULL;
	}
}

static int __xgeXuiTimelineInitInteraction(xge_xui_timeline_view pTimeline)
{
	if ( pTimeline == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pTimeline->pLayerMenu = (xge_xui_menu)xrtMalloc(sizeof(xge_xui_menu_t));
	pTimeline->pFrameMenu = (xge_xui_menu)xrtMalloc(sizeof(xge_xui_menu_t));
	pTimeline->pRenameBox = (xge_xui_input_box)xrtMalloc(sizeof(xge_xui_input_box_t));
	pTimeline->pRenameWidget = xgeXuiWidgetCreate();
	if ( (pTimeline->pLayerMenu == NULL) || (pTimeline->pFrameMenu == NULL) || (pTimeline->pRenameBox == NULL) || (pTimeline->pRenameWidget == NULL) ) {
		__xgeXuiTimelineUnitInteraction(pTimeline);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pTimeline->pLayerMenu, 0, sizeof(xge_xui_menu_t));
	memset(pTimeline->pFrameMenu, 0, sizeof(xge_xui_menu_t));
	memset(pTimeline->pRenameBox, 0, sizeof(xge_xui_input_box_t));
	if ( xgeXuiMenuInit(pTimeline->pLayerMenu, pTimeline->pContext) != XGE_OK ||
	     xgeXuiMenuInit(pTimeline->pFrameMenu, pTimeline->pContext) != XGE_OK ||
	     xgeXuiInputBoxInit(pTimeline->pRenameBox, pTimeline->pContext, pTimeline->pRenameWidget, pTimeline->pFont) != XGE_OK ) {
		__xgeXuiTimelineUnitInteraction(pTimeline);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiMenuSetSelect(pTimeline->pLayerMenu, __xgeXuiTimelineMenuSelect, pTimeline);
	xgeXuiMenuSetSelect(pTimeline->pFrameMenu, __xgeXuiTimelineMenuSelect, pTimeline);
	xgeXuiInputBoxSetResult(pTimeline->pRenameBox, __xgeXuiTimelineRenameSubmit, pTimeline);
	xgeXuiInputBoxSetModal(pTimeline->pRenameBox, 1);
	return XGE_OK;
}

static int __xgeXuiTimelineForwardScrollBars(xge_xui_timeline_view pTimeline, const xge_event_t* pEvent)
{
	xge_xui_widget pHWidget;
	xge_xui_widget pVWidget;

	if ( (pTimeline == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	pHWidget = xgeXuiScrollFrameGetHScrollBarWidget(pTimeline->pFrame);
	pVWidget = xgeXuiScrollFrameGetVScrollBarWidget(pTimeline->pFrame);
	if ( (pHWidget != NULL) && __xgeXuiRectContains(pHWidget->tRect, pEvent->fX, pEvent->fY) ) {
		return xgeXuiScrollBarEvent(&pTimeline->pFrame->tHScrollBar, pEvent);
	}
	if ( (pVWidget != NULL) && __xgeXuiRectContains(pVWidget->tRect, pEvent->fX, pEvent->fY) ) {
		return xgeXuiScrollBarEvent(&pTimeline->pFrame->tVScrollBar, pEvent);
	}
	if ( (pHWidget != NULL) && (xgeXuiGetPointerCapture(pTimeline->pContext, pEvent->iPointerId) == pHWidget) ) {
		return xgeXuiScrollBarEvent(&pTimeline->pFrame->tHScrollBar, pEvent);
	}
	if ( (pVWidget != NULL) && (xgeXuiGetPointerCapture(pTimeline->pContext, pEvent->iPointerId) == pVWidget) ) {
		return xgeXuiScrollBarEvent(&pTimeline->pFrame->tVScrollBar, pEvent);
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static void __xgeXuiTimelineFrameChanged(xge_xui_scroll_frame pFrame, float fScrollX, float fScrollY, void* pUser)
{
	xge_xui_timeline_view pTimeline;

	(void)pFrame;
	(void)fScrollX;
	(void)fScrollY;
	pTimeline = (xge_xui_timeline_view)pUser;
	__xgeXuiTimelineInvalidate(pTimeline);
}

static int __xgeXuiTimelineFirstVisibleLayer(xge_xui_timeline_view pTimeline, float* pLayerTop)
{
	float fTop;
	float fBottom;
	uint32 i;

	if ( pLayerTop != NULL ) {
		*pLayerTop = 0.0f;
	}
	if ( pTimeline == NULL ) {
		return 0;
	}
	fTop = 0.0f;
	for ( i = 0; i < pTimeline->arrLayers.Count; i++ ) {
		fBottom = fTop + __xgeXuiTimelineLayerHeight(pTimeline, (int)i);
		if ( fBottom >= pTimeline->tScroll.fScrollY ) {
			if ( pLayerTop != NULL ) {
				*pLayerTop = fTop;
			}
			return (int)i;
		}
		fTop = fBottom;
	}
	if ( pLayerTop != NULL ) {
		*pLayerTop = fTop;
	}
	return (int)pTimeline->arrLayers.Count;
}

static int __xgeXuiTimelineFrameState(xge_xui_timeline_view pTimeline, int iLayer, int iFrame, const xge_xui_timeline_frame_t* pFrame)
{
	xge_xui_timeline_layer_t* pLayer;
	int iState;

	iState = 0;
	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( pLayer == NULL ) {
		return iState;
	}
	if ( xgeXuiTimelineViewIsFrameSelected(pTimeline, iLayer, iFrame) ) {
		iState |= XGE_XUI_TIMELINE_STATE_SELECTED;
	}
	if ( (pTimeline->iHoverLayer == iLayer) && (pTimeline->iHoverFrame == iFrame) ) {
		iState |= XGE_XUI_TIMELINE_STATE_HOVER;
	}
	if ( pLayer->bLocked ) {
		iState |= XGE_XUI_TIMELINE_STATE_LOCKED;
	}
	if ( pLayer->bVisible == 0 ) {
		iState |= XGE_XUI_TIMELINE_STATE_HIDDEN;
	}
	if ( pTimeline->iCurrentFrame == iFrame ) {
		iState |= XGE_XUI_TIMELINE_STATE_CURRENT;
	}
	if ( pFrame != NULL ) {
		if ( pFrame->iType == XGE_XUI_TIMELINE_FRAME_KEY ) {
			iState |= XGE_XUI_TIMELINE_STATE_KEY;
		} else if ( pFrame->iType == XGE_XUI_TIMELINE_FRAME_BLANK_KEY ) {
			iState |= XGE_XUI_TIMELINE_STATE_BLANK_KEY;
		}
	}
	return iState;
}

static xge_xui_timeline_frame_t* __xgeXuiTimelineFrameGet(xge_xui_timeline_layer_t* pLayer, int iFrame)
{
	if ( (pLayer == NULL) || (iFrame < 0) ) {
		return NULL;
	}
	return (xge_xui_timeline_frame_t*)xrtListGet(&pLayer->tFrames, (int64)iFrame);
}

static void __xgeXuiTimelineDrawKeyMarker(xge_rect_t tRect, int iType, uint32_t iColor)
{
	xge_rect_t tMark;
	float fSize;

	if ( iType == XGE_XUI_TIMELINE_FRAME_EMPTY ) {
		return;
	}
	fSize = (tRect.fW < tRect.fH ? tRect.fW : tRect.fH) - 8.0f;
	if ( fSize < 4.0f ) {
		fSize = 4.0f;
	}
	if ( fSize > 9.0f ) {
		fSize = 9.0f;
	}
	tMark = (xge_rect_t){ tRect.fX + (tRect.fW - fSize) * 0.5f, tRect.fY + (tRect.fH - fSize) * 0.5f, fSize, fSize };
	if ( iType == XGE_XUI_TIMELINE_FRAME_BLANK_KEY ) {
		__xgeXuiHostDrawBorderRect(tMark, 1.0f, iColor);
	} else if ( iType == XGE_XUI_TIMELINE_FRAME_KEY ) {
		__xgeXuiHostDrawRect(tMark, iColor);
	} else if ( iType == XGE_XUI_TIMELINE_FRAME_NORMAL ) {
		tMark.fY += tMark.fH * 0.5f - 1.0f;
		tMark.fH = 2.0f;
		__xgeXuiHostDrawRect(tMark, iColor);
	}
}

typedef struct xge_xui_timeline_span_paint_ctx_t {
	xge_xui_timeline_view pTimeline;
	xge_xui_timeline_layer_t* pLayer;
	int iLayer;
	xge_rect_t tGrid;
	float fLayerY;
	float fLayerH;
} xge_xui_timeline_span_paint_ctx_t;

static bool __xgeXuiTimelinePaintSpanProc(int64 iKey, ptr pVal, ptr pArg)
{
	xge_xui_timeline_span_paint_ctx_t* pCtx;
	xge_xui_timeline_span_t* pSpan;
	xge_rect_t tSpan;
	xge_rect_t tPaint;
	int iState;
	int iHandled;

	(void)iKey;
	pCtx = (xge_xui_timeline_span_paint_ctx_t*)pArg;
	pSpan = (xge_xui_timeline_span_t*)pVal;
	if ( (pCtx == NULL) || (pSpan == NULL) || (pCtx->pTimeline == NULL) ) {
		return FALSE;
	}
	if ( (pSpan->iEndFrame < 0) || (pSpan->iStartFrame >= pCtx->pTimeline->iFrameCount) ) {
		return FALSE;
	}
	tSpan.fX = pCtx->tGrid.fX + (float)pSpan->iStartFrame * pCtx->pTimeline->fFrameWidth - pCtx->pTimeline->tScroll.fScrollX + 2.0f;
	tSpan.fY = pCtx->fLayerY + 4.0f;
	tSpan.fW = (float)(pSpan->iEndFrame - pSpan->iStartFrame + 1) * pCtx->pTimeline->fFrameWidth - 4.0f;
	tSpan.fH = pCtx->fLayerH - 8.0f;
	if ( tSpan.fW < 2.0f ) {
		tSpan.fW = 2.0f;
	}
	if ( __xgeXuiTimelineRectIntersects(tSpan, pCtx->tGrid) == 0 ) {
		return FALSE;
	}
	tPaint = __xgeXuiTimelineRectClip(tSpan, pCtx->tGrid);
	iState = XGE_XUI_TIMELINE_STATE_SPAN;
	if ( pCtx->pLayer->bLocked ) {
		iState |= XGE_XUI_TIMELINE_STATE_LOCKED;
	}
	if ( pCtx->pLayer->bVisible == 0 ) {
		iState |= XGE_XUI_TIMELINE_STATE_HIDDEN;
	}
	iHandled = 0;
	if ( pCtx->pTimeline->procSpanRenderer != NULL ) {
		iHandled = pCtx->pTimeline->procSpanRenderer(pCtx->pTimeline->pWidget, pCtx->iLayer, pSpan, tPaint, iState, pCtx->pTimeline->pSpanRendererUser);
	}
	if ( iHandled == 0 ) {
		__xgeXuiHostDrawRect(tPaint, (pSpan->iColor != 0) ? pSpan->iColor : pCtx->pTimeline->iSpanColor);
		__xgeXuiHostDrawBorderRect(tPaint, 1.0f, XGE_COLOR_RGBA(78, 132, 186, 160));
		if ( (pSpan->sLabel != NULL) && (pSpan->sLabel[0] != 0) && (pCtx->pTimeline->pFont != NULL) && (tPaint.fW > 8.0f) ) {
			__xgeXuiHostDrawTextRect(pCtx->pTimeline->pFont, pSpan->sLabel, (xge_rect_t){ tPaint.fX + 4.0f, tPaint.fY, tPaint.fW - 8.0f, tPaint.fH }, pCtx->pTimeline->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	return FALSE;
}

static void __xgeXuiTimelineDrawSelectionOutline(xge_xui_timeline_view pTimeline, int iLayer, int iFirstFrame, int iLastFrame, xge_rect_t tGrid, float fY, float fH)
{
	xge_rect_t tRun;
	xge_rect_t tPaint;
	xge_rect_t tLine;
	float fX1;
	float fX2;
	int iFrame;
	int iStart;
	int iEnd;
	int bLeftEdge;
	int bRightEdge;
	uint32_t iColor;

	if ( (pTimeline == NULL) || (iLayer < 0) || (fH <= 0.0f) || (pTimeline->fFrameWidth <= 0.0f) ) {
		return;
	}
	iColor = XGE_COLOR_RGBA(36, 116, 204, 235);
	iFrame = iFirstFrame;
	while ( iFrame <= iLastFrame ) {
		if ( xrtListExists(&pTimeline->tSelection, __xgeXuiTimelineSelectionKey(iLayer, iFrame)) == 0 ) {
			iFrame++;
			continue;
		}
		iStart = iFrame;
		while ( (iFrame + 1 <= iLastFrame) && xrtListExists(&pTimeline->tSelection, __xgeXuiTimelineSelectionKey(iLayer, iFrame + 1)) ) {
			iFrame++;
		}
		iEnd = iFrame;
		fX1 = tGrid.fX + (float)iStart * pTimeline->fFrameWidth - pTimeline->tScroll.fScrollX;
		fX2 = tGrid.fX + (float)(iEnd + 1) * pTimeline->fFrameWidth - pTimeline->tScroll.fScrollX;
		tRun = (xge_rect_t){ fX1, fY - 1.0f, fX2 - fX1, fH + 1.0f };
		if ( __xgeXuiTimelineRectIntersects(tRun, tGrid) ) {
			tPaint = __xgeXuiTimelineRectClip(tRun, tGrid);
			if ( (tPaint.fW > 0.0f) && (tPaint.fH > 0.0f) ) {
				tLine = (xge_rect_t){ tPaint.fX, tPaint.fY, tPaint.fW, 1.0f };
				__xgeXuiHostDrawRect(tLine, iColor);
				tLine = (xge_rect_t){ tPaint.fX, tPaint.fY + tPaint.fH - 1.0f, tPaint.fW, 1.0f };
				__xgeXuiHostDrawRect(tLine, iColor);
				bLeftEdge = (iStart <= 0) || (xrtListExists(&pTimeline->tSelection, __xgeXuiTimelineSelectionKey(iLayer, iStart - 1)) == 0);
				bRightEdge = (iEnd + 1 >= pTimeline->iFrameCount) || (xrtListExists(&pTimeline->tSelection, __xgeXuiTimelineSelectionKey(iLayer, iEnd + 1)) == 0);
				if ( bLeftEdge && (fX1 >= tGrid.fX) && (fX1 < tGrid.fX + tGrid.fW) ) {
					tLine = (xge_rect_t){ fX1, tPaint.fY, 1.0f, tPaint.fH };
					__xgeXuiHostDrawRect(tLine, iColor);
				}
				if ( bRightEdge && (fX2 > tGrid.fX) && (fX2 <= tGrid.fX + tGrid.fW) ) {
					tLine = (xge_rect_t){ fX2 - 1.0f, tPaint.fY, 1.0f, tPaint.fH };
					__xgeXuiHostDrawRect(tLine, iColor);
				}
			}
		}
		iFrame++;
	}
}

static void __xgeXuiTimelineViewportPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_timeline_view pTimeline;
	xge_xui_timeline_layer_t* pLayer;
	xge_xui_timeline_frame_t* pFrame;
	xge_xui_timeline_span_paint_ctx_t tSpanCtx;
	xge_rect_t tViewport;
	xge_rect_t tCorner;
	xge_rect_t tRuler;
	xge_rect_t tLayerList;
	xge_rect_t tGrid;
	xge_rect_t tGridRows;
	xge_rect_t tRect;
	xge_rect_t tPaint;
	xge_rect_t tLine;
	float fLayerTop;
	float fY;
	float fH;
	float fBottom;
	float fRowsBottom;
	float fX;
	int iLayer;
	int iFrame;
	int iFirstFrame;
	int iLastFrame;
	int iState;
	int iHandled;
	char sBuffer[64];
	uint32_t iRowColor;

	(void)pWidget;
	pTimeline = (xge_xui_timeline_view)pUser;
	if ( pTimeline == NULL ) {
		return;
	}
	__xgeXuiTimelineRects(pTimeline, &tViewport, &tCorner, &tRuler, &tLayerList, &tGrid);
	if ( (tViewport.fW <= 0.0f) || (tViewport.fH <= 0.0f) ) {
		return;
	}
	__xgeXuiHostDrawRect(tViewport, pTimeline->iBackgroundColor);
	__xgeXuiHostDrawRect(tCorner, pTimeline->iHeaderColor);
	__xgeXuiHostDrawRect(tRuler, pTimeline->iHeaderColor);
	__xgeXuiHostDrawRect(tLayerList, pTimeline->iLayerColor);
	__xgeXuiHostDrawRect(tGrid, pTimeline->iBackgroundColor);
	__xgeXuiHostDrawBorderRect(tViewport, 1.0f, pTimeline->iGridColor);
	tLine = (xge_rect_t){ tViewport.fX, tGrid.fY - 1.0f, tViewport.fW, 1.0f };
	__xgeXuiHostDrawRect(tLine, pTimeline->iGridColor);
	if ( pTimeline->fFrameWidth <= 0.0f ) {
		return;
	}
	fRowsBottom = tGrid.fY + __xgeXuiTimelineContentHeight(pTimeline) - pTimeline->tScroll.fScrollY;
	fRowsBottom = __xgeXuiTimelineClampFloat(fRowsBottom, tGrid.fY, tGrid.fY + tGrid.fH);
	tGridRows = (xge_rect_t){ tGrid.fX, tGrid.fY, tGrid.fW, fRowsBottom - tGrid.fY };
	iFirstFrame = (int)(pTimeline->tScroll.fScrollX / pTimeline->fFrameWidth) - 1;
	if ( iFirstFrame < 0 ) {
		iFirstFrame = 0;
	}
	iLastFrame = (int)((pTimeline->tScroll.fScrollX + tGrid.fW) / pTimeline->fFrameWidth) + 2;
	if ( iLastFrame >= pTimeline->iFrameCount ) {
		iLastFrame = pTimeline->iFrameCount - 1;
	}
	for ( iFrame = iFirstFrame; iFrame <= iLastFrame; iFrame++ ) {
		fX = tGrid.fX + (float)iFrame * pTimeline->fFrameWidth - pTimeline->tScroll.fScrollX;
		tRect = (xge_rect_t){ fX, tRuler.fY, pTimeline->fFrameWidth, tRuler.fH };
		if ( __xgeXuiTimelineRectIntersects(tRect, tRuler) ) {
			tPaint = __xgeXuiTimelineRectClip(tRect, tRuler);
			iState = (iFrame == pTimeline->iCurrentFrame) ? XGE_XUI_TIMELINE_STATE_CURRENT : 0;
			iHandled = 0;
			if ( pTimeline->procRulerRenderer != NULL ) {
				iHandled = pTimeline->procRulerRenderer(pTimeline->pWidget, iFrame, tPaint, iState, pTimeline->pRulerRendererUser);
			}
			if ( iHandled == 0 ) {
				if ( (iFrame % 5) == 0 ) {
					snprintf(sBuffer, sizeof(sBuffer), "%d", iFrame);
					if ( pTimeline->pFont != NULL ) {
						tPaint = __xgeXuiTimelineRectClip((xge_rect_t){ tRect.fX + 2.0f, tRect.fY, tRect.fW * 3.0f, tRect.fH - 2.0f }, tRuler);
						if ( tPaint.fW > 0.0f ) {
							__xgeXuiHostDrawTextRect(pTimeline->pFont, sBuffer, tPaint, pTimeline->iHeaderTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
						}
					}
				}
				if ( (fX >= tRuler.fX) && (fX < tRuler.fX + tRuler.fW) ) {
					tLine = (xge_rect_t){ fX, tRuler.fY + ((iFrame % 5) == 0 ? 4.0f : tRuler.fH - 8.0f), 1.0f, ((iFrame % 5) == 0 ? tRuler.fH - 4.0f : 8.0f) };
					__xgeXuiHostDrawRect(tLine, ((iFrame % 5) == 0) ? pTimeline->iMajorGridColor : pTimeline->iGridColor);
				}
			}
		}
		if ( (tGridRows.fH > 0.0f) && (fX >= tGrid.fX) && (fX < tGrid.fX + tGrid.fW) ) {
			tLine = (xge_rect_t){ fX, tGridRows.fY, 1.0f, tGridRows.fH };
			__xgeXuiHostDrawRect(tLine, ((iFrame % 5) == 0) ? pTimeline->iMajorGridColor : pTimeline->iGridColor);
		}
	}
	iLayer = __xgeXuiTimelineFirstVisibleLayer(pTimeline, &fLayerTop);
	fY = tGrid.fY + fLayerTop - pTimeline->tScroll.fScrollY;
	fBottom = tGrid.fY + tGrid.fH;
	while ( (iLayer < (int)pTimeline->arrLayers.Count) && (fY < fBottom) ) {
		pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
		fH = __xgeXuiTimelineLayerHeight(pTimeline, iLayer);
		if ( pLayer != NULL ) {
			iRowColor = ((iLayer % 2) == 0) ? pTimeline->iLayerColor : pTimeline->iLayerAltColor;
			tRect = (xge_rect_t){ tLayerList.fX, fY, tLayerList.fW, fH };
			if ( __xgeXuiTimelineRectIntersects(tRect, tLayerList) ) {
				tPaint = __xgeXuiTimelineRectClip(tRect, tLayerList);
				__xgeXuiHostDrawRect(tPaint, pLayer->bSelected ? pTimeline->iLayerSelectedColor : iRowColor);
				iState = 0;
				if ( pLayer->bSelected ) {
					iState |= XGE_XUI_TIMELINE_STATE_SELECTED;
				}
				if ( pLayer->bLocked ) {
					iState |= XGE_XUI_TIMELINE_STATE_LOCKED;
				}
				if ( pLayer->bVisible == 0 ) {
					iState |= XGE_XUI_TIMELINE_STATE_HIDDEN;
				}
				iHandled = 0;
				if ( pTimeline->procLayerRenderer != NULL ) {
					iHandled = pTimeline->procLayerRenderer(pTimeline->pWidget, iLayer, pLayer, tPaint, iState, pTimeline->pLayerRendererUser);
				}
				if ( iHandled == 0 ) {
					if ( pTimeline->pFont != NULL ) {
						__xgeXuiHostDrawTextRect(pTimeline->pFont, (pLayer->sName != NULL) ? pLayer->sName : "", (xge_rect_t){ tPaint.fX + 8.0f, tPaint.fY, tPaint.fW - 50.0f, tPaint.fH }, (pLayer->bVisible != 0) ? pTimeline->iTextColor : pTimeline->iMutedTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
						if ( pTimeline->bShowVisibilityFeature ) {
							__xgeXuiHostDrawTextRect(pTimeline->pFont, pLayer->bVisible ? "E" : "-", (xge_rect_t){ tLayerList.fX + tLayerList.fW - 42.0f, tPaint.fY, 18.0f, tPaint.fH }, pTimeline->iMutedTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
						}
						if ( pTimeline->bShowLockFeature ) {
							__xgeXuiHostDrawTextRect(pTimeline->pFont, pLayer->bLocked ? "L" : "-", (xge_rect_t){ tLayerList.fX + tLayerList.fW - 22.0f, tPaint.fY, 18.0f, tPaint.fH }, pTimeline->iMutedTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
						}
					}
				}
				tLine = (xge_rect_t){ tRect.fX, tRect.fY + tRect.fH - 1.0f, tRect.fW, 1.0f };
				if ( __xgeXuiTimelineRectIntersects(tLine, tLayerList) ) {
					__xgeXuiHostDrawRect(__xgeXuiTimelineRectClip(tLine, tLayerList), pTimeline->iGridColor);
				}
			}
			tRect = (xge_rect_t){ tGrid.fX, fY, tGrid.fW, fH };
			if ( __xgeXuiTimelineRectIntersects(tRect, tGrid) ) {
				tPaint = __xgeXuiTimelineRectClip(tRect, tGrid);
				__xgeXuiHostDrawRect(tPaint, (iLayer % 2) == 0 ? XGE_COLOR_RGBA(248, 251, 255, 255) : XGE_COLOR_RGBA(243, 247, 252, 255));
				if ( pLayer->bVisible == 0 ) {
					__xgeXuiHostDrawRect(tPaint, XGE_COLOR_RGBA(238, 241, 245, 120));
				}
				tSpanCtx.pTimeline = pTimeline;
				tSpanCtx.pLayer = pLayer;
				tSpanCtx.iLayer = iLayer;
				tSpanCtx.tGrid = tGrid;
				tSpanCtx.fLayerY = fY;
				tSpanCtx.fLayerH = fH;
				xrtListWalk(&pLayer->tSpans, __xgeXuiTimelinePaintSpanProc, &tSpanCtx);
				for ( iFrame = iFirstFrame; iFrame <= iLastFrame; iFrame++ ) {
					fX = tGrid.fX + (float)iFrame * pTimeline->fFrameWidth - pTimeline->tScroll.fScrollX;
					tRect = (xge_rect_t){ fX, fY, pTimeline->fFrameWidth, fH };
					if ( __xgeXuiTimelineRectIntersects(tRect, tGrid) == 0 ) {
						continue;
					}
					tPaint = __xgeXuiTimelineRectClip(tRect, tGrid);
					pFrame = __xgeXuiTimelineFrameGet(pLayer, iFrame);
					iState = __xgeXuiTimelineFrameState(pTimeline, iLayer, iFrame, pFrame);
					if ( (iState & XGE_XUI_TIMELINE_STATE_SELECTED) != 0 ) {
						__xgeXuiHostDrawRect(tPaint, pTimeline->iSelectionColor);
					} else if ( (iState & XGE_XUI_TIMELINE_STATE_HOVER) != 0 ) {
						__xgeXuiHostDrawRect(tPaint, XGE_COLOR_RGBA(218, 234, 248, 130));
					}
					iHandled = 0;
					if ( pTimeline->procFrameRenderer != NULL ) {
						iHandled = pTimeline->procFrameRenderer(pTimeline->pWidget, iLayer, iFrame, pFrame, tPaint, iState, pTimeline->pFrameRendererUser);
					}
					if ( iHandled == 0 && pFrame != NULL ) {
						__xgeXuiTimelineDrawKeyMarker(tPaint, pFrame->iType, pTimeline->iKeyFrameColor);
					}
				}
				tLine = (xge_rect_t){ tGrid.fX, fY + fH - 1.0f, tGrid.fW, 1.0f };
				if ( __xgeXuiTimelineRectIntersects(tLine, tGrid) ) {
					__xgeXuiHostDrawRect(__xgeXuiTimelineRectClip(tLine, tGrid), pTimeline->iGridColor);
				}
				__xgeXuiTimelineDrawSelectionOutline(pTimeline, iLayer, iFirstFrame, iLastFrame, tGrid, fY, fH);
			}
		}
		fLayerTop += fH;
		fY = tGrid.fY + fLayerTop - pTimeline->tScroll.fScrollY;
		iLayer++;
	}
	if ( pTimeline->bDraggingSelection ) {
		xge_rect_t tSel;
		float fX1;
		float fX2;
		float fY1;
		float fY2;
		fX1 = pTimeline->fDragStartX < pTimeline->fDragCurrentX ? pTimeline->fDragStartX : pTimeline->fDragCurrentX;
		fX2 = pTimeline->fDragStartX > pTimeline->fDragCurrentX ? pTimeline->fDragStartX : pTimeline->fDragCurrentX;
		fY1 = pTimeline->fDragStartY < pTimeline->fDragCurrentY ? pTimeline->fDragStartY : pTimeline->fDragCurrentY;
		fY2 = pTimeline->fDragStartY > pTimeline->fDragCurrentY ? pTimeline->fDragStartY : pTimeline->fDragCurrentY;
		tSel = (xge_rect_t){ fX1, fY1, fX2 - fX1, fY2 - fY1 };
		__xgeXuiHostDrawRect(tSel, XGE_COLOR_RGBA(46, 124, 214, 40));
		__xgeXuiHostDrawBorderRect(tSel, 1.0f, XGE_COLOR_RGBA(46, 124, 214, 180));
	}
	if ( (pTimeline->iCurrentFrame >= 0) && (pTimeline->iCurrentFrame < pTimeline->iFrameCount) ) {
		fX = tGrid.fX + (float)pTimeline->iCurrentFrame * pTimeline->fFrameWidth - pTimeline->tScroll.fScrollX;
		if ( (fX >= tGrid.fX) && (fX <= tGrid.fX + tGrid.fW) ) {
			tRect = (xge_rect_t){ fX - 1.0f, tRuler.fY, 3.0f, tRuler.fH };
			tPaint = __xgeXuiTimelineRectClip(tRect, tRuler);
			if ( tPaint.fW > 0.0f ) {
				__xgeXuiHostDrawRect(tPaint, pTimeline->iCurrentFrameColor);
			}
			if ( tGridRows.fH > 0.0f ) {
				tRect = (xge_rect_t){ fX, tGridRows.fY, 1.0f, tGridRows.fH };
				__xgeXuiHostDrawRect(tRect, pTimeline->iCurrentFrameColor);
			}
		}
	}
	tLine = (xge_rect_t){ tGrid.fX - 1.0f, tViewport.fY, 1.0f, tViewport.fH };
	__xgeXuiHostDrawRect(tLine, pTimeline->iGridColor);
}

int xgeXuiTimelineViewInit(xge_xui_timeline_view pTimeline, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pTimeline == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pTimeline, 0, sizeof(*pTimeline));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTimeline->pContext = pContext;
	pTimeline->pWidget = pWidget;
	pTheme = xgeXuiGetTheme(pContext);
	pTimeline->pFont = (pTheme != NULL) ? pTheme->pFont : NULL;
	pTimeline->iFrameCount = 120;
	pTimeline->iFrameRate = 24;
	pTimeline->iCurrentFrame = 0;
	pTimeline->iAnchorLayer = -1;
	pTimeline->iAnchorFrame = -1;
	pTimeline->iHoverLayer = -1;
	pTimeline->iHoverFrame = -1;
	pTimeline->iActiveLayer = -1;
	pTimeline->iActiveFrame = -1;
	pTimeline->iNextLayerId = 1;
	pTimeline->iNextSpanId = 1;
	pTimeline->bShowVisibilityFeature = 1;
	pTimeline->bShowLockFeature = 1;
	pTimeline->fLayerHeaderWidth = 160.0f;
	pTimeline->fFrameWidth = 12.0f;
	pTimeline->fMinFrameWidth = 6.0f;
	pTimeline->fMaxFrameWidth = 40.0f;
	pTimeline->fRowHeight = 22.0f;
	pTimeline->fRulerHeight = 24.0f;
	pTimeline->iBackgroundColor = XGE_COLOR_RGBA(248, 251, 255, 255);
	pTimeline->iHeaderColor = XGE_COLOR_RGBA(226, 239, 248, 255);
	pTimeline->iHeaderTextColor = XGE_COLOR_RGBA(72, 94, 118, 255);
	pTimeline->iLayerColor = XGE_COLOR_RGBA(244, 248, 252, 255);
	pTimeline->iLayerAltColor = XGE_COLOR_RGBA(238, 244, 250, 255);
	pTimeline->iLayerSelectedColor = XGE_COLOR_RGBA(198, 226, 246, 255);
	pTimeline->iGridColor = XGE_COLOR_RGBA(187, 207, 226, 255);
	pTimeline->iMajorGridColor = XGE_COLOR_RGBA(152, 178, 204, 255);
	pTimeline->iTextColor = XGE_COLOR_RGBA(41, 58, 78, 255);
	pTimeline->iMutedTextColor = XGE_COLOR_RGBA(110, 130, 150, 255);
	pTimeline->iSelectionColor = XGE_COLOR_RGBA(60, 142, 220, 130);
	pTimeline->iCurrentFrameColor = XGE_COLOR_RGBA(224, 55, 65, 230);
	pTimeline->iKeyFrameColor = XGE_COLOR_RGBA(42, 105, 174, 230);
	pTimeline->iSpanColor = XGE_COLOR_RGBA(143, 201, 160, 150);
	pTimeline->iBarColor = XGE_COLOR_RGBA(220, 236, 248, 255);
	pTimeline->iThumbColor = XGE_COLOR_RGBA(74, 142, 210, 255);
	xrtPtrArrayInit(&pTimeline->arrLayers, XRT_OBJMODE_LOCAL);
	xrtListInit(&pTimeline->tSelection, sizeof(xge_xui_timeline_selection_t), XRT_OBJMODE_LOCAL);
	xgeXuiScrollModelInit(&pTimeline->tScroll);
	pTimeline->pFrame = (xge_xui_scroll_frame_t*)xrtMalloc(sizeof(xge_xui_scroll_frame_t));
	if ( pTimeline->pFrame == NULL ) {
		xrtListUnit(&pTimeline->tSelection);
		xrtPtrArrayUnit(&pTimeline->arrLayers);
		memset(pTimeline, 0, sizeof(*pTimeline));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pTimeline->pFrame, 0, sizeof(*pTimeline->pFrame));
	if ( xgeXuiScrollFrameInit(pTimeline->pFrame, pContext, pWidget, &pTimeline->tScroll) != XGE_OK ) {
		xrtFree(pTimeline->pFrame);
		xrtListUnit(&pTimeline->tSelection);
		xrtPtrArrayUnit(&pTimeline->arrLayers);
		memset(pTimeline, 0, sizeof(*pTimeline));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( __xgeXuiTimelineInitInteraction(pTimeline) != XGE_OK ) {
		xgeXuiScrollFrameUnit(pTimeline->pFrame);
		xrtFree(pTimeline->pFrame);
		xrtListUnit(&pTimeline->tSelection);
		xrtPtrArrayUnit(&pTimeline->arrLayers);
		memset(pTimeline, 0, sizeof(*pTimeline));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetEvent(pWidget, xgeXuiTimelineViewEventProc, pTimeline);
	xgeXuiWidgetSetTooltipResolver(pWidget, __xgeXuiTimelineTooltipResolve, pTimeline);
	xgeXuiWidgetSetEventInterest(pWidget, XGE_XUI_EVENT_MASK_CONTEXT_MENU, 1);
	xgeXuiWidgetSetLayoutProc(pWidget, xgeXuiScrollFrameLayoutProc, pTimeline->pFrame);
	xgeXuiWidgetSetPaint(xgeXuiScrollFrameGetViewportWidget(pTimeline->pFrame), __xgeXuiTimelineViewportPaintProc, pTimeline);
	xgeXuiScrollFrameSetScrollbarPolicy(pTimeline->pFrame, XGE_XUI_SCROLLBAR_POLICY_AUTO, XGE_XUI_SCROLLBAR_POLICY_AUTO);
	xgeXuiScrollFrameSetScrollbarMode(pTimeline->pFrame, XGE_XUI_SCROLLBAR_MODE_COMPACT);
	xgeXuiScrollFrameSetContentDragEnabled(pTimeline->pFrame, 0);
	xgeXuiScrollFrameSetWheelAxis(pTimeline->pFrame, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollFrameSetChange(pTimeline->pFrame, __xgeXuiTimelineFrameChanged, pTimeline);
	xgeXuiScrollFrameSetColors(pTimeline->pFrame, pTimeline->iBarColor, pTimeline->iThumbColor, pTimeline->iThumbColor, pTimeline->iThumbColor, pTimeline->iThumbColor, pTimeline->iBarColor);
	xgeXuiScrollFrameSetButtonColors(pTimeline->pFrame, pTimeline->iBarColor, pTimeline->iThumbColor);
	xgeXuiScrollFrameSetCornerColors(pTimeline->pFrame, pTimeline->iBarColor, pTimeline->iThumbColor);
	__xgeXuiTimelineSyncFrame(pTimeline);
	return XGE_OK;
}

void xgeXuiTimelineViewUnit(xge_xui_timeline_view pTimeline)
{
	xge_xui_timeline_layer_t* pLayer;
	uint32 i;

	if ( pTimeline == NULL ) {
		return;
	}
	xgeXuiMenuClose(pTimeline->pLayerMenu);
	xgeXuiMenuClose(pTimeline->pFrameMenu);
	xgeXuiInputBoxSetOpen(pTimeline->pRenameBox, 0);
	for ( i = 0; i < pTimeline->arrLayers.Count; i++ ) {
		pLayer = __xgeXuiTimelineLayerAt(pTimeline, (int)i);
		__xgeXuiTimelineLayerFree(pLayer);
	}
	xrtPtrArrayUnit(&pTimeline->arrLayers);
	xrtListUnit(&pTimeline->tSelection);
	__xgeXuiTimelineUnitInteraction(pTimeline);
	xgeXuiScrollFrameUnit(pTimeline->pFrame);
	xrtFree(pTimeline->pFrame);
	if ( pTimeline->pWidget != NULL && pTimeline->pWidget->pUser == pTimeline ) {
		pTimeline->pWidget->pUser = NULL;
		pTimeline->pWidget->procEvent = NULL;
	}
	memset(pTimeline, 0, sizeof(*pTimeline));
}

void xgeXuiTimelineViewClear(xge_xui_timeline_view pTimeline)
{
	xge_xui_timeline_layer_t* pLayer;
	uint32 i;

	if ( pTimeline == NULL ) {
		return;
	}
	for ( i = 0; i < pTimeline->arrLayers.Count; i++ ) {
		pLayer = __xgeXuiTimelineLayerAt(pTimeline, (int)i);
		__xgeXuiTimelineLayerFree(pLayer);
	}
	xrtPtrArrayUnit(&pTimeline->arrLayers);
	xrtPtrArrayInit(&pTimeline->arrLayers, XRT_OBJMODE_LOCAL);
	__xgeXuiTimelineSelectionClearRaw(pTimeline);
	pTimeline->iAnchorLayer = -1;
	pTimeline->iAnchorFrame = -1;
	pTimeline->iHoverLayer = -1;
	pTimeline->iHoverFrame = -1;
	pTimeline->iActiveLayer = -1;
	pTimeline->iActiveFrame = -1;
	__xgeXuiTimelineSyncFrame(pTimeline);
}

void xgeXuiTimelineViewSetFont(xge_xui_timeline_view pTimeline, xge_font pFont)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->pFont = pFont;
	xgeXuiMenuSetFont(pTimeline->pLayerMenu, pFont);
	xgeXuiMenuSetFont(pTimeline->pFrameMenu, pFont);
	if ( pTimeline->pRenameBox != NULL ) {
		xgeXuiInputBoxSetText(pTimeline->pRenameBox, pFont, pTimeline->pRenameBox->sTitle, pTimeline->pRenameBox->sPrompt, xgeXuiInputGetText(&pTimeline->pRenameBox->tInput));
	}
	__xgeXuiTimelineInvalidate(pTimeline);
}

void xgeXuiTimelineViewSetFrameCount(xge_xui_timeline_view pTimeline, int iFrameCount)
{
	if ( pTimeline == NULL ) {
		return;
	}
	if ( iFrameCount < 1 ) {
		iFrameCount = 1;
	}
	pTimeline->iFrameCount = iFrameCount;
	if ( pTimeline->iCurrentFrame >= iFrameCount ) {
		pTimeline->iCurrentFrame = iFrameCount - 1;
	}
	__xgeXuiTimelineSyncFrame(pTimeline);
}

int xgeXuiTimelineViewGetFrameCount(xge_xui_timeline_view pTimeline)
{
	return (pTimeline != NULL) ? pTimeline->iFrameCount : 0;
}

void xgeXuiTimelineViewSetFrameRate(xge_xui_timeline_view pTimeline, int iFrameRate)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->iFrameRate = (iFrameRate > 0) ? iFrameRate : 24;
	__xgeXuiTimelineInvalidate(pTimeline);
}

int xgeXuiTimelineViewGetFrameRate(xge_xui_timeline_view pTimeline)
{
	return (pTimeline != NULL) ? pTimeline->iFrameRate : 0;
}

int xgeXuiTimelineViewSetCurrentFrame(xge_xui_timeline_view pTimeline, int iFrame)
{
	int iOld;

	if ( pTimeline == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iFrame = __xgeXuiTimelineClampInt(iFrame, 0, pTimeline->iFrameCount - 1);
	iOld = pTimeline->iCurrentFrame;
	if ( iOld == iFrame ) {
		return XGE_OK;
	}
	if ( pTimeline->procCurrentFrameChanging != NULL && pTimeline->procCurrentFrameChanging(pTimeline->pWidget, iOld, iFrame, pTimeline->pCurrentFrameUser) == 0 ) {
		return XGE_ERROR;
	}
	pTimeline->iCurrentFrame = iFrame;
	pTimeline->iCurrentFrameChangeCount++;
	if ( pTimeline->procCurrentFrameChanged != NULL ) {
		pTimeline->procCurrentFrameChanged(pTimeline->pWidget, iOld, iFrame, pTimeline->pCurrentFrameUser);
	}
	__xgeXuiTimelineInvalidate(pTimeline);
	return XGE_OK;
}

int xgeXuiTimelineViewGetCurrentFrame(xge_xui_timeline_view pTimeline)
{
	return (pTimeline != NULL) ? pTimeline->iCurrentFrame : 0;
}

void xgeXuiTimelineViewSetMetrics(xge_xui_timeline_view pTimeline, float fLayerHeaderWidth, float fFrameWidth, float fRowHeight, float fRulerHeight)
{
	if ( pTimeline == NULL ) {
		return;
	}
	if ( fLayerHeaderWidth > 0.0f ) {
		pTimeline->fLayerHeaderWidth = fLayerHeaderWidth;
	}
	if ( fRowHeight > 0.0f ) {
		pTimeline->fRowHeight = fRowHeight;
	}
	if ( fRulerHeight > 0.0f ) {
		pTimeline->fRulerHeight = fRulerHeight;
	}
	xgeXuiTimelineViewSetFrameWidth(pTimeline, fFrameWidth);
	__xgeXuiTimelineSyncFrame(pTimeline);
}

void xgeXuiTimelineViewSetFrameWidth(xge_xui_timeline_view pTimeline, float fFrameWidth)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->fFrameWidth = __xgeXuiTimelineClampFloat(fFrameWidth, pTimeline->fMinFrameWidth, pTimeline->fMaxFrameWidth);
	__xgeXuiTimelineSyncFrame(pTimeline);
}

void xgeXuiTimelineViewSetLayerHeaderWidth(xge_xui_timeline_view pTimeline, float fWidth)
{
	if ( (pTimeline == NULL) || (fWidth <= 0.0f) ) {
		return;
	}
	pTimeline->fLayerHeaderWidth = fWidth;
	__xgeXuiTimelineSyncFrame(pTimeline);
}

void xgeXuiTimelineViewSetRowHeight(xge_xui_timeline_view pTimeline, float fHeight)
{
	if ( (pTimeline == NULL) || (fHeight <= 0.0f) ) {
		return;
	}
	pTimeline->fRowHeight = fHeight;
	__xgeXuiTimelineSyncFrame(pTimeline);
}

void xgeXuiTimelineViewSetRulerHeight(xge_xui_timeline_view pTimeline, float fHeight)
{
	if ( (pTimeline == NULL) || (fHeight <= 0.0f) ) {
		return;
	}
	pTimeline->fRulerHeight = fHeight;
	__xgeXuiTimelineSyncFrame(pTimeline);
}

void xgeXuiTimelineViewSetFeatureFlags(xge_xui_timeline_view pTimeline, int bVisibility, int bLock)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->bShowVisibilityFeature = bVisibility ? 1 : 0;
	pTimeline->bShowLockFeature = bLock ? 1 : 0;
	__xgeXuiTimelineInvalidate(pTimeline);
}

void xgeXuiTimelineViewSetScrollbarMode(xge_xui_timeline_view pTimeline, int iMode)
{
	if ( pTimeline == NULL ) {
		return;
	}
	xgeXuiScrollFrameSetScrollbarMode(pTimeline->pFrame, iMode);
	__xgeXuiTimelineInvalidate(pTimeline);
}

int xgeXuiTimelineViewGetScrollbarMode(xge_xui_timeline_view pTimeline)
{
	return (pTimeline != NULL) ? xgeXuiScrollFrameGetScrollbarMode(pTimeline->pFrame) : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

int xgeXuiTimelineViewAddLayer(xge_xui_timeline_view pTimeline, int iId, const char* sName)
{
	xge_xui_timeline_layer_t* pLayer;
	uint32 iIndex;

	if ( pTimeline == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iId <= 0 ) {
		iId = pTimeline->iNextLayerId++;
	} else if ( iId >= pTimeline->iNextLayerId ) {
		pTimeline->iNextLayerId = iId + 1;
	}
	if ( __xgeXuiTimelineLayerIndexById(pTimeline, iId) >= 0 ) {
		return XGE_ERROR;
	}
	pLayer = (xge_xui_timeline_layer_t*)xrtMalloc(sizeof(*pLayer));
	if ( pLayer == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(pLayer, 0, sizeof(*pLayer));
	pLayer->iId = iId;
	pLayer->bVisible = 1;
	pLayer->fHeight = pTimeline->fRowHeight;
	pLayer->iColor = XGE_COLOR_RGBA(74, 142, 210, 255);
	xrtListInit(&pLayer->tFrames, sizeof(xge_xui_timeline_frame_t), XRT_OBJMODE_LOCAL);
	xrtListInit(&pLayer->tSpans, sizeof(xge_xui_timeline_span_t), XRT_OBJMODE_LOCAL);
	if ( __xgeXuiTimelineLayerNameSetRaw(pLayer, sName) != XGE_OK ) {
		__xgeXuiTimelineLayerFree(pLayer);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iIndex = xrtPtrArrayAppend(&pTimeline->arrLayers, pLayer);
	if ( iIndex == 0u ) {
		__xgeXuiTimelineLayerFree(pLayer);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	__xgeXuiTimelineSyncFrame(pTimeline);
	return (int)iIndex - 1;
}

int xgeXuiTimelineViewRemoveLayer(xge_xui_timeline_view pTimeline, int iLayer)
{
	xge_xui_timeline_layer_t* pLayer;

	if ( (pTimeline == NULL) || (iLayer < 0) || ((uint32)iLayer >= pTimeline->arrLayers.Count) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( !xrtPtrArrayRemove(&pTimeline->arrLayers, (uint32)iLayer + 1u, 1u) ) {
		return XGE_ERROR;
	}
	__xgeXuiTimelineLayerFree(pLayer);
	__xgeXuiTimelineSelectionClearRaw(pTimeline);
	__xgeXuiTimelineSyncFrame(pTimeline);
	return XGE_OK;
}

int xgeXuiTimelineViewMoveLayer(xge_xui_timeline_view pTimeline, int iFrom, int iTo)
{
	xge_xui_timeline_layer_t* pLayer;
	uint32 iInserted;

	if ( (pTimeline == NULL) || (iFrom < 0) || (iTo < 0) || ((uint32)iFrom >= pTimeline->arrLayers.Count) || ((uint32)iTo >= pTimeline->arrLayers.Count) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iFrom == iTo ) {
		return XGE_OK;
	}
	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iFrom);
	if ( !xrtPtrArrayRemove(&pTimeline->arrLayers, (uint32)iFrom + 1u, 1u) ) {
		return XGE_ERROR;
	}
	if ( iTo > iFrom ) {
		iTo--;
	}
	iInserted = xrtPtrArrayInsert(&pTimeline->arrLayers, (uint32)iTo, pLayer);
	if ( iInserted == 0u ) {
		xrtPtrArrayAppend(&pTimeline->arrLayers, pLayer);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	__xgeXuiTimelineSelectionClearRaw(pTimeline);
	__xgeXuiTimelineSyncFrame(pTimeline);
	return XGE_OK;
}

int xgeXuiTimelineViewGetLayerCount(xge_xui_timeline_view pTimeline)
{
	return (pTimeline != NULL) ? (int)pTimeline->arrLayers.Count : 0;
}

xge_xui_timeline_layer_t* xgeXuiTimelineViewGetLayer(xge_xui_timeline_view pTimeline, int iLayer)
{
	return __xgeXuiTimelineLayerAt(pTimeline, iLayer);
}

int xgeXuiTimelineViewSetLayerName(xge_xui_timeline_view pTimeline, int iLayer, const char* sName)
{
	xge_xui_timeline_layer_t* pLayer;
	const char* sOld;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( pLayer == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sOld = (pLayer->sName != NULL) ? pLayer->sName : "";
	if ( pTimeline->procLayerChanging != NULL && pTimeline->procLayerChanging(pTimeline->pWidget, iLayer, pLayer->iId, XGE_XUI_TIMELINE_LAYER_CHANGE_NAME, sOld, sName, 0, 0, pTimeline->pLayerUser) == 0 ) {
		return XGE_ERROR;
	}
	if ( __xgeXuiTimelineLayerNameSetRaw(pLayer, sName) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pTimeline->iLayerChangeCount++;
	if ( pTimeline->procLayerChanged != NULL ) {
		pTimeline->procLayerChanged(pTimeline->pWidget, iLayer, pLayer->iId, XGE_XUI_TIMELINE_LAYER_CHANGE_NAME, pTimeline->pLayerUser);
	}
	__xgeXuiTimelineInvalidate(pTimeline);
	return XGE_OK;
}

int xgeXuiTimelineViewSetLayerVisible(xge_xui_timeline_view pTimeline, int iLayer, int bVisible)
{
	xge_xui_timeline_layer_t* pLayer;
	int iOld;
	int iNew;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( pLayer == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iOld = pLayer->bVisible ? 1 : 0;
	iNew = bVisible ? 1 : 0;
	if ( iOld == iNew ) {
		return XGE_OK;
	}
	if ( pTimeline->procLayerChanging != NULL && pTimeline->procLayerChanging(pTimeline->pWidget, iLayer, pLayer->iId, XGE_XUI_TIMELINE_LAYER_CHANGE_VISIBLE, NULL, NULL, iOld, iNew, pTimeline->pLayerUser) == 0 ) {
		return XGE_ERROR;
	}
	pLayer->bVisible = iNew;
	pTimeline->iLayerChangeCount++;
	if ( pTimeline->procLayerChanged != NULL ) {
		pTimeline->procLayerChanged(pTimeline->pWidget, iLayer, pLayer->iId, XGE_XUI_TIMELINE_LAYER_CHANGE_VISIBLE, pTimeline->pLayerUser);
	}
	__xgeXuiTimelineInvalidate(pTimeline);
	return XGE_OK;
}

int xgeXuiTimelineViewSetLayerLocked(xge_xui_timeline_view pTimeline, int iLayer, int bLocked)
{
	xge_xui_timeline_layer_t* pLayer;
	int iOld;
	int iNew;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( pLayer == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iOld = pLayer->bLocked ? 1 : 0;
	iNew = bLocked ? 1 : 0;
	if ( iOld == iNew ) {
		return XGE_OK;
	}
	if ( pTimeline->procLayerChanging != NULL && pTimeline->procLayerChanging(pTimeline->pWidget, iLayer, pLayer->iId, XGE_XUI_TIMELINE_LAYER_CHANGE_LOCKED, NULL, NULL, iOld, iNew, pTimeline->pLayerUser) == 0 ) {
		return XGE_ERROR;
	}
	pLayer->bLocked = iNew;
	pTimeline->iLayerChangeCount++;
	if ( pTimeline->procLayerChanged != NULL ) {
		pTimeline->procLayerChanged(pTimeline->pWidget, iLayer, pLayer->iId, XGE_XUI_TIMELINE_LAYER_CHANGE_LOCKED, pTimeline->pLayerUser);
	}
	__xgeXuiTimelineInvalidate(pTimeline);
	return XGE_OK;
}

int xgeXuiTimelineViewSetLayerHeight(xge_xui_timeline_view pTimeline, int iLayer, float fHeight)
{
	xge_xui_timeline_layer_t* pLayer;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( (pLayer == NULL) || (fHeight <= 0.0f) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pLayer->fHeight = fHeight;
	__xgeXuiTimelineSyncFrame(pTimeline);
	return XGE_OK;
}

int xgeXuiTimelineViewSetLayerColor(xge_xui_timeline_view pTimeline, int iLayer, uint32_t iColor)
{
	xge_xui_timeline_layer_t* pLayer;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( pLayer == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pLayer->iColor = iColor;
	__xgeXuiTimelineInvalidate(pTimeline);
	return XGE_OK;
}

int xgeXuiTimelineViewSetFrameType(xge_xui_timeline_view pTimeline, int iLayer, int iFrame, int iType)
{
	xge_xui_timeline_layer_t* pLayer;
	xge_xui_timeline_frame_t* pFrame;
	int iOld;
	bool bNew;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( (pTimeline == NULL) || (pLayer == NULL) || (iFrame < 0) || (iFrame >= pTimeline->iFrameCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pLayer->bLocked ) {
		return XGE_ERROR;
	}
	iType = __xgeXuiTimelineFrameTypeNormalize(iType);
	iOld = xgeXuiTimelineViewGetFrameType(pTimeline, iLayer, iFrame);
	if ( iOld == iType ) {
		return XGE_OK;
	}
	if ( pTimeline->procFrameChanging != NULL && pTimeline->procFrameChanging(pTimeline->pWidget, iLayer, pLayer->iId, iFrame, iOld, iType, pTimeline->pFrameUser) == 0 ) {
		return XGE_ERROR;
	}
	if ( iType == XGE_XUI_TIMELINE_FRAME_EMPTY ) {
		xrtListRemove(&pLayer->tFrames, (int64)iFrame);
	} else {
		pFrame = (xge_xui_timeline_frame_t*)xrtListSet(&pLayer->tFrames, (int64)iFrame, &bNew);
		if ( pFrame == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pFrame->iFrame = iFrame;
		pFrame->iType = iType;
		if ( bNew ) {
			pFrame->pUser = NULL;
		}
	}
	pTimeline->iFrameChangeCount++;
	if ( pTimeline->procFrameChanged != NULL ) {
		pTimeline->procFrameChanged(pTimeline->pWidget, iLayer, pLayer->iId, iFrame, iOld, iType, pTimeline->pFrameUser);
	}
	__xgeXuiTimelineInvalidate(pTimeline);
	return XGE_OK;
}

int xgeXuiTimelineViewGetFrameType(xge_xui_timeline_view pTimeline, int iLayer, int iFrame)
{
	xge_xui_timeline_layer_t* pLayer;
	xge_xui_timeline_frame_t* pFrame;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	pFrame = __xgeXuiTimelineFrameGet(pLayer, iFrame);
	return (pFrame != NULL) ? pFrame->iType : XGE_XUI_TIMELINE_FRAME_EMPTY;
}

int xgeXuiTimelineViewClearFrame(xge_xui_timeline_view pTimeline, int iLayer, int iFrame)
{
	return xgeXuiTimelineViewSetFrameType(pTimeline, iLayer, iFrame, XGE_XUI_TIMELINE_FRAME_EMPTY);
}

typedef struct xge_xui_timeline_span_overlap_ctx_t {
	int iStart;
	int iEnd;
	int iIgnoreId;
	int bOverlap;
} xge_xui_timeline_span_overlap_ctx_t;

static bool __xgeXuiTimelineSpanOverlapProc(int64 iKey, ptr pVal, ptr pArg)
{
	xge_xui_timeline_span_overlap_ctx_t* pCtx;
	xge_xui_timeline_span_t* pSpan;

	(void)iKey;
	pCtx = (xge_xui_timeline_span_overlap_ctx_t*)pArg;
	pSpan = (xge_xui_timeline_span_t*)pVal;
	if ( (pCtx == NULL) || (pSpan == NULL) ) {
		return FALSE;
	}
	if ( pSpan->iId == pCtx->iIgnoreId ) {
		return FALSE;
	}
	if ( (pCtx->iStart <= pSpan->iEndFrame) && (pCtx->iEnd >= pSpan->iStartFrame) ) {
		pCtx->bOverlap = 1;
		return TRUE;
	}
	return FALSE;
}

int xgeXuiTimelineViewAddSpan(xge_xui_timeline_view pTimeline, int iLayer, int iId, int iStartFrame, int iEndFrame, int iType, const char* sLabel)
{
	xge_xui_timeline_layer_t* pLayer;
	xge_xui_timeline_span_t* pSpan;
	xge_xui_timeline_span_overlap_ctx_t tCtx;
	char* sLabelCopy;
	bool bNew;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( (pTimeline == NULL) || (pLayer == NULL) || (iStartFrame < 0) || (iEndFrame < iStartFrame) || (iEndFrame >= pTimeline->iFrameCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pLayer->bLocked ) {
		return XGE_ERROR;
	}
	memset(&tCtx, 0, sizeof(tCtx));
	tCtx.iStart = iStartFrame;
	tCtx.iEnd = iEndFrame;
	tCtx.iIgnoreId = -1;
	xrtListWalk(&pLayer->tSpans, __xgeXuiTimelineSpanOverlapProc, &tCtx);
	if ( tCtx.bOverlap ) {
		return XGE_ERROR;
	}
	if ( iId <= 0 ) {
		iId = pTimeline->iNextSpanId++;
	} else if ( iId >= pTimeline->iNextSpanId ) {
		pTimeline->iNextSpanId = iId + 1;
	}
	iType = __xgeXuiTimelineSpanTypeNormalize(iType);
	if ( pTimeline->procSpanChanging != NULL && pTimeline->procSpanChanging(pTimeline->pWidget, iLayer, pLayer->iId, iId, -1, -1, iStartFrame, iEndFrame, -1, iType, NULL, sLabel, pTimeline->pSpanUser) == 0 ) {
		return XGE_ERROR;
	}
	sLabelCopy = __xgeXuiTimelineCopyText(sLabel);
	if ( sLabelCopy == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSpan = (xge_xui_timeline_span_t*)xrtListSet(&pLayer->tSpans, (int64)iStartFrame, &bNew);
	if ( pSpan == NULL ) {
		xrtFree(sLabelCopy);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( !bNew ) {
		if ( pSpan->sLabel != NULL ) {
			xrtFree(pSpan->sLabel);
		}
		if ( pSpan->sCustomType != NULL ) {
			xrtFree(pSpan->sCustomType);
		}
	}
	memset(pSpan, 0, sizeof(*pSpan));
	pSpan->iId = iId;
	pSpan->iStartFrame = iStartFrame;
	pSpan->iEndFrame = iEndFrame;
	pSpan->iType = iType;
	pSpan->sLabel = sLabelCopy;
	pSpan->iColor = 0;
	pTimeline->iSpanChangeCount++;
	if ( pTimeline->procSpanChanged != NULL ) {
		pTimeline->procSpanChanged(pTimeline->pWidget, iLayer, pLayer->iId, iId, -1, -1, iStartFrame, iEndFrame, -1, iType, pTimeline->pSpanUser);
	}
	__xgeXuiTimelineInvalidate(pTimeline);
	return iId;
}

typedef struct xge_xui_timeline_span_find_ctx_t {
	int iId;
	int iKey;
	int bFound;
} xge_xui_timeline_span_find_ctx_t;

static bool __xgeXuiTimelineSpanFindProc(int64 iKey, ptr pVal, ptr pArg)
{
	xge_xui_timeline_span_find_ctx_t* pCtx;
	xge_xui_timeline_span_t* pSpan;

	pCtx = (xge_xui_timeline_span_find_ctx_t*)pArg;
	pSpan = (xge_xui_timeline_span_t*)pVal;
	if ( (pCtx != NULL) && (pSpan != NULL) && (pSpan->iId == pCtx->iId) ) {
		pCtx->iKey = (int)iKey;
		pCtx->bFound = 1;
		return TRUE;
	}
	return FALSE;
}

static xge_xui_timeline_span_t* __xgeXuiTimelineSpanFind(xge_xui_timeline_layer_t* pLayer, int iSpanId, int* pKey)
{
	xge_xui_timeline_span_find_ctx_t tCtx;

	if ( pKey != NULL ) {
		*pKey = 0;
	}
	if ( (pLayer == NULL) || (iSpanId <= 0) ) {
		return NULL;
	}
	memset(&tCtx, 0, sizeof(tCtx));
	tCtx.iId = iSpanId;
	xrtListWalk(&pLayer->tSpans, __xgeXuiTimelineSpanFindProc, &tCtx);
	if ( !tCtx.bFound ) {
		return NULL;
	}
	if ( pKey != NULL ) {
		*pKey = tCtx.iKey;
	}
	return (xge_xui_timeline_span_t*)xrtListGet(&pLayer->tSpans, (int64)tCtx.iKey);
}

int xgeXuiTimelineViewRemoveSpan(xge_xui_timeline_view pTimeline, int iLayer, int iSpanId)
{
	xge_xui_timeline_layer_t* pLayer;
	xge_xui_timeline_span_t* pSpan;
	int iKey;
	int iOldStart;
	int iOldEnd;
	int iOldType;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( (pLayer == NULL) || (iSpanId <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pLayer->bLocked ) {
		return XGE_ERROR;
	}
	pSpan = __xgeXuiTimelineSpanFind(pLayer, iSpanId, &iKey);
	if ( pSpan == NULL ) {
		return XGE_ERROR;
	}
	iOldStart = pSpan->iStartFrame;
	iOldEnd = pSpan->iEndFrame;
	iOldType = pSpan->iType;
	if ( pTimeline->procSpanChanging != NULL && pTimeline->procSpanChanging(pTimeline->pWidget, iLayer, pLayer->iId, iSpanId, iOldStart, iOldEnd, -1, -1, iOldType, -1, pSpan->sLabel, NULL, pTimeline->pSpanUser) == 0 ) {
		return XGE_ERROR;
	}
	if ( pSpan != NULL ) {
		if ( pSpan->sLabel != NULL ) {
			xrtFree(pSpan->sLabel);
		}
		if ( pSpan->sCustomType != NULL ) {
			xrtFree(pSpan->sCustomType);
		}
	}
	xrtListRemove(&pLayer->tSpans, (int64)iKey);
	pTimeline->iSpanChangeCount++;
	if ( pTimeline->procSpanChanged != NULL ) {
		pTimeline->procSpanChanged(pTimeline->pWidget, iLayer, pLayer->iId, iSpanId, iOldStart, iOldEnd, -1, -1, iOldType, -1, pTimeline->pSpanUser);
	}
	__xgeXuiTimelineInvalidate(pTimeline);
	return XGE_OK;
}

int xgeXuiTimelineViewSetSpan(xge_xui_timeline_view pTimeline, int iLayer, int iSpanId, int iStartFrame, int iEndFrame, int iType, const char* sLabel)
{
	xge_xui_timeline_layer_t* pLayer;
	xge_xui_timeline_span_t* pSpan;
	xge_xui_timeline_span_t* pNewSpan;
	xge_xui_timeline_span_t tNew;
	xge_xui_timeline_span_overlap_ctx_t tCtx;
	char* sLabelCopy;
	char* sOldLabel;
	int iOldKey;
	int iOldStart;
	int iOldEnd;
	int iOldType;
	const char* sOldLabelText;
	const char* sNewLabelText;
	bool bNew;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( (pTimeline == NULL) || (pLayer == NULL) || (iSpanId <= 0) || (iStartFrame < 0) || (iEndFrame < iStartFrame) || (iEndFrame >= pTimeline->iFrameCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pLayer->bLocked ) {
		return XGE_ERROR;
	}
	pSpan = __xgeXuiTimelineSpanFind(pLayer, iSpanId, &iOldKey);
	if ( pSpan == NULL ) {
		return XGE_ERROR;
	}
	iType = __xgeXuiTimelineSpanTypeNormalize(iType);
	memset(&tCtx, 0, sizeof(tCtx));
	tCtx.iStart = iStartFrame;
	tCtx.iEnd = iEndFrame;
	tCtx.iIgnoreId = iSpanId;
	xrtListWalk(&pLayer->tSpans, __xgeXuiTimelineSpanOverlapProc, &tCtx);
	if ( tCtx.bOverlap ) {
		return XGE_ERROR;
	}
	iOldStart = pSpan->iStartFrame;
	iOldEnd = pSpan->iEndFrame;
	iOldType = pSpan->iType;
	sOldLabelText = (pSpan->sLabel != NULL) ? pSpan->sLabel : "";
	sNewLabelText = (sLabel != NULL) ? sLabel : sOldLabelText;
	if ( (iOldStart == iStartFrame) && (iOldEnd == iEndFrame) && (iOldType == iType) && (strcmp(sOldLabelText, sNewLabelText) == 0) ) {
		return XGE_OK;
	}
	if ( pTimeline->procSpanChanging != NULL && pTimeline->procSpanChanging(pTimeline->pWidget, iLayer, pLayer->iId, iSpanId, iOldStart, iOldEnd, iStartFrame, iEndFrame, iOldType, iType, sOldLabelText, sNewLabelText, pTimeline->pSpanUser) == 0 ) {
		return XGE_ERROR;
	}
	sLabelCopy = __xgeXuiTimelineCopyText(sNewLabelText);
	if ( sLabelCopy == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	tNew = *pSpan;
	tNew.iStartFrame = iStartFrame;
	tNew.iEndFrame = iEndFrame;
	tNew.iType = iType;
	tNew.sLabel = sLabelCopy;
	sOldLabel = pSpan->sLabel;
	if ( iOldKey == iStartFrame ) {
		*pSpan = tNew;
	} else {
		pNewSpan = (xge_xui_timeline_span_t*)xrtListSet(&pLayer->tSpans, (int64)iStartFrame, &bNew);
		if ( pNewSpan == NULL ) {
			xrtFree(sLabelCopy);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		if ( !bNew ) {
			xrtFree(sLabelCopy);
			return XGE_ERROR;
		}
		*pNewSpan = tNew;
		xrtListRemove(&pLayer->tSpans, (int64)iOldKey);
	}
	if ( sOldLabel != NULL ) {
		xrtFree(sOldLabel);
	}
	pTimeline->iSpanChangeCount++;
	if ( pTimeline->procSpanChanged != NULL ) {
		pTimeline->procSpanChanged(pTimeline->pWidget, iLayer, pLayer->iId, iSpanId, iOldStart, iOldEnd, iStartFrame, iEndFrame, iOldType, iType, pTimeline->pSpanUser);
	}
	__xgeXuiTimelineInvalidate(pTimeline);
	return XGE_OK;
}

xge_xui_timeline_span_t* xgeXuiTimelineViewGetSpan(xge_xui_timeline_view pTimeline, int iLayer, int iSpanId)
{
	xge_xui_timeline_layer_t* pLayer;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	return __xgeXuiTimelineSpanFind(pLayer, iSpanId, NULL);
}

int xgeXuiTimelineViewSetLayerUserData(xge_xui_timeline_view pTimeline, int iLayer, void* pUser)
{
	xge_xui_timeline_layer_t* pLayer;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( pLayer == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pLayer->pUser = pUser;
	return XGE_OK;
}

void* xgeXuiTimelineViewGetLayerUserData(xge_xui_timeline_view pTimeline, int iLayer)
{
	xge_xui_timeline_layer_t* pLayer;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	return (pLayer != NULL) ? pLayer->pUser : NULL;
}

int xgeXuiTimelineViewSetFrameUserData(xge_xui_timeline_view pTimeline, int iLayer, int iFrame, void* pUser)
{
	xge_xui_timeline_layer_t* pLayer;
	xge_xui_timeline_frame_t* pFrame;
	bool bNew;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( (pTimeline == NULL) || (pLayer == NULL) || (iFrame < 0) || (iFrame >= pTimeline->iFrameCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFrame = __xgeXuiTimelineFrameGet(pLayer, iFrame);
	if ( pFrame == NULL ) {
		if ( pUser == NULL ) {
			return XGE_OK;
		}
		pFrame = (xge_xui_timeline_frame_t*)xrtListSet(&pLayer->tFrames, (int64)iFrame, &bNew);
		if ( pFrame == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pFrame->iFrame = iFrame;
		pFrame->iType = XGE_XUI_TIMELINE_FRAME_EMPTY;
	}
	pFrame->pUser = pUser;
	return XGE_OK;
}

void* xgeXuiTimelineViewGetFrameUserData(xge_xui_timeline_view pTimeline, int iLayer, int iFrame)
{
	xge_xui_timeline_layer_t* pLayer;
	xge_xui_timeline_frame_t* pFrame;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	pFrame = __xgeXuiTimelineFrameGet(pLayer, iFrame);
	return (pFrame != NULL) ? pFrame->pUser : NULL;
}

int xgeXuiTimelineViewSetSpanUserData(xge_xui_timeline_view pTimeline, int iLayer, int iSpanId, void* pUser)
{
	xge_xui_timeline_span_t* pSpan;

	pSpan = xgeXuiTimelineViewGetSpan(pTimeline, iLayer, iSpanId);
	if ( pSpan == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pSpan->pUser = pUser;
	return XGE_OK;
}

void* xgeXuiTimelineViewGetSpanUserData(xge_xui_timeline_view pTimeline, int iLayer, int iSpanId)
{
	xge_xui_timeline_span_t* pSpan;

	pSpan = xgeXuiTimelineViewGetSpan(pTimeline, iLayer, iSpanId);
	return (pSpan != NULL) ? pSpan->pUser : NULL;
}

int xgeXuiTimelineViewClearSelection(xge_xui_timeline_view pTimeline)
{
	if ( pTimeline == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xrtListCount(&pTimeline->tSelection) == 0 ) {
		return XGE_OK;
	}
	__xgeXuiTimelineSelectionClearRaw(pTimeline);
	__xgeXuiTimelineNotifySelection(pTimeline);
	return XGE_OK;
}

int xgeXuiTimelineViewSelectFrame(xge_xui_timeline_view pTimeline, int iLayer, int iFrame, int bSelected)
{
	int bChanged;

	bChanged = __xgeXuiTimelineSelectFrameRaw(pTimeline, iLayer, iFrame, bSelected);
	if ( bChanged ) {
		__xgeXuiTimelineNotifySelection(pTimeline);
	}
	return bChanged ? XGE_OK : XGE_ERROR;
}

int xgeXuiTimelineViewSelectRange(xge_xui_timeline_view pTimeline, int iLayerA, int iFrameA, int iLayerB, int iFrameB)
{
	if ( pTimeline == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiTimelineSelectionClearRaw(pTimeline);
	__xgeXuiTimelineSelectRangeRaw(pTimeline, iLayerA, iFrameA, iLayerB, iFrameB);
	__xgeXuiTimelineNotifySelection(pTimeline);
	return XGE_OK;
}

int xgeXuiTimelineViewIsFrameSelected(xge_xui_timeline_view pTimeline, int iLayer, int iFrame)
{
	if ( pTimeline == NULL ) {
		return 0;
	}
	return xrtListExists(&pTimeline->tSelection, __xgeXuiTimelineSelectionKey(iLayer, iFrame)) ? 1 : 0;
}

int xgeXuiTimelineViewGetSelectionCount(xge_xui_timeline_view pTimeline)
{
	return (pTimeline != NULL) ? (int)xrtListCount(&pTimeline->tSelection) : 0;
}

void xgeXuiTimelineViewSetCurrentFrameProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_current_frame_changing_proc procChanging, xge_xui_timeline_current_frame_changed_proc procChanged, void* pUser)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->procCurrentFrameChanging = procChanging;
	pTimeline->procCurrentFrameChanged = procChanged;
	pTimeline->pCurrentFrameUser = pUser;
}

void xgeXuiTimelineViewSetLayerProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_layer_changing_proc procChanging, xge_xui_timeline_layer_changed_proc procChanged, void* pUser)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->procLayerChanging = procChanging;
	pTimeline->procLayerChanged = procChanged;
	pTimeline->pLayerUser = pUser;
}

void xgeXuiTimelineViewSetFrameProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_frame_changing_proc procChanging, xge_xui_timeline_frame_changed_proc procChanged, void* pUser)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->procFrameChanging = procChanging;
	pTimeline->procFrameChanged = procChanged;
	pTimeline->pFrameUser = pUser;
}

void xgeXuiTimelineViewSetSpanProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_span_changing_proc procChanging, xge_xui_timeline_span_changed_proc procChanged, void* pUser)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->procSpanChanging = procChanging;
	pTimeline->procSpanChanged = procChanged;
	pTimeline->pSpanUser = pUser;
}

void xgeXuiTimelineViewSetLayerSelectedProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_layer_selected_proc procSelected, void* pUser)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->procLayerSelected = procSelected;
	pTimeline->pLayerSelectedUser = pUser;
}

void xgeXuiTimelineViewSetContextMenuProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_context_menu_opening_proc procOpening, xge_xui_timeline_context_menu_command_proc procCommand, void* pUser)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->procContextMenuOpening = procOpening;
	pTimeline->procContextMenuCommand = procCommand;
	pTimeline->pContextMenuUser = pUser;
}

void xgeXuiTimelineViewSetFrameClick(xge_xui_timeline_view pTimeline, xge_xui_timeline_frame_click_proc procClick, void* pUser)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->procFrameClick = procClick;
	pTimeline->pFrameClickUser = pUser;
}

void xgeXuiTimelineViewSetFrameDoubleClick(xge_xui_timeline_view pTimeline, xge_xui_timeline_frame_click_proc procDoubleClick, void* pUser)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->procFrameDoubleClick = procDoubleClick;
	pTimeline->pFrameDoubleClickUser = pUser;
}

void xgeXuiTimelineViewSetSelectionProc(xge_xui_timeline_view pTimeline, xge_xui_timeline_selection_proc procSelection, void* pUser)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->procSelection = procSelection;
	pTimeline->pSelectionUser = pUser;
}

void xgeXuiTimelineViewSetRenderers(xge_xui_timeline_view pTimeline, xge_xui_timeline_layer_renderer_proc procLayer, void* pLayerUser, xge_xui_timeline_ruler_renderer_proc procRuler, void* pRulerUser, xge_xui_timeline_frame_renderer_proc procFrame, void* pFrameUser, xge_xui_timeline_span_renderer_proc procSpan, void* pSpanUser)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->procLayerRenderer = procLayer;
	pTimeline->pLayerRendererUser = pLayerUser;
	pTimeline->procRulerRenderer = procRuler;
	pTimeline->pRulerRendererUser = pRulerUser;
	pTimeline->procFrameRenderer = procFrame;
	pTimeline->pFrameRendererUser = pFrameUser;
	pTimeline->procSpanRenderer = procSpan;
	pTimeline->pSpanRendererUser = pSpanUser;
	__xgeXuiTimelineInvalidate(pTimeline);
}

void xgeXuiTimelineViewSetColors(xge_xui_timeline_view pTimeline, uint32_t iBackground, uint32_t iHeader, uint32_t iLayer, uint32_t iSelected, uint32_t iGrid, uint32_t iText)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->iBackgroundColor = iBackground;
	pTimeline->iHeaderColor = iHeader;
	pTimeline->iLayerColor = iLayer;
	pTimeline->iLayerSelectedColor = iSelected;
	pTimeline->iGridColor = iGrid;
	pTimeline->iTextColor = iText;
	__xgeXuiTimelineInvalidate(pTimeline);
}

void xgeXuiTimelineViewEnsureFrameVisible(xge_xui_timeline_view pTimeline, int iLayer, int iFrame)
{
	xge_rect_t tRect;

	if ( (pTimeline == NULL) || (iLayer < 0) || ((uint32)iLayer >= pTimeline->arrLayers.Count) || (iFrame < 0) || (iFrame >= pTimeline->iFrameCount) ) {
		return;
	}
	tRect.fX = pTimeline->fLayerHeaderWidth + (float)iFrame * pTimeline->fFrameWidth;
	tRect.fY = pTimeline->fRulerHeight + __xgeXuiTimelineLayerTop(pTimeline, iLayer);
	tRect.fW = pTimeline->fFrameWidth;
	tRect.fH = __xgeXuiTimelineLayerHeight(pTimeline, iLayer);
	if ( xgeXuiScrollModelEnsureRectVisible(&pTimeline->tScroll, tRect) ) {
		xgeXuiScrollFrameSetOffset(pTimeline->pFrame, pTimeline->tScroll.fScrollX, pTimeline->tScroll.fScrollY);
		__xgeXuiTimelineInvalidate(pTimeline);
	}
}

static void __xgeXuiTimelineSetActiveLayer(xge_xui_timeline_view pTimeline, int iLayer)
{
	xge_xui_timeline_layer_t* pLayer;
	uint32 i;

	if ( pTimeline == NULL ) {
		return;
	}
	if ( iLayer < 0 || ((uint32)iLayer >= pTimeline->arrLayers.Count) ) {
		return;
	}
	if ( pTimeline->iActiveLayer == iLayer ) {
		return;
	}
	for ( i = 0; i < pTimeline->arrLayers.Count; i++ ) {
		pLayer = __xgeXuiTimelineLayerAt(pTimeline, (int)i);
		if ( pLayer != NULL ) {
			pLayer->bSelected = ((int)i == iLayer);
		}
	}
	pTimeline->iActiveLayer = iLayer;
	pTimeline->iLayerChangeCount++;
	pTimeline->iLayerSelectCount++;
	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( pTimeline->procLayerChanged != NULL && pLayer != NULL ) {
		pTimeline->procLayerChanged(pTimeline->pWidget, iLayer, pLayer->iId, XGE_XUI_TIMELINE_LAYER_CHANGE_SELECT, pTimeline->pLayerUser);
	}
	if ( pTimeline->procLayerSelected != NULL && pLayer != NULL ) {
		pTimeline->procLayerSelected(pTimeline->pWidget, iLayer, pLayer->iId, pTimeline->pLayerSelectedUser);
	}
	__xgeXuiTimelineInvalidate(pTimeline);
}

static void __xgeXuiTimelineCommitMouseSelection(xge_xui_timeline_view pTimeline, int iModifiers)
{
	int iLayerA;
	int iLayerB;
	int iFrameA;
	int iFrameB;

	if ( pTimeline == NULL ) {
		return;
	}
	__xgeXuiTimelineHitFrame(pTimeline, pTimeline->fDragStartX, pTimeline->fDragStartY, &iLayerA, &iFrameA);
	__xgeXuiTimelineHitFrame(pTimeline, pTimeline->fDragCurrentX, pTimeline->fDragCurrentY, &iLayerB, &iFrameB);
	if ( (iLayerA < 0) || (iLayerB < 0) || (iFrameA < 0) || (iFrameB < 0) ) {
		return;
	}
	if ( (iModifiers & XGE_KEY_MOD_CTRL) == 0 ) {
		__xgeXuiTimelineSelectionClearRaw(pTimeline);
	}
	__xgeXuiTimelineSelectRangeRaw(pTimeline, iLayerA, iFrameA, iLayerB, iFrameB);
	pTimeline->iAnchorLayer = iLayerA;
	pTimeline->iAnchorFrame = iFrameA;
	__xgeXuiTimelineNotifySelection(pTimeline);
}

static int __xgeXuiTimelineLayerIconHit(xge_xui_timeline_view pTimeline, int iLayer, float fX, float fY, int* pHit)
{
	xge_rect_t tLayerList;
	xge_xui_timeline_layer_t* pLayer;
	float fTop;
	float fRowY;
	float fH;
	xge_rect_t tVisible;
	xge_rect_t tLock;

	if ( pHit != NULL ) {
		*pHit = XGE_XUI_TIMELINE_HIT_LAYER_ROW;
	}
	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( (pTimeline == NULL) || (pLayer == NULL) ) {
		return 0;
	}
	__xgeXuiTimelineRects(pTimeline, NULL, NULL, NULL, &tLayerList, NULL);
	fTop = __xgeXuiTimelineLayerTop(pTimeline, iLayer);
	fRowY = tLayerList.fY + fTop - pTimeline->tScroll.fScrollY;
	fH = __xgeXuiTimelineLayerHeight(pTimeline, iLayer);
	if ( (fY < fRowY) || (fY >= fRowY + fH) || (fX < tLayerList.fX) || (fX >= tLayerList.fX + tLayerList.fW) ) {
		return 0;
	}
	tVisible = (xge_rect_t){ tLayerList.fX + tLayerList.fW - 44.0f, fRowY, 22.0f, fH };
	tLock = (xge_rect_t){ tLayerList.fX + tLayerList.fW - 24.0f, fRowY, 22.0f, fH };
	if ( pTimeline->bShowVisibilityFeature && __xgeXuiRectContains(tVisible, fX, fY) ) {
		if ( pHit != NULL ) {
			*pHit = XGE_XUI_TIMELINE_HIT_LAYER_VISIBLE;
		}
		return 1;
	}
	if ( pTimeline->bShowLockFeature && __xgeXuiRectContains(tLock, fX, fY) ) {
		if ( pHit != NULL ) {
			*pHit = XGE_XUI_TIMELINE_HIT_LAYER_LOCK;
		}
		return 1;
	}
	if ( pHit != NULL ) {
		*pHit = XGE_XUI_TIMELINE_HIT_LAYER_NAME;
	}
	return 1;
}

typedef struct xge_xui_timeline_span_hit_ctx_t {
	xge_xui_timeline_view pTimeline;
	int iLayer;
	float fX;
	float fY;
	xge_rect_t tGrid;
	float fLayerY;
	float fLayerH;
	xge_xui_timeline_hit_t* pHit;
	int bHit;
} xge_xui_timeline_span_hit_ctx_t;

static bool __xgeXuiTimelineHitSpanProc(int64 iKey, ptr pVal, ptr pArg)
{
	xge_xui_timeline_span_hit_ctx_t* pCtx;
	xge_xui_timeline_span_t* pSpan;
	xge_rect_t tSpan;

	(void)iKey;
	pCtx = (xge_xui_timeline_span_hit_ctx_t*)pArg;
	pSpan = (xge_xui_timeline_span_t*)pVal;
	if ( (pCtx == NULL) || (pCtx->pTimeline == NULL) || (pSpan == NULL) ) {
		return FALSE;
	}
	tSpan.fX = pCtx->tGrid.fX + (float)pSpan->iStartFrame * pCtx->pTimeline->fFrameWidth - pCtx->pTimeline->tScroll.fScrollX + 2.0f;
	tSpan.fY = pCtx->fLayerY + 4.0f;
	tSpan.fW = (float)(pSpan->iEndFrame - pSpan->iStartFrame + 1) * pCtx->pTimeline->fFrameWidth - 4.0f;
	tSpan.fH = pCtx->fLayerH - 8.0f;
	if ( tSpan.fW < 2.0f ) {
		tSpan.fW = 2.0f;
	}
	if ( __xgeXuiRectContains(tSpan, pCtx->fX, pCtx->fY) == 0 ) {
		return FALSE;
	}
	if ( pCtx->pHit != NULL ) {
		pCtx->pHit->iType = XGE_XUI_TIMELINE_HIT_SPAN;
		pCtx->pHit->iLayer = pCtx->iLayer;
		pCtx->pHit->iLayerId = __xgeXuiTimelineLayerAt(pCtx->pTimeline, pCtx->iLayer)->iId;
		pCtx->pHit->iFrame = pSpan->iStartFrame;
		pCtx->pHit->iSpanId = pSpan->iId;
		pCtx->pHit->tRect = tSpan;
	}
	pCtx->bHit = 1;
	return TRUE;
}

static void __xgeXuiTimelineHitInit(xge_xui_timeline_hit_t* pHit)
{
	if ( pHit == NULL ) {
		return;
	}
	memset(pHit, 0, sizeof(*pHit));
	pHit->iType = XGE_XUI_TIMELINE_HIT_NONE;
	pHit->iLayer = -1;
	pHit->iLayerId = 0;
	pHit->iFrame = -1;
	pHit->iSpanId = 0;
}

int xgeXuiTimelineViewHitTest(xge_xui_timeline_view pTimeline, float fX, float fY, xge_xui_timeline_hit_t* pHit)
{
	xge_xui_widget pHWidget;
	xge_xui_widget pVWidget;
	xge_xui_timeline_layer_t* pLayer;
	xge_xui_timeline_span_hit_ctx_t tSpanCtx;
	xge_rect_t tViewport;
	xge_rect_t tCorner;
	xge_rect_t tRuler;
	xge_rect_t tLayerList;
	xge_rect_t tGrid;
	xge_rect_t tRect;
	xge_rect_t tVisible;
	xge_rect_t tLock;
	float fPlayheadX;
	float fLayerY;
	float fLayerH;
	int iLayer;
	int iFrame;
	int iHit;

	__xgeXuiTimelineHitInit(pHit);
	if ( (pTimeline == NULL) || (pTimeline->pWidget == NULL) || (__xgeXuiRectContains(pTimeline->pWidget->tRect, fX, fY) == 0) ) {
		return 0;
	}
	pHWidget = xgeXuiScrollFrameGetHScrollBarWidget(pTimeline->pFrame);
	pVWidget = xgeXuiScrollFrameGetVScrollBarWidget(pTimeline->pFrame);
	if ( (pHWidget != NULL) && __xgeXuiRectContains(pHWidget->tRect, fX, fY) ) {
		if ( pHit != NULL ) {
			pHit->iType = XGE_XUI_TIMELINE_HIT_HSCROLLBAR;
			pHit->tRect = pHWidget->tRect;
		}
		return 1;
	}
	if ( (pVWidget != NULL) && __xgeXuiRectContains(pVWidget->tRect, fX, fY) ) {
		if ( pHit != NULL ) {
			pHit->iType = XGE_XUI_TIMELINE_HIT_VSCROLLBAR;
			pHit->tRect = pVWidget->tRect;
		}
		return 1;
	}
	__xgeXuiTimelineRects(pTimeline, &tViewport, &tCorner, &tRuler, &tLayerList, &tGrid);
	if ( __xgeXuiRectContains(tCorner, fX, fY) ) {
		if ( pHit != NULL ) {
			pHit->iType = XGE_XUI_TIMELINE_HIT_CORNER;
			pHit->tRect = tCorner;
		}
		return 1;
	}
	fPlayheadX = tGrid.fX + (float)pTimeline->iCurrentFrame * pTimeline->fFrameWidth - pTimeline->tScroll.fScrollX;
	tRect = (xge_rect_t){ fPlayheadX - 3.0f, tRuler.fY, 7.0f, tRuler.fH + tGrid.fH };
	if ( (pTimeline->iCurrentFrame >= 0) && (pTimeline->iCurrentFrame < pTimeline->iFrameCount) && __xgeXuiRectContains(tRect, fX, fY) ) {
		if ( pHit != NULL ) {
			pHit->iType = XGE_XUI_TIMELINE_HIT_PLAYHEAD;
			pHit->iFrame = pTimeline->iCurrentFrame;
			pHit->tRect = tRect;
		}
		return 1;
	}
	if ( __xgeXuiRectContains(tRuler, fX, fY) ) {
		iFrame = __xgeXuiTimelineFrameAtX(pTimeline, fX);
		if ( pHit != NULL ) {
			pHit->iType = XGE_XUI_TIMELINE_HIT_RULER;
			pHit->iFrame = iFrame;
			if ( iFrame >= 0 ) {
				pHit->tRect = (xge_rect_t){ tGrid.fX + (float)iFrame * pTimeline->fFrameWidth - pTimeline->tScroll.fScrollX, tRuler.fY, pTimeline->fFrameWidth, tRuler.fH };
			} else {
				pHit->tRect = tRuler;
			}
		}
		return 1;
	}
	if ( __xgeXuiRectContains(tLayerList, fX, fY) ) {
		iLayer = __xgeXuiTimelineLayerAtY(pTimeline, fY);
		pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
		if ( pLayer == NULL ) {
			return 0;
		}
		fLayerY = tLayerList.fY + __xgeXuiTimelineLayerTop(pTimeline, iLayer) - pTimeline->tScroll.fScrollY;
		fLayerH = __xgeXuiTimelineLayerHeight(pTimeline, iLayer);
		iHit = XGE_XUI_TIMELINE_HIT_LAYER_ROW;
		__xgeXuiTimelineLayerIconHit(pTimeline, iLayer, fX, fY, &iHit);
		tRect = (xge_rect_t){ tLayerList.fX, fLayerY, tLayerList.fW, fLayerH };
		tVisible = (xge_rect_t){ tLayerList.fX + tLayerList.fW - 44.0f, fLayerY, 22.0f, fLayerH };
		tLock = (xge_rect_t){ tLayerList.fX + tLayerList.fW - 24.0f, fLayerY, 22.0f, fLayerH };
		if ( pHit != NULL ) {
			pHit->iType = iHit;
			pHit->iLayer = iLayer;
			pHit->iLayerId = pLayer->iId;
			pHit->tRect = (iHit == XGE_XUI_TIMELINE_HIT_LAYER_VISIBLE) ? tVisible : ((iHit == XGE_XUI_TIMELINE_HIT_LAYER_LOCK) ? tLock : tRect);
		}
		return 1;
	}
	if ( __xgeXuiTimelineHitFrame(pTimeline, fX, fY, &iLayer, &iFrame) ) {
		pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
		if ( pLayer == NULL ) {
			return 0;
		}
		fLayerY = tGrid.fY + __xgeXuiTimelineLayerTop(pTimeline, iLayer) - pTimeline->tScroll.fScrollY;
		fLayerH = __xgeXuiTimelineLayerHeight(pTimeline, iLayer);
		memset(&tSpanCtx, 0, sizeof(tSpanCtx));
		tSpanCtx.pTimeline = pTimeline;
		tSpanCtx.iLayer = iLayer;
		tSpanCtx.fX = fX;
		tSpanCtx.fY = fY;
		tSpanCtx.tGrid = tGrid;
		tSpanCtx.fLayerY = fLayerY;
		tSpanCtx.fLayerH = fLayerH;
		tSpanCtx.pHit = pHit;
		xrtListWalk(&pLayer->tSpans, __xgeXuiTimelineHitSpanProc, &tSpanCtx);
		if ( tSpanCtx.bHit ) {
			return 1;
		}
		tRect = (xge_rect_t){ tGrid.fX + (float)iFrame * pTimeline->fFrameWidth - pTimeline->tScroll.fScrollX, fLayerY, pTimeline->fFrameWidth, fLayerH };
		if ( pHit != NULL ) {
			pHit->iType = xgeXuiTimelineViewIsFrameSelected(pTimeline, iLayer, iFrame) ? XGE_XUI_TIMELINE_HIT_SELECTION : XGE_XUI_TIMELINE_HIT_FRAME;
			pHit->iLayer = iLayer;
			pHit->iLayerId = pLayer->iId;
			pHit->iFrame = iFrame;
			pHit->tRect = tRect;
		}
		return 1;
	}
	return 0;
}

static void __xgeXuiTimelineKeyboardSelection(xge_xui_timeline_view pTimeline, int iOldLayer, int iOldFrame, int iModifiers)
{
	int iLayer;
	int iFrame;

	if ( pTimeline == NULL ) {
		return;
	}
	iLayer = (pTimeline->iActiveLayer >= 0) ? pTimeline->iActiveLayer : 0;
	iFrame = pTimeline->iCurrentFrame;
	if ( (iModifiers & XGE_KEY_MOD_SHIFT) == 0 ) {
		pTimeline->iAnchorLayer = iLayer;
		pTimeline->iAnchorFrame = iFrame;
		return;
	}
	if ( pTimeline->iAnchorLayer < 0 || pTimeline->iAnchorFrame < 0 ) {
		pTimeline->iAnchorLayer = (iOldLayer >= 0) ? iOldLayer : iLayer;
		pTimeline->iAnchorFrame = (iOldFrame >= 0) ? iOldFrame : iFrame;
	}
	__xgeXuiTimelineSelectionClearRaw(pTimeline);
	__xgeXuiTimelineSelectRangeRaw(pTimeline, pTimeline->iAnchorLayer, pTimeline->iAnchorFrame, iLayer, iFrame);
	__xgeXuiTimelineNotifySelection(pTimeline);
}

static void __xgeXuiTimelineMenuItem(xge_xui_menu pMenu, const char* sText, int iType, int iState, int iValue)
{
	xge_xui_menu_item_t tItem;

	memset(&tItem, 0, sizeof(tItem));
	tItem.sText = sText;
	tItem.iType = iType;
	tItem.iState = iState;
	tItem.iValue = iValue;
	xgeXuiMenuAddItem(pMenu, &tItem);
}

static int __xgeXuiTimelineMenuState(int bEnabled)
{
	return bEnabled ? XGE_XUI_MENU_ITEM_ENABLED : 0;
}

static int __xgeXuiTimelineCloseMenus(xge_xui_timeline_view pTimeline)
{
	int bClosed;

	if ( pTimeline == NULL ) {
		return 0;
	}
	bClosed = 0;
	if ( (pTimeline->pLayerMenu != NULL) && xgeXuiMenuIsOpen(pTimeline->pLayerMenu) ) {
		xgeXuiMenuClose(pTimeline->pLayerMenu);
		bClosed = 1;
	}
	if ( (pTimeline->pFrameMenu != NULL) && xgeXuiMenuIsOpen(pTimeline->pFrameMenu) ) {
		xgeXuiMenuClose(pTimeline->pFrameMenu);
		bClosed = 1;
	}
	return bClosed;
}

static void __xgeXuiTimelineBuildLayerMenu(xge_xui_timeline_view pTimeline, const xge_xui_timeline_hit_t* pHit)
{
	xge_xui_timeline_layer_t* pLayer;
	int iState;

	if ( (pTimeline == NULL) || (pTimeline->pLayerMenu == NULL) || (pHit == NULL) ) {
		return;
	}
	pLayer = __xgeXuiTimelineLayerAt(pTimeline, pHit->iLayer);
	xgeXuiMenuBeginUpdate(pTimeline->pLayerMenu);
	xgeXuiMenuClear(pTimeline->pLayerMenu);
	if ( pLayer == NULL ) {
		xgeXuiMenuEndUpdate(pTimeline->pLayerMenu);
		return;
	}
	__xgeXuiTimelineMenuItem(pTimeline->pLayerMenu, "Rename", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, XGE_XUI_TIMELINE_MENU_LAYER_RENAME);
	iState = __xgeXuiTimelineMenuState(pTimeline->bShowVisibilityFeature);
	if ( pLayer->bVisible ) {
		iState |= XGE_XUI_MENU_ITEM_CHECKED;
	}
	__xgeXuiTimelineMenuItem(pTimeline->pLayerMenu, "Visible", XGE_XUI_MENU_ITEM_CHECK, iState, XGE_XUI_TIMELINE_MENU_LAYER_SHOW_HIDE);
	iState = __xgeXuiTimelineMenuState(pTimeline->bShowLockFeature);
	if ( pLayer->bLocked ) {
		iState |= XGE_XUI_MENU_ITEM_CHECKED;
	}
	__xgeXuiTimelineMenuItem(pTimeline->pLayerMenu, "Locked", XGE_XUI_MENU_ITEM_CHECK, iState, XGE_XUI_TIMELINE_MENU_LAYER_LOCK_UNLOCK);
	xgeXuiMenuAddSeparator(pTimeline->pLayerMenu);
	__xgeXuiTimelineMenuItem(pTimeline->pLayerMenu, "Add Layer", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, XGE_XUI_TIMELINE_MENU_LAYER_ADD);
	__xgeXuiTimelineMenuItem(pTimeline->pLayerMenu, "Delete Layer", XGE_XUI_MENU_ITEM_NORMAL, __xgeXuiTimelineMenuState(pTimeline->arrLayers.Count > 1), XGE_XUI_TIMELINE_MENU_LAYER_DELETE);
	xgeXuiMenuAddSeparator(pTimeline->pLayerMenu);
	__xgeXuiTimelineMenuItem(pTimeline->pLayerMenu, "Move Up", XGE_XUI_MENU_ITEM_NORMAL, __xgeXuiTimelineMenuState(pHit->iLayer > 0), XGE_XUI_TIMELINE_MENU_LAYER_MOVE_UP);
	__xgeXuiTimelineMenuItem(pTimeline->pLayerMenu, "Move Down", XGE_XUI_MENU_ITEM_NORMAL, __xgeXuiTimelineMenuState((uint32)(pHit->iLayer + 1) < pTimeline->arrLayers.Count), XGE_XUI_TIMELINE_MENU_LAYER_MOVE_DOWN);
	xgeXuiMenuEndUpdate(pTimeline->pLayerMenu);
}

static void __xgeXuiTimelineBuildFrameMenu(xge_xui_timeline_view pTimeline, const xge_xui_timeline_hit_t* pHit)
{
	xge_xui_timeline_layer_t* pLayer;
	int bEditable;
	int bSpanSelection;
	int iSpanStart;
	int iSpanEnd;

	if ( (pTimeline == NULL) || (pTimeline->pFrameMenu == NULL) || (pHit == NULL) ) {
		return;
	}
	pLayer = __xgeXuiTimelineLayerAt(pTimeline, pHit->iLayer);
	bEditable = (pLayer != NULL) && (pLayer->bLocked == 0) && (pHit->iFrame >= 0);
	bSpanSelection = bEditable &&
		xrtListExists(&pTimeline->tSelection, __xgeXuiTimelineSelectionKey(pHit->iLayer, pHit->iFrame)) &&
		__xgeXuiTimelineSelectionRangeForLayer(pTimeline, pHit->iLayer, &iSpanStart, &iSpanEnd) &&
		(iSpanEnd > iSpanStart);
	xgeXuiMenuBeginUpdate(pTimeline->pFrameMenu);
	xgeXuiMenuClear(pTimeline->pFrameMenu);
	__xgeXuiTimelineMenuItem(pTimeline->pFrameMenu, "Insert Frame", XGE_XUI_MENU_ITEM_NORMAL, __xgeXuiTimelineMenuState(bEditable), XGE_XUI_TIMELINE_MENU_FRAME_INSERT);
	__xgeXuiTimelineMenuItem(pTimeline->pFrameMenu, "Insert Keyframe", XGE_XUI_MENU_ITEM_NORMAL, __xgeXuiTimelineMenuState(bEditable), XGE_XUI_TIMELINE_MENU_FRAME_KEY);
	__xgeXuiTimelineMenuItem(pTimeline->pFrameMenu, "Insert Blank Keyframe", XGE_XUI_MENU_ITEM_NORMAL, __xgeXuiTimelineMenuState(bEditable), XGE_XUI_TIMELINE_MENU_FRAME_BLANK_KEY);
	__xgeXuiTimelineMenuItem(pTimeline->pFrameMenu, "Clear Keyframe", XGE_XUI_MENU_ITEM_NORMAL, __xgeXuiTimelineMenuState(bEditable), XGE_XUI_TIMELINE_MENU_FRAME_CLEAR);
	xgeXuiMenuAddSeparator(pTimeline->pFrameMenu);
	__xgeXuiTimelineMenuItem(pTimeline->pFrameMenu, bSpanSelection ? "Create Span From Selection" : "Create Span", XGE_XUI_MENU_ITEM_NORMAL, __xgeXuiTimelineMenuState(bEditable), XGE_XUI_TIMELINE_MENU_SPAN_CREATE);
	__xgeXuiTimelineMenuItem(pTimeline->pFrameMenu, "Clear Span", XGE_XUI_MENU_ITEM_NORMAL, __xgeXuiTimelineMenuState(bEditable && pHit->iSpanId > 0), XGE_XUI_TIMELINE_MENU_SPAN_CLEAR);
	xgeXuiMenuEndUpdate(pTimeline->pFrameMenu);
}

static void __xgeXuiTimelineOpenRename(xge_xui_timeline_view pTimeline, int iLayer)
{
	xge_xui_timeline_layer_t* pLayer;

	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	if ( (pTimeline == NULL) || (pTimeline->pRenameBox == NULL) || (pLayer == NULL) ) {
		return;
	}
	pTimeline->iRenameLayer = iLayer;
	xgeXuiInputBoxSetText(pTimeline->pRenameBox, pTimeline->pFont, "Rename Layer", "Layer name", (pLayer->sName != NULL) ? pLayer->sName : "");
	xgeXuiInputBoxSetOpen(pTimeline->pRenameBox, 1);
}

static void __xgeXuiTimelineRenameSubmit(xge_xui_widget pWidget, const char* sText, void* pUser)
{
	xge_xui_timeline_view pTimeline;

	(void)pWidget;
	pTimeline = (xge_xui_timeline_view)pUser;
	if ( pTimeline == NULL ) {
		return;
	}
	xgeXuiTimelineViewSetLayerName(pTimeline, pTimeline->iRenameLayer, (sText != NULL) ? sText : "");
}

static void __xgeXuiTimelineNotifyContextCommand(xge_xui_timeline_view pTimeline, int iCommand)
{
	if ( pTimeline == NULL ) {
		return;
	}
	pTimeline->iContextMenuCommandCount++;
	if ( pTimeline->procContextMenuCommand != NULL ) {
		pTimeline->procContextMenuCommand(pTimeline->pWidget, &pTimeline->tContextHit, iCommand, pTimeline->pContextMenuUser);
	}
}

static void __xgeXuiTimelineMenuSelect(xge_xui_widget pOwner, int iIndex, int iValue, void* pUser)
{
	xge_xui_timeline_view pTimeline;
	xge_xui_timeline_layer_t* pLayer;
	int iLayer;
	int iFrame;
	int iNewLayer;
	int iStart;
	int iEnd;
	int bUseSelectionRange;
	char sName[64];

	(void)pOwner;
	(void)iIndex;
	pTimeline = (xge_xui_timeline_view)pUser;
	if ( pTimeline == NULL ) {
		return;
	}
	iLayer = pTimeline->tContextHit.iLayer;
	iFrame = pTimeline->tContextHit.iFrame;
	pLayer = __xgeXuiTimelineLayerAt(pTimeline, iLayer);
	switch ( iValue ) {
		case XGE_XUI_TIMELINE_MENU_LAYER_RENAME:
			__xgeXuiTimelineOpenRename(pTimeline, iLayer);
			break;
		case XGE_XUI_TIMELINE_MENU_LAYER_SHOW_HIDE:
			if ( pLayer != NULL && pTimeline->bShowVisibilityFeature ) {
				xgeXuiTimelineViewSetLayerVisible(pTimeline, iLayer, !pLayer->bVisible);
			}
			break;
		case XGE_XUI_TIMELINE_MENU_LAYER_LOCK_UNLOCK:
			if ( pLayer != NULL && pTimeline->bShowLockFeature ) {
				xgeXuiTimelineViewSetLayerLocked(pTimeline, iLayer, !pLayer->bLocked);
			}
			break;
		case XGE_XUI_TIMELINE_MENU_LAYER_ADD:
			snprintf(sName, sizeof(sName), "Layer %d", (int)pTimeline->arrLayers.Count + 1);
			iNewLayer = xgeXuiTimelineViewAddLayer(pTimeline, 0, sName);
			if ( iNewLayer >= 0 && iLayer >= 0 && iNewLayer > iLayer + 1 ) {
				xgeXuiTimelineViewMoveLayer(pTimeline, iNewLayer, iLayer + 1);
				iNewLayer = iLayer + 1;
			}
			if ( iNewLayer >= 0 ) {
				__xgeXuiTimelineSetActiveLayer(pTimeline, iNewLayer);
			}
			break;
		case XGE_XUI_TIMELINE_MENU_LAYER_DELETE:
			if ( pLayer != NULL && pTimeline->arrLayers.Count > 1 && xgeXuiTimelineViewRemoveLayer(pTimeline, iLayer) == XGE_OK ) {
				__xgeXuiTimelineSetActiveLayer(pTimeline, __xgeXuiTimelineClampInt(iLayer, 0, (int)pTimeline->arrLayers.Count - 1));
			}
			break;
		case XGE_XUI_TIMELINE_MENU_LAYER_MOVE_UP:
			if ( iLayer > 0 && xgeXuiTimelineViewMoveLayer(pTimeline, iLayer, iLayer - 1) == XGE_OK ) {
				__xgeXuiTimelineSetActiveLayer(pTimeline, iLayer - 1);
			}
			break;
		case XGE_XUI_TIMELINE_MENU_LAYER_MOVE_DOWN:
			if ( pLayer != NULL && (uint32)(iLayer + 1) < pTimeline->arrLayers.Count && xgeXuiTimelineViewMoveLayer(pTimeline, iLayer, iLayer + 1) == XGE_OK ) {
				__xgeXuiTimelineSetActiveLayer(pTimeline, iLayer + 1);
			}
			break;
		case XGE_XUI_TIMELINE_MENU_FRAME_INSERT:
			xgeXuiTimelineViewSetFrameType(pTimeline, iLayer, iFrame, XGE_XUI_TIMELINE_FRAME_NORMAL);
			break;
		case XGE_XUI_TIMELINE_MENU_FRAME_KEY:
			xgeXuiTimelineViewSetFrameType(pTimeline, iLayer, iFrame, XGE_XUI_TIMELINE_FRAME_KEY);
			break;
		case XGE_XUI_TIMELINE_MENU_FRAME_BLANK_KEY:
			xgeXuiTimelineViewSetFrameType(pTimeline, iLayer, iFrame, XGE_XUI_TIMELINE_FRAME_BLANK_KEY);
			break;
		case XGE_XUI_TIMELINE_MENU_FRAME_CLEAR:
			xgeXuiTimelineViewClearFrame(pTimeline, iLayer, iFrame);
			break;
		case XGE_XUI_TIMELINE_MENU_SPAN_CREATE:
			iStart = iFrame;
			iEnd = iFrame;
			bUseSelectionRange = (iFrame >= 0) &&
				xrtListExists(&pTimeline->tSelection, __xgeXuiTimelineSelectionKey(iLayer, iFrame)) &&
				__xgeXuiTimelineSelectionRangeForLayer(pTimeline, iLayer, &iStart, &iEnd);
			if ( bUseSelectionRange ) {
				xgeXuiTimelineViewAddSpan(pTimeline, iLayer, 0, iStart, iEnd, XGE_XUI_TIMELINE_SPAN_CUSTOM, "span");
			} else {
				iEnd = __xgeXuiTimelineClampInt(iFrame + 5, iFrame, pTimeline->iFrameCount - 1);
				if ( xgeXuiTimelineViewAddSpan(pTimeline, iLayer, 0, iFrame, iEnd, XGE_XUI_TIMELINE_SPAN_CUSTOM, "span") < 0 ) {
					xgeXuiTimelineViewAddSpan(pTimeline, iLayer, 0, iFrame, iFrame, XGE_XUI_TIMELINE_SPAN_CUSTOM, "span");
				}
			}
			break;
		case XGE_XUI_TIMELINE_MENU_SPAN_CLEAR:
			if ( pTimeline->tContextHit.iSpanId > 0 ) {
				xgeXuiTimelineViewRemoveSpan(pTimeline, iLayer, pTimeline->tContextHit.iSpanId);
			}
			break;
		default:
			break;
	}
	__xgeXuiTimelineNotifyContextCommand(pTimeline, iValue);
}

static int __xgeXuiTimelineOpenContextMenu(xge_xui_timeline_view pTimeline, float fX, float fY)
{
	xge_xui_timeline_hit_t tHit;
	xge_xui_menu pMenu;

	if ( (pTimeline == NULL) || !xgeXuiTimelineViewHitTest(pTimeline, fX, fY, &tHit) ) {
		return __xgeXuiTimelineCloseMenus(pTimeline) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
	}
	pMenu = NULL;
	if ( (tHit.iType == XGE_XUI_TIMELINE_HIT_LAYER_ROW) || (tHit.iType == XGE_XUI_TIMELINE_HIT_LAYER_NAME) ||
	     (tHit.iType == XGE_XUI_TIMELINE_HIT_LAYER_VISIBLE) || (tHit.iType == XGE_XUI_TIMELINE_HIT_LAYER_LOCK) ) {
		pMenu = pTimeline->pLayerMenu;
	} else if ( (tHit.iType == XGE_XUI_TIMELINE_HIT_FRAME) || (tHit.iType == XGE_XUI_TIMELINE_HIT_SELECTION) || (tHit.iType == XGE_XUI_TIMELINE_HIT_SPAN) ) {
		pMenu = pTimeline->pFrameMenu;
	} else {
		return __xgeXuiTimelineCloseMenus(pTimeline) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
	}
	__xgeXuiTimelineCloseMenus(pTimeline);
	if ( pMenu == pTimeline->pLayerMenu ) {
		__xgeXuiTimelineBuildLayerMenu(pTimeline, &tHit);
	} else if ( pMenu == pTimeline->pFrameMenu ) {
		__xgeXuiTimelineBuildFrameMenu(pTimeline, &tHit);
	}
	pTimeline->tContextHit = tHit;
	pTimeline->iContextMenuOpenCount++;
	if ( pTimeline->procContextMenuOpening != NULL && pTimeline->procContextMenuOpening(pTimeline->pWidget, &pTimeline->tContextHit, pTimeline->pContextMenuUser) == 0 ) {
		return XGE_XUI_EVENT_CONSUMED;
	}
	xgeXuiMenuOpenAt(pMenu, pTimeline->pWidget, fX, fY);
	return XGE_XUI_EVENT_CONSUMED;
}

static int __xgeXuiTimelineTooltipResolve(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_tooltip_desc pDesc, void* pUser)
{
	xge_xui_timeline_view pTimeline;
	xge_xui_timeline_hit_t tHit;
	xge_xui_timeline_layer_t* pLayer;
	xge_xui_timeline_span_t* pSpan;

	(void)pWidget;
	pTimeline = (xge_xui_timeline_view)pUser;
	if ( (pTimeline == NULL) || (pContext == NULL) || (pDesc == NULL) ) {
		return 0;
	}
	if ( !xgeXuiTimelineViewHitTest(pTimeline, pContext->fTooltipMouseX, pContext->fTooltipMouseY, &tHit) ) {
		return 0;
	}
	pLayer = __xgeXuiTimelineLayerAt(pTimeline, tHit.iLayer);
	switch ( tHit.iType ) {
		case XGE_XUI_TIMELINE_HIT_LAYER_NAME:
		case XGE_XUI_TIMELINE_HIT_LAYER_ROW:
			snprintf(pTimeline->sTooltipText, sizeof(pTimeline->sTooltipText), "Layer %d: %s", tHit.iLayer + 1, (pLayer != NULL && pLayer->sName != NULL) ? pLayer->sName : "");
			break;
		case XGE_XUI_TIMELINE_HIT_LAYER_VISIBLE:
			snprintf(pTimeline->sTooltipText, sizeof(pTimeline->sTooltipText), "%s layer", (pLayer != NULL && pLayer->bVisible) ? "Hide" : "Show");
			break;
		case XGE_XUI_TIMELINE_HIT_LAYER_LOCK:
			snprintf(pTimeline->sTooltipText, sizeof(pTimeline->sTooltipText), "%s layer", (pLayer != NULL && pLayer->bLocked) ? "Unlock" : "Lock");
			break;
		case XGE_XUI_TIMELINE_HIT_PLAYHEAD:
			snprintf(pTimeline->sTooltipText, sizeof(pTimeline->sTooltipText), "Current frame: %d", tHit.iFrame);
			break;
		case XGE_XUI_TIMELINE_HIT_RULER:
			snprintf(pTimeline->sTooltipText, sizeof(pTimeline->sTooltipText), "Frame %d", tHit.iFrame);
			break;
		case XGE_XUI_TIMELINE_HIT_FRAME:
		case XGE_XUI_TIMELINE_HIT_SELECTION:
			snprintf(pTimeline->sTooltipText, sizeof(pTimeline->sTooltipText), "Layer %d, frame %d", tHit.iLayer + 1, tHit.iFrame);
			break;
		case XGE_XUI_TIMELINE_HIT_SPAN:
			pSpan = xgeXuiTimelineViewGetSpan(pTimeline, tHit.iLayer, tHit.iSpanId);
			if ( pSpan != NULL && pSpan->sLabel != NULL && pSpan->sLabel[0] != 0 ) {
				snprintf(pTimeline->sTooltipText, sizeof(pTimeline->sTooltipText), "%s: %d - %d", pSpan->sLabel, pSpan->iStartFrame, pSpan->iEndFrame);
			} else {
				snprintf(pTimeline->sTooltipText, sizeof(pTimeline->sTooltipText), "Span %d - %d", (pSpan != NULL) ? pSpan->iStartFrame : tHit.iFrame, (pSpan != NULL) ? pSpan->iEndFrame : tHit.iFrame);
			}
			break;
		default:
			return 0;
	}
	pTimeline->sTooltipText[sizeof(pTimeline->sTooltipText) - 1] = 0;
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iType = XGE_XUI_TOOLTIP_TEXT;
	pDesc->sText = pTimeline->sTooltipText;
	pDesc->iAnchor = XGE_XUI_TOOLTIP_ANCHOR_CURSOR;
	pDesc->fOffsetX = 12.0f;
	pDesc->fOffsetY = 18.0f;
	pDesc->fDelay = 0.45f;
	pDesc->bFollowCursor = 1;
	return 1;
}

int xgeXuiTimelineViewEvent(xge_xui_timeline_view pTimeline, const xge_event_t* pEvent)
{
	xge_rect_t tViewport;
	xge_rect_t tRuler;
	xge_rect_t tLayerList;
	xge_rect_t tGrid;
	xge_xui_widget pCapture;
	int iRet;
	int iInside;
	int iLayer;
	int iFrame;
	int iHit;

	if ( (pTimeline == NULL) || (pTimeline->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( ((pTimeline->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || ((pTimeline->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pTimeline->pWidget->tRect, pEvent->fX, pEvent->fY);
	__xgeXuiTimelineRects(pTimeline, &tViewport, NULL, &tRuler, &tLayerList, &tGrid);
	switch ( pEvent->iType ) {
		case XGE_EVENT_XUI_CONTEXT_MENU:
			return __xgeXuiTimelineOpenContextMenu(pTimeline, pEvent->fX, pEvent->fY);

		case XGE_EVENT_MOUSE_WHEEL:
			if ( (pEvent->iParam2 & XGE_KEY_MOD_CTRL) != 0 ) {
				if ( __xgeXuiTimelineZoomAt(pTimeline, pEvent->fX, pEvent->fY, (pEvent->fDY != 0.0f) ? pEvent->fDY : pEvent->fDX) ) {
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			iRet = __xgeXuiTimelineForwardScrollBars(pTimeline, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				return iRet;
			}
			iRet = xgeXuiScrollFrameEvent(pTimeline->pFrame, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				__xgeXuiTimelineInvalidate(pTimeline);
			}
			return iRet;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			pCapture = xgeXuiGetPointerCapture(pTimeline->pContext, pEvent->iPointerId);
			if ( pTimeline->bDraggingPlayhead && pCapture == pTimeline->pWidget ) {
				iFrame = __xgeXuiTimelineFrameAtX(pTimeline, pEvent->fX);
				if ( iFrame >= 0 ) {
					xgeXuiTimelineViewSetCurrentFrame(pTimeline, iFrame);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pTimeline->bDraggingSelection && pCapture == pTimeline->pWidget ) {
				pTimeline->fDragCurrentX = __xgeXuiTimelineClampFloat(pEvent->fX, tGrid.fX, tGrid.fX + tGrid.fW);
				pTimeline->fDragCurrentY = __xgeXuiTimelineClampFloat(pEvent->fY, tGrid.fY, tGrid.fY + tGrid.fH);
				__xgeXuiTimelineInvalidate(pTimeline);
				return XGE_XUI_EVENT_CONSUMED;
			}
			iRet = __xgeXuiTimelineForwardScrollBars(pTimeline, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				return iRet;
			}
			if ( __xgeXuiTimelineHitFrame(pTimeline, pEvent->fX, pEvent->fY, &iLayer, &iFrame) ) {
				if ( pTimeline->iHoverLayer != iLayer || pTimeline->iHoverFrame != iFrame ) {
					pTimeline->iHoverLayer = iLayer;
					pTimeline->iHoverFrame = iFrame;
					__xgeXuiTimelineInvalidate(pTimeline);
				}
			} else if ( pTimeline->iHoverLayer >= 0 || pTimeline->iHoverFrame >= 0 ) {
				pTimeline->iHoverLayer = -1;
				pTimeline->iHoverFrame = -1;
				__xgeXuiTimelineInvalidate(pTimeline);
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiTimelineCloseMenus(pTimeline);
			xgeXuiSetFocus(pTimeline->pContext, pTimeline->pWidget);
			iRet = __xgeXuiTimelineForwardScrollBars(pTimeline, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				return iRet;
			}
			if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 != XGE_MOUSE_LEFT) ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( __xgeXuiRectContains(tRuler, pEvent->fX, pEvent->fY) ) {
				iFrame = __xgeXuiTimelineFrameAtX(pTimeline, pEvent->fX);
				if ( iFrame >= 0 ) {
					xgeXuiTimelineViewSetCurrentFrame(pTimeline, iFrame);
					pTimeline->bDraggingPlayhead = 1;
					xgeXuiSetPointerCapture(pTimeline->pContext, pEvent->iPointerId, pTimeline->pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			if ( __xgeXuiRectContains(tLayerList, pEvent->fX, pEvent->fY) ) {
				iLayer = __xgeXuiTimelineLayerAtY(pTimeline, pEvent->fY);
				if ( iLayer >= 0 && __xgeXuiTimelineLayerIconHit(pTimeline, iLayer, pEvent->fX, pEvent->fY, &iHit) ) {
					if ( iHit == XGE_XUI_TIMELINE_HIT_LAYER_VISIBLE ) {
						xgeXuiTimelineViewSetLayerVisible(pTimeline, iLayer, !__xgeXuiTimelineLayerAt(pTimeline, iLayer)->bVisible);
					} else if ( iHit == XGE_XUI_TIMELINE_HIT_LAYER_LOCK ) {
						xgeXuiTimelineViewSetLayerLocked(pTimeline, iLayer, !__xgeXuiTimelineLayerAt(pTimeline, iLayer)->bLocked);
					} else {
						__xgeXuiTimelineSetActiveLayer(pTimeline, iLayer);
					}
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			if ( __xgeXuiTimelineHitFrame(pTimeline, pEvent->fX, pEvent->fY, &iLayer, &iFrame) ) {
				xgeXuiTimelineViewSetCurrentFrame(pTimeline, iFrame);
				__xgeXuiTimelineSetActiveLayer(pTimeline, iLayer);
				if ( (pEvent->iParam2 & XGE_KEY_MOD_SHIFT) && pTimeline->iAnchorLayer >= 0 && pTimeline->iAnchorFrame >= 0 ) {
					xgeXuiTimelineViewSelectRange(pTimeline, pTimeline->iAnchorLayer, pTimeline->iAnchorFrame, iLayer, iFrame);
				} else if ( pEvent->iParam2 & XGE_KEY_MOD_CTRL ) {
					__xgeXuiTimelineSelectFrameRaw(pTimeline, iLayer, iFrame, !xgeXuiTimelineViewIsFrameSelected(pTimeline, iLayer, iFrame));
					pTimeline->iAnchorLayer = iLayer;
					pTimeline->iAnchorFrame = iFrame;
					__xgeXuiTimelineNotifySelection(pTimeline);
				} else {
					__xgeXuiTimelineSelectionClearRaw(pTimeline);
					__xgeXuiTimelineSelectFrameRaw(pTimeline, iLayer, iFrame, 1);
					pTimeline->iAnchorLayer = iLayer;
					pTimeline->iAnchorFrame = iFrame;
					__xgeXuiTimelineNotifySelection(pTimeline);
				}
				if ( pTimeline->procFrameClick != NULL ) {
					pTimeline->procFrameClick(pTimeline->pWidget, iLayer, __xgeXuiTimelineLayerAt(pTimeline, iLayer)->iId, iFrame, pEvent->iParam2, pTimeline->pFrameClickUser);
				}
				pTimeline->iActiveLayer = iLayer;
				pTimeline->iActiveFrame = iFrame;
				pTimeline->bDraggingSelection = 1;
				pTimeline->fDragStartX = pEvent->fX;
				pTimeline->fDragStartY = pEvent->fY;
				pTimeline->fDragCurrentX = pEvent->fX;
				pTimeline->fDragCurrentY = pEvent->fY;
				xgeXuiSetPointerCapture(pTimeline->pContext, pEvent->iPointerId, pTimeline->pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			pCapture = xgeXuiGetPointerCapture(pTimeline->pContext, pEvent->iPointerId);
			if ( pTimeline->bDraggingPlayhead && pCapture == pTimeline->pWidget ) {
				pTimeline->bDraggingPlayhead = 0;
				xgeXuiSetPointerCapture(pTimeline->pContext, pEvent->iPointerId, NULL);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pTimeline->bDraggingSelection && pCapture == pTimeline->pWidget ) {
				pTimeline->bDraggingSelection = 0;
				pTimeline->fDragCurrentX = __xgeXuiTimelineClampFloat(pEvent->fX, tGrid.fX, tGrid.fX + tGrid.fW);
				pTimeline->fDragCurrentY = __xgeXuiTimelineClampFloat(pEvent->fY, tGrid.fY, tGrid.fY + tGrid.fH);
				if ( (fabsf(pTimeline->fDragCurrentX - pTimeline->fDragStartX) > 3.0f) || (fabsf(pTimeline->fDragCurrentY - pTimeline->fDragStartY) > 3.0f) ) {
					__xgeXuiTimelineCommitMouseSelection(pTimeline, pEvent->iParam2);
				} else {
					__xgeXuiTimelineInvalidate(pTimeline);
				}
				xgeXuiSetPointerCapture(pTimeline->pContext, pEvent->iPointerId, NULL);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return __xgeXuiTimelineForwardScrollBars(pTimeline, pEvent);

		case XGE_EVENT_XUI_DOUBLE_CLICK:
			if ( xgeXuiTimelineViewHitTest(pTimeline, pEvent->fX, pEvent->fY, &pTimeline->tContextHit) &&
			     pTimeline->tContextHit.iType == XGE_XUI_TIMELINE_HIT_LAYER_NAME ) {
				__xgeXuiTimelineOpenRename(pTimeline, pTimeline->tContextHit.iLayer);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( __xgeXuiTimelineHitFrame(pTimeline, pEvent->fX, pEvent->fY, &iLayer, &iFrame) ) {
				if ( pTimeline->procFrameDoubleClick != NULL ) {
					pTimeline->procFrameDoubleClick(pTimeline->pWidget, iLayer, __xgeXuiTimelineLayerAt(pTimeline, iLayer)->iId, iFrame, pEvent->iParam2, pTimeline->pFrameDoubleClickUser);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pTimeline->bDraggingSelection = 0;
			pTimeline->bDraggingPlayhead = 0;
			__xgeXuiTimelineInvalidate(pTimeline);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			pTimeline->iHoverLayer = -1;
			pTimeline->iHoverFrame = -1;
			__xgeXuiTimelineInvalidate(pTimeline);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_KEY_DOWN:
			if ( (pTimeline->pContext == NULL) || (pTimeline->pContext->pFocus != pTimeline->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( pEvent->iParam1 == XGE_KEY_RIGHT ) {
				iLayer = (pTimeline->iActiveLayer >= 0) ? pTimeline->iActiveLayer : 0;
				iFrame = pTimeline->iCurrentFrame;
				xgeXuiTimelineViewSetCurrentFrame(pTimeline, pTimeline->iCurrentFrame + 1);
				pTimeline->iActiveFrame = pTimeline->iCurrentFrame;
				__xgeXuiTimelineKeyboardSelection(pTimeline, iLayer, iFrame, pEvent->iParam2);
				xgeXuiTimelineViewEnsureFrameVisible(pTimeline, pTimeline->iActiveLayer >= 0 ? pTimeline->iActiveLayer : 0, pTimeline->iCurrentFrame);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_LEFT ) {
				iLayer = (pTimeline->iActiveLayer >= 0) ? pTimeline->iActiveLayer : 0;
				iFrame = pTimeline->iCurrentFrame;
				xgeXuiTimelineViewSetCurrentFrame(pTimeline, pTimeline->iCurrentFrame - 1);
				pTimeline->iActiveFrame = pTimeline->iCurrentFrame;
				__xgeXuiTimelineKeyboardSelection(pTimeline, iLayer, iFrame, pEvent->iParam2);
				xgeXuiTimelineViewEnsureFrameVisible(pTimeline, pTimeline->iActiveLayer >= 0 ? pTimeline->iActiveLayer : 0, pTimeline->iCurrentFrame);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_DOWN ) {
				iLayer = (pTimeline->iActiveLayer >= 0) ? pTimeline->iActiveLayer : 0;
				iFrame = pTimeline->iCurrentFrame;
				__xgeXuiTimelineSetActiveLayer(pTimeline, __xgeXuiTimelineClampInt(pTimeline->iActiveLayer + 1, 0, (int)pTimeline->arrLayers.Count - 1));
				pTimeline->iActiveFrame = pTimeline->iCurrentFrame;
				__xgeXuiTimelineKeyboardSelection(pTimeline, iLayer, iFrame, pEvent->iParam2);
				xgeXuiTimelineViewEnsureFrameVisible(pTimeline, pTimeline->iActiveLayer, pTimeline->iCurrentFrame);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_UP ) {
				iLayer = (pTimeline->iActiveLayer >= 0) ? pTimeline->iActiveLayer : 0;
				iFrame = pTimeline->iCurrentFrame;
				__xgeXuiTimelineSetActiveLayer(pTimeline, __xgeXuiTimelineClampInt(pTimeline->iActiveLayer - 1, 0, (int)pTimeline->arrLayers.Count - 1));
				pTimeline->iActiveFrame = pTimeline->iCurrentFrame;
				__xgeXuiTimelineKeyboardSelection(pTimeline, iLayer, iFrame, pEvent->iParam2);
				xgeXuiTimelineViewEnsureFrameVisible(pTimeline, pTimeline->iActiveLayer, pTimeline->iCurrentFrame);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiTimelineViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiTimelineViewEvent((xge_xui_timeline_view)pUser, pEvent);
}

void xgeXuiTimelineViewPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_timeline_view pTimeline;

	pTimeline = (xge_xui_timeline_view)pUser;
	if ( pTimeline == NULL && pWidget != NULL ) {
		pTimeline = (xge_xui_timeline_view)pWidget->pUser;
	}
	if ( pTimeline == NULL ) {
		return;
	}
	__xgeXuiTimelineViewportPaintProc(xgeXuiScrollFrameGetViewportWidget(pTimeline->pFrame), pTimeline);
}
