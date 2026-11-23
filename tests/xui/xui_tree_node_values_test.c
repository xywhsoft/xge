#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#include "xui.h"
#include "xui_test_proxy.h"

#include <assert.h>
#include <stdio.h>

typedef struct tree_values_test_t {
    xui_widget tree;
    xvalue* value;
    bool fail_second;
    int drops;
} tree_values_test_t;

static int adapter_count(xui_widget tree, void* user)
{
    tree_values_test_t* test = user;
    assert(tree == test->tree && xuiTreeViewGetNodeCount(tree) == 2);
    return 2;
}

static int adapter_node(xui_widget tree, int index, xui_tree_view_node_output_t* node,
                        xui_tree_view_value_output_t* output, void* user)
{
    tree_values_test_t* test = user;
    assert(tree == test->tree);
    node->iId = 20 + index;
    node->iParent = index ? 20 : -1;
    node->bExpanded = 1;
    int status = xuiTreeViewValueOutputSet(output, test->value);
    if (status != XUI_OK) return status;
    if (index && test->fail_second) return XUI_ERROR_INVALID_ARGUMENT;
    return xuiTreeViewNodeOutputSetText(node, XRT_STR_LITERAL("adapter"));
}

typedef struct edge_t {
    xrtownershipref target;
    int count;
} edge_t;

static bool count_edge(xrtownershipref ref, void* data)
{
    edge_t* edge = data;
    if (ref.Data == edge->target.Data && ref.Ops == edge->target.Ops) ++edge->count;
    return true;
}

static void populate_after_clear(void* data, void* user)
{
    tree_values_test_t* test = data;
    (void)user;
    ++test->drops;
    assert(xuiTreeViewGetNodeCount(test->tree) == 0);
    assert(xuiTreeViewAddNode(test->tree, 20, -1, "Replacement") == XUI_OK);
    assert(xuiTreeViewSetNodeValue(test->tree, 20, test->value) == XUI_OK);
}

static void refuse_busy_clear(void* data, void* user)
{
    tree_values_test_t* test = data;
    (void)user;
    assert(xuiTreeViewSetNodeValue(test->tree, 20, test->value) == XUI_ERROR_BUSY);
    assert(xuiTreeViewClear(test->tree) == XUI_ERROR_BUSY);
    ++test->drops;
}

