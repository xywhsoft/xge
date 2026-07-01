#include "xui.h"
#include "xui_test_proxy.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define XUI_PERF_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_edit_large_perf_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static double __xuiPerfNowMs(void)
{
	LARGE_INTEGER tFreq;
	LARGE_INTEGER tNow;

	QueryPerformanceFrequency(&tFreq);
	QueryPerformanceCounter(&tNow);
	return ((double)tNow.QuadPart * 1000.0) / (double)tFreq.QuadPart;
}

static char* __xuiPerfBuildCodeText(int iMegabytes, int iLineBytes, int* pLength)
{
	static const char* sPrefix = "int value = 123; // generated large code line ";
	char* sText;
	int iLength;
	int iPrefixLength;
	int iColumn;
	int i;

	if ( iMegabytes <= 0 ) iMegabytes = 16;
	if ( iLineBytes < 32 ) iLineBytes = 80;
	iLength = iMegabytes * 1024 * 1024;
	iPrefixLength = (int)strlen(sPrefix);
	sText = (char*)xrtMalloc((size_t)iLength + 1u);
	if ( sText == NULL ) return NULL;
	iColumn = 0;
	for ( i = 0; i < iLength; i++ ) {
		if ( iColumn + 1 >= iLineBytes ) {
			sText[i] = '\n';
			iColumn = 0;
		} else {
			sText[i] = (iColumn < iPrefixLength) ? sPrefix[iColumn] : (char)('a' + (iColumn % 26));
			iColumn++;
		}
	}
	sText[iLength] = '\0';
	if ( pLength != NULL ) *pLength = iLength;
	return sText;
}

