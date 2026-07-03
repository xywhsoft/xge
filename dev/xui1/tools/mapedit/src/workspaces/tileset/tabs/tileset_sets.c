#include "tileset_sets.h"
#include "tileset_arrange.h"
#include "tileset_passage.h"
#include "tileset_actor_overlay.h"
#include "tileset_tags.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAPEDIT_TILESET_SET_ASSET_DIR "图块集"
#define MAPEDIT_TILESET_SET_KEY_NAME "图集名称"
#define MAPEDIT_TILESET_SET_KEY_SPECIAL_COUNT "特殊图块数量"
#define MAPEDIT_TILESET_SET_KEY_STATIC_TILES "静态图块集"
#define MAPEDIT_TILESET_SET_KEY_CUSTOM_DATA "自定义数据"
#define MAPEDIT_TILESET_SET_KEY_SPECIAL_TILES "特殊图块"
#define MAPEDIT_TILESET_SET_KEY_SPECIAL_TYPE "特殊图块类型"
#define MAPEDIT_TILESET_SET_KEY_SPECIAL_FILE "特殊图块文件名"
#define MAPEDIT_TILESET_SET_KEY_PASSAGE "通行数据"
#define MAPEDIT_TILESET_SET_KEY_ACTOR_OVERLAY "角色覆盖数据"
#define MAPEDIT_TILESET_SET_DEFAULT_FILE "default.xson"
#define MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_DATA "tileCustomData"
#define MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_TILE "tile"
#define MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_VALUE "value"

typedef struct mapedit_tileset_set_scan_t {
	mapedit_app_t* pApp;
	int iLoadedCount;
} mapedit_tileset_set_scan_t;

typedef struct mapedit_tileset_special_category_t {
	const char* sType;
	const char* sDir;
	const char* sMapFile;
} mapedit_tileset_special_category_t;

typedef struct mapedit_tileset_special_file_scan_t {
	mapedit_app_t* pApp;
	xvalue pMap;
	const char* sCurrentFile;
	int bCurrentFound;
	int bChanged;
} mapedit_tileset_special_file_scan_t;

static const mapedit_tileset_special_category_t g_arrTilesetSpecialCategories[] = {
	{ "动态图块", "animated_tiles", "animated_tiles.xson" },
	{ "自动图块", "autotiles", "autotiles.xson" },
	{ "多状态图块", "state_tiles", "state_tiles.xson" },
	{ "多状态自动图块", "state_autotiles", "state_autotiles.xson" },
	{ "对象", "objects", "objects.xson" }
};

static const char* g_arrTilesetSpecialTypeOptions[] = {
	"动态图块",
	"自动图块",
	"多状态图块",
	"多状态自动图块",
	"对象"
};

