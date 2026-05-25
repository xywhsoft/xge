#include "map_maps.h"
#include "map_tile_select.h"
#include "map_edit.h"
#include "map_passage.h"
#include "map_tags.h"
#include "../../tileset/tabs/tileset_sets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAPEDIT_MAP_ASSET_DIR "maps"
#define MAPEDIT_MAP_DEFAULT_FILE "default.xson"
#define MAPEDIT_MAP_KEY_NAME "地图名称"
#define MAPEDIT_MAP_KEY_TILESET "图集"
#define MAPEDIT_MAP_KEY_STATE "当前状态"
#define MAPEDIT_MAP_KEY_WIDTH "地图宽度"
#define MAPEDIT_MAP_KEY_HEIGHT "地图高度"
#define MAPEDIT_MAP_KEY_CUSTOM_DATA "自定义数据"
#define MAPEDIT_MAP_KEY_TILES "图块数据"
#define MAPEDIT_MAP_KEY_PASSAGE_OVERRIDES "通行修正"
#define MAPEDIT_MAP_KEY_CELL_CUSTOM_DATA "cellCustomData"
#define MAPEDIT_MAP_KEY_CELL "cell"
#define MAPEDIT_MAP_KEY_VALUE "value"

#define MAPEDIT_BLOB47_COLS 8
#define MAPEDIT_BLOB47_ROWS 6
#define MAPEDIT_BLOB47_COUNT 47
#define MAPEDIT_BLOB47_N 0x01
#define MAPEDIT_BLOB47_E 0x02
#define MAPEDIT_BLOB47_S 0x04
#define MAPEDIT_BLOB47_W 0x08
#define MAPEDIT_BLOB47_NE 0x10
#define MAPEDIT_BLOB47_SE 0x20
#define MAPEDIT_BLOB47_SW 0x40
#define MAPEDIT_BLOB47_NW 0x80

typedef struct mapedit_map_scan_t {
	mapedit_app_t* pApp;
	int iLoadedCount;
} mapedit_map_scan_t;

static void MapEditMapCopyText(char* sDst, int iDstSize, const char* sSrc)
{
	size_t iLen;

	if ( sDst == NULL || iDstSize <= 0 ) {
		return;
	}
	if ( sSrc == NULL ) {
		sSrc = "";
	}
	iLen = strlen(sSrc);
	if ( iLen >= (size_t)iDstSize ) {
		iLen = (size_t)iDstSize - 1u;
	}
	memmove(sDst, sSrc, iLen);
	sDst[iLen] = 0;
}

static str MapEditMapDirPath(void)
{
	return xrtPathJoin(3, xCore.AppPath, (str)"assets", (str)MAPEDIT_MAP_ASSET_DIR);
}

static str MapEditMapFilePath(const char* sFile)
{
	if ( sFile == NULL || sFile[0] == 0 ) {
		return NULL;
	}
	return xrtPathJoin(4, xCore.AppPath, (str)"assets", (str)MAPEDIT_MAP_ASSET_DIR, (str)sFile);
}

static int MapEditMapEnsureDir(void)
{
	str sDir;
	int iRet;

	sDir = MapEditMapDirPath();
	if ( sDir == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = XGE_OK;
	if ( !xrtDirExists(sDir) && !xrtDirCreateAll(sDir) ) {
		iRet = XGE_ERROR;
	}
	xrtFree(sDir);
	return iRet;
}

static int MapEditMapIsXsonFile(const char* sPath)
{
	const char* sExt;
	char sLower[16];
	int i;

	if ( sPath == NULL ) {
		return 0;
	}
	sExt = strrchr(sPath, '.');
	if ( sExt == NULL ) {
		return 0;
	}
	for ( i = 0; i < (int)sizeof(sLower) - 1 && sExt[i] != 0; i++ ) {
		char ch;

		ch = sExt[i];
		if ( ch >= 'A' && ch <= 'Z' ) {
			ch = (char)(ch - 'A' + 'a');
		}
		sLower[i] = ch;
	}
	sLower[i] = 0;
	return strcmp(sLower, ".xson") == 0;
}

static int MapEditMapReadInt(xvalue pTable, const char* sKey, int iDefault)
{
	xvalue pValue;
	int iType;

	if ( pTable == NULL || sKey == NULL || xvoType(pTable) != XVO_DT_TABLE ) {
		return iDefault;
	}
	pValue = xvoTableGetValue(pTable, sKey, (uint32)strlen(sKey));
	iType = xvoType(pValue);
	if ( iType == XVO_DT_INT ) {
		return (int)xvoGetInt(pValue);
	}
	if ( iType == XVO_DT_FLOAT ) {
		return (int)xvoGetFloat(pValue);
	}
	return iDefault;
}

static const char* MapEditMapReadText(xvalue pTable, const char* sKey, const char* sDefault)
{
	xvalue pValue;

	if ( pTable == NULL || sKey == NULL || xvoType(pTable) != XVO_DT_TABLE ) {
		return sDefault;
	}
	pValue = xvoTableGetValue(pTable, sKey, (uint32)strlen(sKey));
	if ( xvoType(pValue) == XVO_DT_TEXT ) {
		return (const char*)xvoGetText(pValue);
	}
	return sDefault;
}

static void MapEditMapValueToText(xvalue pValue, char* sDst, int iDstSize)
{
	if ( sDst == NULL || iDstSize <= 0 ) {
		return;
	}
	sDst[0] = 0;
	if ( pValue == NULL || xvoType(pValue) == XVO_DT_NULL ) {
		return;
	}
	switch ( xvoType(pValue) ) {
	case XVO_DT_BOOL:
		MapEditMapCopyText(sDst, iDstSize, xvoGetBool(pValue) ? "true" : "false");
		break;
	case XVO_DT_INT:
		snprintf(sDst, (size_t)iDstSize, "%lld", (long long)xvoGetInt(pValue));
		break;
	case XVO_DT_FLOAT:
		snprintf(sDst, (size_t)iDstSize, "%.6g", xvoGetFloat(pValue));
		break;
	case XVO_DT_TEXT:
		MapEditMapCopyText(sDst, iDstSize, (const char*)xvoGetText(pValue));
		break;
	default:
		break;
	}
}

static const char* MapEditMapCustomDisplayValue(const mapedit_custom_channel_def_t* pDef, const char* sValue)
{
	int i;

	if ( pDef == NULL || sValue == NULL ) {
		return "";
	}
	if ( strcmp(pDef->sMarkMode, "enum") == 0 ) {
		for ( i = 0; i < pDef->iOptionCount; i++ ) {
			if ( strcmp(pDef->arrOptions[i].sValue, sValue) == 0 ) {
				return pDef->arrOptions[i].sText;
			}
		}
	}
	return sValue;
}

int MapEditMapSetupLayerCount(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupLayerCount <= 0 ) {
		return MAPEDIT_MAP_LAYER_DEFAULT;
	}
	if ( pApp->iSetupLayerCount > MAPEDIT_MAP_LAYER_OPTION_MAX ) {
		return MAPEDIT_MAP_LAYER_OPTION_MAX;
	}
	return pApp->iSetupLayerCount;
}

const char* MapEditMapSetupLayerName(mapedit_app_t* pApp, int iLayer)
{
	if ( pApp == NULL || iLayer < 0 || iLayer >= pApp->iSetupLayerCount || iLayer >= MAPEDIT_MAP_LAYER_OPTION_MAX ) {
		return NULL;
	}
	if ( pApp->arrSetupLayers[iLayer].sName[0] == 0 ) {
		return NULL;
	}
	return pApp->arrSetupLayers[iLayer].sName;
}

int MapEditMapSetupLayerAboveActor(mapedit_app_t* pApp, int iLayer)
{
	if ( pApp == NULL || iLayer < 0 || iLayer >= pApp->iSetupLayerCount || iLayer >= MAPEDIT_MAP_LAYER_OPTION_MAX ) {
		return 0;
	}
	return pApp->arrSetupLayers[iLayer].bAboveActor != 0;
}

static int MapEditMapSetupStateMin(mapedit_app_t* pApp)
{
	return (pApp != NULL) ? pApp->iSetupStateMin : 0;
}

static int MapEditMapSetupStateMax(mapedit_app_t* pApp)
{
	int iMin;
	int iMax;

	iMin = MapEditMapSetupStateMin(pApp);
	iMax = (pApp != NULL) ? pApp->iSetupStateMax : 3;
	return (iMax >= iMin) ? iMax : iMin;
}

static int MapEditMapClampState(mapedit_app_t* pApp, int iState)
{
	int iMin;
	int iMax;

	iMin = MapEditMapSetupStateMin(pApp);
	iMax = MapEditMapSetupStateMax(pApp);
	if ( iState < iMin ) {
		return iMin;
	}
	if ( iState > iMax ) {
		return iMax;
	}
	return iState;
}

static void MapEditMapNormalize(mapedit_app_t* pApp, mapedit_map_item_t* pItem)
{
	long long iCount;

	if ( pItem == NULL ) {
		return;
	}
	pItem->iState = MapEditMapClampState(pApp, pItem->iState);
	if ( pItem->iWidth < 1 ) {
		pItem->iWidth = 1;
	} else if ( pItem->iWidth > 2048 ) {
		pItem->iWidth = 2048;
	}
	if ( pItem->iHeight < 1 ) {
		pItem->iHeight = 1;
	} else if ( pItem->iHeight > 2048 ) {
		pItem->iHeight = 2048;
	}
	pItem->iLayerCount = MapEditMapSetupLayerCount(pApp);
	iCount = (long long)pItem->iWidth * (long long)pItem->iHeight * (long long)pItem->iLayerCount;
	while ( iCount > MAPEDIT_MAP_SIZE_MAX && pItem->iHeight > 1 ) {
		pItem->iHeight--;
		iCount = (long long)pItem->iWidth * (long long)pItem->iHeight * (long long)pItem->iLayerCount;
	}
	while ( iCount > MAPEDIT_MAP_SIZE_MAX && pItem->iWidth > 1 ) {
		pItem->iWidth--;
		iCount = (long long)pItem->iWidth * (long long)pItem->iHeight * (long long)pItem->iLayerCount;
	}
}

static int MapEditMapTileCount(mapedit_app_t* pApp, mapedit_map_item_t* pItem)
{
	long long iCount;

	if ( pItem == NULL ) {
		return 0;
	}
	MapEditMapNormalize(pApp, pItem);
	iCount = (long long)pItem->iWidth * (long long)pItem->iHeight * (long long)pItem->iLayerCount;
	if ( iCount <= 0 || iCount > MAPEDIT_MAP_SIZE_MAX ) {
		return 0;
	}
	return (int)iCount;
}

static int MapEditMapCellCount(mapedit_map_item_t* pItem)
{
	long long iCount;

	if ( pItem == NULL ) {
		return 0;
	}
	iCount = (long long)pItem->iWidth * (long long)pItem->iHeight;
	if ( iCount <= 0 || iCount > MAPEDIT_MAP_SIZE_MAX ) {
		return 0;
	}
	return (int)iCount;
}

int MapEditMapCellId(mapedit_map_item_t* pMap, int x, int y)
{
	if ( pMap == NULL || x < 0 || y < 0 || x >= pMap->iWidth || y >= pMap->iHeight ) {
		return -1;
	}
	return y * pMap->iWidth + x;
}

int MapEditMapTileIndex(mapedit_map_item_t* pMap, int iLayer, int x, int y)
{
	if ( pMap == NULL || iLayer < 0 || iLayer >= pMap->iLayerCount || x < 0 || y < 0 || x >= pMap->iWidth || y >= pMap->iHeight ) {
		return -1;
	}
	return (iLayer * pMap->iHeight + y) * pMap->iWidth + x;
}

int MapEditMapGetTileAt(mapedit_app_t* pApp, int iLayer, int x, int y)
{
	mapedit_map_item_t* pMap;
	int iIndex;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pApp == NULL || pMap == NULL || pApp->pMapTileData == NULL ) {
		return 0;
	}
	iIndex = MapEditMapTileIndex(pMap, iLayer, x, y);
	if ( iIndex < 0 || iIndex >= pApp->iMapTileDataCount ) {
		return 0;
	}
	return pApp->pMapTileData[iIndex];
}

