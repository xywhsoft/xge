#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LABEL_COUNT 10
#define PROGRESS_COUNT 10
#define TEXTURE_COUNT 6

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
	xge_texture_t tTexture[TEXTURE_COUNT];
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	float fProgress;
	float fPause;
} app_state_t;

static const char* g_arrProgressIds[PROGRESS_COUNT] = { "p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9" };

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[28,18,28,18],\"gap\":5,\"background\":\"#E5EBF4FF\"},"
"\"label\":{\"font\":\"@fonts.body\",\"textColor\":\"#2A3444FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\",\"width\":\"100%\",\"height\":20,\"padding\":[2,0,2,0]},"
"\"progress\":{\"width\":\"100%\",\"height\":24,\"padding\":3,\"background\":\"#F7FAFDFF\",\"borderColor\":\"#9AB0CAFF\",\"borderWidth\":1,\"trackColor\":\"#D7E8F4FF\",\"fillColor\":\"#2E7CD6FF\",\"textColor\":\"#243446FF\",\"fillTextColor\":\"#FFFFFFFF\",\"font\":\"@fonts.body\"},"
"\"custom\":{\"width\":\"100%\",\"height\":24,\"padding\":3,\"background\":\"#FFF9EEFF\",\"borderColor\":\"#D37624FF\",\"borderWidth\":2,\"trackColor\":\"#F8E2BCFF\",\"fillColor\":\"#D37624FF\",\"textColor\":\"#3F2C16FF\",\"fillTextColor\":\"#FFFFFFFF\",\"font\":\"@fonts.body\"},"
"\"plain\":{\"width\":\"100%\",\"height\":24,\"padding\":3,\"background\":\"#00000000\",\"borderColor\":\"#00000000\",\"borderWidth\":0,\"trackColor\":\"#D3E3EFFF\",\"fillColor\":\"#34976FFF\",\"textColor\":\"#243446FF\",\"fillTextColor\":\"#FFFFFFFF\",\"font\":\"@fonts.body\"},"
"\"imageStretchProgress\":{\"width\":\"100%\",\"height\":24,\"padding\":3,\"background\":\"#00000000\",\"borderColor\":\"#00000000\",\"borderWidth\":0,\"textColor\":\"#243446FF\",\"fillTextColor\":\"#FFFFFFFF\",\"font\":\"@fonts.body\",\"fillPatchMode\":\"stretch\",\"trackPatch\":{\"texture\":\"@textures.imgTrack\",\"mode\":\"stretch\"},\"fillPatch\":{\"texture\":\"@textures.imgFill\",\"mode\":\"stretch\"}},"
"\"imageRevealProgress\":{\"width\":\"100%\",\"height\":24,\"padding\":3,\"background\":\"#00000000\",\"borderColor\":\"#00000000\",\"borderWidth\":0,\"textColor\":\"#243446FF\",\"fillTextColor\":\"#FFFFFFFF\",\"font\":\"@fonts.body\",\"fillPatchMode\":\"reveal\",\"trackPatch\":{\"texture\":\"@textures.imgTrack\",\"mode\":\"stretch\"},\"fillPatch\":{\"texture\":\"@textures.imgFill\",\"mode\":\"stretch\"}},"
"\"stretchProgress\":{\"width\":\"100%\",\"height\":24,\"padding\":3,\"background\":\"#00000000\",\"borderColor\":\"#00000000\",\"borderWidth\":0,\"textColor\":\"#243446FF\",\"fillTextColor\":\"#FFFFFFFF\",\"font\":\"@fonts.body\",\"trackPatch\":{\"texture\":\"@textures.roundTrack\",\"center\":[0.375,0.25,0.625,0.75],\"mode\":\"stretch\"},\"fillPatch\":{\"texture\":\"@textures.roundFill\",\"center\":[0.375,0.25,0.625,0.75],\"mode\":\"stretch\"}},"
"\"tileProgress\":{\"width\":\"100%\",\"height\":24,\"padding\":3,\"background\":\"#00000000\",\"borderColor\":\"#00000000\",\"borderWidth\":0,\"textColor\":\"#243446FF\",\"fillTextColor\":\"#FFFFFFFF\",\"font\":\"@fonts.body\",\"trackPatch\":{\"texture\":\"@textures.tileTrack\",\"center\":[0.375,0.25,0.625,0.75],\"mode\":\"tile\"},\"fillPatch\":{\"texture\":\"@textures.tileFill\",\"center\":[0.375,0.25,0.625,0.75],\"mode\":\"tile\"}}"
"},"
"\"tree\":{\"type\":\"column\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"id\":\"l0\",\"style\":\"label\",\"text\":\"空白进度条\"},"
"{\"type\":\"progress\",\"id\":\"p0\",\"style\":\"progress\",\"min\":0,\"max\":100,\"value\":0},"
"{\"type\":\"label\",\"id\":\"l1\",\"style\":\"label\",\"text\":\"显示百分比\"},"
"{\"type\":\"progress\",\"id\":\"p1\",\"style\":\"progress\",\"min\":0,\"max\":100,\"value\":0,\"text\":\"%1.0f%%\"},"
"{\"type\":\"label\",\"id\":\"l2\",\"style\":\"label\",\"text\":\"固定字符串\"},"
"{\"type\":\"progress\",\"id\":\"p2\",\"style\":\"progress\",\"min\":0,\"max\":100,\"value\":0,\"text\":\"Loading\"},"
"{\"type\":\"label\",\"id\":\"l3\",\"style\":\"label\",\"text\":\"自定义内容\"},"
"{\"type\":\"progress\",\"id\":\"p3\",\"style\":\"progress\",\"min\":0,\"max\":100,\"value\":0,\"text\":\"进度：%1.2f\"},"
"{\"type\":\"label\",\"id\":\"l4\",\"style\":\"label\",\"text\":\"自定义配色\"},"
"{\"type\":\"progress\",\"id\":\"p4\",\"style\":\"custom\",\"min\":0,\"max\":100,\"value\":0,\"text\":\"%1.0f%%\"},"
"{\"type\":\"label\",\"id\":\"l5\",\"style\":\"label\",\"text\":\"不显示边框\"},"
"{\"type\":\"progress\",\"id\":\"p5\",\"style\":\"plain\",\"min\":0,\"max\":100,\"value\":0,\"text\":\"%1.0f%%\"},"
"{\"type\":\"label\",\"id\":\"l6\",\"style\":\"label\",\"text\":\"图片拉伸进度条\"},"
"{\"type\":\"progress\",\"id\":\"p6\",\"style\":\"imageStretchProgress\",\"min\":0,\"max\":100,\"value\":0,\"text\":\"%1.0f%%\"},"
"{\"type\":\"label\",\"id\":\"l7\",\"style\":\"label\",\"text\":\"图片裁剪进度条\"},"
"{\"type\":\"progress\",\"id\":\"p7\",\"style\":\"imageRevealProgress\",\"min\":0,\"max\":100,\"value\":0,\"text\":\"%1.0f%%\"},"
"{\"type\":\"label\",\"id\":\"l8\",\"style\":\"label\",\"text\":\"九宫格拉伸\"},"
"{\"type\":\"progress\",\"id\":\"p8\",\"style\":\"stretchProgress\",\"min\":0,\"max\":100,\"value\":0,\"text\":\"%1.0f%%\"},"
"{\"type\":\"label\",\"id\":\"l9\",\"style\":\"label\",\"text\":\"九宫格平铺\"},"
"{\"type\":\"progress\",\"id\":\"p9\",\"style\":\"tileProgress\",\"min\":0,\"max\":100,\"value\":0,\"text\":\"%1.0f%%\"}"
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

