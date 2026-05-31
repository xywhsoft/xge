#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_layout_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

int main(void)
{
	xui_code_document pDocument;
	xui_code_layout_desc_t tDesc;
	xui_code_layout_line_t arrLines[8];
	xui_code_fold_range_t tFold;
	xui_code_hit_t tHit;
	xui_vec2_t tContent;
	xui_rect_t tTextRect;
	xui_rect_t tCaret;
	int iCount;
	int iFailed;
	int iRet;

	pDocument = NULL;
	iFailed = 0;

	iRet = xuiCodeDocumentCreate(&pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK, "document create");
	iRet = xuiCodeDocumentSetText(pDocument, "alpha\nbeta\ngamma\ndelta\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "document text");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pDocument = pDocument;
	tDesc.tViewportRect = (xui_rect_t){10.0f, 20.0f, 200.0f, 48.0f};
	tDesc.fLineHeight = 16.0f;
	tDesc.fCharWidth = 8.0f;
	tDesc.fMarginWidth = 40.0f;

	iRet = xuiCodeLayoutBuildVisibleLines(&tDesc, arrLines, 8, &iCount, &tContent, &tTextRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout visible lines");
	XUI_TEST_CHECK(iCount == 4, "visible clipped line count");
	XUI_TEST_CHECK(arrLines[0].iLine == 0 && arrLines[0].tRect.fX == 50.0f && arrLines[0].tRect.fY == 20.0f, "first line rect");
	XUI_TEST_CHECK((int)tContent.fY == 80, "content height");
	XUI_TEST_CHECK(tTextRect.fX == 50.0f && tTextRect.fW == 160.0f, "text rect");

	iRet = xuiCodeLayoutHitTest(&tDesc, 66.0f, 37.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iLine == 1 && tHit.iOffset == 8, "hit test text");
	iRet = xuiCodeLayoutGetCaretRect(&tDesc, 2, 2, &tCaret);
	XUI_TEST_CHECK(iRet == XUI_OK && tCaret.fX == 66.0f && tCaret.fY == 52.0f, "caret rect");

	memset(&tFold, 0, sizeof(tFold));
	tFold.iSize = sizeof(tFold);
	tFold.iStartLine = 0;
	tFold.iEndLine = 2;
	tFold.iFlags = XUI_CODE_FOLD_COLLAPSED | XUI_CODE_FOLD_HEADER;
	tDesc.pFoldRanges = &tFold;
	tDesc.iFoldRangeCount = 1;
	iRet = xuiCodeLayoutBuildVisibleLines(&tDesc, arrLines, 8, &iCount, &tContent, &tTextRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "folded layout");
	XUI_TEST_CHECK(iCount == 3 && arrLines[0].iLine == 0 && arrLines[1].iLine == 3, "fold hides inner lines");
	iRet = xuiCodeLayoutGetCaretRect(&tDesc, 1, 0, &tCaret);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "hidden caret rejected");

	tDesc.fScrollY = 16.0f;
	iRet = xuiCodeLayoutHitTest(&tDesc, 50.0f, 20.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iLine == 3, "scroll hit test");

	tDesc.pFoldRanges = NULL;
	tDesc.iFoldRangeCount = 0;
	tDesc.fScrollY = 0.0f;
	tDesc.iTabColumns = 4;
	iRet = xuiCodeDocumentSetText(pDocument, "a\tb\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "tab document text");
	iRet = xuiCodeLayoutBuildVisibleLines(&tDesc, arrLines, 8, &iCount, &tContent, &tTextRect);
	XUI_TEST_CHECK(iRet == XUI_OK && (int)tContent.fX == 40, "tab content width");
	iRet = xuiCodeLayoutGetCaretRect(&tDesc, 0, 2, &tCaret);
	XUI_TEST_CHECK(iRet == XUI_OK && tCaret.fX == 82.0f, "tab caret rect");
	iRet = xuiCodeLayoutHitTest(&tDesc, 82.0f, 20.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iLine == 0 && tHit.iColumn == 2 && tHit.iOffset == 2, "tab hit test");

cleanup:
	xuiCodeDocumentDestroy(pDocument);
	if ( iFailed ) return 1;
	printf("xui_code_layout_test passed\n");
	return 0;
}
