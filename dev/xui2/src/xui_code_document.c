#include "../xui.h"

#include <string.h>

typedef struct xui_code_line_t {
	int iStart;
	int iEnd;
	int iTextEnd;
} xui_code_line_t;

typedef struct xui_code_snapshot_t {
	char* sText;
	int iLength;
	uint32_t iVersion;
	uint32_t iChangeVersion;
	uint32_t iSavedVersion;
	int bDirty;
} xui_code_snapshot_t;

typedef struct xui_code_piece_t {
	int iBuffer;
	int iStart;
	int iLength;
} xui_code_piece_t;

#define XUI_CODE_PIECE_ORIGINAL 1
#define XUI_CODE_PIECE_ADD 2

struct xui_code_document_t {
	char* sText;
	int iLength;
	int iCapacity;
	char* sOriginal;
	int iOriginalLength;
	int iOriginalCapacity;
	char* sAdd;
	int iAddLength;
	int iAddCapacity;
	xui_code_piece_t* pPieces;
	int iPieceCount;
	int iPieceCapacity;
	xui_code_line_t* pLines;
	int iLineCount;
	int iLineCapacity;
	xui_code_snapshot_t* pUndo;
	int iUndoCount;
	int iUndoCapacity;
	xui_code_snapshot_t* pRedo;
	int iRedoCount;
	int iRedoCapacity;
	int iEditDepth;
	int bSnapshotPending;
	int bHasLastEditRange;
	int bChangePending;
	int iLastEditStart;
	int iLastEditEnd;
	uint32_t iVersion;
	uint32_t iChangeVersion;
	uint32_t iSavedVersion;
	int bDirty;
	xui_code_document_change_proc onChange;
	void* pChangeUser;
	char sError[160];
};

static int __xuiCodeDocumentRebuildLines(xui_code_document pDocument);

static void __xuiCodeDocumentSetError(xui_code_document pDocument, const char* sError)
{
	if ( pDocument == NULL ) return;
	if ( sError == NULL ) sError = "";
	strncpy(pDocument->sError, sError, sizeof(pDocument->sError) - 1u);
	pDocument->sError[sizeof(pDocument->sError) - 1u] = '\0';
}

static int __xuiCodeDocumentUtf8Next(const char* sText, int iLength, int iOffset)
{
	unsigned char c;
	int iStep;

	if ( sText == NULL ) return 0;
	if ( iOffset < 0 ) return 0;
	if ( iOffset >= iLength ) return iLength;
	c = (unsigned char)sText[iOffset];
	if ( c < 0x80u ) iStep = 1;
	else if ( (c & 0xE0u) == 0xC0u ) iStep = 2;
	else if ( (c & 0xF0u) == 0xE0u ) iStep = 3;
	else if ( (c & 0xF8u) == 0xF0u ) iStep = 4;
	else iStep = 1;
	if ( iOffset + iStep > iLength ) return iLength;
	return iOffset + iStep;
}

static int __xuiCodeDocumentClampOffset(xui_code_document pDocument, int iOffset)
{
	int iPos;
	int iNext;

	if ( pDocument == NULL ) return 0;
	if ( iOffset <= 0 ) return 0;
	if ( iOffset >= pDocument->iLength ) return pDocument->iLength;
	iPos = 0;
	while ( iPos < pDocument->iLength ) {
		iNext = __xuiCodeDocumentUtf8Next(pDocument->sText, pDocument->iLength, iPos);
		if ( iNext >= iOffset ) {
			return (iNext == iOffset) ? iOffset : iPos;
		}
		iPos = iNext;
	}
	return pDocument->iLength;
}

