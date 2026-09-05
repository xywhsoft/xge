#include "xge_map.h"
#include "xge.h"

#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XGE_MAP_KEY_FORMAT "format"
#define XGE_MAP_KEY_VERSION "version"
#define XGE_MAP_KEY_LAYER_COUNT "layerCount"
#define XGE_MAP_KEY_TILE_WIDTH "tileWidth"
#define XGE_MAP_KEY_TILE_HEIGHT "tileHeight"
#define XGE_MAP_KEY_NAME "地图名称"
#define XGE_MAP_KEY_TILESET "图集"
#define XGE_MAP_KEY_STATE "当前状态"
#define XGE_MAP_KEY_WIDTH "地图宽度"
#define XGE_MAP_KEY_HEIGHT "地图高度"
#define XGE_MAP_KEY_CUSTOM "自定义数据"
#define XGE_MAP_KEY_TILES "图块数据"
#define XGE_MAP_KEY_PASSAGE "通行修正"
#define XGE_MAP_KEY_CELL_DATA "cellCustomData"
#define XGE_MAP_KEY_CELL "cell"
#define XGE_MAP_KEY_VALUE "value"

static xstrview xgeMapText(const char* sText)
{
	xstrview tView;
	tView.Data = sText != NULL ? sText : "";
	tView.Size = strlen(tView.Data);
	return tView;
}

static void xgeMapError(char* sError, size_t iCapacity, const char* sFormat, ...)
{
	va_list tArgs;
	if ( sError == NULL || iCapacity == 0u ) return;
	va_start(tArgs, sFormat);
	(void)vsnprintf(sError, iCapacity, sFormat, tArgs);
	va_end(tArgs);
	sError[iCapacity - 1u] = 0;
}

static void xgeMapCopyText(char* sOut, size_t iCapacity, const char* sText, size_t iSize)
{
	if ( sOut == NULL || iCapacity == 0u ) return;
	if ( sText == NULL ) {
		sOut[0] = 0;
		return;
	}
	if ( iSize >= iCapacity ) iSize = iCapacity - 1u;
	if ( iSize > 0u ) memcpy(sOut, sText, iSize);
	sOut[iSize] = 0;
}

static xvalue* xgeMapObjectGet(const xvalue* pObject, const char* sKey)
{
	return pObject != NULL ? xrtValueObjectGet(pObject, xgeMapText(sKey)) : NULL;
}

static int xgeMapValueInt(const xvalue* pValue, int iDefault, int* pValid)
{
	int64 iValue = 0;
	if ( pValid != NULL ) *pValid = 0;
	if ( pValue == NULL || !xrtValueGetInt(pValue, &iValue) || iValue < INT_MIN || iValue > INT_MAX ) return iDefault;
	if ( pValid != NULL ) *pValid = 1;
	return (int)iValue;
}

static int xgeMapObjectInt(const xvalue* pObject, const char* sKey, int iDefault, int* pValid)
{
	return xgeMapValueInt(xgeMapObjectGet(pObject, sKey), iDefault, pValid);
}

static int xgeMapObjectReadInt(const xvalue* pObject, const char* sKey, int iDefault,
	int* pOut, int* pPresent)
{
	xvalue* pValue = xgeMapObjectGet(pObject, sKey);
	int bValid = 0;
	if ( pOut == NULL ) return 0;
	if ( pPresent != NULL ) *pPresent = pValue != NULL;
	if ( pValue == NULL ) {
		*pOut = iDefault;
		return 1;
	}
	*pOut = xgeMapValueInt(pValue, iDefault, &bValid);
	return bValid;
}

static int xgeMapObjectReadText(const xvalue* pObject, const char* sKey, const char* sDefault,
	char* sOut, size_t iCapacity)
{
	xstrview tText = { 0 };
	xvalue* pValue = xgeMapObjectGet(pObject, sKey);
	if ( pValue == NULL ) {
		xgeMapCopyText(sOut, iCapacity, sDefault, sDefault != NULL ? strlen(sDefault) : 0u);
		return 1;
	}
	if ( pValue != NULL && xrtValueGetString(pValue, &tText) ) {
		xgeMapCopyText(sOut, iCapacity, tText.Data, tText.Size);
		return 1;
	}
	return 0;
}

