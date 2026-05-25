#ifndef UID_APP_H
#define UID_APP_H

#include "xge.h"
#include "uid_document.h"
#include "uid_project.h"
#include "uid_registry.h"
#include "uid_undo.h"
#include "uid_refresh.h"

#define UID_SHELL_LABEL_CAPACITY 128
#define UID_DOCK_WINDOW_COUNT 8

enum {
	UID_DOCK_DOCUMENT = 0,
	UID_DOCK_TOOLBOX,
	UID_DOCK_STRUCTURE,
	UID_DOCK_PROJECT,
	UID_DOCK_PROPERTIES,
	UID_DOCK_ACTIONS,
	UID_DOCK_OUTPUT,
	UID_DOCK_RESOURCES
};

typedef struct uid_label_slot_t {
	xge_xui_widget pWidget;
	xge_xui_label_t tLabel;
	int bReady;
} uid_label_slot_t;

typedef struct uid_app_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bContinuousRefresh;

	uid_document_t tDocument;
	uid_project_t tProject;
	uid_control_registry_t tRegistry;
	uid_undo_stack_t tUndo;
	uid_refresh_state_t tRefresh;

	xge_xui_widget pRoot;
	xge_xui_widget pMenuBar;
	xge_xui_widget pToolbar;
	xge_xui_widget pDockHost;
	xge_xui_widget pShellStatusBar;
	xge_xui_dock_layout_t tDockLayout;
	xge_xui_dock_window_t arrDockWindow[UID_DOCK_WINDOW_COUNT];
	xge_xui_dock_pane pDocumentPane;
	xge_xui_dock_pane pToolPane;
	xge_xui_dock_pane pProjectPane;
	xge_xui_dock_pane pOutputPane;
	int bDockLayoutReady;
	int iDockWindowReadyCount;
	xge_xui_widget pToolbox;
	xge_xui_widget pStructure;
	xge_xui_widget pCenterPane;
	xge_xui_widget pDesignerTabs;
	xge_xui_widget pCanvas;
	xge_xui_widget pProjectExplorer;
	xge_xui_widget pInspector;
	xge_xui_widget pActions;
	xge_xui_widget pOutput;
	xge_xui_widget pResources;

	uid_label_slot_t arrLabels[UID_SHELL_LABEL_CAPACITY];
	int iLabelCount;
} uid_app_t;

int UIDesignAppInit(uid_app_t* pApp);
void UIDesignAppUnit(uid_app_t* pApp);
int UIDesignAppEnter(xge_scene pScene);
int UIDesignAppLeave(xge_scene pScene);
int UIDesignAppEvent(xge_scene pScene, const xge_event_t* pEvent);
int UIDesignAppUpdate(xge_scene pScene, float fDelta);
int UIDesignAppDraw(xge_scene pScene);

int UIDesignShellBuild(uid_app_t* pApp);
void UIDesignShellUnit(uid_app_t* pApp);

#endif
