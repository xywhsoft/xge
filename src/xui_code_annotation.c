#include "../xui.h"

#include <string.h>

struct xui_code_annotation_store_t {
	xui_code_marker_t* pMarkers;
	int iMarkerCount;
	int iMarkerCapacity;
	xui_code_indicator_t* pIndicators;
	int iIndicatorCount;
	int iIndicatorCapacity;
	xui_code_diagnostic_t* pDiagnostics;
	int iDiagnosticCount;
	int iDiagnosticCapacity;
};

static char* __xuiCodeAnnotationStrDup(const char* sText)
{
	char* sCopy;
	size_t iLength;

	if ( sText == NULL ) return NULL;
	iLength = strlen(sText);
	sCopy = (char*)xrtMalloc(iLength + 1u);
	if ( sCopy == NULL ) return NULL;
	memcpy(sCopy, sText, iLength + 1u);
	return sCopy;
}

static void __xuiCodeAnnotationFreeMarker(xui_code_marker_t* pMarker)
{
	if ( pMarker == NULL ) return;
	xrtFree((void*)pMarker->sTooltip);
	memset(pMarker, 0, sizeof(*pMarker));
}

static void __xuiCodeAnnotationFreeDiagnostic(xui_code_diagnostic_t* pDiagnostic)
{
	if ( pDiagnostic == NULL ) return;
	xrtFree((void*)pDiagnostic->sCode);
	xrtFree((void*)pDiagnostic->sMessage);
	xrtFree((void*)pDiagnostic->sSource);
	memset(pDiagnostic, 0, sizeof(*pDiagnostic));
}

