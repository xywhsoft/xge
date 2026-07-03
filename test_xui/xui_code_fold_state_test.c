#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_fold_state_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int xuiTestFoldProvider(xui_code_document_t* pDocument, xui_code_fold_range_t* pRanges, int iRangeCapacity, int* pRangeCount, void* pUser)
{
	(void)pDocument;
	(void)pUser;
	if ( pRangeCount == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pRangeCount = 1;
	if ( pRanges != NULL && iRangeCapacity > 0 ) {
		memset(&pRanges[0], 0, sizeof(pRanges[0]));
		pRanges[0].iSize = sizeof(pRanges[0]);
		pRanges[0].iStartLine = 1;
		pRanges[0].iEndLine = 3;
		pRanges[0].iLevel = 0;
		pRanges[0].iFlags = XUI_CODE_FOLD_CUSTOM;
	}
	return XUI_OK;
}

int main(void)
{
	xui_code_fold_state pState;
	xui_code_document pDocument;
	xui_code_fold_range_t arrRanges[2];
	xui_code_fold_range_t tRange;
	int arrVisible[8];
	int iCount;
	int iLine;
	int iRow;
	int bVisible;
	int iFailed;
	int iRet;

	pState = NULL;
	pDocument = NULL;
	iFailed = 0;

	iRet = xuiCodeFoldStateCreate(&pState);
	XUI_TEST_CHECK(iRet == XUI_OK && pState != NULL, "state create");

	memset(arrRanges, 0, sizeof(arrRanges));
	arrRanges[0].iSize = sizeof(arrRanges[0]);
	arrRanges[0].iStartLine = 0;
	arrRanges[0].iEndLine = 2;
	arrRanges[0].iLevel = 0;
	arrRanges[1].iSize = sizeof(arrRanges[1]);
	arrRanges[1].iStartLine = 4;
	arrRanges[1].iEndLine = 6;
	arrRanges[1].iLevel = 0;
	arrRanges[1].iFlags = XUI_CODE_FOLD_REGION;
	iRet = xuiCodeFoldStateSetRanges(pState, arrRanges, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeFoldStateGetCount(pState) == 2, "set ranges");
	iRet = xuiCodeFoldStateGetRange(pState, 0, &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && (tRange.iFlags & XUI_CODE_FOLD_HEADER) != 0, "header flag applied");

	iRet = xuiCodeFoldStateToggleLine(pState, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "toggle header");
	iRet = xuiCodeFoldStateIsLineVisible(pState, 1, &bVisible);
	XUI_TEST_CHECK(iRet == XUI_OK && !bVisible, "line hidden after toggle");
	iRet = xuiCodeFoldStateIsLineVisible(pState, 0, &bVisible);
	XUI_TEST_CHECK(iRet == XUI_OK && bVisible, "header remains visible");
	iRet = xuiCodeFoldStateBuildVisibleLines(pState, 7, arrVisible, 8, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 5, "visible line count after toggle");
	XUI_TEST_CHECK(arrVisible[0] == 0 && arrVisible[1] == 3, "visible line contents");
	iRet = xuiCodeFoldStateGetVisibleLineCount(pState, 7, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 5, "visible count query after toggle");
	iRet = xuiCodeFoldStateLineToVisibleRow(pState, 4, &iRow);
	XUI_TEST_CHECK(iRet == XUI_OK && iRow == 2, "line to visible row after toggle");
	iRet = xuiCodeFoldStateVisibleRowToLine(pState, 7, 2, &iLine);
	XUI_TEST_CHECK(iRet == XUI_OK && iLine == 4, "visible row to line after toggle");

	iRet = xuiCodeFoldStateFoldAll(pState);
	XUI_TEST_CHECK(iRet == XUI_OK, "fold all");
	iRet = xuiCodeFoldStateBuildVisibleLines(pState, 7, arrVisible, 8, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 3, "visible line count after fold all");
	iRet = xuiCodeFoldStateVisibleRowToLine(pState, 7, 1, &iLine);
	XUI_TEST_CHECK(iRet == XUI_OK && iLine == 3, "visible row to line after fold all");
	iRet = xuiCodeFoldStateUnfoldAll(pState);
	XUI_TEST_CHECK(iRet == XUI_OK, "unfold all");
	iRet = xuiCodeFoldStateBuildVisibleLines(pState, 7, arrVisible, 8, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 7, "visible line count after unfold all");
	iRet = xuiCodeFoldStateToggleLine(pState, 99);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED, "toggle missing fold");

	iRet = xuiCodeDocumentCreate(&pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK, "document create");
	iRet = xuiCodeDocumentSetText(pDocument, "a\nb\nc\nd\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "document text");
	iRet = xuiCodeFoldStateBuildFromProvider(pState, pDocument, xuiTestFoldProvider, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeFoldStateGetCount(pState) == 1, "provider fold");
	iRet = xuiCodeFoldStateGetRange(pState, 0, &tRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tRange.iStartLine == 1 && tRange.iEndLine == 3 && (tRange.iFlags & XUI_CODE_FOLD_CUSTOM) != 0, "provider range");

	memset(arrRanges, 0, sizeof(arrRanges));
	arrRanges[0].iSize = sizeof(arrRanges[0]);
	arrRanges[0].iStartLine = 0;
	arrRanges[0].iEndLine = 5;
	arrRanges[0].iFlags = XUI_CODE_FOLD_COLLAPSED;
	arrRanges[1].iSize = sizeof(arrRanges[1]);
	arrRanges[1].iStartLine = 2;
	arrRanges[1].iEndLine = 8;
	arrRanges[1].iFlags = XUI_CODE_FOLD_COLLAPSED;
	iRet = xuiCodeFoldStateSetRanges(pState, arrRanges, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlap ranges set");
	iRet = xuiCodeFoldStateGetVisibleLineCount(pState, 10, &iCount);
	XUI_TEST_CHECK(iRet == XUI_OK && iCount == 2, "overlap visible count");
	iRet = xuiCodeFoldStateVisibleRowToLine(pState, 10, 1, &iLine);
	XUI_TEST_CHECK(iRet == XUI_OK && iLine == 9, "overlap visible row to line");
	iRet = xuiCodeFoldStateLineToVisibleRow(pState, 9, &iRow);
	XUI_TEST_CHECK(iRet == XUI_OK && iRow == 1, "overlap line to visible row");

cleanup:
	xuiCodeDocumentDestroy(pDocument);
	xuiCodeFoldStateDestroy(pState);
	if ( iFailed ) return 1;
	printf("xui_code_fold_state_test passed\n");
	return 0;
}
