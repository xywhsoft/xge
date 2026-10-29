#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_lexer_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int find_kind(const xui_code_token_t* pTokens, int iCount, int iKind)
{
	int i;

	for ( i = 0; i < iCount; i++ ) {
		if ( pTokens[i].iKind == iKind ) return i;
	}
	return -1;
}

static int find_span(const xui_code_token_t* pTokens, int iCount,
	int iKind, int iStart, int iEnd)
{
	int i;

	for ( i = 0; i < iCount; i++ ) {
		if ( pTokens[i].iKind == iKind && pTokens[i].iStartOffset == iStart &&
		     pTokens[i].iEndOffset == iEnd ) return i;
	}
	return -1;
}

static int test_regex_allocation_failures(const xui_code_regex_rule_t* pRule)
{
	static const size_t arrBudgets[] = {
		1u, 8u, 16u, 32u, 64u, 128u, 256u, 512u, 1024u,
		2048u, 4096u, 8192u, 16384u, 32768u, 65536u
	};
	xallocationbudget* pBudget;
	xallocationscope tScope;
	xallocationbudgetinfo tInfo;
	xui_code_token_t arrTokens[8];
	char sError[80];
	int bSawFailure;
	int bSawSuccess;
	int iCount;
	int iRet;
	size_t i;

	bSawFailure = 0;
	bSawSuccess = 0;
	for ( i = 0u; i < sizeof(arrBudgets) / sizeof(arrBudgets[0]); i++ ) {
		pBudget = NULL;
		memset(&tScope, 0, sizeof(tScope));
		memset(&tInfo, 0, sizeof(tInfo));
		tInfo.Size = sizeof(tInfo);
		if ( !xrtAllocationBudgetCreate(arrBudgets[i], &pBudget) ||
		     !xrtAllocationBudgetEnter(pBudget, &tScope) ) {
			xrtAllocationBudgetFree(pBudget);
			return 0;
		}
		iCount = 77;
		memset(sError, 0, sizeof(sError));
		iRet = xuiCodeLexerRegexTokenize("alpha", 5, pRule, 1, arrTokens,
			8, &iCount, sError, sizeof(sError));
		if ( !xrtAllocationBudgetLeave(&tScope) ||
		     !xrtAllocationBudgetInfo(pBudget, &tInfo) ) {
			xrtAllocationBudgetFree(pBudget);
			return 0;
		}
		xrtAllocationBudgetFree(pBudget);
		if ( tInfo.LiveBytes != 0u || tInfo.LiveCount != 0u ) return 0;
		if ( iRet == XUI_OK ) {
			if ( iCount != 1 ) return 0;
			bSawSuccess = 1;
			break;
		}
		if ( tInfo.Rejections == 0u || iRet != XUI_ERROR_OUT_OF_MEMORY ||
		     strcmp(sError, "out of memory") != 0 || iCount != 0 ) return 0;
		bSawFailure = 1;
	}
	return bSawFailure && bSawSuccess;
}

static int test_regex_sparse_match(const xui_code_regex_rule_t* pRule)
{
	xui_code_regex_rule_t tRule;
	xui_code_token_t arrTokens[2];
	char* sText;
	char sError[80];
	int iCount;
	int iRet;
	int i;

	sText = (char*)xrtMalloc(65537u);
	if ( sText == NULL ) return 0;
	for ( i = 0; i < 65535; i++ ) sText[i] = 'a';
	sText[65535] = 'Z';
	sText[65536] = '\0';
	tRule = *pRule;
	tRule.sPattern = "Z";
	iRet = xuiCodeLexerRegexTokenize(sText, 65536, &tRule, 1,
		arrTokens, 2, &iCount, sError, sizeof(sError));
	xrtFree(sText);
	return iRet == XUI_OK && iCount == 2 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_TEXT &&
		arrTokens[0].iStartOffset == 0 && arrTokens[0].iEndOffset == 65535 &&
		arrTokens[1].iKind == tRule.iTokenKind &&
		arrTokens[1].iStartOffset == 65535 && arrTokens[1].iEndOffset == 65536;
}

