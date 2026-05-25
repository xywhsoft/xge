#ifndef MAPEDIT_TILESET_ACTOR_OVERLAY_H
#define MAPEDIT_TILESET_ACTOR_OVERLAY_H

#include "mapedit_workspace.h"

int MapEditTilesetActorOverlayCreate(mapedit_app_t* pApp, xge_xui_dock_window pWindow);
void MapEditTilesetActorOverlayRefresh(mapedit_app_t* pApp);
int MapEditTilesetActorOverlayHandleEvent(mapedit_app_t* pApp, const xge_event_t* pEvent);
void MapEditTilesetActorOverlayUnit(mapedit_app_t* pApp);

#endif
