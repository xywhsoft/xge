#include "ui_design_model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int __uiDesignRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX <= tRect.fX + tRect.fW) && (fY <= tRect.fY + tRect.fH);
}

static void __uiDesignCopyText(char* sDst, int iCapacity, const char* sSrc)
{
	if ( (sDst == NULL) || (iCapacity <= 0) ) return;
	if ( sSrc == NULL ) sSrc = "";
	snprintf(sDst, (size_t)iCapacity, "%s", sSrc);
}

void uiDesignModelInit(ui_design_model_t* pModel)
{
	if ( pModel == NULL ) return;
	memset(pModel, 0, sizeof(*pModel));
	pModel->iNextId = 1;
	pModel->iSelectedId = 0;
	pModel->iSelectedCount = 0;
	pModel->iRevision = 1u;
}

const char* uiDesignNodeTypeName(ui_design_node_type_t iType)
{
	switch ( iType ) {
	case UI_DESIGN_NODE_WIDGET: return "widget";
	case UI_DESIGN_NODE_LABEL: return "label";
	case UI_DESIGN_NODE_HYPERLINK: return "hyperlink";
	case UI_DESIGN_NODE_BUTTON: return "button";
	case UI_DESIGN_NODE_CHECKBOX: return "checkbox";
	case UI_DESIGN_NODE_RADIO: return "radio";
	case UI_DESIGN_NODE_TOGGLE: return "toggle";
	case UI_DESIGN_NODE_INPUT: return "input";
	case UI_DESIGN_NODE_TAG_INPUT: return "tag_input";
	case UI_DESIGN_NODE_NUMERIC_INPUT: return "numeric_input";
	case UI_DESIGN_NODE_TEXT_EDIT: return "text_edit";
	case UI_DESIGN_NODE_PANEL: return "panel";
	case UI_DESIGN_NODE_SEPARATOR: return "separator";
	case UI_DESIGN_NODE_PROGRESS: return "progress";
	case UI_DESIGN_NODE_STEP_BAR: return "step_bar";
	case UI_DESIGN_NODE_CHART: return "chart";
	case UI_DESIGN_NODE_SCROLLBAR: return "scrollbar";
	case UI_DESIGN_NODE_SLIDER: return "slider";
	case UI_DESIGN_NODE_RANGE_SLIDER: return "range_slider";
	case UI_DESIGN_NODE_PAGE: return "page";
	case UI_DESIGN_NODE_CAROUSEL: return "carousel";
	case UI_DESIGN_NODE_COMBOBOX: return "combobox";
	case UI_DESIGN_NODE_LISTVIEW: return "listview";
	case UI_DESIGN_NODE_TREEVIEW: return "treeview";
	case UI_DESIGN_NODE_TABLEVIEW: return "tableview";
	case UI_DESIGN_NODE_TABLEGRID: return "tablegrid";
	case UI_DESIGN_NODE_QRCODE: return "qrcode";
	case UI_DESIGN_NODE_IMAGE: return "image";
	case UI_DESIGN_NODE_BREADCRUMB: return "breadcrumb";
	case UI_DESIGN_NODE_CHECK_CARD: return "check_card";
	case UI_DESIGN_NODE_RADIO_GROUP: return "radio_group";
	case UI_DESIGN_NODE_VIRTUAL_JOYSTICK: return "virtual_joystick";
	case UI_DESIGN_NODE_INVENTORY_GRID: return "inventory_grid";
	case UI_DESIGN_NODE_TERMINAL: return "terminal";
	case UI_DESIGN_NODE_SPLIT_LAYOUT: return "split_layout";
	case UI_DESIGN_NODE_TABS: return "tabs";
	case UI_DESIGN_NODE_ACCORDION: return "accordion";
	case UI_DESIGN_NODE_WINDOW: return "window";
	case UI_DESIGN_NODE_SCROLL_FRAME: return "scroll_frame";
	case UI_DESIGN_NODE_SCROLL_VIEW: return "scroll_view";
	case UI_DESIGN_NODE_CANVAS: return "canvas";
	case UI_DESIGN_NODE_MESSAGE_LIST: return "message_list";
	case UI_DESIGN_NODE_TIMELINE_VIEW: return "timeline_view";
	case UI_DESIGN_NODE_PROPERTY_GRID: return "property_grid";
	case UI_DESIGN_NODE_MENU_BAR: return "menubar";
	case UI_DESIGN_NODE_TOOLBAR: return "toolbar";
	case UI_DESIGN_NODE_STATUS_BAR: return "statusbar";
	case UI_DESIGN_NODE_DOCK_PANEL: return "dock_panel";
	case UI_DESIGN_NODE_POPUP: return "popup";
	case UI_DESIGN_NODE_MENU: return "menu";
	case UI_DESIGN_NODE_MSG_BOX: return "msgbox";
	case UI_DESIGN_NODE_FILE_DIALOG: return "file_dialog";
	case UI_DESIGN_NODE_MSG_TIP: return "msgtip";
	case UI_DESIGN_NODE_TOAST: return "toast";
	case UI_DESIGN_NODE_CASCADER: return "cascader";
	case UI_DESIGN_NODE_COLOR_PICKER: return "color_picker";
	case UI_DESIGN_NODE_DATE_PICKER: return "date_picker";
	case UI_DESIGN_NODE_CODE_EDIT: return "code_edit";
	case UI_DESIGN_NODE_FLOW_GRAPH: return "flow_graph";
	case UI_DESIGN_NODE_WORKFLOW: return "workflow";
	default: return "none";
	}
}

