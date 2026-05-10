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
	xge_xui_widget pIconActionWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tLabel;
	xge_xui_image_t tImage;
	xge_xui_button_t tButton;
	xge_xui_button_t tIconAction;
	xge_xui_panel_t tPanel;
	xge_xui_separator_t tSeparator;
	int bFontReady;
	int bTextureReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iButtonCallbackCount;
	int iIconActionCallbackCount;
	int bLabelOK;
	int bImageOK;
	int bButtonOK;
	int bIconActionOK;
	int bPanelOK;
	int bSeparatorOK;
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
			printf("xui-basic-controls-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-basic-controls-lab font load failed\n");
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
			if ( (x == y) || (x == (7 - y)) || (y == 3) || (y == 4) ) {
				arrPixels[i + 0] = 255;
				arrPixels[i + 1] = 255;
				arrPixels[i + 2] = 255;
				arrPixels[i + 3] = 255;
			}
		}
	}
	if ( xgeTextureCreateRGBA(&pApp->tIconTexture, 8, 8, arrPixels) != XGE_OK ) {
		printf("xui-basic-controls-lab texture create failed\n");
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

static void IconActionClick(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iIconActionCallbackCount++;
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
	if ( tRoot.fW < 500.0f ) {
		tRoot.fW = 500.0f;
	}
	if ( tRoot.fH < 280.0f ) {
		tRoot.fH = 280.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 38.0f });
	xgeXuiWidgetSetRect(pApp->pPanelWidget, (xge_rect_t){ 20.0f, 58.0f, 240.0f, 164.0f });
	xgeXuiWidgetSetRect(pApp->pLabelWidget, (xge_rect_t){ 16.0f, 40.0f, 208.0f, 28.0f });
	xgeXuiWidgetSetRect(pApp->pImageWidget, (xge_rect_t){ 16.0f, 84.0f, 92.0f, 56.0f });
	xgeXuiWidgetSetRect(pApp->pSeparatorWidget, (xge_rect_t){ 284.0f, 60.0f, 4.0f, 164.0f });
	xgeXuiWidgetSetRect(pApp->pButtonWidget, (xge_rect_t){ 312.0f, 78.0f, 170.0f, 44.0f });
	xgeXuiWidgetSetRect(pApp->pIconActionWidget, (xge_rect_t){ 365.0f, 140.0f, 64.0f, 64.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"label=%d image=%d button=%d icon=%d panel=%d separator=%d state=%d clicks=%d/%d cb=%d/%d",
		pApp->bLabelOK,
		pApp->bImageOK,
		pApp->bButtonOK,
		pApp->bIconActionOK,
		pApp->bPanelOK,
		pApp->bSeparatorOK,
		pApp->bStateOK,
		pApp->tButton.iClickCount,
		pApp->tIconAction.iClickCount,
		pApp->iButtonCallbackCount,
		pApp->iIconActionCallbackCount);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(240, 244, 250, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 36, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(32, 40, 54, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(86, 100, 124, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(92, 164, 255, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(54, 124, 214, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(76, 148, 236, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(36, 96, 176, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(58, 86, 126, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(84, 86, 92, 180);
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
	pApp->pIconActionWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pPanelWidget == NULL) || (pApp->pLabelWidget == NULL) || (pApp->pImageWidget == NULL) || (pApp->pSeparatorWidget == NULL) || (pApp->pButtonWidget == NULL) || (pApp->pIconActionWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(22, 28, 38, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	if ( xgeXuiWidgetAdd(pRoot, pApp->pRootPanel) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui basic controls lab");
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanelWidget);
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPanelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pApp->tLabel, pApp->pLabelWidget, pFont, "Label Default");
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pLabelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiImageInit(&pApp->tImage, pApp->pImageWidget, pApp->bTextureReady ? &pApp->tIconTexture : NULL);
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pImageWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiSeparatorInit(&pApp->tSeparator, pApp->pSeparatorWidget);
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pSeparatorWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiButtonInit(&pApp->tButton, &pApp->tXui, pApp->pButtonWidget);
	xgeXuiButtonSetText(&pApp->tButton, pFont, "Action Button");
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pButtonWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiButtonInit(&pApp->tIconAction, &pApp->tXui, pApp->pIconActionWidget);
	xgeXuiButtonSetText(&pApp->tIconAction, NULL, "");
	xgeXuiButtonSetIconLayout(&pApp->tIconAction, XGE_XUI_BUTTON_ICON_LEFT, 18.0f, 0.0f);
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pIconActionWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tSrc;
	xge_vec2_t tMeasure;
	xge_vec2_t tCenter;
	int bLabelDefaultOK;
	int bImageDefaultOK;
	int bButtonThemeOK;
	int bButtonMouseOK;
	int bButtonKeyOK;
	int bIconThemeOK;
	int bIconKeyOK;
	int bIconMouseOK;
	int bPanelDefaultOK;
	int bPanelUpdateOK;
	int bSeparatorDefaultOK;
	int bSeparatorUpdateOK;

	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	bLabelDefaultOK =
		(pApp->tLabel.pWidget == pApp->pLabelWidget) &&
		(pApp->tLabel.pFont == (pApp->bFontReady ? &pApp->tFont : NULL)) &&
		(strcmp(pApp->tLabel.sText, "Label Default") == 0) &&
		(pApp->tLabel.iColor == XGE_COLOR_RGBA(255, 255, 255, 255)) &&
		(pApp->tLabel.iTextFlags == (XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP | XGE_TEXT_CLIP));
	xgeXuiLabelSetText(&pApp->tLabel, "Label Updated");
	xgeXuiLabelSetFont(&pApp->tLabel, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiLabelSetColor(&pApp->tLabel, XGE_COLOR_RGBA(250, 214, 120, 255));
	xgeXuiLabelSetAlign(&pApp->tLabel, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	tMeasure = xgeXuiLabelMeasure(&pApp->tLabel);
	pApp->bLabelOK =
		bLabelDefaultOK &&
		(strcmp(pApp->tLabel.sText, "Label Updated") == 0) &&
		(pApp->tLabel.iColor == XGE_COLOR_RGBA(250, 214, 120, 255)) &&
		(pApp->tLabel.iTextFlags == (XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP)) &&
		FloatNear(tMeasure.fX, pApp->tLabel.tMeasuredSize.fX, 0.01f) &&
		FloatNear(tMeasure.fY, pApp->tLabel.tMeasuredSize.fY, 0.01f) &&
		(tMeasure.fX > 0.0f) &&
		(tMeasure.fY > 0.0f);

	bImageDefaultOK =
		(pApp->tImage.pWidget == pApp->pImageWidget) &&
		(pApp->tImage.pTexture == (pApp->bTextureReady ? &pApp->tIconTexture : NULL)) &&
		(pApp->tImage.iColor == XGE_COLOR_RGBA(255, 255, 255, 255)) &&
		(pApp->tImage.iMode == XGE_XUI_IMAGE_NATURAL);
	tSrc = (xge_rect_t){ 2.0f, 2.0f, 4.0f, 4.0f };
	xgeXuiImageSetTexture(&pApp->tImage, pApp->bTextureReady ? &pApp->tIconTexture : NULL);
	xgeXuiImageSetSource(&pApp->tImage, tSrc);
	xgeXuiImageSetColor(&pApp->tImage, XGE_COLOR_RGBA(112, 220, 176, 255));
	xgeXuiImageSetMode(&pApp->tImage, XGE_XUI_IMAGE_FIT);
	pApp->bImageOK =
		bImageDefaultOK &&
		(pApp->tImage.pTexture == (pApp->bTextureReady ? &pApp->tIconTexture : NULL)) &&
		FloatNear(pApp->tImage.tSrc.fX, tSrc.fX, 0.01f) &&
		FloatNear(pApp->tImage.tSrc.fY, tSrc.fY, 0.01f) &&
		FloatNear(pApp->tImage.tSrc.fW, tSrc.fW, 0.01f) &&
		FloatNear(pApp->tImage.tSrc.fH, tSrc.fH, 0.01f) &&
		(pApp->tImage.iColor == XGE_COLOR_RGBA(112, 220, 176, 255)) &&
		(pApp->tImage.iMode == XGE_XUI_IMAGE_FIT);

	bPanelDefaultOK =
		(pApp->tPanel.pWidget == pApp->pPanelWidget) &&
		(pApp->pPanelWidget->tStyle.iBackgroundColor == XGE_COLOR_RGBA(249, 253, 255, 255)) &&
		FloatNear(pApp->pPanelWidget->tStyle.fBorderWidth, 1.5f, 0.01f) &&
		(pApp->pPanelWidget->tStyle.iBorderColor == XGE_COLOR_RGBA(127, 196, 229, 255)) &&
		(pApp->tPanel.iTitleColor == XGE_COLOR_RGBA(24, 56, 79, 255)) &&
		(pApp->tPanel.iTitleFlags == (XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP | XGE_TEXT_CLIP)) &&
		(pApp->tPanel.bClip == 0);
	xgeXuiPanelSetBackground(&pApp->tPanel, XGE_COLOR_RGBA(34, 44, 62, 255));
	xgeXuiPanelSetTitle(&pApp->tPanel, pApp->bFontReady ? &pApp->tFont : NULL, "Controls Panel");
	xgeXuiPanelSetTitleColor(&pApp->tPanel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiPanelSetTitleAlign(&pApp->tPanel, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE);
	xgeXuiPanelSetClip(&pApp->tPanel, 1);
	bPanelUpdateOK =
		(pApp->tPanel.pFont == (pApp->bFontReady ? &pApp->tFont : NULL)) &&
		(strcmp(pApp->tPanel.sTitle, "Controls Panel") == 0) &&
		(pApp->pPanelWidget->tStyle.iBackgroundColor == XGE_COLOR_RGBA(34, 44, 62, 255)) &&
		(pApp->tPanel.iTitleColor == XGE_COLOR_RGBA(248, 250, 252, 255)) &&
		(pApp->tPanel.iTitleFlags == (XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP)) &&
		(pApp->tPanel.bClip == 1);
	pApp->bPanelOK = bPanelDefaultOK && bPanelUpdateOK;

	bSeparatorDefaultOK =
		(pApp->tSeparator.pWidget == pApp->pSeparatorWidget) &&
		(pApp->tSeparator.iColor == XGE_COLOR_RGBA(90, 104, 122, 180)) &&
		FloatNear(pApp->tSeparator.fThickness, 1.0f, 0.01f) &&
		(pApp->tSeparator.iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL);
	xgeXuiSeparatorSetColor(&pApp->tSeparator, XGE_COLOR_RGBA(244, 178, 92, 255));
	xgeXuiSeparatorSetThickness(&pApp->tSeparator, 0.25f);
	xgeXuiSeparatorSetOrientation(&pApp->tSeparator, XGE_XUI_SEPARATOR_VERTICAL);
	bSeparatorUpdateOK =
		(pApp->tSeparator.iColor == XGE_COLOR_RGBA(244, 178, 92, 255)) &&
		FloatNear(pApp->tSeparator.fThickness, 0.25f, 0.01f) &&
		(pApp->tSeparator.iOrientation == XGE_XUI_SEPARATOR_VERTICAL);
	pApp->bSeparatorOK = bSeparatorDefaultOK && bSeparatorUpdateOK;

	xgeXuiButtonSetText(&pApp->tButton, pApp->bFontReady ? &pApp->tFont : NULL, "Button Updated");
	xgeXuiButtonSetTextColor(&pApp->tButton, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiButtonSetColors(
		&pApp->tButton,
		XGE_COLOR_RGBA(68, 124, 216, 255),
		XGE_COLOR_RGBA(88, 148, 240, 255),
		XGE_COLOR_RGBA(42, 92, 170, 255),
		XGE_COLOR_RGBA(82, 112, 164, 255),
		XGE_COLOR_RGBA(82, 84, 90, 180));
	xgeXuiButtonSetClick(&pApp->tButton, ButtonClick, pApp);
	bButtonThemeOK =
		(pApp->tButton.pFont == (pApp->bFontReady ? &pApp->tFont : NULL)) &&
		(strcmp(pApp->tButton.sText, "Button Updated") == 0) &&
		(pApp->tButton.iTextColor == XGE_COLOR_RGBA(248, 250, 252, 255)) &&
		(pApp->tButton.procClick == ButtonClick) &&
		(pApp->tButton.pUser == pApp) &&
		(pApp->tButton.iColorNormal == XGE_COLOR_RGBA(68, 124, 216, 255)) &&
		(pApp->tButton.iColorHover == XGE_COLOR_RGBA(88, 148, 240, 255)) &&
		(pApp->tButton.iColorActive == XGE_COLOR_RGBA(42, 92, 170, 255)) &&
		(pApp->tButton.iColorFocus == XGE_COLOR_RGBA(82, 112, 164, 255)) &&
		(pApp->tButton.iColorDisabled == XGE_COLOR_RGBA(82, 84, 90, 180));
	tCenter = WidgetCenter(pApp->pButtonWidget);
	xgeXuiSetFocus(&pApp->tXui, pApp->pButtonWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tCenter.fX, tCenter.fY);
	xgeXuiButtonEvent(&pApp->tButton, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiButtonEvent(&pApp->tButton, &tEvent);
	bButtonMouseOK =
		((xgeXuiButtonGetState(&pApp->tButton) & XGE_XUI_STATE_ACTIVE) != 0) &&
		((xgeXuiButtonGetState(&pApp->tButton) & XGE_XUI_STATE_FOCUS) != 0);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiButtonEvent(&pApp->tButton, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	xgeXuiButtonEvent(&pApp->tButton, &tEvent);
	bButtonKeyOK =
		(pApp->tButton.iClickCount == 2) &&
		(pApp->iButtonCallbackCount == 2);
	xgeXuiWidgetSetEnabled(pApp->pButtonWidget, 0);
	pApp->bStateOK = ((xgeXuiButtonGetState(&pApp->tButton) & XGE_XUI_STATE_DISABLED) != 0);
	xgeXuiWidgetSetEnabled(pApp->pButtonWidget, 1);
	pApp->bButtonOK = bButtonThemeOK && bButtonMouseOK && bButtonKeyOK;

	xgeXuiButtonSetClick(&pApp->tIconAction, IconActionClick, pApp);
	xgeXuiButtonSetIcon(&pApp->tIconAction, pApp->bTextureReady ? &pApp->tIconTexture : NULL, tSrc);
	xgeXuiButtonSetIconColor(&pApp->tIconAction, XGE_COLOR_RGBA(255, 214, 96, 255));
	xgeXuiButtonSetIconLayout(&pApp->tIconAction, XGE_XUI_BUTTON_ICON_LEFT, 18.0f, 0.0f);
	xgeXuiButtonSetColors(
		&pApp->tIconAction,
		XGE_COLOR_RGBA(64, 76, 98, 255),
		XGE_COLOR_RGBA(84, 96, 122, 255),
		XGE_COLOR_RGBA(42, 52, 72, 255),
		XGE_COLOR_RGBA(108, 92, 54, 255),
		XGE_COLOR_RGBA(84, 84, 84, 180));
	bIconThemeOK =
		(pApp->tIconAction.pIconTexture == (pApp->bTextureReady ? &pApp->tIconTexture : NULL)) &&
		FloatNear(pApp->tIconAction.tIconSrc.fW, 4.0f, 0.01f) &&
		(pApp->tIconAction.iIconColor == XGE_COLOR_RGBA(255, 214, 96, 255)) &&
		(pApp->tIconAction.procClick == IconActionClick) &&
		(pApp->tIconAction.pUser == pApp);
	xgeXuiSetFocus(&pApp->tXui, pApp->pIconActionWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	xgeXuiButtonEvent(&pApp->tIconAction, &tEvent);
	bIconKeyOK =
		(pApp->tIconAction.iClickCount == 1) &&
		(pApp->iIconActionCallbackCount == 1) &&
		((xgeXuiButtonGetState(&pApp->tIconAction) & XGE_XUI_STATE_FOCUS) != 0);
	tCenter = WidgetCenter(pApp->pIconActionWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tCenter.fX, tCenter.fY);
	xgeXuiButtonEvent(&pApp->tIconAction, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiButtonEvent(&pApp->tIconAction, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiButtonEvent(&pApp->tIconAction, &tEvent);
	bIconMouseOK =
		(pApp->tIconAction.iClickCount == 2) &&
		(pApp->iIconActionCallbackCount == 2);
	xgeXuiWidgetSetEnabled(pApp->pIconActionWidget, 0);
	pApp->bStateOK =
		pApp->bStateOK &&
		((xgeXuiButtonGetState(&pApp->tIconAction) & XGE_XUI_STATE_DISABLED) != 0);
	xgeXuiWidgetSetEnabled(pApp->pIconActionWidget, 1);
	pApp->bIconActionOK = bIconThemeOK && bIconKeyOK && bIconMouseOK;

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
	if ( CreateIconTexture(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	return RunStaticChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiSeparatorUnit(&pApp->tSeparator);
	xgeXuiPanelUnit(&pApp->tPanel);
	xgeXuiButtonUnit(&pApp->tIconAction);
	xgeXuiButtonUnit(&pApp->tButton);
	xgeXuiImageUnit(&pApp->tImage);
	xgeXuiLabelUnit(&pApp->tLabel);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	if ( pApp->bTextureReady ) {
		xgeTextureFree(&pApp->tIconTexture);
	}
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
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-basic-controls-lab final-summary frames=%d label=%d image=%d button=%d icon=%d panel=%d separator=%d state=%d clicks(button=%d icon=%d cb=%d/%d) label_size=%.0fx%.0f image_src=%.0f,%.0f,%.0f,%.0f panel_clip=%d sep(thickness=%.1f orientation=%d)\n",
			pApp->iFrameCount,
			pApp->bLabelOK,
			pApp->bImageOK,
			pApp->bButtonOK,
			pApp->bIconActionOK,
			pApp->bPanelOK,
			pApp->bSeparatorOK,
			pApp->bStateOK,
			pApp->tButton.iClickCount,
			pApp->tIconAction.iClickCount,
			pApp->iButtonCallbackCount,
			pApp->iIconActionCallbackCount,
			pApp->tLabel.tMeasuredSize.fX,
			pApp->tLabel.tMeasuredSize.fY,
			pApp->tImage.tSrc.fX,
			pApp->tImage.tSrc.fY,
			pApp->tImage.tSrc.fW,
			pApp->tImage.tSrc.fH,
			pApp->tPanel.bClip,
			pApp->tSeparator.fThickness,
			pApp->tSeparator.iOrientation);
		printf("xui-basic-controls-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	iFrameLimit = ArgInt(getenv("XGE_XUI_BASIC_CONTROLS_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			iFrameLimit = ArgInt(argv[++i], iFrameLimit);
		}
	}

	tDesc.iWidth = 540;
	tDesc.iHeight = 320;
	tDesc.sTitle = "XGE XUI Basic Controls Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		printf("xui-basic-controls-lab init failed\n");
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	iFrameLimit =
		(tApp.bLabelOK && tApp.bImageOK && tApp.bButtonOK && tApp.bIconActionOK &&
		 tApp.bPanelOK && tApp.bSeparatorOK && tApp.bStateOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iFrameLimit;
}

