#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pEditCaptionWidget;
	xge_xui_widget pEditWidget;
	xge_xui_widget pHintWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tEditCaption;
	xge_xui_label_t tHint;
	xge_xui_text_edit_t tEdit;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bWrapOK;
	int bScrollOK;
	int bPageOK;
	int bHomeEndOK;
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
			printf("xui-text-wrap-scroll-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-text-wrap-scroll-lab font load failed\n");
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
	if ( tRoot.fH < 460.0f ) {
		tRoot.fH = 460.0f;
	}
	fInnerW = tRoot.fW - 40.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pEditCaptionWidget, (xge_rect_t){ 20.0f, 58.0f, fInnerW, 22.0f });
	xgeXuiWidgetSetRect(pApp->pEditWidget, (xge_rect_t){ 20.0f, 86.0f, fInnerW, tRoot.fH - 156.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 20.0f, tRoot.fH - 44.0f, fInnerW, 22.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sStatus[256];
	char sCaption[256];

	snprintf(
		sStatus,
		sizeof(sStatus),
		"wrap=%d scroll=%d page=%d homeend=%d",
		pApp->bWrapOK,
		pApp->bScrollOK,
		pApp->bPageOK,
		pApp->bHomeEndOK);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sStatus);

	snprintf(
		sCaption,
		sizeof(sCaption),
		"TextEdit: cursor=%d scroll=%.1f,%.1f lines=%d visual=%d wrap=%d",
		xgeXuiTextGetCursor(&pApp->tEdit.tText),
		pApp->tEdit.fScrollX,
		pApp->tEdit.fScrollY,
		pApp->tEdit.iLineCount,
		pApp->tEdit.iVisualLineCount,
		pApp->tEdit.bWordWrap);
	xgeXuiLabelSetText(&pApp->tEditCaption, sCaption);
}

static void PaintOnce(app_state_t* pApp)
{
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
}

