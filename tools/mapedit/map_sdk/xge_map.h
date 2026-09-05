#ifndef XGE_MAP_H
#define XGE_MAP_H

#include <stddef.h>

typedef struct xvalue xvalue;

#ifdef __cplusplus
extern "C" {
#endif

#define XGE_MAP_FORMAT_NAME "xge.map"
#define XGE_MAP_FORMAT_VERSION 2
#define XGE_MAP_LEGACY_DEFAULT_LAYERS 3
#define XGE_MAP_LAYER_MAX 8
#define XGE_MAP_DIMENSION_MAX 2048
#define XGE_MAP_CELL_MAX 1048576
#define XGE_MAP_PATH_CAPACITY 512
#define XGE_MAP_NAME_CAPACITY 128
#define XGE_MAP_FILE_CAPACITY 256

typedef enum xge_map_result_t {
	XGE_MAP_OK = 0,
	XGE_MAP_ERROR_INVALID_ARGUMENT = -1,
	XGE_MAP_ERROR_OUT_OF_MEMORY = -2,
	XGE_MAP_ERROR_FILE = -3,
	XGE_MAP_ERROR_FORMAT = -4,
	XGE_MAP_ERROR_UNSUPPORTED_VERSION = -5,
	XGE_MAP_ERROR_LIMIT = -6
} xge_map_result_t;

typedef struct xge_map_size_t {
	int iWidth;
	int iHeight;
	int iLayerCount;
	int iTileWidth;
	int iTileHeight;
} xge_map_size_t;

typedef struct xge_map_passage_override_t {
	int iCellId;
	unsigned char iValue;
} xge_map_passage_override_t;

/* Shared editor/runtime map document. Call xgeMapInit before first use. */
typedef struct xge_map_t {
	char sPath[XGE_MAP_PATH_CAPACITY];
	char sName[XGE_MAP_NAME_CAPACITY];
	char sTileset[XGE_MAP_FILE_CAPACITY];
	char sCustomData[XGE_MAP_NAME_CAPACITY];
	int iWidth;
	int iHeight;
	int iLayers;
	int iTileWidth;
	int iTileHeight;
	int iState;
	int* pTiles;
	int iTileCount;
	xvalue* pPassageRaw;
	xvalue* pCellDataRaw;
	xge_map_passage_override_t* arrPassageOverrides;
	int iPassageOverrideCount;
	int iPassageOverrideCapacity;
	int bDirty;
	int iSchemaVersion;
	int iSourceSchemaVersion;
} xge_map_t;

void xgeMapInit(xge_map_t* pMap);
void xgeMapSetDefault(xge_map_t* pMap);
void xgeMapUnit(xge_map_t* pMap);

int xgeMapValidateSize(int iWidth, int iHeight, int iLayerCount,
	size_t* pCellCount, size_t* pTileCount);
int xgeMapAllocate(xge_map_t* pMap);
int xgeMapLoadXson(xge_map_t* pMap, const char* sPath, char* sError, size_t iErrorCapacity);
int xgeMapSaveXson(xge_map_t* pMap, const char* sPath, char* sError, size_t iErrorCapacity);

int xgeMapCellId(const xge_map_t* pMap, int iX, int iY);
int xgeMapFindPassageOverride(const xge_map_t* pMap, int iCellId);
int xgeMapSetPassageOverride(xge_map_t* pMap, int iCellId, unsigned char iValue);
int xgeMapRemovePassageOverride(xge_map_t* pMap, int iIndex);

#ifdef __cplusplus
}
#endif

#endif
