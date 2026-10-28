#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui_internal.h"

#include <limits.h>
#include <math.h>
#include <string.h>

static int __xuiCodeLayoutTabColumns(const xui_code_layout_desc_t* pDesc)
{
	return (pDesc != NULL && pDesc->iTabColumns > 0) ? pDesc->iTabColumns : 4;
}

static float __xuiCodeLayoutMetric(float fValue, float fFallback)
{
	return (isfinite((double)fValue) && fValue > 0.0f) ? fValue : fFallback;
}

static int __xuiCodeLayoutGeometryValid(const xui_code_layout_desc_t* pDesc)
{
	if ( pDesc == NULL ) return 0;
	return isfinite((double)pDesc->tViewportRect.fX) &&
		isfinite((double)pDesc->tViewportRect.fY) &&
		isfinite((double)pDesc->tViewportRect.fW) &&
		isfinite((double)pDesc->tViewportRect.fH) &&
		isfinite((double)pDesc->fScrollX) &&
		isfinite((double)pDesc->fScrollY) &&
		isfinite((double)pDesc->fMarginWidth);
}

static int __xuiCodeLayoutNextVisualColumn(int iVisualColumn, char c, int iTabColumns)
{
	int iStep;

	if ( iVisualColumn >= INT_MAX ) return INT_MAX;
	if ( c != '\t' ) return iVisualColumn + 1;
	iStep = iTabColumns - (iVisualColumn % iTabColumns);
	if ( iStep <= 0 ) iStep = iTabColumns;
	return (iVisualColumn > INT_MAX - iStep) ? INT_MAX : iVisualColumn + iStep;
}

static int __xuiCodeLayoutUnicodeRead(void* pUser, int iOffset, unsigned char* pByte)
{
	char c;

	if ( pUser == NULL || pByte == NULL ) return 0;
	c = '\0';
	if ( xuiCodeDocumentGetByte((xui_code_document)pUser, iOffset, &c) != XUI_OK ) return 0;
	*pByte = (unsigned char)c;
	return 1;
}

static int __xuiCodeLayoutNextOffset(xui_code_document pDocument, int iOffset, int iEnd)
{
	int iNext;

	iNext = xuiInternalTextGraphemeNextRead(__xuiCodeLayoutUnicodeRead,
		pDocument, xuiCodeDocumentGetLength(pDocument), iOffset);
	if ( iNext <= iOffset ) iNext = iOffset + 1;
	if ( iNext > iEnd ) iNext = iEnd;
	return iNext;
}

static int __xuiCodeLayoutLineVisualColumns(xui_code_document pDocument, int iLine, int iTabColumns)
{
	int iStart;
	int iEnd;
	int i;
	int iColumn;
	char ch;

	if ( xuiCodeDocumentGetLineRange(pDocument, iLine, &iStart, &iEnd) != XUI_OK ) return 0;
	iColumn = 0;
	for ( i = iStart; i < iEnd; i = __xuiCodeLayoutNextOffset(pDocument, i, iEnd) ) {
		if ( xuiCodeDocumentGetByte(pDocument, i, &ch) != XUI_OK ) break;
		iColumn = __xuiCodeLayoutNextVisualColumn(iColumn, ch, iTabColumns);
	}
	return iColumn;
}

static float __xuiCodeLayoutLineWidth(xui_code_document pDocument, int iLine, float fCharWidth, int iTabColumns)
{
	return (float)__xuiCodeLayoutLineVisualColumns(pDocument, iLine, iTabColumns) * fCharWidth;
}

static int __xuiCodeLayoutColumnToVisual(xui_code_document pDocument, int iLine, int iColumn, int iTabColumns)
{
	int iStart;
	int iEnd;
	int i;
	int iLogicalColumn;
	int iVisual;
	char ch;

	if ( xuiCodeDocumentGetLineRange(pDocument, iLine, &iStart, &iEnd) != XUI_OK ) return 0;
	if ( iColumn < 0 ) iColumn = 0;
	iVisual = 0;
	iLogicalColumn = 0;
	for ( i = iStart; i < iEnd && iLogicalColumn < iColumn;
	      i = __xuiCodeLayoutNextOffset(pDocument, i, iEnd), iLogicalColumn++ ) {
		if ( xuiCodeDocumentGetByte(pDocument, i, &ch) != XUI_OK ) break;
		iVisual = __xuiCodeLayoutNextVisualColumn(iVisual, ch, iTabColumns);
	}
	return iVisual;
}

