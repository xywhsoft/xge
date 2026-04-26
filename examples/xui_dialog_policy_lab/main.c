#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pOwnerWidget;
	xge_xui_widget pDialogWidget;
	xge_xui_widget pDialogLabelWidget;
	xge_xui_widget pHintWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tOwnerLabel;
	xge_xui_label_t tDialogLabel;
	xge_xui_label_t tHint;
	xge_xui_dialog_t tDialog;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iCloseCount;
	int bInitOK;
	int bVisualOK;
	int bEventMatrixOK;
	int bPolicyOK;
	int bCloseButtonOK;
	int bReopenOK;
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
	return (fA >= fB - fEpsilon) && (fA <= fB + fEpsilon);
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

static xge_rect_t DialogCloseRect(xge_xui_dialog pDialog)
{
	xge_rect_t tClose;

	memset(&tClose, 0, sizeof(tClose));
	tClose.fW = 18.0f;
	tClose.fH = 18.0f;
	tClose.fX = pDialog->pWidget->tContentRect.fX + pDialog->pWidget->tContentRect.fW - tClose.fW - 2.0f;
	tClose.fY = pDialog->pWidget->tContentRect.fY + 3.0f;
	return tClose;
}

static void DialogClose(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iCloseCount++;
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
			printf("xui-dialog-policy-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-dialog-policy-lab font load failed\n");
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
	if ( tRoot.fH < 380.0f ) {
		tRoot.fH = 380.0f;
	}
	fInnerW = tRoot.fW - 40.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pOwnerWidget, (xge_rect_t){ 24.0f, 84.0f, 220.0f, 42.0f });
	xgeXuiWidgetSetRect(pApp->pDialogWidget, (xge_rect_t){ 276.0f, 82.0f, 300.0f, 164.0f });
	xgeXuiWidgetSetRect(pApp->pDialogLabelWidget, (xge_rect_t){ 14.0f, 38.0f, 248.0f, 84.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 24.0f, tRoot.fH - 46.0f, fInnerW, 24.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[320];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d visual=%d event=%d policy=%d closebtn=%d reopen=%d close=%d open=%d modal=%d show=%d esc=%d",
		pApp->bInitOK,
		pApp->bVisualOK,
		pApp->bEventMatrixOK,
		pApp->bPolicyOK,
		pApp->bCloseButtonOK,
		pApp->bReopenOK,
		pApp->iCloseCount,
		xgeXuiDialogIsOpen(&pApp->tDialog),
		pApp->tDialog.bModal,
		pApp->tDialog.bShowClose,
		pApp->tDialog.bCloseOnEscape);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;
	xge_xui_widget pOverlayRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pOverlayRoot = xgeXuiOverlayRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( (pRoot == NULL) || (pOverlayRoot == NULL) ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(238, 242, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 34, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(34, 42, 56, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 96, 118, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(212, 96, 96, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(50, 64, 84, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(68, 84, 108, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(44, 58, 78, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(54, 74, 104, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(78, 82, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pOwnerWidget = xgeXuiWidgetCreate();
	pApp->pDialogWidget = xgeXuiWidgetCreate();
	pApp->pDialogLabelWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pOwnerWidget == NULL) ||
	     (pApp->pDialogWidget == NULL) || (pApp->pDialogLabelWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pOwnerWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pOwnerWidget, XGE_COLOR_RGBA(52, 70, 94, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pOwnerWidget, 12.0f, 10.0f, 12.0f, 10.0f);
	xgeXuiWidgetSetFocusable(pApp->pOwnerWidget, 1);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui dialog policy lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tOwnerLabel, pApp->pOwnerWidget, pFont, "Dialog owner") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tOwnerLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tDialogLabel, pApp->pDialogLabelWidget, pFont, "Dialog policy matrix\nmodal / escape / close button") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tDialogLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiLabelInit(&pApp->tHint, pApp->pHintWidget, pFont, "Auto-check covers modal vs non-modal event consumption, show-close, close-on-escape, direct event path and reopen.") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tHint, XGE_COLOR_RGBA(164, 182, 206, 255));

	if ( xgeXuiDialogInit(&pApp->tDialog, &pApp->tXui, pApp->pDialogWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetAdd(pOverlayRoot, pApp->pDialogWidget);
	xgeXuiWidgetAdd(pApp->pDialogWidget, pApp->pDialogLabelWidget);
	return XGE_OK;
}

static int RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tClose;
	xge_rect_t tRect;
	int iRet;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	pApp->bInitOK =
		(pApp->tDialog.pWidget == pApp->pDialogWidget) &&
		(pApp->pDialogWidget->procEvent == xgeXuiDialogEventProc) &&
		(pApp->pDialogWidget->procPaint == xgeXuiDialogPaintProc) &&
		(pApp->tDialog.bModal == 1) &&
		(pApp->tDialog.bCloseOnEscape == 1) &&
		(pApp->tDialog.bShowClose == 1);

	xgeXuiDialogSetTitle(&pApp->tDialog, pApp->bFontReady ? &pApp->tFont : NULL, "Policy dialog");
	xgeXuiDialogSetClose(&pApp->tDialog, DialogClose, pApp);
	xgeXuiDialogSetColors(&pApp->tDialog, XGE_COLOR_RGBA(0, 0, 0, 96), XGE_COLOR_RGBA(42, 48, 60, 255), XGE_COLOR_RGBA(248, 250, 252, 255), XGE_COLOR_RGBA(212, 96, 96, 255));
	xgeXuiDialogSetOpen(&pApp->tDialog, 0);
	xgeXuiDialogSetOpen(&pApp->tDialog, 1);
	pApp->bVisualOK =
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0) &&
		(strcmp(pApp->tDialog.sTitle, "Policy dialog") == 0) &&
		(pApp->tDialog.iBackdropColor == XGE_COLOR_RGBA(0, 0, 0, 96)) &&
		(pApp->tDialog.iBackgroundColor == XGE_COLOR_RGBA(42, 48, 60, 255)) &&
		(pApp->tDialog.iCloseColor == XGE_COLOR_RGBA(212, 96, 96, 255)) &&
		(pApp->tXui.pFocus == pApp->pDialogWidget);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_WHEEL, 0, 8.0f, 8.0f);
	iRet = xgeXuiDialogEvent(&pApp->tDialog, &tEvent);
	pApp->bEventMatrixOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0);

	xgeXuiDialogSetModal(&pApp->tDialog, 0);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_WHEEL, 0, 8.0f, 8.0f);
	iRet = xgeXuiDialogEventProc(pApp->pDialogWidget, &tEvent, &pApp->tDialog);
	pApp->bEventMatrixOK =
		pApp->bEventMatrixOK &&
		(iRet == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0);

	xgeXuiDialogSetModal(&pApp->tDialog, 1);
	xgeXuiDialogSetCloseOnEscape(&pApp->tDialog, 0);
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	iRet = xgeXuiDialogEvent(&pApp->tDialog, &tEvent);
	pApp->bPolicyOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0);

	xgeXuiDialogSetShowClose(&pApp->tDialog, 0);
	tClose = DialogCloseRect(&pApp->tDialog);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tClose.fX + 1.0f, tClose.fY + 1.0f);
	iRet = xgeXuiDialogEventProc(pApp->pDialogWidget, &tEvent, &pApp->tDialog);
	pApp->bCloseButtonOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0) &&
		(pApp->iCloseCount == 0);

	xgeXuiDialogSetShowClose(&pApp->tDialog, 1);
	xgeXuiDialogPaintProc(pApp->pDialogWidget, &pApp->tDialog);
	tClose = pApp->tDialog.tCloseRect;
	pApp->bCloseButtonOK =
		pApp->bCloseButtonOK &&
		(tClose.fW > 0.0f) &&
		(tClose.fH > 0.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tClose.fX + 1.0f, tClose.fY + 1.0f);
	iRet = xgeXuiDialogEvent(&pApp->tDialog, &tEvent);
	pApp->bCloseButtonOK =
		pApp->bCloseButtonOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) == 0) &&
		(pApp->iCloseCount == 1);

	xgeXuiDialogSetOpen(&pApp->tDialog, 1);
	xgeXuiDialogSetCloseOnEscape(&pApp->tDialog, 1);
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE);
	iRet = xgeXuiDialogEvent(&pApp->tDialog, &tEvent);
	pApp->bPolicyOK =
		pApp->bPolicyOK &&
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiDialogIsOpen(&pApp->tDialog) == 0) &&
		(pApp->iCloseCount == 2);

	xgeXuiDialogSetOpen(&pApp->tDialog, 1);
	xgeXuiDialogSetTitle(&pApp->tDialog, pApp->bFontReady ? &pApp->tFont : NULL, "Policy dialog reopened");
	xgeXuiDialogSetModal(&pApp->tDialog, 1);
	xgeXuiDialogSetShowClose(&pApp->tDialog, 1);
	xgeXuiDialogSetCloseOnEscape(&pApp->tDialog, 1);
	tRect = xgeXuiWidgetGetRect(pApp->pDialogWidget);
	pApp->bReopenOK =
		(xgeXuiDialogIsOpen(&pApp->tDialog) != 0) &&
		(strcmp(pApp->tDialog.sTitle, "Policy dialog reopened") == 0) &&
		(FloatNear(tRect.fW, 300.0f, 0.01f)) &&
		(FloatNear(tRect.fH, 164.0f, 0.01f));

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
	xgeXuiDialogUnit(&pApp->tDialog);
	xgeXuiLabelUnit(&pApp->tHint);
	xgeXuiLabelUnit(&pApp->tDialogLabel);
	xgeXuiLabelUnit(&pApp->tOwnerLabel);
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
			"xui-dialog-policy-lab final-summary frames=%d init=%d visual=%d event=%d policy=%d closebtn=%d reopen=%d close=%d open=%d modal=%d show=%d esc=%d title=%s\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bVisualOK,
			pApp->bEventMatrixOK,
			pApp->bPolicyOK,
			pApp->bCloseButtonOK,
			pApp->bReopenOK,
			pApp->iCloseCount,
			xgeXuiDialogIsOpen(&pApp->tDialog),
			pApp->tDialog.bModal,
			pApp->tDialog.bShowClose,
			pApp->tDialog.bCloseOnEscape,
			pApp->tDialog.sTitle != NULL ? pApp->tDialog.sTitle : "");
		printf("xui-dialog-policy-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.iWidth = 740;
	tDesc.iHeight = 390;
	tDesc.sTitle = "XGE XUI Dialog Policy Lab";
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
		(tApp.bInitOK && tApp.bVisualOK && tApp.bEventMatrixOK &&
		 tApp.bPolicyOK && tApp.bCloseButtonOK && tApp.bReopenOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

