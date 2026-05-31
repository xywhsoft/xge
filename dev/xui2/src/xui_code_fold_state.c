#include "../xui.h"

#include <string.h>

struct xui_code_fold_state_t {
	xui_code_fold_range_t* pRanges;
	int iRangeCount;
	int iRangeCapacity;
};

static int __xuiCodeFoldStateReserve(xui_code_fold_state pState, int iCapacity)
{
	xui_code_fold_range_t* pNew;

	if ( iCapacity <= pState->iRangeCapacity ) return XUI_OK;
	if ( iCapacity < pState->iRangeCapacity * 2 ) iCapacity = pState->iRangeCapacity * 2;
	if ( iCapacity < 16 ) iCapacity = 16;
	pNew = (xui_code_fold_range_t*)xrtRealloc(pState->pRanges, sizeof(*pNew) * (size_t)iCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pState->pRanges = pNew;
	pState->iRangeCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiCodeFoldStateRangeValid(const xui_code_fold_range_t* pRange)
{
	return pRange != NULL && pRange->iStartLine >= 0 && pRange->iEndLine > pRange->iStartLine;
}

static void __xuiCodeFoldStateCopyOut(xui_code_fold_range_t* pOut, const xui_code_fold_range_t* pRange)
{
	*pOut = *pRange;
	pOut->iSize = sizeof(*pOut);
}

static int __xuiCodeFoldStateFindHeader(xui_code_fold_state pState, int iLine)
{
	int i;

	for ( i = 0; i < pState->iRangeCount; i++ ) {
		if ( pState->pRanges[i].iStartLine == iLine ) return i;
	}
	for ( i = 0; i < pState->iRangeCount; i++ ) {
		if ( iLine > pState->pRanges[i].iStartLine && iLine <= pState->pRanges[i].iEndLine ) return i;
	}
	return -1;
}

XUI_API int xuiCodeFoldStateCreate(xui_code_fold_state* ppState)
{
	xui_code_fold_state pState;

	if ( ppState == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppState = NULL;
	pState = (xui_code_fold_state)xrtMalloc(sizeof(*pState));
	if ( pState == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pState, 0, sizeof(*pState));
	*ppState = pState;
	return XUI_OK;
}

XUI_API void xuiCodeFoldStateDestroy(xui_code_fold_state pState)
{
	if ( pState == NULL ) return;
	xrtFree(pState->pRanges);
	xrtFree(pState);
}

XUI_API void xuiCodeFoldStateClear(xui_code_fold_state pState)
{
	if ( pState == NULL ) return;
	pState->iRangeCount = 0;
}

XUI_API int xuiCodeFoldStateSetRanges(xui_code_fold_state pState, const xui_code_fold_range_t* pRanges, int iRangeCount)
{
	int i;
	int iRet;

	if ( (pState == NULL) || (iRangeCount < 0) || (iRangeCount > 0 && pRanges == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < iRangeCount; i++ ) {
		if ( !__xuiCodeFoldStateRangeValid(&pRanges[i]) ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiCodeFoldStateReserve(pState, iRangeCount);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < iRangeCount; i++ ) {
		pState->pRanges[i] = pRanges[i];
		pState->pRanges[i].iSize = sizeof(pState->pRanges[i]);
		pState->pRanges[i].iFlags |= XUI_CODE_FOLD_HEADER;
	}
	pState->iRangeCount = iRangeCount;
	return XUI_OK;
}

XUI_API int xuiCodeFoldStateBuildFromProvider(xui_code_fold_state pState, xui_code_document pDocument, xui_code_fold_proc onFold, void* pUser)
{
	xui_code_fold_range_t* pRanges;
	int iCount;
	int iRet;

	if ( (pState == NULL) || (pDocument == NULL) || (onFold == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = onFold(pDocument, NULL, 0, &iCount, pUser);
	if ( iRet != XUI_OK ) return iRet;
	pRanges = NULL;
	if ( iCount > 0 ) {
		pRanges = (xui_code_fold_range_t*)xrtMalloc(sizeof(*pRanges) * (size_t)iCount);
		if ( pRanges == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		iRet = onFold(pDocument, pRanges, iCount, &iCount, pUser);
		if ( iRet != XUI_OK ) {
			xrtFree(pRanges);
			return iRet;
		}
	}
	iRet = xuiCodeFoldStateSetRanges(pState, pRanges, iCount);
	xrtFree(pRanges);
	return iRet;
}

XUI_API int xuiCodeFoldStateGetCount(xui_code_fold_state pState)
{
	return (pState != NULL) ? pState->iRangeCount : 0;
}

XUI_API int xuiCodeFoldStateGetRange(xui_code_fold_state pState, int iIndex, xui_code_fold_range_t* pRange)
{
	if ( (pState == NULL) || (pRange == NULL) || (iIndex < 0) || (iIndex >= pState->iRangeCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiCodeFoldStateCopyOut(pRange, &pState->pRanges[iIndex]);
	return XUI_OK;
}

XUI_API int xuiCodeFoldStateGetRanges(xui_code_fold_state pState, xui_code_fold_range_t* pRanges, int iRangeCapacity, int* pRangeCount)
{
	int i;

	if ( (pState == NULL) || (pRangeCount == NULL) || (iRangeCapacity < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pState->iRangeCount; i++ ) {
		if ( pRanges != NULL && i < iRangeCapacity ) __xuiCodeFoldStateCopyOut(&pRanges[i], &pState->pRanges[i]);
	}
	*pRangeCount = pState->iRangeCount;
	return XUI_OK;
}

XUI_API int xuiCodeFoldStateToggleLine(xui_code_fold_state pState, int iLine)
{
	int iIndex;

	if ( (pState == NULL) || (iLine < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiCodeFoldStateFindHeader(pState, iLine);
	if ( iIndex < 0 ) return XUI_ERROR_UNSUPPORTED;
	pState->pRanges[iIndex].iFlags ^= XUI_CODE_FOLD_COLLAPSED;
	return XUI_OK;
}

XUI_API int xuiCodeFoldStateFoldAll(xui_code_fold_state pState)
{
	int i;

	if ( pState == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pState->iRangeCount; i++ ) pState->pRanges[i].iFlags |= XUI_CODE_FOLD_COLLAPSED;
	return XUI_OK;
}

XUI_API int xuiCodeFoldStateUnfoldAll(xui_code_fold_state pState)
{
	int i;

	if ( pState == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pState->iRangeCount; i++ ) pState->pRanges[i].iFlags &= ~XUI_CODE_FOLD_COLLAPSED;
	return XUI_OK;
}

XUI_API int xuiCodeFoldStateIsLineVisible(xui_code_fold_state pState, int iLine, int* pVisible)
{
	int i;

	if ( (pState == NULL) || (iLine < 0) || (pVisible == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	*pVisible = 1;
	for ( i = 0; i < pState->iRangeCount; i++ ) {
		if ( (pState->pRanges[i].iFlags & XUI_CODE_FOLD_COLLAPSED) != 0 &&
		     iLine > pState->pRanges[i].iStartLine && iLine <= pState->pRanges[i].iEndLine ) {
			*pVisible = 0;
			return XUI_OK;
		}
	}
	return XUI_OK;
}

XUI_API int xuiCodeFoldStateBuildVisibleLines(xui_code_fold_state pState, int iLineCount, int* pVisibleLines, int iVisibleCapacity, int* pVisibleCount)
{
	if ( pState == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiCodeFoldBuildVisibleLines(iLineCount, pState->pRanges, pState->iRangeCount, pVisibleLines, iVisibleCapacity, pVisibleCount);
}
