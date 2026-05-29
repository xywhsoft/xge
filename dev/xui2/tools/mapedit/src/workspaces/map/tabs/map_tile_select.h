#ifndef MAPEDIT_MAP_TILE_SELECT_H
#define MAPEDIT_MAP_TILE_SELECT_H

#include "mapedit_workspace.h"

int MapEditMapTileSelectCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow);
void MapEditMapTileSelectRefresh(mapedit_app_t* pApp);
int MapEditMapTileSelectHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent);
void MapEditMapTileSelectUnit(mapedit_app_t* pApp);

#endif
