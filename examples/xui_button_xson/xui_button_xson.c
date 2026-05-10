#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXTURE_COUNT 4

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
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[22,18,22,18],\"gap\":9,\"background\":\"#E5EBF4FF\"},"
"\"label\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"height\":28,\"textColor\":\"#263040FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"row7\":{\"type\":\"grid\",\"width\":\"100%\",\"height\":46,\"columns\":7,\"rowHeight\":46,\"columnGap\":10,\"rowGap\":10},"
"\"row6\":{\"type\":\"grid\",\"width\":\"100%\",\"height\":58,\"columns\":6,\"rowHeight\":58,\"columnGap\":10,\"rowGap\":10},"
"\"row4\":{\"type\":\"grid\",\"width\":\"100%\",\"height\":66,\"columns\":4,\"rowHeight\":66,\"columnGap\":10,\"rowGap\":10},"
"\"button\":{\"type\":\"button\",\"font\":\"@fonts.body\",\"width\":\"100%\",\"height\":42,\"padding\":[10,6,10,6],\"radius\":4,\"borderWidth\":1,\"textColor\":\"#FFFFFFFF\",\"selectedColor\":\"#1E744AFF\"},"
"\"blue\":{\"@parent\":\"button\",\"color\":\"#3874BEFF\",\"hoverColor\":\"#488CD8FF\",\"activeColor\":\"#245CA4FF\",\"borderColor\":\"#1C467CFF\"},"
"\"imageButton\":{\"@parent\":\"button\",\"color\":\"#00000000\",\"hoverColor\":\"#00000000\",\"activeColor\":\"#00000000\",\"borderColor\":\"#00000000\",\"patch\":{\"texture\":\"@textures.img\",\"src\":[0,0,96,40],\"color\":\"#EBF4FFFF\"},\"hoverPatch\":{\"texture\":\"@textures.img\",\"src\":[0,0,96,40],\"color\":\"#FFF691FF\"},\"activePatch\":{\"texture\":\"@textures.img\",\"src\":[0,0,96,40],\"color\":\"#5C9CFFFF\"},\"disabledPatch\":{\"texture\":\"@textures.img\",\"src\":[0,0,96,40],\"color\":\"#969AA096\"},\"selectedPatch\":{\"texture\":\"@textures.img\",\"src\":[0,0,96,40],\"color\":\"#FFC43AFF\"}},"
"\"nineStretch\":{\"@parent\":\"button\",\"color\":\"#00000000\",\"hoverColor\":\"#00000000\",\"activeColor\":\"#00000000\",\"borderColor\":\"#00000000\",\"patch\":{\"texture\":\"@textures.nine\",\"src\":[0,0,64,32],\"center\":[0.25,0.25,0.75,0.75],\"mode\":\"stretch\",\"color\":\"#EBF4FFFF\"},\"hoverPatch\":{\"texture\":\"@textures.nine\",\"src\":[0,0,64,32],\"center\":[0.25,0.25,0.75,0.75],\"mode\":\"stretch\",\"color\":\"#FFF691FF\"},\"activePatch\":{\"texture\":\"@textures.nine\",\"src\":[0,0,64,32],\"center\":[0.25,0.25,0.75,0.75],\"mode\":\"stretch\",\"color\":\"#5C9CFFFF\"},\"disabledPatch\":{\"texture\":\"@textures.nine\",\"src\":[0,0,64,32],\"center\":[0.25,0.25,0.75,0.75],\"mode\":\"stretch\",\"color\":\"#969AA096\"},\"selectedPatch\":{\"texture\":\"@textures.nine\",\"src\":[0,0,64,32],\"center\":[0.25,0.25,0.75,0.75],\"mode\":\"stretch\",\"color\":\"#FFC43AFF\"}},"
"\"nineTile\":{\"@parent\":\"button\",\"color\":\"#00000000\",\"hoverColor\":\"#00000000\",\"activeColor\":\"#00000000\",\"borderColor\":\"#00000000\",\"patch\":{\"texture\":\"@textures.tile\",\"src\":[0,0,64,32],\"center\":[0.25,0.25,0.75,0.75],\"mode\":\"tile\",\"color\":\"#EBF4FFFF\"},\"hoverPatch\":{\"texture\":\"@textures.tile\",\"src\":[0,0,64,32],\"center\":[0.25,0.25,0.75,0.75],\"mode\":\"tile\",\"color\":\"#FFF691FF\"},\"activePatch\":{\"texture\":\"@textures.tile\",\"src\":[0,0,64,32],\"center\":[0.25,0.25,0.75,0.75],\"mode\":\"tile\",\"color\":\"#5C9CFFFF\"},\"disabledPatch\":{\"texture\":\"@textures.tile\",\"src\":[0,0,64,32],\"center\":[0.25,0.25,0.75,0.75],\"mode\":\"tile\",\"color\":\"#969AA096\"},\"selectedPatch\":{\"texture\":\"@textures.tile\",\"src\":[0,0,64,32],\"center\":[0.25,0.25,0.75,0.75],\"mode\":\"tile\",\"color\":\"#FFC43AFF\"}}"
"},"
"\"tree\":{\"type\":\"column\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Seven color schemes\"},"
"{\"type\":\"grid\",\"style\":\"row7\",\"children\":["
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Red\",\"color\":\"#D44448FF\",\"hoverColor\":\"#DC696DFF\",\"activeColor\":\"#AA363AFF\",\"borderColor\":\"#7A2428FF\"},"
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Orange\",\"color\":\"#DC7B26FF\",\"hoverColor\":\"#E6964FFF\",\"activeColor\":\"#B0601DFF\",\"borderColor\":\"#7C4218FF\"},"
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Yellow\",\"color\":\"#CEA830FF\",\"hoverColor\":\"#DABC58FF\",\"activeColor\":\"#A88725FF\",\"borderColor\":\"#7A6118FF\"},"
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Green\",\"color\":\"#46A45CFF\",\"hoverColor\":\"#68B879FF\",\"activeColor\":\"#348248FF\",\"borderColor\":\"#255D35FF\"},"
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Cyan\",\"color\":\"#22A2AEFF\",\"hoverColor\":\"#4BB5BEFF\",\"activeColor\":\"#1B818AFF\",\"borderColor\":\"#175C62FF\"},"
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Blue\",\"color\":\"#367EDAFF\",\"hoverColor\":\"#5A95E0FF\",\"activeColor\":\"#2A65AEFF\",\"borderColor\":\"#214A80FF\"},"
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Purple\",\"color\":\"#8A5CCAFF\",\"hoverColor\":\"#A07AD4FF\",\"activeColor\":\"#6E49A2FF\",\"borderColor\":\"#503679FF\"}"
"]},"
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Icon and text layout, selectable buttons\"},"
"{\"type\":\"grid\",\"style\":\"row7\",\"children\":["
"{\"type\":\"button\",\"style\":\"blue\",\"text\":\"Left\",\"icon\":\"@textures.icon\",\"iconSrc\":[0,0,32,32],\"iconPlacement\":\"left\",\"iconSize\":18,\"iconGap\":5},"
"{\"type\":\"button\",\"style\":\"blue\",\"text\":\"Right\",\"icon\":\"@textures.icon\",\"iconSrc\":[0,0,32,32],\"iconPlacement\":\"right\",\"iconSize\":18,\"iconGap\":5},"
"{\"type\":\"button\",\"style\":\"blue\",\"text\":\"Top\",\"icon\":\"@textures.icon\",\"iconSrc\":[0,0,32,32],\"iconPlacement\":\"top\",\"iconSize\":18,\"iconGap\":5},"
"{\"type\":\"button\",\"style\":\"blue\",\"text\":\"Bottom\",\"icon\":\"@textures.icon\",\"iconSrc\":[0,0,32,32],\"iconPlacement\":\"bottom\",\"iconSize\":18,\"iconGap\":5},"
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Select\",\"color\":\"#3A8258FF\",\"hoverColor\":\"#469A68FF\",\"activeColor\":\"#246844FF\",\"selectedColor\":\"#12603EFF\",\"borderColor\":\"#1D5035FF\",\"borderWidth\":2,\"checkedBorderColor\":\"#FFD236FF\",\"selectable\":true,\"selected\":true},"
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Toggle\",\"color\":\"#607084FF\",\"hoverColor\":\"#74869EFF\",\"activeColor\":\"#48586EFF\",\"selectedColor\":\"#205CB0FF\",\"borderColor\":\"#344052FF\",\"borderWidth\":2,\"checkedBorderColor\":\"#FFD236FF\",\"selectable\":true},"
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Disabled\",\"color\":\"#607084FF\",\"disabledColor\":\"#6D7888FF\",\"textColor\":\"#E8ECF2B4\",\"borderColor\":\"#3A4350FF\",\"enabled\":false}"
"]},"
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Image button, nine-patch button and badge\"},"
"{\"type\":\"grid\",\"style\":\"row4\",\"children\":["
"{\"type\":\"button\",\"style\":\"imageButton\",\"text\":\"Image\"},"
"{\"type\":\"button\",\"style\":\"nineStretch\",\"text\":\"9-Slice\"},"
"{\"type\":\"button\",\"style\":\"nineTile\",\"text\":\"9-Slice Tile\"},"
"{\"type\":\"button\",\"style\":\"blue\",\"text\":\"Badge\",\"badge\":{\"visible\":true,\"size\":12}}"
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

