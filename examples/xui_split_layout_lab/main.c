#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pStatusTextWidget;
	xge_xui_widget pContentWidget;
	xge_xui_widget pVerticalCardWidget;
	xge_xui_widget pHorizontalCardWidget;
	xge_xui_widget pVerticalTitleWidget;
	xge_xui_widget pHorizontalTitleWidget;
	xge_xui_widget pVerticalSplitWidget;
	xge_xui_widget pHorizontalSplitWidget;
	xge_xui_widget pHintWidget;
	xge_xui_label_t arrLabels[64];
	int iLabelCount;
	xge_xui_label_t* pStatusLabel;
	xge_xui_split_layout_t tVerticalSplit;
	xge_xui_split_layout_t tHorizontalSplit;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bInitOK;
	int bVerticalOK;
	int bHorizontalOK;
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
			printf("xui-split-layout-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-split-layout-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_label_t* AllocLabel(app_state_t* pApp)
{
	if ( pApp->iLabelCount >= (int)(sizeof(pApp->arrLabels) / sizeof(pApp->arrLabels[0])) ) {
		return NULL;
	}
	return &pApp->arrLabels[pApp->iLabelCount++];
}

static xge_xui_widget CreateLabelWidget(app_state_t* pApp, xge_xui_widget pParent, const char* sText, uint32_t iColor, uint32_t iFlags, float fHeight, int bGrow)
{
	xge_xui_widget pWidget;
	xge_xui_label_t* pLabel;

	pWidget = xgeXuiWidgetCreate();
	pLabel = AllocLabel(pApp);
	if ( (pWidget == NULL) || (pLabel == NULL) ) {
		return NULL;
	}
	if ( bGrow != 0 ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	} else {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(fHeight));
	}
	if ( xgeXuiLabelInit(pLabel, pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return NULL;
	}
	xgeXuiLabelSetColor(pLabel, iColor);
	xgeXuiLabelSetAlign(pLabel, iFlags);
	(void)xgeXuiWidgetAdd(pParent, pWidget);
	return pWidget;
}

static xge_xui_widget CreateBlock(app_state_t* pApp, xge_xui_widget pParent, const char* sText, uint32_t iBackground, float fWidth, float fHeight, int bGrow)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return NULL;
	}
	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetRadius(pWidget, 6.0f);
	xgeXuiWidgetSetBackground(pWidget, iBackground);
	if ( bGrow != 0 ) {
		xgeXuiWidgetSetSize(pWidget, (fWidth > 0.0f) ? xgeXuiSizePx(fWidth) : xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	} else {
		xgeXuiWidgetSetSize(pWidget, (fWidth > 0.0f) ? xgeXuiSizePx(fWidth) : xgeXuiSizeGrow(1.0f), xgeXuiSizePx(fHeight));
	}
	(void)xgeXuiWidgetAdd(pParent, pWidget);
	(void)CreateLabelWidget(
		pApp,
		pWidget,
		sText,
		XGE_COLOR_RGBA(242, 246, 252, 255),
		XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP,
		22.0f,
		1);
	return pWidget;
}

