#ifndef UI_DESIGN_APP_H
#define UI_DESIGN_APP_H

#include "xge.h"
#include "xui.h"
#include "ui_design_model.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UI_DESIGN_TARGET_W 1440
#define UI_DESIGN_TARGET_H 900
#define UI_DESIGN_COMPLEX_EDITOR_MAX_ROWS 128
#define UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS 24
#define UI_DESIGN_COMPLEX_EDITOR_COLUMN_CAPACITY 40
#define UI_DESIGN_COMPLEX_EDITOR_CELL_CAPACITY 192
#define UI_DESIGN_HISTORY_LIMIT 64
#define UI_DESIGN_STATUS_CAPACITY 160
#define UI_DESIGN_PATH_CAPACITY 520

typedef enum ui_design_command_t {
	UI_DESIGN_COMMAND_NONE = 0,
	UI_DESIGN_COMMAND_FILE_NEW,
	UI_DESIGN_COMMAND_FILE_OPEN,
	UI_DESIGN_COMMAND_FILE_SAVE,
	UI_DESIGN_COMMAND_FILE_SAVE_AS,
	UI_DESIGN_COMMAND_FILE_EXPORT,
	UI_DESIGN_COMMAND_FILE_EXIT,
	UI_DESIGN_COMMAND_EDIT_UNDO,
	UI_DESIGN_COMMAND_EDIT_REDO,
	UI_DESIGN_COMMAND_EDIT_CUT,
	UI_DESIGN_COMMAND_EDIT_COPY,
	UI_DESIGN_COMMAND_EDIT_PASTE,
	UI_DESIGN_COMMAND_EDIT_DUPLICATE,
	UI_DESIGN_COMMAND_EDIT_DELETE,
	UI_DESIGN_COMMAND_EDIT_SELECT_ALL,
	UI_DESIGN_COMMAND_ARRANGE_ALIGN_LEFT,
	UI_DESIGN_COMMAND_ARRANGE_ALIGN_CENTER,
	UI_DESIGN_COMMAND_ARRANGE_ALIGN_RIGHT,
	UI_DESIGN_COMMAND_ARRANGE_ALIGN_TOP,
	UI_DESIGN_COMMAND_ARRANGE_ALIGN_MIDDLE,
	UI_DESIGN_COMMAND_ARRANGE_ALIGN_BOTTOM,
	UI_DESIGN_COMMAND_ARRANGE_DISTRIBUTE_H,
	UI_DESIGN_COMMAND_ARRANGE_DISTRIBUTE_V,
	UI_DESIGN_COMMAND_ARRANGE_BRING_FORWARD,
	UI_DESIGN_COMMAND_ARRANGE_SEND_BACKWARD,
	UI_DESIGN_COMMAND_VIEW_TOOLBOX,
	UI_DESIGN_COMMAND_VIEW_INSPECTOR,
	UI_DESIGN_COMMAND_VIEW_GRID,
	UI_DESIGN_COMMAND_VIEW_SNAP,
	UI_DESIGN_COMMAND_VIEW_MARQUEE_CONTAIN,
	UI_DESIGN_COMMAND_VIEW_MARQUEE_TOUCH,
	UI_DESIGN_COMMAND_VIEW_ZOOM_IN,
	UI_DESIGN_COMMAND_VIEW_ZOOM_OUT,
	UI_DESIGN_COMMAND_VIEW_ZOOM_FIT,
	UI_DESIGN_COMMAND_VIEW_ZOOM_100,
	UI_DESIGN_COMMAND_TOOL_POINTER,
	UI_DESIGN_COMMAND_TOOL_WIDGET,
	UI_DESIGN_COMMAND_TOOL_LABEL,
	UI_DESIGN_COMMAND_TOOL_BUTTON,
	UI_DESIGN_COMMAND_TOOL_CHECKBOX,
	UI_DESIGN_COMMAND_TOOL_RADIO,
	UI_DESIGN_COMMAND_TOOL_INPUT,
	UI_DESIGN_COMMAND_TOOL_TEXT_EDIT,
	UI_DESIGN_COMMAND_PREVIEW,
	UI_DESIGN_COMMAND_HELP_ABOUT,
	UI_DESIGN_COMMAND_HELP_SHORTCUTS
} ui_design_command_t;

typedef struct ui_design_history_entry_t {
	ui_design_command_t iCommand;
	char sName[48];
	char* sBefore;
	char* sAfter;
} ui_design_history_entry_t;

typedef enum ui_design_property_t {
	UI_DESIGN_PROPERTY_NONE = 0,
	UI_DESIGN_PROPERTY_TEXT,
	UI_DESIGN_PROPERTY_X,
	UI_DESIGN_PROPERTY_Y,
	UI_DESIGN_PROPERTY_W,
	UI_DESIGN_PROPERTY_H
} ui_design_property_t;

