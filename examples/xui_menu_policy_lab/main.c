#include "../../xge.h"
#include "../xui_demo_style.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* g_arrMenuItems[] = {
	"Alpha",
	"Bravo",
	"Charlie",
	"Delta",
	"Echo",
	"Foxtrot"
};

static const int g_arrMenuEnabled[] = {
	1, 1, 1, 1, 1, 1
};

static const int g_arrMenuEnabledReset[] = {
	1, 1, 0, 1, 1, 1
};

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pOwnerWidget;
	xge_xui_widget pHintWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tOwnerLabel;
	xge_xui_label_t tHint;
	xge_xui_menu_t tMenu;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iSelectCount;
	int iLastSelected;
	int bInitOK;
	int bConfigOK;
	int bResetOK;
	int bClampOK;
	int bReopenOK;
	int bFocusOK;
	int bEnabledResetOK;
} app_state_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int FloatNear(float fA, float fB, float fEpsilon)
{
	return fabsf(fA - fB) <= fEpsilon;
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MenuSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSelectCount++;
		pApp->iLastSelected = iIndex;
	}
}

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-menu-policy-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-menu-policy-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	float fInnerW;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}

	tRoot.fX = 18.0f;
	tRoot.fY = 18.0f;
	tRoot.fW = (float)iWidth - 36.0f;
	tRoot.fH = (float)iHeight - 36.0f;
	if ( tRoot.fW < 660.0f ) {
		tRoot.fW = 660.0f;
	}
	if ( tRoot.fH < 350.0f ) {
		tRoot.fH = 350.0f;
	}
	fInnerW = tRoot.fW - 40.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pOwnerWidget, (xge_rect_t){ 24.0f, 84.0f, 220.0f, 42.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 24.0f, tRoot.fH - 56.0f, fInnerW, 32.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[320];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d config=%d reset=%d clamp=%d reopen=%d focus=%d enabled=%d open=%d sel=%d scroll=%.2f cb=%d last=%d",
		pApp->bInitOK,
		pApp->bConfigOK,
		pApp->bResetOK,
		pApp->bClampOK,
		pApp->bReopenOK,
		pApp->bFocusOK,
		pApp->bEnabledResetOK,
		xgeXuiMenuIsOpen(&pApp->tMenu),
		xgeXuiListViewGetSelected(&pApp->tMenu.tList),
		xgeXuiListViewGetScroll(&pApp->tMenu.tList),
		pApp->iSelectCount,
		pApp->iLastSelected);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(238, 242, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(34, 42, 56, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 96, 118, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(255, 166, 86, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(50, 64, 84, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(68, 84, 108, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(44, 58, 78, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(54, 74, 104, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(78, 82, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pOwnerWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pOwnerWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pOwnerWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pOwnerWidget, XGE_COLOR_RGBA(52, 70, 94, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pOwnerWidget, 12.0f, 10.0f, 12.0f, 10.0f);
	xgeXuiWidgetSetFocusable(pApp->pOwnerWidget, 1);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui menu policy lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tOwnerLabel, pApp->pOwnerWidget, pFont, "Menu owner") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tOwnerLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tHint, pApp->pHintWidget, pFont, "Auto-check covers MenuOpen reset, repeated reopen, size update, clamp and focus return after close.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tHint, XGE_COLOR_RGBA(164, 182, 206, 255));

	if ( xgeXuiMenuInit(&pApp->tMenu, &pApp->tXui, pApp->pOwnerWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iRet;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	pApp->bInitOK =
		(pApp->tMenu.pOwner == pApp->pOwnerWidget) &&
		(pApp->tMenu.pPopupWidget != NULL) &&
		(pApp->tMenu.pListWidget != NULL) &&
		(pApp->tMenu.tPopup.pOwner == pApp->pOwnerWidget) &&
		(pApp->tMenu.tPopup.pFocusRestore == pApp->pOwnerWidget) &&
		(pApp->tMenu.tPopup.iPlacement == XGE_XUI_OVERLAY_PLACEMENT_CURSOR) &&
		(xgeXuiWidgetGetZ(pApp->tMenu.pPopupWidget) == 1200) &&
		(pApp->tMenu.fWidth > 0.0f) &&
		(pApp->tMenu.fItemHeight > 0.0f);

	xgeXuiMenuSetItems(&pApp->tMenu, g_arrMenuItems, (int)(sizeof(g_arrMenuItems) / sizeof(g_arrMenuItems[0])));
	xgeXuiMenuSetEnabledItems(&pApp->tMenu, g_arrMenuEnabled, (int)(sizeof(g_arrMenuEnabled) / sizeof(g_arrMenuEnabled[0])));
	xgeXuiMenuSetFont(&pApp->tMenu, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiMenuSetSelect(&pApp->tMenu, MenuSelect, pApp);
	xgeXuiMenuSetSize(&pApp->tMenu, 152.0f, 60.0f);
	xgeXuiMenuSetColors(&pApp->tMenu, XGE_COLOR_RGBA(42, 50, 64, 245), XGE_COLOR_RGBA(52, 64, 82, 255), XGE_COLOR_RGBA(84, 136, 214, 255), XGE_COLOR_RGBA(248, 250, 252, 255), XGE_COLOR_RGBA(128, 138, 150, 220));
	pApp->bConfigOK =
		(pApp->tMenu.arrItems == g_arrMenuItems) &&
		(pApp->tMenu.arrEnabled == g_arrMenuEnabled) &&
		(pApp->tMenu.iItemCount == 6) &&
		(pApp->tMenu.iEnabledCount == 6) &&
		(pApp->tMenu.procSelect == MenuSelect) &&
		(pApp->tMenu.pUser == pApp) &&
		FloatNear(pApp->tMenu.fWidth, 152.0f, 0.01f) &&
		FloatNear(pApp->tMenu.fMaxHeight, 60.0f, 0.01f);

	xgeXuiListViewSetSelected(&pApp->tMenu.tList, 4);
	xgeXuiListViewSetScroll(&pApp->tMenu.tList, 48.0f);
	xgeXuiMenuOpen(&pApp->tMenu, (float)xgeGetWidth() - 8.0f, (float)xgeGetHeight() - 6.0f);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pApp->tMenu.pPopupWidget);
	pApp->bResetOK =
		(xgeXuiMenuIsOpen(&pApp->tMenu) != 0) &&
		(xgeXuiListViewGetSelected(&pApp->tMenu.tList) == -1) &&
		FloatNear(xgeXuiListViewGetScroll(&pApp->tMenu.tList), 0.0f, 0.01f) &&
		(pApp->tXui.pFocus == pApp->tMenu.pListWidget);
	pApp->bClampOK =
		FloatNear(tRect.fW, 152.0f, 0.01f) &&
		FloatNear(tRect.fH, 60.0f, 0.01f) &&
		(tRect.fX >= 0.0f) &&
		(tRect.fY >= 0.0f) &&
		(tRect.fX + tRect.fW <= (float)xgeGetWidth() + 0.01f) &&
		(tRect.fY + tRect.fH <= (float)xgeGetHeight() + 0.01f);

	xgeXuiMenuClose(&pApp->tMenu);
	pApp->bFocusOK =
		(xgeXuiMenuIsOpen(&pApp->tMenu) == 0) &&
		(pApp->tXui.pFocus == pApp->pOwnerWidget);
	xgeXuiMenuClose(&pApp->tMenu);
	pApp->bFocusOK =
		pApp->bFocusOK &&
		(xgeXuiMenuIsOpen(&pApp->tMenu) == 0) &&
		(pApp->tXui.pFocus == pApp->pOwnerWidget);

	xgeXuiListViewSetSelected(&pApp->tMenu.tList, 2);
	xgeXuiMenuSetEnabledItems(&pApp->tMenu, g_arrMenuEnabledReset, (int)(sizeof(g_arrMenuEnabledReset) / sizeof(g_arrMenuEnabledReset[0])));
	pApp->bEnabledResetOK = (xgeXuiListViewGetSelected(&pApp->tMenu.tList) == -1);
	xgeXuiMenuSetEnabledItems(&pApp->tMenu, g_arrMenuEnabled, (int)(sizeof(g_arrMenuEnabled) / sizeof(g_arrMenuEnabled[0])));

	xgeXuiMenuSetSize(&pApp->tMenu, 208.0f, 96.0f);
	xgeXuiListViewSetSelected(&pApp->tMenu.tList, 5);
	xgeXuiListViewSetScroll(&pApp->tMenu.tList, 72.0f);
	xgeXuiMenuOpen(&pApp->tMenu, pApp->pOwnerWidget->tRect.fX + 12.0f, pApp->pOwnerWidget->tRect.fY + pApp->pOwnerWidget->tRect.fH + 4.0f);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	tRect = xgeXuiWidgetGetRect(pApp->tMenu.pPopupWidget);
	pApp->bReopenOK =
		(xgeXuiMenuIsOpen(&pApp->tMenu) != 0) &&
		FloatNear(tRect.fW, 208.0f, 0.01f) &&
		FloatNear(tRect.fH, 96.0f, 0.01f) &&
		(xgeXuiListViewGetSelected(&pApp->tMenu.tList) == -1) &&
		FloatNear(xgeXuiListViewGetScroll(&pApp->tMenu.tList), 0.0f, 0.01f) &&
		(pApp->tXui.pFocus == pApp->tMenu.pListWidget);

	MakeMouseEvent(
		&tEvent,
		XGE_EVENT_MOUSE_DOWN,
		XGE_MOUSE_LEFT,
		pApp->tMenu.pListWidget->tContentRect.fX + 12.0f,
		pApp->tMenu.pListWidget->tContentRect.fY + pApp->tMenu.tList.fItemHeight * 1.0f + 6.0f);
	iRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bReopenOK =
		pApp->bReopenOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiMenuIsOpen(&pApp->tMenu) == 0) &&
		(pApp->iSelectCount == 1) &&
		(pApp->iLastSelected == 1);

	xgeXuiMenuOpen(&pApp->tMenu, pApp->pOwnerWidget->tRect.fX + 12.0f, pApp->pOwnerWidget->tRect.fY + pApp->pOwnerWidget->tRect.fH + 4.0f);
	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( LoadFont(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	return RunChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiMenuUnit(&pApp->tMenu);
	xgeXuiLabelUnit(&pApp->tHint);
	xgeXuiLabelUnit(&pApp->tOwnerLabel);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
	memset(pApp, 0, sizeof(*pApp));
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;
	xge_rect_t tRect;

	pApp = (app_state_t*)pUser;
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());

	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		tRect = xgeXuiWidgetGetRect(pApp->tMenu.pPopupWidget);
		printf(
			"xui-menu-policy-lab final-summary frames=%d init=%d config=%d reset=%d clamp=%d reopen=%d focus=%d enabled=%d open=%d rect=%.2f,%.2f,%.2f,%.2f selected=%d scroll=%.2f cb=%d last=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bConfigOK,
			pApp->bResetOK,
			pApp->bClampOK,
			pApp->bReopenOK,
			pApp->bFocusOK,
			pApp->bEnabledResetOK,
			xgeXuiMenuIsOpen(&pApp->tMenu),
			tRect.fX,
			tRect.fY,
			tRect.fW,
			tRect.fH,
			xgeXuiListViewGetSelected(&pApp->tMenu.tList),
			xgeXuiListViewGetScroll(&pApp->tMenu.tList),
			pApp->iSelectCount,
			pApp->iLastSelected);
		printf("xui-menu-policy-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int iFrameLimit;
	int iExitCode;
	int i;

	iFrameLimit = 0;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			iFrameLimit = ArgInt(argv[i + 1], iFrameLimit);
			i++;
		}
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 720;
	tDesc.iHeight = 380;
	tDesc.sTitle = "XGE XUI Menu Policy Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	iExitCode =
		(tApp.bInitOK && tApp.bConfigOK && tApp.bResetOK && tApp.bClampOK &&
		 tApp.bReopenOK && tApp.bFocusOK && tApp.bEnabledResetOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