/* On failure these helpers release the value passed to them. */
static int xgeMapObjectSetTake(xvalue* pObject, const char* sKey, xvalue* pValue)
{
	xvalue* pTake = pValue;
	if ( pObject != NULL && pTake != NULL && xrtValueObjectSetTake(pObject, xgeMapText(sKey), &pTake) ) return 1;
	if ( pTake != NULL ) xrtValueRelease(pTake);
	return 0;
}

static int xgeMapObjectSetInt(xvalue* pObject, const char* sKey, int iValue)
{
	return xgeMapObjectSetTake(pObject, sKey, xrtValueInt((int64)iValue));
}

static int xgeMapObjectSetText(xvalue* pObject, const char* sKey, const char* sValue)
{
	return xgeMapObjectSetTake(pObject, sKey, xrtValueString(xgeMapText(sValue)));
}

static int xgeMapArrayAppendInt(xvalue* pArray, int iValue)
{
	xvalue* pTake = xrtValueInt((int64)iValue);
	if ( pTake != NULL && xrtValueArrayAppendTake(pArray, &pTake) ) return 1;
	if ( pTake != NULL ) xrtValueRelease(pTake);
	return 0;
}

static int xgeMapArrayAppendTake(xvalue* pArray, xvalue* pValue)
{
	xvalue* pTake = pValue;
	if ( pTake != NULL && xrtValueArrayAppendTake(pArray, &pTake) ) return 1;
	if ( pTake != NULL ) xrtValueRelease(pTake);
	return 0;
}

void xgeMapInit(xge_map_t* pMap)
{
	if ( pMap == NULL ) return;
	memset(pMap, 0, sizeof(*pMap));
	pMap->iWidth = 100;
	pMap->iHeight = 100;
	pMap->iLayers = XGE_MAP_LEGACY_DEFAULT_LAYERS;
	pMap->iTileWidth = 16;
	pMap->iTileHeight = 16;
	pMap->iSchemaVersion = XGE_MAP_FORMAT_VERSION;
	xgeMapCopyText(pMap->sName, sizeof(pMap->sName), "默认地图", strlen("默认地图"));
	xgeMapCopyText(pMap->sTileset, sizeof(pMap->sTileset), "default.xson", strlen("default.xson"));
}

void xgeMapSetDefault(xge_map_t* pMap)
{
	if ( pMap == NULL ) return;
	xgeMapUnit(pMap);
	xgeMapInit(pMap);
}

void xgeMapUnit(xge_map_t* pMap)
{
	if ( pMap == NULL ) return;
	free(pMap->pTiles);
	free(pMap->arrPassageOverrides);
	if ( pMap->pPassageRaw != NULL ) xrtValueRelease(pMap->pPassageRaw);
	if ( pMap->pCellDataRaw != NULL ) xrtValueRelease(pMap->pCellDataRaw);
	memset(pMap, 0, sizeof(*pMap));
}

int xgeMapValidateSize(int iWidth, int iHeight, int iLayerCount,
	size_t* pCellCount, size_t* pTileCount)
{
	size_t iCells;
	size_t iTiles;
	if ( iWidth <= 0 || iHeight <= 0 || iLayerCount <= 0 ) return XGE_MAP_ERROR_INVALID_ARGUMENT;
	if ( iWidth > XGE_MAP_DIMENSION_MAX || iHeight > XGE_MAP_DIMENSION_MAX || iLayerCount > XGE_MAP_LAYER_MAX ) return XGE_MAP_ERROR_LIMIT;
	if ( (size_t)iWidth > SIZE_MAX / (size_t)iHeight ) return XGE_MAP_ERROR_LIMIT;
	iCells = (size_t)iWidth * (size_t)iHeight;
	if ( iCells > XGE_MAP_CELL_MAX ) return XGE_MAP_ERROR_LIMIT;
	if ( iCells > SIZE_MAX / (size_t)iLayerCount ) return XGE_MAP_ERROR_LIMIT;
	iTiles = iCells * (size_t)iLayerCount;
	if ( iTiles > (size_t)INT_MAX || iTiles > SIZE_MAX / sizeof(int) ) return XGE_MAP_ERROR_LIMIT;
	if ( pCellCount != NULL ) *pCellCount = iCells;
	if ( pTileCount != NULL ) *pTileCount = iTiles;
	return XGE_MAP_OK;
}

