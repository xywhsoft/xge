#ifndef MAPEDIT_TILESET_SETS_H
#define MAPEDIT_TILESET_SETS_H

#include "mapedit_workspace.h"

int MapEditTilesetSetsCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow);
int MapEditTilesetPropertiesCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow);
void MapEditTilesetSetsUnit(mapedit_app_t* pApp);
void MapEditTilesetPropertiesRefresh(mapedit_app_t* pApp);
void MapEditTilesetPropertiesShowTileset(mapedit_app_t* pApp);
void MapEditTilesetPropertiesShowSpecialTile(mapedit_app_t* pApp, int iTileId);
void MapEditTilesetPropertiesShowTileCustomData(mapedit_app_t* pApp, int iTileId);
mapedit_tileset_set_item_t* MapEditTilesetSetSelectedItem(mapedit_app_t* pApp);
int MapEditTilesetSetSaveSelected(mapedit_app_t* pApp);
void MapEditTilesetSetEnsurePassageCount(mapedit_tileset_set_item_t* pItem, int iCount);
void MapEditTilesetSetEnsureActorOverlayCount(mapedit_tileset_set_item_t* pItem, int iCount);
void MapEditTilesetSetClearCustomData(mapedit_tileset_set_item_t* pItem);
int MapEditTilesetSetGetTileCustomValue(mapedit_tileset_set_item_t* pItem, const char* sChannelId, int iTileId, char* sValue, int iValueSize);
int MapEditTilesetSetSetTileCustomValue(mapedit_tileset_set_item_t* pItem, const char* sChannelId, int iTileId, const char* sValue);
const mapedit_custom_channel_def_t* MapEditCustomChannelFind(mapedit_app_t* pApp, const char* sChannelId);
const mapedit_custom_channel_def_t* MapEditCustomChannelGet(mapedit_app_t* pApp, int iChannel);

#endif
