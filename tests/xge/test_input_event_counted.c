#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include "xge.h"
#include "xrt.h"

#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int32 reject_other_thread(void* data)
{
    const xge_input_event_info_t* event = data;
    xge_input_event_info_t output = { .iType = -1 };
    unsigned char text = 0xcc;
    assert(xgeInputEventRead(&output, &text, 1) == XGE_ERROR_INVALID_STATE);
    assert(output.iType == -1 && text == 0xcc);
    assert(xgeInputEventPostData(event, NULL, 0) == XGE_ERROR_INVALID_STATE);
    return 0;
}

int main(void)
{
    xge_input_event_info_t ime = {
        .iType = XGE_EVENT_IME_UPDATE, .iFlags = XGE_INPUT_EVENT_FLAG_NATIVE_IME,
        .iSequence = 250, .fTime = 12.5, .iKey = 11, .iNativeKey = 12, .iScanCode = 13,
        .iModifiers = XGE_KEY_MOD_CTRL, .iPointerId = UINT64_C(0x100000003),
        .iButton = 16, .iButtons = 17, .fX = 18, .fY = 19, .fDX = 20, .fDY = 21,
        .iCodepoint = 0x4e2d, .iTextSize = 999, .iCursor = 4, .iSelectStart = 1,
        .iSelectEnd = 4, .bReplacementRange = 1, .iReplacementStart = 6,
        .iReplacementEnd = 9, .iAttribute = XGE_IME_ATTRIBUTE_NONE
    };
    xge_input_event_info_t key = { .iType = XGE_EVENT_KEY_UP, .iKey = 'A' };
    xge_input_event_info_t got = { .iType = -1 };
    unsigned char output[512];
    unsigned char text[384];
    memset(output, 0xcc, sizeof(output));
    memset(text, 'x', sizeof(text));
    text[1] = 0;
    assert(xgeInputEventRead(&got, output, sizeof(output)) == XGE_ERROR_NOT_INITIALIZED);
    assert(got.iType == -1 && output[0] == 0xcc);
    assert(xgeInputEventPostData(&key, NULL, 0) == XGE_ERROR_NOT_INITIALIZED);
    xge_desc_t desc = { .iRunMode = XGE_RUN_MANUAL };
    assert(xgeInit(&desc) == XGE_OK);

    assert(xgeInputEventPostData(&ime, "old", 3) == XGE_OK);
    xge_input_event_t raw;
    assert(xgeInputEventGet(&raw) == 1 && raw.iTextSize == 3);
    assert(xgeInputEventPostData(&ime, text, sizeof(text)) == XGE_OK);
    text[2] = 'y';
    assert(xgeInputEventPostData(&key, NULL, 0) == XGE_OK);

    xthread* worker = xrtThreadCreate(reject_other_thread, &key, 0);
    assert(worker != NULL && xrtThreadWait(worker) == XWAIT_OK);
    xrtThreadDestroy(worker);
    assert(xgeInputEventPendingCount() == 2);
    assert(xgeInputEventRead(&got, NULL, 1) == XGE_ERROR_INVALID_ARGUMENT && got.iType == -1);
    assert(xgeInputEventRead(&got, output, sizeof(text) - 1) == XGE_INPUT_READ_NEED_BUFFER);
    assert(xgeInputEventPendingCount() == 2 && output[0] == 0xcc);
    assert(got.iTextSize == sizeof(text) && got.iSequence > raw.iSequence);
    uint64_t sequence = got.iSequence;
    assert(xgeInputEventRead(&got, output, sizeof(output)) == XGE_INPUT_READ_READY);
    assert(got.iSize == sizeof(got) && got.iSequence == sequence && got.iSequence != ime.iSequence);
    assert(got.iType == ime.iType && got.iFlags == (ime.iFlags | XGE_INPUT_EVENT_FLAG_SYNTHETIC));
    assert(got.fTime == ime.fTime && got.iKey == ime.iKey && got.iNativeKey == ime.iNativeKey);
    assert(got.iScanCode == ime.iScanCode && got.iModifiers == ime.iModifiers && got.iPointerId == ime.iPointerId);
    assert(got.iButton == ime.iButton && got.iButtons == ime.iButtons);
    assert(got.fX == ime.fX && got.fY == ime.fY && got.fDX == ime.fDX && got.fDY == ime.fDY);
    assert(got.iCodepoint == ime.iCodepoint && got.iTextSize == sizeof(text) && got.iCursor == ime.iCursor);
    assert(got.iSelectStart == ime.iSelectStart && got.iSelectEnd == ime.iSelectEnd);
    assert(got.bReplacementRange == ime.bReplacementRange && got.iAttribute == ime.iAttribute);
    assert(got.iReplacementStart == ime.iReplacementStart && got.iReplacementEnd == ime.iReplacementEnd);
    assert(output[0] == 'x' && output[1] == 0 && output[2] == 'x' && output[sizeof(text)] == 0xcc);
    assert(memcmp(raw.sText, "old", 4) == 0);
    assert(xgeInputEventGet(&raw) == 1 && raw.iType == key.iType && raw.iKey == key.iKey);
    assert(xgeInputEventRead(&got, NULL, 0) == XGE_INPUT_READ_EMPTY && got.iType == 0 && got.iSize == sizeof(got));

    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    uint64_t dropped = xgeInputEventDroppedCount();
    assert(xgeInputEventPostData(&key, NULL, 0) == XGE_OK);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeInputEventPostData(&ime, text, sizeof(text)) == XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtClearError();
    assert(xgeInputEventDroppedCount() == dropped + 1 && xgeInputEventPendingCount() == 1);
    assert(xgeInputEventRead(&got, NULL, 0) == XGE_INPUT_READ_READY && got.iKey == key.iKey);

    assert(xgeUnitChecked() == XGE_OK);
    puts("XGE_COUNTED_INPUT_NATIVE_OK");
    return 0;
}
