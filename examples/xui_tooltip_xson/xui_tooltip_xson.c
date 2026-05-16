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
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bTooltipOK;
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[24,48,24,24],\"gap\":12,\"background\":\"#E5EBF4FF\"},"
"\"panel\":{\"type\":\"column\",\"width\":360,\"height\":330,\"padding\":[18,18,18,18],\"gap\":12,\"background\":\"#F8FAFDFF\",\"borderWidth\":1,\"borderColor\":\"#64B0E8FF\",\"radius\":4},"
"\"target\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"width\":\"100%\",\"height\":42,\"padding\":[12,8,12,8],\"background\":\"#E8F1FCFF\",\"borderWidth\":1,\"borderColor\":\"#5A8ECEFF\",\"radius\":4,\"textColor\":\"#263242FF\",\"textAlign\":\"center\",\"textVAlign\":\"middle\"}"
"},"
"\"tree\":{\"type\":\"column\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"column\",\"id\":\"panel\",\"style\":\"panel\",\"children\":["
"{\"type\":\"label\",\"id\":\"stringTip\",\"style\":\"target\",\"text\":\"String tooltip\",\"tooltip\":\"XSON shorthand: tooltip string\"},"
"{\"type\":\"label\",\"id\":\"rightTip\",\"style\":\"target\",\"text\":\"Right anchor\",\"tooltip\":{\"text\":\"XSON object tooltip on the right\",\"anchor\":\"right\",\"offsetX\":8,\"offsetY\":0,\"delay\":0}},"
"{\"type\":\"label\",\"id\":\"topTip\",\"style\":\"target\",\"text\":\"Top anchor with delay\",\"tooltip\":{\"text\":\"This tip opens after a short delay\",\"anchor\":\"top\",\"offsetY\":-6,\"delay\":0.25}},"
"{\"type\":\"label\",\"id\":\"cursorTip\",\"style\":\"target\",\"text\":\"Cursor follow\",\"tooltip\":{\"text\":\"Cursor anchored tooltip\",\"anchor\":\"cursor\",\"offsetX\":10,\"offsetY\":12,\"delay\":0,\"followCursor\":true}},"
"{\"type\":\"label\",\"id\":\"disabledTip\",\"style\":\"target\",\"text\":\"Tooltip disabled by XSON\",\"tooltip\":{\"enabled\":false}}"
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

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_tooltip_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_vec2_t WidgetCenter(xge_xui_widget pWidget)
{
	return (xge_vec2_t){ pWidget->tRect.fX + pWidget->tRect.fW * 0.5f, pWidget->tRect.fY + pWidget->tRect.fH * 0.5f };
}

static void MakeMouseMove(xge_event_t* pEvent, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_MOUSE_MOVE;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	int iWidth;
	int iHeight;
	float fW;
	float fH;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (pApp->iLastWidth == iWidth) && (pApp->iLastHeight == iHeight) ) {
		return;
	}
	pRoot = xgeXuiPageFind(&pApp->tPage, "root");
	if ( pRoot == NULL ) {
		return;
	}
	fW = (float)iWidth;
	fH = (float)iHeight;
	if ( fW < 700.0f ) {
		fW = 700.0f;
	}
	if ( fH < 460.0f ) {
		fH = 460.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fW, fH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp, int bExerciseTooltip)
{
	xge_xui_widget pStringTip;
	xge_xui_widget pRightTip;
	xge_xui_widget pCursorTip;
	xge_xui_widget pDisabledTip;
	xge_xui_tooltip_desc pTip;
	xge_event_t tEvent;
	xge_vec2_t tCenter;

	pStringTip = xgeXuiPageFind(&pApp->tPage, "stringTip");
	pRightTip = xgeXuiPageFind(&pApp->tPage, "rightTip");
	pCursorTip = xgeXuiPageFind(&pApp->tPage, "cursorTip");
	pDisabledTip = xgeXuiPageFind(&pApp->tPage, "disabledTip");
	pApp->bCreateOK = (pStringTip != NULL) && (pRightTip != NULL) && (pCursorTip != NULL) && (pDisabledTip != NULL);
	if ( !pApp->bCreateOK ) {
		return;
	}
	pTip = (xge_xui_tooltip_desc)xgeXuiWidgetGetTooltip(pRightTip);
	pApp->bTooltipOK =
		(xgeXuiWidgetGetTooltip(pStringTip)->iType == XGE_XUI_TOOLTIP_TEXT) &&
		(pTip->iAnchor == XGE_XUI_TOOLTIP_ANCHOR_WIDGET_RIGHT) &&
		(pTip->fDelay == 0.0f) &&
		(xgeXuiWidgetGetTooltip(pCursorTip)->bFollowCursor != 0) &&
		(xgeXuiWidgetGetTooltip(pDisabledTip)->iType == XGE_XUI_TOOLTIP_NONE);
	if ( !bExerciseTooltip ) {
		return;
	}
	tCenter = WidgetCenter(pRightTip);
	MakeMouseMove(&tEvent, tCenter.fX, tCenter.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bTooltipOK = pApp->bTooltipOK && (xgeXuiWidgetTooltipIsOpen(&pApp->tXui) != 0);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;
	xge_xui_theme_t tTheme;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	XgeXuiDemoTheme(&tTheme, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_tooltip_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp, pApp->iFrameLimit > 0);
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
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	return xgeXuiDispatchEvent(&pApp->tXui, pEvent);
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunChecks(pApp, pApp->iFrameLimit > 0);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_tooltip_xson final-summary frames=%d create=%d tooltip=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bTooltipOK);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(229, 235, 244, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TOOLTIP_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 700;
	tDesc.iHeight = 460;
	tDesc.sTitle = "XUI Tooltip XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bTooltipOK) ? 0 : 3;
}
