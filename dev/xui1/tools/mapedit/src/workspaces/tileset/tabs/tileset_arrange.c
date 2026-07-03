#include "tileset_arrange.h"
#include "tileset_sets.h"
#include <stdio.h>
#include <string.h>

#define MAPEDIT_TILESET_ARRANGE_SCROLLBAR_SIZE 12.0f
#define MAPEDIT_TILESET_ARRANGE_PLACEHOLDER_ROWS 8

typedef struct mapedit_tileset_arrange_special_dir_t {
	const char* sType;
	const char* sDir;
} mapedit_tileset_arrange_special_dir_t;

static const mapedit_tileset_arrange_special_dir_t g_arrArrangeSpecialDirs[] = {
	{ "动态图块", "animated_tiles" },
	{ "自动图块", "autotiles" },
	{ "多状态图块", "state_tiles" },
	{ "多状态自动图块", "state_autotiles" },
	{ "对象", "objects" }
};

static float MapEditTilesetArrangeAbsFloat(float fValue)
{
	return (fValue < 0.0f) ? -fValue : fValue;
}

static int MapEditTilesetArrangeRectAlmostEqual(xge_rect_t a, xge_rect_t b)
{
	return MapEditTilesetArrangeAbsFloat(a.fX - b.fX) < 0.01f &&
		MapEditTilesetArrangeAbsFloat(a.fY - b.fY) < 0.01f &&
		MapEditTilesetArrangeAbsFloat(a.fW - b.fW) < 0.01f &&
		MapEditTilesetArrangeAbsFloat(a.fH - b.fH) < 0.01f;
}

static int MapEditTilesetArrangeRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static int MapEditTilesetArrangeWidgetInteractive(xge_xui_widget pWidget)
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

static int MapEditTilesetArrangeMouseGridEvent(const xge_event_t* pEvent)
{
	if ( pEvent == NULL ) {
		return 0;
	}
	return (pEvent->iType == XGE_EVENT_MOUSE_DOWN && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_UP && pEvent->iParam1 == XGE_MOUSE_LEFT) ||
		(pEvent->iType == XGE_EVENT_MOUSE_MOVE);
}

static mapedit_tileset_set_item_t* MapEditTilesetArrangeSelectedItem(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iTilesetSetSelected < 0 || pApp->iTilesetSetSelected >= pApp->iTilesetSetCount ) {
		return NULL;
	}
	return &pApp->arrTilesetSetItems[pApp->iTilesetSetSelected];
}

static int MapEditTilesetArrangeTilesPerRow(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTilesPerRow <= 0 ) {
		return 20;
	}
	return pApp->iSetupTilesPerRow;
}

static int MapEditTilesetArrangeCellWidth(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTileWidth <= 0 ) {
		return 16;
	}
	return pApp->iSetupTileWidth;
}

static int MapEditTilesetArrangeCellHeight(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTileHeight <= 0 ) {
		return 16;
	}
	return pApp->iSetupTileHeight;
}

static int MapEditTilesetArrangeStaticStart(mapedit_app_t* pApp)
{
	mapedit_tileset_set_item_t* pItem;
	int iPerRow;
	int iReserved;

	pItem = MapEditTilesetArrangeSelectedItem(pApp);
	iPerRow = MapEditTilesetArrangeTilesPerRow(pApp);
	iReserved = 1 + ((pItem != NULL) ? pItem->iSpecialTileCount : 0);
	if ( iReserved < 1 ) {
		iReserved = 1;
	}
	return ((iReserved + iPerRow - 1) / iPerRow) * iPerRow;
}

static int MapEditTilesetArrangeStaticRows(mapedit_app_t* pApp)
{
	int iCellH;
	int iRows;

	if ( pApp == NULL ) {
		return MAPEDIT_TILESET_ARRANGE_PLACEHOLDER_ROWS;
	}
	iCellH = MapEditTilesetArrangeCellHeight(pApp);
	if ( pApp->tTilesetArrangeStaticDesc.iHeight > 0 ) {
		iRows = (pApp->tTilesetArrangeStaticDesc.iHeight + iCellH - 1) / iCellH;
		return (iRows > 0) ? iRows : 1;
	}
	return MAPEDIT_TILESET_ARRANGE_PLACEHOLDER_ROWS;
}

static float MapEditTilesetArrangeGridContentWidth(mapedit_app_t* pApp)
{
	return (float)(MapEditTilesetArrangeTilesPerRow(pApp) * MapEditTilesetArrangeCellWidth(pApp));
}

