#ifndef MAPEDIT_APP_H
#define MAPEDIT_APP_H

#include "xge.h"
#include "map_sdk/xge_map.h"

#define MAPEDIT_WORKSPACE_COUNT 2
#define MAPEDIT_WORKSPACE_DOCK_WINDOW_MAX 8
#define MAPEDIT_TILESET_DOCK_WINDOW_COUNT 6
#define MAPEDIT_MAP_DOCK_WINDOW_COUNT 7
#define MAPEDIT_CONTENT_COUNT 16
#define MAPEDIT_LABEL_COUNT 64

#define MAPEDIT_WORKSPACE_TILESET 0
#define MAPEDIT_WORKSPACE_MAP 1

typedef struct mapedit_workspace_def_t mapedit_workspace_def_t;

enum {
	MAPEDIT_CMD_OPEN_MAP = 100,
	MAPEDIT_CMD_SAVE_MAP,
	MAPEDIT_CMD_SAVE_MAP_AS,
	MAPEDIT_CMD_EXIT,
	MAPEDIT_CMD_RESET_LAYOUT,
	MAPEDIT_CMD_ABOUT
};

typedef struct mapedit_workspace_t {
	int iType;
	const char* sName;
	const char* sLayoutFile;
	const char* sDefaultLayoutFile;
	const mapedit_workspace_def_t* pDef;
	xge_xui_widget pPageWidget;
	xge_xui_widget pDockWidget;
	xge_xui_dock_layout_t tDockLayout;
	xge_xui_dock_window_t arrDockWindow[MAPEDIT_WORKSPACE_DOCK_WINDOW_MAX];
	int iWindowCount;
	int bDockReady;
	int bLayoutLoaded;
	int bLayoutSaved;
} mapedit_workspace_t;

typedef struct mapedit_app_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_map_t tMap;
	xge_xui_widget pMenuWidget;
	xge_xui_widget pWorkspaceWidget;
	xge_xui_widget pTilesetEditorButtonWidget;
	xge_xui_widget pMapEditorButtonWidget;
	xge_xui_widget pClientWidget;
	xge_xui_widget pStatusWidget;
	xge_xui_menubar_t tMenuBar;
	xge_xui_button_t tTilesetEditorButton;
	xge_xui_button_t tMapEditorButton;
	xge_xui_menu_t tFileMenu;
	xge_xui_menu_t tViewMenu;
	xge_xui_menu_t tHelpMenu;
	xge_xui_status_bar_t tStatusBar;
	mapedit_workspace_t arrWorkspace[MAPEDIT_WORKSPACE_COUNT];
	xge_xui_widget arrContent[MAPEDIT_CONTENT_COUNT];
	xge_xui_label_t arrLabel[MAPEDIT_LABEL_COUNT];
	int iContentCount;
	int iLabelCount;
	int bFontReady;
	int bDockReady;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bLayoutLoaded;
	int bLayoutSaved;
	int bSetupLoaded;
	int iWorkspace;
	xui_texture pTilesetEditorIcon;
	xui_texture pMapEditorIcon;
	char sStatus[256];
} mapedit_app_t;

int MapEditAppInit(mapedit_app_t* pApp);
void MapEditAppUnit(mapedit_app_t* pApp);
int MapEditAppEnter(xge_scene pScene);
int MapEditAppLeave(xge_scene pScene);
int MapEditAppEvent(xge_scene pScene, const xge_event_t* pEvent);
int MapEditAppUpdate(xge_scene pScene, float fDelta);
int MapEditAppDraw(xge_scene pScene);
void MapEditAppSetStatus(mapedit_app_t* pApp, const char* sText);
void MapEditAppMenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser);
void MapEditAppWorkspaceClick(xge_xui_widget pWidget, void* pUser);

#endif
