#include "map_tile_select.h"
#include "map_maps.h"
#include "map_edit.h"
#include <stdio.h>
#include <string.h>

#define MAPEDIT_MAP_TILE_SELECT_SCROLLBAR_SIZE 12.0f
#define MAPEDIT_MAP_TILE_SELECT_PLACEHOLDER_ROWS 8

typedef struct mapedit_map_tile_select_special_dir_t {
	const char* sType;
	const char* sDir;
} mapedit_map_tile_select_special_dir_t;

static const mapedit_map_tile_select_special_dir_t g_arrMapTileSelectSpecialDirs[] = {
	{ "动态图块", "animated_tiles" },
	{ "自动图块", "autotiles" },
	{ "多状态图块", "state_tiles" },
	{ "多状态自动图块", "state_autotiles" },
	{ "对象", "objects" }
};

static float MapEditMapTileSelectAbsFloat(float fValue)
{
	return (fValue < 0.0f) ? -fValue : fValue;
}

static int MapEditMapTileSelectRectAlmostEqual(xge_rect_t a, xge_rect_t b)
{
	return MapEditMapTileSelectAbsFloat(a.fX - b.fX) < 0.01f &&
		MapEditMapTileSelectAbsFloat(a.fY - b.fY) < 0.01f &&
		MapEditMapTileSelectAbsFloat(a.fW - b.fW) < 0.01f &&
		MapEditMapTileSelectAbsFloat(a.fH - b.fH) < 0.01f;
}

static int MapEditMapTileSelectRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int MapEditMapTileSelectWidgetInteractive(xge_xui_widget pWidget)
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

static int MapEditMapTileSelectMouseGridEvent(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return (pEvent->iType == XGE_EVENT_MOUSE_DOWN && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_MOVE);
}

static int MapEditMapTileSelectStaticStart(mapedit_app_t* pApp)
{
	return MapEditMapStaticStart(pApp, MapEditMapSelectedTileset(pApp));
}

static int MapEditMapTileSelectSpecialDirCount(void)
{
	return (int)(sizeof(g_arrMapTileSelectSpecialDirs) / sizeof(g_arrMapTileSelectSpecialDirs[0]));
}

static void MapEditMapTileSelectClearSpecialTexture(mapedit_app_t* pApp, int iSlot)
{
	if ( pApp == NULL || iSlot < 0 || iSlot >= MAPEDIT_TILESET_SPECIAL_TILE_MAX ) {
		return;
	}
	if ( pApp->arrMapTilesetSpecialTexture[iSlot] != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pApp->arrMapTilesetSpecialTexture[iSlot]);
		pApp->arrMapTilesetSpecialTexture[iSlot] = NULL;
	}
	memset(&pApp->arrMapTilesetSpecialDesc[iSlot], 0, sizeof(pApp->arrMapTilesetSpecialDesc[iSlot]));
}

static void MapEditMapTileSelectClearSpecialTextures(mapedit_app_t* pApp)
{
	int i;

	if ( pApp == NULL ) {
		return;
	}
	for ( i = 0; i < MAPEDIT_TILESET_SPECIAL_TILE_MAX; i++ ) {
		MapEditMapTileSelectClearSpecialTexture(pApp, i);
	}
}

static int MapEditMapTileSelectSpecialTypeDirIndex(const char* sType)
{
	int i;

	if ( sType == NULL || sType[0] == 0 ) {
		return -1;
	}
	for ( i = 0; i < MapEditMapTileSelectSpecialDirCount(); i++ ) {
		if ( strcmp(g_arrMapTileSelectSpecialDirs[i].sType, sType) == 0 ) {
			return i;
		}
	}
	return -1;
}

