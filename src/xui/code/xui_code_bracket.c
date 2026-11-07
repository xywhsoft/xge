#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT

#include "xui_internal.h"
#include "xui_code_bracket_internal.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

static int __xuiCodeBracketIsOpen(char iByte)
{
	return iByte == '(' || iByte == '[' || iByte == '{';
}

static int __xuiCodeBracketIsClose(char iByte)
{
	return iByte == ')' || iByte == ']' || iByte == '}';
}

static char __xuiCodeBracketPair(char iByte)
{
	switch ( iByte ) {
	case '(': return ')';
	case '[': return ']';
	case '{': return '}';
	case ')': return '(';
	case ']': return '[';
	case '}': return '{';
	default: return 0;
	}
}

static void __xuiCodeBracketClearSearch(xui_internal_code_bracket_state_t* pState)
{
	if ( pState == NULL ) return;
	pState->iDocumentVersion = 0u;
	pState->iCaretOffset = -1;
	pState->iDocumentLength = 0;
	pState->iSourceOffset = -1;
	pState->iMatchOffset = -1;
	pState->iScanOffset = 0;
	pState->iDirection = 0;
	pState->iStatus = XUI_CODE_BRACKET_NONE;
	pState->bInitialized = 0;
	pState->bHasLanguage = 0;
	pState->iLanguageLexerType = 0;
	pState->onLanguageLex = NULL;
	pState->pLanguageRegexRules = NULL;
	pState->iLanguageRegexRuleCount = 0;
	pState->pLanguageUser = NULL;
	pState->iSource = 0;
	pState->iStackCount = 0;
}

void xuiInternalCodeBracketReset(xui_internal_code_bracket_state_t* pState)
{
	__xuiCodeBracketClearSearch(pState);
}

void xuiInternalCodeBracketDestroy(xui_internal_code_bracket_state_t* pState)
{
	if ( pState == NULL ) return;
	xrtFree(pState->pStack);
	xrtFree(pState->pTokens);
	xrtFree(pState->sBytes);
	memset(pState, 0, sizeof(*pState));
}

