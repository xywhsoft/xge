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
	xge_xui_widget pDarkSwatchWidget;
	xge_xui_widget pLightSwatchWidget;
	xge_xui_widget pDipProbeWidget;
	xge_xui_widget pThemeButtonWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_button_t tThemeButton;
	xge_xui_theme_t tDefaultTheme;
	xge_xui_theme_t tDarkTheme;
	xge_xui_theme_t tLightTheme;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bDefaultThemeOK;
	int bThemeSetGetOK;
	int bDipScaleOK;
	int bThemeStyleOK;
	int bLightDarkOK;
	int bButtonThemeOK;
	float fDipWidthBefore;
	float fDipHeightBefore;
	float fDipWidthAfter;
	float fDipHeightAfter;
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
			printf("xui-theme-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-theme-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void BuildThemes(app_state_t* pApp)
{
	pApp->tDefaultTheme = *xgeXuiGetTheme(&pApp->tXui);

	pApp->tDarkTheme = pApp->tDefaultTheme;
	pApp->tDarkTheme.pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	pApp->tDarkTheme.iTextColor = XGE_COLOR_RGBA(238, 242, 248, 255);
	pApp->tDarkTheme.iBackgroundColor = XGE_COLOR_RGBA(24, 30, 42, 255);
	pApp->tDarkTheme.iPanelColor = XGE_COLOR_RGBA(34, 42, 58, 255);
	pApp->tDarkTheme.iBorderColor = XGE_COLOR_RGBA(88, 104, 132, 255);
	pApp->tDarkTheme.iAccentColor = XGE_COLOR_RGBA(92, 164, 255, 255);
	pApp->tDarkTheme.iSelectionColor = XGE_COLOR_RGBA(92, 164, 255, 128);
	pApp->tDarkTheme.iStateNormal = XGE_COLOR_RGBA(52, 74, 104, 255);
	pApp->tDarkTheme.iStateHover = XGE_COLOR_RGBA(70, 92, 126, 255);
	pApp->tDarkTheme.iStateActive = XGE_COLOR_RGBA(40, 58, 82, 255);
	pApp->tDarkTheme.iStateFocus = XGE_COLOR_RGBA(58, 86, 122, 255);
	pApp->tDarkTheme.iStateDisabled = XGE_COLOR_RGBA(68, 72, 78, 180);
	pApp->tDarkTheme.fRadius = 8.0f;
	pApp->tDarkTheme.fPadding = 10.0f;
	pApp->tDarkTheme.fSpacing = 12.0f;
	pApp->tDarkTheme.fBorderWidth = 2.0f;

	pApp->tLightTheme = pApp->tDefaultTheme;
	pApp->tLightTheme.pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	pApp->tLightTheme.iTextColor = XGE_COLOR_RGBA(38, 48, 62, 255);
	pApp->tLightTheme.iBackgroundColor = XGE_COLOR_RGBA(242, 245, 250, 255);
	pApp->tLightTheme.iPanelColor = XGE_COLOR_RGBA(224, 232, 242, 255);
	pApp->tLightTheme.iBorderColor = XGE_COLOR_RGBA(148, 160, 176, 255);
	pApp->tLightTheme.iAccentColor = XGE_COLOR_RGBA(90, 150, 104, 255);
	pApp->tLightTheme.iSelectionColor = XGE_COLOR_RGBA(90, 150, 104, 112);
	pApp->tLightTheme.iStateNormal = XGE_COLOR_RGBA(204, 214, 228, 255);
	pApp->tLightTheme.iStateHover = XGE_COLOR_RGBA(214, 224, 236, 255);
	pApp->tLightTheme.iStateActive = XGE_COLOR_RGBA(186, 198, 214, 255);
	pApp->tLightTheme.iStateFocus = XGE_COLOR_RGBA(172, 194, 220, 255);
	pApp->tLightTheme.iStateDisabled = XGE_COLOR_RGBA(180, 180, 180, 180);
	pApp->tLightTheme.fRadius = 6.0f;
	pApp->tLightTheme.fPadding = 6.0f;
	pApp->tLightTheme.fSpacing = 8.0f;
	pApp->tLightTheme.fBorderWidth = 1.0f;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	tRoot.fX = 20.0f;
	tRoot.fY = 20.0f;
	tRoot.fW = (float)iWidth - 40.0f;
	tRoot.fH = (float)iHeight - 40.0f;
	if ( tRoot.fW < 420.0f ) {
		tRoot.fW = 420.0f;
	}
	if ( tRoot.fH < 220.0f ) {
		tRoot.fH = 220.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pDarkSwatchWidget, (xge_rect_t){ 24.0f, 60.0f, 140.0f, 92.0f });
	xgeXuiWidgetSetRect(pApp->pLightSwatchWidget, (xge_rect_t){ 184.0f, 60.0f, 140.0f, 92.0f });
	xgeXuiWidgetSetRect(pApp->pDipProbeWidget, (xge_rect_t){ 348.0f, 68.0f, 0.0f, 0.0f });
	xgeXuiWidgetSetRect(pApp->pThemeButtonWidget, (xge_rect_t){ 24.0f, 172.0f, 156.0f, 42.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"default=%d setget=%d dip=%d style=%d lightdark=%d button=%d dip=%.0fx%.0f->%.0fx%.0f scale=%.1f",
		pApp->bDefaultThemeOK,
		pApp->bThemeSetGetOK,
		pApp->bDipScaleOK,
		pApp->bThemeStyleOK,
		pApp->bLightDarkOK,
		pApp->bButtonThemeOK,
		pApp->fDipWidthBefore,
		pApp->fDipHeightBefore,
		pApp->fDipWidthAfter,
		pApp->fDipHeightAfter,
		xgeXuiGetDipScale(&pApp->tXui));
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_style_t tStyle;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiSetTheme(&pApp->tXui, &pApp->tDarkTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pDarkSwatchWidget = xgeXuiWidgetCreate();
	pApp->pLightSwatchWidget = xgeXuiWidgetCreate();
	pApp->pDipProbeWidget = xgeXuiWidgetCreate();
	pApp->pThemeButtonWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pDarkSwatchWidget == NULL) || (pApp->pLightSwatchWidget == NULL) || (pApp->pDipProbeWidget == NULL) || (pApp->pThemeButtonWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 42, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 62, 84, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui theme lab");
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);

	xgeXuiStyleFromTheme(&tStyle, &pApp->tDarkTheme);
	xgeXuiWidgetSetStyle(pApp->pDarkSwatchWidget, &tStyle);
	xgeXuiWidgetSetBackground(pApp->pDarkSwatchWidget, pApp->tDarkTheme.iPanelColor);
	xgeXuiWidgetSetRadius(pApp->pDarkSwatchWidget, pApp->tDarkTheme.fRadius);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pDarkSwatchWidget);

	xgeXuiStyleFromTheme(&tStyle, &pApp->tLightTheme);
	xgeXuiWidgetSetStyle(pApp->pLightSwatchWidget, &tStyle);
	xgeXuiWidgetSetBackground(pApp->pLightSwatchWidget, pApp->tLightTheme.iPanelColor);
	xgeXuiWidgetSetRadius(pApp->pLightSwatchWidget, pApp->tLightTheme.fRadius);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pLightSwatchWidget);

	xgeXuiWidgetSetBackground(pApp->pDipProbeWidget, XGE_COLOR_RGBA(244, 174, 94, 255));
	xgeXuiWidgetSetRadius(pApp->pDipProbeWidget, 6.0f);
	xgeXuiWidgetSetSize(pApp->pDipProbeWidget, xgeXuiSizeDip(24.0f), xgeXuiSizeDip(12.0f));
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pDipProbeWidget);

	xgeXuiButtonInit(&pApp->tThemeButton, &pApp->tXui, pApp->pThemeButtonWidget);
	xgeXuiButtonSetText(&pApp->tThemeButton, pFont, "Dark Theme Button");
	xgeXuiButtonSetTextColor(&pApp->tThemeButton, pApp->tDarkTheme.iTextColor);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pThemeButtonWidget);
	return XGE_OK;
}

