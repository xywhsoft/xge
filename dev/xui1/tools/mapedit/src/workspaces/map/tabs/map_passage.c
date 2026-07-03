#include "map_passage.h"
#include "map_maps.h"
#include <stdio.h>

#define MAPEDIT_MAP_PASSAGE_SCROLLBAR_SIZE 12.0f
#define MAPEDIT_MAP_PASSAGE_MIN_CELL 32

static float MapEditMapPassageAbsFloat(float fValue)
{
	return (fValue < 0.0f) ? -fValue : fValue;
}

static float MapEditMapPassageMinFloat(float a, float b)
{
	return (a < b) ? a : b;
}

static int MapEditMapPassageRectAlmostEqual(xge_rect_t a, xge_rect_t b)
{
	return MapEditMapPassageAbsFloat(a.fX - b.fX) < 0.01f &&
		MapEditMapPassageAbsFloat(a.fY - b.fY) < 0.01f &&
		MapEditMapPassageAbsFloat(a.fW - b.fW) < 0.01f &&
		MapEditMapPassageAbsFloat(a.fH - b.fH) < 0.01f;
}

static int MapEditMapPassageRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int MapEditMapPassageWidgetInteractive(xge_xui_widget pWidget)
{
	xge_xui_widget pIt;

	if ( pWidget == NULL ) {
		return 0;
	}
	for ( pIt = pWidget; pIt != NULL; pIt = pIt->pParent ) {
		if ( !xgeXuiWidgetIsVisible(pIt) || !xgeXuiWidgetIsEnabled(pIt) ) {
			return 0;
		}
	}
	return 1;
}

static int MapEditMapPassageMouseGridEvent(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return (pEvent->iType == XGE_EVENT_MOUSE_DOWN && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_MOVE);
}

static int MapEditMapPassageCellWidth(mapedit_app_t* pApp)
{
	int iCellW;

	iCellW = MapEditMapTileWidth(pApp);
	return (iCellW < MAPEDIT_MAP_PASSAGE_MIN_CELL) ? MAPEDIT_MAP_PASSAGE_MIN_CELL : iCellW;
}

static int MapEditMapPassageCellHeight(mapedit_app_t* pApp)
{
	int iCellH;

	iCellH = MapEditMapTileHeight(pApp);
	return (iCellH < MAPEDIT_MAP_PASSAGE_MIN_CELL) ? MAPEDIT_MAP_PASSAGE_MIN_CELL : iCellH;
}

static float MapEditMapPassageContentWidth(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pMap;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pMap == NULL ) {
		return 1.0f;
	}
	return (float)(pMap->iWidth * MapEditMapPassageCellWidth(pApp));
}

static float MapEditMapPassageContentHeight(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pMap;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pMap == NULL ) {
		return 1.0f;
	}
	return (float)(pMap->iHeight * MapEditMapPassageCellHeight(pApp));
}

static void MapEditMapPassageSetScrollContentRect(xge_xui_widget pWidget, xge_rect_t tLocal, xge_rect_t tRect)
{
	int bSizeChanged;

	if ( pWidget == NULL ) {
		return;
	}
	if ( MapEditMapPassageRectAlmostEqual(pWidget->tLocalRect, tLocal) && MapEditMapPassageRectAlmostEqual(pWidget->tRect, tRect) ) {
		return;
	}
	bSizeChanged = pWidget->tStyle.tWidth.iUnit != XGE_XUI_SIZE_PX ||
		pWidget->tStyle.tHeight.iUnit != XGE_XUI_SIZE_PX ||
		MapEditMapPassageAbsFloat(pWidget->tStyle.tWidth.fValue - tLocal.fW) >= 0.01f ||
		MapEditMapPassageAbsFloat(pWidget->tStyle.tHeight.fValue - tLocal.fH) >= 0.01f;
	if ( bSizeChanged ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(tLocal.fW), xgeXuiSizePx(tLocal.fH));
	}
	xgeXuiWidgetSetRect(pWidget, tRect);
	pWidget->tLocalRect = tLocal;
}

