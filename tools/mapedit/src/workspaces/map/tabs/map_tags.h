#ifndef MAPEDIT_MAP_TAGS_H
#define MAPEDIT_MAP_TAGS_H

#include "mapedit_workspace.h"

int MapEditMapTagsCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow);
void MapEditMapTagsRefresh(mapedit_app_t* pApp);
int MapEditMapTagsHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent);
void MapEditMapTagsUnit(mapedit_app_t* pApp);

#endif
