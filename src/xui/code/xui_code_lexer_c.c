#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui_internal.h"

#include <ctype.h>
#include <limits.h>
#include <string.h>

enum {
	XUI_CODE_LEXER_C_NORMAL = 0,
	XUI_CODE_LEXER_C_BLOCK_COMMENT,
	XUI_CODE_LEXER_C_STRING,
	XUI_CODE_LEXER_C_CHAR,
	XUI_CODE_LEXER_C_PREPROCESSOR,
	XUI_CODE_LEXER_C_LINE_COMMENT
};

typedef struct xui_code_lexer_c_state_t {
	unsigned char iKind;
	unsigned char bDirectiveStart;
	unsigned char bPendingEscape;
	unsigned char bPendingStar;
	unsigned char bPendingSlash;
} xui_code_lexer_c_state_t;

typedef struct xui_code_lexer_c_cache_t {
	xui_code_lexer_c_state_t* pStates;
	int iStateCapacity;
	int iValidLineCount;
	char* sScratch;
	int iScratchCapacity;
	uint32_t iVersion;
	int bInitialized;
} xui_code_lexer_c_cache_t;

static void __xuiCodeLexerCScanState(const char* sText, int iTextSize,
	xui_code_lexer_c_state_t* pState)
{
	int i;

	if ( sText == NULL || iTextSize <= 0 || pState == NULL ) return;
	i = 0;
	while ( i < iTextSize ) {
		switch ( pState->iKind ) {
		case XUI_CODE_LEXER_C_BLOCK_COMMENT:
			if ( pState->bPendingStar ) {
				pState->bPendingStar = 0;
				if ( sText[i] == '/' ) {
					pState->iKind = XUI_CODE_LEXER_C_NORMAL;
					i++;
					break;
				}
			}
			if ( i + 1 < iTextSize && sText[i] == '*' && sText[i + 1] == '/' ) {
				pState->iKind = XUI_CODE_LEXER_C_NORMAL;
				i += 2;
			} else {
				if ( sText[i] == '\n' ) pState->bDirectiveStart = 1;
				if ( sText[i] == '*' && i + 1 == iTextSize ) {
					pState->bPendingStar = 1;
				}
				i++;
			}
			break;
		case XUI_CODE_LEXER_C_STRING:
		case XUI_CODE_LEXER_C_CHAR:
			if ( pState->bPendingEscape ) {
				pState->bPendingEscape = 0;
				if ( sText[i] == '\r' && i + 1 < iTextSize &&
				     sText[i + 1] == '\n' ) i += 2;
				else i++;
			} else if ( sText[i] == '\\' && i + 1 < iTextSize ) {
				if ( sText[i + 1] == '\r' && i + 2 < iTextSize &&
				     sText[i + 2] == '\n' ) i += 3;
				else i += 2;
			} else if ( sText[i] == '\\' ) {
				pState->bPendingEscape = 1;
				i++;
			} else if ( (pState->iKind == XUI_CODE_LEXER_C_STRING && sText[i] == '"') ||
			            (pState->iKind == XUI_CODE_LEXER_C_CHAR && sText[i] == '\'') ) {
				pState->iKind = XUI_CODE_LEXER_C_NORMAL;
				i++;
			} else if ( sText[i] == '\n' || sText[i] == '\r' ) {
				pState->iKind = XUI_CODE_LEXER_C_NORMAL;
			} else {
				i++;
			}
			break;
		case XUI_CODE_LEXER_C_PREPROCESSOR:
			if ( pState->bPendingEscape ) {
				pState->bPendingEscape = 0;
				if ( sText[i] == '\r' && i + 1 < iTextSize &&
				     sText[i + 1] == '\n' ) i += 2;
				else i++;
			} else if ( sText[i] == '\\' && i + 1 < iTextSize ) {
				if ( sText[i + 1] == '\r' && i + 2 < iTextSize &&
				     sText[i + 2] == '\n' ) i += 3;
				else if ( sText[i + 1] == '\n' ) i += 2;
				else i += 2;
			} else if ( sText[i] == '\\' ) {
				pState->bPendingEscape = 1;
				i++;
			} else if ( sText[i] == '\n' ) {
				pState->iKind = XUI_CODE_LEXER_C_NORMAL;
				pState->bDirectiveStart = 1;
				i++;
			} else {
				i++;
			}
			break;
		case XUI_CODE_LEXER_C_LINE_COMMENT:
			if ( sText[i] == '\n' ) {
				pState->iKind = XUI_CODE_LEXER_C_NORMAL;
				pState->bDirectiveStart = 1;
			}
			i++;
			break;
		default:
			if ( pState->bPendingSlash ) {
				pState->bPendingSlash = 0;
				if ( sText[i] == '/' ) {
					pState->iKind = XUI_CODE_LEXER_C_LINE_COMMENT;
					i++;
					break;
				}
				if ( sText[i] == '*' ) {
					pState->iKind = XUI_CODE_LEXER_C_BLOCK_COMMENT;
					i++;
					break;
				}
			}
			if ( isspace((unsigned char)sText[i]) ) {
				if ( sText[i] == '\n' ) pState->bDirectiveStart = 1;
				i++;
			} else if ( i + 1 < iTextSize && sText[i] == '/' && sText[i + 1] == '/' ) {
				pState->iKind = XUI_CODE_LEXER_C_LINE_COMMENT;
				i += 2;
			} else if ( i + 1 < iTextSize && sText[i] == '/' && sText[i + 1] == '*' ) {
				pState->iKind = XUI_CODE_LEXER_C_BLOCK_COMMENT;
				i += 2;
			} else if ( sText[i] == '/' ) {
				pState->bPendingSlash = 1;
				pState->bDirectiveStart = 0;
				i++;
			} else if ( sText[i] == '"' ) {
				pState->iKind = XUI_CODE_LEXER_C_STRING;
				pState->bDirectiveStart = 0;
				i++;
			} else if ( sText[i] == '\'' ) {
				pState->iKind = XUI_CODE_LEXER_C_CHAR;
				pState->bDirectiveStart = 0;
				i++;
			} else if ( sText[i] == '#' && pState->bDirectiveStart ) {
				pState->iKind = XUI_CODE_LEXER_C_PREPROCESSOR;
				pState->bDirectiveStart = 0;
				i++;
			} else {
				pState->bDirectiveStart = 0;
				i++;
			}
			break;
		}
	}
}

