#ifndef UI_DESIGN_MODEL_H
#define UI_DESIGN_MODEL_H

#include "xui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UI_DESIGN_MAX_NODES 512
#define UI_DESIGN_TEXT_CAPACITY 96
#define UI_DESIGN_FLAT_CAPACITY UI_DESIGN_MAX_NODES
#define UI_DESIGN_MAX_NODE_PROPERTIES 128
#define UI_DESIGN_PROPERTY_ID_CAPACITY 48
#define UI_DESIGN_PROPERTY_VALUE_CAPACITY 1024
#define UI_DESIGN_RUNTIME_TEXT_COUNT 1100
#define UI_DESIGN_RUNTIME_TEXT_CAPACITY 128
#define UI_DESIGN_RUNTIME_TABLE_ROWS 64
#define UI_DESIGN_RUNTIME_TABLE_COLUMNS 16
#define UI_DESIGN_RUNTIME_EDITOR_OPTIONS 32
#define UI_DESIGN_RUNTIME_EDITOR_OPTION_CAPACITY 64
#define UI_DESIGN_RUNTIME_EDITOR_PALETTE 32
#define UI_DESIGN_RUNTIME_SURFACE_COUNT XUI_TABS_PAGE_CAPACITY

typedef enum ui_design_node_type_t {
	UI_DESIGN_NODE_NONE = 0,
	UI_DESIGN_NODE_WIDGET,
	UI_DESIGN_NODE_LABEL,
	UI_DESIGN_NODE_HYPERLINK,
	UI_DESIGN_NODE_BUTTON,
	UI_DESIGN_NODE_CHECKBOX,
	UI_DESIGN_NODE_RADIO,
	UI_DESIGN_NODE_TOGGLE,
	UI_DESIGN_NODE_INPUT,
	UI_DESIGN_NODE_TAG_INPUT,
	UI_DESIGN_NODE_NUMERIC_INPUT,
	UI_DESIGN_NODE_TEXT_EDIT,
	UI_DESIGN_NODE_PANEL,
	UI_DESIGN_NODE_SEPARATOR,
	UI_DESIGN_NODE_PROGRESS,
	UI_DESIGN_NODE_STEP_BAR,
	UI_DESIGN_NODE_CHART,
	UI_DESIGN_NODE_SCROLLBAR,
	UI_DESIGN_NODE_SLIDER,
	UI_DESIGN_NODE_RANGE_SLIDER,
	UI_DESIGN_NODE_PAGE,
	UI_DESIGN_NODE_CAROUSEL,
	UI_DESIGN_NODE_COMBOBOX,
	UI_DESIGN_NODE_LISTVIEW,
	UI_DESIGN_NODE_TREEVIEW,
	UI_DESIGN_NODE_TABLEVIEW,
	UI_DESIGN_NODE_TABLEGRID,
	UI_DESIGN_NODE_QRCODE,
	UI_DESIGN_NODE_IMAGE,
	UI_DESIGN_NODE_BREADCRUMB,
	UI_DESIGN_NODE_CHECK_CARD,
	UI_DESIGN_NODE_RADIO_GROUP,
	UI_DESIGN_NODE_VIRTUAL_JOYSTICK,
	UI_DESIGN_NODE_INVENTORY_GRID,
	UI_DESIGN_NODE_TERMINAL,
	UI_DESIGN_NODE_SPLIT_LAYOUT,
	UI_DESIGN_NODE_TABS,
	UI_DESIGN_NODE_ACCORDION,
	UI_DESIGN_NODE_WINDOW,
	UI_DESIGN_NODE_SCROLL_FRAME,
	UI_DESIGN_NODE_SCROLL_VIEW,
	UI_DESIGN_NODE_CANVAS,
	UI_DESIGN_NODE_MESSAGE_LIST,
	UI_DESIGN_NODE_TIMELINE_VIEW,
	UI_DESIGN_NODE_PROPERTY_GRID,
	UI_DESIGN_NODE_MENU_BAR,
	UI_DESIGN_NODE_TOOLBAR,
	UI_DESIGN_NODE_STATUS_BAR,
	UI_DESIGN_NODE_DOCK_PANEL,
	UI_DESIGN_NODE_POPUP,
	UI_DESIGN_NODE_MENU,
	UI_DESIGN_NODE_MSG_BOX,
	UI_DESIGN_NODE_FILE_DIALOG,
	UI_DESIGN_NODE_MSG_TIP,
	UI_DESIGN_NODE_TOAST,
	UI_DESIGN_NODE_CASCADER,
	UI_DESIGN_NODE_COLOR_PICKER,
	UI_DESIGN_NODE_DATE_PICKER,
	UI_DESIGN_NODE_CODE_EDIT,
	UI_DESIGN_NODE_FLOW_GRAPH,
	UI_DESIGN_NODE_WORKFLOW,
	UI_DESIGN_NODE_LAST = UI_DESIGN_NODE_WORKFLOW
} ui_design_node_type_t;

typedef struct ui_design_property_value_t {
	char sId[UI_DESIGN_PROPERTY_ID_CAPACITY];
	char sValue[UI_DESIGN_PROPERTY_VALUE_CAPACITY];
} ui_design_property_value_t;