static int MapEditMapTileSelectTryLoadSpecialTexture(mapedit_app_t* pApp, int iSlot, const char* sDir, const char* sFile)
{
	str sPath;
	xui_texture pTexture;
	xui_texture_desc_t tDesc;

	if ( pApp == NULL || iSlot < 0 || iSlot >= MAPEDIT_TILESET_SPECIAL_TILE_MAX || sDir == NULL || sFile == NULL || sFile[0] == 0 ) {
		return 0;
	}
	sPath = xrtPathJoin(4, xCore.AppPath, (str)"assets", (str)sDir, (str)sFile);
	if ( sPath == NULL ) {
		return 0;
	}
	if ( !xrtFileExists(sPath) ) {
		xrtFree(sPath);
		return 0;
	}
	pTexture = NULL;
	memset(&tDesc, 0, sizeof(tDesc));
	if ( xgeXuiTextureCreateFile(&pApp->tXui, (const char*)sPath, XGE_IMAGE_STRAIGHT_ALPHA, &pTexture) == XGE_OK &&
	     xgeXuiTextureGetDesc(&pApp->tXui, pTexture, &tDesc) == XGE_OK &&
	     tDesc.iWidth > 0 && tDesc.iHeight > 0 ) {
		pApp->arrMapTilesetSpecialTexture[iSlot] = pTexture;
		pApp->arrMapTilesetSpecialDesc[iSlot] = tDesc;
		xrtFree(sPath);
		return 1;
	}
	if ( pTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pTexture);
	}
	xrtFree(sPath);
	return 0;
}

static void MapEditMapTileSelectLoadSpecialTexture(mapedit_app_t* pApp, const mapedit_tileset_special_tile_t* pSpecial, int iSlot)
{
	int iTypeDir;
	int i;

	if ( pApp == NULL || pSpecial == NULL || iSlot < 0 || iSlot >= MAPEDIT_TILESET_SPECIAL_TILE_MAX || pSpecial->sFile[0] == 0 ) {
		return;
	}
	iTypeDir = MapEditMapTileSelectSpecialTypeDirIndex(pSpecial->sType);
	if ( iTypeDir >= 0 && MapEditMapTileSelectTryLoadSpecialTexture(pApp, iSlot, g_arrMapTileSelectSpecialDirs[iTypeDir].sDir, pSpecial->sFile) ) {
		return;
	}
	for ( i = 0; i < MapEditMapTileSelectSpecialDirCount(); i++ ) {
		if ( i == iTypeDir ) {
			continue;
		}
		if ( MapEditMapTileSelectTryLoadSpecialTexture(pApp, iSlot, g_arrMapTileSelectSpecialDirs[i].sDir, pSpecial->sFile) ) {
			return;
		}
	}
}

static void MapEditMapTileSelectReloadSpecialTextures(mapedit_app_t* pApp)
{
	mapedit_tileset_set_item_t* pTileset;
	int i;
	int iCount;

	if ( pApp == NULL ) {
		return;
	}
	MapEditMapTileSelectClearSpecialTextures(pApp);
	pTileset = MapEditMapSelectedTileset(pApp);
	if ( pTileset == NULL ) {
		return;
	}
	iCount = pTileset->iSpecialTileCount;
	if ( iCount > MAPEDIT_TILESET_SPECIAL_TILE_MAX ) {
		iCount = MAPEDIT_TILESET_SPECIAL_TILE_MAX;
	}
	for ( i = 0; i < iCount; i++ ) {
		MapEditMapTileSelectLoadSpecialTexture(pApp, &pTileset->arrSpecialTiles[i], i);
	}
}

static int MapEditMapTileSelectRowCount(mapedit_app_t* pApp)
{
	int iPerRow;
	int iStaticStart;
	int iRows;

	if ( pApp == NULL ) {
		return MAPEDIT_MAP_TILE_SELECT_PLACEHOLDER_ROWS;
	}
	iPerRow = MapEditMapTilesPerRow(pApp);
	iStaticStart = MapEditMapTileSelectStaticStart(pApp);
	iRows = (iStaticStart / iPerRow) + MapEditMapStaticRows(pApp);
	if ( iRows < MAPEDIT_MAP_TILE_SELECT_PLACEHOLDER_ROWS ) {
		iRows = MAPEDIT_MAP_TILE_SELECT_PLACEHOLDER_ROWS;
	}
	return iRows;
}

