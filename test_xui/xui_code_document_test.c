#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_code_document_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_code_document_change_test_t {
	int iCount;
	xui_code_range_t tRange;
	uint32_t iVersion;
} xui_code_document_change_test_t;

static void __xuiCodeDocumentChange(xui_code_document pDocument, xui_code_range_t tRange, uint32_t iVersion, void* pUser)
{
	xui_code_document_change_test_t* pState;

	(void)pDocument;
	pState = (xui_code_document_change_test_t*)pUser;
	if ( pState == NULL ) return;
	pState->iCount++;
	pState->tRange = tRange;
	pState->iVersion = iVersion;
}

static uint32_t __xuiCodeDocumentRandom(uint32_t* pState)
{
	*pState = *pState * 1664525u + 1013904223u;
	return *pState;
}

static int __xuiCodeDocumentValidateLines(xui_code_document pDocument, const char* sText)
{
	int iExpectedStart;
	int iExpectedLine;
	int iLength;
	int iStart;
	int iEnd;
	int i;

	iLength = (int)strlen(sText);
	iExpectedStart = 0;
	iExpectedLine = 0;
	for ( i = 0; i <= iLength; i++ ) {
		if ( i < iLength && sText[i] != '\n' ) continue;
		if ( xuiCodeDocumentGetLineRange(pDocument, iExpectedLine, &iStart, &iEnd) != XUI_OK ) return 0;
		if ( iStart != iExpectedStart || iEnd != i ) {
			printf("line mismatch line=%d actual=%d..%d expected=%d..%d text=[%s]\n",
				iExpectedLine, iStart, iEnd, iExpectedStart, i, sText);
			return 0;
		}
		iExpectedLine++;
		iExpectedStart = i + 1;
	}
	if ( xuiCodeDocumentGetLineCount(pDocument) != iExpectedLine ) {
		printf("line count mismatch actual=%d expected=%d text=[%s]\n",
			xuiCodeDocumentGetLineCount(pDocument), iExpectedLine, sText);
		return 0;
	}
	return 1;
}

