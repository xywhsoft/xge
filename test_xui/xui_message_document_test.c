#include "../xui_document_ui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef XUI_DOCUMENT_HEADLESS_IMAGE_STUBS
#include "../xge.h"
/* PNG clipboard paths are outside this headless MessageList test. */
int xgeImageInfoMemory(const void* data, int size, int* width, int* height)
{
    (void)data; (void)size; (void)width; (void)height;
    fputs("unexpected XGE image probe in headless MessageList test\n", stderr);
    abort();
}
int xgeImageEncodePNGEx(int width, int height, const void* pixels,
    int stride, uint32_t flags, void** data, size_t* size)
{
    (void)width; (void)height; (void)pixels; (void)stride;
    (void)flags; (void)data; (void)size;
    fputs("unexpected XGE PNG encode in headless MessageList test\n", stderr);
    abort();
}
#endif

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #c); exit(1); } } while (0)

static xui_text_shape_proc message_base_shape;
static uint64_t message_shaped_bytes;
static int message_count_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    int result = message_base_shape(proxy, pTextItem, shape);
    if (result == XUI_OK && bytes > 0) message_shaped_bytes += (uint64_t)bytes;
    return result;
}

static xui_doc_renderer_stats_t message_render_stats(xui_widget list)
{
    xui_doc_renderer_stats_t stats = {0};
    uint64_t shaped_before = message_shaped_bytes;
    stats.iSize = sizeof(stats);
    CHECK(xuiMessageListGetNodeDocumentRenderStats(list, "long", &stats) == XUI_OK);
    CHECK(message_shaped_bytes == shaped_before);
    return stats;
}

static void dispatch(xui_context context)
{
    CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
}

static void activated(xui_widget list, xui_doc_node_id node, const char* resource, void* user)
{
    int* count = (int*)user;
    (void)list;
    if (node != 0 && resource != NULL &&
        (!strcmp(resource, "https://example.test/rich") ||
         !strcmp(resource, "https://example.test/linked-image"))) (*count)++;
}

static int formula_ready;
static int measure_formula(xui_document_snapshot snapshot, xui_doc_node_id node,
    float width, float zoom, xui_vec2_t* size, float* baseline, void* user)
{
    xui_doc_node_info_t info = {0};
    (void)width; (void)zoom; (void)user;
    info.iSize = sizeof(info);
    if (!formula_ready || xuiDocumentSnapshotGetNode(snapshot, node, &info) != XUI_OK ||
            info.iKind != XUI_DOC_MATH) return XUI_ERROR_UNSUPPORTED;
    size->fX = 140;
    size->fY = 96;
    *baseline = 72;
    return XUI_OK;
}

typedef struct message_font_switch { xui_font current; } message_font_switch;
static xui_font message_select_font(xui_context context, const char* family,
    uint32_t marks, float size, void* user)
{
    message_font_switch* state = (message_font_switch*)user;
    (void)context; (void)family; (void)marks; (void)size;
    return state->current;
}

static void test_message_document_font_invalidation(void)
{
    static const char source[] =
        "one\ntwo\nthree\nfour\nfive\nsix\nseven\neight\nnine\nten\n";
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_document document = NULL;
    xui_widget list = NULL;
    xui_font normal = NULL, alternate = NULL;
    xui_doc_desc_t doc_desc = {0};
    xui_message_document_desc_t binding = {0};
    xui_message_list_desc_t list_desc = {0};
    xui_message_node_t nodes[2] = {{0}};
    message_font_switch switcher = {0};
    xui_rect_t before, stale, after;
    float tail_before, tail_after;
    int changes;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadMemory(&proxy.tProxy, &normal,
        NULL, 0, 14, 0) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadMemory(&proxy.tProxy, &alternate,
        NULL, 0, 24, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, normal) == XUI_OK);
    CHECK(xuiInputViewport(context, 480, 400) == XUI_OK);
    doc_desc.iSize = sizeof(doc_desc);
    doc_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&doc_desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, source, sizeof(source) - 1) == XUI_OK);
    nodes[0].iSize = sizeof(nodes[0]); nodes[0].iType = XUI_MESSAGE_NODE_OTHER;
    nodes[0].sId = "answer"; nodes[0].sSender = "Agent";
    nodes[1].iSize = sizeof(nodes[1]); nodes[1].iType = XUI_MESSAGE_NODE_OTHER;
    nodes[1].sId = "tail"; nodes[1].sSender = "Agent"; nodes[1].sText = "next";
    list_desc.iSize = sizeof(list_desc);
    list_desc.arrNodes = nodes; list_desc.iNodeCount = 2;
    CHECK(xuiMessageListCreate(context, &list, &list_desc) == XUI_OK);
    CHECK(xuiSetRootWidget(context, list) == XUI_OK);
    CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 480, 400}) == XUI_OK);
    switcher.current = normal;
    binding.iSize = sizeof(binding); binding.pDocument = document;
    binding.tRenderer.iSize = sizeof(binding.tRenderer);
    binding.tRenderer.onFont = message_select_font;
    binding.tRenderer.pUser = &switcher;
    CHECK(xuiMessageListSetNodeDocument(list, "answer", &binding) == XUI_OK);
    before = xuiMessageListGetNodeRect(list, 0);
    tail_before = xuiMessageListGetNodeRect(list, 1).fY;
    switcher.current = alternate;
    stale = xuiMessageListGetNodeRect(list, 0);
    CHECK(stale.fH == before.fH);
    changes = xuiMessageListGetChangeCount(list);
    CHECK(xuiMessageListInvalidateNodeDocumentFonts(list, "answer") == XUI_OK);
    after = xuiMessageListGetNodeRect(list, 0);
    tail_after = xuiMessageListGetNodeRect(list, 1).fY;
    CHECK(after.fH > before.fH + 20 && tail_after > tail_before + 20);
    CHECK(xuiMessageListGetChangeCount(list) == changes + 1);
    CHECK(xuiMessageListInvalidateNodeDocumentFonts(list, "missing") == XUI_ERROR_NOT_FOUND);
    CHECK(xuiMessageListSetNodeDocument(list, "answer", NULL) == XUI_OK);
    CHECK(xuiMessageListInvalidateNodeDocumentFonts(list, "answer") == XUI_ERROR_NOT_FOUND);
    xuiDestroy(context);
    xuiDocumentRelease(document);
    proxy.tProxy.fontDestroy(&proxy.tProxy, alternate);
    proxy.tProxy.fontDestroy(&proxy.tProxy, normal);
    puts("MessageList bound Document font invalidation updates row and successor geometry");
}

