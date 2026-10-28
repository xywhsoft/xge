#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

typedef struct input_lifetime_test_t {
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pInput;
	xui_font pFont;
	xui_test_proxy_state_t tProxy;
	const char* sName;
	int iExpectedEditEvent;
	int iCallbackCount;
	int iDecorationClickCount;
	int bContextDestroyed;
	int iFailures;
} input_lifetime_test_t;

#define CHECK(pTest, condition, message) do { if (!(condition)) { \
	printf("FAIL %s: %s (%d)\n", (pTest)->sName, (message), __LINE__); \
	(pTest)->iFailures++; \
} } while (0)

static int setup_test(input_lifetime_test_t* pTest, const char* sName)
{
	int iRet;

	memset(pTest, 0, sizeof(*pTest));
	pTest->sName = sName;
	xuiTestProxyInit(&pTest->tProxy);
	iRet = xuiCreate(&pTest->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pTest->pContext, &pTest->tProxy.tProxy);
	if ( iRet == XUI_OK ) iRet = pTest->tProxy.tProxy.fontLoadFile(
		&pTest->tProxy.tProxy, &pTest->pFont, "test.ttf", 14, 0);
	if ( iRet == XUI_OK ) iRet = xuiSetDefaultFont(pTest->pContext, pTest->pFont);
	if ( iRet == XUI_OK ) iRet = xuiSetViewportSize(pTest->pContext, 320, 120);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreate(pTest->pContext, &pTest->pRoot);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pTest->pRoot, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiInputCreate(pTest->pContext, &pTest->pInput, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pTest->pInput, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pTest->pInput,
		(xui_rect_t){10, 10, 240, 36});
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pTest->pRoot, pTest->pInput);
	if ( iRet == XUI_OK ) iRet = xuiSetRootWidget(pTest->pContext, pTest->pRoot);
	if ( iRet == XUI_OK ) iRet = xuiUpdate(pTest->pContext, 0.0f);
	return iRet;
}

static void cleanup_test(input_lifetime_test_t* pTest)
{
	if ( !pTest->bContextDestroyed && pTest->pContext != NULL ) {
		xuiDestroy(pTest->pContext);
	}
	if ( pTest->pFont != NULL ) {
		pTest->tProxy.tProxy.fontDestroy(&pTest->tProxy.tProxy, pTest->pFont);
	}
}

static void destroy_on_change(xui_widget pWidget, const char* sText, void* pUser)
{
	input_lifetime_test_t* pTest = (input_lifetime_test_t*)pUser;
	(void)sText;
	pTest->iCallbackCount++;
	if ( strcmp(pTest->sName, "change_destroy_context") == 0 ) {
		xuiDestroy(pTest->pContext);
		pTest->bContextDestroyed = 1;
	} else {
		xuiWidgetDestroy(pWidget);
	}
}

static int destroy_on_edit_event(xui_widget pWidget,
	const xui_edit_event_t* pEvent, void* pUser)
{
	input_lifetime_test_t* pTest = (input_lifetime_test_t*)pUser;
	if ( pEvent->iType != pTest->iExpectedEditEvent ) return XUI_OK;
	pTest->iCallbackCount++;
	xuiWidgetDestroy(pWidget);
	return XUI_OK;
}

static void decoration_clicked(xui_widget pWidget,
	xui_input_decoration pDecoration, void* pUser)
{
	input_lifetime_test_t* pTest = (input_lifetime_test_t*)pUser;
	(void)pWidget;
	(void)pDecoration;
	pTest->iDecorationClickCount++;
}

static int dispatch_text(input_lifetime_test_t* pTest, uint32_t iCodepoint)
{
	int iRet = xuiInputText(pTest->pContext, iCodepoint);
	return (iRet == XUI_OK) ? xuiDispatchPendingEvents(pTest->pContext) : iRet;
}

