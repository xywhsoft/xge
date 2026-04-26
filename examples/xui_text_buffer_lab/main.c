#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFontPrimary;
	xge_font_t tFontAlt;
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
	int bFontsReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bTextCoreOK;
	int bTextEventOK;
	int bFontSetOK;
	int bEditEventOK;
	int bImeOK;
	int bCandidateOK;
	int iDirectCursor;
	int iDirectSelectionStart;
	int iDirectSelectionEnd;
	xge_rect_t tInputCandidate;
	xge_rect_t tEditCandidate;
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
	float fDelta;

	fDelta = fA - fB;
	if ( fDelta < 0.0f ) {
		fDelta = -fDelta;
	}
	return fDelta <= fEpsilon;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static void MakeTextEvent(xge_event_t* pEvent, uint32_t iCodepoint)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_TEXT;
	pEvent->iCodepoint = iCodepoint;
}

static void MakeImeEvent(xge_event_t* pEvent, int iType, xge_ime_event_t* pIme, const char* sText)
{
	memset(pEvent, 0, sizeof(*pEvent));
	memset(pIme, 0, sizeof(*pIme));
	pEvent->iType = iType;
	if ( (iType == XGE_EVENT_IME_UPDATE) && (pIme != NULL) ) {
		pIme->sText = sText;
		pEvent->pData = pIme;
	}
}