static void drag_selection(xui_context context, int from_x, int from_y, int to_x, int to_y)
{
    CHECK(xuiInputPointerDown(context, from_x, from_y,
        XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    dispatch(context);
    CHECK(xuiInputPointerMove(context, to_x, to_y, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    dispatch(context);
    CHECK(xuiInputPointerUp(context, to_x, to_y, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    dispatch(context);
}

static void message_image_surface_destroy(xui_context context, void* handle, void* user)
{
    xui_test_proxy_state_t* proxy = (xui_test_proxy_state_t*)user;
    (void)context;
    proxy->tProxy.surfaceDestroy(&proxy->tProxy, (xui_surface)handle);
}

static void message_image_resource_set(xui_context context, xui_test_proxy_state_t* proxy,
    int width, int height, xui_resource* resource)
{
    xui_surface surface = NULL;
    xui_resource_desc_t desc = {0};
    CHECK(xuiTestSurfaceCreate(proxy, &surface, width, height, 0) == XUI_OK);
    desc.iSize = sizeof(desc);
    desc.sName = "doc.message.image";
    desc.iKind = XUI_RESOURCE_SURFACE;
    desc.pHandle = surface;
    desc.pUser = proxy;
    desc.onDestroy = message_image_surface_destroy;
    CHECK(xuiResourceSet(context, resource, &desc) == XUI_OK);
}

static void test_message_image_registry_update(void)
{
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_widget list = NULL;
    xui_font font = NULL;
    xui_document document = NULL;
    xui_resource resource = NULL;
    xui_doc_desc_t doc_desc = {0};
    xui_message_document_desc_t binding = {0};
    xui_message_list_desc_t list_desc = {0};
    xui_message_node_t nodes[2] = {{0}};
    xui_rect_t before, loaded, replaced, removed;
    float tail_before, tail_loaded, tail_replaced;
    uint64_t revision;
    const char* source = "![alt](doc.message.image)\n";
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadMemory(&proxy.tProxy, &font, NULL, 0, 16.0f, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiInputViewport(context, 480, 320) == XUI_OK);
    doc_desc.iSize = sizeof(doc_desc);
    doc_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    doc_desc.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&doc_desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK);
    revision = xuiDocumentGetRevision(document);
    nodes[0].iSize = sizeof(nodes[0]); nodes[0].iType = XUI_MESSAGE_NODE_OTHER;
    nodes[0].sId = "image"; nodes[0].sSender = "Agent"; nodes[0].sText = "fallback";
    nodes[1].iSize = sizeof(nodes[1]); nodes[1].iType = XUI_MESSAGE_NODE_OTHER;
    nodes[1].sId = "tail"; nodes[1].sSender = "Agent"; nodes[1].sText = "tail";
    list_desc.iSize = sizeof(list_desc); list_desc.arrNodes = nodes; list_desc.iNodeCount = 2;
    CHECK(xuiMessageListCreate(context, &list, &list_desc) == XUI_OK);
    CHECK(xuiSetRootWidget(context, list) == XUI_OK);
    CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 480, 320}) == XUI_OK);
    binding.iSize = sizeof(binding); binding.pDocument = document;
    CHECK(xuiMessageListSetNodeDocument(list, "image", &binding) == XUI_OK);
    before = xuiMessageListGetNodeRect(list, 0);
    tail_before = xuiMessageListGetNodeRect(list, 1).fY;
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    xuiWidgetClearDirty(list, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
    message_image_resource_set(context, &proxy, 240, 120, &resource);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiWidgetGetDirtyFlags(list) & XUI_WIDGET_DIRTY_RENDER);
    loaded = xuiMessageListGetNodeRect(list, 0);
    tail_loaded = xuiMessageListGetNodeRect(list, 1).fY;
    CHECK(loaded.fH > before.fH + 50 && tail_loaded > tail_before + 50);
    message_image_resource_set(context, &proxy, 80, 200, &resource);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    replaced = xuiMessageListGetNodeRect(list, 0);
    tail_replaced = xuiMessageListGetNodeRect(list, 1).fY;
    CHECK(replaced.fH > loaded.fH + 50 && tail_replaced > tail_loaded + 50);
    CHECK(xuiResourceRemove(resource) == XUI_OK);
    resource = NULL;
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    removed = xuiMessageListGetNodeRect(list, 0);
    CHECK(removed.fH < loaded.fH && xuiMessageListGetNodeRect(list, 1).fY < tail_loaded);
    CHECK(xuiDocumentGetRevision(document) == revision);
    xuiDestroy(context);
    xuiDocumentRelease(document);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("MessageList named image: registry add/replace/remove updates height and following rows");
}

static void test_cross_message_selection(void)
{
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_widget list = NULL;
    xui_surface surface = NULL, cache = NULL;
    xui_font font = NULL;
    xui_document first = NULL, last = NULL;
    xui_doc_desc_t doc_desc = {0};
    xui_message_list_desc_t list_desc = {0};
    xui_message_document_desc_t binding = {0};
    xui_message_node_t nodes[5] = {{0}};
    xui_message_list_metrics_t metrics;
    xui_rect_t content, world, bubble;
    xui_doc_range_t range;
    char selected[512], forward[512];
    int body_x[5] = {0}, body_y[5] = {0};
    int i;
    xui_rect_i_t damage = {0, 0, 480, 600};
    uint32_t first_selection_color = XUI_COLOR_RGBA(201, 81, 43, 117);
    uint32_t last_selection_color = XUI_COLOR_RGBA(63, 171, 89, 119);
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadMemory(&proxy.tProxy, &font, NULL, 0, 16.0f, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiInputViewport(context, 480, 600) == XUI_OK);
    doc_desc.iSize = sizeof(doc_desc);
    doc_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    doc_desc.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&doc_desc, &first) == XUI_OK);
    CHECK(xuiDocumentCreate(&doc_desc, &last) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(first, "Alpha first line.\n", strlen("Alpha first line.\n")) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(last, "Gamma final line.\n", strlen("Gamma final line.\n")) == XUI_OK);
    for (i = 0; i < 5; i++) {
        nodes[i].iSize = sizeof(nodes[i]);
        nodes[i].iType = XUI_MESSAGE_NODE_OTHER;
        nodes[i].sSender = "Agent";
    }
    nodes[0].sId = "first"; nodes[0].sText = "first fallback";
    nodes[1].sId = "middle"; nodes[1].sText = "middle plain";
    nodes[2].sId = "system"; nodes[2].sText = "skip system";
    nodes[2].iType = XUI_MESSAGE_NODE_SYSTEM;
    nodes[3].sId = "last"; nodes[3].sText = "last fallback";
    nodes[4].sId = "tail"; nodes[4].sText = "tail plain";
    list_desc.iSize = sizeof(list_desc);
    list_desc.arrNodes = nodes;
    list_desc.iNodeCount = 5;
    CHECK(xuiMessageListCreate(context, &list, &list_desc) == XUI_OK);
    CHECK(xuiSetRootWidget(context, list) == XUI_OK);
    CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 480, 600}) == XUI_OK);
    CHECK(xuiMessageListSetAutoScroll(list, 0) == XUI_OK);
    binding.iSize = sizeof(binding);
    binding.pDocument = first;
    binding.iSelectionColor = first_selection_color;
    CHECK(xuiMessageListSetNodeDocument(list, "first", &binding) == XUI_OK);
    binding.pDocument = last;
    binding.iSelectionColor = last_selection_color;
    CHECK(xuiMessageListSetNodeDocument(list, "last", &binding) == XUI_OK);
    CHECK(xuiMessageListSetScroll(list, 0) == XUI_OK);
    CHECK(xuiMessageListGetMetrics(list, &metrics) == XUI_OK);
    content = xuiWidgetGetContentRect(list);
    world = xuiWidgetGetWorldRect(list);
    for (i = 0; i < 5; i++) {
        bubble = xuiMessageListGetBubbleRect(list, i);
        body_x[i] = (int)(world.fX + content.fX + bubble.fX + metrics.fBubblePaddingX);
        body_y[i] = (int)(world.fY + content.fY + bubble.fY + metrics.fBubblePaddingY + 8);
    }
    drag_selection(context, body_x[0] + 2, body_y[0], body_x[3] + 90, body_y[3]);
    CHECK(xuiMessageListGetSelectedText(list, forward, sizeof(forward)) > 1);
    CHECK(strstr(forward, "Alpha") != NULL && strstr(forward, "middle plain") != NULL);
    CHECK(strstr(forward, "Gamma") != NULL && strstr(forward, "skip system") == NULL);
    CHECK(strstr(forward, "fallback") == NULL);
    CHECK(xuiMessageListGetNodeDocumentSelection(list, "first", &range) == XUI_OK);
    CHECK(range.tAnchor.iNodeId != 0 && range.tCaret.iNodeId != 0);
    CHECK(xuiMessageListGetNodeDocumentSelection(list, "last", &range) == XUI_OK);
    CHECK(range.tAnchor.iNodeId != 0 && range.tCaret.iNodeId != 0);
    CHECK(xuiTestSurfaceCreate(&proxy, &surface, 480, 600, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiRender(context, surface, &damage, 1) == XUI_OK);
    cache = xuiWidgetGetCacheSurface(list, xuiWidgetGetStateId(list));
    CHECK(cache != NULL);
    CHECK(xuiTestSurfaceGetRectFillColorCount(cache, first_selection_color) > 0);
    CHECK(xuiTestSurfaceGetRectFillColorCount(cache, last_selection_color) > 0);
    CHECK(xuiMessageListCopySelection(list) == XUI_OK);
    CHECK(!strcmp(xuiTestProxyGetClipboardText(&proxy), forward));
	{
		xui_doc_position_t hit;
		xui_document_transaction transaction = NULL;
		CHECK(xuiMessageListHitNodeDocument(list, "first", body_x[0] + 2,
			body_y[0], &hit) == XUI_OK && hit.iKind == XUI_DOC_POSITION_TEXT);
		CHECK(xuiDocumentBeginTransaction(first, NULL, &transaction) == XUI_OK);
		CHECK(xuiDocumentTxnReplaceText(transaction, hit.iNodeId, 6, 11, "next", 4) == XUI_OK);
		CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
		xuiDocumentTxnRelease(transaction);
		CHECK(xuiMessageListGetSelectedText(list, forward, sizeof(forward)) > 1);
		CHECK(strstr(forward, "Alpha next line.") != NULL);
	}

    drag_selection(context, body_x[3] + 90, body_y[3], body_x[0] + 2, body_y[0]);
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) > 1);
    CHECK(!strcmp(selected, forward));
    drag_selection(context, body_x[0] + 2, body_y[0], body_x[1] + 45, body_y[1]);
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) > 1);
    CHECK(strstr(selected, "Alpha") != NULL && strstr(selected, "midd") != NULL);
    CHECK(strstr(selected, "Gamma") == NULL);
    drag_selection(context, body_x[0] + 2, body_y[0],
        (int)(world.fX + content.fX + 4), body_y[1]);
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) > 1);
    CHECK(strstr(selected, "middle plain") != NULL);
    drag_selection(context, body_x[1] + 35, body_y[1], body_x[3] + 90, body_y[3]);
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) > 1);
    CHECK(strstr(selected, "plain") != NULL && strstr(selected, "Gamma") != NULL);
    CHECK(strstr(selected, "Alpha") == NULL && strstr(selected, "skip system") == NULL);
    drag_selection(context, body_x[0] + 2, body_y[0], body_x[4] + 45, body_y[4]);
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) > 1);
    CHECK(strstr(selected, "Alpha next") != NULL && strstr(selected, "middle plain") != NULL);
    CHECK(strstr(selected, "Gamma final line.") != NULL && strstr(selected, "tail") != NULL);
    CHECK(xuiMessageListGetNodeDocumentSelection(list, "last", &range) == XUI_OK);
    CHECK(range.tAnchor.iNodeId == XUI_DOCUMENT_ROOT && range.tAnchor.iOffset == 0);
    CHECK(range.tCaret.iNodeId == XUI_DOCUMENT_ROOT && range.tCaret.iOffset > 0);
    CHECK(xuiMessageListClearTextSelection(list) == XUI_OK);
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) == 0);
    CHECK(xuiMessageListGetNodeDocumentSelection(list, "first", &range) == XUI_ERROR_NOT_FOUND);
    drag_selection(context, body_x[0] + 2, body_y[0], body_x[3] + 90, body_y[3]);
    CHECK(xuiMessageListSetNodeDocument(list, "first", NULL) == XUI_OK);
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) == 0);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, surface);
    xuiDocumentRelease(first);
    xuiDocumentRelease(last);
    xuiDestroy(context);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
}