static int test_document_lexer_allocation_failures(void)
{
	static const size_t arrBudgets[] = {
		1u, 8u, 16u, 32u, 64u, 128u, 256u, 512u, 1024u,
		2048u, 4096u, 8192u, 16384u, 32768u, 65536u
	};
	xallocationbudget* pBudget;
	xallocationscope tScope;
	xallocationbudgetinfo tInfo;
	xui_code_document pDocument;
	xui_code_token_t arrTokens[8];
	int bSawFailure;
	int bSawSuccess;
	int iCount;
	int iStart;
	int iEnd;
	int iRet;
	size_t i;

	bSawFailure = 0;
	bSawSuccess = 0;
	for ( i = 0u; i < sizeof(arrBudgets) / sizeof(arrBudgets[0]); i++ ) {
		pBudget = NULL;
		pDocument = NULL;
		memset(&tScope, 0, sizeof(tScope));
		memset(&tInfo, 0, sizeof(tInfo));
		tInfo.Size = sizeof(tInfo);
		if ( !xrtAllocationBudgetCreate(arrBudgets[i], &pBudget) ||
		     !xrtAllocationBudgetEnter(pBudget, &tScope) ) {
			xrtAllocationBudgetFree(pBudget);
			return 0;
		}
		iRet = xuiCodeDocumentCreate(&pDocument);
		if ( iRet == XUI_OK ) {
			iRet = xuiCodeDocumentSetText(pDocument,
				"/* start\ninside\nend */\n");
		}
		if ( iRet == XUI_OK ) {
			iRet = xuiCodeDocumentGetLineRange(pDocument, 1, &iStart, &iEnd);
		}
		if ( iRet == XUI_OK ) {
			iRet = xuiCodeLexerCTokenizeDocumentRange(pDocument, iStart, iEnd,
				arrTokens, 8, &iCount);
		}
		xuiCodeDocumentDestroy(pDocument);
		if ( !xrtAllocationBudgetLeave(&tScope) ||
		     !xrtAllocationBudgetInfo(pBudget, &tInfo) ) {
			xrtAllocationBudgetFree(pBudget);
			return 0;
		}
		xrtAllocationBudgetFree(pBudget);
		if ( tInfo.LiveBytes != 0u || tInfo.LiveCount != 0u ) return 0;
		if ( iRet == XUI_OK ) {
			if ( iCount != 1 || arrTokens[0].iKind != XUI_CODE_TOKEN_COMMENT ) {
				return 0;
			}
			bSawSuccess = 1;
			break;
		}
		if ( iRet != XUI_ERROR_OUT_OF_MEMORY || tInfo.Rejections == 0u ) return 0;
		bSawFailure = 1;
	}
	return bSawFailure && bSawSuccess;
}

