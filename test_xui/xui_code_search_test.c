#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_search_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

int main(void)
{
	xui_code_document pDocument;
	xui_code_range_t tRange;
	xui_code_search_result_t tResult;
	char* sLarge;
	char sError[128];
	int iCount;
	int iFailed;
	int iRet;

	pDocument = NULL;
	sLarge = NULL;
	iFailed = 0;

	iRet = xuiCodeDocumentCreate(&pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK, "document create");
	iRet = xuiCodeDocumentSetText(pDocument, "alpha beta Alpha alphabet\nfoo_1 foo_2\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "document text");

	iRet = xuiCodeSearchFindPlain(pDocument, "ALPHA", 0, 0, &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tRange.iStart == 0 && tRange.iEnd == 5, "plain case insensitive");
	iRet = xuiCodeSearchFindPlain(pDocument, "Alpha", 0, XUI_CODE_SEARCH_CASE_SENSITIVE, &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tRange.iStart == 11, "plain case sensitive");
	iRet = xuiCodeSearchFindPlain(pDocument, "alpha", 1, XUI_CODE_SEARCH_WHOLE_WORD, &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tRange.iStart == 11, "whole word skips alphabet");
	iRet = xuiCodeSearchFindPlain(pDocument, "alpha", 8, XUI_CODE_SEARCH_BACKWARD, &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tRange.iStart == 0, "plain backward");
	iRet = xuiCodeSearchFindPlain(pDocument, "beta", 30, XUI_CODE_SEARCH_WRAP, &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tRange.iStart == 6, "plain wrap");
	iRet = xuiCodeSearchFindPlain(pDocument, "missing", 0, 0, &tRange);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED, "plain missing");
	iRet = xuiCodeSearchFindPlainRange(pDocument, "alpha", 0, 10, 25, 0, &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tRange.iStart == 11 && tRange.iEnd == 16, "plain range forward");
	iRet = xuiCodeSearchFindPlainRange(pDocument, "alpha", 25, 10, 25, XUI_CODE_SEARCH_BACKWARD, &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tRange.iStart == 17, "plain range backward");
	iRet = xuiCodeSearchFindPlainRange(pDocument, "beta", 20, 10, 25, XUI_CODE_SEARCH_WRAP, &tRange);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED, "plain range excludes outside match");
	iRet = xuiCodeSearchFindPlainRange(pDocument, "alpha", 24, 10, 25, XUI_CODE_SEARCH_WRAP, &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tRange.iStart == 11, "plain range wrap");

	iRet = xuiCodeDocumentSetText(pDocument,
		"\xE5\x8F\x98\xE9\x87\x8F \xE5\x8F\x98\xE9\x87\x8Fx \xE5\x8F\x98\xE9\x87\x8F");
	XUI_TEST_CHECK(iRet == XUI_OK, "set Unicode whole word document");
	iRet = xuiCodeSearchFindPlain(pDocument, "\xE5\x8F\x98\xE9\x87\x8F", 1,
		XUI_CODE_SEARCH_CASE_SENSITIVE | XUI_CODE_SEARCH_WHOLE_WORD, &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tRange.iStart == 15,
		"Unicode whole word skips longer identifier");
	iRet = xuiCodeDocumentSetText(pDocument, "alpha beta Alpha alphabet\nfoo_1 foo_2\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore document before regex tests");

	memset(&tResult, 0, sizeof(tResult));
	memset(sError, 0, sizeof(sError));
	iRet = xuiCodeSearchFindRegex(pDocument, "foo_([0-9])", 0, XUI_CODE_SEARCH_CASE_SENSITIVE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK, "regex find");
	XUI_TEST_CHECK(tResult.iStart == 26 && tResult.iEnd == 31, "regex range");
	XUI_TEST_CHECK(tResult.iCaptureCount >= 2, "regex capture count");
	XUI_TEST_CHECK(tResult.arrCaptures[1].iStart == 30 && tResult.arrCaptures[1].iEnd == 31, "regex capture range");

	iRet = xuiCodeSearchFindRegex(pDocument, "foo_([0-9])", xuiCodeDocumentGetLength(pDocument), XUI_CODE_SEARCH_BACKWARD | XUI_CODE_SEARCH_CASE_SENSITIVE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 32, "regex backward");
	iRet = xuiCodeDocumentInsert(pDocument, 29, "x");
	XUI_TEST_CHECK(iRet == XUI_OK, "regex range gap insert");
	iRet = xuiCodeDocumentDelete(pDocument, 29, 30);
	XUI_TEST_CHECK(iRet == XUI_OK, "regex range gap restore");
	iRet = xuiCodeSearchFindRegexRange(pDocument, "foo_([0-9])", 30, 25, 38,
		XUI_CODE_SEARCH_CASE_SENSITIVE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 32 &&
		tResult.arrCaptures[1].iStart == 36 && tResult.arrCaptures[1].iEnd == 37,
		"regex range across gap");
	iRet = xuiCodeSearchFindRegex(pDocument, "foo_([0-9])", 34, XUI_CODE_SEARCH_WRAP | XUI_CODE_SEARCH_CASE_SENSITIVE, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && tResult.iStart == 26, "regex wrap");
	iRet = xuiCodeSearchFindRegex(pDocument, "foo_([0-9]", 0, 0, &tResult, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && sError[0] != '\0', "regex invalid pattern");

	sLarge = (char*)xrtMalloc(70001u);
	XUI_TEST_CHECK(sLarge != NULL, "allocate chunk boundary text");
	memset(sLarge, 'a', 70000u);
	memcpy(sLarge + 65534, "needle", 6u);
	sLarge[70000] = '\0';
	iRet = xuiCodeDocumentSetTextLength(pDocument, sLarge, 70000);
	XUI_TEST_CHECK(iRet == XUI_OK, "set chunk boundary text");
	iRet = xuiCodeSearchFindPlain(pDocument, "needle", 0,
		XUI_CODE_SEARCH_CASE_SENSITIVE, &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tRange.iStart == 65534, "plain search crosses chunk boundary");
	iRet = xuiCodeSearchFindPlain(pDocument, "needle", 70000,
		XUI_CODE_SEARCH_CASE_SENSITIVE | XUI_CODE_SEARCH_BACKWARD, &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tRange.iStart == 65534, "backward search crosses chunk boundary");
	xrtFree(sLarge);
	sLarge = NULL;

	iRet = xuiCodeDocumentSetText(pDocument, "alpha beta Alpha alphabet\nfoo_1 foo_2\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore search document");
	iRet = xuiCodeSearchReplaceAllPlain(pDocument, "foo", "bar", XUI_CODE_SEARCH_CASE_SENSITIVE, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "plain replace count");
	XUI_TEST_CHECK(strstr(xuiCodeDocumentGetText(pDocument), "bar_1 bar_2") != NULL, "plain replace text");

	iRet = xuiCodeDocumentSetText(pDocument, "int a = 1;\nint b = 2;\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "reset document");
	iRet = xuiCodeSearchReplaceAllRegex(pDocument, "int ([a-z]) = ([0-9]);", "long $1 = $2;", XUI_CODE_SEARCH_CASE_SENSITIVE, &iCount, sError, sizeof(sError));
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "regex replace count");
	XUI_TEST_CHECK(strcmp(xuiCodeDocumentGetText(pDocument), "long a = 1;\nlong b = 2;\n") == 0, "regex replace text");

cleanup:
	xrtFree(sLarge);
	xuiCodeDocumentDestroy(pDocument);
	if ( iFailed ) return 1;
	printf("xui_code_search_test passed\n");
	return 0;
}