int uiDesignNodeTypeIsContainer(ui_design_node_type_t iType)
{
	return (iType == UI_DESIGN_NODE_WIDGET) ||
		(iType == UI_DESIGN_NODE_PANEL) ||
		(iType == UI_DESIGN_NODE_CHECK_CARD) ||
		(iType == UI_DESIGN_NODE_RADIO_GROUP) ||
		(iType == UI_DESIGN_NODE_CAROUSEL) ||
		(iType == UI_DESIGN_NODE_SPLIT_LAYOUT) ||
		(iType == UI_DESIGN_NODE_TABS) ||
		(iType == UI_DESIGN_NODE_ACCORDION) ||
		(iType == UI_DESIGN_NODE_DOCK_PANEL) ||
		(iType == UI_DESIGN_NODE_WINDOW) ||
		(iType == UI_DESIGN_NODE_SCROLL_FRAME) ||
		(iType == UI_DESIGN_NODE_SCROLL_VIEW) ||
		(iType == UI_DESIGN_NODE_POPUP);
}

int uiDesignModelCanFreeTransformNode(const ui_design_model_t* pModel, const ui_design_node_t* pNode)
{
	const ui_design_node_t* pParent;
	int iLayoutType;
	int iChildFlowMode;

	if ( (pModel == NULL) || (pNode == NULL) ) return 0;
	if ( pNode->iParentId == 0 ) return 1;
	pParent = uiDesignModelGetNodeConst(pModel, pNode->iParentId);
	if ( pParent == NULL ) return 1;
	switch ( pParent->iType ) {
	case UI_DESIGN_NODE_CAROUSEL:
	case UI_DESIGN_NODE_SPLIT_LAYOUT:
	case UI_DESIGN_NODE_ACCORDION:
	case UI_DESIGN_NODE_RADIO_GROUP:
		return 0;
	default:
		break;
	}
	iLayoutType = uiDesignNodeGetPropertyInt(pParent, "layout.type", XUI_LAYOUT_MANUAL);
	if ( iLayoutType == XUI_LAYOUT_MANUAL ) return 1;
	if ( iLayoutType == XUI_LAYOUT_OVERLAY ) {
		iChildFlowMode = uiDesignNodeGetPropertyInt(pNode, "layout.flowMode", XUI_FLOW_BLOCK);
		return iChildFlowMode == XUI_FLOW_ABSOLUTE;
	}
	return 0;
}

