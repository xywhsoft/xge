#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LABEL_AREA_COUNT 4
#define LABEL_PER_AREA 9
#define LABEL_COUNT (LABEL_AREA_COUNT * LABEL_PER_AREA)

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_font_t tFontSmall;
	xge_font_t tFontLarge;
	xge_xui_widget pArea[LABEL_AREA_COUNT];
	xge_xui_widget pLabelWidget[LABEL_COUNT];
	xge_xui_label_t tLabel[LABEL_COUNT];
	int bFontReady;
	int bSmallFontReady;
	int bLargeFontReady;
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

static int LoadFontOne(xge_font pFont, float fSize)
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
		if ( xgeFontLoad(pFont, arrFonts[i], fSize) == XGE_OK ) {
			printf("xui_label font loaded: %s size=%.1f\n", arrFonts[i], fSize);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void LoadFonts(app_state_t* pApp)
{
	pApp->bFontReady = (LoadFontOne(&pApp->tFont, XGE_XUI_DEMO_FONT_SIZE) == XGE_OK);
	pApp->bSmallFontReady = (LoadFontOne(&pApp->tFontSmall, 10.0f) == XGE_OK);
	pApp->bLargeFontReady = (LoadFontOne(&pApp->tFontLarge, 18.0f) == XGE_OK);
}

static xge_font AppFont(app_state_t* pApp)
{
	return pApp->bFontReady ? &pApp->tFont : NULL;
}

static xge_font AppSmallFont(app_state_t* pApp)
{
	return pApp->bSmallFontReady ? &pApp->tFontSmall : AppFont(pApp);
}

static xge_font AppLargeFont(app_state_t* pApp)
{
	return pApp->bLargeFontReady ? &pApp->tFontLarge : AppFont(pApp);
}

static uint32_t AlignFlag(int iIndex)
{
	static const uint32_t arrFlags[LABEL_PER_AREA] = {
		XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP,
		XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_TOP,
		XGE_TEXT_ALIGN_RIGHT | XGE_TEXT_ALIGN_TOP,
		XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE,
		XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE,
		XGE_TEXT_ALIGN_RIGHT | XGE_TEXT_ALIGN_MIDDLE,
		XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_BOTTOM,
		XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_BOTTOM,
		XGE_TEXT_ALIGN_RIGHT | XGE_TEXT_ALIGN_BOTTOM
	};

	return arrFlags[iIndex % LABEL_PER_AREA];
}

static const char* AlignText(int iIndex)
{
	static const char* arrText[LABEL_PER_AREA] = {
		"Left Top",
		"Center Top",
		"Right Top",
		"Left Middle",
		"Center Middle",
		"Right Middle",
		"Left Bottom",
		"Center Bottom",
		"Right Bottom"
	};

	return arrText[iIndex % LABEL_PER_AREA];
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

static int AddLabel(app_state_t* pApp, int iArea, int iSlot, const char* sText, uint32_t iFlags, xge_font pFont, uint32_t iTextColor)
{
	int iIndex;
	xge_xui_widget pWidget;

	iIndex = iArea * LABEL_PER_AREA + iSlot;
	pWidget = NewWidget();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetMarginPx(pWidget, 4.0f, 4.0f, 4.0f, 4.0f);
	xgeXuiWidgetSetPaddingPx(pWidget, 7.0f, 6.0f, 7.0f, 6.0f);
	if ( xgeXuiLabelInit(&pApp->tLabel[iIndex], pWidget, pFont, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], iTextColor);
	xgeXuiLabelSetDisabledColor(&pApp->tLabel[iIndex], XGE_COLOR_RGBA(130, 138, 150, 255));
	xgeXuiLabelSetAlign(&pApp->tLabel[iIndex], iFlags);
	xgeXuiWidgetAdd(pApp->pArea[iArea], pWidget);
	pApp->pLabelWidget[iIndex] = pWidget;
	return XGE_OK;
}

static void StyleArea(xge_xui_widget pWidget, uint32_t iBackground)
{
	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_GRID);
	xgeXuiWidgetSetGrid(pWidget, 3, 72.0f, 8.0f, 8.0f);
	xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePercent(100.0f));
	xgeXuiWidgetSetPaddingPx(pWidget, 10.0f, 10.0f, 10.0f, 10.0f);
	xgeXuiWidgetSetMarginPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetBackground(pWidget, iBackground);
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(170, 184, 202, 255));
	xgeXuiWidgetSetRadius(pWidget, 4.0f);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;
	int i;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = AppFont(pApp);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	XgeXuiDemoApplyRootPanel(pRoot);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_GRID);
	xgeXuiWidgetSetGrid(pRoot, 2, 300.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetPaddingPx(pRoot, 18.0f, 18.0f, 18.0f, 18.0f);

	for ( i = 0; i < LABEL_AREA_COUNT; i++ ) {
		pApp->pArea[i] = NewWidget();
		if ( pApp->pArea[i] == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		StyleArea(pApp->pArea[i], (i == 2) ? XGE_COLOR_RGBA(231, 238, 247, 255) : XGE_COLOR_RGBA(248, 250, 253, 255));
		xgeXuiWidgetAdd(pRoot, pApp->pArea[i]);
	}

	for ( i = 0; i < LABEL_PER_AREA; i++ ) {
		if ( AddLabel(pApp, 0, i, AlignText(i), AlignFlag(i), pFont, XGE_COLOR_RGBA(36, 42, 52, 255)) != XGE_OK ) {
			return XGE_ERROR;
		}
	}

	for ( i = 0; i < LABEL_PER_AREA; i++ ) {
		if ( AddLabel(pApp, 1, i, AlignText(i), AlignFlag(i), pFont, XGE_COLOR_RGBA(32, 62, 96, 255)) != XGE_OK ) {
			return XGE_ERROR;
		}
		xgeXuiWidgetSetBorder(pApp->pLabelWidget[LABEL_PER_AREA + i], 1.0f, XGE_COLOR_RGBA(74, 126, 184, 255));
	}

	for ( i = 0; i < LABEL_PER_AREA; i++ ) {
		static const uint32_t arrBg[LABEL_PER_AREA] = {
			XGE_COLOR_RGBA(234, 242, 255, 255), XGE_COLOR_RGBA(233, 248, 239, 255), XGE_COLOR_RGBA(255, 243, 216, 255),
			XGE_COLOR_RGBA(243, 234, 251, 255), XGE_COLOR_RGBA(255, 236, 236, 255), XGE_COLOR_RGBA(232, 246, 250, 255),
			XGE_COLOR_RGBA(244, 246, 250, 255), XGE_COLOR_RGBA(239, 242, 231, 255), XGE_COLOR_RGBA(248, 238, 247, 255)
		};
		if ( AddLabel(pApp, 2, i, AlignText(i), AlignFlag(i), pFont, XGE_COLOR_RGBA(34, 46, 62, 255)) != XGE_OK ) {
			return XGE_ERROR;
		}
		xgeXuiWidgetSetBackground(pApp->pLabelWidget[(2 * LABEL_PER_AREA) + i], arrBg[i]);
	}

	if ( AddLabel(pApp, 3, 0, "Small font", XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE, AppSmallFont(pApp), XGE_COLOR_RGBA(34, 42, 56, 255)) != XGE_OK ||
		AddLabel(pApp, 3, 1, "Large font", XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE, AppLargeFont(pApp), XGE_COLOR_RGBA(34, 42, 56, 255)) != XGE_OK ||
		AddLabel(pApp, 3, 2, "Disabled", XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE, pFont, XGE_COLOR_RGBA(34, 42, 56, 255)) != XGE_OK ||
		AddLabel(pApp, 3, 3, "Underline", XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE, pFont, XGE_COLOR_RGBA(26, 96, 176, 255)) != XGE_OK ||
		AddLabel(pApp, 3, 4, "Border + fill", XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE, pFont, XGE_COLOR_RGBA(38, 58, 82, 255)) != XGE_OK ||
		AddLabel(pApp, 3, 5, "Direct draw", XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE, pFont, XGE_COLOR_RGBA(96, 56, 18, 255)) != XGE_OK ||
		AddLabel(pApp, 3, 6, "Two lines\ncentered", XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE, pFont, XGE_COLOR_RGBA(36, 42, 52, 255)) != XGE_OK ||
		AddLabel(pApp, 3, 7, "Padded text", XGE_TEXT_ALIGN_RIGHT | XGE_TEXT_ALIGN_BOTTOM, pFont, XGE_COLOR_RGBA(36, 42, 52, 255)) != XGE_OK ||
		AddLabel(pApp, 3, 8, "Accent text", XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP, pFont, XGE_COLOR_RGBA(20, 112, 210, 255)) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetEnabled(pApp->pLabelWidget[(3 * LABEL_PER_AREA) + 2], 0);
	xgeXuiLabelSetUnderline(&pApp->tLabel[(3 * LABEL_PER_AREA) + 3], 1);
	xgeXuiWidgetSetBackground(pApp->pLabelWidget[(3 * LABEL_PER_AREA) + 4], XGE_COLOR_RGBA(229, 240, 255, 255));
	xgeXuiWidgetSetBorder(pApp->pLabelWidget[(3 * LABEL_PER_AREA) + 4], 1.0f, XGE_COLOR_RGBA(42, 120, 210, 255));
	xgeXuiWidgetSetBackground(pApp->pLabelWidget[(3 * LABEL_PER_AREA) + 7], XGE_COLOR_RGBA(248, 244, 236, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pLabelWidget[(3 * LABEL_PER_AREA) + 7], 12.0f, 10.0f, 12.0f, 10.0f);
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
	if ( fAreaH < 240.0f ) {
		fAreaH = 240.0f;
	}
	xgeXuiWidgetSetGrid(pRoot, 2, fAreaH, 12.0f, 12.0f);
	fCellH = (fAreaH - 20.0f - 16.0f) / 3.0f;
	if ( fCellH < 54.0f ) {
		fCellH = 54.0f;
	}
	for ( i = 0; i < LABEL_AREA_COUNT; i++ ) {
		xgeXuiWidgetSetGrid(pApp->pArea[i], 3, fCellH, 8.0f, 8.0f);
	}
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	int i;

	pApp->bCreateOK = 1;
	for ( i = 0; i < LABEL_AREA_COUNT; i++ ) {
		if ( pApp->pArea[i] == NULL ) {
			pApp->bCreateOK = 0;
		}
	}
	for ( i = 0; i < LABEL_COUNT; i++ ) {
		if ( (pApp->pLabelWidget[i] == NULL) || (pApp->tLabel[i].pWidget != pApp->pLabelWidget[i]) ) {
			pApp->bCreateOK = 0;
		}
	}
	pApp->bLayoutOK =
		(pApp->pArea[0] != NULL) &&
		(pApp->pArea[1] != NULL) &&
		(pApp->pArea[2] != NULL) &&
		(pApp->pArea[3] != NULL) &&
		(pApp->pArea[0]->tRect.fW > 200.0f) &&
		(pApp->pArea[3]->tRect.fH > 200.0f);
	pApp->bStateOK =
		(pApp->pLabelWidget[LABEL_PER_AREA]->tStyle.fBorderWidth > 0.0f) &&
		(XGE_COLOR_GET_A(pApp->pLabelWidget[2 * LABEL_PER_AREA]->tStyle.iBackgroundColor) != 0) &&
		(xgeXuiWidgetIsEnabled(pApp->pLabelWidget[(3 * LABEL_PER_AREA) + 2]) == 0) &&
		(pApp->tLabel[(3 * LABEL_PER_AREA) + 3].bUnderline != 0);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFonts(pApp);
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
	for ( i = 0; i < LABEL_COUNT; i++ ) {
		xgeXuiLabelUnit(&pApp->tLabel[i]);
	}
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bLargeFontReady ) {
		xgeFontFree(&pApp->tFontLarge);
	}
	if ( pApp->bSmallFontReady ) {
		xgeFontFree(&pApp->tFontSmall);
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
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui_label final-summary frames=%d create=%d layout=%d state=%d labels=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			LABEL_COUNT);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_LABEL_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 960;
	tDesc.iHeight = 680;
	tDesc.sTitle = "XUI Label";
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
