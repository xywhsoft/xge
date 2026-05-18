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
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"grid\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[20,20,20,20],\"columns\":2,\"rowHeight\":248,\"columnGap\":12,\"rowGap\":12,\"background\":\"#E5EBF4FF\"},"
"\"label\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"height\":24,\"textColor\":\"#303A48FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"panel\":{\"type\":\"panel\",\"font\":\"@fonts.body\",\"width\":\"100%\",\"height\":\"100%\",\"clientLayout\":\"column\",\"clientPadding\":[8,8,8,8],\"clientGap\":6},"
"\"customPanel\":{\"@parent\":\"panel\",\"headerColor\":\"#2A70A8FF\",\"titleColor\":\"#F8FCFFFF\",\"clientColor\":\"#ECF8FCFF\"}"
"},"
"\"tree\":{\"type\":\"grid\",\"style\":\"root\",\"children\":["
"{\"type\":\"panel\",\"style\":\"panel\",\"title\":\"Default panel\",\"children\":["
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Header and client are composed widgets.\"},"
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"XSON children are attached to the client.\"}"
"]},"
"{\"type\":\"panel\",\"style\":\"panel\",\"title\":\"Icon title\",\"icon\":\"@textures.icon\",\"iconSrc\":[0,0,18,18],\"clientLayout\":\"row\",\"children\":["
"{\"type\":\"button\",\"font\":\"@fonts.body\",\"text\":\"Apply\",\"height\":34,\"color\":\"#387ECCFF\",\"hoverColor\":\"#4692DEFF\",\"activeColor\":\"#2460AAFF\",\"textColor\":\"#F8FCFFFF\"},"
"{\"type\":\"button\",\"font\":\"@fonts.body\",\"text\":\"Reset\",\"height\":34,\"color\":\"#607084FF\",\"hoverColor\":\"#74869EFF\",\"activeColor\":\"#48586EFF\",\"textColor\":\"#F8FCFFFF\"}"
"]},"
"{\"type\":\"panel\",\"style\":\"customPanel\",\"title\":\"Custom colors\",\"children\":["
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Header, title and client colors are set by properties.\"},"
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Client layout still uses normal widget layout.\"}"
"]},"
"{\"type\":\"panel\",\"style\":\"panel\",\"title\":\"Disabled and clipped\",\"headerHeight\":24,\"enabled\":false,\"children\":["
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Disabled state comes from widget.\"},"
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Client content remains clipped inside the panel.\"},"
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Extra row for branch coverage.\"}"
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
			printf("xui_panel_xson font loaded: %s\n", arrFonts[i]);
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
	unsigned char arrPixels[18 * 18 * 4];
	int x;
	int y;
	uint32_t iColor;

	for ( y = 0; y < 18; y++ ) {
		for ( x = 0; x < 18; x++ ) {
			iColor = XGE_COLOR_RGBA(0, 0, 0, 0);
			if ( x >= 2 && x <= 15 && y >= 2 && y <= 15 ) {
				iColor = XGE_COLOR_RGBA(48, 126, 214, 255);
			}
			if ( (x >= 5 && x <= 12 && y >= 5 && y <= 12) || (x >= 8 && x <= 9 && y >= 3 && y <= 14) || (y >= 8 && y <= 9 && x >= 3 && x <= 14) ) {
				iColor = XGE_COLOR_RGBA(240, 248, 255, 255);
			}
			WritePixel(arrPixels, 18, x, y, iColor);
		}
	}
	return xgeTextureCreateRGBA(pTexture, 18, 18, arrPixels);
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ||
	     xgeXuiTokenSetTexture(&pApp->tXui, "icon", &pApp->tIcon) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_panel_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( iWidth == pApp->iLastWidth && iHeight == pApp->iLastHeight ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 900.0f ) {
		fRootW = 900.0f;
	}
	if ( fRootH < 560.0f ) {
		fRootH = 560.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	pApp->bCreateOK = (pApp->tPage.iPanelCount == 4) && (pApp->tPage.iLabelCount == 7) && (pApp->tPage.iButtonCount == 2);
	pApp->bLayoutOK = xgeXuiRoot(&pApp->tXui)->tRect.fW >= 900.0f;
	pApp->bStateOK = (xgeXuiPanelGetClientWidget(&pApp->tPage.arrPanel[0]) != NULL) &&
		(pApp->tPage.arrPanel[1].tIconImage.pTexture == &pApp->tIcon) &&
		(pApp->tPage.arrPanel[2].iHeaderColor == XGE_COLOR_RGBA(42, 112, 168, 255)) &&
		((pApp->tPage.arrPanel[3].pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( CreateIconTexture(&pApp->tIcon) != XGE_OK || xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
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
	xgeTextureFree(&pApp->tIcon);
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
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_panel_xson final-summary frames=%d create=%d layout=%d state=%d panels=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->tPage.iPanelCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_PANEL_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 920;
	tDesc.iHeight = 580;
	tDesc.sTitle = "XUI Panel XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK) ? 0 : 3;
}
