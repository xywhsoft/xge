#ifndef MAPEDIT_TILESET_ARRANGE_H
#define MAPEDIT_TILESET_ARRANGE_H

#include "mapedit_workspace.h"

int MapEditTilesetArrangeCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow);
void MapEditTilesetArrangeRefresh(mapedit_app_t* pApp);
void MapEditTilesetArrangeReloadSpecialTextures(mapedit_app_t* pApp);
int MapEditTilesetArrangeHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent);
void MapEditTilesetArrangeUnit(mapedit_app_t* pApp);

#endif