static void MapEditMapPassageUpdateScrollContentSize(mapedit_app_t* pApp)
{
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tMapPassageScrollFrame.pWidget == NULL ) {
		return;
	}
	fContentW = MapEditMapPassageContentWidth(pApp);
	fContentH = MapEditMapPassageContentHeight(pApp);
	if ( MapEditMapPassageAbsFloat(pApp->tMapPassageScrollModel.fContentW - fContentW) >= 0.01f ||
	     MapEditMapPassageAbsFloat(pApp->tMapPassageScrollModel.fContentH - fContentH) >= 0.01f ) {
		xgeXuiScrollFrameSetContentSize(&pApp->tMapPassageScrollFrame, fContentW, fContentH);
	}
}

static void MapEditMapPassageSyncScrollContent(mapedit_app_t* pApp)
{
	xge_rect_t tViewport;
	xge_rect_t tLocal;
	xge_rect_t tRect;
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tMapPassageScrollFrame.pWidget == NULL || pApp->pMapPassageGridWidget == NULL ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tMapPassageScrollFrame);
	fContentW = (pApp->tMapPassageScrollModel.fContentW > 1.0f) ? pApp->tMapPassageScrollModel.fContentW : MapEditMapPassageContentWidth(pApp);
	fContentH = (pApp->tMapPassageScrollModel.fContentH > 1.0f) ? pApp->tMapPassageScrollModel.fContentH : MapEditMapPassageContentHeight(pApp);
	tLocal.fX = -pApp->tMapPassageScrollModel.fScrollX;
	tLocal.fY = -pApp->tMapPassageScrollModel.fScrollY;
	tLocal.fW = (fContentW > tViewport.fW) ? fContentW : tViewport.fW;
	tLocal.fH = (fContentH > tViewport.fH) ? fContentH : tViewport.fH;
	tRect.fX = tViewport.fX + tLocal.fX;
	tRect.fY = tViewport.fY + tLocal.fY;
	tRect.fW = tLocal.fW;
	tRect.fH = tLocal.fH;
	MapEditMapPassageSetScrollContentRect(pApp->pMapPassageGridWidget, tLocal, tRect);
}

static void MapEditMapPassageUpdateAndSyncScrollContent(mapedit_app_t* pApp)
{
	MapEditMapPassageUpdateScrollContentSize(pApp);
	MapEditMapPassageSyncScrollContent(pApp);
}

static void MapEditMapPassageScrollChanged(xge_xui_scroll_frame pFrame, float fScrollX, float fScrollY, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pFrame;
	(void)fScrollX;
	(void)fScrollY;
	pApp = (mapedit_app_t*)pUser;
	MapEditMapPassageSyncScrollContent(pApp);
}

static void MapEditMapPassageScrollLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiScrollFrameLayout(&pApp->tMapPassageScrollFrame);
	MapEditMapPassageSyncScrollContent(pApp);
}

static unsigned char MapEditMapPassageBitForRegion(int iRegionCol, int iRegionRow)
{
	if ( iRegionRow == 0 && iRegionCol == 0 ) {
		return 0x01;
	}
	if ( iRegionRow == 0 && iRegionCol == 1 ) {
		return 0x02;
	}
	if ( iRegionRow == 0 && iRegionCol == 2 ) {
		return 0x04;
	}
	if ( iRegionRow == 1 && iRegionCol == 0 ) {
		return 0x08;
	}
	if ( iRegionRow == 1 && iRegionCol == 2 ) {
		return 0x10;
	}
	if ( iRegionRow == 2 && iRegionCol == 0 ) {
		return 0x20;
	}
	if ( iRegionRow == 2 && iRegionCol == 1 ) {
		return 0x40;
	}
	if ( iRegionRow == 2 && iRegionCol == 2 ) {
		return 0x80;
	}
	return 0;
}

static void MapEditMapPassageDrawLineTriangle(xge_vec2_t a, xge_vec2_t b, xge_vec2_t c)
{
	uint32_t iRed;

	iRed = XGE_COLOR_RGBA(232, 26, 36, 225);
	xgeShapeLinePx(a.fX, a.fY, b.fX, b.fY, 2.0f, iRed);
	xgeShapeLinePx(b.fX, b.fY, c.fX, c.fY, 2.0f, iRed);
	xgeShapeLinePx(c.fX, c.fY, a.fX, a.fY, 2.0f, iRed);
}