static void __xuiCodeLexerCCacheDestroy(void* pValue)
{
	xui_code_lexer_c_cache_t* pCache = (xui_code_lexer_c_cache_t*)pValue;

	if ( pCache == NULL ) return;
	xrtFree(pCache->pStates);
	xrtFree(pCache->sScratch);
	xrtFree(pCache);
}

static int __xuiCodeLexerCCacheReserveStates(xui_code_lexer_c_cache_t* pCache,
	int iCapacity)
{
	xui_code_lexer_c_state_t* pNew;
	int iNewCapacity;

	if ( pCache == NULL || iCapacity < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pCache->iStateCapacity ) return XUI_OK;
	iNewCapacity = pCache->iStateCapacity > 0 ? pCache->iStateCapacity : 64;
	while ( iNewCapacity < iCapacity ) {
		if ( iNewCapacity > INT_MAX / 2 ) {
			iNewCapacity = iCapacity;
			break;
		}
		iNewCapacity *= 2;
	}
	if ( (size_t)iNewCapacity > SIZE_MAX / sizeof(*pNew) ) {
		return XUI_ERROR_LIMIT_EXCEEDED;
	}
	pNew = (xui_code_lexer_c_state_t*)xrtRealloc(pCache->pStates,
		sizeof(*pNew) * (size_t)iNewCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pCache->pStates = pNew;
	pCache->iStateCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiCodeLexerCCacheReserveScratch(xui_code_lexer_c_cache_t* pCache,
	int iCapacity)
{
	char* sNew;
	int iNewCapacity;

	if ( pCache == NULL || iCapacity < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pCache->iScratchCapacity ) return XUI_OK;
	iNewCapacity = pCache->iScratchCapacity > 0 ? pCache->iScratchCapacity : 256;
	while ( iNewCapacity < iCapacity ) {
		if ( iNewCapacity > INT_MAX / 2 ) {
			iNewCapacity = iCapacity;
			break;
		}
		iNewCapacity *= 2;
	}
	sNew = (char*)xrtRealloc(pCache->sScratch, (size_t)iNewCapacity);
	if ( sNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pCache->sScratch = sNew;
	pCache->iScratchCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiCodeLexerCGetCache(xui_code_document pDocument,
	xui_code_lexer_c_cache_t** ppCache)
{
	xui_code_lexer_c_cache_t* pCache;
	int iRet;

	if ( pDocument == NULL || ppCache == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pCache = (xui_code_lexer_c_cache_t*)
		xuiInternalCodeDocumentGetLexerCCache(pDocument);
	if ( pCache == NULL ) {
		pCache = (xui_code_lexer_c_cache_t*)xrtCalloc(1u, sizeof(*pCache));
		if ( pCache == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		iRet = xuiInternalCodeDocumentSetLexerCCache(pDocument, pCache,
			__xuiCodeLexerCCacheDestroy);
		if ( iRet != XUI_OK ) {
			__xuiCodeLexerCCacheDestroy(pCache);
			return iRet;
		}
	}
	*ppCache = pCache;
	return XUI_OK;
}

static int __xuiCodeLexerCIsIdentStart(int c)
{
	return (c == '_') || isalpha((unsigned char)c);
}

static int __xuiCodeLexerCIsIdent(int c)
{
	return (c == '_') || isalnum((unsigned char)c);
}

static int __xuiCodeLexerCUnicodeIdentifierEnd(const char* sText,
	int iTextSize, int iOffset)
{
	int iStart;
	int iEnd;
	xui_internal_word_kind_t iKind;

	if ( sText == NULL || iOffset < 0 || iOffset >= iTextSize ) return iOffset;
	iKind = xuiInternalTextWordRange(sText, iTextSize, iOffset,
		XUI_INTERNAL_WORD_IDENTIFIER, &iStart, &iEnd);
	return (iKind == XUI_INTERNAL_WORD_TEXT && iStart <= iOffset && iEnd > iOffset) ?
		iEnd : iOffset;
}

static int __xuiCodeLexerCIsKeyword(const char* sText, int iStart, int iEnd)
{
	static const char* const arrKeywords[] = {
		"auto", "break", "case", "const", "continue", "default", "do",
		"else", "enum", "extern", "for", "goto", "if", "inline", "register",
		"restrict", "return", "sizeof", "static", "struct", "switch",
		"typedef", "union", "volatile", "while", "_Alignas", "_Alignof",
		"_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary",
		"_Noreturn", "_Static_assert", "_Thread_local"
	};
	static const char* const arrTypes[] = {
		"char", "double", "float", "int", "long", "short", "signed",
		"unsigned", "void"
	};
	int i;
	int iLen;

	iLen = iEnd - iStart;
	for ( i = 0; i < (int)(sizeof(arrKeywords) / sizeof(arrKeywords[0])); i++ ) {
		if ( (int)strlen(arrKeywords[i]) == iLen &&
		     strncmp(sText + iStart, arrKeywords[i], (size_t)iLen) == 0 ) {
			return XUI_CODE_TOKEN_KEYWORD;
		}
	}
	for ( i = 0; i < (int)(sizeof(arrTypes) / sizeof(arrTypes[0])); i++ ) {
		if ( (int)strlen(arrTypes[i]) == iLen &&
		     strncmp(sText + iStart, arrTypes[i], (size_t)iLen) == 0 ) {
			return XUI_CODE_TOKEN_TYPE;
		}
	}
	return XUI_CODE_TOKEN_IDENTIFIER;
}

static int __xuiCodeLexerCAtDirectiveStart(const char* sText, int iOffset)
{
	int i;

	if ( sText == NULL || iOffset < 0 ) return 0;
	for ( i = iOffset; i > 0; ) {
		unsigned char iChar = (unsigned char)sText[--i];
		if ( iChar == '\n' ) break;
		if ( iChar != ' ' && iChar != '\t' && iChar != '\r' &&
		     iChar != '\f' && iChar != '\v' ) return 0;
	}
	return 1;
}

static int __xuiCodeLexerCDocumentClampOffset(xui_code_document pDocument,
	int iOffset, int bForward, int* pClamped)
{
	return xuiInternalCodeDocumentClampOffset(pDocument, iOffset,
		bForward, pClamped);
}

static int __xuiCodeLexerCDocumentStateAt(xui_code_document pDocument,
	int iOffset, xui_code_lexer_c_state_t* pState)
{
	xui_code_lexer_c_cache_t* pCache;
	xui_code_range_t tEditRange;
	xui_code_lexer_c_state_t tState;
	uint32_t iVersion;
	int iLineCount;
	int iTargetLine;
	int iEditLine;
	int iLine;
	int iStart;
	int iEnd;
	int iLength;
	int iRet;

	if ( pDocument == NULL || pState == NULL || iOffset < 0 ||
	     iOffset > xuiCodeDocumentGetLength(pDocument) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiCodeLexerCGetCache(pDocument, &pCache);
	if ( iRet != XUI_OK ) return iRet;
	iLineCount = xuiCodeDocumentGetLineCount(pDocument);
	iRet = xuiCodeDocumentOffsetToLineColumn(pDocument, iOffset,
		&iTargetLine, NULL);
	if ( iRet != XUI_OK ) return iRet;
	iVersion = xuiCodeDocumentGetVersion(pDocument);
	if ( !pCache->bInitialized || pCache->iVersion != iVersion ) {
		if ( pCache->bInitialized && iVersion == pCache->iVersion + 1u &&
		     pCache->iValidLineCount > 0 ) {
			memset(&tEditRange, 0, sizeof(tEditRange));
			if ( xuiCodeDocumentGetLastEditRange(pDocument, &tEditRange) == XUI_OK &&
			     xuiCodeDocumentOffsetToLineColumn(pDocument, tEditRange.iStart,
				     &iEditLine, NULL) == XUI_OK ) {
				if ( pCache->iValidLineCount > iEditLine + 1 ) {
					pCache->iValidLineCount = iEditLine + 1;
				}
			} else {
				pCache->iValidLineCount = 0;
			}
		} else {
			pCache->iValidLineCount = 0;
		}
		pCache->iVersion = iVersion;
		pCache->bInitialized = 1;
	}
	if ( pCache->iValidLineCount > iLineCount ) {
		pCache->iValidLineCount = iLineCount;
	}
	iRet = __xuiCodeLexerCCacheReserveStates(pCache, iTargetLine + 1);
	if ( iRet != XUI_OK ) return iRet;
	if ( pCache->iValidLineCount == 0 ) {
		pCache->pStates[0].iKind = XUI_CODE_LEXER_C_NORMAL;
		pCache->pStates[0].bDirectiveStart = 1;
		pCache->iValidLineCount = 1;
	}
	while ( pCache->iValidLineCount <= iTargetLine ) {
		iLine = pCache->iValidLineCount - 1;
		iRet = xuiCodeDocumentGetLineRange(pDocument, iLine, &iStart, NULL);
		if ( iRet != XUI_OK ) return iRet;
		if ( iLine + 1 < iLineCount ) {
			iRet = xuiCodeDocumentGetLineRange(pDocument, iLine + 1,
				&iEnd, NULL);
			if ( iRet != XUI_OK ) return iRet;
		} else {
			iEnd = xuiCodeDocumentGetLength(pDocument);
		}
		iLength = iEnd - iStart;
		if ( iLength < 0 || iLength == INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
		iRet = __xuiCodeLexerCCacheReserveScratch(pCache, iLength + 1);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiCodeDocumentCopyRange(pDocument, iStart, iEnd,
			pCache->sScratch, iLength + 1, NULL);
		if ( iRet != XUI_OK ) return iRet;
		tState = pCache->pStates[iLine];
		__xuiCodeLexerCScanState(pCache->sScratch, iLength, &tState);
		pCache->pStates[pCache->iValidLineCount++] = tState;
	}
	tState = pCache->pStates[iTargetLine];
	iRet = xuiCodeDocumentGetLineRange(pDocument, iTargetLine, &iStart, NULL);
	if ( iRet != XUI_OK ) return iRet;
	iLength = iOffset - iStart;
	if ( iLength < 0 || iLength == INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
	if ( iLength > 0 ) {
		iRet = __xuiCodeLexerCCacheReserveScratch(pCache, iLength + 1);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiCodeDocumentCopyRange(pDocument, iStart, iOffset,
			pCache->sScratch, iLength + 1, NULL);
		if ( iRet != XUI_OK ) return iRet;
		__xuiCodeLexerCScanState(pCache->sScratch, iLength, &tState);
	}
	*pState = tState;
	return XUI_OK;
}

static int __xuiCodeLexerCAddToken(xui_code_token_t* pTokens, int iCapacity, int* pCount, int iStart, int iEnd, int iKind)
{
	xui_code_token_t* pToken;

	if ( (pCount == NULL) || (iStart >= iEnd) ) return XUI_OK;
	if ( (*pCount >= iCapacity) || (pTokens == NULL) ) {
		(*pCount)++;
		return XUI_OK;
	}
	pToken = &pTokens[*pCount];
	memset(pToken, 0, sizeof(*pToken));
	pToken->iStartOffset = iStart;
	pToken->iEndOffset = iEnd;
	pToken->iKind = iKind;
	pToken->iStyle = iKind;
	(*pCount)++;
	return XUI_OK;
}

static int __xuiCodeLexerCTokenizeIsolated(const char* sText, int iTextSize,
	int iStartOffset, int iEndOffset, xui_code_token_t* pTokens,
	int iTokenCapacity, int* pTokenCount)
{
	int i;
	int iStart;
	int iKind;
	int iOriginalStart;
	int iNext;
	int bDirectiveStart;
	int bKeepsDirectiveStart;
	size_t iTextSizeValue;

	if ( pTokenCount == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pTokenCount = 0;
	if ( sText == NULL || iTokenCapacity < 0 ||
	     (pTokens == NULL && iTokenCapacity > 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iTextSize < 0 ) {
		iTextSizeValue = strlen(sText);
		if ( iTextSizeValue > (size_t)INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
		iTextSize = (int)iTextSizeValue;
	}
	if ( iStartOffset < 0 ) iStartOffset = 0;
	if ( iStartOffset > iTextSize ) iStartOffset = iTextSize;
	if ( iEndOffset < 0 || iEndOffset > iTextSize ) iEndOffset = iTextSize;
	iOriginalStart = iStartOffset;
	iStartOffset = xuiInternalTextGraphemeClamp(sText, iTextSize, iStartOffset);
	if ( iStartOffset < iOriginalStart ) {
		iStartOffset = xuiInternalTextGraphemeNext(sText, iTextSize, iStartOffset);
	}
	iEndOffset = xuiInternalTextGraphemeClamp(sText, iTextSize, iEndOffset);
	if ( iEndOffset < iStartOffset ) iEndOffset = iStartOffset;
	i = iStartOffset;
	bDirectiveStart = __xuiCodeLexerCAtDirectiveStart(sText, iStartOffset);
	while ( i < iEndOffset ) {
		if ( isspace((unsigned char)sText[i]) ) {
			if ( sText[i] == '\n' ) bDirectiveStart = 1;
			i++;
			continue;
		}
		iStart = i;
		bKeepsDirectiveStart = 0;
		if ( sText[i] == '#' && bDirectiveStart ) {
			i++;
			while ( i < iEndOffset ) {
				if ( sText[i] == '\n' ) {
					int iBack = i - 1;
					if ( iBack >= iStart && sText[iBack] == '\r' ) iBack--;
					if ( iBack >= iStart && sText[iBack] == '\\' ) {
						i++;
						continue;
					}
					break;
				}
				i++;
			}
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_PREPROCESSOR);
		} else if ( (sText[i] == '/') && (i + 1 < iEndOffset) && (sText[i + 1] == '/') ) {
			i += 2;
			while ( i < iEndOffset && sText[i] != '\n' ) i++;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_COMMENT);
		} else if ( (sText[i] == '/') && (i + 1 < iEndOffset) && (sText[i + 1] == '*') ) {
			i += 2;
			bKeepsDirectiveStart = 1;
			while ( i + 1 < iEndOffset && !(sText[i] == '*' && sText[i + 1] == '/') ) {
				if ( sText[i] == '\n' ) bDirectiveStart = 1;
				i++;
			}
			if ( i + 1 < iEndOffset ) i += 2;
			else i = iEndOffset;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_COMMENT);
		} else if ( sText[i] == '"' ) {
			i++;
			while ( i < iEndOffset ) {
				if ( sText[i] == '\\' && i + 1 < iEndOffset ) {
					if ( sText[i + 1] == '\r' && i + 2 < iEndOffset &&
					     sText[i + 2] == '\n' ) i += 3;
					else i += 2;
				} else if ( sText[i] == '"' ) {
					i++;
					break;
				} else if ( sText[i] == '\n' || sText[i] == '\r' ) {
					break;
				} else {
					i++;
				}
			}
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_STRING);
		} else if ( sText[i] == '\'' ) {
			i++;
			while ( i < iEndOffset ) {
				if ( sText[i] == '\\' && i + 1 < iEndOffset ) {
					if ( sText[i + 1] == '\r' && i + 2 < iEndOffset &&
					     sText[i + 2] == '\n' ) i += 3;
					else i += 2;
				} else if ( sText[i] == '\'' ) {
					i++;
					break;
				} else if ( sText[i] == '\n' || sText[i] == '\r' ) {
					break;
				} else {
					i++;
				}
			}
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_CHAR);
		} else if ( isdigit((unsigned char)sText[i]) ||
		            (sText[i] == '.' && i + 1 < iEndOffset &&
		             isdigit((unsigned char)sText[i + 1])) ) {
			i++;
			while ( i < iEndOffset ) {
				if ( isalnum((unsigned char)sText[i]) || sText[i] == '.' || sText[i] == '_' ) {
					i++;
				} else if ( (sText[i] == '+' || sText[i] == '-') && i > iStart &&
				            (sText[i - 1] == 'e' || sText[i - 1] == 'E' ||
				             sText[i - 1] == 'p' || sText[i - 1] == 'P') ) {
					i++;
				} else {
					break;
				}
			}
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_NUMBER);
		} else if ( __xuiCodeLexerCIsIdentStart((unsigned char)sText[i]) ) {
			i++;
			while ( i < iEndOffset ) {
				if ( __xuiCodeLexerCIsIdent((unsigned char)sText[i]) ) {
					i++;
					continue;
				}
				if ( (unsigned char)sText[i] >= 0x80u ) {
					iNext = __xuiCodeLexerCUnicodeIdentifierEnd(sText, iEndOffset, i);
					if ( iNext > i ) {
						i = iNext;
						continue;
					}
				}
				break;
			}
			iKind = __xuiCodeLexerCIsKeyword(sText, iStart, i);
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, iKind);
		} else if ( strchr("{}[]()", sText[i]) != NULL ) {
			i++;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_BRACE);
		} else if ( strchr("+-*/%=!<>|&^~?:;.,#", sText[i]) != NULL ) {
			i++;
			while ( i < iEndOffset && strchr("+-*/%=!<>|&^~", sText[i]) != NULL ) i++;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_OPERATOR);
		} else if ( ((unsigned char)sText[i]) >= 0x80u ) {
			iNext = __xuiCodeLexerCUnicodeIdentifierEnd(sText, iEndOffset, i);
			if ( iNext > i ) {
				i = iNext;
				iKind = XUI_CODE_TOKEN_IDENTIFIER;
			} else {
				i = xuiInternalTextGraphemeNext(sText, iEndOffset, i);
				if ( i <= iStart ) i = iStart + 1;
				iKind = XUI_CODE_TOKEN_TEXT;
			}
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, iKind);
		} else {
			i++;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_TEXT);
		}
		if ( !bKeepsDirectiveStart ) bDirectiveStart = 0;
	}
	return XUI_OK;
}

static const char* __xuiCodeLexerCPrefixForState(
	const xui_code_lexer_c_state_t* pState)
{
	if ( pState == NULL ) return NULL;
	switch ( pState->iKind ) {
	case XUI_CODE_LEXER_C_BLOCK_COMMENT:
		return pState->bPendingStar ? "/**" : "/*";
	case XUI_CODE_LEXER_C_STRING:
		return pState->bPendingEscape ? "\"\\" : "\"";
	case XUI_CODE_LEXER_C_CHAR:
		return pState->bPendingEscape ? "'\\" : "'";
	case XUI_CODE_LEXER_C_PREPROCESSOR:
		return pState->bPendingEscape ? "#\\" : "#";
	case XUI_CODE_LEXER_C_LINE_COMMENT:
		return "//";
	case XUI_CODE_LEXER_C_NORMAL:
		if ( pState->bPendingSlash ) return "/";
		return pState->bDirectiveStart ? "" : "x ";
	default:
		return NULL;
	}
}

static int __xuiCodeLexerCTokenizeWithState(const char* sText, int iTextSize,
	const xui_code_lexer_c_state_t* pState, xui_code_token_t* pTokens,
	int iTokenCapacity, int* pTokenCount)
{
	xui_code_token_t* pScratchTokens;
	const char* sPrefix;
	char* sLexText;
	int iPrefixLength;
	int iLexLength;
	int iScratchCapacity;
	int iRawCount;
	int iStoredCount;
	int iPrefixOnlyCount;
	int iOutputStoredCount;
	int iLocalStart;
	int iLocalEnd;
	int i;
	int iRet;

	if ( pTokenCount == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pTokenCount = 0;
	if ( sText == NULL || iTextSize < 0 || pState == NULL ||
	     iTokenCapacity < 0 || (pTokens == NULL && iTokenCapacity > 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iTextSize == 0 ) return XUI_OK;
	sPrefix = __xuiCodeLexerCPrefixForState(pState);
	if ( sPrefix == NULL ) return XUI_ERROR_INVALID_STATE;
	iPrefixLength = (int)strlen(sPrefix);
	if ( iPrefixLength == 0 ) {
		return __xuiCodeLexerCTokenizeIsolated(sText, iTextSize, 0,
			iTextSize, pTokens, iTokenCapacity, pTokenCount);
	}
	if ( iTextSize > INT_MAX - iPrefixLength ) return XUI_ERROR_LIMIT_EXCEEDED;
	iLexLength = iPrefixLength + iTextSize;
	sLexText = (char*)xrtMalloc((size_t)iLexLength + 1u);
	if ( sLexText == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memcpy(sLexText, sPrefix, (size_t)iPrefixLength);
	memcpy(sLexText + iPrefixLength, sText, (size_t)iTextSize);
	sLexText[iLexLength] = '\0';
	if ( iTokenCapacity == INT_MAX ) {
		xrtFree(sLexText);
		return XUI_ERROR_LIMIT_EXCEEDED;
	}
	iScratchCapacity = iTokenCapacity + 1;
	if ( (size_t)iScratchCapacity > SIZE_MAX / sizeof(*pScratchTokens) ) {
		xrtFree(sLexText);
		return XUI_ERROR_LIMIT_EXCEEDED;
	}
	pScratchTokens = (xui_code_token_t*)xrtMalloc(
		sizeof(*pScratchTokens) * (size_t)iScratchCapacity);
	if ( pScratchTokens == NULL ) {
		xrtFree(sLexText);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRawCount = 0;
	iRet = __xuiCodeLexerCTokenizeIsolated(sLexText, iLexLength, 0,
		iLexLength, pScratchTokens, iScratchCapacity, &iRawCount);
	if ( iRet == XUI_OK ) {
		iStoredCount = iRawCount;
		if ( iStoredCount > iScratchCapacity ) iStoredCount = iScratchCapacity;
		iPrefixOnlyCount = 0;
		iOutputStoredCount = 0;
		for ( i = 0; i < iStoredCount; i++ ) {
			iLocalStart = pScratchTokens[i].iStartOffset - iPrefixLength;
			iLocalEnd = pScratchTokens[i].iEndOffset - iPrefixLength;
			if ( iLocalEnd <= 0 ) {
				iPrefixOnlyCount++;
				continue;
			}
			if ( iLocalStart < 0 ) iLocalStart = 0;
			if ( iLocalEnd > iTextSize ) iLocalEnd = iTextSize;
			if ( iLocalEnd <= iLocalStart ) continue;
			if ( pTokens != NULL && iOutputStoredCount < iTokenCapacity ) {
				pTokens[iOutputStoredCount] = pScratchTokens[i];
				pTokens[iOutputStoredCount].iStartOffset = iLocalStart;
				pTokens[iOutputStoredCount].iEndOffset = iLocalEnd;
			}
			iOutputStoredCount++;
		}
		*pTokenCount = iRawCount - iPrefixOnlyCount;
	}
	xrtFree(pScratchTokens);
	xrtFree(sLexText);
	return iRet;
}

XUI_API int xuiCodeLexerCTokenizeRange(const char* sText, int iTextSize,
	int iStartOffset, int iEndOffset, xui_code_token_t* pTokens,
	int iTokenCapacity, int* pTokenCount)
{
	xui_code_lexer_c_state_t tState;
	int iOriginalStart;
	int iStoredCount;
	int i;
	int iRet;
	size_t iTextSizeValue;

	if ( pTokenCount == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pTokenCount = 0;
	if ( sText == NULL || iTokenCapacity < 0 ||
	     (pTokens == NULL && iTokenCapacity > 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iTextSize < 0 ) {
		iTextSizeValue = strlen(sText);
		if ( iTextSizeValue > (size_t)INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
		iTextSize = (int)iTextSizeValue;
	}
	if ( iStartOffset < 0 ) iStartOffset = 0;
	if ( iStartOffset > iTextSize ) iStartOffset = iTextSize;
	if ( iEndOffset < 0 || iEndOffset > iTextSize ) iEndOffset = iTextSize;
	iOriginalStart = iStartOffset;
	iStartOffset = xuiInternalTextGraphemeClamp(sText, iTextSize, iStartOffset);
	if ( iStartOffset < iOriginalStart ) {
		iStartOffset = xuiInternalTextGraphemeNext(sText, iTextSize,
			iStartOffset);
	}
	iEndOffset = xuiInternalTextGraphemeClamp(sText, iTextSize, iEndOffset);
	if ( iEndOffset < iStartOffset ) iEndOffset = iStartOffset;
	memset(&tState, 0, sizeof(tState));
	tState.iKind = XUI_CODE_LEXER_C_NORMAL;
	tState.bDirectiveStart = 1;
	__xuiCodeLexerCScanState(sText, iStartOffset, &tState);
	iRet = __xuiCodeLexerCTokenizeWithState(sText + iStartOffset,
		iEndOffset - iStartOffset, &tState, pTokens, iTokenCapacity,
		pTokenCount);
	if ( iRet == XUI_OK && pTokens != NULL ) {
		iStoredCount = *pTokenCount;
		if ( iStoredCount > iTokenCapacity ) iStoredCount = iTokenCapacity;
		for ( i = 0; i < iStoredCount; i++ ) {
			pTokens[i].iStartOffset += iStartOffset;
			pTokens[i].iEndOffset += iStartOffset;
		}
	}
	return iRet;
}

XUI_API int xuiCodeLexerCTokenize(const char* sText, int iTextSize, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount)
{
	return xuiCodeLexerCTokenizeRange(sText, iTextSize, 0, iTextSize, pTokens, iTokenCapacity, pTokenCount);
}

XUI_API int xuiCodeLexerCTokenizeDocumentRange(xui_code_document pDocument,
	int iStartOffset, int iEndOffset, xui_code_token_t* pTokens,
	int iTokenCapacity, int* pTokenCount)
{
	xui_code_lexer_c_state_t tState;
	char* sRange;
	int iDocumentLength;
	int iRangeLength;
	int iStoredCount;
	int i;
	int iRet;

	if ( pTokenCount == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pTokenCount = 0;
	if ( pDocument == NULL || iTokenCapacity < 0 ||
	     (pTokens == NULL && iTokenCapacity > 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iDocumentLength = xuiCodeDocumentGetLength(pDocument);
	if ( iStartOffset < 0 ) iStartOffset = 0;
	if ( iStartOffset > iDocumentLength ) iStartOffset = iDocumentLength;
	if ( iEndOffset < 0 ) iEndOffset = iDocumentLength;
	if ( iEndOffset < iStartOffset ) iEndOffset = iStartOffset;
	if ( iEndOffset > iDocumentLength ) iEndOffset = iDocumentLength;
	iRet = __xuiCodeLexerCDocumentClampOffset(pDocument, iStartOffset, 1,
		&iStartOffset);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCodeLexerCDocumentClampOffset(pDocument, iEndOffset, 0,
		&iEndOffset);
	if ( iRet != XUI_OK ) return iRet;
	if ( iEndOffset < iStartOffset ) iEndOffset = iStartOffset;
	iRangeLength = iEndOffset - iStartOffset;
	if ( iRangeLength == 0 ) return XUI_OK;
	iRet = __xuiCodeLexerCDocumentStateAt(pDocument, iStartOffset, &tState);
	if ( iRet != XUI_OK ) return iRet;
	sRange = (char*)xrtMalloc((size_t)iRangeLength + 1u);
	if ( sRange == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = xuiCodeDocumentCopyRange(pDocument, iStartOffset, iEndOffset,
		sRange, iRangeLength + 1, NULL);
	if ( iRet != XUI_OK ) {
		xrtFree(sRange);
		return iRet;
	}
	iRet = __xuiCodeLexerCTokenizeWithState(sRange, iRangeLength,
		&tState, pTokens, iTokenCapacity, pTokenCount);
	if ( iRet == XUI_OK && pTokens != NULL ) {
		iStoredCount = *pTokenCount;
		if ( iStoredCount > iTokenCapacity ) iStoredCount = iTokenCapacity;
		for ( i = 0; i < iStoredCount; i++ ) {
			pTokens[i].iStartOffset += iStartOffset;
			pTokens[i].iEndOffset += iStartOffset;
		}
	}
	xrtFree(sRange);
	return iRet;
}

#endif