static uint32_t BlendColor(uint32_t iA, uint32_t iB, int iStep, int iTotal)
{
	int r;
	int g;
	int b;
	int a;

	if ( iTotal <= 0 ) {
		return iA;
	}
	r = (int)XGE_COLOR_GET_R(iA) + ((int)XGE_COLOR_GET_R(iB) - (int)XGE_COLOR_GET_R(iA)) * iStep / iTotal;
	g = (int)XGE_COLOR_GET_G(iA) + ((int)XGE_COLOR_GET_G(iB) - (int)XGE_COLOR_GET_G(iA)) * iStep / iTotal;
	b = (int)XGE_COLOR_GET_B(iA) + ((int)XGE_COLOR_GET_B(iB) - (int)XGE_COLOR_GET_B(iA)) * iStep / iTotal;
	a = (int)XGE_COLOR_GET_A(iA) + ((int)XGE_COLOR_GET_A(iB) - (int)XGE_COLOR_GET_A(iA)) * iStep / iTotal;
	return XGE_COLOR_RGBA(r, g, b, a);
}

static int CreateBarTexture(xge_texture pTexture, int iWidth, int iHeight, uint32_t iTop, uint32_t iBottom, uint32_t iBorder, int bRounded, int bPattern)
{
	unsigned char arrPixels[64 * 24 * 4];
	int iRadius;
	int x;
	int y;
	int dx;
	int dy;
	int iInside;
	uint32_t iColor;

	iRadius = iHeight / 2;
	for ( y = 0; y < iHeight; y++ ) {
		for ( x = 0; x < iWidth; x++ ) {
			iInside = 1;
			if ( bRounded ) {
				if ( x < iRadius ) {
					dx = iRadius - x - 1;
					dy = iRadius - y - 1;
					iInside = (dx * dx + dy * dy) <= (iRadius * iRadius);
				} else if ( x >= iWidth - iRadius ) {
					dx = x - (iWidth - iRadius);
					dy = iRadius - y - 1;
					iInside = (dx * dx + dy * dy) <= (iRadius * iRadius);
				}
			}
			if ( !iInside ) {
				iColor = XGE_COLOR_RGBA(0, 0, 0, 0);
				arrPixels[((y * iWidth + x) * 4) + 0] = (unsigned char)XGE_COLOR_GET_R(iColor);
				arrPixels[((y * iWidth + x) * 4) + 1] = (unsigned char)XGE_COLOR_GET_G(iColor);
				arrPixels[((y * iWidth + x) * 4) + 2] = (unsigned char)XGE_COLOR_GET_B(iColor);
				arrPixels[((y * iWidth + x) * 4) + 3] = (unsigned char)XGE_COLOR_GET_A(iColor);
				continue;
			}
			iColor = BlendColor(iTop, iBottom, y, iHeight - 1);
			if ( bPattern && ((x / 4) % 2 == 0) && (x > iRadius) && (x < iWidth - iRadius) ) {
				iColor = BlendColor(iColor, XGE_COLOR_RGBA(255, 255, 255, 255), 1, 5);
			}
			if ( (y == 0) || (y == iHeight - 1) || (x == 0) || (x == iWidth - 1) ) {
				iColor = iBorder;
			}
			arrPixels[((y * iWidth + x) * 4) + 0] = (unsigned char)XGE_COLOR_GET_R(iColor);
			arrPixels[((y * iWidth + x) * 4) + 1] = (unsigned char)XGE_COLOR_GET_G(iColor);
			arrPixels[((y * iWidth + x) * 4) + 2] = (unsigned char)XGE_COLOR_GET_B(iColor);
			arrPixels[((y * iWidth + x) * 4) + 3] = (unsigned char)XGE_COLOR_GET_A(iColor);
		}
	}
	return xgeTextureCreateRGBA(pTexture, iWidth, iHeight, arrPixels);
}

