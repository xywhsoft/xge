#ifndef MAPEDIT_TILESET_PASSAGE_H
#define MAPEDIT_TILESET_PASSAGE_H

#include "mapedit_workspace.h"

int MapEditTilesetPassageCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow);
void MapEditTilesetPassageRefresh(mapedit_app_t* pApp);
int MapEditTilesetPassageHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent);
void MapEditTilesetPassageUnit(mapedit_app_t* pApp);

#endif
