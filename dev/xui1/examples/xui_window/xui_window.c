#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINDOW_COUNT 4
#define LABEL_COUNT 18
#define BUTTON_COUNT 6

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_texture_t tIcon;
	xge_xui_window_t arrWindow[WINDOW_COUNT];
	xge_xui_widget arrWindowWidget[WINDOW_COUNT];
	xge_xui_label_t arrLabel[LABEL_COUNT];
	xge_xui_button_t arrButton[BUTTON_COUNT];
	int iLabelCount;
	int iButtonCount;
	int bFontReady;
	int bIconReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bLayerOK;
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

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = { "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/Deng.ttf", "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_window font loaded: %s\n", arrFonts[i]);
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

static xge_xui_widget NewWidget(float fHeight)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), (fHeight > 0.0f) ? xgeXuiSizePx(fHeight) : xgeXuiSizeGrow(1.0f));
	}
	return pWidget;
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pParent, const char* sText, float fHeight)
{
	xge_xui_widget pWidget;
	xge_xui_label pLabel;

	if ( pApp->iLabelCount >= LABEL_COUNT ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = NewWidget(fHeight);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pLabel = &pApp->arrLabel[pApp->iLabelCount++];
	if ( xgeXuiLabelInit(pLabel, pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(pLabel, XGE_COLOR_RGBA(54, 66, 82, 255));
	xgeXuiLabelSetAlign(pLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	return xgeXuiWidgetAdd(pParent, pWidget);
}

static int AddButton(app_state_t* pApp, xge_xui_widget pParent, const char* sText)
{
	xge_xui_widget pWidget;
	xge_xui_button pButton;

	if ( pApp->iButtonCount >= BUTTON_COUNT ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = NewWidget(32.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pButton = &pApp->arrButton[pApp->iButtonCount++];
	if ( xgeXuiButtonInit(pButton, &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(pButton, pApp->bFontReady ? &pApp->tFont : NULL, sText);
	xgeXuiButtonSetColors(pButton, XGE_COLOR_RGBA(56, 126, 204, 255), XGE_COLOR_RGBA(70, 146, 222, 255), XGE_COLOR_RGBA(36, 96, 170, 255), XGE_COLOR_RGBA(126, 166, 220, 255), XGE_COLOR_RGBA(196, 208, 224, 255));
	xgeXuiButtonSetTextColor(pButton, XGE_COLOR_RGBA(248, 252, 255, 255));
	return xgeXuiWidgetAdd(pParent, pWidget);
}

static int InitWindow(app_state_t* pApp, int iIndex, xge_rect_t tRect, const char* sTitle)
{
	xge_xui_widget pWidget;
	xge_xui_widget pClient;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pWidget, tRect);
	if ( xgeXuiWindowInit(&pApp->arrWindow[iIndex], &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	pApp->arrWindowWidget[iIndex] = pWidget;
	xgeXuiWindowSetTitle(&pApp->arrWindow[iIndex], pApp->bFontReady ? &pApp->tFont : NULL, sTitle);
	if ( pApp->bIconReady != 0 ) {
		xgeXuiWindowSetIcon(&pApp->arrWindow[iIndex], &pApp->tIcon, (xge_rect_t){ 0.0f, 0.0f, 16.0f, 16.0f });
	}
	pClient = xgeXuiWindowGetClientWidget(&pApp->arrWindow[iIndex]);
	xgeXuiWidgetSetLayout(pClient, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pClient, 12.0f, 10.0f, 12.0f, 10.0f);
	xgeXuiWidgetSetGap(pClient, 8.0f);
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_widget pClient;

	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	pRoot = xgeXuiRoot(&pApp->tXui);
	XgeXuiDemoApplyRootPanel(pRoot);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_ABSOLUTE);
	if ( AddLabel(pApp, pRoot, "Click windows to raise them. TopMost stays above normal windows.", 28.0f) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->arrLabel[0].pWidget->tLocalRect = (xge_rect_t){ 24.0f, 18.0f, 640.0f, 28.0f };
	xgeXuiWidgetSetRect(pApp->arrLabel[0].pWidget, pApp->arrLabel[0].pWidget->tLocalRect);

	if ( InitWindow(pApp, 0, (xge_rect_t){ 54.0f, 74.0f, 410.0f, 260.0f }, "Normal document window") != XGE_OK ||
		InitWindow(pApp, 1, (xge_rect_t){ 328.0f, 132.0f, 360.0f, 210.0f }, "TopMost inspector") != XGE_OK ||
		InitWindow(pApp, 2, (xge_rect_t){ 112.0f, 388.0f, 300.0f, 150.0f }, "Frameless floating tool") != XGE_OK ||
		InitWindow(pApp, 3, (xge_rect_t){ 520.0f, 410.0f, 300.0f, 130.0f }, "Collapsed window") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWindowSetTopMost(&pApp->arrWindow[1], 1);
	xgeXuiWindowSetShowTitleBar(&pApp->arrWindow[2], 0);
	xgeXuiWindowSetDragAnywhere(&pApp->arrWindow[2], 1);
	xgeXuiWindowSetShowCollapse(&pApp->arrWindow[2], 0);
	xgeXuiWindowSetShowMaximize(&pApp->arrWindow[2], 0);
	xgeXuiWindowSetShowClose(&pApp->arrWindow[2], 0);
	xgeXuiWindowSetResizable(&pApp->arrWindow[3], 0);
	xgeXuiWindowSetShowMaximize(&pApp->arrWindow[3], 0);
	xgeXuiWindowSetShowClose(&pApp->arrWindow[3], 0);
	xgeXuiWindowSetCollapsed(&pApp->arrWindow[3], 1);

	pClient = xgeXuiWindowGetClientWidget(&pApp->arrWindow[0]);
	if ( AddLabel(pApp, pClient, "Client area uses normal layout.", 24.0f) != XGE_OK ||
		AddLabel(pApp, pClient, "Title bar supports icon, buttons, move and resize.", 24.0f) != XGE_OK ||
		AddButton(pApp, pClient, "Child button does not start drag") != XGE_OK ) {
		return XGE_ERROR;
	}
	pClient = xgeXuiWindowGetClientWidget(&pApp->arrWindow[1]);
	if ( AddLabel(pApp, pClient, "This window is TopMost.", 24.0f) != XGE_OK ||
		AddLabel(pApp, pClient, "Normal windows cannot cover it.", 24.0f) != XGE_OK ||
		AddButton(pApp, pClient, "Pinned action") != XGE_OK ) {
		return XGE_ERROR;
	}
	pClient = xgeXuiWindowGetClientWidget(&pApp->arrWindow[2]);
	if ( AddLabel(pApp, pClient, "Frameless windows can drag anywhere.", 24.0f) != XGE_OK ||
		AddLabel(pApp, pClient, "Useful for floating tools and palettes.", 24.0f) != XGE_OK ) {
		return XGE_ERROR;
	}
	pClient = xgeXuiWindowGetClientWidget(&pApp->arrWindow[3]);
	if ( AddLabel(pApp, pClient, "Hidden while collapsed.", 24.0f) != XGE_OK ) {
		return XGE_ERROR;
	}
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
	pApp->bCreateOK = (pApp->arrWindow[0].pWidget != NULL) && (pApp->iLabelCount >= 8) && (pApp->iButtonCount >= 2);
	pApp->bLayerOK = (xgeXuiWidgetGetLayer(pApp->arrWindow[0].pWidget) == XGE_XUI_LAYER_FLOATING) &&
		(xgeXuiWidgetGetZ(pApp->arrWindow[1].pWidget) > xgeXuiWidgetGetZ(pApp->arrWindow[0].pWidget));
	pApp->bStateOK = (xgeXuiWindowIsTopMost(&pApp->arrWindow[1]) != 0) &&
		(xgeXuiWindowIsCollapsed(&pApp->arrWindow[3]) != 0) &&
		(xgeXuiWindowGetActive(&pApp->tXui) != NULL);
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
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < WINDOW_COUNT; i++ ) {
		if ( pApp->arrWindow[i].pWidget != NULL ) {
			xgeXuiWindowUnit(&pApp->arrWindow[i]);
		}
	}
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
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_window final-summary frames=%d create=%d layer=%d state=%d topMostZ=%d normalZ=%d active=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayerOK,
			pApp->bStateOK,
			xgeXuiWidgetGetZ(pApp->arrWindow[1].pWidget),
			xgeXuiWidgetGetZ(pApp->arrWindow[0].pWidget),
			xgeXuiWindowGetActive(&pApp->tXui) != NULL);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_WINDOW_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 940;
	tDesc.iHeight = 620;
	tDesc.sTitle = "XUI Window";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayerOK && tApp.bStateOK) ? 0 : 3;
}
