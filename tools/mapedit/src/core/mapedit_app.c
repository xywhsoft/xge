#include "mapedit_app.h"
#include "mapedit_layout.h"
#include "mapedit_workspace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static xge_xui_menu_item_t g_arrFileItems[] = {
	{ "打开地图", "Ctrl+O", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, MAPEDIT_CMD_OPEN_MAP, 0, NULL, NULL },
	{ "保存地图", "Ctrl+S", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, MAPEDIT_CMD_SAVE_MAP, 0, NULL, NULL },
	{ "另存为", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, MAPEDIT_CMD_SAVE_MAP_AS, 0, NULL, NULL },
	{ NULL, NULL, XGE_XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL },
	{ "退出", "Alt+F4", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, MAPEDIT_CMD_EXIT, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrViewItems[] = {
	{ "重置布局", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, MAPEDIT_CMD_RESET_LAYOUT, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrHelpItems[] = {
	{ "关于", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, MAPEDIT_CMD_ABOUT, 0, NULL, NULL }
};

static int MapEditLoadFont(mapedit_app_t* pApp)
{
	str sXrfPath;

	sXrfPath = xrtPathJoin(2, xCore.AppPath, (str)"res/fonts/simsun12.xrf");
	if ( sXrfPath != NULL ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoadXRF(&pApp->tFont, (const char*)sXrfPath) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("mapedit font loaded: %s\n", sXrfPath);
			xrtFree(sXrfPath);
			return XGE_OK;
		}
		printf("mapedit font load failed: %s\n", sXrfPath);
		xrtFree(sXrfPath);
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xui_texture MapEditLoadXuiTexture(mapedit_app_t* pApp, const char* sFileName)
{
	str sPath;
	xui_texture pTexture;

	if ( (pApp == NULL) || (sFileName == NULL) ) {
		return NULL;
	}
	sPath = xrtPathJoin(2, xCore.AppPath, (str)sFileName);
	if ( sPath == NULL ) {
		return NULL;
	}
	pTexture = NULL;
	if ( xgeXuiTextureCreateFile(&pApp->tXui, (const char*)sPath, 0, &pTexture) != XGE_OK ) {
		printf("mapedit icon load failed: %s\n", sPath);
		pTexture = NULL;
	} else {
		printf("mapedit icon loaded: %s\n", sPath);
	}
	xrtFree(sPath);
	return pTexture;
}

static int MapEditLoadSetup(mapedit_app_t* pApp)
{
	str sPath;
	xvalue pSetup;

	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	sPath = xrtPathJoin(2, xCore.AppPath, (str)"option/setup.xson");
	if ( sPath == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSetup = xrtParseXSON_File(sPath);
	if ( pSetup == NULL ) {
		printf("mapedit setup load failed: %s\n", sPath);
		xrtFree(sPath);
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pApp->bSetupLoaded = 1;
	printf("mapedit setup loaded: %s\n", sPath);
	xvoUnref(pSetup);
	xrtFree(sPath);
	return XGE_OK;
}

static void MapEditStyleWorkspaceButton(xge_xui_button pButton, int bSelected);

static void MapEditSelectWorkspace(mapedit_app_t* pApp, int iWorkspace)
{
	if ( pApp == NULL ) {
		return;
	}
	MapEditStyleWorkspaceButton(&pApp->tTilesetEditorButton, iWorkspace == MAPEDIT_WORKSPACE_TILESET);
	MapEditStyleWorkspaceButton(&pApp->tMapEditorButton, iWorkspace == MAPEDIT_WORKSPACE_MAP);
	if ( pApp->iWorkspace == iWorkspace ) {
		return;
	}
	pApp->iWorkspace = iWorkspace;
	MapEditWorkspacesSelect(pApp, iWorkspace);
	if ( iWorkspace == MAPEDIT_WORKSPACE_TILESET ) {
		MapEditAppSetStatus(pApp, "当前工作区: 图块编辑器");
	} else {
		MapEditAppSetStatus(pApp, "当前工作区: 地图编辑器");
	}
}

static void MapEditStyleWorkspaceButton(xge_xui_button pButton, int bSelected)
{
	if ( pButton == NULL ) {
		return;
	}
	xgeXuiButtonSetColors(pButton,
		bSelected ? XGE_COLOR_RGBA(211, 236, 249, 255) : XGE_COLOR_RGBA(255, 255, 255, 0),
		bSelected ? XGE_COLOR_RGBA(211, 236, 249, 255) : XGE_COLOR_RGBA(224, 243, 253, 255),
		XGE_COLOR_RGBA(72, 154, 218, 255),
		XGE_COLOR_RGBA(54, 118, 178, 220),
		XGE_COLOR_RGBA(255, 255, 255, 0));
	xgeXuiWidgetSetBorder(pButton->pWidget, bSelected ? 1.0f : 0.0f, bSelected ? XGE_COLOR_RGBA(54, 118, 178, 220) : XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeXuiWidgetSetStateBorder(pButton->pWidget, XGE_XUI_STATE_HOVER, 1.0f, XGE_COLOR_RGBA(54, 118, 178, 220));
	xgeXuiWidgetSetStateBorder(pButton->pWidget, XGE_XUI_STATE_ACTIVE, 1.0f, XGE_COLOR_RGBA(34, 103, 166, 235));
	xgeXuiWidgetSetStateBorder(pButton->pWidget, XGE_XUI_STATE_CHECKED, 0.0f, XGE_COLOR_RGBA(0, 0, 0, 0));
	pButton->iState = XGE_XUI_STATE_NORMAL;
	pButton->bSelected = 0;
	xgeXuiWidgetSetVisualState(pButton->pWidget, XGE_XUI_STATE_NORMAL);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

static int MapEditInitWorkspaceBar(mapedit_app_t* pApp)
{
	xge_rect_t tIconSrc;

	if ( (pApp == NULL) || (pApp->pTilesetEditorButtonWidget == NULL) || (pApp->pMapEditorButtonWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiButtonInit(&pApp->tTilesetEditorButton, &pApp->tXui, pApp->pTilesetEditorButtonWidget) != XGE_OK ||
		xgeXuiButtonInit(&pApp->tMapEditorButton, &pApp->tXui, pApp->pMapEditorButtonWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->pTilesetEditorIcon = MapEditLoadXuiTexture(pApp, "res/icon_tileset_editor.png");
	pApp->pMapEditorIcon = MapEditLoadXuiTexture(pApp, "res/icon_map_editor.png");
	tIconSrc = (xge_rect_t){ 0.0f, 0.0f, 24.0f, 24.0f };
	xgeXuiButtonSetText(&pApp->tTilesetEditorButton, pApp->bFontReady ? &pApp->tFont : NULL, "图块编辑器");
	xgeXuiButtonSetText(&pApp->tMapEditorButton, pApp->bFontReady ? &pApp->tFont : NULL, "地图编辑器");
	MapEditStyleWorkspaceButton(&pApp->tTilesetEditorButton, 0);
	MapEditStyleWorkspaceButton(&pApp->tMapEditorButton, 0);
	xgeXuiButtonSetIcon(&pApp->tTilesetEditorButton, pApp->pTilesetEditorIcon, tIconSrc);
	xgeXuiButtonSetIcon(&pApp->tMapEditorButton, pApp->pMapEditorIcon, tIconSrc);
	xgeXuiButtonSetIconLayout(&pApp->tTilesetEditorButton, XGE_XUI_BUTTON_ICON_TOP, 24.0f, 3.0f);
	xgeXuiButtonSetIconLayout(&pApp->tMapEditorButton, XGE_XUI_BUTTON_ICON_TOP, 24.0f, 3.0f);
	xgeXuiButtonSetTextColor(&pApp->tTilesetEditorButton, XGE_COLOR_RGBA(34, 74, 102, 255));
	xgeXuiButtonSetTextColor(&pApp->tMapEditorButton, XGE_COLOR_RGBA(34, 74, 102, 255));
	xgeXuiButtonSetIconColor(&pApp->tTilesetEditorButton, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiButtonSetIconColor(&pApp->tMapEditorButton, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiButtonSetSelectable(&pApp->tTilesetEditorButton, 0);
	xgeXuiButtonSetSelectable(&pApp->tMapEditorButton, 0);
	xgeXuiButtonSetClick(&pApp->tTilesetEditorButton, MapEditAppWorkspaceClick, pApp);
	xgeXuiButtonSetClick(&pApp->tMapEditorButton, MapEditAppWorkspaceClick, pApp);
	MapEditSelectWorkspace(pApp, MAPEDIT_WORKSPACE_TILESET);
	return XGE_OK;
}

static int MapEditInitMenus(mapedit_app_t* pApp)
{
	if ( xgeXuiMenuInit(&pApp->tFileMenu, &pApp->tXui) != XGE_OK ||
		xgeXuiMenuInit(&pApp->tViewMenu, &pApp->tXui) != XGE_OK ||
		xgeXuiMenuInit(&pApp->tHelpMenu, &pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiMenuSetItems(&pApp->tFileMenu, g_arrFileItems, (int)(sizeof(g_arrFileItems) / sizeof(g_arrFileItems[0])));
	xgeXuiMenuSetItems(&pApp->tViewMenu, g_arrViewItems, (int)(sizeof(g_arrViewItems) / sizeof(g_arrViewItems[0])));
	xgeXuiMenuSetItems(&pApp->tHelpMenu, g_arrHelpItems, (int)(sizeof(g_arrHelpItems) / sizeof(g_arrHelpItems[0])));
	if ( xgeXuiMenuBarInit(&pApp->tMenuBar, &pApp->tXui, pApp->pMenuWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiMenuBarSetFont(&pApp->tMenuBar, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiMenuBarSetSelect(&pApp->tMenuBar, MapEditAppMenuSelect, pApp);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "文件", &pApp->tFileMenu, 1);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "视图", &pApp->tViewMenu, 2);
	xgeXuiMenuBarAddItem(&pApp->tMenuBar, "帮助", &pApp->tHelpMenu, 3);
	return XGE_OK;
}

static int MapEditInitStatusBar(mapedit_app_t* pApp)
{
	if ( xgeXuiStatusBarInit(&pApp->tStatusBar, &pApp->tXui, pApp->pStatusWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiStatusBarSetFont(&pApp->tStatusBar, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "坐标: 0,0", 148.0f, 0);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "地图: 100 x 100", 132.0f, 0);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "图块: tileset 0 / tile 0 / layer 0", 230.0f, 0);
	xgeXuiStatusBarAddFlexibleSpacer(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, 1.0f);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_RIGHT, "位置: passable / region 0 / event none", 280.0f, 0);
	MapEditAppSetStatus(pApp, "就绪");
	return XGE_OK;
}

int MapEditAppInit(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pApp, 0, sizeof(*pApp));
	pApp->iWorkspace = -1;
	xgeMapSetDefault(&pApp->tMap);
	return XGE_OK;
}

void MapEditAppUnit(mapedit_app_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	xgeMapUnit(&pApp->tMap);
}

int MapEditAppEnter(xge_scene pScene)
{
	mapedit_app_t* pApp;
	xge_xui_widget pRoot;

	pApp = (mapedit_app_t*)pScene->pUser;
	MapEditLoadSetup(pApp);
	MapEditLoadFont(pApp);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		printf("mapedit enter failed: xgeXuiInit\n");
		return XGE_ERROR;
	}
	if ( pApp->bFontReady ) {
		xgeXuiSetDefaultFont(&pApp->tXui, &pApp->tFont);
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetBackground(pRoot, XGE_COLOR_RGBA(226, 235, 242, 255));
	if ( MapEditLayoutBuild(pApp) != XGE_OK ) {
		printf("mapedit enter failed: MapEditLayoutBuild\n");
		return XGE_ERROR;
	}
	if ( MapEditInitMenus(pApp) != XGE_OK || MapEditInitWorkspaceBar(pApp) != XGE_OK || MapEditInitStatusBar(pApp) != XGE_OK ) {
		printf("mapedit enter failed: menu/workspace/status init\n");
		return XGE_ERROR;
	}
	if ( MapEditLayoutLoadUser(pApp) == XGE_OK ) {
		pApp->bLayoutLoaded = 1;
		MapEditAppSetStatus(pApp, "已加载工作区 DockPanel 布局");
	} else if ( MapEditLayoutLoadDefault(pApp) == XGE_OK ) {
		pApp->bLayoutLoaded = 1;
		MapEditAppSetStatus(pApp, "已加载默认 DockPanel 布局");
	} else {
		MapEditAppSetStatus(pApp, "默认布局不可用，使用内置布局");
	}
	pApp->bCreateOK = 1;
	return XGE_OK;
}

int MapEditAppLeave(xge_scene pScene)
{
	mapedit_app_t* pApp;

	pApp = (mapedit_app_t*)pScene->pUser;
	if ( pApp != NULL ) {
		MapEditLayoutSaveUser(pApp);
		xgeXuiButtonUnit(&pApp->tTilesetEditorButton);
		xgeXuiButtonUnit(&pApp->tMapEditorButton);
		if ( pApp->pTilesetEditorIcon != NULL ) {
			xgeXuiTextureDestroy(&pApp->tXui, pApp->pTilesetEditorIcon);
			pApp->pTilesetEditorIcon = NULL;
		}
		if ( pApp->pMapEditorIcon != NULL ) {
			xgeXuiTextureDestroy(&pApp->tXui, pApp->pMapEditorIcon);
			pApp->pMapEditorIcon = NULL;
		}
		MapEditWorkspacesUnit(pApp);
		xgeXuiStatusBarUnit(&pApp->tStatusBar);
		xgeXuiMenuBarUnit(&pApp->tMenuBar);
		xgeXuiMenuUnit(&pApp->tFileMenu);
		xgeXuiMenuUnit(&pApp->tViewMenu);
		xgeXuiMenuUnit(&pApp->tHelpMenu);
		xgeXuiUnit(&pApp->tXui);
		if ( pApp->bFontReady ) {
			xgeFontFree(&pApp->tFont);
		}
	}
	return XGE_OK;
}

void MapEditAppWorkspaceClick(xge_xui_widget pWidget, void* pUser)
{
	mapedit_app_t* pApp;

	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	if ( pWidget == pApp->pTilesetEditorButtonWidget ) {
		MapEditSelectWorkspace(pApp, MAPEDIT_WORKSPACE_TILESET);
	} else if ( pWidget == pApp->pMapEditorButtonWidget ) {
		MapEditSelectWorkspace(pApp, MAPEDIT_WORKSPACE_MAP);
	}
}

int MapEditAppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	mapedit_app_t* pApp;

	pApp = (mapedit_app_t*)pScene->pUser;
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	return XGE_OK;
}

int MapEditAppUpdate(xge_scene pScene, float fDelta)
{
	mapedit_app_t* pApp;

	pApp = (mapedit_app_t*)pScene->pUser;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		MapEditLayoutSaveUser(pApp);
		printf("mapedit final-summary frames=%d create=%d setupLoaded=%d layoutLoaded=%d layoutSaved=%d dockReady=%d\n",
			pApp->iFrameCount, pApp->bCreateOK, pApp->bSetupLoaded, pApp->bLayoutLoaded, pApp->bLayoutSaved, pApp->bDockReady);
		xgeQuit();
	}
	return XGE_OK;
}

int MapEditAppDraw(xge_scene pScene)
{
	mapedit_app_t* pApp;

	pApp = (mapedit_app_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(226, 235, 242, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

void MapEditAppSetStatus(mapedit_app_t* pApp, const char* sText)
{
	if ( (pApp == NULL) || (sText == NULL) ) {
		return;
	}
	snprintf(pApp->sStatus, sizeof(pApp->sStatus), "%s", sText);
	if ( pApp->tStatusBar.iItemCount > 0 ) {
		xgeXuiStatusBarSetItemText(&pApp->tStatusBar, 0, pApp->sStatus);
	}
}

void MapEditAppMenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	mapedit_app_t* pApp;

	(void)pWidget;
	(void)iIndex;
	pApp = (mapedit_app_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	switch ( iValue ) {
	case MAPEDIT_CMD_OPEN_MAP:
		MapEditAppSetStatus(pApp, "打开地图: 占位命令");
		break;
	case MAPEDIT_CMD_SAVE_MAP:
		MapEditLayoutSaveUser(pApp);
		MapEditAppSetStatus(pApp, "保存地图: 占位命令，DockPanel 布局已保存");
		break;
	case MAPEDIT_CMD_SAVE_MAP_AS:
		MapEditAppSetStatus(pApp, "另存为: 占位命令");
		break;
	case MAPEDIT_CMD_RESET_LAYOUT:
		if ( MapEditLayoutReset(pApp) == XGE_OK ) {
			MapEditLayoutSaveUser(pApp);
			MapEditAppSetStatus(pApp, "已重置并保存默认 DockPanel 布局");
		} else {
			MapEditAppSetStatus(pApp, "重置 DockPanel 布局失败");
		}
		break;
	case MAPEDIT_CMD_ABOUT:
		MapEditAppSetStatus(pApp, "XGE MapEdit - 通用地图编辑器框架");
		break;
	case MAPEDIT_CMD_EXIT:
		xgeQuit();
		break;
	default:
		MapEditAppSetStatus(pApp, "未知菜单命令");
		break;
	}
}
