#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_AREA_COUNT 4
#define IMAGE_PER_AREA 9
#define IMAGE_COUNT (IMAGE_AREA_COUNT * IMAGE_PER_AREA)

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_texture_t tTexture;
	xge_xui_widget pArea[IMAGE_AREA_COUNT];
	xge_xui_widget pImageWidget[IMAGE_COUNT];
	xge_xui_image_t tImage[IMAGE_COUNT];
	int bTextureReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
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

static void MakeImagePixels(unsigned char* pPixels, int iWidth, int iHeight)
{
	int x;
	int y;

	for ( y = 0; y < iHeight; y++ ) {
		for ( x = 0; x < iWidth; x++ ) {
			int i = (y * iWidth + x) * 4;
			int bLine = ((x % 8) == 0) || ((y % 8) == 0) || x == 0 || y == 0 || x == iWidth - 1 || y == iHeight - 1;
			pPixels[i + 0] = (unsigned char)(bLine ? 32 : 48 + x * 150 / iWidth);
			pPixels[i + 1] = (unsigned char)(bLine ? 92 : 112 + y * 90 / iHeight);
			pPixels[i + 2] = (unsigned char)(bLine ? 168 : 210);
			pPixels[i + 3] = 255;
		}
	}
}

static int CreateTexture(app_state_t* pApp)
{
	unsigned char arrPixels[96 * 56 * 4];

	MakeImagePixels(arrPixels, 96, 56);
	pApp->bTextureReady = (xgeTextureCreateRGBA(&pApp->tTexture, 96, 56, arrPixels) == XGE_OK);
	return pApp->bTextureReady ? XGE_OK : XGE_ERROR_RESOURCE_FAILED;
}

static int AlignX(int iSlot)
{
	static const int arrAlign[IMAGE_PER_AREA] = {
		XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER, XGE_XUI_ALIGN_END,
		XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER, XGE_XUI_ALIGN_END,
		XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER, XGE_XUI_ALIGN_END
	};

	return arrAlign[iSlot % IMAGE_PER_AREA];
}

static int AlignY(int iSlot)
{
	static const int arrAlign[IMAGE_PER_AREA] = {
		XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_START,
		XGE_XUI_ALIGN_CENTER, XGE_XUI_ALIGN_CENTER, XGE_XUI_ALIGN_CENTER,
		XGE_XUI_ALIGN_END, XGE_XUI_ALIGN_END, XGE_XUI_ALIGN_END
	};

	return arrAlign[iSlot % IMAGE_PER_AREA];
}

static xge_xui_widget NewWidget(void)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePercent(100.0f));
	}
	return pWidget;
}

