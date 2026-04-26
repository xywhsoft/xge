#include "../../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pHeaderWidget;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pGalleryWidget;
	xge_xui_widget pLeftColumn;
	xge_xui_widget pRightColumn;
	xge_xui_widget pAbsolutePanel;
	xge_xui_widget pAbsoluteA;
	xge_xui_widget pAbsoluteB;
	xge_xui_widget pRowPanel;
	xge_xui_widget pRowA;
	xge_xui_widget pRowB;
	xge_xui_widget pColumnPanel;
	xge_xui_widget pColumnA;
	xge_xui_widget pColumnB;
	xge_xui_widget pStackPanel;
	xge_xui_widget pStackBack;
	xge_xui_widget pStackFront;
	xge_xui_widget pJustifyPanel;
	xge_xui_widget pJustifyA;
	xge_xui_widget pJustifyB;
	xge_xui_widget pJustifyC;
	xge_xui_widget pGridPanel;
	xge_xui_widget pGridA;
	xge_xui_widget pGridB;
	xge_xui_widget pGridC;
	xge_xui_widget pGridD;
	xge_xui_widget pMetricsPanel;
	xge_xui_widget pContentWidget;
	xge_xui_widget pGrowWidget;
	xge_xui_widget pPercentWidget;
	xge_xui_widget pPxWidget;
	xge_xui_label_t tHeaderLabel;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tContentLabel;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bAbsoluteOK;
	int bRowOK;
	int bColumnOK;
	int bStackOK;
	int bGridOK;
	int bSizeOK;
	int bGapJustifyAlignOK;
	int bDirtyLayoutOK;
	float fColumnGapBefore;
	float fColumnGapAfter;
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

static int FloatNear(float fA, float fB, float fEpsilon)
{
	return fabsf(fA - fB) <= fEpsilon;
}

