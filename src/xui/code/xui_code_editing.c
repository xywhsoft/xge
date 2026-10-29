#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui_internal.h"

#include <limits.h>
#include <string.h>

static int __xuiCodeEditingUnicodeRead(void* pUser, int iOffset,
	unsigned char* pByte);

static int __xuiCodeEditingWritable(xui_code_document pDocument, xui_code_selection_model pSelection, int bReadonly)
{
	if ( (pDocument == NULL) || (pSelection == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	return bReadonly ? XUI_ERROR_UNSUPPORTED : XUI_OK;
}

static int __xuiCodeEditingClampOffset(xui_code_document pDocument, int iOffset)
{
	int iLength;

	if ( pDocument == NULL ) return 0;
	iLength = xuiCodeDocumentGetLength(pDocument);
	if ( iOffset <= 0 ) return 0;
	if ( iOffset >= iLength ) return iLength;
	return xuiInternalTextGraphemeClampRead(__xuiCodeEditingUnicodeRead,
		pDocument, iLength, iOffset);
}

static int __xuiCodeEditingSelectionRange(xui_code_document pDocument,
	xui_code_selection_model pSelection, int* pStart, int* pEnd)
{
	int iRet;

	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiCodeSelectionGetRange(pSelection, pStart, pEnd);
	if ( iRet != XUI_OK ) return iRet;
	*pStart = __xuiCodeEditingClampOffset(pDocument, *pStart);
	*pEnd = __xuiCodeEditingClampOffset(pDocument, *pEnd);
	return XUI_OK;
}

static int __xuiCodeEditingSelectedLines(xui_code_document pDocument, xui_code_selection_model pSelection, int* pStartLine, int* pEndLine)
{
	int iStart;
	int iEnd;
	int iRet;

	iRet = __xuiCodeEditingSelectionRange(pDocument, pSelection, &iStart, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeDocumentOffsetToLineColumn(pDocument, iStart, pStartLine, NULL);
	if ( iRet != XUI_OK ) return iRet;
	if ( iEnd > iStart ) {
		int iLineStart;
		iRet = xuiCodeDocumentOffsetToLineColumn(pDocument, iEnd, pEndLine, NULL);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiCodeDocumentGetLineRange(pDocument, *pEndLine, &iLineStart, NULL);
		if ( iRet != XUI_OK ) return iRet;
		if ( iEnd == iLineStart && *pEndLine > *pStartLine ) (*pEndLine)--;
	} else {
		*pEndLine = *pStartLine;
	}
	return XUI_OK;
}

static int __xuiCodeEditingAdjustOffsetForInsert(int iOffset, int iAt, int iLength)
{
	return (iOffset >= iAt) ? iOffset + iLength : iOffset;
}

static int __xuiCodeEditingAdjustOffsetForDelete(int iOffset, int iStart, int iEnd)
{
	int iLength;

	iLength = iEnd - iStart;
	if ( iOffset <= iStart ) return iOffset;
	if ( iOffset >= iEnd ) return iOffset - iLength;
	return iStart;
}

static int __xuiCodeEditingTextLength(const char* sText, int* pLength)
{
	size_t iLength;

	if ( sText == NULL || pLength == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iLength = strlen(sText);
	if ( iLength > (size_t)INT_MAX ) return XUI_ERROR_INVALID_ARGUMENT;
	*pLength = (int)iLength;
	return XUI_OK;
}

static int __xuiCodeEditingCanGrowDocument(xui_code_document pDocument,
	int iItemCount, int iItemLength)
{
	int iLength;

	if ( pDocument == NULL || iItemCount < 0 || iItemLength < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iLength = xuiCodeDocumentGetLength(pDocument);
	if ( iItemLength > 0 && iItemCount > (INT_MAX - iLength) / iItemLength ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	return XUI_OK;
}

static int __xuiCodeEditingNormalizeCollapsedSelection(
	xui_code_selection_model pSelection, xui_code_document pDocument, int iOffset)
{
	xui_code_selection_t tState;
	int iRet;

	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	if ( iRet != XUI_OK ) return iRet;
	if ( tState.iAnchorOffset == iOffset && tState.iCaretOffset == iOffset ) {
		return XUI_OK;
	}
	return xuiCodeSelectionGotoOffset(pSelection, pDocument, iOffset, 0);
}

static char __xuiCodeEditingByteAt(xui_code_document pDocument, int iOffset)
{
	char c;

	c = '\0';
	(void)xuiCodeDocumentGetByte(pDocument, iOffset, &c);
	return c;
}

static int __xuiCodeEditingUnicodeRead(void* pUser, int iOffset, unsigned char* pByte)
{
	char c;

	if ( pUser == NULL || pByte == NULL ) return 0;
	c = '\0';
	if ( xuiCodeDocumentGetByte((xui_code_document)pUser, iOffset, &c) != XUI_OK ) return 0;
	*pByte = (unsigned char)c;
	return 1;
}

static int __xuiCodeEditingUtf8Next(xui_code_document pDocument, int iLength, int iOffset)
{
	if ( pDocument == NULL ) return 0;
	return xuiInternalTextGraphemeNextRead(__xuiCodeEditingUnicodeRead,
		pDocument, iLength, iOffset);
}

static int __xuiCodeEditingUtf8Prev(xui_code_document pDocument, int iLength, int iOffset)
{
	if ( pDocument == NULL ) return 0;
	return xuiInternalTextGraphemePrevRead(__xuiCodeEditingUnicodeRead,
		pDocument, iLength, iOffset);
}

static int __xuiCodeEditingWordLeft(xui_code_document pDocument, int iOffset)
{
	int iLength;

	iLength = xuiCodeDocumentGetLength(pDocument);
	if ( iOffset < 0 ) iOffset = 0;
	if ( iOffset > iLength ) iOffset = iLength;
	return xuiInternalTextWordPrevRead(__xuiCodeEditingUnicodeRead, pDocument,
		iLength, iOffset, XUI_INTERNAL_WORD_IDENTIFIER);
}

static int __xuiCodeEditingWordRight(xui_code_document pDocument, int iOffset)
{
	xui_internal_word_kind_t iKind;
	int iLength;
	int iStart;
	int iEnd;

	iLength = xuiCodeDocumentGetLength(pDocument);
	if ( iOffset < 0 ) iOffset = 0;
	if ( iOffset > iLength ) iOffset = iLength;
	iKind = xuiInternalTextWordRangeRead(__xuiCodeEditingUnicodeRead, pDocument,
		iLength, iOffset, XUI_INTERNAL_WORD_IDENTIFIER, &iStart, &iEnd);
	if ( iKind == XUI_INTERNAL_WORD_TEXT && iStart <= iOffset && iEnd > iOffset ) return iEnd;
	return xuiInternalTextWordNextRead(__xuiCodeEditingUnicodeRead, pDocument,
		iLength, iOffset, XUI_INTERNAL_WORD_IDENTIFIER);
}

XUI_API int xuiCodeEditingInsertText(xui_code_document pDocument, xui_code_selection_model pSelection, const char* sText, int bReadonly)
{
	int iStart;
	int iEnd;
	int iLength;
	int iOldDocumentLength;
	int iRet;

	iRet = __xuiCodeEditingWritable(pDocument, pSelection, bReadonly);
	if ( iRet != XUI_OK ) return iRet;
	if ( sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCodeEditingSelectionRange(pDocument, pSelection, &iStart, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	iOldDocumentLength = xuiCodeDocumentGetLength(pDocument);
	iRet = xuiCodeDocumentReplace(pDocument, iStart, iEnd, sText);
	if ( iRet != XUI_OK ) return iRet;
	iLength = xuiCodeDocumentGetLength(pDocument) -
		(iOldDocumentLength - (iEnd - iStart));
	return xuiCodeSelectionGotoOffset(pSelection, pDocument, iStart + iLength, 0);
}

XUI_API int xuiCodeEditingDeleteBackward(xui_code_document pDocument, xui_code_selection_model pSelection, int bReadonly)
{
	int iStart;
	int iEnd;
	int iRet;

	iRet = __xuiCodeEditingWritable(pDocument, pSelection, bReadonly);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditingSelectionRange(pDocument, pSelection, &iStart, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	if ( iStart == iEnd ) {
		if ( iStart <= 0 ) {
			return __xuiCodeEditingNormalizeCollapsedSelection(pSelection, pDocument, 0);
		}
		iStart = __xuiCodeEditingUtf8Prev(pDocument, xuiCodeDocumentGetLength(pDocument), iStart);
	}
	iRet = xuiCodeDocumentDelete(pDocument, iStart, iEnd);
	if ( iRet != XUI_OK ) return iRet;
	return xuiCodeSelectionGotoOffset(pSelection, pDocument, iStart, 0);
}

XUI_API int xuiCodeEditingDeleteForward(xui_code_document pDocument, xui_code_selection_model pSelection, int bReadonly)
{
	int iStart;
	int iEnd;
	int iLength;
	int iRet;

	iRet = __xuiCodeEditingWritable(pDocument, pSelection, bReadonly);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditingSelectionRange(pDocument, pSelection, &iStart, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	iLength = xuiCodeDocumentGetLength(pDocument);
	if ( iStart == iEnd ) {
		if ( iEnd >= iLength ) {
			return __xuiCodeEditingNormalizeCollapsedSelection(pSelection, pDocument, iLength);
		}
		iEnd = __xuiCodeEditingUtf8Next(pDocument, iLength, iEnd);
	}
	iRet = xuiCodeDocumentDelete(pDocument, iStart, iEnd);
	if ( iRet != XUI_OK ) return iRet;
	return xuiCodeSelectionGotoOffset(pSelection, pDocument, iStart, 0);
}

XUI_API int xuiCodeEditingDeleteWordBackward(xui_code_document pDocument, xui_code_selection_model pSelection, int bReadonly)
{
	int iStart;
	int iEnd;
	int iCaret;
	int iRet;

	iRet = __xuiCodeEditingWritable(pDocument, pSelection, bReadonly);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditingSelectionRange(pDocument, pSelection, &iStart, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	if ( iStart == iEnd ) {
		iCaret = iStart;
		if ( iCaret <= 0 ) {
			return __xuiCodeEditingNormalizeCollapsedSelection(pSelection, pDocument, 0);
		}
		iStart = __xuiCodeEditingWordLeft(pDocument, iCaret);
		iEnd = iCaret;
	}
	iRet = xuiCodeDocumentDelete(pDocument, iStart, iEnd);
	if ( iRet != XUI_OK ) return iRet;
	return xuiCodeSelectionGotoOffset(pSelection, pDocument, iStart, 0);
}

XUI_API int xuiCodeEditingDeleteWordForward(xui_code_document pDocument, xui_code_selection_model pSelection, int bReadonly)
{
	int iStart;
	int iEnd;
	int iCaret;
	int iRet;

	iRet = __xuiCodeEditingWritable(pDocument, pSelection, bReadonly);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditingSelectionRange(pDocument, pSelection, &iStart, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	if ( iStart == iEnd ) {
		iCaret = iStart;
		if ( iCaret >= xuiCodeDocumentGetLength(pDocument) ) {
			return __xuiCodeEditingNormalizeCollapsedSelection(pSelection,
				pDocument, xuiCodeDocumentGetLength(pDocument));
		}
		iStart = iCaret;
		iEnd = __xuiCodeEditingWordRight(pDocument, iCaret);
	}
	iRet = xuiCodeDocumentDelete(pDocument, iStart, iEnd);
	if ( iRet != XUI_OK ) return iRet;
	return xuiCodeSelectionGotoOffset(pSelection, pDocument, iStart, 0);
}

XUI_API int xuiCodeEditingIndentSelection(xui_code_document pDocument, xui_code_selection_model pSelection, const char* sIndent, int bReadonly)
{
	xui_code_selection_t tState;
	int iStartLine;
	int iEndLine;
	int iLineStart;
	int iIndentLength;
	int iAnchor;
	int iCaret;
	int iLine;
	int iRet;

	iRet = __xuiCodeEditingWritable(pDocument, pSelection, bReadonly);
	if ( iRet != XUI_OK ) return iRet;
	if ( sIndent == NULL || sIndent[0] == '\0' ||
	     strchr(sIndent, '\r') != NULL || strchr(sIndent, '\n') != NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiCodeEditingTextLength(sIndent, &iIndentLength);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	if ( iRet != XUI_OK ) return iRet;
	iAnchor = __xuiCodeEditingClampOffset(pDocument, tState.iAnchorOffset);
	iCaret = __xuiCodeEditingClampOffset(pDocument, tState.iCaretOffset);
	iRet = __xuiCodeEditingSelectedLines(pDocument, pSelection, &iStartLine, &iEndLine);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditingCanGrowDocument(pDocument,
		iEndLine - iStartLine + 1, iIndentLength);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeDocumentBeginEdit(pDocument);
	if ( iRet != XUI_OK ) return iRet;
	for ( iLine = iStartLine; iLine <= iEndLine; iLine++ ) {
		iRet = xuiCodeDocumentGetLineRange(pDocument, iLine, &iLineStart, NULL);
		if ( iRet != XUI_OK ) break;
		iRet = xuiCodeDocumentInsert(pDocument, iLineStart, sIndent);
		if ( iRet != XUI_OK ) break;
		iAnchor = __xuiCodeEditingAdjustOffsetForInsert(iAnchor, iLineStart, iIndentLength);
		iCaret = __xuiCodeEditingAdjustOffsetForInsert(iCaret, iLineStart, iIndentLength);
	}
	(void)xuiCodeDocumentEndEdit(pDocument);
	if ( iRet != XUI_OK ) return iRet;
	return xuiCodeSelectionSetRange(pSelection, pDocument, iAnchor, iCaret);
}

XUI_API int xuiCodeEditingOutdentSelection(xui_code_document pDocument, xui_code_selection_model pSelection, int iIndentColumns, int bReadonly)
{
	xui_code_selection_t tState;
	int iStartLine;
	int iEndLine;
	int iLineStart;
	int iLineEnd;
	int iDeleteEnd;
	int iAnchor;
	int iCaret;
	int iLine;
	int i;
	int iRet;
	char c;

	iRet = __xuiCodeEditingWritable(pDocument, pSelection, bReadonly);
	if ( iRet != XUI_OK ) return iRet;
	if ( iIndentColumns <= 0 ) iIndentColumns = 4;
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	if ( iRet != XUI_OK ) return iRet;
	iAnchor = __xuiCodeEditingClampOffset(pDocument, tState.iAnchorOffset);
	iCaret = __xuiCodeEditingClampOffset(pDocument, tState.iCaretOffset);
	iRet = __xuiCodeEditingSelectedLines(pDocument, pSelection, &iStartLine, &iEndLine);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeDocumentBeginEdit(pDocument);
	if ( iRet != XUI_OK ) return iRet;
	for ( iLine = iStartLine; iLine <= iEndLine; iLine++ ) {
		iRet = xuiCodeDocumentGetLineRange(pDocument, iLine, &iLineStart, &iLineEnd);
		if ( iRet != XUI_OK ) break;
		iDeleteEnd = iLineStart;
		c = __xuiCodeEditingByteAt(pDocument, iDeleteEnd);
		if ( iDeleteEnd < iLineEnd && c == '\t' ) {
			iDeleteEnd++;
		 } else {
			for ( i = 0; i < iIndentColumns && iDeleteEnd < iLineEnd &&
			      __xuiCodeEditingByteAt(pDocument, iDeleteEnd) == ' '; i++ ) {
				iDeleteEnd++;
			}
		}
		if ( iDeleteEnd > iLineStart ) {
			iRet = xuiCodeDocumentDelete(pDocument, iLineStart, iDeleteEnd);
			if ( iRet != XUI_OK ) break;
			iAnchor = __xuiCodeEditingAdjustOffsetForDelete(iAnchor, iLineStart, iDeleteEnd);
			iCaret = __xuiCodeEditingAdjustOffsetForDelete(iCaret, iLineStart, iDeleteEnd);
		}
	}
	(void)xuiCodeDocumentEndEdit(pDocument);
	if ( iRet != XUI_OK ) return iRet;
	return xuiCodeSelectionSetRange(pSelection, pDocument, iAnchor, iCaret);
}

static int __xuiCodeEditingLineCommentPos(xui_code_document pDocument, int iLine,
	const char* sLineComment, int iCommentLength, int* pPos, int* pHasComment)
{
	int iStart;
	int iEnd;
	int iRet;
	int i;
	int j;

	iRet = xuiCodeDocumentGetLineRange(pDocument, iLine, &iStart, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	i = iStart;
	while ( i < iEnd &&
	       (__xuiCodeEditingByteAt(pDocument, i) == ' ' || __xuiCodeEditingByteAt(pDocument, i) == '\t') ) i++;
	*pPos = i;
	*pHasComment = (iCommentLength <= iEnd - i);
	for ( j = 0; *pHasComment && j < iCommentLength; j++ ) {
		if ( __xuiCodeEditingByteAt(pDocument, i + j) != sLineComment[j] ) *pHasComment = 0;
	}
	return XUI_OK;
}

XUI_API int xuiCodeEditingToggleLineComment(xui_code_document pDocument, xui_code_selection_model pSelection, const char* sLineComment, int bReadonly)
{
	xui_code_selection_t tState;
	int iStartLine;
	int iEndLine;
	int iLine;
	int iPos;
	int bHasComment;
	int bAllCommented;
	int iCommentLength;
	int iAnchor;
	int iCaret;
	int iRet;

	iRet = __xuiCodeEditingWritable(pDocument, pSelection, bReadonly);
	if ( iRet != XUI_OK ) return iRet;
	if ( sLineComment == NULL || sLineComment[0] == '\0' ||
	     strchr(sLineComment, '\r') != NULL || strchr(sLineComment, '\n') != NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiCodeEditingTextLength(sLineComment, &iCommentLength);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	if ( iRet != XUI_OK ) return iRet;
	iAnchor = __xuiCodeEditingClampOffset(pDocument, tState.iAnchorOffset);
	iCaret = __xuiCodeEditingClampOffset(pDocument, tState.iCaretOffset);
	iRet = __xuiCodeEditingSelectedLines(pDocument, pSelection, &iStartLine, &iEndLine);
	if ( iRet != XUI_OK ) return iRet;
	bAllCommented = 1;
	for ( iLine = iStartLine; iLine <= iEndLine; iLine++ ) {
		iRet = __xuiCodeEditingLineCommentPos(pDocument, iLine,
			sLineComment, iCommentLength, &iPos, &bHasComment);
		if ( iRet != XUI_OK ) return iRet;
		if ( !bHasComment ) {
			bAllCommented = 0;
			break;
		}
	}
	if ( !bAllCommented ) {
		iRet = __xuiCodeEditingCanGrowDocument(pDocument,
			iEndLine - iStartLine + 1, iCommentLength);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = xuiCodeDocumentBeginEdit(pDocument);
	if ( iRet != XUI_OK ) return iRet;
	for ( iLine = iStartLine; iLine <= iEndLine; iLine++ ) {
		iRet = __xuiCodeEditingLineCommentPos(pDocument, iLine,
			sLineComment, iCommentLength, &iPos, &bHasComment);
		if ( iRet != XUI_OK ) break;
		if ( bAllCommented ) {
			iRet = xuiCodeDocumentDelete(pDocument, iPos, iPos + iCommentLength);
			if ( iRet != XUI_OK ) break;
			iAnchor = __xuiCodeEditingAdjustOffsetForDelete(iAnchor, iPos, iPos + iCommentLength);
			iCaret = __xuiCodeEditingAdjustOffsetForDelete(iCaret, iPos, iPos + iCommentLength);
		} else {
			iRet = xuiCodeDocumentInsert(pDocument, iPos, sLineComment);
			if ( iRet != XUI_OK ) break;
			iAnchor = __xuiCodeEditingAdjustOffsetForInsert(iAnchor, iPos, iCommentLength);
			iCaret = __xuiCodeEditingAdjustOffsetForInsert(iCaret, iPos, iCommentLength);
		}
	}
	(void)xuiCodeDocumentEndEdit(pDocument);
	if ( iRet != XUI_OK ) return iRet;
	return xuiCodeSelectionSetRange(pSelection, pDocument, iAnchor, iCaret);
}

#endif
