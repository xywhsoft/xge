#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include "xui.h"
#include "xrt.h"
#include "xui_test_proxy.h"

#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>

enum { NODE_COUNT = 128 };

typedef struct tree_adapter_state {
    uintptr_t text[NODE_COUNT];
} tree_adapter_state;

static int count_nodes(xui_widget tree, void* user)
{
    (void)tree;
    (void)user;
    return NODE_COUNT;
}

static int read_node(xui_widget tree, int index, xui_tree_view_node_output_t* node, void* user)
{
    (void)tree;
    tree_adapter_state* state = user;
    node->iId = 100 + index;
    node->iParent = index ? 100 : -1;
    node->bExpanded = 1;
    node->bEnabled = 1;
    int status = xuiTreeViewNodeOutputSetText(node, (xstrview){ .Data = "Adapter", .Size = 7 });
    if (status == XUI_OK) state->text[index] = (uintptr_t)xuiTreeViewNodeOutputGetText(node).Data;
    return status;
}

static void measure_adapter(xui_widget tree)
{
    tree_adapter_state state = {0};
    assert(xuiTreeViewSetAdapterOwned(tree, count_nodes, NULL, NULL, read_node, &state, NULL) == XUI_OK);
    assert(xuiTreeViewSetSelected(tree, 101) == XUI_OK);
    xmemdebugsnapshot before, after;
    xrtMemDebugSnapshot(&before);
    assert(xuiTreeViewRefreshAdapter(tree) == XUI_OK);
    xrtMemDebugSnapshot(&after);
    assert(after.LiveCount == before.LiveCount && after.LiveBytes == before.LiveBytes);
    assert(xuiTreeViewGetNodeCount(tree) == NODE_COUNT && xuiTreeViewGetSelected(tree) == 101);
    unsigned transferred = 0;
    for (int i = 0; i < NODE_COUNT; ++i) {
        const xui_tree_view_node_t* node = xuiTreeViewGetNode(tree, i);
        assert(node && node->iId == 100 + i && !strcmp(node->sText, "Adapter"));
        transferred += (uintptr_t)node->sText == state.text[i];
    }
#ifdef XUI_TREE_ADAPTER_BASELINE
    assert(transferred == 0);
#else
    assert(transferred == NODE_COUNT);
#endif
    printf("XUI_TREE_REFRESH nodes=%d transferred=%u alloc=%llu free=%llu realloc=%llu\n",
        NODE_COUNT, transferred, (unsigned long long)(after.AllocCount - before.AllocCount),
        (unsigned long long)(after.FreeCount - before.FreeCount),
        (unsigned long long)(after.ReallocCount - before.ReallocCount));
    assert(xuiTreeViewSetAdapterOwned(tree, NULL, NULL, NULL, NULL, NULL, NULL) == XUI_OK);
}

#ifndef XUI_TREE_ADAPTER_BASELINE
static void check_descriptors(xui_widget tree)
{
    char rootText[] = "Root";
    xui_tree_node_desc_t nodes[] = {
        { .tState = { .iId = 10, .iParent = -1, .iDepth = 326, .bEnabled = 1 }, .sText = rootText },
        { .tState = { .iId = 11, .iParent = 10, .bExpanded = 1, .bEnabled = 1, .bIconReserved = 1 }, .sText = "Child" },
        { .tState = { .iId = 12, .iParent = 11, .bCheckReserved = 1, .bChecked = 1 } }
    };
    assert(xuiTreeViewSetNodeDescriptors(tree, nodes, 3) == XUI_OK);
    rootText[0] = '!';
    xui_tree_node_desc_t node;
    assert(xuiTreeViewGetVisibleCount(tree) == 1);
    assert(xuiTreeViewGetVisibleNodeDescriptor(tree, 0, &node) == XUI_OK && !strcmp(node.sText, "Root"));
    assert(node.tState.iId == 10 && node.tState.iDepth == 0 && node.tState.bHasChildren);
    assert(xuiTreeViewGetVisibleNodeDescriptor(tree, 1, &node) == XUI_ERROR_NOT_FOUND && node.tState.iId == 10);
    assert(xuiTreeViewGetNodeDescriptorById(tree, 12, &node) == XUI_OK);
    assert(node.tState.iDepth == 2 && node.tState.bChecked && node.tState.bCheckReserved && !node.tState.bEnabled);
    assert(node.sText && !node.sText[0]);
    assert(xuiTreeViewSetNodeExpanded(tree, 10, 1) == XUI_OK && xuiTreeViewGetVisibleCount(tree) == 3);
    assert(xuiTreeViewGetNodeDescriptor(tree, 1, &node) == XUI_OK && node.tState.bIconReserved);
    assert(xuiTreeViewSetSelected(tree, 11) == XUI_OK);

    nodes[2].tState.iId = 11;
    assert(xuiTreeViewSetNodeDescriptors(tree, nodes, 3) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiTreeViewGetVisibleCount(tree) == 3 && xuiTreeViewGetSelected(tree) == 11);
    nodes[2].tState.iId = 12;
    nodes[2].tState.iParent = 99;
    assert(xuiTreeViewSetNodeDescriptors(tree, nodes, 3) == XUI_ERROR_INVALID_ARGUMENT);
    nodes[2].tState.iParent = 11;

    int failures = 0;
    for (int budget = 0; budget < 64; ++budget) {
        xmemdebugsnapshot before, after;
        xrtMemDebugSnapshot(&before);
        assert(xrtMemDebugFailAfter(budget));
        int status = xuiTreeViewSetNodeDescriptors(tree, nodes, 3);
        int triggered = xrtMemDebugFailTriggered();
        xrtMemDebugFailClear();
        if (!triggered) {
            assert(status == XUI_OK && failures > 2);
            break;
        }
        xrtClearError();
        ++failures;
        assert(status == XUI_ERROR_OUT_OF_MEMORY);
        assert(xuiTreeViewGetVisibleCount(tree) == 3 && xuiTreeViewGetSelected(tree) == 11);
        assert(xuiTreeViewGetNodeDescriptor(tree, 0, &node) == XUI_OK && !strcmp(node.sText, "Root"));
        xrtMemDebugSnapshot(&after);
        assert(after.LiveCount == before.LiveCount && after.LiveBytes == before.LiveBytes);
    }
    assert(failures < 64);
    assert(xuiTreeViewSetNodeDescriptors(tree, NULL, 0) == XUI_OK && xuiTreeViewGetNodeCount(tree) == 0);
    printf("XUI_TREE_DESCRIPTORS_OOM failures=%d\n", failures);
}
#endif

int main(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
    xui_test_proxy_state_t proxy;
    xuiTestProxyInit(&proxy);
    xui_context context = NULL;
    xui_widget tree = NULL;
    assert(xuiCreate(&context) == XUI_OK);
    assert(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    assert(xuiTreeViewCreate(context, &tree, NULL) == XUI_OK);
#ifndef XUI_TREE_ADAPTER_BASELINE
    check_descriptors(tree);
#endif
    measure_adapter(tree);
    xuiDestroy(context);
    puts("XUI_TREE_DESCRIPTORS_NATIVE_OK");
    return 0;
}