static void test_message_long_paragraph_continuation(int inline_image)
{
    static const char pattern[] = "Alpha beta gamma. ";
    const size_t unit = sizeof(pattern) - 1, bytes = unit * 5000;
    char* text = malloc(bytes + 1);
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_widget list = NULL;
    xui_font font = NULL, larger_font = NULL;
    xui_surface target = NULL, cache = NULL;
    xui_document document = NULL;
    xui_resource image_resource = NULL;
    xui_document_transaction txn = NULL;
    xui_doc_node_desc_t node = {0};
    xui_message_document_desc_t binding = {0};
    xui_message_list_desc_t list_desc = {0};
    xui_message_node_t message = {0};
    xui_message_list_metrics_t metrics = {0};
    xui_rect_t bubble, content, world;
    xui_doc_position_t first_hit, second_hit, reading, after_change;
    uint64_t paragraph, text_id, image_id, first_shape, second_shape, third_shape;
    uint64_t font_offset, dpi_offset, restored_offset;
    uint64_t narrow_offset, wide_offset, font_restored_offset;
    uint64_t shape_before_resize, resize_work;
    xui_doc_renderer_stats_t first_stats, second_stats, third_stats, font_stats, resize_stats;
    xui_rect_i_t damage = {0, 0, 480, 190};
    float body_x; size_t i;
    CHECK(text != NULL);
    for (i = 0; i < bytes; i += unit) memcpy(text + i, pattern, unit);
    text[bytes] = 0;
    xuiTestProxyInit(&proxy);
    message_base_shape = proxy.tProxy.textShape;
    proxy.tProxy.textShape = message_count_shape;
    message_shaped_bytes = 0;
    CHECK(xuiCreate(&context) == XUI_OK &&
        xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadMemory(&proxy.tProxy, &font, NULL, 0, 16, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK &&
        xuiInputViewport(context, 480, 190) == XUI_OK);
    if (inline_image)
        message_image_resource_set(context, &proxy, 40, 40, &image_resource);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(txn, XUI_DOCUMENT_ROOT,
        XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
    if (inline_image) {
        memset(&node, 0, sizeof(node));
        node.iSize = sizeof(node); node.iKind = XUI_DOC_IMAGE;
        node.sText = "alt"; node.iTextBytes = 3;
        node.sResource = "doc.message.image";
        CHECK(xuiDocumentTxnInsertNode(txn, paragraph,
            XUI_DOCUMENT_APPEND, &node, &image_id) == XUI_OK);
    }
    memset(&node, 0, sizeof(node));
    node.iKind = XUI_DOC_TEXT; node.sText = text; node.iTextBytes = bytes;
    node.iSize = sizeof(node);
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph,
        XUI_DOCUMENT_APPEND, &node, &text_id) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    message.iSize = sizeof(message); message.iType = XUI_MESSAGE_NODE_OTHER;
    message.sId = "long"; message.sSender = "Agent"; message.sText = "fallback";
    list_desc.iSize = sizeof(list_desc); list_desc.arrNodes = &message;
    list_desc.iNodeCount = 1;
    CHECK(xuiMessageListCreate(context, &list, &list_desc) == XUI_OK);
    CHECK(xuiSetRootWidget(context, list) == XUI_OK);
    CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 480, 190}) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&proxy, &target, 480, 190,
        XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiMessageListSetAutoScroll(list, 0) == XUI_OK);
    binding.iSize = sizeof(binding); binding.pDocument = document;
    memset(&first_stats, 0, sizeof(first_stats));
    first_stats.iSize = sizeof(first_stats); first_stats.iShapedBytes = 42;
    second_stats = first_stats;
    CHECK(xuiMessageListGetNodeDocumentRenderStats(NULL, "long", &first_stats) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiMessageListGetNodeDocumentRenderStats(list, NULL, &first_stats) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiMessageListGetNodeDocumentRenderStats(list, "long", NULL) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiMessageListGetNodeDocumentRenderStats(list, "missing", &first_stats) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiMessageListGetNodeDocumentRenderStats(list, "long", &first_stats) == XUI_ERROR_NOT_FOUND);
    CHECK(!memcmp(&first_stats, &second_stats, sizeof(first_stats)));
    CHECK(xuiMessageListSetNodeDocument(list, "long", &binding) == XUI_OK);
    first_stats.iSize--;
    CHECK(xuiMessageListGetNodeDocumentRenderStats(list, "long", &first_stats) == XUI_ERROR_INVALID_ARGUMENT &&
        first_stats.iShapedBytes == 42);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    first_shape = message_shaped_bytes;
    CHECK(first_shape < bytes / 4);
    first_stats = message_render_stats(list);
    CHECK(first_stats.iBlocks > 0 && first_stats.iTextRunBytes > 0 && first_stats.iTextRunBytes < bytes / 4);
    CHECK(xuiMessageListGetMetrics(list, &metrics) == XUI_OK);
    bubble = xuiMessageListGetBubbleRect(list, 0);
    content = xuiWidgetGetContentRect(list); world = xuiWidgetGetWorldRect(list);
    body_x = world.fX + content.fX + bubble.fX + metrics.fBubblePaddingX + 2;
    CHECK(xuiMessageListSetScroll(list, 5000) == XUI_OK &&
        xuiMessageListGetScroll(list) > 4500);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
    second_shape = message_shaped_bytes;
    second_stats = message_render_stats(list);
    printf("MessageList first continuation (%s): source=%llu initial-shaped=%llu growth-shaped=%llu\n",
        inline_image ? "inline image" : "text", (unsigned long long)bytes,
        (unsigned long long)first_shape, (unsigned long long)(second_shape - first_shape));
    CHECK(second_shape > first_shape &&
        second_stats.iTextRunBytes > first_stats.iTextRunBytes &&
        second_stats.iTextRunBytes - first_stats.iTextRunBytes < bytes / 2 &&
        second_shape - first_shape < (second_stats.iTextRunBytes - first_stats.iTextRunBytes) * 4);
    CHECK(xuiMessageListHitNodeDocument(list, "long", body_x, 100, &first_hit) == XUI_OK &&
        first_hit.iNodeId == text_id && first_hit.iOffset > 0);
    CHECK(xuiRender(context, target, &damage, 1) == XUI_OK);
    cache = xuiWidgetGetCacheSurface(list, xuiWidgetGetStateId(list));
    CHECK(cache != NULL && xuiTestSurfaceGetTextDrawCount(cache) > 0);
    CHECK(xuiMessageListSetScroll(list, 16000) == XUI_OK &&
        xuiMessageListGetScroll(list) > 15000);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
    third_shape = message_shaped_bytes;
    third_stats = message_render_stats(list);
    printf("MessageList second continuation (%s): growth-shaped=%llu\n",
        inline_image ? "inline image" : "text", (unsigned long long)(third_shape - second_shape));
    CHECK(third_shape > second_shape &&
        third_stats.iTextRunBytes > second_stats.iTextRunBytes &&
        third_stats.iTextRunBytes - second_stats.iTextRunBytes < bytes / 2 &&
        third_shape - second_shape < (third_stats.iTextRunBytes - second_stats.iTextRunBytes) * 4);
    CHECK(xuiMessageListHitNodeDocument(list, "long", body_x, 100, &second_hit) == XUI_OK &&
        second_hit.iNodeId == text_id && second_hit.iOffset > first_hit.iOffset);
    CHECK(xuiRender(context, target, &damage, 1) == XUI_OK);
    CHECK(xuiTestSurfaceGetTextDrawCount(cache) > 0);
    CHECK(xuiMessageListHitNodeDocument(list, "long", body_x + 43, 20,
        &reading) == XUI_OK && reading.iNodeId == text_id);
    CHECK(proxy.tProxy.fontLoadMemory(&proxy.tProxy, &larger_font,
        NULL, 0, 21, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, larger_font) == XUI_OK);
    CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
    CHECK(xuiMessageListHitNodeDocument(list, "long", body_x + 43, 20,
        &after_change) == XUI_OK && after_change.iNodeId == text_id &&
        after_change.iOffset + 64 > reading.iOffset &&
        after_change.iOffset < reading.iOffset + 64);
    font_offset = after_change.iOffset;
    font_stats = message_render_stats(list);
    /* Replacing the context default font recreates this bound renderer; its
     * counters belong to the new instance rather than the previous one. */
    printf("MessageList font replacement (%s): new-renderer-input=%llu growth-shaped=%llu\n",
        inline_image ? "inline image" : "text", (unsigned long long)font_stats.iTextRunBytes,
        (unsigned long long)(message_shaped_bytes - third_shape));
    CHECK(font_stats.iTextRunBytes > 0 && font_stats.iTextRunBytes < bytes &&
        message_shaped_bytes - third_shape < font_stats.iTextRunBytes * 4);
    CHECK(xuiSetVirtualDpi(context, 1.5f) == XUI_OK);
    CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
    CHECK(xuiMessageListHitNodeDocument(list, "long", body_x + 43, 20,
        &after_change) == XUI_OK && after_change.iNodeId == text_id &&
        after_change.iOffset + 64 > reading.iOffset &&
        after_change.iOffset < reading.iOffset + 64);
    dpi_offset = after_change.iOffset;
    CHECK(xuiSetVirtualDpi(context, 1.0f) == XUI_OK);
    CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
    CHECK(xuiMessageListHitNodeDocument(list, "long", body_x + 43, 20,
        &after_change) == XUI_OK && after_change.iNodeId == text_id &&
        after_change.iOffset + 64 > reading.iOffset &&
        after_change.iOffset < reading.iOffset + 64);
    restored_offset = after_change.iOffset;
    CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 360, 190}) == XUI_OK);
    CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
    CHECK(xuiMessageListHitNodeDocument(list, "long", body_x + 43, 20,
        &after_change) == XUI_OK && after_change.iNodeId == text_id &&
        after_change.iOffset + 64 > reading.iOffset &&
        after_change.iOffset < reading.iOffset + 64);
    narrow_offset = after_change.iOffset;
    CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 480, 190}) == XUI_OK);
    CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
    CHECK(xuiMessageListHitNodeDocument(list, "long", body_x + 43, 20,
        &after_change) == XUI_OK && after_change.iNodeId == text_id &&
        after_change.iOffset + 64 > reading.iOffset &&
        after_change.iOffset < reading.iOffset + 64);
    wide_offset = after_change.iOffset;
    shape_before_resize = message_shaped_bytes;
    resize_stats = message_render_stats(list);
    for (i = 0; i < 8; i++) {
        CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 360, 190}) == XUI_OK);
        CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
        CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 480, 190}) == XUI_OK);
        CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
        CHECK(xuiMessageListHitNodeDocument(list, "long", body_x + 43, 20,
            &after_change) == XUI_OK && after_change.iNodeId == text_id);
        wide_offset = after_change.iOffset;
    }
    resize_work = message_shaped_bytes - shape_before_resize;
    printf("MessageList repeated-width anchor: base=%llu after=%llu shaped=%llu\n",
        (unsigned long long)reading.iOffset, (unsigned long long)wide_offset,
        (unsigned long long)resize_work);
    CHECK(wide_offset + 64 > reading.iOffset &&
        wide_offset < reading.iOffset + 64);
    CHECK(resize_work < bytes * 16);
    CHECK(message_render_stats(list).iTextRunBytes == resize_stats.iTextRunBytes);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
    CHECK(xuiMessageListHitNodeDocument(list, "long", body_x + 43, 20,
        &after_change) == XUI_OK && after_change.iNodeId == text_id &&
        after_change.iOffset + 64 > reading.iOffset &&
        after_change.iOffset < reading.iOffset + 64);
    font_restored_offset = after_change.iOffset;
    if (inline_image) {
        uint64_t revision = xuiDocumentGetRevision(document);
        uint64_t shape_before_image = message_shaped_bytes;
        xui_doc_renderer_stats_t image_stats = message_render_stats(list);
        float scroll_before = xuiMessageListGetScroll(list);
        message_image_resource_set(context, &proxy, 40, 140,
            &image_resource);
        CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
        CHECK(xuiMessageListGetScroll(list) > scroll_before + 50);
        CHECK(xuiMessageListHitNodeDocument(list, "long", body_x + 43, 20,
            &after_change) == XUI_OK && after_change.iNodeId == text_id &&
            after_change.iOffset + 64 > reading.iOffset &&
            after_change.iOffset < reading.iOffset + 64);
        scroll_before = xuiMessageListGetScroll(list);
        message_image_resource_set(context, &proxy, 40, 20,
            &image_resource);
        CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
        CHECK(xuiMessageListGetScroll(list) < scroll_before - 50);
        CHECK(xuiMessageListHitNodeDocument(list, "long", body_x + 43, 20,
            &after_change) == XUI_OK && after_change.iNodeId == text_id &&
            after_change.iOffset + 64 > reading.iOffset &&
            after_change.iOffset < reading.iOffset + 64);
        CHECK(xuiDocumentGetRevision(document) == revision);
        CHECK(message_shaped_bytes - shape_before_image < bytes * 2);
        CHECK(message_render_stats(list).iTextRunBytes == image_stats.iTextRunBytes);
        printf("MessageList long paragraph inline image: shape-growth=%llu\n",
            (unsigned long long)(message_shaped_bytes - shape_before_image));
    }
    {
        message_font_switch switcher = {0};
        xui_doc_position_t callback_reading, callback_after;
        uint64_t revision = xuiDocumentGetRevision(document);
        switcher.current = font;
        binding.tRenderer.iSize = sizeof(binding.tRenderer);
        binding.tRenderer.onFont = message_select_font;
        binding.tRenderer.pUser = &switcher;
        CHECK(xuiMessageListSetNodeDocument(list, "long", &binding) == XUI_OK);
        CHECK(xuiMessageListSetScroll(list, 16000) == XUI_OK);
        CHECK(xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiMessageListHitNodeDocument(list, "long", body_x + 43, 20,
            &callback_reading) == XUI_OK && callback_reading.iNodeId == text_id);
        switcher.current = larger_font;
        CHECK(xuiMessageListInvalidateNodeDocumentFonts(list, "long") == XUI_OK);
        CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
        CHECK(xuiMessageListHitNodeDocument(list, "long", body_x + 43, 20,
            &callback_after) == XUI_OK && callback_after.iNodeId == text_id &&
            callback_after.iOffset + 64 > callback_reading.iOffset &&
            callback_after.iOffset < callback_reading.iOffset + 64);
        switcher.current = font;
        CHECK(xuiMessageListInvalidateNodeDocumentFonts(list, "long") == XUI_OK);
        CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
        CHECK(xuiMessageListHitNodeDocument(list, "long", body_x + 43, 20,
            &callback_after) == XUI_OK && callback_after.iNodeId == text_id &&
            callback_after.iOffset + 64 > callback_reading.iOffset &&
            callback_after.iOffset < callback_reading.iOffset + 64);
        CHECK(xuiDocumentGetRevision(document) == revision);
        printf("MessageList long paragraph callback-font anchor (%s): base=%llu restored=%llu\n",
            inline_image ? "inline image" : "text",
            (unsigned long long)callback_reading.iOffset,
            (unsigned long long)callback_after.iOffset);
    }
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    CHECK(xuiMessageListSetNodeDocument(list, "long", NULL) == XUI_OK);
    CHECK(xuiMessageListGetNodeDocumentRenderStats(list, "long", &first_stats) == XUI_ERROR_NOT_FOUND);
    xuiDestroy(context);
    xuiDocumentRelease(document);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    proxy.tProxy.fontDestroy(&proxy.tProxy, larger_font);
    free(text);
    printf("MessageList long paragraph: initial=%llu first_extend=%llu second_extend=%llu, hit advances within one message\n",
        (unsigned long long)first_shape,
        (unsigned long long)(second_shape - first_shape),
        (unsigned long long)(third_shape - second_shape));
    printf("MessageList new run inputs: initial=%llu first_extend=%llu second_extend=%llu; statistics reads do not shape\n",
        (unsigned long long)first_stats.iTextRunBytes,
        (unsigned long long)(second_stats.iTextRunBytes - first_stats.iTextRunBytes),
        (unsigned long long)(third_stats.iTextRunBytes - second_stats.iTextRunBytes));
    printf("MessageList long paragraph anchor (%s): base=%llu font=%llu dpi=%llu restored=%llu narrow=%llu wide=%llu font-restored=%llu\n",
        inline_image ? "inline image" : "text",
        (unsigned long long)reading.iOffset, (unsigned long long)font_offset,
        (unsigned long long)dpi_offset, (unsigned long long)restored_offset,
        (unsigned long long)narrow_offset, (unsigned long long)wide_offset,
        (unsigned long long)font_restored_offset);
}

typedef struct message_accessibility_events_t {
    xui_widget list;
    int tree;
    int selection;
    int state;
    int value;
    int bounds;
} message_accessibility_events_t;

static void message_accessibility_event(xui_context context, xui_widget widget,
    const xui_accessibility_event_t* event, void* user)
{
    message_accessibility_events_t* events = (message_accessibility_events_t*)user;
    (void)context;
    if (widget != events->list) return;
    if (event->iType == XUI_ACCESSIBLE_EVENT_TREE_CHANGED) events->tree++;
    if (event->iType == XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED) events->selection++;
    if (event->iType == XUI_ACCESSIBLE_EVENT_STATE_CHANGED) events->state++;
    if (event->iType == XUI_ACCESSIBLE_EVENT_VALUE_CHANGED) events->value++;
    if (event->iType == XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED) events->bounds++;
}

static int message_accessible_index_by_id(xui_widget list, uint64_t id)
{
    int i, count = xuiWidgetGetAccessibleNodeCount(list);
    for (i = 0; i < count; i++) {
        xui_accessible_node_t node = {0};
        node.iSize = sizeof(node);
        if (xuiWidgetGetAccessibleNode(list, i, &node) != XUI_OK) return -1;
        if (node.iId == id) return i;
    }
    return -1;
}

static xui_accessible_node_t message_accessible_by_role_value(xui_widget widget,
    int role, const char* value)
{
    xui_accessible_node_t node = {0};
    int i, count = xuiWidgetGetAccessibleNodeCount(widget);
    for (i = 0; i < count; i++) {
        node.iSize = sizeof(node);
        CHECK(xuiWidgetGetAccessibleNode(widget, i, &node) == XUI_OK);
        if (node.iRole == role && node.sValue && !strcmp(node.sValue, value))
            return node;
    }
    fprintf(stderr, "missing accessible role %d value %s\n", role, value);
    exit(1);
}