int xgeMapAllocate(xge_map_t* pMap)
{
	size_t iTileCount = 0u;
	int* pTiles;
	int iRet;
	if ( pMap == NULL ) return XGE_MAP_ERROR_INVALID_ARGUMENT;
	iRet = xgeMapValidateSize(pMap->iWidth, pMap->iHeight, pMap->iLayers, NULL, &iTileCount);
	if ( iRet != XGE_MAP_OK ) return iRet;
	pTiles = (int*)calloc(iTileCount, sizeof(*pTiles));
	if ( pTiles == NULL ) return XGE_MAP_ERROR_OUT_OF_MEMORY;
	free(pMap->pTiles);
	pMap->pTiles = pTiles;
	pMap->iTileCount = (int)iTileCount;
	return XGE_MAP_OK;
}

int xgeMapCellId(const xge_map_t* pMap, int iX, int iY)
{
	if ( pMap == NULL || iX < 0 || iY < 0 || iX >= pMap->iWidth || iY >= pMap->iHeight ) return -1;
	return iY * pMap->iWidth + iX;
}

int xgeMapFindPassageOverride(const xge_map_t* pMap, int iCellId)
{
	int i;
	if ( pMap == NULL || iCellId < 0 ) return -1;
	for ( i = 0; i < pMap->iPassageOverrideCount; ++i ) {
		if ( pMap->arrPassageOverrides[i].iCellId == iCellId ) return i;
	}
	return -1;
}

int xgeMapSetPassageOverride(xge_map_t* pMap, int iCellId, unsigned char iValue)
{
	xge_map_passage_override_t* pNew;
	int iIndex;
	int iCapacity;
	size_t iCellCount = 0u;
	if ( pMap == NULL || xgeMapValidateSize(pMap->iWidth, pMap->iHeight, pMap->iLayers, &iCellCount, NULL) != XGE_MAP_OK ||
	     iCellId < 0 || (size_t)iCellId >= iCellCount ) return XGE_MAP_ERROR_INVALID_ARGUMENT;
	iIndex = xgeMapFindPassageOverride(pMap, iCellId);
	if ( iIndex >= 0 ) {
		pMap->arrPassageOverrides[iIndex].iValue = iValue;
		pMap->bDirty = 1;
		return XGE_MAP_OK;
	}
	if ( pMap->iPassageOverrideCount >= pMap->iPassageOverrideCapacity ) {
		iCapacity = pMap->iPassageOverrideCapacity > 0 ? pMap->iPassageOverrideCapacity * 2 : 64;
		if ( (size_t)iCapacity > iCellCount ) iCapacity = (int)iCellCount;
		pNew = (xge_map_passage_override_t*)realloc(pMap->arrPassageOverrides,
			(size_t)iCapacity * sizeof(*pNew));
		if ( pNew == NULL ) return XGE_MAP_ERROR_OUT_OF_MEMORY;
		pMap->arrPassageOverrides = pNew;
		pMap->iPassageOverrideCapacity = iCapacity;
	}
	pMap->arrPassageOverrides[pMap->iPassageOverrideCount].iCellId = iCellId;
	pMap->arrPassageOverrides[pMap->iPassageOverrideCount].iValue = iValue;
	++pMap->iPassageOverrideCount;
	pMap->bDirty = 1;
	return XGE_MAP_OK;
}

