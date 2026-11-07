#ifndef XUI_CODE_BRACKET_INTERNAL_H
#define XUI_CODE_BRACKET_INTERNAL_H

#include "xui.h"

enum {
	XUI_CODE_BRACKET_NONE = 0,
	XUI_CODE_BRACKET_PENDING,
	XUI_CODE_BRACKET_MATCHED,
	XUI_CODE_BRACKET_UNMATCHED
};

typedef struct xui_internal_code_bracket_state_t {
	uint32_t iDocumentVersion;
	int iCaretOffset;
	int iDocumentLength;
	int iSourceOffset;
	int iMatchOffset;
	int iScanOffset;
	int iDirection;
	int iStatus;
	int bInitialized;
	int bHasLanguage;
	int iLanguageLexerType;
	xui_code_lexer_proc onLanguageLex;
	const xui_code_regex_rule_t* pLanguageRegexRules;
	int iLanguageRegexRuleCount;
	void* pLanguageUser;
	char iSource;
	char* pStack;
	int iStackCount;
	int iStackCapacity;
	xui_code_token_t* pTokens;
	int iTokenCapacity;
	char* sBytes;
	int iByteCapacity;
} xui_internal_code_bracket_state_t;

void xuiInternalCodeBracketReset(xui_internal_code_bracket_state_t* pState);
void xuiInternalCodeBracketDestroy(xui_internal_code_bracket_state_t* pState);
int xuiInternalCodeBracketUpdate(xui_internal_code_bracket_state_t* pState,
	xui_code_document pDocument, const xui_code_language_t* pLanguage,
	int iCaretOffset, int iByteBudget, int* pChanged);

#endif
