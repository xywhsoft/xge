#include "xui.h"

#include <stdio.h>
#include <string.h>

static int g_iFoldFailAllocation = -1;

static void* xuiTestFoldMalloc(size_t iSize)
{
	if ( g_iFoldFailAllocation >= 0 && g_iFoldFailAllocation-- == 0 ) return NULL;
	return xrtMalloc(iSize);
}

#define xrtMalloc xuiTestFoldMalloc
#include "../../src/xui/code/xui_code_fold.c"
#undef xrtMalloc

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
	xui_code_fold_range_t arrHidden[3];
	char sDeep[XUI_CODE_FOLD_STACK_MAX + 2];
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

	iRet = xuiCodeFoldCBuildRanges("#iffy\n#endifx\n", -1,
		arrRanges, 32, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 0,
		"preprocessor directive requires token boundary");
	iRet = xuiCodeFoldCBuildRanges("/* note\n */\n#if A\nx\n#endif\n", -1,
		arrRanges, 32, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "preprocessor after multiline comment");
	iIndex = find_range_with_flags(arrRanges, iCount, XUI_CODE_FOLD_PREPROCESSOR);
	XUI_TEST_CHECK(iIndex >= 0 && arrRanges[iIndex].iStartLine == 2 &&
		arrRanges[iIndex].iEndLine == 4,
		"multiline comment preserves following line start");
	iRet = xuiCodeFoldCBuildRanges("\"continued\\\ntext\";\n{\n}\n", -1,
		arrRanges, 32, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrRanges[0].iStartLine == 2 && arrRanges[0].iEndLine == 3,
		"escaped string newline preserves fold lines");
	iRet = xuiCodeFoldCBuildRanges("\"unterminated\n{\n}\n", -1,
		arrRanges, 32, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 1 &&
		arrRanges[0].iStartLine == 1 && arrRanges[0].iEndLine == 2,
		"invalid string recovers at newline");

	memset(sDeep, '{', XUI_CODE_FOLD_STACK_MAX + 1);
	sDeep[XUI_CODE_FOLD_STACK_MAX + 1] = '\0';
	iCount = 99;
	iRet = xuiCodeFoldCBuildRanges(sDeep, -1, arrRanges, 32, &iCount);
	XUI_TEST_CHECK(iRet == XUI_ERROR_LIMIT_EXCEEDED && iCount == 0,
		"deep nesting rejected without partial ranges");
	XUI_TEST_CHECK(xuiCodeFoldCBuildRanges("{}", -1, NULL, 1, &iCount) ==
		XUI_ERROR_INVALID_ARGUMENT, "missing range buffer rejected");
	XUI_TEST_CHECK(xuiCodeFoldCBuildRanges("{}", -1, NULL, -1, &iCount) ==
		XUI_ERROR_INVALID_ARGUMENT, "negative range capacity rejected");

	memset(arrHidden, 0, sizeof(arrHidden));
	arrHidden[0].iStartLine = 5;
	arrHidden[0].iEndLine = 8;
	arrHidden[0].iFlags = XUI_CODE_FOLD_COLLAPSED;
	arrHidden[1].iStartLine = 1;
	arrHidden[1].iEndLine = 3;
	arrHidden[1].iFlags = XUI_CODE_FOLD_COLLAPSED;
	arrHidden[2].iStartLine = 2;
	arrHidden[2].iEndLine = 6;
	arrHidden[2].iFlags = XUI_CODE_FOLD_COLLAPSED;
	iRet = xuiCodeFoldBuildVisibleLines(10, arrHidden, 3, NULL, 0, &iVisible);
	XUI_TEST_CHECK(iRet == XUI_OK && iVisible == 3,
		"visible line count probe merges unsorted folds");
	iRet = xuiCodeFoldBuildVisibleLines(10, arrHidden, 3, arrVisible, 2, &iVisible);
	XUI_TEST_CHECK(iRet == XUI_OK && iVisible == 3 &&
		arrVisible[0] == 0 && arrVisible[1] == 1,
		"visible line truncated output reports required count");
	g_iFoldFailAllocation = 0;
	iRet = xuiCodeFoldBuildVisibleLines(10, arrHidden, 3, arrVisible, 16, &iVisible);
	g_iFoldFailAllocation = -1;
	XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY && iVisible == 0,
		"visible line allocation failure");
	arrHidden[0].iStartLine = -1;
	iRet = xuiCodeFoldBuildVisibleLines(10, arrHidden, 1, arrVisible, 16, &iVisible);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT,
		"invalid fold geometry rejected");
	XUI_TEST_CHECK(xuiCodeFoldBuildVisibleLines(10, NULL, 1,
		arrVisible, 16, &iVisible) == XUI_ERROR_INVALID_ARGUMENT,
		"missing fold ranges rejected");
	XUI_TEST_CHECK(xuiCodeFoldBuildVisibleLines(10, arrHidden, 0,
		NULL, 1, &iVisible) == XUI_ERROR_INVALID_ARGUMENT,
		"missing visible buffer rejected");

cleanup:
	g_iFoldFailAllocation = -1;
	xuiCodeDocumentDestroy(pDocument);
	return iFailed ? 1 : (printf("xui_code_fold_test passed\n"), 0);
}