int xgeMapRemovePassageOverride(xge_map_t* pMap, int iIndex)
{
	if ( pMap == NULL || iIndex < 0 || iIndex >= pMap->iPassageOverrideCount ) return XGE_MAP_ERROR_INVALID_ARGUMENT;
	if ( iIndex + 1 < pMap->iPassageOverrideCount ) {
		memmove(&pMap->arrPassageOverrides[iIndex], &pMap->arrPassageOverrides[iIndex + 1],
			(size_t)(pMap->iPassageOverrideCount - iIndex - 1) * sizeof(pMap->arrPassageOverrides[0]));
	}
	--pMap->iPassageOverrideCount;
	pMap->bDirty = 1;
	return XGE_MAP_OK;
}

static int xgeMapInferLegacyLayers(size_t iSerializedTiles, size_t iCellCount)
{
	size_t iLayers;
	if ( iCellCount == 0u || iSerializedTiles == 0u || iSerializedTiles % iCellCount != 0u ) return XGE_MAP_LEGACY_DEFAULT_LAYERS;
	iLayers = iSerializedTiles / iCellCount;
	return iLayers >= 1u && iLayers <= XGE_MAP_LAYER_MAX ? (int)iLayers : XGE_MAP_LEGACY_DEFAULT_LAYERS;
}

static int xgeMapLoadPassageOverrides(xge_map_t* pMap, const xvalue* pArray)
{
	size_t i;
	size_t iCount;
	size_t iCellCount = 0u;
	if ( pArray == NULL || !xrtValueIs(pArray, XVALUE_ARRAY) ) return XGE_MAP_OK;
	if ( xgeMapValidateSize(pMap->iWidth, pMap->iHeight, pMap->iLayers, &iCellCount, NULL) != XGE_MAP_OK ) return XGE_MAP_ERROR_FORMAT;
	iCount = xrtValueCount(pArray);
	for ( i = 0u; i < iCount; ++i ) {
		xvalue* pEntry = xrtValueArrayGet(pArray, i);
		int iCell;
		int iValue;
		if ( pEntry == NULL || !xrtValueIs(pEntry, XVALUE_OBJECT) ) continue;
		iCell = xgeMapObjectInt(pEntry, XGE_MAP_KEY_CELL, -1, NULL);
		iValue = xgeMapObjectInt(pEntry, XGE_MAP_KEY_VALUE, 255, NULL);
		if ( iCell < 0 || (size_t)iCell >= iCellCount ) continue;
		if ( iValue < 0 ) iValue = 0;
		if ( iValue > 255 ) iValue = 255;
		if ( xgeMapSetPassageOverride(pMap, iCell, (unsigned char)iValue) != XGE_MAP_OK ) return XGE_MAP_ERROR_OUT_OF_MEMORY;
	}
	return XGE_MAP_OK;
}

