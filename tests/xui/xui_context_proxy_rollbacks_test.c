/* Exercise the only remaining Proxy owner through real Context shutdown. */
#include "xui.h"

#include <assert.h>
#include <stdio.h>

typedef struct Backend {
    size_t references;
    unsigned dropped;
} Backend;

static bool ownerCount(const void* raw, size_t* output)
{
    *output = ((const Backend*)raw)->references;
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
    ++((Backend*)raw)->references;
    return true;
}

static bool dropOwner(const void* raw)
{
    Backend* backend = (Backend*)raw;
    --backend->references;
    ++backend->dropped;
    return true;
}

static void invalidate(void* raw)
{
    ++*(unsigned*)raw;
}

static void reportFinalizerFailure(xvalue* retired, ptr raw)
{
    (void)retired;
    xrtSetErrorTake(xrtErrorRef(raw));
}

static void closeWithPendingOutputs(bool failingFinalizer)
{
    static const xrtownershipops ops = {ownerCount, ownerTrace};
    Backend backend = {1, 0};
    const xui_proxy_t table = xuiProxyXge();
    const xui_callback_owner_t callbacks = {{&backend, &ops}, holdOwner, dropOwner};
    xui_proxy proxy = NULL;
    xui_context context = NULL;
    assert(xuiProxyCreateOwned(&table, &callbacks, &proxy) == XUI_OK);
    assert(xuiCreateOwned(proxy, &context) == XUI_OK);
    assert(xuiProxyReleaseChecked(proxy) == XUI_OK);
    assert(xuiGetProxyOwned(context) == proxy && backend.references == 2);

    unsigned invalidated = 0;
    xui_lifetime_link_t watch = {0};
    assert(xuiContextLifetimeWatch(context, &watch, invalidate, &invalidated) == XUI_OK);

    xvalue* state = xrtValueObject();
    xui_surface surface = NULL;
    xui_draw_context draw = NULL;
    assert(state && xuiSurfaceCreateWithValueOwned(proxy, state, &surface) == XUI_OK);
    assert(xuiDrawStageCallbackOutput(&draw, proxy, NULL, surface) == XUI_OK);
    assert(xuiSurfaceReleaseChecked(surface) == XUI_OK);

    xerror* failure = NULL;
    xvalue* faulty = NULL;
    xui_surface faultySurface = NULL;
    xvalueviewv1 faultyReader = {0};
    if (failingFinalizer) {
        failure = xrtErrorCreate(XERR_STATE, "test", 23, "Proxy finalizer failure");
        faulty = xrtValueObject();
        assert(failure && faulty && xrtValueObjectFinalizerBind(faulty, reportFinalizerFailure, failure));
        assert(xuiSurfaceStageCallbackOutput(&faultySurface, proxy, faulty) == XUI_OK);
        assert(xrtValueViewBeginV1(faulty, &faultyReader));
    }
    xvalueviewv1 reader = {0};
    assert(xrtValueViewBeginV1(state, &reader));
    xuiDrawRollbackCallbackOutput(draw);
    if (failingFinalizer) {
        xuiSurfaceRollbackCallbackOutput(faultySurface);
        assert(xrtValueViewEndV1(&faultyReader));
        xrtValueRelease(faulty);
        bool consumed = true;
        assert(xuiDestroyOwnerChecked(context, &consumed) == XUI_ERROR_CALLBACK_FAILED);
        assert(!consumed && xrtGetError() == failure);
        assert(invalidated == 0 && backend.references == 2 && backend.dropped == 0);
        xrtClearError();
        xrtErrorFree(failure);
    }

    assert(xrtMemDebugFailAfter(0));
    for (int attempt = 0; attempt < 2; ++attempt) {
        bool consumed = true;
        assert(xuiDestroyOwnerChecked(context, &consumed) == XUI_STATUS_DEFERRED);
        assert(!consumed && !xrtGetError());
        assert(invalidated == 0 && backend.references == 2 && backend.dropped == 0);
        assert(xuiDrawGetTarget(draw) == surface);
    }
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();

    assert(xrtValueViewEndV1(&reader));
    bool consumed = false;
    assert(xuiDestroyOwnerChecked(context, &consumed) == XUI_OK);
    assert(consumed && invalidated == 1 && backend.references == 1 && backend.dropped == 1);
    assert(!watch.ppPrevious && !xrtGetError());
    xrtValueRelease(state);
}

int main(void)
{
    assert(xrtMemDebugEnable(true));
    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    closeWithPendingOutputs(false);
    closeWithPendingOutputs(true);
    xrtMemDebugSnapshot(&after);
    assert(after.LiveCount == before.LiveCount && after.LiveBytes == before.LiveBytes);
    assert(after.DoubleFreeCount == before.DoubleFreeCount && after.InvalidFreeCount == before.InvalidFreeCount);
    puts("XUI_CONTEXT_PROXY_ROLLBACKS_OK");
    return 0;
}
