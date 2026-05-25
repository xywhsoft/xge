#include "tileset_actor_overlay.h"
#include "tileset_sets.h"
#include <stdio.h>
#include <string.h>

#define MAPEDIT_TILESET_ACTOR_OVERLAY_SCROLLBAR_SIZE 12.0f
#define MAPEDIT_TILESET_ACTOR_OVERLAY_PLACEHOLDER_ROWS 8

static float MapEditTilesetActorOverlayAbsFloat(float fValue)
{
	return (fValue < 0.0f) ? -fValue : fValue;
}

static int MapEditTilesetActorOverlayRectAlmostEqual(xge_rect_t a, xge_rect_t b)
{
	return MapEditTilesetActorOverlayAbsFloat(a.fX - b.fX) < 0.01f &&
		MapEditTilesetActorOverlayAbsFloat(a.fY - b.fY) < 0.01f &&
		MapEditTilesetActorOverlayAbsFloat(a.fW - b.fW) < 0.01f &&
		MapEditTilesetActorOverlayAbsFloat(a.fH - b.fH) < 0.01f;
}

static int MapEditTilesetActorOverlayRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int MapEditTilesetActorOverlayWidgetInteractive(xge_xui_widget pWidget)
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

static int MapEditTilesetActorOverlayMouseGridEvent(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return (pEvent->iType == XGE_EVENT_MOUSE_DOWN && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_MOVE);
}

static int MapEditTilesetActorOverlayTilesPerRow(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTilesPerRow <= 0 ) {
		return 20;
	}
	return pApp->iSetupTilesPerRow;
}

static int MapEditTilesetActorOverlayCellWidth(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTileWidth <= 0 ) {
		return 16;
	}
	return pApp->iSetupTileWidth;
}

static int MapEditTilesetActorOverlayCellHeight(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTileHeight <= 0 ) {
		return 16;
	}
	return pApp->iSetupTileHeight;
}

static int MapEditTilesetActorOverlayStaticStart(mapedit_app_t* pApp)
{
	mapedit_tileset_set_item_t* pItem;
	int iPerRow;
	int iReserved;

	pItem = MapEditTilesetSetSelectedItem(pApp);
	iPerRow = MapEditTilesetActorOverlayTilesPerRow(pApp);
	iReserved = 1 + ((pItem != NULL) ? pItem->iSpecialTileCount : 0);
	if ( iReserved < 1 ) {
		iReserved = 1;
	}
	return ((iReserved + iPerRow - 1) / iPerRow) * iPerRow;
}

static int MapEditTilesetActorOverlayStaticRows(mapedit_app_t* pApp)
{
	int iCellH;
	int iRows;

	if ( pApp == NULL ) {
		return MAPEDIT_TILESET_ACTOR_OVERLAY_PLACEHOLDER_ROWS;
	}
	iCellH = MapEditTilesetActorOverlayCellHeight(pApp);
	if ( pApp->tTilesetActorOverlayStaticDesc.iHeight > 0 ) {
		iRows = (pApp->tTilesetActorOverlayStaticDesc.iHeight + iCellH - 1) / iCellH;
		return (iRows > 0) ? iRows : 1;
	}
	return MAPEDIT_TILESET_ACTOR_OVERLAY_PLACEHOLDER_ROWS;
}

static int MapEditTilesetActorOverlayStaticCols(mapedit_app_t* pApp)
{
	int iCellW;
	int iCols;
	int iPerRow;

	if ( pApp == NULL || pApp->tTilesetActorOverlayStaticDesc.iWidth <= 0 ) {
		return 0;
	}
	iCellW = MapEditTilesetActorOverlayCellWidth(pApp);
	iPerRow = MapEditTilesetActorOverlayTilesPerRow(pApp);
	iCols = (pApp->tTilesetActorOverlayStaticDesc.iWidth + iCellW - 1) / iCellW;
	if ( iCols > iPerRow ) {
		iCols = iPerRow;
	}
	return (iCols > 0) ? iCols : 0;
}

