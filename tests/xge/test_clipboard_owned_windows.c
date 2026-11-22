/* Link the direct-XRT XGE archive with
 * --wrap=__imp_IsClipboardFormatAvailable,--wrap=__imp_OpenClipboard,
 * --wrap=__imp_GetClipboardData,--wrap=__imp_CloseClipboard.
 * The shim never opens or changes the user's clipboard. */
#define WIN32_LEAN_AND_MEAN
#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#define XRT_USE_SHARED
#include <windows.h>
#include "xge.h"
#include "xrt.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static HGLOBAL payload;
static bool opened, blocked, reenter;
static unsigned requests;

static BOOL WINAPI hasFormat(UINT format)
{
    assert(format == CF_UNICODETEXT);
    return payload != NULL;
}

static BOOL WINAPI openClipboard(HWND owner)
{
    (void)owner;
    assert(!opened);
    opened = !blocked;
    return opened;
}

static BOOL WINAPI closeClipboard(void)
{
    assert(opened);
    opened = false;
    return TRUE;
}

static HANDLE WINAPI getClipboard(UINT format)
{
    assert(opened && format == CF_UNICODETEXT);
    ++requests;
    if (reenter) {
        char* result = (char*)1;
        size_t bytes = 7;
        assert(xgeClipboardGetTextOwned(&result, &bytes) == XGE_ERROR_BUSY && !result && !bytes);
        assert(xgeUnitChecked() == XGE_ERROR_BUSY);
    }
    return payload;
}

BOOL (WINAPI* __wrap___imp_IsClipboardFormatAvailable)(UINT) = hasFormat;
BOOL (WINAPI* __wrap___imp_OpenClipboard)(HWND) = openClipboard;
HANDLE (WINAPI* __wrap___imp_GetClipboardData)(UINT) = getClipboard;
BOOL (WINAPI* __wrap___imp_CloseClipboard)(void) = closeClipboard;

static void setPayload(const WCHAR* source, size_t bytes)
{
    if (payload) GlobalFree(payload);
    payload = GlobalAlloc(GMEM_MOVEABLE, bytes);
    assert(payload);
    void* target = GlobalLock(payload);
    assert(target);
    memset(target, 1, GlobalSize(payload));
    memcpy(target, source, bytes);
    GlobalUnlock(payload);
}

static int32 foreignThread(ptr unused)
{
    (void)unused;
    char* result = (char*)1;
    size_t bytes = 7;
    assert(xgeClipboardGetTextOwned(&result, &bytes) == XGE_ERROR_INVALID_STATE && !result && !bytes);
    return 0;
}

int main(void)
{
    const WCHAR valid[] = {0x4e2d,0xd83d,0xde00,0};
    const WCHAR empty[] = {0};
    const WCHAR surrogate[] = {0xd800,0};
    const WCHAR unterminated[] = {'A'};
    const char expected[] = "\xe4\xb8\xad\xf0\x9f\x98\x80";
    char* text = (char*)1;
    size_t bytes = 7;
    assert(xgeClipboardGetTextOwned(&text, &bytes) == XGE_ERROR_NOT_INITIALIZED && !text && !bytes);
    assert(xgeInit(NULL) == XGE_OK);
    assert(xgeClipboardGetTextOwned(NULL, &bytes) == XGE_ERROR_INVALID_ARGUMENT && !bytes);
    assert(xgeClipboardGetTextOwned(&text, NULL) == XGE_ERROR_INVALID_ARGUMENT && !text);
    assert(xgeClipboardGetTextOwned(&text, &bytes) == XGE_OK && !text && !bytes);
    xthread* thread = xrtThreadCreate(foreignThread,NULL,0);
    assert(thread && xrtThreadWait(thread) == XWAIT_OK);
    xrtThreadDestroy(thread);
    assert(requests == 0);

    setPayload(valid,sizeof(valid));
    reenter = true;
    assert(xgeClipboardGetTextOwned(&text, &bytes) == XGE_OK);
    reenter = false;
    assert(requests == 1 && bytes == sizeof(expected)-1 && !memcmp(text,expected,sizeof(expected)) && !opened);
    char* retained = text;
    setPayload(empty,sizeof(empty));
    assert(xgeClipboardGetTextOwned(&text, &bytes) == XGE_OK && !text && !bytes);
    assert(!memcmp(retained,expected,sizeof(expected)));

    setPayload(surrogate,sizeof(surrogate));
    assert(xgeClipboardGetTextOwned(&text, &bytes) == XGE_ERROR_RESOURCE_FAILED && !text && !bytes && !opened);
    setPayload(unterminated,sizeof(unterminated));
    assert(xgeClipboardGetTextOwned(&text, &bytes) == XGE_ERROR_RESOURCE_FAILED && !text && !bytes && !opened);
    setPayload(valid,sizeof(valid));
    assert(xrtMemDebugFailAfter(0));
    assert(xgeClipboardGetTextOwned(&text, &bytes) == XGE_ERROR_OUT_OF_MEMORY && !text && !bytes && !opened);
    assert(xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtClearError();
    blocked = true;
    assert(xgeClipboardGetTextOwned(&text, &bytes) == XGE_ERROR_BACKEND_FAILED && !text && !bytes);
    blocked = false;
    assert(xgeClipboardGetTextOwned(&text, &bytes) == XGE_OK && !memcmp(text,expected,sizeof(expected)));
    xrtFree(text);
    assert(xgeUnitChecked() == XGE_OK && !memcmp(retained,expected,sizeof(expected)));
    xrtFree(retained);
    GlobalFree(payload);
    puts("XGE_CLIPBOARD_WINDOWS_OWNED_OK");
    return 0;
}
