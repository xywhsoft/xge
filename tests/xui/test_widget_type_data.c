#include "xui.h"
#include <assert.h>
#include <stdio.h>

typedef struct {
    xvalue* wanted;
    unsigned count;
} trace_state;

static bool count_value(xrtownershipref edge, void* raw)
{
    trace_state* state = raw;
    if (edge.Data == state->wanted) {
        assert(edge.Ops == xrtValueOwnership(state->wanted).Ops);
        ++state->count;
    }
    return true;
}

static unsigned count_edges(xui_context context, xvalue* value)
{
    xrtownershipscope frozen = {0};
    trace_state state = {value, 0};
    assert(xrtOwnershipFreezeTryBegin(&frozen));
    assert(xuiContextTraceOwnership(context, count_value, &state));
    assert(xrtOwnershipScopeEnd(&frozen));
    return state.count;
}

static int check_other_thread(void* raw)
{
    xui_widget_type type = raw;
    xvalue* value = xrtValueNull();
    assert(xuiWidgetTypeGetUserValue(type, &value) == XUI_ERROR_BUSY && !value);
    assert(xuiWidgetTypeSetUserValue(type, NULL) == XUI_ERROR_BUSY);
    assert(xuiWidgetUnregisterType(type) == XUI_ERROR_BUSY);
    return 0;
}

int main(void)
{
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);

    xui_proxy proxy = NULL;
    xui_context context = NULL;
    const xui_context_desc_t options = {.iSize = sizeof(options), .iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED};
    assert(xuiProxyXgeCreate(&proxy) == XUI_OK);
    assert(xuiCreateOwnedEx(proxy, &options, &context) == XUI_OK);
    xui_widget_type base = xuiWidgetGetBaseType();
    xvalue* output = xrtValueNull();
    assert(xuiWidgetTypeGetUserValue(base, &output) == XUI_OK && !output);
    assert(xuiWidgetTypeSetUserValue(base, NULL) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiWidgetTypeGetUserValue(NULL, &output) == XUI_ERROR_INVALID_ARGUMENT && !output);
    assert(xuiWidgetTypeGetUserValue(base, NULL) == XUI_ERROR_INVALID_ARGUMENT);

    int native_user = 9;
    xui_widget_type_desc_t desc = {.iSize = sizeof(desc), .sName = "type-data",
        .iTypeDataSize = sizeof(int), .pUser = &native_user};
    xui_widget_type type = NULL;
    assert(xuiWidgetRegisterType(context, &type, &desc) == XUI_OK);
    assert(xuiWidgetTypeGetUserData(type) == &native_user);
    assert(xuiWidgetTypeGetUserValue(type, &output) == XUI_OK && !output);

    xvalue* value = xrtValueObject();
    xvalue* scalar = xrtValueInt(42);
    assert(value && scalar && xrtValueObjectSet(value, XRT_STR_LITERAL("number"), scalar));
    assert(xuiWidgetTypeSetUserValue(type, value) == XUI_OK);
    assert(xuiWidgetTypeSetUserValue(type, value) == XUI_OK);
    assert(count_edges(context, value) == 1);
    assert(xuiWidgetTypeGetUserValue(type, &output) == XUI_OK && output == value);
    assert(xuiWidgetTypeGetUserData(type) == &native_user);
    xrtValueRelease(output);

    xui_widget widget = NULL;
    assert(xuiWidgetCreateTyped(context, type, &widget, NULL) == XUI_OK);
    int* native_data = xuiWidgetGetTypeData(widget);
    assert(native_data);
    *native_data = 12;
    assert(xuiWidgetSetUserValue(widget, scalar) == XUI_OK);
    assert(count_edges(context, value) == 1 && count_edges(context, scalar) == 1);
    assert(*native_data == 12);
    assert(xuiWidgetUnregisterType(type) == XUI_ERROR_ALREADY_INITIALIZED);
    assert(xuiWidgetDestroyChecked(widget) == XUI_OK);

    xvalueviewv1 read = {0};
    assert(xrtValueViewBeginV1(value, &read));
    assert(xuiContextOwnershipBusy(context));
    assert(xuiWidgetTypeSetUserValue(type, scalar) == XUI_ERROR_BUSY);
    assert(xuiWidgetUnregisterType(type) == XUI_ERROR_BUSY);
    assert(xuiWidgetFindType(context, desc.sName) == type);
    assert(xrtValueViewEndV1(&read));
    assert(!xuiContextOwnershipBusy(context));

    xthread* thread = xrtThreadCreate(check_other_thread, type, 0);
    assert(thread && xrtThreadWaitFor(thread, 5000) == XWAIT_OK);
    xrtThreadDestroy(thread);

    assert(xrtMemDebugFailAfter(0));
    assert(xuiWidgetTypeSetUserValue(type, scalar) == XUI_OK);
    assert(xuiWidgetTypeGetUserValue(type, &output) == XUI_OK && output == scalar);
    assert(xuiWidgetTypeSetUserValue(type, xrtValueNull()) == XUI_OK);
    assert(count_edges(context, scalar) == 0);
    assert(xrtMemDebugFailAfter(-1));
    xrtValueRelease(output);

    xvalue* weak = xrtValueWeakRef(value);
    assert(weak);
    assert(xuiWidgetTypeSetUserValue(type, value) == XUI_OK);
    xrtValueRelease(value);
    assert(xuiWidgetUnregisterType(type) == XUI_OK && xrtValueWeakRefExpired(weak));
    xrtValueRelease(weak);
    assert(!xuiWidgetFindType(context, desc.sName));

    assert(xuiWidgetRegisterType(context, &type, &desc) == XUI_OK);
    value = xrtValueArray();
    weak = xrtValueWeakRef(value);
    assert(value && weak);
    assert(xuiWidgetTypeSetUserValue(type, value) == XUI_OK);
    assert(xrtValueViewBeginV1(value, &read));
    bool consumed = true;
    assert(xuiDestroyOwnerChecked(context, &consumed) == XUI_STATUS_DEFERRED && !consumed);
    assert(!xrtValueWeakRefExpired(weak));
    assert(xrtValueViewEndV1(&read));
    xrtValueRelease(value);
    assert(xuiDestroyOwnerChecked(context, &consumed) == XUI_OK && consumed);
    assert(xrtValueWeakRefExpired(weak));
    xrtValueRelease(weak);
    xrtValueRelease(scalar);
    assert(xuiProxyReleaseChecked(proxy) == XUI_OK);
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount == after.LiveCount && before.LiveBytes == after.LiveBytes);
    puts("XUI_WIDGET_TYPE_DATA_NATIVE_OK");
    return 0;
}