static float MapEditTilesetActorOverlayGridContentWidth(mapedit_app_t* pApp)
{
	return (float)(MapEditTilesetActorOverlayTilesPerRow(pApp) * MapEditTilesetActorOverlayCellWidth(pApp));
}

static float MapEditTilesetActorOverlayGridContentHeight(mapedit_app_t* pApp)
{
	int iRows;

	if ( pApp == NULL ) {
		return 1.0f;
	}
	iRows = pApp->tTilesetActorOverlayGrid.iRowCount;
	if ( iRows <= 0 ) {
		iRows = 1;
	}
	return (float)(iRows * MapEditTilesetActorOverlayCellHeight(pApp));
}

static void MapEditTilesetActorOverlaySetScrollContentRect(xge_xui_widget pWidget, xge_rect_t tLocal, xge_rect_t tRect)
{
	int bSizeChanged;

	if ( pWidget == NULL ) {
		return;
	}
	if ( MapEditTilesetActorOverlayRectAlmostEqual(pWidget->tLocalRect, tLocal) && MapEditTilesetActorOverlayRectAlmostEqual(pWidget->tRect, tRect) ) {
		return;
	}
	bSizeChanged = pWidget->tStyle.tWidth.iUnit != XGE_XUI_SIZE_PX ||
		pWidget->tStyle.tHeight.iUnit != XGE_XUI_SIZE_PX ||
		MapEditTilesetActorOverlayAbsFloat(pWidget->tStyle.tWidth.fValue - tLocal.fW) >= 0.01f ||
		MapEditTilesetActorOverlayAbsFloat(pWidget->tStyle.tHeight.fValue - tLocal.fH) >= 0.01f;
	if ( bSizeChanged ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(tLocal.fW), xgeXuiSizePx(tLocal.fH));
	}
	xgeXuiWidgetSetRect(pWidget, tRect);
	pWidget->tLocalRect = tLocal;
}

static void MapEditTilesetActorOverlayClearStaticTexture(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pTilesetActorOverlayStaticTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pApp->pTilesetActorOverlayStaticTexture);
		pApp->pTilesetActorOverlayStaticTexture = NULL;
	}
	memset(&pApp->tTilesetActorOverlayStaticDesc, 0, sizeof(pApp->tTilesetActorOverlayStaticDesc));
}

static void MapEditTilesetActorOverlayLoadStaticTexture(mapedit_app_t* pApp, mapedit_tileset_set_item_t* pItem)
{
	str sPath;
	xui_texture pTexture;
	xui_texture_desc_t tDesc;

	MapEditTilesetActorOverlayClearStaticTexture(pApp);
	if ( pApp == NULL || pItem == NULL || pItem->sStaticTiles[0] == 0 ) {
		return;
	}
	sPath = xrtPathJoin(4, xCore.AppPath, (str)"assets", (str)"tilesheets", (str)pItem->sStaticTiles);
	if ( sPath == NULL ) {
		return;
	}
	if ( !xrtFileExists(sPath) ) {
		xrtFree(sPath);
		return;
	}
	pTexture = NULL;
	memset(&tDesc, 0, sizeof(tDesc));
	if ( xgeXuiTextureCreateFile(&pApp->tXui, (const char*)sPath, XGE_IMAGE_STRAIGHT_ALPHA, &pTexture) == XGE_OK &&
	     xgeXuiTextureGetDesc(&pApp->tXui, pTexture, &tDesc) == XGE_OK &&
	     tDesc.iWidth > 0 && tDesc.iHeight > 0 ) {
		pApp->pTilesetActorOverlayStaticTexture = pTexture;
		pApp->tTilesetActorOverlayStaticDesc = tDesc;
	} else if ( pTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pTexture);
	}
	xrtFree(sPath);
}