static int __xuiCodeAnnotationReserveMarkers(xui_code_annotation_store pStore, int iCapacity)
{
	xui_code_marker_t* pNew;

	if ( iCapacity <= pStore->iMarkerCapacity ) return XUI_OK;
	if ( iCapacity < pStore->iMarkerCapacity * 2 ) iCapacity = pStore->iMarkerCapacity * 2;
	if ( iCapacity < 16 ) iCapacity = 16;
	pNew = (xui_code_marker_t*)xrtRealloc(pStore->pMarkers, sizeof(*pNew) * (size_t)iCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pStore->pMarkers = pNew;
	pStore->iMarkerCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiCodeAnnotationReserveIndicators(xui_code_annotation_store pStore, int iCapacity)
{
	xui_code_indicator_t* pNew;

	if ( iCapacity <= pStore->iIndicatorCapacity ) return XUI_OK;
	if ( iCapacity < pStore->iIndicatorCapacity * 2 ) iCapacity = pStore->iIndicatorCapacity * 2;
	if ( iCapacity < 16 ) iCapacity = 16;
	pNew = (xui_code_indicator_t*)xrtRealloc(pStore->pIndicators, sizeof(*pNew) * (size_t)iCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pStore->pIndicators = pNew;
	pStore->iIndicatorCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiCodeAnnotationReserveDiagnostics(xui_code_annotation_store pStore, int iCapacity)
{
	xui_code_diagnostic_t* pNew;

	if ( iCapacity <= pStore->iDiagnosticCapacity ) return XUI_OK;
	if ( iCapacity < pStore->iDiagnosticCapacity * 2 ) iCapacity = pStore->iDiagnosticCapacity * 2;
	if ( iCapacity < 16 ) iCapacity = 16;
	pNew = (xui_code_diagnostic_t*)xrtRealloc(pStore->pDiagnostics, sizeof(*pNew) * (size_t)iCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pStore->pDiagnostics = pNew;
	pStore->iDiagnosticCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiCodeAnnotationRangeOverlaps(int iStartA, int iEndA, int iStartB, int iEndB)
{
	return iStartA < iEndB && iStartB < iEndA;
}

static int __xuiCodeAnnotationRangeContains(const xui_code_range_t* pRange, int iOffset)
{
	return pRange->iStart <= iOffset && iOffset < pRange->iEnd;
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

XUI_API void xuiCodeAnnotationStoreDestroy(xui_code_annotation_store pStore)
{
	if ( pStore == NULL ) return;
	xuiCodeAnnotationStoreClear(pStore);
	xrtFree(pStore->pMarkers);
	xrtFree(pStore->pIndicators);
	xrtFree(pStore->pDiagnostics);
	xrtFree(pStore);
}

XUI_API void xuiCodeAnnotationStoreClear(xui_code_annotation_store pStore)
{
	int i;

	if ( pStore == NULL ) return;
	for ( i = 0; i < pStore->iMarkerCount; i++ ) __xuiCodeAnnotationFreeMarker(&pStore->pMarkers[i]);
	for ( i = 0; i < pStore->iDiagnosticCount; i++ ) __xuiCodeAnnotationFreeDiagnostic(&pStore->pDiagnostics[i]);
	pStore->iMarkerCount = 0;
	pStore->iIndicatorCount = 0;
	pStore->iDiagnosticCount = 0;
}

XUI_API int xuiCodeAnnotationSetMarker(xui_code_annotation_store pStore, int iLine, int iMarker, uint32_t iFlags, const char* sTooltip, uintptr_t iUserData)
{
	xui_code_marker_t* pMarker;
	char* sCopy;
	int i;
	int iRet;

	if ( (pStore == NULL) || (iLine < 0) || (iMarker <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	sCopy = __xuiCodeAnnotationStrDup(sTooltip);
	if ( sTooltip != NULL && sCopy == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	for ( i = 0; i < pStore->iMarkerCount; i++ ) {
		if ( pStore->pMarkers[i].iLine == iLine && pStore->pMarkers[i].iMarker == iMarker ) {
			xrtFree((void*)pStore->pMarkers[i].sTooltip);
			pStore->pMarkers[i].sTooltip = sCopy;
			pStore->pMarkers[i].iFlags = iFlags;
			pStore->pMarkers[i].iUserData = iUserData;
			return XUI_OK;
		}
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
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationClearMarker(xui_code_annotation_store pStore, int iLine, int iMarker)
{
	int i;

	if ( (pStore == NULL) || (iLine < 0) || (iMarker <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pStore->iMarkerCount; i++ ) {
		if ( pStore->pMarkers[i].iLine == iLine && pStore->pMarkers[i].iMarker == iMarker ) {
			__xuiCodeAnnotationFreeMarker(&pStore->pMarkers[i]);
			memmove(&pStore->pMarkers[i], &pStore->pMarkers[i + 1], sizeof(pStore->pMarkers[i]) * (size_t)(pStore->iMarkerCount - i - 1));
			pStore->iMarkerCount--;
			return XUI_OK;
		}
	}
	return XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiCodeAnnotationClearMarkers(xui_code_annotation_store pStore, int iLine)
{
	int i;
	int iCleared;

	if ( (pStore == NULL) || (iLine < -1) ) return XUI_ERROR_INVALID_ARGUMENT;
	iCleared = 0;
	for ( i = pStore->iMarkerCount - 1; i >= 0; i-- ) {
		if ( iLine < 0 || pStore->pMarkers[i].iLine == iLine ) {
			__xuiCodeAnnotationFreeMarker(&pStore->pMarkers[i]);
			memmove(&pStore->pMarkers[i], &pStore->pMarkers[i + 1], sizeof(pStore->pMarkers[i]) * (size_t)(pStore->iMarkerCount - i - 1));
			pStore->iMarkerCount--;
			iCleared++;
		}
	}
	return (iCleared > 0) ? XUI_OK : XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiCodeAnnotationGetMarkers(xui_code_annotation_store pStore, int iLine, xui_code_marker_t* pMarkers, int iMarkerCapacity, int* pMarkerCount)
{
	int i;
	int iCount;

	if ( (pStore == NULL) || (iLine < 0) || (pMarkerCount == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iCount = 0;
	for ( i = 0; i < pStore->iMarkerCount; i++ ) {
		if ( pStore->pMarkers[i].iLine != iLine ) continue;
		if ( pMarkers != NULL && iCount < iMarkerCapacity ) __xuiCodeAnnotationCopyMarkerOut(&pMarkers[iCount], &pStore->pMarkers[i]);
		iCount++;
	}
	*pMarkerCount = iCount;
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationSetIndicator(xui_code_annotation_store pStore, int iIndicator, int iStyle, int iStart, int iEnd, uint32_t iFlags, uintptr_t iUserData)
{
	xui_code_indicator_t* pIndicator;
	int iRet;

	if ( (pStore == NULL) || (iIndicator <= 0) || (iStart < 0) || (iEnd <= iStart) ) return XUI_ERROR_INVALID_ARGUMENT;
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

XUI_API int xuiCodeAnnotationClearIndicator(xui_code_annotation_store pStore, int iIndicator, int iStart, int iEnd)
{
	int i;
	int iCleared;

	if ( (pStore == NULL) || (iIndicator <= 0) || (iStart < 0) || (iEnd <= iStart) ) return XUI_ERROR_INVALID_ARGUMENT;
	iCleared = 0;
	for ( i = pStore->iIndicatorCount - 1; i >= 0; i-- ) {
		if ( pStore->pIndicators[i].iIndicator == iIndicator && __xuiCodeAnnotationRangeOverlaps(pStore->pIndicators[i].tRange.iStart, pStore->pIndicators[i].tRange.iEnd, iStart, iEnd) ) {
			memmove(&pStore->pIndicators[i], &pStore->pIndicators[i + 1], sizeof(pStore->pIndicators[i]) * (size_t)(pStore->iIndicatorCount - i - 1));
			pStore->iIndicatorCount--;
			iCleared++;
		}
	}
	return (iCleared > 0) ? XUI_OK : XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiCodeAnnotationClearIndicators(xui_code_annotation_store pStore, int iIndicator)
{
	int i;
	int iCleared;

	if ( (pStore == NULL) || (iIndicator < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	iCleared = 0;
	for ( i = pStore->iIndicatorCount - 1; i >= 0; i-- ) {
		if ( iIndicator == 0 || pStore->pIndicators[i].iIndicator == iIndicator ) {
			memmove(&pStore->pIndicators[i], &pStore->pIndicators[i + 1], sizeof(pStore->pIndicators[i]) * (size_t)(pStore->iIndicatorCount - i - 1));
			pStore->iIndicatorCount--;
			iCleared++;
		}
	}
	return (iCleared > 0) ? XUI_OK : XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiCodeAnnotationGetIndicatorsAt(xui_code_annotation_store pStore, int iOffset, xui_code_indicator_t* pIndicators, int iIndicatorCapacity, int* pIndicatorCount)
{
	int i;
	int iCount;

	if ( (pStore == NULL) || (iOffset < 0) || (pIndicatorCount == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iCount = 0;
	for ( i = 0; i < pStore->iIndicatorCount; i++ ) {
		if ( !__xuiCodeAnnotationRangeContains(&pStore->pIndicators[i].tRange, iOffset) ) continue;
		if ( pIndicators != NULL && iCount < iIndicatorCapacity ) __xuiCodeAnnotationCopyIndicatorOut(&pIndicators[iCount], &pStore->pIndicators[i]);
		iCount++;
	}
	*pIndicatorCount = iCount;
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationSetDiagnostics(xui_code_annotation_store pStore, const xui_code_diagnostic_t* pDiagnostics, int iDiagnosticCount)
{
	xui_code_diagnostic_t* pDiagnostic;
	char* sCode;
	char* sMessage;
	char* sSource;
	int i;
	int iRet;

	if ( (pStore == NULL) || (iDiagnosticCount < 0) || (iDiagnosticCount > 0 && pDiagnostics == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	xuiCodeAnnotationClearDiagnostics(pStore);
	iRet = __xuiCodeAnnotationReserveDiagnostics(pStore, iDiagnosticCount);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < iDiagnosticCount; i++ ) {
		if ( pDiagnostics[i].tRange.iStart < 0 || pDiagnostics[i].tRange.iEnd <= pDiagnostics[i].tRange.iStart ) return XUI_ERROR_INVALID_ARGUMENT;
		sCode = __xuiCodeAnnotationStrDup(pDiagnostics[i].sCode);
		sMessage = __xuiCodeAnnotationStrDup(pDiagnostics[i].sMessage);
		sSource = __xuiCodeAnnotationStrDup(pDiagnostics[i].sSource);
		if ( (pDiagnostics[i].sCode != NULL && sCode == NULL) || (pDiagnostics[i].sMessage != NULL && sMessage == NULL) || (pDiagnostics[i].sSource != NULL && sSource == NULL) ) {
			xrtFree(sCode);
			xrtFree(sMessage);
			xrtFree(sSource);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		pDiagnostic = &pStore->pDiagnostics[pStore->iDiagnosticCount++];
		memset(pDiagnostic, 0, sizeof(*pDiagnostic));
		*pDiagnostic = pDiagnostics[i];
		pDiagnostic->iSize = sizeof(*pDiagnostic);
		pDiagnostic->sCode = sCode;
		pDiagnostic->sMessage = sMessage;
		pDiagnostic->sSource = sSource;
	}
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationClearDiagnostics(xui_code_annotation_store pStore)
{
	int i;

	if ( pStore == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pStore->iDiagnosticCount; i++ ) __xuiCodeAnnotationFreeDiagnostic(&pStore->pDiagnostics[i]);
	pStore->iDiagnosticCount = 0;
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationGetDiagnosticCount(xui_code_annotation_store pStore)
{
	return (pStore != NULL) ? pStore->iDiagnosticCount : 0;
}

XUI_API int xuiCodeAnnotationGetDiagnostic(xui_code_annotation_store pStore, int iIndex, xui_code_diagnostic_t* pDiagnostic)
{
	if ( (pStore == NULL) || (pDiagnostic == NULL) || (iIndex < 0) || (iIndex >= pStore->iDiagnosticCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiCodeAnnotationCopyDiagnosticOut(pDiagnostic, &pStore->pDiagnostics[iIndex]);
	return XUI_OK;
}

XUI_API int xuiCodeAnnotationGetDiagnosticsAt(xui_code_annotation_store pStore, int iOffset, xui_code_diagnostic_t* pDiagnostics, int iDiagnosticCapacity, int* pDiagnosticCount)
{
	int i;
	int iCount;

	if ( (pStore == NULL) || (iOffset < 0) || (pDiagnosticCount == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iCount = 0;
	for ( i = 0; i < pStore->iDiagnosticCount; i++ ) {
		if ( !__xuiCodeAnnotationRangeContains(&pStore->pDiagnostics[i].tRange, iOffset) ) continue;
		if ( pDiagnostics != NULL && iCount < iDiagnosticCapacity ) __xuiCodeAnnotationCopyDiagnosticOut(&pDiagnostics[iCount], &pStore->pDiagnostics[i]);
		iCount++;
	}
	*pDiagnosticCount = iCount;
	return XUI_OK;
}

static void __xuiCodeAnnotationTrackRange(xui_code_range_t* pRange, int iStartOffset, int iEndOffset, int iNewEndOffset)
{
	int iDelta;

	iDelta = iNewEndOffset - iEndOffset;
	if ( pRange->iEnd <= iStartOffset ) return;
	if ( pRange->iStart >= iEndOffset ) {
		pRange->iStart += iDelta;
		pRange->iEnd += iDelta;
		return;
	}
	if ( pRange->iStart > iStartOffset ) pRange->iStart = iStartOffset;
	pRange->iEnd = iNewEndOffset;
	if ( pRange->iEnd <= pRange->iStart ) pRange->iEnd = pRange->iStart + 1;
}

XUI_API int xuiCodeAnnotationTrackEdit(xui_code_annotation_store pStore, int iStartOffset, int iEndOffset, int iNewEndOffset, int iStartLine, int iEndLine, int iNewEndLine)
{
	int i;
	int iLineDelta;

	if ( (pStore == NULL) || (iStartOffset < 0) || (iEndOffset < iStartOffset) || (iNewEndOffset < iStartOffset) || (iStartLine < 0) || (iEndLine < iStartLine) || (iNewEndLine < iStartLine) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iLineDelta = iNewEndLine - iEndLine;
	for ( i = 0; i < pStore->iMarkerCount; i++ ) {
		if ( pStore->pMarkers[i].iLine > iEndLine ) pStore->pMarkers[i].iLine += iLineDelta;
		else if ( pStore->pMarkers[i].iLine >= iStartLine ) pStore->pMarkers[i].iLine = iStartLine;
	}
	for ( i = 0; i < pStore->iIndicatorCount; i++ ) {
		__xuiCodeAnnotationTrackRange(&pStore->pIndicators[i].tRange, iStartOffset, iEndOffset, iNewEndOffset);
	}
	for ( i = 0; i < pStore->iDiagnosticCount; i++ ) {
		__xuiCodeAnnotationTrackRange(&pStore->pDiagnostics[i].tRange, iStartOffset, iEndOffset, iNewEndOffset);
	}
	return XUI_OK;
}
