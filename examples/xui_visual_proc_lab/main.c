#include "../../xge.h"
#include "../xui_demo_style.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_texture_t tIconTexture;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pPanelWidget;
	xge_xui_widget pLabelWidget;
	xge_xui_widget pImageWidget;
	xge_xui_widget pSeparatorWidget;
	xge_xui_widget pButtonWidget;
	xge_xui_widget pIconButtonWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tLabel;
	xge_xui_image_t tImage;
	xge_xui_button_t tButton;
	xge_xui_icon_button_t tIconButton;
	xge_xui_panel_t tPanel;
	xge_xui_separator_t tSeparator;
	int bFontReady;
	int bTextureReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iButtonCallbackCount;
	int iIconButtonCallbackCount;
	int bLabelOK;
	int bImageOK;
	int bButtonOK;
	int bIconButtonOK;
	int bPanelOK;
	int bSeparatorOK;
	int bDirectPaintOK;
	int bScriptRan;
	xge_vec2_t tLabelMeasure;
	xge_vec2_t tLabelMeasureProc;
	xge_vec2_t tImageMeasureProc;
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

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static xge_vec2_t WidgetCenter(xge_xui_widget pWidget)
{
	xge_vec2_t tCenter;

	tCenter.fX = pWidget->tRect.fX + (pWidget->tRect.fW * 0.5f);
	tCenter.fY = pWidget->tRect.fY + (pWidget->tRect.fH * 0.5f);
	return tCenter;
}

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-visual-proc-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-visual-proc-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static int CreateIconTexture(app_state_t* pApp)
{
	unsigned char arrPixels[8 * 8 * 4];
	int x;
	int y;
	int i;

	memset(arrPixels, 0, sizeof(arrPixels));
	for ( y = 0; y < 8; y++ ) {
		for ( x = 0; x < 8; x++ ) {
			i = (y * 8 + x) * 4;
			arrPixels[i + 0] = (unsigned char)(24 + x * 24);
			arrPixels[i + 1] = (unsigned char)(112 + y * 14);
			arrPixels[i + 2] = (unsigned char)(200 - x * 10);
			arrPixels[i + 3] = ((x == y) || (x == (7 - y)) || (x == 3) || (x == 4) || (y == 3) || (y == 4)) ? 255 : 96;
		}
	}
	if ( xgeTextureCreateRGBA(&pApp->tIconTexture, 8, 8, arrPixels) != XGE_OK ) {
		printf("xui-visual-proc-lab texture create failed\n");
		return XGE_ERROR_RESOURCE_FAILED;
	}
	pApp->bTextureReady = 1;
	return XGE_OK;
}

static void ButtonClick(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iButtonCallbackCount++;
	}
}