static int RunStaticChecks(app_state_t* pApp)
{
	const xge_xui_theme_t* pTheme;
	xge_xui_style_t tStyleDark;
	xge_xui_style_t tStyleLight;

	pTheme = xgeXuiGetTheme(&pApp->tXui);
	pApp->bDefaultThemeOK =
		(xgeXuiGetDipScale(&pApp->tXui) == 1.0f) &&
		(pTheme->iTextColor == XGE_COLOR_RGBA(255, 255, 255, 255)) &&
		(pTheme->iPanelColor == XGE_COLOR_RGBA(32, 38, 46, 255)) &&
		FloatNear(pTheme->fRadius, 4.0f, 0.001f) &&
		FloatNear(pTheme->fPadding, 4.0f, 0.001f) &&
		FloatNear(pTheme->fSpacing, 4.0f, 0.001f) &&
		FloatNear(pTheme->fBorderWidth, 1.0f, 0.001f);

	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	pTheme = xgeXuiGetTheme(&pApp->tXui);
	pApp->bThemeSetGetOK =
		(xgeXuiRefreshNeeded(&pApp->tXui) != 0) &&
		(pTheme->pFont == pApp->tDarkTheme.pFont) &&
		(pTheme->iPanelColor == pApp->tDarkTheme.iPanelColor) &&
		(pTheme->iAccentColor == pApp->tDarkTheme.iAccentColor) &&
		FloatNear(pTheme->fPadding, pApp->tDarkTheme.fPadding, 0.001f) &&
		FloatNear(pTheme->fSpacing, pApp->tDarkTheme.fSpacing, 0.001f);
	xgeXuiRefreshClear(&pApp->tXui);

	xgeXuiStyleFromTheme(&tStyleDark, pTheme);
	pApp->bThemeStyleOK =
		(tStyleDark.iBackgroundColor == pApp->tDarkTheme.iPanelColor) &&
		FloatNear(tStyleDark.fRadius, pApp->tDarkTheme.fRadius, 0.001f) &&
		FloatNear(tStyleDark.tPadding.tLeft.fValue, pApp->tDarkTheme.fPadding, 0.001f) &&
		(tStyleDark.tPadding.tLeft.iUnit == XGE_XUI_SIZE_PX) &&
		FloatNear(tStyleDark.fGap, pApp->tDarkTheme.fSpacing, 0.001f);

	pApp->bButtonThemeOK =
		(pApp->tThemeButton.pFont == pApp->tDarkTheme.pFont) &&
		(pApp->tThemeButton.iColorNormal == pApp->tDarkTheme.iStateNormal) &&
		(pApp->tThemeButton.iColorHover == pApp->tDarkTheme.iStateHover) &&
		(pApp->tThemeButton.iColorActive == pApp->tDarkTheme.iStateActive) &&
		(pApp->tThemeButton.iColorFocus == pApp->tDarkTheme.iStateFocus) &&
		(pApp->tThemeButton.iColorDisabled == pApp->tDarkTheme.iStateDisabled);

	pApp->fDipWidthBefore = pApp->pDipProbeWidget->tRect.fW;
	pApp->fDipHeightBefore = pApp->pDipProbeWidget->tRect.fH;
	xgeXuiSetDipScale(&pApp->tXui, 2.0f);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->fDipWidthAfter = pApp->pDipProbeWidget->tRect.fW;
	pApp->fDipHeightAfter = pApp->pDipProbeWidget->tRect.fH;
	pApp->bDipScaleOK =
		FloatNear(xgeXuiGetDipScale(&pApp->tXui), 2.0f, 0.001f) &&
		FloatNear(pApp->fDipWidthBefore, 24.0f, 0.01f) &&
		FloatNear(pApp->fDipHeightBefore, 12.0f, 0.01f) &&
		FloatNear(pApp->fDipWidthAfter, 48.0f, 0.01f) &&
		FloatNear(pApp->fDipHeightAfter, 24.0f, 0.01f);

	xgeXuiSetTheme(&pApp->tXui, &pApp->tLightTheme);
	pTheme = xgeXuiGetTheme(&pApp->tXui);
	xgeXuiStyleFromTheme(&tStyleLight, pTheme);
	pApp->bLightDarkOK =
		(xgeXuiRefreshNeeded(&pApp->tXui) != 0) &&
		(pTheme->iPanelColor == pApp->tLightTheme.iPanelColor) &&
		(pTheme->iTextColor == pApp->tLightTheme.iTextColor) &&
		(tStyleLight.iBackgroundColor == pApp->tLightTheme.iPanelColor) &&
		FloatNear(tStyleLight.fRadius, pApp->tLightTheme.fRadius, 0.001f) &&
		(pApp->tLightTheme.iPanelColor != pApp->tDarkTheme.iPanelColor) &&
		(pApp->tLightTheme.iTextColor != pApp->tDarkTheme.iTextColor);
	xgeXuiRefreshClear(&pApp->tXui);

	xgeXuiWidgetSetBackground(pApp->pRootPanel, pApp->tLightTheme.iBackgroundColor);
	xgeXuiWidgetSetBackground(pApp->pStatusWidget, pApp->tLightTheme.iPanelColor);
	xgeXuiLabelSetColor(&pApp->tStatusLabel, pApp->tLightTheme.iTextColor);
	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	(void)LoadFont(pApp);
	BuildThemes(pApp);
	return RunStaticChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiButtonUnit(&pApp->tThemeButton);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());
	xgeBegin();
	xgeClear(pApp->tLightTheme.iBackgroundColor);
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-theme-lab final-summary frames=%d default=%d setget=%d dip=%d style=%d lightdark=%d button=%d dip=%.0fx%.0f->%.0fx%.0f scale=%.1f dark(panel=%u accent=%u radius=%.1f pad=%.1f) light(panel=%u accent=%u radius=%.1f pad=%.1f)\n",
			pApp->iFrameCount,
			pApp->bDefaultThemeOK,
			pApp->bThemeSetGetOK,
			pApp->bDipScaleOK,
			pApp->bThemeStyleOK,
			pApp->bLightDarkOK,
			pApp->bButtonThemeOK,
			pApp->fDipWidthBefore,
			pApp->fDipHeightBefore,
			pApp->fDipWidthAfter,
			pApp->fDipHeightAfter,
			xgeXuiGetDipScale(&pApp->tXui),
			pApp->tDarkTheme.iPanelColor,
			pApp->tDarkTheme.iAccentColor,
			pApp->tDarkTheme.fRadius,
			pApp->tDarkTheme.fPadding,
			pApp->tLightTheme.iPanelColor,
			pApp->tLightTheme.iAccentColor,
			pApp->tLightTheme.fRadius,
			pApp->tLightTheme.fPadding);
		printf("xui-theme-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int iFrameLimit;
	int i;

	memset(&tDesc, 0, sizeof(tDesc));
	iFrameLimit = ArgInt(getenv("XGE_XUI_THEME_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			iFrameLimit = ArgInt(argv[++i], iFrameLimit);
		}
	}

	tDesc.iWidth = 520;
	tDesc.iHeight = 250;
	tDesc.sTitle = "XGE XUI Theme Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		printf("xui-theme-lab init failed\n");
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	AppUnit(&tApp);
	xgeUnit();
	return 0;
}

