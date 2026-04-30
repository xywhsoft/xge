#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pInputCaptionWidget;
	xge_xui_widget pInputWidget;
	xge_xui_widget pEditCaptionWidget;
	xge_xui_widget pEditWidget;
	xge_xui_widget pHintWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tInputCaption;
	xge_xui_label_t tEditCaption;
	xge_xui_label_t tHint;
	xge_xui_input_t tInput;
	xge_xui_text_edit_t tEdit;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bClipboardReady;
	int bInputFlagsOK;
	int bInputActionsOK;
	int bInputGuardOK;
	int bEditFlagsOK;
	int bEditActionsOK;
	int bEditGuardOK;
	int iInputMenus;
	int iEditMenus;
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

static xge_vec2_t ContentPoint(xge_xui_widget pWidget, float fRatioX, float fRatioY)
{
	xge_vec2_t tPoint;

	tPoint.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW * fRatioX;
	tPoint.fY = pWidget->tContentRect.fY + pWidget->tContentRect.fH * fRatioY;
	return tPoint;
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
			printf("xui-text-menu-detail-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-text-menu-detail-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	float fInnerW;
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
	if ( tRoot.fW < 700.0f ) {
		tRoot.fW = 700.0f;
	}
	if ( tRoot.fH < 400.0f ) {
		tRoot.fH = 400.0f;
	}
	fInnerW = tRoot.fW - 40.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pInputCaptionWidget, (xge_rect_t){ 20.0f, 58.0f, fInnerW, 22.0f });
	xgeXuiWidgetSetRect(pApp->pInputWidget, (xge_rect_t){ 20.0f, 84.0f, fInnerW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pEditCaptionWidget, (xge_rect_t){ 20.0f, 144.0f, fInnerW, 22.0f });
	xgeXuiWidgetSetRect(pApp->pEditWidget, (xge_rect_t){ 20.0f, 170.0f, fInnerW, 124.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 20.0f, tRoot.fH - 48.0f, fInnerW, 24.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sStatus[256];
	char sInput[192];
	char sEdit[192];

	snprintf(
		sStatus,
		sizeof(sStatus),
		"input_flags=%d input_actions=%d input_guard=%d edit_flags=%d edit_actions=%d edit_guard=%d menus=%d/%d",
		pApp->bInputFlagsOK,
		pApp->bInputActionsOK,
		pApp->bInputGuardOK,
		pApp->bEditFlagsOK,
		pApp->bEditActionsOK,
		pApp->bEditGuardOK,
		pApp->iInputMenus,
		pApp->iEditMenus);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sStatus);

	snprintf(sInput, sizeof(sInput), "Input: \"%s\"", xgeXuiInputGetText(&pApp->tInput));
	snprintf(sEdit, sizeof(sEdit), "TextEdit: \"%s\"", xgeXuiTextEditGetText(&pApp->tEdit));
	xgeXuiLabelSetText(&pApp->tInputCaption, sInput);
	xgeXuiLabelSetText(&pApp->tEditCaption, sEdit);
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
	tTheme.iTextColor = XGE_COLOR_RGBA(236, 241, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(34, 42, 56, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 96, 118, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(86, 166, 255, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(52, 64, 84, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(72, 88, 112, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(44, 58, 78, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(54, 74, 104, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(78, 82, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pInputCaptionWidget = xgeXuiWidgetCreate();
	pApp->pInputWidget = xgeXuiWidgetCreate();
	pApp->pEditCaptionWidget = xgeXuiWidgetCreate();
	pApp->pEditWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pInputCaptionWidget == NULL) ||
	     (pApp->pInputWidget == NULL) || (pApp->pEditCaptionWidget == NULL) || (pApp->pEditWidget == NULL) ||
	     (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pInputCaptionWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pInputWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pEditCaptionWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pEditWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pInputWidget, 8.0f, 7.0f, 8.0f, 7.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pEditWidget, 8.0f, 8.0f, 8.0f, 8.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui text menu detail lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tInputCaption, pApp->pInputCaptionWidget, pFont, "Input") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tInputCaption, XGE_COLOR_RGBA(188, 204, 224, 255));

	if ( xgeXuiInputInit(&pApp->tInput, &pApp->tXui, pApp->pInputWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiInputSetColors(
		&pApp->tInput,
		XGE_COLOR_RGBA(242, 246, 252, 255),
		XGE_COLOR_RGBA(28, 36, 48, 255),
		XGE_COLOR_RGBA(42, 58, 82, 255),
		XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiInputSetText(&pApp->tInput, "alpha beta");

	if ( xgeXuiLabelInit(&pApp->tEditCaption, pApp->pEditCaptionWidget, pFont, "TextEdit") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tEditCaption, XGE_COLOR_RGBA(188, 204, 224, 255));

	if ( xgeXuiTextEditInit(&pApp->tEdit, &pApp->tXui, pApp->pEditWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiTextEditSetColors(
		&pApp->tEdit,
		XGE_COLOR_RGBA(242, 246, 252, 255),
		XGE_COLOR_RGBA(26, 34, 46, 255),
		XGE_COLOR_RGBA(42, 58, 82, 255),
		XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiTextEditSetText(&pApp->tEdit, "ab\ncd");
	xgeXuiTextEditSetWordWrap(&pApp->tEdit, 0);

	if ( xgeXuiLabelInit(&pApp->tHint, pApp->pHintWidget, pFont, "Auto-check opens default menus, inspects enabled items, and drives SelectAll/Cut/Copy/Paste/Delete for Input and TextEdit.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tHint, XGE_COLOR_RGBA(164, 182, 206, 255));
	return XGE_OK;
}

static int OpenInputMenu(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tPoint;

	tPoint = ContentPoint(pApp->pInputWidget, 0.12f, 0.5f);
	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_CONTEXT_BEGIN, XGE_MOUSE_RIGHT, tPoint.fX, tPoint.fY);
	if ( xgeXuiInputEvent(&pApp->tInput, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return 0;
	}
	pApp->iInputMenus++;
	return xgeXuiMenuIsOpen(pApp->tInput.pDefaultMenu);
}

static int OpenEditMenu(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tPoint;

	tPoint = ContentPoint(pApp->pEditWidget, 0.12f, 0.18f);
	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_CONTEXT_BEGIN, XGE_MOUSE_RIGHT, tPoint.fX, tPoint.fY);
	if ( xgeXuiTextEditEvent(&pApp->tEdit, &tEvent) != XGE_XUI_EVENT_CONSUMED ) {
		return 0;
	}
	pApp->iEditMenus++;
	return xgeXuiMenuIsOpen(pApp->tEdit.pDefaultMenu);
}

static int RunChecks(app_state_t* pApp)
{
	int iStart;
	int iEnd;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiSetFocus(&pApp->tXui, pApp->pInputWidget);
	xgeClipboardSetText("PASTE");
	pApp->bClipboardReady = (xgeClipboardGetText() != NULL) && (xgeClipboardGetText()[0] != 0);

	xgeXuiInputSetReadonly(&pApp->tInput, 0);
	xgeXuiInputSetPassword(&pApp->tInput, 0);
	xgeXuiInputSetText(&pApp->tInput, "alpha beta");
	xgeXuiInputSetSelection(&pApp->tInput, 0, 5);
	pApp->bInputFlagsOK =
		OpenInputMenu(pApp) &&
		(pApp->tInput.arrDefaultMenuEnabled[0] == 1) &&
		(pApp->tInput.arrDefaultMenuEnabled[1] == 1) &&
		(pApp->tInput.arrDefaultMenuEnabled[2] == 1) &&
		(pApp->tInput.arrDefaultMenuEnabled[3] == pApp->bClipboardReady) &&
		(pApp->tInput.arrDefaultMenuEnabled[4] == 1);
	xgeXuiMenuClose(pApp->tInput.pDefaultMenu);

	pApp->tInput.pDefaultMenu->procSelect(pApp->pInputWidget, 2, pApp->tInput.pDefaultMenu->pUser);
	pApp->bInputActionsOK = 1;
	if ( pApp->bClipboardReady ) {
		pApp->bInputActionsOK = (strcmp(xgeClipboardGetText(), "alpha") == 0);
	}
	xgeXuiInputSetSelection(&pApp->tInput, 0, 5);
	pApp->tInput.pDefaultMenu->procSelect(pApp->pInputWidget, 1, pApp->tInput.pDefaultMenu->pUser);
	pApp->bInputActionsOK =
		pApp->bInputActionsOK &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), " beta") == 0);
	if ( pApp->bClipboardReady ) {
		pApp->tInput.pDefaultMenu->procSelect(pApp->pInputWidget, 3, pApp->tInput.pDefaultMenu->pUser);
		pApp->bInputActionsOK =
			pApp->bInputActionsOK &&
			(strcmp(xgeXuiInputGetText(&pApp->tInput), "alpha beta") == 0);
	} else {
		xgeXuiInputSetText(&pApp->tInput, "alpha beta");
	}
	xgeXuiInputSetSelection(&pApp->tInput, 0, 5);
	pApp->tInput.pDefaultMenu->procSelect(pApp->pInputWidget, 4, pApp->tInput.pDefaultMenu->pUser);
	pApp->bInputActionsOK =
		pApp->bInputActionsOK &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), " beta") == 0);
	pApp->tInput.pDefaultMenu->procSelect(pApp->pInputWidget, 0, pApp->tInput.pDefaultMenu->pUser);
	xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
	pApp->bInputActionsOK =
		pApp->bInputActionsOK &&
		(iStart == 0) &&
		(iEnd == (int)strlen(xgeXuiInputGetText(&pApp->tInput)));

	xgeXuiInputSetReadonly(&pApp->tInput, 1);
	xgeXuiInputSetPassword(&pApp->tInput, 0);
	xgeXuiInputSetSelection(&pApp->tInput, 0, 1);
	pApp->bInputGuardOK =
		OpenInputMenu(pApp) &&
		(pApp->tInput.arrDefaultMenuEnabled[0] == 1) &&
		(pApp->tInput.arrDefaultMenuEnabled[1] == 0) &&
		(pApp->tInput.arrDefaultMenuEnabled[2] == 1) &&
		(pApp->tInput.arrDefaultMenuEnabled[3] == 0) &&
		(pApp->tInput.arrDefaultMenuEnabled[4] == 0);
	xgeXuiMenuClose(pApp->tInput.pDefaultMenu);
	xgeXuiInputSetReadonly(&pApp->tInput, 0);
	xgeXuiInputSetPassword(&pApp->tInput, 1);
	pApp->bInputGuardOK =
		pApp->bInputGuardOK &&
		(OpenInputMenu(pApp) == 0) &&
		(xgeXuiMenuIsOpen(pApp->tInput.pDefaultMenu) == 0);
	xgeXuiInputSetPassword(&pApp->tInput, 0);

	xgeXuiSetFocus(&pApp->tXui, pApp->pEditWidget);
	xgeXuiTextEditSetReadonly(&pApp->tEdit, 0);
	xgeXuiTextEditSetText(&pApp->tEdit, "ab\ncd");
	xgeXuiTextSetSelection(&pApp->tEdit.tText, 0, 2);
	pApp->bEditFlagsOK =
		OpenEditMenu(pApp) &&
		(pApp->tEdit.arrDefaultMenuEnabled[0] == 1) &&
		(pApp->tEdit.arrDefaultMenuEnabled[1] == 1) &&
		(pApp->tEdit.arrDefaultMenuEnabled[2] == 1) &&
		(pApp->tEdit.arrDefaultMenuEnabled[3] == pApp->bClipboardReady) &&
		(pApp->tEdit.arrDefaultMenuEnabled[4] == 1);
	xgeXuiMenuClose(pApp->tEdit.pDefaultMenu);

	pApp->tEdit.pDefaultMenu->procSelect(pApp->pEditWidget, 2, pApp->tEdit.pDefaultMenu->pUser);
	pApp->bEditActionsOK = 1;
	if ( pApp->bClipboardReady ) {
		pApp->bEditActionsOK = (strcmp(xgeClipboardGetText(), "ab") == 0);
	}
	xgeXuiTextSetSelection(&pApp->tEdit.tText, 0, 2);
	pApp->tEdit.pDefaultMenu->procSelect(pApp->pEditWidget, 1, pApp->tEdit.pDefaultMenu->pUser);
	pApp->bEditActionsOK =
		pApp->bEditActionsOK &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "\ncd") == 0);
	if ( pApp->bClipboardReady ) {
		pApp->tEdit.pDefaultMenu->procSelect(pApp->pEditWidget, 3, pApp->tEdit.pDefaultMenu->pUser);
		pApp->bEditActionsOK =
			pApp->bEditActionsOK &&
			(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "ab\ncd") == 0);
	} else {
		xgeXuiTextEditSetText(&pApp->tEdit, "ab\ncd");
	}
	xgeXuiTextSetSelection(&pApp->tEdit.tText, 0, 2);
	pApp->tEdit.pDefaultMenu->procSelect(pApp->pEditWidget, 4, pApp->tEdit.pDefaultMenu->pUser);
	pApp->bEditActionsOK =
		pApp->bEditActionsOK &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "\ncd") == 0);
	pApp->tEdit.pDefaultMenu->procSelect(pApp->pEditWidget, 0, pApp->tEdit.pDefaultMenu->pUser);
	xgeXuiTextGetSelection(&pApp->tEdit.tText, &iStart, &iEnd);
	pApp->bEditActionsOK =
		pApp->bEditActionsOK &&
		(iStart == 0) &&
		(iEnd == pApp->tEdit.tText.iSize);

	xgeXuiTextEditSetReadonly(&pApp->tEdit, 1);
	xgeXuiTextEditSetText(&pApp->tEdit, "ab\ncd");
	xgeXuiTextSetSelection(&pApp->tEdit.tText, 0, 2);
	pApp->bEditGuardOK =
		OpenEditMenu(pApp) &&
		(pApp->tEdit.arrDefaultMenuEnabled[0] == 1) &&
		(pApp->tEdit.arrDefaultMenuEnabled[1] == 0) &&
		(pApp->tEdit.arrDefaultMenuEnabled[2] == 1) &&
		(pApp->tEdit.arrDefaultMenuEnabled[3] == 0) &&
		(pApp->tEdit.arrDefaultMenuEnabled[4] == 0);
	xgeXuiMenuClose(pApp->tEdit.pDefaultMenu);
	xgeXuiTextEditSetReadonly(&pApp->tEdit, 0);

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
	xgeXuiTextEditUnit(&pApp->tEdit);
	xgeXuiInputUnit(&pApp->tInput);
	xgeXuiLabelUnit(&pApp->tHint);
	xgeXuiLabelUnit(&pApp->tEditCaption);
	xgeXuiLabelUnit(&pApp->tInputCaption);
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
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-text-menu-detail-lab final-summary frames=%d input_flags=%d input_actions=%d input_guard=%d edit_flags=%d edit_actions=%d edit_guard=%d menus=%d/%d input=%s edit=%s\n",
			pApp->iFrameCount,
			pApp->bInputFlagsOK,
			pApp->bInputActionsOK,
			pApp->bInputGuardOK,
			pApp->bEditFlagsOK,
			pApp->bEditActionsOK,
			pApp->bEditGuardOK,
			pApp->iInputMenus,
			pApp->iEditMenus,
			xgeXuiInputGetText(&pApp->tInput),
			xgeXuiTextEditGetText(&pApp->tEdit));
		printf("xui-text-menu-detail-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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

	iFrameLimit = 0;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			iFrameLimit = ArgInt(argv[i + 1], iFrameLimit);
			i++;
		}
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 760;
	tDesc.iHeight = 420;
	tDesc.sTitle = "XGE XUI Text Menu Detail Lab";
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
		(tApp.bInputFlagsOK && tApp.bInputActionsOK && tApp.bInputGuardOK &&
		 tApp.bEditFlagsOK && tApp.bEditActionsOK && tApp.bEditGuardOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

