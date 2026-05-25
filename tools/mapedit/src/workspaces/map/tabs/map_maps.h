#ifndef MAPEDIT_MAP_MAPS_H
#define MAPEDIT_MAP_MAPS_H

#include "mapedit_workspace.h"

int MapEditMapListCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow);
int MapEditMapPropertiesCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow);
void MapEditMapListUnit(mapedit_app_t* pApp);
void MapEditMapEnsureDefaults(mapedit_app_t* pApp);
void MapEditMapRefreshDependents(mapedit_app_t* pApp);
void MapEditMapPropertiesRefresh(mapedit_app_t* pApp);
mapedit_map_item_t* MapEditMapSelectedItem(mapedit_app_t* pApp);
mapedit_tileset_set_item_t* MapEditMapSelectedTileset(mapedit_app_t* pApp);
int MapEditMapEnsureTileData(mapedit_app_t* pApp);
int MapEditMapSaveSelected(mapedit_app_t* pApp);
int MapEditMapTilesPerRow(mapedit_app_t* pApp);
int MapEditMapTileWidth(mapedit_app_t* pApp);
int MapEditMapTileHeight(mapedit_app_t* pApp);
int MapEditMapSetupLayerCount(mapedit_app_t* pApp);
const char* MapEditMapSetupLayerName(mapedit_app_t* pApp, int iLayer);
int MapEditMapSetupLayerAboveActor(mapedit_app_t* pApp, int iLayer);
int MapEditMapStaticStart(mapedit_app_t* pApp, mapedit_tileset_set_item_t* pTileset);
int MapEditMapStaticCols(mapedit_app_t* pApp);
int MapEditMapStaticRows(mapedit_app_t* pApp);
void MapEditMapLoadTilesetTexture(mapedit_app_t* pApp);
void MapEditMapClearTilesetTexture(mapedit_app_t* pApp);
int MapEditMapCellId(mapedit_map_item_t* pMap, int x, int y);
int MapEditMapTileIndex(mapedit_map_item_t* pMap, int iLayer, int x, int y);
int MapEditMapGetTileAt(mapedit_app_t* pApp, int iLayer, int x, int y);
int MapEditMapTopTileAt(mapedit_app_t* pApp, int x, int y);
void MapEditMapDrawTile(mapedit_app_t* pApp, mapedit_tile_grid_t* pGrid, int iTileId, xge_rect_t tCell);
void MapEditMapDrawCell(mapedit_app_t* pApp, mapedit_tile_grid_t* pGrid, int iCol, int iRow, xge_rect_t tCell);
unsigned char MapEditMapDefaultPassageForCell(mapedit_app_t* pApp, int x, int y);
unsigned char MapEditMapGetCellPassage(mapedit_app_t* pApp, int x, int y);
int MapEditMapSetCellPassage(mapedit_app_t* pApp, int x, int y, unsigned char iValue);
void MapEditMapClearCellCustomData(mapedit_map_item_t* pItem);
int MapEditMapGetCellCustomValue(mapedit_app_t* pApp, const char* sChannelId, int x, int y, char* sValue, int iValueSize);
int MapEditMapSetCellCustomValue(mapedit_app_t* pApp, const char* sChannelId, int x, int y, const char* sValue);
void MapEditMapPropertiesShowCellCustomData(mapedit_app_t* pApp, int x, int y);

#endif
