#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui_internal.h"

#include <ctype.h>
#include <limits.h>
#include <string.h>

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

XUI_API int xuiCodeLexerCTokenizeRange(const char* sText, int iTextSize, int iStartOffset, int iEndOffset, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount)
{
	int i;
	int iStart;
	int iKind;
	int iOriginalStart;
	int iNext;
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
	while ( i < iEndOffset ) {
		if ( isspace((unsigned char)sText[i]) ) {
			i++;
			continue;
		}
		iStart = i;
		if ( sText[i] == '#' ) {
			while ( i < iEndOffset && sText[i] != '\n' ) i++;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_PREPROCESSOR);
		} else if ( (sText[i] == '/') && (i + 1 < iEndOffset) && (sText[i + 1] == '/') ) {
			i += 2;
			while ( i < iEndOffset && sText[i] != '\n' ) i++;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_COMMENT);
		} else if ( (sText[i] == '/') && (i + 1 < iEndOffset) && (sText[i + 1] == '*') ) {
			i += 2;
			while ( i + 1 < iEndOffset && !(sText[i] == '*' && sText[i + 1] == '/') ) i++;
			if ( i + 1 < iEndOffset ) i += 2;
			else i = iEndOffset;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_COMMENT);
		} else if ( sText[i] == '"' ) {
			i++;
			while ( i < iEndOffset ) {
				if ( sText[i] == '\\' && i + 1 < iEndOffset ) {
					i += 2;
				} else if ( sText[i] == '"' ) {
					i++;
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
					i += 2;
				} else if ( sText[i] == '\'' ) {
					i++;
					break;
				} else {
					i++;
				}
			}
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_CHAR);
		} else if ( isdigit((unsigned char)sText[i]) ) {
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
		} else if ( strchr("+-*/%=!<>|&^~?:;.,", sText[i]) != NULL ) {
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
	}
	return XUI_OK;
}

XUI_API int xuiCodeLexerCTokenize(const char* sText, int iTextSize, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount)
{
	return xuiCodeLexerCTokenizeRange(sText, iTextSize, 0, iTextSize, pTokens, iTokenCapacity, pTokenCount);
}

XUI_API int xuiCodeLexerCTokenizeDocumentRange(xui_code_document pDocument,
	int iStartOffset, int iEndOffset, xui_code_token_t* pTokens,
	int iTokenCapacity, int* pTokenCount)
{
	char* sRange;
	int iDocumentLength;
	int iRangeLength;
	int iStoredCount;
	int i;
	int iRet;

	if ( pDocument == NULL || pTokenCount == NULL || iTokenCapacity < 0 ||
	     (pTokens == NULL && iTokenCapacity > 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iDocumentLength = xuiCodeDocumentGetLength(pDocument);
	if ( iStartOffset < 0 ) iStartOffset = 0;
	if ( iEndOffset < iStartOffset ) iEndOffset = iStartOffset;
	if ( iEndOffset > iDocumentLength ) iEndOffset = iDocumentLength;
	iRangeLength = iEndOffset - iStartOffset;
	sRange = (char*)xrtMalloc((size_t)iRangeLength + 1u);
	if ( sRange == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = xuiCodeDocumentCopyRange(pDocument, iStartOffset, iEndOffset,
		sRange, iRangeLength + 1, NULL);
	if ( iRet == XUI_OK ) {
		iRet = xuiCodeLexerCTokenizeRange(sRange, iRangeLength, 0, iRangeLength,
			pTokens, iTokenCapacity, pTokenCount);
	}
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
