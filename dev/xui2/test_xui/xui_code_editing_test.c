#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_editing_test failed: %s\n", (msg)); \
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
	iRet = xuiCodeSelectionCreate(&pSelection);
	XUI_TEST_CHECK(iRet == XUI_OK, "selection create");

	iRet = xuiCodeDocumentSetText(pDocument, "abc\ndef\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "set text");
	iRet = xuiCodeSelectionGotoOffset(pSelection, pDocument, 1, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "goto insert offset");
	iRet = xuiCodeEditingInsertText(pDocument, pSelection, "XX", 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "aXXbc\ndef\n") == 0, "insert text");
	iRet = xuiCodeSelectionGetState(pSelection, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iCaretOffset == 3 && tState.iAnchorOffset == 3, "insert caret");
	iRet = xuiCodeSelectionSetRange(pSelection, pDocument, 1, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "select replace range");
	iRet = xuiCodeEditingInsertText(pDocument, pSelection, "Q", 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "aQ\ndef\n") == 0, "replace selection");
	iRet = xuiCodeEditingInsertText(pDocument, pSelection, "!", 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED && strcmp(xuiCodeDocumentGetText(pDocument), "aQ\ndef\n") == 0, "readonly insert rejected");

	iRet = xuiCodeSelectionGotoOffset(pSelection, pDocument, 2, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "goto delete back");
	iRet = xuiCodeEditingDeleteBackward(pDocument, pSelection, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "a\ndef\n") == 0, "delete backward");
	iRet = xuiCodeEditingDeleteForward(pDocument, pSelection, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "adef\n") == 0, "delete forward");
	iRet = xuiCodeSelectionSetRange(pSelection, pDocument, 1, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "select delete range");
	iRet = xuiCodeEditingDeleteForward(pDocument, pSelection, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "af\n") == 0, "delete selection");
	iRet = xuiCodeDocumentSetText(pDocument, "alpha beta gamma");
	XUI_TEST_CHECK(iRet == XUI_OK, "reset word delete text");
	iRet = xuiCodeSelectionGotoOffset(pSelection, pDocument, 10, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "goto word delete back");
	iRet = xuiCodeEditingDeleteWordBackward(pDocument, pSelection, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "alpha  gamma") == 0, "delete word backward");
	iRet = xuiCodeSelectionGotoOffset(pSelection, pDocument, 7, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "goto word delete forward");
	iRet = xuiCodeEditingDeleteWordForward(pDocument, pSelection, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "alpha  ") == 0, "delete word forward");

	iRet = xuiCodeDocumentSetText(pDocument, "one\ntwo\nthree\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "reset indent text");
	iRet = xuiCodeSelectionSetRange(pSelection, pDocument, 0, 8);
	XUI_TEST_CHECK(iRet == XUI_OK, "select two lines");
	iRet = xuiCodeEditingIndentSelection(pDocument, pSelection, "\t", 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "\tone\n\ttwo\nthree\n") == 0, "indent selection");
	iRet = xuiCodeSelectionGetRange(pSelection, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == 1 && iEnd == 10, "indent selection adjusted");
	iRet = xuiCodeEditingOutdentSelection(pDocument, pSelection, 4, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "one\ntwo\nthree\n") == 0, "outdent selection");

	iRet = xuiCodeDocumentSetText(pDocument, "  one\n  two\nthree\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "reset comment text");
	iRet = xuiCodeSelectionSetRange(pSelection, pDocument, 0, 12);
	XUI_TEST_CHECK(iRet == XUI_OK, "select comment lines");
	iRet = xuiCodeEditingToggleLineComment(pDocument, pSelection, "//", 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "  //one\n  //two\nthree\n") == 0, "comment selection");
	iRet = xuiCodeEditingToggleLineComment(pDocument, pSelection, "//", 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "  one\n  two\nthree\n") == 0, "uncomment selection");
	iRet = xuiCodeEditingToggleLineComment(pDocument, pSelection, "", 0);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "empty comment invalid");

cleanup:
	xuiCodeSelectionDestroy(pSelection);
	xuiCodeDocumentDestroy(pDocument);
	if ( iFailed ) return 1;
	printf("xui_code_editing_test passed\n");
	return 0;
}