static float MapEditTilesetArrangeGridContentHeight(mapedit_app_t* pApp)
{
	int iRows;

	if ( pApp == NULL ) {
		return 1.0f;
	}
	iRows = pApp->tTilesetArrangeGrid.iRowCount;
	if ( iRows <= 0 ) {
		iRows = 1;
	}
	return (float)(iRows * MapEditTilesetArrangeCellHeight(pApp));
}

static void MapEditTilesetArrangeSetScrollContentRect(xge_xui_widget pWidget, xge_rect_t tLocal, xge_rect_t tRect)
{
	int bSizeChanged;

	if ( pWidget == NULL ) {
		return;
	}
	if ( MapEditTilesetArrangeRectAlmostEqual(pWidget->tLocalRect, tLocal) && MapEditTilesetArrangeRectAlmostEqual(pWidget->tRect, tRect) ) {
		return;
	}
	bSizeChanged = pWidget->tStyle.tWidth.iUnit != XGE_XUI_SIZE_PX ||
		pWidget->tStyle.tHeight.iUnit != XGE_XUI_SIZE_PX ||
		MapEditTilesetArrangeAbsFloat(pWidget->tStyle.tWidth.fValue - tLocal.fW) >= 0.01f ||
		MapEditTilesetArrangeAbsFloat(pWidget->tStyle.tHeight.fValue - tLocal.fH) >= 0.01f;
	if ( bSizeChanged ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(tLocal.fW), xgeXuiSizePx(tLocal.fH));
	}
	xgeXuiWidgetSetRect(pWidget, tRect);
	pWidget->tLocalRect = tLocal;
}

static void MapEditTilesetArrangeClearStaticTexture(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pTilesetArrangeStaticTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pApp->pTilesetArrangeStaticTexture);
		pApp->pTilesetArrangeStaticTexture = NULL;
	}
	memset(&pApp->tTilesetArrangeStaticDesc, 0, sizeof(pApp->tTilesetArrangeStaticDesc));
}

static int MapEditTilesetArrangeSpecialDirCount(void)
{
	return (int)(sizeof(g_arrArrangeSpecialDirs) / sizeof(g_arrArrangeSpecialDirs[0]));
}

static void MapEditTilesetArrangeClearSpecialTexture(mapedit_app_t* pApp, int iSlot)
{
	if ( pApp == NULL || iSlot < 0 || iSlot >= MAPEDIT_TILESET_SPECIAL_TILE_MAX ) {
		return;
	}
	if ( pApp->arrTilesetArrangeSpecialTexture[iSlot] != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pApp->arrTilesetArrangeSpecialTexture[iSlot]);
		pApp->arrTilesetArrangeSpecialTexture[iSlot] = NULL;
	}
	memset(&pApp->arrTilesetArrangeSpecialDesc[iSlot], 0, sizeof(pApp->arrTilesetArrangeSpecialDesc[iSlot]));
}

static void MapEditTilesetArrangeClearSpecialTextures(mapedit_app_t* pApp)
{
	int i;

	if ( pApp == NULL ) {
		return;
	}
	for ( i = 0; i < MAPEDIT_TILESET_SPECIAL_TILE_MAX; i++ ) {
		MapEditTilesetArrangeClearSpecialTexture(pApp, i);
	}
}

static int MapEditTilesetArrangeSpecialTypeDirIndex(const char* sType)
{
	int i;

	if ( sType == NULL || sType[0] == 0 ) {
		return -1;
	}
	for ( i = 0; i < MapEditTilesetArrangeSpecialDirCount(); i++ ) {
		if ( strcmp(g_arrArrangeSpecialDirs[i].sType, sType) == 0 ) {
			return i;
		}
	}
	return -1;
}

static int MapEditTilesetArrangeTryLoadSpecialTexture(mapedit_app_t* pApp, int iSlot, const char* sDir, const char* sFile)
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
		pApp->arrTilesetArrangeSpecialTexture[iSlot] = pTexture;
		pApp->arrTilesetArrangeSpecialDesc[iSlot] = tDesc;
		xrtFree(sPath);
		return 1;
	}
	if ( pTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pTexture);
	}
	xrtFree(sPath);
	return 0;
}