static int CreateTextures(app_state_t* pApp)
{
	unsigned char arrReadback[32 * 12 * 4];

	if ( CreateBarTexture(&pApp->tTexture[0], 32, 12, XGE_COLOR_RGBA(208, 232, 246, 255), XGE_COLOR_RGBA(186, 217, 236, 255), XGE_COLOR_RGBA(128, 170, 196, 255), 0, 0) != XGE_OK ||
		CreateBarTexture(&pApp->tTexture[1], 32, 12, XGE_COLOR_RGBA(72, 150, 222, 255), XGE_COLOR_RGBA(35, 110, 196, 255), XGE_COLOR_RGBA(18, 82, 160, 255), 0, 0) != XGE_OK ||
		CreateBarTexture(&pApp->tTexture[2], 48, 18, XGE_COLOR_RGBA(218, 236, 246, 255), XGE_COLOR_RGBA(194, 222, 238, 255), XGE_COLOR_RGBA(130, 174, 202, 255), 1, 0) != XGE_OK ||
		CreateBarTexture(&pApp->tTexture[3], 48, 18, XGE_COLOR_RGBA(75, 166, 230, 255), XGE_COLOR_RGBA(33, 126, 206, 255), XGE_COLOR_RGBA(19, 88, 168, 255), 1, 0) != XGE_OK ||
		CreateBarTexture(&pApp->tTexture[4], 48, 18, XGE_COLOR_RGBA(226, 235, 226, 255), XGE_COLOR_RGBA(198, 218, 204, 255), XGE_COLOR_RGBA(126, 164, 140, 255), 1, 1) != XGE_OK ||
		CreateBarTexture(&pApp->tTexture[5], 48, 18, XGE_COLOR_RGBA(85, 180, 136, 255), XGE_COLOR_RGBA(43, 146, 98, 255), XGE_COLOR_RGBA(26, 105, 72, 255), 1, 1) != XGE_OK ) {
		return XGE_ERROR;
	}
	memset(arrReadback, 0, sizeof(arrReadback));
	if ( xgeTextureReadPixels(&pApp->tTexture[1], arrReadback, 32 * 4) != XGE_OK ||
		arrReadback[((2 * 32) + 2) * 4 + 0] != 66 ||
		arrReadback[((2 * 32) + 2) * 4 + 1] != 143 ||
		arrReadback[((2 * 32) + 2) * 4 + 2] != 218 ||
		arrReadback[((2 * 32) + 2) * 4 + 3] != 255 ) {
		printf("xui_progress_xson texture byte check failed: %u,%u,%u,%u\n", (unsigned)arrReadback[((2 * 32) + 2) * 4 + 0], (unsigned)arrReadback[((2 * 32) + 2) * 4 + 1], (unsigned)arrReadback[((2 * 32) + 2) * 4 + 2], (unsigned)arrReadback[((2 * 32) + 2) * 4 + 3]);
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_progress_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ||
		xgeXuiTokenSetTexture(&pApp->tXui, "imgTrack", &pApp->tTexture[0]) != XGE_OK ||
		xgeXuiTokenSetTexture(&pApp->tXui, "imgFill", &pApp->tTexture[1]) != XGE_OK ||
		xgeXuiTokenSetTexture(&pApp->tXui, "roundTrack", &pApp->tTexture[2]) != XGE_OK ||
		xgeXuiTokenSetTexture(&pApp->tXui, "roundFill", &pApp->tTexture[3]) != XGE_OK ||
		xgeXuiTokenSetTexture(&pApp->tXui, "tileTrack", &pApp->tTexture[4]) != XGE_OK ||
		xgeXuiTokenSetTexture(&pApp->tXui, "tileFill", &pApp->tTexture[5]) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_progress_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
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
	pRoot = xgeXuiPageFind(&pApp->tPage, "root");
	if ( pRoot == NULL ) {
		return;
	}
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 720.0f ) {
		fRootW = 720.0f;
	}
	if ( fRootH < 560.0f ) {
		fRootH = 560.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateProgress(app_state_t* pApp, float fDelta)
{
	xge_xui_widget pWidget;
	xge_xui_progress pProgress;
	int i;

	if ( pApp->fProgress >= 100.0f ) {
		pApp->fPause += fDelta;
		if ( pApp->fPause >= 0.75f ) {
			pApp->fProgress = 0.0f;
			pApp->fPause = 0.0f;
		}
	} else {
		pApp->fProgress += fDelta * 34.0f;
		if ( pApp->fProgress > 100.0f ) {
			pApp->fProgress = 100.0f;
		}
	}
	for ( i = 0; i < PROGRESS_COUNT; i++ ) {
		pWidget = xgeXuiPageFind(&pApp->tPage, g_arrProgressIds[i]);
		pProgress = (pWidget != NULL) ? (xge_xui_progress)pWidget->pUser : NULL;
		if ( pProgress != NULL ) {
			xgeXuiProgressSetValue(pProgress, pApp->fProgress);
		}
	}
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_widget pBlank;
	xge_xui_widget pReveal;
	xge_xui_widget pTile;
	xge_xui_progress pBlankProgress;
	xge_xui_progress pRevealProgress;
	xge_xui_progress pTileProgress;

	pRoot = xgeXuiPageFind(&pApp->tPage, "root");
	pBlank = xgeXuiPageFind(&pApp->tPage, "p0");
	pReveal = xgeXuiPageFind(&pApp->tPage, "p7");
	pTile = xgeXuiPageFind(&pApp->tPage, "p9");
	pBlankProgress = (pBlank != NULL) ? (xge_xui_progress)pBlank->pUser : NULL;
	pRevealProgress = (pReveal != NULL) ? (xge_xui_progress)pReveal->pUser : NULL;
	pTileProgress = (pTile != NULL) ? (xge_xui_progress)pTile->pUser : NULL;
	pApp->bCreateOK = (pApp->tPage.iLabelCount == LABEL_COUNT) && (pApp->tPage.iProgressCount == PROGRESS_COUNT);
	pApp->bLayoutOK = (pRoot != NULL) && (pRoot->tRect.fW >= 720.0f);
	pApp->bStateOK = (pBlankProgress != NULL) && (pBlankProgress->sTextTemplate == NULL) && (pRevealProgress != NULL) && (pRevealProgress->iFillPatchMode == XGE_XUI_PROGRESS_FILL_REVEAL) && (pTileProgress != NULL) && (pTileProgress->bHasFillPatch == 1) && (pTileProgress->tFillPatch.iMode == XGE_NINE_PATCH_TILE);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( CreateTextures(pApp) != XGE_OK || xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
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
	int i;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiPageUnload(&pApp->tPage);
	xgeXuiUnit(&pApp->tXui);
	for ( i = 0; i < TEXTURE_COUNT; i++ ) {
		xgeTextureFree(&pApp->tTexture[i]);
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
	return xgeXuiDispatchEvent(&pApp->tXui, pEvent);
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	UpdateProgress(pApp, fDelta);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_progress_xson final-summary frames=%d create=%d layout=%d state=%d labels=%d progress=%d value=%.2f\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->tPage.iLabelCount, pApp->tPage.iProgressCount, pApp->fProgress);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_PROGRESS_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 640;
	tDesc.sTitle = "XUI Progress XSON";
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