static xge_xui_widget MakeWidget(uint32_t iColor)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetBackground(pWidget, iColor);
		xgeXuiWidgetSetRadius(pWidget, 6.0f);
	}
	return pWidget;
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
			printf("xui-layout-gallery font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-layout-gallery font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRect;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	tRect.fX = 20.0f;
	tRect.fY = 20.0f;
	tRect.fW = (float)iWidth - 40.0f;
	tRect.fH = (float)iHeight - 40.0f;
	if ( tRect.fW < 300.0f ) {
		tRect.fW = 300.0f;
	}
	if ( tRect.fH < 220.0f ) {
		tRect.fH = 220.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRect);
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static int PaintFrame(app_state_t* pApp)
{
	int iPaintCommands;

	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	iPaintCommands = xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return iPaintCommands;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"absolute=%d row=%d column=%d stack=%d grid=%d size=%d gja=%d dirty=%d gap=%.0f->%.0f",
		pApp->bAbsoluteOK,
		pApp->bRowOK,
		pApp->bColumnOK,
		pApp->bStackOK,
		pApp->bGridOK,
		pApp->bSizeOK,
		pApp->bGapJustifyAlignOK,
		pApp->bDirtyLayoutOK,
		pApp->fColumnGapBefore,
		pApp->fColumnGapAfter);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_theme_t tTheme;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(238, 242, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(24, 30, 42, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(32, 40, 56, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(88, 156, 255, 255);
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 8.0f;
	tTheme.fRadius = 6.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = MakeWidget(XGE_COLOR_RGBA(24, 30, 42, 255));
	pApp->pHeaderWidget = MakeWidget(XGE_COLOR_RGBA(46, 62, 84, 255));
	pApp->pStatusWidget = MakeWidget(XGE_COLOR_RGBA(38, 48, 64, 255));
	pApp->pGalleryWidget = MakeWidget(XGE_COLOR_RGBA(18, 24, 34, 255));
	pApp->pLeftColumn = MakeWidget(XGE_COLOR_RGBA(26, 32, 44, 255));
	pApp->pRightColumn = MakeWidget(XGE_COLOR_RGBA(26, 32, 44, 255));
	pApp->pAbsolutePanel = MakeWidget(XGE_COLOR_RGBA(40, 58, 84, 255));
	pApp->pAbsoluteA = MakeWidget(XGE_COLOR_RGBA(92, 164, 255, 255));
	pApp->pAbsoluteB = MakeWidget(XGE_COLOR_RGBA(246, 186, 92, 255));
	pApp->pRowPanel = MakeWidget(XGE_COLOR_RGBA(34, 56, 48, 255));
	pApp->pRowA = MakeWidget(XGE_COLOR_RGBA(86, 212, 150, 255));
	pApp->pRowB = MakeWidget(XGE_COLOR_RGBA(236, 110, 132, 255));
	pApp->pColumnPanel = MakeWidget(XGE_COLOR_RGBA(56, 42, 72, 255));
	pApp->pColumnA = MakeWidget(XGE_COLOR_RGBA(122, 174, 255, 255));
	pApp->pColumnB = MakeWidget(XGE_COLOR_RGBA(255, 132, 92, 255));
	pApp->pStackPanel = MakeWidget(XGE_COLOR_RGBA(60, 48, 38, 255));
	pApp->pStackBack = MakeWidget(XGE_COLOR_RGBA(70, 110, 160, 255));
	pApp->pStackFront = MakeWidget(XGE_COLOR_RGBA(248, 214, 126, 245));
	pApp->pJustifyPanel = MakeWidget(XGE_COLOR_RGBA(34, 44, 62, 255));
	pApp->pJustifyA = MakeWidget(XGE_COLOR_RGBA(88, 156, 255, 255));
	pApp->pJustifyB = MakeWidget(XGE_COLOR_RGBA(116, 214, 150, 255));
	pApp->pJustifyC = MakeWidget(XGE_COLOR_RGBA(244, 138, 164, 255));
	pApp->pGridPanel = MakeWidget(XGE_COLOR_RGBA(46, 54, 38, 255));
	pApp->pGridA = MakeWidget(XGE_COLOR_RGBA(90, 180, 255, 255));
	pApp->pGridB = MakeWidget(XGE_COLOR_RGBA(255, 176, 92, 255));
	pApp->pGridC = MakeWidget(XGE_COLOR_RGBA(116, 214, 150, 255));
	pApp->pGridD = MakeWidget(XGE_COLOR_RGBA(244, 138, 164, 255));
	pApp->pMetricsPanel = MakeWidget(XGE_COLOR_RGBA(40, 44, 56, 255));
	pApp->pContentWidget = MakeWidget(XGE_COLOR_RGBA(60, 74, 96, 255));
	pApp->pGrowWidget = MakeWidget(XGE_COLOR_RGBA(86, 210, 140, 255));
	pApp->pPercentWidget = MakeWidget(XGE_COLOR_RGBA(244, 184, 98, 255));
	pApp->pPxWidget = MakeWidget(XGE_COLOR_RGBA(232, 116, 148, 255));

	if ( (pApp->pRootPanel == NULL) || (pApp->pHeaderWidget == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pGalleryWidget == NULL) || (pApp->pLeftColumn == NULL) || (pApp->pRightColumn == NULL) || (pApp->pAbsolutePanel == NULL) || (pApp->pAbsoluteA == NULL) || (pApp->pAbsoluteB == NULL) || (pApp->pRowPanel == NULL) || (pApp->pRowA == NULL) || (pApp->pRowB == NULL) || (pApp->pColumnPanel == NULL) || (pApp->pColumnA == NULL) || (pApp->pColumnB == NULL) || (pApp->pStackPanel == NULL) || (pApp->pStackBack == NULL) || (pApp->pStackFront == NULL) || (pApp->pJustifyPanel == NULL) || (pApp->pJustifyA == NULL) || (pApp->pJustifyB == NULL) || (pApp->pJustifyC == NULL) || (pApp->pGridPanel == NULL) || (pApp->pGridA == NULL) || (pApp->pGridB == NULL) || (pApp->pGridC == NULL) || (pApp->pGridD == NULL) || (pApp->pMetricsPanel == NULL) || (pApp->pContentWidget == NULL) || (pApp->pGrowWidget == NULL) || (pApp->pPercentWidget == NULL) || (pApp->pPxWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pApp->pRootPanel, 16.0f, 16.0f, 16.0f, 16.0f);
	xgeXuiWidgetSetGap(pApp->pRootPanel, 10.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);

	xgeXuiLabelInit(&pApp->tHeaderLabel, pApp->pHeaderWidget, pFont, "XUI Layout Gallery | absolute row column stack grid | auto smoke");
	xgeXuiLabelSetColor(&pApp->tHeaderLabel, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pHeaderWidget, 10.0f, 6.0f, 10.0f, 6.0f);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHeaderWidget);

	xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "layout checks pending");
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(228, 236, 248, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 6.0f, 10.0f, 6.0f);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);

	xgeXuiWidgetSetLayout(pApp->pGalleryWidget, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetSize(pApp->pGalleryWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetGap(pApp->pGalleryWidget, 12.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pGalleryWidget, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pGalleryWidget);

	xgeXuiWidgetSetLayout(pApp->pLeftColumn, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pApp->pLeftColumn, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetGap(pApp->pLeftColumn, 10.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pLeftColumn, 6.0f, 6.0f, 6.0f, 6.0f);
	xgeXuiWidgetAdd(pApp->pGalleryWidget, pApp->pLeftColumn);

	xgeXuiWidgetSetLayout(pApp->pRightColumn, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pApp->pRightColumn, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetGap(pApp->pRightColumn, 10.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pRightColumn, 6.0f, 6.0f, 6.0f, 6.0f);
	xgeXuiWidgetAdd(pApp->pGalleryWidget, pApp->pRightColumn);

	xgeXuiWidgetSetLayout(pApp->pAbsolutePanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetSize(pApp->pAbsolutePanel, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(120.0f));
	xgeXuiWidgetSetPaddingPx(pApp->pAbsolutePanel, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetAdd(pApp->pLeftColumn, pApp->pAbsolutePanel);
	xgeXuiWidgetSetRect(pApp->pAbsoluteA, (xge_rect_t){ 14.0f, 16.0f, 72.0f, 28.0f });
	xgeXuiWidgetSetRect(pApp->pAbsoluteB, (xge_rect_t){ 110.0f, 54.0f, 88.0f, 34.0f });
	xgeXuiWidgetAdd(pApp->pAbsolutePanel, pApp->pAbsoluteA);
	xgeXuiWidgetAdd(pApp->pAbsolutePanel, pApp->pAbsoluteB);

	xgeXuiWidgetSetLayout(pApp->pRowPanel, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetSize(pApp->pRowPanel, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(84.0f));
	xgeXuiWidgetSetGap(pApp->pRowPanel, 10.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pRowPanel, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetAdd(pApp->pLeftColumn, pApp->pRowPanel);
	xgeXuiWidgetSetSize(pApp->pRowA, xgeXuiSizePx(72.0f), xgeXuiSizePx(24.0f));
	xgeXuiWidgetSetAlign(pApp->pRowA, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER);
	xgeXuiWidgetSetSize(pApp->pRowB, xgeXuiSizePx(84.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetAlign(pApp->pRowB, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_END);
	xgeXuiWidgetAdd(pApp->pRowPanel, pApp->pRowA);
	xgeXuiWidgetAdd(pApp->pRowPanel, pApp->pRowB);

	xgeXuiWidgetSetLayout(pApp->pColumnPanel, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pApp->pColumnPanel, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(118.0f));
	xgeXuiWidgetSetGap(pApp->pColumnPanel, 6.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pColumnPanel, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetAdd(pApp->pLeftColumn, pApp->pColumnPanel);
	xgeXuiWidgetSetSize(pApp->pColumnA, xgeXuiSizePercent(60.0f), xgeXuiSizePx(24.0f));
	xgeXuiWidgetSetAlign(pApp->pColumnA, XGE_XUI_ALIGN_END, XGE_XUI_ALIGN_START);
	xgeXuiWidgetSetSize(pApp->pColumnB, xgeXuiSizePx(96.0f), xgeXuiSizePx(30.0f));
	xgeXuiWidgetSetAlign(pApp->pColumnB, XGE_XUI_ALIGN_CENTER, XGE_XUI_ALIGN_START);
	xgeXuiWidgetAdd(pApp->pColumnPanel, pApp->pColumnA);
	xgeXuiWidgetAdd(pApp->pColumnPanel, pApp->pColumnB);

	xgeXuiWidgetSetLayout(pApp->pStackPanel, XGE_XUI_LAYOUT_STACK);
	xgeXuiWidgetSetSize(pApp->pStackPanel, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(92.0f));
	xgeXuiWidgetSetPaddingPx(pApp->pStackPanel, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetAdd(pApp->pRightColumn, pApp->pStackPanel);
	xgeXuiWidgetSetSize(pApp->pStackBack, xgeXuiSizePercent(100.0f), xgeXuiSizePercent(100.0f));
	xgeXuiWidgetSetSize(pApp->pStackFront, xgeXuiSizePx(96.0f), xgeXuiSizePx(40.0f));
	xgeXuiWidgetSetAlign(pApp->pStackFront, XGE_XUI_ALIGN_CENTER, XGE_XUI_ALIGN_CENTER);
	xgeXuiWidgetAdd(pApp->pStackPanel, pApp->pStackBack);
	xgeXuiWidgetAdd(pApp->pStackPanel, pApp->pStackFront);

	xgeXuiWidgetSetLayout(pApp->pJustifyPanel, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetSize(pApp->pJustifyPanel, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(70.0f));
	xgeXuiWidgetSetGap(pApp->pJustifyPanel, 4.0f);
	xgeXuiWidgetSetJustify(pApp->pJustifyPanel, XGE_XUI_JUSTIFY_SPACE_BETWEEN);
	xgeXuiWidgetSetPaddingPx(pApp->pJustifyPanel, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetAdd(pApp->pRightColumn, pApp->pJustifyPanel);
	xgeXuiWidgetSetSize(pApp->pJustifyA, xgeXuiSizePx(28.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetSize(pApp->pJustifyB, xgeXuiSizePx(28.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetSize(pApp->pJustifyC, xgeXuiSizePx(28.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetAlign(pApp->pJustifyA, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER);
	xgeXuiWidgetSetAlign(pApp->pJustifyB, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER);
	xgeXuiWidgetSetAlign(pApp->pJustifyC, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER);
	xgeXuiWidgetAdd(pApp->pJustifyPanel, pApp->pJustifyA);
	xgeXuiWidgetAdd(pApp->pJustifyPanel, pApp->pJustifyB);
	xgeXuiWidgetAdd(pApp->pJustifyPanel, pApp->pJustifyC);

	xgeXuiWidgetSetLayout(pApp->pGridPanel, XGE_XUI_LAYOUT_GRID);
	xgeXuiWidgetSetGrid(pApp->pGridPanel, 2, 28.0f, 8.0f, 6.0f);
	xgeXuiWidgetSetSize(pApp->pGridPanel, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(108.0f));
	xgeXuiWidgetSetPaddingPx(pApp->pGridPanel, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetAdd(pApp->pRightColumn, pApp->pGridPanel);
	xgeXuiWidgetSetAlign(pApp->pGridA, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetAlign(pApp->pGridB, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetAlign(pApp->pGridC, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetSetAlign(pApp->pGridD, XGE_XUI_ALIGN_STRETCH, XGE_XUI_ALIGN_STRETCH);
	xgeXuiWidgetAdd(pApp->pGridPanel, pApp->pGridA);
	xgeXuiWidgetAdd(pApp->pGridPanel, pApp->pGridB);
	xgeXuiWidgetAdd(pApp->pGridPanel, pApp->pGridC);
	xgeXuiWidgetAdd(pApp->pGridPanel, pApp->pGridD);

	xgeXuiWidgetSetLayout(pApp->pMetricsPanel, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetSize(pApp->pMetricsPanel, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(78.0f));
	xgeXuiWidgetSetGap(pApp->pMetricsPanel, 8.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pMetricsPanel, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetAdd(pApp->pRightColumn, pApp->pMetricsPanel);
	xgeXuiLabelInit(&pApp->tContentLabel, pApp->pContentWidget, pFont, "content");
	xgeXuiLabelSetColor(&pApp->tContentLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pContentWidget, 8.0f, 4.0f, 8.0f, 4.0f);
	xgeXuiWidgetSetSize(pApp->pGrowWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(24.0f));
	xgeXuiWidgetSetAlign(pApp->pGrowWidget, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER);
	xgeXuiWidgetSetSize(pApp->pPercentWidget, xgeXuiSizePercent(28.0f), xgeXuiSizePx(24.0f));
	xgeXuiWidgetSetAlign(pApp->pPercentWidget, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER);
	xgeXuiWidgetSetSize(pApp->pPxWidget, xgeXuiSizePx(46.0f), xgeXuiSizePx(24.0f));
	xgeXuiWidgetSetAlign(pApp->pPxWidget, XGE_XUI_ALIGN_START, XGE_XUI_ALIGN_CENTER);
	xgeXuiWidgetAdd(pApp->pMetricsPanel, pApp->pContentWidget);
	xgeXuiWidgetAdd(pApp->pMetricsPanel, pApp->pGrowWidget);
	xgeXuiWidgetAdd(pApp->pMetricsPanel, pApp->pPercentWidget);
	xgeXuiWidgetAdd(pApp->pMetricsPanel, pApp->pPxWidget);
	return XGE_OK;
}

static float RectGapY(xge_rect_t tA, xge_rect_t tB)
{
	return tB.fY - (tA.fY + tA.fH);
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_rect_t tPanel;
	xge_rect_t tRectA;
	xge_rect_t tRectB;
	xge_rect_t tRowA;
	xge_rect_t tRowB;
	xge_rect_t tColumnA;
	xge_rect_t tColumnB;
	xge_rect_t tContent;
	xge_rect_t tGrow;
	xge_rect_t tPercent;
	xge_rect_t tPx;
	xge_rect_t tStackBack;
	xge_rect_t tStackFront;
	xge_rect_t tGridA;
	xge_rect_t tGridB;
	xge_rect_t tGridC;
	xge_rect_t tGridD;
	xge_rect_t tJustifyA;
	xge_rect_t tJustifyB;
	xge_rect_t tJustifyC;
	xge_vec2_t tDesired;
	int bNoRefreshDuringBatch;
	int bRefreshAfterBatch;
	int iPaintCommands;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	(void)PaintFrame(pApp);

	tPanel = pApp->pAbsolutePanel->tContentRect;
	tRectA = xgeXuiWidgetGetRect(pApp->pAbsoluteA);
	tRectB = xgeXuiWidgetGetRect(pApp->pAbsoluteB);
	pApp->bAbsoluteOK =
		FloatNear(tRectA.fX, tPanel.fX + 14.0f, 0.5f) &&
		FloatNear(tRectA.fY, tPanel.fY + 16.0f, 0.5f) &&
		FloatNear(tRectA.fW, 72.0f, 0.5f) &&
		FloatNear(tRectA.fH, 28.0f, 0.5f) &&
		FloatNear(tRectB.fX, tPanel.fX + 110.0f, 0.5f) &&
		FloatNear(tRectB.fY, tPanel.fY + 54.0f, 0.5f) &&
		(tRectB.fX > tRectA.fX) &&
		(tRectB.fY > tRectA.fY);

	tPanel = pApp->pRowPanel->tContentRect;
	tRowA = xgeXuiWidgetGetRect(pApp->pRowA);
	tRowB = xgeXuiWidgetGetRect(pApp->pRowB);
	pApp->bRowOK =
		(tRowA.fX < tRowB.fX) &&
		FloatNear(tRowB.fX - (tRowA.fX + tRowA.fW), 10.0f, 1.0f) &&
		(tRowA.fY > tPanel.fY) &&
		FloatNear((tRowB.fY + tRowB.fH), (tPanel.fY + tPanel.fH), 1.5f);

	tColumnA = xgeXuiWidgetGetRect(pApp->pColumnA);
	tColumnB = xgeXuiWidgetGetRect(pApp->pColumnB);
	pApp->fColumnGapBefore = RectGapY(tColumnA, tColumnB);
	pApp->bColumnOK =
		FloatNear(pApp->fColumnGapBefore, 6.0f, 1.0f) &&
		(tColumnA.fY < tColumnB.fY) &&
		(tColumnA.fX > tColumnB.fX);

	tStackBack = xgeXuiWidgetGetRect(pApp->pStackBack);
	tStackFront = xgeXuiWidgetGetRect(pApp->pStackFront);
	pApp->bStackOK =
		(tStackBack.fW > tStackFront.fW) &&
		(tStackBack.fH > tStackFront.fH) &&
		FloatNear((tStackFront.fX + tStackFront.fW * 0.5f), (tStackBack.fX + tStackBack.fW * 0.5f), 1.0f) &&
		FloatNear((tStackFront.fY + tStackFront.fH * 0.5f), (tStackBack.fY + tStackBack.fH * 0.5f), 1.0f);

	tGridA = xgeXuiWidgetGetRect(pApp->pGridA);
	tGridB = xgeXuiWidgetGetRect(pApp->pGridB);
	tGridC = xgeXuiWidgetGetRect(pApp->pGridC);
	tGridD = xgeXuiWidgetGetRect(pApp->pGridD);
	pApp->bGridOK =
		FloatNear(tGridA.fY, tGridB.fY, 0.5f) &&
		FloatNear(tGridC.fY, tGridD.fY, 0.5f) &&
		(tGridB.fX > tGridA.fX) &&
		(tGridC.fY > tGridA.fY) &&
		(tGridD.fX > tGridC.fX);

	tContent = xgeXuiWidgetGetRect(pApp->pContentWidget);
	tGrow = xgeXuiWidgetGetRect(pApp->pGrowWidget);
	tPercent = xgeXuiWidgetGetRect(pApp->pPercentWidget);
	tPx = xgeXuiWidgetGetRect(pApp->pPxWidget);
	tDesired = xgeXuiWidgetGetDesiredSize(pApp->pContentWidget);
	pApp->bSizeOK =
		(pApp->pContentWidget->tStyle.tWidth.iUnit == XGE_XUI_SIZE_CONTENT) &&
		(tDesired.fX > 0.0f) &&
		(tContent.fW > 0.0f) &&
		(pApp->pGrowWidget->tStyle.tWidth.iUnit == XGE_XUI_SIZE_GROW) &&
		(tGrow.fW > tPx.fW) &&
		(pApp->pPercentWidget->tStyle.tWidth.iUnit == XGE_XUI_SIZE_PERCENT) &&
		(tPercent.fW > 20.0f) &&
		(tPercent.fW < tGrow.fW) &&
		(pApp->pPxWidget->tStyle.tWidth.iUnit == XGE_XUI_SIZE_PX) &&
		FloatNear(tPx.fW, 46.0f, 1.0f);

	tPanel = pApp->pJustifyPanel->tContentRect;
	tJustifyA = xgeXuiWidgetGetRect(pApp->pJustifyA);
	tJustifyB = xgeXuiWidgetGetRect(pApp->pJustifyB);
	tJustifyC = xgeXuiWidgetGetRect(pApp->pJustifyC);
	pApp->bGapJustifyAlignOK =
		FloatNear(tJustifyA.fX, tPanel.fX, 1.0f) &&
		FloatNear((tJustifyC.fX + tJustifyC.fW), (tPanel.fX + tPanel.fW), 1.0f) &&
		(tJustifyB.fX > tJustifyA.fX) &&
		(tRowA.fY > pApp->pRowPanel->tContentRect.fY);

	iPaintCommands = 0;
	bNoRefreshDuringBatch = (xgeXuiRefreshNeeded(&pApp->tXui) == 0);
	xgeXuiLayoutBatchBegin(&pApp->tXui);
	xgeXuiWidgetSetGap(pApp->pColumnPanel, 18.0f);
	bNoRefreshDuringBatch = bNoRefreshDuringBatch && (xgeXuiRefreshNeeded(&pApp->tXui) == 0);
	xgeXuiLayoutBatchEnd(&pApp->tXui);
	bRefreshAfterBatch =
		(xgeXuiRefreshNeeded(&pApp->tXui) != 0) &&
		((xgeXuiRoot(&pApp->tXui)->iFlags & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0);
	xgeXuiWidgetSetGap(pApp->pColumnPanel, 18.0f);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	iPaintCommands = PaintFrame(pApp);
	tColumnA = xgeXuiWidgetGetRect(pApp->pColumnA);
	tColumnB = xgeXuiWidgetGetRect(pApp->pColumnB);
	pApp->fColumnGapAfter = RectGapY(tColumnA, tColumnB);
	pApp->bDirtyLayoutOK =
		bNoRefreshDuringBatch &&
		bRefreshAfterBatch &&
		FloatNear(pApp->fColumnGapBefore, 6.0f, 1.0f) &&
		FloatNear(pApp->fColumnGapAfter, 18.0f, 1.0f) &&
		(iPaintCommands > 0);

	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	(void)PaintFrame(pApp);
	return (pApp->bAbsoluteOK &&
		pApp->bRowOK &&
		pApp->bColumnOK &&
		pApp->bStackOK &&
		pApp->bGridOK &&
		pApp->bSizeOK &&
		pApp->bGapJustifyAlignOK &&
		pApp->bDirtyLayoutOK) ? XGE_OK : XGE_ERROR;
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	pApp->iFrameCount++;
	if ( xgeKeyDown(XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());
	(void)PaintFrame(pApp);
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		xgeQuit();
	}
	return 0;
}

int main(int argc, char** argv)
{
	app_state_t tApp;
	xge_desc_t tDesc;
	int i;
	int iRet;

	memset(&tApp, 0, sizeof(tApp));
	memset(&tDesc, 0, sizeof(tDesc));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_LAYOUT_GALLERY_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}

	tDesc.iWidth = 900;
	tDesc.iHeight = 560;
	tDesc.sTitle = "XGE XUI Layout Gallery";
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
	(void)LoadFont(&tApp);
	if ( CreateUI(&tApp) != XGE_OK ) {
		xgeXuiUnit(&tApp.tXui);
		xgeUnit();
		return 3;
	}
	iRet = RunStaticChecks(&tApp);
	xgeRun(AppFrame, &tApp);
	printf(
		"xui-layout-gallery final-summary frames=%d absolute=%d row=%d column=%d stack=%d grid=%d size=%d gja=%d dirty=%d gap=%.0f->%.0f font=%d\n",
		tApp.iFrameCount,
		tApp.bAbsoluteOK,
		tApp.bRowOK,
		tApp.bColumnOK,
		tApp.bStackOK,
		tApp.bGridOK,
		tApp.bSizeOK,
		tApp.bGapJustifyAlignOK,
		tApp.bDirtyLayoutOK,
		tApp.fColumnGapBefore,
		tApp.fColumnGapAfter,
		tApp.bFontReady);
	printf("xui-layout-gallery summary frames=%d/%d\n", tApp.iFrameCount, tApp.iFrameLimit);
	xgeXuiLabelUnit(&tApp.tHeaderLabel);
	xgeXuiLabelUnit(&tApp.tStatusLabel);
	xgeXuiLabelUnit(&tApp.tContentLabel);
	if ( tApp.bFontReady != 0 ) {
		xgeFontFree(&tApp.tFont);
	}
	xgeXuiUnit(&tApp.tXui);
	xgeUnit();
	return (iRet == XGE_OK) ? 0 : 4;
}

