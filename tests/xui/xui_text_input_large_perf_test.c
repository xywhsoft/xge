#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define XUI_PERF_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_text_input_large_perf_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static double __xuiPerfNowMs(void)
{
	LARGE_INTEGER tFrequency;
	LARGE_INTEGER tNow;
	QueryPerformanceFrequency(&tFrequency);
	QueryPerformanceCounter(&tNow);
	return ((double)tNow.QuadPart * 1000.0) / (double)tFrequency.QuadPart;
}

static char* __xuiPerfBuildText(int iBytes, int iLineBytes)
{
	char* sText;
	int i;
	int iColumn;
	if ( iBytes <= 0 || iLineBytes < 8 ) return NULL;
	sText = (char*)xrtMalloc((size_t)iBytes + 1u);
	if ( sText == NULL ) return NULL;
	iColumn = 0;
	for ( i = 0; i < iBytes; i++ ) {
		if ( iColumn + 1 >= iLineBytes ) {
			sText[i] = '\n';
			iColumn = 0;
		} else {
			sText[i] = (char)('a' + (iColumn % 26));
			iColumn++;
		}
	}
	sText[iBytes] = '\0';
	return sText;
}

static int __xuiPerfDispatch(xui_context pContext)
{
	return xuiDispatchPendingEvents(pContext);
}

