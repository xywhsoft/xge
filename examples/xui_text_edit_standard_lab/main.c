#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_xui_widget pEditWidget;
	xge_xui_text_edit_t tEdit;
	xge_xui_text_edit_highlight_t arrHighlights[2];
	int bReadonlyOK;
	int bUndoOK;
	int bTabOK;
	int bReserveOK;
	int bStyleOK;
} app_state_t;

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

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return 0;
	}
	pApp->pEditWidget = xgeXuiWidgetCreate();
	if ( pApp->pEditWidget == NULL ) {
		return 0;
	}
	xgeXuiWidgetSetRect(pApp->pEditWidget, (xge_rect_t){ 20.0f, 20.0f, 260.0f, 120.0f });
	xgeXuiWidgetAdd(pRoot, pApp->pEditWidget);
	if ( xgeXuiTextEditInit(&pApp->tEdit, &pApp->tXui, pApp->pEditWidget, NULL) != XGE_OK ) {
		return 0;
	}
	xgeXuiTextEditSetText(&pApp->tEdit, "alpha\nbeta");
	xgeXuiSetFocus(&pApp->tXui, pApp->pEditWidget);
	return 1;
}

static int TestTextEditStandard(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tContentBefore;
	const xge_xui_state_style_t* pStateStyle;
	int iStart;
	int iEnd;

	xgeXuiTextEditSetReadonly(&pApp->tEdit, 1);
	MakeKeyEvent(&tEvent, 'A', XGE_KEY_MOD_CTRL);
	pApp->bReadonlyOK = (xgeXuiTextEditEvent(&pApp->tEdit, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	xgeXuiTextGetSelection(&pApp->tEdit.tText, &iStart, &iEnd);
	pApp->bReadonlyOK = pApp->bReadonlyOK &&
		(iStart == 0) &&
		(iEnd == pApp->tEdit.tText.iSize);
	MakeTextEvent(&tEvent, 'Z');
	pApp->bReadonlyOK = pApp->bReadonlyOK &&
		(xgeXuiTextEditEvent(&pApp->tEdit, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "alpha\nbeta") == 0) &&
		(pApp->tEdit.iUndoCount == 0);
	MakeKeyEvent(&tEvent, XGE_KEY_BACKSPACE, 0);
	pApp->bReadonlyOK = pApp->bReadonlyOK &&
		(xgeXuiTextEditEvent(&pApp->tEdit, &tEvent) == XGE_XUI_EVENT_CONTINUE) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "alpha\nbeta") == 0);
	xgeXuiTextEditSetReadonly(&pApp->tEdit, 0);

	xgeXuiTextSetCursor(&pApp->tEdit.tText, pApp->tEdit.tText.iSize);
	MakeTextEvent(&tEvent, '!');
	pApp->bUndoOK = (xgeXuiTextEditEvent(&pApp->tEdit, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "alpha\nbeta!") == 0) &&
		(pApp->tEdit.iUndoCount == 1) &&
		(xgeXuiTextEditUndo(&pApp->tEdit) == XGE_OK) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "alpha\nbeta") == 0) &&
		(pApp->tEdit.iRedoCount == 1) &&
		(xgeXuiTextEditRedo(&pApp->tEdit) == XGE_OK) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "alpha\nbeta!") == 0);

	MakeKeyEvent(&tEvent, XGE_KEY_TAB, 0);
	pApp->bTabOK = (xgeXuiTextEditEvent(&pApp->tEdit, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "alpha\nbeta!\t") == 0) &&
		(pApp->tEdit.iUndoCount >= 2);
	pApp->bTabOK = pApp->bTabOK &&
		(xgeXuiTextEditUndo(&pApp->tEdit) == XGE_OK) &&
		(strcmp(xgeXuiTextEditGetText(&pApp->tEdit), "alpha\nbeta!") == 0);

	pApp->arrHighlights[0].iStart = 0;
	pApp->arrHighlights[0].iEnd = 5;
	pApp->arrHighlights[1].iStart = 6;
	pApp->arrHighlights[1].iEnd = 10;
	xgeXuiTextEditSetFindHighlights(&pApp->tEdit, pApp->arrHighlights, 2);
	xgeXuiTextEditSetLineNumbers(&pApp->tEdit, 1, 36.0f);
	xgeXuiTextEditSetReserveColors(
		&pApp->tEdit,
		XGE_COLOR_RGBA(255, 222, 92, 96),
		XGE_COLOR_RGBA(92, 118, 142, 255),
		XGE_COLOR_RGBA(232, 242, 250, 255));
	tContentBefore = pApp->pEditWidget->tContentRect;
	xgeXuiTextEditPaintProc(pApp->pEditWidget, &pApp->tEdit);
	pApp->bReserveOK = (pApp->tEdit.arrFindHighlights == pApp->arrHighlights) &&
		(pApp->tEdit.iFindHighlightCount == 2) &&
		(pApp->tEdit.bLineNumbers == 1) &&
		(pApp->tEdit.fLineNumberWidth == 36.0f) &&
		(pApp->pEditWidget->tContentRect.fX == tContentBefore.fX) &&
		(pApp->pEditWidget->tContentRect.fW == tContentBefore.fW);

	xgeXuiTextEditSetFrameColors(
		&pApp->tEdit,
		XGE_COLOR_RGBA(249, 252, 255, 255),
		XGE_COLOR_RGBA(243, 249, 253, 255),
		XGE_COLOR_RGBA(160, 190, 216, 255),
		XGE_COLOR_RGBA(96, 158, 205, 255),
		XGE_COLOR_RGBA(38, 128, 216, 255));
	xgeXuiTextEditSetDisabledColors(
		&pApp->tEdit,
		XGE_COLOR_RGBA(128, 140, 154, 255),
		XGE_COLOR_RGBA(226, 234, 242, 255),
		XGE_COLOR_RGBA(188, 198, 208, 255));
	xgeXuiTextEditSetCurrentLineColor(&pApp->tEdit, XGE_COLOR_RGBA(255, 246, 194, 120));
	xgeXuiTextEditSetScrollbarColors(
		&pApp->tEdit,
		XGE_COLOR_RGBA(255, 255, 255, 255),
		XGE_COLOR_RGBA(184, 223, 245, 255),
		XGE_COLOR_RGBA(104, 142, 178, 245));
	pStateStyle = xgeXuiWidgetGetStateStyle(pApp->pEditWidget, XGE_XUI_STATE_FOCUS);
	pApp->bStyleOK = (pApp->pEditWidget->tStyle.iBackgroundColor == XGE_COLOR_RGBA(249, 252, 255, 255)) &&
		(pApp->pEditWidget->tStyle.iBorderColor == XGE_COLOR_RGBA(160, 190, 216, 255)) &&
		(pStateStyle != NULL) &&
		(pStateStyle->iBorderColor == XGE_COLOR_RGBA(38, 128, 216, 255)) &&
		(pApp->tEdit.iCurrentLineColor == XGE_COLOR_RGBA(255, 246, 194, 120)) &&
		(pApp->tEdit.iScrollbarThumbColor == XGE_COLOR_RGBA(104, 142, 178, 245));

	return pApp->bReadonlyOK && pApp->bUndoOK && pApp->bTabOK && pApp->bReserveOK && pApp->bStyleOK;
}

