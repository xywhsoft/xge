#include "xui.h"
#include <assert.h>
#include <stdio.h>

typedef struct {
    size_t references;
    xui_widget target;
    xui_widget_callback_snapshot_t* active;
    bool failHold;
    bool failDrop;
} callback_state;

static bool hold(const void* raw)
{
    callback_state* state = (callback_state*)raw;
    if (state->active) {
        bool consumed = true;
        assert(xuiWidgetCallbackSnapshotReleaseOwnerChecked(state->active, &consumed) == XUI_ERROR_BUSY);
        assert(!consumed);
        assert(xuiWidgetCallbackSnapshotInstall(state->active, state->target) == XUI_ERROR_BUSY);
    }
    ++state->references;
    if (state->failHold) xrtSetErrorKind(XERR_IO);
    return true;
}

static bool drop(const void* raw)
{
    callback_state* state = (callback_state*)raw;
    assert(state->references > 1);
    --state->references;
    if (state->failDrop) xrtSetErrorKind(XERR_IO);
    return !state->failDrop;
}

static bool count(const void* raw, size_t* output)
{
    *output = ((const callback_state*)raw)->references;
    return true;
}

static bool trace(const void* raw, xrtownershipvisitor visit, void* user)
{
    (void)raw; (void)visit; (void)user;
    return true;
}

static bool same_owner(xrtownershipref edge, void* user)
{
    assert(edge.Data == user && edge.Ops && edge.Ops->Count == count);
    return true;
}

static int measure(xui_widget widget, xui_vec2_t constraint, xui_vec2_t* size, void* user)
{
    (void)widget; (void)constraint;
    assert(user);
    *size = (xui_vec2_t){12, 8};
    return XUI_OK;
}

static int prepare(xui_widget widget, void* user)
{ (void)widget; assert(user); return XUI_OK; }

static int layout(xui_widget widget, xui_rect_t rect, void* user)
{ (void)widget; (void)rect; assert(user); return XUI_OK; }

static xui_rect_t ime(xui_widget widget, void* user)
{ (void)widget; assert(user); return (xui_rect_t){0, 0, 1, 2}; }

static int cursor(xui_widget widget, int x, int y, void* user)
{ (void)widget; (void)x; (void)y; assert(user); return 0; }

static int render(xui_widget widget, xui_draw_context draw, uint32_t state, void* user)
{ (void)widget; (void)draw; (void)state; assert(user); return XUI_OK; }

static int style(xui_code_document_t* document, const xui_code_token_t* token, int* id, void* user)
{ (void)document; (void)token; (void)id; assert(user); return XUI_OK; }

static int wrong_thread(void* raw)
{
    callback_state* state = raw;
    bool consumed = true;
    assert(xuiWidgetCallbackSnapshotOwnershipBusy(state->active));
    assert(xuiWidgetCallbackSnapshotInstall(state->active, state->target) == XUI_ERROR_BUSY);
    assert(xuiWidgetCallbackSnapshotReleaseOwnerChecked(state->active, &consumed) == XUI_ERROR_BUSY);
    assert(!consumed);
    xui_widget_callback_snapshot_t* output = NULL;
    assert(xuiWidgetCaptureContentMeasureCallback(state->target, &output) == XUI_ERROR_BUSY);
    assert(!output);
    return 0;
}

#define TRANSFER(setter, capture, getter, callback_type, function, from, to) do { \
    xui_widget_callback_snapshot_t* snapshot = NULL; \
    assert(setter(from, function, &state, &owner) == XUI_OK && state.references == 2); \
    assert(capture(from, &snapshot) == XUI_OK && snapshot && state.references == 3); \
    assert(setter(from, NULL, NULL, NULL) == XUI_OK && state.references == 2); \
    assert(xuiWidgetCallbackSnapshotInstall(snapshot, to) == XUI_OK && state.references == 3); \
    assert(xuiWidgetCallbackSnapshotRelease(snapshot) == XUI_OK && state.references == 2); \
    callback_type callback = NULL; \
    void* user = NULL; \
    assert(getter(to, &callback, &user) == XUI_OK && callback == function && user == &state); \
    assert(setter(to, NULL, NULL, NULL) == XUI_OK && state.references == 1); \
} while (0)

