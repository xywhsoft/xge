#include "uid_app.h"
#include <stdio.h>
#include <string.h>

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

int UIDesignShellBuild(uid_app_t* pApp)
{
	static const char* arrCommon[] = { "Pointer", "Label", "Button", "Input", "Image", "CheckBox", "Slider" };
	static const char* arrContainers[] = { "Panel", "Row", "Column", "Grid", "ScrollView", "Tabs", "DockLayout" };
	static const char* arrProject[] = { "project.xson", "pages/main.xson", "styles/theme.xson", "res/", "assets/" };
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
	if ( UIDesignAddLabel(pApp, pApp->pMenuBar, "File   Edit   View   Project   Format   Run   Tools   Window   Help", 26.0f, XGE_COLOR_RGBA(32, 48, 60, 255), XGE_COLOR_RGBA(243, 247, 250, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pToolbar, "New  Open  Save  |  Cut  Copy  Paste  |  Undo  Redo  |  Start Preview  |  Align  Snap  Tab Order", 34.0f, XGE_COLOR_RGBA(32, 48, 60, 255), XGE_COLOR_RGBA(236, 244, 250, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pShellStatusBar, "Ready    |    Target: Desktop 1366x768    |    Document: pages/main.xson    |    On-demand refresh", 24.0f, XGE_COLOR_RGBA(52, 70, 86, 255), XGE_COLOR_RGBA(238, 244, 249, 255)) != XGE_OK ) {
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
	pApp->pInspector = UIDesignCreateWidget(XGE_XUI_LAYOUT_COLUMN, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pActions = UIDesignCreateWidget(XGE_XUI_LAYOUT_COLUMN, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pOutput = UIDesignCreateWidget(XGE_XUI_LAYOUT_COLUMN, -1.0f, -1.0f, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(194, 210, 224, 255));
	pApp->pResources = NULL;
	if ( (pApp->pToolbox == NULL) || (pApp->pStructure == NULL) || (pApp->pCenterPane == NULL) ||
	     (pApp->pProjectExplorer == NULL) || (pApp->pInspector == NULL) || (pApp->pActions == NULL) || (pApp->pOutput == NULL) ) {
		return XGE_ERROR;
	}
	if ( UIDesignBuildListSection(pApp, pApp->pToolbox, "Toolbox", arrCommon, (int)(sizeof(arrCommon) / sizeof(arrCommon[0]))) != XGE_OK ||
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
	pContentRow = UIDesignCreateWidget(XGE_XUI_LAYOUT_COLUMN, 520.0f, 360.0f, XGE_COLOR_RGBA(250, 252, 254, 255), XGE_COLOR_RGBA(92, 122, 148, 255));
	if ( UIDesignAdd(pApp->pCanvas, pContentRow) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetRect(pContentRow, (xge_rect_t){ 64.0f, 54.0f, 520.0f, 360.0f });
	if ( UIDesignAddLabel(pApp, pContentRow, "Form / Page Surface", 30.0f, XGE_COLOR_RGBA(32, 56, 76, 255), XGE_COLOR_RGBA(232, 240, 247, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pContentRow, "Drop XUI controls here", 28.0f, XGE_COLOR_RGBA(92, 108, 120, 255), XGE_COLOR_RGBA(250, 252, 254, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pContentRow, "Selection handles, grid, snap lines and adorners will render in this canvas.", 28.0f, XGE_COLOR_RGBA(92, 108, 120, 255), XGE_COLOR_RGBA(250, 252, 254, 255)) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( UIDesignBuildListSection(pApp, pApp->pProjectExplorer, "Project Explorer", arrProject, (int)(sizeof(arrProject) / sizeof(arrProject[0]))) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pInspector, "Properties", 24.0f, XGE_COLOR_RGBA(24, 48, 70, 255), XGE_COLOR_RGBA(220, 232, 242, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pInspector, "Object: mainForm", 22.0f, XGE_COLOR_RGBA(52, 70, 86, 255), XGE_COLOR_RGBA(248, 252, 255, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pInspector, "(Name)    mainForm", 22.0f, XGE_COLOR_RGBA(52, 70, 86, 255), XGE_COLOR_RGBA(248, 252, 255, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pInspector, "Width     1366", 22.0f, XGE_COLOR_RGBA(52, 70, 86, 255), XGE_COLOR_RGBA(248, 252, 255, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pInspector, "Height    768", 22.0f, XGE_COLOR_RGBA(52, 70, 86, 255), XGE_COLOR_RGBA(248, 252, 255, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pActions, "Actions", 24.0f, XGE_COLOR_RGBA(24, 48, 70, 255), XGE_COLOR_RGBA(220, 232, 242, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pActions, "Edit Items...", 22.0f, XGE_COLOR_RGBA(52, 70, 86, 255), XGE_COLOR_RGBA(248, 252, 255, 255)) != XGE_OK ||
	     UIDesignAddLabel(pApp, pApp->pActions, "Edit Dock Layout...", 22.0f, XGE_COLOR_RGBA(52, 70, 86, 255), XGE_COLOR_RGBA(248, 252, 255, 255)) != XGE_OK ) {
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
	for ( i = 0; i < pApp->iLabelCount; i++ ) {
		if ( pApp->arrLabels[i].bReady ) {
			xgeXuiLabelUnit(&pApp->arrLabels[i].tLabel);
			pApp->arrLabels[i].bReady = 0;
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
}
