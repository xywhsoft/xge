#include "../../xge.h"
#include <stdio.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_xui_widget pInputWidget;
	xge_xui_input_t tInput;
	int iChangeCount;
	int iSubmitCount;
	int iFilterCount;
	char sLastChange[64];
	char sLastSubmit[64];
} app_state_t;

static void MakeTextEvent(xge_event_t* pEvent, uint32_t iCodepoint)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_TEXT;
	pEvent->iCodepoint = iCodepoint;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static void MakeCtrlKeyEvent(xge_event_t* pEvent, int iKey)
{
	MakeKeyEvent(pEvent, iKey);
	pEvent->iParam2 = XGE_KEY_MOD_CTRL;
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void CopyText(char* sDst, int iSize, const char* sText)
{
	if ( (sDst == NULL) || (iSize <= 0) ) {
		return;
	}
	if ( sText == NULL ) {
		sText = "";
	}
	strncpy(sDst, sText, (size_t)iSize - 1u);
	sDst[iSize - 1] = 0;
}

static void InputChange(xge_xui_widget pWidget, const char* sText, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iChangeCount++;
		CopyText(pApp->sLastChange, (int)sizeof(pApp->sLastChange), sText);
	}
}

static void InputSubmit(xge_xui_widget pWidget, const char* sText, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSubmitCount++;
		CopyText(pApp->sLastSubmit, (int)sizeof(pApp->sLastSubmit), sText);
	}
}

static int InputFilter(xge_xui_widget pWidget, const char* sOldText, const char* sNewText, void* pUser)
{
	app_state_t* pApp;
	const char* s;

	(void)pWidget;
	(void)sOldText;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iFilterCount++;
	}
	if ( sNewText == NULL ) {
		return 1;
	}
	for ( s = sNewText; *s != 0; s++ ) {
		if ( (*s >= '0') && (*s <= '9') ) {
			return 0;
		}
	}
	return 1;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return 0;
	}
	pApp->pInputWidget = xgeXuiWidgetCreate();
	if ( pApp->pInputWidget == NULL ) {
		return 0;
	}
	xgeXuiWidgetSetRect(pApp->pInputWidget, (xge_rect_t){ 20.0f, 20.0f, 180.0f, 24.0f });
	xgeXuiWidgetAdd(pRoot, pApp->pInputWidget);
	if ( xgeXuiInputInit(&pApp->tInput, &pApp->tXui, pApp->pInputWidget, NULL) != XGE_OK ) {
		return 0;
	}
	xgeXuiInputSetChange(&pApp->tInput, InputChange, pApp);
	xgeXuiInputSetSubmit(&pApp->tInput, InputSubmit, pApp);
	xgeXuiSetFocus(&pApp->tXui, pApp->pInputWidget);
	return 1;
}