static void MapEditTilesetActorOverlayUpdateScrollContentSize(mapedit_app_t* pApp)
{
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tTilesetActorOverlayScrollFrame.pWidget == NULL ) {
		return;
	}
	fContentW = MapEditTilesetActorOverlayGridContentWidth(pApp);
	fContentH = MapEditTilesetActorOverlayGridContentHeight(pApp);
	if ( MapEditTilesetActorOverlayAbsFloat(pApp->tTilesetActorOverlayScrollModel.fContentW - fContentW) >= 0.01f ||
	     MapEditTilesetActorOverlayAbsFloat(pApp->tTilesetActorOverlayScrollModel.fContentH - fContentH) >= 0.01f ) {
		xgeXuiScrollFrameSetContentSize(&pApp->tTilesetActorOverlayScrollFrame, fContentW, fContentH);
	}
}

static void MapEditTilesetActorOverlaySyncScrollContent(mapedit_app_t* pApp)
{
	xge_rect_t tViewport;
	xge_rect_t tLocal;
	xge_rect_t tRect;
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tTilesetActorOverlayScrollFrame.pWidget == NULL || pApp->pTilesetActorOverlayGridWidget == NULL ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tTilesetActorOverlayScrollFrame);
	fContentW = (pApp->tTilesetActorOverlayScrollModel.fContentW > 1.0f) ? pApp->tTilesetActorOverlayScrollModel.fContentW : MapEditTilesetActorOverlayGridContentWidth(pApp);
	fContentH = (pApp->tTilesetActorOverlayScrollModel.fContentH > 1.0f) ? pApp->tTilesetActorOverlayScrollModel.fContentH : MapEditTilesetActorOverlayGridContentHeight(pApp);
	tLocal.fX = -pApp->tTilesetActorOverlayScrollModel.fScrollX;
	tLocal.fY = -pApp->tTilesetActorOverlayScrollModel.fScrollY;
	tLocal.fW = (fContentW > tViewport.fW) ? fContentW : tViewport.fW;
	tLocal.fH = (fContentH > tViewport.fH) ? fContentH : tViewport.fH;
	tRect.fX = tViewport.fX + tLocal.fX;
	tRect.fY = tViewport.fY + tLocal.fY;
	tRect.fW = tLocal.fW;
	tRect.fH = tLocal.fH;
	MapEditTilesetActorOverlaySetScrollContentRect(pApp->pTilesetActorOverlayGridWidget, tLocal, tRect);
}

static void MapEditTilesetActorOverlayUpdateAndSyncScrollContent(mapedit_app_t* pApp)
{
	MapEditTilesetActorOverlayUpdateScrollContentSize(pApp);
	MapEditTilesetActorOverlaySyncScrollContent(pApp);
}

static void MapEditTilesetActorOverlayScrollChanged(xge_xui_scroll_frame pFrame, float fScrollX, float fScrollY, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pFrame;
	(void)fScrollX;
	(void)fScrollY;
	pApp = (mapedit_app_t*)pUser;
	MapEditTilesetActorOverlaySyncScrollContent(pApp);
}

static void MapEditTilesetActorOverlayScrollLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiScrollFrameLayout(&pApp->tTilesetActorOverlayScrollFrame);
	MapEditTilesetActorOverlaySyncScrollContent(pApp);
}

static int MapEditTilesetActorOverlayTileEditable(mapedit_app_t* pApp, mapedit_tileset_set_item_t* pItem, int iTileId)
{
	int iStaticStart;
	int iLocal;
	int iRow;
	int iCol;

	if ( pApp == NULL || pItem == NULL || iTileId <= 0 ) {
		return 0;
	}
	if ( iTileId >= 1 && iTileId <= pItem->iSpecialTileCount ) {
		return 1;
	}
	iStaticStart = MapEditTilesetActorOverlayStaticStart(pApp);
	if ( iTileId < iStaticStart || pApp->pTilesetActorOverlayStaticTexture == NULL ) {
		return 0;
	}
	iLocal = iTileId - iStaticStart;
	iRow = iLocal / MapEditTilesetActorOverlayTilesPerRow(pApp);
	iCol = iLocal % MapEditTilesetActorOverlayTilesPerRow(pApp);
	if ( iRow < 0 || iRow >= MapEditTilesetActorOverlayStaticRows(pApp) ) {
		return 0;
	}
	return iCol < MapEditTilesetActorOverlayStaticCols(pApp);
}