int main(void)
{
    assert(xrtMemDebugEnable(true));
    xui_proxy proxy = NULL;
    xui_context context = NULL;
    const xui_context_desc_t desc = {.iSize = sizeof(desc), .iSystemFontPolicy = XUI_SYSTEM_FONT_DISABLED};
    assert(xuiProxyXgeCreate(&proxy) == XUI_OK && xuiCreateOwnedEx(proxy, &desc, &context) == XUI_OK);
    xui_widget from = NULL, to = NULL, editor = NULL, secondEditor = NULL;
    assert(xuiWidgetCreate(context, &from) == XUI_OK && xuiWidgetCreate(context, &to) == XUI_OK);
    assert(xuiCodeEditCreate(context, &editor, NULL) == XUI_OK);
    assert(xuiCodeEditCreate(context, &secondEditor, NULL) == XUI_OK);
    callback_state state = {.references = 1, .target = to};
    const xrtownershipops ops = {count, trace};
    const xui_callback_owner_t owner = {{&state, &ops}, hold, drop};

    TRANSFER(xuiWidgetSetLayoutPrepareCallbackOwned, xuiWidgetCaptureLayoutPrepareCallback,
        xuiWidgetGetLayoutPrepareCallback, xui_widget_layout_prepare_proc, prepare, from, to);
    TRANSFER(xuiWidgetSetLayoutChildrenCallbackOwned, xuiWidgetCaptureLayoutChildrenCallback,
        xuiWidgetGetLayoutChildrenCallback, xui_widget_layout_children_proc, layout, from, to);
    TRANSFER(xuiWidgetSetLayoutCompleteCallbackOwned, xuiWidgetCaptureLayoutCompleteCallback,
        xuiWidgetGetLayoutCompleteCallback, xui_widget_layout_complete_proc, layout, from, to);
    TRANSFER(xuiWidgetSetImeCandidateRectOwned, xuiWidgetCaptureImeCandidateRect,
        xuiWidgetGetImeCandidateRect, xui_widget_ime_rect_proc, ime, from, to);
    TRANSFER(xuiWidgetSetCursorQueryCallbackOwned, xuiWidgetCaptureCursorQueryCallback,
        xuiWidgetGetCursorQueryCallback, xui_widget_cursor_proc, cursor, from, to);
    TRANSFER(xuiWidgetSetCacheRenderCallbackOwned, xuiWidgetCaptureCacheRenderCallback,
        xuiWidgetGetCacheRenderCallback, xui_widget_cache_render_proc, render, from, to);
    TRANSFER(xuiCodeEditSetTokenStyleResolverOwned, xuiCodeEditCaptureTokenStyleResolver,
        xuiCodeEditGetTokenStyleResolver, xui_code_token_style_proc, style, editor, secondEditor);

    xui_widget_callback_snapshot_t* snapshot = NULL;
    assert(xrtMemDebugFailAfter(0));
    assert(xuiWidgetCaptureContentMeasureCallback(from, &snapshot) == XUI_OK && !snapshot);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    assert(xuiWidgetSetContentMeasureCallback(from, measure, &state) == XUI_OK);
    assert(xuiWidgetCaptureContentMeasureCallback(from, &snapshot) == XUI_ERROR_UNSUPPORTED && !snapshot);
    assert(xuiWidgetSetContentMeasureCallbackOwned(from, measure, &state, &owner) == XUI_OK);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiWidgetCaptureContentMeasureCallback(from, &snapshot) == XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();
    xrtClearError();
    assert(!snapshot && state.references == 2);
    state.failHold = true;
    assert(xuiWidgetCaptureContentMeasureCallback(from, &snapshot) == XUI_ERROR_CALLBACK_FAILED);
    assert(!snapshot && state.references == 2 && xrtGetError());
    xrtClearError();
    state.failHold = false;

    assert(xuiWidgetCaptureContentMeasureCallback(from, &snapshot) == XUI_OK && snapshot);
    assert(xuiWidgetCallbackSnapshotTraceOwnership(snapshot, same_owner, &state));
    assert(xuiWidgetDestroyChecked(from) == XUI_OK && state.references == 2);
    state.active = snapshot;
    xthread* worker = xrtThreadCreate(wrong_thread, &state, 0);
    assert(worker && xrtThreadWaitFor(worker, 5000) == XWAIT_OK);
    xrtThreadDestroy(worker);
    assert(xuiWidgetCallbackSnapshotInstall(snapshot, to) == XUI_OK && state.references == 3);
    state.active = NULL;
    state.failDrop = true;
    bool consumed = false;
    assert(xuiWidgetCallbackSnapshotReleaseOwnerChecked(snapshot, &consumed) == XUI_ERROR_CALLBACK_FAILED);
    assert(consumed && state.references == 2 && xrtGetError());
    xrtClearError();
    state.failDrop = false;
    xui_vec2_t size = {0};
    assert(xuiWidgetMeasureContent(to, (xui_vec2_t){100, 100}, &size) == XUI_OK);
    assert(size.fX == 12 && size.fY == 8);
    assert(xuiDestroyChecked(context) == XUI_OK && state.references == 1);
    assert(xuiProxyReleaseChecked(proxy) == XUI_OK);
    puts("XUI_WIDGET_CALLBACK_SNAPSHOT_NATIVE_OK");
    return 0;
}