static float MapEditMapTileSelectContentWidth(mapedit_app_t* pApp)
{
	return (float)(MapEditMapTilesPerRow(pApp) * MapEditMapTileWidth(pApp));
}

static float MapEditMapTileSelectContentHeight(mapedit_app_t* pApp)
{
	return (float)(MapEditMapTileSelectRowCount(pApp) * MapEditMapTileHeight(pApp));
}

static void MapEditMapTileSelectSetScrollContentRect(xge_xui_widget pWidget, xge_rect_t tLocal, xge_rect_t tRect)
{
	int bSizeChanged;

	if ( pWidget == NULL ) {
		return;
	}
	if ( MapEditMapTileSelectRectAlmostEqual(pWidget->tLocalRect, tLocal) && MapEditMapTileSelectRectAlmostEqual(pWidget->tRect, tRect) ) {
		return;
	}
	bSizeChanged = pWidget->tStyle.tWidth.iUnit != XGE_XUI_SIZE_PX ||
		pWidget->tStyle.tHeight.iUnit != XGE_XUI_SIZE_PX ||
		MapEditMapTileSelectAbsFloat(pWidget->tStyle.tWidth.fValue - tLocal.fW) >= 0.01f ||
		MapEditMapTileSelectAbsFloat(pWidget->tStyle.tHeight.fValue - tLocal.fH) >= 0.01f;
	if ( bSizeChanged ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(tLocal.fW), xgeXuiSizePx(tLocal.fH));
	}
	xgeXuiWidgetSetRect(pWidget, tRect);
	pWidget->tLocalRect = tLocal;
}

static void MapEditMapTileSelectUpdateScrollContentSize(mapedit_app_t* pApp)
{
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tMapTileSelectScrollFrame.pWidget == NULL ) {
		return;
	}
	fContentW = MapEditMapTileSelectContentWidth(pApp);
	fContentH = MapEditMapTileSelectContentHeight(pApp);
	if ( MapEditMapTileSelectAbsFloat(pApp->tMapTileSelectScrollModel.fContentW - fContentW) >= 0.01f ||
	     MapEditMapTileSelectAbsFloat(pApp->tMapTileSelectScrollModel.fContentH - fContentH) >= 0.01f ) {
		xgeXuiScrollFrameSetContentSize(&pApp->tMapTileSelectScrollFrame, fContentW, fContentH);
	}
}

static void MapEditMapTileSelectSyncScrollContent(mapedit_app_t* pApp)
{
	xge_rect_t tViewport;
	xge_rect_t tLocal;
	xge_rect_t tRect;
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tMapTileSelectScrollFrame.pWidget == NULL || pApp->pMapTileSelectGridWidget == NULL ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tMapTileSelectScrollFrame);
	fContentW = (pApp->tMapTileSelectScrollModel.fContentW > 1.0f) ? pApp->tMapTileSelectScrollModel.fContentW : MapEditMapTileSelectContentWidth(pApp);
	fContentH = (pApp->tMapTileSelectScrollModel.fContentH > 1.0f) ? pApp->tMapTileSelectScrollModel.fContentH : MapEditMapTileSelectContentHeight(pApp);
	tLocal.fX = -pApp->tMapTileSelectScrollModel.fScrollX;
	tLocal.fY = -pApp->tMapTileSelectScrollModel.fScrollY;
	tLocal.fW = (fContentW > tViewport.fW) ? fContentW : tViewport.fW;
	tLocal.fH = (fContentH > tViewport.fH) ? fContentH : tViewport.fH;
	tRect.fX = tViewport.fX + tLocal.fX;
	tRect.fY = tViewport.fY + tLocal.fY;
	tRect.fW = tLocal.fW;
	tRect.fH = tLocal.fH;
	MapEditMapTileSelectSetScrollContentRect(pApp->pMapTileSelectGridWidget, tLocal, tRect);
}

