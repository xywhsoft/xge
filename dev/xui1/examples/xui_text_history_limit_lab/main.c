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
	xge_xui_widget pCaptionWidget;
	xge_xui_widget pEditWidget;
	xge_xui_widget pHintWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tCaption;
	xge_xui_label_t tHint;
	xge_xui_text_edit_t tEdit;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bLimitOK;
	int bUndoBoundOK;
	int bRedoBoundOK;
	int bResetOK;
	int bKeyboardOK;
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
			printf("xui-text-history-limit-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-text-history-limit-lab font load failed\n");
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
	if ( tRoot.fW < 720.0f ) {
		tRoot.fW = 720.0f;
	}
	if ( tRoot.fH < 420.0f ) {
		tRoot.fH = 420.0f;
	}
	fInnerW = tRoot.fW - 40.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pCaptionWidget, (xge_rect_t){ 20.0f, 58.0f, fInnerW, 44.0f });
	xgeXuiWidgetSetRect(pApp->pEditWidget, (xge_rect_t){ 20.0f, 112.0f, fInnerW, tRoot.fH - 176.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 20.0f, tRoot.fH - 46.0f, fInnerW, 24.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sStatus[256];
	char sCaption[320];

	snprintf(
		sStatus,
		sizeof(sStatus),
		"limit=%d undo=%d redo=%d reset=%d keyboard=%d",
		pApp->bLimitOK,
		pApp->bUndoBoundOK,
		pApp->bRedoBoundOK,
		pApp->bResetOK,
		pApp->bKeyboardOK);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sStatus);

	snprintf(
		sCaption,
		sizeof(sCaption),
		"TextEdit: limit=%d undo=%d redo=%d cursor=%d text=\"%s\"",
		pApp->tEdit.iUndoLimit,
		pApp->tEdit.iUndoCount,
		pApp->tEdit.iRedoCount,
		xgeXuiTextGetCursor(&pApp->tEdit.tText),
		xgeXuiTextEditGetText(&pApp->tEdit));
	xgeXuiLabelSetText(&pApp->tCaption, sCaption);
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
	tTheme.iAccentColor = XGE_COLOR_RGBA(108, 190, 168, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(50, 64, 84, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(68, 84, 108, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(44, 58, 78, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(44, 88, 86, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(78, 82, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pCaptionWidget = xgeXuiWidgetCreate();
	pApp->pEditWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pCaptionWidget == NULL) ||
	     (pApp->pEditWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pCaptionWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pEditWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pEditWidget, 8.0f, 8.0f, 8.0f, 8.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui text history limit lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tCaption, pApp->pCaptionWidget, pFont, "TextEdit") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tCaption, XGE_COLOR_RGBA(188, 204, 224, 255));

	if ( xgeXuiTextEditInit(&pApp->tEdit, &pApp->tXui, pApp->pEditWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiTextEditSetColors(
		&pApp->tEdit,
		XGE_COLOR_RGBA(242, 246, 252, 255),
		XGE_COLOR_RGBA(26, 34, 46, 255),
		XGE_COLOR_RGBA(42, 72, 76, 255),
		XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiTextEditSetWordWrap(&pApp->tEdit, 0);
	xgeXuiTextEditSetText(&pApp->tEdit, "A");

	if ( xgeXuiLabelInit(&pApp->tHint, pApp->pHintWidget, pFont, "Auto-check drives a small undo-limit scenario. You can visually inspect the final text and status before exit.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tHint, XGE_COLOR_RGBA(164, 182, 206, 255));
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiSetFocus(&pApp->tXui, pApp->pEditWidget);

	pApp->tEdit.iUndoLimit = 2;
	pApp->bLimitOK =
		(pApp->tEdit.iUndoLimit == 2) &&
		(pApp->tEdit.iUndoCount == 0) &&
		(pApp->tEdit.iRedoCount == 0);

	MakeTextEvent(&tEvent, 'B');
	pApp->bLimitOK =
		pApp->bLimitOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	MakeTextEvent(&tEvent, 'C');
	pApp->bLimitOK =
		pApp->bLimitOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	MakeTextEvent(&tEvent, 'D');
	pApp->bLimitOK =
		pApp->bLimitOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "ABCD") == 0) &&
		(pApp->tEdit.iUndoCount == 2) &&
		(pApp->tEdit.iRedoCount == 0);

	pApp->bUndoBoundOK =
		(xgeXuiTextEditUndo(&pApp->tEdit) == XGE_OK) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "ABC") == 0) &&
		(xgeXuiTextEditUndo(&pApp->tEdit) == XGE_OK) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "AB") == 0) &&
		(pApp->tEdit.iUndoCount == 0) &&
		(pApp->tEdit.iRedoCount == 2) &&
		(xgeXuiTextEditUndo(&pApp->tEdit) != XGE_OK) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "AB") == 0);

	pApp->bRedoBoundOK =
		(xgeXuiTextEditRedo(&pApp->tEdit) == XGE_OK) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "ABC") == 0) &&
		(xgeXuiTextEditRedo(&pApp->tEdit) == XGE_OK) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "ABCD") == 0) &&
		(pApp->tEdit.iUndoCount == 2) &&
		(pApp->tEdit.iRedoCount == 0) &&
		(xgeXuiTextEditRedo(&pApp->tEdit) != XGE_OK) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "ABCD") == 0);

	xgeXuiTextEditSetText(&pApp->tEdit, "xy");
	pApp->bResetOK =
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "xy") == 0) &&
		(pApp->tEdit.iUndoCount == 0) &&
		(pApp->tEdit.iRedoCount == 0) &&
		(xgeXuiTextEditUndo(&pApp->tEdit) != XGE_OK);

	pApp->tEdit.iUndoLimit = 1;
	xgeXuiSetFocus(&pApp->tXui, pApp->pEditWidget);
	MakeTextEvent(&tEvent, '1');
	pApp->bKeyboardOK =
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	MakeTextEvent(&tEvent, '2');
	pApp->bKeyboardOK =
		pApp->bKeyboardOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "xy12") == 0) &&
		(pApp->tEdit.iUndoCount == 1);

	MakeKeyEvent(&tEvent, 'Z', XGE_KEY_MOD_CTRL);
	pApp->bKeyboardOK =
		pApp->bKeyboardOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "xy1") == 0) &&
		(pApp->tEdit.iRedoCount == 1);
	MakeKeyEvent(&tEvent, 'Y', XGE_KEY_MOD_CTRL);
	pApp->bKeyboardOK =
		pApp->bKeyboardOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "xy12") == 0) &&
		(pApp->tEdit.iUndoCount == 1) &&
		(pApp->tEdit.iRedoCount == 0);

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
	xgeXuiLabelUnit(&pApp->tHint);
	xgeXuiLabelUnit(&pApp->tCaption);
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
			"xui-text-history-limit-lab final-summary frames=%d limit=%d undo=%d redo=%d reset=%d keyboard=%d limit=%d undo=%d redo=%d cursor=%d text=%s\n",
			pApp->iFrameCount,
			pApp->bLimitOK,
			pApp->bUndoBoundOK,
			pApp->bRedoBoundOK,
			pApp->bResetOK,
			pApp->bKeyboardOK,
			pApp->tEdit.iUndoLimit,
			pApp->tEdit.iUndoCount,
			pApp->tEdit.iRedoCount,
			xgeXuiTextGetCursor(&pApp->tEdit.tText),
			xgeXuiTextEditGetText(&pApp->tEdit));
		printf("xui-text-history-limit-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.sTitle = "XGE XUI Text History Limit Lab";
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
		(tApp.bLimitOK && tApp.bUndoBoundOK && tApp.bRedoBoundOK && tApp.bResetOK && tApp.bKeyboardOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

