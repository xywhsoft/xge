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
	xge_texture_t tIcon;
	int bFontReady;
	int bIconReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bLayoutOK;
	int bLayerOK;
	int bStateOK;
	int bInteractionOK;
	int bInteractionRan;
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"absolute\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[0,0,0,0],\"background\":\"#ECF0F6FF\"},"
"\"hint\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"width\":660,\"height\":28,\"textColor\":\"#303A48FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"clientLabel\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"height\":24,\"textColor\":\"#364252FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"button\":{\"type\":\"button\",\"font\":\"@fonts.body\",\"height\":32,\"color\":\"#387ECCFF\",\"hoverColor\":\"#4692DEFF\",\"activeColor\":\"#2460AAFF\",\"textColor\":\"#F8FCFFFF\"},"
"\"window\":{\"type\":\"window\",\"font\":\"@fonts.body\",\"clientLayout\":\"column\",\"clientPadding\":[12,10,12,10],\"clientGap\":8,"
"\"background\":\"#F8FCFFFF\",\"clientColor\":\"#F6FAFEFF\",\"titleBarColor\":\"#2F7FD0FF\",\"titleTextColor\":\"#F8FCFFFF\",\"borderColor\":\"#4D92D9FF\","
"\"buttonColor\":\"#D8E8F7FF\",\"buttonHoverColor\":\"#BFD9F0FF\",\"buttonActiveColor\":\"#8FBCDFFF\"},"
"\"topMost\":{\"@parent\":\"window\",\"titleBarColor\":\"#285D91FF\",\"clientColor\":\"#EEF6FCFF\",\"borderColor\":\"#24547EFF\"},"
"\"floating\":{\"@parent\":\"window\",\"showTitleBar\":false,\"dragAnywhere\":true,\"showCollapse\":false,\"showMaximize\":false,\"showClose\":false,\"borderColor\":\"#7BA8C8FF\",\"clientColor\":\"#F4FAFCFF\"},"
"\"collapsed\":{\"@parent\":\"window\",\"resizable\":false,\"showMaximize\":false,\"showClose\":false,\"titleBarColor\":\"#6E7E93FF\",\"borderColor\":\"#7C8CA0FF\"}"
"},"
"\"tree\":{\"type\":\"absolute\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"style\":\"hint\",\"anchor\":{\"left\":24,\"top\":18},\"text\":\"XSON windows use floating overlay, active state and TopMost ordering.\"},"
"{\"type\":\"window\",\"id\":\"normal\",\"style\":\"window\",\"anchor\":{\"left\":54,\"top\":74},\"width\":410,\"height\":260,\"title\":\"Normal document window\",\"icon\":\"@textures.icon\",\"iconSrc\":[0,0,16,16],\"children\":["
"{\"type\":\"label\",\"style\":\"clientLabel\",\"text\":\"Client area uses normal layout.\"},"
"{\"type\":\"label\",\"style\":\"clientLabel\",\"text\":\"Title bar supports icon, buttons, move and resize.\"},"
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Child button does not start drag\"}"
"]},"
"{\"type\":\"window\",\"id\":\"inspector\",\"style\":\"topMost\",\"anchor\":{\"left\":328,\"top\":132},\"width\":360,\"height\":210,\"title\":\"TopMost inspector\",\"topMost\":true,\"icon\":\"@textures.icon\",\"iconSrc\":[0,0,16,16],\"children\":["
"{\"type\":\"label\",\"style\":\"clientLabel\",\"text\":\"This window is TopMost.\"},"
"{\"type\":\"label\",\"style\":\"clientLabel\",\"text\":\"Normal windows cannot cover it.\"},"
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Pinned action\"}"
"]},"
"{\"type\":\"window\",\"id\":\"tool\",\"style\":\"floating\",\"anchor\":{\"left\":112,\"top\":388},\"width\":300,\"height\":150,\"title\":\"Frameless floating tool\",\"children\":["
"{\"type\":\"label\",\"style\":\"clientLabel\",\"text\":\"Frameless windows can drag anywhere.\"},"
"{\"type\":\"label\",\"style\":\"clientLabel\",\"text\":\"Useful for floating tools and palettes.\"}"
"]},"
"{\"type\":\"window\",\"id\":\"collapsed\",\"style\":\"collapsed\",\"anchor\":{\"left\":520,\"top\":410},\"width\":300,\"height\":130,\"title\":\"Collapsed window\",\"collapsed\":true,\"children\":["
"{\"type\":\"label\",\"style\":\"clientLabel\",\"text\":\"Hidden while collapsed.\"}"
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
	const char* arrFonts[] = { "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/Deng.ttf", "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_window_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void WritePixel(unsigned char* pPixels, int iWidth, int x, int y, uint32_t iColor)
{
	pPixels[((y * iWidth + x) * 4) + 0] = (unsigned char)XGE_COLOR_GET_R(iColor);
	pPixels[((y * iWidth + x) * 4) + 1] = (unsigned char)XGE_COLOR_GET_G(iColor);
	pPixels[((y * iWidth + x) * 4) + 2] = (unsigned char)XGE_COLOR_GET_B(iColor);
	pPixels[((y * iWidth + x) * 4) + 3] = (unsigned char)XGE_COLOR_GET_A(iColor);
}

static int CreateIconTexture(xge_texture pTexture)
{
	unsigned char arrPixels[16 * 16 * 4];
	int x;
	int y;
	uint32_t iColor;

	for ( y = 0; y < 16; y++ ) {
		for ( x = 0; x < 16; x++ ) {
			iColor = XGE_COLOR_RGBA(0, 0, 0, 0);
			if ( x >= 2 && x <= 13 && y >= 2 && y <= 13 ) {
				iColor = XGE_COLOR_RGBA(46, 124, 214, 255);
			}
			if ( (x >= 6 && x <= 9 && y >= 4 && y <= 11) || (y >= 6 && y <= 9 && x >= 4 && x <= 11) ) {
				iColor = XGE_COLOR_RGBA(244, 250, 254, 255);
			}
			WritePixel(arrPixels, 16, x, y, iColor);
		}
	}
	return xgeTextureCreateRGBA(pTexture, 16, 16, arrPixels);
}

static int NearFloat(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	return (fD >= -1.0f) && (fD <= 1.0f);
}

static int RectNear(xge_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return NearFloat(tRect.fX, fX) && NearFloat(tRect.fY, fY) && NearFloat(tRect.fW, fW) && NearFloat(tRect.fH, fH);
}

static int SameWindowPos(xge_rect_t tA, xge_rect_t tB)
{
	return NearFloat(tA.fX, tB.fX) && NearFloat(tA.fY, tB.fY);
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
	pEvent->iPointerId = 1;
}

static void DispatchMouse(app_state_t* pApp, int iType, float fX, float fY)
{
	xge_event_t tEvent;

	MakeMouse(&tEvent, iType, fX, fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ||
		xgeXuiTokenSetTexture(&pApp->tXui, "icon", &pApp->tIcon) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_window_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	pApp->bInteractionOK = 1;
	return XGE_OK;
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
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	fW = (float)iWidth;
	fH = (float)iHeight;
	if ( fW < 920.0f ) {
		fW = 920.0f;
	}
	if ( fH < 600.0f ) {
		fH = 600.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fW, fH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_window pNormal;
	xge_xui_window pTopMost;
	xge_xui_window pTool;
	xge_xui_window pCollapsed;

	pNormal = (pApp->tPage.iWindowCount > 0) ? pApp->tPage.arrWindow[0] : NULL;
	pTopMost = (pApp->tPage.iWindowCount > 1) ? pApp->tPage.arrWindow[1] : NULL;
	pTool = (pApp->tPage.iWindowCount > 2) ? pApp->tPage.arrWindow[2] : NULL;
	pCollapsed = (pApp->tPage.iWindowCount > 3) ? pApp->tPage.arrWindow[3] : NULL;
	pApp->bCreateOK = (pApp->tPage.iWindowCount == 4) && (pApp->tPage.iLabelCount >= 8) && (pApp->tPage.iButtonCount == 2);
	pApp->bLayoutOK = (pNormal != NULL) && (pTopMost != NULL) && (pTool != NULL) && (pCollapsed != NULL) &&
		RectNear(pNormal->pWidget->tRect, 54.0f, 74.0f, 410.0f, 260.0f) &&
		RectNear(pTopMost->pWidget->tRect, 328.0f, 132.0f, 360.0f, 210.0f) &&
		RectNear(pTool->pWidget->tRect, 112.0f, 388.0f, 300.0f, 150.0f) &&
		NearFloat(pCollapsed->pWidget->tRect.fX, 520.0f) &&
		NearFloat(pCollapsed->pWidget->tRect.fY, 410.0f) &&
		NearFloat(pCollapsed->pWidget->tRect.fW, 300.0f) &&
		(pCollapsed->pWidget->tRect.fH > 20.0f) &&
		(pCollapsed->pWidget->tRect.fH < 130.0f);
	pApp->bLayerOK = (pNormal != NULL) && (pTopMost != NULL) &&
		(xgeXuiWidgetGetLayer(pNormal->pWidget) == XGE_XUI_LAYER_FLOATING) &&
		(xgeXuiWidgetGetZ(pTopMost->pWidget) > xgeXuiWidgetGetZ(pNormal->pWidget));
	pApp->bStateOK = (pTopMost != NULL) && (pTool != NULL) && (pCollapsed != NULL) &&
		(xgeXuiWindowIsTopMost(pTopMost) != 0) &&
		(pTool->bShowTitleBar == 0) &&
		(pTool->bDragAnywhere != 0) &&
		(pCollapsed->bShowClose == 0) &&
		(xgeXuiWindowIsCollapsed(pCollapsed) != 0) &&
		(xgeXuiWindowGetActive(&pApp->tXui) != NULL);
}

static void RunInteractionChecks(app_state_t* pApp)
{
	xge_xui_window pNormal;
	xge_xui_window pTopMost;
	xge_xui_widget pButton;
	xge_rect_t tBefore;
	xge_rect_t tButton;
	float fX;
	float fY;
	int bTopMostAbove;
	int bWindowDrag;
	int bMaximizeMovesToRoot;
	int bMaximizeRestore;
	int bChildButtonNoDrag;

	if ( (pApp->bInteractionRan != 0) || (pApp->iFrameLimit <= 0) || (pApp->bLayoutOK == 0) ) {
		return;
	}
	pNormal = (pApp->tPage.iWindowCount > 0) ? pApp->tPage.arrWindow[0] : NULL;
	pTopMost = (pApp->tPage.iWindowCount > 1) ? pApp->tPage.arrWindow[1] : NULL;
	pButton = (pApp->tPage.iButtonCount > 0) ? pApp->tPage.arrButton[0].pWidget : NULL;
	if ( (pNormal == NULL) || (pTopMost == NULL) || (pButton == NULL) ) {
		pApp->bInteractionOK = 0;
		pApp->bInteractionRan = 1;
		return;
	}

	fX = pNormal->pWidget->tRect.fX + 80.0f;
	fY = pNormal->pWidget->tRect.fY + 12.0f;
	DispatchMouse(pApp, XGE_EVENT_MOUSE_DOWN, fX, fY);
	DispatchMouse(pApp, XGE_EVENT_MOUSE_UP, fX, fY);
	bTopMostAbove = xgeXuiWidgetGetZ(pTopMost->pWidget) > xgeXuiWidgetGetZ(pNormal->pWidget);

	tBefore = pNormal->pWidget->tRect;
	fX = pNormal->pWidget->tRect.fX + 80.0f;
	fY = pNormal->pWidget->tRect.fY + 12.0f;
	DispatchMouse(pApp, XGE_EVENT_MOUSE_DOWN, fX, fY);
	DispatchMouse(pApp, XGE_EVENT_MOUSE_MOVE, fX + 36.0f, fY + 20.0f);
	DispatchMouse(pApp, XGE_EVENT_MOUSE_UP, fX + 36.0f, fY + 20.0f);
	bWindowDrag = !SameWindowPos(pNormal->pWidget->tRect, tBefore);
	xgeXuiWidgetSetRect(pNormal->pWidget, tBefore);
	xgeXuiWidgetSetSize(pNormal->pWidget, xgeXuiSizePx(tBefore.fW), xgeXuiSizePx(tBefore.fH));
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	tBefore = pNormal->pWidget->tRect;
	xgeXuiWindowSetMaximized(pNormal, 1);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	bMaximizeMovesToRoot = NearFloat(pNormal->pWidget->tRect.fX, 0.0f) &&
		NearFloat(pNormal->pWidget->tRect.fY, 0.0f) &&
		(pNormal->pWidget->tRect.fW > tBefore.fW) &&
		(pNormal->pWidget->tRect.fH > tBefore.fH);
	xgeXuiWindowSetMaximized(pNormal, 0);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	bMaximizeRestore = RectNear(pNormal->pWidget->tRect, tBefore.fX, tBefore.fY, tBefore.fW, tBefore.fH);

	tBefore = pNormal->pWidget->tRect;
	tButton = pButton->tRect;
	fX = tButton.fX + tButton.fW * 0.5f;
	fY = tButton.fY + tButton.fH * 0.5f;
	DispatchMouse(pApp, XGE_EVENT_MOUSE_DOWN, fX, fY);
	DispatchMouse(pApp, XGE_EVENT_MOUSE_MOVE, fX + 36.0f, fY + 20.0f);
	DispatchMouse(pApp, XGE_EVENT_MOUSE_UP, fX + 36.0f, fY + 20.0f);
	bChildButtonNoDrag = SameWindowPos(pNormal->pWidget->tRect, tBefore);

	pApp->bInteractionOK = bTopMostAbove && bWindowDrag && bMaximizeMovesToRoot && bMaximizeRestore && bChildButtonNoDrag;
	pApp->bInteractionRan = 1;
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	pApp->bIconReady = (CreateIconTexture(&pApp->tIcon) == XGE_OK);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiPageUnload(&pApp->tPage);
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bIconReady ) {
		xgeTextureFree(&pApp->tIcon);
	}
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
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunChecks(pApp);
	RunInteractionChecks(pApp);
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_window_xson final-summary frames=%d create=%d layout=%d layer=%d state=%d interaction=%d windows=%d active=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bLayerOK,
			pApp->bStateOK,
			pApp->bInteractionOK,
			pApp->tPage.iWindowCount,
			xgeXuiWindowGetActive(&pApp->tXui) != NULL);
		if ( pApp->bLayoutOK == 0 && pApp->tPage.iWindowCount >= 4 ) {
			printf("xui_window_xson layout-debug normal=%.1f,%.1f,%.1f,%.1f topmost=%.1f,%.1f,%.1f,%.1f tool=%.1f,%.1f,%.1f,%.1f collapsed=%.1f,%.1f,%.1f,%.1f\n",
				pApp->tPage.arrWindow[0]->pWidget->tRect.fX, pApp->tPage.arrWindow[0]->pWidget->tRect.fY, pApp->tPage.arrWindow[0]->pWidget->tRect.fW, pApp->tPage.arrWindow[0]->pWidget->tRect.fH,
				pApp->tPage.arrWindow[1]->pWidget->tRect.fX, pApp->tPage.arrWindow[1]->pWidget->tRect.fY, pApp->tPage.arrWindow[1]->pWidget->tRect.fW, pApp->tPage.arrWindow[1]->pWidget->tRect.fH,
				pApp->tPage.arrWindow[2]->pWidget->tRect.fX, pApp->tPage.arrWindow[2]->pWidget->tRect.fY, pApp->tPage.arrWindow[2]->pWidget->tRect.fW, pApp->tPage.arrWindow[2]->pWidget->tRect.fH,
				pApp->tPage.arrWindow[3]->pWidget->tRect.fX, pApp->tPage.arrWindow[3]->pWidget->tRect.fY, pApp->tPage.arrWindow[3]->pWidget->tRect.fW, pApp->tPage.arrWindow[3]->pWidget->tRect.fH);
		}
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(236, 240, 246, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_WINDOW_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 940;
	tDesc.iHeight = 620;
	tDesc.sTitle = "XUI Window XSON";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC | XGE_INIT_RESIZABLE;
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bLayerOK && tApp.bStateOK && tApp.bInteractionOK) ? 0 : 3;
}
