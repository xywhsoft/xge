#include "src/xui/core/xui_internal.h"
#include <assert.h>
#include <stdio.h>
#include <limits.h>

xui_widget xuiInternalPopupPointerPassthroughOwner(xui_context context,
    xui_widget hit, float x, float y)
{ (void)context; (void)x; (void)y; return hit; }
int xuiInternalPopupPrepareOwnerPress(xui_context context,
    xui_widget target, float x, float y)
{ (void)context; (void)target; (void)x; (void)y; return XUI_OK; }

enum { CLEAR, REWATCH_SELF, DESTROY_CONTEXT, FREE_PAIR };
typedef struct observer {
    xui_lifetime_link_t link;
    void* value;
    xui_widget widget;
    xui_context context;
    struct observer* peer;
    unsigned calls, action;
} observer;
static unsigned notifications, destructors;

static void invalidated(void* user)
{
    observer* state = user;
    assert(!state->link.ppPrevious && !state->link.pNext && !state->link.onInvalidate && !state->link.pUser);
    state->value = NULL; ++state->calls; ++notifications;
    if (state->action == REWATCH_SELF) {
        assert(xuiWidgetLifetimeWatch(state->widget, &state->link, invalidated, state) == XUI_ERROR_INVALID_STATE);
        xuiWidgetDestroy(state->widget);
    } else if (state->action == DESTROY_CONTEXT) {
        xuiDestroy(state->context);
    } else if (state->action == FREE_PAIR) {
        observer* peer = state->peer;
        xuiLifetimeUnwatch(&peer->link);
        xrtFree(peer); xrtFree(state);
    }
}

static observer watch_widget(xui_context context, xui_widget widget)
{
    observer result = {0}; result.context = context; result.widget = widget; result.value = widget;
    /* Returning an UNATTACHED zeroed link by value is legal. Attach only after
     * the result has reached its final address. */
    return result;
}

static void attach_and_detach(void)
{
    xui_context context = NULL; xui_widget first = NULL, second = NULL;
    assert(xuiCreate(&context) == XUI_OK);
    assert(xuiWidgetCreate(context, &first) == XUI_OK && xuiWidgetCreate(context, &second) == XUI_OK);
    observer a = watch_widget(context, first), b = watch_widget(context, first);
    assert(xuiContextLifetimeWatch(NULL, &a.link, invalidated, &a) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiWidgetLifetimeWatch(NULL, &a.link, invalidated, &a) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiWidgetLifetimeWatch(first, NULL, invalidated, &a) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiWidgetLifetimeWatch(first, &a.link, NULL, &a) == XUI_ERROR_INVALID_ARGUMENT);
    xmemdebugsnapshot before, after; xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiWidgetLifetimeWatch(first, &a.link, invalidated, &a) == XUI_OK);
    assert(xuiWidgetLifetimeWatch(first, &b.link, invalidated, &b) == XUI_OK);
    assert(xuiWidgetLifetimeWatch(second, &a.link, invalidated, &a) == XUI_ERROR_INVALID_STATE);
    assert(xuiContextLifetimeWatch(context, &a.link, invalidated, &a) == XUI_ERROR_INVALID_STATE);
    xuiLifetimeUnwatch(&a.link); xuiLifetimeUnwatch(&a.link); xuiLifetimeUnwatch(NULL);
    assert(xuiWidgetLifetimeWatch(second, &a.link, invalidated, &a) == XUI_OK);
    assert(!xrtMemDebugFailTriggered()); xrtMemDebugFailClear();
    xrtMemDebugSnapshot(&after);
    assert(before.AllocCount == after.AllocCount && before.FreeCount == after.FreeCount && before.ReallocCount == after.ReallocCount);
    xuiWidgetDestroy(first); assert(!b.value && b.calls == 1 && a.calls == 0);
    xuiWidgetDestroy(second); assert(!a.value && a.calls == 1);
    xuiDestroy(context);
    xuiLifetimeUnwatch(&a.link); xuiLifetimeUnwatch(&b.link);
}

