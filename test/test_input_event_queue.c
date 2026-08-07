#include "xge.h"

#include <stdio.h>
#include <string.h>

#define TEST_CHECK(expr, message) \
	do { \
		if ( !(expr) ) { \
			printf("test_input_event_queue failed: %s\n", (message)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __testPost(int iType, int iKey, uint32_t iCodepoint, const char* sText, uint32_t iFlags)
{
	xge_input_event_t tEvent;

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = iType;
	tEvent.iKey = iKey;
	tEvent.iCodepoint = iCodepoint;
	tEvent.iModifiers = XGE_KEY_MOD_CTRL;
	tEvent.iFlags = iFlags;
	tEvent.sText = sText;
	tEvent.iTextSize = (sText != NULL) ? (int)strlen(sText) : 0;
	tEvent.iCursor = tEvent.iTextSize;
	tEvent.iSelectStart = tEvent.iCursor;
	tEvent.iSelectEnd = tEvent.iCursor;
	return xgeInputEventPost(&tEvent);
}

static int __testPostPointer(int iType, uint64_t iPointerId, int iButton,
	uint32_t iButtons, float fX, float fY)
{
	xge_input_event_t tEvent;

	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = iType;
	tEvent.iPointerId = iPointerId;
	tEvent.iButton = iButton;
	tEvent.iButtons = iButtons;
	tEvent.fX = fX;
	tEvent.fY = fY;
	return xgeInputEventPost(&tEvent);
}

int main(void)
{
	xge_desc_t tDesc;
	xge_input_event_t tEvent;
	char sLongText[384];
	uint64_t iLastSequence;
	int iFailed;
	int i;
	int iRet;

	iFailed = 0;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iRunMode = XGE_RUN_MANUAL;
	TEST_CHECK(xgeInit(&tDesc) == XGE_OK, "xgeInit");

	TEST_CHECK(__testPost(XGE_EVENT_KEY_DOWN, 'A', 0u, NULL, 0u) == XGE_OK, "post key down");
	TEST_CHECK(__testPostPointer(XGE_EVENT_MOUSE_DOWN, 0u, XGE_MOUSE_LEFT,
		XGE_MOUSE_LEFT, 25.0f, 30.0f) == XGE_OK, "post mouse down");
	TEST_CHECK(__testPost(XGE_EVENT_TEXT, 0, 'a', NULL, 0u) == XGE_OK, "post text");
	TEST_CHECK(__testPostPointer(XGE_EVENT_TOUCH_MOVE, 7u, XGE_MOUSE_LEFT,
		XGE_MOUSE_LEFT, 80.0f, 90.0f) == XGE_OK, "post touch move");
	TEST_CHECK(__testPost(XGE_EVENT_IME_UPDATE, 0, 0u, "nihao", XGE_INPUT_EVENT_FLAG_NATIVE_IME) == XGE_OK, "post IME update");
	TEST_CHECK(__testPost(XGE_EVENT_KEY_UP, 'A', 0u, NULL, 0u) == XGE_OK, "post key up");
	TEST_CHECK(xgeInputEventPendingCount() == 6, "mixed event count");

	iLastSequence = 0u;
	for ( i = 0; i < 6; i++ ) {
		memset(&tEvent, 0, sizeof(tEvent));
		iRet = xgeInputEventGet(&tEvent);
		TEST_CHECK(iRet == 1, "mixed event dequeue");
		TEST_CHECK(tEvent.iSequence > iLastSequence, "monotonic sequence");
		TEST_CHECK((tEvent.iFlags & XGE_INPUT_EVENT_FLAG_SYNTHETIC) != 0u, "synthetic flag");
		iLastSequence = tEvent.iSequence;
		if ( i == 0 ) TEST_CHECK(tEvent.iType == XGE_EVENT_KEY_DOWN && tEvent.iKey == 'A', "key down order");
		if ( i == 1 ) TEST_CHECK(tEvent.iType == XGE_EVENT_MOUSE_DOWN &&
			tEvent.iButton == XGE_MOUSE_LEFT && tEvent.fX == 25.0f, "mouse order");
		if ( i == 2 ) TEST_CHECK(tEvent.iType == XGE_EVENT_TEXT && tEvent.iCodepoint == 'a', "text order");
		if ( i == 3 ) TEST_CHECK(tEvent.iType == XGE_EVENT_TOUCH_MOVE &&
			tEvent.iPointerId == 7u && tEvent.fY == 90.0f, "touch order");
		if ( i == 4 ) TEST_CHECK(tEvent.iType == XGE_EVENT_IME_UPDATE && strcmp(tEvent.sText, "nihao") == 0, "IME order");
		if ( i == 5 ) TEST_CHECK(tEvent.iType == XGE_EVENT_KEY_UP && tEvent.iKey == 'A', "key up order");
	}
	TEST_CHECK(xgeInputEventGet(&tEvent) == 0, "queue empty");

	for ( i = 0; i < (int)sizeof(sLongText) - 1; i++ ) sLongText[i] = (char)('a' + (i % 26));
	sLongText[sizeof(sLongText) - 1] = '\0';
	TEST_CHECK(__testPost(XGE_EVENT_IME_UPDATE, 0, 0u, sLongText, XGE_INPUT_EVENT_FLAG_NATIVE_IME) == XGE_OK, "post long IME text");
	TEST_CHECK(xgeInputEventGet(&tEvent) == 1, "get long IME text");
	TEST_CHECK(tEvent.iTextSize == (int)strlen(sLongText), "long IME size");
	TEST_CHECK(strcmp(tEvent.sText, sLongText) == 0, "long IME content");

	for ( i = 0; i < 600; i++ ) {
		TEST_CHECK(__testPost(XGE_EVENT_KEY_DOWN, 'A' + (i % 26), 0u, NULL, XGE_INPUT_EVENT_FLAG_REPEAT) == XGE_OK, "post expanded queue event");
	}
	TEST_CHECK(xgeInputEventPendingCount() == 600, "expanded queue count");
	for ( i = 0; i < 600; i++ ) {
		TEST_CHECK(xgeInputEventGet(&tEvent) == 1, "get expanded queue event");
		TEST_CHECK(tEvent.iKey == 'A' + (i % 26), "expanded queue order");
	}
	TEST_CHECK(xgeInputEventDroppedCount() == 0u, "no dropped events");

cleanup:
	xgeUnit();
	if ( iFailed ) return 1;
	printf("test_input_event_queue passed\n");
	return 0;
}
