#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

#define XGE_CLIPBOARD_TEST_SIZE ((2u * 1024u * 1024u) + 17u)

static int __testSetClipboardWideFromAscii(const char* sText, size_t iLength)
{
	HGLOBAL hMemory;
	WCHAR* sWide;
	size_t i;

	hMemory = GlobalAlloc(GMEM_MOVEABLE, (iLength + 1u) * sizeof(WCHAR));
	if ( hMemory == NULL ) return 0;
	sWide = (WCHAR*)GlobalLock(hMemory);
	if ( sWide == NULL ) {
		GlobalFree(hMemory);
		return 0;
	}
	for ( i = 0; i < iLength; i++ ) sWide[i] = (WCHAR)(unsigned char)sText[i];
	sWide[iLength] = L'\0';
	GlobalUnlock(hMemory);
	if ( !OpenClipboard(NULL) ) {
		GlobalFree(hMemory);
		return 0;
	}
	if ( !EmptyClipboard() || SetClipboardData(CF_UNICODETEXT, hMemory) == NULL ) {
		CloseClipboard();
		GlobalFree(hMemory);
		return 0;
	}
	CloseClipboard();
	return 1;
}

static int __testSystemClipboardMatchesAscii(const char* sText, size_t iLength)
{
	HANDLE hData;
	const WCHAR* sWide;
	size_t i;
	int bMatch;

	if ( !OpenClipboard(NULL) ) return 0;
	hData = GetClipboardData(CF_UNICODETEXT);
	if ( hData == NULL ) {
		CloseClipboard();
		return 0;
	}
	sWide = (const WCHAR*)GlobalLock(hData);
	if ( sWide == NULL ) {
		CloseClipboard();
		return 0;
	}
	bMatch = 1;
	for ( i = 0; i < iLength; i++ ) {
		if ( sWide[i] != (WCHAR)(unsigned char)sText[i] ) {
			bMatch = 0;
			break;
		}
	}
	if ( bMatch && sWide[iLength] != L'\0' ) bMatch = 0;
	GlobalUnlock(hData);
	CloseClipboard();
	return bMatch;
}

int main(void)
{
	xge_desc_t tDesc;
	const char* sCurrent;
	const char* sRead;
	char* sOriginal;
	char* sLong;
	size_t iOriginalSize;
	size_t i;
	int iRet;
	int iFailed;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iRunMode = XGE_RUN_MANUAL;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "xgeInit failed: %d\n", iRet);
		return 1;
	}
	sCurrent = xgeClipboardGetText();
	iOriginalSize = strlen(sCurrent);
	sOriginal = (char*)malloc(iOriginalSize + 1u);
	sLong = (char*)malloc(XGE_CLIPBOARD_TEST_SIZE + 1u);
	if ( sOriginal == NULL || sLong == NULL ) {
		free(sOriginal);
		free(sLong);
		xgeUnit();
		return 1;
	}
	memcpy(sOriginal, sCurrent, iOriginalSize + 1u);
	for ( i = 0; i < XGE_CLIPBOARD_TEST_SIZE; i++ ) sLong[i] = (char)('A' + (i % 26u));
	sLong[XGE_CLIPBOARD_TEST_SIZE] = '\0';
	iFailed = 0;
	if ( !__testSetClipboardWideFromAscii(sLong, XGE_CLIPBOARD_TEST_SIZE) ) {
		fprintf(stderr, "failed to prepare native 2 MiB clipboard payload\n");
		iFailed = 1;
	} else {
		sRead = xgeClipboardGetText();
		if ( strlen(sRead) != XGE_CLIPBOARD_TEST_SIZE || memcmp(sRead, sLong, XGE_CLIPBOARD_TEST_SIZE) != 0 ) {
			fprintf(stderr, "XGE truncated native 2 MiB clipboard payload\n");
			iFailed = 1;
		}
	}
	if ( !iFailed ) {
		xgeClipboardSetText(sLong);
		if ( !__testSystemClipboardMatchesAscii(sLong, XGE_CLIPBOARD_TEST_SIZE) ) {
			fprintf(stderr, "XGE truncated 2 MiB clipboard output\n");
			iFailed = 1;
		}
	}
	xgeClipboardSetText(sOriginal);
	free(sOriginal);
	free(sLong);
	xgeUnit();
	if ( iFailed ) return 1;
	printf("xge_clipboard_win32_test passed\n");
	return 0;
}

#else
int main(void)
{
	printf("xge_clipboard_win32_test skipped\n");
	return 0;
}
#endif