static void IconButtonClick(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iIconButtonCallbackCount++;
	}
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
	if ( tRoot.fW < 560.0f ) {
		tRoot.fW = 560.0f;
	}
	if ( tRoot.fH < 320.0f ) {
		tRoot.fH = 320.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 38.0f });
	xgeXuiWidgetSetRect(pApp->pPanelWidget, (xge_rect_t){ 18.0f, 58.0f, 248.0f, 178.0f });
	xgeXuiWidgetSetRect(pApp->pLabelWidget, (xge_rect_t){ 16.0f, 42.0f, 216.0f, 28.0f });
	xgeXuiWidgetSetRect(pApp->pImageWidget, (xge_rect_t){ 16.0f, 86.0f, 80.0f, 80.0f });
	xgeXuiWidgetSetRect(pApp->pSeparatorWidget, (xge_rect_t){ 292.0f, 64.0f, 8.0f, 166.0f });
	xgeXuiWidgetSetRect(pApp->pButtonWidget, (xge_rect_t){ 324.0f, 84.0f, 176.0f, 46.0f });
	xgeXuiWidgetSetRect(pApp->pIconButtonWidget, (xge_rect_t){ 382.0f, 148.0f, 60.0f, 60.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"label=%d image=%d button=%d icon=%d panel=%d separator=%d paint=%d clicks=%d/%d cb=%d/%d",
		pApp->bLabelOK,
		pApp->bImageOK,
		pApp->bButtonOK,
		pApp->bIconButtonOK,
		pApp->bPanelOK,
		pApp->bSeparatorOK,
		pApp->bDirectPaintOK,
		pApp->tButton.iClickCount,
		pApp->tIconButton.iClickCount,
		pApp->iButtonCallbackCount,
		pApp->iIconButtonCallbackCount);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;
	xge_font pFont;
	xge_rect_t tSrc;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(240, 244, 250, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(18, 22, 30, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(34, 44, 62, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(86, 98, 122, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(96, 214, 144, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(52, 96, 164, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(72, 122, 196, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(42, 78, 138, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(86, 104, 132, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(84, 88, 94, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pPanelWidget = xgeXuiWidgetCreate();
	pApp->pLabelWidget = xgeXuiWidgetCreate();
	pApp->pImageWidget = xgeXuiWidgetCreate();
	pApp->pSeparatorWidget = xgeXuiWidgetCreate();
	pApp->pButtonWidget = xgeXuiWidgetCreate();
	pApp->pIconButtonWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pPanelWidget == NULL) ||
	     (pApp->pLabelWidget == NULL) || (pApp->pImageWidget == NULL) || (pApp->pSeparatorWidget == NULL) ||
	     (pApp->pButtonWidget == NULL) || (pApp->pIconButtonWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPanelWidget);
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pLabelWidget);
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pImageWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pSeparatorWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pButtonWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pIconButtonWidget);

	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui visual proc lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiPanelInit(&pApp->tPanel, pApp->pPanelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "Visual Proc Surface");
	xgeXuiPanelSetBackground(&pApp->tPanel, XGE_COLOR_RGBA(34, 44, 62, 255));
	xgeXuiPanelSetTitleColor(&pApp->tPanel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiPanelSetTitleAlign(&pApp->tPanel, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_TOP);
	xgeXuiPanelSetClip(&pApp->tPanel, 1);

	if ( xgeXuiLabelInit(&pApp->tLabel, pApp->pLabelWidget, pFont, "Label/Image/Panel proc bindings") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tLabel, XGE_COLOR_RGBA(232, 238, 244, 255));
	xgeXuiLabelSetAlign(&pApp->tLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);

	if ( xgeXuiImageInit(&pApp->tImage, pApp->pImageWidget, pApp->bTextureReady ? &pApp->tIconTexture : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	tSrc = (xge_rect_t){ 1.0f, 1.0f, 6.0f, 6.0f };
	xgeXuiImageSetSource(&pApp->tImage, tSrc);
	xgeXuiImageSetColor(&pApp->tImage, XGE_COLOR_RGBA(112, 220, 176, 255));
	xgeXuiImageSetMode(&pApp->tImage, XGE_XUI_IMAGE_CENTER);

	if ( xgeXuiSeparatorInit(&pApp->tSeparator, pApp->pSeparatorWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiSeparatorSetColor(&pApp->tSeparator, XGE_COLOR_RGBA(244, 178, 92, 255));
	xgeXuiSeparatorSetThickness(&pApp->tSeparator, 3.0f);
	xgeXuiSeparatorSetOrientation(&pApp->tSeparator, XGE_XUI_SEPARATOR_VERTICAL);

	if ( xgeXuiButtonInit(&pApp->tButton, &pApp->tXui, pApp->pButtonWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(&pApp->tButton, pFont, "Button Proc");
	xgeXuiButtonSetClick(&pApp->tButton, ButtonClick, pApp);
	xgeXuiButtonSetTextColor(&pApp->tButton, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiIconButtonInit(&pApp->tIconButton, &pApp->tXui, pApp->pIconButtonWidget, pApp->bTextureReady ? &pApp->tIconTexture : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiIconButtonSetClick(&pApp->tIconButton, IconButtonClick, pApp);
	xgeXuiIconButtonSetSource(&pApp->tIconButton, tSrc);
	xgeXuiIconButtonSetIconColor(&pApp->tIconButton, XGE_COLOR_RGBA(255, 214, 96, 255));
	xgeXuiIconButtonSetMode(&pApp->tIconButton, XGE_XUI_IMAGE_STRETCH);
	xgeXuiIconButtonSetColors(
		&pApp->tIconButton,
		XGE_COLOR_RGBA(54, 98, 166, 255),
		XGE_COLOR_RGBA(72, 128, 204, 255),
		XGE_COLOR_RGBA(44, 80, 136, 255),
		XGE_COLOR_RGBA(92, 108, 138, 255),
		XGE_COLOR_RGBA(88, 90, 96, 180));

	return XGE_OK;
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tButtonCenter;
	xge_vec2_t tIconCenter;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	pApp->tLabelMeasure = xgeXuiLabelMeasure(&pApp->tLabel);
	pApp->tLabelMeasureProc = xgeXuiLabelMeasureProc(pApp->pLabelWidget, &pApp->tLabel);
	pApp->bLabelOK =
		(pApp->pLabelWidget->procMeasure == xgeXuiLabelMeasureProc) &&
		(pApp->pLabelWidget->procPaint == xgeXuiLabelPaintProc) &&
		(pApp->tLabelMeasure.fX > 0.0f) &&
		(pApp->tLabelMeasure.fY > 0.0f) &&
		FloatNear(pApp->tLabelMeasure.fX, pApp->tLabelMeasureProc.fX, 0.01f) &&
		FloatNear(pApp->tLabelMeasure.fY, pApp->tLabelMeasureProc.fY, 0.01f);

	pApp->tImageMeasureProc = xgeXuiImageMeasureProc(pApp->pImageWidget, &pApp->tImage);
	pApp->bImageOK =
		(pApp->pImageWidget->procMeasure == xgeXuiImageMeasureProc) &&
		(pApp->pImageWidget->procPaint == xgeXuiImagePaintProc) &&
		FloatNear(pApp->tImageMeasureProc.fX, 6.0f, 0.01f) &&
		FloatNear(pApp->tImageMeasureProc.fY, 6.0f, 0.01f);

	pApp->bPanelOK =
		(pApp->pPanelWidget->procPaint == xgeXuiPanelPaintProc) &&
		(pApp->pPanelWidget->pUser == &pApp->tPanel) &&
		(pApp->tPanel.bClip != 0);

	pApp->bSeparatorOK =
		(pApp->pSeparatorWidget->procPaint == xgeXuiSeparatorPaintProc) &&
		(pApp->tSeparator.iOrientation == XGE_XUI_SEPARATOR_VERTICAL) &&
		FloatNear(pApp->tSeparator.fThickness, 3.0f, 0.01f);

	tButtonCenter = WidgetCenter(pApp->pButtonWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tButtonCenter.fX, tButtonCenter.fY);
	(void)xgeXuiButtonEventProc(pApp->pButtonWidget, &tEvent, &pApp->tButton);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tButtonCenter.fX, tButtonCenter.fY);
	(void)xgeXuiButtonEventProc(pApp->pButtonWidget, &tEvent, &pApp->tButton);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tButtonCenter.fX, tButtonCenter.fY);
	(void)xgeXuiButtonEventProc(pApp->pButtonWidget, &tEvent, &pApp->tButton);
	xgeXuiSetFocus(&pApp->tXui, pApp->pButtonWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	(void)xgeXuiButtonEventProc(pApp->pButtonWidget, &tEvent, &pApp->tButton);
	pApp->bButtonOK =
		(pApp->pButtonWidget->procEvent == xgeXuiButtonEventProc) &&
		(pApp->pButtonWidget->procPaint == xgeXuiButtonPaintProc) &&
		(pApp->tButton.iClickCount >= 2) &&
		(pApp->iButtonCallbackCount >= 2);

	tIconCenter = WidgetCenter(pApp->pIconButtonWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tIconCenter.fX, tIconCenter.fY);
	(void)xgeXuiIconButtonEventProc(pApp->pIconButtonWidget, &tEvent, &pApp->tIconButton);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tIconCenter.fX, tIconCenter.fY);
	(void)xgeXuiIconButtonEventProc(pApp->pIconButtonWidget, &tEvent, &pApp->tIconButton);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tIconCenter.fX, tIconCenter.fY);
	(void)xgeXuiIconButtonEventProc(pApp->pIconButtonWidget, &tEvent, &pApp->tIconButton);
	xgeXuiSetFocus(&pApp->tXui, pApp->pIconButtonWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	(void)xgeXuiIconButtonEventProc(pApp->pIconButtonWidget, &tEvent, &pApp->tIconButton);
	pApp->bIconButtonOK =
		(pApp->pIconButtonWidget->procEvent == xgeXuiIconButtonEventProc) &&
		(pApp->pIconButtonWidget->procPaint == xgeXuiIconButtonPaintProc) &&
		(pApp->tIconButton.iClickCount >= 2) &&
		(pApp->iIconButtonCallbackCount >= 2);

	return XGE_OK;
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiIconButtonUnit(&pApp->tIconButton);
	xgeXuiButtonUnit(&pApp->tButton);
	xgeXuiSeparatorUnit(&pApp->tSeparator);
	xgeXuiImageUnit(&pApp->tImage);
	xgeXuiLabelUnit(&pApp->tLabel);
	xgeXuiPanelUnit(&pApp->tPanel);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bTextureReady ) {
		xgeTextureFree(&pApp->tIconTexture);
	}
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	memset(pApp, 0, sizeof(*pApp));
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( LoadFont(pApp) != XGE_OK ) {
		AppUnit(pApp);
		return XGE_ERROR;
	}
	if ( CreateIconTexture(pApp) != XGE_OK ) {
		AppUnit(pApp);
		return XGE_ERROR;
	}
	if ( CreateUI(pApp) != XGE_OK ) {
		AppUnit(pApp);
		return XGE_ERROR;
	}
	if ( RunStaticChecks(pApp) != XGE_OK ) {
		AppUnit(pApp);
		return XGE_ERROR;
	}
	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());

	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	if ( pApp->bDirectPaintOK == 0 ) {
		xgeXuiPanelPaintProc(pApp->pPanelWidget, &pApp->tPanel);
		xgeXuiLabelPaintProc(pApp->pLabelWidget, &pApp->tLabel);
		xgeXuiImagePaintProc(pApp->pImageWidget, &pApp->tImage);
		xgeXuiSeparatorPaintProc(pApp->pSeparatorWidget, &pApp->tSeparator);
		xgeXuiButtonPaintProc(pApp->pButtonWidget, &pApp->tButton);
		xgeXuiIconButtonPaintProc(pApp->pIconButtonWidget, &pApp->tIconButton);
		pApp->bDirectPaintOK = 1;
	}
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-visual-proc-lab final-summary frames=%d label=%d image=%d button=%d icon=%d panel=%d separator=%d paint=%d label=%.2fx%.2f proc=%.2fx%.2f image=%.2fx%.2f clicks=%d/%d cb=%d/%d\n",
			pApp->iFrameCount,
			pApp->bLabelOK,
			pApp->bImageOK,
			pApp->bButtonOK,
			pApp->bIconButtonOK,
			pApp->bPanelOK,
			pApp->bSeparatorOK,
			pApp->bDirectPaintOK,
			pApp->tLabelMeasure.fX,
			pApp->tLabelMeasure.fY,
			pApp->tLabelMeasureProc.fX,
			pApp->tLabelMeasureProc.fY,
			pApp->tImageMeasureProc.fX,
			pApp->tImageMeasureProc.fY,
			pApp->tButton.iClickCount,
			pApp->tIconButton.iClickCount,
			pApp->iButtonCallbackCount,
			pApp->iIconButtonCallbackCount);
		printf("xui-visual-proc-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	int iExitCode;

	iFrameLimit = 0;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			iFrameLimit = ArgInt(argv[i + 1], iFrameLimit);
			i++;
		}
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 760;
	tDesc.iHeight = 380;
	tDesc.sTitle = "XGE XUI Visual Proc Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	iExitCode =
		(tApp.bLabelOK && tApp.bImageOK && tApp.bButtonOK && tApp.bIconButtonOK &&
		 tApp.bPanelOK && tApp.bSeparatorOK && tApp.bDirectPaintOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

