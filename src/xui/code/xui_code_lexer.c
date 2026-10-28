#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui_internal.h"
#include "xui_xrt_port.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static void __xuiCodeLexerSetError(char* sError, int iCapacity, const char* sMessage)
{
	if ( (sError == NULL) || (iCapacity <= 0) ) return;
	if ( sMessage == NULL ) sMessage = "";
	snprintf(sError, (size_t)iCapacity, "%s", sMessage);
}

static int __xuiCodeLexerAddToken(xui_code_token_t* pTokens, int iCapacity, int* pCount, int iStart, int iEnd, int iKind, int iStyle)
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
	pToken->iStyle = iStyle;
	(*pCount)++;
	return XUI_OK;
}

XUI_API int xuiCodeLexerRegexTokenize(const char* sText, int iTextSize, const xui_code_regex_rule_t* pRules, int iRuleCount, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount, char* sError, int iErrorCapacity)
{
#ifdef XRT_NO_REGEX
	(void)sText;
	(void)iTextSize;
	(void)pRules;
	(void)iRuleCount;
	(void)pTokens;
	(void)iTokenCapacity;
	(void)pTokenCount;
	__xuiCodeLexerSetError(sError, iErrorCapacity, "XRT regex support is disabled");
	return XUI_ERROR_UNSUPPORTED;
#else
	xui_xrt_regex_t** pRegexes;
	xregexspan tSpan = { 0 };
	size_t iBegin;
	size_t iEnd;
	int i;
	int iPos;
	int iNext;
	int iTextStart;
	int iMatched;
	int iResult;
	int iRet;
	size_t iTextSizeValue;

	if ( pTokenCount == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pTokenCount = 0;
	__xuiCodeLexerSetError(sError, iErrorCapacity, "");
	if ( (sText == NULL) || (pRules == NULL) || (iRuleCount <= 0) ||
	     iTokenCapacity < 0 || (pTokens == NULL && iTokenCapacity > 0) ) {
		__xuiCodeLexerSetError(sError, iErrorCapacity, "invalid argument");
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iTextSize < 0 ) {
		iTextSizeValue = strlen(sText);
		if ( iTextSizeValue > (size_t)INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
		iTextSize = (int)iTextSizeValue;
	}
	for ( i = 0; i < iRuleCount; i++ ) {
		if ( pRules[i].sPattern == NULL ) {
			__xuiCodeLexerSetError(sError, iErrorCapacity, "invalid regex rule");
			return XUI_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( (size_t)iRuleCount > SIZE_MAX / sizeof(*pRegexes) ) return XUI_ERROR_LIMIT_EXCEEDED;
	pRegexes = (xui_xrt_regex_t**)xrtMalloc(sizeof(*pRegexes) * (size_t)iRuleCount);
	if ( pRegexes == NULL ) {
		__xuiCodeLexerSetError(sError, iErrorCapacity, "out of memory");
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	memset(pRegexes, 0, sizeof(*pRegexes) * (size_t)iRuleCount);
	for ( i = 0; i < iRuleCount; i++ ) {
		pRegexes[i] = xuiXrtRegexCreate(pRules[i].sPattern,
			strlen(pRules[i].sPattern),
			pRules[i].iFlags);
		if ( pRegexes[i] == NULL ) {
			__xuiCodeLexerSetError(sError, iErrorCapacity, "regex compile failed");
			goto fail;
		}
	}
	iPos = 0;
	iTextStart = 0;
	iResult = XUI_OK;
	while ( iPos < iTextSize ) {
		iMatched = 0;
		for ( i = 0; i < iRuleCount; i++ ) {
			iRet = xuiXrtRegexFindAt(pRegexes[i], sText, (size_t)iTextSize, (size_t)iPos, &tSpan);
			if ( iRet == XREGEX_ERROR ) {
				__xuiCodeLexerSetError(sError, iErrorCapacity, "regex match failed");
				iResult = XUI_ERROR_INVALID_STATE;
				goto done;
			}
			if ( iRet != XREGEX_MATCH ) continue;
			iBegin = tSpan.Begin;
			iEnd = tSpan.End;
			if ( iBegin > (size_t)iTextSize || iEnd > (size_t)iTextSize ) {
				__xuiCodeLexerSetError(sError, iErrorCapacity, "invalid regex span");
				iResult = XUI_ERROR_INVALID_STATE;
				goto done;
			}
			if ( (int)iBegin != iPos || iEnd <= iBegin ) continue;
			iNext = xuiInternalTextGraphemeClamp(sText, iTextSize, (int)iEnd);
			if ( iNext != (int)iEnd ) {
				iNext = xuiInternalTextGraphemeNext(sText, iTextSize, iNext);
				if ( iNext <= iPos || iNext > iTextSize ) iNext = (int)iEnd;
			} else {
				iNext = (int)iEnd;
			}
			(void)__xuiCodeLexerAddToken(pTokens, iTokenCapacity, pTokenCount,
				iTextStart, iPos, XUI_CODE_TOKEN_TEXT, XUI_CODE_STYLE_DEFAULT);
			(void)__xuiCodeLexerAddToken(pTokens, iTokenCapacity, pTokenCount,
				(int)iBegin, iNext, pRules[i].iTokenKind, pRules[i].iStyle);
			iPos = iNext;
			iTextStart = iPos;
			iMatched = 1;
			break;
		}
		if ( !iMatched ) {
			iNext = xuiInternalTextGraphemeNext(sText, iTextSize, iPos);
			if ( iNext <= iPos || iNext > iTextSize ) iNext = iPos + 1;
			iPos = iNext;
		}
	}
	(void)__xuiCodeLexerAddToken(pTokens, iTokenCapacity, pTokenCount,
		iTextStart, iTextSize, XUI_CODE_TOKEN_TEXT, XUI_CODE_STYLE_DEFAULT);
done:
	for ( i = 0; i < iRuleCount; i++ ) {
		if ( pRegexes[i] != NULL ) xuiXrtRegexDestroy(pRegexes[i]);
	}
	xrtFree(pRegexes);
	if ( iResult == XUI_OK ) __xuiCodeLexerSetError(sError, iErrorCapacity, "");
	return iResult;

fail:
	for ( i = 0; i < iRuleCount; i++ ) {
		if ( pRegexes[i] != NULL ) xuiXrtRegexDestroy(pRegexes[i]);
	}
	xrtFree(pRegexes);
	return XUI_ERROR_INVALID_ARGUMENT;
#endif
}

#endif
