#ifndef UI_DESIGN_APP_H
#define UI_DESIGN_APP_H

#include "xge.h"
#include "xui.h"
#include "ui_design_model.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UI_DESIGN_TARGET_W 1280
#define UI_DESIGN_TARGET_H 760
#define UI_DESIGN_COMPLEX_EDITOR_MAX_ROWS 128
#define UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS 24
#define UI_DESIGN_COMPLEX_EDITOR_COLUMN_CAPACITY 40
#define UI_DESIGN_COMPLEX_EDITOR_CELL_CAPACITY 192

typedef enum ui_design_property_t {
	UI_DESIGN_PROPERTY_NONE = 0,
	UI_DESIGN_PROPERTY_TEXT,
	UI_DESIGN_PROPERTY_X,
	UI_DESIGN_PROPERTY_Y,
	UI_DESIGN_PROPERTY_W,
	UI_DESIGN_PROPERTY_H
} ui_design_property_t;

typedef struct ui_design_app_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pDock;
	xui_widget pToolbox;
	xui_widget pCanvas;
	xui_widget pArtboard;
	xui_widget pOverlay;
	xui_widget pInspector;
	xui_widget pTree;
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
	int iToolboxWindow;
	int iCanvasWindow;
	int iInspectorWindow;
	int iCanvasPane;
	ui_design_model_t tModel;
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
int uiDesignAppAddNodeAt(ui_design_app_t* pApp, ui_design_node_type_t iType, float fDesignX, float fDesignY, int* pId);
int uiDesignAppCreateNodeWidget(ui_design_app_t* pApp, ui_design_node_t* pNode);
int uiDesignAppSyncNodeWidget(ui_design_app_t* pApp, ui_design_node_t* pNode);
int uiDesignAppSetNodeRect(ui_design_app_t* pApp, int iId, xui_rect_t tRect);
int uiDesignAppSetNodeText(ui_design_app_t* pApp, int iId, const char* sText);
int uiDesignAppSetNodeChecked(ui_design_app_t* pApp, int iId, int bChecked);
int uiDesignAppSetNodeVisible(ui_design_app_t* pApp, int iId, int bVisible);
int uiDesignAppSetNodeEnabled(ui_design_app_t* pApp, int iId, int bEnabled);
int uiDesignAppSetNodeProperty(ui_design_app_t* pApp, int iId, const char* sPropertyId, const char* sValue);

#ifdef __cplusplus
}
#endif

#endif
