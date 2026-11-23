#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#include "xui.h"
#include "xui_test_proxy.h"

#include <assert.h>
#include <stdio.h>

static int check_other_thread(void* user)
{
    xui_list_selection_buffer buffer = user;
    assert(xuiListSelectionBufferGetCapacity(buffer) == 4);
    assert(xuiListSelectionBufferGet(buffer, 0) == XUI_ERROR_BUSY);
    xuiListSelectionBufferRelease(buffer);
    return 0;
}

int main(void)
{
    xui_test_proxy_state_t backend;
    xuiTestProxyInit(&backend);
    xui_context_desc_t desc;
    xuiContextDescDefault(&desc);
    desc.iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED;
    xui_context context = NULL;
    assert(xuiCreateEx(&desc, &context) == XUI_OK);
    assert(xuiSetProxy(context, &backend.tProxy) == XUI_OK);
    xui_widget list = NULL;
    xui_widget second = NULL;
    assert(xuiListViewCreate(context, &list, NULL) == XUI_OK);
    assert(xuiListViewCreate(context, &second, NULL) == XUI_OK);

    xui_list_selection_buffer buffer = NULL;
    assert(xuiListSelectionBufferCreate(&buffer, 0) == XUI_ERROR_INVALID_ARGUMENT && !buffer);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiListSelectionBufferCreate(&buffer, 4) == XUI_ERROR_OUT_OF_MEMORY && !buffer);
    assert(xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtSetErrorTake(NULL);
    assert(xuiListSelectionBufferCreate(&buffer, 4) == XUI_OK);
    assert(xuiListSelectionBufferGet(buffer, -1) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiListSelectionBufferGet(buffer, 4) == XUI_ERROR_INVALID_ARGUMENT);

    const char* rows[] = {"One", "Two", "Three", "Four", "Five"};
    assert(xuiListViewSetItems(list, rows, 3) == XUI_OK);
    assert(xuiListViewSetSelectionMode(list, XUI_SELECTION_MULTI) == XUI_OK);
    assert(xuiListViewSetItemSelected(list, 1, 1) == XUI_OK);
    assert(xuiListViewSetSelectionBufferOwned(list, buffer) == XUI_OK);
    assert(xuiListViewSetSelectionBufferOwned(list, buffer) == XUI_OK);
    assert(xuiListViewGetSelectionBufferOwned(list) == buffer);
    assert(xuiListSelectionBufferGet(buffer, 1) == 1);
    assert(xuiListViewSetSelectionBufferOwned(second, buffer) == XUI_ERROR_BUSY);
    assert(xuiListViewGetSelectionBufferOwned(second) == NULL);

    assert(xuiListSelectionBufferAddRef(buffer) == XUI_OK);
    xthread* worker = xrtThreadCreate(check_other_thread, buffer, 0);
    assert(worker && xrtThreadWaitFor(worker, 5000) == XWAIT_OK);
    xrtThreadDestroy(worker);
    xuiListSelectionBufferRelease(buffer);
    buffer = xuiListViewGetSelectionBufferOwned(list);
    assert(xuiListSelectionBufferAddRef(buffer) == XUI_OK);
    assert(xuiListViewSetItemSelected(list, 2, 1) == XUI_OK);
    assert(xuiListViewSetItems(list, rows, 5) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiListViewGetItemCount(list) == 3 && xuiListSelectionBufferGet(buffer, 2) == 1);
    assert(xuiListViewSetItems(list, rows, 2) == XUI_OK);
    assert(xuiListSelectionBufferGet(buffer, 2) == 0);
    assert(xuiListViewSetItems(list, rows, 4) == XUI_OK);
    assert(xuiListSelectionBufferGet(buffer, 2) == 0 && xuiListSelectionBufferGet(buffer, 1) == 1);

    assert(xrtMemDebugFailAfter(0));
    assert(xuiListViewSetSelectionBufferOwned(list, NULL) == XUI_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtSetErrorTake(NULL);
    assert(xuiListViewGetSelectionBufferOwned(list) == buffer && xuiListSelectionBufferGet(buffer, 1) == 1);
    assert(xuiListViewSetSelectionBufferOwned(list, NULL) == XUI_OK);
    assert(xuiListViewIsItemSelected(list, 1) == 1 && xuiListViewGetSelectionBufferOwned(list) == NULL);

    assert(xuiListViewSetItems(second, rows, 2) == XUI_OK);
    assert(xuiListViewSetSelected(second, 0) == XUI_OK);
    assert(xuiListViewSetSelectionBufferOwned(second, buffer) == XUI_OK);
    assert(xuiListSelectionBufferGet(buffer, 0) == 1 && xuiListSelectionBufferGet(buffer, 1) == 0);
    int raw[] = {19, 19, 19};
    assert(xuiListViewSetSelectionBuffer(second, raw, 1) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiListViewSetSelectionBuffer(second, raw, 3) == XUI_OK);
    assert(raw[0] == 1 && raw[1] == 0 && raw[2] == 19 && xuiListViewGetSelectionBufferOwned(second) == NULL);
    assert(xuiListViewSetSelected(second, 1) == XUI_OK);
    assert(raw[0] == 0 && raw[1] == 1 && xuiListSelectionBufferGet(buffer, 0) == 1);
    assert(xuiListViewSetSelectionBufferOwned(second, buffer) == XUI_OK);
    assert(xuiWidgetDestroyChecked(second) == XUI_OK);
    assert(xuiListSelectionBufferGet(buffer, 1) == 1);
    assert(xuiListViewSetSelectionBufferOwned(list, buffer) == XUI_OK);
    assert(xuiDestroyChecked(context) == XUI_OK);
    assert(xuiListSelectionBufferGet(buffer, 1) == 1);
    xuiListSelectionBufferRelease(buffer);
    puts("xui_list_selection_buffer_test passed");
    return 0;
}