static void MapEditMapPassageDrawForbiddenTriangle(xge_rect_t tCell, int iRegionCol, int iRegionRow)
{
	float fSize;
	float fPad;
	float fTri;
	float fX;
	float fY;
	float fW;
	float fH;
	xge_vec2_t a;
	xge_vec2_t b;
	xge_vec2_t c;

	fX = tCell.fX;
	fY = tCell.fY;
	fW = tCell.fW;
	fH = tCell.fH;
	fSize = MapEditMapPassageMinFloat(fW, fH);
	fPad = (fSize <= 16.0f) ? 2.0f : 3.0f;
	fTri = fSize * 0.36f;
	if ( fTri < 5.0f ) {
		fTri = 5.0f;
	}
	if ( iRegionRow == 0 && iRegionCol == 0 ) {
		a = (xge_vec2_t){ fX + fPad, fY + fPad };
		b = (xge_vec2_t){ fX + fPad + fTri, fY + fPad };
		c = (xge_vec2_t){ fX + fPad, fY + fPad + fTri };
	} else if ( iRegionRow == 0 && iRegionCol == 1 ) {
		a = (xge_vec2_t){ fX + fW * 0.5f, fY + fPad + fTri };
		b = (xge_vec2_t){ fX + fW * 0.5f - fTri * 0.5f, fY + fPad };
		c = (xge_vec2_t){ fX + fW * 0.5f + fTri * 0.5f, fY + fPad };
	} else if ( iRegionRow == 0 && iRegionCol == 2 ) {
		a = (xge_vec2_t){ fX + fW - fPad, fY + fPad };
		b = (xge_vec2_t){ fX + fW - fPad - fTri, fY + fPad };
		c = (xge_vec2_t){ fX + fW - fPad, fY + fPad + fTri };
	} else if ( iRegionRow == 1 && iRegionCol == 0 ) {
		a = (xge_vec2_t){ fX + fPad + fTri, fY + fH * 0.5f };
		b = (xge_vec2_t){ fX + fPad, fY + fH * 0.5f - fTri * 0.5f };
		c = (xge_vec2_t){ fX + fPad, fY + fH * 0.5f + fTri * 0.5f };
	} else if ( iRegionRow == 1 && iRegionCol == 2 ) {
		a = (xge_vec2_t){ fX + fW - fPad - fTri, fY + fH * 0.5f };
		b = (xge_vec2_t){ fX + fW - fPad, fY + fH * 0.5f - fTri * 0.5f };
		c = (xge_vec2_t){ fX + fW - fPad, fY + fH * 0.5f + fTri * 0.5f };
	} else if ( iRegionRow == 2 && iRegionCol == 0 ) {
		a = (xge_vec2_t){ fX + fPad, fY + fH - fPad };
		b = (xge_vec2_t){ fX + fPad + fTri, fY + fH - fPad };
		c = (xge_vec2_t){ fX + fPad, fY + fH - fPad - fTri };
	} else if ( iRegionRow == 2 && iRegionCol == 1 ) {
		a = (xge_vec2_t){ fX + fW * 0.5f, fY + fH - fPad - fTri };
		b = (xge_vec2_t){ fX + fW * 0.5f - fTri * 0.5f, fY + fH - fPad };
		c = (xge_vec2_t){ fX + fW * 0.5f + fTri * 0.5f, fY + fH - fPad };
	} else if ( iRegionRow == 2 && iRegionCol == 2 ) {
		a = (xge_vec2_t){ fX + fW - fPad, fY + fH - fPad };
		b = (xge_vec2_t){ fX + fW - fPad - fTri, fY + fH - fPad };
		c = (xge_vec2_t){ fX + fW - fPad, fY + fH - fPad - fTri };
	} else {
		return;
	}
	MapEditMapPassageDrawLineTriangle(a, b, c);
}

