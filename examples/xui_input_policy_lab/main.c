#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pHintWidget;
	xge_xui_widget pPlaceholderLabelWidget;
	xge_xui_widget pPlaceholderInputWidget;
	xge_xui_widget pScrollLabelWidget;
	xge_xui_widget pScrollInputWidget;
	xge_xui_widget pPasswordLabelWidget;
	xge_xui_widget pPasswordInputWidget;
	xge_xui_widget pReadonlyLabelWidget;
	xge_xui_widget pReadonlyInputWidget;
	xge_xui_widget pDisabledLabelWidget;
	xge_xui_widget pDisabledInputWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tHintLabel;
	xge_xui_label_t tPlaceholderLabel;
	xge_xui_label_t tScrollLabel;
	xge_xui_label_t tPasswordLabel;
	xge_xui_label_t tReadonlyLabel;
	xge_xui_label_t tDisabledLabel;
	xge_xui_input_t tPlaceholderInput;
	xge_xui_input_t tScrollInput;
	xge_xui_input_t tPasswordInput;
	xge_xui_input_t tReadonlyInput;
	xge_xui_input_t tDisabledInput;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bInitOK;
	int bPlaceholderOK;
	int bScrollOK;
	int bPasswordOK;
	int bReadonlyOK;
	int bDisabledOK;
	float fScrollProbe;
	xge_rect_t tScrollCandidate;
	int iReadonlySelectStart;
	int iReadonlySelectEnd;
} app_state_t;

static const char* g_sScrollText =
	"alpha beta gamma delta epsilon zeta eta theta iota kappa lambda mu";

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
	return (fA >= (fB - fEpsilon)) && (fA <= (fB + fEpsilon));
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeTextEvent(xge_event_t* pEvent, uint32_t iCodepoint)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_TEXT;
	pEvent->iCodepoint = iCodepoint;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey, int iMods)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
	pEvent->iParam2 = iMods;
}

