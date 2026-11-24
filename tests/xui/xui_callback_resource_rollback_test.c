#include "xui.h"

#include <assert.h>
#include <stdio.h>

typedef struct BackendOwner { size_t references; } BackendOwner;

static bool ownerCount(const void* raw, size_t* output)
{
    *output = ((const BackendOwner*)raw)->references;
    return true;
}

static bool ownerTrace(const void* raw, xrtownershipvisitor visit, void* context)
{
    (void)raw;
    (void)visit;
    (void)context;
    return true;
}

static bool holdOwner(const void* raw)
{
    ++((BackendOwner*)raw)->references;
    return true;
}

static bool dropOwner(const void* raw)
{
    --((BackendOwner*)raw)->references;
    return true;
}

static xui_proxy makeProxy(BackendOwner* owner)
{
    static const xrtownershipops ops = {ownerCount, ownerTrace};
    const xui_proxy_t table = {.iSize = sizeof(table), .iVersion = XUI_PROXY_VERSION};
    const xui_callback_owner_t callbacks = {{owner, &ops}, holdOwner, dropOwner};
    xui_proxy proxy = NULL;
    assert(xuiProxyCreateOwned(&table, &callbacks, &proxy) == XUI_OK && proxy);
    return proxy;
}

static bool countEdge(xrtownershipref reference, void* raw)
{
    assert(reference.Ops && reference.Data);
    ++*(unsigned*)raw;
    return true;
}

static void expectEdges(xrtownershipref node, unsigned expected)
{
    unsigned observed = 0;
    xrtownershipscope freeze = {0};
    assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(node.Ops->Trace(node.Data, countEdge, &observed) && observed == expected);
    assert(xrtOwnershipScopeEnd(&freeze));
}

static int32 drainOnWorker(ptr raw)
{return xuiProxyDrainCallbackRollbacks(raw);}

static void busyOutputs(xui_proxy proxy)
{
    xvalue* state = xrtValueObject();
    assert(state);
    xui_surface surface = NULL;
    xui_font font = NULL;
    xui_draw_context draw = NULL;
    assert(xuiSurfaceStageCallbackOutput(&surface, proxy, state) == XUI_OK);
    assert(xuiFontStageCallbackOutput(&font, proxy, state) == XUI_OK);
    assert(xuiDrawStageCallbackOutput(&draw, proxy, state, NULL) == XUI_OK);
    const xui_surface original = surface;
    assert(xuiSurfaceStageCallbackOutput(&surface, proxy, NULL) == XUI_ERROR_INVALID_ARGUMENT);
    assert(surface == original);

    xerror* primary = xrtErrorCreate(XERR_STATE, "test", -7, "callback failure");
    assert(primary);
    xvalueviewv1 view = {0};
    assert(xrtValueViewBeginV1(state, &view));
    assert(xrtMemDebugFailAfter(0));
    xrtSetErrorTake(primary);
    xuiDrawRollbackCallbackOutput(draw);
    xuiFontRollbackCallbackOutput(font);
    xuiSurfaceRollbackCallbackOutput(surface);
    assert(xrtGetError() == primary);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtClearError();

    expectEdges(xuiProxyOwnership(proxy), 4);
    assert(xuiProxyResourceOwnershipBusy(xuiProxyOwnership(proxy)));
    assert(xuiProxyDrainCallbackRollbacks(proxy) == XUI_ERROR_BUSY && !xrtGetError());
    expectEdges(xuiProxyOwnership(proxy), 4);
    xthread* worker = xrtThreadCreate(drainOnWorker, proxy, 0);
    assert(worker && xrtThreadWait(worker) == XWAIT_OK && xrtThreadExitCode(worker) == XUI_ERROR_BUSY);
    xrtThreadDestroy(worker);
    assert(xrtValueViewEndV1(&view));
    assert(xuiProxyDrainCallbackRollbacks(proxy) == XUI_OK);
    expectEdges(xuiProxyOwnership(proxy), 1);
    assert(!xuiProxyResourceOwnershipBusy(xuiProxyOwnership(proxy)));
    xrtValueRelease(state);
}

