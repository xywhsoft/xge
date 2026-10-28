#include "xui.h"

#include <stdio.h>
#include <string.h>

static int g_iLanguageFailAllocation = -1;
static int g_bLanguageAllocationFailed = 0;

static void* xuiTestLanguageMalloc(size_t iSize)
{
	if ( g_iLanguageFailAllocation >= 0 && g_iLanguageFailAllocation-- == 0 ) {
		g_bLanguageAllocationFailed = 1;
		return NULL;
	}
	return xrtMalloc(iSize);
}

static void* xuiTestLanguageRealloc(void* pMemory, size_t iSize)
{
	if ( g_iLanguageFailAllocation >= 0 && g_iLanguageFailAllocation-- == 0 ) {
		g_bLanguageAllocationFailed = 1;
		return NULL;
	}
	return xrtRealloc(pMemory, iSize);
}

#define xrtMalloc xuiTestLanguageMalloc
#define xrtRealloc xuiTestLanguageRealloc
#include "../../src/xui/code/xui_code_language.c"
#undef xrtMalloc
#undef xrtRealloc

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

static int xuiTestInvalidLexer(xui_code_document_t* pDocument, int iStartLine,
	int iEndLine, xui_code_token_t* pTokens, int iTokenCapacity,
	int* pTokenCount, void* pUser)
{
	(void)pDocument;
	(void)iStartLine;
	(void)iEndLine;
	(void)pTokens;
	(void)iTokenCapacity;
	(void)pUser;
	*pTokenCount = -1;
	return XUI_OK;
}

static int xuiTestInvalidFold(xui_code_document_t* pDocument,
	xui_code_fold_range_t* pRanges, int iRangeCapacity, int* pRangeCount,
	void* pUser)
{
	(void)pDocument;
	(void)pRanges;
	(void)iRangeCapacity;
	(void)pUser;
	*pRangeCount = -1;
	return XUI_OK;
}

static int xuiTestLanguageAllocationRollback(const xui_code_language_t* pLanguage)
{
	xui_code_language_registry pRegistry;
	int iRet;
	int iFail;
	int bSucceeded;

	bSucceeded = 0;
	for ( iFail = 0; iFail < 32; iFail++ ) {
		pRegistry = NULL;
		g_iLanguageFailAllocation = -1;
		if ( xuiCodeLanguageRegistryCreate(&pRegistry) != XUI_OK ) return 0;
		g_iLanguageFailAllocation = iFail;
		g_bLanguageAllocationFailed = 0;
		iRet = xuiCodeLanguageRegistryRegister(pRegistry, pLanguage);
		g_iLanguageFailAllocation = -1;
		if ( iRet == XUI_OK ) {
			bSucceeded = xuiCodeLanguageRegistryGetCount(pRegistry) == 1;
			xuiCodeLanguageRegistryDestroy(pRegistry);
			break;
		}
		if ( iRet != XUI_ERROR_OUT_OF_MEMORY || !g_bLanguageAllocationFailed ||
		     xuiCodeLanguageRegistryGetCount(pRegistry) != 0 ) {
			xuiCodeLanguageRegistryDestroy(pRegistry);
			return 0;
		}
		xuiCodeLanguageRegistryDestroy(pRegistry);
	}
	return bSucceeded;
}

int main(void)
{
	xui_code_language_registry pRegistry;
	xui_code_document pDocument;
	xui_code_language_t tLanguage;
	xui_code_language_t tCustom;
	xui_code_language_t tInvalid;
	xui_code_regex_rule_t tRule;
	xui_code_regex_rule_t arrFailureRules[2];
	xui_code_token_t arrTokens[16];
	xui_code_fold_range_t arrFolds[8];
	char sError[128];
	const char* arrExt[] = { "toy" };
	const char* arrFailureExt[] = { ".fail", "failure" };
	const char* arrInvalidExt[] = { "." };
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
	iRet = xuiCodeDocumentInsert(pDocument, 8, "x");
	XUI_TEST_CHECK(iRet == XUI_OK, "document gap insert");
	iRet = xuiCodeDocumentDelete(pDocument, 8, 9);
	XUI_TEST_CHECK(iRet == XUI_OK, "document gap restore");
	iRet = xuiCodeLanguageLex(&tLanguage, pDocument, 1, 1, arrTokens, 16, &iTokenCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iTokenCount > 0 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_KEYWORD && arrTokens[0].iStartOffset == 17,
		"c language line range lex across gap");
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

	memset(arrFailureRules, 0, sizeof(arrFailureRules));
	arrFailureRules[0].iSize = sizeof(arrFailureRules[0]);
	arrFailureRules[0].sName = "word";
	arrFailureRules[0].sPattern = "[a-z]+";
	arrFailureRules[1].iSize = sizeof(arrFailureRules[1]);
	arrFailureRules[1].sName = "number";
	arrFailureRules[1].sPattern = "[0-9]+";
	memset(&tInvalid, 0, sizeof(tInvalid));
	tInvalid.iSize = sizeof(tInvalid);
	tInvalid.sId = "allocation-failure";
	tInvalid.sName = "Allocation Failure";
	tInvalid.arrExtensions = arrFailureExt;
	tInvalid.iExtensionCount = 2;
	tInvalid.sLineComment = "//";
	tInvalid.sBlockCommentStart = "/*";
	tInvalid.sBlockCommentEnd = "*/";
	tInvalid.iLexerType = XUI_CODE_LEXER_REGEX;
	tInvalid.pRegexRules = arrFailureRules;
	tInvalid.iRegexRuleCount = 2;
	XUI_TEST_CHECK(xuiTestLanguageAllocationRollback(&tInvalid),
		"register allocation rollback");

	tInvalid.arrExtensions = arrInvalidExt;
	tInvalid.iExtensionCount = 1;
	iRet = xuiCodeLanguageRegistryRegister(pRegistry, &tInvalid);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "empty extension rejected");
	tInvalid.arrExtensions = arrFailureExt;
	tInvalid.iExtensionCount = 2;
	arrFailureRules[0].sPattern = NULL;
	iRet = xuiCodeLanguageRegistryRegister(pRegistry, &tInvalid);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "missing regex pattern rejected");
	arrFailureRules[0].sPattern = "[a-z]+";

	iRet = xuiCodeLanguageLex(&tLanguage, pDocument, 0, -1, arrTokens, -1,
		&iTokenCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "negative token capacity rejected");
	iRet = xuiCodeLanguageFold(&tLanguage, pDocument, arrFolds, -1, &iFoldCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "negative fold capacity rejected");
	memset(&tInvalid, 0, sizeof(tInvalid));
	tInvalid.onLex = xuiTestInvalidLexer;
	tInvalid.onFold = xuiTestInvalidFold;
	iRet = xuiCodeLanguageLex(&tInvalid, pDocument, 0, -1, NULL, 0,
		&iTokenCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && iTokenCount == 0,
		"negative callback token count rejected");
	iRet = xuiCodeLanguageFold(&tInvalid, pDocument, NULL, 0, &iFoldCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && iFoldCount == 0,
		"negative callback fold count rejected");

	xuiCodeLanguageRegistryClear(pRegistry);
	XUI_TEST_CHECK(xuiCodeLanguageRegistryGetCount(pRegistry) == 0, "clear registry");

cleanup:
	xuiCodeDocumentDestroy(pDocument);
	xuiCodeLanguageRegistryDestroy(pRegistry);
	if ( iFailed ) return 1;
	printf("xui_code_language_test passed\n");
	return 0;
}