static int AddImage(app_state_t* pApp, int iArea, int iSlot)
{
	int iIndex;
	xge_xui_widget pWidget;

	iIndex = iArea * IMAGE_PER_AREA + iSlot;
	pWidget = NewWidget();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetMarginPx(pWidget, 4.0f, 4.0f, 4.0f, 4.0f);
	xgeXuiWidgetSetPaddingPx(pWidget, 6.0f, 6.0f, 6.0f, 6.0f);
	if ( xgeXuiImageInit(&pApp->tImage[iIndex], pWidget, pApp->bTextureReady ? &pApp->tTexture : NULL) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiImageSetSourceRect(&pApp->tImage[iIndex], 8.0f, 8.0f, 72.0f, 48.0f);
	xgeXuiImageSetAlign(&pApp->tImage[iIndex], AlignX(iSlot), AlignY(iSlot));
	xgeXuiWidgetAdd(pApp->pArea[iArea], pWidget);
	pApp->pImageWidget[iIndex] = pWidget;
	return XGE_OK;
}

static void StyleArea(xge_xui_widget pWidget, uint32_t iBackground)
{
	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_GRID);
	xgeXuiWidgetSetGrid(pWidget, 3, 72.0f, 8.0f, 8.0f);
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePercent(100.0f));
	xgeXuiWidgetSetPaddingPx(pWidget, 10.0f, 10.0f, 10.0f, 10.0f);
	xgeXuiWidgetSetBackground(pWidget, iBackground);
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(170, 184, 202, 255));
	xgeXuiWidgetSetRadius(pWidget, 4.0f);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	int i;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, NULL);
	XgeXuiDemoApplyRootPanel(pRoot);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_GRID);
	xgeXuiWidgetSetGrid(pRoot, 2, 300.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetPaddingPx(pRoot, 18.0f, 18.0f, 18.0f, 18.0f);
	for ( i = 0; i < IMAGE_AREA_COUNT; i++ ) {
		pApp->pArea[i] = NewWidget();
		if ( pApp->pArea[i] == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		StyleArea(pApp->pArea[i], (i == 2) ? XGE_COLOR_RGBA(231, 238, 247, 255) : XGE_COLOR_RGBA(248, 250, 253, 255));
		xgeXuiWidgetAdd(pRoot, pApp->pArea[i]);
	}
	for ( i = 0; i < IMAGE_PER_AREA; i++ ) {
		if ( AddImage(pApp, 0, i) != XGE_OK || AddImage(pApp, 1, i) != XGE_OK || AddImage(pApp, 2, i) != XGE_OK ) {
			return XGE_ERROR;
		}
		xgeXuiWidgetSetBorder(pApp->pImageWidget[IMAGE_PER_AREA + i], 1.0f, XGE_COLOR_RGBA(74, 126, 184, 255));
		xgeXuiWidgetSetBackground(pApp->pImageWidget[(2 * IMAGE_PER_AREA) + i], XGE_COLOR_RGBA(235 + (i % 3) * 4, 240 + (i / 3) * 4, 247, 255));
	}
	for ( i = 0; i < IMAGE_PER_AREA; i++ ) {
		if ( AddImage(pApp, 3, i) != XGE_OK ) {
			return XGE_ERROR;
		}
		xgeXuiWidgetSetBorder(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + i], 1.0f, XGE_COLOR_RGBA(198, 208, 222, 255));
	}
	xgeXuiWidgetSetBackground(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + 0], XGE_COLOR_RGBA(232, 240, 252, 255));
	xgeXuiImageSetMode(&pApp->tImage[(3 * IMAGE_PER_AREA) + 0], XGE_XUI_IMAGE_STRETCH);
	xgeXuiWidgetSetBackground(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + 1], XGE_COLOR_RGBA(239, 247, 244, 255));
	xgeXuiImageSetMode(&pApp->tImage[(3 * IMAGE_PER_AREA) + 1], XGE_XUI_IMAGE_CONTAIN);
	xgeXuiWidgetSetBackground(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + 2], XGE_COLOR_RGBA(248, 240, 236, 255));
	xgeXuiImageSetMode(&pApp->tImage[(3 * IMAGE_PER_AREA) + 2], XGE_XUI_IMAGE_COVER);
	xgeXuiWidgetSetBackground(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + 3], XGE_COLOR_RGBA(246, 248, 251, 255));
	xgeXuiImageSetMode(&pApp->tImage[(3 * IMAGE_PER_AREA) + 3], XGE_XUI_IMAGE_SCALE_DOWN);
	xgeXuiWidgetSetBackground(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + 4], XGE_COLOR_RGBA(255, 248, 230, 255));
	xgeXuiWidgetSetBorder(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + 4], 2.0f, XGE_COLOR_RGBA(212, 150, 38, 255));
	xgeXuiImageSetCustomRect(&pApp->tImage[(3 * IMAGE_PER_AREA) + 4], 14.0f, 10.0f, 74.0f, 52.0f);
	xgeXuiWidgetSetBackground(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + 5], XGE_COLOR_RGBA(236, 247, 250, 255));
	xgeXuiImageClearSource(&pApp->tImage[(3 * IMAGE_PER_AREA) + 5]);
	xgeXuiWidgetSetBackground(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + 6], XGE_COLOR_RGBA(232, 238, 255, 255));
	xgeXuiWidgetSetBorder(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + 6], 1.0f, XGE_COLOR_RGBA(92, 116, 210, 255));
	xgeXuiImageSetTint(&pApp->tImage[(3 * IMAGE_PER_AREA) + 6], XGE_COLOR_RGBA(120, 190, 255, 220));
	xgeXuiWidgetSetBackground(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + 7], XGE_COLOR_RGBA(238, 240, 244, 255));
	xgeXuiWidgetSetEnabled(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + 7], 0);
	xgeXuiWidgetSetBackground(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + 8], XGE_COLOR_RGBA(250, 244, 246, 255));
	xgeXuiWidgetSetBorder(pApp->pImageWidget[(3 * IMAGE_PER_AREA) + 8], 1.0f, XGE_COLOR_RGBA(210, 160, 176, 255));
	xgeXuiImageSetTexture(&pApp->tImage[(3 * IMAGE_PER_AREA) + 8], NULL);
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	float fAreaH;
	float fCellH;
	int iWidth;
	int iHeight;
	int i;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 760.0f ) {
		fRootW = 760.0f;
	}
	if ( fRootH < 560.0f ) {
		fRootH = 560.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	fAreaH = (fRootH - 36.0f - 12.0f) * 0.5f;
	xgeXuiWidgetSetGrid(pRoot, 2, fAreaH, 12.0f, 12.0f);
	fCellH = (fAreaH - 20.0f - 16.0f) / 3.0f;
	for ( i = 0; i < IMAGE_AREA_COUNT; i++ ) {
		xgeXuiWidgetSetGrid(pApp->pArea[i], 3, fCellH, 8.0f, 8.0f);
	}
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	int i;

	pApp->bCreateOK = pApp->bTextureReady;
	for ( i = 0; i < IMAGE_COUNT; i++ ) {
		if ( (pApp->pImageWidget[i] == NULL) || (pApp->tImage[i].pWidget != pApp->pImageWidget[i]) ) {
			pApp->bCreateOK = 0;
		}
	}
	pApp->bLayoutOK = (pApp->pArea[0]->tRect.fW > 200.0f) && (pApp->pArea[3]->tRect.fH > 200.0f);
	pApp->bStateOK =
		(pApp->tImage[0].iMode == XGE_XUI_IMAGE_NATURAL) &&
		(pApp->pImageWidget[IMAGE_PER_AREA]->tStyle.fBorderWidth > 0.0f) &&
		(XGE_COLOR_GET_A(pApp->pImageWidget[2 * IMAGE_PER_AREA]->tStyle.iBackgroundColor) != 0) &&
		(pApp->tImage[(3 * IMAGE_PER_AREA) + 4].iMode == XGE_XUI_IMAGE_CUSTOM) &&
		(pApp->tImage[(3 * IMAGE_PER_AREA) + 8].pTexture == NULL);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( CreateTexture(pApp) != XGE_OK || xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
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
	for ( i = 0; i < IMAGE_COUNT; i++ ) {
		xgeXuiImageUnit(&pApp->tImage[i]);
	}
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bTextureReady ) {
		xgeTextureFree(&pApp->tTexture);
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
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_image final-summary frames=%d create=%d layout=%d state=%d images=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, IMAGE_COUNT);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_IMAGE_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 960;
	tDesc.iHeight = 680;
	tDesc.sTitle = "XUI Image";
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
