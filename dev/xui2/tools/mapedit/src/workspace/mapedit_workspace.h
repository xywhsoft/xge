#ifndef MAPEDIT_WORKSPACE_H
#define MAPEDIT_WORKSPACE_H

#include "mapedit_app.h"

#define MAPEDIT_REGION_LEFT 0
#define MAPEDIT_REGION_DOCUMENT 1
#define MAPEDIT_REGION_RIGHT 2

typedef struct mapedit_window_desc_t {
	const char* sId;
	const char* sTitle;
	const char* sBody;
	int iRegion;
	int bClosable;
	int bDockable;
} mapedit_window_desc_t;

struct mapedit_workspace_def_t {
	int iType;
	const char* sName;
	const char* sLayoutFile;
	const char* sDefaultLayoutFile;
	const mapedit_window_desc_t* pWindows;
	int iWindowCount;
};

xge_xui_widget MapEditWorkspaceNewContent(mapedit_app_t* pApp);
int MapEditWorkspaceAddLabel(mapedit_app_t* pApp, xge_xui_widget pParent, const char* sText, uint32_t iColor);

int MapEditWorkspacesCreate(mapedit_app_t* pApp, xge_xui_widget pClientStack);
void MapEditWorkspacesUnit(mapedit_app_t* pApp);
void MapEditWorkspacesSelect(mapedit_app_t* pApp, int iWorkspace);
int MapEditWorkspacesLoadUser(mapedit_app_t* pApp);
int MapEditWorkspacesLoadDefault(mapedit_app_t* pApp);
int MapEditWorkspacesSaveUser(mapedit_app_t* pApp);
int MapEditWorkspacesReset(mapedit_app_t* pApp);

#endif