static unsigned char MapEditTilesetActorOverlayValue(mapedit_tileset_set_item_t* pItem, int iTileId)
{
	if ( pItem == NULL || iTileId < 0 || iTileId >= MAPEDIT_TILESET_ACTOR_OVERLAY_MAX ) {
		return (unsigned char)MAPEDIT_TILESET_ACTOR_OVERLAY_DEFAULT;
	}
	if ( iTileId >= pItem->iActorOverlayCount ) {
		return (unsigned char)MAPEDIT_TILESET_ACTOR_OVERLAY_DEFAULT;
	}
	return pItem->arrActorOverlay[iTileId] ? 1 : 0;
}

static void MapEditTilesetActorOverlayDrawStaticTexture(mapedit_app_t* pApp, xge_rect_t tRect)
{
	xge_draw_t tDraw;
	int iStaticStartRow;

	if ( pApp == NULL || pApp->pTilesetActorOverlayStaticTexture == NULL ||
	     pApp->tTilesetActorOverlayStaticDesc.iWidth <= 0 || pApp->tTilesetActorOverlayStaticDesc.iHeight <= 0 ) {
		return;
	}
	iStaticStartRow = MapEditTilesetActorOverlayStaticStart(pApp) / MapEditTilesetActorOverlayTilesPerRow(pApp);
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = (xge_texture)pApp->pTilesetActorOverlayStaticTexture;
	tDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, (float)pApp->tTilesetActorOverlayStaticDesc.iWidth, (float)pApp->tTilesetActorOverlayStaticDesc.iHeight };
	tDraw.tDst = (xge_rect_t){
		tRect.fX,
		tRect.fY + (float)(iStaticStartRow * MapEditTilesetActorOverlayCellHeight(pApp)),
		(float)pApp->tTilesetActorOverlayStaticDesc.iWidth,
		(float)pApp->tTilesetActorOverlayStaticDesc.iHeight
	};
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
	xgeFlush();
}

