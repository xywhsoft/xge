#include "../xui.h"

#include <ctype.h>
#include <string.h>

static int __xuiCodeLexerCIsIdentStart(int c)
{
	return (c == '_') || isalpha((unsigned char)c);
}

static int __xuiCodeLexerCIsIdent(int c)
{
	return (c == '_') || isalnum((unsigned char)c);
}

static int __xuiCodeLexerCUtf8Next(const char* sText, int iTextSize, int iOffset)
{
	unsigned char c;
	int iStep;

	if ( sText == NULL ) return 0;
	if ( iOffset < 0 ) return 0;
	if ( iOffset >= iTextSize ) return iTextSize;
	c = (unsigned char)sText[iOffset];
	if ( c < 0x80u ) iStep = 1;
	else if ( (c & 0xE0u) == 0xC0u ) iStep = 2;
	else if ( (c & 0xF0u) == 0xE0u ) iStep = 3;
	else if ( (c & 0xF8u) == 0xF0u ) iStep = 4;
	else iStep = 1;
	if ( iOffset + iStep > iTextSize ) return iTextSize;
	return iOffset + iStep;
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

XUI_API int xuiCodeLexerCTokenize(const char* sText, int iTextSize, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount)
{
	int i;
	int iStart;
	int iKind;

	if ( pTokenCount == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pTokenCount = 0;
	if ( sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iTextSize < 0 ) iTextSize = (int)strlen(sText);
	i = 0;
	while ( i < iTextSize ) {
		if ( isspace((unsigned char)sText[i]) ) {
			i++;
			continue;
		}
		iStart = i;
		if ( sText[i] == '#' ) {
			while ( i < iTextSize && sText[i] != '\n' ) i++;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_PREPROCESSOR);
		} else if ( (sText[i] == '/') && (i + 1 < iTextSize) && (sText[i + 1] == '/') ) {
			i += 2;
			while ( i < iTextSize && sText[i] != '\n' ) i++;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_COMMENT);
		} else if ( (sText[i] == '/') && (i + 1 < iTextSize) && (sText[i + 1] == '*') ) {
			i += 2;
			while ( i + 1 < iTextSize && !(sText[i] == '*' && sText[i + 1] == '/') ) i++;
			if ( i + 1 < iTextSize ) i += 2;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_COMMENT);
		} else if ( sText[i] == '"' ) {
			i++;
			while ( i < iTextSize ) {
				if ( sText[i] == '\\' && i + 1 < iTextSize ) {
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
			while ( i < iTextSize ) {
				if ( sText[i] == '\\' && i + 1 < iTextSize ) {
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
			while ( i < iTextSize &&
			        (isalnum((unsigned char)sText[i]) || sText[i] == '.' || sText[i] == '_' || sText[i] == '+' || sText[i] == '-') ) {
				i++;
			}
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_NUMBER);
		} else if ( __xuiCodeLexerCIsIdentStart((unsigned char)sText[i]) ) {
			i++;
			while ( i < iTextSize && __xuiCodeLexerCIsIdent((unsigned char)sText[i]) ) i++;
			iKind = __xuiCodeLexerCIsKeyword(sText, iStart, i);
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, iKind);
		} else if ( strchr("{}[]()", sText[i]) != NULL ) {
			i++;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_BRACE);
		} else if ( strchr("+-*/%=!<>|&^~?:;.,", sText[i]) != NULL ) {
			i++;
			while ( i < iTextSize && strchr("+-*/%=!<>|&^~", sText[i]) != NULL ) i++;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_OPERATOR);
		} else if ( ((unsigned char)sText[i]) >= 0x80u ) {
			i = __xuiCodeLexerCUtf8Next(sText, iTextSize, i);
			while ( i < iTextSize && ((unsigned char)sText[i]) >= 0x80u ) {
				i = __xuiCodeLexerCUtf8Next(sText, iTextSize, i);
			}
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_TEXT);
		} else {
			i++;
			(void)__xuiCodeLexerCAddToken(pTokens, iTokenCapacity, pTokenCount, iStart, i, XUI_CODE_TOKEN_TEXT);
		}
	}
	return XUI_OK;
}
