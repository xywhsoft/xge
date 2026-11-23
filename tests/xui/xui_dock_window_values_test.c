#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#include "xui.h"
#include "xui_test_proxy.h"

#include <assert.h>
#include <stdio.h>

typedef struct window_value_state_t {
    xui_widget dock;
    xvalue* replacement;
    int drops;
} window_value_state_t;

static bool trace_empty(const xvalue* value, xrtownershipvisitor visit, void* user)
{
    (void)value;
    (void)visit;
    (void)user;
    return true;
}

static void restore_after_clear(void* data, void* user)
{
    window_value_state_t* state = data;
    (void)user;
    ++state->drops;
    assert(xuiDockPanelGetWindowCount(state->dock) == 0);
    int index = -1;
    assert(xuiDockPanelAddWindow(state->dock, "Replacement", NULL, &index) == XUI_OK);
    assert(index == 0);
    assert(xuiDockPanelSetWindowValue(state->dock, index, state->replacement) == XUI_OK);
}

static void count_drop(void* data, void* user)
{
    (void)user;
    ++*(int*)data;
}

static xvalue* make_owned(void* data, const xvaluehandleops* ops)
{
    xvalue* value = xrtValueHandleTake(&data, ops, NULL);
    assert(value != NULL && data == NULL);
    assert(xrtValueHandleOwnershipBindPhased(value, trace_empty));
    return value;
}

typedef struct value_edge_t {
    xrtownershipref ref;
    int count;
} value_edge_t;

static bool count_edge(xrtownershipref ref, void* data)
{
    value_edge_t* edge = data;
    if (ref.Data == edge->ref.Data && ref.Ops == edge->ref.Ops) ++edge->count;
    return true;
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
    xui_widget dock = NULL;
    assert(xuiDockPanelCreate(context, &dock, NULL) == XUI_OK);

    int document = -1;
    int tools = -1;
    assert(xuiDockPanelAddWindow(dock, "Document", NULL, &document) == XUI_OK);
    assert(xuiDockPanelAddWindow(dock, "Tools", NULL, &tools) == XUI_OK);
    assert(document == 0 && tools == 1);
    assert(xuiDockPanelDockWindow(dock, document, XUI_DOCK_PANEL_REGION_DOCUMENT,
                                 XUI_DOCK_PANEL_SIDE_FILL, 0, NULL) == XUI_OK);
    assert(xuiDockPanelDockWindow(dock, tools, XUI_DOCK_PANEL_REGION_RIGHT,
                                 XUI_DOCK_PANEL_SIDE_FILL, 0, NULL) == XUI_OK);
    assert(xuiDockPanelSetWindowUserData(dock, tools, &backend) == XUI_OK);

    xvalue* value = xrtValueInt(77);
    xvalue* replacement = xrtValueInt(78);
    assert(value != NULL && replacement != NULL);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiDockPanelSetWindowValue(dock, tools, value) == XUI_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtSetErrorTake(NULL);
    xvalue* output = value;
    assert(xuiDockPanelGetWindowValue(dock, tools, &output) == XUI_OK && output == NULL);
    assert(xuiDockPanelSetWindowValue(dock, tools, value) == XUI_OK);
    assert(xuiDockPanelGetWindowUserData(dock, tools) == &backend);
    value_edge_t edge = {xrtValueOwnership(value), 0};
    assert(xuiContextTraceOwnership(context, count_edge, &edge) && edge.count == 1);

    xvalue* layout = NULL;
    assert(xuiDockPanelSaveState(dock, &layout) == XUI_OK && layout != NULL);
    assert(xuiDockPanelFloatWindow(dock, tools, (xui_rect_t){24, 28, 280, 180}) == XUI_OK);
    assert(xuiDockPanelGetWindowValue(dock, tools, &output) == XUI_OK && output == value);
    xrtValueRelease(output);
    assert(xuiDockPanelHideWindow(dock, tools) == XUI_OK);
    assert(xuiDockPanelGetWindowValue(dock, tools, &output) == XUI_OK && output == value);
    xrtValueRelease(output);
    assert(xuiDockPanelLoadState(dock, layout) == XUI_OK);
    xuiDockPanelStateFree(layout);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiDockPanelGetWindowValue(dock, tools, &output) == XUI_OK && output == value);
    assert(xuiDockPanelSetWindowValue(dock, tools, replacement) == XUI_OK);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtValueRelease(output);

    static const xvaluehandleops restore_ops = {.Drop = restore_after_clear};
    window_value_state_t state = {dock, replacement, 0};
    xvalue* restorer = make_owned(&state, &restore_ops);
    assert(xuiDockPanelSetWindowValue(dock, document, restorer) == XUI_OK);
    xrtValueRelease(restorer);
    assert(xuiDockPanelClear(dock) == XUI_OK && state.drops == 1);
    assert(xuiDockPanelGetWindowCount(dock) == 1);
    assert(xuiDockPanelGetWindowValue(dock, document, &output) == XUI_OK && output == replacement);
    xrtValueRelease(output);
    assert(xuiDockPanelGetWindowValue(dock, tools, &output) == XUI_ERROR_INVALID_ARGUMENT && output == NULL);

    int destroy_drops = 0;
    static const xvaluehandleops count_ops = {.Drop = count_drop};
    xvalue* on_destroy = make_owned(&destroy_drops, &count_ops);
    assert(xuiDockPanelSetWindowValue(dock, document, on_destroy) == XUI_OK);
    xrtValueRelease(on_destroy);
    assert(xuiWidgetDestroyChecked(dock) == XUI_OK && destroy_drops == 1);
    assert(xuiDestroyChecked(context) == XUI_OK);
    xrtValueRelease(value);
    xrtValueRelease(replacement);
    puts("xui_dock_window_values_test passed");
    return 0;
}
