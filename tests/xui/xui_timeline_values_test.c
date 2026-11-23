#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#include "xui.h"
#include "xui_test_proxy.h"

#include <assert.h>
#include <stdio.h>

typedef struct timeline_value_drop_t {
    xui_widget timeline;
    xvalue* replacement;
    int calls;
} timeline_value_drop_t;

static bool trace_empty(const xvalue* value, xrtownershipvisitor visit, void* user)
{
    (void)value;
    (void)visit;
    (void)user;
    return true;
}

static void restore_after_clear(void* data, void* user)
{
    timeline_value_drop_t* state = data;
    (void)user;
    ++state->calls;
    assert(xuiTimeLineViewGetLayerCount(state->timeline) == 0);
    int layer = -1;
    assert(xuiTimeLineViewAddLayer(state->timeline, "replacement", &layer) == XUI_OK);
    assert(layer == 0);
    assert(xuiTimeLineViewSetLayerValue(state->timeline, layer, state->replacement) == XUI_OK);
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

static bool count_edge(xrtownershipref ref, void* raw)
{
    value_edge_t* edge = raw;
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
    xui_widget timeline = NULL;
    assert(xuiTimeLineViewCreate(context, &timeline, NULL) == XUI_OK);
    int intro = -1;
    int motion = -1;
    assert(xuiTimeLineViewAddLayer(timeline, "intro", &intro) == XUI_OK);
    assert(xuiTimeLineViewAddLayer(timeline, "motion", &motion) == XUI_OK);
    assert(intro == 0 && motion == 1);

    xvalue* value = xrtValueInt(77);
    xvalue* replacement = xrtValueInt(78);
    assert(value != NULL && replacement != NULL);
    assert(xuiTimeLineViewSetLayerUserData(timeline, motion, &backend) == XUI_OK);
    for (int budget = 0; budget < 2; ++budget) {
        assert(xrtMemDebugFailAfter(budget));
        assert(xuiTimeLineViewSetLayerValue(timeline, motion, value) == XUI_ERROR_OUT_OF_MEMORY);
        assert(xrtMemDebugFailTriggered());
        xrtMemDebugFailClear();
        xrtSetErrorTake(NULL);
        xvalue* output = value;
        assert(xuiTimeLineViewGetLayerValue(timeline, motion, &output) == XUI_OK && output == NULL);
    }
    assert(xuiTimeLineViewSetLayerValue(timeline, motion, value) == XUI_OK);
    assert(xuiTimeLineViewGetLayerUserData(timeline, motion) == &backend);
    value_edge_t edge = {xrtValueOwnership(value), 0};
    assert(xuiContextTraceOwnership(context, count_edge, &edge) && edge.count == 1);

    assert(xrtMemDebugFailAfter(0));
    assert(xuiTimeLineViewSetLayerValue(timeline, motion, replacement) == XUI_OK);
    xvalue* output = NULL;
    assert(xuiTimeLineViewGetLayerValue(timeline, motion, &output) == XUI_OK && output == replacement);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtValueRelease(output);

    int frameDrops = 0;
    int spanDrops = 0;
    int destroyDrops = 0;
    static const xvaluehandleops count_ops = {.Drop = count_drop};
    static const xvaluehandleops restore_ops = {.Drop = restore_after_clear};
    xvalue* frameValue = make_owned(&frameDrops, &count_ops);
    xvalue* spanValue = make_owned(&spanDrops, &count_ops);
    assert(xuiTimeLineViewSetFrame(timeline, motion, 4, XUI_TIMELINE_FRAME_KEY, &backend) == XUI_OK);
    assert(xuiTimeLineViewSetFrame(timeline, motion, 20, XUI_TIMELINE_FRAME_KEY, NULL) == XUI_OK);
    assert(xuiTimeLineViewSetFrameValue(timeline, motion, 4, value) == XUI_OK);
    assert(xuiTimeLineViewSetFrameValue(timeline, motion, 20, frameValue) == XUI_OK);
    xrtValueRelease(frameValue);
    int span = 0;
    int trimmed = 0;
    assert(xuiTimeLineViewAddSpan(timeline, motion, 2, 20, XUI_TIMELINE_SPAN_MOTION, "move", &span) == XUI_OK);
    assert(xuiTimeLineViewAddSpan(timeline, motion, 19, 20, XUI_TIMELINE_SPAN_MOTION, "trim", &trimmed) == XUI_OK);
    assert(xuiTimeLineViewSetSpanValue(timeline, span, value) == XUI_OK);
    assert(xuiTimeLineViewSetSpanValue(timeline, trimmed, spanValue) == XUI_OK);
    xrtValueRelease(spanValue);
    assert(xuiTimeLineViewMoveLayer(timeline, motion, intro) == XUI_OK);
    assert(xuiTimeLineViewGetLayerUserData(timeline, intro) == &backend);
    assert(xuiTimeLineViewGetFrameUserData(timeline, intro, 4) == &backend);
    assert(xuiTimeLineViewGetFrameValue(timeline, intro, 4, &output) == XUI_OK && output == value);
    xrtValueRelease(output);

    assert(xuiTimeLineViewSetFrameCount(timeline, 12) == XUI_OK);
    assert(frameDrops == 1 && spanDrops == 1);
    assert(xuiTimeLineViewGetSpanValue(timeline, trimmed, &output) == XUI_ERROR_INVALID_ARGUMENT && output == NULL);
    assert(xuiTimeLineViewGetSpanValue(timeline, span, &output) == XUI_OK && output == value);
    xrtValueRelease(output);
    assert(xuiTimeLineViewRemoveSpan(timeline, span) == XUI_OK);
    assert(xuiTimeLineViewClearFrame(timeline, intro, 4) == XUI_OK);
    assert(xuiTimeLineViewGetFrameValue(timeline, intro, 4, &output) == XUI_OK && output == NULL);
    edge.count = 0;
    assert(xuiContextTraceOwnership(context, count_edge, &edge) && edge.count == 0);
    assert(xuiTimeLineViewRemoveLayer(timeline, intro) == XUI_OK);
    assert(xuiTimeLineViewGetLayerValue(timeline, intro, &output) == XUI_OK && output == NULL);

    timeline_value_drop_t state = {timeline, replacement, 0};
    xvalue* restore = make_owned(&state, &restore_ops);
    assert(xuiTimeLineViewSetLayerValue(timeline, intro, restore) == XUI_OK);
    xrtValueRelease(restore);
    assert(xuiTimeLineViewClear(timeline) == XUI_OK && state.calls == 1);
    assert(xuiTimeLineViewGetLayerValue(timeline, intro, &output) == XUI_OK && output == replacement);
    xrtValueRelease(output);

    xvalue* onDestroy = make_owned(&destroyDrops, &count_ops);
    assert(xuiTimeLineViewSetLayerValue(timeline, intro, onDestroy) == XUI_OK);
    xrtValueRelease(onDestroy);
    assert(xuiWidgetDestroyChecked(timeline) == XUI_OK && destroyDrops == 1);
    assert(xuiDestroyChecked(context) == XUI_OK);
    xrtValueRelease(value);
    xrtValueRelease(replacement);
    puts("xui_timeline_values_test passed");
    return 0;
}