typedef struct ui_design_node_t {
	int iId;
	ui_design_node_type_t iType;
	int iParentId;
	xui_rect_t tRect;
	char sText[UI_DESIGN_TEXT_CAPACITY];
	int bChecked;
	int bVisible;
	int bEnabled;
	xui_widget pWidget;
	xui_font pRuntimeFont;
	char sRuntimeFontSource[UI_DESIGN_PROPERTY_VALUE_CAPACITY];
	float fRuntimeFontSize;
	uint32_t iRuntimeFontFlags;
	xui_surface pRuntimeSurface;
	char sRuntimeSurfaceSource[UI_DESIGN_PROPERTY_VALUE_CAPACITY];
	xui_surface arrRuntimeSurface[UI_DESIGN_RUNTIME_SURFACE_COUNT];
	char arrRuntimeSurfaceSource[UI_DESIGN_RUNTIME_SURFACE_COUNT][UI_DESIGN_PROPERTY_VALUE_CAPACITY];
	ui_design_property_value_t arrProperties[UI_DESIGN_MAX_NODE_PROPERTIES];
	int iPropertyCount;
	char arrRuntimeText[UI_DESIGN_RUNTIME_TEXT_COUNT][UI_DESIGN_RUNTIME_TEXT_CAPACITY];
	int iRuntimeTextCount;
	int iRuntimeTableRowCount;
	int iRuntimeTableColumnCount;
	int arrRuntimeTableColumnType[UI_DESIGN_RUNTIME_TABLE_COLUMNS];
	int arrRuntimeTableText[UI_DESIGN_RUNTIME_TABLE_ROWS][UI_DESIGN_RUNTIME_TABLE_COLUMNS];
	xui_table_view_cell_t arrRuntimeTableCells[UI_DESIGN_RUNTIME_TABLE_ROWS][UI_DESIGN_RUNTIME_TABLE_COLUMNS];
	int arrRuntimeTableCellTypeSet[UI_DESIGN_RUNTIME_TABLE_ROWS][UI_DESIGN_RUNTIME_TABLE_COLUMNS];
	char arrRuntimeEditorOptionText[UI_DESIGN_RUNTIME_EDITOR_OPTIONS][UI_DESIGN_RUNTIME_EDITOR_OPTION_CAPACITY];
	const char* arrRuntimeEditorOptions[UI_DESIGN_RUNTIME_EDITOR_OPTIONS];
	uint32_t arrRuntimeEditorPalette[UI_DESIGN_RUNTIME_EDITOR_PALETTE];
} ui_design_node_t;

typedef struct ui_design_flat_node_t {
	int iId;
	int iDepth;
} ui_design_flat_node_t;

typedef struct ui_design_model_t {
	ui_design_node_t arrNodes[UI_DESIGN_MAX_NODES];
	int iNodeCount;
	int iNextId;
	int iSelectedId;
	uint32_t iRevision;
} ui_design_model_t;

void uiDesignModelInit(ui_design_model_t* pModel);
const char* uiDesignNodeTypeName(ui_design_node_type_t iType);
int uiDesignNodeTypeIsContainer(ui_design_node_type_t iType);
void uiDesignNodeTypeDefaultSize(ui_design_node_type_t iType, float* pW, float* pH);
int uiDesignModelCanFreeTransformNode(const ui_design_model_t* pModel, const ui_design_node_t* pNode);
ui_design_node_t* uiDesignModelGetNode(ui_design_model_t* pModel, int iId);
const ui_design_node_t* uiDesignModelGetNodeConst(const ui_design_model_t* pModel, int iId);
ui_design_node_t* uiDesignModelGetSelected(ui_design_model_t* pModel);
int uiDesignModelAddNode(ui_design_model_t* pModel, ui_design_node_type_t iType, int iParentId, float fX, float fY, int* pId);
int uiDesignModelSetSelected(ui_design_model_t* pModel, int iId);
int uiDesignModelSetText(ui_design_model_t* pModel, int iId, const char* sText);
int uiDesignModelSetRect(ui_design_model_t* pModel, int iId, xui_rect_t tRect);
int uiDesignModelSetChecked(ui_design_model_t* pModel, int iId, int bChecked);
int uiDesignModelSetVisible(ui_design_model_t* pModel, int iId, int bVisible);
int uiDesignModelSetEnabled(ui_design_model_t* pModel, int iId, int bEnabled);
int uiDesignModelSetProperty(ui_design_model_t* pModel, int iId, const char* sPropertyId, const char* sValue);
int uiDesignNodeSetProperty(ui_design_node_t* pNode, const char* sPropertyId, const char* sValue);
const char* uiDesignNodeGetProperty(const ui_design_node_t* pNode, const char* sPropertyId, const char* sDefaultValue);
int uiDesignNodeGetPropertyBool(const ui_design_node_t* pNode, const char* sPropertyId, int bDefaultValue);
int uiDesignNodeGetPropertyInt(const ui_design_node_t* pNode, const char* sPropertyId, int iDefaultValue);
float uiDesignNodeGetPropertyFloat(const ui_design_node_t* pNode, const char* sPropertyId, float fDefaultValue);
uint32_t uiDesignNodeGetPropertyColor(const ui_design_node_t* pNode, const char* sPropertyId, uint32_t iDefaultValue);
int uiDesignModelGetAbsoluteRect(const ui_design_model_t* pModel, int iId, xui_rect_t* pRect);
int uiDesignModelGetChildHostRect(const ui_design_model_t* pModel, int iId, xui_rect_t* pRect);
int uiDesignModelHitTest(const ui_design_model_t* pModel, float fX, float fY);
int uiDesignModelFindDropParent(const ui_design_model_t* pModel, float fX, float fY);
int uiDesignModelFlatten(const ui_design_model_t* pModel, ui_design_flat_node_t* pItems, int iCapacity);

#ifdef __cplusplus
}
#endif

#endif