int main(void)
{
	const char* sCode;
	xui_code_document pDocument;
	xui_code_token_t arrTokens[128];
	xui_code_regex_rule_t arrRules[3];
	char sError[128];
	int iCount;
	int iStart;
	int iEnd;
	int iIndex;
	int iFailed;
	int iRet;

	iFailed = 0;
	pDocument = NULL;
	memset(arrTokens, 0, sizeof(arrTokens));

	sCode =
		"#include <stdio.h>\n"
		"int main(void) {\n"
		"  // hello\n"
		"  const char* s = \"text\";\n"
		"  return 42;\n"
		"}\n";
	iRet = xuiCodeLexerCTokenize(sCode, -1, arrTokens, (int)(sizeof(arrTokens) / sizeof(arrTokens[0])), &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "c tokenize");
	XUI_TEST_CHECK(iCount > 10, "c token count");
	XUI_TEST_CHECK(find_kind(arrTokens, iCount, XUI_CODE_TOKEN_PREPROCESSOR) >= 0, "preprocessor token");
	XUI_TEST_CHECK(find_kind(arrTokens, iCount, XUI_CODE_TOKEN_TYPE) >= 0, "type token");
	XUI_TEST_CHECK(find_kind(arrTokens, iCount, XUI_CODE_TOKEN_KEYWORD) >= 0, "keyword token");
	XUI_TEST_CHECK(find_kind(arrTokens, iCount, XUI_CODE_TOKEN_COMMENT) >= 0, "comment token");
	XUI_TEST_CHECK(find_kind(arrTokens, iCount, XUI_CODE_TOKEN_STRING) >= 0, "string token");
	XUI_TEST_CHECK(find_kind(arrTokens, iCount, XUI_CODE_TOKEN_NUMBER) >= 0, "number token");
	XUI_TEST_CHECK(find_kind(arrTokens, iCount, XUI_CODE_TOKEN_BRACE) >= 0, "brace token");

	iCount = 0;
	iRet = xuiCodeLexerCTokenize("int x = 1;", -1, arrTokens, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount > 2, "capacity-independent token count");

	memset(arrTokens, 0, sizeof(arrTokens));
	iRet = xuiCodeLexerCTokenizeRange("int a;\nfloat b;\n", -1, 7, 15, arrTokens, 2, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount > 2, "range token count");
	XUI_TEST_CHECK(arrTokens[0].iKind == XUI_CODE_TOKEN_TYPE && arrTokens[0].iStartOffset == 7 && arrTokens[0].iEndOffset == 12, "range absolute offsets");
	iRet = xuiCodeLexerCTokenizeRange("/*x*/", -1, 4, 5,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_COMMENT &&
		arrTokens[0].iStartOffset == 4 && arrTokens[0].iEndOffset == 5,
		"range inherits pending block comment close");
	iRet = xuiCodeLexerCTokenizeRange("/*abc*/", -1, 1, 7,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_COMMENT &&
		arrTokens[0].iStartOffset == 1 && arrTokens[0].iEndOffset == 7,
		"range inherits pending block comment opener");
	iRet = xuiCodeLexerCTokenizeRange("//abc", -1, 1, 5,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_COMMENT &&
		arrTokens[0].iStartOffset == 1 && arrTokens[0].iEndOffset == 5,
		"range inherits pending line comment opener");
	iRet = xuiCodeLexerCTokenizeRange("\"a\\\"b\"", -1, 3, 5,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_STRING &&
		arrTokens[0].iStartOffset == 3 && arrTokens[0].iEndOffset == 5,
		"range inherits pending string escape");
	iRet = xuiCodeLexerCTokenizeRange("int x; //abc\n", -1, 10, 12,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_COMMENT &&
		arrTokens[0].iStartOffset == 10 && arrTokens[0].iEndOffset == 12,
		"range inherits line comment state");
	iRet = xuiCodeLexerCTokenizeRange("x #define Y", -1, 2, 9,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		find_kind(arrTokens, iCount, XUI_CODE_TOKEN_PREPROCESSOR) < 0,
		"range keeps non-directive line context");

	memset(arrTokens, 0, sizeof(arrTokens));
	iRet = xuiCodeLexerCTokenize("int \xE5\x8F\x98\xE9\x87\x8F = 1;", -1, arrTokens, (int)(sizeof(arrTokens) / sizeof(arrTokens[0])), &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "c tokenize unicode");
	XUI_TEST_CHECK(find_span(arrTokens, iCount, XUI_CODE_TOKEN_IDENTIFIER, 4, 10) >= 0,
		"unicode identifier keeps utf8 span");

	memset(arrTokens, 0, sizeof(arrTokens));
	iRet = xuiCodeLexerCTokenize("e\xCC\x81 = 1;", -1, arrTokens,
		(int)(sizeof(arrTokens) / sizeof(arrTokens[0])), &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		find_span(arrTokens, iCount, XUI_CODE_TOKEN_IDENTIFIER, 0, 3) >= 0,
		"combining identifier keeps grapheme span");
	iRet = xuiCodeLexerCTokenize("/*x", -1, arrTokens,
		(int)(sizeof(arrTokens) / sizeof(arrTokens[0])), &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_COMMENT && arrTokens[0].iEndOffset == 3,
		"unterminated block comment includes final byte");
	iRet = xuiCodeLexerCTokenize("1+2", -1, arrTokens,
		(int)(sizeof(arrTokens) / sizeof(arrTokens[0])), &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 3 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_NUMBER &&
		arrTokens[1].iKind == XUI_CODE_TOKEN_OPERATOR &&
		arrTokens[2].iKind == XUI_CODE_TOKEN_NUMBER,
		"number does not consume binary operator");
	iRet = xuiCodeLexerCTokenize(".5", -1, arrTokens,
		(int)(sizeof(arrTokens) / sizeof(arrTokens[0])), &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_NUMBER &&
		arrTokens[0].iStartOffset == 0 && arrTokens[0].iEndOffset == 2,
		"fraction beginning with dot is one number");
	iRet = xuiCodeLexerCTokenize("x # not_a_directive", -1, arrTokens,
		(int)(sizeof(arrTokens) / sizeof(arrTokens[0])), &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		find_kind(arrTokens, iCount, XUI_CODE_TOKEN_PREPROCESSOR) < 0,
		"hash after code is not a preprocessor directive");
	iRet = xuiCodeLexerCTokenize("  #define X 1", -1, arrTokens,
		(int)(sizeof(arrTokens) / sizeof(arrTokens[0])), &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		find_kind(arrTokens, iCount, XUI_CODE_TOKEN_PREPROCESSOR) >= 0,
		"leading whitespace preserves directive start");
	iRet = xuiCodeLexerCTokenize("/* banner */ #define X 1", -1, arrTokens,
		(int)(sizeof(arrTokens) / sizeof(arrTokens[0])), &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		find_kind(arrTokens, iCount, XUI_CODE_TOKEN_PREPROCESSOR) >= 0,
		"leading comment preserves directive start");
	iRet = xuiCodeLexerCTokenize("\"abc\nint", -1, arrTokens,
		(int)(sizeof(arrTokens) / sizeof(arrTokens[0])), &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		find_span(arrTokens, iCount, XUI_CODE_TOKEN_STRING, 0, 4) >= 0 &&
		find_span(arrTokens, iCount, XUI_CODE_TOKEN_TYPE, 5, 8) >= 0,
		"unescaped newline terminates string highlighting");
	iRet = xuiCodeLexerCTokenize("x", -1, NULL, 1, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "missing token buffer rejected");

	iRet = xuiCodeDocumentCreate(&pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK, "document create for range lexer");
	iRet = xuiCodeDocumentSetText(pDocument, "int \xE5\x8F\x98\xE9\x87\x8F;");
	XUI_TEST_CHECK(iRet == XUI_OK, "document range text");
	iCount = 77;
	iRet = xuiCodeLexerCTokenizeDocumentRange(pDocument, 99, 199,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 0,
		"document range beyond end clamps to empty");
	iRet = xuiCodeLexerCTokenizeDocumentRange(pDocument, 0, -1,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		find_span(arrTokens, iCount, XUI_CODE_TOKEN_TYPE, 0, 3) >= 0,
		"negative document range end uses document end");
	iRet = xuiCodeLexerCTokenizeDocumentRange(pDocument, 5, 10,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iStartOffset == 7 && arrTokens[0].iEndOffset == 10,
		"document range start advances to next utf8 grapheme");
	iCount = 77;
	iRet = xuiCodeLexerCTokenizeDocumentRange(NULL, 0, 0,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iCount == 0,
		"document range failure clears token count");

	iRet = xuiCodeDocumentSetText(pDocument,
		"int x;\n/* start\ninside\nend */\nint y;\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "multiline lexical state text");
	iRet = xuiCodeDocumentGetLineRange(pDocument, 2, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK, "multiline comment line range");
	iRet = xuiCodeLexerCTokenizeDocumentRange(pDocument, iStart, iEnd,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_COMMENT &&
		arrTokens[0].iStartOffset == iStart && arrTokens[0].iEndOffset == iEnd,
		"document range inherits multiline comment state");
	iRet = xuiCodeDocumentDelete(pDocument, 7, 9);
	XUI_TEST_CHECK(iRet == XUI_OK, "remove multiline comment opener");
	iRet = xuiCodeDocumentGetLineRange(pDocument, 2, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK, "edited line range");
	iRet = xuiCodeLexerCTokenizeDocumentRange(pDocument, iStart, iEnd,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		find_kind(arrTokens, iCount, XUI_CODE_TOKEN_COMMENT) < 0,
		"document edit invalidates following lexical states");
	iRet = xuiCodeDocumentSetText(pDocument,
		"#define X \\\n  1\nconst char* s = \"abc\\\ncontinued\";\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "continued lexical state text");
	iRet = xuiCodeDocumentGetLineRange(pDocument, 1, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK, "continued preprocessor line range");
	iRet = xuiCodeLexerCTokenizeDocumentRange(pDocument, iStart, iEnd,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_PREPROCESSOR,
		"document range inherits continued preprocessor state");
	iRet = xuiCodeDocumentGetLineRange(pDocument, 3, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK, "continued string line range");
	iRet = xuiCodeLexerCTokenizeDocumentRange(pDocument, iStart, iEnd,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		find_kind(arrTokens, iCount, XUI_CODE_TOKEN_STRING) >= 0,
		"document range inherits continued string state");
	iRet = xuiCodeDocumentSetText(pDocument,
		"int x; // comment\n\"a\\\"b\"\n/*x*/\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "mid-token lexical state text");
	iRet = xuiCodeLexerCTokenizeDocumentRange(pDocument, 12, 17,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_COMMENT &&
		arrTokens[0].iStartOffset == 12 && arrTokens[0].iEndOffset == 17,
		"document range inherits line comment state mid-line");
	iRet = xuiCodeLexerCTokenizeDocumentRange(pDocument, 21, 23,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_STRING,
		"document range preserves pending string escape");
	iRet = xuiCodeLexerCTokenizeDocumentRange(pDocument, 29, 30,
		arrTokens, 128, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_COMMENT,
		"document range preserves pending block close");
	XUI_TEST_CHECK(test_document_lexer_allocation_failures(),
		"document lexer cache allocation failures are leak free");

	memset(arrRules, 0, sizeof(arrRules));
	arrRules[0].iSize = sizeof(arrRules[0]);
	arrRules[0].sName = "number";
	arrRules[0].sPattern = "[0-9]+";
	arrRules[0].iTokenKind = XUI_CODE_TOKEN_NUMBER;
	arrRules[0].iStyle = XUI_CODE_TOKEN_NUMBER;
	arrRules[1].iSize = sizeof(arrRules[1]);
	arrRules[1].sName = "identifier";
	arrRules[1].sPattern = "[A-Za-z_][A-Za-z0-9_]*";
	arrRules[1].iTokenKind = XUI_CODE_TOKEN_IDENTIFIER;
	arrRules[1].iStyle = XUI_CODE_TOKEN_IDENTIFIER;
	arrRules[2].iSize = sizeof(arrRules[2]);
	arrRules[2].sName = "operator";
	arrRules[2].sPattern = "[=+]+";
	arrRules[2].iTokenKind = XUI_CODE_TOKEN_OPERATOR;
	arrRules[2].iStyle = XUI_CODE_TOKEN_OPERATOR;
	memset(sError, 0, sizeof(sError));
	iRet = xuiCodeLexerRegexTokenize("abc = 123", -1, arrRules, 3, arrTokens, (int)(sizeof(arrTokens) / sizeof(arrTokens[0])), &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK, "regex tokenize");
	iIndex = find_kind(arrTokens, iCount, XUI_CODE_TOKEN_IDENTIFIER);
	XUI_TEST_CHECK(iIndex >= 0 && arrTokens[iIndex].iStartOffset == 0 && arrTokens[iIndex].iEndOffset == 3, "regex identifier");
	iIndex = find_kind(arrTokens, iCount, XUI_CODE_TOKEN_NUMBER);
	XUI_TEST_CHECK(iIndex >= 0 && arrTokens[iIndex].iStartOffset == 6 && arrTokens[iIndex].iEndOffset == 9, "regex number");
	iIndex = find_kind(arrTokens, iCount, XUI_CODE_TOKEN_OPERATOR);
	XUI_TEST_CHECK(iIndex >= 0, "regex operator");
	XUI_TEST_CHECK(test_regex_allocation_failures(&arrRules[1]),
		"regex lexer reports allocation failures transactionally");
	XUI_TEST_CHECK(test_regex_sparse_match(&arrRules[0]),
		"regex lexer skips a long unmatched span in one token");

	arrRules[0].iSize = sizeof(arrRules[0]) - 1u;
	iCount = 77;
	iRet = xuiCodeLexerRegexTokenize("123", -1, arrRules, 1,
		arrTokens, 16, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iCount == 0,
		"undersized regex rule rejected transactionally");
	arrRules[0].iSize = sizeof(arrRules[0]);
	arrRules[0].sPattern = "a*";
	iCount = 77;
	iRet = xuiCodeLexerRegexTokenize("bbb", -1, arrRules, 1,
		arrTokens, 16, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iCount == 0 &&
		strstr(sError, "empty") != NULL,
		"empty regex match rejected transactionally");

	arrRules[0].sPattern = "[0-9]+";
	iRet = xuiCodeLexerRegexTokenize("e\xCC\x81", -1, arrRules, 1,
		arrTokens, 16, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_TEXT &&
		arrTokens[0].iStartOffset == 0 && arrTokens[0].iEndOffset == 3,
		"regex unmatched grapheme is one text token");
	arrRules[0].sPattern = "e";
	iRet = xuiCodeLexerRegexTokenize("e\xCC\x81", -1, arrRules, 1,
		arrTokens, 16, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrTokens[0].iKind == XUI_CODE_TOKEN_NUMBER && arrTokens[0].iEndOffset == 3,
		"regex token expands to grapheme boundary");
	iRet = xuiCodeLexerRegexTokenize("abc", -1, arrRules, 0,
		arrTokens, 16, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "empty regex rules rejected");

	arrRules[0].sPattern = "[";
	iRet = xuiCodeLexerRegexTokenize("abc", -1, arrRules, 1, arrTokens, 16, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet != XUI_OK && strlen(sError) > 0, "regex compile error");

cleanup:
	xuiCodeDocumentDestroy(pDocument);
	if ( iFailed ) return 1;
	printf("xui_code_lexer_test passed\n");
	return 0;
}