static int __xuiCodeLayoutVisualToColumn(xui_code_document pDocument, int iLine, int iVisualColumn, int iTabColumns)
{
	int iStart;
	int iEnd;
	int i;
	int iColumn;
	int iVisual;
	int iNext;
	char ch;

	if ( xuiCodeDocumentGetLineRange(pDocument, iLine, &iStart, &iEnd) != XUI_OK ) return 0;
	if ( iVisualColumn <= 0 ) return 0;
	iVisual = 0;
	iColumn = 0;
	for ( i = iStart; i < iEnd; i = __xuiCodeLayoutNextOffset(pDocument, i, iEnd) ) {
		if ( xuiCodeDocumentGetByte(pDocument, i, &ch) != XUI_OK ) break;
		iNext = __xuiCodeLayoutNextVisualColumn(iVisual, ch, iTabColumns);
		if ( iVisualColumn <= iNext ) return iColumn + ((iVisualColumn - iVisual > iNext - iVisualColumn) ? 1 : 0);
		iVisual = iNext;
		iColumn++;
	}
	return iColumn;
}

static int __xuiCodeLayoutVisibleLines(const xui_code_layout_desc_t* pDesc, int** ppVisible, int* pCount)
{
	int iLineCount;
	int* pVisible;
	int iRet;

	if ( (pDesc == NULL) || (pDesc->pDocument == NULL) || (ppVisible == NULL) || (pCount == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pDesc->iFoldRangeCount < 0 ||
	     (pDesc->iFoldRangeCount > 0 && pDesc->pFoldRanges == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iLineCount = xuiCodeDocumentGetLineCount(pDesc->pDocument);
	pVisible = (int*)xrtMalloc(sizeof(*pVisible) * (size_t)((iLineCount > 0) ? iLineCount : 1));
	if ( pVisible == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = xuiCodeFoldBuildVisibleLines(iLineCount, pDesc->pFoldRanges, pDesc->iFoldRangeCount, pVisible, iLineCount, pCount);
	if ( iRet != XUI_OK ) {
		xrtFree(pVisible);
		return iRet;
	}
	*ppVisible = pVisible;
	return XUI_OK;
}

static int __xuiCodeLayoutFindVisibleIndex(const int* pVisible, int iCount, int iLine)
{
	int iLow;
	int iHigh;
	int iMid;

	if ( pVisible == NULL || iCount <= 0 ) return -1;
	iLow = 0;
	iHigh = iCount - 1;
	while ( iLow <= iHigh ) {
		iMid = iLow + (iHigh - iLow) / 2;
		if ( pVisible[iMid] < iLine ) {
			iLow = iMid + 1;
		} else if ( pVisible[iMid] > iLine ) {
			iHigh = iMid - 1;
		} else {
			return iMid;
		}
	}
	return -1;
}

typedef struct __xui_code_layout_segment_t {
	int iStartOffset;
	int iEndOffset;
	int iStartColumn;
	int iEndColumn;
	int iStartVisual;
	int iEndVisual;
} __xui_code_layout_segment_t;

static int __xuiCodeLayoutWrapColumns(const xui_code_layout_desc_t* pDesc, const xui_rect_t* pTextRect)
{
	float fCharWidth = __xuiCodeLayoutMetric(pDesc->fCharWidth, 8.0f);
	int iColumns;

	if ( !pDesc->bWordWrap ) return 0;
	if ( !isfinite((double)pTextRect->fW) ) return 1;
	if ( pTextRect->fW / fCharWidth >= (float)INT_MAX ) return INT_MAX;
	iColumns = (int)(pTextRect->fW / fCharWidth);
	return (iColumns > 0) ? iColumns : 1;
}

static int __xuiCodeLayoutBreakAfter(char ch)
{
	return ch == ' ' || ch == '\t' || ch == '-' || ch == '/' || ch == '\\' ||
		ch == ',' || ch == ';' || ch == ':' || ch == '.';
}

static int __xuiCodeLayoutNextSegment(xui_code_document pDocument,
	int iLineEnd, int iTabColumns, int iWrapColumns, int iStart,
	int iStartColumn, int iStartVisual, __xui_code_layout_segment_t* pSegment)
{
	int i;
	int iNextOffset;
	int iVisual;
	int iNextVisual;
	int iLastBreak;
	int iColumn;
	int iLastBreakColumn;
	int iLastBreakVisual;
	int bHasContent;
	char ch;

	if ( pDocument == NULL || pSegment == NULL || iStart < 0 ||
	     iLineEnd < iStart || iWrapColumns <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	i = iStart;
	iVisual = iStartVisual;
	iColumn = iStartColumn;
	iLastBreak = -1;
	iLastBreakColumn = iStartColumn;
	iLastBreakVisual = iStartVisual;
	bHasContent = 0;
	while ( i < iLineEnd ) {
		if ( xuiCodeDocumentGetByte(pDocument, i, &ch) != XUI_OK ) break;
		iNextOffset = __xuiCodeLayoutNextOffset(pDocument, i, iLineEnd);
		iNextVisual = __xuiCodeLayoutNextVisualColumn(iVisual, ch, iTabColumns);
		if ( iNextVisual - iStartVisual > iWrapColumns ) break;
		iVisual = iNextVisual;
		i = iNextOffset;
		iColumn++;
		if ( __xuiCodeLayoutBreakAfter(ch) && bHasContent ) {
			iLastBreak = i;
			iLastBreakColumn = iColumn;
			iLastBreakVisual = iVisual;
		}
		if ( ch != ' ' && ch != '\t' ) bHasContent = 1;
	}
	if ( i < iLineEnd && iLastBreak > iStart ) {
		i = iLastBreak;
		iColumn = iLastBreakColumn;
		iVisual = iLastBreakVisual;
	} else if ( i == iStart && i < iLineEnd ) {
		if ( xuiCodeDocumentGetByte(pDocument, i, &ch) == XUI_OK ) {
			iVisual = __xuiCodeLayoutNextVisualColumn(iStartVisual, ch, iTabColumns);
		}
		i = __xuiCodeLayoutNextOffset(pDocument, i, iLineEnd);
		iColumn++;
	}
	pSegment->iStartOffset = iStart;
	pSegment->iEndOffset = i;
	pSegment->iStartColumn = iStartColumn;
	pSegment->iEndColumn = iColumn;
	pSegment->iStartVisual = iStartVisual;
	pSegment->iEndVisual = iVisual;
	return (i > iStart) ? XUI_OK : XUI_ERROR_RESOURCE_FAILED;
}

static int __xuiCodeLayoutFirstSegment(xui_code_document pDocument, int iLine,
	int iTabColumns, int iWrapColumns, __xui_code_layout_segment_t* pSegment,
	int* pLineEnd)
{
	int iLineStart;
	int iLineEnd;
	int iOffset;
	int iColumn;
	int iVisual;
	char ch;

	if ( pDocument == NULL || pSegment == NULL ||
	     xuiCodeDocumentGetLineRange(pDocument, iLine, &iLineStart, &iLineEnd) != XUI_OK ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pLineEnd != NULL ) *pLineEnd = iLineEnd;
	if ( iWrapColumns > 0 && iLineStart < iLineEnd ) {
		return __xuiCodeLayoutNextSegment(pDocument, iLineEnd, iTabColumns,
			iWrapColumns, iLineStart, 0, 0, pSegment);
	}
	iOffset = iLineStart;
	iColumn = 0;
	iVisual = 0;
	while ( iOffset < iLineEnd ) {
		if ( xuiCodeDocumentGetByte(pDocument, iOffset, &ch) != XUI_OK ) {
			return XUI_ERROR_RESOURCE_FAILED;
		}
		iVisual = __xuiCodeLayoutNextVisualColumn(iVisual, ch, iTabColumns);
		iOffset = __xuiCodeLayoutNextOffset(pDocument, iOffset, iLineEnd);
		iColumn++;
	}
	pSegment->iStartOffset = iLineStart;
	pSegment->iEndOffset = iLineEnd;
	pSegment->iStartColumn = 0;
	pSegment->iEndColumn = iColumn;
	pSegment->iStartVisual = 0;
	pSegment->iEndVisual = iVisual;
	return XUI_OK;
}

static int __xuiCodeLayoutAdvanceSegment(xui_code_document pDocument,
	int iLineEnd, int iTabColumns, int iWrapColumns,
	const __xui_code_layout_segment_t* pCurrent,
	__xui_code_layout_segment_t* pNext)
{
	if ( pDocument == NULL || pCurrent == NULL || pNext == NULL ||
	     iWrapColumns <= 0 || pCurrent->iEndOffset >= iLineEnd ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return __xuiCodeLayoutNextSegment(pDocument, iLineEnd, iTabColumns,
		iWrapColumns, pCurrent->iEndOffset, pCurrent->iEndColumn,
		pCurrent->iEndVisual, pNext);
}

static int __xuiCodeLayoutGetSegment(xui_code_document pDocument, int iLine,
	int iTabColumns, int iWrapColumns, int iWanted,
	__xui_code_layout_segment_t* pSegment, int* pSegmentCount)
{
	int iLineEnd;
	int iSegment;
	__xui_code_layout_segment_t tCurrent;
	__xui_code_layout_segment_t tNext;

	if ( __xuiCodeLayoutFirstSegment(pDocument, iLine, iTabColumns,
	     iWrapColumns, &tCurrent, &iLineEnd) != XUI_OK ) return XUI_ERROR_INVALID_ARGUMENT;
	iSegment = 0;
	for ( ;; ) {
		if ( pSegment != NULL && iSegment == iWanted ) *pSegment = tCurrent;
		iSegment++;
		if ( iWrapColumns <= 0 || tCurrent.iEndOffset >= iLineEnd ) break;
		if ( __xuiCodeLayoutAdvanceSegment(pDocument, iLineEnd, iTabColumns,
		     iWrapColumns, &tCurrent, &tNext) != XUI_OK ) {
			return XUI_ERROR_RESOURCE_FAILED;
		}
		tCurrent = tNext;
	}
	if ( pSegmentCount != NULL ) *pSegmentCount = iSegment;
	return (iWanted < 0 || iWanted < iSegment) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiCodeLayoutVisualRow(const xui_code_layout_desc_t* pDesc,
	const int* pVisible, int iVisibleCount, int iWrapColumns,
	int iWantedRow, int* pLine, int* pSegmentIndex,
	__xui_code_layout_segment_t* pSegment, int* pResolvedRow, int* pTotalRows)
{
	int i;
	int iRows;
	int iLineEnd;
	int iSegment;
	int iLastLine;
	int iLastSegment;
	int bFound;
	__xui_code_layout_segment_t tCurrent;
	__xui_code_layout_segment_t tNext;
	__xui_code_layout_segment_t tLast;

	iRows = 0;
	iLastLine = -1;
	iLastSegment = -1;
	bFound = 0;
	memset(&tLast, 0, sizeof(tLast));
	for ( i = 0; i < iVisibleCount; i++ ) {
		if ( __xuiCodeLayoutFirstSegment(pDesc->pDocument, pVisible[i],
		     __xuiCodeLayoutTabColumns(pDesc), iWrapColumns,
		     &tCurrent, &iLineEnd) != XUI_OK ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		iSegment = 0;
		for ( ;; ) {
			iLastLine = pVisible[i];
			iLastSegment = iSegment;
			tLast = tCurrent;
			if ( iWantedRow == iRows ) {
				if ( pLine != NULL ) *pLine = pVisible[i];
				if ( pSegmentIndex != NULL ) *pSegmentIndex = iSegment;
				if ( pSegment != NULL ) *pSegment = tCurrent;
				if ( pResolvedRow != NULL ) *pResolvedRow = iRows;
				bFound = 1;
			}
			if ( iRows == INT_MAX ) return XUI_ERROR_RESOURCE_FAILED;
			iRows++;
			iSegment++;
			if ( iWrapColumns <= 0 || tCurrent.iEndOffset >= iLineEnd ) break;
			if ( __xuiCodeLayoutAdvanceSegment(pDesc->pDocument, iLineEnd,
			     __xuiCodeLayoutTabColumns(pDesc), iWrapColumns,
			     &tCurrent, &tNext) != XUI_OK ) {
				return XUI_ERROR_RESOURCE_FAILED;
			}
			tCurrent = tNext;
		}
	}
	if ( iWantedRow >= 0 && !bFound && iRows > 0 ) {
		if ( pLine != NULL ) *pLine = iLastLine;
		if ( pSegmentIndex != NULL ) *pSegmentIndex = iLastSegment;
		if ( pSegment != NULL ) *pSegment = tLast;
		if ( pResolvedRow != NULL ) *pResolvedRow = iRows - 1;
		bFound = 1;
	}
	if ( pTotalRows != NULL ) *pTotalRows = iRows;
	return (iWantedRow < 0 || bFound) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API int xuiCodeLayoutBuildVisibleLines(const xui_code_layout_desc_t* pDesc, xui_code_layout_line_t* pLines, int iLineCapacity, int* pLineCount, xui_vec2_t* pContentSize, xui_rect_t* pTextRect)
{
	int* pVisible;
	int iVisibleCount;
	int i;
	int iOut;
	int iRet;
	float fLineHeight;
	float fCharWidth;
	float fContentWidth;
	float fY;
	int iTabColumns;
	int iWrapColumns;
	int iVisualRow;
	int iTotalRows;
	int iLineEnd;
	__xui_code_layout_segment_t tSegment;
	__xui_code_layout_segment_t tNextSegment;
	xui_rect_t tTextRect;

	if ( (pDesc == NULL) || (pDesc->pDocument == NULL) || (pLineCount == NULL) ||
	     !__xuiCodeLayoutGeometryValid(pDesc) ||
	     (iLineCapacity < 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pLineCount = 0;
	fLineHeight = __xuiCodeLayoutMetric(pDesc->fLineHeight, 18.0f);
	fCharWidth = __xuiCodeLayoutMetric(pDesc->fCharWidth, 8.0f);
	iTabColumns = __xuiCodeLayoutTabColumns(pDesc);
	tTextRect = pDesc->tViewportRect;
	tTextRect.fX += pDesc->fMarginWidth;
	tTextRect.fW -= pDesc->fMarginWidth;
	if ( tTextRect.fW < 0.0f ) tTextRect.fW = 0.0f;
	iWrapColumns = __xuiCodeLayoutWrapColumns(pDesc, &tTextRect);
	iRet = __xuiCodeLayoutVisibleLines(pDesc, &pVisible, &iVisibleCount);
	if ( iRet != XUI_OK ) return iRet;
	fContentWidth = 0.0f;
	if ( !pDesc->bWordWrap ) {
		for ( i = 0; i < iVisibleCount; i++ ) {
			float fWidth = __xuiCodeLayoutLineWidth(pDesc->pDocument, pVisible[i], fCharWidth, iTabColumns);
			if ( fWidth > fContentWidth ) fContentWidth = fWidth;
		}
	}
	iOut = 0;
	iVisualRow = 0;
	for ( i = 0; i < iVisibleCount; i++ ) {
		iRet = __xuiCodeLayoutFirstSegment(pDesc->pDocument, pVisible[i],
			iTabColumns, iWrapColumns, &tSegment, &iLineEnd);
		if ( iRet != XUI_OK ) {
			xrtFree(pVisible);
			return iRet;
		}
		for ( ;; ) {
			fY = pDesc->tViewportRect.fY + (float)iVisualRow * fLineHeight - pDesc->fScrollY;
			if ( fY + fLineHeight >= pDesc->tViewportRect.fY &&
			     fY <= pDesc->tViewportRect.fY + pDesc->tViewportRect.fH ) {
				if ( pLines != NULL && iOut < iLineCapacity ) {
					memset(&pLines[iOut], 0, sizeof(pLines[iOut]));
					pLines[iOut].iSize = sizeof(pLines[iOut]);
					pLines[iOut].iLine = pVisible[i];
					pLines[iOut].iVisibleIndex = iVisualRow;
					pLines[iOut].iStartOffset = tSegment.iStartOffset;
					pLines[iOut].iEndOffset = tSegment.iEndOffset;
					pLines[iOut].tRect = (xui_rect_t){
						tTextRect.fX - (pDesc->bWordWrap ? 0.0f : pDesc->fScrollX),
						fY,
						pDesc->bWordWrap ? tTextRect.fW : ((fContentWidth > tTextRect.fW) ? fContentWidth : tTextRect.fW),
						fLineHeight
					};
				}
				iOut++;
			}
			if ( iVisualRow == INT_MAX ) {
				xrtFree(pVisible);
				return XUI_ERROR_RESOURCE_FAILED;
			}
			iVisualRow++;
			if ( iWrapColumns <= 0 || tSegment.iEndOffset >= iLineEnd ) break;
			iRet = __xuiCodeLayoutAdvanceSegment(pDesc->pDocument, iLineEnd,
				iTabColumns, iWrapColumns, &tSegment, &tNextSegment);
			if ( iRet != XUI_OK ) {
				xrtFree(pVisible);
				return iRet;
			}
			tSegment = tNextSegment;
		}
	}
	*pLineCount = iOut;
	iTotalRows = iVisualRow;
	if ( pContentSize != NULL ) {
		pContentSize->fX = pDesc->bWordWrap ? tTextRect.fW : fContentWidth;
		pContentSize->fY = (float)iTotalRows * fLineHeight;
	}
	if ( pTextRect != NULL ) *pTextRect = tTextRect;
	xrtFree(pVisible);
	return XUI_OK;
}

XUI_API int xuiCodeLayoutHitTest(const xui_code_layout_desc_t* pDesc, float fX, float fY, xui_code_hit_t* pHit)
{
	int* pVisible;
	int iVisibleCount;
	int iIndex;
	int iLine = 0;
	int iColumn;
	int iOffset;
	int iRet;
	float fLineHeight;
	float fCharWidth;
	int iTabColumns;
	int iWrapColumns;
	int iSegmentVisual;
	double fSegmentVisual;
	float fRow;
	__xui_code_layout_segment_t tSegment;
	xui_rect_t tTextRect;

	if ( (pDesc == NULL) || (pDesc->pDocument == NULL) || (pHit == NULL) ||
	     !__xuiCodeLayoutGeometryValid(pDesc) ||
	     !isfinite((double)fX) || !isfinite((double)fY) ||
	     !isfinite((double)pDesc->fScrollX) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pHit, 0, sizeof(*pHit));
	memset(&tSegment, 0, sizeof(tSegment));
	pHit->iSize = sizeof(*pHit);
	fLineHeight = __xuiCodeLayoutMetric(pDesc->fLineHeight, 18.0f);
	fCharWidth = __xuiCodeLayoutMetric(pDesc->fCharWidth, 8.0f);
	iTabColumns = __xuiCodeLayoutTabColumns(pDesc);
	iRet = __xuiCodeLayoutVisibleLines(pDesc, &pVisible, &iVisibleCount);
	if ( iRet != XUI_OK ) return iRet;
	tTextRect = pDesc->tViewportRect;
	tTextRect.fX += pDesc->fMarginWidth;
	tTextRect.fW -= pDesc->fMarginWidth;
	if ( tTextRect.fW < 0.0f ) tTextRect.fW = 0.0f;
	iWrapColumns = __xuiCodeLayoutWrapColumns(pDesc, &tTextRect);
	fRow = (fY - pDesc->tViewportRect.fY + pDesc->fScrollY) / fLineHeight;
	if ( fRow <= 0.0f ) iIndex = 0;
	else if ( fRow >= (float)INT_MAX ) iIndex = INT_MAX;
	else iIndex = (int)fRow;
	iRet = __xuiCodeLayoutVisualRow(pDesc, pVisible, iVisibleCount, iWrapColumns,
		iIndex, &iLine, NULL, &tSegment, &iIndex, NULL);
	if ( iRet != XUI_OK ) {
		xrtFree(pVisible);
		return iRet;
	}
	fSegmentVisual = ((double)fX - (double)tTextRect.fX +
		(pDesc->bWordWrap ? 0.0 : (double)pDesc->fScrollX)) /
		(double)fCharWidth + 0.5;
	if ( fSegmentVisual <= 0.0 ) iSegmentVisual = 0;
	else if ( fSegmentVisual >= (double)(INT_MAX - tSegment.iStartVisual) ) {
		iSegmentVisual = INT_MAX - tSegment.iStartVisual;
	} else {
		iSegmentVisual = (int)fSegmentVisual;
	}
	iColumn = __xuiCodeLayoutVisualToColumn(pDesc->pDocument, iLine,
		tSegment.iStartVisual + iSegmentVisual, iTabColumns);
	if ( iColumn < tSegment.iStartColumn ) iColumn = tSegment.iStartColumn;
	if ( iColumn > tSegment.iEndColumn ) iColumn = tSegment.iEndColumn;
	(void)xuiCodeDocumentLineColumnToOffset(pDesc->pDocument, iLine, iColumn, &iOffset);
	pHit->iPart = (fX < tTextRect.fX) ? 1 : 2;
	pHit->iLine = iLine;
	pHit->iColumn = iColumn;
	pHit->iOffset = iOffset;
	pHit->tLineRect = (xui_rect_t){tTextRect.fX - (pDesc->bWordWrap ? 0.0f : pDesc->fScrollX), pDesc->tViewportRect.fY + (float)iIndex * fLineHeight - pDesc->fScrollY, tTextRect.fW, fLineHeight};
	xrtFree(pVisible);
	return XUI_OK;
}

XUI_API int xuiCodeLayoutGetCaretRect(const xui_code_layout_desc_t* pDesc, int iLine, int iColumn, xui_rect_t* pRect)
{
	int* pVisible;
	int iVisibleCount;
	int iIndex;
	int iRet;
	float fLineHeight;
	float fCharWidth;
	int iVisualColumn;
	int iTabColumns;
	int iWrapColumns;
	int iSegmentCount;
	int iLineEnd;
	int iTargetOffset;
	int iVisualRow;
	int i;
	__xui_code_layout_segment_t tSegment;
	__xui_code_layout_segment_t tNextSegment;
	xui_rect_t tTextRect;

	if ( (pDesc == NULL) || (pDesc->pDocument == NULL) || (pRect == NULL) ||
	     !__xuiCodeLayoutGeometryValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiCodeLayoutVisibleLines(pDesc, &pVisible, &iVisibleCount);
	if ( iRet != XUI_OK ) return iRet;
	iIndex = __xuiCodeLayoutFindVisibleIndex(pVisible, iVisibleCount, iLine);
	if ( iIndex < 0 ) {
		xrtFree(pVisible);
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iColumn < 0 ) iColumn = 0;
	fLineHeight = __xuiCodeLayoutMetric(pDesc->fLineHeight, 18.0f);
	fCharWidth = __xuiCodeLayoutMetric(pDesc->fCharWidth, 8.0f);
	iTabColumns = __xuiCodeLayoutTabColumns(pDesc);
	iVisualColumn = __xuiCodeLayoutColumnToVisual(pDesc->pDocument, iLine, iColumn, iTabColumns);
	tTextRect = pDesc->tViewportRect;
	tTextRect.fX += pDesc->fMarginWidth;
	tTextRect.fW -= pDesc->fMarginWidth;
	if ( tTextRect.fW < 0.0f ) tTextRect.fW = 0.0f;
	iWrapColumns = __xuiCodeLayoutWrapColumns(pDesc, &tTextRect);
	iRet = xuiCodeDocumentLineColumnToOffset(pDesc->pDocument, iLine, iColumn,
		&iTargetOffset);
	if ( iRet != XUI_OK ) {
		xrtFree(pVisible);
		return iRet;
	}
	iVisualRow = 0;
	for ( i = 0; i < iIndex; i++ ) {
		iRet = __xuiCodeLayoutGetSegment(pDesc->pDocument, pVisible[i],
			iTabColumns, iWrapColumns, -1, NULL, &iSegmentCount);
		if ( iRet != XUI_OK || iVisualRow > INT_MAX - iSegmentCount ) {
			xrtFree(pVisible);
			return (iRet != XUI_OK) ? iRet : XUI_ERROR_RESOURCE_FAILED;
		}
		iVisualRow += iSegmentCount;
	}
	iRet = __xuiCodeLayoutFirstSegment(pDesc->pDocument, iLine, iTabColumns,
		iWrapColumns, &tSegment, &iLineEnd);
	if ( iRet != XUI_OK ) {
		xrtFree(pVisible);
		return iRet;
	}
	while ( iTargetOffset >= tSegment.iEndOffset &&
	        tSegment.iEndOffset < iLineEnd && iWrapColumns > 0 ) {
		iRet = __xuiCodeLayoutAdvanceSegment(pDesc->pDocument, iLineEnd,
			iTabColumns, iWrapColumns, &tSegment, &tNextSegment);
		if ( iRet != XUI_OK ) {
			xrtFree(pVisible);
			return iRet;
		}
		tSegment = tNextSegment;
		iVisualRow++;
	}
	xrtFree(pVisible);
	*pRect = (xui_rect_t){
		tTextRect.fX + (float)(iVisualColumn - tSegment.iStartVisual) * fCharWidth -
			(pDesc->bWordWrap ? 0.0f : pDesc->fScrollX),
		pDesc->tViewportRect.fY + (float)iVisualRow * fLineHeight - pDesc->fScrollY,
		1.0f,
		fLineHeight
	};
	return XUI_OK;
}

#endif