static void MapEditTilesetArrangeLoadSpecialTexture(mapedit_app_t* pApp, mapedit_tileset_special_tile_t* pSpecial, int iSlot)
{
	int iTypeDir;
	int i;

	if ( pApp == NULL || pSpecial == NULL || iSlot < 0 || iSlot >= MAPEDIT_TILESET_SPECIAL_TILE_MAX || pSpecial->sFile[0] == 0 ) {
		return;
	}
	iTypeDir = MapEditTilesetArrangeSpecialTypeDirIndex(pSpecial->sType);
	if ( iTypeDir >= 0 && MapEditTilesetArrangeTryLoadSpecialTexture(pApp, iSlot, g_arrArrangeSpecialDirs[iTypeDir].sDir, pSpecial->sFile) ) {
		return;
	}
	for ( i = 0; i < MapEditTilesetArrangeSpecialDirCount(); i++ ) {
		if ( i == iTypeDir ) {
			continue;
		}
		if ( MapEditTilesetArrangeTryLoadSpecialTexture(pApp, iSlot, g_arrArrangeSpecialDirs[i].sDir, pSpecial->sFile) ) {
			return;
		}
	}
}

void MapEditTilesetArrangeReloadSpecialTextures(mapedit_app_t* pApp)
{
	mapedit_tileset_set_item_t* pItem;
	int i;
	int iCount;

	if ( pApp == NULL ) {
		return;
	}
	MapEditTilesetArrangeClearSpecialTextures(pApp);
	pItem = MapEditTilesetArrangeSelectedItem(pApp);
	if ( pItem == NULL ) {
		return;
	}
	iCount = pItem->iSpecialTileCount;
	if ( iCount > MAPEDIT_TILESET_SPECIAL_TILE_MAX ) {
		iCount = MAPEDIT_TILESET_SPECIAL_TILE_MAX;
	}
	for ( i = 0; i < iCount; i++ ) {
		MapEditTilesetArrangeLoadSpecialTexture(pApp, &pItem->arrSpecialTiles[i], i);
	}
	if ( pApp->pTilesetArrangeGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pTilesetArrangeGridWidget);
	}
}

static void MapEditTilesetArrangeLoadStaticTexture(mapedit_app_t* pApp, mapedit_tileset_set_item_t* pItem)
{
	str sPath;
	xui_texture pTexture;
	xui_texture_desc_t tDesc;

	MapEditTilesetArrangeClearStaticTexture(pApp);
	if ( pApp == NULL || pItem == NULL || pItem->sStaticTiles[0] == 0 || strcmp(pItem->sStaticTiles, "未设置") == 0 ) {
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
		pApp->pTilesetArrangeStaticTexture = pTexture;
		pApp->tTilesetArrangeStaticDesc = tDesc;
	} else if ( pTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pTexture);
	}
	xrtFree(sPath);
}

static void MapEditTilesetArrangeUpdateScrollContentSize(mapedit_app_t* pApp)
{
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tTilesetArrangeScrollFrame.pWidget == NULL ) {
		return;
	}
	fContentW = MapEditTilesetArrangeGridContentWidth(pApp);
	fContentH = MapEditTilesetArrangeGridContentHeight(pApp);
	if ( MapEditTilesetArrangeAbsFloat(pApp->tTilesetArrangeScrollModel.fContentW - fContentW) >= 0.01f ||
	     MapEditTilesetArrangeAbsFloat(pApp->tTilesetArrangeScrollModel.fContentH - fContentH) >= 0.01f ) {
		xgeXuiScrollFrameSetContentSize(&pApp->tTilesetArrangeScrollFrame, fContentW, fContentH);
	}
}

static void MapEditTilesetArrangeSyncScrollContent(mapedit_app_t* pApp)
{
	xge_rect_t tViewport;
	xge_rect_t tLocal;
	xge_rect_t tRect;
	float fContentW;
	float fContentH;

	if ( pApp == NULL || pApp->tTilesetArrangeScrollFrame.pWidget == NULL || pApp->pTilesetArrangeGridWidget == NULL ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tTilesetArrangeScrollFrame);
	fContentW = (pApp->tTilesetArrangeScrollModel.fContentW > 1.0f) ? pApp->tTilesetArrangeScrollModel.fContentW : MapEditTilesetArrangeGridContentWidth(pApp);
	fContentH = (pApp->tTilesetArrangeScrollModel.fContentH > 1.0f) ? pApp->tTilesetArrangeScrollModel.fContentH : MapEditTilesetArrangeGridContentHeight(pApp);
	tLocal.fX = -pApp->tTilesetArrangeScrollModel.fScrollX;
	tLocal.fY = -pApp->tTilesetArrangeScrollModel.fScrollY;
	tLocal.fW = (fContentW > tViewport.fW) ? fContentW : tViewport.fW;
	tLocal.fH = (fContentH > tViewport.fH) ? fContentH : tViewport.fH;
	tRect.fX = tViewport.fX + tLocal.fX;
	tRect.fY = tViewport.fY + tLocal.fY;
	tRect.fW = tLocal.fW;
	tRect.fH = tLocal.fH;
	MapEditTilesetArrangeSetScrollContentRect(pApp->pTilesetArrangeGridWidget, tLocal, tRect);
}