static void MapEditMapPassageDrawOverlay(xge_rect_t tCell, unsigned char iValue)
{
	float fSize;
	float fDotRadius;
	int iRegionCol;
	int iRegionRow;
	unsigned char iBit;

	fSize = MapEditMapPassageMinFloat(tCell.fW, tCell.fH);
	if ( iValue == 255 ) {
		fDotRadius = fSize * 0.12f;
		if ( fDotRadius < 2.0f ) {
			fDotRadius = 2.0f;
		} else if ( fDotRadius > 4.0f ) {
			fDotRadius = 4.0f;
		}
		xgeShapeCircleFillPx(tCell.fX + tCell.fW * 0.5f, tCell.fY + tCell.fH * 0.5f, fDotRadius + 1.0f, XGE_COLOR_RGBA(245, 252, 246, 210));
		xgeShapeCircleFillPx(tCell.fX + tCell.fW * 0.5f, tCell.fY + tCell.fH * 0.5f, fDotRadius, XGE_COLOR_RGBA(26, 166, 62, 230));
		return;
	}
	if ( iValue == 0 ) {
		xgeShapeRectStrokePx((xge_rect_t){ tCell.fX + 1.5f, tCell.fY + 1.5f, tCell.fW - 3.0f, tCell.fH - 3.0f }, 2.0f, XGE_COLOR_RGBA(232, 26, 36, 225));
		xgeShapeLinePx(tCell.fX + 3.0f, tCell.fY + tCell.fH - 3.0f, tCell.fX + tCell.fW - 3.0f, tCell.fY + 3.0f, 2.0f, XGE_COLOR_RGBA(232, 26, 36, 225));
		return;
	}
	for ( iRegionRow = 0; iRegionRow < 3; iRegionRow++ ) {
		for ( iRegionCol = 0; iRegionCol < 3; iRegionCol++ ) {
			if ( iRegionRow == 1 && iRegionCol == 1 ) {
				continue;
			}
			iBit = MapEditMapPassageBitForRegion(iRegionCol, iRegionRow);
			if ( (iValue & iBit) == 0 ) {
				MapEditMapPassageDrawForbiddenTriangle(tCell, iRegionCol, iRegionRow);
			}
		}
	}
}

static void MapEditMapPassageCellPaint(mapedit_tile_grid_t* pGrid, int iCol, int iRow, xge_rect_t tCell, void* pUser)
{
	mapedit_app_t* pApp;
	unsigned char iValue;

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	MapEditMapDrawCell(pApp, pGrid, iCol, iRow, tCell);
	iValue = MapEditMapGetCellPassage(pApp, iCol, iRow);
	MapEditMapPassageDrawOverlay(tCell, iValue);
}

static void MapEditMapPassageCellClick(mapedit_tile_grid_t* pGrid, int iCol, int iRow, void* pUser)
{
	mapedit_app_t* pApp;
	xge_rect_t tCell;
	char sStatus[160];
	float fRelX;
	float fRelY;
	int iRegionCol;
	int iRegionRow;
	int iCellId;
	unsigned char iValue;
	unsigned char iBit;

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || pGrid == NULL ) {
		return;
	}
	iCellId = MapEditMapCellId(MapEditMapSelectedItem(pApp), iCol, iRow);
	if ( iCellId < 0 ) {
		MapEditAppSetStatus(pApp, "该位置不在地图范围内");
		return;
	}
	pApp->iMapPassageSelectedCell = iCellId;
	iValue = MapEditMapGetCellPassage(pApp, iCol, iRow);
	tCell = (xge_rect_t){
		pGrid->pWidget->tContentRect.fX + (float)(iCol * pGrid->iCellWidth),
		pGrid->pWidget->tContentRect.fY + (float)(iRow * pGrid->iCellHeight),
		(float)pGrid->iCellWidth,
		(float)pGrid->iCellHeight
	};
	fRelX = pApp->fMapPassageLastMouseX - tCell.fX;
	fRelY = pApp->fMapPassageLastMouseY - tCell.fY;
	iRegionCol = (int)(fRelX / (tCell.fW / 3.0f));
	iRegionRow = (int)(fRelY / (tCell.fH / 3.0f));
	if ( iRegionCol < 0 ) {
		iRegionCol = 0;
	} else if ( iRegionCol > 2 ) {
		iRegionCol = 2;
	}
	if ( iRegionRow < 0 ) {
		iRegionRow = 0;
	} else if ( iRegionRow > 2 ) {
		iRegionRow = 2;
	}
	iBit = MapEditMapPassageBitForRegion(iRegionCol, iRegionRow);
	if ( iBit == 0 ) {
		iValue = (iValue == 0) ? 255 : 0;
	} else {
		iValue = (unsigned char)(iValue ^ iBit);
	}
	if ( MapEditMapSetCellPassage(pApp, iCol, iRow, iValue) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "地图通行修正写入失败");
		return;
	}
	if ( MapEditMapSaveSelected(pApp) == XGE_OK ) {
		snprintf(sStatus, sizeof(sStatus), "地图通行修正已保存: (%d,%d) = %u", iCol, iRow, (unsigned int)iValue);
	} else {
		snprintf(sStatus, sizeof(sStatus), "地图通行修正已修改，但保存失败: (%d,%d)", iCol, iRow);
	}
	MapEditAppSetStatus(pApp, sStatus);
}

