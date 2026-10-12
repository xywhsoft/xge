#include "ui_design_session.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

void uiDesignHistoryEntryFree(ui_design_history_entry_t* pEntry)
{
	if ( pEntry == NULL ) return;
	free(pEntry->sBefore);
	free(pEntry->sAfter);
	memset(pEntry, 0, sizeof(*pEntry));
}

void uiDesignHistoryClear(ui_design_history_entry_t* pEntries, int* pCount)
{
	int i;
	if ( pEntries == NULL || pCount == NULL ) return;
	for ( i = 0; i < *pCount; ++i ) uiDesignHistoryEntryFree(&pEntries[i]);
	*pCount = 0;
}

static void __uiDesignHistoryDropFirst(ui_design_history_entry_t* pEntries, int* pCount)
{
	if ( *pCount <= 0 ) return;
	uiDesignHistoryEntryFree(pEntries);
	--*pCount;
	memmove(pEntries, pEntries + 1, (size_t)*pCount * sizeof(*pEntries));
	memset(&pEntries[*pCount], 0, sizeof(*pEntries));
}

void uiDesignHistoryPush(ui_design_history_entry_t* pEntries, int* pCount, ui_design_history_entry_t* pEntry)
{
	if ( pEntries == NULL || pCount == NULL || pEntry == NULL ) return;
	if ( *pCount >= UI_DESIGN_HISTORY_LIMIT ) __uiDesignHistoryDropFirst(pEntries, pCount);
	if ( pEntry->iBytes == 0 ) pEntry->iBytes = (pEntry->sBefore ? strlen(pEntry->sBefore) + 1u : 0) + (pEntry->sAfter ? strlen(pEntry->sAfter) + 1u : 0);
	pEntries[(*pCount)++] = *pEntry;
	memset(pEntry, 0, sizeof(*pEntry));
}

size_t uiDesignSessionHistoryBytes(const ui_design_session_t* pSession)
{
	size_t n = 0;
	int i, stack;
	if ( pSession == NULL ) return 0;
	for ( stack = 0; stack < 2; ++stack ) {
		int count = stack ? pSession->iRedoCount : pSession->iUndoCount;
		const ui_design_history_entry_t* entries = stack ? pSession->arrRedo : pSession->arrUndo;
		for ( i = 0; i < count; ++i ) {
			n += entries[i].iBytes;
		}
	}
	return n;
}

void uiDesignSessionTrimHistory(ui_design_session_t* pSession, size_t iBudget)
{
	if ( pSession == NULL ) return;
	while ( uiDesignSessionHistoryBytes(pSession) > iBudget ) {
		/* Discard farthest redo entries first, preserving the next operation. */
		if ( pSession->iRedoCount > 0 ) __uiDesignHistoryDropFirst(pSession->arrRedo, &pSession->iRedoCount);
		else if ( pSession->iUndoCount > 0 ) __uiDesignHistoryDropFirst(pSession->arrUndo, &pSession->iUndoCount);
		else break;
	}
}

ui_design_session_t* uiDesignSessionCreate(uint64_t iId)
{
	ui_design_session_t* p = (ui_design_session_t*)calloc(1, sizeof(*p));
	if ( p == NULL ) return NULL;
	p->iId = iId;
	p->iRevision = 1;
	p->iHistoryBudget = UI_DESIGN_HISTORY_BYTES;
	p->fZoom = 1.0f;
	p->bGridVisible = 1;
	p->bMarqueeSelectContain = 1;
	uiDesignModelInit(&p->tModel);
	if ( uiDesignSessionSetClean(p) != XUI_OK ) { uiDesignSessionDestroy(p); return NULL; }
	return p;
}

void uiDesignSessionDestroy(ui_design_session_t* p)
{
	if ( p == NULL ) return;
	uiDesignModelDestroy(&p->tModel);
	uiDesignHistoryClear(p->arrUndo, &p->iUndoCount);
	uiDesignHistoryClear(p->arrRedo, &p->iRedoCount);
	free(p->sHistoryTransactionBefore);
	free(p->sCleanSnapshot);
	free(p);
}

