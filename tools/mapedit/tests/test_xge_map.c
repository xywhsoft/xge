#include "map_sdk/xge_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if ( !(expr) ) { \
	fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
	return 0; \
} } while ( 0 )

static int write_text_file(const char* sPath, const char* sText)
{
	FILE* pFile = fopen(sPath, "wb");
	size_t iSize = strlen(sText);
	if ( pFile == NULL ) return 0;
	if ( fwrite(sText, 1u, iSize, pFile) != iSize ) {
		(void)fclose(pFile);
		return 0;
	}
	return fclose(pFile) == 0;
}

static int write_legacy_map(const char* sPath, int iLayers, int iWidth, int iHeight, int iVersion)
{
	FILE* pFile = fopen(sPath, "wb");
	int i;
	int iCount = iLayers * iWidth * iHeight;
	if ( pFile == NULL ) return 0;
	fputs("{\n", pFile);
	if ( iVersion > 0 ) fprintf(pFile, "  \"version\": %d,\n", iVersion);
	fprintf(pFile,
		"  \"地图名称\": \"legacy\",\n"
		"  \"图集\": \"default.xson\",\n"
		"  \"地图宽度\": %d,\n"
		"  \"地图高度\": %d,\n"
		"  \"当前状态\": 0,\n"
		"  \"图块数据\": [", iWidth, iHeight);
	for ( i = 0; i < iCount; ++i ) fprintf(pFile, "%s%d", i == 0 ? "" : ",", i + 1);
	fputs("],\n  \"通行修正\": [],\n  \"cellCustomData\": {}\n}\n", pFile);
	return fclose(pFile) == 0;
}

static int test_size_validation(void)
{
	size_t cells = 0u;
	size_t tiles = 0u;
	CHECK(xgeMapValidateSize(100, 100, 3, &cells, &tiles) == XGE_MAP_OK);
	CHECK(cells == 10000u && tiles == 30000u);
	CHECK(xgeMapValidateSize(0, 100, 3, NULL, NULL) != XGE_MAP_OK);
	CHECK(xgeMapValidateSize(XGE_MAP_DIMENSION_MAX + 1, 1, 1, NULL, NULL) == XGE_MAP_ERROR_LIMIT);
	CHECK(xgeMapValidateSize(2048, 2048, 8, NULL, NULL) == XGE_MAP_ERROR_LIMIT);
	CHECK(xgeMapValidateSize(10, 10, XGE_MAP_LAYER_MAX + 1, NULL, NULL) == XGE_MAP_ERROR_LIMIT);
	return 1;
}

static int test_v2_round_trip(int iLayers)
{
	char path[64];
	char error[256];
	xge_map_t source;
	xge_map_t loaded;
	int i;
	snprintf(path, sizeof(path), "map_sdk_test_v2_%d.xson", iLayers);
	xgeMapInit(&source);
	xgeMapInit(&loaded);
	source.iWidth = 4;
	source.iHeight = 3;
	source.iLayers = iLayers;
	source.iTileWidth = 16;
	source.iTileHeight = 16;
	CHECK(xgeMapAllocate(&source) == XGE_MAP_OK);
	for ( i = 0; i < source.iTileCount; ++i ) source.pTiles[i] = i + 17;
	CHECK(xgeMapSetPassageOverride(&source, 5, 42) == XGE_MAP_OK);
	CHECK(xgeMapSaveXson(&source, path, error, sizeof(error)) == XGE_MAP_OK);
	CHECK(xgeMapLoadXson(&loaded, path, error, sizeof(error)) == XGE_MAP_OK);
	CHECK(loaded.iSchemaVersion == XGE_MAP_FORMAT_VERSION);
	CHECK(loaded.iSourceSchemaVersion == XGE_MAP_FORMAT_VERSION);
	CHECK(loaded.iWidth == 4 && loaded.iHeight == 3 && loaded.iLayers == iLayers);
	CHECK(loaded.iTileCount == source.iTileCount);
	for ( i = 0; i < loaded.iTileCount; ++i ) CHECK(loaded.pTiles[i] == source.pTiles[i]);
	CHECK(loaded.iPassageOverrideCount == 1);
	CHECK(loaded.arrPassageOverrides[0].iCellId == 5);
	CHECK(loaded.arrPassageOverrides[0].iValue == 42);
	xgeMapUnit(&source);
	xgeMapUnit(&loaded);
	(void)remove(path);
	return 1;
}