int xgeMapLoadXson(xge_map_t* pMap, const char* sPath, char* sError, size_t iErrorCapacity)
{
	xge_map_t tLoaded;
	xvalue* pRoot;
	xvalue* pTiles;
	xvalue* pPassage;
	xvalue* pCellData;
	size_t iCellCount = 0u;
	size_t iSerializedTiles = 0u;
	int iVersion;
	int bVersionPresent;
	int bLayerPresent;
	int iRet;
	size_t i;
	char sFormat[32];
	if ( sError != NULL && iErrorCapacity > 0u ) sError[0] = 0;
	if ( pMap == NULL || sPath == NULL || sPath[0] == 0 ) return XGE_MAP_ERROR_INVALID_ARGUMENT;
	pRoot = xrtXsonParseFile(sPath);
	if ( pRoot == NULL ) {
		xgeMapError(sError, iErrorCapacity, "无法读取地图文件: %s", sPath);
		return XGE_MAP_ERROR_FILE;
	}
	if ( !xrtValueIs(pRoot, XVALUE_OBJECT) ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "地图根节点必须是对象");
		return XGE_MAP_ERROR_FORMAT;
	}
	xgeMapInit(&tLoaded);
	if ( !xgeMapObjectReadInt(pRoot, XGE_MAP_KEY_VERSION, 1, &iVersion, &bVersionPresent) ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "地图 version 必须是整数");
		return XGE_MAP_ERROR_FORMAT;
	}
	if ( bVersionPresent && (iVersion <= 0 || iVersion > XGE_MAP_FORMAT_VERSION) ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "不支持的地图格式版本: %d", iVersion);
		return XGE_MAP_ERROR_UNSUPPORTED_VERSION;
	}
	tLoaded.iSourceSchemaVersion = bVersionPresent ? iVersion : 1;
	tLoaded.iSchemaVersion = XGE_MAP_FORMAT_VERSION;
	if ( !xgeMapObjectReadText(pRoot, XGE_MAP_KEY_FORMAT, "", sFormat, sizeof(sFormat)) ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "地图 format 必须是字符串");
		return XGE_MAP_ERROR_FORMAT;
	}
	if ( sFormat[0] != 0 && strcmp(sFormat, XGE_MAP_FORMAT_NAME) != 0 ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "文件不是 XGE 地图: %s", sFormat);
		return XGE_MAP_ERROR_FORMAT;
	}
	if ( !xgeMapObjectReadText(pRoot, XGE_MAP_KEY_NAME, "默认地图", tLoaded.sName, sizeof(tLoaded.sName)) ||
	     !xgeMapObjectReadText(pRoot, XGE_MAP_KEY_TILESET, "default.xson", tLoaded.sTileset, sizeof(tLoaded.sTileset)) ||
	     !xgeMapObjectReadText(pRoot, XGE_MAP_KEY_CUSTOM, "", tLoaded.sCustomData, sizeof(tLoaded.sCustomData)) ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "地图文本字段类型无效");
		return XGE_MAP_ERROR_FORMAT;
	}
	if ( !xgeMapObjectReadInt(pRoot, XGE_MAP_KEY_WIDTH, 100, &tLoaded.iWidth, NULL) ||
	     !xgeMapObjectReadInt(pRoot, XGE_MAP_KEY_HEIGHT, 100, &tLoaded.iHeight, NULL) ||
	     !xgeMapObjectReadInt(pRoot, XGE_MAP_KEY_STATE, 0, &tLoaded.iState, NULL) ||
	     !xgeMapObjectReadInt(pRoot, XGE_MAP_KEY_TILE_WIDTH, 16, &tLoaded.iTileWidth, NULL) ||
	     !xgeMapObjectReadInt(pRoot, XGE_MAP_KEY_TILE_HEIGHT, 16, &tLoaded.iTileHeight, NULL) ||
	     tLoaded.iTileWidth <= 0 || tLoaded.iTileHeight <= 0 ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "地图数值字段类型或取值无效");
		return XGE_MAP_ERROR_FORMAT;
	}
	pTiles = xgeMapObjectGet(pRoot, XGE_MAP_KEY_TILES);
	if ( pTiles != NULL && !xrtValueIs(pTiles, XVALUE_ARRAY) ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "图块数据必须是数组");
		return XGE_MAP_ERROR_FORMAT;
	}
	iSerializedTiles = pTiles != NULL ? xrtValueCount(pTiles) : 0u;
	if ( xgeMapValidateSize(tLoaded.iWidth, tLoaded.iHeight, 1, &iCellCount, NULL) != XGE_MAP_OK ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "地图尺寸超出限制: %d x %d", tLoaded.iWidth, tLoaded.iHeight);
		return XGE_MAP_ERROR_LIMIT;
	}
	if ( !xgeMapObjectReadInt(pRoot, XGE_MAP_KEY_LAYER_COUNT,
		xgeMapInferLegacyLayers(iSerializedTiles, iCellCount), &tLoaded.iLayers, &bLayerPresent) ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "地图 layerCount 必须是整数");
		return XGE_MAP_ERROR_FORMAT;
	}
	if ( bVersionPresent && iVersion >= 2 && !bLayerPresent ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "V2 地图缺少 layerCount");
		return XGE_MAP_ERROR_FORMAT;
	}
	if ( !bLayerPresent && iSerializedTiles > 0u && iSerializedTiles % iCellCount != 0u ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "旧版图块数量 %zu 与地图尺寸不一致", iSerializedTiles);
		return XGE_MAP_ERROR_FORMAT;
	}
	iRet = xgeMapAllocate(&tLoaded);
	if ( iRet != XGE_MAP_OK ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "地图尺寸或层数无效: %d x %d x %d", tLoaded.iWidth, tLoaded.iHeight, tLoaded.iLayers);
		return iRet;
	}
	if ( iSerializedTiles > (size_t)tLoaded.iTileCount ) {
		xgeMapUnit(&tLoaded);
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "图块数据数量 %zu 超过声明容量 %d", iSerializedTiles, tLoaded.iTileCount);
		return XGE_MAP_ERROR_FORMAT;
	}
	for ( i = 0u; i < iSerializedTiles; ++i ) {
		int bValid = 0;
		int iTile = xgeMapValueInt(xrtValueArrayGet(pTiles, i), 0, &bValid);
		if ( !bValid ) {
			xgeMapUnit(&tLoaded);
			xrtValueRelease(pRoot);
			xgeMapError(sError, iErrorCapacity, "图块数据第 %zu 项不是整数", i);
			return XGE_MAP_ERROR_FORMAT;
		}
		tLoaded.pTiles[i] = iTile;
	}
	pPassage = xgeMapObjectGet(pRoot, XGE_MAP_KEY_PASSAGE);
	pCellData = xgeMapObjectGet(pRoot, XGE_MAP_KEY_CELL_DATA);
	if ( (pPassage != NULL && !xrtValueIs(pPassage, XVALUE_ARRAY)) ||
	     (pCellData != NULL && !xrtValueIs(pCellData, XVALUE_OBJECT)) ) {
		xgeMapUnit(&tLoaded);
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "地图附加数据类型无效");
		return XGE_MAP_ERROR_FORMAT;
	}
	tLoaded.pPassageRaw = pPassage != NULL ? xrtValueClone(pPassage) : xrtValueArray();
	tLoaded.pCellDataRaw = pCellData != NULL ? xrtValueClone(pCellData) : xrtValueObject();
	if ( tLoaded.pPassageRaw == NULL || tLoaded.pCellDataRaw == NULL ||
	     xgeMapLoadPassageOverrides(&tLoaded, tLoaded.pPassageRaw) != XGE_MAP_OK ) {
		xgeMapUnit(&tLoaded);
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "地图附加数据内存不足");
		return XGE_MAP_ERROR_OUT_OF_MEMORY;
	}
	xgeMapCopyText(tLoaded.sPath, sizeof(tLoaded.sPath), sPath, strlen(sPath));
	tLoaded.bDirty = 0;
	xgeMapUnit(pMap);
	*pMap = tLoaded;
	xrtValueRelease(pRoot);
	return XGE_MAP_OK;
}

