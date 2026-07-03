#include "mapedit_workspace.h"
#include "tileset_workspace.h"
#include "map_workspace.h"
#include "tabs/tileset_materials.h"
#include "tabs/tileset_arrange.h"
#include "tabs/tileset_sets.h"
#include "tabs/tileset_passage.h"
#include "tabs/tileset_actor_overlay.h"
#include "tabs/tileset_tags.h"
#include "tabs/map_maps.h"
#include "tabs/map_tile_select.h"
#include "tabs/map_edit.h"
#include "tabs/map_passage.h"
#include "tabs/map_tags.h"
#include <stdio.h>
#include <string.h>

static str MapEditWorkspaceMakeAppPath(const char* sFileName)
{
	if ( sFileName == NULL ) {
		return NULL;
	}
	return xrtPathJoin(2, xCore.AppPath, (str)sFileName);
}

static int MapEditWorkspaceFileExists(const char* sPath)
{
	FILE* fp;

	if ( sPath == NULL ) {
		return 0;
	}
	fp = fopen(sPath, "rb");
	if ( fp == NULL ) {
		return 0;
	}
	fclose(fp);
	return 1;
}

xge_xui_widget MapEditWorkspaceNewContent(mapedit_app_t* pApp)
{
	xge_xui_widget pWidget;

	if ( pApp->iContentCount >= MAPEDIT_CONTENT_COUNT ) {
		return NULL;
	}
	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return NULL;
	}
	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetGap(pWidget, 6.0f);
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	pApp->arrContent[pApp->iContentCount++] = pWidget;
	return pWidget;
}

