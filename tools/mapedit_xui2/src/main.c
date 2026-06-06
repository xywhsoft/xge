#include "xui.h"
#include "xge.h"
#include "map_sdk/xge_map.h"
#include "mapedit_xui2_mapdoc.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAPEDIT_XUI2_W 1280
#define MAPEDIT_XUI2_H 800
#define MAPEDIT_XUI2_TOOLBAR_H 34.0f
#define MAPEDIT_XUI2_STATUS_H 28.0f
#define MAPEDIT_XUI2_CANVAS_W 2048.0f
#define MAPEDIT_XUI2_CANVAS_H 1536.0f
#define MAPEDIT_XUI2_PANEL_MAX 16
#define MAPEDIT_XUI2_ASSET_NAME_MAX 8
#define MAPEDIT_XUI2_ASSET_NAME_LEN 96

enum {
	MAPEDIT_XUI2_PANEL_ASSETS = 1,
	MAPEDIT_XUI2_PANEL_MAP_LIST,
	MAPEDIT_XUI2_PANEL_TILE_SELECT,
	MAPEDIT_XUI2_PANEL_INSPECTOR,
	MAPEDIT_XUI2_PANEL_OUTPUT,
	MAPEDIT_XUI2_PANEL_TILESET
};

typedef struct mapedit_xui2_panel_t {
	struct mapedit_xui2_app_t* pApp;
	const char* sTitle;
	const char* sLine1;
	const char* sLine2;
	uint32_t iAccent;
	int iKind;
} mapedit_xui2_panel_t;

typedef struct mapedit_xui2_app_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_surface pTilesheetPreview;
	xui_surface pAutotilePreview;
	xui_surface_desc_t tTilesheetPreviewDesc;
	xui_surface_desc_t tAutotilePreviewDesc;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pToolbar;
	xui_widget pDock;
	xui_widget pCanvas;
	xui_widget pStatus;
	xge_map_t tMap;
	mapedit_xui2_mapdoc_t tDoc;
	xui_widget arrPanelWidgets[MAPEDIT_XUI2_PANEL_MAX];
	mapedit_xui2_panel_t arrPanels[MAPEDIT_XUI2_PANEL_MAX];
	int arrWindowIds[MAPEDIT_XUI2_PANEL_MAX + 1];
	int iFrame;
	int iMaxFrames;
	int iTilesheetFiles;
	int iAutotileFiles;
	int iMapFiles;
	char arrTilesheetNames[MAPEDIT_XUI2_ASSET_NAME_MAX][MAPEDIT_XUI2_ASSET_NAME_LEN];
	char arrAutotileNames[MAPEDIT_XUI2_ASSET_NAME_MAX][MAPEDIT_XUI2_ASSET_NAME_LEN];
	char arrMapNames[MAPEDIT_XUI2_ASSET_NAME_MAX][MAPEDIT_XUI2_ASSET_NAME_LEN];
	int iTilesheetNameCount;
	int iAutotileNameCount;
	int iMapNameCount;
	int iToolbarSelects;
	int iSelectedTool;
	int iActiveWorkspace;
	int iStatusModeIndex;
	int bCommandExerciseDone;
	int iDocPane;
	int iLeftPane;
	int iRightPane;
	int bCreateOK;
	int bLayoutOK;
	int bCanvasOK;
	int bAssetsOK;
	int bPreviewOK;
	int bMapSdkOK;
	int bMapFileOK;
	int bMapSaveOK;
	int bTileEditOK;
	int bContentDrawn;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	uint32_t iLastButtons;
} mapedit_xui2_app_t;

static const char* __mapeditFindTtf(void)
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

static int __mapeditHasExt(const char* sName, const char* sExt)
{
	size_t nName;
	size_t nExt;
	if ( sName == NULL || sExt == NULL ) return 0;
	nName = strlen(sName);
	nExt = strlen(sExt);
	if ( nName < nExt ) return 0;
	return _stricmp(sName + nName - nExt, sExt) == 0;
}

static int __mapeditCountFiles(const char* sPath, const char* sExt)
{
	DIR* pDir;
	struct dirent* pEntry;
	int iCount;
	pDir = opendir(sPath);
	if ( pDir == NULL ) return 0;
	iCount = 0;
	while ( (pEntry = readdir(pDir)) != NULL ) {
		if ( __mapeditHasExt(pEntry->d_name, sExt) ) iCount++;
	}
	closedir(pDir);
	return iCount;
}

static int __mapeditScanNames(const char* sPath, const char* sExt, char arrNames[][MAPEDIT_XUI2_ASSET_NAME_LEN], int iMaxNames)
{
	DIR* pDir;
	struct dirent* pEntry;
	int iCount;
	pDir = opendir(sPath);
	if ( pDir == NULL ) return 0;
	iCount = 0;
	while ( (pEntry = readdir(pDir)) != NULL ) {
		if ( !__mapeditHasExt(pEntry->d_name, sExt) ) continue;
		if ( iCount < iMaxNames ) {
			snprintf(arrNames[iCount], MAPEDIT_XUI2_ASSET_NAME_LEN, "%s", pEntry->d_name);
		}
		iCount++;
	}
	closedir(pDir);
	return iCount;
}