int uiDesignSessionSetClean(ui_design_session_t* p)
{
	char* sContent = NULL;
	int ret;
	if ( p == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	ret = uiDesignDocumentSaveContent(&p->tModel, &sContent);
	if ( ret != XUI_OK ) return ret;
	free(p->sCleanSnapshot);
	p->sCleanSnapshot = sContent;
	p->bDocumentDirty = 0;
	return XUI_OK;
}

int uiDesignSessionUpdateDirty(ui_design_session_t* p)
{
	char* sContent = NULL;
	int ret;
	if ( p == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	++p->iRevision;
	ret = uiDesignDocumentSaveContent(&p->tModel, &sContent);
	if ( ret != XUI_OK ) { p->bDocumentDirty = 1; return ret; }
	p->bDocumentDirty = p->sCleanSnapshot == NULL || strcmp(p->sCleanSnapshot, sContent) != 0;
	free(sContent);
	return XUI_OK;
}

int uiDesignSessionAcceptResult(const ui_design_session_t* p, uint64_t iId, uint64_t iRevision)
{
	return p != NULL && p->iId == iId && p->iRevision == iRevision;
}

char* uiDesignFileRead(const char* sPath, size_t iLimit)
{
	size_t n = 0;
	unsigned char* data;
	char* text;
	if ( sPath == NULL ) return NULL;
	data = xrtFileReadAllLimit(sPath, iLimit, &n);
	if ( data == NULL ) return NULL;
	/* Embedded NUL is not a valid text document; do not silently load a prefix. */
	if ( memchr(data, 0, n) != NULL ) { xrtFree(data); return NULL; }
	text = (char*)malloc(n + 1u);
	if ( text != NULL ) { memcpy(text, data, n); text[n] = 0; }
	xrtFree(data);
	return text;
}

int uiDesignFileWrite(const char* sPath, const char* sText)
{
	if ( sPath == NULL || sPath[0] == 0 || sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xrtFileWriteAtomic(sPath, (xbytesview){(const unsigned char*)sText, strlen(sText)}) ? XUI_OK : XUI_ERROR_RESOURCE_FAILED;
}

int uiDesignFileCanonicalPath(const char* sPath, char* sOut, size_t iCapacity)
{
	if ( sPath == NULL || sPath[0] == 0 || sOut == NULL || iCapacity == 0 ) return XUI_ERROR_INVALID_ARGUMENT;
#if defined(_WIN32)
	{
		wchar_t input[UI_DESIGN_PATH_CAPACITY], absolute[UI_DESIGN_PATH_CAPACITY];
		DWORD n;
		if ( !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, sPath, -1, input, UI_DESIGN_PATH_CAPACITY) ) return XUI_ERROR_INVALID_ARGUMENT;
		n = GetFullPathNameW(input, UI_DESIGN_PATH_CAPACITY, absolute, NULL);
		if ( n == 0 || n >= UI_DESIGN_PATH_CAPACITY || !WideCharToMultiByte(CP_UTF8, 0, absolute, -1, sOut, (int)iCapacity, NULL, NULL) ) return XUI_ERROR_BUFFER_TOO_SMALL;
	}
#else
	if ( strlen(sPath) >= iCapacity ) return XUI_ERROR_BUFFER_TOO_SMALL;
	memcpy(sOut, sPath, strlen(sPath) + 1u);
#endif
	return XUI_OK;
}

int uiDesignFileSamePath(const char* sA, const char* sB)
{
	char a[UI_DESIGN_PATH_CAPACITY], b[UI_DESIGN_PATH_CAPACITY];
	if ( uiDesignFileCanonicalPath(sA, a, sizeof(a)) != XUI_OK || uiDesignFileCanonicalPath(sB, b, sizeof(b)) != XUI_OK ) return 0;
#if defined(_WIN32)
	{
		wchar_t wa[UI_DESIGN_PATH_CAPACITY], wb[UI_DESIGN_PATH_CAPACITY];
		if ( !MultiByteToWideChar(CP_UTF8, 0, a, -1, wa, UI_DESIGN_PATH_CAPACITY) || !MultiByteToWideChar(CP_UTF8, 0, b, -1, wb, UI_DESIGN_PATH_CAPACITY) ) return 0;
		return CompareStringOrdinal(wa, -1, wb, -1, TRUE) == CSTR_EQUAL;
	}
#else
	return strcmp(a, b) == 0;
#endif
}

ui_design_file_stamp_t uiDesignFileStamp(const char* sPath)
{
	ui_design_file_stamp_t stamp = {0, 0, 0};
#if defined(_WIN32)
	wchar_t path[UI_DESIGN_PATH_CAPACITY];
	WIN32_FILE_ATTRIBUTE_DATA data;
	if ( sPath != NULL && sPath[0] != 0 && MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, sPath, -1, path, UI_DESIGN_PATH_CAPACITY) &&
	     GetFileAttributesExW(path, GetFileExInfoStandard, &data) && !(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
		stamp.iSize = ((uint64_t)data.nFileSizeHigh << 32) | data.nFileSizeLow;
		stamp.iTime = ((uint64_t)data.ftLastWriteTime.dwHighDateTime << 32) | data.ftLastWriteTime.dwLowDateTime;
		stamp.bPresent = 1;
	}
#else
	(void)sPath;
#endif
	return stamp;
}

int uiDesignFileStampEqual(ui_design_file_stamp_t a, ui_design_file_stamp_t b)
{
	return a.bPresent == b.bPresent && a.iTime == b.iTime && a.iSize == b.iSize;
}
