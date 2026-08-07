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

int main(void)
{
	xge_desc_t tDesc;
	xge_tsf_text_store_t tStore;
	TS_TEXTCHANGE tChange;
	xge_ime_text_snapshot_t tSnapshot;
	WINBOOL bOk;
	int iFailed;

	iFailed = 0;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iRunMode = XGE_RUN_MANUAL;
	TEST_CHECK(xgeInit(&tDesc) == XGE_OK, "xgeInit");
	__xgeTsfStoreInitialize(&tStore);

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
	TEST_CHECK(__xgeTsfStoreApplyClientSnapshot(&tStore, &tSnapshot), "apply client snapshot");
	bOk = FALSE;
	TEST_CHECK(__xgeTsfCompositionStart(&tStore.tCompositionSink, NULL, &bOk) == S_OK && bOk,
	           "snapshot composition start");
	tStore.iLockType = TS_LF_READWRITE;
	TEST_CHECK(__xgeTsfStoreSetText(&tStore.tStore, 0, 3, 6, L"nihao", 5, &tChange) == S_OK,
	           "replace selected snapshot text");
	tStore.iLockType = 0;
	__xgeTsfStoreFlushEvents(&tStore);
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_START, ""), "snapshot IME start");
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_UPDATE, "nihao"), "composition range update");
	TEST_CHECK(__xgeTsfCompositionEnd(&tStore.tCompositionSink, NULL) == S_OK,
	           "snapshot composition end");
	TEST_CHECK(__testNextIme(XGE_EVENT_IME_COMMIT, "nihao"), "composition-only commit");
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

cleanup:
	__xgeTsfStoreUnit(&tStore);
	xgeUnit();
	if ( iFailed ) return 1;
	printf("test_tsf_composition_store passed\n");
	return 0;
}