static void MapEditTilesetArrangeUpdateAndSyncScrollContent(mapedit_app_t* pApp)
{
	MapEditTilesetArrangeUpdateScrollContentSize(pApp);
	MapEditTilesetArrangeSyncScrollContent(pApp);
}

static void MapEditTilesetArrangeScrollChanged(xge_xui_scroll_frame pFrame, float fScrollX, float fScrollY, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pFrame;
	(void)fScrollX;
	(void)fScrollY;
	pApp = (mapedit_app_t*)pUser;
	MapEditTilesetArrangeSyncScrollContent(pApp);
}

static void MapEditTilesetArrangeScrollLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiScrollFrameLayout(&pApp->tTilesetArrangeScrollFrame);
	MapEditTilesetArrangeSyncScrollContent(pApp);
}

static void MapEditTilesetArrangeContentPaint(mapedit_tile_grid_t* pGrid, xge_rect_t tRect, void* pUser)
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
	pItem = MapEditTilesetArrangeSelectedItem(pApp);
	iCellH = MapEditTilesetArrangeCellHeight(pApp);
	iStaticStartRow = MapEditTilesetArrangeStaticStart(pApp) / MapEditTilesetArrangeTilesPerRow(pApp);
	tStaticRect = (xge_rect_t){
		tRect.fX,
		tRect.fY + (float)(iStaticStartRow * iCellH),
		(float)(MapEditTilesetArrangeTilesPerRow(pApp) * MapEditTilesetArrangeCellWidth(pApp)),
		(float)(MapEditTilesetArrangeStaticRows(pApp) * iCellH)
	};
	if ( pApp->pTilesetArrangeStaticTexture != NULL && pApp->tTilesetArrangeStaticDesc.iWidth > 0 && pApp->tTilesetArrangeStaticDesc.iHeight > 0 ) {
		xge_draw_t tDraw;

		memset(&tDraw, 0, sizeof(tDraw));
		tDraw.pTexture = (xge_texture)pApp->pTilesetArrangeStaticTexture;
		tDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, (float)pApp->tTilesetArrangeStaticDesc.iWidth, (float)pApp->tTilesetArrangeStaticDesc.iHeight };
		tDraw.tDst = (xge_rect_t){ tStaticRect.fX, tStaticRect.fY, (float)pApp->tTilesetArrangeStaticDesc.iWidth, (float)pApp->tTilesetArrangeStaticDesc.iHeight };
		tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
		xgeDrawEx(&tDraw);
		xgeFlush();
		return;
	}
	xgeShapeRectFillPx(tStaticRect, XGE_COLOR_RGBA(235, 244, 250, 255));
	sText = (pItem == NULL) ? "未选择图集" : "静态图块集未设置或图片不可用";
	xgeTextDrawRect(pApp->bFontReady ? &pApp->tFont : NULL, sText, tStaticRect, XGE_COLOR_RGBA(104, 128, 148, 255),
		XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
}

