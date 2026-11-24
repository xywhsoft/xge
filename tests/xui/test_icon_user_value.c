#include "xui.h"
#include <assert.h>
#include <stdio.h>

static xui_icon create_icon(xui_icon_category category, const char* name, void* user)
{
    xui_icon_desc_t info;
    xuiIconDescDefault(&info);
    info.pUser = user;
    xui_icon icon = NULL;
    const xui_rect_t bounds = {0, 0, 16, 16};
    assert(xuiIconAddSvgPath(category, name, "M0 0 H16 V16 Z", bounds, NULL, &info, &icon) == XUI_OK);
    return icon;
}

typedef struct {
    xvalue* value;
    unsigned edges;
} value_count;

static bool count_value(xrtownershipref ref, void* raw)
{
    value_count* result = raw;
    if (ref.Data == result->value) {
        assert(ref.Ops == xrtValueOwnership(result->value).Ops);
        ++result->edges;
    }
    return true;
}

static unsigned edges(xui_context context, xvalue* value)
{
    value_count result = {value, 0};
    xrtownershipscope scope = {0};
    assert(xrtOwnershipFreezeTryBegin(&scope));
    assert(xuiContextTraceOwnership(context, count_value, &result));
    assert(xrtOwnershipScopeEnd(&scope));
    return result.edges;
}

static int wrong_thread(void* raw)
{
    xvalue* value = xrtValueNull();
    assert(xuiIconGetUserValue(raw, &value) == XUI_ERROR_BUSY && !value);
    assert(xuiIconSetUserValue(raw, NULL) == XUI_ERROR_BUSY);
    return 0;
}

static void invalidate(void* raw)
{
    ++*(unsigned*)raw;
}

typedef struct {
    xui_icon_reference_t* peer;
    unsigned drops;
} retained_peer;

static bool refuse_clone(void* raw, void** output, void* user)
{
    (void)raw;
    (void)user;
    *output = NULL;
    return false;
}

static void drop_peer(void* raw, void* user)
{
    (void)user;
    retained_peer* state = raw;
    bool consumed = false;
    assert(xuiIconReferenceReleaseOwnerChecked(state->peer, &consumed) == XUI_OK && consumed);
    ++state->drops;
}