static int __xuiCodeBracketReserveStack(
	xui_internal_code_bracket_state_t* pState, int iCapacity)
{
	char* pNew;
	int iNewCapacity;

	if ( pState == NULL || iCapacity < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pState->iStackCapacity ) return XUI_OK;
	iNewCapacity = (pState->iStackCapacity > 0) ? pState->iStackCapacity : 32;
	while ( iNewCapacity < iCapacity ) {
		if ( iNewCapacity > INT_MAX / 2 ) return XUI_ERROR_LIMIT_EXCEEDED;
		iNewCapacity *= 2;
	}
	pNew = (char*)xrtRealloc(pState->pStack, (size_t)iNewCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pState->pStack = pNew;
	pState->iStackCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiCodeBracketPush(xui_internal_code_bracket_state_t* pState,
	char iExpected)
{
	int iRet;

	iRet = __xuiCodeBracketReserveStack(pState, pState->iStackCount + 1);
	if ( iRet != XUI_OK ) return iRet;
	pState->pStack[pState->iStackCount++] = iExpected;
	return XUI_OK;
}

static int __xuiCodeBracketReserveTokens(
	xui_internal_code_bracket_state_t* pState, int iCapacity)
{
	xui_code_token_t* pNew;
	int iNewCapacity;

	if ( pState == NULL || iCapacity < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pState->iTokenCapacity ) return XUI_OK;
	iNewCapacity = (pState->iTokenCapacity > 0) ? pState->iTokenCapacity : 256;
	while ( iNewCapacity < iCapacity ) {
		if ( iNewCapacity > INT_MAX / 2 ) return XUI_ERROR_LIMIT_EXCEEDED;
		iNewCapacity *= 2;
	}
	if ( (size_t)iNewCapacity > SIZE_MAX / sizeof(*pNew) ) {
		return XUI_ERROR_LIMIT_EXCEEDED;
	}
	pNew = (xui_code_token_t*)xrtRealloc(pState->pTokens,
		(size_t)iNewCapacity * sizeof(*pNew));
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pState->pTokens = pNew;
	pState->iTokenCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiCodeBracketReserveBytes(
	xui_internal_code_bracket_state_t* pState, int iCapacity)
{
	char* pNew;

	if ( pState == NULL || iCapacity < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pState->iByteCapacity ) return XUI_OK;
	pNew = (char*)xrtRealloc(pState->sBytes, (size_t)iCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pState->sBytes = pNew;
	pState->iByteCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiCodeBracketLexRange(
	xui_internal_code_bracket_state_t* pState,
	xui_code_document pDocument, const xui_code_language_t* pLanguage,
	int iStartOffset, int iEndOffset, int* pTokenCount)
{
	char sError[160];
	int iStartLine;
	int iEndLine;
	int iProbe;
	int iCount;
	int iRet;
	int iAttempt;

	if ( pState == NULL || pDocument == NULL || pLanguage == NULL ||
	     pTokenCount == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pTokenCount = 0;
	if ( iEndOffset <= iStartOffset ) return XUI_OK;
	iRet = __xuiCodeBracketReserveTokens(pState, 256);
	if ( iRet != XUI_OK ) return iRet;
	for ( iAttempt = 0; iAttempt < 2; iAttempt++ ) {
		iCount = 0;
		if ( xuiInternalCodeLanguageUsesBuiltinC(pLanguage) ) {
			iRet = xuiCodeLexerCTokenizeDocumentRange(pDocument,
				iStartOffset, iEndOffset, pState->pTokens,
				pState->iTokenCapacity, &iCount);
		} else {
			iProbe = (iEndOffset > iStartOffset) ? iEndOffset - 1 : iEndOffset;
			iRet = xuiCodeDocumentOffsetToLineColumn(pDocument,
				iStartOffset, &iStartLine, NULL);
			if ( iRet == XUI_OK ) {
				iRet = xuiCodeDocumentOffsetToLineColumn(pDocument,
					iProbe, &iEndLine, NULL);
			}
			sError[0] = '\0';
			if ( iRet == XUI_OK ) {
				iRet = xuiCodeLanguageLex(pLanguage, pDocument,
					iStartLine, iEndLine, pState->pTokens,
					pState->iTokenCapacity, &iCount, sError,
					(int)sizeof(sError));
			}
		}
		if ( iRet != XUI_OK ) return iRet;
		if ( iCount <= pState->iTokenCapacity ) break;
		if ( iAttempt != 0 ) return XUI_ERROR_INVALID_STATE;
		iRet = __xuiCodeBracketReserveTokens(pState, iCount);
		if ( iRet != XUI_OK ) return iRet;
	}
	*pTokenCount = iCount;
	return XUI_OK;
}

static int __xuiCodeBracketTokenContains(
	xui_internal_code_bracket_state_t* pState,
	xui_code_document pDocument, const xui_code_language_t* pLanguage,
	int iOffset, int* pContains)
{
	int iCount;
	int i;
	int iRet;

	if ( pContains == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pContains = 0;
	iRet = __xuiCodeBracketLexRange(pState, pDocument, pLanguage,
		iOffset, iOffset + 1, &iCount);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < iCount; i++ ) {
		if ( pState->pTokens[i].iKind == XUI_CODE_TOKEN_BRACE &&
		     pState->pTokens[i].iStartOffset <= iOffset &&
		     pState->pTokens[i].iEndOffset > iOffset ) {
			*pContains = 1;
			break;
		}
	}
	return XUI_OK;
}

static int __xuiCodeBracketCandidate(
	xui_internal_code_bracket_state_t* pState,
	xui_code_document pDocument, const xui_code_language_t* pLanguage,
	int iOffset, char* pByte, int* pAccepted)
{
	int iRet;
	int bTokenBrace;

	if ( pByte == NULL || pAccepted == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pAccepted = 0;
	if ( iOffset < 0 || iOffset >= xuiCodeDocumentGetLength(pDocument) ) {
		return XUI_OK;
	}
	iRet = xuiCodeDocumentGetByte(pDocument, iOffset, pByte);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiCodeBracketIsOpen(*pByte) &&
	     !__xuiCodeBracketIsClose(*pByte) ) return XUI_OK;
	if ( pLanguage == NULL ) {
		*pAccepted = 1;
		return XUI_OK;
	}
	iRet = __xuiCodeBracketTokenContains(pState, pDocument, pLanguage,
		iOffset, &bTokenBrace);
	if ( iRet != XUI_OK ) return iRet;
	*pAccepted = bTokenBrace;
	return XUI_OK;
}

static int __xuiCodeBracketBegin(xui_internal_code_bracket_state_t* pState,
	xui_code_document pDocument, const xui_code_language_t* pLanguage,
	int iCaretOffset)
{
	char iByte;
	int iOffset;
	int bAccepted;
	int iRet;

	pState->iDocumentVersion = xuiCodeDocumentGetVersion(pDocument);
	pState->iDocumentLength = xuiCodeDocumentGetLength(pDocument);
	pState->iCaretOffset = iCaretOffset;
	pState->iSourceOffset = -1;
	pState->iMatchOffset = -1;
	pState->iStatus = XUI_CODE_BRACKET_NONE;
	pState->iStackCount = 0;
	pState->bInitialized = 1;
	pState->bHasLanguage = pLanguage != NULL;
	if ( pLanguage != NULL ) {
		pState->iLanguageLexerType = pLanguage->iLexerType;
		pState->onLanguageLex = pLanguage->onLex;
		pState->pLanguageRegexRules = pLanguage->pRegexRules;
		pState->iLanguageRegexRuleCount = pLanguage->iRegexRuleCount;
		pState->pLanguageUser = pLanguage->pUser;
	}

	for ( iOffset = iCaretOffset - 1; iOffset <= iCaretOffset; iOffset++ ) {
		if ( iOffset < 0 || iOffset >= pState->iDocumentLength ) continue;
		iRet = __xuiCodeBracketCandidate(pState, pDocument, pLanguage,
			iOffset, &iByte, &bAccepted);
		if ( iRet != XUI_OK ) return iRet;
		if ( !bAccepted ) continue;
		pState->iSourceOffset = iOffset;
		pState->iSource = iByte;
		break;
	}
	if ( pState->iSourceOffset < 0 ) return XUI_OK;
	pState->iDirection = __xuiCodeBracketIsOpen(pState->iSource) ? 1 : -1;
	pState->iScanOffset = pState->iSourceOffset +
		((pState->iDirection > 0) ? 1 : 0);
	pState->iStatus = XUI_CODE_BRACKET_PENDING;
	return __xuiCodeBracketPush(pState,
		__xuiCodeBracketPair(pState->iSource));
}

static int __xuiCodeBracketConsume(
	xui_internal_code_bracket_state_t* pState, char iByte, int iOffset)
{
	char iExpected;

	if ( pState->iDirection > 0 ) {
		if ( __xuiCodeBracketIsOpen(iByte) ) {
			return __xuiCodeBracketPush(pState,
				__xuiCodeBracketPair(iByte));
		}
		if ( !__xuiCodeBracketIsClose(iByte) ) return XUI_OK;
	} else {
		if ( __xuiCodeBracketIsClose(iByte) ) {
			return __xuiCodeBracketPush(pState,
				__xuiCodeBracketPair(iByte));
		}
		if ( !__xuiCodeBracketIsOpen(iByte) ) return XUI_OK;
	}
	if ( pState->iStackCount <= 0 ) {
		pState->iStatus = XUI_CODE_BRACKET_UNMATCHED;
		return XUI_OK;
	}
	iExpected = pState->pStack[pState->iStackCount - 1];
	if ( iExpected != iByte ) {
		pState->iStatus = XUI_CODE_BRACKET_UNMATCHED;
		return XUI_OK;
	}
	pState->iStackCount--;
	if ( pState->iStackCount == 0 ) {
		pState->iMatchOffset = iOffset;
		pState->iStatus = XUI_CODE_BRACKET_MATCHED;
	}
	return XUI_OK;
}

static int __xuiCodeBracketScanPlain(
	xui_internal_code_bracket_state_t* pState, xui_code_document pDocument,
	int iStartOffset, int iEndOffset)
{
	int iLength;
	int i;
	int iRet;

	iLength = iEndOffset - iStartOffset;
	if ( iLength <= 0 ) return XUI_OK;
	iRet = __xuiCodeBracketReserveBytes(pState, iLength + 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeDocumentCopyRange(pDocument, iStartOffset, iEndOffset,
		pState->sBytes, pState->iByteCapacity, NULL);
	if ( iRet != XUI_OK ) return iRet;
	if ( pState->iDirection > 0 ) {
		for ( i = 0; i < iLength &&
		     pState->iStatus == XUI_CODE_BRACKET_PENDING; i++ ) {
			iRet = __xuiCodeBracketConsume(pState, pState->sBytes[i],
				iStartOffset + i);
			if ( iRet != XUI_OK ) return iRet;
		}
	} else {
		for ( i = iLength - 1; i >= 0 &&
		     pState->iStatus == XUI_CODE_BRACKET_PENDING; i-- ) {
			iRet = __xuiCodeBracketConsume(pState, pState->sBytes[i],
				iStartOffset + i);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiCodeBracketScanTokens(
	xui_internal_code_bracket_state_t* pState, xui_code_document pDocument,
	const xui_code_language_t* pLanguage, int iStartOffset, int iEndOffset)
{
	int iCount;
	int iToken;
	int iStart;
	int iEnd;
	int iOffset;
	char iByte;
	int iRet;

	iRet = __xuiCodeBracketLexRange(pState, pDocument, pLanguage,
		iStartOffset, iEndOffset, &iCount);
	if ( iRet != XUI_OK ) return iRet;
	if ( pState->iDirection > 0 ) {
		for ( iToken = 0; iToken < iCount &&
		     pState->iStatus == XUI_CODE_BRACKET_PENDING; iToken++ ) {
			if ( pState->pTokens[iToken].iKind != XUI_CODE_TOKEN_BRACE ) continue;
			iStart = pState->pTokens[iToken].iStartOffset;
			iEnd = pState->pTokens[iToken].iEndOffset;
			if ( iStart < iStartOffset ) iStart = iStartOffset;
			if ( iEnd > iEndOffset ) iEnd = iEndOffset;
			for ( iOffset = iStart; iOffset < iEnd &&
			     pState->iStatus == XUI_CODE_BRACKET_PENDING; iOffset++ ) {
				iRet = xuiCodeDocumentGetByte(pDocument, iOffset, &iByte);
				if ( iRet != XUI_OK ) return iRet;
				iRet = __xuiCodeBracketConsume(pState, iByte, iOffset);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	} else {
		for ( iToken = iCount - 1; iToken >= 0 &&
		     pState->iStatus == XUI_CODE_BRACKET_PENDING; iToken-- ) {
			if ( pState->pTokens[iToken].iKind != XUI_CODE_TOKEN_BRACE ) continue;
			iStart = pState->pTokens[iToken].iStartOffset;
			iEnd = pState->pTokens[iToken].iEndOffset;
			if ( iStart < iStartOffset ) iStart = iStartOffset;
			if ( iEnd > iEndOffset ) iEnd = iEndOffset;
			for ( iOffset = iEnd - 1; iOffset >= iStart &&
			     pState->iStatus == XUI_CODE_BRACKET_PENDING; iOffset-- ) {
				iRet = xuiCodeDocumentGetByte(pDocument, iOffset, &iByte);
				if ( iRet != XUI_OK ) return iRet;
				iRet = __xuiCodeBracketConsume(pState, iByte, iOffset);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	return XUI_OK;
}

int xuiInternalCodeBracketUpdate(xui_internal_code_bracket_state_t* pState,
	xui_code_document pDocument, const xui_code_language_t* pLanguage,
	int iCaretOffset, int iByteBudget, int* pChanged)
{
	uint32_t iVersion;
	int iOldStatus;
	int iOldSource;
	int iOldMatch;
	int iStartOffset;
	int iEndOffset;
	int iRet;
	int bLanguageChanged;

	if ( pState == NULL || pDocument == NULL || iByteBudget <= 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pChanged != NULL ) *pChanged = 0;
	iOldStatus = pState->iStatus;
	iOldSource = pState->iSourceOffset;
	iOldMatch = pState->iMatchOffset;
	iVersion = xuiCodeDocumentGetVersion(pDocument);
	bLanguageChanged = pState->bHasLanguage != (pLanguage != NULL);
	if ( !bLanguageChanged && pLanguage != NULL ) {
		bLanguageChanged =
			pState->iLanguageLexerType != pLanguage->iLexerType ||
			pState->onLanguageLex != pLanguage->onLex ||
			pState->pLanguageRegexRules != pLanguage->pRegexRules ||
			pState->iLanguageRegexRuleCount != pLanguage->iRegexRuleCount ||
			pState->pLanguageUser != pLanguage->pUser;
	}
	if ( !pState->bInitialized || pState->iDocumentVersion != iVersion ||
	     pState->iCaretOffset != iCaretOffset || bLanguageChanged ) {
		__xuiCodeBracketClearSearch(pState);
		iRet = __xuiCodeBracketBegin(pState, pDocument, pLanguage,
			iCaretOffset);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pState->iStatus == XUI_CODE_BRACKET_PENDING ) {
		if ( pState->iDirection > 0 ) {
			iStartOffset = pState->iScanOffset;
			iEndOffset = iStartOffset + iByteBudget;
			if ( iEndOffset < iStartOffset ||
			     iEndOffset > pState->iDocumentLength ) {
				iEndOffset = pState->iDocumentLength;
			}
		} else {
			iEndOffset = pState->iScanOffset;
			iStartOffset = iEndOffset - iByteBudget;
			if ( iStartOffset < 0 ) iStartOffset = 0;
		}
		if ( pLanguage != NULL ) {
			iRet = __xuiCodeBracketScanTokens(pState, pDocument, pLanguage,
				iStartOffset, iEndOffset);
		} else {
			iRet = __xuiCodeBracketScanPlain(pState, pDocument,
				iStartOffset, iEndOffset);
		}
		if ( iRet != XUI_OK ) return iRet;
		if ( pState->iStatus == XUI_CODE_BRACKET_PENDING ) {
			pState->iScanOffset = (pState->iDirection > 0) ?
				iEndOffset : iStartOffset;
			if ( (pState->iDirection > 0 &&
			     pState->iScanOffset >= pState->iDocumentLength) ||
			     (pState->iDirection < 0 && pState->iScanOffset <= 0) ) {
				pState->iStatus = XUI_CODE_BRACKET_UNMATCHED;
			}
		}
	}
	if ( pChanged != NULL ) {
		*pChanged = iOldStatus != pState->iStatus ||
			iOldSource != pState->iSourceOffset ||
			iOldMatch != pState->iMatchOffset;
	}
	return XUI_OK;
}

#endif
