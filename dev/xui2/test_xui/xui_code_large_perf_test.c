#include "xui.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static double __xuiPerfNowMs(void)
{
	LARGE_INTEGER tFreq;
	LARGE_INTEGER tNow;

	QueryPerformanceFrequency(&tFreq);
	QueryPerformanceCounter(&tNow);
	return ((double)tNow.QuadPart * 1000.0) / (double)tFreq.QuadPart;
}

static char* __xuiPerfBuildText(int iMegabytes, int iLineBytes, int* pLength)
{
	char* sText;
	int iLength;
	int i;
	int iColumn;

	if ( iMegabytes <= 0 ) iMegabytes = 16;
	if ( iLineBytes < 8 ) iLineBytes = 80;
	iLength = iMegabytes * 1024 * 1024;
	sText = (char*)xrtMalloc((size_t)iLength + 1u);
	if ( sText == NULL ) return NULL;
	iColumn = 0;
	for ( i = 0; i < iLength; i++ ) {
		if ( iColumn + 1 >= iLineBytes ) {
			sText[i] = '\n';
			iColumn = 0;
		} else {
			sText[i] = (char)('a' + (iColumn % 26));
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
	xui_code_document pDocument;
	xui_code_document pLoadDocument;
	char* sText;
	char sTempPath[MAX_PATH];
	double fStartMs;
	double fSetMs;
	double fFileLoadMs;
	double fLineSeekMs;
	double fOffsetSeekMs;
	double fColumnSeekMs;
	uint32_t iSeed;
	uint64_t iChecksum;
	int iMegabytes;
	int iLineBytes;
	int iLength;
	int iLineCount;
	int iFileLineCount;
	int iIterations;
	int bFileLoad;
	int iStart;
	int iEnd;
	int iLine;
	int iColumn;
	int iOffset;
	int iRet;
	int i;

	iMegabytes = (argc > 1) ? atoi(argv[1]) : 16;
	iLineBytes = (argc > 2) ? atoi(argv[2]) : 80;
	iIterations = (argc > 3) ? atoi(argv[3]) : 100000;
	bFileLoad = (argc > 4 && strcmp(argv[4], "file") == 0);
	if ( iMegabytes <= 0 ) iMegabytes = 16;
	if ( iLineBytes < 8 ) iLineBytes = 80;
	if ( iIterations <= 0 ) iIterations = 100000;
	pDocument = NULL;
	pLoadDocument = NULL;
	iChecksum = 0;
	fFileLoadMs = 0.0;
	iFileLineCount = 0;
	sTempPath[0] = '\0';
	sText = __xuiPerfBuildText(iMegabytes, iLineBytes, &iLength);
	if ( sText == NULL ) {
		printf("xui_code_large_perf_test failed: allocate %d MB\n", iMegabytes);
		return 1;
	}
	iRet = xuiCodeDocumentCreate(&pDocument);
	if ( iRet != XUI_OK || pDocument == NULL ) {
		printf("xui_code_large_perf_test failed: create document\n");
		xrtFree(sText);
		return 1;
	}
	fStartMs = __xuiPerfNowMs();
	iRet = xuiCodeDocumentSetTextLength(pDocument, sText, iLength);
	fSetMs = __xuiPerfNowMs() - fStartMs;
	if ( iRet == XUI_OK && bFileLoad ) {
		iRet = __xuiPerfWriteTempFile(sText, iLength, sTempPath, (int)sizeof(sTempPath));
		if ( iRet == XUI_OK ) iRet = xuiCodeDocumentCreate(&pLoadDocument);
		if ( iRet == XUI_OK ) {
			fStartMs = __xuiPerfNowMs();
			iRet = xuiCodeDocumentLoadTextFile(pLoadDocument, sTempPath, XRT_CP_UTF8);
			fFileLoadMs = __xuiPerfNowMs() - fStartMs;
			iFileLineCount = xuiCodeDocumentGetLineCount(pLoadDocument);
			if ( iRet == XUI_OK && xuiCodeDocumentGetLength(pLoadDocument) != iLength ) iRet = XUI_ERROR_UNSUPPORTED;
		}
	}
	xrtFree(sText);
	if ( iRet != XUI_OK ) {
		printf("xui_code_large_perf_test failed: set text ret=%d error=%s\n", iRet, xuiCodeDocumentGetLastError(pDocument));
		if ( pLoadDocument != NULL ) xuiCodeDocumentDestroy(pLoadDocument);
		if ( sTempPath[0] != '\0' ) DeleteFileA(sTempPath);
		xuiCodeDocumentDestroy(pDocument);
		return 1;
	}
	if ( pLoadDocument != NULL ) {
		xuiCodeDocumentDestroy(pLoadDocument);
		pLoadDocument = NULL;
	}
	if ( sTempPath[0] != '\0' ) {
		DeleteFileA(sTempPath);
		sTempPath[0] = '\0';
	}
	iLineCount = xuiCodeDocumentGetLineCount(pDocument);
	if ( xuiCodeDocumentGetLength(pDocument) != iLength || iLineCount <= 0 ) {
		printf("xui_code_large_perf_test failed: invalid document length=%d lines=%d\n", xuiCodeDocumentGetLength(pDocument), iLineCount);
		xuiCodeDocumentDestroy(pDocument);
		return 1;
	}
	iSeed = 0x12345678u;
	fStartMs = __xuiPerfNowMs();
	for ( i = 0; i < iIterations; i++ ) {
		iSeed = iSeed * 1664525u + 1013904223u;
		iLine = (int)(iSeed % (uint32_t)iLineCount);
		iStart = 0;
		iEnd = 0;
		iRet = xuiCodeDocumentGetLineRange(pDocument, iLine, &iStart, &iEnd);
		if ( iRet != XUI_OK ) break;
		iChecksum += (uint64_t)(uint32_t)iStart + (uint64_t)(uint32_t)iEnd;
	}
	fLineSeekMs = __xuiPerfNowMs() - fStartMs;
	if ( iRet != XUI_OK ) {
		printf("xui_code_large_perf_test failed: line seek ret=%d\n", iRet);
		xuiCodeDocumentDestroy(pDocument);
		return 1;
	}
	iSeed = 0x87654321u;
	fStartMs = __xuiPerfNowMs();
	for ( i = 0; i < iIterations; i++ ) {
		iSeed = iSeed * 1664525u + 1013904223u;
		iOffset = (int)(((uint64_t)iSeed * (uint64_t)iLength) >> 32);
		iLine = 0;
		iRet = xuiCodeDocumentOffsetToLineColumn(pDocument, iOffset, &iLine, NULL);
		if ( iRet != XUI_OK ) break;
		iChecksum += (uint64_t)(uint32_t)iLine;
	}
	fOffsetSeekMs = __xuiPerfNowMs() - fStartMs;
	if ( iRet != XUI_OK ) {
		printf("xui_code_large_perf_test failed: offset seek ret=%d\n", iRet);
		xuiCodeDocumentDestroy(pDocument);
		return 1;
	}
	iSeed = 0x2468ace0u;
	fStartMs = __xuiPerfNowMs();
	for ( i = 0; i < iIterations; i++ ) {
		iSeed = iSeed * 1664525u + 1013904223u;
		iLine = (int)(iSeed % (uint32_t)iLineCount);
		iColumn = (int)((iSeed >> 8) % (uint32_t)iLineBytes);
		iOffset = 0;
		iRet = xuiCodeDocumentLineColumnToOffset(pDocument, iLine, iColumn, &iOffset);
		if ( iRet != XUI_OK ) break;
		iChecksum += (uint64_t)(uint32_t)iOffset;
	}
	fColumnSeekMs = __xuiPerfNowMs() - fStartMs;
	if ( iRet != XUI_OK ) {
		printf("xui_code_large_perf_test failed: column seek ret=%d\n", iRet);
		xuiCodeDocumentDestroy(pDocument);
		return 1;
	}
	printf("xui_code_large_perf_test summary mb=%d bytes=%d lines=%d iterations=%d set_ms=%.3f file_load_ms=%.3f file_lines=%d line_seek_ms=%.3f offset_seek_ms=%.3f column_seek_ms=%.3f checksum=%llu\n",
		iMegabytes,
		iLength,
		iLineCount,
		iIterations,
		fSetMs,
		fFileLoadMs,
		iFileLineCount,
		fLineSeekMs,
		fOffsetSeekMs,
		fColumnSeekMs,
		(unsigned long long)iChecksum);
	xuiCodeDocumentDestroy(pDocument);
	return 0;
}
