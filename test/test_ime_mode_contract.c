#include <stdio.h>
#include <string.h>

/* White-box coverage keeps the platform policy test close to the private
 * IMM fallback without adding test-only exports to the public ABI. */
#include "../xge.c"

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

cleanup:
	xgeUnit();
	if ( iFailed ) return 1;
	printf("test_ime_mode_contract passed\n");
	return 0;
}
