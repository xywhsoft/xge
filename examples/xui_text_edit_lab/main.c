#include "../../xge.h"
#include <math.h>
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
	int iInputContextMenuOpenCount;
	int iEditContextMenuOpenCount;
	int bInputOK;
	int bTextEditOK;
	int bCursorSelectionOK;
	int bReadonlyDisabledPasswordOK;
	int bClipboardContextOK;
	int bUndoRedoOK;
	int bWrapOK;
	int bImeOK;
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

static void MakeImeEvent(xge_event_t* pEvent, int iType, xge_ime_event_t* pIme, const char* sText)
{
	memset(pEvent, 0, sizeof(*pEvent));
	memset(pIme, 0, sizeof(*pIme));
	pIme->sText = sText;
	pEvent->iType = iType;
	pEvent->pData = (iType == XGE_EVENT_IME_END) ? NULL : pIme;
}

static xge_vec2_t ContentPoint(xge_xui_widget pWidget, float fRatioX, float fRatioY)
{
	xge_vec2_t tPoint;

	tPoint.fX = pWidget->tContentRect.fX + (pWidget->tContentRect.fW * fRatioX);
	tPoint.fY = pWidget->tContentRect.fY + (pWidget->tContentRect.fH * fRatioY);
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
			printf("xui-text-edit-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-text-edit-lab font load failed\n");
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
	if ( tRoot.fW < 620.0f ) {
		tRoot.fW = 620.0f;
	}
	if ( tRoot.fH < 380.0f ) {
		tRoot.fH = 380.0f;
	}
	fInnerW = tRoot.fW - 40.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 38.0f });
	xgeXuiWidgetSetRect(pApp->pInputCaptionWidget, (xge_rect_t){ 20.0f, 56.0f, fInnerW, 22.0f });
	xgeXuiWidgetSetRect(pApp->pInputWidget, (xge_rect_t){ 20.0f, 82.0f, fInnerW, 38.0f });
	xgeXuiWidgetSetRect(pApp->pEditCaptionWidget, (xge_rect_t){ 20.0f, 136.0f, fInnerW, 22.0f });
	xgeXuiWidgetSetRect(pApp->pEditWidget, (xge_rect_t){ 20.0f, 162.0f, fInnerW, tRoot.fH - 232.0f });
	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 20.0f, tRoot.fH - 50.0f, fInnerW, 26.0f });

	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	pApp->bClipboardContextOK = (pApp->iInputContextMenuOpenCount > 0) && (pApp->iEditContextMenuOpenCount > 0);
	snprintf(
		sText,
		sizeof(sText),
		"input=%d edit=%d cursor=%d ro=%d clipmenu=%d undo=%d wrap=%d ime=%d menus=%d/%d",
		pApp->bInputOK,
		pApp->bTextEditOK,
		pApp->bCursorSelectionOK,
		pApp->bReadonlyDisabledPasswordOK,
		pApp->bClipboardContextOK,
		pApp->bUndoRedoOK,
		pApp->bWrapOK,
		pApp->bImeOK,
		pApp->iInputContextMenuOpenCount,
		pApp->iEditContextMenuOpenCount);
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

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pInputCaptionWidget = xgeXuiWidgetCreate();
	pApp->pInputWidget = xgeXuiWidgetCreate();
	pApp->pEditCaptionWidget = xgeXuiWidgetCreate();
	pApp->pEditWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pInputCaptionWidget == NULL) || (pApp->pInputWidget == NULL) || (pApp->pEditCaptionWidget == NULL) || (pApp->pEditWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 40, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	if ( xgeXuiWidgetAdd(pRoot, pApp->pRootPanel) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui text edit lab");
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pApp->tInputCaption, pApp->pInputCaptionWidget, pFont, "Input: placeholder / password / readonly / clipboard / IME / context menu");
	xgeXuiLabelSetColor(&pApp->tInputCaption, XGE_COLOR_RGBA(188, 204, 224, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pInputCaptionWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetPaddingPx(pApp->pInputWidget, 8.0f, 7.0f, 8.0f, 7.0f);
	xgeXuiInputInit(&pApp->tInput, &pApp->tXui, pApp->pInputWidget, pFont);
	xgeXuiInputSetColors(
		&pApp->tInput,
		XGE_COLOR_RGBA(242, 246, 252, 255),
		XGE_COLOR_RGBA(28, 36, 48, 255),
		XGE_COLOR_RGBA(42, 58, 82, 255),
		XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiInputSetPlaceholder(&pApp->tInput, "Type here, use Ctrl+C / Ctrl+V or right click");
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pInputWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pApp->tEditCaption, pApp->pEditCaptionWidget, pFont, "TextEdit: selection / undo redo / wrap / IME / page navigation");
	xgeXuiLabelSetColor(&pApp->tEditCaption, XGE_COLOR_RGBA(188, 204, 224, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pEditCaptionWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetPaddingPx(pApp->pEditWidget, 8.0f, 8.0f, 8.0f, 8.0f);
	xgeXuiTextEditInit(&pApp->tEdit, &pApp->tXui, pApp->pEditWidget, pFont);
	xgeXuiTextEditSetColors(
		&pApp->tEdit,
		XGE_COLOR_RGBA(242, 246, 252, 255),
		XGE_COLOR_RGBA(26, 34, 46, 255),
		XGE_COLOR_RGBA(42, 58, 82, 255),
		XGE_COLOR_RGBA(255, 214, 92, 255));
	xgeXuiTextEditSetText(
		&pApp->tEdit,
		"Line 1: keyboard, mouse, IME, wrap.\n"
		"Line 2: Ctrl+A/C/X/V and Ctrl+Z/Y are covered.\n"
		"Line 3: PageUp/PageDown, Home/End, Ctrl+Home/Ctrl+End.\n"
		"Long line: abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz.");
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pEditWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pApp->tHint, pApp->pHintWidget, pFont, "Auto-check runs at startup. You can still inspect caret, selection, wrapping and context menu before exit.");
	xgeXuiLabelSetColor(&pApp->tHint, XGE_COLOR_RGBA(164, 182, 206, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pHintWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	return XGE_OK;
}

static void ResetDemoState(app_state_t* pApp)
{
	xgeXuiWidgetSetRect(pApp->pInputWidget, (xge_rect_t){ pApp->pInputWidget->tRect.fX, pApp->pInputWidget->tRect.fY, pApp->pInputWidget->tRect.fW, 38.0f });
	xgeXuiInputSetReadonly(&pApp->tInput, 0);
	xgeXuiInputSetDisabled(&pApp->tInput, 0);
	xgeXuiInputSetPassword(&pApp->tInput, 0);
	xgeXuiInputSetPlaceholder(&pApp->tInput, "Type here, use Ctrl+C / Ctrl+V or right click");
	xgeXuiInputSetText(&pApp->tInput, "Input demo: copy / paste / IME");
	xgeXuiInputSetSelection(&pApp->tInput, 0, 0);

	xgeXuiTextEditSetReadonly(&pApp->tEdit, 0);
	xgeXuiTextEditSetWordWrap(&pApp->tEdit, 1);
	xgeXuiTextEditSetText(
		&pApp->tEdit,
		"Line 1: keyboard, mouse, IME, wrap.\n"
		"Line 2: Ctrl+A/C/X/V and Ctrl+Z/Y are covered.\n"
		"Line 3: PageUp/PageDown, Home/End, Ctrl+Home/Ctrl+End.\n"
		"Long line: abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz.");
	xgeXuiTextEditSetScroll(&pApp->tEdit, 0.0f, 0.0f);
	xgeXuiTextSetSelection(&pApp->tEdit.tText, 0, 0);
	xgeXuiSetFocus(&pApp->tXui, pApp->pEditWidget);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_ime_event_t tIme;
	xge_vec2_t tPointA;
	xge_vec2_t tPointB;
	xge_rect_t tCandidateInput;
	xge_rect_t tCandidateEdit;
	int iStart;
	int iEnd;
	int bInputSettersOK;
	int bInputEditOK;
	int bInputSelectionOK;
	int bInputReadonlyOK;
	int bInputDisabledOK;
	int bInputClipboardOK;
	int bInputMenuOK;
	int bTextEditSettersOK;
	int bTextEditEditOK;
	int bTextEditSelectionOK;
	int bTextEditReadonlyOK;
	int bTextEditClipboardOK;
	int bTextEditMenuOK;
	int bTextEditImeOK;

	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);

	bInputSettersOK =
		(pApp->tInput.pWidget == pApp->pInputWidget) &&
		(pApp->tInput.pFont == (pApp->bFontReady ? &pApp->tFont : NULL)) &&
		(pApp->pInputWidget->procEvent == xgeXuiInputEventProc) &&
		(pApp->pInputWidget->procUpdate == xgeXuiInputUpdateProc) &&
		(pApp->pInputWidget->procPaint == xgeXuiInputPaintProc) &&
		(pApp->tInput.pDefaultMenu != NULL);

	tPointA = ContentPoint(pApp->pInputWidget, 0.02f, 0.5f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tPointA.fX, tPointA.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tPointA.fX, tPointA.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);

	xgeXuiUpdate(&pApp->tXui, 0.51f);
	xgeXuiUpdate(&pApp->tXui, 0.51f);

	xgeXuiInputSetText(&pApp->tInput, "A");
	MakeTextEvent(&tEvent, 0x4E2D);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_BACKSPACE, 0);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiInputSetText(&pApp->tInput, "ABCD");
	xgeXuiInputSetSelection(&pApp->tInput, 1, 3);
	xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
	MakeTextEvent(&tEvent, 'Z');
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bInputEditOK =
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "AZD") == 0) &&
		(iStart == 1) &&
		(iEnd == 3) &&
		(pApp->tInput.bCursorVisible != 0);

	xgeXuiInputSetText(&pApp->tInput, "hello world");
	xgeXuiTextSetCursor(&pApp->tInput.tText, 11);
	MakeKeyEvent(&tEvent, XGE_KEY_LEFT, XGE_KEY_MOD_CTRL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_RIGHT, XGE_KEY_MOD_CTRL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiInputSetText(&pApp->tInput, "drag-select");
	tPointA = ContentPoint(pApp->pInputWidget, 0.02f, 0.5f);
	tPointB = ContentPoint(pApp->pInputWidget, 0.36f, 0.5f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tPointA.fX, tPointA.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tPointB.fX, tPointB.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tPointB.fX, tPointB.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
	bInputSelectionOK =
		(xgeXuiTextGetCursor(&pApp->tInput.tText) >= 0) &&
		(iStart != iEnd);

	xgeXuiInputSetPlaceholder(&pApp->tInput, "placeholder");
	xgeXuiInputSetPassword(&pApp->tInput, 1);
	xgeXuiInputSetReadonly(&pApp->tInput, 1);
	xgeXuiInputSetText(&pApp->tInput, "RO");
	MakeTextEvent(&tEvent, 'X');
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bInputReadonlyOK =
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "RO") == 0) &&
		(pApp->tInput.bPassword != 0) &&
		(pApp->tInput.sPlaceholder != NULL) &&
		(strcmp(pApp->tInput.sPlaceholder, "placeholder") == 0);
	xgeXuiInputSetReadonly(&pApp->tInput, 0);
	xgeXuiInputSetPassword(&pApp->tInput, 0);
	xgeXuiInputSetDisabled(&pApp->tInput, 1);
	MakeTextEvent(&tEvent, 'Q');
	bInputDisabledOK =
		((pApp->pInputWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) &&
		(xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONTINUE);
	xgeXuiInputSetDisabled(&pApp->tInput, 0);

	xgeXuiInputSetText(&pApp->tInput, "ABCD");
	xgeXuiInputSetSelection(&pApp->tInput, 1, 3);
	MakeKeyEvent(&tEvent, 'C', XGE_KEY_MOD_CTRL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bInputClipboardOK = (xgeClipboardGetText()[0] != 0);
	MakeKeyEvent(&tEvent, 'X', XGE_KEY_MOD_CTRL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bInputClipboardOK = bInputClipboardOK && (strcmp(xgeXuiInputGetText(&pApp->tInput), "AD") == 0);
	xgeClipboardSetText("BC");
	xgeXuiInputSetSelection(&pApp->tInput, 1, 1);
	MakeKeyEvent(&tEvent, 'V', XGE_KEY_MOD_CTRL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeKeyEvent(&tEvent, 'A', XGE_KEY_MOD_CTRL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiInputGetSelection(&pApp->tInput, &iStart, &iEnd);
	bInputClipboardOK =
		bInputClipboardOK &&
		(strstr(xgeXuiInputGetText(&pApp->tInput), "BC") != NULL) &&
		(iStart == 0) &&
		(iEnd == pApp->tInput.tText.iSize);

	tPointA = ContentPoint(pApp->pInputWidget, 0.25f, 0.5f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_RIGHT, tPointA.fX, tPointA.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_RIGHT, tPointA.fX, tPointA.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bInputMenuOK = (pApp->tInput.pDefaultMenu != NULL) && xgeXuiMenuIsOpen(pApp->tInput.pDefaultMenu);
	if ( bInputMenuOK ) {
		pApp->iInputContextMenuOpenCount++;
		xgeXuiMenuClose(pApp->tInput.pDefaultMenu);
	}

	MakeImeEvent(&tEvent, XGE_EVENT_IME_UPDATE, &tIme, "ime");
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	tCandidateInput = xgeXuiInputGetCandidateRect(&pApp->tInput);
	MakeImeEvent(&tEvent, XGE_EVENT_IME_END, &tIme, NULL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);

	bTextEditSettersOK =
		(pApp->tEdit.pWidget == pApp->pEditWidget) &&
		(pApp->tEdit.pFont == (pApp->bFontReady ? &pApp->tFont : NULL)) &&
		(pApp->pEditWidget->procEvent == xgeXuiTextEditEventProc) &&
		(pApp->pEditWidget->procUpdate == xgeXuiTextEditUpdateProc) &&
		(pApp->pEditWidget->procPaint == xgeXuiTextEditPaintProc) &&
		(pApp->tEdit.pDefaultMenu != NULL);

	xgeXuiSetFocus(&pApp->tXui, pApp->pEditWidget);
	xgeXuiTextEditSetText(&pApp->tEdit, "ab\ncd");
	xgeXuiPaint(&pApp->tXui);
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER, 0);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeTextEvent(&tEvent, 'e');
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bTextEditEditOK =
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "ab\ncd\ne") == 0) &&
		(pApp->tEdit.iLineCount == 3) &&
		(pApp->tEdit.arrLineStarts != NULL);

	xgeXuiTextEditSetText(&pApp->tEdit, "ab\ncd\ne");
	MakeKeyEvent(&tEvent, XGE_KEY_HOME, 0);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_END, XGE_KEY_MOD_CTRL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_HOME, XGE_KEY_MOD_CTRL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_PAGE_DOWN, 0);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_RIGHT, XGE_KEY_MOD_SHIFT);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiTextGetSelection(&pApp->tEdit.tText, &iStart, &iEnd);
	tPointA = ContentPoint(pApp->pEditWidget, 0.02f, 0.08f);
	tPointB = ContentPoint(pApp->pEditWidget, 0.18f, 0.28f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tPointA.fX, tPointA.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tPointB.fX, tPointB.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tPointB.fX, tPointB.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bTextEditSelectionOK = (iStart != iEnd) || (pApp->tEdit.tText.iSelectStart != pApp->tEdit.tText.iSelectEnd);

	xgeXuiTextEditSetText(&pApp->tEdit, "ab\ncd");
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER, 0);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeTextEvent(&tEvent, 'e');
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bTextEditEditOK = bTextEditEditOK && (xgeXuiTextEditUndo(&pApp->tEdit) == XGE_OK) && (xgeXuiTextEditRedo(&pApp->tEdit) == XGE_OK);
	MakeKeyEvent(&tEvent, 'Z', XGE_KEY_MOD_CTRL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeKeyEvent(&tEvent, 'Y', XGE_KEY_MOD_CTRL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bTextEditEditOK = bTextEditEditOK && (strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "ab\ncd\ne") == 0);

	xgeXuiWidgetSetRect(pApp->pEditWidget, (xge_rect_t){ pApp->pEditWidget->tRect.fX, pApp->pEditWidget->tRect.fY, 220.0f, pApp->pEditWidget->tRect.fH });
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiTextEditSetText(&pApp->tEdit, "abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz");
	xgeXuiTextEditSetWordWrap(&pApp->tEdit, 1);
	xgeXuiPaint(&pApp->tXui);
	pApp->bWrapOK =
		(pApp->tEdit.bWordWrap != 0) &&
		(pApp->tEdit.arrVisualLines != NULL) &&
		(pApp->tEdit.iVisualLineCount >= pApp->tEdit.iLineCount);
	xgeXuiTextEditSetWordWrap(&pApp->tEdit, 0);
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	xgeXuiTextEditSetText(&pApp->tEdit, "AB\nCD");
	MakeKeyEvent(&tEvent, 'A', XGE_KEY_MOD_CTRL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeKeyEvent(&tEvent, 'X', XGE_KEY_MOD_CTRL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bTextEditClipboardOK = (xgeXuiTextEditGetText(&pApp->tEdit)[0] == 0);
	xgeClipboardSetText("AB\nCD");
	MakeKeyEvent(&tEvent, 'V', XGE_KEY_MOD_CTRL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bTextEditClipboardOK =
		bTextEditClipboardOK &&
		(strstr(xgeXuiTextEditGetText(&pApp->tEdit), "AB") != NULL) &&
		(strstr(xgeXuiTextEditGetText(&pApp->tEdit), "CD") != NULL);

	tPointA = ContentPoint(pApp->pEditWidget, 0.30f, 0.16f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_RIGHT, tPointA.fX, tPointA.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_RIGHT, tPointA.fX, tPointA.fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bTextEditMenuOK = (pApp->tEdit.pDefaultMenu != NULL) && xgeXuiMenuIsOpen(pApp->tEdit.pDefaultMenu);
	if ( bTextEditMenuOK ) {
		pApp->iEditContextMenuOpenCount++;
		xgeXuiMenuClose(pApp->tEdit.pDefaultMenu);
	}

	MakeImeEvent(&tEvent, XGE_EVENT_IME_UPDATE, &tIme, "ime");
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	tCandidateEdit = xgeXuiTextEditGetCandidateRect(&pApp->tEdit);
	bTextEditImeOK = (strcmp(xgeXuiTextGetComposition(&pApp->tEdit.tText), "ime") == 0);
	MakeImeEvent(&tEvent, XGE_EVENT_IME_END, &tIme, NULL);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bTextEditImeOK = bTextEditImeOK && (xgeXuiTextGetComposition(&pApp->tEdit.tText)[0] == 0);

	xgeXuiTextEditSetReadonly(&pApp->tEdit, 1);
	xgeXuiTextEditSetText(&pApp->tEdit, "readonly");
	MakeTextEvent(&tEvent, 'x');
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	bTextEditReadonlyOK = (strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "readonly") == 0);
	xgeXuiTextEditSetScroll(&pApp->tEdit, 4.0f, 5.0f);
	bTextEditSettersOK =
		bTextEditSettersOK &&
		FloatNear(pApp->tEdit.fScrollX, 4.0f, 0.01f) &&
		FloatNear(pApp->tEdit.fScrollY, 5.0f, 0.01f);

	pApp->bInputOK = bInputSettersOK && bInputEditOK;
	pApp->bTextEditOK = bTextEditSettersOK && bTextEditEditOK;
	pApp->bCursorSelectionOK = bInputSelectionOK && bTextEditSelectionOK;
	pApp->bReadonlyDisabledPasswordOK = bInputReadonlyOK && bInputDisabledOK && bTextEditReadonlyOK;
	pApp->bClipboardContextOK = (pApp->iInputContextMenuOpenCount > 0) && (pApp->iEditContextMenuOpenCount > 0);
	pApp->bUndoRedoOK = (pApp->tEdit.iUndoCount >= 0) && (pApp->tEdit.iRedoCount >= 0) && bTextEditEditOK;
	pApp->bImeOK =
		(tCandidateInput.fW > 0.0f) &&
		(tCandidateInput.fH > 0.0f) &&
		(tCandidateEdit.fW > 0.0f) &&
		(tCandidateEdit.fH > 0.0f) &&
		bTextEditImeOK;

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
	(void)LoadFont(pApp);
	return RunStaticChecks(pApp);
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
			"xui-text-edit-lab final-summary frames=%d input=%d edit=%d cursor=%d ro=%d clipmenu=%d undo=%d wrap=%d ime=%d menus=%d/%d input(len=%d cursor=%d select=%d,%d password=%d disabled=%d) edit(len=%d lines=%d visual=%d undo=%d redo=%d scroll=%.2f,%.2f wrap=%d)\n",
			pApp->iFrameCount,
			pApp->bInputOK,
			pApp->bTextEditOK,
			pApp->bCursorSelectionOK,
			pApp->bReadonlyDisabledPasswordOK,
			pApp->bClipboardContextOK,
			pApp->bUndoRedoOK,
			pApp->bWrapOK,
			pApp->bImeOK,
			pApp->iInputContextMenuOpenCount,
			pApp->iEditContextMenuOpenCount,
			(int)strlen(xgeXuiInputGetText(&pApp->tInput)),
			pApp->tInput.tText.iCursor,
			pApp->tInput.tText.iSelectStart,
			pApp->tInput.tText.iSelectEnd,
			pApp->tInput.bPassword,
			pApp->tInput.bDisabled,
			(int)strlen(xgeXuiTextEditGetText(&pApp->tEdit)),
			pApp->tEdit.iLineCount,
			pApp->tEdit.iVisualLineCount,
			pApp->tEdit.iUndoCount,
			pApp->tEdit.iRedoCount,
			pApp->tEdit.fScrollX,
			pApp->tEdit.fScrollY,
			pApp->tEdit.bWordWrap);
		printf("xui-text-edit-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	iFrameLimit = ArgInt(getenv("XGE_XUI_TEXT_EDIT_LAB_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			iFrameLimit = ArgInt(argv[++i], iFrameLimit);
		}
	}

	tDesc.iWidth = 760;
	tDesc.iHeight = 460;
	tDesc.sTitle = "XGE XUI Text Edit Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		printf("xui-text-edit-lab init failed\n");
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	AppUnit(&tApp);
	xgeUnit();
	return 0;
}