static void WritePixel(unsigned char* pPixels, int iWidth, int x, int y, uint32_t iColor)
{
	pPixels[((y * iWidth + x) * 4) + 0] = (unsigned char)XGE_COLOR_GET_R(iColor);
	pPixels[((y * iWidth + x) * 4) + 1] = (unsigned char)XGE_COLOR_GET_G(iColor);
	pPixels[((y * iWidth + x) * 4) + 2] = (unsigned char)XGE_COLOR_GET_B(iColor);
	pPixels[((y * iWidth + x) * 4) + 3] = (unsigned char)XGE_COLOR_GET_A(iColor);
}

static int CreateIconTexture(xge_texture pTexture)
{
	unsigned char arrPixels[32 * 32 * 4];
	int x;
	int y;
	uint32_t iColor;

	for ( y = 0; y < 32; y++ ) {
		for ( x = 0; x < 32; x++ ) {
			iColor = XGE_COLOR_RGBA(0, 0, 0, 0);
			if ( (x >= 7 && x <= 24 && y >= 7 && y <= 24) || (x >= 13 && x <= 18 && y >= 3 && y <= 28) || (y >= 13 && y <= 18 && x >= 3 && x <= 28) ) {
				iColor = BlendColor(XGE_COLOR_RGBA(255, 255, 255, 255), XGE_COLOR_RGBA(210, 236, 255, 255), y, 31);
			}
			if ( (x == 7 || x == 24 || y == 7 || y == 24) && (x >= 7 && x <= 24 && y >= 7 && y <= 24) ) {
				iColor = XGE_COLOR_RGBA(40, 104, 178, 255);
			}
			WritePixel(arrPixels, 32, x, y, iColor);
		}
	}
	return xgeTextureCreateRGBA(pTexture, 32, 32, arrPixels);
}