static xui_accessible_node_t message_accessible_by_role_name(xui_widget widget,
    int role, const char* name)
{
    xui_accessible_node_t node = {0};
    int i, count = xuiWidgetGetAccessibleNodeCount(widget);
    for (i = 0; i < count; i++) {
        node.iSize = sizeof(node);
        CHECK(xuiWidgetGetAccessibleNode(widget, i, &node) == XUI_OK);
        if (node.iRole == role && node.sName && !strcmp(node.sName, name))
            return node;
    }
    fprintf(stderr, "missing accessible role %d name %s\n", role, name);
    exit(1);
}

static void test_message_structured_accessibility(void)
{
    static const char markdown[] =
        "| A | B |\n| --- | --- |\n| c | 你 |\n\n![alt](/img)\n";
    static const char changed_markdown[] =
        "| A | B |\n| --- | --- |\n| c | 我 |\n\n![alt](/img)\n";
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_document document = NULL;
    xui_widget list = NULL, view = NULL;
    xui_surface surface = NULL;
    xui_font font = NULL;
    xui_doc_desc_t doc_desc = {0};
    xui_doc_view_desc_t view_desc = {0};
    xui_message_document_desc_t binding = {0};
    xui_message_list_desc_t list_desc = {0};
    xui_message_node_t message = {0};
    xui_accessible_node_t table, row, cell, first_cell, image, direct;
    xui_accessible_selection_t selection = {0};
    xui_doc_table_selection_t rectangle = {0};
    xui_event_t pointer = {0};
    xui_event_t key = {0};
    xui_rect_i_t damage = {0, 0, 300, 140};
    uint32_t selection_color = XUI_COLOR_RGBA(31, 95, 163, 140);
    char selected[32] = {0};
    uint64_t revision;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadMemory(&proxy.tProxy, &font, NULL, 0, 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiInputViewport(context, 300, 140) == XUI_OK);
    doc_desc.iSize = sizeof(doc_desc);
    doc_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    doc_desc.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&doc_desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, markdown, sizeof(markdown) - 1) == XUI_OK);
    revision = xuiDocumentGetRevision(document);
    message.iSize = sizeof(message); message.iType = XUI_MESSAGE_NODE_OTHER;
    message.sId = "table"; message.sText = "fallback";
    list_desc.iSize = sizeof(list_desc); list_desc.arrNodes = &message;
    list_desc.iNodeCount = 1;
    CHECK(xuiMessageListCreate(context, &list, &list_desc) == XUI_OK);
    CHECK(xuiSetRootWidget(context, list) == XUI_OK);
    CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 300, 140}) == XUI_OK);
    binding.iSize = sizeof(binding); binding.pDocument = document;
    binding.iSelectionColor = selection_color;
    CHECK(xuiMessageListSetNodeDocument(list, "table", &binding) == XUI_OK);
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 300, 140}) == XUI_OK);
    table = message_accessible_by_role_value(list, XUI_ACCESSIBLE_ROLE_TABLE,
        "A\tB\nc\t你");
    row = message_accessible_by_role_value(list, XUI_ACCESSIBLE_ROLE_GROUP,
        "c\t你");
    cell = message_accessible_by_role_value(list, XUI_ACCESSIBLE_ROLE_CELL,
        "你");
    CHECK(table.iParentId != 0 && row.iParentId == table.iId &&
        cell.iParentId == row.iId && cell.iRow == 1 && cell.iColumn == 1);
    direct = message_accessible_by_role_value(view, XUI_ACCESSIBLE_ROLE_TABLE,
        "A\tB\nc\t你");
    CHECK(direct.iRole == table.iRole && !strcmp(direct.sValue, table.sValue));
    direct = message_accessible_by_role_value(view, XUI_ACCESSIBLE_ROLE_CELL,
        "你");
    CHECK(direct.iRole == cell.iRole && !strcmp(direct.sValue, cell.sValue));
    CHECK((table.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)) &&
        (row.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)) &&
        (cell.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
    selection.iSize = sizeof(selection); selection.iAnchor = 3; selection.iCaret = 0;
    CHECK(xuiWidgetPerformAccessibleAction(list, cell.iId,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_OK);
    cell.iSize = sizeof(cell);
    CHECK(xuiWidgetGetAccessibleNode(list,
        message_accessible_index_by_id(list, cell.iId), &cell) == XUI_OK &&
        cell.iTextStart == 3 && cell.iTextEnd == 0 &&
        (cell.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) == 4 &&
        !strcmp(selected, "你"));
    selection.iAnchor = 1; selection.iCaret = 3;
    CHECK(xuiWidgetPerformAccessibleAction(list, cell.iId,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_DOC_ERROR_UTF8);
    selection.iAnchor = 9; selection.iCaret = 4;
    CHECK(xuiWidgetPerformAccessibleAction(list, table.iId,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_OK);
    table.iSize = row.iSize = sizeof(table);
    CHECK(xuiWidgetGetAccessibleNode(list,
        message_accessible_index_by_id(list, table.iId), &table) == XUI_OK &&
        table.iTextStart == 9 && table.iTextEnd == 4 &&
        (table.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    CHECK(xuiWidgetGetAccessibleNode(list,
        message_accessible_index_by_id(list, row.iId), &row) == XUI_OK &&
        row.iTextStart == 5 && row.iTextEnd == 0 &&
        (row.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) == 6 &&
        !strcmp(selected, "c\t你"));
    CHECK(xuiWidgetPerformAccessibleAction(list, cell.iId,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "table", &rectangle) == XUI_OK &&
        rectangle.iRow == 1 && rectangle.iColumn == 1 &&
        rectangle.iRows == 1 && rectangle.iColumns == 1);
    cell.iSize = sizeof(cell);
    CHECK(xuiWidgetGetAccessibleNode(list,
        message_accessible_index_by_id(list, cell.iId), &cell) == XUI_OK &&
        (cell.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) == 4 &&
        !strcmp(selected, "你"));
    CHECK(xuiMessageListCopySelection(list) == XUI_OK &&
        !strcmp(xuiTestProxyGetClipboardText(&proxy), "你"));
    rectangle.iRow = rectangle.iColumn = 0;
    rectangle.iRows = rectangle.iColumns = 2;
    CHECK(xuiMessageListSetNodeDocumentTableSelection(list, "table", &rectangle) == XUI_OK);
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) == 10 &&
        !strcmp(selected, "A\tB\nc\t你"));
    CHECK(xuiMessageListCopySelection(list) == XUI_OK &&
        !strcmp(xuiTestProxyGetClipboardText(&proxy), "A\tB\nc\t你"));
    {
        xui_doc_range_t text_range = {0};
        CHECK(xuiMessageListGetNodeDocumentSelection(list, "table", &text_range) ==
            XUI_ERROR_NOT_FOUND);
    }
    CHECK(xuiTestSurfaceCreate(&proxy, &surface, 300, 140,
        XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiUpdate(context, 0) == XUI_OK);
    CHECK(xuiRender(context, surface, &damage, 1) == XUI_OK);
    CHECK(xuiTestSurfaceGetRectFillColorCount(
        xuiWidgetGetCacheSurface(list, xuiWidgetGetStateId(list)),
        selection_color) >= 4);
    rectangle.iRow = 99;
    CHECK(xuiMessageListSetNodeDocumentTableSelection(list, "table", &rectangle) != XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "table", &rectangle) == XUI_OK &&
        rectangle.iRow == 0 && rectangle.iColumn == 0 &&
        rectangle.iRows == 2 && rectangle.iColumns == 2);
    CHECK(xuiMessageListClearTextSelection(list) == XUI_OK);
    first_cell = message_accessible_by_role_value(list, XUI_ACCESSIBLE_ROLE_CELL, "A");
    cell = message_accessible_by_role_value(list, XUI_ACCESSIBLE_ROLE_CELL, "你");
    pointer.iSize = sizeof(pointer); pointer.pTarget = list;
    pointer.iPointerId = 71; pointer.iPointerType = XUI_POINTER_TYPE_MOUSE;
    pointer.iModifiers = XUI_MOD_ALT; pointer.iButton = pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
    pointer.fX = first_cell.tBounds.fX + first_cell.tBounds.fW * 0.5f;
    pointer.fY = first_cell.tBounds.fY + first_cell.tBounds.fH * 0.5f;
    pointer.iType = XUI_EVENT_POINTER_DOWN;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    pointer.iType = XUI_EVENT_POINTER_MOVE; pointer.iButton = 0;
    pointer.fX = cell.tBounds.fX + cell.tBounds.fW * 0.5f;
    pointer.fY = cell.tBounds.fY + cell.tBounds.fH * 0.5f;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButton = XUI_POINTER_BUTTON_LEFT;
    pointer.iButtons = 0;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "table", &rectangle) == XUI_OK &&
        rectangle.iRow == 0 && rectangle.iColumn == 0 &&
        rectangle.iRows == 2 && rectangle.iColumns == 2);
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) == 10 &&
        !strcmp(selected, "A\tB\nc\t你"));
    pointer.iType = XUI_EVENT_POINTER_DOWN; pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
    pointer.fX = cell.tBounds.fX + cell.tBounds.fW * 0.5f;
    pointer.fY = cell.tBounds.fY + cell.tBounds.fH * 0.5f;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButtons = 0;
    pointer.fX = first_cell.tBounds.fX + first_cell.tBounds.fW * 0.5f;
    pointer.fY = first_cell.tBounds.fY + first_cell.tBounds.fH * 0.5f;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "table", &rectangle) == XUI_OK &&
        rectangle.iRow == 0 && rectangle.iColumn == 0 &&
        rectangle.iRows == 2 && rectangle.iColumns == 2);
    key.iSize = sizeof(key); key.pTarget = list;
    key.iType = XUI_EVENT_KEY_DOWN; key.iModifiers = XUI_MOD_ALT | XUI_MOD_SHIFT;
    key.iKey = XUI_KEY_RIGHT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "table", &rectangle) == XUI_OK &&
        rectangle.iRow == 0 && rectangle.iColumn == 1 &&
        rectangle.iRows == 2 && rectangle.iColumns == 1);
    pointer.iModifiers = 0; pointer.iType = XUI_EVENT_POINTER_DOWN;
    pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButtons = 0;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "table", &rectangle) ==
        XUI_ERROR_NOT_FOUND);
    key.iSize = sizeof(key); key.pTarget = list;
    key.iType = XUI_EVENT_KEY_DOWN; key.iModifiers = XUI_MOD_ALT | XUI_MOD_SHIFT;
    key.iKey = XUI_KEY_RIGHT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "table", &rectangle) == XUI_OK &&
        rectangle.iRow == 0 && rectangle.iColumn == 0 &&
        rectangle.iRows == 1 && rectangle.iColumns == 2);
    key.iKey = XUI_KEY_DOWN;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "table", &rectangle) == XUI_OK &&
        rectangle.iRow == 0 && rectangle.iColumn == 0 &&
        rectangle.iRows == 2 && rectangle.iColumns == 2);
    key.iKey = XUI_KEY_LEFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "table", &rectangle) == XUI_OK &&
        rectangle.iRow == 0 && rectangle.iColumn == 0 &&
        rectangle.iRows == 2 && rectangle.iColumns == 1);
    key.iModifiers = 0; key.iKey = XUI_KEY_ESCAPE;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "table", &rectangle) ==
        XUI_ERROR_NOT_FOUND);
    selection.iAnchor = 0; selection.iCaret = 1;
    CHECK(xuiWidgetPerformAccessibleAction(list, table.iId,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "table", &rectangle) == XUI_ERROR_NOT_FOUND);
    image = message_accessible_by_role_name(list, XUI_ACCESSIBLE_ROLE_IMAGE, "alt");
    CHECK((image.iState & XUI_ACCESSIBLE_STATE_SELECTABLE) &&
        (image.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
    CHECK(xuiWidgetPerformAccessibleAction(list, image.iId,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
    image.iSize = sizeof(image);
    CHECK(xuiWidgetGetAccessibleNode(list,
        message_accessible_index_by_id(list, image.iId), &image) == XUI_OK &&
        (image.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    {
        xui_doc_range_t object = {0};
        xui_document_snapshot snapshot = NULL;
        xui_doc_node_id image_node = 0;
        xui_doc_node_info_t info = {0};
        CHECK(xuiMessageListGetNodeDocumentSelection(list, "table", &object) == XUI_OK &&
            object.tAnchor.iNodeId == object.tCaret.iNodeId &&
            object.tAnchor.iKind == XUI_DOC_POSITION_GAP &&
            object.tCaret.iOffset == object.tAnchor.iOffset + 1);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, object.tAnchor.iNodeId,
            object.tAnchor.iOffset, &image_node) == XUI_OK);
        info.iSize = sizeof(info);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, image_node, &info) == XUI_OK &&
            info.iKind == XUI_DOC_IMAGE);
        xuiDocumentSnapshotRelease(snapshot);
    }
    CHECK(xuiDocumentGetRevision(document) == revision);
    rectangle.iSize = sizeof(rectangle);
    rectangle.iRow = rectangle.iColumn = 0;
    rectangle.iRows = rectangle.iColumns = 2;
    CHECK(xuiMessageListSetNodeDocumentTableSelection(list, "table", &rectangle) == XUI_OK);
    CHECK(xuiMessageListClearTextSelection(list) == XUI_OK);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "table", &rectangle) ==
        XUI_ERROR_NOT_FOUND);
    CHECK(xuiMessageListSetNodeDocumentTableSelection(list, "table", &rectangle) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, changed_markdown,
        sizeof(changed_markdown) - 1) == XUI_OK);
    CHECK(xuiDocumentGetRevision(document) > revision);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "table", &rectangle) ==
        XUI_ERROR_NOT_FOUND);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, surface);
    xuiWidgetDestroy(view);
    xuiDestroy(context);
    xuiDocumentRelease(document);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("MessageList structured accessibility: table rectangles, rendering, TSV copy and reset passed");
}