static xvalue* xgeMapBuildPassage(const xge_map_t* pMap)
{
	xvalue* pArray = xrtValueArray();
	int i;
	if ( pArray == NULL ) return NULL;
	for ( i = 0; i < pMap->iPassageOverrideCount; ++i ) {
		xvalue* pEntry;
		if ( pMap->arrPassageOverrides[i].iCellId < 0 ||
		    pMap->arrPassageOverrides[i].iCellId >= pMap->iWidth * pMap->iHeight ) continue;
		pEntry = xrtValueObject();
		if ( pEntry == NULL ||
		     !xgeMapObjectSetInt(pEntry, XGE_MAP_KEY_CELL, pMap->arrPassageOverrides[i].iCellId) ||
		     !xgeMapObjectSetInt(pEntry, XGE_MAP_KEY_VALUE, (int)pMap->arrPassageOverrides[i].iValue) ) {
			if ( pEntry != NULL ) xrtValueRelease(pEntry);
			xrtValueRelease(pArray);
			return NULL;
		}
		if ( !xgeMapArrayAppendTake(pArray, pEntry) ) {
			xrtValueRelease(pArray);
			return NULL;
		}
	}
	return pArray;
}

static xvalue* xgeMapBuildCellData(const xge_map_t* pMap)
{
	xvalue* pResult;
	size_t i;
	size_t iCount;
	int iCellCount = pMap->iWidth * pMap->iHeight;
	if ( pMap->pCellDataRaw == NULL || !xrtValueIs(pMap->pCellDataRaw, XVALUE_OBJECT) ) return xrtValueObject();
	pResult = xrtValueObject();
	if ( pResult == NULL ) return NULL;
	iCount = xrtValueCount(pMap->pCellDataRaw);
	for ( i = 0u; i < iCount; ++i ) {
		xstrview tKey = { 0 };
		xvalue* pSource = xrtValueObjectAt(pMap->pCellDataRaw, i, &tKey);
		xvalue* pDest;
		size_t j;
		size_t iEntries;
		if ( pSource == NULL || !xrtValueIs(pSource, XVALUE_ARRAY) || tKey.Data == NULL ) continue;
		pDest = xrtValueArray();
		if ( pDest == NULL ) {
			xrtValueRelease(pResult);
			return NULL;
		}
		iEntries = xrtValueCount(pSource);
		for ( j = 0u; j < iEntries; ++j ) {
			xvalue* pEntry = xrtValueArrayGet(pSource, j);
			int iCell;
			xvalue* pCopy;
			if ( pEntry == NULL || !xrtValueIs(pEntry, XVALUE_OBJECT) ) continue;
			iCell = xgeMapObjectInt(pEntry, XGE_MAP_KEY_CELL, -1, NULL);
			if ( iCell < 0 || iCell >= iCellCount ) continue;
			pCopy = xrtValueClone(pEntry);
			if ( pCopy == NULL ) {
				xrtValueRelease(pDest);
				xrtValueRelease(pResult);
				return NULL;
			}
			if ( !xgeMapArrayAppendTake(pDest, pCopy) ) {
				xrtValueRelease(pDest);
				xrtValueRelease(pResult);
				return NULL;
			}
		}
		if ( xrtValueCount(pDest) == 0u ) {
			xrtValueRelease(pDest);
			continue;
		}
		{
			xvalue* pTake = pDest;
			if ( !xrtValueObjectSetTake(pResult, tKey, &pTake) ) {
				if ( pTake != NULL ) xrtValueRelease(pTake);
				xrtValueRelease(pResult);
				return NULL;
			}
		}
	}
	return pResult;
}

