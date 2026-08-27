#include "../xui.h"

#include <string.h>

static int __xuiCodeLayoutTabColumns(const xui_code_layout_desc_t* pDesc)
{
	return (pDesc != NULL && pDesc->iTabColumns > 0) ? pDesc->iTabColumns : 4;
}

static int __xuiCodeLayoutNextVisualColumn(int iVisualColumn, char c, int iTabColumns)
{
	int iStep;

	if ( c != '\t' ) return iVisualColumn + 1;
	iStep = iTabColumns - (iVisualColumn % iTabColumns);
	return iVisualColumn + ((iStep > 0) ? iStep : iTabColumns);
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
	for ( i = iStart; i < iEnd; i++ ) {
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
	int iLimit;
	int iVisual;
	char ch;

	if ( xuiCodeDocumentGetLineRange(pDocument, iLine, &iStart, &iEnd) != XUI_OK ) return 0;
	if ( iColumn < 0 ) iColumn = 0;
	iLimit = iStart + iColumn;
	if ( iLimit > iEnd ) iLimit = iEnd;
	iVisual = 0;
	for ( i = iStart; i < iLimit; i++ ) {
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
	for ( i = iStart; i < iEnd; i++ ) {
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
	int i;

	if ( pVisible == NULL ) return -1;
	for ( i = 0; i < iCount; i++ ) {
		if ( pVisible[i] == iLine ) return i;
	}
	return -1;
}

typedef struct __xui_code_layout_segment_t {
	int iStartOffset;
	int iEndOffset;
	int iStartVisual;
	int iEndVisual;
} __xui_code_layout_segment_t;

static int __xuiCodeLayoutWrapColumns(const xui_code_layout_desc_t* pDesc, const xui_rect_t* pTextRect)
{
	float fCharWidth = (pDesc->fCharWidth > 0.0f) ? pDesc->fCharWidth : 8.0f;
	int iColumns;

	if ( !pDesc->bWordWrap ) return 0;
	iColumns = (int)(pTextRect->fW / fCharWidth);
	return (iColumns > 0) ? iColumns : 1;
}

static int __xuiCodeLayoutBreakAfter(char ch)
{
	return ch == ' ' || ch == '\t' || ch == '-' || ch == '/' || ch == '\\' ||
		ch == ',' || ch == ';' || ch == ':' || ch == '.';
}

static int __xuiCodeLayoutGetSegment(xui_code_document pDocument, int iLine,
	int iTabColumns, int iWrapColumns, int iWanted,
	__xui_code_layout_segment_t* pSegment, int* pSegmentCount)
{
	int iLineStart;
	int iLineEnd;
	int iSegment;
	int iStart;
	int iStartVisual;
	int i;
	int iVisual;
	int iNext;
	int iLastBreak;
	int iLastBreakVisual;
	int bHasContent;
	char ch;

	if ( xuiCodeDocumentGetLineRange(pDocument, iLine, &iLineStart, &iLineEnd) != XUI_OK ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iWrapColumns <= 0 ) {
		if ( pSegment != NULL && iWanted == 0 ) {
			pSegment->iStartOffset = iLineStart;
			pSegment->iEndOffset = iLineEnd;
			pSegment->iStartVisual = 0;
			pSegment->iEndVisual = __xuiCodeLayoutLineVisualColumns(pDocument, iLine, iTabColumns);
		}
		if ( pSegmentCount != NULL ) *pSegmentCount = 1;
		return (iWanted <= 0) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iLineStart == iLineEnd ) {
		if ( pSegment != NULL && iWanted == 0 ) {
			memset(pSegment, 0, sizeof(*pSegment));
			pSegment->iStartOffset = iLineStart;
			pSegment->iEndOffset = iLineEnd;
		}
		if ( pSegmentCount != NULL ) *pSegmentCount = 1;
		return (iWanted <= 0) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
	}

	iSegment = 0;
	iStart = iLineStart;
	iStartVisual = 0;
	while ( iStart < iLineEnd ) {
		iVisual = iStartVisual;
		iLastBreak = -1;
		iLastBreakVisual = iStartVisual;
		bHasContent = 0;
		for ( i = iStart; i < iLineEnd; i++ ) {
			if ( xuiCodeDocumentGetByte(pDocument, i, &ch) != XUI_OK ) break;
			iNext = __xuiCodeLayoutNextVisualColumn(iVisual, ch, iTabColumns);
			if ( iNext - iStartVisual > iWrapColumns ) break;
			iVisual = iNext;
			if ( __xuiCodeLayoutBreakAfter(ch) && bHasContent ) {
				iLastBreak = i + 1;
				iLastBreakVisual = iVisual;
			}
			if ( ch != ' ' && ch != '\t' ) bHasContent = 1;
		}
		if ( i < iLineEnd && iLastBreak > iStart ) {
			i = iLastBreak;
			iVisual = iLastBreakVisual;
		} else if ( i == iStart ) {
			if ( xuiCodeDocumentGetByte(pDocument, i, &ch) == XUI_OK ) {
				iVisual = __xuiCodeLayoutNextVisualColumn(iStartVisual, ch, iTabColumns);
			}
			i++;
		}
		if ( pSegment != NULL && iSegment == iWanted ) {
			pSegment->iStartOffset = iStart;
			pSegment->iEndOffset = i;
			pSegment->iStartVisual = iStartVisual;
			pSegment->iEndVisual = iVisual;
		}
		iSegment++;
		iStart = i;
		iStartVisual = iVisual;
	}
	if ( pSegmentCount != NULL ) *pSegmentCount = iSegment;
	return (iWanted < 0 || iWanted < iSegment) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}

static int __xuiCodeLayoutVisualRow(const xui_code_layout_desc_t* pDesc,
	const int* pVisible, int iVisibleCount, int iWrapColumns,
	int iWantedRow, int* pLine, int* pSegmentIndex,
	__xui_code_layout_segment_t* pSegment, int* pTotalRows)
{
	int i;
	int iRows;
	int iCount;

	iRows = 0;
	for ( i = 0; i < iVisibleCount; i++ ) {
		if ( __xuiCodeLayoutGetSegment(pDesc->pDocument, pVisible[i],
			__xuiCodeLayoutTabColumns(pDesc), iWrapColumns, -1, NULL, &iCount) != XUI_OK ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		if ( iWantedRow >= iRows && iWantedRow < iRows + iCount ) {
			int iSegment = iWantedRow - iRows;
			if ( pLine != NULL ) *pLine = pVisible[i];
			if ( pSegmentIndex != NULL ) *pSegmentIndex = iSegment;
			if ( pSegment != NULL ) {
				(void)__xuiCodeLayoutGetSegment(pDesc->pDocument, pVisible[i],
					__xuiCodeLayoutTabColumns(pDesc), iWrapColumns, iSegment, pSegment, NULL);
			}
		}
		iRows += iCount;
	}
	if ( pTotalRows != NULL ) *pTotalRows = iRows;
	return (iWantedRow < 0 || iWantedRow < iRows) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
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
	int iSegmentCount;
	int iSegment;
	int iTotalRows;
	__xui_code_layout_segment_t tSegment;
	xui_rect_t tTextRect;

	if ( (pDesc == NULL) || (pDesc->pDocument == NULL) || (pLineCount == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pLineCount = 0;
	fLineHeight = (pDesc->fLineHeight > 0.0f) ? pDesc->fLineHeight : 18.0f;
	fCharWidth = (pDesc->fCharWidth > 0.0f) ? pDesc->fCharWidth : 8.0f;
	iTabColumns = __xuiCodeLayoutTabColumns(pDesc);
	tTextRect = pDesc->tViewportRect;
	tTextRect.fX += pDesc->fMarginWidth;
	tTextRect.fW -= pDesc->fMarginWidth;
	if ( tTextRect.fW < 0.0f ) tTextRect.fW = 0.0f;
	iWrapColumns = __xuiCodeLayoutWrapColumns(pDesc, &tTextRect);
	iRet = __xuiCodeLayoutVisibleLines(pDesc, &pVisible, &iVisibleCount);
	if ( iRet != XUI_OK ) return iRet;
	fContentWidth = 0.0f;
	for ( i = 0; i < iVisibleCount; i++ ) {
		float fWidth = __xuiCodeLayoutLineWidth(pDesc->pDocument, pVisible[i], fCharWidth, iTabColumns);
		if ( fWidth > fContentWidth ) fContentWidth = fWidth;
	}
	iOut = 0;
	iVisualRow = 0;
	for ( i = 0; i < iVisibleCount; i++ ) {
		(void)__xuiCodeLayoutGetSegment(pDesc->pDocument, pVisible[i], iTabColumns,
			iWrapColumns, -1, NULL, &iSegmentCount);
		for ( iSegment = 0; iSegment < iSegmentCount; iSegment++, iVisualRow++ ) {
			fY = pDesc->tViewportRect.fY + (float)iVisualRow * fLineHeight - pDesc->fScrollY;
			if ( fY + fLineHeight < pDesc->tViewportRect.fY ) continue;
			if ( fY > pDesc->tViewportRect.fY + pDesc->tViewportRect.fH ) continue;
			(void)__xuiCodeLayoutGetSegment(pDesc->pDocument, pVisible[i], iTabColumns,
				iWrapColumns, iSegment, &tSegment, NULL);
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
	int iSegmentIndex;
	int iSegmentVisual;
	int iTotalRows = 0;
	int iLineStart;
	int iMaxColumn;
	__xui_code_layout_segment_t tSegment;
	xui_rect_t tTextRect;

	if ( (pDesc == NULL) || (pDesc->pDocument == NULL) || (pHit == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pHit, 0, sizeof(*pHit));
	pHit->iSize = sizeof(*pHit);
	fLineHeight = (pDesc->fLineHeight > 0.0f) ? pDesc->fLineHeight : 18.0f;
	fCharWidth = (pDesc->fCharWidth > 0.0f) ? pDesc->fCharWidth : 8.0f;
	iTabColumns = __xuiCodeLayoutTabColumns(pDesc);
	iRet = __xuiCodeLayoutVisibleLines(pDesc, &pVisible, &iVisibleCount);
	if ( iRet != XUI_OK ) return iRet;
	tTextRect = pDesc->tViewportRect;
	tTextRect.fX += pDesc->fMarginWidth;
	tTextRect.fW -= pDesc->fMarginWidth;
	if ( tTextRect.fW < 0.0f ) tTextRect.fW = 0.0f;
	iWrapColumns = __xuiCodeLayoutWrapColumns(pDesc, &tTextRect);
	iRet = __xuiCodeLayoutVisualRow(pDesc, pVisible, iVisibleCount, iWrapColumns,
		-1, NULL, NULL, NULL, &iTotalRows);
	if ( iRet != XUI_OK ) {
		xrtFree(pVisible);
		return iRet;
	}
	iIndex = (int)((fY - pDesc->tViewportRect.fY + pDesc->fScrollY) / fLineHeight);
	if ( iIndex < 0 ) iIndex = 0;
	if ( iIndex >= iTotalRows ) iIndex = iTotalRows - 1;
	if ( iIndex < 0 ) {
		xrtFree(pVisible);
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiCodeLayoutVisualRow(pDesc, pVisible, iVisibleCount, iWrapColumns,
		iIndex, &iLine, &iSegmentIndex, &tSegment, NULL);
	if ( iRet != XUI_OK ) {
		xrtFree(pVisible);
		return iRet;
	}
	iSegmentVisual = (int)((fX - tTextRect.fX + (pDesc->bWordWrap ? 0.0f : pDesc->fScrollX)) / fCharWidth + 0.5f);
	if ( iSegmentVisual < 0 ) iSegmentVisual = 0;
	iColumn = __xuiCodeLayoutVisualToColumn(pDesc->pDocument, iLine,
		tSegment.iStartVisual + iSegmentVisual, iTabColumns);
	(void)xuiCodeDocumentGetLineRange(pDesc->pDocument, iLine, &iLineStart, NULL);
	if ( iColumn < tSegment.iStartOffset - iLineStart ) {
		iColumn = tSegment.iStartOffset - iLineStart;
	}
	iMaxColumn = tSegment.iEndOffset - iLineStart;
	if ( iColumn > iMaxColumn ) iColumn = iMaxColumn;
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
	int iSegment;
	int iLineStart;
	int iTargetOffset;
	int iVisualRow;
	int i;
	__xui_code_layout_segment_t tSegment;
	xui_rect_t tTextRect;

	if ( (pDesc == NULL) || (pDesc->pDocument == NULL) || (pRect == NULL) ) {
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
	fLineHeight = (pDesc->fLineHeight > 0.0f) ? pDesc->fLineHeight : 18.0f;
	fCharWidth = (pDesc->fCharWidth > 0.0f) ? pDesc->fCharWidth : 8.0f;
	iTabColumns = __xuiCodeLayoutTabColumns(pDesc);
	iVisualColumn = __xuiCodeLayoutColumnToVisual(pDesc->pDocument, iLine, iColumn, iTabColumns);
	tTextRect = pDesc->tViewportRect;
	tTextRect.fX += pDesc->fMarginWidth;
	tTextRect.fW -= pDesc->fMarginWidth;
	if ( tTextRect.fW < 0.0f ) tTextRect.fW = 0.0f;
	iWrapColumns = __xuiCodeLayoutWrapColumns(pDesc, &tTextRect);
	(void)xuiCodeDocumentGetLineRange(pDesc->pDocument, iLine, &iLineStart, NULL);
	iTargetOffset = iLineStart + iColumn;
	iVisualRow = 0;
	for ( i = 0; i < iIndex; i++ ) {
		(void)__xuiCodeLayoutGetSegment(pDesc->pDocument, pVisible[i], iTabColumns,
			iWrapColumns, -1, NULL, &iSegmentCount);
		iVisualRow += iSegmentCount;
	}
	(void)__xuiCodeLayoutGetSegment(pDesc->pDocument, iLine, iTabColumns,
		iWrapColumns, -1, NULL, &iSegmentCount);
	for ( iSegment = 0; iSegment < iSegmentCount; iSegment++ ) {
		(void)__xuiCodeLayoutGetSegment(pDesc->pDocument, iLine, iTabColumns,
			iWrapColumns, iSegment, &tSegment, NULL);
		if ( iTargetOffset < tSegment.iEndOffset || iSegment == iSegmentCount - 1 ) break;
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
