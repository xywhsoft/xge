#include "xui.h"

#include <stdio.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_fold_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int find_range_with_flags(xui_code_fold_range_t* pRanges, int iCount, uint32_t iFlags)
{
	int i;

	for ( i = 0; i < iCount; i++ ) {
		if ( (pRanges[i].iFlags & iFlags) == iFlags ) return i;
	}
	return -1;
}

int main(void)
{
	const char* sCode;
	xui_code_document pDocument;
	xui_code_fold_range_t arrRanges[32];
	int arrVisible[16];
	int iCount;
	int iVisible;
	int iIndex;
	int iFailed;
	int iRet;

	pDocument = NULL;
	iFailed = 0;
	sCode =
		"#if ENABLED\n"
		"int main(void) {\n"
		"  /* block\n"
		"     comment */\n"
		"  if (1) {\n"
		"    return 0;\n"
		"  }\n"
		"}\n"
		"#endif\n";

	iRet = xuiCodeFoldCBuildRanges(sCode, -1, arrRanges, 32, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "build c folds");
	XUI_TEST_CHECK(iCount >= 4, "fold count");
	iIndex = find_range_with_flags(arrRanges, iCount, XUI_CODE_FOLD_COMMENT);
	XUI_TEST_CHECK(iIndex >= 0 && arrRanges[iIndex].iStartLine == 2 && arrRanges[iIndex].iEndLine == 3, "comment fold");
	iIndex = find_range_with_flags(arrRanges, iCount, XUI_CODE_FOLD_PREPROCESSOR);
	XUI_TEST_CHECK(iIndex >= 0 && arrRanges[iIndex].iStartLine == 0 && arrRanges[iIndex].iEndLine == 8, "preprocessor fold");
	iIndex = find_range_with_flags(arrRanges, iCount, XUI_CODE_FOLD_HEADER);
	XUI_TEST_CHECK(iIndex >= 0, "header flag");

	iRet = xuiCodeDocumentCreate(&pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK && pDocument != NULL, "create fold document");
	iRet = xuiCodeDocumentSetText(pDocument, sCode);
	XUI_TEST_CHECK(iRet == XUI_OK, "set fold document");
	iRet = xuiCodeDocumentInsert(pDocument, 4, " ");
	XUI_TEST_CHECK(iRet == XUI_OK, "move fold document gap");
	iRet = xuiCodeDocumentDelete(pDocument, 4, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore fold document text");
	iRet = xuiCodeFoldCBuildDocumentRanges(pDocument, arrRanges, 32, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount >= 4, "document fold across gap");

	iIndex = find_range_with_flags(arrRanges, iCount, XUI_CODE_FOLD_HEADER);
	XUI_TEST_CHECK(iIndex >= 0, "document fold header flag");
	arrRanges[iIndex].iFlags |= XUI_CODE_FOLD_COLLAPSED;
	iRet = xuiCodeFoldBuildVisibleLines(9, arrRanges, iCount, arrVisible, 16, &iVisible);
	XUI_TEST_CHECK(iRet == XUI_OK, "visible lines");
	XUI_TEST_CHECK(iVisible < 9, "collapsed hides lines");
	XUI_TEST_CHECK(arrVisible[0] == 0, "header remains visible");

cleanup:
	xuiCodeDocumentDestroy(pDocument);
	return iFailed ? 1 : (printf("xui_code_fold_test passed\n"), 0);
}
