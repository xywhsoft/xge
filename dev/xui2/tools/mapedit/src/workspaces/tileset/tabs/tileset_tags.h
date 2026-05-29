#ifndef MAPEDIT_TILESET_TAGS_H
#define MAPEDIT_TILESET_TAGS_H

#include "mapedit_workspace.h"

int MapEditTilesetTagsCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow);
void MapEditTilesetTagsRefresh(mapedit_app_t* pApp);
int MapEditTilesetTagsHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent);
void MapEditTilesetTagsUnit(mapedit_app_t* pApp);

#endif