static void lastTarget(xui_proxy proxy)
{
    xvalue* pixels = xrtValueObject();
    xui_surface surface = NULL;
    xui_draw_context draw = NULL;
    assert(pixels && xuiSurfaceCreateWithValueOwned(proxy, pixels, &surface) == XUI_OK);
    assert(xuiDrawStageCallbackOutput(&draw, proxy, NULL, surface) == XUI_OK);
    assert(xuiSurfaceReleaseChecked(surface) == XUI_OK);
    xvalueviewv1 view = {0};
    assert(xrtValueViewBeginV1(pixels, &view));
    xuiDrawRollbackCallbackOutput(draw);
    expectEdges(xuiDrawOwnership(draw), 2);
    expectEdges(xuiProxyOwnership(proxy), 2);
    assert(xuiProxyDrainCallbackRollbacks(proxy) == XUI_ERROR_BUSY);

    assert(xrtValueViewEndV1(&view));
    assert(xuiProxyDrainCallbackRollbacks(proxy) == XUI_OK);
    expectEdges(xuiProxyOwnership(proxy), 1);
    xrtValueRelease(pixels);
}

static void publishedTarget(xui_proxy proxy, bool closeInScope)
{
    xvalue* pixels = xrtValueObject();
    xui_surface surface = NULL;
    xui_draw_context draw = NULL;
    xui_draw_scope_t scope = {0};
    assert(pixels && xuiSurfaceCreateWithValueOwned(proxy, pixels, &surface) == XUI_OK);
    assert(xuiDrawCreateWithValueOwned(proxy, NULL, surface, &draw) == XUI_OK);
    assert(xuiSurfaceReleaseChecked(surface) == XUI_OK);
    bool consumed = false;
    if (closeInScope) {
        assert(xuiDrawCallScopeBegin(draw, &scope) == XUI_OK);
        assert(xuiDrawReleaseOwnerChecked(draw, &consumed) == XUI_OK && consumed);
    }

    xvalueviewv1 view = {0};
    assert(xrtValueViewBeginV1(pixels, &view));
    if (closeInScope) {
        assert(xuiDrawCallScopeEnd(&scope) == XUI_ERROR_BUSY);
        assert(!scope._pSelf && !scope._pDraw);
        expectEdges(xuiProxyOwnership(proxy), 2);
        assert(xuiProxyDrainCallbackRollbacks(proxy) == XUI_ERROR_BUSY);
    } else {
        assert(xuiDrawReleaseOwnerChecked(draw, &consumed) == XUI_ERROR_BUSY && !consumed);
    }
    assert(xuiDrawGetProxy(draw) == proxy && xuiDrawGetTarget(draw) == surface);
    assert(xrtValueViewEndV1(&view));

    if (closeInScope) assert(xuiProxyDrainCallbackRollbacks(proxy) == XUI_OK);
    else assert(xuiDrawReleaseOwnerChecked(draw, &consumed) == XUI_OK && consumed);
    expectEdges(xuiProxyOwnership(proxy), 1);
    xrtValueRelease(pixels);
}

typedef struct ClosingEnd {
    xui_draw_context draw;
    xvalue* pixels;
    xvalueviewv1 reader;
    unsigned calls;
} ClosingEnd;

static int closeDuringEnd(xui_proxy proxy, void* raw)
{
    (void)proxy;
    ClosingEnd* state = *(ClosingEnd**)raw;
    bool consumed = false;
    ++state->calls;
    assert(xuiDrawReleaseOwnerChecked(state->draw, &consumed) == XUI_OK && consumed);
    assert(xrtValueViewBeginV1(state->pixels, &state->reader));
    return XUI_OK;
}

static void reentrantEndWithBusyTarget(xui_proxy proxy)
{
    ClosingEnd state = {.pixels = xrtValueObject()};
    xui_surface surface = NULL;
    void* payload = NULL;
    assert(state.pixels && xuiSurfaceCreateWithValueOwned(proxy, state.pixels, &surface) == XUI_OK);
    assert(xuiDrawCreateOwned(proxy, surface, sizeof(ClosingEnd*), closeDuringEnd, &state.draw, &payload) == XUI_OK);
    *(ClosingEnd**)payload = &state;
    assert(xuiSurfaceReleaseChecked(surface) == XUI_OK);

    assert(xuiDrawEndChecked(state.draw) == XUI_ERROR_BUSY && state.calls == 1);
    assert(xuiDrawGetTarget(state.draw) == surface && xuiDrawGetProxy(state.draw) == proxy);
    expectEdges(xuiProxyOwnership(proxy), 2);
    assert(xrtValueViewEndV1(&state.reader));
    assert(xuiProxyDrainCallbackRollbacks(proxy) == XUI_OK && state.calls == 1);
    expectEdges(xuiProxyOwnership(proxy), 1);
    xrtValueRelease(state.pixels);
}

