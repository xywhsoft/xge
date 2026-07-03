#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_language_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int xuiTestLexer(xui_code_document_t* pDocument, int iStartLine, int iEndLine, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount, void* pUser)
{
	(void)iStartLine;
	(void)iEndLine;
	(void)pUser;
	if ( pDocument == NULL || pTokenCount == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pTokens != NULL && iTokenCapacity > 0 ) {
		pTokens[0].iStartOffset = 0;
		pTokens[0].iEndOffset = xuiCodeDocumentGetLength(pDocument);
		pTokens[0].iKind = XUI_CODE_TOKEN_CUSTOM_BASE + 1;
		pTokens[0].iStyle = XUI_CODE_STYLE_CUSTOM_BASE + 1;
	}
	*pTokenCount = 1;
	return XUI_OK;
}

int main(void)
{
	xui_code_language_registry pRegistry;
	xui_code_document pDocument;
	xui_code_language_t tLanguage;
	xui_code_language_t tCustom;
	xui_code_regex_rule_t tRule;
	xui_code_token_t arrTokens[16];
	xui_code_fold_range_t arrFolds[8];
	char sError[128];
	const char* arrExt[] = { "toy" };
	int iTokenCount;
	int iFoldCount;
	int iFailed;
	int iRet;

	pRegistry = NULL;
	pDocument = NULL;
	iFailed = 0;

	iRet = xuiCodeLanguageRegistryCreate(&pRegistry);
	XUI_TEST_CHECK(iRet == XUI_OK, "registry create");
	iRet = xuiCodeLanguageRegistryLoadDefaults(pRegistry);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeLanguageRegistryGetCount(pRegistry) == 1, "load defaults");
	iRet = xuiCodeLanguageRegistryFind(pRegistry, "C", &tLanguage);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tLanguage.sId, "c") == 0, "find c");
	iRet = xuiCodeLanguageRegistryFindByExtension(pRegistry, ".H", &tLanguage);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tLanguage.sLineComment, "//") == 0, "find by extension");

	iRet = xuiCodeDocumentCreate(&pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK, "document create");
	iRet = xuiCodeDocumentSetText(pDocument, "int main(void) {\nreturn 1;\n}\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "document text");
	iRet = xuiCodeLanguageLex(&tLanguage, pDocument, 0, -1, arrTokens, 16, &iTokenCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iTokenCount > 0, "c language lex");
	XUI_TEST_CHECK(arrTokens[0].iKind == XUI_CODE_TOKEN_TYPE, "c language token");
	iRet = xuiCodeLanguageFold(&tLanguage, pDocument, arrFolds, 8, &iFoldCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iFoldCount == 1, "c language fold");

	memset(&tCustom, 0, sizeof(tCustom));
	tCustom.iSize = sizeof(tCustom);
	tCustom.sId = "toy";
	tCustom.sName = "Toy";
	tCustom.arrExtensions = arrExt;
	tCustom.iExtensionCount = 1;
	tCustom.sLineComment = "#";
	tCustom.iLexerType = XUI_CODE_LEXER_CALLBACK;
	tCustom.onLex = xuiTestLexer;
	iRet = xuiCodeLanguageRegistryRegister(pRegistry, &tCustom);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeLanguageRegistryGetCount(pRegistry) == 2, "register callback language");
	iRet = xuiCodeLanguageRegistryFindByExtension(pRegistry, "toy", &tLanguage);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tLanguage.sId, "toy") == 0, "find callback language");
	iRet = xuiCodeLanguageLex(&tLanguage, pDocument, 0, -1, arrTokens, 16, &iTokenCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iTokenCount == 1 && arrTokens[0].iKind == XUI_CODE_TOKEN_CUSTOM_BASE + 1, "callback lex");

	memset(&tRule, 0, sizeof(tRule));
	tRule.iSize = sizeof(tRule);
	tRule.sName = "word";
	tRule.sPattern = "[a-z]+";
	tRule.iTokenKind = XUI_CODE_TOKEN_IDENTIFIER;
	tRule.iStyle = XUI_CODE_STYLE_IDENTIFIER;
	memset(&tCustom, 0, sizeof(tCustom));
	tCustom.iSize = sizeof(tCustom);
	tCustom.sId = "regex";
	tCustom.sName = "Regex";
	tCustom.iLexerType = XUI_CODE_LEXER_REGEX;
	tCustom.pRegexRules = &tRule;
	tCustom.iRegexRuleCount = 1;
	iRet = xuiCodeLanguageRegistryRegister(pRegistry, &tCustom);
	XUI_TEST_CHECK(iRet == XUI_OK, "register regex language");
	iRet = xuiCodeLanguageRegistryFind(pRegistry, "regex", &tLanguage);
	XUI_TEST_CHECK(iRet == XUI_OK, "find regex language");
	iRet = xuiCodeLanguageLex(&tLanguage, pDocument, 0, -1, arrTokens, 16, &iTokenCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iTokenCount > 0 && arrTokens[0].iKind == XUI_CODE_TOKEN_IDENTIFIER, "regex lex");

	xuiCodeLanguageRegistryClear(pRegistry);
	XUI_TEST_CHECK(xuiCodeLanguageRegistryGetCount(pRegistry) == 0, "clear registry");

cleanup:
	xuiCodeDocumentDestroy(pDocument);
	xuiCodeLanguageRegistryDestroy(pRegistry);
	if ( iFailed ) return 1;
	printf("xui_code_language_test passed\n");
	return 0;
}