static int test_v1_layer_inference(int iLayers)
{
	char path[64];
	char error[256];
	xge_map_t map;
	snprintf(path, sizeof(path), "map_sdk_test_v1_%d.xson", iLayers);
	CHECK(write_legacy_map(path, iLayers, 4, 3, 0));
	xgeMapInit(&map);
	CHECK(xgeMapLoadXson(&map, path, error, sizeof(error)) == XGE_MAP_OK);
	CHECK(map.iSourceSchemaVersion == 1);
	CHECK(map.iLayers == iLayers);
	CHECK(map.iTileCount == 4 * 3 * iLayers);
	CHECK(map.pTiles[map.iTileCount - 1] == map.iTileCount);
	xgeMapUnit(&map);
	(void)remove(path);
	return 1;
}

static int test_invalid_file_preserves_document(void)
{
	const char* path = "map_sdk_test_invalid.xson";
	char error[256];
	xge_map_t map;
	CHECK(write_legacy_map(path, 1, XGE_MAP_DIMENSION_MAX + 1, 1, 0));
	xgeMapInit(&map);
	map.iWidth = 7;
	map.iHeight = 9;
	CHECK(xgeMapLoadXson(&map, path, error, sizeof(error)) == XGE_MAP_ERROR_LIMIT);
	CHECK(map.iWidth == 7 && map.iHeight == 9);
	xgeMapUnit(&map);
	(void)remove(path);
	return 1;
}

static int test_future_version_rejected(void)
{
	const char* path = "map_sdk_test_future.xson";
	char error[256];
	xge_map_t map;
	CHECK(write_legacy_map(path, 1, 1, 1, XGE_MAP_FORMAT_VERSION + 1));
	xgeMapInit(&map);
	CHECK(xgeMapLoadXson(&map, path, error, sizeof(error)) == XGE_MAP_ERROR_UNSUPPORTED_VERSION);
	xgeMapUnit(&map);
	(void)remove(path);
	return 1;
}

static int expect_load_failure_preserves(const char* sPath, const char* sText, int iExpected)
{
	char error[256];
	xge_map_t map;
	CHECK(write_text_file(sPath, sText));
	xgeMapInit(&map);
	map.iWidth = 7;
	map.iHeight = 9;
	CHECK(xgeMapLoadXson(&map, sPath, error, sizeof(error)) == iExpected);
	CHECK(map.iWidth == 7 && map.iHeight == 9 && map.pTiles == NULL);
	xgeMapUnit(&map);
	(void)remove(sPath);
	return 1;
}

static int test_malformed_schema_rejected(void)
{
	CHECK(expect_load_failure_preserves("map_sdk_test_bad_version.xson",
		"{\"version\":\"2\",\"地图宽度\":1,\"地图高度\":1,\"图块数据\":[]}",
		XGE_MAP_ERROR_FORMAT));
	CHECK(expect_load_failure_preserves("map_sdk_test_missing_layers.xson",
		"{\"format\":\"xge.map\",\"version\":2,\"地图宽度\":1,\"地图高度\":1,\"图块数据\":[]}",
		XGE_MAP_ERROR_FORMAT));
	CHECK(expect_load_failure_preserves("map_sdk_test_bad_extra.xson",
		"{\"format\":\"xge.map\",\"version\":2,\"layerCount\":1,\"地图宽度\":1,\"地图高度\":1,\"图块数据\":[],\"通行修正\":{}}",
		XGE_MAP_ERROR_FORMAT));
	CHECK(expect_load_failure_preserves("map_sdk_test_bad_legacy_count.xson",
		"{\"地图宽度\":3,\"地图高度\":1,\"图块数据\":[1,2]}",
		XGE_MAP_ERROR_FORMAT));
	return 1;
}

int main(void)
{
	CHECK(test_size_validation());
	CHECK(test_v2_round_trip(1));
	CHECK(test_v2_round_trip(3));
	CHECK(test_v2_round_trip(8));
	CHECK(test_v1_layer_inference(1));
	CHECK(test_v1_layer_inference(3));
	CHECK(test_v1_layer_inference(8));
	CHECK(test_invalid_file_preserves_document());
	CHECK(test_future_version_rejected());
	CHECK(test_malformed_schema_rejected());
	puts("xge_map tests: OK");
	return 0;
}