static void ResetDemoState(app_state_t* pApp)
{
	xgeXuiWidgetSetRect(
		pApp->pEditWidget,
		(xge_rect_t){
			pApp->pEditWidget->tRect.fX,
			pApp->pEditWidget->tRect.fY,
			pApp->pEditWidget->tRect.fW,
			pApp->pRootPanel->tRect.fH - 156.0f
		});
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiTextEditSetText(
		&pApp->tEdit,
		"Line 1: wrap and scroll focus.\n"
		"Line 2: PageUp and PageDown should move cursor by viewport height.\n"
		"Line 3: Ctrl+Home/Ctrl+End should jump to boundaries.\n"
		"Long line: abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz.\n"
		"Tail line: keep enough rows to make vertical navigation meaningful.");
	xgeXuiTextEditSetWordWrap(&pApp->tEdit, 1);
	xgeXuiTextEditSetScroll(&pApp->tEdit, 0.0f, 18.0f);
	xgeXuiSetFocus(&pApp->tXui, pApp->pEditWidget);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	PaintOnce(pApp);
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
	tTheme.iAccentColor = XGE_COLOR_RGBA(86, 166, 255, 255);
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
	pApp->pEditCaptionWidget = xgeXuiWidgetCreate();
	pApp->pEditWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pEditCaptionWidget == NULL) ||
	     (pApp->pEditWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pEditCaptionWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pEditWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pEditWidget, 8.0f, 8.0f, 8.0f, 8.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui text wrap scroll lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

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
	xgeXuiTextEditSetText(
		&pApp->tEdit,
		"Line 1: wrap and scroll focus.\n"
		"Line 2: PageUp and PageDown should move cursor by viewport height.\n"
		"Line 3: Ctrl+Home/Ctrl+End should jump to boundaries.\n"
		"Long line: abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz.\n"
		"Tail line: keep enough rows to make vertical navigation meaningful.");
	xgeXuiTextEditSetWordWrap(&pApp->tEdit, 1);

	if ( xgeXuiLabelInit(&pApp->tHint, pApp->pHintWidget, pFont, "Auto-check runs on startup. You can still inspect wrapped lines, scroll position and caret location before exit.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tHint, XGE_COLOR_RGBA(164, 182, 206, 255));
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	int iBefore;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiSetFocus(&pApp->tXui, pApp->pEditWidget);

	xgeXuiTextEditSetText(&pApp->tEdit, "abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz");
	xgeXuiWidgetSetRect(pApp->pEditWidget, (xge_rect_t){ pApp->pEditWidget->tRect.fX, pApp->pEditWidget->tRect.fY, 120.0f, pApp->pEditWidget->tRect.fH });
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiTextEditSetWordWrap(&pApp->tEdit, 1);
	PaintOnce(pApp);
	pApp->bWrapOK =
		(pApp->tEdit.bWordWrap != 0) &&
		(pApp->tEdit.arrVisualLines != NULL) &&
		(pApp->tEdit.iVisualLineCount >= pApp->tEdit.iLineCount) &&
		(pApp->tEdit.bVisualCacheDirty == 0) &&
		(pApp->tEdit.fScrollX == 0.0f);

	xgeXuiTextEditSetWordWrap(&pApp->tEdit, 0);
	xgeXuiWidgetSetRect(pApp->pEditWidget, (xge_rect_t){ pApp->pEditWidget->tRect.fX, pApp->pEditWidget->tRect.fY, 640.0f, 96.0f });
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiTextEditSetText(&pApp->tEdit, "ab\ncd\ne");
	xgeXuiSetFocus(&pApp->tXui, pApp->pEditWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_HOME, 0);
	pApp->bHomeEndOK =
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiTextGetCursor(&pApp->tEdit.tText) == 6);
	MakeKeyEvent(&tEvent, XGE_KEY_END, XGE_KEY_MOD_CTRL);
	pApp->bHomeEndOK =
		pApp->bHomeEndOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiTextGetCursor(&pApp->tEdit.tText) == pApp->tEdit.tText.iSize);
	MakeKeyEvent(&tEvent, XGE_KEY_HOME, XGE_KEY_MOD_CTRL);
	pApp->bHomeEndOK =
		pApp->bHomeEndOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiTextGetCursor(&pApp->tEdit.tText) == 0);

	xgeXuiTextEditSetText(&pApp->tEdit, "a\nb\nc\nd\ne\nf\ng\nh");
	pApp->bScrollOK = (pApp->tEdit.fScrollY > 0.0f);
	xgeXuiTextEditSetScroll(&pApp->tEdit, 4.0f, 5.0f);
	pApp->bScrollOK =
		pApp->bScrollOK &&
		(pApp->tEdit.fScrollX == 4.0f) &&
		(pApp->tEdit.fScrollY == 5.0f);

	xgeXuiTextEditSetText(&pApp->tEdit, "ab\ncd\ne");
	MakeKeyEvent(&tEvent, XGE_KEY_HOME, XGE_KEY_MOD_CTRL);
	(void)xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_PAGE_DOWN, 0);
	iBefore = xgeXuiTextGetCursor(&pApp->tEdit.tText);
	pApp->bPageOK =
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiTextGetCursor(&pApp->tEdit.tText) > iBefore);
	MakeKeyEvent(&tEvent, XGE_KEY_PAGE_UP, 0);
	pApp->bPageOK =
		pApp->bPageOK &&
		(xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiTextGetCursor(&pApp->tEdit.tText) >= 0);

	ResetDemoState(pApp);
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
	xgeXuiLabelUnit(&pApp->tEditCaption);
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
	if ( pApp->iFrameCount >= pApp->iFrameLimit ) {
		printf(
			"xui-text-wrap-scroll-lab final-summary frames=%d wrap=%d scroll=%d page=%d homeend=%d cursor=%d scroll=%.2f,%.2f lines=%d visual=%d wrap=%d\n",
			pApp->iFrameCount,
			pApp->bWrapOK,
			pApp->bScrollOK,
			pApp->bPageOK,
			pApp->bHomeEndOK,
			xgeXuiTextGetCursor(&pApp->tEdit.tText),
			pApp->tEdit.fScrollX,
			pApp->tEdit.fScrollY,
			pApp->tEdit.iLineCount,
			pApp->tEdit.iVisualLineCount,
			pApp->tEdit.bWordWrap);
		printf("xui-text-wrap-scroll-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.iWidth = 780;
	tDesc.iHeight = 500;
	tDesc.sTitle = "XGE XUI Text Wrap Scroll Lab";
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
		(tApp.bWrapOK && tApp.bScrollOK && tApp.bPageOK && tApp.bHomeEndOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}
