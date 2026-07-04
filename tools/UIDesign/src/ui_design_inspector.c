#include "ui_design_inspector.h"
#include "ui_design_registry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UI_DESIGN_INSPECTOR_SPLIT_MIN 150.0f
#define UI_DESIGN_INSPECTOR_GAP 6.0f
#define UI_DESIGN_COMPLEX_EDITOR_W 760.0f
#define UI_DESIGN_COMPLEX_EDITOR_H 520.0f

#define UI_DESIGN_COUNT_OF(a) ((int)(sizeof(a) / sizeof((a)[0])))

static const xui_combobox_item_t g_arrLayoutTypeEnum[] = {
	{"Manual", XUI_LAYOUT_MANUAL, 1, 0, 0, NULL},
	{"Overlay", XUI_LAYOUT_OVERLAY, 1, 0, 0, NULL},
	{"Row", XUI_LAYOUT_ROW, 1, 0, 0, NULL},
	{"Column", XUI_LAYOUT_COLUMN, 1, 0, 0, NULL},
	{"Flow", XUI_LAYOUT_FLOW, 1, 0, 0, NULL},
	{"Table", XUI_LAYOUT_TABLE, 1, 0, 0, NULL},
	{"Dock", XUI_LAYOUT_DOCK, 1, 0, 0, NULL},
	{"Grid", XUI_LAYOUT_GRID, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrSizeModeEnum[] = {
	{"Fixed", XUI_SIZE_FIXED, 1, 0, 0, NULL},
	{"Content", XUI_SIZE_CONTENT, 1, 0, 0, NULL},
	{"Fill", XUI_SIZE_FILL, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrFlowModeEnum[] = {
	{"Block", XUI_FLOW_BLOCK, 1, 0, 0, NULL},
	{"Inline", XUI_FLOW_INLINE, 1, 0, 0, NULL},
	{"Inline Block", XUI_FLOW_INLINE_BLOCK, 1, 0, 0, NULL},
	{"None", XUI_FLOW_NONE, 1, 0, 0, NULL},
	{"Absolute", XUI_FLOW_ABSOLUTE, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrDockEnum[] = {
	{"None", 0, 1, 0, 0, NULL},
	{"Left", XUI_DOCK_LEFT, 1, 0, 0, NULL},
	{"Top", XUI_DOCK_TOP, 1, 0, 0, NULL},
	{"Right", XUI_DOCK_RIGHT, 1, 0, 0, NULL},
	{"Bottom", XUI_DOCK_BOTTOM, 1, 0, 0, NULL},
	{"Fill", XUI_DOCK_FILL, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrOverflowEnum[] = {
	{"Visible", XUI_OVERFLOW_VISIBLE, 1, 0, 0, NULL},
	{"Hidden", XUI_OVERFLOW_HIDDEN, 1, 0, 0, NULL},
	{"Clip", XUI_OVERFLOW_CLIP, 1, 0, 0, NULL},
	{"Report", XUI_OVERFLOW_REPORT, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrAlignEnum[] = {
	{"Start", XUI_ALIGN_START, 1, 0, 0, NULL},
	{"Center", XUI_ALIGN_CENTER, 1, 0, 0, NULL},
	{"End", XUI_ALIGN_END, 1, 0, 0, NULL},
	{"Stretch", XUI_ALIGN_STRETCH, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrLayerEnum[] = {
	{"Normal", XUI_LAYER_NORMAL, 1, 0, 0, NULL},
	{"Floating", XUI_LAYER_FLOATING, 1, 0, 0, NULL},
	{"Popup", XUI_LAYER_POPUP, 1, 0, 0, NULL},
	{"Modal", XUI_LAYER_MODAL, 1, 0, 0, NULL},
	{"Tooltip", XUI_LAYER_TOOLTIP, 1, 0, 0, NULL},
	{"Drag", XUI_LAYER_DRAG, 1, 0, 0, NULL},
	{"Debug", XUI_LAYER_DEBUG, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrImeModeEnum[] = {
	{"Disabled", XUI_IME_DISABLED, 1, 0, 0, NULL},
	{"Enabled", XUI_IME_ENABLED, 1, 0, 0, NULL},
	{"Auto", XUI_IME_AUTO, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrTooltipAnchorEnum[] = {
	{"Widget Bottom", XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM, 1, 0, 0, NULL},
	{"Widget Top", XUI_TOOLTIP_ANCHOR_WIDGET_TOP, 1, 0, 0, NULL},
	{"Widget Right", XUI_TOOLTIP_ANCHOR_WIDGET_RIGHT, 1, 0, 0, NULL},
	{"Widget Left", XUI_TOOLTIP_ANCHOR_WIDGET_LEFT, 1, 0, 0, NULL},
	{"Cursor", XUI_TOOLTIP_ANCHOR_CURSOR, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrCachePolicyEnum[] = {
	{"None", XUI_CACHE_POLICY_NONE, 1, 0, 0, NULL},
	{"Self", XUI_CACHE_POLICY_SELF, 1, 0, 0, NULL},
	{"Subtree", XUI_CACHE_POLICY_SUBTREE, 1, 0, 0, NULL},
	{"Subtree Tiled", XUI_CACHE_POLICY_SUBTREE_TILED, 1, 0, 0, NULL},
	{"Display List", XUI_CACHE_POLICY_DISPLAY_LIST, 1, 0, 0, NULL},
	{"Auto", XUI_CACHE_POLICY_AUTO, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrFontFormatEnum[] = {
	{"Auto", 0, 1, 0, 0, NULL},
	{"TTF", XUI_FONT_FORMAT_TTF, 1, 0, 0, NULL},
	{"XRF", XUI_FONT_FORMAT_XRF, 1, 0, 0, NULL}
};

typedef struct ui_design_layout_property_def_t {
	const char* sId;
	const char* sName;
	const char* sDescription;
	const char* sDefaultValue;
	int iType;
	const xui_combobox_item_t* pEnumItems;
	int iEnumCount;
} ui_design_layout_property_def_t;

static const ui_design_layout_property_def_t g_arrLayoutProperties[] = {
	{"layout.type", "Layout Type", "Child layout algorithm used by this widget.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrLayoutTypeEnum, UI_DESIGN_COUNT_OF(g_arrLayoutTypeEnum)},
	{"layout.widthMode", "Width Mode", "How parent layouts resolve this widget width.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrSizeModeEnum, UI_DESIGN_COUNT_OF(g_arrSizeModeEnum)},
	{"layout.heightMode", "Height Mode", "How parent layouts resolve this widget height.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrSizeModeEnum, UI_DESIGN_COUNT_OF(g_arrSizeModeEnum)},
	{"layout.flowMode", "Flow Mode", "Flow layout participation mode.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrFlowModeEnum, UI_DESIGN_COUNT_OF(g_arrFlowModeEnum)},
	{"layout.dock", "Dock", "Dock side when parent layout is dock.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrDockEnum, UI_DESIGN_COUNT_OF(g_arrDockEnum)},
	{"layout.overflow", "Overflow", "Overflow policy for child content.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrOverflowEnum, UI_DESIGN_COUNT_OF(g_arrOverflowEnum)},
	{"layout.alignX", "Align X", "Horizontal alignment in parent layouts.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrAlignEnum, UI_DESIGN_COUNT_OF(g_arrAlignEnum)},
	{"layout.alignY", "Align Y", "Vertical alignment in parent layouts.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrAlignEnum, UI_DESIGN_COUNT_OF(g_arrAlignEnum)},
	{"layout.flexGrow", "Flex Grow", "Flex grow factor.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.flexShrink", "Flex Shrink", "Flex shrink factor.", "1", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.gap", "Gap", "Child gap for layout containers.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.gridColumns", "Grid Columns", "Grid layout column count.", "1", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.gridItemWidth", "Grid Item Width", "Grid layout item width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.gridItemHeight", "Grid Item Height", "Grid layout item height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.layer", "Layer", "Overlay/rendering layer.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrLayerEnum, UI_DESIGN_COUNT_OF(g_arrLayerEnum)},
	{"layout.zIndex", "Z Index", "Ordering inside the layer.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.preferredWidth", "Preferred Width", "Preferred width for layout measurement. 0 keeps the default.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.preferredHeight", "Preferred Height", "Preferred height for layout measurement. 0 keeps the default.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.minWidth", "Min Width", "Minimum layout width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.minHeight", "Min Height", "Minimum layout height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.maxWidth", "Max Width", "Maximum layout width. 0 means unbounded.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.maxHeight", "Max Height", "Maximum layout height. 0 means unbounded.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.marginLeft", "Margin Left", "Left margin.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.marginTop", "Margin Top", "Top margin.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.marginRight", "Margin Right", "Right margin.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.marginBottom", "Margin Bottom", "Bottom margin.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.paddingLeft", "Padding Left", "Left padding.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.paddingTop", "Padding Top", "Top padding.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.paddingRight", "Padding Right", "Right padding.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.paddingBottom", "Padding Bottom", "Bottom padding.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.tableRows", "Table Rows", "Row count for table layout containers.", "1", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.tableColumns", "Table Columns", "Column count for table layout containers.", "1", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.tableRowsConfig", "Table Rows Config", "Rows: index|mode|value|min|max|weight for all table layout rows.", "", XUI_TABLE_CELL_TYPE_CUSTOM, NULL, 0},
	{"layout.tableColumnsConfig", "Table Columns Config", "Rows: index|mode|value|min|max|weight for all table layout columns.", "", XUI_TABLE_CELL_TYPE_CUSTOM, NULL, 0},
	{"layout.tableTrackRow", "Table Track Row", "Row index configured by Table Row Size Mode/Value.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.tableRowSizeMode", "Table Row Size Mode", "Track sizing mode for the selected table row.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrSizeModeEnum, UI_DESIGN_COUNT_OF(g_arrSizeModeEnum)},
	{"layout.tableRowValue", "Table Row Value", "Track value for the selected table row.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.tableTrackColumn", "Table Track Column", "Column index configured by Table Column Size Mode/Value.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.tableColumnSizeMode", "Table Column Size Mode", "Track sizing mode for the selected table column.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrSizeModeEnum, UI_DESIGN_COUNT_OF(g_arrSizeModeEnum)},
	{"layout.tableColumnValue", "Table Column Value", "Track value for the selected table column.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"layout.tableCellRow", "Cell Row", "Table row used by this widget in a table-layout parent.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.tableCellColumn", "Cell Column", "Table column used by this widget in a table-layout parent.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.tableCellRowSpan", "Row Span", "Table row span.", "1", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.tableCellColumnSpan", "Column Span", "Table column span.", "1", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.carouselPage", "Carousel Page", "Page index used when this widget is inside a Carousel parent.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.splitPane", "Split Pane", "Pane index used when this widget is inside a SplitLayout parent.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.tabPage", "Tab Page", "Page index used when this widget is inside a Tabs parent.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.accordionSection", "Accordion Section", "Section index used when this widget is inside an Accordion parent.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"layout.dockWindow", "Dock Window", "Window index used when this widget is inside a DockPanel parent.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0}
};

static const ui_design_layout_property_def_t g_arrCommonWidgetProperties[] = {
	{"common.hitTestVisible", "Hit Test", "Participates in pointer hit-testing.", "true", XUI_TABLE_CELL_TYPE_BOOL, NULL, 0},
	{"common.focusable", "Focusable", "Can receive keyboard focus.", "false", XUI_TABLE_CELL_TYPE_BOOL, NULL, 0},
	{"common.tabStop", "Tab Stop", "Can be reached by tab navigation.", "false", XUI_TABLE_CELL_TYPE_BOOL, NULL, 0},
	{"common.tabIndex", "Tab Index", "Tab navigation order.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"common.focusScope", "Focus Scope", "Creates an independent focus scope.", "false", XUI_TABLE_CELL_TYPE_BOOL, NULL, 0},
	{"common.dragEnabled", "Drag Enabled", "Enables widget drag events.", "false", XUI_TABLE_CELL_TYPE_BOOL, NULL, 0},
	{"common.imeMode", "IME Mode", "Input method mode.", "2", XUI_TABLE_CELL_TYPE_ENUM, g_arrImeModeEnum, UI_DESIGN_COUNT_OF(g_arrImeModeEnum)},
	{"common.tooltipText", "Tooltip", "Text shown by the built-in tooltip.", "", XUI_TABLE_CELL_TYPE_TEXT, NULL, 0},
	{"common.tooltipAnchor", "Tooltip Anchor", "Tooltip placement anchor.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrTooltipAnchorEnum, UI_DESIGN_COUNT_OF(g_arrTooltipAnchorEnum)},
	{"common.tooltipFollowCursor", "Tooltip Follow Cursor", "Repositions the tooltip while the pointer moves.", "false", XUI_TABLE_CELL_TYPE_BOOL, NULL, 0},
	{"common.tooltipDelay", "Tooltip Delay", "Hover delay before opening the tooltip, in seconds.", "0.35", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"common.tooltipOffsetX", "Tooltip Offset X", "Horizontal tooltip offset.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"common.tooltipOffsetY", "Tooltip Offset Y", "Vertical tooltip offset.", "6", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"common.tooltipCustomAnchor", "Tooltip Custom Anchor", "Use an explicit anchor rectangle instead of the widget rectangle.", "false", XUI_TABLE_CELL_TYPE_BOOL, NULL, 0},
	{"common.tooltipAnchorX", "Tooltip Anchor X", "Custom tooltip anchor rectangle X.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"common.tooltipAnchorY", "Tooltip Anchor Y", "Custom tooltip anchor rectangle Y.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"common.tooltipAnchorW", "Tooltip Anchor W", "Custom tooltip anchor rectangle width.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"common.tooltipAnchorH", "Tooltip Anchor H", "Custom tooltip anchor rectangle height.", "0", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"style.name", "Style Name", "Named XUI style applied to this widget.", "", XUI_TABLE_CELL_TYPE_TEXT, NULL, 0},
	{"style.classes", "Style Classes", "Whitespace, comma, or semicolon separated style classes.", "", XUI_TABLE_CELL_TYPE_TEXT, NULL, 0},
	{"style.stateId", "State Id", "Style preview state bitmask: hover=1, active=2, focus=4, disabled=8.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"style.inlineProperties", "Inline Style", "Rows: name|type|value|dirtyFlags. Types: int, float, color, bool, string, token, none.", "", XUI_TABLE_CELL_TYPE_CUSTOM, NULL, 0},
	{"cache.policy", "Cache Policy", "Widget cache policy. Empty/default keeps the control type policy.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrCachePolicyEnum, UI_DESIGN_COUNT_OF(g_arrCachePolicyEnum)},
	{"cache.updateAllStates", "Cache All States", "Update all cached states together.", "false", XUI_TABLE_CELL_TYPE_BOOL, NULL, 0},
	{"cache.clearOnUpdate", "Cache Clear Update", "Clear cache surface before redraw.", "false", XUI_TABLE_CELL_TYPE_BOOL, NULL, 0},
	{"cache.pinned", "Cache Pinned", "Keep cache surfaces pinned in the cache budget.", "false", XUI_TABLE_CELL_TYPE_BOOL, NULL, 0},
	{"cache.clearColor", "Cache Clear Color", "Clear color used when cache clearing is enabled.", "#00000000", XUI_TABLE_CELL_TYPE_COLOR, NULL, 0},
	{"cache.tileWidth", "Cache Tile Width", "Tile width for tiled subtree caching. 0 keeps XUI default.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"cache.tileHeight", "Cache Tile Height", "Tile height for tiled subtree caching. 0 keeps XUI default.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"cache.maxBytes", "Cache Max Bytes", "Maximum bytes for this widget cache. 0 keeps XUI default.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"cache.stateCount", "Cache State Count", "Number of cache states. 0 keeps XUI default.", "0", XUI_TABLE_CELL_TYPE_INT, NULL, 0},
	{"cache.stateIds", "Cache State Ids", "Rows: index|stateId.", "", XUI_TABLE_CELL_TYPE_CUSTOM, NULL, 0}
};

static const ui_design_layout_property_def_t g_arrFontProperties[] = {
	{"font.name", "Registered Name", "Name registered in the XUI context. Empty uses the default font.", "", XUI_TABLE_CELL_TYPE_TEXT, NULL, 0},
	{"font.path", "Font File", "TTF/XRF font file used by this control. When set, it overrides Registered Name.", "", XUI_TABLE_CELL_TYPE_FILE, NULL, 0},
	{"font.size", "Size", "Font size used when Font File is set.", "13", XUI_TABLE_CELL_TYPE_FLOAT, NULL, 0},
	{"font.format", "Format", "Font file format. Auto infers from the file extension.", "0", XUI_TABLE_CELL_TYPE_ENUM, g_arrFontFormatEnum, UI_DESIGN_COUNT_OF(g_arrFontFormatEnum)}
};

static int __uiDesignInspectorHasChildren(ui_design_app_t* pApp, int iNodeId)
{
	int i;

	if ( pApp == NULL ) return 0;
	for ( i = 0; i < pApp->tModel.iNodeCount; i++ ) {
		if ( pApp->tModel.arrNodes[i].iParentId == iNodeId ) return 1;
	}
	return 0;
}

static const char* __uiDesignInspectorControlTitle(ui_design_node_type_t iType)
{
	const ui_design_control_desc_t* pDesc;

	pDesc = uiDesignRegistryFind(iType);
	return (pDesc != NULL && pDesc->sTitle != NULL) ? pDesc->sTitle : uiDesignNodeTypeName(iType);
}

static int __uiDesignInspectorMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pSize == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pSize = tConstraint;
	if ( pSize->fX <= 0.0f || pSize->fX >= XUI_LAYOUT_UNBOUNDED ) pSize->fX = 300.0f;
	if ( pSize->fY <= 0.0f || pSize->fY >= XUI_LAYOUT_UNBOUNDED ) pSize->fY = 520.0f;
	return XUI_OK;
}

static int __uiDesignInspectorArrange(xui_widget pWidget, xui_rect_t tContentRect, void* pUser)
{
	ui_design_app_t* pApp;
	xui_rect_t tTree;
	xui_rect_t tProps;
	float fTreeH;
	int iRet;

	(void)pWidget;
	pApp = (ui_design_app_t*)pUser;
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	fTreeH = tContentRect.fH * 0.42f;
	if ( fTreeH < UI_DESIGN_INSPECTOR_SPLIT_MIN ) fTreeH = UI_DESIGN_INSPECTOR_SPLIT_MIN;
	if ( fTreeH > tContentRect.fH - UI_DESIGN_INSPECTOR_SPLIT_MIN ) fTreeH = tContentRect.fH - UI_DESIGN_INSPECTOR_SPLIT_MIN;
	if ( fTreeH < 80.0f ) fTreeH = tContentRect.fH * 0.5f;
	tTree = (xui_rect_t){tContentRect.fX, tContentRect.fY, tContentRect.fW, fTreeH};
	tProps = (xui_rect_t){tContentRect.fX, tContentRect.fY + fTreeH + UI_DESIGN_INSPECTOR_GAP, tContentRect.fW, tContentRect.fH - fTreeH - UI_DESIGN_INSPECTOR_GAP};
	if ( pApp->pTree != NULL ) {
		iRet = xuiWidgetArrange(pApp->pTree, tTree);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pApp->pPropertyGrid != NULL ) {
		iRet = xuiWidgetArrange(pApp->pPropertyGrid, tProps);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static void __uiDesignInspectorFormatNode(const ui_design_node_t* pNode, char* sBuffer, int iCapacity)
{
	if ( (sBuffer == NULL) || (iCapacity <= 0) ) return;
	if ( pNode == NULL ) {
		snprintf(sBuffer, (size_t)iCapacity, "Root");
		return;
	}
	snprintf(sBuffer, (size_t)iCapacity, "%s  %s", __uiDesignInspectorControlTitle(pNode->iType), pNode->sText);
	sBuffer[iCapacity - 1] = '\0';
}

static int __uiDesignInspectorRefreshTree(ui_design_app_t* pApp)
{
	xui_tree_view_node_t arrNodes[UI_DESIGN_MAX_NODES + 1];
	ui_design_node_t* pNode;
	char arrText[UI_DESIGN_MAX_NODES + 1][160];
	int i;
	int n;
	int iRet;

	if ( (pApp == NULL) || (pApp->pTree == NULL) ) return XUI_OK;
	memset(arrNodes, 0, sizeof(arrNodes));
	memset(arrText, 0, sizeof(arrText));
	n = 0;
	snprintf(arrText[n], sizeof(arrText[n]), "Root");
	arrNodes[n].iId = 0;
	arrNodes[n].iParent = -1;
	arrNodes[n].sText = arrText[n];
	arrNodes[n].bExpanded = 1;
	arrNodes[n].bEnabled = 1;
	arrNodes[n].bHasChildren = __uiDesignInspectorHasChildren(pApp, 0);
	arrNodes[n].bIconReserved = 1;
	n++;
	for ( i = 0; i < pApp->tModel.iNodeCount && n < UI_DESIGN_MAX_NODES + 1; i++ ) {
		pNode = &pApp->tModel.arrNodes[i];
		__uiDesignInspectorFormatNode(pNode, arrText[n], (int)sizeof(arrText[n]));
		arrNodes[n].iId = pNode->iId;
		arrNodes[n].iParent = pNode->iParentId;
		arrNodes[n].sText = arrText[n];
		arrNodes[n].bExpanded = 1;
		arrNodes[n].bEnabled = 1;
		arrNodes[n].bHasChildren = __uiDesignInspectorHasChildren(pApp, pNode->iId);
		arrNodes[n].bIconReserved = 1;
		n++;
	}
	pApp->bSyncingTree = 1;
	iRet = xuiTreeViewSetNodes(pApp->pTree, arrNodes, n);
	if ( iRet == XUI_OK ) {
		iRet = xuiTreeViewSetSelected(pApp->pTree, pApp->tModel.iSelectedId);
	}
	if ( iRet == XUI_OK ) {
		(void)xuiTreeViewEnsureVisible(pApp->pTree, pApp->tModel.iSelectedId);
	}
	pApp->bSyncingTree = 0;
	return iRet;
}

static int __uiDesignInspectorAddProperty(xui_widget pGrid, int iCategory, const char* sId, const char* sName, const char* sDescription, int iType, const char* sValue, int iFlags)
{
	xui_property_desc_t tProp;

	if ( pGrid == NULL ) return -1;
	memset(&tProp, 0, sizeof(tProp));
	tProp.sId = sId;
	tProp.sName = sName;
	tProp.sDescription = sDescription;
	tProp.iType = iType;
	tProp.sValue = sValue;
	tProp.sDefaultValue = sValue;
	tProp.iFlags = iFlags;
	return xuiPropertyGridAddProperty(pGrid, iCategory, &tProp);
}

static int __uiDesignInspectorEnsureCategory(xui_widget pGrid, const char* sId, const char* sName)
{
	int iCategory;

	if ( (pGrid == NULL) || (sId == NULL) || (sName == NULL) ) return -1;
	iCategory = xuiPropertyGridFindCategory(pGrid, sId);
	if ( iCategory >= 0 ) return iCategory;
	return xuiPropertyGridAddCategory(pGrid, sId, sName, 1);
}

static void __uiDesignInspectorSetEnumConfig(xui_widget pGrid, int iProperty, const xui_combobox_item_t* pItems, int iCount, const char* sValue)
{
	xui_table_grid_editor_config_t tConfig;

	if ( (pGrid == NULL) || (iProperty < 0) || (pItems == NULL) || (iCount <= 0) ) return;
	memset(&tConfig, 0, sizeof(tConfig));
	tConfig.arrEnumItemData = pItems;
	tConfig.iEnumItemCount = iCount;
	tConfig.bEnumUseValue = 1;
	tConfig.iEnumSelectedValue = (sValue != NULL) ? atoi(sValue) : 0;
	(void)xuiPropertyGridSetEditorConfig(pGrid, iProperty, &tConfig);
}

typedef struct ui_design_complex_schema_t {
	int bStructured;
	char cDelimiter;
	const char* sColumns;
	int bExplodeCommaRows;
	int bTreeIndentRows;
	int bCascaderPathRows;
} ui_design_complex_schema_t;

typedef struct ui_design_complex_column_config_t {
	int iType;
	int bRequired;
	int bRangeSet;
	float fMin;
	float fMax;
	int iPrecision;
	const xui_combobox_item_t* pEnumItems;
	int iEnumCount;
	int bEnumUseValue;
} ui_design_complex_column_config_t;

static const xui_combobox_item_t g_arrComplexAlignEnum[] = {
	{"left", 0, 1, 0, 0, NULL},
	{"center", 0, 1, 0, 0, NULL},
	{"right", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexInputDecorationSideEnum[] = {
	{"leading", 0, 1, 0, 0, NULL},
	{"trailing", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexInputDecorationKindEnum[] = {
	{"none", 0, 1, 0, 0, NULL},
	{"icon", 0, 1, 0, 0, NULL},
	{"text", 0, 1, 0, 0, NULL},
	{"texture", 0, 1, 0, 0, NULL},
	{"clear", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexInputDecorationVisibleEnum[] = {
	{"always", 0, 1, 0, 0, NULL},
	{"notEmpty", 0, 1, 0, 0, NULL},
	{"focused", 0, 1, 0, 0, NULL},
	{"focusedNotEmpty", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexInputDecorationIconEnum[] = {
	{"none", 0, 1, 0, 0, NULL},
	{"search", 0, 1, 0, 0, NULL},
	{"user", 0, 1, 0, 0, NULL},
	{"lock", 0, 1, 0, 0, NULL},
	{"eye", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexInputMenuCommandEnum[] = {
	{"undo", 0, 1, 0, 0, NULL},
	{"redo", 0, 1, 0, 0, NULL},
	{"cut", 0, 1, 0, 0, NULL},
	{"copy", 0, 1, 0, 0, NULL},
	{"paste", 0, 1, 0, 0, NULL},
	{"delete", 0, 1, 0, 0, NULL},
	{"selectAll", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexCodeEditMenuCommandEnum[] = {
	{"undo", 0, 1, 0, 0, NULL},
	{"redo", 0, 1, 0, 0, NULL},
	{"cut", 0, 1, 0, 0, NULL},
	{"copy", 0, 1, 0, 0, NULL},
	{"paste", 0, 1, 0, 0, NULL},
	{"delete", 0, 1, 0, 0, NULL},
	{"selectAll", 0, 1, 0, 0, NULL},
	{"find", 0, 1, 0, 0, NULL},
	{"replace", 0, 1, 0, 0, NULL},
	{"gotoLine", 0, 1, 0, 0, NULL},
	{"toggleComment", 0, 1, 0, 0, NULL},
	{"toggleFold", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexTerminalMenuCommandEnum[] = {
	{"copy", 0, 1, 0, 0, NULL},
	{"paste", 0, 1, 0, 0, NULL},
	{"selectAll", 0, 1, 0, 0, NULL},
	{"clearScreen", 0, 1, 0, 0, NULL},
	{"clearScrollback", 0, 1, 0, 0, NULL},
	{"find", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexDockPanelMenuCommandEnum[] = {
	{"float", 0, 1, 0, 0, NULL},
	{"autoHide", 0, 1, 0, 0, NULL},
	{"close", 0, 1, 0, 0, NULL},
	{"closeOthers", 0, 1, 0, 0, NULL},
	{"closeAll", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexDockPanelTooltipEnum[] = {
	{"close", 0, 1, 0, 0, NULL},
	{"autoHide", 0, 1, 0, 0, NULL},
	{"options", 0, 1, 0, 0, NULL},
	{"moreTabs", 0, 1, 0, 0, NULL},
	{"dock", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexMsgBoxButtonTitleEnum[] = {
	{"ok", 0, 1, 0, 0, NULL},
	{"cancel", 0, 1, 0, 0, NULL},
	{"yes", 0, 1, 0, 0, NULL},
	{"no", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexFileDialogTextTargetEnum[] = {
	{"ok", 0, 1, 0, 0, NULL},
	{"cancel", 0, 1, 0, 0, NULL},
	{"up", 0, 1, 0, 0, NULL},
	{"refresh", 0, 1, 0, 0, NULL},
	{"path", 0, 1, 0, 0, NULL},
	{"file", 0, 1, 0, 0, NULL},
	{"folder", 0, 1, 0, 0, NULL},
	{"type", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexFileDialogEntryKindEnum[] = {
	{"file", 0, 1, 0, 0, NULL},
	{"folder", 0, 1, 0, 0, NULL},
	{"image", 0, 1, 0, 0, NULL},
	{"script", 0, 1, 0, 0, NULL},
	{"document", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexTimelineMenuCommandEnum[] = {
	{"rename", 0, 1, 0, 0, NULL},
	{"visible", 0, 1, 0, 0, NULL},
	{"locked", 0, 1, 0, 0, NULL},
	{"addLayer", 0, 1, 0, 0, NULL},
	{"deleteLayer", 0, 1, 0, 0, NULL},
	{"moveUp", 0, 1, 0, 0, NULL},
	{"moveDown", 0, 1, 0, 0, NULL},
	{"insertFrame", 0, 1, 0, 0, NULL},
	{"insertKeyframe", 0, 1, 0, 0, NULL},
	{"insertBlankKeyframe", 0, 1, 0, 0, NULL},
	{"clearKeyframe", 0, 1, 0, 0, NULL},
	{"createSpan", 0, 1, 0, 0, NULL},
	{"createSpanFromSelection", 0, 1, 0, 0, NULL},
	{"clearSpan", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexStyleValueTypeEnum[] = {
	{"none", 0, 1, 0, 0, NULL},
	{"int", 0, 1, 0, 0, NULL},
	{"float", 0, 1, 0, 0, NULL},
	{"color", 0, 1, 0, 0, NULL},
	{"bool", 0, 1, 0, 0, NULL},
	{"string", 0, 1, 0, 0, NULL},
	{"token", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexSplitPaneModeEnum[] = {
	{"grow", 0, 1, 0, 0, NULL},
	{"fixed", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexDockSideEnum[] = {
	{"fill", 0, 1, 0, 0, NULL},
	{"left", 0, 1, 0, 0, NULL},
	{"right", 0, 1, 0, 0, NULL},
	{"top", 0, 1, 0, 0, NULL},
	{"bottom", 0, 1, 0, 0, NULL},
	{"none", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexDockRegionEnum[] = {
	{"document", 0, 1, 0, 0, NULL},
	{"left", 0, 1, 0, 0, NULL},
	{"right", 0, 1, 0, 0, NULL},
	{"top", 0, 1, 0, 0, NULL},
	{"bottom", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexDockSizeModeEnum[] = {
	{"portion", 0, 1, 0, 0, NULL},
	{"pixel", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexTimelineKindEnum[] = {
	{"layer", 0, 1, 0, 0, NULL},
	{"frame", 0, 1, 0, 0, NULL},
	{"span", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexTimelineFrameEnum[] = {
	{"empty", 0, 1, 0, 0, NULL},
	{"normal", 0, 1, 0, 0, NULL},
	{"key", 0, 1, 0, 0, NULL},
	{"blank", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexTimelineSpanEnum[] = {
	{"custom", 0, 1, 0, 0, NULL},
	{"motion", 0, 1, 0, 0, NULL},
	{"shape", 0, 1, 0, 0, NULL},
	{"event", 0, 1, 0, 0, NULL},
	{"audio", 0, 1, 0, 0, NULL},
	{"hold", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexMessageTypeEnum[] = {
	{"other", 0, 1, 0, 0, NULL},
	{"self", 0, 1, 0, 0, NULL},
	{"system", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexChartSeriesTypeEnum[] = {
	{"line", 0, 1, 0, 0, NULL},
	{"bar", 0, 1, 0, 0, NULL},
	{"pie", 0, 1, 0, 0, NULL},
	{"scatter", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexChartSymbolEnum[] = {
	{"none", 0, 1, 0, 0, NULL},
	{"circle", 0, 1, 0, 0, NULL},
	{"rect", 0, 1, 0, 0, NULL},
	{"triangle", 0, 1, 0, 0, NULL},
	{"diamond", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexCanvasCommandEnum[] = {
	{"clear", 0, 1, 0, 0, NULL},
	{"clearRect", 0, 1, 0, 0, NULL},
	{"surface", 0, 1, 0, 0, NULL},
	{"point", 0, 1, 0, 0, NULL},
	{"line", 0, 1, 0, 0, NULL},
	{"rect", 0, 1, 0, 0, NULL},
	{"rectFill", 0, 1, 0, 0, NULL},
	{"circle", 0, 1, 0, 0, NULL},
	{"circleFill", 0, 1, 0, 0, NULL},
	{"triangle", 0, 1, 0, 0, NULL},
	{"triangleFill", 0, 1, 0, 0, NULL},
	{"text", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexToastTypeEnum[] = {
	{"info", 0, 1, 0, 0, NULL},
	{"success", 0, 1, 0, 0, NULL},
	{"warning", 0, 1, 0, 0, NULL},
	{"error", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexPropertyTypeEnum[] = {
	{"text", 0, 1, 0, 0, NULL},
	{"int", 0, 1, 0, 0, NULL},
	{"float", 0, 1, 0, 0, NULL},
	{"bool", 0, 1, 0, 0, NULL},
	{"textarea", 0, 1, 0, 0, NULL},
	{"date", 0, 1, 0, 0, NULL},
	{"time", 0, 1, 0, 0, NULL},
	{"datetime", 0, 1, 0, 0, NULL},
	{"enum", 0, 1, 0, 0, NULL},
	{"color", 0, 1, 0, 0, NULL},
	{"picker", 0, 1, 0, 0, NULL},
	{"custom", 0, 1, 0, 0, NULL},
	{"file", 0, 1, 0, 0, NULL},
	{"image", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexDateModeEnum[] = {
	{"date", XUI_DATE_PICKER_MODE_DATE, 1, 0, 0, NULL},
	{"time", XUI_DATE_PICKER_MODE_TIME, 1, 0, 0, NULL},
	{"datetime", XUI_DATE_PICKER_MODE_DATETIME, 1, 0, 0, NULL},
	{"dateRange", XUI_DATE_PICKER_MODE_DATE_RANGE, 1, 0, 0, NULL},
	{"timeRange", XUI_DATE_PICKER_MODE_TIME_RANGE, 1, 0, 0, NULL},
	{"datetimeRange", XUI_DATE_PICKER_MODE_DATETIME_RANGE, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexJoystickChannelEnum[] = {
	{"left", XUI_VIRTUAL_JOYSTICK_CHANNEL_LEFT, 1, 0, 0, NULL},
	{"right", XUI_VIRTUAL_JOYSTICK_CHANNEL_RIGHT, 1, 0, 0, NULL},
	{"up", XUI_VIRTUAL_JOYSTICK_CHANNEL_UP, 1, 0, 0, NULL},
	{"down", XUI_VIRTUAL_JOYSTICK_CHANNEL_DOWN, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexFlowRouteEnum[] = {
	{"Auto", XUI_FLOW_ROUTE_AUTO, 1, 0, 0, NULL},
	{"Straight", XUI_FLOW_ROUTE_STRAIGHT, 1, 0, 0, NULL},
	{"Orthogonal", XUI_FLOW_ROUTE_ORTHOGONAL, 1, 0, 0, NULL},
	{"Bezier", XUI_FLOW_ROUTE_BEZIER, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexFlowDirectionEnum[] = {
	{"input", XUI_FLOW_PORT_INPUT, 1, 0, 0, NULL},
	{"output", XUI_FLOW_PORT_OUTPUT, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexFlowKindEnum[] = {
	{"control", XUI_FLOW_PORT_CONTROL, 1, 0, 0, NULL},
	{"data", XUI_FLOW_PORT_DATA, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexFlowNodeRunEnum[] = {
	{"idle", XUI_WORKFLOW_NODE_RUN_IDLE, 1, 0, 0, NULL},
	{"queued", XUI_WORKFLOW_NODE_RUN_QUEUED, 1, 0, 0, NULL},
	{"running", XUI_WORKFLOW_NODE_RUN_RUNNING, 1, 0, 0, NULL},
	{"success", XUI_WORKFLOW_NODE_RUN_SUCCESS, 1, 0, 0, NULL},
	{"failed", XUI_WORKFLOW_NODE_RUN_FAILED, 1, 0, 0, NULL},
	{"skipped", XUI_WORKFLOW_NODE_RUN_SKIPPED, 1, 0, 0, NULL},
	{"warning", XUI_WORKFLOW_NODE_RUN_WARNING, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexFlowEdgeRunEnum[] = {
	{"idle", XUI_WORKFLOW_EDGE_RUN_IDLE, 1, 0, 0, NULL},
	{"active", XUI_WORKFLOW_EDGE_RUN_ACTIVE, 1, 0, 0, NULL},
	{"taken", XUI_WORKFLOW_EDGE_RUN_TAKEN, 1, 0, 0, NULL},
	{"skipped", XUI_WORKFLOW_EDGE_RUN_SKIPPED, 1, 0, 0, NULL},
	{"invalid", XUI_WORKFLOW_EDGE_RUN_INVALID, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexWorkflowConfigKindEnum[] = {
	{"string", XUI_WORKFLOW_CONFIG_FIELD_STRING, 1, 0, 0, NULL},
	{"textarea", XUI_WORKFLOW_CONFIG_FIELD_TEXTAREA, 1, 0, 0, NULL},
	{"int", XUI_WORKFLOW_CONFIG_FIELD_INT, 1, 0, 0, NULL},
	{"float", XUI_WORKFLOW_CONFIG_FIELD_FLOAT, 1, 0, 0, NULL},
	{"bool", XUI_WORKFLOW_CONFIG_FIELD_BOOL, 1, 0, 0, NULL},
	{"expression", XUI_WORKFLOW_CONFIG_FIELD_EXPRESSION, 1, 0, 0, NULL},
	{"select", XUI_WORKFLOW_CONFIG_FIELD_SELECT, 1, 0, 0, NULL},
	{"multiSelect", XUI_WORKFLOW_CONFIG_FIELD_MULTI_SELECT, 1, 0, 0, NULL},
	{"variableRef", XUI_WORKFLOW_CONFIG_FIELD_VARIABLE_REF, 1, 0, 0, NULL},
	{"nodeOutputRef", XUI_WORKFLOW_CONFIG_FIELD_NODE_OUTPUT_REF, 1, 0, 0, NULL},
	{"object", XUI_WORKFLOW_CONFIG_FIELD_OBJECT, 1, 0, 0, NULL},
	{"array", XUI_WORKFLOW_CONFIG_FIELD_ARRAY, 1, 0, 0, NULL},
	{"group", XUI_WORKFLOW_CONFIG_FIELD_GROUP, 1, 0, 0, NULL},
	{"tabs", XUI_WORKFLOW_CONFIG_FIELD_TABS, 1, 0, 0, NULL},
	{"conditionBuilder", XUI_WORKFLOW_CONFIG_FIELD_CONDITION_BUILDER, 1, 0, 0, NULL},
	{"mappingBuilder", XUI_WORKFLOW_CONFIG_FIELD_MAPPING_BUILDER, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexDiagnosticSeverityEnum[] = {
	{"error", XUI_CODE_DIAGNOSTIC_ERROR, 1, 0, 0, NULL},
	{"warning", XUI_CODE_DIAGNOSTIC_WARNING, 1, 0, 0, NULL},
	{"info", XUI_CODE_DIAGNOSTIC_INFO, 1, 0, 0, NULL},
	{"hint", XUI_CODE_DIAGNOSTIC_HINT, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexCodeStyleEnum[] = {
	{"default", XUI_CODE_STYLE_DEFAULT, 1, 0, 0, NULL},
	{"keyword", XUI_CODE_STYLE_KEYWORD, 1, 0, 0, NULL},
	{"type", XUI_CODE_STYLE_TYPE, 1, 0, 0, NULL},
	{"identifier", XUI_CODE_STYLE_IDENTIFIER, 1, 0, 0, NULL},
	{"number", XUI_CODE_STYLE_NUMBER, 1, 0, 0, NULL},
	{"string", XUI_CODE_STYLE_STRING, 1, 0, 0, NULL},
	{"char", XUI_CODE_STYLE_CHAR, 1, 0, 0, NULL},
	{"comment", XUI_CODE_STYLE_COMMENT, 1, 0, 0, NULL},
	{"preprocessor", XUI_CODE_STYLE_PREPROCESSOR, 1, 0, 0, NULL},
	{"operator", XUI_CODE_STYLE_OPERATOR, 1, 0, 0, NULL},
	{"brace", XUI_CODE_STYLE_BRACE, 1, 0, 0, NULL},
	{"error", XUI_CODE_STYLE_ERROR, 1, 0, 0, NULL},
	{"selection", XUI_CODE_STYLE_SELECTION, 1, 0, 0, NULL},
	{"currentLine", XUI_CODE_STYLE_CURRENT_LINE, 1, 0, 0, NULL},
	{"searchResult", XUI_CODE_STYLE_SEARCH_RESULT, 1, 0, 0, NULL},
	{"matchedBrace", XUI_CODE_STYLE_MATCHED_BRACE, 1, 0, 0, NULL},
	{"margin", XUI_CODE_STYLE_MARGIN, 1, 0, 0, NULL},
	{"lineNumber", XUI_CODE_STYLE_LINE_NUMBER, 1, 0, 0, NULL},
	{"foldIcon", XUI_CODE_STYLE_FOLD_ICON, 1, 0, 0, NULL},
	{"marker", XUI_CODE_STYLE_MARKER, 1, 0, 0, NULL},
	{"diagnosticError", XUI_CODE_STYLE_DIAGNOSTIC_ERROR, 1, 0, 0, NULL},
	{"diagnosticWarning", XUI_CODE_STYLE_DIAGNOSTIC_WARNING, 1, 0, 0, NULL},
	{"diagnosticInfo", XUI_CODE_STYLE_DIAGNOSTIC_INFO, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexCodeTokenEnum[] = {
	{"text", XUI_CODE_TOKEN_TEXT, 1, 0, 0, NULL},
	{"keyword", XUI_CODE_TOKEN_KEYWORD, 1, 0, 0, NULL},
	{"type", XUI_CODE_TOKEN_TYPE, 1, 0, 0, NULL},
	{"identifier", XUI_CODE_TOKEN_IDENTIFIER, 1, 0, 0, NULL},
	{"number", XUI_CODE_TOKEN_NUMBER, 1, 0, 0, NULL},
	{"string", XUI_CODE_TOKEN_STRING, 1, 0, 0, NULL},
	{"char", XUI_CODE_TOKEN_CHAR, 1, 0, 0, NULL},
	{"comment", XUI_CODE_TOKEN_COMMENT, 1, 0, 0, NULL},
	{"preprocessor", XUI_CODE_TOKEN_PREPROCESSOR, 1, 0, 0, NULL},
	{"operator", XUI_CODE_TOKEN_OPERATOR, 1, 0, 0, NULL},
	{"brace", XUI_CODE_TOKEN_BRACE, 1, 0, 0, NULL},
	{"error", XUI_CODE_TOKEN_ERROR, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexCodeMarkerEnum[] = {
	{"breakpoint", XUI_CODE_MARKER_BREAKPOINT, 1, 0, 0, NULL},
	{"breakpointDisabled", XUI_CODE_MARKER_BREAKPOINT_DISABLED, 1, 0, 0, NULL},
	{"executionLine", XUI_CODE_MARKER_EXECUTION_LINE, 1, 0, 0, NULL},
	{"bookmark", XUI_CODE_MARKER_BOOKMARK, 1, 0, 0, NULL},
	{"modifiedLine", XUI_CODE_MARKER_MODIFIED_LINE, 1, 0, 0, NULL},
	{"savedLine", XUI_CODE_MARKER_SAVED_LINE, 1, 0, 0, NULL},
	{"errorLine", XUI_CODE_MARKER_ERROR_LINE, 1, 0, 0, NULL},
	{"warningLine", XUI_CODE_MARKER_WARNING_LINE, 1, 0, 0, NULL},
	{"infoLine", XUI_CODE_MARKER_INFO_LINE, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexCodeIndicatorStyleEnum[] = {
	{"squiggle", XUI_CODE_INDICATOR_SQUIGGLE, 1, 0, 0, NULL},
	{"underline", XUI_CODE_INDICATOR_UNDERLINE, 1, 0, 0, NULL},
	{"box", XUI_CODE_INDICATOR_BOX, 1, 0, 0, NULL},
	{"roundedBox", XUI_CODE_INDICATOR_ROUNDED_BOX, 1, 0, 0, NULL},
	{"background", XUI_CODE_INDICATOR_BACKGROUND, 1, 0, 0, NULL},
	{"textForeground", XUI_CODE_INDICATOR_TEXT_FOREGROUND, 1, 0, 0, NULL},
	{"searchResult", XUI_CODE_INDICATOR_SEARCH_RESULT, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexCodeMarginKindEnum[] = {
	{"change", XUI_CODE_MARGIN_CHANGE, 1, 0, 0, NULL},
	{"marker", XUI_CODE_MARGIN_MARKER, 1, 0, 0, NULL},
	{"fold", XUI_CODE_MARGIN_FOLD, 1, 0, 0, NULL},
	{"lineNumber", XUI_CODE_MARGIN_LINE_NUMBER, 1, 0, 0, NULL},
	{"diagnostic", XUI_CODE_MARGIN_DIAGNOSTIC, 1, 0, 0, NULL},
	{"custom", XUI_CODE_MARGIN_CUSTOM, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexStatusSectionEnum[] = {
	{"left", 0, 1, 0, 0, NULL},
	{"center", 0, 1, 0, 0, NULL},
	{"right", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexStatusTypeEnum[] = {
	{"text", 0, 1, 0, 0, NULL},
	{"progress", 0, 1, 0, 0, NULL},
	{"spacer", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexToolbarTypeEnum[] = {
	{"button", 0, 1, 0, 0, NULL},
	{"toggle", 0, 1, 0, 0, NULL},
	{"separator", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexMenuTypeEnum[] = {
	{"normal", 0, 1, 0, 0, NULL},
	{"check", 0, 1, 0, 0, NULL},
	{"radio", 0, 1, 0, 0, NULL},
	{"separator", 0, 1, 0, 0, NULL},
	{"submenu", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexButtonSemanticEnum[] = {
	{"default", 0, 1, 0, 0, NULL},
	{"primary", 0, 1, 0, 0, NULL},
	{"danger", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexButtonPatchStateEnum[] = {
	{"normal", 0, 1, 0, 0, NULL},
	{"hover", 0, 1, 0, 0, NULL},
	{"active", 0, 1, 0, 0, NULL},
	{"focus", 0, 1, 0, 0, NULL},
	{"disabled", 0, 1, 0, 0, NULL},
	{"checked", 0, 1, 0, 0, NULL}
};

static const xui_combobox_item_t g_arrComplexNinePatchModeEnum[] = {
	{"stretch", 0, 1, 0, 0, NULL},
	{"tile", 0, 1, 0, 0, NULL}
};

static int __uiDesignComplexLower(int c)
{
	if ( c >= 'A' && c <= 'Z' ) return c - 'A' + 'a';
	return c;
}

static int __uiDesignComplexEquals(const char* sA, const char* sB)
{
	if ( (sA == NULL) || (sB == NULL) ) return 0;
	return strcmp(sA, sB) == 0;
}

static int __uiDesignComplexTextEqualsNoCase(const char* sA, const char* sB)
{
	int a;
	int b;

	if ( (sA == NULL) || (sB == NULL) ) return 0;
	while ( sA[0] != 0 && sB[0] != 0 ) {
		a = __uiDesignComplexLower((unsigned char)sA[0]);
		b = __uiDesignComplexLower((unsigned char)sB[0]);
		if ( a != b ) return 0;
		++sA;
		++sB;
	}
	return sA[0] == 0 && sB[0] == 0;
}

static int __uiDesignComplexContainsNoCase(const char* sText, const char* sNeedle)
{
	char sHay[128];
	char sSub[64];
	int i;

	if ( (sText == NULL) || (sNeedle == NULL) ) return 0;
	for ( i = 0; sText[i] != 0 && i < (int)sizeof(sHay) - 1; ++i ) {
		sHay[i] = (char)__uiDesignComplexLower((unsigned char)sText[i]);
	}
	sHay[i] = 0;
	for ( i = 0; sNeedle[i] != 0 && i < (int)sizeof(sSub) - 1; ++i ) {
		sSub[i] = (char)__uiDesignComplexLower((unsigned char)sNeedle[i]);
	}
	sSub[i] = 0;
	return strstr(sHay, sSub) != NULL;
}

static void __uiDesignComplexCopy(char* sDst, int iCapacity, const char* sSrc)
{
	if ( (sDst == NULL) || (iCapacity <= 0) ) return;
	if ( sSrc == NULL ) sSrc = "";
	snprintf(sDst, (size_t)iCapacity, "%s", sSrc);
	sDst[iCapacity - 1] = '\0';
}

static void __uiDesignComplexTrim(char* sText)
{
	char* sStart;
	char* sEnd;
	size_t n;

	if ( sText == NULL ) return;
	sStart = sText;
	while ( sStart[0] == ' ' || sStart[0] == '\t' || sStart[0] == '\r' ) ++sStart;
	sEnd = sStart + strlen(sStart);
	while ( sEnd > sStart && (sEnd[-1] == ' ' || sEnd[-1] == '\t' || sEnd[-1] == '\r') ) --sEnd;
	n = (size_t)(sEnd - sStart);
	if ( sStart != sText && n > 0 ) memmove(sText, sStart, n);
	sText[n] = '\0';
}

static int __uiDesignComplexColumnType(const char* sTitle)
{
	if ( __uiDesignComplexContainsNoCase(sTitle, "color") || __uiDesignComplexContainsNoCase(sTitle, "colour") ) {
		return XUI_TABLE_CELL_TYPE_COLOR;
	}
	if ( __uiDesignComplexContainsNoCase(sTitle, "visible") ||
	     __uiDesignComplexContainsNoCase(sTitle, "enabled") ||
	     __uiDesignComplexContainsNoCase(sTitle, "disabled") ||
	     __uiDesignComplexContainsNoCase(sTitle, "checked") ||
	     __uiDesignComplexContainsNoCase(sTitle, "expanded") ||
	     __uiDesignComplexContainsNoCase(sTitle, "locked") ||
	     __uiDesignComplexContainsNoCase(sTitle, "leaf") ||
	     __uiDesignComplexContainsNoCase(sTitle, "closable") ||
	     __uiDesignComplexContainsNoCase(sTitle, "dockable") ||
	     __uiDesignComplexContainsNoCase(sTitle, "dirty") ) {
		return XUI_TABLE_CELL_TYPE_BOOL;
	}
	if ( __uiDesignComplexContainsNoCase(sTitle, "line") ||
	     __uiDesignComplexContainsNoCase(sTitle, "offset") ||
	     __uiDesignComplexContainsNoCase(sTitle, "column") ||
	     __uiDesignComplexContainsNoCase(sTitle, "level") ) {
		return XUI_TABLE_CELL_TYPE_INT;
	}
	if ( __uiDesignComplexTextEqualsNoCase(sTitle, "x") ||
	     __uiDesignComplexTextEqualsNoCase(sTitle, "y") ||
	     __uiDesignComplexTextEqualsNoCase(sTitle, "w") ||
	     __uiDesignComplexTextEqualsNoCase(sTitle, "h") ||
	     __uiDesignComplexContainsNoCase(sTitle, "width") ||
	     __uiDesignComplexContainsNoCase(sTitle, "height") ||
	     __uiDesignComplexContainsNoCase(sTitle, "rate") ||
	     __uiDesignComplexContainsNoCase(sTitle, "ratio") ||
	     __uiDesignComplexContainsNoCase(sTitle, "weight") ||
	     __uiDesignComplexContainsNoCase(sTitle, "fixed") ||
	     __uiDesignComplexTextEqualsNoCase(sTitle, "min") ||
	     __uiDesignComplexTextEqualsNoCase(sTitle, "max") ||
	     __uiDesignComplexContainsNoCase(sTitle, "flex") ||
	     __uiDesignComplexContainsNoCase(sTitle, "start") ||
	     __uiDesignComplexContainsNoCase(sTitle, "end") ) {
		return XUI_TABLE_CELL_TYPE_FLOAT;
	}
	if ( __uiDesignComplexContainsNoCase(sTitle, "count") ||
	     __uiDesignComplexContainsNoCase(sTitle, "hotkey") ||
	     __uiDesignComplexContainsNoCase(sTitle, "index") ||
	     __uiDesignComplexContainsNoCase(sTitle, "frame") ||
	     __uiDesignComplexContainsNoCase(sTitle, "route") ||
	     __uiDesignComplexContainsNoCase(sTitle, "state") ||
	     __uiDesignComplexContainsNoCase(sTitle, "flags") ||
	     __uiDesignComplexContainsNoCase(sTitle, "group") ||
	     __uiDesignComplexTextEqualsNoCase(sTitle, "id") ||
	     __uiDesignComplexContainsNoCase(sTitle, "itemid") ||
	     __uiDesignComplexContainsNoCase(sTitle, "parent") ) {
		return XUI_TABLE_CELL_TYPE_INT;
	}
	return XUI_TABLE_CELL_TYPE_TEXT;
}

static void __uiDesignComplexColumnSetEnum(ui_design_complex_column_config_t* pConfig,
	const xui_combobox_item_t* pItems, int iCount, int bUseValue)
{
	if ( (pConfig == NULL) || (pItems == NULL) || (iCount <= 0) ) return;
	pConfig->iType = XUI_TABLE_CELL_TYPE_ENUM;
	pConfig->pEnumItems = pItems;
	pConfig->iEnumCount = iCount;
	pConfig->bEnumUseValue = bUseValue;
}

static void __uiDesignComplexColumnSetRange(ui_design_complex_column_config_t* pConfig, float fMin, float fMax, int iPrecision)
{
	if ( pConfig == NULL ) return;
	pConfig->bRangeSet = 1;
	pConfig->fMin = fMin;
	pConfig->fMax = fMax;
	pConfig->iPrecision = iPrecision;
}

static void __uiDesignComplexColumnConfig(const ui_design_node_t* pNode, const char* sId, int iColumn,
	const char* sTitle, ui_design_complex_column_config_t* pConfig)
{
	if ( pConfig == NULL ) return;
	memset(pConfig, 0, sizeof(*pConfig));
	pConfig->iType = __uiDesignComplexColumnType(sTitle);
	pConfig->iPrecision = -1;
	pConfig->bRequired = (iColumn == 0);
	if ( sId == NULL ) return;
	if ( __uiDesignComplexEquals(sId, "data.source") ) {
		pConfig->iType = XUI_TABLE_CELL_TYPE_FILE;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.commands") ) {
		if ( iColumn == 0 ) {
			__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexCanvasCommandEnum, UI_DESIGN_COUNT_OF(g_arrComplexCanvasCommandEnum), 0);
		} else if ( iColumn == 1 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FILE;
		} else if ( iColumn >= 2 && iColumn <= 16 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, -1000000.0f, 1000000.0f, 3);
		} else if ( iColumn == 17 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
		} else if ( iColumn == 18 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		} else if ( iColumn == 19 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.statePatches") ) {
		if ( iColumn == 0 ) {
			__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexButtonPatchStateEnum, UI_DESIGN_COUNT_OF(g_arrComplexButtonPatchStateEnum), 0);
		} else if ( iColumn == 1 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FILE;
		} else if ( iColumn >= 2 && iColumn <= 9 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 100000.0f, 2);
		} else if ( iColumn == 10 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		} else if ( iColumn == 11 ) {
			__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexNinePatchModeEnum, UI_DESIGN_COUNT_OF(g_arrComplexNinePatchModeEnum), 0);
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.decorations") ) {
		if ( iColumn == 0 ) {
			__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexInputDecorationSideEnum, UI_DESIGN_COUNT_OF(g_arrComplexInputDecorationSideEnum), 0);
		} else if ( iColumn == 1 ) {
			__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexInputDecorationKindEnum, UI_DESIGN_COUNT_OF(g_arrComplexInputDecorationKindEnum), 0);
		} else if ( iColumn == 2 ) {
			__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexInputDecorationVisibleEnum, UI_DESIGN_COUNT_OF(g_arrComplexInputDecorationVisibleEnum), 0);
		} else if ( iColumn == 3 || iColumn == 4 || (iColumn >= 8 && iColumn <= 11) ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 100000.0f, 2);
		} else if ( iColumn == 5 ) {
			__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexInputDecorationIconEnum, UI_DESIGN_COUNT_OF(g_arrComplexInputDecorationIconEnum), 0);
		} else if ( iColumn == 7 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FILE;
		} else if ( iColumn >= 12 && iColumn <= 15 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.menuTitles") ) {
		if ( iColumn == 0 ) {
			if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
				__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexCodeEditMenuCommandEnum, UI_DESIGN_COUNT_OF(g_arrComplexCodeEditMenuCommandEnum), 0);
			} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_TERMINAL ) {
				__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexTerminalMenuCommandEnum, UI_DESIGN_COUNT_OF(g_arrComplexTerminalMenuCommandEnum), 0);
			} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_DOCK_PANEL ) {
				__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexDockPanelMenuCommandEnum, UI_DESIGN_COUNT_OF(g_arrComplexDockPanelMenuCommandEnum), 0);
			} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_TIMELINE_VIEW ) {
				__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexTimelineMenuCommandEnum, UI_DESIGN_COUNT_OF(g_arrComplexTimelineMenuCommandEnum), 0);
			} else {
				__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexInputMenuCommandEnum, UI_DESIGN_COUNT_OF(g_arrComplexInputMenuCommandEnum), 0);
			}
		} else if ( iColumn == 1 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.tooltips") ) {
		if ( iColumn == 0 ) {
			if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_DOCK_PANEL ) {
				__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexDockPanelTooltipEnum, UI_DESIGN_COUNT_OF(g_arrComplexDockPanelTooltipEnum), 0);
			} else {
				pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
			}
		} else if ( iColumn == 1 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "style.inlineProperties") ) {
		if ( iColumn == 1 ) {
			__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexStyleValueTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexStyleValueTypeEnum), 0);
		} else if ( iColumn == 2 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		} else if ( iColumn == 3 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "cache.stateIds") ) {
		if ( iColumn == 0 || iColumn == 1 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 2147483647.0f, 0);
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "layout.tableRowsConfig") || __uiDesignComplexEquals(sId, "layout.tableColumnsConfig") ) {
		if ( iColumn == 0 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 2147483647.0f, 0);
		} else if ( iColumn == 1 ) {
			__uiDesignComplexColumnSetEnum(pConfig, g_arrSizeModeEnum, UI_DESIGN_COUNT_OF(g_arrSizeModeEnum), 0);
		} else if ( iColumn >= 2 && iColumn <= 5 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 1000000.0f, 2);
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.palette") ) {
		if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_TERMINAL ) {
			if ( iColumn == 0 ) {
				pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
				__uiDesignComplexColumnSetRange(pConfig, 0.0f, 255.0f, 0);
			} else if ( iColumn == 1 ) {
				pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
			}
		} else if ( iColumn == 0 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.slots") ) {
		if ( iColumn == 1 || iColumn == 5 || iColumn == 6 || iColumn == 7 || iColumn == 10 || iColumn == 11 || iColumn == 18 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		} else if ( iColumn == 4 || iColumn == 12 || iColumn == 20 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		} else if ( iColumn == 8 || iColumn == 9 || (iColumn >= 14 && iColumn <= 17) || iColumn == 19 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 1000000.0f, 3);
		} else if ( iColumn == 13 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FILE;
		} else {
			pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.selectedSlots") ) {
		if ( iColumn == 0 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		if ( iColumn == 1 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.selection") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_TERMINAL ) {
		pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.channels") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_VIRTUAL_JOYSTICK ) {
		if ( iColumn == 0 ) {
			__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexJoystickChannelEnum, UI_DESIGN_COUNT_OF(g_arrComplexJoystickChannelEnum), 0);
		} else if ( iColumn == 1 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		} else if ( iColumn == 2 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 1.0f, 2);
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.options") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_RADIO_GROUP ) {
		if ( iColumn == 1 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.seriesList") ) {
		if ( iColumn == 1 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexChartSeriesTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexChartSeriesTypeEnum), 0);
		if ( iColumn == 3 || iColumn == 5 || iColumn == 7 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		if ( iColumn == 4 || iColumn == 6 || iColumn == 13 || iColumn == 14 ) pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		if ( iColumn == 9 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexChartSymbolEnum, UI_DESIGN_COUNT_OF(g_arrComplexChartSymbolEnum), 0);
		if ( iColumn >= 10 && iColumn <= 12 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 1000000.0f, 3);
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.series") ) {
		if ( iColumn >= 2 && iColumn <= 4 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, -1000000.0f, 1000000.0f, 3);
		}
		if ( iColumn == 5 ) pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.columns") ) {
		if ( iColumn == 1 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 1.0f, 10000.0f, 0);
		} else if ( iColumn == 2 ) {
			__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexPropertyTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexPropertyTypeEnum), 0);
		} else if ( iColumn == 4 || iColumn == 5 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		} else if ( iColumn == 6 ) {
			__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexAlignEnum, UI_DESIGN_COUNT_OF(g_arrComplexAlignEnum), 0);
		} else if ( iColumn == 7 || iColumn == 8 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 10000.0f, 0);
		} else if ( iColumn >= 9 && iColumn <= 11 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.rowStyles") ) {
		if ( iColumn == 0 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		if ( iColumn == 1 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 1.0f, 10000.0f, 0);
		}
		if ( iColumn == 2 || iColumn == 3 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		if ( iColumn >= 4 && iColumn <= 6 ) pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.cellStyles") ) {
		if ( iColumn == 0 || iColumn == 1 || iColumn == 4 || iColumn == 5 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		if ( iColumn == 2 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexPropertyTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexPropertyTypeEnum), 0);
		if ( iColumn >= 6 && iColumn <= 9 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		if ( iColumn >= 10 && iColumn <= 12 ) pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.merges") ) {
		if ( iColumn >= 0 && iColumn <= 3 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		if ( iColumn == 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.editorConfigs") ) {
		if ( iColumn == 0 || iColumn == 1 || iColumn == 7 || iColumn == 22 || iColumn == 24 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		if ( iColumn == 2 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexPropertyTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexPropertyTypeEnum), 0);
		if ( iColumn >= 4 && iColumn <= 6 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, -1000000.0f, 1000000.0f, 3);
		}
		if ( iColumn == 8 || iColumn == 9 || iColumn == 13 || iColumn == 23 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		if ( iColumn == 12 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexDateModeEnum, UI_DESIGN_COUNT_OF(g_arrComplexDateModeEnum), 0);
		if ( iColumn == 15 || iColumn == 16 ) pConfig->iType = XUI_TABLE_CELL_TYPE_DATETIME;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.panes") ) {
		if ( iColumn == 1 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexSplitPaneModeEnum, UI_DESIGN_COUNT_OF(g_arrComplexSplitPaneModeEnum), 0);
		if ( iColumn >= 2 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 100000.0f, 2);
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.sections") ) {
		if ( iColumn == 1 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		if ( iColumn == 3 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.messages") ) {
		if ( iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
		if ( iColumn == 3 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexMessageTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexMessageTypeEnum), 0);
		if ( iColumn == 5 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.timelineLayers") ) {
		if ( iColumn == 1 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		if ( iColumn == 3 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 1.0f, 10000.0f, 1);
		}
		if ( iColumn == 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.timelineFrames") ) {
		if ( iColumn == 1 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		if ( iColumn == 2 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexTimelineFrameEnum, UI_DESIGN_COUNT_OF(g_arrComplexTimelineFrameEnum), 0);
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.timelineSpans") ) {
		if ( iColumn == 1 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		if ( iColumn == 3 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexTimelineSpanEnum, UI_DESIGN_COUNT_OF(g_arrComplexTimelineSpanEnum), 0);
		if ( iColumn == 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
		if ( iColumn == 5 ) pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.layers") ) {
		if ( iColumn == 0 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexTimelineKindEnum, UI_DESIGN_COUNT_OF(g_arrComplexTimelineKindEnum), 0);
		if ( iColumn == 6 ) pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.selection") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_TIMELINE_VIEW ) {
		if ( iColumn == 1 || iColumn == 3 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		if ( iColumn == 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.properties") ) {
		if ( iColumn == 2 || iColumn == 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
		if ( iColumn == 3 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexPropertyTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexPropertyTypeEnum), 0);
		if ( iColumn == 8 || iColumn == 14 || iColumn == 15 || iColumn == 19 || iColumn == 29 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		if ( iColumn >= 10 && iColumn <= 12 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, -1000000.0f, 1000000.0f, 3);
		}
		if ( iColumn == 13 || iColumn == 28 || iColumn == 30 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		if ( iColumn == 18 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexDateModeEnum, UI_DESIGN_COUNT_OF(g_arrComplexDateModeEnum), 0);
		if ( iColumn == 21 || iColumn == 22 ) pConfig->iType = XUI_TABLE_CELL_TYPE_DATETIME;
		if ( iColumn == 25 || iColumn == 26 || iColumn == 27 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.windows") ) {
		if ( iColumn == 1 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexDockSideEnum, UI_DESIGN_COUNT_OF(g_arrComplexDockSideEnum), 0);
		if ( iColumn == 2 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 1.0f, 2);
		}
		if ( iColumn == 3 || iColumn == 4 || iColumn == 7 || iColumn == 8 || iColumn == 10 || iColumn == 11 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		if ( iColumn == 9 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexDockRegionEnum, UI_DESIGN_COUNT_OF(g_arrComplexDockRegionEnum), 0);
		if ( iColumn >= 12 && iColumn <= 15 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, -100000.0f, 100000.0f, 1);
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.regions") ) {
		if ( iColumn == 0 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexDockRegionEnum, UI_DESIGN_COUNT_OF(g_arrComplexDockRegionEnum), 0);
		if ( iColumn == 1 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexDockSizeModeEnum, UI_DESIGN_COUNT_OF(g_arrComplexDockSizeModeEnum), 0);
		if ( iColumn == 2 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 100000.0f, 2);
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.content") ) {
		if ( iColumn >= 1 && iColumn <= 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
		if ( iColumn == 5 ) pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		if ( iColumn == 6 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexAlignEnum, UI_DESIGN_COUNT_OF(g_arrComplexAlignEnum), 0);
		return;
	}
	if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_FILE_DIALOG && __uiDesignComplexEquals(sId, "data.entries") ) {
		if ( iColumn == 1 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexFileDialogEntryKindEnum, UI_DESIGN_COUNT_OF(g_arrComplexFileDialogEntryKindEnum), 0);
		if ( iColumn == 4 || iColumn == 5 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		return;
	}
	if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_FILE_DIALOG && __uiDesignComplexEquals(sId, "data.roots") ) {
		if ( iColumn == 2 || iColumn == 3 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.buttonTitles") ) {
		if ( iColumn == 0 ) {
			if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_FILE_DIALOG ) {
				__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexFileDialogTextTargetEnum, UI_DESIGN_COUNT_OF(g_arrComplexFileDialogTextTargetEnum), 0);
			} else {
				__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexMsgBoxButtonTitleEnum, UI_DESIGN_COUNT_OF(g_arrComplexMsgBoxButtonTitleEnum), 0);
			}
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.customButtons") ) {
		if ( iColumn == 1 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		if ( iColumn == 2 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexButtonSemanticEnum, UI_DESIGN_COUNT_OF(g_arrComplexButtonSemanticEnum), 0);
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.toasts") ) {
		if ( iColumn == 0 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexToastTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexToastTypeEnum), 0);
		if ( iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
		if ( iColumn == 3 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 3600.0f, 1);
		}
		if ( iColumn == 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		if ( iColumn == 5 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 1.0f, 2);
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.nodeTypes") ) {
		if ( iColumn == 0 || iColumn == 1 || iColumn == 2 || iColumn == 3 || iColumn == 4 || iColumn == 7 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		if ( iColumn == 5 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		if ( iColumn == 6 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.configFields") ) {
		if ( iColumn == 3 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexWorkflowConfigKindEnum, UI_DESIGN_COUNT_OF(g_arrComplexWorkflowConfigKindEnum), 1);
		else if ( iColumn == 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		else if ( iColumn == 6 || iColumn == 7 ) pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
		else if ( iColumn == 9 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
		else pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.nodeConfigs") ) {
		if ( iColumn == 2 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexWorkflowConfigKindEnum, UI_DESIGN_COUNT_OF(g_arrComplexWorkflowConfigKindEnum), 1);
		else if ( iColumn == 3 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
		else pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.variables") ) {
		if ( iColumn == 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
		else pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.ports") || __uiDesignComplexEquals(sId, "data.typePorts") ) {
		if ( iColumn == 3 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexFlowDirectionEnum, UI_DESIGN_COUNT_OF(g_arrComplexFlowDirectionEnum), 1);
		else if ( iColumn == 4 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexFlowKindEnum, UI_DESIGN_COUNT_OF(g_arrComplexFlowKindEnum), 1);
		else if ( iColumn >= 5 && iColumn <= 7 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		else pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.styles") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		if ( iColumn == 0 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexCodeStyleEnum, UI_DESIGN_COUNT_OF(g_arrComplexCodeStyleEnum), 0);
		else if ( iColumn == 1 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_COLOR;
		else if ( iColumn >= 3 && iColumn <= 6 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.tokenStyles") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		if ( iColumn == 0 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexCodeTokenEnum, UI_DESIGN_COUNT_OF(g_arrComplexCodeTokenEnum), 0);
		else if ( iColumn == 1 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexCodeStyleEnum, UI_DESIGN_COUNT_OF(g_arrComplexCodeStyleEnum), 0);
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.markers") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		if ( iColumn == 0 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		else if ( iColumn == 1 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexCodeMarkerEnum, UI_DESIGN_COUNT_OF(g_arrComplexCodeMarkerEnum), 0);
		else if ( iColumn == 3 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.indicators") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		if ( iColumn == 0 || iColumn == 2 || iColumn == 3 || iColumn == 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		else if ( iColumn == 1 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexCodeIndicatorStyleEnum, UI_DESIGN_COUNT_OF(g_arrComplexCodeIndicatorStyleEnum), 0);
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.margins") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		if ( iColumn == 0 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		else if ( iColumn == 1 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexCodeMarginKindEnum, UI_DESIGN_COUNT_OF(g_arrComplexCodeMarginKindEnum), 0);
		else if ( iColumn == 2 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			__uiDesignComplexColumnSetRange(pConfig, 0.0f, 10000.0f, 1);
		} else if ( iColumn >= 3 && iColumn <= 5 ) {
			pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.selections") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		if ( iColumn >= 0 && iColumn <= 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		else if ( iColumn == 3 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.folds") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		if ( iColumn >= 0 && iColumn <= 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		else if ( iColumn == 3 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.diagnostics") ) {
		if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
			if ( iColumn == 0 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexDiagnosticSeverityEnum, UI_DESIGN_COUNT_OF(g_arrComplexDiagnosticSeverityEnum), 0);
			else if ( iColumn == 1 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
			else if ( iColumn == 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
			else pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		} else {
			if ( iColumn == 0 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexDiagnosticSeverityEnum, UI_DESIGN_COUNT_OF(g_arrComplexDiagnosticSeverityEnum), 1);
			else if ( iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
			else pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.nodes") ) {
		if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_FLOW_GRAPH ) {
			if ( iColumn == 0 || iColumn == 1 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
			if ( iColumn >= 3 && iColumn <= 6 ) pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			if ( iColumn == 7 || iColumn == 9 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
			if ( iColumn == 8 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexFlowNodeRunEnum, UI_DESIGN_COUNT_OF(g_arrComplexFlowNodeRunEnum), 1);
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_WORKFLOW ) {
			if ( iColumn == 0 || iColumn == 1 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
			if ( iColumn >= 3 && iColumn <= 6 ) pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			if ( iColumn == 7 || iColumn == 9 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
			if ( iColumn == 8 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexFlowNodeRunEnum, UI_DESIGN_COUNT_OF(g_arrComplexFlowNodeRunEnum), 1);
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_TREEVIEW ) {
			if ( iColumn == 0 || iColumn == 1 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
			if ( iColumn >= 4 && iColumn <= 8 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.edges") ) {
		if ( iColumn >= 0 && iColumn <= 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXT;
		if ( iColumn == 5 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexFlowRouteEnum, UI_DESIGN_COUNT_OF(g_arrComplexFlowRouteEnum), 1);
		if ( iColumn == 6 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexFlowKindEnum, UI_DESIGN_COUNT_OF(g_arrComplexFlowKindEnum), 1);
		if ( iColumn == 7 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexFlowEdgeRunEnum, UI_DESIGN_COUNT_OF(g_arrComplexFlowEdgeRunEnum), 1);
		if ( iColumn == 8 ) pConfig->iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
		if ( iColumn >= 9 && iColumn <= 11 ) pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.items") ) {
		if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_BREADCRUMB ) {
			if ( iColumn == 1 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
			if ( iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_COMBOBOX ) {
			if ( iColumn == 1 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
			if ( iColumn == 3 || iColumn == 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
			if ( iColumn == 5 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_LISTVIEW ) {
			if ( iColumn == 1 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_MENU_BAR ) {
			if ( iColumn == 1 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
			if ( iColumn == 3 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_TOOLBAR ) {
			if ( iColumn == 1 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexToolbarTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexToolbarTypeEnum), 0);
			if ( iColumn == 2 || iColumn == 3 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
			if ( iColumn == 5 || iColumn == 6 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
			if ( iColumn == 7 ) pConfig->iType = XUI_TABLE_CELL_TYPE_FILE;
			if ( iColumn >= 8 && iColumn <= 11 ) pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_TABS ) {
			if ( iColumn == 1 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
			if ( iColumn == 3 ) pConfig->iType = XUI_TABLE_CELL_TYPE_FILE;
			if ( iColumn >= 4 && iColumn <= 7 ) pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_STATUS_BAR ) {
			if ( iColumn == 1 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexStatusTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexStatusTypeEnum), 0);
			if ( iColumn == 2 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexStatusSectionEnum, UI_DESIGN_COUNT_OF(g_arrComplexStatusSectionEnum), 0);
			if ( iColumn == 3 || iColumn == 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
			if ( iColumn >= 5 && iColumn <= 9 ) pConfig->iType = XUI_TABLE_CELL_TYPE_FLOAT;
			if ( iColumn == 10 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_MENU ) {
			if ( iColumn == 1 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexMenuTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexMenuTypeEnum), 0);
			if ( iColumn >= 2 && iColumn <= 5 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
			if ( iColumn == 7 || iColumn == 8 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_CASCADER ) {
			if ( iColumn == 1 || iColumn == 2 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
			if ( iColumn == 3 || iColumn == 4 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		}
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.menus") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_MENU_BAR ) {
		if ( iColumn == 2 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexMenuTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexMenuTypeEnum), 0);
		if ( iColumn >= 3 && iColumn <= 6 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		if ( iColumn == 8 || iColumn == 9 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.submenus") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_MENU_BAR ) {
		if ( iColumn == 3 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexMenuTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexMenuTypeEnum), 0);
		if ( iColumn >= 4 && iColumn <= 7 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		if ( iColumn == 9 || iColumn == 10 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		return;
	}
	if ( __uiDesignComplexEquals(sId, "data.submenus") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_MENU ) {
		if ( iColumn == 2 ) __uiDesignComplexColumnSetEnum(pConfig, g_arrComplexMenuTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexMenuTypeEnum), 0);
		if ( iColumn >= 3 && iColumn <= 6 ) pConfig->iType = XUI_TABLE_CELL_TYPE_BOOL;
		if ( iColumn == 8 || iColumn == 9 ) pConfig->iType = XUI_TABLE_CELL_TYPE_INT;
		return;
	}
	if ( __uiDesignComplexTextEqualsNoCase(sTitle, "Type") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_STATUS_BAR ) {
		__uiDesignComplexColumnSetEnum(pConfig, g_arrComplexStatusTypeEnum, UI_DESIGN_COUNT_OF(g_arrComplexStatusTypeEnum), 0);
	}
}

static ui_design_complex_schema_t __uiDesignComplexSchema(const ui_design_node_t* pNode, const char* sId)
{
	ui_design_complex_schema_t tSchema;

	memset(&tSchema, 0, sizeof(tSchema));
	tSchema.bStructured = 1;
	tSchema.cDelimiter = '|';
	tSchema.sColumns = "Text";
	if ( sId == NULL ) return tSchema;
	if ( __uiDesignComplexEquals(sId, "data.commands") ) {
		tSchema.sColumns = "Type|Source|X|Y|W|H|X2|Y2|X3|Y3|SrcX|SrcY|SrcW|SrcH|Radius|Size|LineWidth|Text|Color|Flags";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.source") ) {
		tSchema.sColumns = "Source";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.statePatches") ) {
		tSchema.sColumns = "State|Source|X|Y|W|H|SliceLeft|SliceTop|SliceRight|SliceBottom|Color|Mode";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.decorations") ) {
		tSchema.sColumns = "Side|Kind|Visible|Width|Padding|Icon|Text|Source|SourceX|SourceY|SourceW|SourceH|Color|HoverColor|ActiveColor|DisabledColor";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.menuTitles") ) {
		tSchema.sColumns = "Command|Title";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.tooltips") ) {
		tSchema.sColumns = "Target|Text";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "style.inlineProperties") ) {
		tSchema.sColumns = "Name|Type|Value|DirtyFlags";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "cache.stateIds") ) {
		tSchema.sColumns = "Index|StateId";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "layout.tableRowsConfig") || __uiDesignComplexEquals(sId, "layout.tableColumnsConfig") ) {
		tSchema.sColumns = "Index|Mode|Value|Min|Max|Weight";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.palette") ) {
		tSchema.sColumns = (pNode != NULL && pNode->iType == UI_DESIGN_NODE_TERMINAL) ? "Index|Color" : "Color";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.seriesList") ) {
		tSchema.sColumns = "Id|Type|Name|Visible|Color|AreaFill|AreaColor|Smooth|Dash|Symbol|SymbolSize|RadiusMin|RadiusMax|ValueMinColor|ValueMaxColor";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.series") ) {
		tSchema.sColumns = "Series|Label|X|Y|Value|Color";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.columns") ) {
		tSchema.sColumns = "Title|Width|Type|Id|Visible|Resizable|Align|Min|Max|Background|Text|Grid";
		tSchema.bExplodeCommaRows = 1;
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.rowStyles") ) {
		tSchema.sColumns = "Row|Height|Selected|Disabled|Background|Text|Grid";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.cellStyles") ) {
		tSchema.sColumns = "Row|Column|Type|Tooltip|RowSpan|ColSpan|Disabled|Editing|Invalid|Dirty|Background|Text|Grid";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.merges") ) {
		tSchema.sColumns = "Row|Column|RowSpan|ColSpan|Enabled";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.editorConfigs") ) {
		tSchema.sColumns = "Row|Column|Type|Options|Min|Max|Step|Precision|Nullable|Alpha|ActionText|FileFilter|DateMode|ShowSecond|DateFormat|DateMin|DateMax|DefaultSpan|RangeSeparator|Palette|EnumValues|EnumEnabled|EnumSelected|EnumUseValue|EnumSelectedValue";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.rows") ) {
		tSchema.cDelimiter = ',';
		tSchema.sColumns = "Cell 1|Cell 2|Cell 3|Cell 4|Cell 5|Cell 6|Cell 7|Cell 8";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.tags") || __uiDesignComplexEquals(sId, "data.steps") ) {
		tSchema.sColumns = "Text";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.options") ) {
		tSchema.sColumns = (pNode != NULL && pNode->iType == UI_DESIGN_NODE_RADIO_GROUP) ? "Text|Enabled|Checked" : "Text";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.slots") ) {
		tSchema.sColumns = "Text|Count|Hotkey|Flags|QualityColor|ItemId|MaxCount|SlotId|CooldownRate|DurabilityRate|SlotType|ItemType|IconTint|IconSource|IconX|IconY|IconW|IconH|AnimationFlags|AnimationScale|AnimationTint";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.selectedSlots") ) {
		tSchema.sColumns = "Slot|Selected";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.channels") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_VIRTUAL_JOYSTICK ) {
		tSchema.sColumns = "Channel|Pressed|Value";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.panes") ) {
		tSchema.sColumns = "Title|Mode|Weight|Fixed|Min|Max";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.sections") ) {
		tSchema.sColumns = "Title|Expanded|Disabled|Id";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.messages") ) {
		tSchema.sColumns = "Sender|Time|Message|Type|Id|Flags";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.timelineLayers") ) {
		tSchema.sColumns = "Name|Visible|Locked|Height|Color";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.timelineFrames") ) {
		tSchema.sColumns = "Layer|Frame|Type";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.timelineSpans") ) {
		tSchema.sColumns = "Layer|Start|End|Type|Label|Color|CustomType";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.layers") ) {
		tSchema.sColumns = "Kind|Layer|Frame/Start|Type/End|Type|Label|Color|CustomType";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.selection") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_TIMELINE_VIEW ) {
		tSchema.sColumns = "Layer|Frame|EndLayer|EndFrame|Selected";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.selection") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_TERMINAL ) {
		tSchema.sColumns = "AnchorLine|AnchorColumn|EndLine|EndColumn";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.properties") ) {
		tSchema.sColumns = "Category|Name|Value|Type|Description|Default|Flags|Id|Expanded|Options|Min|Max|Step|Precision|Nullable|Alpha|ActionText|FileFilter|DateMode|ShowSecond|DateFormat|DateMin|DateMax|DefaultSpan|RangeSeparator|Palette|EnumValues|EnumEnabled|EnumSelected|EnumUseValue|EnumSelectedValue";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.windows") ) {
		tSchema.sColumns = "Title|Side|Ratio|Closable|Dockable|Content|Pane|Active|AutoHide|Region|Hidden|Floating|Float X|Float Y|Float W|Float H";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.regions") ) {
		tSchema.sColumns = "Region|Mode|Value";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.content") ) {
		tSchema.sColumns = "Text|X|Y|W|H|Color|Align";
		return tSchema;
	}
	if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_FILE_DIALOG && __uiDesignComplexEquals(sId, "data.entries") ) {
		tSchema.sColumns = "Name|Kind|Size|Modified|Selected|Enabled";
		return tSchema;
	}
	if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_FILE_DIALOG && __uiDesignComplexEquals(sId, "data.roots") ) {
		tSchema.sColumns = "Label|Path|Selected|Enabled";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.buttonTitles") ) {
		tSchema.sColumns = (pNode != NULL && pNode->iType == UI_DESIGN_NODE_FILE_DIALOG) ? "Target|Text" : "Button|Title";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.customButtons") ) {
		tSchema.sColumns = "Text|Result|Semantic";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.toasts") ) {
		tSchema.sColumns = "Type|Title|Message|Duration|Clickable|Progress";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.filter") ) {
		tSchema.sColumns = "Name|Pattern";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.nodeTypes") ) {
		tSchema.sColumns = "Id|Title|Category|Inputs|Outputs|Version|Description|Icon";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.configFields") ) {
		tSchema.sColumns = "Type|Id|Title|Kind|Required|Default|Min|Max|Options|Children|ExpressionLanguage|RefScope|RefType|Group|Tab";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.nodeConfigs") ) {
		tSchema.sColumns = "Node|Key|Type|Value";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.typePorts") ) {
		tSchema.sColumns = "Type|Id|Title|Direction|Kind|Required|Multi|Dynamic|DataType";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.ports") ) {
		tSchema.sColumns = "Node|Id|Title|Direction|Kind|Required|Multi|Dynamic|DataType";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.styles") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		tSchema.sColumns = "Style|Foreground|Background|Bold|Italic|Underline|Strike";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.tokenStyles") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		tSchema.sColumns = "Token|Style";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.markers") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		tSchema.sColumns = "Line|Marker|Flags|Tooltip";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.indicators") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		tSchema.sColumns = "Indicator|Style|StartOffset|EndOffset|Flags";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.margins") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		tSchema.sColumns = "Id|Kind|Width|Visible|Clickable|Resizable";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.selections") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		tSchema.sColumns = "Anchor|Caret|PreferredColumn|Flags";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.folds") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
		tSchema.sColumns = "StartLine|EndLine|Level|Flags";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.diagnostics") ) {
		tSchema.sColumns = (pNode != NULL && pNode->iType == UI_DESIGN_NODE_CODE_EDIT) ?
			"Severity|StartOffset|EndOffset|Code|Message|Source" : "Severity|Code|Message|Node|Edge|Path";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.variables") ) {
		tSchema.sColumns = "Id|Title|Type|Scope|Default";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.nodes") ) {
		if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_FLOW_GRAPH ) {
			tSchema.sColumns = "Id|Type|Title|X|Y|W|H|Summary|RunState|RunPreview";
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_WORKFLOW ) {
			tSchema.sColumns = "Type|Id|Title|X|Y|W|H|Summary|RunState|RunPreview";
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_TREEVIEW ) {
			tSchema.sColumns = "Id|Parent|Depth|Text|Expanded|Enabled|Icon|CheckBox|Checked";
			tSchema.bTreeIndentRows = 1;
		} else {
			tSchema.sColumns = "Text";
		}
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.edges") ) {
		if ( pNode != NULL && (pNode->iType == UI_DESIGN_NODE_FLOW_GRAPH || pNode->iType == UI_DESIGN_NODE_WORKFLOW) ) {
			tSchema.sColumns = "Id|FromNode|FromPort|ToNode|ToPort|Route|Kind|RunState|RunPreview|RouteBias|SourceOffset|TargetOffset";
		} else {
			tSchema.sColumns = "Id|FromNode|FromPort|ToNode|ToPort|Route";
		}
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.items") ) {
		if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_BREADCRUMB ) {
			tSchema.sColumns = "Text|Clickable|Value";
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_COMBOBOX ) {
			tSchema.sColumns = "Text|Enabled|Selected|Value|Icon|Separator";
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_LISTVIEW ) {
			tSchema.sColumns = "Text|Enabled|Selected";
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_TABS ) {
			tSchema.sColumns = "Title|Enabled|Dirty|IconSource|IconX|IconY|IconW|IconH";
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_MENU_BAR ) {
			tSchema.sColumns = "Text|Enabled|Mnemonic|Value";
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_TOOLBAR ) {
			tSchema.sColumns = "Text|Type|Enabled|Checked|Tooltip|Group|Value|IconSource|IconX|IconY|IconW|IconH";
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_STATUS_BAR ) {
			tSchema.sColumns = "Text|Type|Section|Enabled|Clickable|Width|Flex|Min|Max|Progress|Value";
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_MENU ) {
			tSchema.sColumns = "Text|Type|Enabled|Checked|Default|Danger|Shortcut|Value|Icon";
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_CASCADER ) {
			tSchema.sColumns = "Text|Value|ParentIndex|Leaf|Disabled";
			tSchema.bCascaderPathRows = 1;
		} else {
			tSchema.sColumns = "Text";
		}
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.menus") && pNode != NULL && pNode->iType == UI_DESIGN_NODE_MENU_BAR ) {
		tSchema.sColumns = "Menu|Text|Type|Enabled|Checked|Default|Danger|Shortcut|Value|Icon";
		return tSchema;
	}
	if ( __uiDesignComplexEquals(sId, "data.submenus") ) {
		if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_MENU_BAR ) {
			tSchema.sColumns = "Menu|Parent|Text|Type|Enabled|Checked|Default|Danger|Shortcut|Value|Icon";
		} else if ( pNode != NULL && pNode->iType == UI_DESIGN_NODE_MENU ) {
			tSchema.sColumns = "Parent|Text|Type|Enabled|Checked|Default|Danger|Shortcut|Value|Icon";
		} else {
			tSchema.sColumns = "Parent|Text";
		}
		return tSchema;
	}
	return tSchema;
}

static int __uiDesignComplexNextLine(const char** ppText, char* sLine, int iCapacity)
{
	const char* sText;
	int i;

	if ( (ppText == NULL) || (sLine == NULL) || (iCapacity <= 0) ) return 0;
	sText = *ppText;
	if ( (sText == NULL) || (sText[0] == 0) ) return 0;
	i = 0;
	while ( sText[0] != 0 && sText[0] != '\n' ) {
		if ( i < iCapacity - 1 ) sLine[i++] = sText[0];
		++sText;
	}
	if ( sText[0] == '\n' ) ++sText;
	sLine[i] = 0;
	if ( i > 0 && sLine[i - 1] == '\r' ) sLine[i - 1] = 0;
	*ppText = sText;
	return 1;
}

static int __uiDesignComplexSplit(char* sLine, char cDelimiter, char** arrFields, int iCapacity, int bTrimFields)
{
	char* sCursor;
	int iCount;
	int i;

	if ( (sLine == NULL) || (arrFields == NULL) || (iCapacity <= 0) ) return 0;
	iCount = 0;
	arrFields[iCount++] = sLine;
	if ( cDelimiter != 0 ) {
		sCursor = sLine;
		while ( sCursor[0] != 0 ) {
			if ( sCursor[0] == cDelimiter ) {
				sCursor[0] = 0;
				if ( iCount < iCapacity ) arrFields[iCount++] = sCursor + 1;
			}
			++sCursor;
		}
	}
	if ( bTrimFields ) {
		for ( i = 0; i < iCount; ++i ) __uiDesignComplexTrim(arrFields[i]);
	}
	return iCount;
}

static int __uiDesignComplexAppend(char* sBuffer, int iCapacity, int* pLength, const char* sText)
{
	int iLen;
	int iCopy;

	if ( (sBuffer == NULL) || (pLength == NULL) || (iCapacity <= 0) || (*pLength >= iCapacity - 1) ) return 0;
	if ( sText == NULL ) sText = "";
	iLen = (int)strlen(sText);
	iCopy = iCapacity - 1 - *pLength;
	if ( iCopy > iLen ) iCopy = iLen;
	if ( iCopy > 0 ) {
		memcpy(sBuffer + *pLength, sText, (size_t)iCopy);
		*pLength += iCopy;
		sBuffer[*pLength] = 0;
	}
	return iCopy == iLen;
}

static int __uiDesignComplexEditorSelectedRow(ui_design_app_t* pApp)
{
	xui_widget pTable;
	int iRow;

	if ( (pApp == NULL) || (pApp->pComplexEditorGrid == NULL) ) return -1;
	pTable = xuiTableGridGetTableView(pApp->pComplexEditorGrid);
	if ( pTable == NULL ) return -1;
	iRow = xuiTableViewGetSelectedRow(pTable);
	if ( iRow < 0 || iRow >= pApp->iComplexEditorRowCount ) return -1;
	return iRow;
}

static int __uiDesignComplexRowKindIs(ui_design_app_t* pApp, int iRow, const char* sKind)
{
	if ( (pApp == NULL) || (sKind == NULL) || (iRow < 0) || (iRow >= pApp->iComplexEditorRowCount) ) return 0;
	return __uiDesignComplexTextEqualsNoCase(pApp->arrComplexEditorCellText[iRow][0], sKind);
}

static int __uiDesignComplexStyleValueTypeFromText(const char* sText)
{
	if ( sText == NULL || sText[0] == '\0' ) return XUI_STYLE_VALUE_STRING;
	if ( __uiDesignComplexTextEqualsNoCase(sText, "none") || strcmp(sText, "0") == 0 ) return XUI_STYLE_VALUE_NONE;
	if ( __uiDesignComplexTextEqualsNoCase(sText, "int") || __uiDesignComplexTextEqualsNoCase(sText, "integer") || strcmp(sText, "1") == 0 ) return XUI_STYLE_VALUE_INT;
	if ( __uiDesignComplexTextEqualsNoCase(sText, "float") || __uiDesignComplexTextEqualsNoCase(sText, "number") || strcmp(sText, "2") == 0 ) return XUI_STYLE_VALUE_FLOAT;
	if ( __uiDesignComplexTextEqualsNoCase(sText, "color") || __uiDesignComplexTextEqualsNoCase(sText, "colour") || strcmp(sText, "3") == 0 ) return XUI_STYLE_VALUE_COLOR;
	if ( __uiDesignComplexTextEqualsNoCase(sText, "bool") || __uiDesignComplexTextEqualsNoCase(sText, "boolean") || strcmp(sText, "4") == 0 ) return XUI_STYLE_VALUE_BOOL;
	if ( __uiDesignComplexTextEqualsNoCase(sText, "string") || __uiDesignComplexTextEqualsNoCase(sText, "text") || strcmp(sText, "5") == 0 ) return XUI_STYLE_VALUE_STRING;
	if ( __uiDesignComplexTextEqualsNoCase(sText, "token") || strcmp(sText, "6") == 0 ) return XUI_STYLE_VALUE_TOKEN;
	return XUI_STYLE_VALUE_STRING;
}

static int __uiDesignComplexStyleValueCellType(const char* sType)
{
	switch ( __uiDesignComplexStyleValueTypeFromText(sType) ) {
	case XUI_STYLE_VALUE_INT:
		return XUI_TABLE_CELL_TYPE_INT;
	case XUI_STYLE_VALUE_FLOAT:
		return XUI_TABLE_CELL_TYPE_FLOAT;
	case XUI_STYLE_VALUE_COLOR:
		return XUI_TABLE_CELL_TYPE_COLOR;
	case XUI_STYLE_VALUE_BOOL:
		return XUI_TABLE_CELL_TYPE_BOOL;
	default:
		return XUI_TABLE_CELL_TYPE_TEXT;
	}
}

static int __uiDesignComplexCellType(ui_design_app_t* pApp, int iRow, int iColumn)
{
	if ( (pApp == NULL) || (iColumn < 0) || (iColumn >= pApp->iComplexEditorColumnCount) ) return XUI_TABLE_CELL_TYPE_TEXT;
	if ( __uiDesignComplexEquals(pApp->sComplexEditorPropertyId, "data.layers") ) {
		if ( iColumn == 0 ) return XUI_TABLE_CELL_TYPE_ENUM;
		if ( __uiDesignComplexRowKindIs(pApp, iRow, "layer") ) {
			if ( iColumn == 2 || iColumn == 3 ) return XUI_TABLE_CELL_TYPE_BOOL;
			if ( iColumn == 4 ) return XUI_TABLE_CELL_TYPE_FLOAT;
			if ( iColumn == 5 ) return XUI_TABLE_CELL_TYPE_COLOR;
			return XUI_TABLE_CELL_TYPE_TEXT;
		}
		if ( __uiDesignComplexRowKindIs(pApp, iRow, "frame") ) {
			if ( iColumn == 2 ) return XUI_TABLE_CELL_TYPE_INT;
			if ( iColumn == 3 ) return XUI_TABLE_CELL_TYPE_ENUM;
			return XUI_TABLE_CELL_TYPE_TEXT;
		}
		if ( __uiDesignComplexRowKindIs(pApp, iRow, "span") ) {
			if ( iColumn == 2 || iColumn == 3 ) return XUI_TABLE_CELL_TYPE_INT;
			if ( iColumn == 4 ) return XUI_TABLE_CELL_TYPE_ENUM;
			if ( iColumn == 6 ) return XUI_TABLE_CELL_TYPE_COLOR;
			return XUI_TABLE_CELL_TYPE_TEXT;
		}
	}
	if ( __uiDesignComplexEquals(pApp->sComplexEditorPropertyId, "style.inlineProperties") ) {
		if ( iColumn == 2 ) return __uiDesignComplexStyleValueCellType(pApp->arrComplexEditorCellText[iRow][1]);
	}
	return pApp->arrComplexEditorColumnType[iColumn];
}

static const xui_combobox_item_t* __uiDesignComplexCellEnumItems(ui_design_app_t* pApp, int iRow, int iColumn, int* pCount, int* pUseValue)
{
	if ( pCount != NULL ) *pCount = 0;
	if ( pUseValue != NULL ) *pUseValue = 0;
	if ( (pApp == NULL) || (iColumn < 0) || (iColumn >= pApp->iComplexEditorColumnCount) ) return NULL;
	if ( __uiDesignComplexEquals(pApp->sComplexEditorPropertyId, "data.layers") ) {
		if ( iColumn == 0 ) {
			if ( pCount != NULL ) *pCount = UI_DESIGN_COUNT_OF(g_arrComplexTimelineKindEnum);
			return g_arrComplexTimelineKindEnum;
		}
		if ( __uiDesignComplexRowKindIs(pApp, iRow, "frame") && iColumn == 3 ) {
			if ( pCount != NULL ) *pCount = UI_DESIGN_COUNT_OF(g_arrComplexTimelineFrameEnum);
			return g_arrComplexTimelineFrameEnum;
		}
		if ( __uiDesignComplexRowKindIs(pApp, iRow, "span") && iColumn == 4 ) {
			if ( pCount != NULL ) *pCount = UI_DESIGN_COUNT_OF(g_arrComplexTimelineSpanEnum);
			return g_arrComplexTimelineSpanEnum;
		}
	}
	if ( pCount != NULL ) *pCount = pApp->arrComplexEditorColumnEnumCount[iColumn];
	if ( pUseValue != NULL ) *pUseValue = pApp->arrComplexEditorColumnEnumUseValue[iColumn];
	return pApp->arrComplexEditorColumnEnumItems[iColumn];
}

static void __uiDesignComplexEditorRefreshGrid(ui_design_app_t* pApp)
{
	xui_widget pTable;

	if ( (pApp == NULL) || (pApp->pComplexEditorGrid == NULL) ) return;
	(void)xuiTableGridSetColumns(pApp->pComplexEditorGrid, pApp->arrComplexEditorColumns, pApp->iComplexEditorColumnCount);
	(void)xuiTableGridSetRows(pApp->pComplexEditorGrid, pApp->arrComplexEditorRows, pApp->iComplexEditorRowCount);
	pTable = xuiTableGridGetTableView(pApp->pComplexEditorGrid);
	if ( pTable != NULL ) {
		(void)xuiTableViewRefreshAdapter(pTable);
	}
	(void)xuiWidgetInvalidate(pApp->pComplexEditorGrid, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static int __uiDesignComplexGridCount(xui_widget pWidget, void* pUser)
{
	ui_design_app_t* pApp;

	(void)pWidget;
	pApp = (ui_design_app_t*)pUser;
	return (pApp != NULL && pApp->bComplexEditorStructured) ? pApp->iComplexEditorRowCount : 0;
}

static int __uiDesignComplexGridCell(xui_widget pWidget, int iRow, int iColumn, xui_table_view_cell_t* pCell, void* pUser)
{
	ui_design_app_t* pApp;

	(void)pWidget;
	pApp = (ui_design_app_t*)pUser;
	if ( (pApp == NULL) || (pCell == NULL) ||
	     (iRow < 0) || (iRow >= pApp->iComplexEditorRowCount) ||
	     (iColumn < 0) || (iColumn >= pApp->iComplexEditorColumnCount) ) {
		return 0;
	}
	memset(pCell, 0, sizeof(*pCell));
	pCell->sText = pApp->arrComplexEditorCellText[iRow][iColumn];
	pCell->iType = __uiDesignComplexCellType(pApp, iRow, iColumn);
	return 1;
}

static void __uiDesignComplexGridSet(xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	ui_design_app_t* pApp;

	(void)pWidget;
	(void)iType;
	pApp = (ui_design_app_t*)pUser;
	if ( (pApp == NULL) ||
	     (iRow < 0) || (iRow >= UI_DESIGN_COMPLEX_EDITOR_MAX_ROWS) ||
	     (iColumn < 0) || (iColumn >= UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS) ) {
		return;
	}
	__uiDesignComplexCopy(pApp->arrComplexEditorCellText[iRow][iColumn], UI_DESIGN_COMPLEX_EDITOR_CELL_CAPACITY, sValue);
	if ( __uiDesignComplexEquals(pApp->sComplexEditorPropertyId, "style.inlineProperties") && iColumn == 1 ) {
		__uiDesignComplexEditorRefreshGrid(pApp);
	}
}

static int __uiDesignComplexParseFloatStrict(const char* sText, float* pValue)
{
	char sWork[96];
	char* pEnd;
	double fValue;

	if ( (sText == NULL) || (pValue == NULL) ) return 0;
	__uiDesignComplexCopy(sWork, sizeof(sWork), sText);
	__uiDesignComplexTrim(sWork);
	if ( sWork[0] == '\0' ) return 0;
	fValue = strtod(sWork, &pEnd);
	if ( pEnd == sWork ) return 0;
	while ( pEnd[0] == ' ' || pEnd[0] == '\t' ) ++pEnd;
	if ( pEnd[0] != '\0' ) return 0;
	*pValue = (float)fValue;
	return 1;
}

static int __uiDesignComplexParseIntStrict(const char* sText, int* pValue)
{
	char sWork[96];
	char* pEnd;
	long iValue;

	if ( (sText == NULL) || (pValue == NULL) ) return 0;
	__uiDesignComplexCopy(sWork, sizeof(sWork), sText);
	__uiDesignComplexTrim(sWork);
	if ( sWork[0] == '\0' ) return 0;
	iValue = strtol(sWork, &pEnd, 10);
	if ( pEnd == sWork ) return 0;
	while ( pEnd[0] == ' ' || pEnd[0] == '\t' ) ++pEnd;
	if ( pEnd[0] != '\0' ) return 0;
	*pValue = (int)iValue;
	return 1;
}

static int __uiDesignComplexIsBoolText(const char* sText)
{
	return __uiDesignComplexTextEqualsNoCase(sText, "true") ||
	       __uiDesignComplexTextEqualsNoCase(sText, "false") ||
	       __uiDesignComplexTextEqualsNoCase(sText, "yes") ||
	       __uiDesignComplexTextEqualsNoCase(sText, "no") ||
	       __uiDesignComplexTextEqualsNoCase(sText, "on") ||
	       __uiDesignComplexTextEqualsNoCase(sText, "off") ||
	       __uiDesignComplexTextEqualsNoCase(sText, "1") ||
	       __uiDesignComplexTextEqualsNoCase(sText, "0");
}

static int __uiDesignComplexIsHexDigit(int c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static int __uiDesignComplexIsColorText(const char* sText)
{
	int iLen;
	int i;

	if ( sText == NULL ) return 0;
	if ( sText[0] != '#' ) return 0;
	iLen = (int)strlen(sText);
	if ( iLen != 7 && iLen != 9 ) return 0;
	for ( i = 1; i < iLen; ++i ) {
		if ( !__uiDesignComplexIsHexDigit((unsigned char)sText[i]) ) return 0;
	}
	return 1;
}

static int __uiDesignComplexEnumIndex(const xui_combobox_item_t* pItems, int iCount, const char* sValue)
{
	int iValue;
	int iParsedValue;
	int i;

	if ( (pItems == NULL) || (iCount <= 0) || (sValue == NULL) ) return -1;
	iParsedValue = __uiDesignComplexParseIntStrict(sValue, &iValue);
	for ( i = 0; i < iCount; ++i ) {
		if ( pItems[i].sText != NULL && __uiDesignComplexTextEqualsNoCase(pItems[i].sText, sValue) ) return i;
		if ( iParsedValue && pItems[i].iValue == iValue ) return i;
	}
	return -1;
}

static int __uiDesignComplexGridValidate(xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	ui_design_app_t* pApp;
	char sWork[UI_DESIGN_COMPLEX_EDITOR_CELL_CAPACITY];
	const xui_combobox_item_t* pEnumItems;
	float fValue;
	int iValue;
	int iEnumCount;
	int iUseValue;
	int iCellType;
	int i;

	(void)pWidget;
	pApp = (ui_design_app_t*)pUser;
	if ( (pApp == NULL) || (iRow < 0) || (iColumn < 0) || (iColumn >= pApp->iComplexEditorColumnCount) ) return 0;
	iCellType = __uiDesignComplexCellType(pApp, iRow, iColumn);
	__uiDesignComplexCopy(sWork, sizeof(sWork), sValue);
	__uiDesignComplexTrim(sWork);
	if ( sWork[0] == '\0' ) {
		if ( !pApp->arrComplexEditorColumnRequired[iColumn] ) return 1;
		for ( i = 0; i < pApp->iComplexEditorColumnCount; ++i ) {
			if ( i != iColumn && pApp->arrComplexEditorCellText[iRow][i][0] != '\0' ) return 0;
		}
		return 1;
	}
	if ( iCellType == XUI_TABLE_CELL_TYPE_INT ) {
		if ( !__uiDesignComplexParseIntStrict(sWork, &iValue) ) return 0;
		if ( pApp->arrComplexEditorColumnRangeSet[iColumn] ) {
			if ( (float)iValue < pApp->arrComplexEditorColumnMin[iColumn] || (float)iValue > pApp->arrComplexEditorColumnMax[iColumn] ) return 0;
		}
		return 1;
	}
	if ( iCellType == XUI_TABLE_CELL_TYPE_FLOAT ) {
		if ( !__uiDesignComplexParseFloatStrict(sWork, &fValue) ) return 0;
		if ( pApp->arrComplexEditorColumnRangeSet[iColumn] ) {
			if ( fValue < pApp->arrComplexEditorColumnMin[iColumn] || fValue > pApp->arrComplexEditorColumnMax[iColumn] ) return 0;
		}
		return 1;
	}
	if ( iCellType == XUI_TABLE_CELL_TYPE_BOOL ) {
		return __uiDesignComplexIsBoolText(sWork);
	}
	if ( iCellType == XUI_TABLE_CELL_TYPE_COLOR ) {
		return __uiDesignComplexIsColorText(sWork);
	}
	if ( iCellType == XUI_TABLE_CELL_TYPE_ENUM ) {
		pEnumItems = __uiDesignComplexCellEnumItems(pApp, iRow, iColumn, &iEnumCount, &iUseValue);
		(void)iUseValue;
		return (iEnumCount <= 0) || (__uiDesignComplexEnumIndex(pEnumItems, iEnumCount, sWork) >= 0);
	}
	(void)iType;
	return 1;
}

static int __uiDesignComplexGridEditorConfig(xui_widget pWidget, int iRow, int iColumn, int iType, xui_table_grid_editor_config_t* pConfig, void* pUser)
{
	ui_design_app_t* pApp;
	const xui_combobox_item_t* pEnumItems;
	const char* sValue;
	int iEnumCount;
	int iSelected;
	int iUseValue;
	int iCellType;

	(void)pWidget;
	(void)iType;
	pApp = (ui_design_app_t*)pUser;
	if ( (pApp == NULL) || (pConfig == NULL) || (iRow < 0) || (iColumn < 0) ||
	     (iRow >= pApp->iComplexEditorRowCount) || (iColumn >= pApp->iComplexEditorColumnCount) ) {
		return 0;
	}
	iCellType = __uiDesignComplexCellType(pApp, iRow, iColumn);
	sValue = pApp->arrComplexEditorCellText[iRow][iColumn];
	pEnumItems = __uiDesignComplexCellEnumItems(pApp, iRow, iColumn, &iEnumCount, &iUseValue);
	if ( iEnumCount > 0 ) {
		pConfig->arrEnumItemData = pEnumItems;
		pConfig->iEnumItemCount = iEnumCount;
		pConfig->bEnumUseValue = iUseValue;
		pConfig->iEnumSelected = -1;
		if ( pConfig->bEnumUseValue ) {
			pConfig->iEnumSelectedValue = (sValue != NULL) ? atoi(sValue) : 0;
		} else {
			iSelected = __uiDesignComplexEnumIndex(pConfig->arrEnumItemData, pConfig->iEnumItemCount, sValue);
			pConfig->iEnumSelected = iSelected;
		}
	}
	if ( pApp->arrComplexEditorColumnRangeSet[iColumn] ) {
		pConfig->fMin = pApp->arrComplexEditorColumnMin[iColumn];
		pConfig->fMax = pApp->arrComplexEditorColumnMax[iColumn];
	}
	if ( pApp->arrComplexEditorColumnPrecision[iColumn] >= 0 ) {
		pConfig->iPrecision = pApp->arrComplexEditorColumnPrecision[iColumn];
	}
	if ( iCellType == XUI_TABLE_CELL_TYPE_INT ) {
		pConfig->fStep = 1.0f;
	}
	if ( iCellType == XUI_TABLE_CELL_TYPE_COLOR ) {
		pConfig->bAlphaEnabled = 1;
	}
	if ( iCellType == XUI_TABLE_CELL_TYPE_FILE ) {
		pConfig->sActionText = "...";
		pConfig->sFileFilter = "All Files (*.*)|*.*";
	}
	return 1;
}

static void __uiDesignComplexEditorSetupColumns(ui_design_app_t* pApp, const ui_design_node_t* pNode, const char* sId, const ui_design_complex_schema_t* pSchema)
{
	char sBuffer[512];
	char* arrFields[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	ui_design_complex_column_config_t tConfig;
	int iCount;
	int i;
	float fWidth;

	if ( (pApp == NULL) || (pSchema == NULL) ) return;
	memset(pApp->arrComplexEditorColumns, 0, sizeof(pApp->arrComplexEditorColumns));
	memset(pApp->arrComplexEditorColumnId, 0, sizeof(pApp->arrComplexEditorColumnId));
	memset(pApp->arrComplexEditorColumnTitle, 0, sizeof(pApp->arrComplexEditorColumnTitle));
	memset(pApp->arrComplexEditorColumnType, 0, sizeof(pApp->arrComplexEditorColumnType));
	memset(pApp->arrComplexEditorColumnRequired, 0, sizeof(pApp->arrComplexEditorColumnRequired));
	memset(pApp->arrComplexEditorColumnRangeSet, 0, sizeof(pApp->arrComplexEditorColumnRangeSet));
	memset(pApp->arrComplexEditorColumnMin, 0, sizeof(pApp->arrComplexEditorColumnMin));
	memset(pApp->arrComplexEditorColumnMax, 0, sizeof(pApp->arrComplexEditorColumnMax));
	memset(pApp->arrComplexEditorColumnPrecision, 0, sizeof(pApp->arrComplexEditorColumnPrecision));
	memset(pApp->arrComplexEditorColumnEnumItems, 0, sizeof(pApp->arrComplexEditorColumnEnumItems));
	memset(pApp->arrComplexEditorColumnEnumCount, 0, sizeof(pApp->arrComplexEditorColumnEnumCount));
	memset(pApp->arrComplexEditorColumnEnumUseValue, 0, sizeof(pApp->arrComplexEditorColumnEnumUseValue));
	__uiDesignComplexCopy(sBuffer, sizeof(sBuffer), pSchema->sColumns);
	iCount = __uiDesignComplexSplit(sBuffer, '|', arrFields, UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS, 1);
	if ( iCount <= 0 ) iCount = 1;
	pApp->iComplexEditorColumnCount = iCount;
	for ( i = 0; i < iCount; ++i ) {
		__uiDesignComplexCopy(pApp->arrComplexEditorColumnTitle[i], UI_DESIGN_COMPLEX_EDITOR_COLUMN_CAPACITY, arrFields[i]);
		snprintf(pApp->arrComplexEditorColumnId[i], UI_DESIGN_COMPLEX_EDITOR_COLUMN_CAPACITY, "c%d", i + 1);
		pApp->arrComplexEditorColumnId[i][UI_DESIGN_COMPLEX_EDITOR_COLUMN_CAPACITY - 1] = '\0';
		__uiDesignComplexColumnConfig(pNode, sId, i, pApp->arrComplexEditorColumnTitle[i], &tConfig);
		pApp->arrComplexEditorColumnType[i] = tConfig.iType;
		pApp->arrComplexEditorColumnRequired[i] = tConfig.bRequired;
		pApp->arrComplexEditorColumnRangeSet[i] = tConfig.bRangeSet;
		pApp->arrComplexEditorColumnMin[i] = tConfig.fMin;
		pApp->arrComplexEditorColumnMax[i] = tConfig.fMax;
		pApp->arrComplexEditorColumnPrecision[i] = tConfig.iPrecision;
		pApp->arrComplexEditorColumnEnumItems[i] = tConfig.pEnumItems;
		pApp->arrComplexEditorColumnEnumCount[i] = tConfig.iEnumCount;
		pApp->arrComplexEditorColumnEnumUseValue[i] = tConfig.bEnumUseValue;
		fWidth = (i == 0) ? 160.0f : 112.0f;
		if ( iCount >= 8 ) fWidth = (i == 0) ? 132.0f : 104.0f;
		if ( pApp->arrComplexEditorColumnType[i] == XUI_TABLE_CELL_TYPE_BOOL ) fWidth = 72.0f;
		if ( pApp->arrComplexEditorColumnType[i] == XUI_TABLE_CELL_TYPE_COLOR ) fWidth = 112.0f;
		if ( pApp->arrComplexEditorColumnType[i] == XUI_TABLE_CELL_TYPE_TEXTAREA ) fWidth = 180.0f;
		pApp->arrComplexEditorColumns[i].iId = i + 1;
		pApp->arrComplexEditorColumns[i].sId = pApp->arrComplexEditorColumnId[i];
		pApp->arrComplexEditorColumns[i].sTitle = pApp->arrComplexEditorColumnTitle[i];
		pApp->arrComplexEditorColumns[i].fWidth = fWidth;
		pApp->arrComplexEditorColumns[i].fMinWidth = 52.0f;
		pApp->arrComplexEditorColumns[i].bVisible = 1;
		pApp->arrComplexEditorColumns[i].bVisibleSet = 1;
		pApp->arrComplexEditorColumns[i].bResizable = 1;
		pApp->arrComplexEditorColumns[i].bResizableSet = 1;
		pApp->arrComplexEditorColumns[i].iAlign = XUI_TEXT_ALIGN_LEFT;
		pApp->arrComplexEditorColumns[i].iType = pApp->arrComplexEditorColumnType[i];
	}
}

static void __uiDesignComplexEditorAddParsedRow(ui_design_app_t* pApp, char** arrFields, int iFieldCount)
{
	int i;
	int iRow;

	if ( (pApp == NULL) || (arrFields == NULL) || (pApp->iComplexEditorRowCount >= UI_DESIGN_COMPLEX_EDITOR_MAX_ROWS) ) return;
	iRow = pApp->iComplexEditorRowCount++;
	pApp->arrComplexEditorRows[iRow].fHeight = 24.0f;
	for ( i = 0; i < pApp->iComplexEditorColumnCount; ++i ) {
		__uiDesignComplexCopy(pApp->arrComplexEditorCellText[iRow][i], UI_DESIGN_COMPLEX_EDITOR_CELL_CAPACITY, (i < iFieldCount) ? arrFields[i] : "");
	}
}

static void __uiDesignComplexEditorAddTreeRow(ui_design_app_t* pApp, int iId, int iParent, int iDepth, char** arrFields, int iFieldCount)
{
	char sId[32];
	char sParent[32];
	char sDepth[32];
	char* arrRow[9];

	if ( (pApp == NULL) || (arrFields == NULL) || (iFieldCount <= 0) ) return;
	snprintf(sId, sizeof(sId), "%d", iId);
	snprintf(sParent, sizeof(sParent), "%d", iParent);
	snprintf(sDepth, sizeof(sDepth), "%d", iDepth);
	arrRow[0] = sId;
	arrRow[1] = sParent;
	arrRow[2] = sDepth;
	arrRow[3] = arrFields[0];
	arrRow[4] = (iFieldCount > 1 && arrFields[1][0] != '\0') ? arrFields[1] : "true";
	arrRow[5] = (iFieldCount > 2 && arrFields[2][0] != '\0') ? arrFields[2] : "true";
	if ( iFieldCount > 5 ) {
		arrRow[6] = (arrFields[3][0] != '\0') ? arrFields[3] : "true";
		arrRow[7] = (arrFields[4][0] != '\0') ? arrFields[4] : "false";
		arrRow[8] = (arrFields[5][0] != '\0') ? arrFields[5] : "false";
	} else {
		arrRow[6] = "true";
		arrRow[7] = (iFieldCount > 3 && arrFields[3][0] != '\0') ? "true" : "false";
		arrRow[8] = (iFieldCount > 3 && arrFields[3][0] != '\0') ? arrFields[3] : "false";
	}
	__uiDesignComplexEditorAddParsedRow(pApp, arrRow, UI_DESIGN_COUNT_OF(arrRow));
}

static void __uiDesignComplexEditorLoadTreeRows(ui_design_app_t* pApp, const char* sValue)
{
	const char* sCursor;
	char sLine[1024];
	char sWork[1024];
	char* arrFields[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	int arrParentByDepth[32];
	int iFieldCount;
	int iDepth;
	int iSpaces;
	int iTabs;
	int i;
	int iId;
	int iParent;
	int iTemp;

	if ( pApp == NULL ) return;
	memset(arrParentByDepth, 0, sizeof(arrParentByDepth));
	sCursor = (sValue != NULL) ? sValue : "";
	while ( __uiDesignComplexNextLine(&sCursor, sLine, sizeof(sLine)) && pApp->iComplexEditorRowCount < UI_DESIGN_COMPLEX_EDITOR_MAX_ROWS ) {
		iSpaces = 0;
		iTabs = 0;
		for ( i = 0; sLine[i] == ' ' || sLine[i] == '\t'; ++i ) {
			if ( sLine[i] == '\t' ) ++iTabs;
			else ++iSpaces;
		}
		iDepth = iTabs + (iSpaces / 2);
		if ( iDepth < 0 ) iDepth = 0;
		if ( iDepth >= (int)UI_DESIGN_COUNT_OF(arrParentByDepth) ) iDepth = (int)UI_DESIGN_COUNT_OF(arrParentByDepth) - 1;
		__uiDesignComplexCopy(sWork, sizeof(sWork), sLine);
		__uiDesignComplexTrim(sWork);
		if ( sWork[0] == '\0' ) continue;
		iFieldCount = __uiDesignComplexSplit(sWork, strchr(sWork, '|') != NULL ? '|' : 0, arrFields, UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS, strchr(sWork, '|') != NULL);
		if ( iFieldCount >= 4 &&
		     __uiDesignComplexParseIntStrict(arrFields[0], &iTemp) &&
		     __uiDesignComplexParseIntStrict(arrFields[1], &iParent) &&
		     __uiDesignComplexParseIntStrict(arrFields[2], &iDepth) ) {
			if ( iFieldCount >= 9 ) {
				__uiDesignComplexEditorAddParsedRow(pApp, arrFields, iFieldCount);
			} else {
				char* arrRow[9];
				arrRow[0] = arrFields[0];
				arrRow[1] = arrFields[1];
				arrRow[2] = arrFields[2];
				arrRow[3] = arrFields[3];
				arrRow[4] = (iFieldCount > 4 && arrFields[4][0] != '\0') ? arrFields[4] : "true";
				arrRow[5] = (iFieldCount > 5 && arrFields[5][0] != '\0') ? arrFields[5] : "true";
				arrRow[6] = "true";
				arrRow[7] = (iFieldCount > 6 && arrFields[6][0] != '\0') ? "true" : "false";
				arrRow[8] = (iFieldCount > 6 && arrFields[6][0] != '\0') ? arrFields[6] : "false";
				__uiDesignComplexEditorAddParsedRow(pApp, arrRow, UI_DESIGN_COUNT_OF(arrRow));
			}
			iId = iTemp;
			if ( iDepth >= 0 && iDepth < (int)UI_DESIGN_COUNT_OF(arrParentByDepth) ) arrParentByDepth[iDepth] = iId;
			continue;
		}
		iId = pApp->iComplexEditorRowCount + 1;
		if ( iFieldCount > 6 && __uiDesignComplexParseIntStrict(arrFields[6], &iTemp) ) iId = iTemp;
		else if ( iFieldCount > 4 && __uiDesignComplexParseIntStrict(arrFields[4], &iTemp) ) iId = iTemp;
		iParent = (iDepth > 0) ? arrParentByDepth[iDepth - 1] : -1;
		__uiDesignComplexEditorAddTreeRow(pApp, iId, iParent, iDepth, arrFields, iFieldCount);
		arrParentByDepth[iDepth] = iId;
	}
}

static int __uiDesignComplexCascaderFindRow(ui_design_app_t* pApp, int iParent, const char* sText)
{
	int iRow;
	int iValue;

	if ( (pApp == NULL) || (sText == NULL) ) return -1;
	for ( iRow = 0; iRow < pApp->iComplexEditorRowCount; ++iRow ) {
		if ( !__uiDesignComplexTextEqualsNoCase(pApp->arrComplexEditorCellText[iRow][0], sText) ) continue;
		if ( !__uiDesignComplexParseIntStrict(pApp->arrComplexEditorCellText[iRow][2], &iValue) ) iValue = -1;
		if ( iValue == iParent ) return iRow;
	}
	return -1;
}

static void __uiDesignComplexEditorAddCascaderRow(ui_design_app_t* pApp, const char* sText, int iParent, int bLeaf)
{
	char sValue[32];
	char sParent[32];
	char* arrRow[5];
	int iRow;

	if ( (pApp == NULL) || (sText == NULL) || (sText[0] == '\0') || (pApp->iComplexEditorRowCount >= UI_DESIGN_COMPLEX_EDITOR_MAX_ROWS) ) return;
	iRow = pApp->iComplexEditorRowCount;
	snprintf(sValue, sizeof(sValue), "%d", iRow + 1);
	snprintf(sParent, sizeof(sParent), "%d", iParent);
	arrRow[0] = (char*)sText;
	arrRow[1] = sValue;
	arrRow[2] = sParent;
	arrRow[3] = bLeaf ? "true" : "false";
	arrRow[4] = "false";
	__uiDesignComplexEditorAddParsedRow(pApp, arrRow, UI_DESIGN_COUNT_OF(arrRow));
}

static void __uiDesignComplexEditorLoadCascaderPath(ui_design_app_t* pApp, const char* sPath)
{
	char sWork[1024];
	char* sCursor;
	char* sPart;
	int iParent;
	int iFound;
	int bLast;

	if ( (pApp == NULL) || (sPath == NULL) ) return;
	__uiDesignComplexCopy(sWork, sizeof(sWork), sPath);
	sCursor = sWork;
	iParent = -1;
	while ( sCursor[0] != '\0' ) {
		sPart = sCursor;
		while ( sCursor[0] != '\0' && sCursor[0] != '/' && sCursor[0] != '\\' ) ++sCursor;
		bLast = (sCursor[0] == '\0');
		if ( !bLast ) {
			sCursor[0] = '\0';
			++sCursor;
		}
		__uiDesignComplexTrim(sPart);
		if ( sPart[0] == '\0' ) continue;
		iFound = __uiDesignComplexCascaderFindRow(pApp, iParent, sPart);
		if ( iFound < 0 ) {
			iFound = pApp->iComplexEditorRowCount;
			__uiDesignComplexEditorAddCascaderRow(pApp, sPart, iParent, bLast);
		} else if ( !bLast ) {
			__uiDesignComplexCopy(pApp->arrComplexEditorCellText[iFound][3], UI_DESIGN_COMPLEX_EDITOR_CELL_CAPACITY, "false");
		}
		iParent = iFound;
	}
}

static void __uiDesignComplexEditorLoadCascaderRows(ui_design_app_t* pApp, const char* sValue)
{
	const char* sCursor;
	char sLine[1024];
	char sWork[1024];
	char* arrFields[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	int iFieldCount;

	if ( pApp == NULL ) return;
	sCursor = (sValue != NULL) ? sValue : "";
	while ( __uiDesignComplexNextLine(&sCursor, sLine, sizeof(sLine)) && pApp->iComplexEditorRowCount < UI_DESIGN_COMPLEX_EDITOR_MAX_ROWS ) {
		__uiDesignComplexCopy(sWork, sizeof(sWork), sLine);
		__uiDesignComplexTrim(sWork);
		if ( sWork[0] == '\0' ) continue;
		if ( strchr(sWork, '|') != NULL ) {
			iFieldCount = __uiDesignComplexSplit(sWork, '|', arrFields, UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS, 1);
			__uiDesignComplexEditorAddParsedRow(pApp, arrFields, iFieldCount);
		} else {
			__uiDesignComplexEditorLoadCascaderPath(pApp, sWork);
		}
	}
}

static void __uiDesignComplexEditorLoadStructured(ui_design_app_t* pApp, const ui_design_node_t* pNode, const char* sId, const ui_design_complex_schema_t* pSchema, const char* sValue)
{
	const char* sCursor;
	char sLine[1024];
	char sWork[1024];
	char* arrFields[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS];
	int iFieldCount;
	int bSplit;
	int i;

	if ( (pApp == NULL) || (pSchema == NULL) ) return;
	memset(pApp->arrComplexEditorRows, 0, sizeof(pApp->arrComplexEditorRows));
	memset(pApp->arrComplexEditorCellText, 0, sizeof(pApp->arrComplexEditorCellText));
	pApp->iComplexEditorRowCount = 0;
	pApp->cComplexEditorDelimiter = pSchema->cDelimiter;
	__uiDesignComplexEditorSetupColumns(pApp, pNode, sId, pSchema);
	if ( pSchema->bTreeIndentRows ) {
		__uiDesignComplexEditorLoadTreeRows(pApp, sValue);
		if ( pApp->iComplexEditorRowCount == 0 ) {
			pApp->iComplexEditorRowCount = 1;
			pApp->arrComplexEditorRows[0].fHeight = 24.0f;
		}
		__uiDesignComplexEditorRefreshGrid(pApp);
		return;
	}
	if ( pSchema->bCascaderPathRows ) {
		__uiDesignComplexEditorLoadCascaderRows(pApp, sValue);
		if ( pApp->iComplexEditorRowCount == 0 ) {
			pApp->iComplexEditorRowCount = 1;
			pApp->arrComplexEditorRows[0].fHeight = 24.0f;
		}
		__uiDesignComplexEditorRefreshGrid(pApp);
		return;
	}
	sCursor = (sValue != NULL) ? sValue : "";
	while ( __uiDesignComplexNextLine(&sCursor, sLine, sizeof(sLine)) && pApp->iComplexEditorRowCount < UI_DESIGN_COMPLEX_EDITOR_MAX_ROWS ) {
		if ( sLine[0] == '\0' ) continue;
		if ( pSchema->bExplodeCommaRows && strchr(sLine, '|') == NULL && strchr(sLine, ',') != NULL ) {
			__uiDesignComplexCopy(sWork, sizeof(sWork), sLine);
			iFieldCount = __uiDesignComplexSplit(sWork, ',', arrFields, UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS, 1);
			for ( i = 0; i < iFieldCount && pApp->iComplexEditorRowCount < UI_DESIGN_COMPLEX_EDITOR_MAX_ROWS; ++i ) {
				char* arrOne[1];
				arrOne[0] = arrFields[i];
				__uiDesignComplexEditorAddParsedRow(pApp, arrOne, 1);
			}
			continue;
		}
		__uiDesignComplexCopy(sWork, sizeof(sWork), sLine);
		bSplit = (pSchema->cDelimiter != 0 && strchr(sWork, pSchema->cDelimiter) != NULL);
		iFieldCount = __uiDesignComplexSplit(sWork, bSplit ? pSchema->cDelimiter : 0, arrFields, UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS, bSplit);
		__uiDesignComplexEditorAddParsedRow(pApp, arrFields, iFieldCount);
	}
	if ( pApp->iComplexEditorRowCount == 0 ) {
		pApp->iComplexEditorRowCount = 1;
		pApp->arrComplexEditorRows[0].fHeight = 24.0f;
	}
	__uiDesignComplexEditorRefreshGrid(pApp);
}

static void __uiDesignComplexEditorSerializeStructured(ui_design_app_t* pApp, char* sBuffer, int iCapacity)
{
	int iLength;
	int iRow;
	int iColumn;
	int iLast;
	int bNeedNewline;

	if ( (sBuffer == NULL) || (iCapacity <= 0) ) return;
	sBuffer[0] = '\0';
	if ( pApp == NULL ) return;
	iLength = 0;
	bNeedNewline = 0;
	for ( iRow = 0; iRow < pApp->iComplexEditorRowCount; ++iRow ) {
		iLast = -1;
		for ( iColumn = 0; iColumn < pApp->iComplexEditorColumnCount; ++iColumn ) {
			if ( pApp->arrComplexEditorCellText[iRow][iColumn][0] != '\0' ) iLast = iColumn;
		}
		if ( iLast < 0 ) continue;
		if ( bNeedNewline ) (void)__uiDesignComplexAppend(sBuffer, iCapacity, &iLength, "\n");
		for ( iColumn = 0; iColumn <= iLast; ++iColumn ) {
			if ( iColumn > 0 ) {
				char sDelim[2];
				sDelim[0] = (pApp->cComplexEditorDelimiter != 0) ? pApp->cComplexEditorDelimiter : '|';
				sDelim[1] = '\0';
				(void)__uiDesignComplexAppend(sBuffer, iCapacity, &iLength, sDelim);
			}
			(void)__uiDesignComplexAppend(sBuffer, iCapacity, &iLength, pApp->arrComplexEditorCellText[iRow][iColumn]);
		}
		bNeedNewline = 1;
	}
}

static void __uiDesignComplexEditorSetStructuredVisible(ui_design_app_t* pApp)
{
	int bStructured;

	if ( pApp == NULL ) return;
	bStructured = pApp->bComplexEditorStructured;
	if ( pApp->pComplexEditorText != NULL ) (void)xuiWidgetSetVisible(pApp->pComplexEditorText, !bStructured);
	if ( pApp->pComplexEditorGrid != NULL ) (void)xuiWidgetSetVisible(pApp->pComplexEditorGrid, bStructured);
	if ( pApp->pComplexEditorAdd != NULL ) (void)xuiWidgetSetVisible(pApp->pComplexEditorAdd, bStructured);
	if ( pApp->pComplexEditorDelete != NULL ) (void)xuiWidgetSetVisible(pApp->pComplexEditorDelete, bStructured);
	if ( pApp->pComplexEditorUp != NULL ) (void)xuiWidgetSetVisible(pApp->pComplexEditorUp, bStructured);
	if ( pApp->pComplexEditorDown != NULL ) (void)xuiWidgetSetVisible(pApp->pComplexEditorDown, bStructured);
}

static void __uiDesignComplexEditorAddRow(xui_widget pWidget, void* pUser)
{
	ui_design_app_t* pApp;
	xui_widget pTable;
	int iRow;
	int i;

	(void)pWidget;
	pApp = (ui_design_app_t*)pUser;
	if ( (pApp == NULL) || !pApp->bComplexEditorStructured || pApp->iComplexEditorRowCount >= UI_DESIGN_COMPLEX_EDITOR_MAX_ROWS ) return;
	(void)xuiTableGridEndEdit(pApp->pComplexEditorGrid, 1);
	iRow = pApp->iComplexEditorRowCount++;
	pApp->arrComplexEditorRows[iRow].fHeight = 24.0f;
	for ( i = 0; i < pApp->iComplexEditorColumnCount; ++i ) {
		pApp->arrComplexEditorCellText[iRow][i][0] = '\0';
	}
	__uiDesignComplexEditorRefreshGrid(pApp);
	pTable = xuiTableGridGetTableView(pApp->pComplexEditorGrid);
	if ( pTable != NULL ) {
		(void)xuiTableViewSetSelectedRow(pTable, iRow);
		(void)xuiTableViewEnsureCellVisible(pTable, iRow, 0);
	}
}

static void __uiDesignComplexEditorDeleteRow(xui_widget pWidget, void* pUser)
{
	ui_design_app_t* pApp;
	xui_widget pTable;
	int iRow;
	int i;

	(void)pWidget;
	pApp = (ui_design_app_t*)pUser;
	if ( (pApp == NULL) || !pApp->bComplexEditorStructured ) return;
	(void)xuiTableGridEndEdit(pApp->pComplexEditorGrid, 1);
	iRow = __uiDesignComplexEditorSelectedRow(pApp);
	if ( iRow < 0 ) iRow = pApp->iComplexEditorRowCount - 1;
	if ( iRow < 0 ) return;
	for ( i = iRow; i < pApp->iComplexEditorRowCount - 1; ++i ) {
		pApp->arrComplexEditorRows[i] = pApp->arrComplexEditorRows[i + 1];
		memcpy(pApp->arrComplexEditorCellText[i], pApp->arrComplexEditorCellText[i + 1], sizeof(pApp->arrComplexEditorCellText[i]));
	}
	if ( pApp->iComplexEditorRowCount > 0 ) --pApp->iComplexEditorRowCount;
	if ( pApp->iComplexEditorRowCount == 0 ) {
		pApp->iComplexEditorRowCount = 1;
		pApp->arrComplexEditorRows[0].fHeight = 24.0f;
		memset(pApp->arrComplexEditorCellText[0], 0, sizeof(pApp->arrComplexEditorCellText[0]));
	}
	__uiDesignComplexEditorRefreshGrid(pApp);
	pTable = xuiTableGridGetTableView(pApp->pComplexEditorGrid);
	if ( pTable != NULL ) {
		if ( iRow >= pApp->iComplexEditorRowCount ) iRow = pApp->iComplexEditorRowCount - 1;
		(void)xuiTableViewSetSelectedRow(pTable, iRow);
	}
}

static void __uiDesignComplexEditorMoveRow(ui_design_app_t* pApp, int iDelta)
{
	char arrCells[UI_DESIGN_COMPLEX_EDITOR_MAX_COLUMNS][UI_DESIGN_COMPLEX_EDITOR_CELL_CAPACITY];
	xui_table_view_row_t tRow;
	xui_widget pTable;
	int iRow;
	int iTarget;

	if ( (pApp == NULL) || !pApp->bComplexEditorStructured ) return;
	(void)xuiTableGridEndEdit(pApp->pComplexEditorGrid, 1);
	iRow = __uiDesignComplexEditorSelectedRow(pApp);
	iTarget = iRow + iDelta;
	if ( iRow < 0 || iTarget < 0 || iTarget >= pApp->iComplexEditorRowCount ) return;
	tRow = pApp->arrComplexEditorRows[iRow];
	memcpy(arrCells, pApp->arrComplexEditorCellText[iRow], sizeof(arrCells));
	pApp->arrComplexEditorRows[iRow] = pApp->arrComplexEditorRows[iTarget];
	memcpy(pApp->arrComplexEditorCellText[iRow], pApp->arrComplexEditorCellText[iTarget], sizeof(arrCells));
	pApp->arrComplexEditorRows[iTarget] = tRow;
	memcpy(pApp->arrComplexEditorCellText[iTarget], arrCells, sizeof(arrCells));
	__uiDesignComplexEditorRefreshGrid(pApp);
	pTable = xuiTableGridGetTableView(pApp->pComplexEditorGrid);
	if ( pTable != NULL ) {
		(void)xuiTableViewSetSelectedRow(pTable, iTarget);
		(void)xuiTableViewEnsureCellVisible(pTable, iTarget, 0);
	}
}

static void __uiDesignComplexEditorMoveUp(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	__uiDesignComplexEditorMoveRow((ui_design_app_t*)pUser, -1);
}

static void __uiDesignComplexEditorMoveDown(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	__uiDesignComplexEditorMoveRow((ui_design_app_t*)pUser, 1);
}

static void __uiDesignComplexEditorClose(ui_design_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->pComplexEditorWindow != NULL ) {
		(void)xuiWindowSetOpen(pApp->pComplexEditorWindow, 0);
	}
	pApp->iComplexEditorNodeId = 0;
	pApp->sComplexEditorPropertyId[0] = '\0';
	pApp->bComplexEditorStructured = 0;
}

static void __uiDesignComplexEditorWindowClose(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	__uiDesignComplexEditorClose((ui_design_app_t*)pUser);
}

static void __uiDesignComplexEditorCancel(xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	__uiDesignComplexEditorClose((ui_design_app_t*)pUser);
}

static void __uiDesignComplexEditorOk(xui_widget pWidget, void* pUser)
{
	ui_design_app_t* pApp;
	const char* sText;
	char sStructured[UI_DESIGN_PROPERTY_VALUE_CAPACITY];

	(void)pWidget;
	pApp = (ui_design_app_t*)pUser;
	if ( (pApp == NULL) || (pApp->iComplexEditorNodeId <= 0) || (pApp->sComplexEditorPropertyId[0] == '\0') ) return;
	if ( pApp->bComplexEditorStructured ) {
		(void)xuiTableGridEndEdit(pApp->pComplexEditorGrid, 1);
		__uiDesignComplexEditorSerializeStructured(pApp, sStructured, sizeof(sStructured));
		sText = sStructured;
	} else {
		sText = xuiTextEditGetText(pApp->pComplexEditorText);
	}
	if ( sText == NULL ) sText = "";
	(void)uiDesignAppCommandSetNodeProperty(pApp, pApp->iComplexEditorNodeId, pApp->sComplexEditorPropertyId, sText);
	__uiDesignComplexEditorClose(pApp);
}

static void __uiDesignComplexEditorLayout(ui_design_app_t* pApp)
{
	xui_rect_t tRoot;
	xui_rect_t tWindow;

	if ( (pApp == NULL) || (pApp->pComplexEditorWindow == NULL) ) return;
	tRoot = (pApp->pRoot != NULL) ? xuiWidgetGetRect(pApp->pRoot) : (xui_rect_t){0.0f, 0.0f, UI_DESIGN_TARGET_W, UI_DESIGN_TARGET_H};
	if ( tRoot.fW <= 0.0f ) tRoot.fW = UI_DESIGN_TARGET_W;
	if ( tRoot.fH <= 0.0f ) tRoot.fH = UI_DESIGN_TARGET_H;
	tWindow.fW = UI_DESIGN_COMPLEX_EDITOR_W;
	tWindow.fH = UI_DESIGN_COMPLEX_EDITOR_H;
	tWindow.fX = tRoot.fX + (tRoot.fW - tWindow.fW) * 0.5f;
	tWindow.fY = tRoot.fY + (tRoot.fH - tWindow.fH) * 0.5f;
	if ( tWindow.fX < tRoot.fX + 12.0f ) tWindow.fX = tRoot.fX + 12.0f;
	if ( tWindow.fY < tRoot.fY + 12.0f ) tWindow.fY = tRoot.fY + 12.0f;
	(void)xuiWidgetSetRect(pApp->pComplexEditorWindow, tWindow);
	if ( pApp->pComplexEditorText != NULL ) {
		(void)xuiWidgetSetRect(pApp->pComplexEditorText, (xui_rect_t){12.0f, 12.0f, 736.0f, 416.0f});
	}
	if ( pApp->pComplexEditorAdd != NULL ) {
		(void)xuiWidgetSetRect(pApp->pComplexEditorAdd, (xui_rect_t){12.0f, 12.0f, 76.0f, 28.0f});
	}
	if ( pApp->pComplexEditorDelete != NULL ) {
		(void)xuiWidgetSetRect(pApp->pComplexEditorDelete, (xui_rect_t){96.0f, 12.0f, 76.0f, 28.0f});
	}
	if ( pApp->pComplexEditorUp != NULL ) {
		(void)xuiWidgetSetRect(pApp->pComplexEditorUp, (xui_rect_t){180.0f, 12.0f, 76.0f, 28.0f});
	}
	if ( pApp->pComplexEditorDown != NULL ) {
		(void)xuiWidgetSetRect(pApp->pComplexEditorDown, (xui_rect_t){264.0f, 12.0f, 76.0f, 28.0f});
	}
	if ( pApp->pComplexEditorGrid != NULL ) {
		(void)xuiWidgetSetRect(pApp->pComplexEditorGrid, (xui_rect_t){12.0f, 48.0f, 736.0f, 380.0f});
	}
	if ( pApp->pComplexEditorOk != NULL ) {
		(void)xuiWidgetSetRect(pApp->pComplexEditorOk, (xui_rect_t){556.0f, 444.0f, 84.0f, 30.0f});
	}
	if ( pApp->pComplexEditorCancel != NULL ) {
		(void)xuiWidgetSetRect(pApp->pComplexEditorCancel, (xui_rect_t){652.0f, 444.0f, 96.0f, 30.0f});
	}
	__uiDesignComplexEditorSetStructuredVisible(pApp);
}

static int __uiDesignComplexEditorEnsure(ui_design_app_t* pApp)
{
	xui_window_desc_t tWindow;
	xui_text_edit_desc_t tText;
	xui_table_grid_desc_t tGrid;
	xui_button_desc_t tButton;
	xui_widget pClient;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->pComplexEditorWindow != NULL ) return XUI_OK;
	memset(&tWindow, 0, sizeof(tWindow));
	tWindow.iSize = sizeof(tWindow);
	tWindow.sTitle = "Edit Property";
	tWindow.pFont = pApp->pFont;
	tWindow.bClosed = 1;
	tWindow.bTopMost = 1;
	tWindow.bHideCollapse = 1;
	tWindow.bHideMaximize = 1;
	tWindow.bNotResizable = 1;
	tWindow.fTitleBarHeight = 28.0f;
	tWindow.fBorderWidth = 1.0f;
	tWindow.fButtonSize = 18.0f;
	tWindow.fMinWidth = UI_DESIGN_COMPLEX_EDITOR_W;
	tWindow.fMinHeight = UI_DESIGN_COMPLEX_EDITOR_H;
	iRet = xuiWindowCreate(pApp->pContext, &pApp->pComplexEditorWindow, &tWindow);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWindowSetClose(pApp->pComplexEditorWindow, __uiDesignComplexEditorWindowClose, pApp);
	pClient = xuiWindowGetClientWidget(pApp->pComplexEditorWindow);
	if ( pClient != NULL ) {
		(void)xuiWidgetSetLayoutType(pClient, XUI_LAYOUT_MANUAL);
		(void)xuiWidgetSetFlowMode(pClient, XUI_FLOW_ABSOLUTE);
		(void)xuiWidgetSetPadding(pClient, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
		(void)xuiWidgetSetGap(pClient, 0.0f);
	}
	memset(&tText, 0, sizeof(tText));
	tText.iSize = sizeof(tText);
	tText.pFont = pApp->pFont;
	tText.iMaxLength = UI_DESIGN_PROPERTY_VALUE_CAPACITY - 1;
	tText.bWordWrap = 0;
	tText.bLineNumbers = 1;
	tText.fLineNumberWidth = 38.0f;
	tText.fBorderWidth = 1.0f;
	tText.iBackgroundColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	tText.iTextColor = XUI_COLOR_RGBA(34, 47, 66, 255);
	tText.iBorderColor = XUI_COLOR_RGBA(166, 188, 214, 255);
	tText.iFocusBorderColor = XUI_COLOR_RGBA(49, 126, 214, 255);
	tText.iLineNumberColor = XUI_COLOR_RGBA(112, 126, 148, 255);
	tText.iLineNumberBackgroundColor = XUI_COLOR_RGBA(243, 247, 252, 255);
	tText.iLineNumberBorderColor = XUI_COLOR_RGBA(210, 222, 236, 255);
	iRet = xuiTextEditCreate(pApp->pContext, &pApp->pComplexEditorText, &tText);
	memset(&tGrid, 0, sizeof(tGrid));
	tGrid.iSize = sizeof(tGrid);
	tGrid.onCount = __uiDesignComplexGridCount;
	tGrid.onCell = __uiDesignComplexGridCell;
	tGrid.onSet = __uiDesignComplexGridSet;
	tGrid.pAdapterUser = pApp;
	tGrid.onValidate = __uiDesignComplexGridValidate;
	tGrid.pValidateUser = pApp;
	tGrid.onEditorConfig = __uiDesignComplexGridEditorConfig;
	tGrid.pEditorConfigUser = pApp;
	tGrid.pFont = pApp->pFont;
	tGrid.fDefaultColumnWidth = 112.0f;
	tGrid.fDefaultRowHeight = 24.0f;
	tGrid.fHeaderHeight = 26.0f;
	tGrid.iSelectionMode = XUI_TABLE_VIEW_SELECTION_ROW;
	tGrid.iEditMode = XUI_TABLE_GRID_EDIT_QUICK;
	tGrid.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	tGrid.tColors.iBackgroundColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	tGrid.tColors.iHeaderColor = XUI_COLOR_RGBA(237, 244, 252, 255);
	tGrid.tColors.iHeaderTextColor = XUI_COLOR_RGBA(43, 60, 80, 255);
	tGrid.tColors.iRowColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	tGrid.tColors.iAltRowColor = XUI_COLOR_RGBA(248, 251, 255, 255);
	tGrid.tColors.iHoverColor = XUI_COLOR_RGBA(226, 238, 252, 255);
	tGrid.tColors.iSelectedColor = XUI_COLOR_RGBA(204, 225, 249, 255);
	tGrid.tColors.iGridColor = XUI_COLOR_RGBA(210, 222, 236, 255);
	tGrid.tColors.iTextColor = XUI_COLOR_RGBA(34, 47, 66, 255);
	tGrid.tColors.iFocusRingColor = XUI_COLOR_RGBA(49, 126, 214, 255);
	if ( iRet == XUI_OK ) iRet = xuiTableGridCreate(pApp->pContext, &pApp->pComplexEditorGrid, &tGrid);
	memset(&tButton, 0, sizeof(tButton));
	tButton.iSize = sizeof(tButton);
	tButton.pFont = pApp->pFont;
	tButton.sText = "Add";
	tButton.fBorderWidth = 1.0f;
	if ( iRet == XUI_OK ) iRet = xuiButtonCreate(pApp->pContext, &pApp->pComplexEditorAdd, &tButton);
	tButton.sText = "Delete";
	if ( iRet == XUI_OK ) iRet = xuiButtonCreate(pApp->pContext, &pApp->pComplexEditorDelete, &tButton);
	tButton.sText = "Up";
	if ( iRet == XUI_OK ) iRet = xuiButtonCreate(pApp->pContext, &pApp->pComplexEditorUp, &tButton);
	tButton.sText = "Down";
	if ( iRet == XUI_OK ) iRet = xuiButtonCreate(pApp->pContext, &pApp->pComplexEditorDown, &tButton);
	tButton.sText = "OK";
	if ( iRet == XUI_OK ) iRet = xuiButtonCreate(pApp->pContext, &pApp->pComplexEditorOk, &tButton);
	tButton.sText = "Cancel";
	if ( iRet == XUI_OK ) iRet = xuiButtonCreate(pApp->pContext, &pApp->pComplexEditorCancel, &tButton);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiButtonSetClick(pApp->pComplexEditorAdd, __uiDesignComplexEditorAddRow, pApp);
	(void)xuiButtonSetClick(pApp->pComplexEditorDelete, __uiDesignComplexEditorDeleteRow, pApp);
	(void)xuiButtonSetClick(pApp->pComplexEditorUp, __uiDesignComplexEditorMoveUp, pApp);
	(void)xuiButtonSetClick(pApp->pComplexEditorDown, __uiDesignComplexEditorMoveDown, pApp);
	(void)xuiButtonSetClick(pApp->pComplexEditorOk, __uiDesignComplexEditorOk, pApp);
	(void)xuiButtonSetClick(pApp->pComplexEditorCancel, __uiDesignComplexEditorCancel, pApp);
	iRet = xuiWindowAddChild(pApp->pComplexEditorWindow, pApp->pComplexEditorText);
	if ( iRet == XUI_OK ) iRet = xuiWindowAddChild(pApp->pComplexEditorWindow, pApp->pComplexEditorGrid);
	if ( iRet == XUI_OK ) iRet = xuiWindowAddChild(pApp->pComplexEditorWindow, pApp->pComplexEditorAdd);
	if ( iRet == XUI_OK ) iRet = xuiWindowAddChild(pApp->pComplexEditorWindow, pApp->pComplexEditorDelete);
	if ( iRet == XUI_OK ) iRet = xuiWindowAddChild(pApp->pComplexEditorWindow, pApp->pComplexEditorUp);
	if ( iRet == XUI_OK ) iRet = xuiWindowAddChild(pApp->pComplexEditorWindow, pApp->pComplexEditorDown);
	if ( iRet == XUI_OK ) iRet = xuiWindowAddChild(pApp->pComplexEditorWindow, pApp->pComplexEditorOk);
	if ( iRet == XUI_OK ) iRet = xuiWindowAddChild(pApp->pComplexEditorWindow, pApp->pComplexEditorCancel);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pApp->pRoot, pApp->pComplexEditorWindow);
	if ( iRet != XUI_OK ) return iRet;
	__uiDesignComplexEditorLayout(pApp);
	return XUI_OK;
}

static int __uiDesignComplexEditorOpen(ui_design_app_t* pApp, const char* sId)
{
	ui_design_node_t* pNode;
	const ui_design_control_desc_t* pDesc;
	const ui_design_property_def_t* pDef;
	ui_design_complex_schema_t tSchema;
	const char* sValue;
	char sTitle[128];
	int iRet;

	if ( (pApp == NULL) || (sId == NULL) || (sId[0] == '\0') ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode = uiDesignModelGetSelected(&pApp->tModel);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pDesc = uiDesignRegistryFind(pNode->iType);
	pDef = uiDesignRegistryFindProperty(pDesc, sId);
	if ( (pDef != NULL) && (pDef->iEditorKind != UI_DESIGN_PROPERTY_COMPLEX) ) return XUI_OK;
	iRet = __uiDesignComplexEditorEnsure(pApp);
	if ( iRet != XUI_OK ) return iRet;
	pApp->iComplexEditorNodeId = pNode->iId;
	snprintf(pApp->sComplexEditorPropertyId, sizeof(pApp->sComplexEditorPropertyId), "%s", sId);
	pApp->sComplexEditorPropertyId[sizeof(pApp->sComplexEditorPropertyId) - 1] = '\0';
	snprintf(sTitle, sizeof(sTitle), "Edit %s", (pDef != NULL && pDef->sName != NULL) ? pDef->sName : sId);
	sTitle[sizeof(sTitle) - 1] = '\0';
	(void)xuiWindowSetTitle(pApp->pComplexEditorWindow, sTitle);
	sValue = uiDesignNodeGetProperty(pNode, sId, (pDef != NULL) ? pDef->sDefaultValue : "");
	tSchema = __uiDesignComplexSchema(pNode, sId);
	pApp->bComplexEditorStructured = tSchema.bStructured;
	if ( pApp->bComplexEditorStructured ) {
		__uiDesignComplexEditorLoadStructured(pApp, pNode, sId, &tSchema, sValue);
	} else {
		(void)xuiTextEditSetText(pApp->pComplexEditorText, sValue);
	}
	__uiDesignComplexEditorLayout(pApp);
	(void)xuiWindowSetOpen(pApp->pComplexEditorWindow, 1);
	(void)xuiWindowBringToFront(pApp->pComplexEditorWindow);
	(void)xuiSetFocusWidget(pApp->pContext, pApp->bComplexEditorStructured ? pApp->pComplexEditorGrid : pApp->pComplexEditorText);
	return XUI_OK;
}

static int __uiDesignInspectorAction(xui_widget pWidget, int iProperty, const char* sId, xui_rect_t tWorldRect, void* pUser)
{
	(void)pWidget;
	(void)iProperty;
	(void)tWorldRect;
	return __uiDesignComplexEditorOpen((ui_design_app_t*)pUser, sId);
}

static int __uiDesignInspectorAddLayoutProperties(ui_design_app_t* pApp, const ui_design_node_t* pNode, int iCategory)
{
	const ui_design_layout_property_def_t* pDef;
	const char* sValue;
	int iProperty;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) || (pApp->pPropertyGrid == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrLayoutProperties); i++ ) {
		pDef = &g_arrLayoutProperties[i];
		sValue = uiDesignNodeGetProperty(pNode, pDef->sId, pDef->sDefaultValue);
		iProperty = __uiDesignInspectorAddProperty(pApp->pPropertyGrid, iCategory, pDef->sId, pDef->sName, pDef->sDescription, pDef->iType, sValue, 0);
		if ( pDef->iType == XUI_TABLE_CELL_TYPE_ENUM ) {
			__uiDesignInspectorSetEnumConfig(pApp->pPropertyGrid, iProperty, pDef->pEnumItems, pDef->iEnumCount, sValue);
		}
		if ( pDef->iType == XUI_TABLE_CELL_TYPE_CUSTOM ) {
			(void)xuiPropertyGridSetAction(pApp->pPropertyGrid, iProperty, __uiDesignInspectorAction, pApp);
		}
	}
	return XUI_OK;
}

static int __uiDesignInspectorAddCommonWidgetProperties(ui_design_app_t* pApp, const ui_design_node_t* pNode, int iCategory)
{
	const ui_design_layout_property_def_t* pDef;
	const char* sValue;
	int iProperty;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) || (pApp->pPropertyGrid == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrCommonWidgetProperties); i++ ) {
		pDef = &g_arrCommonWidgetProperties[i];
		sValue = uiDesignNodeGetProperty(pNode, pDef->sId, pDef->sDefaultValue);
		iProperty = __uiDesignInspectorAddProperty(pApp->pPropertyGrid, iCategory, pDef->sId, pDef->sName, pDef->sDescription, pDef->iType, sValue, 0);
		if ( pDef->iType == XUI_TABLE_CELL_TYPE_ENUM ) {
			__uiDesignInspectorSetEnumConfig(pApp->pPropertyGrid, iProperty, pDef->pEnumItems, pDef->iEnumCount, sValue);
		}
		if ( pDef->iType == XUI_TABLE_CELL_TYPE_CUSTOM ) {
			(void)xuiPropertyGridSetAction(pApp->pPropertyGrid, iProperty, __uiDesignInspectorAction, pApp);
		}
	}
	return XUI_OK;
}

static int __uiDesignInspectorNodeSupportsFont(ui_design_node_type_t iType)
{
	switch ( iType ) {
	case UI_DESIGN_NODE_LABEL:
	case UI_DESIGN_NODE_HYPERLINK:
	case UI_DESIGN_NODE_BUTTON:
	case UI_DESIGN_NODE_CHECKBOX:
	case UI_DESIGN_NODE_RADIO:
	case UI_DESIGN_NODE_RADIO_GROUP:
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
		return 1;
	default:
		return 0;
	}
}

static int __uiDesignInspectorAddFontProperties(ui_design_app_t* pApp, const ui_design_node_t* pNode, int iCategory)
{
	const ui_design_layout_property_def_t* pDef;
	const char* sValue;
	int iProperty;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) || (pApp->pPropertyGrid == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__uiDesignInspectorNodeSupportsFont(pNode->iType) ) return XUI_OK;
	for ( i = 0; i < UI_DESIGN_COUNT_OF(g_arrFontProperties); i++ ) {
		pDef = &g_arrFontProperties[i];
		sValue = uiDesignNodeGetProperty(pNode, pDef->sId, pDef->sDefaultValue);
		iProperty = __uiDesignInspectorAddProperty(pApp->pPropertyGrid, iCategory, pDef->sId, pDef->sName, pDef->sDescription, pDef->iType, sValue, 0);
		if ( pDef->iType == XUI_TABLE_CELL_TYPE_ENUM ) {
			__uiDesignInspectorSetEnumConfig(pApp->pPropertyGrid, iProperty, pDef->pEnumItems, pDef->iEnumCount, sValue);
		}
	}
	return XUI_OK;
}

static int __uiDesignInspectorAddControlProperties(ui_design_app_t* pApp, const ui_design_node_t* pNode)
{
	const ui_design_control_desc_t* pDesc;
	const ui_design_property_def_t* pDef;
	const char* sValue;
	int iCategory;
	int iProperty;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) || (pApp->pPropertyGrid == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pDesc = uiDesignRegistryFind(pNode->iType);
	if ( pDesc == NULL ) return XUI_OK;
	for ( i = 0; i < pDesc->iPropertyCount; i++ ) {
		pDef = &pDesc->pProperties[i];
		iCategory = __uiDesignInspectorEnsureCategory(pApp->pPropertyGrid, pDef->sCategory, pDef->sCategory);
		if ( iCategory < 0 ) return XUI_ERROR;
		sValue = uiDesignNodeGetProperty(pNode, pDef->sId, pDef->sDefaultValue);
		iProperty = __uiDesignInspectorAddProperty(pApp->pPropertyGrid, iCategory, pDef->sId, pDef->sName, pDef->sDescription, pDef->iType, sValue, 0);
		if ( pDef->iType == XUI_TABLE_CELL_TYPE_ENUM ) {
			__uiDesignInspectorSetEnumConfig(pApp->pPropertyGrid, iProperty, pDef->pEnumItems, pDef->iEnumCount, sValue);
		}
		if ( pDef->iEditorKind == UI_DESIGN_PROPERTY_COMPLEX || pDef->iType == XUI_TABLE_CELL_TYPE_CUSTOM ||
		     __uiDesignComplexEquals(pDef->sId, "style.inlineProperties") || __uiDesignComplexEquals(pDef->sId, "cache.stateIds") ) {
			(void)xuiPropertyGridSetAction(pApp->pPropertyGrid, iProperty, __uiDesignInspectorAction, pApp);
		}
	}
	return XUI_OK;
}

static int __uiDesignInspectorPrecreateControlCategories(ui_design_app_t* pApp, const ui_design_node_t* pNode)
{
	const ui_design_control_desc_t* pDesc;
	const ui_design_property_def_t* pDef;
	int i;

	if ( (pApp == NULL) || (pNode == NULL) || (pApp->pPropertyGrid == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pDesc = uiDesignRegistryFind(pNode->iType);
	if ( pDesc == NULL ) return XUI_OK;
	for ( i = 0; i < pDesc->iPropertyCount; i++ ) {
		pDef = &pDesc->pProperties[i];
		if ( __uiDesignInspectorEnsureCategory(pApp->pPropertyGrid, pDef->sCategory, pDef->sCategory) < 0 ) return XUI_ERROR;
	}
	return XUI_OK;
}

static int __uiDesignInspectorCanEditGeometry(ui_design_app_t* pApp, const ui_design_node_t* pNode)
{
	if ( (pApp == NULL) || (pNode == NULL) ) return 0;
	return uiDesignModelCanFreeTransformNode(&pApp->tModel, pNode);
}

static int __uiDesignInspectorIsGeometryId(const char* sId)
{
	return (sId != NULL) &&
		(strcmp(sId, "x") == 0 ||
		 strcmp(sId, "y") == 0 ||
		 strcmp(sId, "width") == 0 ||
		 strcmp(sId, "height") == 0);
}

static int __uiDesignInspectorRefreshProperties(ui_design_app_t* pApp)
{
	ui_design_node_t* pNode;
	char sType[64];
	char sId[32];
	char sX[32];
	char sY[32];
	char sW[32];
	char sH[32];
	int iIdentity;
	int iGeometry;
	int iCommon;
	int iFont;
	int iLayout;
	int iControl;
	int iGeometryFlags;
	int iRet;

	if ( (pApp == NULL) || (pApp->pPropertyGrid == NULL) ) return XUI_OK;
	pApp->bSyncingInspector = 1;
	iRet = xuiPropertyGridClear(pApp->pPropertyGrid);
	if ( iRet != XUI_OK ) {
		pApp->bSyncingInspector = 0;
		return iRet;
	}
	pNode = uiDesignModelGetSelected(&pApp->tModel);
	if ( pNode == NULL ) {
		iIdentity = xuiPropertyGridAddCategory(pApp->pPropertyGrid, "selection", "Selection", 1);
		if ( iIdentity >= 0 ) {
			(void)__uiDesignInspectorAddProperty(pApp->pPropertyGrid, iIdentity, "empty", "Selected", "No control is selected.", XUI_TABLE_CELL_TYPE_TEXT, "None", XUI_PROPERTY_FLAG_READONLY);
		}
		pApp->bSyncingInspector = 0;
		return XUI_OK;
	}
	iIdentity = xuiPropertyGridAddCategory(pApp->pPropertyGrid, "identity", "Identity", 1);
	iGeometry = xuiPropertyGridAddCategory(pApp->pPropertyGrid, "geometry", "Geometry", 1);
	if ( iIdentity < 0 || iGeometry < 0 ) {
		pApp->bSyncingInspector = 0;
		return XUI_ERROR;
	}
	iRet = __uiDesignInspectorPrecreateControlCategories(pApp, pNode);
	if ( iRet != XUI_OK ) {
		pApp->bSyncingInspector = 0;
		return iRet;
	}
	iCommon = xuiPropertyGridAddCategory(pApp->pPropertyGrid, "common", "Common", 1);
	iFont = __uiDesignInspectorNodeSupportsFont(pNode->iType) ? xuiPropertyGridAddCategory(pApp->pPropertyGrid, "font", "Font", 0) : -1;
	iLayout = xuiPropertyGridAddCategory(pApp->pPropertyGrid, "layout", "Layout", 0);
	iControl = xuiPropertyGridAddCategory(pApp->pPropertyGrid, "control", "Control", 1);
	if ( iCommon < 0 || (__uiDesignInspectorNodeSupportsFont(pNode->iType) && iFont < 0) || iLayout < 0 || iControl < 0 ) {
		pApp->bSyncingInspector = 0;
		return XUI_ERROR;
	}
	snprintf(sType, sizeof(sType), "%s", __uiDesignInspectorControlTitle(pNode->iType));
	snprintf(sId, sizeof(sId), "%d", pNode->iId);
	snprintf(sX, sizeof(sX), "%.0f", pNode->tRect.fX);
	snprintf(sY, sizeof(sY), "%.0f", pNode->tRect.fY);
	snprintf(sW, sizeof(sW), "%.0f", pNode->tRect.fW);
	snprintf(sH, sizeof(sH), "%.0f", pNode->tRect.fH);
	iGeometryFlags = __uiDesignInspectorCanEditGeometry(pApp, pNode) ? 0 : (XUI_PROPERTY_FLAG_READONLY | XUI_PROPERTY_FLAG_DISABLED);
	(void)__uiDesignInspectorAddProperty(pApp->pPropertyGrid, iIdentity, "type", "Type", "XUI control type.", XUI_TABLE_CELL_TYPE_TEXT, sType, XUI_PROPERTY_FLAG_READONLY);
	(void)__uiDesignInspectorAddProperty(pApp->pPropertyGrid, iIdentity, "id", "Id", "Designer node id.", XUI_TABLE_CELL_TYPE_TEXT, sId, XUI_PROPERTY_FLAG_READONLY);
	(void)__uiDesignInspectorAddProperty(pApp->pPropertyGrid, iGeometry, "x", "X", "Left position relative to the parent container. Disabled when parent layout manages placement.", XUI_TABLE_CELL_TYPE_FLOAT, sX, iGeometryFlags);
	(void)__uiDesignInspectorAddProperty(pApp->pPropertyGrid, iGeometry, "y", "Y", "Top position relative to the parent container. Disabled when parent layout manages placement.", XUI_TABLE_CELL_TYPE_FLOAT, sY, iGeometryFlags);
	(void)__uiDesignInspectorAddProperty(pApp->pPropertyGrid, iGeometry, "width", "Width", "Control width. Disabled when parent layout manages size.", XUI_TABLE_CELL_TYPE_FLOAT, sW, iGeometryFlags);
	(void)__uiDesignInspectorAddProperty(pApp->pPropertyGrid, iGeometry, "height", "Height", "Control height. Disabled when parent layout manages size.", XUI_TABLE_CELL_TYPE_FLOAT, sH, iGeometryFlags);
	(void)__uiDesignInspectorAddProperty(pApp->pPropertyGrid, iCommon, "text", "Text", "Displayed text.", XUI_TABLE_CELL_TYPE_TEXT, pNode->sText, 0);
	(void)__uiDesignInspectorAddProperty(pApp->pPropertyGrid, iCommon, "visible", "Visible", "Controls whether the widget is visible.", XUI_TABLE_CELL_TYPE_BOOL, pNode->bVisible ? "true" : "false", 0);
	(void)__uiDesignInspectorAddProperty(pApp->pPropertyGrid, iCommon, "enabled", "Enabled", "Controls whether the widget is enabled.", XUI_TABLE_CELL_TYPE_BOOL, pNode->bEnabled ? "true" : "false", 0);
	iRet = __uiDesignInspectorAddCommonWidgetProperties(pApp, pNode, iCommon);
	if ( iRet != XUI_OK ) {
		pApp->bSyncingInspector = 0;
		return iRet;
	}
	if ( (pNode->iType == UI_DESIGN_NODE_CHECKBOX) || (pNode->iType == UI_DESIGN_NODE_RADIO) ) {
		(void)__uiDesignInspectorAddProperty(pApp->pPropertyGrid, iControl, "checked", "Checked", "Checked state.", XUI_TABLE_CELL_TYPE_BOOL, pNode->bChecked ? "true" : "false", 0);
	}
	if ( iFont >= 0 ) {
		iRet = __uiDesignInspectorAddFontProperties(pApp, pNode, iFont);
		if ( iRet != XUI_OK ) {
			pApp->bSyncingInspector = 0;
			return iRet;
		}
	}
	iRet = __uiDesignInspectorAddLayoutProperties(pApp, pNode, iLayout);
	if ( iRet != XUI_OK ) {
		pApp->bSyncingInspector = 0;
		return iRet;
	}
	iRet = __uiDesignInspectorAddControlProperties(pApp, pNode);
	if ( iRet != XUI_OK ) {
		pApp->bSyncingInspector = 0;
		return iRet;
	}
	pApp->bSyncingInspector = 0;
	return XUI_OK;
}

static int __uiDesignInspectorParseFloat(const char* sValue, float* pValue)
{
	char* pEnd;
	double fValue;

	if ( (sValue == NULL) || (sValue[0] == '\0') || (pValue == NULL) ) return 0;
	fValue = strtod(sValue, &pEnd);
	if ( pEnd == sValue ) return 0;
	while ( (pEnd != NULL) && (*pEnd == ' ' || *pEnd == '\t') ) pEnd++;
	if ( (pEnd == NULL) || (*pEnd != '\0') ) return 0;
	*pValue = (float)fValue;
	return 1;
}

static int __uiDesignInspectorValidate(xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	ui_design_app_t* pApp;
	ui_design_node_t* pNode;
	float fValue;
	int iValue;

	(void)pWidget;
	(void)iProperty;
	if ( sId == NULL ) return 1;
	pApp = (ui_design_app_t*)pUser;
	pNode = (pApp != NULL) ? uiDesignModelGetSelected(&pApp->tModel) : NULL;
	if ( __uiDesignInspectorIsGeometryId(sId) && !__uiDesignInspectorCanEditGeometry(pApp, pNode) ) return 0;
	if ( strcmp(sId, "width") == 0 || strcmp(sId, "height") == 0 ) {
		return __uiDesignInspectorParseFloat(sValue, &fValue) && fValue >= 8.0f;
	}
	if ( strcmp(sId, "x") == 0 || strcmp(sId, "y") == 0 ) {
		return __uiDesignInspectorParseFloat(sValue, &fValue);
	}
	if ( iType == XUI_TABLE_CELL_TYPE_FLOAT ) {
		if ( !__uiDesignInspectorParseFloat(sValue, &fValue) ) return 0;
		if ( strcmp(sId, "metrics.offsetX") == 0 ||
		     strcmp(sId, "metrics.offsetY") == 0 ) return 1;
		if ( strncmp(sId, "metrics.", 8) == 0 ||
		     strncmp(sId, "layout.", 7) == 0 ||
		     strcmp(sId, "source.w") == 0 ||
		     strcmp(sId, "source.h") == 0 ||
		     strcmp(sId, "custom.w") == 0 ||
		     strcmp(sId, "custom.h") == 0 ||
		     strcmp(sId, "icon.w") == 0 ||
		     strcmp(sId, "icon.h") == 0 ||
		     strcmp(sId, "unchecked.w") == 0 ||
		     strcmp(sId, "unchecked.h") == 0 ||
		     strcmp(sId, "checked.w") == 0 ||
		     strcmp(sId, "checked.h") == 0 ||
		     strcmp(sId, "base.w") == 0 ||
		     strcmp(sId, "base.h") == 0 ||
		     strcmp(sId, "baseActive.w") == 0 ||
		     strcmp(sId, "baseActive.h") == 0 ||
		     strcmp(sId, "knob.w") == 0 ||
		     strcmp(sId, "knob.h") == 0 ||
		     strcmp(sId, "knobActive.w") == 0 ||
		     strcmp(sId, "knobActive.h") == 0 ||
		     strcmp(sId, "ripple.w") == 0 ||
		     strcmp(sId, "ripple.h") == 0 ||
		     strcmp(sId, "appearance.borderWidth") == 0 ||
		     strcmp(sId, "appearance.hoverBorderWidth") == 0 ||
		     strcmp(sId, "appearance.activeBorderWidth") == 0 ||
		     strcmp(sId, "appearance.focusBorderWidth") == 0 ||
		     strcmp(sId, "appearance.disabledBorderWidth") == 0 ||
		     strcmp(sId, "appearance.checkedBorderWidth") == 0 ||
		     strcmp(sId, "text.lineGap") == 0 ||
		     strcmp(sId, "text.paragraphGap") == 0 ||
		     strcmp(sId, "common.tooltipDelay") == 0 ||
		     strcmp(sId, "common.tooltipAnchorW") == 0 ||
		     strcmp(sId, "common.tooltipAnchorH") == 0 ||
		     strcmp(sId, "behavior.animationDuration") == 0 ) {
			return fValue >= 0.0f;
		}
		return 1;
	}
	if ( iType == XUI_TABLE_CELL_TYPE_INT || iType == XUI_TABLE_CELL_TYPE_ENUM ) {
		if ( (sValue == NULL) || (sValue[0] == '\0') ) return 0;
		iValue = atoi(sValue);
		if ( strcmp(sId, "layout.tableRows") == 0 ||
		     strcmp(sId, "layout.tableColumns") == 0 ||
		     strcmp(sId, "layout.gridColumns") == 0 ||
		     strcmp(sId, "data.paneCount") == 0 ||
		     strcmp(sId, "layout.tableCellRowSpan") == 0 ||
		     strcmp(sId, "layout.tableCellColumnSpan") == 0 ) return iValue >= 1;
		if ( strcmp(sId, "layout.tableCellRow") == 0 ||
		     strcmp(sId, "layout.tableCellColumn") == 0 ||
		     strcmp(sId, "layout.tableTrackRow") == 0 ||
		     strcmp(sId, "layout.tableTrackColumn") == 0 ||
		     strcmp(sId, "layout.carouselPage") == 0 ||
		     strcmp(sId, "layout.splitPane") == 0 ||
		     strcmp(sId, "layout.tabPage") == 0 ||
		     strcmp(sId, "layout.accordionSection") == 0 ||
		     strcmp(sId, "layout.dockWindow") == 0 ||
		     strcmp(sId, "data.current") == 0 ||
		     strcmp(sId, "data.selected") == 0 ||
		     strcmp(sId, "behavior.maxTags") == 0 ||
		     strcmp(sId, "text.maxLength") == 0 ||
		     strcmp(sId, "common.tabIndex") == 0 ||
		     strcmp(sId, "cache.tileWidth") == 0 ||
		     strcmp(sId, "cache.tileHeight") == 0 ||
		     strcmp(sId, "cache.maxBytes") == 0 ||
		     strcmp(sId, "cache.stateCount") == 0 ) return iValue >= 0;
		return 1;
	}
	return 1;
}

static void __uiDesignInspectorChange(xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	ui_design_app_t* pApp;
	ui_design_node_t* pNode;
	xui_rect_t tRect;
	float fValue;
	int bBool;
	int bTreeChanged;
	const char* sNewValue;
	char sBool[8];

	pApp = (ui_design_app_t*)pUser;
	if ( (pApp == NULL) || (pWidget == NULL) || (sId == NULL) || pApp->bSyncingInspector ) return;
	pNode = uiDesignModelGetSelected(&pApp->tModel);
	if ( pNode == NULL ) return;
	if ( __uiDesignInspectorIsGeometryId(sId) && !__uiDesignInspectorCanEditGeometry(pApp, pNode) ) return;
	tRect = pNode->tRect;
	bTreeChanged = 0;
	pApp->bSyncingInspector = 1;
	if ( strcmp(sId, "text") == 0 ) {
		(void)uiDesignAppCommandSetNodeText(pApp, pNode->iId, sValue);
		bTreeChanged = 1;
	} else if ( strcmp(sId, "x") == 0 && __uiDesignInspectorParseFloat(sValue, &fValue) ) {
		tRect.fX = fValue;
		(void)uiDesignAppCommandSetNodeRect(pApp, pNode->iId, tRect, "Edit Geometry");
	} else if ( strcmp(sId, "y") == 0 && __uiDesignInspectorParseFloat(sValue, &fValue) ) {
		tRect.fY = fValue;
		(void)uiDesignAppCommandSetNodeRect(pApp, pNode->iId, tRect, "Edit Geometry");
	} else if ( strcmp(sId, "width") == 0 && __uiDesignInspectorParseFloat(sValue, &fValue) ) {
		tRect.fW = fValue;
		(void)uiDesignAppCommandSetNodeRect(pApp, pNode->iId, tRect, "Edit Geometry");
	} else if ( strcmp(sId, "height") == 0 && __uiDesignInspectorParseFloat(sValue, &fValue) ) {
		tRect.fH = fValue;
		(void)uiDesignAppCommandSetNodeRect(pApp, pNode->iId, tRect, "Edit Geometry");
	} else if ( strcmp(sId, "visible") == 0 ) {
		bBool = xuiPropertyGridGetBool(pWidget, iProperty, pNode->bVisible);
		(void)uiDesignAppCommandSetNodeVisible(pApp, pNode->iId, bBool);
	} else if ( strcmp(sId, "enabled") == 0 ) {
		bBool = xuiPropertyGridGetBool(pWidget, iProperty, pNode->bEnabled);
		(void)uiDesignAppCommandSetNodeEnabled(pApp, pNode->iId, bBool);
	} else if ( strcmp(sId, "checked") == 0 ) {
		bBool = xuiPropertyGridGetBool(pWidget, iProperty, pNode->bChecked);
		snprintf(sBool, sizeof(sBool), "%s", bBool ? "true" : "false");
		(void)uiDesignAppCommandSetNodeProperty(pApp, pNode->iId, sId, sBool);
	} else {
		sNewValue = sValue;
		if ( iType == XUI_TABLE_CELL_TYPE_BOOL ) {
			bBool = xuiPropertyGridGetBool(pWidget, iProperty, 0);
			snprintf(sBool, sizeof(sBool), "%s", bBool ? "true" : "false");
			sNewValue = sBool;
		} else {
			sNewValue = xuiPropertyGridGetValue(pWidget, iProperty);
			if ( sNewValue == NULL ) sNewValue = sValue;
		}
		(void)uiDesignAppCommandSetNodeProperty(pApp, pNode->iId, sId, sNewValue);
	}
	pApp->bSyncingInspector = 0;
	if ( bTreeChanged ) {
		(void)__uiDesignInspectorRefreshTree(pApp);
	}
	uiDesignAppInvalidate(pApp);
}

static void __uiDesignInspectorSelectTree(xui_widget pWidget, int iNodeId, void* pUser)
{
	ui_design_app_t* pApp;

	(void)pWidget;
	pApp = (ui_design_app_t*)pUser;
	if ( (pApp == NULL) || pApp->bSyncingTree ) return;
	(void)uiDesignAppSelectNode(pApp, iNodeId);
}

static int __uiDesignInspectorExerciseOneComplex(ui_design_app_t* pApp, ui_design_node_t* pNode, const char* sPropertyId, int bRequireStructured)
{
	const char* sValue;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) || (sPropertyId == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	(void)uiDesignModelSetSelected(&pApp->tModel, pNode->iId);
	iRet = __uiDesignComplexEditorOpen(pApp, sPropertyId);
	if ( iRet != XUI_OK ) {
		printf("xui_uidesign exercise-complex-open-failed type=%s id=%d property=%s ret=%d\n",
			uiDesignNodeTypeName(pNode->iType), pNode->iId, sPropertyId, iRet);
		return iRet;
	}
	__uiDesignComplexEditorOk(NULL, pApp);
	sValue = uiDesignNodeGetProperty(pNode, sPropertyId, "");
	if ( bRequireStructured && (sValue == NULL || (strchr(sValue, '|') == NULL && strchr(sValue, ',') == NULL)) ) {
		printf("xui_uidesign exercise-complex-serialize-failed type=%s id=%d property=%s\n",
			uiDesignNodeTypeName(pNode->iType), pNode->iId, sPropertyId);
		return XUI_ERROR;
	}
	return XUI_OK;
}

int uiDesignInspectorExerciseComplexEditors(ui_design_app_t* pApp)
{
	ui_design_node_t* pNode;
	int bButtonStatePatches;
	int bInlineStyles;
	int bCacheStateIds;
	int bLayoutTableRowsConfig;
	int bLayoutTableColumnsConfig;
	int bInputDecorations;
	int bInputMenuTitles;
	int bTextEditFindPreview;
	int bTree;
	int bComboBoxItems;
	int bListViewItems;
	int bCascader;
	int bBreadcrumb;
	int bTagInputTags;
	int bStepBarSteps;
	int bTimelineLayers;
	int bTimelineFrames;
	int bTimelineSpans;
	int bTimelineSelection;
	int bTimelineMenuTitles;
	int bDock;
	int bDockRegions;
	int bDockMenuTitles;
	int bDockTooltips;
	int bSplit;
	int bTabsItems;
	int bAccordionSections;
	int bCanvasCommands;
	int bPopupContent;
	int bTerminalPalette;
	int bTerminalSelection;
	int bTerminalMenuTitles;
	int bCodeEditMarkers;
	int bCodeEditIndicators;
	int bCodeEditDiagnostics;
	int bCodeEditMargins;
	int bCodeEditSelections;
	int bCodeEditFolds;
	int bCodeEditStyles;
	int bCodeEditTokenStyles;
	int bCodeEditMenuTitles;
	int bCodeEditFindPreview;
	int bFlowNodes;
	int bFlowPorts;
	int bFlowNodeConfigs;
	int bFlowEdges;
	int bFlowDiagnostics;
	int bWorkflowNodeTypes;
	int bWorkflowConfigFields;
	int bWorkflowTypePorts;
	int bWorkflowNodes;
	int bWorkflowNodeConfigs;
	int bWorkflowEdges;
	int bWorkflowVariables;
	int bWorkflowDiagnostics;
	int bVirtualJoystickChannels;
	int bInventorySlots;
	int bInventorySelectedSlots;
	int bInventoryCurrentFallback;
	int bRadioGroupOptions;
	int bMsgBoxButtonTitles;
	int bMsgBoxButtons;
	int bMsgTipOffset;
	int bFileFilter;
	int bFileRoots;
	int bFileEntries;
	int bFileButtonTitles;
	int bToastQueue;
	int bColorPickerPalette;
	int bColorPickerHex;
	int bMenuBarItems;
	int bMenuBarMenus;
	int bMenuBarSubmenus;
	int bToolbarItems;
	int bStatusBarItems;
	int bMenuItems;
	int bMenuSubmenus;
	int bChartSeriesList;
	int bChartSeries;
	int bTableViewColumns;
	int bTableViewRows;
	int bTableViewRowStyles;
	int bTableViewStyles;
	int bTableViewMerges;
	int bTableGridColumns;
	int bTableGridRows;
	int bTableGridRowStyles;
	int bTableGridStyles;
	int bTableGridMerges;
	int bTableGridEditorConfigs;
	int bMessageListMessages;
	int bMessageListImportText;
	int bPropertyGridProperties;
	int bSourceEditor;
	char sLayoutTable[16];
	int i;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	snprintf(sLayoutTable, sizeof(sLayoutTable), "%d", XUI_LAYOUT_TABLE);
	sLayoutTable[sizeof(sLayoutTable) - 1] = '\0';
	bButtonStatePatches = 0;
	bInlineStyles = 0;
	bCacheStateIds = 0;
	bLayoutTableRowsConfig = 0;
	bLayoutTableColumnsConfig = 0;
	bInputDecorations = 0;
	bInputMenuTitles = 0;
	bTextEditFindPreview = 0;
	bTree = 0;
	bComboBoxItems = 0;
	bListViewItems = 0;
	bCascader = 0;
	bBreadcrumb = 0;
	bTagInputTags = 0;
	bStepBarSteps = 0;
	bTimelineLayers = 0;
	bTimelineFrames = 0;
	bTimelineSpans = 0;
	bTimelineSelection = 0;
	bTimelineMenuTitles = 0;
	bDock = 0;
	bDockRegions = 0;
	bDockMenuTitles = 0;
	bDockTooltips = 0;
	bSplit = 0;
	bTabsItems = 0;
	bAccordionSections = 0;
	bCanvasCommands = 0;
	bPopupContent = 0;
	bTerminalPalette = 0;
	bTerminalSelection = 0;
	bTerminalMenuTitles = 0;
	bCodeEditMarkers = 0;
	bCodeEditIndicators = 0;
	bCodeEditDiagnostics = 0;
	bCodeEditMargins = 0;
	bCodeEditSelections = 0;
	bCodeEditFolds = 0;
	bCodeEditStyles = 0;
	bCodeEditTokenStyles = 0;
	bCodeEditMenuTitles = 0;
	bCodeEditFindPreview = 0;
	bFlowNodes = 0;
	bFlowPorts = 0;
	bFlowNodeConfigs = 0;
	bFlowEdges = 0;
	bFlowDiagnostics = 0;
	bWorkflowNodeTypes = 0;
	bWorkflowConfigFields = 0;
	bWorkflowTypePorts = 0;
	bWorkflowNodes = 0;
	bWorkflowNodeConfigs = 0;
	bWorkflowEdges = 0;
	bWorkflowVariables = 0;
	bWorkflowDiagnostics = 0;
	bVirtualJoystickChannels = 0;
	bInventorySlots = 0;
	bInventorySelectedSlots = 0;
	bInventoryCurrentFallback = 0;
	bRadioGroupOptions = 0;
	bMsgBoxButtonTitles = 0;
	bMsgBoxButtons = 0;
	bMsgTipOffset = 0;
	bFileFilter = 0;
	bFileRoots = 0;
	bFileEntries = 0;
	bFileButtonTitles = 0;
	bToastQueue = 0;
	bColorPickerPalette = 0;
	bColorPickerHex = 0;
	bMenuBarItems = 0;
	bMenuBarMenus = 0;
	bMenuBarSubmenus = 0;
	bToolbarItems = 0;
	bStatusBarItems = 0;
	bMenuItems = 0;
	bMenuSubmenus = 0;
	bChartSeriesList = 0;
	bChartSeries = 0;
	bTableViewColumns = 0;
	bTableViewRows = 0;
	bTableViewRowStyles = 0;
	bTableViewStyles = 0;
	bTableViewMerges = 0;
	bTableGridColumns = 0;
	bTableGridRows = 0;
	bTableGridRowStyles = 0;
	bTableGridStyles = 0;
	bTableGridMerges = 0;
	bTableGridEditorConfigs = 0;
	bMessageListMessages = 0;
	bMessageListImportText = 0;
	bPropertyGridProperties = 0;
	bSourceEditor = 0;
	for ( i = 0; i < pApp->tModel.iNodeCount; ++i ) {
		pNode = &pApp->tModel.arrNodes[i];
		if ( (!bLayoutTableRowsConfig || !bLayoutTableColumnsConfig) && pNode->iType == UI_DESIGN_NODE_WIDGET ) {
			iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "layout.type", sLayoutTable);
			if ( iRet != XUI_OK ) return iRet;
			iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "layout.tableRows", "2");
			if ( iRet != XUI_OK ) return iRet;
			iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "layout.tableColumns", "2");
			if ( iRet != XUI_OK ) return iRet;
			if ( !bLayoutTableRowsConfig ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "layout.tableRowsConfig", "0|Fixed|36|0||1\n1|Fill|0|24||1");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "layout.tableRowsConfig", 1);
				if ( iRet != XUI_OK ) return iRet;
				bLayoutTableRowsConfig = 1;
			}
			if ( !bLayoutTableColumnsConfig ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "layout.tableColumnsConfig", "0|Content|0|80||1\n1|Fill|0|120||2");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "layout.tableColumnsConfig", 1);
				if ( iRet != XUI_OK ) return iRet;
				bLayoutTableColumnsConfig = 1;
			}
		} else if ( (!bButtonStatePatches || !bInlineStyles || !bCacheStateIds) && pNode->iType == UI_DESIGN_NODE_BUTTON ) {
			if ( !bButtonStatePatches ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.statePatches", 1);
				if ( iRet != XUI_OK ) return iRet;
				bButtonStatePatches = 1;
			}
			if ( !bInlineStyles ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "style.inlineProperties", "button.normal_color|color|#D9EAFE|render");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "style.inlineProperties", 1);
				if ( iRet != XUI_OK ) return iRet;
				bInlineStyles = 1;
			}
			if ( !bCacheStateIds ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "cache.policy", "1");
				if ( iRet != XUI_OK ) return iRet;
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "cache.clearOnUpdate", "true");
				if ( iRet != XUI_OK ) return iRet;
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "cache.stateIds", "0|0\n1|1");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "cache.stateIds", 1);
				if ( iRet != XUI_OK ) return iRet;
				bCacheStateIds = 1;
			}
		} else if ( (!bInputDecorations || !bInputMenuTitles) && pNode->iType == UI_DESIGN_NODE_INPUT ) {
			if ( !bInputDecorations ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.decorations",
					"leading|icon|always|22|0|search|||||||#4C596DD2|#2F80ED|#215EAA|#96A0AC\n"
					"trailing|clear|notEmpty|22|0|none|||||||#4C596DD2|#D64856|#A63843|#96A0AC");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.decorations", 1);
				if ( iRet != XUI_OK ) return iRet;
				bInputDecorations = 1;
			}
			if ( !bInputMenuTitles ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.menuTitles", "undo|Undo Text\nselectAll|Select All Text");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.menuTitles", 1);
				if ( iRet != XUI_OK ) return iRet;
				bInputMenuTitles = 1;
			}
		} else if ( !bTextEditFindPreview && pNode->iType == UI_DESIGN_NODE_TEXT_EDIT ) {
			iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "find.pattern", "Line");
			if ( iRet != XUI_OK ) return iRet;
			iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "find.replacement", "Row");
			if ( iRet != XUI_OK ) return iRet;
			iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "find.previewMode", "1");
			if ( iRet != XUI_OK ) return iRet;
			iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "behavior.findWindow", "1");
			if ( iRet != XUI_OK ) return iRet;
			bTextEditFindPreview = 1;
		} else if ( !bTree && pNode->iType == UI_DESIGN_NODE_TREEVIEW ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.nodes", 1);
			if ( iRet != XUI_OK ) return iRet;
			bTree = 1;
		} else if ( !bComboBoxItems && pNode->iType == UI_DESIGN_NODE_COMBOBOX ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.items", 1);
			if ( iRet != XUI_OK ) return iRet;
			bComboBoxItems = 1;
		} else if ( !bListViewItems && pNode->iType == UI_DESIGN_NODE_LISTVIEW ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.items", 1);
			if ( iRet != XUI_OK ) return iRet;
			bListViewItems = 1;
		} else if ( !bCascader && pNode->iType == UI_DESIGN_NODE_CASCADER ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.items", 1);
			if ( iRet != XUI_OK ) return iRet;
			bCascader = 1;
		} else if ( !bBreadcrumb && pNode->iType == UI_DESIGN_NODE_BREADCRUMB ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.items", 1);
			if ( iRet != XUI_OK ) return iRet;
			bBreadcrumb = 1;
		} else if ( !bTagInputTags && pNode->iType == UI_DESIGN_NODE_TAG_INPUT ) {
			iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "text.inputText", "gamma");
			if ( iRet != XUI_OK ) return iRet;
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.tags", 0);
			if ( iRet != XUI_OK ) return iRet;
			bTagInputTags = 1;
		} else if ( !bStepBarSteps && pNode->iType == UI_DESIGN_NODE_STEP_BAR ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.steps", 0);
			if ( iRet != XUI_OK ) return iRet;
			bStepBarSteps = 1;
		} else if ( (!bTimelineLayers || !bTimelineFrames || !bTimelineSpans || !bTimelineSelection || !bTimelineMenuTitles) && pNode->iType == UI_DESIGN_NODE_TIMELINE_VIEW ) {
			if ( !bTimelineLayers ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.timelineLayers", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTimelineLayers = 1;
			}
			if ( !bTimelineFrames ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.timelineFrames", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTimelineFrames = 1;
			}
			if ( !bTimelineSpans ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.timelineSpans", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTimelineSpans = 1;
			}
			if ( !bTimelineSelection ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.selection", "Layer 1|24|Layer 1|24|true\nLayer 2|12|Layer 2|12|true");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.selection", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTimelineSelection = 1;
			}
			if ( !bTimelineMenuTitles ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.menuTitles",
					"rename|Rename Track\n"
					"insertKeyframe|Insert Key\n"
					"createSpanFromSelection|Create Span From Selection\n"
					"clearSpan|Clear Span");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.menuTitles", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTimelineMenuTitles = 1;
			}
		} else if ( (!bDock || !bDockRegions || !bDockMenuTitles || !bDockTooltips) && pNode->iType == UI_DESIGN_NODE_DOCK_PANEL ) {
			if ( !bDock ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.windows",
					"Document|fill|0|true|true|Document|doc|true|false|document|false|false|80|70|260|180\n"
					"Toolbox|left|0.25|true|true|Toolbox|leftTools|true|false|document|false|false|80|70|260|180\n"
					"Detached Help|none|0|true|true|Detached Help||false|false|document|false|true|90|80|220|140\n"
					"Hidden Log|bottom|0.22|true|true|Hidden Log||false|false|document|true|false|80|70|260|180");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.windows", 1);
				if ( iRet != XUI_OK ) return iRet;
				bDock = 1;
			}
			if ( !bDockRegions ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.regions", "left|portion|0.26\nright|pixel|180\nbottom|portion|0.22");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.regions", 1);
				if ( iRet != XUI_OK ) return iRet;
				bDockRegions = 1;
			}
			if ( !bDockMenuTitles ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.menuTitles",
					"float|Float Window\n"
					"autoHide|Auto Hide Window\n"
					"closeOthers|Close Other Windows\n"
					"closeAll|Close All Windows");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.menuTitles", 1);
				if ( iRet != XUI_OK ) return iRet;
				bDockMenuTitles = 1;
			}
			if ( !bDockTooltips ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.tooltips",
					"close|Close Dock Window\n"
					"autoHide|Auto Hide Dock Window\n"
					"options|Dock Window Options\n"
					"moreTabs|Show Hidden Tabs\n"
					"dock|Dock Window");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.tooltips", 1);
				if ( iRet != XUI_OK ) return iRet;
				bDockTooltips = 1;
			}
		} else if ( !bSplit && pNode->iType == UI_DESIGN_NODE_SPLIT_LAYOUT ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.panes", 1);
			if ( iRet != XUI_OK ) return iRet;
			bSplit = 1;
		} else if ( !bTabsItems && pNode->iType == UI_DESIGN_NODE_TABS ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.items", 1);
			if ( iRet != XUI_OK ) return iRet;
			bTabsItems = 1;
		} else if ( !bAccordionSections && pNode->iType == UI_DESIGN_NODE_ACCORDION ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.sections", 1);
			if ( iRet != XUI_OK ) return iRet;
			bAccordionSections = 1;
		} else if ( !bCanvasCommands && pNode->iType == UI_DESIGN_NODE_CANVAS ) {
			iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.commands",
				"line||20|20|||180|120||||||||2||#317ED6|\n"
				"triangleFill||70|110|||140|48|210|110|||||||||#CDE9D6|");
			if ( iRet != XUI_OK ) return iRet;
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.commands", 1);
			if ( iRet != XUI_OK ) return iRet;
			bCanvasCommands = 1;
		} else if ( !bPopupContent && pNode->iType == UI_DESIGN_NODE_POPUP ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.content", 1);
			if ( iRet != XUI_OK ) return iRet;
			bPopupContent = 1;
		} else if ( (!bTerminalPalette || !bTerminalSelection || !bTerminalMenuTitles) && pNode->iType == UI_DESIGN_NODE_TERMINAL ) {
			if ( !bTerminalPalette ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.palette", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTerminalPalette = 1;
			}
			if ( !bTerminalSelection ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.findText", "ready");
				if ( iRet != XUI_OK ) return iRet;
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.selection", "1|1|1|5");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.selection", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTerminalSelection = 1;
			}
			if ( !bTerminalMenuTitles ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.menuTitles",
					"copy|Copy Terminal Text\n"
					"clearScreen|Clear Terminal Screen\n"
					"find|Find Terminal Text");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.menuTitles", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTerminalMenuTitles = 1;
			}
		} else if ( (!bCodeEditMarkers || !bCodeEditIndicators || !bCodeEditDiagnostics || !bCodeEditMargins || !bCodeEditSelections || !bCodeEditFolds || !bCodeEditStyles || !bCodeEditTokenStyles || !bCodeEditMenuTitles || !bCodeEditFindPreview) && pNode->iType == UI_DESIGN_NODE_CODE_EDIT ) {
			if ( !bCodeEditMarkers ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.markers", 1);
				if ( iRet != XUI_OK ) return iRet;
				bCodeEditMarkers = 1;
			}
			if ( !bCodeEditIndicators ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.indicators", 1);
				if ( iRet != XUI_OK ) return iRet;
				bCodeEditIndicators = 1;
			}
			if ( !bCodeEditDiagnostics ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.diagnostics", 1);
				if ( iRet != XUI_OK ) return iRet;
				bCodeEditDiagnostics = 1;
			}
			if ( !bCodeEditMargins ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.margins",
					"1|change|4|true|false|false\n"
					"2|marker|16|true|true|false\n"
					"3|fold|14|true|true|false\n"
					"4|lineNumber|44|true|false|false\n"
					"5|diagnostic|14|true|true|false");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.margins", 1);
				if ( iRet != XUI_OK ) return iRet;
				bCodeEditMargins = 1;
			}
			if ( !bCodeEditSelections ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.selections", "4|8|-1|\n15|21|-1|inactive");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.selections", 1);
				if ( iRet != XUI_OK ) return iRet;
				bCodeEditSelections = 1;
			}
			if ( !bCodeEditFolds ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.folds", "1|3|0|header");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.folds", 1);
				if ( iRet != XUI_OK ) return iRet;
				bCodeEditFolds = 1;
			}
			if ( !bCodeEditStyles ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.styles",
					"keyword|#1F5FA8|#00000000|true|false|false|false\n"
					"comment|#6B7F95|#00000000|false|true|false|false\n"
					"searchResult|#233246|#FFE08A|false|false|false|false\n"
					"diagnosticError|#D64856|#00000000|false|false|true|false");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.styles", 1);
				if ( iRet != XUI_OK ) return iRet;
				bCodeEditStyles = 1;
			}
			if ( !bCodeEditTokenStyles ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.tokenStyles",
					"keyword|keyword\n"
					"type|type\n"
					"comment|comment\n"
					"error|diagnosticError");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.tokenStyles", 1);
				if ( iRet != XUI_OK ) return iRet;
				bCodeEditTokenStyles = 1;
			}
			if ( !bCodeEditMenuTitles ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.menuTitles",
					"find|Find in Code\n"
					"replace|Replace in Code\n"
					"toggleFold|Toggle Fold Text");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.menuTitles", 1);
				if ( iRet != XUI_OK ) return iRet;
				bCodeEditMenuTitles = 1;
			}
			if ( !bCodeEditFindPreview ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "find.pattern", "return");
				if ( iRet != XUI_OK ) return iRet;
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "find.replacement", "yield");
				if ( iRet != XUI_OK ) return iRet;
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "find.previewMode", "3");
				if ( iRet != XUI_OK ) return iRet;
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "behavior.findWindow", "1");
				if ( iRet != XUI_OK ) return iRet;
				bCodeEditFindPreview = 1;
			}
		} else if ( (!bFlowNodes || !bFlowPorts || !bFlowNodeConfigs || !bFlowEdges || !bFlowDiagnostics) && pNode->iType == UI_DESIGN_NODE_FLOW_GRAPH ) {
			if ( !bFlowNodes ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.nodes", 1);
				if ( iRet != XUI_OK ) return iRet;
				bFlowNodes = 1;
			}
			if ( !bFlowPorts ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.ports", 1);
				if ( iRet != XUI_OK ) return iRet;
				bFlowPorts = 1;
			}
			if ( !bFlowNodeConfigs ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.nodeConfigs", 1);
				if ( iRet != XUI_OK ) return iRet;
				bFlowNodeConfigs = 1;
			}
			if ( !bFlowEdges ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.edges", 1);
				if ( iRet != XUI_OK ) return iRet;
				bFlowEdges = 1;
			}
			if ( !bFlowDiagnostics ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.diagnostics", 1);
				if ( iRet != XUI_OK ) return iRet;
				bFlowDiagnostics = 1;
			}
		} else if ( (!bWorkflowNodeTypes || !bWorkflowConfigFields || !bWorkflowTypePorts || !bWorkflowNodes || !bWorkflowNodeConfigs || !bWorkflowEdges || !bWorkflowVariables || !bWorkflowDiagnostics) && pNode->iType == UI_DESIGN_NODE_WORKFLOW ) {
			if ( !bWorkflowNodeTypes ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.nodeTypes", 1);
				if ( iRet != XUI_OK ) return iRet;
				bWorkflowNodeTypes = 1;
			}
			if ( !bWorkflowConfigFields ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.configFields", 1);
				if ( iRet != XUI_OK ) return iRet;
				bWorkflowConfigFields = 1;
			}
			if ( !bWorkflowTypePorts ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.typePorts", 1);
				if ( iRet != XUI_OK ) return iRet;
				bWorkflowTypePorts = 1;
			}
			if ( !bWorkflowNodes ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.nodes", 1);
				if ( iRet != XUI_OK ) return iRet;
				bWorkflowNodes = 1;
			}
			if ( !bWorkflowNodeConfigs ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.nodeConfigs", 1);
				if ( iRet != XUI_OK ) return iRet;
				bWorkflowNodeConfigs = 1;
			}
			if ( !bWorkflowEdges ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.edges", 1);
				if ( iRet != XUI_OK ) return iRet;
				bWorkflowEdges = 1;
			}
			if ( !bWorkflowVariables ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.variables", "var_prompt|Prompt|string|workflow|user_prompt\nvar_retry|Retry Count|int|workflow|3\nvar_debug|Debug|bool|workflow|true");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.variables", 1);
				if ( iRet != XUI_OK ) return iRet;
				bWorkflowVariables = 1;
			}
			if ( !bWorkflowDiagnostics ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.diagnostics", 0);
				if ( iRet != XUI_OK ) return iRet;
				bWorkflowDiagnostics = 1;
			}
		} else if ( !bVirtualJoystickChannels && pNode->iType == UI_DESIGN_NODE_VIRTUAL_JOYSTICK ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.channels", 1);
			if ( iRet != XUI_OK ) return iRet;
			bVirtualJoystickChannels = 1;
		} else if ( (!bInventorySlots || !bInventorySelectedSlots || !bInventoryCurrentFallback) && pNode->iType == UI_DESIGN_NODE_INVENTORY_GRID ) {
			if ( !bInventorySlots ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.slots", 1);
				if ( iRet != XUI_OK ) return iRet;
				bInventorySlots = 1;
			}
			if ( !bInventorySelectedSlots ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.selectedSlots", 1);
				if ( iRet != XUI_OK ) return iRet;
				bInventorySelectedSlots = 1;
			}
			if ( !bInventoryCurrentFallback ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.current", "2");
				if ( iRet != XUI_OK ) return iRet;
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.selectedSlots", "");
				if ( iRet != XUI_OK ) return iRet;
				bInventoryCurrentFallback = 1;
			}
		} else if ( !bRadioGroupOptions && pNode->iType == UI_DESIGN_NODE_RADIO_GROUP ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.options", 1);
			if ( iRet != XUI_OK ) return iRet;
			bRadioGroupOptions = 1;
		} else if ( (!bMsgBoxButtonTitles || !bMsgBoxButtons) && pNode->iType == UI_DESIGN_NODE_MSG_BOX ) {
			if ( !bMsgBoxButtonTitles ) {
				(void)uiDesignNodeSetProperty(pNode, "behavior.buttons", "1");
				(void)uiDesignNodeSetProperty(pNode, "data.buttonTitles", "ok|Proceed\ncancel|Abort\nyes|Confirm\nno|Decline");
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.buttonTitles", 1);
				if ( iRet != XUI_OK ) return iRet;
				bMsgBoxButtonTitles = 1;
			}
			if ( !bMsgBoxButtons ) {
				(void)uiDesignNodeSetProperty(pNode, "behavior.buttons", "4");
				(void)uiDesignNodeSetProperty(pNode, "metrics.minMessageHeight", "44");
				(void)uiDesignNodeSetProperty(pNode, "metrics.maxWidthRatio", "0.5");
				(void)uiDesignNodeSetProperty(pNode, "data.customButtons", "Deploy|0|primary\nDelete|2|danger\nCancel|1|default");
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.customButtons", 1);
				if ( iRet != XUI_OK ) return iRet;
				bMsgBoxButtons = 1;
			}
		} else if ( !bMsgTipOffset && pNode->iType == UI_DESIGN_NODE_MSG_TIP ) {
			if ( !__uiDesignInspectorValidate(NULL, 0, "metrics.offsetY", "-72", XUI_TABLE_CELL_TYPE_FLOAT, pApp) ) return XUI_ERROR;
			iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "metrics.offsetY", "-72");
			if ( iRet != XUI_OK ) return iRet;
			bMsgTipOffset = 1;
		} else if ( (!bFileFilter || !bFileRoots || !bFileEntries || !bFileButtonTitles) && pNode->iType == UI_DESIGN_NODE_FILE_DIALOG ) {
			if ( !bFileFilter ) {
				(void)uiDesignNodeSetProperty(pNode, "data.filter", "All Files (*.*)|*.*\nUI Files (*.ui)|*.ui\nScripts (*.c;*.h)|*.c;*.h");
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.filter", 1);
				if ( iRet != XUI_OK ) return iRet;
				bFileFilter = 1;
			}
			if ( !bFileRoots ) {
				(void)uiDesignNodeSetProperty(pNode, "data.roots",
					"Project|D:\\GIT\\xge|true|true\n"
					"Desktop|%USERPROFILE%\\Desktop|false|true\n"
					"Archive|D:\\Archive|false|false");
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.roots", 1);
				if ( iRet != XUI_OK ) return iRet;
				bFileRoots = 1;
			}
			if ( !bFileEntries ) {
				(void)uiDesignNodeSetProperty(pNode, "data.entries",
					"main.c|script|18 KB|Today|true|true\n"
					"assets|folder|Folder|Yesterday|false|true\n"
					"preview.png|image|240 KB|Monday|false|true\n"
					"legacy.tmp|file|1 KB|Old|false|false");
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.entries", 1);
				if ( iRet != XUI_OK ) return iRet;
				bFileEntries = 1;
			}
			if ( !bFileButtonTitles ) {
				(void)uiDesignNodeSetProperty(pNode, "data.buttonTitles",
					"ok|Choose\n"
					"cancel|Dismiss\n"
					"up|Parent\n"
					"refresh|Reload\n"
					"path|Location\n"
					"file|File name\n"
					"type|File type");
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.buttonTitles", 1);
				if ( iRet != XUI_OK ) return iRet;
				bFileButtonTitles = 1;
			}
		} else if ( !bToastQueue && pNode->iType == UI_DESIGN_NODE_TOAST ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.toasts", 1);
			if ( iRet != XUI_OK ) return iRet;
			bToastQueue = 1;
		} else if ( (!bColorPickerPalette || !bColorPickerHex) && pNode->iType == UI_DESIGN_NODE_COLOR_PICKER ) {
			if ( !bColorPickerPalette ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.palette", 0);
				if ( iRet != XUI_OK ) return iRet;
				bColorPickerPalette = 1;
			}
			if ( !bColorPickerHex ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "value.useHex", "true");
				if ( iRet != XUI_OK ) return iRet;
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "value.hex", "#22A8CCB8");
				if ( iRet != XUI_OK ) return iRet;
				bColorPickerHex = 1;
			}
		} else if ( (!bMenuBarItems || !bMenuBarMenus || !bMenuBarSubmenus) && pNode->iType == UI_DESIGN_NODE_MENU_BAR ) {
			if ( !bMenuBarItems ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.items", 1);
				if ( iRet != XUI_OK ) return iRet;
				bMenuBarItems = 1;
			}
			if ( !bMenuBarMenus ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.menus",
					"File|Recent|submenu|true|false|false|false||103|0\n"
					"File|Exit|normal|true|false|false|false||199|0");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.menus", 1);
				if ( iRet != XUI_OK ) return iRet;
				bMenuBarMenus = 1;
			}
			if ( !bMenuBarSubmenus ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.submenus",
					"File|Recent|Project Alpha|normal|true|false|false|false|Ctrl+1|1101|0\n"
					"File|Recent|Project Beta|normal|true|false|false|false|Ctrl+2|1102|0");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.submenus", 1);
				if ( iRet != XUI_OK ) return iRet;
				bMenuBarSubmenus = 1;
			}
		} else if ( !bToolbarItems && pNode->iType == UI_DESIGN_NODE_TOOLBAR ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.items", 1);
			if ( iRet != XUI_OK ) return iRet;
			bToolbarItems = 1;
		} else if ( !bStatusBarItems && pNode->iType == UI_DESIGN_NODE_STATUS_BAR ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.items", 1);
			if ( iRet != XUI_OK ) return iRet;
			bStatusBarItems = 1;
		} else if ( (!bMenuItems || !bMenuSubmenus) && pNode->iType == UI_DESIGN_NODE_MENU ) {
			if ( !bMenuItems ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.items",
					"New|normal|true|false|false|false|Ctrl+N|1|0\n"
					"More|submenu|true|false|false|false||5|0");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.items", 1);
				if ( iRet != XUI_OK ) return iRet;
				bMenuItems = 1;
			}
			if ( !bMenuSubmenus ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.submenus",
					"More|Recent Project|normal|true|false|false|false|Ctrl+R|51|0\n"
					"More|Settings|normal|true|false|false|false||52|0");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.submenus", 1);
				if ( iRet != XUI_OK ) return iRet;
				bMenuSubmenus = 1;
			}
		} else if ( (!bChartSeriesList || !bChartSeries) && pNode->iType == UI_DESIGN_NODE_CHART ) {
			if ( !bChartSeriesList ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.seriesList", 1);
				if ( iRet != XUI_OK ) return iRet;
				bChartSeriesList = 1;
			}
			if ( !bChartSeries ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.series", 1);
				if ( iRet != XUI_OK ) return iRet;
				bChartSeries = 1;
			}
		} else if ( (!bTableViewColumns || !bTableViewRows || !bTableViewRowStyles || !bTableViewStyles || !bTableViewMerges) && pNode->iType == UI_DESIGN_NODE_TABLEVIEW ) {
			if ( !bTableViewColumns ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.columns", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTableViewColumns = 1;
			}
			if ( !bTableViewRows ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.rows", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTableViewRows = 1;
			}
			if ( !bTableViewRowStyles ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.rowStyles", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTableViewRowStyles = 1;
			}
			if ( !bTableViewStyles ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.cellStyles", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTableViewStyles = 1;
			}
			if ( !bTableViewMerges ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.merges", "0|0|1|2|true");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.merges", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTableViewMerges = 1;
			}
		} else if ( (!bTableGridColumns || !bTableGridRows || !bTableGridRowStyles || !bTableGridStyles || !bTableGridMerges || !bTableGridEditorConfigs) && pNode->iType == UI_DESIGN_NODE_TABLEGRID ) {
			if ( !bTableGridColumns ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.columns", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTableGridColumns = 1;
			}
			if ( !bTableGridRows ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.rows", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTableGridRows = 1;
			}
			if ( !bTableGridRowStyles ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.rowStyles", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTableGridRowStyles = 1;
			}
			if ( !bTableGridStyles ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.cellStyles", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTableGridStyles = 1;
			}
			if ( !bTableGridMerges ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.merges", "0|0|1|2|true");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.merges", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTableGridMerges = 1;
			}
			if ( !bTableGridEditorConfigs ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.editorConfigs",
					"-1|1|float||0|1000|1|0\n"
					"-1|0|enum|Small,Medium,Large|||||||||||||||||10,20,30|true,true,false|1|true|20");
				if ( iRet != XUI_OK ) return iRet;
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.editorConfigs", 1);
				if ( iRet != XUI_OK ) return iRet;
				bTableGridEditorConfigs = 1;
			}
		} else if ( (!bMessageListMessages || !bMessageListImportText) && pNode->iType == UI_DESIGN_NODE_MESSAGE_LIST ) {
			if ( !bMessageListMessages ) {
				iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.messages", 1);
				if ( iRet != XUI_OK ) return iRet;
				bMessageListMessages = 1;
			}
			if ( !bMessageListImportText ) {
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.importText",
					"MESSAGELIST1\nN\t1\t0\tm1\tYou\t09:21\tImported message\nN\t2\t0\tm2\tSystem\t09:22\tImported notice\n");
				if ( iRet != XUI_OK ) return iRet;
				iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.sourceMode", "1");
				if ( iRet != XUI_OK ) return iRet;
				bMessageListImportText = 1;
			}
		} else if ( !bPropertyGridProperties && pNode->iType == UI_DESIGN_NODE_PROPERTY_GRID ) {
			iRet = uiDesignAppSetNodeProperty(pApp, pNode->iId, "data.properties",
				"General|Theme|20|enum|Theme selection|10||general.theme|true|Light,Dark,Blue|||||||||||||||||10,20,30|true,true,false|1|true|20");
			if ( iRet != XUI_OK ) return iRet;
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.properties", 1);
			if ( iRet != XUI_OK ) return iRet;
			bPropertyGridProperties = 1;
		} else if ( !bSourceEditor && pNode->iType == UI_DESIGN_NODE_IMAGE ) {
			iRet = __uiDesignInspectorExerciseOneComplex(pApp, pNode, "data.source", 0);
			if ( iRet != XUI_OK ) return iRet;
			bSourceEditor = 1;
		}
	}
	return (bButtonStatePatches && bInlineStyles && bCacheStateIds && bLayoutTableRowsConfig && bLayoutTableColumnsConfig && bInputDecorations && bInputMenuTitles && bTextEditFindPreview &&
		bTree && bComboBoxItems && bListViewItems && bCascader && bBreadcrumb &&
		bTagInputTags && bStepBarSteps && bTimelineLayers && bTimelineFrames && bTimelineSpans && bTimelineSelection && bTimelineMenuTitles && bDock && bDockRegions && bDockMenuTitles && bDockTooltips && bSplit &&
		bTabsItems && bAccordionSections && bCanvasCommands && bPopupContent && bTerminalPalette && bTerminalSelection && bTerminalMenuTitles &&
		bCodeEditMarkers && bCodeEditIndicators && bCodeEditDiagnostics && bCodeEditMargins && bCodeEditSelections && bCodeEditFolds && bCodeEditStyles && bCodeEditTokenStyles && bCodeEditMenuTitles && bCodeEditFindPreview &&
		bFlowNodes && bFlowPorts && bFlowNodeConfigs && bFlowEdges && bFlowDiagnostics &&
		bWorkflowNodeTypes && bWorkflowConfigFields && bWorkflowTypePorts && bWorkflowNodes && bWorkflowNodeConfigs && bWorkflowEdges &&
		bWorkflowVariables && bWorkflowDiagnostics && bVirtualJoystickChannels &&
		bInventorySlots && bInventorySelectedSlots && bInventoryCurrentFallback && bRadioGroupOptions &&
		bMsgBoxButtonTitles && bMsgBoxButtons && bMsgTipOffset && bFileFilter && bFileRoots && bFileEntries && bFileButtonTitles && bToastQueue && bColorPickerPalette && bColorPickerHex &&
		bMenuBarItems && bMenuBarMenus && bMenuBarSubmenus && bToolbarItems && bStatusBarItems && bMenuItems && bMenuSubmenus &&
		bChartSeriesList && bChartSeries && bTableViewColumns && bTableViewRows && bTableViewRowStyles && bTableViewStyles && bTableViewMerges &&
		bTableGridColumns && bTableGridRows && bTableGridRowStyles && bTableGridStyles && bTableGridMerges && bTableGridEditorConfigs &&
		bMessageListMessages && bMessageListImportText && bPropertyGridProperties && bSourceEditor) ? XUI_OK : XUI_ERROR;
}

static int __uiDesignInspectorExpectedPropertyCount(const ui_design_node_t* pNode)
{
	const ui_design_control_desc_t* pDesc;
	int iCount;

	if ( pNode == NULL ) return 0;
	pDesc = uiDesignRegistryFind(pNode->iType);
	iCount = 2 + 4 + 3;
	iCount += UI_DESIGN_COUNT_OF(g_arrCommonWidgetProperties);
	iCount += UI_DESIGN_COUNT_OF(g_arrLayoutProperties);
	if ( __uiDesignInspectorNodeSupportsFont(pNode->iType) ) {
		iCount += UI_DESIGN_COUNT_OF(g_arrFontProperties);
	}
	if ( (pNode->iType == UI_DESIGN_NODE_CHECKBOX) || (pNode->iType == UI_DESIGN_NODE_RADIO) ) {
		iCount += 1;
	}
	if ( pDesc != NULL ) {
		iCount += pDesc->iPropertyCount;
	}
	return iCount;
}

int uiDesignInspectorExercisePropertyCoverage(ui_design_app_t* pApp)
{
	ui_design_node_t* pNode;
	int iOldSelected;
	int iExpected;
	int iActual;
	int iRet;
	int i;

	if ( (pApp == NULL) || (pApp->pPropertyGrid == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iOldSelected = pApp->tModel.iSelectedId;
	for ( i = 0; i < pApp->tModel.iNodeCount; ++i ) {
		pNode = &pApp->tModel.arrNodes[i];
		if ( uiDesignRegistryFind(pNode->iType) == NULL ) continue;
		(void)uiDesignModelSetSelected(&pApp->tModel, pNode->iId);
		iRet = uiDesignInspectorRefresh(pApp);
		if ( iRet != XUI_OK ) return iRet;
		iExpected = __uiDesignInspectorExpectedPropertyCount(pNode);
		iActual = xuiPropertyGridGetPropertyCount(pApp->pPropertyGrid);
		if ( iActual < iExpected ) {
			printf("xui_uidesign exercise-property-coverage-failed type=%s id=%d actual=%d expected=%d capacity=%d\n",
				uiDesignNodeTypeName(pNode->iType), pNode->iId, iActual, iExpected, XUI_PROPERTY_GRID_PROPERTY_CAPACITY);
			(void)uiDesignModelSetSelected(&pApp->tModel, iOldSelected);
			(void)uiDesignInspectorRefresh(pApp);
			return XUI_ERROR;
		}
	}
	(void)uiDesignModelSetSelected(&pApp->tModel, iOldSelected);
	return uiDesignInspectorRefresh(pApp);
}

int uiDesignInspectorRefresh(ui_design_app_t* pApp)
{
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __uiDesignInspectorRefreshTree(pApp);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignInspectorRefreshProperties(pApp);
	if ( iRet != XUI_OK ) return iRet;
	if ( pApp->pInspector != NULL ) {
		(void)xuiWidgetInvalidate(pApp->pInspector, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_OK;
}

int uiDesignInspectorCreate(ui_design_app_t* pApp)
{
	xui_tree_view_desc_t tTreeDesc;
	xui_property_grid_desc_t tPropDesc;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiWidgetCreate(pApp->pContext, &pApp->pInspector);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pApp->pInspector, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetLayoutCallbacks(pApp->pInspector, __uiDesignInspectorMeasure, __uiDesignInspectorArrange, pApp);
	memset(&tTreeDesc, 0, sizeof(tTreeDesc));
	tTreeDesc.iSize = sizeof(tTreeDesc);
	tTreeDesc.pFont = pApp->pFont;
	tTreeDesc.fItemHeight = 23.0f;
	tTreeDesc.fIndent = 18.0f;
	tTreeDesc.fPadding = 8.0f;
	tTreeDesc.fBorderWidth = 1.0f;
	tTreeDesc.iSelectedId = 0;
	tTreeDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	tTreeDesc.iBackgroundColor = XUI_COLOR_RGBA(248, 251, 255, 255);
	tTreeDesc.iBorderColor = XUI_COLOR_RGBA(172, 192, 214, 255);
	tTreeDesc.iSelectedColor = XUI_COLOR_RGBA(49, 126, 214, 255);
	tTreeDesc.iTextColor = XUI_COLOR_RGBA(35, 50, 70, 255);
	iRet = xuiTreeViewCreate(pApp->pContext, &pApp->pTree, &tTreeDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiTreeViewSetSelect(pApp->pTree, __uiDesignInspectorSelectTree, pApp);
	memset(&tPropDesc, 0, sizeof(tPropDesc));
	tPropDesc.iSize = sizeof(tPropDesc);
	tPropDesc.pFont = pApp->pFont;
	tPropDesc.fNameWidth = 96.0f;
	tPropDesc.fRowHeight = 24.0f;
	tPropDesc.fCategoryHeight = 24.0f;
	tPropDesc.iDescriptionMode = XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP;
	tPropDesc.iEditMode = XUI_TABLE_GRID_EDIT_QUICK;
	tPropDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	iRet = xuiPropertyGridCreate(pApp->pContext, &pApp->pPropertyGrid, &tPropDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiPropertyGridSetValidate(pApp->pPropertyGrid, __uiDesignInspectorValidate, pApp);
	(void)xuiPropertyGridSetChange(pApp->pPropertyGrid, __uiDesignInspectorChange, pApp);
	iRet = xuiWidgetAddChild(pApp->pInspector, pApp->pTree);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pApp->pInspector, pApp->pPropertyGrid);
	if ( iRet != XUI_OK ) return iRet;
	return uiDesignInspectorRefresh(pApp);
}
