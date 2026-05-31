#include "../xui.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void __xuiCodeSearchSetError(char* sError, int iCapacity, const char* sMessage)
{
	if ( (sError == NULL) || (iCapacity <= 0) ) return;
	if ( sMessage == NULL ) sMessage = "";
	snprintf(sError, (size_t)iCapacity, "%s", sMessage);
}

static int __xuiCodeSearchClamp(int iValue, int iMin, int iMax)
{
	if ( iValue < iMin ) return iMin;
	if ( iValue > iMax ) return iMax;
	return iValue;
}

static int __xuiCodeSearchIsWordChar(char c)
{
	unsigned char ch = (unsigned char)c;
	return isalnum(ch) || ch == '_';
}

static int __xuiCodeSearchIsWholeWord(const char* sText, int iLength, int iStart, int iEnd)
{
	if ( iStart > 0 && __xuiCodeSearchIsWordChar(sText[iStart - 1]) ) return 0;
	if ( iEnd < iLength && __xuiCodeSearchIsWordChar(sText[iEnd]) ) return 0;
	return 1;
}

static int __xuiCodeSearchCharEquals(char a, char b, uint32_t iFlags)
{
	if ( (iFlags & XUI_CODE_SEARCH_CASE_SENSITIVE) != 0 ) return a == b;
	return tolower((unsigned char)a) == tolower((unsigned char)b);
}

static int __xuiCodeSearchMatchAt(const char* sText, int iTextLength, const char* sPattern, int iPatternLength, int iOffset, uint32_t iFlags)
{
	int i;

	if ( iOffset < 0 || iOffset + iPatternLength > iTextLength ) return 0;
	for ( i = 0; i < iPatternLength; i++ ) {
		if ( !__xuiCodeSearchCharEquals(sText[iOffset + i], sPattern[i], iFlags) ) return 0;
	}
	if ( (iFlags & XUI_CODE_SEARCH_WHOLE_WORD) != 0 ) {
		return __xuiCodeSearchIsWholeWord(sText, iTextLength, iOffset, iOffset + iPatternLength);
	}
	return 1;
}