static int MapEditTilesetArrangeDrawSpecialPreview(mapedit_app_t* pApp, int iSlot, xge_rect_t tCell)
{
	xui_texture_desc_t* pDesc;
	xge_draw_t tDraw;
	float fSrcW;
	float fSrcH;

	if ( pApp == NULL || iSlot < 0 || iSlot >= MAPEDIT_TILESET_SPECIAL_TILE_MAX ||
	     pApp->arrTilesetArrangeSpecialTexture[iSlot] == NULL ) {
		return 0;
	}
	pDesc = &pApp->arrTilesetArrangeSpecialDesc[iSlot];
	if ( pDesc->iWidth <= 0 || pDesc->iHeight <= 0 ) {
		return 0;
	}
	fSrcW = (float)MapEditTilesetArrangeCellWidth(pApp);
	fSrcH = (float)MapEditTilesetArrangeCellHeight(pApp);
	if ( fSrcW > (float)pDesc->iWidth ) {
		fSrcW = (float)pDesc->iWidth;
	}
	if ( fSrcH > (float)pDesc->iHeight ) {
		fSrcH = (float)pDesc->iHeight;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = (xge_texture)pApp->arrTilesetArrangeSpecialTexture[iSlot];
	tDraw.tSrc = (xge_rect_t){ 0.0f, 0.0f, fSrcW, fSrcH };
	tDraw.tDst = tCell;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
	xgeFlush();
	return 1;
}

static void MapEditTilesetArrangeCellPaint(mapedit_tile_grid_t* pGrid, int iCol, int iRow, xge_rect_t tCell, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_tileset_set_item_t* pItem;
	char sText[32];
	int iPerRow;
	int iTileId;
	int iStaticStart;
	int iSpecialCount;

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pItem = MapEditTilesetArrangeSelectedItem(pApp);
	iPerRow = MapEditTilesetArrangeTilesPerRow(pApp);
	iTileId = iRow * iPerRow + iCol;
	iSpecialCount = (pItem != NULL) ? pItem->iSpecialTileCount : 0;
	iStaticStart = MapEditTilesetArrangeStaticStart(pApp);
	if ( iTileId == 0 ) {
		xgeShapeRectFillPx(tCell, XGE_COLOR_RGBA(242, 248, 252, 235));
		xgeShapeRectStrokePx(tCell, 1.0f, XGE_COLOR_RGBA(98, 158, 108, 220));
		snprintf(sText, sizeof(sText), "0");
		xgeTextDrawRect(pGrid->pFont, sText, tCell, XGE_COLOR_RGBA(48, 112, 62, 255),
			XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
	} else if ( iTileId >= 1 && iTileId <= iSpecialCount ) {
		uint32_t iFill;
		int iSlot;
		int bConfigured;

		iSlot = iTileId - 1;
		bConfigured = (pItem != NULL &&
			(pItem->arrSpecialTiles[iSlot].sFile[0] != 0 || pItem->arrSpecialTiles[iSlot].sType[0] != 0));
		iFill = bConfigured ?
			XGE_COLOR_RGBA(246, 218, 158, 205) : XGE_COLOR_RGBA(204, 232, 250, 205);
		xgeShapeRectFillPx(tCell, iFill);
		xgeShapeRectStrokePx(tCell, 1.0f, XGE_COLOR_RGBA(72, 150, 208, 230));
		if ( !MapEditTilesetArrangeDrawSpecialPreview(pApp, iSlot, tCell) && bConfigured ) {
			xgeTextDrawRect(pGrid->pFont, "!", tCell, XGE_COLOR_RGBA(168, 82, 42, 255),
				XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
		}
	} else if ( iTileId > iSpecialCount && iTileId < iStaticStart ) {
		xgeShapeRectFillPx(tCell, XGE_COLOR_RGBA(224, 232, 238, 180));
		if ( pGrid->iCellWidth >= 20 && pGrid->iCellHeight >= 16 ) {
			xgeTextDrawRect(pGrid->pFont, "-", tCell, XGE_COLOR_RGBA(120, 136, 148, 255),
				XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
		}
	}
}

static void MapEditTilesetArrangeCellClick(mapedit_tile_grid_t* pGrid, int iCol, int iRow, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_tileset_set_item_t* pItem;
	char sStatus[256];
	int iPerRow;
	int iTileId;
	int iStaticStart;
	int iSpecialCount;

	(void)pGrid;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pItem = MapEditTilesetArrangeSelectedItem(pApp);
	iPerRow = MapEditTilesetArrangeTilesPerRow(pApp);
	iTileId = iRow * iPerRow + iCol;
	iSpecialCount = (pItem != NULL) ? pItem->iSpecialTileCount : 0;
	iStaticStart = MapEditTilesetArrangeStaticStart(pApp);
	pApp->iTilesetArrangeSelectedTile = iTileId;
	if ( iTileId == 0 ) {
		MapEditTilesetPropertiesShowTileset(pApp);
		MapEditAppSetStatus(pApp, "0 号橡皮图块固定，不参与编辑");
	} else if ( iTileId >= 1 && iTileId <= iSpecialCount ) {
		snprintf(sStatus, sizeof(sStatus), "已选择特殊图块槽位: ID %d / 特殊槽 %d", iTileId, iTileId);
		MapEditTilesetPropertiesShowSpecialTile(pApp, iTileId);
		MapEditAppSetStatus(pApp, sStatus);
	} else if ( iTileId > iSpecialCount && iTileId < iStaticStart ) {
		MapEditTilesetPropertiesShowTileset(pApp);
		snprintf(sStatus, sizeof(sStatus), "ID %d 是对齐占位，不写入图集数据", iTileId);
		MapEditAppSetStatus(pApp, sStatus);
	} else {
		MapEditTilesetPropertiesShowTileset(pApp);
		snprintf(sStatus, sizeof(sStatus), "静态图块 ID %d，来自 %s", iTileId, (pItem != NULL && pItem->sStaticTiles[0] != 0) ? pItem->sStaticTiles : "未设置");
		MapEditAppSetStatus(pApp, sStatus);
	}
	if ( pApp->pTilesetArrangeGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pTilesetArrangeGridWidget);
	}
}

void MapEditTilesetArrangeRefresh(mapedit_app_t* pApp)
{
	mapedit_tileset_set_item_t* pItem;
	int iPerRow;
	int iCellW;
	int iCellH;
	int iStaticStart;
	int iStaticRows;
	int iRows;

	if ( pApp == NULL || pApp->pTilesetArrangeGridWidget == NULL ) {
		return;
	}
	pItem = MapEditTilesetArrangeSelectedItem(pApp);
	iPerRow = MapEditTilesetArrangeTilesPerRow(pApp);
	iCellW = MapEditTilesetArrangeCellWidth(pApp);
	iCellH = MapEditTilesetArrangeCellHeight(pApp);
	MapEditTilesetArrangeLoadStaticTexture(pApp, pItem);
	MapEditTilesetArrangeReloadSpecialTextures(pApp);
	iStaticStart = MapEditTilesetArrangeStaticStart(pApp);
	iStaticRows = MapEditTilesetArrangeStaticRows(pApp);
	iRows = (iStaticStart / iPerRow) + iStaticRows;
	if ( iRows < MAPEDIT_TILESET_ARRANGE_PLACEHOLDER_ROWS ) {
		iRows = MAPEDIT_TILESET_ARRANGE_PLACEHOLDER_ROWS;
	}
	MapEditTileGridSetCellSize(&pApp->tTilesetArrangeGrid, iCellW, iCellH);
	MapEditTileGridSetGridSize(&pApp->tTilesetArrangeGrid, iPerRow, iRows);
	MapEditTileGridSetMaxCells(&pApp->tTilesetArrangeGrid, iPerRow * iRows);
	pApp->tTilesetArrangeGrid.bHasSelection = 0;
	pApp->tTilesetArrangeGrid.iHoverCol = -1;
	pApp->tTilesetArrangeGrid.iHoverRow = -1;
	pApp->iTilesetArrangeSelectedTile = -1;
	if ( pApp->tTilesetArrangeScrollFrame.pWidget != NULL ) {
		xgeXuiScrollFrameSetOffset(&pApp->tTilesetArrangeScrollFrame, 0.0f, 0.0f);
		MapEditTilesetArrangeUpdateAndSyncScrollContent(pApp);
	}
	xgeXuiWidgetMarkLayout(pApp->pTilesetArrangeCanvasWidget);
	xgeXuiWidgetMarkPaint(pApp->pTilesetArrangeGridWidget);
}

int MapEditTilesetArrangeHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent)
{
	xge_rect_t tViewport;

	if ( pApp == NULL || pEvent == NULL || pApp->pTilesetArrangeGridWidget == NULL || !MapEditTilesetArrangeMouseGridEvent(pEvent) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( !MapEditTilesetArrangeWidgetInteractive(pApp->pTilesetArrangeGridWidget) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pApp->tTilesetArrangeScrollFrame);
	if ( pApp->tTilesetArrangeGrid.bDragging ||
	     (MapEditTilesetArrangeRectContains(tViewport, pEvent->fX, pEvent->fY) &&
	      MapEditTilesetArrangeRectContains(pApp->pTilesetArrangeGridWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		return MapEditTileGridEventProc(pApp->pTilesetArrangeGridWidget, pEvent, &pApp->tTilesetArrangeGrid);
	}
	return XGE_XUI_EVENT_CONTINUE;
}

int MapEditTilesetArrangeCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
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
	xgeXuiScrollModelInit(&pApp->tTilesetArrangeScrollModel);
	if ( xgeXuiScrollFrameInit(&pApp->tTilesetArrangeScrollFrame, &pApp->tXui, pCanvasWidget, &pApp->tTilesetArrangeScrollModel) != XGE_OK ) {
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	xgeXuiWidgetSetEvent(pCanvasWidget, xgeXuiScrollFrameEventProc, &pApp->tTilesetArrangeScrollFrame);
	xgeXuiWidgetSetLayoutProc(pCanvasWidget, MapEditTilesetArrangeScrollLayoutProc, pApp);
	xgeXuiWidgetSetLayout(xgeXuiScrollFrameGetViewportWidget(&pApp->tTilesetArrangeScrollFrame), XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiScrollFrameSetChange(&pApp->tTilesetArrangeScrollFrame, MapEditTilesetArrangeScrollChanged, pApp);
	xgeXuiScrollFrameSetScrollbarMode(&pApp->tTilesetArrangeScrollFrame, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiScrollFrameSetWheelAxis(&pApp->tTilesetArrangeScrollFrame, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollFrameSetContentDragEnabled(&pApp->tTilesetArrangeScrollFrame, 0);
	xgeXuiScrollFrameSetMetrics(&pApp->tTilesetArrangeScrollFrame, MAPEDIT_TILESET_ARRANGE_SCROLLBAR_SIZE, 28.0f, 0.0f, 0.0f);
	xgeXuiScrollFrameSetColors(&pApp->tTilesetArrangeScrollFrame, XGE_COLOR_RGBA(226, 240, 249, 255), XGE_COLOR_RGBA(90, 164, 214, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(68, 142, 204, 255), XGE_COLOR_RGBA(108, 180, 226, 255), XGE_COLOR_RGBA(180, 204, 218, 255));
	if ( xgeXuiWidgetAdd(xgeXuiScrollFrameGetViewportWidget(&pApp->tTilesetArrangeScrollFrame), pGridWidget) != XGE_OK ||
	     MapEditTileGridInit(&pApp->tTilesetArrangeGrid, pGridWidget, pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		xgeXuiScrollFrameUnit(&pApp->tTilesetArrangeScrollFrame);
		xgeXuiWidgetFree(pCanvasWidget);
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	MapEditTileGridSetExpand(&pApp->tTilesetArrangeGrid, 0, 0);
	MapEditTileGridSetColors(&pApp->tTilesetArrangeGrid,
		XGE_COLOR_RGBA(236, 246, 252, 255),
		XGE_COLOR_RGBA(124, 181, 219, 255),
		XGE_COLOR_RGBA(188, 224, 244, 150),
		XGE_COLOR_RGBA(102, 181, 230, 8),
		XGE_COLOR_RGBA(42, 140, 210, 105),
		XGE_COLOR_RGBA(104, 128, 148, 255));
	MapEditTileGridSetEmptyText(&pApp->tTilesetArrangeGrid, "");
	MapEditTileGridSetContentPaint(&pApp->tTilesetArrangeGrid, MapEditTilesetArrangeContentPaint, pApp);
	MapEditTileGridSetCellPaint(&pApp->tTilesetArrangeGrid, MapEditTilesetArrangeCellPaint, pApp);
	MapEditTileGridSetCellClick(&pApp->tTilesetArrangeGrid, MapEditTilesetArrangeCellClick, pApp);
	pApp->pTilesetArrangeCanvasWidget = pCanvasWidget;
	pApp->pTilesetArrangeGridWidget = pGridWidget;
	if ( xgeXuiWidgetAdd(pContent, pCanvasWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	MapEditTilesetArrangeRefresh(pApp);
	return XGE_OK;
}

void MapEditTilesetArrangeUnit(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	MapEditTilesetArrangeClearStaticTexture(pApp);
	MapEditTilesetArrangeClearSpecialTextures(pApp);
	if ( pApp->pTilesetArrangeGridWidget != NULL ) {
		MapEditTileGridUnit(&pApp->tTilesetArrangeGrid);
		pApp->pTilesetArrangeGridWidget = NULL;
	}
	if ( pApp->pTilesetArrangeCanvasWidget != NULL ) {
		xgeXuiScrollFrameUnit(&pApp->tTilesetArrangeScrollFrame);
		pApp->pTilesetArrangeCanvasWidget = NULL;
	}
}