void MapEditMapPassageRefresh(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pMap;
	int iCols;
	int iRows;

	if ( pApp == NULL || pApp->pMapPassageGridWidget == NULL ) {
		return;
	}
	MapEditMapEnsureDefaults(pApp);
	pMap = MapEditMapSelectedItem(pApp);
	if ( pMap != NULL ) {
		MapEditMapEnsureTileData(pApp);
		iCols = pMap->iWidth;
		iRows = pMap->iHeight;
	} else {
		iCols = 1;
		iRows = 1;
	}
	MapEditTileGridSetCellSize(&pApp->tMapPassageGrid, MapEditMapPassageCellWidth(pApp), MapEditMapPassageCellHeight(pApp));
	MapEditTileGridSetGridSize(&pApp->tMapPassageGrid, iCols, iRows);
	MapEditTileGridSetMaxCells(&pApp->tMapPassageGrid, iCols * iRows);
	pApp->tMapPassageGrid.iHoverCol = -1;
	pApp->tMapPassageGrid.iHoverRow = -1;
	pApp->tMapPassageGrid.bHasSelection = 0;
	pApp->iMapPassageSelectedCell = -1;
	if ( pApp->tMapPassageScrollFrame.pWidget != NULL ) {
		xgeXuiScrollFrameSetOffset(&pApp->tMapPassageScrollFrame, 0.0f, 0.0f);
		MapEditMapPassageUpdateAndSyncScrollContent(pApp);
	}
	xgeXuiWidgetMarkLayout(pApp->pMapPassageCanvasWidget);
	xgeXuiWidgetMarkPaint(pApp->pMapPassageGridWidget);
}

int MapEditMapPassageHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent)
{
	xge_rect_t tViewport;

	if ( pApp == NULL || pEvent == NULL || pApp->pMapPassageGridWidget == NULL || !MapEditMapPassageMouseGridEvent(pEvent) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( !MapEditMapPassageWidgetInteractive(pApp->pMapPassageGridWidget) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tMapPassageScrollFrame);
	if ( pApp->tMapPassageGrid.bDragging ||
	     (MapEditMapPassageRectContains(tViewport, pEvent->fX, pEvent->fY) &&
	      MapEditMapPassageRectContains(pApp->pMapPassageGridWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		pApp->fMapPassageLastMouseX = pEvent->fX;
		pApp->fMapPassageLastMouseY = pEvent->fY;
		return MapEditTileGridEventProc(pApp->pMapPassageGridWidget, pEvent, &pApp->tMapPassageGrid);
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int MapEditMapPassageCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
{
	xge_xui_widget pContent;
	xge_xui_widget pCanvasWidget;
	xge_xui_widget pGridWidget;

	if ( pApp == NULL || pWindow == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pContent = MapEditWorkspaceNewContent(pApp);
	pCanvasWidget = xgeXuiWidgetCreate();
	pGridWidget = xgeXuiWidgetCreate();
	if ( pContent == NULL || pCanvasWidget == NULL || pGridWidget == NULL ) {
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetSize(pCanvasWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetBackground(pCanvasWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pCanvasWidget, 1.0f, XGE_COLOR_RGBA(164, 206, 236, 255));
	xgeXuiWidgetSetSize(pGridWidget, xgeXuiSizePx(1.0f), xgeXuiSizePx(1.0f));
	xgeXuiScrollModelInit(&pApp->tMapPassageScrollModel);
	if ( xgeXuiScrollFrameInit(&pApp->tMapPassageScrollFrame, &pApp->tXui, pCanvasWidget, &pApp->tMapPassageScrollModel) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetEvent(pCanvasWidget, xgeXuiScrollFrameEventProc, &pApp->tMapPassageScrollFrame);
	xgeXuiWidgetSetLayoutProc(pCanvasWidget, MapEditMapPassageScrollLayoutProc, pApp);
	xgeXuiWidgetSetLayout(xgeXuiScrollFrameGetViewportWidget(&pApp->tMapPassageScrollFrame), XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiScrollFrameSetChange(&pApp->tMapPassageScrollFrame, MapEditMapPassageScrollChanged, pApp);
	xgeXuiScrollFrameSetScrollbarMode(&pApp->tMapPassageScrollFrame, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiScrollFrameSetWheelAxis(&pApp->tMapPassageScrollFrame, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollFrameSetContentDragEnabled(&pApp->tMapPassageScrollFrame, 0);
	xgeXuiScrollFrameSetMetrics(&pApp->tMapPassageScrollFrame, MAPEDIT_MAP_PASSAGE_SCROLLBAR_SIZE, 28.0f, 0.0f, 0.0f);
	xgeXuiScrollFrameSetColors(&pApp->tMapPassageScrollFrame, XGE_COLOR_RGBA(226, 240, 249, 255), XGE_COLOR_RGBA(90, 164, 214, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(68, 142, 204, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(180, 204, 218, 255));
	if ( xgeXuiWidgetAdd(xgeXuiScrollFrameGetViewportWidget(&pApp->tMapPassageScrollFrame), pGridWidget) != XGE_OK ||
	     MapEditTileGridInit(&pApp->tMapPassageGrid, pGridWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	MapEditTileGridSetExpand(&pApp->tMapPassageGrid, 0, 0);
	MapEditTileGridSetColors(&pApp->tMapPassageGrid,
		XGE_COLOR_RGBA(236, 246, 252, 255),
		XGE_COLOR_RGBA(124, 181, 219, 255),
		XGE_COLOR_RGBA(188, 224, 244, 130),
		XGE_COLOR_RGBA(102, 181, 230, 8),
		XGE_COLOR_RGBA(42, 140, 210, 80),
		XGE_COLOR_RGBA(104, 128, 148, 255));
	MapEditTileGridSetInteractionPaint(&pApp->tMapPassageGrid, 0, 0);
	MapEditTileGridSetEmptyText(&pApp->tMapPassageGrid, "未选择地图");
	MapEditTileGridSetCellPaint(&pApp->tMapPassageGrid, MapEditMapPassageCellPaint, pApp);
	MapEditTileGridSetCellClick(&pApp->tMapPassageGrid, MapEditMapPassageCellClick, pApp);
	pApp->pMapPassageCanvasWidget = pCanvasWidget;
	pApp->pMapPassageGridWidget = pGridWidget;
	if ( xgeXuiWidgetAdd(pContent, pCanvasWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	MapEditMapPassageRefresh(pApp);
	return XGE_OK;
}

void MapEditMapPassageUnit(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pMapPassageGridWidget != NULL ) {
		MapEditTileGridUnit(&pApp->tMapPassageGrid);
		pApp->pMapPassageGridWidget = NULL;
	}
	if ( pApp->pMapPassageCanvasWidget != NULL ) {
		xgeXuiScrollFrameUnit(&pApp->tMapPassageScrollFrame);
		pApp->pMapPassageCanvasWidget = NULL;
	}
}
