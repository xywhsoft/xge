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

static int __xuiCodeLexerRegexFindNext(xui_xrt_regex_t* pRegex,
	const char* sText, int iTextSize, int iStart, xregexspan* pSpan,
	int* pFound, char* sError, int iErrorCapacity)
{
	xregexspan tSpan;
	size_t iSearch;
	int iBoundary;
	int iNext;
	int iRet;

	if ( pRegex == NULL || sText == NULL || iTextSize < 0 ||
	     iStart < 0 || iStart > iTextSize || pSpan == NULL || pFound == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pFound = 0;
	iSearch = (size_t)iStart;
	while ( iSearch <= (size_t)iTextSize ) {
		iRet = xuiXrtRegexFindAt(pRegex, sText, (size_t)iTextSize,
			iSearch, &tSpan);
		if ( iRet == XREGEX_ERROR ) {
			iRet = xuiXrtRegexFailureStatus();
			__xuiCodeLexerSetError(sError, iErrorCapacity,
				iRet == XUI_ERROR_OUT_OF_MEMORY ? "out of memory" :
				"regex match failed");
			return iRet;
		}
		if ( iRet != XREGEX_MATCH ) return XUI_OK;
		if ( tSpan.Begin > (size_t)iTextSize ||
		     tSpan.End > (size_t)iTextSize || tSpan.End < tSpan.Begin ) {
			__xuiCodeLexerSetError(sError, iErrorCapacity, "invalid regex span");
			return XUI_ERROR_INVALID_STATE;
		}
		if ( tSpan.End == tSpan.Begin ) {
			__xuiCodeLexerSetError(sError, iErrorCapacity,
				"regex rule matched empty text");
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		iBoundary = xuiInternalTextGraphemeClamp(sText, iTextSize,
			(int)tSpan.Begin);
		if ( iBoundary == (int)tSpan.Begin ) {
			*pSpan = tSpan;
			*pFound = 1;
			return XUI_OK;
		}
		iNext = xuiInternalTextGraphemeNext(sText, iTextSize, iBoundary);
		if ( iNext <= (int)iSearch || iNext > iTextSize ) {
			__xuiCodeLexerSetError(sError, iErrorCapacity, "invalid regex span");
			return XUI_ERROR_INVALID_STATE;
		}
		iSearch = (size_t)iNext;
	}
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
	if ( pTokenCount == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pTokenCount = 0;
	if ( iErrorCapacity < 0 || (sError == NULL && iErrorCapacity > 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiCodeLexerSetError(sError, iErrorCapacity, "XRT regex support is disabled");
	return XUI_ERROR_UNSUPPORTED;
#else
	xui_xrt_regex_t** pRegexes;
	xregexspan tSpan = { 0 };
	xregexspan tBestSpan = { 0 };
	int i;
	int iPos;
	int iNext;
	int iFound;
	int iBestRule;
	int iResult;
	int iRet;
	size_t iTextSizeValue;

	if ( pTokenCount == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pTokenCount = 0;
	__xuiCodeLexerSetError(sError, iErrorCapacity, "");
	if ( (sText == NULL) || (pRules == NULL) || (iRuleCount <= 0) ||
	     iErrorCapacity < 0 || (sError == NULL && iErrorCapacity > 0) ||
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
		if ( pRules[i].iSize < sizeof(pRules[i]) ||
		     pRules[i].sPattern == NULL ) {
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
	iResult = XUI_OK;
	for ( i = 0; i < iRuleCount; i++ ) {
		iResult = xuiXrtRegexCreateChecked(pRules[i].sPattern,
			strlen(pRules[i].sPattern),
			pRules[i].iFlags, &pRegexes[i]);
		if ( iResult != XUI_OK ) {
			if ( iResult == XUI_ERROR_OUT_OF_MEMORY ) {
				__xuiCodeLexerSetError(sError, iErrorCapacity, "out of memory");
				goto fail;
			}
			__xuiCodeLexerSetError(sError, iErrorCapacity, "regex compile failed");
			goto fail;
		}
	}
	iPos = 0;
	while ( iPos < iTextSize ) {
		iBestRule = -1;
		for ( i = 0; i < iRuleCount; i++ ) {
			iRet = __xuiCodeLexerRegexFindNext(pRegexes[i], sText,
				iTextSize, iPos, &tSpan, &iFound, sError, iErrorCapacity);
			if ( iRet != XUI_OK ) {
				iResult = iRet;
				goto done;
			}
			if ( !iFound ) continue;
			if ( iBestRule < 0 || tSpan.Begin < tBestSpan.Begin ) {
				iBestRule = i;
				tBestSpan = tSpan;
			}
		}
		if ( iBestRule < 0 ) {
			(void)__xuiCodeLexerAddToken(pTokens, iTokenCapacity, pTokenCount,
				iPos, iTextSize, XUI_CODE_TOKEN_TEXT, XUI_CODE_STYLE_DEFAULT);
			break;
		}
		(void)__xuiCodeLexerAddToken(pTokens, iTokenCapacity, pTokenCount,
			iPos, (int)tBestSpan.Begin, XUI_CODE_TOKEN_TEXT,
			XUI_CODE_STYLE_DEFAULT);
		iNext = xuiInternalTextGraphemeClamp(sText, iTextSize,
			(int)tBestSpan.End);
		if ( iNext != (int)tBestSpan.End ) {
			iNext = xuiInternalTextGraphemeNext(sText, iTextSize, iNext);
			if ( iNext <= (int)tBestSpan.Begin || iNext > iTextSize ) {
				iNext = (int)tBestSpan.End;
			}
		} else {
			iNext = (int)tBestSpan.End;
		}
		(void)__xuiCodeLexerAddToken(pTokens, iTokenCapacity, pTokenCount,
			(int)tBestSpan.Begin, iNext, pRules[iBestRule].iTokenKind,
			pRules[iBestRule].iStyle);
		iPos = iNext;
	}
done:
	for ( i = 0; i < iRuleCount; i++ ) {
		if ( pRegexes[i] != NULL ) xuiXrtRegexDestroy(pRegexes[i]);
	}
	xrtFree(pRegexes);
	if ( iResult == XUI_OK ) __xuiCodeLexerSetError(sError, iErrorCapacity, "");
	else *pTokenCount = 0;
	return iResult;

fail:
	for ( i = 0; i < iRuleCount; i++ ) {
		if ( pRegexes[i] != NULL ) xuiXrtRegexDestroy(pRegexes[i]);
	}
	xrtFree(pRegexes);
	return iResult;
#endif
}

#endif