static int __xuiPerfWriteTempFile(const char* sText, int iLength, char* sPath, int iPathCapacity)
{
	HANDLE hFile;
	DWORD iWritten;
	DWORD iChunk;
	int iOffset;

	if ( sText == NULL || iLength < 0 || sPath == NULL || iPathCapacity <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( GetTempPathA((DWORD)iPathCapacity, sPath) == 0 ) return XUI_ERROR_UNSUPPORTED;
	if ( GetTempFileNameA(sPath, "xui", 0, sPath) == 0 ) return XUI_ERROR_UNSUPPORTED;
	hFile = CreateFileA(sPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) return XUI_ERROR_UNSUPPORTED;
	iOffset = 0;
	while ( iOffset < iLength ) {
		iChunk = (DWORD)((iLength - iOffset) > (1024 * 1024) ? (1024 * 1024) : (iLength - iOffset));
		if ( !WriteFile(hFile, sText + iOffset, iChunk, &iWritten, NULL) || iWritten != iChunk ) {
			CloseHandle(hFile);
			DeleteFileA(sPath);
			sPath[0] = '\0';
			return XUI_ERROR_UNSUPPORTED;
		}
		iOffset += (int)iWritten;
	}
	CloseHandle(hFile);
	return XUI_OK;
}

int main(int argc, char** argv)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pCodeEdit;
	xui_surface pTarget;
	xui_surface pCodeCache;
	xui_font pFont;
	xui_code_edit_desc_t tDesc;
	xui_code_fold_range_t* pFoldRanges;
	char* sText;
	char sTempPath[MAX_PATH];
	double fStartMs;
	double fSetMs;
	double fLoadMs;
	double fFirstFrameMs;
	double fScrollMs;
	double fFoldSetMs;
	double fFoldFirstFrameMs;
	double fFoldScrollMs;
	float fScrollY;
	int iMegabytes;
	int iLineBytes;
	int iFrames;
	int iLength;
	int iLineCount;
	int iFoldVisibleCount;
	int iFoldRangeCount;
	int iFoldRangeCapacity;
	int iTokenCount;
	int iTextDrawCount;
	int bFileLoad;
	int iFrame;
	int iLine;
	int iRet;
	int iFailed;

	iMegabytes = (argc > 1) ? atoi(argv[1]) : 16;
	iLineBytes = (argc > 2) ? atoi(argv[2]) : 80;
	iFrames = (argc > 3) ? atoi(argv[3]) : 20;
	bFileLoad = (argc > 4 && strcmp(argv[4], "file") == 0);
	if ( iMegabytes <= 0 ) iMegabytes = 16;
	if ( iLineBytes < 32 ) iLineBytes = 80;
	if ( iFrames <= 0 ) iFrames = 20;
	pContext = NULL;
	pRoot = NULL;
	pCodeEdit = NULL;
	pTarget = NULL;
	pCodeCache = NULL;
	pFont = NULL;
	pFoldRanges = NULL;
	sText = NULL;
	sTempPath[0] = '\0';
	fSetMs = 0.0;
	fLoadMs = 0.0;
	iFailed = 0;
	memset(&tState, 0, sizeof(tState));
	xuiTestProxyInit(&tState);

	sText = __xuiPerfBuildCodeText(iMegabytes, iLineBytes, &iLength);
	XUI_PERF_CHECK(sText != NULL, "allocate text");
	iRet = xuiCreate(&pContext);
	XUI_PERF_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_PERF_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadFile(&tState.tProxy, &pFont, "codeedit_perf.ttf", 14.0f, 0);
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
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sText = "";
	tDesc.sLanguage = "c";
	tDesc.bReadonly = 1;
	tDesc.bShowLineNumbers = 1;
	tDesc.iTabColumns = 4;
	tDesc.iIndentColumns = 4;
	iRet = xuiCodeEditCreate(pContext, &pCodeEdit, &tDesc);
	XUI_PERF_CHECK(iRet == XUI_OK && pCodeEdit != NULL, "codeedit create");
	xuiWidgetSetRect(pCodeEdit, (xui_rect_t){12.0f, 12.0f, 936.0f, 616.0f});
	iRet = xuiWidgetAddChild(pRoot, pCodeEdit);
	XUI_PERF_CHECK(iRet == XUI_OK, "codeedit add");
	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 960, 640, XUI_SURFACE_USAGE_TARGET);
	XUI_PERF_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");

	fStartMs = __xuiPerfNowMs();
	if ( bFileLoad ) {
		iRet = __xuiPerfWriteTempFile(sText, iLength, sTempPath, (int)sizeof(sTempPath));
		xrtFree(sText);
		sText = NULL;
		XUI_PERF_CHECK(iRet == XUI_OK, "write temp file");
		fStartMs = __xuiPerfNowMs();
		iRet = xuiCodeEditLoadTextFile(pCodeEdit, sTempPath, XRT_CP_UTF8);
		fLoadMs = __xuiPerfNowMs() - fStartMs;
	} else {
		iRet = xuiCodeEditSetTextLength(pCodeEdit, sText, iLength);
		fSetMs = __xuiPerfNowMs() - fStartMs;
		xrtFree(sText);
		sText = NULL;
	}
	XUI_PERF_CHECK(iRet == XUI_OK, bFileLoad ? "load file" : "set text");
	XUI_PERF_CHECK(xuiCodeDocumentGetLength(xuiCodeEditGetDocument(pCodeEdit)) == iLength, "document length");
	iLineCount = xuiCodeDocumentGetLineCount(xuiCodeEditGetDocument(pCodeEdit));
	XUI_PERF_CHECK(iLineCount > 0, "line count");

	fStartMs = __xuiPerfNowMs();
	iRet = xuiLayout(pContext);
	XUI_PERF_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_PERF_CHECK(iRet == XUI_OK, "update");
	iRet = xuiRender(pContext, pTarget, NULL, 0);
	fFirstFrameMs = __xuiPerfNowMs() - fStartMs;
	XUI_PERF_CHECK(iRet == XUI_OK, "first render");
	pCodeCache = xuiWidgetGetCacheSurface(pCodeEdit, xuiWidgetGetStateId(pCodeEdit));
	XUI_PERF_CHECK(pCodeCache != NULL, "cache surface");
	iTextDrawCount = xuiTestSurfaceGetTextDrawCount(pCodeCache);
	iTokenCount = xuiCodeTokenBufferGetCount(xuiCodeEditGetTokenBuffer(pCodeEdit));
	XUI_PERF_CHECK(iTextDrawCount > 0 && iTokenCount > 0, "first render visible content");

	fStartMs = __xuiPerfNowMs();
	for ( iFrame = 0; iFrame < iFrames; iFrame++ ) {
		fScrollY = (float)(((uint64_t)iLineCount * (uint64_t)(iFrame + 1)) / (uint64_t)(iFrames + 1)) * 18.0f;
		iRet = xuiCodeEditSetScroll(pCodeEdit, 0.0f, fScrollY);
		XUI_PERF_CHECK(iRet == XUI_OK, "set scroll");
		iRet = xuiUpdate(pContext, 0.016f);
		XUI_PERF_CHECK(iRet == XUI_OK, "scroll update");
		iRet = xuiRender(pContext, pTarget, NULL, 0);
		XUI_PERF_CHECK(iRet == XUI_OK, "scroll render");
		pCodeCache = xuiWidgetGetCacheSurface(pCodeEdit, xuiWidgetGetStateId(pCodeEdit));
		XUI_PERF_CHECK(pCodeCache != NULL && xuiTestSurfaceGetTextDrawCount(pCodeCache) > 0, "scroll visible content");
	}
	fScrollMs = __xuiPerfNowMs() - fStartMs;
	iTokenCount = xuiCodeTokenBufferGetCount(xuiCodeEditGetTokenBuffer(pCodeEdit));
	iFoldRangeCapacity = (iLineCount > 1) ? ((iLineCount + 1023) / 1024) : 0;
	iFoldRangeCount = 0;
	iFoldVisibleCount = iLineCount;
	fFoldSetMs = 0.0;
	fFoldFirstFrameMs = 0.0;
	fFoldScrollMs = 0.0;
	if ( iFoldRangeCapacity > 0 ) {
		pFoldRanges = (xui_code_fold_range_t*)xrtMalloc(sizeof(*pFoldRanges) * (size_t)iFoldRangeCapacity);
		XUI_PERF_CHECK(pFoldRanges != NULL, "allocate fold ranges");
		for ( iLine = 0; iLine + 2 < iLineCount; iLine += 1024 ) {
			memset(&pFoldRanges[iFoldRangeCount], 0, sizeof(pFoldRanges[iFoldRangeCount]));
			pFoldRanges[iFoldRangeCount].iSize = sizeof(pFoldRanges[iFoldRangeCount]);
			pFoldRanges[iFoldRangeCount].iStartLine = iLine;
			pFoldRanges[iFoldRangeCount].iEndLine = iLine + 511;
			if ( pFoldRanges[iFoldRangeCount].iEndLine >= iLineCount ) pFoldRanges[iFoldRangeCount].iEndLine = iLineCount - 1;
			pFoldRanges[iFoldRangeCount].iFlags = XUI_CODE_FOLD_COLLAPSED;
			if ( pFoldRanges[iFoldRangeCount].iEndLine > pFoldRanges[iFoldRangeCount].iStartLine ) iFoldRangeCount++;
		}
		fStartMs = __xuiPerfNowMs();
		iRet = xuiCodeFoldStateSetRanges(xuiCodeEditGetFoldState(pCodeEdit), pFoldRanges, iFoldRangeCount);
		XUI_PERF_CHECK(iRet == XUI_OK, "set fold ranges");
		iRet = xuiWidgetInvalidate(pCodeEdit, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		XUI_PERF_CHECK(iRet == XUI_OK, "invalidate folded codeedit");
		fFoldSetMs = __xuiPerfNowMs() - fStartMs;
		(void)xuiCodeFoldStateGetVisibleLineCount(xuiCodeEditGetFoldState(pCodeEdit), iLineCount, &iFoldVisibleCount);
		iRet = xuiCodeEditSetScroll(pCodeEdit, 0.0f, 0.0f);
		XUI_PERF_CHECK(iRet == XUI_OK, "fold scroll reset");
		fStartMs = __xuiPerfNowMs();
		iRet = xuiLayout(pContext);
		XUI_PERF_CHECK(iRet == XUI_OK, "fold layout");
		iRet = xuiUpdate(pContext, 0.016f);
		XUI_PERF_CHECK(iRet == XUI_OK, "fold update");
		iRet = xuiRender(pContext, pTarget, NULL, 0);
		fFoldFirstFrameMs = __xuiPerfNowMs() - fStartMs;
		XUI_PERF_CHECK(iRet == XUI_OK, "fold first render");
		fStartMs = __xuiPerfNowMs();
		for ( iFrame = 0; iFrame < iFrames; iFrame++ ) {
			fScrollY = (float)(((uint64_t)iFoldVisibleCount * (uint64_t)(iFrame + 1)) / (uint64_t)(iFrames + 1)) * 18.0f;
			iRet = xuiCodeEditSetScroll(pCodeEdit, 0.0f, fScrollY);
			XUI_PERF_CHECK(iRet == XUI_OK, "fold set scroll");
			iRet = xuiUpdate(pContext, 0.016f);
			XUI_PERF_CHECK(iRet == XUI_OK, "fold scroll update");
			iRet = xuiRender(pContext, pTarget, NULL, 0);
			XUI_PERF_CHECK(iRet == XUI_OK, "fold scroll render");
			pCodeCache = xuiWidgetGetCacheSurface(pCodeEdit, xuiWidgetGetStateId(pCodeEdit));
			XUI_PERF_CHECK(pCodeCache != NULL && xuiTestSurfaceGetTextDrawCount(pCodeCache) > 0, "fold scroll visible content");
		}
		fFoldScrollMs = __xuiPerfNowMs() - fStartMs;
	}
	printf("xui_code_edit_large_perf_test summary mode=%s mb=%d bytes=%d lines=%d frames=%d set_ms=%.3f load_ms=%.3f first_frame_ms=%.3f scroll_total_ms=%.3f scroll_avg_ms=%.3f fold_ranges=%d fold_visible=%d fold_set_ms=%.3f fold_first_frame_ms=%.3f fold_scroll_total_ms=%.3f fold_scroll_avg_ms=%.3f token_count=%d text_draw_count=%d\n",
		bFileLoad ? "file" : "memory",
		iMegabytes,
		iLength,
		iLineCount,
		iFrames,
		fSetMs,
		fLoadMs,
		fFirstFrameMs,
		fScrollMs,
		fScrollMs / (double)iFrames,
		iFoldRangeCount,
		iFoldVisibleCount,
		fFoldSetMs,
		fFoldFirstFrameMs,
		fFoldScrollMs,
		(iFrames > 0) ? (fFoldScrollMs / (double)iFrames) : 0.0,
		iTokenCount,
		xuiTestSurfaceGetTextDrawCount(pCodeCache));

cleanup:
	if ( pFoldRanges != NULL ) xrtFree(pFoldRanges);
	if ( sText != NULL ) xrtFree(sText);
	if ( sTempPath[0] != '\0' ) DeleteFileA(sTempPath);
	if ( pTarget != NULL && tState.tProxy.surfaceDestroy != NULL ) tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	if ( pContext != NULL ) xuiDestroy(pContext);
	return iFailed ? 1 : 0;
}
