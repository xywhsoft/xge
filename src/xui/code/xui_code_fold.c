#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define XUI_CODE_FOLD_STACK_MAX 256

typedef struct xui_code_fold_stack_t {
	int iLine;
	int iLevel;
	uint32_t iFlags;
} xui_code_fold_stack_t;

typedef struct xui_code_fold_source_t {
	const char* sText;
	xui_code_document pDocument;
	int iLength;
} xui_code_fold_source_t;

typedef struct xui_code_fold_hidden_t {
	int iStartLine;
	int iEndLine;
} xui_code_fold_hidden_t;

static int __xuiCodeFoldRangeValid(const xui_code_fold_range_t* pRange)
{
	return pRange != NULL &&
		(pRange->iSize == 0u || pRange->iSize >= sizeof(*pRange)) &&
		pRange->iStartLine >= 0 && pRange->iEndLine > pRange->iStartLine;
}

static int __xuiCodeFoldCompareHidden(const void* pA, const void* pB)
{
	const xui_code_fold_hidden_t* pLeft = (const xui_code_fold_hidden_t*)pA;
	const xui_code_fold_hidden_t* pRight = (const xui_code_fold_hidden_t*)pB;

	if ( pLeft->iStartLine < pRight->iStartLine ) return -1;
	if ( pLeft->iStartLine > pRight->iStartLine ) return 1;
	if ( pLeft->iEndLine > pRight->iEndLine ) return -1;
	if ( pLeft->iEndLine < pRight->iEndLine ) return 1;
	return 0;
}

static char __xuiCodeFoldByteAt(const xui_code_fold_source_t* pSource, int iOffset)
{
	char c;

	if ( pSource == NULL || iOffset < 0 || iOffset >= pSource->iLength ) return '\0';
	if ( pSource->sText != NULL ) return pSource->sText[iOffset];
	c = '\0';
	(void)xuiCodeDocumentGetByte(pSource->pDocument, iOffset, &c);
	return c;
}

static int __xuiCodeFoldMatch(const xui_code_fold_source_t* pSource, int iOffset, const char* sValue)
{
	int i;

	if ( pSource == NULL || sValue == NULL ) return 0;
	for ( i = 0; sValue[i] != '\0'; i++ ) {
		if ( iOffset + i >= pSource->iLength || __xuiCodeFoldByteAt(pSource, iOffset + i) != sValue[i] ) return 0;
	}
	return 1;
}

static int __xuiCodeFoldDirectiveMatch(const xui_code_fold_source_t* pSource,
	int iOffset, const char* sValue)
{
	char c;
	int iLength;

	if ( !__xuiCodeFoldMatch(pSource, iOffset, sValue) ) return 0;
	iLength = (int)strlen(sValue);
	c = __xuiCodeFoldByteAt(pSource, iOffset + iLength);
	return !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
		(c >= '0' && c <= '9') || c == '_');
}