void uiDesignNodeTypeDefaultSize(ui_design_node_type_t iType, float* pW, float* pH)
{
	float fW;
	float fH;

	switch ( iType ) {
	case UI_DESIGN_NODE_WIDGET:
		fW = 180.0f;
		fH = 116.0f;
		break;
	case UI_DESIGN_NODE_LABEL:
		fW = 120.0f;
		fH = 26.0f;
		break;
	case UI_DESIGN_NODE_HYPERLINK:
		fW = 140.0f;
		fH = 26.0f;
		break;
	case UI_DESIGN_NODE_BUTTON:
		fW = 118.0f;
		fH = 32.0f;
		break;
	case UI_DESIGN_NODE_CHECKBOX:
		fW = 148.0f;
		fH = 28.0f;
		break;
	case UI_DESIGN_NODE_RADIO:
		fW = 132.0f;
		fH = 28.0f;
		break;
	case UI_DESIGN_NODE_TOGGLE:
		fW = 148.0f;
		fH = 30.0f;
		break;
	case UI_DESIGN_NODE_INPUT:
		fW = 180.0f;
		fH = 30.0f;
		break;
	case UI_DESIGN_NODE_TAG_INPUT:
		fW = 220.0f;
		fH = 34.0f;
		break;
	case UI_DESIGN_NODE_NUMERIC_INPUT:
		fW = 150.0f;
		fH = 30.0f;
		break;
	case UI_DESIGN_NODE_TEXT_EDIT:
		fW = 280.0f;
		fH = 160.0f;
		break;
	case UI_DESIGN_NODE_PANEL:
		fW = 260.0f;
		fH = 160.0f;
		break;
	case UI_DESIGN_NODE_SEPARATOR:
		fW = 180.0f;
		fH = 8.0f;
		break;
	case UI_DESIGN_NODE_PROGRESS:
		fW = 180.0f;
		fH = 24.0f;
		break;
	case UI_DESIGN_NODE_STEP_BAR:
		fW = 300.0f;
		fH = 54.0f;
		break;
	case UI_DESIGN_NODE_CHART:
		fW = 320.0f;
		fH = 210.0f;
		break;
	case UI_DESIGN_NODE_SCROLLBAR:
		fW = 180.0f;
		fH = 18.0f;
		break;
	case UI_DESIGN_NODE_SLIDER:
		fW = 180.0f;
		fH = 28.0f;
		break;
	case UI_DESIGN_NODE_RANGE_SLIDER:
		fW = 190.0f;
		fH = 30.0f;
		break;
	case UI_DESIGN_NODE_PAGE:
		fW = 320.0f;
		fH = 34.0f;
		break;
	case UI_DESIGN_NODE_CAROUSEL:
		fW = 260.0f;
		fH = 150.0f;
		break;
	case UI_DESIGN_NODE_COMBOBOX:
		fW = 180.0f;
		fH = 30.0f;
		break;
	case UI_DESIGN_NODE_LISTVIEW:
		fW = 220.0f;
		fH = 150.0f;
		break;
	case UI_DESIGN_NODE_TREEVIEW:
		fW = 220.0f;
		fH = 160.0f;
		break;
	case UI_DESIGN_NODE_TABLEVIEW:
		fW = 320.0f;
		fH = 180.0f;
		break;
	case UI_DESIGN_NODE_TABLEGRID:
		fW = 320.0f;
		fH = 180.0f;
		break;
	case UI_DESIGN_NODE_QRCODE:
		fW = 140.0f;
		fH = 140.0f;
		break;
	case UI_DESIGN_NODE_IMAGE:
		fW = 160.0f;
		fH = 100.0f;
		break;
	case UI_DESIGN_NODE_BREADCRUMB:
		fW = 260.0f;
		fH = 32.0f;
		break;
	case UI_DESIGN_NODE_CHECK_CARD:
		fW = 168.0f;
		fH = 88.0f;
		break;
	case UI_DESIGN_NODE_RADIO_GROUP:
		fW = 180.0f;
		fH = 96.0f;
		break;
	case UI_DESIGN_NODE_VIRTUAL_JOYSTICK:
		fW = 132.0f;
		fH = 132.0f;
		break;
	case UI_DESIGN_NODE_INVENTORY_GRID:
		fW = 264.0f;
		fH = 196.0f;
		break;
	case UI_DESIGN_NODE_TERMINAL:
		fW = 340.0f;
		fH = 200.0f;
		break;
	case UI_DESIGN_NODE_SPLIT_LAYOUT:
		fW = 300.0f;
		fH = 180.0f;
		break;
	case UI_DESIGN_NODE_TABS:
		fW = 300.0f;
		fH = 180.0f;
		break;
	case UI_DESIGN_NODE_ACCORDION:
		fW = 280.0f;
		fH = 190.0f;
		break;
	case UI_DESIGN_NODE_WINDOW:
		fW = 280.0f;
		fH = 190.0f;
		break;
	case UI_DESIGN_NODE_SCROLL_FRAME:
	case UI_DESIGN_NODE_SCROLL_VIEW:
	case UI_DESIGN_NODE_CANVAS:
		fW = 260.0f;
		fH = 170.0f;
		break;
	case UI_DESIGN_NODE_MESSAGE_LIST:
		fW = 300.0f;
		fH = 220.0f;
		break;
	case UI_DESIGN_NODE_TIMELINE_VIEW:
		fW = 360.0f;
		fH = 220.0f;
		break;
	case UI_DESIGN_NODE_PROPERTY_GRID:
		fW = 300.0f;
		fH = 220.0f;
		break;
	case UI_DESIGN_NODE_MENU_BAR:
		fW = 340.0f;
		fH = 30.0f;
		break;
	case UI_DESIGN_NODE_TOOLBAR:
		fW = 340.0f;
		fH = 38.0f;
		break;
	case UI_DESIGN_NODE_STATUS_BAR:
		fW = 360.0f;
		fH = 28.0f;
		break;
	case UI_DESIGN_NODE_DOCK_PANEL:
		fW = 420.0f;
		fH = 260.0f;
		break;
	case UI_DESIGN_NODE_POPUP:
		fW = 260.0f;
		fH = 180.0f;
		break;
	case UI_DESIGN_NODE_MENU:
		fW = 210.0f;
		fH = 132.0f;
		break;
	case UI_DESIGN_NODE_MSG_BOX:
		fW = 320.0f;
		fH = 190.0f;
		break;
	case UI_DESIGN_NODE_FILE_DIALOG:
		fW = 380.0f;
		fH = 240.0f;
		break;
	case UI_DESIGN_NODE_MSG_TIP:
		fW = 220.0f;
		fH = 48.0f;
		break;
	case UI_DESIGN_NODE_TOAST:
		fW = 300.0f;
		fH = 86.0f;
		break;
	case UI_DESIGN_NODE_CASCADER:
	case UI_DESIGN_NODE_COLOR_PICKER:
	case UI_DESIGN_NODE_DATE_PICKER:
		fW = 190.0f;
		fH = 30.0f;
		break;
	case UI_DESIGN_NODE_CODE_EDIT:
		fW = 360.0f;
		fH = 220.0f;
		break;
	case UI_DESIGN_NODE_FLOW_GRAPH:
		fW = 420.0f;
		fH = 260.0f;
		break;
	case UI_DESIGN_NODE_WORKFLOW:
		fW = 460.0f;
		fH = 280.0f;
		break;
	default:
		fW = 80.0f;
		fH = 24.0f;
		break;
	}
	if ( pW != NULL ) *pW = fW;
	if ( pH != NULL ) *pH = fH;
}

ui_design_node_t* uiDesignModelGetNode(ui_design_model_t* pModel, int iId)
{
	int i;

	if ( (pModel == NULL) || (iId <= 0) ) return NULL;
	for ( i = 0; i < pModel->iNodeCount; i++ ) {
		if ( pModel->arrNodes[i].iId == iId ) return &pModel->arrNodes[i];
	}
	return NULL;
}

const ui_design_node_t* uiDesignModelGetNodeConst(const ui_design_model_t* pModel, int iId)
{
	int i;

	if ( (pModel == NULL) || (iId <= 0) ) return NULL;
	for ( i = 0; i < pModel->iNodeCount; i++ ) {
		if ( pModel->arrNodes[i].iId == iId ) return &pModel->arrNodes[i];
	}
	return NULL;
}

ui_design_node_t* uiDesignModelGetSelected(ui_design_model_t* pModel)
{
	if ( pModel == NULL ) return NULL;
	return uiDesignModelGetNode(pModel, pModel->iSelectedId);
}