int MapEditMapTopTileAt(mapedit_app_t* pApp, int x, int y)
{
	mapedit_map_item_t* pMap;
	int iLayer;
	int iTileId;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pApp == NULL || pMap == NULL ) {
		return 0;
	}
	for ( iLayer = pMap->iLayerCount - 1; iLayer >= 0; iLayer-- ) {
		iTileId = MapEditMapGetTileAt(pApp, iLayer, x, y);
		if ( iTileId > 0 ) {
			return iTileId;
		}
	}
	return 0;
}

static unsigned char MapEditMapBlob47NormalizeMask(unsigned char iMask)
{
	if ( (iMask & (MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E)) != (MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E) ) {
		iMask = (unsigned char)(iMask & ~MAPEDIT_BLOB47_NE);
	}
	if ( (iMask & (MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S)) != (MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S) ) {
		iMask = (unsigned char)(iMask & ~MAPEDIT_BLOB47_SE);
	}
	if ( (iMask & (MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W)) != (MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W) ) {
		iMask = (unsigned char)(iMask & ~MAPEDIT_BLOB47_SW);
	}
	if ( (iMask & (MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_N)) != (MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_N) ) {
		iMask = (unsigned char)(iMask & ~MAPEDIT_BLOB47_NW);
	}
	return iMask;
}

static int MapEditMapBlob47IndexFromMask(unsigned char iMask)
{
	static const unsigned char arrBlob47Masks[MAPEDIT_BLOB47_COUNT] = {
		0,
		MAPEDIT_BLOB47_E,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_S,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_SE,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S,
		MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_S,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_SW | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SW | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W,
		MAPEDIT_BLOB47_N,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_NE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_NE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SW | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_SE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_SW | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SW | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SE,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_SE | MAPEDIT_BLOB47_NW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W | MAPEDIT_BLOB47_NE | MAPEDIT_BLOB47_SW,
		MAPEDIT_BLOB47_N | MAPEDIT_BLOB47_E | MAPEDIT_BLOB47_S | MAPEDIT_BLOB47_W
	};
	int i;

	iMask = MapEditMapBlob47NormalizeMask(iMask);
	for ( i = 0; i < MAPEDIT_BLOB47_COUNT; i++ ) {
		if ( arrBlob47Masks[i] == iMask ) {
			return i;
		}
	}
	return 0;
}

static int MapEditMapTileMatchesAt(mapedit_app_t* pApp, mapedit_map_item_t* pMap, int iLayer, int x, int y, int iTileId)
{
	int iIndex;

	if ( pApp == NULL || pMap == NULL || pApp->pMapTileData == NULL ||
	     iLayer < 0 || iLayer >= pMap->iLayerCount || x < 0 || y < 0 || x >= pMap->iWidth || y >= pMap->iHeight ) {
		return 0;
	}
	iIndex = MapEditMapTileIndex(pMap, iLayer, x, y);
	if ( iIndex < 0 || iIndex >= pApp->iMapTileDataCount ) {
		return 0;
	}
	return pApp->pMapTileData[iIndex] == iTileId;
}

static unsigned char MapEditMapBlob47MaskForCell(mapedit_app_t* pApp, int iLayer, int iCol, int iRow, int iTileId)
{
	mapedit_map_item_t* pMap;
	unsigned char iMask;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pMap == NULL || iLayer < 0 || iCol < 0 || iRow < 0 ) {
		return 0;
	}
	iMask = 0;
	if ( MapEditMapTileMatchesAt(pApp, pMap, iLayer, iCol, iRow - 1, iTileId) ) {
		iMask = (unsigned char)(iMask | MAPEDIT_BLOB47_N);
	}
	if ( MapEditMapTileMatchesAt(pApp, pMap, iLayer, iCol + 1, iRow, iTileId) ) {
		iMask = (unsigned char)(iMask | MAPEDIT_BLOB47_E);
	}
	if ( MapEditMapTileMatchesAt(pApp, pMap, iLayer, iCol, iRow + 1, iTileId) ) {
		iMask = (unsigned char)(iMask | MAPEDIT_BLOB47_S);
	}
	if ( MapEditMapTileMatchesAt(pApp, pMap, iLayer, iCol - 1, iRow, iTileId) ) {
		iMask = (unsigned char)(iMask | MAPEDIT_BLOB47_W);
	}
	if ( MapEditMapTileMatchesAt(pApp, pMap, iLayer, iCol + 1, iRow - 1, iTileId) ) {
		iMask = (unsigned char)(iMask | MAPEDIT_BLOB47_NE);
	}
	if ( MapEditMapTileMatchesAt(pApp, pMap, iLayer, iCol + 1, iRow + 1, iTileId) ) {
		iMask = (unsigned char)(iMask | MAPEDIT_BLOB47_SE);
	}
	if ( MapEditMapTileMatchesAt(pApp, pMap, iLayer, iCol - 1, iRow + 1, iTileId) ) {
		iMask = (unsigned char)(iMask | MAPEDIT_BLOB47_SW);
	}
	if ( MapEditMapTileMatchesAt(pApp, pMap, iLayer, iCol - 1, iRow - 1, iTileId) ) {
		iMask = (unsigned char)(iMask | MAPEDIT_BLOB47_NW);
	}
	return MapEditMapBlob47NormalizeMask(iMask);
}

static int MapEditMapSpecialTileIsBlob47(const mapedit_tileset_special_tile_t* pSpecial)
{
	if ( pSpecial == NULL ) {
		return 0;
	}
	return strcmp(pSpecial->sType, "自动图块") == 0 || strcmp(pSpecial->sType, "多状态自动图块") == 0;
}

static int MapEditMapSpecialTileHasState(const mapedit_tileset_special_tile_t* pSpecial)
{
	if ( pSpecial == NULL ) {
		return 0;
	}
	return strcmp(pSpecial->sType, "多状态图块") == 0 || strcmp(pSpecial->sType, "多状态自动图块") == 0;
}

static int MapEditMapSpecialTileFrameBlockWidth(const mapedit_tileset_special_tile_t* pSpecial, int iTileW)
{
	if ( iTileW <= 0 ) {
		return 0;
	}
	return MapEditMapSpecialTileIsBlob47(pSpecial) ? (iTileW * MAPEDIT_BLOB47_COLS) : iTileW;
}

static int MapEditMapCurrentStateIndex(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pMap;
	int iState;

	pMap = MapEditMapSelectedItem(pApp);
	iState = (pMap != NULL) ? pMap->iState : MapEditMapSetupStateMin(pApp);
	iState = MapEditMapClampState(pApp, iState);
	return iState - MapEditMapSetupStateMin(pApp);
}

static void MapEditMapFreeCustomDataArray(mapedit_tileset_custom_channel_data_t* arrData, int* pCount)
{
	int i;
	int iCount;

	if ( arrData == NULL || pCount == NULL ) {
		return;
	}
	iCount = *pCount;
	if ( iCount < 0 ) {
		iCount = 0;
	}
	if ( iCount > MAPEDIT_CUSTOM_CHANNEL_MAX ) {
		iCount = MAPEDIT_CUSTOM_CHANNEL_MAX;
	}
	for ( i = 0; i < iCount; i++ ) {
		free(arrData[i].arrValues);
		memset(&arrData[i], 0, sizeof(arrData[i]));
	}
	*pCount = 0;
}

void MapEditMapClearCellCustomData(mapedit_map_item_t* pItem)
{
	if ( pItem == NULL ) {
		return;
	}
	MapEditMapFreeCustomDataArray(pItem->arrCellCustomData, &pItem->iCellCustomDataCount);
}

static void MapEditMapFreeItemRuntimeData(mapedit_map_item_t* pItem)
{
	if ( pItem == NULL ) {
		return;
	}
	free(pItem->arrPassageOverrides);
	pItem->arrPassageOverrides = NULL;
	pItem->iPassageOverrideCount = 0;
	pItem->iPassageOverrideCapacity = 0;
	MapEditMapClearCellCustomData(pItem);
}

static int MapEditMapFindPassageOverride(mapedit_map_item_t* pItem, int iCellId)
{
	int i;

	if ( pItem == NULL || iCellId < 0 ) {
		return -1;
	}
	for ( i = 0; i < pItem->iPassageOverrideCount; i++ ) {
		if ( pItem->arrPassageOverrides[i].iCellId == iCellId ) {
			return i;
		}
	}
	return -1;
}

static void MapEditMapRemovePassageOverride(mapedit_map_item_t* pItem, int iIndex)
{
	if ( pItem == NULL || iIndex < 0 || iIndex >= pItem->iPassageOverrideCount ) {
		return;
	}
	if ( iIndex + 1 < pItem->iPassageOverrideCount ) {
		memmove(&pItem->arrPassageOverrides[iIndex], &pItem->arrPassageOverrides[iIndex + 1],
			(size_t)(pItem->iPassageOverrideCount - iIndex - 1) * sizeof(mapedit_map_passage_override_t));
	}
	pItem->iPassageOverrideCount--;
}