int main(void)
{
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    xui_proxy proxy = NULL;
    xui_context context = NULL;
    const xui_context_desc_t desc = {.iSize = sizeof(desc), .iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED};
    assert(xuiProxyXgeCreate(&proxy) == XUI_OK && xuiCreateOwnedEx(proxy, &desc, &context) == XUI_OK);
    xui_icon_category category = NULL;
    assert(xuiIconCategoryCreate(context, "first", NULL, &category) == XUI_OK);
    int native_user = 1;
    xui_icon target = create_icon(category, "target", &native_user);
    xui_icon alias = NULL;
    assert(xuiIconAddAlias(category, "alias", target, NULL, &alias) == XUI_OK);
    xvalue* data = xrtValueObject();
    xvalue* scalar = xrtValueInt(19);
    assert(data && scalar);
    xvalue* out = scalar;
    assert(xuiIconGetUserValue(target, &out) == XUI_OK && !out);
    assert(xuiIconSetUserValue(target, data) == XUI_OK && xuiIconSetUserValue(target, data) == XUI_OK);
    assert(xuiIconGetUser(target) == &native_user && edges(context, data) == 1);
    assert(xuiIconGetUserValue(alias, &out) == XUI_OK && !out);

    assert(xrtMemDebugFailAfter(0));
    assert(xuiIconSetUserValue(alias, scalar) == XUI_OK);
    assert(xuiIconGetUserValue(alias, &out) == XUI_OK && out == scalar);
    xrtValueRelease(out);
    assert(xuiIconSetUserValue(alias, xrtValueNull()) == XUI_OK);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();

    xthread* thread = xrtThreadCreate(wrong_thread, target, 0);
    assert(thread && xrtThreadWaitFor(thread, 5000) == XWAIT_OK);
    xrtThreadDestroy(thread);
    xvalueviewv1 view = {0};
    assert(xrtValueViewBeginV1(data, &view));
    assert(xuiContextOwnershipBusy(context));
    assert(xuiIconSetUserValue(target, scalar) == XUI_ERROR_BUSY);
    bool accepted = false;
    assert(xuiIconRemoveOwnerChecked(target, &accepted) == XUI_OK && accepted);
    assert(xuiIconRemoveOwnerChecked(alias, &accepted) == XUI_ERROR_BUSY && !accepted);
    assert(xuiIconFind(category, "alias") == alias);
    assert(xrtValueViewEndV1(&view));
    assert(edges(context, data) == 1);
    assert(xuiIconRemoveOwnerChecked(alias, &accepted) == XUI_OK && accepted);
    assert(edges(context, data) == 0);
    xrtValueRelease(data);

    xui_icon retained = create_icon(category, "retained", NULL);
    data = xrtValueObject();
    assert(data && xuiIconSetUserValue(retained, data) == XUI_OK);
    xui_icon_reference_t* reference = NULL;
    assert(xuiIconRetainReference(retained, &reference) == XUI_OK);
    assert(xuiIconRemoveOwnerChecked(retained, &accepted) == XUI_OK && accepted);
    assert(xrtValueViewBeginV1(data, &view));
    bool consumed = true;
    assert(xuiIconReferenceReleaseOwnerChecked(reference, &consumed) == XUI_ERROR_BUSY && !consumed);
    assert(xuiIconReferenceIsLive(reference));
    assert(xrtValueViewEndV1(&view));
    assert(xuiIconReferenceReleaseOwnerChecked(reference, &consumed) == XUI_OK && consumed);
    xrtValueRelease(data);

    xui_icon pending = create_icon(category, "pending", NULL);
    data = xrtValueObject();
    xvalue* weak = xrtValueWeakRef(data);
    assert(data && weak && xuiIconSetUserValue(pending, data) == XUI_OK);
    unsigned invalidated = 0;
    xui_lifetime_link_t link = {0};
    assert(xuiIconLifetimeWatch(pending, &link, invalidate, &invalidated) == XUI_OK);
    assert(xrtValueViewBeginV1(data, &view));
    assert(xuiIconCategoryRemoveOwnerChecked(category, &accepted) == XUI_OK && accepted);
    assert(!invalidated && xuiContextOwnershipBusy(context));
    assert(xrtValueViewEndV1(&view));
    assert(edges(context, data) == 1);
    xrtValueRelease(data);
    xui_call_scope_t scope = {0};
    assert(xuiCallScopeBegin(context, &scope) == XUI_OK && xuiCallScopeEnd(&scope) == XUI_OK);
    assert(invalidated == 1 && xrtValueWeakRefExpired(weak));
    xrtValueRelease(weak);

    assert(xuiIconCategoryCreate(context, "last", NULL, &category) == XUI_OK);
    retained_peer state = {0};
    xui_icon peer = create_icon(category, "peer", NULL);
    assert(xuiIconRetainReference(peer, &state.peer) == XUI_OK);
    assert(xuiIconRemoveOwnerChecked(peer, &accepted) == XUI_OK && accepted);
    xui_icon holder = create_icon(category, "holder", NULL);
    const xvaluehandleops ops = {refuse_clone, drop_peer, NULL, NULL};
    void* raw = &state;
    data = xrtValueHandleTake(&raw, &ops, NULL);
    assert(data && !raw && xuiIconSetUserValue(holder, data) == XUI_OK);
    xrtValueRelease(data);

    xui_icon last = create_icon(category, "last", NULL);
    data = xrtValueObject();
    weak = xrtValueWeakRef(data);
    assert(data && weak && xuiIconSetUserValue(last, data) == XUI_OK);
    assert(xrtValueViewBeginV1(data, &view));
    assert(xuiDestroyOwnerChecked(context, &consumed) == XUI_STATUS_DEFERRED && !consumed);
    assert(state.drops == 1 && !xrtValueWeakRefExpired(weak));
    assert(xuiIconGetUserValue(last, &out) == XUI_ERROR_INVALID_STATE && !out);
    assert(xrtValueViewEndV1(&view));
    xrtValueRelease(data);
    assert(xuiDestroyOwnerChecked(context, &consumed) == XUI_OK && consumed);
    assert(xrtValueWeakRefExpired(weak));
    xrtValueRelease(weak);
    xrtValueRelease(scalar);
    assert(xuiProxyReleaseChecked(proxy) == XUI_OK);

    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount == after.LiveCount && before.LiveBytes == after.LiveBytes);
    puts("XUI_ICON_USER_VALUE_NATIVE_OK");
    return 0;
}
