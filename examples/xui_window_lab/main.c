#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_texture_t tIconTexture;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pStatusTextWidget;
	xge_xui_widget pMainWindowWidget;
	xge_xui_widget pToolWindowWidget;
	xge_xui_widget pFixedWindowWidget;
	xge_xui_widget pActionWindowWidget;
	xge_xui_label_t arrLabels[40];
	xge_xui_button_t arrButtons[6];
	int iLabelCount;
	int iButtonCount;
	xge_xui_label_t* pStatusLabel;
	xge_xui_window_t tMainWindow;
	xge_xui_window_t tToolWindow;
	xge_xui_window_t tFixedWindow;
	xge_xui_window_t tActionWindow;
	int bFontReady;
	int bIconReady;
	int iFrameLimit;
	int iFrameCount;
	int iPingCount;
	int bInitOK;
	int bMainOK;
	int bToolOK;
	int bFixedOK;
	int bActionOK;
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

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simhei.ttf",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], 18.0f) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-window-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-window-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static int CreateIconTexture(app_state_t* pApp)
{
	uint32_t arrPixels[16 * 16];
	int x;
	int y;

	for ( y = 0; y < 16; y++ ) {
		for ( x = 0; x < 16; x++ ) {
			uint32_t iColor;

			iColor = XGE_COLOR_RGBA(38, 86, 158, 255);
			if ( x >= 2 && x <= 13 && y >= 2 && y <= 13 ) {
				iColor = XGE_COLOR_RGBA(72, 164, 255, 255);
			}
			if ( x >= 5 && x <= 10 && y >= 5 && y <= 10 ) {
				iColor = XGE_COLOR_RGBA(244, 248, 252, 255);
			}
			if ( (x == 3 || x == 12 || y == 3 || y == 12) && x >= 3 && x <= 12 && y >= 3 && y <= 12 ) {
				iColor = XGE_COLOR_RGBA(24, 48, 86, 255);
			}
			arrPixels[y * 16 + x] = iColor;
		}
	}
	if ( xgeTextureCreateRGBA(&pApp->tIconTexture, 16, 16, arrPixels) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->bIconReady = 1;
	return XGE_OK;
}

static xge_xui_label_t* AllocLabel(app_state_t* pApp)
{
	if ( pApp->iLabelCount >= (int)(sizeof(pApp->arrLabels) / sizeof(pApp->arrLabels[0])) ) {
		return NULL;
	}
	return &pApp->arrLabels[pApp->iLabelCount++];
}

static xge_xui_button_t* AllocButton(app_state_t* pApp)
{
	if ( pApp->iButtonCount >= (int)(sizeof(pApp->arrButtons) / sizeof(pApp->arrButtons[0])) ) {
		return NULL;
	}
	return &pApp->arrButtons[pApp->iButtonCount++];
}

static xge_xui_widget CreateLabelWidget(app_state_t* pApp, xge_xui_widget pParent, const char* sText, uint32_t iColor, uint32_t iAlign, float fHeight, int bGrow)
{
	xge_xui_widget pWidget;
	xge_xui_label_t* pLabel;

	pWidget = xgeXuiWidgetCreate();
	pLabel = AllocLabel(pApp);
	if ( (pWidget == NULL) || (pLabel == NULL) ) {
		return NULL;
	}
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizeGrow(1.0f), bGrow ? xgeXuiSizeGrow(1.0f) : xgeXuiSizePx(fHeight));
	if ( xgeXuiLabelInit(pLabel, pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return NULL;
	}
	xgeXuiLabelSetColor(pLabel, iColor);
	xgeXuiLabelSetAlign(pLabel, iAlign);
	(void)xgeXuiWidgetAdd(pParent, pWidget);
	return pWidget;
}

static xge_xui_widget CreateCard(app_state_t* pApp, xge_xui_widget pParent, const char* sTitle, uint32_t iBackground, float fWidth, int bGrow)
{
	xge_xui_widget pCard;

	pCard = xgeXuiWidgetCreate();
	if ( pCard == NULL ) {
		return NULL;
	}
	xgeXuiWidgetSetLayout(pCard, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetGap(pCard, 8.0f);
	xgeXuiWidgetSetPaddingPx(pCard, 10.0f, 10.0f, 10.0f, 10.0f);
	xgeXuiWidgetSetBackground(pCard, iBackground);
	xgeXuiWidgetSetRadius(pCard, 8.0f);
	xgeXuiWidgetSetSize(pCard, bGrow ? xgeXuiSizeGrow(1.0f) : xgeXuiSizePx(fWidth), xgeXuiSizeGrow(1.0f));
	(void)xgeXuiWidgetAdd(pParent, pCard);
	(void)CreateLabelWidget(pApp, pCard, sTitle, XGE_COLOR_RGBA(244, 248, 252, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 24.0f, 0);
	return pCard;
}

static void PingClick(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iPingCount++;
	}
}

static xge_xui_widget CreateButtonWidget(app_state_t* pApp, xge_xui_widget pParent, const char* sText, xge_xui_click_proc procClick, void* pUser, float fWidth)
{
	xge_xui_widget pWidget;
	xge_xui_button_t* pButton;

	pWidget = xgeXuiWidgetCreate();
	pButton = AllocButton(pApp);
	if ( (pWidget == NULL) || (pButton == NULL) ) {
		return NULL;
	}
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(fWidth), xgeXuiSizePx(30.0f));
	if ( xgeXuiButtonInit(pButton, &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return NULL;
	}
	xgeXuiButtonSetText(pButton, pApp->bFontReady ? &pApp->tFont : NULL, sText);
	xgeXuiButtonSetClick(pButton, procClick, pUser);
	xgeXuiButtonSetColors(
		pButton,
		XGE_COLOR_RGBA(62, 102, 170, 255),
		XGE_COLOR_RGBA(82, 126, 198, 255),
		XGE_COLOR_RGBA(48, 86, 146, 255),
		XGE_COLOR_RGBA(82, 126, 198, 255),
		XGE_COLOR_RGBA(66, 74, 88, 180));
	(void)xgeXuiWidgetAdd(pParent, pWidget);
	return pWidget;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[320];
	xge_rect_t tMainRect;
	xge_rect_t tToolRect;
	xge_rect_t tFixedRect;
	xge_rect_t tActionRect;

	tMainRect = xgeXuiWidgetGetRect(pApp->pMainWindowWidget);
	tToolRect = xgeXuiWidgetGetRect(pApp->pToolWindowWidget);
	tFixedRect = xgeXuiWidgetGetRect(pApp->pFixedWindowWidget);
	tActionRect = xgeXuiWidgetGetRect(pApp->pActionWindowWidget);
	snprintf(
		sText,
		sizeof(sText),
		"init=%d main=%d tool=%d fixed=%d action=%d ping=%d main(open=%d max=%d collapse=%d %.0fx%.0f) tool(open=%d %.0fx%.0f) fixed(open=%d %.0fx%.0f) action(open=%d collapse=%d %.0fx%.0f)",
		pApp->bInitOK,
		pApp->bMainOK,
		pApp->bToolOK,
		pApp->bFixedOK,
		pApp->bActionOK,
		pApp->iPingCount,
		xgeXuiWindowIsOpen(&pApp->tMainWindow),
		xgeXuiWindowIsMaximized(&pApp->tMainWindow),
		xgeXuiWindowIsCollapsed(&pApp->tMainWindow),
		tMainRect.fW,
		tMainRect.fH,
		xgeXuiWindowIsOpen(&pApp->tToolWindow),
		tToolRect.fW,
		tToolRect.fH,
		xgeXuiWindowIsOpen(&pApp->tFixedWindow),
		tFixedRect.fW,
		tFixedRect.fH,
		xgeXuiWindowIsOpen(&pApp->tActionWindow),
		xgeXuiWindowIsCollapsed(&pApp->tActionWindow),
		tActionRect.fW,
		tActionRect.fH);
	xgeXuiLabelSetText(pApp->pStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;
	xge_xui_widget pClient;
	xge_xui_widget pRow;
	xge_xui_widget pCard;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	tTheme.iTextColor = XGE_COLOR_RGBA(236, 242, 250, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(18, 24, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(28, 36, 48, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(80, 96, 124, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(92, 160, 255, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(46, 60, 80, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(64, 80, 104, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(78, 122, 192, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(78, 122, 192, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(72, 76, 88, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 8.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pRoot, XGE_COLOR_RGBA(18, 24, 34, 255));

	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pStatusTextWidget = xgeXuiWidgetCreate();
	pApp->pMainWindowWidget = xgeXuiWidgetCreate();
	pApp->pToolWindowWidget = xgeXuiWidgetCreate();
	pApp->pFixedWindowWidget = xgeXuiWidgetCreate();
	pApp->pActionWindowWidget = xgeXuiWidgetCreate();
	if ( (pApp->pStatusWidget == NULL) || (pApp->pStatusTextWidget == NULL) || (pApp->pMainWindowWidget == NULL) || (pApp->pToolWindowWidget == NULL) || (pApp->pFixedWindowWidget == NULL) || (pApp->pActionWindowWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 18.0f, 18.0f, 980.0f, 40.0f });
	xgeXuiWidgetSetRect(pApp->pStatusTextWidget, (xge_rect_t){ 10.0f, 8.0f, 940.0f, 24.0f });
	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 56, 74, 255));
	xgeXuiWidgetSetRadius(pApp->pStatusWidget, 8.0f);
	(void)xgeXuiWidgetAdd(pRoot, pApp->pStatusWidget);
	(void)xgeXuiWidgetAdd(pApp->pStatusWidget, pApp->pStatusTextWidget);

	pApp->pStatusLabel = AllocLabel(pApp);
	if ( pApp->pStatusLabel == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiLabelInit(pApp->pStatusLabel, pApp->pStatusTextWidget, pApp->bFontReady ? &pApp->tFont : NULL, "xui window lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(pApp->pStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiLabelSetAlign(pApp->pStatusLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);

	xgeXuiWidgetSetRect(pApp->pMainWindowWidget, (xge_rect_t){ 64.0f, 84.0f, 470.0f, 300.0f });
	xgeXuiWidgetSetRect(pApp->pToolWindowWidget, (xge_rect_t){ 576.0f, 126.0f, 260.0f, 176.0f });
	xgeXuiWidgetSetRect(pApp->pFixedWindowWidget, (xge_rect_t){ 610.0f, 350.0f, 232.0f, 158.0f });
	xgeXuiWidgetSetRect(pApp->pActionWindowWidget, (xge_rect_t){ 292.0f, 430.0f, 286.0f, 160.0f });
	(void)xgeXuiWidgetAdd(pRoot, pApp->pMainWindowWidget);
	(void)xgeXuiWidgetAdd(pRoot, pApp->pToolWindowWidget);
	(void)xgeXuiWidgetAdd(pRoot, pApp->pFixedWindowWidget);
	(void)xgeXuiWidgetAdd(pRoot, pApp->pActionWindowWidget);

	if ( xgeXuiWindowInit(&pApp->tMainWindow, &pApp->tXui, pApp->pMainWindowWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiWindowInit(&pApp->tToolWindow, &pApp->tXui, pApp->pToolWindowWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiWindowInit(&pApp->tFixedWindow, &pApp->tXui, pApp->pFixedWindowWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiWindowInit(&pApp->tActionWindow, &pApp->tXui, pApp->pActionWindowWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWindowSetTitle(&pApp->tMainWindow, pApp->bFontReady ? &pApp->tFont : NULL, "XUI Window");
	xgeXuiWindowSetChrome(&pApp->tMainWindow, 28.0f, 1.0f, 6.0f, 18.0f);
	xgeXuiWindowSetIcon(&pApp->tMainWindow, pApp->bIconReady ? &pApp->tIconTexture : NULL, (xge_rect_t){ 0.0f, 0.0f, 16.0f, 16.0f });
	xgeXuiWindowSetColors(
		&pApp->tMainWindow,
		XGE_COLOR_RGBA(28, 34, 44, 255),
		XGE_COLOR_RGBA(42, 96, 172, 255),
		XGE_COLOR_RGBA(246, 248, 252, 255),
		XGE_COLOR_RGBA(82, 102, 130, 255),
		XGE_COLOR_RGBA(255, 255, 255, 24),
		XGE_COLOR_RGBA(255, 255, 255, 52),
		XGE_COLOR_RGBA(0, 0, 0, 70));

	xgeXuiWindowSetTitle(&pApp->tToolWindow, pApp->bFontReady ? &pApp->tFont : NULL, "Floating Tool");
	xgeXuiWindowSetShowTitleBar(&pApp->tToolWindow, 0);
	xgeXuiWindowSetDragAnywhere(&pApp->tToolWindow, 1);
	xgeXuiWindowSetShowCollapse(&pApp->tToolWindow, 0);
	xgeXuiWindowSetShowMaximize(&pApp->tToolWindow, 0);
	xgeXuiWindowSetShowClose(&pApp->tToolWindow, 0);
	xgeXuiWindowSetColors(
		&pApp->tToolWindow,
		XGE_COLOR_RGBA(26, 32, 42, 255),
		XGE_COLOR_RGBA(26, 32, 42, 255),
		XGE_COLOR_RGBA(244, 248, 252, 255),
		XGE_COLOR_RGBA(86, 104, 132, 255),
		XGE_COLOR_RGBA(255, 255, 255, 20),
		XGE_COLOR_RGBA(255, 255, 255, 42),
		XGE_COLOR_RGBA(0, 0, 0, 56));

	xgeXuiWindowSetTitle(&pApp->tFixedWindow, pApp->bFontReady ? &pApp->tFont : NULL, "Fixed Window");
	xgeXuiWindowSetResizable(&pApp->tFixedWindow, 0);
	xgeXuiWindowSetShowCollapse(&pApp->tFixedWindow, 0);
	xgeXuiWindowSetShowMaximize(&pApp->tFixedWindow, 0);
	xgeXuiWindowSetShowClose(&pApp->tFixedWindow, 1);
	xgeXuiWindowSetColors(
		&pApp->tFixedWindow,
		XGE_COLOR_RGBA(30, 36, 46, 255),
		XGE_COLOR_RGBA(84, 92, 120, 255),
		XGE_COLOR_RGBA(246, 248, 252, 255),
		XGE_COLOR_RGBA(90, 104, 130, 255),
		XGE_COLOR_RGBA(255, 255, 255, 18),
		XGE_COLOR_RGBA(255, 255, 255, 42),
		XGE_COLOR_RGBA(0, 0, 0, 70));

	xgeXuiWindowSetTitle(&pApp->tActionWindow, pApp->bFontReady ? &pApp->tFont : NULL, "Action Window");
	xgeXuiWindowSetResizable(&pApp->tActionWindow, 0);
	xgeXuiWindowSetShowCollapse(&pApp->tActionWindow, 1);
	xgeXuiWindowSetShowMaximize(&pApp->tActionWindow, 0);
	xgeXuiWindowSetShowClose(&pApp->tActionWindow, 0);
	xgeXuiWindowSetColors(
		&pApp->tActionWindow,
		XGE_COLOR_RGBA(28, 34, 44, 255),
		XGE_COLOR_RGBA(62, 104, 150, 255),
		XGE_COLOR_RGBA(246, 248, 252, 255),
		XGE_COLOR_RGBA(82, 102, 130, 255),
		XGE_COLOR_RGBA(255, 255, 255, 20),
		XGE_COLOR_RGBA(255, 255, 255, 46),
		XGE_COLOR_RGBA(0, 0, 0, 72));

	pClient = xgeXuiWindowGetClientWidget(&pApp->tMainWindow);
	xgeXuiWidgetSetLayout(pClient, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetGap(pClient, 10.0f);
	xgeXuiWidgetSetPaddingPx(pClient, 12.0f, 12.0f, 12.0f, 12.0f);
	(void)CreateLabelWidget(pApp, pClient, "Title bar drag, collapse/maximize/close, and edge resize are all enabled.", XGE_COLOR_RGBA(208, 218, 232, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 24.0f, 0);
	pRow = xgeXuiWidgetCreate();
	if ( pRow == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetLayout(pRow, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetGap(pRow, 10.0f);
	xgeXuiWidgetSetSize(pRow, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	(void)xgeXuiWidgetAdd(pClient, pRow);
	pCard = CreateCard(pApp, pRow, "Navigator", XGE_COLOR_RGBA(44, 58, 80, 255), 124.0f, 0);
	(void)CreateLabelWidget(pApp, pCard, "Scene", XGE_COLOR_RGBA(236, 242, 250, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 24.0f, 0);
	(void)CreateLabelWidget(pApp, pCard, "Assets", XGE_COLOR_RGBA(236, 242, 250, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 24.0f, 0);
	(void)CreateLabelWidget(pApp, pCard, "Layers", XGE_COLOR_RGBA(236, 242, 250, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 24.0f, 0);
	pCard = CreateCard(pApp, pRow, "Client Area", XGE_COLOR_RGBA(36, 48, 68, 255), 0.0f, 1);
	(void)CreateLabelWidget(pApp, pCard, "Interactive child content should not start a window drag.", XGE_COLOR_RGBA(236, 242, 250, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 24.0f, 0);
	(void)CreateButtonWidget(pApp, pCard, "Ping Client", PingClick, pApp, 128.0f);
	(void)CreateLabelWidget(pApp, pCard, "Try dragging only from the title bar.", XGE_COLOR_RGBA(196, 208, 224, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 24.0f, 0);

	pClient = xgeXuiWindowGetClientWidget(&pApp->tToolWindow);
	xgeXuiWidgetSetLayout(pClient, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetGap(pClient, 8.0f);
	xgeXuiWidgetSetPaddingPx(pClient, 12.0f, 12.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetBackground(pClient, XGE_COLOR_RGBA(34, 42, 54, 255));
	xgeXuiWidgetSetRadius(pClient, 8.0f);
	(void)CreateLabelWidget(pApp, pClient, "Frameless Window", XGE_COLOR_RGBA(246, 248, 252, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 24.0f, 0);
	(void)CreateLabelWidget(pApp, pClient, "Drag anywhere on the background.", XGE_COLOR_RGBA(206, 216, 228, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 22.0f, 0);
	(void)CreateLabelWidget(pApp, pClient, "Resize from all four edges.", XGE_COLOR_RGBA(206, 216, 228, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 22.0f, 0);
	(void)CreateLabelWidget(pApp, pClient, "No title bar, no caption buttons.", XGE_COLOR_RGBA(172, 186, 206, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 22.0f, 0);

	pClient = xgeXuiWindowGetClientWidget(&pApp->tFixedWindow);
	xgeXuiWidgetSetLayout(pClient, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetGap(pClient, 8.0f);
	xgeXuiWidgetSetPaddingPx(pClient, 12.0f, 12.0f, 12.0f, 12.0f);
	(void)CreateLabelWidget(pApp, pClient, "Resize Disabled", XGE_COLOR_RGBA(246, 248, 252, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 24.0f, 0);
	(void)CreateLabelWidget(pApp, pClient, "Only close button is visible.", XGE_COLOR_RGBA(206, 216, 228, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 22.0f, 0);
	(void)CreateLabelWidget(pApp, pClient, "Dragging still works.", XGE_COLOR_RGBA(172, 186, 206, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 22.0f, 0);

	pClient = xgeXuiWindowGetClientWidget(&pApp->tActionWindow);
	xgeXuiWidgetSetLayout(pClient, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetGap(pClient, 8.0f);
	xgeXuiWidgetSetPaddingPx(pClient, 12.0f, 12.0f, 12.0f, 12.0f);
	(void)CreateLabelWidget(pApp, pClient, "Collapse Only", XGE_COLOR_RGBA(246, 248, 252, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 24.0f, 0);
	(void)CreateLabelWidget(pApp, pClient, "Resize disabled, maximize hidden.", XGE_COLOR_RGBA(206, 216, 228, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 22.0f, 0);
	(void)CreateLabelWidget(pApp, pClient, "Close button hidden too.", XGE_COLOR_RGBA(172, 186, 206, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP, 22.0f, 0);

	pApp->bInitOK = 1;
	pApp->bMainOK = (xgeXuiWindowGetClientWidget(&pApp->tMainWindow) != NULL);
	pApp->bToolOK = (xgeXuiWindowGetClientWidget(&pApp->tToolWindow) != NULL);
	pApp->bFixedOK = (xgeXuiWindowGetClientWidget(&pApp->tFixedWindow) != NULL);
	pApp->bActionOK = (xgeXuiWindowGetClientWidget(&pApp->tActionWindow) != NULL);
	UpdateStatus(pApp);
	return XGE_OK;
}

static int SceneEnter(xge_scene pScene)
{
	(void)pScene;
	return XGE_OK;
}

static int SceneLeave(xge_scene pScene)
{
	(void)pScene;
	return XGE_OK;
}

static int SceneEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( xgeXuiDispatchEvent(&pApp->tXui, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	return XGE_OK;
}

static int SceneUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	(void)xgeXuiUpdate(&pApp->tXui, fDelta);
	UpdateStatus(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		xgeQuit();
	}
	return XGE_OK;
}

static int SceneDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 24, 34, 255));
	(void)xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

static void AppUnit(app_state_t* pApp)
{
	if ( pApp->tToolWindow.pWidget != NULL ) {
		xgeXuiWindowUnit(&pApp->tToolWindow);
	}
	if ( pApp->tFixedWindow.pWidget != NULL ) {
		xgeXuiWindowUnit(&pApp->tFixedWindow);
	}
	if ( pApp->tActionWindow.pWidget != NULL ) {
		xgeXuiWindowUnit(&pApp->tActionWindow);
	}
	if ( pApp->tMainWindow.pWidget != NULL ) {
		xgeXuiWindowUnit(&pApp->tMainWindow);
	}
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bIconReady != 0 ) {
		xgeTextureFree(&pApp->tIconTexture);
	}
	if ( pApp->bFontReady != 0 ) {
		xgeFontFree(&pApp->tFont);
	}
}

int main(int argc, char** argv)
{
	app_state_t tApp;
	xge_desc_t tDesc;
	int i;

	memset(&tApp, 0, sizeof(tApp));
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			tApp.iFrameLimit = ArgInt(argv[++i], 0);
		}
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 1080;
	tDesc.iHeight = 720;
	tDesc.sTitle = "XGE XUI Window Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC | XGE_INIT_RESIZABLE;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( LoadFont(&tApp) != XGE_OK ) {
		xgeUnit();
		return 1;
	}
	if ( CreateIconTexture(&tApp) != XGE_OK ) {
		AppUnit(&tApp);
		xgeUnit();
		return 1;
	}
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		AppUnit(&tApp);
		xgeUnit();
		return 1;
	}
	if ( CreateUI(&tApp) != XGE_OK ) {
		AppUnit(&tApp);
		xgeUnit();
		return 1;
	}
	memset(&tApp.tScene, 0, sizeof(tApp.tScene));
	tApp.tScene.pUser = &tApp;
	tApp.tScene.onEnter = SceneEnter;
	tApp.tScene.onLeave = SceneLeave;
	tApp.tScene.onEvent = SceneEvent;
	tApp.tScene.onUpdate = SceneUpdate;
	tApp.tScene.onDraw = SceneDraw;
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		AppUnit(&tApp);
		xgeUnit();
		return 1;
	}
	(void)xgeRun(NULL, NULL);
	printf(
		"xui-window-lab final-summary frames=%d init=%d main=%d tool=%d fixed=%d action=%d ping=%d main(open=%d max=%d collapse=%d) tool(open=%d) fixed(open=%d) action(open=%d collapse=%d)\n",
		tApp.iFrameCount,
		tApp.bInitOK,
		tApp.bMainOK,
		tApp.bToolOK,
		tApp.bFixedOK,
		tApp.bActionOK,
		tApp.iPingCount,
		xgeXuiWindowIsOpen(&tApp.tMainWindow),
		xgeXuiWindowIsMaximized(&tApp.tMainWindow),
		xgeXuiWindowIsCollapsed(&tApp.tMainWindow),
		xgeXuiWindowIsOpen(&tApp.tToolWindow),
		xgeXuiWindowIsOpen(&tApp.tFixedWindow),
		xgeXuiWindowIsOpen(&tApp.tActionWindow),
		xgeXuiWindowIsCollapsed(&tApp.tActionWindow));
	xgeSceneSet(NULL);
	AppUnit(&tApp);
	xgeUnit();
	return 0;
}
