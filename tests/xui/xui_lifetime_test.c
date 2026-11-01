#include "src/xui/core/xui_internal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* No window, renderer, desktop input, or network is used. */
xui_widget xuiInternalPopupPointerPassthroughOwner(xui_context context,
    xui_widget hit, float x, float y)
{ (void)context; (void)x; (void)y; return hit; }
int xuiInternalPopupPrepareOwnerPress(xui_context context,
    xui_widget target, float x, float y)
{ (void)context; (void)target; (void)x; (void)y; return XUI_OK; }

static unsigned destroyed;
static void destroyed_widget(xui_widget widget, void* data, void* user)
{ (void)widget; (void)data; (void)user; ++destroyed; }

static xui_widget_type test_type(xui_context context)
{
    xui_widget_type type = NULL;
    xui_widget_type_desc_t description = {0};
    description.iSize = sizeof(description);
    description.sName = "test.lifetime";
    description.pParent = xuiWidgetGetBaseType();
    description.onDestroy = destroyed_widget;
    assert(xuiWidgetRegisterType(context, &type, &description) == XUI_OK);
    return type;
}

static void orphan_context(void)
{
    xui_context context = NULL;
    xui_widget first = NULL, second = NULL;
    assert(xuiCreate(&context) == XUI_OK);
    xui_widget_type type = test_type(context);
    assert(xuiWidgetCreateTyped(context, type, &first, NULL) == XUI_OK);
    assert(xuiWidgetCreateTyped(context, type, &second, NULL) == XUI_OK);
    assert(first && second && first != second);
    destroyed = 0;
    xuiDestroy(context);
    printf("unparented widgets destroyed: %u (expected 2)\n", destroyed);
    assert(destroyed == 2);
}

static void mixed_context(void)
{
    xui_context context = NULL;
    xui_widget widgets[5] = {0};
    assert(xuiCreate(&context) == XUI_OK);
    xui_widget_type type = test_type(context);
    for (unsigned i = 0; i < 5; ++i)
        assert(xuiWidgetCreateTyped(context, type, &widgets[i], NULL) == XUI_OK);
    assert(xuiSetRootWidget(context, widgets[0]) == XUI_OK);
    assert(xuiWidgetAddChild(widgets[0], widgets[1]) == XUI_OK);
    assert(xuiWidgetAddChild(widgets[2], widgets[3]) == XUI_OK);
    assert(xuiWidgetRemoveFromParent(widgets[3]) == XUI_OK);
    destroyed = 0;
    xuiWidgetDestroy(widgets[1]);
    assert(destroyed == 1);
    xuiDestroy(context);
    assert(destroyed == 5);
}

static int closing_init(xui_widget widget, void* data, const void* create, void* user)
{
    (void)data; (void)create; (void)user;
    xuiDestroy(xuiWidgetGetContext(widget));
    return XUI_OK;
}

static unsigned closing_creation_rejected;
static void reentrant_destroy(xui_widget widget, void* data, void* user)
{
    (void)widget; (void)data;
    xui_widget orphan = NULL;
    assert(xuiWidgetCreate((xui_context)user, &orphan) == XUI_ERROR_INVALID_STATE);
    assert(orphan == NULL);
    ++closing_creation_rejected;
}

static void creation_during_shutdown(void)
{
    xui_context context = NULL;
    xui_widget_type type = NULL;
    xui_widget widget = (xui_widget)(uintptr_t)1;
    assert(xuiCreate(&context) == XUI_OK);
    xui_widget_type_desc_t description = {0};
    description.iSize = sizeof(description);
    description.sName = "test.closing-creation";
    description.pParent = xuiWidgetGetBaseType();
    description.onInit = closing_init;
    description.onDestroy = reentrant_destroy;
    description.pUser = context;
    assert(xuiWidgetRegisterType(context, &type, &description) == XUI_OK);
    assert(xuiWidgetCreateTyped(context, type, &widget, NULL) != XUI_OK);
    assert(widget == NULL && closing_creation_rejected == 1);
}

typedef struct init_state {
    xui_context context;
    xui_widget_type leaf, child;
    unsigned mode, parents, leaves;
} init_state;

static void ready_parent_destroy(xui_widget widget, void* data, void* user)
{ (void)widget; (void)data; ++((init_state*)user)->parents; }
static void ready_leaf_destroy(xui_widget widget, void* data, void* user)
{ (void)widget; (void)data; ++((init_state*)user)->leaves; }
static int queued_init(xui_widget widget, void* data, const void* create, void* user)
{
    (void)data; (void)create;
    init_state* state = user;
    xui_widget child = NULL;
    /* A type cannot disappear while one of its init hooks is on the stack. */
    assert(xuiWidgetUnregisterType(state->leaf) == XUI_ERROR_ALREADY_INITIALIZED);
    assert(xuiWidgetCreateTyped(state->context, state->child, &child, NULL) == XUI_OK);
    assert(xuiWidgetAddChild(widget, child) == XUI_OK);
    if (state->mode == 2) xuiDestroy(state->context);
    else xuiWidgetDestroy(widget);
    return state->mode == 1 ? XUI_OK : XUI_ERROR;
}

