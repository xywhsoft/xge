#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PANEL_COUNT 4
#define LABEL_COUNT 7
#define BUTTON_COUNT 2

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_texture_t tIcon;
	xge_xui_panel_t tPanel[PANEL_COUNT];
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_button_t tButton[BUTTON_COUNT];
	int iPanelCount;
	int iLabelCount;
	int iButtonCount;
	int bFontReady;
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

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = { "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/Deng.ttf", "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_panel font loaded: %s\n", arrFonts[i]);
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

static xge_xui_widget NewWidget(float fHeight)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), fHeight > 0.0f ? xgeXuiSizePx(fHeight) : xgeXuiSizeGrow(1.0f));
	}
	return pWidget;
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pParent, const char* sText)
{
	xge_xui_widget pWidget;
	xge_xui_label pLabel;

	if ( pApp->iLabelCount >= LABEL_COUNT ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = NewWidget(24.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pLabel = &pApp->tLabel[pApp->iLabelCount++];
	if ( xgeXuiLabelInit(pLabel, pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(pLabel, XGE_COLOR_RGBA(48, 58, 72, 255));
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
	pButton = &pApp->tButton[pApp->iButtonCount++];
	if ( xgeXuiButtonInit(pButton, &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(pButton, pApp->bFontReady ? &pApp->tFont : NULL, sText);
	xgeXuiButtonSetColors(pButton, XGE_COLOR_RGBA(56, 126, 204, 255), XGE_COLOR_RGBA(70, 146, 222, 255), XGE_COLOR_RGBA(36, 96, 170, 255), XGE_COLOR_RGBA(126, 166, 220, 255), XGE_COLOR_RGBA(196, 208, 224, 255));
	xgeXuiButtonSetTextColor(pButton, XGE_COLOR_RGBA(248, 252, 255, 255));
	return xgeXuiWidgetAdd(pParent, pWidget);
}

static xge_xui_panel AddPanel(app_state_t* pApp, xge_xui_widget pRoot, const char* sTitle)
{
	xge_xui_widget pWidget;
	xge_xui_panel pPanel;

	if ( pApp->iPanelCount >= PANEL_COUNT ) {
		return NULL;
	}
	pWidget = NewWidget(0.0f);
	if ( pWidget == NULL ) {
		return NULL;
	}
	pPanel = &pApp->tPanel[pApp->iPanelCount++];
	if ( xgeXuiPanelInit(pPanel, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return NULL;
	}
	xgeXuiPanelSetTitle(pPanel, pApp->bFontReady ? &pApp->tFont : NULL, sTitle);
	if ( xgeXuiWidgetAdd(pRoot, pWidget) != XGE_OK ) {
		xgeXuiPanelUnit(pPanel);
		xgeXuiWidgetFree(pWidget);
		return NULL;
	}
	return pPanel;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_panel pPanel;
	xge_rect_t tIconSrc;

	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	pRoot = xgeXuiRoot(&pApp->tXui);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_GRID);
	xgeXuiWidgetSetGrid(pRoot, 2, 0.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetPaddingPx(pRoot, 20.0f, 20.0f, 20.0f, 20.0f);
	xgeXuiWidgetSetBackground(pRoot, XGE_COLOR_RGBA(229, 235, 244, 255));

	pPanel = AddPanel(pApp, pRoot, "Default panel");
	if ( pPanel == NULL ) {
		return XGE_ERROR;
	}
	AddLabel(pApp, xgeXuiPanelGetClientWidget(pPanel), "Header and client are internal widgets.");
	AddLabel(pApp, xgeXuiPanelGetClientWidget(pPanel), "Children are added to the client area.");

	pPanel = AddPanel(pApp, pRoot, "Icon title and actions");
	if ( pPanel == NULL ) {
		return XGE_ERROR;
	}
	tIconSrc = (xge_rect_t){ 0.0f, 0.0f, 18.0f, 18.0f };
	xgeXuiPanelSetIcon(pPanel, &pApp->tIcon, tIconSrc);
	xgeXuiWidgetSetLayout(xgeXuiPanelGetClientWidget(pPanel), XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetGap(xgeXuiPanelGetClientWidget(pPanel), 8.0f);
	AddButton(pApp, xgeXuiPanelGetClientWidget(pPanel), "Apply");
	AddButton(pApp, xgeXuiPanelGetClientWidget(pPanel), "Reset");

	pPanel = AddPanel(pApp, pRoot, "Custom colors");
	if ( pPanel == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiPanelSetHeaderColor(pPanel, XGE_COLOR_RGBA(42, 112, 168, 255));
	xgeXuiPanelSetClientColor(pPanel, XGE_COLOR_RGBA(236, 248, 252, 255));
	xgeXuiPanelSetTitleColor(pPanel, XGE_COLOR_RGBA(248, 252, 255, 255));
	AddLabel(pApp, xgeXuiPanelGetClientWidget(pPanel), "Header, title, border, and client colors can be changed.");
	AddLabel(pApp, xgeXuiPanelGetClientWidget(pPanel), "The client keeps normal widget layout behavior.");

	pPanel = AddPanel(pApp, pRoot, "Disabled and clipped panel");
	if ( pPanel == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiPanelSetHeaderHeight(pPanel, 24.0f);
	xgeXuiPanelSetClip(pPanel, 1);
	AddLabel(pApp, xgeXuiPanelGetClientWidget(pPanel), "Disabled state is inherited from widget.");
	AddLabel(pApp, xgeXuiPanelGetClientWidget(pPanel), "Overflowing client content is clipped.");
	AddLabel(pApp, xgeXuiPanelGetClientWidget(pPanel), "This extra row stays inside the client box.");
	xgeXuiWidgetSetEnabled(pPanel->pWidget, 0);
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
	pApp->bCreateOK = (pApp->iPanelCount == PANEL_COUNT) && (pApp->iLabelCount == LABEL_COUNT) && (pApp->iButtonCount == BUTTON_COUNT);
	pApp->bLayoutOK = xgeXuiRoot(&pApp->tXui)->tRect.fW >= 900.0f;
	pApp->bStateOK = (xgeXuiPanelGetClientWidget(&pApp->tPanel[0]) != NULL) &&
		(pApp->tPanel[1].tIconImage.pTexture == &pApp->tIcon) &&
		(pApp->tPanel[2].iHeaderColor == XGE_COLOR_RGBA(42, 112, 168, 255)) &&
		((pApp->tPanel[3].pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0);
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
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < pApp->iButtonCount; i++ ) {
		xgeXuiButtonUnit(&pApp->tButton[i]);
	}
	for ( i = 0; i < pApp->iLabelCount; i++ ) {
		xgeXuiLabelUnit(&pApp->tLabel[i]);
	}
	for ( i = 0; i < pApp->iPanelCount; i++ ) {
		xgeXuiPanelUnit(&pApp->tPanel[i]);
	}
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
		printf("xui_panel final-summary frames=%d create=%d layout=%d state=%d panels=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->iPanelCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_PANEL_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 920;
	tDesc.iHeight = 580;
	tDesc.sTitle = "XUI Panel";
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