static void test_message_merged_table_selection(void)
{
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_document document = NULL;
    xui_document_transaction txn = NULL;
    xui_widget list = NULL;
    xui_font font = NULL;
    xui_doc_node_id table = 0, merged = 0;
    xui_message_node_t message = {0};
    xui_message_list_desc_t list_desc = {0};
    xui_message_document_desc_t binding = {0};
    xui_doc_table_selection_t rectangle = {0};
    xui_accessible_node_t cell = {0};
    xui_event_t pointer = {0};
    char selected[16] = {0};
    uint64_t revision;
    int i, count, found = 0;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadMemory(&proxy.tProxy, &font, NULL, 0, 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiInputViewport(context, 320, 180) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, XUI_DOCUMENT_ROOT, 0, 2, 3, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnMergeCells(txn, table, 0, 1, 2, 1, &merged) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    revision = xuiDocumentGetRevision(document);
    message.iSize = sizeof(message); message.iType = XUI_MESSAGE_NODE_OTHER;
    message.sId = "merged"; message.sText = "fallback";
    list_desc.iSize = sizeof(list_desc); list_desc.arrNodes = &message;
    list_desc.iNodeCount = 1;
    CHECK(xuiMessageListCreate(context, &list, &list_desc) == XUI_OK);
    CHECK(xuiSetRootWidget(context, list) == XUI_OK);
    CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 320, 180}) == XUI_OK);
    binding.iSize = sizeof(binding); binding.pDocument = document;
    CHECK(xuiMessageListSetNodeDocument(list, "merged", &binding) == XUI_OK);
    rectangle.iSize = sizeof(rectangle); rectangle.iTableId = table;
    rectangle.iRow = 1; rectangle.iColumn = 1;
    rectangle.iRows = rectangle.iColumns = 1;
    CHECK(xuiMessageListSetNodeDocumentTableSelection(list, "merged", &rectangle) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "merged", &rectangle) == XUI_OK &&
        rectangle.iRow == 0 && rectangle.iColumn == 1 &&
        rectangle.iRows == 2 && rectangle.iColumns == 1);
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) == 5 &&
        !strcmp(selected, "\"\n\"\n"));
    {
        xui_event_t key = {0};
        key.iSize = sizeof(key); key.pTarget = list;
        key.iType = XUI_EVENT_KEY_DOWN;
        key.iModifiers = XUI_MOD_ALT | XUI_MOD_SHIFT;
        key.iKey = XUI_KEY_RIGHT;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        rectangle.iSize = sizeof(rectangle);
        CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "merged", &rectangle) == XUI_OK &&
            rectangle.iRow == 0 && rectangle.iColumn == 1 &&
            rectangle.iRows == 2 && rectangle.iColumns == 2);
        key.iKey = XUI_KEY_LEFT;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        rectangle.iSize = sizeof(rectangle);
        CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "merged", &rectangle) == XUI_OK &&
            rectangle.iRow == 0 && rectangle.iColumn == 1 &&
            rectangle.iRows == 2 && rectangle.iColumns == 1);
    }
    count = xuiWidgetGetAccessibleNodeCount(list);
    for (i = 0; i < count; i++) {
        cell.iSize = sizeof(cell);
        CHECK(xuiWidgetGetAccessibleNode(list, i, &cell) == XUI_OK);
        if (cell.iRole == XUI_ACCESSIBLE_ROLE_CELL && cell.iRowCount == 2) {
            found = 1;
            break;
        }
    }
    CHECK(found && cell.iRow == 0 && cell.iColumn == 1 &&
        cell.iColumnCount == 1 && (cell.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    CHECK(xuiMessageListClearTextSelection(list) == XUI_OK);
    CHECK(xuiWidgetPerformAccessibleAction(list, cell.iId,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "merged", &rectangle) == XUI_OK &&
        rectangle.iRow == 0 && rectangle.iColumn == 1 &&
        rectangle.iRows == 2 && rectangle.iColumns == 1);
    CHECK(xuiMessageListClearTextSelection(list) == XUI_OK);
    pointer.iSize = sizeof(pointer); pointer.pTarget = list;
    pointer.iPointerId = 72; pointer.iPointerType = XUI_POINTER_TYPE_MOUSE;
    pointer.iModifiers = XUI_MOD_ALT; pointer.iButton = pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
    pointer.fX = cell.tBounds.fX + cell.tBounds.fW * 0.5f;
    pointer.fY = cell.tBounds.fY + cell.tBounds.fH * 0.5f;
    pointer.iType = XUI_EVENT_POINTER_DOWN;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButtons = 0;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "merged", &rectangle) == XUI_OK &&
        rectangle.iRow == 0 && rectangle.iColumn == 1 &&
        rectangle.iRows == 2 && rectangle.iColumns == 1);
    CHECK(xuiInputViewport(context, 320, 60) == XUI_OK);
    CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 320, 60}) == XUI_OK);
    CHECK(xuiUpdate(context, 0) == XUI_OK);
    rectangle.iSize = sizeof(rectangle); rectangle.iTableId = table;
    rectangle.iRow = rectangle.iColumn = 0;
    rectangle.iRows = rectangle.iColumns = 1;
    CHECK(xuiMessageListSetNodeDocumentTableSelection(list, "merged", &rectangle) == XUI_OK);
    CHECK(xuiMessageListSetScroll(list, 0) == XUI_OK);
    {
        xui_event_t key = {0};
        key.iSize = sizeof(key); key.pTarget = list;
        key.iType = XUI_EVENT_KEY_DOWN;
        key.iModifiers = XUI_MOD_ALT | XUI_MOD_SHIFT;
        key.iKey = XUI_KEY_DOWN;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        rectangle.iSize = sizeof(rectangle);
        CHECK(xuiMessageListGetNodeDocumentTableSelection(list, "merged", &rectangle) == XUI_OK &&
            rectangle.iRow == 0 && rectangle.iColumn == 0 &&
            rectangle.iRows == 2 && rectangle.iColumns == 1);
        CHECK(xuiMessageListGetScroll(list) > 0);
    }
    CHECK(xuiInputViewport(context, 320, 180) == XUI_OK);
    CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 320, 180}) == XUI_OK);
    CHECK(xuiMessageListSetScroll(list, 0) == XUI_OK);
    CHECK(xuiDocumentGetRevision(document) == revision);
    xuiDestroy(context);
    xuiDocumentRelease(document);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("MessageList merged table selection: partial grid, accessibility and Alt click passed");
}