static void initialization_reentry(void)
{
    for (unsigned mode = 0; mode < 3; ++mode) {
        init_state state = {0}; state.mode = mode;
        assert(xuiCreate(&state.context) == XUI_OK);
        state.child = test_type(state.context);
        xui_widget_type parent = NULL;
        xui_widget_type_desc_t description = {0};
        description.iSize = sizeof(description);
        description.sName = "test.ready-parent";
        description.pParent = xuiWidgetGetBaseType();
        description.pUser = &state;
        description.onDestroy = ready_parent_destroy;
        assert(xuiWidgetRegisterType(state.context, &parent, &description) == XUI_OK);
        description.sName = "test.queued-init";
        description.pParent = parent;
        description.onInit = queued_init;
        description.onDestroy = ready_leaf_destroy;
        assert(xuiWidgetRegisterType(state.context, &state.leaf, &description) == XUI_OK);
        xui_widget widget = (xui_widget)(uintptr_t)1;
        destroyed = 0;
        assert(xuiWidgetCreateTyped(state.context, state.leaf, &widget, NULL) != XUI_OK);
        assert(!widget && state.parents == 1 && state.leaves == (mode == 1) && destroyed == 1);
        if (mode != 2) {
            assert(!state.context->pAllWidgets && !state.context->pDeferredDestroyHead);
            assert(xuiWidgetUnregisterType(state.leaf) == XUI_OK);
            assert(xuiWidgetUnregisterType(parent) == XUI_OK);
            xuiDestroy(state.context);
        }
    }
}

#ifdef XRT_MODULE_MEMORY_DEBUG
static int oom_init(xui_widget widget, void* data, const void* create, void* user)
{
    (void)data; (void)create;
    init_state* state = user;
    ++state->mode;
    xui_widget child = NULL;
    int result = xuiWidgetCreateTyped(state->context, state->child, &child, NULL);
    if (result != XUI_OK) return result;
    result = xuiWidgetAddChild(widget, child);
    if (result != XUI_OK) xuiWidgetDestroy(child);
    return XUI_ERROR;
}
static void allocation_failures(void)
{
    unsigned prefixes = 0;
    for (uint64_t n = 0; n < 64; ++n) {
        xmemdebugsnapshot before, after;
        xrtMemDebugSnapshot(&before);
        init_state state = {0};
        assert(xuiCreate(&state.context) == XUI_OK);
        state.child = test_type(state.context);
        xui_widget_type parent = NULL;
        xui_widget_type_desc_t description = {0};
        description.iSize = sizeof(description);
        description.sName = "test.oom-parent";
        description.pParent = xuiWidgetGetBaseType();
        description.pUser = &state;
        description.onDestroy = ready_parent_destroy;
        assert(xuiWidgetRegisterType(state.context, &parent, &description) == XUI_OK);
        description.sName = "test.oom-init";
        description.pParent = parent;
        description.iTypeDataSize = 64;
        description.onInit = oom_init;
        description.onDestroy = ready_leaf_destroy;
        assert(xuiWidgetRegisterType(state.context, &state.leaf, &description) == XUI_OK);
        xui_widget widget = (xui_widget)(uintptr_t)1;
        assert(xrtMemDebugFailAfter(n));
        assert(xuiWidgetCreateTyped(state.context, state.leaf, &widget, NULL) != XUI_OK);
        int triggered = xrtMemDebugFailTriggered();
        xrtMemDebugFailClear(); xrtClearError();
        assert(!widget && !state.context->pAllWidgets && !state.context->pDeferredDestroyHead);
        assert(state.parents == state.mode && state.leaves == 0);
        assert(state.leaf->iWidgetCount == 0 && state.child->iWidgetCount == 0);
        xuiDestroy(state.context); xrtClearError();
        xrtMemDebugSnapshot(&after);
        assert(before.LiveCount == after.LiveCount && before.LiveBytes == after.LiveBytes);
        assert(before.InvalidFreeCount == after.InvalidFreeCount && before.DoubleFreeCount == after.DoubleFreeCount);
        if (!triggered) { assert(prefixes >= 2); printf("allocation rollback prefixes: %u\n", prefixes); return; }
        ++prefixes;
    }
    assert(!"OOM prefix sweep did not reach the non-injected path");
}
#endif

int main(void)
{
#ifdef XRT_MODULE_MEMORY_DEBUG
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before, after; xrtMemDebugSnapshot(&before);
#endif
    orphan_context();
    mixed_context();
    creation_during_shutdown();
    initialization_reentry();
#ifdef XRT_MODULE_MEMORY_DEBUG
    allocation_failures();
    xrtClearError(); xrtMemDebugSnapshot(&after);
    assert(before.LiveCount == after.LiveCount && before.LiveBytes == after.LiveBytes);
    assert(before.InvalidFreeCount == after.InvalidFreeCount && before.DoubleFreeCount == after.DoubleFreeCount);
#endif
    puts("XUI native lifetime tests passed");
    return 0;
}