int main(void)
{
	app_state_t tApp;
	int bCreateOK;
	int bStandardOK;

	memset(&tApp, 0, sizeof(tApp));
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		printf("xui-text-edit-standard-lab init failed\n");
		return 1;
	}
	bCreateOK = CreateUI(&tApp);
	bStandardOK = bCreateOK && TestTextEditStandard(&tApp);
	printf(
		"xui-text-edit-standard-lab final-summary create=%d standard=%d readonlyOK=%d undoOK=%d tabOK=%d reserveOK=%d styleOK=%d readonly=%d undo=%d redo=%d highlights=%d lineNumbers=%d size=%d\n",
		bCreateOK,
		bStandardOK,
		tApp.bReadonlyOK,
		tApp.bUndoOK,
		tApp.bTabOK,
		tApp.bReserveOK,
		tApp.bStyleOK,
		tApp.tEdit.bReadonly,
		tApp.tEdit.iUndoCount,
		tApp.tEdit.iRedoCount,
		tApp.tEdit.iFindHighlightCount,
		tApp.tEdit.bLineNumbers,
		tApp.tEdit.tText.iSize);
	xgeXuiTextEditUnit(&tApp.tEdit);
	xgeXuiUnit(&tApp.tXui);
	return (bCreateOK && bStandardOK) ? 0 : 2;
}
