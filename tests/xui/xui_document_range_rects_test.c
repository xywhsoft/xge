#define XRT_MODULE_ALL
#include "xui_document_ui.h"
#include "src/xui/document/xui_document_internal.h"

#include <assert.h>
#include <stdio.h>

static void* (*original_alloc)(void*, size_t);
static int allocation_budget;

static void* fail_alloc(void* user, size_t bytes)
{
    if (allocation_budget-- == 0) return NULL;
    return original_alloc(user, bytes);
}

static int check_oom(xui_document_renderer renderer, xui_document_snapshot snapshot,
    const xui_doc_range_t* range)
{
    doc_allocator* allocator = snapshot->state->allocator;
    uint64_t baseline = atomic_load(&allocator->live);
    original_alloc = allocator->alloc;
    for (int budget = 0; budget < 16; ++budget) {
        xui_document_range_rects rectangles = NULL;
        allocation_budget = budget;
        allocator->alloc = fail_alloc;
        int status = xuiDocumentRendererCollectRangeRects(renderer, range, &rectangles);
        allocator->alloc = original_alloc;
        if (status == XUI_OK) {
            assert(rectangles && xuiDocumentRangeRectsReleaseChecked(rectangles) == XUI_OK);
            assert(atomic_load(&allocator->live) == baseline);
            return budget;
        }
        assert(status == XUI_ERROR_OUT_OF_MEMORY && !rectangles);
        assert(atomic_load(&allocator->live) == baseline);
    }
    return -1;
}

typedef struct close_during_measure {
    xui_document_renderer renderer;
    unsigned calls;
} close_during_measure;

static int measure_then_close(xui_document_snapshot snapshot, xui_doc_node_id node,
    float width, float zoom, xui_vec2_t* size, float* baseline, void* user)
{
    (void)snapshot;
    (void)node;
    (void)width;
    close_during_measure* state = user;
    ++state->calls;
    *size = (xui_vec2_t){30 * zoom, 20 * zoom};
    *baseline = 16 * zoom;
    assert(xuiDocumentRendererRequestClose(state->renderer) == XUI_OK);
    return XUI_OK;
}