static void pending_scope(void)
{
    xui_context context = NULL; xui_widget parent = NULL, child = NULL, orphan = NULL;
    assert(xuiCreate(&context) == XUI_OK);
    assert(xuiWidgetCreate(context, &parent) == XUI_OK && xuiWidgetCreate(context, &child) == XUI_OK);
    assert(xuiWidgetCreate(context, &orphan) == XUI_OK && xuiWidgetAddChild(parent, child) == XUI_OK);
    observer a = watch_widget(context, parent), b = watch_widget(context, child), c = watch_widget(context, orphan);
    observer owner = {0}; owner.context = context; owner.value = context;
    assert(xuiContextLifetimeWatch(context, &owner.link, invalidated, &owner) == XUI_OK);
    assert(xuiWidgetLifetimeWatch(parent, &a.link, invalidated, &a) == XUI_OK);
    assert(xuiWidgetLifetimeWatch(child, &b.link, invalidated, &b) == XUI_OK);
    assert(xuiWidgetLifetimeWatch(orphan, &c.link, invalidated, &c) == XUI_OK);
    xuiInternalOperationEnter(context);
    xuiDestroy(context); xuiDestroy(context);
    assert(!owner.value && !a.value && !b.value && !c.value);
    assert(owner.calls == 1 && a.calls == 1 && b.calls == 1 && c.calls == 1);
    assert(context->bDestroyPending && context->pAllWidgets);
    assert(xuiWidgetLifetimeWatch(orphan, &c.link, invalidated, &c) == XUI_ERROR_INVALID_STATE);
    assert(xuiContextLifetimeWatch(context, &owner.link, invalidated, &owner) == XUI_ERROR_INVALID_STATE);
    xuiInternalOperationLeave(context);
    xuiLifetimeUnwatch(&owner.link); xuiLifetimeUnwatch(&c.link);
}

static void scope_destroy(xui_widget widget, void* instance, void* user)
{ (void)widget; (void)instance; (void)user; ++destructors; }

static void public_call_scopes(void)
{
    xui_context context = NULL; xui_widget_type type = NULL; xui_widget a = NULL, b = NULL;
    xui_call_scope_t outer = {0}, inner = {0}, rejected = {0};
    assert(xuiCallScopeBegin(NULL, &outer) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCallScopeEnd(NULL) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCallScopeEnd(&outer) == XUI_OK);
    assert(xuiCreate(&context) == XUI_OK);
    context->iOperationDepth = INT_MAX;
    assert(xuiCallScopeBegin(context, &rejected) == XUI_ERROR_INVALID_STATE);
    assert(!rejected.pSelf && !rejected.pContext); context->iOperationDepth = 0;
    xmemdebugsnapshot before, after; xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiCallScopeBegin(context, &outer) == XUI_OK);
    assert(xuiCallScopeEnd(&outer) == XUI_OK);
    assert(xrtMemDebugFailAfter(-1)); xrtMemDebugSnapshot(&after);
    assert(before.AllocCount == after.AllocCount && before.FreeCount == after.FreeCount && before.ReallocCount == after.ReallocCount);
    xui_widget_type_desc_t description = {0};
    description.iSize = sizeof(description); description.sName = "test.call-scope";
    description.pParent = xuiWidgetGetBaseType(); description.onDestroy = scope_destroy;
    assert(xuiWidgetRegisterType(context, &type, &description) == XUI_OK);
    assert(xuiWidgetCreateTyped(context, type, &a, NULL) == XUI_OK);
    assert(xuiWidgetCreateTyped(context, type, &b, NULL) == XUI_OK);
    observer watched = watch_widget(context, a);
    assert(xuiWidgetLifetimeWatch(a, &watched.link, invalidated, &watched) == XUI_OK);
    assert(xuiCallScopeBegin(context, &outer) == XUI_OK);
    assert(xuiCallScopeBegin(context, &outer) == XUI_ERROR_INVALID_STATE);
    assert(xuiCallScopeBegin(context, &inner) == XUI_OK);
    xui_call_scope_t copied = outer;
    assert(xuiCallScopeEnd(&copied) == XUI_ERROR_INVALID_STATE);
    unsigned before_destroy = destructors;
    xuiDestroy(context);
    assert(!watched.value && watched.calls == 1 && destructors == before_destroy);
    assert(xuiCallScopeBegin(context, &rejected) == XUI_ERROR_INVALID_STATE);
    assert(xuiCallScopeEnd(&outer) == XUI_OK && destructors == before_destroy);
    assert(xuiCallScopeEnd(&inner) == XUI_OK && destructors == before_destroy+2);
    assert(!outer.pContext && !outer.pSelf && !inner.pContext && !inner.pSelf);
    assert(xuiCallScopeEnd(&inner) == XUI_OK);
    xuiLifetimeUnwatch(&watched.link);
}

