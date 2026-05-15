#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MENU_OWNER_COUNT 4

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pOwner[MENU_OWNER_COUNT];
	xge_xui_panel_t tPanel;
	xge_xui_label_t tStatus;
	xge_xui_label_t tOwnerLabel[MENU_OWNER_COUNT];
	xge_xui_menu_t tMenu[MENU_OWNER_COUNT];
	xge_xui_menu_t tSubmenu;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iSelectCount;
	int iLastValue;
	int bCreateOK;
	int bOpenOK;
	int bSubmenuOK;
	int bHotkeyOK;
} app_state_t;

static const xge_xui_menu_item_t g_arrBasicItems[] = {
	{ "Open", "Ctrl+O", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 10, 1, NULL, NULL },
	{ "Save", "Ctrl+S", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 20, 2, NULL, NULL },
	{ NULL, NULL, XGE_XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL },
	{ "Disabled", NULL, XGE_XUI_MENU_ITEM_NORMAL, 0, 30, 0, NULL, NULL },
	{ "Delete", "Del", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED | XGE_XUI_MENU_ITEM_DANGER, 40, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrStateItems[] = {
	{ "Show grid", NULL, XGE_XUI_MENU_ITEM_CHECK, XGE_XUI_MENU_ITEM_ENABLED | XGE_XUI_MENU_ITEM_CHECKED, 101, 0, NULL, NULL },
	{ "Snap to pixel", NULL, XGE_XUI_MENU_ITEM_CHECK, XGE_XUI_MENU_ITEM_ENABLED, 102, 0, NULL, NULL },
	{ NULL, NULL, XGE_XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL },
	{ "Small", NULL, XGE_XUI_MENU_ITEM_RADIO, XGE_XUI_MENU_ITEM_ENABLED, 111, 0, NULL, NULL },
	{ "Normal", NULL, XGE_XUI_MENU_ITEM_RADIO, XGE_XUI_MENU_ITEM_ENABLED | XGE_XUI_MENU_ITEM_CHECKED, 112, 0, NULL, NULL },
	{ "Large", NULL, XGE_XUI_MENU_ITEM_RADIO, XGE_XUI_MENU_ITEM_ENABLED, 113, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrSubItems[] = {
	{ "Align left", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 201, 0, NULL, NULL },
	{ "Align center", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 202, 0, NULL, NULL },
	{ "Align right", NULL, XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 203, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrNestedItems[] = {
	{ "Arrange", NULL, XGE_XUI_MENU_ITEM_SUBMENU, XGE_XUI_MENU_ITEM_ENABLED, 0, 0, NULL, NULL },
	{ "Refresh", "F5", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 210, 0, NULL, NULL },
	{ "Properties", "Alt+Enter", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 220, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrLongItems[XGE_XUI_MENU_ITEM_CAPACITY];

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;
	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui_menu font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void MenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iIndex;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pApp->iSelectCount++;
	pApp->iLastValue = iValue;
}

static int OwnerEvent(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pUser;
	if ( (pApp == NULL) || (pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType != XGE_EVENT_MOUSE_DOWN || pEvent->iParam1 != XGE_MOUSE_LEFT ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	for ( i = 0; i < MENU_OWNER_COUNT; i++ ) {
		if ( pApp->pOwner[i] == pWidget ) {
			xgeXuiMenuOpenForOwner(&pApp->tMenu[i], pWidget);
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static int AddOwner(app_state_t* pApp, int iIndex, const char* sText, float fX, float fY, xge_font pFont)
{
	pApp->pOwner[iIndex] = xgeXuiWidgetCreate();
	if ( pApp->pOwner[iIndex] == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pOwner[iIndex], (xge_rect_t){ fX, fY, 188.0f, 34.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pOwner[iIndex], 10.0f, 6.0f, 10.0f, 6.0f);
	xgeXuiWidgetSetBackground(pApp->pOwner[iIndex], XGE_COLOR_RGBA(232, 243, 252, 255));
	xgeXuiWidgetSetBorder(pApp->pOwner[iIndex], 1.0f, XGE_COLOR_RGBA(75, 138, 196, 255));
	xgeXuiWidgetSetFocusable(pApp->pOwner[iIndex], 1);
	xgeXuiWidgetSetEvent(pApp->pOwner[iIndex], OwnerEvent, pApp);
	xgeXuiLabelInit(&pApp->tOwnerLabel[iIndex], pApp->pOwner[iIndex], pFont, sText);
	xgeXuiLabelSetColor(&pApp->tOwnerLabel[iIndex], XGE_COLOR_RGBA(40, 72, 104, 255));
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pOwner[iIndex]);
	return XGE_OK;
}

static void SetupLongItems(void)
{
	static char arrText[XGE_XUI_MENU_ITEM_CAPACITY][32];
	int i;

	for ( i = 0; i < 36; i++ ) {
		snprintf(arrText[i], sizeof(arrText[i]), "Menu item %02d", i + 1);
		memset(&g_arrLongItems[i], 0, sizeof(g_arrLongItems[i]));
		g_arrLongItems[i].sText = arrText[i];
		g_arrLongItems[i].iType = XGE_XUI_MENU_ITEM_NORMAL;
		g_arrLongItems[i].iState = XGE_XUI_MENU_ITEM_ENABLED;
		g_arrLongItems[i].iValue = 300 + i;
	}
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	SetupLongItems();
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pStatusWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 20.0f, 18.0f, 700.0f, 400.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "Menu");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);

	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 18.0f, 34.0f, 640.0f, 30.0f });
	xgeXuiLabelInit(&pApp->tStatus, pApp->pStatusWidget, pFont, "Click each owner to open a menu");
	xgeXuiLabelSetColor(&pApp->tStatus, XGE_COLOR_RGBA(36, 82, 118, 255));
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pStatusWidget);

	if ( AddOwner(pApp, 0, "basic command menu", 38.0f, 92.0f, pFont) != XGE_OK ||
		AddOwner(pApp, 1, "check / radio menu", 302.0f, 92.0f, pFont) != XGE_OK ||
		AddOwner(pApp, 2, "submenu menu", 38.0f, 170.0f, pFont) != XGE_OK ||
		AddOwner(pApp, 3, "long scrolling menu", 302.0f, 170.0f, pFont) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiMenuInit(&pApp->tMenu[0], &pApp->tXui);
	xgeXuiMenuInit(&pApp->tMenu[1], &pApp->tXui);
	xgeXuiMenuInit(&pApp->tMenu[2], &pApp->tXui);
	xgeXuiMenuInit(&pApp->tMenu[3], &pApp->tXui);
	xgeXuiMenuInit(&pApp->tSubmenu, &pApp->tXui);
	xgeXuiMenuSetItems(&pApp->tMenu[0], g_arrBasicItems, (int)(sizeof(g_arrBasicItems) / sizeof(g_arrBasicItems[0])));
	xgeXuiMenuSetItems(&pApp->tMenu[1], g_arrStateItems, (int)(sizeof(g_arrStateItems) / sizeof(g_arrStateItems[0])));
	xgeXuiMenuSetItems(&pApp->tSubmenu, g_arrSubItems, (int)(sizeof(g_arrSubItems) / sizeof(g_arrSubItems[0])));
	g_arrNestedItems[0].pSubmenu = &pApp->tSubmenu;
	xgeXuiMenuSetItems(&pApp->tMenu[2], g_arrNestedItems, (int)(sizeof(g_arrNestedItems) / sizeof(g_arrNestedItems[0])));
	xgeXuiMenuSetItems(&pApp->tMenu[3], g_arrLongItems, 36);
	xgeXuiMenuSetSelect(&pApp->tMenu[0], MenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tMenu[1], MenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tMenu[2], MenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tMenu[3], MenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tSubmenu, MenuSelect, pApp);
	pApp->bCreateOK = 1;
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiMenuOpenForOwner(&pApp->tMenu[0], pApp->pOwner[0]);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bOpenOK = xgeXuiMenuIsOpen(&pApp->tMenu[0]) && (pApp->tMenu[0].pContentWidget != NULL) && (pApp->tMenu[0].fContentW >= pApp->tMenu[0].tMetrics.fMinWidth);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = 'S';
	tEvent.iParam2 = XGE_KEY_MOD_CTRL;
	(void)xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bHotkeyOK = (pApp->iLastValue == 20) && (pApp->iSelectCount > 0) && (xgeXuiMenuIsOpen(&pApp->tMenu[0]) == 0);
	xgeXuiMenuClose(&pApp->tMenu[0]);
	xgeXuiMenuOpenForOwner(&pApp->tMenu[2], pApp->pOwner[2]);
	(void)xgeXuiMenuIsOpen(&pApp->tMenu[2]);
	pApp->bSubmenuOK = (pApp->tMenu[2].arrItems[0].pSubmenu == &pApp->tSubmenu);
	xgeXuiMenuClose(&pApp->tMenu[2]);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
	RunChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < MENU_OWNER_COUNT; i++ ) {
		xgeXuiMenuUnit(&pApp->tMenu[i]);
	}
	xgeXuiMenuUnit(&pApp->tSubmenu);
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN && pEvent->iParam1 == XGE_KEY_ESCAPE ) {
		xgeQuit();
		return XGE_OK;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;
	char sText[160];

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	snprintf(sText, sizeof(sText), "create=%d open=%d submenu=%d hotkey=%d selected=%d count=%d", pApp->bCreateOK, pApp->bOpenOK, pApp->bSubmenuOK, pApp->bHotkeyOK, pApp->iLastValue, pApp->iSelectCount);
	xgeXuiLabelSetText(&pApp->tStatus, sText);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_menu final-summary frames=%d create=%d open=%d submenu=%d hotkey=%d select=%d last=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bOpenOK, pApp->bSubmenuOK, pApp->bHotkeyOK, pApp->iSelectCount, pApp->iLastValue);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(232, 241, 248, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	static app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iLastValue = -1;
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_MENU_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 460;
	tDesc.sTitle = "XUI Menu";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	tApp.tScene.pUser = &tApp;
	tApp.tScene.onEnter = AppEnter;
	tApp.tScene.onLeave = AppLeave;
	tApp.tScene.onEvent = AppEvent;
	tApp.tScene.onUpdate = AppUpdate;
	tApp.tScene.onDraw = AppDraw;
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bOpenOK && tApp.bSubmenuOK && tApp.bHotkeyOK) ? 0 : 3;
}
