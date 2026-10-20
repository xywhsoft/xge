#include <stdio.h>
#include <string.h>

/* White-box coverage keeps the platform policy test close to the private
 * IMM fallback without adding test-only exports to the public ABI. */
#include "single/xge.c"

#define TEST_CHECK(expr, message) \
	do { \
		if ( !(expr) ) { \
			printf("test_ime_mode_contract failed: %s\n", (message)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

int main(void)
{
	xge_desc_t tDesc;
	xge_input_event_t tEvent;
	xge_ime_event_t tImeEvent;
	xge_tsf_text_store_t* pStore;
	ITfContext* pSavedContext;
	int bHandled;
	int iFailed;

	iFailed = 0;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iRunMode = XGE_RUN_MANUAL;
	TEST_CHECK(xgeInit(&tDesc) == XGE_OK, "xgeInit");

	/* Native mode delegates preedit and candidate presentation to Windows. */
	g_xgeWin32Ime.iMode = XGE_IME_MODE_NATIVE;
	g_xgeWin32Ime.bTsfInitialized = 0;
	bHandled = 0;
	(void)__xgeImeHandleImmMessage(NULL, WM_IME_STARTCOMPOSITION, 0, 0, &bHandled);
	TEST_CHECK(xgeInputEventPendingCount() == 0, "native mode must not publish marked text");
	bHandled = 0;
	(void)__xgeImeHandleImmMessage(NULL, WM_IME_ENDCOMPOSITION, 0, 0, &bHandled);
	TEST_CHECK(xgeInputEventPendingCount() == 0, "native mode must not publish composition end");

	/* Composition/Full retain the IMM marked-text fallback when TSF is absent. */
	g_xgeWin32Ime.iMode = XGE_IME_MODE_COMPOSITION;
	bHandled = 0;
	(void)__xgeImeHandleImmMessage(NULL, WM_IME_STARTCOMPOSITION, 0, 0, &bHandled);
	memset(&tEvent, 0, sizeof(tEvent));
	TEST_CHECK(xgeInputEventGet(&tEvent) == 1, "composition fallback event");
	TEST_CHECK(tEvent.iType == XGE_EVENT_IME_START, "composition fallback start type");
	bHandled = 0;
	(void)__xgeImeHandleImmMessage(NULL, WM_IME_ENDCOMPOSITION, 0, 0, &bHandled);
	memset(&tEvent, 0, sizeof(tEvent));
	TEST_CHECK(xgeInputEventGet(&tEvent) == 1, "composition fallback end event");
	TEST_CHECK(tEvent.iType == XGE_EVENT_IME_END, "composition fallback end type");

	/* Losing the logical text owner must recover even when the TIP omitted END. */
	pStore = &g_xgeWin32Ime.tTextStore;
	pSavedContext = g_xgeWin32Ime.pContext;
	g_xgeWin32Ime.pContext = NULL;
	g_xge.iImeFocusOwner = 41u;
	pStore->iImeOwner = 41u;
	pStore->bComposing = 1;
	pStore->iAttribute = XGE_IME_ATTRIBUTE_INPUT_ERROR;
	xgeInternalImeSetFocusOwner(42u);
	TEST_CHECK(!pStore->bComposing && !pStore->bEndPending,
		"owner change clears stale TSF composition");
	TEST_CHECK(pStore->iAttribute == XGE_IME_ATTRIBUTE_NONE,
		"owner change clears stale display attribute");
	memset(&tEvent, 0, sizeof(tEvent));
	TEST_CHECK(xgeInputEventGet(&tEvent) == 1 &&
		tEvent.iType == XGE_EVENT_IME_END,
		"owner change publishes composition end");
	TEST_CHECK(xgeInternalInputImeOwner() == 41u,
		"composition end keeps the stale owner's identity");
	g_xgeWin32Ime.pContext = pSavedContext;

	/* Invalid marked text is dismissed by the next real key, even if the TIP
	 * never sends another update or end notification. Modifier keys alone do
	 * not disturb the composition. */
	g_xgeWin32Ime.pContext = NULL;
	pStore->iImeOwner = 42u;
	pStore->bComposing = 1;
	pStore->iAttribute = XGE_IME_ATTRIBUTE_INPUT_ERROR;
	__xgeTsfRecoverErrorComposition(WM_KEYDOWN, VK_SHIFT);
	TEST_CHECK(pStore->bComposing, "modifier preserves erroneous composition");
	__xgeTsfRecoverErrorComposition(WM_KEYDOWN, 'A');
	TEST_CHECK(!pStore->bComposing &&
		pStore->iAttribute == XGE_IME_ATTRIBUTE_NONE,
		"next input clears erroneous composition");
	memset(&tEvent, 0, sizeof(tEvent));
	TEST_CHECK(xgeInputEventGet(&tEvent) == 1 &&
		tEvent.iType == XGE_EVENT_IME_END,
		"error recovery publishes composition end");
	g_xgeWin32Ime.pContext = pSavedContext;

	/* Display attributes must survive both the ordered and legacy IME queues. */
	TEST_CHECK(__xgeImeQueuePushOwnedRangeAttribute(XGE_EVENT_IME_UPDATE,
		"bad", 3, 3, 3, 3, 0, 0, 0,
		XGE_IME_ATTRIBUTE_INPUT_ERROR, 42u), "queue attributed update");
	memset(&tEvent, 0, sizeof(tEvent));
	TEST_CHECK(xgeInputEventGet(&tEvent) == 1 &&
		tEvent.iAttribute == XGE_IME_ATTRIBUTE_INPUT_ERROR,
		"ordered input preserves IME error attribute");
	TEST_CHECK(__xgeImeQueuePushOwnedRangeAttribute(XGE_EVENT_IME_UPDATE,
		"bad", 3, 3, 3, 3, 0, 0, 0,
		XGE_IME_ATTRIBUTE_INPUT_ERROR, 42u), "queue legacy attributed update");
	memset(&tImeEvent, 0, sizeof(tImeEvent));
	TEST_CHECK(xgeImeEventGet(&tImeEvent) == 1 &&
		tImeEvent.iAttribute == XGE_IME_ATTRIBUTE_INPUT_ERROR,
		"legacy queue preserves IME error attribute");

cleanup:
	xgeUnit();
	if ( iFailed ) return 1;
	printf("test_ime_mode_contract passed\n");
	return 0;
}