static int MapEditMapSetPassageOverride(mapedit_map_item_t* pItem, int iCellId, unsigned char iValue)
{
	mapedit_map_passage_override_t* pNew;
	int iIndex;
	int iNewCapacity;

	if ( pItem == NULL || iCellId < 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iIndex = MapEditMapFindPassageOverride(pItem, iCellId);
	if ( iIndex >= 0 ) {
		pItem->arrPassageOverrides[iIndex].iValue = iValue;
		return XGE_OK;
	}
	if ( pItem->iPassageOverrideCount >= pItem->iPassageOverrideCapacity ) {
		iNewCapacity = (pItem->iPassageOverrideCapacity > 0) ? pItem->iPassageOverrideCapacity * 2 : 64;
		pNew = (mapedit_map_passage_override_t*)realloc(pItem->arrPassageOverrides, (size_t)iNewCapacity * sizeof(mapedit_map_passage_override_t));
		if ( pNew == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pItem->arrPassageOverrides = pNew;
		pItem->iPassageOverrideCapacity = iNewCapacity;
	}
	pItem->arrPassageOverrides[pItem->iPassageOverrideCount].iCellId = iCellId;
	pItem->arrPassageOverrides[pItem->iPassageOverrideCount].iValue = iValue;
	pItem->iPassageOverrideCount++;
	return XGE_OK;
}

static mapedit_tileset_custom_channel_data_t* MapEditMapFindCellCustomChannelData(mapedit_map_item_t* pItem, const char* sChannelId)
{
	int i;

	if ( pItem == NULL || sChannelId == NULL || sChannelId[0] == 0 ) {
		return NULL;
	}
	for ( i = 0; i < pItem->iCellCustomDataCount && i < MAPEDIT_CUSTOM_CHANNEL_MAX; i++ ) {
		if ( strcmp(pItem->arrCellCustomData[i].sChannelId, sChannelId) == 0 ) {
			return &pItem->arrCellCustomData[i];
		}
	}
	return NULL;
}

static mapedit_tileset_custom_channel_data_t* MapEditMapEnsureCellCustomChannelData(mapedit_map_item_t* pItem, const char* sChannelId)
{
	mapedit_tileset_custom_channel_data_t* pData;

	pData = MapEditMapFindCellCustomChannelData(pItem, sChannelId);
	if ( pData != NULL ) {
		return pData;
	}
	if ( pItem == NULL || pItem->iCellCustomDataCount >= MAPEDIT_CUSTOM_CHANNEL_MAX ) {
		return NULL;
	}
	pData = &pItem->arrCellCustomData[pItem->iCellCustomDataCount++];
	memset(pData, 0, sizeof(*pData));
	MapEditMapCopyText(pData->sChannelId, sizeof(pData->sChannelId), sChannelId);
	return pData;
}

static int MapEditMapFindCellCustomValue(mapedit_tileset_custom_channel_data_t* pData, int iCellId)
{
	int i;

	if ( pData == NULL || iCellId < 0 ) {
		return -1;
	}
	for ( i = 0; i < pData->iValueCount; i++ ) {
		if ( pData->arrValues[i].iTileId == iCellId ) {
			return i;
		}
	}
	return -1;
}

static void MapEditMapRemoveCellCustomValue(mapedit_tileset_custom_channel_data_t* pData, int iIndex)
{
	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iValueCount ) {
		return;
	}
	if ( iIndex + 1 < pData->iValueCount ) {
		memmove(&pData->arrValues[iIndex], &pData->arrValues[iIndex + 1],
			(size_t)(pData->iValueCount - iIndex - 1) * sizeof(mapedit_tileset_custom_value_t));
	}
	pData->iValueCount--;
}

static int MapEditMapGetCellCustomOverride(mapedit_map_item_t* pItem, const char* sChannelId, int iCellId, char* sValue, int iValueSize)
{
	mapedit_tileset_custom_channel_data_t* pData;
	int iIndex;

	if ( sValue != NULL && iValueSize > 0 ) {
		sValue[0] = 0;
	}
	if ( pItem == NULL || sChannelId == NULL || iCellId < 0 ) {
		return 0;
	}
	pData = MapEditMapFindCellCustomChannelData(pItem, sChannelId);
	iIndex = MapEditMapFindCellCustomValue(pData, iCellId);
	if ( iIndex < 0 ) {
		return 0;
	}
	MapEditMapCopyText(sValue, iValueSize, pData->arrValues[iIndex].sValue);
	return 1;
}

static int MapEditMapSetCellCustomOverride(mapedit_map_item_t* pItem, const char* sChannelId, int iCellId, const char* sValue)
{
	mapedit_tileset_custom_channel_data_t* pData;
	mapedit_tileset_custom_value_t* pNewValues;
	int iIndex;
	int iNewCapacity;

	if ( pItem == NULL || sChannelId == NULL || sChannelId[0] == 0 || iCellId < 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pData = MapEditMapEnsureCellCustomChannelData(pItem, sChannelId);
	if ( pData == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iIndex = MapEditMapFindCellCustomValue(pData, iCellId);
	if ( iIndex < 0 ) {
		if ( pData->iValueCount >= pData->iValueCapacity ) {
			iNewCapacity = (pData->iValueCapacity > 0) ? pData->iValueCapacity * 2 : 64;
			pNewValues = (mapedit_tileset_custom_value_t*)realloc(pData->arrValues, (size_t)iNewCapacity * sizeof(mapedit_tileset_custom_value_t));
			if ( pNewValues == NULL ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			pData->arrValues = pNewValues;
			pData->iValueCapacity = iNewCapacity;
		}
		iIndex = pData->iValueCount++;
		memset(&pData->arrValues[iIndex], 0, sizeof(pData->arrValues[iIndex]));
		pData->arrValues[iIndex].iTileId = iCellId;
	}
	MapEditMapCopyText(pData->arrValues[iIndex].sValue, sizeof(pData->arrValues[iIndex].sValue), sValue);
	return XGE_OK;
}

static void MapEditMapCompactRuntimeData(mapedit_map_item_t* pItem)
{
	mapedit_tileset_custom_channel_data_t* pData;
	int iCellCount;
	int i;
	int j;
	int n;

	if ( pItem == NULL ) {
		return;
	}
	iCellCount = MapEditMapCellCount(pItem);
	n = 0;
	for ( i = 0; i < pItem->iPassageOverrideCount; i++ ) {
		if ( pItem->arrPassageOverrides[i].iCellId < 0 || pItem->arrPassageOverrides[i].iCellId >= iCellCount ) {
			continue;
		}
		if ( n != i ) {
			pItem->arrPassageOverrides[n] = pItem->arrPassageOverrides[i];
		}
		n++;
	}
	pItem->iPassageOverrideCount = n;
	for ( i = 0; i < pItem->iCellCustomDataCount && i < MAPEDIT_CUSTOM_CHANNEL_MAX; i++ ) {
		pData = &pItem->arrCellCustomData[i];
		n = 0;
		for ( j = 0; j < pData->iValueCount; j++ ) {
			if ( pData->arrValues[j].iTileId < 0 || pData->arrValues[j].iTileId >= iCellCount ) {
				continue;
			}
			if ( n != j ) {
				pData->arrValues[n] = pData->arrValues[j];
			}
			n++;
		}
		pData->iValueCount = n;
	}
}

static void MapEditMapBuildText(mapedit_map_item_t* pItem)
{
	char sText[sizeof(pItem->sText)];

	if ( pItem == NULL ) {
		return;
	}
	if ( pItem->sFile[0] != 0 ) {
		snprintf(sText, sizeof(sText), "%s    %s", pItem->sName, pItem->sFile);
	} else {
		snprintf(sText, sizeof(sText), "%s    未保存", pItem->sName);
	}
	MapEditMapCopyText(pItem->sText, sizeof(pItem->sText), sText);
}

static void MapEditMapMakeUniqueFile(char* sDst, int iDstSize)
{
	char sCandidate[MAPEDIT_MAP_FILE_CAPACITY];
	str sPath;
	int i;

	if ( sDst == NULL || iDstSize <= 0 ) {
		return;
	}
	sDst[0] = 0;
	for ( i = 1; i < 10000; i++ ) {
		snprintf(sCandidate, sizeof(sCandidate), "map_%03d.xson", i);
		sPath = MapEditMapFilePath(sCandidate);
		if ( sPath == NULL ) {
			continue;
		}
		if ( !xrtFileExists(sPath) ) {
			MapEditMapCopyText(sDst, iDstSize, sCandidate);
			xrtFree(sPath);
			return;
		}
		xrtFree(sPath);
	}
	MapEditMapCopyText(sDst, iDstSize, "map.xson");
}

static int MapEditMapAppend(mapedit_app_t* pApp, const char* sName, const char* sFile)
{
	mapedit_map_item_t* pItem;
	int iIndex;

	if ( pApp == NULL || pApp->iMapCount >= MAPEDIT_MAP_ITEM_MAX ) {
		return -1;
	}
	iIndex = pApp->iMapCount++;
	pItem = &pApp->arrMapItems[iIndex];
	memset(pItem, 0, sizeof(*pItem));
	if ( sFile != NULL && sFile[0] != 0 ) {
		MapEditMapCopyText(pItem->sFile, sizeof(pItem->sFile), sFile);
	} else {
		MapEditMapMakeUniqueFile(pItem->sFile, sizeof(pItem->sFile));
	}
	MapEditMapCopyText(pItem->sName, sizeof(pItem->sName), sName);
	if ( pApp->iTilesetSetCount > 0 ) {
		MapEditMapCopyText(pItem->sTileset, sizeof(pItem->sTileset), pApp->arrTilesetSetItems[0].sFile);
	}
	pItem->iWidth = MAPEDIT_MAP_WIDTH_DEFAULT;
	pItem->iHeight = MAPEDIT_MAP_HEIGHT_DEFAULT;
	pItem->iLayerCount = MapEditMapSetupLayerCount(pApp);
	pItem->iState = MapEditMapSetupStateMin(pApp);
	MapEditMapNormalize(pApp, pItem);
	MapEditMapBuildText(pItem);
	pApp->arrMapListItems[iIndex] = pItem->sText;
	return iIndex;
}

static int MapEditMapFindTilesetByFile(mapedit_app_t* pApp, const char* sFile)
{
	int i;

	if ( pApp == NULL || sFile == NULL || sFile[0] == 0 ) {
		return -1;
	}
	for ( i = 0; i < pApp->iTilesetSetCount; i++ ) {
		if ( strcmp(pApp->arrTilesetSetItems[i].sFile, sFile) == 0 ) {
			return i;
		}
	}
	return -1;
}

mapedit_tileset_set_item_t* MapEditMapSelectedTileset(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pMap;
	int iIndex;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pApp == NULL || pMap == NULL ) {
		return NULL;
	}
	iIndex = MapEditMapFindTilesetByFile(pApp, pMap->sTileset);
	if ( iIndex < 0 && pApp->iTilesetSetCount > 0 ) {
		iIndex = 0;
	}
	if ( iIndex < 0 || iIndex >= pApp->iTilesetSetCount ) {
		return NULL;
	}
	return &pApp->arrTilesetSetItems[iIndex];
}

static void MapEditMapTilesetOptionAdd(mapedit_app_t* pApp, const char* sText, const char* sValue)
{
	int iIndex;

	if ( pApp == NULL || sText == NULL || pApp->iMapTilesetOptionCount >= MAPEDIT_MAP_TILESET_OPTION_MAX ) {
		return;
	}
	iIndex = pApp->iMapTilesetOptionCount++;
	MapEditMapCopyText(pApp->arrMapTilesetOptionText[iIndex], sizeof(pApp->arrMapTilesetOptionText[iIndex]), sText);
	MapEditMapCopyText(pApp->arrMapTilesetOptionValue[iIndex], sizeof(pApp->arrMapTilesetOptionValue[iIndex]), sValue);
	pApp->arrMapTilesetOptions[iIndex] = pApp->arrMapTilesetOptionText[iIndex];
}

static void MapEditMapTilesetLoadOptions(mapedit_app_t* pApp, const char* sCurrentFile)
{
	char sText[MAPEDIT_MAP_TEXT_CAPACITY * 2];
	int i;
	int bFound;

	if ( pApp == NULL ) {
		return;
	}
	pApp->iMapTilesetOptionCount = 0;
	MapEditMapTilesetOptionAdd(pApp, "未设置", "");
	bFound = (sCurrentFile == NULL || sCurrentFile[0] == 0);
	for ( i = 0; i < pApp->iTilesetSetCount; i++ ) {
		snprintf(sText, sizeof(sText), "%s (%s)", pApp->arrTilesetSetItems[i].sName, pApp->arrTilesetSetItems[i].sFile);
		MapEditMapTilesetOptionAdd(pApp, sText, pApp->arrTilesetSetItems[i].sFile);
		if ( sCurrentFile != NULL && strcmp(pApp->arrTilesetSetItems[i].sFile, sCurrentFile) == 0 ) {
			bFound = 1;
		}
	}
	if ( !bFound && sCurrentFile != NULL && sCurrentFile[0] != 0 ) {
		snprintf(sText, sizeof(sText), "已丢失(%s)", sCurrentFile);
		MapEditMapTilesetOptionAdd(pApp, sText, sCurrentFile);
	}
}

static int MapEditMapTilesetFindByValue(mapedit_app_t* pApp, const char* sValue)
{
	int i;

	if ( pApp == NULL ) {
		return -1;
	}
	if ( sValue == NULL ) {
		sValue = "";
	}
	for ( i = 0; i < pApp->iMapTilesetOptionCount; i++ ) {
		if ( strcmp(pApp->arrMapTilesetOptionValue[i], sValue) == 0 ) {
			return i;
		}
	}
	return -1;
}

static int MapEditMapTilesetFindByText(mapedit_app_t* pApp, const char* sText)
{
	int i;

	if ( pApp == NULL || sText == NULL ) {
		return -1;
	}
	for ( i = 0; i < pApp->iMapTilesetOptionCount; i++ ) {
		if ( strcmp(pApp->arrMapTilesetOptionText[i], sText) == 0 ) {
			return i;
		}
	}
	return -1;
}

static const char* MapEditMapTilesetDisplayForValue(mapedit_app_t* pApp, const char* sValue)
{
	int iIndex;

	iIndex = MapEditMapTilesetFindByValue(pApp, sValue);
	if ( iIndex >= 0 ) {
		return pApp->arrMapTilesetOptionText[iIndex];
	}
	return "未设置";
}

static const char* MapEditMapTilesetValueForDisplay(mapedit_app_t* pApp, const char* sText)
{
	int iIndex;

	iIndex = MapEditMapTilesetFindByText(pApp, sText);
	if ( iIndex >= 0 ) {
		return pApp->arrMapTilesetOptionValue[iIndex];
	}
	iIndex = MapEditMapTilesetFindByValue(pApp, sText);
	if ( iIndex >= 0 ) {
		return pApp->arrMapTilesetOptionValue[iIndex];
	}
	return "";
}

static xvalue MapEditMapBuildPassageOverridesValue(mapedit_map_item_t* pItem)
{
	xvalue pArray;
	xvalue pEntry;
	int i;
	int iCellCount;

	pArray = xvoCreateArray();
	if ( pArray == NULL ) {
		return NULL;
	}
	if ( pItem == NULL ) {
		return pArray;
	}
	iCellCount = MapEditMapCellCount(pItem);
	for ( i = 0; i < pItem->iPassageOverrideCount; i++ ) {
		if ( pItem->arrPassageOverrides[i].iCellId < 0 || pItem->arrPassageOverrides[i].iCellId >= iCellCount ) {
			continue;
		}
		pEntry = xvoCreateTable();
		if ( pEntry == NULL ) {
			xvoUnref(pArray);
			return NULL;
		}
		if ( !xvoTableSetInt(pEntry, MAPEDIT_MAP_KEY_CELL, (uint32)strlen(MAPEDIT_MAP_KEY_CELL), pItem->arrPassageOverrides[i].iCellId) ||
		     !xvoTableSetInt(pEntry, MAPEDIT_MAP_KEY_VALUE, (uint32)strlen(MAPEDIT_MAP_KEY_VALUE), (int)pItem->arrPassageOverrides[i].iValue) ||
		     !xvoArrayAppendValue(pArray, pEntry, TRUE) ) {
			xvoUnref(pEntry);
			xvoUnref(pArray);
			return NULL;
		}
	}
	return pArray;
}

static xvalue MapEditMapBuildCellCustomDataValue(mapedit_map_item_t* pItem)
{
	mapedit_tileset_custom_channel_data_t* pData;
	mapedit_tileset_custom_value_t* pValue;
	xvalue pRoot;
	xvalue pArray;
	xvalue pEntry;
	int i;
	int j;
	int iCellCount;

	pRoot = xvoCreateTable();
	if ( pRoot == NULL ) {
		return NULL;
	}
	if ( pItem == NULL ) {
		return pRoot;
	}
	iCellCount = MapEditMapCellCount(pItem);
	for ( i = 0; i < pItem->iCellCustomDataCount && i < MAPEDIT_CUSTOM_CHANNEL_MAX; i++ ) {
		pData = &pItem->arrCellCustomData[i];
		if ( pData->sChannelId[0] == 0 || pData->iValueCount <= 0 ) {
			continue;
		}
		pArray = xvoCreateArray();
		if ( pArray == NULL ) {
			xvoUnref(pRoot);
			return NULL;
		}
		for ( j = 0; j < pData->iValueCount; j++ ) {
			pValue = &pData->arrValues[j];
			if ( pValue->iTileId < 0 || pValue->iTileId >= iCellCount ) {
				continue;
			}
			pEntry = xvoCreateTable();
			if ( pEntry == NULL ) {
				xvoUnref(pArray);
				xvoUnref(pRoot);
				return NULL;
			}
			if ( !xvoTableSetInt(pEntry, MAPEDIT_MAP_KEY_CELL, (uint32)strlen(MAPEDIT_MAP_KEY_CELL), pValue->iTileId) ||
			     !xvoTableSetText(pEntry, MAPEDIT_MAP_KEY_VALUE, (uint32)strlen(MAPEDIT_MAP_KEY_VALUE), (str)pValue->sValue, 0, FALSE) ||
			     !xvoArrayAppendValue(pArray, pEntry, TRUE) ) {
				xvoUnref(pEntry);
				xvoUnref(pArray);
				xvoUnref(pRoot);
				return NULL;
			}
		}
		if ( !xvoTableSetValue(pRoot, pData->sChannelId, (uint32)strlen(pData->sChannelId), pArray, TRUE) ) {
			xvoUnref(pArray);
			xvoUnref(pRoot);
			return NULL;
		}
	}
	return pRoot;
}

static int MapEditMapSaveItem(mapedit_app_t* pApp, int iIndex)
{
	mapedit_map_item_t* pItem;
	xvalue pRoot;
	xvalue pTiles;
	xvalue pPassageOverrides;
	xvalue pCellCustomData;
	str sPath;
	int iTileCount;
	int i;
	int iRet;

	if ( pApp == NULL || iIndex < 0 || iIndex >= pApp->iMapCount ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pItem = &pApp->arrMapItems[iIndex];
	if ( pItem->sFile[0] == 0 ) {
		MapEditMapMakeUniqueFile(pItem->sFile, sizeof(pItem->sFile));
	}
	MapEditMapNormalize(pApp, pItem);
	if ( MapEditMapEnsureDir() != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( iIndex == pApp->iMapSelected && MapEditMapEnsureTileData(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	MapEditMapCompactRuntimeData(pItem);
	pRoot = xvoCreateTable();
	pTiles = xvoCreateArray();
	if ( pRoot == NULL || pTiles == NULL ) {
		xvoUnref(pRoot);
		xvoUnref(pTiles);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iTileCount = (iIndex == pApp->iMapSelected) ? pApp->iMapTileDataCount : 0;
	if ( !xvoTableSetText(pRoot, MAPEDIT_MAP_KEY_NAME, (uint32)strlen(MAPEDIT_MAP_KEY_NAME), (str)pItem->sName, 0, FALSE) ||
	     !xvoTableSetText(pRoot, MAPEDIT_MAP_KEY_TILESET, (uint32)strlen(MAPEDIT_MAP_KEY_TILESET), (str)pItem->sTileset, 0, FALSE) ||
	     !xvoTableSetInt(pRoot, MAPEDIT_MAP_KEY_STATE, (uint32)strlen(MAPEDIT_MAP_KEY_STATE), pItem->iState) ||
	     !xvoTableSetInt(pRoot, MAPEDIT_MAP_KEY_WIDTH, (uint32)strlen(MAPEDIT_MAP_KEY_WIDTH), pItem->iWidth) ||
	     !xvoTableSetInt(pRoot, MAPEDIT_MAP_KEY_HEIGHT, (uint32)strlen(MAPEDIT_MAP_KEY_HEIGHT), pItem->iHeight) ||
	     !xvoTableSetText(pRoot, MAPEDIT_MAP_KEY_CUSTOM_DATA, (uint32)strlen(MAPEDIT_MAP_KEY_CUSTOM_DATA), (str)pItem->sCustomData, 0, FALSE) ) {
		xvoUnref(pRoot);
		xvoUnref(pTiles);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < iTileCount; i++ ) {
		if ( !xvoArrayAppendInt(pTiles, pApp->pMapTileData[i]) ) {
			xvoUnref(pRoot);
			xvoUnref(pTiles);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( !xvoTableSetValue(pRoot, MAPEDIT_MAP_KEY_TILES, (uint32)strlen(MAPEDIT_MAP_KEY_TILES), pTiles, TRUE) ) {
		xvoUnref(pRoot);
		xvoUnref(pTiles);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pPassageOverrides = MapEditMapBuildPassageOverridesValue(pItem);
	pCellCustomData = MapEditMapBuildCellCustomDataValue(pItem);
	if ( pPassageOverrides == NULL || pCellCustomData == NULL ) {
		xvoUnref(pRoot);
		xvoUnref(pPassageOverrides);
		xvoUnref(pCellCustomData);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( !xvoTableSetValue(pRoot, MAPEDIT_MAP_KEY_PASSAGE_OVERRIDES, (uint32)strlen(MAPEDIT_MAP_KEY_PASSAGE_OVERRIDES), pPassageOverrides, TRUE) ||
	     !xvoTableSetValue(pRoot, MAPEDIT_MAP_KEY_CELL_CUSTOM_DATA, (uint32)strlen(MAPEDIT_MAP_KEY_CELL_CUSTOM_DATA), pCellCustomData, TRUE) ) {
		xvoUnref(pRoot);
		xvoUnref(pPassageOverrides);
		xvoUnref(pCellCustomData);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	sPath = MapEditMapFilePath(pItem->sFile);
	if ( sPath == NULL ) {
		xvoUnref(pRoot);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xrtStringifyXSON_File(sPath, pRoot, 1, 0) ? XGE_OK : XGE_ERROR;
	xrtFree(sPath);
	xvoUnref(pRoot);
	return iRet;
}

int MapEditMapSaveSelected(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return MapEditMapSaveItem(pApp, pApp->iMapSelected);
}

static void MapEditMapLoadPassageOverrides(mapedit_map_item_t* pItem, xvalue pRoot)
{
	xvalue pArray;
	xvalue pEntry;
	int i;
	int iCount;
	int iCellCount;
	int iCellId;
	int iValue;

	if ( pItem == NULL || pRoot == NULL || xvoType(pRoot) != XVO_DT_TABLE ) {
		return;
	}
	free(pItem->arrPassageOverrides);
	pItem->arrPassageOverrides = NULL;
	pItem->iPassageOverrideCount = 0;
	pItem->iPassageOverrideCapacity = 0;
	pArray = xvoTableGetValue(pRoot, MAPEDIT_MAP_KEY_PASSAGE_OVERRIDES, (uint32)strlen(MAPEDIT_MAP_KEY_PASSAGE_OVERRIDES));
	if ( xvoType(pArray) != XVO_DT_ARRAY ) {
		return;
	}
	iCellCount = MapEditMapCellCount(pItem);
	iCount = (int)xvoArrayItemCount(pArray);
	for ( i = 0; i < iCount; i++ ) {
		pEntry = xvoArrayGetValue(pArray, (uint32)i);
		if ( xvoType(pEntry) != XVO_DT_TABLE ) {
			continue;
		}
		iCellId = MapEditMapReadInt(pEntry, MAPEDIT_MAP_KEY_CELL, -1);
		iValue = MapEditMapReadInt(pEntry, MAPEDIT_MAP_KEY_VALUE, MAPEDIT_TILESET_PASSAGE_DEFAULT);
		if ( iCellId < 0 || iCellId >= iCellCount ) {
			continue;
		}
		if ( iValue < 0 ) {
			iValue = 0;
		} else if ( iValue > 255 ) {
			iValue = 255;
		}
		MapEditMapSetPassageOverride(pItem, iCellId, (unsigned char)iValue);
	}
}

static void MapEditMapLoadCellCustomData(mapedit_app_t* pApp, mapedit_map_item_t* pItem, xvalue pRoot)
{
	const mapedit_custom_channel_def_t* pDef;
	xvalue pCellCustomData;
	xvalue pArray;
	xvalue pEntry;
	xvalue pValue;
	char sValue[MAPEDIT_CUSTOM_VALUE_CAPACITY];
	int i;
	int j;
	int iCount;
	int iCellCount;
	int iCellId;

	if ( pApp == NULL || pItem == NULL || pRoot == NULL || xvoType(pRoot) != XVO_DT_TABLE ) {
		return;
	}
	MapEditMapClearCellCustomData(pItem);
	pCellCustomData = xvoTableGetValue(pRoot, MAPEDIT_MAP_KEY_CELL_CUSTOM_DATA, (uint32)strlen(MAPEDIT_MAP_KEY_CELL_CUSTOM_DATA));
	if ( xvoType(pCellCustomData) != XVO_DT_TABLE ) {
		return;
	}
	iCellCount = MapEditMapCellCount(pItem);
	for ( i = 0; i < pApp->iCustomChannelCount; i++ ) {
		pDef = &pApp->arrCustomChannels[i];
		if ( pDef->sId[0] == 0 ) {
			continue;
		}
		pArray = xvoTableGetValue(pCellCustomData, pDef->sId, (uint32)strlen(pDef->sId));
		if ( xvoType(pArray) != XVO_DT_ARRAY ) {
			continue;
		}
		iCount = (int)xvoArrayItemCount(pArray);
		for ( j = 0; j < iCount; j++ ) {
			pEntry = xvoArrayGetValue(pArray, (uint32)j);
			if ( xvoType(pEntry) != XVO_DT_TABLE ) {
				continue;
			}
			iCellId = MapEditMapReadInt(pEntry, MAPEDIT_MAP_KEY_CELL, -1);
			if ( iCellId < 0 || iCellId >= iCellCount ) {
				continue;
			}
			pValue = xvoTableGetValue(pEntry, MAPEDIT_MAP_KEY_VALUE, (uint32)strlen(MAPEDIT_MAP_KEY_VALUE));
			MapEditMapValueToText(pValue, sValue, sizeof(sValue));
			MapEditMapSetCellCustomOverride(pItem, pDef->sId, iCellId, sValue);
		}
	}
}

static int MapEditMapLoadFile(mapedit_app_t* pApp, const char* sPath, size_t iSize)
{
	mapedit_map_item_t* pItem;
	xvalue pRoot;
	str sFile;
	int iIndex;

	if ( pApp == NULL || sPath == NULL || pApp->iMapCount >= MAPEDIT_MAP_ITEM_MAX || !MapEditMapIsXsonFile(sPath) ) {
		return 0;
	}
	pRoot = xrtParseXSON_File((str)sPath);
	if ( pRoot == NULL || xvoType(pRoot) != XVO_DT_TABLE ) {
		xvoUnref(pRoot);
		return 0;
	}
	sFile = xrtPathGetNameExt((str)sPath, iSize);
	if ( sFile == NULL || sFile[0] == 0 ) {
		xvoUnref(pRoot);
		xrtFree(sFile);
		return 0;
	}
	iIndex = pApp->iMapCount++;
	pItem = &pApp->arrMapItems[iIndex];
	memset(pItem, 0, sizeof(*pItem));
	MapEditMapCopyText(pItem->sFile, sizeof(pItem->sFile), (const char*)sFile);
	MapEditMapCopyText(pItem->sName, sizeof(pItem->sName), MapEditMapReadText(pRoot, MAPEDIT_MAP_KEY_NAME, "未命名地图"));
	MapEditMapCopyText(pItem->sTileset, sizeof(pItem->sTileset), MapEditMapReadText(pRoot, MAPEDIT_MAP_KEY_TILESET, ""));
	MapEditMapCopyText(pItem->sCustomData, sizeof(pItem->sCustomData), MapEditMapReadText(pRoot, MAPEDIT_MAP_KEY_CUSTOM_DATA, ""));
	pItem->iState = MapEditMapReadInt(pRoot, MAPEDIT_MAP_KEY_STATE, MapEditMapSetupStateMin(pApp));
	pItem->iWidth = MapEditMapReadInt(pRoot, MAPEDIT_MAP_KEY_WIDTH, MAPEDIT_MAP_WIDTH_DEFAULT);
	pItem->iHeight = MapEditMapReadInt(pRoot, MAPEDIT_MAP_KEY_HEIGHT, MAPEDIT_MAP_HEIGHT_DEFAULT);
	pItem->iLayerCount = MapEditMapSetupLayerCount(pApp);
	MapEditMapNormalize(pApp, pItem);
	MapEditMapLoadPassageOverrides(pItem, pRoot);
	MapEditMapLoadCellCustomData(pApp, pItem, pRoot);
	MapEditMapBuildText(pItem);
	pApp->arrMapListItems[iIndex] = pItem->sText;
	xrtFree(sFile);
	xvoUnref(pRoot);
	return 1;
}

static int MapEditMapScanProc(str sPath, size_t iSize, int bDir, ptr pData, ptr pParam)
{
	mapedit_map_scan_t* pScan;

	(void)pData;
	pScan = (mapedit_map_scan_t*)pParam;
	if ( pScan == NULL || pScan->pApp == NULL || bDir ) {
		return 0;
	}
	if ( MapEditMapLoadFile(pScan->pApp, (const char*)sPath, iSize) ) {
		pScan->iLoadedCount++;
	}
	return 0;
}

static void MapEditMapLoadAll(mapedit_app_t* pApp)
{
	mapedit_map_scan_t tScan;
	str sDir;
	int iIndex;

	if ( pApp == NULL ) {
		return;
	}
	for ( iIndex = 0; iIndex < pApp->iMapCount && iIndex < MAPEDIT_MAP_ITEM_MAX; iIndex++ ) {
		MapEditMapFreeItemRuntimeData(&pApp->arrMapItems[iIndex]);
	}
	pApp->iMapCount = 0;
	pApp->iMapSelected = -1;
	if ( MapEditMapEnsureDir() != XGE_OK ) {
		iIndex = MapEditMapAppend(pApp, "默认地图", MAPEDIT_MAP_DEFAULT_FILE);
		pApp->iMapSelected = iIndex;
		return;
	}
	sDir = MapEditMapDirPath();
	if ( sDir != NULL && xrtDirExists(sDir) ) {
		memset(&tScan, 0, sizeof(tScan));
		tScan.pApp = pApp;
		xrtDirScan(sDir, 0, MapEditMapScanProc, &tScan);
	}
	xrtFree(sDir);
	if ( pApp->iMapCount <= 0 ) {
		iIndex = MapEditMapAppend(pApp, "默认地图", MAPEDIT_MAP_DEFAULT_FILE);
		if ( iIndex >= 0 ) {
			pApp->iMapSelected = iIndex;
			MapEditMapSaveItem(pApp, iIndex);
		}
	} else {
		pApp->iMapSelected = 0;
	}
}

static void MapEditMapRefreshList(mapedit_app_t* pApp)
{
	int i;

	if ( pApp == NULL || pApp->pMapListWidget == NULL ) {
		return;
	}
	for ( i = 0; i < pApp->iMapCount; i++ ) {
		MapEditMapBuildText(&pApp->arrMapItems[i]);
		pApp->arrMapListItems[i] = pApp->arrMapItems[i].sText;
	}
	xgeXuiListViewSetItems(&pApp->tMapList, pApp->arrMapListItems, pApp->iMapCount);
	xgeXuiListViewSetSelected(&pApp->tMapList, pApp->iMapSelected);
}

void MapEditMapEnsureDefaults(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->iMapCount <= 0 ) {
		MapEditMapLoadAll(pApp);
	}
	MapEditMapEnsureTileData(pApp);
	if ( pApp->pMapTilesetStaticTexture == NULL ) {
		MapEditMapLoadTilesetTexture(pApp);
	}
}

mapedit_map_item_t* MapEditMapSelectedItem(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iMapSelected < 0 || pApp->iMapSelected >= pApp->iMapCount ) {
		return NULL;
	}
	return &pApp->arrMapItems[pApp->iMapSelected];
}

static int MapEditMapLoadSelectedTileData(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pItem;
	xvalue pRoot;
	xvalue pTiles;
	xvalue pValue;
	str sPath;
	int iTileCount;
	int iLoadCount;
	int i;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pItem = MapEditMapSelectedItem(pApp);
	if ( pItem == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iTileCount = MapEditMapTileCount(pApp, pItem);
	if ( iTileCount <= 0 ) {
		return XGE_ERROR;
	}
	free(pApp->pMapTileData);
	pApp->pMapTileData = (int*)calloc((size_t)iTileCount, sizeof(int));
	if ( pApp->pMapTileData == NULL ) {
		pApp->iMapTileDataCount = 0;
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pApp->iMapTileDataCount = iTileCount;
	sPath = MapEditMapFilePath(pItem->sFile);
	if ( sPath == NULL || !xrtFileExists(sPath) ) {
		xrtFree(sPath);
		return XGE_OK;
	}
	pRoot = xrtParseXSON_File(sPath);
	xrtFree(sPath);
	if ( pRoot == NULL || xvoType(pRoot) != XVO_DT_TABLE ) {
		xvoUnref(pRoot);
		return XGE_OK;
	}
	pTiles = xvoTableGetValue(pRoot, MAPEDIT_MAP_KEY_TILES, (uint32)strlen(MAPEDIT_MAP_KEY_TILES));
	if ( xvoType(pTiles) == XVO_DT_ARRAY ) {
		iLoadCount = (int)xvoArrayItemCount(pTiles);
		if ( iLoadCount > iTileCount ) {
			iLoadCount = iTileCount;
		}
		for ( i = 0; i < iLoadCount; i++ ) {
			pValue = xvoArrayGetValue(pTiles, (uint32)i);
			if ( xvoType(pValue) != XVO_DT_NULL ) {
				pApp->pMapTileData[i] = (int)xvoGetInt(pValue);
			}
		}
	}
	xvoUnref(pRoot);
	return XGE_OK;
}

int MapEditMapEnsureTileData(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pItem;
	int iTileCount;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pItem = MapEditMapSelectedItem(pApp);
	if ( pItem == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iTileCount = MapEditMapTileCount(pApp, pItem);
	if ( iTileCount <= 0 ) {
		return XGE_ERROR;
	}
	if ( pApp->pMapTileData != NULL && pApp->iMapTileDataCount == iTileCount ) {
		return XGE_OK;
	}
	return MapEditMapLoadSelectedTileData(pApp);
}

static int MapEditMapResizeTileData(mapedit_app_t* pApp, int iOldWidth, int iOldHeight, int iOldLayers)
{
	mapedit_map_item_t* pItem;
	int* pOld;
	int* pNew;
	int iOldCount;
	int iNewCount;
	int iCopyW;
	int iCopyH;
	int iCopyL;
	int l;
	int y;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pItem = MapEditMapSelectedItem(pApp);
	if ( pItem == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iOldCount = pApp->iMapTileDataCount;
	pOld = pApp->pMapTileData;
	iNewCount = MapEditMapTileCount(pApp, pItem);
	if ( iNewCount <= 0 ) {
		return XGE_ERROR;
	}
	pNew = (int*)calloc((size_t)iNewCount, sizeof(int));
	if ( pNew == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iCopyW = (iOldWidth < pItem->iWidth) ? iOldWidth : pItem->iWidth;
	iCopyH = (iOldHeight < pItem->iHeight) ? iOldHeight : pItem->iHeight;
	iCopyL = (iOldLayers < pItem->iLayerCount) ? iOldLayers : pItem->iLayerCount;
	if ( pOld != NULL && iOldCount > 0 && iCopyW > 0 && iCopyH > 0 && iCopyL > 0 ) {
		for ( l = 0; l < iCopyL; l++ ) {
			for ( y = 0; y < iCopyH; y++ ) {
				memcpy(&pNew[l * pItem->iWidth * pItem->iHeight + y * pItem->iWidth],
					&pOld[l * iOldWidth * iOldHeight + y * iOldWidth],
					(size_t)iCopyW * sizeof(int));
			}
		}
	}
	free(pOld);
	pApp->pMapTileData = pNew;
	pApp->iMapTileDataCount = iNewCount;
	return XGE_OK;
}

int MapEditMapTilesPerRow(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTilesPerRow <= 0 ) {
		return 20;
	}
	return pApp->iSetupTilesPerRow;
}

int MapEditMapTileWidth(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTileWidth <= 0 ) {
		return 16;
	}
	return pApp->iSetupTileWidth;
}

int MapEditMapTileHeight(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iSetupTileHeight <= 0 ) {
		return 16;
	}
	return pApp->iSetupTileHeight;
}

int MapEditMapStaticStart(mapedit_app_t* pApp, mapedit_tileset_set_item_t* pTileset)
{
	int iReserved;
	int iPerRow;

	iPerRow = MapEditMapTilesPerRow(pApp);
	iReserved = 1 + ((pTileset != NULL) ? pTileset->iSpecialTileCount : 0);
	if ( iReserved < 1 ) {
		iReserved = 1;
	}
	return ((iReserved + iPerRow - 1) / iPerRow) * iPerRow;
}

int MapEditMapStaticCols(mapedit_app_t* pApp)
{
	int iCellW;
	int iCols;
	int iPerRow;

	if ( pApp == NULL || pApp->tMapTilesetStaticDesc.iWidth <= 0 ) {
		return 0;
	}
	iCellW = MapEditMapTileWidth(pApp);
	iPerRow = MapEditMapTilesPerRow(pApp);
	iCols = (pApp->tMapTilesetStaticDesc.iWidth + iCellW - 1) / iCellW;
	if ( iCols > iPerRow ) {
		iCols = iPerRow;
	}
	return (iCols > 0) ? iCols : 0;
}

int MapEditMapStaticRows(mapedit_app_t* pApp)
{
	int iCellH;
	int iRows;

	if ( pApp == NULL || pApp->tMapTilesetStaticDesc.iHeight <= 0 ) {
		return 8;
	}
	iCellH = MapEditMapTileHeight(pApp);
	iRows = (pApp->tMapTilesetStaticDesc.iHeight + iCellH - 1) / iCellH;
	return (iRows > 0) ? iRows : 1;
}

void MapEditMapClearTilesetTexture(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pMapTilesetStaticTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pApp->pMapTilesetStaticTexture);
		pApp->pMapTilesetStaticTexture = NULL;
	}
	memset(&pApp->tMapTilesetStaticDesc, 0, sizeof(pApp->tMapTilesetStaticDesc));
}

void MapEditMapLoadTilesetTexture(mapedit_app_t* pApp)
{
	mapedit_tileset_set_item_t* pTileset;
	str sPath;
	xui_texture pTexture;
	xui_texture_desc_t tDesc;

	MapEditMapClearTilesetTexture(pApp);
	pTileset = MapEditMapSelectedTileset(pApp);
	if ( pApp == NULL || pTileset == NULL || pTileset->sStaticTiles[0] == 0 ) {
		return;
	}
	sPath = xrtPathJoin(4, xCore.AppPath, (str)"assets", (str)"tilesheets", (str)pTileset->sStaticTiles);
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
		pApp->pMapTilesetStaticTexture = pTexture;
		pApp->tMapTilesetStaticDesc = tDesc;
	} else if ( pTexture != NULL ) {
		xgeXuiTextureDestroy(&pApp->tXui, pTexture);
	}
	xrtFree(sPath);
}

static int MapEditMapDrawSpecialTile(mapedit_app_t* pApp, mapedit_tile_grid_t* pGrid, int iTileId, xge_rect_t tCell, int iLayer, int iCol, int iRow)
{
	mapedit_tileset_set_item_t* pTileset;
	mapedit_tileset_special_tile_t* pSpecial;
	xui_texture_desc_t* pDesc;
	xge_draw_t tDraw;
	char sText[32];
	float fSrcW;
	float fSrcH;
	float fSrcX;
	float fSrcY;
	int iBlobIndex;
	int iSlot;
	int iStateIndex;
	int iStateBlockH;
	int iAvailableStates;
	int iFrameBlockW;
	int iFrameCount;
	int iFrameIndex;
	int iTileW;
	int iTileH;

	if ( pApp == NULL || iTileId <= 0 ) {
		return 0;
	}
	pTileset = MapEditMapSelectedTileset(pApp);
	if ( pTileset == NULL || iTileId > pTileset->iSpecialTileCount ) {
		return 0;
	}
	iSlot = iTileId - 1;
	if ( iSlot < 0 || iSlot >= MAPEDIT_TILESET_SPECIAL_TILE_MAX ) {
		return 0;
	}
	pSpecial = &pTileset->arrSpecialTiles[iSlot];
	if ( pApp->arrMapTilesetSpecialTexture[iSlot] != NULL ) {
		pDesc = &pApp->arrMapTilesetSpecialDesc[iSlot];
		iTileW = MapEditMapTileWidth(pApp);
		iTileH = MapEditMapTileHeight(pApp);
		fSrcX = 0.0f;
		fSrcY = 0.0f;
		fSrcW = (float)iTileW;
		fSrcH = (float)iTileH;
		if ( MapEditMapSpecialTileIsBlob47(pSpecial) &&
		     iLayer >= 0 && iCol >= 0 && iRow >= 0 &&
		     pDesc->iWidth >= iTileW * MAPEDIT_BLOB47_COLS &&
		     pDesc->iHeight >= iTileH * MAPEDIT_BLOB47_ROWS ) {
			iBlobIndex = MapEditMapBlob47IndexFromMask(MapEditMapBlob47MaskForCell(pApp, iLayer, iCol, iRow, iTileId));
			fSrcX = (float)((iBlobIndex % MAPEDIT_BLOB47_COLS) * iTileW);
			fSrcY = (float)((iBlobIndex / MAPEDIT_BLOB47_COLS) * iTileH);
		}
		if ( MapEditMapSpecialTileHasState(pSpecial) ) {
			iStateIndex = MapEditMapCurrentStateIndex(pApp);
			iStateBlockH = MapEditMapSpecialTileIsBlob47(pSpecial) ? (iTileH * MAPEDIT_BLOB47_ROWS) : iTileH;
			iAvailableStates = (iStateBlockH > 0) ? (pDesc->iHeight / iStateBlockH) : 0;
			if ( iAvailableStates > 0 ) {
				if ( iStateIndex >= iAvailableStates ) {
					iStateIndex = iAvailableStates - 1;
				}
				if ( iStateIndex < 0 ) {
					iStateIndex = 0;
				}
				fSrcY += (float)(iStateIndex * iStateBlockH);
			}
		}
		if ( pApp->bMapPreviewMode ) {
			iFrameBlockW = MapEditMapSpecialTileFrameBlockWidth(pSpecial, iTileW);
			iFrameCount = (iFrameBlockW > 0) ? (pDesc->iWidth / iFrameBlockW) : 0;
			if ( iFrameCount > 1 ) {
				iFrameIndex = pApp->iMapPreviewAnimFrame % iFrameCount;
				if ( iFrameIndex < 0 ) {
					iFrameIndex = 0;
				}
				fSrcX += (float)(iFrameIndex * iFrameBlockW);
			}
		}
		if ( fSrcX >= (float)pDesc->iWidth || fSrcY >= (float)pDesc->iHeight ) {
			return 1;
		}
		if ( fSrcW > (float)pDesc->iWidth - fSrcX ) {
			fSrcW = (float)pDesc->iWidth - fSrcX;
		}
		if ( fSrcH > (float)pDesc->iHeight - fSrcY ) {
			fSrcH = (float)pDesc->iHeight - fSrcY;
		}
		if ( fSrcW <= 0.0f || fSrcH <= 0.0f ) {
			return 1;
		}
		memset(&tDraw, 0, sizeof(tDraw));
		tDraw.pTexture = (xge_texture)pApp->arrMapTilesetSpecialTexture[iSlot];
		tDraw.tSrc = (xge_rect_t){ fSrcX, fSrcY, fSrcW, fSrcH };
		tDraw.tDst = tCell;
		tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
		xgeDrawEx(&tDraw);
		xgeFlush();
		return 1;
	}
	xgeShapeRectFillPx(tCell, XGE_COLOR_RGBA(246, 218, 158, 205));
	xgeShapeRectStrokePx(tCell, 1.0f, XGE_COLOR_RGBA(190, 140, 42, 220));
	if ( pGrid != NULL && pGrid->iCellWidth >= 18 && pGrid->iCellHeight >= 14 ) {
		snprintf(sText, sizeof(sText), "%d", iTileId);
		xgeTextDrawRect(pGrid->pFont, sText, tCell, XGE_COLOR_RGBA(96, 72, 24, 255),
			XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP | XGE_TEXT_SCREEN_SPACE);
	}
	return 1;
}

static void MapEditMapDrawTileAt(mapedit_app_t* pApp, mapedit_tile_grid_t* pGrid, int iTileId, xge_rect_t tCell, int iLayer, int iCol, int iRow)
{
	mapedit_tileset_set_item_t* pTileset;
	xge_draw_t tDraw;
	int iStaticStart;
	int iLocal;
	int iSrcCol;
	int iSrcRow;
	int iTileW;
	int iTileH;

	if ( pApp == NULL || iTileId <= 0 ) {
		return;
	}
	if ( MapEditMapDrawSpecialTile(pApp, pGrid, iTileId, tCell, iLayer, iCol, iRow) ) {
		return;
	}
	if ( pApp->pMapTilesetStaticTexture == NULL ) {
		return;
	}
	pTileset = MapEditMapSelectedTileset(pApp);
	iStaticStart = MapEditMapStaticStart(pApp, pTileset);
	if ( iTileId < iStaticStart ) {
		return;
	}
	iTileW = MapEditMapTileWidth(pApp);
	iTileH = MapEditMapTileHeight(pApp);
	iLocal = iTileId - iStaticStart;
	iSrcCol = iLocal % MapEditMapTilesPerRow(pApp);
	iSrcRow = iLocal / MapEditMapTilesPerRow(pApp);
	if ( iSrcCol < 0 || iSrcCol >= MapEditMapStaticCols(pApp) || iSrcRow < 0 || iSrcRow >= MapEditMapStaticRows(pApp) ) {
		return;
	}
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = (xge_texture)pApp->pMapTilesetStaticTexture;
	tDraw.tSrc = (xge_rect_t){ (float)(iSrcCol * iTileW), (float)(iSrcRow * iTileH), (float)iTileW, (float)iTileH };
	tDraw.tDst = tCell;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
	xgeFlush();
}

void MapEditMapDrawTile(mapedit_app_t* pApp, mapedit_tile_grid_t* pGrid, int iTileId, xge_rect_t tCell)
{
	MapEditMapDrawTileAt(pApp, pGrid, iTileId, tCell, -1, -1, -1);
}

void MapEditMapDrawCell(mapedit_app_t* pApp, mapedit_tile_grid_t* pGrid, int iCol, int iRow, xge_rect_t tCell)
{
	mapedit_map_item_t* pMap;
	int iLayer;
	int iTileId;

	pMap = MapEditMapSelectedItem(pApp);
	if ( pApp == NULL || pMap == NULL || pApp->pMapTileData == NULL || iCol < 0 || iRow < 0 || iCol >= pMap->iWidth || iRow >= pMap->iHeight ) {
		return;
	}
	for ( iLayer = 0; iLayer < pMap->iLayerCount; iLayer++ ) {
		iTileId = MapEditMapGetTileAt(pApp, iLayer, iCol, iRow);
		MapEditMapDrawTileAt(pApp, pGrid, iTileId, tCell, iLayer, iCol, iRow);
	}
}

unsigned char MapEditMapDefaultPassageForCell(mapedit_app_t* pApp, int x, int y)
{
	mapedit_tileset_set_item_t* pTileset;
	int iTileId;

	iTileId = MapEditMapTopTileAt(pApp, x, y);
	if ( iTileId <= 0 ) {
		return (unsigned char)MAPEDIT_TILESET_PASSAGE_DEFAULT;
	}
	pTileset = MapEditMapSelectedTileset(pApp);
	if ( pTileset == NULL || iTileId < 0 || iTileId >= pTileset->iPassageCount || iTileId >= MAPEDIT_TILESET_PASSAGE_MAX ) {
		return (unsigned char)MAPEDIT_TILESET_PASSAGE_DEFAULT;
	}
	return pTileset->arrPassage[iTileId];
}

unsigned char MapEditMapGetCellPassage(mapedit_app_t* pApp, int x, int y)
{
	mapedit_map_item_t* pMap;
	int iCellId;
	int iIndex;

	pMap = MapEditMapSelectedItem(pApp);
	iCellId = MapEditMapCellId(pMap, x, y);
	if ( iCellId < 0 ) {
		return (unsigned char)MAPEDIT_TILESET_PASSAGE_DEFAULT;
	}
	iIndex = MapEditMapFindPassageOverride(pMap, iCellId);
	if ( iIndex >= 0 ) {
		return pMap->arrPassageOverrides[iIndex].iValue;
	}
	return MapEditMapDefaultPassageForCell(pApp, x, y);
}

int MapEditMapSetCellPassage(mapedit_app_t* pApp, int x, int y, unsigned char iValue)
{
	mapedit_map_item_t* pMap;
	unsigned char iDefault;
	int iCellId;
	int iIndex;

	pMap = MapEditMapSelectedItem(pApp);
	iCellId = MapEditMapCellId(pMap, x, y);
	if ( pApp == NULL || pMap == NULL || iCellId < 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iDefault = MapEditMapDefaultPassageForCell(pApp, x, y);
	iIndex = MapEditMapFindPassageOverride(pMap, iCellId);
	if ( iValue == iDefault ) {
		if ( iIndex >= 0 ) {
			MapEditMapRemovePassageOverride(pMap, iIndex);
		}
	} else if ( MapEditMapSetPassageOverride(pMap, iCellId, iValue) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( pApp->pMapPassageGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pMapPassageGridWidget);
	}
	return XGE_OK;
}

static void MapEditMapGetCellCustomFallback(mapedit_app_t* pApp, const mapedit_custom_channel_def_t* pDef, int x, int y, char* sValue, int iValueSize)
{
	mapedit_tileset_set_item_t* pTileset;
	int iTileId;

	if ( sValue != NULL && iValueSize > 0 ) {
		MapEditMapCopyText(sValue, iValueSize, (pDef != NULL) ? pDef->sDefaultValue : "");
	}
	if ( pApp == NULL || pDef == NULL ) {
		return;
	}
	if ( pDef->sScope[0] != 0 && strcmp(pDef->sScope, "tile") != 0 ) {
		return;
	}
	iTileId = MapEditMapTopTileAt(pApp, x, y);
	if ( iTileId < 0 ) {
		iTileId = 0;
	}
	pTileset = MapEditMapSelectedTileset(pApp);
	if ( pTileset == NULL ) {
		return;
	}
	if ( !MapEditTilesetSetGetTileCustomValue(pTileset, pDef->sId, iTileId, sValue, iValueSize) ) {
		MapEditMapCopyText(sValue, iValueSize, pDef->sDefaultValue);
	}
}

int MapEditMapGetCellCustomValue(mapedit_app_t* pApp, const char* sChannelId, int x, int y, char* sValue, int iValueSize)
{
	mapedit_map_item_t* pMap;
	const mapedit_custom_channel_def_t* pDef;
	int iCellId;

	if ( sValue != NULL && iValueSize > 0 ) {
		sValue[0] = 0;
	}
	pMap = MapEditMapSelectedItem(pApp);
	iCellId = MapEditMapCellId(pMap, x, y);
	if ( pApp == NULL || pMap == NULL || sChannelId == NULL || iCellId < 0 ) {
		return 0;
	}
	if ( MapEditMapGetCellCustomOverride(pMap, sChannelId, iCellId, sValue, iValueSize) ) {
		return 1;
	}
	pDef = MapEditCustomChannelFind(pApp, sChannelId);
	MapEditMapGetCellCustomFallback(pApp, pDef, x, y, sValue, iValueSize);
	return 0;
}

int MapEditMapSetCellCustomValue(mapedit_app_t* pApp, const char* sChannelId, int x, int y, const char* sValue)
{
	mapedit_map_item_t* pMap;
	const mapedit_custom_channel_def_t* pDef;
	mapedit_tileset_custom_channel_data_t* pData;
	char sFallback[MAPEDIT_CUSTOM_VALUE_CAPACITY];
	int iCellId;
	int iIndex;

	pMap = MapEditMapSelectedItem(pApp);
	iCellId = MapEditMapCellId(pMap, x, y);
	if ( pApp == NULL || pMap == NULL || sChannelId == NULL || sChannelId[0] == 0 || iCellId < 0 ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pDef = MapEditCustomChannelFind(pApp, sChannelId);
	MapEditMapGetCellCustomFallback(pApp, pDef, x, y, sFallback, sizeof(sFallback));
	if ( sValue == NULL ) {
		sValue = "";
	}
	pData = MapEditMapFindCellCustomChannelData(pMap, sChannelId);
	iIndex = MapEditMapFindCellCustomValue(pData, iCellId);
	if ( strcmp(sValue, sFallback) == 0 ) {
		if ( iIndex >= 0 ) {
			MapEditMapRemoveCellCustomValue(pData, iIndex);
		}
	} else if ( MapEditMapSetCellCustomOverride(pMap, sChannelId, iCellId, sValue) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( pApp->pMapTagsGridWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pApp->pMapTagsGridWidget);
	}
	return XGE_OK;
}

void MapEditMapRefreshDependents(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pItem;
	char sSize[64];

	if ( pApp == NULL ) {
		return;
	}
	MapEditMapLoadTilesetTexture(pApp);
	MapEditMapRefreshList(pApp);
	MapEditMapPropertiesRefresh(pApp);
	MapEditMapTileSelectRefresh(pApp);
	MapEditMapEditRefresh(pApp);
	MapEditMapPassageRefresh(pApp);
	MapEditMapTagsRefresh(pApp);
	pItem = MapEditMapSelectedItem(pApp);
	if ( pItem != NULL && pApp->tStatusBar.iItemCount > 1 ) {
		snprintf(sSize, sizeof(sSize), "地图: %d x %d", pItem->iWidth, pItem->iHeight);
		xgeXuiStatusBarSetItemText(&pApp->tStatusBar, 1, sSize);
	}
}

static void MapEditMapSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || iIndex < 0 || iIndex >= pApp->iMapCount ) {
		return;
	}
	if ( pApp->iMapSelected == iIndex ) {
		return;
	}
	pApp->iMapSelected = iIndex;
	pApp->iMapBrushTileId = -1;
	pApp->iMapActiveLayer = 0;
	MapEditMapEditClearHistory(pApp);
	MapEditMapLoadSelectedTileData(pApp);
	MapEditMapRefreshDependents(pApp);
	MapEditAppSetStatus(pApp, "已切换地图");
}

static void MapEditMapAddClick(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;
	char sName[64];
	int iIndex;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->iMapCount >= MAPEDIT_MAP_ITEM_MAX ) {
		MapEditAppSetStatus(pApp, "地图数量已达到上限");
		return;
	}
	snprintf(sName, sizeof(sName), "新建地图%d", pApp->iMapCount + 1);
	iIndex = MapEditMapAppend(pApp, sName, NULL);
	if ( iIndex < 0 ) {
		MapEditAppSetStatus(pApp, "添加地图失败");
		return;
	}
	pApp->iMapSelected = iIndex;
	pApp->iMapActiveLayer = 0;
	MapEditMapEditClearHistory(pApp);
	MapEditMapLoadSelectedTileData(pApp);
	if ( MapEditMapSaveItem(pApp, iIndex) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "已添加地图，但保存失败");
	} else {
		MapEditAppSetStatus(pApp, "已添加地图");
	}
	MapEditMapRefreshDependents(pApp);
}

static int MapEditMapAddPropertyEx(xge_xui_property_grid pGrid, int iCategory, const char* sId, const char* sName, const char* sDescription, int iType, const char* sValue, const char* sDefault, int iFlags)
{
	xge_xui_property_desc_t tDesc;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.sId = sId;
	tDesc.sName = sName;
	tDesc.sDescription = sDescription;
	tDesc.iType = iType;
	tDesc.sValue = sValue;
	tDesc.sDefaultValue = sDefault;
	tDesc.iFlags = iFlags;
	return xgeXuiPropertyGridAddProperty(pGrid, iCategory, &tDesc);
}

static int MapEditMapAddProperty(xge_xui_property_grid pGrid, int iCategory, const char* sId, const char* sName, const char* sDescription, int iType, const char* sValue, const char* sDefault)
{
	return MapEditMapAddPropertyEx(pGrid, iCategory, sId, sName, sDescription, iType, sValue, sDefault, 0);
}

static void MapEditMapPropertiesBuildEmpty(mapedit_app_t* pApp)
{
	xge_xui_property_grid pGrid;
	int iCategory;

	if ( pApp == NULL || pApp->pMapPropertiesGridWidget == NULL ) {
		return;
	}
	pGrid = &pApp->tMapPropertiesGrid;
	xgeXuiPropertyGridClear(pGrid);
	iCategory = xgeXuiPropertyGridAddCategory(pGrid, "empty", "地图属性", 1);
	if ( iCategory >= 0 ) {
		MapEditMapAddPropertyEx(pGrid, iCategory, "empty.hint", "当前对象", "选择地图后显示地图属性", XGE_XUI_TABLE_CELL_TYPE_TEXT, "未选择", "未选择", XGE_XUI_PROPERTY_FLAG_READONLY);
	}
}

void MapEditMapPropertiesShowCellCustomData(mapedit_app_t* pApp, int x, int y)
{
	mapedit_map_item_t* pMap;
	const mapedit_custom_channel_def_t* pDef;
	xge_xui_property_grid pGrid;
	char sValue[MAPEDIT_CUSTOM_VALUE_CAPACITY];
	char sDisplay[MAPEDIT_CUSTOM_VALUE_CAPACITY];
	char sPropId[MAPEDIT_CUSTOM_ID_CAPACITY + 16];
	char sPosition[64];
	char sTileId[32];
	char sPassage[32];
	int iCategory;
	int iTileId;
	int i;

	if ( pApp == NULL || pApp->pMapPropertiesGridWidget == NULL ) {
		return;
	}
	pMap = MapEditMapSelectedItem(pApp);
	if ( pMap == NULL || x < 0 || y < 0 || x >= pMap->iWidth || y >= pMap->iHeight ) {
		MapEditMapPropertiesBuildEmpty(pApp);
		return;
	}
	pGrid = &pApp->tMapPropertiesGrid;
	xgeXuiPropertyGridClear(pGrid);
	iCategory = xgeXuiPropertyGridAddCategory(pGrid, "map.cell_custom", "地图位置自定义数据", 1);
	if ( iCategory < 0 ) {
		return;
	}
	iTileId = MapEditMapTopTileAt(pApp, x, y);
	snprintf(sPosition, sizeof(sPosition), "%d, %d", x, y);
	snprintf(sTileId, sizeof(sTileId), "%d", iTileId);
	snprintf(sPassage, sizeof(sPassage), "%u", (unsigned int)MapEditMapGetCellPassage(pApp, x, y));
	MapEditMapAddPropertyEx(pGrid, iCategory, "cell.position", "地图位置", "当前检查的地图位置", XGE_XUI_TABLE_CELL_TYPE_TEXT, sPosition, sPosition, XGE_XUI_PROPERTY_FLAG_READONLY);
	MapEditMapAddPropertyEx(pGrid, iCategory, "cell.tile", "参考图块", "当前位置最上层非空图块", XGE_XUI_TABLE_CELL_TYPE_INT, sTileId, sTileId, XGE_XUI_PROPERTY_FLAG_READONLY);
	MapEditMapAddPropertyEx(pGrid, iCategory, "cell.passage", "通行数据", "当前位置最终通行修正值", XGE_XUI_TABLE_CELL_TYPE_INT, sPassage, sPassage, XGE_XUI_PROPERTY_FLAG_READONLY);
	for ( i = 0; i < pApp->iCustomChannelCount; i++ ) {
		pDef = &pApp->arrCustomChannels[i];
		if ( pDef->sId[0] == 0 || (pDef->sScope[0] != 0 && strcmp(pDef->sScope, "tile") != 0 && strcmp(pDef->sScope, "cell") != 0) ) {
			continue;
		}
		MapEditMapGetCellCustomValue(pApp, pDef->sId, x, y, sValue, sizeof(sValue));
		MapEditMapCopyText(sDisplay, sizeof(sDisplay), MapEditMapCustomDisplayValue(pDef, sValue));
		snprintf(sPropId, sizeof(sPropId), "cell_custom.%s", pDef->sId);
		MapEditMapAddPropertyEx(pGrid, iCategory, sPropId, pDef->sName[0] ? pDef->sName : pDef->sId,
			pDef->sId, XGE_XUI_TABLE_CELL_TYPE_TEXT, sDisplay, "", XGE_XUI_PROPERTY_FLAG_READONLY);
	}
	xgeXuiPropertyGridSetSelected(pGrid, xgeXuiPropertyGridFindProperty(pGrid, "cell.position"));
}

void MapEditMapPropertiesRefresh(mapedit_app_t* pApp)
{
	mapedit_map_item_t* pItem;
	xge_xui_property_grid pGrid;
	xge_xui_table_grid_editor_config_t tEnum;
	const char* sTilesetDisplay;
	char sState[32];
	char sWidth[32];
	char sHeight[32];
	int iCategory;
	int iTilesetProp;

	if ( pApp == NULL || pApp->pMapPropertiesGridWidget == NULL ) {
		return;
	}
	pItem = MapEditMapSelectedItem(pApp);
	if ( pItem == NULL ) {
		MapEditMapPropertiesBuildEmpty(pApp);
		return;
	}
	pGrid = &pApp->tMapPropertiesGrid;
	xgeXuiPropertyGridClear(pGrid);
	iCategory = xgeXuiPropertyGridAddCategory(pGrid, "map", "地图", 1);
	if ( iCategory < 0 ) {
		return;
	}
	snprintf(sState, sizeof(sState), "%d", pItem->iState);
	snprintf(sWidth, sizeof(sWidth), "%d", pItem->iWidth);
	snprintf(sHeight, sizeof(sHeight), "%d", pItem->iHeight);
	MapEditMapTilesetLoadOptions(pApp, pItem->sTileset);
	sTilesetDisplay = MapEditMapTilesetDisplayForValue(pApp, pItem->sTileset);
	MapEditMapAddProperty(pGrid, iCategory, "map.name", "地图名称", "当前地图的显示名称", XGE_XUI_TABLE_CELL_TYPE_TEXT, pItem->sName, "默认地图");
	iTilesetProp = MapEditMapAddProperty(pGrid, iCategory, "map.tileset", "图集", "地图使用的图集", XGE_XUI_TABLE_CELL_TYPE_ENUM, sTilesetDisplay, "未设置");
	if ( iTilesetProp >= 0 ) {
		memset(&tEnum, 0, sizeof(tEnum));
		tEnum.arrEnumItems = pApp->arrMapTilesetOptions;
		tEnum.iEnumItemCount = pApp->iMapTilesetOptionCount;
		tEnum.iEnumSelected = MapEditMapTilesetFindByValue(pApp, pItem->sTileset);
		xgeXuiPropertyGridSetEditorConfig(pGrid, iTilesetProp, &tEnum);
	}
	MapEditMapAddProperty(pGrid, iCategory, "map.state", "当前状态", "地图当前状态，用于多状态图块显示", XGE_XUI_TABLE_CELL_TYPE_INT, sState, "0");
	MapEditMapAddProperty(pGrid, iCategory, "map.width", "地图宽度", "横向图块数量", XGE_XUI_TABLE_CELL_TYPE_INT, sWidth, "100");
	MapEditMapAddProperty(pGrid, iCategory, "map.height", "地图高度", "纵向图块数量", XGE_XUI_TABLE_CELL_TYPE_INT, sHeight, "100");
	MapEditMapAddProperty(pGrid, iCategory, "map.custom_data", "自定义数据", "地图级自定义数据", XGE_XUI_TABLE_CELL_TYPE_TEXTAREA, pItem->sCustomData, "");
	xgeXuiPropertyGridSetSelected(pGrid, xgeXuiPropertyGridFindProperty(pGrid, "map.name"));
}

static void MapEditMapPropertyChange(xge_xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_map_item_t* pItem;
	int iOldWidth;
	int iOldHeight;
	int iOldLayers;
	int iValue;
	int bResize;
	int bRefreshAll;

	(void)pWidget;
	(void)iProperty;
	(void)iType;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || sId == NULL ) {
		return;
	}
	pItem = MapEditMapSelectedItem(pApp);
	if ( pItem == NULL ) {
		return;
	}
	iOldWidth = pItem->iWidth;
	iOldHeight = pItem->iHeight;
	iOldLayers = pItem->iLayerCount;
	bResize = 0;
	bRefreshAll = 0;
	if ( strcmp(sId, "map.name") == 0 ) {
		MapEditMapCopyText(pItem->sName, sizeof(pItem->sName), sValue);
		bRefreshAll = 1;
	} else if ( strcmp(sId, "map.tileset") == 0 ) {
		MapEditMapCopyText(pItem->sTileset, sizeof(pItem->sTileset), MapEditMapTilesetValueForDisplay(pApp, sValue));
		pApp->iMapBrushTileId = -1;
		bRefreshAll = 1;
	} else if ( strcmp(sId, "map.state") == 0 ) {
		iValue = (sValue != NULL) ? atoi(sValue) : MapEditMapSetupStateMin(pApp);
		pItem->iState = MapEditMapClampState(pApp, iValue);
		bRefreshAll = 1;
	} else if ( strcmp(sId, "map.width") == 0 ) {
		iValue = (sValue != NULL) ? atoi(sValue) : MAPEDIT_MAP_WIDTH_DEFAULT;
		pItem->iWidth = iValue;
		bResize = 1;
		bRefreshAll = 1;
	} else if ( strcmp(sId, "map.height") == 0 ) {
		iValue = (sValue != NULL) ? atoi(sValue) : MAPEDIT_MAP_HEIGHT_DEFAULT;
		pItem->iHeight = iValue;
		bResize = 1;
		bRefreshAll = 1;
	} else if ( strcmp(sId, "map.custom_data") == 0 ) {
		MapEditMapCopyText(pItem->sCustomData, sizeof(pItem->sCustomData), sValue);
	} else {
		return;
	}
	MapEditMapNormalize(pApp, pItem);
	if ( bResize ) {
		if ( MapEditMapResizeTileData(pApp, iOldWidth, iOldHeight, iOldLayers) != XGE_OK ) {
			MapEditAppSetStatus(pApp, "地图尺寸已修改，但图块数据重建失败");
			return;
		}
		if ( pApp->iMapActiveLayer >= pItem->iLayerCount ) {
			pApp->iMapActiveLayer = pItem->iLayerCount - 1;
		}
		if ( pApp->iMapActiveLayer < 0 ) {
			pApp->iMapActiveLayer = 0;
		}
		MapEditMapEditClearHistory(pApp);
		MapEditMapCompactRuntimeData(pItem);
	}
	if ( MapEditMapSaveSelected(pApp) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "地图属性已修改，但保存失败");
	} else {
		MapEditAppSetStatus(pApp, "地图属性已修改并保存");
	}
	if ( bRefreshAll ) {
		MapEditMapRefreshDependents(pApp);
	}
}

int MapEditMapListCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
{
	xge_xui_widget pContent;
	xge_xui_widget pListWidget;
	xge_xui_widget pButtonWidget;

	if ( pApp == NULL || pWindow == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pContent = MapEditWorkspaceNewContent(pApp);
	pListWidget = xgeXuiWidgetCreate();
	pButtonWidget = xgeXuiWidgetCreate();
	if ( pContent == NULL || pListWidget == NULL || pButtonWidget == NULL ) {
		xgeXuiWidgetFree(pListWidget);
		xgeXuiWidgetFree(pButtonWidget);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	MapEditMapEnsureDefaults(pApp);
	xgeXuiWidgetSetSize(pListWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pButtonWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(30.0f));
	xgeXuiWidgetSetBackground(pListWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pListWidget, 1.0f, XGE_COLOR_RGBA(164, 206, 236, 255));
	if ( xgeXuiListViewInit(&pApp->tMapList, &pApp->tXui, pListWidget) != XGE_OK ||
	     xgeXuiButtonInit(&pApp->tMapAddButton, &pApp->tXui, pButtonWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pListWidget);
		xgeXuiWidgetFree(pButtonWidget);
		return XGE_ERROR;
	}
	xgeXuiListViewSetFont(&pApp->tMapList, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiListViewSetItemHeight(&pApp->tMapList, 24.0f);
	xgeXuiListViewSetScrollbarMode(&pApp->tMapList, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiListViewSetColors(&pApp->tMapList, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(204, 232, 250, 255), XGE_COLOR_RGBA(31, 75, 112, 255), XGE_COLOR_RGBA(230, 240, 247, 255), XGE_COLOR_RGBA(120, 176, 212, 255));
	xgeXuiListViewSetSelect(&pApp->tMapList, MapEditMapSelect, pApp);
	xgeXuiButtonSetText(&pApp->tMapAddButton, pApp->bFontReady ? &pApp->tFont : NULL, "添加地图");
	xgeXuiButtonSetSemantic(&pApp->tMapAddButton, XGE_XUI_BUTTON_SEMANTIC_PRIMARY);
	xgeXuiButtonSetClick(&pApp->tMapAddButton, MapEditMapAddClick, pApp);
	pApp->pMapListWidget = pListWidget;
	pApp->pMapAddButtonWidget = pButtonWidget;
	MapEditMapRefreshList(pApp);
	if ( xgeXuiWidgetAdd(pContent, pListWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pContent, pButtonWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	return XGE_OK;
}

int MapEditMapPropertiesCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
{
	xge_xui_widget pContent;
	xge_xui_widget pGridWidget;

	if ( pApp == NULL || pWindow == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pContent = MapEditWorkspaceNewContent(pApp);
	pGridWidget = xgeXuiWidgetCreate();
	if ( pContent == NULL || pGridWidget == NULL ) {
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetSize(pGridWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetBackground(pGridWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pGridWidget, 1.0f, XGE_COLOR_RGBA(164, 206, 236, 255));
	if ( xgeXuiPropertyGridInit(&pApp->tMapPropertiesGrid, &pApp->tXui, pGridWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	xgeXuiPropertyGridSetFont(&pApp->tMapPropertiesGrid, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiPropertyGridSetMetrics(&pApp->tMapPropertiesGrid, 112.0f, 24.0f, 25.0f);
	xgeXuiPropertyGridSetDescriptionMode(&pApp->tMapPropertiesGrid, XGE_XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, 0.0f);
	xgeXuiPropertyGridSetEditMode(&pApp->tMapPropertiesGrid, XGE_XUI_TABLE_GRID_EDIT_QUICK);
	xgeXuiPropertyGridSetScrollbarMode(&pApp->tMapPropertiesGrid, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiPropertyGridSetColors(&pApp->tMapPropertiesGrid, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(226, 242, 252, 255), XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(204, 232, 250, 255), XGE_COLOR_RGBA(164, 206, 236, 255), XGE_COLOR_RGBA(31, 75, 112, 255));
	xgeXuiPropertyGridSetChange(&pApp->tMapPropertiesGrid, MapEditMapPropertyChange, pApp);
	pApp->pMapPropertiesGridWidget = pGridWidget;
	if ( xgeXuiWidgetAdd(pContent, pGridWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	MapEditMapEnsureDefaults(pApp);
	MapEditMapPropertiesRefresh(pApp);
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	return XGE_OK;
}

void MapEditMapListUnit(mapedit_app_t* pApp)
{
	int i;

	if ( pApp == NULL ) {
		return;
	}
	MapEditMapClearTilesetTexture(pApp);
	if ( pApp->pMapPropertiesGridWidget != NULL ) {
		xgeXuiPropertyGridUnit(&pApp->tMapPropertiesGrid);
		pApp->pMapPropertiesGridWidget = NULL;
	}
	if ( pApp->pMapAddButtonWidget != NULL ) {
		xgeXuiButtonUnit(&pApp->tMapAddButton);
		pApp->pMapAddButtonWidget = NULL;
	}
	if ( pApp->pMapListWidget != NULL ) {
		xgeXuiListViewUnit(&pApp->tMapList);
		pApp->pMapListWidget = NULL;
	}
	free(pApp->pMapTileData);
	pApp->pMapTileData = NULL;
	pApp->iMapTileDataCount = 0;
	for ( i = 0; i < pApp->iMapCount && i < MAPEDIT_MAP_ITEM_MAX; i++ ) {
		MapEditMapFreeItemRuntimeData(&pApp->arrMapItems[i]);
	}
	pApp->iMapCount = 0;
	pApp->iMapSelected = -1;
}
