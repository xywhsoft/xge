#include "uid_app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int UIDesignCanvasRebuild(uid_app_t* pApp);
static float UIDesignClampFloat(float fValue, float fMin, float fMax);
static xge_rect_t UIDesignCanvasClampMoveRect(uid_app_t* pApp, xge_rect_t tRect);

static const xge_xui_menu_item_t g_arrUidFileMenu[] = {
	{ "New Project", "Ctrl+N", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, UID_CMD_NEW_PROJECT, 0, NULL, NULL },
	{ "Save Page", "Ctrl+S", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, UID_CMD_SAVE_PAGE, 0, NULL, NULL },
	{ NULL, NULL, XGE_XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL },
	{ "Exit", "Alt+F4", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, UID_CMD_EXIT, 0, NULL, NULL }
};

static const xge_xui_menu_item_t g_arrUidProjectMenu[] = {
	{ "Add Page", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, UID_CMD_ADD_PAGE, 0, NULL, NULL },
	{ "Refresh Project", "F5", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, UID_CMD_REFRESH_PROJECT, 0, NULL, NULL }
};

static const xge_xui_menu_item_t g_arrUidViewMenu[] = {
	{ "Reset Layout", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, UID_CMD_RESET_LAYOUT, 0, NULL, NULL }
};

static const xge_xui_menu_item_t g_arrUidHelpMenu[] = {
	{ "About XUI Designer", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, UID_CMD_ABOUT, 0, NULL, NULL }
};

static const char* g_arrUidToolbarText[] = {
	"New", "Save", "", "Label", "Button", "Input", "Panel", "", "Preview"
};

static const int g_arrUidToolbarType[] = {
	XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_SEPARATOR,
	XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_BUTTON,
	XGE_XUI_TOOLBAR_ITEM_SEPARATOR, XGE_XUI_TOOLBAR_ITEM_BUTTON
};

static xge_xui_widget UIDesignCreateWidget(int iLayout, float fW, float fH, uint32_t iBackground, uint32_t iBorder)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return NULL;
	}
	xgeXuiWidgetSetLayout(pWidget, iLayout);
	xgeXuiWidgetSetSize(pWidget, (fW < 0.0f) ? xgeXuiSizeGrow(1.0f) : xgeXuiSizePx(fW), (fH < 0.0f) ? xgeXuiSizeGrow(1.0f) : xgeXuiSizePx(fH));
	xgeXuiWidgetSetBackground(pWidget, iBackground);
	xgeXuiWidgetSetBorder(pWidget, 1.0f, iBorder);
	return pWidget;
}

static int UIDesignAdd(xge_xui_widget pParent, xge_xui_widget pChild)
{
	if ( (pParent == NULL) || (pChild == NULL) ) {
		return XGE_ERROR;
	}
	return xgeXuiWidgetAdd(pParent, pChild);
}

static int UIDesignAddLabel(uid_app_t* pApp, xge_xui_widget pParent, const char* sText, float fH, uint32_t iTextColor, uint32_t iBackground)
{
	uid_label_slot_t* pSlot;

	if ( (pApp == NULL) || (pParent == NULL) || (pApp->iLabelCount >= UID_SHELL_LABEL_CAPACITY) ) {
		return XGE_ERROR;
	}
	pSlot = &pApp->arrLabels[pApp->iLabelCount++];
	memset(pSlot, 0, sizeof(*pSlot));
	pSlot->pWidget = UIDesignCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, -1.0f, fH, iBackground, XGE_COLOR_RGBA(0, 0, 0, 0));
	if ( pSlot->pWidget == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetPaddingPx(pSlot->pWidget, 8.0f, 0.0f, 8.0f, 0.0f);
	if ( xgeXuiLabelInit(&pSlot->tLabel, pSlot->pWidget, xgeXuiGetDefaultFont(&pApp->tXui), sText) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pSlot->tLabel, iTextColor);
	xgeXuiLabelSetAlign(&pSlot->tLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	pSlot->bReady = 1;
	return UIDesignAdd(pParent, pSlot->pWidget);
}

static int UIDesignBuildListSection(uid_app_t* pApp, xge_xui_widget pParent, const char* sTitle, const char** arrItems, int iCount)
{
	int i;

	if ( UIDesignAddLabel(pApp, pParent, sTitle, 24.0f, XGE_COLOR_RGBA(24, 48, 70, 255), XGE_COLOR_RGBA(220, 232, 242, 255)) != XGE_OK ) {
		return XGE_ERROR;
	}
	for ( i = 0; i < iCount; i++ ) {
		if ( UIDesignAddLabel(pApp, pParent, arrItems[i], 22.0f, XGE_COLOR_RGBA(52, 70, 86, 255), XGE_COLOR_RGBA(248, 252, 255, 255)) != XGE_OK ) {
			return XGE_ERROR;
		}
	}
	return XGE_OK;
}

void UIDesignStatusSet(uid_app_t* pApp, const char* sText)
{
	if ( (pApp == NULL) || !pApp->bStatusBarReady || (pApp->iStatusMain < 0) ) {
		return;
	}
	xgeXuiStatusBarSetItemText(&pApp->tStatusBar, pApp->iStatusMain, (sText != NULL) ? sText : "Ready");
	xgeXuiWidgetMarkPaint(pApp->pShellStatusBar);
}

void UIDesignStatusRefresh(uid_app_t* pApp)
{
	char sProject[256];
	char sSelection[256];
	uid_node_t* pNode;

	if ( (pApp == NULL) || !pApp->bStatusBarReady ) {
		return;
	}
	snprintf(sProject, sizeof(sProject), "Project: %s | Page: %s", pApp->tProject.bReady ? pApp->tProject.sName : "None", pApp->tProject.sCurrentPage);
	pNode = UIDesignDocumentGetSelected(&pApp->tDocument);
	if ( pNode != NULL ) {
		snprintf(sSelection, sizeof(sSelection), "Selected: %s  %.0f,%.0f %.0fx%.0f", pNode->sName, pNode->tRect.fX, pNode->tRect.fY, pNode->tRect.fW, pNode->tRect.fH);
	} else {
		snprintf(sSelection, sizeof(sSelection), "Selected: None");
	}
	if ( pApp->iStatusProject >= 0 ) {
		xgeXuiStatusBarSetItemText(&pApp->tStatusBar, pApp->iStatusProject, sProject);
	}
	if ( pApp->iStatusSelection >= 0 ) {
		xgeXuiStatusBarSetItemText(&pApp->tStatusBar, pApp->iStatusSelection, sSelection);
	}
	xgeXuiWidgetMarkPaint(pApp->pShellStatusBar);
}

static void UIDesignMenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	(void)pWidget;
	(void)iIndex;
	UIDesignCommandExecute((uid_app_t*)pUser, iValue);
}

static void UIDesignToolbarSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	static const int arrCommands[] = {
		UID_CMD_NEW_PROJECT, UID_CMD_SAVE_PAGE, 0,
		UID_CMD_ADD_LABEL, UID_CMD_ADD_BUTTON, UID_CMD_ADD_INPUT, UID_CMD_ADD_PANEL,
		0, UID_CMD_PREVIEW
	};

	(void)pWidget;
	if ( (iIndex >= 0) && (iIndex < (int)(sizeof(arrCommands) / sizeof(arrCommands[0]))) && (arrCommands[iIndex] != 0) ) {
		UIDesignCommandExecute((uid_app_t*)pUser, arrCommands[iIndex]);
	}
}

