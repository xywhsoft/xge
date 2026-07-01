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

int main(void)
{
	const char* sCode;
	xui_code_token_t arrTokens[128];
	xui_code_regex_rule_t arrRules[3];
	char sError[128];
	int iCount;
	int iIndex;
	int iFailed;
	int iRet;

	iFailed = 0;
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

	memset(arrTokens, 0, sizeof(arrTokens));
	iRet = xuiCodeLexerCTokenize("int \xE5\x8F\x98\xE9\x87\x8F = 1;", -1, arrTokens, (int)(sizeof(arrTokens) / sizeof(arrTokens[0])), &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "c tokenize unicode");
	iIndex = find_kind(arrTokens, iCount, XUI_CODE_TOKEN_TEXT);
	XUI_TEST_CHECK(iIndex >= 0 && arrTokens[iIndex].iStartOffset == 4 && arrTokens[iIndex].iEndOffset == 10, "unicode token keeps utf8 span");

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

	arrRules[0].sPattern = "[";
	iRet = xuiCodeLexerRegexTokenize("abc", -1, arrRules, 1, arrTokens, 16, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet != XUI_OK && strlen(sError) > 0, "regex compile error");

cleanup:
	if ( iFailed ) return 1;
	printf("xui_code_lexer_test passed\n");
	return 0;
}