static int CreateButtonTexture(xge_texture pTexture, int iWidth, int iHeight, uint32_t iTop, uint32_t iBottom, uint32_t iBorder, int bRounded, int bPattern)
{
	unsigned char arrPixels[96 * 40 * 4];
	int x;
	int y;
	int r;
	int dx;
	int dy;
	int iInside;
	uint32_t iColor;

	r = iHeight / 2;
	for ( y = 0; y < iHeight; y++ ) {
		for ( x = 0; x < iWidth; x++ ) {
			iInside = 1;
			if ( bRounded ) {
				if ( x < r && y < r ) {
					dx = r - x - 1;
					dy = r - y - 1;
					iInside = (dx * dx + dy * dy) <= r * r;
				} else if ( x >= iWidth - r && y < r ) {
					dx = x - (iWidth - r);
					dy = r - y - 1;
					iInside = (dx * dx + dy * dy) <= r * r;
				} else if ( x < r && y >= iHeight - r ) {
					dx = r - x - 1;
					dy = y - (iHeight - r);
					iInside = (dx * dx + dy * dy) <= r * r;
				} else if ( x >= iWidth - r && y >= iHeight - r ) {
					dx = x - (iWidth - r);
					dy = y - (iHeight - r);
					iInside = (dx * dx + dy * dy) <= r * r;
				}
			}
			if ( !iInside ) {
				WritePixel(arrPixels, iWidth, x, y, XGE_COLOR_RGBA(0, 0, 0, 0));
				continue;
			}
			iColor = BlendColor(iTop, iBottom, y, iHeight - 1);
			if ( bPattern && ((x / 6) % 2 == 0) && x > r && x < iWidth - r ) {
				iColor = BlendColor(iColor, XGE_COLOR_RGBA(255, 255, 255, 255), 1, 6);
			}
			if ( x == 0 || y == 0 || x == iWidth - 1 || y == iHeight - 1 ) {
				iColor = iBorder;
			}
			WritePixel(arrPixels, iWidth, x, y, iColor);
		}
	}
	return xgeTextureCreateRGBA(pTexture, iWidth, iHeight, arrPixels);
}