static void MapEditTilesetSetCopyText(char* sDst, int iDstSize, const char* sSrc)
{
	size_t iLen;

	if ( (sDst == NULL) || (iDstSize <= 0) ) {
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

const mapedit_custom_channel_def_t* MapEditCustomChannelGet(mapedit_app_t* pApp, int iChannel)
{
	if ( pApp == NULL || iChannel < 0 || iChannel >= pApp->iCustomChannelCount ) {
		return NULL;
	}
	return &pApp->arrCustomChannels[iChannel];
}

const mapedit_custom_channel_def_t* MapEditCustomChannelFind(mapedit_app_t* pApp, const char* sChannelId)
{
	int i;

	if ( pApp == NULL || sChannelId == NULL ) {
		return NULL;
	}
	for ( i = 0; i < pApp->iCustomChannelCount; i++ ) {
		if ( strcmp(pApp->arrCustomChannels[i].sId, sChannelId) == 0 ) {
			return &pApp->arrCustomChannels[i];
		}
	}
	return NULL;
}

void MapEditTilesetSetClearCustomData(mapedit_tileset_set_item_t* pItem)
{
	int i;

	if ( pItem == NULL ) {
		return;
	}
	for ( i = 0; i < pItem->iCustomTileDataCount && i < MAPEDIT_CUSTOM_CHANNEL_MAX; i++ ) {
		free(pItem->arrCustomTileData[i].arrValues);
		pItem->arrCustomTileData[i].arrValues = NULL;
		pItem->arrCustomTileData[i].iValueCount = 0;
		pItem->arrCustomTileData[i].iValueCapacity = 0;
		pItem->arrCustomTileData[i].sChannelId[0] = 0;
	}
	pItem->iCustomTileDataCount = 0;
}

static mapedit_tileset_custom_channel_data_t* MapEditTilesetSetFindCustomChannelData(mapedit_tileset_set_item_t* pItem, const char* sChannelId)
{
	int i;

	if ( pItem == NULL || sChannelId == NULL || sChannelId[0] == 0 ) {
		return NULL;
	}
	for ( i = 0; i < pItem->iCustomTileDataCount && i < MAPEDIT_CUSTOM_CHANNEL_MAX; i++ ) {
		if ( strcmp(pItem->arrCustomTileData[i].sChannelId, sChannelId) == 0 ) {
			return &pItem->arrCustomTileData[i];
		}
	}
	return NULL;
}

static mapedit_tileset_custom_channel_data_t* MapEditTilesetSetEnsureCustomChannelData(mapedit_tileset_set_item_t* pItem, const char* sChannelId)
{
	mapedit_tileset_custom_channel_data_t* pData;

	pData = MapEditTilesetSetFindCustomChannelData(pItem, sChannelId);
	if ( pData != NULL ) {
		return pData;
	}
	if ( pItem == NULL || sChannelId == NULL || sChannelId[0] == 0 || pItem->iCustomTileDataCount >= MAPEDIT_CUSTOM_CHANNEL_MAX ) {
		return NULL;
	}
	pData = &pItem->arrCustomTileData[pItem->iCustomTileDataCount++];
	memset(pData, 0, sizeof(*pData));
	MapEditTilesetSetCopyText(pData->sChannelId, sizeof(pData->sChannelId), sChannelId);
	return pData;
}

static int MapEditTilesetSetFindCustomValue(mapedit_tileset_custom_channel_data_t* pData, int iTileId)
{
	int i;

	if ( pData == NULL ) {
		return -1;
	}
	for ( i = 0; i < pData->iValueCount; i++ ) {
		if ( pData->arrValues[i].iTileId == iTileId ) {
			return i;
		}
	}
	return -1;
}

int MapEditTilesetSetGetTileCustomValue(mapedit_tileset_set_item_t* pItem, const char* sChannelId, int iTileId, char* sValue, int iValueSize)
{
	mapedit_tileset_custom_channel_data_t* pData;
	int iIndex;

	if ( sValue != NULL && iValueSize > 0 ) {
		sValue[0] = 0;
	}
	if ( pItem == NULL || sChannelId == NULL || iTileId < 0 || iTileId >= MAPEDIT_TILESET_CUSTOM_TILE_MAX ) {
		return 0;
	}
	pData = MapEditTilesetSetFindCustomChannelData(pItem, sChannelId);
	iIndex = MapEditTilesetSetFindCustomValue(pData, iTileId);
	if ( iIndex < 0 ) {
		return 0;
	}
	MapEditTilesetSetCopyText(sValue, iValueSize, pData->arrValues[iIndex].sValue);
	return 1;
}

int MapEditTilesetSetSetTileCustomValue(mapedit_tileset_set_item_t* pItem, const char* sChannelId, int iTileId, const char* sValue)
{
	mapedit_tileset_custom_channel_data_t* pData;
	mapedit_tileset_custom_value_t* pNewValues;
	int iIndex;
	int iNewCapacity;

	if ( pItem == NULL || sChannelId == NULL || sChannelId[0] == 0 || iTileId < 0 || iTileId >= MAPEDIT_TILESET_CUSTOM_TILE_MAX ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pData = MapEditTilesetSetEnsureCustomChannelData(pItem, sChannelId);
	if ( pData == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iIndex = MapEditTilesetSetFindCustomValue(pData, iTileId);
	if ( iIndex < 0 ) {
		if ( pData->iValueCount >= pData->iValueCapacity ) {
			iNewCapacity = (pData->iValueCapacity > 0) ? (pData->iValueCapacity * 2) : 64;
			pNewValues = (mapedit_tileset_custom_value_t*)realloc(pData->arrValues, (size_t)iNewCapacity * sizeof(mapedit_tileset_custom_value_t));
			if ( pNewValues == NULL ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			pData->arrValues = pNewValues;
			pData->iValueCapacity = iNewCapacity;
		}
		iIndex = pData->iValueCount++;
		memset(&pData->arrValues[iIndex], 0, sizeof(pData->arrValues[iIndex]));
		pData->arrValues[iIndex].iTileId = iTileId;
	}
	MapEditTilesetSetCopyText(pData->arrValues[iIndex].sValue, sizeof(pData->arrValues[iIndex].sValue), sValue);
	return XGE_OK;
}

static int MapEditTilesetSpecialCategoryCount(void)
{
	return (int)(sizeof(g_arrTilesetSpecialCategories) / sizeof(g_arrTilesetSpecialCategories[0]));
}

static int MapEditTilesetSpecialTypeFind(const char* sType)
{
	int i;

	if ( sType == NULL || sType[0] == 0 ) {
		return 0;
	}
	for ( i = 0; i < MapEditTilesetSpecialCategoryCount(); i++ ) {
		if ( strcmp(g_arrTilesetSpecialCategories[i].sType, sType) == 0 ) {
			return i;
		}
	}
	return 0;
}

static const char* MapEditTilesetSpecialTypeOrDefault(const char* sType)
{
	return g_arrTilesetSpecialCategories[MapEditTilesetSpecialTypeFind(sType)].sType;
}

static str MapEditTilesetAssetPath(const char* sSubPath)
{
	return xrtPathJoin(3, xCore.AppPath, (str)"assets", (str)sSubPath);
}

static xvalue MapEditTilesetSpecialLoadMap(const char* sPath)
{
	xvalue pMap;

	pMap = NULL;
	if ( (sPath != NULL) && xrtFileExists((str)sPath) ) {
		pMap = xrtParseXSON_File((str)sPath);
	}
	if ( (pMap == NULL) || (xvoType(pMap) != XVO_DT_TABLE) ) {
		if ( pMap != NULL ) {
			xvoUnref(pMap);
		}
		pMap = xvoCreateTable();
	}
	return pMap;
}

static void MapEditTilesetSpecialFileAddOption(mapedit_app_t* pApp, const char* sText, const char* sValue)
{
	int iIndex;

	if ( pApp == NULL || sText == NULL || pApp->iTilesetSpecialFileOptionCount >= MAPEDIT_TILESET_SPECIAL_FILE_OPTION_MAX ) {
		return;
	}
	iIndex = pApp->iTilesetSpecialFileOptionCount++;
	MapEditTilesetSetCopyText(pApp->arrTilesetSpecialFileOptionText[iIndex], sizeof(pApp->arrTilesetSpecialFileOptionText[iIndex]), sText);
	MapEditTilesetSetCopyText(pApp->arrTilesetSpecialFileOptionValue[iIndex], sizeof(pApp->arrTilesetSpecialFileOptionValue[iIndex]), sValue);
	pApp->arrTilesetSpecialFileOptions[iIndex] = pApp->arrTilesetSpecialFileOptionText[iIndex];
}

static int MapEditTilesetSpecialFileFindByValue(mapedit_app_t* pApp, const char* sValue)
{
	int i;

	if ( pApp == NULL ) {
		return -1;
	}
	if ( sValue == NULL ) {
		sValue = "";
	}
	for ( i = 0; i < pApp->iTilesetSpecialFileOptionCount; i++ ) {
		if ( strcmp(pApp->arrTilesetSpecialFileOptionValue[i], sValue) == 0 ) {
			return i;
		}
	}
	return -1;
}

static int MapEditTilesetSpecialFileFindByText(mapedit_app_t* pApp, const char* sText)
{
	int i;

	if ( pApp == NULL || sText == NULL ) {
		return -1;
	}
	for ( i = 0; i < pApp->iTilesetSpecialFileOptionCount; i++ ) {
		if ( strcmp(pApp->arrTilesetSpecialFileOptionText[i], sText) == 0 ) {
			return i;
		}
	}
	return -1;
}

static const char* MapEditTilesetSpecialFileDisplayForValue(mapedit_app_t* pApp, const char* sValue)
{
	int iIndex;

	iIndex = MapEditTilesetSpecialFileFindByValue(pApp, sValue);
	if ( iIndex >= 0 ) {
		return pApp->arrTilesetSpecialFileOptionText[iIndex];
	}
	return "未设置";
}

static const char* MapEditTilesetSpecialFileValueForDisplay(mapedit_app_t* pApp, const char* sText)
{
	int iIndex;

	iIndex = MapEditTilesetSpecialFileFindByText(pApp, sText);
	if ( iIndex >= 0 ) {
		return pApp->arrTilesetSpecialFileOptionValue[iIndex];
	}
	iIndex = MapEditTilesetSpecialFileFindByValue(pApp, sText);
	if ( iIndex >= 0 ) {
		return pApp->arrTilesetSpecialFileOptionValue[iIndex];
	}
	return "";
}

static str MapEditTilesetSetDirPath(void)
{
	return xrtPathJoin(3, xCore.AppPath, (str)"assets", (str)MAPEDIT_TILESET_SET_ASSET_DIR);
}

static str MapEditTilesetSetFilePath(const char* sFile)
{
	if ( (sFile == NULL) || (sFile[0] == 0) ) {
		return NULL;
	}
	return xrtPathJoin(4, xCore.AppPath, (str)"assets", (str)MAPEDIT_TILESET_SET_ASSET_DIR, (str)sFile);
}

static int MapEditTilesetSetEnsureDir(void)
{
	str sDir;
	int iRet;

	sDir = MapEditTilesetSetDirPath();
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

static int MapEditTilesetSetIsXsonFile(const char* sPath)
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
	for ( i = 0; (i < (int)sizeof(sLower) - 1) && sExt[i] != 0; i++ ) {
		char ch = sExt[i];
		if ( ch >= 'A' && ch <= 'Z' ) {
			ch = (char)(ch - 'A' + 'a');
		}
		sLower[i] = ch;
	}
	sLower[i] = 0;
	return strcmp(sLower, ".xson") == 0;
}

static void MapEditTilesetSetNormalizeSpecialCount(mapedit_tileset_set_item_t* pItem)
{
	if ( pItem == NULL ) {
		return;
	}
	if ( pItem->iSpecialTileCount < 0 ) {
		pItem->iSpecialTileCount = 0;
	} else if ( pItem->iSpecialTileCount > MAPEDIT_TILESET_SPECIAL_TILE_MAX ) {
		pItem->iSpecialTileCount = MAPEDIT_TILESET_SPECIAL_TILE_MAX;
	}
}

void MapEditTilesetSetEnsurePassageCount(mapedit_tileset_set_item_t* pItem, int iCount)
{
	int i;

	if ( pItem == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	} else if ( iCount > MAPEDIT_TILESET_PASSAGE_MAX ) {
		iCount = MAPEDIT_TILESET_PASSAGE_MAX;
	}
	if ( pItem->iPassageCount < 0 || pItem->iPassageCount > MAPEDIT_TILESET_PASSAGE_MAX ) {
		pItem->iPassageCount = 0;
	}
	for ( i = pItem->iPassageCount; i < iCount; i++ ) {
		pItem->arrPassage[i] = (unsigned char)MAPEDIT_TILESET_PASSAGE_DEFAULT;
	}
	if ( pItem->iPassageCount < iCount ) {
		pItem->iPassageCount = iCount;
	}
}

void MapEditTilesetSetEnsureActorOverlayCount(mapedit_tileset_set_item_t* pItem, int iCount)
{
	int i;

	if ( pItem == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	} else if ( iCount > MAPEDIT_TILESET_ACTOR_OVERLAY_MAX ) {
		iCount = MAPEDIT_TILESET_ACTOR_OVERLAY_MAX;
	}
	if ( pItem->iActorOverlayCount < 0 || pItem->iActorOverlayCount > MAPEDIT_TILESET_ACTOR_OVERLAY_MAX ) {
		pItem->iActorOverlayCount = 0;
	}
	for ( i = pItem->iActorOverlayCount; i < iCount; i++ ) {
		pItem->arrActorOverlay[i] = (unsigned char)MAPEDIT_TILESET_ACTOR_OVERLAY_DEFAULT;
	}
	if ( pItem->iActorOverlayCount < iCount ) {
		pItem->iActorOverlayCount = iCount;
	}
}

mapedit_tileset_set_item_t* MapEditTilesetSetSelectedItem(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iTilesetSetSelected < 0 || pApp->iTilesetSetSelected >= pApp->iTilesetSetCount ) {
		return NULL;
	}
	return &pApp->arrTilesetSetItems[pApp->iTilesetSetSelected];
}

static void MapEditTilesetSetMakeUniqueFile(char* sDst, int iDstSize)
{
	char sCandidate[MAPEDIT_TILESET_SET_FILE_CAPACITY];
	str sPath;
	int i;

	if ( (sDst == NULL) || (iDstSize <= 0) ) {
		return;
	}
	sDst[0] = 0;
	for ( i = 1; i < 10000; i++ ) {
		snprintf(sCandidate, sizeof(sCandidate), "tileset_%03d.xson", i);
		sPath = MapEditTilesetSetFilePath(sCandidate);
		if ( sPath == NULL ) {
			continue;
		}
		if ( !xrtFileExists(sPath) ) {
			MapEditTilesetSetCopyText(sDst, iDstSize, sCandidate);
			xrtFree(sPath);
			return;
		}
		xrtFree(sPath);
	}
	MapEditTilesetSetCopyText(sDst, iDstSize, "tileset.xson");
}

static int MapEditTilesetSetIsImageFile(const char* sPath)
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
	for ( i = 0; (i < (int)sizeof(sLower) - 1) && sExt[i] != 0; i++ ) {
		char ch = sExt[i];
		if ( ch >= 'A' && ch <= 'Z' ) {
			ch = (char)(ch - 'A' + 'a');
		}
		sLower[i] = ch;
	}
	sLower[i] = 0;
	return strcmp(sLower, ".png") == 0 ||
		strcmp(sLower, ".jpg") == 0 ||
		strcmp(sLower, ".jpeg") == 0 ||
		strcmp(sLower, ".bmp") == 0 ||
		strcmp(sLower, ".tga") == 0 ||
		strcmp(sLower, ".webp") == 0;
}

static int MapEditTilesetSpecialFileScanProc(str sPath, size_t iSize, int bDir, ptr pData, ptr pParam)
{
	mapedit_tileset_special_file_scan_t* pScan;
	str sNameExt;
	xvalue pNameValue;
	const char* sName;
	char sDisplay[MAPEDIT_TILESET_SET_TEXT_CAPACITY * 2];

	(void)pData;
	pScan = (mapedit_tileset_special_file_scan_t*)pParam;
	if ( pScan == NULL || pScan->pApp == NULL || bDir || !MapEditTilesetSetIsImageFile((const char*)sPath) ) {
		return 0;
	}
	sNameExt = xrtPathGetNameExt(sPath, iSize);
	if ( sNameExt == NULL || sNameExt[0] == 0 ) {
		xrtFree(sNameExt);
		return 0;
	}
	pNameValue = xvoTableGetValue(pScan->pMap, sNameExt, (uint32)strlen((const char*)sNameExt));
	if ( xvoType(pNameValue) == XVO_DT_TEXT ) {
		sName = (const char*)xvoGetText(pNameValue);
	} else {
		sName = "未命名图块";
		xvoTableSetText(pScan->pMap, sNameExt, (uint32)strlen((const char*)sNameExt), (str)sName, 0, TRUE);
		pScan->bChanged = 1;
	}
	snprintf(sDisplay, sizeof(sDisplay), "%s (%s)", sName, (const char*)sNameExt);
	MapEditTilesetSpecialFileAddOption(pScan->pApp, sDisplay, (const char*)sNameExt);
	if ( pScan->sCurrentFile != NULL && strcmp(pScan->sCurrentFile, (const char*)sNameExt) == 0 ) {
		pScan->bCurrentFound = 1;
	}
	xrtFree(sNameExt);
	return 0;
}

static void MapEditTilesetSpecialFileLoadOptions(mapedit_app_t* pApp, const char* sType, const char* sCurrentFile)
{
	const mapedit_tileset_special_category_t* pCategory;
	mapedit_tileset_special_file_scan_t tScan;
	str sDir;
	str sMapPath;
	xvalue pMap;
	char sMissing[MAPEDIT_TILESET_SET_TEXT_CAPACITY * 2];
	int iType;

	if ( pApp == NULL ) {
		return;
	}
	pApp->iTilesetSpecialFileOptionCount = 0;
	MapEditTilesetSpecialFileAddOption(pApp, "未设置", "");
	iType = MapEditTilesetSpecialTypeFind(sType);
	pCategory = &g_arrTilesetSpecialCategories[iType];
	sDir = MapEditTilesetAssetPath(pCategory->sDir);
	sMapPath = MapEditTilesetAssetPath(pCategory->sMapFile);
	if ( sDir == NULL || sMapPath == NULL ) {
		xrtFree(sDir);
		xrtFree(sMapPath);
		return;
	}
	if ( !xrtDirExists(sDir) ) {
		xrtDirCreateAll(sDir);
	}
	pMap = MapEditTilesetSpecialLoadMap((const char*)sMapPath);
	if ( pMap != NULL ) {
		memset(&tScan, 0, sizeof(tScan));
		tScan.pApp = pApp;
		tScan.pMap = pMap;
		tScan.sCurrentFile = sCurrentFile;
		if ( xrtDirExists(sDir) ) {
			xrtDirScan(sDir, 0, MapEditTilesetSpecialFileScanProc, &tScan);
		}
		if ( tScan.bChanged != 0 ) {
			xrtStringifyXSON_File(sMapPath, pMap, 1, 0);
		}
		xvoUnref(pMap);
		if ( sCurrentFile != NULL && sCurrentFile[0] != 0 && tScan.bCurrentFound == 0 &&
		     pApp->iTilesetSpecialFileOptionCount < MAPEDIT_TILESET_SPECIAL_FILE_OPTION_MAX ) {
			snprintf(sMissing, sizeof(sMissing), "已丢失 (%s)", sCurrentFile);
			MapEditTilesetSpecialFileAddOption(pApp, sMissing, sCurrentFile);
		}
	}
	xrtFree(sDir);
	xrtFree(sMapPath);
}

static void MapEditTilesetStaticTileAddOption(mapedit_app_t* pApp, const char* sText)
{
	int iIndex;

	if ( (pApp == NULL) || (sText == NULL) || (sText[0] == 0) ||
		(pApp->iTilesetStaticTileOptionCount >= MAPEDIT_TILESET_STATIC_TILE_OPTION_MAX) ) {
		return;
	}
	iIndex = pApp->iTilesetStaticTileOptionCount++;
	MapEditTilesetSetCopyText(pApp->arrTilesetStaticTileOptionText[iIndex], MAPEDIT_TILESET_SET_TEXT_CAPACITY, sText);
	pApp->arrTilesetStaticTileOptions[iIndex] = pApp->arrTilesetStaticTileOptionText[iIndex];
}

static int MapEditTilesetStaticTileFindOption(mapedit_app_t* pApp, const char* sText)
{
	int i;

	if ( (pApp == NULL) || (sText == NULL) ) {
		return -1;
	}
	for ( i = 0; i < pApp->iTilesetStaticTileOptionCount; i++ ) {
		if ( strcmp(pApp->arrTilesetStaticTileOptions[i], sText) == 0 ) {
			return i;
		}
	}
	return -1;
}

static void MapEditTilesetStaticTileEnsureOption(mapedit_app_t* pApp, const char* sText)
{
	if ( (sText == NULL) || (sText[0] == 0) ) {
		return;
	}
	if ( MapEditTilesetStaticTileFindOption(pApp, sText) < 0 ) {
		MapEditTilesetStaticTileAddOption(pApp, sText);
	}
}

static int MapEditTilesetStaticTileScanProc(str sPath, size_t iSize, int bDir, ptr pData, ptr pParam)
{
	mapedit_app_t* pApp;
	str sFile;

	(void)iSize;
	(void)pData;
	pApp = (mapedit_app_t*)pParam;
	if ( (pApp == NULL) || bDir || !MapEditTilesetSetIsImageFile((const char*)sPath) ) {
		return 0;
	}
	sFile = xrtPathGetNameExt(sPath, iSize);
	if ( sFile == NULL ) {
		return 0;
	}
	MapEditTilesetStaticTileAddOption(pApp, (const char*)sFile);
	xrtFree(sFile);
	return 0;
}

static void MapEditTilesetStaticTileLoadOptions(mapedit_app_t* pApp)
{
	str sDir;

	if ( pApp == NULL ) {
		return;
	}
	pApp->iTilesetStaticTileOptionCount = 0;
	MapEditTilesetStaticTileAddOption(pApp, "未设置");
	sDir = xrtPathJoin(3, xCore.AppPath, (str)"assets", (str)"tilesheets");
	if ( sDir == NULL ) {
		return;
	}
	if ( !xrtDirExists(sDir) ) {
		xrtDirCreateAll(sDir);
	} else {
		xrtDirScan(sDir, 0, MapEditTilesetStaticTileScanProc, pApp);
	}
	xrtFree(sDir);
}

static int MapEditTilesetSetAddPropertyEx(xge_xui_property_grid pGrid, int iCategory, const char* sId, const char* sName, const char* sDescription, int iType, const char* sValue, const char* sDefault, int iFlags)
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

static int MapEditTilesetSetAddProperty(xge_xui_property_grid pGrid, int iCategory, const char* sId, const char* sName, const char* sDescription, int iType, const char* sValue, const char* sDefault)
{
	return MapEditTilesetSetAddPropertyEx(pGrid, iCategory, sId, sName, sDescription, iType, sValue, sDefault, 0);
}

static void MapEditTilesetSetBuildText(mapedit_tileset_set_item_t* pItem)
{
	char sText[MAPEDIT_TILESET_SET_TEXT_CAPACITY * 3];

	if ( pItem == NULL ) {
		return;
	}
	if ( pItem->sStaticTiles[0] != 0 ) {
		snprintf(sText, sizeof(sText), "%s    %s", pItem->sName, pItem->sStaticTiles);
	} else {
		snprintf(sText, sizeof(sText), "%s", pItem->sName);
	}
	MapEditTilesetSetCopyText(pItem->sText, sizeof(pItem->sText), sText);
}

static xvalue MapEditTilesetSetBuildCustomDataValue(mapedit_tileset_set_item_t* pItem)
{
	mapedit_tileset_custom_channel_data_t* pData;
	mapedit_tileset_custom_value_t* pValue;
	xvalue pRoot;
	xvalue pArray;
	xvalue pEntry;
	int i;
	int j;

	pRoot = xvoCreateTable();
	if ( pRoot == NULL ) {
		return NULL;
	}
	if ( pItem == NULL ) {
		return pRoot;
	}
	for ( i = 0; i < pItem->iCustomTileDataCount && i < MAPEDIT_CUSTOM_CHANNEL_MAX; i++ ) {
		pData = &pItem->arrCustomTileData[i];
		if ( pData->sChannelId[0] == 0 || pData->iValueCount <= 0 || pData->arrValues == NULL ) {
			continue;
		}
		pArray = xvoCreateArray();
		if ( pArray == NULL ) {
			xvoUnref(pRoot);
			return NULL;
		}
		for ( j = 0; j < pData->iValueCount; j++ ) {
			pValue = &pData->arrValues[j];
			pEntry = xvoCreateTable();
			if ( pEntry == NULL ) {
				xvoUnref(pArray);
				xvoUnref(pRoot);
				return NULL;
			}
			if ( !xvoTableSetInt(pEntry, MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_TILE, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_TILE), pValue->iTileId) ||
			     !xvoTableSetText(pEntry, MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_VALUE, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_VALUE), (str)pValue->sValue, 0, FALSE) ||
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

static int MapEditTilesetSetSaveItem(mapedit_app_t* pApp, int iIndex)
{
	mapedit_tileset_set_item_t* pItem;
	xvalue pRoot;
	xvalue pSpecialTiles;
	xvalue pSpecialTile;
	xvalue pPassage;
	xvalue pActorOverlay;
	xvalue pTileCustomData;
	str sPath;
	int i;
	int iRet;

	if ( (pApp == NULL) || (iIndex < 0) || (iIndex >= pApp->iTilesetSetCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pItem = &pApp->arrTilesetSetItems[iIndex];
	if ( pItem->sFile[0] == 0 ) {
		MapEditTilesetSetMakeUniqueFile(pItem->sFile, sizeof(pItem->sFile));
	}
	MapEditTilesetSetNormalizeSpecialCount(pItem);
	if ( MapEditTilesetSetEnsureDir() != XGE_OK ) {
		return XGE_ERROR;
	}
	pRoot = xvoCreateTable();
	pSpecialTiles = xvoCreateArray();
	pPassage = xvoCreateArray();
	if ( (pRoot == NULL) || (pSpecialTiles == NULL) || (pPassage == NULL) ) {
		xvoUnref(pRoot);
		xvoUnref(pSpecialTiles);
		xvoUnref(pPassage);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( !xvoTableSetText(pRoot, MAPEDIT_TILESET_SET_KEY_NAME, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_NAME), (str)pItem->sName, 0, FALSE) ||
	     !xvoTableSetInt(pRoot, MAPEDIT_TILESET_SET_KEY_SPECIAL_COUNT, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_SPECIAL_COUNT), pItem->iSpecialTileCount) ||
	     !xvoTableSetText(pRoot, MAPEDIT_TILESET_SET_KEY_STATIC_TILES, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_STATIC_TILES), (str)pItem->sStaticTiles, 0, FALSE) ||
	     !xvoTableSetText(pRoot, MAPEDIT_TILESET_SET_KEY_CUSTOM_DATA, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_CUSTOM_DATA), (str)pItem->sCustomData, 0, FALSE) ) {
		xvoUnref(pRoot);
		xvoUnref(pSpecialTiles);
		xvoUnref(pPassage);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < pItem->iSpecialTileCount; i++ ) {
		pSpecialTile = xvoCreateTable();
		if ( pSpecialTile == NULL ) {
			xvoUnref(pRoot);
			xvoUnref(pSpecialTiles);
			xvoUnref(pPassage);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		if ( !xvoTableSetText(pSpecialTile, MAPEDIT_TILESET_SET_KEY_SPECIAL_TYPE, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_SPECIAL_TYPE), (str)pItem->arrSpecialTiles[i].sType, 0, FALSE) ||
		     !xvoTableSetText(pSpecialTile, MAPEDIT_TILESET_SET_KEY_SPECIAL_FILE, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_SPECIAL_FILE), (str)pItem->arrSpecialTiles[i].sFile, 0, FALSE) ||
		     !xvoArrayAppendValue(pSpecialTiles, pSpecialTile, TRUE) ) {
			xvoUnref(pSpecialTile);
			xvoUnref(pRoot);
			xvoUnref(pSpecialTiles);
			xvoUnref(pPassage);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( !xvoTableSetValue(pRoot, MAPEDIT_TILESET_SET_KEY_SPECIAL_TILES, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_SPECIAL_TILES), pSpecialTiles, TRUE) ) {
		xvoUnref(pRoot);
		xvoUnref(pSpecialTiles);
		xvoUnref(pPassage);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( pItem->iPassageCount < 0 ) {
		pItem->iPassageCount = 0;
	} else if ( pItem->iPassageCount > MAPEDIT_TILESET_PASSAGE_MAX ) {
		pItem->iPassageCount = MAPEDIT_TILESET_PASSAGE_MAX;
	}
	for ( i = 0; i < pItem->iPassageCount; i++ ) {
		if ( !xvoArrayAppendInt(pPassage, (int)pItem->arrPassage[i]) ) {
			xvoUnref(pRoot);
			xvoUnref(pPassage);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( !xvoTableSetValue(pRoot, MAPEDIT_TILESET_SET_KEY_PASSAGE, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_PASSAGE), pPassage, TRUE) ) {
		xvoUnref(pRoot);
		xvoUnref(pPassage);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pActorOverlay = xvoCreateArray();
	if ( pActorOverlay == NULL ) {
		xvoUnref(pRoot);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( pItem->iActorOverlayCount < 0 ) {
		pItem->iActorOverlayCount = 0;
	} else if ( pItem->iActorOverlayCount > MAPEDIT_TILESET_ACTOR_OVERLAY_MAX ) {
		pItem->iActorOverlayCount = MAPEDIT_TILESET_ACTOR_OVERLAY_MAX;
	}
	for ( i = 0; i < pItem->iActorOverlayCount; i++ ) {
		if ( !xvoArrayAppendInt(pActorOverlay, pItem->arrActorOverlay[i] ? 1 : 0) ) {
			xvoUnref(pRoot);
			xvoUnref(pActorOverlay);
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( !xvoTableSetValue(pRoot, MAPEDIT_TILESET_SET_KEY_ACTOR_OVERLAY, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_ACTOR_OVERLAY), pActorOverlay, TRUE) ) {
		xvoUnref(pRoot);
		xvoUnref(pActorOverlay);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pTileCustomData = MapEditTilesetSetBuildCustomDataValue(pItem);
	if ( pTileCustomData == NULL ) {
		xvoUnref(pRoot);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( !xvoTableSetValue(pRoot, MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_DATA, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_DATA), pTileCustomData, TRUE) ) {
		xvoUnref(pRoot);
		xvoUnref(pTileCustomData);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	sPath = MapEditTilesetSetFilePath(pItem->sFile);
	if ( sPath == NULL ) {
		xvoUnref(pRoot);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xrtStringifyXSON_File(sPath, pRoot, 1, 0) ? XGE_OK : XGE_ERROR;
	xrtFree(sPath);
	xvoUnref(pRoot);
	return iRet;
}

int MapEditTilesetSetSaveSelected(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return MapEditTilesetSetSaveItem(pApp, pApp->iTilesetSetSelected);
}

static int MapEditTilesetSetAppend(mapedit_app_t* pApp, const char* sName, const char* sFile)
{
	mapedit_tileset_set_item_t* pItem;
	int iIndex;

	if ( pApp == NULL || pApp->iTilesetSetCount >= MAPEDIT_TILESET_SET_MAX ) {
		return -1;
	}
	iIndex = pApp->iTilesetSetCount++;
	pItem = &pApp->arrTilesetSetItems[iIndex];
	memset(pItem, 0, sizeof(*pItem));
	if ( (sFile != NULL) && (sFile[0] != 0) ) {
		MapEditTilesetSetCopyText(pItem->sFile, sizeof(pItem->sFile), sFile);
	} else {
		MapEditTilesetSetMakeUniqueFile(pItem->sFile, sizeof(pItem->sFile));
	}
	MapEditTilesetSetCopyText(pItem->sName, sizeof(pItem->sName), sName);
	MapEditTilesetSetCopyText(pItem->sStaticTiles, sizeof(pItem->sStaticTiles), "未设置");
	MapEditTilesetSetCopyText(pItem->sCustomData, sizeof(pItem->sCustomData), "");
	pItem->iSpecialTileCount = 0;
	MapEditTilesetSetBuildText(pItem);
	pApp->arrTilesetSetListItems[iIndex] = pItem->sText;
	return iIndex;
}

static const char* MapEditTilesetSetReadText(xvalue pTable, const char* sKey, const char* sDefault)
{
	xvalue pValue;

	if ( (pTable == NULL) || (sKey == NULL) ) {
		return sDefault;
	}
	pValue = xvoTableGetValue(pTable, sKey, (uint32)strlen(sKey));
	if ( xvoType(pValue) == XVO_DT_TEXT ) {
		return (const char*)xvoGetText(pValue);
	}
	return sDefault;
}

static int MapEditTilesetSetReadInt(xvalue pTable, const char* sKey, int iDefault)
{
	xvalue pValue;

	if ( (pTable == NULL) || (sKey == NULL) ) {
		return iDefault;
	}
	pValue = xvoTableGetValue(pTable, sKey, (uint32)strlen(sKey));
	if ( xvoType(pValue) == XVO_DT_NULL ) {
		return iDefault;
	}
	return (int)xvoGetInt(pValue);
}

static void MapEditTilesetSetValueToText(xvalue pValue, char* sDst, int iDstSize)
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
		MapEditTilesetSetCopyText(sDst, iDstSize, xvoGetBool(pValue) ? "true" : "false");
		break;
	case XVO_DT_INT:
		snprintf(sDst, (size_t)iDstSize, "%lld", (long long)xvoGetInt(pValue));
		break;
	case XVO_DT_FLOAT:
		snprintf(sDst, (size_t)iDstSize, "%.6g", xvoGetFloat(pValue));
		break;
	case XVO_DT_TEXT:
		MapEditTilesetSetCopyText(sDst, iDstSize, (const char*)xvoGetText(pValue));
		break;
	default:
		break;
	}
}

static void MapEditTilesetSetLoadCustomData(mapedit_app_t* pApp, mapedit_tileset_set_item_t* pItem, xvalue pRoot)
{
	const mapedit_custom_channel_def_t* pDef;
	xvalue pTileCustomData;
	xvalue pArray;
	xvalue pEntry;
	xvalue pValue;
	char sValue[MAPEDIT_CUSTOM_VALUE_CAPACITY];
	int i;
	int j;
	int iCount;
	int iTileId;

	if ( pApp == NULL || pItem == NULL || pRoot == NULL ) {
		return;
	}
	pTileCustomData = xvoTableGetValue(pRoot, MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_DATA, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_DATA));
	if ( xvoType(pTileCustomData) != XVO_DT_TABLE ) {
		return;
	}
	for ( i = 0; i < pApp->iCustomChannelCount; i++ ) {
		pDef = &pApp->arrCustomChannels[i];
		if ( pDef->sId[0] == 0 ) {
			continue;
		}
		pArray = xvoTableGetValue(pTileCustomData, pDef->sId, (uint32)strlen(pDef->sId));
		if ( xvoType(pArray) != XVO_DT_ARRAY ) {
			continue;
		}
		iCount = (int)xvoArrayItemCount(pArray);
		for ( j = 0; j < iCount; j++ ) {
			pEntry = xvoArrayGetValue(pArray, (uint32)j);
			if ( xvoType(pEntry) != XVO_DT_TABLE ) {
				continue;
			}
			iTileId = MapEditTilesetSetReadInt(pEntry, MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_TILE, -1);
			if ( iTileId < 0 || iTileId >= MAPEDIT_TILESET_CUSTOM_TILE_MAX ) {
				continue;
			}
			pValue = xvoTableGetValue(pEntry, MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_VALUE, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_TILE_CUSTOM_VALUE));
			MapEditTilesetSetValueToText(pValue, sValue, sizeof(sValue));
			MapEditTilesetSetSetTileCustomValue(pItem, pDef->sId, iTileId, sValue);
		}
	}
}

static int MapEditTilesetSetLoadFile(mapedit_app_t* pApp, const char* sPath, size_t iSize)
{
	mapedit_tileset_set_item_t* pItem;
	xvalue pRoot;
	xvalue pSpecialTiles;
	xvalue pSpecialTile;
	xvalue pPassage;
	xvalue pPassageValue;
	xvalue pActorOverlay;
	xvalue pActorOverlayValue;
	str sFile;
	const char* sName;
	const char* sStaticTiles;
	const char* sCustomData;
	int iIndex;
	int iCount;
	int iPassageCount;
	int iPassageValue;
	int iActorOverlayCount;
	int iActorOverlayValue;
	int i;

	if ( (pApp == NULL) || (sPath == NULL) || (pApp->iTilesetSetCount >= MAPEDIT_TILESET_SET_MAX) || !MapEditTilesetSetIsXsonFile(sPath) ) {
		return 0;
	}
	pRoot = xrtParseXSON_File((str)sPath);
	if ( (pRoot == NULL) || (xvoType(pRoot) != XVO_DT_TABLE) ) {
		xvoUnref(pRoot);
		return 0;
	}
	sFile = xrtPathGetNameExt((str)sPath, iSize);
	if ( sFile == NULL || sFile[0] == 0 ) {
		xvoUnref(pRoot);
		xrtFree(sFile);
		return 0;
	}
	sName = MapEditTilesetSetReadText(pRoot, MAPEDIT_TILESET_SET_KEY_NAME, (const char*)sFile);
	sStaticTiles = MapEditTilesetSetReadText(pRoot, MAPEDIT_TILESET_SET_KEY_STATIC_TILES, "未设置");
	sCustomData = MapEditTilesetSetReadText(pRoot, MAPEDIT_TILESET_SET_KEY_CUSTOM_DATA, "");
	pSpecialTiles = xvoTableGetValue(pRoot, MAPEDIT_TILESET_SET_KEY_SPECIAL_TILES, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_SPECIAL_TILES));
	pPassage = xvoTableGetValue(pRoot, MAPEDIT_TILESET_SET_KEY_PASSAGE, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_PASSAGE));
	pActorOverlay = xvoTableGetValue(pRoot, MAPEDIT_TILESET_SET_KEY_ACTOR_OVERLAY, (uint32)strlen(MAPEDIT_TILESET_SET_KEY_ACTOR_OVERLAY));
	iCount = MapEditTilesetSetReadInt(pRoot, MAPEDIT_TILESET_SET_KEY_SPECIAL_COUNT,
		(xvoType(pSpecialTiles) == XVO_DT_ARRAY) ? (int)xvoArrayItemCount(pSpecialTiles) : 0);
	iIndex = pApp->iTilesetSetCount++;
	pItem = &pApp->arrTilesetSetItems[iIndex];
	memset(pItem, 0, sizeof(*pItem));
	MapEditTilesetSetCopyText(pItem->sFile, sizeof(pItem->sFile), (const char*)sFile);
	MapEditTilesetSetCopyText(pItem->sName, sizeof(pItem->sName), sName);
	MapEditTilesetSetCopyText(pItem->sStaticTiles, sizeof(pItem->sStaticTiles), sStaticTiles);
	MapEditTilesetSetCopyText(pItem->sCustomData, sizeof(pItem->sCustomData), sCustomData);
	pItem->iSpecialTileCount = iCount;
	MapEditTilesetSetNormalizeSpecialCount(pItem);
	if ( xvoType(pSpecialTiles) == XVO_DT_ARRAY ) {
		iCount = (int)xvoArrayItemCount(pSpecialTiles);
		if ( iCount > pItem->iSpecialTileCount ) {
			iCount = pItem->iSpecialTileCount;
		}
		for ( i = 0; i < iCount; i++ ) {
			pSpecialTile = xvoArrayGetValue(pSpecialTiles, (uint32)i);
			if ( xvoType(pSpecialTile) != XVO_DT_TABLE ) {
				continue;
			}
			MapEditTilesetSetCopyText(pItem->arrSpecialTiles[i].sType, sizeof(pItem->arrSpecialTiles[i].sType),
				MapEditTilesetSetReadText(pSpecialTile, MAPEDIT_TILESET_SET_KEY_SPECIAL_TYPE, ""));
			MapEditTilesetSetCopyText(pItem->arrSpecialTiles[i].sFile, sizeof(pItem->arrSpecialTiles[i].sFile),
				MapEditTilesetSetReadText(pSpecialTile, MAPEDIT_TILESET_SET_KEY_SPECIAL_FILE, ""));
		}
	}
	if ( xvoType(pPassage) == XVO_DT_ARRAY ) {
		iPassageCount = (int)xvoArrayItemCount(pPassage);
		if ( iPassageCount > MAPEDIT_TILESET_PASSAGE_MAX ) {
			iPassageCount = MAPEDIT_TILESET_PASSAGE_MAX;
		}
		for ( i = 0; i < iPassageCount; i++ ) {
			pPassageValue = xvoArrayGetValue(pPassage, (uint32)i);
			if ( xvoType(pPassageValue) == XVO_DT_NULL ) {
				pItem->arrPassage[i] = (unsigned char)MAPEDIT_TILESET_PASSAGE_DEFAULT;
				continue;
			}
			iPassageValue = (int)xvoGetInt(pPassageValue);
			if ( iPassageValue < 0 ) {
				iPassageValue = 0;
			} else if ( iPassageValue > 255 ) {
				iPassageValue = 255;
			}
			pItem->arrPassage[i] = (unsigned char)iPassageValue;
		}
		pItem->iPassageCount = iPassageCount;
	}
	if ( xvoType(pActorOverlay) == XVO_DT_ARRAY ) {
		iActorOverlayCount = (int)xvoArrayItemCount(pActorOverlay);
		if ( iActorOverlayCount > MAPEDIT_TILESET_ACTOR_OVERLAY_MAX ) {
			iActorOverlayCount = MAPEDIT_TILESET_ACTOR_OVERLAY_MAX;
		}
		for ( i = 0; i < iActorOverlayCount; i++ ) {
			pActorOverlayValue = xvoArrayGetValue(pActorOverlay, (uint32)i);
			if ( xvoType(pActorOverlayValue) == XVO_DT_NULL ) {
				pItem->arrActorOverlay[i] = (unsigned char)MAPEDIT_TILESET_ACTOR_OVERLAY_DEFAULT;
				continue;
			}
			iActorOverlayValue = (int)xvoGetInt(pActorOverlayValue);
			pItem->arrActorOverlay[i] = (unsigned char)(iActorOverlayValue != 0);
		}
		pItem->iActorOverlayCount = iActorOverlayCount;
	}
	MapEditTilesetSetLoadCustomData(pApp, pItem, pRoot);
	MapEditTilesetSetBuildText(pItem);
	pApp->arrTilesetSetListItems[iIndex] = pItem->sText;
	xrtFree(sFile);
	xvoUnref(pRoot);
	return 1;
}

static int MapEditTilesetSetScanProc(str sPath, size_t iSize, int bDir, ptr pData, ptr pParam)
{
	mapedit_tileset_set_scan_t* pScan;

	(void)pData;
	pScan = (mapedit_tileset_set_scan_t*)pParam;
	if ( (pScan == NULL) || (pScan->pApp == NULL) || bDir ) {
		return 0;
	}
	if ( MapEditTilesetSetLoadFile(pScan->pApp, (const char*)sPath, iSize) ) {
		pScan->iLoadedCount++;
	}
	return 0;
}

static void MapEditTilesetSetLoadAll(mapedit_app_t* pApp)
{
	mapedit_tileset_set_scan_t tScan;
	str sDir;
	int iIndex;
	int i;

	if ( pApp == NULL ) {
		return;
	}
	for ( i = 0; i < pApp->iTilesetSetCount; i++ ) {
		MapEditTilesetSetClearCustomData(&pApp->arrTilesetSetItems[i]);
	}
	pApp->iTilesetSetCount = 0;
	pApp->iTilesetSetSelected = -1;
	if ( MapEditTilesetSetEnsureDir() != XGE_OK ) {
		iIndex = MapEditTilesetSetAppend(pApp, "默认图集", MAPEDIT_TILESET_SET_DEFAULT_FILE);
		pApp->iTilesetSetSelected = iIndex;
		return;
	}
	sDir = MapEditTilesetSetDirPath();
	if ( sDir != NULL ) {
		memset(&tScan, 0, sizeof(tScan));
		tScan.pApp = pApp;
		xrtDirScan(sDir, 0, MapEditTilesetSetScanProc, &tScan);
		xrtFree(sDir);
	}
	if ( pApp->iTilesetSetCount <= 0 ) {
		iIndex = MapEditTilesetSetAppend(pApp, "默认图集", MAPEDIT_TILESET_SET_DEFAULT_FILE);
		if ( iIndex >= 0 ) {
			pApp->iTilesetSetSelected = iIndex;
			MapEditTilesetSetSaveItem(pApp, iIndex);
		}
	} else {
		pApp->iTilesetSetSelected = 0;
	}
}

static void MapEditTilesetSetEnsureDefaults(mapedit_app_t* pApp)
{
	if ( pApp == NULL || pApp->iTilesetSetCount > 0 ) {
		return;
	}
	MapEditTilesetSetLoadAll(pApp);
}

static void MapEditTilesetSetRefreshList(mapedit_app_t* pApp)
{
	int i;

	if ( pApp == NULL ) {
		return;
	}
	for ( i = 0; i < pApp->iTilesetSetCount; i++ ) {
		MapEditTilesetSetBuildText(&pApp->arrTilesetSetItems[i]);
		pApp->arrTilesetSetListItems[i] = pApp->arrTilesetSetItems[i].sText;
	}
	if ( pApp->pTilesetSetListWidget != NULL ) {
		xgeXuiListViewSetItems(&pApp->tTilesetSetList, pApp->arrTilesetSetListItems, pApp->iTilesetSetCount);
		xgeXuiListViewSetSelected(&pApp->tTilesetSetList, pApp->iTilesetSetSelected);
		xgeXuiWidgetMarkLayout(pApp->pTilesetSetListWidget);
	}
}

static void MapEditTilesetPropertiesBuildEmpty(mapedit_app_t* pApp)
{
	xge_xui_property_grid pGrid;
	int iCategory;

	if ( pApp == NULL || pApp->pTilesetPropertiesGridWidget == NULL ) {
		return;
	}
	pGrid = &pApp->tTilesetPropertiesGrid;
	xgeXuiPropertyGridClear(pGrid);
	iCategory = xgeXuiPropertyGridAddCategory(pGrid, "empty", "图块属性", 1);
	if ( iCategory >= 0 ) {
		MapEditTilesetSetAddPropertyEx(pGrid, iCategory, "empty.hint", "当前对象", "选择图集后显示图集属性", XGE_XUI_TABLE_CELL_TYPE_TEXT, "未选择", "未选择", XGE_XUI_PROPERTY_FLAG_READONLY);
	}
}

static void MapEditTilesetPropertiesBuildTileset(mapedit_app_t* pApp)
{
	mapedit_tileset_set_item_t* pItem;
	xge_xui_property_grid pGrid;
	xge_xui_table_grid_editor_config_t tEnum;
	char sSpecialCount[32];
	int iCategory;
	int iStaticProp;

	if ( (pApp == NULL) || (pApp->pTilesetPropertiesGridWidget == NULL) ) {
		return;
	}
	pGrid = &pApp->tTilesetPropertiesGrid;
	xgeXuiPropertyGridClear(pGrid);
	if ( pApp->iTilesetSetSelected < 0 || pApp->iTilesetSetSelected >= pApp->iTilesetSetCount ) {
		MapEditTilesetPropertiesBuildEmpty(pApp);
		return;
	}
	pItem = &pApp->arrTilesetSetItems[pApp->iTilesetSetSelected];
	iCategory = xgeXuiPropertyGridAddCategory(pGrid, "tileset.set", "图集", 1);
	if ( iCategory < 0 ) {
		return;
	}
	snprintf(sSpecialCount, sizeof(sSpecialCount), "%d", pItem->iSpecialTileCount);
	MapEditTilesetStaticTileLoadOptions(pApp);
	MapEditTilesetStaticTileEnsureOption(pApp, pItem->sStaticTiles);
	MapEditTilesetSetAddProperty(pGrid, iCategory, "tileset.name", "图集名称", "当前图集的显示名称", XGE_XUI_TABLE_CELL_TYPE_TEXT, pItem->sName, "默认图集");
	MapEditTilesetSetAddProperty(pGrid, iCategory, "tileset.special_tile_count", "特殊图块数量", "独立素材图块槽位数量，0 号图块仍保留为橡皮", XGE_XUI_TABLE_CELL_TYPE_INT, sSpecialCount, "0");
	iStaticProp = MapEditTilesetSetAddProperty(pGrid, iCategory, "tileset.static_tilesheet", "静态图块集", "从 assets/tilesheets 中选择静态图块集素材", XGE_XUI_TABLE_CELL_TYPE_ENUM, pItem->sStaticTiles, "未设置");
	if ( iStaticProp >= 0 ) {
		memset(&tEnum, 0, sizeof(tEnum));
		tEnum.arrEnumItems = pApp->arrTilesetStaticTileOptions;
		tEnum.iEnumItemCount = pApp->iTilesetStaticTileOptionCount;
		tEnum.iEnumSelected = MapEditTilesetStaticTileFindOption(pApp, pItem->sStaticTiles);
		xgeXuiPropertyGridSetEditorConfig(pGrid, iStaticProp, &tEnum);
	}
	MapEditTilesetSetAddProperty(pGrid, iCategory, "tileset.custom_data", "自定义数据", "图集级自定义数据，后续可用于通道默认值或扩展配置", XGE_XUI_TABLE_CELL_TYPE_TEXTAREA, pItem->sCustomData, "");
	xgeXuiPropertyGridSetSelected(pGrid, xgeXuiPropertyGridFindProperty(pGrid, "tileset.name"));
}

static void MapEditTilesetPropertiesBuildSpecialTile(mapedit_app_t* pApp)
{
	mapedit_tileset_set_item_t* pItem;
	mapedit_tileset_special_tile_t* pSpecial;
	xge_xui_property_grid pGrid;
	xge_xui_table_grid_editor_config_t tEnum;
	const char* sType;
	const char* sFileDisplay;
	char sTileId[32];
	char sSlotIndex[32];
	int iTileId;
	int iSlot;
	int iCategory;
	int iTypeProp;
	int iFileProp;

	if ( (pApp == NULL) || (pApp->pTilesetPropertiesGridWidget == NULL) ) {
		return;
	}
	pGrid = &pApp->tTilesetPropertiesGrid;
	xgeXuiPropertyGridClear(pGrid);
	if ( pApp->iTilesetSetSelected < 0 || pApp->iTilesetSetSelected >= pApp->iTilesetSetCount ) {
		pApp->iTilesetPropertiesMode = MAPEDIT_TILESET_PROPERTY_MODE_SET;
		pApp->iTilesetSelectedSpecialTileId = -1;
		MapEditTilesetPropertiesBuildEmpty(pApp);
		return;
	}
	pItem = &pApp->arrTilesetSetItems[pApp->iTilesetSetSelected];
	iTileId = pApp->iTilesetSelectedSpecialTileId;
	if ( iTileId < 1 || iTileId > pItem->iSpecialTileCount || iTileId > MAPEDIT_TILESET_SPECIAL_TILE_MAX ) {
		pApp->iTilesetPropertiesMode = MAPEDIT_TILESET_PROPERTY_MODE_SET;
		pApp->iTilesetSelectedSpecialTileId = -1;
		MapEditTilesetPropertiesBuildTileset(pApp);
		return;
	}
	iSlot = iTileId - 1;
	pSpecial = &pItem->arrSpecialTiles[iSlot];
	sType = MapEditTilesetSpecialTypeOrDefault(pSpecial->sType);
	MapEditTilesetSpecialFileLoadOptions(pApp, sType, pSpecial->sFile);
	sFileDisplay = MapEditTilesetSpecialFileDisplayForValue(pApp, pSpecial->sFile);
	iCategory = xgeXuiPropertyGridAddCategory(pGrid, "tileset.special", "特殊图块", 1);
	if ( iCategory < 0 ) {
		return;
	}
	snprintf(sTileId, sizeof(sTileId), "%d", iTileId);
	snprintf(sSlotIndex, sizeof(sSlotIndex), "%d", iSlot);
	MapEditTilesetSetAddPropertyEx(pGrid, iCategory, "special.tile_id", "图块ID", "图集中的图块 ID", XGE_XUI_TABLE_CELL_TYPE_INT, sTileId, sTileId, XGE_XUI_PROPERTY_FLAG_READONLY);
	MapEditTilesetSetAddPropertyEx(pGrid, iCategory, "special.slot_index", "槽位序号", "特殊图块数组中的序号，从 0 开始", XGE_XUI_TABLE_CELL_TYPE_INT, sSlotIndex, sSlotIndex, XGE_XUI_PROPERTY_FLAG_READONLY);
	iTypeProp = MapEditTilesetSetAddProperty(pGrid, iCategory, "special.type", "特殊图块类型", "选择这个特殊槽位使用的素材类型", XGE_XUI_TABLE_CELL_TYPE_ENUM, sType, "动态图块");
	if ( iTypeProp >= 0 ) {
		memset(&tEnum, 0, sizeof(tEnum));
		tEnum.arrEnumItems = g_arrTilesetSpecialTypeOptions;
		tEnum.iEnumItemCount = MapEditTilesetSpecialCategoryCount();
		tEnum.iEnumSelected = MapEditTilesetSpecialTypeFind(sType);
		xgeXuiPropertyGridSetEditorConfig(pGrid, iTypeProp, &tEnum);
	}
	iFileProp = MapEditTilesetSetAddProperty(pGrid, iCategory, "special.file", "特殊图块文件", "选择对应素材目录下的图块文件，显示为映射名和文件名", XGE_XUI_TABLE_CELL_TYPE_ENUM, sFileDisplay, "未设置");
	if ( iFileProp >= 0 ) {
		memset(&tEnum, 0, sizeof(tEnum));
		tEnum.arrEnumItems = pApp->arrTilesetSpecialFileOptions;
		tEnum.iEnumItemCount = pApp->iTilesetSpecialFileOptionCount;
		tEnum.iEnumSelected = MapEditTilesetSpecialFileFindByValue(pApp, pSpecial->sFile);
		xgeXuiPropertyGridSetEditorConfig(pGrid, iFileProp, &tEnum);
	}
	xgeXuiPropertyGridSetSelected(pGrid, xgeXuiPropertyGridFindProperty(pGrid, "special.type"));
}

void MapEditTilesetPropertiesRefresh(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->iTilesetPropertiesMode == MAPEDIT_TILESET_PROPERTY_MODE_SPECIAL ) {
		MapEditTilesetPropertiesBuildSpecialTile(pApp);
	} else if ( pApp->iTilesetPropertiesMode == MAPEDIT_TILESET_PROPERTY_MODE_TILE_CUSTOM ) {
		MapEditTilesetPropertiesShowTileCustomData(pApp, pApp->iTilesetTagsSelectedTile);
	} else {
		MapEditTilesetPropertiesBuildTileset(pApp);
	}
}

void MapEditTilesetPropertiesShowTileset(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	pApp->iTilesetPropertiesMode = MAPEDIT_TILESET_PROPERTY_MODE_SET;
	pApp->iTilesetSelectedSpecialTileId = -1;
	MapEditTilesetPropertiesRefresh(pApp);
}

void MapEditTilesetPropertiesShowSpecialTile(mapedit_app_t* pApp, int iTileId)
{
	mapedit_tileset_set_item_t* pItem;

	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->iTilesetSetSelected < 0 || pApp->iTilesetSetSelected >= pApp->iTilesetSetCount ) {
		MapEditTilesetPropertiesShowTileset(pApp);
		return;
	}
	pItem = &pApp->arrTilesetSetItems[pApp->iTilesetSetSelected];
	if ( iTileId < 1 || iTileId > pItem->iSpecialTileCount || iTileId > MAPEDIT_TILESET_SPECIAL_TILE_MAX ) {
		MapEditTilesetPropertiesShowTileset(pApp);
		return;
	}
	pApp->iTilesetPropertiesMode = MAPEDIT_TILESET_PROPERTY_MODE_SPECIAL;
	pApp->iTilesetSelectedSpecialTileId = iTileId;
	MapEditTilesetPropertiesRefresh(pApp);
}

static const char* MapEditTilesetCustomDisplayValue(const mapedit_custom_channel_def_t* pDef, const char* sValue)
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

void MapEditTilesetPropertiesShowTileCustomData(mapedit_app_t* pApp, int iTileId)
{
	mapedit_tileset_set_item_t* pItem;
	const mapedit_custom_channel_def_t* pDef;
	xge_xui_property_grid pGrid;
	char sTileId[32];
	char sValue[MAPEDIT_CUSTOM_VALUE_CAPACITY];
	char sPropId[MAPEDIT_CUSTOM_ID_CAPACITY + 16];
	int iCategory;
	int i;

	if ( pApp == NULL || pApp->pTilesetPropertiesGridWidget == NULL ) {
		return;
	}
	pGrid = &pApp->tTilesetPropertiesGrid;
	xgeXuiPropertyGridClear(pGrid);
	pItem = MapEditTilesetSetSelectedItem(pApp);
	if ( pItem == NULL || iTileId < 0 ) {
		MapEditTilesetPropertiesBuildEmpty(pApp);
		return;
	}
	pApp->iTilesetPropertiesMode = MAPEDIT_TILESET_PROPERTY_MODE_TILE_CUSTOM;
	pApp->iTilesetSelectedSpecialTileId = -1;
	iCategory = xgeXuiPropertyGridAddCategory(pGrid, "tileset.tile_custom", "图块自定义数据", 1);
	if ( iCategory < 0 ) {
		return;
	}
	snprintf(sTileId, sizeof(sTileId), "%d", iTileId);
	MapEditTilesetSetAddPropertyEx(pGrid, iCategory, "tile_custom.tile_id", "图块ID", "当前查看的图块 ID", XGE_XUI_TABLE_CELL_TYPE_INT, sTileId, sTileId, XGE_XUI_PROPERTY_FLAG_READONLY);
	for ( i = 0; i < pApp->iCustomChannelCount; i++ ) {
		pDef = &pApp->arrCustomChannels[i];
		if ( pDef->sId[0] == 0 || (pDef->sScope[0] != 0 && strcmp(pDef->sScope, "tile") != 0) ) {
			continue;
		}
		if ( !MapEditTilesetSetGetTileCustomValue(pItem, pDef->sId, iTileId, sValue, sizeof(sValue)) ) {
			MapEditTilesetSetCopyText(sValue, sizeof(sValue), pDef->sDefaultValue);
		}
		snprintf(sPropId, sizeof(sPropId), "tile_custom.%s", pDef->sId);
		MapEditTilesetSetAddPropertyEx(pGrid, iCategory, sPropId, pDef->sName[0] ? pDef->sName : pDef->sId,
			pDef->sId, XGE_XUI_TABLE_CELL_TYPE_TEXT, MapEditTilesetCustomDisplayValue(pDef, sValue), "", XGE_XUI_PROPERTY_FLAG_READONLY);
	}
	xgeXuiPropertyGridSetSelected(pGrid, xgeXuiPropertyGridFindProperty(pGrid, "tile_custom.tile_id"));
}

static void MapEditTilesetSetSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	if ( iIndex < 0 || iIndex >= pApp->iTilesetSetCount ) {
		return;
	}
	pApp->iTilesetSetSelected = iIndex;
	MapEditTilesetPropertiesShowTileset(pApp);
	MapEditTilesetArrangeRefresh(pApp);
	MapEditTilesetPassageRefresh(pApp);
	MapEditTilesetActorOverlayRefresh(pApp);
	MapEditTilesetTagsRefresh(pApp);
	MapEditAppSetStatus(pApp, "已选择图集");
}

static void MapEditTilesetSetAddClick(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;
	char sName[64];
	int iIndex;

	(void)pWidget;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->iTilesetSetCount >= MAPEDIT_TILESET_SET_MAX ) {
		MapEditAppSetStatus(pApp, "图集数量已达到上限");
		return;
	}
	snprintf(sName, sizeof(sName), "新建图集%d", pApp->iTilesetSetCount + 1);
	iIndex = MapEditTilesetSetAppend(pApp, sName, NULL);
	if ( iIndex < 0 ) {
		MapEditAppSetStatus(pApp, "添加图集失败");
		return;
	}
	pApp->iTilesetSetSelected = iIndex;
	if ( MapEditTilesetSetSaveItem(pApp, iIndex) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "已添加图集，但保存失败");
	} else {
		MapEditAppSetStatus(pApp, "已添加图集");
	}
	MapEditTilesetSetRefreshList(pApp);
	MapEditTilesetPropertiesShowTileset(pApp);
	MapEditTilesetArrangeRefresh(pApp);
	MapEditTilesetPassageRefresh(pApp);
	MapEditTilesetActorOverlayRefresh(pApp);
	MapEditTilesetTagsRefresh(pApp);
}

static void MapEditTilesetPropertyChange(xge_xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	mapedit_app_t* pApp;
	mapedit_tileset_set_item_t* pItem;
	int iValue;
	int iFileProp;
	int bHandled;
	int bRefreshArrange;
	int bRepaintArrange;

	(void)pWidget;
	(void)iType;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL || sId == NULL || pApp->iTilesetSetSelected < 0 || pApp->iTilesetSetSelected >= pApp->iTilesetSetCount ) {
		return;
	}
	pItem = &pApp->arrTilesetSetItems[pApp->iTilesetSetSelected];
	bHandled = 0;
	bRefreshArrange = 0;
	bRepaintArrange = 0;
	if ( strcmp(sId, "tileset.name") == 0 ) {
		MapEditTilesetSetCopyText(pItem->sName, sizeof(pItem->sName), sValue);
		MapEditTilesetSetRefreshList(pApp);
		bHandled = 1;
	} else if ( strcmp(sId, "tileset.special_tile_count") == 0 ) {
		char sCount[32];

		iValue = (sValue != NULL) ? atoi(sValue) : 0;
		if ( iValue < 0 ) {
			iValue = 0;
		} else if ( iValue > MAPEDIT_TILESET_SPECIAL_TILE_MAX ) {
			iValue = MAPEDIT_TILESET_SPECIAL_TILE_MAX;
		}
		pItem->iSpecialTileCount = iValue;
		MapEditTilesetSetNormalizeSpecialCount(pItem);
		snprintf(sCount, sizeof(sCount), "%d", pItem->iSpecialTileCount);
		(void)xgeXuiPropertyGridSetValue(&pApp->tTilesetPropertiesGrid, iProperty, sCount);
		bRefreshArrange = 1;
		bHandled = 1;
	} else if ( strcmp(sId, "tileset.static_tilesheet") == 0 ) {
		MapEditTilesetSetCopyText(pItem->sStaticTiles, sizeof(pItem->sStaticTiles), sValue);
		MapEditTilesetSetRefreshList(pApp);
		bRefreshArrange = 1;
		bHandled = 1;
	} else if ( strcmp(sId, "tileset.custom_data") == 0 ) {
		MapEditTilesetSetCopyText(pItem->sCustomData, sizeof(pItem->sCustomData), sValue);
		bHandled = 1;
	} else if ( strcmp(sId, "special.type") == 0 ) {
		mapedit_tileset_special_tile_t* pSpecial;
		xge_xui_table_grid_editor_config_t tEnum;
		const char* sType;

		if ( pApp->iTilesetSelectedSpecialTileId < 1 || pApp->iTilesetSelectedSpecialTileId > pItem->iSpecialTileCount ||
		     pApp->iTilesetSelectedSpecialTileId > MAPEDIT_TILESET_SPECIAL_TILE_MAX ) {
			return;
		}
		pSpecial = &pItem->arrSpecialTiles[pApp->iTilesetSelectedSpecialTileId - 1];
		sType = MapEditTilesetSpecialTypeOrDefault(sValue);
		MapEditTilesetSetCopyText(pSpecial->sType, sizeof(pSpecial->sType), sType);
		MapEditTilesetSetCopyText(pSpecial->sFile, sizeof(pSpecial->sFile), "");
		MapEditTilesetSpecialFileLoadOptions(pApp, pSpecial->sType, pSpecial->sFile);
		iFileProp = xgeXuiPropertyGridFindProperty(&pApp->tTilesetPropertiesGrid, "special.file");
		if ( iFileProp >= 0 ) {
			memset(&tEnum, 0, sizeof(tEnum));
			tEnum.arrEnumItems = pApp->arrTilesetSpecialFileOptions;
			tEnum.iEnumItemCount = pApp->iTilesetSpecialFileOptionCount;
			tEnum.iEnumSelected = 0;
			xgeXuiPropertyGridSetValue(&pApp->tTilesetPropertiesGrid, iFileProp, "未设置");
			xgeXuiPropertyGridSetEditorConfig(&pApp->tTilesetPropertiesGrid, iFileProp, &tEnum);
		}
		bHandled = 1;
		bRepaintArrange = 1;
	} else if ( strcmp(sId, "special.file") == 0 ) {
		mapedit_tileset_special_tile_t* pSpecial;
		const char* sFile;
		const char* sDisplay;

		if ( pApp->iTilesetSelectedSpecialTileId < 1 || pApp->iTilesetSelectedSpecialTileId > pItem->iSpecialTileCount ||
		     pApp->iTilesetSelectedSpecialTileId > MAPEDIT_TILESET_SPECIAL_TILE_MAX ) {
			return;
		}
		pSpecial = &pItem->arrSpecialTiles[pApp->iTilesetSelectedSpecialTileId - 1];
		if ( pSpecial->sType[0] == 0 ) {
			MapEditTilesetSetCopyText(pSpecial->sType, sizeof(pSpecial->sType), MapEditTilesetSpecialTypeOrDefault(pSpecial->sType));
		}
		sFile = MapEditTilesetSpecialFileValueForDisplay(pApp, sValue);
		MapEditTilesetSetCopyText(pSpecial->sFile, sizeof(pSpecial->sFile), sFile);
		sDisplay = MapEditTilesetSpecialFileDisplayForValue(pApp, pSpecial->sFile);
		(void)xgeXuiPropertyGridSetValue(&pApp->tTilesetPropertiesGrid, iProperty, sDisplay);
		bHandled = 1;
		bRepaintArrange = 1;
	}
	if ( bHandled == 0 ) {
		return;
	}
	if ( MapEditTilesetSetSaveItem(pApp, pApp->iTilesetSetSelected) != XGE_OK ) {
		MapEditAppSetStatus(pApp, "图集属性已修改，但保存失败");
	} else {
		MapEditAppSetStatus(pApp, "图集属性已修改并保存");
	}
	if ( bRefreshArrange ) {
		MapEditTilesetArrangeRefresh(pApp);
		MapEditTilesetPassageRefresh(pApp);
		MapEditTilesetActorOverlayRefresh(pApp);
		MapEditTilesetTagsRefresh(pApp);
	} else if ( bRepaintArrange && pApp->pTilesetArrangeGridWidget != NULL ) {
		MapEditTilesetArrangeReloadSpecialTextures(pApp);
		xgeXuiWidgetMarkPaint(pApp->pTilesetArrangeGridWidget);
	}
}

int MapEditTilesetSetsCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
{
	xge_xui_widget pContent;
	xge_xui_widget pListWidget;
	xge_xui_widget pButtonWidget;

	if ( (pApp == NULL) || (pWindow == NULL) ) {
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
	MapEditTilesetSetEnsureDefaults(pApp);
	xgeXuiWidgetSetSize(pListWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pButtonWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(30.0f));
	xgeXuiWidgetSetBackground(pListWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pListWidget, 1.0f, XGE_COLOR_RGBA(164, 206, 236, 255));
	if ( xgeXuiListViewInit(&pApp->tTilesetSetList, &pApp->tXui, pListWidget) != XGE_OK ||
		xgeXuiButtonInit(&pApp->tTilesetSetAddButton, &pApp->tXui, pButtonWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pListWidget);
		xgeXuiWidgetFree(pButtonWidget);
		return XGE_ERROR;
	}
	xgeXuiListViewSetFont(&pApp->tTilesetSetList, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiListViewSetItemHeight(&pApp->tTilesetSetList, 24.0f);
	xgeXuiListViewSetScrollbarMode(&pApp->tTilesetSetList, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiListViewSetColors(&pApp->tTilesetSetList, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(204, 232, 250, 255), XGE_COLOR_RGBA(31, 75, 112, 255), XGE_COLOR_RGBA(230, 240, 247, 255), XGE_COLOR_RGBA(120, 176, 212, 255));
	xgeXuiListViewSetSelect(&pApp->tTilesetSetList, MapEditTilesetSetSelect, pApp);
	xgeXuiButtonSetText(&pApp->tTilesetSetAddButton, pApp->bFontReady ? &pApp->tFont : NULL, "添加图集");
	xgeXuiButtonSetSemantic(&pApp->tTilesetSetAddButton, XGE_XUI_BUTTON_SEMANTIC_PRIMARY);
	xgeXuiButtonSetClick(&pApp->tTilesetSetAddButton, MapEditTilesetSetAddClick, pApp);
	pApp->pTilesetSetListWidget = pListWidget;
	pApp->pTilesetSetAddButtonWidget = pButtonWidget;
	MapEditTilesetSetRefreshList(pApp);
	if ( xgeXuiWidgetAdd(pContent, pListWidget) != XGE_OK ||
	     xgeXuiWidgetAdd(pContent, pButtonWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	return XGE_OK;
}

int MapEditTilesetPropertiesCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow)
{
	xge_xui_widget pContent;
	xge_xui_widget pGridWidget;

	if ( (pApp == NULL) || (pWindow == NULL) ) {
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
	if ( xgeXuiPropertyGridInit(&pApp->tTilesetPropertiesGrid, &pApp->tXui, pGridWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pGridWidget);
		return XGE_ERROR;
	}
	xgeXuiPropertyGridSetFont(&pApp->tTilesetPropertiesGrid, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiPropertyGridSetMetrics(&pApp->tTilesetPropertiesGrid, 112.0f, 24.0f, 25.0f);
	xgeXuiPropertyGridSetDescriptionMode(&pApp->tTilesetPropertiesGrid, XGE_XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, 0.0f);
	xgeXuiPropertyGridSetEditMode(&pApp->tTilesetPropertiesGrid, XGE_XUI_TABLE_GRID_EDIT_QUICK);
	xgeXuiPropertyGridSetScrollbarMode(&pApp->tTilesetPropertiesGrid, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiPropertyGridSetColors(&pApp->tTilesetPropertiesGrid, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(226, 242, 252, 255), XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(204, 232, 250, 255), XGE_COLOR_RGBA(164, 206, 236, 255), XGE_COLOR_RGBA(31, 75, 112, 255));
	xgeXuiPropertyGridSetChange(&pApp->tTilesetPropertiesGrid, MapEditTilesetPropertyChange, pApp);
	pApp->pTilesetPropertiesGridWidget = pGridWidget;
	if ( xgeXuiWidgetAdd(pContent, pGridWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	MapEditTilesetSetEnsureDefaults(pApp);
	MapEditTilesetPropertiesShowTileset(pApp);
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	return XGE_OK;
}

void MapEditTilesetSetsUnit(mapedit_app_t* pApp)
{
	int i;

	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->pTilesetPropertiesGridWidget != NULL ) {
		xgeXuiPropertyGridUnit(&pApp->tTilesetPropertiesGrid);
		pApp->pTilesetPropertiesGridWidget = NULL;
	}
	if ( pApp->pTilesetSetAddButtonWidget != NULL ) {
		xgeXuiButtonUnit(&pApp->tTilesetSetAddButton);
		pApp->pTilesetSetAddButtonWidget = NULL;
	}
	if ( pApp->pTilesetSetListWidget != NULL ) {
		xgeXuiListViewUnit(&pApp->tTilesetSetList);
		pApp->pTilesetSetListWidget = NULL;
	}
	for ( i = 0; i < pApp->iTilesetSetCount; i++ ) {
		MapEditTilesetSetClearCustomData(&pApp->arrTilesetSetItems[i]);
	}
}