typedef struct Reentry {
    xui_proxy proxy;
    xvalue* other;
    xvalueviewv1 view;
    unsigned calls;
} Reentry;

static void reenter(xvalue* retired, ptr raw)
{
    (void)retired;
    Reentry* state = raw;
    ++state->calls;
    assert(xuiProxyDrainCallbackRollbacks(state->proxy) == XUI_ERROR_BUSY);
    xui_surface another = NULL;
    assert(xuiSurfaceStageCallbackOutput(&another, state->proxy, state->other) == XUI_OK);
    assert(xrtValueViewBeginV1(state->other, &state->view));
    xuiSurfaceRollbackCallbackOutput(another);
}

static void reentrantBatch(xui_proxy proxy)
{
    Reentry state = {.proxy = proxy, .other = xrtValueObject()};
    xvalue* value = xrtValueObject();
    assert(state.other && value && xrtValueObjectFinalizerBind(value, reenter, &state));
    xui_surface output = NULL;
    assert(xuiSurfaceStageCallbackOutput(&output, proxy, value) == XUI_OK);
    xvalueviewv1 view = {0};
    assert(xrtValueViewBeginV1(value, &view));
    xuiSurfaceRollbackCallbackOutput(output);
    assert(xrtValueViewEndV1(&view));
    xrtValueRelease(value);

    assert(xuiProxyDrainCallbackRollbacks(proxy) == XUI_ERROR_BUSY);
    assert(state.calls == 1 && xrtValueViewEndV1(&state.view));
    expectEdges(xuiProxyOwnership(proxy), 2);
    assert(xuiProxyDrainCallbackRollbacks(proxy) == XUI_OK);
    assert(state.calls == 1);
    xrtValueRelease(state.other);
}

static void allocationAndPublication(xui_proxy proxy)
{
    xvalue* value = xrtValueObject();
    xui_surface surface = NULL, normal = NULL;
    xui_font font = NULL;
    xui_draw_context draw = NULL;
    assert(value);
    for (unsigned kind = 0; kind < 3; ++kind) {
        assert(xrtMemDebugFailAfter(0));
        int error;
        if (kind == 0) error = xuiSurfaceStageCallbackOutput(&surface, proxy, value);
        else if (kind == 1) error = xuiFontStageCallbackOutput(&font, proxy, value);
        else error = xuiDrawStageCallbackOutput(&draw, proxy, value, NULL);
        assert(error == XUI_ERROR_OUT_OF_MEMORY && !surface && !font && !draw);
        assert(xrtMemDebugFailTriggered());
        xrtMemDebugFailClear();
        xrtClearError();
    }

    xmemdebugsnapshot before, ordinary, staged;
    xrtMemDebugSnapshot(&before);
    assert(xuiSurfaceCreateWithValueOwned(proxy, value, &normal) == XUI_OK);
    xrtMemDebugSnapshot(&ordinary);
    assert(xuiSurfaceStageCallbackOutput(&surface, proxy, value) == XUI_OK);
    xrtMemDebugSnapshot(&staged);
    assert(staged.AllocCount - ordinary.AllocCount == 1 && ordinary.AllocCount - before.AllocCount == 1);
    assert(staged.LiveBytes - ordinary.LiveBytes == ordinary.LiveBytes - before.LiveBytes);
    xvalue* published = NULL;
    assert(xuiSurfaceGetValueOwned(surface, &published) == XUI_OK && published == value);
    xrtValueRelease(published);
    assert(xuiSurfaceReleaseChecked(surface) == XUI_OK);
    assert(xuiSurfaceReleaseChecked(normal) == XUI_OK);
    xrtValueRelease(value);
}

int main(void)
{
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    BackendOwner owner = {1};
    xui_proxy proxy = makeProxy(&owner);
    busyOutputs(proxy);
    lastTarget(proxy);
    publishedTarget(proxy, false);
    publishedTarget(proxy, true);
    reentrantEndWithBusyTarget(proxy);
    reentrantBatch(proxy);
    allocationAndPublication(proxy);
    assert(xuiProxyReleaseChecked(proxy) == XUI_OK && owner.references == 1);
    xrtMemDebugSnapshot(&after);
    assert(after.LiveCount == before.LiveCount && after.LiveBytes == before.LiveBytes);
    assert(after.DoubleFreeCount == before.DoubleFreeCount && after.InvalidFreeCount == before.InvalidFreeCount);
    puts("XUI_CALLBACK_ROLLBACK_NATIVE_OK");
    return 0;
}