static void MapEditTilesetActorOverlayContentPaint(mapedit_tile_grid_t* pGrid, xge_rect_t tRect, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_tileset_set_item_t* pItem;
	xge_rect_t tStaticRect;
	int iStaticStartRow;
	int iCellH;
	const char* sText;

	(void)pGrid;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pItem = MapEditTilesetSetSelectedItem(pApp);
	if ( pApp->pTilesetActorOverlayStaticTexture != NULL ) {
		MapEditTilesetActorOverlayDrawStaticTexture(pApp, tRect);
		return;
	}
	iCellH = MapEditTilesetActorOverlayCellHeight(pApp);
	iStaticStartRow = MapEditTilesetActorOverlayStaticStart(pApp) / MapEditTilesetActorOverlayTilesPerRow(pApp);
	tStaticRect = (xge_rect_t){
		tRect.fX,
		tRect.fY + (float)(iStaticStartRow * iCellH),
		(float)(MapEditTilesetActorOverlayTilesPerRow(pApp) * MapEditTilesetActorOverlayCellWidth(pApp)),
		(float)(MapEditTilesetActorOverlayStaticRows(pApp) * iCellH)
	};
	xgeShapeRectFillPx(tStaticRect, XGE_COLOR_RGBA(235, 244, 250, 255));
	sText = (pItem == NULL) ? "未选择图集" : "静态图块集未设置或图片不可用";
	xgeTextDrawRect(pApp->bFontReady ? &pApp->tFont : NULL, sText, tStaticRect, XGE_COLOR_RGBA(104, 128, 148, 255),
		XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
}

static void MapEditTilesetActorOverlayDrawMarker(mapedit_tile_grid_t* pGrid, xge_rect_t tCell)
{
	float fShadow;

	fShadow = (tCell.fW < 20.0f || tCell.fH < 20.0f) ? 0.0f : 1.0f;
	if ( fShadow > 0.0f ) {
		xgeTextDrawRect(pGrid->pFont, "≈",
			(xge_rect_t){ tCell.fX + fShadow, tCell.fY + fShadow, tCell.fW, tCell.fH },
			XGE_COLOR_RGBA(248, 252, 255, 210),
			XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
	}
	xgeTextDrawRect(pGrid->pFont, "≈", tCell, XGE_COLOR_RGBA(28, 118, 78, 245),
		XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
}

static void MapEditTilesetActorOverlayCellPaint(mapedit_tile_grid_t* pGrid, int iCol, int iRow, xge_rect_t tCell, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_tileset_set_item_t* pItem;
	int iPerRow;
	int iTileId;
	int iStaticStart;
	int iSpecialCount;

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pItem = MapEditTilesetSetSelectedItem(pApp);
	iPerRow = MapEditTilesetActorOverlayTilesPerRow(pApp);
	iTileId = iRow * iPerRow + iCol;
	iSpecialCount = (pItem != NULL) ? pItem->iSpecialTileCount : 0;
	iStaticStart = MapEditTilesetActorOverlayStaticStart(pApp);
	if ( iTileId == 0 ) {
		xgeShapeRectFillPx(tCell, XGE_COLOR_RGBA(242, 248, 252, 235));
		xgeShapeRectStrokePx(tCell, 1.0f, XGE_COLOR_RGBA(98, 158, 108, 220));
		xgeTextDrawRect(pGrid->pFont, "0", tCell, XGE_COLOR_RGBA(48, 112, 62, 255),
			XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
		return;
	}
	if ( iTileId >= 1 && iTileId <= iSpecialCount ) {
		xgeShapeRectFillPx(tCell, XGE_COLOR_RGBA(204, 232, 250, 168));
		xgeShapeRectStrokePx(tCell, 1.0f, XGE_COLOR_RGBA(72, 150, 208, 210));
	}
	if ( iTileId > iSpecialCount && iTileId < iStaticStart ) {
		xgeShapeRectFillPx(tCell, XGE_COLOR_RGBA(224, 232, 238, 180));
		if ( pGrid->iCellWidth >= 20 && pGrid->iCellHeight >= 16 ) {
			xgeTextDrawRect(pGrid->pFont, "-", tCell, XGE_COLOR_RGBA(120, 136, 148, 255),
				XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
		}
		return;
	}
	if ( !MapEditTilesetActorOverlayTileEditable(pApp, pItem, iTileId) ) {
		return;
	}
	if ( MapEditTilesetActorOverlayValue(pItem, iTileId) ) {
		MapEditTilesetActorOverlayDrawMarker(pGrid, tCell);
	}
}

static void MapEditTilesetActorOverlayCellClick(mapedit_tile_grid_t* pGrid, int iCol, int iRow, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_tileset_set_item_t* pItem;
	char sStatus[128];
	int iTileId;
	int iPerRow;
	unsigned char iValue;

	(void)pGrid;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pItem = MapEditTilesetSetSelectedItem(pApp);
	iPerRow = MapEditTilesetActorOverlayTilesPerRow(pApp);
	iTileId = iRow * iPerRow + iCol;
	pApp->iTilesetActorOverlaySelectedTile = iTileId;
	if ( !MapEditTilesetActorOverlayTileEditable(pApp, pItem, iTileId) ) {
		snprintf(sStatus, sizeof(sStatus), "ID %d 不是可编辑角色覆盖图块", iTileId);
		MapEditAppSetStatus(pApp, sStatus);
		return;
	}
	MapEditTilesetSetEnsureActorOverlayCount(pItem, iTileId + 1);
	iValue = pItem->arrActorOverlay[iTileId] ? 1 : 0;
	pItem->arrActorOverlay[iTileId] = (unsigned char)(iValue ? 0 : 1);
	if ( MapEditTilesetSetSaveSelected(pApp) == XGE_OK ) {
		snprintf(sStatus, sizeof(sStatus), "角色覆盖数据已保存: ID %d = %s", iTileId, pItem->arrActorOverlay[iTileId] ? "覆盖" : "不覆盖");
	} else {
		snprintf(sStatus, sizeof(sStatus), "角色覆盖数据已修改，但保存失败: ID %d", iTileId);
	}
	MapEditAppSetStatus(pApp, sStatus);
	if ( pApp->pTilesetActorOverlayGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pTilesetActorOverlayGridWidget);
	}
}

void MapEditTilesetActorOverlayRefresh(mapedit_app_t* pApp)
{
	mapedit_tileset_set_item_t* pItem;
	int iPerRow;
	int iCellW;
	int iCellH;
	int iStaticStart;
	int iStaticRows;
	int iRows;

	if ( pApp == NULL || pApp->pTilesetActorOverlayGridWidget == NULL ) {
		return;
	}
	pItem = MapEditTilesetSetSelectedItem(pApp);
	iPerRow = MapEditTilesetActorOverlayTilesPerRow(pApp);
	iCellW = MapEditTilesetActorOverlayCellWidth(pApp);
	iCellH = MapEditTilesetActorOverlayCellHeight(pApp);
	MapEditTilesetActorOverlayLoadStaticTexture(pApp, pItem);
	iStaticStart = MapEditTilesetActorOverlayStaticStart(pApp);
	iStaticRows = MapEditTilesetActorOverlayStaticRows(pApp);
	iRows = (iStaticStart / iPerRow) + iStaticRows;
	if ( iRows < MAPEDIT_TILESET_ACTOR_OVERLAY_PLACEHOLDER_ROWS ) {
		iRows = MAPEDIT_TILESET_ACTOR_OVERLAY_PLACEHOLDER_ROWS;
	}
	MapEditTileGridSetCellSize(&pApp->tTilesetActorOverlayGrid, iCellW, iCellH);
	MapEditTileGridSetGridSize(&pApp->tTilesetActorOverlayGrid, iPerRow, iRows);
	MapEditTileGridSetMaxCells(&pApp->tTilesetActorOverlayGrid, iPerRow * iRows);
	pApp->tTilesetActorOverlayGrid.bHasSelection = 0;
	pApp->tTilesetActorOverlayGrid.iHoverCol = -1;
	pApp->tTilesetActorOverlayGrid.iHoverRow = -1;
	pApp->iTilesetActorOverlaySelectedTile = -1;
	if ( pApp->tTilesetActorOverlayScrollFrame.pWidget != NULL ) {
		xgeXuiScrollFrameSetOffset(&pApp->tTilesetActorOverlayScrollFrame, 0.0f, 0.0f);
		MapEditTilesetActorOverlayUpdateAndSyncScrollContent(pApp);
	}
	xgeXuiWidgetMarkLayout(pApp->pTilesetActorOverlayCanvasWidget);
	xgeXuiWidgetMarkPaint(pApp->pTilesetActorOverlayGridWidget);
}

int MapEditTilesetActorOverlayHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent)
{
	xge_rect_t tViewport;

	if ( pApp == NULL || pEvent == NULL || pApp->pTilesetActorOverlayGridWidget == NULL || !MapEditTilesetActorOverlayMouseGridEvent(pEvent) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( !MapEditTilesetActorOverlayWidgetInteractive(pApp->pTilesetActorOverlayGridWidget) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tTilesetActorOverlayScrollFrame);
	if ( pApp->tTilesetActorOverlayGrid.bDragging ||
	     (MapEditTilesetActorOverlayRectContains(tViewport, pEvent->fX, pEvent->fY) &&
	      MapEditTilesetActorOverlayRectContains(pApp->pTilesetActorOverlayGridWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		return MapEditTileGridEventProc(pApp->pTilesetActorOverlayGridWidget, pEvent, &pApp->tTilesetActorOverlayGrid);
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int MapEditTilesetActorOverlayCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
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
	xgeXuiScrollModelInit(&pApp->tTilesetActorOverlayScrollModel);
	if ( xgeXuiScrollFrameInit(&pApp->tTilesetActorOverlayScrollFrame, &pApp->tXui, pCanvasWidget, &pApp->tTilesetActorOverlayScrollModel) != XGE_OK ) {
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	xgeXuiWidgetSetEvent(pCanvasWidget, xgeXuiScrollFrameEventProc, &pApp->tTilesetActorOverlayScrollFrame);
	xgeXuiWidgetSetLayoutProc(pCanvasWidget, MapEditTilesetActorOverlayScrollLayoutProc, pApp);
	xgeXuiWidgetSetLayout(xgeXuiScrollFrameGetViewportWidget(&pApp->tTilesetActorOverlayScrollFrame), XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiScrollFrameSetChange(&pApp->tTilesetActorOverlayScrollFrame, MapEditTilesetActorOverlayScrollChanged, pApp);
	xgeXuiScrollFrameSetScrollbarMode(&pApp->tTilesetActorOverlayScrollFrame, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiScrollFrameSetWheelAxis(&pApp->tTilesetActorOverlayScrollFrame, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollFrameSetContentDragEnabled(&pApp->tTilesetActorOverlayScrollFrame, 0);
	xgeXuiScrollFrameSetMetrics(&pApp->tTilesetActorOverlayScrollFrame, MAPEDIT_TILESET_ACTOR_OVERLAY_SCROLLBAR_SIZE, 28.0f, 0.0f, 0.0f);
	xgeXuiScrollFrameSetColors(&pApp->tTilesetActorOverlayScrollFrame, XGE_COLOR_RGBA(226, 240, 249, 255), XGE_COLOR_RGBA(90, 164, 214, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(68, 142, 204, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(180, 204, 218, 255));
	if ( xgeXuiWidgetAdd(xgeXuiScrollFrameGetViewportWidget(&pApp->tTilesetActorOverlayScrollFrame), pGridWidget) != XGE_OK ||
	     MapEditTileGridInit(&pApp->tTilesetActorOverlayGrid, pGridWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		xgeXuiScrollFrameUnit(&pApp->tTilesetActorOverlayScrollFrame);
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	MapEditTileGridSetExpand(&pApp->tTilesetActorOverlayGrid, 0, 0);
	MapEditTileGridSetColors(&pApp->tTilesetActorOverlayGrid,
		XGE_COLOR_RGBA(236, 246, 252, 255),
		XGE_COLOR_RGBA(124, 181, 219, 255),
		XGE_COLOR_RGBA(188, 224, 244, 150),
		XGE_COLOR_RGBA(102, 181, 230, 8),
		XGE_COLOR_RGBA(42, 140, 210, 105),
		XGE_COLOR_RGBA(104, 128, 148, 255));
	MapEditTileGridSetInteractionPaint(&pApp->tTilesetActorOverlayGrid, 0, 0);
	MapEditTileGridSetEmptyText(&pApp->tTilesetActorOverlayGrid, "");
	MapEditTileGridSetContentPaint(&pApp->tTilesetActorOverlayGrid, MapEditTilesetActorOverlayContentPaint, pApp);
	MapEditTileGridSetCellPaint(&pApp->tTilesetActorOverlayGrid, MapEditTilesetActorOverlayCellPaint, pApp);
	MapEditTileGridSetCellClick(&pApp->tTilesetActorOverlayGrid, MapEditTilesetActorOverlayCellClick, pApp);
	pApp->pTilesetActorOverlayCanvasWidget = pCanvasWidget;
	pApp->pTilesetActorOverlayGridWidget = pGridWidget;
	if ( xgeXuiWidgetAdd(pContent, pCanvasWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	MapEditTilesetActorOverlayRefresh(pApp);
	return XGE_OK;
}

void MapEditTilesetActorOverlayUnit(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	MapEditTilesetActorOverlayClearStaticTexture(pApp);
	if ( pApp->pTilesetActorOverlayGridWidget != NULL ) {
		MapEditTileGridUnit(&pApp->tTilesetActorOverlayGrid);
		pApp->pTilesetActorOverlayGridWidget = NULL;
	}
	if ( pApp->pTilesetActorOverlayCanvasWidget != NULL ) {
		xgeXuiScrollFrameUnit(&pApp->tTilesetActorOverlayScrollFrame);
		pApp->pTilesetActorOverlayCanvasWidget = NULL;
	}
}