int main(void)
{
	xui_code_document pDocument;
	xui_code_document_change_test_t tChange;
	xui_code_range_t tEditRange;
	const char* sFile;
	const char* sBadUtf8;
	uint32_t iVersion;
	int iChangeCount;
	int iStart;
	int iEnd;
	int iLine;
	int iColumn;
	int iOffset;
	char cByte;
	char sRange[32];
	char sReference[8192];
	const char* arrInsert[] = {"x", "\n", "ab", "", "q\nr"};
	uint32_t iRandomState;
	int iReferenceLength;
	int iInsertLength;
	int iIteration;
	int iPattern;
	int iFailed;
	int iRet;

	pDocument = NULL;
	iFailed = 0;
	sFile = "build\\xui_code_document_tmp.txt";
	sBadUtf8 = "\xC3\x28";
	memset(&tChange, 0, sizeof(tChange));

	iRet = xuiCodeDocumentCreate(&pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK && pDocument != NULL, "create document");
	iRet = xuiCodeDocumentSetChangeCallback(pDocument, __xuiCodeDocumentChange, &tChange);
	XUI_TEST_CHECK(iRet == XUI_OK, "set change callback");
	XUI_TEST_CHECK(strcmp(xuiCodeDocumentGetText(pDocument), "") == 0, "initial text");
	XUI_TEST_CHECK(xuiCodeDocumentGetLength(pDocument) == 0, "initial length");
	XUI_TEST_CHECK(xuiCodeDocumentGetLineCount(pDocument) == 1, "initial line count");

	iRet = xuiCodeDocumentSetText(pDocument, "one\r\ntwo\rthree\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "set text");
	XUI_TEST_CHECK(strcmp(xuiCodeDocumentGetText(pDocument), "one\ntwo\nthree\n") == 0, "newline normalization");
	XUI_TEST_CHECK(xuiCodeDocumentGetLineCount(pDocument) == 4, "line count after set");
	iRet = xuiCodeDocumentGetLineRange(pDocument, 1, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == 4 && iEnd == 7, "line range");
	iRet = xuiCodeDocumentOffsetToLineColumn(pDocument, 5, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && iLine == 1 && iColumn == 1, "offset to line column");
	iRet = xuiCodeDocumentOffsetToLineColumn(pDocument, 4, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && iLine == 1 && iColumn == 0, "line start offset mapping");
	iRet = xuiCodeDocumentLineColumnToOffset(pDocument, 2, 2, &iOffset);
	XUI_TEST_CHECK(iRet == XUI_OK && iOffset == 10, "line column to offset");
	XUI_TEST_CHECK(!xuiCodeDocumentGetDirty(pDocument), "set text clean");

	iRet = xuiCodeDocumentSetTextLength(pDocument, "alpha\r\nbeta\nignored", 12);
	XUI_TEST_CHECK(iRet == XUI_OK, "set text length");
	XUI_TEST_CHECK(strcmp(xuiCodeDocumentGetText(pDocument), "alpha\nbeta\n") == 0, "set text length ignores trailing bytes");
	XUI_TEST_CHECK(xuiCodeDocumentGetLineCount(pDocument) == 3, "line count after set length");

	iRet = xuiCodeDocumentSetText(pDocument, "a\xE4\xBD\xA0" "b\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "set unicode text");
	iRet = xuiCodeDocumentOffsetToLineColumn(pDocument, 4, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && iLine == 0 && iColumn == 2, "unicode offset to character column");
	iRet = xuiCodeDocumentOffsetToLineColumn(pDocument, 2, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && iLine == 0 && iColumn == 1, "unicode middle byte clamps to character");
	iRet = xuiCodeDocumentLineColumnToOffset(pDocument, 0, 2, &iOffset);
	XUI_TEST_CHECK(iRet == XUI_OK && iOffset == 4, "unicode character column to offset");

	iVersion = xuiCodeDocumentGetVersion(pDocument);
	iRet = xuiCodeDocumentSetText(pDocument, "one\ntwo\nthree\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "reset after unicode mapping");
	iVersion = xuiCodeDocumentGetVersion(pDocument);
	iChangeCount = tChange.iCount;
	iRet = xuiCodeDocumentInsert(pDocument, 0, "zero\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "insert");
	XUI_TEST_CHECK(tChange.iCount == iChangeCount + 1 && tChange.tRange.iStart == 0 && tChange.tRange.iEnd == 5 && tChange.iVersion == xuiCodeDocumentGetVersion(pDocument), "insert change callback");
	XUI_TEST_CHECK(strcmp(xuiCodeDocumentGetText(pDocument), "zero\none\ntwo\nthree\n") == 0, "insert text");
	iRet = xuiCodeDocumentGetLastEditRange(pDocument, &tEditRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tEditRange.iStart == 0 && tEditRange.iEnd == 5, "insert edit range");
	XUI_TEST_CHECK(xuiCodeDocumentGetDirty(pDocument), "insert dirty");
	XUI_TEST_CHECK(xuiCodeDocumentGetVersion(pDocument) == iVersion + 1u, "version after insert");
	XUI_TEST_CHECK(xuiCodeDocumentCanUndo(pDocument), "can undo insert");

	iRet = xuiCodeDocumentDelete(pDocument, 5, 9);
	XUI_TEST_CHECK(iRet == XUI_OK, "delete");
	iRet = xuiCodeDocumentGetLastEditRange(pDocument, &tEditRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tEditRange.iStart == 5 && tEditRange.iEnd == 5, "delete edit range");
	XUI_TEST_CHECK(strcmp(xuiCodeDocumentGetText(pDocument), "zero\ntwo\nthree\n") == 0, "delete text");
	iRet = xuiCodeDocumentReplace(pDocument, 5, 8, "TWO");
	XUI_TEST_CHECK(iRet == XUI_OK, "replace");
	iRet = xuiCodeDocumentGetLastEditRange(pDocument, &tEditRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tEditRange.iStart == 5 && tEditRange.iEnd == 8, "replace edit range");
	XUI_TEST_CHECK(strcmp(xuiCodeDocumentGetText(pDocument), "zero\nTWO\nthree\n") == 0, "replace text");

	iRet = xuiCodeDocumentUndo(pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "zero\ntwo\nthree\n") == 0, "undo replace");
	iRet = xuiCodeDocumentUndo(pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "zero\none\ntwo\nthree\n") == 0, "undo delete");
	iRet = xuiCodeDocumentRedo(pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "zero\ntwo\nthree\n") == 0, "redo delete");

	iRet = xuiCodeDocumentSetDirty(pDocument, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiCodeDocumentGetDirty(pDocument), "set clean");
	iRet = xuiCodeDocumentInsert(pDocument, xuiCodeDocumentGetLength(pDocument), "tail");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCodeDocumentGetDirty(pDocument), "dirty after clean edit");

	iRet = xuiCodeDocumentSetText(pDocument, "a\nb\nc");
	XUI_TEST_CHECK(iRet == XUI_OK, "reset text");
	iRet = xuiCodeDocumentBeginEdit(pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK, "begin edit");
	iRet = xuiCodeDocumentInsert(pDocument, 1, "1");
	XUI_TEST_CHECK(iRet == XUI_OK, "transaction insert 1");
	iRet = xuiCodeDocumentInsert(pDocument, 3, "2");
	XUI_TEST_CHECK(iRet == XUI_OK, "transaction insert 2");
	iRet = xuiCodeDocumentEndEdit(pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "a1\n2b\nc") == 0, "transaction text");
	XUI_TEST_CHECK(tChange.tRange.iStart == 1 && tChange.tRange.iEnd == 4, "transaction change range");
	iRet = xuiCodeDocumentGetLastEditRange(pDocument, &tEditRange);
	XUI_TEST_CHECK(iRet == XUI_OK && tEditRange.iStart == 1 && tEditRange.iEnd == 4, "transaction edit range");
	iRet = xuiCodeDocumentUndo(pDocument);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "a\nb\nc") == 0, "transaction undo");
	XUI_TEST_CHECK(tChange.tRange.iStart == 0 && tChange.tRange.iEnd == xuiCodeDocumentGetLength(pDocument), "undo change callback");

	iRet = xuiCodeDocumentSetText(pDocument, sBadUtf8);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && strstr(xuiCodeDocumentGetLastError(pDocument), "UTF-8") != NULL, "reject invalid utf8");
	XUI_TEST_CHECK(strcmp(xuiCodeDocumentGetText(pDocument), "a\nb\nc") == 0, "invalid utf8 does not mutate");

	iRet = xuiCodeDocumentSetText(pDocument, "");
	XUI_TEST_CHECK(iRet == XUI_OK, "reset for typing coalescing");
	XUI_TEST_CHECK(xuiCodeDocumentInsert(pDocument, 0, "a") == XUI_OK, "typing insert a");
	XUI_TEST_CHECK(xuiCodeDocumentInsert(pDocument, 1, "b") == XUI_OK, "typing insert b");
	XUI_TEST_CHECK(xuiCodeDocumentInsert(pDocument, 2, "c") == XUI_OK, "typing insert c");
	XUI_TEST_CHECK(strcmp(xuiCodeDocumentGetText(pDocument), "abc") == 0, "typing text");
	XUI_TEST_CHECK(xuiCodeDocumentUndo(pDocument) == XUI_OK, "typing undo");
	XUI_TEST_CHECK(strcmp(xuiCodeDocumentGetText(pDocument), "") == 0, "typing coalesced undo");
	XUI_TEST_CHECK(xuiCodeDocumentRedo(pDocument) == XUI_OK, "typing redo");
	XUI_TEST_CHECK(strcmp(xuiCodeDocumentGetText(pDocument), "abc") == 0, "typing coalesced redo");

	XUI_TEST_CHECK(xuiCodeDocumentDelete(pDocument, 2, 3) == XUI_OK, "backspace c");
	XUI_TEST_CHECK(xuiCodeDocumentDelete(pDocument, 1, 2) == XUI_OK, "backspace b");
	XUI_TEST_CHECK(xuiCodeDocumentDelete(pDocument, 0, 1) == XUI_OK, "backspace a");
	XUI_TEST_CHECK(strcmp(xuiCodeDocumentGetText(pDocument), "") == 0, "backspace text");
	XUI_TEST_CHECK(xuiCodeDocumentUndo(pDocument) == XUI_OK, "backspace undo");
	XUI_TEST_CHECK(strcmp(xuiCodeDocumentGetText(pDocument), "abc") == 0, "backspace coalesced undo");

	iRet = xuiCodeDocumentSetText(pDocument, "abcdef");
	XUI_TEST_CHECK(iRet == XUI_OK, "range read reset");
	XUI_TEST_CHECK(xuiCodeDocumentDelete(pDocument, 2, 4) == XUI_OK, "range read creates middle gap");
	XUI_TEST_CHECK(xuiCodeDocumentInsert(pDocument, 2, "CDE") == XUI_OK, "range read inserts before gap");
	iRet = xuiCodeDocumentCopyRange(pDocument, 1, 6, sRange, (int)sizeof(sRange), &iOffset);
	XUI_TEST_CHECK(iRet == XUI_OK && iOffset == 5 && strcmp(sRange, "bCDEe") == 0,
		"copy range spans gap");
	iRet = xuiCodeDocumentGetByte(pDocument, 5, &cByte);
	XUI_TEST_CHECK(iRet == XUI_OK && cByte == 'e', "get byte after gap");

	iRet = xuiCodeDocumentSetText(pDocument, "alpha\nbeta\ngamma\n");
	XUI_TEST_CHECK(iRet == XUI_OK, "random edit reset");
	strcpy(sReference, "alpha\nbeta\ngamma\n");
	iReferenceLength = (int)strlen(sReference);
	iRandomState = 0xC0DE1234u;
	for ( iIteration = 0; iIteration < 1000; iIteration++ ) {
		iStart = (int)(__xuiCodeDocumentRandom(&iRandomState) % (uint32_t)(iReferenceLength + 1));
		iEnd = iStart + (int)(__xuiCodeDocumentRandom(&iRandomState) %
			(uint32_t)(iReferenceLength - iStart + 1));
		iPattern = (int)(__xuiCodeDocumentRandom(&iRandomState) %
			(uint32_t)(sizeof(arrInsert) / sizeof(arrInsert[0])));
		iInsertLength = (int)strlen(arrInsert[iPattern]);
		if ( iReferenceLength - (iEnd - iStart) + iInsertLength >= (int)sizeof(sReference) ) {
			iPattern = 3;
			iInsertLength = 0;
		}
		memmove(sReference + iStart + iInsertLength, sReference + iEnd,
			(size_t)(iReferenceLength - iEnd) + 1u);
		memcpy(sReference + iStart, arrInsert[iPattern], (size_t)iInsertLength);
		iReferenceLength += iInsertLength - (iEnd - iStart);
		iRet = xuiCodeDocumentReplace(pDocument, iStart, iEnd, arrInsert[iPattern]);
		XUI_TEST_CHECK(iRet == XUI_OK, "random edit replace");
		XUI_TEST_CHECK(xuiCodeDocumentGetLength(pDocument) == iReferenceLength &&
			strcmp(xuiCodeDocumentGetText(pDocument), sReference) == 0,
			"random edit text matches reference");
		if ( !__xuiCodeDocumentValidateLines(pDocument, sReference) ) {
			printf("random edit iteration=%d replace=%d..%d insert=[%s]\n",
				iIteration, iStart, iEnd, arrInsert[iPattern]);
			XUI_TEST_CHECK(0, "random edit line index matches reference");
		}
	}

	iRet = xuiCodeDocumentSetText(pDocument, "file\r\ntext");
	XUI_TEST_CHECK(iRet == XUI_OK, "set file text");
	iRet = xuiCodeDocumentSaveTextFile(pDocument, sFile, XRT_CP_UTF8);
	XUI_TEST_CHECK(iRet == XUI_OK, "save text file");
	iRet = xuiCodeDocumentSetText(pDocument, "");
	XUI_TEST_CHECK(iRet == XUI_OK, "clear before load");
	iRet = xuiCodeDocumentLoadTextFile(pDocument, sFile, XRT_CP_UTF8);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCodeDocumentGetText(pDocument), "file\ntext") == 0, "load text file");

cleanup:
	xrtFileDelete((str)sFile);
	xuiCodeDocumentDestroy(pDocument);
	if ( iFailed ) return 1;
	printf("xui_code_document_test passed\n");
	return 0;
}