static void recursive_close(void)
{
    xui_context context = NULL; xui_widget parent = NULL, child = NULL;
    assert(xuiCreate(&context) == XUI_OK);
    assert(xuiWidgetCreate(context, &parent) == XUI_OK && xuiWidgetCreate(context, &child) == XUI_OK);
    assert(xuiWidgetAddChild(parent, child) == XUI_OK);
    observer a = watch_widget(context, parent), b = watch_widget(context, child);
    a.action = REWATCH_SELF; b.action = DESTROY_CONTEXT;
    assert(xuiWidgetLifetimeWatch(parent, &a.link, invalidated, &a) == XUI_OK);
    assert(xuiWidgetLifetimeWatch(child, &b.link, invalidated, &b) == XUI_OK);
    xuiWidgetDestroy(parent);
    assert(a.calls == 1 && b.calls == 1 && !a.value && !b.value);
    xuiLifetimeUnwatch(&a.link); xuiLifetimeUnwatch(&b.link);
}

static void detached_storage_can_disappear(void)
{
  for (unsigned watch_context = 0; watch_context < 2; ++watch_context) {
    xui_context context = NULL; xui_widget widget = NULL;
    assert(xuiCreate(&context) == XUI_OK && xuiWidgetCreate(context, &widget) == XUI_OK);
    observer *a = xrtCalloc(1, sizeof(*a)), *b = xrtCalloc(1, sizeof(*b));
    assert(a && b); a->peer = b; b->peer = a; a->action = b->action = FREE_PAIR;
    if (watch_context) {
        assert(xuiContextLifetimeWatch(context, &a->link, invalidated, a) == XUI_OK);
        assert(xuiContextLifetimeWatch(context, &b->link, invalidated, b) == XUI_OK);
    } else {
        assert(xuiWidgetLifetimeWatch(widget, &a->link, invalidated, a) == XUI_OK);
        assert(xuiWidgetLifetimeWatch(widget, &b->link, invalidated, b) == XUI_OK);
    }
    unsigned before = notifications;
    if (watch_context) xuiDestroy(context); else xuiWidgetDestroy(widget);
    assert(notifications == before + 1);
    if (!watch_context) xuiDestroy(context);
  }
}

typedef struct init_observers { observer widget, context; unsigned fail; } init_observers;
static int watched_init(xui_widget widget, void* data, const void* create, void* user)
{
    (void)data; (void)create;
    init_observers* state = user;
    state->widget = watch_widget(state->context.context, widget);
    assert(xuiWidgetLifetimeWatch(widget, &state->widget.link, invalidated, &state->widget) == XUI_OK);
    xuiDestroy(state->context.context);
    assert(!state->widget.value && !state->context.value);
    return state->fail ? XUI_ERROR : XUI_OK;
}
static void watched_destroy(xui_widget widget, void* data, void* user)
{
    (void)widget; (void)data;
    init_observers* state = user;
    assert(!state->widget.value && !state->context.value); ++destructors;
}
static void init_shutdown(void)
{
    for (unsigned fail = 0; fail < 2; ++fail) {
        xui_context context = NULL; xui_widget_type type = NULL; xui_widget widget = NULL;
        assert(xuiCreate(&context) == XUI_OK);
        init_observers state = {0}; state.fail = fail;
        state.context.context = context; state.context.value = context;
        assert(xuiContextLifetimeWatch(context, &state.context.link, invalidated, &state.context) == XUI_OK);
        xui_widget_type_desc_t description = {0};
        description.iSize = sizeof(description); description.sName = "test.watched-init";
        description.pParent = xuiWidgetGetBaseType(); description.pUser = &state;
        description.onInit = watched_init; description.onDestroy = watched_destroy;
        assert(xuiWidgetRegisterType(context, &type, &description) == XUI_OK);
        unsigned before = destructors;
        assert(xuiWidgetCreateTyped(context, type, &widget, NULL) != XUI_OK && !widget);
        assert(state.widget.calls == 1 && state.context.calls == 1);
        assert(destructors == before + !fail);
        xuiLifetimeUnwatch(&state.widget.link); xuiLifetimeUnwatch(&state.context.link);
    }
}

int main(void)
{
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before, after; xrtMemDebugSnapshot(&before);
    attach_and_detach(); pending_scope(); public_call_scopes(); recursive_close();
    detached_storage_can_disappear(); init_shutdown();
    xrtClearError(); xrtMemDebugSnapshot(&after);
    assert(before.LiveCount == after.LiveCount && before.LiveBytes == after.LiveBytes);
    assert(before.InvalidFreeCount == after.InvalidFreeCount && before.DoubleFreeCount == after.DoubleFreeCount);
    puts("XUI weak lifetime: logical invalidation, reentry, zero-allocation attach/detach and complete cleanup passed");
    return 0;
}
