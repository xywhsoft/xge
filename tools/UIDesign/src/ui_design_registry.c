#include "ui_design_registry.h"
#include "ui_design_app.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UI_DESIGN_COUNT_OF(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define UI_DESIGN_INLINE_STYLE_CAPACITY 64
#define UI_DESIGN_CACHE_STATE_ID_CAPACITY 32

#define UI_DESIGN_PROP(id, name, category, desc, value, type, flags, editor) \
	{id, name, category, desc, value, type, flags, editor, NULL, 0}

#define UI_DESIGN_ENUM_PROP(id, name, category, desc, value, items, editor) \
	{id, name, category, desc, value, XUI_TABLE_CELL_TYPE_ENUM, 0, editor, items, UI_DESIGN_COUNT_OF(items)}

#define UI_DESIGN_TERMINAL_DEFAULT_PALETTE \
	"0|#000000\n1|#CD3131\n2|#0DBC79\n3|#E5E510\n4|#2472C8\n5|#BC3FBC\n6|#11A8CD\n7|#E5E5E5\n" \
	"8|#666666\n9|#F14C4C\n10|#23D18B\n11|#F5F543\n12|#3B8EEA\n13|#D670D6\n14|#29B8DB\n15|#FFFFFF"

static int __uiDesignCreateWidget(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateLabel(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateHyperlink(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateButton(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateCheckBox(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateRadio(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateToggle(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateInput(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateTagInput(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateNumericInput(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateTextEdit(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreatePanel(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateSeparator(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateProgress(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateStepBar(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateChart(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateScrollBar(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateSlider(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateRangeSlider(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreatePage(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateCarousel(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateComboBox(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateListView(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateTreeView(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateTableView(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateTableGrid(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateQrCode(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateImage(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateBreadcrumb(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateCheckCard(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateRadioGroup(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateVirtualJoystick(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateInventoryGrid(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateTerminal(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateSplitLayout(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateTabs(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateAccordion(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateWindow(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateScrollFrame(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateScrollView(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateCanvas(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateMessageList(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateTimelineView(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreatePropertyGrid(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateMenuBar(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateToolbar(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateStatusBar(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateDockPanel(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreatePopup(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateMenu(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateOverlayPreview(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateCascader(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateColorPicker(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateDatePicker(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateCodeEdit(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateFlowGraph(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignCreateWorkflow(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget);
static int __uiDesignApplyNode(struct ui_design_app_t* pApp, ui_design_node_t* pNode);
static int __uiDesignApplyButtonPatches(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pWidget);
static int __uiDesignApplyProgressPatches(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pWidget);
static int __uiDesignApplyInputDecorations(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pWidget);
static int __uiDesignApplyInputMenuTitles(ui_design_node_t* pNode, xui_widget pWidget, int iTarget);
static int __uiDesignApplyCodeEditMenuTitles(ui_design_node_t* pNode, xui_widget pWidget);
static int __uiDesignApplyTerminalMenuTitles(ui_design_node_t* pNode, xui_widget pWidget);
static int __uiDesignApplyDockPanelMenuTitles(ui_design_node_t* pNode, xui_widget pWidget);
static int __uiDesignApplyDockPanelTooltips(ui_design_node_t* pNode, xui_widget pWidget);
static void __uiDesignApplyTooltip(ui_design_node_t* pNode, xui_widget pWidget);
static void __uiDesignApplyInlineStyle(ui_design_node_t* pNode, xui_widget pWidget);
static void __uiDesignApplyWidgetCache(ui_design_node_t* pNode, xui_widget pWidget);
static int __uiDesignTableMergeProvider(xui_widget pWidget, int iRow, int iColumn, int* pRowSpan, int* pColSpan, void* pUser);
static int __uiDesignTableGridEditorConfig(xui_widget pWidget, int iRow, int iColumn, int iType, xui_table_grid_editor_config_t* pConfig, void* pUser);
static xui_font __uiDesignResolveNodeFont(struct ui_design_app_t* pApp, ui_design_node_t* pNode);
static int __uiDesignApplyNodeFont(struct ui_design_app_t* pApp, ui_design_node_t* pNode);
static int __uiDesignLoadRuntimeSurfaceSlot(struct ui_design_app_t* pApp, ui_design_node_t* pNode, int iSlot, const char* sSource, xui_surface* ppSurface);
static int __uiDesignLoadRuntimeSurface(struct ui_design_app_t* pApp, ui_design_node_t* pNode, const char* sSource, xui_surface* ppSurface);
static int __uiDesignWorkflowBuildTypeConfigSchema(ui_design_node_t* pNode, const char* sType, xvalue* ppSchema);
static void __uiDesignReadTwoStateSourceRects(const ui_design_node_t* pNode, xui_rect_t* pUncheckedSrc, xui_rect_t* pCheckedSrc);

static const xui_combobox_item_t g_arrTextAlignEnum[] = {
	{"Left", XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP, 1, 0, 0, NULL},
	{"Center", XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP, 1, 0, 0, NULL},
	{"Right", XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrWrapModeEnum[] = {
	{"None", XUI_TEXT_WRAP_NONE, 1, 0, 0, NULL},
	{"Word", XUI_TEXT_WRAP_WORD, 1, 0, 0, NULL},
	{"Char", XUI_TEXT_WRAP_CHAR, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrOrientationEnum[] = {
	{"Horizontal", XUI_ORIENTATION_HORIZONTAL, 1, 0, 0, NULL},
	{"Vertical", XUI_ORIENTATION_VERTICAL, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrSplitLayoutDirectionEnum[] = {
	{"Horizontal Layout (Left / Right)", XUI_ORIENTATION_VERTICAL, 1, 0, 0, NULL},
	{"Vertical Layout (Top / Bottom)", XUI_ORIENTATION_HORIZONTAL, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrSeparatorStyleEnum[] = {
	{"Solid", XUI_SEPARATOR_SOLID, 1, 0, 0, NULL},
	{"Dot", XUI_SEPARATOR_DOT, 1, 0, 0, NULL},
	{"Dash", XUI_SEPARATOR_DASH, 1, 0, 0, NULL},
	{"Dash Dot", XUI_SEPARATOR_DASH_DOT, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrAlignEnum[] = {
	{"Start", XUI_ALIGN_START, 1, 0, 0, NULL},
	{"Center", XUI_ALIGN_CENTER, 1, 0, 0, NULL},
	{"End", XUI_ALIGN_END, 1, 0, 0, NULL},
	{"Stretch", XUI_ALIGN_STRETCH, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrProgressFillEnum[] = {
	{"Left To Right", XUI_PROGRESS_LEFT_TO_RIGHT, 1, 0, 0, NULL},
	{"Right To Left", XUI_PROGRESS_RIGHT_TO_LEFT, 1, 0, 0, NULL},
	{"Bottom To Top", XUI_PROGRESS_BOTTOM_TO_TOP, 1, 0, 0, NULL},
	{"Top To Bottom", XUI_PROGRESS_TOP_TO_BOTTOM, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrProgressPatchModeEnum[] = {
	{"Stretch", XUI_PROGRESS_FILL_STRETCH, 1, 0, 0, NULL},
	{"Reveal", XUI_PROGRESS_FILL_REVEAL, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrNinePatchModeEnum[] = {
	{"Stretch", XUI_NINE_PATCH_STRETCH, 1, 0, 0, NULL},
	{"Tile", XUI_NINE_PATCH_TILE, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrButtonIconPlacementEnum[] = {
	{"Left", XUI_BUTTON_ICON_LEFT, 1, 0, 0, NULL},
	{"Right", XUI_BUTTON_ICON_RIGHT, 1, 0, 0, NULL},
	{"Top", XUI_BUTTON_ICON_TOP, 1, 0, 0, NULL},
	{"Bottom", XUI_BUTTON_ICON_BOTTOM, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrButtonBadgeAnchorEnum[] = {
	{"Content Top Right", XUI_BUTTON_BADGE_CONTENT_TOP_RIGHT, 1, 0, 0, NULL},
	{"Widget Top Right", XUI_BUTTON_BADGE_WIDGET_TOP_RIGHT, 1, 0, 0, NULL},
	{"Icon Top Right", XUI_BUTTON_BADGE_ICON_TOP_RIGHT, 1, 0, 0, NULL},
	{"Text Top Right", XUI_BUTTON_BADGE_TEXT_TOP_RIGHT, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrButtonSemanticEnum[] = {
	{"Default", XUI_BUTTON_SEMANTIC_DEFAULT, 1, 0, 0, NULL},
	{"Primary", XUI_BUTTON_SEMANTIC_PRIMARY, 1, 0, 0, NULL},
	{"Danger", XUI_BUTTON_SEMANTIC_DANGER, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrInputAlignEnum[] = {
	{"Left", XUI_INPUT_ALIGN_LEFT, 1, 0, 0, NULL},
	{"Center", XUI_INPUT_ALIGN_CENTER, 1, 0, 0, NULL},
	{"Right", XUI_INPUT_ALIGN_RIGHT, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComboModeEnum[] = {
	{"Select", XUI_COMBOBOX_MODE_SELECT, 1, 0, 0, NULL},
	{"Edit", XUI_COMBOBOX_MODE_EDIT, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrFindWindowEnum[] = {
	{"None", 0, 1, 0, 0, NULL},
	{"Find", 1, 1, 0, 0, NULL},
	{"Replace", 2, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrFindPreviewEnum[] = {
	{"None", 0, 1, 0, 0, NULL},
	{"Next", 1, 1, 0, 0, NULL},
	{"Previous", 2, 1, 0, 0, NULL},
	{"All", 3, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrPopupPlacementEnum[] = {
	{"Auto", XUI_COMBOBOX_POPUP_AUTO, 1, 0, 0, NULL},
	{"Bottom", XUI_COMBOBOX_POPUP_BOTTOM, 1, 0, 0, NULL},
	{"Top", XUI_COMBOBOX_POPUP_TOP, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrSelectionEnum[] = {
	{"Single", XUI_SELECTION_SINGLE, 1, 0, 0, NULL},
	{"Multi", XUI_SELECTION_MULTI, 1, 0, 0, NULL},
	{"Range", XUI_SELECTION_RANGE, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrInventorySelectionEnum[] = {
	{"None", XUI_INVENTORY_SELECTION_NONE, 1, 0, 0, NULL},
	{"Single", XUI_INVENTORY_SELECTION_SINGLE, 1, 0, 0, NULL},
	{"Multi", XUI_INVENTORY_SELECTION_MULTI, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrTableSelectionEnum[] = {
	{"Cell", XUI_TABLE_VIEW_SELECTION_CELL, 1, 0, 0, NULL},
	{"Row", XUI_TABLE_VIEW_SELECTION_ROW, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrScrollbarModeEnum[] = {
	{"Full", XUI_SCROLLBAR_MODE_FULL, 1, 0, 0, NULL},
	{"Compact", XUI_SCROLLBAR_MODE_COMPACT, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrScrollbarButtonsEnum[] = {
	{"Auto", XUI_SCROLLBAR_BUTTONS_AUTO, 1, 0, 0, NULL},
	{"On", XUI_SCROLLBAR_BUTTONS_ON, 1, 0, 0, NULL},
	{"Off", XUI_SCROLLBAR_BUTTONS_OFF, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrStepBarStyleEnum[] = {
	{"Arrow", XUI_STEP_BAR_STYLE_ARROW, 1, 0, 0, NULL},
	{"Dot", XUI_STEP_BAR_STYLE_DOT, 1, 0, 0, NULL},
	{"Vertical", XUI_STEP_BAR_STYLE_VERTICAL, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrChartSeriesEnum[] = {
	{"Line", XUI_CHART_SERIES_LINE, 1, 0, 0, NULL},
	{"Bar", XUI_CHART_SERIES_BAR, 1, 0, 0, NULL},
	{"Pie", XUI_CHART_SERIES_PIE, 1, 0, 0, NULL},
	{"Scatter", XUI_CHART_SERIES_SCATTER, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrChartAxisEnum[] = {
	{"Value", XUI_CHART_AXIS_VALUE, 1, 0, 0, NULL},
	{"Category", XUI_CHART_AXIS_CATEGORY, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrChartBarModeEnum[] = {
	{"Grouped", XUI_CHART_BAR_GROUPED, 1, 0, 0, NULL},
	{"Stacked", XUI_CHART_BAR_STACKED, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrChartBarDirectionEnum[] = {
	{"Vertical", XUI_CHART_BAR_VERTICAL, 1, 0, 0, NULL},
	{"Horizontal", XUI_CHART_BAR_HORIZONTAL, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrChartPieModeEnum[] = {
	{"Normal", XUI_CHART_PIE_NORMAL, 1, 0, 0, NULL},
	{"Rose", XUI_CHART_PIE_ROSE, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrImageModeEnum[] = {
	{"Natural", XUI_IMAGE_NATURAL, 1, 0, 0, NULL},
	{"Stretch", XUI_IMAGE_STRETCH, 1, 0, 0, NULL},
	{"Contain", XUI_IMAGE_CONTAIN, 1, 0, 0, NULL},
	{"Cover", XUI_IMAGE_COVER, 1, 0, 0, NULL},
	{"Scale Down", XUI_IMAGE_SCALE_DOWN, 1, 0, 0, NULL},
	{"Custom", XUI_IMAGE_CUSTOM, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrScrollPolicyEnum[] = {
	{"Auto", XUI_SCROLLBAR_POLICY_AUTO, 1, 0, 0, NULL},
	{"Always", XUI_SCROLLBAR_POLICY_ALWAYS, 1, 0, 0, NULL},
	{"Hidden", XUI_SCROLLBAR_POLICY_HIDDEN, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrWheelAxisEnum[] = {
	{"Vertical", XUI_WHEEL_AXIS_VERTICAL, 1, 0, 0, NULL},
	{"Horizontal", XUI_WHEEL_AXIS_HORIZONTAL, 1, 0, 0, NULL},
	{"Both", XUI_WHEEL_AXIS_BOTH, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrCornerModeEnum[] = {
	{"None", XUI_SCROLL_FRAME_CORNER_NONE, 1, 0, 0, NULL},
	{"Auto", XUI_SCROLL_FRAME_CORNER_AUTO, 1, 0, 0, NULL},
	{"Grip", XUI_SCROLL_FRAME_CORNER_GRIP, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrTabsPlacementEnum[] = {
	{"Top", XUI_TABS_PLACEMENT_TOP, 1, 0, 0, NULL},
	{"Bottom", XUI_TABS_PLACEMENT_BOTTOM, 1, 0, 0, NULL},
	{"Left", XUI_TABS_PLACEMENT_LEFT, 1, 0, 0, NULL},
	{"Right", XUI_TABS_PLACEMENT_RIGHT, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrAccordionModeEnum[] = {
	{"Multiple", XUI_ACCORDION_MODE_MULTIPLE, 1, 0, 0, NULL},
	{"Single", XUI_ACCORDION_MODE_SINGLE, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrSplitPaneModeEnum[] = {
	{"Grow", XUI_SPLIT_PANE_GROW, 1, 0, 0, NULL},
	{"Fixed", XUI_SPLIT_PANE_FIXED, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrPropertyDescriptionEnum[] = {
	{"None", XUI_PROPERTY_GRID_DESCRIPTION_NONE, 1, 0, 0, NULL},
	{"Tooltip", XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, 1, 0, 0, NULL},
	{"Panel", XUI_PROPERTY_GRID_DESCRIPTION_PANEL, 1, 0, 0, NULL},
	{"Both", XUI_PROPERTY_GRID_DESCRIPTION_BOTH, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrTableGridEditModeEnum[] = {
	{"Display", XUI_TABLE_GRID_EDIT_DISPLAY, 1, 0, 0, NULL},
	{"Quick", XUI_TABLE_GRID_EDIT_QUICK, 1, 0, 0, NULL},
	{"Immediate", XUI_TABLE_GRID_EDIT_IMMEDIATE, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrMessageDataSourceEnum[] = {
	{"Structured", 0, 1, 0, 0, NULL},
	{"Import Text", 1, 1, 0, 0, NULL},
	{"File", 2, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrCascaderPopupEnum[] = {
	{"Auto", XUI_CASCADER_POPUP_AUTO, 1, 0, 0, NULL},
	{"Bottom", XUI_CASCADER_POPUP_BOTTOM, 1, 0, 0, NULL},
	{"Top", XUI_CASCADER_POPUP_TOP, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrCascaderExpandEnum[] = {
	{"Click", XUI_CASCADER_EXPAND_CLICK, 1, 0, 0, NULL},
	{"Hover", XUI_CASCADER_EXPAND_HOVER, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrColorPickerPopupEnum[] = {
	{"Auto", XUI_COLOR_PICKER_POPUP_AUTO, 1, 0, 0, NULL},
	{"Bottom", XUI_COLOR_PICKER_POPUP_BOTTOM, 1, 0, 0, NULL},
	{"Top", XUI_COLOR_PICKER_POPUP_TOP, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrDatePickerModeEnum[] = {
	{"Date", XUI_DATE_PICKER_MODE_DATE, 1, 0, 0, NULL},
	{"Time", XUI_DATE_PICKER_MODE_TIME, 1, 0, 0, NULL},
	{"Date Time", XUI_DATE_PICKER_MODE_DATETIME, 1, 0, 0, NULL},
	{"Date Range", XUI_DATE_PICKER_MODE_DATE_RANGE, 1, 0, 0, NULL},
	{"Time Range", XUI_DATE_PICKER_MODE_TIME_RANGE, 1, 0, 0, NULL},
	{"Date Time Range", XUI_DATE_PICKER_MODE_DATETIME_RANGE, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrDatePickerPopupEnum[] = {
	{"Auto", XUI_DATE_PICKER_POPUP_AUTO, 1, 0, 0, NULL},
	{"Bottom", XUI_DATE_PICKER_POPUP_BOTTOM, 1, 0, 0, NULL},
	{"Top", XUI_DATE_PICKER_POPUP_TOP, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrCodeEolEnum[] = {
	{"Auto", XUI_CODE_EOL_AUTO, 1, 0, 0, NULL},
	{"LF", XUI_CODE_EOL_LF, 1, 0, 0, NULL},
	{"CRLF", XUI_CODE_EOL_CRLF, 1, 0, 0, NULL},
	{"CR", XUI_CODE_EOL_CR, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrPopupAnchorEnum[] = {
	{"Bottom Left", XUI_POPUP_ANCHOR_BOTTOM_LEFT, 1, 0, 0, NULL},
	{"Bottom Right", XUI_POPUP_ANCHOR_BOTTOM_RIGHT, 1, 0, 0, NULL},
	{"Top Left", XUI_POPUP_ANCHOR_TOP_LEFT, 1, 0, 0, NULL},
	{"Top Right", XUI_POPUP_ANCHOR_TOP_RIGHT, 1, 0, 0, NULL},
	{"Cursor", XUI_POPUP_ANCHOR_CURSOR, 1, 0, 0, NULL},
	{"Fixed", XUI_POPUP_ANCHOR_FIXED, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrPopupDirectionEnum[] = {
	{"Right Down", XUI_POPUP_DIRECTION_RIGHT_DOWN, 1, 0, 0, NULL},
	{"Right Up", XUI_POPUP_DIRECTION_RIGHT_UP, 1, 0, 0, NULL},
	{"Left Down", XUI_POPUP_DIRECTION_LEFT_DOWN, 1, 0, 0, NULL},
	{"Left Up", XUI_POPUP_DIRECTION_LEFT_UP, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrPopupOutsidePolicyEnum[] = {
	{"Close", XUI_POPUP_OUTSIDE_CLOSE, 1, 0, 0, NULL},
	{"Ignore", XUI_POPUP_OUTSIDE_IGNORE, 1, 0, 0, NULL},
	{"Consume", XUI_POPUP_OUTSIDE_CONSUME, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrPopupOwnerPolicyEnum[] = {
	{"Passthrough", XUI_POPUP_OWNER_PASSTHROUGH, 1, 0, 0, NULL},
	{"Close", XUI_POPUP_OWNER_CLOSE, 1, 0, 0, NULL},
	{"Toggle", XUI_POPUP_OWNER_TOGGLE, 1, 0, 0, NULL},
	{"Consume", XUI_POPUP_OWNER_CONSUME, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrPopupEscapePolicyEnum[] = {
	{"Close", XUI_POPUP_ESCAPE_CLOSE, 1, 0, 0, NULL},
	{"Ignore", XUI_POPUP_ESCAPE_IGNORE, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrPopupFocusPolicyEnum[] = {
	{"Popup", XUI_POPUP_FOCUS_POPUP, 1, 0, 0, NULL},
	{"None", XUI_POPUP_FOCUS_NONE, 1, 0, 0, NULL},
	{"First Child", XUI_POPUP_FOCUS_FIRST_CHILD, 1, 0, 0, NULL},
	{"Custom", XUI_POPUP_FOCUS_CUSTOM, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrMsgIconEnum[] = {
	{"None", XUI_MSGBOX_ICON_NONE, 1, 0, 0, NULL},
	{"Info", XUI_MSGBOX_ICON_INFO, 1, 0, 0, NULL},
	{"Question", XUI_MSGBOX_ICON_QUEST, 1, 0, 0, NULL},
	{"Warning", XUI_MSGBOX_ICON_WAR, 1, 0, 0, NULL},
	{"Error", XUI_MSGBOX_ICON_ERROR, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrMsgBoxButtonsEnum[] = {
	{"OK", XUI_MSGBOX_BUTTON_OK, 1, 0, 0, NULL},
	{"OK Cancel", XUI_MSGBOX_BUTTON_OK_CANCEL, 1, 0, 0, NULL},
	{"Yes No", XUI_MSGBOX_BUTTON_YES_NO, 1, 0, 0, NULL},
	{"Yes No Cancel", XUI_MSGBOX_BUTTON_YES_NO_CANCEL, 1, 0, 0, NULL},
	{"Custom", XUI_MSGBOX_BUTTON_CUSTOM, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrFileDialogModeEnum[] = {
	{"Open File", XUI_FILE_DIALOG_MODE_OPEN_FILE, 1, 0, 0, NULL},
	{"Save File", XUI_FILE_DIALOG_MODE_SAVE_FILE, 1, 0, 0, NULL},
	{"Select Folder", XUI_FILE_DIALOG_MODE_SELECT_FOLDER, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrToastTypeEnum[] = {
	{"Info", XUI_TOAST_TYPE_INFO, 1, 0, 0, NULL},
	{"Success", XUI_TOAST_TYPE_SUCCESS, 1, 0, 0, NULL},
	{"Warning", XUI_TOAST_TYPE_WARNING, 1, 0, 0, NULL},
	{"Error", XUI_TOAST_TYPE_ERROR, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrToastPlacementEnum[] = {
	{"Top Right", XUI_TOAST_PLACEMENT_TOP_RIGHT, 1, 0, 0, NULL},
	{"Top Left", XUI_TOAST_PLACEMENT_TOP_LEFT, 1, 0, 0, NULL},
	{"Bottom Right", XUI_TOAST_PLACEMENT_BOTTOM_RIGHT, 1, 0, 0, NULL},
	{"Bottom Left", XUI_TOAST_PLACEMENT_BOTTOM_LEFT, 1, 0, 0, NULL},
	{"Top Center", XUI_TOAST_PLACEMENT_TOP_CENTER, 1, 0, 0, NULL},
	{"Bottom Center", XUI_TOAST_PLACEMENT_BOTTOM_CENTER, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrToastDirectionEnum[] = {
	{"Auto", XUI_TOAST_DIRECTION_AUTO, 1, 0, 0, NULL},
	{"Down", XUI_TOAST_DIRECTION_DOWN, 1, 0, 0, NULL},
	{"Up", XUI_TOAST_DIRECTION_UP, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrFlowRouteEnum[] = {
	{"Auto", XUI_FLOW_ROUTE_AUTO, 1, 0, 0, NULL},
	{"Straight", XUI_FLOW_ROUTE_STRAIGHT, 1, 0, 0, NULL},
	{"Orthogonal", XUI_FLOW_ROUTE_ORTHOGONAL, 1, 0, 0, NULL},
	{"Bezier", XUI_FLOW_ROUTE_BEZIER, 1, 0, 0, NULL}
};

static const char* g_arrDefaultItems[] = {
	"Item 1",
	"Item 2",
	"Item 3"
};

static const xui_breadcrumb_item_t g_arrDefaultBreadcrumbItems[] = {
	{"Home", 1, 1},
	{"Project", 1, 2},
	{"Editor", 0, 3}
};

static const char* g_arrDefaultTabs[] = {
	"Design",
	"Preview",
	"Code"
};

static const int g_arrDefaultEnabled[] = {
	1,
	1,
	1
};

static const int g_arrDefaultDirty[] = {
	0,
	1,
	0
};

static const xui_accordion_section_desc_t g_arrDefaultAccordionSections[] = {
	{"General", 1, 1, 0},
	{"Appearance", 2, 0, 0},
	{"Layout", 3, 0, 0}
};

static const xui_menubar_item_t g_arrDefaultMenuBarItems[] = {
	{"File", XUI_MENUBAR_ITEM_ENABLED, 1, 'F', NULL, {0.0f, 0.0f, 0.0f, 0.0f}, NULL},
	{"Edit", XUI_MENUBAR_ITEM_ENABLED, 2, 'E', NULL, {0.0f, 0.0f, 0.0f, 0.0f}, NULL},
	{"View", XUI_MENUBAR_ITEM_ENABLED, 3, 'V', NULL, {0.0f, 0.0f, 0.0f, 0.0f}, NULL}
};

static const xui_toolbar_item_t g_arrDefaultToolbarItems[] = {
	{"New", "New file", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 1, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL},
	{"Open", "Open file", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 2, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL},
	{NULL, NULL, XUI_TOOLBAR_ITEM_SEPARATOR, 0, 0, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL},
	{"Run", "Run", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED, 3, 1, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL}
};

static const xui_statusbar_item_t g_arrDefaultStatusItems[] = {
	{"Ready", XUI_STATUSBAR_ITEM_TEXT, XUI_STATUSBAR_SECTION_LEFT, XUI_STATUSBAR_ITEM_ENABLED, 1, 96.0f, 0.0f, 0.0f, 0.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, NULL},
	{"", XUI_STATUSBAR_ITEM_SPACER, XUI_STATUSBAR_SECTION_CENTER, XUI_STATUSBAR_ITEM_ENABLED, 0, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, NULL},
	{"100%", XUI_STATUSBAR_ITEM_TEXT, XUI_STATUSBAR_SECTION_RIGHT, XUI_STATUSBAR_ITEM_ENABLED, 2, 72.0f, 0.0f, 0.0f, 0.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, NULL}
};

static const xui_menu_item_t g_arrDefaultMenuItems[] = {
	{"New", "Ctrl+N", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DEFAULT, 1, 0, NULL, NULL},
	{"Open", "Ctrl+O", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, 2, 0, NULL, NULL},
	{NULL, NULL, XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL},
	{"Show Grid", NULL, XUI_MENU_ITEM_CHECK, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED, 3, 0, NULL, NULL},
	{"Danger Action", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DANGER, 4, 0, NULL, NULL},
	{"More", NULL, XUI_MENU_ITEM_SUBMENU, XUI_MENU_ITEM_ENABLED, 5, 0, NULL, NULL}
};

static const char g_sDefaultMenuBarMenus[] =
	"File|New|normal|true|false|true|false|Ctrl+N|101|0\n"
	"File|Open|normal|true|false|false|false|Ctrl+O|102|0\n"
	"File|Recent|submenu|true|false|false|false||103|0\n"
	"File|-|separator|false|false|false|false||0|0\n"
	"File|Exit|normal|true|false|false|false||199|0\n"
	"Edit|Undo|normal|true|false|false|false|Ctrl+Z|201|0\n"
	"Edit|Redo|normal|true|false|false|false|Ctrl+Y|202|0\n"
	"Edit|-|separator|false|false|false|false||0|0\n"
	"Edit|Find|normal|true|false|false|false|Ctrl+F|203|0\n"
	"View|Show Grid|check|true|true|false|false||301|0";

static const char g_sDefaultMenuBarSubmenus[] =
	"File|Recent|Project Alpha|normal|true|false|false|false|Ctrl+1|1101|0\n"
	"File|Recent|Project Beta|normal|true|false|false|false|Ctrl+2|1102|0";

static const char g_sDefaultMenuSubmenus[] =
	"More|Recent Project|normal|true|false|false|false|Ctrl+R|51|0\n"
	"More|Settings|normal|true|false|false|false||52|0";

static const xui_cascader_item_t g_arrDefaultCascaderItems[] = {
	{"UI", 1, -1, 0, NULL},
	{"Controls", 2, 0, 0, NULL},
	{"Button", 3, 1, XUI_CASCADER_ITEM_LEAF, NULL},
	{"Input", 4, 1, XUI_CASCADER_ITEM_LEAF, NULL}
};

static const uint32_t g_arrDefaultPalette[] = {
	XUI_COLOR_RGBA(49, 126, 214, 255),
	XUI_COLOR_RGBA(214, 72, 86, 255),
	XUI_COLOR_RGBA(45, 157, 110, 255),
	XUI_COLOR_RGBA(245, 166, 35, 255),
	XUI_COLOR_RGBA(91, 91, 232, 255)
};

static const xui_message_node_t g_arrDefaultMessages[] = {
	{sizeof(xui_message_node_t), "m1", "Designer", "09:20", "Widget created", XUI_MESSAGE_NODE_OTHER, 0, NULL},
	{sizeof(xui_message_node_t), "m2", "You", "09:21", "Adjust layout", XUI_MESSAGE_NODE_SELF, 0, NULL},
	{sizeof(xui_message_node_t), "m3", "System", "09:22", "Preview updated", XUI_MESSAGE_NODE_SYSTEM, 0, NULL}
};

static const xui_inventory_slot_t g_arrDefaultInventorySlots[] = {
	{sizeof(xui_inventory_slot_t), 1, 1001, 1, 1, 0, 0, 0u, XUI_COLOR_RGBA(49, 126, 214, 255), XUI_COLOR_WHITE, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, 0.0f, 1.0f, "Sword", "1", NULL, 0u, 1.0f, XUI_COLOR_WHITE},
	{sizeof(xui_inventory_slot_t), 2, 1002, 8, 20, 0, 0, 0u, XUI_COLOR_RGBA(45, 157, 110, 255), XUI_COLOR_WHITE, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, 0.0f, 1.0f, "Potion", "2", NULL, 0u, 1.0f, XUI_COLOR_WHITE},
	{sizeof(xui_inventory_slot_t), 3, 1003, 1, 1, 0, 0, XUI_INVENTORY_SLOT_EQUIPPED, XUI_COLOR_RGBA(245, 166, 35, 255), XUI_COLOR_WHITE, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, 0.0f, 0.7f, "Shield", "3", NULL, 0u, 1.0f, XUI_COLOR_WHITE}
};

static const char* g_arrDefaultStepTitles[] = {
	"Start",
	"Edit",
	"Done"
};

static const char* g_arrDefaultTags[] = {
	"alpha",
	"beta"
};

static const char* g_arrDefaultRadioOptions[] = {
	"Option A",
	"Option B",
	"Option C"
};

static const xui_chart_point_t g_arrChartPoints[] = {
	{0.0, 2.0, 2.0, "A", XUI_COLOR_RGBA(49, 126, 214, 255)},
	{1.0, 5.0, 5.0, "B", XUI_COLOR_RGBA(49, 126, 214, 255)},
	{2.0, 3.0, 3.0, "C", XUI_COLOR_RGBA(49, 126, 214, 255)},
	{3.0, 7.0, 7.0, "D", XUI_COLOR_RGBA(49, 126, 214, 255)}
};

static const char* g_arrTableCellText[3][3] = {
	{"Button", "118", "32"},
	{"Input", "180", "30"},
	{"Table", "320", "180"}
};

static const ui_design_property_def_t g_arrWidgetProperties[] = {
	UI_DESIGN_PROP("appearance.backgroundColor", "Background Color", "Appearance", "Designer fill color for the container placeholder.", "#F7FBFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border Color", "Appearance", "Designer stroke color for the container placeholder.", "#6F87A4", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrLabelProperties[] = {
	UI_DESIGN_PROP("appearance.textColor", "Text Color", "Appearance", "Normal text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("text.flags", "Text Align", "Text", "Horizontal text alignment.", "272", g_arrTextAlignEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("text.wrapMode", "Wrap Mode", "Text", "Text wrapping mode.", "0", g_arrWrapModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.underline", "Underline", "Text", "Draw underline.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.lineGap", "Line Gap", "Text", "Extra line gap.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.paragraphGap", "Paragraph Gap", "Text", "Extra paragraph gap.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrHyperlinkProperties[] = {
	UI_DESIGN_PROP("appearance.textColor", "Text Color", "Appearance", "Normal text color.", "#1C66B0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverTextColor", "Hover Text", "Appearance", "Hover text color.", "#2484D6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeTextColor", "Active Text", "Appearance", "Pressed text color.", "#1668B0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("text.flags", "Text Align", "Text", "Horizontal text alignment.", "272", g_arrTextAlignEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("text.wrapMode", "Wrap Mode", "Text", "Text wrapping mode.", "0", g_arrWrapModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.underline", "Underline", "Text", "Underline normal state.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.hoverUnderline", "Hover Underline", "Text", "Underline hover state.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.activeUnderline", "Active Underline", "Text", "Underline active state.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.lineGap", "Line Gap", "Text", "Extra line gap.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.paragraphGap", "Paragraph Gap", "Text", "Extra paragraph gap.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrButtonProperties[] = {
	UI_DESIGN_PROP("behavior.selectable", "Selectable", "Behavior", "Button can stay selected.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.selected", "Selected", "Behavior", "Current selected state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.semantic", "Semantic", "Behavior", "Built-in button semantic.", "0", g_arrButtonSemanticEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.iconSource", "Icon Source", "Data", "Optional button icon image.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.statePatches", "State Patches", "Appearance", "Rows: state|source|x|y|w|h|sliceLeft|sliceTop|sliceRight|sliceBottom|color|mode.", "normal||||||||||#FFFFFFFF|stretch", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("appearance.textColor", "Text Color", "Appearance", "Normal text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("text.flags", "Text Align", "Text", "Button text alignment.", "273", g_arrTextAlignEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.normalColor", "Normal Fill", "Appearance", "Normal background color.", "#F8FBFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover Fill", "Appearance", "Hover background color.", "#EAF3FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeColor", "Active Fill", "Appearance", "Pressed background color.", "#D9EAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus Ring", "Appearance", "Focus ring color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled Fill", "Appearance", "Disabled background color.", "#EEF2F7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.checkedColor", "Checked Fill", "Appearance", "Selected background color.", "#D9EAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border Color", "Border", "Normal border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBorderColor", "Hover Border", "Border", "Hover border color.", "#7EAADC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeBorderColor", "Active Border", "Border", "Pressed border color.", "#6F98C7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledBorderColor", "Disabled Border", "Border", "Disabled border color.", "#D6DBE1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.checkedBorderColor", "Checked Border", "Border", "Selected border color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderWidth", "Border Width", "Border", "Normal border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBorderWidth", "Hover Border W", "Border", "Hover border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeBorderWidth", "Active Border W", "Border", "Pressed border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusBorderWidth", "Focus Ring W", "Border", "Focus ring width.", "2", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledBorderWidth", "Disabled Border W", "Border", "Disabled border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.checkedBorderWidth", "Checked Border W", "Border", "Selected border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.iconColor", "Icon Color", "Icon", "Icon tint color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.iconPlacement", "Icon Placement", "Icon", "Icon placement relative to text.", "0", g_arrButtonIconPlacementEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconSize", "Icon Size", "Icon", "Icon display size.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconGap", "Icon Gap", "Icon", "Gap between icon and text.", "6", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.x", "Icon Source X", "Icon", "Icon source rectangle x. 0 with zero width keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.y", "Icon Source Y", "Icon", "Icon source rectangle y. 0 with zero height keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.w", "Icon Source W", "Icon", "Icon source rectangle width. 0 keeps the full image width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.h", "Icon Source H", "Icon", "Icon source rectangle height. 0 keeps the full image height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("badge.visible", "Badge Visible", "Badge", "Show the badge dot.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("badge.anchor", "Badge Anchor", "Badge", "Badge anchor position.", "0", g_arrButtonBadgeAnchorEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("badge.offsetX", "Badge Offset X", "Badge", "Horizontal badge offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("badge.offsetY", "Badge Offset Y", "Badge", "Vertical badge offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.badgeSize", "Badge Size", "Badge", "Badge dot size.", "12", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.badgeSource", "Badge Source", "Badge", "Optional badge image source.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("badge.sourceX", "Badge Source X", "Badge", "Badge image source rectangle x. 0 with zero width keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("badge.sourceY", "Badge Source Y", "Badge", "Badge image source rectangle y. 0 with zero height keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("badge.sourceW", "Badge Source W", "Badge", "Badge image source rectangle width. 0 keeps the full image width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("badge.sourceH", "Badge Source H", "Badge", "Badge image source rectangle height. 0 keeps the full image height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrCheckProperties[] = {
	UI_DESIGN_PROP("behavior.useBuiltinAtlas", "Builtin Atlas", "Behavior", "Use built-in indicator atlas.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.indicatorSource", "Indicator Source", "Data", "Optional indicator image atlas.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text Color", "Appearance", "Normal text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("text.flags", "Text Align", "Text", "Choice text alignment.", "272", g_arrTextAlignEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.accentColor", "Accent Color", "Appearance", "Checked accent color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border Color", "Appearance", "Indicator border color.", "#8DA5C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBorderColor", "Hover Border", "Appearance", "Hover indicator border color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus Color", "Appearance", "Focus ring color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.indicatorSize", "Indicator Size", "Metrics", "Indicator glyph size.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.gap", "Gap", "Metrics", "Gap between indicator and text.", "7", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("unchecked.x", "Unchecked X", "Unchecked Image", "Unchecked indicator source x. 0 with zero width keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("unchecked.y", "Unchecked Y", "Unchecked Image", "Unchecked indicator source y. 0 with zero height keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("unchecked.w", "Unchecked W", "Unchecked Image", "Unchecked indicator source width. 0 keeps the full image width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("unchecked.h", "Unchecked H", "Unchecked Image", "Unchecked indicator source height. 0 keeps the full image height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("checked.x", "Checked X", "Checked Image", "Checked indicator source x. 0 with zero width keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("checked.y", "Checked Y", "Checked Image", "Checked indicator source y. 0 with zero height keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("checked.w", "Checked W", "Checked Image", "Checked indicator source width. 0 keeps the full image width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("checked.h", "Checked H", "Checked Image", "Checked indicator source height. 0 keeps the full image height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrToggleProperties[] = {
	UI_DESIGN_PROP("checked", "Checked", "Behavior", "Checked state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.useBuiltinAtlas", "Builtin Atlas", "Behavior", "Use built-in toggle atlas.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.uncheckedText", "Unchecked Text", "Text", "Inner text for the off state.", "Off", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.checkedText", "Checked Text", "Text", "Inner text for the on state.", "On", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text Color", "Appearance", "Normal label text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled label text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.accentColor", "Accent Color", "Appearance", "Checked accent color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track Color", "Appearance", "Unchecked track color.", "#D5DEE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverTrackColor", "Hover Track", "Appearance", "Hover track color.", "#C7D7EA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus Color", "Appearance", "Focus ring color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.uncheckedTextColor", "Off Text Color", "Appearance", "Inner off text color.", "#52657A", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.checkedTextColor", "On Text Color", "Appearance", "Inner on text color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.trackWidth", "Track Width", "Metrics", "Track width.", "44", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.trackHeight", "Track Height", "Metrics", "Track height.", "22", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.thumbSize", "Thumb Size", "Metrics", "Thumb size.", "18", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.gap", "Gap", "Metrics", "Gap between track and text.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.innerTextPadding", "Inner Text Padding", "Metrics", "Padding around the inner state text.", "6", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.innerTextGap", "Inner Text Gap", "Metrics", "Gap between the thumb and inner state text.", "2", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.indicatorSource", "Indicator Image", "Data", "Optional custom indicator image atlas.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("unchecked.x", "Off X", "Image Source", "Unchecked source x.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("unchecked.y", "Off Y", "Image Source", "Unchecked source y.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("unchecked.w", "Off W", "Image Source", "Unchecked source width. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("unchecked.h", "Off H", "Image Source", "Unchecked source height. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("checked.x", "On X", "Image Source", "Checked source x.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("checked.y", "On Y", "Image Source", "Checked source y.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("checked.w", "On W", "Image Source", "Checked source width. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("checked.h", "On H", "Image Source", "Checked source height. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrInputProperties[] = {
	UI_DESIGN_PROP("data.decorations", "Decorations", "Data", "Rows: side|kind|visible|width|padding|icon|text|source|sourceX|sourceY|sourceW|sourceH|color|hoverColor|activeColor|disabledColor.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.menuTitles", "Menu Titles", "Data", "Rows: command|title. Commands: undo, redo, cut, copy, paste, delete, selectAll.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("text.placeholder", "Placeholder", "Text", "Placeholder text.", "Input text", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.maxLength", "Max Length", "Text", "Maximum text length. 0 means unlimited.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("text.align", "Text Align", "Text", "Text alignment.", "0", g_arrInputAlignEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.selectionStart", "Selection Start", "Value", "Selection start byte offset.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.selectionEnd", "Selection End", "Value", "Selection end byte offset.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.password", "Password", "Behavior", "Mask user input.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.readonly", "Readonly", "Behavior", "Readonly state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.error", "Error", "Behavior", "Error visual state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBackgroundColor", "Hover Background", "Appearance", "Hover background color.", "#F9FCFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledBackgroundColor", "Disabled Background", "Appearance", "Disabled background color.", "#F2F5F9", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text Color", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.placeholderColor", "Placeholder", "Appearance", "Placeholder text color.", "#8794A6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#96A0AC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border Color", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBorderColor", "Hover Border", "Appearance", "Hover border color.", "#69A6E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusBorderColor", "Focus Border", "Appearance", "Focus border color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.errorBackgroundColor", "Error Background", "Appearance", "Error background color.", "#FFF2F3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.errorBorderColor", "Error Border", "Appearance", "Error border color.", "#D64856", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectionColor", "Selection", "Appearance", "Text selection color.", "#2F80ED4E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.cursorColor", "Cursor", "Appearance", "Caret color.", "#215EAA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrTagInputProperties[] = {
	UI_DESIGN_PROP("data.tags", "Tags", "Data", "Initial tags.", "alpha\nbeta", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.menuTitles", "Menu Titles", "Data", "Rows: command|title. Commands: undo, redo, cut, copy, paste, delete, selectAll.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("text.inputText", "Input Text", "Text", "Typed text currently shown in the tag editor.", "", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.placeholder", "Placeholder", "Text", "Placeholder text.", "Add tag", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.maxLength", "Max Length", "Text", "Maximum text length. 0 means unlimited.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.maxTags", "Max Tags", "Behavior", "Maximum tag count. 0 means unlimited.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.tagHeight", "Tag Height", "Metrics", "Tag chip height.", "22", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBackgroundColor", "Hover Background", "Appearance", "Hover background color.", "#FAFDFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusBackgroundColor", "Focus Background", "Appearance", "Focus background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledBackgroundColor", "Disabled Background", "Appearance", "Disabled background color.", "#F2F5F9", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Input Text", "Appearance", "Typed input text color.", "#1F2937", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.placeholderColor", "Placeholder", "Appearance", "Placeholder text color.", "#8794A6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#96A0AC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBorderColor", "Hover Border", "Appearance", "Hover border color.", "#7EB0E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusBorderColor", "Focus Border", "Appearance", "Focus border color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.tagBackgroundColor", "Tag Background", "Appearance", "Tag background color.", "#EAF3FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.tagHoverBackgroundColor", "Tag Hover Background", "Appearance", "Tag hover background color.", "#EBF1F8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.tagTextColor", "Tag Text", "Appearance", "Tag text color.", "#1C66B0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.tagCloseColor", "Tag Close", "Appearance", "Tag close icon color.", "#748090", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.tagCloseHoverColor", "Tag Close Hover", "Appearance", "Tag close icon hover color.", "#424E5E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrNumericInputProperties[] = {
	UI_DESIGN_PROP("data.menuTitles", "Menu Titles", "Data", "Rows: command|title. Commands: undo, redo, cut, copy, paste, delete, selectAll.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("text.placeholder", "Placeholder", "Text", "Placeholder text.", "0", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.rawText", "Raw Text", "Text", "Optional raw editor text used when Text Override is enabled.", "", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.min", "Minimum", "Value", "Minimum value.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.max", "Maximum", "Value", "Maximum value.", "100", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.value", "Value", "Value", "Current value.", "25", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.step", "Step", "Value", "Increment step.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.integer", "Integer", "Value", "Restrict to integer values.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.precision", "Precision", "Value", "Decimal precision.", "2", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.readonly", "Readonly", "Behavior", "Readonly state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.spinnerVisible", "Spinner Visible", "Behavior", "Show spinner buttons.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.textOverride", "Text Override", "Behavior", "Use Raw Text instead of formatting the numeric value.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.spinnerWidth", "Spinner Width", "Metrics", "Spinner button width.", "22", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Input border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBackgroundColor", "Hover Background", "Appearance", "Hover background color.", "#F9FCFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledBackgroundColor", "Disabled Background", "Appearance", "Disabled background color.", "#F2F5F9", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text Color", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.placeholderColor", "Placeholder", "Appearance", "Placeholder text color.", "#8794A6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#96A0AC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border Color", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBorderColor", "Hover Border", "Appearance", "Hover border color.", "#69A6E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusBorderColor", "Focus Border", "Appearance", "Focus border color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.errorBackgroundColor", "Error Background", "Appearance", "Error background color.", "#FFF2F3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.errorBorderColor", "Error Border", "Appearance", "Error border color.", "#D64856", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectionColor", "Selection", "Appearance", "Text selection color.", "#2F80ED4E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.cursorColor", "Cursor", "Appearance", "Caret color.", "#215EAA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.spinnerColor", "Spinner Color", "Appearance", "Spinner button color.", "#EFF5FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.spinnerHoverColor", "Spinner Hover", "Appearance", "Spinner hover color.", "#DCEBFB", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.spinnerActiveColor", "Spinner Active", "Appearance", "Spinner active color.", "#C8DDF5", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.spinnerBorderColor", "Spinner Border", "Appearance", "Spinner border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.spinnerIconColor", "Spinner Icon", "Appearance", "Spinner icon color.", "#34475D", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.spinnerDisabledIconColor", "Disabled Icon", "Appearance", "Disabled spinner icon color.", "#9AA8BA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrTextEditProperties[] = {
	UI_DESIGN_PROP("data.text", "Text", "Data", "Editor text.", "Line 1\nLine 2\nLine 3", XUI_TABLE_CELL_TYPE_TEXTAREA, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.menuTitles", "Menu Titles", "Data", "Rows: command|title. Commands: undo, redo, cut, copy, paste, delete, selectAll.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("text.placeholder", "Placeholder", "Text", "Placeholder text.", "Multi-line text", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.maxLength", "Max Length", "Text", "Maximum text length. 0 means unlimited.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.pattern", "Pattern", "Find", "Pattern used to preview built-in find results.", "", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.replacement", "Replacement", "Find", "Replacement text used by the replace window.", "", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("find.previewMode", "Preview", "Find", "Precompute a find preview.", "0", g_arrFindPreviewEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.caseSensitive", "Case Sensitive", "Find", "Match case.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.wholeWord", "Whole Word", "Find", "Match whole words.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.regex", "Regex", "Find", "Use regular expression matching.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.escape", "Escape", "Find", "Parse escape sequences.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.selection", "Selection Scope", "Find", "Restrict search to the current selection/range.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.wrap", "Wrap", "Find", "Wrap search at document boundaries.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.backward", "Backward", "Find", "Search backward.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.startOffset", "Start Offset", "Find", "Search start byte offset.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.rangeStart", "Range Start", "Find", "Optional search range start byte offset.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.rangeEnd", "Range End", "Find", "Optional search range end byte offset. 0 means document end.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.readonly", "Readonly", "Behavior", "Readonly state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.wordWrap", "Word Wrap", "Behavior", "Enable word wrapping.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.lineNumbers", "Line Numbers", "Behavior", "Show line numbers.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.findWindow", "Find Window", "Behavior", "Open the built-in find or replace window in the designer preview.", "0", g_arrFindWindowEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.selectionStart", "Selection Start", "Value", "Initial selection start offset.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.selectionEnd", "Selection End", "Value", "Initial selection end offset.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.scrollX", "Scroll X", "Viewport", "Initial horizontal scroll offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.scrollY", "Scroll Y", "Viewport", "Initial vertical scroll offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.lineNumberWidth", "Line Number Width", "Metrics", "Line number gutter width.", "42", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Editor border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.lineGap", "Line Gap", "Metrics", "Extra gap between text lines.", "2", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBackgroundColor", "Hover Background", "Appearance", "Hover background color.", "#F9FCFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledBackgroundColor", "Disabled Background", "Appearance", "Disabled background color.", "#F2F5F9", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text Color", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.placeholderColor", "Placeholder", "Appearance", "Placeholder text color.", "#8794A6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#96A0AC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border Color", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBorderColor", "Hover Border", "Appearance", "Hover border color.", "#69A6E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusBorderColor", "Focus Border", "Appearance", "Focus border color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectionColor", "Selection", "Appearance", "Selection background color.", "#2F80ED4E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.cursorColor", "Cursor", "Appearance", "Cursor color.", "#215EAA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.findResultColor", "Find Result", "Appearance", "Find result highlight color.", "#FFEB8096", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.findActiveColor", "Find Active", "Appearance", "Active find result color.", "#FFB74DBE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.lineNumberColor", "Line Number Text", "Appearance", "Line number text color.", "#6B7F95", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.lineNumberBackgroundColor", "Line Number Back", "Appearance", "Line number gutter background.", "#F3F7FB", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.lineNumberBorderColor", "Line Number Border", "Appearance", "Line number gutter border.", "#CFDAE7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrPanelProperties[] = {
	UI_DESIGN_PROP("data.iconSource", "Icon Source", "Data", "Optional panel icon image path.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.titleColor", "Title Color", "Appearance", "Panel title color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTitleColor", "Disabled Title", "Appearance", "Disabled title color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Outer background color.", "#F7FBFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.headerColor", "Header", "Appearance", "Header background color.", "#EAF2FB", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.clientColor", "Client", "Appearance", "Client background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Panel border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderWidth", "Border Width", "Appearance", "Panel border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("text.flags", "Title Align", "Text", "Panel title alignment.", "272", g_arrTextAlignEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.headerHeight", "Header Height", "Metrics", "Panel header height.", "28", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.headerGap", "Header Gap", "Metrics", "Gap below header.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconSize", "Icon Size", "Metrics", "Header icon size.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.clientClip", "Clip Client", "Behavior", "Clip panel client content.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.x", "Icon Source X", "Icon Source", "Icon source rectangle x. 0 uses the image origin.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.y", "Icon Source Y", "Icon Source", "Icon source rectangle y. 0 uses the image origin.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.w", "Icon Source W", "Icon Source", "Icon source rectangle width. 0 keeps the full image width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.h", "Icon Source H", "Icon Source", "Icon source rectangle height. 0 keeps the full image height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrSeparatorProperties[] = {
	UI_DESIGN_PROP("appearance.color", "Color", "Appearance", "Line color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.thickness", "Thickness", "Metrics", "Line thickness.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.orientation", "Orientation", "Behavior", "Separator orientation.", "0", g_arrOrientationEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.align", "Align", "Behavior", "Line alignment inside bounds.", "1", g_arrAlignEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.lineStyle", "Line Style", "Behavior", "Stroke style.", "0", g_arrSeparatorStyleEnum, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrProgressProperties[] = {
	UI_DESIGN_PROP("value.min", "Minimum", "Value", "Minimum value.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.max", "Maximum", "Value", "Maximum value.", "100", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.value", "Value", "Value", "Current value.", "42", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.template", "Text Template", "Text", "Progress text template.", "{value}%", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("text.flags", "Text Align", "Text", "Progress text alignment.", "272", g_arrTextAlignEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track Color", "Appearance", "Track color.", "#E3EAF2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.fillColor", "Fill Color", "Appearance", "Fill color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text Color", "Appearance", "Text color over track.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.fillTextColor", "Fill Text Color", "Appearance", "Text color over fill.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.fillDirection", "Fill Direction", "Behavior", "Progress fill direction.", "0", g_arrProgressFillEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.fillPatchMode", "Fill Patch Mode", "Behavior", "Nine-patch fill behavior when fill patch is configured.", "0", g_arrProgressPatchModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.trackPatchSource", "Track Patch", "Track Patch", "Track nine-patch image file. Empty uses Track Color.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("trackPatch.x", "Source X", "Track Patch", "Track patch source x. 0 uses the image origin.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("trackPatch.y", "Source Y", "Track Patch", "Track patch source y. 0 uses the image origin.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("trackPatch.w", "Source W", "Track Patch", "Track patch source width. 0 keeps the full image width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("trackPatch.h", "Source H", "Track Patch", "Track patch source height. 0 keeps the full image height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("trackPatch.sliceLeft", "Slice Left", "Track Patch", "Left fixed edge in pixels.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("trackPatch.sliceTop", "Slice Top", "Track Patch", "Top fixed edge in pixels.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("trackPatch.sliceRight", "Slice Right", "Track Patch", "Right fixed edge in pixels.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("trackPatch.sliceBottom", "Slice Bottom", "Track Patch", "Bottom fixed edge in pixels.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("trackPatch.color", "Tint", "Track Patch", "Track patch tint color.", "#FFFFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("trackPatch.mode", "Mode", "Track Patch", "Track patch stretch or tile mode.", "0", g_arrNinePatchModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.fillPatchSource", "Fill Patch", "Fill Patch", "Fill nine-patch image file. Empty uses Fill Color.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("fillPatch.x", "Source X", "Fill Patch", "Fill patch source x. 0 uses the image origin.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("fillPatch.y", "Source Y", "Fill Patch", "Fill patch source y. 0 uses the image origin.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("fillPatch.w", "Source W", "Fill Patch", "Fill patch source width. 0 keeps the full image width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("fillPatch.h", "Source H", "Fill Patch", "Fill patch source height. 0 keeps the full image height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("fillPatch.sliceLeft", "Slice Left", "Fill Patch", "Left fixed edge in pixels.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("fillPatch.sliceTop", "Slice Top", "Fill Patch", "Top fixed edge in pixels.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("fillPatch.sliceRight", "Slice Right", "Fill Patch", "Right fixed edge in pixels.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("fillPatch.sliceBottom", "Slice Bottom", "Fill Patch", "Bottom fixed edge in pixels.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("fillPatch.color", "Tint", "Fill Patch", "Fill patch tint color.", "#FFFFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("fillPatch.mode", "Mode", "Fill Patch", "Fill patch stretch or tile mode.", "0", g_arrNinePatchModeEnum, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrStepBarProperties[] = {
	UI_DESIGN_PROP("data.steps", "Steps", "Data", "Step titles.", "Start\nEdit\nDone", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.current", "Current", "Data", "Current step index.", "1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.style", "Style", "Behavior", "Step bar style.", "1", g_arrStepBarStyleEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.barHeight", "Bar Height", "Metrics", "Bar height.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.dotRadius", "Dot Radius", "Metrics", "Dot radius.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.lineWidth", "Line Width", "Metrics", "Connector line width.", "2", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.doneColor", "Done Color", "Appearance", "Completed step color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeColor", "Active Color", "Appearance", "Current step color.", "#D64856", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.pendingColor", "Pending Color", "Appearance", "Pending step color.", "#B8C5D3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.lineColor", "Line Color", "Appearance", "Connector line color.", "#D5DEE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background under progress markers.", "#00000000", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text Color", "Appearance", "Normal text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeTextColor", "Active Text", "Appearance", "Current text color.", "#D64856", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.pendingTextColor", "Pending Text", "Appearance", "Pending text color.", "#6B7F95", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrChartProperties[] = {
	UI_DESIGN_PROP("data.seriesList", "Series", "Data", "Rows: id|type|name|visible|color|areaFill|areaColor|smooth|dash|symbol|symbolSize|radiusMin|radiusMax|valueMinColor|valueMaxColor.", "s1|line|Series|true|#317ED6|true|#317ED640|true||circle|5|0|0||", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.series", "Points", "Data", "Rows: series|label|x|y|value|color. Legacy label,y rows map to the first series.", "s1|A|0|2|2|#317ED6\ns1|B|1|5|5|#317ED6\ns1|C|2|3|3|#317ED6\ns1|D|3|7|7|#317ED6", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_ENUM_PROP("data.seriesType", "Series Type", "Data", "Default series type.", "1", g_arrChartSeriesEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.xAxis", "X Axis", "Behavior", "X axis type.", "1", g_arrChartAxisEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.yAxis", "Y Axis", "Behavior", "Y axis type.", "1", g_arrChartAxisEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.barMode", "Bar Mode", "Behavior", "Bar chart mode.", "0", g_arrChartBarModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.barDirection", "Bar Direction", "Behavior", "Bar chart direction.", "0", g_arrChartBarDirectionEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.pieMode", "Pie Mode", "Behavior", "Pie chart mode.", "0", g_arrChartPieModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.legendVisible", "Legend Visible", "Behavior", "Show legend.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.tooltipVisible", "Tooltip Visible", "Behavior", "Show tooltip.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.animation", "Animation", "Behavior", "Enable animation.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.animationDuration", "Animation Duration", "Behavior", "Animation duration in seconds.", "0.25", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.lodThreshold", "LOD Threshold", "Behavior", "Point count threshold for level-of-detail rendering.", "800", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("viewport.useViewRange", "Use View Range", "Viewport", "Use an explicit visible data range.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("viewport.minX", "View Min X", "Viewport", "Visible minimum X.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("viewport.maxX", "View Max X", "Viewport", "Visible maximum X.", "10", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("viewport.minY", "View Min Y", "Viewport", "Visible minimum Y.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("viewport.maxY", "View Max Y", "Viewport", "Visible maximum Y.", "10", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("viewport.useBrushRange", "Use Brush Range", "Viewport", "Use an explicit brush/data selection range.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("viewport.brushMinX", "Brush Min X", "Viewport", "Brush minimum X.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("viewport.brushMaxX", "Brush Max X", "Viewport", "Brush maximum X.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("viewport.brushMinY", "Brush Min Y", "Viewport", "Brush minimum Y.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("viewport.brushMaxY", "Brush Max Y", "Viewport", "Brush maximum Y.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingLeft", "Padding Left", "Metrics", "Plot padding left.", "38", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingTop", "Padding Top", "Metrics", "Plot padding top.", "28", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingRight", "Padding Right", "Metrics", "Plot padding right.", "18", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingBottom", "Padding Bottom", "Metrics", "Plot padding bottom.", "34", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.pieInnerRadius", "Pie Inner Radius", "Metrics", "Pie inner radius ratio. 0 renders a normal pie.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Chart background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.plotColor", "Plot", "Appearance", "Plot area color.", "#F7FAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.gridColor", "Grid", "Appearance", "Grid color.", "#D9E3EF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.axisColor", "Axis", "Appearance", "Axis color.", "#6B7F95", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.tooltipColor", "Tooltip", "Appearance", "Tooltip background color.", "#233246EB", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.tooltipTextColor", "Tooltip Text", "Appearance", "Tooltip text color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.seriesColor", "Series Color", "Appearance", "First series color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrScrollBarProperties[] = {
	UI_DESIGN_PROP("value.min", "Minimum", "Value", "Minimum value.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.max", "Maximum", "Value", "Maximum value.", "100", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.value", "Value", "Value", "Current value.", "30", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.page", "Page", "Value", "Page size.", "20", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.smallStep", "Small Step", "Value", "Small step.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.largeStep", "Large Step", "Value", "Large step.", "10", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.orientation", "Orientation", "Behavior", "Scrollbar orientation.", "0", g_arrOrientationEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.mode", "Mode", "Behavior", "Scrollbar visual mode.", "0", g_arrScrollbarModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.buttonMode", "Button Mode", "Behavior", "Button display mode.", "0", g_arrScrollbarButtonsEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.thickness", "Thickness", "Metrics", "Track thickness.", "14", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minThumbSize", "Min Thumb Size", "Metrics", "Minimum thumb size.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.buttonSize", "Button Size", "Metrics", "Decrease/increase button size.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track", "Appearance", "Track color.", "#E4EBF3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.thumbColor", "Thumb", "Appearance", "Thumb color.", "#AAB8C8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeColor", "Active", "Appearance", "Active color.", "#6F89A6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled", "Appearance", "Disabled color.", "#CED7E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonColor", "Button", "Appearance", "Button color.", "#EFF5FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonIconColor", "Button Icon", "Appearance", "Button icon color.", "#34475D", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrSliderProperties[] = {
	UI_DESIGN_PROP("value.min", "Minimum", "Value", "Minimum value.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.max", "Maximum", "Value", "Maximum value.", "100", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.value", "Value", "Value", "Current value.", "35", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.step", "Step", "Value", "Small step.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.pageStep", "Page Step", "Value", "Large step.", "10", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.orientation", "Orientation", "Behavior", "Slider orientation.", "0", g_arrOrientationEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.trackSize", "Track Size", "Metrics", "Track thickness.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.knobSize", "Knob Size", "Metrics", "Knob diameter.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.trackRadius", "Track Radius", "Metrics", "Temporary rounded track radius retained for slider only.", "2", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track Color", "Appearance", "Track color.", "#D5DEE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.fillColor", "Fill Color", "Appearance", "Fill color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.knobColor", "Knob Color", "Appearance", "Knob fill color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.knobBorderColor", "Knob Border", "Appearance", "Knob border color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus Color", "Appearance", "Focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled Color", "Appearance", "Disabled color.", "#B7C3D0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrRangeSliderProperties[] = {
	UI_DESIGN_PROP("value.min", "Minimum", "Value", "Minimum value.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.max", "Maximum", "Value", "Maximum value.", "100", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.start", "Start", "Value", "Range start.", "25", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.end", "End", "Value", "Range end.", "75", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.step", "Step", "Value", "Small step.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.pageStep", "Page Step", "Value", "Large step.", "10", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.minInterval", "Min Interval", "Value", "Minimum range interval.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.maxInterval", "Max Interval", "Value", "Maximum range interval. 0 means unlimited.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.orientation", "Orientation", "Behavior", "Slider orientation.", "0", g_arrOrientationEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.trackSize", "Track Size", "Metrics", "Track thickness.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.knobSize", "Knob Size", "Metrics", "Knob diameter.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track Color", "Appearance", "Track color.", "#D5DEE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.fillColor", "Fill Color", "Appearance", "Fill color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.knobColor", "Knob Color", "Appearance", "Knob fill color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.knobBorderColor", "Knob Border", "Appearance", "Knob border color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus Color", "Appearance", "Focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled Color", "Appearance", "Disabled color.", "#B7C3D0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrPageProperties[] = {
	UI_DESIGN_PROP("data.pageCount", "Page Count", "Data", "Total pages.", "12", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.useTotal", "Use Total", "Behavior", "Compute page count from total count and page size.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.totalCount", "Total Count", "Data", "Total item count used when Use Total is enabled.", "120", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.pageSize", "Page Size", "Data", "Page size used when Use Total is enabled.", "10", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.current", "Current Page", "Data", "Current page index.", "1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.windowSize", "Window Size", "Data", "Visible page window size.", "5", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.first", "First Text", "Text", "First page text.", "<<", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.last", "Last Text", "Text", "Last page text.", ">>", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.prev", "Prev Text", "Text", "Previous page text.", "<", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.next", "Next Text", "Text", "Next page text.", ">", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.itemHeight", "Item Height", "Metrics", "Item height.", "28", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.pageWidth", "Page Width", "Metrics", "Page item width.", "28", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.textWidth", "Text Width", "Metrics", "Text item width.", "42", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.navWidth", "Nav Width", "Metrics", "Navigation item width.", "36", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.ellipsisWidth", "Ellipsis Width", "Metrics", "Ellipsis item width.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover color.", "#EAF3FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeColor", "Active", "Appearance", "Active color.", "#D9EAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.currentColor", "Current", "Appearance", "Current page color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.currentTextColor", "Current Text", "Appearance", "Current page text color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrCarouselProperties[] = {
	UI_DESIGN_PROP("data.pageCount", "Page Count", "Data", "Carousel page count.", "3", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.current", "Current", "Data", "Current page index.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.loop", "Loop", "Behavior", "Loop pages.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.autoPlay", "Auto Play", "Behavior", "Enable autoplay.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.autoInterval", "Auto Interval", "Behavior", "Autoplay interval in seconds.", "3", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.showIndicators", "Indicators", "Behavior", "Show page indicators.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.arrowsOnHover", "Arrows On Hover", "Behavior", "Only show arrows while hovered.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.arrowSize", "Arrow Size", "Metrics", "Arrow button size.", "32", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.indicatorSize", "Indicator Size", "Metrics", "Indicator dot size.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.indicatorGap", "Indicator Gap", "Metrics", "Indicator gap.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.indicatorBottom", "Indicator Bottom", "Metrics", "Indicator bottom offset.", "12", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#F7FAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.arrowColor", "Arrow", "Appearance", "Arrow background color.", "#5A6D82AA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.arrowHoverColor", "Arrow Hover", "Appearance", "Arrow hover color.", "#34475DCC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.arrowTextColor", "Arrow Text", "Appearance", "Arrow text color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.indicatorColor", "Indicator", "Appearance", "Indicator color.", "#AAB8C8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.indicatorActiveColor", "Indicator Active", "Appearance", "Active indicator color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.indicatorHoverColor", "Indicator Hover", "Appearance", "Indicator hover color.", "#6DA4E8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrComboBoxProperties[] = {
	UI_DESIGN_PROP("data.items", "Items", "Data", "Rows: text|enabled|selected|value|icon|separator. Plain text rows are supported.", "Item 1|true|true|1|0|false\nItem 2|true|false|2|0|false\nItem 3|false|false|3|0|false", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.menuTitles", "Menu Titles", "Data", "Rows: command|title for editable input menu. Commands: undo, redo, cut, copy, paste, delete, selectAll.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.selected", "Selected Index", "Data", "Selected item index.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.selectedValue", "Selected Value", "Data", "Selected item value when selecting by value.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.inputText", "Input Text", "Text", "Editable combobox text.", "", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.placeholder", "Placeholder", "Text", "Editable combobox placeholder.", "Select or type", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.maxLength", "Max Length", "Text", "Maximum editable text length. 0 means unlimited.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.useValue", "Use Value", "Behavior", "Select by item value instead of item index.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.mode", "Mode", "Behavior", "Select or edit mode.", "0", g_arrComboModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.open", "Open", "Behavior", "Preview the popup open state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.popupPlacement", "Popup Placement", "Behavior", "Popup placement.", "0", g_arrPopupPlacementEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.itemHeight", "Item Height", "Metrics", "Popup item height.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.popupHeight", "Popup Height", "Metrics", "Preferred popup height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.popupMaxHeight", "Popup Max Height", "Metrics", "Maximum popup height.", "240", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text Color", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBackgroundColor", "Hover Background", "Appearance", "Hover background color.", "#F0F6FD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.openBackgroundColor", "Open Background", "Appearance", "Open background color.", "#E7F1FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledBackgroundColor", "Disabled Back", "Appearance", "Disabled background color.", "#EEF2F7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBorderColor", "Hover Border", "Appearance", "Hover border color.", "#7EA4CF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusBorderColor", "Focus Border", "Appearance", "Focus border color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.arrowColor", "Arrow", "Appearance", "Arrow color.", "#34475D", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledArrowColor", "Disabled Arrow", "Appearance", "Disabled arrow color.", "#9AA8BA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonColor", "Button", "Appearance", "Drop button color.", "#ECF6FD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonHoverColor", "Button Hover", "Appearance", "Drop button hover color.", "#DCEFFB", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonOpenColor", "Button Open", "Appearance", "Drop button open color.", "#CFE5F7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupPanelColor", "Popup Panel", "Appearance", "Popup panel color.", "#FAFDFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupBorderColor", "Popup Border", "Appearance", "Popup border color.", "#7AA4CA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupShadowColor", "Popup Shadow", "Appearance", "Popup shadow color.", "#2C46602E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupHoverColor", "Popup Hover", "Appearance", "Popup item hover color.", "#367DBE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupTextColor", "Popup Text", "Appearance", "Popup item text color.", "#1C3C5E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupHoverTextColor", "Popup Hover Text", "Appearance", "Popup hover text color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupDisabledTextColor", "Popup Disabled Text", "Appearance", "Popup disabled text color.", "#8E98A6D2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupSeparatorColor", "Popup Separator", "Appearance", "Popup separator color.", "#CADAE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrListViewProperties[] = {
	UI_DESIGN_PROP("data.items", "Items", "Data", "Rows: text|enabled|selected. Plain text rows are supported.", "Item 1|true|true\nItem 2|true|false\nItem 3|false|false", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.selected", "Selected Index", "Data", "Selected item index.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.scrollY", "Scroll Y", "Viewport", "Initial vertical scroll offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.selectionMode", "Selection Mode", "Behavior", "Selection mode.", "0", g_arrSelectionEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.repeatSelect", "Repeat Select", "Behavior", "Notify repeated selection.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.scrollbarMode", "Scrollbar Mode", "Behavior", "Scrollbar visual mode.", "0", g_arrScrollbarModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.itemHeight", "Item Height", "Metrics", "Item row height.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.padding", "Padding", "Metrics", "Item horizontal padding.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus ring color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.rowColor", "Row", "Appearance", "Row color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover row color.", "#EAF3FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectedColor", "Selected", "Appearance", "Selected row color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track", "Appearance", "Scrollbar track color.", "#E4EBF3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.thumbColor", "Thumb", "Appearance", "Scrollbar thumb color.", "#AAB8C8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarHoverColor", "Scrollbar Hover", "Appearance", "Scrollbar hover color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarActiveColor", "Scrollbar Active", "Appearance", "Scrollbar active color.", "#6F89A6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarFocusColor", "Scrollbar Focus", "Appearance", "Scrollbar focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarDisabledColor", "Scrollbar Disabled", "Appearance", "Scrollbar disabled color.", "#CED7E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrTreeViewProperties[] = {
	UI_DESIGN_PROP("data.nodes", "Nodes", "Data", "Rows: id|parent|depth|text|expanded|enabled|icon|checkbox|checked. Indented text rows are supported.", "1|-1|0|Root|true|true|true|false|false\n2|1|1|Child|true|true|true|true|false", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.selected", "Selected Id", "Data", "Selected node id.", "1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.scrollY", "Scroll Y", "Viewport", "Initial vertical scroll offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.scrollbarMode", "Scrollbar Mode", "Behavior", "Scrollbar visual mode.", "0", g_arrScrollbarModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.itemHeight", "Item Height", "Metrics", "Tree row height.", "23", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.indent", "Indent", "Metrics", "Indent per level.", "18", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.padding", "Padding", "Metrics", "Item horizontal padding.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus ring color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.rowColor", "Row", "Appearance", "Row color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover row color.", "#EAF3FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectedColor", "Selected", "Appearance", "Selected row color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.expanderColor", "Expander", "Appearance", "Expander color.", "#52657A", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.iconColor", "Icon", "Appearance", "Icon color.", "#7A8EA5", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.checkColor", "Check", "Appearance", "Check color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track", "Appearance", "Scrollbar track color.", "#E4EBF3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.thumbColor", "Thumb", "Appearance", "Scrollbar thumb color.", "#AAB8C8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarHoverColor", "Scrollbar Hover", "Appearance", "Scrollbar hover color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarActiveColor", "Scrollbar Active", "Appearance", "Scrollbar active color.", "#6F89A6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarFocusColor", "Scrollbar Focus", "Appearance", "Scrollbar focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarDisabledColor", "Scrollbar Disabled", "Appearance", "Scrollbar disabled color.", "#CED7E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrTableViewProperties[] = {
	UI_DESIGN_PROP("data.columns", "Columns", "Data", "Column definitions: title|width|type|id|visible|resizable|align|min|max|background|text|grid.", "Name|130|text|name|true|true|left|48|0|#EEF5FC|#233246|#D0DBE8\nWidth|82|float|width|true|true|right|48|0|#EEF5FC|#233246|#D0DBE8\nHeight|82|float|height|true|true|right|48|0|#EEF5FC|#233246|#D0DBE8", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.rows", "Rows", "Data", "Row data.", "Button,118,32\nInput,180,30\nTable,320,180", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.rowStyles", "Row Styles", "Data", "Row style rows: row|height|selected|disabled|background|text|grid.", "0|24|true|false|#D8EAFE|#233246|#D0DBE8", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.cellStyles", "Cell Styles", "Data", "Cell style rows: row|column|type|tooltip|rowSpan|colSpan|disabled|editing|invalid|dirty|background|text|grid.", "0|0|text|Primary label|1|1|false|false|false|false|#FFFFFF|#233246|#D0DBE8", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.merges", "Merged Cells", "Data", "Merged cell regions: row|column|rowSpan|colSpan|enabled.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_ENUM_PROP("behavior.selectionMode", "Selection Mode", "Behavior", "Selection mode.", "1", g_arrTableSelectionEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.scrollbarMode", "Scrollbar Mode", "Behavior", "Scrollbar visual mode.", "0", g_arrScrollbarModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.selectedRow", "Selected Row", "Data", "Selected row index.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.selectedColumn", "Selected Column", "Data", "Selected column index.", "-1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.offsetX", "Offset X", "Viewport", "Initial horizontal scroll offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.offsetY", "Offset Y", "Viewport", "Initial vertical scroll offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.columnWidth", "Column Width", "Metrics", "Default column width.", "100", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.rowHeight", "Row Height", "Metrics", "Default row height.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.headerHeight", "Header Height", "Metrics", "Header row height.", "26", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.headerColor", "Header", "Appearance", "Header background color.", "#EEF5FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.headerTextColor", "Header Text", "Appearance", "Header text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.rowColor", "Row", "Appearance", "Row background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.altRowColor", "Alt Row", "Appearance", "Alternating row background color.", "#F9FCFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover row or cell color.", "#E7F3FD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectedColor", "Selected", "Appearance", "Selected cell or row color.", "#D8EAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled Row", "Appearance", "Disabled row background color.", "#EEF4FA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.gridColor", "Grid", "Appearance", "Grid line color.", "#D0DBE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus ring color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track", "Appearance", "Scrollbar track color.", "#E4EBF3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.thumbColor", "Thumb", "Appearance", "Scrollbar thumb color.", "#AAB8C8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarHoverColor", "Scrollbar Hover", "Appearance", "Scrollbar hover color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarActiveColor", "Scrollbar Active", "Appearance", "Scrollbar active color.", "#6F89A6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarFocusColor", "Scrollbar Focus", "Appearance", "Scrollbar focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarDisabledColor", "Scrollbar Disabled", "Appearance", "Scrollbar disabled color.", "#CED7E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrTableGridProperties[] = {
	UI_DESIGN_PROP("data.columns", "Columns", "Data", "Column definitions: title|width|type|id|visible|resizable|align|min|max|background|text|grid.", "Name|130|text|name|true|true|left|48|0|#EEF5FC|#233246|#D0DBE8\nWidth|82|float|width|true|true|right|48|0|#EEF5FC|#233246|#D0DBE8\nHeight|82|float|height|true|true|right|48|0|#EEF5FC|#233246|#D0DBE8", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.rows", "Rows", "Data", "Editable row data.", "Button,118,32\nInput,180,30\nTable,320,180", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.rowStyles", "Row Styles", "Data", "Row style rows: row|height|selected|disabled|background|text|grid.", "1|24|false|false|#F9FCFF|#233246|#D0DBE8", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.cellStyles", "Cell Styles", "Data", "Cell style rows: row|column|type|tooltip|rowSpan|colSpan|disabled|editing|invalid|dirty|background|text|grid.", "1|1|float|Editable width|1|1|false|true|false|true|#FFFFFF|#233246|#D0DBE8", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.merges", "Merged Cells", "Data", "Merged cell regions: row|column|rowSpan|colSpan|enabled.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.editorConfigs", "Editor Configs", "Data", "Rows: row|column|type|options|min|max|step|precision|nullable|alpha|actionText|fileFilter|dateMode|showSecond|dateFormat|dateMin|dateMax|defaultSpan|rangeSeparator|palette|enumValues|enumEnabled|enumSelected|enumUseValue|enumSelectedValue. Use -1 as wildcard.", "-1|1|float||0|1000|1|0", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_ENUM_PROP("behavior.selectionMode", "Selection Mode", "Behavior", "Selection mode.", "1", g_arrTableSelectionEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.scrollbarMode", "Scrollbar Mode", "Behavior", "Scrollbar visual mode.", "0", g_arrScrollbarModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.editMode", "Edit Mode", "Behavior", "Table grid edit mode.", "1", g_arrTableGridEditModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.selectedRow", "Selected Row", "Data", "Selected row index.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.selectedColumn", "Selected Column", "Data", "Selected column index.", "-1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.offsetX", "Offset X", "Viewport", "Initial horizontal scroll offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.offsetY", "Offset Y", "Viewport", "Initial vertical scroll offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.columnWidth", "Column Width", "Metrics", "Default column width.", "100", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.rowHeight", "Row Height", "Metrics", "Default row height.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.headerHeight", "Header Height", "Metrics", "Header row height.", "26", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.headerColor", "Header", "Appearance", "Header background color.", "#EEF5FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.headerTextColor", "Header Text", "Appearance", "Header text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.rowColor", "Row", "Appearance", "Row background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.altRowColor", "Alt Row", "Appearance", "Alternating row background color.", "#F9FCFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover row or cell color.", "#E7F3FD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectedColor", "Selected", "Appearance", "Selected cell or row color.", "#D8EAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled Row", "Appearance", "Disabled row background color.", "#EEF4FA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.gridColor", "Grid", "Appearance", "Grid line color.", "#D0DBE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus ring color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track", "Appearance", "Scrollbar track color.", "#E4EBF3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.thumbColor", "Thumb", "Appearance", "Scrollbar thumb color.", "#AAB8C8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarHoverColor", "Scrollbar Hover", "Appearance", "Scrollbar hover color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarActiveColor", "Scrollbar Active", "Appearance", "Scrollbar active color.", "#6F89A6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarFocusColor", "Scrollbar Focus", "Appearance", "Scrollbar focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarDisabledColor", "Scrollbar Disabled", "Appearance", "Scrollbar disabled color.", "#CED7E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrQrCodeProperties[] = {
	UI_DESIGN_PROP("data.value", "Value", "Data", "QR code value.", "https://xui.local", XUI_TABLE_CELL_TYPE_TEXTAREA, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.iconSource", "Icon Source", "Data", "Optional center icon image.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.foregroundColor", "Foreground", "Appearance", "Foreground color.", "#111827", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.padding", "Padding", "Metrics", "QR quiet-zone padding.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconSize", "Icon Size", "Metrics", "Center icon size. 0 uses automatic sizing.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.x", "Icon Source X", "Icon", "Icon source rectangle x. 0 with zero width keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.y", "Icon Source Y", "Icon", "Icon source rectangle y. 0 with zero height keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.w", "Icon Source W", "Icon", "Icon source rectangle width. 0 keeps the full image width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.h", "Icon Source H", "Icon", "Icon source rectangle height. 0 keeps the full image height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.minVersion", "Min Version", "Behavior", "Minimum QR version.", "1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.maxVersion", "Max Version", "Behavior", "Maximum QR version.", "10", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrImageProperties[] = {
	UI_DESIGN_PROP("data.source", "Source", "Data", "Image file source.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("appearance.tintColor", "Tint Color", "Appearance", "Image tint color.", "#FFFFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.mode", "Mode", "Behavior", "Image scaling mode.", "2", g_arrImageModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.alignX", "Align X", "Behavior", "Horizontal image alignment.", "1", g_arrAlignEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.alignY", "Align Y", "Behavior", "Vertical image alignment.", "1", g_arrAlignEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("source.x", "Source X", "Source", "Source rectangle x. 0 with zero width keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("source.y", "Source Y", "Source", "Source rectangle y. 0 with zero height keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("source.w", "Source W", "Source", "Source rectangle width. 0 keeps the full image width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("source.h", "Source H", "Source", "Source rectangle height. 0 keeps the full image height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("custom.x", "Custom X", "Custom Rect", "Custom destination x used by Custom mode.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("custom.y", "Custom Y", "Custom Rect", "Custom destination y used by Custom mode.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("custom.w", "Custom W", "Custom Rect", "Custom destination width used by Custom mode.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("custom.h", "Custom H", "Custom Rect", "Custom destination height used by Custom mode.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrBreadcrumbProperties[] = {
	UI_DESIGN_PROP("data.items", "Items", "Data", "Rows: text|clickable|value.", "Home|true|1\nProject|true|2\nEditor|false|3", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("text.separator", "Separator", "Text", "Separator text.", "/", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.separatorIconSource", "Separator Icon", "Images", "Optional separator icon image.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("separatorIcon.x", "Icon X", "Images", "Separator icon source x.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("separatorIcon.y", "Icon Y", "Images", "Separator icon source y.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("separatorIcon.w", "Icon W", "Images", "Separator icon source width. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("separatorIcon.h", "Icon H", "Images", "Separator icon source height. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text Color", "Appearance", "Normal item text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverTextColor", "Hover Text", "Appearance", "Hover item text color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeTextColor", "Active Text", "Appearance", "Active item text color.", "#1F5FA8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled item text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.separatorColor", "Separator Color", "Appearance", "Separator color.", "#6B7F95", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#00000000", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.separatorIconSize", "Icon Size", "Metrics", "Separator icon draw size.", "12", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.gap", "Gap", "Metrics", "Gap between items and separators.", "6", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingX", "Padding X", "Metrics", "Horizontal padding.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingY", "Padding Y", "Metrics", "Vertical padding.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrCheckCardProperties[] = {
	UI_DESIGN_PROP("checked", "Checked", "Behavior", "Checked state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minWidth", "Min Width", "Metrics", "Minimum card width.", "96", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minHeight", "Min Height", "Metrics", "Minimum card height.", "56", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingLeft", "Padding Left", "Metrics", "Content padding left.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingTop", "Padding Top", "Metrics", "Content padding top.", "14", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingRight", "Padding Right", "Metrics", "Content padding right.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingBottom", "Padding Bottom", "Metrics", "Content padding bottom.", "14", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Normal border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.checkedBorderWidth", "Checked Border", "Metrics", "Checked border width.", "2", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.cornerSize", "Corner Size", "Metrics", "Check corner marker size.", "18", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.focusWidth", "Focus Width", "Metrics", "Focus outline width.", "2", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Normal background.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBackgroundColor", "Hover Background", "Appearance", "Hover background.", "#F0F6FD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeBackgroundColor", "Active Background", "Appearance", "Active background.", "#E2F0FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.checkedBackgroundColor", "Checked Background", "Appearance", "Checked background.", "#F4F9FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Normal border.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBorderColor", "Hover Border", "Appearance", "Hover border.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.checkedBorderColor", "Checked Border", "Appearance", "Checked border.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledBorderColor", "Disabled Border", "Appearance", "Disabled border color.", "#D6DBE1A0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.cornerColor", "Corner", "Appearance", "Corner marker color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.checkColor", "Check", "Appearance", "Check mark color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus outline color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrRadioGroupProperties[] = {
	UI_DESIGN_ENUM_PROP("behavior.orientation", "Orientation", "Behavior", "Option orientation.", "1", g_arrOrientationEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.useBuiltinAtlas", "Builtin Atlas", "Behavior", "Use built-in radio indicator atlas.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.indicatorSource", "Indicator Source", "Data", "Optional indicator image atlas shared by generated radio options.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.selected", "Selected Index", "Data", "Selected option index.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.options", "Options", "Data", "Rows: text|enabled|checked. Plain text rows are supported.", "Option A|true|true\nOption B|true|false\nOption C|false|false", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_ENUM_PROP("text.flags", "Text Align", "Text", "Option text alignment.", "272", g_arrTextAlignEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.gap", "Option Gap", "Metrics", "Gap between options.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.indicatorSize", "Indicator Size", "Metrics", "Indicator glyph size.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.indicatorTextGap", "Indicator Text Gap", "Metrics", "Gap between indicator and option text.", "7", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("unchecked.x", "Unchecked X", "Indicator Image", "Unchecked indicator source x. 0 with zero width keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("unchecked.y", "Unchecked Y", "Indicator Image", "Unchecked indicator source y. 0 with zero height keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("unchecked.w", "Unchecked W", "Indicator Image", "Unchecked indicator source width. 0 keeps the full image width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("unchecked.h", "Unchecked H", "Indicator Image", "Unchecked indicator source height. 0 keeps the full image height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("checked.x", "Checked X", "Indicator Image", "Checked indicator source x. 0 with zero width keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("checked.y", "Checked Y", "Indicator Image", "Checked indicator source y. 0 with zero height keeps the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("checked.w", "Checked W", "Indicator Image", "Checked indicator source width. 0 keeps the full image width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("checked.h", "Checked H", "Indicator Image", "Checked indicator source height. 0 keeps the full image height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text Color", "Appearance", "Normal option text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled option text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.accentColor", "Accent Color", "Appearance", "Checked accent color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border Color", "Appearance", "Indicator border color.", "#8DA5C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBorderColor", "Hover Border", "Appearance", "Hover indicator border color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus Color", "Appearance", "Focus ring color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrVirtualJoystickProperties[] = {
	UI_DESIGN_PROP("value.x", "X", "Value", "Preview X value.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.y", "Y", "Value", "Preview Y value.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.channels", "Channels", "Value", "Rows: channel|pressed|value. Empty uses X/Y preview value.", "right|true|0.65\nup|true|0.25", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("metrics.radius", "Radius", "Metrics", "Base radius.", "58", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.knobSize", "Knob Size", "Metrics", "Knob size.", "34", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.deadZone", "Dead Zone", "Metrics", "Dead-zone ratio.", "0.1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.useBuiltinAtlas", "Builtin Atlas", "Behavior", "Use built-in joystick atlas.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.baseColor", "Base", "Appearance", "Base color.", "#E2EAF3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.baseActiveColor", "Base Active", "Appearance", "Active base color.", "#CFE3FA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.knobColor", "Knob", "Appearance", "Knob color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.knobActiveColor", "Knob Active", "Appearance", "Active knob color.", "#F7FBFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.rippleColor", "Ripple", "Appearance", "Ripple color.", "#317ED633", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled", "Appearance", "Disabled joystick color.", "#B7C3D0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.baseSource", "Base Image", "Images", "Optional base image.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.baseActiveSource", "Base Active Image", "Images", "Optional active base image.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.knobSource", "Knob Image", "Images", "Optional knob image.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.knobActiveSource", "Knob Active Image", "Images", "Optional active knob image.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.rippleSource", "Ripple Image", "Images", "Optional ripple image.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("base.x", "Base X", "Image Source", "Base source x.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("base.y", "Base Y", "Image Source", "Base source y.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("base.w", "Base W", "Image Source", "Base source width. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("base.h", "Base H", "Image Source", "Base source height. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("baseActive.x", "Base Active X", "Image Source", "Active base source x.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("baseActive.y", "Base Active Y", "Image Source", "Active base source y.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("baseActive.w", "Base Active W", "Image Source", "Active base source width. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("baseActive.h", "Base Active H", "Image Source", "Active base source height. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("knob.x", "Knob X", "Image Source", "Knob source x.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("knob.y", "Knob Y", "Image Source", "Knob source y.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("knob.w", "Knob W", "Image Source", "Knob source width. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("knob.h", "Knob H", "Image Source", "Knob source height. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("knobActive.x", "Knob Active X", "Image Source", "Active knob source x.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("knobActive.y", "Knob Active Y", "Image Source", "Active knob source y.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("knobActive.w", "Knob Active W", "Image Source", "Active knob source width. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("knobActive.h", "Knob Active H", "Image Source", "Active knob source height. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("ripple.x", "Ripple X", "Image Source", "Ripple source x.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("ripple.y", "Ripple Y", "Image Source", "Ripple source y.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("ripple.w", "Ripple W", "Image Source", "Ripple source width. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("ripple.h", "Ripple H", "Image Source", "Ripple source height. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrInventoryGridProperties[] = {
	UI_DESIGN_PROP("data.slots", "Slots", "Data", "Rows: text|count|hotkey|flags|qualityColor|itemId|maxCount|slotId|cooldown|durability|slotType|itemType|iconTint|iconSource|iconX|iconY|iconW|iconH|animationFlags|animationScale|animationTint.", "Sword|1|1||#317ED6|1001|1|1|0|1|0|0|#FFFFFF||||||0|1|#FFFFFF\nPotion|8|2||#2D9D6E|1002|20|2|0|1|0|0|#FFFFFF||||||0|1|#FFFFFF\nShield|1|3|equipped|#F5A623|1003|1|3|0|0.7|0|0|#FFFFFF||||||0|1|#FFFFFF", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.selectedSlots", "Selected Slots", "Data", "Rows: slot|selected. Empty selects Current Slot when selection is enabled.", "0|true", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.slotCount", "Slot Count", "Data", "Slot count.", "24", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.current", "Current Slot", "Data", "Current slot index.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.columns", "Columns", "Metrics", "Fixed column count. 0 uses automatic layout.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.slotSize", "Slot Size", "Metrics", "Slot size.", "42", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.slotGap", "Slot Gap", "Metrics", "Gap between slots.", "6", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.padding", "Padding", "Metrics", "Grid padding.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconPadding", "Icon Padding", "Metrics", "Icon padding.", "5", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Slot border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.wheelStep", "Wheel Step", "Metrics", "Mouse wheel step.", "48", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.dragThreshold", "Drag Threshold", "Metrics", "Drag start threshold.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#17202B", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.slotColor", "Slot", "Appearance", "Slot fill color.", "#253245", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.emptyColor", "Empty", "Appearance", "Empty slot color.", "#1C2634", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover color.", "#33445A", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeColor", "Active", "Appearance", "Active slot color.", "#3B5270", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectedColor", "Selected", "Appearance", "Selected color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled", "Appearance", "Disabled slot color.", "#2E3743", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.lockedColor", "Locked", "Appearance", "Locked slot color.", "#50373E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#52657A", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.qualityColor", "Quality", "Appearance", "Default quality color.", "#F5A623", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Slot text color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.mutedTextColor", "Muted Text", "Appearance", "Muted text color.", "#ACBACB", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.countColor", "Count", "Appearance", "Item count color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hotkeyColor", "Hotkey", "Appearance", "Hotkey text color.", "#C4CFDE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.cooldownColor", "Cooldown", "Appearance", "Cooldown overlay color.", "#00000070", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.durabilityColor", "Durability", "Appearance", "Durability bar color.", "#2D9D6E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.dragColor", "Drag", "Appearance", "Drag preview color.", "#317ED678", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.dropColor", "Drop", "Appearance", "Drop target color.", "#2D9D6E78", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.selectionMode", "Selection Mode", "Behavior", "Inventory selection mode.", "1", g_arrInventorySelectionEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.tooltipVisible", "Tooltip", "Behavior", "Show tooltips.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.previewSplitOpen", "Preview Split", "Behavior", "Open split popup preview for a stackable slot.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.previewSplitSlot", "Split Slot", "Behavior", "Slot used by split popup preview.", "1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.previewSplitX", "Split X", "Behavior", "Preview popup x anchor.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.previewSplitY", "Split Y", "Behavior", "Preview popup y anchor.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.wrapRows", "Wrap Rows", "Gamepad", "Gamepad navigation wraps within a row.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.wrapColumns", "Wrap Columns", "Gamepad", "Gamepad navigation wraps within a column.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.skipDisabled", "Skip Disabled", "Gamepad", "Skip disabled slots during navigation.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.selectOnMove", "Select On Move", "Gamepad", "Select the target slot while navigating.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.multiSelectModifiers", "Multi Modifiers", "Gamepad", "Respect modifier keys for multi-select.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.acceptButton", "Accept", "Gamepad", "Accept button id.", "5", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.cancelButton", "Cancel", "Gamepad", "Cancel button id.", "6", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.contextButton", "Context", "Gamepad", "Context button id.", "7", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.leftButton", "Left", "Gamepad", "Left button id.", "1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.rightButton", "Right", "Gamepad", "Right button id.", "2", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.upButton", "Up", "Gamepad", "Up button id.", "3", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.downButton", "Down", "Gamepad", "Down button id.", "4", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.pageUpButton", "Page Up", "Gamepad", "Page up button id.", "8", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.pageDownButton", "Page Down", "Gamepad", "Page down button id.", "9", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.homeButton", "Home", "Gamepad", "Home button id.", "10", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("gamepad.endButton", "End", "Gamepad", "End button id.", "11", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrTerminalProperties[] = {
	UI_DESIGN_PROP("data.text", "Text", "Data", "Preview terminal text.", "$ xui_uidesign\nready", XUI_TABLE_CELL_TYPE_TEXTAREA, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.palette", "Palette", "Data", "ANSI palette rows: index|color.", UI_DESIGN_TERMINAL_DEFAULT_PALETTE, XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.selection", "Selection", "Data", "Row: anchorLine|anchorColumn|endLine|endColumn. Lines and columns are 1-based; empty clears selection.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.menuTitles", "Menu Titles", "Data", "Rows: command|title. Commands: copy, paste, selectAll, clearScreen, clearScrollback, find.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.findText", "Find Text", "Data", "Initial terminal search text. Empty clears search.", "", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.columns", "Columns", "Metrics", "Terminal columns.", "80", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.rows", "Rows", "Metrics", "Terminal rows.", "24", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.scrollback", "Scrollback", "Metrics", "Scrollback line limit.", "1000", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.parseBudget", "Parse Budget", "Metrics", "Bytes parsed per update.", "4096", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.cellWidth", "Cell Width", "Metrics", "Cell width override.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.cellHeight", "Cell Height", "Metrics", "Cell height override.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.padding", "Padding", "Metrics", "Terminal padding.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.scrollX", "Scroll X", "Value", "Initial horizontal scroll when Use Scroll is enabled.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.scrollY", "Scroll Y", "Value", "Initial vertical scroll when Use Scroll is enabled.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Terminal background.", "#101820", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.foregroundColor", "Foreground", "Appearance", "Terminal foreground.", "#D7E4F2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.cursorColor", "Cursor", "Appearance", "Cursor color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectionColor", "Selection", "Appearance", "Selection color.", "#317ED680", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectionTextColor", "Selection Text", "Appearance", "Selected text color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.searchHighlightColor", "Search Highlight", "Appearance", "Search highlight color.", "#FFC5426E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.linkHoverColor", "Link Hover", "Appearance", "Hovered link color.", "#569CD6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.bracketedPaste", "Bracketed Paste", "Behavior", "Enable bracketed paste.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.ligaturesEnabled", "Ligatures", "Behavior", "Enable font ligatures.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.findCaseSensitive", "Find Case", "Behavior", "Case-sensitive terminal search.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.findBackward", "Find Backward", "Behavior", "Start terminal search from the bottom.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.useScroll", "Use Scroll", "Behavior", "Apply Scroll X/Y after rendering preview text and search.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrSplitLayoutProperties[] = {
	UI_DESIGN_ENUM_PROP("behavior.orientation", "Layout Direction", "Behavior", "Pane layout direction.", "1", g_arrSplitLayoutDirectionEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.paneCount", "Pane Count", "Data", "Pane count.", "2", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.panes", "Panes", "Data", "Rows: title|mode|weight|fixed|min|max.", "Pane 1|grow|1|0|40|0\nPane 2|grow|1|0|40|0", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_ENUM_PROP("behavior.pane0Mode", "Pane 0 Mode", "Behavior", "First pane sizing mode.", "0", g_arrSplitPaneModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.pane1Mode", "Pane 1 Mode", "Behavior", "Second pane sizing mode.", "0", g_arrSplitPaneModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.dividerSize", "Divider Size", "Metrics", "Layout divider size.", "6", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.dividerVisualSize", "Divider Visual", "Metrics", "Visible divider size.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.dividerHitSize", "Divider Hit", "Metrics", "Divider hit target size.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.shadowDrag", "Shadow Drag", "Behavior", "Use shadow drag preview.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.dividerColor", "Divider", "Appearance", "Divider color.", "#CAD6E4", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover divider color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeColor", "Active", "Appearance", "Active divider color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.shadowColor", "Shadow", "Appearance", "Shadow drag color.", "#317ED633", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrTabsProperties[] = {
	UI_DESIGN_PROP("data.items", "Pages", "Data", "Rows: title|enabled|dirty|iconSource|iconX|iconY|iconW|iconH.", "Design|true|false|||||\nPreview|true|false|||||\nCode|true|true|||||", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.selected", "Selected", "Data", "Selected tab index.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.scrollX", "Scroll X", "Value", "Horizontal tab strip scroll.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.placement", "Placement", "Behavior", "Tab placement.", "0", g_arrTabsPlacementEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.scrollable", "Scrollable", "Behavior", "Enable tab scrolling.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.closeButtons", "Close Buttons", "Behavior", "Show close buttons.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.tabWidth", "Tab Width", "Metrics", "Tab width.", "92", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.tabHeight", "Tab Height", "Metrics", "Tab height.", "30", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Tabs background.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.tabColor", "Tab", "Appearance", "Tab color.", "#EAF1F8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover tab color.", "#F4F9FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeColor", "Active", "Appearance", "Active tab color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled", "Appearance", "Disabled tab color.", "#EEF2F7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Tab text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeTextColor", "Active Text", "Appearance", "Active tab text.", "#1F5FA8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Tab frame border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.clientColor", "Client", "Appearance", "Tab client color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrAccordionProperties[] = {
	UI_DESIGN_PROP("data.sections", "Sections", "Data", "Rows: title|expanded|disabled|id.", "General|true|false|1\nAppearance|false|false|2\nLayout|false|false|3", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_ENUM_PROP("behavior.mode", "Mode", "Behavior", "Accordion selection mode.", "0", g_arrAccordionModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.headerHeight", "Header Height", "Metrics", "Header height.", "28", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.spacing", "Spacing", "Metrics", "Section spacing.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.contentPadding", "Content Padding", "Metrics", "Client padding.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.headerColor", "Header", "Appearance", "Header color.", "#EAF1F8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover color.", "#F4F9FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.expandedColor", "Expanded", "Appearance", "Expanded header color.", "#D9EAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.contentColor", "Content", "Appearance", "Content color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Header text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeTextColor", "Active Text", "Appearance", "Expanded header text color.", "#1F5FA8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled header text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrWindowProperties[] = {
	UI_DESIGN_PROP("text.title", "Title", "Text", "Window title.", "Window", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.open", "Open", "Behavior", "Open state.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.topMost", "Top Most", "Behavior", "Top-most state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.showTitleBar", "Title Bar", "Behavior", "Show title bar.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.movable", "Movable", "Behavior", "Allow moving.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.dragAnywhere", "Drag Anywhere", "Behavior", "Drag from client area.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.resizable", "Resizable", "Behavior", "Allow resizing.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.resizeEdges", "Resize Edges", "Behavior", "Resize edge bitmask: left=1, top=2, right=4, bottom=8.", "15", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.showCollapse", "Collapse", "Behavior", "Show collapse button.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.showMaximize", "Maximize", "Behavior", "Show maximize button.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.showClose", "Close", "Behavior", "Show close button.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.collapsed", "Collapsed", "Behavior", "Collapsed state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.maximized", "Maximized", "Behavior", "Maximized state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.iconSource", "Icon Source", "Data", "Window title icon image path.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.x", "Icon X", "Data", "Icon source rectangle X.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.y", "Icon Y", "Data", "Icon source rectangle Y.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.w", "Icon W", "Data", "Icon source rectangle width. Zero uses default source.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.h", "Icon H", "Data", "Icon source rectangle height. Zero uses default source.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.titleBarHeight", "Title Height", "Metrics", "Title bar height.", "30", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.resizeGrip", "Resize Grip", "Metrics", "Resize grip size.", "6", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.buttonSize", "Button Size", "Metrics", "Title button size.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconSize", "Icon Size", "Metrics", "Title icon size.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minWidth", "Min Width", "Metrics", "Minimum width.", "120", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minHeight", "Min Height", "Metrics", "Minimum height.", "80", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Outer background.", "#F7FAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.clientColor", "Client", "Appearance", "Client color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.titleBarColor", "Title Bar", "Appearance", "Title bar color.", "#DDEBFA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.inactiveTitleBarColor", "Inactive Title", "Appearance", "Inactive title bar color.", "#ECF5FB", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.titleTextColor", "Title Text", "Appearance", "Title text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.inactiveTitleTextColor", "Inactive Text", "Appearance", "Inactive title text color.", "#667B8E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeBorderColor", "Active Border", "Appearance", "Active border color.", "#2F80ED", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonColor", "Button", "Appearance", "Button color.", "#00000000", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonHoverColor", "Button Hover", "Appearance", "Hover button color.", "#BCD7F5", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonActiveColor", "Button Active", "Appearance", "Active button color.", "#A7C9EE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.closeHoverColor", "Close Hover", "Appearance", "Close hover color.", "#FFE2E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.closeActiveColor", "Close Active", "Appearance", "Close active color.", "#F4CDCD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrScrollContainerProperties[] = {
	UI_DESIGN_PROP("metrics.contentWidth", "Content Width", "Metrics", "Virtual content width.", "520", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.contentHeight", "Content Height", "Metrics", "Virtual content height.", "360", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.offsetX", "Offset X", "Value", "Horizontal offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.offsetY", "Offset Y", "Value", "Vertical offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.policyX", "Policy X", "Behavior", "Horizontal scrollbar policy.", "0", g_arrScrollPolicyEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.policyY", "Policy Y", "Behavior", "Vertical scrollbar policy.", "0", g_arrScrollPolicyEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.scrollbarMode", "Scrollbar Mode", "Behavior", "Scrollbar visual mode.", "0", g_arrScrollbarModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.wheelAxis", "Wheel Axis", "Behavior", "Mouse wheel axis.", "0", g_arrWheelAxisEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.cornerMode", "Corner", "Behavior", "Corner display mode.", "1", g_arrCornerModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.contentDrag", "Content Drag", "Behavior", "Allow content drag scrolling.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.scrollbarSize", "Scrollbar Size", "Metrics", "Scrollbar size.", "14", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minThumbSize", "Min Thumb", "Metrics", "Minimum thumb size.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.buttonSize", "Button Size", "Metrics", "Scrollbar button size.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.wheelStep", "Wheel Step", "Metrics", "Wheel scroll step.", "48", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track", "Appearance", "Scrollbar track color.", "#E4EBF3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.thumbColor", "Thumb", "Appearance", "Scrollbar thumb color.", "#AAB8C8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Scrollbar hover color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeColor", "Active", "Appearance", "Scrollbar active color.", "#6F89A6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled", "Appearance", "Disabled scrollbar color.", "#CED7E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonColor", "Button", "Appearance", "Scrollbar button color.", "#EFF5FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonIconColor", "Button Icon", "Appearance", "Scrollbar button icon color.", "#34475D", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.cornerColor", "Corner", "Appearance", "Corner color.", "#EEF3F9", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.gripColor", "Grip", "Appearance", "Grip color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrCanvasProperties[] = {
	UI_DESIGN_PROP("metrics.canvasWidth", "Canvas Width", "Metrics", "Canvas width.", "520", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.canvasHeight", "Canvas Height", "Metrics", "Canvas height.", "360", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.offsetX", "Offset X", "Value", "Horizontal offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.offsetY", "Offset Y", "Value", "Vertical offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.commands", "Draw Commands", "Data", "Canvas drawing commands.", "line||20|20|||180|120||||||||2||#317ED6|\nrectFill||48|58|96|54||||||||||||#D7E9FF|\nrect||48|58|96|54|||||||||||1||#9EB6D1|\ncircleFill||182|86|||||||||||24||||#317ED6|\ntext||18|126|220|28||||||||||||Canvas preview|#34475D|", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_ENUM_PROP("behavior.policyX", "Policy X", "Behavior", "Horizontal scrollbar policy.", "0", g_arrScrollPolicyEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.policyY", "Policy Y", "Behavior", "Vertical scrollbar policy.", "0", g_arrScrollPolicyEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.scrollbarMode", "Scrollbar Mode", "Behavior", "Scrollbar visual mode.", "1", g_arrScrollbarModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.wheelAxis", "Wheel Axis", "Behavior", "Mouse wheel axis.", "2", g_arrWheelAxisEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.cornerMode", "Corner", "Behavior", "Corner display mode.", "1", g_arrCornerModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.contentDrag", "Content Drag", "Behavior", "Allow content drag scrolling.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.penEnabled", "Pen Enabled", "Behavior", "Enable pen drawing.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.scrollbarSize", "Scrollbar Size", "Metrics", "Scrollbar size.", "14", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minThumbSize", "Min Thumb", "Metrics", "Minimum thumb size.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.buttonSize", "Button Size", "Metrics", "Scrollbar button size.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.wheelStep", "Wheel Step", "Metrics", "Wheel scroll step.", "48", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.penWidth", "Pen Width", "Metrics", "Pen width.", "2", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.clearColor", "Clear", "Appearance", "Canvas clear color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Viewport background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.penColor", "Pen", "Appearance", "Pen color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Canvas border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track", "Appearance", "Scrollbar track color.", "#E4EBF3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.thumbColor", "Thumb", "Appearance", "Scrollbar thumb color.", "#AAB8C8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Scrollbar hover color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeColor", "Active", "Appearance", "Scrollbar active color.", "#6F89A6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Scrollbar focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled", "Appearance", "Disabled scrollbar color.", "#CED7E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonColor", "Button", "Appearance", "Scrollbar button color.", "#EFF5FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonIconColor", "Button Icon", "Appearance", "Scrollbar button icon color.", "#34475D", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.cornerColor", "Corner", "Appearance", "Corner color.", "#EEF3F9", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.gripColor", "Grip", "Appearance", "Grip color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrMessageListProperties[] = {
	UI_DESIGN_ENUM_PROP("data.sourceMode", "Data Source", "Data", "Message data source.", "0", g_arrMessageDataSourceEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.messages", "Messages", "Data", "Rows: sender|time|message|type|id|flags.", "Designer|09:20|Widget created|other|m1\nYou|09:21|Adjust layout|self|m2\nSystem|09:22|Preview updated|system|m3", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.importText", "Import Text", "Data", "Native XUI MessageList import text.", "MESSAGELIST1\nN\t1\t0\tm1\tYou\t09:21\tImported message\nN\t2\t0\tm2\tSystem\t09:22\tImported notice\n", XUI_TABLE_CELL_TYPE_TEXTAREA, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.sourcePath", "Source File", "Data", "Native XUI MessageList file path.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.autoScroll", "Auto Scroll", "Behavior", "Auto-scroll to end.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.selected", "Selected", "Data", "Selected message index.", "1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.scrollY", "Scroll Y", "Viewport", "Initial vertical scroll offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingX", "Padding X", "Metrics", "Horizontal padding.", "10", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingY", "Padding Y", "Metrics", "Vertical padding.", "10", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.nodeGap", "Node Gap", "Metrics", "Message gap.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.avatarSize", "Avatar Size", "Metrics", "Avatar size.", "28", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.avatarGap", "Avatar Gap", "Metrics", "Avatar gap.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.bubbleMaxWidth", "Bubble Max Width", "Metrics", "Max bubble width.", "220", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.bubblePaddingX", "Bubble Padding X", "Metrics", "Bubble horizontal padding.", "10", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.bubblePaddingY", "Bubble Padding Y", "Metrics", "Bubble vertical padding.", "7", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.systemPaddingX", "System Padding X", "Metrics", "System message horizontal padding.", "10", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.systemPaddingY", "System Padding Y", "Metrics", "System message vertical padding.", "5", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.metaHeight", "Meta Height", "Metrics", "Metadata row height.", "18", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minBubbleHeight", "Min Bubble Height", "Metrics", "Minimum bubble height.", "28", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.wheelStep", "Wheel Step", "Metrics", "Mouse wheel step.", "48", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#F7FAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selfBubbleColor", "Self Bubble", "Appearance", "Self bubble color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.otherBubbleColor", "Other Bubble", "Appearance", "Other bubble color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.systemBubbleColor", "System Bubble", "Appearance", "System bubble color.", "#EAF1F8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selfTextColor", "Self Text", "Appearance", "Self message text.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.otherTextColor", "Other Text", "Appearance", "Other message text.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.systemTextColor", "System Text", "Appearance", "System message text.", "#52657A", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.metaTextColor", "Meta Text", "Appearance", "Metadata text color.", "#6B7F95", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.avatarSelfColor", "Self Avatar", "Appearance", "Self avatar color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.avatarOtherColor", "Other Avatar", "Appearance", "Other avatar color.", "#8096B0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover row color.", "#EAF3FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectedColor", "Selected", "Appearance", "Selected row color.", "#D9EAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Message border color.", "#D0DBE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrTimelineProperties[] = {
	UI_DESIGN_PROP("data.timelineLayers", "Layers", "Data", "Rows: name|visible|locked|height|color.", "Layer 1|true|false|24|#4A90E2\nLayer 2|true|false|24|#D64856", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.timelineFrames", "Frames", "Data", "Rows: layer|frame|type.", "Layer 1|0|key\nLayer 1|24|key\nLayer 2|12|key", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.timelineSpans", "Spans", "Data", "Rows: layer|start|end|type|label|color|customType.", "Layer 1|4|22|motion|Move|#317ED6|\nLayer 2|12|38|event|Event|#D64856|", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.selection", "Selection", "Data", "Rows: layer|frame|endLayer|endFrame|selected. Empty end columns select a single frame.", "Layer 1|24|Layer 1|24|true\nLayer 2|12|Layer 2|12|true", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.menuTitles", "Menu Titles", "Data", "Rows: command|title. Commands: rename, visible, locked, addLayer, deleteLayer, moveUp, moveDown, insertFrame, insertKeyframe, insertBlankKeyframe, clearKeyframe, createSpan, createSpanFromSelection, clearSpan.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.frameCount", "Frame Count", "Data", "Frame count.", "120", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.currentFrame", "Current Frame", "Data", "Current frame.", "24", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.frameRate", "Frame Rate", "Value", "Frame rate.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.offsetX", "Offset X", "Viewport", "Initial horizontal timeline offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.offsetY", "Offset Y", "Viewport", "Initial vertical timeline offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.layerHeaderWidth", "Layer Header", "Metrics", "Layer header width.", "110", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.frameWidth", "Frame Width", "Metrics", "Frame cell width.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minFrameWidth", "Min Frame Width", "Metrics", "Minimum zoomed frame width.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.maxFrameWidth", "Max Frame Width", "Metrics", "Maximum zoomed frame width.", "64", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.rowHeight", "Row Height", "Metrics", "Layer row height.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.rulerHeight", "Ruler Height", "Metrics", "Ruler height.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.scrollbarMode", "Scrollbar Mode", "Behavior", "Scrollbar mode.", "0", g_arrScrollbarModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.visibilityFeature", "Visibility", "Behavior", "Show visibility feature.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.lockFeature", "Lock", "Behavior", "Show lock feature.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Timeline background.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.cornerColor", "Corner", "Appearance", "Corner background.", "#EEF5FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.rulerColor", "Ruler", "Appearance", "Ruler background.", "#EAF1F8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.layerColor", "Layer", "Appearance", "Layer row background.", "#F7FAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.layerAltColor", "Layer Alt", "Appearance", "Alternate layer row background.", "#F1F6FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.gridColor", "Grid", "Appearance", "Grid color.", "#D0DBE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.gridStrongColor", "Grid Strong", "Appearance", "Strong grid color.", "#B4C4D8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.mutedTextColor", "Muted Text", "Appearance", "Muted text color.", "#6B7F95", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectedColor", "Selected", "Appearance", "Selected color.", "#D9EAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover color.", "#EAF3FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.currentColor", "Current", "Appearance", "Current frame color.", "#D64856", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.frameColor", "Frame", "Appearance", "Frame cell color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.keyFrameColor", "Key Frame", "Appearance", "Key frame color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.blankKeyFrameColor", "Blank Key Frame", "Appearance", "Blank key frame color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.spanColor", "Span", "Appearance", "Default span color.", "#317ED6B4", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.spanTextColor", "Span Text", "Appearance", "Span text color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.iconColor", "Icon", "Appearance", "Icon color.", "#52657A", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hiddenIconColor", "Hidden Icon", "Appearance", "Hidden icon color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.lockedIconColor", "Locked Icon", "Appearance", "Locked icon color.", "#D64856", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled", "Appearance", "Disabled color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track", "Appearance", "Scrollbar track color.", "#E4EBF3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.thumbColor", "Thumb", "Appearance", "Scrollbar thumb color.", "#AAB8C8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarHoverColor", "Scrollbar Hover", "Appearance", "Scrollbar hover color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarActiveColor", "Scrollbar Active", "Appearance", "Scrollbar active color.", "#6F89A6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarFocusColor", "Scrollbar Focus", "Appearance", "Scrollbar focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarDisabledColor", "Scrollbar Disabled", "Appearance", "Scrollbar disabled color.", "#CED7E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const char g_sDefaultPropertyGridProperties[] =
	"General|Name|Button1|text|Control name|Button1||general.name|true||||||||||||||||||||||\n"
	"General|Enabled|true|bool|Enabled state|true||general.enabled|true||||||||||||||||||||||\n"
	"General|Theme|20|enum|Theme selection|10||general.theme|true|Light,Dark,Blue|||||||||||||||||10,20,30|true,true,false|1|true|20\n"
	"Runtime|Max Players|4|int|Integer property with range|4||runtime.maxPlayers|false||1|64|1|0|||||||||||||||||\n"
	"Appearance|Background|#FFFFFF|color|Background color|#FFFFFF|dirty|appearance.background|true|||||||true||||||||||#FFFFFF,#F7FAFE,#D9EAFE,#317ED6|||||";

static const ui_design_property_def_t g_arrPropertyGridProperties[] = {
	UI_DESIGN_PROP("data.properties", "Properties", "Data", "Rows: category|name|value|type|description|default|flags|id|expanded|options|min|max|step|precision|nullable|alpha|actionText|fileFilter|dateMode|showSecond|dateFormat|dateMin|dateMax|defaultSpan|rangeSeparator|palette|enumValues|enumEnabled|enumSelected|enumUseValue|enumSelectedValue. Legacy Category.Name=value rows still load.", g_sDefaultPropertyGridProperties, XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.selected", "Selected", "Data", "Selected property index.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.scrollY", "Scroll Y", "Viewport", "Initial vertical scroll offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.nameWidth", "Name Width", "Metrics", "Property name column width.", "110", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.rowHeight", "Row Height", "Metrics", "Property row height.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.categoryHeight", "Category Height", "Metrics", "Category row height.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.descriptionHeight", "Description Height", "Metrics", "Description panel height.", "70", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.descriptionMode", "Description", "Behavior", "Description display mode.", "1", g_arrPropertyDescriptionEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.editMode", "Edit Mode", "Behavior", "Editor activation mode.", "1", g_arrTableGridEditModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.scrollbarMode", "Scrollbar Mode", "Behavior", "Scrollbar mode.", "0", g_arrScrollbarModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.categoryColor", "Category", "Appearance", "Category color.", "#EAF1F8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.categoryHoverColor", "Category Hover", "Appearance", "Category hover color.", "#DDEAF7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.categoryTextColor", "Category Text", "Appearance", "Category text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.categoryIconColor", "Category Icon", "Appearance", "Category expander icon color.", "#52657A", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.rowColor", "Row", "Appearance", "Row color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.nameBackgroundColor", "Name Back", "Appearance", "Property name cell background.", "#F7FAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.nameTextColor", "Name Text", "Appearance", "Property name text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.nameHoverColor", "Name Hover", "Appearance", "Property name hover color.", "#F0F6FD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.valueBackgroundColor", "Value Back", "Appearance", "Property value cell background.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.valueTextColor", "Value Text", "Appearance", "Property value text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectedColor", "Selected", "Appearance", "Selected row color.", "#D9EAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.gridColor", "Grid", "Appearance", "Grid color.", "#D0DBE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.readonlyTextColor", "Readonly Text", "Appearance", "Readonly value text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.invalidColor", "Invalid", "Appearance", "Invalid property marker color.", "#D64856", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.dirtyColor", "Dirty", "Appearance", "Dirty property marker color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track", "Appearance", "Scrollbar track color.", "#E4EBF3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.thumbColor", "Thumb", "Appearance", "Scrollbar thumb color.", "#AAB8C8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarHoverColor", "Scrollbar Hover", "Appearance", "Scrollbar hover color.", "#8EA6C0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarActiveColor", "Scrollbar Active", "Appearance", "Scrollbar active color.", "#6F89A6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarFocusColor", "Scrollbar Focus", "Appearance", "Scrollbar focus color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.scrollbarDisabledColor", "Scrollbar Disabled", "Appearance", "Scrollbar disabled color.", "#CED7E2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrMenuBarProperties[] = {
	UI_DESIGN_PROP("data.items", "Items", "Data", "Rows: text|enabled|mnemonic|value.", "File|true|F|1\nEdit|true|E|2\nView|true|V|3", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.menus", "Menus", "Data", "Rows: menu|text|type|enabled|checked|default|danger|shortcut|value|icon. Menu accepts item text, index, or value.", g_sDefaultMenuBarMenus, XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.submenus", "Submenus", "Data", "Rows: menu|parent|text|type|enabled|checked|default|danger|shortcut|value|icon. Parent accepts item text, index, value, or nested path.", g_sDefaultMenuBarSubmenus, XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.hoverIndex", "Hover Index", "Data", "Preview hover item index. -1 clears hover.", "-1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.openIndex", "Open Index", "Data", "Preview opened item index. -1 closes the menu.", "-1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.height", "Height", "Metrics", "Menu bar height.", "28", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingX", "Padding X", "Metrics", "Outer horizontal padding.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingY", "Padding Y", "Metrics", "Outer vertical padding.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.itemPaddingX", "Item Padding", "Metrics", "Item horizontal padding.", "12", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.itemGap", "Item Gap", "Metrics", "Gap between items.", "2", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#F7FAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.itemColor", "Item", "Appearance", "Item color.", "#00000000", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover color.", "#EAF3FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeColor", "Active", "Appearance", "Active color.", "#D9EAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus outline color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrToolbarProperties[] = {
	UI_DESIGN_PROP("data.items", "Items", "Data", "Rows: text|type|enabled|checked|tooltip|group|value|iconSource|iconX|iconY|iconW|iconH. Type is button, toggle, or separator.", "New|button|true|false|Create new file|0|1|||||\nOpen|button|true|false|Open file|0|2|||||\n-|separator|false|false||0|0|||||\nRun|toggle|true|true|Run selected task|1|3|||||", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.hoverIndex", "Hover Index", "Data", "Preview hover item index. -1 clears hover.", "-1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.orientation", "Orientation", "Behavior", "Toolbar orientation.", "0", g_arrOrientationEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.overflow", "Overflow", "Behavior", "Enable overflow menu.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.itemWidth", "Item Width", "Metrics", "Item width.", "64", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.itemHeight", "Item Height", "Metrics", "Item height.", "30", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.separatorSize", "Separator", "Metrics", "Separator size.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.groupGap", "Group Gap", "Metrics", "Gap between item groups.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.overflowButtonSize", "Overflow Button", "Metrics", "Overflow button size.", "28", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingX", "Padding X", "Metrics", "Horizontal padding.", "6", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingY", "Padding Y", "Metrics", "Vertical padding.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconSize", "Icon Size", "Metrics", "Icon size.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconGap", "Icon Gap", "Metrics", "Gap between icon and text.", "6", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#F7FAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.itemColor", "Item", "Appearance", "Item color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover item color.", "#EAF3FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeColor", "Active", "Appearance", "Active item color.", "#D9EAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.checkedColor", "Checked", "Appearance", "Checked item color.", "#BCD7F5", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus outline color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled", "Appearance", "Disabled item color.", "#EEF2F7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.separatorColor", "Separator", "Appearance", "Separator color.", "#D0DBE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.iconColor", "Icon", "Appearance", "Default icon color.", "#34475D", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrStatusBarProperties[] = {
	UI_DESIGN_PROP("data.items", "Items", "Data", "Rows: text|type|section|enabled|clickable|width|flex|min|max|progress|value.", "Ready|text|left|true|false|96|0|0|1|0|1\n|spacer|center|true|false|0|1|0|1|0|2\n100%|progress|right|true|false|120|0|0|1|1|3", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.hoverIndex", "Hover Index", "Data", "Preview hover item index. -1 clears hover.", "-1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.height", "Height", "Metrics", "Status bar height.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingX", "Padding X", "Metrics", "Horizontal padding.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingY", "Padding Y", "Metrics", "Vertical padding.", "2", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.gap", "Gap", "Metrics", "Gap between items.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.itemPaddingX", "Item Padding X", "Metrics", "Item horizontal padding.", "6", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.itemPaddingY", "Item Padding Y", "Metrics", "Item vertical padding.", "2", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.progressHeight", "Progress Height", "Metrics", "Progress item height.", "6", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.topBorderWidth", "Top Border", "Metrics", "Top border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#F7FAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.highlightColor", "Highlight", "Appearance", "Highlight color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.itemColor", "Item", "Appearance", "Item background color.", "#00000000", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover color.", "#EAF3FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeColor", "Active", "Appearance", "Active item color.", "#D9EAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus outline color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.progressTrackColor", "Progress Track", "Appearance", "Progress track color.", "#E4EBF3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.progressFillColor", "Progress Fill", "Appearance", "Progress fill color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrDockPanelProperties[] = {
	UI_DESIGN_PROP("data.windows", "Windows", "Data", "Rows: title|side|ratio|closable|dockable|content|pane|active|autoHide|region|hidden|floating|floatX|floatY|floatW|floatH.", "Document|fill|0|true|true|Document|doc|true|false|document|false|false|80|70|260|180\nPreview|fill|0|true|true|Preview|doc|false|false|document|false|false|80|70|260|180\nToolbox|left|0.22|true|true|Toolbox|leftTools|true|false|document|false|false|80|70|260|180\nAssets|left|0.22|true|true|Assets|leftTools|false|false|document|false|false|80|70|260|180\nProperties|right|0.24|true|true|Properties|rightTools|true|false|document|false|false|80|70|260|180\nOutput|bottom|0.24|true|true|Output|bottom|true|false|document|false|false|80|70|260|180", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.regions", "Regions", "Layout", "Rows: region|mode|value. Regions: document,left,right,top,bottom. Modes: portion,pixel.", "left|portion|0.22\nright|portion|0.24\nbottom|portion|0.24", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.menuTitles", "Menu Titles", "Data", "Rows: command|title. Commands: float, autoHide, close, closeOthers, closeAll.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.tooltips", "Tooltips", "Data", "Rows: target|text. Targets: close, autoHide, options, moreTabs, dock.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("metrics.captionHeight", "Caption Height", "Metrics", "Pane caption height.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.tabStripHeight", "Tab Height", "Metrics", "Tab strip height.", "25", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.buttonSize", "Button Size", "Metrics", "Caption button size.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.buttonGap", "Button Gap", "Metrics", "Caption button gap.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.splitterSize", "Splitter", "Metrics", "Splitter size.", "5", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.splitterHitSize", "Splitter Hit", "Metrics", "Splitter hit size.", "7", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minPaneWidth", "Min Pane W", "Metrics", "Minimum pane width.", "96", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minPaneHeight", "Min Pane H", "Metrics", "Minimum pane height.", "72", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.tabMinWidth", "Tab Min W", "Metrics", "Minimum tab width.", "68", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.tabMaxWidth", "Tab Max W", "Metrics", "Maximum tab width.", "150", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.tabPaddingX", "Tab Padding", "Metrics", "Tab horizontal padding.", "12", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.floatTitleHeight", "Float Title", "Metrics", "Floating window title height.", "27", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.floatBorderWidth", "Float Border", "Metrics", "Floating window border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.autoHideStripSize", "Auto-hide Strip", "Metrics", "Auto-hide strip size.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.leftRatio", "Left Ratio", "Layout", "Default left pane ratio.", "0.22", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.rightRatio", "Right Ratio", "Layout", "Default right pane ratio.", "0.24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.bottomRatio", "Bottom Ratio", "Layout", "Default bottom pane ratio.", "0.24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Panel background.", "#E8F1FA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.paneColor", "Pane", "Appearance", "Pane color.", "#F6FAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.clientColor", "Client", "Appearance", "Client area color.", "#FAFDFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.captionColor", "Caption", "Appearance", "Caption color.", "#E2EEF9", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeCaptionColor", "Active Caption", "Appearance", "Active caption color.", "#2E7DD7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.captionTextColor", "Caption Text", "Appearance", "Caption text color.", "#2D4158", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeCaptionTextColor", "Active Text", "Appearance", "Active caption text color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.tabColor", "Tab", "Appearance", "Tab color.", "#EBF4FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.tabHoverColor", "Tab Hover", "Appearance", "Hover tab color.", "#DBECFA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeTabColor", "Active Tab", "Appearance", "Active tab color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.tabTextColor", "Tab Text", "Appearance", "Tab text color.", "#374C63", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeTabTextColor", "Active Tab Text", "Appearance", "Active tab text color.", "#23374C", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#91B0CF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus color.", "#2F7DD6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.splitterColor", "Splitter", "Appearance", "Splitter color.", "#D3E2F0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.splitterHoverColor", "Splitter Hover", "Appearance", "Hover splitter color.", "#B1CFEB", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.splitterActiveColor", "Splitter Active", "Appearance", "Active splitter color.", "#388ADC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonColor", "Button", "Appearance", "Button glyph color.", "#466380", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonHoverColor", "Button Hover", "Appearance", "Hover button color.", "#2976C8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonActiveColor", "Button Active", "Appearance", "Active button color.", "#1458A2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.autoHideColor", "Auto-hide", "Appearance", "Auto-hide tab color.", "#E1EFFA", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.autoHideHoverColor", "Auto-hide Hover", "Appearance", "Hover auto-hide tab color.", "#CAE2F7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.floatTitleColor", "Float Title", "Appearance", "Floating window title color.", "#2F7DD6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.floatBorderColor", "Float Border", "Appearance", "Floating window border color.", "#2F7DD6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrPopupProperties[] = {
	UI_DESIGN_PROP("data.content", "Content", "Data", "Rows: text|x|y|w|h|color|align.", "Project actions|12|10|220|24|#354960|left\nCreate new scene|12|44|220|28|#354960|left\nOpen recent workspace|12|80|220|28|#354960|left", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("behavior.open", "Preview Open", "Behavior", "Show popup preview on the artboard.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.modal", "Modal", "Behavior", "Modal popup behavior.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.consumeInside", "Consume Inside", "Behavior", "Consume inside pointer events.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.matchOwnerWidth", "Match Owner", "Behavior", "Match owner width.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.anchor", "Anchor", "Behavior", "Popup anchor.", "0", g_arrPopupAnchorEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.direction", "Direction", "Behavior", "Popup growth direction.", "0", g_arrPopupDirectionEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.outsidePolicy", "Outside", "Behavior", "Outside pointer policy.", "0", g_arrPopupOutsidePolicyEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.ownerPolicy", "Owner", "Behavior", "Owner pointer policy.", "0", g_arrPopupOwnerPolicyEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.escapePolicy", "Escape", "Behavior", "Escape key policy.", "0", g_arrPopupEscapePolicyEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.focusPolicy", "Focus", "Behavior", "Focus policy.", "0", g_arrPopupFocusPolicyEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.scrollbarMode", "Scrollbar", "Behavior", "Scrollbar mode.", "1", g_arrScrollbarModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.scrollX", "Scroll X", "Viewport", "Initial horizontal content scroll offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.scrollY", "Scroll Y", "Viewport", "Initial vertical content scroll offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.contentWidth", "Content Width", "Metrics", "Virtual content width.", "260", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.contentHeight", "Content Height", "Metrics", "Virtual content height.", "180", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.maxWidth", "Max Width", "Metrics", "Maximum popup width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.maxHeight", "Max Height", "Metrics", "Maximum popup height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.gap", "Gap", "Metrics", "Owner gap.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.offsetX", "Offset X", "Metrics", "Horizontal offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.offsetY", "Offset Y", "Metrics", "Vertical offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.margin", "Margin", "Metrics", "Viewport margin.", "6", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.padding", "Padding", "Metrics", "Inner padding.", "5", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.shadowSize", "Shadow", "Metrics", "Shadow size.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.scrollbarSize", "Scrollbar Size", "Metrics", "Scrollbar size.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.panelColor", "Panel", "Appearance", "Panel color.", "#FAFDFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#98B4D3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.shadowColor", "Shadow", "Appearance", "Shadow color.", "#233F6028", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backdropColor", "Backdrop", "Appearance", "Backdrop color.", "#00000000", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.trackColor", "Track", "Appearance", "Scrollbar track color.", "#E2EEF9", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.thumbColor", "Thumb", "Appearance", "Scrollbar thumb color.", "#5B8BBED2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Scrollbar hover color.", "#4480C6EB", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.activeColor", "Active", "Appearance", "Scrollbar active color.", "#2868AE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Scrollbar focus color.", "#3694E0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledColor", "Disabled", "Appearance", "Scrollbar disabled color.", "#B0BECE96", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrMenuProperties[] = {
	UI_DESIGN_PROP("data.items", "Items", "Data", "Rows: text|type|enabled|checked|default|danger|shortcut|value|icon. Type is normal, check, radio, separator, or submenu.", "New|normal|true|false|false|false|Ctrl+N|1|0\nOpen|normal|true|false|false|false|Ctrl+O|2|0\n-|separator|false|false|false|false||0|0\nShow Grid|check|true|true|false|false||3|0\nDanger Action|normal|true|false|false|true||4|0\nMore|submenu|true|false|false|false||5|0", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.submenus", "Submenus", "Data", "Rows: parent|text|type|enabled|checked|default|danger|shortcut|value|icon. Parent accepts item text, index, value, or nested path.", g_sDefaultMenuSubmenus, XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.hoverIndex", "Hover Index", "Data", "Preview hover item index. -1 clears hover.", "-1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.previewOpen", "Preview Open", "Behavior", "Show menu as an artboard item.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.itemHeight", "Item Height", "Metrics", "Item height.", "26", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.separatorHeight", "Separator", "Metrics", "Separator height.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingX", "Padding X", "Metrics", "Horizontal padding.", "5", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingY", "Padding Y", "Metrics", "Vertical padding.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.markWidth", "Mark Width", "Metrics", "Check/radio mark width.", "22", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconWidth", "Icon Width", "Metrics", "Icon slot width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.shortcutGap", "Shortcut Gap", "Metrics", "Shortcut text gap.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.arrowWidth", "Arrow Width", "Metrics", "Submenu arrow width.", "18", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minWidth", "Min Width", "Metrics", "Minimum menu width.", "168", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.maxHeight", "Max Height", "Metrics", "Maximum menu height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.panelColor", "Panel", "Appearance", "Panel color.", "#FAFDFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#98B4D3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.shadowColor", "Shadow", "Appearance", "Shadow color.", "#233F6028", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover color.", "#EAF3FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverTextColor", "Hover Text", "Appearance", "Hover text color.", "#1F5FA8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.shortcutColor", "Shortcut", "Appearance", "Shortcut text color.", "#6D7F95", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.dangerTextColor", "Danger", "Appearance", "Danger text color.", "#D64856", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.markColor", "Mark", "Appearance", "Check/radio mark color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.separatorColor", "Separator", "Appearance", "Separator color.", "#D0DBE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusColor", "Focus", "Appearance", "Focus outline color.", "#5B9BE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrMsgBoxProperties[] = {
	UI_DESIGN_PROP("text.title", "Title", "Text", "Dialog title.", "Message", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.message", "Message", "Text", "Dialog message.", "Operation completed.", XUI_TABLE_CELL_TYPE_TEXTAREA, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.open", "Preview Open", "Behavior", "Show the dialog preview on the artboard.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.type", "Icon", "Behavior", "Message icon.", "0", g_arrMsgIconEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.buttons", "Buttons", "Behavior", "Button set.", "0", g_arrMsgBoxButtonsEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.buttonTitles", "Button Titles", "Data", "Rows: button|title for preset button sets. Buttons: ok, cancel, yes, no.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.customButtons", "Custom Buttons", "Data", "Rows: text|result|semantic(default|primary|danger).", "Accept|0|primary\nCancel|1|default", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.iconSource", "Icon Source", "Data", "Optional custom icon image path. Empty uses the built-in icon.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.x", "Icon Source X", "Icon", "Custom icon source rectangle x. 0 with zero width uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.y", "Icon Source Y", "Icon", "Custom icon source rectangle y. 0 with zero height uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.w", "Icon Source W", "Icon", "Custom icon source rectangle width. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.h", "Icon Source H", "Icon", "Custom icon source rectangle height. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.modal", "Modal", "Behavior", "Modal dialog.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingX", "Padding X", "Metrics", "Horizontal padding.", "18", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingY", "Padding Y", "Metrics", "Vertical padding.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconSize", "Icon Size", "Metrics", "Icon size.", "38", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconGap", "Icon Gap", "Metrics", "Gap between icon and message.", "14", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.buttonWidth", "Button Width", "Metrics", "Button width.", "78", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.buttonHeight", "Button Height", "Metrics", "Button height.", "28", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.buttonGap", "Button Gap", "Metrics", "Button gap.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.footerHeight", "Footer Height", "Metrics", "Footer area height.", "50", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minMessageHeight", "Min Message H", "Metrics", "Minimum message text height.", "38", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minWidth", "Min Width", "Metrics", "Minimum dialog width.", "320", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.maxWidth", "Max Width", "Metrics", "Maximum dialog width.", "680", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.maxWidthRatio", "Max Width Ratio", "Metrics", "Viewport max width ratio.", "0.8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backdropColor", "Backdrop", "Appearance", "Modal backdrop color.", "#122A425A", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.clientColor", "Client", "Appearance", "Dialog client color.", "#FAFDFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Dialog border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.messageColor", "Message", "Appearance", "Message text color.", "#22344E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.mutedTextColor", "Muted Text", "Appearance", "Muted text color.", "#5D718A", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.iconColor", "Icon", "Appearance", "Icon color.", "#2384D6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonColor", "Button", "Appearance", "Button color.", "#F7FBFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonHoverColor", "Button Hover", "Appearance", "Button hover color.", "#E8F3FD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonActiveColor", "Button Active", "Appearance", "Button active color.", "#D4E7F8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonFocusColor", "Button Focus", "Appearance", "Button focus color.", "#2F80D6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonDisabledColor", "Button Disabled", "Appearance", "Disabled button color.", "#E2EBF3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrFileDialogProperties[] = {
	UI_DESIGN_PROP("text.title", "Title", "Text", "Dialog title.", "Open File", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.initialDir", "Initial Dir", "Text", "Initial directory.", ".", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.fileName", "File Name", "Text", "Initial file name.", "", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.filter", "Filter", "Data", "Filter list.", "All Files (*.*)|*.*\nText Files (*.txt)|*.txt", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.roots", "Root Places", "Data", "Rows: label|path|selected|enabled.", "Project|D:\\GIT\\xge|true|true\nDesktop|%USERPROFILE%\\Desktop|false|true\nDocuments|%USERPROFILE%\\Documents|false|true", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.entries", "Entries", "Data", "Rows: name|kind|size|modified|selected|enabled.", "scene.ui|file|14 KB|Today|true|true\nassets|folder|Folder|Yesterday|false|true\npreview.png|image|240 KB|Monday|false|true", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.buttonTitles", "Button And Label Text", "Data", "Rows: target|text. Targets: ok, cancel, up, refresh, path, file, folder, type.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.selectedFilter", "Selected Filter", "Data", "Selected filter row index.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.hoverEntry", "Hover Entry", "Data", "Preview hover entry row index.", "1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.open", "Preview Open", "Behavior", "Show the dialog preview on the artboard.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.mode", "Mode", "Behavior", "Dialog mode.", "1", g_arrFileDialogModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.modal", "Modal", "Behavior", "Modal dialog.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.pathEditMode", "Path Edit Mode", "Behavior", "Show path as an editable input instead of breadcrumb.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.showRoots", "Show Roots", "Behavior", "Show the root places sidebar.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.dialogWidth", "Dialog Width", "Metrics", "Runtime dialog width.", "760", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.dialogHeight", "Dialog Height", "Metrics", "Runtime dialog height.", "520", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.padding", "Padding", "Metrics", "Dialog content padding.", "12", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.sidebarWidth", "Sidebar Width", "Metrics", "Root places sidebar width.", "118", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.toolbarHeight", "Toolbar Height", "Metrics", "Path toolbar height.", "30", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.footerHeight", "Footer Height", "Metrics", "File name and action footer height.", "74", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.rowHeight", "Row Height", "Metrics", "File list row height.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.buttonWidth", "Button Width", "Metrics", "Action button width.", "76", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.buttonHeight", "Button Height", "Metrics", "Action button height.", "26", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.buttonGap", "Button Gap", "Metrics", "Action button gap.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Preview background.", "#F7FAFE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.headerColor", "Header", "Appearance", "Preview header color.", "#EAF2FB", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.clientColor", "Client", "Appearance", "Preview client color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.sidebarColor", "Sidebar", "Appearance", "Root places sidebar color.", "#F1F6FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.listColor", "List Background", "Appearance", "File list background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.inputColor", "Input Background", "Appearance", "Path, name and filter input background.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonColor", "Button", "Appearance", "Normal button color.", "#F7FBFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonHoverColor", "Button Hover", "Appearance", "Button hover color.", "#E8F3FD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonActiveColor", "Button Active", "Appearance", "Button active color.", "#D4E7F8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonFocusColor", "Button Focus", "Appearance", "Button focus color.", "#2F80D6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonDisabledColor", "Button Disabled", "Appearance", "Disabled button color.", "#E2EBF3", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectedColor", "Selected", "Appearance", "Selected row color.", "#DCEEFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverColor", "Hover", "Appearance", "Hover row color.", "#EEF6FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled entry text color.", "#9AA8B8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Main text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.mutedTextColor", "Muted Text", "Appearance", "Secondary text color.", "#6B7F95", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Preview border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrMsgTipProperties[] = {
	UI_DESIGN_PROP("text.text", "Text", "Text", "Tip text.", "Saved successfully.", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.open", "Preview Open", "Behavior", "Show the tip preview on the artboard.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.type", "Icon", "Behavior", "Tip icon.", "0", g_arrMsgIconEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.duration", "Duration", "Behavior", "Visible duration in seconds.", "2.4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.iconSource", "Icon Source", "Data", "Optional custom icon image path. Empty uses the built-in icon.", "", XUI_TABLE_CELL_TYPE_FILE, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.x", "Icon Source X", "Icon", "Custom icon source rectangle x. 0 with zero width uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.y", "Icon Source Y", "Icon", "Custom icon source rectangle y. 0 with zero height uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.w", "Icon Source W", "Icon", "Custom icon source rectangle width. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("icon.h", "Icon Source H", "Icon", "Custom icon source rectangle height. 0 uses the full image.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minWidth", "Min Width", "Metrics", "Minimum width.", "92", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.maxWidth", "Max Width", "Metrics", "Maximum width.", "360", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minHeight", "Min Height", "Metrics", "Minimum height.", "40", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingX", "Padding X", "Metrics", "Horizontal padding.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingY", "Padding Y", "Metrics", "Vertical padding.", "10", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconSize", "Icon Size", "Metrics", "Icon size.", "20", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconGap", "Icon Gap", "Metrics", "Icon gap.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.offsetY", "Offset Y", "Metrics", "Vertical screen offset.", "-72", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Tip background.", "#202A36DE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Tip border.", "#7297BE78", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Tip text color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.iconColor", "Icon", "Appearance", "Tip icon color.", "#4E9FDC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.shadowColor", "Shadow", "Appearance", "Tip shadow color.", "#00000000", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrToastProperties[] = {
	UI_DESIGN_PROP("text.title", "Title", "Text", "Toast title.", "Build Finished", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.message", "Message", "Text", "Toast message.", "No errors were reported.", XUI_TABLE_CELL_TYPE_TEXTAREA, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.toasts", "Toast Queue", "Data", "Rows: type|title|message|duration|clickable|progress.", "success|Build Finished|No errors were reported.|4|true|0.65\nwarning|Slow Step|Asset scan took longer than expected.|6|false|0.35", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("behavior.open", "Preview Open", "Behavior", "Show active toast previews on the artboard.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.type", "Type", "Behavior", "Toast type.", "1", g_arrToastTypeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.placement", "Placement", "Behavior", "Toast placement.", "0", g_arrToastPlacementEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.direction", "Direction", "Behavior", "Stack direction.", "0", g_arrToastDirectionEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.duration", "Duration", "Behavior", "Visible duration in seconds.", "4", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.width", "Width", "Metrics", "Toast width.", "320", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minWidth", "Min Width", "Metrics", "Minimum width.", "180", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.minHeight", "Min Height", "Metrics", "Minimum height.", "58", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.margin", "Margin", "Metrics", "Viewport margin.", "18", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.gap", "Gap", "Metrics", "Gap between visible toasts.", "8", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingX", "Padding X", "Metrics", "Horizontal padding.", "14", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.paddingY", "Padding Y", "Metrics", "Vertical padding.", "10", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconSize", "Icon Size", "Metrics", "Icon size.", "28", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.iconGap", "Icon Gap", "Metrics", "Icon gap.", "10", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.closeSize", "Close Size", "Metrics", "Close button size.", "16", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.progressHeight", "Progress", "Metrics", "Progress height.", "2", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.maxVisible", "Max Visible", "Metrics", "Visible toast capacity. 0 lets XUI fit by viewport.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Toast background.", "#F7FCFFF6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Toast border.", "#80AECFD2", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Toast text color.", "#1F3A52", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.mutedTextColor", "Muted Text", "Appearance", "Toast muted text.", "#5C7082", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.shadowColor", "Shadow", "Appearance", "Toast shadow.", "#06162A30", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.infoColor", "Info", "Appearance", "Info accent.", "#4E9FDC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.successColor", "Success", "Appearance", "Success accent.", "#2BB896", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.warningColor", "Warning", "Appearance", "Warning accent.", "#E79C2D", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.errorColor", "Error", "Appearance", "Error accent.", "#D84C5A", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.closeColor", "Close", "Appearance", "Close icon color.", "#607E94", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.closeHoverColor", "Close Hover", "Appearance", "Close hover color.", "#1F3A52", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrCascaderProperties[] = {
	UI_DESIGN_PROP("data.items", "Items", "Data", "Path rows or text|value|parentIndex|leaf|disabled.", "UI/Controls/Button\nUI/Controls/Input", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.selectedPath", "Selected Path", "Data", "Initial selected path.", "UI/Controls/Button", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.placeholder", "Placeholder", "Text", "Placeholder text.", "Select path", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.separator", "Separator", "Text", "Path separator.", "/", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.showAllLevels", "Show All Levels", "Behavior", "Show full selected path.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.clearable", "Clearable", "Behavior", "Enable clear button.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.selectAnyLevel", "Any Level", "Behavior", "Allow selecting non-leaf nodes.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.expandTrigger", "Expand Trigger", "Behavior", "Expansion trigger.", "0", g_arrCascaderExpandEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.open", "Open", "Behavior", "Preview the popup open state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.popupPlacement", "Popup", "Behavior", "Popup placement.", "0", g_arrCascaderPopupEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.itemHeight", "Item Height", "Metrics", "Item height.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.columnWidth", "Column Width", "Metrics", "Popup column width.", "150", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.popupHeight", "Popup Height", "Metrics", "Popup height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.popupMaxHeight", "Popup Max", "Metrics", "Popup max height.", "240", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.placeholderColor", "Placeholder", "Appearance", "Placeholder color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBackgroundColor", "Hover Back", "Appearance", "Hover background color.", "#F0F6FD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.openBackgroundColor", "Open Back", "Appearance", "Open background color.", "#E7F1FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledBackgroundColor", "Disabled Back", "Appearance", "Disabled background color.", "#EEF2F7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBorderColor", "Hover Border", "Appearance", "Hover border color.", "#7EA4CF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusBorderColor", "Focus Border", "Appearance", "Focus border color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.arrowColor", "Arrow", "Appearance", "Arrow color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledArrowColor", "Disabled Arrow", "Appearance", "Disabled arrow color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonColor", "Button", "Appearance", "Drop button color.", "#F6FAFD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonHoverColor", "Button Hover", "Appearance", "Drop button hover color.", "#E6F6EE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonOpenColor", "Button Open", "Appearance", "Drop button open color.", "#DCF2E8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupPanelColor", "Popup Panel", "Popup", "Popup panel color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupBorderColor", "Popup Border", "Popup", "Popup border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupShadowColor", "Popup Shadow", "Popup", "Popup shadow color.", "#0000002E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupTextColor", "Popup Text", "Popup", "Popup text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupMutedTextColor", "Popup Muted", "Popup", "Popup muted text color.", "#6B7F95", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupHoverColor", "Popup Hover", "Popup", "Popup hover color.", "#F0F6FD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupActiveColor", "Popup Active", "Popup", "Popup active color.", "#E7F1FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupSelectedColor", "Popup Selected", "Popup", "Popup selected color.", "#EEF6FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupActiveTextColor", "Popup Active Text", "Popup", "Popup active text color.", "#1F5FA8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupDisabledTextColor", "Popup Disabled", "Popup", "Popup disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupSeparatorColor", "Popup Separator", "Popup", "Popup separator color.", "#D0DBE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrColorPickerProperties[] = {
	UI_DESIGN_PROP("value.color", "Color", "Value", "Selected color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.useHex", "Use Hex", "Value", "Use Hex text as the selected color source.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.hex", "Hex", "Value", "Selected color as #RRGGBB or #RRGGBBAA.", "#317ED6FF", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.palette", "Palette", "Data", "Custom palette.", "#317ED6\n#D64856\n#2D9D6E", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("behavior.alphaEnabled", "Alpha", "Behavior", "Enable alpha editing.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.open", "Open", "Behavior", "Preview the popup open state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.popupPlacement", "Popup", "Behavior", "Popup placement.", "0", g_arrColorPickerPopupEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.popupWidth", "Popup Width", "Metrics", "Popup width.", "260", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.popupHeight", "Popup Height", "Metrics", "Popup height.", "250", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBackgroundColor", "Hover Back", "Appearance", "Hover background color.", "#F0F6FD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.openBackgroundColor", "Open Back", "Appearance", "Open background color.", "#E7F1FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledBackgroundColor", "Disabled Back", "Appearance", "Disabled background color.", "#EEF2F7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBorderColor", "Hover Border", "Appearance", "Hover border color.", "#7EA4CF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusBorderColor", "Focus Border", "Appearance", "Focus border color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.arrowColor", "Arrow", "Appearance", "Arrow color.", "#2A5C88", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledArrowColor", "Disabled Arrow", "Appearance", "Disabled arrow color.", "#8694A4B4", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonColor", "Button", "Appearance", "Drop button color.", "#ECF6FD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonHoverColor", "Button Hover", "Appearance", "Drop button hover color.", "#DCEEFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.buttonOpenColor", "Button Open", "Appearance", "Drop button open color.", "#CFE5F7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupPanelColor", "Popup Panel", "Popup", "Popup panel color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupBorderColor", "Popup Border", "Popup", "Popup border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupShadowColor", "Popup Shadow", "Popup", "Popup shadow color.", "#0000002E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupTextColor", "Popup Text", "Popup", "Popup text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupMutedTextColor", "Popup Muted", "Popup", "Popup muted text color.", "#6B7F95", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupAccentColor", "Popup Accent", "Popup", "Popup accent color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupFieldColor", "Popup Field", "Popup", "Popup field color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupFieldBorderColor", "Popup Field Border", "Popup", "Popup field border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupSeparatorColor", "Popup Separator", "Popup", "Popup separator color.", "#D0DBE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrDatePickerProperties[] = {
	UI_DESIGN_ENUM_PROP("behavior.mode", "Mode", "Behavior", "Date picker mode.", "0", g_arrDatePickerModeEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.hasValue", "Has Value", "Value", "Use explicit value.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.value", "Value", "Value", "Single value, using yyyy-mm-dd, hh:nn:ss or yyyy-mm-dd hh:nn:ss.", "2026-01-01", XUI_TABLE_CELL_TYPE_DATETIME, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.rangeStart", "Range Start", "Value", "Range start value.", "2026-01-01", XUI_TABLE_CELL_TYPE_DATETIME, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.rangeEnd", "Range End", "Value", "Range end value.", "2026-01-02", XUI_TABLE_CELL_TYPE_DATETIME, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.defaultRangeSpan", "Default Span", "Value", "Default range span, such as 1d, 4h, 90m or seconds.", "1d", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("limits.hasMin", "Has Min", "Limits", "Enable minimum limit.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("limits.min", "Min", "Limits", "Minimum selectable value.", "2020-01-01", XUI_TABLE_CELL_TYPE_DATETIME, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("limits.hasMax", "Has Max", "Limits", "Enable maximum limit.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("limits.max", "Max", "Limits", "Maximum selectable value.", "2030-12-31", XUI_TABLE_CELL_TYPE_DATETIME, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.nullable", "Nullable", "Behavior", "Allow empty value.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.showSecond", "Show Second", "Behavior", "Show seconds.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.firstDayOfWeek", "First Day", "Behavior", "First day of week.", "1", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.open", "Open", "Behavior", "Preview the popup open state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.format", "Format", "Text", "Display format.", "yyyy-mm-dd", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("text.rangeSeparator", "Range Separator", "Text", "Range separator.", " - ", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.popupPlacement", "Popup", "Behavior", "Popup placement.", "0", g_arrDatePickerPopupEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.popupWidth", "Popup Width", "Metrics", "Popup width.", "280", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.popupHeight", "Popup Height", "Metrics", "Popup height.", "300", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.borderWidth", "Border Width", "Metrics", "Border width.", "1", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.textColor", "Text", "Appearance", "Text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledTextColor", "Disabled Text", "Appearance", "Disabled text color.", "#8C9AAF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBackgroundColor", "Hover Back", "Appearance", "Hover background color.", "#F0F6FD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.openBackgroundColor", "Open Back", "Appearance", "Open background color.", "#E7F1FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledBackgroundColor", "Disabled Back", "Appearance", "Disabled background color.", "#EEF2F7", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.borderColor", "Border", "Appearance", "Border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.hoverBorderColor", "Hover Border", "Appearance", "Hover border color.", "#7EA4CF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.focusBorderColor", "Focus Border", "Appearance", "Focus border color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.arrowColor", "Arrow", "Appearance", "Arrow color.", "#3C5268", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.disabledArrowColor", "Disabled Arrow", "Appearance", "Disabled arrow color.", "#96A2AE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupPanelColor", "Popup Panel", "Popup", "Popup panel color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupBorderColor", "Popup Border", "Popup", "Popup border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupShadowColor", "Popup Shadow", "Popup", "Popup shadow color.", "#0000002E", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupTextColor", "Popup Text", "Popup", "Popup text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupMutedTextColor", "Popup Muted", "Popup", "Popup muted text color.", "#6B7F95", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupAccentColor", "Popup Accent", "Popup", "Popup accent color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupFieldColor", "Popup Field", "Popup", "Popup field color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupFieldBorderColor", "Popup Field Border", "Popup", "Popup field border color.", "#9EB6D1", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupSelectedTextColor", "Popup Selected Text", "Popup", "Popup selected text color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupDisabledDayColor", "Popup Disabled Day", "Popup", "Popup disabled day color.", "#B7C3D0", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.popupSeparatorColor", "Popup Separator", "Popup", "Popup separator color.", "#D0DBE8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrCodeEditProperties[] = {
	UI_DESIGN_PROP("data.text", "Text", "Data", "Editor text.", "int main(void) {\n    return 0;\n}", XUI_TABLE_CELL_TYPE_TEXTAREA, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.markers", "Markers", "Data", "Rows: line|marker|flags|tooltip. Line is 1-based.", "1|breakpoint|0|Breakpoint\n2|bookmark|0|Bookmark", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.indicators", "Indicators", "Data", "Rows: indicator|style|startOffset|endOffset|flags.", "1|squiggle|15|21|0\n2|searchResult|4|8|0", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.diagnostics", "Diagnostics", "Data", "Rows: severity|startOffset|endOffset|code|message|source.", "warning|15|21|sample.return|Sample diagnostic|UIDesign", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.margins", "Margins", "Data", "Rows: id|kind|width|visible|clickable|resizable. Empty keeps the simple margin switches.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.selections", "Selections", "Data", "Rows: anchor|caret|preferredColumn|flags. Offsets are UTF-8 byte offsets; flags: rect, reversed, inactive.", "4|8|-1|", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.folds", "Folds", "Data", "Rows: startLine|endLine|level|flags. Lines are 1-based; flags: collapsed, header, comment, preprocessor, region, custom.", "1|3|0|header", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.menuTitles", "Menu Titles", "Data", "Rows: command|title. Commands: undo, redo, cut, copy, paste, delete, selectAll, find, replace, gotoLine, toggleComment, toggleFold.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.styles", "Styles", "Theme", "Rows: style|foreground|background|bold|italic|underline|strike. Style accepts built-in names or ids.", "keyword|#1F5FA8|#00000000|true|false|false|false\ncomment|#6B7F95|#00000000|false|true|false|false\nsearchResult|#233246|#FFE08A|false|false|false|false", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.tokenStyles", "Token Styles", "Theme", "Rows: token|style. Controls how lexer token kinds resolve to theme styles.", "text|default\nkeyword|keyword\ntype|type\nidentifier|identifier\nnumber|number\nstring|string\nchar|char\ncomment|comment\npreprocessor|preprocessor\noperator|operator\nbrace|brace\nerror|error", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("text.language", "Language", "Text", "Language id.", "c", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.pattern", "Pattern", "Find", "Pattern used to preview built-in find results.", "", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.replacement", "Replacement", "Find", "Replacement text used by the replace window.", "", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("find.previewMode", "Preview", "Find", "Precompute a find preview or result list.", "0", g_arrFindPreviewEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.caseSensitive", "Case Sensitive", "Find", "Match case.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.wholeWord", "Whole Word", "Find", "Match whole words.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.regex", "Regex", "Find", "Use regular expression matching.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.escape", "Escape", "Find", "Parse escape sequences.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.selection", "Selection Scope", "Find", "Restrict search to the current selection/range.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.wrap", "Wrap", "Find", "Wrap search at document boundaries.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.backward", "Backward", "Find", "Search backward.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.startOffset", "Start Offset", "Find", "Search start byte offset.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.rangeStart", "Range Start", "Find", "Optional search range start byte offset.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("find.rangeEnd", "Range End", "Find", "Optional search range end byte offset. 0 means document end.", "0", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.readonly", "Readonly", "Behavior", "Readonly state.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.wordWrap", "Word Wrap", "Behavior", "Enable word wrapping.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.showWhitespace", "Whitespace", "Behavior", "Show whitespace.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.showEol", "EOL", "Behavior", "Show end-of-line marks.", "false", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.eolMode", "EOL Mode", "Behavior", "Line ending mode used by editing commands.", "1", g_arrCodeEolEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.indentGuides", "Indent Guides", "Behavior", "Show indent guides.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.expandTabs", "Expand Tabs", "Behavior", "Expand tabs into spaces.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.lineNumbers", "Line Numbers", "Behavior", "Show line number margin.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.markerMargin", "Marker Margin", "Behavior", "Show marker margin.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.foldMargin", "Fold Margin", "Behavior", "Show code folding margin.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.diagnosticMargin", "Diagnostic Margin", "Behavior", "Show diagnostic margin.", "true", XUI_TABLE_CELL_TYPE_BOOL, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.findWindow", "Find Window", "Behavior", "Open the built-in find or replace window in the designer preview.", "0", g_arrFindWindowEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.tabColumns", "Tab Columns", "Metrics", "Tab width in columns.", "4", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("metrics.indentColumns", "Indent Columns", "Metrics", "Indent width in columns.", "4", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.scrollX", "Scroll X", "Value", "Horizontal scroll.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.scrollY", "Scroll Y", "Value", "Vertical scroll.", "0", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.backgroundColor", "Background", "Theme", "Editor background color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.textColor", "Text", "Theme", "Default source text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.keywordColor", "Keyword", "Theme", "Keyword token color.", "#1F5FA8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.typeColor", "Type", "Theme", "Type token color.", "#8A4CB8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.identifierColor", "Identifier", "Theme", "Identifier token color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.numberColor", "Number", "Theme", "Number literal color.", "#B45F06", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.stringColor", "String", "Theme", "String literal color.", "#2F7D32", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.charColor", "Char", "Theme", "Character literal color.", "#2F7D32", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.commentColor", "Comment", "Theme", "Comment token color.", "#6B7F95", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.preprocessorColor", "Preprocessor", "Theme", "Preprocessor token color.", "#9C4D7B", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.operatorColor", "Operator", "Theme", "Operator token color.", "#34475D", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.braceColor", "Brace", "Theme", "Brace token color.", "#34475D", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.errorColor", "Error Text", "Theme", "Lexical or syntax error text color.", "#B42318", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.selectionTextColor", "Selection Text", "Theme", "Selected text color.", "#12365C", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.selectionColor", "Selection", "Theme", "Selection background color.", "#D7E8FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.currentLineColor", "Current Line", "Theme", "Current line background color.", "#F3F8FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.searchTextColor", "Search Text", "Theme", "Search match text color.", "#233246", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.searchColor", "Search Result", "Theme", "Search match background color.", "#FFE08A", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.matchedBraceColor", "Matched Brace", "Theme", "Matched brace text color.", "#0F5132", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.matchedBraceBackground", "Matched Brace Back", "Theme", "Matched brace background color.", "#D1F2DD", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.marginColor", "Margin", "Theme", "Margin background color.", "#F0F4F8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.lineNumberColor", "Line Number", "Theme", "Line number text color.", "#6B7F95", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.foldIconColor", "Fold Icon", "Theme", "Fold icon color.", "#52657A", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.markerColor", "Marker", "Theme", "Marker color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.diagnosticErrorColor", "Diagnostic Error", "Theme", "Diagnostic error indicator color.", "#D64856", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.diagnosticWarningColor", "Diagnostic Warning", "Theme", "Diagnostic warning indicator color.", "#D68910", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("theme.diagnosticInfoColor", "Diagnostic Info", "Theme", "Diagnostic info indicator color.", "#317ED6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrFlowGraphProperties[] = {
	UI_DESIGN_PROP("data.nodes", "Nodes", "Data", "Rows: id|type|title|x|y|w|h|summary|runState|runPreview.", "start|event.start|Start|40|150|150|86|Entry|0|\nllm|action.llm|LLM Prompt|260|150|170|96|Prompt step|0|\ncondition|logic.condition|Condition|500|150|170|96|Branch|0|\nend|event.end|End|740|80|150|86|Done|0|\nfallback|event.end|Fallback|740|240|150|86|Retry path|0|", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.ports", "Ports", "Data", "Rows: node|id|title|direction|kind|required|multi|dynamic|dataType.", "start|out|Out|output|control|false|false|false|\nllm|in|In|input|control|true|false|false|\nllm|out|Out|output|control|false|false|false|\ncondition|in|In|input|control|true|false|false|\ncondition|true|True|output|control|false|false|false|\ncondition|false|False|output|control|false|false|false|\nend|in|In|input|control|true|false|false|\nfallback|in|In|input|control|true|false|false|", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.nodeConfigs", "Node Configs", "Data", "Rows: node|key|type|value. Types: string,int,float,bool,array.", "llm|model|string|glm-4\nllm|temperature|float|0.7\ncondition|enabled|bool|true", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.edges", "Edges", "Data", "Rows: id|fromNode|fromPort|toNode|toPort|route|kind|runState|runPreview|routeBias|sourceOffset|targetOffset.", "e_start_llm|start|out|llm|in|1|0|0||0.5|36|36\ne_llm_condition|llm|out|condition|in|3|0|0||0.5|48|48\ne_condition_end|condition|true|end|in|0|0|0||0.45|36|36\ne_condition_fallback|condition|false|fallback|in|2|0|0||0.55|36|36", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.diagnostics", "Diagnostics", "Data", "Rows: severity|code|message|node|edge|path.", "warning|sample.branch|Check branch condition|condition||nodes.condition", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.selectedNode", "Selected Node", "Data", "Initially selected node id.", "condition", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("data.selectedEdge", "Selected Edge", "Data", "Initially selected edge id.", "e_llm_condition", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.panX", "Pan X", "Viewport", "Viewport pan X.", "28", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.panY", "Pan Y", "Viewport", "Viewport pan Y.", "24", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.zoom", "Zoom", "Viewport", "Viewport zoom.", "0.85", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.commandHistoryLimit", "Command History", "Behavior", "Undo/redo command history capacity.", "128", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.defaultRoute", "Default Route", "Behavior", "Default route for edges with empty route.", "0", g_arrFlowRouteEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Canvas background color.", "#F6F8FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.gridColor", "Grid", "Appearance", "Grid color.", "#D6DCE67A", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.nodeColor", "Node", "Appearance", "Node fill color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectedNodeColor", "Selected Node", "Appearance", "Selected node fill color.", "#E2F2FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.nodeBorderColor", "Node Border", "Appearance", "Node border color.", "#4A5D76B4", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.portColor", "Port", "Appearance", "Port color.", "#2A86E6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.edgeColor", "Edge", "Appearance", "Edge color.", "#566C86DC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectedEdgeColor", "Selected Edge", "Appearance", "Selected edge color.", "#1A73E8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_property_def_t g_arrWorkflowProperties[] = {
	UI_DESIGN_PROP("data.nodeTypes", "Node Types", "Data", "Rows: id|title|category|inputs|outputs|version|description|icon.", "start|Start|Flow||out|1|Workflow entry|\nllm.mock|LLM Mock|Action|in|out|1|Prompt processing|\ncondition|Condition|Logic|in|true,false|1|Branch by expression|\nhttp.mock|HTTP Mock|Action|in|out|1|HTTP request|\nvariable.mock|Variable Mock|Data|in|out|1|Variable operation|\nend|End|Flow|in||1|Workflow exit|", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.typePorts", "Type Ports", "Data", "Rows: type|id|title|direction|kind|required|multi|dynamic|dataType.", "start|out|Out|output|control|false|false|false|\nllm.mock|in|In|input|control|true|false|false|\nllm.mock|out|Out|output|control|false|false|false|\ncondition|in|In|input|control|true|false|false|\ncondition|true|True|output|control|false|false|false|\ncondition|false|False|output|control|false|false|false|\nhttp.mock|in|In|input|control|true|false|false|\nhttp.mock|out|Out|output|control|false|false|false|\nvariable.mock|in|In|input|control|true|false|false|\nvariable.mock|out|Out|output|control|false|false|false|\nend|in|In|input|control|true|false|false|", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.configFields", "Config Fields", "Data", "Rows: type|id|title|kind|required|default|min|max|options|children|expressionLanguage|refScope|refType|group|tab.", "llm.mock|model|Model|string|true|glm|||||||||\nllm.mock|temperature|Temperature|float|false|0.7|0|2|||||||\ncondition|expression|Expression|expression|true||||||xexpr||||\ncondition|mode|Mode|select|false|auto|||auto,manual||||||", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.nodes", "Nodes", "Data", "Rows: type|id|title|x|y|w|h|summary|runState|runPreview.", "start|start|Start|30|130|150|86|Entry|0|\nllm.mock|llm|LLM Mock|290|130|170|96|Prompt|2|Running\ncondition|condition|Condition|550|130|170|96|Branch|0|\nend|end|End|810|40|150|86|Done|0|\nhttp.mock|http|HTTP Mock|810|220|170|96|Fallback|0|", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.nodeConfigs", "Node Configs", "Data", "Rows: node|key|type|value. Values are applied after default schema config.", "llm|model|string|glm-4\nllm|temperature|float|0.7\ncondition|expression|expression|${input.ready}\ncondition|mode|string|auto", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.edges", "Edges", "Data", "Rows: id|fromNode|fromPort|toNode|toPort|route|kind|runState|runPreview|routeBias|sourceOffset|targetOffset.", "e_start_llm|start|out|llm|in|1|0|1||0.5|36|36\ne_llm_condition|llm|out|condition|in|3|0|0||0.5|48|48\ne_condition_end|condition|true|end|in|0|0|0||0.45|36|36\ne_condition_http|condition|false|http|in|2|0|0||0.55|36|36", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.variables", "Variables", "Data", "Rows: id|title|type|scope|default.", "var_prompt|Prompt|string|workflow|user_prompt", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.diagnostics", "Diagnostics", "Data", "Rows: severity|code|message|node|edge|path.", "", XUI_TABLE_CELL_TYPE_CUSTOM, 0, UI_DESIGN_PROPERTY_COMPLEX),
	UI_DESIGN_PROP("data.selectedNode", "Selected Node", "Data", "Initially selected node id.", "condition", XUI_TABLE_CELL_TYPE_TEXT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.panX", "Pan X", "Viewport", "Viewport pan X.", "20", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.panY", "Pan Y", "Viewport", "Viewport pan Y.", "90", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("value.zoom", "Zoom", "Viewport", "Viewport zoom.", "0.75", XUI_TABLE_CELL_TYPE_FLOAT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("behavior.commandHistoryLimit", "Command History", "Behavior", "Undo/redo command history capacity.", "128", XUI_TABLE_CELL_TYPE_INT, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_ENUM_PROP("behavior.defaultRoute", "Default Route", "Behavior", "Default route for edges with empty route.", "0", g_arrFlowRouteEnum, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.backgroundColor", "Background", "Appearance", "Canvas background color.", "#F4F8FC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.gridColor", "Grid", "Appearance", "Grid color.", "#D2DCE8BE", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.nodeColor", "Node", "Appearance", "Node fill color.", "#FFFFFF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectedNodeColor", "Selected Node", "Appearance", "Selected node fill color.", "#E2F2FF", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.nodeBorderColor", "Node Border", "Appearance", "Node border color.", "#4A5D76B4", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.portColor", "Port", "Appearance", "Port color.", "#2A86E6", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.edgeColor", "Edge", "Appearance", "Edge color.", "#566C86DC", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE),
	UI_DESIGN_PROP("appearance.selectedEdgeColor", "Selected Edge", "Appearance", "Selected edge color.", "#1A73E8", XUI_TABLE_CELL_TYPE_COLOR, 0, UI_DESIGN_PROPERTY_INLINE)
};

static const ui_design_control_desc_t g_arrControls[] = {
	{UI_DESIGN_NODE_WIDGET, "widget", "Widget", "Containers", UI_DESIGN_CONTROL_CONTAINER, 180.0f, 116.0f, g_arrWidgetProperties, UI_DESIGN_COUNT_OF(g_arrWidgetProperties), __uiDesignCreateWidget, __uiDesignApplyNode},
	{UI_DESIGN_NODE_PANEL, "panel", "Panel", "Containers", UI_DESIGN_CONTROL_CONTAINER, 260.0f, 160.0f, g_arrPanelProperties, UI_DESIGN_COUNT_OF(g_arrPanelProperties), __uiDesignCreatePanel, __uiDesignApplyNode},
	{UI_DESIGN_NODE_LABEL, "label", "Label", "Text", 0u, 120.0f, 26.0f, g_arrLabelProperties, UI_DESIGN_COUNT_OF(g_arrLabelProperties), __uiDesignCreateLabel, __uiDesignApplyNode},
	{UI_DESIGN_NODE_HYPERLINK, "hyperlink", "Hyperlink", "Text", 0u, 140.0f, 26.0f, g_arrHyperlinkProperties, UI_DESIGN_COUNT_OF(g_arrHyperlinkProperties), __uiDesignCreateHyperlink, __uiDesignApplyNode},
	{UI_DESIGN_NODE_INPUT, "input", "Input", "Input", 0u, 180.0f, 30.0f, g_arrInputProperties, UI_DESIGN_COUNT_OF(g_arrInputProperties), __uiDesignCreateInput, __uiDesignApplyNode},
	{UI_DESIGN_NODE_TAG_INPUT, "tag_input", "Tag Input", "Input", 0u, 220.0f, 34.0f, g_arrTagInputProperties, UI_DESIGN_COUNT_OF(g_arrTagInputProperties), __uiDesignCreateTagInput, __uiDesignApplyNode},
	{UI_DESIGN_NODE_NUMERIC_INPUT, "numeric_input", "Numeric Input", "Input", 0u, 150.0f, 30.0f, g_arrNumericInputProperties, UI_DESIGN_COUNT_OF(g_arrNumericInputProperties), __uiDesignCreateNumericInput, __uiDesignApplyNode},
	{UI_DESIGN_NODE_TEXT_EDIT, "text_edit", "Text Edit", "Input", 0u, 280.0f, 160.0f, g_arrTextEditProperties, UI_DESIGN_COUNT_OF(g_arrTextEditProperties), __uiDesignCreateTextEdit, __uiDesignApplyNode},
	{UI_DESIGN_NODE_BUTTON, "button", "Button", "Buttons", 0u, 118.0f, 32.0f, g_arrButtonProperties, UI_DESIGN_COUNT_OF(g_arrButtonProperties), __uiDesignCreateButton, __uiDesignApplyNode},
	{UI_DESIGN_NODE_CHECKBOX, "checkbox", "CheckBox", "Selection", 0u, 148.0f, 28.0f, g_arrCheckProperties, UI_DESIGN_COUNT_OF(g_arrCheckProperties), __uiDesignCreateCheckBox, __uiDesignApplyNode},
	{UI_DESIGN_NODE_RADIO, "radio", "Radio", "Selection", 0u, 132.0f, 28.0f, g_arrCheckProperties, UI_DESIGN_COUNT_OF(g_arrCheckProperties), __uiDesignCreateRadio, __uiDesignApplyNode},
	{UI_DESIGN_NODE_TOGGLE, "toggle", "Toggle", "Selection", 0u, 148.0f, 30.0f, g_arrToggleProperties, UI_DESIGN_COUNT_OF(g_arrToggleProperties), __uiDesignCreateToggle, __uiDesignApplyNode},
	{UI_DESIGN_NODE_SCROLLBAR, "scrollbar", "ScrollBar", "Selection", 0u, 180.0f, 18.0f, g_arrScrollBarProperties, UI_DESIGN_COUNT_OF(g_arrScrollBarProperties), __uiDesignCreateScrollBar, __uiDesignApplyNode},
	{UI_DESIGN_NODE_SLIDER, "slider", "Slider", "Selection", 0u, 180.0f, 28.0f, g_arrSliderProperties, UI_DESIGN_COUNT_OF(g_arrSliderProperties), __uiDesignCreateSlider, __uiDesignApplyNode},
	{UI_DESIGN_NODE_RANGE_SLIDER, "range_slider", "Range Slider", "Selection", 0u, 190.0f, 30.0f, g_arrRangeSliderProperties, UI_DESIGN_COUNT_OF(g_arrRangeSliderProperties), __uiDesignCreateRangeSlider, __uiDesignApplyNode},
	{UI_DESIGN_NODE_COMBOBOX, "combobox", "ComboBox", "Selection", 0u, 180.0f, 30.0f, g_arrComboBoxProperties, UI_DESIGN_COUNT_OF(g_arrComboBoxProperties), __uiDesignCreateComboBox, __uiDesignApplyNode},
	{UI_DESIGN_NODE_PAGE, "page", "Page", "Navigation", 0u, 320.0f, 34.0f, g_arrPageProperties, UI_DESIGN_COUNT_OF(g_arrPageProperties), __uiDesignCreatePage, __uiDesignApplyNode},
	{UI_DESIGN_NODE_CAROUSEL, "carousel", "Carousel", "Navigation", UI_DESIGN_CONTROL_CONTAINER, 260.0f, 150.0f, g_arrCarouselProperties, UI_DESIGN_COUNT_OF(g_arrCarouselProperties), __uiDesignCreateCarousel, __uiDesignApplyNode},
	{UI_DESIGN_NODE_LISTVIEW, "listview", "ListView", "Data", 0u, 220.0f, 150.0f, g_arrListViewProperties, UI_DESIGN_COUNT_OF(g_arrListViewProperties), __uiDesignCreateListView, __uiDesignApplyNode},
	{UI_DESIGN_NODE_TREEVIEW, "treeview", "TreeView", "Data", 0u, 220.0f, 160.0f, g_arrTreeViewProperties, UI_DESIGN_COUNT_OF(g_arrTreeViewProperties), __uiDesignCreateTreeView, __uiDesignApplyNode},
	{UI_DESIGN_NODE_TABLEVIEW, "tableview", "TableView", "Data", 0u, 320.0f, 180.0f, g_arrTableViewProperties, UI_DESIGN_COUNT_OF(g_arrTableViewProperties), __uiDesignCreateTableView, __uiDesignApplyNode},
	{UI_DESIGN_NODE_TABLEGRID, "tablegrid", "TableGrid", "Data", 0u, 320.0f, 180.0f, g_arrTableGridProperties, UI_DESIGN_COUNT_OF(g_arrTableGridProperties), __uiDesignCreateTableGrid, __uiDesignApplyNode},
	{UI_DESIGN_NODE_SEPARATOR, "separator", "Separator", "Display", 0u, 180.0f, 8.0f, g_arrSeparatorProperties, UI_DESIGN_COUNT_OF(g_arrSeparatorProperties), __uiDesignCreateSeparator, __uiDesignApplyNode},
	{UI_DESIGN_NODE_PROGRESS, "progress", "Progress", "Display", 0u, 180.0f, 24.0f, g_arrProgressProperties, UI_DESIGN_COUNT_OF(g_arrProgressProperties), __uiDesignCreateProgress, __uiDesignApplyNode},
	{UI_DESIGN_NODE_STEP_BAR, "step_bar", "StepBar", "Display", 0u, 300.0f, 54.0f, g_arrStepBarProperties, UI_DESIGN_COUNT_OF(g_arrStepBarProperties), __uiDesignCreateStepBar, __uiDesignApplyNode},
	{UI_DESIGN_NODE_CHART, "chart", "Chart", "Display", 0u, 320.0f, 210.0f, g_arrChartProperties, UI_DESIGN_COUNT_OF(g_arrChartProperties), __uiDesignCreateChart, __uiDesignApplyNode},
	{UI_DESIGN_NODE_QRCODE, "qrcode", "QrCode", "Display", 0u, 140.0f, 140.0f, g_arrQrCodeProperties, UI_DESIGN_COUNT_OF(g_arrQrCodeProperties), __uiDesignCreateQrCode, __uiDesignApplyNode},
	{UI_DESIGN_NODE_IMAGE, "image", "Image", "Display", 0u, 160.0f, 100.0f, g_arrImageProperties, UI_DESIGN_COUNT_OF(g_arrImageProperties), __uiDesignCreateImage, __uiDesignApplyNode},
	{UI_DESIGN_NODE_BREADCRUMB, "breadcrumb", "Breadcrumb", "Navigation", 0u, 260.0f, 32.0f, g_arrBreadcrumbProperties, UI_DESIGN_COUNT_OF(g_arrBreadcrumbProperties), __uiDesignCreateBreadcrumb, __uiDesignApplyNode},
	{UI_DESIGN_NODE_CHECK_CARD, "check_card", "CheckCard", "Selection", UI_DESIGN_CONTROL_CONTAINER, 168.0f, 88.0f, g_arrCheckCardProperties, UI_DESIGN_COUNT_OF(g_arrCheckCardProperties), __uiDesignCreateCheckCard, __uiDesignApplyNode},
	{UI_DESIGN_NODE_RADIO_GROUP, "radio_group", "RadioGroup", "Selection", UI_DESIGN_CONTROL_CONTAINER, 180.0f, 96.0f, g_arrRadioGroupProperties, UI_DESIGN_COUNT_OF(g_arrRadioGroupProperties), __uiDesignCreateRadioGroup, __uiDesignApplyNode},
	{UI_DESIGN_NODE_VIRTUAL_JOYSTICK, "virtual_joystick", "VirtualJoystick", "Game", 0u, 132.0f, 132.0f, g_arrVirtualJoystickProperties, UI_DESIGN_COUNT_OF(g_arrVirtualJoystickProperties), __uiDesignCreateVirtualJoystick, __uiDesignApplyNode},
	{UI_DESIGN_NODE_INVENTORY_GRID, "inventory_grid", "InventoryGrid", "Game", 0u, 264.0f, 196.0f, g_arrInventoryGridProperties, UI_DESIGN_COUNT_OF(g_arrInventoryGridProperties), __uiDesignCreateInventoryGrid, __uiDesignApplyNode},
	{UI_DESIGN_NODE_TERMINAL, "terminal", "Terminal", "Editors", 0u, 340.0f, 200.0f, g_arrTerminalProperties, UI_DESIGN_COUNT_OF(g_arrTerminalProperties), __uiDesignCreateTerminal, __uiDesignApplyNode},
	{UI_DESIGN_NODE_SPLIT_LAYOUT, "split_layout", "SplitLayout", "Containers", UI_DESIGN_CONTROL_CONTAINER, 300.0f, 180.0f, g_arrSplitLayoutProperties, UI_DESIGN_COUNT_OF(g_arrSplitLayoutProperties), __uiDesignCreateSplitLayout, __uiDesignApplyNode},
	{UI_DESIGN_NODE_TABS, "tabs", "Tabs", "Containers", UI_DESIGN_CONTROL_CONTAINER, 300.0f, 180.0f, g_arrTabsProperties, UI_DESIGN_COUNT_OF(g_arrTabsProperties), __uiDesignCreateTabs, __uiDesignApplyNode},
	{UI_DESIGN_NODE_ACCORDION, "accordion", "Accordion", "Containers", UI_DESIGN_CONTROL_CONTAINER, 280.0f, 190.0f, g_arrAccordionProperties, UI_DESIGN_COUNT_OF(g_arrAccordionProperties), __uiDesignCreateAccordion, __uiDesignApplyNode},
	{UI_DESIGN_NODE_WINDOW, "window", "Window", "Containers", UI_DESIGN_CONTROL_CONTAINER, 280.0f, 190.0f, g_arrWindowProperties, UI_DESIGN_COUNT_OF(g_arrWindowProperties), __uiDesignCreateWindow, __uiDesignApplyNode},
	{UI_DESIGN_NODE_SCROLL_FRAME, "scroll_frame", "ScrollFrame", "Containers", UI_DESIGN_CONTROL_CONTAINER, 260.0f, 170.0f, g_arrScrollContainerProperties, UI_DESIGN_COUNT_OF(g_arrScrollContainerProperties), __uiDesignCreateScrollFrame, __uiDesignApplyNode},
	{UI_DESIGN_NODE_SCROLL_VIEW, "scroll_view", "ScrollView", "Containers", UI_DESIGN_CONTROL_CONTAINER, 260.0f, 170.0f, g_arrScrollContainerProperties, UI_DESIGN_COUNT_OF(g_arrScrollContainerProperties), __uiDesignCreateScrollView, __uiDesignApplyNode},
	{UI_DESIGN_NODE_CANVAS, "canvas", "Canvas", "Display", 0u, 260.0f, 170.0f, g_arrCanvasProperties, UI_DESIGN_COUNT_OF(g_arrCanvasProperties), __uiDesignCreateCanvas, __uiDesignApplyNode},
	{UI_DESIGN_NODE_MESSAGE_LIST, "message_list", "MessageList", "Data", 0u, 300.0f, 220.0f, g_arrMessageListProperties, UI_DESIGN_COUNT_OF(g_arrMessageListProperties), __uiDesignCreateMessageList, __uiDesignApplyNode},
	{UI_DESIGN_NODE_TIMELINE_VIEW, "timeline_view", "TimeLineView", "Data", 0u, 360.0f, 220.0f, g_arrTimelineProperties, UI_DESIGN_COUNT_OF(g_arrTimelineProperties), __uiDesignCreateTimelineView, __uiDesignApplyNode},
	{UI_DESIGN_NODE_PROPERTY_GRID, "property_grid", "PropertyGrid", "Data", 0u, 300.0f, 220.0f, g_arrPropertyGridProperties, UI_DESIGN_COUNT_OF(g_arrPropertyGridProperties), __uiDesignCreatePropertyGrid, __uiDesignApplyNode},
	{UI_DESIGN_NODE_MENU_BAR, "menubar", "MenuBar", "Bars", 0u, 340.0f, 30.0f, g_arrMenuBarProperties, UI_DESIGN_COUNT_OF(g_arrMenuBarProperties), __uiDesignCreateMenuBar, __uiDesignApplyNode},
	{UI_DESIGN_NODE_TOOLBAR, "toolbar", "Toolbar", "Bars", 0u, 340.0f, 38.0f, g_arrToolbarProperties, UI_DESIGN_COUNT_OF(g_arrToolbarProperties), __uiDesignCreateToolbar, __uiDesignApplyNode},
	{UI_DESIGN_NODE_STATUS_BAR, "statusbar", "StatusBar", "Bars", 0u, 360.0f, 28.0f, g_arrStatusBarProperties, UI_DESIGN_COUNT_OF(g_arrStatusBarProperties), __uiDesignCreateStatusBar, __uiDesignApplyNode},
	{UI_DESIGN_NODE_DOCK_PANEL, "dock_panel", "DockPanel", "Containers", UI_DESIGN_CONTROL_CONTAINER, 420.0f, 260.0f, g_arrDockPanelProperties, UI_DESIGN_COUNT_OF(g_arrDockPanelProperties), __uiDesignCreateDockPanel, __uiDesignApplyNode},
	{UI_DESIGN_NODE_POPUP, "popup", "Popup", "Overlay", UI_DESIGN_CONTROL_CONTAINER, 260.0f, 180.0f, g_arrPopupProperties, UI_DESIGN_COUNT_OF(g_arrPopupProperties), __uiDesignCreatePopup, __uiDesignApplyNode},
	{UI_DESIGN_NODE_MENU, "menu", "Menu", "Overlay", 0u, 210.0f, 132.0f, g_arrMenuProperties, UI_DESIGN_COUNT_OF(g_arrMenuProperties), __uiDesignCreateMenu, __uiDesignApplyNode},
	{UI_DESIGN_NODE_MSG_BOX, "msgbox", "MsgBox", "Overlay", 0u, 320.0f, 190.0f, g_arrMsgBoxProperties, UI_DESIGN_COUNT_OF(g_arrMsgBoxProperties), __uiDesignCreateOverlayPreview, __uiDesignApplyNode},
	{UI_DESIGN_NODE_FILE_DIALOG, "file_dialog", "FileDialog", "Overlay", 0u, 380.0f, 240.0f, g_arrFileDialogProperties, UI_DESIGN_COUNT_OF(g_arrFileDialogProperties), __uiDesignCreateOverlayPreview, __uiDesignApplyNode},
	{UI_DESIGN_NODE_MSG_TIP, "msgtip", "MsgTip", "Overlay", 0u, 220.0f, 48.0f, g_arrMsgTipProperties, UI_DESIGN_COUNT_OF(g_arrMsgTipProperties), __uiDesignCreateOverlayPreview, __uiDesignApplyNode},
	{UI_DESIGN_NODE_TOAST, "toast", "Toast", "Overlay", 0u, 320.0f, 170.0f, g_arrToastProperties, UI_DESIGN_COUNT_OF(g_arrToastProperties), __uiDesignCreateOverlayPreview, __uiDesignApplyNode},
	{UI_DESIGN_NODE_CASCADER, "cascader", "Cascader", "Pickers", 0u, 190.0f, 30.0f, g_arrCascaderProperties, UI_DESIGN_COUNT_OF(g_arrCascaderProperties), __uiDesignCreateCascader, __uiDesignApplyNode},
	{UI_DESIGN_NODE_COLOR_PICKER, "color_picker", "ColorPicker", "Pickers", 0u, 190.0f, 30.0f, g_arrColorPickerProperties, UI_DESIGN_COUNT_OF(g_arrColorPickerProperties), __uiDesignCreateColorPicker, __uiDesignApplyNode},
	{UI_DESIGN_NODE_DATE_PICKER, "date_picker", "DatePicker", "Pickers", 0u, 190.0f, 30.0f, g_arrDatePickerProperties, UI_DESIGN_COUNT_OF(g_arrDatePickerProperties), __uiDesignCreateDatePicker, __uiDesignApplyNode},
	{UI_DESIGN_NODE_CODE_EDIT, "code_edit", "CodeEdit", "Editors", 0u, 360.0f, 220.0f, g_arrCodeEditProperties, UI_DESIGN_COUNT_OF(g_arrCodeEditProperties), __uiDesignCreateCodeEdit, __uiDesignApplyNode},
	{UI_DESIGN_NODE_FLOW_GRAPH, "flow_graph", "FlowGraph", "Graphs", 0u, 420.0f, 260.0f, g_arrFlowGraphProperties, UI_DESIGN_COUNT_OF(g_arrFlowGraphProperties), __uiDesignCreateFlowGraph, __uiDesignApplyNode},
	{UI_DESIGN_NODE_WORKFLOW, "workflow", "Workflow", "Graphs", 0u, 460.0f, 280.0f, g_arrWorkflowProperties, UI_DESIGN_COUNT_OF(g_arrWorkflowProperties), __uiDesignCreateWorkflow, __uiDesignApplyNode}
};

static const char* g_arrCategories[] = {
	"Containers",
	"Text",
	"Input",
	"Buttons",
	"Selection",
	"Navigation",
	"Data",
	"Display",
	"Bars",
	"Overlay",
	"Pickers",
	"Editors",
	"Graphs",
	"Game"
};

static uint32_t __uiDesignColor(const ui_design_node_t* pNode, const char* sId, uint32_t iDefault)
{
	return uiDesignNodeGetPropertyColor(pNode, sId, iDefault);
}

static float __uiDesignFloat(const ui_design_node_t* pNode, const char* sId, float fDefault)
{
	return uiDesignNodeGetPropertyFloat(pNode, sId, fDefault);
}

static int __uiDesignInt(const ui_design_node_t* pNode, const char* sId, int iDefault)
{
	return uiDesignNodeGetPropertyInt(pNode, sId, iDefault);
}

static int __uiDesignBool(const ui_design_node_t* pNode, const char* sId, int bDefault)
{
	return uiDesignNodeGetPropertyBool(pNode, sId, bDefault);
}

static const char* __uiDesignText(const ui_design_node_t* pNode, const char* sId, const char* sDefault)
{
	return uiDesignNodeGetProperty(pNode, sId, sDefault);
}

static int __uiDesignHasProperty(const ui_design_node_t* pNode, const char* sId)
{
	int i;

	if ( (pNode == NULL) || (sId == NULL) ) return 0;
	for ( i = 0; i < pNode->iPropertyCount; i++ ) {
		if ( strcmp(pNode->arrProperties[i].sId, sId) == 0 ) return 1;
	}
	return 0;
}

static int __uiDesignTryFloat(const ui_design_node_t* pNode, const char* sId, float* pValue)
{
	if ( !__uiDesignHasProperty(pNode, sId) || (pValue == NULL) ) return 0;
	*pValue = __uiDesignFloat(pNode, sId, 0.0f);
	return 1;
}

static int __uiDesignTryInt(const ui_design_node_t* pNode, const char* sId, int* pValue)
{
	if ( !__uiDesignHasProperty(pNode, sId) || (pValue == NULL) ) return 0;
	*pValue = __uiDesignInt(pNode, sId, 0);
	return 1;
}

static int __uiDesignTryBool(const ui_design_node_t* pNode, const char* sId, int* pValue)
{
	if ( !__uiDesignHasProperty(pNode, sId) || (pValue == NULL) ) return 0;
	*pValue = __uiDesignBool(pNode, sId, 0);
	return 1;
}

typedef struct ui_design_flow_port_seen_t {
	char sNode[64];
	char sPort[64];
	int iDirection;
} ui_design_flow_port_seen_t;

typedef struct ui_design_workflow_port_text_t {
	char sId[64];
	char sTitle[64];
	char sDataType[64];
} ui_design_workflow_port_text_t;

typedef struct ui_design_node_config_table_t {
	char sNode[64];
	xvalue pConfig;
} ui_design_node_config_table_t;

typedef struct ui_design_split_pane_def_t {
	const char* sTitle;
	int iMode;
	int bModeSet;
	float fWeight;
	float fFixedSize;
	float fMinSize;
	float fMaxSize;
} ui_design_split_pane_def_t;

typedef struct ui_design_msgbox_button_preview_t {
	const char* sText;
	int iResult;
	int iSemantic;
} ui_design_msgbox_button_preview_t;

typedef struct ui_design_file_dialog_root_preview_t {
	const char* sLabel;
	const char* sPath;
	int bSelected;
	int bEnabled;
} ui_design_file_dialog_root_preview_t;

typedef struct ui_design_file_dialog_entry_preview_t {
	const char* sName;
	const char* sKind;
	const char* sSize;
	const char* sModified;
	int bSelected;
	int bEnabled;
} ui_design_file_dialog_entry_preview_t;

typedef struct ui_design_dock_window_def_t {
	const char* sTitle;
	const char* sContent;
	const char* sPane;
	int iSide;
	int iRegion;
	float fRatio;
	xui_rect_t tFloatRect;
	int bClosable;
	int bDockable;
	int bActive;
	int bAutoHide;
	int bHidden;
	int bFloating;
	int iWindow;
	int iPane;
} ui_design_dock_window_def_t;

typedef struct ui_design_chart_series_def_t {
	const char* sId;
	const char* sName;
	const char* sDash;
	int iType;
	int bVisible;
	uint32_t iColor;
	int bAreaFill;
	uint32_t iAreaColor;
	int bSmooth;
	int iSymbol;
	float fSymbolSize;
	float fRadiusMin;
	float fRadiusMax;
	int bValueColor;
	uint32_t iValueMinColor;
	uint32_t iValueMaxColor;
} ui_design_chart_series_def_t;

enum {
	UI_DESIGN_INPUT_MENU_TARGET_INPUT = 0,
	UI_DESIGN_INPUT_MENU_TARGET_TAG_INPUT,
	UI_DESIGN_INPUT_MENU_TARGET_NUMERIC_INPUT,
	UI_DESIGN_INPUT_MENU_TARGET_TEXT_EDIT,
	UI_DESIGN_INPUT_MENU_TARGET_COMBOBOX
};

static void __uiDesignCopyText(char* sDst, int iCapacity, const char* sSrc)
{
	size_t iLen;

	if ( (sDst == NULL) || (iCapacity <= 0) ) return;
	if ( sSrc == NULL ) sSrc = "";
	iLen = strlen(sSrc);
	if ( iLen >= (size_t)iCapacity ) iLen = (size_t)iCapacity - 1u;
	if ( iLen > 0u ) memcpy(sDst, sSrc, iLen);
	sDst[iLen] = 0;
}

static void __uiDesignTrim(char* sText)
{
	char* sStart;
	size_t iLen;

	if ( sText == NULL ) return;
	sStart = sText;
	while ( *sStart == ' ' || *sStart == '\t' || *sStart == '\r' || *sStart == '\n' ) {
		++sStart;
	}
	if ( sStart != sText ) {
		memmove(sText, sStart, strlen(sStart) + 1u);
	}
	iLen = strlen(sText);
	while ( iLen > 0u && (sText[iLen - 1u] == ' ' || sText[iLen - 1u] == '\t' || sText[iLen - 1u] == '\r' || sText[iLen - 1u] == '\n') ) {
		sText[--iLen] = 0;
	}
}

static int __uiDesignNextLine(const char** ppText, char* sLine, int iCapacity)
{
	const char* sText;
	int i;

	if ( (ppText == NULL) || (sLine == NULL) || (iCapacity <= 0) ) return 0;
	sText = *ppText;
	if ( (sText == NULL) || (sText[0] == 0) ) return 0;
	i = 0;
	while ( sText[0] != 0 && sText[0] != '\n' ) {
		if ( i < iCapacity - 1 ) {
			sLine[i++] = sText[0];
		}
		++sText;
	}
	if ( sText[0] == '\n' ) ++sText;
	sLine[i] = 0;
	__uiDesignTrim(sLine);
	*ppText = sText;
	return 1;
}

static int __uiDesignSplitFields(char* sLine, char** arrFields, int iCapacity)
{
	char* sCursor;
	int iCount;
	int i;

	if ( (sLine == NULL) || (arrFields == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	arrFields[iCount++] = sLine;
	sCursor = sLine;
	while ( sCursor[0] != 0 ) {
		if ( sCursor[0] == '|' ) {
			sCursor[0] = 0;
			if ( iCount < iCapacity ) {
				arrFields[iCount++] = sCursor + 1;
			}
		}
		++sCursor;
	}
	for ( i = 0; i < iCount; ++i ) {
		__uiDesignTrim(arrFields[i]);
	}
	return iCount;
}

static int __uiDesignSplitCommaList(const char* sText, char arrItems[][64], int iCapacity)
{
	char sBuffer[512];
	char* sCursor;
	char* sItem;
	int iOut;

	if ( (arrItems == NULL) || (iCapacity <= 0) ) return 0;
	__uiDesignCopyText(sBuffer, sizeof(sBuffer), sText);
	sCursor = sBuffer;
	iOut = 0;
	while ( sCursor[0] != 0 && iOut < iCapacity ) {
		sItem = sCursor;
		while ( sCursor[0] != 0 && sCursor[0] != ',' && sCursor[0] != ';' ) ++sCursor;
		if ( sCursor[0] == ',' || sCursor[0] == ';' ) {
			sCursor[0] = 0;
			++sCursor;
		}
		__uiDesignTrim(sItem);
		if ( sItem[0] == 0 ) continue;
		__uiDesignCopyText(arrItems[iOut], 64, sItem);
		++iOut;
	}
	return iOut;
}

static const char* __uiDesignField(char** arrFields, int iCount, int iIndex, const char* sDefault)
{
	if ( (arrFields == NULL) || (iIndex < 0) || (iIndex >= iCount) || (arrFields[iIndex] == NULL) || (arrFields[iIndex][0] == 0) ) {
		return sDefault;
	}
	return arrFields[iIndex];
}

static float __uiDesignParseFloatText(const char* sText, float fDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return fDefault;
	return (float)atof(sText);
}

static int __uiDesignParseIntText(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	return atoi(sText);
}

static int __uiDesignLower(int c)
{
	if ( c >= 'A' && c <= 'Z' ) return c - 'A' + 'a';
	return c;
}

static int __uiDesignTextEqualsNoCase(const char* sA, const char* sB)
{
	int a;
	int b;

	if ( (sA == NULL) || (sB == NULL) ) return 0;
	while ( sA[0] != 0 && sB[0] != 0 ) {
		a = __uiDesignLower((unsigned char)sA[0]);
		b = __uiDesignLower((unsigned char)sB[0]);
		if ( a != b ) return 0;
		++sA;
		++sB;
	}
	return (sA[0] == 0 && sB[0] == 0);
}

static int __uiDesignParseBoolText(const char* sText, int bDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return bDefault;
	if ( __uiDesignTextEqualsNoCase(sText, "true") || __uiDesignTextEqualsNoCase(sText, "yes") ||
	     __uiDesignTextEqualsNoCase(sText, "on") || __uiDesignTextEqualsNoCase(sText, "enabled") ||
	     __uiDesignTextEqualsNoCase(sText, "checked") || strcmp(sText, "1") == 0 ) {
		return 1;
	}
	if ( __uiDesignTextEqualsNoCase(sText, "false") || __uiDesignTextEqualsNoCase(sText, "no") ||
	     __uiDesignTextEqualsNoCase(sText, "off") || __uiDesignTextEqualsNoCase(sText, "disabled") ||
	     __uiDesignTextEqualsNoCase(sText, "unchecked") || strcmp(sText, "0") == 0 ) {
		return 0;
	}
	return bDefault;
}

static int __uiDesignIsBoolText(const char* sText)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return 0;
	return __uiDesignTextEqualsNoCase(sText, "true") || __uiDesignTextEqualsNoCase(sText, "yes") ||
		__uiDesignTextEqualsNoCase(sText, "on") || __uiDesignTextEqualsNoCase(sText, "enabled") ||
		__uiDesignTextEqualsNoCase(sText, "checked") || strcmp(sText, "1") == 0 ||
		__uiDesignTextEqualsNoCase(sText, "false") || __uiDesignTextEqualsNoCase(sText, "no") ||
		__uiDesignTextEqualsNoCase(sText, "off") || __uiDesignTextEqualsNoCase(sText, "disabled") ||
		__uiDesignTextEqualsNoCase(sText, "unchecked") || strcmp(sText, "0") == 0;
}

static int __uiDesignTokenIs(const char* sText, const char* sToken)
{
	return __uiDesignTextEqualsNoCase(sText, sToken);
}

static int __uiDesignTextEndsWithNoCase(const char* sText, const char* sSuffix)
{
	size_t iTextLen;
	size_t iSuffixLen;

	if ( (sText == NULL) || (sSuffix == NULL) ) return 0;
	iTextLen = strlen(sText);
	iSuffixLen = strlen(sSuffix);
	if ( iSuffixLen > iTextLen ) return 0;
	return __uiDesignTextEqualsNoCase(sText + iTextLen - iSuffixLen, sSuffix);
}

static uint32_t __uiDesignResolveFontFlags(const ui_design_node_t* pNode, const char* sPath)
{
	int iFormat;

	iFormat = __uiDesignInt(pNode, "font.format", 0);
	if ( iFormat == XUI_FONT_FORMAT_TTF || iFormat == XUI_FONT_FORMAT_XRF ) return (uint32_t)iFormat;
	if ( __uiDesignTextEndsWithNoCase(sPath, ".xrf") ) return XUI_FONT_FORMAT_XRF;
	return XUI_FONT_FORMAT_TTF;
}

static void __uiDesignClearNodeRuntimeFont(struct ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	if ( (pApp == NULL) || (pNode == NULL) || (pNode->pRuntimeFont == NULL) ) return;
	if ( pApp->tProxy.fontDestroy != NULL ) {
		pApp->tProxy.fontDestroy(&pApp->tProxy, pNode->pRuntimeFont);
	}
	pNode->pRuntimeFont = NULL;
	pNode->sRuntimeFontSource[0] = '\0';
	pNode->fRuntimeFontSize = 0.0f;
	pNode->iRuntimeFontFlags = 0u;
}

static xui_font __uiDesignResolveNodeFont(struct ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	const char* sPath;
	const char* sName;
	xui_font pFont;
	float fSize;
	uint32_t iFlags;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) ) return NULL;
	sPath = __uiDesignText(pNode, "font.path", "");
	if ( sPath != NULL && sPath[0] != 0 ) {
		fSize = __uiDesignFloat(pNode, "font.size", 13.0f);
		if ( fSize <= 0.0f ) fSize = 13.0f;
		iFlags = __uiDesignResolveFontFlags(pNode, sPath);
		if ( pNode->pRuntimeFont != NULL &&
		     strcmp(pNode->sRuntimeFontSource, sPath) == 0 &&
		     pNode->fRuntimeFontSize == fSize &&
		     pNode->iRuntimeFontFlags == iFlags ) {
			return pNode->pRuntimeFont;
		}
		__uiDesignClearNodeRuntimeFont(pApp, pNode);
		if ( pApp->tProxy.fontLoadFile != NULL ) {
			pFont = NULL;
			iRet = pApp->tProxy.fontLoadFile(&pApp->tProxy, &pFont, sPath, fSize, iFlags);
			if ( iRet == XUI_OK && pFont != NULL ) {
				pNode->pRuntimeFont = pFont;
				__uiDesignCopyText(pNode->sRuntimeFontSource, sizeof(pNode->sRuntimeFontSource), sPath);
				pNode->fRuntimeFontSize = fSize;
				pNode->iRuntimeFontFlags = iFlags;
				return pFont;
			}
		}
		return (pApp->pFont != NULL) ? pApp->pFont : xuiGetDefaultFont(pApp->pContext);
	}
	__uiDesignClearNodeRuntimeFont(pApp, pNode);
	sName = __uiDesignText(pNode, "font.name", "");
	if ( sName != NULL && sName[0] != 0 ) {
		pFont = xuiFindFont(pApp->pContext, sName);
		if ( pFont != NULL ) return pFont;
	}
	return (pApp->pFont != NULL) ? pApp->pFont : xuiGetDefaultFont(pApp->pContext);
}

static int __uiDesignApplyNodeFont(struct ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	xui_widget pMenuContent;
	xui_widget pMenuWidget;
	xui_font pFont;

	if ( (pApp == NULL) || (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	switch ( pNode->iType ) {
	case UI_DESIGN_NODE_LABEL:
	case UI_DESIGN_NODE_HYPERLINK:
	case UI_DESIGN_NODE_BUTTON:
	case UI_DESIGN_NODE_CHECKBOX:
	case UI_DESIGN_NODE_RADIO:
	case UI_DESIGN_NODE_TOGGLE:
	case UI_DESIGN_NODE_INPUT:
	case UI_DESIGN_NODE_TAG_INPUT:
	case UI_DESIGN_NODE_NUMERIC_INPUT:
	case UI_DESIGN_NODE_TEXT_EDIT:
	case UI_DESIGN_NODE_PANEL:
	case UI_DESIGN_NODE_PROGRESS:
	case UI_DESIGN_NODE_STEP_BAR:
	case UI_DESIGN_NODE_CHART:
	case UI_DESIGN_NODE_PAGE:
	case UI_DESIGN_NODE_CAROUSEL:
	case UI_DESIGN_NODE_COMBOBOX:
	case UI_DESIGN_NODE_LISTVIEW:
	case UI_DESIGN_NODE_TREEVIEW:
	case UI_DESIGN_NODE_TABLEVIEW:
	case UI_DESIGN_NODE_TABLEGRID:
	case UI_DESIGN_NODE_BREADCRUMB:
	case UI_DESIGN_NODE_INVENTORY_GRID:
	case UI_DESIGN_NODE_TERMINAL:
	case UI_DESIGN_NODE_TABS:
	case UI_DESIGN_NODE_ACCORDION:
	case UI_DESIGN_NODE_WINDOW:
	case UI_DESIGN_NODE_MESSAGE_LIST:
	case UI_DESIGN_NODE_TIMELINE_VIEW:
	case UI_DESIGN_NODE_PROPERTY_GRID:
	case UI_DESIGN_NODE_MENU_BAR:
	case UI_DESIGN_NODE_TOOLBAR:
	case UI_DESIGN_NODE_STATUS_BAR:
	case UI_DESIGN_NODE_DOCK_PANEL:
	case UI_DESIGN_NODE_MENU:
	case UI_DESIGN_NODE_MSG_BOX:
	case UI_DESIGN_NODE_FILE_DIALOG:
	case UI_DESIGN_NODE_MSG_TIP:
	case UI_DESIGN_NODE_TOAST:
	case UI_DESIGN_NODE_CASCADER:
	case UI_DESIGN_NODE_COLOR_PICKER:
	case UI_DESIGN_NODE_DATE_PICKER:
	case UI_DESIGN_NODE_CODE_EDIT:
		break;
	default:
		return XUI_OK;
	}
	pFont = __uiDesignResolveNodeFont(pApp, pNode);
	if ( pFont == NULL ) return XUI_OK;
	switch ( pNode->iType ) {
	case UI_DESIGN_NODE_LABEL:
		(void)xuiLabelSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_HYPERLINK:
		(void)xuiHyperlinkSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_BUTTON:
		(void)xuiButtonSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_CHECKBOX:
		(void)xuiCheckBoxSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_RADIO:
		(void)xuiRadioSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_TOGGLE:
		(void)xuiToggleSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_INPUT:
		(void)xuiInputSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_TAG_INPUT:
		(void)xuiTagInputSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_NUMERIC_INPUT:
		(void)xuiNumericInputSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_TEXT_EDIT:
		(void)xuiTextEditSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_PANEL:
		(void)xuiPanelSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_PROGRESS:
		(void)xuiProgressSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_STEP_BAR:
		(void)xuiStepBarSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_CHART:
		(void)xuiChartSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_PAGE:
		(void)xuiPageSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_CAROUSEL:
		(void)xuiCarouselSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_COMBOBOX:
		(void)xuiComboBoxSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_LISTVIEW:
		(void)xuiListViewSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_TREEVIEW:
		(void)xuiTreeViewSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_TABLEVIEW:
		(void)xuiTableViewSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_TABLEGRID:
		(void)xuiTableGridSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_BREADCRUMB:
		(void)xuiBreadcrumbSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_INVENTORY_GRID:
		(void)xuiInventoryGridSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_TERMINAL:
		(void)xuiTerminalSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_TABS:
		(void)xuiTabsSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_ACCORDION:
		(void)xuiAccordionSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_WINDOW:
		(void)xuiWindowSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_MESSAGE_LIST:
		(void)xuiMessageListSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_TIMELINE_VIEW:
		(void)xuiTimeLineViewSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_PROPERTY_GRID:
		(void)xuiPropertyGridSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_MENU_BAR:
		(void)xuiMenuBarSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_TOOLBAR:
		(void)xuiToolbarSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_STATUS_BAR:
		(void)xuiStatusBarSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_DOCK_PANEL:
		(void)xuiDockPanelSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_MENU:
		pMenuContent = xuiPopupGetContentWidget(pNode->pWidget);
		pMenuWidget = (pMenuContent != NULL) ? xuiWidgetGetFirstChild(pMenuContent) : NULL;
		if ( pMenuWidget != NULL ) (void)xuiMenuSetFont(pMenuWidget, pFont);
		break;
	case UI_DESIGN_NODE_MSG_BOX:
	case UI_DESIGN_NODE_FILE_DIALOG:
	case UI_DESIGN_NODE_MSG_TIP:
	case UI_DESIGN_NODE_TOAST:
		(void)xuiPanelSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_CASCADER:
		(void)xuiCascaderSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_COLOR_PICKER:
		(void)xuiColorPickerSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_DATE_PICKER:
		(void)xuiDatePickerSetFont(pNode->pWidget, pFont);
		break;
	case UI_DESIGN_NODE_CODE_EDIT:
		(void)xuiCodeEditSetFont(pNode->pWidget, pFont);
		break;
	default:
		break;
	}
	return XUI_OK;
}

static int __uiDesignInputMenuCommandFromText(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "undo") ) return XUI_INPUT_MENU_UNDO;
	if ( __uiDesignTokenIs(sText, "redo") ) return XUI_INPUT_MENU_REDO;
	if ( __uiDesignTokenIs(sText, "cut") ) return XUI_INPUT_MENU_CUT;
	if ( __uiDesignTokenIs(sText, "copy") ) return XUI_INPUT_MENU_COPY;
	if ( __uiDesignTokenIs(sText, "paste") ) return XUI_INPUT_MENU_PASTE;
	if ( __uiDesignTokenIs(sText, "delete") || __uiDesignTokenIs(sText, "del") ) return XUI_INPUT_MENU_DELETE;
	if ( __uiDesignTokenIs(sText, "selectAll") || __uiDesignTokenIs(sText, "select_all") ||
	     __uiDesignTokenIs(sText, "select-all") || __uiDesignTokenIs(sText, "all") ) {
		return XUI_INPUT_MENU_SELECT_ALL;
	}
	return __uiDesignParseIntText(sText, iDefault);
}

static int __uiDesignSetInputMenuTitle(xui_widget pWidget, int iTarget, int iCommand, const char* sTitle)
{
	xui_widget pInputWidget;

	if ( (pWidget == NULL) || (iCommand < 0) || (iCommand >= XUI_INPUT_MENU_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	switch ( iTarget ) {
	case UI_DESIGN_INPUT_MENU_TARGET_INPUT:
		return xuiInputSetMenuTitle(pWidget, iCommand, sTitle);
	case UI_DESIGN_INPUT_MENU_TARGET_TAG_INPUT:
		pInputWidget = xuiTagInputGetInputWidget(pWidget);
		return (pInputWidget != NULL) ? xuiInputSetMenuTitle(pInputWidget, iCommand, sTitle) : XUI_ERROR_INVALID_ARGUMENT;
	case UI_DESIGN_INPUT_MENU_TARGET_NUMERIC_INPUT:
		return xuiNumericInputSetMenuTitle(pWidget, iCommand, sTitle);
	case UI_DESIGN_INPUT_MENU_TARGET_TEXT_EDIT:
		return xuiTextEditSetMenuTitle(pWidget, iCommand, sTitle);
	case UI_DESIGN_INPUT_MENU_TARGET_COMBOBOX:
		return xuiComboBoxSetInputMenuTitle(pWidget, iCommand, sTitle);
	default:
		break;
	}
	return XUI_ERROR_INVALID_ARGUMENT;
}

static int __uiDesignApplyInputMenuTitles(ui_design_node_t* pNode, xui_widget pWidget, int iTarget)
{
	const char* sCursor;
	char sLine[512];
	char* arrFields[2];
	int iFieldCount;
	int iCommand;
	int i;

	if ( (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < XUI_INPUT_MENU_COUNT; ++i ) {
		(void)__uiDesignSetInputMenuTitle(pWidget, iTarget, i, "");
	}
	sCursor = __uiDesignText(pNode, "data.menuTitles", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount < 2) || (arrFields[0][0] == 0) ) continue;
		iCommand = __uiDesignInputMenuCommandFromText(arrFields[0], -1);
		if ( iCommand < 0 || iCommand >= XUI_INPUT_MENU_COUNT ) continue;
		(void)__uiDesignSetInputMenuTitle(pWidget, iTarget, iCommand, arrFields[1]);
	}
	return XUI_OK;
}

static int __uiDesignCodeEditMenuCommandFromText(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "undo") ) return XUI_CODE_COMMAND_UNDO;
	if ( __uiDesignTokenIs(sText, "redo") ) return XUI_CODE_COMMAND_REDO;
	if ( __uiDesignTokenIs(sText, "cut") ) return XUI_CODE_COMMAND_CUT;
	if ( __uiDesignTokenIs(sText, "copy") ) return XUI_CODE_COMMAND_COPY;
	if ( __uiDesignTokenIs(sText, "paste") ) return XUI_CODE_COMMAND_PASTE;
	if ( __uiDesignTokenIs(sText, "delete") || __uiDesignTokenIs(sText, "del") ||
	     __uiDesignTokenIs(sText, "deleteForward") || __uiDesignTokenIs(sText, "delete_forward") ) {
		return XUI_CODE_COMMAND_DELETE_FORWARD;
	}
	if ( __uiDesignTokenIs(sText, "selectAll") || __uiDesignTokenIs(sText, "select_all") ||
	     __uiDesignTokenIs(sText, "select-all") || __uiDesignTokenIs(sText, "all") ) {
		return XUI_CODE_COMMAND_SELECT_ALL;
	}
	if ( __uiDesignTokenIs(sText, "find") || __uiDesignTokenIs(sText, "openFind") ||
	     __uiDesignTokenIs(sText, "open_find") ) {
		return XUI_CODE_COMMAND_OPEN_FIND;
	}
	if ( __uiDesignTokenIs(sText, "replace") || __uiDesignTokenIs(sText, "openReplace") ||
	     __uiDesignTokenIs(sText, "open_replace") ) {
		return XUI_CODE_COMMAND_OPEN_REPLACE;
	}
	if ( __uiDesignTokenIs(sText, "gotoLine") || __uiDesignTokenIs(sText, "goToLine") ||
	     __uiDesignTokenIs(sText, "goto") || __uiDesignTokenIs(sText, "goto_line") ) {
		return XUI_CODE_COMMAND_GOTO_LINE;
	}
	if ( __uiDesignTokenIs(sText, "toggleComment") || __uiDesignTokenIs(sText, "comment") ||
	     __uiDesignTokenIs(sText, "toggle_line_comment") ) {
		return XUI_CODE_COMMAND_TOGGLE_LINE_COMMENT;
	}
	if ( __uiDesignTokenIs(sText, "toggleFold") || __uiDesignTokenIs(sText, "foldToggle") ||
	     __uiDesignTokenIs(sText, "toggle_fold") ) {
		return XUI_CODE_COMMAND_FOLD_TOGGLE;
	}
	return __uiDesignParseIntText(sText, iDefault);
}

static int __uiDesignApplyCodeEditMenuTitles(ui_design_node_t* pNode, xui_widget pWidget)
{
	static const int arrCommands[] = {
		XUI_CODE_COMMAND_UNDO,
		XUI_CODE_COMMAND_REDO,
		XUI_CODE_COMMAND_CUT,
		XUI_CODE_COMMAND_COPY,
		XUI_CODE_COMMAND_PASTE,
		XUI_CODE_COMMAND_DELETE_FORWARD,
		XUI_CODE_COMMAND_SELECT_ALL,
		XUI_CODE_COMMAND_OPEN_FIND,
		XUI_CODE_COMMAND_OPEN_REPLACE,
		XUI_CODE_COMMAND_GOTO_LINE,
		XUI_CODE_COMMAND_TOGGLE_LINE_COMMENT,
		XUI_CODE_COMMAND_FOLD_TOGGLE
	};
	const char* sCursor;
	char sLine[512];
	char* arrFields[2];
	int iFieldCount;
	int iCommand;
	int i;

	if ( (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < (int)UI_DESIGN_COUNT_OF(arrCommands); ++i ) {
		(void)xuiCodeEditSetMenuTitle(pWidget, arrCommands[i], "");
	}
	sCursor = __uiDesignText(pNode, "data.menuTitles", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount < 2) || (arrFields[0][0] == 0) ) continue;
		iCommand = __uiDesignCodeEditMenuCommandFromText(arrFields[0], 0);
		if ( iCommand <= 0 ) continue;
		(void)xuiCodeEditSetMenuTitle(pWidget, iCommand, arrFields[1]);
	}
	return XUI_OK;
}

static int __uiDesignTerminalMenuCommandFromText(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "copy") ) return XUI_TERMINAL_MENU_COPY;
	if ( __uiDesignTokenIs(sText, "paste") ) return XUI_TERMINAL_MENU_PASTE;
	if ( __uiDesignTokenIs(sText, "selectAll") || __uiDesignTokenIs(sText, "select_all") ||
	     __uiDesignTokenIs(sText, "select-all") || __uiDesignTokenIs(sText, "all") ) {
		return XUI_TERMINAL_MENU_SELECT_ALL;
	}
	if ( __uiDesignTokenIs(sText, "clearScreen") || __uiDesignTokenIs(sText, "clear_screen") ||
	     __uiDesignTokenIs(sText, "screen") ) {
		return XUI_TERMINAL_MENU_CLEAR_SCREEN;
	}
	if ( __uiDesignTokenIs(sText, "clearScrollback") || __uiDesignTokenIs(sText, "clear_scrollback") ||
	     __uiDesignTokenIs(sText, "scrollback") ) {
		return XUI_TERMINAL_MENU_CLEAR_SCROLLBACK;
	}
	if ( __uiDesignTokenIs(sText, "find") ) return XUI_TERMINAL_MENU_FIND;
	return __uiDesignParseIntText(sText, iDefault);
}

static int __uiDesignApplyTerminalMenuTitles(ui_design_node_t* pNode, xui_widget pWidget)
{
	static const int arrCommands[] = {
		XUI_TERMINAL_MENU_COPY,
		XUI_TERMINAL_MENU_PASTE,
		XUI_TERMINAL_MENU_SELECT_ALL,
		XUI_TERMINAL_MENU_CLEAR_SCREEN,
		XUI_TERMINAL_MENU_CLEAR_SCROLLBACK,
		XUI_TERMINAL_MENU_FIND
	};
	const char* sCursor;
	char sLine[512];
	char* arrFields[2];
	int iFieldCount;
	int iCommand;
	int i;

	if ( (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < (int)UI_DESIGN_COUNT_OF(arrCommands); ++i ) {
		(void)xuiTerminalSetMenuTitle(pWidget, arrCommands[i], "");
	}
	sCursor = __uiDesignText(pNode, "data.menuTitles", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount < 2) || (arrFields[0][0] == 0) ) continue;
		iCommand = __uiDesignTerminalMenuCommandFromText(arrFields[0], 0);
		if ( iCommand <= 0 ) continue;
		(void)xuiTerminalSetMenuTitle(pWidget, iCommand, arrFields[1]);
	}
	return XUI_OK;
}

static int __uiDesignDockPanelMenuCommandFromText(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "float") ) return XUI_DOCK_PANEL_MENU_FLOAT;
	if ( __uiDesignTokenIs(sText, "autoHide") || __uiDesignTokenIs(sText, "auto_hide") ||
	     __uiDesignTokenIs(sText, "auto-hide") || __uiDesignTokenIs(sText, "pin") ) {
		return XUI_DOCK_PANEL_MENU_AUTO_HIDE;
	}
	if ( __uiDesignTokenIs(sText, "close") ) return XUI_DOCK_PANEL_MENU_CLOSE;
	if ( __uiDesignTokenIs(sText, "closeOthers") || __uiDesignTokenIs(sText, "close_others") ||
	     __uiDesignTokenIs(sText, "close-others") || __uiDesignTokenIs(sText, "others") ) {
		return XUI_DOCK_PANEL_MENU_CLOSE_OTHERS;
	}
	if ( __uiDesignTokenIs(sText, "closeAll") || __uiDesignTokenIs(sText, "close_all") ||
	     __uiDesignTokenIs(sText, "close-all") || __uiDesignTokenIs(sText, "all") ) {
		return XUI_DOCK_PANEL_MENU_CLOSE_ALL;
	}
	return __uiDesignParseIntText(sText, iDefault);
}

static int __uiDesignApplyDockPanelMenuTitles(ui_design_node_t* pNode, xui_widget pWidget)
{
	static const int arrCommands[] = {
		XUI_DOCK_PANEL_MENU_FLOAT,
		XUI_DOCK_PANEL_MENU_AUTO_HIDE,
		XUI_DOCK_PANEL_MENU_CLOSE,
		XUI_DOCK_PANEL_MENU_CLOSE_OTHERS,
		XUI_DOCK_PANEL_MENU_CLOSE_ALL
	};
	const char* sCursor;
	char sLine[512];
	char* arrFields[2];
	int iFieldCount;
	int iCommand;
	int i;

	if ( (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < (int)UI_DESIGN_COUNT_OF(arrCommands); ++i ) {
		(void)xuiDockPanelSetMenuTitle(pWidget, arrCommands[i], "");
	}
	sCursor = __uiDesignText(pNode, "data.menuTitles", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount < 2) || (arrFields[0][0] == 0) ) continue;
		iCommand = __uiDesignDockPanelMenuCommandFromText(arrFields[0], 0);
		if ( iCommand <= 0 ) continue;
		(void)xuiDockPanelSetMenuTitle(pWidget, iCommand, arrFields[1]);
	}
	return XUI_OK;
}

static int __uiDesignDockPanelTooltipFromText(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "close") ) return XUI_DOCK_PANEL_TOOLTIP_CLOSE;
	if ( __uiDesignTokenIs(sText, "autoHide") || __uiDesignTokenIs(sText, "auto_hide") ||
	     __uiDesignTokenIs(sText, "auto-hide") || __uiDesignTokenIs(sText, "pin") ) {
		return XUI_DOCK_PANEL_TOOLTIP_AUTO_HIDE;
	}
	if ( __uiDesignTokenIs(sText, "options") || __uiDesignTokenIs(sText, "option") ||
	     __uiDesignTokenIs(sText, "menu") ) {
		return XUI_DOCK_PANEL_TOOLTIP_OPTIONS;
	}
	if ( __uiDesignTokenIs(sText, "moreTabs") || __uiDesignTokenIs(sText, "more_tabs") ||
	     __uiDesignTokenIs(sText, "more-tabs") || __uiDesignTokenIs(sText, "overflow") ) {
		return XUI_DOCK_PANEL_TOOLTIP_MORE_TABS;
	}
	if ( __uiDesignTokenIs(sText, "dock") || __uiDesignTokenIs(sText, "restoreDock") ||
	     __uiDesignTokenIs(sText, "restore_dock") ) {
		return XUI_DOCK_PANEL_TOOLTIP_DOCK;
	}
	return __uiDesignParseIntText(sText, iDefault);
}

static int __uiDesignApplyDockPanelTooltips(ui_design_node_t* pNode, xui_widget pWidget)
{
	static const int arrTooltips[] = {
		XUI_DOCK_PANEL_TOOLTIP_CLOSE,
		XUI_DOCK_PANEL_TOOLTIP_AUTO_HIDE,
		XUI_DOCK_PANEL_TOOLTIP_OPTIONS,
		XUI_DOCK_PANEL_TOOLTIP_MORE_TABS,
		XUI_DOCK_PANEL_TOOLTIP_DOCK
	};
	const char* sCursor;
	char sLine[512];
	char* arrFields[2];
	int iFieldCount;
	int iTooltip;
	int i;

	if ( (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < (int)UI_DESIGN_COUNT_OF(arrTooltips); ++i ) {
		(void)xuiDockPanelSetTooltipText(pWidget, arrTooltips[i], "");
	}
	sCursor = __uiDesignText(pNode, "data.tooltips", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount < 2) || (arrFields[0][0] == 0) ) continue;
		iTooltip = __uiDesignDockPanelTooltipFromText(arrFields[0], 0);
		if ( iTooltip <= 0 ) continue;
		(void)xuiDockPanelSetTooltipText(pWidget, iTooltip, arrFields[1]);
	}
	return XUI_OK;
}

static void __uiDesignTextPoolReset(ui_design_node_t* pNode)
{
	if ( pNode == NULL ) return;
	pNode->iRuntimeTextCount = 0;
	pNode->iRuntimePropertyPaletteCount = 0;
	memset(pNode->arrRuntimePropertyPalettes, 0, sizeof(pNode->arrRuntimePropertyPalettes));
}

static int __uiDesignTextPoolAddIndex(ui_design_node_t* pNode, const char* sText)
{
	int iIndex;

	if ( pNode == NULL ) return 0;
	if ( pNode->iRuntimeTextCount >= UI_DESIGN_RUNTIME_TEXT_COUNT ) return 0;
	iIndex = pNode->iRuntimeTextCount++;
	__uiDesignCopyText(pNode->arrRuntimeText[iIndex], UI_DESIGN_RUNTIME_TEXT_CAPACITY, sText);
	return iIndex + 1;
}

static const char* __uiDesignTextPoolAt(const ui_design_node_t* pNode, int iIndex)
{
	if ( (pNode == NULL) || (iIndex <= 0) || (iIndex > pNode->iRuntimeTextCount) ) return "";
	return pNode->arrRuntimeText[iIndex - 1];
}

static const char* __uiDesignTextPoolAdd(ui_design_node_t* pNode, const char* sText)
{
	int iIndex;

	iIndex = __uiDesignTextPoolAddIndex(pNode, sText);
	return __uiDesignTextPoolAt(pNode, iIndex);
}

static void __uiDesignRuntimeTableReset(ui_design_node_t* pNode)
{
	if ( pNode == NULL ) return;
	pNode->iRuntimeTableRowCount = 0;
	pNode->iRuntimeTableColumnCount = 0;
	memset(pNode->arrRuntimeTableColumnType, 0, sizeof(pNode->arrRuntimeTableColumnType));
	memset(pNode->arrRuntimeTableText, 0, sizeof(pNode->arrRuntimeTableText));
	memset(pNode->arrRuntimeTableCells, 0, sizeof(pNode->arrRuntimeTableCells));
	memset(pNode->arrRuntimeTableCellTypeSet, 0, sizeof(pNode->arrRuntimeTableCellTypeSet));
	memset(pNode->arrRuntimeEditorOptionText, 0, sizeof(pNode->arrRuntimeEditorOptionText));
	memset(pNode->arrRuntimeEditorOptions, 0, sizeof(pNode->arrRuntimeEditorOptions));
	memset(pNode->arrRuntimeEditorItemData, 0, sizeof(pNode->arrRuntimeEditorItemData));
	memset(pNode->arrRuntimeEditorEnabled, 0, sizeof(pNode->arrRuntimeEditorEnabled));
	memset(pNode->arrRuntimeEditorPalette, 0, sizeof(pNode->arrRuntimeEditorPalette));
	pNode->iRuntimePropertyPaletteCount = 0;
	memset(pNode->arrRuntimePropertyPalettes, 0, sizeof(pNode->arrRuntimePropertyPalettes));
}

static int __uiDesignParseTextLines(ui_design_node_t* pNode, const char* sPropertyId, const char* const* ppDefaultItems, int iDefaultCount, const char** ppItems, int iCapacity)
{
	const char* sCursor;
	char sLine[256];
	int iCount;
	int i;

	if ( (pNode == NULL) || (ppItems == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	sCursor = __uiDesignText(pNode, sPropertyId, "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		ppItems[iCount++] = __uiDesignTextPoolAdd(pNode, sLine);
	}
	if ( iCount == 0 && ppDefaultItems != NULL ) {
		for ( i = 0; i < iDefaultCount && iCount < iCapacity; ++i ) {
			ppItems[iCount++] = __uiDesignTextPoolAdd(pNode, ppDefaultItems[i]);
		}
	}
	return iCount;
}

static int __uiDesignItemEnabledText(const char* sText, int bDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return bDefault;
	if ( __uiDesignTokenIs(sText, "enabled") ) return 1;
	if ( __uiDesignTokenIs(sText, "disabled") ) return 0;
	return __uiDesignParseBoolText(sText, bDefault);
}

static int __uiDesignItemSelectedText(const char* sText, int bDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return bDefault;
	if ( __uiDesignTokenIs(sText, "selected") || __uiDesignTokenIs(sText, "current") || __uiDesignTokenIs(sText, "active") ) return 1;
	if ( __uiDesignTokenIs(sText, "unselected") || __uiDesignTokenIs(sText, "normal") ) return 0;
	return __uiDesignParseBoolText(sText, bDefault);
}

static int __uiDesignBuildComboBoxItems(ui_design_node_t* pNode, xui_combobox_item_t* pItems, int iCapacity, int* pSelectedIndex, int* pSelectedValue)
{
	const char* sCursor;
	char sLine[512];
	char* arrFields[6];
	int iCount;
	int iFieldCount;
	int i;
	int bSelected;
	int bSeparator;

	if ( pSelectedIndex != NULL ) *pSelectedIndex = -1;
	if ( pSelectedValue != NULL ) *pSelectedValue = 0;
	if ( (pNode == NULL) || (pItems == NULL) || (iCapacity <= 0) ) return 0;
	memset(pItems, 0, (size_t)iCapacity * sizeof(*pItems));
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.items", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( arrFields[0][0] == 0 ) continue;
		memset(&pItems[iCount], 0, sizeof(pItems[iCount]));
		bSeparator = __uiDesignTokenIs(arrFields[0], "-") || __uiDesignTokenIs(arrFields[0], "separator");
		if ( iFieldCount > 5 && __uiDesignParseBoolText(arrFields[5], 0) ) bSeparator = 1;
		if ( bSeparator ) {
			pItems[iCount].sText = __uiDesignTextPoolAdd(pNode, "-");
			pItems[iCount].bSeparator = 1;
			pItems[iCount].bEnabled = 0;
		} else {
			pItems[iCount].sText = __uiDesignTextPoolAdd(pNode, arrFields[0]);
			pItems[iCount].bEnabled = (iFieldCount > 1) ? __uiDesignItemEnabledText(arrFields[1], 1) : 1;
		}
		pItems[iCount].iValue = (iFieldCount > 3) ? __uiDesignParseIntText(arrFields[3], iCount) : iCount;
		pItems[iCount].iIcon = (iFieldCount > 4) ? __uiDesignParseIntText(arrFields[4], 0) : 0;
		bSelected = (iFieldCount > 2) ? __uiDesignItemSelectedText(arrFields[2], 0) : 0;
		if ( bSelected && !pItems[iCount].bSeparator && pItems[iCount].bEnabled && pSelectedIndex != NULL && *pSelectedIndex < 0 ) {
			*pSelectedIndex = iCount;
			if ( pSelectedValue != NULL ) *pSelectedValue = pItems[iCount].iValue;
		}
		++iCount;
	}
	if ( iCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultItems) && iCount < iCapacity; ++i ) {
			memset(&pItems[iCount], 0, sizeof(pItems[iCount]));
			pItems[iCount].sText = __uiDesignTextPoolAdd(pNode, g_arrDefaultItems[i]);
			pItems[iCount].iValue = iCount;
			pItems[iCount].bEnabled = 1;
			++iCount;
		}
	}
	return iCount;
}

static int __uiDesignBuildListViewItems(ui_design_node_t* pNode, const char** ppItems, int* pEnabled, int* pSelected, int iCapacity, int* pExplicitSelection)
{
	const char* sCursor;
	char sLine[512];
	char* arrFields[3];
	int iCount;
	int iFieldCount;
	int i;

	if ( pExplicitSelection != NULL ) *pExplicitSelection = 0;
	if ( (pNode == NULL) || (ppItems == NULL) || (pEnabled == NULL) || (pSelected == NULL) || (iCapacity <= 0) ) return 0;
	memset(ppItems, 0, (size_t)iCapacity * sizeof(*ppItems));
	memset(pEnabled, 0, (size_t)iCapacity * sizeof(*pEnabled));
	memset(pSelected, 0, (size_t)iCapacity * sizeof(*pSelected));
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.items", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( arrFields[0][0] == 0 ) continue;
		ppItems[iCount] = __uiDesignTextPoolAdd(pNode, arrFields[0]);
		pEnabled[iCount] = (iFieldCount > 1) ? __uiDesignItemEnabledText(arrFields[1], 1) : 1;
		if ( iFieldCount > 2 ) {
			if ( pExplicitSelection != NULL ) *pExplicitSelection = 1;
			pSelected[iCount] = pEnabled[iCount] ? __uiDesignItemSelectedText(arrFields[2], 0) : 0;
		}
		++iCount;
	}
	if ( iCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultItems) && iCount < iCapacity; ++i ) {
			ppItems[iCount] = __uiDesignTextPoolAdd(pNode, g_arrDefaultItems[i]);
			pEnabled[iCount] = 1;
			pSelected[iCount] = 0;
			++iCount;
		}
	}
	return iCount;
}

static int __uiDesignHexNibble(int c)
{
	if ( c >= '0' && c <= '9' ) return c - '0';
	if ( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
	if ( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
	return -1;
}

static uint32_t __uiDesignParseColorText(const char* sText, uint32_t iDefault)
{
	uint32_t iValue;
	int iLen;
	int i;
	int v;

	if ( sText == NULL ) return iDefault;
	while ( *sText == ' ' || *sText == '\t' ) ++sText;
	if ( *sText == '#' ) ++sText;
	iLen = 0;
	while ( sText[iLen] != 0 && sText[iLen] != ' ' && sText[iLen] != '\t' ) ++iLen;
	if ( iLen != 6 && iLen != 8 ) return iDefault;
	iValue = 0u;
	for ( i = 0; i < iLen; ++i ) {
		v = __uiDesignHexNibble((unsigned char)sText[i]);
		if ( v < 0 ) return iDefault;
		iValue = (iValue << 4) | (uint32_t)v;
	}
	if ( iLen == 6 ) return XUI_COLOR_RGBA((iValue >> 16) & 0xffu, (iValue >> 8) & 0xffu, iValue & 0xffu, 255u);
	return XUI_COLOR_RGBA((iValue >> 24) & 0xffu, (iValue >> 16) & 0xffu, (iValue >> 8) & 0xffu, iValue & 0xffu);
}

static int __uiDesignBuildColorPalette(const ui_design_node_t* pNode, uint32_t* pColors, int iCapacity)
{
	const char* sCursor;
	char sLine[128];
	int iCount;
	int i;

	if ( (pColors == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.palette", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		pColors[iCount++] = __uiDesignParseColorText(sLine, XUI_COLOR_RGBA(49, 126, 214, 255));
	}
	if ( iCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultPalette) && iCount < iCapacity; ++i ) {
			pColors[iCount++] = g_arrDefaultPalette[i];
		}
	}
	return iCount;
}

static uint32_t __uiDesignColorPickerValue(const ui_design_node_t* pNode)
{
	if ( __uiDesignBool(pNode, "value.useHex", 0) ) {
		return __uiDesignParseColorText(__uiDesignText(pNode, "value.hex", "#317ED6FF"), XUI_COLOR_RGBA(49, 126, 214, 255));
	}
	return __uiDesignColor(pNode, "value.color", XUI_COLOR_RGBA(49, 126, 214, 255));
}

static void __uiDesignApplyTerminalPaletteText(xui_widget pWidget, const char* sText)
{
	const char* sCursor;
	char sLine[128];
	char* arrFields[3];
	int iFieldCount;
	int iIndex;
	int iSequential;
	uint32_t iColor;

	if ( pWidget == NULL || sText == NULL ) return;
	sCursor = sText;
	iSequential = 0;
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount <= 0 || arrFields[0][0] == 0 ) continue;
		if ( iFieldCount >= 2 ) {
			iIndex = __uiDesignParseIntText(arrFields[0], -1);
			iColor = __uiDesignParseColorText(arrFields[1], XUI_COLOR_RGBA(0, 0, 0, 0));
		} else {
			iIndex = iSequential;
			iColor = __uiDesignParseColorText(arrFields[0], XUI_COLOR_RGBA(0, 0, 0, 0));
		}
		if ( iIndex >= 0 && iIndex < 256 ) {
			(void)xuiTerminalSetPalette(pWidget, iIndex, iColor);
		}
		iSequential = (iIndex >= iSequential) ? (iIndex + 1) : (iSequential + 1);
	}
}

static void __uiDesignApplyTerminalPalette(ui_design_node_t* pNode, xui_widget pWidget)
{
	const char* sPalette;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	__uiDesignApplyTerminalPaletteText(pWidget, UI_DESIGN_TERMINAL_DEFAULT_PALETTE);
	sPalette = __uiDesignText(pNode, "data.palette", "");
	if ( (sPalette != NULL) && (sPalette[0] != 0) ) {
		__uiDesignApplyTerminalPaletteText(pWidget, sPalette);
	}
}

static int __uiDesignClampIntRange(int iValue, int iMin, int iMax);

static void __uiDesignApplyTerminalSelection(ui_design_node_t* pNode, xui_widget pWidget)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[4];
	int iFieldCount;
	int iAnchorLine;
	int iAnchorColumn;
	int iEndLine;
	int iEndColumn;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	sCursor = __uiDesignText(pNode, "data.selection", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 4 ) break;
		iAnchorLine = __uiDesignParseIntText(arrFields[0], 1) - 1;
		iAnchorColumn = __uiDesignParseIntText(arrFields[1], 1) - 1;
		iEndLine = __uiDesignParseIntText(arrFields[2], iAnchorLine + 1) - 1;
		iEndColumn = __uiDesignParseIntText(arrFields[3], iAnchorColumn + 1) - 1;
		(void)xuiTerminalSetSelectionRange(pWidget,
			__uiDesignClampIntRange(iAnchorLine, 0, 1000000),
			__uiDesignClampIntRange(iAnchorColumn, 0, 1000000),
			__uiDesignClampIntRange(iEndLine, 0, 1000000),
			__uiDesignClampIntRange(iEndColumn, 0, 1000000));
		return;
	}
	(void)xuiTerminalClearSelection(pWidget);
}

static void __uiDesignApplyTerminalFind(ui_design_node_t* pNode, xui_widget pWidget)
{
	const char* sFindText;
	uint32_t iFlags;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	sFindText = __uiDesignText(pNode, "data.findText", "");
	if ( sFindText == NULL || sFindText[0] == 0 ) {
		(void)xuiTerminalClearFind(pWidget);
		return;
	}
	iFlags = __uiDesignBool(pNode, "behavior.findCaseSensitive", 0) ? XUI_TERMINAL_SEARCH_CASE_SENSITIVE : 0u;
	if ( __uiDesignBool(pNode, "behavior.findBackward", 0) ) {
		(void)xuiTerminalFindPrev(pWidget, sFindText, iFlags, NULL, NULL);
	} else {
		(void)xuiTerminalFindText(pWidget, sFindText, iFlags, NULL, NULL);
	}
}

static void __uiDesignApplyTerminalScroll(ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_scroll_model_t* pScroll;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	if ( !__uiDesignBool(pNode, "behavior.useScroll", 0) ) return;
	pScroll = xuiTerminalGetScrollModel(pWidget);
	if ( pScroll == NULL ) return;
	(void)xuiScrollModelSetOffset(pScroll,
		__uiDesignFloat(pNode, "value.scrollX", 0.0f),
		__uiDesignFloat(pNode, "value.scrollY", 0.0f));
}

static int __uiDesignDatePickerModeIsRange(int iMode)
{
	return (iMode == XUI_DATE_PICKER_MODE_DATE_RANGE) ||
	       (iMode == XUI_DATE_PICKER_MODE_TIME_RANGE) ||
	       (iMode == XUI_DATE_PICKER_MODE_DATETIME_RANGE);
}

static int __uiDesignParseDatePickerTime(const char* sText, xtime* pValue)
{
	int iHour;
	int iMinute;
	int iSecond;
	int iRead;

	if ( (sText == NULL) || (pValue == NULL) ) return 0;
	iHour = 0;
	iMinute = 0;
	iSecond = 0;
	iRead = sscanf(sText, "%d:%d:%d", &iHour, &iMinute, &iSecond);
	if ( iRead < 2 ) return 0;
	if ( iHour < 0 ) iHour = 0;
	if ( iHour > 23 ) iHour = 23;
	if ( iMinute < 0 ) iMinute = 0;
	if ( iMinute > 59 ) iMinute = 59;
	if ( iSecond < 0 ) iSecond = 0;
	if ( iSecond > 59 ) iSecond = 59;
	*pValue = (xtime)iHour * XRT_TIME_HOUR + (xtime)iMinute * XRT_TIME_MINUTE + (xtime)iSecond;
	return 1;
}

static int __uiDesignParseDatePickerValue(const char* sText, int iMode, xtime* pValue)
{
	xtime tParsed;
	int iYear;
	int iMonth;
	int iDay;
	int iHour;
	int iMinute;
	int iSecond;
	int iRead;

	if ( (sText == NULL) || (pValue == NULL) ) return 0;
	while ( sText[0] == ' ' || sText[0] == '\t' ) ++sText;
	if ( sText[0] == '\0' ) return 0;
	if ( (iMode == XUI_DATE_PICKER_MODE_TIME) || (iMode == XUI_DATE_PICKER_MODE_TIME_RANGE) ) {
		if ( __uiDesignParseDatePickerTime(sText, pValue) ) return 1;
		tParsed = xrtTimeParse((str)sText, (str)"hh:nn:ss");
		if ( tParsed > 0 ) {
			*pValue = xrtTimePart(tParsed);
			return 1;
		}
		return 0;
	}
	iYear = 0;
	iMonth = 0;
	iDay = 0;
	iHour = 0;
	iMinute = 0;
	iSecond = 0;
	iRead = sscanf(sText, "%d-%d-%d %d:%d:%d", &iYear, &iMonth, &iDay, &iHour, &iMinute, &iSecond);
	if ( iRead >= 3 ) {
		*pValue = xrtDateSerial(iYear, iMonth, iDay);
		if ( iRead >= 5 ) *pValue += (xtime)iHour * XRT_TIME_HOUR + (xtime)iMinute * XRT_TIME_MINUTE + (xtime)iSecond;
		return 1;
	}
	iRead = sscanf(sText, "%d/%d/%d %d:%d:%d", &iYear, &iMonth, &iDay, &iHour, &iMinute, &iSecond);
	if ( iRead >= 3 ) {
		*pValue = xrtDateSerial(iYear, iMonth, iDay);
		if ( iRead >= 5 ) *pValue += (xtime)iHour * XRT_TIME_HOUR + (xtime)iMinute * XRT_TIME_MINUTE + (xtime)iSecond;
		return 1;
	}
	tParsed = xrtTimeParse((str)sText, (str)"yyyy-mm-dd hh:nn:ss");
	if ( tParsed > 0 ) {
		*pValue = tParsed;
		return 1;
	}
	tParsed = xrtTimeParse((str)sText, (str)"yyyy-mm-dd");
	if ( tParsed > 0 ) {
		*pValue = tParsed;
		return 1;
	}
	if ( ((iMode == XUI_DATE_PICKER_MODE_DATETIME) || (iMode == XUI_DATE_PICKER_MODE_DATETIME_RANGE)) &&
	     __uiDesignParseDatePickerTime(sText, pValue) ) {
		*pValue += xrtDatePart(xrtNow());
		return 1;
	}
	return 0;
}

static xtime __uiDesignParseDatePickerSpan(const char* sText, xtime tDefault)
{
	const char* sCursor;
	char* sEnd;
	double fValue;
	double fSeconds;

	if ( sText == NULL ) return tDefault;
	while ( sText[0] == ' ' || sText[0] == '\t' ) ++sText;
	if ( sText[0] == '\0' ) return tDefault;
	if ( __uiDesignParseDatePickerTime(sText, &tDefault) ) return tDefault;
	sCursor = sText;
	fSeconds = 0.0;
	while ( sCursor[0] != '\0' ) {
		while ( sCursor[0] == ' ' || sCursor[0] == '\t' || sCursor[0] == ',') ++sCursor;
		if ( sCursor[0] == '\0' ) break;
		fValue = strtod(sCursor, &sEnd);
		if ( sEnd == sCursor ) return tDefault;
		sCursor = sEnd;
		while ( sCursor[0] == ' ' || sCursor[0] == '\t' ) ++sCursor;
		switch ( sCursor[0] ) {
		case 'd':
		case 'D':
			fSeconds += fValue * (double)XRT_TIME_DAY;
			++sCursor;
			break;
		case 'h':
		case 'H':
			fSeconds += fValue * (double)XRT_TIME_HOUR;
			++sCursor;
			break;
		case 'm':
		case 'M':
			fSeconds += fValue * (double)XRT_TIME_MINUTE;
			++sCursor;
			break;
		case 's':
		case 'S':
			fSeconds += fValue;
			++sCursor;
			break;
		default:
			fSeconds += fValue;
			break;
		}
	}
	if ( fSeconds < 0.0 ) fSeconds = 0.0;
	return (xtime)(fSeconds + 0.5);
}

static void __uiDesignFillDatePickerValue(ui_design_node_t* pNode, xui_date_picker_desc_t* pDesc)
{
	xtime tValue;
	xtime tStart;
	xtime tEnd;
	int iMode;

	if ( (pNode == NULL) || (pDesc == NULL) ) return;
	iMode = pDesc->iMode;
	pDesc->tDefaultRangeSpan = __uiDesignParseDatePickerSpan(__uiDesignText(pNode, "value.defaultRangeSpan", "1d"), 0);
	if ( __uiDesignBool(pNode, "limits.hasMin", 0) &&
	     __uiDesignParseDatePickerValue(__uiDesignText(pNode, "limits.min", "2020-01-01"), iMode, &tValue) ) {
		pDesc->bHasMin = 1;
		pDesc->tMin = tValue;
	}
	if ( __uiDesignBool(pNode, "limits.hasMax", 0) &&
	     __uiDesignParseDatePickerValue(__uiDesignText(pNode, "limits.max", "2030-12-31"), iMode, &tValue) ) {
		pDesc->bHasMax = 1;
		pDesc->tMax = tValue;
	}
	if ( !__uiDesignBool(pNode, "value.hasValue", 1) ) return;
	if ( __uiDesignDatePickerModeIsRange(iMode) ) {
		if ( __uiDesignParseDatePickerValue(__uiDesignText(pNode, "value.rangeStart", "2026-01-01"), iMode, &tStart) &&
		     __uiDesignParseDatePickerValue(__uiDesignText(pNode, "value.rangeEnd", "2026-01-02"), iMode, &tEnd) ) {
			pDesc->bHasRangeValue = 1;
			pDesc->tStart = tStart;
			pDesc->tEnd = tEnd;
		}
	} else if ( __uiDesignParseDatePickerValue(__uiDesignText(pNode, "value.value", "2026-01-01"), iMode, &tValue) ) {
		pDesc->bHasValue = 1;
		pDesc->tValue = tValue;
	}
}

static void __uiDesignApplyDatePickerValue(ui_design_node_t* pNode, xui_widget pWidget)
{
	xtime tValue;
	xtime tStart;
	xtime tEnd;
	xtime tMin;
	xtime tMax;
	int iMode;
	int bHasMin;
	int bHasMax;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	iMode = __uiDesignInt(pNode, "behavior.mode", XUI_DATE_PICKER_MODE_DATE);
	bHasMin = __uiDesignBool(pNode, "limits.hasMin", 0) &&
	          __uiDesignParseDatePickerValue(__uiDesignText(pNode, "limits.min", "2020-01-01"), iMode, &tMin);
	bHasMax = __uiDesignBool(pNode, "limits.hasMax", 0) &&
	          __uiDesignParseDatePickerValue(__uiDesignText(pNode, "limits.max", "2030-12-31"), iMode, &tMax);
	(void)xuiDatePickerSetLimitRange(pWidget, bHasMin, bHasMin ? tMin : 0, bHasMax, bHasMax ? tMax : 0);
	(void)xuiDatePickerSetDefaultRangeSpan(pWidget, __uiDesignParseDatePickerSpan(__uiDesignText(pNode, "value.defaultRangeSpan", "1d"), 0));
	if ( !__uiDesignBool(pNode, "value.hasValue", 1) ) {
		(void)xuiDatePickerClearValue(pWidget);
		return;
	}
	if ( __uiDesignDatePickerModeIsRange(iMode) ) {
		if ( __uiDesignParseDatePickerValue(__uiDesignText(pNode, "value.rangeStart", "2026-01-01"), iMode, &tStart) &&
		     __uiDesignParseDatePickerValue(__uiDesignText(pNode, "value.rangeEnd", "2026-01-02"), iMode, &tEnd) ) {
			(void)xuiDatePickerSetRangeValue(pWidget, tStart, tEnd);
		}
	} else if ( __uiDesignParseDatePickerValue(__uiDesignText(pNode, "value.value", "2026-01-01"), iMode, &tValue) ) {
		(void)xuiDatePickerSetValue(pWidget, tValue);
	}
}

static int __uiDesignSplitDataFields(char* sLine, char** arrFields, int iCapacity);
static int __uiDesignLooksIntText(const char* sText);
static int __uiDesignLooksNumberText(const char* sText);

static int __uiDesignChartSeriesTypeFromText(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "line") ) return XUI_CHART_SERIES_LINE;
	if ( __uiDesignTokenIs(sText, "bar") ) return XUI_CHART_SERIES_BAR;
	if ( __uiDesignTokenIs(sText, "pie") ) return XUI_CHART_SERIES_PIE;
	if ( __uiDesignTokenIs(sText, "scatter") || __uiDesignTokenIs(sText, "point") ) return XUI_CHART_SERIES_SCATTER;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static int __uiDesignChartSymbolFromText(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "none") || __uiDesignTokenIs(sText, "hidden") ) return 0;
	if ( __uiDesignTokenIs(sText, "circle") ) return XUI_CHART_SYMBOL_CIRCLE;
	if ( __uiDesignTokenIs(sText, "rect") || __uiDesignTokenIs(sText, "rectangle") || __uiDesignTokenIs(sText, "square") ) return XUI_CHART_SYMBOL_RECT;
	if ( __uiDesignTokenIs(sText, "triangle") ) return XUI_CHART_SYMBOL_TRIANGLE;
	if ( __uiDesignTokenIs(sText, "diamond") ) return XUI_CHART_SYMBOL_DIAMOND;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static int __uiDesignBuildChartSeriesDefs(ui_design_node_t* pNode, ui_design_chart_series_def_t* pSeries, int iCapacity)
{
	const char* sCursor;
	char sLine[512];
	char sDefaultId[32];
	char* arrFields[16];
	uint32_t iDefaultColor;
	int iCount;
	int iFieldCount;

	if ( (pNode == NULL) || (pSeries == NULL) || (iCapacity <= 0) ) return 0;
	memset(pSeries, 0, (size_t)iCapacity * sizeof(*pSeries));
	iDefaultColor = __uiDesignColor(pNode, "appearance.seriesColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.seriesList", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		snprintf(sDefaultId, sizeof(sDefaultId), "s%d", iCount + 1);
		memset(&pSeries[iCount], 0, sizeof(pSeries[iCount]));
		pSeries[iCount].sId = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 0, sDefaultId));
		pSeries[iCount].iType = __uiDesignChartSeriesTypeFromText(__uiDesignField(arrFields, iFieldCount, 1, ""), __uiDesignInt(pNode, "data.seriesType", XUI_CHART_SERIES_LINE));
		pSeries[iCount].sName = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 2, pSeries[iCount].sId));
		pSeries[iCount].bVisible = __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 3, ""), 1);
		pSeries[iCount].iColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 4, ""), iDefaultColor);
		pSeries[iCount].bAreaFill = __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 5, ""), 0);
		pSeries[iCount].iAreaColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 6, ""), pSeries[iCount].iColor);
		pSeries[iCount].bSmooth = __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 7, ""), 0);
		pSeries[iCount].sDash = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 8, ""));
		pSeries[iCount].iSymbol = __uiDesignChartSymbolFromText(__uiDesignField(arrFields, iFieldCount, 9, ""), XUI_CHART_SYMBOL_CIRCLE);
		pSeries[iCount].fSymbolSize = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 10, ""), 5.0f);
		pSeries[iCount].fRadiusMin = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 11, ""), 0.0f);
		pSeries[iCount].fRadiusMax = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 12, ""), 0.0f);
		pSeries[iCount].bValueColor = (__uiDesignField(arrFields, iFieldCount, 13, "")[0] != 0 || __uiDesignField(arrFields, iFieldCount, 14, "")[0] != 0);
		pSeries[iCount].iValueMinColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 13, ""), pSeries[iCount].iColor);
		pSeries[iCount].iValueMaxColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 14, ""), pSeries[iCount].iColor);
		++iCount;
	}
	if ( iCount == 0 ) {
		memset(&pSeries[0], 0, sizeof(pSeries[0]));
		pSeries[0].sId = __uiDesignTextPoolAdd(pNode, "s1");
		pSeries[0].sName = __uiDesignTextPoolAdd(pNode, "Series");
		pSeries[0].iType = __uiDesignInt(pNode, "data.seriesType", XUI_CHART_SERIES_LINE);
		pSeries[0].bVisible = 1;
		pSeries[0].iColor = iDefaultColor;
		pSeries[0].iAreaColor = iDefaultColor;
		pSeries[0].iSymbol = XUI_CHART_SYMBOL_CIRCLE;
		pSeries[0].fSymbolSize = 5.0f;
		iCount = 1;
	}
	return iCount;
}

static int __uiDesignParseChartDash(const char* sText, float* pDash, int iCapacity)
{
	char sBuffer[128];
	char* sCursor;
	char* sEnd;
	int iCount;
	int i;

	if ( (sText == NULL) || (pDash == NULL) || (iCapacity <= 0) ) return 0;
	snprintf(sBuffer, sizeof(sBuffer), "%s", sText);
	sBuffer[sizeof(sBuffer) - 1] = 0;
	for ( i = 0; sBuffer[i] != 0; ++i ) {
		if ( sBuffer[i] == ',' || sBuffer[i] == ';' || sBuffer[i] == '/' ) sBuffer[i] = ' ';
	}
	iCount = 0;
	sCursor = sBuffer;
	while ( sCursor[0] != 0 && iCount < iCapacity ) {
		while ( sCursor[0] == ' ' || sCursor[0] == '\t' ) ++sCursor;
		if ( sCursor[0] == 0 ) break;
		pDash[iCount] = (float)strtod(sCursor, &sEnd);
		if ( sEnd == sCursor ) break;
		if ( pDash[iCount] > 0.0f ) ++iCount;
		sCursor = sEnd;
	}
	return iCount;
}

static int __uiDesignBuildChartPointsForSeries(ui_design_node_t* pNode, const char* sSeriesId, int bDefaultSeries,
	uint32_t iDefaultColor, xui_chart_point_t* pPoints, int iCapacity)
{
	const char* sCursor;
	char sLine[512];
	char* arrFields[8];
	int iCount;
	int iFieldCount;
	int bSawRows;
	int bPipeRow;
	int i;

	if ( (pNode == NULL) || (pPoints == NULL) || (iCapacity <= 0) ) return 0;
	memset(pPoints, 0, (size_t)iCapacity * sizeof(*pPoints));
	iCount = 0;
	bSawRows = 0;
	sCursor = __uiDesignText(pNode, "data.series", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		bSawRows = 1;
		bPipeRow = (strchr(sLine, '|') != NULL);
		iFieldCount = bPipeRow ? __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields)) : __uiDesignSplitDataFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount <= 0 ) continue;
		if ( bPipeRow ) {
			if ( (iFieldCount < 4) || !__uiDesignTextEqualsNoCase(arrFields[0], sSeriesId) ) continue;
			memset(&pPoints[iCount], 0, sizeof(pPoints[iCount]));
			pPoints[iCount].label = (arrFields[1][0] != 0) ? __uiDesignTextPoolAdd(pNode, arrFields[1]) : NULL;
			pPoints[iCount].x = atof(__uiDesignField(arrFields, iFieldCount, 2, "0"));
			pPoints[iCount].y = atof(__uiDesignField(arrFields, iFieldCount, 3, "0"));
			pPoints[iCount].value = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 4, ""), (float)pPoints[iCount].y);
			pPoints[iCount].color = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 5, ""), iDefaultColor);
			++iCount;
			continue;
		}
		if ( !bDefaultSeries ) continue;
		memset(&pPoints[iCount], 0, sizeof(pPoints[iCount]));
		pPoints[iCount].color = iDefaultColor;
		if ( (iFieldCount >= 2) && !__uiDesignLooksNumberText(arrFields[0]) ) {
			pPoints[iCount].label = __uiDesignTextPoolAdd(pNode, arrFields[0]);
			if ( iFieldCount == 2 ) {
				pPoints[iCount].x = (double)iCount;
				pPoints[iCount].y = atof(arrFields[1]);
				pPoints[iCount].value = pPoints[iCount].y;
			} else {
				pPoints[iCount].x = atof(arrFields[1]);
				pPoints[iCount].y = atof(arrFields[2]);
				pPoints[iCount].value = (iFieldCount > 3 && arrFields[3][0] != 0) ? atof(arrFields[3]) : pPoints[iCount].y;
				if ( iFieldCount > 4 ) pPoints[iCount].color = __uiDesignParseColorText(arrFields[4], pPoints[iCount].color);
			}
		} else {
			pPoints[iCount].x = (iFieldCount > 0) ? atof(arrFields[0]) : (double)iCount;
			pPoints[iCount].y = (iFieldCount > 1) ? atof(arrFields[1]) : 0.0;
			pPoints[iCount].value = (iFieldCount > 2 && arrFields[2][0] != 0) ? atof(arrFields[2]) : pPoints[iCount].y;
			pPoints[iCount].label = (iFieldCount > 3 && arrFields[3][0] != 0) ? __uiDesignTextPoolAdd(pNode, arrFields[3]) : NULL;
			if ( iFieldCount > 4 ) pPoints[iCount].color = __uiDesignParseColorText(arrFields[4], pPoints[iCount].color);
		}
		++iCount;
	}
	if ( (iCount == 0) && !bSawRows && bDefaultSeries ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrChartPoints) && iCount < iCapacity; ++i ) {
			pPoints[iCount] = g_arrChartPoints[i];
			pPoints[iCount].label = __uiDesignTextPoolAdd(pNode, g_arrChartPoints[i].label);
			pPoints[iCount].color = iDefaultColor;
			++iCount;
		}
	}
	return iCount;
}

static void __uiDesignApplyChartSeriesStyle(xui_widget pWidget, int iSeries, const ui_design_chart_series_def_t* pDef)
{
	float arrDash[8];
	int iDashCount;

	if ( (pWidget == NULL) || (pDef == NULL) ) return;
	(void)xuiChartSetSeriesColor(pWidget, iSeries, pDef->iColor);
	(void)xuiChartSetSeriesVisible(pWidget, iSeries, pDef->bVisible);
	(void)xuiChartSetSeriesAreaFill(pWidget, iSeries, pDef->bAreaFill, pDef->iAreaColor);
	(void)xuiChartSetSeriesSmooth(pWidget, iSeries, pDef->bSmooth);
	(void)xuiChartSetSeriesSymbol(pWidget, iSeries, pDef->iSymbol);
	(void)xuiChartSetSeriesSymbolSize(pWidget, iSeries, pDef->fSymbolSize);
	iDashCount = __uiDesignParseChartDash(pDef->sDash, arrDash, UI_DESIGN_COUNT_OF(arrDash));
	if ( iDashCount > 0 ) (void)xuiChartSetSeriesDash(pWidget, iSeries, arrDash, iDashCount);
	if ( pDef->fRadiusMin > 0.0f || pDef->fRadiusMax > 0.0f ) {
		(void)xuiChartSetSeriesValueRadius(pWidget, iSeries, pDef->fRadiusMin, pDef->fRadiusMax);
	}
	if ( pDef->bValueColor ) {
		(void)xuiChartSetSeriesValueColor(pWidget, iSeries, pDef->iValueMinColor, pDef->iValueMaxColor);
	}
}

static int __uiDesignNextRawLine(const char** ppText, char* sLine, int iCapacity)
{
	const char* sText;
	int i;

	if ( (ppText == NULL) || (sLine == NULL) || (iCapacity <= 0) ) return 0;
	sText = *ppText;
	if ( (sText == NULL) || (sText[0] == 0) ) return 0;
	i = 0;
	while ( sText[0] != 0 && sText[0] != '\n' ) {
		if ( i < iCapacity - 1 ) {
			sLine[i++] = sText[0];
		}
		++sText;
	}
	if ( sText[0] == '\n' ) ++sText;
	sLine[i] = 0;
	*ppText = sText;
	return 1;
}

static int __uiDesignSplitDataFields(char* sLine, char** arrFields, int iCapacity)
{
	char* sCursor;
	char cDelimiter;
	int iCount;
	int i;

	if ( (sLine == NULL) || (arrFields == NULL) || (iCapacity <= 0) ) return 0;
	cDelimiter = (strchr(sLine, ',') != NULL) ? ',' : '|';
	iCount = 0;
	arrFields[iCount++] = sLine;
	sCursor = sLine;
	while ( sCursor[0] != 0 ) {
		if ( sCursor[0] == cDelimiter ) {
			sCursor[0] = 0;
			if ( iCount < iCapacity ) {
				arrFields[iCount++] = sCursor + 1;
			}
		}
		++sCursor;
	}
	for ( i = 0; i < iCount; ++i ) {
		__uiDesignTrim(arrFields[i]);
	}
	return iCount;
}

static int __uiDesignLooksIntText(const char* sText)
{
	int i;

	if ( (sText == NULL) || (sText[0] == 0) ) return 0;
	i = 0;
	if ( sText[i] == '-' || sText[i] == '+' ) ++i;
	if ( sText[i] == 0 ) return 0;
	for ( ; sText[i] != 0; ++i ) {
		if ( sText[i] < '0' || sText[i] > '9' ) return 0;
	}
	return 1;
}

static int __uiDesignLooksNumberText(const char* sText)
{
	int i;
	int bDigit;

	if ( (sText == NULL) || (sText[0] == 0) ) return 0;
	i = 0;
	bDigit = 0;
	if ( sText[i] == '-' || sText[i] == '+' ) ++i;
	for ( ; sText[i] != 0; ++i ) {
		if ( sText[i] >= '0' && sText[i] <= '9' ) {
			bDigit = 1;
			continue;
		}
		if ( sText[i] == '.' || sText[i] == 'e' || sText[i] == 'E' || sText[i] == '-' || sText[i] == '+' ) continue;
		return 0;
	}
	return bDigit;
}

static xui_table_track_t __uiDesignTableTrackDefault(void)
{
	xui_table_track_t tTrack;

	tTrack.iSizeMode = XUI_SIZE_CONTENT;
	tTrack.fValue = 0.0f;
	tTrack.fMin = 0.0f;
	tTrack.fMax = XUI_LAYOUT_UNBOUNDED;
	tTrack.fWeight = 1.0f;
	return tTrack;
}

static int __uiDesignLayoutSizeModeFromText(const char* sText, int iDefault)
{
	int iValue;

	if ( __uiDesignTokenIs(sText, "fixed") || __uiDesignTokenIs(sText, "pixel") || __uiDesignTokenIs(sText, "pixels") || __uiDesignTokenIs(sText, "px") ) return XUI_SIZE_FIXED;
	if ( __uiDesignTokenIs(sText, "content") || __uiDesignTokenIs(sText, "auto") ) return XUI_SIZE_CONTENT;
	if ( __uiDesignTokenIs(sText, "fill") || __uiDesignTokenIs(sText, "stretch") || __uiDesignTokenIs(sText, "star") || __uiDesignTokenIs(sText, "grow") ) return XUI_SIZE_FILL;
	if ( __uiDesignLooksIntText(sText) ) {
		iValue = __uiDesignParseIntText(sText, iDefault);
		if ( iValue == XUI_SIZE_FIXED || iValue == XUI_SIZE_CONTENT || iValue == XUI_SIZE_FILL ) return iValue;
	}
	return iDefault;
}

static xui_table_track_t __uiDesignTableTrackFromValues(int iSizeMode, float fValue,
	float fMin, int bHasMin, float fMax, int bHasMax, float fWeight, int bHasWeight)
{
	xui_table_track_t tTrack;

	tTrack = __uiDesignTableTrackDefault();
	tTrack.iSizeMode = iSizeMode;
	if ( bHasMin ) tTrack.fMin = (fMin > 0.0f) ? fMin : 0.0f;
	if ( bHasMax ) tTrack.fMax = (fMax > 0.0f) ? fMax : 0.0f;
	if ( tTrack.fMax < tTrack.fMin ) tTrack.fMax = tTrack.fMin;
	if ( bHasWeight ) tTrack.fWeight = (fWeight > 0.0f) ? fWeight : 1.0f;
	if ( tTrack.iSizeMode == XUI_SIZE_FIXED ) {
		tTrack.fValue = (fValue > 0.0f) ? fValue : 0.0f;
	} else if ( tTrack.iSizeMode == XUI_SIZE_FILL ) {
		tTrack.fWeight = bHasWeight ? tTrack.fWeight : ((fValue > 0.0f) ? fValue : 1.0f);
	}
	return tTrack;
}

static xui_table_track_t __uiDesignTableTrackFromValue(int iSizeMode, float fValue)
{
	return __uiDesignTableTrackFromValues(iSizeMode, fValue, 0.0f, 0, 0.0f, 0, 1.0f, 0);
}

static void __uiDesignApplyTableTrackConfig(ui_design_node_t* pNode, xui_widget pWidget, const char* sPropertyId, int bRows)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[6];
	xui_table_track_t tTrack;
	int iFieldCount;
	int iTrack;
	int iSizeMode;
	float fValue;

	if ( (pNode == NULL) || (pWidget == NULL) || (sPropertyId == NULL) || !__uiDesignHasProperty(pNode, sPropertyId) ) return;
	sCursor = __uiDesignText(pNode, sPropertyId, "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 || !__uiDesignLooksIntText(arrFields[0]) ) continue;
		iTrack = __uiDesignParseIntText(arrFields[0], -1);
		if ( iTrack < 0 ) continue;
		iSizeMode = __uiDesignLayoutSizeModeFromText(__uiDesignField(arrFields, iFieldCount, 1, "content"), XUI_SIZE_CONTENT);
		fValue = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 2, "0"), 0.0f);
		tTrack = __uiDesignTableTrackFromValues(iSizeMode, fValue,
			__uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 3, "0"), 0.0f),
			iFieldCount > 3 && arrFields[3][0] != 0,
			__uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 4, "0"), 0.0f),
			iFieldCount > 4 && arrFields[4][0] != 0,
			__uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 5, "1"), 1.0f),
			iFieldCount > 5 && arrFields[5][0] != 0);
		if ( bRows ) {
			(void)xuiWidgetSetTableRow(pWidget, iTrack, &tTrack);
		} else {
			(void)xuiWidgetSetTableColumn(pWidget, iTrack, &tTrack);
		}
	}
}

static int __uiDesignStyleValueTypeFromText(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "none") ) return XUI_STYLE_VALUE_NONE;
	if ( __uiDesignTokenIs(sText, "int") || __uiDesignTokenIs(sText, "integer") ) return XUI_STYLE_VALUE_INT;
	if ( __uiDesignTokenIs(sText, "float") || __uiDesignTokenIs(sText, "number") ) return XUI_STYLE_VALUE_FLOAT;
	if ( __uiDesignTokenIs(sText, "color") ) return XUI_STYLE_VALUE_COLOR;
	if ( __uiDesignTokenIs(sText, "bool") || __uiDesignTokenIs(sText, "boolean") ) return XUI_STYLE_VALUE_BOOL;
	if ( __uiDesignTokenIs(sText, "string") || __uiDesignTokenIs(sText, "text") ) return XUI_STYLE_VALUE_STRING;
	if ( __uiDesignTokenIs(sText, "token") ) return XUI_STYLE_VALUE_TOKEN;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static int __uiDesignInferStyleValueType(const char* sText)
{
	const char* sValue;
	int iLen;

	if ( sText == NULL ) return XUI_STYLE_VALUE_STRING;
	sValue = sText;
	while ( sValue[0] == ' ' || sValue[0] == '\t' ) ++sValue;
	if ( sValue[0] == '#' ) {
		iLen = 0;
		while ( sValue[1 + iLen] != 0 && sValue[1 + iLen] != ' ' && sValue[1 + iLen] != '\t' ) ++iLen;
		if ( iLen == 6 || iLen == 8 ) return XUI_STYLE_VALUE_COLOR;
	}
	if ( __uiDesignIsBoolText(sValue) ) return XUI_STYLE_VALUE_BOOL;
	if ( __uiDesignLooksIntText(sValue) ) return XUI_STYLE_VALUE_INT;
	if ( __uiDesignLooksNumberText(sValue) ) return XUI_STYLE_VALUE_FLOAT;
	return XUI_STYLE_VALUE_STRING;
}

static uint32_t __uiDesignStyleDirtyToken(const char* sToken)
{
	if ( __uiDesignTokenIs(sToken, "layout") ) return XUI_WIDGET_DIRTY_LAYOUT;
	if ( __uiDesignTokenIs(sToken, "style") ) return XUI_WIDGET_DIRTY_STYLE;
	if ( __uiDesignTokenIs(sToken, "cache") ) return XUI_WIDGET_DIRTY_CACHE;
	if ( __uiDesignTokenIs(sToken, "render") || __uiDesignTokenIs(sToken, "paint") ) return XUI_WIDGET_DIRTY_RENDER;
	if ( __uiDesignTokenIs(sToken, "tree") ) return XUI_WIDGET_DIRTY_TREE;
	if ( __uiDesignTokenIs(sToken, "all") ) return XUI_WIDGET_DIRTY_ALL;
	if ( __uiDesignTokenIs(sToken, "default") || __uiDesignTokenIs(sToken, "auto") ) return 0u;
	return 0u;
}

static uint32_t __uiDesignStyleDirtyFlagsFromText(const char* sText, uint32_t iDefault)
{
	char sWork[128];
	char* sCursor;
	char* sToken;
	char* sEnd;
	unsigned long iValue;
	uint32_t iFlags;
	int bAny;
	int i;
	int iLen;

	if ( sText == NULL ) return iDefault;
	while ( sText[0] == ' ' || sText[0] == '\t' ) ++sText;
	if ( sText[0] == '\0' ) return iDefault;
	iValue = strtoul(sText, &sEnd, 0);
	while ( sEnd != NULL && (sEnd[0] == ' ' || sEnd[0] == '\t') ) ++sEnd;
	if ( sEnd != sText && sEnd != NULL && sEnd[0] == '\0' ) {
		return (uint32_t)iValue & XUI_WIDGET_DIRTY_ALL;
	}
	__uiDesignCopyText(sWork, sizeof(sWork), sText);
	iLen = (int)strlen(sWork);
	for ( i = 0; i < iLen; ++i ) {
		if ( sWork[i] == '|' || sWork[i] == ',' || sWork[i] == ';' || sWork[i] == '+' || sWork[i] == ' ' || sWork[i] == '\t' ) {
			sWork[i] = '\0';
		}
	}
	iFlags = 0u;
	bAny = 0;
	sCursor = sWork;
	while ( sCursor < sWork + iLen ) {
		while ( sCursor < sWork + iLen && *sCursor == '\0' ) ++sCursor;
		if ( sCursor >= sWork + iLen ) break;
		sToken = sCursor;
		while ( sCursor < sWork + iLen && *sCursor != '\0' ) ++sCursor;
		iValue = __uiDesignStyleDirtyToken(sToken);
		if ( iValue != 0u || __uiDesignTokenIs(sToken, "default") || __uiDesignTokenIs(sToken, "auto") ) {
			iFlags |= (uint32_t)iValue;
			bAny = 1;
		}
	}
	return bAny ? iFlags : iDefault;
}

static int __uiDesignBuildInlineStyles(ui_design_node_t* pNode, xui_style_property_t* pProperties, int iCapacity)
{
	const char* sCursor;
	const char* sValue;
	char sLine[512];
	char* arrFields[4];
	xui_style_property_t* pProperty;
	int iFieldCount;
	int iType;
	int iCount;

	if ( (pNode == NULL) || (pProperties == NULL) || (iCapacity <= 0) ) return 0;
	memset(pProperties, 0, (size_t)iCapacity * sizeof(*pProperties));
	iCount = 0;
	sCursor = __uiDesignText(pNode, "style.inlineProperties", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == '\0' ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount <= 0 || arrFields[0][0] == '\0' ) continue;
		sValue = __uiDesignField(arrFields, iFieldCount, 2, "");
		iType = __uiDesignStyleValueTypeFromText(__uiDesignField(arrFields, iFieldCount, 1, ""), __uiDesignInferStyleValueType(sValue));
		if ( iType < XUI_STYLE_VALUE_NONE || iType > XUI_STYLE_VALUE_TOKEN ) continue;
		pProperty = &pProperties[iCount];
		memset(pProperty, 0, sizeof(*pProperty));
		pProperty->iSize = sizeof(*pProperty);
		pProperty->sName = __uiDesignTextPoolAdd(pNode, arrFields[0]);
		pProperty->iDirtyFlags = __uiDesignStyleDirtyFlagsFromText(__uiDesignField(arrFields, iFieldCount, 3, ""), 0u);
		pProperty->tValue.iSize = sizeof(pProperty->tValue);
		pProperty->tValue.iType = iType;
		switch ( iType ) {
		case XUI_STYLE_VALUE_INT:
			pProperty->tValue.iInt = __uiDesignParseIntText(sValue, 0);
			break;
		case XUI_STYLE_VALUE_FLOAT:
			pProperty->tValue.fFloat = __uiDesignParseFloatText(sValue, 0.0f);
			break;
		case XUI_STYLE_VALUE_COLOR:
			pProperty->tValue.iColor = __uiDesignParseColorText(sValue, XUI_COLOR_RGBA(0, 0, 0, 0));
			break;
		case XUI_STYLE_VALUE_BOOL:
			pProperty->tValue.iInt = __uiDesignParseBoolText(sValue, 0);
			break;
		case XUI_STYLE_VALUE_STRING:
		case XUI_STYLE_VALUE_TOKEN:
			pProperty->tValue.sText = __uiDesignTextPoolAdd(pNode, sValue);
			break;
		case XUI_STYLE_VALUE_NONE:
		default:
			break;
		}
		++iCount;
	}
	return iCount;
}

static void __uiDesignApplyInlineStyle(ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_style_property_t arrProperties[UI_DESIGN_INLINE_STYLE_CAPACITY];
	int iCount;

	if ( (pNode == NULL) || (pWidget == NULL) || !__uiDesignHasProperty(pNode, "style.inlineProperties") ) return;
	__uiDesignTextPoolReset(pNode);
	iCount = __uiDesignBuildInlineStyles(pNode, arrProperties, UI_DESIGN_COUNT_OF(arrProperties));
	(void)xuiWidgetSetInlineStyle(pWidget, (iCount > 0) ? arrProperties : NULL, iCount);
}

static int __uiDesignHasCachePolicyProperties(const ui_design_node_t* pNode)
{
	return __uiDesignHasProperty(pNode, "cache.policy") ||
	       __uiDesignHasProperty(pNode, "cache.updateAllStates") ||
	       __uiDesignHasProperty(pNode, "cache.clearOnUpdate") ||
	       __uiDesignHasProperty(pNode, "cache.pinned") ||
	       __uiDesignHasProperty(pNode, "cache.clearColor") ||
	       __uiDesignHasProperty(pNode, "cache.tileWidth") ||
	       __uiDesignHasProperty(pNode, "cache.tileHeight") ||
	       __uiDesignHasProperty(pNode, "cache.maxBytes");
}

static void __uiDesignApplyWidgetCache(ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_cache_policy_t tPolicy;
	const char* sCursor;
	char sLine[128];
	char* arrFields[2];
	int arrIndex[UI_DESIGN_CACHE_STATE_ID_CAPACITY];
	uint32_t arrStateId[UI_DESIGN_CACHE_STATE_ID_CAPACITY];
	int iFieldCount;
	int iCount;
	int iStateCount;
	int iMaxIndex;
	int iValue;
	int i;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	if ( __uiDesignHasCachePolicyProperties(pNode) ) {
		memset(&tPolicy, 0, sizeof(tPolicy));
		tPolicy.iSize = sizeof(tPolicy);
		tPolicy.iPolicy = __uiDesignInt(pNode, "cache.policy", XUI_CACHE_POLICY_NONE);
		if ( __uiDesignBool(pNode, "cache.updateAllStates", 0) ) tPolicy.iFlags |= XUI_CACHE_UPDATE_ALL_STATES;
		if ( __uiDesignBool(pNode, "cache.clearOnUpdate", 0) ) tPolicy.iFlags |= XUI_CACHE_CLEAR_ON_UPDATE;
		if ( __uiDesignBool(pNode, "cache.pinned", 0) ) tPolicy.iFlags |= XUI_CACHE_PINNED;
		tPolicy.iClearColor = __uiDesignColor(pNode, "cache.clearColor", XUI_COLOR_RGBA(0, 0, 0, 0));
		iValue = __uiDesignInt(pNode, "cache.tileWidth", 0);
		tPolicy.iTileWidth = (iValue > 0) ? iValue : 0;
		iValue = __uiDesignInt(pNode, "cache.tileHeight", 0);
		tPolicy.iTileHeight = (iValue > 0) ? iValue : 0;
		iValue = __uiDesignInt(pNode, "cache.maxBytes", 0);
		tPolicy.iMaxBytes = (iValue > 0) ? (size_t)iValue : 0u;
		(void)xuiWidgetSetCachePolicy(pWidget, &tPolicy);
	}
	iStateCount = -1;
	if ( __uiDesignTryInt(pNode, "cache.stateCount", &iValue) ) {
		iStateCount = (iValue > 0) ? iValue : 0;
		(void)xuiWidgetSetCacheStateCount(pWidget, iStateCount);
	}
	if ( !__uiDesignHasProperty(pNode, "cache.stateIds") ) return;
	iCount = 0;
	iMaxIndex = -1;
	sCursor = __uiDesignText(pNode, "cache.stateIds", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < UI_DESIGN_COUNT_OF(arrIndex) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 || !__uiDesignLooksIntText(arrFields[0]) || !__uiDesignLooksIntText(arrFields[1]) ) continue;
		arrIndex[iCount] = __uiDesignParseIntText(arrFields[0], -1);
		arrStateId[iCount] = (uint32_t)__uiDesignParseIntText(arrFields[1], 0);
		if ( arrIndex[iCount] < 0 ) continue;
		if ( arrIndex[iCount] > iMaxIndex ) iMaxIndex = arrIndex[iCount];
		++iCount;
	}
	if ( iMaxIndex < 0 ) return;
	if ( iStateCount <= iMaxIndex ) {
		iStateCount = iMaxIndex + 1;
		(void)xuiWidgetSetCacheStateCount(pWidget, iStateCount);
	}
	for ( i = 0; i < iCount; ++i ) {
		if ( arrIndex[i] >= 0 && arrIndex[i] < iStateCount ) {
			(void)xuiWidgetSetCacheStateId(pWidget, arrIndex[i], arrStateId[i]);
		}
	}
}

static int __uiDesignTextContainsNoCase(const char* sText, const char* sNeedle)
{
	char sHay[128];
	char sSub[64];
	int i;

	if ( (sText == NULL) || (sNeedle == NULL) ) return 0;
	for ( i = 0; sText[i] != 0 && i < (int)sizeof(sHay) - 1; ++i ) {
		sHay[i] = (char)__uiDesignLower((unsigned char)sText[i]);
	}
	sHay[i] = 0;
	for ( i = 0; sNeedle[i] != 0 && i < (int)sizeof(sSub) - 1; ++i ) {
		sSub[i] = (char)__uiDesignLower((unsigned char)sNeedle[i]);
	}
	sSub[i] = 0;
	return strstr(sHay, sSub) != NULL;
}

static int __uiDesignTableCellTypeFromText(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "text") || __uiDesignTokenIs(sText, "string") ) return XUI_TABLE_CELL_TYPE_TEXT;
	if ( __uiDesignTokenIs(sText, "int") || __uiDesignTokenIs(sText, "integer") ) return XUI_TABLE_CELL_TYPE_INT;
	if ( __uiDesignTokenIs(sText, "float") || __uiDesignTokenIs(sText, "number") || __uiDesignTokenIs(sText, "double") ) return XUI_TABLE_CELL_TYPE_FLOAT;
	if ( __uiDesignTokenIs(sText, "bool") || __uiDesignTokenIs(sText, "boolean") || __uiDesignTokenIs(sText, "check") ) return XUI_TABLE_CELL_TYPE_BOOL;
	if ( __uiDesignTokenIs(sText, "textarea") || __uiDesignTokenIs(sText, "memo") || __uiDesignTokenIs(sText, "multiline") ) return XUI_TABLE_CELL_TYPE_TEXTAREA;
	if ( __uiDesignTokenIs(sText, "date") ) return XUI_TABLE_CELL_TYPE_DATE;
	if ( __uiDesignTokenIs(sText, "time") ) return XUI_TABLE_CELL_TYPE_TIME;
	if ( __uiDesignTokenIs(sText, "datetime") ) return XUI_TABLE_CELL_TYPE_DATETIME;
	if ( __uiDesignTokenIs(sText, "enum") || __uiDesignTokenIs(sText, "combo") ) return XUI_TABLE_CELL_TYPE_ENUM;
	if ( __uiDesignTokenIs(sText, "color") ) return XUI_TABLE_CELL_TYPE_COLOR;
	if ( __uiDesignTokenIs(sText, "picker") ) return XUI_TABLE_CELL_TYPE_PICKER;
	if ( __uiDesignTokenIs(sText, "custom") ) return XUI_TABLE_CELL_TYPE_CUSTOM;
	if ( __uiDesignTokenIs(sText, "file") ) return XUI_TABLE_CELL_TYPE_FILE;
	if ( __uiDesignTokenIs(sText, "image") ) return XUI_TABLE_CELL_TYPE_IMAGE;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static int __uiDesignDefaultTableCellType(const char* sTitle, int iColumn, int bGrid)
{
	if ( !bGrid ) return XUI_TABLE_CELL_TYPE_TEXT;
	if ( __uiDesignTextContainsNoCase(sTitle, "color") || __uiDesignTextContainsNoCase(sTitle, "colour") ) return XUI_TABLE_CELL_TYPE_COLOR;
	if ( __uiDesignTextContainsNoCase(sTitle, "enabled") || __uiDesignTextContainsNoCase(sTitle, "visible") || __uiDesignTextContainsNoCase(sTitle, "checked") ) return XUI_TABLE_CELL_TYPE_BOOL;
	if ( iColumn > 0 && (__uiDesignTextContainsNoCase(sTitle, "width") || __uiDesignTextContainsNoCase(sTitle, "height") ||
	     __uiDesignTextContainsNoCase(sTitle, "size") || __uiDesignTextContainsNoCase(sTitle, "value") ||
	     __uiDesignTextEqualsNoCase(sTitle, "x") || __uiDesignTextEqualsNoCase(sTitle, "y")) ) {
		return XUI_TABLE_CELL_TYPE_FLOAT;
	}
	return XUI_TABLE_CELL_TYPE_TEXT;
}

static int __uiDesignTableAlignFromText(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "left") ) return XUI_TEXT_ALIGN_LEFT;
	if ( __uiDesignTokenIs(sText, "center") || __uiDesignTokenIs(sText, "middle") ) return XUI_TEXT_ALIGN_CENTER;
	if ( __uiDesignTokenIs(sText, "right") ) return XUI_TEXT_ALIGN_RIGHT;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static int __uiDesignBuildTreeNodes(ui_design_node_t* pNode, xui_tree_view_node_t* pNodes, int iCapacity)
{
	const char* sCursor;
	char sLine[512];
	char* arrFields[9];
	int arrParentByDepth[32];
	int iCount;
	int iFieldCount;
	int iDepth;
	int iSpaces;
	int iTabs;
	int i;
	int iParent;
	int bExplicit;

	if ( (pNode == NULL) || (pNodes == NULL) || (iCapacity <= 0) ) return 0;
	memset(arrParentByDepth, 0, sizeof(arrParentByDepth));
	memset(pNodes, 0, (size_t)iCapacity * sizeof(*pNodes));
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.nodes", "Root\n  Child");
	while ( __uiDesignNextRawLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		iSpaces = 0;
		iTabs = 0;
		for ( i = 0; sLine[i] == ' ' || sLine[i] == '\t'; ++i ) {
			if ( sLine[i] == '\t' ) ++iTabs;
			else ++iSpaces;
		}
		iDepth = iTabs + (iSpaces / 2);
		__uiDesignTrim(sLine);
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( arrFields[0][0] == 0 ) continue;
		bExplicit = (iFieldCount >= 4) && __uiDesignLooksIntText(arrFields[0]) && __uiDesignLooksIntText(arrFields[1]) && __uiDesignLooksIntText(arrFields[2]);
		memset(&pNodes[iCount], 0, sizeof(pNodes[iCount]));
		if ( bExplicit ) {
			pNodes[iCount].iId = __uiDesignParseIntText(arrFields[0], iCount + 1);
			pNodes[iCount].iParent = __uiDesignParseIntText(arrFields[1], -1);
			pNodes[iCount].iDepth = __uiDesignParseIntText(arrFields[2], 0);
			pNodes[iCount].sText = __uiDesignTextPoolAdd(pNode, arrFields[3]);
			pNodes[iCount].bExpanded = (iFieldCount > 4) ? __uiDesignParseBoolText(arrFields[4], 1) : 1;
			pNodes[iCount].bEnabled = (iFieldCount > 5) ? __uiDesignParseBoolText(arrFields[5], 1) : 1;
			if ( iFieldCount >= 9 ) {
				pNodes[iCount].bIconReserved = __uiDesignParseBoolText(arrFields[6], 1);
				pNodes[iCount].bCheckReserved = __uiDesignParseBoolText(arrFields[7], 0);
				pNodes[iCount].bChecked = __uiDesignParseBoolText(arrFields[8], 0);
			} else {
				pNodes[iCount].bIconReserved = 1;
				pNodes[iCount].bCheckReserved = (iFieldCount > 6);
				pNodes[iCount].bChecked = (iFieldCount > 6) ? __uiDesignParseBoolText(arrFields[6], 0) : 0;
			}
			if ( pNodes[iCount].iDepth >= 0 && pNodes[iCount].iDepth < (int)UI_DESIGN_COUNT_OF(arrParentByDepth) ) {
				arrParentByDepth[pNodes[iCount].iDepth] = pNodes[iCount].iId;
			}
		} else {
			if ( iDepth < 0 ) iDepth = 0;
			if ( iDepth >= (int)UI_DESIGN_COUNT_OF(arrParentByDepth) ) iDepth = (int)UI_DESIGN_COUNT_OF(arrParentByDepth) - 1;
			iParent = (iDepth > 0) ? arrParentByDepth[iDepth - 1] : -1;
			pNodes[iCount].iId = (iFieldCount > 6) ? __uiDesignParseIntText(arrFields[6], iCount + 1) :
				((iFieldCount > 4) ? __uiDesignParseIntText(arrFields[4], iCount + 1) : (iCount + 1));
			pNodes[iCount].iParent = iParent;
			pNodes[iCount].iDepth = iDepth;
			pNodes[iCount].sText = __uiDesignTextPoolAdd(pNode, arrFields[0]);
			pNodes[iCount].bExpanded = (iFieldCount > 1) ? __uiDesignParseBoolText(arrFields[1], 1) : 1;
			pNodes[iCount].bEnabled = (iFieldCount > 2) ? __uiDesignParseBoolText(arrFields[2], 1) : 1;
			if ( iFieldCount > 5 ) {
				pNodes[iCount].bIconReserved = __uiDesignParseBoolText(arrFields[3], 1);
				pNodes[iCount].bCheckReserved = __uiDesignParseBoolText(arrFields[4], 0);
				pNodes[iCount].bChecked = __uiDesignParseBoolText(arrFields[5], 0);
			} else {
				pNodes[iCount].bIconReserved = 1;
				pNodes[iCount].bCheckReserved = (iFieldCount > 3);
				pNodes[iCount].bChecked = (iFieldCount > 3) ? __uiDesignParseBoolText(arrFields[3], 0) : 0;
			}
			arrParentByDepth[iDepth] = pNodes[iCount].iId;
		}
		++iCount;
	}
	for ( i = 0; i < iCount; ++i ) {
		int j;
		pNodes[i].bHasChildren = 0;
		for ( j = 0; j < iCount; ++j ) {
			if ( pNodes[j].iParent == pNodes[i].iId ) {
				pNodes[i].bHasChildren = 1;
				break;
			}
		}
	}
	return iCount;
}

static int __uiDesignBuildTableColumns(ui_design_node_t* pNode, xui_table_view_column_t* pColumns, int iCapacity, float fDefaultWidth, int bGrid)
{
	const char* sCursor;
	const char* sTitle;
	char sLine[512];
	char* arrFields[12];
	int iCount;
	int iFieldCount;
	int i;
	int iType;

	if ( (pNode == NULL) || (pColumns == NULL) || (iCapacity <= 0) ) return 0;
	memset(pColumns, 0, (size_t)iCapacity * sizeof(*pColumns));
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.columns", "Name,Width,Height");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		if ( strchr(sLine, '|') == NULL && strchr(sLine, ',') != NULL ) {
			iFieldCount = __uiDesignSplitDataFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
			for ( i = 0; i < iFieldCount && iCount < iCapacity; ++i ) {
				if ( arrFields[i][0] == 0 ) continue;
				sTitle = __uiDesignTextPoolAdd(pNode, arrFields[i]);
				iType = __uiDesignDefaultTableCellType(sTitle, iCount, bGrid);
				memset(&pColumns[iCount], 0, sizeof(pColumns[iCount]));
				pColumns[iCount].iId = iCount + 1;
				pColumns[iCount].sId = sTitle;
				pColumns[iCount].sTitle = sTitle;
				pColumns[iCount].fWidth = (iCount == 0) ? 130.0f : fDefaultWidth;
				pColumns[iCount].bVisible = 1;
				pColumns[iCount].bVisibleSet = 1;
				pColumns[iCount].bResizable = 1;
				pColumns[iCount].bResizableSet = 1;
				pColumns[iCount].iType = iType;
				pNode->arrRuntimeTableColumnType[iCount] = iType;
				++iCount;
			}
			continue;
		}
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( arrFields[0][0] == 0 ) continue;
		sTitle = __uiDesignTextPoolAdd(pNode, arrFields[0]);
		iType = __uiDesignTableCellTypeFromText(__uiDesignField(arrFields, iFieldCount, 2, ""), __uiDesignDefaultTableCellType(sTitle, iCount, bGrid));
		memset(&pColumns[iCount], 0, sizeof(pColumns[iCount]));
		pColumns[iCount].iId = iCount + 1;
		pColumns[iCount].sId = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 3, sTitle));
		pColumns[iCount].sTitle = sTitle;
		pColumns[iCount].fWidth = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 1, ""), (iCount == 0) ? 130.0f : fDefaultWidth);
		pColumns[iCount].bVisible = (iFieldCount > 4) ? __uiDesignParseBoolText(arrFields[4], 1) : 1;
		pColumns[iCount].bVisibleSet = 1;
		pColumns[iCount].bResizable = (iFieldCount > 5) ? __uiDesignParseBoolText(arrFields[5], 1) : 1;
		pColumns[iCount].bResizableSet = 1;
		pColumns[iCount].iAlign = (iFieldCount > 6) ? __uiDesignTableAlignFromText(arrFields[6], XUI_TEXT_ALIGN_LEFT) : XUI_TEXT_ALIGN_LEFT;
		pColumns[iCount].fMinWidth = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 7, "24"), 24.0f);
		pColumns[iCount].fMaxWidth = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 8, "0"), 0.0f);
		if ( __uiDesignField(arrFields, iFieldCount, 9, "")[0] != 0 ||
		     __uiDesignField(arrFields, iFieldCount, 10, "")[0] != 0 ||
		     __uiDesignField(arrFields, iFieldCount, 11, "")[0] != 0 ) {
			pColumns[iCount].bHasStyle = 1;
			pColumns[iCount].iBackgroundColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 9, ""), 0u);
			pColumns[iCount].iTextColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 10, ""), 0u);
			pColumns[iCount].iGridColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 11, ""), 0u);
		}
		pColumns[iCount].iType = iType;
		pNode->arrRuntimeTableColumnType[iCount] = iType;
		++iCount;
	}
	if ( iCount == 0 ) {
		const char* arrDefaultTitles[] = {"Name", "Width", "Height"};
		for ( i = 0; i < 3 && iCount < iCapacity; ++i ) {
			sTitle = __uiDesignTextPoolAdd(pNode, arrDefaultTitles[i]);
			iType = __uiDesignDefaultTableCellType(sTitle, i, bGrid);
			memset(&pColumns[iCount], 0, sizeof(pColumns[iCount]));
			pColumns[iCount].iId = iCount + 1;
			pColumns[iCount].sId = sTitle;
			pColumns[iCount].sTitle = sTitle;
			pColumns[iCount].fWidth = (iCount == 0) ? 130.0f : fDefaultWidth;
			pColumns[iCount].bVisible = 1;
			pColumns[iCount].bVisibleSet = 1;
			pColumns[iCount].bResizable = 1;
			pColumns[iCount].bResizableSet = 1;
			pColumns[iCount].iType = iType;
			pNode->arrRuntimeTableColumnType[iCount] = iType;
			++iCount;
		}
	}
	pNode->iRuntimeTableColumnCount = iCount;
	return iCount;
}

static int __uiDesignBuildTableRows(ui_design_node_t* pNode, xui_table_view_row_t* pRows, int iCapacity, int iColumnCount, float fRowHeight)
{
	const char* sCursor;
	char sLine[1024];
	char* arrFields[UI_DESIGN_RUNTIME_TABLE_COLUMNS];
	int iCount;
	int iFieldCount;
	int i;
	int iTextIndex;

	if ( (pNode == NULL) || (pRows == NULL) || (iCapacity <= 0) || (iColumnCount <= 0) ) return 0;
	memset(pRows, 0, (size_t)iCapacity * sizeof(*pRows));
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.rows", "Button,118,32\nInput,180,30\nTable,320,180");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitDataFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount <= 0 ) continue;
		pRows[iCount].fHeight = fRowHeight;
		for ( i = 0; i < iColumnCount && i < UI_DESIGN_RUNTIME_TABLE_COLUMNS; ++i ) {
			iTextIndex = __uiDesignTextPoolAddIndex(pNode, (i < iFieldCount) ? arrFields[i] : "");
			pNode->arrRuntimeTableText[iCount][i] = iTextIndex;
		}
		++iCount;
	}
	if ( iCount == 0 ) {
		for ( iCount = 0; iCount < 3 && iCount < iCapacity; ++iCount ) {
			pRows[iCount].fHeight = fRowHeight;
			for ( i = 0; i < iColumnCount && i < 3 && i < UI_DESIGN_RUNTIME_TABLE_COLUMNS; ++i ) {
				iTextIndex = __uiDesignTextPoolAddIndex(pNode, g_arrTableCellText[iCount][i]);
				pNode->arrRuntimeTableText[iCount][i] = iTextIndex;
			}
		}
	}
	pNode->iRuntimeTableRowCount = iCount;
	return iCount;
}

static void __uiDesignApplyTableRowStyles(ui_design_node_t* pNode, xui_table_view_row_t* pRows, int iCapacity, int* pRowCount, float fDefaultRowHeight)
{
	const char* sCursor;
	char sLine[512];
	char* arrFields[7];
	int iFieldCount;
	int iRow;
	int i;

	if ( (pNode == NULL) || (pRows == NULL) || (pRowCount == NULL) || (iCapacity <= 0) ) return;
	sCursor = __uiDesignText(pNode, "data.rowStyles", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount <= 0) || !__uiDesignLooksIntText(arrFields[0]) ) continue;
		iRow = __uiDesignParseIntText(arrFields[0], -1);
		if ( (iRow < 0) || (iRow >= iCapacity) ) continue;
		for ( i = *pRowCount; i <= iRow && i < iCapacity; ++i ) {
			pRows[i].fHeight = fDefaultRowHeight;
		}
		if ( iRow >= *pRowCount ) *pRowCount = iRow + 1;
		if ( __uiDesignField(arrFields, iFieldCount, 1, "")[0] != 0 ) pRows[iRow].fHeight = __uiDesignParseFloatText(arrFields[1], fDefaultRowHeight);
		if ( __uiDesignField(arrFields, iFieldCount, 2, "")[0] != 0 ) pRows[iRow].bSelected = __uiDesignParseBoolText(arrFields[2], 0);
		if ( __uiDesignField(arrFields, iFieldCount, 3, "")[0] != 0 ) pRows[iRow].bDisabled = __uiDesignParseBoolText(arrFields[3], 0);
		if ( __uiDesignField(arrFields, iFieldCount, 4, "")[0] != 0 ||
		     __uiDesignField(arrFields, iFieldCount, 5, "")[0] != 0 ||
		     __uiDesignField(arrFields, iFieldCount, 6, "")[0] != 0 ) {
			pRows[iRow].bHasStyle = 1;
			pRows[iRow].iBackgroundColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 4, ""), 0u);
			pRows[iRow].iTextColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 5, ""), 0u);
			pRows[iRow].iGridColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 6, ""), 0u);
		}
	}
}

static void __uiDesignApplyTableCellStyles(ui_design_node_t* pNode, xui_table_view_row_t* pRows, int iRowCapacity, int iColumnCount, int* pRowCount, float fDefaultRowHeight)
{
	const char* sCursor;
	const char* sField;
	char sLine[1024];
	char* arrFields[13];
	xui_table_view_cell_t* pCell;
	int iFieldCount;
	int iRow;
	int iColumn;
	int i;

	if ( (pNode == NULL) || (pRows == NULL) || (pRowCount == NULL) || (iRowCapacity <= 0) || (iColumnCount <= 0) ) return;
	sCursor = __uiDesignText(pNode, "data.cellStyles", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 || !__uiDesignLooksIntText(arrFields[0]) || !__uiDesignLooksIntText(arrFields[1]) ) continue;
		iRow = __uiDesignParseIntText(arrFields[0], -1);
		iColumn = __uiDesignParseIntText(arrFields[1], -1);
		if ( (iRow < 0) || (iRow >= iRowCapacity) || (iColumn < 0) || (iColumn >= iColumnCount) || (iColumn >= UI_DESIGN_RUNTIME_TABLE_COLUMNS) ) continue;
		for ( i = *pRowCount; i <= iRow && i < iRowCapacity; ++i ) {
			pRows[i].fHeight = fDefaultRowHeight;
		}
		if ( iRow >= *pRowCount ) *pRowCount = iRow + 1;
		pCell = &pNode->arrRuntimeTableCells[iRow][iColumn];
		sField = __uiDesignField(arrFields, iFieldCount, 2, "");
		if ( sField[0] != 0 ) {
			pCell->iType = __uiDesignTableCellTypeFromText(sField, XUI_TABLE_CELL_TYPE_TEXT);
			pNode->arrRuntimeTableCellTypeSet[iRow][iColumn] = 1;
		}
		sField = __uiDesignField(arrFields, iFieldCount, 3, "");
		if ( sField[0] != 0 ) pCell->sTooltip = __uiDesignTextPoolAdd(pNode, sField);
		sField = __uiDesignField(arrFields, iFieldCount, 4, "");
		if ( sField[0] != 0 ) pCell->iRowSpan = __uiDesignParseIntText(sField, 1);
		sField = __uiDesignField(arrFields, iFieldCount, 5, "");
		if ( sField[0] != 0 ) pCell->iColSpan = __uiDesignParseIntText(sField, 1);
		if ( __uiDesignField(arrFields, iFieldCount, 6, "")[0] != 0 ) pCell->bDisabled = __uiDesignParseBoolText(arrFields[6], 0);
		if ( __uiDesignField(arrFields, iFieldCount, 7, "")[0] != 0 ) pCell->bEditing = __uiDesignParseBoolText(arrFields[7], 0);
		if ( __uiDesignField(arrFields, iFieldCount, 8, "")[0] != 0 ) pCell->bInvalid = __uiDesignParseBoolText(arrFields[8], 0);
		if ( __uiDesignField(arrFields, iFieldCount, 9, "")[0] != 0 ) pCell->bDirty = __uiDesignParseBoolText(arrFields[9], 0);
		if ( __uiDesignField(arrFields, iFieldCount, 10, "")[0] != 0 ||
		     __uiDesignField(arrFields, iFieldCount, 11, "")[0] != 0 ||
		     __uiDesignField(arrFields, iFieldCount, 12, "")[0] != 0 ) {
			pCell->bHasStyle = 1;
			pCell->iBackgroundColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 10, ""), 0u);
			pCell->iTextColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 11, ""), 0u);
			pCell->iGridColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 12, ""), 0u);
		}
	}
}

static int __uiDesignBuildTableData(ui_design_node_t* pNode, xui_table_view_column_t* pColumns, int* pColumnCount, xui_table_view_row_t* pRows, int* pRowCount, int bGrid)
{
	float fDefaultColumnWidth;
	float fRowHeight;
	int iColumns;
	int iRows;

	if ( (pNode == NULL) || (pColumns == NULL) || (pColumnCount == NULL) || (pRows == NULL) || (pRowCount == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignTextPoolReset(pNode);
	__uiDesignRuntimeTableReset(pNode);
	fDefaultColumnWidth = __uiDesignFloat(pNode, "metrics.columnWidth", 100.0f);
	fRowHeight = __uiDesignFloat(pNode, "metrics.rowHeight", 24.0f);
	iColumns = __uiDesignBuildTableColumns(pNode, pColumns, UI_DESIGN_RUNTIME_TABLE_COLUMNS, fDefaultColumnWidth, bGrid);
	iRows = __uiDesignBuildTableRows(pNode, pRows, UI_DESIGN_RUNTIME_TABLE_ROWS, iColumns, fRowHeight);
	__uiDesignApplyTableRowStyles(pNode, pRows, UI_DESIGN_RUNTIME_TABLE_ROWS, &iRows, fRowHeight);
	__uiDesignApplyTableCellStyles(pNode, pRows, UI_DESIGN_RUNTIME_TABLE_ROWS, iColumns, &iRows, fRowHeight);
	pNode->iRuntimeTableRowCount = iRows;
	*pColumnCount = iColumns;
	*pRowCount = iRows;
	return XUI_OK;
}

static int __uiDesignBuildBreadcrumbItems(ui_design_node_t* pNode, xui_breadcrumb_item_t* pItems, int iCapacity)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[4];
	int arrExplicit[64];
	int iCount;
	int iFieldCount;
	int i;

	if ( (pNode == NULL) || (pItems == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.items", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( arrFields[0][0] == 0 ) continue;
		memset(&pItems[iCount], 0, sizeof(pItems[iCount]));
		pItems[iCount].sText = __uiDesignTextPoolAdd(pNode, arrFields[0]);
		pItems[iCount].bClickable = (iFieldCount > 1) ? __uiDesignParseBoolText(arrFields[1], 1) : 1;
		pItems[iCount].iValue = (iFieldCount > 2) ? __uiDesignParseIntText(arrFields[2], iCount + 1) : (iCount + 1);
		arrExplicit[iCount] = (iFieldCount > 1);
		++iCount;
	}
	if ( iCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultBreadcrumbItems) && iCount < iCapacity; ++i ) {
			pItems[iCount] = g_arrDefaultBreadcrumbItems[i];
			pItems[iCount].sText = __uiDesignTextPoolAdd(pNode, g_arrDefaultBreadcrumbItems[i].sText);
			arrExplicit[iCount] = 1;
			++iCount;
		}
	} else if ( !arrExplicit[iCount - 1] ) {
		pItems[iCount - 1].bClickable = 0;
	}
	return iCount;
}

static int __uiDesignBuildTabsItems(struct ui_design_app_t* pApp, ui_design_node_t* pNode, const char** ppTitles, int* pEnabled, int* pDirty, xui_surface* ppIcons, xui_rect_t* pIconSrc, int iCapacity)
{
	const char* sCursor;
	char sLine[512];
	char* arrFields[8];
	xui_surface pSurface;
	xui_rect_t tSrc;
	int iCount;
	int iFieldCount;
	int i;

	if ( (pNode == NULL) || (ppTitles == NULL) || (pEnabled == NULL) || (pDirty == NULL) || (iCapacity <= 0) ) return 0;
	for ( i = 0; i < iCapacity; ++i ) {
		if ( ppIcons != NULL ) ppIcons[i] = NULL;
		if ( pIconSrc != NULL ) pIconSrc[i] = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	}
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.items", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( arrFields[0][0] == 0 ) continue;
		ppTitles[iCount] = __uiDesignTextPoolAdd(pNode, arrFields[0]);
		pEnabled[iCount] = 1;
		pDirty[iCount] = 0;
		if ( iFieldCount > 1 ) {
			if ( __uiDesignTokenIs(arrFields[1], "dirty") ) pDirty[iCount] = 1;
			else if ( __uiDesignTokenIs(arrFields[1], "disabled") ) pEnabled[iCount] = 0;
			else pEnabled[iCount] = __uiDesignParseBoolText(arrFields[1], 1);
		}
		if ( iFieldCount > 2 ) {
			if ( __uiDesignTokenIs(arrFields[2], "dirty") ) pDirty[iCount] = 1;
			else pDirty[iCount] = __uiDesignParseBoolText(arrFields[2], 0);
		}
		if ( (ppIcons != NULL) && (pIconSrc != NULL) && (pApp != NULL) && (iFieldCount > 3) ) {
			pSurface = NULL;
			(void)__uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, iCount, arrFields[3], &pSurface);
			ppIcons[iCount] = pSurface;
			memset(&tSrc, 0, sizeof(tSrc));
			if ( iFieldCount > 7 ) {
				tSrc.fX = __uiDesignParseFloatText(arrFields[4], 0.0f);
				tSrc.fY = __uiDesignParseFloatText(arrFields[5], 0.0f);
				tSrc.fW = __uiDesignParseFloatText(arrFields[6], 0.0f);
				tSrc.fH = __uiDesignParseFloatText(arrFields[7], 0.0f);
				if ( (tSrc.fW <= 0.0f) || (tSrc.fH <= 0.0f) ) memset(&tSrc, 0, sizeof(tSrc));
			}
			pIconSrc[iCount] = tSrc;
		} else if ( (ppIcons != NULL) && (pApp != NULL) ) {
			pSurface = NULL;
			(void)__uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, iCount, "", &pSurface);
		}
		++iCount;
	}
	if ( iCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultTabs) && iCount < iCapacity; ++i ) {
			ppTitles[iCount] = __uiDesignTextPoolAdd(pNode, g_arrDefaultTabs[i]);
			pEnabled[iCount] = g_arrDefaultEnabled[i];
			pDirty[iCount] = g_arrDefaultDirty[i];
			++iCount;
		}
	}
	if ( pApp != NULL ) {
		for ( i = iCount; i < iCapacity && i < UI_DESIGN_RUNTIME_SURFACE_COUNT; ++i ) {
			pSurface = NULL;
			(void)__uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, i, "", &pSurface);
		}
	}
	return iCount;
}

static int __uiDesignBuildAccordionSections(ui_design_node_t* pNode, xui_accordion_section_desc_t* pSections, int iCapacity)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[5];
	int iCount;
	int iFieldCount;
	int i;

	if ( (pNode == NULL) || (pSections == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.sections", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( arrFields[0][0] == 0 ) continue;
		memset(&pSections[iCount], 0, sizeof(pSections[iCount]));
		pSections[iCount].sTitle = __uiDesignTextPoolAdd(pNode, arrFields[0]);
		pSections[iCount].iId = (iFieldCount > 3) ? __uiDesignParseIntText(arrFields[3], iCount + 1) : (iCount + 1);
		pSections[iCount].bExpanded = (iFieldCount > 1) ? __uiDesignParseBoolText(arrFields[1], iCount == 0) : (iCount == 0);
		pSections[iCount].bDisabled = (iFieldCount > 2) ? __uiDesignParseBoolText(arrFields[2], 0) : 0;
		++iCount;
	}
	if ( iCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultAccordionSections) && iCount < iCapacity; ++i ) {
			pSections[iCount] = g_arrDefaultAccordionSections[i];
			pSections[iCount].sTitle = __uiDesignTextPoolAdd(pNode, g_arrDefaultAccordionSections[i].sTitle);
			++iCount;
		}
	}
	return iCount;
}

static int __uiDesignToolbarItemType(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "button") ) return XUI_TOOLBAR_ITEM_BUTTON;
	if ( __uiDesignTokenIs(sText, "toggle") || __uiDesignTokenIs(sText, "radio") ) return XUI_TOOLBAR_ITEM_TOGGLE;
	if ( __uiDesignTokenIs(sText, "separator") || __uiDesignTokenIs(sText, "-") ) return XUI_TOOLBAR_ITEM_SEPARATOR;
	return iDefault;
}

static int __uiDesignMenuItemType(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "normal") || __uiDesignTokenIs(sText, "button") ) return XUI_MENU_ITEM_NORMAL;
	if ( __uiDesignTokenIs(sText, "separator") || __uiDesignTokenIs(sText, "-") ) return XUI_MENU_ITEM_SEPARATOR;
	if ( __uiDesignTokenIs(sText, "check") || __uiDesignTokenIs(sText, "checkbox") ) return XUI_MENU_ITEM_CHECK;
	if ( __uiDesignTokenIs(sText, "radio") ) return XUI_MENU_ITEM_RADIO;
	if ( __uiDesignTokenIs(sText, "submenu") ) return XUI_MENU_ITEM_SUBMENU;
	return iDefault;
}

static int __uiDesignBuildMenuBarItems(ui_design_node_t* pNode, xui_menubar_item_t* pItems, int iCapacity)
{
	const char* sCursor;
	const char* sTitle;
	char sLine[256];
	char* arrFields[5];
	int iCount;
	int iFieldCount;
	int i;
	int j;

	if ( (pNode == NULL) || (pItems == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.items", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( arrFields[0][0] == 0 ) continue;
		memset(&pItems[iCount], 0, sizeof(pItems[iCount]));
		sTitle = __uiDesignTextPoolAdd(pNode, arrFields[0]);
		pItems[iCount].sText = sTitle;
		pItems[iCount].iState = XUI_MENUBAR_ITEM_ENABLED;
		pItems[iCount].iValue = iCount + 1;
		pItems[iCount].iMnemonic = (sTitle[0] == '&' && sTitle[1] != 0) ? sTitle[1] : sTitle[0];
		for ( j = 1; j < iFieldCount; ++j ) {
			if ( j == 1 && __uiDesignIsBoolText(arrFields[j]) ) {
				if ( __uiDesignParseBoolText(arrFields[j], 1) ) pItems[iCount].iState |= XUI_MENUBAR_ITEM_ENABLED;
				else pItems[iCount].iState &= ~XUI_MENUBAR_ITEM_ENABLED;
			} else if ( j == 2 && arrFields[j][0] != 0 ) {
				pItems[iCount].iMnemonic = arrFields[j][0];
			} else if ( j == 3 ) {
				pItems[iCount].iValue = __uiDesignParseIntText(arrFields[j], pItems[iCount].iValue);
			} else if ( __uiDesignTokenIs(arrFields[j], "disabled") ) {
				pItems[iCount].iState &= ~XUI_MENUBAR_ITEM_ENABLED;
			} else if ( __uiDesignTokenIs(arrFields[j], "enabled") ) {
				pItems[iCount].iState |= XUI_MENUBAR_ITEM_ENABLED;
			} else if ( strlen(arrFields[j]) == 1u ) {
				pItems[iCount].iMnemonic = arrFields[j][0];
			} else {
				pItems[iCount].iValue = __uiDesignParseIntText(arrFields[j], pItems[iCount].iValue);
			}
		}
		++iCount;
	}
	if ( iCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultMenuBarItems) && iCount < iCapacity; ++i ) {
			pItems[iCount] = g_arrDefaultMenuBarItems[i];
			pItems[iCount].sText = __uiDesignTextPoolAdd(pNode, g_arrDefaultMenuBarItems[i].sText);
			++iCount;
		}
	}
	return iCount;
}

static int __uiDesignBuildToolbarItems(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_toolbar_item_t* pItems, int iCapacity)
{
	const char* sCursor;
	char sLine[1024];
	char* arrFields[12];
	xui_surface pIcon;
	xui_rect_t tIconSrc;
	float fX;
	float fY;
	float fW;
	float fH;
	int iCount;
	int iFieldCount;
	int i;
	int j;
	int bTooltipSet;
	int iType;

	if ( (pNode == NULL) || (pItems == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.items", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		memset(&pItems[iCount], 0, sizeof(pItems[iCount]));
		pItems[iCount].iType = XUI_TOOLBAR_ITEM_BUTTON;
		pItems[iCount].iState = XUI_TOOLBAR_ITEM_ENABLED;
		pItems[iCount].iValue = iCount + 1;
		bTooltipSet = 0;
		if ( __uiDesignTokenIs(arrFields[0], "-") || __uiDesignTokenIs(arrFields[0], "separator") ) {
			pItems[iCount].iType = XUI_TOOLBAR_ITEM_SEPARATOR;
			pItems[iCount].iState = 0;
		} else {
			pItems[iCount].sText = __uiDesignTextPoolAdd(pNode, arrFields[0]);
			pItems[iCount].sTooltip = pItems[iCount].sText;
		}
		for ( j = 1; j < iFieldCount; ++j ) {
			iType = (j == 1) ? __uiDesignToolbarItemType(arrFields[j], -1) : -1;
			if ( iType >= 0 ) {
				pItems[iCount].iType = iType;
				if ( iType == XUI_TOOLBAR_ITEM_SEPARATOR ) pItems[iCount].iState = 0;
			} else if ( j == 1 && __uiDesignIsBoolText(arrFields[j]) ) {
				if ( __uiDesignParseBoolText(arrFields[j], 1) ) pItems[iCount].iState |= XUI_TOOLBAR_ITEM_ENABLED;
				else pItems[iCount].iState &= ~XUI_TOOLBAR_ITEM_ENABLED;
			} else if ( j == 2 && __uiDesignIsBoolText(arrFields[j]) ) {
				if ( __uiDesignParseBoolText(arrFields[j], 1) ) pItems[iCount].iState |= XUI_TOOLBAR_ITEM_ENABLED;
				else pItems[iCount].iState &= ~XUI_TOOLBAR_ITEM_ENABLED;
			} else if ( j == 3 && __uiDesignIsBoolText(arrFields[j]) ) {
				if ( __uiDesignParseBoolText(arrFields[j], 0) ) pItems[iCount].iState |= XUI_TOOLBAR_ITEM_CHECKED;
				else pItems[iCount].iState &= ~XUI_TOOLBAR_ITEM_CHECKED;
			} else if ( j == 4 ) {
				pItems[iCount].sTooltip = __uiDesignTextPoolAdd(pNode, arrFields[j]);
				bTooltipSet = 1;
			} else if ( j == 5 ) {
				pItems[iCount].iGroup = __uiDesignParseIntText(arrFields[j], pItems[iCount].iGroup);
			} else if ( j == 6 ) {
				pItems[iCount].iValue = __uiDesignParseIntText(arrFields[j], pItems[iCount].iValue);
			} else if ( __uiDesignTokenIs(arrFields[j], "separator") ) {
				pItems[iCount].iType = XUI_TOOLBAR_ITEM_SEPARATOR;
				pItems[iCount].iState = 0;
			} else if ( __uiDesignTokenIs(arrFields[j], "toggle") || __uiDesignTokenIs(arrFields[j], "radio") ) {
				pItems[iCount].iType = XUI_TOOLBAR_ITEM_TOGGLE;
			} else if ( __uiDesignTokenIs(arrFields[j], "button") ) {
				pItems[iCount].iType = XUI_TOOLBAR_ITEM_BUTTON;
			} else if ( __uiDesignTokenIs(arrFields[j], "checked") ) {
				pItems[iCount].iState |= XUI_TOOLBAR_ITEM_CHECKED;
			} else if ( __uiDesignTokenIs(arrFields[j], "disabled") ) {
				pItems[iCount].iState &= ~XUI_TOOLBAR_ITEM_ENABLED;
			} else if ( __uiDesignTokenIs(arrFields[j], "enabled") ) {
				pItems[iCount].iState |= XUI_TOOLBAR_ITEM_ENABLED;
			} else if ( !bTooltipSet ) {
				pItems[iCount].sTooltip = __uiDesignTextPoolAdd(pNode, arrFields[j]);
				bTooltipSet = 1;
			} else {
				pItems[iCount].iGroup = __uiDesignParseIntText(arrFields[j], pItems[iCount].iGroup);
			}
		}
		if ( (pApp != NULL) && (iCount < UI_DESIGN_RUNTIME_SURFACE_COUNT) ) {
			pIcon = NULL;
			(void)__uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, iCount, __uiDesignField(arrFields, iFieldCount, 7, ""), &pIcon);
			pItems[iCount].pIcon = pIcon;
			memset(&tIconSrc, 0, sizeof(tIconSrc));
			if ( pIcon != NULL ) {
				fX = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 8, ""), 0.0f);
				fY = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 9, ""), 0.0f);
				fW = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 10, ""), 0.0f);
				fH = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 11, ""), 0.0f);
				if ( fW > 0.0f && fH > 0.0f ) {
					tIconSrc.fX = fX;
					tIconSrc.fY = fY;
					tIconSrc.fW = fW;
					tIconSrc.fH = fH;
				}
			}
			pItems[iCount].tIconSrc = tIconSrc;
		}
		++iCount;
	}
	if ( iCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultToolbarItems) && iCount < iCapacity; ++i ) {
			pItems[iCount] = g_arrDefaultToolbarItems[i];
			pItems[iCount].sText = __uiDesignTextPoolAdd(pNode, g_arrDefaultToolbarItems[i].sText);
			pItems[iCount].sTooltip = __uiDesignTextPoolAdd(pNode, g_arrDefaultToolbarItems[i].sTooltip);
			++iCount;
		}
	}
	if ( pApp != NULL ) {
		for ( i = iCount; i < UI_DESIGN_RUNTIME_SURFACE_COUNT; ++i ) {
			pIcon = NULL;
			(void)__uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, i, "", &pIcon);
		}
	}
	return iCount;
}

static int __uiDesignStatusBarSection(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "left") ) return XUI_STATUSBAR_SECTION_LEFT;
	if ( __uiDesignTokenIs(sText, "center") ) return XUI_STATUSBAR_SECTION_CENTER;
	if ( __uiDesignTokenIs(sText, "right") ) return XUI_STATUSBAR_SECTION_RIGHT;
	return iDefault;
}

static int __uiDesignStatusBarType(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "text") ) return XUI_STATUSBAR_ITEM_TEXT;
	if ( __uiDesignTokenIs(sText, "progress") ) return XUI_STATUSBAR_ITEM_PROGRESS;
	if ( __uiDesignTokenIs(sText, "spacer") ) return XUI_STATUSBAR_ITEM_SPACER;
	return iDefault;
}

static int __uiDesignBuildStatusBarItems(ui_design_node_t* pNode, xui_statusbar_item_t* pItems, int iCapacity)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[11];
	int iCount;
	int iFieldCount;
	int i;
	int j;
	int iType;
	int iSection;

	if ( (pNode == NULL) || (pItems == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.items", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		memset(&pItems[iCount], 0, sizeof(pItems[iCount]));
		iType = __uiDesignStatusBarType(arrFields[0], XUI_STATUSBAR_ITEM_TEXT);
		if ( iFieldCount > 1 ) iType = __uiDesignStatusBarType(arrFields[1], iType);
		pItems[iCount].iType = iType;
		pItems[iCount].iSection = (iType == XUI_STATUSBAR_ITEM_SPACER) ? XUI_STATUSBAR_SECTION_CENTER : XUI_STATUSBAR_SECTION_LEFT;
		pItems[iCount].iState = (iType == XUI_STATUSBAR_ITEM_SPACER) ? 0u : XUI_STATUSBAR_ITEM_ENABLED;
		pItems[iCount].iValue = iCount + 1;
		pItems[iCount].fWidth = (iType == XUI_STATUSBAR_ITEM_TEXT) ? 96.0f : 0.0f;
		pItems[iCount].fFlex = (iType == XUI_STATUSBAR_ITEM_SPACER) ? 1.0f : 0.0f;
		pItems[iCount].fMin = 0.0f;
		pItems[iCount].fMax = 1.0f;
		pItems[iCount].fValue = 0.0f;
		pItems[iCount].sText = (iType == XUI_STATUSBAR_ITEM_SPACER && __uiDesignStatusBarType(arrFields[0], -1) >= 0) ? "" : __uiDesignTextPoolAdd(pNode, arrFields[0]);
		for ( j = 1; j < iFieldCount; ++j ) {
			iType = __uiDesignStatusBarType(arrFields[j], -1);
			iSection = __uiDesignStatusBarSection(arrFields[j], -1);
			if ( j == 1 && iType >= 0 ) {
				pItems[iCount].iType = iType;
			} else if ( (j == 2 && iSection >= 0) || (j != 2 && iSection >= 0) ) {
				pItems[iCount].iSection = iSection;
			} else if ( j == 3 && __uiDesignIsBoolText(arrFields[j]) ) {
				if ( __uiDesignParseBoolText(arrFields[j], 1) ) pItems[iCount].iState |= XUI_STATUSBAR_ITEM_ENABLED;
				else pItems[iCount].iState &= ~XUI_STATUSBAR_ITEM_ENABLED;
			} else if ( j == 4 && __uiDesignIsBoolText(arrFields[j]) ) {
				if ( __uiDesignParseBoolText(arrFields[j], 0) ) pItems[iCount].iState |= XUI_STATUSBAR_ITEM_CLICKABLE;
				else pItems[iCount].iState &= ~XUI_STATUSBAR_ITEM_CLICKABLE;
			} else if ( j == 5 && (arrFields[j][0] == 0 || __uiDesignLooksNumberText(arrFields[j])) ) {
				pItems[iCount].fWidth = __uiDesignParseFloatText(arrFields[j], pItems[iCount].fWidth);
			} else if ( j == 6 && (arrFields[j][0] == 0 || __uiDesignLooksNumberText(arrFields[j])) ) {
				pItems[iCount].fFlex = __uiDesignParseFloatText(arrFields[j], pItems[iCount].fFlex);
			} else if ( j == 7 && (arrFields[j][0] == 0 || __uiDesignLooksNumberText(arrFields[j])) ) {
				pItems[iCount].fMin = __uiDesignParseFloatText(arrFields[j], pItems[iCount].fMin);
			} else if ( j == 8 && (arrFields[j][0] == 0 || __uiDesignLooksNumberText(arrFields[j])) ) {
				pItems[iCount].fMax = __uiDesignParseFloatText(arrFields[j], pItems[iCount].fMax);
			} else if ( j == 9 && (arrFields[j][0] == 0 || __uiDesignLooksNumberText(arrFields[j])) ) {
				pItems[iCount].fValue = __uiDesignParseFloatText(arrFields[j], pItems[iCount].fValue);
			} else if ( j == 10 && (arrFields[j][0] == 0 || __uiDesignLooksIntText(arrFields[j])) ) {
				pItems[iCount].iValue = __uiDesignParseIntText(arrFields[j], pItems[iCount].iValue);
			} else if ( __uiDesignTokenIs(arrFields[j], "disabled") ) {
				pItems[iCount].iState &= ~XUI_STATUSBAR_ITEM_ENABLED;
			} else if ( __uiDesignTokenIs(arrFields[j], "enabled") ) {
				pItems[iCount].iState |= XUI_STATUSBAR_ITEM_ENABLED;
			} else if ( __uiDesignTokenIs(arrFields[j], "clickable") ) {
				pItems[iCount].iState |= XUI_STATUSBAR_ITEM_CLICKABLE;
			} else if ( __uiDesignIsBoolText(arrFields[j]) ) {
				if ( __uiDesignParseBoolText(arrFields[j], 1) ) pItems[iCount].iState |= XUI_STATUSBAR_ITEM_ENABLED;
				else pItems[iCount].iState &= ~XUI_STATUSBAR_ITEM_ENABLED;
			} else if ( iType >= 0 ) {
				pItems[iCount].iType = iType;
			} else if ( pItems[iCount].fWidth == 0.0f ) {
				pItems[iCount].fWidth = __uiDesignParseFloatText(arrFields[j], pItems[iCount].fWidth);
			} else {
				pItems[iCount].fFlex = __uiDesignParseFloatText(arrFields[j], pItems[iCount].fFlex);
			}
		}
		++iCount;
	}
	if ( iCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultStatusItems) && iCount < iCapacity; ++i ) {
			pItems[iCount] = g_arrDefaultStatusItems[i];
			pItems[iCount].sText = __uiDesignTextPoolAdd(pNode, g_arrDefaultStatusItems[i].sText);
			++iCount;
		}
	}
	return iCount;
}

static void __uiDesignFillMenuItemFromFields(ui_design_node_t* pNode, xui_menu_item_t* pItem, char** arrFields, int iFieldCount, int iOffset, int iDefaultValue)
{
	int j;
	int iColumn;
	int bShortcutSet;
	int iType;

	if ( (pNode == NULL) || (pItem == NULL) || (arrFields == NULL) || (iOffset < 0) || (iOffset >= iFieldCount) ) return;
	memset(pItem, 0, sizeof(*pItem));
	pItem->iType = XUI_MENU_ITEM_NORMAL;
	pItem->iState = XUI_MENU_ITEM_ENABLED;
	pItem->iValue = iDefaultValue;
	bShortcutSet = 0;
	if ( __uiDesignTokenIs(arrFields[iOffset], "-") || __uiDesignTokenIs(arrFields[iOffset], "separator") ) {
		pItem->iType = XUI_MENU_ITEM_SEPARATOR;
		pItem->iState = 0;
	} else {
		pItem->sText = __uiDesignTextPoolAdd(pNode, arrFields[iOffset]);
	}
	for ( j = iOffset + 1; j < iFieldCount; ++j ) {
		iColumn = j - iOffset;
		iType = (iColumn == 1) ? __uiDesignMenuItemType(arrFields[j], -1) : -1;
		if ( iType >= 0 ) {
			pItem->iType = iType;
			if ( iType == XUI_MENU_ITEM_SEPARATOR ) pItem->iState = 0;
		} else if ( iColumn == 1 && __uiDesignIsBoolText(arrFields[j]) ) {
			if ( __uiDesignParseBoolText(arrFields[j], 1) ) pItem->iState |= XUI_MENU_ITEM_ENABLED;
			else pItem->iState &= ~XUI_MENU_ITEM_ENABLED;
		} else if ( iColumn == 2 && __uiDesignIsBoolText(arrFields[j]) ) {
			if ( __uiDesignParseBoolText(arrFields[j], 1) ) pItem->iState |= XUI_MENU_ITEM_ENABLED;
			else pItem->iState &= ~XUI_MENU_ITEM_ENABLED;
		} else if ( iColumn == 3 && __uiDesignIsBoolText(arrFields[j]) ) {
			if ( __uiDesignParseBoolText(arrFields[j], 0) ) pItem->iState |= XUI_MENU_ITEM_CHECKED;
			else pItem->iState &= ~XUI_MENU_ITEM_CHECKED;
		} else if ( iColumn == 4 && __uiDesignIsBoolText(arrFields[j]) ) {
			if ( __uiDesignParseBoolText(arrFields[j], 0) ) pItem->iState |= XUI_MENU_ITEM_DEFAULT;
			else pItem->iState &= ~XUI_MENU_ITEM_DEFAULT;
		} else if ( iColumn == 5 && __uiDesignIsBoolText(arrFields[j]) ) {
			if ( __uiDesignParseBoolText(arrFields[j], 0) ) pItem->iState |= XUI_MENU_ITEM_DANGER;
			else pItem->iState &= ~XUI_MENU_ITEM_DANGER;
		} else if ( iColumn == 6 ) {
			pItem->sShortcut = __uiDesignTextPoolAdd(pNode, arrFields[j]);
			bShortcutSet = 1;
		} else if ( iColumn == 7 ) {
			pItem->iValue = __uiDesignParseIntText(arrFields[j], pItem->iValue);
		} else if ( iColumn == 8 ) {
			pItem->iIcon = __uiDesignParseIntText(arrFields[j], pItem->iIcon);
		} else if ( __uiDesignTokenIs(arrFields[j], "separator") ) {
			pItem->iType = XUI_MENU_ITEM_SEPARATOR;
			pItem->iState = 0;
		} else if ( __uiDesignTokenIs(arrFields[j], "check") ) {
			pItem->iType = XUI_MENU_ITEM_CHECK;
		} else if ( __uiDesignTokenIs(arrFields[j], "radio") ) {
			pItem->iType = XUI_MENU_ITEM_RADIO;
		} else if ( __uiDesignTokenIs(arrFields[j], "submenu") ) {
			pItem->iType = XUI_MENU_ITEM_SUBMENU;
		} else if ( __uiDesignTokenIs(arrFields[j], "checked") ) {
			pItem->iState |= XUI_MENU_ITEM_CHECKED;
		} else if ( __uiDesignTokenIs(arrFields[j], "default") ) {
			pItem->iState |= XUI_MENU_ITEM_DEFAULT;
		} else if ( __uiDesignTokenIs(arrFields[j], "danger") ) {
			pItem->iState |= XUI_MENU_ITEM_DANGER;
		} else if ( __uiDesignTokenIs(arrFields[j], "disabled") ) {
			pItem->iState &= ~XUI_MENU_ITEM_ENABLED;
		} else if ( __uiDesignTokenIs(arrFields[j], "enabled") ) {
			pItem->iState |= XUI_MENU_ITEM_ENABLED;
		} else if ( !bShortcutSet ) {
			pItem->sShortcut = __uiDesignTextPoolAdd(pNode, arrFields[j]);
			bShortcutSet = 1;
		} else {
			pItem->iValue = __uiDesignParseIntText(arrFields[j], pItem->iValue);
		}
	}
}

static int __uiDesignBuildMenuItems(ui_design_node_t* pNode, xui_menu_item_t* pItems, int iCapacity)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[9];
	int iCount;
	int iFieldCount;
	int i;

	if ( (pNode == NULL) || (pItems == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.items", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount <= 0 ) continue;
		__uiDesignFillMenuItemFromFields(pNode, &pItems[iCount], arrFields, iFieldCount, 0, iCount + 1);
		++iCount;
	}
	if ( iCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultMenuItems) && iCount < iCapacity; ++i ) {
			pItems[iCount] = g_arrDefaultMenuItems[i];
			pItems[iCount].sText = __uiDesignTextPoolAdd(pNode, g_arrDefaultMenuItems[i].sText);
			pItems[iCount].sShortcut = __uiDesignTextPoolAdd(pNode, g_arrDefaultMenuItems[i].sShortcut);
			++iCount;
		}
	}
	return iCount;
}

static int __uiDesignMessageTypeFromText(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "self") || __uiDesignTokenIs(sText, "me") || __uiDesignTokenIs(sText, "you") ) return XUI_MESSAGE_NODE_SELF;
	if ( __uiDesignTokenIs(sText, "other") || __uiDesignTokenIs(sText, "user") ) return XUI_MESSAGE_NODE_OTHER;
	if ( __uiDesignTokenIs(sText, "system") || __uiDesignTokenIs(sText, "notice") ) return XUI_MESSAGE_NODE_SYSTEM;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static int __uiDesignMessageTypeFromSender(const char* sSender)
{
	if ( __uiDesignTokenIs(sSender, "you") || __uiDesignTokenIs(sSender, "self") || __uiDesignTokenIs(sSender, "me") ) return XUI_MESSAGE_NODE_SELF;
	if ( __uiDesignTokenIs(sSender, "system") ) return XUI_MESSAGE_NODE_SYSTEM;
	return XUI_MESSAGE_NODE_OTHER;
}

static int __uiDesignBuildMessageNodes(ui_design_node_t* pNode, xui_message_node_t* pNodes, int iCapacity)
{
	const char* sCursor;
	char sLine[1024];
	char sId[32];
	char* arrFields[6];
	char* sColon;
	int iCount;
	int iFieldCount;
	int i;

	if ( (pNode == NULL) || (pNodes == NULL) || (iCapacity <= 0) ) return 0;
	memset(pNodes, 0, (size_t)iCapacity * sizeof(*pNodes));
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.messages", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		memset(&pNodes[iCount], 0, sizeof(pNodes[iCount]));
		pNodes[iCount].iSize = sizeof(pNodes[iCount]);
		snprintf(sId, sizeof(sId), "m%d", iCount + 1);
		if ( strchr(sLine, '|') != NULL ) {
			iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
			pNodes[iCount].sSender = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 0, ""));
			pNodes[iCount].sTime = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 1, ""));
			pNodes[iCount].sText = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 2, ""));
			pNodes[iCount].iType = __uiDesignMessageTypeFromText(__uiDesignField(arrFields, iFieldCount, 3, ""), __uiDesignMessageTypeFromSender(pNodes[iCount].sSender));
			pNodes[iCount].sId = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 4, sId));
			pNodes[iCount].iFlags = __uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 5, ""), 0);
		} else {
			sColon = strchr(sLine, ':');
			if ( sColon != NULL ) {
				sColon[0] = 0;
				__uiDesignTrim(sLine);
				__uiDesignTrim(sColon + 1);
				pNodes[iCount].sSender = __uiDesignTextPoolAdd(pNode, sLine);
				pNodes[iCount].sText = __uiDesignTextPoolAdd(pNode, sColon + 1);
			} else {
				pNodes[iCount].sSender = __uiDesignTextPoolAdd(pNode, "Message");
				pNodes[iCount].sText = __uiDesignTextPoolAdd(pNode, sLine);
			}
			pNodes[iCount].sTime = __uiDesignTextPoolAdd(pNode, "");
			pNodes[iCount].sId = __uiDesignTextPoolAdd(pNode, sId);
			pNodes[iCount].iType = __uiDesignMessageTypeFromSender(pNodes[iCount].sSender);
			pNodes[iCount].iFlags = 0;
		}
		++iCount;
	}
	if ( iCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultMessages) && iCount < iCapacity; ++i ) {
			pNodes[iCount++] = g_arrDefaultMessages[i];
		}
	}
	return iCount;
}

static int __uiDesignApplyMessageData(ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_message_node_t arrNodes[64];
	const char* sText;
	const char* sPath;
	int iNodeCount;
	int iMode;
	int iRet;

	if ( (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iMode = __uiDesignInt(pNode, "data.sourceMode", 0);
	if ( iMode == 1 ) {
		(void)xuiMessageListClear(pWidget);
		sText = __uiDesignText(pNode, "data.importText", "");
		if ( (sText == NULL) || (sText[0] == 0) ) return XUI_OK;
		iRet = xuiMessageListImportText(pWidget, sText);
		return (iRet == XUI_OK) ? XUI_OK : iRet;
	}
	if ( iMode == 2 ) {
		(void)xuiMessageListClear(pWidget);
		sPath = __uiDesignText(pNode, "data.sourcePath", "");
		if ( (sPath == NULL) || (sPath[0] == 0) ) return XUI_OK;
		iRet = xuiMessageListLoadFile(pWidget, sPath);
		return (iRet == XUI_ERROR_FILE_NOT_FOUND) ? XUI_OK : iRet;
	}
	iNodeCount = __uiDesignBuildMessageNodes(pNode, arrNodes, UI_DESIGN_COUNT_OF(arrNodes));
	return xuiMessageListSetNodes(pWidget, arrNodes, iNodeCount);
}

static int __uiDesignInventoryFlagsFromText(const char* sText, uint32_t iDefault)
{
	char sBuffer[256];
	char* sCursor;
	char* sToken;
	uint32_t iFlags;

	if ( (sText == NULL) || (sText[0] == 0) ) return (int)iDefault;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, (int)iDefault);
	__uiDesignCopyText(sBuffer, sizeof(sBuffer), sText);
	iFlags = 0u;
	sCursor = sBuffer;
	while ( sCursor[0] != 0 ) {
		sToken = sCursor;
		while ( sCursor[0] != 0 && sCursor[0] != ',' && sCursor[0] != '+' && sCursor[0] != ' ' ) ++sCursor;
		if ( sCursor[0] != 0 ) {
			sCursor[0] = 0;
			++sCursor;
		}
		__uiDesignTrim(sToken);
		if ( sToken[0] == 0 || __uiDesignTokenIs(sToken, "none") ) continue;
		if ( __uiDesignTokenIs(sToken, "empty") ) iFlags |= XUI_INVENTORY_SLOT_EMPTY;
		else if ( __uiDesignTokenIs(sToken, "disabled") ) iFlags |= XUI_INVENTORY_SLOT_DISABLED;
		else if ( __uiDesignTokenIs(sToken, "locked") ) iFlags |= XUI_INVENTORY_SLOT_LOCKED;
		else if ( __uiDesignTokenIs(sToken, "selected") ) iFlags |= XUI_INVENTORY_SLOT_SELECTED;
		else if ( __uiDesignTokenIs(sToken, "highlight") || __uiDesignTokenIs(sToken, "highlighted") ) iFlags |= XUI_INVENTORY_SLOT_HIGHLIGHT;
		else if ( __uiDesignTokenIs(sToken, "new") ) iFlags |= XUI_INVENTORY_SLOT_NEW;
		else if ( __uiDesignTokenIs(sToken, "equipped") ) iFlags |= XUI_INVENTORY_SLOT_EQUIPPED;
		else if ( __uiDesignTokenIs(sToken, "cooldown") ) iFlags |= XUI_INVENTORY_SLOT_COOLDOWN;
		else if ( __uiDesignTokenIs(sToken, "durability") ) iFlags |= XUI_INVENTORY_SLOT_DURABILITY;
		else if ( __uiDesignTokenIs(sToken, "animation") ) iFlags |= XUI_INVENTORY_SLOT_ANIMATION;
		else if ( __uiDesignTokenIs(sToken, "radial") || __uiDesignTokenIs(sToken, "cooldown_radial") ) iFlags |= XUI_INVENTORY_SLOT_COOLDOWN_RADIAL;
	}
	return (int)iFlags;
}

static int __uiDesignBuildInventorySlots(ui_design_app_t* pApp, ui_design_node_t* pNode, xui_inventory_slot_t* pSlots, int iCapacity)
{
	xui_surface pIcon;
	xui_rect_t tIconSrc;
	const char* sCursor;
	char sLine[1024];
	char* arrFields[21];
	int iCount;
	int iFieldCount;
	int iDefaultMax;
	int i;

	if ( (pNode == NULL) || (pSlots == NULL) || (iCapacity <= 0) ) return 0;
	memset(pSlots, 0, (size_t)iCapacity * sizeof(*pSlots));
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.slots", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		memset(&pSlots[iCount], 0, sizeof(pSlots[iCount]));
		pSlots[iCount].iSize = sizeof(pSlots[iCount]);
		pSlots[iCount].iSlotId = (iFieldCount > 7) ? __uiDesignParseIntText(arrFields[7], iCount + 1) : (iCount + 1);
		pSlots[iCount].iItemId = (iFieldCount > 5) ? __uiDesignParseIntText(arrFields[5], 1000 + iCount + 1) : (1000 + iCount + 1);
		pSlots[iCount].iCount = (iFieldCount > 1) ? __uiDesignParseIntText(arrFields[1], 1) : 1;
		iDefaultMax = (pSlots[iCount].iCount > 1) ? pSlots[iCount].iCount : 1;
		pSlots[iCount].iMaxCount = (iFieldCount > 6) ? __uiDesignParseIntText(arrFields[6], iDefaultMax) : iDefaultMax;
		pSlots[iCount].iSlotType = (iFieldCount > 10) ? __uiDesignParseIntText(arrFields[10], 0) : 0;
		pSlots[iCount].iItemType = (iFieldCount > 11) ? __uiDesignParseIntText(arrFields[11], 0) : 0;
		pSlots[iCount].iFlags = (uint32_t)__uiDesignInventoryFlagsFromText(__uiDesignField(arrFields, iFieldCount, 3, ""), 0u);
		pSlots[iCount].iQualityColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 4, ""), XUI_COLOR_RGBA(49, 126, 214, 255));
		pSlots[iCount].iIconTint = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 12, ""), XUI_COLOR_WHITE);
		pSlots[iCount].fCooldownRate = (iFieldCount > 8) ? __uiDesignParseFloatText(arrFields[8], 0.0f) : 0.0f;
		pSlots[iCount].fDurabilityRate = (iFieldCount > 9) ? __uiDesignParseFloatText(arrFields[9], 1.0f) : 1.0f;
		pSlots[iCount].iAnimationFlags = (uint32_t)__uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 18, ""), 0);
		pSlots[iCount].fAnimationScale = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 19, ""), 1.0f);
		pSlots[iCount].iAnimationTint = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 20, ""), XUI_COLOR_WHITE);
		if ( (pSlots[iCount].iAnimationFlags != 0u) || (pSlots[iCount].fAnimationScale != 1.0f) || (pSlots[iCount].iAnimationTint != XUI_COLOR_WHITE) ) {
			pSlots[iCount].iFlags |= XUI_INVENTORY_SLOT_ANIMATION;
		}
		if ( (pApp != NULL) && (iCount < UI_DESIGN_RUNTIME_SURFACE_COUNT) ) {
			pIcon = NULL;
			(void)__uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, iCount, __uiDesignField(arrFields, iFieldCount, 13, ""), &pIcon);
			pSlots[iCount].pIcon = pIcon;
			memset(&tIconSrc, 0, sizeof(tIconSrc));
			if ( pIcon != NULL ) {
				tIconSrc.fX = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 14, NULL), 0.0f);
				tIconSrc.fY = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 15, NULL), 0.0f);
				tIconSrc.fW = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 16, NULL), 0.0f);
				tIconSrc.fH = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 17, NULL), 0.0f);
				if ( tIconSrc.fW <= 0.0f || tIconSrc.fH <= 0.0f ) memset(&tIconSrc, 0, sizeof(tIconSrc));
			}
			pSlots[iCount].tIconSrc = tIconSrc;
		}
		__uiDesignCopyText(pSlots[iCount].sText, sizeof(pSlots[iCount].sText), __uiDesignField(arrFields, iFieldCount, 0, ""));
		__uiDesignCopyText(pSlots[iCount].sHotkey, sizeof(pSlots[iCount].sHotkey), __uiDesignField(arrFields, iFieldCount, 2, ""));
		++iCount;
	}
	if ( pApp != NULL ) {
		for ( i = iCount; i < UI_DESIGN_RUNTIME_SURFACE_COUNT; ++i ) {
			pIcon = NULL;
			(void)__uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, i, "", &pIcon);
		}
	}
	return iCount;
}

static int __uiDesignCascaderFindChild(const xui_cascader_item_t* pItems, int iCount, int iParent, const char* sText)
{
	int i;

	if ( (pItems == NULL) || (sText == NULL) ) return -1;
	for ( i = 0; i < iCount; ++i ) {
		if ( pItems[i].iParent == iParent && __uiDesignTextEqualsNoCase(pItems[i].sText, sText) ) return i;
	}
	return -1;
}

static int __uiDesignCascaderAddItem(ui_design_node_t* pNode, xui_cascader_item_t* pItems, int* pCount, int iCapacity, const char* sText, int iParent, int iValue, uint32_t iFlags)
{
	int iIndex;

	if ( (pNode == NULL) || (pItems == NULL) || (pCount == NULL) || (*pCount >= iCapacity) || (sText == NULL) || (sText[0] == 0) ) return -1;
	iIndex = *pCount;
	memset(&pItems[iIndex], 0, sizeof(pItems[iIndex]));
	pItems[iIndex].sText = __uiDesignTextPoolAdd(pNode, sText);
	pItems[iIndex].iValue = (iValue != 0) ? iValue : (iIndex + 1);
	pItems[iIndex].iParent = iParent;
	pItems[iIndex].iFlags = iFlags;
	++(*pCount);
	return iIndex;
}

static int __uiDesignCascaderAddPath(ui_design_node_t* pNode, xui_cascader_item_t* pItems, int* pCount, int iCapacity, const char* sPath)
{
	char sBuffer[512];
	char* sCursor;
	char* sPart;
	int iParent;
	int iFound;

	if ( (pNode == NULL) || (pItems == NULL) || (pCount == NULL) || (sPath == NULL) ) return -1;
	__uiDesignCopyText(sBuffer, sizeof(sBuffer), sPath);
	sCursor = sBuffer;
	iParent = -1;
	while ( sCursor[0] != 0 ) {
		sPart = sCursor;
		while ( sCursor[0] != 0 && sCursor[0] != '/' && sCursor[0] != '\\' ) ++sCursor;
		if ( sCursor[0] != 0 ) {
			sCursor[0] = 0;
			++sCursor;
		}
		__uiDesignTrim(sPart);
		if ( sPart[0] == 0 ) continue;
		iFound = __uiDesignCascaderFindChild(pItems, *pCount, iParent, sPart);
		if ( iFound < 0 ) iFound = __uiDesignCascaderAddItem(pNode, pItems, pCount, iCapacity, sPart, iParent, 0, 0u);
		if ( iFound < 0 ) return iParent;
		iParent = iFound;
	}
	return iParent;
}

static int __uiDesignCascaderBuildValuePath(const xui_cascader_item_t* pItems, int iCount, int iIndex, int* pPath, int iCapacity)
{
	int arrReverse[32];
	int iDepth;
	int i;

	if ( (pItems == NULL) || (pPath == NULL) || (iIndex < 0) || (iIndex >= iCount) || (iCapacity <= 0) ) return 0;
	iDepth = 0;
	while ( iIndex >= 0 && iIndex < iCount && iDepth < (int)UI_DESIGN_COUNT_OF(arrReverse) ) {
		arrReverse[iDepth++] = pItems[iIndex].iValue;
		iIndex = pItems[iIndex].iParent;
	}
	if ( iDepth > iCapacity ) iDepth = iCapacity;
	for ( i = 0; i < iDepth; ++i ) {
		pPath[i] = arrReverse[iDepth - i - 1];
	}
	return iDepth;
}

static int __uiDesignBuildCascaderItems(ui_design_node_t* pNode, xui_cascader_item_t* pItems, int iCapacity, int* pSelectedPath, int* pSelectedDepth)
{
	const char* sCursor;
	const char* sSelected;
	char sLine[512];
	char* arrFields[6];
	int arrLeafExplicit[XUI_CASCADER_ITEM_CAPACITY];
	int iCount;
	int iFieldCount;
	int i;
	int j;
	int iSelectedIndex;
	uint32_t iFlags;

	if ( (pNode == NULL) || (pItems == NULL) || (pSelectedPath == NULL) || (pSelectedDepth == NULL) || (iCapacity <= 0) ) return 0;
	memset(pItems, 0, (size_t)iCapacity * sizeof(*pItems));
	memset(arrLeafExplicit, 0, sizeof(arrLeafExplicit));
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.items", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		if ( strchr(sLine, '|') != NULL ) {
			iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
			if ( arrFields[0][0] == 0 ) continue;
			iFlags = 0u;
			for ( j = 3; j < iFieldCount; ++j ) {
				if ( __uiDesignTokenIs(arrFields[j], "leaf") ) {
					iFlags |= XUI_CASCADER_ITEM_LEAF;
					arrLeafExplicit[iCount] = 1;
				} else if ( __uiDesignTokenIs(arrFields[j], "disabled") ) {
					iFlags |= XUI_CASCADER_ITEM_DISABLED;
				} else if ( j == 3 ) {
					if ( __uiDesignParseBoolText(arrFields[j], 0) ) iFlags |= XUI_CASCADER_ITEM_LEAF;
					arrLeafExplicit[iCount] = 1;
				} else if ( j == 4 && __uiDesignParseBoolText(arrFields[j], 0) ) {
					iFlags |= XUI_CASCADER_ITEM_DISABLED;
				}
			}
			(void)__uiDesignCascaderAddItem(pNode, pItems, &iCount, iCapacity, arrFields[0],
				__uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 2, ""), -1),
				__uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 1, ""), iCount + 1),
				iFlags);
		} else {
			(void)__uiDesignCascaderAddPath(pNode, pItems, &iCount, iCapacity, sLine);
		}
	}
	if ( iCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultCascaderItems) && iCount < iCapacity; ++i ) {
			pItems[iCount] = g_arrDefaultCascaderItems[i];
			pItems[iCount].sText = __uiDesignTextPoolAdd(pNode, g_arrDefaultCascaderItems[i].sText);
			arrLeafExplicit[iCount] = ((pItems[iCount].iFlags & XUI_CASCADER_ITEM_LEAF) != 0u);
			++iCount;
		}
	}
	iSelectedIndex = -1;
	sSelected = __uiDesignText(pNode, "data.selectedPath", "");
	if ( sSelected[0] != 0 ) iSelectedIndex = __uiDesignCascaderAddPath(pNode, pItems, &iCount, iCapacity, sSelected);
	for ( i = 0; i < iCount; ++i ) {
		if ( arrLeafExplicit[i] ) continue;
		for ( j = 0; j < iCount; ++j ) {
			if ( pItems[j].iParent == i ) break;
		}
		if ( j == iCount ) pItems[i].iFlags |= XUI_CASCADER_ITEM_LEAF;
	}
	if ( iSelectedIndex < 0 ) {
		for ( i = 0; i < iCount; ++i ) {
			if ( (pItems[i].iFlags & XUI_CASCADER_ITEM_LEAF) != 0u ) {
				iSelectedIndex = i;
				break;
			}
		}
	}
	*pSelectedDepth = __uiDesignCascaderBuildValuePath(pItems, iCount, iSelectedIndex, pSelectedPath, 16);
	return iCount;
}

static void __uiDesignPropertyIdFromText(char* sDst, int iCapacity, const char* sText)
{
	int i;
	int o;
	int c;

	if ( (sDst == NULL) || (iCapacity <= 0) ) return;
	o = 0;
	if ( sText == NULL ) sText = "";
	for ( i = 0; sText[i] != 0 && o < iCapacity - 1; ++i ) {
		c = (unsigned char)sText[i];
		if ( (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ) {
			sDst[o++] = (char)c;
		} else if ( c >= 'A' && c <= 'Z' ) {
			sDst[o++] = (char)(c - 'A' + 'a');
		} else if ( c == '_' || c == '-' || c == '.' ) {
			sDst[o++] = (char)c;
		} else if ( c == ' ' || c == '\t' ) {
			sDst[o++] = '_';
		}
	}
	if ( o == 0 && iCapacity > 1 ) sDst[o++] = 'x';
	sDst[o] = 0;
}

static int __uiDesignPropertyGridFlagsFromText(const char* sText, int iDefault)
{
	char sBuffer[256];
	char* sCursor;
	char* sToken;
	int iFlags;

	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	__uiDesignCopyText(sBuffer, sizeof(sBuffer), sText);
	iFlags = 0;
	sCursor = sBuffer;
	while ( sCursor[0] != 0 ) {
		sToken = sCursor;
		while ( sCursor[0] != 0 && sCursor[0] != ',' && sCursor[0] != '+' && sCursor[0] != ';' && sCursor[0] != ' ' && sCursor[0] != '\t' ) ++sCursor;
		if ( sCursor[0] != 0 ) {
			sCursor[0] = 0;
			++sCursor;
		}
		__uiDesignTrim(sToken);
		if ( sToken[0] == 0 ) continue;
		if ( __uiDesignTokenIs(sToken, "readonly") || __uiDesignTokenIs(sToken, "readOnly") || __uiDesignTokenIs(sToken, "ro") ) iFlags |= XUI_PROPERTY_FLAG_READONLY;
		else if ( __uiDesignTokenIs(sToken, "disabled") ) iFlags |= XUI_PROPERTY_FLAG_DISABLED;
		else if ( __uiDesignTokenIs(sToken, "dirty") || __uiDesignTokenIs(sToken, "modified") ) iFlags |= XUI_PROPERTY_FLAG_DIRTY;
		else if ( __uiDesignTokenIs(sToken, "invalid") || __uiDesignTokenIs(sToken, "error") ) iFlags |= XUI_PROPERTY_FLAG_INVALID;
		else if ( __uiDesignTokenIs(sToken, "hidden") ) iFlags |= XUI_PROPERTY_FLAG_HIDDEN;
		else if ( __uiDesignLooksIntText(sToken) ) iFlags |= __uiDesignParseIntText(sToken, 0);
	}
	return iFlags;
}

static int __uiDesignPropertyGridDateModeFromText(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "date") ) return XUI_DATE_PICKER_MODE_DATE;
	if ( __uiDesignTokenIs(sText, "time") ) return XUI_DATE_PICKER_MODE_TIME;
	if ( __uiDesignTokenIs(sText, "datetime") || __uiDesignTokenIs(sText, "dateTime") || __uiDesignTokenIs(sText, "date_time") ) return XUI_DATE_PICKER_MODE_DATETIME;
	if ( __uiDesignTokenIs(sText, "dateRange") || __uiDesignTokenIs(sText, "date_range") || __uiDesignTokenIs(sText, "range") ) return XUI_DATE_PICKER_MODE_DATE_RANGE;
	if ( __uiDesignTokenIs(sText, "timeRange") || __uiDesignTokenIs(sText, "time_range") ) return XUI_DATE_PICKER_MODE_TIME_RANGE;
	if ( __uiDesignTokenIs(sText, "datetimeRange") || __uiDesignTokenIs(sText, "dateTimeRange") || __uiDesignTokenIs(sText, "datetime_range") ) return XUI_DATE_PICKER_MODE_DATETIME_RANGE;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static int __uiDesignPropertyGridBuildEnumItems(ui_design_node_t* pNode,
	const char* sText,
	const char* sValues,
	const char* sEnabled,
	const char** ppOptions,
	xui_combobox_item_t* pItems,
	int* pEnabled,
	int iCapacity)
{
	char arrItems[XUI_PROPERTY_GRID_OPTION_CAPACITY][64];
	char arrValues[XUI_PROPERTY_GRID_OPTION_CAPACITY][64];
	char arrEnabled[XUI_PROPERTY_GRID_OPTION_CAPACITY][64];
	int iCount;
	int iValueCount;
	int iEnabledCount;
	int i;

	if ( (pNode == NULL) || (ppOptions == NULL) || (pItems == NULL) || (pEnabled == NULL) || (iCapacity <= 0) || (sText == NULL) || (sText[0] == 0) ) return 0;
	if ( iCapacity > XUI_PROPERTY_GRID_OPTION_CAPACITY ) iCapacity = XUI_PROPERTY_GRID_OPTION_CAPACITY;
	memset(ppOptions, 0, (size_t)iCapacity * sizeof(*ppOptions));
	memset(pItems, 0, (size_t)iCapacity * sizeof(*pItems));
	memset(pEnabled, 0, (size_t)iCapacity * sizeof(*pEnabled));
	iCount = __uiDesignSplitCommaList(sText, arrItems, iCapacity);
	iValueCount = __uiDesignSplitCommaList(sValues, arrValues, UI_DESIGN_COUNT_OF(arrValues));
	iEnabledCount = __uiDesignSplitCommaList(sEnabled, arrEnabled, UI_DESIGN_COUNT_OF(arrEnabled));
	for ( i = 0; i < iCount; ++i ) {
		ppOptions[i] = __uiDesignTextPoolAdd(pNode, arrItems[i]);
		pItems[i].sText = ppOptions[i];
		pItems[i].iValue = (i < iValueCount) ? __uiDesignParseIntText(arrValues[i], i) : i;
		pItems[i].bEnabled = (i < iEnabledCount) ? __uiDesignParseBoolText(arrEnabled[i], 1) : 1;
		pEnabled[i] = pItems[i].bEnabled;
	}
	return iCount;
}

static int __uiDesignPropertyGridBuildPalette(ui_design_node_t* pNode, const char* sText, const uint32_t** ppPalette)
{
	uint32_t* pPalette;
	char sBuffer[1024];
	char* sCursor;
	char* sToken;
	int iCount;

	if ( ppPalette != NULL ) *ppPalette = NULL;
	if ( (pNode == NULL) || (ppPalette == NULL) || (sText == NULL) || (sText[0] == 0) ) return 0;
	if ( pNode->iRuntimePropertyPaletteCount >= UI_DESIGN_RUNTIME_PROPERTY_PALETTES ) return 0;
	pPalette = pNode->arrRuntimePropertyPalettes[pNode->iRuntimePropertyPaletteCount];
	memset(pPalette, 0, sizeof(pNode->arrRuntimePropertyPalettes[pNode->iRuntimePropertyPaletteCount]));
	__uiDesignCopyText(sBuffer, sizeof(sBuffer), sText);
	iCount = 0;
	sCursor = sBuffer;
	while ( sCursor[0] != 0 && iCount < UI_DESIGN_RUNTIME_EDITOR_PALETTE ) {
		sToken = sCursor;
		while ( sCursor[0] != 0 && sCursor[0] != ',' && sCursor[0] != ';' ) ++sCursor;
		if ( sCursor[0] != 0 ) {
			sCursor[0] = 0;
			++sCursor;
		}
		__uiDesignTrim(sToken);
		if ( sToken[0] == 0 ) continue;
		pPalette[iCount++] = __uiDesignParseColorText(sToken, XUI_COLOR_RGBA(49, 126, 214, 255));
	}
	if ( iCount <= 0 ) return 0;
	*ppPalette = pPalette;
	++pNode->iRuntimePropertyPaletteCount;
	return iCount;
}

static int __uiDesignPropertyGridApplyEditorConfig(ui_design_node_t* pNode,
	int iProperty,
	int iType,
	const char* sValue,
	char** arrFields,
	int iFieldCount,
	int iOptionsField,
	int iMinField,
	int iMaxField,
	int iStepField,
	int iPrecisionField,
	int iNullableField,
	int iAlphaField,
	int iActionTextField,
	int iFileFilterField,
	int iDateModeField,
	int iShowSecondField,
	int iDateFormatField,
	int iDateMinField,
	int iDateMaxField,
	int iDefaultSpanField,
	int iRangeSeparatorField,
	int iPaletteField,
	int iEnumValuesField,
	int iEnumEnabledField,
	int iEnumSelectedField,
	int iEnumUseValueField,
	int iEnumSelectedValueField)
{
	const char* arrOptions[XUI_PROPERTY_GRID_OPTION_CAPACITY];
	xui_combobox_item_t arrEnumItemData[XUI_PROPERTY_GRID_OPTION_CAPACITY];
	int arrEnumEnabled[XUI_PROPERTY_GRID_OPTION_CAPACITY];
	const uint32_t* pPalette;
	xui_table_grid_editor_config_t tConfig;
	const char* sField;
	xtime tDate;
	int i;
	int iOptionCount;
	int iPaletteCount;
	int bHasConfig;
	int bHasMin;
	int bHasMax;
	int bHasDateConfig;
	int iDateMode;
	int iEnumValue;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) || (iProperty < 0) || (arrFields == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.iEnumSelected = -1;
	tConfig.iPrecision = -1;
	tConfig.sRangeSeparator = " - ";
	bHasConfig = 0;
	sField = __uiDesignField(arrFields, iFieldCount, iOptionsField, "");
	if ( (iType == XUI_TABLE_CELL_TYPE_ENUM) && (sField[0] != 0) ) {
		iOptionCount = __uiDesignPropertyGridBuildEnumItems(pNode, sField,
			__uiDesignField(arrFields, iFieldCount, iEnumValuesField, ""),
			__uiDesignField(arrFields, iFieldCount, iEnumEnabledField, ""),
			arrOptions, arrEnumItemData, arrEnumEnabled, UI_DESIGN_COUNT_OF(arrOptions));
		if ( iOptionCount > 0 ) {
			tConfig.arrEnumItems = arrOptions;
			tConfig.arrEnumItemData = arrEnumItemData;
			tConfig.arrEnumEnabled = arrEnumEnabled;
			tConfig.iEnumItemCount = iOptionCount;
			for ( i = 0; i < iOptionCount; ++i ) {
				if ( __uiDesignTextEqualsNoCase(arrOptions[i], sValue) ) {
					tConfig.iEnumSelected = i;
					break;
				}
			}
			sField = __uiDesignField(arrFields, iFieldCount, iEnumUseValueField, "");
			if ( sField[0] != 0 ) tConfig.bEnumUseValue = __uiDesignParseBoolText(sField, 0);
			if ( tConfig.bEnumUseValue && __uiDesignLooksIntText(sValue) ) {
				iEnumValue = __uiDesignParseIntText(sValue, 0);
				for ( i = 0; i < iOptionCount; ++i ) {
					if ( arrEnumItemData[i].iValue == iEnumValue ) {
						tConfig.iEnumSelected = i;
						break;
					}
				}
			}
			sField = __uiDesignField(arrFields, iFieldCount, iEnumSelectedField, "");
			if ( sField[0] != 0 ) tConfig.iEnumSelected = __uiDesignParseIntText(sField, tConfig.iEnumSelected);
			if ( tConfig.iEnumSelected < 0 ) tConfig.iEnumSelected = 0;
			sField = __uiDesignField(arrFields, iFieldCount, iEnumSelectedValueField, "");
			if ( sField[0] != 0 ) {
				tConfig.iEnumSelectedValue = __uiDesignParseIntText(sField, 0);
			} else if ( tConfig.iEnumSelected >= 0 && tConfig.iEnumSelected < iOptionCount ) {
				tConfig.iEnumSelectedValue = arrEnumItemData[tConfig.iEnumSelected].iValue;
			}
			bHasConfig = 1;
		}
	}
	if ( (iType == XUI_TABLE_CELL_TYPE_INT) || (iType == XUI_TABLE_CELL_TYPE_FLOAT) ) {
		bHasMin = 0;
		bHasMax = 0;
		sField = __uiDesignField(arrFields, iFieldCount, iMinField, "");
		if ( sField[0] != 0 ) {
			tConfig.fMin = __uiDesignParseFloatText(sField, tConfig.fMin);
			bHasMin = 1;
			bHasConfig = 1;
		}
		sField = __uiDesignField(arrFields, iFieldCount, iMaxField, "");
		if ( sField[0] != 0 ) {
			tConfig.fMax = __uiDesignParseFloatText(sField, tConfig.fMax);
			bHasMax = 1;
			bHasConfig = 1;
		}
		if ( bHasMin && !bHasMax ) tConfig.fMax = 1000000000.0f;
		if ( bHasMax && !bHasMin ) tConfig.fMin = -1000000000.0f;
		sField = __uiDesignField(arrFields, iFieldCount, iStepField, "");
		if ( sField[0] != 0 ) {
			tConfig.fStep = __uiDesignParseFloatText(sField, 0.0f);
			bHasConfig = 1;
		}
		sField = __uiDesignField(arrFields, iFieldCount, iPrecisionField, "");
		if ( sField[0] != 0 ) {
			tConfig.iPrecision = __uiDesignParseIntText(sField, (iType == XUI_TABLE_CELL_TYPE_INT) ? 0 : 3);
			bHasConfig = 1;
		}
	}
	if ( iType == XUI_TABLE_CELL_TYPE_COLOR ) {
		sField = __uiDesignField(arrFields, iFieldCount, iAlphaField, "");
		if ( sField[0] != 0 ) {
			tConfig.bAlphaEnabled = __uiDesignParseBoolText(sField, 1);
			bHasConfig = 1;
		}
		sField = __uiDesignField(arrFields, iFieldCount, iPaletteField, "");
		iPaletteCount = __uiDesignPropertyGridBuildPalette(pNode, sField, &pPalette);
		if ( iPaletteCount > 0 ) {
			tConfig.arrPalette = pPalette;
			tConfig.iPaletteCount = iPaletteCount;
			bHasConfig = 1;
		}
	}
	if ( iType == XUI_TABLE_CELL_TYPE_FILE || iType == XUI_TABLE_CELL_TYPE_IMAGE || iType == XUI_TABLE_CELL_TYPE_PICKER || iType == XUI_TABLE_CELL_TYPE_CUSTOM ) {
		sField = __uiDesignField(arrFields, iFieldCount, iActionTextField, "");
		if ( sField[0] != 0 ) {
			tConfig.sActionText = __uiDesignTextPoolAdd(pNode, sField);
			bHasConfig = 1;
		}
		sField = __uiDesignField(arrFields, iFieldCount, iFileFilterField, "");
		if ( sField[0] != 0 ) {
			tConfig.sFileFilter = __uiDesignTextPoolAdd(pNode, sField);
			bHasConfig = 1;
		}
	}
	if ( iType == XUI_TABLE_CELL_TYPE_DATE || iType == XUI_TABLE_CELL_TYPE_TIME || iType == XUI_TABLE_CELL_TYPE_DATETIME ) {
		bHasDateConfig = (__uiDesignField(arrFields, iFieldCount, iDateModeField, "")[0] != 0) ||
			(__uiDesignField(arrFields, iFieldCount, iShowSecondField, "")[0] != 0) ||
			(__uiDesignField(arrFields, iFieldCount, iNullableField, "")[0] != 0) ||
			(__uiDesignField(arrFields, iFieldCount, iDateFormatField, "")[0] != 0) ||
			(__uiDesignField(arrFields, iFieldCount, iDateMinField, "")[0] != 0) ||
			(__uiDesignField(arrFields, iFieldCount, iDateMaxField, "")[0] != 0) ||
			(__uiDesignField(arrFields, iFieldCount, iDefaultSpanField, "")[0] != 0) ||
			(__uiDesignField(arrFields, iFieldCount, iRangeSeparatorField, "")[0] != 0);
		if ( !bHasDateConfig ) {
			if ( bHasConfig ) return xuiPropertyGridSetEditorConfig(pNode->pWidget, iProperty, &tConfig);
			return XUI_OK;
		}
		iDateMode = __uiDesignPropertyGridDateModeFromText(__uiDesignField(arrFields, iFieldCount, iDateModeField, ""), (iType == XUI_TABLE_CELL_TYPE_TIME) ? XUI_DATE_PICKER_MODE_TIME : ((iType == XUI_TABLE_CELL_TYPE_DATETIME) ? XUI_DATE_PICKER_MODE_DATETIME : XUI_DATE_PICKER_MODE_DATE));
		tConfig.iDateMode = iDateMode;
		tConfig.bDateModeSet = (__uiDesignField(arrFields, iFieldCount, iDateModeField, "")[0] != 0);
		tConfig.bShowSecond = __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, iShowSecondField, ""), 1);
		sField = __uiDesignField(arrFields, iFieldCount, iNullableField, "");
		tConfig.bNullable = (sField[0] != 0) ? __uiDesignParseBoolText(sField, 1) : 1;
		sField = __uiDesignField(arrFields, iFieldCount, iDateFormatField, "");
		if ( sField[0] != 0 ) tConfig.sDateFormat = __uiDesignTextPoolAdd(pNode, sField);
		sField = __uiDesignField(arrFields, iFieldCount, iDateMinField, "");
		if ( sField[0] != 0 && __uiDesignParseDatePickerValue(sField, iDateMode, &tDate) ) {
			tConfig.bDateHasMin = 1;
			tConfig.tDateMin = tDate;
		}
		sField = __uiDesignField(arrFields, iFieldCount, iDateMaxField, "");
		if ( sField[0] != 0 && __uiDesignParseDatePickerValue(sField, iDateMode, &tDate) ) {
			tConfig.bDateHasMax = 1;
			tConfig.tDateMax = tDate;
		}
		sField = __uiDesignField(arrFields, iFieldCount, iDefaultSpanField, "");
		if ( sField[0] != 0 ) tConfig.tDefaultRangeSpan = __uiDesignParseDatePickerSpan(sField, tConfig.tDefaultRangeSpan);
		sField = __uiDesignField(arrFields, iFieldCount, iRangeSeparatorField, "");
		tConfig.sRangeSeparator = (sField[0] != 0) ? __uiDesignTextPoolAdd(pNode, sField) : " - ";
		bHasConfig = 1;
	}
	if ( bHasConfig ) return xuiPropertyGridSetEditorConfig(pNode->pWidget, iProperty, &tConfig);
	return XUI_OK;
}

static int __uiDesignPropertyGridEnsureCategory(xui_widget pWidget, const char* sName, int bExpanded)
{
	char sId[64];
	int iCategory;

	if ( pWidget == NULL ) return -1;
	__uiDesignPropertyIdFromText(sId, sizeof(sId), sName);
	iCategory = xuiPropertyGridFindCategory(pWidget, sId);
	if ( iCategory >= 0 ) {
		(void)xuiPropertyGridSetCategoryExpanded(pWidget, iCategory, bExpanded);
		return iCategory;
	}
	return xuiPropertyGridAddCategory(pWidget, sId, (sName != NULL && sName[0] != 0) ? sName : sId, bExpanded);
}

static int __uiDesignApplyPropertyGridData(ui_design_node_t* pNode)
{
	const char* sCursor;
	char sLine[1024];
	char sPath[256];
	char sCategory[96];
	char sName[96];
	char sId[128];
	char sRawId[256];
	char* arrFields[32];
	char* arrCategoryFields[2];
	char* sEqual;
	char* sDot;
	const char* sValue;
	xui_property_desc_t tProp;
	int iFieldCount;
	int iCategoryFieldCount;
	int iCategory;
	int iProperty;
	int iPropertyCount;
	int bEqualForm;
	int iTypeField;
	int iDescriptionField;
	int iDefaultField;
	int iFlagsField;
	int iIdField;
	int iExpandedField;
	int iOptionsField;
	int iMinField;
	int iMaxField;
	int iStepField;
	int iPrecisionField;
	int iNullableField;
	int iAlphaField;
	int iActionTextField;
	int iFileFilterField;
	int iDateModeField;
	int iShowSecondField;
	int iDateFormatField;
	int iDateMinField;
	int iDateMaxField;
	int iDefaultSpanField;
	int iRangeSeparatorField;
	int iPaletteField;
	int iEnumValuesField;
	int iEnumEnabledField;
	int iEnumSelectedField;
	int iEnumUseValueField;
	int iEnumSelectedValueField;
	int bCategoryExpanded;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iPropertyCount = 0;
	sCursor = __uiDesignText(pNode, "data.properties", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		if ( sLine[0] == '[' ) {
			sEqual = strchr(sLine, ']');
			if ( sEqual != NULL ) sEqual[0] = 0;
			iCategoryFieldCount = __uiDesignSplitFields(sLine + 1, arrCategoryFields, UI_DESIGN_COUNT_OF(arrCategoryFields));
			if ( iCategoryFieldCount > 0 && arrCategoryFields[0][0] != 0 ) {
				bCategoryExpanded = (iCategoryFieldCount > 1) ? __uiDesignParseBoolText(arrCategoryFields[1], 1) : 1;
				(void)__uiDesignPropertyGridEnsureCategory(pNode->pWidget, arrCategoryFields[0], bCategoryExpanded);
			}
			continue;
		}
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount <= 0 || arrFields[0][0] == 0 ) continue;
		__uiDesignCopyText(sPath, sizeof(sPath), arrFields[0]);
		sValue = (iFieldCount > 2) ? arrFields[2] : "";
		sEqual = strchr(sPath, '=');
		bEqualForm = 0;
		if ( sEqual != NULL ) {
			sEqual[0] = 0;
			sValue = sEqual + 1;
			bEqualForm = 1;
		}
		if ( (sEqual == NULL) && (iFieldCount >= 3) ) {
			__uiDesignCopyText(sCategory, sizeof(sCategory), arrFields[0]);
			__uiDesignCopyText(sName, sizeof(sName), arrFields[1]);
			sValue = arrFields[2];
		} else {
			sDot = strrchr(sPath, '.');
			if ( sDot != NULL ) {
				sDot[0] = 0;
				__uiDesignCopyText(sCategory, sizeof(sCategory), sPath);
				__uiDesignCopyText(sName, sizeof(sName), sDot + 1);
			} else {
				__uiDesignCopyText(sCategory, sizeof(sCategory), "General");
				__uiDesignCopyText(sName, sizeof(sName), sPath);
			}
		}
		__uiDesignTrim(sCategory);
		__uiDesignTrim(sName);
		if ( sName[0] == 0 ) continue;
		memset(&tProp, 0, sizeof(tProp));
		snprintf(sRawId, sizeof(sRawId), "%s.%s", sCategory, sName);
		iTypeField = bEqualForm ? 1 : 3;
		iDescriptionField = bEqualForm ? 2 : 4;
		iDefaultField = bEqualForm ? 3 : 5;
		iFlagsField = bEqualForm ? 4 : 6;
		iIdField = bEqualForm ? 5 : 7;
		iExpandedField = bEqualForm ? 6 : 8;
		iOptionsField = bEqualForm ? 7 : 9;
		iMinField = bEqualForm ? 8 : 10;
		iMaxField = bEqualForm ? 9 : 11;
		iStepField = bEqualForm ? 10 : 12;
		iPrecisionField = bEqualForm ? 11 : 13;
		iNullableField = bEqualForm ? 12 : 14;
		iAlphaField = bEqualForm ? 13 : 15;
		iActionTextField = bEqualForm ? 14 : 16;
		iFileFilterField = bEqualForm ? 15 : 17;
		iDateModeField = bEqualForm ? 16 : 18;
		iShowSecondField = bEqualForm ? 17 : 19;
		iDateFormatField = bEqualForm ? 18 : 20;
		iDateMinField = bEqualForm ? 19 : 21;
		iDateMaxField = bEqualForm ? 20 : 22;
		iDefaultSpanField = bEqualForm ? 21 : 23;
		iRangeSeparatorField = bEqualForm ? 22 : 24;
		iPaletteField = bEqualForm ? 23 : 25;
		iEnumValuesField = bEqualForm ? 24 : 26;
		iEnumEnabledField = bEqualForm ? 25 : 27;
		iEnumSelectedField = bEqualForm ? 26 : 28;
		iEnumUseValueField = bEqualForm ? 27 : 29;
		iEnumSelectedValueField = bEqualForm ? 28 : 30;
		bCategoryExpanded = (iFieldCount > iExpandedField) ? __uiDesignParseBoolText(arrFields[iExpandedField], 1) : 1;
		iCategory = __uiDesignPropertyGridEnsureCategory(pNode->pWidget, sCategory, bCategoryExpanded);
		if ( iCategory < 0 ) continue;
		__uiDesignPropertyIdFromText(sId, sizeof(sId), (iFieldCount > iIdField) ? arrFields[iIdField] : sRawId);
		tProp.sId = sId;
		tProp.sName = sName;
		tProp.sDescription = (iFieldCount > iDescriptionField) ? arrFields[iDescriptionField] : "";
		tProp.iType = __uiDesignTableCellTypeFromText((iFieldCount > iTypeField) ? arrFields[iTypeField] : "", XUI_TABLE_CELL_TYPE_TEXT);
		tProp.sValue = sValue;
		tProp.sDefaultValue = (iFieldCount > iDefaultField) ? arrFields[iDefaultField] : "";
		tProp.iFlags = (iFieldCount > iFlagsField) ? __uiDesignPropertyGridFlagsFromText(arrFields[iFlagsField], 0) : 0;
		iProperty = xuiPropertyGridAddProperty(pNode->pWidget, iCategory, &tProp);
		if ( iProperty >= 0 ) {
			(void)__uiDesignPropertyGridApplyEditorConfig(pNode, iProperty, tProp.iType, tProp.sValue, arrFields, iFieldCount,
				iOptionsField, iMinField, iMaxField, iStepField, iPrecisionField, iNullableField, iAlphaField,
				iActionTextField, iFileFilterField, iDateModeField, iShowSecondField, iDateFormatField,
				iDateMinField, iDateMaxField, iDefaultSpanField, iRangeSeparatorField, iPaletteField,
				iEnumValuesField, iEnumEnabledField, iEnumSelectedField, iEnumUseValueField, iEnumSelectedValueField);
			++iPropertyCount;
		}
	}
	if ( iPropertyCount == 0 ) {
		xui_property_desc_t tDefault;
		int iGeneral;

		iGeneral = __uiDesignPropertyGridEnsureCategory(pNode->pWidget, "General", 1);
		memset(&tDefault, 0, sizeof(tDefault));
		tDefault.sId = "name";
		tDefault.sName = "Name";
		tDefault.sDescription = "Control name";
		tDefault.iType = XUI_TABLE_CELL_TYPE_TEXT;
		tDefault.sValue = "Button1";
		(void)xuiPropertyGridAddProperty(pNode->pWidget, iGeneral, &tDefault);
		tDefault.sId = "enabled";
		tDefault.sName = "Enabled";
		tDefault.iType = XUI_TABLE_CELL_TYPE_BOOL;
		tDefault.sValue = "true";
		(void)xuiPropertyGridAddProperty(pNode->pWidget, iGeneral, &tDefault);
	}
	return XUI_OK;
}

static int __uiDesignSplitCommandFields(char* sLine, char** arrFields, int iCapacity)
{
	char* sCursor;
	int iCount;

	if ( (sLine == NULL) || (arrFields == NULL) || (iCapacity <= 0) ) return 0;
	if ( strchr(sLine, '|') != NULL ) return __uiDesignSplitFields(sLine, arrFields, iCapacity);
	iCount = 0;
	sCursor = sLine;
	while ( sCursor[0] != 0 && iCount < iCapacity ) {
		while ( sCursor[0] == ' ' || sCursor[0] == '\t' ) ++sCursor;
		if ( sCursor[0] == 0 ) break;
		arrFields[iCount++] = sCursor;
		while ( sCursor[0] != 0 && sCursor[0] != ' ' && sCursor[0] != '\t' ) ++sCursor;
		if ( sCursor[0] != 0 ) {
			sCursor[0] = 0;
			++sCursor;
		}
	}
	return iCount;
}

static int __uiDesignApplyCanvasCommands(ui_design_app_t* pApp, ui_design_node_t* pNode, uint32_t iDefaultPen, uint32_t iDefaultBorder)
{
	const char* sCursor;
	const char* sType;
	const char* sSource;
	const char* sColor;
	const char* sFlags;
	const char* sText;
	char sLine[1024];
	char* arrFields[24];
	xui_rect_t tRect;
	xui_rect_t tSrc;
	xui_vec2_t tA;
	xui_vec2_t tB;
	xui_vec2_t tC;
	xui_surface pSurface;
	xui_font pFont;
	int iFieldCount;
	int iCommandCount;
	int iSurfaceSlot;
	int bStructured;
	uint32_t iColor;
	uint32_t iFlags;
	float fX;
	float fY;
	float fW;
	float fH;
	float fX2;
	float fY2;
	float fX3;
	float fY3;
	float fLineWidth;
	float fRadius;
	float fSize;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iCommandCount = 0;
	iSurfaceSlot = 0;
	pFont = xuiGetDefaultFont(xuiWidgetGetContext(pNode->pWidget));
	sCursor = __uiDesignText(pNode, "data.commands", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		bStructured = (strchr(sLine, '|') != NULL);
		iFieldCount = __uiDesignSplitCommandFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount <= 0 ) continue;
		sType = arrFields[0];
		if ( bStructured ) {
			sSource = __uiDesignField(arrFields, iFieldCount, 1, "");
			fX = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 2, ""), 0.0f);
			fY = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 3, ""), 0.0f);
			fW = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 4, ""), 0.0f);
			fH = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 5, ""), 0.0f);
			fX2 = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 6, ""), 0.0f);
			fY2 = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 7, ""), 0.0f);
			fX3 = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 8, ""), 0.0f);
			fY3 = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 9, ""), 0.0f);
			tSrc = (xui_rect_t){
				__uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 10, ""), 0.0f),
				__uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 11, ""), 0.0f),
				__uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 12, ""), 0.0f),
				__uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 13, ""), 0.0f)
			};
			fRadius = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 14, ""), 0.0f);
			fSize = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 15, ""), 1.0f);
			fLineWidth = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 16, ""), __uiDesignFloat(pNode, "metrics.penWidth", 2.0f));
			sText = __uiDesignField(arrFields, iFieldCount, 17, "");
			sColor = __uiDesignField(arrFields, iFieldCount, 18, "");
			sFlags = __uiDesignField(arrFields, iFieldCount, 19, "");
			if ( __uiDesignTokenIs(sType, "clear") ) {
				(void)xuiCanvasClear(pNode->pWidget, __uiDesignParseColorText(sColor, XUI_COLOR_WHITE));
				++iCommandCount;
			} else if ( __uiDesignTokenIs(sType, "clearRect") || __uiDesignTokenIs(sType, "clearrect") ) {
				tRect = (xui_rect_t){fX, fY, fW, fH};
				(void)xuiCanvasClearRect(pNode->pWidget, tRect, __uiDesignParseColorText(sColor, XUI_COLOR_WHITE));
				++iCommandCount;
			} else if ( __uiDesignTokenIs(sType, "surface") || __uiDesignTokenIs(sType, "image") ) {
				pSurface = NULL;
				if ( (pApp != NULL) && (sSource[0] != 0) && (iSurfaceSlot < UI_DESIGN_RUNTIME_SURFACE_COUNT) ) {
					(void)__uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, iSurfaceSlot, sSource, &pSurface);
					++iSurfaceSlot;
				}
				if ( pSurface != NULL ) {
					tRect = (xui_rect_t){fX, fY, fW, fH};
					iColor = __uiDesignParseColorText(sColor, XUI_COLOR_WHITE);
					iFlags = __uiDesignParseIntText(sFlags, 0);
					(void)xuiCanvasDrawSurface(pNode->pWidget, pSurface, tSrc, tRect, iColor, iFlags);
					++iCommandCount;
				}
			} else if ( __uiDesignTokenIs(sType, "point") ) {
				(void)xuiCanvasDrawPoint(pNode->pWidget, fX, fY, fSize, __uiDesignParseColorText(sColor, iDefaultPen));
				++iCommandCount;
			} else if ( __uiDesignTokenIs(sType, "line") ) {
				(void)xuiCanvasDrawLine(pNode->pWidget, fX, fY, fX2, fY2, fLineWidth, __uiDesignParseColorText(sColor, iDefaultPen));
				++iCommandCount;
			} else if ( __uiDesignTokenIs(sType, "rect") || __uiDesignTokenIs(sType, "rectStroke") || __uiDesignTokenIs(sType, "rectstroke") ) {
				tRect = (xui_rect_t){fX, fY, fW, fH};
				(void)xuiCanvasDrawRectStroke(pNode->pWidget, tRect, fLineWidth, __uiDesignParseColorText(sColor, iDefaultBorder));
				++iCommandCount;
			} else if ( __uiDesignTokenIs(sType, "rectFill") || __uiDesignTokenIs(sType, "rectfill") || __uiDesignTokenIs(sType, "fillRect") || __uiDesignTokenIs(sType, "fillrect") ) {
				tRect = (xui_rect_t){fX, fY, fW, fH};
				(void)xuiCanvasDrawRectFill(pNode->pWidget, tRect, __uiDesignParseColorText(sColor, iDefaultPen));
				++iCommandCount;
			} else if ( __uiDesignTokenIs(sType, "circle") || __uiDesignTokenIs(sType, "circleStroke") || __uiDesignTokenIs(sType, "circlestroke") ) {
				(void)xuiCanvasDrawCircleStroke(pNode->pWidget, fX, fY, fRadius, fLineWidth, __uiDesignParseColorText(sColor, iDefaultBorder));
				++iCommandCount;
			} else if ( __uiDesignTokenIs(sType, "circleFill") || __uiDesignTokenIs(sType, "circlefill") || __uiDesignTokenIs(sType, "fillCircle") || __uiDesignTokenIs(sType, "fillcircle") ) {
				(void)xuiCanvasDrawCircleFill(pNode->pWidget, fX, fY, fRadius, __uiDesignParseColorText(sColor, iDefaultPen));
				++iCommandCount;
			} else if ( __uiDesignTokenIs(sType, "triangleFill") || __uiDesignTokenIs(sType, "trianglefill") || __uiDesignTokenIs(sType, "fillTriangle") || __uiDesignTokenIs(sType, "filltriangle") ) {
				tA = (xui_vec2_t){fX, fY};
				tB = (xui_vec2_t){fX2, fY2};
				tC = (xui_vec2_t){fX3, fY3};
				(void)xuiCanvasDrawTriangleFill(pNode->pWidget, tA, tB, tC, __uiDesignParseColorText(sColor, iDefaultPen));
				++iCommandCount;
			} else if ( __uiDesignTokenIs(sType, "triangle") || __uiDesignTokenIs(sType, "triangleStroke") || __uiDesignTokenIs(sType, "trianglestroke") ) {
				tA = (xui_vec2_t){fX, fY};
				tB = (xui_vec2_t){fX2, fY2};
				tC = (xui_vec2_t){fX3, fY3};
				(void)xuiCanvasDrawTriangleStroke(pNode->pWidget, tA, tB, tC, fLineWidth, __uiDesignParseColorText(sColor, iDefaultBorder));
				++iCommandCount;
			} else if ( __uiDesignTokenIs(sType, "text") && pFont != NULL ) {
				tRect = (xui_rect_t){fX, fY, fW, fH};
				iFlags = (sFlags[0] != 0) ? __uiDesignParseIntText(sFlags, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) : (XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
				(void)xuiCanvasDrawText(pNode->pWidget, pFont, sText, tRect, __uiDesignParseColorText(sColor, iDefaultPen), iFlags);
				++iCommandCount;
			}
			continue;
		}
		if ( __uiDesignTokenIs(arrFields[0], "clear") ) {
			(void)xuiCanvasClear(pNode->pWidget, (iFieldCount > 1) ? __uiDesignParseColorText(arrFields[1], XUI_COLOR_WHITE) : XUI_COLOR_WHITE);
			++iCommandCount;
		} else if ( (__uiDesignTokenIs(arrFields[0], "clearrect") || __uiDesignTokenIs(arrFields[0], "clearRect")) && iFieldCount >= 5 ) {
			tRect = (xui_rect_t){
				__uiDesignParseFloatText(arrFields[1], 0.0f),
				__uiDesignParseFloatText(arrFields[2], 0.0f),
				__uiDesignParseFloatText(arrFields[3], 0.0f),
				__uiDesignParseFloatText(arrFields[4], 0.0f)
			};
			(void)xuiCanvasClearRect(pNode->pWidget, tRect, (iFieldCount > 5) ? __uiDesignParseColorText(arrFields[5], XUI_COLOR_WHITE) : XUI_COLOR_WHITE);
			++iCommandCount;
		} else if ( (__uiDesignTokenIs(arrFields[0], "surface") || __uiDesignTokenIs(arrFields[0], "image")) && iFieldCount >= 6 ) {
			pSurface = NULL;
			if ( (pApp != NULL) && (iSurfaceSlot < UI_DESIGN_RUNTIME_SURFACE_COUNT) ) {
				(void)__uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, iSurfaceSlot, arrFields[1], &pSurface);
				++iSurfaceSlot;
			}
			if ( pSurface != NULL ) {
				tRect = (xui_rect_t){
					__uiDesignParseFloatText(arrFields[2], 0.0f),
					__uiDesignParseFloatText(arrFields[3], 0.0f),
					__uiDesignParseFloatText(arrFields[4], 0.0f),
					__uiDesignParseFloatText(arrFields[5], 0.0f)
				};
				tSrc = (xui_rect_t){
					(iFieldCount > 6) ? __uiDesignParseFloatText(arrFields[6], 0.0f) : 0.0f,
					(iFieldCount > 7) ? __uiDesignParseFloatText(arrFields[7], 0.0f) : 0.0f,
					(iFieldCount > 8) ? __uiDesignParseFloatText(arrFields[8], 0.0f) : 0.0f,
					(iFieldCount > 9) ? __uiDesignParseFloatText(arrFields[9], 0.0f) : 0.0f
				};
				(void)xuiCanvasDrawSurface(pNode->pWidget, pSurface, tSrc, tRect,
					(iFieldCount > 10) ? __uiDesignParseColorText(arrFields[10], XUI_COLOR_WHITE) : XUI_COLOR_WHITE,
					(iFieldCount > 11) ? (uint32_t)__uiDesignParseIntText(arrFields[11], 0) : 0u);
				++iCommandCount;
			}
		} else if ( __uiDesignTokenIs(arrFields[0], "point") && iFieldCount >= 3 ) {
			(void)xuiCanvasDrawPoint(pNode->pWidget,
				__uiDesignParseFloatText(arrFields[1], 0.0f),
				__uiDesignParseFloatText(arrFields[2], 0.0f),
				(iFieldCount > 3) ? __uiDesignParseFloatText(arrFields[3], 1.0f) : 1.0f,
				(iFieldCount > 4) ? __uiDesignParseColorText(arrFields[4], iDefaultPen) : iDefaultPen);
			++iCommandCount;
		} else if ( __uiDesignTokenIs(arrFields[0], "line") && iFieldCount >= 5 ) {
			(void)xuiCanvasDrawLine(pNode->pWidget,
				__uiDesignParseFloatText(arrFields[1], 0.0f),
				__uiDesignParseFloatText(arrFields[2], 0.0f),
				__uiDesignParseFloatText(arrFields[3], 0.0f),
				__uiDesignParseFloatText(arrFields[4], 0.0f),
				(iFieldCount > 5) ? __uiDesignParseFloatText(arrFields[5], __uiDesignFloat(pNode, "metrics.penWidth", 2.0f)) : __uiDesignFloat(pNode, "metrics.penWidth", 2.0f),
				(iFieldCount > 6) ? __uiDesignParseColorText(arrFields[6], iDefaultPen) : iDefaultPen);
			++iCommandCount;
		} else if ( (__uiDesignTokenIs(arrFields[0], "rect") || __uiDesignTokenIs(arrFields[0], "rectstroke")) && iFieldCount >= 5 ) {
			tRect = (xui_rect_t){
				__uiDesignParseFloatText(arrFields[1], 0.0f),
				__uiDesignParseFloatText(arrFields[2], 0.0f),
				__uiDesignParseFloatText(arrFields[3], 0.0f),
				__uiDesignParseFloatText(arrFields[4], 0.0f)
			};
			(void)xuiCanvasDrawRectStroke(pNode->pWidget, tRect,
				(iFieldCount > 5) ? __uiDesignParseFloatText(arrFields[5], 1.0f) : 1.0f,
				(iFieldCount > 6) ? __uiDesignParseColorText(arrFields[6], iDefaultBorder) : iDefaultBorder);
			++iCommandCount;
		} else if ( (__uiDesignTokenIs(arrFields[0], "rectfill") || __uiDesignTokenIs(arrFields[0], "fillrect")) && iFieldCount >= 5 ) {
			tRect = (xui_rect_t){
				__uiDesignParseFloatText(arrFields[1], 0.0f),
				__uiDesignParseFloatText(arrFields[2], 0.0f),
				__uiDesignParseFloatText(arrFields[3], 0.0f),
				__uiDesignParseFloatText(arrFields[4], 0.0f)
			};
			(void)xuiCanvasDrawRectFill(pNode->pWidget, tRect, (iFieldCount > 5) ? __uiDesignParseColorText(arrFields[5], iDefaultPen) : iDefaultPen);
			++iCommandCount;
		} else if ( (__uiDesignTokenIs(arrFields[0], "circle") || __uiDesignTokenIs(arrFields[0], "circlestroke")) && iFieldCount >= 4 ) {
			(void)xuiCanvasDrawCircleStroke(pNode->pWidget,
				__uiDesignParseFloatText(arrFields[1], 0.0f),
				__uiDesignParseFloatText(arrFields[2], 0.0f),
				__uiDesignParseFloatText(arrFields[3], 0.0f),
				(iFieldCount > 4) ? __uiDesignParseFloatText(arrFields[4], 1.0f) : 1.0f,
				(iFieldCount > 5) ? __uiDesignParseColorText(arrFields[5], iDefaultBorder) : iDefaultBorder);
			++iCommandCount;
		} else if ( (__uiDesignTokenIs(arrFields[0], "circlefill") || __uiDesignTokenIs(arrFields[0], "fillcircle")) && iFieldCount >= 4 ) {
			(void)xuiCanvasDrawCircleFill(pNode->pWidget,
				__uiDesignParseFloatText(arrFields[1], 0.0f),
				__uiDesignParseFloatText(arrFields[2], 0.0f),
				__uiDesignParseFloatText(arrFields[3], 0.0f),
				(iFieldCount > 4) ? __uiDesignParseColorText(arrFields[4], iDefaultPen) : iDefaultPen);
			++iCommandCount;
		} else if ( (__uiDesignTokenIs(arrFields[0], "trianglefill") || __uiDesignTokenIs(arrFields[0], "filltriangle")) && iFieldCount >= 7 ) {
			tA = (xui_vec2_t){__uiDesignParseFloatText(arrFields[1], 0.0f), __uiDesignParseFloatText(arrFields[2], 0.0f)};
			tB = (xui_vec2_t){__uiDesignParseFloatText(arrFields[3], 0.0f), __uiDesignParseFloatText(arrFields[4], 0.0f)};
			tC = (xui_vec2_t){__uiDesignParseFloatText(arrFields[5], 0.0f), __uiDesignParseFloatText(arrFields[6], 0.0f)};
			(void)xuiCanvasDrawTriangleFill(pNode->pWidget, tA, tB, tC, (iFieldCount > 7) ? __uiDesignParseColorText(arrFields[7], iDefaultPen) : iDefaultPen);
			++iCommandCount;
		} else if ( (__uiDesignTokenIs(arrFields[0], "triangle") || __uiDesignTokenIs(arrFields[0], "trianglestroke")) && iFieldCount >= 7 ) {
			tA = (xui_vec2_t){__uiDesignParseFloatText(arrFields[1], 0.0f), __uiDesignParseFloatText(arrFields[2], 0.0f)};
			tB = (xui_vec2_t){__uiDesignParseFloatText(arrFields[3], 0.0f), __uiDesignParseFloatText(arrFields[4], 0.0f)};
			tC = (xui_vec2_t){__uiDesignParseFloatText(arrFields[5], 0.0f), __uiDesignParseFloatText(arrFields[6], 0.0f)};
			(void)xuiCanvasDrawTriangleStroke(pNode->pWidget, tA, tB, tC,
				(iFieldCount > 7) ? __uiDesignParseFloatText(arrFields[7], 1.0f) : 1.0f,
				(iFieldCount > 8) ? __uiDesignParseColorText(arrFields[8], iDefaultBorder) : iDefaultBorder);
			++iCommandCount;
		} else if ( __uiDesignTokenIs(arrFields[0], "text") && iFieldCount >= 6 && pFont != NULL ) {
			tRect = (xui_rect_t){
				__uiDesignParseFloatText(arrFields[1], 0.0f),
				__uiDesignParseFloatText(arrFields[2], 0.0f),
				__uiDesignParseFloatText(arrFields[3], 80.0f),
				__uiDesignParseFloatText(arrFields[4], 24.0f)
			};
			(void)xuiCanvasDrawText(pNode->pWidget, pFont, arrFields[5], tRect,
				(iFieldCount > 6) ? __uiDesignParseColorText(arrFields[6], iDefaultPen) : iDefaultPen,
				(iFieldCount > 7) ? __uiDesignParseIntText(arrFields[7], XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) : (XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP));
			++iCommandCount;
		}
	}
	if ( pApp != NULL ) {
		while ( iSurfaceSlot < UI_DESIGN_RUNTIME_SURFACE_COUNT ) {
			pSurface = NULL;
			(void)__uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, iSurfaceSlot, "", &pSurface);
			++iSurfaceSlot;
		}
	}
	return iCommandCount;
}

static int __uiDesignTimelineFrameTypeFromText(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "empty") ) return XUI_TIMELINE_FRAME_EMPTY;
	if ( __uiDesignTokenIs(sText, "normal") ) return XUI_TIMELINE_FRAME_NORMAL;
	if ( __uiDesignTokenIs(sText, "key") || __uiDesignTokenIs(sText, "keyframe") ) return XUI_TIMELINE_FRAME_KEY;
	if ( __uiDesignTokenIs(sText, "blank") || __uiDesignTokenIs(sText, "blank_key") ) return XUI_TIMELINE_FRAME_BLANK_KEY;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static int __uiDesignTimelineSpanTypeFromText(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "custom") ) return XUI_TIMELINE_SPAN_CUSTOM;
	if ( __uiDesignTokenIs(sText, "motion") ) return XUI_TIMELINE_SPAN_MOTION;
	if ( __uiDesignTokenIs(sText, "shape") ) return XUI_TIMELINE_SPAN_SHAPE;
	if ( __uiDesignTokenIs(sText, "event") ) return XUI_TIMELINE_SPAN_EVENT;
	if ( __uiDesignTokenIs(sText, "audio") ) return XUI_TIMELINE_SPAN_AUDIO;
	if ( __uiDesignTokenIs(sText, "hold") ) return XUI_TIMELINE_SPAN_HOLD;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static int __uiDesignTimelineFindLayer(char arrNames[][64], int iCount, const char* sName)
{
	int i;

	if ( sName == NULL ) return -1;
	if ( __uiDesignLooksIntText(sName) ) {
		i = __uiDesignParseIntText(sName, -1);
		if ( i >= 0 && i < iCount ) return i;
	}
	for ( i = 0; i < iCount; ++i ) {
		if ( __uiDesignTextEqualsNoCase(arrNames[i], sName) ) return i;
	}
	return -1;
}

static int __uiDesignTimelineEnsureLayer(xui_widget pWidget, char arrNames[][64], int* pCount, int iCapacity, const char* sName)
{
	int iLayer;

	if ( (pWidget == NULL) || (arrNames == NULL) || (pCount == NULL) || (sName == NULL) || (sName[0] == 0) ) return -1;
	iLayer = __uiDesignTimelineFindLayer(arrNames, *pCount, sName);
	if ( iLayer >= 0 ) return iLayer;
	if ( *pCount >= iCapacity ) return -1;
	if ( xuiTimeLineViewAddLayer(pWidget, sName, &iLayer) != XUI_OK ) return -1;
	__uiDesignCopyText(arrNames[*pCount], 64, sName);
	++(*pCount);
	return iLayer;
}

static void __uiDesignTimelineClearLayers(xui_widget pWidget)
{
	int iCount;

	if ( pWidget == NULL ) return;
	(void)xuiTimeLineViewClearSelection(pWidget);
	iCount = xuiTimeLineViewGetLayerCount(pWidget);
	while ( iCount > 0 ) {
		--iCount;
		(void)xuiTimeLineViewRemoveLayer(pWidget, iCount);
	}
}

static int __uiDesignApplyTimelineSelection(ui_design_node_t* pNode, char arrLayerNames[][64], int iLayerCount)
{
	const char* sCursor;
	char sLine[512];
	char* arrFields[5];
	int iFieldCount;
	int iLayer0;
	int iLayer1;
	int iFrame0;
	int iFrame1;
	int bSelected;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	(void)xuiTimeLineViewClearSelection(pNode->pWidget);
	sCursor = __uiDesignText(pNode, "data.selection", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 || arrFields[0][0] == 0 || arrFields[1][0] == 0 ) continue;
		iLayer0 = __uiDesignTimelineFindLayer(arrLayerNames, iLayerCount, arrFields[0]);
		if ( iLayer0 < 0 ) continue;
		iFrame0 = __uiDesignParseIntText(arrFields[1], 0);
		iLayer1 = iLayer0;
		iFrame1 = iFrame0;
		if ( iFieldCount > 2 && arrFields[2][0] != 0 ) {
			iLayer1 = __uiDesignTimelineFindLayer(arrLayerNames, iLayerCount, arrFields[2]);
			if ( iLayer1 < 0 ) iLayer1 = iLayer0;
		}
		if ( iFieldCount > 3 && arrFields[3][0] != 0 ) {
			iFrame1 = __uiDesignParseIntText(arrFields[3], iFrame0);
		}
		bSelected = (iFieldCount > 4) ? __uiDesignParseBoolText(arrFields[4], 1) : 1;
		if ( (iLayer0 != iLayer1) || (iFrame0 != iFrame1) ) {
			(void)xuiTimeLineViewSelectRange(pNode->pWidget, iLayer0, iFrame0, iLayer1, iFrame1, bSelected);
		} else {
			(void)xuiTimeLineViewSelectFrame(pNode->pWidget, iLayer0, iFrame0, bSelected);
		}
	}
	return XUI_OK;
}

static int __uiDesignApplyTimelineData(ui_design_node_t* pNode)
{
	const char* sCursor;
	char sLine[1024];
	char* arrFields[8];
	char arrLayerNames[XUI_TIMELINE_LAYER_CAPACITY][64];
	int iLayerCount;
	int iFieldCount;
	int iLayer;
	int iSpan;
	int bUseSplitTables;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignTimelineClearLayers(pNode->pWidget);
	memset(arrLayerNames, 0, sizeof(arrLayerNames));
	iLayerCount = 0;
	bUseSplitTables = __uiDesignHasProperty(pNode, "data.timelineLayers") ||
		__uiDesignHasProperty(pNode, "data.timelineFrames") ||
		__uiDesignHasProperty(pNode, "data.timelineSpans");
	if ( bUseSplitTables ) {
		sCursor = __uiDesignText(pNode, "data.timelineLayers", "");
		while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
			if ( sLine[0] == 0 ) continue;
			iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
			if ( iFieldCount <= 0 || arrFields[0][0] == 0 ) continue;
			iLayer = __uiDesignTimelineEnsureLayer(pNode->pWidget, arrLayerNames, &iLayerCount, UI_DESIGN_COUNT_OF(arrLayerNames), arrFields[0]);
			if ( iLayer >= 0 ) {
				if ( iFieldCount > 1 ) (void)xuiTimeLineViewSetLayerVisible(pNode->pWidget, iLayer, __uiDesignParseBoolText(arrFields[1], 1));
				if ( iFieldCount > 2 ) (void)xuiTimeLineViewSetLayerLocked(pNode->pWidget, iLayer, __uiDesignParseBoolText(arrFields[2], 0));
				if ( iFieldCount > 3 ) (void)xuiTimeLineViewSetLayerHeight(pNode->pWidget, iLayer, __uiDesignParseFloatText(arrFields[3], __uiDesignFloat(pNode, "metrics.rowHeight", 24.0f)));
				if ( iFieldCount > 4 ) (void)xuiTimeLineViewSetLayerColor(pNode->pWidget, iLayer, __uiDesignParseColorText(arrFields[4], XUI_COLOR_RGBA(74, 144, 226, 255)));
			}
		}
		sCursor = __uiDesignText(pNode, "data.timelineFrames", "");
		while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
			if ( sLine[0] == 0 ) continue;
			iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
			if ( iFieldCount < 2 || arrFields[0][0] == 0 ) continue;
			iLayer = __uiDesignTimelineEnsureLayer(pNode->pWidget, arrLayerNames, &iLayerCount, UI_DESIGN_COUNT_OF(arrLayerNames), arrFields[0]);
			if ( iLayer >= 0 ) {
				(void)xuiTimeLineViewSetFrame(pNode->pWidget,
					iLayer,
					__uiDesignParseIntText(arrFields[1], 0),
					(iFieldCount > 2) ? __uiDesignTimelineFrameTypeFromText(arrFields[2], XUI_TIMELINE_FRAME_KEY) : XUI_TIMELINE_FRAME_KEY,
					NULL);
			}
		}
		sCursor = __uiDesignText(pNode, "data.timelineSpans", "");
		while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
			if ( sLine[0] == 0 ) continue;
			iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
			if ( iFieldCount < 4 || arrFields[0][0] == 0 ) continue;
			iLayer = __uiDesignTimelineEnsureLayer(pNode->pWidget, arrLayerNames, &iLayerCount, UI_DESIGN_COUNT_OF(arrLayerNames), arrFields[0]);
			if ( iLayer >= 0 && xuiTimeLineViewAddSpan(pNode->pWidget, iLayer, __uiDesignParseIntText(arrFields[1], 0), __uiDesignParseIntText(arrFields[2], 1), __uiDesignTimelineSpanTypeFromText(arrFields[3], XUI_TIMELINE_SPAN_CUSTOM), __uiDesignField(arrFields, iFieldCount, 4, ""), &iSpan) == XUI_OK ) {
				if ( iFieldCount > 5 && arrFields[5][0] != 0 ) (void)xuiTimeLineViewSetSpanColor(pNode->pWidget, iSpan, __uiDesignParseColorText(arrFields[5], XUI_COLOR_RGBA(49, 126, 214, 255)));
				if ( iFieldCount > 6 && arrFields[6][0] != 0 ) (void)xuiTimeLineViewSetSpanCustomType(pNode->pWidget, iSpan, arrFields[6]);
			}
		}
		return __uiDesignApplyTimelineSelection(pNode, arrLayerNames, iLayerCount);
	}
	sCursor = __uiDesignText(pNode, "data.layers", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount <= 0 || arrFields[0][0] == 0 ) continue;
		if ( __uiDesignTokenIs(arrFields[0], "frame") ) {
			if ( iFieldCount < 4 ) continue;
			iLayer = __uiDesignTimelineEnsureLayer(pNode->pWidget, arrLayerNames, &iLayerCount, UI_DESIGN_COUNT_OF(arrLayerNames), arrFields[1]);
			if ( iLayer >= 0 ) (void)xuiTimeLineViewSetFrame(pNode->pWidget, iLayer, __uiDesignParseIntText(arrFields[2], 0), __uiDesignTimelineFrameTypeFromText(arrFields[3], XUI_TIMELINE_FRAME_KEY), NULL);
		} else if ( __uiDesignTokenIs(arrFields[0], "span") ) {
			if ( iFieldCount < 6 ) continue;
			iLayer = __uiDesignTimelineEnsureLayer(pNode->pWidget, arrLayerNames, &iLayerCount, UI_DESIGN_COUNT_OF(arrLayerNames), arrFields[1]);
			if ( iLayer >= 0 && xuiTimeLineViewAddSpan(pNode->pWidget, iLayer, __uiDesignParseIntText(arrFields[2], 0), __uiDesignParseIntText(arrFields[3], 1), __uiDesignTimelineSpanTypeFromText(arrFields[4], XUI_TIMELINE_SPAN_CUSTOM), arrFields[5], &iSpan) == XUI_OK ) {
				if ( iFieldCount > 6 && arrFields[6][0] != 0 ) (void)xuiTimeLineViewSetSpanColor(pNode->pWidget, iSpan, __uiDesignParseColorText(arrFields[6], XUI_COLOR_RGBA(49, 126, 214, 255)));
				if ( iFieldCount > 7 && arrFields[7][0] != 0 ) (void)xuiTimeLineViewSetSpanCustomType(pNode->pWidget, iSpan, arrFields[7]);
			}
		} else {
			if ( __uiDesignTokenIs(arrFields[0], "layer") ) {
				if ( iFieldCount < 2 ) continue;
				iLayer = __uiDesignTimelineEnsureLayer(pNode->pWidget, arrLayerNames, &iLayerCount, UI_DESIGN_COUNT_OF(arrLayerNames), arrFields[1]);
				if ( iLayer >= 0 ) {
					if ( iFieldCount > 2 ) (void)xuiTimeLineViewSetLayerVisible(pNode->pWidget, iLayer, __uiDesignParseBoolText(arrFields[2], 1));
					if ( iFieldCount > 3 ) (void)xuiTimeLineViewSetLayerLocked(pNode->pWidget, iLayer, __uiDesignParseBoolText(arrFields[3], 0));
					if ( iFieldCount > 4 ) (void)xuiTimeLineViewSetLayerHeight(pNode->pWidget, iLayer, __uiDesignParseFloatText(arrFields[4], __uiDesignFloat(pNode, "metrics.rowHeight", 24.0f)));
					if ( iFieldCount > 5 ) (void)xuiTimeLineViewSetLayerColor(pNode->pWidget, iLayer, __uiDesignParseColorText(arrFields[5], XUI_COLOR_RGBA(74, 144, 226, 255)));
				}
			} else {
				iLayer = __uiDesignTimelineEnsureLayer(pNode->pWidget, arrLayerNames, &iLayerCount, UI_DESIGN_COUNT_OF(arrLayerNames), arrFields[0]);
				if ( iLayer >= 0 ) {
					if ( iFieldCount > 1 ) (void)xuiTimeLineViewSetLayerVisible(pNode->pWidget, iLayer, __uiDesignParseBoolText(arrFields[1], 1));
					if ( iFieldCount > 2 ) (void)xuiTimeLineViewSetLayerLocked(pNode->pWidget, iLayer, __uiDesignParseBoolText(arrFields[2], 0));
					if ( iFieldCount > 3 ) (void)xuiTimeLineViewSetLayerHeight(pNode->pWidget, iLayer, __uiDesignParseFloatText(arrFields[3], __uiDesignFloat(pNode, "metrics.rowHeight", 24.0f)));
					if ( iFieldCount > 4 ) (void)xuiTimeLineViewSetLayerColor(pNode->pWidget, iLayer, __uiDesignParseColorText(arrFields[4], XUI_COLOR_RGBA(74, 144, 226, 255)));
				}
			}
		}
	}
	if ( iLayerCount == 0 ) {
		iLayer = __uiDesignTimelineEnsureLayer(pNode->pWidget, arrLayerNames, &iLayerCount, UI_DESIGN_COUNT_OF(arrLayerNames), "Layer 1");
		if ( iLayer >= 0 ) {
			(void)xuiTimeLineViewSetFrame(pNode->pWidget, iLayer, 0, XUI_TIMELINE_FRAME_KEY, NULL);
			(void)xuiTimeLineViewSetFrame(pNode->pWidget, iLayer, 24, XUI_TIMELINE_FRAME_KEY, NULL);
			(void)xuiTimeLineViewAddSpan(pNode->pWidget, iLayer, 4, 22, XUI_TIMELINE_SPAN_MOTION, "Move", &iSpan);
		}
		iLayer = __uiDesignTimelineEnsureLayer(pNode->pWidget, arrLayerNames, &iLayerCount, UI_DESIGN_COUNT_OF(arrLayerNames), "Layer 2");
		if ( iLayer >= 0 ) {
			(void)xuiTimeLineViewSetFrame(pNode->pWidget, iLayer, 12, XUI_TIMELINE_FRAME_KEY, NULL);
			(void)xuiTimeLineViewAddSpan(pNode->pWidget, iLayer, 12, 38, XUI_TIMELINE_SPAN_EVENT, "Event", &iSpan);
		}
	}
	return __uiDesignApplyTimelineSelection(pNode, arrLayerNames, iLayerCount);
}

static int __uiDesignTimelineMenuCommandFromText(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "rename") || __uiDesignTokenIs(sText, "layerRename") ) return XUI_TIMELINE_MENU_LAYER_RENAME;
	if ( __uiDesignTokenIs(sText, "visible") || __uiDesignTokenIs(sText, "showHide") ||
	     __uiDesignTokenIs(sText, "show_hide") || __uiDesignTokenIs(sText, "layerVisible") ) {
		return XUI_TIMELINE_MENU_LAYER_SHOW_HIDE;
	}
	if ( __uiDesignTokenIs(sText, "locked") || __uiDesignTokenIs(sText, "lock") ||
	     __uiDesignTokenIs(sText, "lockUnlock") || __uiDesignTokenIs(sText, "layerLocked") ) {
		return XUI_TIMELINE_MENU_LAYER_LOCK_UNLOCK;
	}
	if ( __uiDesignTokenIs(sText, "addLayer") || __uiDesignTokenIs(sText, "add_layer") ||
	     __uiDesignTokenIs(sText, "add") ) {
		return XUI_TIMELINE_MENU_LAYER_ADD;
	}
	if ( __uiDesignTokenIs(sText, "deleteLayer") || __uiDesignTokenIs(sText, "delete_layer") ||
	     __uiDesignTokenIs(sText, "delete") ) {
		return XUI_TIMELINE_MENU_LAYER_DELETE;
	}
	if ( __uiDesignTokenIs(sText, "moveUp") || __uiDesignTokenIs(sText, "move_up") ) return XUI_TIMELINE_MENU_LAYER_MOVE_UP;
	if ( __uiDesignTokenIs(sText, "moveDown") || __uiDesignTokenIs(sText, "move_down") ) return XUI_TIMELINE_MENU_LAYER_MOVE_DOWN;
	if ( __uiDesignTokenIs(sText, "insertFrame") || __uiDesignTokenIs(sText, "insert_frame") ||
	     __uiDesignTokenIs(sText, "frame") ) {
		return XUI_TIMELINE_MENU_FRAME_INSERT;
	}
	if ( __uiDesignTokenIs(sText, "insertKeyframe") || __uiDesignTokenIs(sText, "insert_keyframe") ||
	     __uiDesignTokenIs(sText, "keyframe") ) {
		return XUI_TIMELINE_MENU_FRAME_KEY;
	}
	if ( __uiDesignTokenIs(sText, "insertBlankKeyframe") || __uiDesignTokenIs(sText, "insert_blank_keyframe") ||
	     __uiDesignTokenIs(sText, "blankKeyframe") || __uiDesignTokenIs(sText, "blank_keyframe") ) {
		return XUI_TIMELINE_MENU_FRAME_BLANK_KEY;
	}
	if ( __uiDesignTokenIs(sText, "clearKeyframe") || __uiDesignTokenIs(sText, "clear_keyframe") ) return XUI_TIMELINE_MENU_FRAME_CLEAR;
	if ( __uiDesignTokenIs(sText, "createSpan") || __uiDesignTokenIs(sText, "create_span") ||
	     __uiDesignTokenIs(sText, "spanCreate") ) {
		return XUI_TIMELINE_MENU_SPAN_CREATE;
	}
	if ( __uiDesignTokenIs(sText, "createSpanFromSelection") || __uiDesignTokenIs(sText, "create_span_from_selection") ||
	     __uiDesignTokenIs(sText, "spanCreateFromSelection") || __uiDesignTokenIs(sText, "selectionSpan") ) {
		return XUI_TIMELINE_MENU_SPAN_CREATE_FROM_SELECTION;
	}
	if ( __uiDesignTokenIs(sText, "clearSpan") || __uiDesignTokenIs(sText, "clear_span") ||
	     __uiDesignTokenIs(sText, "spanClear") ) {
		return XUI_TIMELINE_MENU_SPAN_CLEAR;
	}
	return __uiDesignParseIntText(sText, iDefault);
}

static int __uiDesignApplyTimelineMenuTitles(ui_design_node_t* pNode, xui_widget pWidget)
{
	static const int arrCommands[] = {
		XUI_TIMELINE_MENU_LAYER_RENAME,
		XUI_TIMELINE_MENU_LAYER_SHOW_HIDE,
		XUI_TIMELINE_MENU_LAYER_LOCK_UNLOCK,
		XUI_TIMELINE_MENU_LAYER_ADD,
		XUI_TIMELINE_MENU_LAYER_DELETE,
		XUI_TIMELINE_MENU_LAYER_MOVE_UP,
		XUI_TIMELINE_MENU_LAYER_MOVE_DOWN,
		XUI_TIMELINE_MENU_FRAME_INSERT,
		XUI_TIMELINE_MENU_FRAME_KEY,
		XUI_TIMELINE_MENU_FRAME_BLANK_KEY,
		XUI_TIMELINE_MENU_FRAME_CLEAR,
		XUI_TIMELINE_MENU_SPAN_CREATE,
		XUI_TIMELINE_MENU_SPAN_CREATE_FROM_SELECTION,
		XUI_TIMELINE_MENU_SPAN_CLEAR
	};
	const char* sCursor;
	char sLine[512];
	char* arrFields[2];
	int iFieldCount;
	int iCommand;
	int i;

	if ( (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < (int)UI_DESIGN_COUNT_OF(arrCommands); ++i ) {
		(void)xuiTimeLineViewSetMenuTitle(pWidget, arrCommands[i], "");
	}
	sCursor = __uiDesignText(pNode, "data.menuTitles", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount < 2) || (arrFields[0][0] == 0) ) continue;
		iCommand = __uiDesignTimelineMenuCommandFromText(arrFields[0], 0);
		if ( iCommand <= 0 ) continue;
		(void)xuiTimeLineViewSetMenuTitle(pWidget, iCommand, arrFields[1]);
	}
	return XUI_OK;
}

static int __uiDesignApplyRadioGroupOptions(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pGroup)
{
	const char* arrOptions[32];
	int arrEnabled[32];
	int arrChecked[32];
	const char* sCursor;
	char sLine[512];
	char* arrFields[3];
	xui_surface pIndicatorSurface;
	xui_rect_t tUncheckedSrc;
	xui_rect_t tCheckedSrc;
	xui_radio_desc_t tRadio;
	xui_widget pChild;
	xui_widget pNext;
	xui_widget pRadio;
	int iCount;
	int iSelected;
	int bHasChecked;
	int iFieldCount;
	int i;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pGroup == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pChild = xuiWidgetGetFirstChild(pGroup);
	while ( pChild != NULL ) {
		pNext = xuiWidgetGetNextSibling(pChild);
		xuiWidgetDestroy(pChild);
		pChild = pNext;
	}
	__uiDesignTextPoolReset(pNode);
	iSelected = __uiDesignInt(pNode, "data.selected", 0);
	bHasChecked = 0;
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.options", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < UI_DESIGN_COUNT_OF(arrOptions) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount <= 0 || arrFields[0][0] == 0 ) continue;
		arrOptions[iCount] = __uiDesignTextPoolAdd(pNode, arrFields[0]);
		arrEnabled[iCount] = (iFieldCount > 1) ? __uiDesignParseBoolText(arrFields[1], 1) : 1;
		arrChecked[iCount] = (iFieldCount > 2) ? __uiDesignParseBoolText(arrFields[2], 0) : 0;
		if ( arrChecked[iCount] && !bHasChecked ) {
			iSelected = iCount;
			bHasChecked = 1;
		}
		++iCount;
	}
	if ( iCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultRadioOptions) && iCount < UI_DESIGN_COUNT_OF(arrOptions); ++i ) {
			arrOptions[iCount] = __uiDesignTextPoolAdd(pNode, g_arrDefaultRadioOptions[i]);
			arrEnabled[iCount] = 1;
			arrChecked[iCount] = 0;
			++iCount;
		}
	}
	for ( i = 0; i < iCount; ++i ) {
		arrChecked[i] = (i == iSelected);
	}
	pIndicatorSurface = NULL;
	iRet = __uiDesignLoadRuntimeSurface(pApp, pNode, __uiDesignText(pNode, "data.indicatorSource", ""), &pIndicatorSurface);
	if ( iRet != XUI_OK ) pIndicatorSurface = NULL;
	__uiDesignReadTwoStateSourceRects((pIndicatorSurface != NULL) ? pNode : NULL, &tUncheckedSrc, &tCheckedSrc);
	memset(&tRadio, 0, sizeof(tRadio));
	tRadio.iSize = sizeof(tRadio);
	tRadio.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tRadio.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tRadio.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tRadio.iTextFlags = (uint32_t)__uiDesignInt(pNode, "text.flags", XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	tRadio.fIndicatorSize = __uiDesignFloat(pNode, "metrics.indicatorSize", 16.0f);
	tRadio.fGap = __uiDesignFloat(pNode, "metrics.indicatorTextGap", 7.0f);
	tRadio.bUseBuiltinAtlas = __uiDesignBool(pNode, "behavior.useBuiltinAtlas", 1);
	for ( i = 0; i < iCount; ++i ) {
		tRadio.sText = arrOptions[i];
		tRadio.bChecked = arrChecked[i];
		iRet = xuiRadioGroupAddOption(pGroup, &pRadio, &tRadio);
		if ( iRet != XUI_OK ) return iRet;
		(void)xuiWidgetSetEnabled(pRadio, arrEnabled[i]);
		(void)xuiRadioSetColors(pRadio,
			__uiDesignColor(pNode, "appearance.accentColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(141, 165, 192, 255)),
			__uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)));
		if ( pIndicatorSurface != NULL ) {
			(void)xuiRadioSetIndicatorSurface(pRadio, pIndicatorSurface, tUncheckedSrc, pIndicatorSurface, tCheckedSrc);
		}
	}
	(void)xuiRadioGroupSetSelectedIndex(pGroup, iSelected);
	return XUI_OK;
}

static int __uiDesignFlowRoute(const char* sText, int iDefault)
{
	int iRoute;

	if ( __uiDesignTokenIs(sText, "auto") ) return XUI_FLOW_ROUTE_AUTO;
	if ( __uiDesignTokenIs(sText, "straight") || __uiDesignTokenIs(sText, "line") ) return XUI_FLOW_ROUTE_STRAIGHT;
	if ( __uiDesignTokenIs(sText, "orthogonal") || __uiDesignTokenIs(sText, "ortho") ) return XUI_FLOW_ROUTE_ORTHOGONAL;
	if ( __uiDesignTokenIs(sText, "bezier") || __uiDesignTokenIs(sText, "curve") ) return XUI_FLOW_ROUTE_BEZIER;
	iRoute = __uiDesignParseIntText(sText, iDefault);
	if ( iRoute < XUI_FLOW_ROUTE_AUTO || iRoute > XUI_FLOW_ROUTE_BEZIER ) return iDefault;
	return iRoute;
}

static int __uiDesignFlowKind(const char* sText, int iDefault)
{
	int iKind;

	if ( __uiDesignTokenIs(sText, "control") || __uiDesignTokenIs(sText, "flow") ) return XUI_FLOW_PORT_CONTROL;
	if ( __uiDesignTokenIs(sText, "data") || __uiDesignTokenIs(sText, "value") ) return XUI_FLOW_PORT_DATA;
	iKind = __uiDesignParseIntText(sText, iDefault);
	if ( iKind != XUI_FLOW_PORT_CONTROL && iKind != XUI_FLOW_PORT_DATA ) return iDefault;
	return iKind;
}

static int __uiDesignFlowDirection(const char* sText, int iDefault)
{
	int iDirection;

	if ( __uiDesignTokenIs(sText, "input") || __uiDesignTokenIs(sText, "in") ) return XUI_FLOW_PORT_INPUT;
	if ( __uiDesignTokenIs(sText, "output") || __uiDesignTokenIs(sText, "out") ) return XUI_FLOW_PORT_OUTPUT;
	iDirection = __uiDesignParseIntText(sText, iDefault);
	if ( iDirection != XUI_FLOW_PORT_INPUT && iDirection != XUI_FLOW_PORT_OUTPUT ) return iDefault;
	return iDirection;
}

static int __uiDesignFlowNodeRunState(const char* sText, int iDefault)
{
	int iState;

	if ( __uiDesignTokenIs(sText, "idle") || __uiDesignTokenIs(sText, "none") ) return XUI_WORKFLOW_NODE_RUN_IDLE;
	if ( __uiDesignTokenIs(sText, "queued") || __uiDesignTokenIs(sText, "queue") ) return XUI_WORKFLOW_NODE_RUN_QUEUED;
	if ( __uiDesignTokenIs(sText, "running") || __uiDesignTokenIs(sText, "active") ) return XUI_WORKFLOW_NODE_RUN_RUNNING;
	if ( __uiDesignTokenIs(sText, "success") || __uiDesignTokenIs(sText, "ok") ) return XUI_WORKFLOW_NODE_RUN_SUCCESS;
	if ( __uiDesignTokenIs(sText, "failed") || __uiDesignTokenIs(sText, "error") ) return XUI_WORKFLOW_NODE_RUN_FAILED;
	if ( __uiDesignTokenIs(sText, "skipped") || __uiDesignTokenIs(sText, "skip") ) return XUI_WORKFLOW_NODE_RUN_SKIPPED;
	if ( __uiDesignTokenIs(sText, "warning") || __uiDesignTokenIs(sText, "warn") ) return XUI_WORKFLOW_NODE_RUN_WARNING;
	iState = __uiDesignParseIntText(sText, iDefault);
	if ( iState < XUI_WORKFLOW_NODE_RUN_IDLE || iState > XUI_WORKFLOW_NODE_RUN_WARNING ) return iDefault;
	return iState;
}

static int __uiDesignFlowEdgeRunState(const char* sText, int iDefault)
{
	int iState;

	if ( __uiDesignTokenIs(sText, "idle") || __uiDesignTokenIs(sText, "none") ) return XUI_WORKFLOW_EDGE_RUN_IDLE;
	if ( __uiDesignTokenIs(sText, "active") || __uiDesignTokenIs(sText, "running") ) return XUI_WORKFLOW_EDGE_RUN_ACTIVE;
	if ( __uiDesignTokenIs(sText, "taken") || __uiDesignTokenIs(sText, "success") ) return XUI_WORKFLOW_EDGE_RUN_TAKEN;
	if ( __uiDesignTokenIs(sText, "skipped") || __uiDesignTokenIs(sText, "skip") ) return XUI_WORKFLOW_EDGE_RUN_SKIPPED;
	if ( __uiDesignTokenIs(sText, "invalid") || __uiDesignTokenIs(sText, "error") ) return XUI_WORKFLOW_EDGE_RUN_INVALID;
	iState = __uiDesignParseIntText(sText, iDefault);
	if ( iState < XUI_WORKFLOW_EDGE_RUN_IDLE || iState > XUI_WORKFLOW_EDGE_RUN_INVALID ) return iDefault;
	return iState;
}

static int __uiDesignFlowDiagnosticSeverity(const char* sText, int iDefault)
{
	int iSeverity;

	if ( __uiDesignTokenIs(sText, "error") ) return XUI_CODE_DIAGNOSTIC_ERROR;
	if ( __uiDesignTokenIs(sText, "warning") || __uiDesignTokenIs(sText, "warn") ) return XUI_CODE_DIAGNOSTIC_WARNING;
	if ( __uiDesignTokenIs(sText, "info") ) return XUI_CODE_DIAGNOSTIC_INFO;
	if ( __uiDesignTokenIs(sText, "hint") ) return XUI_CODE_DIAGNOSTIC_HINT;
	iSeverity = __uiDesignParseIntText(sText, iDefault);
	if ( iSeverity < XUI_CODE_DIAGNOSTIC_ERROR || iSeverity > XUI_CODE_DIAGNOSTIC_HINT ) return iDefault;
	return iSeverity;
}

static int __uiDesignFlowPortSeen(const ui_design_flow_port_seen_t* pSeen, int iCount, const char* sNode, const char* sPort)
{
	int i;

	if ( (pSeen == NULL) || (sNode == NULL) || (sPort == NULL) ) return 0;
	for ( i = 0; i < iCount; ++i ) {
		if ( strcmp(pSeen[i].sNode, sNode) == 0 && strcmp(pSeen[i].sPort, sPort) == 0 ) return 1;
	}
	return 0;
}

static int __uiDesignFlowEnsurePort(xui_flow_graph pGraph, ui_design_flow_port_seen_t* pSeen, int* pSeenCount, int iSeenCapacity,
	const char* sNode, const char* sPort, const char* sTitle, int iDirection, int iKind,
	int bRequired, int bMulti, int bDynamic, const char* sDataType)
{
	xui_flow_port_desc_t tPort;
	int iNode;
	int iRet;

	if ( (pGraph == NULL) || (pSeen == NULL) || (pSeenCount == NULL) || (sNode == NULL) || (sNode[0] == 0) || (sPort == NULL) || (sPort[0] == 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( __uiDesignFlowPortSeen(pSeen, *pSeenCount, sNode, sPort) ) return XUI_OK;
	iNode = xuiFlowGraphFindNode(pGraph, sNode);
	if ( iNode < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tPort, 0, sizeof(tPort));
	tPort.iSize = sizeof(tPort);
	tPort.sId = sPort;
	tPort.sTitle = (sTitle != NULL && sTitle[0] != 0) ? sTitle : sPort;
	tPort.sDataType = (sDataType != NULL && sDataType[0] != 0) ? sDataType : NULL;
	tPort.iDirection = iDirection;
	tPort.iKind = iKind;
	tPort.bRequired = bRequired;
	tPort.bMulti = bMulti;
	tPort.bDynamic = bDynamic;
	iRet = xuiFlowGraphAddPort(pGraph, iNode, &tPort, NULL);
	if ( iRet != XUI_OK && iRet != XUI_ERROR_ALREADY_INITIALIZED ) return iRet;
	if ( *pSeenCount < iSeenCapacity ) {
		__uiDesignCopyText(pSeen[*pSeenCount].sNode, sizeof(pSeen[*pSeenCount].sNode), sNode);
		__uiDesignCopyText(pSeen[*pSeenCount].sPort, sizeof(pSeen[*pSeenCount].sPort), sPort);
		pSeen[*pSeenCount].iDirection = iDirection;
		++(*pSeenCount);
	}
	return XUI_OK;
}

static int __uiDesignApplyFlowGraphPorts(ui_design_node_t* pNode, xui_flow_graph pGraph, ui_design_flow_port_seen_t* pSeen, int* pSeenCount, int iSeenCapacity)
{
	const char* sRows;
	const char* sLineCursor;
	char sLine[1024];
	char* arrFields[9];
	int iFieldCount;
	int iDirection;
	int iKind;
	int iRet;

	if ( (pNode == NULL) || (pGraph == NULL) || (pSeen == NULL) || (pSeenCount == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	sRows = __uiDesignText(pNode, "data.ports", "");
	sLineCursor = sRows;
	while ( __uiDesignNextLine(&sLineCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 || arrFields[0][0] == 0 || arrFields[1][0] == 0 ) continue;
		iDirection = __uiDesignFlowDirection(__uiDesignField(arrFields, iFieldCount, 3, NULL), XUI_FLOW_PORT_INPUT);
		iKind = __uiDesignFlowKind(__uiDesignField(arrFields, iFieldCount, 4, NULL), XUI_FLOW_PORT_CONTROL);
		iRet = __uiDesignFlowEnsurePort(pGraph, pSeen, pSeenCount, iSeenCapacity,
			arrFields[0], arrFields[1], __uiDesignField(arrFields, iFieldCount, 2, NULL),
			iDirection, iKind,
			__uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 5, NULL), 0),
			__uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 6, NULL), 0),
			__uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 7, NULL), 0),
			__uiDesignField(arrFields, iFieldCount, 8, NULL));
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __uiDesignApplyFlowDiagnostics(ui_design_node_t* pNode, xui_flow_graph pGraph)
{
	xui_flow_diagnostic_desc_t tDiag;
	const char* sRows;
	const char* sLineCursor;
	char sLine[1024];
	char* arrFields[6];
	int iFieldCount;
	int iRet;

	if ( (pNode == NULL) || (pGraph == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	sRows = __uiDesignText(pNode, "data.diagnostics", "");
	sLineCursor = sRows;
	while ( __uiDesignNextLine(&sLineCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 3 || arrFields[2][0] == 0 ) continue;
		memset(&tDiag, 0, sizeof(tDiag));
		tDiag.iSize = sizeof(tDiag);
		tDiag.iSeverity = __uiDesignFlowDiagnosticSeverity(__uiDesignField(arrFields, iFieldCount, 0, NULL), XUI_CODE_DIAGNOSTIC_ERROR);
		tDiag.sCode = __uiDesignField(arrFields, iFieldCount, 1, "");
		tDiag.sMessage = __uiDesignField(arrFields, iFieldCount, 2, "");
		tDiag.sNode = __uiDesignField(arrFields, iFieldCount, 3, "");
		tDiag.sEdge = __uiDesignField(arrFields, iFieldCount, 4, "");
		tDiag.sPath = __uiDesignField(arrFields, iFieldCount, 5, "");
		iRet = xuiFlowGraphAddDiagnostic(pGraph, &tDiag, NULL);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static void __uiDesignFillFlowGraphDesc(const ui_design_node_t* pNode, xui_flow_graph_desc_t* pDesc)
{
	if ( pDesc == NULL ) return;
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iSize = sizeof(*pDesc);
	pDesc->iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(246, 248, 252, 255));
	pDesc->iGridColor = __uiDesignColor(pNode, "appearance.gridColor", XUI_COLOR_RGBA(214, 220, 230, 120));
	pDesc->iNodeColor = __uiDesignColor(pNode, "appearance.nodeColor", XUI_COLOR_WHITE);
	pDesc->iSelectedNodeColor = __uiDesignColor(pNode, "appearance.selectedNodeColor", XUI_COLOR_RGBA(226, 242, 255, 255));
	pDesc->iNodeBorderColor = __uiDesignColor(pNode, "appearance.nodeBorderColor", XUI_COLOR_RGBA(74, 93, 118, 180));
	pDesc->iPortColor = __uiDesignColor(pNode, "appearance.portColor", XUI_COLOR_RGBA(42, 134, 230, 255));
	pDesc->iEdgeColor = __uiDesignColor(pNode, "appearance.edgeColor", XUI_COLOR_RGBA(86, 108, 134, 220));
	pDesc->iSelectedEdgeColor = __uiDesignColor(pNode, "appearance.selectedEdgeColor", XUI_COLOR_RGBA(26, 115, 232, 255));
}

static int __uiDesignApplyFlowGraphStyle(xui_widget pWidget, const ui_design_node_t* pNode)
{
	xui_style_property_t arrStyle[8];
	const char* arrNames[8];
	uint32_t arrColors[8];
	int i;

	if ( (pWidget == NULL) || (pNode == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	arrNames[0] = "flowgraph.background.color";
	arrNames[1] = "flowgraph.grid.color";
	arrNames[2] = "flowgraph.node.color";
	arrNames[3] = "flowgraph.node.selected_color";
	arrNames[4] = "flowgraph.node.border_color";
	arrNames[5] = "flowgraph.port.color";
	arrNames[6] = "flowgraph.edge.color";
	arrNames[7] = "flowgraph.edge.selected_color";
	arrColors[0] = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(246, 248, 252, 255));
	arrColors[1] = __uiDesignColor(pNode, "appearance.gridColor", XUI_COLOR_RGBA(214, 220, 230, 120));
	arrColors[2] = __uiDesignColor(pNode, "appearance.nodeColor", XUI_COLOR_WHITE);
	arrColors[3] = __uiDesignColor(pNode, "appearance.selectedNodeColor", XUI_COLOR_RGBA(226, 242, 255, 255));
	arrColors[4] = __uiDesignColor(pNode, "appearance.nodeBorderColor", XUI_COLOR_RGBA(74, 93, 118, 180));
	arrColors[5] = __uiDesignColor(pNode, "appearance.portColor", XUI_COLOR_RGBA(42, 134, 230, 255));
	arrColors[6] = __uiDesignColor(pNode, "appearance.edgeColor", XUI_COLOR_RGBA(86, 108, 134, 220));
	arrColors[7] = __uiDesignColor(pNode, "appearance.selectedEdgeColor", XUI_COLOR_RGBA(26, 115, 232, 255));
	memset(arrStyle, 0, sizeof(arrStyle));
	for ( i = 0; i < UI_DESIGN_COUNT_OF(arrStyle); ++i ) {
		arrStyle[i].iSize = sizeof(arrStyle[i]);
		arrStyle[i].sName = arrNames[i];
		arrStyle[i].tValue.iSize = sizeof(arrStyle[i].tValue);
		arrStyle[i].tValue.iType = XUI_STYLE_VALUE_COLOR;
		arrStyle[i].tValue.iColor = arrColors[i];
	}
	return xuiWidgetSetInlineStyle(pWidget, arrStyle, UI_DESIGN_COUNT_OF(arrStyle));
}

static int __uiDesignApplyViewportToGraph(const ui_design_node_t* pNode, xui_flow_graph pGraph)
{
	xui_flow_viewport_t tViewport;

	if ( (pNode == NULL) || (pGraph == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tViewport, 0, sizeof(tViewport));
	tViewport.iSize = sizeof(tViewport);
	tViewport.fPanX = __uiDesignFloat(pNode, "value.panX", 0.0f);
	tViewport.fPanY = __uiDesignFloat(pNode, "value.panY", 0.0f);
	tViewport.fZoom = __uiDesignFloat(pNode, "value.zoom", 1.0f);
	if ( tViewport.fZoom < 0.01f ) tViewport.fZoom = 1.0f;
	tViewport.fWidth = pNode->tRect.fW;
	tViewport.fHeight = pNode->tRect.fH;
	return xuiFlowGraphSetViewport(pGraph, &tViewport);
}

static xvalue __uiDesignCreateNodeConfigArrayValue(const char* sText)
{
	char arrItems[32][64];
	xvalue pArray;
	int iCount;
	int i;

	pArray = xvoCreateArray();
	if ( pArray == NULL ) return NULL;
	iCount = __uiDesignSplitCommaList(sText, arrItems, UI_DESIGN_COUNT_OF(arrItems));
	for ( i = 0; i < iCount; ++i ) {
		if ( !xvoArrayAppendText(pArray, arrItems[i], (uint32_t)strlen(arrItems[i]), FALSE) ) {
			xvoUnref(pArray);
			return NULL;
		}
	}
	return pArray;
}

static xvalue __uiDesignCreateNodeConfigValue(const char* sType, const char* sText)
{
	if ( sText == NULL ) sText = "";
	if ( __uiDesignTokenIs(sType, "null") ) return xvoCreateNull();
	if ( __uiDesignTokenIs(sType, "bool") || __uiDesignTokenIs(sType, "boolean") ) {
		return xvoCreateBool(__uiDesignParseBoolText(sText, 0) ? TRUE : FALSE);
	}
	if ( __uiDesignTokenIs(sType, "int") || __uiDesignTokenIs(sType, "integer") ) {
		return xvoCreateInt(__uiDesignParseIntText(sText, 0));
	}
	if ( __uiDesignTokenIs(sType, "float") || __uiDesignTokenIs(sType, "number") || __uiDesignTokenIs(sType, "double") ) {
		return xvoCreateFloat((double)__uiDesignParseFloatText(sText, 0.0f));
	}
	if ( __uiDesignTokenIs(sType, "array") || __uiDesignTokenIs(sType, "csv") || __uiDesignTokenIs(sType, "list") || __uiDesignTokenIs(sType, "multiSelect") ) {
		return __uiDesignCreateNodeConfigArrayValue(sText);
	}
	if ( __uiDesignTokenIs(sType, "table") || __uiDesignTokenIs(sType, "object") || __uiDesignTokenIs(sType, "group") || __uiDesignTokenIs(sType, "tabs") ) {
		return xvoCreateTable();
	}
	return xvoCreateText((ptr)(void*)sText, (uint32_t)strlen(sText), FALSE);
}

static void __uiDesignReleaseNodeConfigTables(ui_design_node_config_table_t* pTables, int iCount)
{
	int i;

	if ( pTables == NULL ) return;
	for ( i = 0; i < iCount; ++i ) {
		if ( pTables[i].pConfig != NULL ) {
			xvoUnref(pTables[i].pConfig);
			pTables[i].pConfig = NULL;
		}
	}
}

static int __uiDesignFindNodeConfigTable(ui_design_node_config_table_t* pTables, int iCount, const char* sNode)
{
	int i;

	if ( (pTables == NULL) || (sNode == NULL) ) return -1;
	for ( i = 0; i < iCount; ++i ) {
		if ( strcmp(pTables[i].sNode, sNode) == 0 ) return i;
	}
	return -1;
}

static int __uiDesignBuildNodeConfigTables(ui_design_node_t* pNode, ui_design_node_config_table_t* pTables, int iCapacity, int* pCount)
{
	const char* sRows;
	const char* sLineCursor;
	char sLine[1024];
	char* arrFields[4];
	xvalue pValue;
	int iFieldCount;
	int iTable;
	int iCount;

	if ( (pNode == NULL) || (pTables == NULL) || (pCount == NULL) || (iCapacity <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pTables, 0, sizeof(pTables[0]) * (size_t)iCapacity);
	*pCount = 0;
	iCount = 0;
	sRows = __uiDesignText(pNode, "data.nodeConfigs", "");
	sLineCursor = sRows;
	while ( __uiDesignNextLine(&sLineCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 || arrFields[0][0] == 0 || arrFields[1][0] == 0 ) continue;
		iTable = __uiDesignFindNodeConfigTable(pTables, iCount, arrFields[0]);
		if ( iTable < 0 ) {
			if ( iCount >= iCapacity ) {
				__uiDesignReleaseNodeConfigTables(pTables, iCount);
				return XUI_ERROR_OUT_OF_MEMORY;
			}
			snprintf(pTables[iCount].sNode, sizeof(pTables[iCount].sNode), "%s", arrFields[0]);
			pTables[iCount].pConfig = xvoCreateTable();
			if ( pTables[iCount].pConfig == NULL ) {
				__uiDesignReleaseNodeConfigTables(pTables, iCount);
				return XUI_ERROR_OUT_OF_MEMORY;
			}
			iTable = iCount++;
		}
		pValue = __uiDesignCreateNodeConfigValue(__uiDesignField(arrFields, iFieldCount, 2, "string"), __uiDesignField(arrFields, iFieldCount, 3, ""));
		if ( pValue == NULL ) {
			__uiDesignReleaseNodeConfigTables(pTables, iCount);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		if ( !xvoTableSetValue(pTables[iTable].pConfig, arrFields[1], (uint32_t)strlen(arrFields[1]), pValue, TRUE) ) {
			xvoUnref(pValue);
			__uiDesignReleaseNodeConfigTables(pTables, iCount);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
	}
	*pCount = iCount;
	return XUI_OK;
}

static int __uiDesignApplyFlowGraphNodeConfigs(ui_design_node_t* pNode, xui_flow_graph pGraph)
{
	ui_design_node_config_table_t arrTables[96];
	int iCount;
	int iRet;
	int i;

	if ( (pNode == NULL) || (pGraph == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignBuildNodeConfigTables(pNode, arrTables, UI_DESIGN_COUNT_OF(arrTables), &iCount);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < iCount; ++i ) {
		iRet = xuiFlowGraphSetNodeConfig(pGraph, arrTables[i].sNode, arrTables[i].pConfig);
		if ( iRet == XUI_ERROR_OUT_OF_MEMORY ) {
			__uiDesignReleaseNodeConfigTables(arrTables, iCount);
			return iRet;
		}
	}
	__uiDesignReleaseNodeConfigTables(arrTables, iCount);
	return XUI_OK;
}

static int __uiDesignApplyWorkflowNodeConfigs(ui_design_node_t* pNode, xui_workflow pWorkflow)
{
	ui_design_node_config_table_t arrTables[96];
	xvalue pMerged;
	int iCount;
	int iRet;
	int iSetRet;
	int i;

	if ( (pNode == NULL) || (pWorkflow == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignBuildNodeConfigTables(pNode, arrTables, UI_DESIGN_COUNT_OF(arrTables), &iCount);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < iCount; ++i ) {
		pMerged = NULL;
		iSetRet = xuiWorkflowGetNodeConfig(pWorkflow, arrTables[i].sNode, &pMerged);
		if ( iSetRet == XUI_OK && pMerged != NULL && xvoTableMerge(pMerged, arrTables[i].pConfig, TRUE) ) {
			iSetRet = xuiWorkflowSetNodeConfig(pWorkflow, arrTables[i].sNode, pMerged);
		} else {
			iSetRet = xuiWorkflowSetNodeConfig(pWorkflow, arrTables[i].sNode, arrTables[i].pConfig);
		}
		if ( pMerged != NULL ) xvoUnref(pMerged);
		if ( iSetRet == XUI_ERROR_OUT_OF_MEMORY ) {
			__uiDesignReleaseNodeConfigTables(arrTables, iCount);
			return iSetRet;
		}
	}
	__uiDesignReleaseNodeConfigTables(arrTables, iCount);
	return XUI_OK;
}

static int __uiDesignBuildFlowGraph(ui_design_node_t* pNode, xui_flow_graph* ppGraph)
{
	xui_flow_graph pGraph;
	xui_flow_node_desc_t tNode;
	xui_flow_edge_desc_t tEdge;
	ui_design_flow_port_seen_t arrPorts[160];
	const char* sRows;
	const char* sLineCursor;
	const char* sSelected;
	char sLine[1024];
	char* arrFields[12];
	int iFieldCount;
	int iRet;
	int iIndex;
	int iRouteDefault;
	int iRoute;
	int iKind;
	int iRunState;
	int iPortCount;
	int iHistoryLimit;

	if ( (pNode == NULL) || (ppGraph == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppGraph = NULL;
	iRet = xuiFlowGraphCreate(&pGraph);
	if ( iRet != XUI_OK ) return iRet;
	iHistoryLimit = __uiDesignInt(pNode, "behavior.commandHistoryLimit", 128);
	if ( iHistoryLimit < 1 ) iHistoryLimit = 1;
	iRet = xuiFlowGraphSetCommandHistoryLimit(pGraph, iHistoryLimit);
	if ( iRet != XUI_OK ) {
		xuiFlowGraphDestroy(pGraph);
		return iRet;
	}
	iRet = __uiDesignApplyViewportToGraph(pNode, pGraph);
	if ( iRet != XUI_OK ) {
		xuiFlowGraphDestroy(pGraph);
		return iRet;
	}
	sRows = __uiDesignText(pNode, "data.nodes", "");
	sLineCursor = sRows;
	while ( __uiDesignNextLine(&sLineCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 1 || arrFields[0][0] == 0 ) continue;
		memset(&tNode, 0, sizeof(tNode));
		tNode.iSize = sizeof(tNode);
		tNode.sId = arrFields[0];
		tNode.sType = __uiDesignField(arrFields, iFieldCount, 1, "node");
		tNode.sTitle = __uiDesignField(arrFields, iFieldCount, 2, arrFields[0]);
		tNode.fX = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 3, NULL), 40.0f);
		tNode.fY = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 4, NULL), 40.0f);
		tNode.fW = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 5, NULL), 150.0f);
		tNode.fH = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 6, NULL), 86.0f);
		if ( tNode.fW < 24.0f ) tNode.fW = 24.0f;
		if ( tNode.fH < 24.0f ) tNode.fH = 24.0f;
		tNode.sSummary = __uiDesignField(arrFields, iFieldCount, 7, "");
		iRet = xuiFlowGraphAddNode(pGraph, &tNode, &iIndex);
		if ( iRet == XUI_ERROR_OUT_OF_MEMORY ) {
			xuiFlowGraphDestroy(pGraph);
			return iRet;
		}
		if ( iRet == XUI_OK ) {
			iRunState = __uiDesignFlowNodeRunState(__uiDesignField(arrFields, iFieldCount, 8, NULL), XUI_WORKFLOW_NODE_RUN_IDLE);
			if ( iRunState != XUI_WORKFLOW_NODE_RUN_IDLE || __uiDesignField(arrFields, iFieldCount, 9, "")[0] != 0 ) {
				(void)xuiFlowGraphSetNodeRunState(pGraph, tNode.sId, iRunState, __uiDesignField(arrFields, iFieldCount, 9, ""));
			}
		}
	}
	memset(arrPorts, 0, sizeof(arrPorts));
	iPortCount = 0;
	iRet = __uiDesignApplyFlowGraphPorts(pNode, pGraph, arrPorts, &iPortCount, UI_DESIGN_COUNT_OF(arrPorts));
	if ( iRet != XUI_OK ) {
		xuiFlowGraphDestroy(pGraph);
		return iRet;
	}
	iRet = __uiDesignApplyFlowGraphNodeConfigs(pNode, pGraph);
	if ( iRet != XUI_OK ) {
		xuiFlowGraphDestroy(pGraph);
		return iRet;
	}
	iRouteDefault = __uiDesignFlowRoute(__uiDesignText(pNode, "behavior.defaultRoute", "0"), XUI_FLOW_ROUTE_AUTO);
	sRows = __uiDesignText(pNode, "data.edges", "");
	sLineCursor = sRows;
	while ( __uiDesignNextLine(&sLineCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 5 || arrFields[0][0] == 0 ) continue;
		iKind = __uiDesignFlowKind(__uiDesignField(arrFields, iFieldCount, 6, NULL), XUI_FLOW_PORT_CONTROL);
		iRet = __uiDesignFlowEnsurePort(pGraph, arrPorts, &iPortCount, UI_DESIGN_COUNT_OF(arrPorts),
			arrFields[1], arrFields[2], NULL, XUI_FLOW_PORT_OUTPUT, iKind, 0, 0, 0, NULL);
		if ( iRet == XUI_ERROR_OUT_OF_MEMORY ) {
			xuiFlowGraphDestroy(pGraph);
			return iRet;
		}
		if ( iRet != XUI_OK ) continue;
		iRet = __uiDesignFlowEnsurePort(pGraph, arrPorts, &iPortCount, UI_DESIGN_COUNT_OF(arrPorts),
			arrFields[3], arrFields[4], NULL, XUI_FLOW_PORT_INPUT, iKind, 0, 0, 0, NULL);
		if ( iRet == XUI_ERROR_OUT_OF_MEMORY ) {
			xuiFlowGraphDestroy(pGraph);
			return iRet;
		}
		if ( iRet != XUI_OK ) continue;
		iRoute = __uiDesignFlowRoute(__uiDesignField(arrFields, iFieldCount, 5, NULL), iRouteDefault);
		memset(&tEdge, 0, sizeof(tEdge));
		tEdge.iSize = sizeof(tEdge);
		tEdge.sId = arrFields[0];
		tEdge.sFromNode = arrFields[1];
		tEdge.sFromPort = arrFields[2];
		tEdge.sToNode = arrFields[3];
		tEdge.sToPort = arrFields[4];
		tEdge.iRouteStyle = iRoute;
		tEdge.iKind = iKind;
		tEdge.fRouteBias = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 9, NULL), 0.0f);
		tEdge.fRouteSourceOffset = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 10, NULL), 0.0f);
		tEdge.fRouteTargetOffset = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 11, NULL), 0.0f);
		iRet = xuiFlowGraphAddEdge(pGraph, &tEdge, NULL);
		if ( iRet == XUI_ERROR_OUT_OF_MEMORY ) {
			xuiFlowGraphDestroy(pGraph);
			return iRet;
		}
		if ( iRet == XUI_OK ) {
			iRunState = __uiDesignFlowEdgeRunState(__uiDesignField(arrFields, iFieldCount, 7, NULL), XUI_WORKFLOW_EDGE_RUN_IDLE);
			if ( iRunState != XUI_WORKFLOW_EDGE_RUN_IDLE || __uiDesignField(arrFields, iFieldCount, 8, "")[0] != 0 ) {
				(void)xuiFlowGraphSetEdgeRunState(pGraph, tEdge.sId, iRunState, __uiDesignField(arrFields, iFieldCount, 8, ""));
			}
		}
	}
	sSelected = __uiDesignText(pNode, "data.selectedNode", "");
	if ( sSelected[0] != 0 ) (void)xuiFlowGraphSelectNode(pGraph, sSelected, 1);
	sSelected = __uiDesignText(pNode, "data.selectedEdge", "");
	if ( sSelected[0] != 0 ) (void)xuiFlowGraphSelectEdge(pGraph, sSelected, 1);
	iRet = __uiDesignApplyFlowDiagnostics(pNode, pGraph);
	if ( iRet != XUI_OK ) {
		xuiFlowGraphDestroy(pGraph);
		return iRet;
	}
	(void)xuiFlowGraphMarkClean(pGraph);
	*ppGraph = pGraph;
	return XUI_OK;
}

static int __uiDesignWorkflowMakePorts(const char* sList, int iDirection, xui_flow_port_desc_t* arrPorts, ui_design_workflow_port_text_t* arrText, int iCapacity)
{
	char arrItems[16][64];
	int iCount;
	int i;

	if ( (arrPorts == NULL) || (arrText == NULL) || (iCapacity <= 0) ) return 0;
	iCount = __uiDesignSplitCommaList(sList, arrItems, UI_DESIGN_COUNT_OF(arrItems));
	if ( iCount > iCapacity ) iCount = iCapacity;
	for ( i = 0; i < iCount; ++i ) {
		__uiDesignCopyText(arrText[i].sId, sizeof(arrText[i].sId), arrItems[i]);
		__uiDesignCopyText(arrText[i].sTitle, sizeof(arrText[i].sTitle), arrItems[i]);
		arrText[i].sDataType[0] = 0;
		memset(&arrPorts[i], 0, sizeof(arrPorts[i]));
		arrPorts[i].iSize = sizeof(arrPorts[i]);
		arrPorts[i].sId = arrText[i].sId;
		arrPorts[i].sTitle = arrText[i].sTitle;
		arrPorts[i].iDirection = iDirection;
		arrPorts[i].iKind = XUI_FLOW_PORT_CONTROL;
	}
	return iCount;
}

static int __uiDesignWorkflowPortExists(const xui_flow_port_desc_t* arrPorts, int iCount, const char* sId)
{
	int i;

	if ( (arrPorts == NULL) || (sId == NULL) || (sId[0] == 0) ) return 0;
	for ( i = 0; i < iCount; ++i ) {
		if ( arrPorts[i].sId != NULL && strcmp(arrPorts[i].sId, sId) == 0 ) return 1;
	}
	return 0;
}

static void __uiDesignWorkflowAppendTypePorts(ui_design_node_t* pNode, const char* sType,
	xui_flow_port_desc_t* arrInputs, int* pInputCount, ui_design_workflow_port_text_t* arrInputText,
	xui_flow_port_desc_t* arrOutputs, int* pOutputCount, ui_design_workflow_port_text_t* arrOutputText,
	int iCapacity)
{
	xui_flow_port_desc_t* arrPorts;
	ui_design_workflow_port_text_t* arrText;
	int* pCount;
	const char* sRows;
	const char* sLineCursor;
	const char* sTitle;
	const char* sDataType;
	char sLine[1024];
	char* arrFields[9];
	int iFieldCount;
	int iDirection;
	int iKind;
	int iIndex;

	if ( (pNode == NULL) || (sType == NULL) || (sType[0] == 0) ||
	     (arrInputs == NULL) || (pInputCount == NULL) || (arrInputText == NULL) ||
	     (arrOutputs == NULL) || (pOutputCount == NULL) || (arrOutputText == NULL) || (iCapacity <= 0) ) {
		return;
	}
	sRows = __uiDesignText(pNode, "data.typePorts", "");
	sLineCursor = sRows;
	while ( __uiDesignNextLine(&sLineCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 || strcmp(arrFields[0], sType) != 0 || arrFields[1][0] == 0 ) continue;
		iDirection = __uiDesignFlowDirection(__uiDesignField(arrFields, iFieldCount, 3, NULL), XUI_FLOW_PORT_INPUT);
		if ( iDirection == XUI_FLOW_PORT_OUTPUT ) {
			arrPorts = arrOutputs;
			arrText = arrOutputText;
			pCount = pOutputCount;
		} else {
			arrPorts = arrInputs;
			arrText = arrInputText;
			pCount = pInputCount;
		}
		if ( *pCount >= iCapacity || __uiDesignWorkflowPortExists(arrPorts, *pCount, arrFields[1]) ) continue;
		iIndex = *pCount;
		sTitle = __uiDesignField(arrFields, iFieldCount, 2, "");
		sDataType = __uiDesignField(arrFields, iFieldCount, 8, "");
		__uiDesignCopyText(arrText[iIndex].sId, sizeof(arrText[iIndex].sId), arrFields[1]);
		__uiDesignCopyText(arrText[iIndex].sTitle, sizeof(arrText[iIndex].sTitle), (sTitle[0] != 0) ? sTitle : arrFields[1]);
		__uiDesignCopyText(arrText[iIndex].sDataType, sizeof(arrText[iIndex].sDataType), sDataType);
		iKind = __uiDesignFlowKind(__uiDesignField(arrFields, iFieldCount, 4, NULL), XUI_FLOW_PORT_CONTROL);
		memset(&arrPorts[iIndex], 0, sizeof(arrPorts[iIndex]));
		arrPorts[iIndex].iSize = sizeof(arrPorts[iIndex]);
		arrPorts[iIndex].sId = arrText[iIndex].sId;
		arrPorts[iIndex].sTitle = arrText[iIndex].sTitle;
		arrPorts[iIndex].sDataType = (arrText[iIndex].sDataType[0] != 0) ? arrText[iIndex].sDataType : NULL;
		arrPorts[iIndex].iDirection = iDirection;
		arrPorts[iIndex].iKind = iKind;
		arrPorts[iIndex].bRequired = __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 5, NULL), 0);
		arrPorts[iIndex].bMulti = __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 6, NULL), 0);
		arrPorts[iIndex].bDynamic = __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 7, NULL), 0);
		++(*pCount);
	}
}

static int __uiDesignWorkflowRegisterTypes(ui_design_node_t* pNode, xui_workflow pWorkflow)
{
	xui_workflow_node_type_desc_t tType;
	xui_flow_port_desc_t arrInputs[16];
	xui_flow_port_desc_t arrOutputs[16];
	ui_design_workflow_port_text_t arrInputText[16];
	ui_design_workflow_port_text_t arrOutputText[16];
	xvalue pConfigSchema;
	const char* sRows;
	const char* sLineCursor;
	char sLine[1024];
	char* arrFields[8];
	int iFieldCount;
	int iRet;
	int iInputCount;
	int iOutputCount;

	if ( (pNode == NULL) || (pWorkflow == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	sRows = __uiDesignText(pNode, "data.nodeTypes", "");
	sLineCursor = sRows;
	while ( __uiDesignNextLine(&sLineCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 1 || arrFields[0][0] == 0 ) continue;
		memset(&tType, 0, sizeof(tType));
		tType.iSize = sizeof(tType);
		tType.sId = arrFields[0];
		tType.iVersion = __uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 5, NULL), 1);
		tType.sTitle = __uiDesignField(arrFields, iFieldCount, 1, arrFields[0]);
		tType.sCategory = __uiDesignField(arrFields, iFieldCount, 2, "Flow");
		tType.sDescription = __uiDesignField(arrFields, iFieldCount, 6, "");
		tType.sIcon = __uiDesignField(arrFields, iFieldCount, 7, "");
		pConfigSchema = NULL;
		iRet = __uiDesignWorkflowBuildTypeConfigSchema(pNode, arrFields[0], &pConfigSchema);
		if ( iRet != XUI_OK ) return iRet;
		tType.pConfigSchema = pConfigSchema;
		iInputCount = __uiDesignWorkflowMakePorts(__uiDesignField(arrFields, iFieldCount, 3, ""), XUI_FLOW_PORT_INPUT, arrInputs, arrInputText, UI_DESIGN_COUNT_OF(arrInputs));
		iOutputCount = __uiDesignWorkflowMakePorts(__uiDesignField(arrFields, iFieldCount, 4, ""), XUI_FLOW_PORT_OUTPUT, arrOutputs, arrOutputText, UI_DESIGN_COUNT_OF(arrOutputs));
		__uiDesignWorkflowAppendTypePorts(pNode, arrFields[0],
			arrInputs, &iInputCount, arrInputText,
			arrOutputs, &iOutputCount, arrOutputText,
			UI_DESIGN_COUNT_OF(arrInputs));
		tType.pInputs = arrInputs;
		tType.iInputCount = iInputCount;
		tType.pOutputs = arrOutputs;
		tType.iOutputCount = iOutputCount;
		iRet = xuiWorkflowRegisterNodeType(pWorkflow, &tType, NULL);
		if ( pConfigSchema != NULL ) xvoUnref(pConfigSchema);
		if ( iRet == XUI_ERROR_OUT_OF_MEMORY ) return iRet;
	}
	return XUI_OK;
}

static int __uiDesignBuildWorkflow(ui_design_node_t* pNode, xui_workflow* ppWorkflow)
{
	xui_workflow pWorkflow;
	xui_flow_graph pGraph;
	xui_workflow_variable_desc_t tVariable;
	xui_workflow_node_run_state_t tNodeState;
	xui_workflow_edge_run_state_t tEdgeState;
	xvalue pDefault;
	const char* sRows;
	const char* sLineCursor;
	const char* sSelected;
	const char* sDefaultText;
	char sLine[1024];
	char* arrFields[12];
	float fX;
	float fY;
	float fW;
	float fH;
	int iFieldCount;
	int iRet;
	int iRouteDefault;
	int iRoute;
	int iRunState;
	int iDiagCount;
	int iHistoryLimit;

	if ( (pNode == NULL) || (ppWorkflow == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWorkflow = NULL;
	iRet = xuiWorkflowCreate(&pWorkflow);
	if ( iRet != XUI_OK ) return iRet;
	pGraph = xuiWorkflowGetGraph(pWorkflow);
	iHistoryLimit = __uiDesignInt(pNode, "behavior.commandHistoryLimit", 128);
	if ( iHistoryLimit < 1 ) iHistoryLimit = 1;
	iRet = xuiFlowGraphSetCommandHistoryLimit(pGraph, iHistoryLimit);
	if ( iRet != XUI_OK ) {
		xuiWorkflowDestroy(pWorkflow);
		return iRet;
	}
	iRet = __uiDesignApplyViewportToGraph(pNode, pGraph);
	if ( iRet != XUI_OK ) {
		xuiWorkflowDestroy(pWorkflow);
		return iRet;
	}
	iRet = __uiDesignWorkflowRegisterTypes(pNode, pWorkflow);
	if ( iRet != XUI_OK ) {
		xuiWorkflowDestroy(pWorkflow);
		return iRet;
	}
	sRows = __uiDesignText(pNode, "data.nodes", "");
	sLineCursor = sRows;
	while ( __uiDesignNextLine(&sLineCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 || arrFields[0][0] == 0 || arrFields[1][0] == 0 ) continue;
		fX = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 3, NULL), 40.0f);
		fY = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 4, NULL), 40.0f);
		fW = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 5, NULL), 220.0f);
		fH = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 6, NULL), 120.0f);
		iRet = xuiWorkflowAddNode(pWorkflow, arrFields[0], arrFields[1], __uiDesignField(arrFields, iFieldCount, 2, arrFields[1]),
			fX, fY, NULL);
		if ( iRet == XUI_ERROR_OUT_OF_MEMORY ) {
			xuiWorkflowDestroy(pWorkflow);
			return iRet;
		}
		if ( iRet == XUI_OK ) {
			(void)xuiFlowGraphSetNodeBounds(pGraph, arrFields[1], fX, fY, fW, fH);
			if ( __uiDesignField(arrFields, iFieldCount, 7, "")[0] != 0 ) {
				(void)xuiFlowGraphSetNodeSummary(pGraph, arrFields[1], __uiDesignField(arrFields, iFieldCount, 7, ""));
			}
			iRunState = __uiDesignFlowNodeRunState(__uiDesignField(arrFields, iFieldCount, 8, NULL), XUI_WORKFLOW_NODE_RUN_IDLE);
			if ( iRunState != XUI_WORKFLOW_NODE_RUN_IDLE || __uiDesignField(arrFields, iFieldCount, 9, "")[0] != 0 ) {
				memset(&tNodeState, 0, sizeof(tNodeState));
				tNodeState.iSize = sizeof(tNodeState);
				tNodeState.sNodeId = arrFields[1];
				tNodeState.iState = iRunState;
				tNodeState.sPreview = __uiDesignField(arrFields, iFieldCount, 9, "");
				(void)xuiWorkflowSetNodeRunState(pWorkflow, &tNodeState);
			}
		}
	}
	iRet = __uiDesignApplyWorkflowNodeConfigs(pNode, pWorkflow);
	if ( iRet != XUI_OK ) {
		xuiWorkflowDestroy(pWorkflow);
		return iRet;
	}
	iRouteDefault = __uiDesignFlowRoute(__uiDesignText(pNode, "behavior.defaultRoute", "0"), XUI_FLOW_ROUTE_AUTO);
	sRows = __uiDesignText(pNode, "data.edges", "");
	sLineCursor = sRows;
	while ( __uiDesignNextLine(&sLineCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 5 || arrFields[0][0] == 0 ) continue;
		iRet = xuiWorkflowConnect(pWorkflow, arrFields[0], arrFields[1], arrFields[2], arrFields[3], arrFields[4], NULL);
		if ( iRet == XUI_ERROR_OUT_OF_MEMORY ) {
			xuiWorkflowDestroy(pWorkflow);
			return iRet;
		}
		if ( iRet == XUI_OK ) {
			iRoute = __uiDesignFlowRoute(__uiDesignField(arrFields, iFieldCount, 5, NULL), iRouteDefault);
			(void)xuiFlowGraphSetEdgeRoute(pGraph, arrFields[0], iRoute,
				__uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 9, NULL), 0.0f),
				__uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 10, NULL), 0.0f),
				__uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 11, NULL), 0.0f));
			iRunState = __uiDesignFlowEdgeRunState(__uiDesignField(arrFields, iFieldCount, 7, NULL), XUI_WORKFLOW_EDGE_RUN_IDLE);
			if ( iRunState != XUI_WORKFLOW_EDGE_RUN_IDLE || __uiDesignField(arrFields, iFieldCount, 8, "")[0] != 0 ) {
				memset(&tEdgeState, 0, sizeof(tEdgeState));
				tEdgeState.iSize = sizeof(tEdgeState);
				tEdgeState.sEdgeId = arrFields[0];
				tEdgeState.iState = iRunState;
				tEdgeState.sPreview = __uiDesignField(arrFields, iFieldCount, 8, "");
				(void)xuiWorkflowSetEdgeRunState(pWorkflow, &tEdgeState);
			}
		}
	}
	sRows = __uiDesignText(pNode, "data.variables", "");
	sLineCursor = sRows;
	while ( __uiDesignNextLine(&sLineCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 1 || arrFields[0][0] == 0 ) continue;
		sDefaultText = __uiDesignField(arrFields, iFieldCount, 4, "");
		pDefault = NULL;
		if ( sDefaultText[0] != 0 ) {
			pDefault = __uiDesignCreateNodeConfigValue(__uiDesignField(arrFields, iFieldCount, 2, "string"), sDefaultText);
			if ( pDefault == NULL ) {
				xuiWorkflowDestroy(pWorkflow);
				return XUI_ERROR_OUT_OF_MEMORY;
			}
		}
		memset(&tVariable, 0, sizeof(tVariable));
		tVariable.iSize = sizeof(tVariable);
		tVariable.sId = arrFields[0];
		tVariable.sTitle = __uiDesignField(arrFields, iFieldCount, 1, arrFields[0]);
		tVariable.sType = __uiDesignField(arrFields, iFieldCount, 2, "string");
		tVariable.sScope = __uiDesignField(arrFields, iFieldCount, 3, "workflow");
		tVariable.pDefaultValue = pDefault;
		iRet = xuiWorkflowAddVariable(pWorkflow, &tVariable, NULL);
		if ( pDefault != NULL ) xvoUnref(pDefault);
		if ( iRet == XUI_ERROR_OUT_OF_MEMORY ) {
			xuiWorkflowDestroy(pWorkflow);
			return iRet;
		}
	}
	sSelected = __uiDesignText(pNode, "data.selectedNode", "");
	if ( sSelected[0] != 0 ) (void)xuiWorkflowSelectNode(pWorkflow, sSelected, 1);
	iDiagCount = 0;
	iRet = xuiWorkflowValidateGraph(pWorkflow, &iDiagCount);
	if ( iRet != XUI_OK ) {
		xuiWorkflowDestroy(pWorkflow);
		return iRet;
	}
	iRet = __uiDesignApplyFlowDiagnostics(pNode, pGraph);
	if ( iRet != XUI_OK ) {
		xuiWorkflowDestroy(pWorkflow);
		return iRet;
	}
	(void)xuiFlowGraphMarkClean(pGraph);
	*ppWorkflow = pWorkflow;
	return XUI_OK;
}

static int __uiDesignApplyFlowGraph(ui_design_node_t* pNode)
{
	xui_flow_graph pGraph;
	int iRet;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignBuildFlowGraph(pNode, &pGraph);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiFlowGraphWidgetSetGraph(pNode->pWidget, pGraph, 1);
	if ( iRet != XUI_OK ) {
		xuiFlowGraphDestroy(pGraph);
		return iRet;
	}
	(void)__uiDesignApplyFlowGraphStyle(pNode->pWidget, pNode);
	return XUI_OK;
}

static int __uiDesignWorkflowConfigKind(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "string") || __uiDesignTokenIs(sText, "text") ) return XUI_WORKFLOW_CONFIG_FIELD_STRING;
	if ( __uiDesignTokenIs(sText, "textarea") || __uiDesignTokenIs(sText, "memo") ) return XUI_WORKFLOW_CONFIG_FIELD_TEXTAREA;
	if ( __uiDesignTokenIs(sText, "int") || __uiDesignTokenIs(sText, "integer") ) return XUI_WORKFLOW_CONFIG_FIELD_INT;
	if ( __uiDesignTokenIs(sText, "float") || __uiDesignTokenIs(sText, "number") || __uiDesignTokenIs(sText, "double") ) return XUI_WORKFLOW_CONFIG_FIELD_FLOAT;
	if ( __uiDesignTokenIs(sText, "bool") || __uiDesignTokenIs(sText, "boolean") ) return XUI_WORKFLOW_CONFIG_FIELD_BOOL;
	if ( __uiDesignTokenIs(sText, "expression") || __uiDesignTokenIs(sText, "expr") ) return XUI_WORKFLOW_CONFIG_FIELD_EXPRESSION;
	if ( __uiDesignTokenIs(sText, "select") || __uiDesignTokenIs(sText, "enum") || __uiDesignTokenIs(sText, "combo") ) return XUI_WORKFLOW_CONFIG_FIELD_SELECT;
	if ( __uiDesignTokenIs(sText, "multiSelect") || __uiDesignTokenIs(sText, "multi_select") || __uiDesignTokenIs(sText, "multiselect") ) return XUI_WORKFLOW_CONFIG_FIELD_MULTI_SELECT;
	if ( __uiDesignTokenIs(sText, "variableRef") || __uiDesignTokenIs(sText, "variable_ref") || __uiDesignTokenIs(sText, "var") ) return XUI_WORKFLOW_CONFIG_FIELD_VARIABLE_REF;
	if ( __uiDesignTokenIs(sText, "nodeOutputRef") || __uiDesignTokenIs(sText, "node_output_ref") || __uiDesignTokenIs(sText, "outputRef") ) return XUI_WORKFLOW_CONFIG_FIELD_NODE_OUTPUT_REF;
	if ( __uiDesignTokenIs(sText, "object") ) return XUI_WORKFLOW_CONFIG_FIELD_OBJECT;
	if ( __uiDesignTokenIs(sText, "array") || __uiDesignTokenIs(sText, "list") ) return XUI_WORKFLOW_CONFIG_FIELD_ARRAY;
	if ( __uiDesignTokenIs(sText, "group") ) return XUI_WORKFLOW_CONFIG_FIELD_GROUP;
	if ( __uiDesignTokenIs(sText, "tabs") || __uiDesignTokenIs(sText, "tab") ) return XUI_WORKFLOW_CONFIG_FIELD_TABS;
	if ( __uiDesignTokenIs(sText, "conditionBuilder") || __uiDesignTokenIs(sText, "condition_builder") || __uiDesignTokenIs(sText, "condition") ) return XUI_WORKFLOW_CONFIG_FIELD_CONDITION_BUILDER;
	if ( __uiDesignTokenIs(sText, "mappingBuilder") || __uiDesignTokenIs(sText, "mapping_builder") || __uiDesignTokenIs(sText, "mapping") ) return XUI_WORKFLOW_CONFIG_FIELD_MAPPING_BUILDER;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static xvalue __uiDesignWorkflowCreateStringArrayValue(const char* sText)
{
	char arrItems[32][64];
	xvalue pArray;
	int iCount;
	int i;

	if ( (sText == NULL) || (sText[0] == 0) ) return NULL;
	pArray = xvoCreateArray();
	if ( pArray == NULL ) return NULL;
	iCount = __uiDesignSplitCommaList(sText, arrItems, UI_DESIGN_COUNT_OF(arrItems));
	for ( i = 0; i < iCount; ++i ) {
		if ( !xvoArrayAppendText(pArray, arrItems[i], (uint32_t)strlen(arrItems[i]), FALSE) ) {
			xvoUnref(pArray);
			return NULL;
		}
	}
	return pArray;
}

static xvalue __uiDesignWorkflowCreateDefaultValue(int iKind, const char* sText)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return NULL;
	switch ( iKind ) {
	case XUI_WORKFLOW_CONFIG_FIELD_STRING:
	case XUI_WORKFLOW_CONFIG_FIELD_TEXTAREA:
	case XUI_WORKFLOW_CONFIG_FIELD_EXPRESSION:
	case XUI_WORKFLOW_CONFIG_FIELD_SELECT:
	case XUI_WORKFLOW_CONFIG_FIELD_VARIABLE_REF:
	case XUI_WORKFLOW_CONFIG_FIELD_NODE_OUTPUT_REF:
		return xvoCreateText((ptr)(void*)sText, (uint32_t)strlen(sText), FALSE);
	case XUI_WORKFLOW_CONFIG_FIELD_INT:
		return xvoCreateInt(__uiDesignParseIntText(sText, 0));
	case XUI_WORKFLOW_CONFIG_FIELD_FLOAT:
		return xvoCreateFloat((double)__uiDesignParseFloatText(sText, 0.0f));
	case XUI_WORKFLOW_CONFIG_FIELD_BOOL:
		return xvoCreateBool(__uiDesignParseBoolText(sText, 0) ? TRUE : FALSE);
	case XUI_WORKFLOW_CONFIG_FIELD_MULTI_SELECT:
	case XUI_WORKFLOW_CONFIG_FIELD_ARRAY:
		return __uiDesignWorkflowCreateStringArrayValue(sText);
	case XUI_WORKFLOW_CONFIG_FIELD_OBJECT:
	case XUI_WORKFLOW_CONFIG_FIELD_GROUP:
	case XUI_WORKFLOW_CONFIG_FIELD_TABS:
	case XUI_WORKFLOW_CONFIG_FIELD_CONDITION_BUILDER:
	case XUI_WORKFLOW_CONFIG_FIELD_MAPPING_BUILDER:
		return xvoCreateTable();
	default:
		break;
	}
	return NULL;
}

static int __uiDesignWorkflowBuildTypeConfigSchema(ui_design_node_t* pNode, const char* sType, xvalue* ppSchema)
{
	xui_workflow_config_field_desc_t tField;
	xvalue pSchema;
	xvalue pDefault;
	xvalue pOptions;
	xvalue pChildren;
	const char* sRows;
	const char* sLineCursor;
	const char* sDefaultText;
	const char* sOptionsText;
	const char* sChildrenText;
	char sLine[1024];
	char* arrFields[15];
	int iFieldCount;
	int iRet;
	int bHasAny;

	if ( (pNode == NULL) || (sType == NULL) || (ppSchema == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppSchema = NULL;
	pSchema = NULL;
	bHasAny = 0;
	sRows = __uiDesignText(pNode, "data.configFields", "");
	sLineCursor = sRows;
	while ( __uiDesignNextLine(&sLineCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 3 || strcmp(arrFields[0], sType) != 0 || arrFields[1][0] == 0 ) continue;
		if ( pSchema == NULL ) {
			iRet = xuiWorkflowConfigSchemaCreate(&pSchema);
			if ( iRet != XUI_OK ) return iRet;
		}
		memset(&tField, 0, sizeof(tField));
		tField.iSize = sizeof(tField);
		tField.sId = arrFields[1];
		tField.sTitle = __uiDesignField(arrFields, iFieldCount, 2, arrFields[1]);
		tField.iKind = __uiDesignWorkflowConfigKind(__uiDesignField(arrFields, iFieldCount, 3, "string"), XUI_WORKFLOW_CONFIG_FIELD_STRING);
		tField.bRequired = __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 4, ""), 0);
		sDefaultText = __uiDesignField(arrFields, iFieldCount, 5, "");
		pDefault = __uiDesignWorkflowCreateDefaultValue(tField.iKind, sDefaultText);
		if ( sDefaultText[0] != 0 && pDefault == NULL ) {
			xvoUnref(pSchema);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		if ( pDefault != NULL ) {
			tField.bHasDefault = 1;
			tField.pDefaultValue = pDefault;
		}
		if ( __uiDesignField(arrFields, iFieldCount, 6, "")[0] != 0 ) {
			tField.bHasMin = 1;
			tField.fMin = (double)__uiDesignParseFloatText(arrFields[6], 0.0f);
		}
		if ( __uiDesignField(arrFields, iFieldCount, 7, "")[0] != 0 ) {
			tField.bHasMax = 1;
			tField.fMax = (double)__uiDesignParseFloatText(arrFields[7], 0.0f);
		}
		sOptionsText = __uiDesignField(arrFields, iFieldCount, 8, "");
		pOptions = __uiDesignWorkflowCreateStringArrayValue(sOptionsText);
		if ( sOptionsText[0] != 0 && pOptions == NULL ) {
			if ( pDefault != NULL ) xvoUnref(pDefault);
			xvoUnref(pSchema);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		tField.pOptions = pOptions;
		sChildrenText = __uiDesignField(arrFields, iFieldCount, 9, "");
		pChildren = __uiDesignWorkflowCreateStringArrayValue(sChildrenText);
		if ( sChildrenText[0] != 0 && pChildren == NULL ) {
			if ( pDefault != NULL ) xvoUnref(pDefault);
			if ( pOptions != NULL ) xvoUnref(pOptions);
			xvoUnref(pSchema);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		tField.pChildren = pChildren;
		tField.sExpressionLanguage = __uiDesignField(arrFields, iFieldCount, 10, NULL);
		tField.sRefScope = __uiDesignField(arrFields, iFieldCount, 11, NULL);
		tField.sRefType = __uiDesignField(arrFields, iFieldCount, 12, NULL);
		tField.sGroup = __uiDesignField(arrFields, iFieldCount, 13, NULL);
		tField.sTab = __uiDesignField(arrFields, iFieldCount, 14, NULL);
		iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
		if ( pDefault != NULL ) xvoUnref(pDefault);
		if ( pOptions != NULL ) xvoUnref(pOptions);
		if ( pChildren != NULL ) xvoUnref(pChildren);
		if ( iRet != XUI_OK ) {
			xvoUnref(pSchema);
			return iRet;
		}
		bHasAny = 1;
	}
	if ( bHasAny ) {
		*ppSchema = pSchema;
	} else if ( pSchema != NULL ) {
		xvoUnref(pSchema);
	}
	return XUI_OK;
}

static int __uiDesignApplyWorkflow(ui_design_node_t* pNode)
{
	xui_workflow pWorkflow;
	xui_widget pCanvas;
	int iRet;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignBuildWorkflow(pNode, &pWorkflow);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWorkflowWidgetSetWorkflow(pNode->pWidget, pWorkflow, 1);
	if ( iRet != XUI_OK ) {
		xuiWorkflowDestroy(pWorkflow);
		return iRet;
	}
	pCanvas = xuiWorkflowWidgetGetCanvas(pNode->pWidget);
	if ( pCanvas != NULL ) (void)__uiDesignApplyFlowGraphStyle(pCanvas, pNode);
	return XUI_OK;
}

static int __uiDesignWidgetRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	ui_design_node_t* pNode;
	xui_proxy_t tProxy;
	xui_rect_t tRect;
	xui_rect_t tText;
	uint32_t iFill;
	uint32_t iBorder;
	int iRet;

	(void)iStateId;
	pNode = (ui_design_node_t*)pUser;
	if ( (pWidget == NULL) || (pNode == NULL) || (pDraw == NULL) ) return XUI_OK;
	if ( xuiGetProxy(xuiWidgetGetContext(pWidget), &tProxy) != XUI_OK ) return XUI_OK;
	tRect = xuiWidgetGetContentRect(pWidget);
	iFill = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 251, 255, 255));
	iBorder = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(111, 135, 164, 255));
	iRet = tProxy.drawRectFill(&tProxy, pDraw, tRect, iFill);
	if ( iRet != XUI_OK ) return iRet;
	iRet = tProxy.drawRectStroke(&tProxy, pDraw, tRect, 1.0f, iBorder);
	if ( iRet != XUI_OK ) return iRet;
	tText = (xui_rect_t){tRect.fX + 8.0f, tRect.fY + 4.0f, tRect.fW - 16.0f, 20.0f};
	return tProxy.drawText(&tProxy, pDraw, xuiGetDefaultFont(xuiWidgetGetContext(pWidget)), pNode->sText, tText, XUI_COLOR_RGBA(72, 88, 108, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
}

int uiDesignRegistryGetCount(void)
{
	return UI_DESIGN_COUNT_OF(g_arrControls);
}

const ui_design_control_desc_t* uiDesignRegistryGetAt(int iIndex)
{
	if ( iIndex < 0 || iIndex >= uiDesignRegistryGetCount() ) return NULL;
	return &g_arrControls[iIndex];
}

const ui_design_control_desc_t* uiDesignRegistryFind(ui_design_node_type_t iType)
{
	int i;

	for ( i = 0; i < uiDesignRegistryGetCount(); i++ ) {
		if ( g_arrControls[i].iType == iType ) return &g_arrControls[i];
	}
	return NULL;
}

int uiDesignRegistryGetCategoryCount(void)
{
	return UI_DESIGN_COUNT_OF(g_arrCategories);
}

const char* uiDesignRegistryGetCategory(int iIndex)
{
	if ( iIndex < 0 || iIndex >= uiDesignRegistryGetCategoryCount() ) return NULL;
	return g_arrCategories[iIndex];
}

int uiDesignRegistryGetCategoryIndex(const char* sCategory)
{
	int i;

	if ( sCategory == NULL ) return -1;
	for ( i = 0; i < uiDesignRegistryGetCategoryCount(); i++ ) {
		if ( strcmp(g_arrCategories[i], sCategory) == 0 ) return i;
	}
	return -1;
}

const ui_design_property_def_t* uiDesignRegistryFindProperty(const ui_design_control_desc_t* pDesc, const char* sId)
{
	int i;

	if ( (pDesc == NULL) || (sId == NULL) ) return NULL;
	for ( i = 0; i < pDesc->iPropertyCount; i++ ) {
		if ( strcmp(pDesc->pProperties[i].sId, sId) == 0 ) return &pDesc->pProperties[i];
	}
	return NULL;
}

int uiDesignRegistryInitNodeProperties(const ui_design_control_desc_t* pDesc, ui_design_node_t* pNode)
{
	int i;
	int iRet;

	if ( (pDesc == NULL) || (pNode == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pDesc->iPropertyCount; i++ ) {
		if ( pDesc->pProperties[i].sDefaultValue == NULL ) continue;
		if ( uiDesignNodeGetProperty(pNode, pDesc->pProperties[i].sId, NULL)[0] != '\0' ) continue;
		iRet = uiDesignNodeSetProperty(pNode, pDesc->pProperties[i].sId, pDesc->pProperties[i].sDefaultValue);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __uiDesignCreateWidget(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_cache_policy_t tPolicy;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiWidgetCreate(pApp->pContext, ppWidget);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tPolicy, 0, sizeof(tPolicy));
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	(void)xuiWidgetSetLayoutType(*ppWidget, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCachePolicy(*ppWidget, &tPolicy);
	(void)xuiWidgetSetCacheRenderCallback(*ppWidget, __uiDesignWidgetRender, pNode);
	return XUI_OK;
}

static int __uiDesignCreateLabel(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_label_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = pNode->sText;
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	return xuiLabelCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateHyperlink(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_hyperlink_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = pNode->sText;
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	tDesc.bUnderline = __uiDesignBool(pNode, "text.underline", 1);
	tDesc.bHoverUnderline = __uiDesignBool(pNode, "text.hoverUnderline", 1);
	tDesc.bActiveUnderline = __uiDesignBool(pNode, "text.activeUnderline", 1);
	tDesc.fLineGap = __uiDesignFloat(pNode, "text.lineGap", 0.0f);
	tDesc.fParagraphGap = __uiDesignFloat(pNode, "text.paragraphGap", 0.0f);
	return xuiHyperlinkCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateButton(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_button_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = pNode->sText;
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	return xuiButtonCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateCheckBox(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_checkbox_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = pNode->sText;
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.bChecked = __uiDesignBool(pNode, "checked", pNode->bChecked);
	tDesc.bUseBuiltinAtlas = __uiDesignBool(pNode, "behavior.useBuiltinAtlas", 1);
	return xuiCheckBoxCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateRadio(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_radio_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = pNode->sText;
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.bChecked = pNode->bChecked;
	tDesc.bUseBuiltinAtlas = __uiDesignBool(pNode, "behavior.useBuiltinAtlas", 1);
	return xuiRadioCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateToggle(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_toggle_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = pNode->sText;
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.bChecked = pNode->bChecked;
	tDesc.bUseBuiltinAtlas = __uiDesignBool(pNode, "behavior.useBuiltinAtlas", 1);
	tDesc.sUncheckedText = __uiDesignText(pNode, "text.uncheckedText", "Off");
	tDesc.sCheckedText = __uiDesignText(pNode, "text.checkedText", "On");
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iUncheckedTextColor = __uiDesignColor(pNode, "appearance.uncheckedTextColor", XUI_COLOR_RGBA(82, 101, 122, 255));
	tDesc.iCheckedTextColor = __uiDesignColor(pNode, "appearance.checkedTextColor", XUI_COLOR_WHITE);
	tDesc.fTrackWidth = __uiDesignFloat(pNode, "metrics.trackWidth", 44.0f);
	tDesc.fTrackHeight = __uiDesignFloat(pNode, "metrics.trackHeight", 22.0f);
	tDesc.fThumbSize = __uiDesignFloat(pNode, "metrics.thumbSize", 18.0f);
	tDesc.fGap = __uiDesignFloat(pNode, "metrics.gap", 8.0f);
	tDesc.fInnerTextPadding = __uiDesignFloat(pNode, "metrics.innerTextPadding", 6.0f);
	tDesc.fInnerTextGap = __uiDesignFloat(pNode, "metrics.innerTextGap", 2.0f);
	return xuiToggleCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateInput(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_input_desc_t tDesc;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = pNode->sText;
	tDesc.sPlaceholder = __uiDesignText(pNode, "text.placeholder", "Input text");
	tDesc.iMaxLength = __uiDesignInt(pNode, "text.maxLength", 0);
	tDesc.iTextAlign = __uiDesignInt(pNode, "text.align", XUI_INPUT_ALIGN_LEFT);
	tDesc.bPassword = __uiDesignBool(pNode, "behavior.password", 0);
	tDesc.bReadonly = __uiDesignBool(pNode, "behavior.readonly", 0);
	tDesc.bError = __uiDesignBool(pNode, "behavior.error", 0);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iPlaceholderColor = __uiDesignColor(pNode, "appearance.placeholderColor", XUI_COLOR_RGBA(135, 148, 166, 255));
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(150, 160, 172, 255));
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iHoverBackgroundColor = __uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(249, 252, 255, 255));
	tDesc.iDisabledBackgroundColor = __uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(242, 245, 249, 255));
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iHoverBorderColor = __uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(105, 166, 226, 255));
	tDesc.iFocusBorderColor = __uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iErrorBackgroundColor = __uiDesignColor(pNode, "appearance.errorBackgroundColor", XUI_COLOR_RGBA(255, 242, 243, 255));
	tDesc.iErrorBorderColor = __uiDesignColor(pNode, "appearance.errorBorderColor", XUI_COLOR_RGBA(214, 72, 86, 255));
	tDesc.iSelectionColor = __uiDesignColor(pNode, "appearance.selectionColor", XUI_COLOR_RGBA(47, 128, 237, 78));
	tDesc.iCursorColor = __uiDesignColor(pNode, "appearance.cursorColor", XUI_COLOR_RGBA(33, 94, 170, 255));
	tDesc.fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	iRet = xuiInputCreate(pApp->pContext, ppWidget, &tDesc);
	if ( iRet == XUI_OK ) iRet = __uiDesignApplyInputDecorations(pApp, pNode, *ppWidget);
	return iRet;
}

static int __uiDesignCreateTagInput(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_tag_input_desc_t tDesc;
	const char* arrTags[XUI_TAG_INPUT_TAG_CAPACITY];
	int iTagCount;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignTextPoolReset(pNode);
	iTagCount = __uiDesignParseTextLines(pNode, "data.tags", g_arrDefaultTags, UI_DESIGN_COUNT_OF(g_arrDefaultTags), arrTags, UI_DESIGN_COUNT_OF(arrTags));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.ppTags = arrTags;
	tDesc.iTagCount = iTagCount;
	tDesc.sText = "";
	tDesc.sPlaceholder = __uiDesignText(pNode, "text.placeholder", "Add tag");
	tDesc.iMaxLength = __uiDesignInt(pNode, "text.maxLength", 0);
	tDesc.iMaxTags = __uiDesignInt(pNode, "behavior.maxTags", 0);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.fTagHeight = __uiDesignFloat(pNode, "metrics.tagHeight", 22.0f);
	tDesc.fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(31, 41, 55, 255));
	tDesc.iPlaceholderColor = __uiDesignColor(pNode, "appearance.placeholderColor", XUI_COLOR_RGBA(135, 148, 166, 255));
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(150, 160, 172, 255));
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iHoverBackgroundColor = __uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(250, 253, 255, 255));
	tDesc.iFocusBackgroundColor = __uiDesignColor(pNode, "appearance.focusBackgroundColor", XUI_COLOR_WHITE);
	tDesc.iDisabledBackgroundColor = __uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(242, 245, 249, 255));
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iHoverBorderColor = __uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(126, 176, 226, 255));
	tDesc.iFocusBorderColor = __uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iTagBackgroundColor = __uiDesignColor(pNode, "appearance.tagBackgroundColor", XUI_COLOR_RGBA(234, 243, 255, 255));
	tDesc.iTagHoverBackgroundColor = __uiDesignColor(pNode, "appearance.tagHoverBackgroundColor", XUI_COLOR_RGBA(235, 241, 248, 255));
	tDesc.iTagTextColor = __uiDesignColor(pNode, "appearance.tagTextColor", XUI_COLOR_RGBA(28, 102, 176, 255));
	tDesc.iTagCloseColor = __uiDesignColor(pNode, "appearance.tagCloseColor", XUI_COLOR_RGBA(116, 128, 144, 255));
	tDesc.iTagCloseHoverColor = __uiDesignColor(pNode, "appearance.tagCloseHoverColor", XUI_COLOR_RGBA(66, 78, 94, 255));
	return xuiTagInputCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateNumericInput(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_numeric_input_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.sPlaceholder = __uiDesignText(pNode, "text.placeholder", "0");
	tDesc.fMin = __uiDesignFloat(pNode, "value.min", 0.0f);
	tDesc.fMax = __uiDesignFloat(pNode, "value.max", 100.0f);
	tDesc.fValue = __uiDesignFloat(pNode, "value.value", 25.0f);
	tDesc.fStep = __uiDesignFloat(pNode, "value.step", 1.0f);
	tDesc.bInteger = __uiDesignBool(pNode, "value.integer", 0);
	tDesc.iPrecision = __uiDesignInt(pNode, "value.precision", 2);
	tDesc.bReadonly = __uiDesignBool(pNode, "behavior.readonly", 0);
	tDesc.bSpinnerVisible = __uiDesignBool(pNode, "behavior.spinnerVisible", 1);
	tDesc.fSpinnerWidth = __uiDesignFloat(pNode, "metrics.spinnerWidth", 22.0f);
	tDesc.fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iPlaceholderColor = __uiDesignColor(pNode, "appearance.placeholderColor", XUI_COLOR_RGBA(135, 148, 166, 255));
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(150, 160, 172, 255));
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iHoverBackgroundColor = __uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(249, 252, 255, 255));
	tDesc.iDisabledBackgroundColor = __uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(242, 245, 249, 255));
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iHoverBorderColor = __uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(105, 166, 226, 255));
	tDesc.iFocusBorderColor = __uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iErrorBackgroundColor = __uiDesignColor(pNode, "appearance.errorBackgroundColor", XUI_COLOR_RGBA(255, 242, 243, 255));
	tDesc.iErrorBorderColor = __uiDesignColor(pNode, "appearance.errorBorderColor", XUI_COLOR_RGBA(214, 72, 86, 255));
	tDesc.iSelectionColor = __uiDesignColor(pNode, "appearance.selectionColor", XUI_COLOR_RGBA(47, 128, 237, 78));
	tDesc.iCursorColor = __uiDesignColor(pNode, "appearance.cursorColor", XUI_COLOR_RGBA(33, 94, 170, 255));
	tDesc.iSpinnerColor = __uiDesignColor(pNode, "appearance.spinnerColor", XUI_COLOR_RGBA(239, 245, 252, 255));
	tDesc.iSpinnerHoverColor = __uiDesignColor(pNode, "appearance.spinnerHoverColor", XUI_COLOR_RGBA(220, 235, 251, 255));
	tDesc.iSpinnerActiveColor = __uiDesignColor(pNode, "appearance.spinnerActiveColor", XUI_COLOR_RGBA(200, 221, 245, 255));
	tDesc.iSpinnerBorderColor = __uiDesignColor(pNode, "appearance.spinnerBorderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iSpinnerIconColor = __uiDesignColor(pNode, "appearance.spinnerIconColor", XUI_COLOR_RGBA(52, 71, 93, 255));
	tDesc.iSpinnerDisabledIconColor = __uiDesignColor(pNode, "appearance.spinnerDisabledIconColor", XUI_COLOR_RGBA(154, 168, 186, 255));
	return xuiNumericInputCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateTextEdit(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_text_edit_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = __uiDesignText(pNode, "data.text", "Line 1\nLine 2\nLine 3");
	tDesc.sPlaceholder = __uiDesignText(pNode, "text.placeholder", "Multi-line text");
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.iMaxLength = __uiDesignInt(pNode, "text.maxLength", 0);
	tDesc.bReadonly = __uiDesignBool(pNode, "behavior.readonly", 0);
	tDesc.bWordWrap = __uiDesignBool(pNode, "behavior.wordWrap", 1);
	tDesc.bLineNumbers = __uiDesignBool(pNode, "behavior.lineNumbers", 1);
	tDesc.fLineNumberWidth = __uiDesignFloat(pNode, "metrics.lineNumberWidth", 42.0f);
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iPlaceholderColor = __uiDesignColor(pNode, "appearance.placeholderColor", XUI_COLOR_RGBA(135, 148, 166, 255));
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(150, 160, 172, 255));
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iHoverBackgroundColor = __uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(249, 252, 255, 255));
	tDesc.iDisabledBackgroundColor = __uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(242, 245, 249, 255));
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iHoverBorderColor = __uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(105, 166, 226, 255));
	tDesc.iFocusBorderColor = __uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iSelectionColor = __uiDesignColor(pNode, "appearance.selectionColor", XUI_COLOR_RGBA(47, 128, 237, 78));
	tDesc.iCursorColor = __uiDesignColor(pNode, "appearance.cursorColor", XUI_COLOR_RGBA(33, 94, 170, 255));
	tDesc.iLineNumberColor = __uiDesignColor(pNode, "appearance.lineNumberColor", XUI_COLOR_RGBA(107, 127, 149, 255));
	tDesc.iLineNumberBackgroundColor = __uiDesignColor(pNode, "appearance.lineNumberBackgroundColor", XUI_COLOR_RGBA(243, 247, 251, 255));
	tDesc.iLineNumberBorderColor = __uiDesignColor(pNode, "appearance.lineNumberBorderColor", XUI_COLOR_RGBA(207, 218, 231, 255));
	tDesc.fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	tDesc.fLineGap = __uiDesignFloat(pNode, "metrics.lineGap", 2.0f);
	return xuiTextEditCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreatePanel(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_panel_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = pNode->sText;
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.iTitleColor = __uiDesignColor(pNode, "appearance.titleColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iDisabledTitleColor = __uiDesignColor(pNode, "appearance.disabledTitleColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 251, 255, 255));
	tDesc.iHeaderColor = __uiDesignColor(pNode, "appearance.headerColor", XUI_COLOR_RGBA(234, 242, 251, 255));
	tDesc.iClientColor = __uiDesignColor(pNode, "appearance.clientColor", XUI_COLOR_WHITE);
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iTitleFlags = __uiDesignInt(pNode, "text.flags", XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	tDesc.fHeaderHeight = __uiDesignFloat(pNode, "metrics.headerHeight", 28.0f);
	tDesc.fHeaderGap = __uiDesignFloat(pNode, "metrics.headerGap", 0.0f);
	tDesc.fIconSize = __uiDesignFloat(pNode, "metrics.iconSize", 16.0f);
	tDesc.fBorderWidth = __uiDesignFloat(pNode, "appearance.borderWidth", 1.0f);
	tDesc.bClipClient = __uiDesignBool(pNode, "behavior.clientClip", 1);
	return xuiPanelCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateSeparator(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_separator_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iColor = __uiDesignColor(pNode, "appearance.color", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.fThickness = __uiDesignFloat(pNode, "metrics.thickness", 1.0f);
	tDesc.iOrientation = __uiDesignInt(pNode, "behavior.orientation", XUI_SEPARATOR_HORIZONTAL);
	tDesc.iAlign = __uiDesignInt(pNode, "behavior.align", XUI_ALIGN_CENTER);
	tDesc.iLineStyle = __uiDesignInt(pNode, "behavior.lineStyle", XUI_SEPARATOR_SOLID);
	return xuiSeparatorCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateProgress(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_progress_desc_t tDesc;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.sText = __uiDesignText(pNode, "text.template", "{value}%");
	tDesc.fMin = __uiDesignFloat(pNode, "value.min", 0.0f);
	tDesc.fMax = __uiDesignFloat(pNode, "value.max", 100.0f);
	tDesc.fValue = __uiDesignFloat(pNode, "value.value", 42.0f);
	tDesc.iTrackColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(227, 234, 242, 255));
	tDesc.iFillColor = __uiDesignColor(pNode, "appearance.fillColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iFillTextColor = __uiDesignColor(pNode, "appearance.fillTextColor", XUI_COLOR_WHITE);
	tDesc.iTextFlags = __uiDesignInt(pNode, "text.flags", XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	tDesc.iFillDirection = __uiDesignInt(pNode, "behavior.fillDirection", XUI_PROGRESS_LEFT_TO_RIGHT);
	tDesc.iFillPatchMode = __uiDesignInt(pNode, "behavior.fillPatchMode", XUI_PROGRESS_FILL_STRETCH);
	iRet = xuiProgressCreate(pApp->pContext, ppWidget, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	return __uiDesignApplyProgressPatches(pApp, pNode, *ppWidget);
}

static int __uiDesignCreateStepBar(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_step_bar_desc_t tDesc;
	const char* arrSteps[XUI_STEP_BAR_MAX_STEPS];
	int iStepCount;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignTextPoolReset(pNode);
	iStepCount = __uiDesignParseTextLines(pNode, "data.steps", g_arrDefaultStepTitles, UI_DESIGN_COUNT_OF(g_arrDefaultStepTitles), arrSteps, UI_DESIGN_COUNT_OF(arrSteps));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.ppTitles = arrSteps;
	tDesc.iStepCount = iStepCount;
	tDesc.iCurrent = __uiDesignInt(pNode, "data.current", 1);
	tDesc.iStyle = __uiDesignInt(pNode, "behavior.style", XUI_STEP_BAR_STYLE_DOT);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.iDoneColor = __uiDesignColor(pNode, "appearance.doneColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(214, 72, 86, 255));
	tDesc.iPendingColor = __uiDesignColor(pNode, "appearance.pendingColor", XUI_COLOR_RGBA(184, 197, 211, 255));
	tDesc.iLineColor = __uiDesignColor(pNode, "appearance.lineColor", XUI_COLOR_RGBA(213, 222, 232, 255));
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iActiveTextColor = __uiDesignColor(pNode, "appearance.activeTextColor", XUI_COLOR_RGBA(214, 72, 86, 255));
	tDesc.iPendingTextColor = __uiDesignColor(pNode, "appearance.pendingTextColor", XUI_COLOR_RGBA(107, 127, 149, 255));
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(255, 255, 255, 0));
	tDesc.fBarHeight = __uiDesignFloat(pNode, "metrics.barHeight", 4.0f);
	tDesc.fDotRadius = __uiDesignFloat(pNode, "metrics.dotRadius", 8.0f);
	tDesc.fLineWidth = __uiDesignFloat(pNode, "metrics.lineWidth", 2.0f);
	return xuiStepBarCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateChart(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_chart_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.sTitle = pNode->sText;
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iPlotColor = __uiDesignColor(pNode, "appearance.plotColor", XUI_COLOR_RGBA(247, 250, 254, 255));
	tDesc.iGridColor = __uiDesignColor(pNode, "appearance.gridColor", XUI_COLOR_RGBA(217, 227, 239, 255));
	tDesc.iAxisColor = __uiDesignColor(pNode, "appearance.axisColor", XUI_COLOR_RGBA(107, 127, 149, 255));
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iTooltipColor = __uiDesignColor(pNode, "appearance.tooltipColor", XUI_COLOR_RGBA(35, 50, 70, 235));
	tDesc.iTooltipTextColor = __uiDesignColor(pNode, "appearance.tooltipTextColor", XUI_COLOR_WHITE);
	return xuiChartCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateScrollBar(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_scrollbar_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fMin = __uiDesignFloat(pNode, "value.min", 0.0f);
	tDesc.fMax = __uiDesignFloat(pNode, "value.max", 100.0f);
	tDesc.fValue = __uiDesignFloat(pNode, "value.value", 30.0f);
	tDesc.fPage = __uiDesignFloat(pNode, "value.page", 20.0f);
	tDesc.fSmallStep = __uiDesignFloat(pNode, "value.smallStep", 1.0f);
	tDesc.fLargeStep = __uiDesignFloat(pNode, "value.largeStep", 10.0f);
	tDesc.iOrientation = __uiDesignInt(pNode, "behavior.orientation", XUI_ORIENTATION_HORIZONTAL);
	tDesc.iMode = __uiDesignInt(pNode, "behavior.mode", XUI_SCROLLBAR_MODE_FULL);
	tDesc.iButtonMode = __uiDesignInt(pNode, "behavior.buttonMode", XUI_SCROLLBAR_BUTTONS_AUTO);
	tDesc.fThickness = __uiDesignFloat(pNode, "metrics.thickness", 14.0f);
	tDesc.fMinThumbSize = __uiDesignFloat(pNode, "metrics.minThumbSize", 24.0f);
	tDesc.fButtonSize = __uiDesignFloat(pNode, "metrics.buttonSize", 16.0f);
	tDesc.iTrackColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255));
	tDesc.iThumbColor = __uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255));
	tDesc.iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	tDesc.iActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(111, 137, 166, 255));
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(206, 215, 226, 255));
	tDesc.iButtonColor = __uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(239, 245, 252, 255));
	tDesc.iButtonIconColor = __uiDesignColor(pNode, "appearance.buttonIconColor", XUI_COLOR_RGBA(52, 71, 93, 255));
	return xuiScrollBarCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateSlider(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_slider_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fMin = __uiDesignFloat(pNode, "value.min", 0.0f);
	tDesc.fMax = __uiDesignFloat(pNode, "value.max", 100.0f);
	tDesc.fValue = __uiDesignFloat(pNode, "value.value", 35.0f);
	tDesc.fStep = __uiDesignFloat(pNode, "value.step", 1.0f);
	tDesc.fPageStep = __uiDesignFloat(pNode, "value.pageStep", 10.0f);
	tDesc.iOrientation = __uiDesignInt(pNode, "behavior.orientation", XUI_ORIENTATION_HORIZONTAL);
	tDesc.fTrackSize = __uiDesignFloat(pNode, "metrics.trackSize", 4.0f);
	tDesc.fKnobSize = __uiDesignFloat(pNode, "metrics.knobSize", 16.0f);
	tDesc.fTrackRadius = __uiDesignFloat(pNode, "metrics.trackRadius", 2.0f);
	tDesc.iTrackColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(213, 222, 232, 255));
	tDesc.iFillColor = __uiDesignColor(pNode, "appearance.fillColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iKnobColor = __uiDesignColor(pNode, "appearance.knobColor", XUI_COLOR_WHITE);
	tDesc.iKnobBorderColor = __uiDesignColor(pNode, "appearance.knobBorderColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(183, 195, 208, 255));
	return xuiSliderCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateRangeSlider(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_range_slider_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fMin = __uiDesignFloat(pNode, "value.min", 0.0f);
	tDesc.fMax = __uiDesignFloat(pNode, "value.max", 100.0f);
	tDesc.fStart = __uiDesignFloat(pNode, "value.start", 25.0f);
	tDesc.fEnd = __uiDesignFloat(pNode, "value.end", 75.0f);
	tDesc.fStep = __uiDesignFloat(pNode, "value.step", 1.0f);
	tDesc.fPageStep = __uiDesignFloat(pNode, "value.pageStep", 10.0f);
	tDesc.fMinInterval = __uiDesignFloat(pNode, "value.minInterval", 0.0f);
	tDesc.fMaxInterval = __uiDesignFloat(pNode, "value.maxInterval", 0.0f);
	tDesc.iOrientation = __uiDesignInt(pNode, "behavior.orientation", XUI_ORIENTATION_HORIZONTAL);
	tDesc.fTrackSize = __uiDesignFloat(pNode, "metrics.trackSize", 4.0f);
	tDesc.fKnobSize = __uiDesignFloat(pNode, "metrics.knobSize", 16.0f);
	tDesc.iTrackColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(213, 222, 232, 255));
	tDesc.iFillColor = __uiDesignColor(pNode, "appearance.fillColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iKnobColor = __uiDesignColor(pNode, "appearance.knobColor", XUI_COLOR_WHITE);
	tDesc.iKnobBorderColor = __uiDesignColor(pNode, "appearance.knobBorderColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(183, 195, 208, 255));
	return xuiRangeSliderCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreatePage(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_page_desc_t tDesc;
	int iTotalCount;
	int iPageSize;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.iPageCount = __uiDesignInt(pNode, "data.pageCount", 12);
	if ( __uiDesignBool(pNode, "behavior.useTotal", 0) ) {
		iTotalCount = __uiDesignInt(pNode, "data.totalCount", 120);
		iPageSize = __uiDesignInt(pNode, "data.pageSize", 10);
		if ( iTotalCount < 0 ) iTotalCount = 0;
		if ( iPageSize <= 0 ) iPageSize = 1;
		tDesc.iPageCount = (iTotalCount + iPageSize - 1) / iPageSize;
		if ( tDesc.iPageCount < 1 ) tDesc.iPageCount = 1;
	}
	tDesc.iCurrentPage = __uiDesignInt(pNode, "data.current", 1);
	tDesc.iWindowSize = __uiDesignInt(pNode, "data.windowSize", 5);
	tDesc.sFirstText = __uiDesignText(pNode, "text.first", "<<");
	tDesc.sLastText = __uiDesignText(pNode, "text.last", ">>");
	tDesc.sPrevText = __uiDesignText(pNode, "text.prev", "<");
	tDesc.sNextText = __uiDesignText(pNode, "text.next", ">");
	tDesc.fItemHeight = __uiDesignFloat(pNode, "metrics.itemHeight", 28.0f);
	tDesc.fPageWidth = __uiDesignFloat(pNode, "metrics.pageWidth", 28.0f);
	tDesc.fTextWidth = __uiDesignFloat(pNode, "metrics.textWidth", 42.0f);
	tDesc.fNavWidth = __uiDesignFloat(pNode, "metrics.navWidth", 36.0f);
	tDesc.fEllipsisWidth = __uiDesignFloat(pNode, "metrics.ellipsisWidth", 24.0f);
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255));
	tDesc.iActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(217, 234, 254, 255));
	tDesc.iCurrentColor = __uiDesignColor(pNode, "appearance.currentColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iCurrentTextColor = __uiDesignColor(pNode, "appearance.currentTextColor", XUI_COLOR_WHITE);
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	return xuiPageCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateCarousel(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_carousel_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.iPageCount = __uiDesignInt(pNode, "data.pageCount", 3);
	tDesc.iCurrent = __uiDesignInt(pNode, "data.current", 0);
	tDesc.bLoop = __uiDesignBool(pNode, "behavior.loop", 1);
	tDesc.bAutoPlay = __uiDesignBool(pNode, "behavior.autoPlay", 0);
	tDesc.fAutoInterval = __uiDesignFloat(pNode, "behavior.autoInterval", 3.0f);
	tDesc.bShowIndicators = __uiDesignBool(pNode, "behavior.showIndicators", 1);
	tDesc.bShowArrowsOnHover = __uiDesignBool(pNode, "behavior.arrowsOnHover", 1);
	tDesc.fArrowSize = __uiDesignFloat(pNode, "metrics.arrowSize", 32.0f);
	tDesc.fIndicatorSize = __uiDesignFloat(pNode, "metrics.indicatorSize", 8.0f);
	tDesc.fIndicatorGap = __uiDesignFloat(pNode, "metrics.indicatorGap", 8.0f);
	tDesc.fIndicatorBottom = __uiDesignFloat(pNode, "metrics.indicatorBottom", 12.0f);
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 250, 254, 255));
	tDesc.iArrowColor = __uiDesignColor(pNode, "appearance.arrowColor", XUI_COLOR_RGBA(90, 109, 130, 170));
	tDesc.iArrowHoverColor = __uiDesignColor(pNode, "appearance.arrowHoverColor", XUI_COLOR_RGBA(52, 71, 93, 204));
	tDesc.iArrowTextColor = __uiDesignColor(pNode, "appearance.arrowTextColor", XUI_COLOR_WHITE);
	tDesc.iIndicatorColor = __uiDesignColor(pNode, "appearance.indicatorColor", XUI_COLOR_RGBA(170, 184, 200, 255));
	tDesc.iIndicatorActiveColor = __uiDesignColor(pNode, "appearance.indicatorActiveColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iIndicatorHoverColor = __uiDesignColor(pNode, "appearance.indicatorHoverColor", XUI_COLOR_RGBA(109, 164, 232, 255));
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	return xuiCarouselCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateComboBox(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_combobox_desc_t tDesc;
	xui_combobox_item_t arrItems[XUI_COMBOBOX_ITEM_CAPACITY];
	int iItemCount;
	int iSelectedIndex;
	int iSelectedValue;
	int bUseValue;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignTextPoolReset(pNode);
	iItemCount = __uiDesignBuildComboBoxItems(pNode, arrItems, UI_DESIGN_COUNT_OF(arrItems), &iSelectedIndex, &iSelectedValue);
	bUseValue = __uiDesignBool(pNode, "behavior.useValue", 0);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItemData = arrItems;
	tDesc.iItemCount = iItemCount;
	tDesc.iSelected = (iSelectedIndex >= 0) ? iSelectedIndex : __uiDesignInt(pNode, "data.selected", 0);
	tDesc.bUseValue = bUseValue;
	tDesc.iSelectedValue = (iSelectedIndex >= 0) ? iSelectedValue : __uiDesignInt(pNode, "data.selectedValue", 0);
	tDesc.sText = __uiDesignText(pNode, "text.inputText", "");
	tDesc.sPlaceholder = __uiDesignText(pNode, "text.placeholder", "Select or type");
	tDesc.iMaxLength = __uiDesignInt(pNode, "text.maxLength", 0);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.iMode = __uiDesignInt(pNode, "behavior.mode", XUI_COMBOBOX_MODE_SELECT);
	tDesc.iPopupPlacement = __uiDesignInt(pNode, "behavior.popupPlacement", XUI_COMBOBOX_POPUP_AUTO);
	tDesc.fItemHeight = __uiDesignFloat(pNode, "metrics.itemHeight", 24.0f);
	tDesc.fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	tDesc.fPopupHeight = __uiDesignFloat(pNode, "metrics.popupHeight", 0.0f);
	tDesc.fPopupMaxHeight = __uiDesignFloat(pNode, "metrics.popupMaxHeight", 240.0f);
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iHoverBackgroundColor = __uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(240, 246, 253, 255));
	tDesc.iOpenBackgroundColor = __uiDesignColor(pNode, "appearance.openBackgroundColor", XUI_COLOR_RGBA(231, 241, 252, 255));
	tDesc.iDisabledBackgroundColor = __uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(238, 242, 247, 255));
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iHoverBorderColor = __uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(126, 164, 207, 255));
	tDesc.iFocusBorderColor = __uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iArrowColor = __uiDesignColor(pNode, "appearance.arrowColor", XUI_COLOR_RGBA(52, 71, 93, 255));
	tDesc.iDisabledArrowColor = __uiDesignColor(pNode, "appearance.disabledArrowColor", XUI_COLOR_RGBA(154, 168, 186, 255));
	tDesc.iButtonColor = __uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(236, 246, 253, 255));
	tDesc.iButtonHoverColor = __uiDesignColor(pNode, "appearance.buttonHoverColor", XUI_COLOR_RGBA(220, 238, 251, 255));
	tDesc.iButtonOpenColor = __uiDesignColor(pNode, "appearance.buttonOpenColor", XUI_COLOR_RGBA(207, 229, 247, 255));
	tDesc.iPopupPanelColor = __uiDesignColor(pNode, "appearance.popupPanelColor", XUI_COLOR_RGBA(250, 253, 255, 255));
	tDesc.iPopupBorderColor = __uiDesignColor(pNode, "appearance.popupBorderColor", XUI_COLOR_RGBA(122, 164, 202, 255));
	tDesc.iPopupShadowColor = __uiDesignColor(pNode, "appearance.popupShadowColor", XUI_COLOR_RGBA(44, 70, 96, 46));
	tDesc.iPopupHoverColor = __uiDesignColor(pNode, "appearance.popupHoverColor", XUI_COLOR_RGBA(54, 125, 190, 255));
	tDesc.iPopupTextColor = __uiDesignColor(pNode, "appearance.popupTextColor", XUI_COLOR_RGBA(28, 60, 94, 255));
	tDesc.iPopupHoverTextColor = __uiDesignColor(pNode, "appearance.popupHoverTextColor", XUI_COLOR_WHITE);
	tDesc.iPopupDisabledTextColor = __uiDesignColor(pNode, "appearance.popupDisabledTextColor", XUI_COLOR_RGBA(142, 152, 166, 210));
	tDesc.iPopupSeparatorColor = __uiDesignColor(pNode, "appearance.popupSeparatorColor", XUI_COLOR_RGBA(202, 218, 232, 255));
	return xuiComboBoxCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateListView(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_list_view_desc_t tDesc;
	const char* arrItems[64];
	int arrEnabled[64];
	int arrSelected[64];
	int iItemCount;
	int bExplicitSelection;
	int iRet;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignTextPoolReset(pNode);
	iItemCount = __uiDesignBuildListViewItems(pNode, arrItems, arrEnabled, arrSelected, UI_DESIGN_COUNT_OF(arrItems), &bExplicitSelection);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = arrItems;
	tDesc.arrEnabled = arrEnabled;
	tDesc.iItemCount = iItemCount;
	tDesc.iSelected = bExplicitSelection ? -1 : __uiDesignInt(pNode, "data.selected", 0);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.fItemHeight = __uiDesignFloat(pNode, "metrics.itemHeight", 24.0f);
	tDesc.fPadding = __uiDesignFloat(pNode, "metrics.padding", 8.0f);
	tDesc.fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	tDesc.iSelectionMode = __uiDesignInt(pNode, "behavior.selectionMode", XUI_SELECTION_SINGLE);
	tDesc.iScrollbarMode = __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL);
	tDesc.bNotifyRepeatSelect = __uiDesignBool(pNode, "behavior.repeatSelect", 0);
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iRowColor = __uiDesignColor(pNode, "appearance.rowColor", XUI_COLOR_WHITE);
	tDesc.iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255));
	tDesc.iSelectedColor = __uiDesignColor(pNode, "appearance.selectedColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iTrackColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255));
	tDesc.iThumbColor = __uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255));
	tDesc.iScrollbarHoverColor = __uiDesignColor(pNode, "appearance.scrollbarHoverColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	tDesc.iScrollbarActiveColor = __uiDesignColor(pNode, "appearance.scrollbarActiveColor", XUI_COLOR_RGBA(111, 137, 166, 255));
	tDesc.iScrollbarFocusColor = __uiDesignColor(pNode, "appearance.scrollbarFocusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iScrollbarDisabledColor = __uiDesignColor(pNode, "appearance.scrollbarDisabledColor", XUI_COLOR_RGBA(206, 215, 226, 255));
	iRet = xuiListViewCreate(pApp->pContext, ppWidget, &tDesc);
	if ( (iRet == XUI_OK) && (*ppWidget != NULL) && bExplicitSelection ) {
		(void)xuiListViewClearSelection(*ppWidget);
		for ( i = 0; i < iItemCount; ++i ) {
			if ( arrSelected[i] ) (void)xuiListViewSetItemSelected(*ppWidget, i, 1);
		}
	}
	return iRet;
}

static int __uiDesignCreateTreeView(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_tree_view_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.fItemHeight = __uiDesignFloat(pNode, "metrics.itemHeight", 23.0f);
	tDesc.fIndent = __uiDesignFloat(pNode, "metrics.indent", 18.0f);
	tDesc.fPadding = __uiDesignFloat(pNode, "metrics.padding", 8.0f);
	tDesc.fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	tDesc.iSelectedId = __uiDesignInt(pNode, "data.selected", 1);
	tDesc.iScrollbarMode = __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL);
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iRowColor = __uiDesignColor(pNode, "appearance.rowColor", XUI_COLOR_WHITE);
	tDesc.iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255));
	tDesc.iSelectedColor = __uiDesignColor(pNode, "appearance.selectedColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iExpanderColor = __uiDesignColor(pNode, "appearance.expanderColor", XUI_COLOR_RGBA(82, 101, 122, 255));
	tDesc.iIconColor = __uiDesignColor(pNode, "appearance.iconColor", XUI_COLOR_RGBA(122, 142, 165, 255));
	tDesc.iCheckColor = __uiDesignColor(pNode, "appearance.checkColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iTrackColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255));
	tDesc.iThumbColor = __uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255));
	tDesc.iScrollbarHoverColor = __uiDesignColor(pNode, "appearance.scrollbarHoverColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	tDesc.iScrollbarActiveColor = __uiDesignColor(pNode, "appearance.scrollbarActiveColor", XUI_COLOR_RGBA(111, 137, 166, 255));
	tDesc.iScrollbarFocusColor = __uiDesignColor(pNode, "appearance.scrollbarFocusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iScrollbarDisabledColor = __uiDesignColor(pNode, "appearance.scrollbarDisabledColor", XUI_COLOR_RGBA(206, 215, 226, 255));
	return xuiTreeViewCreate(pApp->pContext, ppWidget, &tDesc);
}

static void __uiDesignFillTableColors(const ui_design_node_t* pNode, xui_table_view_colors_t* pColors)
{
	if ( pColors == NULL ) return;
	memset(pColors, 0, sizeof(*pColors));
	pColors->iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	pColors->iHeaderColor = __uiDesignColor(pNode, "appearance.headerColor", XUI_COLOR_RGBA(238, 245, 252, 255));
	pColors->iHeaderTextColor = __uiDesignColor(pNode, "appearance.headerTextColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	pColors->iRowColor = __uiDesignColor(pNode, "appearance.rowColor", XUI_COLOR_WHITE);
	pColors->iAltRowColor = __uiDesignColor(pNode, "appearance.altRowColor", XUI_COLOR_RGBA(249, 252, 255, 255));
	pColors->iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(231, 243, 253, 255));
	pColors->iSelectedColor = __uiDesignColor(pNode, "appearance.selectedColor", XUI_COLOR_RGBA(216, 234, 254, 255));
	pColors->iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(238, 244, 250, 255));
	pColors->iGridColor = __uiDesignColor(pNode, "appearance.gridColor", XUI_COLOR_RGBA(208, 219, 232, 255));
	pColors->iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	pColors->iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	pColors->iFocusRingColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	pColors->iBarColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255));
	pColors->iThumbColor = __uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255));
}

static int __uiDesignCreateTableView(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_table_view_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.fDefaultColumnWidth = __uiDesignFloat(pNode, "metrics.columnWidth", 100.0f);
	tDesc.fDefaultRowHeight = __uiDesignFloat(pNode, "metrics.rowHeight", 24.0f);
	tDesc.fHeaderHeight = __uiDesignFloat(pNode, "metrics.headerHeight", 26.0f);
	tDesc.iSelectionMode = __uiDesignInt(pNode, "behavior.selectionMode", XUI_TABLE_VIEW_SELECTION_ROW);
	tDesc.iSelectedRow = __uiDesignInt(pNode, "data.selectedRow", 0);
	tDesc.iSelectedColumn = __uiDesignInt(pNode, "data.selectedColumn", -1);
	tDesc.iScrollbarMode = __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL);
	__uiDesignFillTableColors(pNode, &tDesc.tColors);
	tDesc.iTrackColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255));
	tDesc.iThumbColor = __uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255));
	tDesc.iScrollbarHoverColor = __uiDesignColor(pNode, "appearance.scrollbarHoverColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	tDesc.iScrollbarActiveColor = __uiDesignColor(pNode, "appearance.scrollbarActiveColor", XUI_COLOR_RGBA(111, 137, 166, 255));
	tDesc.iScrollbarFocusColor = __uiDesignColor(pNode, "appearance.scrollbarFocusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iScrollbarDisabledColor = __uiDesignColor(pNode, "appearance.scrollbarDisabledColor", XUI_COLOR_RGBA(206, 215, 226, 255));
	return xuiTableViewCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateTableGrid(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_table_grid_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.fDefaultColumnWidth = __uiDesignFloat(pNode, "metrics.columnWidth", 100.0f);
	tDesc.fDefaultRowHeight = __uiDesignFloat(pNode, "metrics.rowHeight", 24.0f);
	tDesc.fHeaderHeight = __uiDesignFloat(pNode, "metrics.headerHeight", 26.0f);
	tDesc.iSelectionMode = __uiDesignInt(pNode, "behavior.selectionMode", XUI_TABLE_VIEW_SELECTION_ROW);
	tDesc.iEditMode = __uiDesignInt(pNode, "behavior.editMode", XUI_TABLE_GRID_EDIT_QUICK);
	tDesc.iScrollbarMode = __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL);
	tDesc.onEditorConfig = __uiDesignTableGridEditorConfig;
	tDesc.pEditorConfigUser = pNode;
	__uiDesignFillTableColors(pNode, &tDesc.tColors);
	tDesc.iTrackColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255));
	tDesc.iThumbColor = __uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255));
	tDesc.iScrollbarHoverColor = __uiDesignColor(pNode, "appearance.scrollbarHoverColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	tDesc.iScrollbarActiveColor = __uiDesignColor(pNode, "appearance.scrollbarActiveColor", XUI_COLOR_RGBA(111, 137, 166, 255));
	tDesc.iScrollbarFocusColor = __uiDesignColor(pNode, "appearance.scrollbarFocusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iScrollbarDisabledColor = __uiDesignColor(pNode, "appearance.scrollbarDisabledColor", XUI_COLOR_RGBA(206, 215, 226, 255));
	return xuiTableGridCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateQrCode(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_qrcode_desc_t tDesc;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sValue = __uiDesignText(pNode, "data.value", "https://xui.local");
	tDesc.iForegroundColor = __uiDesignColor(pNode, "appearance.foregroundColor", XUI_COLOR_RGBA(17, 24, 39, 255));
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.fPadding = __uiDesignFloat(pNode, "metrics.padding", 8.0f);
	tDesc.iMinVersion = __uiDesignInt(pNode, "behavior.minVersion", XUI_QRCODE_MIN_VERSION);
	tDesc.iMaxVersion = __uiDesignInt(pNode, "behavior.maxVersion", XUI_QRCODE_MAX_VERSION);
	iRet = xuiQrCodeCreate(pApp->pContext, ppWidget, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_OK;
}

static int __uiDesignLoadRuntimeSurfaceSlot(struct ui_design_app_t* pApp, ui_design_node_t* pNode, int iSlot, const char* sSource, xui_surface* ppSurface)
{
	xui_surface pSurface;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppSurface == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (iSlot < 0) || (iSlot >= UI_DESIGN_RUNTIME_SURFACE_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( sSource == NULL ) sSource = "";
	*ppSurface = NULL;
	if ( strcmp(sSource, pNode->arrRuntimeSurfaceSource[iSlot]) == 0 ) {
		*ppSurface = pNode->arrRuntimeSurface[iSlot];
		return XUI_OK;
	}
	if ( pNode->arrRuntimeSurface[iSlot] != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pNode->arrRuntimeSurface[iSlot]);
		pNode->arrRuntimeSurface[iSlot] = NULL;
		pNode->arrRuntimeSurfaceSource[iSlot][0] = '\0';
	}
	if ( sSource[0] == '\0' || pApp->tProxy.surfaceLoadFile == NULL ) {
		return XUI_OK;
	}
	pSurface = NULL;
	iRet = pApp->tProxy.surfaceLoadFile(&pApp->tProxy, &pSurface, sSource, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	if ( iRet != XUI_OK ) {
		return XUI_OK;
	}
	pNode->arrRuntimeSurface[iSlot] = pSurface;
	snprintf(pNode->arrRuntimeSurfaceSource[iSlot], sizeof(pNode->arrRuntimeSurfaceSource[iSlot]), "%s", sSource);
	*ppSurface = pNode->arrRuntimeSurface[iSlot];
	return XUI_OK;
}

static int __uiDesignLoadRuntimeSurface(struct ui_design_app_t* pApp, ui_design_node_t* pNode, const char* sSource, xui_surface* ppSurface)
{
	xui_surface pSurface;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppSurface == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( sSource == NULL ) sSource = "";
	*ppSurface = NULL;
	if ( strcmp(sSource, pNode->sRuntimeSurfaceSource) == 0 ) {
		*ppSurface = pNode->pRuntimeSurface;
		return XUI_OK;
	}
	if ( pNode->pRuntimeSurface != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pNode->pRuntimeSurface);
		pNode->pRuntimeSurface = NULL;
		pNode->sRuntimeSurfaceSource[0] = '\0';
	}
	if ( sSource[0] == '\0' || pApp->tProxy.surfaceLoadFile == NULL ) {
		return XUI_OK;
	}
	pSurface = NULL;
	iRet = pApp->tProxy.surfaceLoadFile(&pApp->tProxy, &pSurface, sSource, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	if ( iRet != XUI_OK ) {
		return XUI_OK;
	}
	pNode->pRuntimeSurface = pSurface;
	snprintf(pNode->sRuntimeSurfaceSource, sizeof(pNode->sRuntimeSurfaceSource), "%s", sSource);
	*ppSurface = pNode->pRuntimeSurface;
	return XUI_OK;
}

static void __uiDesignReadPrefixedSourceRect(const ui_design_node_t* pNode, const char* sPrefix, xui_rect_t* pSrc)
{
	char sId[64];
	float fX;
	float fY;
	float fW;
	float fH;

	if ( pSrc == NULL ) return;
	memset(pSrc, 0, sizeof(*pSrc));
	if ( (pNode == NULL) || (sPrefix == NULL) || (sPrefix[0] == '\0') ) return;
	snprintf(sId, sizeof(sId), "%s.x", sPrefix);
	fX = __uiDesignFloat(pNode, sId, 0.0f);
	snprintf(sId, sizeof(sId), "%s.y", sPrefix);
	fY = __uiDesignFloat(pNode, sId, 0.0f);
	snprintf(sId, sizeof(sId), "%s.w", sPrefix);
	fW = __uiDesignFloat(pNode, sId, 0.0f);
	snprintf(sId, sizeof(sId), "%s.h", sPrefix);
	fH = __uiDesignFloat(pNode, sId, 0.0f);
	if ( fW > 0.0f && fH > 0.0f ) {
		pSrc->fX = fX;
		pSrc->fY = fY;
		pSrc->fW = fW;
		pSrc->fH = fH;
	}
}

static int __uiDesignLoadRuntimeSurfaceRectSlot(ui_design_app_t* pApp, ui_design_node_t* pNode, int iSlot,
	const char* sSourceProperty, const char* sRectPrefix, xui_surface* ppSurface, xui_rect_t* pSrc)
{
	int iRet;

	if ( ppSurface != NULL ) *ppSurface = NULL;
	if ( pSrc != NULL ) memset(pSrc, 0, sizeof(*pSrc));
	if ( (pApp == NULL) || (pNode == NULL) || (ppSurface == NULL) || (pSrc == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, iSlot, __uiDesignText(pNode, sSourceProperty, ""), ppSurface);
	if ( iRet != XUI_OK ) return iRet;
	if ( *ppSurface != NULL ) __uiDesignReadPrefixedSourceRect(pNode, sRectPrefix, pSrc);
	return XUI_OK;
}

static float __uiDesignNonNegativeFloat(const ui_design_node_t* pNode, const char* sId, float fDefault)
{
	float fValue;

	fValue = __uiDesignFloat(pNode, sId, fDefault);
	return (fValue < 0.0f) ? 0.0f : fValue;
}

static float __uiDesignNonNegativeFloatText(const char* sText, float fDefault)
{
	float fValue;

	fValue = __uiDesignParseFloatText(sText, fDefault);
	return (fValue < 0.0f) ? 0.0f : fValue;
}

static int __uiDesignInputDecorationSideFromText(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "leading") || __uiDesignTokenIs(sText, "left") ||
	     __uiDesignTokenIs(sText, "prefix") || __uiDesignTokenIs(sText, "start") ) {
		return XUI_INPUT_DECORATION_SIDE_LEADING;
	}
	if ( __uiDesignTokenIs(sText, "trailing") || __uiDesignTokenIs(sText, "right") ||
	     __uiDesignTokenIs(sText, "suffix") || __uiDesignTokenIs(sText, "end") ) {
		return XUI_INPUT_DECORATION_SIDE_TRAILING;
	}
	if ( __uiDesignLooksIntText(sText) ) {
		iDefault = __uiDesignParseIntText(sText, iDefault);
	}
	return (iDefault == XUI_INPUT_DECORATION_SIDE_TRAILING) ? XUI_INPUT_DECORATION_SIDE_TRAILING : XUI_INPUT_DECORATION_SIDE_LEADING;
}

static int __uiDesignInputDecorationKindFromText(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "none") ) return XUI_INPUT_DECORATION_NONE;
	if ( __uiDesignTokenIs(sText, "icon") ) return XUI_INPUT_DECORATION_ICON;
	if ( __uiDesignTokenIs(sText, "text") || __uiDesignTokenIs(sText, "label") ) return XUI_INPUT_DECORATION_TEXT;
	if ( __uiDesignTokenIs(sText, "texture") || __uiDesignTokenIs(sText, "image") || __uiDesignTokenIs(sText, "surface") ) return XUI_INPUT_DECORATION_TEXTURE;
	if ( __uiDesignTokenIs(sText, "clear") || __uiDesignTokenIs(sText, "clearButton") ) return XUI_INPUT_DECORATION_CLEAR;
	if ( __uiDesignTokenIs(sText, "customPaint") || __uiDesignTokenIs(sText, "custom") ) return XUI_INPUT_DECORATION_CUSTOM_PAINT;
	if ( !__uiDesignLooksIntText(sText) ) return iDefault;
	iValue = __uiDesignParseIntText(sText, iDefault);
	if ( iValue < XUI_INPUT_DECORATION_NONE || iValue > XUI_INPUT_DECORATION_CUSTOM_PAINT ) return iDefault;
	return iValue;
}

static int __uiDesignInputDecorationVisibleFromText(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "always") || __uiDesignTokenIs(sText, "visible") ) return XUI_INPUT_DECORATION_VISIBLE_ALWAYS;
	if ( __uiDesignTokenIs(sText, "notEmpty") || __uiDesignTokenIs(sText, "not_empty") ||
	     __uiDesignTokenIs(sText, "filled") ) return XUI_INPUT_DECORATION_VISIBLE_NOT_EMPTY;
	if ( __uiDesignTokenIs(sText, "focused") || __uiDesignTokenIs(sText, "focus") ) return XUI_INPUT_DECORATION_VISIBLE_FOCUSED;
	if ( __uiDesignTokenIs(sText, "focusedNotEmpty") || __uiDesignTokenIs(sText, "focusNotEmpty") ||
	     __uiDesignTokenIs(sText, "focused_not_empty") || __uiDesignTokenIs(sText, "focus_not_empty") ) {
		return XUI_INPUT_DECORATION_VISIBLE_FOCUSED_NOT_EMPTY;
	}
	if ( !__uiDesignLooksIntText(sText) ) return iDefault;
	iValue = __uiDesignParseIntText(sText, iDefault);
	if ( iValue < XUI_INPUT_DECORATION_VISIBLE_ALWAYS || iValue > XUI_INPUT_DECORATION_VISIBLE_FOCUSED_NOT_EMPTY ) return iDefault;
	return iValue;
}

static int __uiDesignInputDecorationIconFromText(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "none") ) return XUI_INPUT_ICON_NONE;
	if ( __uiDesignTokenIs(sText, "search") ) return XUI_INPUT_ICON_SEARCH;
	if ( __uiDesignTokenIs(sText, "user") ) return XUI_INPUT_ICON_USER;
	if ( __uiDesignTokenIs(sText, "lock") ) return XUI_INPUT_ICON_LOCK;
	if ( __uiDesignTokenIs(sText, "eye") ) return XUI_INPUT_ICON_EYE;
	if ( !__uiDesignLooksIntText(sText) ) return iDefault;
	iValue = __uiDesignParseIntText(sText, iDefault);
	if ( iValue < XUI_INPUT_ICON_NONE || iValue > XUI_INPUT_ICON_EYE ) return iDefault;
	return iValue;
}

static int __uiDesignInputDecorationInferKind(int iKind, int iIcon, const char* sText, const char* sSource)
{
	if ( iKind >= XUI_INPUT_DECORATION_NONE && iKind <= XUI_INPUT_DECORATION_CUSTOM_PAINT ) return iKind;
	if ( (sSource != NULL) && (sSource[0] != 0) ) return XUI_INPUT_DECORATION_TEXTURE;
	if ( iIcon != XUI_INPUT_ICON_NONE ) return XUI_INPUT_DECORATION_ICON;
	if ( (sText != NULL) && (sText[0] != 0) ) return XUI_INPUT_DECORATION_TEXT;
	return XUI_INPUT_DECORATION_NONE;
}

static int __uiDesignApplyInputDecorations(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pWidget)
{
	const char* sCursor;
	const char* sSource;
	const char* sText;
	char sLine[1024];
	char* arrFields[16];
	xui_input_decoration_desc_t tDesc;
	xui_surface pSurface;
	xui_rect_t tSrc;
	int iFieldCount;
	int iCount;
	int iSide;
	int iKind;
	int i;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	(void)xuiInputDecorationClear(pWidget, XUI_INPUT_DECORATION_SIDE_LEADING);
	(void)xuiInputDecorationClear(pWidget, XUI_INPUT_DECORATION_SIDE_TRAILING);
	__uiDesignTextPoolReset(pNode);
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.decorations", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < UI_DESIGN_RUNTIME_SURFACE_COUNT ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount <= 0) || (arrFields[0][0] == 0) ) continue;
		memset(&tDesc, 0, sizeof(tDesc));
		tDesc.iSize = sizeof(tDesc);
		iSide = __uiDesignInputDecorationSideFromText(__uiDesignField(arrFields, iFieldCount, 0, ""), XUI_INPUT_DECORATION_SIDE_LEADING);
		tDesc.iVisibleMode = __uiDesignInputDecorationVisibleFromText(__uiDesignField(arrFields, iFieldCount, 2, ""), XUI_INPUT_DECORATION_VISIBLE_ALWAYS);
		tDesc.fWidth = __uiDesignNonNegativeFloatText(__uiDesignField(arrFields, iFieldCount, 3, ""), 0.0f);
		tDesc.fPadding = __uiDesignNonNegativeFloatText(__uiDesignField(arrFields, iFieldCount, 4, ""), 0.0f);
		tDesc.iIcon = __uiDesignInputDecorationIconFromText(__uiDesignField(arrFields, iFieldCount, 5, ""), XUI_INPUT_ICON_NONE);
		sText = __uiDesignField(arrFields, iFieldCount, 6, "");
		tDesc.sText = (sText[0] != 0) ? __uiDesignTextPoolAdd(pNode, sText) : NULL;
		sSource = __uiDesignField(arrFields, iFieldCount, 7, "");
		iKind = __uiDesignInputDecorationKindFromText(__uiDesignField(arrFields, iFieldCount, 1, ""), -1);
		tDesc.iKind = __uiDesignInputDecorationInferKind(iKind, tDesc.iIcon, sText, sSource);
		if ( sSource[0] != 0 ) {
			pSurface = NULL;
			iRet = __uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, iCount, sSource, &pSurface);
			if ( iRet != XUI_OK ) return iRet;
			tDesc.pSurface = pSurface;
			memset(&tSrc, 0, sizeof(tSrc));
			if ( pSurface != NULL ) {
				tSrc.fX = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 8, ""), 0.0f);
				tSrc.fY = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 9, ""), 0.0f);
				tSrc.fW = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 10, ""), 0.0f);
				tSrc.fH = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 11, ""), 0.0f);
				if ( tSrc.fW <= 0.0f || tSrc.fH <= 0.0f ) memset(&tSrc, 0, sizeof(tSrc));
			}
			tDesc.tSrc = tSrc;
		} else {
			pSurface = NULL;
			(void)__uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, iCount, "", &pSurface);
		}
		tDesc.iColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 12, ""), 0u);
		tDesc.iHoverColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 13, ""), 0u);
		tDesc.iActiveColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 14, ""), 0u);
		tDesc.iDisabledColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 15, ""), 0u);
		iRet = xuiInputDecorationAdd(pWidget, iSide, NULL, &tDesc);
		if ( iRet != XUI_OK ) return iRet;
		++iCount;
	}
	for ( i = iCount; i < UI_DESIGN_RUNTIME_SURFACE_COUNT; ++i ) {
		pSurface = NULL;
		(void)__uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, i, "", &pSurface);
	}
	return XUI_OK;
}

static int __uiDesignButtonPatchStateFromText(const char* sText, uint32_t* pState, int* pIndex)
{
	int iValue;

	if ( (pState == NULL) || (pIndex == NULL) || (sText == NULL) || (sText[0] == 0) ) return 0;
	if ( __uiDesignTokenIs(sText, "normal") || __uiDesignTokenIs(sText, "default") || __uiDesignTokenIs(sText, "base") ) {
		*pState = 0u;
		*pIndex = 0;
		return 1;
	}
	if ( __uiDesignTokenIs(sText, "hover") ) {
		*pState = XUI_WIDGET_STATE_HOVER;
		*pIndex = 1;
		return 1;
	}
	if ( __uiDesignTokenIs(sText, "active") || __uiDesignTokenIs(sText, "pressed") ) {
		*pState = XUI_WIDGET_STATE_ACTIVE;
		*pIndex = 2;
		return 1;
	}
	if ( __uiDesignTokenIs(sText, "focus") || __uiDesignTokenIs(sText, "focused") ) {
		*pState = XUI_WIDGET_STATE_FOCUS;
		*pIndex = 3;
		return 1;
	}
	if ( __uiDesignTokenIs(sText, "disabled") ) {
		*pState = XUI_WIDGET_STATE_DISABLED;
		*pIndex = 4;
		return 1;
	}
	if ( __uiDesignTokenIs(sText, "checked") || __uiDesignTokenIs(sText, "selected") ) {
		*pState = XUI_BUTTON_STATE_CHECKED;
		*pIndex = 5;
		return 1;
	}
	if ( !__uiDesignLooksIntText(sText) ) return 0;
	iValue = __uiDesignParseIntText(sText, -1);
	if ( iValue == 0 ) {
		*pState = 0u;
		*pIndex = 0;
		return 1;
	}
	if ( iValue == (int)XUI_WIDGET_STATE_HOVER ) {
		*pState = XUI_WIDGET_STATE_HOVER;
		*pIndex = 1;
		return 1;
	}
	if ( iValue == (int)XUI_WIDGET_STATE_ACTIVE ) {
		*pState = XUI_WIDGET_STATE_ACTIVE;
		*pIndex = 2;
		return 1;
	}
	if ( iValue == (int)XUI_WIDGET_STATE_FOCUS ) {
		*pState = XUI_WIDGET_STATE_FOCUS;
		*pIndex = 3;
		return 1;
	}
	if ( iValue == (int)XUI_WIDGET_STATE_DISABLED ) {
		*pState = XUI_WIDGET_STATE_DISABLED;
		*pIndex = 4;
		return 1;
	}
	if ( iValue == (int)XUI_BUTTON_STATE_CHECKED ) {
		*pState = XUI_BUTTON_STATE_CHECKED;
		*pIndex = 5;
		return 1;
	}
	switch ( iValue ) {
	case 3:
		*pState = XUI_WIDGET_STATE_FOCUS;
		*pIndex = 3;
		return 1;
	case 5:
		*pState = XUI_BUTTON_STATE_CHECKED;
		*pIndex = 5;
		return 1;
	default:
		break;
	}
	return 0;
}

static int __uiDesignNinePatchModeFromText(const char* sText)
{
	if ( __uiDesignTokenIs(sText, "tile") || __uiDesignTokenIs(sText, "tiled") ) return XUI_NINE_PATCH_TILE;
	if ( __uiDesignLooksIntText(sText) && __uiDesignParseIntText(sText, XUI_NINE_PATCH_STRETCH) == XUI_NINE_PATCH_TILE ) {
		return XUI_NINE_PATCH_TILE;
	}
	return XUI_NINE_PATCH_STRETCH;
}

static int __uiDesignApplyButtonPatches(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pWidget)
{
	static const uint32_t arrStates[] = {
		0u,
		XUI_WIDGET_STATE_HOVER,
		XUI_WIDGET_STATE_ACTIVE,
		XUI_WIDGET_STATE_FOCUS,
		XUI_WIDGET_STATE_DISABLED,
		XUI_BUTTON_STATE_CHECKED
	};
	const char* sCursor;
	const char* sSource;
	char sLine[1024];
	char* arrFields[12];
	xui_nine_patch_t tPatch;
	xui_surface pSurface;
	float fX;
	float fY;
	float fW;
	float fH;
	uint32_t iState;
	int iIndex;
	int iFieldCount;
	int iRet;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < UI_DESIGN_COUNT_OF(arrStates); ++i ) {
		(void)xuiButtonClearPatch(pWidget, arrStates[i]);
		pSurface = NULL;
		(void)__uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, 2 + i, "", &pSurface);
	}
	sCursor = __uiDesignText(pNode, "data.statePatches", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 || !__uiDesignButtonPatchStateFromText(arrFields[0], &iState, &iIndex) ) continue;
		sSource = __uiDesignField(arrFields, iFieldCount, 1, "");
		if ( sSource[0] == 0 ) continue;
		pSurface = NULL;
		iRet = __uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, 2 + iIndex, sSource, &pSurface);
		if ( iRet != XUI_OK ) return iRet;
		if ( pSurface == NULL ) continue;
		memset(&tPatch, 0, sizeof(tPatch));
		tPatch.iSize = sizeof(tPatch);
		tPatch.pSurface = pSurface;
		fX = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 2, ""), 0.0f);
		fY = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 3, ""), 0.0f);
		fW = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 4, ""), 0.0f);
		fH = __uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 5, ""), 0.0f);
		if ( fW > 0.0f && fH > 0.0f ) {
			tPatch.tSrc.fX = fX;
			tPatch.tSrc.fY = fY;
			tPatch.tSrc.fW = fW;
			tPatch.tSrc.fH = fH;
		}
		tPatch.tSlice.fLeft = __uiDesignNonNegativeFloatText(__uiDesignField(arrFields, iFieldCount, 6, ""), 4.0f);
		tPatch.tSlice.fTop = __uiDesignNonNegativeFloatText(__uiDesignField(arrFields, iFieldCount, 7, ""), 4.0f);
		tPatch.tSlice.fRight = __uiDesignNonNegativeFloatText(__uiDesignField(arrFields, iFieldCount, 8, ""), 4.0f);
		tPatch.tSlice.fBottom = __uiDesignNonNegativeFloatText(__uiDesignField(arrFields, iFieldCount, 9, ""), 4.0f);
		tPatch.iColor = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 10, ""), XUI_COLOR_WHITE);
		tPatch.iMode = __uiDesignNinePatchModeFromText(__uiDesignField(arrFields, iFieldCount, 11, ""));
		iRet = xuiButtonSetPatch(pWidget, iState, &tPatch);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __uiDesignLoadProgressPatch(ui_design_app_t* pApp, ui_design_node_t* pNode, int iSlot,
	const char* sSourceProperty, const char* sPatchPrefix, xui_nine_patch_t* pPatch, int* pHasPatch)
{
	char sId[80];
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pPatch == NULL) || (pHasPatch == NULL) ||
	     (sSourceProperty == NULL) || (sPatchPrefix == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pPatch, 0, sizeof(*pPatch));
	*pHasPatch = 0;
	iRet = __uiDesignLoadRuntimeSurfaceRectSlot(pApp, pNode, iSlot, sSourceProperty, sPatchPrefix, &pPatch->pSurface, &pPatch->tSrc);
	if ( iRet != XUI_OK ) return iRet;
	if ( pPatch->pSurface == NULL ) return XUI_OK;
	pPatch->iSize = sizeof(*pPatch);
	snprintf(sId, sizeof(sId), "%s.sliceLeft", sPatchPrefix);
	pPatch->tSlice.fLeft = __uiDesignNonNegativeFloat(pNode, sId, 4.0f);
	snprintf(sId, sizeof(sId), "%s.sliceTop", sPatchPrefix);
	pPatch->tSlice.fTop = __uiDesignNonNegativeFloat(pNode, sId, 4.0f);
	snprintf(sId, sizeof(sId), "%s.sliceRight", sPatchPrefix);
	pPatch->tSlice.fRight = __uiDesignNonNegativeFloat(pNode, sId, 4.0f);
	snprintf(sId, sizeof(sId), "%s.sliceBottom", sPatchPrefix);
	pPatch->tSlice.fBottom = __uiDesignNonNegativeFloat(pNode, sId, 4.0f);
	snprintf(sId, sizeof(sId), "%s.color", sPatchPrefix);
	pPatch->iColor = __uiDesignColor(pNode, sId, XUI_COLOR_WHITE);
	snprintf(sId, sizeof(sId), "%s.mode", sPatchPrefix);
	pPatch->iMode = __uiDesignInt(pNode, sId, XUI_NINE_PATCH_STRETCH);
	if ( pPatch->iMode != XUI_NINE_PATCH_TILE ) pPatch->iMode = XUI_NINE_PATCH_STRETCH;
	*pHasPatch = 1;
	return XUI_OK;
}

static int __uiDesignApplyProgressPatches(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_nine_patch_t tPatch;
	int bHasPatch;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignLoadProgressPatch(pApp, pNode, 0, "data.trackPatchSource", "trackPatch", &tPatch, &bHasPatch);
	if ( iRet != XUI_OK ) return iRet;
	iRet = bHasPatch ? xuiProgressSetTrackPatch(pWidget, &tPatch) : xuiProgressClearTrackPatch(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignLoadProgressPatch(pApp, pNode, 1, "data.fillPatchSource", "fillPatch", &tPatch, &bHasPatch);
	if ( iRet != XUI_OK ) return iRet;
	return bHasPatch ? xuiProgressSetFillPatch(pWidget, &tPatch) : xuiProgressClearFillPatch(pWidget);
}

static int __uiDesignApplyImageSurface(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_surface pSurface;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignLoadRuntimeSurface(pApp, pNode, __uiDesignText(pNode, "data.source", ""), &pSurface);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiImageSetSurface(pWidget, pSurface);
	return XUI_OK;
}

static int __uiDesignApplyQrCodeIcon(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_surface pSurface;
	xui_rect_t tIconSrc;
	float fX;
	float fY;
	float fW;
	float fH;
	float fIconSize;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignLoadRuntimeSurface(pApp, pNode, __uiDesignText(pNode, "data.iconSource", ""), &pSurface);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tIconSrc, 0, sizeof(tIconSrc));
	fIconSize = __uiDesignFloat(pNode, "metrics.iconSize", 0.0f);
	if ( pSurface != NULL ) {
		fX = __uiDesignFloat(pNode, "icon.x", 0.0f);
		fY = __uiDesignFloat(pNode, "icon.y", 0.0f);
		fW = __uiDesignFloat(pNode, "icon.w", 0.0f);
		fH = __uiDesignFloat(pNode, "icon.h", 0.0f);
		if ( fW > 0.0f && fH > 0.0f ) {
			tIconSrc.fX = fX;
			tIconSrc.fY = fY;
			tIconSrc.fW = fW;
			tIconSrc.fH = fH;
		}
		return xuiQrCodeSetIcon(pWidget, pSurface, tIconSrc, fIconSize);
	}
	return xuiQrCodeSetIcon(pWidget, NULL, tIconSrc, 0.0f);
}

static void __uiDesignApplyButtonProperties(struct ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	xui_widget pWidget;
	xui_surface pIconSurface;
	xui_surface pBadgeSurface;
	xui_rect_t tIconSrc;
	xui_rect_t tBadgeSrc;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iNormalColor;
	uint32_t iHoverColor;
	uint32_t iActiveColor;
	uint32_t iFocusColor;
	uint32_t iDisabledColor;
	uint32_t iCheckedColor;
	uint32_t iBorderColor;
	uint32_t iHoverBorderColor;
	uint32_t iActiveBorderColor;
	uint32_t iDisabledBorderColor;
	uint32_t iCheckedBorderColor;
	float fBorderWidth;
	float fHoverBorderWidth;
	float fActiveBorderWidth;
	float fFocusBorderWidth;
	float fDisabledBorderWidth;
	float fCheckedBorderWidth;
	float fX;
	float fY;
	float fW;
	float fH;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pNode->pWidget == NULL) ) return;
	pWidget = pNode->pWidget;
	iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	iNormalColor = __uiDesignColor(pNode, "appearance.normalColor", XUI_COLOR_RGBA(248, 251, 255, 255));
	iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255));
	iActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(217, 234, 254, 255));
	iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(238, 242, 247, 255));
	iCheckedColor = __uiDesignColor(pNode, "appearance.checkedColor", XUI_COLOR_RGBA(217, 234, 254, 255));
	iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	iHoverBorderColor = __uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(126, 170, 220, 255));
	iActiveBorderColor = __uiDesignColor(pNode, "appearance.activeBorderColor", XUI_COLOR_RGBA(111, 152, 199, 255));
	iDisabledBorderColor = __uiDesignColor(pNode, "appearance.disabledBorderColor", XUI_COLOR_RGBA(214, 219, 225, 255));
	iCheckedBorderColor = __uiDesignColor(pNode, "appearance.checkedBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	fBorderWidth = __uiDesignFloat(pNode, "appearance.borderWidth", 1.0f);
	fHoverBorderWidth = __uiDesignFloat(pNode, "appearance.hoverBorderWidth", 1.0f);
	fActiveBorderWidth = __uiDesignFloat(pNode, "appearance.activeBorderWidth", 1.0f);
	fFocusBorderWidth = __uiDesignFloat(pNode, "appearance.focusBorderWidth", 2.0f);
	fDisabledBorderWidth = __uiDesignFloat(pNode, "appearance.disabledBorderWidth", 1.0f);
	fCheckedBorderWidth = __uiDesignFloat(pNode, "appearance.checkedBorderWidth", 1.0f);

	(void)xuiButtonSetText(pWidget, pNode->sText);
	(void)xuiButtonSetSelectable(pWidget, __uiDesignBool(pNode, "behavior.selectable", 0));
	(void)xuiButtonSetSemantic(pWidget, __uiDesignInt(pNode, "behavior.semantic", XUI_BUTTON_SEMANTIC_DEFAULT));
	(void)xuiButtonSetTextColor(pWidget, iTextColor);
	(void)xuiButtonSetDisabledTextColor(pWidget, iDisabledTextColor);
	(void)xuiButtonSetTextFlags(pWidget, (uint32_t)__uiDesignInt(pNode, "text.flags", XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP));
	(void)xuiButtonSetStateVisual(pWidget, 0u, iNormalColor, fBorderWidth, iBorderColor);
	(void)xuiButtonSetStateVisual(pWidget, XUI_WIDGET_STATE_HOVER, iHoverColor, fHoverBorderWidth, iHoverBorderColor);
	(void)xuiButtonSetStateVisual(pWidget, XUI_WIDGET_STATE_ACTIVE, iActiveColor, fActiveBorderWidth, iActiveBorderColor);
	(void)xuiButtonSetStateVisual(pWidget, XUI_WIDGET_STATE_FOCUS, iNormalColor, fFocusBorderWidth, iFocusColor);
	(void)xuiButtonSetStateVisual(pWidget, XUI_WIDGET_STATE_DISABLED, iDisabledColor, fDisabledBorderWidth, iDisabledBorderColor);
	(void)xuiButtonSetStateVisual(pWidget, XUI_BUTTON_STATE_CHECKED, iCheckedColor, fCheckedBorderWidth, iCheckedBorderColor);
	(void)xuiButtonSetSelected(pWidget, __uiDesignBool(pNode, "behavior.selected", 0));

	memset(&tIconSrc, 0, sizeof(tIconSrc));
	iRet = __uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, 0, __uiDesignText(pNode, "data.iconSource", ""), &pIconSurface);
	if ( iRet == XUI_OK ) {
		if ( pIconSurface != NULL ) {
			fX = __uiDesignFloat(pNode, "icon.x", 0.0f);
			fY = __uiDesignFloat(pNode, "icon.y", 0.0f);
			fW = __uiDesignFloat(pNode, "icon.w", 0.0f);
			fH = __uiDesignFloat(pNode, "icon.h", 0.0f);
			if ( fW > 0.0f && fH > 0.0f ) {
				tIconSrc.fX = fX;
				tIconSrc.fY = fY;
				tIconSrc.fW = fW;
				tIconSrc.fH = fH;
			}
		}
		(void)xuiButtonSetIcon(pWidget, pIconSurface, tIconSrc);
	}
	(void)xuiButtonSetIconColor(pWidget, __uiDesignColor(pNode, "appearance.iconColor", iTextColor));
	(void)xuiButtonSetIconLayout(pWidget,
		__uiDesignInt(pNode, "behavior.iconPlacement", XUI_BUTTON_ICON_LEFT),
		__uiDesignFloat(pNode, "metrics.iconSize", 16.0f),
		__uiDesignFloat(pNode, "metrics.iconGap", 6.0f));
	(void)xuiButtonSetBadgeVisible(pWidget, __uiDesignBool(pNode, "badge.visible", 0));
	(void)xuiButtonSetBadgeAnchor(pWidget, __uiDesignInt(pNode, "badge.anchor", XUI_BUTTON_BADGE_CONTENT_TOP_RIGHT));
	(void)xuiButtonSetBadgeOffset(pWidget,
		__uiDesignFloat(pNode, "badge.offsetX", 0.0f),
		__uiDesignFloat(pNode, "badge.offsetY", 0.0f));
	(void)xuiButtonSetBadgeSize(pWidget, __uiDesignFloat(pNode, "metrics.badgeSize", 12.0f));
	memset(&tBadgeSrc, 0, sizeof(tBadgeSrc));
	iRet = __uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, 1, __uiDesignText(pNode, "data.badgeSource", ""), &pBadgeSurface);
	if ( iRet == XUI_OK ) {
		if ( pBadgeSurface != NULL ) {
			fX = __uiDesignFloat(pNode, "badge.sourceX", 0.0f);
			fY = __uiDesignFloat(pNode, "badge.sourceY", 0.0f);
			fW = __uiDesignFloat(pNode, "badge.sourceW", 0.0f);
			fH = __uiDesignFloat(pNode, "badge.sourceH", 0.0f);
			if ( fW > 0.0f && fH > 0.0f ) {
				tBadgeSrc.fX = fX;
				tBadgeSrc.fY = fY;
				tBadgeSrc.fW = fW;
				tBadgeSrc.fH = fH;
			}
		}
		(void)xuiButtonSetBadgeSurface(pWidget, pBadgeSurface, tBadgeSrc);
	}
	(void)__uiDesignApplyButtonPatches(pApp, pNode, pWidget);
}

static void __uiDesignApplyImageRects(ui_design_node_t* pNode, xui_widget pWidget)
{
	float fX;
	float fY;
	float fW;
	float fH;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	fX = __uiDesignFloat(pNode, "source.x", 0.0f);
	fY = __uiDesignFloat(pNode, "source.y", 0.0f);
	fW = __uiDesignFloat(pNode, "source.w", 0.0f);
	fH = __uiDesignFloat(pNode, "source.h", 0.0f);
	if ( fW > 0.0f && fH > 0.0f ) {
		(void)xuiImageSetSourceRect(pWidget, fX, fY, fX + fW, fY + fH);
	} else {
		(void)xuiImageSetSource(pWidget, (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f});
	}
	fX = __uiDesignFloat(pNode, "custom.x", 0.0f);
	fY = __uiDesignFloat(pNode, "custom.y", 0.0f);
	fW = __uiDesignFloat(pNode, "custom.w", 0.0f);
	fH = __uiDesignFloat(pNode, "custom.h", 0.0f);
	if ( fW > 0.0f && fH > 0.0f ) {
		(void)xuiImageSetCustomRect(pWidget, fX, fY, fX + fW, fY + fH);
	} else {
		(void)xuiImageSetCustomRect(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	}
}

static int __uiDesignCreateImage(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_image_desc_t tDesc;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iColor = __uiDesignColor(pNode, "appearance.tintColor", XUI_COLOR_WHITE);
	tDesc.iMode = __uiDesignInt(pNode, "behavior.mode", XUI_IMAGE_CONTAIN);
	tDesc.iAlignX = __uiDesignInt(pNode, "behavior.alignX", XUI_ALIGN_CENTER);
	tDesc.iAlignY = __uiDesignInt(pNode, "behavior.alignY", XUI_ALIGN_CENTER);
	iRet = xuiImageCreate(pApp->pContext, ppWidget, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)__uiDesignApplyImageSurface(pApp, pNode, *ppWidget);
	__uiDesignApplyImageRects(pNode, *ppWidget);
	(void)xuiImageSetMode(*ppWidget, __uiDesignInt(pNode, "behavior.mode", XUI_IMAGE_CONTAIN));
	return XUI_OK;
}

static int __uiDesignRadioGroupOrientation(const ui_design_node_t* pNode)
{
	int iOrientation;

	iOrientation = __uiDesignInt(pNode, "behavior.orientation", XUI_ORIENTATION_VERTICAL);
	return (iOrientation == XUI_ORIENTATION_HORIZONTAL) ? XUI_RADIO_GROUP_HORIZONTAL : XUI_RADIO_GROUP_VERTICAL;
}

static uint32_t __uiDesignCodeEditOptions(const ui_design_node_t* pNode)
{
	uint32_t iOptions;

	iOptions = 0u;
	if ( __uiDesignBool(pNode, "behavior.showWhitespace", 0) ) iOptions |= XUI_CODE_EDIT_SHOW_WHITESPACE;
	if ( __uiDesignBool(pNode, "behavior.showEol", 0) ) iOptions |= XUI_CODE_EDIT_SHOW_EOL;
	if ( __uiDesignBool(pNode, "behavior.indentGuides", 1) ) iOptions |= XUI_CODE_EDIT_SHOW_INDENT_GUIDES;
	if ( __uiDesignBool(pNode, "behavior.expandTabs", 1) ) iOptions |= XUI_CODE_EDIT_EXPAND_TABS;
	return iOptions;
}

static uint32_t __uiDesignFindFlags(const ui_design_node_t* pNode)
{
	uint32_t iFlags;

	iFlags = 0u;
	if ( __uiDesignBool(pNode, "find.caseSensitive", 0) ) iFlags |= XUI_FIND_CASE_SENSITIVE;
	if ( __uiDesignBool(pNode, "find.wholeWord", 0) ) iFlags |= XUI_FIND_WHOLE_WORD;
	if ( __uiDesignBool(pNode, "find.regex", 0) ) iFlags |= XUI_FIND_REGEX;
	if ( __uiDesignBool(pNode, "find.escape", 0) ) iFlags |= XUI_FIND_ESCAPE;
	if ( __uiDesignBool(pNode, "find.selection", 0) ) iFlags |= XUI_FIND_SELECTION;
	if ( __uiDesignBool(pNode, "find.wrap", 1) ) iFlags |= XUI_FIND_WRAP;
	if ( __uiDesignBool(pNode, "find.backward", 0) ) iFlags |= XUI_FIND_BACKWARD;
	return iFlags;
}

static void __uiDesignFillFindOptions(const ui_design_node_t* pNode, xui_find_options_t* pOptions)
{
	if ( pOptions == NULL ) return;
	memset(pOptions, 0, sizeof(*pOptions));
	pOptions->iSize = sizeof(*pOptions);
	pOptions->sPattern = __uiDesignText(pNode, "find.pattern", "");
	pOptions->sReplacement = __uiDesignText(pNode, "find.replacement", "");
	pOptions->iFlags = __uiDesignFindFlags(pNode);
	pOptions->iStartOffset = __uiDesignInt(pNode, "find.startOffset", 0);
	pOptions->iRangeStart = __uiDesignInt(pNode, "find.rangeStart", 0);
	pOptions->iRangeEnd = __uiDesignInt(pNode, "find.rangeEnd", 0);
}

static void __uiDesignApplyTextEditFindPreview(ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_find_options_t tOptions;
	const char* sPattern;
	int iMode;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	sPattern = __uiDesignText(pNode, "find.pattern", "");
	iMode = __uiDesignInt(pNode, "find.previewMode", 0);
	if ( (sPattern == NULL) || (sPattern[0] == 0) || (iMode <= 0) ) {
		(void)xuiTextEditClearFind(pWidget);
		return;
	}
	__uiDesignFillFindOptions(pNode, &tOptions);
	if ( (iMode == 2) || ((tOptions.iFlags & XUI_FIND_BACKWARD) != 0u) ) {
		(void)xuiTextEditFindPrevious(pWidget, &tOptions);
	} else {
		(void)xuiTextEditFindNext(pWidget, &tOptions);
	}
}

static void __uiDesignApplyCodeEditFindPreview(ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_find_options_t tOptions;
	const char* sPattern;
	int iResultCount;
	int iMode;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	sPattern = __uiDesignText(pNode, "find.pattern", "");
	iMode = __uiDesignInt(pNode, "find.previewMode", 0);
	if ( (sPattern == NULL) || (sPattern[0] == 0) || (iMode <= 0) ) {
		(void)xuiCodeEditClearFind(pWidget);
		return;
	}
	__uiDesignFillFindOptions(pNode, &tOptions);
	if ( iMode == 3 ) {
		iResultCount = 0;
		(void)xuiCodeEditFindAll(pWidget, &tOptions, &iResultCount);
	} else if ( (iMode == 2) || ((tOptions.iFlags & XUI_FIND_BACKWARD) != 0u) ) {
		(void)xuiCodeEditFindPrevious(pWidget, &tOptions);
	} else {
		(void)xuiCodeEditFindNext(pWidget, &tOptions);
	}
}

static void __uiDesignApplyTextEditFindWindow(xui_widget pWidget, int iMode)
{
	xui_widget pWindow;

	if ( pWidget == NULL ) return;
	if ( iMode == 1 ) {
		(void)xuiTextEditOpenFind(pWidget);
		return;
	}
	if ( iMode == 2 ) {
		(void)xuiTextEditOpenReplace(pWidget);
		return;
	}
	pWindow = xuiTextEditGetFindWindow(pWidget);
	if ( pWindow != NULL ) (void)xuiWindowSetOpen(pWindow, 0);
}

static void __uiDesignApplyCodeEditFindWindow(xui_widget pWidget, int iMode)
{
	xui_widget pWindow;

	if ( pWidget == NULL ) return;
	if ( iMode == 1 ) {
		(void)xuiCodeEditOpenFind(pWidget);
		return;
	}
	if ( iMode == 2 ) {
		(void)xuiCodeEditOpenReplace(pWidget);
		return;
	}
	pWindow = xuiCodeEditGetFindWindow(pWidget);
	if ( pWindow != NULL ) (void)xuiWindowSetOpen(pWindow, 0);
}

static void __uiDesignCodeEditSetStyle(xui_widget pWidget, int iStyleId, uint32_t iForeground, uint32_t iBackground, uint32_t iFlags)
{
	xui_code_style_t tStyle;

	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iSize = sizeof(tStyle);
	tStyle.iForeground = iForeground;
	tStyle.iBackground = iBackground;
	tStyle.iFlags = iFlags;
	(void)xuiCodeEditSetStyle(pWidget, iStyleId, &tStyle);
}

#define UI_DESIGN_CODE_THEME_STYLE_CAPACITY 128

static int __uiDesignCodeStyleIdFromText(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "default") || __uiDesignTokenIs(sText, "text") ) return XUI_CODE_STYLE_DEFAULT;
	if ( __uiDesignTokenIs(sText, "keyword") ) return XUI_CODE_STYLE_KEYWORD;
	if ( __uiDesignTokenIs(sText, "type") ) return XUI_CODE_STYLE_TYPE;
	if ( __uiDesignTokenIs(sText, "identifier") ) return XUI_CODE_STYLE_IDENTIFIER;
	if ( __uiDesignTokenIs(sText, "number") ) return XUI_CODE_STYLE_NUMBER;
	if ( __uiDesignTokenIs(sText, "string") ) return XUI_CODE_STYLE_STRING;
	if ( __uiDesignTokenIs(sText, "char") || __uiDesignTokenIs(sText, "character") ) return XUI_CODE_STYLE_CHAR;
	if ( __uiDesignTokenIs(sText, "comment") ) return XUI_CODE_STYLE_COMMENT;
	if ( __uiDesignTokenIs(sText, "preprocessor") || __uiDesignTokenIs(sText, "preproc") ) return XUI_CODE_STYLE_PREPROCESSOR;
	if ( __uiDesignTokenIs(sText, "operator") ) return XUI_CODE_STYLE_OPERATOR;
	if ( __uiDesignTokenIs(sText, "brace") || __uiDesignTokenIs(sText, "bracket") ) return XUI_CODE_STYLE_BRACE;
	if ( __uiDesignTokenIs(sText, "error") || __uiDesignTokenIs(sText, "errorText") ||
	     __uiDesignTokenIs(sText, "error_text") ) return XUI_CODE_STYLE_ERROR;
	if ( __uiDesignTokenIs(sText, "selection") || __uiDesignTokenIs(sText, "selected") ) return XUI_CODE_STYLE_SELECTION;
	if ( __uiDesignTokenIs(sText, "currentLine") || __uiDesignTokenIs(sText, "current_line") ||
	     __uiDesignTokenIs(sText, "caretLine") || __uiDesignTokenIs(sText, "caret_line") ) return XUI_CODE_STYLE_CURRENT_LINE;
	if ( __uiDesignTokenIs(sText, "searchResult") || __uiDesignTokenIs(sText, "search_result") ||
	     __uiDesignTokenIs(sText, "search") ) return XUI_CODE_STYLE_SEARCH_RESULT;
	if ( __uiDesignTokenIs(sText, "matchedBrace") || __uiDesignTokenIs(sText, "matched_brace") ||
	     __uiDesignTokenIs(sText, "matchBrace") || __uiDesignTokenIs(sText, "match_brace") ) return XUI_CODE_STYLE_MATCHED_BRACE;
	if ( __uiDesignTokenIs(sText, "margin") || __uiDesignTokenIs(sText, "gutter") ) return XUI_CODE_STYLE_MARGIN;
	if ( __uiDesignTokenIs(sText, "lineNumber") || __uiDesignTokenIs(sText, "line_number") ||
	     __uiDesignTokenIs(sText, "linenumber") ) return XUI_CODE_STYLE_LINE_NUMBER;
	if ( __uiDesignTokenIs(sText, "foldIcon") || __uiDesignTokenIs(sText, "fold_icon") ||
	     __uiDesignTokenIs(sText, "fold") ) return XUI_CODE_STYLE_FOLD_ICON;
	if ( __uiDesignTokenIs(sText, "marker") || __uiDesignTokenIs(sText, "markers") ) return XUI_CODE_STYLE_MARKER;
	if ( __uiDesignTokenIs(sText, "diagnosticError") || __uiDesignTokenIs(sText, "diagnostic_error") ||
	     __uiDesignTokenIs(sText, "diagError") || __uiDesignTokenIs(sText, "diag_error") ) return XUI_CODE_STYLE_DIAGNOSTIC_ERROR;
	if ( __uiDesignTokenIs(sText, "diagnosticWarning") || __uiDesignTokenIs(sText, "diagnostic_warning") ||
	     __uiDesignTokenIs(sText, "diagWarning") || __uiDesignTokenIs(sText, "diag_warning") ) return XUI_CODE_STYLE_DIAGNOSTIC_WARNING;
	if ( __uiDesignTokenIs(sText, "diagnosticInfo") || __uiDesignTokenIs(sText, "diagnostic_info") ||
	     __uiDesignTokenIs(sText, "diagInfo") || __uiDesignTokenIs(sText, "diag_info") ) return XUI_CODE_STYLE_DIAGNOSTIC_INFO;
	iValue = __uiDesignParseIntText(sText, iDefault);
	return (iValue >= 0 && iValue < UI_DESIGN_CODE_THEME_STYLE_CAPACITY) ? iValue : iDefault;
}

static int __uiDesignCodeTokenKindFromText(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "text") || __uiDesignTokenIs(sText, "default") ) return XUI_CODE_TOKEN_TEXT;
	if ( __uiDesignTokenIs(sText, "keyword") ) return XUI_CODE_TOKEN_KEYWORD;
	if ( __uiDesignTokenIs(sText, "type") ) return XUI_CODE_TOKEN_TYPE;
	if ( __uiDesignTokenIs(sText, "identifier") || __uiDesignTokenIs(sText, "id") ) return XUI_CODE_TOKEN_IDENTIFIER;
	if ( __uiDesignTokenIs(sText, "number") ) return XUI_CODE_TOKEN_NUMBER;
	if ( __uiDesignTokenIs(sText, "string") ) return XUI_CODE_TOKEN_STRING;
	if ( __uiDesignTokenIs(sText, "char") || __uiDesignTokenIs(sText, "character") ) return XUI_CODE_TOKEN_CHAR;
	if ( __uiDesignTokenIs(sText, "comment") ) return XUI_CODE_TOKEN_COMMENT;
	if ( __uiDesignTokenIs(sText, "preprocessor") || __uiDesignTokenIs(sText, "preproc") ) return XUI_CODE_TOKEN_PREPROCESSOR;
	if ( __uiDesignTokenIs(sText, "operator") ) return XUI_CODE_TOKEN_OPERATOR;
	if ( __uiDesignTokenIs(sText, "brace") || __uiDesignTokenIs(sText, "bracket") ) return XUI_CODE_TOKEN_BRACE;
	if ( __uiDesignTokenIs(sText, "error") ) return XUI_CODE_TOKEN_ERROR;
	iValue = __uiDesignParseIntText(sText, iDefault);
	return (iValue >= 0 && iValue < 128) ? iValue : iDefault;
}

static uint32_t __uiDesignCodeStyleFlagsFromFields(char** arrFields, int iFieldCount)
{
	uint32_t iFlags;

	iFlags = 0u;
	if ( __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 3, "false"), 0) ) iFlags |= XUI_CODE_STYLE_BOLD;
	if ( __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 4, "false"), 0) ) iFlags |= XUI_CODE_STYLE_ITALIC;
	if ( __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 5, "false"), 0) ) iFlags |= XUI_CODE_STYLE_UNDERLINE;
	if ( __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 6, "false"), 0) ) iFlags |= XUI_CODE_STYLE_STRIKE;
	return iFlags;
}

static void __uiDesignApplyCodeEditStyleTable(ui_design_node_t* pNode, xui_widget pWidget)
{
	const char* sCursor;
	char sLine[512];
	char* arrFields[7];
	int iFieldCount;
	int iStyleId;
	uint32_t iForeground;
	uint32_t iBackground;
	uint32_t iFlags;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	sCursor = __uiDesignText(pNode, "data.styles", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 1 || arrFields[0][0] == 0 ) continue;
		iStyleId = __uiDesignCodeStyleIdFromText(arrFields[0], -1);
		if ( iStyleId < 0 ) continue;
		iForeground = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 1, ""),
			__uiDesignColor(pNode, "theme.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)));
		iBackground = __uiDesignParseColorText(__uiDesignField(arrFields, iFieldCount, 2, ""), XUI_COLOR_RGBA(0, 0, 0, 0));
		iFlags = __uiDesignCodeStyleFlagsFromFields(arrFields, iFieldCount);
		__uiDesignCodeEditSetStyle(pWidget, iStyleId, iForeground, iBackground, iFlags);
	}
}

static void __uiDesignApplyCodeEditTokenStyles(ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_code_theme pTheme;
	const char* sCursor;
	char sLine[256];
	char* arrFields[2];
	int iFieldCount;
	int iTokenKind;
	int iStyleId;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	pTheme = xuiCodeEditGetTheme(pWidget);
	if ( pTheme == NULL ) return;
	sCursor = __uiDesignText(pNode, "data.tokenStyles", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 || arrFields[0][0] == 0 || arrFields[1][0] == 0 ) continue;
		iTokenKind = __uiDesignCodeTokenKindFromText(arrFields[0], -1);
		iStyleId = __uiDesignCodeStyleIdFromText(arrFields[1], -1);
		if ( iTokenKind < 0 || iStyleId < 0 ) continue;
		(void)xuiCodeThemeMapTokenKind(pTheme, iTokenKind, iStyleId);
	}
}

static void __uiDesignApplyCodeEditTheme(ui_design_node_t* pNode, xui_widget pWidget)
{
	uint32_t iTransparent;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	iTransparent = XUI_COLOR_RGBA(0, 0, 0, 0);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_DEFAULT,
		__uiDesignColor(pNode, "theme.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
		__uiDesignColor(pNode, "theme.backgroundColor", XUI_COLOR_WHITE), 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_KEYWORD,
		__uiDesignColor(pNode, "theme.keywordColor", XUI_COLOR_RGBA(31, 95, 168, 255)), iTransparent, XUI_CODE_STYLE_BOLD);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_TYPE,
		__uiDesignColor(pNode, "theme.typeColor", XUI_COLOR_RGBA(138, 76, 184, 255)), iTransparent, 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_IDENTIFIER,
		__uiDesignColor(pNode, "theme.identifierColor", XUI_COLOR_RGBA(35, 50, 70, 255)), iTransparent, 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_NUMBER,
		__uiDesignColor(pNode, "theme.numberColor", XUI_COLOR_RGBA(180, 95, 6, 255)), iTransparent, 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_STRING,
		__uiDesignColor(pNode, "theme.stringColor", XUI_COLOR_RGBA(47, 125, 50, 255)), iTransparent, 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_CHAR,
		__uiDesignColor(pNode, "theme.charColor", XUI_COLOR_RGBA(47, 125, 50, 255)), iTransparent, 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_COMMENT,
		__uiDesignColor(pNode, "theme.commentColor", XUI_COLOR_RGBA(107, 127, 149, 255)), iTransparent, XUI_CODE_STYLE_ITALIC);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_PREPROCESSOR,
		__uiDesignColor(pNode, "theme.preprocessorColor", XUI_COLOR_RGBA(156, 77, 123, 255)), iTransparent, 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_OPERATOR,
		__uiDesignColor(pNode, "theme.operatorColor", XUI_COLOR_RGBA(52, 71, 93, 255)), iTransparent, 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_BRACE,
		__uiDesignColor(pNode, "theme.braceColor", XUI_COLOR_RGBA(52, 71, 93, 255)), iTransparent, 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_ERROR,
		__uiDesignColor(pNode, "theme.errorColor", XUI_COLOR_RGBA(180, 35, 24, 255)), iTransparent, XUI_CODE_STYLE_UNDERLINE);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_SELECTION,
		__uiDesignColor(pNode, "theme.selectionTextColor", XUI_COLOR_RGBA(18, 54, 92, 255)),
		__uiDesignColor(pNode, "theme.selectionColor", XUI_COLOR_RGBA(215, 232, 255, 255)), 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_CURRENT_LINE,
		__uiDesignColor(pNode, "theme.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
		__uiDesignColor(pNode, "theme.currentLineColor", XUI_COLOR_RGBA(243, 248, 255, 255)), 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_SEARCH_RESULT,
		__uiDesignColor(pNode, "theme.searchTextColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
		__uiDesignColor(pNode, "theme.searchColor", XUI_COLOR_RGBA(255, 224, 138, 255)), 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_MATCHED_BRACE,
		__uiDesignColor(pNode, "theme.matchedBraceColor", XUI_COLOR_RGBA(15, 81, 50, 255)),
		__uiDesignColor(pNode, "theme.matchedBraceBackground", XUI_COLOR_RGBA(209, 242, 221, 255)), XUI_CODE_STYLE_BOLD);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_MARGIN,
		__uiDesignColor(pNode, "theme.lineNumberColor", XUI_COLOR_RGBA(107, 127, 149, 255)),
		__uiDesignColor(pNode, "theme.marginColor", XUI_COLOR_RGBA(240, 244, 248, 255)), 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_LINE_NUMBER,
		__uiDesignColor(pNode, "theme.lineNumberColor", XUI_COLOR_RGBA(107, 127, 149, 255)),
		__uiDesignColor(pNode, "theme.marginColor", XUI_COLOR_RGBA(240, 244, 248, 255)), 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_FOLD_ICON,
		__uiDesignColor(pNode, "theme.foldIconColor", XUI_COLOR_RGBA(82, 101, 122, 255)),
		__uiDesignColor(pNode, "theme.marginColor", XUI_COLOR_RGBA(240, 244, 248, 255)), 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_MARKER,
		__uiDesignColor(pNode, "theme.markerColor", XUI_COLOR_RGBA(49, 126, 214, 255)), iTransparent, 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_DIAGNOSTIC_ERROR,
		__uiDesignColor(pNode, "theme.diagnosticErrorColor", XUI_COLOR_RGBA(214, 72, 86, 255)), iTransparent, 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_DIAGNOSTIC_WARNING,
		__uiDesignColor(pNode, "theme.diagnosticWarningColor", XUI_COLOR_RGBA(214, 137, 16, 255)), iTransparent, 0u);
	__uiDesignCodeEditSetStyle(pWidget, XUI_CODE_STYLE_DIAGNOSTIC_INFO,
		__uiDesignColor(pNode, "theme.diagnosticInfoColor", XUI_COLOR_RGBA(49, 126, 214, 255)), iTransparent, 0u);
	__uiDesignApplyCodeEditStyleTable(pNode, pWidget);
	__uiDesignApplyCodeEditTokenStyles(pNode, pWidget);
}

static int __uiDesignCodeMarkerFromText(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "breakpoint") || __uiDesignTokenIs(sText, "bp") ) return XUI_CODE_MARKER_BREAKPOINT;
	if ( __uiDesignTokenIs(sText, "breakpointDisabled") || __uiDesignTokenIs(sText, "breakpoint_disabled") ||
	     __uiDesignTokenIs(sText, "disabledBreakpoint") || __uiDesignTokenIs(sText, "disabled_breakpoint") ) {
		return XUI_CODE_MARKER_BREAKPOINT_DISABLED;
	}
	if ( __uiDesignTokenIs(sText, "execution") || __uiDesignTokenIs(sText, "executionLine") ||
	     __uiDesignTokenIs(sText, "execution_line") ) return XUI_CODE_MARKER_EXECUTION_LINE;
	if ( __uiDesignTokenIs(sText, "bookmark") ) return XUI_CODE_MARKER_BOOKMARK;
	if ( __uiDesignTokenIs(sText, "modified") || __uiDesignTokenIs(sText, "modifiedLine") ||
	     __uiDesignTokenIs(sText, "modified_line") ) return XUI_CODE_MARKER_MODIFIED_LINE;
	if ( __uiDesignTokenIs(sText, "saved") || __uiDesignTokenIs(sText, "savedLine") ||
	     __uiDesignTokenIs(sText, "saved_line") ) return XUI_CODE_MARKER_SAVED_LINE;
	if ( __uiDesignTokenIs(sText, "error") || __uiDesignTokenIs(sText, "errorLine") ||
	     __uiDesignTokenIs(sText, "error_line") ) return XUI_CODE_MARKER_ERROR_LINE;
	if ( __uiDesignTokenIs(sText, "warning") || __uiDesignTokenIs(sText, "warn") ||
	     __uiDesignTokenIs(sText, "warningLine") || __uiDesignTokenIs(sText, "warning_line") ) {
		return XUI_CODE_MARKER_WARNING_LINE;
	}
	if ( __uiDesignTokenIs(sText, "info") || __uiDesignTokenIs(sText, "infoLine") ||
	     __uiDesignTokenIs(sText, "info_line") ) return XUI_CODE_MARKER_INFO_LINE;
	iValue = __uiDesignParseIntText(sText, iDefault);
	return (iValue > 0) ? iValue : iDefault;
}

static int __uiDesignCodeIndicatorStyleFromText(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "squiggle") || __uiDesignTokenIs(sText, "wave") ) return XUI_CODE_INDICATOR_SQUIGGLE;
	if ( __uiDesignTokenIs(sText, "underline") ) return XUI_CODE_INDICATOR_UNDERLINE;
	if ( __uiDesignTokenIs(sText, "box") ) return XUI_CODE_INDICATOR_BOX;
	if ( __uiDesignTokenIs(sText, "roundedBox") || __uiDesignTokenIs(sText, "rounded_box") ||
	     __uiDesignTokenIs(sText, "rounded") ) return XUI_CODE_INDICATOR_ROUNDED_BOX;
	if ( __uiDesignTokenIs(sText, "background") || __uiDesignTokenIs(sText, "back") ) return XUI_CODE_INDICATOR_BACKGROUND;
	if ( __uiDesignTokenIs(sText, "textForeground") || __uiDesignTokenIs(sText, "text_foreground") ||
	     __uiDesignTokenIs(sText, "foreground") || __uiDesignTokenIs(sText, "text") ) return XUI_CODE_INDICATOR_TEXT_FOREGROUND;
	if ( __uiDesignTokenIs(sText, "searchResult") || __uiDesignTokenIs(sText, "search_result") ||
	     __uiDesignTokenIs(sText, "search") ) return XUI_CODE_INDICATOR_SEARCH_RESULT;
	iValue = __uiDesignParseIntText(sText, iDefault);
	return (iValue > 0) ? iValue : iDefault;
}

static int __uiDesignCodeMarginKindFromText(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "lineNumber") || __uiDesignTokenIs(sText, "line_number") ||
	     __uiDesignTokenIs(sText, "line") || __uiDesignTokenIs(sText, "linenumber") ) return XUI_CODE_MARGIN_LINE_NUMBER;
	if ( __uiDesignTokenIs(sText, "marker") || __uiDesignTokenIs(sText, "markers") ) return XUI_CODE_MARGIN_MARKER;
	if ( __uiDesignTokenIs(sText, "fold") || __uiDesignTokenIs(sText, "folding") ) return XUI_CODE_MARGIN_FOLD;
	if ( __uiDesignTokenIs(sText, "change") || __uiDesignTokenIs(sText, "changes") ) return XUI_CODE_MARGIN_CHANGE;
	if ( __uiDesignTokenIs(sText, "diagnostic") || __uiDesignTokenIs(sText, "diagnostics") ) return XUI_CODE_MARGIN_DIAGNOSTIC;
	if ( __uiDesignTokenIs(sText, "custom") ) return XUI_CODE_MARGIN_CUSTOM;
	iValue = __uiDesignParseIntText(sText, iDefault);
	return (iValue > 0) ? iValue : iDefault;
}

static int __uiDesignClampIntRange(int iValue, int iMin, int iMax)
{
	if ( iMax < iMin ) return iMin;
	if ( iValue < iMin ) return iMin;
	if ( iValue > iMax ) return iMax;
	return iValue;
}

static uint32_t __uiDesignCodeSelectionFlagsFromText(const char* sText, uint32_t iDefault)
{
	uint32_t iFlags;

	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignLooksIntText(sText) ) return (uint32_t)__uiDesignParseIntText(sText, (int)iDefault);
	iFlags = 0u;
	if ( __uiDesignTextContainsNoCase(sText, "rect") || __uiDesignTextContainsNoCase(sText, "column") ) {
		iFlags |= XUI_CODE_SELECTION_RECT;
	}
	if ( __uiDesignTextContainsNoCase(sText, "reverse") || __uiDesignTextContainsNoCase(sText, "reversed") ) {
		iFlags |= XUI_CODE_SELECTION_REVERSED;
	}
	if ( __uiDesignTextContainsNoCase(sText, "inactive") || __uiDesignTextContainsNoCase(sText, "extra") ) {
		iFlags |= XUI_CODE_SELECTION_INACTIVE;
	}
	return iFlags;
}

static uint32_t __uiDesignCodeFoldFlagsFromText(const char* sText, uint32_t iDefault)
{
	uint32_t iFlags;

	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignLooksIntText(sText) ) return (uint32_t)__uiDesignParseIntText(sText, (int)iDefault);
	iFlags = 0u;
	if ( __uiDesignTextContainsNoCase(sText, "collapsed") || __uiDesignTextContainsNoCase(sText, "folded") ) {
		iFlags |= XUI_CODE_FOLD_COLLAPSED;
	}
	if ( __uiDesignTextContainsNoCase(sText, "header") ) {
		iFlags |= XUI_CODE_FOLD_HEADER;
	}
	if ( __uiDesignTextContainsNoCase(sText, "comment") ) {
		iFlags |= XUI_CODE_FOLD_COMMENT;
	}
	if ( __uiDesignTextContainsNoCase(sText, "preprocessor") || __uiDesignTextContainsNoCase(sText, "preproc") ) {
		iFlags |= XUI_CODE_FOLD_PREPROCESSOR;
	}
	if ( __uiDesignTextContainsNoCase(sText, "region") ) {
		iFlags |= XUI_CODE_FOLD_REGION;
	}
	if ( __uiDesignTextContainsNoCase(sText, "custom") ) {
		iFlags |= XUI_CODE_FOLD_CUSTOM;
	}
	return iFlags;
}

static int __uiDesignApplyCodeEditMargins(ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_code_margin_model pMargins;
	xui_code_margin_desc_t tDesc;
	const char* sRows;
	const char* sCursor;
	char sLine[512];
	char* arrFields[6];
	int iFieldCount;
	int iId;
	int iKind;
	float fWidth;
	uint32_t iFlags;

	if ( (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	sRows = __uiDesignText(pNode, "data.margins", "");
	if ( (sRows == NULL) || (sRows[0] == 0) ) {
		return xuiCodeEditSetDefaultMargins(pWidget,
			__uiDesignBool(pNode, "behavior.lineNumbers", 1),
			__uiDesignBool(pNode, "behavior.markerMargin", 1),
			__uiDesignBool(pNode, "behavior.foldMargin", 1),
			__uiDesignBool(pNode, "behavior.diagnosticMargin", 1));
	}
	pMargins = xuiCodeEditGetMargins(pWidget);
	if ( pMargins == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	xuiCodeMarginModelClear(pMargins);
	sCursor = sRows;
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 3 ) continue;
		iId = __uiDesignParseIntText(arrFields[0], 0);
		iKind = __uiDesignCodeMarginKindFromText(arrFields[1], 0);
		fWidth = __uiDesignParseFloatText(arrFields[2], 0.0f);
		if ( (iId <= 0) || (iKind <= 0) || (fWidth < 0.0f) ) continue;
		iFlags = 0u;
		if ( __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 3, "true"), 1) ) iFlags |= XUI_CODE_MARGIN_VISIBLE;
		if ( __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 4, "false"), 0) ) iFlags |= XUI_CODE_MARGIN_CLICKABLE;
		if ( __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 5, "false"), 0) ) iFlags |= XUI_CODE_MARGIN_RESIZABLE;
		memset(&tDesc, 0, sizeof(tDesc));
		tDesc.iSize = sizeof(tDesc);
		tDesc.iId = iId;
		tDesc.iKind = iKind;
		tDesc.fWidth = fWidth;
		tDesc.iFlags = iFlags;
		(void)xuiCodeMarginModelAdd(pMargins, &tDesc);
	}
	return XUI_OK;
}

static void __uiDesignApplyCodeEditAnnotations(ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_code_annotation_store pStore;
	xui_code_diagnostic_t arrDiagnostics[UI_DESIGN_RUNTIME_TABLE_ROWS];
	const char* sCursor;
	char sLine[1024];
	char* arrFields[6];
	int iFieldCount;
	int iLine;
	int iMarker;
	int iIndicator;
	int iStyle;
	int iStart;
	int iEnd;
	int iDiagnosticCount;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	pStore = xuiCodeEditGetAnnotations(pWidget);
	if ( pStore == NULL ) return;
	(void)xuiCodeAnnotationStoreClear(pStore);

	sCursor = __uiDesignText(pNode, "data.markers", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 || arrFields[0][0] == 0 || arrFields[1][0] == 0 ) continue;
		iLine = __uiDesignParseIntText(arrFields[0], 1) - 1;
		iMarker = __uiDesignCodeMarkerFromText(arrFields[1], XUI_CODE_MARKER_BOOKMARK);
		if ( (iLine < 0) || (iMarker <= 0) ) continue;
		(void)xuiCodeAnnotationSetMarker(pStore, iLine, iMarker,
			(uint32_t)__uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 2, "0"), 0),
			__uiDesignField(arrFields, iFieldCount, 3, ""), 0);
	}

	sCursor = __uiDesignText(pNode, "data.indicators", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 4 || arrFields[0][0] == 0 || arrFields[1][0] == 0 ) continue;
		iIndicator = __uiDesignParseIntText(arrFields[0], 1);
		iStyle = __uiDesignCodeIndicatorStyleFromText(arrFields[1], XUI_CODE_INDICATOR_SQUIGGLE);
		iStart = __uiDesignParseIntText(arrFields[2], 0);
		iEnd = __uiDesignParseIntText(arrFields[3], iStart + 1);
		if ( (iIndicator <= 0) || (iStyle <= 0) || (iStart < 0) || (iEnd <= iStart) ) continue;
		(void)xuiCodeAnnotationSetIndicator(pStore, iIndicator, iStyle, iStart, iEnd,
			(uint32_t)__uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 4, "0"), 0), 0);
	}

	memset(arrDiagnostics, 0, sizeof(arrDiagnostics));
	iDiagnosticCount = 0;
	sCursor = __uiDesignText(pNode, "data.diagnostics", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iDiagnosticCount < UI_DESIGN_COUNT_OF(arrDiagnostics) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 5 ) continue;
		iStart = __uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 1, "0"), 0);
		iEnd = __uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 2, "0"), 0);
		if ( iStart < 0 || iEnd <= iStart ) continue;
		arrDiagnostics[iDiagnosticCount].iSize = sizeof(arrDiagnostics[iDiagnosticCount]);
		arrDiagnostics[iDiagnosticCount].tRange.iStart = iStart;
		arrDiagnostics[iDiagnosticCount].tRange.iEnd = iEnd;
		arrDiagnostics[iDiagnosticCount].iSeverity = __uiDesignFlowDiagnosticSeverity(__uiDesignField(arrFields, iFieldCount, 0, "error"), XUI_CODE_DIAGNOSTIC_ERROR);
		arrDiagnostics[iDiagnosticCount].sCode = __uiDesignField(arrFields, iFieldCount, 3, "");
		arrDiagnostics[iDiagnosticCount].sMessage = __uiDesignField(arrFields, iFieldCount, 4, "");
		arrDiagnostics[iDiagnosticCount].sSource = __uiDesignField(arrFields, iFieldCount, 5, "");
		++iDiagnosticCount;
	}
	if ( iDiagnosticCount > 0 ) {
		(void)xuiCodeAnnotationSetDiagnostics(pStore, arrDiagnostics, iDiagnosticCount);
	}
}

static void __uiDesignApplyCodeEditSelections(ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_code_document pDocument;
	xui_code_selection_model pSelection;
	xui_code_selection_t tSelection;
	const char* sCursor;
	char sLine[512];
	char* arrFields[4];
	int iFieldCount;
	int iLength;
	int iAnchor;
	int iCaret;
	int iCount;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	pDocument = xuiCodeEditGetDocument(pWidget);
	pSelection = xuiCodeEditGetSelection(pWidget);
	if ( (pDocument == NULL) || (pSelection == NULL) ) return;
	iLength = xuiCodeDocumentGetLength(pDocument);
	if ( iLength < 0 ) iLength = 0;
	xuiCodeSelectionClear(pSelection);
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.selections", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 ) continue;
		iAnchor = __uiDesignClampIntRange(__uiDesignParseIntText(arrFields[0], 0), 0, iLength);
		iCaret = __uiDesignClampIntRange(__uiDesignParseIntText(arrFields[1], iAnchor), 0, iLength);
		memset(&tSelection, 0, sizeof(tSelection));
		tSelection.iSize = sizeof(tSelection);
		tSelection.iAnchorOffset = iAnchor;
		tSelection.iCaretOffset = iCaret;
		tSelection.iPreferredColumn = __uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 2, "-1"), -1);
		tSelection.iFlags = __uiDesignCodeSelectionFlagsFromText(__uiDesignField(arrFields, iFieldCount, 3, ""), 0u);
		if ( iCount == 0 ) {
			(void)xuiCodeSelectionSetState(pSelection, pDocument, &tSelection);
		} else {
			(void)xuiCodeSelectionAdd(pSelection, &tSelection);
		}
		++iCount;
	}
	if ( iCount == 0 ) {
		(void)xuiCodeSelectionSetRange(pSelection, pDocument, 0, 0);
	}
	(void)xuiCodeEditEnsureCaretVisible(pWidget);
}

static void __uiDesignApplyCodeEditFolds(ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_code_document pDocument;
	xui_code_fold_state pFoldState;
	xui_code_fold_range_t arrRanges[UI_DESIGN_RUNTIME_TABLE_ROWS];
	const char* sCursor;
	char sLine[512];
	char* arrFields[4];
	int iFieldCount;
	int iLineCount;
	int iStartLine;
	int iEndLine;
	int iCount;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	pDocument = xuiCodeEditGetDocument(pWidget);
	pFoldState = xuiCodeEditGetFoldState(pWidget);
	if ( (pDocument == NULL) || (pFoldState == NULL) ) return;
	iLineCount = xuiCodeDocumentGetLineCount(pDocument);
	if ( iLineCount < 1 ) iLineCount = 1;
	memset(arrRanges, 0, sizeof(arrRanges));
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.folds", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < UI_DESIGN_COUNT_OF(arrRanges) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 ) continue;
		iStartLine = __uiDesignClampIntRange(__uiDesignParseIntText(arrFields[0], 1) - 1, 0, iLineCount - 1);
		iEndLine = __uiDesignClampIntRange(__uiDesignParseIntText(arrFields[1], iStartLine + 2) - 1, 0, iLineCount - 1);
		if ( iEndLine <= iStartLine ) continue;
		arrRanges[iCount].iSize = sizeof(arrRanges[iCount]);
		arrRanges[iCount].iStartLine = iStartLine;
		arrRanges[iCount].iEndLine = iEndLine;
		arrRanges[iCount].iLevel = __uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 2, "0"), 0);
		arrRanges[iCount].iFlags = __uiDesignCodeFoldFlagsFromText(__uiDesignField(arrFields, iFieldCount, 3, "header"), XUI_CODE_FOLD_HEADER);
		++iCount;
	}
	xuiCodeFoldStateClear(pFoldState);
	if ( iCount > 0 ) {
		(void)xuiCodeFoldStateSetRanges(pFoldState, arrRanges, iCount);
	}
}

static void __uiDesignFillInventoryLayout(const ui_design_node_t* pNode, xui_inventory_grid_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iSize = sizeof(*pLayout);
	pLayout->iColumns = __uiDesignInt(pNode, "metrics.columns", 0);
	pLayout->iSelectionMode = __uiDesignInt(pNode, "behavior.selectionMode", XUI_INVENTORY_SELECTION_SINGLE);
	pLayout->fSlotSize = __uiDesignFloat(pNode, "metrics.slotSize", 42.0f);
	pLayout->fSlotGap = __uiDesignFloat(pNode, "metrics.slotGap", 6.0f);
	pLayout->fPadding = __uiDesignFloat(pNode, "metrics.padding", 8.0f);
	pLayout->fIconPadding = __uiDesignFloat(pNode, "metrics.iconPadding", 5.0f);
	pLayout->fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	pLayout->fWheelStep = __uiDesignFloat(pNode, "metrics.wheelStep", 48.0f);
	pLayout->fDragThreshold = __uiDesignFloat(pNode, "metrics.dragThreshold", 4.0f);
}

static void __uiDesignFillInventoryColors(const ui_design_node_t* pNode, xui_inventory_grid_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iSize = sizeof(*pColors);
	pColors->iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(23, 32, 43, 255));
	pColors->iSlotColor = __uiDesignColor(pNode, "appearance.slotColor", XUI_COLOR_RGBA(37, 50, 69, 255));
	pColors->iEmptyColor = __uiDesignColor(pNode, "appearance.emptyColor", XUI_COLOR_RGBA(28, 38, 52, 255));
	pColors->iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(51, 68, 90, 255));
	pColors->iActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(59, 82, 112, 255));
	pColors->iSelectedColor = __uiDesignColor(pNode, "appearance.selectedColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	pColors->iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(46, 55, 67, 255));
	pColors->iLockedColor = __uiDesignColor(pNode, "appearance.lockedColor", XUI_COLOR_RGBA(80, 55, 62, 255));
	pColors->iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(82, 101, 122, 255));
	pColors->iQualityColor = __uiDesignColor(pNode, "appearance.qualityColor", XUI_COLOR_RGBA(245, 166, 35, 255));
	pColors->iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	pColors->iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_WHITE);
	pColors->iMutedTextColor = __uiDesignColor(pNode, "appearance.mutedTextColor", XUI_COLOR_RGBA(172, 186, 203, 255));
	pColors->iCountColor = __uiDesignColor(pNode, "appearance.countColor", XUI_COLOR_WHITE);
	pColors->iHotkeyColor = __uiDesignColor(pNode, "appearance.hotkeyColor", XUI_COLOR_RGBA(196, 207, 222, 255));
	pColors->iCooldownColor = __uiDesignColor(pNode, "appearance.cooldownColor", XUI_COLOR_RGBA(0, 0, 0, 112));
	pColors->iDurabilityColor = __uiDesignColor(pNode, "appearance.durabilityColor", XUI_COLOR_RGBA(45, 157, 110, 255));
	pColors->iDragColor = __uiDesignColor(pNode, "appearance.dragColor", XUI_COLOR_RGBA(49, 126, 214, 120));
	pColors->iDropColor = __uiDesignColor(pNode, "appearance.dropColor", XUI_COLOR_RGBA(45, 157, 110, 120));
}

static void __uiDesignFillMessageMetrics(const ui_design_node_t* pNode, xui_message_list_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->fPaddingX = __uiDesignFloat(pNode, "metrics.paddingX", 10.0f);
	pMetrics->fPaddingY = __uiDesignFloat(pNode, "metrics.paddingY", 10.0f);
	pMetrics->fNodeGap = __uiDesignFloat(pNode, "metrics.nodeGap", 8.0f);
	pMetrics->fAvatarSize = __uiDesignFloat(pNode, "metrics.avatarSize", 28.0f);
	pMetrics->fAvatarGap = __uiDesignFloat(pNode, "metrics.avatarGap", 8.0f);
	pMetrics->fBubbleMaxWidth = __uiDesignFloat(pNode, "metrics.bubbleMaxWidth", 220.0f);
	pMetrics->fBubblePaddingX = __uiDesignFloat(pNode, "metrics.bubblePaddingX", 10.0f);
	pMetrics->fBubblePaddingY = __uiDesignFloat(pNode, "metrics.bubblePaddingY", 7.0f);
	pMetrics->fSystemPaddingX = __uiDesignFloat(pNode, "metrics.systemPaddingX", 10.0f);
	pMetrics->fSystemPaddingY = __uiDesignFloat(pNode, "metrics.systemPaddingY", 5.0f);
	pMetrics->fMetaHeight = __uiDesignFloat(pNode, "metrics.metaHeight", 18.0f);
	pMetrics->fMinBubbleHeight = __uiDesignFloat(pNode, "metrics.minBubbleHeight", 28.0f);
	pMetrics->fWheelStep = __uiDesignFloat(pNode, "metrics.wheelStep", 48.0f);
}

static void __uiDesignFillMessageColors(const ui_design_node_t* pNode, xui_message_list_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 250, 254, 255));
	pColors->iSelfBubbleColor = __uiDesignColor(pNode, "appearance.selfBubbleColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	pColors->iOtherBubbleColor = __uiDesignColor(pNode, "appearance.otherBubbleColor", XUI_COLOR_WHITE);
	pColors->iSystemBubbleColor = __uiDesignColor(pNode, "appearance.systemBubbleColor", XUI_COLOR_RGBA(234, 241, 248, 255));
	pColors->iSelfTextColor = __uiDesignColor(pNode, "appearance.selfTextColor", XUI_COLOR_WHITE);
	pColors->iOtherTextColor = __uiDesignColor(pNode, "appearance.otherTextColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	pColors->iSystemTextColor = __uiDesignColor(pNode, "appearance.systemTextColor", XUI_COLOR_RGBA(82, 101, 122, 255));
	pColors->iMetaTextColor = __uiDesignColor(pNode, "appearance.metaTextColor", XUI_COLOR_RGBA(107, 127, 149, 255));
	pColors->iAvatarSelfColor = __uiDesignColor(pNode, "appearance.avatarSelfColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	pColors->iAvatarOtherColor = __uiDesignColor(pNode, "appearance.avatarOtherColor", XUI_COLOR_RGBA(128, 150, 176, 255));
	pColors->iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255));
	pColors->iSelectedColor = __uiDesignColor(pNode, "appearance.selectedColor", XUI_COLOR_RGBA(217, 234, 254, 255));
	pColors->iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(208, 219, 232, 255));
}

static void __uiDesignFillTimelineColors(const ui_design_node_t* pNode, xui_timeline_view_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	pColors->iCornerColor = __uiDesignColor(pNode, "appearance.cornerColor", XUI_COLOR_RGBA(238, 245, 252, 255));
	pColors->iRulerColor = __uiDesignColor(pNode, "appearance.rulerColor", XUI_COLOR_RGBA(234, 241, 248, 255));
	pColors->iLayerColor = __uiDesignColor(pNode, "appearance.layerColor", XUI_COLOR_RGBA(247, 250, 254, 255));
	pColors->iLayerAltColor = __uiDesignColor(pNode, "appearance.layerAltColor", XUI_COLOR_RGBA(241, 246, 252, 255));
	pColors->iGridColor = __uiDesignColor(pNode, "appearance.gridColor", XUI_COLOR_RGBA(208, 219, 232, 255));
	pColors->iGridStrongColor = __uiDesignColor(pNode, "appearance.gridStrongColor", XUI_COLOR_RGBA(180, 196, 216, 255));
	pColors->iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	pColors->iMutedTextColor = __uiDesignColor(pNode, "appearance.mutedTextColor", XUI_COLOR_RGBA(107, 127, 149, 255));
	pColors->iSelectedColor = __uiDesignColor(pNode, "appearance.selectedColor", XUI_COLOR_RGBA(217, 234, 254, 255));
	pColors->iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255));
	pColors->iCurrentColor = __uiDesignColor(pNode, "appearance.currentColor", XUI_COLOR_RGBA(214, 72, 86, 255));
	pColors->iFrameColor = __uiDesignColor(pNode, "appearance.frameColor", XUI_COLOR_WHITE);
	pColors->iKeyFrameColor = __uiDesignColor(pNode, "appearance.keyFrameColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	pColors->iBlankKeyFrameColor = __uiDesignColor(pNode, "appearance.blankKeyFrameColor", XUI_COLOR_WHITE);
	pColors->iSpanColor = __uiDesignColor(pNode, "appearance.spanColor", XUI_COLOR_RGBA(49, 126, 214, 180));
	pColors->iSpanTextColor = __uiDesignColor(pNode, "appearance.spanTextColor", XUI_COLOR_WHITE);
	pColors->iIconColor = __uiDesignColor(pNode, "appearance.iconColor", XUI_COLOR_RGBA(82, 101, 122, 255));
	pColors->iHiddenIconColor = __uiDesignColor(pNode, "appearance.hiddenIconColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	pColors->iLockedIconColor = __uiDesignColor(pNode, "appearance.lockedIconColor", XUI_COLOR_RGBA(214, 72, 86, 255));
	pColors->iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	pColors->iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	pColors->iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	pColors->iTrackColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255));
	pColors->iThumbColor = __uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255));
	pColors->iScrollbarHoverColor = __uiDesignColor(pNode, "appearance.scrollbarHoverColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	pColors->iScrollbarActiveColor = __uiDesignColor(pNode, "appearance.scrollbarActiveColor", XUI_COLOR_RGBA(111, 137, 166, 255));
	pColors->iScrollbarFocusColor = __uiDesignColor(pNode, "appearance.scrollbarFocusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	pColors->iScrollbarDisabledColor = __uiDesignColor(pNode, "appearance.scrollbarDisabledColor", XUI_COLOR_RGBA(206, 215, 226, 255));
}

static void __uiDesignFillPropertyGridStyle(const ui_design_node_t* pNode, xui_property_grid_style_t* pStyle)
{
	if ( pStyle == NULL ) return;
	memset(pStyle, 0, sizeof(*pStyle));
	pStyle->iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	pStyle->iGridColor = __uiDesignColor(pNode, "appearance.gridColor", XUI_COLOR_RGBA(208, 219, 232, 255));
	pStyle->iCategoryBackgroundColor = __uiDesignColor(pNode, "appearance.categoryColor", XUI_COLOR_RGBA(234, 241, 248, 255));
	pStyle->iCategoryHoverColor = __uiDesignColor(pNode, "appearance.categoryHoverColor", XUI_COLOR_RGBA(221, 234, 247, 255));
	pStyle->iCategoryTextColor = __uiDesignColor(pNode, "appearance.categoryTextColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	pStyle->iCategoryIconColor = __uiDesignColor(pNode, "appearance.categoryIconColor", XUI_COLOR_RGBA(82, 101, 122, 255));
	pStyle->iNameBackgroundColor = __uiDesignColor(pNode, "appearance.nameBackgroundColor", XUI_COLOR_RGBA(247, 250, 254, 255));
	pStyle->iNameTextColor = __uiDesignColor(pNode, "appearance.nameTextColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	pStyle->iNameHoverColor = __uiDesignColor(pNode, "appearance.nameHoverColor", XUI_COLOR_RGBA(240, 246, 253, 255));
	pStyle->iValueBackgroundColor = __uiDesignColor(pNode, "appearance.valueBackgroundColor", XUI_COLOR_WHITE);
	pStyle->iValueTextColor = __uiDesignColor(pNode, "appearance.valueTextColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	pStyle->iSelectedColor = __uiDesignColor(pNode, "appearance.selectedColor", XUI_COLOR_RGBA(217, 234, 254, 255));
	pStyle->iReadonlyTextColor = __uiDesignColor(pNode, "appearance.readonlyTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	pStyle->iInvalidColor = __uiDesignColor(pNode, "appearance.invalidColor", XUI_COLOR_RGBA(214, 72, 86, 255));
	pStyle->iDirtyColor = __uiDesignColor(pNode, "appearance.dirtyColor", XUI_COLOR_RGBA(49, 126, 214, 255));
}

static void __uiDesignFillMenuBarMetrics(const ui_design_node_t* pNode, xui_menubar_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->fHeight = __uiDesignFloat(pNode, "metrics.height", 28.0f);
	pMetrics->fPaddingX = __uiDesignFloat(pNode, "metrics.paddingX", 8.0f);
	pMetrics->fPaddingY = __uiDesignFloat(pNode, "metrics.paddingY", 0.0f);
	pMetrics->fItemPaddingX = __uiDesignFloat(pNode, "metrics.itemPaddingX", 12.0f);
	pMetrics->fItemGap = __uiDesignFloat(pNode, "metrics.itemGap", 2.0f);
	pMetrics->fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
}

static void __uiDesignFillMenuBarColors(const ui_design_node_t* pNode, xui_menubar_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iSize = sizeof(*pColors);
	pColors->iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 250, 254, 255));
	pColors->iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	pColors->iItemColor = __uiDesignColor(pNode, "appearance.itemColor", XUI_COLOR_RGBA(0, 0, 0, 0));
	pColors->iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255));
	pColors->iActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(217, 234, 254, 255));
	pColors->iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	pColors->iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	pColors->iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
}

static void __uiDesignFillToolbarMetrics(const ui_design_node_t* pNode, xui_toolbar_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->iOrientation = __uiDesignInt(pNode, "behavior.orientation", XUI_ORIENTATION_HORIZONTAL);
	pMetrics->fItemWidth = __uiDesignFloat(pNode, "metrics.itemWidth", 64.0f);
	pMetrics->fItemHeight = __uiDesignFloat(pNode, "metrics.itemHeight", 30.0f);
	pMetrics->fSeparatorSize = __uiDesignFloat(pNode, "metrics.separatorSize", 8.0f);
	pMetrics->fGroupGap = __uiDesignFloat(pNode, "metrics.groupGap", 8.0f);
	pMetrics->fPaddingX = __uiDesignFloat(pNode, "metrics.paddingX", 6.0f);
	pMetrics->fPaddingY = __uiDesignFloat(pNode, "metrics.paddingY", 4.0f);
	pMetrics->fOverflowSize = __uiDesignFloat(pNode, "metrics.overflowButtonSize", 28.0f);
	pMetrics->fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	pMetrics->fIconSize = __uiDesignFloat(pNode, "metrics.iconSize", 16.0f);
	pMetrics->fIconGap = __uiDesignFloat(pNode, "metrics.iconGap", 6.0f);
}

static void __uiDesignFillToolbarColors(const ui_design_node_t* pNode, xui_toolbar_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iSize = sizeof(*pColors);
	pColors->iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 250, 254, 255));
	pColors->iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	pColors->iItemColor = __uiDesignColor(pNode, "appearance.itemColor", XUI_COLOR_WHITE);
	pColors->iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255));
	pColors->iActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(217, 234, 254, 255));
	pColors->iCheckedColor = __uiDesignColor(pNode, "appearance.checkedColor", XUI_COLOR_RGBA(188, 215, 245, 255));
	pColors->iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	pColors->iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(238, 242, 247, 255));
	pColors->iSeparatorColor = __uiDesignColor(pNode, "appearance.separatorColor", XUI_COLOR_RGBA(208, 219, 232, 255));
	pColors->iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	pColors->iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	pColors->iIconColor = __uiDesignColor(pNode, "appearance.iconColor", XUI_COLOR_RGBA(52, 71, 93, 255));
}

static void __uiDesignFillStatusBarMetrics(const ui_design_node_t* pNode, xui_statusbar_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->fHeight = __uiDesignFloat(pNode, "metrics.height", 24.0f);
	pMetrics->fPaddingX = __uiDesignFloat(pNode, "metrics.paddingX", 8.0f);
	pMetrics->fPaddingY = __uiDesignFloat(pNode, "metrics.paddingY", 2.0f);
	pMetrics->fGap = __uiDesignFloat(pNode, "metrics.gap", 8.0f);
	pMetrics->fItemPaddingX = __uiDesignFloat(pNode, "metrics.itemPaddingX", 6.0f);
	pMetrics->fItemPaddingY = __uiDesignFloat(pNode, "metrics.itemPaddingY", 2.0f);
	pMetrics->fProgressHeight = __uiDesignFloat(pNode, "metrics.progressHeight", 6.0f);
	pMetrics->fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	pMetrics->fTopBorderWidth = __uiDesignFloat(pNode, "metrics.topBorderWidth", 1.0f);
}

static void __uiDesignFillStatusBarColors(const ui_design_node_t* pNode, xui_statusbar_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iSize = sizeof(*pColors);
	pColors->iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 250, 254, 255));
	pColors->iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	pColors->iHighlightColor = __uiDesignColor(pNode, "appearance.highlightColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	pColors->iItemColor = __uiDesignColor(pNode, "appearance.itemColor", XUI_COLOR_RGBA(0, 0, 0, 0));
	pColors->iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255));
	pColors->iActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(217, 234, 254, 255));
	pColors->iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	pColors->iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	pColors->iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	pColors->iProgressTrackColor = __uiDesignColor(pNode, "appearance.progressTrackColor", XUI_COLOR_RGBA(228, 235, 243, 255));
	pColors->iProgressFillColor = __uiDesignColor(pNode, "appearance.progressFillColor", XUI_COLOR_RGBA(49, 126, 214, 255));
}

static void __uiDesignFillDockPanelMetrics(const ui_design_node_t* pNode, xui_dock_panel_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->fCaptionHeight = __uiDesignFloat(pNode, "metrics.captionHeight", 24.0f);
	pMetrics->fTabStripHeight = __uiDesignFloat(pNode, "metrics.tabStripHeight", 25.0f);
	pMetrics->fButtonSize = __uiDesignFloat(pNode, "metrics.buttonSize", 16.0f);
	pMetrics->fButtonGap = __uiDesignFloat(pNode, "metrics.buttonGap", 4.0f);
	pMetrics->fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	pMetrics->fSplitterSize = __uiDesignFloat(pNode, "metrics.splitterSize", 5.0f);
	pMetrics->fSplitterHitSize = __uiDesignFloat(pNode, "metrics.splitterHitSize", 7.0f);
	pMetrics->fMinPaneWidth = __uiDesignFloat(pNode, "metrics.minPaneWidth", 96.0f);
	pMetrics->fMinPaneHeight = __uiDesignFloat(pNode, "metrics.minPaneHeight", 72.0f);
	pMetrics->fTabMinWidth = __uiDesignFloat(pNode, "metrics.tabMinWidth", 68.0f);
	pMetrics->fTabMaxWidth = __uiDesignFloat(pNode, "metrics.tabMaxWidth", 150.0f);
	pMetrics->fTabPaddingX = __uiDesignFloat(pNode, "metrics.tabPaddingX", 12.0f);
	pMetrics->fFloatTitleHeight = __uiDesignFloat(pNode, "metrics.floatTitleHeight", 27.0f);
	pMetrics->fFloatBorderWidth = __uiDesignFloat(pNode, "metrics.floatBorderWidth", 1.0f);
	pMetrics->fAutoHideStripSize = __uiDesignFloat(pNode, "metrics.autoHideStripSize", 24.0f);
}

static void __uiDesignFillDockPanelColors(const ui_design_node_t* pNode, xui_dock_panel_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(232, 241, 250, 255));
	pColors->iPaneColor = __uiDesignColor(pNode, "appearance.paneColor", XUI_COLOR_RGBA(246, 250, 254, 255));
	pColors->iClientColor = __uiDesignColor(pNode, "appearance.clientColor", XUI_COLOR_RGBA(250, 253, 255, 255));
	pColors->iCaptionColor = __uiDesignColor(pNode, "appearance.captionColor", XUI_COLOR_RGBA(226, 238, 249, 255));
	pColors->iActiveCaptionColor = __uiDesignColor(pNode, "appearance.activeCaptionColor", XUI_COLOR_RGBA(46, 125, 215, 255));
	pColors->iCaptionTextColor = __uiDesignColor(pNode, "appearance.captionTextColor", XUI_COLOR_RGBA(45, 65, 88, 255));
	pColors->iActiveCaptionTextColor = __uiDesignColor(pNode, "appearance.activeCaptionTextColor", XUI_COLOR_WHITE);
	pColors->iTabColor = __uiDesignColor(pNode, "appearance.tabColor", XUI_COLOR_RGBA(235, 244, 252, 255));
	pColors->iTabHoverColor = __uiDesignColor(pNode, "appearance.tabHoverColor", XUI_COLOR_RGBA(219, 236, 250, 255));
	pColors->iActiveTabColor = __uiDesignColor(pNode, "appearance.activeTabColor", XUI_COLOR_WHITE);
	pColors->iTabTextColor = __uiDesignColor(pNode, "appearance.tabTextColor", XUI_COLOR_RGBA(55, 76, 99, 255));
	pColors->iActiveTabTextColor = __uiDesignColor(pNode, "appearance.activeTabTextColor", XUI_COLOR_RGBA(35, 55, 76, 255));
	pColors->iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(145, 176, 207, 255));
	pColors->iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(47, 125, 214, 255));
	pColors->iSplitterColor = __uiDesignColor(pNode, "appearance.splitterColor", XUI_COLOR_RGBA(211, 226, 240, 255));
	pColors->iSplitterHoverColor = __uiDesignColor(pNode, "appearance.splitterHoverColor", XUI_COLOR_RGBA(177, 207, 235, 255));
	pColors->iSplitterActiveColor = __uiDesignColor(pNode, "appearance.splitterActiveColor", XUI_COLOR_RGBA(56, 138, 220, 255));
	pColors->iButtonColor = __uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(70, 99, 128, 255));
	pColors->iButtonHoverColor = __uiDesignColor(pNode, "appearance.buttonHoverColor", XUI_COLOR_RGBA(41, 118, 200, 255));
	pColors->iButtonActiveColor = __uiDesignColor(pNode, "appearance.buttonActiveColor", XUI_COLOR_RGBA(20, 88, 162, 255));
	pColors->iAutoHideColor = __uiDesignColor(pNode, "appearance.autoHideColor", XUI_COLOR_RGBA(225, 239, 250, 255));
	pColors->iAutoHideHoverColor = __uiDesignColor(pNode, "appearance.autoHideHoverColor", XUI_COLOR_RGBA(202, 226, 247, 255));
	pColors->iFloatTitleColor = __uiDesignColor(pNode, "appearance.floatTitleColor", XUI_COLOR_RGBA(47, 125, 214, 255));
	pColors->iFloatBorderColor = __uiDesignColor(pNode, "appearance.floatBorderColor", XUI_COLOR_RGBA(47, 125, 214, 255));
}

static void __uiDesignFillMenuMetrics(const ui_design_node_t* pNode, xui_menu_metrics_t* pMetrics)
{
	memset(pMetrics, 0, sizeof(*pMetrics));
	pMetrics->iSize = sizeof(*pMetrics);
	pMetrics->fItemHeight = __uiDesignFloat(pNode, "metrics.itemHeight", 26.0f);
	pMetrics->fSeparatorHeight = __uiDesignFloat(pNode, "metrics.separatorHeight", 8.0f);
	pMetrics->fPaddingX = __uiDesignFloat(pNode, "metrics.paddingX", 5.0f);
	pMetrics->fPaddingY = __uiDesignFloat(pNode, "metrics.paddingY", 4.0f);
	pMetrics->fMarkWidth = __uiDesignFloat(pNode, "metrics.markWidth", 22.0f);
	pMetrics->fIconWidth = __uiDesignFloat(pNode, "metrics.iconWidth", 0.0f);
	pMetrics->fShortcutGap = __uiDesignFloat(pNode, "metrics.shortcutGap", 24.0f);
	pMetrics->fArrowWidth = __uiDesignFloat(pNode, "metrics.arrowWidth", 18.0f);
	pMetrics->fMinWidth = __uiDesignFloat(pNode, "metrics.minWidth", 168.0f);
	pMetrics->fMaxHeight = __uiDesignFloat(pNode, "metrics.maxHeight", 0.0f);
}

static void __uiDesignFillMenuColors(const ui_design_node_t* pNode, xui_menu_colors_t* pColors)
{
	memset(pColors, 0, sizeof(*pColors));
	pColors->iSize = sizeof(*pColors);
	pColors->iPanelColor = __uiDesignColor(pNode, "appearance.panelColor", XUI_COLOR_RGBA(250, 253, 255, 255));
	pColors->iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(152, 180, 211, 255));
	pColors->iShadowColor = __uiDesignColor(pNode, "appearance.shadowColor", XUI_COLOR_RGBA(35, 63, 96, 40));
	pColors->iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255));
	pColors->iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	pColors->iHoverTextColor = __uiDesignColor(pNode, "appearance.hoverTextColor", XUI_COLOR_RGBA(31, 95, 168, 255));
	pColors->iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	pColors->iShortcutColor = __uiDesignColor(pNode, "appearance.shortcutColor", XUI_COLOR_RGBA(109, 127, 149, 255));
	pColors->iDangerTextColor = __uiDesignColor(pNode, "appearance.dangerTextColor", XUI_COLOR_RGBA(214, 72, 86, 255));
	pColors->iMarkColor = __uiDesignColor(pNode, "appearance.markColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	pColors->iSeparatorColor = __uiDesignColor(pNode, "appearance.separatorColor", XUI_COLOR_RGBA(208, 219, 232, 255));
	pColors->iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
}

static void __uiDesignClearRuntimeMenus(ui_design_node_t* pNode)
{
	int i;

	if ( pNode == NULL ) return;
	for ( i = 0; i < UI_DESIGN_RUNTIME_MENUBAR_MENUS; ++i ) {
		if ( pNode->arrRuntimeMenuPopup[i] != NULL ) {
			xuiWidgetDestroy(pNode->arrRuntimeMenuPopup[i]);
			pNode->arrRuntimeMenuPopup[i] = NULL;
		}
	}
}

static int __uiDesignMenuBarMenuMatches(const xui_menubar_item_t* pItem, int iIndex, const char* sMenu)
{
	int iValue;

	if ( (pItem == NULL) || (sMenu == NULL) || (sMenu[0] == 0) ) return 0;
	if ( (pItem->sText != NULL) && __uiDesignTextEqualsNoCase(pItem->sText, sMenu) ) return 1;
	if ( !__uiDesignLooksIntText(sMenu) ) return 0;
	iValue = __uiDesignParseIntText(sMenu, -9999);
	return (iValue == iIndex) || (iValue == iIndex + 1) || (iValue == pItem->iValue);
}

static int __uiDesignMenuItemMatches(const xui_menu_item_t* pItem, int iIndex, const char* sPath, const char* sMenu)
{
	int iValue;

	if ( (pItem == NULL) || (sMenu == NULL) || (sMenu[0] == 0) ) return 0;
	if ( (sPath != NULL) && (sPath[0] != 0) && __uiDesignTextEqualsNoCase(sPath, sMenu) ) return 1;
	if ( (pItem->sText != NULL) && __uiDesignTextEqualsNoCase(pItem->sText, sMenu) ) return 1;
	if ( !__uiDesignLooksIntText(sMenu) ) return 0;
	iValue = __uiDesignParseIntText(sMenu, -9999);
	return (iValue == iIndex) || (iValue == iIndex + 1) || (iValue == pItem->iValue);
}

static void __uiDesignMenuItemPath(char* sOut, int iCapacity, const char* sParentPath, const xui_menu_item_t* pItem, int iIndex)
{
	const char* sName;
	char sFallback[32];

	if ( (sOut == NULL) || (iCapacity <= 0) ) return;
	sOut[0] = 0;
	if ( pItem == NULL ) return;
	sName = pItem->sText;
	if ( (sName == NULL) || (sName[0] == 0) ) {
		snprintf(sFallback, sizeof(sFallback), "%d", iIndex + 1);
		sName = sFallback;
	}
	if ( (sParentPath != NULL) && (sParentPath[0] != 0) ) {
		snprintf(sOut, (size_t)iCapacity, "%s/%s", sParentPath, sName);
	} else {
		snprintf(sOut, (size_t)iCapacity, "%s", sName);
	}
	sOut[iCapacity - 1] = '\0';
}

static int __uiDesignBuildMenuBarMenuItems(ui_design_node_t* pNode, const xui_menubar_item_t* pBarItem, int iBarIndex, xui_menu_item_t* pItems, int iCapacity)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[10];
	int iCount;
	int iFieldCount;

	if ( (pNode == NULL) || (pBarItem == NULL) || (pItems == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.menus", g_sDefaultMenuBarMenus);
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount < 2) || !__uiDesignMenuBarMenuMatches(pBarItem, iBarIndex, arrFields[0]) ) continue;
		__uiDesignFillMenuItemFromFields(pNode, &pItems[iCount], arrFields, iFieldCount, 1, iCount + 1);
		++iCount;
	}
	return iCount;
}

static int __uiDesignBuildMenuSubmenuItems(ui_design_node_t* pNode, const xui_menubar_item_t* pBarItem, int iBarIndex,
	const xui_menu_item_t* pParentItem, int iParentIndex, const char* sParentPath, int bMenuBar, xui_menu_item_t* pItems, int iCapacity)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[11];
	const char* sParent;
	int iCount;
	int iFieldCount;
	int iOffset;

	if ( (pNode == NULL) || (pParentItem == NULL) || (pItems == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.submenus", bMenuBar ? g_sDefaultMenuBarSubmenus : g_sDefaultMenuSubmenus);
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( bMenuBar ) {
			if ( (iFieldCount < 3) || (pBarItem == NULL) || !__uiDesignMenuBarMenuMatches(pBarItem, iBarIndex, arrFields[0]) ) continue;
			sParent = arrFields[1];
			iOffset = 2;
		} else {
			if ( iFieldCount < 2 ) continue;
			sParent = arrFields[0];
			iOffset = 1;
		}
		if ( !__uiDesignMenuItemMatches(pParentItem, iParentIndex, sParentPath, sParent) ) continue;
		__uiDesignFillMenuItemFromFields(pNode, &pItems[iCount], arrFields, iFieldCount, iOffset, iCount + 1);
		++iCount;
	}
	return iCount;
}

static int __uiDesignStoreRuntimeMenuPopup(ui_design_node_t* pNode, xui_widget pPopup)
{
	int i;

	if ( (pNode == NULL) || (pPopup == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < UI_DESIGN_RUNTIME_MENUBAR_MENUS; ++i ) {
		if ( pNode->arrRuntimeMenuPopup[i] == NULL ) {
			pNode->arrRuntimeMenuPopup[i] = pPopup;
			return XUI_OK;
		}
	}
	return XUI_ERROR_OUT_OF_MEMORY;
}

static int __uiDesignCreateRuntimeMenu(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pOwner,
	const xui_menu_metrics_t* pMetrics, const xui_menu_colors_t* pColors, xui_menu_item_t* pItems, int iItemCount, xui_widget* ppMenu)
{
	xui_menu_desc_t tDesc;
	xui_widget pMenu;
	xui_widget pPopup;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pItems == NULL) || (iItemCount <= 0) || (ppMenu == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppMenu = NULL;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pOwner;
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.bHasMetrics = 1;
	tDesc.bHasColors = 1;
	if ( pMetrics != NULL ) tDesc.tMetrics = *pMetrics;
	else __uiDesignFillMenuMetrics(pNode, &tDesc.tMetrics);
	if ( pColors != NULL ) tDesc.tColors = *pColors;
	else __uiDesignFillMenuColors(pNode, &tDesc.tColors);
	pMenu = NULL;
	iRet = xuiMenuCreate(pApp->pContext, &pMenu, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiMenuSetItems(pMenu, pItems, iItemCount);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(xuiMenuGetPopupWidget(pMenu));
		return iRet;
	}
	pPopup = xuiMenuGetPopupWidget(pMenu);
	if ( pPopup == NULL ) {
		xuiWidgetDestroy(pMenu);
		return XUI_ERROR_NOT_INITIALIZED;
	}
	iRet = __uiDesignStoreRuntimeMenuPopup(pNode, pPopup);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pPopup);
		return iRet;
	}
	*ppMenu = pMenu;
	return XUI_OK;
}

static int __uiDesignApplyMenuItemSubmenus(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pOwner,
	const xui_menubar_item_t* pBarItem, int iBarIndex, int bMenuBar, xui_menu_item_t* pItems, int iItemCount,
	const char* sParentPath, const xui_menu_metrics_t* pMetrics, const xui_menu_colors_t* pColors)
{
	xui_menu_item_t arrSubmenuItems[XUI_MENU_ITEM_CAPACITY];
	xui_widget pSubmenu;
	char sItemPath[256];
	int iSubmenuItemCount;
	int iRet;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) || (pItems == NULL) || (iItemCount < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < iItemCount; ++i ) {
		pItems[i].pSubmenu = NULL;
		if ( pItems[i].iType != XUI_MENU_ITEM_SUBMENU ) continue;
		__uiDesignMenuItemPath(sItemPath, sizeof(sItemPath), sParentPath, &pItems[i], i);
		iSubmenuItemCount = __uiDesignBuildMenuSubmenuItems(pNode, pBarItem, iBarIndex, &pItems[i], i, sItemPath, bMenuBar,
			arrSubmenuItems, UI_DESIGN_COUNT_OF(arrSubmenuItems));
		if ( iSubmenuItemCount <= 0 ) continue;
		iRet = __uiDesignApplyMenuItemSubmenus(pApp, pNode, pOwner, pBarItem, iBarIndex, bMenuBar, arrSubmenuItems,
			iSubmenuItemCount, sItemPath, pMetrics, pColors);
		if ( iRet != XUI_OK ) return iRet;
		pSubmenu = NULL;
		iRet = __uiDesignCreateRuntimeMenu(pApp, pNode, pOwner, pMetrics, pColors, arrSubmenuItems, iSubmenuItemCount, &pSubmenu);
		if ( iRet != XUI_OK ) return iRet;
		pItems[i].pSubmenu = pSubmenu;
	}
	return XUI_OK;
}

static int __uiDesignApplyMenuBarMenus(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pMenuBar, xui_menubar_item_t* pItems, int iItemCount)
{
	xui_menu_item_t arrMenuItems[XUI_MENU_ITEM_CAPACITY];
	xui_menu_metrics_t tMetrics;
	xui_menu_colors_t tColors;
	xui_widget pMenu;
	int iMenuItemCount;
	int iRet;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) || (pMenuBar == NULL) || (pItems == NULL) || (iItemCount < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignClearRuntimeMenus(pNode);
	__uiDesignFillMenuMetrics(pNode, &tMetrics);
	__uiDesignFillMenuColors(pNode, &tColors);
	for ( i = 0; i < iItemCount && i < UI_DESIGN_RUNTIME_MENUBAR_MENUS; ++i ) {
		pItems[i].pMenu = NULL;
		iMenuItemCount = __uiDesignBuildMenuBarMenuItems(pNode, &pItems[i], i, arrMenuItems, UI_DESIGN_COUNT_OF(arrMenuItems));
		if ( iMenuItemCount <= 0 ) continue;
		iRet = __uiDesignApplyMenuItemSubmenus(pApp, pNode, pMenuBar, &pItems[i], i, 1, arrMenuItems, iMenuItemCount, "", &tMetrics, &tColors);
		if ( iRet != XUI_OK ) return iRet;
		pMenu = NULL;
		iRet = __uiDesignCreateRuntimeMenu(pApp, pNode, pMenuBar, &tMetrics, &tColors, arrMenuItems, iMenuItemCount, &pMenu);
		if ( iRet != XUI_OK ) return iRet;
		pItems[i].pMenu = pMenu;
	}
	return XUI_OK;
}

static void __uiDesignFillPopupDesc(const ui_design_node_t* pNode, xui_popup_desc_t* pDesc)
{
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iSize = sizeof(*pDesc);
	pDesc->fContentWidth = __uiDesignFloat(pNode, "metrics.contentWidth", 260.0f);
	pDesc->fContentHeight = __uiDesignFloat(pNode, "metrics.contentHeight", 180.0f);
	pDesc->fMaxWidth = __uiDesignFloat(pNode, "metrics.maxWidth", 0.0f);
	pDesc->fMaxHeight = __uiDesignFloat(pNode, "metrics.maxHeight", 0.0f);
	pDesc->fGap = __uiDesignFloat(pNode, "metrics.gap", 4.0f);
	pDesc->fOffsetX = __uiDesignFloat(pNode, "metrics.offsetX", 0.0f);
	pDesc->fOffsetY = __uiDesignFloat(pNode, "metrics.offsetY", 0.0f);
	pDesc->fMargin = __uiDesignFloat(pNode, "metrics.margin", 6.0f);
	pDesc->fPadding = __uiDesignFloat(pNode, "metrics.padding", 5.0f);
	pDesc->fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	pDesc->fShadowSize = __uiDesignFloat(pNode, "metrics.shadowSize", 4.0f);
	pDesc->iAnchor = __uiDesignInt(pNode, "behavior.anchor", XUI_POPUP_ANCHOR_BOTTOM_LEFT);
	pDesc->iDirection = __uiDesignInt(pNode, "behavior.direction", XUI_POPUP_DIRECTION_RIGHT_DOWN);
	pDesc->iOutsidePolicy = __uiDesignInt(pNode, "behavior.outsidePolicy", XUI_POPUP_OUTSIDE_CLOSE);
	pDesc->iOwnerPolicy = __uiDesignInt(pNode, "behavior.ownerPolicy", XUI_POPUP_OWNER_PASSTHROUGH);
	pDesc->iEscapePolicy = __uiDesignInt(pNode, "behavior.escapePolicy", XUI_POPUP_ESCAPE_CLOSE);
	pDesc->iFocusPolicy = __uiDesignInt(pNode, "behavior.focusPolicy", XUI_POPUP_FOCUS_POPUP);
	pDesc->bOpen = 0;
	pDesc->bModal = __uiDesignBool(pNode, "behavior.modal", 0);
	pDesc->bConsumeInside = __uiDesignBool(pNode, "behavior.consumeInside", 0);
	pDesc->bMatchOwnerWidth = __uiDesignBool(pNode, "behavior.matchOwnerWidth", 0);
	pDesc->iScrollbarMode = __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_COMPACT);
	pDesc->fScrollbarSize = __uiDesignFloat(pNode, "metrics.scrollbarSize", 8.0f);
	pDesc->iPanelColor = __uiDesignColor(pNode, "appearance.panelColor", XUI_COLOR_RGBA(250, 253, 255, 255));
	pDesc->iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(152, 180, 211, 255));
	pDesc->iShadowColor = __uiDesignColor(pNode, "appearance.shadowColor", XUI_COLOR_RGBA(35, 63, 96, 40));
	pDesc->iBackdropColor = __uiDesignColor(pNode, "appearance.backdropColor", XUI_COLOR_RGBA(0, 0, 0, 0));
	pDesc->iTrackColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(226, 238, 249, 255));
	pDesc->iThumbColor = __uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(91, 139, 190, 210));
	pDesc->iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(68, 128, 198, 235));
	pDesc->iActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(40, 104, 174, 255));
	pDesc->iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(54, 148, 224, 255));
	pDesc->iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(176, 190, 206, 150));
}

static void __uiDesignApplyPopupScrollStyle(const ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_widget pScrollView;
	int iMode;
	float fSize;

	if ( (pNode == NULL) || (pWidget == NULL) ) return;
	pScrollView = xuiPopupGetScrollViewWidget(pWidget);
	if ( pScrollView == NULL ) return;
	iMode = __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_COMPACT);
	fSize = __uiDesignFloat(pNode, "metrics.scrollbarSize", 8.0f);
	(void)xuiPopupSetScrollbarStyle(pWidget, iMode, fSize);
	(void)xuiScrollViewSetColors(pScrollView,
		__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(226, 238, 249, 255)),
		__uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(91, 139, 190, 210)),
		__uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(68, 128, 198, 235)),
		__uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(40, 104, 174, 255)),
		__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(54, 148, 224, 255)),
		__uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(176, 190, 206, 150)));
}

static xui_font __uiDesignPreviewChildFont(ui_design_app_t* pApp, xui_widget pParent)
{
	xui_widget pScan;
	xui_font pFont;

	if ( pApp == NULL ) return NULL;
	for ( pScan = (pParent != NULL) ? xuiWidgetGetParent(pParent) : NULL; pScan != NULL; pScan = xuiWidgetGetParent(pScan) ) {
		pFont = xuiPanelGetFont(pScan);
		if ( pFont != NULL ) return pFont;
	}
	return pApp->pFont;
}

static int __uiDesignAddPreviewLabelEx(ui_design_app_t* pApp, xui_widget pParent, const char* sText, xui_rect_t tRect, uint32_t iTextColor, uint32_t iTextFlags)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( (pApp == NULL) || (pParent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = __uiDesignPreviewChildFont(pApp, pParent);
	tDesc.iTextColor = iTextColor;
	tDesc.iTextFlags = iTextFlags | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pApp->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pLabel, tRect);
	iRet = xuiWidgetAddChild(pParent, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
	}
	return iRet;
}

static int __uiDesignAddPreviewLabel(ui_design_app_t* pApp, xui_widget pParent, const char* sText, xui_rect_t tRect)
{
	return __uiDesignAddPreviewLabelEx(pApp, pParent, sText, tRect,
		XUI_COLOR_RGBA(53, 73, 96, 255),
		XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
}

static int __uiDesignAddPreviewPanel(ui_design_app_t* pApp, xui_widget pParent, xui_rect_t tRect,
	uint32_t iFillColor, uint32_t iBorderColor, float fBorderWidth, xui_widget* ppPanel)
{
	xui_panel_desc_t tDesc;
	xui_widget pPanel;
	int iRet;

	if ( ppPanel != NULL ) *ppPanel = NULL;
	if ( (pApp == NULL) || (pParent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignPreviewChildFont(pApp, pParent);
	tDesc.iBackgroundColor = iFillColor;
	tDesc.iHeaderColor = iFillColor;
	tDesc.iClientColor = iFillColor;
	tDesc.iBorderColor = iBorderColor;
	tDesc.fHeaderHeight = 0.0f;
	tDesc.fBorderWidth = fBorderWidth;
	tDesc.bClipClient = 1;
	iRet = xuiPanelCreate(pApp->pContext, &pPanel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiPanelSetHeaderHeight(pPanel, 0.0f);
	(void)xuiPanelSetHeaderGap(pPanel, 0.0f);
	(void)xuiPanelSetBorder(pPanel, fBorderWidth, iBorderColor);
	(void)xuiWidgetSetRect(pPanel, tRect);
	(void)xuiWidgetSetHitTestVisible(pPanel, 0);
	iRet = xuiWidgetAddChild(pParent, pPanel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pPanel);
		return iRet;
	}
	if ( ppPanel != NULL ) *ppPanel = pPanel;
	return XUI_OK;
}

static int __uiDesignAddPreviewImage(ui_design_app_t* pApp, xui_widget pParent, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tRect, uint32_t iTint)
{
	xui_image_desc_t tDesc;
	xui_widget pImage;
	int iRet;

	if ( (pApp == NULL) || (pParent == NULL) || (pSurface == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pSurface = pSurface;
	tDesc.tSrc = tSrc;
	tDesc.iColor = iTint;
	tDesc.iMode = XUI_IMAGE_CONTAIN;
	tDesc.iAlignX = XUI_ALIGN_CENTER;
	tDesc.iAlignY = XUI_ALIGN_CENTER;
	iRet = xuiImageCreate(pApp->pContext, &pImage, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pImage, tRect);
	iRet = xuiWidgetAddChild(pParent, pImage);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pImage);
	}
	return iRet;
}

static int __uiDesignAddPreviewButtonEx(ui_design_app_t* pApp, xui_widget pParent, const char* sText, xui_rect_t tRect,
	uint32_t iTextColor, uint32_t iDisabledTextColor, uint32_t iNormalColor, uint32_t iHoverColor,
	uint32_t iActiveColor, uint32_t iFocusColor, uint32_t iDisabledColor, uint32_t iBorderColor,
	float fBorderWidth, int iSemantic)
{
	xui_button_desc_t tDesc;
	xui_widget pButton;
	int iRet;

	if ( (pApp == NULL) || (pParent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = __uiDesignPreviewChildFont(pApp, pParent);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiButtonCreate(pApp->pContext, &pButton, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	if ( iSemantic != XUI_BUTTON_SEMANTIC_DEFAULT ) (void)xuiButtonSetSemantic(pButton, iSemantic);
	(void)xuiButtonSetTextColor(pButton, iTextColor);
	(void)xuiButtonSetDisabledTextColor(pButton, iDisabledTextColor);
	(void)xuiButtonSetColors(pButton, iNormalColor, iHoverColor, iActiveColor, iFocusColor, iDisabledColor);
	(void)xuiButtonSetBorder(pButton, fBorderWidth, iBorderColor);
	(void)xuiWidgetSetRect(pButton, tRect);
	iRet = xuiWidgetAddChild(pParent, pButton);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pButton);
	}
	return iRet;
}

static int __uiDesignAddPreviewRect(ui_design_app_t* pApp, xui_widget pParent, xui_rect_t tRect,
	uint32_t iFillColor, uint32_t iBorderColor, float fBorderWidth);

static int __uiDesignAddPreviewButton(ui_design_app_t* pApp, xui_widget pParent, const char* sText, xui_rect_t tRect,
	uint32_t iTextColor, uint32_t iFillColor, uint32_t iBorderColor, float fBorderWidth)
{
	if ( (sText == NULL) || (sText[0] == '\0') ) {
		return __uiDesignAddPreviewRect(pApp, pParent, tRect, iFillColor, iBorderColor, fBorderWidth);
	}
	return __uiDesignAddPreviewButtonEx(pApp, pParent, sText, tRect,
		iTextColor,
		XUI_COLOR_RGBA(140, 154, 175, 255),
		iFillColor,
		iFillColor,
		iFillColor,
		iBorderColor,
		XUI_COLOR_RGBA(238, 242, 247, 255),
		iBorderColor,
		fBorderWidth,
		XUI_BUTTON_SEMANTIC_DEFAULT);
}

static int __uiDesignAddPreviewRect(ui_design_app_t* pApp, xui_widget pParent, xui_rect_t tRect,
	uint32_t iFillColor, uint32_t iBorderColor, float fBorderWidth)
{
	xui_widget pPanel;

	pPanel = NULL;
	return __uiDesignAddPreviewPanel(pApp, pParent, tRect, iFillColor, iBorderColor, fBorderWidth, &pPanel);
}

static int __uiDesignCreatePreviewLabel(ui_design_app_t* pApp, const char* sText, xui_widget* ppWidget)
{
	xui_label_desc_t tDesc;

	if ( (pApp == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pApp->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(53, 73, 96, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	return xuiLabelCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateDockClient(ui_design_app_t* pApp, xui_widget* ppWidget)
{
	int iRet;

	if ( (pApp == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiWidgetCreate(pApp->pContext, ppWidget);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(*ppWidget, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetFlowMode(*ppWidget, XUI_FLOW_ABSOLUTE);
	(void)xuiWidgetSetPadding(*ppWidget, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	(void)xuiWidgetSetGap(*ppWidget, 0.0f);
	return XUI_OK;
}

static void __uiDesignClearChildren(xui_widget pParent)
{
	xui_widget pChild;
	xui_widget pNext;

	if ( pParent == NULL ) return;
	pChild = xuiWidgetGetFirstChild(pParent);
	while ( pChild != NULL ) {
		pNext = xuiWidgetGetNextSibling(pChild);
		xuiWidgetDestroy(pChild);
		pChild = pNext;
	}
}

static int __uiDesignSplitDockFields(char* sLine, char** arrFields, int iCapacity)
{
	char* sColon;

	if ( (sLine == NULL) || (arrFields == NULL) || (iCapacity <= 0) ) return 0;
	if ( strchr(sLine, '|') != NULL ) return __uiDesignSplitFields(sLine, arrFields, iCapacity);
	sColon = strchr(sLine, ':');
	if ( sColon == NULL ) return __uiDesignSplitFields(sLine, arrFields, iCapacity);
	sColon[0] = 0;
	arrFields[0] = sLine;
	if ( iCapacity > 1 ) {
		arrFields[1] = sColon + 1;
		__uiDesignTrim(arrFields[0]);
		__uiDesignTrim(arrFields[1]);
		return 2;
	}
	__uiDesignTrim(arrFields[0]);
	return 1;
}

static int __uiDesignSplitPaneModeFromText(const char* sText, int* pMode)
{
	if ( pMode == NULL ) return 0;
	if ( __uiDesignTokenIs(sText, "grow") || __uiDesignTokenIs(sText, "weight") || __uiDesignTokenIs(sText, "stretch") ) {
		*pMode = XUI_SPLIT_PANE_GROW;
		return 1;
	}
	if ( __uiDesignTokenIs(sText, "fixed") || __uiDesignTokenIs(sText, "pixel") || __uiDesignTokenIs(sText, "px") ) {
		*pMode = XUI_SPLIT_PANE_FIXED;
		return 1;
	}
	if ( __uiDesignLooksIntText(sText) ) {
		*pMode = __uiDesignParseIntText(sText, XUI_SPLIT_PANE_GROW);
		return 1;
	}
	return 0;
}

static int __uiDesignBuildSplitPaneDefs(ui_design_node_t* pNode, ui_design_split_pane_def_t* pPanes, int iCapacity)
{
	const char* sCursor;
	char sLine[512];
	char* arrFields[8];
	int iCount;
	int iFieldCount;
	int iMode;

	if ( (pNode == NULL) || (pPanes == NULL) || (iCapacity <= 0) ) return 0;
	__uiDesignTextPoolReset(pNode);
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.panes", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount <= 0) || (arrFields[0][0] == 0) ) continue;
		memset(&pPanes[iCount], 0, sizeof(pPanes[iCount]));
		pPanes[iCount].sTitle = __uiDesignTextPoolAdd(pNode, arrFields[0]);
		pPanes[iCount].iMode = XUI_SPLIT_PANE_GROW;
		pPanes[iCount].fWeight = 1.0f;
		pPanes[iCount].fFixedSize = 0.0f;
		pPanes[iCount].fMinSize = 0.0f;
		pPanes[iCount].fMaxSize = 0.0f;
		if ( (iFieldCount > 1) && __uiDesignSplitPaneModeFromText(arrFields[1], &iMode) ) {
			pPanes[iCount].iMode = iMode;
			pPanes[iCount].bModeSet = 1;
		}
		pPanes[iCount].fWeight = (iFieldCount > 2) ? __uiDesignParseFloatText(arrFields[2], 1.0f) : 1.0f;
		pPanes[iCount].fFixedSize = (iFieldCount > 3) ? __uiDesignParseFloatText(arrFields[3], 0.0f) : 0.0f;
		pPanes[iCount].fMinSize = (iFieldCount > 4) ? __uiDesignParseFloatText(arrFields[4], 0.0f) : 0.0f;
		pPanes[iCount].fMaxSize = (iFieldCount > 5) ? __uiDesignParseFloatText(arrFields[5], 0.0f) : 0.0f;
		if ( pPanes[iCount].fWeight <= 0.0f ) pPanes[iCount].fWeight = 1.0f;
		++iCount;
	}
	return iCount;
}

static int __uiDesignContainerHasModelChildren(const ui_design_app_t* pApp, const ui_design_node_t* pNode)
{
	int i;

	if ( (pApp == NULL) || (pNode == NULL) ) return 0;
	for ( i = 0; i < pApp->tModel.iNodeCount; ++i ) {
		if ( pApp->tModel.arrNodes[i].iParentId == pNode->iId ) return 1;
	}
	return 0;
}

static void __uiDesignDetachContainerModelChildren(ui_design_app_t* pApp, const ui_design_node_t* pNode)
{
	ui_design_node_t* pChild;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) ) return;
	for ( i = 0; i < pApp->tModel.iNodeCount; ++i ) {
		pChild = &pApp->tModel.arrNodes[i];
		if ( (pChild->iParentId == pNode->iId) && (pChild->pWidget != NULL) && (xuiWidgetGetParent(pChild->pWidget) != NULL) ) {
			(void)xuiWidgetRemoveFromParent(pChild->pWidget);
		}
	}
}

static int __uiDesignClampSlot(int iValue, int iCount)
{
	if ( iCount <= 0 ) return 0;
	if ( iValue < 0 ) return 0;
	if ( iValue >= iCount ) return iCount - 1;
	return iValue;
}

static void __uiDesignApplyContainerHostLayout(ui_design_node_t* pNode, xui_widget pHost, int bApplyOverflow)
{
	xui_thickness_t tPadding;
	xui_table_track_t tTrack;
	float fA;
	float fB;
	int iA;

	if ( (pNode == NULL) || (pHost == NULL) ) return;
	(void)xuiWidgetSetLayoutType(pHost, __uiDesignInt(pNode, "layout.type", XUI_LAYOUT_MANUAL));
	if ( bApplyOverflow ) {
		(void)xuiWidgetSetOverflow(pHost, __uiDesignInt(pNode, "layout.overflow", XUI_OVERFLOW_VISIBLE));
	}
	(void)xuiWidgetSetGap(pHost, __uiDesignFloat(pNode, "layout.gap", 0.0f));
	tPadding = (xui_thickness_t){
		__uiDesignFloat(pNode, "layout.paddingLeft", 0.0f),
		__uiDesignFloat(pNode, "layout.paddingTop", 0.0f),
		__uiDesignFloat(pNode, "layout.paddingRight", 0.0f),
		__uiDesignFloat(pNode, "layout.paddingBottom", 0.0f)
	};
	(void)xuiWidgetSetPadding(pHost, tPadding);
	(void)xuiWidgetSetTableSize(pHost, __uiDesignInt(pNode, "layout.tableRows", 1), __uiDesignInt(pNode, "layout.tableColumns", 1));
	if ( __uiDesignTryInt(pNode, "layout.gridColumns", &iA) ||
	     __uiDesignTryFloat(pNode, "layout.gridItemWidth", &fA) ||
	     __uiDesignTryFloat(pNode, "layout.gridItemHeight", &fB) ) {
		(void)xuiWidgetSetGridMetrics(pHost,
			__uiDesignInt(pNode, "layout.gridColumns", 1),
			__uiDesignFloat(pNode, "layout.gridItemWidth", 0.0f),
			__uiDesignFloat(pNode, "layout.gridItemHeight", 0.0f));
	}
	if ( __uiDesignHasProperty(pNode, "layout.tableRowSizeMode") || __uiDesignHasProperty(pNode, "layout.tableRowValue") ) {
		tTrack = __uiDesignTableTrackFromValue(
			__uiDesignInt(pNode, "layout.tableRowSizeMode", XUI_SIZE_FIXED),
			__uiDesignFloat(pNode, "layout.tableRowValue", 0.0f));
		(void)xuiWidgetSetTableRow(pHost, __uiDesignInt(pNode, "layout.tableTrackRow", 0), &tTrack);
	}
	if ( __uiDesignHasProperty(pNode, "layout.tableColumnSizeMode") || __uiDesignHasProperty(pNode, "layout.tableColumnValue") ) {
		tTrack = __uiDesignTableTrackFromValue(
			__uiDesignInt(pNode, "layout.tableColumnSizeMode", XUI_SIZE_FIXED),
			__uiDesignFloat(pNode, "layout.tableColumnValue", 0.0f));
		(void)xuiWidgetSetTableColumn(pHost, __uiDesignInt(pNode, "layout.tableTrackColumn", 0), &tTrack);
	}
	__uiDesignApplyTableTrackConfig(pNode, pHost, "layout.tableRowsConfig", 1);
	__uiDesignApplyTableTrackConfig(pNode, pHost, "layout.tableColumnsConfig", 0);
}

static int __uiDesignAttachContainerModelChild(ui_design_app_t* pApp, ui_design_node_t* pParent, ui_design_node_t* pChild)
{
	xui_widget pContent;
	int iSlot;
	int iCount;
	int iRet;

	if ( (pApp == NULL) || (pParent == NULL) || (pChild == NULL) || (pParent->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pChild->pWidget == NULL ) {
		iRet = uiDesignAppCreateNodeWidget(pApp, pChild);
		if ( iRet != XUI_OK ) return iRet;
		if ( pChild->pWidget == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	}
	if ( xuiWidgetGetParent(pChild->pWidget) != NULL ) {
		(void)xuiWidgetRemoveFromParent(pChild->pWidget);
	}
	switch ( pParent->iType ) {
	case UI_DESIGN_NODE_CAROUSEL:
		iCount = xuiCarouselGetPageCount(pParent->pWidget);
		iSlot = __uiDesignClampSlot(__uiDesignInt(pChild, "layout.carouselPage", 0), iCount);
		return xuiCarouselAddPageChild(pParent->pWidget, iSlot, pChild->pWidget);
	case UI_DESIGN_NODE_SPLIT_LAYOUT:
		iCount = xuiSplitLayoutGetPaneCount(pParent->pWidget);
		iSlot = __uiDesignClampSlot(__uiDesignInt(pChild, "layout.splitPane", 0), iCount);
		return xuiSplitLayoutAddPaneChild(pParent->pWidget, iSlot, pChild->pWidget);
	case UI_DESIGN_NODE_TABS:
		iCount = xuiTabsGetItemCount(pParent->pWidget);
		iSlot = __uiDesignClampSlot(__uiDesignInt(pChild, "layout.tabPage", 0), iCount);
		pContent = xuiTabsGetPageWidget(pParent->pWidget, iSlot);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignApplyContainerHostLayout(pParent, pContent, 1);
		return xuiWidgetAddChild(pContent, pChild->pWidget);
	case UI_DESIGN_NODE_ACCORDION:
		iCount = xuiAccordionGetSectionCount(pParent->pWidget);
		iSlot = __uiDesignClampSlot(__uiDesignInt(pChild, "layout.accordionSection", 0), iCount);
		return xuiAccordionAddSectionChild(pParent->pWidget, iSlot, pChild->pWidget);
	case UI_DESIGN_NODE_RADIO_GROUP:
		if ( pChild->iType == UI_DESIGN_NODE_RADIO ) return xuiRadioGroupAddRadio(pParent->pWidget, pChild->pWidget);
		if ( pChild->iType == UI_DESIGN_NODE_CHECK_CARD ) return xuiRadioGroupAddCheckCard(pParent->pWidget, pChild->pWidget);
		return xuiWidgetAddChild(pParent->pWidget, pChild->pWidget);
	case UI_DESIGN_NODE_PANEL:
		pContent = xuiPanelGetClientWidget(pParent->pWidget);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignApplyContainerHostLayout(pParent, pContent, 0);
		return xuiPanelAddChild(pParent->pWidget, pChild->pWidget);
	case UI_DESIGN_NODE_WINDOW:
		pContent = xuiWindowGetClientWidget(pParent->pWidget);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignApplyContainerHostLayout(pParent, pContent, 1);
		return xuiWindowAddChild(pParent->pWidget, pChild->pWidget);
	case UI_DESIGN_NODE_SCROLL_FRAME:
		pContent = xuiScrollFrameGetViewportWidget(pParent->pWidget);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignApplyContainerHostLayout(pParent, pContent, 0);
		return xuiWidgetAddChild(pContent, pChild->pWidget);
	case UI_DESIGN_NODE_SCROLL_VIEW:
		pContent = xuiScrollViewGetContentWidget(pParent->pWidget);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignApplyContainerHostLayout(pParent, pContent, 1);
		return xuiWidgetAddChild(pContent, pChild->pWidget);
	case UI_DESIGN_NODE_DOCK_PANEL:
		iCount = xuiDockPanelGetWindowCount(pParent->pWidget);
		if ( iCount <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
		iSlot = __uiDesignClampSlot(__uiDesignInt(pChild, "layout.dockWindow", 0), iCount);
		pContent = xuiDockPanelGetWindowClient(pParent->pWidget, iSlot);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignApplyContainerHostLayout(pParent, pContent, 1);
		return xuiWidgetAddChild(pContent, pChild->pWidget);
	case UI_DESIGN_NODE_POPUP:
		pContent = xuiPopupGetContentWidget(pParent->pWidget);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignApplyContainerHostLayout(pParent, pContent, 1);
		return xuiWidgetAddChild(pContent, pChild->pWidget);
	default:
		return xuiWidgetAddChild(pParent->pWidget, pChild->pWidget);
	}
}

static int __uiDesignAttachContainerModelChildren(ui_design_app_t* pApp, ui_design_node_t* pParent)
{
	ui_design_node_t* pChild;
	int i;
	int iRet;

	if ( (pApp == NULL) || (pParent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pApp->tModel.iNodeCount; ++i ) {
		pChild = &pApp->tModel.arrNodes[i];
		if ( pChild->iParentId != pParent->iId ) continue;
		iRet = __uiDesignAttachContainerModelChild(pApp, pParent, pChild);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __uiDesignRefreshContainerModelChildren(ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	if ( !__uiDesignContainerHasModelChildren(pApp, pNode) ) return XUI_OK;
	__uiDesignDetachContainerModelChildren(pApp, pNode);
	return __uiDesignAttachContainerModelChildren(pApp, pNode);
}

static int __uiDesignApplySplitLayoutPanes(ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	ui_design_split_pane_def_t arrPanes[XUI_SPLIT_LAYOUT_MAX_PANES];
	xui_widget pPane;
	char sPropertyId[64];
	char sTitle[64];
	int bHasModelChildren;
	int iPaneCount;
	int iDefCount;
	int iMode;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iDefCount = __uiDesignBuildSplitPaneDefs(pNode, arrPanes, UI_DESIGN_COUNT_OF(arrPanes));
	iPaneCount = __uiDesignInt(pNode, "data.paneCount", (iDefCount > 0) ? iDefCount : 2);
	if ( iPaneCount < iDefCount ) iPaneCount = iDefCount;
	if ( iPaneCount < 1 ) iPaneCount = 1;
	if ( iPaneCount > XUI_SPLIT_LAYOUT_MAX_PANES ) iPaneCount = XUI_SPLIT_LAYOUT_MAX_PANES;
	(void)xuiSplitLayoutSetPaneCount(pNode->pWidget, iPaneCount);
	bHasModelChildren = __uiDesignContainerHasModelChildren(pApp, pNode);
	for ( i = 0; i < iPaneCount; ++i ) {
		if ( i < iDefCount && arrPanes[i].bModeSet ) {
			iMode = arrPanes[i].iMode;
		} else {
			snprintf(sPropertyId, sizeof(sPropertyId), "behavior.pane%dMode", i);
			iMode = __uiDesignInt(pNode, sPropertyId, XUI_SPLIT_PANE_GROW);
		}
		(void)xuiSplitLayoutSetPaneMode(pNode->pWidget, i, iMode);
		(void)xuiSplitLayoutSetPaneWeight(pNode->pWidget, i, (i < iDefCount) ? arrPanes[i].fWeight : 1.0f);
		(void)xuiSplitLayoutSetPaneFixedSize(pNode->pWidget, i, (i < iDefCount) ? arrPanes[i].fFixedSize : 0.0f);
		(void)xuiSplitLayoutSetPaneMinSize(pNode->pWidget, i, (i < iDefCount) ? arrPanes[i].fMinSize : 0.0f);
		(void)xuiSplitLayoutSetPaneMaxSize(pNode->pWidget, i, (i < iDefCount) ? arrPanes[i].fMaxSize : 0.0f);
		pPane = xuiSplitLayoutGetPaneWidget(pNode->pWidget, i);
		if ( pPane != NULL ) {
			__uiDesignClearChildren(pPane);
			if ( bHasModelChildren ) {
				continue;
			}
			if ( i < iDefCount && arrPanes[i].sTitle != NULL && arrPanes[i].sTitle[0] != 0 ) {
				(void)__uiDesignAddPreviewLabelEx(pApp, pPane, arrPanes[i].sTitle, (xui_rect_t){8.0f, 8.0f, 130.0f, 24.0f},
					XUI_COLOR_RGBA(53, 73, 96, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			} else {
				snprintf(sTitle, sizeof(sTitle), "Pane %d", i + 1);
				(void)__uiDesignAddPreviewLabelEx(pApp, pPane, sTitle, (xui_rect_t){8.0f, 8.0f, 130.0f, 24.0f},
					XUI_COLOR_RGBA(53, 73, 96, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			}
		}
	}
	return XUI_OK;
}

static int __uiDesignDockSideFromText(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "left") ) return XUI_DOCK_PANEL_SIDE_LEFT;
	if ( __uiDesignTokenIs(sText, "right") ) return XUI_DOCK_PANEL_SIDE_RIGHT;
	if ( __uiDesignTokenIs(sText, "top") ) return XUI_DOCK_PANEL_SIDE_TOP;
	if ( __uiDesignTokenIs(sText, "bottom") ) return XUI_DOCK_PANEL_SIDE_BOTTOM;
	if ( __uiDesignTokenIs(sText, "fill") || __uiDesignTokenIs(sText, "center") || __uiDesignTokenIs(sText, "document") || __uiDesignTokenIs(sText, "doc") ) return XUI_DOCK_PANEL_SIDE_FILL;
	if ( __uiDesignTokenIs(sText, "hidden") || __uiDesignTokenIs(sText, "none") ) return XUI_DOCK_PANEL_SIDE_NONE;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static int __uiDesignDockRegionFromText(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "document") || __uiDesignTokenIs(sText, "fill") || __uiDesignTokenIs(sText, "center") || __uiDesignTokenIs(sText, "doc") ) return XUI_DOCK_PANEL_REGION_DOCUMENT;
	if ( __uiDesignTokenIs(sText, "left") ) return XUI_DOCK_PANEL_REGION_LEFT;
	if ( __uiDesignTokenIs(sText, "right") ) return XUI_DOCK_PANEL_REGION_RIGHT;
	if ( __uiDesignTokenIs(sText, "top") ) return XUI_DOCK_PANEL_REGION_TOP;
	if ( __uiDesignTokenIs(sText, "bottom") ) return XUI_DOCK_PANEL_REGION_BOTTOM;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static int __uiDesignDockSizeModeFromText(const char* sText, int iDefault)
{
	if ( __uiDesignTokenIs(sText, "portion") || __uiDesignTokenIs(sText, "ratio") || __uiDesignTokenIs(sText, "percent") ) return XUI_DOCK_PANEL_SIZE_PORTION;
	if ( __uiDesignTokenIs(sText, "pixel") || __uiDesignTokenIs(sText, "pixels") || __uiDesignTokenIs(sText, "px") || __uiDesignTokenIs(sText, "fixed") ) return XUI_DOCK_PANEL_SIZE_PIXEL;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static float __uiDesignDockDefaultRatio(ui_design_node_t* pNode, int iSide)
{
	switch ( iSide ) {
	case XUI_DOCK_PANEL_SIDE_LEFT:
		return __uiDesignFloat(pNode, "metrics.leftRatio", 0.22f);
	case XUI_DOCK_PANEL_SIDE_RIGHT:
		return __uiDesignFloat(pNode, "metrics.rightRatio", 0.24f);
	case XUI_DOCK_PANEL_SIDE_BOTTOM:
		return __uiDesignFloat(pNode, "metrics.bottomRatio", 0.24f);
	case XUI_DOCK_PANEL_SIDE_TOP:
		return 0.24f;
	default:
		return 0.0f;
	}
}

static int __uiDesignBuildDockWindowDefs(ui_design_node_t* pNode, ui_design_dock_window_def_t* pWindows, int iCapacity)
{
	const char* sCursor;
	char sLine[512];
	char* arrFields[16];
	int iCount;
	int iFieldCount;

	if ( (pNode == NULL) || (pWindows == NULL) || (iCapacity <= 0) ) return 0;
	__uiDesignTextPoolReset(pNode);
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.windows", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitDockFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount <= 0) || (arrFields[0][0] == 0) ) continue;
		memset(&pWindows[iCount], 0, sizeof(pWindows[iCount]));
		pWindows[iCount].sTitle = __uiDesignTextPoolAdd(pNode, arrFields[0]);
		pWindows[iCount].iSide = (iFieldCount > 1) ? __uiDesignDockSideFromText(arrFields[1], XUI_DOCK_PANEL_SIDE_FILL) : XUI_DOCK_PANEL_SIDE_FILL;
		pWindows[iCount].fRatio = (iFieldCount > 2) ? __uiDesignParseFloatText(arrFields[2], __uiDesignDockDefaultRatio(pNode, pWindows[iCount].iSide)) : __uiDesignDockDefaultRatio(pNode, pWindows[iCount].iSide);
		pWindows[iCount].bClosable = (iFieldCount > 3) ? __uiDesignParseBoolText(arrFields[3], 1) : 1;
		pWindows[iCount].bDockable = (iFieldCount > 4) ? __uiDesignParseBoolText(arrFields[4], 1) : 1;
		pWindows[iCount].sContent = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 5, arrFields[0]));
		pWindows[iCount].sPane = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 6, ""));
		pWindows[iCount].bActive = (iFieldCount > 7) ? __uiDesignParseBoolText(arrFields[7], 0) : 0;
		pWindows[iCount].bAutoHide = (iFieldCount > 8) ? __uiDesignParseBoolText(arrFields[8], 0) : 0;
		pWindows[iCount].iRegion = (iFieldCount > 9) ? __uiDesignDockRegionFromText(arrFields[9], XUI_DOCK_PANEL_REGION_DOCUMENT) : XUI_DOCK_PANEL_REGION_DOCUMENT;
		pWindows[iCount].bHidden = (iFieldCount > 10) ? __uiDesignParseBoolText(arrFields[10], 0) : 0;
		pWindows[iCount].bFloating = (iFieldCount > 11) ? __uiDesignParseBoolText(arrFields[11], 0) : 0;
		pWindows[iCount].tFloatRect.fX = (iFieldCount > 12) ? __uiDesignParseFloatText(arrFields[12], 80.0f) : 80.0f;
		pWindows[iCount].tFloatRect.fY = (iFieldCount > 13) ? __uiDesignParseFloatText(arrFields[13], 70.0f) : 70.0f;
		pWindows[iCount].tFloatRect.fW = (iFieldCount > 14) ? __uiDesignParseFloatText(arrFields[14], 260.0f) : 260.0f;
		pWindows[iCount].tFloatRect.fH = (iFieldCount > 15) ? __uiDesignParseFloatText(arrFields[15], 180.0f) : 180.0f;
		if ( pWindows[iCount].iRegion < 0 || pWindows[iCount].iRegion >= XUI_DOCK_PANEL_REGION_COUNT ) pWindows[iCount].iRegion = XUI_DOCK_PANEL_REGION_DOCUMENT;
		if ( pWindows[iCount].tFloatRect.fW < 80.0f ) pWindows[iCount].tFloatRect.fW = 80.0f;
		if ( pWindows[iCount].tFloatRect.fH < 60.0f ) pWindows[iCount].tFloatRect.fH = 60.0f;
		pWindows[iCount].iWindow = -1;
		pWindows[iCount].iPane = -1;
		++iCount;
	}
	return iCount;
}

static int __uiDesignFindDockPaneByName(ui_design_dock_window_def_t* pWindows, int iCount, int iLimit, const char* sPane)
{
	int i;

	if ( (pWindows == NULL) || (sPane == NULL) || (sPane[0] == 0) ) return -1;
	if ( iLimit > iCount ) iLimit = iCount;
	for ( i = 0; i < iLimit; ++i ) {
		if ( pWindows[i].iPane >= 0 && __uiDesignTextEqualsNoCase(pWindows[i].sPane, sPane) ) return pWindows[i].iPane;
	}
	return -1;
}

static void __uiDesignDestroyDockClients(xui_widget pDock)
{
	xui_widget pClient;
	int iCount;
	int i;

	if ( pDock == NULL ) return;
	iCount = xuiDockPanelGetWindowCount(pDock);
	for ( i = 0; i < iCount; ++i ) {
		pClient = xuiDockPanelGetWindowClient(pDock, i);
		if ( pClient != NULL ) {
			(void)xuiDockPanelSetWindowClient(pDock, i, NULL);
			xuiWidgetDestroy(pClient);
		}
	}
}

static int __uiDesignApplyDockPanelWindows(ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	ui_design_dock_window_def_t arrWindows[XUI_DOCK_PANEL_WINDOW_CAPACITY];
	xui_widget pClient;
	int bHasModelChildren;
	int iWindowCount;
	int iPane;
	int iFill;
	int iRet;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iWindowCount = __uiDesignBuildDockWindowDefs(pNode, arrWindows, UI_DESIGN_COUNT_OF(arrWindows));
	bHasModelChildren = __uiDesignContainerHasModelChildren(pApp, pNode);
	if ( bHasModelChildren ) __uiDesignDetachContainerModelChildren(pApp, pNode);
	__uiDesignDestroyDockClients(pNode->pWidget);
	(void)xuiDockPanelClear(pNode->pWidget);
	if ( iWindowCount <= 0 ) return XUI_OK;
	iFill = -1;
	for ( i = 0; i < iWindowCount; ++i ) {
		if ( bHasModelChildren ) {
			iRet = __uiDesignCreateDockClient(pApp, &pClient);
		} else {
			iRet = __uiDesignCreatePreviewLabel(pApp, arrWindows[i].sContent, &pClient);
		}
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiDockPanelAddWindow(pNode->pWidget, arrWindows[i].sTitle, pClient, &arrWindows[i].iWindow);
		if ( iRet != XUI_OK ) {
			xuiWidgetDestroy(pClient);
			return iRet;
		}
		(void)xuiDockPanelSetWindowFlags(pNode->pWidget, arrWindows[i].iWindow, arrWindows[i].bClosable, arrWindows[i].bDockable);
		if ( (iFill < 0) && !arrWindows[i].bHidden && !arrWindows[i].bFloating && (arrWindows[i].iSide == XUI_DOCK_PANEL_SIDE_FILL) ) iFill = i;
	}
	if ( iFill < 0 ) {
		for ( i = 0; i < iWindowCount; ++i ) {
			if ( !arrWindows[i].bHidden && !arrWindows[i].bFloating ) {
				iFill = i;
				break;
			}
		}
	}
	if ( iFill < 0 ) {
		for ( i = 0; i < iWindowCount; ++i ) {
			if ( arrWindows[i].bFloating ) (void)xuiDockPanelFloatWindow(pNode->pWidget, arrWindows[i].iWindow, arrWindows[i].tFloatRect);
			if ( arrWindows[i].bHidden ) (void)xuiDockPanelHideWindow(pNode->pWidget, arrWindows[i].iWindow);
		}
		if ( bHasModelChildren ) {
			iRet = __uiDesignAttachContainerModelChildren(pApp, pNode);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_OK;
	}
	if ( arrWindows[iFill].iSide == XUI_DOCK_PANEL_SIDE_NONE ) arrWindows[iFill].iSide = XUI_DOCK_PANEL_SIDE_FILL;
	iRet = xuiDockPanelDockWindow(pNode->pWidget, arrWindows[iFill].iWindow, arrWindows[iFill].iRegion, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &arrWindows[iFill].iPane);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < iWindowCount; ++i ) {
		if ( i == iFill || arrWindows[i].bHidden || arrWindows[i].bFloating || arrWindows[i].iSide == XUI_DOCK_PANEL_SIDE_NONE ) continue;
		iPane = __uiDesignFindDockPaneByName(arrWindows, iWindowCount, i, arrWindows[i].sPane);
		if ( iPane >= 0 ) {
			iRet = xuiDockPanelDockWindowToPane(pNode->pWidget, arrWindows[i].iWindow, iPane);
			if ( iRet == XUI_OK ) arrWindows[i].iPane = iPane;
		} else {
			iRet = xuiDockPanelDockWindow(pNode->pWidget, arrWindows[i].iWindow, arrWindows[i].iRegion, arrWindows[i].iSide, arrWindows[i].fRatio, &arrWindows[i].iPane);
		}
		if ( iRet != XUI_OK ) return iRet;
	}
	(void)xuiDockPanelSetPaneActiveWindow(pNode->pWidget, arrWindows[iFill].iPane, arrWindows[iFill].iWindow);
	for ( i = 0; i < iWindowCount; ++i ) {
		if ( arrWindows[i].iPane < 0 ) continue;
		if ( arrWindows[i].bActive ) (void)xuiDockPanelSetPaneActiveWindow(pNode->pWidget, arrWindows[i].iPane, arrWindows[i].iWindow);
	}
	for ( i = 0; i < iWindowCount; ++i ) {
		if ( arrWindows[i].iPane < 0 ) continue;
		if ( arrWindows[i].bAutoHide ) (void)xuiDockPanelAutoHideWindow(pNode->pWidget, arrWindows[i].iWindow);
	}
	for ( i = 0; i < iWindowCount; ++i ) {
		if ( arrWindows[i].bFloating ) (void)xuiDockPanelFloatWindow(pNode->pWidget, arrWindows[i].iWindow, arrWindows[i].tFloatRect);
		if ( arrWindows[i].bHidden ) (void)xuiDockPanelHideWindow(pNode->pWidget, arrWindows[i].iWindow);
	}
	if ( bHasModelChildren ) {
		iRet = __uiDesignAttachContainerModelChildren(pApp, pNode);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __uiDesignApplyDockPanelRegions(ui_design_node_t* pNode)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[4];
	int iFieldCount;
	int iRegion;
	int iMode;
	float fValue;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	sCursor = __uiDesignText(pNode, "data.regions", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 3 || arrFields[0][0] == 0 ) continue;
		iRegion = __uiDesignDockRegionFromText(arrFields[0], -1);
		if ( iRegion < 0 || iRegion >= XUI_DOCK_PANEL_REGION_COUNT ) continue;
		iMode = __uiDesignDockSizeModeFromText(arrFields[1], XUI_DOCK_PANEL_SIZE_PORTION);
		if ( iMode != XUI_DOCK_PANEL_SIZE_PIXEL ) iMode = XUI_DOCK_PANEL_SIZE_PORTION;
		fValue = __uiDesignParseFloatText(arrFields[2], 0.0f);
		if ( fValue < 0.0f ) fValue = 0.0f;
		(void)xuiDockPanelSetRegionSize(pNode->pWidget, iRegion, iMode, fValue);
	}
	return XUI_OK;
}

static int __uiDesignApplyPopupContent(ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	xui_widget pContent;
	const char* sCursor;
	char sLine[512];
	char* arrFields[8];
	xui_rect_t tRect;
	uint32_t iColor;
	uint32_t iFlags;
	int iFieldCount;
	int iCount;
	float fY;

	if ( (pApp == NULL) || (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContent = xuiPopupGetContentWidget(pNode->pWidget);
	if ( pContent == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	__uiDesignClearChildren(pContent);
	sCursor = __uiDesignText(pNode, "data.content", "");
	iCount = 0;
	fY = 10.0f;
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount <= 0) || (arrFields[0][0] == 0) ) continue;
		tRect.fX = (iFieldCount > 1) ? __uiDesignParseFloatText(arrFields[1], 12.0f) : 12.0f;
		tRect.fY = (iFieldCount > 2) ? __uiDesignParseFloatText(arrFields[2], fY) : fY;
		tRect.fW = (iFieldCount > 3) ? __uiDesignParseFloatText(arrFields[3], 220.0f) : 220.0f;
		tRect.fH = (iFieldCount > 4) ? __uiDesignParseFloatText(arrFields[4], 26.0f) : 26.0f;
		iColor = (iFieldCount > 5) ? __uiDesignParseColorText(arrFields[5], XUI_COLOR_RGBA(53, 73, 96, 255)) : XUI_COLOR_RGBA(53, 73, 96, 255);
		iFlags = ((iFieldCount > 6) ? __uiDesignTableAlignFromText(arrFields[6], XUI_TEXT_ALIGN_LEFT) : XUI_TEXT_ALIGN_LEFT) | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
		(void)__uiDesignAddPreviewLabelEx(pApp, pContent, arrFields[0], tRect, iColor, iFlags);
		fY = tRect.fY + tRect.fH + 8.0f;
		++iCount;
	}
	return (iCount > 0) ? XUI_OK : __uiDesignAddPreviewLabelEx(pApp, pContent, "Popup", (xui_rect_t){12.0f, 10.0f, 220.0f, 24.0f},
		XUI_COLOR_RGBA(53, 73, 96, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
}

static xui_widget __uiDesignPreviewLabelAt(xui_widget pPanel, int iIndex)
{
	xui_widget pClient;
	xui_widget pChild;
	int i;

	pClient = xuiPanelGetClientWidget(pPanel);
	pChild = (pClient != NULL) ? xuiWidgetGetFirstChild(pClient) : NULL;
	for ( i = 0; (pChild != NULL) && (i < iIndex); i++ ) {
		pChild = xuiWidgetGetNextSibling(pChild);
	}
	return pChild;
}

static const char* __uiDesignOverlayPreviewTitle(const ui_design_node_t* pNode)
{
	if ( pNode == NULL ) return "Overlay";
	switch ( pNode->iType ) {
	case UI_DESIGN_NODE_MSG_BOX:
		return __uiDesignText(pNode, "text.title", "Message");
	case UI_DESIGN_NODE_FILE_DIALOG:
		return __uiDesignText(pNode, "text.title", "Open File");
	case UI_DESIGN_NODE_MSG_TIP:
		return "MsgTip";
	case UI_DESIGN_NODE_TOAST:
		return __uiDesignText(pNode, "text.title", "Build Finished");
	default:
		return "Overlay";
	}
}

static uint32_t __uiDesignOverlayPreviewAccent(const ui_design_node_t* pNode)
{
	int iType;

	if ( pNode == NULL ) return XUI_COLOR_RGBA(49, 126, 214, 255);
	switch ( pNode->iType ) {
	case UI_DESIGN_NODE_MSG_BOX:
		iType = __uiDesignInt(pNode, "behavior.type", XUI_MSGBOX_ICON_INFO);
		if ( iType == XUI_MSGBOX_ICON_ERROR ) return __uiDesignColor(pNode, "appearance.iconColor", XUI_COLOR_RGBA(210, 70, 84, 255));
		if ( iType == XUI_MSGBOX_ICON_WAR ) return __uiDesignColor(pNode, "appearance.iconColor", XUI_COLOR_RGBA(219, 141, 36, 255));
		if ( iType == XUI_MSGBOX_ICON_QUEST ) return __uiDesignColor(pNode, "appearance.iconColor", XUI_COLOR_RGBA(59, 126, 204, 255));
		return __uiDesignColor(pNode, "appearance.iconColor", XUI_COLOR_RGBA(35, 132, 214, 255));
	case UI_DESIGN_NODE_MSG_TIP:
		return __uiDesignColor(pNode, "appearance.iconColor", XUI_COLOR_RGBA(78, 159, 220, 255));
	case UI_DESIGN_NODE_TOAST:
		iType = __uiDesignInt(pNode, "behavior.type", XUI_TOAST_TYPE_SUCCESS);
		if ( iType == XUI_TOAST_TYPE_ERROR ) return __uiDesignColor(pNode, "appearance.errorColor", XUI_COLOR_RGBA(216, 76, 90, 255));
		if ( iType == XUI_TOAST_TYPE_WARNING ) return __uiDesignColor(pNode, "appearance.warningColor", XUI_COLOR_RGBA(231, 156, 45, 255));
		if ( iType == XUI_TOAST_TYPE_SUCCESS ) return __uiDesignColor(pNode, "appearance.successColor", XUI_COLOR_RGBA(43, 184, 150, 255));
		return __uiDesignColor(pNode, "appearance.infoColor", XUI_COLOR_RGBA(78, 159, 220, 255));
	default:
		return XUI_COLOR_RGBA(49, 126, 214, 255);
	}
}

static int __uiDesignToastTypeFromText(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "info") ) return XUI_TOAST_TYPE_INFO;
	if ( __uiDesignTokenIs(sText, "success") ) return XUI_TOAST_TYPE_SUCCESS;
	if ( __uiDesignTokenIs(sText, "warning") || __uiDesignTokenIs(sText, "warn") ) return XUI_TOAST_TYPE_WARNING;
	if ( __uiDesignTokenIs(sText, "error") || __uiDesignTokenIs(sText, "err") ) return XUI_TOAST_TYPE_ERROR;
	return __uiDesignParseIntText(sText, iDefault);
}

static uint32_t __uiDesignToastAccentColor(const ui_design_node_t* pNode, int iType)
{
	if ( pNode == NULL ) return XUI_COLOR_RGBA(49, 126, 214, 255);
	if ( iType == XUI_TOAST_TYPE_ERROR ) return __uiDesignColor(pNode, "appearance.errorColor", XUI_COLOR_RGBA(216, 76, 90, 255));
	if ( iType == XUI_TOAST_TYPE_WARNING ) return __uiDesignColor(pNode, "appearance.warningColor", XUI_COLOR_RGBA(231, 156, 45, 255));
	if ( iType == XUI_TOAST_TYPE_SUCCESS ) return __uiDesignColor(pNode, "appearance.successColor", XUI_COLOR_RGBA(43, 184, 150, 255));
	return __uiDesignColor(pNode, "appearance.infoColor", XUI_COLOR_RGBA(78, 159, 220, 255));
}

static const char* __uiDesignToastIconGlyph(int iType)
{
	if ( iType == XUI_TOAST_TYPE_ERROR ) return "x";
	if ( iType == XUI_TOAST_TYPE_WARNING ) return "!";
	if ( iType == XUI_TOAST_TYPE_SUCCESS ) return "ok";
	return "i";
}

static int __uiDesignLoadOverlayIcon(ui_design_app_t* pApp, ui_design_node_t* pNode, xui_surface* ppSurface, xui_rect_t* pSrc)
{
	float fX;
	float fY;
	float fW;
	float fH;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppSurface == NULL) || (pSrc == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pSrc, 0, sizeof(*pSrc));
	iRet = __uiDesignLoadRuntimeSurfaceSlot(pApp, pNode, 0, __uiDesignText(pNode, "data.iconSource", ""), ppSurface);
	if ( iRet != XUI_OK ) return iRet;
	if ( *ppSurface == NULL ) return XUI_OK;
	fX = __uiDesignFloat(pNode, "icon.x", 0.0f);
	fY = __uiDesignFloat(pNode, "icon.y", 0.0f);
	fW = __uiDesignFloat(pNode, "icon.w", 0.0f);
	fH = __uiDesignFloat(pNode, "icon.h", 0.0f);
	if ( fW > 0.0f && fH > 0.0f ) {
		pSrc->fX = fX;
		pSrc->fY = fY;
		pSrc->fW = fW;
		pSrc->fH = fH;
	}
	return XUI_OK;
}

static void __uiDesignOverlayPreviewText(const ui_design_node_t* pNode, char* sLine1, int iLine1, char* sLine2, int iLine2, char* sLine3, int iLine3)
{
	if ( (sLine1 == NULL) || (sLine2 == NULL) || (sLine3 == NULL) ) return;
	sLine1[0] = '\0';
	sLine2[0] = '\0';
	sLine3[0] = '\0';
	if ( pNode == NULL ) return;
	switch ( pNode->iType ) {
	case UI_DESIGN_NODE_MSG_BOX:
		snprintf(sLine1, (size_t)iLine1, "%s", __uiDesignText(pNode, "text.message", "Operation completed."));
		snprintf(sLine2, (size_t)iLine2, "buttons=%d modal=%s",
			__uiDesignInt(pNode, "behavior.buttons", XUI_MSGBOX_BUTTON_OK),
			__uiDesignBool(pNode, "behavior.modal", 1) ? "true" : "false");
		snprintf(sLine3, (size_t)iLine3, "min %.0f max %.0f",
			__uiDesignFloat(pNode, "metrics.minWidth", 320.0f),
			__uiDesignFloat(pNode, "metrics.maxWidth", 680.0f));
		break;
	case UI_DESIGN_NODE_FILE_DIALOG:
		snprintf(sLine1, (size_t)iLine1, "%s", __uiDesignText(pNode, "text.initialDir", "."));
		snprintf(sLine2, (size_t)iLine2, "%s", __uiDesignText(pNode, "data.filter", "All Files (*.*)|*.*"));
		snprintf(sLine3, (size_t)iLine3, "%.0f x %.0f mode=%d",
			__uiDesignFloat(pNode, "metrics.dialogWidth", 760.0f),
			__uiDesignFloat(pNode, "metrics.dialogHeight", 520.0f),
			__uiDesignInt(pNode, "behavior.mode", XUI_FILE_DIALOG_MODE_OPEN_FILE));
		break;
	case UI_DESIGN_NODE_MSG_TIP:
		snprintf(sLine1, (size_t)iLine1, "%s", __uiDesignText(pNode, "text.text", "Saved successfully."));
		snprintf(sLine2, (size_t)iLine2, "duration %.1fs offset %.0f",
			__uiDesignFloat(pNode, "behavior.duration", 2.4f),
			__uiDesignFloat(pNode, "metrics.offsetY", -72.0f));
		snprintf(sLine3, (size_t)iLine3, "range %.0f - %.0f",
			__uiDesignFloat(pNode, "metrics.minWidth", 92.0f),
			__uiDesignFloat(pNode, "metrics.maxWidth", 360.0f));
		break;
	case UI_DESIGN_NODE_TOAST:
		snprintf(sLine1, (size_t)iLine1, "%s", __uiDesignText(pNode, "text.message", "No errors were reported."));
		snprintf(sLine2, (size_t)iLine2, "placement=%d direction=%d",
			__uiDesignInt(pNode, "behavior.placement", XUI_TOAST_PLACEMENT_TOP_RIGHT),
			__uiDesignInt(pNode, "behavior.direction", XUI_TOAST_DIRECTION_AUTO));
		snprintf(sLine3, (size_t)iLine3, "%.0f wide max=%d",
			__uiDesignFloat(pNode, "metrics.width", 320.0f),
			__uiDesignInt(pNode, "metrics.maxVisible", 0));
		break;
	default:
		break;
	}
}

static const char* __uiDesignMsgBoxIconGlyph(int iType)
{
	switch ( iType ) {
	case XUI_MSGBOX_ICON_QUEST:
		return "?";
	case XUI_MSGBOX_ICON_WAR:
		return "!";
	case XUI_MSGBOX_ICON_ERROR:
		return "x";
	case XUI_MSGBOX_ICON_INFO:
		return "i";
	default:
		return "";
	}
}

static int __uiDesignButtonSemanticFromText(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "default") || __uiDesignTokenIs(sText, "normal") ) return XUI_BUTTON_SEMANTIC_DEFAULT;
	if ( __uiDesignTokenIs(sText, "primary") || __uiDesignTokenIs(sText, "ok") || __uiDesignTokenIs(sText, "accept") ) return XUI_BUTTON_SEMANTIC_PRIMARY;
	if ( __uiDesignTokenIs(sText, "danger") || __uiDesignTokenIs(sText, "destructive") ) return XUI_BUTTON_SEMANTIC_DANGER;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static void __uiDesignSetMsgBoxPreviewButton(ui_design_msgbox_button_preview_t* pButton, const char* sText, int iResult, int iSemantic)
{
	if ( pButton == NULL ) return;
	pButton->sText = (sText != NULL) ? sText : "";
	pButton->iResult = iResult;
	pButton->iSemantic = iSemantic;
}

static int __uiDesignMsgBoxButtonTitleFromText(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "ok") || __uiDesignTokenIs(sText, "accept") ) return XUI_MSGBOX_BUTTON_TITLE_OK;
	if ( __uiDesignTokenIs(sText, "cancel") || __uiDesignTokenIs(sText, "abort") ) return XUI_MSGBOX_BUTTON_TITLE_CANCEL;
	if ( __uiDesignTokenIs(sText, "yes") || __uiDesignTokenIs(sText, "confirm") ) return XUI_MSGBOX_BUTTON_TITLE_YES;
	if ( __uiDesignTokenIs(sText, "no") || __uiDesignTokenIs(sText, "decline") ) return XUI_MSGBOX_BUTTON_TITLE_NO;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static const char* __uiDesignDefaultMsgBoxButtonTitle(int iTitle)
{
	switch ( iTitle ) {
	case XUI_MSGBOX_BUTTON_TITLE_CANCEL:
		return "Cancel";
	case XUI_MSGBOX_BUTTON_TITLE_YES:
		return "Yes";
	case XUI_MSGBOX_BUTTON_TITLE_NO:
		return "No";
	case XUI_MSGBOX_BUTTON_TITLE_OK:
	default:
		return "OK";
	}
}

static const char* __uiDesignMsgBoxPreviewButtonTitle(ui_design_node_t* pNode, int iTitle)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[3];
	int iFieldCount;

	if ( pNode == NULL ) return __uiDesignDefaultMsgBoxButtonTitle(iTitle);
	sCursor = __uiDesignText(pNode, "data.buttonTitles", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount < 2) || (arrFields[0][0] == 0) || (arrFields[1][0] == 0) ) continue;
		if ( __uiDesignMsgBoxButtonTitleFromText(arrFields[0], 0) == iTitle ) {
			return __uiDesignTextPoolAdd(pNode, arrFields[1]);
		}
	}
	return __uiDesignDefaultMsgBoxButtonTitle(iTitle);
}

static int __uiDesignBuildMsgBoxButtons(ui_design_node_t* pNode, ui_design_msgbox_button_preview_t* pButtons, int iCapacity)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[4];
	int iButtons;
	int iCount;
	int iFieldCount;

	if ( (pNode == NULL) || (pButtons == NULL) || (iCapacity <= 0) ) return 0;
	iButtons = __uiDesignInt(pNode, "behavior.buttons", XUI_MSGBOX_BUTTON_OK);
	iCount = 0;
	if ( iButtons == XUI_MSGBOX_BUTTON_CUSTOM ) {
		sCursor = __uiDesignText(pNode, "data.customButtons", "");
		while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
			if ( sLine[0] == 0 ) continue;
			iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
			if ( (iFieldCount <= 0) || (arrFields[0][0] == 0) ) continue;
			__uiDesignSetMsgBoxPreviewButton(&pButtons[iCount],
				__uiDesignTextPoolAdd(pNode, arrFields[0]),
				__uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 1, ""), iCount),
				__uiDesignButtonSemanticFromText(__uiDesignField(arrFields, iFieldCount, 2, ""), XUI_BUTTON_SEMANTIC_DEFAULT));
			++iCount;
		}
	}
	if ( iCount > 0 ) return iCount;
	switch ( iButtons ) {
	case XUI_MSGBOX_BUTTON_OK_CANCEL:
		__uiDesignSetMsgBoxPreviewButton(&pButtons[0], __uiDesignMsgBoxPreviewButtonTitle(pNode, XUI_MSGBOX_BUTTON_TITLE_OK), XUI_MSGBOX_RESULT_OK, XUI_BUTTON_SEMANTIC_PRIMARY);
		if ( iCapacity > 1 ) __uiDesignSetMsgBoxPreviewButton(&pButtons[1], __uiDesignMsgBoxPreviewButtonTitle(pNode, XUI_MSGBOX_BUTTON_TITLE_CANCEL), XUI_MSGBOX_RESULT_CANCEL, XUI_BUTTON_SEMANTIC_DEFAULT);
		return (iCapacity >= 2) ? 2 : 1;
	case XUI_MSGBOX_BUTTON_YES_NO:
		__uiDesignSetMsgBoxPreviewButton(&pButtons[0], __uiDesignMsgBoxPreviewButtonTitle(pNode, XUI_MSGBOX_BUTTON_TITLE_YES), XUI_MSGBOX_RESULT_YES, XUI_BUTTON_SEMANTIC_PRIMARY);
		if ( iCapacity > 1 ) __uiDesignSetMsgBoxPreviewButton(&pButtons[1], __uiDesignMsgBoxPreviewButtonTitle(pNode, XUI_MSGBOX_BUTTON_TITLE_NO), XUI_MSGBOX_RESULT_NO, XUI_BUTTON_SEMANTIC_DEFAULT);
		return (iCapacity >= 2) ? 2 : 1;
	case XUI_MSGBOX_BUTTON_YES_NO_CANCEL:
		__uiDesignSetMsgBoxPreviewButton(&pButtons[0], __uiDesignMsgBoxPreviewButtonTitle(pNode, XUI_MSGBOX_BUTTON_TITLE_YES), XUI_MSGBOX_RESULT_YES, XUI_BUTTON_SEMANTIC_PRIMARY);
		if ( iCapacity > 1 ) __uiDesignSetMsgBoxPreviewButton(&pButtons[1], __uiDesignMsgBoxPreviewButtonTitle(pNode, XUI_MSGBOX_BUTTON_TITLE_NO), XUI_MSGBOX_RESULT_NO, XUI_BUTTON_SEMANTIC_DEFAULT);
		if ( iCapacity > 2 ) __uiDesignSetMsgBoxPreviewButton(&pButtons[2], __uiDesignMsgBoxPreviewButtonTitle(pNode, XUI_MSGBOX_BUTTON_TITLE_CANCEL), XUI_MSGBOX_RESULT_CANCEL, XUI_BUTTON_SEMANTIC_DEFAULT);
		return (iCapacity >= 3) ? 3 : iCapacity;
	case XUI_MSGBOX_BUTTON_CUSTOM:
		__uiDesignSetMsgBoxPreviewButton(&pButtons[0], "Custom", XUI_MSGBOX_RESULT_OK, XUI_BUTTON_SEMANTIC_DEFAULT);
		return 1;
	case XUI_MSGBOX_BUTTON_OK:
	default:
		__uiDesignSetMsgBoxPreviewButton(&pButtons[0], __uiDesignMsgBoxPreviewButtonTitle(pNode, XUI_MSGBOX_BUTTON_TITLE_OK), XUI_MSGBOX_RESULT_OK, XUI_BUTTON_SEMANTIC_PRIMARY);
		return 1;
	}
}

static int __uiDesignApplyMsgBoxPreview(ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pClient,
	float fW, float fClientH, uint32_t iText, uint32_t iMuted, uint32_t iAccent, uint32_t iBorder)
{
	ui_design_msgbox_button_preview_t arrButtons[XUI_MSGBOX_BUTTON_CAPACITY];
	xui_surface pIconSurface;
	xui_rect_t tIconSrc;
	xui_rect_t tRect;
	float fPadX;
	float fPadY;
	float fIconSize;
	float fIconGap;
	float fButtonW;
	float fButtonH;
	float fButtonGap;
	float fFooterH;
	float fMinMessageH;
	float fMinW;
	float fMaxW;
	float fMaxRatio;
	float fLayoutX;
	float fLayoutW;
	float fMessageX;
	float fMessageW;
	float fMessageH;
	float fButtonTotalW;
	float fButtonX;
	float fButtonY;
	uint32_t iButtonColor;
	uint32_t iButtonHoverColor;
	uint32_t iButtonActiveColor;
	uint32_t iButtonFocusColor;
	uint32_t iButtonDisabledColor;
	int iType;
	int iButtonCount;
	int iRet;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) || (pClient == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	fPadX = __uiDesignFloat(pNode, "metrics.paddingX", 18.0f);
	fPadY = __uiDesignFloat(pNode, "metrics.paddingY", 16.0f);
	fIconSize = __uiDesignFloat(pNode, "metrics.iconSize", 38.0f);
	fIconGap = __uiDesignFloat(pNode, "metrics.iconGap", 14.0f);
	fButtonW = __uiDesignFloat(pNode, "metrics.buttonWidth", 78.0f);
	fButtonH = __uiDesignFloat(pNode, "metrics.buttonHeight", 28.0f);
	fButtonGap = __uiDesignFloat(pNode, "metrics.buttonGap", 8.0f);
	fFooterH = __uiDesignFloat(pNode, "metrics.footerHeight", 50.0f);
	fMinMessageH = __uiDesignFloat(pNode, "metrics.minMessageHeight", 38.0f);
	fMinW = __uiDesignFloat(pNode, "metrics.minWidth", 320.0f);
	fMaxW = __uiDesignFloat(pNode, "metrics.maxWidth", 680.0f);
	fMaxRatio = __uiDesignFloat(pNode, "metrics.maxWidthRatio", 0.8f);
	if ( fMinMessageH < 0.0f ) fMinMessageH = 0.0f;
	if ( fMaxRatio < 0.1f ) fMaxRatio = 0.1f;
	if ( fMaxRatio > 1.0f ) fMaxRatio = 1.0f;
	fLayoutW = fW * fMaxRatio;
	if ( fMaxW > 0.0f && fLayoutW > fMaxW ) fLayoutW = fMaxW;
	if ( fLayoutW < fMinW ) fLayoutW = fMinW;
	if ( fLayoutW > fW ) fLayoutW = fW;
	if ( fLayoutW < 40.0f ) fLayoutW = 40.0f;
	fLayoutX = (fW - fLayoutW) * 0.5f;
	iType = __uiDesignInt(pNode, "behavior.type", XUI_MSGBOX_ICON_INFO);
	iButtonColor = __uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(247, 251, 255, 255));
	iButtonHoverColor = __uiDesignColor(pNode, "appearance.buttonHoverColor", XUI_COLOR_RGBA(232, 243, 253, 255));
	iButtonActiveColor = __uiDesignColor(pNode, "appearance.buttonActiveColor", XUI_COLOR_RGBA(212, 231, 248, 255));
	iButtonFocusColor = __uiDesignColor(pNode, "appearance.buttonFocusColor", XUI_COLOR_RGBA(47, 128, 214, 255));
	iButtonDisabledColor = __uiDesignColor(pNode, "appearance.buttonDisabledColor", XUI_COLOR_RGBA(226, 235, 243, 255));
	pIconSurface = NULL;
	memset(&tIconSrc, 0, sizeof(tIconSrc));
	(void)__uiDesignLoadOverlayIcon(pApp, pNode, &pIconSurface, &tIconSrc);
	fMessageX = fLayoutX + fPadX;
	if ( pIconSurface != NULL ) {
		tRect = (xui_rect_t){fLayoutX + fPadX, fPadY, fIconSize, fIconSize};
		iRet = __uiDesignAddPreviewImage(pApp, pClient, pIconSurface, tIconSrc, tRect, XUI_COLOR_WHITE);
		if ( iRet != XUI_OK ) return iRet;
		fMessageX += fIconSize + fIconGap;
	} else if ( iType != XUI_MSGBOX_ICON_NONE ) {
		tRect = (xui_rect_t){fLayoutX + fPadX, fPadY, fIconSize, fIconSize};
		iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, __uiDesignMsgBoxIconGlyph(iType), tRect, iAccent,
			XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
		fMessageX += fIconSize + fIconGap;
	}
	fMessageW = fLayoutX + fLayoutW - fMessageX - fPadX;
	if ( fMessageW < 32.0f ) fMessageW = 32.0f;
	fMessageH = fClientH - fPadY * 2.0f - fFooterH;
	if ( fMessageH < fMinMessageH ) fMessageH = fMinMessageH;
	if ( fMessageH < 24.0f ) fMessageH = 24.0f;
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, __uiDesignText(pNode, "text.message", "Operation completed."),
		(xui_rect_t){fMessageX, fPadY, fMessageW, fMessageH}, iText,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_WRAP_WORD | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient,
		__uiDesignBool(pNode, "behavior.modal", 1) ? "Modal backdrop enabled" : "Modeless dialog",
		(xui_rect_t){fLayoutX + fPadX, fClientH - fFooterH - 18.0f, fLayoutW - fPadX * 2.0f, 18.0f}, iMuted,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	iButtonCount = __uiDesignBuildMsgBoxButtons(pNode, arrButtons, UI_DESIGN_COUNT_OF(arrButtons));
	if ( iButtonCount <= 0 ) return XUI_OK;
	fButtonTotalW = (float)iButtonCount * fButtonW + (float)(iButtonCount - 1) * fButtonGap;
	fButtonX = fLayoutX + fLayoutW - fPadX - fButtonTotalW;
	if ( fButtonX < fLayoutX + fPadX ) fButtonX = fLayoutX + fPadX;
	fButtonY = fClientH - fFooterH + (fFooterH - fButtonH) * 0.5f;
	for ( i = 0; i < iButtonCount; ++i ) {
		iRet = __uiDesignAddPreviewButtonEx(pApp, pClient, arrButtons[i].sText,
			(xui_rect_t){fButtonX + (float)i * (fButtonW + fButtonGap), fButtonY, fButtonW, fButtonH},
			iText,
			XUI_COLOR_RGBA(140, 154, 175, 255),
			iButtonColor,
			iButtonHoverColor,
			iButtonActiveColor,
			iButtonFocusColor,
			iButtonDisabledColor,
			iBorder,
			1.0f,
			arrButtons[i].iSemantic);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __uiDesignFileDialogTextTargetMatches(const char* sText, const char* sTarget)
{
	if ( (sText == NULL) || (sTarget == NULL) ) return 0;
	if ( __uiDesignTokenIs(sText, sTarget) ) return 1;
	if ( __uiDesignTokenIs(sTarget, "ok") ) {
		return __uiDesignTokenIs(sText, "open") || __uiDesignTokenIs(sText, "save") ||
			__uiDesignTokenIs(sText, "select") || __uiDesignTokenIs(sText, "accept") ||
			__uiDesignTokenIs(sText, "confirm");
	}
	if ( __uiDesignTokenIs(sTarget, "cancel") ) return __uiDesignTokenIs(sText, "dismiss") || __uiDesignTokenIs(sText, "abort");
	if ( __uiDesignTokenIs(sTarget, "up") ) return __uiDesignTokenIs(sText, "parent") || __uiDesignTokenIs(sText, "upButton");
	if ( __uiDesignTokenIs(sTarget, "refresh") ) return __uiDesignTokenIs(sText, "reload") || __uiDesignTokenIs(sText, "refreshButton");
	if ( __uiDesignTokenIs(sTarget, "path") ) return __uiDesignTokenIs(sText, "location") || __uiDesignTokenIs(sText, "address");
	if ( __uiDesignTokenIs(sTarget, "file") ) return __uiDesignTokenIs(sText, "name") || __uiDesignTokenIs(sText, "filename");
	if ( __uiDesignTokenIs(sTarget, "folder") ) return __uiDesignTokenIs(sText, "directory") || __uiDesignTokenIs(sText, "dir");
	if ( __uiDesignTokenIs(sTarget, "type") ) return __uiDesignTokenIs(sText, "filter") || __uiDesignTokenIs(sText, "filetype");
	return 0;
}

static const char* __uiDesignFileDialogPreviewText(ui_design_node_t* pNode, const char* sTarget, const char* sDefault)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[3];
	int iFieldCount;

	if ( pNode == NULL ) return sDefault;
	sCursor = __uiDesignText(pNode, "data.buttonTitles", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount < 2) || (arrFields[0][0] == 0) || (arrFields[1][0] == 0) ) continue;
		if ( __uiDesignFileDialogTextTargetMatches(arrFields[0], sTarget) ) return __uiDesignTextPoolAdd(pNode, arrFields[1]);
	}
	return sDefault;
}

static const char* __uiDesignFileDialogDefaultOkText(int iMode)
{
	switch ( iMode ) {
	case XUI_FILE_DIALOG_MODE_SAVE_FILE:
		return "Save";
	case XUI_FILE_DIALOG_MODE_SELECT_FOLDER:
		return "Select";
	case XUI_FILE_DIALOG_MODE_OPEN_FILE:
	default:
		return "Open";
	}
}

static int __uiDesignFileDialogKindIsFolder(const char* sKind)
{
	return __uiDesignTokenIs(sKind, "folder") || __uiDesignTokenIs(sKind, "dir") ||
		__uiDesignTokenIs(sKind, "directory");
}

static const char* __uiDesignFileDialogKindText(const char* sKind)
{
	if ( (sKind == NULL) || (sKind[0] == 0) || __uiDesignTokenIs(sKind, "file") ) return "File";
	if ( __uiDesignFileDialogKindIsFolder(sKind) ) return "Folder";
	if ( __uiDesignTokenIs(sKind, "image") ) return "Image";
	if ( __uiDesignTokenIs(sKind, "script") ) return "Script";
	if ( __uiDesignTokenIs(sKind, "document") || __uiDesignTokenIs(sKind, "doc") ) return "Document";
	return sKind;
}

static const char* __uiDesignFileDialogKindGlyph(const char* sKind)
{
	if ( __uiDesignFileDialogKindIsFolder(sKind) ) return "DIR";
	if ( __uiDesignTokenIs(sKind, "image") ) return "IMG";
	if ( __uiDesignTokenIs(sKind, "script") ) return "SRC";
	if ( __uiDesignTokenIs(sKind, "document") || __uiDesignTokenIs(sKind, "doc") ) return "DOC";
	return "FILE";
}

static int __uiDesignBuildFileDialogRoots(ui_design_node_t* pNode,
	ui_design_file_dialog_root_preview_t* pRoots, int iCapacity, int* pSelectedIndex)
{
	const char* sCursor;
	const char* sInitialDir;
	char sLine[256];
	char* arrFields[5];
	int iFieldCount;
	int iCount;

	if ( pSelectedIndex != NULL ) *pSelectedIndex = -1;
	if ( (pNode == NULL) || (pRoots == NULL) || (iCapacity <= 0) ) return 0;
	sCursor = __uiDesignText(pNode, "data.roots", "");
	iCount = 0;
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount <= 0) || (arrFields[0][0] == 0) ) continue;
		pRoots[iCount].sLabel = __uiDesignTextPoolAdd(pNode, arrFields[0]);
		pRoots[iCount].sPath = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 1, ""));
		pRoots[iCount].bSelected = __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 2, ""), iCount == 0);
		pRoots[iCount].bEnabled = __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 3, ""), 1);
		if ( pRoots[iCount].bSelected && pRoots[iCount].bEnabled && pSelectedIndex != NULL ) *pSelectedIndex = iCount;
		++iCount;
	}
	if ( iCount == 0 ) {
		sInitialDir = __uiDesignText(pNode, "text.initialDir", ".");
		pRoots[0].sLabel = __uiDesignTextPoolAdd(pNode, "Project");
		pRoots[0].sPath = __uiDesignTextPoolAdd(pNode, sInitialDir);
		pRoots[0].bSelected = 1;
		pRoots[0].bEnabled = 1;
		if ( iCapacity > 1 ) {
			pRoots[1].sLabel = __uiDesignTextPoolAdd(pNode, "Desktop");
			pRoots[1].sPath = __uiDesignTextPoolAdd(pNode, "%USERPROFILE%\\Desktop");
			pRoots[1].bSelected = 0;
			pRoots[1].bEnabled = 1;
			iCount = 2;
		} else {
			iCount = 1;
		}
		if ( pSelectedIndex != NULL ) *pSelectedIndex = 0;
	} else if ( pSelectedIndex != NULL && *pSelectedIndex < 0 ) {
		*pSelectedIndex = 0;
		pRoots[0].bSelected = 1;
	}
	return iCount;
}

static int __uiDesignBuildFileDialogEntries(ui_design_node_t* pNode,
	ui_design_file_dialog_entry_preview_t* pEntries, int iCapacity, int* pSelectedIndex)
{
	const char* sCursor;
	char sLine[320];
	char* arrFields[7];
	int iFieldCount;
	int iCount;

	if ( pSelectedIndex != NULL ) *pSelectedIndex = -1;
	if ( (pNode == NULL) || (pEntries == NULL) || (iCapacity <= 0) ) return 0;
	sCursor = __uiDesignText(pNode, "data.entries", "");
	iCount = 0;
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iCapacity ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount <= 0) || (arrFields[0][0] == 0) ) continue;
		pEntries[iCount].sName = __uiDesignTextPoolAdd(pNode, arrFields[0]);
		pEntries[iCount].sKind = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 1, "file"));
		pEntries[iCount].sSize = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 2,
			__uiDesignFileDialogKindIsFolder(pEntries[iCount].sKind) ? "Folder" : ""));
		pEntries[iCount].sModified = __uiDesignTextPoolAdd(pNode, __uiDesignField(arrFields, iFieldCount, 3, ""));
		pEntries[iCount].bSelected = __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 4, ""), iCount == 0);
		pEntries[iCount].bEnabled = __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 5, ""), 1);
		if ( pEntries[iCount].bSelected && pEntries[iCount].bEnabled && pSelectedIndex != NULL ) *pSelectedIndex = iCount;
		++iCount;
	}
	if ( iCount == 0 ) {
		pEntries[0].sName = __uiDesignTextPoolAdd(pNode, "scene.ui");
		pEntries[0].sKind = __uiDesignTextPoolAdd(pNode, "file");
		pEntries[0].sSize = __uiDesignTextPoolAdd(pNode, "14 KB");
		pEntries[0].sModified = __uiDesignTextPoolAdd(pNode, "Today");
		pEntries[0].bSelected = 1;
		pEntries[0].bEnabled = 1;
		if ( iCapacity > 1 ) {
			pEntries[1].sName = __uiDesignTextPoolAdd(pNode, "assets");
			pEntries[1].sKind = __uiDesignTextPoolAdd(pNode, "folder");
			pEntries[1].sSize = __uiDesignTextPoolAdd(pNode, "Folder");
			pEntries[1].sModified = __uiDesignTextPoolAdd(pNode, "Yesterday");
			pEntries[1].bSelected = 0;
			pEntries[1].bEnabled = 1;
			iCount = 2;
		} else {
			iCount = 1;
		}
		if ( pSelectedIndex != NULL ) *pSelectedIndex = 0;
	} else if ( pSelectedIndex != NULL && *pSelectedIndex < 0 ) {
		*pSelectedIndex = 0;
		pEntries[0].bSelected = 1;
	}
	return iCount;
}

static const char* __uiDesignFileDialogSelectedFilter(ui_design_node_t* pNode)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[3];
	int iFieldCount;
	int iIndex;
	int iSelected;

	if ( pNode == NULL ) return "All Files (*.*)";
	iSelected = __uiDesignInt(pNode, "data.selectedFilter", 0);
	sCursor = __uiDesignText(pNode, "data.filter", "");
	iIndex = 0;
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount <= 0) || (arrFields[0][0] == 0) ) continue;
		if ( iIndex == iSelected ) return __uiDesignTextPoolAdd(pNode, arrFields[0]);
		++iIndex;
	}
	return "All Files (*.*)";
}

static int __uiDesignApplyFileDialogPreview(ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pClient,
	float fW, float fClientH, uint32_t iText, uint32_t iMuted, uint32_t iBorder)
{
	ui_design_file_dialog_root_preview_t arrRoots[8];
	ui_design_file_dialog_entry_preview_t arrEntries[24];
	char sLine[512];
	float fPad;
	float fToolbarH;
	float fFooterH;
	float fRowH;
	float fSidebarW;
	float fGap;
	float fButtonW;
	float fButtonH;
	float fButtonGap;
	float fToolbarY;
	float fListY;
	float fListH;
	float fListX;
	float fListW;
	float fPathX;
	float fPathW;
	float fFieldY;
	float fFieldLabelW;
	float fActionX;
	float fNameW;
	float fFilterW;
	float fButtonY;
	float fColKindW;
	float fColSizeW;
	float fColModifiedW;
	float fNameColW;
	float fRowY;
	uint32_t iSidebarColor;
	uint32_t iListColor;
	uint32_t iInputColor;
	uint32_t iButtonColor;
	uint32_t iButtonHoverColor;
	uint32_t iButtonActiveColor;
	uint32_t iButtonFocusColor;
	uint32_t iButtonDisabledColor;
	uint32_t iSelectedColor;
	uint32_t iHoverColor;
	uint32_t iDisabledText;
	const char* sFileLabel;
	const char* sTypeLabel;
	const char* sPathLabel;
	const char* sOkText;
	const char* sCancelText;
	const char* sFileName;
	const char* sFilterName;
	int iRootCount;
	int iEntryCount;
	int iSelectedRoot;
	int iSelectedEntry;
	int iHoverEntry;
	int iMode;
	int bSelectFolder;
	int bPathEdit;
	int bShowRoots;
	int iRet;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) || (pClient == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iMode = __uiDesignInt(pNode, "behavior.mode", XUI_FILE_DIALOG_MODE_OPEN_FILE);
	bSelectFolder = (iMode == XUI_FILE_DIALOG_MODE_SELECT_FOLDER);
	fPad = __uiDesignFloat(pNode, "metrics.padding", 12.0f);
	fToolbarH = __uiDesignFloat(pNode, "metrics.toolbarHeight", 30.0f);
	fFooterH = __uiDesignFloat(pNode, "metrics.footerHeight", 74.0f);
	fRowH = __uiDesignFloat(pNode, "metrics.rowHeight", 24.0f);
	fSidebarW = __uiDesignFloat(pNode, "metrics.sidebarWidth", 118.0f);
	fButtonW = __uiDesignFloat(pNode, "metrics.buttonWidth", 76.0f);
	fButtonH = __uiDesignFloat(pNode, "metrics.buttonHeight", 26.0f);
	fButtonGap = __uiDesignFloat(pNode, "metrics.buttonGap", 8.0f);
	fGap = 8.0f;
	if ( fPad < 2.0f ) fPad = 2.0f;
	if ( fToolbarH < 22.0f ) fToolbarH = 22.0f;
	if ( fFooterH < 56.0f ) fFooterH = 56.0f;
	if ( fRowH < 18.0f ) fRowH = 18.0f;
	if ( fSidebarW < 72.0f ) fSidebarW = 72.0f;
	if ( fSidebarW > fW * 0.45f ) fSidebarW = fW * 0.45f;
	if ( fButtonW < 52.0f ) fButtonW = 52.0f;
	if ( fButtonH < 22.0f ) fButtonH = 22.0f;
	iSidebarColor = __uiDesignColor(pNode, "appearance.sidebarColor", XUI_COLOR_RGBA(241, 246, 252, 255));
	iListColor = __uiDesignColor(pNode, "appearance.listColor", XUI_COLOR_WHITE);
	iInputColor = __uiDesignColor(pNode, "appearance.inputColor", XUI_COLOR_WHITE);
	iButtonColor = __uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(247, 251, 255, 255));
	iButtonHoverColor = __uiDesignColor(pNode, "appearance.buttonHoverColor", XUI_COLOR_RGBA(232, 243, 253, 255));
	iButtonActiveColor = __uiDesignColor(pNode, "appearance.buttonActiveColor", XUI_COLOR_RGBA(212, 231, 248, 255));
	iButtonFocusColor = __uiDesignColor(pNode, "appearance.buttonFocusColor", XUI_COLOR_RGBA(47, 128, 214, 255));
	iButtonDisabledColor = __uiDesignColor(pNode, "appearance.buttonDisabledColor", XUI_COLOR_RGBA(226, 235, 243, 255));
	iSelectedColor = __uiDesignColor(pNode, "appearance.selectedColor", XUI_COLOR_RGBA(220, 238, 255, 255));
	iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(238, 246, 255, 255));
	iDisabledText = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(154, 168, 184, 255));
	bPathEdit = __uiDesignBool(pNode, "behavior.pathEditMode", 0);
	bShowRoots = __uiDesignBool(pNode, "behavior.showRoots", 1);
	iRootCount = __uiDesignBuildFileDialogRoots(pNode, arrRoots, UI_DESIGN_COUNT_OF(arrRoots), &iSelectedRoot);
	iEntryCount = __uiDesignBuildFileDialogEntries(pNode, arrEntries, UI_DESIGN_COUNT_OF(arrEntries), &iSelectedEntry);
	iHoverEntry = __uiDesignInt(pNode, "data.hoverEntry", 1);
	sFileLabel = __uiDesignFileDialogPreviewText(pNode, bSelectFolder ? "folder" : "file", bSelectFolder ? "Folder" : "File");
	sTypeLabel = __uiDesignFileDialogPreviewText(pNode, "type", "Type");
	sPathLabel = __uiDesignFileDialogPreviewText(pNode, "path", "Path");
	sOkText = __uiDesignFileDialogPreviewText(pNode, "ok", __uiDesignFileDialogDefaultOkText(iMode));
	sCancelText = __uiDesignFileDialogPreviewText(pNode, "cancel", "Cancel");
	sFileName = __uiDesignText(pNode, "text.fileName", "");
	if ( (sFileName == NULL || sFileName[0] == 0) && iSelectedEntry >= 0 && iSelectedEntry < iEntryCount ) {
		sFileName = arrEntries[iSelectedEntry].sName;
	}
	if ( sFileName == NULL ) sFileName = "";
	sFilterName = __uiDesignFileDialogSelectedFilter(pNode);
	fToolbarY = fPad;
	fListY = fToolbarY + fToolbarH + fGap;
	fListH = fClientH - fListY - fFooterH - fPad;
	if ( fListH < fRowH * 3.0f ) fListH = fRowH * 3.0f;
	fListX = fPad;
	if ( bShowRoots ) fListX += fSidebarW + fGap;
	fListW = fW - fListX - fPad;
	if ( fListW < 120.0f ) fListW = 120.0f;
	if ( bShowRoots ) {
		iRet = __uiDesignAddPreviewRect(pApp, pClient, (xui_rect_t){fPad, fListY, fSidebarW, fListH}, iSidebarColor, iBorder, 1.0f);
		if ( iRet != XUI_OK ) return iRet;
		for ( i = 0; i < iRootCount; ++i ) {
			fRowY = 4.0f + (float)i * fRowH;
			if ( arrRoots[i].bSelected ) {
				iRet = __uiDesignAddPreviewRect(pApp, pClient,
					(xui_rect_t){fPad + 4.0f, fListY + fRowY, fSidebarW - 8.0f, fRowH - 2.0f}, iSelectedColor, XUI_COLOR_RGBA(0, 0, 0, 0), 0.0f);
				if ( iRet != XUI_OK ) return iRet;
			}
			iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, arrRoots[i].sLabel,
				(xui_rect_t){fPad + 10.0f, fListY + fRowY, fSidebarW - 18.0f, fRowH - 2.0f},
				arrRoots[i].bEnabled ? iText : iDisabledText,
				XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	iRet = __uiDesignAddPreviewButtonEx(pApp, pClient, __uiDesignFileDialogPreviewText(pNode, "up", "Up"),
		(xui_rect_t){fPad, fToolbarY + 2.0f, 52.0f, fButtonH},
		iText, iDisabledText, iButtonColor, iButtonHoverColor, iButtonActiveColor, iButtonFocusColor, iButtonDisabledColor, iBorder, 1.0f, XUI_BUTTON_SEMANTIC_DEFAULT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAddPreviewButtonEx(pApp, pClient, __uiDesignFileDialogPreviewText(pNode, "refresh", "Refresh"),
		(xui_rect_t){fPad + 56.0f, fToolbarY + 2.0f, 72.0f, fButtonH},
		iText, iDisabledText, iButtonColor, iButtonHoverColor, iButtonActiveColor, iButtonFocusColor, iButtonDisabledColor, iBorder, 1.0f, XUI_BUTTON_SEMANTIC_DEFAULT);
	if ( iRet != XUI_OK ) return iRet;
	snprintf(sLine, sizeof(sLine), "%s: %s", sPathLabel, __uiDesignText(pNode, "text.initialDir", "."));
	fPathX = fPad + 136.0f;
	fPathW = fW - fPad - fPathX;
	if ( fPathW < 80.0f ) fPathW = 80.0f;
	iRet = __uiDesignAddPreviewRect(pApp, pClient,
		(xui_rect_t){fPathX, fToolbarY + 2.0f, fPathW, fButtonH},
		bPathEdit ? iInputColor : __uiDesignColor(pNode, "appearance.headerColor", XUI_COLOR_RGBA(234, 242, 251, 255)), iBorder, 1.0f);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, sLine,
		(xui_rect_t){fPathX + 8.0f, fToolbarY + 2.0f, fPathW - 16.0f, fButtonH}, bPathEdit ? iText : iMuted,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	fColKindW = 46.0f;
	fColSizeW = 72.0f;
	fColModifiedW = 92.0f;
	fNameColW = fListW - fColKindW - fColSizeW - fColModifiedW - 24.0f;
	if ( fNameColW < 72.0f ) fNameColW = 72.0f;
	iRet = __uiDesignAddPreviewRect(pApp, pClient, (xui_rect_t){fListX, fListY, fListW, fListH}, iListColor, iBorder, 1.0f);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAddPreviewRect(pApp, pClient, (xui_rect_t){fListX, fListY, fListW, fRowH}, __uiDesignColor(pNode, "appearance.headerColor", XUI_COLOR_RGBA(234, 242, 251, 255)), iBorder, 0.0f);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, "Kind", (xui_rect_t){fListX + 8.0f, fListY, fColKindW, fRowH}, iMuted,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, "Name", (xui_rect_t){fListX + 8.0f + fColKindW, fListY, fNameColW, fRowH}, iMuted,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, "Size", (xui_rect_t){fListX + 8.0f + fColKindW + fNameColW, fListY, fColSizeW, fRowH}, iMuted,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, "Modified", (xui_rect_t){fListX + 8.0f + fColKindW + fNameColW + fColSizeW, fListY, fColModifiedW, fRowH}, iMuted,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < iEntryCount; ++i ) {
		fRowY = fRowH + (float)i * fRowH;
		if ( fRowY + fRowH > fListH ) break;
		if ( i == iSelectedEntry ) {
			iRet = __uiDesignAddPreviewRect(pApp, pClient, (xui_rect_t){fListX, fListY + fRowY, fListW, fRowH}, iSelectedColor, XUI_COLOR_RGBA(0, 0, 0, 0), 0.0f);
			if ( iRet != XUI_OK ) return iRet;
		} else if ( i == iHoverEntry ) {
			iRet = __uiDesignAddPreviewRect(pApp, pClient, (xui_rect_t){fListX, fListY + fRowY, fListW, fRowH}, iHoverColor, XUI_COLOR_RGBA(0, 0, 0, 0), 0.0f);
			if ( iRet != XUI_OK ) return iRet;
		}
		iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, __uiDesignFileDialogKindGlyph(arrEntries[i].sKind),
			(xui_rect_t){fListX + 8.0f, fListY + fRowY, fColKindW, fRowH}, arrEntries[i].bEnabled ? iMuted : iDisabledText,
			XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, arrEntries[i].sName,
			(xui_rect_t){fListX + 8.0f + fColKindW, fListY + fRowY, fNameColW, fRowH}, arrEntries[i].bEnabled ? iText : iDisabledText,
			XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, (arrEntries[i].sSize != NULL && arrEntries[i].sSize[0] != 0) ? arrEntries[i].sSize : __uiDesignFileDialogKindText(arrEntries[i].sKind),
			(xui_rect_t){fListX + 8.0f + fColKindW + fNameColW, fListY + fRowY, fColSizeW, fRowH}, arrEntries[i].bEnabled ? iMuted : iDisabledText,
			XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, arrEntries[i].sModified,
			(xui_rect_t){fListX + 8.0f + fColKindW + fNameColW + fColSizeW, fListY + fRowY, fColModifiedW, fRowH}, arrEntries[i].bEnabled ? iMuted : iDisabledText,
			XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	fFieldY = fClientH - fFooterH + 8.0f;
	if ( fFieldY < fListY + fListH + 6.0f ) fFieldY = fListY + fListH + 6.0f;
	fFieldLabelW = 52.0f;
	fActionX = fW - fPad - fButtonW * 2.0f - fButtonGap;
	fNameW = fActionX - fPad - fFieldLabelW - fButtonGap;
	if ( fNameW < 120.0f ) fNameW = 120.0f;
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, sFileLabel, (xui_rect_t){fPad, fFieldY, fFieldLabelW, fButtonH}, iMuted,
		XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAddPreviewRect(pApp, pClient, (xui_rect_t){fPad + fFieldLabelW + 6.0f, fFieldY, fNameW, fButtonH}, iInputColor, iBorder, 1.0f);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, sFileName, (xui_rect_t){fPad + fFieldLabelW + 13.0f, fFieldY, fNameW - 14.0f, fButtonH}, iText,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	fButtonY = fFieldY + 1.0f;
	iRet = __uiDesignAddPreviewButtonEx(pApp, pClient, sOkText,
		(xui_rect_t){fActionX, fButtonY, fButtonW, fButtonH},
		iText, iDisabledText, iButtonColor, iButtonHoverColor, iButtonActiveColor, iButtonFocusColor, iButtonDisabledColor, iBorder, 1.0f, XUI_BUTTON_SEMANTIC_PRIMARY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAddPreviewButtonEx(pApp, pClient, sCancelText,
		(xui_rect_t){fActionX + fButtonW + fButtonGap, fButtonY, fButtonW, fButtonH},
		iText, iDisabledText, iButtonColor, iButtonHoverColor, iButtonActiveColor, iButtonFocusColor, iButtonDisabledColor, iBorder, 1.0f, XUI_BUTTON_SEMANTIC_DEFAULT);
	if ( iRet != XUI_OK ) return iRet;
	if ( !bSelectFolder ) {
		fFilterW = fNameW;
		iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, sTypeLabel,
			(xui_rect_t){fPad, fFieldY + fButtonH + 6.0f, fFieldLabelW, fButtonH}, iMuted,
			XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __uiDesignAddPreviewRect(pApp, pClient,
			(xui_rect_t){fPad + fFieldLabelW + 6.0f, fFieldY + fButtonH + 6.0f, fFilterW, fButtonH},
			iInputColor, iBorder, 1.0f);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, sFilterName,
			(xui_rect_t){fPad + fFieldLabelW + 13.0f, fFieldY + fButtonH + 6.0f, fFilterW - 22.0f, fButtonH}, iText,
			XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, "v",
			(xui_rect_t){fPad + fFieldLabelW + 6.0f + fFilterW - 18.0f, fFieldY + fButtonH + 6.0f, 16.0f, fButtonH}, iMuted,
			XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	snprintf(sLine, sizeof(sLine), "%s%s", __uiDesignBool(pNode, "behavior.modal", 1) ? "Modal" : "Modeless",
		bPathEdit ? ", path edit" : ", breadcrumb");
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, sLine,
		(xui_rect_t){fActionX, fFieldY + fButtonH + 7.0f, fButtonW * 2.0f + fButtonGap, 18.0f}, iMuted,
		XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	return iRet;
}

static int __uiDesignApplyMsgTipPreview(ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pClient,
	float fW, float fClientH, uint32_t iText, uint32_t iMuted, uint32_t iAccent)
{
	xui_surface pIconSurface;
	xui_rect_t tIconSrc;
	xui_rect_t tIconRect;
	float fPadX;
	float fPadY;
	float fIconSize;
	float fIconGap;
	float fTextX;
	float fTextW;
	float fLineH;
	uint32_t iShadowColor;
	int bShowMeta;
	int iType;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pClient == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	fPadX = __uiDesignFloat(pNode, "metrics.paddingX", 16.0f);
	fPadY = __uiDesignFloat(pNode, "metrics.paddingY", 10.0f);
	fIconSize = __uiDesignFloat(pNode, "metrics.iconSize", 20.0f);
	fIconGap = __uiDesignFloat(pNode, "metrics.iconGap", 8.0f);
	iType = __uiDesignInt(pNode, "behavior.type", XUI_MSGBOX_ICON_INFO);
	iShadowColor = __uiDesignColor(pNode, "appearance.shadowColor", XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( (iShadowColor & 0xffu) != 0u ) {
		iRet = __uiDesignAddPreviewRect(pApp, pClient, (xui_rect_t){2.0f, 2.0f, fW, fClientH}, iShadowColor, iShadowColor, 0.0f);
		if ( iRet != XUI_OK ) return iRet;
	}
	pIconSurface = NULL;
	memset(&tIconSrc, 0, sizeof(tIconSrc));
	(void)__uiDesignLoadOverlayIcon(pApp, pNode, &pIconSurface, &tIconSrc);
	fTextX = fPadX;
	if ( pIconSurface != NULL ) {
		tIconRect = (xui_rect_t){fPadX, fPadY, fIconSize, fIconSize};
		iRet = __uiDesignAddPreviewImage(pApp, pClient, pIconSurface, tIconSrc, tIconRect, iAccent);
		if ( iRet != XUI_OK ) return iRet;
		fTextX += fIconSize + fIconGap;
	} else if ( iType != XUI_MSGBOX_ICON_NONE ) {
		iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, __uiDesignMsgBoxIconGlyph(iType),
			(xui_rect_t){fPadX, fPadY, fIconSize, fIconSize}, iAccent,
			XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
		fTextX += fIconSize + fIconGap;
	}
	fTextW = fW - fTextX - fPadX;
	if ( fTextW < 24.0f ) fTextW = 24.0f;
	fLineH = fClientH - fPadY * 2.0f;
	bShowMeta = (fClientH >= 56.0f);
	if ( bShowMeta ) fLineH -= 18.0f;
	if ( fLineH < 18.0f ) fLineH = 18.0f;
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, __uiDesignText(pNode, "text.text", "Saved successfully."),
		(xui_rect_t){fTextX, fPadY, fTextW, fLineH}, iText,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_WRAP_WORD | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	if ( bShowMeta ) {
		char sMeta[96];
		snprintf(sMeta, sizeof(sMeta), "%.1fs, offset %.0f, width %.0f-%.0f",
			__uiDesignFloat(pNode, "behavior.duration", 2.4f),
			__uiDesignFloat(pNode, "metrics.offsetY", -72.0f),
			__uiDesignFloat(pNode, "metrics.minWidth", 92.0f),
			__uiDesignFloat(pNode, "metrics.maxWidth", 360.0f));
		iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, sMeta,
			(xui_rect_t){fTextX, fClientH - fPadY - 16.0f, fTextW, 16.0f}, iMuted,
			XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return iRet;
}

static int __uiDesignApplyToastPreviewItem(ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pClient,
	float fX, float fY, float fW, float fH, const char* sType, const char* sTitle, const char* sMessage,
	const char* sDuration, const char* sClickable, const char* sProgress, uint32_t iText, uint32_t iMuted, uint32_t iBorder)
{
	char sTitleLine[256];
	char sMeta[128];
	float fPadX;
	float fPadY;
	float fIconSize;
	float fIconGap;
	float fCloseSize;
	float fProgressH;
	float fTextX;
	float fTextW;
	float fProgress;
	int iType;
	int iRet;
	uint32_t iAccent;
	uint32_t iCloseColor;
	uint32_t iShadowColor;

	if ( (pApp == NULL) || (pNode == NULL) || (pClient == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iType = __uiDesignToastTypeFromText(sType, __uiDesignInt(pNode, "behavior.type", XUI_TOAST_TYPE_SUCCESS));
	iAccent = __uiDesignToastAccentColor(pNode, iType);
	fPadX = __uiDesignFloat(pNode, "metrics.paddingX", 14.0f);
	fPadY = __uiDesignFloat(pNode, "metrics.paddingY", 10.0f);
	fIconSize = __uiDesignFloat(pNode, "metrics.iconSize", 28.0f);
	fIconGap = __uiDesignFloat(pNode, "metrics.iconGap", 10.0f);
	fCloseSize = __uiDesignFloat(pNode, "metrics.closeSize", 16.0f);
	fProgressH = __uiDesignFloat(pNode, "metrics.progressHeight", 2.0f);
	iShadowColor = __uiDesignColor(pNode, "appearance.shadowColor", XUI_COLOR_RGBA(6, 22, 42, 48));
	if ( (iShadowColor & 0xffu) != 0u ) {
		iRet = __uiDesignAddPreviewButton(pApp, pClient, "",
			(xui_rect_t){fX + 2.0f, fY + 2.0f, fW, fH},
			iText,
			iShadowColor,
			iShadowColor,
			0.0f);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __uiDesignAddPreviewButton(pApp, pClient, "",
		(xui_rect_t){fX, fY, fW, fH},
		iText,
		__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 252, 255, 246)),
		iBorder,
		1.0f);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, __uiDesignToastIconGlyph(iType),
		(xui_rect_t){fX + fPadX, fY + fPadY, fIconSize, fIconSize}, iAccent,
		XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	fTextX = fX + fPadX + fIconSize + fIconGap;
	fTextW = fW - (fTextX - fX) - fPadX - fCloseSize;
	if ( fTextW < 32.0f ) fTextW = 32.0f;
	snprintf(sTitleLine, sizeof(sTitleLine), "%s", (sTitle != NULL && sTitle[0] != 0) ? sTitle : __uiDesignText(pNode, "text.title", "Build Finished"));
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, sTitleLine,
		(xui_rect_t){fTextX, fY + fPadY - 1.0f, fTextW, 20.0f}, iText,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient,
		(sMessage != NULL && sMessage[0] != 0) ? sMessage : __uiDesignText(pNode, "text.message", "No errors were reported."),
		(xui_rect_t){fTextX, fY + fPadY + 21.0f, fTextW, fH - fPadY * 2.0f - 24.0f}, iMuted,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_WRAP_WORD | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	if ( __uiDesignTokenIs(sClickable, "hover") ) {
		iCloseColor = __uiDesignColor(pNode, "appearance.closeHoverColor", XUI_COLOR_RGBA(31, 58, 82, 255));
	} else if ( __uiDesignParseBoolText(sClickable, 1) ) {
		iCloseColor = __uiDesignColor(pNode, "appearance.closeColor", XUI_COLOR_RGBA(96, 126, 148, 255));
	} else {
		iCloseColor = XUI_COLOR_RGBA(160, 170, 182, 150);
	}
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, "x",
		(xui_rect_t){fX + fW - fPadX - fCloseSize, fY + fPadY, fCloseSize, fCloseSize},
		iCloseColor,
		XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet != XUI_OK ) return iRet;
	fProgress = __uiDesignParseFloatText(sProgress, 0.0f);
	if ( fProgress < 0.0f ) fProgress = 0.0f;
	if ( fProgress > 1.0f ) fProgress = 1.0f;
	if ( fProgressH > 0.0f && fProgress > 0.0f ) {
		iRet = __uiDesignAddPreviewButton(pApp, pClient, "",
			(xui_rect_t){fX, fY + fH - fProgressH, fW * fProgress, fProgressH}, iText, iAccent, iAccent, 0.0f);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fH >= 78.0f ) {
		snprintf(sMeta, sizeof(sMeta), "%.1fs%s", __uiDesignParseFloatText(sDuration, __uiDesignFloat(pNode, "behavior.duration", 4.0f)),
			__uiDesignParseBoolText(sClickable, 1) ? ", clickable" : "");
		iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, sMeta,
			(xui_rect_t){fTextX, fY + fH - fPadY - 18.0f, fTextW, 16.0f}, iMuted,
			XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return iRet;
}

static int __uiDesignApplyToastPreview(ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pClient,
	float fW, float fClientH, uint32_t iText, uint32_t iMuted, uint32_t iBorder)
{
	const char* sCursor;
	char sLine[1024];
	char* arrFields[8];
	float fCardW;
	float fCardH;
	float fMargin;
	float fAvailableW;
	float fAvailableH;
	float fGap;
	float fX;
	float fY;
	float fStepY;
	int iPlacement;
	int iDirection;
	int iMaxVisible;
	int iFitVisible;
	int iCount;
	int iFieldCount;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pClient == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	fMargin = __uiDesignFloat(pNode, "metrics.margin", 18.0f);
	if ( fMargin < 0.0f ) fMargin = 0.0f;
	fAvailableW = fW - fMargin * 2.0f;
	if ( fAvailableW < 32.0f ) fAvailableW = (fW > 8.0f) ? fW - 8.0f : fW;
	fAvailableH = fClientH - fMargin * 2.0f;
	if ( fAvailableH < 32.0f ) fAvailableH = (fClientH > 8.0f) ? fClientH - 8.0f : fClientH;
	fCardW = __uiDesignFloat(pNode, "metrics.width", 320.0f);
	if ( fCardW > fAvailableW ) fCardW = fAvailableW;
	if ( fCardW < __uiDesignFloat(pNode, "metrics.minWidth", 180.0f) ) fCardW = __uiDesignFloat(pNode, "metrics.minWidth", 180.0f);
	if ( fCardW > fAvailableW ) fCardW = fAvailableW;
	fCardH = __uiDesignFloat(pNode, "metrics.minHeight", 58.0f);
	if ( fCardH < 46.0f ) fCardH = 46.0f;
	if ( fCardH > fAvailableH ) fCardH = fAvailableH;
	if ( fCardH < 32.0f ) fCardH = 32.0f;
	fGap = __uiDesignFloat(pNode, "metrics.gap", 8.0f);
	iMaxVisible = __uiDesignInt(pNode, "metrics.maxVisible", 0);
	iFitVisible = (int)((fAvailableH + fGap) / (fCardH + fGap));
	if ( iFitVisible < 1 ) iFitVisible = 1;
	if ( iMaxVisible < 1 ) iMaxVisible = iFitVisible;
	if ( iMaxVisible > iFitVisible ) iMaxVisible = iFitVisible;
	iPlacement = __uiDesignInt(pNode, "behavior.placement", XUI_TOAST_PLACEMENT_TOP_RIGHT);
	iDirection = __uiDesignInt(pNode, "behavior.direction", XUI_TOAST_DIRECTION_AUTO);
	if ( iDirection == XUI_TOAST_DIRECTION_AUTO ) {
		iDirection = (iPlacement == XUI_TOAST_PLACEMENT_BOTTOM_LEFT || iPlacement == XUI_TOAST_PLACEMENT_BOTTOM_RIGHT || iPlacement == XUI_TOAST_PLACEMENT_BOTTOM_CENTER) ? XUI_TOAST_DIRECTION_UP : XUI_TOAST_DIRECTION_DOWN;
	}
	if ( iPlacement == XUI_TOAST_PLACEMENT_TOP_LEFT || iPlacement == XUI_TOAST_PLACEMENT_BOTTOM_LEFT ) fX = fMargin;
	else if ( iPlacement == XUI_TOAST_PLACEMENT_TOP_CENTER || iPlacement == XUI_TOAST_PLACEMENT_BOTTOM_CENTER ) fX = (fW - fCardW) * 0.5f;
	else fX = fW - fCardW - fMargin;
	if ( fX > fW - fCardW ) fX = fW - fCardW;
	if ( fX < 0.0f ) fX = 0.0f;
	if ( iDirection == XUI_TOAST_DIRECTION_UP ) {
		fY = fClientH - fCardH - fMargin;
		fStepY = -(fCardH + fGap);
	} else {
		fY = fMargin;
		fStepY = fCardH + fGap;
	}
	if ( fY > fClientH - fCardH ) fY = fClientH - fCardH;
	if ( fY < 0.0f ) fY = 0.0f;
	iCount = 0;
	sCursor = __uiDesignText(pNode, "data.toasts", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) && iCount < iMaxVisible ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount <= 0 ) continue;
		iRet = __uiDesignApplyToastPreviewItem(pApp, pNode, pClient, fX, fY, fCardW, fCardH,
			__uiDesignField(arrFields, iFieldCount, 0, ""),
			__uiDesignField(arrFields, iFieldCount, 1, ""),
			__uiDesignField(arrFields, iFieldCount, 2, ""),
			__uiDesignField(arrFields, iFieldCount, 3, ""),
			__uiDesignField(arrFields, iFieldCount, 4, "true"),
			__uiDesignField(arrFields, iFieldCount, 5, "0"),
			iText, iMuted, iBorder);
		if ( iRet != XUI_OK ) return iRet;
		fY += fStepY;
		++iCount;
	}
	if ( iCount > 0 ) return XUI_OK;
	return __uiDesignApplyToastPreviewItem(pApp, pNode, pClient, fX, fY, fCardW, fCardH,
		"", __uiDesignText(pNode, "text.title", "Build Finished"), __uiDesignText(pNode, "text.message", "No errors were reported."),
		"", "true", "0.5", iText, iMuted, iBorder);
}

static int __uiDesignApplySimpleOverlayPreview(ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pClient,
	float fW, float fClientH, uint32_t iText, uint32_t iMuted, uint32_t iAccent)
{
	char sLine1[128];
	char sLine2[128];
	char sLine3[128];
	float fLineH;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pClient == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	fLineH = (fClientH - 14.0f) / 3.0f;
	if ( fLineH < 16.0f ) fLineH = 16.0f;
	__uiDesignOverlayPreviewText(pNode, sLine1, sizeof(sLine1), sLine2, sizeof(sLine2), sLine3, sizeof(sLine3));
	iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, sLine1, (xui_rect_t){10.0f, 6.0f, fW - 20.0f, fLineH}, iText,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet == XUI_OK ) iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, sLine2, (xui_rect_t){10.0f, 6.0f + fLineH, fW - 20.0f, fLineH}, iMuted,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( iRet == XUI_OK ) iRet = __uiDesignAddPreviewLabelEx(pApp, pClient, sLine3, (xui_rect_t){10.0f, 6.0f + fLineH * 2.0f, fW - 20.0f, fLineH}, iAccent,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	return iRet;
}

static int __uiDesignApplyOverlayPreview(ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	xui_widget pPanel;
	xui_widget pClientWidget;
	uint32_t iBack;
	uint32_t iHeader;
	uint32_t iClient;
	uint32_t iBorder;
	uint32_t iText;
	uint32_t iMuted;
	uint32_t iAccent;
	float fW;
	float fH;
	xui_rect_t tPanelRect;
	float fHeaderH;
	float fClientH;
	float fMinW;
	float fMaxW;
	float fMinH;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pPanel = pNode->pWidget;
	pClientWidget = xuiPanelGetClientWidget(pPanel);
	if ( pClientWidget == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	iBack = XUI_COLOR_RGBA(247, 250, 254, 255);
	iHeader = XUI_COLOR_RGBA(234, 242, 251, 255);
	iClient = XUI_COLOR_WHITE;
	iBorder = XUI_COLOR_RGBA(158, 182, 209, 255);
	iText = XUI_COLOR_RGBA(35, 50, 70, 255);
	iMuted = XUI_COLOR_RGBA(107, 127, 149, 255);
	if ( pNode->iType == UI_DESIGN_NODE_MSG_BOX ) {
		iBack = __uiDesignColor(pNode, "appearance.backdropColor", XUI_COLOR_RGBA(18, 42, 66, 90));
		iClient = __uiDesignColor(pNode, "appearance.clientColor", XUI_COLOR_RGBA(250, 253, 255, 255));
		iBorder = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
		iText = __uiDesignColor(pNode, "appearance.messageColor", XUI_COLOR_RGBA(34, 52, 78, 255));
		iMuted = __uiDesignColor(pNode, "appearance.mutedTextColor", XUI_COLOR_RGBA(93, 113, 138, 255));
	} else if ( pNode->iType == UI_DESIGN_NODE_FILE_DIALOG ) {
		iBack = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 250, 254, 255));
		iHeader = __uiDesignColor(pNode, "appearance.headerColor", XUI_COLOR_RGBA(234, 242, 251, 255));
		iClient = __uiDesignColor(pNode, "appearance.clientColor", XUI_COLOR_WHITE);
		iBorder = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
		iText = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
		iMuted = __uiDesignColor(pNode, "appearance.mutedTextColor", XUI_COLOR_RGBA(107, 127, 149, 255));
	} else if ( pNode->iType == UI_DESIGN_NODE_MSG_TIP ) {
		iBack = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(32, 42, 54, 222));
		iHeader = iBack;
		iClient = iBack;
		iBorder = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(114, 151, 190, 120));
		iText = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_WHITE);
		iMuted = XUI_COLOR_RGBA(210, 220, 232, 255);
	} else if ( pNode->iType == UI_DESIGN_NODE_TOAST ) {
		iClient = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 252, 255, 246));
		iBorder = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(128, 174, 207, 210));
		iText = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(31, 58, 82, 255));
		iMuted = __uiDesignColor(pNode, "appearance.mutedTextColor", XUI_COLOR_RGBA(92, 112, 130, 255));
	}
	iAccent = __uiDesignOverlayPreviewAccent(pNode);
	fW = (pNode->tRect.fW > 40.0f) ? pNode->tRect.fW : 40.0f;
	fH = (pNode->tRect.fH > 36.0f) ? pNode->tRect.fH : 36.0f;
	if ( pNode->iType == UI_DESIGN_NODE_FILE_DIALOG ) {
		fW = __uiDesignFloat(pNode, "metrics.dialogWidth", fW);
		fH = __uiDesignFloat(pNode, "metrics.dialogHeight", fH);
		if ( fW < 220.0f ) fW = 220.0f;
		if ( fH < 160.0f ) fH = 160.0f;
		tPanelRect = pNode->tRect;
		tPanelRect.fW = fW;
		tPanelRect.fH = fH;
		(void)xuiWidgetSetRect(pPanel, tPanelRect);
	} else if ( pNode->iType == UI_DESIGN_NODE_MSG_TIP ) {
		fMinW = __uiDesignFloat(pNode, "metrics.minWidth", 92.0f);
		fMaxW = __uiDesignFloat(pNode, "metrics.maxWidth", 360.0f);
		fMinH = __uiDesignFloat(pNode, "metrics.minHeight", 40.0f);
		if ( fMinW < 40.0f ) fMinW = 40.0f;
		if ( fMinH < 24.0f ) fMinH = 24.0f;
		if ( fMaxW > 0.0f && fMaxW < fMinW ) fMaxW = fMinW;
		if ( fMaxW > 0.0f && fW > fMaxW ) fW = fMaxW;
		if ( fW < fMinW ) fW = fMinW;
		if ( fH < fMinH ) fH = fMinH;
		tPanelRect = pNode->tRect;
		tPanelRect.fW = fW;
		tPanelRect.fH = fH;
		(void)xuiWidgetSetRect(pPanel, tPanelRect);
	}
	fHeaderH = (pNode->iType == UI_DESIGN_NODE_MSG_TIP) ? 0.0f : 25.0f;
	fClientH = fH - fHeaderH;
	if ( fClientH < 24.0f ) fClientH = 24.0f;
	(void)xuiPanelSetTitle(pPanel, __uiDesignOverlayPreviewTitle(pNode));
	(void)xuiPanelSetHeaderHeight(pPanel, fHeaderH);
	(void)xuiPanelSetBackgroundColor(pPanel, iBack);
	(void)xuiPanelSetHeaderColor(pPanel, iHeader);
	(void)xuiPanelSetClientColor(pPanel, iClient);
	(void)xuiPanelSetBorder(pPanel, 1.0f, iBorder);
	__uiDesignClearChildren(pClientWidget);
	__uiDesignTextPoolReset(pNode);
	if ( pNode->iType == UI_DESIGN_NODE_MSG_BOX ) {
		iRet = __uiDesignApplyMsgBoxPreview(pApp, pNode, pClientWidget, fW, fClientH, iText, iMuted, iAccent, iBorder);
	} else if ( pNode->iType == UI_DESIGN_NODE_FILE_DIALOG ) {
		iRet = __uiDesignApplyFileDialogPreview(pApp, pNode, pClientWidget, fW, fClientH, iText, iMuted, iBorder);
	} else if ( pNode->iType == UI_DESIGN_NODE_MSG_TIP ) {
		iRet = __uiDesignApplyMsgTipPreview(pApp, pNode, pClientWidget, fW, fClientH, iText, iMuted, iAccent);
	} else if ( pNode->iType == UI_DESIGN_NODE_TOAST ) {
		iRet = __uiDesignApplyToastPreview(pApp, pNode, pClientWidget, fW, fClientH, iText, iMuted, iBorder);
	} else {
		iRet = __uiDesignApplySimpleOverlayPreview(pApp, pNode, pClientWidget, fW, fClientH, iText, iMuted, iAccent);
	}
	(void)xuiWidgetSetVisible(pPanel, pNode->bVisible && __uiDesignBool(pNode, "behavior.open", 1));
	return iRet;
}

static int __uiDesignCreateOverlayPreview(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_panel_desc_t tDesc;
	xui_widget pPanel;
	xui_widget pOldWidget;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = __uiDesignOverlayPreviewTitle(pNode);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.fHeaderHeight = (pNode->iType == UI_DESIGN_NODE_MSG_TIP) ? 0.0f : 25.0f;
	tDesc.fBorderWidth = 1.0f;
	tDesc.bClipClient = 1;
	iRet = xuiPanelCreate(pApp->pContext, &pPanel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	pOldWidget = pNode->pWidget;
	pNode->pWidget = pPanel;
	iRet = __uiDesignApplyOverlayPreview(pApp, pNode);
	pNode->pWidget = pOldWidget;
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pPanel);
		return iRet;
	}
	*ppWidget = pPanel;
	return XUI_OK;
}

static int __uiDesignLoadBreadcrumbSeparatorIcon(ui_design_app_t* pApp, ui_design_node_t* pNode, xui_surface* ppSurface, xui_rect_t* pSrc)
{
	return __uiDesignLoadRuntimeSurfaceRectSlot(pApp, pNode, 0, "data.separatorIconSource", "separatorIcon", ppSurface, pSrc);
}

static int __uiDesignApplyBreadcrumbSeparatorIcon(ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_surface pSurface;
	xui_rect_t tSrc;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pSurface = NULL;
	memset(&tSrc, 0, sizeof(tSrc));
	iRet = __uiDesignLoadBreadcrumbSeparatorIcon(pApp, pNode, &pSurface, &tSrc);
	if ( iRet != XUI_OK ) return iRet;
	return xuiBreadcrumbSetSeparatorIcon(pWidget, pSurface, tSrc, __uiDesignFloat(pNode, "metrics.separatorIconSize", 12.0f));
}

static int __uiDesignCreateBreadcrumb(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_breadcrumb_desc_t tDesc;
	xui_breadcrumb_item_t arrItems[XUI_BREADCRUMB_MAX_ITEMS];
	xui_surface pSeparatorIcon;
	xui_rect_t tSeparatorIconSrc;
	int iItemCount;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignTextPoolReset(pNode);
	iItemCount = __uiDesignBuildBreadcrumbItems(pNode, arrItems, UI_DESIGN_COUNT_OF(arrItems));
	pSeparatorIcon = NULL;
	memset(&tSeparatorIconSrc, 0, sizeof(tSeparatorIconSrc));
	iRet = __uiDesignLoadBreadcrumbSeparatorIcon(pApp, pNode, &pSeparatorIcon, &tSeparatorIconSrc);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pItems = arrItems;
	tDesc.iItemCount = iItemCount;
	tDesc.sSeparator = __uiDesignText(pNode, "text.separator", "/");
	tDesc.pSeparatorIcon = pSeparatorIcon;
	tDesc.tSeparatorIconSrc = tSeparatorIconSrc;
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iHoverTextColor = __uiDesignColor(pNode, "appearance.hoverTextColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iActiveTextColor = __uiDesignColor(pNode, "appearance.activeTextColor", XUI_COLOR_RGBA(31, 95, 168, 255));
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iSeparatorColor = __uiDesignColor(pNode, "appearance.separatorColor", XUI_COLOR_RGBA(107, 127, 149, 255));
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(0, 0, 0, 0));
	tDesc.fSeparatorIconSize = __uiDesignFloat(pNode, "metrics.separatorIconSize", 12.0f);
	tDesc.fGap = __uiDesignFloat(pNode, "metrics.gap", 6.0f);
	tDesc.fPaddingX = __uiDesignFloat(pNode, "metrics.paddingX", 8.0f);
	tDesc.fPaddingY = __uiDesignFloat(pNode, "metrics.paddingY", 4.0f);
	return xuiBreadcrumbCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignFillVirtualJoystickSurfaces(ui_design_app_t* pApp, ui_design_node_t* pNode, xui_virtual_joystick_desc_t* pDesc)
{
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pDesc == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignLoadRuntimeSurfaceRectSlot(pApp, pNode, 0, "data.baseSource", "base", &pDesc->pBaseSurface, &pDesc->tBaseSrc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignLoadRuntimeSurfaceRectSlot(pApp, pNode, 1, "data.baseActiveSource", "baseActive", &pDesc->pBaseActiveSurface, &pDesc->tBaseActiveSrc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignLoadRuntimeSurfaceRectSlot(pApp, pNode, 2, "data.knobSource", "knob", &pDesc->pKnobSurface, &pDesc->tKnobSrc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignLoadRuntimeSurfaceRectSlot(pApp, pNode, 3, "data.knobActiveSource", "knobActive", &pDesc->pKnobActiveSurface, &pDesc->tKnobActiveSrc);
	if ( iRet != XUI_OK ) return iRet;
	return __uiDesignLoadRuntimeSurfaceRectSlot(pApp, pNode, 4, "data.rippleSource", "ripple", &pDesc->pRippleSurface, &pDesc->tRippleSrc);
}

static int __uiDesignApplyVirtualJoystickSurfaces(ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	xui_virtual_joystick_desc_t tDesc;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	iRet = __uiDesignFillVirtualJoystickSurfaces(pApp, pNode, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiVirtualJoystickSetSurface(pNode->pWidget, XUI_VIRTUAL_JOYSTICK_PART_BASE, tDesc.pBaseSurface, tDesc.tBaseSrc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiVirtualJoystickSetSurface(pNode->pWidget, XUI_VIRTUAL_JOYSTICK_PART_BASE_ACTIVE, tDesc.pBaseActiveSurface, tDesc.tBaseActiveSrc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiVirtualJoystickSetSurface(pNode->pWidget, XUI_VIRTUAL_JOYSTICK_PART_KNOB, tDesc.pKnobSurface, tDesc.tKnobSrc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiVirtualJoystickSetSurface(pNode->pWidget, XUI_VIRTUAL_JOYSTICK_PART_KNOB_ACTIVE, tDesc.pKnobActiveSurface, tDesc.tKnobActiveSrc);
	if ( iRet != XUI_OK ) return iRet;
	return xuiVirtualJoystickSetSurface(pNode->pWidget, XUI_VIRTUAL_JOYSTICK_PART_RIPPLE, tDesc.pRippleSurface, tDesc.tRippleSrc);
}

static int __uiDesignCreateCheckCard(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_check_card_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.bChecked = pNode->bChecked;
	tDesc.fMinWidth = __uiDesignFloat(pNode, "metrics.minWidth", 96.0f);
	tDesc.fMinHeight = __uiDesignFloat(pNode, "metrics.minHeight", 56.0f);
	tDesc.tPadding.fLeft = __uiDesignFloat(pNode, "metrics.paddingLeft", 16.0f);
	tDesc.tPadding.fTop = __uiDesignFloat(pNode, "metrics.paddingTop", 14.0f);
	tDesc.tPadding.fRight = __uiDesignFloat(pNode, "metrics.paddingRight", 16.0f);
	tDesc.tPadding.fBottom = __uiDesignFloat(pNode, "metrics.paddingBottom", 14.0f);
	tDesc.fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	tDesc.fCheckedBorderWidth = __uiDesignFloat(pNode, "metrics.checkedBorderWidth", 2.0f);
	tDesc.fCornerSize = __uiDesignFloat(pNode, "metrics.cornerSize", 18.0f);
	tDesc.fFocusWidth = __uiDesignFloat(pNode, "metrics.focusWidth", 2.0f);
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iHoverBackgroundColor = __uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(240, 246, 253, 255));
	tDesc.iActiveBackgroundColor = __uiDesignColor(pNode, "appearance.activeBackgroundColor", XUI_COLOR_RGBA(226, 240, 255, 255));
	tDesc.iCheckedBackgroundColor = __uiDesignColor(pNode, "appearance.checkedBackgroundColor", XUI_COLOR_RGBA(244, 249, 255, 255));
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iHoverBorderColor = __uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iCheckedBorderColor = __uiDesignColor(pNode, "appearance.checkedBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iDisabledBorderColor = __uiDesignColor(pNode, "appearance.disabledBorderColor", XUI_COLOR_RGBA(214, 219, 225, 160));
	tDesc.iCornerColor = __uiDesignColor(pNode, "appearance.cornerColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iCheckColor = __uiDesignColor(pNode, "appearance.checkColor", XUI_COLOR_WHITE);
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	return xuiCheckCardCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateRadioGroup(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_radio_group_desc_t tDesc;
	xui_widget pGroup;
	xui_widget pOldWidget;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iOrientation = __uiDesignRadioGroupOrientation(pNode);
	tDesc.iSelectedIndex = __uiDesignInt(pNode, "data.selected", 0);
	tDesc.fGap = __uiDesignFloat(pNode, "metrics.gap", 8.0f);
	pGroup = NULL;
	iRet = xuiRadioGroupCreate(pApp->pContext, &pGroup, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	if ( __uiDesignContainerHasModelChildren(pApp, pNode) ) {
		pOldWidget = pNode->pWidget;
		pNode->pWidget = pGroup;
		iRet = __uiDesignAttachContainerModelChildren(pApp, pNode);
		pNode->pWidget = pOldWidget;
		(void)xuiRadioGroupSetSelectedIndex(pGroup, __uiDesignInt(pNode, "data.selected", 0));
	} else {
		iRet = __uiDesignApplyRadioGroupOptions(pApp, pNode, pGroup);
	}
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pGroup);
		return iRet;
	}
	*ppWidget = pGroup;
	return XUI_OK;
}

static int __uiDesignVirtualJoystickChannelFromText(const char* sText, int iDefault)
{
	if ( (sText == NULL) || (sText[0] == 0) ) return iDefault;
	if ( __uiDesignTokenIs(sText, "left") || __uiDesignTokenIs(sText, "l") ) return XUI_VIRTUAL_JOYSTICK_CHANNEL_LEFT;
	if ( __uiDesignTokenIs(sText, "right") || __uiDesignTokenIs(sText, "r") ) return XUI_VIRTUAL_JOYSTICK_CHANNEL_RIGHT;
	if ( __uiDesignTokenIs(sText, "up") || __uiDesignTokenIs(sText, "u") ) return XUI_VIRTUAL_JOYSTICK_CHANNEL_UP;
	if ( __uiDesignTokenIs(sText, "down") || __uiDesignTokenIs(sText, "d") ) return XUI_VIRTUAL_JOYSTICK_CHANNEL_DOWN;
	if ( __uiDesignLooksIntText(sText) ) return __uiDesignParseIntText(sText, iDefault);
	return iDefault;
}

static int __uiDesignApplyVirtualJoystickChannels(ui_design_node_t* pNode, xui_widget pWidget)
{
	const char* sCursor;
	char sLine[256];
	char* arrFields[3];
	int iFieldCount;
	int iChannel;
	int bHasChannels;

	if ( (pNode == NULL) || (pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	bHasChannels = 0;
	sCursor = __uiDesignText(pNode, "data.channels", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount <= 0) || (arrFields[0][0] == 0) ) continue;
		iChannel = __uiDesignVirtualJoystickChannelFromText(arrFields[0], -1);
		if ( (iChannel < 0) || (iChannel >= XUI_VIRTUAL_JOYSTICK_CHANNEL_COUNT) ) continue;
		if ( !bHasChannels ) {
			(void)xuiVirtualJoystickClearChannels(pWidget, 0);
			bHasChannels = 1;
		}
		(void)xuiVirtualJoystickSetChannel(pWidget,
			iChannel,
			__uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 1, "true"), 1),
			__uiDesignParseFloatText(__uiDesignField(arrFields, iFieldCount, 2, "1"), 1.0f),
			0);
	}
	if ( bHasChannels ) return XUI_OK;
	return xuiVirtualJoystickSetValue(pWidget,
		__uiDesignFloat(pNode, "value.x", 0.0f),
		__uiDesignFloat(pNode, "value.y", 0.0f),
		0);
}

static int __uiDesignCreateVirtualJoystick(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_virtual_joystick_desc_t tDesc;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fRadius = __uiDesignFloat(pNode, "metrics.radius", 58.0f);
	tDesc.fKnobSize = __uiDesignFloat(pNode, "metrics.knobSize", 34.0f);
	tDesc.fDeadZone = __uiDesignFloat(pNode, "metrics.deadZone", 0.1f);
	tDesc.bUseBuiltinAtlas = __uiDesignBool(pNode, "behavior.useBuiltinAtlas", 1);
	tDesc.iBaseColor = __uiDesignColor(pNode, "appearance.baseColor", XUI_COLOR_RGBA(226, 234, 243, 255));
	tDesc.iBaseActiveColor = __uiDesignColor(pNode, "appearance.baseActiveColor", XUI_COLOR_RGBA(207, 227, 250, 255));
	tDesc.iKnobColor = __uiDesignColor(pNode, "appearance.knobColor", XUI_COLOR_WHITE);
	tDesc.iKnobActiveColor = __uiDesignColor(pNode, "appearance.knobActiveColor", XUI_COLOR_RGBA(247, 251, 255, 255));
	tDesc.iRippleColor = __uiDesignColor(pNode, "appearance.rippleColor", XUI_COLOR_RGBA(49, 126, 214, 51));
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(183, 195, 208, 255));
	iRet = __uiDesignFillVirtualJoystickSurfaces(pApp, pNode, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	return xuiVirtualJoystickCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateInventoryGrid(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_inventory_grid_desc_t tDesc;
	xui_inventory_slot_t arrSlots[UI_DESIGN_RUNTIME_TABLE_ROWS];
	int iSlotDataCount;
	int iSlotCount;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iSlotDataCount = __uiDesignBuildInventorySlots(pApp, pNode, arrSlots, UI_DESIGN_COUNT_OF(arrSlots));
	iSlotCount = __uiDesignInt(pNode, "data.slotCount", 24);
	if ( iSlotCount < iSlotDataCount ) iSlotCount = iSlotDataCount;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.arrSlots = (iSlotDataCount > 0) ? arrSlots : NULL;
	tDesc.iSlotCount = iSlotCount;
	tDesc.bHasLayout = 1;
	tDesc.bHasColors = 1;
	__uiDesignFillInventoryLayout(pNode, &tDesc.tLayout);
	__uiDesignFillInventoryColors(pNode, &tDesc.tColors);
	return xuiInventoryGridCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateTerminal(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_terminal_desc_t tDesc;
	xui_widget pTerminal;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.iColumns = __uiDesignInt(pNode, "metrics.columns", 80);
	tDesc.iRows = __uiDesignInt(pNode, "metrics.rows", 24);
	tDesc.iScrollbackLimit = __uiDesignInt(pNode, "metrics.scrollback", 1000);
	tDesc.iParseBudgetBytes = __uiDesignInt(pNode, "metrics.parseBudget", 4096);
	tDesc.fCellWidth = __uiDesignFloat(pNode, "metrics.cellWidth", 0.0f);
	tDesc.fCellHeight = __uiDesignFloat(pNode, "metrics.cellHeight", 0.0f);
	tDesc.fPadding = __uiDesignFloat(pNode, "metrics.padding", 8.0f);
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(16, 24, 32, 255));
	tDesc.iForegroundColor = __uiDesignColor(pNode, "appearance.foregroundColor", XUI_COLOR_RGBA(215, 228, 242, 255));
	tDesc.iCursorColor = __uiDesignColor(pNode, "appearance.cursorColor", XUI_COLOR_WHITE);
	tDesc.iSelectionColor = __uiDesignColor(pNode, "appearance.selectionColor", XUI_COLOR_RGBA(49, 126, 214, 128));
	tDesc.iSelectionTextColor = __uiDesignColor(pNode, "appearance.selectionTextColor", XUI_COLOR_WHITE);
	tDesc.iSearchHighlightColor = __uiDesignColor(pNode, "appearance.searchHighlightColor", XUI_COLOR_RGBA(255, 197, 66, 110));
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iLinkHoverColor = __uiDesignColor(pNode, "appearance.linkHoverColor", XUI_COLOR_RGBA(86, 156, 214, 255));
	pTerminal = NULL;
	iRet = xuiTerminalCreate(pApp->pContext, &pTerminal, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	__uiDesignApplyTerminalPalette(pNode, pTerminal);
	(void)xuiTerminalSetBracketedPaste(pTerminal, __uiDesignBool(pNode, "behavior.bracketedPaste", 0));
	(void)xuiTerminalSetLigaturesEnabled(pTerminal, __uiDesignBool(pNode, "behavior.ligaturesEnabled", 0));
	*ppWidget = pTerminal;
	return XUI_OK;
}

static int __uiDesignCreateSplitLayout(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_split_layout_desc_t tDesc;
	xui_widget pSplit;
	xui_widget pOldWidget;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iOrientation = __uiDesignInt(pNode, "behavior.orientation", XUI_ORIENTATION_VERTICAL);
	tDesc.iPaneCount = __uiDesignInt(pNode, "data.paneCount", 2);
	tDesc.bShadowDrag = __uiDesignBool(pNode, "behavior.shadowDrag", 1);
	tDesc.fDividerSize = __uiDesignFloat(pNode, "metrics.dividerSize", 6.0f);
	tDesc.fDividerVisualSize = __uiDesignFloat(pNode, "metrics.dividerVisualSize", 1.0f);
	tDesc.fDividerHitSize = __uiDesignFloat(pNode, "metrics.dividerHitSize", 8.0f);
	tDesc.iDividerColor = __uiDesignColor(pNode, "appearance.dividerColor", XUI_COLOR_RGBA(202, 214, 228, 255));
	tDesc.iDividerHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	tDesc.iDividerActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iShadowColor = __uiDesignColor(pNode, "appearance.shadowColor", XUI_COLOR_RGBA(49, 126, 214, 51));
	iRet = xuiSplitLayoutCreate(pApp->pContext, &pSplit, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	pOldWidget = pNode->pWidget;
	pNode->pWidget = pSplit;
	iRet = __uiDesignApplySplitLayoutPanes(pApp, pNode);
	pNode->pWidget = pOldWidget;
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pSplit);
		return iRet;
	}
	*ppWidget = pSplit;
	return XUI_OK;
}

static void __uiDesignTabsCloseNoop(xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	(void)iIndex;
	(void)pUser;
}

static int __uiDesignCreateTabs(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_tabs_desc_t tDesc;
	const char* arrItems[XUI_TABS_PAGE_CAPACITY];
	int arrEnabled[XUI_TABS_PAGE_CAPACITY];
	int arrDirty[XUI_TABS_PAGE_CAPACITY];
	xui_surface arrIcons[XUI_TABS_PAGE_CAPACITY];
	xui_rect_t arrIconSrc[XUI_TABS_PAGE_CAPACITY];
	xui_widget pTabs;
	int iItemCount;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	__uiDesignTextPoolReset(pNode);
	iItemCount = __uiDesignBuildTabsItems(pApp, pNode, arrItems, arrEnabled, arrDirty, arrIcons, arrIconSrc, UI_DESIGN_COUNT_OF(arrItems));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = arrItems;
	tDesc.arrEnabled = arrEnabled;
	tDesc.arrDirty = arrDirty;
	tDesc.arrIcons = arrIcons;
	tDesc.arrIconSrc = arrIconSrc;
	tDesc.iItemCount = iItemCount;
	tDesc.iSelected = __uiDesignInt(pNode, "data.selected", 0);
	tDesc.iPlacement = __uiDesignInt(pNode, "behavior.placement", XUI_TABS_PLACEMENT_TOP);
	tDesc.bScrollable = __uiDesignBool(pNode, "behavior.scrollable", 1);
	tDesc.bCloseButtons = __uiDesignBool(pNode, "behavior.closeButtons", 1);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.fTabWidth = __uiDesignFloat(pNode, "metrics.tabWidth", 92.0f);
	tDesc.fTabHeight = __uiDesignFloat(pNode, "metrics.tabHeight", 30.0f);
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iTabColor = __uiDesignColor(pNode, "appearance.tabColor", XUI_COLOR_RGBA(234, 241, 248, 255));
	tDesc.iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(244, 249, 255, 255));
	tDesc.iActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_WHITE);
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(238, 242, 247, 255));
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iActiveTextColor = __uiDesignColor(pNode, "appearance.activeTextColor", XUI_COLOR_RGBA(31, 95, 168, 255));
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iClientColor = __uiDesignColor(pNode, "appearance.clientColor", XUI_COLOR_WHITE);
	pTabs = NULL;
	iRet = xuiTabsCreate(pApp->pContext, &pTabs, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiTabsSetClose(pTabs, __uiDesignTabsCloseNoop, tDesc.bCloseButtons, NULL);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pTabs);
		return iRet;
	}
	*ppWidget = pTabs;
	return XUI_OK;
}

static int __uiDesignCreateAccordion(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_accordion_desc_t tDesc;
	xui_accordion_section_desc_t arrSections[XUI_ACCORDION_SECTION_CAPACITY];
	int iSectionCount;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignTextPoolReset(pNode);
	iSectionCount = __uiDesignBuildAccordionSections(pNode, arrSections, UI_DESIGN_COUNT_OF(arrSections));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrSections = arrSections;
	tDesc.iSectionCount = iSectionCount;
	tDesc.iMode = __uiDesignInt(pNode, "behavior.mode", XUI_ACCORDION_MODE_MULTIPLE);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.fHeaderHeight = __uiDesignFloat(pNode, "metrics.headerHeight", 28.0f);
	tDesc.fSpacing = __uiDesignFloat(pNode, "metrics.spacing", 4.0f);
	tDesc.fContentPadding = __uiDesignFloat(pNode, "metrics.contentPadding", 8.0f);
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iHeaderColor = __uiDesignColor(pNode, "appearance.headerColor", XUI_COLOR_RGBA(234, 241, 248, 255));
	tDesc.iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(244, 249, 255, 255));
	tDesc.iExpandedColor = __uiDesignColor(pNode, "appearance.expandedColor", XUI_COLOR_RGBA(217, 234, 254, 255));
	tDesc.iContentColor = __uiDesignColor(pNode, "appearance.contentColor", XUI_COLOR_WHITE);
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iActiveTextColor = __uiDesignColor(pNode, "appearance.activeTextColor", XUI_COLOR_RGBA(31, 95, 168, 255));
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	return xuiAccordionCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateWindow(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_window_desc_t tDesc;
	xui_surface pIconSurface;
	xui_rect_t tIconSrc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = __uiDesignText(pNode, "text.title", "Window");
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.bClosed = 1;
	tDesc.bTopMost = __uiDesignBool(pNode, "behavior.topMost", 0);
	tDesc.bNoTitleBar = !__uiDesignBool(pNode, "behavior.showTitleBar", 1);
	tDesc.bNotMovable = !__uiDesignBool(pNode, "behavior.movable", 1);
	tDesc.bDragAnywhere = __uiDesignBool(pNode, "behavior.dragAnywhere", 0);
	tDesc.bNotResizable = !__uiDesignBool(pNode, "behavior.resizable", 1);
	tDesc.bHideCollapse = !__uiDesignBool(pNode, "behavior.showCollapse", 1);
	tDesc.bHideMaximize = !__uiDesignBool(pNode, "behavior.showMaximize", 1);
	tDesc.bHideClose = !__uiDesignBool(pNode, "behavior.showClose", 1);
	tDesc.bCollapsed = __uiDesignBool(pNode, "behavior.collapsed", 0);
	tDesc.bMaximized = __uiDesignBool(pNode, "behavior.maximized", 0);
	tDesc.iResizeEdges = (uint32_t)__uiDesignInt(pNode, "behavior.resizeEdges", XUI_WINDOW_EDGE_ALL);
	tDesc.fTitleBarHeight = __uiDesignFloat(pNode, "metrics.titleBarHeight", 30.0f);
	tDesc.fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	tDesc.fResizeGrip = __uiDesignFloat(pNode, "metrics.resizeGrip", 6.0f);
	tDesc.fButtonSize = __uiDesignFloat(pNode, "metrics.buttonSize", 24.0f);
	tDesc.fIconSize = __uiDesignFloat(pNode, "metrics.iconSize", 16.0f);
	tDesc.fMinWidth = __uiDesignFloat(pNode, "metrics.minWidth", 120.0f);
	tDesc.fMinHeight = __uiDesignFloat(pNode, "metrics.minHeight", 80.0f);
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 250, 254, 255));
	tDesc.iClientColor = __uiDesignColor(pNode, "appearance.clientColor", XUI_COLOR_WHITE);
	tDesc.iTitleBarColor = __uiDesignColor(pNode, "appearance.titleBarColor", XUI_COLOR_RGBA(221, 235, 250, 255));
	tDesc.iInactiveTitleBarColor = __uiDesignColor(pNode, "appearance.inactiveTitleBarColor", XUI_COLOR_RGBA(236, 245, 251, 255));
	tDesc.iTitleTextColor = __uiDesignColor(pNode, "appearance.titleTextColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iInactiveTitleTextColor = __uiDesignColor(pNode, "appearance.inactiveTitleTextColor", XUI_COLOR_RGBA(102, 123, 142, 255));
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	tDesc.iActiveBorderColor = __uiDesignColor(pNode, "appearance.activeBorderColor", XUI_COLOR_RGBA(47, 128, 237, 255));
	tDesc.iButtonColor = __uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(0, 0, 0, 0));
	tDesc.iButtonHoverColor = __uiDesignColor(pNode, "appearance.buttonHoverColor", XUI_COLOR_RGBA(188, 215, 245, 255));
	tDesc.iButtonActiveColor = __uiDesignColor(pNode, "appearance.buttonActiveColor", XUI_COLOR_RGBA(167, 201, 238, 255));
	tDesc.iCloseHoverColor = __uiDesignColor(pNode, "appearance.closeHoverColor", XUI_COLOR_RGBA(255, 226, 226, 255));
	tDesc.iCloseActiveColor = __uiDesignColor(pNode, "appearance.closeActiveColor", XUI_COLOR_RGBA(244, 205, 205, 255));
	if ( __uiDesignLoadRuntimeSurface(pApp, pNode, __uiDesignText(pNode, "data.iconSource", ""), &pIconSurface) == XUI_OK && pIconSurface != NULL ) {
		memset(&tIconSrc, 0, sizeof(tIconSrc));
		__uiDesignReadPrefixedSourceRect(pNode, "icon", &tIconSrc);
		tDesc.pIconSurface = pIconSurface;
		tDesc.tIconSrc = tIconSrc;
		tDesc.bHasIcon = 1;
	}
	return xuiWindowCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateScrollFrame(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_scroll_frame_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fContentWidth = __uiDesignFloat(pNode, "metrics.contentWidth", 520.0f);
	tDesc.fContentHeight = __uiDesignFloat(pNode, "metrics.contentHeight", 360.0f);
	tDesc.fOffsetX = __uiDesignFloat(pNode, "value.offsetX", 0.0f);
	tDesc.fOffsetY = __uiDesignFloat(pNode, "value.offsetY", 0.0f);
	tDesc.iPolicyX = __uiDesignInt(pNode, "behavior.policyX", XUI_SCROLLBAR_POLICY_AUTO);
	tDesc.iPolicyY = __uiDesignInt(pNode, "behavior.policyY", XUI_SCROLLBAR_POLICY_AUTO);
	tDesc.iScrollbarMode = __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL);
	tDesc.iWheelAxis = __uiDesignInt(pNode, "behavior.wheelAxis", XUI_WHEEL_AXIS_VERTICAL);
	tDesc.iCornerMode = __uiDesignInt(pNode, "behavior.cornerMode", XUI_SCROLL_FRAME_CORNER_AUTO);
	tDesc.bContentDragEnabled = __uiDesignBool(pNode, "behavior.contentDrag", 0);
	tDesc.fScrollbarSize = __uiDesignFloat(pNode, "metrics.scrollbarSize", 14.0f);
	tDesc.fMinThumbSize = __uiDesignFloat(pNode, "metrics.minThumbSize", 24.0f);
	tDesc.fButtonSize = __uiDesignFloat(pNode, "metrics.buttonSize", 16.0f);
	tDesc.fWheelStep = __uiDesignFloat(pNode, "metrics.wheelStep", 48.0f);
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iTrackColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255));
	tDesc.iThumbColor = __uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255));
	tDesc.iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	tDesc.iActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(111, 137, 166, 255));
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(206, 215, 226, 255));
	tDesc.iButtonColor = __uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(239, 245, 252, 255));
	tDesc.iButtonIconColor = __uiDesignColor(pNode, "appearance.buttonIconColor", XUI_COLOR_RGBA(52, 71, 93, 255));
	tDesc.iCornerColor = __uiDesignColor(pNode, "appearance.cornerColor", XUI_COLOR_RGBA(238, 243, 249, 255));
	tDesc.iGripColor = __uiDesignColor(pNode, "appearance.gripColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	return xuiScrollFrameCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateScrollView(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_scroll_view_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fContentWidth = __uiDesignFloat(pNode, "metrics.contentWidth", 520.0f);
	tDesc.fContentHeight = __uiDesignFloat(pNode, "metrics.contentHeight", 360.0f);
	tDesc.fOffsetX = __uiDesignFloat(pNode, "value.offsetX", 0.0f);
	tDesc.fOffsetY = __uiDesignFloat(pNode, "value.offsetY", 0.0f);
	tDesc.iPolicyX = __uiDesignInt(pNode, "behavior.policyX", XUI_SCROLLBAR_POLICY_AUTO);
	tDesc.iPolicyY = __uiDesignInt(pNode, "behavior.policyY", XUI_SCROLLBAR_POLICY_AUTO);
	tDesc.iScrollbarMode = __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL);
	tDesc.iWheelAxis = __uiDesignInt(pNode, "behavior.wheelAxis", XUI_WHEEL_AXIS_VERTICAL);
	tDesc.iCornerMode = __uiDesignInt(pNode, "behavior.cornerMode", XUI_SCROLL_FRAME_CORNER_AUTO);
	tDesc.bContentDragEnabled = __uiDesignBool(pNode, "behavior.contentDrag", 0);
	tDesc.fScrollbarSize = __uiDesignFloat(pNode, "metrics.scrollbarSize", 14.0f);
	tDesc.fMinThumbSize = __uiDesignFloat(pNode, "metrics.minThumbSize", 24.0f);
	tDesc.fButtonSize = __uiDesignFloat(pNode, "metrics.buttonSize", 16.0f);
	tDesc.fWheelStep = __uiDesignFloat(pNode, "metrics.wheelStep", 48.0f);
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iTrackColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255));
	tDesc.iThumbColor = __uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255));
	tDesc.iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	tDesc.iActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(111, 137, 166, 255));
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(206, 215, 226, 255));
	tDesc.iButtonColor = __uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(239, 245, 252, 255));
	tDesc.iButtonIconColor = __uiDesignColor(pNode, "appearance.buttonIconColor", XUI_COLOR_RGBA(52, 71, 93, 255));
	tDesc.iCornerColor = __uiDesignColor(pNode, "appearance.cornerColor", XUI_COLOR_RGBA(238, 243, 249, 255));
	tDesc.iGripColor = __uiDesignColor(pNode, "appearance.gripColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	return xuiScrollViewCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateCanvas(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_canvas_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fCanvasWidth = __uiDesignFloat(pNode, "metrics.canvasWidth", 520.0f);
	tDesc.fCanvasHeight = __uiDesignFloat(pNode, "metrics.canvasHeight", 360.0f);
	tDesc.fOffsetX = __uiDesignFloat(pNode, "value.offsetX", 0.0f);
	tDesc.fOffsetY = __uiDesignFloat(pNode, "value.offsetY", 0.0f);
	tDesc.iPolicyX = __uiDesignInt(pNode, "behavior.policyX", XUI_SCROLLBAR_POLICY_AUTO);
	tDesc.iPolicyY = __uiDesignInt(pNode, "behavior.policyY", XUI_SCROLLBAR_POLICY_AUTO);
	tDesc.iScrollbarMode = __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_COMPACT);
	tDesc.iWheelAxis = __uiDesignInt(pNode, "behavior.wheelAxis", XUI_WHEEL_AXIS_BOTH);
	tDesc.iCornerMode = __uiDesignInt(pNode, "behavior.cornerMode", XUI_SCROLL_FRAME_CORNER_AUTO);
	tDesc.bContentDragEnabled = __uiDesignBool(pNode, "behavior.contentDrag", 1);
	tDesc.bPenEnabled = __uiDesignBool(pNode, "behavior.penEnabled", 0);
	tDesc.fPenWidth = __uiDesignFloat(pNode, "metrics.penWidth", 2.0f);
	tDesc.fScrollbarSize = __uiDesignFloat(pNode, "metrics.scrollbarSize", 14.0f);
	tDesc.fMinThumbSize = __uiDesignFloat(pNode, "metrics.minThumbSize", 24.0f);
	tDesc.fButtonSize = __uiDesignFloat(pNode, "metrics.buttonSize", 0.0f);
	tDesc.fWheelStep = __uiDesignFloat(pNode, "metrics.wheelStep", 48.0f);
	tDesc.iClearColor = __uiDesignColor(pNode, "appearance.clearColor", XUI_COLOR_WHITE);
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iPenColor = __uiDesignColor(pNode, "appearance.penColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iTrackColor = __uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255));
	tDesc.iThumbColor = __uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255));
	tDesc.iHoverColor = __uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	tDesc.iActiveColor = __uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(111, 137, 166, 255));
	tDesc.iFocusColor = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	tDesc.iDisabledColor = __uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(206, 215, 226, 255));
	tDesc.iButtonColor = __uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(239, 245, 252, 255));
	tDesc.iButtonIconColor = __uiDesignColor(pNode, "appearance.buttonIconColor", XUI_COLOR_RGBA(52, 71, 93, 255));
	tDesc.iCornerColor = __uiDesignColor(pNode, "appearance.cornerColor", XUI_COLOR_RGBA(238, 243, 249, 255));
	tDesc.iGripColor = __uiDesignColor(pNode, "appearance.gripColor", XUI_COLOR_RGBA(142, 166, 192, 255));
	return xuiCanvasCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateMessageList(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_message_list_desc_t tDesc;
	xui_message_node_t arrNodes[64];
	int iNodeCount;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignTextPoolReset(pNode);
	iNodeCount = (__uiDesignInt(pNode, "data.sourceMode", 0) == 0) ? __uiDesignBuildMessageNodes(pNode, arrNodes, UI_DESIGN_COUNT_OF(arrNodes)) : 0;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrNodes = arrNodes;
	tDesc.iNodeCount = iNodeCount;
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.bHasMetrics = 1;
	tDesc.bHasColors = 1;
	tDesc.bAutoScroll = __uiDesignBool(pNode, "behavior.autoScroll", 1);
	__uiDesignFillMessageMetrics(pNode, &tDesc.tMetrics);
	__uiDesignFillMessageColors(pNode, &tDesc.tColors);
	iRet = xuiMessageListCreate(pApp->pContext, ppWidget, &tDesc);
	if ( iRet == XUI_OK && __uiDesignInt(pNode, "data.sourceMode", 0) != 0 ) {
		iRet = __uiDesignApplyMessageData(pNode, *ppWidget);
		if ( iRet != XUI_OK ) {
			xuiWidgetDestroy(*ppWidget);
			*ppWidget = NULL;
		}
	}
	return iRet;
}

static int __uiDesignCreateTimelineView(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_timeline_view_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.iFrameCount = __uiDesignInt(pNode, "data.frameCount", 120);
	tDesc.fFrameRate = __uiDesignFloat(pNode, "value.frameRate", 24.0f);
	tDesc.iCurrentFrame = __uiDesignInt(pNode, "data.currentFrame", 24);
	tDesc.fLayerHeaderWidth = __uiDesignFloat(pNode, "metrics.layerHeaderWidth", 110.0f);
	tDesc.fFrameWidth = __uiDesignFloat(pNode, "metrics.frameWidth", 16.0f);
	tDesc.fMinFrameWidth = __uiDesignFloat(pNode, "metrics.minFrameWidth", 8.0f);
	tDesc.fMaxFrameWidth = __uiDesignFloat(pNode, "metrics.maxFrameWidth", 64.0f);
	tDesc.fRowHeight = __uiDesignFloat(pNode, "metrics.rowHeight", 24.0f);
	tDesc.fRulerHeight = __uiDesignFloat(pNode, "metrics.rulerHeight", 24.0f);
	tDesc.iScrollbarMode = __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL);
	tDesc.bShowVisibilityFeature = __uiDesignBool(pNode, "behavior.visibilityFeature", 1);
	tDesc.bShowLockFeature = __uiDesignBool(pNode, "behavior.lockFeature", 1);
	__uiDesignFillTimelineColors(pNode, &tDesc.tColors);
	return xuiTimeLineViewCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreatePropertyGrid(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_property_grid_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.fNameWidth = __uiDesignFloat(pNode, "metrics.nameWidth", 110.0f);
	tDesc.fRowHeight = __uiDesignFloat(pNode, "metrics.rowHeight", 24.0f);
	tDesc.fCategoryHeight = __uiDesignFloat(pNode, "metrics.categoryHeight", 24.0f);
	tDesc.iDescriptionMode = __uiDesignInt(pNode, "behavior.descriptionMode", XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP);
	tDesc.fDescriptionPanelHeight = __uiDesignFloat(pNode, "metrics.descriptionHeight", 70.0f);
	tDesc.iEditMode = __uiDesignInt(pNode, "behavior.editMode", XUI_TABLE_GRID_EDIT_QUICK);
	tDesc.iScrollbarMode = __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL);
	__uiDesignFillPropertyGridStyle(pNode, &tDesc.tStyle);
	return xuiPropertyGridCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateMenuBar(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_menubar_desc_t tDesc;
	xui_widget pMenuBar;
	xui_menubar_item_t arrItems[XUI_MENUBAR_ITEM_CAPACITY];
	int iItemCount;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	__uiDesignTextPoolReset(pNode);
	iItemCount = __uiDesignBuildMenuBarItems(pNode, arrItems, UI_DESIGN_COUNT_OF(arrItems));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.bHasMetrics = 1;
	tDesc.bHasColors = 1;
	__uiDesignFillMenuBarMetrics(pNode, &tDesc.tMetrics);
	__uiDesignFillMenuBarColors(pNode, &tDesc.tColors);
	iRet = xuiMenuBarCreate(pApp->pContext, &pMenuBar, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignApplyMenuBarMenus(pApp, pNode, pMenuBar, arrItems, iItemCount);
	if ( iRet != XUI_OK ) {
		__uiDesignClearRuntimeMenus(pNode);
		xuiWidgetDestroy(pMenuBar);
		return iRet;
	}
	iRet = xuiMenuBarSetItems(pMenuBar, arrItems, iItemCount);
	if ( iRet != XUI_OK ) {
		__uiDesignClearRuntimeMenus(pNode);
		xuiWidgetDestroy(pMenuBar);
		return iRet;
	}
	*ppWidget = pMenuBar;
	return XUI_OK;
}

static int __uiDesignCreateToolbar(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_toolbar_desc_t tDesc;
	xui_toolbar_item_t arrItems[XUI_TOOLBAR_ITEM_CAPACITY];
	int iItemCount;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignTextPoolReset(pNode);
	iItemCount = __uiDesignBuildToolbarItems(pApp, pNode, arrItems, UI_DESIGN_COUNT_OF(arrItems));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pItems = arrItems;
	tDesc.iItemCount = iItemCount;
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.bHasMetrics = 1;
	tDesc.bHasColors = 1;
	tDesc.bOverflowEnabled = __uiDesignBool(pNode, "behavior.overflow", 1);
	__uiDesignFillToolbarMetrics(pNode, &tDesc.tMetrics);
	__uiDesignFillToolbarColors(pNode, &tDesc.tColors);
	return xuiToolbarCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateStatusBar(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_statusbar_desc_t tDesc;
	xui_statusbar_item_t arrItems[XUI_STATUSBAR_ITEM_CAPACITY];
	int iItemCount;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignTextPoolReset(pNode);
	iItemCount = __uiDesignBuildStatusBarItems(pNode, arrItems, UI_DESIGN_COUNT_OF(arrItems));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pItems = arrItems;
	tDesc.iItemCount = iItemCount;
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.bHasMetrics = 1;
	tDesc.bHasColors = 1;
	__uiDesignFillStatusBarMetrics(pNode, &tDesc.tMetrics);
	__uiDesignFillStatusBarColors(pNode, &tDesc.tColors);
	return xuiStatusBarCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateDockPanel(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_dock_panel_desc_t tDesc;
	xui_widget pDock;
	xui_widget pOldWidget;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.bHasMetrics = 1;
	tDesc.bHasColors = 1;
	__uiDesignFillDockPanelMetrics(pNode, &tDesc.tMetrics);
	__uiDesignFillDockPanelColors(pNode, &tDesc.tColors);
	iRet = xuiDockPanelCreate(pApp->pContext, &pDock, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	pOldWidget = pNode->pWidget;
	pNode->pWidget = pDock;
	iRet = __uiDesignApplyDockPanelWindows(pApp, pNode);
	if ( iRet == XUI_OK ) iRet = __uiDesignApplyDockPanelRegions(pNode);
	if ( iRet == XUI_OK ) iRet = __uiDesignApplyDockPanelMenuTitles(pNode, pDock);
	if ( iRet == XUI_OK ) iRet = __uiDesignApplyDockPanelTooltips(pNode, pDock);
	pNode->pWidget = pOldWidget;
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pDock);
		return iRet;
	}
	*ppWidget = pDock;
	return XUI_OK;
}

static int __uiDesignCreatePopup(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_popup_desc_t tDesc;
	xui_widget pContent;
	xui_widget pPopup;
	xui_widget pOldWidget;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignFillPopupDesc(pNode, &tDesc);
	iRet = xuiPopupCreate(pApp->pContext, &pPopup, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	pOldWidget = pNode->pWidget;
	pNode->pWidget = pPopup;
	if ( __uiDesignContainerHasModelChildren(pApp, pNode) ) {
		pContent = xuiPopupGetContentWidget(pPopup);
		if ( pContent != NULL ) {
			__uiDesignApplyContainerHostLayout(pNode, pContent, 1);
			__uiDesignClearChildren(pContent);
		}
		iRet = __uiDesignAttachContainerModelChildren(pApp, pNode);
	} else {
		iRet = __uiDesignApplyPopupContent(pApp, pNode);
	}
	pNode->pWidget = pOldWidget;
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pPopup);
		return iRet;
	}
	*ppWidget = pPopup;
	return XUI_OK;
}

static int __uiDesignCreateMenu(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_menu_desc_t tDesc;
	xui_menu_item_t arrItems[XUI_MENU_ITEM_CAPACITY];
	xui_menu_metrics_t tMetrics;
	xui_menu_colors_t tColors;
	xui_widget pMenu;
	xui_widget pPopup;
	int iItemCount;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	__uiDesignClearRuntimeMenus(pNode);
	__uiDesignTextPoolReset(pNode);
	iItemCount = __uiDesignBuildMenuItems(pNode, arrItems, UI_DESIGN_COUNT_OF(arrItems));
	__uiDesignFillMenuMetrics(pNode, &tMetrics);
	__uiDesignFillMenuColors(pNode, &tColors);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.bHasMetrics = 1;
	tDesc.bHasColors = 1;
	tDesc.tMetrics = tMetrics;
	tDesc.tColors = tColors;
	iRet = xuiMenuCreate(pApp->pContext, &pMenu, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignApplyMenuItemSubmenus(pApp, pNode, pMenu, NULL, -1, 0, arrItems, iItemCount, "", &tMetrics, &tColors);
	if ( iRet != XUI_OK ) {
		__uiDesignClearRuntimeMenus(pNode);
		xuiWidgetDestroy(xuiMenuGetPopupWidget(pMenu));
		return iRet;
	}
	iRet = xuiMenuSetItems(pMenu, arrItems, iItemCount);
	if ( iRet != XUI_OK ) {
		__uiDesignClearRuntimeMenus(pNode);
		xuiWidgetDestroy(xuiMenuGetPopupWidget(pMenu));
		return iRet;
	}
	pPopup = xuiMenuGetPopupWidget(pMenu);
	if ( pPopup == NULL ) {
		__uiDesignClearRuntimeMenus(pNode);
		xuiWidgetDestroy(pMenu);
		return XUI_ERROR_NOT_INITIALIZED;
	}
	*ppWidget = pPopup;
	return XUI_OK;
}

static int __uiDesignCreateCascader(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_cascader_desc_t tDesc;
	xui_cascader_item_t arrItems[XUI_CASCADER_ITEM_CAPACITY];
	int arrSelectedPath[16];
	int iItemCount;
	int iSelectedDepth;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignTextPoolReset(pNode);
	iSelectedDepth = 0;
	iItemCount = __uiDesignBuildCascaderItems(pNode, arrItems, UI_DESIGN_COUNT_OF(arrItems), arrSelectedPath, &iSelectedDepth);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = arrItems;
	tDesc.iItemCount = iItemCount;
	tDesc.arrSelectedValues = arrSelectedPath;
	tDesc.iSelectedDepth = iSelectedDepth;
	tDesc.bShowLastLevelOnly = !__uiDesignBool(pNode, "behavior.showAllLevels", 1);
	tDesc.bClearable = __uiDesignBool(pNode, "behavior.clearable", 1);
	tDesc.bSelectAnyLevel = __uiDesignBool(pNode, "behavior.selectAnyLevel", 0);
	tDesc.iExpandTrigger = __uiDesignInt(pNode, "behavior.expandTrigger", XUI_CASCADER_EXPAND_CLICK);
	tDesc.iPopupPlacement = __uiDesignInt(pNode, "behavior.popupPlacement", XUI_CASCADER_POPUP_AUTO);
	tDesc.sPlaceholder = __uiDesignText(pNode, "text.placeholder", "Select path");
	tDesc.sSeparator = __uiDesignText(pNode, "text.separator", "/");
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.fItemHeight = __uiDesignFloat(pNode, "metrics.itemHeight", 24.0f);
	tDesc.fColumnWidth = __uiDesignFloat(pNode, "metrics.columnWidth", 150.0f);
	tDesc.fPopupHeight = __uiDesignFloat(pNode, "metrics.popupHeight", 0.0f);
	tDesc.fPopupMaxHeight = __uiDesignFloat(pNode, "metrics.popupMaxHeight", 240.0f);
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iPlaceholderColor = __uiDesignColor(pNode, "appearance.placeholderColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iHoverBackgroundColor = __uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(240, 246, 253, 255));
	tDesc.iOpenBackgroundColor = __uiDesignColor(pNode, "appearance.openBackgroundColor", XUI_COLOR_RGBA(231, 241, 252, 255));
	tDesc.iDisabledBackgroundColor = __uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(238, 242, 247, 255));
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iHoverBorderColor = __uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(126, 164, 207, 255));
	tDesc.iFocusBorderColor = __uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iArrowColor = __uiDesignColor(pNode, "appearance.arrowColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iDisabledArrowColor = __uiDesignColor(pNode, "appearance.disabledArrowColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iButtonColor = __uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(246, 250, 253, 255));
	tDesc.iButtonHoverColor = __uiDesignColor(pNode, "appearance.buttonHoverColor", XUI_COLOR_RGBA(230, 246, 238, 255));
	tDesc.iButtonOpenColor = __uiDesignColor(pNode, "appearance.buttonOpenColor", XUI_COLOR_RGBA(220, 242, 232, 255));
	tDesc.iPopupPanelColor = __uiDesignColor(pNode, "appearance.popupPanelColor", XUI_COLOR_WHITE);
	tDesc.iPopupBorderColor = __uiDesignColor(pNode, "appearance.popupBorderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iPopupShadowColor = __uiDesignColor(pNode, "appearance.popupShadowColor", XUI_COLOR_RGBA(0, 0, 0, 46));
	tDesc.iPopupTextColor = __uiDesignColor(pNode, "appearance.popupTextColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iPopupMutedTextColor = __uiDesignColor(pNode, "appearance.popupMutedTextColor", XUI_COLOR_RGBA(107, 127, 149, 255));
	tDesc.iPopupHoverColor = __uiDesignColor(pNode, "appearance.popupHoverColor", XUI_COLOR_RGBA(240, 246, 253, 255));
	tDesc.iPopupActiveColor = __uiDesignColor(pNode, "appearance.popupActiveColor", XUI_COLOR_RGBA(231, 241, 252, 255));
	tDesc.iPopupSelectedColor = __uiDesignColor(pNode, "appearance.popupSelectedColor", XUI_COLOR_RGBA(238, 246, 255, 255));
	tDesc.iPopupActiveTextColor = __uiDesignColor(pNode, "appearance.popupActiveTextColor", XUI_COLOR_RGBA(31, 95, 168, 255));
	tDesc.iPopupDisabledTextColor = __uiDesignColor(pNode, "appearance.popupDisabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iPopupSeparatorColor = __uiDesignColor(pNode, "appearance.popupSeparatorColor", XUI_COLOR_RGBA(208, 219, 232, 255));
	tDesc.fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	return xuiCascaderCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateColorPicker(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_color_picker_desc_t tDesc;
	uint32_t arrPalette[XUI_COLOR_PICKER_PALETTE_CAPACITY];
	int iPaletteCount;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iPaletteCount = __uiDesignBuildColorPalette(pNode, arrPalette, UI_DESIGN_COUNT_OF(arrPalette));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iColor = __uiDesignColorPickerValue(pNode);
	tDesc.arrPalette = arrPalette;
	tDesc.iPaletteCount = iPaletteCount;
	tDesc.bAlphaEnabled = __uiDesignBool(pNode, "behavior.alphaEnabled", 1);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.fPopupWidth = __uiDesignFloat(pNode, "metrics.popupWidth", 260.0f);
	tDesc.fPopupHeight = __uiDesignFloat(pNode, "metrics.popupHeight", 250.0f);
	tDesc.iPopupPlacement = __uiDesignInt(pNode, "behavior.popupPlacement", XUI_COLOR_PICKER_POPUP_AUTO);
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iHoverBackgroundColor = __uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(240, 246, 253, 255));
	tDesc.iOpenBackgroundColor = __uiDesignColor(pNode, "appearance.openBackgroundColor", XUI_COLOR_RGBA(231, 241, 252, 255));
	tDesc.iDisabledBackgroundColor = __uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(238, 242, 247, 255));
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iHoverBorderColor = __uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(126, 164, 207, 255));
	tDesc.iFocusBorderColor = __uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iArrowColor = __uiDesignColor(pNode, "appearance.arrowColor", XUI_COLOR_RGBA(42, 92, 136, 255));
	tDesc.iDisabledArrowColor = __uiDesignColor(pNode, "appearance.disabledArrowColor", XUI_COLOR_RGBA(134, 148, 164, 180));
	tDesc.iButtonColor = __uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(236, 246, 253, 255));
	tDesc.iButtonHoverColor = __uiDesignColor(pNode, "appearance.buttonHoverColor", XUI_COLOR_RGBA(220, 238, 251, 255));
	tDesc.iButtonOpenColor = __uiDesignColor(pNode, "appearance.buttonOpenColor", XUI_COLOR_RGBA(207, 229, 247, 255));
	tDesc.iPopupPanelColor = __uiDesignColor(pNode, "appearance.popupPanelColor", XUI_COLOR_WHITE);
	tDesc.iPopupBorderColor = __uiDesignColor(pNode, "appearance.popupBorderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iPopupShadowColor = __uiDesignColor(pNode, "appearance.popupShadowColor", XUI_COLOR_RGBA(0, 0, 0, 46));
	tDesc.iPopupTextColor = __uiDesignColor(pNode, "appearance.popupTextColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iPopupMutedTextColor = __uiDesignColor(pNode, "appearance.popupMutedTextColor", XUI_COLOR_RGBA(107, 127, 149, 255));
	tDesc.iAccentColor = __uiDesignColor(pNode, "appearance.popupAccentColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iFieldColor = __uiDesignColor(pNode, "appearance.popupFieldColor", XUI_COLOR_WHITE);
	tDesc.iFieldBorderColor = __uiDesignColor(pNode, "appearance.popupFieldBorderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iSeparatorColor = __uiDesignColor(pNode, "appearance.popupSeparatorColor", XUI_COLOR_RGBA(208, 219, 232, 255));
	tDesc.fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	return xuiColorPickerCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateDatePicker(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_date_picker_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iMode = __uiDesignInt(pNode, "behavior.mode", XUI_DATE_PICKER_MODE_DATE);
	tDesc.bNullable = __uiDesignBool(pNode, "behavior.nullable", 1);
	tDesc.bShowSecond = __uiDesignBool(pNode, "behavior.showSecond", 0);
	tDesc.iFirstDayOfWeek = __uiDesignInt(pNode, "behavior.firstDayOfWeek", 1);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.sFormat = __uiDesignText(pNode, "text.format", "yyyy-mm-dd");
	tDesc.sRangeSeparator = __uiDesignText(pNode, "text.rangeSeparator", " - ");
	tDesc.fPopupWidth = __uiDesignFloat(pNode, "metrics.popupWidth", 280.0f);
	tDesc.fPopupHeight = __uiDesignFloat(pNode, "metrics.popupHeight", 300.0f);
	tDesc.iPopupPlacement = __uiDesignInt(pNode, "behavior.popupPlacement", XUI_DATE_PICKER_POPUP_AUTO);
	__uiDesignFillDatePickerValue(pNode, &tDesc);
	tDesc.iTextColor = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iDisabledTextColor = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	tDesc.iBackgroundColor = __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE);
	tDesc.iHoverBackgroundColor = __uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(240, 246, 253, 255));
	tDesc.iOpenBackgroundColor = __uiDesignColor(pNode, "appearance.openBackgroundColor", XUI_COLOR_RGBA(231, 241, 252, 255));
	tDesc.iDisabledBackgroundColor = __uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(238, 242, 247, 255));
	tDesc.iBorderColor = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iHoverBorderColor = __uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(126, 164, 207, 255));
	tDesc.iFocusBorderColor = __uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iArrowColor = __uiDesignColor(pNode, "appearance.arrowColor", XUI_COLOR_RGBA(60, 82, 104, 255));
	tDesc.iDisabledArrowColor = __uiDesignColor(pNode, "appearance.disabledArrowColor", XUI_COLOR_RGBA(150, 162, 174, 255));
	tDesc.iPopupPanelColor = __uiDesignColor(pNode, "appearance.popupPanelColor", XUI_COLOR_WHITE);
	tDesc.iPopupBorderColor = __uiDesignColor(pNode, "appearance.popupBorderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iPopupShadowColor = __uiDesignColor(pNode, "appearance.popupShadowColor", XUI_COLOR_RGBA(0, 0, 0, 46));
	tDesc.iPopupTextColor = __uiDesignColor(pNode, "appearance.popupTextColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	tDesc.iPopupMutedTextColor = __uiDesignColor(pNode, "appearance.popupMutedTextColor", XUI_COLOR_RGBA(107, 127, 149, 255));
	tDesc.iAccentColor = __uiDesignColor(pNode, "appearance.popupAccentColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	tDesc.iFieldColor = __uiDesignColor(pNode, "appearance.popupFieldColor", XUI_COLOR_WHITE);
	tDesc.iFieldBorderColor = __uiDesignColor(pNode, "appearance.popupFieldBorderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	tDesc.iSelectedTextColor = __uiDesignColor(pNode, "appearance.popupSelectedTextColor", XUI_COLOR_WHITE);
	tDesc.iDisabledDayColor = __uiDesignColor(pNode, "appearance.popupDisabledDayColor", XUI_COLOR_RGBA(183, 195, 208, 255));
	tDesc.iSeparatorColor = __uiDesignColor(pNode, "appearance.popupSeparatorColor", XUI_COLOR_RGBA(208, 219, 232, 255));
	tDesc.fBorderWidth = __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f);
	return xuiDatePickerCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateCodeEdit(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_code_edit_desc_t tDesc;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = __uiDesignResolveNodeFont(pApp, pNode);
	tDesc.sText = __uiDesignText(pNode, "data.text", "int main(void) {\n    return 0;\n}");
	tDesc.sLanguage = __uiDesignText(pNode, "text.language", "c");
	tDesc.bReadonly = __uiDesignBool(pNode, "behavior.readonly", 0);
	tDesc.bShowLineNumbers = __uiDesignBool(pNode, "behavior.lineNumbers", 1);
	tDesc.bShowFoldMargin = __uiDesignBool(pNode, "behavior.foldMargin", 1);
	tDesc.bShowMarkerMargin = __uiDesignBool(pNode, "behavior.markerMargin", 1);
	tDesc.bShowDiagnosticMargin = __uiDesignBool(pNode, "behavior.diagnosticMargin", 1);
	tDesc.bWordWrap = __uiDesignBool(pNode, "behavior.wordWrap", 0);
	tDesc.iEolMode = __uiDesignInt(pNode, "behavior.eolMode", XUI_CODE_EOL_LF);
	tDesc.iTabColumns = __uiDesignInt(pNode, "metrics.tabColumns", 4);
	tDesc.iIndentColumns = __uiDesignInt(pNode, "metrics.indentColumns", 4);
	tDesc.iFlags = (int)__uiDesignCodeEditOptions(pNode);
	return xuiCodeEditCreate(pApp->pContext, ppWidget, &tDesc);
}

static int __uiDesignCreateFlowGraph(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_flow_graph_desc_t tDesc;
	xui_flow_graph pGraph;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignBuildFlowGraph(pNode, &pGraph);
	if ( iRet != XUI_OK ) return iRet;
	__uiDesignFillFlowGraphDesc(pNode, &tDesc);
	tDesc.pGraph = pGraph;
	tDesc.bOwnGraph = 1;
	iRet = xuiFlowGraphWidgetCreate(pApp->pContext, ppWidget, &tDesc);
	if ( iRet != XUI_OK ) {
		xuiFlowGraphDestroy(pGraph);
		return iRet;
	}
	(void)__uiDesignApplyFlowGraphStyle(*ppWidget, pNode);
	return XUI_OK;
}

static int __uiDesignCreateWorkflow(struct ui_design_app_t* pApp, ui_design_node_t* pNode, xui_widget* ppWidget)
{
	xui_workflow_desc_t tDesc;
	xui_workflow pWorkflow;
	xui_widget pCanvas;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (ppWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignBuildWorkflow(pNode, &pWorkflow);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pWorkflow = pWorkflow;
	tDesc.bOwnWorkflow = 1;
	__uiDesignFillFlowGraphDesc(pNode, &tDesc.tGraph);
	iRet = xuiWorkflowWidgetCreate(pApp->pContext, ppWidget, &tDesc);
	if ( iRet != XUI_OK ) {
		xuiWorkflowDestroy(pWorkflow);
		return iRet;
	}
	pCanvas = xuiWorkflowWidgetGetCanvas(*ppWidget);
	if ( pCanvas != NULL ) (void)__uiDesignApplyFlowGraphStyle(pCanvas, pNode);
	return XUI_OK;
}

static int __uiDesignTableCount(xui_widget pWidget, void* pUser)
{
	ui_design_node_t* pNode;

	(void)pWidget;
	pNode = (ui_design_node_t*)pUser;
	if ( (pNode != NULL) && (pNode->iRuntimeTableRowCount > 0) ) return pNode->iRuntimeTableRowCount;
	return 3;
}

static int __uiDesignTableCell(xui_widget pWidget, int iRow, int iColumn, xui_table_view_cell_t* pCell, void* pUser)
{
	ui_design_node_t* pNode;
	int iTextIndex;

	(void)pWidget;
	pNode = (ui_design_node_t*)pUser;
	if ( pCell == NULL || iRow < 0 || iColumn < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pCell, 0, sizeof(*pCell));
	if ( (pNode != NULL) && (iRow < pNode->iRuntimeTableRowCount) && (iColumn < pNode->iRuntimeTableColumnCount) ) {
		iTextIndex = pNode->arrRuntimeTableText[iRow][iColumn];
		*pCell = pNode->arrRuntimeTableCells[iRow][iColumn];
		pCell->sText = __uiDesignTextPoolAt(pNode, iTextIndex);
		if ( !pNode->arrRuntimeTableCellTypeSet[iRow][iColumn] ) {
			pCell->iType = pNode->arrRuntimeTableColumnType[iColumn];
		}
		return XUI_OK;
	}
	if ( (iRow >= 3) || (iColumn >= 3) ) return XUI_ERROR_INVALID_ARGUMENT;
	pCell->sText = g_arrTableCellText[iRow][iColumn];
	pCell->iType = XUI_TABLE_CELL_TYPE_TEXT;
	return XUI_OK;
}

static int __uiDesignTableMergeProvider(xui_widget pWidget, int iRow, int iColumn, int* pRowSpan, int* pColSpan, void* pUser)
{
	ui_design_node_t* pNode;
	const char* sCursor;
	const char* sField;
	char sLine[256];
	char* arrFields[5];
	int iFieldCount;
	int iMergeRow;
	int iMergeColumn;
	int iRowSpan;
	int iColSpan;

	(void)pWidget;
	pNode = (ui_design_node_t*)pUser;
	if ( (pNode == NULL) || (iRow < 0) || (iColumn < 0) || (pRowSpan == NULL) || (pColSpan == NULL) ) return 0;
	sCursor = __uiDesignText(pNode, "data.merges", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( (iFieldCount < 4) || !__uiDesignLooksIntText(arrFields[0]) || !__uiDesignLooksIntText(arrFields[1]) ) continue;
		iMergeRow = __uiDesignParseIntText(arrFields[0], -1);
		iMergeColumn = __uiDesignParseIntText(arrFields[1], -1);
		if ( (iMergeRow != iRow) || (iMergeColumn != iColumn) ) continue;
		sField = __uiDesignField(arrFields, iFieldCount, 4, "true");
		if ( !__uiDesignParseBoolText(sField, 1) ) {
			*pRowSpan = 1;
			*pColSpan = 1;
			return 1;
		}
		iRowSpan = __uiDesignParseIntText(arrFields[2], 1);
		iColSpan = __uiDesignParseIntText(arrFields[3], 1);
		*pRowSpan = (iRowSpan > 0) ? iRowSpan : 1;
		*pColSpan = (iColSpan > 0) ? iColSpan : 1;
		return 1;
	}
	return 0;
}

static void __uiDesignTableGridSet(xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	ui_design_node_t* pNode;
	int iTextIndex;

	(void)pWidget;
	(void)iType;
	pNode = (ui_design_node_t*)pUser;
	if ( (pNode == NULL) || (iRow < 0) || (iColumn < 0) || (iRow >= UI_DESIGN_RUNTIME_TABLE_ROWS) || (iColumn >= UI_DESIGN_RUNTIME_TABLE_COLUMNS) ) return;
	iTextIndex = __uiDesignTextPoolAddIndex(pNode, sValue != NULL ? sValue : "");
	if ( iTextIndex <= 0 ) return;
	pNode->arrRuntimeTableText[iRow][iColumn] = iTextIndex;
	if ( iRow >= pNode->iRuntimeTableRowCount ) pNode->iRuntimeTableRowCount = iRow + 1;
	if ( iColumn >= pNode->iRuntimeTableColumnCount ) pNode->iRuntimeTableColumnCount = iColumn + 1;
}

static int __uiDesignTableGridBuildRuntimeEnumItems(ui_design_node_t* pNode, const char* sText, const char* sValues, const char* sEnabled)
{
	char arrItems[UI_DESIGN_RUNTIME_EDITOR_OPTIONS][64];
	char arrValues[UI_DESIGN_RUNTIME_EDITOR_OPTIONS][64];
	char arrEnabled[UI_DESIGN_RUNTIME_EDITOR_OPTIONS][64];
	int iCount;
	int iValueCount;
	int iEnabledCount;
	int i;

	if ( (pNode == NULL) || (sText == NULL) || (sText[0] == 0) ) return 0;
	memset(pNode->arrRuntimeEditorOptionText, 0, sizeof(pNode->arrRuntimeEditorOptionText));
	memset(pNode->arrRuntimeEditorOptions, 0, sizeof(pNode->arrRuntimeEditorOptions));
	memset(pNode->arrRuntimeEditorItemData, 0, sizeof(pNode->arrRuntimeEditorItemData));
	memset(pNode->arrRuntimeEditorEnabled, 0, sizeof(pNode->arrRuntimeEditorEnabled));
	iCount = __uiDesignSplitCommaList(sText, arrItems, UI_DESIGN_COUNT_OF(arrItems));
	iValueCount = __uiDesignSplitCommaList(sValues, arrValues, UI_DESIGN_COUNT_OF(arrValues));
	iEnabledCount = __uiDesignSplitCommaList(sEnabled, arrEnabled, UI_DESIGN_COUNT_OF(arrEnabled));
	for ( i = 0; i < iCount; ++i ) {
		__uiDesignCopyText(pNode->arrRuntimeEditorOptionText[i], sizeof(pNode->arrRuntimeEditorOptionText[i]), arrItems[i]);
		pNode->arrRuntimeEditorOptions[i] = pNode->arrRuntimeEditorOptionText[i];
		pNode->arrRuntimeEditorItemData[i].sText = pNode->arrRuntimeEditorOptionText[i];
		pNode->arrRuntimeEditorItemData[i].iValue = (i < iValueCount) ? __uiDesignParseIntText(arrValues[i], i) : i;
		pNode->arrRuntimeEditorItemData[i].bEnabled = (i < iEnabledCount) ? __uiDesignParseBoolText(arrEnabled[i], 1) : 1;
		pNode->arrRuntimeEditorEnabled[i] = pNode->arrRuntimeEditorItemData[i].bEnabled;
	}
	return iCount;
}

static int __uiDesignTableGridBuildRuntimePalette(ui_design_node_t* pNode, const char* sText)
{
	char sBuffer[1024];
	char* sCursor;
	char* sToken;
	int iCount;

	if ( (pNode == NULL) || (sText == NULL) || (sText[0] == 0) ) return 0;
	memset(pNode->arrRuntimeEditorPalette, 0, sizeof(pNode->arrRuntimeEditorPalette));
	__uiDesignCopyText(sBuffer, sizeof(sBuffer), sText);
	iCount = 0;
	sCursor = sBuffer;
	while ( sCursor[0] != 0 && iCount < UI_DESIGN_RUNTIME_EDITOR_PALETTE ) {
		sToken = sCursor;
		while ( sCursor[0] != 0 && sCursor[0] != ',' && sCursor[0] != ';' ) ++sCursor;
		if ( sCursor[0] != 0 ) {
			sCursor[0] = 0;
			++sCursor;
		}
		__uiDesignTrim(sToken);
		if ( sToken[0] == 0 ) continue;
		pNode->arrRuntimeEditorPalette[iCount++] = __uiDesignParseColorText(sToken, XUI_COLOR_RGBA(49, 126, 214, 255));
	}
	return iCount;
}

static int __uiDesignTableGridEditorConfig(xui_widget pWidget, int iRow, int iColumn, int iType, xui_table_grid_editor_config_t* pConfig, void* pUser)
{
	ui_design_node_t* pNode;
	const char* sCursor;
	const char* sField;
	char sLine[1024];
	char* arrFields[25];
	xtime tDate;
	int iFieldCount;
	int iConfigRow;
	int iConfigColumn;
	int iConfigType;
	int iOptionCount;
	int iPaletteCount;
	int bMatched;
	int bHasMin;
	int bHasMax;

	(void)pWidget;
	pNode = (ui_design_node_t*)pUser;
	if ( (pNode == NULL) || (pConfig == NULL) ) return 0;
	bMatched = 0;
	sCursor = __uiDesignText(pNode, "data.editorConfigs", "");
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 2 ) continue;
		iConfigRow = __uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 0, "-1"), -1);
		iConfigColumn = __uiDesignParseIntText(__uiDesignField(arrFields, iFieldCount, 1, "-1"), -1);
		if ( (iConfigRow >= 0 && iConfigRow != iRow) || (iConfigColumn >= 0 && iConfigColumn != iColumn) ) continue;
		sField = __uiDesignField(arrFields, iFieldCount, 2, "");
		if ( sField[0] != 0 ) {
			iConfigType = __uiDesignTableCellTypeFromText(sField, -1);
			if ( iConfigType != iType ) continue;
		}
		bMatched = 1;
		if ( iType == XUI_TABLE_CELL_TYPE_ENUM ) {
			sField = __uiDesignField(arrFields, iFieldCount, 3, "");
			iOptionCount = __uiDesignTableGridBuildRuntimeEnumItems(pNode, sField,
				__uiDesignField(arrFields, iFieldCount, 20, ""),
				__uiDesignField(arrFields, iFieldCount, 21, ""));
			if ( iOptionCount > 0 ) {
				pConfig->arrEnumItems = pNode->arrRuntimeEditorOptions;
				pConfig->arrEnumItemData = pNode->arrRuntimeEditorItemData;
				pConfig->arrEnumEnabled = pNode->arrRuntimeEditorEnabled;
				pConfig->iEnumItemCount = iOptionCount;
				pConfig->iEnumSelected = -1;
				sField = __uiDesignField(arrFields, iFieldCount, 22, "");
				if ( sField[0] != 0 ) pConfig->iEnumSelected = __uiDesignParseIntText(sField, -1);
				sField = __uiDesignField(arrFields, iFieldCount, 23, "");
				if ( sField[0] != 0 ) pConfig->bEnumUseValue = __uiDesignParseBoolText(sField, 0);
				sField = __uiDesignField(arrFields, iFieldCount, 24, "");
				if ( sField[0] != 0 ) pConfig->iEnumSelectedValue = __uiDesignParseIntText(sField, 0);
			}
		}
		if ( (iType == XUI_TABLE_CELL_TYPE_INT) || (iType == XUI_TABLE_CELL_TYPE_FLOAT) ) {
			bHasMin = 0;
			bHasMax = 0;
			sField = __uiDesignField(arrFields, iFieldCount, 4, "");
			if ( sField[0] != 0 ) {
				pConfig->fMin = __uiDesignParseFloatText(sField, pConfig->fMin);
				bHasMin = 1;
			}
			sField = __uiDesignField(arrFields, iFieldCount, 5, "");
			if ( sField[0] != 0 ) {
				pConfig->fMax = __uiDesignParseFloatText(sField, pConfig->fMax);
				bHasMax = 1;
			}
			if ( bHasMin && !bHasMax ) pConfig->fMax = 1000000000.0f;
			if ( bHasMax && !bHasMin ) pConfig->fMin = -1000000000.0f;
			sField = __uiDesignField(arrFields, iFieldCount, 6, "");
			if ( sField[0] != 0 ) pConfig->fStep = __uiDesignParseFloatText(sField, pConfig->fStep);
			sField = __uiDesignField(arrFields, iFieldCount, 7, "");
			if ( sField[0] != 0 ) pConfig->iPrecision = __uiDesignParseIntText(sField, pConfig->iPrecision);
		}
		sField = __uiDesignField(arrFields, iFieldCount, 8, "");
		if ( sField[0] != 0 ) pConfig->bNullable = __uiDesignParseBoolText(sField, pConfig->bNullable);
		if ( iType == XUI_TABLE_CELL_TYPE_COLOR ) {
			sField = __uiDesignField(arrFields, iFieldCount, 9, "");
			if ( sField[0] != 0 ) pConfig->bAlphaEnabled = __uiDesignParseBoolText(sField, pConfig->bAlphaEnabled);
			sField = __uiDesignField(arrFields, iFieldCount, 19, "");
			iPaletteCount = __uiDesignTableGridBuildRuntimePalette(pNode, sField);
			if ( iPaletteCount > 0 ) {
				pConfig->arrPalette = pNode->arrRuntimeEditorPalette;
				pConfig->iPaletteCount = iPaletteCount;
			}
		}
		if ( iType == XUI_TABLE_CELL_TYPE_FILE || iType == XUI_TABLE_CELL_TYPE_IMAGE || iType == XUI_TABLE_CELL_TYPE_PICKER || iType == XUI_TABLE_CELL_TYPE_CUSTOM ) {
			sField = __uiDesignField(arrFields, iFieldCount, 10, "");
			if ( sField[0] != 0 ) pConfig->sActionText = __uiDesignTextPoolAdd(pNode, sField);
			sField = __uiDesignField(arrFields, iFieldCount, 11, "");
			if ( sField[0] != 0 ) pConfig->sFileFilter = __uiDesignTextPoolAdd(pNode, sField);
		}
		if ( iType == XUI_TABLE_CELL_TYPE_DATE || iType == XUI_TABLE_CELL_TYPE_TIME || iType == XUI_TABLE_CELL_TYPE_DATETIME ) {
			sField = __uiDesignField(arrFields, iFieldCount, 12, "");
			if ( sField[0] != 0 ) {
				pConfig->iDateMode = __uiDesignPropertyGridDateModeFromText(sField, pConfig->iDateMode);
				pConfig->bDateModeSet = 1;
			}
			sField = __uiDesignField(arrFields, iFieldCount, 13, "");
			if ( sField[0] != 0 ) pConfig->bShowSecond = __uiDesignParseBoolText(sField, pConfig->bShowSecond);
			sField = __uiDesignField(arrFields, iFieldCount, 14, "");
			if ( sField[0] != 0 ) pConfig->sDateFormat = __uiDesignTextPoolAdd(pNode, sField);
			sField = __uiDesignField(arrFields, iFieldCount, 15, "");
			if ( sField[0] != 0 && __uiDesignParseDatePickerValue(sField, pConfig->iDateMode, &tDate) ) {
				pConfig->tDateMin = tDate;
				pConfig->bDateHasMin = 1;
			}
			sField = __uiDesignField(arrFields, iFieldCount, 16, "");
			if ( sField[0] != 0 && __uiDesignParseDatePickerValue(sField, pConfig->iDateMode, &tDate) ) {
				pConfig->tDateMax = tDate;
				pConfig->bDateHasMax = 1;
			}
			sField = __uiDesignField(arrFields, iFieldCount, 17, "");
			if ( sField[0] != 0 ) pConfig->tDefaultRangeSpan = __uiDesignParseDatePickerSpan(sField, pConfig->tDefaultRangeSpan);
			sField = __uiDesignField(arrFields, iFieldCount, 18, "");
			if ( sField[0] != 0 ) pConfig->sRangeSeparator = __uiDesignTextPoolAdd(pNode, sField);
		}
	}
	return bMatched;
}

int uiDesignRegistryResolveTableGridEditorConfig(ui_design_node_t* pNode, xui_widget pWidget, int iRow, int iColumn, int iType, xui_table_grid_editor_config_t* pConfig)
{
	if ( (pNode == NULL) || (pConfig == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pConfig, 0, sizeof(*pConfig));
	pConfig->iEnumSelected = -1;
	pConfig->iDateMode = XUI_DATE_PICKER_MODE_DATE;
	pConfig->bShowSecond = 1;
	pConfig->iPrecision = -1;
	return __uiDesignTableGridEditorConfig(pWidget, iRow, iColumn, iType, pConfig, pNode) ? XUI_OK : XUI_ERROR;
}

static void __uiDesignApplyGenericLayout(ui_design_node_t* pNode)
{
	xui_widget pWidget;
	xui_thickness_t tMargin;
	xui_thickness_t tPadding;
	xui_vec2_t tSize;
	xui_table_track_t tTrack;
	const char* sText;
	char sClass[64];
	float fA;
	float fB;
	int iA;
	int iB;
	int iC;
	int iD;
	int iLen;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) ) return;
	pWidget = pNode->pWidget;
	(void)xuiWidgetSetLayoutType(pWidget, __uiDesignInt(pNode, "layout.type", XUI_LAYOUT_MANUAL));
	(void)xuiWidgetSetSizeMode(pWidget, __uiDesignInt(pNode, "layout.widthMode", XUI_SIZE_FIXED), __uiDesignInt(pNode, "layout.heightMode", XUI_SIZE_FIXED));
	(void)xuiWidgetSetFlowMode(pWidget, __uiDesignInt(pNode, "layout.flowMode", XUI_FLOW_BLOCK));
	(void)xuiWidgetSetDock(pWidget, __uiDesignInt(pNode, "layout.dock", 0));
	(void)xuiWidgetSetOverflow(pWidget, __uiDesignInt(pNode, "layout.overflow", XUI_OVERFLOW_VISIBLE));
	(void)xuiWidgetSetAlign(pWidget, __uiDesignInt(pNode, "layout.alignX", XUI_ALIGN_START), __uiDesignInt(pNode, "layout.alignY", XUI_ALIGN_START));
	(void)xuiWidgetSetFlex(pWidget, __uiDesignFloat(pNode, "layout.flexGrow", 0.0f), __uiDesignFloat(pNode, "layout.flexShrink", 1.0f));
	(void)xuiWidgetSetGap(pWidget, __uiDesignFloat(pNode, "layout.gap", 0.0f));
	tMargin = (xui_thickness_t){
		__uiDesignFloat(pNode, "layout.marginLeft", 0.0f),
		__uiDesignFloat(pNode, "layout.marginTop", 0.0f),
		__uiDesignFloat(pNode, "layout.marginRight", 0.0f),
		__uiDesignFloat(pNode, "layout.marginBottom", 0.0f)
	};
	tPadding = (xui_thickness_t){
		__uiDesignFloat(pNode, "layout.paddingLeft", 0.0f),
		__uiDesignFloat(pNode, "layout.paddingTop", 0.0f),
		__uiDesignFloat(pNode, "layout.paddingRight", 0.0f),
		__uiDesignFloat(pNode, "layout.paddingBottom", 0.0f)
	};
	(void)xuiWidgetSetMargin(pWidget, tMargin);
	(void)xuiWidgetSetPadding(pWidget, tPadding);
	(void)xuiWidgetSetTableSize(pWidget, __uiDesignInt(pNode, "layout.tableRows", 1), __uiDesignInt(pNode, "layout.tableColumns", 1));
	if ( __uiDesignTryInt(pNode, "layout.gridColumns", &iA) ||
	     __uiDesignTryFloat(pNode, "layout.gridItemWidth", &fA) ||
	     __uiDesignTryFloat(pNode, "layout.gridItemHeight", &fB) ) {
		(void)xuiWidgetSetGridMetrics(pWidget,
			__uiDesignInt(pNode, "layout.gridColumns", 1),
			__uiDesignFloat(pNode, "layout.gridItemWidth", 0.0f),
			__uiDesignFloat(pNode, "layout.gridItemHeight", 0.0f));
	}
	if ( __uiDesignTryInt(pNode, "layout.layer", &iA) || __uiDesignTryInt(pNode, "layout.zIndex", &iB) ) {
		(void)xuiWidgetSetLayer(pWidget, __uiDesignInt(pNode, "layout.layer", XUI_LAYER_NORMAL), __uiDesignInt(pNode, "layout.zIndex", 0));
	}
	if ( __uiDesignTryFloat(pNode, "layout.preferredWidth", &fA) || __uiDesignTryFloat(pNode, "layout.preferredHeight", &fB) ) {
		tSize = (xui_vec2_t){__uiDesignFloat(pNode, "layout.preferredWidth", 0.0f), __uiDesignFloat(pNode, "layout.preferredHeight", 0.0f)};
		(void)xuiWidgetSetPreferredSize(pWidget, tSize);
	}
	if ( __uiDesignTryFloat(pNode, "layout.minWidth", &fA) || __uiDesignTryFloat(pNode, "layout.minHeight", &fB) ) {
		tSize = (xui_vec2_t){__uiDesignFloat(pNode, "layout.minWidth", 0.0f), __uiDesignFloat(pNode, "layout.minHeight", 0.0f)};
		(void)xuiWidgetSetMinSize(pWidget, tSize);
	}
	if ( __uiDesignTryFloat(pNode, "layout.maxWidth", &fA) || __uiDesignTryFloat(pNode, "layout.maxHeight", &fB) ) {
		fA = __uiDesignFloat(pNode, "layout.maxWidth", 0.0f);
		fB = __uiDesignFloat(pNode, "layout.maxHeight", 0.0f);
		tSize = (xui_vec2_t){fA > 0.0f ? fA : XUI_LAYOUT_UNBOUNDED, fB > 0.0f ? fB : XUI_LAYOUT_UNBOUNDED};
		(void)xuiWidgetSetMaxSize(pWidget, tSize);
	}
	if ( __uiDesignTryInt(pNode, "layout.tableCellRow", &iA) ||
	     __uiDesignTryInt(pNode, "layout.tableCellColumn", &iB) ||
	     __uiDesignTryInt(pNode, "layout.tableCellRowSpan", &iC) ||
	     __uiDesignTryInt(pNode, "layout.tableCellColumnSpan", &iD) ) {
		(void)xuiWidgetSetTableCell(pWidget,
			__uiDesignInt(pNode, "layout.tableCellRow", 0),
			__uiDesignInt(pNode, "layout.tableCellColumn", 0),
			__uiDesignInt(pNode, "layout.tableCellRowSpan", 1),
			__uiDesignInt(pNode, "layout.tableCellColumnSpan", 1));
	}
	if ( __uiDesignHasProperty(pNode, "layout.tableRowSizeMode") || __uiDesignHasProperty(pNode, "layout.tableRowValue") ) {
		tTrack = __uiDesignTableTrackFromValue(
			__uiDesignInt(pNode, "layout.tableRowSizeMode", XUI_SIZE_FIXED),
			__uiDesignFloat(pNode, "layout.tableRowValue", 0.0f));
		(void)xuiWidgetSetTableRow(pWidget, __uiDesignInt(pNode, "layout.tableTrackRow", __uiDesignInt(pNode, "layout.tableCellRow", 0)), &tTrack);
	}
	if ( __uiDesignHasProperty(pNode, "layout.tableColumnSizeMode") || __uiDesignHasProperty(pNode, "layout.tableColumnValue") ) {
		tTrack = __uiDesignTableTrackFromValue(
			__uiDesignInt(pNode, "layout.tableColumnSizeMode", XUI_SIZE_FIXED),
			__uiDesignFloat(pNode, "layout.tableColumnValue", 0.0f));
		(void)xuiWidgetSetTableColumn(pWidget, __uiDesignInt(pNode, "layout.tableTrackColumn", __uiDesignInt(pNode, "layout.tableCellColumn", 0)), &tTrack);
	}
	__uiDesignApplyTableTrackConfig(pNode, pWidget, "layout.tableRowsConfig", 1);
	__uiDesignApplyTableTrackConfig(pNode, pWidget, "layout.tableColumnsConfig", 0);
	if ( __uiDesignTryBool(pNode, "common.hitTestVisible", &iA) ) (void)xuiWidgetSetHitTestVisible(pWidget, iA);
	if ( __uiDesignTryBool(pNode, "common.focusable", &iA) ) (void)xuiWidgetSetFocusable(pWidget, iA);
	if ( __uiDesignTryBool(pNode, "common.tabStop", &iA) ) (void)xuiWidgetSetTabStop(pWidget, iA);
	if ( __uiDesignTryInt(pNode, "common.tabIndex", &iA) ) (void)xuiWidgetSetTabIndex(pWidget, iA);
	if ( __uiDesignTryBool(pNode, "common.focusScope", &iA) ) (void)xuiWidgetSetFocusScope(pWidget, iA);
	if ( __uiDesignTryBool(pNode, "common.dragEnabled", &iA) ) (void)xuiWidgetSetDragEnabled(pWidget, iA);
	if ( __uiDesignTryInt(pNode, "common.imeMode", &iA) ) (void)xuiWidgetSetImeMode(pWidget, iA);
	__uiDesignApplyTooltip(pNode, pWidget);
	if ( __uiDesignHasProperty(pNode, "style.name") ) {
		(void)xuiWidgetSetStyleName(pWidget, __uiDesignText(pNode, "style.name", ""));
	}
	if ( __uiDesignTryInt(pNode, "style.stateId", &iA) ) {
		(void)xuiWidgetSetStateId(pWidget, (uint32_t)iA);
	}
	if ( __uiDesignHasProperty(pNode, "style.classes") ) {
		(void)xuiWidgetClearStyleClasses(pWidget);
		sText = __uiDesignText(pNode, "style.classes", "");
		while ( sText != NULL && *sText != '\0' ) {
			while ( *sText == ' ' || *sText == '\t' || *sText == '\r' || *sText == '\n' || *sText == ',' || *sText == ';' ) sText++;
			iLen = 0;
			while ( sText[iLen] != '\0' && sText[iLen] != ' ' && sText[iLen] != '\t' && sText[iLen] != '\r' && sText[iLen] != '\n' && sText[iLen] != ',' && sText[iLen] != ';' ) iLen++;
			if ( iLen <= 0 ) break;
			if ( iLen >= (int)sizeof(sClass) ) iLen = (int)sizeof(sClass) - 1;
			memcpy(sClass, sText, (size_t)iLen);
			sClass[iLen] = '\0';
			(void)xuiWidgetAddStyleClass(pWidget, sClass);
			sText += iLen;
		}
	}
	__uiDesignApplyInlineStyle(pNode, pWidget);
	__uiDesignApplyWidgetCache(pNode, pWidget);
}

static void __uiDesignReadTwoStateSourceRects(const ui_design_node_t* pNode, xui_rect_t* pUncheckedSrc, xui_rect_t* pCheckedSrc)
{
	float fX;
	float fY;
	float fW;
	float fH;

	if ( pUncheckedSrc != NULL ) memset(pUncheckedSrc, 0, sizeof(*pUncheckedSrc));
	if ( pCheckedSrc != NULL ) memset(pCheckedSrc, 0, sizeof(*pCheckedSrc));
	if ( pNode == NULL ) return;
	if ( pUncheckedSrc != NULL ) {
		fX = __uiDesignFloat(pNode, "unchecked.x", 0.0f);
		fY = __uiDesignFloat(pNode, "unchecked.y", 0.0f);
		fW = __uiDesignFloat(pNode, "unchecked.w", 0.0f);
		fH = __uiDesignFloat(pNode, "unchecked.h", 0.0f);
		if ( fW > 0.0f && fH > 0.0f ) {
			pUncheckedSrc->fX = fX;
			pUncheckedSrc->fY = fY;
			pUncheckedSrc->fW = fW;
			pUncheckedSrc->fH = fH;
		}
	}
	if ( pCheckedSrc != NULL ) {
		fX = __uiDesignFloat(pNode, "checked.x", 0.0f);
		fY = __uiDesignFloat(pNode, "checked.y", 0.0f);
		fW = __uiDesignFloat(pNode, "checked.w", 0.0f);
		fH = __uiDesignFloat(pNode, "checked.h", 0.0f);
		if ( fW > 0.0f && fH > 0.0f ) {
			pCheckedSrc->fX = fX;
			pCheckedSrc->fY = fY;
			pCheckedSrc->fW = fW;
			pCheckedSrc->fH = fH;
		}
	}
}

static void __uiDesignApplyCheckProperties(struct ui_design_app_t* pApp, ui_design_node_t* pNode, int bRadio)
{
	xui_widget pWidget;
	xui_surface pSurface;
	xui_rect_t tUncheckedSrc;
	xui_rect_t tCheckedSrc;
	uint32_t iText;
	uint32_t iDisabledText;
	uint32_t iAccent;
	uint32_t iBorder;
	uint32_t iHoverBorder;
	uint32_t iFocus;
	float fIndicator;
	float fGap;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pNode->pWidget == NULL) ) return;
	pWidget = pNode->pWidget;
	iText = __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255));
	iDisabledText = __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255));
	iAccent = __uiDesignColor(pNode, "appearance.accentColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	iBorder = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(141, 165, 192, 255));
	iHoverBorder = __uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	iFocus = __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255));
	fIndicator = __uiDesignFloat(pNode, "metrics.indicatorSize", 16.0f);
	fGap = __uiDesignFloat(pNode, "metrics.gap", 7.0f);
	iRet = __uiDesignLoadRuntimeSurface(pApp, pNode, __uiDesignText(pNode, "data.indicatorSource", ""), &pSurface);
	if ( iRet != XUI_OK ) pSurface = NULL;
	__uiDesignReadTwoStateSourceRects((pSurface != NULL) ? pNode : NULL, &tUncheckedSrc, &tCheckedSrc);
	if ( bRadio ) {
		(void)xuiRadioSetText(pWidget, pNode->sText);
		(void)xuiRadioSetChecked(pWidget, pNode->bChecked);
		(void)xuiRadioSetTextColor(pWidget, iText);
		(void)xuiRadioSetDisabledTextColor(pWidget, iDisabledText);
		(void)xuiRadioSetTextFlags(pWidget, (uint32_t)__uiDesignInt(pNode, "text.flags", XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP));
		(void)xuiRadioSetIndicatorSize(pWidget, fIndicator);
		(void)xuiRadioSetGap(pWidget, fGap);
		(void)xuiRadioSetColors(pWidget, iAccent, iBorder, iHoverBorder, iFocus);
		(void)xuiRadioSetIndicatorSurface(pWidget, pSurface, tUncheckedSrc, pSurface, tCheckedSrc);
		(void)xuiRadioUseBuiltinAtlas(pWidget, __uiDesignBool(pNode, "behavior.useBuiltinAtlas", 1));
	} else {
		(void)xuiCheckBoxSetText(pWidget, pNode->sText);
		(void)xuiCheckBoxSetChecked(pWidget, pNode->bChecked);
		(void)xuiCheckBoxSetTextColor(pWidget, iText);
		(void)xuiCheckBoxSetDisabledTextColor(pWidget, iDisabledText);
		(void)xuiCheckBoxSetTextFlags(pWidget, (uint32_t)__uiDesignInt(pNode, "text.flags", XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP));
		(void)xuiCheckBoxSetIndicatorSize(pWidget, fIndicator);
		(void)xuiCheckBoxSetGap(pWidget, fGap);
		(void)xuiCheckBoxSetColors(pWidget, iAccent, iBorder, iHoverBorder, iFocus);
		(void)xuiCheckBoxSetIndicatorSurface(pWidget, pSurface, tUncheckedSrc, pSurface, tCheckedSrc);
		(void)xuiCheckBoxUseBuiltinAtlas(pWidget, __uiDesignBool(pNode, "behavior.useBuiltinAtlas", 1));
	}
}

static void __uiDesignApplyToggleIndicatorSurface(struct ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	xui_surface pSurface;
	xui_rect_t tUncheckedSrc;
	xui_rect_t tCheckedSrc;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (pNode->pWidget == NULL) ) return;
	iRet = __uiDesignLoadRuntimeSurface(pApp, pNode, __uiDesignText(pNode, "data.indicatorSource", ""), &pSurface);
	if ( iRet != XUI_OK ) pSurface = NULL;
	__uiDesignReadTwoStateSourceRects((pSurface != NULL) ? pNode : NULL, &tUncheckedSrc, &tCheckedSrc);
	(void)xuiToggleSetIndicatorSurface(pNode->pWidget, pSurface, tUncheckedSrc, pSurface, tCheckedSrc);
}

static void __uiDesignApplyTreeDefaults(ui_design_node_t* pNode)
{
	xui_tree_view_node_t arrNodes[UI_DESIGN_RUNTIME_TABLE_ROWS];
	int iCount;

	__uiDesignTextPoolReset(pNode);
	iCount = __uiDesignBuildTreeNodes(pNode, arrNodes, UI_DESIGN_COUNT_OF(arrNodes));
	(void)xuiTreeViewSetNodes(pNode->pWidget, arrNodes, iCount);
}

static void __uiDesignApplyTableDefaults(ui_design_node_t* pNode)
{
	xui_table_view_column_t arrColumns[UI_DESIGN_RUNTIME_TABLE_COLUMNS];
	xui_table_view_row_t arrRows[UI_DESIGN_RUNTIME_TABLE_ROWS];
	int iColumnCount;
	int iRowCount;

	(void)__uiDesignBuildTableData(pNode, arrColumns, &iColumnCount, arrRows, &iRowCount, 0);
	(void)xuiTableViewSetColumns(pNode->pWidget, arrColumns, iColumnCount);
	(void)xuiTableViewSetRows(pNode->pWidget, arrRows, iRowCount);
	(void)xuiTableViewSetAdapter(pNode->pWidget, __uiDesignTableCount, __uiDesignTableCell, pNode);
	(void)xuiTableViewSetMergeProvider(pNode->pWidget, __uiDesignTableMergeProvider, pNode);
}

static void __uiDesignApplyTableGridDefaults(ui_design_node_t* pNode)
{
	xui_table_view_column_t arrColumns[UI_DESIGN_RUNTIME_TABLE_COLUMNS];
	xui_table_view_row_t arrRows[UI_DESIGN_RUNTIME_TABLE_ROWS];
	xui_widget pTableView;
	int iColumnCount;
	int iRowCount;

	(void)__uiDesignBuildTableData(pNode, arrColumns, &iColumnCount, arrRows, &iRowCount, 1);
	(void)xuiTableGridSetColumns(pNode->pWidget, arrColumns, iColumnCount);
	(void)xuiTableGridSetRows(pNode->pWidget, arrRows, iRowCount);
	(void)xuiTableGridSetAdapter(pNode->pWidget, __uiDesignTableCount, __uiDesignTableCell, __uiDesignTableGridSet, pNode);
	pTableView = xuiTableGridGetTableView(pNode->pWidget);
	if ( pTableView != NULL ) {
		(void)xuiTableViewSetMergeProvider(pTableView, __uiDesignTableMergeProvider, pNode);
	}
}

static void __uiDesignApplyInventoryGamepadProfile(ui_design_node_t* pNode)
{
	xui_inventory_gamepad_profile_t tProfile;
	uint32_t iFlags;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) ) return;
	iFlags = 0u;
	if ( __uiDesignBool(pNode, "gamepad.wrapRows", 0) ) iFlags |= XUI_INVENTORY_GAMEPAD_WRAP_ROWS;
	if ( __uiDesignBool(pNode, "gamepad.wrapColumns", 0) ) iFlags |= XUI_INVENTORY_GAMEPAD_WRAP_COLUMNS;
	if ( __uiDesignBool(pNode, "gamepad.skipDisabled", 1) ) iFlags |= XUI_INVENTORY_GAMEPAD_SKIP_DISABLED;
	if ( __uiDesignBool(pNode, "gamepad.selectOnMove", 1) ) iFlags |= XUI_INVENTORY_GAMEPAD_SELECT_ON_MOVE;
	if ( __uiDesignBool(pNode, "gamepad.multiSelectModifiers", 0) ) iFlags |= XUI_INVENTORY_GAMEPAD_MULTI_SELECT_MODIFIERS;
	memset(&tProfile, 0, sizeof(tProfile));
	tProfile.iSize = sizeof(tProfile);
	tProfile.iFlags = iFlags;
	tProfile.iAcceptButton = __uiDesignInt(pNode, "gamepad.acceptButton", XUI_INVENTORY_GAMEPAD_BUTTON_ACCEPT);
	tProfile.iCancelButton = __uiDesignInt(pNode, "gamepad.cancelButton", XUI_INVENTORY_GAMEPAD_BUTTON_CANCEL);
	tProfile.iContextButton = __uiDesignInt(pNode, "gamepad.contextButton", XUI_INVENTORY_GAMEPAD_BUTTON_CONTEXT);
	tProfile.iLeftButton = __uiDesignInt(pNode, "gamepad.leftButton", XUI_INVENTORY_GAMEPAD_BUTTON_LEFT);
	tProfile.iRightButton = __uiDesignInt(pNode, "gamepad.rightButton", XUI_INVENTORY_GAMEPAD_BUTTON_RIGHT);
	tProfile.iUpButton = __uiDesignInt(pNode, "gamepad.upButton", XUI_INVENTORY_GAMEPAD_BUTTON_UP);
	tProfile.iDownButton = __uiDesignInt(pNode, "gamepad.downButton", XUI_INVENTORY_GAMEPAD_BUTTON_DOWN);
	tProfile.iPageUpButton = __uiDesignInt(pNode, "gamepad.pageUpButton", XUI_INVENTORY_GAMEPAD_BUTTON_PAGE_UP);
	tProfile.iPageDownButton = __uiDesignInt(pNode, "gamepad.pageDownButton", XUI_INVENTORY_GAMEPAD_BUTTON_PAGE_DOWN);
	tProfile.iHomeButton = __uiDesignInt(pNode, "gamepad.homeButton", XUI_INVENTORY_GAMEPAD_BUTTON_HOME);
	tProfile.iEndButton = __uiDesignInt(pNode, "gamepad.endButton", XUI_INVENTORY_GAMEPAD_BUTTON_END);
	(void)xuiInventoryGridSetGamepadProfile(pNode->pWidget, &tProfile);
}

static void __uiDesignApplyInventorySelection(ui_design_node_t* pNode)
{
	const char* sCursor;
	char sLine[128];
	char* arrFields[2];
	int iFieldCount;
	int iSlot;
	int iCurrent;
	int iSelectionMode;
	int bHasExplicit;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) ) return;
	iCurrent = __uiDesignInt(pNode, "data.current", 0);
	iSelectionMode = __uiDesignInt(pNode, "behavior.selectionMode", XUI_INVENTORY_SELECTION_SINGLE);
	sCursor = __uiDesignText(pNode, "data.selectedSlots", "");
	bHasExplicit = 0;
	while ( __uiDesignNextLine(&sCursor, sLine, sizeof(sLine)) ) {
		if ( sLine[0] == 0 ) continue;
		if ( !bHasExplicit ) {
			(void)xuiInventoryGridClearSelection(pNode->pWidget);
			bHasExplicit = 1;
		}
		iFieldCount = __uiDesignSplitFields(sLine, arrFields, UI_DESIGN_COUNT_OF(arrFields));
		if ( iFieldCount < 1 || arrFields[0][0] == 0 ) continue;
		iSlot = __uiDesignParseIntText(arrFields[0], -1);
		if ( iSlot < 0 ) continue;
		(void)xuiInventoryGridSetSelected(pNode->pWidget, iSlot, __uiDesignParseBoolText(__uiDesignField(arrFields, iFieldCount, 1, "true"), 1), 0);
	}
	if ( !bHasExplicit ) {
		(void)xuiInventoryGridClearSelection(pNode->pWidget);
		if ( iSelectionMode == XUI_INVENTORY_SELECTION_NONE ) {
			(void)xuiInventoryGridSetCurrent(pNode->pWidget, iCurrent, 0);
		} else {
			(void)xuiInventoryGridSetSelected(pNode->pWidget, iCurrent, 1, 0);
		}
	} else {
		(void)xuiInventoryGridSetCurrent(pNode->pWidget, iCurrent, 0);
	}
}

static void __uiDesignApplyInventorySplitPreview(ui_design_node_t* pNode)
{
	if ( (pNode == NULL) || (pNode->pWidget == NULL) ) return;
	if ( __uiDesignBool(pNode, "behavior.previewSplitOpen", 0) ) {
		(void)xuiInventoryGridOpenSplitPopup(pNode->pWidget,
			__uiDesignInt(pNode, "value.previewSplitSlot", 1),
			__uiDesignFloat(pNode, "value.previewSplitX", 0.0f),
			__uiDesignFloat(pNode, "value.previewSplitY", 0.0f));
	} else {
		(void)xuiInventoryGridCloseSplitPopup(pNode->pWidget);
	}
}

static void __uiDesignApplyInventoryDefaults(ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	xui_inventory_grid_layout_t tLayout;
	xui_inventory_grid_colors_t tColors;
	xui_inventory_slot_t arrSlots[UI_DESIGN_RUNTIME_TABLE_ROWS];
	int iSlotCount;
	int iSlotDataCount;
	int i;

	iSlotDataCount = __uiDesignBuildInventorySlots(pApp, pNode, arrSlots, UI_DESIGN_COUNT_OF(arrSlots));
	iSlotCount = __uiDesignInt(pNode, "data.slotCount", 24);
	if ( iSlotCount < iSlotDataCount ) iSlotCount = iSlotDataCount;
	(void)xuiInventoryGridSetSlotCount(pNode->pWidget, iSlotCount);
	(void)xuiInventoryGridClearAll(pNode->pWidget);
	if ( iSlotDataCount == 0 ) {
		for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrDefaultInventorySlots) && i < iSlotCount; i++ ) {
			(void)xuiInventoryGridSetSlot(pNode->pWidget, i, &g_arrDefaultInventorySlots[i]);
		}
	} else {
		for ( i = 0; i < iSlotDataCount && i < iSlotCount; i++ ) {
			(void)xuiInventoryGridSetSlot(pNode->pWidget, i, &arrSlots[i]);
		}
	}
	__uiDesignFillInventoryLayout(pNode, &tLayout);
	__uiDesignFillInventoryColors(pNode, &tColors);
	(void)xuiInventoryGridSetLayout(pNode->pWidget, &tLayout);
	(void)xuiInventoryGridSetColors(pNode->pWidget, &tColors);
	__uiDesignApplyInventorySelection(pNode);
	(void)xuiInventoryGridSetTooltipVisible(pNode->pWidget, __uiDesignBool(pNode, "behavior.tooltipVisible", 1));
	__uiDesignApplyInventoryGamepadProfile(pNode);
	__uiDesignApplyInventorySplitPreview(pNode);
}

static void __uiDesignApplyScrollFrame(ui_design_node_t* pNode)
{
	xui_widget pWidget;

	pWidget = pNode->pWidget;
	(void)xuiScrollFrameSetContentSize(pWidget, __uiDesignFloat(pNode, "metrics.contentWidth", 520.0f), __uiDesignFloat(pNode, "metrics.contentHeight", 360.0f));
	(void)xuiScrollFrameSetOffset(pWidget, __uiDesignFloat(pNode, "value.offsetX", 0.0f), __uiDesignFloat(pNode, "value.offsetY", 0.0f));
	(void)xuiScrollFrameSetScrollbarPolicy(pWidget, __uiDesignInt(pNode, "behavior.policyX", XUI_SCROLLBAR_POLICY_AUTO), __uiDesignInt(pNode, "behavior.policyY", XUI_SCROLLBAR_POLICY_AUTO));
	(void)xuiScrollFrameSetScrollbarMode(pWidget, __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL));
	(void)xuiScrollFrameSetWheelAxis(pWidget, __uiDesignInt(pNode, "behavior.wheelAxis", XUI_WHEEL_AXIS_VERTICAL));
	(void)xuiScrollFrameSetWheelStep(pWidget, __uiDesignFloat(pNode, "metrics.wheelStep", 48.0f));
	(void)xuiScrollFrameSetContentDragEnabled(pWidget, __uiDesignBool(pNode, "behavior.contentDrag", 0));
	(void)xuiScrollFrameSetCornerMode(pWidget, __uiDesignInt(pNode, "behavior.cornerMode", XUI_SCROLL_FRAME_CORNER_AUTO));
	(void)xuiScrollFrameSetMetrics(pWidget,
		__uiDesignFloat(pNode, "metrics.scrollbarSize", 14.0f),
		__uiDesignFloat(pNode, "metrics.minThumbSize", 24.0f),
		__uiDesignFloat(pNode, "metrics.buttonSize", 16.0f));
	(void)xuiScrollFrameSetBackgroundColor(pWidget, __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE));
	(void)xuiScrollFrameSetColors(pWidget,
		__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255)),
		__uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255)),
		__uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(142, 166, 192, 255)),
		__uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(111, 137, 166, 255)),
		__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
		__uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(206, 215, 226, 255)));
	(void)xuiScrollFrameSetButtonColors(pWidget,
		__uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(239, 245, 252, 255)),
		__uiDesignColor(pNode, "appearance.buttonIconColor", XUI_COLOR_RGBA(52, 71, 93, 255)));
	(void)xuiScrollFrameSetCornerColors(pWidget,
		__uiDesignColor(pNode, "appearance.cornerColor", XUI_COLOR_RGBA(238, 243, 249, 255)),
		__uiDesignColor(pNode, "appearance.gripColor", XUI_COLOR_RGBA(142, 166, 192, 255)));
}

static void __uiDesignApplyScrollView(ui_design_node_t* pNode)
{
	xui_widget pWidget;

	pWidget = pNode->pWidget;
	(void)xuiScrollViewSetContentSize(pWidget, __uiDesignFloat(pNode, "metrics.contentWidth", 520.0f), __uiDesignFloat(pNode, "metrics.contentHeight", 360.0f));
	(void)xuiScrollViewSetOffset(pWidget, __uiDesignFloat(pNode, "value.offsetX", 0.0f), __uiDesignFloat(pNode, "value.offsetY", 0.0f));
	(void)xuiScrollViewSetScrollbarPolicy(pWidget, __uiDesignInt(pNode, "behavior.policyX", XUI_SCROLLBAR_POLICY_AUTO), __uiDesignInt(pNode, "behavior.policyY", XUI_SCROLLBAR_POLICY_AUTO));
	(void)xuiScrollViewSetScrollbarMode(pWidget, __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL));
	(void)xuiScrollViewSetWheelAxis(pWidget, __uiDesignInt(pNode, "behavior.wheelAxis", XUI_WHEEL_AXIS_VERTICAL));
	(void)xuiScrollViewSetWheelStep(pWidget, __uiDesignFloat(pNode, "metrics.wheelStep", 48.0f));
	(void)xuiScrollViewSetContentDragEnabled(pWidget, __uiDesignBool(pNode, "behavior.contentDrag", 0));
	(void)xuiScrollViewSetCornerMode(pWidget, __uiDesignInt(pNode, "behavior.cornerMode", XUI_SCROLL_FRAME_CORNER_AUTO));
	(void)xuiScrollViewSetMetrics(pWidget,
		__uiDesignFloat(pNode, "metrics.scrollbarSize", 14.0f),
		__uiDesignFloat(pNode, "metrics.minThumbSize", 24.0f),
		__uiDesignFloat(pNode, "metrics.buttonSize", 16.0f));
	(void)xuiScrollViewSetBackgroundColor(pWidget, __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE));
	(void)xuiScrollViewSetColors(pWidget,
		__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255)),
		__uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255)),
		__uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(142, 166, 192, 255)),
		__uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(111, 137, 166, 255)),
		__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
		__uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(206, 215, 226, 255)));
	(void)xuiScrollViewSetButtonColors(pWidget,
		__uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(239, 245, 252, 255)),
		__uiDesignColor(pNode, "appearance.buttonIconColor", XUI_COLOR_RGBA(52, 71, 93, 255)));
	(void)xuiScrollViewSetCornerColors(pWidget,
		__uiDesignColor(pNode, "appearance.cornerColor", XUI_COLOR_RGBA(238, 243, 249, 255)),
		__uiDesignColor(pNode, "appearance.gripColor", XUI_COLOR_RGBA(142, 166, 192, 255)));
}

static void __uiDesignApplyMessageDefaults(ui_design_node_t* pNode)
{
	xui_message_list_metrics_t tMetrics;
	xui_message_list_colors_t tColors;

	__uiDesignFillMessageMetrics(pNode, &tMetrics);
	__uiDesignFillMessageColors(pNode, &tColors);
	__uiDesignTextPoolReset(pNode);
	(void)__uiDesignApplyMessageData(pNode, pNode->pWidget);
	(void)xuiMessageListSetSelected(pNode->pWidget, __uiDesignInt(pNode, "data.selected", 1));
	(void)xuiMessageListSetScroll(pNode->pWidget, __uiDesignFloat(pNode, "value.scrollY", 0.0f));
	(void)xuiMessageListSetAutoScroll(pNode->pWidget, __uiDesignBool(pNode, "behavior.autoScroll", 1));
	(void)xuiMessageListSetMetrics(pNode->pWidget, &tMetrics);
	(void)xuiMessageListSetColors(pNode->pWidget, &tColors);
}

static void __uiDesignApplyTimelineDefaults(ui_design_node_t* pNode)
{
	xui_timeline_view_colors_t tColors;

	__uiDesignFillTimelineColors(pNode, &tColors);
	(void)xuiTimeLineViewClear(pNode->pWidget);
	(void)xuiTimeLineViewSetFrameCount(pNode->pWidget, __uiDesignInt(pNode, "data.frameCount", 120));
	(void)xuiTimeLineViewSetFrameRate(pNode->pWidget, __uiDesignFloat(pNode, "value.frameRate", 24.0f));
	(void)xuiTimeLineViewSetCurrentFrame(pNode->pWidget, __uiDesignInt(pNode, "data.currentFrame", 24));
	(void)xuiTimeLineViewSetMetrics(pNode->pWidget,
		__uiDesignFloat(pNode, "metrics.layerHeaderWidth", 110.0f),
		__uiDesignFloat(pNode, "metrics.frameWidth", 16.0f),
		__uiDesignFloat(pNode, "metrics.rowHeight", 24.0f),
		__uiDesignFloat(pNode, "metrics.rulerHeight", 24.0f));
	(void)xuiTimeLineViewSetFrameWidthRange(pNode->pWidget,
		__uiDesignFloat(pNode, "metrics.minFrameWidth", 8.0f),
		__uiDesignFloat(pNode, "metrics.maxFrameWidth", 64.0f));
	(void)xuiTimeLineViewSetFeatureFlags(pNode->pWidget,
		__uiDesignBool(pNode, "behavior.visibilityFeature", 1),
		__uiDesignBool(pNode, "behavior.lockFeature", 1));
	(void)xuiTimeLineViewSetScrollbarMode(pNode->pWidget, __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL));
	(void)xuiTimeLineViewSetColors(pNode->pWidget, &tColors);
	(void)__uiDesignApplyTimelineData(pNode);
	(void)xuiTimeLineViewSetOffset(pNode->pWidget,
		__uiDesignFloat(pNode, "value.offsetX", 0.0f),
		__uiDesignFloat(pNode, "value.offsetY", 0.0f));
	(void)__uiDesignApplyTimelineMenuTitles(pNode, pNode->pWidget);
}

static void __uiDesignApplyPropertyGridDefaults(ui_design_node_t* pNode)
{
	xui_property_grid_style_t tStyle;
	xui_widget pTableView;

	__uiDesignTextPoolReset(pNode);
	(void)xuiPropertyGridClear(pNode->pWidget);
	(void)xuiPropertyGridSetMetrics(pNode->pWidget,
		__uiDesignFloat(pNode, "metrics.nameWidth", 110.0f),
		__uiDesignFloat(pNode, "metrics.rowHeight", 24.0f),
		__uiDesignFloat(pNode, "metrics.categoryHeight", 24.0f));
	(void)xuiPropertyGridSetDescriptionMode(pNode->pWidget,
		__uiDesignInt(pNode, "behavior.descriptionMode", XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP),
		__uiDesignFloat(pNode, "metrics.descriptionHeight", 70.0f));
	(void)xuiPropertyGridSetEditMode(pNode->pWidget, __uiDesignInt(pNode, "behavior.editMode", XUI_TABLE_GRID_EDIT_QUICK));
	(void)xuiPropertyGridSetScrollbarMode(pNode->pWidget, __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL));
	(void)xuiPropertyGridSetColors(pNode->pWidget,
		__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
		__uiDesignColor(pNode, "appearance.categoryColor", XUI_COLOR_RGBA(234, 241, 248, 255)),
		__uiDesignColor(pNode, "appearance.rowColor", XUI_COLOR_WHITE),
		__uiDesignColor(pNode, "appearance.selectedColor", XUI_COLOR_RGBA(217, 234, 254, 255)),
		__uiDesignColor(pNode, "appearance.gridColor", XUI_COLOR_RGBA(208, 219, 232, 255)),
		__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)));
	__uiDesignFillPropertyGridStyle(pNode, &tStyle);
	(void)xuiPropertyGridSetStyle(pNode->pWidget, &tStyle);
	pTableView = xuiPropertyGridGetTableView(pNode->pWidget);
	if ( pTableView != NULL ) {
		(void)xuiTableViewSetScrollbarColors(pTableView,
			__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255)),
			__uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarHoverColor", XUI_COLOR_RGBA(142, 166, 192, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarActiveColor", XUI_COLOR_RGBA(111, 137, 166, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarFocusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarDisabledColor", XUI_COLOR_RGBA(206, 215, 226, 255)));
	}
	(void)__uiDesignApplyPropertyGridData(pNode);
	(void)xuiPropertyGridSetSelected(pNode->pWidget, __uiDesignInt(pNode, "data.selected", 0));
	(void)xuiPropertyGridSetScroll(pNode->pWidget, __uiDesignFloat(pNode, "value.scrollY", 0.0f));
}

static int __uiDesignHasTooltipProperties(const ui_design_node_t* pNode)
{
	return __uiDesignHasProperty(pNode, "common.tooltipText") ||
	       __uiDesignHasProperty(pNode, "common.tooltipAnchor") ||
	       __uiDesignHasProperty(pNode, "common.tooltipFollowCursor") ||
	       __uiDesignHasProperty(pNode, "common.tooltipDelay") ||
	       __uiDesignHasProperty(pNode, "common.tooltipOffsetX") ||
	       __uiDesignHasProperty(pNode, "common.tooltipOffsetY") ||
	       __uiDesignHasProperty(pNode, "common.tooltipCustomAnchor") ||
	       __uiDesignHasProperty(pNode, "common.tooltipAnchorX") ||
	       __uiDesignHasProperty(pNode, "common.tooltipAnchorY") ||
	       __uiDesignHasProperty(pNode, "common.tooltipAnchorW") ||
	       __uiDesignHasProperty(pNode, "common.tooltipAnchorH");
}

static void __uiDesignApplyTooltip(ui_design_node_t* pNode, xui_widget pWidget)
{
	xui_tooltip_desc_t tTooltip;
	const char* sText;

	if ( (pNode == NULL) || (pWidget == NULL) || !__uiDesignHasTooltipProperties(pNode) ) return;
	sText = __uiDesignText(pNode, "common.tooltipText", "");
	if ( sText[0] == '\0' ) {
		(void)xuiWidgetClearTooltip(pWidget);
		return;
	}
	memset(&tTooltip, 0, sizeof(tTooltip));
	tTooltip.iSize = sizeof(tTooltip);
	tTooltip.iType = XUI_TOOLTIP_TEXT;
	tTooltip.sText = sText;
	tTooltip.iAnchor = __uiDesignInt(pNode, "common.tooltipAnchor", XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM);
	tTooltip.fOffsetX = __uiDesignFloat(pNode, "common.tooltipOffsetX", 0.0f);
	tTooltip.fOffsetY = __uiDesignFloat(pNode, "common.tooltipOffsetY", 6.0f);
	tTooltip.fDelay = __uiDesignFloat(pNode, "common.tooltipDelay", 0.35f);
	tTooltip.bFollowCursor = __uiDesignBool(pNode, "common.tooltipFollowCursor", 0);
	tTooltip.bCustomAnchorRect = __uiDesignBool(pNode, "common.tooltipCustomAnchor", 0);
	if ( tTooltip.bCustomAnchorRect ) {
		tTooltip.tAnchorRect.fX = __uiDesignFloat(pNode, "common.tooltipAnchorX", 0.0f);
		tTooltip.tAnchorRect.fY = __uiDesignFloat(pNode, "common.tooltipAnchorY", 0.0f);
		tTooltip.tAnchorRect.fW = __uiDesignFloat(pNode, "common.tooltipAnchorW", 0.0f);
		tTooltip.tAnchorRect.fH = __uiDesignFloat(pNode, "common.tooltipAnchorH", 0.0f);
	}
	(void)xuiWidgetSetTooltip(pWidget, &tTooltip);
}

static void __uiDesignApplyCanvasPreview(ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	uint32_t iClear;
	uint32_t iPen;
	uint32_t iBorder;
	xui_widget pFrame;

	iClear = __uiDesignColor(pNode, "appearance.clearColor", XUI_COLOR_WHITE);
	iPen = __uiDesignColor(pNode, "appearance.penColor", XUI_COLOR_RGBA(49, 126, 214, 255));
	iBorder = __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255));
	(void)xuiCanvasSetCanvasSize(pNode->pWidget, __uiDesignFloat(pNode, "metrics.canvasWidth", 520.0f), __uiDesignFloat(pNode, "metrics.canvasHeight", 360.0f));
	(void)xuiCanvasSetOffset(pNode->pWidget, __uiDesignFloat(pNode, "value.offsetX", 0.0f), __uiDesignFloat(pNode, "value.offsetY", 0.0f));
	(void)xuiCanvasSetScrollbarPolicy(pNode->pWidget, __uiDesignInt(pNode, "behavior.policyX", XUI_SCROLLBAR_POLICY_AUTO), __uiDesignInt(pNode, "behavior.policyY", XUI_SCROLLBAR_POLICY_AUTO));
	(void)xuiCanvasSetScrollbarMode(pNode->pWidget, __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_COMPACT));
	pFrame = xuiCanvasGetFrameWidget(pNode->pWidget);
	if ( pFrame != NULL ) {
		(void)xuiScrollFrameSetWheelAxis(pFrame, __uiDesignInt(pNode, "behavior.wheelAxis", XUI_WHEEL_AXIS_BOTH));
		(void)xuiScrollFrameSetWheelStep(pFrame, __uiDesignFloat(pNode, "metrics.wheelStep", 48.0f));
		(void)xuiScrollFrameSetContentDragEnabled(pFrame, __uiDesignBool(pNode, "behavior.contentDrag", 1));
		(void)xuiScrollFrameSetCornerMode(pFrame, __uiDesignInt(pNode, "behavior.cornerMode", XUI_SCROLL_FRAME_CORNER_AUTO));
		(void)xuiScrollFrameSetMetrics(pFrame,
			__uiDesignFloat(pNode, "metrics.scrollbarSize", 14.0f),
			__uiDesignFloat(pNode, "metrics.minThumbSize", 24.0f),
			__uiDesignFloat(pNode, "metrics.buttonSize", 0.0f));
		(void)xuiScrollFrameSetColors(pFrame,
			__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255)),
			__uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255)),
			__uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(142, 166, 192, 255)),
			__uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(111, 137, 166, 255)),
			__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
			__uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(206, 215, 226, 255)));
		(void)xuiScrollFrameSetButtonColors(pFrame,
			__uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(239, 245, 252, 255)),
			__uiDesignColor(pNode, "appearance.buttonIconColor", XUI_COLOR_RGBA(52, 71, 93, 255)));
		(void)xuiScrollFrameSetCornerColors(pFrame,
			__uiDesignColor(pNode, "appearance.cornerColor", XUI_COLOR_RGBA(238, 243, 249, 255)),
			__uiDesignColor(pNode, "appearance.gripColor", XUI_COLOR_RGBA(142, 166, 192, 255)));
	}
	(void)xuiCanvasSetColors(pNode->pWidget,
		__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
		iBorder,
		iPen);
	(void)xuiCanvasSetPen(pNode->pWidget, __uiDesignBool(pNode, "behavior.penEnabled", 0), __uiDesignFloat(pNode, "metrics.penWidth", 2.0f), iPen);
	(void)xuiCanvasClear(pNode->pWidget, iClear);
	(void)__uiDesignApplyCanvasCommands(pApp, pNode, iPen, iBorder);
}

static void __uiDesignApplyTableSelection(xui_widget pTableWidget, const ui_design_node_t* pNode)
{
	int iMode;
	int iRow;
	int iColumn;

	if ( (pTableWidget == NULL) || (pNode == NULL) ) return;
	iMode = __uiDesignInt(pNode, "behavior.selectionMode", XUI_TABLE_VIEW_SELECTION_ROW);
	iRow = __uiDesignInt(pNode, "data.selectedRow", 0);
	iColumn = __uiDesignInt(pNode, "data.selectedColumn", -1);
	(void)xuiTableViewSetSelectionMode(pTableWidget, iMode);
	if ( iMode == XUI_TABLE_VIEW_SELECTION_ROW ) {
		(void)xuiTableViewSetSelectedRow(pTableWidget, iRow);
	} else if ( iColumn >= 0 ) {
		(void)xuiTableViewSetSelectedCell(pTableWidget, iRow, iColumn);
	}
}

static int __uiDesignApplyNode(struct ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	xui_widget pWidget;
	xui_table_view_colors_t tTableColors;
	xui_menubar_metrics_t tMenuBarMetrics;
	xui_menubar_colors_t tMenuBarColors;
	xui_toolbar_metrics_t tToolbarMetrics;
	xui_toolbar_colors_t tToolbarColors;
	xui_statusbar_metrics_t tStatusBarMetrics;
	xui_statusbar_colors_t tStatusBarColors;
	xui_dock_panel_metrics_t tDockMetrics;
	xui_dock_panel_colors_t tDockColors;
	xui_menu_metrics_t tMenuMetrics;
	xui_menu_colors_t tMenuColors;
	xui_widget pMenuContent;
	xui_widget pMenuWidget;
	xui_widget pTableWidget;
	xui_widget pInputWidget;
	xui_surface pSurface;
	xui_rect_t tSrc;
	xui_rect_t tPreviewAnchor;
	const char* arrTextItems[UI_DESIGN_RUNTIME_TEXT_COUNT];
	xui_combobox_item_t arrComboItems[XUI_COMBOBOX_ITEM_CAPACITY];
	int arrListEnabled[UI_DESIGN_RUNTIME_TEXT_COUNT];
	int arrListSelected[UI_DESIGN_RUNTIME_TEXT_COUNT];
	xui_breadcrumb_item_t arrBreadcrumbItems[XUI_BREADCRUMB_MAX_ITEMS];
	const char* arrTabItems[XUI_TABS_PAGE_CAPACITY];
	int arrTabEnabled[XUI_TABS_PAGE_CAPACITY];
	int arrTabDirty[XUI_TABS_PAGE_CAPACITY];
	xui_surface arrTabIcons[XUI_TABS_PAGE_CAPACITY];
	xui_rect_t arrTabIconSrc[XUI_TABS_PAGE_CAPACITY];
	xui_accordion_section_desc_t arrAccordionSections[XUI_ACCORDION_SECTION_CAPACITY];
	xui_menubar_item_t arrMenuBarItems[XUI_MENUBAR_ITEM_CAPACITY];
	xui_toolbar_item_t arrToolbarItems[XUI_TOOLBAR_ITEM_CAPACITY];
	xui_statusbar_item_t arrStatusBarItems[XUI_STATUSBAR_ITEM_CAPACITY];
	xui_menu_item_t arrMenuItems[XUI_MENU_ITEM_CAPACITY];
	xui_cascader_item_t arrCascaderItems[XUI_CASCADER_ITEM_CAPACITY];
	int arrCascaderPath[16];
	ui_design_chart_series_def_t arrChartSeries[16];
	xui_chart_point_t arrChartPoints[UI_DESIGN_RUNTIME_TABLE_ROWS];
	uint32_t arrPalette[XUI_COLOR_PICKER_PALETTE_CAPACITY];
	xui_thickness_t tChartPadding;
	float fMin;
	float fMax;
	int iTextItemCount;
	int iBreadcrumbItemCount;
	int iTabItemCount;
	int iAccordionSectionCount;
	int iMenuBarItemCount;
	int iToolbarItemCount;
	int iStatusBarItemCount;
	int iMenuItemCount;
	int iCascaderItemCount;
	int iCascaderDepth;
	int iChartSeriesCount;
	int iChartSeriesIndex;
	int iChartPointCount;
	int iPaletteCount;
	int iComboSelectedIndex;
	int iComboSelectedValue;
	int iPreviewIndex;
	int bListExplicitSelection;
	int iRet;
	int i;

	if ( (pNode == NULL) || (pNode->pWidget == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pWidget = pNode->pWidget;
	(void)xuiWidgetSetRect(pWidget, pNode->tRect);
	(void)xuiWidgetSetVisible(pWidget, pNode->bVisible);
	(void)xuiWidgetSetEnabled(pWidget, pNode->bEnabled);
	if ( !pApp->bPreviewRunner ) {
		(void)xuiWidgetSetHitTestVisible(pWidget, 0);
	}
	__uiDesignApplyGenericLayout(pNode);
	(void)__uiDesignApplyNodeFont(pApp, pNode);
	switch ( pNode->iType ) {
	case UI_DESIGN_NODE_LABEL:
		(void)xuiLabelSetText(pWidget, pNode->sText);
		(void)xuiLabelSetTextColor(pWidget, __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)));
		(void)xuiLabelSetDisabledTextColor(pWidget, __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)));
		(void)xuiLabelSetTextFlags(pWidget, __uiDesignInt(pNode, "text.flags", XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP));
		(void)xuiLabelSetWrapMode(pWidget, __uiDesignInt(pNode, "text.wrapMode", XUI_TEXT_WRAP_NONE));
		(void)xuiLabelSetUnderline(pWidget, __uiDesignBool(pNode, "text.underline", 0));
		(void)xuiLabelSetLineGap(pWidget, __uiDesignFloat(pNode, "text.lineGap", 0.0f));
		(void)xuiLabelSetParagraphGap(pWidget, __uiDesignFloat(pNode, "text.paragraphGap", 0.0f));
		break;
	case UI_DESIGN_NODE_HYPERLINK:
		(void)xuiHyperlinkSetText(pWidget, pNode->sText);
		(void)xuiHyperlinkSetTextColors(pWidget,
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(28, 102, 176, 255)),
			__uiDesignColor(pNode, "appearance.hoverTextColor", XUI_COLOR_RGBA(36, 132, 214, 255)),
			__uiDesignColor(pNode, "appearance.activeTextColor", XUI_COLOR_RGBA(22, 104, 176, 255)),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)));
		(void)xuiHyperlinkSetTextFlags(pWidget, __uiDesignInt(pNode, "text.flags", XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP));
		(void)xuiHyperlinkSetWrapMode(pWidget, __uiDesignInt(pNode, "text.wrapMode", XUI_TEXT_WRAP_NONE));
		(void)xuiHyperlinkSetUnderline(pWidget, __uiDesignBool(pNode, "text.underline", 1), __uiDesignBool(pNode, "text.hoverUnderline", 1), __uiDesignBool(pNode, "text.activeUnderline", 1));
		(void)xuiHyperlinkSetLineGap(pWidget, __uiDesignFloat(pNode, "text.lineGap", 0.0f));
		(void)xuiHyperlinkSetParagraphGap(pWidget, __uiDesignFloat(pNode, "text.paragraphGap", 0.0f));
		break;
	case UI_DESIGN_NODE_BUTTON:
		__uiDesignApplyButtonProperties(pApp, pNode);
		break;
	case UI_DESIGN_NODE_CHECKBOX:
		__uiDesignApplyCheckProperties(pApp, pNode, 0);
		break;
	case UI_DESIGN_NODE_RADIO:
		__uiDesignApplyCheckProperties(pApp, pNode, 1);
		break;
	case UI_DESIGN_NODE_TOGGLE:
		(void)xuiToggleSetText(pWidget, pNode->sText);
		(void)xuiToggleSetChecked(pWidget, pNode->bChecked);
		(void)xuiToggleUseBuiltinAtlas(pWidget, __uiDesignBool(pNode, "behavior.useBuiltinAtlas", 1));
		(void)xuiToggleSetInnerText(pWidget, __uiDesignText(pNode, "text.uncheckedText", "Off"), __uiDesignText(pNode, "text.checkedText", "On"));
		(void)xuiToggleSetInnerTextColor(pWidget, __uiDesignColor(pNode, "appearance.uncheckedTextColor", XUI_COLOR_RGBA(82, 101, 122, 255)), __uiDesignColor(pNode, "appearance.checkedTextColor", XUI_COLOR_WHITE));
		(void)xuiToggleSetTextColor(pWidget, __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)));
		(void)xuiToggleSetDisabledTextColor(pWidget, __uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)));
		(void)xuiToggleSetTrackSize(pWidget, __uiDesignFloat(pNode, "metrics.trackWidth", 44.0f), __uiDesignFloat(pNode, "metrics.trackHeight", 22.0f));
		(void)xuiToggleSetThumbSize(pWidget, __uiDesignFloat(pNode, "metrics.thumbSize", 18.0f));
		(void)xuiToggleSetGap(pWidget, __uiDesignFloat(pNode, "metrics.gap", 8.0f));
		(void)xuiToggleSetInnerTextMetrics(pWidget, __uiDesignFloat(pNode, "metrics.innerTextPadding", 6.0f), __uiDesignFloat(pNode, "metrics.innerTextGap", 2.0f));
		(void)xuiToggleSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.accentColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(213, 222, 232, 255)),
			__uiDesignColor(pNode, "appearance.hoverTrackColor", XUI_COLOR_RGBA(199, 215, 234, 255)),
			__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)));
		__uiDesignApplyToggleIndicatorSurface(pApp, pNode);
		break;
	case UI_DESIGN_NODE_INPUT:
		(void)xuiInputSetText(pWidget, pNode->sText);
		(void)xuiInputSetPlaceholder(pWidget, __uiDesignText(pNode, "text.placeholder", "Input text"));
		(void)xuiInputSetMaxLength(pWidget, __uiDesignInt(pNode, "text.maxLength", 0));
		(void)xuiInputSetTextAlign(pWidget, __uiDesignInt(pNode, "text.align", XUI_INPUT_ALIGN_LEFT));
		(void)xuiInputSetSelection(pWidget, __uiDesignInt(pNode, "value.selectionStart", 0), __uiDesignInt(pNode, "value.selectionEnd", 0));
		(void)xuiInputSetPassword(pWidget, __uiDesignBool(pNode, "behavior.password", 0));
		(void)xuiInputSetReadonly(pWidget, __uiDesignBool(pNode, "behavior.readonly", 0));
		(void)xuiInputSetError(pWidget, __uiDesignBool(pNode, "behavior.error", 0));
		(void)xuiInputSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255)));
		(void)xuiInputSetErrorColors(pWidget,
			__uiDesignColor(pNode, "appearance.errorBackgroundColor", XUI_COLOR_RGBA(255, 242, 243, 255)),
			__uiDesignColor(pNode, "appearance.errorBorderColor", XUI_COLOR_RGBA(214, 72, 86, 255)));
		(void)xuiInputSetExtendedColors(pWidget,
			__uiDesignColor(pNode, "appearance.placeholderColor", XUI_COLOR_RGBA(135, 148, 166, 255)),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(150, 160, 172, 255)),
			__uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(249, 252, 255, 255)),
			__uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(242, 245, 249, 255)),
			__uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(105, 166, 226, 255)),
			__uiDesignColor(pNode, "appearance.selectionColor", XUI_COLOR_RGBA(47, 128, 237, 78)),
			__uiDesignColor(pNode, "appearance.cursorColor", XUI_COLOR_RGBA(33, 94, 170, 255)));
		(void)xuiInputSetBorderWidth(pWidget, __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f));
		(void)__uiDesignApplyInputDecorations(pApp, pNode, pWidget);
		(void)__uiDesignApplyInputMenuTitles(pNode, pWidget, UI_DESIGN_INPUT_MENU_TARGET_INPUT);
		break;
	case UI_DESIGN_NODE_TAG_INPUT:
		__uiDesignTextPoolReset(pNode);
		iTextItemCount = __uiDesignParseTextLines(pNode, "data.tags", g_arrDefaultTags, UI_DESIGN_COUNT_OF(g_arrDefaultTags), arrTextItems, UI_DESIGN_COUNT_OF(arrTextItems));
		(void)xuiTagInputSetTags(pWidget, arrTextItems, iTextItemCount);
		(void)xuiTagInputSetText(pWidget, __uiDesignText(pNode, "text.inputText", ""));
		(void)xuiTagInputSetPlaceholder(pWidget, __uiDesignText(pNode, "text.placeholder", "Add tag"));
		(void)xuiTagInputSetMaxLength(pWidget, __uiDesignInt(pNode, "text.maxLength", 0));
		(void)xuiTagInputSetMaxTags(pWidget, __uiDesignInt(pNode, "behavior.maxTags", 0));
		(void)xuiTagInputSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.tagBackgroundColor", XUI_COLOR_RGBA(234, 243, 255, 255)),
			__uiDesignColor(pNode, "appearance.tagTextColor", XUI_COLOR_RGBA(28, 102, 176, 255)));
		(void)xuiTagInputSetExtendedColors(pWidget,
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(31, 41, 55, 255)),
			__uiDesignColor(pNode, "appearance.placeholderColor", XUI_COLOR_RGBA(135, 148, 166, 255)),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(150, 160, 172, 255)),
			__uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(250, 253, 255, 255)),
			__uiDesignColor(pNode, "appearance.focusBackgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(242, 245, 249, 255)),
			__uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(126, 176, 226, 255)),
			__uiDesignColor(pNode, "appearance.tagHoverBackgroundColor", XUI_COLOR_RGBA(235, 241, 248, 255)),
			__uiDesignColor(pNode, "appearance.tagCloseColor", XUI_COLOR_RGBA(116, 128, 144, 255)),
			__uiDesignColor(pNode, "appearance.tagCloseHoverColor", XUI_COLOR_RGBA(66, 78, 94, 255)));
		(void)xuiTagInputSetVisualMetrics(pWidget, __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f), __uiDesignFloat(pNode, "metrics.tagHeight", 22.0f));
		(void)__uiDesignApplyInputMenuTitles(pNode, pWidget, UI_DESIGN_INPUT_MENU_TARGET_TAG_INPUT);
		break;
	case UI_DESIGN_NODE_NUMERIC_INPUT:
		fMin = __uiDesignFloat(pNode, "value.min", 0.0f);
		fMax = __uiDesignFloat(pNode, "value.max", 100.0f);
		(void)xuiNumericInputSetRange(pWidget, fMin, fMax);
		(void)xuiNumericInputSetStep(pWidget, __uiDesignFloat(pNode, "value.step", 1.0f));
		(void)xuiNumericInputSetInteger(pWidget, __uiDesignBool(pNode, "value.integer", 0));
		(void)xuiNumericInputSetPrecision(pWidget, __uiDesignInt(pNode, "value.precision", 2));
		(void)xuiNumericInputSetReadonly(pWidget, __uiDesignBool(pNode, "behavior.readonly", 0));
		(void)xuiNumericInputSetSpinnerVisible(pWidget, __uiDesignBool(pNode, "behavior.spinnerVisible", 1));
		(void)xuiNumericInputSetSpinnerWidth(pWidget, __uiDesignFloat(pNode, "metrics.spinnerWidth", 22.0f));
		pInputWidget = xuiNumericInputGetInputWidget(pWidget);
		if ( pInputWidget != NULL ) {
			(void)xuiInputSetPlaceholder(pInputWidget, __uiDesignText(pNode, "text.placeholder", "0"));
		}
		(void)xuiNumericInputSetValue(pWidget, __uiDesignFloat(pNode, "value.value", 25.0f));
		if ( __uiDesignBool(pNode, "behavior.textOverride", 0) ) {
			(void)xuiNumericInputSetText(pWidget, __uiDesignText(pNode, "text.rawText", ""));
		}
		(void)xuiNumericInputSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255)));
		(void)xuiNumericInputSetExtendedColors(pWidget,
			__uiDesignColor(pNode, "appearance.placeholderColor", XUI_COLOR_RGBA(135, 148, 166, 255)),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(150, 160, 172, 255)),
			__uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(249, 252, 255, 255)),
			__uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(242, 245, 249, 255)),
			__uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(105, 166, 226, 255)),
			__uiDesignColor(pNode, "appearance.errorBackgroundColor", XUI_COLOR_RGBA(255, 242, 243, 255)),
			__uiDesignColor(pNode, "appearance.errorBorderColor", XUI_COLOR_RGBA(214, 72, 86, 255)),
			__uiDesignColor(pNode, "appearance.selectionColor", XUI_COLOR_RGBA(47, 128, 237, 78)),
			__uiDesignColor(pNode, "appearance.cursorColor", XUI_COLOR_RGBA(33, 94, 170, 255)));
		(void)xuiNumericInputSetBorderWidth(pWidget, __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f));
		(void)xuiNumericInputSetSpinnerColors(pWidget,
			__uiDesignColor(pNode, "appearance.spinnerColor", XUI_COLOR_RGBA(239, 245, 252, 255)),
			__uiDesignColor(pNode, "appearance.spinnerHoverColor", XUI_COLOR_RGBA(220, 235, 251, 255)),
			__uiDesignColor(pNode, "appearance.spinnerActiveColor", XUI_COLOR_RGBA(200, 221, 245, 255)),
			__uiDesignColor(pNode, "appearance.spinnerBorderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.spinnerIconColor", XUI_COLOR_RGBA(52, 71, 93, 255)),
			__uiDesignColor(pNode, "appearance.spinnerDisabledIconColor", XUI_COLOR_RGBA(154, 168, 186, 255)));
		(void)__uiDesignApplyInputMenuTitles(pNode, pWidget, UI_DESIGN_INPUT_MENU_TARGET_NUMERIC_INPUT);
		break;
	case UI_DESIGN_NODE_TEXT_EDIT:
		(void)xuiTextEditSetText(pWidget, __uiDesignText(pNode, "data.text", "Line 1\nLine 2\nLine 3"));
		(void)xuiTextEditSetPlaceholder(pWidget, __uiDesignText(pNode, "text.placeholder", "Multi-line text"));
		(void)xuiTextEditSetMaxLength(pWidget, __uiDesignInt(pNode, "text.maxLength", 0));
		(void)xuiTextEditSetReadonly(pWidget, __uiDesignBool(pNode, "behavior.readonly", 0));
		(void)xuiTextEditSetWordWrap(pWidget, __uiDesignBool(pNode, "behavior.wordWrap", 1));
		(void)xuiTextEditSetLineNumbers(pWidget, __uiDesignBool(pNode, "behavior.lineNumbers", 1), __uiDesignFloat(pNode, "metrics.lineNumberWidth", 42.0f));
		(void)xuiTextEditSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255)));
		(void)xuiTextEditSetExtendedColors(pWidget,
			__uiDesignColor(pNode, "appearance.placeholderColor", XUI_COLOR_RGBA(135, 148, 166, 255)),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(150, 160, 172, 255)),
			__uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(249, 252, 255, 255)),
			__uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(242, 245, 249, 255)),
			__uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(105, 166, 226, 255)),
			__uiDesignColor(pNode, "appearance.selectionColor", XUI_COLOR_RGBA(47, 128, 237, 78)),
			__uiDesignColor(pNode, "appearance.cursorColor", XUI_COLOR_RGBA(33, 94, 170, 255)),
			__uiDesignColor(pNode, "appearance.findResultColor", XUI_COLOR_RGBA(255, 235, 128, 150)),
			__uiDesignColor(pNode, "appearance.findActiveColor", XUI_COLOR_RGBA(255, 183, 77, 190)));
		(void)xuiTextEditSetVisualMetrics(pWidget,
			__uiDesignFloat(pNode, "metrics.borderWidth", 1.0f),
			__uiDesignFloat(pNode, "metrics.lineGap", 2.0f));
		(void)xuiTextEditSetSelection(pWidget,
			__uiDesignInt(pNode, "value.selectionStart", 0),
			__uiDesignInt(pNode, "value.selectionEnd", 0));
		(void)xuiTextEditSetScroll(pWidget,
			__uiDesignFloat(pNode, "value.scrollX", 0.0f),
			__uiDesignFloat(pNode, "value.scrollY", 0.0f));
		(void)xuiTextEditSetLineNumberColors(pWidget,
			__uiDesignColor(pNode, "appearance.lineNumberColor", XUI_COLOR_RGBA(107, 127, 149, 255)),
			__uiDesignColor(pNode, "appearance.lineNumberBackgroundColor", XUI_COLOR_RGBA(243, 247, 251, 255)),
			__uiDesignColor(pNode, "appearance.lineNumberBorderColor", XUI_COLOR_RGBA(207, 218, 231, 255)));
		(void)__uiDesignApplyInputMenuTitles(pNode, pWidget, UI_DESIGN_INPUT_MENU_TARGET_TEXT_EDIT);
		__uiDesignApplyTextEditFindPreview(pNode, pWidget);
		__uiDesignApplyTextEditFindWindow(pWidget, __uiDesignInt(pNode, "behavior.findWindow", 0));
		break;
	case UI_DESIGN_NODE_PANEL:
		(void)xuiPanelSetTitle(pWidget, pNode->sText);
		(void)xuiPanelSetTitleColor(pWidget, __uiDesignColor(pNode, "appearance.titleColor", XUI_COLOR_RGBA(35, 50, 70, 255)));
		(void)xuiPanelSetDisabledTitleColor(pWidget, __uiDesignColor(pNode, "appearance.disabledTitleColor", XUI_COLOR_RGBA(140, 154, 175, 255)));
		(void)xuiPanelSetBackgroundColor(pWidget, __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 251, 255, 255)));
		(void)xuiPanelSetHeaderColor(pWidget, __uiDesignColor(pNode, "appearance.headerColor", XUI_COLOR_RGBA(234, 242, 251, 255)));
		(void)xuiPanelSetClientColor(pWidget, __uiDesignColor(pNode, "appearance.clientColor", XUI_COLOR_WHITE));
		(void)xuiPanelSetBorder(pWidget, __uiDesignFloat(pNode, "appearance.borderWidth", 1.0f), __uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)));
		(void)xuiPanelSetTitleAlign(pWidget, __uiDesignInt(pNode, "text.flags", XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP));
		(void)xuiPanelSetHeaderHeight(pWidget, __uiDesignFloat(pNode, "metrics.headerHeight", 28.0f));
		(void)xuiPanelSetHeaderGap(pWidget, __uiDesignFloat(pNode, "metrics.headerGap", 0.0f));
		(void)xuiPanelSetIconSize(pWidget, __uiDesignFloat(pNode, "metrics.iconSize", 16.0f));
		tSrc = (xui_rect_t){
			__uiDesignFloat(pNode, "icon.x", 0.0f),
			__uiDesignFloat(pNode, "icon.y", 0.0f),
			__uiDesignFloat(pNode, "icon.w", 0.0f),
			__uiDesignFloat(pNode, "icon.h", 0.0f)
		};
		if ( __uiDesignLoadRuntimeSurface(pApp, pNode, __uiDesignText(pNode, "data.iconSource", ""), &pSurface) == XUI_OK ) {
			(void)xuiPanelSetIcon(pWidget, pSurface, tSrc);
		}
		(void)xuiPanelSetClientClip(pWidget, __uiDesignBool(pNode, "behavior.clientClip", 1));
		iRet = __uiDesignRefreshContainerModelChildren(pApp, pNode);
		if ( iRet != XUI_OK ) return iRet;
		break;
	case UI_DESIGN_NODE_SEPARATOR:
		(void)xuiSeparatorSetColor(pWidget, __uiDesignColor(pNode, "appearance.color", XUI_COLOR_RGBA(158, 182, 209, 255)));
		(void)xuiSeparatorSetThickness(pWidget, __uiDesignFloat(pNode, "metrics.thickness", 1.0f));
		(void)xuiSeparatorSetOrientation(pWidget, __uiDesignInt(pNode, "behavior.orientation", XUI_SEPARATOR_HORIZONTAL));
		(void)xuiSeparatorSetAlign(pWidget, __uiDesignInt(pNode, "behavior.align", XUI_ALIGN_CENTER));
		(void)xuiSeparatorSetLineStyle(pWidget, __uiDesignInt(pNode, "behavior.lineStyle", XUI_SEPARATOR_SOLID));
		break;
	case UI_DESIGN_NODE_PROGRESS:
		(void)xuiProgressSetRange(pWidget, __uiDesignFloat(pNode, "value.min", 0.0f), __uiDesignFloat(pNode, "value.max", 100.0f));
		(void)xuiProgressSetValue(pWidget, __uiDesignFloat(pNode, "value.value", 42.0f));
		(void)xuiProgressSetTextTemplate(pWidget, __uiDesignText(pNode, "text.template", "{value}%"));
		(void)xuiProgressSetTextFlags(pWidget, __uiDesignInt(pNode, "text.flags", XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP));
		(void)xuiProgressSetTextColor(pWidget, __uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)));
		(void)xuiProgressSetFillTextColor(pWidget, __uiDesignColor(pNode, "appearance.fillTextColor", XUI_COLOR_WHITE));
		(void)xuiProgressSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(227, 234, 242, 255)),
			__uiDesignColor(pNode, "appearance.fillColor", XUI_COLOR_RGBA(49, 126, 214, 255)));
		(void)xuiProgressSetFillDirection(pWidget, __uiDesignInt(pNode, "behavior.fillDirection", XUI_PROGRESS_LEFT_TO_RIGHT));
		(void)xuiProgressSetFillPatchMode(pWidget, __uiDesignInt(pNode, "behavior.fillPatchMode", XUI_PROGRESS_FILL_STRETCH));
		(void)__uiDesignApplyProgressPatches(pApp, pNode, pWidget);
		break;
	case UI_DESIGN_NODE_STEP_BAR:
		__uiDesignTextPoolReset(pNode);
		iTextItemCount = __uiDesignParseTextLines(pNode, "data.steps", g_arrDefaultStepTitles, UI_DESIGN_COUNT_OF(g_arrDefaultStepTitles), arrTextItems, XUI_STEP_BAR_MAX_STEPS);
		(void)xuiStepBarSetSteps(pWidget, arrTextItems, iTextItemCount);
		(void)xuiStepBarSetCurrent(pWidget, __uiDesignInt(pNode, "data.current", 1));
		(void)xuiStepBarSetStyle(pWidget, __uiDesignInt(pNode, "behavior.style", XUI_STEP_BAR_STYLE_DOT));
		(void)xuiStepBarSetMetrics(pWidget, __uiDesignFloat(pNode, "metrics.barHeight", 4.0f), __uiDesignFloat(pNode, "metrics.dotRadius", 8.0f), __uiDesignFloat(pNode, "metrics.lineWidth", 2.0f));
		(void)xuiStepBarSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.doneColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(214, 72, 86, 255)),
			__uiDesignColor(pNode, "appearance.pendingColor", XUI_COLOR_RGBA(184, 197, 211, 255)),
			__uiDesignColor(pNode, "appearance.lineColor", XUI_COLOR_RGBA(213, 222, 232, 255)));
		(void)xuiStepBarSetTextColors(pWidget,
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.activeTextColor", XUI_COLOR_RGBA(214, 72, 86, 255)),
			__uiDesignColor(pNode, "appearance.pendingTextColor", XUI_COLOR_RGBA(107, 127, 149, 255)));
		(void)xuiStepBarSetBackgroundColor(pWidget, __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(255, 255, 255, 0)));
		break;
	case UI_DESIGN_NODE_CHART:
		(void)xuiChartSetTitle(pWidget, pNode->sText);
		(void)xuiChartSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.plotColor", XUI_COLOR_RGBA(247, 250, 254, 255)),
			__uiDesignColor(pNode, "appearance.gridColor", XUI_COLOR_RGBA(217, 227, 239, 255)),
			__uiDesignColor(pNode, "appearance.axisColor", XUI_COLOR_RGBA(107, 127, 149, 255)),
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.tooltipColor", XUI_COLOR_RGBA(35, 50, 70, 235)),
			__uiDesignColor(pNode, "appearance.tooltipTextColor", XUI_COLOR_WHITE));
		(void)xuiChartSetXAxis(pWidget, __uiDesignInt(pNode, "behavior.xAxis", XUI_CHART_AXIS_VALUE));
		(void)xuiChartSetYAxis(pWidget, __uiDesignInt(pNode, "behavior.yAxis", XUI_CHART_AXIS_VALUE));
		(void)xuiChartSetBarMode(pWidget, __uiDesignInt(pNode, "behavior.barMode", XUI_CHART_BAR_GROUPED));
		(void)xuiChartSetBarDirection(pWidget, __uiDesignInt(pNode, "behavior.barDirection", XUI_CHART_BAR_VERTICAL));
		(void)xuiChartSetLegendVisible(pWidget, __uiDesignBool(pNode, "behavior.legendVisible", 1));
		(void)xuiChartSetTooltipVisible(pWidget, __uiDesignBool(pNode, "behavior.tooltipVisible", 1));
		(void)xuiChartSetAnimation(pWidget, __uiDesignBool(pNode, "behavior.animation", 0), __uiDesignFloat(pNode, "behavior.animationDuration", 0.25f));
		(void)xuiChartSetLodThreshold(pWidget, __uiDesignInt(pNode, "behavior.lodThreshold", 800));
		(void)xuiChartSetPieMode(pWidget, __uiDesignInt(pNode, "behavior.pieMode", XUI_CHART_PIE_NORMAL));
		(void)xuiChartSetPieInnerRadius(pWidget, __uiDesignFloat(pNode, "metrics.pieInnerRadius", 0.0f));
		tChartPadding = (xui_thickness_t){
			__uiDesignFloat(pNode, "metrics.paddingLeft", 38.0f),
			__uiDesignFloat(pNode, "metrics.paddingTop", 28.0f),
			__uiDesignFloat(pNode, "metrics.paddingRight", 18.0f),
			__uiDesignFloat(pNode, "metrics.paddingBottom", 34.0f)
		};
		(void)xuiChartSetPadding(pWidget, tChartPadding);
		if ( __uiDesignBool(pNode, "viewport.useViewRange", 0) ) {
			(void)xuiChartSetViewRange(pWidget,
				__uiDesignFloat(pNode, "viewport.minX", 0.0f),
				__uiDesignFloat(pNode, "viewport.maxX", 10.0f),
				__uiDesignFloat(pNode, "viewport.minY", 0.0f),
				__uiDesignFloat(pNode, "viewport.maxY", 10.0f));
		} else {
			(void)xuiChartResetViewRange(pWidget);
		}
		if ( __uiDesignBool(pNode, "viewport.useBrushRange", 0) ) {
			(void)xuiChartSetBrushRange(pWidget,
				__uiDesignFloat(pNode, "viewport.brushMinX", 0.0f),
				__uiDesignFloat(pNode, "viewport.brushMaxX", 0.0f),
				__uiDesignFloat(pNode, "viewport.brushMinY", 0.0f),
				__uiDesignFloat(pNode, "viewport.brushMaxY", 0.0f));
		} else {
			(void)xuiChartClearBrushRange(pWidget);
		}
		(void)xuiChartClearSeries(pWidget);
		__uiDesignTextPoolReset(pNode);
		iChartSeriesCount = __uiDesignBuildChartSeriesDefs(pNode, arrChartSeries, UI_DESIGN_COUNT_OF(arrChartSeries));
		for ( i = 0; i < iChartSeriesCount; ++i ) {
			if ( xuiChartAddSeries(pWidget, arrChartSeries[i].iType, arrChartSeries[i].sName, &iChartSeriesIndex) == XUI_OK ) {
				iChartPointCount = __uiDesignBuildChartPointsForSeries(pNode, arrChartSeries[i].sId, i == 0,
					arrChartSeries[i].iColor, arrChartPoints, UI_DESIGN_COUNT_OF(arrChartPoints));
				(void)xuiChartSetSeriesData(pWidget, iChartSeriesIndex, arrChartPoints, iChartPointCount);
				__uiDesignApplyChartSeriesStyle(pWidget, iChartSeriesIndex, &arrChartSeries[i]);
			}
		}
		break;
	case UI_DESIGN_NODE_SCROLLBAR:
		(void)xuiScrollBarSetRange(pWidget, __uiDesignFloat(pNode, "value.min", 0.0f), __uiDesignFloat(pNode, "value.max", 100.0f), __uiDesignFloat(pNode, "value.page", 20.0f));
		(void)xuiScrollBarSetValue(pWidget, __uiDesignFloat(pNode, "value.value", 30.0f));
		(void)xuiScrollBarSetSteps(pWidget, __uiDesignFloat(pNode, "value.smallStep", 1.0f), __uiDesignFloat(pNode, "value.largeStep", 10.0f));
		(void)xuiScrollBarSetOrientation(pWidget, __uiDesignInt(pNode, "behavior.orientation", XUI_ORIENTATION_HORIZONTAL));
		(void)xuiScrollBarSetMode(pWidget, __uiDesignInt(pNode, "behavior.mode", XUI_SCROLLBAR_MODE_FULL));
		(void)xuiScrollBarSetButtonMode(pWidget, __uiDesignInt(pNode, "behavior.buttonMode", XUI_SCROLLBAR_BUTTONS_AUTO));
		(void)xuiScrollBarSetMetrics(pWidget, __uiDesignFloat(pNode, "metrics.thickness", 14.0f), __uiDesignFloat(pNode, "metrics.minThumbSize", 24.0f), __uiDesignFloat(pNode, "metrics.buttonSize", 16.0f));
		(void)xuiScrollBarSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255)),
			__uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255)),
			__uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(142, 166, 192, 255)),
			__uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(111, 137, 166, 255)),
			__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
			__uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(206, 215, 226, 255)));
		(void)xuiScrollBarSetButtonColors(pWidget,
			__uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(239, 245, 252, 255)),
			__uiDesignColor(pNode, "appearance.buttonIconColor", XUI_COLOR_RGBA(52, 71, 93, 255)));
		break;
	case UI_DESIGN_NODE_SLIDER:
		(void)xuiSliderSetRange(pWidget, __uiDesignFloat(pNode, "value.min", 0.0f), __uiDesignFloat(pNode, "value.max", 100.0f));
		(void)xuiSliderSetValue(pWidget, __uiDesignFloat(pNode, "value.value", 35.0f));
		(void)xuiSliderSetStep(pWidget, __uiDesignFloat(pNode, "value.step", 1.0f), __uiDesignFloat(pNode, "value.pageStep", 10.0f));
		(void)xuiSliderSetOrientation(pWidget, __uiDesignInt(pNode, "behavior.orientation", XUI_ORIENTATION_HORIZONTAL));
		(void)xuiSliderSetMetrics(pWidget, __uiDesignFloat(pNode, "metrics.trackSize", 4.0f), __uiDesignFloat(pNode, "metrics.knobSize", 16.0f), __uiDesignFloat(pNode, "metrics.trackRadius", 2.0f));
		(void)xuiSliderSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(213, 222, 232, 255)),
			__uiDesignColor(pNode, "appearance.fillColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.knobColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
			__uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(183, 195, 208, 255)));
		(void)xuiSliderSetKnobBorderColor(pWidget, __uiDesignColor(pNode, "appearance.knobBorderColor", XUI_COLOR_RGBA(142, 166, 192, 255)));
		break;
	case UI_DESIGN_NODE_RANGE_SLIDER:
		(void)xuiRangeSliderSetRange(pWidget, __uiDesignFloat(pNode, "value.min", 0.0f), __uiDesignFloat(pNode, "value.max", 100.0f));
		(void)xuiRangeSliderSetValues(pWidget, __uiDesignFloat(pNode, "value.start", 25.0f), __uiDesignFloat(pNode, "value.end", 75.0f));
		(void)xuiRangeSliderSetStep(pWidget, __uiDesignFloat(pNode, "value.step", 1.0f), __uiDesignFloat(pNode, "value.pageStep", 10.0f));
		(void)xuiRangeSliderSetIntervalLimits(pWidget, __uiDesignFloat(pNode, "value.minInterval", 0.0f), __uiDesignFloat(pNode, "value.maxInterval", 0.0f));
		(void)xuiRangeSliderSetOrientation(pWidget, __uiDesignInt(pNode, "behavior.orientation", XUI_ORIENTATION_HORIZONTAL));
		(void)xuiRangeSliderSetMetrics(pWidget, __uiDesignFloat(pNode, "metrics.trackSize", 4.0f), __uiDesignFloat(pNode, "metrics.knobSize", 16.0f));
		(void)xuiRangeSliderSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(213, 222, 232, 255)),
			__uiDesignColor(pNode, "appearance.fillColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.knobColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
			__uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(183, 195, 208, 255)));
		(void)xuiRangeSliderSetKnobBorderColor(pWidget, __uiDesignColor(pNode, "appearance.knobBorderColor", XUI_COLOR_RGBA(142, 166, 192, 255)));
		break;
	case UI_DESIGN_NODE_PAGE:
		if ( __uiDesignBool(pNode, "behavior.useTotal", 0) ) {
			(void)xuiPageSetTotal(pWidget, __uiDesignInt(pNode, "data.totalCount", 120), __uiDesignInt(pNode, "data.pageSize", 10));
		} else {
			(void)xuiPageSetPageCount(pWidget, __uiDesignInt(pNode, "data.pageCount", 12));
		}
		(void)xuiPageSetCurrent(pWidget, __uiDesignInt(pNode, "data.current", 1), 0);
		(void)xuiPageSetWindowSize(pWidget, __uiDesignInt(pNode, "data.windowSize", 5));
		(void)xuiPageSetText(pWidget, __uiDesignText(pNode, "text.first", "<<"), __uiDesignText(pNode, "text.last", ">>"), __uiDesignText(pNode, "text.prev", "<"), __uiDesignText(pNode, "text.next", ">"));
		(void)xuiPageSetMetrics(pWidget,
			__uiDesignFloat(pNode, "metrics.itemHeight", 28.0f),
			__uiDesignFloat(pNode, "metrics.pageWidth", 28.0f),
			__uiDesignFloat(pNode, "metrics.textWidth", 42.0f),
			__uiDesignFloat(pNode, "metrics.navWidth", 36.0f),
			__uiDesignFloat(pNode, "metrics.ellipsisWidth", 24.0f));
		(void)xuiPageSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255)),
			__uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(217, 234, 254, 255)),
			__uiDesignColor(pNode, "appearance.currentColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.currentTextColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)));
		(void)xuiPageSetFocusColor(pWidget, __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)));
		break;
	case UI_DESIGN_NODE_CAROUSEL:
		__uiDesignDetachContainerModelChildren(pApp, pNode);
		(void)xuiCarouselSetPageCount(pWidget, __uiDesignInt(pNode, "data.pageCount", 3));
		(void)xuiCarouselSetCurrent(pWidget, __uiDesignInt(pNode, "data.current", 0), 0);
		(void)xuiCarouselSetLoop(pWidget, __uiDesignBool(pNode, "behavior.loop", 1));
		(void)xuiCarouselSetAutoPlay(pWidget, __uiDesignBool(pNode, "behavior.autoPlay", 0), __uiDesignFloat(pNode, "behavior.autoInterval", 3.0f));
		(void)xuiCarouselSetIndicatorsVisible(pWidget, __uiDesignBool(pNode, "behavior.showIndicators", 1));
		(void)xuiCarouselSetArrowsOnHover(pWidget, __uiDesignBool(pNode, "behavior.arrowsOnHover", 1));
		(void)xuiCarouselSetMetrics(pWidget,
			__uiDesignFloat(pNode, "metrics.arrowSize", 32.0f),
			__uiDesignFloat(pNode, "metrics.indicatorSize", 8.0f),
			__uiDesignFloat(pNode, "metrics.indicatorGap", 8.0f),
			__uiDesignFloat(pNode, "metrics.indicatorBottom", 12.0f));
		(void)xuiCarouselSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 250, 254, 255)),
			__uiDesignColor(pNode, "appearance.arrowColor", XUI_COLOR_RGBA(90, 109, 130, 170)),
			__uiDesignColor(pNode, "appearance.arrowHoverColor", XUI_COLOR_RGBA(52, 71, 93, 204)),
			__uiDesignColor(pNode, "appearance.arrowTextColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.indicatorColor", XUI_COLOR_RGBA(170, 184, 200, 255)),
			__uiDesignColor(pNode, "appearance.indicatorActiveColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.indicatorHoverColor", XUI_COLOR_RGBA(109, 164, 232, 255)));
		(void)xuiCarouselSetFocusColor(pWidget, __uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)));
		(void)__uiDesignAttachContainerModelChildren(pApp, pNode);
		break;
	case UI_DESIGN_NODE_COMBOBOX:
		__uiDesignTextPoolReset(pNode);
		iTextItemCount = __uiDesignBuildComboBoxItems(pNode, arrComboItems, UI_DESIGN_COUNT_OF(arrComboItems), &iComboSelectedIndex, &iComboSelectedValue);
		(void)xuiComboBoxSetItemData(pWidget, arrComboItems, iTextItemCount);
		if ( __uiDesignBool(pNode, "behavior.useValue", 0) ) {
			(void)xuiComboBoxSetSelectedValue(pWidget, (iComboSelectedIndex >= 0) ? iComboSelectedValue : __uiDesignInt(pNode, "data.selectedValue", 0));
		} else {
			(void)xuiComboBoxSetSelected(pWidget, (iComboSelectedIndex >= 0) ? iComboSelectedIndex : __uiDesignInt(pNode, "data.selected", 0));
		}
		(void)xuiComboBoxSetMode(pWidget, __uiDesignInt(pNode, "behavior.mode", XUI_COMBOBOX_MODE_SELECT));
		if ( __uiDesignInt(pNode, "behavior.mode", XUI_COMBOBOX_MODE_SELECT) == XUI_COMBOBOX_MODE_EDIT ) {
			(void)xuiComboBoxSetText(pWidget, __uiDesignText(pNode, "text.inputText", ""));
		}
		(void)xuiComboBoxSetPlaceholder(pWidget, __uiDesignText(pNode, "text.placeholder", "Select or type"));
		(void)xuiComboBoxSetMaxLength(pWidget, __uiDesignInt(pNode, "text.maxLength", 0));
		(void)xuiComboBoxSetMetrics(pWidget, __uiDesignFloat(pNode, "metrics.itemHeight", 24.0f), __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f));
		(void)xuiComboBoxSetPopupHeight(pWidget, __uiDesignFloat(pNode, "metrics.popupHeight", 0.0f));
		(void)xuiComboBoxSetPopupMaxHeight(pWidget, __uiDesignFloat(pNode, "metrics.popupMaxHeight", 240.0f));
		(void)xuiComboBoxSetPopupPlacement(pWidget, __uiDesignInt(pNode, "behavior.popupPlacement", XUI_COMBOBOX_POPUP_AUTO));
		(void)xuiComboBoxSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)),
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(240, 246, 253, 255)),
			__uiDesignColor(pNode, "appearance.openBackgroundColor", XUI_COLOR_RGBA(231, 241, 252, 255)),
			__uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(238, 242, 247, 255)));
		(void)xuiComboBoxSetBorderColors(pWidget,
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(126, 164, 207, 255)),
			__uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255)));
		(void)xuiComboBoxSetArrowColors(pWidget,
			__uiDesignColor(pNode, "appearance.arrowColor", XUI_COLOR_RGBA(52, 71, 93, 255)),
			__uiDesignColor(pNode, "appearance.disabledArrowColor", XUI_COLOR_RGBA(154, 168, 186, 255)));
		(void)xuiComboBoxSetButtonColors(pWidget,
			__uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(236, 246, 253, 255)),
			__uiDesignColor(pNode, "appearance.buttonHoverColor", XUI_COLOR_RGBA(220, 238, 251, 255)),
			__uiDesignColor(pNode, "appearance.buttonOpenColor", XUI_COLOR_RGBA(207, 229, 247, 255)));
		(void)xuiComboBoxSetPopupColors(pWidget,
			__uiDesignColor(pNode, "appearance.popupPanelColor", XUI_COLOR_RGBA(250, 253, 255, 255)),
			__uiDesignColor(pNode, "appearance.popupBorderColor", XUI_COLOR_RGBA(122, 164, 202, 255)),
			__uiDesignColor(pNode, "appearance.popupShadowColor", XUI_COLOR_RGBA(44, 70, 96, 46)),
			__uiDesignColor(pNode, "appearance.popupHoverColor", XUI_COLOR_RGBA(54, 125, 190, 255)),
			__uiDesignColor(pNode, "appearance.popupTextColor", XUI_COLOR_RGBA(28, 60, 94, 255)),
			__uiDesignColor(pNode, "appearance.popupHoverTextColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.popupDisabledTextColor", XUI_COLOR_RGBA(142, 152, 166, 210)),
			__uiDesignColor(pNode, "appearance.popupSeparatorColor", XUI_COLOR_RGBA(202, 218, 232, 255)));
		(void)__uiDesignApplyInputMenuTitles(pNode, pWidget, UI_DESIGN_INPUT_MENU_TARGET_COMBOBOX);
		if ( __uiDesignBool(pNode, "behavior.open", 0) ) (void)xuiComboBoxOpen(pWidget);
		else (void)xuiComboBoxClose(pWidget);
		break;
	case UI_DESIGN_NODE_LISTVIEW:
		__uiDesignTextPoolReset(pNode);
		iTextItemCount = __uiDesignBuildListViewItems(pNode, arrTextItems, arrListEnabled, arrListSelected, UI_DESIGN_COUNT_OF(arrTextItems), &bListExplicitSelection);
		(void)xuiListViewSetItems(pWidget, arrTextItems, iTextItemCount);
		(void)xuiListViewSetSelectionMode(pWidget, __uiDesignInt(pNode, "behavior.selectionMode", XUI_SELECTION_SINGLE));
		(void)xuiListViewSetEnabledItems(pWidget, arrListEnabled, iTextItemCount);
		if ( bListExplicitSelection ) {
			(void)xuiListViewClearSelection(pWidget);
			for ( i = 0; i < iTextItemCount; ++i ) {
				if ( arrListSelected[i] ) (void)xuiListViewSetItemSelected(pWidget, i, 1);
			}
		} else {
			(void)xuiListViewSetSelected(pWidget, __uiDesignInt(pNode, "data.selected", 0));
		}
		(void)xuiListViewSetNotifyRepeatSelect(pWidget, __uiDesignBool(pNode, "behavior.repeatSelect", 0));
		(void)xuiListViewSetScrollbarMode(pWidget, __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL));
		(void)xuiListViewSetScroll(pWidget, __uiDesignFloat(pNode, "value.scrollY", 0.0f));
		(void)xuiListViewSetMetrics(pWidget, __uiDesignFloat(pNode, "metrics.itemHeight", 24.0f), __uiDesignFloat(pNode, "metrics.padding", 8.0f), __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f));
		(void)xuiListViewSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
			__uiDesignColor(pNode, "appearance.rowColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255)),
			__uiDesignColor(pNode, "appearance.selectedColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)));
		(void)xuiListViewSetScrollbarColors(pWidget,
			__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255)),
			__uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarHoverColor", XUI_COLOR_RGBA(142, 166, 192, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarActiveColor", XUI_COLOR_RGBA(111, 137, 166, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarFocusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarDisabledColor", XUI_COLOR_RGBA(206, 215, 226, 255)));
		break;
	case UI_DESIGN_NODE_TREEVIEW:
		__uiDesignApplyTreeDefaults(pNode);
		(void)xuiTreeViewSetSelected(pWidget, __uiDesignInt(pNode, "data.selected", 1));
		(void)xuiTreeViewSetScrollbarMode(pWidget, __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL));
		(void)xuiTreeViewSetScroll(pWidget, __uiDesignFloat(pNode, "value.scrollY", 0.0f));
		(void)xuiTreeViewSetMetrics(pWidget, __uiDesignFloat(pNode, "metrics.itemHeight", 23.0f), __uiDesignFloat(pNode, "metrics.indent", 18.0f), __uiDesignFloat(pNode, "metrics.padding", 8.0f), __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f));
		(void)xuiTreeViewSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
			__uiDesignColor(pNode, "appearance.rowColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(234, 243, 255, 255)),
			__uiDesignColor(pNode, "appearance.selectedColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)));
		(void)xuiTreeViewSetDecorationColors(pWidget,
			__uiDesignColor(pNode, "appearance.expanderColor", XUI_COLOR_RGBA(82, 101, 122, 255)),
			__uiDesignColor(pNode, "appearance.iconColor", XUI_COLOR_RGBA(122, 142, 165, 255)),
			__uiDesignColor(pNode, "appearance.checkColor", XUI_COLOR_RGBA(49, 126, 214, 255)));
		(void)xuiTreeViewSetScrollbarColors(pWidget,
			__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255)),
			__uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarHoverColor", XUI_COLOR_RGBA(142, 166, 192, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarActiveColor", XUI_COLOR_RGBA(111, 137, 166, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarFocusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarDisabledColor", XUI_COLOR_RGBA(206, 215, 226, 255)));
		break;
	case UI_DESIGN_NODE_TABLEVIEW:
		__uiDesignApplyTableDefaults(pNode);
		(void)xuiTableViewSetDefaultMetrics(pWidget, __uiDesignFloat(pNode, "metrics.columnWidth", 100.0f), __uiDesignFloat(pNode, "metrics.rowHeight", 24.0f), __uiDesignFloat(pNode, "metrics.headerHeight", 26.0f));
		__uiDesignApplyTableSelection(pWidget, pNode);
		(void)xuiTableViewSetScrollbarMode(pWidget, __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL));
		(void)xuiTableViewSetOffset(pWidget, __uiDesignFloat(pNode, "value.offsetX", 0.0f), __uiDesignFloat(pNode, "value.offsetY", 0.0f));
		__uiDesignFillTableColors(pNode, &tTableColors);
		(void)xuiTableViewSetColorStyle(pWidget, &tTableColors);
		(void)xuiTableViewSetScrollbarColors(pWidget,
			__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255)),
			__uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarHoverColor", XUI_COLOR_RGBA(142, 166, 192, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarActiveColor", XUI_COLOR_RGBA(111, 137, 166, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarFocusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
			__uiDesignColor(pNode, "appearance.scrollbarDisabledColor", XUI_COLOR_RGBA(206, 215, 226, 255)));
		break;
	case UI_DESIGN_NODE_TABLEGRID:
		__uiDesignApplyTableGridDefaults(pNode);
		(void)xuiTableGridSetEditorConfig(pWidget, __uiDesignTableGridEditorConfig, pNode);
		(void)xuiTableGridSetDefaultMetrics(pWidget, __uiDesignFloat(pNode, "metrics.columnWidth", 100.0f), __uiDesignFloat(pNode, "metrics.rowHeight", 24.0f), __uiDesignFloat(pNode, "metrics.headerHeight", 26.0f));
		(void)xuiTableGridSetSelectionMode(pWidget, __uiDesignInt(pNode, "behavior.selectionMode", XUI_TABLE_VIEW_SELECTION_ROW));
		(void)xuiTableGridSetScrollbarMode(pWidget, __uiDesignInt(pNode, "behavior.scrollbarMode", XUI_SCROLLBAR_MODE_FULL));
		(void)xuiTableGridSetEditMode(pWidget, __uiDesignInt(pNode, "behavior.editMode", XUI_TABLE_GRID_EDIT_QUICK));
		__uiDesignFillTableColors(pNode, &tTableColors);
		(void)xuiTableGridSetColorStyle(pWidget, &tTableColors);
		pTableWidget = xuiTableGridGetTableView(pWidget);
		if ( pTableWidget != NULL ) {
			__uiDesignApplyTableSelection(pTableWidget, pNode);
			(void)xuiTableViewSetOffset(pTableWidget, __uiDesignFloat(pNode, "value.offsetX", 0.0f), __uiDesignFloat(pNode, "value.offsetY", 0.0f));
			(void)xuiTableViewSetScrollbarColors(pTableWidget,
				__uiDesignColor(pNode, "appearance.trackColor", XUI_COLOR_RGBA(228, 235, 243, 255)),
				__uiDesignColor(pNode, "appearance.thumbColor", XUI_COLOR_RGBA(170, 184, 200, 255)),
				__uiDesignColor(pNode, "appearance.scrollbarHoverColor", XUI_COLOR_RGBA(142, 166, 192, 255)),
				__uiDesignColor(pNode, "appearance.scrollbarActiveColor", XUI_COLOR_RGBA(111, 137, 166, 255)),
				__uiDesignColor(pNode, "appearance.scrollbarFocusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
				__uiDesignColor(pNode, "appearance.scrollbarDisabledColor", XUI_COLOR_RGBA(206, 215, 226, 255)));
		}
		break;
	case UI_DESIGN_NODE_QRCODE:
		(void)xuiQrCodeSetValue(pWidget, __uiDesignText(pNode, "data.value", "https://xui.local"));
		(void)xuiQrCodeSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.foregroundColor", XUI_COLOR_RGBA(17, 24, 39, 255)),
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE));
		(void)xuiQrCodeSetPadding(pWidget, __uiDesignFloat(pNode, "metrics.padding", 8.0f));
		(void)xuiQrCodeSetVersionRange(pWidget, __uiDesignInt(pNode, "behavior.minVersion", XUI_QRCODE_MIN_VERSION), __uiDesignInt(pNode, "behavior.maxVersion", XUI_QRCODE_MAX_VERSION));
		(void)__uiDesignApplyQrCodeIcon(pApp, pNode, pWidget);
		break;
	case UI_DESIGN_NODE_IMAGE:
		(void)__uiDesignApplyImageSurface(pApp, pNode, pWidget);
		(void)xuiImageSetColor(pWidget, __uiDesignColor(pNode, "appearance.tintColor", XUI_COLOR_WHITE));
		(void)xuiImageSetAlign(pWidget, __uiDesignInt(pNode, "behavior.alignX", XUI_ALIGN_CENTER), __uiDesignInt(pNode, "behavior.alignY", XUI_ALIGN_CENTER));
		__uiDesignApplyImageRects(pNode, pWidget);
		(void)xuiImageSetMode(pWidget, __uiDesignInt(pNode, "behavior.mode", XUI_IMAGE_CONTAIN));
		break;
	case UI_DESIGN_NODE_BREADCRUMB:
		__uiDesignTextPoolReset(pNode);
		iBreadcrumbItemCount = __uiDesignBuildBreadcrumbItems(pNode, arrBreadcrumbItems, UI_DESIGN_COUNT_OF(arrBreadcrumbItems));
		(void)xuiBreadcrumbSetItems(pWidget, arrBreadcrumbItems, iBreadcrumbItemCount);
		(void)xuiBreadcrumbSetSeparator(pWidget, __uiDesignText(pNode, "text.separator", "/"));
		(void)__uiDesignApplyBreadcrumbSeparatorIcon(pApp, pNode, pWidget);
		(void)xuiBreadcrumbSetTextColors(pWidget,
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.hoverTextColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.activeTextColor", XUI_COLOR_RGBA(31, 95, 168, 255)),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)));
		(void)xuiBreadcrumbSetSeparatorColor(pWidget, __uiDesignColor(pNode, "appearance.separatorColor", XUI_COLOR_RGBA(107, 127, 149, 255)));
		(void)xuiBreadcrumbSetBackgroundColor(pWidget, __uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(0, 0, 0, 0)));
		(void)xuiBreadcrumbSetMetrics(pWidget,
			__uiDesignFloat(pNode, "metrics.gap", 6.0f),
			__uiDesignFloat(pNode, "metrics.paddingX", 8.0f),
			__uiDesignFloat(pNode, "metrics.paddingY", 4.0f));
		break;
	case UI_DESIGN_NODE_CHECK_CARD:
		(void)xuiCheckCardSetChecked(pWidget, __uiDesignBool(pNode, "checked", pNode->bChecked));
		(void)xuiWidgetSetMinSize(pWidget, (xui_vec2_t){
			__uiDesignFloat(pNode, "metrics.minWidth", 96.0f),
			__uiDesignFloat(pNode, "metrics.minHeight", 56.0f)
		});
		(void)xuiWidgetSetPadding(pWidget, (xui_thickness_t){
			__uiDesignFloat(pNode, "metrics.paddingLeft", 16.0f),
			__uiDesignFloat(pNode, "metrics.paddingTop", 14.0f),
			__uiDesignFloat(pNode, "metrics.paddingRight", 16.0f),
			__uiDesignFloat(pNode, "metrics.paddingBottom", 14.0f)
		});
		(void)xuiCheckCardSetMetrics(pWidget,
			__uiDesignFloat(pNode, "metrics.borderWidth", 1.0f),
			__uiDesignFloat(pNode, "metrics.checkedBorderWidth", 2.0f),
			__uiDesignFloat(pNode, "metrics.cornerSize", 18.0f),
			__uiDesignFloat(pNode, "metrics.focusWidth", 2.0f));
		(void)xuiCheckCardSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(240, 246, 253, 255)),
			__uiDesignColor(pNode, "appearance.activeBackgroundColor", XUI_COLOR_RGBA(226, 240, 255, 255)),
			__uiDesignColor(pNode, "appearance.checkedBackgroundColor", XUI_COLOR_RGBA(244, 249, 255, 255)),
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.checkedBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.cornerColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.checkColor", XUI_COLOR_WHITE));
		(void)xuiCheckCardSetStateColors(pWidget,
			__uiDesignColor(pNode, "appearance.disabledBorderColor", XUI_COLOR_RGBA(214, 219, 225, 160)),
			__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)));
		break;
	case UI_DESIGN_NODE_RADIO_GROUP:
		(void)xuiRadioGroupSetOrientation(pWidget, __uiDesignRadioGroupOrientation(pNode));
		(void)xuiRadioGroupSetGap(pWidget, __uiDesignFloat(pNode, "metrics.gap", 8.0f));
		if ( __uiDesignContainerHasModelChildren(pApp, pNode) ) {
			__uiDesignDetachContainerModelChildren(pApp, pNode);
			__uiDesignClearChildren(pWidget);
			(void)__uiDesignAttachContainerModelChildren(pApp, pNode);
			(void)xuiRadioGroupSetSelectedIndex(pWidget, __uiDesignInt(pNode, "data.selected", 0));
		} else {
			(void)__uiDesignApplyRadioGroupOptions(pApp, pNode, pWidget);
			(void)xuiRadioGroupSetSelectedIndex(pWidget, __uiDesignInt(pNode, "data.selected", 0));
		}
		break;
	case UI_DESIGN_NODE_VIRTUAL_JOYSTICK:
		(void)__uiDesignApplyVirtualJoystickChannels(pNode, pWidget);
		(void)xuiVirtualJoystickSetMetrics(pWidget,
			__uiDesignFloat(pNode, "metrics.radius", 58.0f),
			__uiDesignFloat(pNode, "metrics.knobSize", 34.0f),
			__uiDesignFloat(pNode, "metrics.deadZone", 0.1f));
		(void)xuiVirtualJoystickUseBuiltinAtlas(pWidget, __uiDesignBool(pNode, "behavior.useBuiltinAtlas", 1));
		(void)__uiDesignApplyVirtualJoystickSurfaces(pApp, pNode);
		(void)xuiVirtualJoystickSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.baseColor", XUI_COLOR_RGBA(226, 234, 243, 255)),
			__uiDesignColor(pNode, "appearance.baseActiveColor", XUI_COLOR_RGBA(207, 227, 250, 255)),
			__uiDesignColor(pNode, "appearance.knobColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.knobActiveColor", XUI_COLOR_RGBA(247, 251, 255, 255)),
			__uiDesignColor(pNode, "appearance.rippleColor", XUI_COLOR_RGBA(49, 126, 214, 51)),
			__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
			__uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(183, 195, 208, 255)));
		break;
	case UI_DESIGN_NODE_INVENTORY_GRID:
		__uiDesignApplyInventoryDefaults(pApp, pNode);
		break;
	case UI_DESIGN_NODE_TERMINAL:
		(void)xuiTerminalSetScrollbackLimit(pWidget, __uiDesignInt(pNode, "metrics.scrollback", 1000));
		(void)xuiTerminalSetMetrics(pWidget,
			__uiDesignFloat(pNode, "metrics.cellWidth", 0.0f),
			__uiDesignFloat(pNode, "metrics.cellHeight", 0.0f),
			__uiDesignFloat(pNode, "metrics.padding", 8.0f));
		(void)xuiTerminalSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(16, 24, 32, 255)),
			__uiDesignColor(pNode, "appearance.foregroundColor", XUI_COLOR_RGBA(215, 228, 242, 255)),
			__uiDesignColor(pNode, "appearance.cursorColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.selectionColor", XUI_COLOR_RGBA(49, 126, 214, 128)),
			__uiDesignColor(pNode, "appearance.selectionTextColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.searchHighlightColor", XUI_COLOR_RGBA(255, 197, 66, 110)),
			__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
			__uiDesignColor(pNode, "appearance.linkHoverColor", XUI_COLOR_RGBA(86, 156, 214, 255)));
		__uiDesignApplyTerminalPalette(pNode, pWidget);
		(void)xuiTerminalSetScrollbackLimit(pWidget, __uiDesignInt(pNode, "metrics.scrollback", 1000));
		(void)xuiTerminalSetMetrics(pWidget,
			__uiDesignFloat(pNode, "metrics.cellWidth", 0.0f),
			__uiDesignFloat(pNode, "metrics.cellHeight", 0.0f),
			__uiDesignFloat(pNode, "metrics.padding", 8.0f));
		(void)xuiTerminalResize(pWidget, __uiDesignInt(pNode, "metrics.columns", 80), __uiDesignInt(pNode, "metrics.rows", 24));
		(void)xuiTerminalClear(pWidget);
		(void)xuiTerminalWriteText(pWidget, __uiDesignText(pNode, "data.text", "$ xui_uidesign\nready"));
		(void)xuiTerminalFlush(pWidget);
		(void)xuiTerminalSetParseBudget(pWidget, __uiDesignInt(pNode, "metrics.parseBudget", 4096));
		(void)xuiTerminalSetBracketedPaste(pWidget, __uiDesignBool(pNode, "behavior.bracketedPaste", 0));
		(void)xuiTerminalSetLigaturesEnabled(pWidget, __uiDesignBool(pNode, "behavior.ligaturesEnabled", 0));
		(void)__uiDesignApplyTerminalMenuTitles(pNode, pWidget);
		__uiDesignApplyTerminalSelection(pNode, pWidget);
		__uiDesignApplyTerminalFind(pNode, pWidget);
		__uiDesignApplyTerminalScroll(pNode, pWidget);
		break;
	case UI_DESIGN_NODE_SPLIT_LAYOUT:
		__uiDesignDetachContainerModelChildren(pApp, pNode);
		(void)xuiSplitLayoutSetOrientation(pWidget, __uiDesignInt(pNode, "behavior.orientation", XUI_ORIENTATION_VERTICAL));
		(void)xuiSplitLayoutSetDividerMetrics(pWidget,
			__uiDesignFloat(pNode, "metrics.dividerSize", 6.0f),
			__uiDesignFloat(pNode, "metrics.dividerVisualSize", 1.0f),
			__uiDesignFloat(pNode, "metrics.dividerHitSize", 8.0f));
		(void)xuiSplitLayoutSetShadowDrag(pWidget, __uiDesignBool(pNode, "behavior.shadowDrag", 1));
		(void)xuiSplitLayoutSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.dividerColor", XUI_COLOR_RGBA(202, 214, 228, 255)),
			__uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(142, 166, 192, 255)),
			__uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.shadowColor", XUI_COLOR_RGBA(49, 126, 214, 51)));
		(void)__uiDesignApplySplitLayoutPanes(pApp, pNode);
		(void)__uiDesignAttachContainerModelChildren(pApp, pNode);
		break;
	case UI_DESIGN_NODE_TABS:
		__uiDesignDetachContainerModelChildren(pApp, pNode);
		__uiDesignTextPoolReset(pNode);
		iTabItemCount = __uiDesignBuildTabsItems(pApp, pNode, arrTabItems, arrTabEnabled, arrTabDirty, arrTabIcons, arrTabIconSrc, UI_DESIGN_COUNT_OF(arrTabItems));
		(void)xuiTabsSetItems(pWidget, arrTabItems, iTabItemCount);
		(void)xuiTabsSetEnabledItems(pWidget, arrTabEnabled, iTabItemCount);
		(void)xuiTabsSetDirtyItems(pWidget, arrTabDirty, iTabItemCount);
		(void)xuiTabsSetIcons(pWidget, arrTabIcons, arrTabIconSrc, iTabItemCount);
		(void)xuiTabsSetSelected(pWidget, __uiDesignInt(pNode, "data.selected", 0));
		(void)xuiTabsSetPlacement(pWidget, __uiDesignInt(pNode, "behavior.placement", XUI_TABS_PLACEMENT_TOP));
		(void)xuiTabsSetScrollable(pWidget, __uiDesignBool(pNode, "behavior.scrollable", 1));
		(void)xuiTabsSetScroll(pWidget, __uiDesignFloat(pNode, "value.scrollX", 0.0f));
		(void)xuiTabsSetClose(pWidget, __uiDesignTabsCloseNoop, __uiDesignBool(pNode, "behavior.closeButtons", 1), NULL);
		(void)xuiTabsSetTabSize(pWidget, __uiDesignFloat(pNode, "metrics.tabWidth", 92.0f), __uiDesignFloat(pNode, "metrics.tabHeight", 30.0f));
		(void)xuiTabsSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.tabColor", XUI_COLOR_RGBA(234, 241, 248, 255)),
			__uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(244, 249, 255, 255)),
			__uiDesignColor(pNode, "appearance.activeColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.focusColor", XUI_COLOR_RGBA(91, 155, 232, 255)),
			__uiDesignColor(pNode, "appearance.disabledColor", XUI_COLOR_RGBA(238, 242, 247, 255)),
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.activeTextColor", XUI_COLOR_RGBA(31, 95, 168, 255)));
		(void)xuiTabsSetFrameColors(pWidget,
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.clientColor", XUI_COLOR_WHITE));
		(void)__uiDesignAttachContainerModelChildren(pApp, pNode);
		break;
	case UI_DESIGN_NODE_ACCORDION:
		__uiDesignDetachContainerModelChildren(pApp, pNode);
		__uiDesignTextPoolReset(pNode);
		iAccordionSectionCount = __uiDesignBuildAccordionSections(pNode, arrAccordionSections, UI_DESIGN_COUNT_OF(arrAccordionSections));
		(void)xuiAccordionClear(pWidget);
		for ( i = 0; i < iAccordionSectionCount; i++ ) {
			(void)xuiAccordionAddSection(pWidget, arrAccordionSections[i].sTitle, arrAccordionSections[i].iId, arrAccordionSections[i].bExpanded, NULL);
			if ( arrAccordionSections[i].bDisabled ) {
				(void)xuiAccordionSetSectionEnabled(pWidget, i, 0);
			}
		}
		(void)xuiAccordionSetMode(pWidget, __uiDesignInt(pNode, "behavior.mode", XUI_ACCORDION_MODE_MULTIPLE));
		(void)xuiAccordionSetMetrics(pWidget,
			__uiDesignFloat(pNode, "metrics.headerHeight", 28.0f),
			__uiDesignFloat(pNode, "metrics.spacing", 4.0f),
			__uiDesignFloat(pNode, "metrics.contentPadding", 8.0f));
		(void)xuiAccordionSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.headerColor", XUI_COLOR_RGBA(234, 241, 248, 255)),
			__uiDesignColor(pNode, "appearance.hoverColor", XUI_COLOR_RGBA(244, 249, 255, 255)),
			__uiDesignColor(pNode, "appearance.expandedColor", XUI_COLOR_RGBA(217, 234, 254, 255)),
			__uiDesignColor(pNode, "appearance.contentColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.activeTextColor", XUI_COLOR_RGBA(31, 95, 168, 255)),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)));
		(void)__uiDesignAttachContainerModelChildren(pApp, pNode);
		break;
	case UI_DESIGN_NODE_WINDOW:
		(void)xuiWindowSetTitle(pWidget, __uiDesignText(pNode, "text.title", "Window"));
		/* UIDesign embeds windows on the artboard; runtime open would reparent to the overlay layer. */
		(void)xuiWidgetSetVisible(pWidget, pNode->bVisible && __uiDesignBool(pNode, "behavior.open", 1));
		(void)xuiWindowSetTopMost(pWidget, __uiDesignBool(pNode, "behavior.topMost", 0));
		(void)xuiWindowSetShowTitleBar(pWidget, __uiDesignBool(pNode, "behavior.showTitleBar", 1));
		(void)xuiWindowSetMovable(pWidget, __uiDesignBool(pNode, "behavior.movable", 1));
		(void)xuiWindowSetDragAnywhere(pWidget, __uiDesignBool(pNode, "behavior.dragAnywhere", 0));
		(void)xuiWindowSetResizable(pWidget, __uiDesignBool(pNode, "behavior.resizable", 1));
		(void)xuiWindowSetResizeEdges(pWidget, (uint32_t)__uiDesignInt(pNode, "behavior.resizeEdges", XUI_WINDOW_EDGE_ALL));
		(void)xuiWindowSetShowCollapse(pWidget, __uiDesignBool(pNode, "behavior.showCollapse", 1));
		(void)xuiWindowSetShowMaximize(pWidget, __uiDesignBool(pNode, "behavior.showMaximize", 1));
		(void)xuiWindowSetShowClose(pWidget, __uiDesignBool(pNode, "behavior.showClose", 1));
		(void)xuiWindowSetCollapsed(pWidget, __uiDesignBool(pNode, "behavior.collapsed", 0));
		(void)xuiWindowSetMaximized(pWidget, __uiDesignBool(pNode, "behavior.maximized", 0));
		(void)xuiWindowSetChrome(pWidget,
			__uiDesignFloat(pNode, "metrics.titleBarHeight", 30.0f),
			__uiDesignFloat(pNode, "metrics.borderWidth", 1.0f),
			__uiDesignFloat(pNode, "metrics.resizeGrip", 6.0f),
			__uiDesignFloat(pNode, "metrics.buttonSize", 24.0f));
		(void)xuiWindowSetIconSize(pWidget, __uiDesignFloat(pNode, "metrics.iconSize", 16.0f));
		tSrc = (xui_rect_t){
			__uiDesignFloat(pNode, "icon.x", 0.0f),
			__uiDesignFloat(pNode, "icon.y", 0.0f),
			__uiDesignFloat(pNode, "icon.w", 0.0f),
			__uiDesignFloat(pNode, "icon.h", 0.0f)
		};
		if ( __uiDesignLoadRuntimeSurface(pApp, pNode, __uiDesignText(pNode, "data.iconSource", ""), &pSurface) == XUI_OK ) {
			(void)xuiWindowSetIcon(pWidget, pSurface, tSrc);
		}
		(void)xuiWindowSetMinSize(pWidget, __uiDesignFloat(pNode, "metrics.minWidth", 120.0f), __uiDesignFloat(pNode, "metrics.minHeight", 80.0f));
		(void)xuiWindowSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_RGBA(247, 250, 254, 255)),
			__uiDesignColor(pNode, "appearance.clientColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.titleBarColor", XUI_COLOR_RGBA(221, 235, 250, 255)),
			__uiDesignColor(pNode, "appearance.titleTextColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(142, 166, 192, 255)),
			__uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(0, 0, 0, 0)),
			__uiDesignColor(pNode, "appearance.buttonHoverColor", XUI_COLOR_RGBA(188, 215, 245, 255)),
			__uiDesignColor(pNode, "appearance.buttonActiveColor", XUI_COLOR_RGBA(167, 201, 238, 255)));
		(void)xuiWindowSetStateColors(pWidget,
			__uiDesignColor(pNode, "appearance.inactiveTitleBarColor", XUI_COLOR_RGBA(236, 245, 251, 255)),
			__uiDesignColor(pNode, "appearance.inactiveTitleTextColor", XUI_COLOR_RGBA(102, 123, 142, 255)),
			__uiDesignColor(pNode, "appearance.activeBorderColor", XUI_COLOR_RGBA(47, 128, 237, 255)),
			__uiDesignColor(pNode, "appearance.closeHoverColor", XUI_COLOR_RGBA(255, 226, 226, 255)),
			__uiDesignColor(pNode, "appearance.closeActiveColor", XUI_COLOR_RGBA(244, 205, 205, 255)));
		iRet = __uiDesignRefreshContainerModelChildren(pApp, pNode);
		if ( iRet != XUI_OK ) return iRet;
		break;
	case UI_DESIGN_NODE_SCROLL_FRAME:
		__uiDesignApplyScrollFrame(pNode);
		iRet = __uiDesignRefreshContainerModelChildren(pApp, pNode);
		if ( iRet != XUI_OK ) return iRet;
		break;
	case UI_DESIGN_NODE_SCROLL_VIEW:
		__uiDesignApplyScrollView(pNode);
		iRet = __uiDesignRefreshContainerModelChildren(pApp, pNode);
		if ( iRet != XUI_OK ) return iRet;
		break;
	case UI_DESIGN_NODE_CANVAS:
		__uiDesignApplyCanvasPreview(pApp, pNode);
		break;
	case UI_DESIGN_NODE_MESSAGE_LIST:
		__uiDesignApplyMessageDefaults(pNode);
		break;
	case UI_DESIGN_NODE_TIMELINE_VIEW:
		__uiDesignApplyTimelineDefaults(pNode);
		break;
	case UI_DESIGN_NODE_PROPERTY_GRID:
		__uiDesignApplyPropertyGridDefaults(pNode);
		break;
	case UI_DESIGN_NODE_MENU_BAR:
		__uiDesignFillMenuBarMetrics(pNode, &tMenuBarMetrics);
		__uiDesignFillMenuBarColors(pNode, &tMenuBarColors);
		__uiDesignTextPoolReset(pNode);
		iMenuBarItemCount = __uiDesignBuildMenuBarItems(pNode, arrMenuBarItems, UI_DESIGN_COUNT_OF(arrMenuBarItems));
		iRet = __uiDesignApplyMenuBarMenus(pApp, pNode, pWidget, arrMenuBarItems, iMenuBarItemCount);
		if ( iRet != XUI_OK ) return iRet;
		(void)xuiMenuBarSetItems(pWidget, arrMenuBarItems, iMenuBarItemCount);
		(void)xuiMenuBarSetMetrics(pWidget, &tMenuBarMetrics);
		(void)xuiMenuBarSetColors(pWidget, &tMenuBarColors);
		iPreviewIndex = __uiDesignInt(pNode, "data.hoverIndex", -1);
		(void)xuiMenuBarSetHoverIndex(pWidget, iPreviewIndex);
		iPreviewIndex = __uiDesignInt(pNode, "data.openIndex", -1);
		if ( iPreviewIndex >= 0 ) (void)xuiMenuBarOpenItem(pWidget, iPreviewIndex);
		else (void)xuiMenuBarClose(pWidget);
		break;
	case UI_DESIGN_NODE_TOOLBAR:
		__uiDesignFillToolbarMetrics(pNode, &tToolbarMetrics);
		__uiDesignFillToolbarColors(pNode, &tToolbarColors);
		__uiDesignTextPoolReset(pNode);
		iToolbarItemCount = __uiDesignBuildToolbarItems(pApp, pNode, arrToolbarItems, UI_DESIGN_COUNT_OF(arrToolbarItems));
		(void)xuiToolbarSetItems(pWidget, arrToolbarItems, iToolbarItemCount);
		(void)xuiToolbarSetOrientation(pWidget, __uiDesignInt(pNode, "behavior.orientation", XUI_ORIENTATION_HORIZONTAL));
		(void)xuiToolbarSetItemSize(pWidget,
			__uiDesignFloat(pNode, "metrics.itemWidth", 64.0f),
			__uiDesignFloat(pNode, "metrics.itemHeight", 30.0f),
			__uiDesignFloat(pNode, "metrics.separatorSize", 8.0f));
		(void)xuiToolbarSetOverflow(pWidget, __uiDesignBool(pNode, "behavior.overflow", 1), __uiDesignFloat(pNode, "metrics.overflowButtonSize", 28.0f), NULL, NULL);
		(void)xuiToolbarSetMetrics(pWidget, &tToolbarMetrics);
		(void)xuiToolbarSetColors(pWidget, &tToolbarColors);
		(void)xuiToolbarSetHoverIndex(pWidget, __uiDesignInt(pNode, "data.hoverIndex", -1));
		break;
	case UI_DESIGN_NODE_STATUS_BAR:
		__uiDesignFillStatusBarMetrics(pNode, &tStatusBarMetrics);
		__uiDesignFillStatusBarColors(pNode, &tStatusBarColors);
		__uiDesignTextPoolReset(pNode);
		iStatusBarItemCount = __uiDesignBuildStatusBarItems(pNode, arrStatusBarItems, UI_DESIGN_COUNT_OF(arrStatusBarItems));
		(void)xuiStatusBarSetItems(pWidget, arrStatusBarItems, iStatusBarItemCount);
		(void)xuiStatusBarSetMetrics(pWidget, &tStatusBarMetrics);
		(void)xuiStatusBarSetColors(pWidget, &tStatusBarColors);
		(void)xuiStatusBarSetHoverIndex(pWidget, __uiDesignInt(pNode, "data.hoverIndex", -1));
		break;
	case UI_DESIGN_NODE_DOCK_PANEL:
		__uiDesignFillDockPanelMetrics(pNode, &tDockMetrics);
		__uiDesignFillDockPanelColors(pNode, &tDockColors);
		(void)xuiDockPanelSetMetrics(pWidget, &tDockMetrics);
		(void)xuiDockPanelSetColors(pWidget, &tDockColors);
		(void)__uiDesignApplyDockPanelWindows(pApp, pNode);
		(void)__uiDesignApplyDockPanelRegions(pNode);
		(void)__uiDesignApplyDockPanelMenuTitles(pNode, pWidget);
		(void)__uiDesignApplyDockPanelTooltips(pNode, pWidget);
		break;
	case UI_DESIGN_NODE_POPUP:
		(void)xuiPopupSetContentSize(pWidget,
			__uiDesignFloat(pNode, "metrics.contentWidth", 260.0f),
			__uiDesignFloat(pNode, "metrics.contentHeight", 180.0f));
		(void)xuiPopupSetMaxSize(pWidget,
			__uiDesignFloat(pNode, "metrics.maxWidth", 0.0f),
			__uiDesignFloat(pNode, "metrics.maxHeight", 0.0f));
		(void)xuiPopupSetGap(pWidget, __uiDesignFloat(pNode, "metrics.gap", 4.0f));
		(void)xuiPopupSetOffset(pWidget,
			__uiDesignFloat(pNode, "metrics.offsetX", 0.0f),
			__uiDesignFloat(pNode, "metrics.offsetY", 0.0f));
		(void)xuiPopupSetMargin(pWidget, __uiDesignFloat(pNode, "metrics.margin", 6.0f));
		(void)xuiPopupSetAnchor(pWidget, __uiDesignInt(pNode, "behavior.anchor", XUI_POPUP_ANCHOR_BOTTOM_LEFT));
		(void)xuiPopupSetDirection(pWidget, __uiDesignInt(pNode, "behavior.direction", XUI_POPUP_DIRECTION_RIGHT_DOWN));
		(void)xuiPopupSetClosePolicy(pWidget,
			__uiDesignInt(pNode, "behavior.outsidePolicy", XUI_POPUP_OUTSIDE_CLOSE),
			__uiDesignInt(pNode, "behavior.ownerPolicy", XUI_POPUP_OWNER_PASSTHROUGH),
			__uiDesignInt(pNode, "behavior.escapePolicy", XUI_POPUP_ESCAPE_CLOSE));
		(void)xuiPopupSetModal(pWidget, __uiDesignBool(pNode, "behavior.modal", 0));
		(void)xuiPopupSetConsumeInside(pWidget, __uiDesignBool(pNode, "behavior.consumeInside", 0));
		(void)xuiPopupSetMatchOwnerWidth(pWidget, __uiDesignBool(pNode, "behavior.matchOwnerWidth", 0));
		(void)xuiPopupSetFocusPolicy(pWidget, __uiDesignInt(pNode, "behavior.focusPolicy", XUI_POPUP_FOCUS_POPUP), NULL);
		(void)xuiPopupSetMetrics(pWidget,
			__uiDesignFloat(pNode, "metrics.padding", 5.0f),
			__uiDesignFloat(pNode, "metrics.borderWidth", 1.0f),
			__uiDesignFloat(pNode, "metrics.shadowSize", 4.0f));
		(void)xuiPopupSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.panelColor", XUI_COLOR_RGBA(250, 253, 255, 255)),
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(152, 180, 211, 255)),
			__uiDesignColor(pNode, "appearance.shadowColor", XUI_COLOR_RGBA(35, 63, 96, 40)),
			__uiDesignColor(pNode, "appearance.backdropColor", XUI_COLOR_RGBA(0, 0, 0, 0)));
		__uiDesignApplyPopupScrollStyle(pNode, pWidget);
		if ( __uiDesignContainerHasModelChildren(pApp, pNode) ) {
			__uiDesignDetachContainerModelChildren(pApp, pNode);
			pMenuContent = xuiPopupGetContentWidget(pWidget);
			if ( pMenuContent != NULL ) {
				__uiDesignApplyContainerHostLayout(pNode, pMenuContent, 1);
				__uiDesignClearChildren(pMenuContent);
			}
			(void)__uiDesignAttachContainerModelChildren(pApp, pNode);
		} else {
			(void)__uiDesignApplyPopupContent(pApp, pNode);
		}
		(void)xuiPopupSetScroll(pWidget, __uiDesignFloat(pNode, "value.scrollX", 0.0f), __uiDesignFloat(pNode, "value.scrollY", 0.0f));
		tPreviewAnchor = (xui_rect_t){pNode->tRect.fX, pNode->tRect.fY, 0.0f, 0.0f};
		(void)xuiPopupSetAnchorRect(pWidget, tPreviewAnchor);
		(void)xuiPopupApplyPlacement(pWidget);
		(void)xuiPopupSetOpen(pWidget, pNode->bVisible && __uiDesignBool(pNode, "behavior.open", 1));
		break;
	case UI_DESIGN_NODE_MENU:
		__uiDesignFillMenuMetrics(pNode, &tMenuMetrics);
		__uiDesignFillMenuColors(pNode, &tMenuColors);
		pMenuContent = xuiPopupGetContentWidget(pWidget);
		pMenuWidget = (pMenuContent != NULL) ? xuiWidgetGetFirstChild(pMenuContent) : NULL;
		if ( pMenuWidget != NULL ) {
			(void)xuiMenuSetFont(pMenuWidget, __uiDesignResolveNodeFont(pApp, pNode));
			__uiDesignClearRuntimeMenus(pNode);
			__uiDesignTextPoolReset(pNode);
			iMenuItemCount = __uiDesignBuildMenuItems(pNode, arrMenuItems, UI_DESIGN_COUNT_OF(arrMenuItems));
			iRet = __uiDesignApplyMenuItemSubmenus(pApp, pNode, pMenuWidget, NULL, -1, 0, arrMenuItems, iMenuItemCount, "", &tMenuMetrics, &tMenuColors);
			if ( iRet != XUI_OK ) return iRet;
			(void)xuiMenuSetItems(pMenuWidget, arrMenuItems, iMenuItemCount);
			(void)xuiMenuSetMetrics(pMenuWidget, &tMenuMetrics);
			(void)xuiMenuSetColors(pMenuWidget, &tMenuColors);
			(void)xuiMenuSetHoverIndex(pMenuWidget, __uiDesignInt(pNode, "data.hoverIndex", -1));
		}
		tPreviewAnchor = (xui_rect_t){pNode->tRect.fX, pNode->tRect.fY, 0.0f, 0.0f};
		(void)xuiPopupSetClosePolicy(pWidget, XUI_POPUP_OUTSIDE_IGNORE, XUI_POPUP_OWNER_PASSTHROUGH, XUI_POPUP_ESCAPE_IGNORE);
		(void)xuiPopupSetAnchorRect(pWidget, tPreviewAnchor);
		(void)xuiPopupSetAnchor(pWidget, XUI_POPUP_ANCHOR_FIXED);
		(void)xuiPopupSetDirection(pWidget, XUI_POPUP_DIRECTION_RIGHT_DOWN);
		(void)xuiPopupSetGap(pWidget, 0.0f);
		(void)xuiPopupApplyPlacement(pWidget);
		(void)xuiPopupSetOpen(pWidget, pNode->bVisible && __uiDesignBool(pNode, "behavior.previewOpen", 1));
		break;
	case UI_DESIGN_NODE_MSG_BOX:
	case UI_DESIGN_NODE_FILE_DIALOG:
	case UI_DESIGN_NODE_MSG_TIP:
	case UI_DESIGN_NODE_TOAST:
		(void)__uiDesignApplyOverlayPreview(pApp, pNode);
		break;
	case UI_DESIGN_NODE_CASCADER:
		__uiDesignTextPoolReset(pNode);
		iCascaderItemCount = __uiDesignBuildCascaderItems(pNode, arrCascaderItems, UI_DESIGN_COUNT_OF(arrCascaderItems), arrCascaderPath, &iCascaderDepth);
		(void)xuiCascaderSetItems(pWidget, arrCascaderItems, iCascaderItemCount);
		if ( iCascaderDepth > 0 ) (void)xuiCascaderSetSelectedPath(pWidget, arrCascaderPath, iCascaderDepth);
		(void)xuiCascaderSetPlaceholder(pWidget, __uiDesignText(pNode, "text.placeholder", "Select path"));
		(void)xuiCascaderSetSeparator(pWidget, __uiDesignText(pNode, "text.separator", "/"));
		(void)xuiCascaderSetShowAllLevels(pWidget, __uiDesignBool(pNode, "behavior.showAllLevels", 1));
		(void)xuiCascaderSetClearable(pWidget, __uiDesignBool(pNode, "behavior.clearable", 1));
		(void)xuiCascaderSetSelectAnyLevel(pWidget, __uiDesignBool(pNode, "behavior.selectAnyLevel", 0));
		(void)xuiCascaderSetExpandTrigger(pWidget, __uiDesignInt(pNode, "behavior.expandTrigger", XUI_CASCADER_EXPAND_CLICK));
		(void)xuiCascaderSetPopupPlacement(pWidget, __uiDesignInt(pNode, "behavior.popupPlacement", XUI_CASCADER_POPUP_AUTO));
		(void)xuiCascaderSetPopupSize(pWidget,
			__uiDesignFloat(pNode, "metrics.columnWidth", 150.0f),
			__uiDesignFloat(pNode, "metrics.popupHeight", 0.0f),
			__uiDesignFloat(pNode, "metrics.popupMaxHeight", 240.0f));
		(void)xuiCascaderSetMetrics(pWidget, __uiDesignFloat(pNode, "metrics.itemHeight", 24.0f), __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f));
		(void)xuiCascaderSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.placeholderColor", XUI_COLOR_RGBA(140, 154, 175, 255)),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)),
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(240, 246, 253, 255)),
			__uiDesignColor(pNode, "appearance.openBackgroundColor", XUI_COLOR_RGBA(231, 241, 252, 255)),
			__uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(238, 242, 247, 255)));
		(void)xuiCascaderSetBorderColors(pWidget,
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(126, 164, 207, 255)),
			__uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255)));
		(void)xuiCascaderSetArrowColors(pWidget,
			__uiDesignColor(pNode, "appearance.arrowColor", XUI_COLOR_RGBA(140, 154, 175, 255)),
			__uiDesignColor(pNode, "appearance.disabledArrowColor", XUI_COLOR_RGBA(140, 154, 175, 255)));
		(void)xuiCascaderSetButtonColors(pWidget,
			__uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(246, 250, 253, 255)),
			__uiDesignColor(pNode, "appearance.buttonHoverColor", XUI_COLOR_RGBA(230, 246, 238, 255)),
			__uiDesignColor(pNode, "appearance.buttonOpenColor", XUI_COLOR_RGBA(220, 242, 232, 255)));
		(void)xuiCascaderSetPopupColors(pWidget,
			__uiDesignColor(pNode, "appearance.popupPanelColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.popupBorderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.popupShadowColor", XUI_COLOR_RGBA(0, 0, 0, 46)),
			__uiDesignColor(pNode, "appearance.popupTextColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.popupMutedTextColor", XUI_COLOR_RGBA(107, 127, 149, 255)),
			__uiDesignColor(pNode, "appearance.popupHoverColor", XUI_COLOR_RGBA(240, 246, 253, 255)),
			__uiDesignColor(pNode, "appearance.popupActiveColor", XUI_COLOR_RGBA(231, 241, 252, 255)),
			__uiDesignColor(pNode, "appearance.popupSelectedColor", XUI_COLOR_RGBA(238, 246, 255, 255)),
			__uiDesignColor(pNode, "appearance.popupActiveTextColor", XUI_COLOR_RGBA(31, 95, 168, 255)),
			__uiDesignColor(pNode, "appearance.popupDisabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)),
			__uiDesignColor(pNode, "appearance.popupSeparatorColor", XUI_COLOR_RGBA(208, 219, 232, 255)));
		if ( __uiDesignBool(pNode, "behavior.open", 0) ) (void)xuiCascaderOpen(pWidget);
		else (void)xuiCascaderClose(pWidget);
		break;
	case UI_DESIGN_NODE_COLOR_PICKER:
		(void)xuiColorPickerSetAlphaEnabled(pWidget, __uiDesignBool(pNode, "behavior.alphaEnabled", 1));
		(void)xuiColorPickerSetColor(pWidget, __uiDesignColorPickerValue(pNode));
		if ( __uiDesignBool(pNode, "value.useHex", 0) ) {
			(void)xuiColorPickerSetHex(pWidget, __uiDesignText(pNode, "value.hex", "#317ED6FF"));
		}
		iPaletteCount = __uiDesignBuildColorPalette(pNode, arrPalette, UI_DESIGN_COUNT_OF(arrPalette));
		(void)xuiColorPickerSetPalette(pWidget, arrPalette, iPaletteCount);
		(void)xuiColorPickerSetPopupSize(pWidget, __uiDesignFloat(pNode, "metrics.popupWidth", 260.0f), __uiDesignFloat(pNode, "metrics.popupHeight", 250.0f));
		(void)xuiColorPickerSetPopupPlacement(pWidget, __uiDesignInt(pNode, "behavior.popupPlacement", XUI_COLOR_PICKER_POPUP_AUTO));
		(void)xuiColorPickerSetMetrics(pWidget, __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f));
		(void)xuiColorPickerSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)),
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(240, 246, 253, 255)),
			__uiDesignColor(pNode, "appearance.openBackgroundColor", XUI_COLOR_RGBA(231, 241, 252, 255)),
			__uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(238, 242, 247, 255)));
		(void)xuiColorPickerSetBorderColors(pWidget,
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(126, 164, 207, 255)),
			__uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255)));
		(void)xuiColorPickerSetArrowColors(pWidget,
			__uiDesignColor(pNode, "appearance.arrowColor", XUI_COLOR_RGBA(42, 92, 136, 255)),
			__uiDesignColor(pNode, "appearance.disabledArrowColor", XUI_COLOR_RGBA(134, 148, 164, 180)));
		(void)xuiColorPickerSetButtonColors(pWidget,
			__uiDesignColor(pNode, "appearance.buttonColor", XUI_COLOR_RGBA(236, 246, 253, 255)),
			__uiDesignColor(pNode, "appearance.buttonHoverColor", XUI_COLOR_RGBA(220, 238, 251, 255)),
			__uiDesignColor(pNode, "appearance.buttonOpenColor", XUI_COLOR_RGBA(207, 229, 247, 255)));
		(void)xuiColorPickerSetPopupColors(pWidget,
			__uiDesignColor(pNode, "appearance.popupPanelColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.popupBorderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.popupShadowColor", XUI_COLOR_RGBA(0, 0, 0, 46)),
			__uiDesignColor(pNode, "appearance.popupTextColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.popupMutedTextColor", XUI_COLOR_RGBA(107, 127, 149, 255)),
			__uiDesignColor(pNode, "appearance.popupAccentColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.popupFieldColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.popupFieldBorderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.popupSeparatorColor", XUI_COLOR_RGBA(208, 219, 232, 255)));
		if ( __uiDesignBool(pNode, "behavior.open", 0) ) (void)xuiColorPickerOpen(pWidget);
		else (void)xuiColorPickerClose(pWidget);
		break;
	case UI_DESIGN_NODE_DATE_PICKER:
		(void)xuiDatePickerSetMode(pWidget, __uiDesignInt(pNode, "behavior.mode", XUI_DATE_PICKER_MODE_DATE));
		(void)xuiDatePickerSetNullable(pWidget, __uiDesignBool(pNode, "behavior.nullable", 1));
		(void)xuiDatePickerSetFormat(pWidget, __uiDesignText(pNode, "text.format", "yyyy-mm-dd"));
		(void)xuiDatePickerSetRangeSeparator(pWidget, __uiDesignText(pNode, "text.rangeSeparator", " - "));
		(void)xuiDatePickerSetShowSecond(pWidget, __uiDesignBool(pNode, "behavior.showSecond", 0));
		(void)xuiDatePickerSetFirstDayOfWeek(pWidget, __uiDesignInt(pNode, "behavior.firstDayOfWeek", 1));
		__uiDesignApplyDatePickerValue(pNode, pWidget);
		(void)xuiDatePickerSetPopupSize(pWidget, __uiDesignFloat(pNode, "metrics.popupWidth", 280.0f), __uiDesignFloat(pNode, "metrics.popupHeight", 300.0f));
		(void)xuiDatePickerSetPopupPlacement(pWidget, __uiDesignInt(pNode, "behavior.popupPlacement", XUI_DATE_PICKER_POPUP_AUTO));
		(void)xuiDatePickerSetMetrics(pWidget, __uiDesignFloat(pNode, "metrics.borderWidth", 1.0f));
		(void)xuiDatePickerSetColors(pWidget,
			__uiDesignColor(pNode, "appearance.textColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.disabledTextColor", XUI_COLOR_RGBA(140, 154, 175, 255)),
			__uiDesignColor(pNode, "appearance.backgroundColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.hoverBackgroundColor", XUI_COLOR_RGBA(240, 246, 253, 255)),
			__uiDesignColor(pNode, "appearance.openBackgroundColor", XUI_COLOR_RGBA(231, 241, 252, 255)),
			__uiDesignColor(pNode, "appearance.disabledBackgroundColor", XUI_COLOR_RGBA(238, 242, 247, 255)));
		(void)xuiDatePickerSetBorderColors(pWidget,
			__uiDesignColor(pNode, "appearance.borderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.hoverBorderColor", XUI_COLOR_RGBA(126, 164, 207, 255)),
			__uiDesignColor(pNode, "appearance.focusBorderColor", XUI_COLOR_RGBA(49, 126, 214, 255)));
		(void)xuiDatePickerSetArrowColors(pWidget,
			__uiDesignColor(pNode, "appearance.arrowColor", XUI_COLOR_RGBA(60, 82, 104, 255)),
			__uiDesignColor(pNode, "appearance.disabledArrowColor", XUI_COLOR_RGBA(150, 162, 174, 255)));
		(void)xuiDatePickerSetPopupColors(pWidget,
			__uiDesignColor(pNode, "appearance.popupPanelColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.popupBorderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.popupShadowColor", XUI_COLOR_RGBA(0, 0, 0, 46)),
			__uiDesignColor(pNode, "appearance.popupTextColor", XUI_COLOR_RGBA(35, 50, 70, 255)),
			__uiDesignColor(pNode, "appearance.popupMutedTextColor", XUI_COLOR_RGBA(107, 127, 149, 255)),
			__uiDesignColor(pNode, "appearance.popupAccentColor", XUI_COLOR_RGBA(49, 126, 214, 255)),
			__uiDesignColor(pNode, "appearance.popupFieldColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.popupFieldBorderColor", XUI_COLOR_RGBA(158, 182, 209, 255)),
			__uiDesignColor(pNode, "appearance.popupSelectedTextColor", XUI_COLOR_WHITE),
			__uiDesignColor(pNode, "appearance.popupDisabledDayColor", XUI_COLOR_RGBA(183, 195, 208, 255)),
			__uiDesignColor(pNode, "appearance.popupSeparatorColor", XUI_COLOR_RGBA(208, 219, 232, 255)));
		if ( __uiDesignBool(pNode, "behavior.open", 0) ) (void)xuiDatePickerOpen(pWidget);
		else (void)xuiDatePickerClose(pWidget);
		break;
	case UI_DESIGN_NODE_CODE_EDIT:
		__uiDesignTextPoolReset(pNode);
		(void)xuiCodeEditSetText(pWidget, __uiDesignText(pNode, "data.text", "int main(void) {\n    return 0;\n}"));
		(void)xuiCodeEditSetLanguage(pWidget, __uiDesignText(pNode, "text.language", "c"));
		(void)xuiCodeEditSetReadonly(pWidget, __uiDesignBool(pNode, "behavior.readonly", 0));
		(void)xuiCodeEditSetWordWrap(pWidget, __uiDesignBool(pNode, "behavior.wordWrap", 0));
		(void)xuiCodeEditSetEolMode(pWidget, __uiDesignInt(pNode, "behavior.eolMode", XUI_CODE_EOL_LF));
		(void)xuiCodeEditSetDisplayOptions(pWidget, __uiDesignCodeEditOptions(pNode));
		(void)__uiDesignApplyCodeEditMargins(pNode, pWidget);
		(void)xuiCodeEditSetTabColumns(pWidget, __uiDesignInt(pNode, "metrics.tabColumns", 4));
		(void)xuiCodeEditSetIndentColumns(pWidget, __uiDesignInt(pNode, "metrics.indentColumns", 4));
		(void)xuiCodeEditSetExpandTabs(pWidget, __uiDesignBool(pNode, "behavior.expandTabs", 1));
		(void)xuiCodeEditSetScroll(pWidget, __uiDesignFloat(pNode, "value.scrollX", 0.0f), __uiDesignFloat(pNode, "value.scrollY", 0.0f));
		(void)__uiDesignApplyCodeEditMenuTitles(pNode, pWidget);
		__uiDesignApplyCodeEditTheme(pNode, pWidget);
		__uiDesignApplyCodeEditAnnotations(pNode, pWidget);
		__uiDesignApplyCodeEditFolds(pNode, pWidget);
		__uiDesignApplyCodeEditSelections(pNode, pWidget);
		__uiDesignApplyCodeEditFindPreview(pNode, pWidget);
		__uiDesignApplyCodeEditFindWindow(pWidget, __uiDesignInt(pNode, "behavior.findWindow", 0));
		break;
	case UI_DESIGN_NODE_FLOW_GRAPH:
		(void)__uiDesignApplyFlowGraph(pNode);
		break;
	case UI_DESIGN_NODE_WORKFLOW:
		(void)__uiDesignApplyWorkflow(pNode);
		break;
	default:
		break;
	}
	(void)xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}
