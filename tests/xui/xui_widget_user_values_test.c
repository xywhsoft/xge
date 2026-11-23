#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#include "xui.h"
#include "xui_test_proxy.h"

#include <assert.h>
#include <stdio.h>

typedef struct user_value_test_t {
    xui_widget widget;
    xvalue* published;
    xvalue* replacement;
    int drops;
} user_value_test_t;

typedef struct user_value_edge_t {
    xrtownershipref identity;
    int count;
} user_value_edge_t;

static bool count_edge(xrtownershipref ref, void* data)
{
    user_value_edge_t* edge = data;
    if (ref.Data == edge->identity.Data && ref.Ops == edge->identity.Ops) ++edge->count;
    return true;
}

static bool trace_empty(const xvalue* value, xrtownershipvisitor visit, void* user)
{
    (void)value;
    (void)visit;
    (void)user;
    return true;
}

static xvalue* make_owned(void* data, const xvaluehandleops* ops)
{
    xvalue* value = xrtValueHandleTake(&data, ops, NULL);
    assert(value != NULL && data == NULL);
    assert(xrtValueHandleOwnershipBindPhased(value, trace_empty));
    return value;
}

static void replace_during_drop(void* data, void* user)
{
    user_value_test_t* state = data;
    (void)user;
    ++state->drops;
    xvalue* output = NULL;
    assert(xuiWidgetGetUserValue(state->widget, &output) == XUI_OK && output == state->published);
    xrtValueRelease(output);
    assert(xuiWidgetSetUserValue(state->widget, state->replacement) == XUI_OK);
    xrtSetErrorInfo(XERR_STATE, "xui_test", XUI_ERROR_CALLBACK_FAILED, "user value drop");
}

static void refuse_during_close(void* data, void* user)
{
    user_value_test_t* state = data;
    (void)user;
    ++state->drops;
    xvalue* output = state->replacement;
    assert(xuiWidgetGetUserValue(state->widget, &output) == XUI_ERROR_INVALID_ARGUMENT && output == NULL);
    assert(xuiWidgetSetUserValue(state->widget, state->replacement) == XUI_ERROR_INVALID_ARGUMENT);
}

static void refuse_busy_value(void* data, void* user)
{
    user_value_test_t* state = data;
    (void)user;
    assert(xuiWidgetSetUserValue(state->widget, state->replacement) == XUI_ERROR_BUSY);
    ++state->drops;
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
    xui_widget root = NULL;
    xui_widget child = NULL;
    assert(xuiWidgetCreate(context, &root) == XUI_OK);
    assert(xuiWidgetCreate(context, &child) == XUI_OK);
    xuiWidgetSetUserData(child, &backend);

    xvalue* first = xrtValueInt(73);
    xvalue* second = xrtValueInt(91);
    assert(first != NULL && second != NULL);
    xvalue* output = first;
    assert(xuiWidgetGetUserValue(child, &output) == XUI_OK && output == NULL);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiWidgetSetUserValue(child, first) == XUI_OK);
    assert(xuiWidgetGetUserValue(child, &output) == XUI_OK && output == first);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtValueRelease(output);

    assert(xuiWidgetAddChild(root, child) == XUI_OK);
    assert(xuiWidgetGetUserValue(root, &output) == XUI_OK && output == NULL);
    user_value_edge_t edge = {xrtValueOwnership(first), 0};
    assert(xuiContextTraceOwnership(context, count_edge, &edge) && edge.count == 1);
    assert(xuiWidgetRemoveFromParent(child) == XUI_OK);
    assert(xuiWidgetGetUserValue(child, &output) == XUI_OK && output == first);
    xrtValueRelease(output);
    assert(xuiWidgetSetUserValue(child, xrtValueNull()) == XUI_OK);
    edge.count = 0;
    assert(xuiContextTraceOwnership(context, count_edge, &edge) && edge.count == 0);
    assert(xuiWidgetGetUserValue(child, &output) == XUI_OK && output == NULL);
    assert(xuiWidgetGetUserData(child) == &backend);

    static const xvaluehandleops replace_ops = {.Drop = replace_during_drop};
    user_value_test_t state = {child, first, second, 0};
    xvalue* reentrant = make_owned(&state, &replace_ops);
    assert(xuiWidgetSetUserValue(child, reentrant) == XUI_OK);
    xrtValueRelease(reentrant);
    assert(xuiWidgetSetUserValue(child, first) == XUI_ERROR_CALLBACK_FAILED);
    assert(state.drops == 1 && xrtGetError() != NULL);
    xrtSetErrorTake(NULL);
    assert(xuiWidgetGetUserValue(child, &output) == XUI_OK && output == second);
    xrtValueRelease(output);

    static const xvaluehandleops busy_ops = {.Drop = refuse_busy_value};
    xvalue* object = xrtValueObject();
    xvalue* member = make_owned(&state, &busy_ops);
    xstrview key = XRT_STR_LITERAL("busy");
    assert(object && xrtValueObjectSet(object, key, member));
    xrtValueRelease(member);
    assert(xuiWidgetSetUserValue(child, object) == XUI_OK);
    assert(xrtValueObjectRemove(object, key) && state.drops == 2);
    assert(xuiWidgetGetUserValue(child, &output) == XUI_OK && output == object);
    xrtValueRelease(output);
    xrtValueRelease(object);

    static const xvaluehandleops close_ops = {.Drop = refuse_during_close};
    xvalue* on_close = make_owned(&state, &close_ops);
    assert(xuiWidgetSetUserValue(child, on_close) == XUI_OK);
    xrtValueRelease(on_close);
    assert(xuiWidgetAddChild(root, child) == XUI_OK);
    assert(xuiWidgetDestroyChecked(root) == XUI_OK && state.drops == 3);
    assert(xuiDestroyChecked(context) == XUI_OK);
    xrtValueRelease(first);
    xrtValueRelease(second);
    puts("xui_widget_user_values_test passed");
    return 0;
}