static void UIDesignProjectTreeSelect(xge_xui_widget pWidget, int iNodeId, void* pUser)
{
	uid_app_t* pApp;

	(void)pWidget;
	pApp = (uid_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	if ( iNodeId == 20 ) {
		UIDesignStatusSet(pApp, "Project page selected: pages/main.xson");
	} else if ( iNodeId == 30 ) {
		UIDesignStatusSet(pApp, "Resource folder selected: res/");
	} else if ( iNodeId == 40 ) {
		UIDesignStatusSet(pApp, "Asset folder selected: assets/");
	} else {
		UIDesignStatusSet(pApp, "Project selected");
	}
}

static int UIDesignInspectorAddProperty(xge_xui_property_grid pGrid, int iCategory, const char* sId, const char* sName, const char* sDescription, int iType, const char* sValue, const char* sDefault, int iFlags)
{
	xge_xui_property_desc_t tDesc;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.sId = sId;
	tDesc.sName = sName;
	tDesc.sDescription = sDescription;
	tDesc.iType = iType;
	tDesc.sValue = sValue;
	tDesc.sDefaultValue = sDefault;
	tDesc.iFlags = iFlags;
	return xgeXuiPropertyGridAddProperty(pGrid, iCategory, &tDesc);
}

static void UIDesignInspectorChange(xge_xui_widget pWidget, int iProperty, const char* sId, const char* sValue, int iType, void* pUser)
{
	uid_app_t* pApp;
	uid_node_t* pNode;
	float fValue;

	(void)pWidget;
	(void)iProperty;
	(void)iType;
	pApp = (uid_app_t*)pUser;
	if ( (pApp == NULL) || pApp->bInspectorRefreshing || (sId == NULL) || (sValue == NULL) ) {
		return;
	}
	pNode = UIDesignDocumentGetSelected(&pApp->tDocument);
	if ( pNode == NULL ) {
		return;
	}
	if ( strcmp(sId, "node.name") == 0 ) {
		snprintf(pNode->sName, sizeof(pNode->sName), "%s", sValue);
	} else if ( strcmp(sId, "node.text") == 0 ) {
		snprintf(pNode->sText, sizeof(pNode->sText), "%s", sValue);
	} else if ( strcmp(sId, "layout.x") == 0 ) {
		fValue = (float)atof(sValue);
		pNode->tRect.fX = UIDesignClampFloat(fValue, 0.0f, pApp->pFormSurface->tRect.fW - pNode->tRect.fW);
	} else if ( strcmp(sId, "layout.y") == 0 ) {
		fValue = (float)atof(sValue);
		pNode->tRect.fY = UIDesignClampFloat(fValue, 0.0f, pApp->pFormSurface->tRect.fH - pNode->tRect.fH);
	} else if ( strcmp(sId, "layout.w") == 0 ) {
		fValue = (float)atof(sValue);
		pNode->tRect.fW = UIDesignClampFloat(fValue, 20.0f, pApp->pFormSurface->tRect.fW - pNode->tRect.fX);
	} else if ( strcmp(sId, "layout.h") == 0 ) {
		fValue = (float)atof(sValue);
		pNode->tRect.fH = UIDesignClampFloat(fValue, 18.0f, pApp->pFormSurface->tRect.fH - pNode->tRect.fY);
	} else {
		return;
	}
	UIDesignDocumentMarkDirty(&pApp->tDocument);
	UIDesignCanvasRebuild(pApp);
	UIDesignInspectorRefresh(pApp);
	UIDesignInvalidate(pApp, UID_INVALIDATE_REASON_DOCUMENT | UID_INVALIDATE_REASON_CANVAS | UID_INVALIDATE_REASON_INSPECTOR);
}

void UIDesignInspectorRefresh(uid_app_t* pApp)
{
	xge_xui_property_grid pGrid;
	uid_node_t* pNode;
	char sId[32];
	char sX[32];
	char sY[32];
	char sW[32];
	char sH[32];
	int iObjectCategory;
	int iLayoutCategory;

	if ( (pApp == NULL) || !pApp->bInspectorGridReady ) {
		return;
	}
	pGrid = &pApp->tInspectorGrid;
	pApp->bInspectorRefreshing = 1;
	xgeXuiPropertyGridClear(pGrid);
	pNode = UIDesignDocumentGetSelected(&pApp->tDocument);
	if ( pNode == NULL ) {
		iObjectCategory = xgeXuiPropertyGridAddCategory(pGrid, "empty", "Selection", 1);
		if ( iObjectCategory >= 0 ) {
			UIDesignInspectorAddProperty(pGrid, iObjectCategory, "empty.hint", "Object", "Select a control on the design surface.", XGE_XUI_TABLE_CELL_TYPE_TEXT, "None", "None", XGE_XUI_PROPERTY_FLAG_READONLY);
		}
		pApp->bInspectorRefreshing = 0;
		xgeXuiWidgetMarkPaint(pApp->pInspectorGridWidget);
		return;
	}
	snprintf(sId, sizeof(sId), "%d", pNode->iId);
	snprintf(sX, sizeof(sX), "%.0f", pNode->tRect.fX);
	snprintf(sY, sizeof(sY), "%.0f", pNode->tRect.fY);
	snprintf(sW, sizeof(sW), "%.0f", pNode->tRect.fW);
	snprintf(sH, sizeof(sH), "%.0f", pNode->tRect.fH);
	iObjectCategory = xgeXuiPropertyGridAddCategory(pGrid, "object", "Object", 1);
	if ( iObjectCategory >= 0 ) {
		UIDesignInspectorAddProperty(pGrid, iObjectCategory, "node.id", "Id", "Document node id.", XGE_XUI_TABLE_CELL_TYPE_INT, sId, sId, XGE_XUI_PROPERTY_FLAG_READONLY);
		UIDesignInspectorAddProperty(pGrid, iObjectCategory, "node.type", "Type", "XUI control type.", XGE_XUI_TABLE_CELL_TYPE_TEXT, pNode->sType, pNode->sType, XGE_XUI_PROPERTY_FLAG_READONLY);
		UIDesignInspectorAddProperty(pGrid, iObjectCategory, "node.name", "(Name)", "Control identifier used by generated code and events.", XGE_XUI_TABLE_CELL_TYPE_TEXT, pNode->sName, pNode->sName, 0);
		UIDesignInspectorAddProperty(pGrid, iObjectCategory, "node.text", "Text", "Control display text.", XGE_XUI_TABLE_CELL_TYPE_TEXT, pNode->sText, pNode->sText, 0);
	}
	iLayoutCategory = xgeXuiPropertyGridAddCategory(pGrid, "layout", "Layout", 1);
	if ( iLayoutCategory >= 0 ) {
		UIDesignInspectorAddProperty(pGrid, iLayoutCategory, "layout.x", "X", "Left position in the page surface.", XGE_XUI_TABLE_CELL_TYPE_FLOAT, sX, sX, 0);
		UIDesignInspectorAddProperty(pGrid, iLayoutCategory, "layout.y", "Y", "Top position in the page surface.", XGE_XUI_TABLE_CELL_TYPE_FLOAT, sY, sY, 0);
		UIDesignInspectorAddProperty(pGrid, iLayoutCategory, "layout.w", "Width", "Control width.", XGE_XUI_TABLE_CELL_TYPE_FLOAT, sW, sW, 0);
		UIDesignInspectorAddProperty(pGrid, iLayoutCategory, "layout.h", "Height", "Control height.", XGE_XUI_TABLE_CELL_TYPE_FLOAT, sH, sH, 0);
	}
	xgeXuiPropertyGridSetSelected(pGrid, xgeXuiPropertyGridFindProperty(pGrid, "node.name"));
	pApp->bInspectorRefreshing = 0;
	xgeXuiWidgetMarkPaint(pApp->pInspectorGridWidget);
}

int UIDesignProjectRefresh(uid_app_t* pApp)
{
	if ( (pApp == NULL) || !pApp->bProjectTreeReady ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiTreeViewClear(&pApp->tProjectTree);
	xgeXuiTreeViewAddNode(&pApp->tProjectTree, 10, -1, pApp->tProject.bReady ? pApp->tProject.sName : "No Project");
	xgeXuiTreeViewAddNode(&pApp->tProjectTree, 20, 10, "pages/main.xson");
	xgeXuiTreeViewAddNode(&pApp->tProjectTree, 30, 10, "res/");
	xgeXuiTreeViewAddNode(&pApp->tProjectTree, 40, 10, "assets/");
	xgeXuiTreeViewAddNode(&pApp->tProjectTree, 50, 10, "styles/theme.xson");
	xgeXuiTreeViewSetNodeExpanded(&pApp->tProjectTree, 10, 1);
	xgeXuiTreeViewSetSelected(&pApp->tProjectTree, 20);
	xgeXuiWidgetMarkPaint(pApp->pProjectTreeWidget);
	UIDesignStatusRefresh(pApp);
	return XGE_OK;
}

int UIDesignCommandExecute(uid_app_t* pApp, int iCommand)
{
	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( iCommand == UID_CMD_NEW_PROJECT ) {
		if ( UIDesignProjectEnsureDefault(&pApp->tProject) == XGE_OK ) {
			UIDesignProjectRefresh(pApp);
			UIDesignStatusSet(pApp, "Project ready: workspace/DemoProject");
			return XGE_OK;
		}
		UIDesignStatusSet(pApp, "Failed to create project");
		return XGE_ERROR;
	}
	if ( iCommand == UID_CMD_SAVE_PAGE ) {
		if ( !pApp->tProject.bReady ) {
			UIDesignProjectEnsureDefault(&pApp->tProject);
			UIDesignProjectRefresh(pApp);
		}
		if ( UIDesignProjectSaveDocument(&pApp->tProject, &pApp->tDocument) == XGE_OK ) {
			pApp->tDocument.iCleanRevision = pApp->tDocument.iRevision;
			pApp->tDocument.bDirty = 0;
			UIDesignStatusSet(pApp, "Page saved: pages/main.xson");
			UIDesignStatusRefresh(pApp);
			return XGE_OK;
		}
		UIDesignStatusSet(pApp, "Failed to save page");
		return XGE_ERROR;
	}
	if ( iCommand == UID_CMD_ADD_PAGE ) {
		if ( !pApp->tProject.bReady ) {
			UIDesignProjectEnsureDefault(&pApp->tProject);
		}
		UIDesignProjectRefresh(pApp);
		UIDesignStatusSet(pApp, "MVP supports one page: pages/main.xson");
		return XGE_OK;
	}
	if ( iCommand == UID_CMD_REFRESH_PROJECT ) {
		UIDesignProjectRefresh(pApp);
		UIDesignStatusSet(pApp, "Project tree refreshed");
		return XGE_OK;
	}
	if ( iCommand == UID_CMD_ADD_LABEL ) {
		return UIDesignAppAddControl(pApp, "label");
	}
	if ( iCommand == UID_CMD_ADD_BUTTON ) {
		return UIDesignAppAddControl(pApp, "button");
	}
	if ( iCommand == UID_CMD_ADD_INPUT ) {
		return UIDesignAppAddControl(pApp, "input");
	}
	if ( iCommand == UID_CMD_ADD_PANEL ) {
		return UIDesignAppAddControl(pApp, "panel");
	}
	if ( iCommand == UID_CMD_PREVIEW ) {
		UIDesignStatusSet(pApp, "Preview command is wired; runtime preview comes next");
		return XGE_OK;
	}
	if ( iCommand == UID_CMD_RESET_LAYOUT ) {
		UIDesignStatusSet(pApp, "Dock layout reset command is wired");
		return XGE_OK;
	}
	if ( iCommand == UID_CMD_ABOUT ) {
		UIDesignStatusSet(pApp, "XUI Designer MVP");
		return XGE_OK;
	}
	if ( iCommand == UID_CMD_EXIT ) {
		xgeQuit();
		return XGE_OK;
	}
	return XGE_ERROR_INVALID_ARGUMENT;
}

static void UIDesignToolboxButtonClick(xge_xui_widget pWidget, void* pUser)
{
	uid_toolbox_button_t* pButton;

	(void)pWidget;
	pButton = (uid_toolbox_button_t*)pUser;
	if ( (pButton == NULL) || (pButton->sType == NULL) ) {
		return;
	}
	UIDesignAppAddControl(pButton->pApp, pButton->sType);
}

static int UIDesignAddToolboxButton(uid_app_t* pApp, xge_xui_widget pParent, const char* sText, const char* sType)
{
	uid_toolbox_button_t* pSlot;

	if ( (pApp == NULL) || (pParent == NULL) || (pApp->iToolboxButtonCount >= UID_TOOLBOX_BUTTON_CAPACITY) ) {
		return XGE_ERROR;
	}
	pSlot = &pApp->arrToolboxButtons[pApp->iToolboxButtonCount++];
	memset(pSlot, 0, sizeof(*pSlot));
	pSlot->pWidget = UIDesignCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, -1.0f, 26.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(0, 0, 0, 0));
	if ( pSlot->pWidget == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetMarginPx(pSlot->pWidget, 4.0f, 1.0f, 4.0f, 1.0f);
	if ( xgeXuiButtonInit(&pSlot->tButton, &pApp->tXui, pSlot->pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(&pSlot->tButton, xgeXuiGetDefaultFont(&pApp->tXui), sText);
	xgeXuiButtonSetTextColor(&pSlot->tButton, XGE_COLOR_RGBA(42, 62, 78, 255));
	xgeXuiButtonSetColors(&pSlot->tButton,
		XGE_COLOR_RGBA(248, 252, 255, 255),
		XGE_COLOR_RGBA(222, 238, 250, 255),
		XGE_COLOR_RGBA(205, 226, 244, 255),
		XGE_COLOR_RGBA(210, 232, 248, 255),
		XGE_COLOR_RGBA(230, 234, 238, 255));
	pSlot->sType = sType;
	pSlot->pApp = pApp;
	xgeXuiButtonSetClick(&pSlot->tButton, UIDesignToolboxButtonClick, pSlot);
	pSlot->bReady = 1;
	return UIDesignAdd(pParent, pSlot->pWidget);
}

static int UIDesignInitDockWindow(uid_app_t* pApp, int iIndex, const char* sTitle, xge_xui_widget pClient, int bClosable)
{
	if ( (pApp == NULL) || (iIndex < 0) || (iIndex >= UID_DOCK_WINDOW_COUNT) || (pClient == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiDockWindowInit(&pApp->arrDockWindow[iIndex], &pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->iDockWindowReadyCount <= iIndex ) {
		pApp->iDockWindowReadyCount = iIndex + 1;
	}
	xgeXuiDockWindowSetTitle(&pApp->arrDockWindow[iIndex], sTitle);
	xgeXuiDockWindowSetClosable(&pApp->arrDockWindow[iIndex], bClosable);
	xgeXuiDockWindowSetClientWidget(&pApp->arrDockWindow[iIndex], pClient);
	return XGE_OK;
}

static int UIDesignInitMenuBar(uid_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->pMenuBar == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiMenuInit(&pApp->tFileMenu, &pApp->tXui) != XGE_OK ||
	     xgeXuiMenuInit(&pApp->tProjectMenu, &pApp->tXui) != XGE_OK ||
	     xgeXuiMenuInit(&pApp->tViewMenu, &pApp->tXui) != XGE_OK ||
	     xgeXuiMenuInit(&pApp->tHelpMenu, &pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiMenuSetItems(&pApp->tFileMenu, g_arrUidFileMenu, (int)(sizeof(g_arrUidFileMenu) / sizeof(g_arrUidFileMenu[0])));
	xgeXuiMenuSetItems(&pApp->tProjectMenu, g_arrUidProjectMenu, (int)(sizeof(g_arrUidProjectMenu) / sizeof(g_arrUidProjectMenu[0])));
	xgeXuiMenuSetItems(&pApp->tViewMenu, g_arrUidViewMenu, (int)(sizeof(g_arrUidViewMenu) / sizeof(g_arrUidViewMenu[0])));
	xgeXuiMenuSetItems(&pApp->tHelpMenu, g_arrUidHelpMenu, (int)(sizeof(g_arrUidHelpMenu) / sizeof(g_arrUidHelpMenu[0])));
	xgeXuiMenuSetSelect(&pApp->tFileMenu, UIDesignMenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tProjectMenu, UIDesignMenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tViewMenu, UIDesignMenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tHelpMenu, UIDesignMenuSelect, pApp);
	if ( xgeXuiMenuBarInit(&pApp->tMenuBar, &pApp->tXui, pApp->pMenuBar) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiMenuBarSetFont(&pApp->tMenuBar, xgeXuiGetDefaultFont(&pApp->tXui));
	xgeXuiMenuBarSetSelect(&pApp->tMenuBar, UIDesignMenuSelect, pApp);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "File", &pApp->tFileMenu, 1);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "Project", &pApp->tProjectMenu, 2);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "View", &pApp->tViewMenu, 3);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "Help", &pApp->tHelpMenu, 4);
	pApp->bMenuReady = 1;
	return XGE_OK;
}

static int UIDesignInitToolbar(uid_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->pToolbar == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiToolbarInit(&pApp->tToolbar, &pApp->tXui, pApp->pToolbar) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiToolbarSetFont(&pApp->tToolbar, xgeXuiGetDefaultFont(&pApp->tXui));
	xgeXuiToolbarSetItems(&pApp->tToolbar, g_arrUidToolbarText, g_arrUidToolbarType, (int)(sizeof(g_arrUidToolbarText) / sizeof(g_arrUidToolbarText[0])));
	xgeXuiToolbarSetItemSize(&pApp->tToolbar, 72.0f, 28.0f, 10.0f);
	xgeXuiToolbarSetGroupGap(&pApp->tToolbar, 8.0f);
	xgeXuiToolbarSetSelect(&pApp->tToolbar, UIDesignToolbarSelect, pApp);
	xgeXuiToolbarSetItemTooltip(&pApp->tToolbar, 0, "Create or initialize project");
	xgeXuiToolbarSetItemTooltip(&pApp->tToolbar, 1, "Save current page");
	xgeXuiToolbarSetItemTooltip(&pApp->tToolbar, 3, "Add Label");
	xgeXuiToolbarSetItemTooltip(&pApp->tToolbar, 4, "Add Button");
	xgeXuiToolbarSetItemTooltip(&pApp->tToolbar, 5, "Add Input");
	xgeXuiToolbarSetItemTooltip(&pApp->tToolbar, 6, "Add Panel");
	xgeXuiToolbarSetItemTooltip(&pApp->tToolbar, 8, "Preview");
	pApp->bToolbarReady = 1;
	return XGE_OK;
}

static int UIDesignInitStatusBar(uid_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->pShellStatusBar == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiStatusBarInit(&pApp->tStatusBar, &pApp->tXui, pApp->pShellStatusBar) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiStatusBarSetFont(&pApp->tStatusBar, xgeXuiGetDefaultFont(&pApp->tXui));
	pApp->iStatusMain = xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "Ready", 180.0f, 0);
	pApp->iStatusProject = xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "Project: None", 320.0f, 0);
	xgeXuiStatusBarAddFlexibleSpacer(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, 1.0f);
	pApp->iStatusSelection = xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_RIGHT, "Selected: None", 270.0f, 0);
	pApp->bStatusBarReady = 1;
	UIDesignStatusRefresh(pApp);
	return XGE_OK;
}

static int UIDesignInitProjectTree(uid_app_t* pApp)
{
	if ( (pApp == NULL) || (pApp->pProjectTreeWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiTreeViewInit(&pApp->tProjectTree, &pApp->tXui, pApp->pProjectTreeWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiTreeViewSetFont(&pApp->tProjectTree, xgeXuiGetDefaultFont(&pApp->tXui));
	xgeXuiTreeViewSetMetrics(&pApp->tProjectTree, 24.0f, 18.0f);
	xgeXuiTreeViewSetScrollbarMode(&pApp->tProjectTree, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiTreeViewSetColors(&pApp->tProjectTree,
		XGE_COLOR_RGBA(248, 252, 255, 255),
		XGE_COLOR_RGBA(248, 252, 255, 255),
		XGE_COLOR_RGBA(204, 232, 250, 255),
		XGE_COLOR_RGBA(31, 75, 112, 255),
		XGE_COLOR_RGBA(230, 240, 247, 255),
		XGE_COLOR_RGBA(120, 176, 212, 255));
	xgeXuiTreeViewSetSelect(&pApp->tProjectTree, UIDesignProjectTreeSelect, pApp);
	pApp->bProjectTreeReady = 1;
	return UIDesignProjectRefresh(pApp);
}

int UIDesignShellBuild(uid_app_t* pApp)
{
	static const char* arrCommon[] = { "Pointer", "Image", "CheckBox", "Slider" };
	static const char* arrContainers[] = { "Panel", "Row", "Column", "Grid", "ScrollView", "Tabs", "DockLayout" };
	xge_xui_widget pContentRow;

	if ( (pApp == NULL) || (pApp->pRoot == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pApp->pMenuBar = UIDesignCreateWidget(XGE_XUI_LAYOUT_ROW, -1.0f, 26.0f, XGE_COLOR_RGBA(243, 247, 250, 255), XGE_COLOR_RGBA(184, 198, 210, 255));
	pApp->pToolbar = UIDesignCreateWidget(XGE_XUI_LAYOUT_ROW, -1.0f, 34.0f, XGE_COLOR_RGBA(236, 244, 250, 255), XGE_COLOR_RGBA(184, 198, 210, 255));
	pApp->pDockHost = UIDesignCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, -1.0f, -1.0f, XGE_COLOR_RGBA(226, 232, 238, 255), XGE_COLOR_RGBA(154, 174, 192, 255));
	pApp->pShellStatusBar = UIDesignCreateWidget(XGE_XUI_LAYOUT_ROW, -1.0f, 24.0f, XGE_COLOR_RGBA(238, 244, 249, 255), XGE_COLOR_RGBA(184, 198, 210, 255));
	if ( UIDesignAdd(pApp->pRoot, pApp->pMenuBar) != XGE_OK ||
	     UIDesignAdd(pApp->pRoot, pApp->pToolbar) != XGE_OK ||
	     UIDesignAdd(pApp->pRoot, pApp->pDockHost) != XGE_OK ||
	     UIDesignAdd(pApp->pRoot, pApp->pShellStatusBar) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( UIDesignInitMenuBar(pApp) != XGE_OK ||
	     UIDesignInitToolbar(pApp) != XGE_OK ||
	     UIDesignInitStatusBar(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiDockLayoutInit(&pApp->tDockLayout, &pApp->tXui, pApp->pDockHost) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->bDockLayoutReady = 1;

	pApp->pToolbox = UIDesignCreateWidget(XGE_XUI_LAYOUT_COLUMN, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pStructure = UIDesignCreateWidget(XGE_XUI_LAYOUT_COLUMN, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pCenterPane = UIDesignCreateWidget(XGE_XUI_LAYOUT_COLUMN, -1.0f, -1.0f, XGE_COLOR_RGBA(218, 225, 232, 255), XGE_COLOR_RGBA(154, 174, 192, 255));
	pApp->pProjectExplorer = UIDesignCreateWidget(XGE_XUI_LAYOUT_COLUMN, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pProjectToolbar = UIDesignCreateWidget(XGE_XUI_LAYOUT_ROW, -1.0f, 24.0f, XGE_COLOR_RGBA(226, 242, 252, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pProjectTreeWidget = UIDesignCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pInspector = UIDesignCreateWidget(XGE_XUI_LAYOUT_COLUMN, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pInspectorGridWidget = UIDesignCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pActions = UIDesignCreateWidget(XGE_XUI_LAYOUT_COLUMN, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pOutput = UIDesignCreateWidget(XGE_XUI_LAYOUT_COLUMN, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pResources = NULL;
	if ( (pApp->pToolbox == NULL) || (pApp->pStructure == NULL) || (pApp->pCenterPane == NULL) ||
	     (pApp->pProjectExplorer == NULL) || (pApp->pProjectToolbar == NULL) || (pApp->pProjectTreeWidget == NULL) ||
	     (pApp->pInspector == NULL) || (pApp->pInspectorGridWidget == NULL) ||
	     (pApp->pActions == NULL) || (pApp->pOutput == NULL) ) {
		return XGE_ERROR;
	}
	if ( UIDesignAddLabel(pApp, pApp->pToolbox, "Toolbox", 24.0f, XGE_COLOR_RGBA(24, 48, 70, 255), XGE_COLOR_RGBA(220, 232, 242, 255)) != XGE_OK ||
	     UIDesignAddToolboxButton(pApp, pApp->pToolbox, "Label", "label") != XGE_OK ||
	     UIDesignAddToolboxButton(pApp, pApp->pToolbox, "Button", "button") != XGE_OK ||
	     UIDesignAddToolboxButton(pApp, pApp->pToolbox, "Input", "input") != XGE_OK ||
	     UIDesignAddToolboxButton(pApp, pApp->pToolbox, "Panel", "panel") != XGE_OK ||
	     UIDesignBuildListSection(pApp, pApp->pToolbox, "More Controls", arrCommon, (int)(sizeof(arrCommon) / sizeof(arrCommon[0]))) != XGE_OK ||
	     UIDesignBuildListSection(pApp, pApp->pToolbox, "Containers", arrContainers, (int)(sizeof(arrContainers) / sizeof(arrContainers[0]))) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pStructure, "Structure", 24.0f, XGE_COLOR_RGBA(24, 48, 70, 255), XGE_COLOR_RGBA(220, 232, 242, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pStructure, "mainForm", 22.0f, XGE_COLOR_RGBA(52, 70, 86, 255), XGE_COLOR_RGBA(248, 252, 255, 255)) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->pDesignerTabs = UIDesignCreateWidget(XGE_XUI_LAYOUT_ROW, -1.0f, 28.0f, XGE_COLOR_RGBA(238, 244, 249, 255), XGE_COLOR_RGBA(184, 198, 210, 255));
	pApp->pCanvas = UIDesignCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, -1.0f, -1.0f, XGE_COLOR_RGBA(202, 211, 220, 255), XGE_COLOR_RGBA(154, 174, 192, 255));
	if ( UIDesignAdd(pApp->pCenterPane, pApp->pDesignerTabs) != XGE_OK ||
	     UIDesignAdd(pApp->pCenterPane, pApp->pCanvas) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pDesignerTabs, "Design   XSON   Preview", 28.0f, XGE_COLOR_RGBA(32, 48, 60, 255), XGE_COLOR_RGBA(238, 244, 249, 255)) != XGE_OK ) {
		return XGE_ERROR;
	}
	pContentRow = UIDesignCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, 520.0f, 360.0f, XGE_COLOR_RGBA(250, 252, 254, 255), XGE_COLOR_RGBA(92, 122, 148, 255));
	pApp->pFormSurface = pContentRow;
	if ( UIDesignAdd(pApp->pCanvas, pApp->pFormSurface) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetRect(pApp->pFormSurface, (xge_rect_t){ 64.0f, 54.0f, 520.0f, 360.0f });
	if ( UIDesignCanvasRebuild(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( UIDesignAddLabel(pApp, pApp->pProjectExplorer, "Project Explorer", 24.0f, XGE_COLOR_RGBA(24, 48, 70, 255), XGE_COLOR_RGBA(220, 232, 242, 255)) != XGE_OK ||
	     UIDesignAdd(pApp->pProjectExplorer, pApp->pProjectToolbar) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pProjectToolbar, "New Project   Save   Refresh", 24.0f, XGE_COLOR_RGBA(32, 64, 92, 255), XGE_COLOR_RGBA(226, 242, 252, 255)) != XGE_OK ||
	     UIDesignAdd(pApp->pProjectExplorer, pApp->pProjectTreeWidget) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pInspector, "Properties", 24.0f, XGE_COLOR_RGBA(24, 48, 70, 255), XGE_COLOR_RGBA(220, 232, 242, 255)) != XGE_OK ||
	     UIDesignAdd(pApp->pInspector, pApp->pInspectorGridWidget) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pActions, "Actions", 24.0f, XGE_COLOR_RGBA(24, 48, 70, 255), XGE_COLOR_RGBA(220, 232, 242, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pActions, "Edit Items...", 22.0f, XGE_COLOR_RGBA(52, 70, 86, 255), XGE_COLOR_RGBA(248, 252, 255, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pActions, "Edit Dock Layout...", 22.0f, XGE_COLOR_RGBA(52, 70, 86, 255), XGE_COLOR_RGBA(248, 252, 255, 255)) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPropertyGridInit(&pApp->tInspectorGrid, &pApp->tXui, pApp->pInspectorGridWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->bInspectorGridReady = 1;
	xgeXuiPropertyGridSetFont(&pApp->tInspectorGrid, xgeXuiGetDefaultFont(&pApp->tXui));
	xgeXuiPropertyGridSetMetrics(&pApp->tInspectorGrid, 104.0f, 24.0f, 25.0f);
	xgeXuiPropertyGridSetDescriptionMode(&pApp->tInspectorGrid, XGE_XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, 0.0f);
	xgeXuiPropertyGridSetEditMode(&pApp->tInspectorGrid, XGE_XUI_TABLE_GRID_EDIT_QUICK);
	xgeXuiPropertyGridSetScrollbarMode(&pApp->tInspectorGrid, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiPropertyGridSetColors(&pApp->tInspectorGrid,
		XGE_COLOR_RGBA(248, 252, 255, 255),
		XGE_COLOR_RGBA(226, 242, 252, 255),
		XGE_COLOR_RGBA(248, 252, 255, 255),
		XGE_COLOR_RGBA(204, 232, 250, 255),
		XGE_COLOR_RGBA(164, 206, 236, 255),
		XGE_COLOR_RGBA(31, 75, 112, 255));
	xgeXuiPropertyGridSetChange(&pApp->tInspectorGrid, UIDesignInspectorChange, pApp);
	UIDesignInspectorRefresh(pApp);
	if ( UIDesignProjectEnsureDefault(&pApp->tProject) != XGE_OK ||
	     UIDesignInitProjectTree(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( UIDesignAddLabel(pApp, pApp->pOutput, "Immediate / Output / Validation / Event Log", 24.0f, XGE_COLOR_RGBA(24, 48, 70, 255), XGE_COLOR_RGBA(220, 232, 242, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pOutput, "XUI Designer shell initialized. MVP framework ready for document, registry and canvas implementation.", 22.0f, XGE_COLOR_RGBA(52, 70, 86, 255), XGE_COLOR_RGBA(248, 252, 255, 255)) != XGE_OK ) {
		return XGE_ERROR;
	}

	if ( UIDesignInitDockWindow(pApp, UID_DOCK_DOCUMENT, "main.xson [Design]", pApp->pCenterPane, 0) != XGE_OK ||
	     UIDesignInitDockWindow(pApp, UID_DOCK_TOOLBOX, "Toolbox", pApp->pToolbox, 1) != XGE_OK ||
	     UIDesignInitDockWindow(pApp, UID_DOCK_STRUCTURE, "Structure", pApp->pStructure, 1) != XGE_OK ||
	     UIDesignInitDockWindow(pApp, UID_DOCK_PROJECT, "Project Explorer", pApp->pProjectExplorer, 1) != XGE_OK ||
	     UIDesignInitDockWindow(pApp, UID_DOCK_PROPERTIES, "Properties", pApp->pInspector, 1) != XGE_OK ||
	     UIDesignInitDockWindow(pApp, UID_DOCK_ACTIONS, "Actions", pApp->pActions, 1) != XGE_OK ||
	     UIDesignInitDockWindow(pApp, UID_DOCK_OUTPUT, "Output", pApp->pOutput, 1) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->pDocumentPane = xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[UID_DOCK_DOCUMENT], XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_FILL, 0.0f);
	pApp->pToolPane = xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[UID_DOCK_TOOLBOX], XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_LEFT, 0.20f);
	if ( xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[UID_DOCK_STRUCTURE], XGE_XUI_DOCK_REGION_LEFT, XGE_XUI_DOCK_SIDE_FILL, 0.0f) == NULL ) {
		return XGE_ERROR;
	}
	pApp->pProjectPane = xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[UID_DOCK_PROJECT], XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_RIGHT, 0.26f);
	if ( xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[UID_DOCK_PROPERTIES], XGE_XUI_DOCK_REGION_RIGHT, XGE_XUI_DOCK_SIDE_FILL, 0.0f) == NULL ||
	     xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[UID_DOCK_ACTIONS], XGE_XUI_DOCK_REGION_RIGHT, XGE_XUI_DOCK_SIDE_FILL, 0.0f) == NULL ) {
		return XGE_ERROR;
	}
	pApp->pOutputPane = xgeXuiDockLayoutDockWindow(&pApp->tDockLayout, &pApp->arrDockWindow[UID_DOCK_OUTPUT], XGE_XUI_DOCK_REGION_DOCUMENT, XGE_XUI_DOCK_SIDE_BOTTOM, 0.22f);
	if ( (pApp->pDocumentPane == NULL) || (pApp->pToolPane == NULL) || (pApp->pProjectPane == NULL) || (pApp->pOutputPane == NULL) ) {
		return XGE_ERROR;
	}
	xgeXuiDockPaneSetActiveIndex(pApp->pDocumentPane, 0);
	xgeXuiDockPaneSetActiveIndex(pApp->pToolPane, 0);
	xgeXuiDockPaneSetActiveIndex(pApp->pProjectPane, 1);
	return XGE_OK;
}

void UIDesignShellUnit(uid_app_t* pApp)
{
	int i;

	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->bProjectTreeReady ) {
		xgeXuiTreeViewUnit(&pApp->tProjectTree);
		pApp->bProjectTreeReady = 0;
	}
	if ( pApp->bInspectorGridReady ) {
		xgeXuiPropertyGridUnit(&pApp->tInspectorGrid);
		pApp->bInspectorGridReady = 0;
	}
	if ( pApp->bStatusBarReady ) {
		xgeXuiStatusBarUnit(&pApp->tStatusBar);
		pApp->bStatusBarReady = 0;
	}
	if ( pApp->bToolbarReady ) {
		xgeXuiToolbarUnit(&pApp->tToolbar);
		pApp->bToolbarReady = 0;
	}
	if ( pApp->bMenuReady ) {
		xgeXuiMenuBarUnit(&pApp->tMenuBar);
		xgeXuiMenuUnit(&pApp->tFileMenu);
		xgeXuiMenuUnit(&pApp->tProjectMenu);
		xgeXuiMenuUnit(&pApp->tViewMenu);
		xgeXuiMenuUnit(&pApp->tHelpMenu);
		pApp->bMenuReady = 0;
	}
	for ( i = 0; i < pApp->iLabelCount; i++ ) {
		if ( pApp->arrLabels[i].bReady ) {
			xgeXuiLabelUnit(&pApp->arrLabels[i].tLabel);
			pApp->arrLabels[i].bReady = 0;
		}
	}
	for ( i = 0; i < pApp->iToolboxButtonCount; i++ ) {
		if ( pApp->arrToolboxButtons[i].bReady ) {
			xgeXuiButtonUnit(&pApp->arrToolboxButtons[i].tButton);
			pApp->arrToolboxButtons[i].bReady = 0;
		}
	}
	for ( i = 0; i < pApp->iPreviewControlCount; i++ ) {
		if ( pApp->arrPreviewControls[i].bReady ) {
			if ( pApp->arrPreviewControls[i].iKind == 1 ) {
				xgeXuiLabelUnit(&pApp->arrPreviewControls[i].tLabel);
			} else if ( pApp->arrPreviewControls[i].iKind == 2 ) {
				xgeXuiButtonUnit(&pApp->arrPreviewControls[i].tButton);
			} else if ( pApp->arrPreviewControls[i].iKind == 3 ) {
				xgeXuiInputUnit(&pApp->arrPreviewControls[i].tInput);
			}
			pApp->arrPreviewControls[i].bReady = 0;
		}
	}
	for ( i = pApp->iDockWindowReadyCount - 1; i >= 0; i-- ) {
		xgeXuiDockWindowUnit(&pApp->arrDockWindow[i]);
	}
	if ( pApp->bDockLayoutReady ) {
		xgeXuiDockLayoutUnit(&pApp->tDockLayout);
		pApp->bDockLayoutReady = 0;
	}
	pApp->iLabelCount = 0;
	pApp->iDockWindowReadyCount = 0;
	pApp->iToolboxButtonCount = 0;
	pApp->iPreviewControlCount = 0;
}

static void UIDesignPreviewClear(uid_app_t* pApp)
{
	int i;

	if ( pApp == NULL ) {
		return;
	}
	for ( i = 0; i < pApp->iPreviewControlCount; i++ ) {
		if ( pApp->arrPreviewControls[i].bReady ) {
			if ( pApp->arrPreviewControls[i].iKind == 1 ) {
				xgeXuiLabelUnit(&pApp->arrPreviewControls[i].tLabel);
			} else if ( pApp->arrPreviewControls[i].iKind == 2 ) {
				xgeXuiButtonUnit(&pApp->arrPreviewControls[i].tButton);
			} else if ( pApp->arrPreviewControls[i].iKind == 3 ) {
				xgeXuiInputUnit(&pApp->arrPreviewControls[i].tInput);
			}
			if ( pApp->arrPreviewControls[i].pWidget != NULL ) {
				xgeXuiWidgetRemove(pApp->arrPreviewControls[i].pWidget);
				xgeXuiWidgetFree(pApp->arrPreviewControls[i].pWidget);
			}
			pApp->arrPreviewControls[i].bReady = 0;
		}
	}
	pApp->iPreviewControlCount = 0;
}

static uid_preview_control_t* UIDesignPreviewAlloc(uid_app_t* pApp)
{
	uid_preview_control_t* pSlot;

	if ( (pApp == NULL) || (pApp->iPreviewControlCount >= UID_PREVIEW_CONTROL_CAPACITY) ) {
		return NULL;
	}
	pSlot = &pApp->arrPreviewControls[pApp->iPreviewControlCount++];
	memset(pSlot, 0, sizeof(*pSlot));
	return pSlot;
}

static int UIDesignRectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX <= (tRect.fX + tRect.fW)) && (fY <= (tRect.fY + tRect.fH));
}

static float UIDesignClampFloat(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

static int UIDesignRectChanged(xge_rect_t a, xge_rect_t b)
{
	return (a.fX != b.fX) || (a.fY != b.fY) || (a.fW != b.fW) || (a.fH != b.fH);
}

static xge_rect_t UIDesignCanvasHandleRect(xge_rect_t tRect, int iHandle)
{
	const float fSize = 7.0f;
	const float fHalf = fSize * 0.5f;
	float fLeft;
	float fCenterX;
	float fRight;
	float fTop;
	float fCenterY;
	float fBottom;
	xge_rect_t tHandle;

	fLeft = tRect.fX;
	fCenterX = tRect.fX + tRect.fW * 0.5f;
	fRight = tRect.fX + tRect.fW;
	fTop = tRect.fY;
	fCenterY = tRect.fY + tRect.fH * 0.5f;
	fBottom = tRect.fY + tRect.fH;
	tHandle = (xge_rect_t){ fCenterX - fHalf, fCenterY - fHalf, fSize, fSize };
	if ( iHandle == UID_CANVAS_HANDLE_NW ) {
		tHandle.fX = fLeft - fHalf;
		tHandle.fY = fTop - fHalf;
	} else if ( iHandle == UID_CANVAS_HANDLE_N ) {
		tHandle.fX = fCenterX - fHalf;
		tHandle.fY = fTop - fHalf;
	} else if ( iHandle == UID_CANVAS_HANDLE_NE ) {
		tHandle.fX = fRight - fHalf;
		tHandle.fY = fTop - fHalf;
	} else if ( iHandle == UID_CANVAS_HANDLE_E ) {
		tHandle.fX = fRight - fHalf;
		tHandle.fY = fCenterY - fHalf;
	} else if ( iHandle == UID_CANVAS_HANDLE_SE ) {
		tHandle.fX = fRight - fHalf;
		tHandle.fY = fBottom - fHalf;
	} else if ( iHandle == UID_CANVAS_HANDLE_S ) {
		tHandle.fX = fCenterX - fHalf;
		tHandle.fY = fBottom - fHalf;
	} else if ( iHandle == UID_CANVAS_HANDLE_SW ) {
		tHandle.fX = fLeft - fHalf;
		tHandle.fY = fBottom - fHalf;
	} else if ( iHandle == UID_CANVAS_HANDLE_W ) {
		tHandle.fX = fLeft - fHalf;
		tHandle.fY = fCenterY - fHalf;
	}
	return tHandle;
}

static int UIDesignCanvasHitHandle(uid_app_t* pApp, float fLocalX, float fLocalY)
{
	const uid_node_t* pNode;
	int i;
	xge_rect_t tHit;

	if ( pApp == NULL ) {
		return -1;
	}
	pNode = UIDesignDocumentGetSelected(&pApp->tDocument);
	if ( pNode == NULL ) {
		return -1;
	}
	for ( i = 0; i < 8; i++ ) {
		tHit = UIDesignCanvasHandleRect(pNode->tRect, i);
		tHit.fX -= 3.0f;
		tHit.fY -= 3.0f;
		tHit.fW += 6.0f;
		tHit.fH += 6.0f;
		if ( UIDesignRectContains(tHit, fLocalX, fLocalY) ) {
			return i;
		}
	}
	return -1;
}

static int UIDesignCanvasHitNode(uid_app_t* pApp, float fLocalX, float fLocalY)
{
	int i;

	if ( pApp == NULL ) {
		return -1;
	}
	for ( i = pApp->tDocument.iNodeCount - 1; i >= 0; i-- ) {
		if ( UIDesignRectContains(pApp->tDocument.arrNodes[i].tRect, fLocalX, fLocalY) ) {
			return i;
		}
	}
	return -1;
}

static int UIDesignToolboxHitButton(uid_app_t* pApp, float fX, float fY)
{
	int i;

	if ( pApp == NULL ) {
		return -1;
	}
	for ( i = 0; i < pApp->iToolboxButtonCount; i++ ) {
		if ( pApp->arrToolboxButtons[i].bReady &&
		     pApp->arrToolboxButtons[i].pWidget != NULL &&
		     UIDesignRectContains(pApp->arrToolboxButtons[i].pWidget->tRect, fX, fY) ) {
			return i;
		}
	}
	return -1;
}

static xge_rect_t UIDesignToolboxDragPreviewRect(uid_app_t* pApp, const char* sType, float fX, float fY, int* pOverCanvas)
{
	xge_rect_t tSurface;
	xge_rect_t tRect;

	tRect = UIDesignDocumentDefaultRect(sType);
	if ( pOverCanvas != NULL ) {
		*pOverCanvas = 0;
	}
	if ( (pApp == NULL) || (pApp->pFormSurface == NULL) ) {
		return tRect;
	}
	tSurface = pApp->pFormSurface->tRect;
	if ( !UIDesignRectContains(tSurface, fX, fY) ) {
		tRect.fX = fX - tRect.fW * 0.5f;
		tRect.fY = fY - tRect.fH * 0.5f;
		return tRect;
	}
	if ( pOverCanvas != NULL ) {
		*pOverCanvas = 1;
	}
	tRect.fX = fX - tSurface.fX;
	tRect.fY = fY - tSurface.fY;
	tRect = UIDesignCanvasClampMoveRect(pApp, tRect);
	tRect.fX += tSurface.fX;
	tRect.fY += tSurface.fY;
	return tRect;
}

static void UIDesignToolboxDragUpdate(uid_app_t* pApp, float fX, float fY)
{
	if ( pApp == NULL ) {
		return;
	}
	pApp->fToolboxDragX = fX;
	pApp->fToolboxDragY = fY;
	pApp->tToolboxDragPreview = UIDesignToolboxDragPreviewRect(pApp, pApp->sToolboxDragType, fX, fY, &pApp->bToolboxDragOverCanvas);
	UIDesignInvalidate(pApp, UID_INVALIDATE_REASON_CANVAS);
}

static int UIDesignToolboxDragHandleEvent(uid_app_t* pApp, const xge_event_t* pEvent)
{
	int iHit;
	float fDx;
	float fDy;
	xge_rect_t tSurface;

	if ( (pApp == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pApp->bToolboxDrag ) {
		if ( pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
			fDx = pEvent->fX - pApp->fToolboxDragStartX;
			fDy = pEvent->fY - pApp->fToolboxDragStartY;
			if ( (fDx * fDx + fDy * fDy) > 16.0f ) {
				pApp->bToolboxDragMoved = 1;
			}
			UIDesignToolboxDragUpdate(pApp, pEvent->fX, pEvent->fY);
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( (pEvent->iType == XGE_EVENT_MOUSE_UP) && (pEvent->iParam1 == XGE_MOUSE_LEFT) ) {
			UIDesignToolboxDragUpdate(pApp, pEvent->fX, pEvent->fY);
			if ( pApp->bToolboxDragOverCanvas ) {
				tSurface = pApp->pFormSurface->tRect;
				UIDesignAppAddControlAt(pApp, pApp->sToolboxDragType, pApp->tToolboxDragPreview.fX - tSurface.fX, pApp->tToolboxDragPreview.fY - tSurface.fY);
			} else if ( !pApp->bToolboxDragMoved ) {
				UIDesignAppAddControl(pApp, pApp->sToolboxDragType);
			}
			pApp->bToolboxDrag = 0;
			pApp->bToolboxDragMoved = 0;
			pApp->bToolboxDragOverCanvas = 0;
			pApp->sToolboxDragType = NULL;
			UIDesignInvalidate(pApp, UID_INVALIDATE_REASON_CANVAS | UID_INVALIDATE_REASON_DOCUMENT | UID_INVALIDATE_REASON_INSPECTOR);
			return XGE_XUI_EVENT_CONSUMED;
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType != XGE_EVENT_MOUSE_DOWN) || (pEvent->iParam1 != XGE_MOUSE_LEFT) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iHit = UIDesignToolboxHitButton(pApp, pEvent->fX, pEvent->fY);
	if ( iHit < 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	pApp->bToolboxDrag = 1;
	pApp->bToolboxDragMoved = 0;
	pApp->sToolboxDragType = pApp->arrToolboxButtons[iHit].sType;
	pApp->fToolboxDragStartX = pEvent->fX;
	pApp->fToolboxDragStartY = pEvent->fY;
	UIDesignToolboxDragUpdate(pApp, pEvent->fX, pEvent->fY);
	return XGE_XUI_EVENT_CONSUMED;
}

static xge_rect_t UIDesignCanvasClampMoveRect(uid_app_t* pApp, xge_rect_t tRect)
{
	xge_rect_t tSurface;

	if ( (pApp == NULL) || (pApp->pFormSurface == NULL) ) {
		return tRect;
	}
	tSurface = pApp->pFormSurface->tRect;
	tRect.fX = UIDesignClampFloat(tRect.fX, 0.0f, tSurface.fW - tRect.fW);
	tRect.fY = UIDesignClampFloat(tRect.fY, 0.0f, tSurface.fH - tRect.fH);
	return tRect;
}

static xge_rect_t UIDesignCanvasClampResizeRect(uid_app_t* pApp, xge_rect_t tRect)
{
	const float fMinW = 20.0f;
	const float fMinH = 18.0f;
	xge_rect_t tSurface;

	if ( (pApp == NULL) || (pApp->pFormSurface == NULL) ) {
		return tRect;
	}
	tSurface = pApp->pFormSurface->tRect;
	if ( tRect.fW < fMinW ) {
		tRect.fW = fMinW;
	}
	if ( tRect.fH < fMinH ) {
		tRect.fH = fMinH;
	}
	if ( tRect.fX < 0.0f ) {
		tRect.fW += tRect.fX;
		tRect.fX = 0.0f;
	}
	if ( tRect.fY < 0.0f ) {
		tRect.fH += tRect.fY;
		tRect.fY = 0.0f;
	}
	if ( (tRect.fX + tRect.fW) > tSurface.fW ) {
		tRect.fW = tSurface.fW - tRect.fX;
	}
	if ( (tRect.fY + tRect.fH) > tSurface.fH ) {
		tRect.fH = tSurface.fH - tRect.fY;
	}
	if ( tRect.fW < fMinW ) {
		tRect.fW = fMinW;
	}
	if ( tRect.fH < fMinH ) {
		tRect.fH = fMinH;
	}
	return tRect;
}

static xge_rect_t UIDesignCanvasResizeRect(uid_app_t* pApp, float fDx, float fDy)
{
	xge_rect_t tStart;
	xge_rect_t tRect;
	float fRight;
	float fBottom;
	float fMinX;
	float fMinY;

	tStart = pApp->tCanvasDragStartRect;
	tRect = tStart;
	fRight = tStart.fX + tStart.fW;
	fBottom = tStart.fY + tStart.fH;
	fMinX = fRight - 20.0f;
	fMinY = fBottom - 18.0f;
	if ( (pApp->iCanvasResizeHandle == UID_CANVAS_HANDLE_NW) ||
	     (pApp->iCanvasResizeHandle == UID_CANVAS_HANDLE_W) ||
	     (pApp->iCanvasResizeHandle == UID_CANVAS_HANDLE_SW) ) {
		tRect.fX = UIDesignClampFloat(tStart.fX + fDx, 0.0f, fMinX);
		tRect.fW = fRight - tRect.fX;
	}
	if ( (pApp->iCanvasResizeHandle == UID_CANVAS_HANDLE_NE) ||
	     (pApp->iCanvasResizeHandle == UID_CANVAS_HANDLE_E) ||
	     (pApp->iCanvasResizeHandle == UID_CANVAS_HANDLE_SE) ) {
		tRect.fW = tStart.fW + fDx;
	}
	if ( (pApp->iCanvasResizeHandle == UID_CANVAS_HANDLE_NW) ||
	     (pApp->iCanvasResizeHandle == UID_CANVAS_HANDLE_N) ||
	     (pApp->iCanvasResizeHandle == UID_CANVAS_HANDLE_NE) ) {
		tRect.fY = UIDesignClampFloat(tStart.fY + fDy, 0.0f, fMinY);
		tRect.fH = fBottom - tRect.fY;
	}
	if ( (pApp->iCanvasResizeHandle == UID_CANVAS_HANDLE_SW) ||
	     (pApp->iCanvasResizeHandle == UID_CANVAS_HANDLE_S) ||
	     (pApp->iCanvasResizeHandle == UID_CANVAS_HANDLE_SE) ) {
		tRect.fH = tStart.fH + fDy;
	}
	return UIDesignCanvasClampResizeRect(pApp, tRect);
}

static void UIDesignCanvasDragStop(uid_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	if ( pApp->bCanvasDragDirty ) {
		UIDesignDocumentMarkDirty(&pApp->tDocument);
	}
	pApp->iCanvasDragMode = UID_CANVAS_DRAG_NONE;
	pApp->iCanvasResizeHandle = -1;
	pApp->iCanvasDragNode = -1;
	pApp->bCanvasDragDirty = 0;
	UIDesignEndContinuousRefresh(pApp, UID_INVALIDATE_REASON_DOCUMENT | UID_INVALIDATE_REASON_CANVAS | UID_INVALIDATE_REASON_INSPECTOR);
}

static int UIDesignPreviewAddSelectionHandles(uid_app_t* pApp)
{
	const uid_node_t* pNode;
	uid_preview_control_t* pSlot;
	int i;
	xge_rect_t tHandle;

	pNode = UIDesignDocumentGetSelected(&pApp->tDocument);
	if ( pNode == NULL ) {
		return XGE_OK;
	}
	for ( i = 0; i < 8; i++ ) {
		pSlot = UIDesignPreviewAlloc(pApp);
		if ( pSlot == NULL ) {
			return XGE_ERROR;
		}
		tHandle = UIDesignCanvasHandleRect(pNode->tRect, i);
		pSlot->pWidget = UIDesignCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, tHandle.fW, tHandle.fH, XGE_COLOR_RGBA(255, 255, 255, 255), XGE_COLOR_RGBA(36, 118, 214, 255));
		if ( pSlot->pWidget == NULL ) {
			return XGE_ERROR;
		}
		xgeXuiWidgetSetRect(pSlot->pWidget, tHandle);
		xgeXuiWidgetSetInputTransparent(pSlot->pWidget, 1);
		pSlot->iKind = 4;
		pSlot->bReady = 1;
		if ( UIDesignAdd(pApp->pFormSurface, pSlot->pWidget) != XGE_OK ) {
			return XGE_ERROR;
		}
	}
	return XGE_OK;
}

static int UIDesignPreviewAddLabel(uid_app_t* pApp, const uid_node_t* pNode)
{
	uid_preview_control_t* pSlot;

	pSlot = UIDesignPreviewAlloc(pApp);
	if ( pSlot == NULL ) {
		return XGE_ERROR;
	}
	pSlot->pWidget = UIDesignCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, pNode->tRect.fW, pNode->tRect.fH, XGE_COLOR_RGBA(255, 255, 255, 0), XGE_COLOR_RGBA(0, 0, 0, 0));
	if ( pSlot->pWidget == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetRect(pSlot->pWidget, pNode->tRect);
	if ( xgeXuiLabelInit(&pSlot->tLabel, pSlot->pWidget, xgeXuiGetDefaultFont(&pApp->tXui), pNode->sText) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pSlot->tLabel, XGE_COLOR_RGBA(36, 56, 72, 255));
	xgeXuiLabelSetAlign(&pSlot->tLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	pSlot->iKind = 1;
	pSlot->bReady = 1;
	return UIDesignAdd(pApp->pFormSurface, pSlot->pWidget);
}

static int UIDesignPreviewAddButton(uid_app_t* pApp, const uid_node_t* pNode)
{
	uid_preview_control_t* pSlot;

	pSlot = UIDesignPreviewAlloc(pApp);
	if ( pSlot == NULL ) {
		return XGE_ERROR;
	}
	pSlot->pWidget = UIDesignCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, pNode->tRect.fW, pNode->tRect.fH, XGE_COLOR_RGBA(238, 246, 252, 255), XGE_COLOR_RGBA(108, 152, 188, 255));
	if ( pSlot->pWidget == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetRect(pSlot->pWidget, pNode->tRect);
	if ( xgeXuiButtonInit(&pSlot->tButton, &pApp->tXui, pSlot->pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(&pSlot->tButton, xgeXuiGetDefaultFont(&pApp->tXui), pNode->sText);
	pSlot->iKind = 2;
	pSlot->bReady = 1;
	return UIDesignAdd(pApp->pFormSurface, pSlot->pWidget);
}

static int UIDesignPreviewAddInput(uid_app_t* pApp, const uid_node_t* pNode)
{
	uid_preview_control_t* pSlot;

	pSlot = UIDesignPreviewAlloc(pApp);
	if ( pSlot == NULL ) {
		return XGE_ERROR;
	}
	pSlot->pWidget = UIDesignCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, pNode->tRect.fW, pNode->tRect.fH, XGE_COLOR_RGBA(255, 255, 255, 255), XGE_COLOR_RGBA(116, 156, 188, 255));
	if ( pSlot->pWidget == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetRect(pSlot->pWidget, pNode->tRect);
	if ( xgeXuiInputInit(&pSlot->tInput, &pApp->tXui, pSlot->pWidget, xgeXuiGetDefaultFont(&pApp->tXui)) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiInputSetText(&pSlot->tInput, pNode->sText);
	xgeXuiInputSetReadonly(&pSlot->tInput, 1);
	pSlot->iKind = 3;
	pSlot->bReady = 1;
	return UIDesignAdd(pApp->pFormSurface, pSlot->pWidget);
}

static int UIDesignPreviewAddPanel(uid_app_t* pApp, const uid_node_t* pNode)
{
	uid_preview_control_t* pSlot;
	xge_xui_widget pTitleWidget;
	xge_xui_label_t* pTitle;

	pSlot = UIDesignPreviewAlloc(pApp);
	if ( pSlot == NULL ) {
		return XGE_ERROR;
	}
	pSlot->pWidget = UIDesignCreateWidget(XGE_XUI_LAYOUT_COLUMN, pNode->tRect.fW, pNode->tRect.fH, XGE_COLOR_RGBA(241, 247, 252, 255), XGE_COLOR_RGBA(116, 156, 188, 255));
	if ( pSlot->pWidget == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetRect(pSlot->pWidget, pNode->tRect);
	pTitleWidget = UIDesignCreateWidget(XGE_XUI_LAYOUT_ABSOLUTE, -1.0f, 24.0f, XGE_COLOR_RGBA(221, 235, 246, 255), XGE_COLOR_RGBA(0, 0, 0, 0));
	if ( pTitleWidget == NULL ) {
		return XGE_ERROR;
	}
	pTitle = &pSlot->tLabel;
	if ( xgeXuiLabelInit(pTitle, pTitleWidget, xgeXuiGetDefaultFont(&pApp->tXui), pNode->sText) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(pTitle, XGE_COLOR_RGBA(36, 56, 72, 255));
	xgeXuiLabelSetAlign(pTitle, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	xgeXuiWidgetSetPaddingPx(pTitleWidget, 8.0f, 0.0f, 8.0f, 0.0f);
	if ( UIDesignAdd(pSlot->pWidget, pTitleWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	pSlot->iKind = 1;
	pSlot->bReady = 1;
	return UIDesignAdd(pApp->pFormSurface, pSlot->pWidget);
}

int UIDesignCanvasRebuild(uid_app_t* pApp)
{
	int i;
	const uid_node_t* pNode;

	if ( (pApp == NULL) || (pApp->pFormSurface == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	UIDesignPreviewClear(pApp);
	if ( pApp->tDocument.iNodeCount == 0 ) {
		uid_node_t tPlaceholder;

		memset(&tPlaceholder, 0, sizeof(tPlaceholder));
		strcpy(tPlaceholder.sText, "Double-click a Toolbox item or click Label/Button/Input/Panel to create controls.");
		tPlaceholder.tRect = (xge_rect_t){ 24.0f, 24.0f, 460.0f, 28.0f };
		if ( UIDesignPreviewAddLabel(pApp, &tPlaceholder) != XGE_OK ) {
			return XGE_ERROR;
		}
		xgeXuiWidgetMarkPaint(pApp->pFormSurface);
		return XGE_OK;
	}
	for ( i = 0; i < pApp->tDocument.iNodeCount; i++ ) {
		pNode = &pApp->tDocument.arrNodes[i];
		if ( strcmp(pNode->sType, "button") == 0 ) {
			if ( UIDesignPreviewAddButton(pApp, pNode) != XGE_OK ) {
				return XGE_ERROR;
			}
		} else if ( strcmp(pNode->sType, "input") == 0 ) {
			if ( UIDesignPreviewAddInput(pApp, pNode) != XGE_OK ) {
				return XGE_ERROR;
			}
		} else if ( strcmp(pNode->sType, "panel") == 0 ) {
			if ( UIDesignPreviewAddPanel(pApp, pNode) != XGE_OK ) {
				return XGE_ERROR;
			}
		} else {
			if ( UIDesignPreviewAddLabel(pApp, pNode) != XGE_OK ) {
				return XGE_ERROR;
			}
		}
		if ( i == pApp->tDocument.iSelectedNode && pApp->arrPreviewControls[pApp->iPreviewControlCount - 1].pWidget != NULL ) {
			xgeXuiWidgetSetDebugOutline(pApp->arrPreviewControls[pApp->iPreviewControlCount - 1].pWidget, 1.0f, XGE_COLOR_RGBA(36, 118, 214, 255));
		}
	}
	if ( UIDesignPreviewAddSelectionHandles(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetMarkPaint(pApp->pFormSurface);
	return XGE_OK;
}

int UIDesignCanvasHandleEvent(uid_app_t* pApp, const xge_event_t* pEvent)
{
	xge_rect_t tSurface;
	xge_rect_t tNext;
	uid_node_t* pNode;
	float fLocalX;
	float fLocalY;
	float fDx;
	float fDy;
	int iHit;

	if ( (pApp == NULL) || (pEvent == NULL) || (pApp->pFormSurface == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( UIDesignToolboxDragHandleEvent(pApp, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType != XGE_EVENT_MOUSE_DOWN) &&
	     (pEvent->iType != XGE_EVENT_MOUSE_UP) &&
	     (pEvent->iType != XGE_EVENT_MOUSE_MOVE) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	tSurface = pApp->pFormSurface->tRect;
	fLocalX = pEvent->fX - tSurface.fX;
	fLocalY = pEvent->fY - tSurface.fY;
	if ( (pApp->iCanvasDragMode != UID_CANVAS_DRAG_NONE) && (pEvent->iType == XGE_EVENT_MOUSE_MOVE) ) {
		pNode = (pApp->iCanvasDragNode >= 0 && pApp->iCanvasDragNode < pApp->tDocument.iNodeCount) ? &pApp->tDocument.arrNodes[pApp->iCanvasDragNode] : NULL;
		if ( pNode == NULL ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		fDx = fLocalX - pApp->fCanvasDragStartX;
		fDy = fLocalY - pApp->fCanvasDragStartY;
		if ( pApp->iCanvasDragMode == UID_CANVAS_DRAG_MOVE ) {
			tNext = pApp->tCanvasDragStartRect;
			tNext.fX += fDx;
			tNext.fY += fDy;
			tNext = UIDesignCanvasClampMoveRect(pApp, tNext);
		} else {
			tNext = UIDesignCanvasResizeRect(pApp, fDx, fDy);
		}
		if ( UIDesignRectChanged(pNode->tRect, tNext) ) {
			pNode->tRect = tNext;
			pApp->bCanvasDragDirty = 1;
			UIDesignCanvasRebuild(pApp);
			UIDesignInspectorRefresh(pApp);
			UIDesignInvalidate(pApp, UID_INVALIDATE_REASON_CANVAS | UID_INVALIDATE_REASON_INSPECTOR);
		}
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pApp->iCanvasDragMode != UID_CANVAS_DRAG_NONE) && (pEvent->iType == XGE_EVENT_MOUSE_UP) && (pEvent->iParam1 == XGE_MOUSE_LEFT) ) {
		UIDesignCanvasDragStop(pApp);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType != XGE_EVENT_MOUSE_DOWN) || (pEvent->iParam1 != XGE_MOUSE_LEFT) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( !UIDesignRectContains(tSurface, pEvent->fX, pEvent->fY) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iHit = UIDesignCanvasHitHandle(pApp, fLocalX, fLocalY);
	if ( iHit >= 0 ) {
		pNode = UIDesignDocumentGetSelected(&pApp->tDocument);
		if ( pNode == NULL ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		pApp->iCanvasDragMode = UID_CANVAS_DRAG_RESIZE;
		pApp->iCanvasResizeHandle = iHit;
		pApp->iCanvasDragNode = pApp->tDocument.iSelectedNode;
		pApp->bCanvasDragDirty = 0;
		pApp->fCanvasDragStartX = fLocalX;
		pApp->fCanvasDragStartY = fLocalY;
		pApp->tCanvasDragStartRect = pNode->tRect;
		UIDesignBeginContinuousRefresh(pApp, UID_INVALIDATE_REASON_CANVAS);
		return XGE_XUI_EVENT_CONSUMED;
	}
	iHit = UIDesignCanvasHitNode(pApp, fLocalX, fLocalY);
	if ( iHit >= 0 ) {
		pApp->tDocument.iSelectedNode = iHit;
		pNode = &pApp->tDocument.arrNodes[iHit];
		pApp->iCanvasDragMode = UID_CANVAS_DRAG_MOVE;
		pApp->iCanvasResizeHandle = -1;
		pApp->iCanvasDragNode = iHit;
		pApp->bCanvasDragDirty = 0;
		pApp->fCanvasDragStartX = fLocalX;
		pApp->fCanvasDragStartY = fLocalY;
		pApp->tCanvasDragStartRect = pNode->tRect;
		UIDesignCanvasRebuild(pApp);
		UIDesignInspectorRefresh(pApp);
		UIDesignBeginContinuousRefresh(pApp, UID_INVALIDATE_REASON_CANVAS);
		UIDesignInvalidate(pApp, UID_INVALIDATE_REASON_CANVAS | UID_INVALIDATE_REASON_INSPECTOR | UID_INVALIDATE_REASON_STRUCTURE);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pApp->tDocument.iSelectedNode >= 0 ) {
		pApp->tDocument.iSelectedNode = -1;
		UIDesignCanvasRebuild(pApp);
		UIDesignInspectorRefresh(pApp);
		UIDesignInvalidate(pApp, UID_INVALIDATE_REASON_CANVAS | UID_INVALIDATE_REASON_INSPECTOR | UID_INVALIDATE_REASON_STRUCTURE);
	}
	return XGE_XUI_EVENT_CONSUMED;
}

void UIDesignCanvasPaintDragOverlay(uid_app_t* pApp)
{
	if ( (pApp == NULL) || !pApp->bToolboxDrag || !pApp->bToolboxDragMoved ) {
		return;
	}
	if ( pApp->bToolboxDragOverCanvas ) {
		xgeShapeRectFillPx(pApp->tToolboxDragPreview, XGE_COLOR_RGBA(56, 136, 220, 54));
		xgeShapeRectStrokePx(pApp->tToolboxDragPreview, 2.0f, XGE_COLOR_RGBA(36, 118, 214, 230));
	} else {
		xgeShapeRectStrokePx(pApp->tToolboxDragPreview, 1.0f, XGE_COLOR_RGBA(112, 128, 142, 150));
	}
}

static void UIDesignCanvasMakeMouseEvent(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

int UIDesignCanvasSmoke(uid_app_t* pApp)
{
	xge_event_t tEvent;
	uid_node_t* pNode;
	xge_rect_t tSurface;
	float fX;
	float fY;

	if ( (pApp == NULL) || (pApp->pFormSurface == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pApp->tDocument.iNodeCount <= 0 ) {
		if ( UIDesignAppAddControl(pApp, "button") != XGE_OK ) {
			return XGE_ERROR;
		}
	}
	pApp->tDocument.iSelectedNode = 0;
	pNode = UIDesignDocumentGetSelected(&pApp->tDocument);
	if ( pNode == NULL ) {
		return XGE_ERROR;
	}
	if ( UIDesignCanvasRebuild(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	tSurface = pApp->pFormSurface->tRect;
	fX = tSurface.fX + pNode->tRect.fX + 8.0f;
	fY = tSurface.fY + pNode->tRect.fY + 8.0f;
	UIDesignCanvasMakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	UIDesignCanvasHandleEvent(pApp, &tEvent);
	UIDesignCanvasMakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, fX + 18.0f, fY + 12.0f);
	UIDesignCanvasHandleEvent(pApp, &tEvent);
	UIDesignCanvasMakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, fX + 18.0f, fY + 12.0f);
	UIDesignCanvasHandleEvent(pApp, &tEvent);
	pNode = UIDesignDocumentGetSelected(&pApp->tDocument);
	if ( pNode == NULL ) {
		return XGE_ERROR;
	}
	fX = tSurface.fX + pNode->tRect.fX + pNode->tRect.fW;
	fY = tSurface.fY + pNode->tRect.fY + pNode->tRect.fH;
	UIDesignCanvasMakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	UIDesignCanvasHandleEvent(pApp, &tEvent);
	UIDesignCanvasMakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, fX + 16.0f, fY + 10.0f);
	UIDesignCanvasHandleEvent(pApp, &tEvent);
	UIDesignCanvasMakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, fX + 16.0f, fY + 10.0f);
	UIDesignCanvasHandleEvent(pApp, &tEvent);
	return XGE_OK;
}

int UIDesignToolboxDragSmoke(uid_app_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tButton;
	xge_rect_t tSurface;
	float fStartX;
	float fStartY;
	float fDropX;
	float fDropY;

	if ( (pApp == NULL) || (pApp->iToolboxButtonCount <= 0) || (pApp->pFormSurface == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	tButton = pApp->arrToolboxButtons[0].pWidget->tRect;
	tSurface = pApp->pFormSurface->tRect;
	fStartX = tButton.fX + tButton.fW * 0.5f;
	fStartY = tButton.fY + tButton.fH * 0.5f;
	fDropX = tSurface.fX + 96.0f;
	fDropY = tSurface.fY + 92.0f;
	UIDesignCanvasMakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, fStartX, fStartY);
	UIDesignCanvasHandleEvent(pApp, &tEvent);
	UIDesignCanvasMakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, fDropX, fDropY);
	UIDesignCanvasHandleEvent(pApp, &tEvent);
	UIDesignCanvasMakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, fDropX, fDropY);
	UIDesignCanvasHandleEvent(pApp, &tEvent);
	return (pApp->tDocument.iNodeCount > 0) ? XGE_OK : XGE_ERROR;
}

int UIDesignProjectSmoke(uid_app_t* pApp)
{
	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( UIDesignCommandExecute(pApp, UID_CMD_NEW_PROJECT) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( UIDesignAppAddControl(pApp, "button") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( UIDesignCommandExecute(pApp, UID_CMD_SAVE_PAGE) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( UIDesignCommandExecute(pApp, UID_CMD_REFRESH_PROJECT) != XGE_OK ) {
		return XGE_ERROR;
	}
	return (pApp->tProject.bReady && pApp->bProjectTreeReady && pApp->tProjectTree.iNodeCount >= 5) ? XGE_OK : XGE_ERROR;
}