static int LoadFonts(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simhei.ttf",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFontPrimary, 0, sizeof(pApp->tFontPrimary));
		memset(&pApp->tFontAlt, 0, sizeof(pApp->tFontAlt));
		if ( xgeFontLoad(&pApp->tFontPrimary, arrFonts[i], 18.0f) != XGE_OK ) {
			continue;
		}
		if ( xgeFontLoad(&pApp->tFontAlt, arrFonts[i], 26.0f) != XGE_OK ) {
			xgeFontFree(&pApp->tFontPrimary);
			memset(&pApp->tFontPrimary, 0, sizeof(pApp->tFontPrimary));
			continue;
		}
		pApp->bFontsReady = 1;
		printf("xui-text-buffer-lab fonts loaded: %s\n", arrFonts[i]);
		return XGE_OK;
	}
	printf("xui-text-buffer-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
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
	if ( tRoot.fH < 360.0f ) {
		tRoot.fH = 360.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 42.0f });
	xgeXuiWidgetSetRect(pApp->pInputCaptionWidget, (xge_rect_t){ 0.0f, 58.0f, tRoot.fW, 26.0f });
	xgeXuiWidgetSetRect(pApp->pInputWidget, (xge_rect_t){ 0.0f, 90.0f, tRoot.fW, 42.0f });
	xgeXuiWidgetSetRect(pApp->pEditCaptionWidget, (xge_rect_t){ 0.0f, 150.0f, tRoot.fW, 26.0f });
	xgeXuiWidgetSetRect(pApp->pEditWidget, (xge_rect_t){ 0.0f, 182.0f, tRoot.fW, tRoot.fH - 250.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 0.0f, tRoot.fH - 46.0f, tRoot.fW, 26.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"text=%d event=%d font=%d edit=%d ime=%d candidate=%d cursor=%d select=%d..%d",
		pApp->bTextCoreOK,
		pApp->bTextEventOK,
		pApp->bFontSetOK,
		pApp->bEditEventOK,
		pApp->bImeOK,
		pApp->bCandidateOK,
		pApp->iDirectCursor,
		pApp->iDirectSelectionStart,
		pApp->iDirectSelectionEnd);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontsReady ? &pApp->tFontPrimary : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(240, 244, 250, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(18, 24, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(28, 36, 48, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(88, 100, 124, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(98, 200, 255, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(68, 92, 126, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(46, 68, 104, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(80, 84, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

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

	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pInputCaptionWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pInputWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pEditCaptionWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pEditWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(18, 24, 34, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(42, 54, 72, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui text buffer lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tInputCaption, pApp->pInputCaptionWidget, pFont, "Input: direct font setter and candidate rect") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tInputCaption, XGE_COLOR_RGBA(188, 204, 224, 255));

	xgeXuiWidgetSetPaddingPx(pApp->pInputWidget, 8.0f, 7.0f, 8.0f, 7.0f);
	if ( xgeXuiInputInit(&pApp->tInput, &pApp->tXui, pApp->pInputWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiInputSetColors(
		&pApp->tInput,
		XGE_COLOR_RGBA(242, 246, 252, 255),
		XGE_COLOR_RGBA(28, 36, 48, 255),
		XGE_COLOR_RGBA(42, 58, 82, 255),
		XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiInputSetPlaceholder(&pApp->tInput, "Input font setter check");
	xgeXuiInputSetText(&pApp->tInput, "input font");

	if ( xgeXuiLabelInit(&pApp->tEditCaption, pApp->pEditCaptionWidget, pFont, "TextEdit: direct xgeXuiText* and TextEditEvent checks") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tEditCaption, XGE_COLOR_RGBA(188, 204, 224, 255));

	xgeXuiWidgetSetPaddingPx(pApp->pEditWidget, 8.0f, 8.0f, 8.0f, 8.0f);
	if ( xgeXuiTextEditInit(&pApp->tEdit, &pApp->tXui, pApp->pEditWidget, pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiTextEditSetColors(
		&pApp->tEdit,
		XGE_COLOR_RGBA(242, 246, 252, 255),
		XGE_COLOR_RGBA(26, 34, 46, 255),
		XGE_COLOR_RGBA(42, 58, 82, 255),
		XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiTextEditSetText(&pApp->tEdit, "text buffer\nedit font");

	if ( xgeXuiLabelInit(&pApp->tHint, pApp->pHintWidget, pFont, "Auto-check runs on startup. You can still inspect caret height and line spacing before exit.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tHint, XGE_COLOR_RGBA(168, 184, 208, 255));
	return XGE_OK;
}

static int RunDirectTextChecks(app_state_t* pApp)
{
	xge_xui_text_t tText;
	xge_event_t tEvent;
	xge_ime_event_t tIme;
	int iStart;
	int iEnd;
	int bCore;
	int bEvent;

	memset(&tText, 0, sizeof(tText));
	if ( xgeXuiTextInit(&tText) != XGE_OK ) {
		return XGE_ERROR;
	}

	bCore = 1;
	bEvent = 1;
	bCore = bCore && (xgeXuiTextSet(&tText, "AB") == XGE_OK);
	xgeXuiTextSetCursor(&tText, 1);
	bCore = bCore && (xgeXuiTextInsert(&tText, "12") == XGE_OK) && (strcmp(tText.sText, "A12B") == 0);
	xgeXuiTextSetCursor(&tText, 3);
	bCore = bCore && (xgeXuiTextInsertCodepoint(&tText, 0x4E2D) == XGE_OK) && (strcmp(tText.sText, "A12" "\xE4\xB8\xAD" "B") == 0);
	xgeXuiTextSetCursor(&tText, 5);
	pApp->iDirectCursor = xgeXuiTextGetCursor(&tText);
	bCore = bCore && (pApp->iDirectCursor == 3);
	xgeXuiTextSetSelection(&tText, 6, 3);
	xgeXuiTextGetSelection(&tText, &iStart, &iEnd);
	pApp->iDirectSelectionStart = iStart;
	pApp->iDirectSelectionEnd = iEnd;
	bCore = bCore && (iStart == 3) && (iEnd == 6);
	bCore = bCore && (xgeXuiTextDeleteBack(&tText) == XGE_OK) && (strcmp(tText.sText, "A12B") == 0);
	xgeXuiTextSetCursor(&tText, 1);
	bCore = bCore && (xgeXuiTextDeleteForward(&tText) == XGE_OK) && (strcmp(tText.sText, "A2B") == 0);
	bCore = bCore && (xgeXuiTextSetComposition(&tText, "ime") == XGE_OK) && (strcmp(xgeXuiTextGetComposition(&tText), "ime") == 0);
	xgeXuiTextClearComposition(&tText);
	bCore = bCore && (xgeXuiTextGetComposition(&tText)[0] == 0);

	MakeTextEvent(&tEvent, 'Q');
	bEvent = bEvent && (xgeXuiTextInputEvent(&tText, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (strcmp(tText.sText, "AQ2B") == 0);
	MakeImeEvent(&tEvent, XGE_EVENT_IME_START, &tIme, NULL);
	bEvent = bEvent && (xgeXuiTextInputEvent(&tText, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	MakeImeEvent(&tEvent, XGE_EVENT_IME_UPDATE, &tIme, "ime");
	bEvent = bEvent && (xgeXuiTextInputEvent(&tText, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (strcmp(xgeXuiTextGetComposition(&tText), "ime") == 0);
	MakeImeEvent(&tEvent, XGE_EVENT_IME_END, &tIme, NULL);
	bEvent = bEvent && (xgeXuiTextInputEvent(&tText, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiTextGetComposition(&tText)[0] == 0);

	pApp->bTextCoreOK = bCore;
	pApp->bTextEventOK = bEvent;
	xgeXuiTextUnit(&tText);
	return XGE_OK;
}

static int RunWidgetChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_ime_event_t tIme;
	int iConsumed;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	xgeXuiInputSetFont(&pApp->tInput, &pApp->tFontAlt);
	xgeXuiTextEditSetFont(&pApp->tEdit, &pApp->tFontAlt);
	pApp->bFontSetOK =
		(pApp->tInput.pFont == &pApp->tFontAlt) &&
		(pApp->tEdit.pFont == &pApp->tFontAlt);

	xgeXuiSetFocus(&pApp->tXui, pApp->pInputWidget);
	xgeXuiTextSetCursor(&pApp->tInput.tText, 5);
	pApp->tInputCandidate = xgeXuiInputGetCandidateRect(&pApp->tInput);
	pApp->bCandidateOK =
		(pApp->tInputCandidate.fW >= 0.0f) &&
		(pApp->tInputCandidate.fH > 0.0f);

	xgeXuiTextEditSetText(&pApp->tEdit, "ab");
	xgeXuiSetFocus(&pApp->tXui, pApp->pEditWidget);
	MakeTextEvent(&tEvent, 'Q');
	iConsumed = xgeXuiTextEditEventProc(pApp->pEditWidget, &tEvent, &pApp->tEdit);
	pApp->bEditEventOK =
		(iConsumed == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "abQ") == 0) &&
		(pApp->pEditWidget->procEvent == xgeXuiTextEditEventProc) &&
		(pApp->pEditWidget->procUpdate == xgeXuiTextEditUpdateProc) &&
		(pApp->pEditWidget->procPaint == xgeXuiTextEditPaintProc);

	MakeKeyEvent(&tEvent, XGE_KEY_BACKSPACE);
	iConsumed = xgeXuiTextEditEvent(&pApp->tEdit, &tEvent);
	pApp->bEditEventOK = pApp->bEditEventOK &&
		(iConsumed == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "ab") == 0);

	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	iConsumed = xgeXuiTextEditEvent(&pApp->tEdit, &tEvent);
	pApp->bEditEventOK = pApp->bEditEventOK &&
		(iConsumed == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "ab\n") == 0);

	MakeImeEvent(&tEvent, XGE_EVENT_IME_START, &tIme, NULL);
	(void)xgeXuiTextEditEvent(&pApp->tEdit, &tEvent);
	MakeImeEvent(&tEvent, XGE_EVENT_IME_UPDATE, &tIme, "ime");
	iConsumed = xgeXuiTextEditEvent(&pApp->tEdit, &tEvent);
	pApp->tEditCandidate = xgeXuiTextEditGetCandidateRect(&pApp->tEdit);
	pApp->bImeOK =
		(iConsumed == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextGetComposition(&pApp->tEdit.tText), "ime") == 0);
	pApp->bCandidateOK = pApp->bCandidateOK &&
		(pApp->tEditCandidate.fW >= 0.0f) &&
		(pApp->tEditCandidate.fH > 0.0f);
	MakeImeEvent(&tEvent, XGE_EVENT_IME_END, &tIme, NULL);
	(void)xgeXuiTextEditEvent(&pApp->tEdit, &tEvent);
	pApp->bImeOK = pApp->bImeOK && (xgeXuiTextGetComposition(&pApp->tEdit.tText)[0] == 0);
	return XGE_OK;
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiTextEditUnit(&pApp->tEdit);
	xgeXuiInputUnit(&pApp->tInput);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	xgeXuiLabelUnit(&pApp->tInputCaption);
	xgeXuiLabelUnit(&pApp->tEditCaption);
	xgeXuiLabelUnit(&pApp->tHint);
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontsReady ) {
		xgeFontFree(&pApp->tFontAlt);
		xgeFontFree(&pApp->tFontPrimary);
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
	if ( LoadFonts(pApp) != XGE_OK ) {
		AppUnit(pApp);
		return XGE_ERROR;
	}
	if ( CreateUI(pApp) != XGE_OK ) {
		AppUnit(pApp);
		return XGE_ERROR;
	}
	if ( RunDirectTextChecks(pApp) != XGE_OK ) {
		AppUnit(pApp);
		return XGE_ERROR;
	}
	if ( RunWidgetChecks(pApp) != XGE_OK ) {
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
	xgeClear(XGE_COLOR_RGBA(14, 18, 24, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-text-buffer-lab final-summary frames=%d text=%d event=%d font=%d edit=%d ime=%d candidate=%d cursor=%d select=%d/%d input_font=%p edit_font=%p input_rect=%.1f,%.1f,%.1f,%.1f edit_rect=%.1f,%.1f,%.1f,%.1f text_len=%d\n",
			pApp->iFrameCount,
			pApp->bTextCoreOK,
			pApp->bTextEventOK,
			pApp->bFontSetOK,
			pApp->bEditEventOK,
			pApp->bImeOK,
			pApp->bCandidateOK,
			pApp->iDirectCursor,
			pApp->iDirectSelectionStart,
			pApp->iDirectSelectionEnd,
			(void*)pApp->tInput.pFont,
			(void*)pApp->tEdit.pFont,
			pApp->tInputCandidate.fX,
			pApp->tInputCandidate.fY,
			pApp->tInputCandidate.fW,
			pApp->tInputCandidate.fH,
			pApp->tEditCandidate.fX,
			pApp->tEditCandidate.fY,
			pApp->tEditCandidate.fW,
			pApp->tEditCandidate.fH,
			(int)strlen(xgeXuiTextEditGetText(&pApp->tEdit)));
		printf("xui-text-buffer-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.iHeight = 520;
	tDesc.sTitle = "XGE XUI Text Buffer Lab";
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
		(tApp.bTextCoreOK && tApp.bTextEventOK && tApp.bFontSetOK &&
		 tApp.bEditEventOK && tApp.bImeOK && tApp.bCandidateOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