static int TestInputStandard(app_state_t* pApp)
{
	xge_event_t tEvent;
	int bChangeOK;
	int bMaxOK;
	int bSubmitOK;
	int bReadonlyOK;
	int bPasswordOK;
	int bCandidateOK;
	int bFilterOK;
	int bDeleteOK;
	int bErrorOK;
	int bClearOK;
	int bIconOK;
	int iBefore;
	xge_rect_t tClear;
	xge_rect_t tPrefix;
	xge_rect_t tSuffix;
	xge_rect_t tSuffixWithClear;
	xge_rect_t tCandidate;
	int iSelStart;
	int iSelEnd;

	xgeXuiInputSetText(&pApp->tInput, "abc");
	bChangeOK = (pApp->iChangeCount == 1) &&
		(pApp->tInput.iChangeCount == 1) &&
		(strcmp(pApp->sLastChange, "abc") == 0);

	xgeXuiInputSetMaxLength(&pApp->tInput, 3);
	iBefore = pApp->iChangeCount;
	MakeTextEvent(&tEvent, 'd');
	bMaxOK = (xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "abc") == 0) &&
		(xgeXuiInputGetMaxLength(&pApp->tInput) == 3) &&
		(pApp->iChangeCount == iBefore);

	xgeXuiInputSetSelection(&pApp->tInput, 0, 3);
	MakeTextEvent(&tEvent, 'Z');
	bChangeOK = bChangeOK &&
		(xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "Z") == 0) &&
		(pApp->iChangeCount == 2) &&
		(strcmp(pApp->sLastChange, "Z") == 0);

	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	bSubmitOK = (xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->iSubmitCount == 1) &&
		(pApp->tInput.iSubmitCount == 1) &&
		(strcmp(pApp->sLastSubmit, "Z") == 0);

	xgeXuiInputSetReadonly(&pApp->tInput, 1);
	iBefore = pApp->iChangeCount;
	MakeTextEvent(&tEvent, 'Q');
	bReadonlyOK = (xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "Z") == 0) &&
		(pApp->iChangeCount == iBefore);
	xgeXuiInputSetSelection(&pApp->tInput, 0, 1);
	MakeCtrlKeyEvent(&tEvent, 'A');
	bReadonlyOK = bReadonlyOK &&
		(xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	xgeXuiInputGetSelection(&pApp->tInput, &iSelStart, &iSelEnd);
	bReadonlyOK = bReadonlyOK &&
		(iSelStart == 0) &&
		(iSelEnd == pApp->tInput.tText.iSize);
	MakeCtrlKeyEvent(&tEvent, 'X');
	bReadonlyOK = bReadonlyOK &&
		(xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "Z") == 0) &&
		(pApp->iChangeCount == iBefore);
	MakeKeyEvent(&tEvent, XGE_KEY_BACKSPACE);
	bReadonlyOK = bReadonlyOK &&
		(xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "Z") == 0) &&
		(pApp->iChangeCount == iBefore);
	xgeXuiInputSetReadonly(&pApp->tInput, 0);

	MakeKeyEvent(&tEvent, XGE_KEY_BACKSPACE);
	bDeleteOK = (xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "") == 0) &&
		(pApp->iChangeCount == 3);

	xgeXuiInputSetText(&pApp->tInput, "abcd");
	bMaxOK = bMaxOK &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "abc") == 0) &&
		(pApp->iChangeCount == 4);

	xgeXuiInputSetError(&pApp->tInput, 1, "Required");
	xgeXuiInputSetErrorColors(&pApp->tInput, XGE_COLOR_RGBA(255, 240, 244, 255), XGE_COLOR_RGBA(220, 74, 84, 255), XGE_COLOR_RGBA(190, 54, 66, 255));
	xgeXuiInputPaintProc(pApp->pInputWidget, &pApp->tInput);
	bErrorOK = (xgeXuiInputGetError(&pApp->tInput) == 1) &&
		(strcmp(xgeXuiInputGetErrorText(&pApp->tInput), "Required") == 0) &&
		(pApp->tInput.tErrorTextRect.fW == pApp->pInputWidget->tRect.fW) &&
		(pApp->tInput.tErrorTextRect.fY > pApp->pInputWidget->tRect.fY);
	xgeXuiInputSetError(&pApp->tInput, 0, NULL);
	bErrorOK = bErrorOK &&
		(xgeXuiInputGetError(&pApp->tInput) == 0) &&
		(xgeXuiInputGetErrorText(&pApp->tInput)[0] == 0);

	xgeXuiInputSetClearButton(&pApp->tInput, 1);
	tClear = xgeXuiInputGetClearRect(&pApp->tInput);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, tClear.fX + 8.0f, tClear.fY + 8.0f);
	(void)xgeXuiInputEvent(&pApp->tInput, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, tClear.fX + 8.0f, tClear.fY + 8.0f);
	bClearOK = (xgeXuiInputGetClearButton(&pApp->tInput) == 1) &&
		(tClear.fW == 16.0f) &&
		(pApp->tInput.bClearHover == 1) &&
		(xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "") == 0) &&
		(pApp->tInput.iClearCount == 1) &&
		(pApp->iChangeCount == 5);
	xgeXuiInputSetText(&pApp->tInput, "xy");
	xgeXuiInputSetReadonly(&pApp->tInput, 1);
	tClear = xgeXuiInputGetClearRect(&pApp->tInput);
	iBefore = pApp->iChangeCount;
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, tClear.fX + 8.0f, tClear.fY + 8.0f);
	bClearOK = bClearOK &&
		(xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "xy") == 0) &&
		(pApp->iChangeCount == iBefore) &&
		(pApp->tInput.iClearCount == 1);
	xgeXuiInputSetReadonly(&pApp->tInput, 0);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, tClear.fX + 8.0f, tClear.fY + 8.0f);
	bClearOK = bClearOK &&
		(xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "") == 0) &&
		(pApp->tInput.iClearCount == 2) &&
		(pApp->iChangeCount == 7);

	xgeXuiInputSetClearButton(&pApp->tInput, 0);
	xgeXuiInputSetIcons(&pApp->tInput, XGE_XUI_INPUT_ICON_SEARCH, XGE_XUI_INPUT_ICON_LOCK);
	xgeXuiInputSetIconColor(&pApp->tInput, XGE_COLOR_RGBA(68, 126, 166, 255));
	xgeXuiInputSetText(&pApp->tInput, "abc");
	tPrefix = xgeXuiInputGetPrefixIconRect(&pApp->tInput);
	tSuffix = xgeXuiInputGetSuffixIconRect(&pApp->tInput);
	xgeXuiInputPaintProc(pApp->pInputWidget, &pApp->tInput);
	xgeXuiInputSetClearButton(&pApp->tInput, 1);
	tSuffixWithClear = xgeXuiInputGetSuffixIconRect(&pApp->tInput);
	bIconOK = (pApp->tInput.iPrefixIcon == XGE_XUI_INPUT_ICON_SEARCH) &&
		(pApp->tInput.iSuffixIcon == XGE_XUI_INPUT_ICON_LOCK) &&
		(tPrefix.fW == 14.0f) &&
		(tSuffix.fW == 14.0f) &&
		(tPrefix.fX < pApp->pInputWidget->tRect.fX + 12.0f) &&
		(tSuffix.fX > tPrefix.fX) &&
		(tSuffixWithClear.fX < tSuffix.fX);

	iBefore = pApp->iChangeCount;
	xgeXuiInputSetPassword(&pApp->tInput, 1);
	xgeXuiInputSetSelection(&pApp->tInput, 0, pApp->tInput.tText.iSize);
	MakeCtrlKeyEvent(&tEvent, 'C');
	bPasswordOK = (xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "abc") == 0) &&
		(pApp->iChangeCount == iBefore);
	MakeTextEvent(&tEvent, 0x4e2d);
	bPasswordOK = bPasswordOK &&
		(xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "abc") == 0) &&
		(pApp->iChangeCount == iBefore);
	(void)xgeXuiTextSetComposition(&pApp->tInput.tText, "ime");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_IME_START;
	bPasswordOK = bPasswordOK &&
		(xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiTextGetComposition(&pApp->tInput.tText)[0] == 0);
	xgeXuiInputSetPassword(&pApp->tInput, 0);

	xgeXuiTextSetCursor(&pApp->tInput.tText, pApp->tInput.tText.iSize);
	tCandidate = xgeXuiInputGetCandidateRect(&pApp->tInput);
	bCandidateOK = (tCandidate.fX >= pApp->pInputWidget->tContentRect.fX) &&
		(tCandidate.fX <= pApp->pInputWidget->tContentRect.fX + pApp->pInputWidget->tContentRect.fW) &&
		(tCandidate.fY == pApp->pInputWidget->tContentRect.fY) &&
		(tCandidate.fW == 1.0f) &&
		(tCandidate.fH == pApp->pInputWidget->tContentRect.fH);

	xgeXuiInputSetMaxLength(&pApp->tInput, 0);
	xgeXuiInputSetFilter(&pApp->tInput, InputFilter, pApp);
	iBefore = pApp->iChangeCount;
	xgeXuiInputSetText(&pApp->tInput, "abc1");
	bFilterOK = (strcmp(xgeXuiInputGetText(&pApp->tInput), "abc") == 0) &&
		(pApp->iFilterCount == 1) &&
		(pApp->tInput.iFilterRejectCount == 1) &&
		(pApp->iChangeCount == iBefore);
	xgeXuiInputSetSelection(&pApp->tInput, pApp->tInput.tText.iSize, pApp->tInput.tText.iSize);
	MakeTextEvent(&tEvent, 'd');
	bFilterOK = bFilterOK &&
		(xgeXuiInputEvent(&pApp->tInput, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(strcmp(xgeXuiInputGetText(&pApp->tInput), "abcd") == 0) &&
		(pApp->iFilterCount == 2) &&
		(pApp->tInput.iFilterRejectCount == 1) &&
		(pApp->iChangeCount == iBefore + 1);

	return bChangeOK && bMaxOK && bSubmitOK && bReadonlyOK && bPasswordOK && bCandidateOK && bFilterOK && bDeleteOK && bErrorOK && bClearOK && bIconOK;
}

int main(void)
{
	app_state_t tApp;
	int bCreateOK;
	int bStandardOK;

	memset(&tApp, 0, sizeof(tApp));
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		printf("xui-input-standard-lab init failed\n");
		return 1;
	}
	bCreateOK = CreateUI(&tApp);
	bStandardOK = bCreateOK && TestInputStandard(&tApp);

	printf(
		"xui-input-standard-lab final-summary create=%d standard=%d change=%d/%d submit=%d/%d filter=%d/%d clear=%d icons=%d/%d max=%d error=%d text=%s lastChange=%s lastSubmit=%s\n",
		bCreateOK,
		bStandardOK,
		tApp.iChangeCount,
		tApp.tInput.iChangeCount,
		tApp.iSubmitCount,
		tApp.tInput.iSubmitCount,
		tApp.iFilterCount,
		tApp.tInput.iFilterRejectCount,
		tApp.tInput.iClearCount,
		tApp.tInput.iPrefixIcon,
		tApp.tInput.iSuffixIcon,
		xgeXuiInputGetMaxLength(&tApp.tInput),
		xgeXuiInputGetError(&tApp.tInput),
		xgeXuiInputGetText(&tApp.tInput),
		tApp.sLastChange,
		tApp.sLastSubmit);

	xgeXuiInputUnit(&tApp.tInput);
	xgeXuiUnit(&tApp.tXui);
	return (bCreateOK && bStandardOK) ? 0 : 2;
}