static void MapEditMapTileSelectUpdateAndSyncScrollContent(mapedit_app_t* pApp)
{
	MapEditMapTileSelectUpdateScrollContentSize(pApp);
	MapEditMapTileSelectSyncScrollContent(pApp);
}

static void MapEditMapTileSelectScrollChanged(xge_xui_scroll_frame pFrame, float fScrollX, float fScrollY, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pFrame;
	(void)fScrollX;
	(void)fScrollY;
	pApp = (mapedit_app_t*)pUser;
	MapEditMapTileSelectSyncScrollContent(pApp);
}

static void MapEditMapTileSelectScrollLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiScrollFrameLayout(&pApp->tMapTileSelectScrollFrame);
	MapEditMapTileSelectSyncScrollContent(pApp);
}

static int MapEditMapTileSelectTileSelectable(mapedit_app_t* pApp, int iTileId)
{
	mapedit_tileset_set_item_t* pTileset;
	int iSpecialCount;
	int iStaticStart;
	int iLocal;
	int iRow;
	int iCol;

	if ( pApp == NULL || iTileId < 0 ) {
		return 0;
	}
	pTileset = MapEditMapSelectedTileset(pApp);
	iSpecialCount = (pTileset != NULL) ? pTileset->iSpecialTileCount : 0;
	if ( iTileId == 0 ) {
		return 1;
	}
	if ( iTileId >= 1 && iTileId <= iSpecialCount ) {
		return 1;
	}
	iStaticStart = MapEditMapTileSelectStaticStart(pApp);
	if ( iTileId < iStaticStart || pApp->pMapTilesetStaticTexture == NULL ) {
		return 0;
	}
	iLocal = iTileId - iStaticStart;
	iRow = iLocal / MapEditMapTilesPerRow(pApp);
	iCol = iLocal % MapEditMapTilesPerRow(pApp);
	return (iRow >= 0 && iRow < MapEditMapStaticRows(pApp) && iCol >= 0 && iCol < MapEditMapStaticCols(pApp));
}

static void MapEditMapTileSelectContentPaint(mapedit_tile_grid_t* pGrid, xge_rect_t tRect, void* pUser)
{
	mapedit_app_t* pApp;
	xge_rect_t tStaticRect;
	int iStaticStartRow;
	int iCellH;

	(void)pGrid;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	iCellH = MapEditMapTileHeight(pApp);
	iStaticStartRow = MapEditMapTileSelectStaticStart(pApp) / MapEditMapTilesPerRow(pApp);
	tStaticRect = (xge_rect_t){
		tRect.fX,
		tRect.fY + (float)(iStaticStartRow * iCellH),
		(float)(MapEditMapTilesPerRow(pApp) * MapEditMapTileWidth(pApp)),
		(float)(MapEditMapStaticRows(pApp) * iCellH)
	};
	if ( pApp->pMapTilesetStaticTexture != NULL && pApp->tMapTilesetStaticDesc.iWidth > 0 && pApp->tMapTilesetStaticDesc.iHeight > 0 ) {
		xge_draw_t tDraw;

		memset(&tDraw, 0, sizeof(tDraw));
		tDraw.pTexture = (xge_texture)pApp->pMapTilesetStaticTexture;
		tDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, (float)pApp->tMapTilesetStaticDesc.iWidth, (float)pApp->tMapTilesetStaticDesc.iHeight };
		tDraw.tDst = (xge_rect_t){ tStaticRect.fX, tStaticRect.fY, (float)pApp->tMapTilesetStaticDesc.iWidth, (float)pApp->tMapTilesetStaticDesc.iHeight };
		tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
		xgeDrawEx(&tDraw);
		xgeFlush();
	} else {
		xgeShapeRectFillPx(tStaticRect, XGE_COLOR_RGBA(235, 244, 250, 255));
		xgeTextDrawRect(pApp->bFontReady ? &pApp->tFont : NULL, "当前地图未设置可用静态图块集", tStaticRect, XGE_COLOR_RGBA(104, 128, 148, 255),
			XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
	}
}