static int CreateTextures(app_state_t* pApp)
{
	if ( CreateIconTexture(&pApp->tTexture[0]) != XGE_OK ||
	     CreateButtonTexture(&pApp->tTexture[1], 96, 40, XGE_COLOR_RGBA(80, 166, 235, 255), XGE_COLOR_RGBA(28, 112, 202, 255), XGE_COLOR_RGBA(20, 88, 165, 255), 0, 0) != XGE_OK ||
	     CreateButtonTexture(&pApp->tTexture[2], 64, 32, XGE_COLOR_RGBA(116, 205, 255, 255), XGE_COLOR_RGBA(30, 137, 222, 255), XGE_COLOR_RGBA(24, 92, 170, 255), 1, 0) != XGE_OK ||
	     CreateButtonTexture(&pApp->tTexture[3], 64, 32, XGE_COLOR_RGBA(167, 226, 154, 255), XGE_COLOR_RGBA(56, 154, 90, 255), XGE_COLOR_RGBA(34, 108, 66, 255), 1, 1) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = { "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/Deng.ttf", "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_button_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ||
	     xgeXuiTokenSetTexture(&pApp->tXui, "icon", &pApp->tTexture[0]) != XGE_OK ||
	     xgeXuiTokenSetTexture(&pApp->tXui, "img", &pApp->tTexture[1]) != XGE_OK ||
	     xgeXuiTokenSetTexture(&pApp->tXui, "nine", &pApp->tTexture[2]) != XGE_OK ||
	     xgeXuiTokenSetTexture(&pApp->tXui, "tile", &pApp->tTexture[3]) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_button_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
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
	if ( fRootW < 920.0f ) {
		fRootW = 920.0f;
	}
	if ( fRootH < 520.0f ) {
		fRootH = 520.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	pApp->bCreateOK = (pApp->tPage.iButtonCount == 18) && (pApp->tPage.iLabelCount == 3);
	pApp->bLayoutOK = xgeXuiRoot(&pApp->tXui)->tRect.fW >= 920.0f;
	pApp->bStateOK = (pApp->tPage.arrButton[7].pIconTexture == &pApp->tTexture[0]) && (pApp->tPage.arrButton[11].bSelectable == 1) && (pApp->tPage.arrButton[11].bSelected == 1) && ((pApp->tPage.arrButton[13].pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) && (pApp->tPage.arrButton[14].arrHasPatch[0] == 1) && (pApp->tPage.arrButton[17].bBadgeVisible == 1);
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
		printf("xui_button_xson final-summary frames=%d create=%d layout=%d state=%d buttons=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->tPage.iButtonCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_BUTTON_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 940;
	tDesc.iHeight = 560;
	tDesc.sTitle = "XUI Button XSON";
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
