#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LABEL_COUNT 17
#define SEPARATOR_COUNT 12

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootRow;
	xge_xui_widget pLeft;
	xge_xui_widget pRight;
	xge_xui_widget pRightGroup[2];
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_separator_t tSeparator[SEPARATOR_COUNT];
	int iLabelCount;
	int iSeparatorCount;
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
			printf("xui_separator font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_widget NewWidget(void)
{
	return xgeXuiWidgetCreate();
}

static xge_xui_widget AddLabel(app_state_t* pApp, xge_xui_widget pParent, const char* sText, uint32_t iColor, xge_xui_size_t tWidth, xge_xui_size_t tHeight)
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
	xgeXuiWidgetSetSize(pWidget, tWidth, tHeight);
	xgeXuiWidgetSetPaddingPx(pWidget, 6.0f, 5.0f, 6.0f, 5.0f);
	if ( xgeXuiLabelInit(&pApp->tLabel[iIndex], pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return NULL;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], iColor);
	xgeXuiLabelSetAlign(&pApp->tLabel[iIndex], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	xgeXuiWidgetAdd(pParent, pWidget);
	return pWidget;
}

static xge_xui_widget AddSeparator(app_state_t* pApp, xge_xui_widget pParent, int iOrientation, int iLineStyle, int iAlign, float fThickness, uint32_t iColor, xge_xui_size_t tWidth, xge_xui_size_t tHeight)
{
	xge_xui_widget pWidget;
	int iIndex;

	if ( pApp->iSeparatorCount >= SEPARATOR_COUNT ) {
		return NULL;
	}
	pWidget = NewWidget();
	if ( pWidget == NULL ) {
		return NULL;
	}
	iIndex = pApp->iSeparatorCount++;
	xgeXuiWidgetSetSize(pWidget, tWidth, tHeight);
	if ( xgeXuiSeparatorInit(&pApp->tSeparator[iIndex], pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return NULL;
	}
	xgeXuiSeparatorSetOrientation(&pApp->tSeparator[iIndex], iOrientation);
	xgeXuiSeparatorSetLineStyle(&pApp->tSeparator[iIndex], iLineStyle);
	xgeXuiSeparatorSetAlign(&pApp->tSeparator[iIndex], iAlign);
	xgeXuiSeparatorSetThickness(&pApp->tSeparator[iIndex], fThickness);
	xgeXuiSeparatorSetColor(&pApp->tSeparator[iIndex], iColor);
	xgeXuiWidgetAdd(pParent, pWidget);
	return pWidget;
}

static int AddRightRow(app_state_t* pApp, xge_xui_widget pParent, int iStyleA, int iStyleB, const char* sA, const char* sB, const char* sC)
{
	xge_xui_widget pRow;

	pRow = NewWidget();
	if ( pRow == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetLayout(pRow, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetSize(pRow, xgeXuiSizePercent(100.0f), xgeXuiSizePx(72.0f));
	xgeXuiWidgetSetGap(pRow, 8.0f);
	xgeXuiWidgetSetPaddingPx(pRow, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiWidgetSetBackground(pRow, XGE_COLOR_RGBA(248, 250, 253, 255));
	xgeXuiWidgetAdd(pParent, pRow);
	if ( AddLabel(pApp, pRow, sA, XGE_COLOR_RGBA(38, 48, 64, 255), xgeXuiSizeGrow(1.0f), xgeXuiSizePercent(100.0f)) == NULL ||
		AddSeparator(pApp, pRow, XGE_XUI_SEPARATOR_VERTICAL, iStyleA, XGE_XUI_ALIGN_CENTER, 2.0f, XGE_COLOR_RGBA(44, 116, 190, 220), xgeXuiSizePx(14.0f), xgeXuiSizePercent(100.0f)) == NULL ||
		AddLabel(pApp, pRow, sB, XGE_COLOR_RGBA(50, 58, 72, 255), xgeXuiSizeGrow(1.0f), xgeXuiSizePercent(100.0f)) == NULL ||
		AddSeparator(pApp, pRow, XGE_XUI_SEPARATOR_VERTICAL, iStyleB, XGE_XUI_ALIGN_CENTER, 2.0f, XGE_COLOR_RGBA(210, 132, 42, 230), xgeXuiSizePx(14.0f), xgeXuiSizePercent(100.0f)) == NULL ||
		AddLabel(pApp, pRow, sC, XGE_COLOR_RGBA(38, 48, 64, 255), xgeXuiSizeGrow(1.0f), xgeXuiSizePercent(100.0f)) == NULL ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_widget pText;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	XgeXuiDemoApplyRootPanel(pRoot);
	pApp->pRootRow = NewWidget();
	pApp->pLeft = NewWidget();
	pApp->pRight = NewWidget();
	pApp->pRightGroup[0] = NewWidget();
	pApp->pRightGroup[1] = NewWidget();
	if ( pApp->pRootRow == NULL || pApp->pLeft == NULL || pApp->pRight == NULL || pApp->pRightGroup[0] == NULL || pApp->pRightGroup[1] == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetLayout(pApp->pRootRow, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetGap(pApp->pRootRow, 14.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pRootRow, 18.0f, 18.0f, 18.0f, 18.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootRow);
	xgeXuiWidgetSetLayout(pApp->pLeft, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pApp->pLeft, xgeXuiSizeGrow(1.35f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetGap(pApp->pLeft, 8.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pLeft, 14.0f, 14.0f, 14.0f, 14.0f);
	xgeXuiWidgetSetBackground(pApp->pLeft, XGE_COLOR_RGBA(248, 250, 253, 255));
	xgeXuiWidgetSetBorder(pApp->pLeft, 1.0f, XGE_COLOR_RGBA(170, 184, 202, 255));
	xgeXuiWidgetAdd(pApp->pRootRow, pApp->pLeft);
	xgeXuiWidgetSetLayout(pApp->pRight, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pApp->pRight, xgeXuiSizeGrow(1.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetGap(pApp->pRight, 12.0f);
	xgeXuiWidgetAdd(pApp->pRootRow, pApp->pRight);
	pText = AddLabel(pApp, pApp->pLeft, "A compact page reads better when related thoughts stay together.", XGE_COLOR_RGBA(36, 42, 52, 255), xgeXuiSizePercent(100.0f), xgeXuiSizePx(54.0f));
	if ( pText == NULL ||
		AddSeparator(pApp, pApp->pLeft, XGE_XUI_SEPARATOR_HORIZONTAL, XGE_XUI_SEPARATOR_SOLID, XGE_XUI_ALIGN_CENTER, 1.0f, XGE_COLOR_RGBA(80, 102, 130, 190), xgeXuiSizePercent(100.0f), xgeXuiSizePx(14.0f)) == NULL ||
		AddLabel(pApp, pApp->pLeft, "The first break is quiet. It marks a pause without becoming a decoration.", XGE_COLOR_RGBA(48, 58, 72, 255), xgeXuiSizePercent(100.0f), xgeXuiSizePx(58.0f)) == NULL ||
		AddSeparator(pApp, pApp->pLeft, XGE_XUI_SEPARATOR_HORIZONTAL, XGE_XUI_SEPARATOR_DOT, XGE_XUI_ALIGN_CENTER, 2.0f, XGE_COLOR_RGBA(46, 124, 214, 210), xgeXuiSizePercent(100.0f), xgeXuiSizePx(16.0f)) == NULL ||
		AddLabel(pApp, pApp->pLeft, "A dotted break is lighter and works well between small notes or metadata rows.", XGE_COLOR_RGBA(48, 58, 72, 255), xgeXuiSizePercent(100.0f), xgeXuiSizePx(58.0f)) == NULL ||
		AddSeparator(pApp, pApp->pLeft, XGE_XUI_SEPARATOR_HORIZONTAL, XGE_XUI_SEPARATOR_DASH, XGE_XUI_ALIGN_START, 3.0f, XGE_COLOR_RGBA(210, 132, 42, 220), xgeXuiSizePercent(100.0f), xgeXuiSizePx(18.0f)) == NULL ||
		AddLabel(pApp, pApp->pLeft, "A dashed break is stronger. Aligning it to the top edge shows the line is independent of the widget height.", XGE_COLOR_RGBA(48, 58, 72, 255), xgeXuiSizePercent(100.0f), xgeXuiSizePx(64.0f)) == NULL ||
		AddSeparator(pApp, pApp->pLeft, XGE_XUI_SEPARATOR_HORIZONTAL, XGE_XUI_SEPARATOR_DASH_DOT, XGE_XUI_ALIGN_END, 2.0f, XGE_COLOR_RGBA(92, 116, 210, 220), xgeXuiSizePercent(100.0f), xgeXuiSizePx(18.0f)) == NULL ||
		AddLabel(pApp, pApp->pLeft, "The last break sits on the lower edge, leaving the paragraph rhythm intact.", XGE_COLOR_RGBA(36, 42, 52, 255), xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f)) == NULL ) {
		return XGE_ERROR;
	}
	for ( int i = 0; i < 2; i++ ) {
		xgeXuiWidgetSetLayout(pApp->pRightGroup[i], XGE_XUI_LAYOUT_COLUMN);
		xgeXuiWidgetSetSize(pApp->pRightGroup[i], xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
		xgeXuiWidgetSetGap(pApp->pRightGroup[i], 10.0f);
		xgeXuiWidgetSetPaddingPx(pApp->pRightGroup[i], 10.0f, 10.0f, 10.0f, 10.0f);
		xgeXuiWidgetSetBackground(pApp->pRightGroup[i], XGE_COLOR_RGBA(241, 245, 250, 255));
		xgeXuiWidgetSetBorder(pApp->pRightGroup[i], 1.0f, XGE_COLOR_RGBA(170, 184, 202, 255));
		xgeXuiWidgetAdd(pApp->pRight, pApp->pRightGroup[i]);
	}
	if ( AddRightRow(pApp, pApp->pRightGroup[0], XGE_XUI_SEPARATOR_SOLID, XGE_XUI_SEPARATOR_DOT, "Profile", "Scope", "Status") != XGE_OK ||
		AddRightRow(pApp, pApp->pRightGroup[0], XGE_XUI_SEPARATOR_SOLID, XGE_XUI_SEPARATOR_DOT, "Author", "Review", "Ready") != XGE_OK ||
		AddRightRow(pApp, pApp->pRightGroup[1], XGE_XUI_SEPARATOR_DASH, XGE_XUI_SEPARATOR_DASH_DOT, "Plan", "Build", "Verify") != XGE_OK ||
		AddRightRow(pApp, pApp->pRightGroup[1], XGE_XUI_SEPARATOR_DASH, XGE_XUI_SEPARATOR_DASH_DOT, "Input", "Output", "Trace") != XGE_OK ) {
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
	if ( fRootW < 900.0f ) {
		fRootW = 900.0f;
	}
	if ( fRootH < 560.0f ) {
		fRootH = 560.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	xgeXuiWidgetSetSize(pApp->pRootRow, xgeXuiSizePercent(100.0f), xgeXuiSizePercent(100.0f));
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	pApp->bCreateOK = (pApp->iLabelCount == LABEL_COUNT) && (pApp->iSeparatorCount == SEPARATOR_COUNT);
	pApp->bLayoutOK = (pApp->pLeft != NULL) && (pApp->pLeft->tRect.fW > 300.0f) && (pApp->pRight != NULL) && (pApp->pRight->tRect.fW > 240.0f);
	pApp->bStateOK = (pApp->tSeparator[0].iLineStyle == XGE_XUI_SEPARATOR_SOLID) && (pApp->tSeparator[1].iLineStyle == XGE_XUI_SEPARATOR_DOT) && (pApp->tSeparator[2].iLineStyle == XGE_XUI_SEPARATOR_DASH) && (pApp->tSeparator[3].iLineStyle == XGE_XUI_SEPARATOR_DASH_DOT);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
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
	for ( i = 0; i < pApp->iSeparatorCount; i++ ) {
		xgeXuiSeparatorUnit(&pApp->tSeparator[i]);
	}
	for ( i = 0; i < pApp->iLabelCount; i++ ) {
		xgeXuiLabelUnit(&pApp->tLabel[i]);
	}
	xgeXuiUnit(&pApp->tXui);
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
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_separator final-summary frames=%d create=%d layout=%d state=%d labels=%d separators=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->iLabelCount, pApp->iSeparatorCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_SEPARATOR_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 1000;
	tDesc.iHeight = 640;
	tDesc.sTitle = "XUI Separator";
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
