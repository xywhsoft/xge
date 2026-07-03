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
#define UID_TOOLBOX_BUTTON_CAPACITY 16
#define UID_PREVIEW_CONTROL_CAPACITY 160

enum {
	UID_CANVAS_DRAG_NONE = 0,
	UID_CANVAS_DRAG_MOVE,
	UID_CANVAS_DRAG_RESIZE
};

enum {
	UID_CANVAS_HANDLE_NW = 0,
	UID_CANVAS_HANDLE_N,
	UID_CANVAS_HANDLE_NE,
	UID_CANVAS_HANDLE_E,
	UID_CANVAS_HANDLE_SE,
	UID_CANVAS_HANDLE_S,
	UID_CANVAS_HANDLE_SW,
	UID_CANVAS_HANDLE_W
};

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

enum {
	UID_CMD_NEW_PROJECT = 100,
	UID_CMD_SAVE_PAGE,
	UID_CMD_ADD_PAGE,
	UID_CMD_REFRESH_PROJECT,
	UID_CMD_EXIT,
	UID_CMD_ADD_LABEL,
	UID_CMD_ADD_BUTTON,
	UID_CMD_ADD_INPUT,
	UID_CMD_ADD_PANEL,
	UID_CMD_PREVIEW,
	UID_CMD_RESET_LAYOUT,
	UID_CMD_ABOUT
};

typedef struct uid_label_slot_t {
	xge_xui_widget pWidget;
	xge_xui_label_t tLabel;
	int bReady;
} uid_label_slot_t;

typedef struct uid_toolbox_button_t {
	xge_xui_widget pWidget;
	xge_xui_button_t tButton;
	struct uid_app_t* pApp;
	const char* sType;
	int bReady;
} uid_toolbox_button_t;

typedef struct uid_preview_control_t {
	xge_xui_widget pWidget;
	xge_xui_label_t tLabel;
	xge_xui_button_t tButton;
	xge_xui_input_t tInput;
	int iKind;
	int bReady;
} uid_preview_control_t;

typedef struct uid_app_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bContinuousRefresh;
	int bMvpSmoke;
	int bCanvasSmoke;
	int bToolboxDragSmoke;
	int bProjectSmoke;

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
	xge_xui_widget pFormSurface;
	xge_xui_widget pProjectExplorer;
	xge_xui_widget pProjectToolbar;
	xge_xui_widget pProjectTreeWidget;
	xge_xui_widget pInspector;
	xge_xui_widget pInspectorGridWidget;
	xge_xui_widget pActions;
	xge_xui_widget pOutput;
	xge_xui_widget pResources;
	xge_xui_property_grid_t tInspectorGrid;
	xge_xui_menubar_t tMenuBar;
	xge_xui_menu_t tFileMenu;
	xge_xui_menu_t tProjectMenu;
	xge_xui_menu_t tViewMenu;
	xge_xui_menu_t tHelpMenu;
	xge_xui_toolbar_t tToolbar;
	xge_xui_status_bar_t tStatusBar;
	xge_xui_tree_view_t tProjectTree;
	int bInspectorGridReady;
	int bMenuReady;
	int bToolbarReady;
	int bStatusBarReady;
	int bProjectTreeReady;
	int bInspectorRefreshing;
	int iStatusMain;
	int iStatusProject;
	int iStatusSelection;

	uid_label_slot_t arrLabels[UID_SHELL_LABEL_CAPACITY];
	int iLabelCount;
	uid_toolbox_button_t arrToolboxButtons[UID_TOOLBOX_BUTTON_CAPACITY];
	int iToolboxButtonCount;
	uid_preview_control_t arrPreviewControls[UID_PREVIEW_CONTROL_CAPACITY];
	int iPreviewControlCount;
	int iCanvasDragMode;
	int iCanvasResizeHandle;
	int iCanvasDragNode;
	int bCanvasDragDirty;
	float fCanvasDragStartX;
	float fCanvasDragStartY;
	xge_rect_t tCanvasDragStartRect;
	int bToolboxDrag;
	int bToolboxDragMoved;
	int bToolboxDragOverCanvas;
	const char* sToolboxDragType;
	float fToolboxDragStartX;
	float fToolboxDragStartY;
	float fToolboxDragX;
	float fToolboxDragY;
	xge_rect_t tToolboxDragPreview;
} uid_app_t;

int UIDesignAppInit(uid_app_t* pApp);
void UIDesignAppUnit(uid_app_t* pApp);
int UIDesignAppEnter(xge_scene pScene);
int UIDesignAppLeave(xge_scene pScene);
int UIDesignAppEvent(xge_scene pScene, const xge_event_t* pEvent);
int UIDesignAppUpdate(xge_scene pScene, float fDelta);
int UIDesignAppDraw(xge_scene pScene);
int UIDesignAppAddControl(uid_app_t* pApp, const char* sType);
int UIDesignAppAddControlAt(uid_app_t* pApp, const char* sType, float fX, float fY);

int UIDesignShellBuild(uid_app_t* pApp);
void UIDesignShellUnit(uid_app_t* pApp);
void UIDesignInspectorRefresh(uid_app_t* pApp);
void UIDesignStatusSet(uid_app_t* pApp, const char* sText);
void UIDesignStatusRefresh(uid_app_t* pApp);
int UIDesignProjectRefresh(uid_app_t* pApp);
int UIDesignCommandExecute(uid_app_t* pApp, int iCommand);
int UIDesignCanvasRebuild(uid_app_t* pApp);
int UIDesignCanvasHandleEvent(uid_app_t* pApp, const xge_event_t* pEvent);
void UIDesignCanvasPaintDragOverlay(uid_app_t* pApp);
int UIDesignCanvasSmoke(uid_app_t* pApp);
int UIDesignToolboxDragSmoke(uid_app_t* pApp);
int UIDesignProjectSmoke(uid_app_t* pApp);

#endif