static bool trace_empty(const xvalue* value, xrtownershipvisitor visit, void* user)
{
    (void)value;
    (void)visit;
    (void)user;
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
    xui_widget tree = NULL;
    assert(xuiTreeViewCreate(context, &tree, NULL) == XUI_OK);
    xui_tree_view_node_t legacy = {.iId = 10, .iParent = -1, .sText = "legacy", .bEnabled = 1, .pUser = &backend};
    assert(xuiTreeViewSetNodes(tree, &legacy, 1) == XUI_OK);

    xvalue* value = xrtValueInt(42);
    xvalue* replacement = xrtValueInt(59);
    assert(value && replacement);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiTreeViewSetNodeValue(tree, 10, value) == XUI_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtSetErrorTake(NULL);
    xvalue* output = value;
    assert(xuiTreeViewGetNodeValue(tree, 10, &output) == XUI_OK && output == NULL);
    assert(xuiTreeViewSetNodeValue(tree, 10, value) == XUI_OK);
    assert(xuiTreeViewGetNode(tree, 0)->pUser == &backend);
    assert(xrtMemDebugFailAfter(0));
    assert(xuiTreeViewSetNodeValue(tree, 10, replacement) == XUI_OK);
    assert(xuiTreeViewGetNodeValue(tree, 10, &output) == XUI_OK && output == replacement);
    assert(!xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();
    xrtValueRelease(output);

    xui_tree_node_desc_t rows[] = {
        {.tState = {.iId = 10, .iParent = -1, .bExpanded = 1, .bEnabled = 1}, .sText = "Root"},
        {.tState = {.iId = 11, .iParent = 10, .bEnabled = 1}, .sText = "Child"},
    };
    xvalue* values = xrtValueArray();
    assert(values && xrtValueArrayAppend(values, value) && xrtValueArrayAppend(values, xrtValueNull()));
    assert(xuiTreeViewSetNodeDescriptorsWithValues(tree, rows, 1, values) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiTreeViewGetNodeValue(tree, 10, &output) == XUI_OK && output == replacement);
    xrtValueRelease(output);
    int budget;
    for (budget = 0; budget < 64; ++budget) {
        assert(xrtMemDebugFailAfter(budget));
        int status = xuiTreeViewSetNodeDescriptorsWithValues(tree, rows, 2, values);
        xrtMemDebugFailClear();
        xrtSetErrorTake(NULL);
        if (status == XUI_OK) break;
        assert(status == XUI_ERROR_OUT_OF_MEMORY && xuiTreeViewGetNodeCount(tree) == 1);
        assert(xuiTreeViewGetNodeValue(tree, 10, &output) == XUI_OK && output == replacement);
        xrtValueRelease(output);
    }
    assert(budget > 0 && budget < 64);
    xrtValueRelease(values);
    assert(xuiTreeViewGetNodeValue(tree, 11, &output) == XUI_OK && output == NULL);
    edge_t edge = {xrtValueOwnership(value), 0};
    assert(xuiContextTraceOwnership(context, count_edge, &edge) && edge.count == 1);

    tree_values_test_t test = {tree, replacement, true, 0};
    assert(xuiTreeViewSetDataAdapterOwned(tree, adapter_count, &test, NULL, adapter_node, &test, NULL) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiTreeViewGetNodeValue(tree, 10, &output) == XUI_OK && output == value);
    xrtValueRelease(output);
    test.fail_second = false;
    assert(xuiTreeViewRefreshAdapter(tree) == XUI_OK);
    assert(xuiTreeViewGetNodeValue(tree, 20, &output) == XUI_OK && output == replacement);
    xrtValueRelease(output);
    assert(xuiTreeViewGetNodeValue(tree, 10, &output) == XUI_ERROR_NOT_FOUND && output == NULL);

    void* input = &test;
    static const xvaluehandleops busy_ops = {.Drop = refuse_busy_clear};
    xvalue* member = xrtValueHandleTake(&input, &busy_ops, NULL);
    xvalue* object = xrtValueObject();
    assert(member && !input && object && xrtValueHandleOwnershipBindPhased(member, trace_empty));
    assert(xrtValueObjectSet(object, XRT_STR_LITERAL("busy"), member));
    xrtValueRelease(member);
    assert(xuiTreeViewSetNodeValue(tree, 20, object) == XUI_OK);
    assert(xrtValueObjectRemove(object, XRT_STR_LITERAL("busy")) && test.drops == 1);
    assert(xuiTreeViewGetNodeValue(tree, 20, &output) == XUI_OK && output == object);
    xrtValueRelease(output);
    xrtValueRelease(object);

    static const xvaluehandleops ops = {.Drop = populate_after_clear};
    input = &test;
    xvalue* retired = xrtValueHandleTake(&input, &ops, NULL);
    assert(retired && !input && xrtValueHandleOwnershipBindPhased(retired, trace_empty));
    assert(xuiTreeViewSetNodeValue(tree, 20, retired) == XUI_OK);
    xrtValueRelease(retired);
    assert(xuiTreeViewClear(tree) == XUI_OK && test.drops == 2);
    assert(xuiTreeViewGetNodeValue(tree, 20, &output) == XUI_OK && output == replacement);
    xrtValueRelease(output);
    assert(xuiTreeViewSetNodeDescriptors(tree, rows, 2) == XUI_OK);
    assert(xuiTreeViewGetNodeValue(tree, 10, &output) == XUI_OK && output == NULL);
    assert(xuiTreeViewSetNodeValue(tree, 10, value) == XUI_OK);
    assert(xuiWidgetDestroyChecked(tree) == XUI_OK);
    assert(xuiDestroyChecked(context) == XUI_OK);
    xrtValueRelease(value);
    xrtValueRelease(replacement);
    puts("xui_tree_node_values_test passed");
    return 0;
}