static xge_xui_widget CreatePaneBody(app_state_t* pApp, xge_xui_widget pPane, const char* sTitle, uint32_t iBackground)
{
	xge_xui_widget pBody;

	xgeXuiWidgetSetBackground(pPane, iBackground);
	xgeXuiWidgetSetRadius(pPane, 8.0f);
	xgeXuiWidgetSetGap(pPane, 8.0f);
	xgeXuiWidgetSetPaddingPx(pPane, 12.0f, 12.0f, 12.0f, 12.0f);
	(void)CreateLabelWidget(
		pApp,
		pPane,
		sTitle,
		XGE_COLOR_RGBA(250, 252, 255, 255),
		XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP,
		24.0f,
		0);
	pBody = xgeXuiWidgetCreate();
	if ( pBody == NULL ) {
		return NULL;
	}
	xgeXuiWidgetSetLayout(pBody, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pBody, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetGap(pBody, 8.0f);
	(void)xgeXuiWidgetAdd(pPane, pBody);
	return pBody;
}

static void PopulateVerticalSplit(app_state_t* pApp)
{
	xge_xui_widget pPane;
	xge_xui_widget pBody;
	xge_xui_widget pLeftColumn;
	xge_xui_widget pRightColumn;
	int i;

	xgeXuiSplitLayoutSetOrientation(&pApp->tVerticalSplit, XGE_XUI_SEPARATOR_VERTICAL);
	xgeXuiSplitLayoutSetPaneCount(&pApp->tVerticalSplit, 3);
	xgeXuiSplitLayoutSetDividerSize(&pApp->tVerticalSplit, 10.0f);
	xgeXuiSplitLayoutSetShadowDrag(&pApp->tVerticalSplit, 1);
	xgeXuiSplitLayoutSetPaneMinSize(&pApp->tVerticalSplit, 0, 130.0f);
	xgeXuiSplitLayoutSetPaneMinSize(&pApp->tVerticalSplit, 1, 180.0f);
	xgeXuiSplitLayoutSetPaneMinSize(&pApp->tVerticalSplit, 2, 150.0f);
	xgeXuiSplitLayoutSetPaneWeight(&pApp->tVerticalSplit, 0, 0.9f);
	xgeXuiSplitLayoutSetPaneWeight(&pApp->tVerticalSplit, 1, 1.8f);
	xgeXuiSplitLayoutSetPaneWeight(&pApp->tVerticalSplit, 2, 1.0f);

	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->tVerticalSplit, 0);
	pBody = CreatePaneBody(pApp, pPane, "Navigator", XGE_COLOR_RGBA(34, 42, 54, 255));
	(void)CreateBlock(pApp, pBody, "Scene Tree", XGE_COLOR_RGBA(54, 92, 126, 255), 0.0f, 28.0f, 0);
	(void)CreateBlock(pApp, pBody, "Assets", XGE_COLOR_RGBA(64, 104, 132, 255), 0.0f, 28.0f, 0);
	(void)CreateBlock(pApp, pBody, "Layers", XGE_COLOR_RGBA(76, 114, 138, 255), 0.0f, 28.0f, 0);
	(void)CreateBlock(pApp, pBody, "History", XGE_COLOR_RGBA(88, 122, 144, 255), 0.0f, 28.0f, 0);
	(void)CreateBlock(pApp, pBody, "Selection", XGE_COLOR_RGBA(44, 56, 72, 255), 0.0f, 0.0f, 1);

	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->tVerticalSplit, 1);
	pBody = CreatePaneBody(pApp, pPane, "Canvas", XGE_COLOR_RGBA(28, 36, 48, 255));
	pBody->tStyle.iLayout = XGE_XUI_LAYOUT_ROW;
	pLeftColumn = xgeXuiWidgetCreate();
	pRightColumn = xgeXuiWidgetCreate();
	if ( (pLeftColumn != NULL) && (pRightColumn != NULL) ) {
		xgeXuiWidgetSetLayout(pLeftColumn, XGE_XUI_LAYOUT_COLUMN);
		xgeXuiWidgetSetLayout(pRightColumn, XGE_XUI_LAYOUT_COLUMN);
		xgeXuiWidgetSetGap(pLeftColumn, 8.0f);
		xgeXuiWidgetSetGap(pRightColumn, 8.0f);
		xgeXuiWidgetSetSize(pLeftColumn, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
		xgeXuiWidgetSetSize(pRightColumn, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
		(void)xgeXuiWidgetAdd(pBody, pLeftColumn);
		(void)xgeXuiWidgetAdd(pBody, pRightColumn);
		(void)CreateBlock(pApp, pLeftColumn, "Viewport", XGE_COLOR_RGBA(82, 118, 164, 255), 0.0f, 0.0f, 1);
		(void)CreateBlock(pApp, pLeftColumn, "Timeline", XGE_COLOR_RGBA(62, 92, 130, 255), 0.0f, 0.0f, 1);
		(void)CreateBlock(pApp, pRightColumn, "Properties", XGE_COLOR_RGBA(56, 82, 112, 255), 0.0f, 0.0f, 1);
		(void)CreateBlock(pApp, pRightColumn, "Preview", XGE_COLOR_RGBA(72, 104, 140, 255), 0.0f, 0.0f, 1);
	}

	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->tVerticalSplit, 2);
	pBody = CreatePaneBody(pApp, pPane, "Inspector", XGE_COLOR_RGBA(36, 44, 58, 255));
	for ( i = 0; i < 3; i++ ) {
		char sText[32];
		snprintf(sText, sizeof(sText), "Field %d", i + 1);
		(void)CreateBlock(pApp, pBody, sText, XGE_COLOR_RGBA(78 + i * 10, 98 + i * 8, 120 + i * 6, 255), 0.0f, 28.0f, 0);
	}
	(void)CreateBlock(pApp, pBody, "Notes", XGE_COLOR_RGBA(48, 60, 78, 255), 0.0f, 0.0f, 1);
}