int xgeMapSaveXson(xge_map_t* pMap, const char* sPath, char* sError, size_t iErrorCapacity)
{
	xvalue* pRoot;
	xvalue* pTiles;
	xvalue* pPassage;
	xvalue* pCellData;
	size_t iTileCount = 0u;
	int iRet;
	int i;
	if ( sError != NULL && iErrorCapacity > 0u ) sError[0] = 0;
	if ( pMap == NULL || sPath == NULL || sPath[0] == 0 || pMap->pTiles == NULL ) return XGE_MAP_ERROR_INVALID_ARGUMENT;
	iRet = xgeMapValidateSize(pMap->iWidth, pMap->iHeight, pMap->iLayers, NULL, &iTileCount);
	if ( iRet != XGE_MAP_OK || iTileCount != (size_t)pMap->iTileCount ) {
		xgeMapError(sError, iErrorCapacity, "地图内存尺寸不一致");
		return iRet != XGE_MAP_OK ? iRet : XGE_MAP_ERROR_FORMAT;
	}
	pRoot = xrtValueObject();
	pTiles = xrtValueArray();
	pPassage = xgeMapBuildPassage(pMap);
	pCellData = xgeMapBuildCellData(pMap);
	if ( pRoot == NULL || pTiles == NULL || pPassage == NULL || pCellData == NULL ) {
		if ( pRoot != NULL ) xrtValueRelease(pRoot);
		if ( pTiles != NULL ) xrtValueRelease(pTiles);
		if ( pPassage != NULL ) xrtValueRelease(pPassage);
		if ( pCellData != NULL ) xrtValueRelease(pCellData);
		xgeMapError(sError, iErrorCapacity, "序列化地图时内存不足");
		return XGE_MAP_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < pMap->iTileCount; ++i ) {
		if ( !xgeMapArrayAppendInt(pTiles, pMap->pTiles[i]) ) {
			xrtValueRelease(pRoot);
			xrtValueRelease(pTiles);
			xrtValueRelease(pPassage);
			xrtValueRelease(pCellData);
			xgeMapError(sError, iErrorCapacity, "序列化图块时内存不足");
			return XGE_MAP_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( !xgeMapObjectSetText(pRoot, XGE_MAP_KEY_FORMAT, XGE_MAP_FORMAT_NAME) ||
	     !xgeMapObjectSetInt(pRoot, XGE_MAP_KEY_VERSION, XGE_MAP_FORMAT_VERSION) ||
	     !xgeMapObjectSetInt(pRoot, XGE_MAP_KEY_LAYER_COUNT, pMap->iLayers) ||
	     !xgeMapObjectSetInt(pRoot, XGE_MAP_KEY_TILE_WIDTH, pMap->iTileWidth > 0 ? pMap->iTileWidth : 16) ||
	     !xgeMapObjectSetInt(pRoot, XGE_MAP_KEY_TILE_HEIGHT, pMap->iTileHeight > 0 ? pMap->iTileHeight : 16) ||
	     !xgeMapObjectSetText(pRoot, XGE_MAP_KEY_NAME, pMap->sName) ||
	     !xgeMapObjectSetText(pRoot, XGE_MAP_KEY_TILESET, pMap->sTileset) ||
	     !xgeMapObjectSetInt(pRoot, XGE_MAP_KEY_STATE, pMap->iState) ||
	     !xgeMapObjectSetInt(pRoot, XGE_MAP_KEY_WIDTH, pMap->iWidth) ||
	     !xgeMapObjectSetInt(pRoot, XGE_MAP_KEY_HEIGHT, pMap->iHeight) ||
	     !xgeMapObjectSetText(pRoot, XGE_MAP_KEY_CUSTOM, pMap->sCustomData) ) {
		xrtValueRelease(pTiles);
		xrtValueRelease(pPassage);
		xrtValueRelease(pCellData);
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "创建地图对象失败");
		return XGE_MAP_ERROR_OUT_OF_MEMORY;
	}
	if ( !xgeMapObjectSetTake(pRoot, XGE_MAP_KEY_TILES, pTiles) ) {
		xrtValueRelease(pPassage);
		xrtValueRelease(pCellData);
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "写入图块数据失败");
		return XGE_MAP_ERROR_OUT_OF_MEMORY;
	}
	if ( !xgeMapObjectSetTake(pRoot, XGE_MAP_KEY_PASSAGE, pPassage) ) {
		xrtValueRelease(pCellData);
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "写入通行数据失败");
		return XGE_MAP_ERROR_OUT_OF_MEMORY;
	}
	if ( !xgeMapObjectSetTake(pRoot, XGE_MAP_KEY_CELL_DATA, pCellData) ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "写入单元格数据失败");
		return XGE_MAP_ERROR_OUT_OF_MEMORY;
	}
	if ( !xrtXsonStringifyFile(sPath, pRoot, true) ) {
		xrtValueRelease(pRoot);
		xgeMapError(sError, iErrorCapacity, "无法原子写入地图: %s", sPath);
		return XGE_MAP_ERROR_FILE;
	}
	xrtValueRelease(pRoot);
	xgeMapCopyText(pMap->sPath, sizeof(pMap->sPath), sPath, strlen(sPath));
	pMap->iSchemaVersion = XGE_MAP_FORMAT_VERSION;
	pMap->bDirty = 0;
	return XGE_MAP_OK;
}
