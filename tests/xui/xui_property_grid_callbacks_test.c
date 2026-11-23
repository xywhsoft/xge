#if XGE_XRT_HOST && !defined(XL6_XGE_STATIC) && defined(_WIN32)
#define XRT_USE_SHARED
#endif
#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#include "xui.h"
#include "xui_test_proxy.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#if XGE_XRT_HOST && !defined(XL6_XGE_STATIC)
#include "xge_host_xrt_contract.h"
static xge_host_xrt_proc resolve(void* user, const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if ( !strcmp(name, #symbol) ) return (xge_host_xrt_proc)&symbol;
#include "xge_host_xrt_slots.inc"
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif

typedef struct CallbackState {
    size_t references;
    unsigned calls;
    bool reject;
    bool statusOnly;
    void (*onHold)(struct CallbackState*);
    bool (*onFinalDrop)(struct CallbackState*);
    void* data;
} CallbackState;

static bool retain(const void* source)
{
    CallbackState* state = (CallbackState*)source;
    ++state->references;
    if ( state->onHold ) state->onHold(state);
    return true;
}

static bool release(const void* source)
{
    CallbackState* state = (CallbackState*)source;
    assert(state->references > 1);
    --state->references;
    if ( state->references == 1 && state->onFinalDrop ) return state->onFinalDrop(state);
    return true;
}

static bool count(const void* source, size_t* references)
{
    *references = ((const CallbackState*)source)->references;
    return true;
}

static bool trace(const void* source, xrtownershipvisitor visitor, void* user)
{
    (void)source;
    (void)visitor;
    (void)user;
    return true;
}

static xui_callback_owner_t owner(CallbackState* state)
{
    static const xrtownershipops ops = {count, trace};
    return (xui_callback_owner_t){{state, &ops}, retain, release};
}

static int validate(xui_widget widget, int property, const char* id, const char* value, int type, void* user)
{
    (void)widget;
    CallbackState* state = user;
    assert(state->references == 3 && value);
    assert((property == 0 && type == XUI_TABLE_CELL_TYPE_BOOL && !strcmp(id, "enabled")) ||
           (property == 2 && type == XUI_TABLE_CELL_TYPE_TEXT && !strcmp(id, "label")));
    ++state->calls;
    if ( state->reject ) {
        if ( !state->statusOnly ) xrtSetErrorKind(XERR_STATE);
        return XUI_ERROR_CALLBACK_FAILED;
    }
    return 1;
}

static int activate(xui_widget widget, int property, const char* id, xui_rect_t rect, void* user)
{
    CallbackState* state = user;
    assert(state->references == 3 && property == 1 && !strcmp(id, "path") && rect.fW > 0);
    ++state->calls;
    assert(xuiPropertyGridSetGlobalAction(widget, NULL, NULL) == XUI_OK);
    assert(state->references == 2 && !strcmp(id, "path"));
    return 1;
}

static int paint(xui_widget widget, int property, int column, const xui_table_view_cell_t* cell,
                 xui_draw_context_t* draw, xui_rect_t rect, int flags, void* user)
{
    (void)widget;
    (void)flags;
    CallbackState* state = user;
    assert(state->references == 3 && property >= 0 && column == 1 && cell && draw && rect.fW > 0);
    ++state->calls;
    return 0;
}

typedef struct MemberFixture {
    xui_context context;
    xui_widget grid;
    CallbackState action;
    CallbackState renderer;
    CallbackState replacement;
    CallbackState fallback;
} MemberFixture;

static void add_asset(xui_widget grid, const char* id)
{
    int category = xuiPropertyGridAddCategory(grid, "files", "Files", 1);
    const xui_property_desc_t desc = {.sId=id, .sValue="data.txt", .iType=XUI_TABLE_CELL_TYPE_FILE};
    assert(category == 0 && xuiPropertyGridAddProperty(grid, category, &desc) == 0);
}

static int member_action(xui_widget widget, int property, const char* id, xui_rect_t rect, void* user)
{
    (void)widget;
    CallbackState* state = user;
    assert(state->references == 3 && property >= 0 && id && rect.fW > 0);
    ++state->calls;
    return 1;
}

static bool visit_live_member(xrtownershipref ref, void* user)
{
    const MemberFixture* fixture = user;
    assert(ref.Data != &fixture->action && ref.Data != &fixture->renderer);
    return true;
}

static bool replace_from_drop(CallbackState* state)
{
    MemberFixture* fixture = state->data;
    state->onFinalDrop = NULL;
    assert(xuiPropertyGridGetPropertyCount(fixture->grid) == 0);
    assert(xuiContextOwnershipBusy(fixture->context));
    add_asset(fixture->grid, "replacement");
    xui_callback_owner_t newOwner = owner(&fixture->replacement);
    assert(xuiPropertyGridSetActionOwned(fixture->grid, 0, member_action, &fixture->replacement, &newOwner) == XUI_OK);
    return false;
}

static void replace_from_hold(CallbackState* state)
{
    MemberFixture* fixture = state->data;
    state->onHold = NULL;
    assert(xuiPropertyGridClear(fixture->grid) == XUI_OK);
    add_asset(fixture->grid, "replacement");
}

static void try_clear_while_opening(CallbackState* state)
{
    MemberFixture* fixture = state->data;
    state->onHold = NULL;
    assert(xuiPropertyGridClear(fixture->grid) == XUI_ERROR_INVALID_STATE);
    assert(xuiPropertyGridGetPropertyCount(fixture->grid) == 1);
}

static void member_callbacks(void)
{
    MemberFixture fixture = {.action={.references=1}, .renderer={.references=1},
                             .replacement={.references=1}, .fallback={.references=1}};
    fixture.action.data = &fixture;
    fixture.renderer.data = &fixture;
    xui_test_proxy_state_t backend;
    xuiTestProxyInit(&backend);
    xui_context_desc_t desc;
    xuiContextDescDefault(&desc);
    desc.iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED;
    assert(xuiCreateEx(&desc, &fixture.context) == XUI_OK);
    assert(xuiSetProxy(fixture.context, &backend.tProxy) == XUI_OK);
    assert(xuiInputViewport(fixture.context, 480, 320) == XUI_OK);
    assert(xuiPropertyGridCreate(fixture.context, &fixture.grid, NULL) == XUI_OK);
    assert(xuiWidgetSetRect(fixture.grid, (xui_rect_t){0, 0, 480, 320}) == XUI_OK);
    assert(xuiSetRootWidget(fixture.context, fixture.grid) == XUI_OK);
    add_asset(fixture.grid, "first");
    const xui_property_desc_t second = {.sId="second", .sValue="other.txt", .iType=XUI_TABLE_CELL_TYPE_FILE};
    assert(xuiPropertyGridAddProperty(fixture.grid, 0, &second) == 1);

    xui_callback_owner_t actionOwner = owner(&fixture.action);
    xui_callback_owner_t rendererOwner = owner(&fixture.renderer);
    xui_callback_owner_t fallbackOwner = owner(&fixture.fallback);
    assert(xuiPropertyGridSetGlobalActionOwned(fixture.grid, member_action, &fixture.fallback, &fallbackOwner) == XUI_OK);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiPropertyGridSetActionOwned(fixture.grid, 0, member_action, &fixture.action, &actionOwner) == XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();
    xrtClearError();
    assert(fixture.action.references == 1);
    assert(xuiPropertyGridSetActionOwned(fixture.grid, 0, member_action, &fixture.action, &actionOwner) == XUI_OK);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiPropertyGridSetActionOwned(fixture.grid, 0, member_action, &fixture.action, &actionOwner) == XUI_OK);
    assert(!xrtMemDebugFailTriggered() && fixture.action.references == 2);
    xrtMemDebugFailClear();
    assert(xuiPropertyGridSetRendererOwned(fixture.grid, 0, paint, &fixture.renderer, &rendererOwner) == XUI_OK);
    assert(xuiLayout(fixture.context) == XUI_OK);
    assert(xuiPropertyGridBeginEditChecked(fixture.grid, 0) == 1 && fixture.action.calls == 1 && fixture.fallback.calls == 0);
    assert(xuiPropertyGridBeginEditChecked(fixture.grid, 1) == 1 && fixture.fallback.calls == 1);
    xui_surface target = NULL;
    assert(xuiTestSurfaceCreate(&backend, &target, 480, 320, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    const xui_rect_i_t region = {0, 0, 480, 320};
    assert(xuiRender(fixture.context, target, &region, 1) == XUI_OK && fixture.renderer.calls > 0);

    fixture.renderer.onFinalDrop = replace_from_drop;
    assert(xuiPropertyGridClear(fixture.grid) == XUI_ERROR_CALLBACK_FAILED && xrtGetError() == NULL);
    assert(fixture.action.references == 1 && fixture.renderer.references == 1 && fixture.replacement.references == 2);
    assert(fixture.fallback.references == 2 && xuiPropertyGridGetPropertyCount(fixture.grid) == 1);
    xrtownershipscope freeze = {0};
    assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(fixture.context, visit_live_member, &fixture));
    assert(xrtOwnershipScopeEnd(&freeze));
    assert(xuiLayout(fixture.context) == XUI_OK);
    assert(xuiPropertyGridBeginEditChecked(fixture.grid, 0) == 1 && fixture.replacement.calls == 1);

    fixture.action.onHold = replace_from_hold;
    assert(xuiPropertyGridSetActionOwned(fixture.grid, 0, member_action, &fixture.action, &actionOwner) == XUI_ERROR_INVALID_STATE);
    assert(fixture.action.references == 1 && fixture.replacement.references == 1);
    assert(xuiLayout(fixture.context) == XUI_OK);
    assert(xuiPropertyGridBeginEditChecked(fixture.grid, 0) == 1 && fixture.fallback.calls == 2);

    assert(xuiPropertyGridSetActionOwned(fixture.grid, 0, member_action, &fixture.action, &actionOwner) == XUI_OK);
    fixture.action.onHold = try_clear_while_opening;
    assert(xuiPropertyGridBeginEditChecked(fixture.grid, 0) == 1 && fixture.action.calls == 2);
    assert(fixture.action.references == 2 && fixture.fallback.references == 2);
    assert(xuiPropertyGridSetRendererOwned(fixture.grid, 0, paint, &fixture.renderer, &rendererOwner) == XUI_OK);
    assert(xuiPropertyGridSetRenderer(fixture.grid, 0, NULL, NULL) == XUI_OK && fixture.renderer.references == 1);

    assert(xuiDestroyChecked(fixture.context) == XUI_OK);
    assert(fixture.action.references == 1 && fixture.fallback.references == 1);
    backend.tProxy.surfaceDestroy(&backend.tProxy, target);
    puts("property_grid_member_callbacks: passed");
}

int main(void)
{
#if XGE_XRT_HOST && !defined(XL6_XGE_STATIC)
    const xge_host_xrt_contract_t contract = XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract, resolve, NULL, NULL) == XGE_HOST_XRT_OK);
#endif
    xui_test_proxy_state_t backend;
    xuiTestProxyInit(&backend);
    xui_context_desc_t desc;
    xuiContextDescDefault(&desc);
    desc.iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED;
    xui_context context = NULL;
    xui_widget grid = NULL;
    xui_surface target = NULL;
    assert(xuiCreateEx(&desc, &context) == XUI_OK);
    assert(xuiSetProxy(context, &backend.tProxy) == XUI_OK);
    assert(xuiInputViewport(context, 480, 320) == XUI_OK);
    assert(xuiPropertyGridCreate(context, &grid, NULL) == XUI_OK);
    assert(xuiWidgetSetRect(grid, (xui_rect_t){0, 0, 480, 320}) == XUI_OK);
    assert(xuiSetRootWidget(context, grid) == XUI_OK);
    int category = xuiPropertyGridAddCategory(grid, "general", "General", 1);
    const xui_property_desc_t enabled = {.sId="enabled", .sValue="false", .iType=XUI_TABLE_CELL_TYPE_BOOL};
    const xui_property_desc_t path = {.sId="path", .sValue="data.txt", .iType=XUI_TABLE_CELL_TYPE_FILE};
    const xui_property_desc_t label = {.sId="label", .sValue="draft", .iType=XUI_TABLE_CELL_TYPE_TEXT};
    assert(category == 0 && xuiPropertyGridAddProperty(grid, category, &enabled) == 0);
    assert(xuiPropertyGridAddProperty(grid, category, &path) == 1);
    assert(xuiPropertyGridAddProperty(grid, category, &label) == 2);

    CallbackState validator = {.references=1};
    CallbackState action = {.references=1};
    CallbackState renderer = {.references=1};
    xui_callback_owner_t validateOwner = owner(&validator);
    xui_callback_owner_t actionOwner = owner(&action);
    xui_callback_owner_t renderOwner = owner(&renderer);

    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    assert(xrtMemDebugFailAfter(0));
    assert(xuiPropertyGridSetValidateOwned(grid, validate, &validator, &validateOwner) == XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();
    xrtClearError();
    assert(validator.references == 1);
    assert(xuiPropertyGridSetValidateOwned(grid, validate, &validator, &validateOwner) == XUI_OK);
    assert(xuiPropertyGridSetGlobalActionOwned(grid, activate, &action, &actionOwner) == XUI_OK);
    assert(xuiPropertyGridSetGlobalRendererOwned(grid, paint, &renderer, &renderOwner) == XUI_OK);
    assert(xuiLayout(context) == XUI_OK);
    assert(xuiPropertyGridBeginEdit(grid, 0) == 1 && validator.calls == 1 && validator.references == 2);
    assert(!strcmp(xuiPropertyGridGetValue(grid, 0), "true"));
    validator.reject = true;
    xui_call_scope_t receipt = {0};
    assert(xuiCallScopeBegin(context, &receipt) == XUI_OK);
    assert(xuiPropertyGridBeginEdit(grid, 0) == 0 && validator.calls == 2);
    assert(xuiCallScopeEnd(&receipt) == XUI_ERROR_CALLBACK_FAILED && xrtGetError() != NULL);
    xrtClearError();
    assert(xuiPropertyGridBeginEditChecked(grid, 0) == XUI_ERROR_CALLBACK_FAILED && validator.calls == 3);
    assert(xrtGetError() != NULL);
    xrtClearError();
    assert(!strcmp(xuiPropertyGridGetValue(grid, 0), "true"));
    validator.statusOnly = true;
    assert(xuiPropertyGridBeginEdit(grid, 0) == 0 && validator.calls == 4);
    assert(xrtGetError() == NULL);
    assert(xuiPropertyGridBeginEditChecked(grid, 0) == XUI_ERROR_CALLBACK_FAILED && validator.calls == 5);
    assert(xrtGetError() == NULL);
    validator.statusOnly = false;
    assert(xuiPropertyGridBeginEditChecked(grid, 2) == 1);
    assert(xuiPropertyGridEndEditChecked(grid, 1) == XUI_ERROR_CALLBACK_FAILED && validator.calls == 6);
    assert(xrtGetError() != NULL);
    xrtClearError();
    assert(xuiPropertyGridEndEditChecked(grid, 0) == 1);
    assert(xuiPropertyGridBeginEdit(grid, 1) == 1 && action.calls == 1 && action.references == 1);

    assert(xuiTestSurfaceCreate(&backend, &target, 480, 320, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    const xui_rect_i_t region = {0, 0, 480, 320};
    assert(xuiRender(context, target, &region, 1) == XUI_OK && renderer.calls > 0 && renderer.references == 2);
    assert(xuiPropertyGridClear(grid) == XUI_OK && renderer.references == 2 && validator.references == 2);
    assert(xuiDestroyChecked(context) == XUI_OK);
    assert(validator.references == 1 && renderer.references == 1);
    backend.tProxy.surfaceDestroy(&backend.tProxy, target);
    puts("property_grid_callbacks: owned validation, action and rendering passed");
    member_callbacks();
    return 0;
}
