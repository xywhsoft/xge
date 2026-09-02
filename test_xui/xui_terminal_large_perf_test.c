#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define XUI_PERF_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_terminal_large_perf_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static double __xuiTerminalPerfNowMs(void)
{
	LARGE_INTEGER tFrequency;
	LARGE_INTEGER tNow;
	QueryPerformanceFrequency(&tFrequency);
	QueryPerformanceCounter(&tNow);
	return ((double)tNow.QuadPart * 1000.0) / (double)tFrequency.QuadPart;
}

static char* __xuiTerminalPerfBuildOutput(int iMegabytes, int* pSize)
{
	static const char sLine[] =
		"\x1b[32m00000000\x1b[0m generated terminal output with colors and cells 0123456789\r\n";
	char* pOutput;
	int iTarget;
	int iLineSize;
	int iOffset;

	if ( iMegabytes <= 0 ) iMegabytes = 4;
	iTarget = iMegabytes * 1024 * 1024;
	iLineSize = (int)sizeof(sLine) - 1;
	pOutput = (char*)xrtMalloc((size_t)iTarget);
	if ( pOutput == NULL ) return NULL;
	for ( iOffset = 0; iOffset < iTarget; iOffset += iLineSize ) {
		int iCopy = iLineSize;
		if ( iCopy > iTarget - iOffset ) iCopy = iTarget - iOffset;
		memcpy(pOutput + iOffset, sLine, (size_t)iCopy);
	}
	if ( pSize != NULL ) *pSize = iTarget;
	return pOutput;
}

int main(int argc, char** argv)
{
	xui_test_proxy_state_t tProxy;
	xui_context pContext;
	xui_widget pTerminal;
	xui_font pFont;
	xui_terminal_desc_t tDesc;
	xui_terminal_stats_t tStats;
	char* pOutput;
	double fStart;
	double fEnqueueMs;
	double fParseMs;
	double fResizeMs;
	double fFindMs;
	double fThroughput;
	int iMegabytes;
	int iSize;
	int iOffset;
	int iRet;
	int iFailed;

	iMegabytes = argc > 1 ? atoi(argv[1]) : 4;
	if ( iMegabytes <= 0 ) iMegabytes = 4;
	pContext = NULL;
	pTerminal = NULL;
	pFont = NULL;
	pOutput = NULL;
	iFailed = 0;
	memset(&tProxy, 0, sizeof(tProxy));
	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tStats, 0, sizeof(tStats));
	xuiTestProxyInit(&tProxy);
	pOutput = __xuiTerminalPerfBuildOutput(iMegabytes, &iSize);
	XUI_PERF_CHECK(pOutput != NULL, "allocate generated output");
	iRet = xuiCreate(&pContext);
	XUI_PERF_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tProxy.tProxy);
	XUI_PERF_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tProxy.tProxy.fontLoadMemory(&tProxy.tProxy, &pFont,
		"terminal-perf", 13, 13.0f, XUI_FONT_FORMAT_TTF);
	XUI_PERF_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	(void)xuiSetDefaultFont(pContext, pFont);
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.iColumns = 120;
	tDesc.iRows = 40;
	tDesc.iScrollbackLimit = 20000;
	tDesc.iParseBudgetBytes = 65536;
	tDesc.fCellWidth = 8.0f;
	tDesc.fCellHeight = 16.0f;
	tDesc.fPadding = 4.0f;
	iRet = xuiTerminalCreate(pContext, &pTerminal, &tDesc);
	XUI_PERF_CHECK(iRet == XUI_OK && pTerminal != NULL, "terminal create");

	fStart = __xuiTerminalPerfNowMs();
	for ( iOffset = 0; iOffset < iSize; iOffset += 65536 ) {
		int iChunk = iSize - iOffset;
		if ( iChunk > 65536 ) iChunk = 65536;
		iRet = xuiTerminalWrite(pTerminal, pOutput + iOffset, iChunk);
		XUI_PERF_CHECK(iRet == XUI_OK, "enqueue output");
	}
	fEnqueueMs = __xuiTerminalPerfNowMs() - fStart;
	fStart = __xuiTerminalPerfNowMs();
	iRet = xuiTerminalFlush(pTerminal);
	fParseMs = __xuiTerminalPerfNowMs() - fStart;
	XUI_PERF_CHECK(iRet == XUI_OK, "parse output");
	fStart = __xuiTerminalPerfNowMs();
	iRet = xuiTerminalResize(pTerminal, 72, 40);
	if ( iRet == XUI_OK ) iRet = xuiTerminalResize(pTerminal, 160, 40);
	fResizeMs = __xuiTerminalPerfNowMs() - fStart;
	XUI_PERF_CHECK(iRet == XUI_OK, "resize reflow");
	fStart = __xuiTerminalPerfNowMs();
	iRet = xuiTerminalFindText(pTerminal, "generated terminal output", 0, NULL, NULL);
	fFindMs = __xuiTerminalPerfNowMs() - fStart;
	XUI_PERF_CHECK(iRet == 1, "find generated output");
	iRet = xuiTerminalGetStats(pTerminal, &tStats);
	XUI_PERF_CHECK(iRet == XUI_OK && tStats.iQueuedBytes == 0 &&
		tStats.iOutputBytesReceived == (uint64_t)iSize &&
		tStats.iOutputBytesParsed == (uint64_t)iSize,
		"runtime statistics");
	fThroughput = fParseMs > 0.0 ? ((double)iSize / (1024.0 * 1024.0)) / (fParseMs / 1000.0) : 0.0;
	printf("xui_terminal_large_perf_test size=%dMiB enqueue=%.2fms parse=%.2fms throughput=%.2fMiB/s resize=%.2fms find=%.2fms history=%d/%d memory=%zuKiB queuePeak=%d\n",
		iMegabytes, fEnqueueMs, fParseMs, fThroughput, fResizeMs, fFindMs,
		tStats.iHistoryLogicalLines, tStats.iHistoryDisplayRows,
		(tStats.iScreenMemoryBytes + tStats.iHistoryMemoryBytes + tStats.iQueueMemoryBytes) / 1024u,
		tStats.iQueuePeakBytes);
	XUI_PERF_CHECK(fThroughput >= 1.0, "parser throughput below 1 MiB/s");

cleanup:
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( pOutput != NULL ) xrtFree(pOutput);
	return iFailed ? 1 : 0;
}