static xge_vec2_t WidgetCenter(xge_xui_widget pWidget)
{
	xge_vec2_t tPoint;

	tPoint.fX = pWidget->tRect.fX + pWidget->tRect.fW * 0.5f;
	tPoint.fY = pWidget->tRect.fY + pWidget->tRect.fH * 0.5f;
	return tPoint;
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
			printf("xui-input-policy-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-input-policy-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void SetupInputColors(xge_xui_input pInput)
{
	xgeXuiInputSetColors(
		pInput,
		XGE_COLOR_RGBA(242, 246, 252, 255),
		XGE_COLOR_RGBA(28, 36, 48, 255),
		XGE_COLOR_RGBA(44, 62, 88, 255),
		XGE_COLOR_RGBA(255, 214, 96, 255));
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	float fLabelX;
	float fInputX;
	float fInputW;
	float fRowY;
	float fRowGap;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}

	tRoot.fX = 18.0f;
	tRoot.fY = 18.0f;
	tRoot.fW = (float)iWidth - 36.0f;
	tRoot.fH = (float)iHeight - 36.0f;
	if ( tRoot.fW < 760.0f ) {
		tRoot.fW = 760.0f;
	}
	if ( tRoot.fH < 380.0f ) {
		tRoot.fH = 380.0f;
	}

	fLabelX = 24.0f;
	fInputX = 178.0f;
	fInputW = tRoot.fW - fInputX - 28.0f;
	fRowY = 64.0f;
	fRowGap = 48.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 24.0f, tRoot.fH - 40.0f, tRoot.fW - 48.0f, 22.0f });

	xgeXuiWidgetSetRect(pApp->pPlaceholderLabelWidget, (xge_rect_t){ fLabelX, fRowY + 4.0f, 136.0f, 24.0f });
	xgeXuiWidgetSetRect(pApp->pPlaceholderInputWidget, (xge_rect_t){ fInputX, fRowY, fInputW, 34.0f });
	fRowY += fRowGap;

	xgeXuiWidgetSetRect(pApp->pScrollLabelWidget, (xge_rect_t){ fLabelX, fRowY + 4.0f, 136.0f, 24.0f });
	xgeXuiWidgetSetRect(pApp->pScrollInputWidget, (xge_rect_t){ fInputX, fRowY, fInputW, 34.0f });
	fRowY += fRowGap;

	xgeXuiWidgetSetRect(pApp->pPasswordLabelWidget, (xge_rect_t){ fLabelX, fRowY + 4.0f, 136.0f, 24.0f });
	xgeXuiWidgetSetRect(pApp->pPasswordInputWidget, (xge_rect_t){ fInputX, fRowY, fInputW, 34.0f });
	fRowY += fRowGap;

	xgeXuiWidgetSetRect(pApp->pReadonlyLabelWidget, (xge_rect_t){ fLabelX, fRowY + 4.0f, 136.0f, 24.0f });
	xgeXuiWidgetSetRect(pApp->pReadonlyInputWidget, (xge_rect_t){ fInputX, fRowY, fInputW, 34.0f });
	fRowY += fRowGap;

	xgeXuiWidgetSetRect(pApp->pDisabledLabelWidget, (xge_rect_t){ fLabelX, fRowY + 4.0f, 136.0f, 24.0f });
	xgeXuiWidgetSetRect(pApp->pDisabledInputWidget, (xge_rect_t){ fInputX, fRowY, fInputW, 34.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[320];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d placeholder=%d scroll=%d password=%d readonly=%d disabled=%d scroll=%.0f cand=%.0f ro=%d..%d pwd=%d",
		pApp->bInitOK,
		pApp->bPlaceholderOK,
		pApp->bScrollOK,
		pApp->bPasswordOK,
		pApp->bReadonlyOK,
		pApp->bDisabledOK,
		pApp->fScrollProbe,
		pApp->tScrollCandidate.fX,
		pApp->iReadonlySelectStart,
		pApp->iReadonlySelectEnd,
		(int)strlen(xgeXuiInputGetText(&pApp->tPasswordInput)));
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
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(18, 24, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(32, 40, 54, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 96, 118, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(50, 64, 84, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(68, 84, 108, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(44, 58, 78, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(44, 64, 96, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(78, 82, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	pApp->pPlaceholderLabelWidget = xgeXuiWidgetCreate();
	pApp->pPlaceholderInputWidget = xgeXuiWidgetCreate();
	pApp->pScrollLabelWidget = xgeXuiWidgetCreate();
	pApp->pScrollInputWidget = xgeXuiWidgetCreate();
	pApp->pPasswordLabelWidget = xgeXuiWidgetCreate();
	pApp->pPasswordInputWidget = xgeXuiWidgetCreate();
	pApp->pReadonlyLabelWidget = xgeXuiWidgetCreate();
	pApp->pReadonlyInputWidget = xgeXuiWidgetCreate();
	pApp->pDisabledLabelWidget = xgeXuiWidgetCreate();
	pApp->pDisabledInputWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pHintWidget == NULL) ||
	     (pApp->pPlaceholderLabelWidget == NULL) || (pApp->pPlaceholderInputWidget == NULL) ||
	     (pApp->pScrollLabelWidget == NULL) || (pApp->pScrollInputWidget == NULL) ||
	     (pApp->pPasswordLabelWidget == NULL) || (pApp->pPasswordInputWidget == NULL) ||
	     (pApp->pReadonlyLabelWidget == NULL) || (pApp->pReadonlyInputWidget == NULL) ||
	     (pApp->pDisabledLabelWidget == NULL) || (pApp->pDisabledInputWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(20, 26, 36, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPlaceholderLabelWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPlaceholderInputWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pScrollLabelWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pScrollInputWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPasswordLabelWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPasswordInputWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pReadonlyLabelWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pReadonlyInputWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pDisabledLabelWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pDisabledInputWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 56, 74, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pPlaceholderInputWidget, 8.0f, 7.0f, 8.0f, 7.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pScrollInputWidget, 8.0f, 7.0f, 8.0f, 7.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pPasswordInputWidget, 8.0f, 7.0f, 8.0f, 7.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pReadonlyInputWidget, 8.0f, 7.0f, 8.0f, 7.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pDisabledInputWidget, 8.0f, 7.0f, 8.0f, 7.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui input policy lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tHintLabel, pApp->pHintWidget, pFont, "Auto-check covers placeholder, horizontal scroll, password guard, readonly consume, and disabled focus blocking.") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tPlaceholderLabel, pApp->pPlaceholderLabelWidget, pFont, "Placeholder") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tScrollLabel, pApp->pScrollLabelWidget, pFont, "Long scroll") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tPasswordLabel, pApp->pPasswordLabelWidget, pFont, "Password") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tReadonlyLabel, pApp->pReadonlyLabelWidget, pFont, "Readonly") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiLabelInit(&pApp->tDisabledLabel, pApp->pDisabledLabelWidget, pFont, "Disabled") != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiLabelSetColor(&pApp->tHintLabel, XGE_COLOR_RGBA(164, 182, 206, 255));
	xgeXuiLabelSetColor(&pApp->tPlaceholderLabel, XGE_COLOR_RGBA(214, 224, 238, 255));
	xgeXuiLabelSetColor(&pApp->tScrollLabel, XGE_COLOR_RGBA(214, 224, 238, 255));
	xgeXuiLabelSetColor(&pApp->tPasswordLabel, XGE_COLOR_RGBA(214, 224, 238, 255));
	xgeXuiLabelSetColor(&pApp->tReadonlyLabel, XGE_COLOR_RGBA(214, 224, 238, 255));
	xgeXuiLabelSetColor(&pApp->tDisabledLabel, XGE_COLOR_RGBA(214, 224, 238, 255));

	if ( xgeXuiInputInit(&pApp->tPlaceholderInput, &pApp->tXui, pApp->pPlaceholderInputWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiInputInit(&pApp->tScrollInput, &pApp->tXui, pApp->pScrollInputWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiInputInit(&pApp->tPasswordInput, &pApp->tXui, pApp->pPasswordInputWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiInputInit(&pApp->tReadonlyInput, &pApp->tXui, pApp->pReadonlyInputWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiInputInit(&pApp->tDisabledInput, &pApp->tXui, pApp->pDisabledInputWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}

	SetupInputColors(&pApp->tPlaceholderInput);
	SetupInputColors(&pApp->tScrollInput);
	SetupInputColors(&pApp->tPasswordInput);
	SetupInputColors(&pApp->tReadonlyInput);
	SetupInputColors(&pApp->tDisabledInput);

	xgeXuiInputSetPlaceholder(&pApp->tPlaceholderInput, "Placeholder stays visible when text is empty");

	xgeXuiInputSetPlaceholder(&pApp->tScrollInput, "Horizontal scroll probe");
	xgeXuiInputSetText(&pApp->tScrollInput, g_sScrollText);

	xgeXuiInputSetPlaceholder(&pApp->tPasswordInput, "Password mask");
	xgeXuiInputSetText(&pApp->tPasswordInput, "Secret42");
	xgeXuiInputSetPassword(&pApp->tPasswordInput, 1);

	xgeXuiInputSetText(&pApp->tReadonlyInput, "locked field");
	xgeXuiInputSetReadonly(&pApp->tReadonlyInput, 1);

	xgeXuiInputSetText(&pApp->tDisabledInput, "disabled field");
	xgeXuiInputSetDisabled(&pApp->tDisabledInput, 1);
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tCandidate;
	xge_vec2_t tCenter;
	int iResult;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	pApp->bInitOK =
		(pApp->pScrollInputWidget->procEvent == xgeXuiInputEventProc) &&
		(pApp->pScrollInputWidget->procUpdate == xgeXuiInputUpdateProc) &&
		(pApp->pScrollInputWidget->procPaint == xgeXuiInputPaintProc) &&
		(pApp->tPasswordInput.bPassword != 0) &&
		(pApp->tReadonlyInput.bReadonly != 0) &&
		(pApp->tDisabledInput.bDisabled != 0) &&
		(xgeXuiWidgetIsEnabled(pApp->pDisabledInputWidget) == 0);

	tCandidate = xgeXuiInputGetCandidateRect(&pApp->tPlaceholderInput);
	pApp->bPlaceholderOK =
		(strcmp(xgeXuiInputGetText(&pApp->tPlaceholderInput), "") == 0) &&
		(pApp->tPlaceholderInput.sPlaceholder != NULL) &&
		(pApp->tPlaceholderInput.sPlaceholder[0] != 0) &&
		FloatNear(tCandidate.fH, pApp->pPlaceholderInputWidget->tContentRect.fH, 0.01f);

	xgeXuiSetFocus(&pApp->tXui, pApp->pScrollInputWidget);
	xgeXuiTextSetCursor(&pApp->tScrollInput.tText, 0);
	MakeKeyEvent(&tEvent, XGE_KEY_END, 0);
	iResult = xgeXuiInputEvent(&pApp->tScrollInput, &tEvent);
	pApp->fScrollProbe = pApp->tScrollInput.fScrollX;
	pApp->tScrollCandidate = xgeXuiInputGetCandidateRect(&pApp->tScrollInput);
	pApp->bScrollOK =
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->fScrollProbe > 0.0f) &&
		(pApp->tScrollCandidate.fX >= (pApp->pScrollInputWidget->tContentRect.fX - 1.0f)) &&
		((pApp->tScrollCandidate.fX + pApp->tScrollCandidate.fW) <= (pApp->pScrollInputWidget->tContentRect.fX + pApp->pScrollInputWidget->tContentRect.fW + 2.0f));

	xgeXuiSetFocus(&pApp->tXui, pApp->pPasswordInputWidget);
	MakeKeyEvent(&tEvent, 'A', XGE_KEY_MOD_CTRL);
	iResult = xgeXuiInputEvent(&pApp->tPasswordInput, &tEvent);
	MakeKeyEvent(&tEvent, 'C', XGE_KEY_MOD_CTRL);
	iResult = (iResult == XGE_XUI_EVENT_CONSUMED) ? xgeXuiInputEvent(&pApp->tPasswordInput, &tEvent) : iResult;
	pApp->bPasswordOK =
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tPasswordInput.bPassword != 0) &&
		(strcmp(xgeXuiInputGetText(&pApp->tPasswordInput), "Secret42") == 0) &&
		(pApp->tPasswordInput.tText.iSelectStart == 0) &&
		(pApp->tPasswordInput.tText.iSelectEnd == (int)strlen("Secret42"));

	xgeXuiSetFocus(&pApp->tXui, pApp->pReadonlyInputWidget);
	MakeKeyEvent(&tEvent, 'A', XGE_KEY_MOD_CTRL);
	iResult = xgeXuiInputEvent(&pApp->tReadonlyInput, &tEvent);
	MakeTextEvent(&tEvent, 'Z');
	iResult = (iResult == XGE_XUI_EVENT_CONSUMED) ? xgeXuiInputEvent(&pApp->tReadonlyInput, &tEvent) : iResult;
	MakeKeyEvent(&tEvent, XGE_KEY_BACKSPACE, 0);
	iResult = (iResult == XGE_XUI_EVENT_CONSUMED) ? xgeXuiInputEvent(&pApp->tReadonlyInput, &tEvent) : iResult;
	xgeXuiInputGetSelection(&pApp->tReadonlyInput, &pApp->iReadonlySelectStart, &pApp->iReadonlySelectEnd);
	pApp->bReadonlyOK =
		(iResult == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tReadonlyInput), "locked field") == 0) &&
		(pApp->iReadonlySelectStart == 0) &&
		(pApp->iReadonlySelectEnd == (int)strlen("locked field"));

	xgeXuiSetFocus(&pApp->tXui, pApp->pScrollInputWidget);
	tCenter = WidgetCenter(pApp->pDisabledInputWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	(void)xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bDisabledOK =
		(xgeXuiWidgetIsEnabled(pApp->pDisabledInputWidget) == 0) &&
		(pApp->tXui.pFocus == pApp->pScrollInputWidget) &&
		(strcmp(xgeXuiInputGetText(&pApp->tDisabledInput), "disabled field") == 0);

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
	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	return RunChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiInputUnit(&pApp->tDisabledInput);
	xgeXuiInputUnit(&pApp->tReadonlyInput);
	xgeXuiInputUnit(&pApp->tPasswordInput);
	xgeXuiInputUnit(&pApp->tScrollInput);
	xgeXuiInputUnit(&pApp->tPlaceholderInput);
	xgeXuiLabelUnit(&pApp->tDisabledLabel);
	xgeXuiLabelUnit(&pApp->tReadonlyLabel);
	xgeXuiLabelUnit(&pApp->tPasswordLabel);
	xgeXuiLabelUnit(&pApp->tScrollLabel);
	xgeXuiLabelUnit(&pApp->tPlaceholderLabel);
	xgeXuiLabelUnit(&pApp->tHintLabel);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
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
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-input-policy-lab final-summary frames=%d init=%d placeholder=%d scroll=%d password=%d readonly=%d disabled=%d scroll=%.2f cand=%.2f readonly=%d..%d password_len=%d focus=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bPlaceholderOK,
			pApp->bScrollOK,
			pApp->bPasswordOK,
			pApp->bReadonlyOK,
			pApp->bDisabledOK,
			pApp->fScrollProbe,
			pApp->tScrollCandidate.fX,
			pApp->iReadonlySelectStart,
			pApp->iReadonlySelectEnd,
			(int)strlen(xgeXuiInputGetText(&pApp->tPasswordInput)),
			pApp->tXui.pFocus == pApp->pScrollInputWidget);
		printf("xui-input-policy-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.iWidth = 820;
	tDesc.iHeight = 430;
	tDesc.sTitle = "XGE XUI Input Policy Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		AppUnit(&tApp);
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	iExitCode =
		(tApp.bInitOK && tApp.bPlaceholderOK && tApp.bScrollOK &&
		 tApp.bPasswordOK && tApp.bReadonlyOK && tApp.bDisabledOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

