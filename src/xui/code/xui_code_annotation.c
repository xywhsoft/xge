#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui.h"
#include "xui_code_diagnostics_internal.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

typedef struct xui_code_diagnostic_index_t {
	int iStart;
	int iEnd;
	int iIndex;
	int iPrefixMaxEnd;
} xui_code_diagnostic_index_t;

struct xui_code_diagnostic_text_t {
	size_t iRefs;
	char sBytes[];
};

struct xui_code_annotation_store_t {
	xui_widget pOwner;
	xui_code_document pDocument;
	const xui_code_annotation_owner_ops_t* pOwnerOps;
	uint64_t iOwnerThread;
	xui_code_marker_t* pMarkers;
	int iMarkerCount;
	int iMarkerCapacity;
	xui_code_indicator_t* pIndicators;
	int iIndicatorCount;
	int iIndicatorCapacity;
	xui_code_diagnostic_t* pDiagnostics;
	xui_code_diagnostic_text_t* pDiagnosticText;
	int iDiagnosticCount;
	int iDiagnosticCapacity;
	xui_code_diagnostic_index_t* pDiagnosticIndex;
	int iDiagnosticIndexCapacity;
	uint32_t iDiagnosticVersion;
};

static int __xuiCodeAnnotationStatus(xui_code_annotation_store store, bool write)
{
	if ( store == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( store->pOwner == NULL ) return XUI_OK;
	if ( store->iOwnerThread != xrtThreadCurrentId() ) return XUI_ERROR_BUSY;
	return store->pOwnerOps->check(store->pOwner, write);
}

int xuiInternalCodeAnnotationClaimOwner(xui_code_annotation_store store, xui_widget editor,
	xui_code_document document, const xui_code_annotation_owner_ops_t* ops)
{
	if ( store == NULL || editor == NULL || document == NULL || ops == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( store->pOwner != NULL ) return XUI_ERROR_INVALID_STATE;
	store->pDocument = document;
	store->pOwnerOps = ops;
	store->iOwnerThread = xrtThreadCurrentId();
	store->pOwner = editor;
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationEditorLifetimeWatch(xui_code_annotation_store store,
	xui_lifetime_link_t* link, xui_lifetime_invalidate_proc invalidate, void* user)
{
	int status = __xuiCodeAnnotationStatus(store, false);
	if ( status != XUI_OK ) return status;
	if ( store->pOwner == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return store->pOwnerOps->watch(store->pOwner, link, invalidate, user);
}

int xuiInternalCodeAnnotationCheckDocument(xui_code_annotation_store store, xui_code_document document)
{
	int status = __xuiCodeAnnotationStatus(store, true);
	if ( status != XUI_OK ) return status;
	return store->pDocument != NULL && store->pDocument != document ? XUI_ERROR_INVALID_ARGUMENT : XUI_OK;
}

void xuiInternalCodeAnnotationNotify(xui_code_annotation_store store, bool diagnostics)
{
	if ( store->pOwner != NULL ) store->pOwnerOps->changed(store->pOwner, diagnostics);
}

static char* __xuiCodeAnnotationStrDup(const char* sText)
{
	return sText != NULL ? xrtStrDup(sText) : NULL;
}

static void __xuiCodeAnnotationFreeMarker(xui_code_marker_t* pMarker)
{
	if ( pMarker == NULL ) return;
	xrtFree((void*)pMarker->sTooltip);
	memset(pMarker, 0, sizeof(*pMarker));
}

int xuiInternalCodeAnnotationPinDiagnosticText(xui_code_annotation_store store,
	xui_code_diagnostic_text_t** text)
{
	*text = store->pDiagnosticText;
	if ( *text == NULL ) return XUI_OK;
	if ( (*text)->iRefs == SIZE_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
	(*text)->iRefs++;
	return XUI_OK;
}

void xuiInternalCodeAnnotationUnpinDiagnosticText(xui_code_diagnostic_text_t* text)
{
	if ( text != NULL && --text->iRefs == 0 ) xrtFree(text);
}

static void __xuiCodeAnnotationClearDiagnosticText(xui_code_annotation_store store)
{
	xuiInternalCodeAnnotationUnpinDiagnosticText(store->pDiagnosticText);
	store->pDiagnosticText = NULL;
}

static int __xuiCodeAnnotationCopyDiagnostics(xui_code_annotation_store store,
	const xui_code_diagnostic_t* diagnostics, int count)
{
	size_t bytes = 0;
	for ( int i = 0; i < count; i++ ) {
		const char* fields[] = { diagnostics[i].sCode, diagnostics[i].sMessage, diagnostics[i].sSource };
		for ( size_t j = 0; j < sizeof(fields) / sizeof(fields[0]); j++ ) {
			if ( fields[j] == NULL ) continue;
			size_t length = strlen(fields[j]);
			if ( length >= SIZE_MAX - sizeof(xui_code_diagnostic_text_t) - bytes ) {
				return XUI_ERROR_LIMIT_EXCEEDED;
			}
			bytes += length + 1;
		}
	}
	char* cursor = NULL;
	if ( bytes != 0 ) {
		store->pDiagnosticText = xrtMalloc(sizeof(*store->pDiagnosticText) + bytes);
		if ( store->pDiagnosticText == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		store->pDiagnosticText->iRefs = 1;
		cursor = store->pDiagnosticText->sBytes;
	}
	for ( int i = 0; i < count; i++ ) {
		xui_code_diagnostic_t* output = &store->pDiagnostics[i];
		*output = diagnostics[i];
		output->iSize = sizeof(*output);
		const char** fields[] = { &output->sCode, &output->sMessage, &output->sSource };
		for ( size_t j = 0; j < sizeof(fields) / sizeof(fields[0]); j++ ) {
			if ( *fields[j] == NULL ) continue;
			size_t size = strlen(*fields[j]) + 1;
			memcpy(cursor, *fields[j], size);
			*fields[j] = cursor;
			cursor += size;
		}
	}
	store->iDiagnosticCount = count;
	return XUI_OK;
}

static int __xuiCodeAnnotationNextCapacity(int iCurrent, int iRequired,
	size_t iItemSize, int* pCapacity)
{
	int iCapacity;

	if ( iCurrent < 0 || iRequired < 0 || iItemSize == 0 || pCapacity == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iRequired <= iCurrent ) {
		*pCapacity = iCurrent;
		return XUI_OK;
	}
	iCapacity = (iCurrent > 0) ? iCurrent : 16;
	while ( iCapacity < iRequired ) {
		if ( iCapacity > INT_MAX / 2 ) {
			iCapacity = iRequired;
			break;
		}
		iCapacity *= 2;
	}
	if ( (size_t)iCapacity > SIZE_MAX / iItemSize ) return XUI_ERROR_LIMIT_EXCEEDED;
	*pCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiCodeAnnotationReserveMarkers(xui_code_annotation_store pStore, int iCapacity)
{
	xui_code_marker_t* pNew;
	int iNewCapacity;
	int iRet;

	if ( pStore == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pStore->iMarkerCapacity ) return XUI_OK;
	iRet = __xuiCodeAnnotationNextCapacity(pStore->iMarkerCapacity, iCapacity,
		sizeof(*pNew), &iNewCapacity);
	if ( iRet != XUI_OK ) return iRet;
	pNew = (xui_code_marker_t*)xrtRealloc(pStore->pMarkers,
		sizeof(*pNew) * (size_t)iNewCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pStore->pMarkers = pNew;
	pStore->iMarkerCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiCodeAnnotationReserveIndicators(xui_code_annotation_store pStore, int iCapacity)
{
	xui_code_indicator_t* pNew;
	int iNewCapacity;
	int iRet;

	if ( pStore == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pStore->iIndicatorCapacity ) return XUI_OK;
	iRet = __xuiCodeAnnotationNextCapacity(pStore->iIndicatorCapacity, iCapacity,
		sizeof(*pNew), &iNewCapacity);
	if ( iRet != XUI_OK ) return iRet;
	pNew = (xui_code_indicator_t*)xrtRealloc(pStore->pIndicators,
		sizeof(*pNew) * (size_t)iNewCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pStore->pIndicators = pNew;
	pStore->iIndicatorCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiCodeAnnotationReserveDiagnostics(xui_code_annotation_store pStore, int iCapacity)
{
	xui_code_diagnostic_t* pNew;
	int iNewCapacity;
	int iRet;

	if ( pStore == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pStore->iDiagnosticCapacity ) return XUI_OK;
	iRet = __xuiCodeAnnotationNextCapacity(pStore->iDiagnosticCapacity, iCapacity,
		sizeof(*pNew), &iNewCapacity);
	if ( iRet != XUI_OK ) return iRet;
	pNew = (xui_code_diagnostic_t*)xrtRealloc(pStore->pDiagnostics,
		sizeof(*pNew) * (size_t)iNewCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pStore->pDiagnostics = pNew;
	pStore->iDiagnosticCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiCodeAnnotationReserveDiagnosticIndex(xui_code_annotation_store pStore, int iCapacity)
{
	xui_code_diagnostic_index_t* pNew;
	int iNewCapacity;
	int iRet;

	if ( pStore == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pStore->iDiagnosticIndexCapacity ) return XUI_OK;
	iRet = __xuiCodeAnnotationNextCapacity(pStore->iDiagnosticIndexCapacity,
		iCapacity, sizeof(*pNew), &iNewCapacity);
	if ( iRet != XUI_OK ) return iRet;
	pNew = (xui_code_diagnostic_index_t*)xrtRealloc(pStore->pDiagnosticIndex,
		sizeof(*pNew) * (size_t)iNewCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pStore->pDiagnosticIndex = pNew;
	pStore->iDiagnosticIndexCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiCodeAnnotationCompareDiagnosticIndex(const void* pLeft, const void* pRight)
{
	const xui_code_diagnostic_index_t* pA = (const xui_code_diagnostic_index_t*)pLeft;
	const xui_code_diagnostic_index_t* pB = (const xui_code_diagnostic_index_t*)pRight;
	if ( pA->iStart != pB->iStart ) return (pA->iStart < pB->iStart) ? -1 : 1;
	if ( pA->iEnd != pB->iEnd ) return (pA->iEnd < pB->iEnd) ? -1 : 1;
	return (pA->iIndex < pB->iIndex) ? -1 : (pA->iIndex > pB->iIndex);
}

static int __xuiCodeAnnotationRebuildDiagnosticIndex(xui_code_annotation_store pStore)
{
	int i;
	int iMaxEnd;
	int iRet;

	if ( pStore == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pStore->iDiagnosticCount == 0 ) return XUI_OK;
	iRet = __xuiCodeAnnotationReserveDiagnosticIndex(pStore, pStore->iDiagnosticCount);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < pStore->iDiagnosticCount; i++ ) {
		pStore->pDiagnosticIndex[i].iStart = pStore->pDiagnostics[i].tRange.iStart;
		pStore->pDiagnosticIndex[i].iEnd = pStore->pDiagnostics[i].tRange.iEnd;
		pStore->pDiagnosticIndex[i].iIndex = i;
		pStore->pDiagnosticIndex[i].iPrefixMaxEnd = pStore->pDiagnostics[i].tRange.iEnd;
	}
	qsort(pStore->pDiagnosticIndex, (size_t)pStore->iDiagnosticCount, sizeof(*pStore->pDiagnosticIndex), __xuiCodeAnnotationCompareDiagnosticIndex);
	iMaxEnd = 0;
	for ( i = 0; i < pStore->iDiagnosticCount; i++ ) {
		if ( pStore->pDiagnosticIndex[i].iEnd > iMaxEnd ) iMaxEnd = pStore->pDiagnosticIndex[i].iEnd;
		pStore->pDiagnosticIndex[i].iPrefixMaxEnd = iMaxEnd;
	}
	return XUI_OK;
}

static void __xuiCodeAnnotationBumpDiagnosticVersion(xui_code_annotation_store pStore)
{
	if ( pStore == NULL ) return;
	pStore->iDiagnosticVersion++;
	if ( pStore->iDiagnosticVersion == 0u ) pStore->iDiagnosticVersion = 1u;
}

static void __xuiCodeAnnotationFreeDiagnosticStorage(xui_code_annotation_store pStore)
{
	if ( pStore == NULL ) return;
	__xuiCodeAnnotationClearDiagnosticText(pStore);
	xrtFree(pStore->pDiagnostics);
	xrtFree(pStore->pDiagnosticIndex);
	pStore->pDiagnostics = NULL;
	pStore->pDiagnosticIndex = NULL;
	pStore->iDiagnosticCount = 0;
	pStore->iDiagnosticCapacity = 0;
	pStore->iDiagnosticIndexCapacity = 0;
}

static int __xuiCodeAnnotationDiagnosticRangeFirst(xui_code_annotation_store pStore, int iStart)
{
	int iLow = 0;
	int iHigh = pStore->iDiagnosticCount;
	int iMid;

	while ( iLow < iHigh ) {
		iMid = iLow + (iHigh - iLow) / 2;
		if ( pStore->pDiagnosticIndex[iMid].iPrefixMaxEnd > iStart ) iHigh = iMid;
		else iLow = iMid + 1;
	}
	return iLow;
}

static int __xuiCodeAnnotationRangeOverlaps(int iStartA, int iEndA, int iStartB, int iEndB)
{
	return iStartA < iEndB && iStartB < iEndA;
}

static int __xuiCodeAnnotationRangeContains(const xui_code_range_t* pRange, int iOffset)
{
	return pRange->iStart <= iOffset && iOffset < pRange->iEnd;
}

static int __xuiCodeAnnotationStringEquals(const char* sLeft, const char* sRight)
{
	if ( sLeft == sRight ) return 1;
	if ( sLeft == NULL || sRight == NULL ) return 0;
	return strcmp(sLeft, sRight) == 0;
}

static int __xuiCodeAnnotationDiagnosticEquals(
	const xui_code_diagnostic_t* pLeft, const xui_code_diagnostic_t* pRight)
{
	if ( pLeft == NULL || pRight == NULL ) return 0;
	return pLeft->tRange.iStart == pRight->tRange.iStart &&
		pLeft->tRange.iEnd == pRight->tRange.iEnd &&
		pLeft->iSeverity == pRight->iSeverity &&
		pLeft->iUserData == pRight->iUserData &&
		__xuiCodeAnnotationStringEquals(pLeft->sCode, pRight->sCode) &&
		__xuiCodeAnnotationStringEquals(pLeft->sMessage, pRight->sMessage) &&
		__xuiCodeAnnotationStringEquals(pLeft->sSource, pRight->sSource);
}

static void __xuiCodeAnnotationCopyMarkerOut(xui_code_marker_t* pOut, const xui_code_marker_t* pMarker)
{
	*pOut = *pMarker;
	pOut->iSize = sizeof(*pOut);
}

static void __xuiCodeAnnotationCopyIndicatorOut(xui_code_indicator_t* pOut, const xui_code_indicator_t* pIndicator)
{
	*pOut = *pIndicator;
	pOut->iSize = sizeof(*pOut);
}

static void __xuiCodeAnnotationCopyDiagnosticOut(xui_code_diagnostic_t* pOut, const xui_code_diagnostic_t* pDiagnostic)
{
	*pOut = *pDiagnostic;
	pOut->iSize = sizeof(*pOut);
}

XUI_API int xuiCodeAnnotationStoreCreate(xui_code_annotation_store* ppStore)
{
	xui_code_annotation_store pStore;

	if ( ppStore == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppStore = NULL;
	pStore = (xui_code_annotation_store)xrtMalloc(sizeof(*pStore));
	if ( pStore == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pStore, 0, sizeof(*pStore));
	*ppStore = pStore;
	return XUI_OK;
}

static void __xuiCodeAnnotationStoreClear(xui_code_annotation_store pStore)
{
	bool hadDiagnostics = pStore->iDiagnosticCount > 0;
	for ( int i = 0; i < pStore->iMarkerCount; i++ ) __xuiCodeAnnotationFreeMarker(&pStore->pMarkers[i]);
	__xuiCodeAnnotationClearDiagnosticText(pStore);
	pStore->iMarkerCount = 0;
	pStore->iIndicatorCount = 0;
	pStore->iDiagnosticCount = 0;
	if ( hadDiagnostics ) __xuiCodeAnnotationBumpDiagnosticVersion(pStore);
}

static void __xuiCodeAnnotationStoreDestroy(xui_code_annotation_store pStore)
{
	__xuiCodeAnnotationStoreClear(pStore);
	xrtFree(pStore->pMarkers);
	xrtFree(pStore->pIndicators);
	xrtFree(pStore->pDiagnostics);
	xrtFree(pStore->pDiagnosticIndex);
	xrtFree(pStore);
}

void xuiInternalCodeAnnotationDestroyOwned(xui_code_annotation_store store, xui_widget editor)
{
	if ( store != NULL && (store->pOwner == NULL || store->pOwner == editor) )
		__xuiCodeAnnotationStoreDestroy(store);
}

XUI_API int xuiCodeAnnotationStoreDestroyChecked(xui_code_annotation_store store)
{
	int status = __xuiCodeAnnotationStatus(store, true);
	if ( status != XUI_OK ) return status;
	if ( store->pOwner != NULL ) return XUI_ERROR_INVALID_STATE;
	__xuiCodeAnnotationStoreDestroy(store);
	return XUI_OK;
}

XUI_API void xuiCodeAnnotationStoreDestroy(xui_code_annotation_store store)
{
	if ( store != NULL ) (void)xuiCodeAnnotationStoreDestroyChecked(store);
}

XUI_API int xuiCodeAnnotationStoreClearChecked(xui_code_annotation_store store)
{
	int status = __xuiCodeAnnotationStatus(store, true);
	if ( status != XUI_OK ) return status;
	if ( store->iMarkerCount || store->iIndicatorCount || store->iDiagnosticCount ) {
		bool diagnostics = store->iDiagnosticCount > 0;
		__xuiCodeAnnotationStoreClear(store);
		xuiInternalCodeAnnotationNotify(store, diagnostics);
	}
	return XUI_OK;
}

XUI_API void xuiCodeAnnotationStoreClear(xui_code_annotation_store store)
{
	if ( store != NULL ) (void)xuiCodeAnnotationStoreClearChecked(store);
}

XUI_API int xuiCodeAnnotationSetMarker(xui_code_annotation_store pStore, int iLine, int iMarker, uint32_t iFlags, const char* sTooltip, uintptr_t iUserData)
{
	xui_code_marker_t* pMarker;
	char* sCopy;
	int i;
	int iRet;

	iRet = __xuiCodeAnnotationStatus(pStore, true);
	if ( iRet != XUI_OK ) return iRet;
	if ( iLine < 0 || iMarker <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	sCopy = __xuiCodeAnnotationStrDup(sTooltip);
	if ( sTooltip != NULL && sCopy == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	for ( i = 0; i < pStore->iMarkerCount; i++ ) {
		if ( pStore->pMarkers[i].iLine == iLine && pStore->pMarkers[i].iMarker == iMarker ) {
			xrtFree((void*)pStore->pMarkers[i].sTooltip);
			pStore->pMarkers[i].sTooltip = sCopy;
			pStore->pMarkers[i].iFlags = iFlags;
			pStore->pMarkers[i].iUserData = iUserData;
			xuiInternalCodeAnnotationNotify(pStore, false);
			return XUI_OK;
		}
	}
	if ( pStore->iMarkerCount == INT_MAX ) {
		xrtFree(sCopy);
		return XUI_ERROR_LIMIT_EXCEEDED;
	}
	iRet = __xuiCodeAnnotationReserveMarkers(pStore, pStore->iMarkerCount + 1);
	if ( iRet != XUI_OK ) {
		xrtFree(sCopy);
		return iRet;
	}
	pMarker = &pStore->pMarkers[pStore->iMarkerCount++];
	memset(pMarker, 0, sizeof(*pMarker));
	pMarker->iSize = sizeof(*pMarker);
	pMarker->iLine = iLine;
	pMarker->iMarker = iMarker;
	pMarker->iFlags = iFlags;
	pMarker->sTooltip = sCopy;
	pMarker->iUserData = iUserData;
	xuiInternalCodeAnnotationNotify(pStore, false);
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationClearMarker(xui_code_annotation_store pStore, int iLine, int iMarker)
{
	int i;

	int status = __xuiCodeAnnotationStatus(pStore, true);
	if ( status != XUI_OK ) return status;
	if ( iLine < 0 || iMarker <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pStore->iMarkerCount; i++ ) {
		if ( pStore->pMarkers[i].iLine == iLine && pStore->pMarkers[i].iMarker == iMarker ) {
			__xuiCodeAnnotationFreeMarker(&pStore->pMarkers[i]);
			memmove(&pStore->pMarkers[i], &pStore->pMarkers[i + 1], sizeof(pStore->pMarkers[i]) * (size_t)(pStore->iMarkerCount - i - 1));
			pStore->iMarkerCount--;
			xuiInternalCodeAnnotationNotify(pStore, false);
			return XUI_OK;
		}
	}
	return XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiCodeAnnotationClearMarkers(xui_code_annotation_store pStore, int iLine)
{
	int iOriginalCount;
	int iWrite;
	int i;
	int iCleared;

	int status = __xuiCodeAnnotationStatus(pStore, true);
	if ( status != XUI_OK ) return status;
	if ( iLine < -1 ) return XUI_ERROR_INVALID_ARGUMENT;
	iOriginalCount = pStore->iMarkerCount;
	iCleared = 0;
	iWrite = 0;
	for ( i = 0; i < iOriginalCount; i++ ) {
		if ( iLine < 0 || pStore->pMarkers[i].iLine == iLine ) {
			__xuiCodeAnnotationFreeMarker(&pStore->pMarkers[i]);
			iCleared++;
			continue;
		}
		if ( iWrite != i ) pStore->pMarkers[iWrite] = pStore->pMarkers[i];
		iWrite++;
	}
	if ( iWrite < iOriginalCount ) {
		memset(&pStore->pMarkers[iWrite], 0,
			sizeof(*pStore->pMarkers) * (size_t)(iOriginalCount - iWrite));
	}
	pStore->iMarkerCount = iWrite;
	if ( iCleared > 0 ) xuiInternalCodeAnnotationNotify(pStore, false);
	return (iCleared > 0) ? XUI_OK : XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiCodeAnnotationGetMarkers(xui_code_annotation_store pStore, int iLine, xui_code_marker_t* pMarkers, int iMarkerCapacity, int* pMarkerCount)
{
	int i;
	int iCount;

	if ( (pStore == NULL) || (iLine < 0) || (pMarkerCount == NULL) ||
	     iMarkerCapacity < 0 || (pMarkers == NULL && iMarkerCapacity > 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iCount = 0;
	for ( i = 0; i < pStore->iMarkerCount; i++ ) {
		if ( pStore->pMarkers[i].iLine != iLine ) continue;
		if ( pMarkers != NULL && iCount < iMarkerCapacity ) __xuiCodeAnnotationCopyMarkerOut(&pMarkers[iCount], &pStore->pMarkers[i]);
		iCount++;
	}
	*pMarkerCount = iCount;
	return XUI_OK;
}

int xuiInternalCodeAnnotationSetIndicator(xui_code_annotation_store pStore, int iIndicator, int iStyle, int iStart, int iEnd, uint32_t iFlags, uintptr_t iUserData)
{
	xui_code_indicator_t* pIndicator;
	int iRet;

	if ( (pStore == NULL) || (iIndicator <= 0) || (iStart < 0) || (iEnd <= iStart) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pStore->iIndicatorCount == INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
	iRet = __xuiCodeAnnotationReserveIndicators(pStore, pStore->iIndicatorCount + 1);
	if ( iRet != XUI_OK ) return iRet;
	pIndicator = &pStore->pIndicators[pStore->iIndicatorCount++];
	memset(pIndicator, 0, sizeof(*pIndicator));
	pIndicator->iSize = sizeof(*pIndicator);
	pIndicator->iIndicator = iIndicator;
	pIndicator->iStyle = iStyle;
	pIndicator->tRange.iStart = iStart;
	pIndicator->tRange.iEnd = iEnd;
	pIndicator->iFlags = iFlags;
	pIndicator->iUserData = iUserData;
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationSetIndicator(xui_code_annotation_store store, int indicator, int style,
	int start, int end, uint32_t flags, uintptr_t userData)
{
	int status = __xuiCodeAnnotationStatus(store, true);
	if ( status != XUI_OK ) return status;
	status = xuiInternalCodeAnnotationSetIndicator(store, indicator, style, start, end, flags, userData);
	if ( status == XUI_OK ) xuiInternalCodeAnnotationNotify(store, false);
	return status;
}

XUI_API int xuiCodeAnnotationClearIndicator(xui_code_annotation_store pStore, int iIndicator, int iStart, int iEnd)
{
	xui_code_indicator_t* pNew;
	xui_code_indicator_t tIndicator;
	xui_code_indicator_t tRight;
	int iOriginalCount;
	int iFinalCount;
	int iSplitCount;
	int iRemoveCount;
	int iWrite;
	int i;
	int iCleared;

	int status = __xuiCodeAnnotationStatus(pStore, true);
	if ( status != XUI_OK ) return status;
	if ( iIndicator <= 0 || iStart < 0 || iEnd <= iStart ) return XUI_ERROR_INVALID_ARGUMENT;
	iCleared = 0;
	iSplitCount = 0;
	iRemoveCount = 0;
	for ( i = 0; i < pStore->iIndicatorCount; i++ ) {
		if ( pStore->pIndicators[i].iIndicator == iIndicator && __xuiCodeAnnotationRangeOverlaps(pStore->pIndicators[i].tRange.iStart, pStore->pIndicators[i].tRange.iEnd, iStart, iEnd) ) {
			iCleared++;
			if ( iStart <= pStore->pIndicators[i].tRange.iStart &&
			     iEnd >= pStore->pIndicators[i].tRange.iEnd ) {
				iRemoveCount++;
			} else if ( iStart > pStore->pIndicators[i].tRange.iStart &&
			            iEnd < pStore->pIndicators[i].tRange.iEnd ) {
				iSplitCount++;
			}
		}
	}
	if ( iCleared == 0 ) return XUI_ERROR_UNSUPPORTED;
	if ( iSplitCount == 0 ) {
		iWrite = 0;
		for ( i = 0; i < pStore->iIndicatorCount; i++ ) {
			tIndicator = pStore->pIndicators[i];
			if ( tIndicator.iIndicator != iIndicator ||
			     !__xuiCodeAnnotationRangeOverlaps(tIndicator.tRange.iStart,
				tIndicator.tRange.iEnd, iStart, iEnd) ) {
				pStore->pIndicators[iWrite++] = tIndicator;
				continue;
			}
			if ( iStart <= tIndicator.tRange.iStart &&
			     iEnd >= tIndicator.tRange.iEnd ) continue;
			if ( iStart <= tIndicator.tRange.iStart ) {
				tIndicator.tRange.iStart = iEnd;
			} else {
				tIndicator.tRange.iEnd = iStart;
			}
			pStore->pIndicators[iWrite++] = tIndicator;
		}
		pStore->iIndicatorCount = iWrite;
		xuiInternalCodeAnnotationNotify(pStore, false);
		return XUI_OK;
	}
	if ( iSplitCount > INT_MAX - pStore->iIndicatorCount ) return XUI_ERROR_LIMIT_EXCEEDED;
	iOriginalCount = pStore->iIndicatorCount;
	iFinalCount = iOriginalCount - iRemoveCount + iSplitCount;
	if ( (size_t)iFinalCount > SIZE_MAX / sizeof(*pNew) ) return XUI_ERROR_LIMIT_EXCEEDED;
	pNew = NULL;
	if ( iFinalCount > 0 ) {
		pNew = (xui_code_indicator_t*)xrtMalloc(sizeof(*pNew) * (size_t)iFinalCount);
		if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	}
	iWrite = 0;
	for ( i = 0; i < iOriginalCount; i++ ) {
		tIndicator = pStore->pIndicators[i];
		if ( tIndicator.iIndicator != iIndicator ||
		     !__xuiCodeAnnotationRangeOverlaps(tIndicator.tRange.iStart,
			 tIndicator.tRange.iEnd, iStart, iEnd) ) {
			pNew[iWrite++] = tIndicator;
			continue;
		}
		if ( iStart <= tIndicator.tRange.iStart && iEnd >= tIndicator.tRange.iEnd ) {
			continue;
		}
		if ( iStart <= tIndicator.tRange.iStart ) {
			tIndicator.tRange.iStart = iEnd;
			pNew[iWrite++] = tIndicator;
			continue;
		}
		if ( iEnd >= tIndicator.tRange.iEnd ) {
			tIndicator.tRange.iEnd = iStart;
			pNew[iWrite++] = tIndicator;
			continue;
		}
		tRight = tIndicator;
		tIndicator.tRange.iEnd = iStart;
		tRight.tRange.iStart = iEnd;
		pNew[iWrite++] = tIndicator;
		pNew[iWrite++] = tRight;
	}
	if ( iWrite != iFinalCount ) {
		xrtFree(pNew);
		return XUI_ERROR_INVALID_STATE;
	}
	xrtFree(pStore->pIndicators);
	pStore->pIndicators = pNew;
	pStore->iIndicatorCount = iFinalCount;
	pStore->iIndicatorCapacity = iFinalCount;
	xuiInternalCodeAnnotationNotify(pStore, false);
	return XUI_OK;
}

int xuiInternalCodeAnnotationClearIndicators(xui_code_annotation_store pStore, int iIndicator)
{
	int iWrite;
	int i;
	int iCleared;

	if ( (pStore == NULL) || (iIndicator < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	iCleared = 0;
	iWrite = 0;
	for ( i = 0; i < pStore->iIndicatorCount; i++ ) {
		if ( iIndicator == 0 || pStore->pIndicators[i].iIndicator == iIndicator ) {
			iCleared++;
			continue;
		}
		if ( iWrite != i ) pStore->pIndicators[iWrite] = pStore->pIndicators[i];
		iWrite++;
	}
	pStore->iIndicatorCount = iWrite;
	return (iCleared > 0) ? XUI_OK : XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiCodeAnnotationClearIndicators(xui_code_annotation_store store, int indicator)
{
	int status = __xuiCodeAnnotationStatus(store, true);
	if ( status != XUI_OK ) return status;
	status = xuiInternalCodeAnnotationClearIndicators(store, indicator);
	if ( status == XUI_OK ) xuiInternalCodeAnnotationNotify(store, false);
	return status;
}

XUI_API int xuiCodeAnnotationGetIndicatorsAt(xui_code_annotation_store pStore, int iOffset, xui_code_indicator_t* pIndicators, int iIndicatorCapacity, int* pIndicatorCount)
{
	int i;
	int iCount;

	if ( (pStore == NULL) || (iOffset < 0) || (pIndicatorCount == NULL) ||
	     iIndicatorCapacity < 0 || (pIndicators == NULL && iIndicatorCapacity > 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iCount = 0;
	for ( i = 0; i < pStore->iIndicatorCount; i++ ) {
		if ( !__xuiCodeAnnotationRangeContains(&pStore->pIndicators[i].tRange, iOffset) ) continue;
		if ( pIndicators != NULL && iCount < iIndicatorCapacity ) __xuiCodeAnnotationCopyIndicatorOut(&pIndicators[iCount], &pStore->pIndicators[i]);
		iCount++;
	}
	*pIndicatorCount = iCount;
	return XUI_OK;
}

int xuiInternalCodeAnnotationApplyDiagnostics(xui_code_annotation_store pStore,
	const xui_code_diagnostic_t* pDiagnostics, int iDiagnosticCount, xui_widget editor)
{
	xui_code_annotation_store_t tNew;
	int i;
	int iRet;

	if ( (pStore == NULL) || (iDiagnosticCount < 0) || (iDiagnosticCount > 0 && pDiagnostics == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < iDiagnosticCount; i++ ) {
		if ( (pDiagnostics[i].iSize != 0u &&
		     pDiagnostics[i].iSize < sizeof(pDiagnostics[i])) ||
		     pDiagnostics[i].tRange.iStart < 0 ||
		     pDiagnostics[i].tRange.iEnd <= pDiagnostics[i].tRange.iStart ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( iDiagnosticCount == 0 && pStore->iDiagnosticCount == 0 ) return XUI_OK;
	if ( iDiagnosticCount == pStore->iDiagnosticCount ) {
		for ( i = 0; i < iDiagnosticCount; i++ ) {
			if ( !__xuiCodeAnnotationDiagnosticEquals(
			     &pStore->pDiagnostics[i], &pDiagnostics[i]) ) break;
		}
		if ( i == iDiagnosticCount ) return XUI_OK;
	}
	memset(&tNew, 0, sizeof(tNew));
	iRet = __xuiCodeAnnotationReserveDiagnostics(&tNew, iDiagnosticCount);
	if ( iRet == XUI_OK ) {
		iRet = __xuiCodeAnnotationReserveDiagnosticIndex(&tNew, iDiagnosticCount);
	}
	if ( iRet != XUI_OK ) {
		__xuiCodeAnnotationFreeDiagnosticStorage(&tNew);
		return iRet;
	}
	iRet = __xuiCodeAnnotationCopyDiagnostics(&tNew, pDiagnostics, iDiagnosticCount);
	if ( iRet != XUI_OK ) {
		__xuiCodeAnnotationFreeDiagnosticStorage(&tNew);
		return iRet;
	}
	iRet = __xuiCodeAnnotationRebuildDiagnosticIndex(&tNew);
	if ( iRet != XUI_OK ) {
		__xuiCodeAnnotationFreeDiagnosticStorage(&tNew);
		return iRet;
	}
	if ( editor != NULL ) {
		iRet = xuiWidgetInvalidate(editor, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) {
			__xuiCodeAnnotationFreeDiagnosticStorage(&tNew);
			return iRet;
		}
	}
	__xuiCodeAnnotationFreeDiagnosticStorage(pStore);
	pStore->pDiagnostics = tNew.pDiagnostics;
	pStore->pDiagnosticText = tNew.pDiagnosticText;
	pStore->iDiagnosticCount = tNew.iDiagnosticCount;
	pStore->iDiagnosticCapacity = tNew.iDiagnosticCapacity;
	pStore->pDiagnosticIndex = tNew.pDiagnosticIndex;
	pStore->iDiagnosticIndexCapacity = tNew.iDiagnosticIndexCapacity;
	__xuiCodeAnnotationBumpDiagnosticVersion(pStore);
	xuiInternalCodeAnnotationNotify(pStore, true);
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationSetDiagnostics(xui_code_annotation_store pStore,
	const xui_code_diagnostic_t* pDiagnostics, int iDiagnosticCount)
{
	int status = __xuiCodeAnnotationStatus(pStore, true);
	if ( status != XUI_OK ) return status;
	return xuiInternalCodeAnnotationApplyDiagnostics(pStore, pDiagnostics, iDiagnosticCount, pStore->pOwner);
}

XUI_API int xuiCodeAnnotationClearDiagnostics(xui_code_annotation_store pStore)
{
	int status = __xuiCodeAnnotationStatus(pStore, true);
	if ( status != XUI_OK ) return status;
	if ( pStore->iDiagnosticCount == 0 ) return XUI_OK;
	__xuiCodeAnnotationClearDiagnosticText(pStore);
	pStore->iDiagnosticCount = 0;
	__xuiCodeAnnotationBumpDiagnosticVersion(pStore);
	xuiInternalCodeAnnotationNotify(pStore, true);
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationGetDiagnosticCount(xui_code_annotation_store pStore)
{
	if ( pStore == NULL ) return 0;
	int status = __xuiCodeAnnotationStatus(pStore, false);
	return status == XUI_OK ? pStore->iDiagnosticCount : status;
}

XUI_API int xuiCodeAnnotationGetDiagnostic(xui_code_annotation_store pStore, int iIndex, xui_code_diagnostic_t* pDiagnostic)
{
	int status = __xuiCodeAnnotationStatus(pStore, false);
	if ( status != XUI_OK ) return status;
	if ( pDiagnostic == NULL || iIndex < 0 || iIndex >= pStore->iDiagnosticCount ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiCodeAnnotationCopyDiagnosticOut(pDiagnostic, &pStore->pDiagnostics[iIndex]);
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationGetDiagnosticsAt(xui_code_annotation_store pStore, int iOffset, xui_code_diagnostic_t* pDiagnostics, int iDiagnosticCapacity, int* pDiagnosticCount)
{
	if ( iOffset < 0 || iOffset == INT_MAX ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiCodeAnnotationGetDiagnosticsInRange(pStore, iOffset, iOffset + 1, pDiagnostics, NULL, iDiagnosticCapacity, pDiagnosticCount);
}

XUI_API int xuiCodeAnnotationGetDiagnosticsInRange(xui_code_annotation_store pStore, int iStart, int iEnd, xui_code_diagnostic_t* pDiagnostics, int* pIndices, int iDiagnosticCapacity, int* pDiagnosticCount)
{
	const xui_code_diagnostic_index_t* pEntry;
	int i;
	int iCount;
	int iIndex;

	if ( (pStore == NULL) || (iStart < 0) || (iEnd <= iStart) ||
	     (iDiagnosticCapacity < 0) || (pDiagnosticCount == NULL) ||
	     (pDiagnostics == NULL && pIndices == NULL && iDiagnosticCapacity > 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iCount = 0;
	i = __xuiCodeAnnotationDiagnosticRangeFirst(pStore, iStart);
	for ( ; i < pStore->iDiagnosticCount; i++ ) {
		pEntry = &pStore->pDiagnosticIndex[i];
		if ( pEntry->iStart >= iEnd ) break;
		if ( pEntry->iEnd <= iStart ) continue;
		iIndex = pEntry->iIndex;
		if ( iCount < iDiagnosticCapacity ) {
			if ( pDiagnostics != NULL ) __xuiCodeAnnotationCopyDiagnosticOut(&pDiagnostics[iCount], &pStore->pDiagnostics[iIndex]);
			if ( pIndices != NULL ) pIndices[iCount] = iIndex;
		}
		iCount++;
	}
	*pDiagnosticCount = iCount;
	return XUI_OK;
}

XUI_API uint32_t xuiCodeAnnotationGetDiagnosticVersion(xui_code_annotation_store pStore)
{
	return (pStore != NULL) ? pStore->iDiagnosticVersion : 0u;
}

static int __xuiCodeAnnotationTrackRange(xui_code_range_t* pRange,
	int iStartOffset, int iEndOffset, int iNewEndOffset, int bApply)
{
	int64_t iDelta;
	int64_t iStart;
	int64_t iEnd;

	if ( pRange == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iDelta = (int64_t)iNewEndOffset - (int64_t)iEndOffset;
	iStart = pRange->iStart;
	iEnd = pRange->iEnd;
	if ( iEnd <= iStartOffset ) return XUI_OK;
	if ( pRange->iStart >= iEndOffset ) {
		iStart += iDelta;
		iEnd += iDelta;
	} else {
		if ( iStart >= iStartOffset ) iStart = iStartOffset;
		if ( iEnd <= iEndOffset ) iEnd = iNewEndOffset;
		else iEnd += iDelta;
		if ( iEnd <= iStart ) iEnd = iStart + 1;
	}
	if ( iStart < 0 || iEnd <= iStart || iStart > INT_MAX || iEnd > INT_MAX ) {
		return XUI_ERROR_LIMIT_EXCEEDED;
	}
	if ( bApply ) {
		pRange->iStart = (int)iStart;
		pRange->iEnd = (int)iEnd;
	}
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationTrackEdit(xui_code_annotation_store pStore, int iStartOffset, int iEndOffset, int iNewEndOffset, int iStartLine, int iEndLine, int iNewEndLine)
{
	xui_code_range_t tTrackedRange;
	int64_t iLineDelta;
	int64_t iNewLine;
	int bDiagnosticRangeChanged;
	int bDiagnosticTouched;
	int i;
	int iRet;

	if ( (pStore == NULL) || (iStartOffset < 0) || (iEndOffset < iStartOffset) || (iNewEndOffset < iStartOffset) || (iStartLine < 0) || (iEndLine < iStartLine) || (iNewEndLine < iStartLine) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iLineDelta = (int64_t)iNewEndLine - (int64_t)iEndLine;
	bDiagnosticRangeChanged = 0;
	bDiagnosticTouched = 0;
	for ( i = 0; i < pStore->iMarkerCount; i++ ) {
		iNewLine = pStore->pMarkers[i].iLine;
		if ( iNewLine > iEndLine ) iNewLine += iLineDelta;
		else if ( iNewLine >= iStartLine ) iNewLine = iStartLine;
		if ( iNewLine < 0 || iNewLine > INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
	}
	for ( i = 0; i < pStore->iIndicatorCount; i++ ) {
		iRet = __xuiCodeAnnotationTrackRange(&pStore->pIndicators[i].tRange,
			iStartOffset, iEndOffset, iNewEndOffset, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( i = 0; i < pStore->iDiagnosticCount; i++ ) {
		tTrackedRange = pStore->pDiagnostics[i].tRange;
		iRet = __xuiCodeAnnotationTrackRange(&tTrackedRange,
			iStartOffset, iEndOffset, iNewEndOffset, 1);
		if ( iRet != XUI_OK ) return iRet;
		if ( tTrackedRange.iStart != pStore->pDiagnostics[i].tRange.iStart ||
		     tTrackedRange.iEnd != pStore->pDiagnostics[i].tRange.iEnd ) {
			bDiagnosticRangeChanged = 1;
			bDiagnosticTouched = 1;
		} else if ( iEndOffset > iStartOffset &&
		           __xuiCodeAnnotationRangeOverlaps(pStore->pDiagnostics[i].tRange.iStart,
			   pStore->pDiagnostics[i].tRange.iEnd, iStartOffset, iEndOffset) ) {
			bDiagnosticTouched = 1;
		}
	}
	if ( bDiagnosticRangeChanged ) {
		iRet = __xuiCodeAnnotationReserveDiagnosticIndex(pStore,
			pStore->iDiagnosticCount);
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( i = 0; i < pStore->iMarkerCount; i++ ) {
		if ( pStore->pMarkers[i].iLine > iEndLine ) {
			pStore->pMarkers[i].iLine = (int)((int64_t)pStore->pMarkers[i].iLine + iLineDelta);
		}
		else if ( pStore->pMarkers[i].iLine >= iStartLine ) pStore->pMarkers[i].iLine = iStartLine;
	}
	for ( i = 0; i < pStore->iIndicatorCount; i++ ) {
		(void)__xuiCodeAnnotationTrackRange(&pStore->pIndicators[i].tRange,
			iStartOffset, iEndOffset, iNewEndOffset, 1);
	}
	for ( i = 0; i < pStore->iDiagnosticCount; i++ ) {
		(void)__xuiCodeAnnotationTrackRange(&pStore->pDiagnostics[i].tRange,
			iStartOffset, iEndOffset, iNewEndOffset, 1);
	}
	if ( bDiagnosticRangeChanged ) {
		iRet = __xuiCodeAnnotationRebuildDiagnosticIndex(pStore);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( bDiagnosticTouched ) __xuiCodeAnnotationBumpDiagnosticVersion(pStore);
	return XUI_OK;
}

typedef enum {
	XUI_ANNOTATION_CAPTURE_MARKERS,
	XUI_ANNOTATION_CAPTURE_INDICATORS,
	XUI_ANNOTATION_CAPTURE_DIAGNOSTICS
} xui_code_annotation_capture_kind_t;

struct xui_code_annotation_snapshot_t {
	xui_code_diagnostic_text_t* pDiagnosticText;
	xui_code_annotation_capture_kind_t kind;
	int count;
};

_Static_assert(sizeof(struct xui_code_annotation_snapshot_t) % _Alignof(xui_code_marker_t) == 0,
	"annotation marker capture alignment");
_Static_assert(sizeof(struct xui_code_annotation_snapshot_t) % _Alignof(xui_code_indicator_t) == 0,
	"annotation indicator capture alignment");
_Static_assert(sizeof(struct xui_code_annotation_snapshot_t) % _Alignof(xui_code_diagnostic_t) == 0 &&
	sizeof(xui_code_diagnostic_t) % _Alignof(int) == 0, "annotation diagnostic capture alignment");

static int __xuiCodeAnnotationSnapshotCreate(xui_code_annotation_capture_kind_t kind,
	int count, size_t stride, size_t extra, xui_code_annotation_snapshot* out)
{
	size_t header = sizeof(**out);
	if ( extra > SIZE_MAX - header || (size_t)count > (SIZE_MAX - header - extra) / stride )
		return XUI_ERROR_LIMIT_EXCEEDED;
	xui_code_annotation_snapshot snapshot = xrtMalloc(header + (size_t)count * stride + extra);
	if ( snapshot == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	*snapshot = (struct xui_code_annotation_snapshot_t){NULL, kind, count};
	*out = snapshot;
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationCaptureMarkers(xui_code_annotation_store store, int line,
	xui_code_annotation_snapshot* out)
{
	if ( out == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	int status = __xuiCodeAnnotationStatus(store, false);
	if ( status != XUI_OK ) return status;
	if ( line < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	size_t bytes = 0;
	int count = 0;
	for ( int i = 0; i < store->iMarkerCount; ++i ) {
		const xui_code_marker_t* marker = &store->pMarkers[i];
		if ( marker->iLine != line ) continue;
		++count;
		if ( marker->sTooltip == NULL ) continue;
		size_t length = strlen(marker->sTooltip);
		if ( length >= SIZE_MAX - bytes ) return XUI_ERROR_LIMIT_EXCEEDED;
		bytes += length + 1;
	}
	xui_code_annotation_snapshot snapshot;
	status = __xuiCodeAnnotationSnapshotCreate(XUI_ANNOTATION_CAPTURE_MARKERS, count,
		sizeof(xui_code_marker_t), bytes, &snapshot);
	if ( status != XUI_OK ) return status;
	xui_code_marker_t* markers = (void*)(snapshot + 1);
	int copied;
	status = xuiCodeAnnotationGetMarkers(store, line, markers, count, &copied);
	if ( status != XUI_OK || copied != count ) {
		xuiCodeAnnotationSnapshotDestroy(snapshot);
		return status != XUI_OK ? status : XUI_ERROR_INVALID_STATE;
	}
	char* text = (void*)(markers + count);
	for ( int i = 0; i < count; ++i ) {
		if ( markers[i].sTooltip == NULL ) continue;
		size_t length = strlen(markers[i].sTooltip) + 1;
		memcpy(text, markers[i].sTooltip, length);
		markers[i].sTooltip = text;
		text += length;
	}
	*out = snapshot;
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationCaptureIndicatorsAt(xui_code_annotation_store store, int offset,
	xui_code_annotation_snapshot* out)
{
	if ( out == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	int status = __xuiCodeAnnotationStatus(store, false);
	if ( status != XUI_OK ) return status;
	int count;
	status = xuiCodeAnnotationGetIndicatorsAt(store, offset, NULL, 0, &count);
	if ( status != XUI_OK ) return status;
	xui_code_annotation_snapshot snapshot;
	status = __xuiCodeAnnotationSnapshotCreate(XUI_ANNOTATION_CAPTURE_INDICATORS, count,
		sizeof(xui_code_indicator_t), 0, &snapshot);
	if ( status != XUI_OK ) return status;
	int copied;
	status = xuiCodeAnnotationGetIndicatorsAt(store, offset, (void*)(snapshot + 1), count, &copied);
	if ( status != XUI_OK || copied != count ) {
		xuiCodeAnnotationSnapshotDestroy(snapshot);
		return status != XUI_OK ? status : XUI_ERROR_INVALID_STATE;
	}
	*out = snapshot;
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationCaptureDiagnosticsInRange(xui_code_annotation_store store, int start, int end,
	xui_code_annotation_snapshot* out)
{
	if ( out == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	int status = __xuiCodeAnnotationStatus(store, false);
	if ( status != XUI_OK ) return status;
	int count;
	status = xuiCodeAnnotationGetDiagnosticsInRange(store, start, end, NULL, NULL, 0, &count);
	if ( status != XUI_OK ) return status;
	xui_code_annotation_snapshot snapshot;
	status = __xuiCodeAnnotationSnapshotCreate(XUI_ANNOTATION_CAPTURE_DIAGNOSTICS, count,
		sizeof(xui_code_diagnostic_t) + sizeof(int), 0, &snapshot);
	if ( status != XUI_OK ) return status;
	if ( count > 0 ) {
		xui_code_diagnostic_t* records = (void*)(snapshot + 1);
		int* indices = (void*)(records + count);
		int copied;
		status = xuiCodeAnnotationGetDiagnosticsInRange(store, start, end, records, indices, count, &copied);
		if ( status == XUI_OK && copied != count ) status = XUI_ERROR_INVALID_STATE;
		if ( status == XUI_OK ) {
			xui_code_diagnostic_text_t* text = NULL;
			status = xuiInternalCodeAnnotationPinDiagnosticText(store, &text);
			if ( status == XUI_OK ) snapshot->pDiagnosticText = text;
		}
		if ( status != XUI_OK ) {
			xuiCodeAnnotationSnapshotDestroy(snapshot);
			return status;
		}
	}
	*out = snapshot;
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationCaptureDiagnosticsAt(xui_code_annotation_store store, int offset,
	xui_code_annotation_snapshot* out)
{
	if ( out == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*out = NULL;
	if ( offset < 0 || offset == INT_MAX ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiCodeAnnotationCaptureDiagnosticsInRange(store, offset, offset + 1, out);
}

XUI_API void xuiCodeAnnotationSnapshotDestroy(xui_code_annotation_snapshot snapshot)
{
	if ( snapshot == NULL ) return;
	xuiInternalCodeAnnotationUnpinDiagnosticText(snapshot->pDiagnosticText);
	xrtFree(snapshot);
}

XUI_API int xuiCodeAnnotationSnapshotGetCount(xui_code_annotation_snapshot snapshot)
{
	return snapshot != NULL ? snapshot->count : XUI_ERROR_INVALID_ARGUMENT;
}

static bool __xuiCodeAnnotationSnapshotHas(xui_code_annotation_snapshot snapshot, int index,
	xui_code_annotation_capture_kind_t kind)
{
	return snapshot != NULL && snapshot->kind == kind && index >= 0 && index < snapshot->count;
}

XUI_API int xuiCodeAnnotationSnapshotGetMarker(xui_code_annotation_snapshot snapshot, int index,
	xui_code_marker_t* marker)
{
	if ( marker == NULL || !__xuiCodeAnnotationSnapshotHas(snapshot, index, XUI_ANNOTATION_CAPTURE_MARKERS) )
		return XUI_ERROR_INVALID_ARGUMENT;
	const xui_code_marker_t* records = (const void*)(snapshot + 1);
	*marker = records[index];
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationSnapshotGetIndicator(xui_code_annotation_snapshot snapshot, int index,
	xui_code_indicator_t* indicator)
{
	if ( indicator == NULL || !__xuiCodeAnnotationSnapshotHas(snapshot, index, XUI_ANNOTATION_CAPTURE_INDICATORS) )
		return XUI_ERROR_INVALID_ARGUMENT;
	const xui_code_indicator_t* records = (const void*)(snapshot + 1);
	*indicator = records[index];
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationSnapshotGetDiagnostic(xui_code_annotation_snapshot snapshot, int index,
	xui_code_diagnostic_capture_t* diagnostic)
{
	if ( diagnostic == NULL || !__xuiCodeAnnotationSnapshotHas(snapshot, index, XUI_ANNOTATION_CAPTURE_DIAGNOSTICS) )
		return XUI_ERROR_INVALID_ARGUMENT;
	const xui_code_diagnostic_t* records = (const void*)(snapshot + 1);
	const int* indices = (const void*)(records + snapshot->count);
	*diagnostic = (xui_code_diagnostic_capture_t){records[index], indices[index]};
	return XUI_OK;
}

#endif
