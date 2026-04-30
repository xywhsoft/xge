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
	int bTextCoreOK;
	int bInputDeleteOK;
	int bEditDeleteOK;
	int bUndoRedoOK;
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

static void MakeKeyEvent(xge_event_t* pEvent, int iKey, int iMods)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
	pEvent->iParam2 = iMods;
}

static void MakeTextEvent(xge_event_t* pEvent, uint32_t iCodepoint)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_TEXT;
	pEvent->iCodepoint = iCodepoint;
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
			printf("xui-text-delete-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-text-delete-lab font load failed\n");
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
	if ( tRoot.fH < 420.0f ) {
		tRoot.fH = 420.0f;
	}
	fInnerW = tRoot.fW - 40.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pInputCaptionWidget, (xge_rect_t){ 20.0f, 58.0f, fInnerW, 22.0f });
	xgeXuiWidgetSetRect(pApp->pInputWidget, (xge_rect_t){ 20.0f, 84.0f, fInnerW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pEditCaptionWidget, (xge_rect_t){ 20.0f, 144.0f, fInnerW, 22.0f });
	xgeXuiWidgetSetRect(pApp->pEditWidget, (xge_rect_t){ 20.0f, 170.0f, fInnerW, tRoot.fH - 240.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 20.0f, tRoot.fH - 48.0f, fInnerW, 24.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sStatus[256];
	char sInput[192];
	char sEdit[192];
	int iStart;
	int iEnd;

	snprintf(
		sStatus,
		sizeof(sStatus),
		"text=%d input=%d edit=%d undo=%d",
		pApp->bTextCoreOK,
		pApp->bInputDeleteOK,
		pApp->bEditDeleteOK,
		pApp->bUndoRedoOK);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sStatus);

	xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
	snprintf(
		sInput,
		sizeof(sInput),
		"Input: cursor=%d selection=%d..%d text=\"%s\"",
		xgeXuiTextGetCursor(&pApp->tInput.tText),
		iStart,
		iEnd,
		xgeXuiInputGetText(&pApp->tInput));
	xgeXuiLabelSetText(&pApp->tInputCaption, sInput);

	xgeXuiTextGetSelection(&pApp->tEdit.tText, &iStart, &iEnd);
	snprintf(
		sEdit,
		sizeof(sEdit),
		"TextEdit: cursor=%d selection=%d..%d text=\"%s\"",
		xgeXuiTextGetCursor(&pApp->tEdit.tText),
		iStart,
		iEnd,
		xgeXuiTextEditGetText(&pApp->tEdit));
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
	tTheme.iTextColor = XGE_COLOR_RGBA(238, 242, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(34, 42, 56, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 96, 118, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(255, 166, 86, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(50, 64, 84, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(68, 84, 108, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(44, 58, 78, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(86, 60, 44, 255);
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

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui text delete lab") != XGE_OK ) {
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
	xgeXuiInputSetText(&pApp->tInput, "ABCD");

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
	xgeXuiTextEditSetWordWrap(&pApp->tEdit, 0);
	xgeXuiTextEditSetText(&pApp->tEdit, "ab\ncd");

	if ( xgeXuiLabelInit(&pApp->tHint, pApp->pHintWidget, pFont, "Auto-check runs on startup. You can still inspect deleted ranges, merged lines and undo/redo before exit.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tHint, XGE_COLOR_RGBA(164, 182, 206, 255));
	return XGE_OK;
}

static int RunCoreTextChecks(app_state_t* pApp)
{
	xge_xui_text_t tText;

	memset(&tText, 0, sizeof(tText));
	if ( xgeXuiTextInit(&tText) != XGE_OK ) {
		return XGE_ERROR;
	}

	pApp->bTextCoreOK = (xgeXuiTextSet(&tText, "ABC") == XGE_OK);
	xgeXuiTextSetSelection(&tText, 1, 3);
	pApp->bTextCoreOK =
		pApp->bTextCoreOK &&
		(xgeXuiTextInsert(&tText, "D") == XGE_OK) &&
		(strcmp(tText.sText, "AD") == 0) &&
		(xgeXuiTextGetCursor(&tText) == 2);
	pApp->bTextCoreOK =
		pApp->bTextCoreOK &&
		(xgeXuiTextSet(&tText, "ABC") == XGE_OK);
	xgeXuiTextSetCursor(&tText, 1);
	pApp->bTextCoreOK =
		pApp->bTextCoreOK &&
		(xgeXuiTextDeleteForward(&tText) == XGE_OK) &&
		(strcmp(tText.sText, "AC") == 0) &&
		(xgeXuiTextGetCursor(&tText) == 1);
	pApp->bTextCoreOK =
		pApp->bTextCoreOK &&
		(xgeXuiTextSet(&tText, "A\xe4\xb8\xad") == XGE_OK) &&
		(xgeXuiTextDeleteBack(&tText) == XGE_OK) &&
		(strcmp(tText.sText, "A") == 0);

	xgeXuiTextUnit(&tText);
	return XGE_OK;
}

static int RunWidgetChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	xgeXuiSetFocus(&pApp->tXui, pApp->pInputWidget);
	xgeXuiInputSetText(&pApp->tInput, "ABCD");
	xgeXuiInputSetSelection(&pApp->tInput, 1, 3);
	MakeTextEvent(&tEvent, 'Z');
	pApp->bInputDeleteOK =
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "AZD") == 0);

	xgeXuiInputSetText(&pApp->tInput, "AB");
	xgeXuiInputSetSelection(&pApp->tInput, 1, 1);
	MakeKeyEvent(&tEvent, XGE_KEY_DELETE, 0);
	pApp->bInputDeleteOK =
		pApp->bInputDeleteOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "A") == 0);

	xgeXuiInputSetText(&pApp->tInput, "AB");
	xgeXuiTextSetCursor(&pApp->tInput.tText, 2);
	MakeKeyEvent(&tEvent, XGE_KEY_BACKSPACE, 0);
	pApp->bInputDeleteOK =
		pApp->bInputDeleteOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "A") == 0) &&
		(xgeXuiTextGetCursor(&pApp->tInput.tText) == 1);

	xgeXuiSetFocus(&pApp->tXui, pApp->pEditWidget);
	xgeXuiTextEditSetText(&pApp->tEdit, "ab\ncd");
	xgeXuiTextSetSelection(&pApp->tEdit.tText, 1, 4);
	MakeTextEvent(&tEvent, 'Z');
	pApp->bEditDeleteOK =
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "aZd") == 0);

	xgeXuiTextEditSetText(&pApp->tEdit, "ab\ncd");
	xgeXuiTextSetCursor(&pApp->tEdit.tText, 3);
	MakeKeyEvent(&tEvent, XGE_KEY_BACKSPACE, 0);
	pApp->bEditDeleteOK =
		pApp->bEditDeleteOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "abcd") == 0);

	xgeXuiTextEditSetText(&pApp->tEdit, "AB");
	xgeXuiTextSetCursor(&pApp->tEdit.tText, 1);
	MakeKeyEvent(&tEvent, XGE_KEY_DELETE, 0);
	pApp->bEditDeleteOK =
		pApp->bEditDeleteOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "A") == 0) &&
		(xgeXuiTextGetCursor(&pApp->tEdit.tText) == 1);

	xgeXuiTextEditSetText(&pApp->tEdit, "ab\ncd");
	xgeXuiTextSetCursor(&pApp->tEdit.tText, 5);
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER, 0);
	(void)xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeTextEvent(&tEvent, 'e');
	(void)xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bUndoRedoOK =
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "ab\ncd\ne") == 0) &&
		(xgeXuiTextEditUndo(&pApp->tEdit) == XGE_OK) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "ab\ncd\n") == 0) &&
		(xgeXuiTextEditRedo(&pApp->tEdit) == XGE_OK) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "ab\ncd\ne") == 0);

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
	if ( RunCoreTextChecks(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	return RunWidgetChecks(pApp);
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
	int iInputStart;
	int iInputEnd;
	int iEditStart;
	int iEditEnd;

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
		xgeXuiInputGetSelection(&pApp->tInput, &iInputStart, &iInputEnd);
		xgeXuiTextGetSelection(&pApp->tEdit.tText, &iEditStart, &iEditEnd);
		printf(
			"xui-text-delete-lab final-summary frames=%d text=%d input=%d edit=%d undo=%d input(cursor=%d select=%d..%d text=%s) edit(cursor=%d select=%d..%d text=%s undo=%d redo=%d)\n",
			pApp->iFrameCount,
			pApp->bTextCoreOK,
			pApp->bInputDeleteOK,
			pApp->bEditDeleteOK,
			pApp->bUndoRedoOK,
			xgeXuiTextGetCursor(&pApp->tInput.tText),
			iInputStart,
			iInputEnd,
			xgeXuiInputGetText(&pApp->tInput),
			xgeXuiTextGetCursor(&pApp->tEdit.tText),
			iEditStart,
			iEditEnd,
			xgeXuiTextEditGetText(&pApp->tEdit),
			pApp->tEdit.iUndoCount,
			pApp->tEdit.iRedoCount);
		printf("xui-text-delete-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.iHeight = 460;
	tDesc.sTitle = "XGE XUI Text Delete Lab";
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
		(tApp.bTextCoreOK && tApp.bInputDeleteOK && tApp.bEditDeleteOK && tApp.bUndoRedoOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