static int __xuiCodeFoldAddRange(xui_code_fold_range_t* pRanges, int iCapacity, int* pCount, int iStartLine, int iEndLine, int iLevel, uint32_t iFlags)
{
	xui_code_fold_range_t* pRange;

	if ( pCount == NULL || *pCount < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iEndLine <= iStartLine ) return XUI_OK;
	if ( *pCount == INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
	if ( (*pCount >= iCapacity) || (pRanges == NULL) ) {
		(*pCount)++;
		return XUI_OK;
	}
	pRange = &pRanges[*pCount];
	memset(pRange, 0, sizeof(*pRange));
	pRange->iSize = sizeof(*pRange);
	pRange->iStartLine = iStartLine;
	pRange->iEndLine = iEndLine;
	pRange->iLevel = iLevel;
	pRange->iFlags = iFlags | XUI_CODE_FOLD_HEADER;
	(*pCount)++;
	return XUI_OK;
}

static int __xuiCodeFoldCBuildRanges(const xui_code_fold_source_t* pSource,
	xui_code_fold_range_t* pRanges, int iRangeCapacity, int* pRangeCount)
{
	xui_code_fold_stack_t arrStack[XUI_CODE_FOLD_STACK_MAX];
	xui_code_fold_stack_t arrPreproc[XUI_CODE_FOLD_STACK_MAX];
	int iStackCount;
	int iPreprocCount;
	int iLine;
	int iLineStart;
	int i;
	int iRet;

	if ( pRangeCount == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pRangeCount = 0;
	if ( pSource == NULL || pSource->iLength < 0 || iRangeCapacity < 0 ||
	     (iRangeCapacity > 0 && pRanges == NULL) ||
	     (pSource->sText == NULL && pSource->pDocument == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iStackCount = 0;
	iPreprocCount = 0;
	iLine = 0;
	iLineStart = 0;
	i = 0;
	while ( i < pSource->iLength ) {
		if ( __xuiCodeFoldByteAt(pSource, i) == '\n' ) {
			iLine++;
			iLineStart = i + 1;
			i++;
			continue;
		}
		if ( (i == iLineStart) || ((i > iLineStart) &&
		     (__xuiCodeFoldByteAt(pSource, i - 1) == ' ' || __xuiCodeFoldByteAt(pSource, i - 1) == '\t')) ) {
			int j = iLineStart;
			while ( j < pSource->iLength &&
			      (__xuiCodeFoldByteAt(pSource, j) == ' ' || __xuiCodeFoldByteAt(pSource, j) == '\t') ) j++;
			if ( i == j && __xuiCodeFoldByteAt(pSource, j) == '#' ) {
				if ( __xuiCodeFoldDirectiveMatch(pSource, j, "#if") ||
				     __xuiCodeFoldDirectiveMatch(pSource, j, "#ifdef") ||
				     __xuiCodeFoldDirectiveMatch(pSource, j, "#ifndef") ) {
					if ( iPreprocCount >= XUI_CODE_FOLD_STACK_MAX ) {
						*pRangeCount = 0;
						return XUI_ERROR_LIMIT_EXCEEDED;
					}
					arrPreproc[iPreprocCount].iLine = iLine;
					arrPreproc[iPreprocCount].iLevel = iPreprocCount;
					arrPreproc[iPreprocCount].iFlags = XUI_CODE_FOLD_PREPROCESSOR;
					iPreprocCount++;
				} else if ( __xuiCodeFoldDirectiveMatch(pSource, j, "#endif") && iPreprocCount > 0 ) {
					iPreprocCount--;
					iRet = __xuiCodeFoldAddRange(pRanges, iRangeCapacity, pRangeCount,
						arrPreproc[iPreprocCount].iLine, iLine, arrPreproc[iPreprocCount].iLevel, arrPreproc[iPreprocCount].iFlags);
					if ( iRet != XUI_OK ) return iRet;
				}
			}
		}
		if ( __xuiCodeFoldByteAt(pSource, i) == '"' ) {
			i++;
			while ( i < pSource->iLength ) {
				if ( __xuiCodeFoldByteAt(pSource, i) == '\\' && i + 1 < pSource->iLength ) {
					if ( __xuiCodeFoldByteAt(pSource, i + 1) == '\n' ) {
						iLine++;
						iLineStart = i + 2;
					}
					i += 2;
				}
				else if ( __xuiCodeFoldByteAt(pSource, i) == '"' ) { i++; break; }
				else if ( __xuiCodeFoldByteAt(pSource, i) == '\n' ) {
					iLine++;
					iLineStart = ++i;
					break;
				}
				else i++;
			}
			continue;
		}
		if ( __xuiCodeFoldByteAt(pSource, i) == '\'' ) {
			i++;
			while ( i < pSource->iLength ) {
				if ( __xuiCodeFoldByteAt(pSource, i) == '\\' && i + 1 < pSource->iLength ) {
					if ( __xuiCodeFoldByteAt(pSource, i + 1) == '\n' ) {
						iLine++;
						iLineStart = i + 2;
					}
					i += 2;
				}
				else if ( __xuiCodeFoldByteAt(pSource, i) == '\'' ) { i++; break; }
				else if ( __xuiCodeFoldByteAt(pSource, i) == '\n' ) {
					iLine++;
					iLineStart = ++i;
					break;
				}
				else i++;
			}
			continue;
		}
		if ( __xuiCodeFoldByteAt(pSource, i) == '/' && i + 1 < pSource->iLength &&
		     __xuiCodeFoldByteAt(pSource, i + 1) == '/' ) {
			while ( i < pSource->iLength && __xuiCodeFoldByteAt(pSource, i) != '\n' ) i++;
			continue;
		}
		if ( __xuiCodeFoldByteAt(pSource, i) == '/' && i + 1 < pSource->iLength &&
		     __xuiCodeFoldByteAt(pSource, i + 1) == '*' ) {
			int iStartLine = iLine;
			i += 2;
			while ( i < pSource->iLength ) {
				if ( i + 1 < pSource->iLength &&
				     __xuiCodeFoldByteAt(pSource, i) == '*' &&
				     __xuiCodeFoldByteAt(pSource, i + 1) == '/' ) {
					i += 2;
					break;
				}
				if ( __xuiCodeFoldByteAt(pSource, i) == '\n' ) {
					iLine++;
					iLineStart = i + 1;
				}
				i++;
			}
			iRet = __xuiCodeFoldAddRange(pRanges, iRangeCapacity, pRangeCount, iStartLine, iLine, 0, XUI_CODE_FOLD_COMMENT);
			if ( iRet != XUI_OK ) return iRet;
			continue;
		}
		if ( __xuiCodeFoldByteAt(pSource, i) == '{' ) {
			if ( iStackCount >= XUI_CODE_FOLD_STACK_MAX ) {
				*pRangeCount = 0;
				return XUI_ERROR_LIMIT_EXCEEDED;
			}
			arrStack[iStackCount].iLine = iLine;
			arrStack[iStackCount].iLevel = iStackCount;
			arrStack[iStackCount].iFlags = 0;
			iStackCount++;
		} else if ( __xuiCodeFoldByteAt(pSource, i) == '}' && iStackCount > 0 ) {
			iStackCount--;
			iRet = __xuiCodeFoldAddRange(pRanges, iRangeCapacity, pRangeCount,
				arrStack[iStackCount].iLine, iLine, arrStack[iStackCount].iLevel, arrStack[iStackCount].iFlags);
			if ( iRet != XUI_OK ) return iRet;
		}
		i++;
	}
	return XUI_OK;
}

XUI_API int xuiCodeFoldCBuildRanges(const char* sText, int iTextSize, xui_code_fold_range_t* pRanges, int iRangeCapacity, int* pRangeCount)
{
	xui_code_fold_source_t tSource;

	if ( sText == NULL || iTextSize < -1 ) return XUI_ERROR_INVALID_ARGUMENT;
	tSource.sText = sText;
	tSource.pDocument = NULL;
	if ( iTextSize < 0 ) {
		size_t iLength = strlen(sText);
		if ( iLength > INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
		tSource.iLength = (int)iLength;
	} else {
		tSource.iLength = iTextSize;
	}
	return __xuiCodeFoldCBuildRanges(&tSource, pRanges, iRangeCapacity, pRangeCount);
}

XUI_API int xuiCodeFoldCBuildDocumentRanges(xui_code_document pDocument, xui_code_fold_range_t* pRanges, int iRangeCapacity, int* pRangeCount)
{
	xui_code_fold_source_t tSource;

	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tSource.sText = NULL;
	tSource.pDocument = pDocument;
	tSource.iLength = xuiCodeDocumentGetLength(pDocument);
	return __xuiCodeFoldCBuildRanges(&tSource, pRanges, iRangeCapacity, pRangeCount);
}

XUI_API int xuiCodeFoldBuildVisibleLines(int iLineCount, const xui_code_fold_range_t* pRanges, int iRangeCount, int* pVisibleLines, int iVisibleCapacity, int* pVisibleCount)
{
	xui_code_fold_hidden_t* pHidden;
	int iCollapsedCount;
	int iHiddenCount;
	int iMergedCount;
	int iHidden;
	int i;

	if ( iLineCount < 0 || iRangeCount < 0 || iVisibleCapacity < 0 ||
	     pVisibleCount == NULL || (iRangeCount > 0 && pRanges == NULL) ||
	     (iVisibleCapacity > 0 && pVisibleLines == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pVisibleCount = 0;
	pHidden = NULL;
	iHiddenCount = 0;
	iCollapsedCount = 0;
	for ( i = 0; i < iRangeCount; i++ ) {
		if ( !__xuiCodeFoldRangeValid(&pRanges[i]) ) return XUI_ERROR_INVALID_ARGUMENT;
		if ( (pRanges[i].iFlags & XUI_CODE_FOLD_COLLAPSED) != 0 &&
		     pRanges[i].iStartLine < iLineCount - 1 ) iCollapsedCount++;
	}
	if ( iCollapsedCount > 0 ) {
		if ( (size_t)iCollapsedCount > SIZE_MAX / sizeof(*pHidden) ) {
			return XUI_ERROR_LIMIT_EXCEEDED;
		}
		pHidden = (xui_code_fold_hidden_t*)xrtMalloc(
			(size_t)iCollapsedCount * sizeof(*pHidden));
		if ( pHidden == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < iRangeCount; i++ ) {
		int iEnd;
		if ( (pRanges[i].iFlags & XUI_CODE_FOLD_COLLAPSED) == 0 ||
		     pRanges[i].iStartLine >= iLineCount - 1 ) continue;
		iEnd = pRanges[i].iEndLine;
		if ( iEnd >= iLineCount ) iEnd = iLineCount - 1;
		pHidden[iHiddenCount].iStartLine = pRanges[i].iStartLine + 1;
		pHidden[iHiddenCount].iEndLine = iEnd;
		iHiddenCount++;
	}
	if ( iHiddenCount > 1 ) {
		qsort(pHidden, (size_t)iHiddenCount, sizeof(*pHidden),
			__xuiCodeFoldCompareHidden);
	}
	iMergedCount = 0;
	for ( i = 0; i < iHiddenCount; i++ ) {
		if ( iMergedCount == 0 ||
		     pHidden[i].iStartLine > pHidden[iMergedCount - 1].iEndLine + 1 ) {
			pHidden[iMergedCount++] = pHidden[i];
		} else if ( pHidden[i].iEndLine > pHidden[iMergedCount - 1].iEndLine ) {
			pHidden[iMergedCount - 1].iEndLine = pHidden[i].iEndLine;
		}
	}
	iHidden = 0;
	for ( i = 0; i < iLineCount; i++ ) {
		while ( iHidden < iMergedCount && i > pHidden[iHidden].iEndLine ) iHidden++;
		if ( iHidden < iMergedCount && i >= pHidden[iHidden].iStartLine ) continue;
		if ( (*pVisibleCount < iVisibleCapacity) && (pVisibleLines != NULL) ) {
			pVisibleLines[*pVisibleCount] = i;
		}
		(*pVisibleCount)++;
	}
	xrtFree(pHidden);
	return XUI_OK;
}

#endif