static int __xuiCodeSearchFindPlainInRange(const char* sText, int iTextLength, const char* sPattern, int iPatternLength, int iStart, int iEnd, uint32_t iFlags, xui_code_range_t* pRange)
{
	int i;

	if ( (iFlags & XUI_CODE_SEARCH_BACKWARD) != 0 ) {
		iStart = __xuiCodeSearchClamp(iStart, 0, iTextLength);
		iEnd = __xuiCodeSearchClamp(iEnd, 0, iTextLength);
		for ( i = iEnd - iPatternLength; i >= iStart; i-- ) {
			if ( __xuiCodeSearchMatchAt(sText, iTextLength, sPattern, iPatternLength, i, iFlags) ) {
				pRange->iStart = i;
				pRange->iEnd = i + iPatternLength;
				return XUI_OK;
			}
		}
	} else {
		iStart = __xuiCodeSearchClamp(iStart, 0, iTextLength);
		iEnd = __xuiCodeSearchClamp(iEnd, 0, iTextLength);
		for ( i = iStart; i + iPatternLength <= iEnd; i++ ) {
			if ( __xuiCodeSearchMatchAt(sText, iTextLength, sPattern, iPatternLength, i, iFlags) ) {
				pRange->iStart = i;
				pRange->iEnd = i + iPatternLength;
				return XUI_OK;
			}
		}
	}
	return XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiCodeSearchFindPlain(xui_code_document pDocument, const char* sPattern, int iStartOffset, uint32_t iFlags, xui_code_range_t* pRange)
{
	const char* sText;
	int iTextLength;
	int iPatternLength;
	int iRet;

	if ( (pDocument == NULL) || (sPattern == NULL) || (pRange == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iPatternLength = (int)strlen(sPattern);
	if ( iPatternLength <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	sText = xuiCodeDocumentGetText(pDocument);
	iTextLength = xuiCodeDocumentGetLength(pDocument);
	iStartOffset = __xuiCodeSearchClamp(iStartOffset, 0, iTextLength);
	memset(pRange, 0, sizeof(*pRange));
	if ( (iFlags & XUI_CODE_SEARCH_BACKWARD) != 0 ) {
		iRet = __xuiCodeSearchFindPlainInRange(sText, iTextLength, sPattern, iPatternLength, 0, iStartOffset, iFlags, pRange);
		if ( iRet == XUI_OK || (iFlags & XUI_CODE_SEARCH_WRAP) == 0 ) return iRet;
		return __xuiCodeSearchFindPlainInRange(sText, iTextLength, sPattern, iPatternLength, iStartOffset, iTextLength, iFlags, pRange);
	}
	iRet = __xuiCodeSearchFindPlainInRange(sText, iTextLength, sPattern, iPatternLength, iStartOffset, iTextLength, iFlags, pRange);
	if ( iRet == XUI_OK || (iFlags & XUI_CODE_SEARCH_WRAP) == 0 ) return iRet;
	return __xuiCodeSearchFindPlainInRange(sText, iTextLength, sPattern, iPatternLength, 0, iStartOffset, iFlags, pRange);
}

XUI_API int xuiCodeSearchFindPlainRange(xui_code_document pDocument, const char* sPattern, int iStartOffset, int iRangeStart, int iRangeEnd, uint32_t iFlags, xui_code_range_t* pRange)
{
	const char* sText;
	int iTextLength;
	int iPatternLength;
	int iRet;

	if ( (pDocument == NULL) || (sPattern == NULL) || (pRange == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iPatternLength = (int)strlen(sPattern);
	if ( iPatternLength <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	sText = xuiCodeDocumentGetText(pDocument);
	iTextLength = xuiCodeDocumentGetLength(pDocument);
	iRangeStart = __xuiCodeSearchClamp(iRangeStart, 0, iTextLength);
	iRangeEnd = __xuiCodeSearchClamp(iRangeEnd, 0, iTextLength);
	if ( iRangeEnd < iRangeStart ) {
		int iSwap = iRangeStart;
		iRangeStart = iRangeEnd;
		iRangeEnd = iSwap;
	}
	iStartOffset = __xuiCodeSearchClamp(iStartOffset, iRangeStart, iRangeEnd);
	memset(pRange, 0, sizeof(*pRange));
	if ( (iFlags & XUI_CODE_SEARCH_BACKWARD) != 0 ) {
		iRet = __xuiCodeSearchFindPlainInRange(sText, iTextLength, sPattern, iPatternLength, iRangeStart, iStartOffset, iFlags, pRange);
		if ( iRet == XUI_OK || (iFlags & XUI_CODE_SEARCH_WRAP) == 0 ) return iRet;
		return __xuiCodeSearchFindPlainInRange(sText, iTextLength, sPattern, iPatternLength, iStartOffset, iRangeEnd, iFlags, pRange);
	}
	iRet = __xuiCodeSearchFindPlainInRange(sText, iTextLength, sPattern, iPatternLength, iStartOffset, iRangeEnd, iFlags, pRange);
	if ( iRet == XUI_OK || (iFlags & XUI_CODE_SEARCH_WRAP) == 0 ) return iRet;
	return __xuiCodeSearchFindPlainInRange(sText, iTextLength, sPattern, iPatternLength, iRangeStart, iStartOffset, iFlags, pRange);
}

#ifndef XRT_NO_REGEX
static int __xuiCodeSearchCreateRegex(xregex** ppRegex, const char* sPattern, uint32_t iFlags, char* sError, int iErrorCapacity)
{
	xregexbuilder* pBuilder;
	xregexflags iRegexFlags;
	int iRet;

	*ppRegex = NULL;
	pBuilder = NULL;
	iRet = xrtRegexBuilderCreate(&pBuilder, sPattern, strlen(sPattern), NULL);
	if ( iRet != 0 || pBuilder == NULL ) {
		__xuiCodeSearchSetError(sError, iErrorCapacity, "regex builder create failed");
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRegexFlags = 0;
	if ( (iFlags & XUI_CODE_SEARCH_CASE_SENSITIVE) == 0 ) iRegexFlags |= XRT_REGEX_FLAG_INSENSITIVE;
	if ( (iFlags & XUI_CODE_SEARCH_MULTILINE) != 0 ) iRegexFlags |= XRT_REGEX_FLAG_MULTILINE;
	if ( (iFlags & XUI_CODE_SEARCH_DOT_NEWLINE) != 0 ) iRegexFlags |= XRT_REGEX_FLAG_DOTNEWLINE;
	xrtRegexBuilderSetFlags(pBuilder, iRegexFlags);
	iRet = xrtRegexCreateFromBuilder(ppRegex, pBuilder, NULL);
	xrtRegexBuilderDestroy(pBuilder);
	if ( iRet != 0 || *ppRegex == NULL ) {
		__xuiCodeSearchSetError(sError, iErrorCapacity, "regex compile failed");
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return XUI_OK;
}

static int __xuiCodeSearchFillRegexResult(xregex* pRegex, const char* sText, int iTextLength, xregexspan tSpan, uint32_t iFlags, xui_code_search_result_t* pResult)
{
	xregexspan arrCaptures[XUI_CODE_SEARCH_MAX_CAPTURES];
	uint32_t arrMatched[XUI_CODE_SEARCH_MAX_CAPTURES];
	uint32_t iCaptureCount;
	uint32_t i;
	int iRet;

	if ( (iFlags & XUI_CODE_SEARCH_WHOLE_WORD) != 0 ) {
		if ( !__xuiCodeSearchIsWholeWord(sText, iTextLength, (int)tSpan.iBegin, (int)tSpan.iEnd) ) return XUI_ERROR_UNSUPPORTED;
	}
	memset(pResult, 0, sizeof(*pResult));
	pResult->iSize = sizeof(*pResult);
	pResult->iStart = (int)tSpan.iBegin;
	pResult->iEnd = (int)tSpan.iEnd;
	iCaptureCount = xrtRegexCaptureCount(pRegex);
	if ( iCaptureCount > XUI_CODE_SEARCH_MAX_CAPTURES ) iCaptureCount = XUI_CODE_SEARCH_MAX_CAPTURES;
	memset(arrCaptures, 0, sizeof(arrCaptures));
	memset(arrMatched, 0, sizeof(arrMatched));
	iRet = xrtRegexWhichCaptures(pRegex, sText + tSpan.iBegin, (size_t)(tSpan.iEnd - tSpan.iBegin), arrCaptures, arrMatched, iCaptureCount);
	if ( iRet == 1 ) {
		pResult->iCaptureCount = (int)iCaptureCount;
		for ( i = 0; i < iCaptureCount; i++ ) {
			if ( arrMatched[i] ) {
				pResult->arrCaptures[i].iStart = (int)(tSpan.iBegin + arrCaptures[i].iBegin);
				pResult->arrCaptures[i].iEnd = (int)(tSpan.iBegin + arrCaptures[i].iEnd);
			} else {
				pResult->arrCaptures[i].iStart = -1;
				pResult->arrCaptures[i].iEnd = -1;
			}
		}
	}
	return XUI_OK;
}

static int __xuiCodeSearchFindRegexForward(xregex* pRegex, const char* sText, int iTextLength, int iStartOffset, int iEndOffset, uint32_t iFlags, xui_code_search_result_t* pResult)
{
	xregexspan tSpan;
	int iPos;
	int iRet;

	iPos = iStartOffset;
	while ( iPos <= iEndOffset ) {
		iRet = xrtRegexFindAt(pRegex, sText, (size_t)iTextLength, (size_t)iPos, &tSpan);
		if ( iRet != 1 || (int)tSpan.iBegin > iEndOffset ) return XUI_ERROR_UNSUPPORTED;
		if ( __xuiCodeSearchFillRegexResult(pRegex, sText, iTextLength, tSpan, iFlags, pResult) == XUI_OK ) return XUI_OK;
		iPos = ((int)tSpan.iEnd > iPos) ? (int)tSpan.iEnd : iPos + 1;
	}
	return XUI_ERROR_UNSUPPORTED;
}

static int __xuiCodeSearchFindRegexBackward(xregex* pRegex, const char* sText, int iTextLength, int iStartOffset, int iEndOffset, uint32_t iFlags, xui_code_search_result_t* pResult)
{
	xui_code_search_result_t tLast;
	xui_code_search_result_t tCurrent;
	int iPos;
	int iFound;

	iFound = 0;
	iPos = iStartOffset;
	while ( __xuiCodeSearchFindRegexForward(pRegex, sText, iTextLength, iPos, iEndOffset, iFlags & ~XUI_CODE_SEARCH_BACKWARD, &tCurrent) == XUI_OK ) {
		if ( tCurrent.iEnd > iEndOffset ) break;
		tLast = tCurrent;
		iFound = 1;
		iPos = (tCurrent.iEnd > tCurrent.iStart) ? tCurrent.iEnd : tCurrent.iStart + 1;
	}
	if ( !iFound ) return XUI_ERROR_UNSUPPORTED;
	*pResult = tLast;
	return XUI_OK;
}
#endif

XUI_API int xuiCodeSearchFindRegex(xui_code_document pDocument, const char* sPattern, int iStartOffset, uint32_t iFlags, xui_code_search_result_t* pResult, char* sError, int iErrorCapacity)
{
#ifdef XRT_NO_REGEX
	(void)pDocument;
	(void)sPattern;
	(void)iStartOffset;
	(void)iFlags;
	(void)pResult;
	__xuiCodeSearchSetError(sError, iErrorCapacity, "XRT regex support is disabled");
	return XUI_ERROR_UNSUPPORTED;
#else
	xregex* pRegex;
	const char* sText;
	int iTextLength;
	int iRet;

	if ( (pDocument == NULL) || (sPattern == NULL) || (sPattern[0] == '\0') || (pResult == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCodeSearchCreateRegex(&pRegex, sPattern, iFlags, sError, iErrorCapacity);
	if ( iRet != XUI_OK ) return iRet;
	sText = xuiCodeDocumentGetText(pDocument);
	iTextLength = xuiCodeDocumentGetLength(pDocument);
	iStartOffset = __xuiCodeSearchClamp(iStartOffset, 0, iTextLength);
	memset(pResult, 0, sizeof(*pResult));
	if ( (iFlags & XUI_CODE_SEARCH_BACKWARD) != 0 ) {
		iRet = __xuiCodeSearchFindRegexBackward(pRegex, sText, iTextLength, 0, iStartOffset, iFlags, pResult);
		if ( iRet != XUI_OK && (iFlags & XUI_CODE_SEARCH_WRAP) != 0 ) {
			iRet = __xuiCodeSearchFindRegexBackward(pRegex, sText, iTextLength, iStartOffset, iTextLength, iFlags, pResult);
		}
	} else {
		iRet = __xuiCodeSearchFindRegexForward(pRegex, sText, iTextLength, iStartOffset, iTextLength, iFlags, pResult);
		if ( iRet != XUI_OK && (iFlags & XUI_CODE_SEARCH_WRAP) != 0 ) {
			iRet = __xuiCodeSearchFindRegexForward(pRegex, sText, iTextLength, 0, iStartOffset, iFlags, pResult);
		}
	}
	xrtRegexDestroy(pRegex);
	__xuiCodeSearchSetError(sError, iErrorCapacity, (iRet == XUI_OK) ? "" : "not found");
	return iRet;
#endif
}

XUI_API int xuiCodeSearchReplaceAllPlain(xui_code_document pDocument, const char* sPattern, const char* sReplacement, uint32_t iFlags, int* pReplaceCount)
{
	xui_code_range_t tRange;
	int iCount;
	int iOffset;
	int iReplacementLength;
	int iRet;

	if ( (pDocument == NULL) || (sPattern == NULL) || (sPattern[0] == '\0') || (sReplacement == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iCount = 0;
	iOffset = 0;
	iReplacementLength = (int)strlen(sReplacement);
	iFlags &= ~XUI_CODE_SEARCH_BACKWARD;
	iFlags &= ~XUI_CODE_SEARCH_WRAP;
	iRet = xuiCodeDocumentBeginEdit(pDocument);
	if ( iRet != XUI_OK ) return iRet;
	while ( xuiCodeSearchFindPlain(pDocument, sPattern, iOffset, iFlags, &tRange) == XUI_OK ) {
		iRet = xuiCodeDocumentReplace(pDocument, tRange.iStart, tRange.iEnd, sReplacement);
		if ( iRet != XUI_OK ) break;
		iCount++;
		iOffset = tRange.iStart + iReplacementLength;
	}
	(void)xuiCodeDocumentEndEdit(pDocument);
	if ( pReplaceCount != NULL ) *pReplaceCount = iCount;
	return (iRet == XUI_OK) ? XUI_OK : iRet;
}

#ifndef XRT_NO_REGEX
static int __xuiCodeSearchAppend(char** psBuffer, int* pLength, int* pCapacity, const char* sText, int iTextLength)
{
	char* sNew;
	int iNewCapacity;

	if ( iTextLength <= 0 ) return XUI_OK;
	if ( *pLength + iTextLength + 1 > *pCapacity ) {
		iNewCapacity = (*pCapacity > 0) ? *pCapacity : 64;
		while ( *pLength + iTextLength + 1 > iNewCapacity ) iNewCapacity *= 2;
		sNew = (char*)xrtRealloc(*psBuffer, (size_t)iNewCapacity);
		if ( sNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		*psBuffer = sNew;
		*pCapacity = iNewCapacity;
	}
	memcpy(*psBuffer + *pLength, sText, (size_t)iTextLength);
	*pLength += iTextLength;
	(*psBuffer)[*pLength] = '\0';
	return XUI_OK;
}

static int __xuiCodeSearchAppendReplacement(char** psBuffer, int* pLength, int* pCapacity, const char* sReplacement, const char* sText, const xui_code_search_result_t* pResult)
{
	int i;
	int iCapture;
	int iRet;

	for ( i = 0; sReplacement[i] != '\0'; i++ ) {
		if ( sReplacement[i] == '$' && sReplacement[i + 1] >= '0' && sReplacement[i + 1] <= '9' ) {
			iCapture = sReplacement[i + 1] - '0';
			i++;
			if ( iCapture < pResult->iCaptureCount && pResult->arrCaptures[iCapture].iStart >= 0 ) {
				iRet = __xuiCodeSearchAppend(psBuffer, pLength, pCapacity, sText + pResult->arrCaptures[iCapture].iStart, pResult->arrCaptures[iCapture].iEnd - pResult->arrCaptures[iCapture].iStart);
				if ( iRet != XUI_OK ) return iRet;
			}
			continue;
		}
		if ( sReplacement[i] == '\\' && sReplacement[i + 1] >= '0' && sReplacement[i + 1] <= '9' ) {
			iCapture = sReplacement[i + 1] - '0';
			i++;
			if ( iCapture < pResult->iCaptureCount && pResult->arrCaptures[iCapture].iStart >= 0 ) {
				iRet = __xuiCodeSearchAppend(psBuffer, pLength, pCapacity, sText + pResult->arrCaptures[iCapture].iStart, pResult->arrCaptures[iCapture].iEnd - pResult->arrCaptures[iCapture].iStart);
				if ( iRet != XUI_OK ) return iRet;
			}
			continue;
		}
		iRet = __xuiCodeSearchAppend(psBuffer, pLength, pCapacity, &sReplacement[i], 1);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}
#endif

XUI_API int xuiCodeSearchReplaceAllRegex(xui_code_document pDocument, const char* sPattern, const char* sReplacement, uint32_t iFlags, int* pReplaceCount, char* sError, int iErrorCapacity)
{
#ifdef XRT_NO_REGEX
	(void)pDocument;
	(void)sPattern;
	(void)sReplacement;
	(void)iFlags;
	(void)pReplaceCount;
	__xuiCodeSearchSetError(sError, iErrorCapacity, "XRT regex support is disabled");
	return XUI_ERROR_UNSUPPORTED;
#else
	xregex* pRegex;
	xui_code_search_result_t tResult;
	const char* sText;
	char* sOutput;
	int iTextLength;
	int iLength;
	int iCapacity;
	int iPos;
	int iCount;
	int iRet;

	if ( (pDocument == NULL) || (sPattern == NULL) || (sPattern[0] == '\0') || (sReplacement == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiCodeSearchCreateRegex(&pRegex, sPattern, iFlags, sError, iErrorCapacity);
	if ( iRet != XUI_OK ) return iRet;
	sText = xuiCodeDocumentGetText(pDocument);
	iTextLength = xuiCodeDocumentGetLength(pDocument);
	sOutput = NULL;
	iLength = 0;
	iCapacity = 0;
	iPos = 0;
	iCount = 0;
	iFlags &= ~XUI_CODE_SEARCH_BACKWARD;
	iFlags &= ~XUI_CODE_SEARCH_WRAP;
	while ( __xuiCodeSearchFindRegexForward(pRegex, sText, iTextLength, iPos, iTextLength, iFlags, &tResult) == XUI_OK ) {
		iRet = __xuiCodeSearchAppend(&sOutput, &iLength, &iCapacity, sText + iPos, tResult.iStart - iPos);
		if ( iRet == XUI_OK ) iRet = __xuiCodeSearchAppendReplacement(&sOutput, &iLength, &iCapacity, sReplacement, sText, &tResult);
		if ( iRet != XUI_OK ) goto cleanup;
		iCount++;
		iPos = (tResult.iEnd > tResult.iStart) ? tResult.iEnd : tResult.iStart + 1;
	}
	iRet = __xuiCodeSearchAppend(&sOutput, &iLength, &iCapacity, sText + iPos, iTextLength - iPos);
	if ( iRet != XUI_OK ) goto cleanup;
	iRet = xuiCodeDocumentReplace(pDocument, 0, iTextLength, (sOutput != NULL) ? sOutput : "");
	if ( pReplaceCount != NULL ) *pReplaceCount = iCount;
	__xuiCodeSearchSetError(sError, iErrorCapacity, (iRet == XUI_OK) ? "" : "replace failed");

cleanup:
	xrtRegexDestroy(pRegex);
	xrtFree(sOutput);
	return iRet;
#endif
}

static int __xuiCodeSearchIndicatorId(int iIndicator)
{
	return (iIndicator > 0) ? iIndicator : XUI_CODE_INDICATOR_SEARCH_RESULT;
}

XUI_API int xuiCodeSearchClearResultIndicators(xui_code_annotation_store pStore, int iIndicator)
{
	if ( pStore == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndicator = __xuiCodeSearchIndicatorId(iIndicator);
	return xuiCodeAnnotationClearIndicators(pStore, iIndicator);
}

XUI_API int xuiCodeSearchMarkAllPlain(xui_code_document pDocument, xui_code_annotation_store pStore, const char* sPattern, uint32_t iFlags, int iIndicator, int* pMatchCount)
{
	xui_code_range_t tRange;
	int iOffset;
	int iCount;
	int iPatternLength;
	int iRet;

	if ( (pDocument == NULL) || (pStore == NULL) || (sPattern == NULL) || (sPattern[0] == '\0') ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndicator = __xuiCodeSearchIndicatorId(iIndicator);
	(void)xuiCodeAnnotationClearIndicators(pStore, iIndicator);
	iFlags &= ~XUI_CODE_SEARCH_BACKWARD;
	iFlags &= ~XUI_CODE_SEARCH_WRAP;
	iPatternLength = (int)strlen(sPattern);
	iOffset = 0;
	iCount = 0;
	while ( xuiCodeSearchFindPlain(pDocument, sPattern, iOffset, iFlags, &tRange) == XUI_OK ) {
		iRet = xuiCodeAnnotationSetIndicator(pStore, iIndicator, XUI_CODE_INDICATOR_BACKGROUND, tRange.iStart, tRange.iEnd, 0u, 0u);
		if ( iRet != XUI_OK ) return iRet;
		iCount++;
		iOffset = tRange.iEnd;
		if ( iOffset <= tRange.iStart ) iOffset = tRange.iStart + iPatternLength;
	}
	if ( pMatchCount != NULL ) *pMatchCount = iCount;
	return XUI_OK;
}

XUI_API int xuiCodeSearchMarkAllRegex(xui_code_document pDocument, xui_code_annotation_store pStore, const char* sPattern, uint32_t iFlags, int iIndicator, int* pMatchCount, char* sError, int iErrorCapacity)
{
	xui_code_search_result_t tResult;
	int iOffset;
	int iCount;
	int iRet;

	if ( (pDocument == NULL) || (pStore == NULL) || (sPattern == NULL) || (sPattern[0] == '\0') ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndicator = __xuiCodeSearchIndicatorId(iIndicator);
	(void)xuiCodeAnnotationClearIndicators(pStore, iIndicator);
	iFlags &= ~XUI_CODE_SEARCH_BACKWARD;
	iFlags &= ~XUI_CODE_SEARCH_WRAP;
	iOffset = 0;
	iCount = 0;
	while ( xuiCodeSearchFindRegex(pDocument, sPattern, iOffset, iFlags, &tResult, sError, iErrorCapacity) == XUI_OK ) {
		if ( tResult.iEnd > tResult.iStart ) {
			iRet = xuiCodeAnnotationSetIndicator(pStore, iIndicator, XUI_CODE_INDICATOR_BACKGROUND, tResult.iStart, tResult.iEnd, 0u, 0u);
			if ( iRet != XUI_OK ) return iRet;
			iCount++;
			iOffset = tResult.iEnd;
		} else {
			iOffset = tResult.iStart + 1;
		}
		if ( iOffset > xuiCodeDocumentGetLength(pDocument) ) break;
	}
	__xuiCodeSearchSetError(sError, iErrorCapacity, "");
	if ( pMatchCount != NULL ) *pMatchCount = iCount;
	return XUI_OK;
}