static int MapEditMapTileSelectDrawSpecialPreview(mapedit_app_t* pApp, int iSlot, xge_rect_t tCell)
{
	xui_texture_desc_t* pDesc;
	xge_draw_t tDraw;
	float fSrcW;
	float fSrcH;

	if ( pApp == NULL || iSlot < 0 || iSlot >= MAPEDIT_TILESET_SPECIAL_TILE_MAX ||
	     pApp->arrMapTilesetSpecialTexture[iSlot] == NULL ) {
		return 0;
	}
	pDesc = &pApp->arrMapTilesetSpecialDesc[iSlot];
	if ( pDesc->iWidth <= 0 || pDesc->iHeight <= 0 ) {
		return 0;
	}
	fSrcW = (float)MapEditMapTileWidth(pApp);
	fSrcH = (float)MapEditMapTileHeight(pApp);
	if ( fSrcW > (float)pDesc->iWidth ) {
		fSrcW = (float)pDesc->iWidth;
	}
	if ( fSrcH > (float)pDesc->iHeight ) {
		fSrcH = (float)pDesc->iHeight;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = (xge_texture)pApp->arrMapTilesetSpecialTexture[iSlot];
	tDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, fSrcW, fSrcH };
	tDraw.tDst = tCell;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
	xgeFlush();
	return 1;
}