static void check_widget_destroyed(input_lifetime_test_t* pTest)
{
	CHECK(pTest, xuiWidgetGetChildCount(pTest->pRoot) == 0,
		"destroyed Input is detached from its parent");
	CHECK(pTest, xuiUpdate(pTest->pContext, 0.0f) == XUI_OK,
		"context remains usable after Input destruction");
}

static int run_case(const char* sName)
{
	input_lifetime_test_t tTest;
	xui_edit_behavior_t tBehavior;
	xui_input_decoration_desc_t tDecorationDesc;
	xui_input_decoration pDecoration = NULL;
	xui_rect_t tDecorationRect;
	xui_rect_t tWorld;
	int iRet;
	int iX;
	int iY;

	iRet = setup_test(&tTest, sName);
	if ( iRet != XUI_OK ) {
		printf("FAIL %s: setup (%d)\n", sName, iRet);
		cleanup_test(&tTest);
		return 1;
	}

	if ( strcmp(sName, "change_destroy_widget") == 0 ||
	     strcmp(sName, "change_destroy_context") == 0 ) {
		CHECK(&tTest, xuiSetFocusWidget(tTest.pContext, tTest.pInput) == XUI_OK,
			"focus Input");
		CHECK(&tTest, xuiInputSetChange(tTest.pInput, destroy_on_change, &tTest) == XUI_OK,
			"set destructive change callback");
		iRet = dispatch_text(&tTest, 'A');
		CHECK(&tTest, iRet == XUI_OK || (tTest.bContextDestroyed &&
			(iRet == XUI_ERROR_INVALID_STATE || iRet == XUI_ERROR_INVALID_ARGUMENT)),
			"dispatch destructive change callback");
		CHECK(&tTest, tTest.iCallbackCount == 1, "change callback runs once");
		if ( !tTest.bContextDestroyed ) check_widget_destroyed(&tTest);
	} else if ( strcmp(sName, "edit_text_destroy_widget") == 0 ) {
		CHECK(&tTest, xuiSetFocusWidget(tTest.pContext, tTest.pInput) == XUI_OK,
			"focus Input");
		tTest.iExpectedEditEvent = XUI_EDIT_EVENT_TEXT_CHANGED;
		CHECK(&tTest, xuiEditSetEvent(tTest.pInput, destroy_on_edit_event, &tTest) == XUI_OK,
			"set destructive edit callback");
		CHECK(&tTest, dispatch_text(&tTest, 'B') == XUI_OK,
			"dispatch destructive text edit event");
		CHECK(&tTest, tTest.iCallbackCount == 1, "text edit callback runs once");
		check_widget_destroyed(&tTest);
	} else if ( strcmp(sName, "selection_destroy_widget") == 0 ) {
		CHECK(&tTest, xuiInputSetText(tTest.pInput, "abc") == XUI_OK, "seed text");
		tTest.iExpectedEditEvent = XUI_EDIT_EVENT_SELECTION_CHANGED;
		CHECK(&tTest, xuiEditSetEvent(tTest.pInput, destroy_on_edit_event, &tTest) == XUI_OK,
			"set destructive selection callback");
		CHECK(&tTest, xuiInputSetSelection(tTest.pInput, 0, 1) == XUI_OK,
			"set selection with destructive callback");
		CHECK(&tTest, tTest.iCallbackCount == 1, "selection callback runs once");
		check_widget_destroyed(&tTest);
	} else if ( strcmp(sName, "validation_destroy_widget") == 0 ) {
		tTest.iExpectedEditEvent = XUI_EDIT_EVENT_VALIDATION_CHANGED;
		CHECK(&tTest, xuiEditSetEvent(tTest.pInput, destroy_on_edit_event, &tTest) == XUI_OK,
			"set destructive validation callback");
		CHECK(&tTest, xuiInputSetError(tTest.pInput, 1) == XUI_OK,
			"set error with destructive callback");
		CHECK(&tTest, tTest.iCallbackCount == 1, "validation callback runs once");
		check_widget_destroyed(&tTest);
	} else if ( strcmp(sName, "ime_destroy_widget") == 0 ) {
		CHECK(&tTest, xuiSetFocusWidget(tTest.pContext, tTest.pInput) == XUI_OK,
			"focus Input for IME");
		tTest.iExpectedEditEvent = XUI_EDIT_EVENT_COMPOSITION_CHANGED;
		CHECK(&tTest, xuiEditSetEvent(tTest.pInput, destroy_on_edit_event, &tTest) == XUI_OK,
			"set destructive IME callback");
		CHECK(&tTest, xuiInputImeComposition(tTest.pContext, "a", 1, 0, 1) == XUI_OK,
			"queue IME preedit");
		CHECK(&tTest, xuiDispatchPendingEvents(tTest.pContext) == XUI_OK,
			"dispatch destructive IME callback");
		CHECK(&tTest, tTest.iCallbackCount == 1, "IME callback runs once");
		check_widget_destroyed(&tTest);
	} else if ( strcmp(sName, "focus_selection_destroy_widget") == 0 ) {
		CHECK(&tTest, xuiInputSetText(tTest.pInput, "focus") == XUI_OK, "seed focus text");
		memset(&tBehavior, 0, sizeof(tBehavior));
		tBehavior.iSize = sizeof(tBehavior);
		tBehavior.bSelectAllOnFocus = 1;
		CHECK(&tTest, xuiEditSetBehavior(tTest.pInput, &tBehavior) == XUI_OK,
			"enable focus selection");
		tTest.iExpectedEditEvent = XUI_EDIT_EVENT_SELECTION_CHANGED;
		CHECK(&tTest, xuiEditSetEvent(tTest.pInput, destroy_on_edit_event, &tTest) == XUI_OK,
			"set destructive focus-selection callback");
		CHECK(&tTest, xuiInputPointerDown(tTest.pContext, 20, 20,
			XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK,
			"queue pointer focus");
		CHECK(&tTest, xuiDispatchPendingEvents(tTest.pContext) == XUI_OK,
			"dispatch pointer focus destruction");
		CHECK(&tTest, tTest.iCallbackCount == 1, "focus selection callback runs once");
		check_widget_destroyed(&tTest);
	} else if ( strcmp(sName, "pointer_selection_destroy_widget") == 0 ) {
		CHECK(&tTest, xuiInputSetText(tTest.pInput, "pointer") == XUI_OK,
			"seed pointer text");
		CHECK(&tTest, xuiSetFocusWidget(tTest.pContext, tTest.pInput) == XUI_OK,
			"focus Input before pointer selection");
		CHECK(&tTest, xuiInputSetSelection(tTest.pInput, 0, 0) == XUI_OK,
			"place pointer-selection cursor");
		tTest.iExpectedEditEvent = XUI_EDIT_EVENT_SELECTION_CHANGED;
		CHECK(&tTest, xuiEditSetEvent(tTest.pInput, destroy_on_edit_event, &tTest) == XUI_OK,
			"set destructive pointer-selection callback");
		CHECK(&tTest, xuiInputPointerDown(tTest.pContext, 180, 28,
			XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK,
			"queue pointer selection");
		CHECK(&tTest, xuiDispatchPendingEvents(tTest.pContext) == XUI_OK,
			"dispatch pointer selection destruction");
		CHECK(&tTest, tTest.iCallbackCount == 1, "pointer selection callback runs once");
		check_widget_destroyed(&tTest);
	} else if ( strcmp(sName, "undo_destroy_widget") == 0 ) {
		CHECK(&tTest, xuiSetFocusWidget(tTest.pContext, tTest.pInput) == XUI_OK,
			"focus Input for undo");
		CHECK(&tTest, dispatch_text(&tTest, 'U') == XUI_OK, "create undo history");
		CHECK(&tTest, xuiInputSetChange(tTest.pInput, destroy_on_change, &tTest) == XUI_OK,
			"set destructive undo callback");
		CHECK(&tTest, xuiInputUndo(tTest.pInput) == XUI_OK,
			"undo with destructive callback");
		CHECK(&tTest, tTest.iCallbackCount == 1, "undo callback runs once");
		check_widget_destroyed(&tTest);
	} else if ( strcmp(sName, "clear_decoration_destroy_widget") == 0 ) {
		memset(&tDecorationDesc, 0, sizeof(tDecorationDesc));
		tDecorationDesc.iSize = sizeof(tDecorationDesc);
		tDecorationDesc.iKind = XUI_INPUT_DECORATION_CLEAR;
		tDecorationDesc.iVisibleMode = XUI_INPUT_DECORATION_VISIBLE_ALWAYS;
		tDecorationDesc.fWidth = 22.0f;
		tDecorationDesc.onClick = decoration_clicked;
		tDecorationDesc.pUser = &tTest;
		CHECK(&tTest, xuiInputDecorationAdd(tTest.pInput,
			XUI_INPUT_DECORATION_SIDE_TRAILING, &pDecoration, &tDecorationDesc) == XUI_OK,
			"add clear decoration");
		CHECK(&tTest, xuiInputSetText(tTest.pInput, "clear") == XUI_OK, "seed clear text");
		CHECK(&tTest, xuiInputSetChange(tTest.pInput, destroy_on_change, &tTest) == XUI_OK,
			"set destructive clear callback");
		CHECK(&tTest, xuiUpdate(tTest.pContext, 0.0f) == XUI_OK, "layout decoration");
		tDecorationRect = xuiInputDecorationGetRect(tTest.pInput, pDecoration);
		tWorld = xuiWidgetGetWorldRect(tTest.pInput);
		iX = (int)(tWorld.fX + tDecorationRect.fX + tDecorationRect.fW * 0.5f);
		iY = (int)(tWorld.fY + tDecorationRect.fY + tDecorationRect.fH * 0.5f);
		CHECK(&tTest, tDecorationRect.fW > 0, "clear decoration has geometry");
		CHECK(&tTest, xuiInputPointerDown(tTest.pContext, iX, iY,
			XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK,
			"queue clear pointer down");
		CHECK(&tTest, xuiDispatchPendingEvents(tTest.pContext) == XUI_OK,
			"dispatch clear pointer down");
		CHECK(&tTest, xuiInputPointerUp(tTest.pContext, iX, iY,
			XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK, "queue clear pointer up");
		CHECK(&tTest, xuiDispatchPendingEvents(tTest.pContext) == XUI_OK,
			"dispatch destructive clear callback");
		CHECK(&tTest, tTest.iCallbackCount == 1, "clear change callback runs once");
		CHECK(&tTest, tTest.iDecorationClickCount == 0,
			"destroyed clear decoration callback is not invoked");
		check_widget_destroyed(&tTest);
	}

	cleanup_test(&tTest);
	if ( tTest.iFailures == 0 ) printf("PASS %s\n", sName);
	return tTest.iFailures != 0;
}

int main(void)
{
	static const char* arrCases[] = {
		"change_destroy_widget", "change_destroy_context",
		"edit_text_destroy_widget", "selection_destroy_widget",
		"validation_destroy_widget", "ime_destroy_widget",
		"focus_selection_destroy_widget", "pointer_selection_destroy_widget",
		"undo_destroy_widget",
		"clear_decoration_destroy_widget"
	};
	int i;
	int iFailed = 0;

	for ( i = 0; i < (int)(sizeof(arrCases) / sizeof(arrCases[0])); i++ ) {
		iFailed += run_case(arrCases[i]);
	}
	printf("Input lifetime: %d passed, %d failed\n",
		(int)(sizeof(arrCases) / sizeof(arrCases[0])) - iFailed, iFailed);
	return iFailed != 0;
}