static void __mapeditLoadAssetCounts(mapedit_xui2_app_t* pApp)
{
	pApp->iTilesheetFiles = __mapeditCountFiles("..\\mapedit\\release\\assets\\tilesheets", ".png");
	pApp->iAutotileFiles = __mapeditCountFiles("..\\mapedit\\release\\assets\\autotiles", ".png");
	pApp->iMapFiles = __mapeditCountFiles("..\\mapedit\\release\\assets\\maps", ".xson");
	pApp->iTilesheetNameCount = __mapeditScanNames("..\\mapedit\\release\\assets\\tilesheets", ".png", pApp->arrTilesheetNames, MAPEDIT_XUI2_ASSET_NAME_MAX);
	pApp->iAutotileNameCount = __mapeditScanNames("..\\mapedit\\release\\assets\\autotiles", ".png", pApp->arrAutotileNames, MAPEDIT_XUI2_ASSET_NAME_MAX);
	pApp->iMapNameCount = __mapeditScanNames("..\\mapedit\\release\\assets\\maps", ".xson", pApp->arrMapNames, MAPEDIT_XUI2_ASSET_NAME_MAX);
	if ( pApp->iTilesheetFiles == 0 && pApp->iAutotileFiles == 0 ) {
		pApp->iTilesheetFiles = __mapeditCountFiles("tools\\mapedit\\release\\assets\\tilesheets", ".png");
		pApp->iAutotileFiles = __mapeditCountFiles("tools\\mapedit\\release\\assets\\autotiles", ".png");
		pApp->iMapFiles = __mapeditCountFiles("tools\\mapedit\\release\\assets\\maps", ".xson");
		pApp->iTilesheetNameCount = __mapeditScanNames("tools\\mapedit\\release\\assets\\tilesheets", ".png", pApp->arrTilesheetNames, MAPEDIT_XUI2_ASSET_NAME_MAX);
		pApp->iAutotileNameCount = __mapeditScanNames("tools\\mapedit\\release\\assets\\autotiles", ".png", pApp->arrAutotileNames, MAPEDIT_XUI2_ASSET_NAME_MAX);
		pApp->iMapNameCount = __mapeditScanNames("tools\\mapedit\\release\\assets\\maps", ".xson", pApp->arrMapNames, MAPEDIT_XUI2_ASSET_NAME_MAX);
	}
	pApp->bAssetsOK = (pApp->iTilesheetFiles + pApp->iAutotileFiles + pApp->iMapFiles) > 0;
}

static int __mapeditTryLoadSurface(mapedit_xui2_app_t* pApp, const char* sPath, xui_surface* ppSurface, xui_surface_desc_t* pDesc)
{
	int ret;
	if ( pApp == NULL || sPath == NULL || ppSurface == NULL || pDesc == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	ret = pApp->tProxy.surfaceLoadFile(&pApp->tProxy, ppSurface, sPath, XUI_SURFACE_ALPHA_PREMULTIPLIED);
	if ( ret != XUI_OK ) return ret;
	memset(pDesc, 0, sizeof(*pDesc));
	(void)pApp->tProxy.surfaceGetDesc(&pApp->tProxy, *ppSurface, pDesc);
	return XUI_OK;
}

static void __mapeditLoadPreviewSurfaces(mapedit_xui2_app_t* pApp)
{
	char sPath[512];
	if ( pApp == NULL ) return;
	if ( pApp->iTilesheetNameCount > 0 ) {
		snprintf(sPath, sizeof(sPath), "..\\mapedit\\release\\assets\\tilesheets\\%s", pApp->arrTilesheetNames[0]);
		if ( __mapeditTryLoadSurface(pApp, sPath, &pApp->pTilesheetPreview, &pApp->tTilesheetPreviewDesc) != XUI_OK ) {
			snprintf(sPath, sizeof(sPath), "..\\..\\mapedit\\release\\assets\\tilesheets\\%s", pApp->arrTilesheetNames[0]);
			if ( __mapeditTryLoadSurface(pApp, sPath, &pApp->pTilesheetPreview, &pApp->tTilesheetPreviewDesc) != XUI_OK ) {
				snprintf(sPath, sizeof(sPath), "tools\\mapedit\\release\\assets\\tilesheets\\%s", pApp->arrTilesheetNames[0]);
				(void)__mapeditTryLoadSurface(pApp, sPath, &pApp->pTilesheetPreview, &pApp->tTilesheetPreviewDesc);
			}
		}
	}
	if ( pApp->iAutotileNameCount > 0 ) {
		snprintf(sPath, sizeof(sPath), "..\\mapedit\\release\\assets\\autotiles\\%s", pApp->arrAutotileNames[0]);
		if ( __mapeditTryLoadSurface(pApp, sPath, &pApp->pAutotilePreview, &pApp->tAutotilePreviewDesc) != XUI_OK ) {
			snprintf(sPath, sizeof(sPath), "..\\..\\mapedit\\release\\assets\\autotiles\\%s", pApp->arrAutotileNames[0]);
			if ( __mapeditTryLoadSurface(pApp, sPath, &pApp->pAutotilePreview, &pApp->tAutotilePreviewDesc) != XUI_OK ) {
				snprintf(sPath, sizeof(sPath), "tools\\mapedit\\release\\assets\\autotiles\\%s", pApp->arrAutotileNames[0]);
				(void)__mapeditTryLoadSurface(pApp, sPath, &pApp->pAutotilePreview, &pApp->tAutotilePreviewDesc);
			}
		}
	}
	pApp->bPreviewOK = (pApp->pTilesheetPreview != NULL) || (pApp->pAutotilePreview != NULL);
}

static void __mapeditLoadDefaultMapLite(mapedit_xui2_app_t* pApp)
{
	if ( pApp == NULL ) return;
	if ( mapeditXui2MapDocLoad(&pApp->tDoc, "..\\mapedit\\release\\assets\\maps\\default.xson") != XUI_OK &&
	     mapeditXui2MapDocLoad(&pApp->tDoc, "..\\..\\mapedit\\release\\assets\\maps\\default.xson") != XUI_OK &&
	     mapeditXui2MapDocLoad(&pApp->tDoc, "tools\\mapedit\\release\\assets\\maps\\default.xson") != XUI_OK ) {
		return;
	}
	pApp->tMap.sName = pApp->tDoc.sName;
	pApp->tMap.iTilesetId = 0;
	pApp->tMap.tSize.iWidth = pApp->tDoc.iWidth;
	pApp->tMap.tSize.iHeight = pApp->tDoc.iHeight;
	pApp->tMap.tSize.iLayerCount = pApp->tDoc.iLayerCount;
	pApp->bMapFileOK = 1;
	pApp->bMapSaveOK = (mapeditXui2MapDocSaveSmoke(&pApp->tDoc, "mapedit_xui2_smoke_saved_map.xson") == XUI_OK);
}

static int __mapeditRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	mapedit_xui2_app_t* pApp;
	xui_rect_t tRect;
	(void)iStateId;
	pApp = (mapedit_xui2_app_t*)pUser;
	if ( pWidget == NULL || pDraw == NULL || pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	return pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, tRect, XUI_COLOR_RGBA(230, 235, 241, 255));
}

