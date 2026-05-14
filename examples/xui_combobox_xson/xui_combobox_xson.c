#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bStateOK;
	int bMetricsOK;
	int bTopOK;
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[22,18,22,18],\"gap\":12,\"background\":\"#E8F1F8FF\"},"
"\"title\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"height\":28,\"textColor\":\"#26384AFF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"grid\":{\"type\":\"grid\",\"width\":\"100%\",\"height\":204,\"columns\":2,\"rowHeight\":34,\"columnGap\":24,\"rowGap\":10},"
"\"combo\":{\"type\":\"comboBox\",\"font\":\"@fonts.body\",\"width\":\"100%\",\"height\":22,\"padding\":[8,2,8,2],\"background\":\"#F7FBFFFF\",\"hoverColor\":\"#EEF7FDFF\",\"focusColor\":\"#DDF0FCFF\",\"textColor\":\"#233446FF\",\"borderColor\":\"#7EA9CCFF\",\"popupColor\":\"#FFFFFFFF\",\"popupMaxHeight\":128,\"itemHeight\":24}"
"},"
"\"tree\":{\"type\":\"column\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"style\":\"title\",\"text\":\"ComboBox XSON\"},"
"{\"type\":\"grid\",\"id\":\"grid\",\"style\":\"grid\",\"children\":["
"{\"type\":\"comboBox\",\"id\":\"basic\",\"style\":\"combo\",\"items\":[\"Compact\",\"Comfortable\",\"Spacious\",\"Custom\"],\"enabledItems\":[true,false,true,true],\"selected\":2},"
"{\"type\":\"comboBox\",\"id\":\"rich\",\"style\":\"combo\",\"items\":[{\"text\":\"Draft\",\"value\":10},{\"text\":\"Archived\",\"value\":20,\"enabled\":false},{\"text\":\"Published\",\"value\":30},{\"text\":\"Separator\",\"separator\":true,\"enabled\":false},{\"text\":\"Scheduled\",\"value\":40}],\"selectedValue\":30,\"itemHoverColor\":\"#E6F6EEFF\",\"itemSelectedColor\":\"#CDECD8FF\",\"disabledTextColor\":\"#98A4B0FF\"},"
"{\"type\":\"comboBox\",\"id\":\"fixed\",\"style\":\"combo\",\"items\":[\"Small\",\"Normal\",\"Large\",\"Huge\",\"Fit content\",\"Fill width\"],\"value\":1,\"popupHeight\":76},"
"{\"type\":\"comboBox\",\"id\":\"disabled\",\"style\":\"combo\",\"items\":[\"On\",\"Off\"],\"selected\":0,\"enabled\":false},"
"{\"type\":\"comboBox\",\"id\":\"clip\",\"style\":\"combo\",\"items\":[\"Short\",\"Selected text should clip before the V button inset\"],\"selected\":1},"
"{\"type\":\"comboBox\",\"id\":\"top\",\"style\":\"combo\",\"items\":[\"North\",\"South\",\"East\",\"West\",\"Center\",\"Edge\",\"Corner\"],\"selected\":1,\"popupPlacement\":\"top\",\"popupMaxHeight\":160}"
"]}"
"]}}";

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
			printf("xui_combobox_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui_combobox_xson font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static int CreateUI(app_state_t* pApp)
{
	if ( pApp->bFontReady ) {
		xgeXuiTokenSetFont(&pApp->tXui, "body", &pApp->tFont);
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_combobox_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_widget pBasicWidget;
	xge_xui_widget pRichWidget;
	xge_xui_widget pFixedWidget;
	xge_xui_widget pDisabledWidget;
	xge_xui_widget pClipWidget;
	xge_xui_widget pTopWidget;
	xge_xui_combo_box pBasic;
	xge_xui_combo_box pRich;
	xge_xui_combo_box pFixed;
	xge_xui_combo_box pDisabled;
	xge_xui_combo_box pClip;
	xge_xui_combo_box pTop;
	xge_event_t tEvent;
	float fX;
	float fY;

	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pBasicWidget = xgeXuiPageFind(&pApp->tPage, "basic");
	pRichWidget = xgeXuiPageFind(&pApp->tPage, "rich");
	pFixedWidget = xgeXuiPageFind(&pApp->tPage, "fixed");
	pDisabledWidget = xgeXuiPageFind(&pApp->tPage, "disabled");
	pClipWidget = xgeXuiPageFind(&pApp->tPage, "clip");
	pTopWidget = xgeXuiPageFind(&pApp->tPage, "top");
	pBasic = (pBasicWidget != NULL) ? (xge_xui_combo_box)pBasicWidget->pUser : NULL;
	pRich = (pRichWidget != NULL) ? (xge_xui_combo_box)pRichWidget->pUser : NULL;
	pFixed = (pFixedWidget != NULL) ? (xge_xui_combo_box)pFixedWidget->pUser : NULL;
	pDisabled = (pDisabledWidget != NULL) ? (xge_xui_combo_box)pDisabledWidget->pUser : NULL;
	pClip = (pClipWidget != NULL) ? (xge_xui_combo_box)pClipWidget->pUser : NULL;
	pTop = (pTopWidget != NULL) ? (xge_xui_combo_box)pTopWidget->pUser : NULL;

	pApp->bCreateOK = (pBasic != NULL) && (pRich != NULL) && (pFixed != NULL) && (pDisabled != NULL) && (pClip != NULL) && (pTop != NULL);
	pApp->bStateOK =
		(pBasic != NULL) && (xgeXuiComboBoxGetSelected(pBasic) == 2) &&
		(pRich != NULL) && (xgeXuiComboBoxGetSelectedValue(pRich) == 30) &&
		(pClip != NULL) && (xgeXuiComboBoxGetSelected(pClip) == 1) &&
		(pDisabledWidget != NULL) && ((pDisabledWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0);
	pApp->bMetricsOK =
		(pBasic != NULL) && (pBasic->fItemHeight == 24.0f) &&
		(pFixed != NULL) && (pFixed->fPopupHeight == 76.0f) &&
		(pTop != NULL) && (pTop->fPopupMaxHeight == 160.0f);

	if ( (pTop != NULL) && (pTopWidget != NULL) ) {
		fX = pTopWidget->tRect.fX + pTopWidget->tRect.fW * 0.5f;
		fY = pTopWidget->tRect.fY + pTopWidget->tRect.fH * 0.5f;
		MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
		xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
		xgeXuiUpdate(&pApp->tXui, 0.0f);
		pApp->bTopOK = xgeXuiComboBoxIsOpen(pTop) && (pTop->pPopupWidget->tRect.fY < pTopWidget->tRect.fY);
	}
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

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiPageUnload(&pApp->tPage);
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

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui_combobox_xson final-summary frames=%d create=%d state=%d metrics=%d top=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bStateOK,
			pApp->bMetricsOK,
			pApp->bTopOK);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_COMBOBOX_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 360;
	tDesc.sTitle = "XUI ComboBox XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bStateOK && tApp.bMetricsOK && tApp.bTopOK) ? 0 : 3;
}