static int __xuiCodeDocumentReserveText(xui_code_document pDocument, int iCapacity)
{
	char* sNew;

	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pDocument->iCapacity ) return XUI_OK;
	if ( iCapacity < pDocument->iCapacity * 2 ) {
		iCapacity = pDocument->iCapacity * 2;
	}
	if ( iCapacity < 32 ) iCapacity = 32;
	sNew = (char*)xrtRealloc(pDocument->sText, (size_t)iCapacity);
	if ( sNew == NULL ) {
		__xuiCodeDocumentSetError(pDocument, "out of memory");
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDocument->sText = sNew;
	pDocument->iCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiCodeDocumentReserveBuffer(xui_code_document pDocument, char** ppBuffer, int* pCapacity, int iCapacity)
{
	char* sNew;

	if ( pDocument == NULL || ppBuffer == NULL || pCapacity == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= *pCapacity ) return XUI_OK;
	if ( iCapacity < (*pCapacity) * 2 ) iCapacity = (*pCapacity) * 2;
	if ( iCapacity < 32 ) iCapacity = 32;
	sNew = (char*)xrtRealloc(*ppBuffer, (size_t)iCapacity);
	if ( sNew == NULL ) {
		__xuiCodeDocumentSetError(pDocument, "out of memory");
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	*ppBuffer = sNew;
	*pCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiCodeDocumentAppendPieceTo(xui_code_document pDocument, xui_code_piece_t** ppPieces, int* pCount, int* pCapacity, int iBuffer, int iStart, int iLength)
{
	xui_code_piece_t* pNew;
	xui_code_piece_t* pPrev;
	int iCapacity;

	if ( pDocument == NULL || ppPieces == NULL || pCount == NULL || pCapacity == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iLength <= 0 ) return XUI_OK;
	if ( *pCount > 0 ) {
		pPrev = &(*ppPieces)[*pCount - 1];
		if ( pPrev->iBuffer == iBuffer && pPrev->iStart + pPrev->iLength == iStart ) {
			pPrev->iLength += iLength;
			return XUI_OK;
		}
	}
	if ( *pCount >= *pCapacity ) {
		iCapacity = (*pCapacity > 0) ? ((*pCapacity) * 2) : 8;
		pNew = (xui_code_piece_t*)xrtRealloc(*ppPieces, sizeof(*pNew) * (size_t)iCapacity);
		if ( pNew == NULL ) {
			__xuiCodeDocumentSetError(pDocument, "out of memory");
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		*ppPieces = pNew;
		*pCapacity = iCapacity;
	}
	(*ppPieces)[*pCount].iBuffer = iBuffer;
	(*ppPieces)[*pCount].iStart = iStart;
	(*ppPieces)[*pCount].iLength = iLength;
	(*pCount)++;
	return XUI_OK;
}

static const char* __xuiCodeDocumentPieceBuffer(xui_code_document pDocument, int iBuffer)
{
	if ( pDocument == NULL ) return "";
	if ( iBuffer == XUI_CODE_PIECE_ORIGINAL ) return (pDocument->sOriginal != NULL) ? pDocument->sOriginal : "";
	if ( iBuffer == XUI_CODE_PIECE_ADD ) return (pDocument->sAdd != NULL) ? pDocument->sAdd : "";
	return "";
}

static int __xuiCodeDocumentRebuildTextFromPieces(xui_code_document pDocument)
{
	const char* sBuffer;
	int i;
	int iLength;
	int iRet;
	int iOffset;

	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iLength = 0;
	for ( i = 0; i < pDocument->iPieceCount; i++ ) {
		iLength += pDocument->pPieces[i].iLength;
	}
	iRet = __xuiCodeDocumentReserveText(pDocument, iLength + 1);
	if ( iRet != XUI_OK ) return iRet;
	iOffset = 0;
	for ( i = 0; i < pDocument->iPieceCount; i++ ) {
		sBuffer = __xuiCodeDocumentPieceBuffer(pDocument, pDocument->pPieces[i].iBuffer);
		memcpy(pDocument->sText + iOffset, sBuffer + pDocument->pPieces[i].iStart, (size_t)pDocument->pPieces[i].iLength);
		iOffset += pDocument->pPieces[i].iLength;
	}
	pDocument->sText[iOffset] = '\0';
	pDocument->iLength = iOffset;
	return __xuiCodeDocumentRebuildLines(pDocument);
}

static int __xuiCodeDocumentSetPieceText(xui_code_document pDocument, const char* sText, int iLength)
{
	int iRet;

	if ( pDocument == NULL || sText == NULL || iLength < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCodeDocumentReserveBuffer(pDocument, &pDocument->sOriginal, &pDocument->iOriginalCapacity, iLength + 1);
	if ( iRet != XUI_OK ) return iRet;
	memcpy(pDocument->sOriginal, sText, (size_t)iLength);
	pDocument->sOriginal[iLength] = '\0';
	pDocument->iOriginalLength = iLength;
	pDocument->iAddLength = 0;
	if ( pDocument->sAdd != NULL ) pDocument->sAdd[0] = '\0';
	pDocument->iPieceCount = 0;
	if ( iLength > 0 ) {
		iRet = __xuiCodeDocumentAppendPieceTo(pDocument, &pDocument->pPieces, &pDocument->iPieceCount, &pDocument->iPieceCapacity, XUI_CODE_PIECE_ORIGINAL, 0, iLength);
		if ( iRet != XUI_OK ) return iRet;
	}
	return __xuiCodeDocumentRebuildTextFromPieces(pDocument);
}

static int __xuiCodeDocumentAppendAddBuffer(xui_code_document pDocument, const char* sText, int iLength, int* pStart)
{
	int iStart;
	int iRet;

	if ( pDocument == NULL || sText == NULL || iLength < 0 || pStart == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iStart = pDocument->iAddLength;
	iRet = __xuiCodeDocumentReserveBuffer(pDocument, &pDocument->sAdd, &pDocument->iAddCapacity, pDocument->iAddLength + iLength + 1);
	if ( iRet != XUI_OK ) return iRet;
	if ( iLength > 0 ) memcpy(pDocument->sAdd + pDocument->iAddLength, sText, (size_t)iLength);
	pDocument->iAddLength += iLength;
	pDocument->sAdd[pDocument->iAddLength] = '\0';
	*pStart = iStart;
	return XUI_OK;
}

static int __xuiCodeDocumentReplacePieces(xui_code_document pDocument, int iStart, int iEnd, int iInsertBuffer, int iInsertStart, int iInsertLength)
{
	xui_code_piece_t* pNew;
	xui_code_piece_t tPiece;
	int iNewCount;
	int iNewCapacity;
	int iPieceStart;
	int iPieceEnd;
	int iBeforeLength;
	int iAfterLength;
	int bInserted;
	int i;
	int iRet;

	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pNew = NULL;
	iNewCount = 0;
	iNewCapacity = 0;
	iPieceStart = 0;
	bInserted = (iInsertLength <= 0) ? 1 : 0;
	for ( i = 0; i < pDocument->iPieceCount; i++ ) {
		tPiece = pDocument->pPieces[i];
		iPieceEnd = iPieceStart + tPiece.iLength;
		if ( iPieceEnd <= iStart || iPieceStart >= iEnd ) {
			if ( !bInserted && iPieceStart >= iStart ) {
				iRet = __xuiCodeDocumentAppendPieceTo(pDocument, &pNew, &iNewCount, &iNewCapacity, iInsertBuffer, iInsertStart, iInsertLength);
				if ( iRet != XUI_OK ) goto error;
				bInserted = 1;
			}
			iRet = __xuiCodeDocumentAppendPieceTo(pDocument, &pNew, &iNewCount, &iNewCapacity, tPiece.iBuffer, tPiece.iStart, tPiece.iLength);
			if ( iRet != XUI_OK ) goto error;
		} else {
			iBeforeLength = iStart - iPieceStart;
			if ( iBeforeLength > 0 ) {
				iRet = __xuiCodeDocumentAppendPieceTo(pDocument, &pNew, &iNewCount, &iNewCapacity, tPiece.iBuffer, tPiece.iStart, iBeforeLength);
				if ( iRet != XUI_OK ) goto error;
			}
			if ( !bInserted ) {
				iRet = __xuiCodeDocumentAppendPieceTo(pDocument, &pNew, &iNewCount, &iNewCapacity, iInsertBuffer, iInsertStart, iInsertLength);
				if ( iRet != XUI_OK ) goto error;
				bInserted = 1;
			}
			iAfterLength = iPieceEnd - iEnd;
			if ( iAfterLength > 0 ) {
				iRet = __xuiCodeDocumentAppendPieceTo(pDocument, &pNew, &iNewCount, &iNewCapacity, tPiece.iBuffer, tPiece.iStart + (iEnd - iPieceStart), iAfterLength);
				if ( iRet != XUI_OK ) goto error;
			}
		}
		iPieceStart = iPieceEnd;
	}
	if ( !bInserted ) {
		iRet = __xuiCodeDocumentAppendPieceTo(pDocument, &pNew, &iNewCount, &iNewCapacity, iInsertBuffer, iInsertStart, iInsertLength);
		if ( iRet != XUI_OK ) goto error;
	}
	xrtFree(pDocument->pPieces);
	pDocument->pPieces = pNew;
	pDocument->iPieceCount = iNewCount;
	pDocument->iPieceCapacity = iNewCapacity;
	return __xuiCodeDocumentRebuildTextFromPieces(pDocument);

error:
	xrtFree(pNew);
	return iRet;
}

static int __xuiCodeDocumentReserveLines(xui_code_document pDocument, int iCapacity)
{
	xui_code_line_t* pNew;

	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pDocument->iLineCapacity ) return XUI_OK;
	if ( iCapacity < pDocument->iLineCapacity * 2 ) {
		iCapacity = pDocument->iLineCapacity * 2;
	}
	if ( iCapacity < 16 ) iCapacity = 16;
	pNew = (xui_code_line_t*)xrtRealloc(pDocument->pLines, sizeof(*pNew) * (size_t)iCapacity);
	if ( pNew == NULL ) {
		__xuiCodeDocumentSetError(pDocument, "out of memory");
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDocument->pLines = pNew;
	pDocument->iLineCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiCodeDocumentAppendLine(xui_code_document pDocument, int iStart, int iEnd, int iTextEnd)
{
	int iRet;

	iRet = __xuiCodeDocumentReserveLines(pDocument, pDocument->iLineCount + 1);
	if ( iRet != XUI_OK ) return iRet;
	pDocument->pLines[pDocument->iLineCount].iStart = iStart;
	pDocument->pLines[pDocument->iLineCount].iEnd = iEnd;
	pDocument->pLines[pDocument->iLineCount].iTextEnd = iTextEnd;
	pDocument->iLineCount++;
	return XUI_OK;
}

static int __xuiCodeDocumentRebuildLines(xui_code_document pDocument)
{
	int i;
	int iStart;
	int iRet;

	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pDocument->iLineCount = 0;
	iStart = 0;
	for ( i = 0; i < pDocument->iLength; i++ ) {
		if ( pDocument->sText[i] == '\n' ) {
			iRet = __xuiCodeDocumentAppendLine(pDocument, iStart, i + 1, i);
			if ( iRet != XUI_OK ) return iRet;
			iStart = i + 1;
		}
	}
	iRet = __xuiCodeDocumentAppendLine(pDocument, iStart, pDocument->iLength, pDocument->iLength);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_OK;
}

static char* __xuiCodeDocumentNormalizeText(const char* sText, int* pLength)
{
	char* sOut;
	int iLen;
	int i;
	int j;

	if ( sText == NULL ) sText = "";
	iLen = (int)strlen(sText);
	sOut = (char*)xrtMalloc((size_t)iLen + 1u);
	if ( sOut == NULL ) return NULL;
	j = 0;
	for ( i = 0; i < iLen; i++ ) {
		if ( sText[i] == '\r' ) {
			if ( (i + 1 < iLen) && (sText[i + 1] == '\n') ) i++;
			sOut[j++] = '\n';
		} else {
			sOut[j++] = sText[i];
		}
	}
	sOut[j] = '\0';
	if ( pLength != NULL ) *pLength = j;
	return sOut;
}

static int __xuiCodeDocumentValidateUtf8(xui_code_document pDocument, const char* sText, int iLength)
{
	if ( sText == NULL ) return XUI_OK;
	if ( iLength < 0 ) iLength = (int)strlen(sText);
	if ( !xrtIsUTF8((str)sText, (size_t)iLength) ) {
		__xuiCodeDocumentSetError(pDocument, "invalid UTF-8 input");
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return XUI_OK;
}

static void __xuiCodeDocumentSnapshotClear(xui_code_snapshot_t* pSnapshot)
{
	if ( pSnapshot == NULL ) return;
	xrtFree(pSnapshot->sText);
	memset(pSnapshot, 0, sizeof(*pSnapshot));
}

static void __xuiCodeDocumentSnapshotArrayClear(xui_code_snapshot_t* pSnapshots, int iCount)
{
	int i;

	if ( pSnapshots == NULL ) return;
	for ( i = 0; i < iCount; i++ ) {
		__xuiCodeDocumentSnapshotClear(&pSnapshots[i]);
	}
}

static int __xuiCodeDocumentSnapshotPush(xui_code_document pDocument, xui_code_snapshot_t** ppArray, int* pCount, int* pCapacity)
{
	xui_code_snapshot_t* pNew;
	xui_code_snapshot_t* pSnapshot;
	int iCapacity;

	if ( (pDocument == NULL) || (ppArray == NULL) || (pCount == NULL) || (pCapacity == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( *pCount >= *pCapacity ) {
		iCapacity = (*pCapacity > 0) ? (*pCapacity * 2) : 8;
		pNew = (xui_code_snapshot_t*)xrtRealloc(*ppArray, sizeof(*pNew) * (size_t)iCapacity);
		if ( pNew == NULL ) {
			__xuiCodeDocumentSetError(pDocument, "out of memory");
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		*ppArray = pNew;
		*pCapacity = iCapacity;
	}
	pSnapshot = &(*ppArray)[*pCount];
	memset(pSnapshot, 0, sizeof(*pSnapshot));
	pSnapshot->sText = (char*)xrtMalloc((size_t)pDocument->iLength + 1u);
	if ( pSnapshot->sText == NULL ) {
		__xuiCodeDocumentSetError(pDocument, "out of memory");
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	memcpy(pSnapshot->sText, pDocument->sText, (size_t)pDocument->iLength + 1u);
	pSnapshot->iLength = pDocument->iLength;
	pSnapshot->iVersion = pDocument->iVersion;
	pSnapshot->iChangeVersion = pDocument->iChangeVersion;
	pSnapshot->iSavedVersion = pDocument->iSavedVersion;
	pSnapshot->bDirty = pDocument->bDirty;
	(*pCount)++;
	return XUI_OK;
}

static int __xuiCodeDocumentPrepareEdit(xui_code_document pDocument)
{
	int iRet;

	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDocument->iEditDepth > 0 ) {
		if ( pDocument->bSnapshotPending ) return XUI_OK;
		pDocument->bSnapshotPending = 1;
	}
	iRet = __xuiCodeDocumentSnapshotPush(pDocument, &pDocument->pUndo, &pDocument->iUndoCount, &pDocument->iUndoCapacity);
	if ( iRet != XUI_OK ) return iRet;
	__xuiCodeDocumentSnapshotArrayClear(pDocument->pRedo, pDocument->iRedoCount);
	pDocument->iRedoCount = 0;
	return XUI_OK;
}

static int __xuiCodeDocumentRestoreSnapshot(xui_code_document pDocument, const xui_code_snapshot_t* pSnapshot)
{
	int iRet;

	if ( (pDocument == NULL) || (pSnapshot == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCodeDocumentSetPieceText(pDocument, pSnapshot->sText, pSnapshot->iLength);
	if ( iRet != XUI_OK ) return iRet;
	pDocument->iVersion = pSnapshot->iVersion;
	pDocument->iChangeVersion = pSnapshot->iChangeVersion;
	pDocument->iSavedVersion = pSnapshot->iSavedVersion;
	pDocument->bDirty = pSnapshot->bDirty;
	return XUI_OK;
}

static void __xuiCodeDocumentSetLastEditRange(xui_code_document pDocument, int iStart, int iEnd)
{
	if ( pDocument == NULL ) return;
	if ( iStart < 0 ) iStart = 0;
	if ( iEnd < iStart ) iEnd = iStart;
	if ( iEnd > pDocument->iLength ) iEnd = pDocument->iLength;
	if ( !pDocument->bHasLastEditRange || pDocument->iEditDepth <= 0 ) {
		pDocument->iLastEditStart = iStart;
		pDocument->iLastEditEnd = iEnd;
		pDocument->bHasLastEditRange = 1;
		return;
	}
	if ( iStart < pDocument->iLastEditStart ) pDocument->iLastEditStart = iStart;
	if ( iEnd > pDocument->iLastEditEnd ) pDocument->iLastEditEnd = iEnd;
}

static void __xuiCodeDocumentNotifyChanged(xui_code_document pDocument)
{
	xui_code_range_t tRange;

	if ( pDocument == NULL ) return;
	pDocument->bChangePending = 1;
	if ( pDocument->iEditDepth > 0 ) return;
	if ( pDocument->onChange == NULL ) {
		pDocument->bChangePending = 0;
		return;
	}
	tRange.iStart = pDocument->bHasLastEditRange ? pDocument->iLastEditStart : 0;
	tRange.iEnd = pDocument->bHasLastEditRange ? pDocument->iLastEditEnd : pDocument->iLength;
	pDocument->bChangePending = 0;
	pDocument->onChange(pDocument, tRange, pDocument->iVersion, pDocument->pChangeUser);
}

XUI_API int xuiCodeDocumentCreate(xui_code_document* ppDocument)
{
	xui_code_document pDocument;
	int iRet;

	if ( ppDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppDocument = NULL;
	pDocument = (xui_code_document)xrtMalloc(sizeof(*pDocument));
	if ( pDocument == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pDocument, 0, sizeof(*pDocument));
	iRet = __xuiCodeDocumentReserveText(pDocument, 1);
	if ( iRet == XUI_OK ) {
		pDocument->sText[0] = '\0';
		iRet = __xuiCodeDocumentRebuildLines(pDocument);
	}
	if ( iRet != XUI_OK ) {
		xuiCodeDocumentDestroy(pDocument);
		return iRet;
	}
	*ppDocument = pDocument;
	return XUI_OK;
}

XUI_API void xuiCodeDocumentDestroy(xui_code_document pDocument)
{
	if ( pDocument == NULL ) return;
	xrtFree(pDocument->sText);
	xrtFree(pDocument->sOriginal);
	xrtFree(pDocument->sAdd);
	xrtFree(pDocument->pPieces);
	xrtFree(pDocument->pLines);
	__xuiCodeDocumentSnapshotArrayClear(pDocument->pUndo, pDocument->iUndoCount);
	__xuiCodeDocumentSnapshotArrayClear(pDocument->pRedo, pDocument->iRedoCount);
	xrtFree(pDocument->pUndo);
	xrtFree(pDocument->pRedo);
	xrtFree(pDocument);
}

XUI_API int xuiCodeDocumentSetText(xui_code_document pDocument, const char* sText)
{
	char* sNormalized;
	int iLength;
	int iRet;

	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCodeDocumentValidateUtf8(pDocument, sText, -1);
	if ( iRet != XUI_OK ) return iRet;
	sNormalized = __xuiCodeDocumentNormalizeText(sText, &iLength);
	if ( sNormalized == NULL ) {
		__xuiCodeDocumentSetError(pDocument, "out of memory");
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xuiCodeDocumentSetPieceText(pDocument, sNormalized, iLength);
	if ( iRet == XUI_OK ) {
		pDocument->iVersion++;
		pDocument->iChangeVersion++;
		pDocument->iSavedVersion = pDocument->iVersion;
		pDocument->bDirty = 0;
		pDocument->bSnapshotPending = 0;
		__xuiCodeDocumentSetLastEditRange(pDocument, 0, iLength);
		__xuiCodeDocumentSnapshotArrayClear(pDocument->pUndo, pDocument->iUndoCount);
		__xuiCodeDocumentSnapshotArrayClear(pDocument->pRedo, pDocument->iRedoCount);
		pDocument->iUndoCount = 0;
		pDocument->iRedoCount = 0;
		__xuiCodeDocumentNotifyChanged(pDocument);
	}
	xrtFree(sNormalized);
	return iRet;
}

XUI_API const char* xuiCodeDocumentGetText(xui_code_document pDocument)
{
	if ( pDocument == NULL ) return "";
	return (pDocument->sText != NULL) ? pDocument->sText : "";
}

XUI_API int xuiCodeDocumentLoadTextFile(xui_code_document pDocument, const char* sPath, int iCharset)
{
	char* sText;
	size_t iSize;
	int iRet;

	if ( (pDocument == NULL) || (sPath == NULL) || (sPath[0] == '\0') ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iSize = 0;
	sText = (char*)xrtFileReadAll((str)sPath, iCharset, &iSize);
	if ( sText == NULL ) {
		__xuiCodeDocumentSetError(pDocument, "file read failed");
		return XUI_ERROR_FILE_NOT_FOUND;
	}
	iRet = __xuiCodeDocumentValidateUtf8(pDocument, sText, (int)iSize);
	if ( iRet == XUI_OK ) iRet = xuiCodeDocumentSetText(pDocument, sText);
	xrtFree(sText);
	return iRet;
}

XUI_API int xuiCodeDocumentSaveTextFile(xui_code_document pDocument, const char* sPath, int iCharset)
{
	int iRet;

	if ( (pDocument == NULL) || (sPath == NULL) || (sPath[0] == '\0') ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xrtFileWriteAll((str)sPath, (str)xuiCodeDocumentGetText(pDocument), (size_t)pDocument->iLength, iCharset);
	if ( iRet == 0 && pDocument->iLength > 0 ) {
		__xuiCodeDocumentSetError(pDocument, "file write failed");
		return XUI_ERROR_RESOURCE_FAILED;
	}
	return XUI_OK;
}

XUI_API int xuiCodeDocumentGetLength(xui_code_document pDocument)
{
	if ( pDocument == NULL ) return 0;
	return pDocument->iLength;
}

XUI_API int xuiCodeDocumentGetLineCount(xui_code_document pDocument)
{
	if ( pDocument == NULL ) return 0;
	return pDocument->iLineCount;
}

XUI_API int xuiCodeDocumentGetLineRange(xui_code_document pDocument, int iLine, int* pStart, int* pEnd)
{
	if ( (pDocument == NULL) || (iLine < 0) || (iLine >= pDocument->iLineCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pStart != NULL ) *pStart = pDocument->pLines[iLine].iStart;
	if ( pEnd != NULL ) *pEnd = pDocument->pLines[iLine].iTextEnd;
	return XUI_OK;
}

XUI_API int xuiCodeDocumentOffsetToLineColumn(xui_code_document pDocument, int iOffset, int* pLine, int* pColumn)
{
	int iLow;
	int iHigh;
	int iMid;
	int iLine;

	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iOffset = __xuiCodeDocumentClampOffset(pDocument, iOffset);
	iLow = 0;
	iHigh = pDocument->iLineCount - 1;
	iLine = 0;
	while ( iLow <= iHigh ) {
		iMid = (iLow + iHigh) / 2;
		if ( iOffset < pDocument->pLines[iMid].iStart ) {
			iHigh = iMid - 1;
		} else if ( (iOffset >= pDocument->pLines[iMid].iEnd) && (iMid + 1 < pDocument->iLineCount) ) {
			iLow = iMid + 1;
		} else {
			iLine = iMid;
			break;
		}
	}
	if ( iLow > iHigh ) iLine = (iLow < pDocument->iLineCount) ? iLow : (pDocument->iLineCount - 1);
	if ( pLine != NULL ) *pLine = iLine;
	if ( pColumn != NULL ) *pColumn = iOffset - pDocument->pLines[iLine].iStart;
	return XUI_OK;
}

XUI_API int xuiCodeDocumentLineColumnToOffset(xui_code_document pDocument, int iLine, int iColumn, int* pOffset)
{
	int iOffset;

	if ( (pDocument == NULL) || (iLine < 0) || (iLine >= pDocument->iLineCount) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iColumn < 0 ) iColumn = 0;
	iOffset = pDocument->pLines[iLine].iStart + iColumn;
	if ( iOffset > pDocument->pLines[iLine].iTextEnd ) iOffset = pDocument->pLines[iLine].iTextEnd;
	iOffset = __xuiCodeDocumentClampOffset(pDocument, iOffset);
	if ( pOffset != NULL ) *pOffset = iOffset;
	return XUI_OK;
}

XUI_API int xuiCodeDocumentReplace(xui_code_document pDocument, int iStart, int iEnd, const char* sText)
{
	char* sNormalized;
	int iInsertLength;
	int iInsertStart;
	int iRet;

	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCodeDocumentValidateUtf8(pDocument, sText, -1);
	if ( iRet != XUI_OK ) return iRet;
	iStart = __xuiCodeDocumentClampOffset(pDocument, iStart);
	iEnd = __xuiCodeDocumentClampOffset(pDocument, iEnd);
	if ( iEnd < iStart ) {
		int iTemp = iStart;
		iStart = iEnd;
		iEnd = iTemp;
	}
	sNormalized = __xuiCodeDocumentNormalizeText(sText, &iInsertLength);
	if ( sNormalized == NULL ) {
		__xuiCodeDocumentSetError(pDocument, "out of memory");
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xuiCodeDocumentPrepareEdit(pDocument);
	if ( iRet == XUI_OK ) {
		iInsertStart = 0;
		iRet = __xuiCodeDocumentAppendAddBuffer(pDocument, sNormalized, iInsertLength, &iInsertStart);
		if ( iRet == XUI_OK ) {
			iRet = __xuiCodeDocumentReplacePieces(pDocument, iStart, iEnd, XUI_CODE_PIECE_ADD, iInsertStart, iInsertLength);
		}
		if ( iRet == XUI_OK ) {
			pDocument->iVersion++;
			pDocument->iChangeVersion++;
			pDocument->bDirty = (pDocument->iVersion != pDocument->iSavedVersion);
			__xuiCodeDocumentSetLastEditRange(pDocument, iStart, iStart + iInsertLength);
			__xuiCodeDocumentNotifyChanged(pDocument);
		}
	}
	xrtFree(sNormalized);
	return iRet;
}

XUI_API int xuiCodeDocumentInsert(xui_code_document pDocument, int iOffset, const char* sText)
{
	return xuiCodeDocumentReplace(pDocument, iOffset, iOffset, sText);
}

XUI_API int xuiCodeDocumentDelete(xui_code_document pDocument, int iStart, int iEnd)
{
	return xuiCodeDocumentReplace(pDocument, iStart, iEnd, "");
}

XUI_API int xuiCodeDocumentBeginEdit(xui_code_document pDocument)
{
	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDocument->iEditDepth == 0 ) pDocument->bHasLastEditRange = 0;
	pDocument->iEditDepth++;
	return XUI_OK;
}

XUI_API int xuiCodeDocumentEndEdit(xui_code_document pDocument)
{
	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDocument->iEditDepth <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	pDocument->iEditDepth--;
	if ( pDocument->iEditDepth == 0 ) {
		pDocument->bSnapshotPending = 0;
		if ( pDocument->bChangePending ) __xuiCodeDocumentNotifyChanged(pDocument);
	}
	return XUI_OK;
}

XUI_API int xuiCodeDocumentSetChangeCallback(xui_code_document pDocument, xui_code_document_change_proc onChange, void* pUser)
{
	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pDocument->onChange = onChange;
	pDocument->pChangeUser = pUser;
	return XUI_OK;
}

XUI_API int xuiCodeDocumentUndo(xui_code_document pDocument)
{
	xui_code_snapshot_t tSnapshot;
	int iRet;

	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDocument->iUndoCount <= 0 ) return XUI_ERROR_UNSUPPORTED;
	iRet = __xuiCodeDocumentSnapshotPush(pDocument, &pDocument->pRedo, &pDocument->iRedoCount, &pDocument->iRedoCapacity);
	if ( iRet != XUI_OK ) return iRet;
	tSnapshot = pDocument->pUndo[pDocument->iUndoCount - 1];
	memset(&pDocument->pUndo[pDocument->iUndoCount - 1], 0, sizeof(tSnapshot));
	pDocument->iUndoCount--;
	iRet = __xuiCodeDocumentRestoreSnapshot(pDocument, &tSnapshot);
	if ( iRet == XUI_OK ) {
		__xuiCodeDocumentSetLastEditRange(pDocument, 0, pDocument->iLength);
		__xuiCodeDocumentNotifyChanged(pDocument);
	}
	__xuiCodeDocumentSnapshotClear(&tSnapshot);
	return iRet;
}

XUI_API int xuiCodeDocumentRedo(xui_code_document pDocument)
{
	xui_code_snapshot_t tSnapshot;
	int iRet;

	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDocument->iRedoCount <= 0 ) return XUI_ERROR_UNSUPPORTED;
	iRet = __xuiCodeDocumentSnapshotPush(pDocument, &pDocument->pUndo, &pDocument->iUndoCount, &pDocument->iUndoCapacity);
	if ( iRet != XUI_OK ) return iRet;
	tSnapshot = pDocument->pRedo[pDocument->iRedoCount - 1];
	memset(&pDocument->pRedo[pDocument->iRedoCount - 1], 0, sizeof(tSnapshot));
	pDocument->iRedoCount--;
	iRet = __xuiCodeDocumentRestoreSnapshot(pDocument, &tSnapshot);
	if ( iRet == XUI_OK ) {
		__xuiCodeDocumentSetLastEditRange(pDocument, 0, pDocument->iLength);
		__xuiCodeDocumentNotifyChanged(pDocument);
	}
	__xuiCodeDocumentSnapshotClear(&tSnapshot);
	return iRet;
}

XUI_API int xuiCodeDocumentCanUndo(xui_code_document pDocument)
{
	return (pDocument != NULL) && (pDocument->iUndoCount > 0);
}

XUI_API int xuiCodeDocumentCanRedo(xui_code_document pDocument)
{
	return (pDocument != NULL) && (pDocument->iRedoCount > 0);
}

XUI_API uint32_t xuiCodeDocumentGetVersion(xui_code_document pDocument)
{
	return (pDocument != NULL) ? pDocument->iVersion : 0u;
}

XUI_API uint32_t xuiCodeDocumentGetChangeVersion(xui_code_document pDocument)
{
	return (pDocument != NULL) ? pDocument->iChangeVersion : 0u;
}

XUI_API int xuiCodeDocumentGetLastEditRange(xui_code_document pDocument, xui_code_range_t* pRange)
{
	if ( (pDocument == NULL) || (pRange == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !pDocument->bHasLastEditRange ) return XUI_ERROR_UNSUPPORTED;
	pRange->iStart = pDocument->iLastEditStart;
	pRange->iEnd = pDocument->iLastEditEnd;
	return XUI_OK;
}

XUI_API int xuiCodeDocumentGetDirty(xui_code_document pDocument)
{
	return (pDocument != NULL) ? pDocument->bDirty : 0;
}

XUI_API int xuiCodeDocumentSetDirty(xui_code_document pDocument, int bDirty)
{
	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pDocument->bDirty = bDirty ? 1 : 0;
	if ( !pDocument->bDirty ) pDocument->iSavedVersion = pDocument->iVersion;
	return XUI_OK;
}

XUI_API const char* xuiCodeDocumentGetLastError(xui_code_document pDocument)
{
	if ( pDocument == NULL ) return "invalid document";
	return pDocument->sError;
}