static int __mapeditPanelRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	mapedit_xui2_panel_t* pPanel;
	mapedit_xui2_app_t* pApp;
	xui_rect_t r;
	char sText[192];
	float y;
	int i;
	int ret;
	(void)iStateId;
	pPanel = (mapedit_xui2_panel_t*)pUser;
	if ( pWidget == NULL || pDraw == NULL || pPanel == NULL || pPanel->pApp == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pApp = pPanel->pApp;
	r = xuiWidgetGetContentRect(pWidget);
	ret = pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, r, XUI_COLOR_RGBA(248, 250, 252, 255));
	if ( ret == XUI_OK ) ret = pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, (xui_rect_t){r.fX, r.fY, 5.0f, r.fH}, pPanel->iAccent);
	if ( ret == XUI_OK ) ret = pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, pPanel->sTitle, (xui_rect_t){r.fX + 16.0f, r.fY + 16.0f, r.fW - 24.0f, 24.0f}, XUI_COLOR_RGBA(35, 48, 64, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( ret == XUI_OK ) ret = pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, pPanel->sLine1, (xui_rect_t){r.fX + 16.0f, r.fY + 52.0f, r.fW - 24.0f, 22.0f}, XUI_COLOR_RGBA(74, 88, 106, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	if ( ret == XUI_OK ) ret = pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, pPanel->sLine2, (xui_rect_t){r.fX + 16.0f, r.fY + 82.0f, r.fW - 24.0f, 22.0f}, XUI_COLOR_RGBA(74, 88, 106, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	y = r.fY + 118.0f;
	if ( pPanel->iKind == MAPEDIT_XUI2_PANEL_ASSETS || pPanel->iKind == MAPEDIT_XUI2_PANEL_TILESET || pPanel->iKind == MAPEDIT_XUI2_PANEL_TILE_SELECT ) {
		if ( pApp->pTilesheetPreview != NULL && pApp->tTilesheetPreviewDesc.iWidth > 0 && pApp->tTilesheetPreviewDesc.iHeight > 0 ) {
			xui_rect_t src = {0.0f, 0.0f, (float)pApp->tTilesheetPreviewDesc.iWidth, (float)pApp->tTilesheetPreviewDesc.iHeight};
			xui_rect_t dst = {r.fX + 16.0f, y, 96.0f, 64.0f};
			if ( ret == XUI_OK ) ret = pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, (xui_rect_t){dst.fX - 1.0f, dst.fY - 1.0f, dst.fW + 2.0f, dst.fH + 2.0f}, XUI_COLOR_RGBA(220, 226, 234, 255));
			if ( ret == XUI_OK ) ret = pApp->tProxy.drawSurface(&pApp->tProxy, pDraw, pApp->pTilesheetPreview, src, dst, XUI_COLOR_WHITE, 0);
		}
		if ( pApp->pAutotilePreview != NULL && pApp->tAutotilePreviewDesc.iWidth > 0 && pApp->tAutotilePreviewDesc.iHeight > 0 ) {
			xui_rect_t src = {0.0f, 0.0f, (float)pApp->tAutotilePreviewDesc.iWidth, (float)pApp->tAutotilePreviewDesc.iHeight};
			xui_rect_t dst = {r.fX + 124.0f, y, 64.0f, 64.0f};
			if ( ret == XUI_OK ) ret = pApp->tProxy.drawRectFill(&pApp->tProxy, pDraw, (xui_rect_t){dst.fX - 1.0f, dst.fY - 1.0f, dst.fW + 2.0f, dst.fH + 2.0f}, XUI_COLOR_RGBA(220, 226, 234, 255));
			if ( ret == XUI_OK ) ret = pApp->tProxy.drawSurface(&pApp->tProxy, pDraw, pApp->pAutotilePreview, src, dst, XUI_COLOR_WHITE, 0);
		}
		y += 78.0f;
		snprintf(sText, sizeof(sText), "Tilesheets: %d", pApp->iTilesheetFiles);
		if ( ret == XUI_OK ) ret = pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText, (xui_rect_t){r.fX + 16.0f, y, r.fW - 24.0f, 20.0f}, XUI_COLOR_RGBA(45, 84, 122, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		y += 24.0f;
		for ( i = 0; ret == XUI_OK && i < pApp->iTilesheetNameCount && i < 3; i++, y += 22.0f ) {
			ret = pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, pApp->arrTilesheetNames[i], (xui_rect_t){r.fX + 24.0f, y, r.fW - 32.0f, 20.0f}, XUI_COLOR_RGBA(64, 78, 96, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		}
		snprintf(sText, sizeof(sText), "Autotiles: %d", pApp->iAutotileFiles);
		if ( ret == XUI_OK ) ret = pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText, (xui_rect_t){r.fX + 16.0f, y + 8.0f, r.fW - 24.0f, 20.0f}, XUI_COLOR_RGBA(45, 84, 122, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		y += 32.0f;
		for ( i = 0; ret == XUI_OK && i < pApp->iAutotileNameCount && i < 4; i++, y += 22.0f ) {
			ret = pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, pApp->arrAutotileNames[i], (xui_rect_t){r.fX + 24.0f, y, r.fW - 32.0f, 20.0f}, XUI_COLOR_RGBA(64, 78, 96, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		}
	} else if ( pPanel->iKind == MAPEDIT_XUI2_PANEL_MAP_LIST ) {
		snprintf(sText, sizeof(sText), "Maps: %d", pApp->iMapFiles);
		if ( ret == XUI_OK ) ret = pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText, (xui_rect_t){r.fX + 16.0f, y, r.fW - 24.0f, 20.0f}, XUI_COLOR_RGBA(45, 84, 122, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		y += 24.0f;
		for ( i = 0; ret == XUI_OK && i < pApp->iMapNameCount && i < MAPEDIT_XUI2_ASSET_NAME_MAX; i++, y += 22.0f ) {
			ret = pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, pApp->arrMapNames[i], (xui_rect_t){r.fX + 24.0f, y, r.fW - 32.0f, 20.0f}, XUI_COLOR_RGBA(64, 78, 96, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		}
	} else if ( pPanel->iKind == MAPEDIT_XUI2_PANEL_INSPECTOR ) {
		snprintf(sText, sizeof(sText), "Active layer: 0    Brush: Paint    Grid: On");
		if ( ret == XUI_OK ) ret = pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText, (xui_rect_t){r.fX + 16.0f, y, r.fW - 24.0f, 20.0f}, XUI_COLOR_RGBA(64, 78, 96, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		y += 24.0f;
		snprintf(sText, sizeof(sText), "SDK map: %dx%d layers=%d tile=%dx%d",
			pApp->tMap.tSize.iWidth, pApp->tMap.tSize.iHeight, pApp->tMap.tSize.iLayerCount,
			pApp->tMap.tSize.iTileWidth, pApp->tMap.tSize.iTileHeight);
		if ( ret == XUI_OK ) ret = pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText, (xui_rect_t){r.fX + 16.0f, y, r.fW - 24.0f, 20.0f}, XUI_COLOR_RGBA(64, 78, 96, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		y += 24.0f;
		snprintf(sText, sizeof(sText), "MapDoc tiles=%d nonzero=%d saved=%d edited=%d", pApp->tDoc.iTileCount, pApp->tDoc.iNonZeroTileCount, pApp->bMapSaveOK, pApp->bTileEditOK);
		if ( ret == XUI_OK ) ret = pApp->tProxy.drawText(&pApp->tProxy, pDraw, pApp->pFont, sText, (xui_rect_t){r.fX + 16.0f, y, r.fW - 24.0f, 20.0f}, XUI_COLOR_RGBA(64, 78, 96, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return ret;
}

static void __mapeditToolbarSelected(xui_widget pToolbar, int iIndex, int iValue, void* pUser)
{
	mapedit_xui2_app_t* pApp;
	char sText[96];
	(void)pToolbar;
	pApp = (mapedit_xui2_app_t*)pUser;
	if ( pApp == NULL ) return;
	pApp->iToolbarSelects++;
	if ( iValue >= 20 && iValue <= 23 ) {
		pApp->iSelectedTool = iValue;
		(void)xuiToolbarSetItemChecked(pApp->pToolbar, 4, iValue == 20);
		(void)xuiToolbarSetItemChecked(pApp->pToolbar, 5, iValue == 21);
		(void)xuiToolbarSetItemChecked(pApp->pToolbar, 6, iValue == 22);
		(void)xuiToolbarSetItemChecked(pApp->pToolbar, 7, iValue == 23);
		if ( iValue == 20 && pApp->tDoc.pTiles != NULL ) {
			pApp->bTileEditOK = (mapeditXui2MapDocSetTile(&pApp->tDoc, 0, 2, 2, 1) == XUI_OK);
			if ( pApp->bTileEditOK ) {
				pApp->bMapSaveOK = (mapeditXui2MapDocSaveSmoke(&pApp->tDoc, "mapedit_xui2_smoke_saved_map.xson") == XUI_OK);
				pApp->bContentDrawn = 0;
			}
		}
	}
	if ( iValue == 10 || iValue == 11 || iValue == 12 ) {
		pApp->iActiveWorkspace = 1;
	}
	snprintf(sText, sizeof(sText), "tool=%d command=%d", pApp->iSelectedTool, iValue);
	if ( pApp->pStatus != NULL && pApp->iStatusModeIndex >= 0 ) {
		(void)xuiStatusBarSetItemText(pApp->pStatus, pApp->iStatusModeIndex, sText);
	}
}

static int __mapeditCreatePanel(mapedit_xui2_app_t* pApp, int iIndex, int iKind, const char* sTitle, const char* sLine1, const char* sLine2, uint32_t iAccent)
{
	xui_widget pWidget;
	xui_cache_policy_t tPolicy;
	int ret;
	ret = xuiWidgetCreate(pApp->pContext, &pWidget);
	if ( ret != XUI_OK ) return ret;
	memset(&tPolicy, 0, sizeof(tPolicy));
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	pApp->arrPanels[iIndex].pApp = pApp;
	pApp->arrPanels[iIndex].sTitle = sTitle;
	pApp->arrPanels[iIndex].sLine1 = sLine1;
	pApp->arrPanels[iIndex].sLine2 = sLine2;
	pApp->arrPanels[iIndex].iAccent = iAccent;
	pApp->arrPanels[iIndex].iKind = iKind;
	(void)xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCachePolicy(pWidget, &tPolicy);
	(void)xuiWidgetSetCacheRenderCallback(pWidget, __mapeditPanelRender, &pApp->arrPanels[iIndex]);
	pApp->arrPanelWidgets[iIndex] = pWidget;
	return XUI_OK;
}

static int __mapeditDrawCanvas(mapedit_xui2_app_t* pApp)
{
	char sText[160];
	float x;
	float y;
	int ret;
	ret = xuiCanvasClear(pApp->pCanvas, XUI_COLOR_RGBA(252, 254, 255, 255));
	for ( x = 0.0f; ret == XUI_OK && x <= MAPEDIT_XUI2_CANVAS_W; x += 32.0f ) {
		ret = xuiCanvasDrawLine(pApp->pCanvas, x, 0.0f, x, MAPEDIT_XUI2_CANVAS_H, 1.0f, XUI_COLOR_RGBA(226, 232, 240, 255));
	}
	for ( y = 0.0f; ret == XUI_OK && y <= MAPEDIT_XUI2_CANVAS_H; y += 32.0f ) {
		ret = xuiCanvasDrawLine(pApp->pCanvas, 0.0f, y, MAPEDIT_XUI2_CANVAS_W, y, 1.0f, XUI_COLOR_RGBA(226, 232, 240, 255));
	}
	if ( ret == XUI_OK ) ret = xuiCanvasDrawRoundRectFill(pApp->pCanvas, (xui_rect_t){96.0f, 96.0f, 320.0f, 180.0f}, 6.0f, XUI_COLOR_RGBA(214, 232, 255, 255));
	if ( ret == XUI_OK ) ret = xuiCanvasDrawRoundRectStroke(pApp->pCanvas, (xui_rect_t){96.0f, 96.0f, 320.0f, 180.0f}, 6.0f, 2.0f, XUI_COLOR_RGBA(48, 112, 190, 255));
	snprintf(sText, sizeof(sText), "Map document: %dx%d layers=%d tiles=%d", pApp->tDoc.iWidth, pApp->tDoc.iHeight, pApp->tDoc.iLayerCount, pApp->tDoc.iTileCount);
	if ( ret == XUI_OK ) ret = xuiCanvasDrawText(pApp->pCanvas, pApp->pFont, sText, (xui_rect_t){112.0f, 128.0f, 288.0f, 28.0f}, XUI_COLOR_RGBA(36, 54, 78, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	snprintf(sText, sizeof(sText), "Loaded from XSON, smoke save=%d, edit=%d, nonzero tiles=%d", pApp->bMapSaveOK, pApp->bTileEditOK, pApp->tDoc.iNonZeroTileCount);
	if ( ret == XUI_OK ) ret = xuiCanvasDrawText(pApp->pCanvas, pApp->pFont, sText, (xui_rect_t){112.0f, 166.0f, 288.0f, 28.0f}, XUI_COLOR_RGBA(72, 88, 108, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	pApp->bCanvasOK = (ret == XUI_OK && xuiCanvasGetDrawCount(pApp->pCanvas) > 10);
	return ret;
}

static int __mapeditCreateToolbar(mapedit_xui2_app_t* pApp)
{
	xui_toolbar_item_t arrItems[8];
	xui_toolbar_desc_t tDesc;
	xui_toolbar_metrics_t tMetrics;
	int ret;
	memset(arrItems, 0, sizeof(arrItems));
	arrItems[0] = (xui_toolbar_item_t){"New", "New map", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 10, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[1] = (xui_toolbar_item_t){"Open", "Open map", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 11, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[2] = (xui_toolbar_item_t){"Save", "Save map", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 12, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[3] = (xui_toolbar_item_t){"", "", XUI_TOOLBAR_ITEM_SEPARATOR, 0, 0, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[4] = (xui_toolbar_item_t){"Paint", "Paint tiles", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED, 20, 1, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[5] = (xui_toolbar_item_t){"Erase", "Erase tiles", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED, 21, 1, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[6] = (xui_toolbar_item_t){"Pass", "Passage overlay", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED, 22, 2, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[7] = (xui_toolbar_item_t){"Tags", "Tag overlay", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED, 23, 2, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	memset(&tMetrics, 0, sizeof(tMetrics));
	tMetrics.iSize = sizeof(tMetrics);
	tMetrics.iOrientation = XUI_ORIENTATION_HORIZONTAL;
	tMetrics.fItemWidth = 64.0f;
	tMetrics.fItemHeight = 26.0f;
	tMetrics.fSeparatorSize = 12.0f;
	tMetrics.fGroupGap = 8.0f;
	tMetrics.fPaddingX = 6.0f;
	tMetrics.fPaddingY = 4.0f;
	tMetrics.fOverflowSize = 28.0f;
	tMetrics.fRadius = 4.0f;
	tMetrics.fBorderWidth = 1.0f;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pItems = arrItems;
	tDesc.iItemCount = 8;
	tDesc.pFont = pApp->pFont;
	tDesc.tMetrics = tMetrics;
	tDesc.bHasMetrics = 1;
	ret = xuiToolbarCreate(pApp->pContext, &pApp->pToolbar, &tDesc);
	if ( ret == XUI_OK ) ret = xuiToolbarSetSelect(pApp->pToolbar, __mapeditToolbarSelected, pApp);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(pApp->pRoot, pApp->pToolbar);
	return ret;
}

static int __mapeditCreateStatus(mapedit_xui2_app_t* pApp)
{
	xui_statusbar_desc_t tDesc;
	char sAssets[128];
	int ret;
	int iIndex;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pApp->pFont;
	ret = xuiStatusBarCreate(pApp->pContext, &pApp->pStatus, &tDesc);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(pApp->pRoot, pApp->pStatus);
	snprintf(sAssets, sizeof(sAssets), "assets tiles=%d autotiles=%d maps=%d", pApp->iTilesheetFiles, pApp->iAutotileFiles, pApp->iMapFiles);
	if ( ret == XUI_OK ) (void)xuiStatusBarAddText(pApp->pStatus, XUI_STATUSBAR_SECTION_LEFT, "XUI2 MapEdit", 110.0f, 0, 0);
	if ( ret == XUI_OK ) (void)xuiStatusBarAddText(pApp->pStatus, XUI_STATUSBAR_SECTION_CENTER, sAssets, 260.0f, 0, 0);
	iIndex = -1;
	if ( ret == XUI_OK ) iIndex = xuiStatusBarAddText(pApp->pStatus, XUI_STATUSBAR_SECTION_RIGHT, "tool=20", 92.0f, 0, 0);
	pApp->iStatusModeIndex = iIndex;
	return ret;
}

static int __mapeditCreateDock(mapedit_xui2_app_t* pApp)
{
	xui_dock_panel_desc_t tDockDesc;
	xui_canvas_desc_t tCanvasDesc;
	int ret;
	int iBottomPane;
	memset(&tCanvasDesc, 0, sizeof(tCanvasDesc));
	tCanvasDesc.iSize = sizeof(tCanvasDesc);
	tCanvasDesc.fCanvasWidth = MAPEDIT_XUI2_CANVAS_W;
	tCanvasDesc.fCanvasHeight = MAPEDIT_XUI2_CANVAS_H;
	tCanvasDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	tCanvasDesc.iCornerMode = XUI_SCROLL_FRAME_CORNER_AUTO;
	tCanvasDesc.iWheelAxis = XUI_WHEEL_AXIS_BOTH;
	tCanvasDesc.bContentDragEnabled = 1;
	tCanvasDesc.iBackgroundColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	tCanvasDesc.iBorderColor = XUI_COLOR_RGBA(196, 204, 214, 255);
	ret = xuiCanvasCreate(pApp->pContext, &pApp->pCanvas, &tCanvasDesc);
	if ( ret != XUI_OK ) return ret;
	ret = __mapeditCreatePanel(pApp, 0, MAPEDIT_XUI2_PANEL_MAP_LIST, "Map Manager", "Legacy tab: map.list", "Create, select and save map xson files.", XUI_COLOR_RGBA(45, 132, 190, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 1, MAPEDIT_XUI2_PANEL_TILE_SELECT, "Map Tile Select", "Legacy tab: map.tile_select", "Tilesheet/autotile picker for map painting.", XUI_COLOR_RGBA(61, 130, 184, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 2, MAPEDIT_XUI2_PANEL_INSPECTOR, "Map Properties", "Legacy tab: map.properties", "Selection, layer and map metadata inspector.", XUI_COLOR_RGBA(80, 150, 112, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 3, MAPEDIT_XUI2_PANEL_OUTPUT, "Passage Adjust", "Legacy tab: map.passage_adjust", "Passage overlay editing workspace.", XUI_COLOR_RGBA(166, 112, 52, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 4, MAPEDIT_XUI2_PANEL_OUTPUT, "Object Edit", "Legacy tab: map.object_edit", "Object list and object placement workspace.", XUI_COLOR_RGBA(166, 112, 52, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 5, MAPEDIT_XUI2_PANEL_OUTPUT, "Event Edit", "Legacy tab: map.event_edit", "Event page and trigger condition workspace.", XUI_COLOR_RGBA(166, 112, 52, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 6, MAPEDIT_XUI2_PANEL_OUTPUT, "Map Tags", "Legacy tab: map.tags", "Map custom data channel editor.", XUI_COLOR_RGBA(166, 112, 52, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 7, MAPEDIT_XUI2_PANEL_ASSETS, "Tileset Materials", "Legacy tab: tileset.materials", "Material categories and source files.", XUI_COLOR_RGBA(132, 94, 180, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 8, MAPEDIT_XUI2_PANEL_TILESET, "Tileset Manager", "Legacy tab: tileset.set", "Tileset and special tile slot management.", XUI_COLOR_RGBA(132, 94, 180, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 9, MAPEDIT_XUI2_PANEL_TILESET, "Tileset Arrange", "Legacy tab: tileset.set_layout", "Final tileset ID and layout arrangement.", XUI_COLOR_RGBA(132, 94, 180, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 10, MAPEDIT_XUI2_PANEL_TILESET, "Tileset Passage", "Legacy tab: tileset.passage", "Simple/expert/eight-direction passage tools.", XUI_COLOR_RGBA(132, 94, 180, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 11, MAPEDIT_XUI2_PANEL_TILESET, "Tileset Tags", "Legacy tab: tileset.tags", "Custom data channel tile tagging.", XUI_COLOR_RGBA(132, 94, 180, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 12, MAPEDIT_XUI2_PANEL_TILESET, "Actor Overlay", "Legacy tab: tileset.actor_overlay", "Actor cover and overhang configuration.", XUI_COLOR_RGBA(132, 94, 180, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 13, MAPEDIT_XUI2_PANEL_INSPECTOR, "Tileset Properties", "Legacy tab: tileset.properties", "Current material, tile and passage inspector.", XUI_COLOR_RGBA(80, 150, 112, 255));
	if ( ret == XUI_OK ) ret = __mapeditCreatePanel(pApp, 14, MAPEDIT_XUI2_PANEL_OUTPUT, "Output", "Smoke and map SDK messages.", "Docked logs keep the editor state visible.", XUI_COLOR_RGBA(166, 112, 52, 255));
	if ( ret != XUI_OK ) return ret;
	memset(&tDockDesc, 0, sizeof(tDockDesc));
	tDockDesc.iSize = sizeof(tDockDesc);
	tDockDesc.pFont = pApp->pFont;
	ret = xuiDockPanelCreate(pApp->pContext, &pApp->pDock, &tDockDesc);
	if ( ret == XUI_OK ) ret = xuiWidgetAddChild(pApp->pRoot, pApp->pDock);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Map Edit", pApp->pCanvas, &pApp->arrWindowIds[0]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Map Manager", pApp->arrPanelWidgets[0], &pApp->arrWindowIds[1]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Map Tile Select", pApp->arrPanelWidgets[1], &pApp->arrWindowIds[2]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Map Properties", pApp->arrPanelWidgets[2], &pApp->arrWindowIds[3]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Passage Adjust", pApp->arrPanelWidgets[3], &pApp->arrWindowIds[4]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Object Edit", pApp->arrPanelWidgets[4], &pApp->arrWindowIds[5]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Event Edit", pApp->arrPanelWidgets[5], &pApp->arrWindowIds[6]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Map Tags", pApp->arrPanelWidgets[6], &pApp->arrWindowIds[7]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Tileset Materials", pApp->arrPanelWidgets[7], &pApp->arrWindowIds[8]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Tileset Manager", pApp->arrPanelWidgets[8], &pApp->arrWindowIds[9]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Tileset Arrange", pApp->arrPanelWidgets[9], &pApp->arrWindowIds[10]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Tileset Passage", pApp->arrPanelWidgets[10], &pApp->arrWindowIds[11]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Tileset Tags", pApp->arrPanelWidgets[11], &pApp->arrWindowIds[12]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Actor Overlay", pApp->arrPanelWidgets[12], &pApp->arrWindowIds[13]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Tileset Properties", pApp->arrPanelWidgets[13], &pApp->arrWindowIds[14]);
	if ( ret == XUI_OK ) ret = xuiDockPanelAddWindow(pApp->pDock, "Output", pApp->arrPanelWidgets[14], &pApp->arrWindowIds[15]);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindow(pApp->pDock, pApp->arrWindowIds[0], XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_FILL, 0.0f, &pApp->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pDock, pApp->arrWindowIds[4], pApp->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pDock, pApp->arrWindowIds[5], pApp->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pDock, pApp->arrWindowIds[6], pApp->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pDock, pApp->arrWindowIds[7], pApp->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pDock, pApp->arrWindowIds[10], pApp->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pDock, pApp->arrWindowIds[11], pApp->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pDock, pApp->arrWindowIds[12], pApp->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pDock, pApp->arrWindowIds[13], pApp->iDocPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindow(pApp->pDock, pApp->arrWindowIds[1], XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_LEFT, 0.24f, &pApp->iLeftPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pDock, pApp->arrWindowIds[2], pApp->iLeftPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pDock, pApp->arrWindowIds[8], pApp->iLeftPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pDock, pApp->arrWindowIds[9], pApp->iLeftPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindow(pApp->pDock, pApp->arrWindowIds[3], XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_RIGHT, 0.25f, &pApp->iRightPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindowToPane(pApp->pDock, pApp->arrWindowIds[14], pApp->iRightPane);
	if ( ret == XUI_OK ) ret = xuiDockPanelDockWindow(pApp->pDock, pApp->arrWindowIds[15], XUI_DOCK_PANEL_REGION_DOCUMENT, XUI_DOCK_PANEL_SIDE_BOTTOM, 0.20f, &iBottomPane);
	return ret;
}

static int __mapeditCreateUi(mapedit_xui2_app_t* pApp)
{
	int ret;
	ret = xuiWidgetCreate(pApp->pContext, &pApp->pRoot);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetLayoutType(pApp->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pApp->pRoot, __mapeditRootRender, pApp);
	ret = xuiSetRootWidget(pApp->pContext, pApp->pRoot);
	if ( ret == XUI_OK ) ret = __mapeditCreateToolbar(pApp);
	if ( ret == XUI_OK ) ret = __mapeditCreateDock(pApp);
	if ( ret == XUI_OK ) ret = __mapeditCreateStatus(pApp);
	return ret;
}

static int __mapeditCreateAssets(mapedit_xui2_app_t* pApp)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int ret;
	pApp->tProxy = xuiProxyXge();
	ret = xuiCreate(&pApp->pContext);
	if ( ret != XUI_OK ) return ret;
	ret = xuiSetProxy(pApp->pContext, &pApp->tProxy);
	if ( ret != XUI_OK ) return ret;
	ret = xuiInputViewport(pApp->pContext, (float)MAPEDIT_XUI2_W, (float)MAPEDIT_XUI2_H);
	if ( ret != XUI_OK ) return ret;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = MAPEDIT_XUI2_W;
	tSurfaceDesc.iHeight = MAPEDIT_XUI2_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	ret = pApp->tProxy.surfaceCreate(&pApp->tProxy, &pApp->pTarget, &tSurfaceDesc);
	if ( ret != XUI_OK ) return ret;
	sFontPath = __mapeditFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	ret = pApp->tProxy.fontLoadFile(&pApp->tProxy, &pApp->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( ret != XUI_OK ) return ret;
	(void)xuiSetDefaultFont(pApp->pContext, pApp->pFont);
	xgeMapInit(&pApp->tMap);
	xgeMapSetDefault(&pApp->tMap);
	mapeditXui2MapDocInit(&pApp->tDoc);
	__mapeditLoadDefaultMapLite(pApp);
	pApp->bMapSdkOK = (pApp->tMap.tSize.iWidth > 0 && pApp->tMap.tSize.iHeight > 0 && pApp->tMap.tSize.iLayerCount > 0);
	__mapeditLoadAssetCounts(pApp);
	__mapeditLoadPreviewSurfaces(pApp);
	return __mapeditCreateUi(pApp);
}

static void __mapeditDestroyAssets(mapedit_xui2_app_t* pApp)
{
	if ( pApp->pContext != NULL ) {
		xuiDestroy(pApp->pContext);
		pApp->pContext = NULL;
	}
	if ( pApp->pFont != NULL ) {
		pApp->tProxy.fontDestroy(&pApp->tProxy, pApp->pFont);
		pApp->pFont = NULL;
	}
	if ( pApp->pTilesheetPreview != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pTilesheetPreview);
		pApp->pTilesheetPreview = NULL;
	}
	if ( pApp->pAutotilePreview != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pAutotilePreview);
		pApp->pAutotilePreview = NULL;
	}
	if ( pApp->pTarget != NULL ) {
		pApp->tProxy.surfaceDestroy(&pApp->tProxy, pApp->pTarget);
		pApp->pTarget = NULL;
	}
	xgeMapUnit(&pApp->tMap);
	mapeditXui2MapDocUnit(&pApp->tDoc);
}

static uint32_t __mapeditReadButtons(void)
{
	uint32_t buttons;
	buttons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) buttons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) buttons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) buttons |= XUI_POINTER_BUTTON_MIDDLE;
	return buttons;
}

static int __mapeditHandleInput(mapedit_xui2_app_t* pApp)
{
	float x;
	float y;
	float wheelX;
	float wheelY;
	uint32_t buttons;
	uint32_t pressed;
	uint32_t released;
	int ret;
	xgeMouseGet(&x, &y);
	xgeMouseGetWheel(&wheelX, &wheelY);
	buttons = __mapeditReadButtons();
	if ( !pApp->bHasMouse || pApp->fLastMouseX != x || pApp->fLastMouseY != y || pApp->iLastButtons != buttons ) {
		ret = xuiInputPointerMove(pApp->pContext, x, y, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	if ( wheelX != 0.0f || wheelY != 0.0f ) {
		ret = xuiInputPointerWheel(pApp->pContext, x, y, wheelX, wheelY, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	pressed = buttons & ~pApp->iLastButtons;
	released = pApp->iLastButtons & ~buttons;
	if ( (pressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		ret = xuiInputPointerDown(pApp->pContext, x, y, XUI_POINTER_BUTTON_LEFT, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	if ( (released & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		ret = xuiInputPointerUp(pApp->pContext, x, y, XUI_POINTER_BUTTON_LEFT, buttons);
		if ( ret != XUI_OK ) return ret;
	}
	pApp->bHasMouse = 1;
	pApp->fLastMouseX = x;
	pApp->fLastMouseY = y;
	pApp->iLastButtons = buttons;
	return XUI_OK;
}

static int __mapeditExerciseCommands(mapedit_xui2_app_t* pApp)
{
	xui_rect_t r;
	float x;
	float y;
	int ret;
	if ( pApp == NULL || pApp->bCommandExerciseDone || pApp->pToolbar == NULL ) return XUI_OK;
	r = xuiToolbarGetItemRect(pApp->pToolbar, 4);
	if ( r.fW <= 0.0f || r.fH <= 0.0f ) return XUI_OK;
	x = r.fX + r.fW * 0.5f;
	y = r.fY + r.fH * 0.5f;
	ret = xuiInputPointerMove(pApp->pContext, x, y, 0);
	if ( ret != XUI_OK ) return ret;
	ret = xuiDispatchPendingEvents(pApp->pContext);
	if ( ret != XUI_OK ) return ret;
	ret = xuiInputPointerDown(pApp->pContext, x, y, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( ret != XUI_OK ) return ret;
	ret = xuiDispatchPendingEvents(pApp->pContext);
	if ( ret != XUI_OK ) return ret;
	ret = xuiInputPointerUp(pApp->pContext, x, y, XUI_POINTER_BUTTON_LEFT, 0);
	if ( ret != XUI_OK ) return ret;
	ret = xuiDispatchPendingEvents(pApp->pContext);
	if ( ret == XUI_OK ) pApp->bCommandExerciseDone = 1;
	return ret;
}

static int __mapeditFrame(void* pUser)
{
	mapedit_xui2_app_t* pApp;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int ret;
	pApp = (mapedit_xui2_app_t*)pUser;
	if ( pApp == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	ret = xgeBegin();
	if ( ret != XGE_OK ) return ret;
	if ( !pApp->bContentDrawn ) {
		ret = __mapeditDrawCanvas(pApp);
		if ( ret != XUI_OK ) return ret;
		pApp->bContentDrawn = 1;
	}
	ret = __mapeditHandleInput(pApp);
	if ( ret != XUI_OK ) return ret;
	ret = xuiDispatchPendingEvents(pApp->pContext);
	if ( ret != XUI_OK ) return ret;
	(void)xuiWidgetSetRect(pApp->pRoot, (xui_rect_t){0.0f, 0.0f, (float)MAPEDIT_XUI2_W, (float)MAPEDIT_XUI2_H});
	(void)xuiWidgetSetRect(pApp->pToolbar, (xui_rect_t){0.0f, 0.0f, (float)MAPEDIT_XUI2_W, MAPEDIT_XUI2_TOOLBAR_H});
	(void)xuiWidgetSetRect(pApp->pDock, (xui_rect_t){0.0f, MAPEDIT_XUI2_TOOLBAR_H, (float)MAPEDIT_XUI2_W, (float)MAPEDIT_XUI2_H - MAPEDIT_XUI2_TOOLBAR_H - MAPEDIT_XUI2_STATUS_H});
	(void)xuiWidgetSetRect(pApp->pStatus, (xui_rect_t){0.0f, (float)MAPEDIT_XUI2_H - MAPEDIT_XUI2_STATUS_H, (float)MAPEDIT_XUI2_W, MAPEDIT_XUI2_STATUS_H});
	ret = xuiLayout(pApp->pContext);
	if ( ret != XUI_OK ) return ret;
	if ( pApp->iMaxFrames > 0 ) {
		ret = __mapeditExerciseCommands(pApp);
		if ( ret != XUI_OK ) return ret;
	}
	pApp->bCreateOK = (pApp->pRoot != NULL && pApp->pToolbar != NULL && pApp->pDock != NULL && pApp->pCanvas != NULL && pApp->pStatus != NULL);
	pApp->bLayoutOK = (xuiDockPanelGetWindowCount(pApp->pDock) >= 16 && xuiDockPanelGetPaneCount(pApp->pDock) >= 4);
	ret = xuiUpdate(pApp->pContext, xgeGetDelta());
	if ( ret != XUI_OK ) return ret;
	ret = pApp->tProxy.surfaceClear(&pApp->tProxy, pApp->pTarget, XUI_COLOR_RGBA(228, 234, 241, 255));
	if ( ret != XUI_OK ) return ret;
	tFullRect = (xui_rect_i_t){0, 0, MAPEDIT_XUI2_W, MAPEDIT_XUI2_H};
	ret = xuiRender(pApp->pContext, pApp->pTarget, &tFullRect, 1);
	if ( ret != XUI_OK ) return ret;
	xgeClear(XUI_COLOR_RGBA(32, 36, 42, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)MAPEDIT_XUI2_W, (float)MAPEDIT_XUI2_H};
	tDst = tSrc;
	ret = pApp->tProxy.surfaceDraw(&pApp->tProxy, pApp->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( ret != XUI_OK ) return ret;
	ret = xgeEnd();
	if ( ret != XGE_OK ) return ret;
	pApp->iFrame++;
	if ( pApp->iMaxFrames > 0 && pApp->iFrame >= pApp->iMaxFrames ) {
		printf("mapedit_xui2 final-summary frames=%d create=%d layout=%d canvas=%d assets=%d preview=%d mapsdk=%d mapfile=%d mapsave=%d tileedit=%d windows=%d panes=%d toolbar=%d selectedTool=%d tilesheets=%d autotiles=%d maps=%d tiles=%d nonzero=%d\n",
			pApp->iFrame, pApp->bCreateOK, pApp->bLayoutOK, pApp->bCanvasOK, pApp->bAssetsOK, pApp->bPreviewOK, pApp->bMapSdkOK, pApp->bMapFileOK, pApp->bMapSaveOK, pApp->bTileEditOK,
			xuiDockPanelGetWindowCount(pApp->pDock), xuiDockPanelGetPaneCount(pApp->pDock),
			pApp->iToolbarSelects, pApp->iSelectedTool, pApp->iTilesheetFiles, pApp->iAutotileFiles, pApp->iMapFiles, pApp->tDoc.iTileCount, pApp->tDoc.iNonZeroTileCount);
		xgeQuit();
	}
	return XUI_OK;
}

static int __mapeditParseArgs(mapedit_xui2_app_t* pApp, int argc, char** argv)
{
	int i;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && i + 1 < argc ) {
			pApp->iMaxFrames = atoi(argv[++i]);
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pApp->iMaxFrames = atoi(argv[i] + 9);
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			printf("usage: xge_mapedit_xui2 [--frames N]\n");
			return 1;
		} else {
			printf("unknown option: %s\n", argv[i]);
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XUI_OK;
}

int main(int argc, char** argv)
{
	mapedit_xui2_app_t app;
	xge_desc_t desc;
	int ret;
	memset(&app, 0, sizeof(app));
	app.iSelectedTool = 20;
	app.iStatusModeIndex = -1;
	ret = __mapeditParseArgs(&app, argc, argv);
	if ( ret != XUI_OK ) return (ret == 1) ? 0 : 1;
	memset(&desc, 0, sizeof(desc));
	desc.iWidth = MAPEDIT_XUI2_W;
	desc.iHeight = MAPEDIT_XUI2_H;
	desc.sTitle = "MapEdit XUI2";
	desc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	if ( app.iMaxFrames <= 0 ) desc.iFlags |= XGE_INIT_ON_DEMAND;
	desc.iRunMode = XGE_RUN_GAME_LOOP;
	desc.iTargetFPS = 60;
	ret = xgeInit(&desc);
	if ( ret != XGE_OK ) {
		printf("mapedit_xui2: xgeInit failed: %d\n", ret);
		return 1;
	}
	ret = __mapeditCreateAssets(&app);
	if ( ret != XUI_OK ) {
		printf("mapedit_xui2: create assets failed: %d\n", ret);
		__mapeditDestroyAssets(&app);
		xgeUnit();
		return 1;
	}
	ret = xgeRun(__mapeditFrame, &app);
	__mapeditDestroyAssets(&app);
	xgeUnit();
	return (ret == XGE_OK && app.bCreateOK && app.bLayoutOK && app.bCanvasOK) ? 0 : 1;
}