static void check_call_tail(xui_context context)
{
    xui_document document = NULL;
    xui_document_transaction edit = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_doc_node_id paragraph, math;
    assert(xuiDocumentCreateOwned(NULL, &document) == XUI_OK);
    assert(xuiDocumentBeginTransaction(document, NULL, &edit) == XUI_OK);
    xui_doc_node_desc_t node = {.iSize = sizeof(node), .iKind = XUI_DOC_PARAGRAPH};
    assert(xuiDocumentTxnInsertNode(edit, XUI_DOCUMENT_ROOT, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
    node = (xui_doc_node_desc_t){.iSize = sizeof(node), .iKind = XUI_DOC_MATH, .sText = "x", .iTextBytes = 1};
    assert(xuiDocumentTxnInsertNode(edit, paragraph, XUI_DOCUMENT_APPEND, &node, &math) == XUI_OK);
    assert(xuiDocumentTxnCommit(edit, NULL) == XUI_OK);
    assert(xuiDocumentTxnReleaseChecked(edit) == XUI_OK);
    assert(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);

    close_during_measure state = {0};
    xui_doc_renderer_desc_t desc = {
        .iSize = sizeof(desc), .onObjectMeasure = measure_then_close, .pUser = &state
    };
    assert(xuiDocumentRendererCreate(context, &desc, &state.renderer) == XUI_OK);
    assert(xuiDocumentRendererSetSnapshot(state.renderer, snapshot, NULL) == XUI_OK);
    xui_doc_position_t start = {
        .iSize = sizeof(start), .iKind = XUI_DOC_POSITION_GAP,
        .iDocumentId = xuiDocumentGetIdentity(document),
        .iRevision = xuiDocumentGetRevision(document), .iNodeId = paragraph
    };
    xui_doc_range_t range = {start, start};
    range.tCaret.iOffset = 1;
    range.tCaret.iAffinity = XUI_DOC_BEFORE;
    xui_document_range_rects output = NULL;
    uint64_t before = atomic_load(&snapshot->state->allocator->live);
    assert(xuiDocumentRendererCollectRangeRects(state.renderer, &range, &output) == XUI_ERROR_INVALID_STATE);
    assert(!output && state.calls == 1);
    assert(atomic_load(&snapshot->state->allocator->live) == before);
    assert(xuiDocumentRendererReleaseChecked(state.renderer) == XUI_OK);
    assert(xuiDocumentSnapshotReleaseChecked(snapshot) == XUI_OK);
    assert(xuiDocumentReleaseChecked(document) == XUI_OK);
}

int main(void)
{
    xui_proxy proxy = NULL;
    xui_context context = NULL;
    xui_document document = NULL;
    xui_document_transaction edit = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_document_renderer renderer = NULL;
    assert(xuiProxyXgeCreate(&proxy) == XUI_OK);
    assert(xuiCreateOwned(proxy, &context) == XUI_OK);
    check_call_tail(context);
    assert(xuiDocumentCreateOwned(NULL, &document) == XUI_OK);
    assert(xuiDocumentBeginTransaction(document, NULL, &edit) == XUI_OK);
    for (size_t i = 0; i < 20; ++i) {
        xui_doc_node_desc_t paragraph = {.iSize = sizeof(paragraph), .iKind = XUI_DOC_PARAGRAPH};
        xui_doc_node_desc_t text = {.iSize = sizeof(text), .iKind = XUI_DOC_TEXT, .sText = "hello", .iTextBytes = 5};
        xui_doc_node_id paragraph_id, text_id;
        assert(xuiDocumentTxnInsertNode(edit, XUI_DOCUMENT_ROOT, XUI_DOCUMENT_APPEND, &paragraph, &paragraph_id) == XUI_OK);
        assert(xuiDocumentTxnInsertNode(edit, paragraph_id, XUI_DOCUMENT_APPEND, &text, &text_id) == XUI_OK);
    }
    assert(xuiDocumentTxnCommit(edit, NULL) == XUI_OK);
    assert(xuiDocumentTxnReleaseChecked(edit) == XUI_OK);
    assert(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    assert(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK);
    assert(xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    assert(xuiDocumentRendererLayout(renderer, 300, 0, 100) == XUI_OK);

    xui_doc_range_t matches[20];
    uint64_t total;
    assert(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "hello", 5, NULL, matches, 20, &total) == XUI_OK && total == 20);
    xui_doc_range_t range = {matches[0].tAnchor, matches[19].tCaret};
    xui_doc_rect_t expected[20];
    assert(xuiDocumentRendererGetRangeRects(renderer, &range, expected, 20, &total) == XUI_OK && total == 20);
    xui_doc_rect_t prefix;
    assert(xuiDocumentRendererGetRangeRects(renderer, &range, &prefix, 1, &total) == XUI_OK && total == 20);
    assert(prefix.width == expected[0].width && prefix.y == expected[0].y);

    xui_document_range_rects rectangles = NULL, empty = NULL, shared = NULL;
    assert(xuiDocumentRendererCollectRangeRects(renderer, &range, &rectangles) == XUI_OK);
    const xui_doc_rect_t* entries = NULL;
    size_t count;
    assert(xuiDocumentRangeRectsGetEntries(rectangles, &entries, &count) == XUI_OK && count == 20);
    xui_doc_range_t collapsed = {range.tAnchor, range.tAnchor};
    assert(xuiDocumentRendererCollectRangeRects(renderer, &collapsed, &empty) == XUI_OK);
    const xui_doc_rect_t* none = entries;
    assert(xuiDocumentRangeRectsGetEntries(empty, &none, &count) == XUI_OK && !none && !count);
    assert(xuiDocumentRangeRectsReleaseChecked(empty) == XUI_OK);
    xui_doc_range_t stale = range;
    stale.tCaret.iRevision += 1;
    empty = rectangles;
    assert(xuiDocumentRendererCollectRangeRects(renderer, &stale, &empty) != XUI_OK && !empty);
    int failures = check_oom(renderer, snapshot, &range);
    assert(failures == 4);

    assert(xuiDocumentRangeRectsShareOwned(rectangles, &shared) == XUI_OK);
    assert(xuiDocumentRangeRectsReleaseChecked(rectangles) == XUI_OK);
    assert(xuiDocumentRendererReleaseChecked(renderer) == XUI_OK);
    assert(xuiDocumentSnapshotReleaseChecked(snapshot) == XUI_OK);
    assert(xuiDocumentReleaseChecked(document) == XUI_OK);
    assert(xuiDestroyChecked(context) == XUI_OK);
    assert(xuiProxyReleaseChecked(proxy) == XUI_OK);
    for (size_t i = 0; i < 20; ++i) {
        assert(entries[i].x == expected[i].x && entries[i].y == expected[i].y &&
            entries[i].width == expected[i].width && entries[i].height == expected[i].height);
    }
    assert(xuiDocumentRangeRectsReleaseChecked(shared) == XUI_OK);
    puts("document-range-rects-ok");
    return 0;
}
