#ifndef MAPEDIT_MAP_PASSAGE_H
#define MAPEDIT_MAP_PASSAGE_H

#include "mapedit_workspace.h"

int MapEditMapPassageCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow);
void MapEditMapPassageRefresh(mapedit_app_t* pApp);
int MapEditMapPassageHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent);
void MapEditMapPassageUnit(mapedit_app_t* pApp);

#endif
