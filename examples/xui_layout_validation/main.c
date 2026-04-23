#include "../../xge.h"
#include <stdio.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	int bFontReady;
	int iFrameCount;
	int iLastW;
	int iLastH;
	xge_xui_widget pRootPanel;
	xge_xui_widget pHeader;
	xge_xui_widget pLeftColumn;
	xge_xui_widget pRightColumn;
	xge_xui_widget pContentPanel;
	xge_xui_widget pJustifyPanel;
	xge_xui_widget pResizePanel;
	xge_xui_widget pA;
	xge_xui_widget pB;
	xge_xui_widget pC;
	xge_xui_widget pD;
	xge_xui_label_t tHeaderLabel;
	xge_xui_label_t tContentLabel;
	xge_xui_label_t tResizeLabel;
} app_state_t;

static int AppLoadFont(app_state_t* pApp)
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
			printf("layout font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("layout font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_widget AppWidget(void)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(42, 52, 66, 255));
	}
	return pWidget;
}

static void AppBox(xge_xui_widget pWidget, float fW, float fH, uint32_t iColor)
{
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(fW), xgeXuiSizePx(fH));
	xgeXuiWidgetSetBackground(pWidget, iColor);
}

static int AppCreateTree(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;
	xge_xui_theme_t tTheme;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(238, 242, 248, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(30, 38, 50, 255);
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 8.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = AppWidget();
	pApp->pHeader = AppWidget();
	pApp->pLeftColumn = AppWidget();
	pApp->pRightColumn = AppWidget();
	pApp->pContentPanel = AppWidget();
	pApp->pJustifyPanel = AppWidget();
	pApp->pResizePanel = AppWidget();
	pApp->pA = AppWidget();
	pApp->pB = AppWidget();
	pApp->pC = AppWidget();
	pApp->pD = AppWidget();
	if ( (pRoot == NULL) || (pApp->pRootPanel == NULL) || (pApp->pHeader == NULL) || (pApp->pLeftColumn == NULL) || (pApp->pRightColumn == NULL) || (pApp->pContentPanel == NULL) || (pApp->pJustifyPanel == NULL) || (pApp->pResizePanel == NULL) || (pApp->pA == NULL) || (pApp->pB == NULL) || (pApp->pC == NULL) || (pApp->pD == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pApp->pRootPanel, 16.0f, 16.0f, 16.0f, 16.0f);
	xgeXuiWidgetSetGap(pApp->pRootPanel, 10.0f);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(18, 23, 31, 255));
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);

	xgeXuiLabelInit(&pApp->tHeaderLabel, pApp->pHeader, pFont, "XUI Layout Validation | resize window | ESC quit");
	xgeXuiLabelSetColor(&pApp->tHeaderLabel, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pHeader, 10.0f, 6.0f, 10.0f, 6.0f);
	xgeXuiWidgetSetBackground(pApp->pHeader, XGE_COLOR_RGBA(48, 72, 96, 255));
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHeader);

	xgeXuiWidgetSetLayout(pApp->pLeftColumn, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetGap(pApp->pLeftColumn, 8.0f);
	xgeXuiWidgetSetSize(pApp->pLeftColumn, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(96.0f));
	xgeXuiWidgetSetBackground(pApp->pLeftColumn, XGE_COLOR_RGBA(30, 42, 56, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pLeftColumn, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pLeftColumn);

	AppBox(pApp->pA, 90.0f, 40.0f, XGE_COLOR_RGBA(76, 156, 232, 255));
	AppBox(pApp->pB, 70.0f, 60.0f, XGE_COLOR_RGBA(238, 184, 74, 255));
	xgeXuiWidgetSetAlign(pApp->pA, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER);
	xgeXuiWidgetSetAlign(pApp->pB, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER);
	xgeXuiWidgetAdd(pApp->pLeftColumn, pApp->pA);
	xgeXuiWidgetAdd(pApp->pLeftColumn, pApp->pB);

	xgeXuiWidgetSetLayout(pApp->pRightColumn, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetGap(pApp->pRightColumn, 8.0f);
	xgeXuiWidgetSetSize(pApp->pRightColumn, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(112.0f));
	xgeXuiWidgetSetBackground(pApp->pRightColumn, XGE_COLOR_RGBA(28, 46, 42, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pRightColumn, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pRightColumn);

	xgeXuiWidgetSetLayout(pApp->pContentPanel, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetGap(pApp->pContentPanel, 6.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pContentPanel, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pContentPanel, XGE_COLOR_RGBA(54, 64, 82, 255));
	xgeXuiWidgetAdd(pApp->pRightColumn, pApp->pContentPanel);
	xgeXuiLabelInit(&pApp->tContentLabel, pApp->pContentPanel, pFont, "content-sized panel");

	xgeXuiWidgetSetLayout(pApp->pJustifyPanel, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetGap(pApp->pJustifyPanel, 12.0f);
	xgeXuiWidgetSetJustify(pApp->pJustifyPanel, XGE_XUI_JUSTIFY_SPACE_BETWEEN);
	xgeXuiWidgetSetSize(pApp->pJustifyPanel, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(72.0f));
	xgeXuiWidgetSetBackground(pApp->pJustifyPanel, XGE_COLOR_RGBA(44, 36, 62, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pJustifyPanel, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetAdd(pApp->pRightColumn, pApp->pJustifyPanel);
	AppBox(pApp->pC, 34.0f, 34.0f, XGE_COLOR_RGBA(92, 214, 140, 255));
	AppBox(pApp->pD, 34.0f, 34.0f, XGE_COLOR_RGBA(238, 100, 130, 255));
	xgeXuiWidgetAdd(pApp->pJustifyPanel, pApp->pC);
	xgeXuiWidgetAdd(pApp->pJustifyPanel, pApp->pD);

	xgeXuiWidgetSetLayout(pApp->pResizePanel, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pApp->pResizePanel, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetPaddingPx(pApp->pResizePanel, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pResizePanel, XGE_COLOR_RGBA(62, 48, 40, 255));
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pResizePanel);
	xgeXuiLabelInit(&pApp->tResizeLabel, pApp->pResizePanel, pFont, "resize tracking");
	return XGE_OK;
}

static void AppUpdateLayout(app_state_t* pApp)
{
	xge_rect_t tRect;
	xge_vec2_t tSize;
	char sText[160];
	int iW;
	int iH;

	iW = xgeGetWidth();
	iH = xgeGetHeight();
	if ( (iW != pApp->iLastW) || (iH != pApp->iLastH) ) {
		tRect.fX = 0.0f;
		tRect.fY = 0.0f;
		tRect.fW = (float)iW;
		tRect.fH = (float)iH;
		xgeXuiWidgetSetRect(pApp->pRootPanel, tRect);
		pApp->iLastW = iW;
		pApp->iLastH = iH;
	}
	tSize = xgeXuiWidgetGetDesiredSize(pApp->pContentPanel);
	snprintf(sText, sizeof(sText), "content desired %.0fx%.0f", tSize.fX, tSize.fY);
	xgeXuiLabelSetText(&pApp->tContentLabel, sText);
	tRect = xgeXuiWidgetGetRect(pApp->pResizePanel);
	snprintf(sText, sizeof(sText), "window %dx%d | resize panel %.0fx%.0f", iW, iH, tRect.fW, tRect.fH);
	xgeXuiLabelSetText(&pApp->tResizeLabel, sText);
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	pApp->iFrameCount++;
	if ( xgeKeyDown(256) ) {
		xgeQuit();
		return 1;
	}
	AppUpdateLayout(pApp);
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 28, 255));
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return 0;
}

int main(int argc, char** argv)
{
	app_state_t tApp;
	xge_desc_t tDesc;

	(void)argc;
	(void)argv;
	memset(&tApp, 0, sizeof(tApp));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 860;
	tDesc.iHeight = 560;
	tDesc.sTitle = "XGE XUI Layout Validation";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	(void)AppLoadFont(&tApp);
	if ( AppCreateTree(&tApp) != XGE_OK ) {
		xgeXuiUnit(&tApp.tXui);
		xgeUnit();
		return 3;
	}
	xgeRun(AppFrame, &tApp);
	{
		xge_vec2_t tContentSize = xgeXuiWidgetGetDesiredSize(tApp.pContentPanel);
		xge_rect_t tResizeRect = xgeXuiWidgetGetRect(tApp.pResizePanel);
		printf("xui layout summary: frames=%d size=%dx%d content=%.0fx%.0f resize=%.0f,%.0f,%.0fx%.0f font=%d\n", tApp.iFrameCount, tApp.iLastW, tApp.iLastH, tContentSize.fX, tContentSize.fY, tResizeRect.fX, tResizeRect.fY, tResizeRect.fW, tResizeRect.fH, tApp.bFontReady);
	}
	xgeXuiLabelUnit(&tApp.tHeaderLabel);
	xgeXuiLabelUnit(&tApp.tContentLabel);
	xgeXuiLabelUnit(&tApp.tResizeLabel);
	if ( tApp.bFontReady != 0 ) {
		xgeFontFree(&tApp.tFont);
	}
	xgeXuiUnit(&tApp.tXui);
	xgeUnit();
	return 0;
}
