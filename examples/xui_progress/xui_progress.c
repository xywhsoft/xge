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
	xge_font_t tFont;
	xge_texture_t tTexture[TEXTURE_COUNT];
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_progress_t tProgress[PROGRESS_COUNT];
	int iLabelCount;
	int iProgressCount;
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

	if ( (iWidth > 64) || (iHeight > 24) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
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
		printf("xui_progress texture byte check failed: %u,%u,%u,%u\n", (unsigned)arrReadback[((2 * 32) + 2) * 4 + 0], (unsigned)arrReadback[((2 * 32) + 2) * 4 + 1], (unsigned)arrReadback[((2 * 32) + 2) * 4 + 2], (unsigned)arrReadback[((2 * 32) + 2) * 4 + 3]);
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
			printf("xui_progress font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_widget NewWidget(void)
{
	return xgeXuiWidgetCreate();
}

static xge_xui_widget AddLabel(app_state_t* pApp, xge_xui_widget pParent, const char* sText)
{
	xge_xui_widget pWidget;
	int iIndex;

	if ( pApp->iLabelCount >= LABEL_COUNT ) {
		return NULL;
	}
	pWidget = NewWidget();
	if ( pWidget == NULL ) {
		return NULL;
	}
	iIndex = pApp->iLabelCount++;
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(20.0f));
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 0.0f, 2.0f, 0.0f);
	if ( xgeXuiLabelInit(&pApp->tLabel[iIndex], pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return NULL;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], XGE_COLOR_RGBA(42, 52, 68, 255));
	xgeXuiLabelSetAlign(&pApp->tLabel[iIndex], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	xgeXuiWidgetAdd(pParent, pWidget);
	return pWidget;
}

static xge_xui_widget AddProgress(app_state_t* pApp, xge_xui_widget pParent, const char* sTemplate, uint32_t iTrack, uint32_t iFill, uint32_t iBg, uint32_t iBorder, float fBorderWidth)
{
	xge_xui_widget pWidget;
	int iIndex;

	if ( pApp->iProgressCount >= PROGRESS_COUNT ) {
		return NULL;
	}
	pWidget = NewWidget();
	if ( pWidget == NULL ) {
		return NULL;
	}
	iIndex = pApp->iProgressCount++;
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(24.0f));
	xgeXuiWidgetSetPaddingPx(pWidget, 3.0f, 3.0f, 3.0f, 3.0f);
	xgeXuiWidgetSetBackground(pWidget, iBg);
	xgeXuiWidgetSetBorder(pWidget, fBorderWidth, iBorder);
	if ( xgeXuiProgressInit(&pApp->tProgress[iIndex], pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return NULL;
	}
	xgeXuiProgressSetRange(&pApp->tProgress[iIndex], 0.0f, 100.0f);
	xgeXuiProgressSetValue(&pApp->tProgress[iIndex], pApp->fProgress);
	xgeXuiProgressSetColors(&pApp->tProgress[iIndex], iTrack, iFill);
	xgeXuiProgressSetText(&pApp->tProgress[iIndex], pApp->bFontReady ? &pApp->tFont : NULL, sTemplate);
	xgeXuiProgressSetTextColor(&pApp->tProgress[iIndex], XGE_COLOR_RGBA(36, 52, 70, 255));
	xgeXuiProgressSetFillTextColor(&pApp->tProgress[iIndex], XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiWidgetAdd(pParent, pWidget);
	return pWidget;
}

static xge_xui_widget AddImageProgress(app_state_t* pApp, xge_xui_widget pParent, const char* sTemplate, int iFillPatchMode)
{
	xge_nine_patch_t tTrackPatch;
	xge_nine_patch_t tFillPatch;
	xge_xui_widget pWidget;

	pWidget = AddProgress(pApp, pParent, sTemplate, XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(0, 0, 0, 0), 0.0f);
	if ( pWidget == NULL ) {
		return NULL;
	}
	xgeNinePatchInitSimple(&tTrackPatch, &pApp->tTexture[0], (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f });
	xgeNinePatchInitSimple(&tFillPatch, &pApp->tTexture[1], (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f });
	xgeNinePatchSetMode(&tTrackPatch, XGE_NINE_PATCH_STRETCH);
	xgeNinePatchSetMode(&tFillPatch, XGE_NINE_PATCH_STRETCH);
	xgeXuiProgressSetTrackPatch(&pApp->tProgress[pApp->iProgressCount - 1], &tTrackPatch);
	xgeXuiProgressSetFillPatch(&pApp->tProgress[pApp->iProgressCount - 1], &tFillPatch);
	xgeXuiProgressSetFillPatchMode(&pApp->tProgress[pApp->iProgressCount - 1], iFillPatchMode);
	return pWidget;
}

static xge_xui_widget AddPatchProgress(app_state_t* pApp, xge_xui_widget pParent, const char* sTemplate, xge_texture pTrackTexture, xge_texture pFillTexture, int iMode)
{
	xge_nine_patch_t tTrackPatch;
	xge_nine_patch_t tFillPatch;
	xge_xui_widget pWidget;

	pWidget = AddProgress(pApp, pParent, sTemplate, XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(0, 0, 0, 0), 0.0f);
	if ( pWidget == NULL ) {
		return NULL;
	}
	xgeNinePatchInit(&tTrackPatch, pTrackTexture, (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f }, 0.375f, 0.25f, 0.625f, 0.75f);
	xgeNinePatchInit(&tFillPatch, pFillTexture, (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f }, 0.375f, 0.25f, 0.625f, 0.75f);
	xgeNinePatchSetMode(&tTrackPatch, iMode);
	xgeNinePatchSetMode(&tFillPatch, iMode);
	xgeXuiProgressSetTrackPatch(&pApp->tProgress[pApp->iProgressCount - 1], &tTrackPatch);
	xgeXuiProgressSetFillPatch(&pApp->tProgress[pApp->iProgressCount - 1], &tFillPatch);
	return pWidget;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	uint32_t iTrack;
	uint32_t iFill;
	uint32_t iFrameBg;
	uint32_t iFrameBorder;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetGap(pRoot, 5.0f);
	xgeXuiWidgetSetPaddingPx(pRoot, 28.0f, 18.0f, 28.0f, 18.0f);
	xgeXuiWidgetSetBackground(pRoot, XGE_COLOR_RGBA(229, 235, 244, 255));
	iTrack = XGE_COLOR_RGBA(215, 232, 244, 255);
	iFill = XGE_COLOR_RGBA(46, 124, 214, 255);
	iFrameBg = XGE_COLOR_RGBA(247, 250, 253, 255);
	iFrameBorder = XGE_COLOR_RGBA(154, 176, 202, 255);
	if ( AddLabel(pApp, pRoot, "空白进度条") == NULL ||
		AddProgress(pApp, pRoot, NULL, iTrack, iFill, iFrameBg, iFrameBorder, 1.0f) == NULL ||
		AddLabel(pApp, pRoot, "显示百分比") == NULL ||
		AddProgress(pApp, pRoot, "%1.0f%%", iTrack, iFill, iFrameBg, iFrameBorder, 1.0f) == NULL ||
		AddLabel(pApp, pRoot, "固定字符串") == NULL ||
		AddProgress(pApp, pRoot, "Loading", iTrack, iFill, iFrameBg, iFrameBorder, 1.0f) == NULL ||
		AddLabel(pApp, pRoot, "自定义内容") == NULL ||
		AddProgress(pApp, pRoot, "进度：%1.2f", iTrack, iFill, iFrameBg, iFrameBorder, 1.0f) == NULL ||
		AddLabel(pApp, pRoot, "自定义配色") == NULL ||
		AddProgress(pApp, pRoot, "%1.0f%%", XGE_COLOR_RGBA(248, 226, 188, 255), XGE_COLOR_RGBA(211, 118, 36, 255), XGE_COLOR_RGBA(255, 249, 238, 255), XGE_COLOR_RGBA(211, 118, 36, 255), 2.0f) == NULL ||
		AddLabel(pApp, pRoot, "不显示边框") == NULL ||
		AddProgress(pApp, pRoot, "%1.0f%%", XGE_COLOR_RGBA(211, 227, 239, 255), XGE_COLOR_RGBA(52, 151, 111, 255), XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(0, 0, 0, 0), 0.0f) == NULL ||
		AddLabel(pApp, pRoot, "图片拉伸进度条") == NULL ||
		AddImageProgress(pApp, pRoot, "%1.0f%%", XGE_XUI_PROGRESS_FILL_STRETCH) == NULL ||
		AddLabel(pApp, pRoot, "图片裁剪进度条") == NULL ||
		AddImageProgress(pApp, pRoot, "%1.0f%%", XGE_XUI_PROGRESS_FILL_REVEAL) == NULL ||
		AddLabel(pApp, pRoot, "九宫格拉伸") == NULL ||
		AddPatchProgress(pApp, pRoot, "%1.0f%%", &pApp->tTexture[2], &pApp->tTexture[3], XGE_NINE_PATCH_STRETCH) == NULL ||
		AddLabel(pApp, pRoot, "九宫格平铺") == NULL ||
		AddPatchProgress(pApp, pRoot, "%1.0f%%", &pApp->tTexture[4], &pApp->tTexture[5], XGE_NINE_PATCH_TILE) == NULL ) {
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
	for ( i = 0; i < pApp->iProgressCount; i++ ) {
		xgeXuiProgressSetValue(&pApp->tProgress[i], pApp->fProgress);
	}
}

static void RunChecks(app_state_t* pApp)
{
	pApp->bCreateOK = (pApp->iLabelCount == LABEL_COUNT) && (pApp->iProgressCount == PROGRESS_COUNT);
	pApp->bLayoutOK = xgeXuiRoot(&pApp->tXui)->tRect.fW >= 720.0f;
	pApp->bStateOK = (pApp->tProgress[0].sTextTemplate == NULL) && (pApp->tProgress[1].bTemplateString == 1) && (pApp->tProgress[2].bTemplateString == 0) && (pApp->tProgress[3].bTemplateString == 1) && (pApp->tProgress[6].bHasFillPatch == 1) && (pApp->tProgress[7].iFillPatchMode == XGE_XUI_PROGRESS_FILL_REVEAL) && (pApp->tProgress[9].tFillPatch.iMode == XGE_NINE_PATCH_TILE);
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
	for ( i = 0; i < pApp->iProgressCount; i++ ) {
		xgeXuiProgressUnit(&pApp->tProgress[i]);
	}
	for ( i = 0; i < pApp->iLabelCount; i++ ) {
		xgeXuiLabelUnit(&pApp->tLabel[i]);
	}
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
		printf("xui_progress final-summary frames=%d create=%d layout=%d state=%d labels=%d progress=%d value=%.2f\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->iLabelCount, pApp->iProgressCount, pApp->fProgress);
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
	app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_PROGRESS_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 640;
	tDesc.sTitle = "XUI Progress";
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
