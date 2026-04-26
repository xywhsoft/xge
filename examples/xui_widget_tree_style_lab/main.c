#include "../../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pProbeWidget;
	xge_xui_widget pAnchoredWidget;
	xge_xui_widget pDetachedWidget;
	xge_xui_label_t tStatusLabel;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iPaintCount;
	int iUpdateCount;
	int bTreeOK;
	int bFindOK;
	int bRectOK;
	int bFlagsOK;
	int bStyleOK;
	int bMarkLayoutOK;
	int bMarkPaintOK;
	int bRefreshOK;
	int bPaintOK;
	int bStaticChecksOK;
	int iLastWidth;
	int iLastHeight;
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

static int RectNear(xge_rect_t tA, xge_rect_t tB, float fEpsilon)
{
	return FloatNear(tA.fX, tB.fX, fEpsilon) &&
		FloatNear(tA.fY, tB.fY, fEpsilon) &&
		FloatNear(tA.fW, tB.fW, fEpsilon) &&
		FloatNear(tA.fH, tB.fH, fEpsilon);
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
			printf("xui-widget-tree-style-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-widget-tree-style-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void ProbeUpdate(xge_xui_widget pWidget, float fDelta, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)fDelta;
	pApp = (app_state_t*)pUser;
	pApp->iUpdateCount++;
}

static void ProbePaint(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;
	xge_rect_t tOuter;
	xge_rect_t tInner;

	pApp = (app_state_t*)pUser;
	pApp->iPaintCount++;
	tOuter = pWidget->tRect;
	tInner = pWidget->tContentRect;
	xgeShapeRectStrokePx(tOuter, 2.0f, XGE_COLOR_RGBA(255, 208, 96, 255));
	xgeShapeRectStrokePx(tInner, 1.0f, XGE_COLOR_RGBA(96, 224, 170, 255));
	xgeShapeLinePx(tInner.fX, tInner.fY, tInner.fX + tInner.fW, tInner.fY + tInner.fH, 2.0f, XGE_COLOR_RGBA(96, 168, 255, 255));
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
	tRect.fX = 24.0f;
	tRect.fY = 24.0f;
	tRect.fW = (float)iWidth - 48.0f;
	tRect.fH = (float)iHeight - 48.0f;
	if ( tRect.fW < 240.0f ) {
		tRect.fW = 240.0f;
	}
	if ( tRect.fH < 180.0f ) {
		tRect.fH = 180.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRect);
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static int PaintXui(app_state_t* pApp)
{
	int iPaintCommands;

	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	iPaintCommands = xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return iPaintCommands;
}

static void UpdateStatusLabel(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"tree=%d find=%d rect=%d flags=%d style=%d marks=%d/%d refresh=%d paint=%d paint_calls=%d update_calls=%d",
		pApp->bTreeOK,
		pApp->bFindOK,
		pApp->bRectOK,
		pApp->bFlagsOK,
		pApp->bStyleOK,
		pApp->bMarkLayoutOK,
		pApp->bMarkPaintOK,
		pApp->bRefreshOK,
		pApp->bPaintOK,
		pApp->iPaintCount,
		pApp->iUpdateCount);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_style_t tStyle;
	xge_xui_theme_t tTheme;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(236, 240, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(26, 32, 44, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(30, 38, 52, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(92, 164, 255, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(54, 88, 130, 255);
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 8.0f;
	tTheme.fRadius = 8.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pProbeWidget = xgeXuiWidgetCreate();
	pApp->pAnchoredWidget = xgeXuiWidgetCreate();
	pApp->pDetachedWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pProbeWidget == NULL) || (pApp->pAnchoredWidget == NULL) || (pApp->pDetachedWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetId(pApp->pRootPanel, 101);
	xgeXuiWidgetSetName(pApp->pRootPanel, "root-panel");
	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 42, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pRootPanel, 16.0f, 16.0f, 16.0f, 16.0f);
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	if ( xgeXuiWidgetAdd(pRoot, pApp->pRootPanel) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetId(pApp->pStatusWidget, 102);
	xgeXuiWidgetSetName(pApp->pStatusWidget, "status");
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, 520.0f, 32.0f });
	xgeXuiWidgetSetAnchorPx(pApp->pStatusWidget, XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_TOP | XGE_XUI_ANCHOR_RIGHT, 24.0f, 12.0f, 24.0f, 0.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 6.0f, 10.0f, 6.0f);
	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 58, 76, 255));
	xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui widget tree/style lab");
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetId(pApp->pProbeWidget, 201);
	xgeXuiWidgetSetName(pApp->pProbeWidget, "probe");
	xgeXuiStyleDefault(&tStyle);
	xgeXuiStyleFromTheme(&tStyle, xgeXuiGetTheme(&pApp->tXui));
	tStyle.iLayout = XGE_XUI_LAYOUT_ABSOLUTE;
	tStyle.iBackgroundColor = XGE_COLOR_RGBA(54, 88, 130, 255);
	tStyle.fRadius = 10.0f;
	xgeXuiWidgetSetStyle(pApp->pProbeWidget, &tStyle);
	xgeXuiWidgetSetRect(pApp->pProbeWidget, (xge_rect_t){ 26.0f, 56.0f, 220.0f, 120.0f });
	xgeXuiWidgetSetMarginPx(pApp->pProbeWidget, 4.0f, 6.0f, 8.0f, 10.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pProbeWidget, 12.0f, 14.0f, 16.0f, 18.0f);
	xgeXuiWidgetSetAnchorPx(pApp->pProbeWidget, XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_TOP, 26.0f, 56.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetEnabled(pApp->pProbeWidget, 1);
	xgeXuiWidgetSetFocusable(pApp->pProbeWidget, 1);
	xgeXuiWidgetSetClip(pApp->pProbeWidget, 1);
	xgeXuiWidgetSetUpdate(pApp->pProbeWidget, ProbeUpdate, pApp);
	xgeXuiWidgetSetPaint(pApp->pProbeWidget, ProbePaint, pApp);
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pProbeWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetId(pApp->pAnchoredWidget, 202);
	xgeXuiWidgetSetName(pApp->pAnchoredWidget, "anchored");
	xgeXuiWidgetSetRect(pApp->pAnchoredWidget, (xge_rect_t){ 0.0f, 0.0f, 260.0f, 74.0f });
	xgeXuiWidgetSetAnchorPx(pApp->pAnchoredWidget, XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_RIGHT | XGE_XUI_ANCHOR_BOTTOM, 32.0f, 0.0f, 36.0f, 24.0f);
	xgeXuiWidgetSetMarginPx(pApp->pAnchoredWidget, 3.0f, 5.0f, 7.0f, 9.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pAnchoredWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pAnchoredWidget, XGE_COLOR_RGBA(48, 72, 56, 255));
	xgeXuiWidgetSetRadius(pApp->pAnchoredWidget, 6.0f);
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pAnchoredWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetId(pApp->pDetachedWidget, 303);
	xgeXuiWidgetSetName(pApp->pDetachedWidget, "temp-remove");
	xgeXuiWidgetSetRect(pApp->pDetachedWidget, (xge_rect_t){ 520.0f, 58.0f, 120.0f, 72.0f });
	xgeXuiWidgetSetBackground(pApp->pDetachedWidget, XGE_COLOR_RGBA(98, 64, 84, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pDetachedWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	pApp->bTreeOK = 1;
	return XGE_OK;
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	const xge_xui_style_t* pStyle;
	xge_rect_t tRootRect;
	xge_rect_t tRootContent;
	xge_rect_t tProbeRect;
	xge_rect_t tExpectedProbeRect;
	xge_rect_t tExpectedProbeContent;
	int bHiddenOK;
	int bVisibleOK;
	int bEnabledOK;
	int bFocusableOK;
	int bClipOK;
	int iPaintCommands;
	int iRefreshPaintOnly;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	(void)PaintXui(pApp);
	pRoot = xgeXuiRoot(&pApp->tXui);
	pApp->bFindOK =
		(xgeXuiWidgetFindById(pRoot, 101) == pApp->pRootPanel) &&
		(xgeXuiWidgetFindById(pRoot, 201) == pApp->pProbeWidget) &&
		(xgeXuiWidgetFindByName(pRoot, "anchored") == pApp->pAnchoredWidget) &&
		(xgeXuiWidgetGetId(pApp->pProbeWidget) == 201) &&
		(strcmp(xgeXuiWidgetGetName(pApp->pProbeWidget), "probe") == 0);

	xgeXuiWidgetSetVisible(pApp->pDetachedWidget, 0);
	bHiddenOK = (xgeXuiWidgetIsVisible(pApp->pDetachedWidget) == 0);
	xgeXuiWidgetSetVisible(pApp->pDetachedWidget, 1);
	xgeXuiWidgetSetEnabled(pApp->pDetachedWidget, 1);
	xgeXuiWidgetSetFocusable(pApp->pDetachedWidget, 1);
	xgeXuiWidgetSetClip(pApp->pDetachedWidget, 1);
	bVisibleOK = (xgeXuiWidgetIsVisible(pApp->pDetachedWidget) != 0);
	bEnabledOK = (xgeXuiWidgetIsEnabled(pApp->pDetachedWidget) != 0);
	bFocusableOK = (xgeXuiWidgetIsFocusable(pApp->pDetachedWidget) != 0);
	bClipOK = ((xgeXuiWidgetGetFlags(pApp->pDetachedWidget) & XGE_XUI_WIDGET_CLIP) != 0);
	pApp->bFlagsOK = bHiddenOK && bVisibleOK && bEnabledOK && bFocusableOK && bClipOK;

	pStyle = xgeXuiWidgetGetStyle(pApp->pProbeWidget);
	pApp->bStyleOK = (pStyle != NULL) &&
		(pStyle->iBackgroundColor == XGE_COLOR_RGBA(54, 88, 130, 255)) &&
		FloatNear(pStyle->fRadius, 10.0f, 0.01f) &&
		(pStyle->tMargin.tLeft.iUnit == XGE_XUI_SIZE_PX) &&
		FloatNear(pStyle->tMargin.tLeft.fValue, 4.0f, 0.01f) &&
		(pStyle->tPadding.tTop.iUnit == XGE_XUI_SIZE_PX) &&
		FloatNear(pStyle->tPadding.tTop.fValue, 14.0f, 0.01f) &&
		(xgeXuiWidgetGetAnchor(pApp->pAnchoredWidget) == (XGE_XUI_ANCHOR_LEFT | XGE_XUI_ANCHOR_RIGHT | XGE_XUI_ANCHOR_BOTTOM));

	tRootRect = xgeXuiWidgetGetRect(pApp->pRootPanel);
	tRootContent = pApp->pRootPanel->tContentRect;
	tProbeRect = xgeXuiWidgetGetRect(pApp->pProbeWidget);
	tExpectedProbeRect = (xge_rect_t){
		tRootContent.fX + 26.0f + 4.0f,
		tRootContent.fY + 56.0f + 6.0f,
		220.0f - 4.0f - 8.0f,
		120.0f - 6.0f - 10.0f
	};
	tExpectedProbeContent = (xge_rect_t){
		tExpectedProbeRect.fX + 12.0f,
		tExpectedProbeRect.fY + 14.0f,
		tExpectedProbeRect.fW - 12.0f - 16.0f,
		tExpectedProbeRect.fH - 14.0f - 18.0f
	};
	pApp->bRectOK =
		RectNear(tRootRect, (xge_rect_t){ 24.0f, 24.0f, (float)xgeGetWidth() - 48.0f, (float)xgeGetHeight() - 48.0f }, 0.01f) &&
		RectNear(pApp->pProbeWidget->tLocalRect, (xge_rect_t){ 26.0f, 56.0f, 220.0f, 120.0f }, 0.01f) &&
		RectNear(tProbeRect, tExpectedProbeRect, 0.01f) &&
		RectNear(pApp->pProbeWidget->tContentRect, tExpectedProbeContent, 0.01f) &&
		(pApp->pAnchoredWidget->tRect.fW > pApp->pAnchoredWidget->tLocalRect.fW);

	xgeXuiWidgetRemove(pApp->pDetachedWidget);
	pApp->bTreeOK = pApp->bTreeOK &&
		(pApp->pDetachedWidget->pParent == NULL) &&
		(xgeXuiWidgetFindById(pRoot, 303) == NULL) &&
		(xgeXuiWidgetFindByName(pRoot, "temp-remove") == NULL);
	xgeXuiWidgetFree(pApp->pDetachedWidget);
	pApp->pDetachedWidget = NULL;

	xgeXuiUpdate(&pApp->tXui, 0.0f);
	(void)PaintXui(pApp);
	pApp->bMarkLayoutOK = (xgeXuiRefreshNeeded(&pApp->tXui) == 0);
	xgeXuiWidgetMarkLayout(pApp->pProbeWidget);
	pApp->bMarkLayoutOK = pApp->bMarkLayoutOK &&
		(xgeXuiRefreshNeeded(&pApp->tXui) != 0) &&
		((xgeXuiWidgetGetFlags(pApp->pProbeWidget) & XGE_XUI_WIDGET_DIRTY_LAYOUT) != 0);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	xgeXuiWidgetMarkPaint(pApp->pProbeWidget);
	pApp->bMarkPaintOK =
		(xgeXuiRefreshNeeded(&pApp->tXui) != 0) &&
		((xgeXuiWidgetGetFlags(pApp->pProbeWidget) & XGE_XUI_WIDGET_DIRTY_PAINT) != 0);
	iPaintCommands = PaintXui(pApp);
	pApp->bPaintOK = (iPaintCommands > 0) && (pApp->iPaintCount > 0);
	pApp->bMarkPaintOK = pApp->bMarkPaintOK &&
		((xgeXuiWidgetGetFlags(pApp->pProbeWidget) & XGE_XUI_WIDGET_DIRTY_PAINT) == 0);
	pApp->bRefreshOK = (xgeXuiRefreshNeeded(&pApp->tXui) == 0);

	xgeXuiRefreshRequest(&pApp->tXui);
	pApp->bRefreshOK = pApp->bRefreshOK && (xgeXuiRefreshNeeded(&pApp->tXui) != 0);
	iRefreshPaintOnly = PaintXui(pApp);
	pApp->bRefreshOK = pApp->bRefreshOK &&
		(iRefreshPaintOnly == 0) &&
		(xgeXuiRefreshNeeded(&pApp->tXui) == 0);

	pApp->bStaticChecksOK = pApp->bTreeOK &&
		pApp->bFindOK &&
		pApp->bRectOK &&
		pApp->bFlagsOK &&
		pApp->bStyleOK &&
		pApp->bMarkLayoutOK &&
		pApp->bMarkPaintOK &&
		pApp->bRefreshOK &&
		pApp->bPaintOK;
	UpdateStatusLabel(pApp);
	xgeXuiWidgetMarkPaint(xgeXuiRoot(&pApp->tXui));
	(void)PaintXui(pApp);
	return pApp->bStaticChecksOK ? XGE_OK : XGE_ERROR;
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
	UpdateStatusLabel(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
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

	memset(&tApp, 0, sizeof(tApp));
	memset(&tDesc, 0, sizeof(tDesc));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_WIDGET_TREE_STYLE_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}

	tDesc.iWidth = 860;
	tDesc.iHeight = 540;
	tDesc.sTitle = "XGE XUI Widget Tree Style Lab";
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
	if ( RunStaticChecks(&tApp) != XGE_OK ) {
		printf("xui-widget-tree-style-lab static-checks failed\n");
	}
	xgeRun(AppFrame, &tApp);
	printf(
		"xui-widget-tree-style-lab final-summary frames=%d tree=%d find=%d rect=%d flags=%d style=%d marks=%d/%d refresh=%d paint=%d paint_calls=%d update_calls=%d font=%d\n",
		tApp.iFrameCount,
		tApp.bTreeOK,
		tApp.bFindOK,
		tApp.bRectOK,
		tApp.bFlagsOK,
		tApp.bStyleOK,
		tApp.bMarkLayoutOK,
		tApp.bMarkPaintOK,
		tApp.bRefreshOK,
		tApp.bPaintOK,
		tApp.iPaintCount,
		tApp.iUpdateCount,
		tApp.bFontReady);
	printf("xui-widget-tree-style-lab summary frames=%d/%d\n", tApp.iFrameCount, tApp.iFrameLimit);
	xgeXuiLabelUnit(&tApp.tStatusLabel);
	if ( tApp.bFontReady != 0 ) {
		xgeFontFree(&tApp.tFont);
	}
	xgeXuiUnit(&tApp.tXui);
	xgeUnit();
	return tApp.bStaticChecksOK ? 0 : 4;
}