static void test_message_accessibility(void)
{
    xui_test_proxy_state_t proxy;
    xui_context context = NULL; xui_widget list = NULL;
    xui_font font = NULL; xui_document document = NULL;
    xui_doc_desc_t doc_desc = {0}; xui_message_document_desc_t binding = {0};
    xui_doc_view_desc_t view_desc = {0}; xui_widget view = NULL;
    xui_message_list_desc_t desc = {0}; xui_message_node_t messages[3] = {{0}};
    xui_accessible_node_t root = {0}, first = {0}, second = {0}, auxiliary = {0};
    xui_accessible_node_t document_root = {0}, paragraph = {0}, leaf = {0};
    xui_accessible_selection_t selection = {0};
    char selected[32] = {0};
    message_accessibility_events_t events = {0};
    xui_rect_t bubble, world, content;
    int before, before_bounds;
    uint64_t first_id, second_id, auxiliary_id, document_root_id;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadMemory(&proxy.tProxy, &font, NULL, 0, 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiInputViewport(context, 240, 76) == XUI_OK);
    messages[0].iSize = sizeof(messages[0]); messages[0].iType = XUI_MESSAGE_NODE_SELF;
    messages[0].sId = "first"; messages[0].sSender = "User";
    messages[0].sText = "hello";
    messages[1].iSize = sizeof(messages[1]); messages[1].iType = XUI_MESSAGE_NODE_OTHER;
    messages[1].sId = "answer"; messages[1].sSender = "Agent";
    messages[1].sText = "fallback";
    messages[2].iSize = sizeof(messages[2]); messages[2].iType = XUI_MESSAGE_NODE_AUXILIARY;
    messages[2].iFlags = XUI_MESSAGE_NODE_FLAG_COLLAPSIBLE | XUI_MESSAGE_NODE_FLAG_COLLAPSED;
    messages[2].sId = "tool"; messages[2].sTitle = "Tool run";
    messages[2].sText = "details";
    desc.iSize = sizeof(desc); desc.arrNodes = messages; desc.iNodeCount = 3;
    CHECK(xuiMessageListCreate(context, &list, &desc) == XUI_OK);
    CHECK(xuiSetRootWidget(context, list) == XUI_OK);
    CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 240, 76}) == XUI_OK);
    CHECK(xuiWidgetSetAccessibleName(list, "Conversation") == XUI_OK);
    CHECK(xuiWidgetGetAccessibleNodeCount(list) == 4);
    root.iSize = first.iSize = second.iSize = auxiliary.iSize = sizeof(root);
    CHECK(xuiWidgetGetAccessibleNode(list, 0, &root) == XUI_OK &&
        root.iRole == XUI_ACCESSIBLE_ROLE_LIST && !strcmp(root.sName, "Conversation"));
    CHECK(xuiWidgetGetAccessibleNode(list, 1, &first) == XUI_OK &&
        first.iRole == XUI_ACCESSIBLE_ROLE_LIST_ITEM && first.iParentId == root.iId &&
        !strcmp(first.sName, "User") && !strcmp(first.sValue, "hello"));
    CHECK(xuiWidgetGetAccessibleNode(list, 2, &second) == XUI_OK &&
        second.iParentId == root.iId && !strcmp(second.sName, "Agent") &&
        !strcmp(second.sValue, "fallback"));
    CHECK(xuiWidgetGetAccessibleNode(list, 3, &auxiliary) == XUI_OK &&
        auxiliary.iRole == XUI_ACCESSIBLE_ROLE_GROUP &&
        auxiliary.iParentId == root.iId && !strcmp(auxiliary.sName, "Tool run") &&
        (auxiliary.iState & XUI_ACCESSIBLE_STATE_COLLAPSED) &&
        (auxiliary.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN) &&
        (auxiliary.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_EXPAND)));
    first_id = first.iId; second_id = second.iId; auxiliary_id = auxiliary.iId;
    CHECK(first_id > root.iId && second_id > first_id && auxiliary_id > second_id);
    doc_desc.iSize = sizeof(doc_desc); doc_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&doc_desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, "**answer**\n", 11) == XUI_OK);
    binding.iSize = sizeof(binding); binding.pDocument = document;
    CHECK(xuiMessageListSetNodeDocument(list, "answer", &binding) == XUI_OK);
    CHECK(xuiWidgetGetAccessibleNodeCount(list) == 7);
    document_root.iSize = paragraph.iSize = leaf.iSize = sizeof(document_root);
    CHECK(xuiWidgetGetAccessibleNode(list, 3, &document_root) == XUI_OK &&
        document_root.iRole == XUI_ACCESSIBLE_ROLE_DOCUMENT &&
        document_root.iParentId == second_id &&
        !strcmp(document_root.sValue, "answer\n"));
    document_root_id = document_root.iId;
    CHECK(xuiWidgetGetAccessibleNode(list, 4, &paragraph) == XUI_OK &&
        paragraph.iRole == XUI_ACCESSIBLE_ROLE_PARAGRAPH &&
        paragraph.iParentId == document_root_id &&
        !strcmp(paragraph.sValue, "answer"));
    CHECK(xuiWidgetGetAccessibleNode(list, 5, &leaf) == XUI_OK &&
        leaf.iRole == XUI_ACCESSIBLE_ROLE_TEXT &&
        leaf.iParentId == paragraph.iId && !strcmp(leaf.sValue, "answer"));
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 240, 76}) == XUI_OK);
    CHECK(xuiWidgetGetAccessibleNodeCount(view) == 3);
    {
        xui_accessible_node_t direct = {0};
        direct.iSize = sizeof(direct);
        CHECK(xuiWidgetGetAccessibleNode(view, 0, &direct) == XUI_OK &&
            direct.iRole == document_root.iRole &&
            !strcmp(direct.sValue, document_root.sValue));
        direct.iSize = sizeof(direct);
        CHECK(xuiWidgetGetAccessibleNode(view, 1, &direct) == XUI_OK &&
            direct.iRole == paragraph.iRole &&
            !strcmp(direct.sValue, paragraph.sValue));
        direct.iSize = sizeof(direct);
        CHECK(xuiWidgetGetAccessibleNode(view, 2, &direct) == XUI_OK &&
            direct.iRole == leaf.iRole && !strcmp(direct.sValue, leaf.sValue));
    }
    xuiWidgetDestroy(view); view = NULL;
    selection.iSize = sizeof(selection); selection.iAnchor = 2; selection.iCaret = 5;
    CHECK((paragraph.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)) &&
        (leaf.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
    CHECK(xuiWidgetPerformAccessibleAction(list, paragraph.iId,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_OK);
    paragraph.iSize = sizeof(paragraph);
    CHECK(xuiWidgetGetAccessibleNode(list, 4, &paragraph) == XUI_OK &&
        paragraph.iTextStart == 2 && paragraph.iTextEnd == 5 &&
        (paragraph.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) == 4 &&
        !strcmp(selected, "swe"));
    selection.iAnchor = 4; selection.iCaret = 1;
    CHECK(xuiWidgetPerformAccessibleAction(list, leaf.iId,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_OK);
    leaf.iSize = sizeof(leaf);
    CHECK(xuiWidgetGetAccessibleNode(list, 5, &leaf) == XUI_OK &&
        leaf.iTextStart == 4 && leaf.iTextEnd == 1 &&
        (leaf.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) == 4 &&
        !strcmp(selected, "nsw"));
    selection.iCaret = 99;
    CHECK(xuiWidgetPerformAccessibleAction(list, leaf.iId,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiWidgetPerformAccessibleAction(list, leaf.iId,
        XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW, NULL) == XUI_OK);
    CHECK(message_accessible_index_by_id(list, auxiliary_id) == 6);
    second.iSize = sizeof(second);
    CHECK(xuiWidgetGetAccessibleNode(list, 2, &second) == XUI_OK &&
        second.iId == second_id && !strcmp(second.sValue, "answer\n"));
    CHECK(xuiDocumentLoadMarkdown(document, "updated\n", 8) == XUI_OK);
    second.iSize = sizeof(second);
    CHECK(xuiWidgetGetAccessibleNode(list, 2, &second) == XUI_OK &&
        second.iId == second_id && !strcmp(second.sValue, "updated\n"));
    document_root.iSize = sizeof(document_root);
    CHECK(xuiWidgetGetAccessibleNode(list, 3, &document_root) == XUI_OK &&
        document_root.iId == document_root_id &&
        !strcmp(document_root.sValue, "updated\n"));
    CHECK(xuiMessageListAppendNodeText(list, "first", "!") == XUI_OK);
    first.iSize = sizeof(first);
    CHECK(xuiWidgetGetAccessibleNode(list, 1, &first) == XUI_OK &&
        first.iId == first_id && !strcmp(first.sValue, "hello!"));
    CHECK(xuiUpdate(context, 0) == XUI_OK);
    events.list = list;
    CHECK(xuiSetAccessibilityEventCallback(context, message_accessibility_event, &events) == XUI_OK);
    selection.iSize = sizeof(selection); selection.iAnchor = 2; selection.iCaret = 5;
    CHECK(xuiWidgetPerformAccessibleAction(list, second_id,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_OK);
    second.iSize = sizeof(second);
    CHECK(xuiWidgetGetAccessibleNode(list, 2, &second) == XUI_OK &&
        second.iTextStart == 2 && second.iTextEnd == 5 &&
        (second.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) == 4 &&
        !strcmp(selected, "dat"));
    selection.iAnchor = 5; selection.iCaret = 2;
    CHECK(xuiWidgetPerformAccessibleAction(list, second_id,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_OK);
    second.iSize = sizeof(second);
    CHECK(xuiWidgetGetAccessibleNode(list, 2, &second) == XUI_OK &&
        second.iTextStart == 5 && second.iTextEnd == 2);
    selection.iCaret = 99;
    CHECK(xuiWidgetPerformAccessibleAction(list, second_id,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_ERROR_INVALID_ARGUMENT);
    second.iSize = sizeof(second);
    CHECK(xuiWidgetGetAccessibleNode(list, 2, &second) == XUI_OK &&
        second.iTextStart == 5 && second.iTextEnd == 2);
    CHECK(xuiMessageListUpdateNodeText(list, "first", "你A") == XUI_OK);
    selection.iAnchor = 4; selection.iCaret = 0;
    CHECK(xuiWidgetPerformAccessibleAction(list, first_id,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_OK);
    first.iSize = sizeof(first);
    CHECK(xuiWidgetGetAccessibleNode(list, 1, &first) == XUI_OK &&
        first.iId == first_id && !strcmp(first.sValue, "你A") &&
        first.iTextStart == 4 && first.iTextEnd == 0 &&
        (first.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) == 5 &&
        !strcmp(selected, "你A"));
    selection.iCaret = 1;
    CHECK(xuiWidgetPerformAccessibleAction(list, first_id,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_DOC_ERROR_UTF8);
    first.iSize = sizeof(first);
    CHECK(xuiWidgetGetAccessibleNode(list, 1, &first) == XUI_OK &&
        first.iTextStart == 4 && first.iTextEnd == 0);
    CHECK(xuiWidgetPerformAccessibleAction(list, second_id,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
    CHECK(xuiMessageListGetSelected(list) == 1);
    second.iSize = sizeof(second);
    CHECK(xuiWidgetGetAccessibleNode(list, 2, &second) == XUI_OK &&
        (second.iState & XUI_ACCESSIBLE_STATE_SELECTED) &&
        second.iTextStart == 0 && second.iTextEnd == 8);
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) == 9 &&
        !strcmp(selected, "updated\n"));
    CHECK(xuiUpdate(context, 0) == XUI_OK && events.selection > 0);
    before = events.selection;
    CHECK(xuiMessageListClearTextSelection(list) == XUI_OK);
    CHECK(xuiUpdate(context, 0) == XUI_OK && events.selection == before + 1);
    selection.iAnchor = 0; selection.iCaret = 1;
    CHECK(xuiWidgetPerformAccessibleAction(list, auxiliary_id,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_ERROR_UNSUPPORTED);
    before_bounds = events.bounds;
    CHECK(xuiWidgetPerformAccessibleAction(list, auxiliary_id,
        XUI_ACCESSIBLE_ACTION_EXPAND, NULL) == XUI_OK);
    CHECK(xuiUpdate(context, 0) == XUI_OK && events.state == 1 &&
        events.value == 1 && events.bounds == before_bounds + 1);
    auxiliary.iSize = sizeof(auxiliary);
    CHECK(xuiWidgetGetAccessibleNode(list,
        message_accessible_index_by_id(list, auxiliary_id), &auxiliary) == XUI_OK &&
        (auxiliary.iState & XUI_ACCESSIBLE_STATE_EXPANDED) &&
        !strcmp(auxiliary.sValue, "details"));
    CHECK(xuiWidgetPerformAccessibleAction(list, auxiliary_id,
        XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW, NULL) == XUI_OK);
    CHECK(xuiMessageListGetScroll(list) > 0);
    auxiliary.iSize = sizeof(auxiliary);
    CHECK(xuiWidgetGetAccessibleNode(list,
        message_accessible_index_by_id(list, auxiliary_id), &auxiliary) == XUI_OK &&
        !(auxiliary.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN));
    before_bounds = events.bounds;
    CHECK(xuiMessageListSetNodeCollapsed(list, "tool", 1) == XUI_OK);
    CHECK(xuiUpdate(context, 0) == XUI_OK && events.state == 2 &&
        events.value == 2 && events.bounds == before_bounds + 1);
    CHECK(xuiWidgetPerformAccessibleAction(list, second_id,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
    CHECK(xuiUpdate(context, 0) == XUI_OK);
    before = events.selection;
    {
        int before_tree = events.tree;
        CHECK(xuiDocumentLoadMarkdown(document, "replaced\n", 9) == XUI_OK);
        CHECK(xuiUpdate(context, 0) == XUI_OK && events.selection == before + 1 &&
            events.tree == before_tree + 1);
    }
    CHECK(xuiWidgetPerformAccessibleAction(list, second_id,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
    CHECK(xuiUpdate(context, 0) == XUI_OK);
    before = events.selection;
    {
        int before_tree = events.tree;
        CHECK(xuiMessageListSetNodeDocument(list, "answer", NULL) == XUI_OK);
        CHECK(xuiUpdate(context, 0) == XUI_OK && events.selection == before + 1 &&
            events.tree == before_tree + 1);
    }
    CHECK(xuiWidgetGetAccessibleNodeCount(list) == 4 &&
        message_accessible_index_by_id(list, document_root_id) == -1);
    CHECK(xuiWidgetPerformAccessibleAction(list, document_root_id,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiMessageListSetNodes(list, messages, 3) == XUI_OK);
    second.iSize = sizeof(second);
    CHECK(xuiWidgetGetAccessibleNode(list, 2, &second) == XUI_OK &&
        second.iId != second_id && !strcmp(second.sValue, "fallback"));
    CHECK(xuiWidgetPerformAccessibleAction(list, second_id,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiMessageListSetScroll(list, 0) == XUI_OK);
    CHECK(xuiUpdate(context, 0) == XUI_OK);
    bubble = xuiMessageListGetBubbleRect(list, 0);
    world = xuiWidgetGetWorldRect(list);
    content = xuiWidgetGetContentRect(list);
    CHECK(xuiInputPointerDown(context,
        world.fX + content.fX + bubble.fX + 13,
        world.fY + content.fY + bubble.fY + 10,
        XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    dispatch(context);
    CHECK(xuiUpdate(context, 0) == XUI_OK);
    before = events.selection;
    CHECK(xuiInputPointerMove(context,
        world.fX + content.fX + bubble.fX + 42,
        world.fY + content.fY + bubble.fY + 10,
        XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    dispatch(context);
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) > 1);
    CHECK(xuiUpdate(context, 0) == XUI_OK && events.selection == before + 1);
    CHECK(xuiInputPointerUp(context,
        world.fX + content.fX + bubble.fX + 42,
        world.fY + content.fY + bubble.fY + 10,
        XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    dispatch(context);
    xuiDestroy(context); xuiDocumentRelease(document);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("MessageList accessibility: values, UTF-8 ranges, stable ids, expansion, scroll and semantic events passed");
}

typedef struct message_task_toggle_state {
    xui_document document;
    xui_doc_node_id last_node;
    int last_checked;
    int calls;
    int commit;
} message_task_toggle_state;

static void message_task_toggle(xui_widget list, xui_doc_node_id item,
    int checked, void* user)
{
    message_task_toggle_state* state = (message_task_toggle_state*)user;
    xui_document_snapshot snapshot = NULL;
    xui_document_transaction transaction = NULL;
    xui_doc_node_info_t info = {0};
    (void)list;
    state->last_node = item;
    state->last_checked = checked;
    state->calls++;
    if (!state->commit) return;
    CHECK(xuiDocumentAcquireSnapshot(state->document, &snapshot) == XUI_OK);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, item, &info) == XUI_OK);
    CHECK(info.iKind == XUI_DOC_LIST_ITEM &&
        (info.tAttributes.iFlags & XUI_DOC_TASK));
    xuiDocumentSnapshotRelease(snapshot);
    if (checked) info.tAttributes.iFlags |= XUI_DOC_CHECKED;
    else info.tAttributes.iFlags &= ~XUI_DOC_CHECKED;
    CHECK(xuiDocumentBeginTransaction(state->document, NULL, &transaction) == XUI_OK);
    CHECK(xuiDocumentTxnSetAttributes(transaction, item, &info.tAttributes) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction);
}

static void message_task_source(xui_document document, const char* expected)
{
    xui_document_snapshot snapshot = NULL;
    char source[128] = {0};
    uint64_t length = 0;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(snapshot, source, sizeof(source), &length) == XUI_OK);
    CHECK(length == strlen(expected) && !strcmp(source, expected));
    xuiDocumentSnapshotRelease(snapshot);
}

static void test_message_task_toggle(void)
{
    static const char initial[] = "- [ ] first\n- [x] second\n";
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_widget list = NULL;
    xui_document document = NULL;
    xui_document rich = NULL;
    xui_font font = NULL;
    xui_doc_desc_t doc_desc = {0};
    xui_message_list_desc_t list_desc = {0};
    xui_message_document_desc_t binding = {0};
    xui_message_node_t message = {0};
    xui_accessible_node_t first, second;
    message_task_toggle_state state = {0};
    uint64_t first_id, second_id, revision;
    int marker_x, marker_y;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadMemory(&proxy.tProxy, &font, NULL, 0, 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiInputViewport(context, 320, 180) == XUI_OK);
    doc_desc.iSize = sizeof(doc_desc);
    doc_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    doc_desc.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&doc_desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, initial, sizeof(initial) - 1) == XUI_OK);
    message.iSize = sizeof(message);
    message.iType = XUI_MESSAGE_NODE_OTHER;
    message.sId = "tasks";
    message.sSender = "Agent";
    list_desc.iSize = sizeof(list_desc);
    list_desc.arrNodes = &message;
    list_desc.iNodeCount = 1;
    CHECK(xuiMessageListCreate(context, &list, &list_desc) == XUI_OK);
    CHECK(xuiSetRootWidget(context, list) == XUI_OK);
    CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 320, 180}) == XUI_OK);
    state.document = document;
    binding.iSize = sizeof(binding);
    binding.pDocument = document;
    binding.onTaskToggle = message_task_toggle;
    binding.pUser = &state;
    CHECK(xuiMessageListSetNodeDocument(list, "tasks", &binding) == XUI_OK);
    first = message_accessible_by_role_value(list, XUI_ACCESSIBLE_ROLE_CHECKBOX, "first");
    second = message_accessible_by_role_value(list, XUI_ACCESSIBLE_ROLE_CHECKBOX, "second");
    first_id = first.iId;
    second_id = second.iId;
    CHECK((first.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE)) &&
        !(first.iState & XUI_ACCESSIBLE_STATE_READONLY) &&
        !(first.iState & XUI_ACCESSIBLE_STATE_CHECKED));
    CHECK((second.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE)) &&
        (second.iState & XUI_ACCESSIBLE_STATE_CHECKED));
    revision = xuiDocumentGetRevision(document);
    CHECK(xuiWidgetPerformAccessibleAction(list, first_id,
        XUI_ACCESSIBLE_ACTION_TOGGLE, NULL) == XUI_OK);
    CHECK(state.calls == 1 && state.last_node != 0 && state.last_checked == 1);
    CHECK(xuiDocumentGetRevision(document) == revision);
    message_task_source(document, initial);
    state.commit = 1;
    CHECK(xuiWidgetPerformAccessibleAction(list, first_id,
        XUI_ACCESSIBLE_ACTION_TOGGLE, NULL) == XUI_OK);
    CHECK(state.calls == 2 && state.last_checked == 1 &&
        xuiDocumentGetRevision(document) > revision);
    message_task_source(document, "- [x] first\n- [x] second\n");
    first = message_accessible_by_role_value(list, XUI_ACCESSIBLE_ROLE_CHECKBOX, "first");
    CHECK(first.iState & XUI_ACCESSIBLE_STATE_CHECKED);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK);
    message_task_source(document, initial);
    first = message_accessible_by_role_value(list, XUI_ACCESSIBLE_ROLE_CHECKBOX, "first");
    CHECK(!(first.iState & XUI_ACCESSIBLE_STATE_CHECKED));
    marker_x = (int)(first.tBounds.fX + 5);
    marker_y = (int)(first.tBounds.fY + 6);
    CHECK(xuiInputPointerDown(context, marker_x, marker_y,
        XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    dispatch(context);
    CHECK(xuiInputPointerUp(context, marker_x, marker_y,
        XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    dispatch(context);
    CHECK(state.calls == 3 && state.last_checked == 1);
    message_task_source(document, "- [x] first\n- [x] second\n");
    first = message_accessible_by_role_value(list, XUI_ACCESSIBLE_ROLE_CHECKBOX, "first");
    marker_x = (int)(first.tBounds.fX + 5);
    marker_y = (int)(first.tBounds.fY + 6);
    CHECK(xuiInputPointerDown(context, marker_x, marker_y,
        XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    dispatch(context);
    CHECK(xuiInputPointerUp(context, marker_x + 70, marker_y,
        XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    dispatch(context);
    CHECK(state.calls == 3);
    second = message_accessible_by_role_value(list, XUI_ACCESSIBLE_ROLE_CHECKBOX, "second");
    CHECK(xuiWidgetPerformAccessibleAction(list, second.iId,
        XUI_ACCESSIBLE_ACTION_TOGGLE, NULL) == XUI_OK);
    CHECK(state.calls == 4 && state.last_checked == 0);
    message_task_source(document, "- [x] first\n- [ ] second\n");
    binding.onTaskToggle = NULL;
    CHECK(xuiMessageListSetNodeDocument(list, "tasks", &binding) == XUI_OK);
    first = message_accessible_by_role_value(list, XUI_ACCESSIBLE_ROLE_CHECKBOX, "first");
    CHECK((first.iState & XUI_ACCESSIBLE_STATE_READONLY) &&
        !(first.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE)));
    CHECK(xuiWidgetPerformAccessibleAction(list, first.iId,
        XUI_ACCESSIBLE_ACTION_TOGGLE, NULL) == XUI_ERROR_UNSUPPORTED);
    binding.onTaskToggle = message_task_toggle;
    CHECK(xuiMessageListSetNodeDocument(list, "tasks", &binding) == XUI_OK);
    first = message_accessible_by_role_value(list, XUI_ACCESSIBLE_ROLE_CHECKBOX, "first");
    marker_x = (int)(first.tBounds.fX + 5);
    marker_y = (int)(first.tBounds.fY + 6);
    CHECK(xuiInputPointerDown(context, marker_x, marker_y,
        XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    dispatch(context);
    CHECK(xuiMessageListSetNodeDocument(list, "tasks", NULL) == XUI_OK);
    CHECK(xuiInputPointerUp(context, marker_x, marker_y,
        XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    dispatch(context);
    CHECK(state.calls == 4);
    CHECK(xuiWidgetPerformAccessibleAction(list, second_id,
        XUI_ACCESSIBLE_ACTION_TOGGLE, NULL) == XUI_ERROR_NOT_FOUND);
    {
        xui_document_transaction transaction = NULL;
        xui_document_snapshot snapshot = NULL;
        xui_doc_node_desc_t node = {0};
        xui_doc_node_info_t info = {0};
        xui_doc_node_id list_node, item_node, paragraph_node, text_node;
        CHECK(xuiDocumentCreate(NULL, &rich) == XUI_OK);
        CHECK(xuiDocumentBeginTransaction(rich, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node);
        node.iKind = XUI_DOC_LIST;
        CHECK(xuiDocumentTxnInsertNode(transaction, XUI_DOCUMENT_ROOT,
            XUI_DOCUMENT_APPEND, &node, &list_node) == XUI_OK);
        node.iKind = XUI_DOC_LIST_ITEM;
        node.tAttributes.iFlags = XUI_DOC_TASK;
        CHECK(xuiDocumentTxnInsertNode(transaction, list_node,
            XUI_DOCUMENT_APPEND, &node, &item_node) == XUI_OK);
        node.iKind = XUI_DOC_PARAGRAPH;
        node.tAttributes.iFlags = 0;
        CHECK(xuiDocumentTxnInsertNode(transaction, item_node,
            XUI_DOCUMENT_APPEND, &node, &paragraph_node) == XUI_OK);
        node.iKind = XUI_DOC_TEXT;
        node.sText = "rich task";
        node.iTextBytes = strlen(node.sText);
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph_node,
            XUI_DOCUMENT_APPEND, &node, &text_node) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        state.document = rich;
        binding.pDocument = rich;
        CHECK(xuiMessageListSetNodeDocument(list, "tasks", &binding) == XUI_OK);
        first = message_accessible_by_role_value(list,
            XUI_ACCESSIBLE_ROLE_CHECKBOX, "rich task");
        marker_x = (int)(first.tBounds.fX + 5);
        marker_y = (int)(first.tBounds.fY + 6);
        CHECK(xuiInputPointerDown(context, marker_x, marker_y,
            XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
        dispatch(context);
        CHECK(xuiInputPointerUp(context, marker_x, marker_y,
            XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
        dispatch(context);
        CHECK(state.calls == 5 && state.last_node == item_node &&
            state.last_checked == 1);
        CHECK(xuiDocumentAcquireSnapshot(rich, &snapshot) == XUI_OK);
        info.iSize = sizeof(info);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, item_node, &info) == XUI_OK &&
            (info.tAttributes.iFlags & XUI_DOC_CHECKED));
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentUndo(rich, NULL) == XUI_OK);
        first = message_accessible_by_role_value(list,
            XUI_ACCESSIBLE_ROLE_CHECKBOX, "rich task");
        CHECK(!(first.iState & XUI_ACCESSIBLE_STATE_CHECKED));
        CHECK(xuiMessageListSetNodeDocument(list, "tasks", NULL) == XUI_OK);
    }
    xuiDestroy(context);
    xuiDocumentRelease(document);
    xuiDocumentRelease(rich);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("MessageList task checkbox: Rich/Markdown host transaction, pointer cancel and accessibility passed");
}

int main(void)
{
    test_message_task_toggle();
    test_message_accessibility();
    test_message_structured_accessibility();
    test_message_merged_table_selection();
    test_message_image_registry_update();
    test_message_document_font_invalidation();
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_widget list = NULL;
    xui_surface surface = NULL;
    xui_surface cache = NULL;
    xui_font font = NULL;
    xui_font larger_font = NULL;
    xui_document document = NULL;
    xui_document rich = NULL;
    xui_document formula_document = NULL;
    xui_doc_desc_t document_desc = {0};
    xui_message_list_desc_t list_desc = {0};
    xui_message_document_desc_t binding = {0};
    xui_message_node_t nodes[2] = {{0}};
    xui_message_list_metrics_t metrics;
    xui_message_list_colors_t colors;
    xui_rect_t before, after, later, bubble, content, world;
    xui_doc_position_t start, end, scrolled;
    xui_doc_range_t selection;
    char selected[256];
    float body_x, body_y;
    float fallback_height;
    float scroll;
    int initial_draws;
    int activations = 0;
    xui_rect_i_t damage = {0, 0, 480, 190};
    const char* initial_markdown = "# Hello world\n\nSecond paragraph with **bold** text.\n\n- first\n- second\n";
    const char* updated_markdown = "# Hello world\n\nSecond paragraph with **bold** text.\n\n- first\n- second\n\nA new ending.\n";

    test_cross_message_selection();
    test_message_long_paragraph_continuation(0);
    test_message_long_paragraph_continuation(1);

    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadMemory(&proxy.tProxy, &font, NULL, 0, 16.0f, 0) == XUI_OK && font != NULL);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiInputViewport(context, 480, 190) == XUI_OK);
    document_desc.iSize = sizeof(document_desc);
    document_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    document_desc.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&document_desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, initial_markdown, strlen(initial_markdown)) == XUI_OK);
    {
        xui_document_snapshot snapshot;
        xui_document_renderer renderer;
        xui_doc_rect_t size;
        int exact;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK);
        CHECK(xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(renderer, 300, 0, 1000000000) == XUI_OK);
        CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK);
        CHECK(exact && size.height > 60);
        xuiDocumentRendererRelease(renderer);
        xuiDocumentSnapshotRelease(snapshot);
    }

    nodes[0].iSize = sizeof(nodes[0]); nodes[0].sId = "answer";
    nodes[0].iType = XUI_MESSAGE_NODE_OTHER; nodes[0].sSender = "Agent";
    nodes[0].sText = "fallback";
    nodes[1].iSize = sizeof(nodes[1]); nodes[1].sId = "next";
    nodes[1].iType = XUI_MESSAGE_NODE_SELF; nodes[1].sSender = "User";
    nodes[1].sText = "next task";
    list_desc.iSize = sizeof(list_desc); list_desc.arrNodes = nodes; list_desc.iNodeCount = 2;
    CHECK(xuiMessageListCreate(context, &list, &list_desc) == XUI_OK);
    CHECK(xuiSetRootWidget(context, list) == XUI_OK);
    CHECK(xuiWidgetSetRect(list, (xui_rect_t){0, 0, 480, 190}) == XUI_OK);
    before = xuiMessageListGetNodeRect(list, 0);
    fallback_height = before.fH;

    binding.iSize = sizeof(binding); binding.pDocument = document;
    CHECK(xuiMessageListSetNodeDocument(list, "answer", &binding) == XUI_OK);
    CHECK(xuiMessageListGetNodeDocument(list, "answer") == document);
    after = xuiMessageListGetNodeRect(list, 0);
    later = xuiMessageListGetNodeRect(list, 1);
    CHECK(after.fH > before.fH + 20 && later.fY > after.fY);
    CHECK(xuiMessageListGetMetrics(list, &metrics) == XUI_OK);
    bubble = xuiMessageListGetBubbleRect(list, 0);
    content = xuiWidgetGetContentRect(list); world = xuiWidgetGetWorldRect(list);
    body_x = world.fX + content.fX + bubble.fX + metrics.fBubblePaddingX;
    body_y = world.fY + content.fY + bubble.fY + metrics.fBubblePaddingY;
    CHECK(xuiMessageListHitNodeDocument(list, "answer", body_x + 2, body_y + 12, &start) == XUI_OK);
    CHECK(xuiMessageListHitNodeDocument(list, "answer", body_x + 95, body_y + 12, &end) == XUI_OK);
    CHECK(start.iDocumentId == end.iDocumentId && start.iNodeId != 0);
    CHECK(start.iNodeId != end.iNodeId || start.iOffset != end.iOffset);

    CHECK(xuiTestSurfaceCreate(&proxy, &surface, 480, 190, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiRender(context, surface, &damage, 1) == XUI_OK);
    cache = xuiWidgetGetCacheSurface(list, xuiWidgetGetStateId(list));
    CHECK(cache != NULL);
    initial_draws = xuiTestSurfaceGetTextDrawCount(cache);
    CHECK(initial_draws > 2);
    CHECK(xuiInputPointerDown(context, (int)(body_x + 2), (int)(body_y + 12),
        XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    dispatch(context);
    CHECK(xuiInputPointerMove(context, (int)(body_x + 95), (int)(body_y + 12), XUI_POINTER_BUTTON_LEFT) == XUI_OK);
    dispatch(context);
    CHECK(xuiInputPointerUp(context, (int)(body_x + 95), (int)(body_y + 12), XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
    dispatch(context);
    CHECK(xuiMessageListGetNodeDocumentSelection(list, "answer", &selection) == XUI_OK);
    CHECK(selection.tAnchor.iOffset != selection.tCaret.iOffset ||
        selection.tAnchor.iNodeId != selection.tCaret.iNodeId);
    CHECK(xuiMessageListGetSelectedText(list, selected, sizeof(selected)) > 1);
    CHECK(strstr(selected, "Hello") != NULL);
    CHECK(xuiMessageListCopySelection(list) == XUI_OK);
    CHECK(!strcmp(xuiTestProxyGetClipboardText(&proxy), selected));

    CHECK(xuiMessageListSetScroll(list, 18.0f) == XUI_OK);
    scroll = xuiMessageListGetScroll(list);
    CHECK(scroll > 0);
    CHECK(xuiMessageListHitNodeDocument(list, "answer", body_x + 95, body_y + 12 - scroll, &scrolled) == XUI_OK);
    CHECK(scrolled.iNodeId == end.iNodeId && scrolled.iOffset == end.iOffset);

    CHECK(xuiDocumentLoadMarkdown(document, updated_markdown, strlen(updated_markdown)) == XUI_OK);
    CHECK(xuiMessageListGetNodeRect(list, 0).fH > after.fH);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiRender(context, surface, &damage, 1) == XUI_OK);
    {
        xui_document_transaction transaction = NULL;
        xui_doc_node_desc_t node = {0};
        xui_doc_node_id paragraph;
        xui_doc_node_id text_node;
        xui_doc_node_id image_node;
        xui_doc_position_t hit;
        CHECK(xuiDocumentCreate(NULL, &rich) == XUI_OK);
        CHECK(xuiDocumentBeginTransaction(rich, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
        CHECK(xuiDocumentTxnInsertNode(transaction, XUI_DOCUMENT_ROOT, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
        memset(&node, 0, sizeof(node)); node.iSize = sizeof(node); node.iKind = XUI_DOC_TEXT;
        node.sText = "Rich link here"; node.iTextBytes = strlen(node.sText);
        node.tAttributes.iMarks = XUI_DOC_BOLD | XUI_DOC_LINK;
        node.sResource = "https://example.test/rich";
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &text_node) == XUI_OK);
        memset(&node, 0, sizeof(node)); node.iSize = sizeof(node); node.iKind = XUI_DOC_IMAGE;
        node.sText = "linked alt"; node.iTextBytes = strlen(node.sText);
        node.sResource = "/asset"; node.sTitle = "Asset title";
        node.sLinkTarget = "https://example.test/linked-image";
        node.sLinkTitle = "Linked image"; node.tAttributes.iMarks = XUI_DOC_LINK;
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &image_node) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        binding.pDocument = rich; binding.onActivate = activated; binding.pUser = &activations;
        CHECK(xuiMessageListSetNodeDocument(list, "next", &binding) == XUI_OK);
        CHECK(xuiMessageListScrollToEnd(list) == XUI_OK);
        bubble = xuiMessageListGetBubbleRect(list, 1);
        scroll = xuiMessageListGetScroll(list);
        body_x = world.fX + content.fX + bubble.fX + metrics.fBubblePaddingX;
        body_y = world.fY + content.fY + bubble.fY + metrics.fBubblePaddingY - scroll;
        CHECK(xuiMessageListHitNodeDocument(list, "next", body_x + 8, body_y + 8, &hit) == XUI_OK);
        CHECK(hit.iNodeId != 0);
        CHECK(xuiInputPointerDown(context, (int)(body_x + 8), (int)(body_y + 8),
            XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
        dispatch(context);
        CHECK(xuiInputPointerUp(context, (int)(body_x + 8), (int)(body_y + 8),
            XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
        dispatch(context);
        CHECK(activations == 1);
        {
            int accessible_count = xuiWidgetGetAccessibleNodeCount(list);
            int accessible_index, found_link = 0, found_image = 0, clicked_image = 0;
            for (accessible_index = 0; accessible_index < accessible_count; accessible_index++) {
                xui_accessible_node_t accessible = {0};
                accessible.iSize = sizeof(accessible);
                CHECK(xuiWidgetGetAccessibleNode(list, accessible_index, &accessible) == XUI_OK);
                if (accessible.iRole != XUI_ACCESSIBLE_ROLE_LINK ||
                    !accessible.sValue || strcmp(accessible.sValue, "Rich link here")) {
                    if (accessible.iRole == XUI_ACCESSIBLE_ROLE_IMAGE &&
                        accessible.sName && !strcmp(accessible.sName, "linked alt")) {
                        CHECK(accessible.sDescription &&
                            !strcmp(accessible.sDescription, "https://example.test/linked-image"));
                        CHECK(accessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE));
                        CHECK(xuiWidgetPerformAccessibleAction(list, accessible.iId,
                            XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) == XUI_OK);
                        if (!(accessible.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN) &&
                            accessible.tBounds.fW > 4 && accessible.tBounds.fH > 4) {
                            float image_x = accessible.tBounds.fX + 3;
                            float image_y = accessible.tBounds.fY + 3;
                            CHECK(xuiMessageListHitNodeDocument(list, "next", image_x, image_y,
                                &hit) == XUI_OK && hit.iNodeId == image_node);
                            CHECK(xuiInputPointerDown(context, (int)image_x, (int)image_y,
                                XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
                            dispatch(context);
                            CHECK(xuiInputPointerUp(context, (int)image_x, (int)image_y,
                                XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
                            dispatch(context);
                            clicked_image = 1;
                        }
                        found_image = 1;
                    }
                    continue;
                }
                CHECK(accessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE));
                CHECK(xuiWidgetPerformAccessibleAction(list, accessible.iId,
                    XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) == XUI_OK);
                found_link = 1;
            }
            CHECK(found_link && found_image && clicked_image && activations == 4);
        }
    }
    CHECK(proxy.tProxy.fontLoadMemory(&proxy.tProxy, &larger_font, NULL, 0, 20.0f, 0) == XUI_OK);
    CHECK(xuiMessageListSetFont(list, larger_font) == XUI_OK);
    CHECK(xuiMessageListGetNodeRect(list, 0).fH > 0);
    CHECK(xuiMessageListGetColors(list, &colors) == XUI_OK);
    colors.iOtherTextColor = XUI_COLOR_RGBA(40, 70, 120, 255);
    CHECK(xuiMessageListSetColors(list, &colors) == XUI_OK);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiRender(context, surface, &damage, 1) == XUI_OK);
    {
        xui_doc_desc_t formula_desc = {0};
        int changes;
        formula_desc.iSize = sizeof(formula_desc);
        formula_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        formula_desc.iMarkdownDialect = XUI_MD_EXTENDED;
        CHECK(xuiDocumentCreate(&formula_desc, &formula_document) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(formula_document,
            "Formula $x^2$ here.\n", strlen("Formula $x^2$ here.\n")) == XUI_OK);
        binding.pDocument = formula_document;
        memset(&binding.tRenderer, 0, sizeof(binding.tRenderer));
        binding.tRenderer.iSize = sizeof(binding.tRenderer);
        binding.tRenderer.onObjectMeasure = measure_formula;
        formula_ready = 0;
        CHECK(xuiMessageListSetNodeDocument(list, "answer", &binding) == XUI_OK);
        before = xuiMessageListGetNodeRect(list, 0);
        changes = xuiMessageListGetChangeCount(list);
        formula_ready = 1;
        CHECK(xuiMessageListInvalidateNodeDocumentObjects(list, "answer") == XUI_OK);
        after = xuiMessageListGetNodeRect(list, 0);
        CHECK(after.fH > before.fH + 40);
        CHECK(xuiMessageListGetChangeCount(list) == changes + 1);
        CHECK(xuiMessageListInvalidateNodeDocumentObjects(list, "missing") == XUI_ERROR_NOT_FOUND);
    }
    CHECK(xuiMessageListSetNodeDocument(list, "answer", NULL) == XUI_OK);
    CHECK(xuiMessageListGetNodeDocument(list, "answer") == NULL);
    CHECK(xuiMessageListInvalidateNodeDocumentObjects(list, "answer") == XUI_ERROR_NOT_FOUND);
    CHECK(xuiMessageListGetNodeRect(list, 0).fH == fallback_height);
    CHECK(xuiMessageListSetNodeDocument(list, "answer", &binding) == XUI_OK);
    CHECK(xuiMessageListSetNodes(list, nodes, 2) == XUI_OK);
    CHECK(xuiMessageListGetNodeDocument(list, "answer") == NULL);
    {
        xui_test_proxy_state_t other_proxy;
        xui_context other_context = NULL;
        xui_widget other_list = NULL;
        int before_changes;
        xuiTestProxyInit(&other_proxy);
        CHECK(xuiCreate(&other_context) == XUI_OK);
        CHECK(xuiSetProxy(other_context, &other_proxy.tProxy) == XUI_OK);
        CHECK(xuiMessageListCreate(other_context, &other_list, &list_desc) == XUI_OK);
        CHECK(xuiSetRootWidget(other_context, other_list) == XUI_OK);
        CHECK(xuiWidgetSetRect(other_list, (xui_rect_t){0, 0, 480, 190}) == XUI_OK);
        before_changes = xuiMessageListGetChangeCount(other_list);
        binding.pDocument = document;
        CHECK(xuiMessageListSetNodeDocument(other_list, "answer", &binding) != XUI_OK);
        CHECK(xuiMessageListGetNodeDocument(other_list, "answer") == NULL);
        CHECK(xuiMessageListGetChangeCount(other_list) == before_changes);
        xuiDestroy(other_context);
    }

    proxy.tProxy.surfaceDestroy(&proxy.tProxy, surface);
    xuiDocumentRelease(document);
    xuiDocumentRelease(rich);
    xuiDocumentRelease(formula_document);
    xuiDestroy(context);
    proxy.tProxy.fontDestroy(&proxy.tProxy, larger_font);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("MessageList Document: height, rendering, hit, scroll, selection, copy, update and lifetime passed");
    return 0;
}