typedef struct ui_design_clipboard_node_t {
	int iOriginalId;
	int iOriginalParentId;
	ui_design_node_type_t iType;
	xui_rect_t tRect;
	char sText[UI_DESIGN_TEXT_CAPACITY];
	int bChecked;
	int bVisible;
	int bEnabled;
	ui_design_property_value_t arrProperties[UI_DESIGN_MAX_NODE_PROPERTIES];
	int iPropertyCount;
} ui_design_clipboard_node_t;

typedef struct ui_design_app_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	int iSurfaceWidth;
	int iSurfaceHeight;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pMenuBar;
	xui_widget pFileMenu;
	xui_widget pEditMenu;
	xui_widget pArrangeMenu;
	xui_widget pViewMenu;
	xui_widget pToolsMenu;
	xui_widget pHelpMenu;
	xui_widget pToolbar;
	xui_widget pStatusBar;
	xui_widget pDock;
	xui_widget pToolbox;
	xui_widget pToolboxScrollBar;
	xui_widget pCanvas;
	xui_widget pArtboard;
	xui_widget pOverlay;
	xui_widget pCanvasContextMenu;
	xui_widget pInspector;
	xui_widget pTree;
	xui_widget pTreeContextMenu;
	xui_widget pPropertyGrid;
	xui_widget pComplexEditorWindow;
	xui_widget pComplexEditorText;
	xui_widget pComplexEditorGrid;
	xui_widget pComplexEditorAdd;
	xui_widget pComplexEditorDelete;
	xui_widget pComplexEditorUp;
	xui_widget pComplexEditorDown;
	xui_widget pComplexEditorOk;
	xui_widget pComplexEditorCancel;
	xui_file_dialog pFileDialog;
	int iToolboxWindow;
	int iCanvasWindow;
	int iInspectorWindow;
	int iCanvasPane;
	ui_design_model_t tModel;
	ui_design_history_entry_t arrUndo[UI_DESIGN_HISTORY_LIMIT];
	ui_design_history_entry_t arrRedo[UI_DESIGN_HISTORY_LIMIT];
	int iUndoCount;
	int iRedoCount;
	char* sHistoryTransactionBefore;
	ui_design_command_t iHistoryTransactionCommand;
	char sHistoryTransactionName[48];
	char sDocumentPath[UI_DESIGN_PATH_CAPACITY];
	char sStatus[UI_DESIGN_STATUS_CAPACITY];
	int bDocumentDirty;
	int bGridVisible;
	int bSnapEnabled;
	int bMarqueeSelectContain;
	int bPreviewMode;
	float fZoom;
	int iStatusMessageItem;
	int iStatusSelectionItem;
	int iStatusZoomItem;
	int iPendingFileCommand;
	ui_design_clipboard_node_t* pClipboardNodes;
	int iClipboardNodeCount;
	int iClipboardCapacity;
	int iClipboardRootOriginalId;
	float fClipboardRootX;
	float fClipboardRootY;
	int iContextMenuNodeId;
	int iTreeContextMenuNodeId;
	float fContextMenuDesignX;
	float fContextMenuDesignY;
	int bContextMenuHasDesignPoint;
	ui_design_node_type_t iActiveTool;
	ui_design_node_type_t iDraggingTool;
	int bDraggingTool;
	float fDragX;
	float fDragY;
	uint32_t iToolboxCollapsedMask;
	int bSyncingInspector;
	int bSyncingTree;
	int iCanvasDragMode;
	int iCanvasDragHandle;
	int iCanvasDragNodeId;
	int arrCanvasDragNodeIds[UI_DESIGN_MAX_NODES];
	xui_rect_t arrCanvasDragStartRects[UI_DESIGN_MAX_NODES];
	int iCanvasDragNodeCount;
	float fCanvasDragStartX;
	float fCanvasDragStartY;
	xui_rect_t tCanvasDragStartRect;
	ui_design_property_t iEditingProperty;
	char sEditBuffer[UI_DESIGN_TEXT_CAPACITY];
	int iEditLength;
	int iComplexEditorNodeId;
	char sComplexEditorPropertyId[UI_DESIGN_PROPERTY_ID_CAPACITY];
	int bComplexEditorStructured;
	char cComplexEditorDelimiter;
	int iComplexEditorRowCount;
	int iComplexEditorColumnCount;
	char arrComplexEditorColumnId[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS][UI_DESIGN_COMPLEX_EDITOR_COLUMN_CAPACITY];
	char arrComplexEditorColumnTitle[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS][UI_DESIGN_COMPLEX_EDITOR_COLUMN_CAPACITY];
	int arrComplexEditorColumnType[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	int arrComplexEditorColumnRequired[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	int arrComplexEditorColumnRangeSet[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	float arrComplexEditorColumnMin[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	float arrComplexEditorColumnMax[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	int arrComplexEditorColumnPrecision[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	const xui_combobox_item_t* arrComplexEditorColumnEnumItems[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	int arrComplexEditorColumnEnumCount[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	int arrComplexEditorColumnEnumUseValue[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	char arrComplexEditorCellText[UI_DESIGN_COMPLEX_EDITOR_MAX_ROWS][UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS][UI_DESIGN_COMPLEX_EDITOR_CELL_CAPACITY];
	xui_table_view_column_t arrComplexEditorColumns[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	xui_table_view_row_t arrComplexEditorRows[UI_DESIGN_COMPLEX_EDITOR_MAX_ROWS];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bExercise;
	int bExerciseSeeded;
	int bPreviewRunner;
	int bPreviewDeleteFile;
	char sPreviewPath[UI_DESIGN_PATH_CAPACITY];
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	uint32_t iLastButtons;
	int bCreateOK;
	int bLayoutOK;
	int bModelOK;
	int bPaintOK;
} ui_design_app_t;

void uiDesignAppInvalidate(ui_design_app_t* pApp);
int uiDesignAppSelectNode(ui_design_app_t* pApp, int iId);
int uiDesignAppAddNodeSelection(ui_design_app_t* pApp, int iId);
int uiDesignAppToggleNodeSelection(ui_design_app_t* pApp, int iId);
int uiDesignAppAddNodeAt(ui_design_app_t* pApp, ui_design_node_type_t iType, float fDesignX, float fDesignY, int* pId);
int uiDesignAppCreateNodeWidget(ui_design_app_t* pApp, ui_design_node_t* pNode);
int uiDesignAppSyncNodeWidget(ui_design_app_t* pApp, ui_design_node_t* pNode);
int uiDesignAppSetNodeRect(ui_design_app_t* pApp, int iId, xui_rect_t tRect);
int uiDesignAppSetNodeText(ui_design_app_t* pApp, int iId, const char* sText);
int uiDesignAppSetNodeChecked(ui_design_app_t* pApp, int iId, int bChecked);
int uiDesignAppSetNodeVisible(ui_design_app_t* pApp, int iId, int bVisible);
int uiDesignAppSetNodeEnabled(ui_design_app_t* pApp, int iId, int bEnabled);
int uiDesignAppSetNodeProperty(ui_design_app_t* pApp, int iId, const char* sPropertyId, const char* sValue);
int uiDesignAppCopyNode(ui_design_app_t* pApp, int iId);
int uiDesignAppCopySelection(ui_design_app_t* pApp);
int uiDesignAppCutNode(ui_design_app_t* pApp, int iId);
int uiDesignAppPasteClipboard(ui_design_app_t* pApp, float fDesignX, float fDesignY, int* pNewRootId);
int uiDesignAppPasteClipboardAsChild(ui_design_app_t* pApp, int iParentId, int* pNewRootId);
int uiDesignAppDeleteNode(ui_design_app_t* pApp, int iId);
int uiDesignAppDeleteSelection(ui_design_app_t* pApp);
int uiDesignAppPromoteNode(ui_design_app_t* pApp, int iId);
int uiDesignAppCommandSetNodeRect(ui_design_app_t* pApp, int iId, xui_rect_t tRect, const char* sName);
int uiDesignAppCommandSetNodeText(ui_design_app_t* pApp, int iId, const char* sText);
int uiDesignAppCommandSetNodeChecked(ui_design_app_t* pApp, int iId, int bChecked);
int uiDesignAppCommandSetNodeVisible(ui_design_app_t* pApp, int iId, int bVisible);
int uiDesignAppCommandSetNodeEnabled(ui_design_app_t* pApp, int iId, int bEnabled);
int uiDesignAppCommandSetNodeProperty(ui_design_app_t* pApp, int iId, const char* sPropertyId, const char* sValue);
int uiDesignAppCanExecuteCommand(const ui_design_app_t* pApp, ui_design_command_t iCommand);
int uiDesignAppExecuteCommand(ui_design_app_t* pApp, ui_design_command_t iCommand);
int uiDesignAppBeginHistoryTransaction(ui_design_app_t* pApp, ui_design_command_t iCommand, const char* sName);
int uiDesignAppCommitHistoryTransaction(ui_design_app_t* pApp);
void uiDesignAppCancelHistoryTransaction(ui_design_app_t* pApp);
void uiDesignAppUpdateCommandUI(ui_design_app_t* pApp);
void uiDesignAppSetStatus(ui_design_app_t* pApp, const char* sStatus);

#ifdef __cplusplus
}
#endif

#endif