static void PopulateHorizontalSplit(app_state_t* pApp)
{
	xge_xui_widget pPane;
	xge_xui_widget pBody;
	xge_xui_widget pRow;
	int i;

	xgeXuiSplitLayoutSetOrientation(&pApp->tHorizontalSplit, XGE_XUI_SEPARATOR_HORIZONTAL);
	xgeXuiSplitLayoutSetPaneCount(&pApp->tHorizontalSplit, 3);
	xgeXuiSplitLayoutSetDividerSize(&pApp->tHorizontalSplit, 10.0f);
	xgeXuiSplitLayoutSetShadowDrag(&pApp->tHorizontalSplit, 1);
	xgeXuiSplitLayoutSetPaneMinSize(&pApp->tHorizontalSplit, 0, 96.0f);
	xgeXuiSplitLayoutSetPaneMinSize(&pApp->tHorizontalSplit, 1, 144.0f);
	xgeXuiSplitLayoutSetPaneMinSize(&pApp->tHorizontalSplit, 2, 110.0f);
	xgeXuiSplitLayoutSetPaneWeight(&pApp->tHorizontalSplit, 0, 0.8f);
	xgeXuiSplitLayoutSetPaneWeight(&pApp->tHorizontalSplit, 1, 1.5f);
	xgeXuiSplitLayoutSetPaneWeight(&pApp->tHorizontalSplit, 2, 0.9f);

	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->tHorizontalSplit, 0);
	pBody = CreatePaneBody(pApp, pPane, "Preview", XGE_COLOR_RGBA(34, 42, 54, 255));
	pBody->tStyle.iLayout = XGE_XUI_LAYOUT_ROW;
	pBody->tStyle.fGap = 8.0f;
	(void)CreateBlock(pApp, pBody, "Hero", XGE_COLOR_RGBA(88, 130, 170, 255), 0.0f, 0.0f, 1);
	(void)CreateBlock(pApp, pBody, "Stats", XGE_COLOR_RGBA(70, 108, 146, 255), 0.0f, 0.0f, 1);
	(void)CreateBlock(pApp, pBody, "Mini Map", XGE_COLOR_RGBA(60, 92, 126, 255), 0.0f, 0.0f, 1);

	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->tHorizontalSplit, 1);
	pBody = CreatePaneBody(pApp, pPane, "Workbench", XGE_COLOR_RGBA(28, 36, 48, 255));
	pBody->tStyle.iLayout = XGE_XUI_LAYOUT_ROW;
	pRow = xgeXuiWidgetCreate();
	if ( pRow != NULL ) {
		xgeXuiWidgetSetLayout(pRow, XGE_XUI_LAYOUT_COLUMN);
		xgeXuiWidgetSetGap(pRow, 8.0f);
		xgeXuiWidgetSetSize(pRow, xgeXuiSizePx(110.0f), xgeXuiSizeGrow(1.0f));
		(void)xgeXuiWidgetAdd(pBody, pRow);
		(void)CreateBlock(pApp, pRow, "Tools", XGE_COLOR_RGBA(72, 106, 140, 255), 0.0f, 0.0f, 1);
		(void)CreateBlock(pApp, pRow, "Brush", XGE_COLOR_RGBA(66, 100, 132, 255), 0.0f, 0.0f, 1);
	}
	pRow = xgeXuiWidgetCreate();
	if ( pRow != NULL ) {
		xgeXuiWidgetSetLayout(pRow, XGE_XUI_LAYOUT_COLUMN);
		xgeXuiWidgetSetGap(pRow, 8.0f);
		xgeXuiWidgetSetSize(pRow, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
		(void)xgeXuiWidgetAdd(pBody, pRow);
		(void)CreateBlock(pApp, pRow, "Stage", XGE_COLOR_RGBA(88, 130, 170, 255), 0.0f, 0.0f, 1);
		(void)CreateBlock(pApp, pRow, "Timeline", XGE_COLOR_RGBA(58, 88, 120, 255), 0.0f, 0.0f, 1);
	}

	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->tHorizontalSplit, 2);
	pBody = CreatePaneBody(pApp, pPane, "Log", XGE_COLOR_RGBA(36, 44, 58, 255));
	for ( i = 0; i < 3; i++ ) {
		char sText[48];
		snprintf(sText, sizeof(sText), "Event line %d", i + 1);
		(void)CreateBlock(pApp, pBody, sText, XGE_COLOR_RGBA(60 + i * 8, 86 + i * 8, 116 + i * 8, 255), 0.0f, 26.0f, 0);
	}
	(void)CreateBlock(pApp, pBody, "Tail Output", XGE_COLOR_RGBA(44, 56, 72, 255), 0.0f, 0.0f, 1);
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[320];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d vertical=%d horizontal=%d shadow=1 v=%.0f/%.0f/%.0f h=%.0f/%.0f/%.0f",
		pApp->bInitOK,
		pApp->bVerticalOK,
		pApp->bHorizontalOK,
		xgeXuiSplitLayoutGetPaneSize(&pApp->tVerticalSplit, 0),
		xgeXuiSplitLayoutGetPaneSize(&pApp->tVerticalSplit, 1),
		xgeXuiSplitLayoutGetPaneSize(&pApp->tVerticalSplit, 2),
		xgeXuiSplitLayoutGetPaneSize(&pApp->tHorizontalSplit, 0),
		xgeXuiSplitLayoutGetPaneSize(&pApp->tHorizontalSplit, 1),
		xgeXuiSplitLayoutGetPaneSize(&pApp->tHorizontalSplit, 2));
	xgeXuiLabelSetText(pApp->pStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	tTheme.iTextColor = XGE_COLOR_RGBA(240, 244, 250, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(18, 24, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(30, 38, 50, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 96, 118, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(112, 172, 255, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(50, 64, 84, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(68, 84, 108, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(94, 142, 216, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(76, 122, 200, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(78, 82, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pRoot, 18.0f, 18.0f, 18.0f, 18.0f);
	xgeXuiWidgetSetGap(pRoot, 12.0f);
	xgeXuiWidgetSetBackground(pRoot, XGE_COLOR_RGBA(18, 24, 34, 255));

	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pStatusTextWidget = xgeXuiWidgetCreate();
	pApp->pContentWidget = xgeXuiWidgetCreate();
	pApp->pVerticalCardWidget = xgeXuiWidgetCreate();
	pApp->pHorizontalCardWidget = xgeXuiWidgetCreate();
	pApp->pVerticalTitleWidget = xgeXuiWidgetCreate();
	pApp->pHorizontalTitleWidget = xgeXuiWidgetCreate();
	pApp->pVerticalSplitWidget = xgeXuiWidgetCreate();
	pApp->pHorizontalSplitWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pStatusWidget == NULL) || (pApp->pStatusTextWidget == NULL) || (pApp->pContentWidget == NULL) ||
	     (pApp->pVerticalCardWidget == NULL) || (pApp->pHorizontalCardWidget == NULL) ||
	     (pApp->pVerticalTitleWidget == NULL) || (pApp->pHorizontalTitleWidget == NULL) ||
	     (pApp->pVerticalSplitWidget == NULL) || (pApp->pHorizontalSplitWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pStatusWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetLayout(pApp->pContentWidget, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetGap(pApp->pContentWidget, 12.0f);
	xgeXuiWidgetSetSize(pApp->pContentWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pApp->pStatusWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(40.0f));
	xgeXuiWidgetSetSize(pApp->pHintWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(28.0f));
	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 56, 74, 255));
	xgeXuiWidgetSetRadius(pApp->pStatusWidget, 8.0f);

	xgeXuiWidgetSetLayout(pApp->pVerticalCardWidget, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetLayout(pApp->pHorizontalCardWidget, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetGap(pApp->pVerticalCardWidget, 10.0f);
	xgeXuiWidgetSetGap(pApp->pHorizontalCardWidget, 10.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pVerticalCardWidget, 12.0f, 12.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pHorizontalCardWidget, 12.0f, 12.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetBackground(pApp->pVerticalCardWidget, XGE_COLOR_RGBA(28, 36, 48, 255));
	xgeXuiWidgetSetBackground(pApp->pHorizontalCardWidget, XGE_COLOR_RGBA(28, 36, 48, 255));
	xgeXuiWidgetSetRadius(pApp->pVerticalCardWidget, 8.0f);
	xgeXuiWidgetSetRadius(pApp->pHorizontalCardWidget, 8.0f);
	xgeXuiWidgetSetSize(pApp->pVerticalCardWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pApp->pHorizontalCardWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pApp->pVerticalTitleWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(24.0f));
	xgeXuiWidgetSetSize(pApp->pHorizontalTitleWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizePx(24.0f));
	xgeXuiWidgetSetSize(pApp->pVerticalSplitWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetSize(pApp->pHorizontalSplitWidget, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));

	(void)xgeXuiWidgetAdd(pRoot, pApp->pStatusWidget);
	(void)xgeXuiWidgetAdd(pRoot, pApp->pContentWidget);
	(void)xgeXuiWidgetAdd(pRoot, pApp->pHintWidget);
	(void)xgeXuiWidgetAdd(pApp->pStatusWidget, pApp->pStatusTextWidget);
	(void)xgeXuiWidgetAdd(pApp->pContentWidget, pApp->pVerticalCardWidget);
	(void)xgeXuiWidgetAdd(pApp->pContentWidget, pApp->pHorizontalCardWidget);
	(void)xgeXuiWidgetAdd(pApp->pVerticalCardWidget, pApp->pVerticalTitleWidget);
	(void)xgeXuiWidgetAdd(pApp->pVerticalCardWidget, pApp->pVerticalSplitWidget);
	(void)xgeXuiWidgetAdd(pApp->pHorizontalCardWidget, pApp->pHorizontalTitleWidget);
	(void)xgeXuiWidgetAdd(pApp->pHorizontalCardWidget, pApp->pHorizontalSplitWidget);

	pApp->pStatusLabel = AllocLabel(pApp);
	if ( pApp->pStatusLabel == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiLabelInit(pApp->pStatusLabel, pApp->pStatusTextWidget, pApp->bFontReady ? &pApp->tFont : NULL, "xui split layout lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(pApp->pStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiLabelSetAlign(pApp->pStatusLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	xgeXuiWidgetSetRect(pApp->pStatusTextWidget, (xge_rect_t){ 10.0f, 8.0f, 900.0f, 24.0f });

	(void)CreateLabelWidget(
		pApp,
		pApp->pVerticalTitleWidget,
		"Vertical SplitLayout (3 panes, shadow drag, min width)",
		XGE_COLOR_RGBA(240, 244, 250, 255),
		XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP,
		24.0f,
		1);
	(void)CreateLabelWidget(
		pApp,
		pApp->pHorizontalTitleWidget,
		"Horizontal SplitLayout (3 panes, shadow drag, min height)",
		XGE_COLOR_RGBA(240, 244, 250, 255),
		XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP,
		24.0f,
		1);
	(void)CreateLabelWidget(
		pApp,
		pApp->pHintWidget,
		"Drag dividers in both cards. A shadow line moves first; layout commits on release.",
		XGE_COLOR_RGBA(164, 182, 206, 255),
		XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP,
		28.0f,
		1);

	if ( xgeXuiSplitLayoutInit(&pApp->tVerticalSplit, &pApp->tXui, pApp->pVerticalSplitWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiSplitLayoutInit(&pApp->tHorizontalSplit, &pApp->tXui, pApp->pHorizontalSplitWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	PopulateVerticalSplit(pApp);
	PopulateHorizontalSplit(pApp);

	pApp->bInitOK = 1;
	pApp->bVerticalOK = (xgeXuiSplitLayoutGetPaneCount(&pApp->tVerticalSplit) == 3);
	pApp->bHorizontalOK = (xgeXuiSplitLayoutGetPaneCount(&pApp->tHorizontalSplit) == 3);
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
	if ( pEvent != NULL ) {
		if ( pEvent->iType == XGE_EVENT_QUIT ) {
			printf("[xui_split_layout_lab] quit-event\n");
		} else if ( pEvent->iType == XGE_EVENT_RESIZE ) {
			printf("[xui_split_layout_lab] resize-event w=%d h=%d\n", pEvent->iParam1, pEvent->iParam2);
		}
	}
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
	if ( pApp->tHorizontalSplit.pWidget != NULL ) {
		xgeXuiSplitLayoutUnit(&pApp->tHorizontalSplit);
	}
	if ( pApp->tVerticalSplit.pWidget != NULL ) {
		xgeXuiSplitLayoutUnit(&pApp->tVerticalSplit);
	}
	xgeXuiUnit(&pApp->tXui);
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
	tApp.iFrameLimit = 0;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			tApp.iFrameLimit = ArgInt(argv[++i], 0);
		}
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 1280;
	tDesc.iHeight = 760;
	tDesc.sTitle = "XGE XUI Split Layout Lab";
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
		"xui-split-layout-lab final-summary frames=%d init=%d vertical=%d horizontal=%d shadow=1 v=%.0f/%.0f/%.0f h=%.0f/%.0f/%.0f\n",
		tApp.iFrameCount,
		tApp.bInitOK,
		tApp.bVerticalOK,
		tApp.bHorizontalOK,
		xgeXuiSplitLayoutGetPaneSize(&tApp.tVerticalSplit, 0),
		xgeXuiSplitLayoutGetPaneSize(&tApp.tVerticalSplit, 1),
		xgeXuiSplitLayoutGetPaneSize(&tApp.tVerticalSplit, 2),
		xgeXuiSplitLayoutGetPaneSize(&tApp.tHorizontalSplit, 0),
		xgeXuiSplitLayoutGetPaneSize(&tApp.tHorizontalSplit, 1),
		xgeXuiSplitLayoutGetPaneSize(&tApp.tHorizontalSplit, 2));
	xgeSceneSet(NULL);
	AppUnit(&tApp);
	xgeUnit();
	return 0;
}
