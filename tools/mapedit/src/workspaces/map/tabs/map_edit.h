#ifndef MAPEDIT_MAP_EDIT_H
#define MAPEDIT_MAP_EDIT_H

#include "mapedit_workspace.h"

int MapEditMapEditCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow);
void MapEditMapEditRefresh(mapedit_app_t* pApp);
void MapEditMapEditClearHistory(mapedit_app_t* pApp);
int MapEditMapEditHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent);
void MapEditMapEditUnit(mapedit_app_t* pApp);

#endif