static void MapEditMapTileSelectCellPaint(mapedit_tile_grid_t* pGrid, int iCol, int iRow, xge_rect_t tCell, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_tileset_set_item_t* pTileset;
	int iTileId;
	int iPerRow;
	int iSpecialCount;
	int iStaticStart;

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pTileset = MapEditMapSelectedTileset(pApp);
	iPerRow = MapEditMapTilesPerRow(pApp);
	iTileId = iRow * iPerRow + iCol;
	iSpecialCount = (pTileset != NULL) ? pTileset->iSpecialTileCount : 0;
	iStaticStart = MapEditMapTileSelectStaticStart(pApp);
	if ( iTileId == 0 ) {
		xgeShapeRectFillPx(tCell, XGE_COLOR_RGBA(242, 248, 252, 235));
		xgeShapeRectStrokePx(tCell, 1.0f, XGE_COLOR_RGBA(98, 158, 108, 220));
		xgeTextDrawRect(pGrid->pFont, "0", tCell, XGE_COLOR_RGBA(48, 112, 62, 255),
			XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
		return;
	}
	if ( iTileId >= 1 && iTileId <= iSpecialCount ) {
		uint32_t iFill;
		int iSlot;
		int bConfigured;

		iSlot = iTileId - 1;
		bConfigured = (pTileset != NULL &&
			(pTileset->arrSpecialTiles[iSlot].sFile[0] != 0 || pTileset->arrSpecialTiles[iSlot].sType[0] != 0));
		iFill = bConfigured ?
			XGE_COLOR_RGBA(246, 218, 158, 205) : XGE_COLOR_RGBA(204, 232, 250, 205);
		xgeShapeRectFillPx(tCell, iFill);
		xgeShapeRectStrokePx(tCell, 1.0f, XGE_COLOR_RGBA(72, 150, 208, 230));
		if ( !MapEditMapTileSelectDrawSpecialPreview(pApp, iSlot, tCell) && bConfigured ) {
			xgeTextDrawRect(pGrid->pFont, "!", tCell, XGE_COLOR_RGBA(168, 82, 42, 255),
				XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
		}
		return;
	}
	if ( iTileId > iSpecialCount && iTileId < iStaticStart ) {
		xgeShapeRectFillPx(tCell, XGE_COLOR_RGBA(224, 232, 238, 180));
		if ( pGrid->iCellWidth >= 20 && pGrid->iCellHeight >= 16 ) {
			xgeTextDrawRect(pGrid->pFont, "-", tCell, XGE_COLOR_RGBA(120, 136, 148, 255),
				XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
		}
	}
}

static void MapEditMapTileSelectSelectionChange(mapedit_tile_grid_t* pGrid, int iCol, int iRow, int iCols, int iRows, void* pUser)
{
	mapedit_app_t* pApp;
	char sStatus[128];
	int iTileId;

	(void)pGrid;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	iTileId = iRow * MapEditMapTilesPerRow(pApp) + iCol;
	if ( !MapEditMapTileSelectTileSelectable(pApp, iTileId) ) {
		pApp->iMapBrushTileId = -1;
		MapEditAppSetStatus(pApp, "当前图块位置不可作为地图画笔");
		return;
	}
	pApp->iMapBrushTileId = iTileId;
	pApp->iMapBrushCols = (iCols > 0) ? iCols : 1;
	pApp->iMapBrushRows = (iRows > 0) ? iRows : 1;
	snprintf(sStatus, sizeof(sStatus), "已选择图块: ID %d / %d x %d", iTileId, pApp->iMapBrushCols, pApp->iMapBrushRows);
	MapEditAppSetStatus(pApp, sStatus);
	if ( pApp->pMapEditGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pMapEditGridWidget);
	}
}

void MapEditMapTileSelectRefresh(mapedit_app_t* pApp)
{
	int iPerRow;
	int iRows;

	if ( pApp == NULL || pApp->pMapTileSelectGridWidget == NULL ) {
		return;
	}
	iPerRow = MapEditMapTilesPerRow(pApp);
	iRows = MapEditMapTileSelectRowCount(pApp);
	MapEditMapTileSelectReloadSpecialTextures(pApp);
	MapEditTileGridSetCellSize(&pApp->tMapTileSelectGrid, MapEditMapTileWidth(pApp), MapEditMapTileHeight(pApp));
	MapEditTileGridSetGridSize(&pApp->tMapTileSelectGrid, iPerRow, iRows);
	MapEditTileGridSetMaxCells(&pApp->tMapTileSelectGrid, iPerRow * iRows);
	pApp->tMapTileSelectGrid.bHasSelection = 0;
	pApp->tMapTileSelectGrid.iHoverCol = -1;
	pApp->tMapTileSelectGrid.iHoverRow = -1;
	if ( pApp->tMapTileSelectScrollFrame.pWidget != NULL ) {
		xgeXuiScrollFrameSetOffset(&pApp->tMapTileSelectScrollFrame, 0.0f, 0.0f);
		MapEditMapTileSelectUpdateAndSyncScrollContent(pApp);
	}
	xgeXuiWidgetMarkLayout(pApp->pMapTileSelectCanvasWidget);
	xgeXuiWidgetMarkPaint(pApp->pMapTileSelectGridWidget);
}

int MapEditMapTileSelectHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent)
{
	xge_rect_t tViewport;

	if ( pApp == NULL || pEvent == NULL || pApp->pMapTileSelectGridWidget == NULL || !MapEditMapTileSelectMouseGridEvent(pEvent) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( !MapEditMapTileSelectWidgetInteractive(pApp->pMapTileSelectGridWidget) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tMapTileSelectScrollFrame);
	if ( pApp->tMapTileSelectGrid.bDragging ||
	     (MapEditMapTileSelectRectContains(tViewport, pEvent->fX, pEvent->fY) &&
	      MapEditMapTileSelectRectContains(pApp->pMapTileSelectGridWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		return MapEditTileGridEventProc(pApp->pMapTileSelectGridWidget, pEvent, &pApp->tMapTileSelectGrid);
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int MapEditMapTileSelectCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
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
	xgeXuiScrollModelInit(&pApp->tMapTileSelectScrollModel);
	if ( xgeXuiScrollFrameInit(&pApp->tMapTileSelectScrollFrame, &pApp->tXui, pCanvasWidget, &pApp->tMapTileSelectScrollModel) != XGE_OK ) {
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	xgeXuiWidgetSetEvent(pCanvasWidget, xgeXuiScrollFrameEventProc, &pApp->tMapTileSelectScrollFrame);
	xgeXuiWidgetSetLayoutProc(pCanvasWidget, MapEditMapTileSelectScrollLayoutProc, pApp);
	xgeXuiWidgetSetLayout(xgeXuiScrollFrameGetViewportWidget(&pApp->tMapTileSelectScrollFrame), XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiScrollFrameSetChange(&pApp->tMapTileSelectScrollFrame, MapEditMapTileSelectScrollChanged, pApp);
	xgeXuiScrollFrameSetScrollbarMode(&pApp->tMapTileSelectScrollFrame, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiScrollFrameSetWheelAxis(&pApp->tMapTileSelectScrollFrame, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollFrameSetContentDragEnabled(&pApp->tMapTileSelectScrollFrame, 0);
	xgeXuiScrollFrameSetMetrics(&pApp->tMapTileSelectScrollFrame, MAPEDIT_MAP_TILE_SELECT_SCROLLBAR_SIZE, 28.0f, 0.0f, 0.0f);
	xgeXuiScrollFrameSetColors(&pApp->tMapTileSelectScrollFrame, XGE_COLOR_RGBA(226, 240, 249, 255), XGE_COLOR_RGBA(90, 164, 214, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(68, 142, 204, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(180, 204, 218, 255));
	if ( xgeXuiWidgetAdd(xgeXuiScrollFrameGetViewportWidget(&pApp->tMapTileSelectScrollFrame), pGridWidget) != XGE_OK ||
	     MapEditTileGridInit(&pApp->tMapTileSelectGrid, pGridWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		xgeXuiScrollFrameUnit(&pApp->tMapTileSelectScrollFrame);
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	MapEditTileGridSetExpand(&pApp->tMapTileSelectGrid, 0, 0);
	MapEditTileGridSetColors(&pApp->tMapTileSelectGrid,
		XGE_COLOR_RGBA(236, 246, 252, 255),
		XGE_COLOR_RGBA(124, 181, 219, 255),
		XGE_COLOR_RGBA(188, 224, 244, 150),
		XGE_COLOR_RGBA(102, 181, 230, 8),
		XGE_COLOR_RGBA(42, 140, 210, 105),
		XGE_COLOR_RGBA(104, 128, 148, 255));
	MapEditTileGridSetEmptyText(&pApp->tMapTileSelectGrid, "");
	MapEditTileGridSetContentPaint(&pApp->tMapTileSelectGrid, MapEditMapTileSelectContentPaint, pApp);
	MapEditTileGridSetCellPaint(&pApp->tMapTileSelectGrid, MapEditMapTileSelectCellPaint, pApp);
	MapEditTileGridSetSelectionChange(&pApp->tMapTileSelectGrid, MapEditMapTileSelectSelectionChange, pApp);
	pApp->pMapTileSelectCanvasWidget = pCanvasWidget;
	pApp->pMapTileSelectGridWidget = pGridWidget;
	if ( xgeXuiWidgetAdd(pContent, pCanvasWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	MapEditMapEnsureDefaults(pApp);
	MapEditMapTileSelectRefresh(pApp);
	return XGE_OK;
}

void MapEditMapTileSelectUnit(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	MapEditMapTileSelectClearSpecialTextures(pApp);
	if ( pApp->pMapTileSelectGridWidget != NULL ) {
		MapEditTileGridUnit(&pApp->tMapTileSelectGrid);
		pApp->pMapTileSelectGridWidget = NULL;
	}
	if ( pApp->pMapTileSelectCanvasWidget != NULL ) {
		xgeXuiScrollFrameUnit(&pApp->tMapTileSelectScrollFrame);
		pApp->pMapTileSelectCanvasWidget = NULL;
	}
}
