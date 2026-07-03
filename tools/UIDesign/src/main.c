#include "ui_design_app.h"
#include "ui_design_canvas.h"
#include "ui_design_inspector.h"
#include "ui_design_registry.h"
#include "ui_design_toolbox.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UI_DESIGN_KEY_LEFT_SHIFT 340
#define UI_DESIGN_KEY_LEFT_CTRL 341
#define UI_DESIGN_KEY_LEFT_ALT 342
#define UI_DESIGN_KEY_RIGHT_SHIFT 344
#define UI_DESIGN_KEY_RIGHT_CTRL 345
#define UI_DESIGN_KEY_RIGHT_ALT 346

static void __uiDesignUsage(void)
{
	printf("usage: xui_uidesign [--frames N] [--seconds N] [--exercise]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __uiDesignParseArgs(ui_design_app_t* pApp, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pApp->iMaxFrames = atoi(argv[++i]);
			if ( pApp->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pApp->iMaxFrames = atoi(argv[i] + 9);
			if ( pApp->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pApp->fMaxSeconds = atof(argv[++i]);
			if ( pApp->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pApp->fMaxSeconds = atof(argv[i] + 10);
			if ( pApp->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--exercise") == 0 ) {
			pApp->bExercise = 1;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__uiDesignUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __uiDesignFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	FILE* pFile;
	int i;

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

void uiDesignAppInvalidate(ui_design_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( pApp->pToolbox != NULL ) (void)xuiWidgetInvalidate(pApp->pToolbox, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pCanvas != NULL ) (void)xuiWidgetInvalidate(pApp->pCanvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pArtboard != NULL ) (void)xuiWidgetInvalidate(pApp->pArtboard, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pOverlay != NULL ) (void)xuiWidgetInvalidate(pApp->pOverlay, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pInspector != NULL ) (void)xuiWidgetInvalidate(pApp->pInspector, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pApp->pContext != NULL ) (void)xuiInvalidateAll(pApp->pContext);
}

static int __uiDesignAppRefreshInspector(ui_design_app_t* pApp)
{
	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pApp->bSyncingInspector || pApp->bSyncingTree || pApp->pInspector == NULL ) return XUI_OK;
	return uiDesignInspectorRefresh(pApp);
}

int uiDesignAppSelectNode(ui_design_app_t* pApp, int iId)
{
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pApp->iEditingProperty = UI_DESIGN_PROPERTY_NONE;
	pApp->sEditBuffer[0] = '\0';
	pApp->iEditLength = 0;
	iRet = uiDesignModelSetSelected(&pApp->tModel, iId);
	if ( iRet != XUI_OK ) return iRet;
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

static int __uiDesignAppCarouselPage(ui_design_node_t* pParentNode, ui_design_node_t* pChildNode)
{
	int iPage;
	int iPageCount;

	if ( pParentNode == NULL ) return 0;
	iPageCount = uiDesignNodeGetPropertyInt(pParentNode, "data.pageCount", 1);
	if ( iPageCount < 1 ) iPageCount = 1;
	iPage = (pChildNode != NULL) ? uiDesignNodeGetPropertyInt(pChildNode, "layout.carouselPage", -1) : -1;
	if ( iPage < 0 ) iPage = uiDesignNodeGetPropertyInt(pParentNode, "data.current", 0);
	if ( iPage < 0 ) iPage = 0;
	if ( iPage >= iPageCount ) iPage = iPageCount - 1;
	return iPage;
}

static int __uiDesignAppSplitPane(ui_design_node_t* pParentNode, ui_design_node_t* pChildNode, float fDesignX, float fDesignY)
{
	int iPane;
	int iPaneCount;
	int iOrientation;
	float fRatio;

	if ( pParentNode == NULL ) return 0;
	iPaneCount = uiDesignNodeGetPropertyInt(pParentNode, "data.paneCount", 2);
	if ( iPaneCount < 1 ) iPaneCount = 1;
	if ( iPaneCount > XUI_SPLIT_LAYOUT_MAX_PANES ) iPaneCount = XUI_SPLIT_LAYOUT_MAX_PANES;
	iPane = (pChildNode != NULL) ? uiDesignNodeGetPropertyInt(pChildNode, "layout.splitPane", -1) : -1;
	if ( iPane < 0 ) {
		iOrientation = uiDesignNodeGetPropertyInt(pParentNode, "behavior.orientation", XUI_ORIENTATION_HORIZONTAL);
		if ( iOrientation == XUI_ORIENTATION_VERTICAL ) {
			fRatio = (pParentNode->tRect.fH > 1.0f) ? (fDesignY / pParentNode->tRect.fH) : 0.0f;
		} else {
			fRatio = (pParentNode->tRect.fW > 1.0f) ? (fDesignX / pParentNode->tRect.fW) : 0.0f;
		}
		iPane = (int)(fRatio * (float)iPaneCount);
	}
	if ( iPane < 0 ) iPane = 0;
	if ( iPane >= iPaneCount ) iPane = iPaneCount - 1;
	return iPane;
}

static int __uiDesignAppTabsPage(ui_design_node_t* pParentNode, ui_design_node_t* pChildNode)
{
	int iPage;
	int iPageCount;

	if ( pParentNode == NULL ) return 0;
	iPageCount = uiDesignNodeGetPropertyInt(pParentNode, "data.pageCount", -1);
	if ( iPageCount < 1 ) {
		const char* sCursor = uiDesignNodeGetProperty(pParentNode, "data.items", "");
		char sLine[256];
		iPageCount = 0;
		while ( sCursor != NULL && *sCursor != 0 && iPageCount < XUI_TABS_PAGE_CAPACITY ) {
			int i = 0;
			while ( sCursor[i] != 0 && sCursor[i] != '\n' && sCursor[i] != '\r' && i < (int)sizeof(sLine) - 1 ) {
				sLine[i] = sCursor[i];
				i++;
			}
			sLine[i] = 0;
			if ( sLine[0] != 0 ) iPageCount++;
			while ( *sCursor != 0 && *sCursor != '\n' && *sCursor != '\r' ) sCursor++;
			while ( *sCursor == '\n' || *sCursor == '\r' ) sCursor++;
		}
	}
	if ( iPageCount < 1 ) iPageCount = 1;
	if ( iPageCount > XUI_TABS_PAGE_CAPACITY ) iPageCount = XUI_TABS_PAGE_CAPACITY;
	iPage = (pChildNode != NULL) ? uiDesignNodeGetPropertyInt(pChildNode, "layout.tabPage", -1) : -1;
	if ( iPage < 0 ) iPage = uiDesignNodeGetPropertyInt(pParentNode, "data.selected", 0);
	if ( iPage < 0 ) iPage = 0;
	if ( iPage >= iPageCount ) iPage = iPageCount - 1;
	return iPage;
}

static int __uiDesignAppAccordionSection(ui_design_node_t* pParentNode, ui_design_node_t* pChildNode)
{
	int iSection;
	int iSectionCount;
	const char* sCursor;
	char sLine[256];

	if ( pParentNode == NULL ) return 0;
	iSection = (pChildNode != NULL) ? uiDesignNodeGetPropertyInt(pChildNode, "layout.accordionSection", -1) : -1;
	if ( iSection < 0 ) {
		iSection = 0;
	}
	sCursor = uiDesignNodeGetProperty(pParentNode, "data.sections", "");
	iSectionCount = 0;
	while ( sCursor != NULL && *sCursor != 0 && iSectionCount < XUI_ACCORDION_SECTION_CAPACITY ) {
		int i = 0;
		while ( sCursor[i] != 0 && sCursor[i] != '\n' && sCursor[i] != '\r' && i < (int)sizeof(sLine) - 1 ) {
			sLine[i] = sCursor[i];
			i++;
		}
		sLine[i] = 0;
		if ( sLine[0] != 0 ) iSectionCount++;
		while ( *sCursor != 0 && *sCursor != '\n' && *sCursor != '\r' ) sCursor++;
		while ( *sCursor == '\n' || *sCursor == '\r' ) sCursor++;
	}
	if ( iSectionCount < 1 ) iSectionCount = 1;
	if ( iSection < 0 ) iSection = 0;
	if ( iSection >= iSectionCount ) iSection = iSectionCount - 1;
	return iSection;
}

static int __uiDesignAppAttachNodeWidget(ui_design_node_t* pParentNode, ui_design_node_t* pChildNode, xui_widget pParent, xui_widget pChild)
{
	xui_widget pContent;
	int iRet;
	int iPage;
	int iPane;
	int iSection;

	if ( (pParent == NULL) || (pChild == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( xuiWidgetGetParent(pChild) != NULL ) {
		iRet = xuiWidgetRemoveFromParent(pChild);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pParentNode == NULL ) return xuiWidgetAddChild(pParent, pChild);
	switch ( pParentNode->iType ) {
	case UI_DESIGN_NODE_PANEL:
		pContent = xuiPanelGetClientWidget(pParent);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		(void)xuiWidgetSetLayoutType(pContent, XUI_LAYOUT_MANUAL);
		(void)xuiWidgetSetFlowMode(pContent, XUI_FLOW_ABSOLUTE);
		(void)xuiWidgetSetPadding(pContent, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
		(void)xuiWidgetSetGap(pContent, 0.0f);
		return xuiWidgetAddChild(pContent, pChild);
	case UI_DESIGN_NODE_WINDOW:
		pContent = xuiWindowGetClientWidget(pParent);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		(void)xuiWidgetSetLayoutType(pContent, XUI_LAYOUT_MANUAL);
		(void)xuiWidgetSetFlowMode(pContent, XUI_FLOW_ABSOLUTE);
		(void)xuiWidgetSetPadding(pContent, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
		(void)xuiWidgetSetGap(pContent, 0.0f);
		return xuiWindowAddChild(pParent, pChild);
	case UI_DESIGN_NODE_SCROLL_VIEW:
		pContent = xuiScrollViewGetContentWidget(pParent);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		(void)xuiWidgetSetLayoutType(pContent, XUI_LAYOUT_MANUAL);
		(void)xuiWidgetSetFlowMode(pContent, XUI_FLOW_ABSOLUTE);
		(void)xuiWidgetSetPadding(pContent, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
		(void)xuiWidgetSetGap(pContent, 0.0f);
		iRet = xuiWidgetAddChild(pContent, pChild);
		return iRet;
	case UI_DESIGN_NODE_SCROLL_FRAME:
		pContent = xuiScrollFrameGetViewportWidget(pParent);
		if ( pContent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		(void)xuiWidgetSetLayoutType(pContent, XUI_LAYOUT_MANUAL);
		(void)xuiWidgetSetFlowMode(pContent, XUI_FLOW_ABSOLUTE);
		(void)xuiWidgetSetPadding(pContent, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
		(void)xuiWidgetSetGap(pContent, 0.0f);
		return xuiWidgetAddChild(pContent, pChild);
	case UI_DESIGN_NODE_CAROUSEL:
		iPage = __uiDesignAppCarouselPage(pParentNode, pChildNode);
		return xuiCarouselAddPageChild(pParent, iPage, pChild);
	case UI_DESIGN_NODE_SPLIT_LAYOUT:
		iPane = __uiDesignAppSplitPane(pParentNode, pChildNode, 0.0f, 0.0f);
		return xuiSplitLayoutAddPaneChild(pParent, iPane, pChild);
	case UI_DESIGN_NODE_TABS:
		iPage = __uiDesignAppTabsPage(pParentNode, pChildNode);
		return xuiTabsAddPageChild(pParent, iPage, pChild);
	case UI_DESIGN_NODE_ACCORDION:
		iSection = __uiDesignAppAccordionSection(pParentNode, pChildNode);
		return xuiAccordionAddSectionChild(pParent, iSection, pChild);
	default:
		return xuiWidgetAddChild(pParent, pChild);
	}
}

int uiDesignAppCreateNodeWidget(ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	const ui_design_control_desc_t* pDesc;
	ui_design_node_t* pParentNode;
	xui_widget pParent;
	int iRet;

	if ( (pApp == NULL) || (pNode == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pNode->pWidget != NULL ) return uiDesignAppSyncNodeWidget(pApp, pNode);
	pDesc = uiDesignRegistryFind(pNode->iType);
	if ( (pDesc == NULL) || (pDesc->onCreate == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pParentNode = NULL;
	if ( pNode->iParentId == 0 ) {
		pParent = pApp->pArtboard;
	} else {
		pParentNode = uiDesignModelGetNode(&pApp->tModel, pNode->iParentId);
		pParent = (pParentNode != NULL) ? pParentNode->pWidget : NULL;
	}
	if ( pParent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = pDesc->onCreate(pApp, pNode, &pNode->pWidget);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __uiDesignAppAttachNodeWidget(pParentNode, pNode, pParent, pNode->pWidget);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pNode->pWidget);
		pNode->pWidget = NULL;
		return iRet;
	}
	return uiDesignAppSyncNodeWidget(pApp, pNode);
}

int uiDesignAppSyncNodeWidget(ui_design_app_t* pApp, ui_design_node_t* pNode)
{
	const ui_design_control_desc_t* pDesc;

	if ( (pApp == NULL) || (pNode == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pNode->pWidget == NULL ) return uiDesignAppCreateNodeWidget(pApp, pNode);
	pDesc = uiDesignRegistryFind(pNode->iType);
	if ( (pDesc == NULL) || (pDesc->onApply == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	return pDesc->onApply(pApp, pNode);
}

int uiDesignAppAddNodeAt(ui_design_app_t* pApp, ui_design_node_type_t iType, float fDesignX, float fDesignY, int* pId)
{
	const ui_design_control_desc_t* pDesc;
	ui_design_node_t* pParentNode;
	ui_design_node_t* pNode;
	xui_rect_t tParentRect;
	float fW;
	float fH;
	float fX;
	float fY;
	char sPage[16];
	char sPane[16];
	char sSection[16];
	int iParentId;
	int iNewId;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pDesc = uiDesignRegistryFind(iType);
	if ( pDesc == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	fW = pDesc->fDefaultW;
	fH = pDesc->fDefaultH;
	iParentId = uiDesignModelFindDropParent(&pApp->tModel, fDesignX, fDesignY);
	fX = fDesignX - fW * 0.5f;
	fY = fDesignY - fH * 0.5f;
	if ( iParentId != 0 && uiDesignModelGetChildHostRect(&pApp->tModel, iParentId, &tParentRect) == XUI_OK ) {
		fX = fDesignX - tParentRect.fX - fW * 0.5f;
		fY = fDesignY - tParentRect.fY - fH * 0.5f;
	}
	if ( fX < 0.0f ) fX = 0.0f;
	if ( fY < 0.0f ) fY = 0.0f;
	iNewId = 0;
	iRet = uiDesignModelAddNode(&pApp->tModel, iType, iParentId, fX, fY, &iNewId);
	if ( iRet == XUI_OK ) {
		pNode = uiDesignModelGetNode(&pApp->tModel, iNewId);
		if ( pNode != NULL ) {
			pNode->tRect.fW = fW;
			pNode->tRect.fH = fH;
			iRet = uiDesignRegistryInitNodeProperties(pDesc, pNode);
			if ( iRet != XUI_OK ) return iRet;
			pParentNode = (iParentId != 0) ? uiDesignModelGetNode(&pApp->tModel, iParentId) : NULL;
			if ( (pParentNode != NULL) && (pParentNode->iType == UI_DESIGN_NODE_CAROUSEL) ) {
				snprintf(sPage, sizeof(sPage), "%d", __uiDesignAppCarouselPage(pParentNode, pNode));
				iRet = uiDesignModelSetProperty(&pApp->tModel, iNewId, "layout.carouselPage", sPage);
				if ( iRet != XUI_OK ) return iRet;
			} else if ( (pParentNode != NULL) && (pParentNode->iType == UI_DESIGN_NODE_SPLIT_LAYOUT) ) {
				snprintf(sPane, sizeof(sPane), "%d", __uiDesignAppSplitPane(pParentNode, pNode, fX + fW * 0.5f, fY + fH * 0.5f));
				iRet = uiDesignModelSetProperty(&pApp->tModel, iNewId, "layout.splitPane", sPane);
				if ( iRet != XUI_OK ) return iRet;
			} else if ( (pParentNode != NULL) && (pParentNode->iType == UI_DESIGN_NODE_TABS) ) {
				snprintf(sPage, sizeof(sPage), "%d", __uiDesignAppTabsPage(pParentNode, pNode));
				iRet = uiDesignModelSetProperty(&pApp->tModel, iNewId, "layout.tabPage", sPage);
				if ( iRet != XUI_OK ) return iRet;
			} else if ( (pParentNode != NULL) && (pParentNode->iType == UI_DESIGN_NODE_ACCORDION) ) {
				snprintf(sSection, sizeof(sSection), "%d", __uiDesignAppAccordionSection(pParentNode, pNode));
				iRet = uiDesignModelSetProperty(&pApp->tModel, iNewId, "layout.accordionSection", sSection);
				if ( iRet != XUI_OK ) return iRet;
			}
			iRet = uiDesignAppCreateNodeWidget(pApp, pNode);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( pId != NULL ) *pId = iNewId;
		pApp->iActiveTool = UI_DESIGN_NODE_NONE;
		pApp->iEditingProperty = UI_DESIGN_PROPERTY_NONE;
		(void)__uiDesignAppRefreshInspector(pApp);
		uiDesignAppInvalidate(pApp);
	}
	return iRet;
}

int uiDesignAppSetNodeRect(ui_design_app_t* pApp, int iId, xui_rect_t tRect)
{
	ui_design_node_t* pNode;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = uiDesignModelSetRect(&pApp->tModel, iId, tRect);
	if ( iRet != XUI_OK ) return iRet;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pNode);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

int uiDesignAppSetNodeText(ui_design_app_t* pApp, int iId, const char* sText)
{
	ui_design_node_t* pNode;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = uiDesignModelSetText(&pApp->tModel, iId, sText);
	if ( iRet != XUI_OK ) return iRet;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pNode);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

int uiDesignAppSetNodeChecked(ui_design_app_t* pApp, int iId, int bChecked)
{
	ui_design_node_t* pNode;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = uiDesignModelSetChecked(&pApp->tModel, iId, bChecked);
	if ( iRet != XUI_OK ) return iRet;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pNode);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

int uiDesignAppSetNodeVisible(ui_design_app_t* pApp, int iId, int bVisible)
{
	ui_design_node_t* pNode;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = uiDesignModelSetVisible(&pApp->tModel, iId, bVisible);
	if ( iRet != XUI_OK ) return iRet;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pNode);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

int uiDesignAppSetNodeEnabled(ui_design_app_t* pApp, int iId, int bEnabled)
{
	ui_design_node_t* pNode;
	int iRet;

	if ( pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = uiDesignModelSetEnabled(&pApp->tModel, iId, bEnabled);
	if ( iRet != XUI_OK ) return iRet;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pNode);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

int uiDesignAppSetNodeProperty(ui_design_app_t* pApp, int iId, const char* sPropertyId, const char* sValue)
{
	ui_design_node_t* pParentNode;
	ui_design_node_t* pNode;
	int iRet;
	int bChecked;

	if ( (pApp == NULL) || (sPropertyId == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = uiDesignModelSetProperty(&pApp->tModel, iId, sPropertyId, sValue);
	if ( iRet != XUI_OK ) return iRet;
	pNode = uiDesignModelGetNode(&pApp->tModel, iId);
	if ( pNode == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( strcmp(sPropertyId, "checked") == 0 ) {
		bChecked = uiDesignNodeGetPropertyBool(pNode, "checked", pNode->bChecked);
		(void)uiDesignModelSetChecked(&pApp->tModel, iId, bChecked);
	}
	if ( (strcmp(sPropertyId, "layout.carouselPage") == 0 ||
	      strcmp(sPropertyId, "layout.splitPane") == 0 ||
	      strcmp(sPropertyId, "layout.tabPage") == 0 ||
	      strcmp(sPropertyId, "layout.accordionSection") == 0) &&
	     pNode->pWidget != NULL && pNode->iParentId != 0 ) {
		pParentNode = uiDesignModelGetNode(&pApp->tModel, pNode->iParentId);
		if ( (pParentNode != NULL) && (pParentNode->pWidget != NULL) ) {
			iRet = __uiDesignAppAttachNodeWidget(pParentNode, pNode, pParentNode->pWidget, pNode->pWidget);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	if ( pNode != NULL ) (void)uiDesignAppSyncNodeWidget(pApp, pNode);
	(void)__uiDesignAppRefreshInspector(pApp);
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

static int __uiDesignRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	ui_design_app_t* pApp;
	xui_rect_t tRect;

	(void)iStateId;
	pApp = (ui_design_app_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pApp == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	return pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tRect, XUI_COLOR_RGBA(229, 236, 245, 255));
}

static int __uiDesignCreateRoot(ui_design_app_t* pApp)
{
	xui_cache_policy_t tPolicy;
	int iRet;

	iRet = xuiWidgetCreate(pApp->pContext, &pApp->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tPolicy, 0, sizeof(tPolicy));
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	(void)xuiWidgetSetLayoutType(pApp->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetRect(pApp->pRoot, (xui_rect_t){0.0f, 0.0f, (float)UI_DESIGN_TARGET_W, (float)UI_DESIGN_TARGET_H});
	(void)xuiWidgetSetCachePolicy(pApp->pRoot, &tPolicy);
	(void)xuiWidgetSetCacheRenderCallback(pApp->pRoot, __uiDesignRootRender, pApp);
	return xuiSetRootWidget(pApp->pContext, pApp->pRoot);
}

static int __uiDesignCreateDock(ui_design_app_t* pApp)
{
	xui_dock_panel_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pApp->pFont;
	iRet = xuiDockPanelCreate(pApp->pContext, &pApp->pDock, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pApp->pDock, (xui_rect_t){8.0f, 8.0f, (float)UI_DESIGN_TARGET_W - 16.0f, (float)UI_DESIGN_TARGET_H - 16.0f});
	iRet = xuiWidgetAddChild(pApp->pRoot, pApp->pDock);
	if ( iRet != XUI_OK ) return iRet;
	iRet = uiDesignToolboxCreate(pApp);
	if ( iRet == XUI_OK ) iRet = uiDesignCanvasCreate(pApp);
	if ( iRet == XUI_OK ) iRet = uiDesignInspectorCreate(pApp);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDockPanelAddWindow(pApp->pDock, "Toolbox", pApp->pToolbox, &pApp->iToolboxWindow);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pApp->pDock, "Designer", pApp->pCanvas, &pApp->iCanvasWindow);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelAddWindow(pApp->pDock, "Inspector", pApp->pInspector, &pApp->iInspectorWindow);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDockPanelDockWindow(pApp->pDock, pApp->iCanvasWindow, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &pApp->iCanvasPane);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindow(pApp->pDock, pApp->iToolboxWindow, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_LEFT, 0.20f, NULL);
	if ( iRet == XUI_OK ) iRet = xuiDockPanelDockWindow(pApp->pDock, pApp->iInspectorWindow, XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_RIGHT, 0.28f, NULL);
	return iRet;
}

static int __uiDesignSeedExercise(ui_design_app_t* pApp)
{
	const ui_design_control_desc_t* pDesc;
	int iWidget;
	int iButton;
	int iCheck;
	int iCarousel;
	int iSplit;
	int iTabs;
	int iAccordion;
	int iNested;
	int iCarouselChild;
	int iSplitChild;
	int iTabsChild;
	int iAccordionChild;
	int iId;
	int i;
	int iCol;
	int iRow;
	int iCreateRet;
	float fX;
	float fY;

	if ( (pApp == NULL) || pApp->bExerciseSeeded ) return XUI_OK;
	(void)uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_WIDGET, 0, 120.0f, 90.0f, &iWidget);
	(void)uiDesignModelSetRect(&pApp->tModel, iWidget, (xui_rect_t){120.0f, 90.0f, 260.0f, 170.0f});
	(void)uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_BUTTON, iWidget, 28.0f, 44.0f, &iButton);
	(void)uiDesignModelSetText(&pApp->tModel, iButton, "Run");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "common.tooltipText", "Run script");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "common.tabStop", "true");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "common.tabIndex", "2");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "layout.minWidth", "80");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "layout.maxWidth", "240");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "layout.tableCellRow", "0");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "layout.tableCellColumn", "0");
	(void)uiDesignModelSetProperty(&pApp->tModel, iButton, "style.classes", "primary accent");
	(void)uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_CHECKBOX, iWidget, 28.0f, 92.0f, &iCheck);
	(void)uiDesignModelSetText(&pApp->tModel, iCheck, "Enabled");
	(void)uiDesignModelSetChecked(&pApp->tModel, iCheck, 1);
	iCarousel = 0;
	iSplit = 0;
	iTabs = 0;
	iAccordion = 0;
	iCarouselChild = 0;
	iSplitChild = 0;
	iTabsChild = 0;
	iAccordionChild = 0;
	iCol = 0;
	iRow = 0;
	for ( i = 0; i < uiDesignRegistryGetCount(); i++ ) {
		pDesc = uiDesignRegistryGetAt(i);
		if ( pDesc == NULL ) continue;
		if ( pDesc->iType == UI_DESIGN_NODE_WIDGET || pDesc->iType == UI_DESIGN_NODE_BUTTON || pDesc->iType == UI_DESIGN_NODE_CHECKBOX ) continue;
		fX = 420.0f + (float)iCol * 170.0f;
		fY = 36.0f + (float)iRow * 112.0f;
		if ( uiDesignModelAddNode(&pApp->tModel, pDesc->iType, 0, fX, fY, &iId) == XUI_OK ) {
			ui_design_node_t* pNode = uiDesignModelGetNode(&pApp->tModel, iId);
			if ( pNode != NULL ) {
				pNode->tRect.fW = pDesc->fDefaultW;
				pNode->tRect.fH = pDesc->fDefaultH;
				(void)uiDesignRegistryInitNodeProperties(pDesc, pNode);
				if ( pDesc->iType == UI_DESIGN_NODE_CAROUSEL ) iCarousel = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_SPLIT_LAYOUT ) iSplit = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_TABS ) iTabs = iId;
				else if ( pDesc->iType == UI_DESIGN_NODE_ACCORDION ) iAccordion = iId;
			}
		}
		iCol++;
		if ( iCol >= 3 ) {
			iCol = 0;
			iRow++;
		}
	}
	if ( iCarousel != 0 && uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_LABEL, iCarousel, 14.0f, 18.0f, &iNested) == XUI_OK ) {
		iCarouselChild = iNested;
		(void)uiDesignModelSetText(&pApp->tModel, iNested, "Carousel child");
		(void)uiDesignModelSetProperty(&pApp->tModel, iNested, "layout.carouselPage", "0");
	}
	if ( iSplit != 0 && uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_LABEL, iSplit, 12.0f, 12.0f, &iNested) == XUI_OK ) {
		iSplitChild = iNested;
		(void)uiDesignModelSetText(&pApp->tModel, iNested, "Split child");
		(void)uiDesignModelSetProperty(&pApp->tModel, iNested, "layout.splitPane", "1");
	}
	if ( iTabs != 0 && uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_BUTTON, iTabs, 18.0f, 18.0f, &iNested) == XUI_OK ) {
		iTabsChild = iNested;
		(void)uiDesignModelSetText(&pApp->tModel, iNested, "Tab child");
		(void)uiDesignModelSetProperty(&pApp->tModel, iNested, "layout.tabPage", "1");
	}
	if ( iAccordion != 0 && uiDesignModelAddNode(&pApp->tModel, UI_DESIGN_NODE_CHECKBOX, iAccordion, 12.0f, 8.0f, &iNested) == XUI_OK ) {
		iAccordionChild = iNested;
		(void)uiDesignModelSetText(&pApp->tModel, iNested, "Section child");
		(void)uiDesignModelSetProperty(&pApp->tModel, iNested, "layout.accordionSection", "1");
	}
	(void)uiDesignModelSetSelected(&pApp->tModel, iButton);
	for ( i = 0; i < pApp->tModel.iNodeCount; i++ ) {
		pDesc = uiDesignRegistryFind(pApp->tModel.arrNodes[i].iType);
		if ( pDesc != NULL ) {
			(void)uiDesignRegistryInitNodeProperties(pDesc, &pApp->tModel.arrNodes[i]);
		}
		iCreateRet = uiDesignAppCreateNodeWidget(pApp, &pApp->tModel.arrNodes[i]);
		if ( iCreateRet != XUI_OK ) {
			printf("xui_uidesign exercise-create-failed type=%s id=%d ret=%d\n",
				uiDesignNodeTypeName(pApp->tModel.arrNodes[i].iType),
				pApp->tModel.arrNodes[i].iId,
				iCreateRet);
		}
	}
	if ( iSplit != 0 ) (void)uiDesignAppSetNodeProperty(pApp, iSplit, "data.paneCount", "3");
	if ( iTabs != 0 ) (void)uiDesignAppSetNodeProperty(pApp, iTabs, "data.selected", "1");
	if ( iAccordion != 0 ) (void)uiDesignAppSetNodeProperty(pApp, iAccordion, "behavior.mode", "1");
	if ( (iCarouselChild != 0) && !uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iCarouselChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=carousel child=%d\n", iCarouselChild);
		return XUI_ERROR;
	}
	if ( (iSplitChild != 0) && uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iSplitChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=split_layout child=%d\n", iSplitChild);
		return XUI_ERROR;
	}
	if ( (iTabsChild != 0) && uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iTabsChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=tabs child=%d\n", iTabsChild);
		return XUI_ERROR;
	}
	if ( (iAccordionChild != 0) && uiDesignModelCanFreeTransformNode(&pApp->tModel, uiDesignModelGetNode(&pApp->tModel, iAccordionChild)) ) {
		printf("xui_uidesign exercise-layout-transform-failed type=accordion child=%d\n", iAccordionChild);
		return XUI_ERROR;
	}
	iCreateRet = uiDesignInspectorExerciseComplexEditors(pApp);
	if ( iCreateRet != XUI_OK ) {
		printf("xui_uidesign exercise-complex-editor-failed ret=%d\n", iCreateRet);
		return iCreateRet;
	}
	(void)uiDesignModelSetSelected(&pApp->tModel, iButton);
	(void)uiDesignInspectorRefresh(pApp);
	pApp->bExerciseSeeded = 1;
	uiDesignAppInvalidate(pApp);
	return XUI_OK;
}

static int __uiDesignCreateAssets(ui_design_app_t* pApp)
{
	xui_surface_desc_t tSurface;
	const char* sFontPath;
	int iRet;

	pApp->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pApp->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pApp->pContext, &pApp->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pApp->pContext, (float)UI_DESIGN_TARGET_W, (float)UI_DESIGN_TARGET_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurface, 0, sizeof(tSurface));
	tSurface.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurface.iWidth = UI_DESIGN_TARGET_W;
	tSurface.iHeight = UI_DESIGN_TARGET_H;
	tSurface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurface.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pApp->tProxy.surfaceCreate(&pApp->tProxy, &pApp->pTarget, &tSurface);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __uiDesignFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pApp->tProxy.fontLoadFile(&pApp->tProxy, &pApp->pFont, sFontPath, 13.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pApp->pContext, pApp->pFont);
	uiDesignModelInit(&pApp->tModel);
	pApp->iActiveTool = UI_DESIGN_NODE_NONE;
	pApp->iDraggingTool = UI_DESIGN_NODE_NONE;
	iRet = __uiDesignCreateRoot(pApp);
	if ( iRet == XUI_OK ) iRet = __uiDesignCreateDock(pApp);
	if ( iRet == XUI_OK && pApp->bExercise ) iRet = __uiDesignSeedExercise(pApp);
	return iRet;
}

static void __uiDesignDestroyAssets(ui_design_app_t* pApp)
{
	int i;
	int j;

	if ( pApp != NULL ) {
		for ( i = 0; i < pApp->tModel.iNodeCount; i++ ) {
			if ( pApp->tModel.arrNodes[i].pRuntimeFont != NULL ) {
				pApp->tProxy.fontDestroy(&pApp->tProxy, pApp->tModel.arrNodes[i].pRuntimeFont);
				pApp->tModel.arrNodes[i].pRuntimeFont = NULL;
				pApp->tModel.arrNodes[i].sRuntimeFontSource[0] = '\0';
				pApp->tModel.arrNodes[i].fRuntimeFontSize = 0.0f;
				pApp->tModel.arrNodes[i].iRuntimeFontFlags = 0u;
			}
			if ( pApp->tModel.arrNodes[i].pRuntimeSurface != NULL ) {
				pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->tModel.arrNodes[i].pRuntimeSurface);
				pApp->tModel.arrNodes[i].pRuntimeSurface = NULL;
				pApp->tModel.arrNodes[i].sRuntimeSurfaceSource[0] = '\0';
			}
			for ( j = 0; j < UI_DESIGN_RUNTIME_SURFACE_COUNT; j++ ) {
				if ( pApp->tModel.arrNodes[i].arrRuntimeSurface[j] != NULL ) {
					pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->tModel.arrNodes[i].arrRuntimeSurface[j]);
					pApp->tModel.arrNodes[i].arrRuntimeSurface[j] = NULL;
					pApp->tModel.arrNodes[i].arrRuntimeSurfaceSource[j][0] = '\0';
				}
			}
		}
	}
	if ( pApp->pContext != NULL ) {
		xuiDestroy(pApp->pContext);
		pApp->pContext = NULL;
	}
	if ( pApp->pFont != NULL ) {
		pApp->tProxy.fontDestroy(&pApp->tProxy, pApp->pFont);
		pApp->pFont = NULL;
	}
	if ( pApp->pTarget != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pTarget);
		pApp->pTarget = NULL;
	}
}

static uint32_t __uiDesignReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static uint32_t __uiDesignReadModifiers(void)
{
	uint32_t iModifiers;

	iModifiers = 0;
	if ( xgeKeyDown(UI_DESIGN_KEY_LEFT_SHIFT) || xgeKeyDown(UI_DESIGN_KEY_RIGHT_SHIFT) ) iModifiers |= XUI_MOD_SHIFT;
	if ( xgeKeyDown(UI_DESIGN_KEY_LEFT_CTRL) || xgeKeyDown(UI_DESIGN_KEY_RIGHT_CTRL) ) iModifiers |= XUI_MOD_CTRL;
	if ( xgeKeyDown(UI_DESIGN_KEY_LEFT_ALT) || xgeKeyDown(UI_DESIGN_KEY_RIGHT_ALT) ) iModifiers |= XUI_MOD_ALT;
	return iModifiers;
}

static int __uiDesignMapKey(int iKey)
{
	switch ( iKey ) {
	case XGE_KEY_ENTER: return XUI_KEY_ENTER;
	case XGE_KEY_TAB: return XUI_KEY_TAB;
	case XGE_KEY_SPACE: return XUI_KEY_SPACE;
	case XGE_KEY_BACKSPACE: return XUI_KEY_BACKSPACE;
	case XGE_KEY_DELETE: return XUI_KEY_DELETE;
	case XGE_KEY_LEFT: return XUI_KEY_LEFT;
	case XGE_KEY_RIGHT: return XUI_KEY_RIGHT;
	case XGE_KEY_UP: return XUI_KEY_UP;
	case XGE_KEY_DOWN: return XUI_KEY_DOWN;
	case XGE_KEY_PAGE_UP: return XUI_KEY_PAGE_UP;
	case XGE_KEY_PAGE_DOWN: return XUI_KEY_PAGE_DOWN;
	case XGE_KEY_HOME: return XUI_KEY_HOME;
	case XGE_KEY_END: return XUI_KEY_END;
	case XGE_KEY_ESCAPE: return XUI_KEY_ESCAPE;
	default: return 0;
	}
}

static int __uiDesignSendKeys(ui_design_app_t* pApp)
{
	static const int arrKeys[] = {
		XGE_KEY_ENTER,
		XGE_KEY_TAB,
		XGE_KEY_SPACE,
		XGE_KEY_BACKSPACE,
		XGE_KEY_DELETE,
		XGE_KEY_LEFT,
		XGE_KEY_RIGHT,
		XGE_KEY_UP,
		XGE_KEY_DOWN,
		XGE_KEY_PAGE_UP,
		XGE_KEY_PAGE_DOWN,
		XGE_KEY_HOME,
		XGE_KEY_END,
		XGE_KEY_ESCAPE
	};
	uint32_t iModifiers;
	uint32_t iResult;
	uint32_t iText;
	int iKey;
	int iRet;
	int i;

	iModifiers = __uiDesignReadModifiers();
	iRet = xuiInputSetModifiers(pApp->pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < (int)(sizeof(arrKeys) / sizeof(arrKeys[0])); i++ ) {
		iKey = __uiDesignMapKey(arrKeys[i]);
		if ( iKey == 0 ) iKey = arrKeys[i];
		if ( xgeKeyPressed(arrKeys[i]) ) {
			iResult = 0u;
			iRet = xuiInputKeyDownEx(pApp->pContext, iKey, iModifiers, &iResult);
			if ( iRet != XUI_OK ) return iRet;
			if ( (iResult & XUI_INPUT_RESULT_CONSUMED) != 0u ) xgeInputConsumeKey(arrKeys[i]);
			if ( arrKeys[i] == XGE_KEY_ESCAPE && (iResult & XUI_INPUT_RESULT_CONSUMED) == 0u && pApp->iMaxFrames <= 0 ) xgeQuit();
		}
		if ( xgeKeyReleased(arrKeys[i]) ) {
			iResult = 0u;
			iRet = xuiInputKeyUpEx(pApp->pContext, iKey, iModifiers, &iResult);
			if ( iRet != XUI_OK ) return iRet;
			if ( (iResult & XUI_INPUT_RESULT_CONSUMED) != 0u ) xgeInputConsumeKey(arrKeys[i]);
		}
	}
	while ( (iText = xgeTextGet()) != 0 ) {
		if ( (iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT)) == 0u ) {
			iRet = xuiInputTextEx(pApp->pContext, iText, NULL);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __uiDesignHandleInput(ui_design_app_t* pApp)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	iButtons = __uiDesignReadButtons();
	if ( !pApp->bHasMouse || pApp->fLastMouseX != fX || pApp->fLastMouseY != fY || pApp->iLastButtons != iButtons ) {
		iRet = xuiInputPointerMove(pApp->pContext, fX, fY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pApp->pContext, fX, fY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pApp->iLastButtons;
	iReleased = pApp->iLastButtons & ~iButtons;
	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pApp->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pApp->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __uiDesignSendKeys(pApp);
	if ( iRet != XUI_OK ) return iRet;
	pApp->bHasMouse = 1;
	pApp->fLastMouseX = fX;
	pApp->fLastMouseY = fY;
	pApp->iLastButtons = iButtons;
	return XUI_OK;
}

static int __uiDesignFrameFail(const char* sStage, int iRet)
{
	printf("xui_uidesign frame failed at %s: %d\n", sStage != NULL ? sStage : "unknown", iRet);
	return iRet;
}

static void __uiDesignRunChecks(ui_design_app_t* pApp)
{
	xui_dock_pane_info_t tPane;
	int i;
	int bWidgetsOK;

	pApp->bCreateOK = (pApp->pDock != NULL) && (xuiDockPanelGetWindowCount(pApp->pDock) == 3);
	pApp->bCreateOK = pApp->bCreateOK && (pApp->pTree != NULL) && (pApp->pPropertyGrid != NULL) && (pApp->pArtboard != NULL) && (pApp->pOverlay != NULL);
	pApp->bLayoutOK = (xuiDockPanelGetPaneInfo(pApp->pDock, pApp->iCanvasPane, &tPane) == XUI_OK) && (tPane.tClientRect.fW > 200.0f) && (tPane.tClientRect.fH > 200.0f);
	bWidgetsOK = 1;
	for ( i = 0; i < pApp->tModel.iNodeCount; i++ ) {
		if ( pApp->tModel.arrNodes[i].pWidget == NULL ) {
			bWidgetsOK = 0;
			break;
		}
	}
	pApp->bModelOK = pApp->bExercise ? (pApp->tModel.iNodeCount >= 3 && pApp->tModel.iSelectedId > 0 && bWidgetsOK) : 1;
	pApp->bPaintOK = pApp->tModel.iRevision > 0u;
}

static int __uiDesignFrame(void* pUser)
{
	ui_design_app_t* pApp;
	xui_render_stats_t tStats;
	xui_rect_i_t tFull;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;

	pApp = (ui_design_app_t*)pUser;
	if ( pApp == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return __uiDesignFrameFail("xgeBegin", iRet);
	iRet = __uiDesignHandleInput(pApp);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("handleInput", iRet);
	iRet = xuiDispatchPendingEvents(pApp->pContext);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("dispatchEvents", iRet);
	iRet = xuiLayout(pApp->pContext);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("layout", iRet);
	iRet = xuiUpdate(pApp->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("update", iRet);
	__uiDesignRunChecks(pApp);
	iRet = pApp->tProxy.surfaceClear(&pApp->tProxy, pApp->pTarget, XUI_COLOR_RGBA(229, 236, 245, 255));
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("surfaceClear", iRet);
	tFull = (xui_rect_i_t){0, 0, UI_DESIGN_TARGET_W, UI_DESIGN_TARGET_H};
	iRet = xuiRender(pApp->pContext, pApp->pTarget, &tFull, 1);
	if ( iRet != XUI_OK ) return __uiDesignFrameFail("render", iRet);
	xgeClear(XUI_COLOR_RGBA(22, 28, 36, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)UI_DESIGN_TARGET_W, (float)UI_DESIGN_TARGET_H};
	tDst = tSrc;
	iRet = pApp->tProxy.surfaceDraw(&pApp->tProxy, pApp->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return __uiDesignFrameFail("present", iRet);
	pApp->iFrame++;
	if ( (pApp->iMaxFrames > 0 && pApp->iFrame >= pApp->iMaxFrames) ||
	     (pApp->fMaxSeconds > 0.0 && xgeTimer() >= pApp->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		(void)xuiGetRenderStats(pApp->pContext, &tStats);
		printf("xui_uidesign final-summary frames=%d create=%d layout=%d model=%d paint=%d nodes=%d selected=%d caches=%d/%d\n",
			pApp->iFrame,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bModelOK,
			pApp->bPaintOK,
			pApp->tModel.iNodeCount,
			pApp->tModel.iSelectedId,
			tStats.iUpdatedCaches,
			tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	ui_design_app_t* pApp;
	xge_desc_t tDesc;
	int iRet;
	int iExit;

	pApp = (ui_design_app_t*)calloc(1, sizeof(*pApp));
	if ( pApp == NULL ) {
		printf("xui_uidesign: out of memory\n");
		return 1;
	}
	iRet = __uiDesignParseArgs(pApp, argc, argv);
	if ( iRet == 1 ) {
		free(pApp);
		return 0;
	}
	if ( iRet != XGE_OK ) {
		__uiDesignUsage();
		free(pApp);
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = UI_DESIGN_TARGET_W;
	tDesc.iHeight = UI_DESIGN_TARGET_H;
	tDesc.sTitle = "XUI UI Design";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_uidesign: xgeInit failed: %d\n", iRet);
		free(pApp);
		return 1;
	}
	iRet = __uiDesignCreateAssets(pApp);
	if ( iRet != XUI_OK ) {
		printf("xui_uidesign: create assets failed: %d\n", iRet);
		__uiDesignDestroyAssets(pApp);
		xgeUnit();
		free(pApp);
		return 1;
	}
	iRet = xgeRun(__uiDesignFrame, pApp);
	iExit = (iRet == XGE_OK && pApp->bCreateOK && pApp->bLayoutOK && pApp->bModelOK && pApp->bPaintOK) ? 0 : 1;
	__uiDesignDestroyAssets(pApp);
	xgeUnit();
	free(pApp);
	return iExit;
}