static int __uiDesignDefaultParentValid(const ui_design_model_t* pModel, int iParentId)
{
	const ui_design_node_t* pParent;

	if ( iParentId == 0 ) return 1;
	pParent = uiDesignModelGetNodeConst(pModel, iParentId);
	return (pParent != NULL) && uiDesignNodeTypeIsContainer(pParent->iType);
}

int uiDesignModelAddNode(ui_design_model_t* pModel, ui_design_node_type_t iType, int iParentId, float fX, float fY, int* pId)
{
	ui_design_node_t* pNode;
	float fW;
	float fH;
	int iId;

	if ( (pModel == NULL) || (iType <= UI_DESIGN_NODE_NONE) || (iType > UI_DESIGN_NODE_LAST) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pModel->iNodeCount >= UI_DESIGN_MAX_NODES ) return XUI_ERROR_OUT_OF_MEMORY;
	if ( !__uiDesignDefaultParentValid(pModel, iParentId) ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode = &pModel->arrNodes[pModel->iNodeCount++];
	memset(pNode, 0, sizeof(*pNode));
	iId = pModel->iNextId++;
	pNode->iId = iId;
	pNode->iType = iType;
	pNode->iParentId = iParentId;
	uiDesignNodeTypeDefaultSize(iType, &fW, &fH);
	pNode->tRect = (xui_rect_t){fX, fY, fW, fH};
	pNode->bVisible = 1;
	pNode->bEnabled = 1;
	if ( iType == UI_DESIGN_NODE_WIDGET ) {
		snprintf(pNode->sText, sizeof(pNode->sText), "widget%d", iId);
	} else {
		snprintf(pNode->sText, sizeof(pNode->sText), "%s%d", uiDesignNodeTypeName(iType), iId);
	}
	if ( iType == UI_DESIGN_NODE_RADIO ) pNode->bChecked = 1;
	(void)uiDesignModelSetSelected(pModel, iId);
	if ( pId != NULL ) *pId = iId;
	return XUI_OK;
}

int uiDesignModelSetSelected(ui_design_model_t* pModel, int iId)
{
	if ( pModel == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (iId != 0) && (uiDesignModelGetNode(pModel, iId) == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iId == 0 ) {
		if ( pModel->iSelectedId != 0 || pModel->iSelectedCount != 0 ) {
			pModel->iSelectedId = 0;
			pModel->iSelectedCount = 0;
			pModel->iRevision++;
		}
		return XUI_OK;
	}
	if ( pModel->iSelectedId != iId || pModel->iSelectedCount != 1 || pModel->arrSelectedIds[0] != iId ) {
		pModel->iSelectedId = iId;
		pModel->arrSelectedIds[0] = iId;
		pModel->iSelectedCount = 1;
		pModel->iRevision++;
	}
	return XUI_OK;
}

int uiDesignModelClearSelection(ui_design_model_t* pModel)
{
	if ( pModel == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return uiDesignModelSetSelected(pModel, 0);
}

int uiDesignModelIsSelected(const ui_design_model_t* pModel, int iId)
{
	int i;

	if ( (pModel == NULL) || (iId <= 0) ) return 0;
	for ( i = 0; i < pModel->iSelectedCount; ++i ) {
		if ( pModel->arrSelectedIds[i] == iId ) return 1;
	}
	return 0;
}

int uiDesignModelAddSelection(ui_design_model_t* pModel, int iId)
{
	if ( pModel == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iId <= 0 ) return uiDesignModelClearSelection(pModel);
	if ( uiDesignModelGetNode(pModel, iId) == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( uiDesignModelIsSelected(pModel, iId) ) {
		if ( pModel->iSelectedId != iId ) {
			pModel->iSelectedId = iId;
			pModel->iRevision++;
		}
		return XUI_OK;
	}
	if ( pModel->iSelectedCount >= UI_DESIGN_MAX_NODES ) return XUI_ERROR_OUT_OF_MEMORY;
	pModel->arrSelectedIds[pModel->iSelectedCount++] = iId;
	pModel->iSelectedId = iId;
	pModel->iRevision++;
	return XUI_OK;
}

int uiDesignModelToggleSelection(ui_design_model_t* pModel, int iId)
{
	int i;
	int j;

	if ( pModel == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iId <= 0 ) return uiDesignModelClearSelection(pModel);
	if ( uiDesignModelGetNode(pModel, iId) == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pModel->iSelectedCount; ++i ) {
		if ( pModel->arrSelectedIds[i] != iId ) continue;
		for ( j = i; j + 1 < pModel->iSelectedCount; ++j ) {
			pModel->arrSelectedIds[j] = pModel->arrSelectedIds[j + 1];
		}
		pModel->iSelectedCount--;
		if ( pModel->iSelectedCount > 0 ) {
			if ( pModel->iSelectedId == iId ) pModel->iSelectedId = pModel->arrSelectedIds[pModel->iSelectedCount - 1];
		} else {
			pModel->iSelectedId = 0;
		}
		pModel->iRevision++;
		return XUI_OK;
	}
	return uiDesignModelAddSelection(pModel, iId);
}

int uiDesignModelGetSelectionCount(const ui_design_model_t* pModel)
{
	return (pModel != NULL) ? pModel->iSelectedCount : 0;
}

int uiDesignModelGetSelectionId(const ui_design_model_t* pModel, int iIndex)
{
	if ( (pModel == NULL) || (iIndex < 0) || (iIndex >= pModel->iSelectedCount) ) return 0;
	return pModel->arrSelectedIds[iIndex];
}

int uiDesignModelGetSelectionBounds(const ui_design_model_t* pModel, xui_rect_t* pRect)
{
	xui_rect_t tRect;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
	int i;
	int iId;
	int bHas;

	if ( (pModel == NULL) || (pRect == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	fLeft = fTop = fRight = fBottom = 0.0f;
	bHas = 0;
	for ( i = 0; i < pModel->iSelectedCount; ++i ) {
		iId = pModel->arrSelectedIds[i];
		if ( uiDesignModelGetAbsoluteRect(pModel, iId, &tRect) != XUI_OK ) continue;
		if ( !bHas ) {
			fLeft = tRect.fX;
			fTop = tRect.fY;
			fRight = tRect.fX + tRect.fW;
			fBottom = tRect.fY + tRect.fH;
			bHas = 1;
		} else {
			if ( tRect.fX < fLeft ) fLeft = tRect.fX;
			if ( tRect.fY < fTop ) fTop = tRect.fY;
			if ( tRect.fX + tRect.fW > fRight ) fRight = tRect.fX + tRect.fW;
			if ( tRect.fY + tRect.fH > fBottom ) fBottom = tRect.fY + tRect.fH;
		}
	}
	if ( !bHas ) return XUI_ERROR;
	*pRect = (xui_rect_t){fLeft, fTop, fRight - fLeft, fBottom - fTop};
	return XUI_OK;
}

int uiDesignModelSetText(ui_design_model_t* pModel, int iId, const char* sText)
{
	ui_design_node_t* pNode;

	pNode = uiDesignModelGetNode(pModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignCopyText(pNode->sText, UI_DESIGN_TEXT_CAPACITY, sText);
	pModel->iRevision++;
	return XUI_OK;
}

int uiDesignModelSetRect(ui_design_model_t* pModel, int iId, xui_rect_t tRect)
{
	ui_design_node_t* pNode;

	pNode = uiDesignModelGetNode(pModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( tRect.fW < 8.0f ) tRect.fW = 8.0f;
	if ( tRect.fH < 8.0f ) tRect.fH = 8.0f;
	pNode->tRect = tRect;
	pModel->iRevision++;
	return XUI_OK;
}

int uiDesignModelSetChecked(ui_design_model_t* pModel, int iId, int bChecked)
{
	ui_design_node_t* pNode;

	pNode = uiDesignModelGetNode(pModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pNode->iType != UI_DESIGN_NODE_CHECKBOX) && (pNode->iType != UI_DESIGN_NODE_RADIO) && (pNode->iType != UI_DESIGN_NODE_TOGGLE) && (pNode->iType != UI_DESIGN_NODE_CHECK_CARD) ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode->bChecked = bChecked ? 1 : 0;
	pModel->iRevision++;
	return XUI_OK;
}

int uiDesignModelSetVisible(ui_design_model_t* pModel, int iId, int bVisible)
{
	ui_design_node_t* pNode;

	pNode = uiDesignModelGetNode(pModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode->bVisible = bVisible ? 1 : 0;
	pModel->iRevision++;
	return XUI_OK;
}

int uiDesignModelSetEnabled(ui_design_model_t* pModel, int iId, int bEnabled)
{
	ui_design_node_t* pNode;

	pNode = uiDesignModelGetNode(pModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode->bEnabled = bEnabled ? 1 : 0;
	pModel->iRevision++;
	return XUI_OK;
}

static int __uiDesignNodeFindProperty(const ui_design_node_t* pNode, const char* sPropertyId)
{
	int i;

	if ( (pNode == NULL) || (sPropertyId == NULL) || (sPropertyId[0] == '\0') ) return -1;
	for ( i = 0; i < pNode->iPropertyCount; i++ ) {
		if ( strcmp(pNode->arrProperties[i].sId, sPropertyId) == 0 ) return i;
	}
	return -1;
}

int uiDesignNodeSetProperty(ui_design_node_t* pNode, const char* sPropertyId, const char* sValue)
{
	ui_design_property_value_t* pProp;
	int iIndex;

	if ( (pNode == NULL) || (sPropertyId == NULL) || (sPropertyId[0] == '\0') ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __uiDesignNodeFindProperty(pNode, sPropertyId);
	if ( iIndex < 0 ) {
		if ( pNode->iPropertyCount >= UI_DESIGN_MAX_NODE_PROPERTIES ) return XUI_ERROR_OUT_OF_MEMORY;
		iIndex = pNode->iPropertyCount++;
	}
	pProp = &pNode->arrProperties[iIndex];
	__uiDesignCopyText(pProp->sId, UI_DESIGN_PROPERTY_ID_CAPACITY, sPropertyId);
	__uiDesignCopyText(pProp->sValue, UI_DESIGN_PROPERTY_VALUE_CAPACITY, sValue);
	return XUI_OK;
}

int uiDesignModelSetProperty(ui_design_model_t* pModel, int iId, const char* sPropertyId, const char* sValue)
{
	ui_design_node_t* pNode;
	int iRet;

	pNode = uiDesignModelGetNode(pModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = uiDesignNodeSetProperty(pNode, sPropertyId, sValue);
	if ( iRet == XUI_OK ) pModel->iRevision++;
	return iRet;
}

const char* uiDesignNodeGetProperty(const ui_design_node_t* pNode, const char* sPropertyId, const char* sDefaultValue)
{
	int iIndex;

	iIndex = __uiDesignNodeFindProperty(pNode, sPropertyId);
	if ( iIndex < 0 ) return (sDefaultValue != NULL) ? sDefaultValue : "";
	return pNode->arrProperties[iIndex].sValue;
}

static int __uiDesignTextNoCaseEqual(const char* sA, const char* sB)
{
	unsigned char a;
	unsigned char b;

	if ( (sA == NULL) || (sB == NULL) ) return 0;
	while ( *sA != '\0' && *sB != '\0' ) {
		a = (unsigned char)*sA++;
		b = (unsigned char)*sB++;
		if ( a >= 'A' && a <= 'Z' ) a = (unsigned char)(a - 'A' + 'a');
		if ( b >= 'A' && b <= 'Z' ) b = (unsigned char)(b - 'A' + 'a');
		if ( a != b ) return 0;
	}
	return *sA == '\0' && *sB == '\0';
}

int uiDesignNodeGetPropertyBool(const ui_design_node_t* pNode, const char* sPropertyId, int bDefaultValue)
{
	const char* sValue;

	sValue = uiDesignNodeGetProperty(pNode, sPropertyId, NULL);
	if ( sValue == NULL || sValue[0] == '\0' ) return bDefaultValue ? 1 : 0;
	return (strcmp(sValue, "1") == 0) ||
	       __uiDesignTextNoCaseEqual(sValue, "true") ||
	       __uiDesignTextNoCaseEqual(sValue, "yes") ||
	       __uiDesignTextNoCaseEqual(sValue, "on") ||
	       __uiDesignTextNoCaseEqual(sValue, "checked") ||
	       __uiDesignTextNoCaseEqual(sValue, "enabled");
}

int uiDesignNodeGetPropertyInt(const ui_design_node_t* pNode, const char* sPropertyId, int iDefaultValue)
{
	const char* sValue;

	sValue = uiDesignNodeGetProperty(pNode, sPropertyId, NULL);
	if ( sValue == NULL || sValue[0] == '\0' ) return iDefaultValue;
	return atoi(sValue);
}

float uiDesignNodeGetPropertyFloat(const ui_design_node_t* pNode, const char* sPropertyId, float fDefaultValue)
{
	const char* sValue;

	sValue = uiDesignNodeGetProperty(pNode, sPropertyId, NULL);
	if ( sValue == NULL || sValue[0] == '\0' ) return fDefaultValue;
	return (float)atof(sValue);
}

static int __uiDesignHexNibble(int c)
{
	if ( c >= '0' && c <= '9' ) return c - '0';
	if ( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
	if ( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
	return -1;
}

uint32_t uiDesignNodeGetPropertyColor(const ui_design_node_t* pNode, const char* sPropertyId, uint32_t iDefaultValue)
{
	const char* sValue;
	uint32_t iValue;
	int iLen;
	int i;
	int v;

	sValue = uiDesignNodeGetProperty(pNode, sPropertyId, NULL);
	if ( sValue == NULL || sValue[0] == '\0' ) return iDefaultValue;
	while ( *sValue == ' ' || *sValue == '\t' ) sValue++;
	if ( *sValue == '#' ) sValue++;
	iLen = 0;
	while ( sValue[iLen] != '\0' && sValue[iLen] != ' ' && sValue[iLen] != '\t' ) iLen++;
	if ( iLen != 6 && iLen != 8 ) return iDefaultValue;
	iValue = 0u;
	for ( i = 0; i < iLen; i++ ) {
		v = __uiDesignHexNibble((unsigned char)sValue[i]);
		if ( v < 0 ) return iDefaultValue;
		iValue = (iValue << 4) | (uint32_t)v;
	}
	if ( iLen == 6 ) {
		return XUI_COLOR_RGBA((iValue >> 16) & 0xffu, (iValue >> 8) & 0xffu, iValue & 0xffu, 255);
	}
	return XUI_COLOR_RGBA((iValue >> 24) & 0xffu, (iValue >> 16) & 0xffu, (iValue >> 8) & 0xffu, iValue & 0xffu);
}

static float __uiDesignMaxFloat(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static float __uiDesignClampRange(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) return fMin;
	if ( fValue > fMax ) return fMax;
	return fValue;
}

static int __uiDesignClampIntRange(int iValue, int iMin, int iMax)
{
	if ( iValue < iMin ) return iMin;
	if ( iValue > iMax ) return iMax;
	return iValue;
}

static int __uiDesignCountRows(const char* sText, int iMaxRows)
{
	const char* sCursor;
	int iRows;
	int bHasText;

	if ( iMaxRows <= 0 ) return 0;
	if ( (sText == NULL) || (sText[0] == 0) ) return 0;
	sCursor = sText;
	iRows = 0;
	while ( *sCursor != 0 && iRows < iMaxRows ) {
		bHasText = 0;
		while ( *sCursor != 0 && *sCursor != '\n' && *sCursor != '\r' ) {
			if ( *sCursor != ' ' && *sCursor != '\t' ) bHasText = 1;
			sCursor++;
		}
		if ( bHasText ) iRows++;
		while ( *sCursor == '\n' || *sCursor == '\r' ) sCursor++;
	}
	return iRows;
}

static void __uiDesignApplyChildHostRect(const ui_design_node_t* pNode, const ui_design_node_t* pChild, xui_rect_t* pRect)
{
	float fHeader;
	float fTitle;
	float fBorder;
	float fClientTop;
	float fDivider;
	float fPaneSize;
	float fTabH;
	float fTabStrip;
	float fPadding;
	float fAvailable;
	int iCount;
	int iIndex;
	int iOrientation;
	int iPlacement;

	if ( (pNode == NULL) || (pRect == NULL) ) return;
	switch ( pNode->iType ) {
	case UI_DESIGN_NODE_PANEL:
		fHeader = uiDesignNodeGetPropertyFloat(pNode, "metrics.headerHeight", 28.0f);
		fHeader = __uiDesignClampRange(fHeader, 0.0f, pRect->fH);
		pRect->fY += fHeader;
		pRect->fH = __uiDesignMaxFloat(0.0f, pRect->fH - fHeader);
		break;
	case UI_DESIGN_NODE_WINDOW:
		fBorder = uiDesignNodeGetPropertyFloat(pNode, "metrics.borderWidth", 1.0f);
		fBorder = __uiDesignClampRange(fBorder, 0.0f, pRect->fW * 0.5f);
		fBorder = __uiDesignClampRange(fBorder, 0.0f, pRect->fH * 0.5f);
		fTitle = uiDesignNodeGetPropertyBool(pNode, "behavior.showTitleBar", 1) ?
			uiDesignNodeGetPropertyFloat(pNode, "metrics.titleBarHeight", 30.0f) : 0.0f;
		fTitle = __uiDesignClampRange(fTitle, 0.0f, pRect->fH);
		fClientTop = __uiDesignClampRange(fBorder + fTitle, 0.0f, pRect->fH);
		pRect->fX += fBorder;
		pRect->fY += fClientTop;
		pRect->fW = __uiDesignMaxFloat(0.0f, pRect->fW - fBorder * 2.0f);
		pRect->fH = __uiDesignMaxFloat(0.0f, pRect->fH - fBorder * 2.0f - fTitle);
		break;
	case UI_DESIGN_NODE_SPLIT_LAYOUT:
		iCount = uiDesignNodeGetPropertyInt(pNode, "data.paneCount", __uiDesignCountRows(uiDesignNodeGetProperty(pNode, "data.panes", ""), XUI_SPLIT_LAYOUT_MAX_PANES));
		if ( iCount < 1 ) iCount = 1;
		if ( iCount > XUI_SPLIT_LAYOUT_MAX_PANES ) iCount = XUI_SPLIT_LAYOUT_MAX_PANES;
		iIndex = (pChild != NULL) ? uiDesignNodeGetPropertyInt(pChild, "layout.splitPane", 0) : 0;
		iIndex = __uiDesignClampIntRange(iIndex, 0, iCount - 1);
		fDivider = uiDesignNodeGetPropertyFloat(pNode, "metrics.dividerSize", 6.0f);
		fDivider = __uiDesignClampRange(fDivider, 0.0f, 1000.0f);
		iOrientation = uiDesignNodeGetPropertyInt(pNode, "behavior.orientation", XUI_ORIENTATION_VERTICAL);
		if ( iOrientation == XUI_ORIENTATION_VERTICAL ) {
			fPaneSize = __uiDesignMaxFloat(0.0f, (pRect->fW - fDivider * (float)(iCount - 1)) / (float)iCount);
			pRect->fX += (fPaneSize + fDivider) * (float)iIndex;
			pRect->fW = fPaneSize;
		} else {
			fPaneSize = __uiDesignMaxFloat(0.0f, (pRect->fH - fDivider * (float)(iCount - 1)) / (float)iCount);
			pRect->fY += (fPaneSize + fDivider) * (float)iIndex;
			pRect->fH = fPaneSize;
		}
		break;
	case UI_DESIGN_NODE_TABS:
		iPlacement = uiDesignNodeGetPropertyInt(pNode, "behavior.placement", XUI_TABS_PLACEMENT_TOP);
		fTabH = uiDesignNodeGetPropertyFloat(pNode, "metrics.tabHeight", 30.0f);
		if ( fTabH < 18.0f ) fTabH = 18.0f;
		fTabStrip = fTabH + 2.0f;
		fTabStrip = __uiDesignClampRange(fTabStrip, 0.0f,
			(iPlacement == XUI_TABS_PLACEMENT_LEFT || iPlacement == XUI_TABS_PLACEMENT_RIGHT) ? pRect->fW : pRect->fH);
		if ( iPlacement == XUI_TABS_PLACEMENT_BOTTOM ) {
			pRect->fH = __uiDesignMaxFloat(0.0f, pRect->fH - fTabStrip);
		} else if ( iPlacement == XUI_TABS_PLACEMENT_LEFT ) {
			pRect->fX += fTabStrip;
			pRect->fW = __uiDesignMaxFloat(0.0f, pRect->fW - fTabStrip);
		} else if ( iPlacement == XUI_TABS_PLACEMENT_RIGHT ) {
			pRect->fW = __uiDesignMaxFloat(0.0f, pRect->fW - fTabStrip);
		} else {
			pRect->fY += fTabStrip;
			pRect->fH = __uiDesignMaxFloat(0.0f, pRect->fH - fTabStrip);
		}
		pRect->fX += 8.0f;
		pRect->fY += 8.0f;
		pRect->fW = __uiDesignMaxFloat(0.0f, pRect->fW - 16.0f);
		pRect->fH = __uiDesignMaxFloat(0.0f, pRect->fH - 16.0f);
		break;
	case UI_DESIGN_NODE_ACCORDION:
		iCount = __uiDesignCountRows(uiDesignNodeGetProperty(pNode, "data.sections", ""), XUI_ACCORDION_SECTION_CAPACITY);
		if ( iCount < 1 ) iCount = 1;
		iIndex = (pChild != NULL) ? uiDesignNodeGetPropertyInt(pChild, "layout.accordionSection", 0) : 0;
		iIndex = __uiDesignClampIntRange(iIndex, 0, iCount - 1);
		fHeader = uiDesignNodeGetPropertyFloat(pNode, "metrics.headerHeight", 28.0f);
		fHeader = __uiDesignClampRange(fHeader, 0.0f, pRect->fH);
		fPadding = uiDesignNodeGetPropertyFloat(pNode, "metrics.contentPadding", 8.0f);
		fPadding = __uiDesignClampRange(fPadding, 0.0f, pRect->fH * 0.5f);
		fAvailable = __uiDesignMaxFloat(0.0f, pRect->fH - fHeader * (float)iCount);
		pRect->fY += fHeader * (float)(iIndex + 1) + fPadding;
		pRect->fX += fPadding;
		pRect->fW = __uiDesignMaxFloat(0.0f, pRect->fW - fPadding * 2.0f);
		pRect->fH = __uiDesignMaxFloat(0.0f, fAvailable - fPadding * 2.0f);
		break;
	case UI_DESIGN_NODE_POPUP:
		pRect->fX -= uiDesignNodeGetPropertyFloat(pNode, "value.scrollX", 0.0f);
		pRect->fY -= uiDesignNodeGetPropertyFloat(pNode, "value.scrollY", 0.0f);
		pRect->fW = __uiDesignMaxFloat(1.0f, uiDesignNodeGetPropertyFloat(pNode, "metrics.contentWidth", pRect->fW));
		pRect->fH = __uiDesignMaxFloat(1.0f, uiDesignNodeGetPropertyFloat(pNode, "metrics.contentHeight", pRect->fH));
		break;
	default:
		break;
	}
}

int uiDesignModelGetChildHostRect(const ui_design_model_t* pModel, int iId, xui_rect_t* pRect)
{
	const ui_design_node_t* pNode;
	xui_rect_t tRect;

	if ( pRect == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode = uiDesignModelGetNodeConst(pModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( uiDesignModelGetAbsoluteRect(pModel, iId, &tRect) != XUI_OK ) return XUI_ERROR_INVALID_ARGUMENT;
	__uiDesignApplyChildHostRect(pNode, NULL, &tRect);
	*pRect = tRect;
	return XUI_OK;
}

int uiDesignModelGetAbsoluteRect(const ui_design_model_t* pModel, int iId, xui_rect_t* pRect)
{
	const ui_design_node_t* pNode;
	xui_rect_t tRect;
	xui_rect_t tParent;

	if ( pRect == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode = uiDesignModelGetNodeConst(pModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = pNode->tRect;
	if ( pNode->iParentId != 0 ) {
		const ui_design_node_t* pParent = uiDesignModelGetNodeConst(pModel, pNode->iParentId);
		if ( pParent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		if ( uiDesignModelGetAbsoluteRect(pModel, pNode->iParentId, &tParent) != XUI_OK ) return XUI_ERROR_INVALID_ARGUMENT;
		__uiDesignApplyChildHostRect(pParent, pNode, &tParent);
		tRect.fX += tParent.fX;
		tRect.fY += tParent.fY;
	}
	*pRect = tRect;
	return XUI_OK;
}

int uiDesignModelHitTest(const ui_design_model_t* pModel, float fX, float fY)
{
	xui_rect_t tRect;
	int i;

	if ( pModel == NULL ) return 0;
	for ( i = pModel->iNodeCount - 1; i >= 0; i-- ) {
		if ( uiDesignModelGetAbsoluteRect(pModel, pModel->arrNodes[i].iId, &tRect) != XUI_OK ) continue;
		if ( __uiDesignRectContains(tRect, fX, fY) ) return pModel->arrNodes[i].iId;
	}
	return 0;
}

int uiDesignModelFindDropParent(const ui_design_model_t* pModel, float fX, float fY)
{
	xui_rect_t tRect;
	int i;

	if ( pModel == NULL ) return 0;
	for ( i = pModel->iNodeCount - 1; i >= 0; i-- ) {
		if ( !uiDesignNodeTypeIsContainer(pModel->arrNodes[i].iType) ) continue;
		if ( uiDesignModelGetChildHostRect(pModel, pModel->arrNodes[i].iId, &tRect) != XUI_OK ) continue;
		if ( __uiDesignRectContains(tRect, fX, fY) ) return pModel->arrNodes[i].iId;
	}
	return 0;
}

static int __uiDesignFlattenChildren(const ui_design_model_t* pModel, int iParentId, int iDepth, ui_design_flat_node_t* pItems, int iCapacity, int* pCount)
{
	int i;

	for ( i = 0; i < pModel->iNodeCount; i++ ) {
		if ( pModel->arrNodes[i].iParentId != iParentId ) continue;
		if ( *pCount >= iCapacity ) return XUI_ERROR_OUT_OF_MEMORY;
		pItems[*pCount].iId = pModel->arrNodes[i].iId;
		pItems[*pCount].iDepth = iDepth;
		(*pCount)++;
		if ( uiDesignNodeTypeIsContainer(pModel->arrNodes[i].iType) ) {
			int iRet = __uiDesignFlattenChildren(pModel, pModel->arrNodes[i].iId, iDepth + 1, pItems, iCapacity, pCount);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

int uiDesignModelFlatten(const ui_design_model_t* pModel, ui_design_flat_node_t* pItems, int iCapacity)
{
	int iCount;
	int iRet;

	if ( (pModel == NULL) || (pItems == NULL) || (iCapacity <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	iCount = 0;
	iRet = __uiDesignFlattenChildren(pModel, 0, 0, pItems, iCapacity, &iCount);
	if ( iRet != XUI_OK ) return iRet;
	return iCount;
}
