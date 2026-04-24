#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_texture_t tIconTexture;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pButtonWidget;
	xge_xui_widget pButtonStateWidget;
	xge_xui_widget pIconWidget;
	xge_xui_widget pIconStateWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tButtonStateLabel;
	xge_xui_label_t tIconStateLabel;
	xge_xui_button_t tButton;
	xge_xui_icon_button_t tIconButton;
	int bFontReady;
	int bTextureReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iButtonCallbacks;
	int iIconCallbacks;
	int bInitOK;
	int bButtonOK;
	int bIconOK;
	int bDispatchOK;
	int bDisabledOK;
	int bDirectPaintOK;
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

static void FormatState(int iState, char* sText, size_t iSize)
{
	int iOffset;

	if ( (sText == NULL) || (iSize == 0) ) {
		return;
	}
	iOffset = snprintf(sText, iSize, "%s", ((iState & XGE_XUI_STATE_HOVER) != 0) ? "hover" : "normal");
	if ( iOffset < 0 || (size_t)iOffset >= iSize ) {
		sText[iSize - 1] = 0;
		return;
	}
	if ( (iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		iOffset += snprintf(sText + iOffset, iSize - (size_t)iOffset, "|active");
	}
	if ( (iState & XGE_XUI_STATE_FOCUS) != 0 ) {
		iOffset += snprintf(sText + iOffset, iSize - (size_t)iOffset, "|focus");
	}
	if ( (iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		(void)snprintf(sText + iOffset, iSize - (size_t)iOffset, "|disabled");
	}
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
			printf("xui-button-state-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-button-state-lab font load failed\n");
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
			arrPixels[i + 0] = (unsigned char)(40 + x * 22);
			arrPixels[i + 1] = (unsigned char)(120 + y * 12);
			arrPixels[i + 2] = (unsigned char)(220 - x * 14);
			arrPixels[i + 3] = ((x == y) || (x == (7 - y)) || (x == 3) || (x == 4) || (y == 3) || (y == 4)) ? 255 : 88;
		}
	}
	if ( xgeTextureCreateRGBA(&pApp->tIconTexture, 8, 8, arrPixels) != XGE_OK ) {
		printf("xui-button-state-lab texture create failed\n");
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
		pApp->iButtonCallbacks++;
	}
}