int main(int argc, char** argv)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pTextEdit;
	xui_widget pInput;
	xui_surface pTarget;
	xui_font pFont;
	xui_text_edit_desc_t tTextEditDesc;
	xui_input_desc_t tInputDesc;
	char* sText;
	char* sInputText;
	double fStartMs;
	double fTextDragMs;
	double fTextTypingMs;
	double fInputDragMs;
	double fInputTypingMs;
	double fSingleLineTypingMs;
	int iTextBytes;
	int iInputBytes;
	int iMoves;
	int iTypingCount;
	int iInputOriginalLength;
	int bWordWrap;
	int i;
	int iStart;
	int iEnd;
	int iRet;
	int iFailed;

	iTextBytes = (argc > 1) ? atoi(argv[1]) * 1024 * 1024 : 2 * 1024 * 1024;
	iInputBytes = (argc > 2) ? atoi(argv[2]) * 1024 : 512 * 1024;
	iMoves = (argc > 3) ? atoi(argv[3]) : 240;
	bWordWrap = (argc > 4 && strcmp(argv[4], "wrap") == 0);
	if ( iTextBytes <= 0 ) iTextBytes = 2 * 1024 * 1024;
	if ( iInputBytes <= 0 ) iInputBytes = 512 * 1024;
	if ( iMoves <= 0 ) iMoves = 240;
	iTypingCount = 8;
	pContext = NULL;
	pRoot = NULL;
	pTextEdit = NULL;
	pInput = NULL;
	pTarget = NULL;
	pFont = NULL;
	sText = NULL;
	sInputText = NULL;
	iFailed = 0;
	memset(&tState, 0, sizeof(tState));
	xuiTestProxyInit(&tState);
	xuiTestProxyEnableRetainedText(&tState);

	sText = __xuiPerfBuildText(iTextBytes, 80);
	XUI_PERF_CHECK(sText != NULL, "allocate multiline text");
	sInputText = __xuiPerfBuildText(iInputBytes, iInputBytes + 1);
	XUI_PERF_CHECK(sInputText != NULL, "allocate input text");
	iRet = xuiCreate(&pContext);
	XUI_PERF_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_PERF_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadFile(&tState.tProxy, &pFont, "text_input_perf.ttf", 14.0f, 0);
	XUI_PERF_CHECK(iRet == XUI_OK && pFont != NULL, "font load");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_PERF_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 960.0f, 640.0f);
	XUI_PERF_CHECK(iRet == XUI_OK, "viewport");
	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_PERF_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 960.0f, 640.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_PERF_CHECK(iRet == XUI_OK, "root set");

	memset(&tTextEditDesc, 0, sizeof(tTextEditDesc));
	tTextEditDesc.iSize = sizeof(tTextEditDesc);
	tTextEditDesc.pFont = pFont;
	tTextEditDesc.sText = "";
	tTextEditDesc.bWordWrap = bWordWrap;
	iRet = xuiTextEditCreate(pContext, &pTextEdit, &tTextEditDesc);
	XUI_PERF_CHECK(iRet == XUI_OK && pTextEdit != NULL, "text edit create");
	xuiWidgetSetRect(pTextEdit, (xui_rect_t){12.0f, 12.0f, 936.0f, 560.0f});
	iRet = xuiWidgetAddChild(pRoot, pTextEdit);
	XUI_PERF_CHECK(iRet == XUI_OK, "text edit add");
	iRet = xuiTextEditSetText(pTextEdit, sText);
	XUI_PERF_CHECK(iRet == XUI_OK, "text edit set text");
	xrtFree(sText);
	sText = NULL;

	memset(&tInputDesc, 0, sizeof(tInputDesc));
	tInputDesc.iSize = sizeof(tInputDesc);
	tInputDesc.pFont = pFont;
	tInputDesc.sText = "";
	tInputDesc.iMaxLength = iInputBytes + 16;
	iRet = xuiInputCreate(pContext, &pInput, &tInputDesc);
	XUI_PERF_CHECK(iRet == XUI_OK && pInput != NULL, "input create");
	xuiWidgetSetRect(pInput, (xui_rect_t){12.0f, 588.0f, 936.0f, 36.0f});
	iRet = xuiWidgetAddChild(pRoot, pInput);
	XUI_PERF_CHECK(iRet == XUI_OK, "input add");
	iRet = xuiInputSetText(pInput, sInputText);
	XUI_PERF_CHECK(iRet == XUI_OK, "input set text");
	xrtFree(sInputText);
	sInputText = NULL;

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 960, 640, XUI_SURFACE_USAGE_TARGET);
	XUI_PERF_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_PERF_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiSetFocusWidget(pContext, pTextEdit);
	XUI_PERF_CHECK(iRet == XUI_OK, "text edit focus");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_PERF_CHECK(iRet == XUI_OK, "warm update");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_PERF_CHECK(iRet == XUI_OK, "warm render");

	iRet = xuiInputPointerDown(pContext, 24.0f, 28.0f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_PERF_CHECK(iRet == XUI_OK, "text drag begin");
	iRet = __xuiPerfDispatch(pContext);
	XUI_PERF_CHECK(iRet == XUI_OK, "text drag begin dispatch");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_PERF_CHECK(iRet == XUI_OK, "text drag begin render");
	xuiTestProxyResetTextCounters(&tState);
	fStartMs = __xuiPerfNowMs();
	for ( i = 0; i < iMoves; i++ ) {
		float fX = 48.0f + (float)((i * 37) % 820);
		float fY = 36.0f + (float)((i * 23) % 500);
		iRet = xuiInputPointerMove(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT);
		XUI_PERF_CHECK(iRet == XUI_OK, "text drag move");
		iRet = __xuiPerfDispatch(pContext);
		XUI_PERF_CHECK(iRet == XUI_OK, "text drag dispatch");
		iRet = xuiRender(pContext, pTarget, NULL, 0);
		XUI_PERF_CHECK(iRet == XUI_OK, "text drag render");
	}
	fTextDragMs = __xuiPerfNowMs() - fStartMs;
	iRet = xuiInputPointerUp(pContext, 500.0f, 280.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_PERF_CHECK(iRet == XUI_OK, "text drag end");
	iRet = __xuiPerfDispatch(pContext);
	XUI_PERF_CHECK(iRet == XUI_OK, "text drag end dispatch");
	iRet = xuiTextEditGetSelection(pTextEdit, &iStart, &iEnd);
	XUI_PERF_CHECK(iRet == XUI_OK && iEnd > iStart, "text drag selection");
	printf("TextEdit counters: measure=%d shape=%d retained_draw=%d\n",
		xuiTestProxyGetTextMeasureCount(&tState), xuiTestProxyGetTextShapeCount(&tState),
		xuiTestProxyGetTextShapeRangeDrawCount(&tState));
	XUI_PERF_CHECK(xuiTestProxyGetTextMeasureCount(&tState) == 0,
		"text drag must not remeasure prefixes");
	XUI_PERF_CHECK(xuiTestProxyGetTextShapeCount(&tState) == 0,
		"text drag must reuse visible line shapes");
	XUI_PERF_CHECK(xuiTestProxyGetTextShapeRangeDrawCount(&tState) > 0,
		"text drag must use retained shape painting");
	iRet = xuiTextEditSetSelection(pTextEdit, 0, 0);
	XUI_PERF_CHECK(iRet == XUI_OK, "text typing cursor");
	xuiTestProxyResetTextCounters(&tState);
	fStartMs = __xuiPerfNowMs();
	for ( i = 0; i < iTypingCount; i++ ) {
		iRet = xuiInputText(pContext, (uint32_t)('a' + i));
		XUI_PERF_CHECK(iRet == XUI_OK, "text typing input");
		iRet = __xuiPerfDispatch(pContext);
		XUI_PERF_CHECK(iRet == XUI_OK, "text typing dispatch");
		iRet = xuiRender(pContext, pTarget, NULL, 0);
		XUI_PERF_CHECK(iRet == XUI_OK, "text typing render");
	}
	fTextTypingMs = __xuiPerfNowMs() - fStartMs;
	printf("TextEdit typing counters: measure=%d shape=%d shape_bytes=%zu retained_draw=%d\n",
		xuiTestProxyGetTextMeasureCount(&tState), xuiTestProxyGetTextShapeCount(&tState),
		xuiTestProxyGetTextShapeBytes(&tState),
		xuiTestProxyGetTextShapeRangeDrawCount(&tState));
	XUI_PERF_CHECK(xuiTestProxyGetTextShapeBytes(&tState) < (size_t)iTextBytes,
		"text typing must not reshape the full document");
	XUI_PERF_CHECK(strncmp(xuiTextEditGetText(pTextEdit), "abcdefgh", 8) == 0,
		"text typing content");
	XUI_PERF_CHECK(xuiTextEditGetLineCount(pTextEdit) > 1000,
		"text typing preserves line index");
	iRet = xuiTextEditUndo(pTextEdit);
	XUI_PERF_CHECK(iRet == XUI_OK &&
		strncmp(xuiTextEditGetText(pTextEdit), "abcdefghijklmnop", 16) == 0,
		"text typing coalesced undo");
	iRet = xuiTextEditRedo(pTextEdit);
	XUI_PERF_CHECK(iRet == XUI_OK &&
		strncmp(xuiTextEditGetText(pTextEdit), "abcdefghabcdefgh", 16) == 0,
		"text typing coalesced redo");

	iRet = xuiSetFocusWidget(pContext, pInput);
	XUI_PERF_CHECK(iRet == XUI_OK, "input focus");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_PERF_CHECK(iRet == XUI_OK, "input warm render");
	iRet = xuiInputPointerDown(pContext, 120.0f, 606.0f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_PERF_CHECK(iRet == XUI_OK, "input drag begin");
	iRet = __xuiPerfDispatch(pContext);
	XUI_PERF_CHECK(iRet == XUI_OK, "input drag begin dispatch");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_PERF_CHECK(iRet == XUI_OK, "input drag begin render");
	xuiTestProxyResetTextCounters(&tState);
	fStartMs = __xuiPerfNowMs();
	for ( i = 0; i < iMoves; i++ ) {
		float fX = 24.0f + (float)((i * 41) % 900);
		iRet = xuiInputPointerMove(pContext, fX, 606.0f, XUI_POINTER_BUTTON_LEFT);
		XUI_PERF_CHECK(iRet == XUI_OK, "input drag move");
		iRet = __xuiPerfDispatch(pContext);
		XUI_PERF_CHECK(iRet == XUI_OK, "input drag dispatch");
		iRet = xuiRender(pContext, pTarget, NULL, 0);
		XUI_PERF_CHECK(iRet == XUI_OK, "input drag render");
	}
	fInputDragMs = __xuiPerfNowMs() - fStartMs;
	iRet = xuiInputPointerUp(pContext, 720.0f, 606.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_PERF_CHECK(iRet == XUI_OK, "input drag end");
	iRet = __xuiPerfDispatch(pContext);
	XUI_PERF_CHECK(iRet == XUI_OK, "input drag end dispatch");
	printf("Input counters: measure=%d shape=%d retained_draw=%d\n",
		xuiTestProxyGetTextMeasureCount(&tState), xuiTestProxyGetTextShapeCount(&tState),
		xuiTestProxyGetTextShapeRangeDrawCount(&tState));
	XUI_PERF_CHECK(xuiTestProxyGetTextMeasureCount(&tState) == 0,
		"input drag must not remeasure prefixes");
	XUI_PERF_CHECK(xuiTestProxyGetTextShapeCount(&tState) == 0,
		"input drag must reuse the full text shape");
	XUI_PERF_CHECK(xuiTestProxyGetTextShapeRangeDrawCount(&tState) > 0,
		"input drag must use retained shape painting");

	iInputOriginalLength = (int)strlen(xuiInputGetText(pInput));
	iRet = xuiInputSetSelection(pInput, 0, 0);
	XUI_PERF_CHECK(iRet == XUI_OK, "input typing cursor");
	xuiTestProxyResetTextCounters(&tState);
	fStartMs = __xuiPerfNowMs();
	for ( i = 0; i < iTypingCount; i++ ) {
		iRet = xuiInputText(pContext, (uint32_t)('a' + i));
		XUI_PERF_CHECK(iRet == XUI_OK, "input typing input");
		iRet = __xuiPerfDispatch(pContext);
		XUI_PERF_CHECK(iRet == XUI_OK, "input typing dispatch");
		iRet = xuiRender(pContext, pTarget, NULL, 0);
		XUI_PERF_CHECK(iRet == XUI_OK, "input typing render");
	}
	fInputTypingMs = __xuiPerfNowMs() - fStartMs;
	printf("Input typing counters: measure=%d shape=%d shape_bytes=%zu retained_draw=%d\n",
		xuiTestProxyGetTextMeasureCount(&tState), xuiTestProxyGetTextShapeCount(&tState),
		xuiTestProxyGetTextShapeBytes(&tState),
		xuiTestProxyGetTextShapeRangeDrawCount(&tState));
	XUI_PERF_CHECK(xuiTestProxyGetTextMeasureCount(&tState) == 0,
		"input typing must not measure prefixes");
	XUI_PERF_CHECK(xuiTestProxyGetTextShapeBytes(&tState) < (size_t)iInputBytes,
		"input typing must reshape bounded chunks");
	XUI_PERF_CHECK(strncmp(xuiInputGetText(pInput), "abcdefghabcdefgh", 16) == 0,
		"input typing content");
	xuiTestProxyResetTextCounters(&tState);
	iRet = xuiInputUndo(pInput);
	XUI_PERF_CHECK(iRet == XUI_OK && (int)strlen(xuiInputGetText(pInput)) == iInputOriginalLength,
		"input typing undo");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_PERF_CHECK(iRet == XUI_OK, "input undo render");
	iRet = xuiInputRedo(pInput);
	XUI_PERF_CHECK(iRet == XUI_OK &&
		(int)strlen(xuiInputGetText(pInput)) == iInputOriginalLength + iTypingCount,
		"input typing redo");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_PERF_CHECK(iRet == XUI_OK, "input redo render");
	XUI_PERF_CHECK(xuiTestProxyGetTextShapeBytes(&tState) < (size_t)iInputBytes,
		"input undo and redo must reshape bounded chunks");

	sText = __xuiPerfBuildText(iInputBytes, iInputBytes + 1);
	XUI_PERF_CHECK(sText != NULL, "allocate single-line text");
	iRet = xuiTextEditSetText(pTextEdit, sText);
	XUI_PERF_CHECK(iRet == XUI_OK, "single-line text set");
	xrtFree(sText);
	sText = NULL;
	iRet = xuiSetFocusWidget(pContext, pTextEdit);
	XUI_PERF_CHECK(iRet == XUI_OK, "single-line text focus");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_PERF_CHECK(iRet == XUI_OK, "single-line warm render");
	xuiTestProxyResetTextCounters(&tState);
	fStartMs = __xuiPerfNowMs();
	for ( i = 0; i < 3; i++ ) {
		int iPosition = (i == 0) ? 0 : ((i == 1) ? iInputBytes / 2 : iInputBytes);
		iRet = xuiTextEditSetSelection(pTextEdit, iPosition, iPosition);
		XUI_PERF_CHECK(iRet == XUI_OK, "single-line edit cursor");
		iRet = xuiInputText(pContext, (uint32_t)('X' + i));
		XUI_PERF_CHECK(iRet == XUI_OK, "single-line edit input");
		iRet = __xuiPerfDispatch(pContext);
		XUI_PERF_CHECK(iRet == XUI_OK, "single-line edit dispatch");
		iRet = xuiRender(pContext, pTarget, NULL, 0);
		XUI_PERF_CHECK(iRet == XUI_OK, "single-line edit render");
	}
	fSingleLineTypingMs = __xuiPerfNowMs() - fStartMs;
	printf("TextEdit single-line counters: measure=%d shape=%d shape_bytes=%zu retained_draw=%d\n",
		xuiTestProxyGetTextMeasureCount(&tState), xuiTestProxyGetTextShapeCount(&tState),
		xuiTestProxyGetTextShapeBytes(&tState),
		xuiTestProxyGetTextShapeRangeDrawCount(&tState));
	XUI_PERF_CHECK(xuiTestProxyGetTextMeasureCount(&tState) == 0,
		"single-line editing must not measure prefixes");
	XUI_PERF_CHECK(xuiTestProxyGetTextShapeBytes(&tState) < (size_t)iInputBytes,
		"single-line editing must reshape bounded chunks");
	xuiTestProxyResetTextCounters(&tState);
	iRet = xuiTextEditUndo(pTextEdit);
	XUI_PERF_CHECK(iRet == XUI_OK, "single-line edit undo");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_PERF_CHECK(iRet == XUI_OK, "single-line undo render");
	iRet = xuiTextEditRedo(pTextEdit);
	XUI_PERF_CHECK(iRet == XUI_OK, "single-line edit redo");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	XUI_PERF_CHECK(iRet == XUI_OK, "single-line redo render");
	XUI_PERF_CHECK(xuiTestProxyGetTextShapeBytes(&tState) < (size_t)iInputBytes,
		"single-line undo and redo must reshape bounded chunks");

	printf("TextEdit %d bytes (%s): %d drag frames %.3f ms (%.3f ms/frame)\n",
		iTextBytes, bWordWrap ? "wrap" : "nowrap", iMoves, fTextDragMs, fTextDragMs / iMoves);
	printf("TextEdit %d bytes (%s): %d typed frames %.3f ms (%.3f ms/frame)\n",
		iTextBytes, bWordWrap ? "wrap" : "nowrap", iTypingCount,
		fTextTypingMs, fTextTypingMs / iTypingCount);
	printf("Input %d bytes: %d drag frames %.3f ms (%.3f ms/frame)\n",
		iInputBytes, iMoves, fInputDragMs, fInputDragMs / iMoves);
	printf("Input %d bytes: %d typed frames %.3f ms (%.3f ms/frame)\n",
		iInputBytes, iTypingCount, fInputTypingMs, fInputTypingMs / iTypingCount);
	printf("TextEdit %d-byte single line: 3 edit frames %.3f ms (%.3f ms/frame)\n",
		iInputBytes, fSingleLineTypingMs, fSingleLineTypingMs / 3.0);

cleanup:
	if ( sInputText != NULL ) xrtFree(sInputText);
	if ( sText != NULL ) xrtFree(sText);
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( pTarget != NULL && tState.tProxy.surfaceDestroy != NULL )
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	if ( pFont != NULL && tState.tProxy.fontDestroy != NULL )
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	return iFailed ? 1 : 0;
}
