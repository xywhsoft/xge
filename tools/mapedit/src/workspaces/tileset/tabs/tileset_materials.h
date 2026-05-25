#ifndef MAPEDIT_TILESET_MATERIALS_H
#define MAPEDIT_TILESET_MATERIALS_H

#include "mapedit_workspace.h"

void MapEditTilesetMaterialsInitDefaults(mapedit_app_t* pApp);
void MapEditTilesetMaterialsUpdateInfo(mapedit_app_t* pApp);
int MapEditTilesetMaterialsCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow);
int MapEditTilesetMaterialsHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent);

#endif