static void IconClick(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iIconCallbacks++;
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
	if ( tRoot.fW < 620.0f ) {
		tRoot.fW = 620.0f;
	}
	if ( tRoot.fH < 260.0f ) {
		tRoot.fH = 260.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 38.0f });
	xgeXuiWidgetSetRect(pApp->pButtonWidget, (xge_rect_t){ 36.0f, 86.0f, 228.0f, 52.0f });
	xgeXuiWidgetSetRect(pApp->pButtonStateWidget, (xge_rect_t){ 36.0f, 154.0f, 248.0f, 28.0f });
	xgeXuiWidgetSetRect(pApp->pIconWidget, (xge_rect_t){ tRoot.fW - 168.0f, 72.0f, 92.0f, 92.0f });
	xgeXuiWidgetSetRect(pApp->pIconStateWidget, (xge_rect_t){ tRoot.fW - 220.0f, 182.0f, 180.0f, 28.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sStatus[256];
	char sButton[96];
	char sIcon[96];

	FormatState(xgeXuiButtonGetState(&pApp->tButton), sButton, sizeof(sButton));
	FormatState(xgeXuiIconButtonGetState(&pApp->tIconButton), sIcon, sizeof(sIcon));
	snprintf(
		sStatus,
		sizeof(sStatus),
		"init=%d button=%d icon=%d dispatch=%d disabled=%d paint=%d cb=%d/%d clicks=%d/%d",
		pApp->bInitOK,
		pApp->bButtonOK,
		pApp->bIconOK,
		pApp->bDispatchOK,
		pApp->bDisabledOK,
		pApp->bDirectPaintOK,
		pApp->iButtonCallbacks,
		pApp->iIconCallbacks,
		pApp->tButton.iClickCount,
		pApp->tIconButton.iClickCount);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sStatus);
	snprintf(sStatus, sizeof(sStatus), "button: %s (%d)", sButton, xgeXuiButtonGetState(&pApp->tButton));
	xgeXuiLabelSetText(&pApp->tButtonStateLabel, sStatus);
	snprintf(sStatus, sizeof(sStatus), "icon: %s (%d)", sIcon, xgeXuiIconButtonGetState(&pApp->tIconButton));
	xgeXuiLabelSetText(&pApp->tIconStateLabel, sStatus);
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
	tTheme.iTextColor = XGE_COLOR_RGBA(244, 247, 252, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(18, 24, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(30, 42, 60, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(86, 100, 124, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(255, 214, 92, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(54, 104, 188, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(78, 132, 220, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(40, 82, 154, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(116, 84, 38, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(88, 90, 96, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pButtonWidget = xgeXuiWidgetCreate();
	pApp->pButtonStateWidget = xgeXuiWidgetCreate();
	pApp->pIconWidget = xgeXuiWidgetCreate();
	pApp->pIconStateWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pButtonWidget == NULL) ||
	     (pApp->pButtonStateWidget == NULL) || (pApp->pIconWidget == NULL) || (pApp->pIconStateWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(18, 24, 34, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pButtonWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pButtonStateWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pIconWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pIconStateWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pButtonStateWidget, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeXuiWidgetSetBackground(pApp->pIconStateWidget, XGE_COLOR_RGBA(0, 0, 0, 0));

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui button state lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tButtonStateLabel, pApp->pButtonStateWidget, pFont, "button") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tButtonStateLabel, XGE_COLOR_RGBA(250, 214, 120, 255));

	if ( xgeXuiLabelInit(&pApp->tIconStateLabel, pApp->pIconStateWidget, pFont, "icon") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tIconStateLabel, XGE_COLOR_RGBA(160, 226, 196, 255));

	if ( xgeXuiButtonInit(&pApp->tButton, &pApp->tXui, pApp->pButtonWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(&pApp->tButton, pFont, "Primary Button");
	xgeXuiButtonSetTextColor(&pApp->tButton, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiButtonSetColors(
		&pApp->tButton,
		XGE_COLOR_RGBA(54, 104, 188, 255),
		XGE_COLOR_RGBA(78, 132, 220, 255),
		XGE_COLOR_RGBA(40, 82, 154, 255),
		XGE_COLOR_RGBA(116, 84, 38, 255),
		XGE_COLOR_RGBA(88, 90, 96, 180));
	xgeXuiButtonSetClick(&pApp->tButton, ButtonClick, pApp);

	tSrc = (xge_rect_t){ 1.0f, 1.0f, 6.0f, 6.0f };
	if ( xgeXuiIconButtonInit(&pApp->tIconButton, &pApp->tXui, pApp->pIconWidget, pApp->bTextureReady ? &pApp->tIconTexture : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiIconButtonSetClick(&pApp->tIconButton, IconClick, pApp);
	xgeXuiIconButtonSetTexture(&pApp->tIconButton, pApp->bTextureReady ? &pApp->tIconTexture : NULL);
	xgeXuiIconButtonSetSource(&pApp->tIconButton, tSrc);
	xgeXuiIconButtonSetIconColor(&pApp->tIconButton, XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiIconButtonSetMode(&pApp->tIconButton, XGE_XUI_IMAGE_FIT);
	xgeXuiIconButtonSetColors(
		&pApp->tIconButton,
		XGE_COLOR_RGBA(58, 72, 96, 255),
		XGE_COLOR_RGBA(82, 100, 128, 255),
		XGE_COLOR_RGBA(42, 56, 80, 255),
		XGE_COLOR_RGBA(116, 84, 38, 255),
		XGE_COLOR_RGBA(88, 90, 96, 180));
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tCenter;
	xge_rect_t tSrc;
	int iState;
	int iButtonDisabledRet;
	int iIconDisabledRet;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	tSrc = (xge_rect_t){ 1.0f, 1.0f, 6.0f, 6.0f };
	pApp->bInitOK =
		(xgeXuiWidgetIsFocusable(pApp->pButtonWidget) != 0) &&
		(xgeXuiWidgetIsFocusable(pApp->pIconWidget) != 0) &&
		(pApp->pButtonWidget->procEvent == xgeXuiButtonEventProc) &&
		(pApp->pButtonWidget->procPaint == xgeXuiButtonPaintProc) &&
		(pApp->pIconWidget->procEvent == xgeXuiIconButtonEventProc) &&
		(pApp->pIconWidget->procPaint == xgeXuiIconButtonPaintProc) &&
		(strcmp(pApp->tButton.sText, "Primary Button") == 0) &&
		(pApp->tButton.iTextColor == XGE_COLOR_RGBA(248, 250, 252, 255)) &&
		(pApp->tButton.procClick == ButtonClick) &&
		(pApp->tButton.pUser == pApp) &&
		(pApp->tIconButton.pTexture == (pApp->bTextureReady ? &pApp->tIconTexture : NULL)) &&
		(pApp->tIconButton.procClick == IconClick) &&
		(pApp->tIconButton.pUser == pApp) &&
		(pApp->tIconButton.iIconColor == XGE_COLOR_RGBA(255, 214, 92, 255)) &&
		(pApp->tIconButton.iMode == XGE_XUI_IMAGE_FIT) &&
		(pApp->tIconButton.tSrc.fX == tSrc.fX) &&
		(pApp->tIconButton.tSrc.fY == tSrc.fY) &&
		(pApp->tIconButton.tSrc.fW == tSrc.fW) &&
		(pApp->tIconButton.tSrc.fH == tSrc.fH);

	tCenter = WidgetCenter(pApp->pButtonWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tCenter.fX, tCenter.fY);
	pApp->bButtonOK = (xgeXuiButtonEvent(&pApp->tButton, &tEvent) == XGE_XUI_EVENT_CONTINUE);
	iState = xgeXuiButtonGetState(&pApp->tButton);
	pApp->bButtonOK = pApp->bButtonOK && ((iState & XGE_XUI_STATE_HOVER) != 0);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	pApp->bButtonOK =
		pApp->bButtonOK &&
		(xgeXuiButtonEvent(&pApp->tButton, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	iState = xgeXuiButtonGetState(&pApp->tButton);
	pApp->bButtonOK =
		pApp->bButtonOK &&
		((iState & XGE_XUI_STATE_ACTIVE) != 0) &&
		((iState & XGE_XUI_STATE_FOCUS) != 0) &&
		(pApp->tXui.pFocus == pApp->pButtonWidget) &&
		(pApp->tXui.pCapture == pApp->pButtonWidget);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	pApp->bButtonOK =
		pApp->bButtonOK &&
		(xgeXuiButtonEvent(&pApp->tButton, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tButton.iClickCount == 1) &&
		(pApp->iButtonCallbacks == 1) &&
		(pApp->tXui.pCapture == NULL);

	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_POINTER_LEAVE, 0, 0.0f, 0.0f);
	pApp->bButtonOK =
		pApp->bButtonOK &&
		(xgeXuiButtonEvent(&pApp->tButton, &tEvent) == XGE_XUI_EVENT_CONTINUE);
	iState = xgeXuiButtonGetState(&pApp->tButton);
	pApp->bButtonOK =
		pApp->bButtonOK &&
		((iState & XGE_XUI_STATE_HOVER) == 0) &&
		((iState & XGE_XUI_STATE_FOCUS) != 0);

	tCenter = WidgetCenter(pApp->pIconWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tCenter.fX, tCenter.fY);
	pApp->bDispatchOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONTINUE);
	iState = xgeXuiIconButtonGetState(&pApp->tIconButton);
	pApp->bDispatchOK = pApp->bDispatchOK && ((iState & XGE_XUI_STATE_HOVER) != 0);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	pApp->bDispatchOK =
		pApp->bDispatchOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	iState = xgeXuiIconButtonGetState(&pApp->tIconButton);
	pApp->bDispatchOK =
		pApp->bDispatchOK &&
		((iState & XGE_XUI_STATE_ACTIVE) != 0) &&
		((iState & XGE_XUI_STATE_FOCUS) != 0) &&
		(pApp->tXui.pFocus == pApp->pIconWidget) &&
		(pApp->tXui.pCapture == pApp->pIconWidget);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	pApp->bDispatchOK =
		pApp->bDispatchOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tIconButton.iClickCount == 1) &&
		(pApp->iIconCallbacks == 1) &&
		(pApp->tXui.pCapture == NULL);

	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	pApp->bDispatchOK =
		pApp->bDispatchOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tIconButton.iClickCount == 2) &&
		(pApp->iIconCallbacks == 2);

	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_POINTER_LEAVE, 0, 0.0f, 0.0f);
	(void)xgeXuiIconButtonEvent(&pApp->tIconButton, &tEvent);
	iState = xgeXuiIconButtonGetState(&pApp->tIconButton);
	pApp->bIconOK =
		((iState & XGE_XUI_STATE_HOVER) == 0) &&
		((iState & XGE_XUI_STATE_FOCUS) != 0) &&
		(pApp->tIconButton.iMode == XGE_XUI_IMAGE_FIT) &&
		(pApp->tIconButton.iIconColor == XGE_COLOR_RGBA(255, 214, 92, 255));

	xgeXuiWidgetSetEnabled(pApp->pButtonWidget, 0);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, 0.0f, 0.0f);
	iButtonDisabledRet = xgeXuiButtonEvent(&pApp->tButton, &tEvent);
	xgeXuiWidgetSetEnabled(pApp->pIconWidget, 0);
	iIconDisabledRet = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bDisabledOK =
		(iButtonDisabledRet == XGE_XUI_EVENT_CONTINUE) &&
		(iIconDisabledRet == XGE_XUI_EVENT_CONTINUE) &&
		((xgeXuiButtonGetState(&pApp->tButton) & XGE_XUI_STATE_DISABLED) != 0) &&
		((xgeXuiIconButtonGetState(&pApp->tIconButton) & XGE_XUI_STATE_DISABLED) != 0);

	xgeXuiWidgetSetEnabled(pApp->pButtonWidget, 1);
	xgeXuiWidgetSetEnabled(pApp->pIconWidget, 1);
	xgeXuiSetFocus(&pApp->tXui, pApp->pIconWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_POINTER_ENTER, 0, 0.0f, 0.0f);
	(void)xgeXuiButtonEvent(&pApp->tButton, &tEvent);

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
	if ( LoadFont(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( CreateIconTexture(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	return RunChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiIconButtonUnit(&pApp->tIconButton);
	xgeXuiButtonUnit(&pApp->tButton);
	xgeXuiLabelUnit(&pApp->tIconStateLabel);
	xgeXuiLabelUnit(&pApp->tButtonStateLabel);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	if ( pApp->bTextureReady ) {
		xgeTextureFree(&pApp->tIconTexture);
	}
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
	memset(pApp, 0, sizeof(*pApp));
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());

	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 24, 34, 255));
	if ( pApp->bDirectPaintOK == 0 ) {
		xgeXuiButtonPaintProc(pApp->pButtonWidget, &pApp->tButton);
		xgeXuiIconButtonPaintProc(pApp->pIconWidget, &pApp->tIconButton);
		pApp->bDirectPaintOK = 1;
	}
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( pApp->iFrameCount >= pApp->iFrameLimit ) {
		printf(
			"xui-button-state-lab final-summary frames=%d init=%d button=%d icon=%d dispatch=%d disabled=%d paint=%d button(state=%d click=%d cb=%d) icon(state=%d click=%d cb=%d)\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bButtonOK,
			pApp->bIconOK,
			pApp->bDispatchOK,
			pApp->bDisabledOK,
			pApp->bDirectPaintOK,
			xgeXuiButtonGetState(&pApp->tButton),
			pApp->tButton.iClickCount,
			pApp->iButtonCallbacks,
			xgeXuiIconButtonGetState(&pApp->tIconButton),
			pApp->tIconButton.iClickCount,
			pApp->iIconCallbacks);
		printf("xui-button-state-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int iFrameLimit;
	int iExitCode;
	int i;

	iFrameLimit = 180;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			iFrameLimit = ArgInt(argv[i + 1], iFrameLimit);
			i++;
		}
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 680;
	tDesc.iHeight = 280;
	tDesc.sTitle = "XGE XUI Button State Lab";
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
		(tApp.bInitOK && tApp.bButtonOK && tApp.bIconOK && tApp.bDispatchOK &&
		 tApp.bDisabledOK && tApp.bDirectPaintOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}
