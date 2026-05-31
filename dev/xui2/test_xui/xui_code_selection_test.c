#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_selection_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

int main(void)
{
	xui_code_document pDocument;
	xui_code_selection_model pSelection;
	xui_code_selection_t tState;
	int iStart;
	int iEnd;
	int iFailed;
	int iRet;

	pDocument = NULL;
	pSelection = NULL;
	iFailed = 0;

	iRet = xuiCodeDocumentCreate(&pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK, "document create");
	iRet = xuiCodeDocumentSetText(pDocument, "alpha beta\nxy\nlast line\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "document text");
	iRet = xuiCodeSelectionCreate(&pSelection);
	XUI_TEST_CHECK(iRet == XUI_OK && pSelection != NULL, "selection create");

	iRet = xuiCodeSelectionSetRange(pSelection, pDocument, 8, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "set reversed range");
	iRet = xuiCodeSelectionGetRange(pSelection, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == 2 && iEnd == 8, "normalized range");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK && (tState.iFlags & XUI_CODE_SELECTION_REVERSED) != 0, "reversed flag");
	XUI_TEST_CHECK(xuiCodeSelectionHasSelection(pSelection) == 1, "has selection");
	XUI_TEST_CHECK(xuiCodeSelectionGetCount(pSelection) == 1, "selection count");
	iRet = xuiCodeSelectionAdd(pSelection, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi selection add");
	XUI_TEST_CHECK(xuiCodeSelectionGetCount(pSelection) == 2, "multi selection count");
	memset(&tState, 0, sizeof(tState));
	iRet = xuiCodeSelectionGetAt(pSelection, 1, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iAnchorOffset == 8 && tState.iCaretOffset == 2, "multi selection get at");
	XUI_TEST_CHECK((tState.iFlags & XUI_CODE_SELECTION_INACTIVE) != 0, "multi selection inactive");
	iRet = xuiCodeSelectionGetAt(pSelection, 2, &tState);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "multi selection invalid index");

	iRet = xuiCodeSelectionGotoOffset(pSelection, pDocument, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "goto offset");
	XUI_TEST_CHECK(xuiCodeSelectionGetCount(pSelection) == 1, "goto clears extra selections");
	iRet = xuiCodeSelectionMove(pSelection, pDocument, XUI_CODE_COMMAND_MOVE_RIGHT);
	XUI_TEST_CHECK(iRet == XUI_OK, "move right");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(tState.iCaretOffset == 1 && tState.iAnchorOffset == 1, "right caret");
	iRet = xuiCodeSelectionMove(pSelection, pDocument, XUI_CODE_COMMAND_SELECT_RIGHT);
	XUI_TEST_CHECK(iRet == XUI_OK, "select right");
	iRet = xuiCodeSelectionGetRange(pSelection, &iStart, &iEnd);
	XUI_TEST_CHECK(iStart == 1 && iEnd == 2, "select right range");
	iRet = xuiCodeSelectionMove(pSelection, pDocument, XUI_CODE_COMMAND_MOVE_WORD_RIGHT);
	XUI_TEST_CHECK(iRet == XUI_OK, "word right");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(tState.iCaretOffset == 6 && tState.iAnchorOffset == 6, "word right caret");
	iRet = xuiCodeSelectionMove(pSelection, pDocument, XUI_CODE_COMMAND_MOVE_WORD_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "word left");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(tState.iCaretOffset == 0, "word left caret");

	iRet = xuiCodeSelectionGotoLineColumn(pSelection, pDocument, 0, 8, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "goto line column");
	iRet = xuiCodeSelectionMove(pSelection, pDocument, XUI_CODE_COMMAND_MOVE_DOWN);
	XUI_TEST_CHECK(iRet == XUI_OK, "move down");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(tState.iCaretOffset == 13, "down clamps short line");
	iRet = xuiCodeSelectionMove(pSelection, pDocument, XUI_CODE_COMMAND_MOVE_DOWN);
	XUI_TEST_CHECK(iRet == XUI_OK, "move down again");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(tState.iCaretOffset == 22, "preferred column preserved");
	iRet = xuiCodeSelectionMove(pSelection, pDocument, XUI_CODE_COMMAND_MOVE_LINE_START);
	XUI_TEST_CHECK(iRet == XUI_OK, "line start");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(tState.iCaretOffset == 14, "line start caret");
	iRet = xuiCodeSelectionMove(pSelection, pDocument, XUI_CODE_COMMAND_MOVE_LINE_END);
	XUI_TEST_CHECK(iRet == XUI_OK, "line end");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(tState.iCaretOffset == 23, "line end caret");

	iRet = xuiCodeSelectionMove(pSelection, pDocument, XUI_CODE_COMMAND_SELECT_UP);
	XUI_TEST_CHECK(iRet == XUI_OK, "select up");
	iRet = xuiCodeSelectionGetRange(pSelection, &iStart, &iEnd);
	XUI_TEST_CHECK(iStart < iEnd && xuiCodeSelectionHasSelection(pSelection), "select up range");
	iRet = xuiCodeSelectionMove(pSelection, pDocument, XUI_CODE_COMMAND_MOVE_DOCUMENT_START);
	XUI_TEST_CHECK(iRet == XUI_OK, "document start");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(tState.iCaretOffset == 0 && !xuiCodeSelectionHasSelection(pSelection), "document start clears selection");
	iRet = xuiCodeSelectionMove(pSelection, pDocument, XUI_CODE_COMMAND_MOVE_DOCUMENT_END);
	XUI_TEST_CHECK(iRet == XUI_OK, "document end");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(tState.iCaretOffset == xuiCodeDocumentGetLength(pDocument), "document end caret");

	iRet = xuiCodeSelectionSelectAll(pSelection, pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK, "select all");
	iRet = xuiCodeSelectionGetRange(pSelection, &iStart, &iEnd);
	XUI_TEST_CHECK(iStart == 0 && iEnd == xuiCodeDocumentGetLength(pDocument), "select all range");
	iRet = xuiCodeSelectionSelectWord(pSelection, pDocument, 7);
	XUI_TEST_CHECK(iRet == XUI_OK, "select word");
	iRet = xuiCodeSelectionGetRange(pSelection, &iStart, &iEnd);
	XUI_TEST_CHECK(iStart == 6 && iEnd == 10, "select word range");
	iRet = xuiCodeSelectionSelectWord(pSelection, pDocument, 9);
	XUI_TEST_CHECK(iRet == XUI_OK, "select word at last char");
	iRet = xuiCodeSelectionGetRange(pSelection, &iStart, &iEnd);
	XUI_TEST_CHECK(iStart == 6 && iEnd == 10, "select word at last char range");
	iRet = xuiCodeSelectionSelectWord(pSelection, pDocument, 5);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiCodeSelectionHasSelection(pSelection), "select separator word empty");
	iRet = xuiCodeSelectionSelectLine(pSelection, pDocument, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "select line text");
	iRet = xuiCodeSelectionGetRange(pSelection, &iStart, &iEnd);
	XUI_TEST_CHECK(iStart == 0 && iEnd == 10, "select line text range");
	iRet = xuiCodeSelectionSelectLine(pSelection, pDocument, 0, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "select line break");
	iRet = xuiCodeSelectionGetRange(pSelection, &iStart, &iEnd);
	XUI_TEST_CHECK(iStart == 0 && iEnd == 11, "select line break range");
	iRet = xuiCodeSelectionSelectLine(pSelection, pDocument, 2, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "select last content line");
	iRet = xuiCodeSelectionGetRange(pSelection, &iStart, &iEnd);
	XUI_TEST_CHECK(iStart == 14 && iEnd == 24, "select last content line range");
	xuiCodeSelectionClear(pSelection);
	XUI_TEST_CHECK(!xuiCodeSelectionHasSelection(pSelection), "clear selection");
	XUI_TEST_CHECK(xuiCodeSelectionGetCount(pSelection) == 1, "clear keeps primary selection slot");

cleanup:
	xuiCodeSelectionDestroy(pSelection);
	xuiCodeDocumentDestroy(pDocument);
	if ( iFailed ) return 1;
	printf("xui_code_selection_test passed\n");
	return 0;
}