int MapEditWorkspaceAddLabel(mapedit_app_t* pApp, xge_xui_widget pParent, const char* sText, uint32_t iColor)
{
	xge_xui_widget pWidget;
	xge_xui_label pLabel;

	if ( pApp->iLabelCount >= MAPEDIT_LABEL_COUNT ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(24.0f));
	pLabel = &pApp->arrLabel[pApp->iLabelCount++];
	if ( xgeXuiLabelInit(pLabel, pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(pLabel, iColor);
	xgeXuiLabelSetAlign(pLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	return xgeXuiWidgetAdd(pParent, pWidget);
}

static int MapEditWorkspaceCreatePlaceholder(mapedit_app_t* pApp, xge_xui_dock_window pWindow, const mapedit_window_desc_t* pDesc)
{
	xge_xui_widget pContent;
	char sInfo[192];

	pContent = MapEditWorkspaceNewContent(pApp);
	if ( pContent == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	snprintf(sInfo, sizeof(sInfo), "窗口ID: %s", pDesc->sId);
	if ( MapEditWorkspaceAddLabel(pApp, pContent, pDesc->sTitle, XGE_COLOR_RGBA(30, 74, 112, 255)) != XGE_OK ||
		MapEditWorkspaceAddLabel(pApp, pContent, pDesc->sBody, XGE_COLOR_RGBA(70, 82, 96, 255)) != XGE_OK ||
		MapEditWorkspaceAddLabel(pApp, pContent, sInfo, XGE_COLOR_RGBA(104, 118, 132, 255)) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDockWindowSetClientWidget(pWindow, pContent);
	return XGE_OK;
}

static int MapEditWorkspaceCreateTab(mapedit_app_t* pApp, xge_xui_dock_window pWindow, const mapedit_window_desc_t* pDesc)
{
	if ( strcmp(pDesc->sId, "tileset.materials") == 0 ) {
		return MapEditTilesetMaterialsCreate(pApp, pWindow);
	}
	if ( strcmp(pDesc->sId, "tileset.set") == 0 ) {
		return MapEditTilesetSetsCreate(pApp, pWindow);
	}
	if ( strcmp(pDesc->sId, "tileset.set_layout") == 0 ) {
		return MapEditTilesetArrangeCreate(pApp, pWindow);
	}
	if ( strcmp(pDesc->sId, "tileset.passage") == 0 ) {
		return MapEditTilesetPassageCreate(pApp, pWindow);
	}
	if ( strcmp(pDesc->sId, "tileset.tags") == 0 ) {
		return MapEditTilesetTagsCreate(pApp, pWindow);
	}
	if ( strcmp(pDesc->sId, "tileset.actor_overlay") == 0 ) {
		return MapEditTilesetActorOverlayCreate(pApp, pWindow);
	}
	if ( strcmp(pDesc->sId, "tileset.properties") == 0 ) {
		return MapEditTilesetPropertiesCreate(pApp, pWindow);
	}
	if ( strcmp(pDesc->sId, "map.list") == 0 ) {
		return MapEditMapListCreate(pApp, pWindow);
	}
	if ( strcmp(pDesc->sId, "map.tile_select") == 0 ) {
		return MapEditMapTileSelectCreate(pApp, pWindow);
	}
	if ( strcmp(pDesc->sId, "map.edit") == 0 ) {
		return MapEditMapEditCreate(pApp, pWindow);
	}
	if ( strcmp(pDesc->sId, "map.passage_adjust") == 0 ) {
		return MapEditMapPassageCreate(pApp, pWindow);
	}
	if ( strcmp(pDesc->sId, "map.tags") == 0 ) {
		return MapEditMapTagsCreate(pApp, pWindow);
	}
	if ( strcmp(pDesc->sId, "map.properties") == 0 ) {
		return MapEditMapPropertiesCreate(pApp, pWindow);
	}
	return MapEditWorkspaceCreatePlaceholder(pApp, pWindow, pDesc);
}

static int MapEditWorkspaceCreateWindow(mapedit_app_t* pApp, mapedit_workspace_t* pWorkspace, const mapedit_window_desc_t* pDesc, int iIndex)
{
	if ( (pApp == NULL) || (pWorkspace == NULL) || (pDesc == NULL) || (iIndex < 0) || (iIndex >= MAPEDIT_WORKSPACE_DOCK_WINDOW_MAX) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiDockWindowInit(&pWorkspace->arrDockWindow[iIndex], &pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pWorkspace->arrDockWindow[iIndex].pWindowWidget != NULL ) {
		xgeXuiWidgetSetName(pWorkspace->arrDockWindow[iIndex].pWindowWidget, pDesc->sId);
	}
	xgeXuiDockWindowSetTitle(&pWorkspace->arrDockWindow[iIndex], pDesc->sTitle);
	xgeXuiDockWindowSetClosable(&pWorkspace->arrDockWindow[iIndex], pDesc->bClosable);
	xgeXuiDockWindowSetDockable(&pWorkspace->arrDockWindow[iIndex], pDesc->bDockable);
	return MapEditWorkspaceCreateTab(pApp, &pWorkspace->arrDockWindow[iIndex], pDesc);
}

static int MapEditWorkspaceDockRegion(mapedit_workspace_t* pWorkspace, const mapedit_window_desc_t* pDesc, int iRegion, int iDockRegion, float fPortion)
{
	xge_xui_dock_pane pPane;
	int i;

	pPane = NULL;
	for ( i = 0; i < pWorkspace->iWindowCount; i++ ) {
		if ( pDesc[i].iRegion != iRegion ) {
			continue;
		}
		if ( pPane == NULL ) {
			pPane = xgeXuiDockLayoutDockWindow(&pWorkspace->tDockLayout, &pWorkspace->arrDockWindow[i], iDockRegion, XGE_XUI_DOCK_SIDE_FILL, fPortion);
			if ( pPane == NULL ) {
				return XGE_ERROR;
			}
		} else if ( xgeXuiDockLayoutDockWindow(&pWorkspace->tDockLayout, &pWorkspace->arrDockWindow[i], iDockRegion, XGE_XUI_DOCK_SIDE_FILL, 0.0f) != pPane ) {
			return XGE_ERROR;
		}
	}
	if ( pPane != NULL ) {
		xgeXuiDockPaneSetActiveIndex(pPane, 0);
	}
	return XGE_OK;
}

static int MapEditWorkspaceCreateDefaultDock(mapedit_workspace_t* pWorkspace, const mapedit_window_desc_t* pDesc)
{
	xgeXuiDockLayoutSetRegionPixelSize(&pWorkspace->tDockLayout, XGE_XUI_DOCK_REGION_LEFT, 320.0f);
	xgeXuiDockLayoutSetRegionPixelSize(&pWorkspace->tDockLayout, XGE_XUI_DOCK_REGION_RIGHT, 280.0f);
	if ( MapEditWorkspaceDockRegion(pWorkspace, pDesc, MAPEDIT_REGION_DOCUMENT, XGE_XUI_DOCK_REGION_DOCUMENT, 0.0f) != XGE_OK ||
		MapEditWorkspaceDockRegion(pWorkspace, pDesc, MAPEDIT_REGION_LEFT, XGE_XUI_DOCK_REGION_LEFT, 0.0f) != XGE_OK ||
		MapEditWorkspaceDockRegion(pWorkspace, pDesc, MAPEDIT_REGION_RIGHT, XGE_XUI_DOCK_REGION_RIGHT, 0.0f) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void MapEditWorkspaceInitDef(mapedit_workspace_t* pWorkspace, const mapedit_workspace_def_t* pDef)
{
	memset(pWorkspace, 0, sizeof(*pWorkspace));
	if ( pDef == NULL ) {
		return;
	}
	pWorkspace->iType = pDef->iType;
	pWorkspace->sName = pDef->sName;
	pWorkspace->sLayoutFile = pDef->sLayoutFile;
	pWorkspace->sDefaultLayoutFile = pDef->sDefaultLayoutFile;
	pWorkspace->pDef = pDef;
	pWorkspace->iWindowCount = pDef->iWindowCount;
}

static int MapEditWorkspaceCreate(mapedit_app_t* pApp, mapedit_workspace_t* pWorkspace, xge_xui_widget pClientStack)
{
	const mapedit_window_desc_t* pDesc;
	int i;

	if ( (pApp == NULL) || (pWorkspace == NULL) || (pClientStack == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pWorkspace->pDef == NULL) || (pWorkspace->pDef->pWindows == NULL) || (pWorkspace->iWindowCount <= 0) ||
		(pWorkspace->iWindowCount > MAPEDIT_WORKSPACE_DOCK_WINDOW_MAX) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pDesc = pWorkspace->pDef->pWindows;
	pWorkspace->pPageWidget = xgeXuiWidgetCreate();
	pWorkspace->pDockWidget = xgeXuiWidgetCreate();
	if ( (pWorkspace->pPageWidget == NULL) || (pWorkspace->pDockWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetLayout(pWorkspace->pPageWidget, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pWorkspace->pPageWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePercent(100.0f));
	xgeXuiWidgetSetPaddingPx(pWorkspace->pPageWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetBackground(pWorkspace->pPageWidget, XGE_COLOR_RGBA(226, 235, 242, 255));
	xgeXuiWidgetSetVisible(pWorkspace->pPageWidget, pWorkspace->iType == MAPEDIT_WORKSPACE_TILESET);
	xgeXuiWidgetSetSize(pWorkspace->pDockWidget, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetMinSize(pWorkspace->pDockWidget, xgeXuiSizePx(760.0f), xgeXuiSizePx(420.0f));
	xgeXuiWidgetSetPaddingPx(pWorkspace->pDockWidget, 4.0f, 4.0f, 4.0f, 4.0f);
	if ( xgeXuiWidgetAdd(pClientStack, pWorkspace->pPageWidget) != XGE_OK ||
		xgeXuiWidgetAdd(pWorkspace->pPageWidget, pWorkspace->pDockWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiDockLayoutInit(&pWorkspace->tDockLayout, &pApp->tXui, pWorkspace->pDockWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	pWorkspace->bDockReady = 1;
	for ( i = 0; i < pWorkspace->iWindowCount; i++ ) {
		if ( MapEditWorkspaceCreateWindow(pApp, pWorkspace, &pDesc[i], i) != XGE_OK ) {
			return XGE_ERROR;
		}
	}
	return MapEditWorkspaceCreateDefaultDock(pWorkspace, pDesc);
}

int MapEditWorkspacesCreate(mapedit_app_t* pApp, xge_xui_widget pClientStack)
{
	if ( (pApp == NULL) || (pClientStack == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	MapEditWorkspaceInitDef(&pApp->arrWorkspace[MAPEDIT_WORKSPACE_TILESET], MapEditTilesetWorkspaceDef());
	MapEditWorkspaceInitDef(&pApp->arrWorkspace[MAPEDIT_WORKSPACE_MAP], MapEditMapWorkspaceDef());
	if ( MapEditWorkspaceCreate(pApp, &pApp->arrWorkspace[MAPEDIT_WORKSPACE_TILESET], pClientStack) != XGE_OK ||
		MapEditWorkspaceCreate(pApp, &pApp->arrWorkspace[MAPEDIT_WORKSPACE_MAP], pClientStack) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->bDockReady = 1;
	return XGE_OK;
}

void MapEditWorkspacesUnit(mapedit_app_t* pApp)
{
	mapedit_workspace_t* pWorkspace;
	int i;
	int w;

	if ( pApp == NULL ) {
		return;
	}
	for ( w = 0; w < MAPEDIT_WORKSPACE_COUNT; w++ ) {
		pWorkspace = &pApp->arrWorkspace[w];
		for ( i = 0; i < pWorkspace->iWindowCount; i++ ) {
			xgeXuiDockWindowUnit(&pWorkspace->arrDockWindow[i]);
		}
		if ( pWorkspace->bDockReady ) {
			xgeXuiDockLayoutUnit(&pWorkspace->tDockLayout);
			pWorkspace->bDockReady = 0;
		}
	}
}

void MapEditWorkspacesSelect(mapedit_app_t* pApp, int iWorkspace)
{
	int i;

	if ( pApp == NULL ) {
		return;
	}
	for ( i = 0; i < MAPEDIT_WORKSPACE_COUNT; i++ ) {
		if ( pApp->arrWorkspace[i].pPageWidget != NULL ) {
			xgeXuiWidgetSetVisible(pApp->arrWorkspace[i].pPageWidget, i == iWorkspace);
		}
	}
}

static int MapEditWorkspaceLoadFile(mapedit_workspace_t* pWorkspace, const char* sFileName)
{
	str sPath;
	xvalue pState;
	int iRet;

	if ( (pWorkspace == NULL) || (pWorkspace->bDockReady == 0) || (sFileName == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sPath = MapEditWorkspaceMakeAppPath(sFileName);
	if ( sPath == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( !MapEditWorkspaceFileExists((const char*)sPath) ) {
		printf("mapedit workspace layout file not found: %s\n", sPath);
		xrtFree(sPath);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pState = xrtParseXSON_File(sPath);
	if ( pState == NULL ) {
		printf("mapedit workspace layout load failed: %s\n", sPath);
		xrtFree(sPath);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	iRet = xgeXuiDockLayoutLoadState(&pWorkspace->tDockLayout, pState);
	if ( iRet == XGE_OK ) {
		printf("mapedit workspace layout loaded: %s\n", sPath);
	} else {
		printf("mapedit workspace layout apply failed: %s\n", sPath);
	}
	xvoUnref(pState);
	xrtFree(sPath);
	return iRet;
}

static int MapEditWorkspaceSaveFile(mapedit_workspace_t* pWorkspace, const char* sFileName)
{
	str sPath;
	xvalue pState;
	int iRet;

	if ( (pWorkspace == NULL) || (pWorkspace->bDockReady == 0) || (sFileName == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pState = xgeXuiDockLayoutSaveState(&pWorkspace->tDockLayout);
	if ( pState == NULL ) {
		return XGE_ERROR;
	}
	sPath = MapEditWorkspaceMakeAppPath(sFileName);
	if ( sPath == NULL ) {
		xgeXuiDockLayoutStateFree(pState);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xrtStringifyXSON_File(sPath, pState, 1, 0);
	if ( iRet != 0 ) {
		printf("mapedit workspace layout saved: %s\n", sPath);
	} else {
		printf("mapedit workspace layout save failed: %s\n", sPath);
	}
	xrtFree(sPath);
	xgeXuiDockLayoutStateFree(pState);
	pWorkspace->bLayoutSaved = (iRet != 0);
	return pWorkspace->bLayoutSaved ? XGE_OK : XGE_ERROR;
}

int MapEditWorkspacesLoadUser(mapedit_app_t* pApp)
{
	int iRet0;
	int iRet1;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet0 = MapEditWorkspaceLoadFile(&pApp->arrWorkspace[MAPEDIT_WORKSPACE_TILESET], pApp->arrWorkspace[MAPEDIT_WORKSPACE_TILESET].sLayoutFile);
	iRet1 = MapEditWorkspaceLoadFile(&pApp->arrWorkspace[MAPEDIT_WORKSPACE_MAP], pApp->arrWorkspace[MAPEDIT_WORKSPACE_MAP].sLayoutFile);
	pApp->arrWorkspace[MAPEDIT_WORKSPACE_TILESET].bLayoutLoaded = (iRet0 == XGE_OK);
	pApp->arrWorkspace[MAPEDIT_WORKSPACE_MAP].bLayoutLoaded = (iRet1 == XGE_OK);
	pApp->bLayoutLoaded = (iRet0 == XGE_OK) || (iRet1 == XGE_OK);
	return pApp->bLayoutLoaded ? XGE_OK : XGE_ERROR_RESOURCE_FAILED;
}

int MapEditWorkspacesLoadDefault(mapedit_app_t* pApp)
{
	int iRet0;
	int iRet1;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet0 = MapEditWorkspaceLoadFile(&pApp->arrWorkspace[MAPEDIT_WORKSPACE_TILESET], pApp->arrWorkspace[MAPEDIT_WORKSPACE_TILESET].sDefaultLayoutFile);
	iRet1 = MapEditWorkspaceLoadFile(&pApp->arrWorkspace[MAPEDIT_WORKSPACE_MAP], pApp->arrWorkspace[MAPEDIT_WORKSPACE_MAP].sDefaultLayoutFile);
	pApp->bLayoutLoaded = (iRet0 == XGE_OK) || (iRet1 == XGE_OK);
	return pApp->bLayoutLoaded ? XGE_OK : XGE_ERROR_RESOURCE_FAILED;
}

int MapEditWorkspacesSaveUser(mapedit_app_t* pApp)
{
	int iRet0;
	int iRet1;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iRet0 = MapEditWorkspaceSaveFile(&pApp->arrWorkspace[MAPEDIT_WORKSPACE_TILESET], pApp->arrWorkspace[MAPEDIT_WORKSPACE_TILESET].sLayoutFile);
	iRet1 = MapEditWorkspaceSaveFile(&pApp->arrWorkspace[MAPEDIT_WORKSPACE_MAP], pApp->arrWorkspace[MAPEDIT_WORKSPACE_MAP].sLayoutFile);
	pApp->bLayoutSaved = (iRet0 == XGE_OK) && (iRet1 == XGE_OK);
	return pApp->bLayoutSaved ? XGE_OK : XGE_ERROR;
}

int MapEditWorkspacesReset(mapedit_app_t* pApp)
{
	return MapEditWorkspacesLoadDefault(pApp);
}
