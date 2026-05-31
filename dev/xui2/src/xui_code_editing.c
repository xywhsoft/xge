#include "../xui.h"

#include <ctype.h>
#include <string.h>

static int __xuiCodeEditingWritable(xui_code_document pDocument, xui_code_selection_model pSelection, int bReadonly)
{
	if ( (pDocument == NULL) || (pSelection == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	return bReadonly ? XUI_ERROR_UNSUPPORTED : XUI_OK;
}

static int __xuiCodeEditingSelectionRange(xui_code_selection_model pSelection, int* pStart, int* pEnd)
{
	int iRet;

	iRet = xuiCodeSelectionGetRange(pSelection, pStart, pEnd);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_OK;
}

static int __xuiCodeEditingSelectedLines(xui_code_document pDocument, xui_code_selection_model pSelection, int* pStartLine, int* pEndLine)
{
	int iStart;
	int iEnd;
	int iRet;

	iRet = __xuiCodeEditingSelectionRange(pSelection, &iStart, &iEnd);
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

static int __xuiCodeEditingWordChar(char c)
{
	unsigned char ch = (unsigned char)c;
	return isalnum(ch) || ch == '_';
}

static int __xuiCodeEditingWordLeft(xui_code_document pDocument, int iOffset)
{
	const char* sText;

	sText = xuiCodeDocumentGetText(pDocument);
	if ( iOffset < 0 ) iOffset = 0;
	if ( iOffset > xuiCodeDocumentGetLength(pDocument) ) iOffset = xuiCodeDocumentGetLength(pDocument);
	if ( iOffset > 0 && __xuiCodeEditingWordChar(sText[iOffset - 1]) ) {
		while ( iOffset > 0 && __xuiCodeEditingWordChar(sText[iOffset - 1]) ) iOffset--;
	} else {
		while ( iOffset > 0 && !__xuiCodeEditingWordChar(sText[iOffset - 1]) ) iOffset--;
	}
	return iOffset;
}

static int __xuiCodeEditingWordRight(xui_code_document pDocument, int iOffset)
{
	const char* sText;
	int iLength;

	sText = xuiCodeDocumentGetText(pDocument);
	iLength = xuiCodeDocumentGetLength(pDocument);
	if ( iOffset < 0 ) iOffset = 0;
	if ( iOffset > iLength ) iOffset = iLength;
	if ( iOffset < iLength && __xuiCodeEditingWordChar(sText[iOffset]) ) {
		while ( iOffset < iLength && __xuiCodeEditingWordChar(sText[iOffset]) ) iOffset++;
	} else {
		while ( iOffset < iLength && !__xuiCodeEditingWordChar(sText[iOffset]) ) iOffset++;
	}
	return iOffset;
}

XUI_API int xuiCodeEditingInsertText(xui_code_document pDocument, xui_code_selection_model pSelection, const char* sText, int bReadonly)
{
	int iStart;
	int iEnd;
	int iLength;
	int iRet;

	iRet = __xuiCodeEditingWritable(pDocument, pSelection, bReadonly);
	if ( iRet != XUI_OK ) return iRet;
	if ( sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iLength = (int)strlen(sText);
	iRet = __xuiCodeEditingSelectionRange(pSelection, &iStart, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeDocumentReplace(pDocument, iStart, iEnd, sText);
	if ( iRet != XUI_OK ) return iRet;
	return xuiCodeSelectionGotoOffset(pSelection, pDocument, iStart + iLength, 0);
}

XUI_API int xuiCodeEditingDeleteBackward(xui_code_document pDocument, xui_code_selection_model pSelection, int bReadonly)
{
	int iStart;
	int iEnd;
	int iRet;

	iRet = __xuiCodeEditingWritable(pDocument, pSelection, bReadonly);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditingSelectionRange(pSelection, &iStart, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	if ( iStart == iEnd ) {
		if ( iStart <= 0 ) return XUI_OK;
		iStart--;
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
	iRet = __xuiCodeEditingSelectionRange(pSelection, &iStart, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	iLength = xuiCodeDocumentGetLength(pDocument);
	if ( iStart == iEnd ) {
		if ( iEnd >= iLength ) return XUI_OK;
		iEnd++;
	}
	iRet = xuiCodeDocumentDelete(pDocument, iStart, iEnd);
	if ( iRet != XUI_OK ) return iRet;
	return xuiCodeSelectionGotoOffset(pSelection, pDocument, iStart, 0);
}

XUI_API int xuiCodeEditingDeleteWordBackward(xui_code_document pDocument, xui_code_selection_model pSelection, int bReadonly)
{
	xui_code_selection_t tState;
	int iStart;
	int iEnd;
	int iCaret;
	int iRet;

	iRet = __xuiCodeEditingWritable(pDocument, pSelection, bReadonly);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditingSelectionRange(pSelection, &iStart, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	if ( iStart == iEnd ) {
		iRet = xuiCodeSelectionGetState(pSelection, &tState);
		if ( iRet != XUI_OK ) return iRet;
		iCaret = tState.iCaretOffset;
		if ( iCaret <= 0 ) return XUI_OK;
		iStart = __xuiCodeEditingWordLeft(pDocument, iCaret);
		iEnd = iCaret;
	}
	iRet = xuiCodeDocumentDelete(pDocument, iStart, iEnd);
	if ( iRet != XUI_OK ) return iRet;
	return xuiCodeSelectionGotoOffset(pSelection, pDocument, iStart, 0);
}

XUI_API int xuiCodeEditingDeleteWordForward(xui_code_document pDocument, xui_code_selection_model pSelection, int bReadonly)
{
	xui_code_selection_t tState;
	int iStart;
	int iEnd;
	int iCaret;
	int iRet;

	iRet = __xuiCodeEditingWritable(pDocument, pSelection, bReadonly);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeEditingSelectionRange(pSelection, &iStart, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	if ( iStart == iEnd ) {
		iRet = xuiCodeSelectionGetState(pSelection, &tState);
		if ( iRet != XUI_OK ) return iRet;
		iCaret = tState.iCaretOffset;
		if ( iCaret >= xuiCodeDocumentGetLength(pDocument) ) return XUI_OK;
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
	if ( sIndent == NULL || sIndent[0] == '\0' ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndentLength = (int)strlen(sIndent);
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	if ( iRet != XUI_OK ) return iRet;
	iAnchor = tState.iAnchorOffset;
	iCaret = tState.iCaretOffset;
	iRet = __xuiCodeEditingSelectedLines(pDocument, pSelection, &iStartLine, &iEndLine);
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
	const char* sText;
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

	iRet = __xuiCodeEditingWritable(pDocument, pSelection, bReadonly);
	if ( iRet != XUI_OK ) return iRet;
	if ( iIndentColumns <= 0 ) iIndentColumns = 4;
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	if ( iRet != XUI_OK ) return iRet;
	iAnchor = tState.iAnchorOffset;
	iCaret = tState.iCaretOffset;
	iRet = __xuiCodeEditingSelectedLines(pDocument, pSelection, &iStartLine, &iEndLine);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeDocumentBeginEdit(pDocument);
	if ( iRet != XUI_OK ) return iRet;
	for ( iLine = iStartLine; iLine <= iEndLine; iLine++ ) {
		iRet = xuiCodeDocumentGetLineRange(pDocument, iLine, &iLineStart, &iLineEnd);
		if ( iRet != XUI_OK ) break;
		sText = xuiCodeDocumentGetText(pDocument);
		iDeleteEnd = iLineStart;
		if ( iDeleteEnd < iLineEnd && sText[iDeleteEnd] == '\t' ) {
			iDeleteEnd++;
		 } else {
			for ( i = 0; i < iIndentColumns && iDeleteEnd < iLineEnd && sText[iDeleteEnd] == ' '; i++ ) {
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

static int __xuiCodeEditingLineCommentPos(xui_code_document pDocument, int iLine, const char* sLineComment, int* pPos, int* pHasComment)
{
	const char* sText;
	int iStart;
	int iEnd;
	int iCommentLength;
	int iRet;
	int i;

	iRet = xuiCodeDocumentGetLineRange(pDocument, iLine, &iStart, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	sText = xuiCodeDocumentGetText(pDocument);
	i = iStart;
	while ( i < iEnd && (sText[i] == ' ' || sText[i] == '\t') ) i++;
	iCommentLength = (int)strlen(sLineComment);
	*pPos = i;
	*pHasComment = (i + iCommentLength <= iEnd && strncmp(sText + i, sLineComment, (size_t)iCommentLength) == 0);
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
	if ( sLineComment == NULL || sLineComment[0] == '\0' ) return XUI_ERROR_INVALID_ARGUMENT;
	iCommentLength = (int)strlen(sLineComment);
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	if ( iRet != XUI_OK ) return iRet;
	iAnchor = tState.iAnchorOffset;
	iCaret = tState.iCaretOffset;
	iRet = __xuiCodeEditingSelectedLines(pDocument, pSelection, &iStartLine, &iEndLine);
	if ( iRet != XUI_OK ) return iRet;
	bAllCommented = 1;
	for ( iLine = iStartLine; iLine <= iEndLine; iLine++ ) {
		iRet = __xuiCodeEditingLineCommentPos(pDocument, iLine, sLineComment, &iPos, &bHasComment);
		if ( iRet != XUI_OK ) return iRet;
		if ( !bHasComment ) {
			bAllCommented = 0;
			break;
		}
	}
	iRet = xuiCodeDocumentBeginEdit(pDocument);
	if ( iRet != XUI_OK ) return iRet;
	for ( iLine = iStartLine; iLine <= iEndLine; iLine++ ) {
		iRet = __xuiCodeEditingLineCommentPos(pDocument, iLine, sLineComment, &iPos, &bHasComment);
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
