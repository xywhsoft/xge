#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

#define LONG_LINE_BYTES (256 * 1024)
#define PATCH_SHAPE_LIMIT (64 * 1024)

#define CHECK(expr, message) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_edit_long_line_test failed: %s\n", (message)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static char* __buildLongLine(void)
{
	static const char sPattern[] = "value_123 + other_value; ";
	char* sText;
	int i;

	sText = (char*)xrtMalloc((size_t)LONG_LINE_BYTES + 1u);
	if ( sText == NULL ) return NULL;
	for ( i = 0; i < LONG_LINE_BYTES; i++ ) {
		sText[i] = sPattern[i % ((int)sizeof(sPattern) - 1)];
	}
	sText[LONG_LINE_BYTES] = '\0';
	return sText;
}

static double __nowMs(void)
{
	LARGE_INTEGER tFrequency;
	LARGE_INTEGER tCounter;

	QueryPerformanceFrequency(&tFrequency);
	QueryPerformanceCounter(&tCounter);
	return (double)tCounter.QuadPart * 1000.0 / (double)tFrequency.QuadPart;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_code_edit_desc_t tDesc;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL;
	xui_widget pCodeEdit = NULL;
	xui_surface pTarget = NULL;
	xui_font pFont = NULL;
	xui_code_document pDocument;
	xui_code_selection_model pSelection;
	xui_code_selection_t tScrollSelection;
	char* sText = NULL;
	char* sScrollRepro = NULL;
	char sInserted[2];
	size_t iInitialShapeBytes;
	size_t iPatchShapeBytes;
	float fScrollX;
	float fScrollBefore;
	float fScrollAfter;
	double fStartMs;
	double fDispatchMs;
	double fRenderMs;
	double fDocumentMs;
	double fSelectionMs;
	double fEnsureMs;
	int iInsertOffset;
	int iRet;
	int iFailed = 0;

	memset(&tState, 0, sizeof(tState));
	xuiTestProxyInit(&tState);
	xuiTestProxyEnableRetainedText(&tState);
	sText = __buildLongLine();
	CHECK(sText != NULL, "allocate long line");
	iRet = xuiCreate(&pContext);
	CHECK(iRet == XUI_OK && pContext != NULL, "create context");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	CHECK(iRet == XUI_OK, "set proxy");
	iRet = tState.tProxy.fontLoadFile(&tState.tProxy, &pFont,
		"codeedit_long_line.ttf", 14.0f, 0);
	CHECK(iRet == XUI_OK && pFont != NULL, "load font");
	CHECK(xuiSetDefaultFont(pContext, pFont) == XUI_OK, "set default font");
	CHECK(xuiInputViewport(pContext, 900.0f, 300.0f) == XUI_OK, "set viewport");
	CHECK(xuiWidgetCreate(pContext, &pRoot) == XUI_OK && pRoot != NULL,
		"create root");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 900.0f, 300.0f});
	CHECK(xuiSetRootWidget(pContext, pRoot) == XUI_OK, "set root");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sText = sText;
	tDesc.sLanguage = "c";
	tDesc.iTabColumns = 4;
	tDesc.iIndentColumns = 4;
	CHECK(xuiCodeEditCreate(pContext, &pCodeEdit, &tDesc) == XUI_OK &&
		pCodeEdit != NULL, "create code edit");
	xuiWidgetSetRect(pCodeEdit, (xui_rect_t){8.0f, 8.0f, 884.0f, 284.0f});
	CHECK(xuiWidgetAddChild(pRoot, pCodeEdit) == XUI_OK, "attach code edit");
	CHECK(xuiTestSurfaceCreate(&tState, &pTarget, 900, 300,
		XUI_SURFACE_USAGE_TARGET) == XUI_OK && pTarget != NULL,
		"create target");
	CHECK(xuiLayout(pContext) == XUI_OK, "initial layout");
	CHECK(xuiUpdate(pContext, 0.016f) == XUI_OK, "initial update");
	CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "initial render");
	iInitialShapeBytes = xuiTestProxyGetTextShapeBytes(&tState);
	CHECK(iInitialShapeBytes >= LONG_LINE_BYTES,
		"initial render must build the complete chunk cache");
	CHECK(xuiTestProxyGetTextShapeRangeDrawCount(&tState) > 0,
		"initial render must draw retained chunk ranges");

	xuiTestProxyResetTextCounters(&tState);
	CHECK(xuiCodeEditSetScroll(pCodeEdit, 500000.0f, 0.0f) == XUI_OK,
		"set distant horizontal scroll");
	CHECK(xuiCodeEditGetScroll(pCodeEdit, &fScrollX, NULL) == XUI_OK,
		"read horizontal scroll");
	CHECK(fScrollX > 100000.0f, "long line must expose its full scroll width");
	CHECK(xuiUpdate(pContext, 0.016f) == XUI_OK, "scroll update");
	CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "scroll render");
	CHECK(xuiTestProxyGetTextShapeBytes(&tState) < 4096u,
		"scrolling an unchanged line must not reshape it");
	pDocument = xuiCodeEditGetDocument(pCodeEdit);
	pSelection = xuiCodeEditGetSelection(pCodeEdit);
	iInsertOffset = LONG_LINE_BYTES / 2;
	CHECK(xuiCodeSelectionGotoOffset(pSelection, pDocument, iInsertOffset, 0) == XUI_OK,
		"move caret into long line");
	CHECK(xuiSetFocusWidget(pContext, pCodeEdit) == XUI_OK, "focus code edit");
	xuiTestProxyResetTextCounters(&tState);
	fStartMs = __nowMs();
	CHECK(xuiInputText(pContext, (uint32_t)'Z') == XUI_OK, "queue input");
	CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK, "dispatch input");
	fDispatchMs = __nowMs() - fStartMs;
	fStartMs = __nowMs();
	CHECK(xuiUpdate(pContext, 0.016f) == XUI_OK, "edit update");
	CHECK(xuiRender(pContext, pTarget, NULL, 0) == XUI_OK, "edit render");
	fRenderMs = __nowMs() - fStartMs;
	CHECK(xuiCodeDocumentGetLength(pDocument) == LONG_LINE_BYTES + 1,
		"insert must update document length");
	CHECK(xuiCodeDocumentCopyRange(pDocument, iInsertOffset,
		iInsertOffset + 1, sInserted, (int)sizeof(sInserted), NULL) == XUI_OK &&
		sInserted[0] == 'Z', "inserted text must be preserved");
	iPatchShapeBytes = xuiTestProxyGetTextShapeBytes(&tState);
	CHECK(iPatchShapeBytes > 0u && iPatchShapeBytes < PATCH_SHAPE_LIMIT,
		"editing one location must reshape only neighbouring chunks");
	fStartMs = __nowMs();
	CHECK(xuiCodeDocumentInsert(pDocument, iInsertOffset + 1, "Q") == XUI_OK,
		"direct document insert");
	fDocumentMs = __nowMs() - fStartMs;
	fStartMs = __nowMs();
	CHECK(xuiCodeSelectionGotoOffset(pSelection, pDocument,
		iInsertOffset + 2, 0) == XUI_OK, "selection update");
	fSelectionMs = __nowMs() - fStartMs;
	fStartMs = __nowMs();
	CHECK(xuiCodeEditEnsureCaretVisible(pCodeEdit) == XUI_OK,
		"ensure caret visible");
	fEnsureMs = __nowMs() - fStartMs;

	/* The sampled extent must not replace a previously measured long line.
	 * This used to reset scrollX to zero before pointer hit testing. */
	sScrollRepro = (char*)xrtMalloc(2050u + 12288u + 1u);
	CHECK(sScrollRepro != NULL, "allocate horizontal scroll regression text");
	memset(sScrollRepro, '\n', 2050u);
	memset(sScrollRepro + 2050, 'A', 12288u);
	sScrollRepro[2050 + 12288] = '\0';
	CHECK(xuiCodeEditSetText(pCodeEdit, sScrollRepro) == XUI_OK,
		"set horizontal scroll regression text");
	pDocument = xuiCodeEditGetDocument(pCodeEdit);
	pSelection = xuiCodeEditGetSelection(pCodeEdit);
	CHECK(xuiCodeSelectionGotoOffset(pSelection, pDocument,
		xuiCodeDocumentGetLength(pDocument), 0) == XUI_OK,
		"move caret to unsampled longest line");
	CHECK(xuiCodeEditEnsureCaretVisible(pCodeEdit) == XUI_OK,
		"measure unsampled longest line");
	CHECK(xuiCodeEditSetScroll(pCodeEdit, 50000.0f, 100000.0f) == XUI_OK,
		"set distant scroll for pointer regression");
	CHECK(xuiCodeEditGetScroll(pCodeEdit, &fScrollBefore, NULL) == XUI_OK &&
		fScrollBefore > 10000.0f, "pointer regression must start scrolled");
	CHECK(xuiCodeDocumentInsert(pDocument, 0, "x") == XUI_OK,
		"change version outside unsampled longest line");
	CHECK(xuiInputPointerDown(pContext, 450.0f, 270.0f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK,
		"queue horizontally scrolled pointer down");
	CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK,
		"dispatch horizontally scrolled pointer down");
	memset(&tScrollSelection, 0, sizeof(tScrollSelection));
	CHECK(xuiCodeSelectionGetState(pSelection, &tScrollSelection) == XUI_OK &&
		tScrollSelection.iCaretOffset > 2050,
		"horizontal pointer regression must hit the unsampled long line");
	CHECK(xuiCodeEditGetScroll(pCodeEdit, &fScrollAfter, NULL) == XUI_OK &&
		fScrollAfter > fScrollBefore - 0.01f &&
		fScrollAfter < fScrollBefore + 0.01f,
		"pointer down must preserve horizontal scroll");
	CHECK(xuiInputPointerMove(pContext, 1200.0f, 270.0f,
		XUI_POINTER_BUTTON_LEFT) == XUI_OK,
		"queue horizontal drag auto scroll");
	CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK,
		"dispatch horizontal drag auto scroll");
	CHECK(xuiCodeEditGetScroll(pCodeEdit, &fScrollAfter, NULL) == XUI_OK &&
		fScrollAfter > fScrollBefore,
		"horizontal drag must advance scroll");
	CHECK(xuiInputPointerUp(pContext, 1200.0f, 270.0f,
		XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK,
		"queue horizontal drag end");
	CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK,
		"dispatch horizontal drag end");


	printf("xui_code_edit_long_line_test passed: initial_shape_bytes=%zu patch_shape_bytes=%zu scroll_x=%.1f dispatch_ms=%.3f render_ms=%.3f document_ms=%.3f selection_ms=%.3f ensure_ms=%.3f\n",
		iInitialShapeBytes, iPatchShapeBytes, fScrollX, fDispatchMs, fRenderMs,
		fDocumentMs, fSelectionMs, fEnsureMs);

cleanup:
	if ( sScrollRepro != NULL ) xrtFree(sScrollRepro);
	if ( sText != NULL ) xrtFree(sText);
	if ( pTarget != NULL && tState.tProxy.surfaceDestroy != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) xuiDestroy(pContext);
	return iFailed ? 1 : 0;
}
