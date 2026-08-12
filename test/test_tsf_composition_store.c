#include <stdio.h>
#include <string.h>

/* Keep this white-box test in the same translation unit as the private TSF
 * store. It validates the platform boundary without exposing test-only ABI. */
#include "../xge.c"

#define TEST_CHECK(expr, message) \
	do { \
		if ( !(expr) ) { \
			printf("test_tsf_composition_store failed: %s\n", (message)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __testNextIme(int iType, const char* sText)
{
	xge_input_event_t tEvent;

	memset(&tEvent, 0, sizeof(tEvent));
	if ( xgeInputEventGet(&tEvent) != 1 ) return 0;
	if ( tEvent.iType != iType ) return 0;
	if ( sText != NULL && strcmp(tEvent.sText, sText) != 0 ) return 0;
	return 1;
}

static int __testNextImeRange(int iType, const char* sText, int iStart, int iEnd)
{
	xge_input_event_t tEvent;

	memset(&tEvent, 0, sizeof(tEvent));
	if ( xgeInputEventGet(&tEvent) != 1 ) return 0;
	if ( tEvent.iType != iType ) return 0;
	if ( sText != NULL && strcmp(tEvent.sText, sText) != 0 ) return 0;
	return tEvent.bReplacementRange &&
		tEvent.iReplacementStart == iStart && tEvent.iReplacementEnd == iEnd;
}

int main(void)
{
	xge_desc_t tDesc;
	xge_tsf_text_store_t tStore;
	TS_TEXTCHANGE tChange;
	xge_ime_text_snapshot_t tSnapshot;
	IMECHARPOSITION tCharPosition;
	RECT tCaretRect;
	RECT tDocumentRect;
	WINBOOL bOk;
	int iFailed;

	iFailed = 0;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iRunMode = XGE_RUN_MANUAL;
	TEST_CHECK(xgeInit(&tDesc) == XGE_OK, "xgeInit");
	__xgeTsfStoreInitialize(&tStore);

	memset(&tCharPosition, 0, sizeof(tCharPosition));
	tCharPosition.dwSize = sizeof(tCharPosition);
	tCharPosition.dwCharPos = 3;
	tCaretRect.left = 120;
	tCaretRect.top = 240;
	tCaretRect.right = 122;
	tCaretRect.bottom = 258;
	tDocumentRect.left = 10;
	tDocumentRect.top = 20;
	tDocumentRect.right = 810;
	tDocumentRect.bottom = 620;
	TEST_CHECK(__xgeImeFillQueryCharPosition(&tCharPosition,
		&tCaretRect, &tDocumentRect), "fill IMR_QUERYCHARPOSITION");
	TEST_CHECK(tCharPosition.dwCharPos == 3 &&
			tCharPosition.pt.x == 120 && tCharPosition.pt.y == 240 &&
			tCharPosition.cLineHeight == 18 &&
			EqualRect(&tCharPosition.rcDocument, &tDocumentRect),
		"IMR_QUERYCHARPOSITION fields");
	tCharPosition.dwSize = sizeof(tCharPosition) - 1u;
	TEST_CHECK(!__xgeImeFillQueryCharPosition(&tCharPosition,
		&tCaretRect, &tDocumentRect), "reject short IMECHARPOSITION");

	bOk = FALSE;
	TEST_CHECK(__xgeTsfCompositionStart(&tStore.tCompositionSink, NULL, &bOk) == S_OK && bOk,
	           "composition start");
	tStore.iLockType = TS_LF_READWRITE;
	memset(&tChange, 0, sizeof(tChange));
	TEST_CHECK(__xgeTsfStoreSetText(&tStore.tStore, 0, 0, 0, L"nihao", 5, &tChange) == S_OK,
	           "insert preedit");
	tStore.iLockType = 0;
	__xgeTsfStoreFlushEvents(&tStore);
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_START, ""), "ordered IME start");
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_UPDATE, "nihao"), "ordered preedit update");
	TEST_CHECK(!__xgeTsfCompositionNeedsImmFallback(&tStore),
		"real TSF preedit does not use IMM fallback");

	/* Backspace is represented only as a TSF text-store edit. It must not leak
	 * a KEY_DOWN event to XUI while the composition is owned by the TIP. */
	tStore.iLockType = TS_LF_READWRITE;
	TEST_CHECK(__xgeTsfStoreSetText(&tStore.tStore, 0, 4, 5, L"", 0, &tChange) == S_OK,
	           "delete from preedit");
	tStore.iLockType = 0;
	__xgeTsfStoreFlushEvents(&tStore);
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_UPDATE, "niha"), "preedit-only backspace");
	TEST_CHECK(xgeInputEventPendingCount() == 0, "no leaked key event");

	TEST_CHECK(__xgeTsfCompositionEnd(&tStore.tCompositionSink, NULL) == S_OK,
	           "composition end");
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_COMMIT, "niha"), "commit text");
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_END, ""), "ordered IME end");

	/* The production store exposes surrounding document text to TSF while
	 * emitting only the active composition range back to XUI. */
	memset(&tSnapshot, 0, sizeof(tSnapshot));
	tSnapshot.iSize = sizeof(tSnapshot);
	tSnapshot.sText = "abc123xyz";
	tSnapshot.iTextSize = 9;
	tSnapshot.iSelectionStart = 3;
	tSnapshot.iSelectionEnd = 6;
	tSnapshot.iDocumentOffset = 100;
	TEST_CHECK(__xgeTsfStoreApplyClientSnapshot(&tStore, &tSnapshot), "apply client snapshot");
	bOk = FALSE;
	TEST_CHECK(__xgeTsfCompositionStart(&tStore.tCompositionSink, NULL, &bOk) == S_OK && bOk,
	           "snapshot composition start");
	TEST_CHECK(__xgeTsfCompositionNeedsImmFallback(&tStore),
		"unchanged selected text allows IMM fallback before TSF edit");
	tStore.iLockType = TS_LF_READWRITE;
	TEST_CHECK(__xgeTsfStoreSetText(&tStore.tStore, 0, 3, 6, L"nihao", 5, &tChange) == S_OK,
	           "replace selected snapshot text");
	tStore.iLockType = 0;
	__xgeTsfStoreFlushEvents(&tStore);
	TEST_CHECK(__testNextImeRange(XGE_EVENT_IME_START, "", 103, 106), "snapshot IME start range");
	TEST_CHECK(__testNextImeRange(XGE_EVENT_IME_UPDATE, "nihao", 103, 106), "composition update range");
	TEST_CHECK(!__xgeTsfCompositionNeedsImmFallback(&tStore),
		"TSF replacement disables IMM fallback");
	TEST_CHECK(__xgeTsfCompositionEnd(&tStore.tCompositionSink, NULL) == S_OK,
	           "snapshot composition end");
	TEST_CHECK(__testNextImeRange(XGE_EVENT_IME_COMMIT, "nihao", 103, 106), "composition commit range");
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_END, ""), "snapshot IME end");
	TEST_CHECK(wcscmp(tStore.sText, L"abcnihaoxyz") == 0, "surrounding text remains in store");

	/* Ending an unchanged composition is cancellation, not a commit. */
	memset(&tSnapshot, 0, sizeof(tSnapshot));
	tSnapshot.iSize = sizeof(tSnapshot);
	tSnapshot.sText = "cancel";
	tSnapshot.iTextSize = 6;
	tSnapshot.iSelectionStart = 0;
	tSnapshot.iSelectionEnd = 6;
	TEST_CHECK(__xgeTsfStoreApplyClientSnapshot(&tStore, &tSnapshot), "apply cancellation snapshot");
	bOk = FALSE;
	TEST_CHECK(__xgeTsfCompositionStart(&tStore.tCompositionSink, NULL, &bOk) == S_OK && bOk,
	           "cancellation composition start");
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_START, ""), "cancellation start event");
	TEST_CHECK(__xgeTsfCompositionEnd(&tStore.tCompositionSink, NULL) == S_OK,
	           "cancellation composition end");
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_END, ""), "cancellation emits only end");
	TEST_CHECK(xgeInputEventPendingCount() == 0, "cancellation emits no commit");

	/* Some TIPs edit the text store before OnStartComposition. The edit must
	 * remain pending and retain the original document replacement range. */
	memset(&tSnapshot, 0, sizeof(tSnapshot));
	tSnapshot.iSize = sizeof(tSnapshot);
	tSnapshot.sText = "abc123xyz";
	tSnapshot.iTextSize = 9;
	tSnapshot.iSelectionStart = 3;
	tSnapshot.iSelectionEnd = 6;
	tSnapshot.iDocumentOffset = 20;
	TEST_CHECK(__xgeTsfStoreApplyClientSnapshot(&tStore, &tSnapshot), "apply insert-first snapshot");
	tStore.iLockType = TS_LF_READWRITE;
	TEST_CHECK(__xgeTsfStoreInsertTextAtSelection(&tStore.tStore, TS_IAS_NOQUERY,
		L"n", 1, NULL, NULL, &tChange) == S_OK,
		"insert before composition start");
	bOk = FALSE;
	TEST_CHECK(__xgeTsfCompositionStart(&tStore.tCompositionSink, NULL, &bOk) == S_OK && bOk,
		"insert-first composition start");
	tStore.iLockType = 0;
	__xgeTsfStoreFlushEvents(&tStore);
	TEST_CHECK(__testNextImeRange(XGE_EVENT_IME_START, "", 23, 26), "insert-first start range");
	TEST_CHECK(__testNextImeRange(XGE_EVENT_IME_UPDATE, "n", 23, 26), "insert-first update range");
	TEST_CHECK(__xgeTsfCompositionEnd(&tStore.tCompositionSink, NULL) == S_OK,
		"insert-first composition end");
	TEST_CHECK(__testNextImeRange(XGE_EVENT_IME_COMMIT, "n", 23, 26), "insert-first commit range");
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_END, ""), "insert-first IME end");
	TEST_CHECK(wcscmp(tStore.sText, L"abcnxyz") == 0, "insert-first store text");

	memset(&tSnapshot, 0, sizeof(tSnapshot));
	tSnapshot.iSize = sizeof(tSnapshot);
	tSnapshot.sText = "selected";
	tSnapshot.iTextSize = 8;
	tSnapshot.iSelectionStart = 0;
	tSnapshot.iSelectionEnd = 8;
	TEST_CHECK(__xgeTsfStoreApplyClientSnapshot(&tStore, &tSnapshot), "apply empty-composition snapshot");
	bOk = FALSE;
	TEST_CHECK(__xgeTsfCompositionStart(&tStore.tCompositionSink, NULL, &bOk) == S_OK && bOk,
		"empty-composition start");
	TEST_CHECK(__testNextImeRange(XGE_EVENT_IME_START, "", 0, 8), "empty-composition start range");
	tStore.iLockType = TS_LF_READWRITE;
	TEST_CHECK(__xgeTsfStoreSetText(&tStore.tStore, 0, 0, 8, L"", 0, &tChange) == S_OK,
		"empty TSF composition edit");
	tStore.iLockType = 0;
	__xgeTsfStoreFlushEvents(&tStore);
	TEST_CHECK(__testNextImeRange(XGE_EVENT_IME_UPDATE, "", 0, 8), "empty TSF update range");
	TEST_CHECK(__xgeTsfCompositionNeedsImmFallback(&tStore),
		"empty TSF composition uses IMM fallback");
	TEST_CHECK(__xgeTsfCompositionEnd(&tStore.tCompositionSink, NULL) == S_OK,
		"empty-composition end");
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_COMMIT, ""), "empty-composition commit placeholder");
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_END, ""), "empty-composition IME end");

	/* A text service may also perform a direct edit without opening a
	 * composition. Such an edit commits when the write lock is released. */
	memset(&tSnapshot, 0, sizeof(tSnapshot));
	tSnapshot.iSize = sizeof(tSnapshot);
	tSnapshot.sText = "direct";
	tSnapshot.iTextSize = 6;
	tSnapshot.iSelectionStart = 0;
	tSnapshot.iSelectionEnd = 6;
	tSnapshot.iDocumentOffset = 40;
	TEST_CHECK(__xgeTsfStoreApplyClientSnapshot(&tStore, &tSnapshot), "apply direct-edit snapshot");
	tStore.iLockType = TS_LF_READWRITE;
	TEST_CHECK(__xgeTsfStoreInsertTextAtSelection(&tStore.tStore, TS_IAS_NOQUERY,
		L"X", 1, NULL, NULL, &tChange) == S_OK, "direct text-store edit");
	tStore.iLockType = 0;
	__xgeTsfStoreFlushEvents(&tStore);
	TEST_CHECK(__testNextImeRange(XGE_EVENT_IME_COMMIT, "X", 40, 46), "direct edit commit range");
	TEST_CHECK(xgeInputEventPendingCount() == 0, "direct edit emits no composition lifecycle");

cleanup:
	__xgeTsfStoreUnit(&tStore);
	xgeUnit();
	if ( iFailed ) return 1;
	printf("test_tsf_composition_store passed\n");
	return 0;
}
