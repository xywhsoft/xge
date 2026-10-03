#include "../xui_document_ui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>

#define CHECK(e) do { if (!(e)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); exit(1); } } while (0)
#define PARAGRAPHS 10000
#define COLUMNS 96
static double now_ms(void)
{
    LARGE_INTEGER rate, value;
    QueryPerformanceFrequency(&rate); QueryPerformanceCounter(&value);
    return (double)value.QuadPart * 1000 / (double)rate.QuadPart;
}
static uint64_t insert(xui_document_transaction t, uint64_t parent, uint32_t kind,
    const char* text, size_t bytes)
{
    xui_doc_node_desc_t desc = {0}; uint64_t id = 0;
    desc.iSize = sizeof(desc); desc.iKind = kind; desc.sText = text; desc.iTextBytes = bytes;
    CHECK(xuiDocumentTxnInsertNode(t, parent, XUI_DOCUMENT_APPEND, &desc, &id) == XUI_OK);
    return id;
}
static xui_doc_position_t position(xui_document d, uint64_t id, uint64_t offset)
{
    xui_doc_position_t p = {0}; p.iSize = sizeof(p); p.iKind = XUI_DOC_POSITION_TEXT;
    p.iDocumentId = xuiDocumentGetIdentity(d); p.iRevision = xuiDocumentGetRevision(d);
    p.iNodeId = id; p.iOffset = offset; return p;
}
static xui_doc_position_t source_position(xui_document d, uint64_t offset)
{
    xui_doc_position_t p = position(d, 1, offset);
    p.iKind = XUI_DOC_POSITION_SOURCE;
    return p;
}
static void copy_check(xui_document d, uint64_t id, uint64_t start, uint64_t end, const char* expected)
{
    xui_document_snapshot snapshot; xui_doc_range_t range; char* copied = NULL; uint64_t bytes = 0;
    range.tAnchor = position(d, id, start); range.tCaret = position(d, id, end);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopyRange(snapshot, &range, &copied, &bytes) == XUI_OK);
    CHECK(bytes == strlen(expected) && !memcmp(copied, expected, (size_t)bytes));
    xuiDocumentFreeBuffer(copied); xuiDocumentSnapshotRelease(snapshot);
}
static void frame(xui_context context, xui_surface target)
{
    CHECK(xuiLayout(context) == XUI_OK);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiRender(context, target, NULL, 0) == XUI_OK);
}
static xui_doc_renderer_stats_t render_stats(xui_widget editor)
{
    xui_doc_renderer_stats_t stats = {0}; stats.iSize = sizeof(stats);
    CHECK(xuiDocumentViewGetRenderStats(editor, &stats) == XUI_OK);
    return stats;
}
typedef struct scale_font_switch { xui_font current; } scale_font_switch;
static xui_text_shape_proc scale_base_shape;
static int scale_fail_shape_after;
static int scale_late_tall_factor = 3;
static char scale_ascii_tall_char;
static int scale_late_tall_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;

    int i, result;
    if (scale_fail_shape_after > 0 && --scale_fail_shape_after == 0)
        return XUI_ERROR_OUT_OF_MEMORY;
    result = scale_base_shape(proxy, pTextItem, shape);
    if (result != XUI_OK) return result;
    if (scale_ascii_tall_char && memchr(text, scale_ascii_tall_char, (size_t)bytes)) {
        shape->fAscent *= scale_late_tall_factor;
        shape->fDescent *= scale_late_tall_factor;
        shape->fLineHeight *= scale_late_tall_factor;
        shape->fHeight = shape->fLineHeight;
        return XUI_OK;
    }
    for (i = 0; i + 2 < bytes; i++) if ((unsigned char)text[i] == 0xe4 &&
        (unsigned char)text[i + 1] == 0xb8 &&
        (unsigned char)text[i + 2] == 0xad) {
        shape->fAscent *= scale_late_tall_factor;
        shape->fDescent *= scale_late_tall_factor;
        shape->fLineHeight *= scale_late_tall_factor;
        shape->fHeight = shape->fLineHeight;
        break;
    }
    return XUI_OK;
}
static xui_font scale_select_font(xui_context context, const char* family,
    uint32_t marks, float size, void* user)
{
    scale_font_switch* state = (scale_font_switch*)user;
    (void)context; (void)family; (void)marks; (void)size;
    return state->current;
}
static void many_paragraphs(xui_context context, xui_surface target)
{
    static const char pattern[] = "Structured rich text benchmark paragraph with styled content and UTF-8-safe editing. ";
    uint64_t* bodies = calloc(PARAGRAPHS, sizeof(*bodies));
    xui_document document; xui_document_transaction txn; xui_widget editor;
    xui_doc_editor_desc_t desc = {0}; xui_doc_renderer_stats_t first, middle, bottom, narrow, warm;
    xui_doc_memory_stats_t memory_before = {0}, memory_after = {0};
    xui_doc_stats_t stats_before = {0}, stats_after = {0};
    xui_doc_rect_t size; xui_doc_range_t selection; char line[COLUMNS + 1];
    double start, build_ms, first_ms, insert_ms, undo_ms, redo_ms, join_ms;
    int exact; uint64_t paragraph, count; xui_document_snapshot snapshot;
    char original_middle[2];
    int i, j;
    CHECK(bodies != NULL);
    for (j = 0; j < COLUMNS; j++) line[j] = pattern[j % (sizeof(pattern) - 1)];
    line[COLUMNS] = 0;
    original_middle[0] = line[COLUMNS / 2]; original_middle[1] = 0;
    start = now_ms();
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    for (i = 0; i < PARAGRAPHS; i++) {
        paragraph = insert(txn, 1, XUI_DOC_PARAGRAPH, NULL, 0);
        bodies[i] = insert(txn, paragraph, XUI_DOC_TEXT, line, COLUMNS);
    }
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentClearHistory(document) == XUI_OK);
    build_ms = now_ms() - start;
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 936, 616}) == XUI_OK);
    start = now_ms(); frame(context, target); first_ms = now_ms() - start;
    first = render_stats(editor);
    CHECK(first.iBlocks == PARAGRAPHS && first.iMeasuredBlocks > 0 &&
        first.iMeasuredBlocks < 150 && first.iShapedBytes < PARAGRAPHS * COLUMNS / 4);
    CHECK(xuiDocumentViewGetContentSize(editor, &size, &exact) == XUI_OK && size.height > 616);
    CHECK(xuiDocumentViewSetScroll(editor, 0, size.height * .50) == XUI_OK);
    frame(context, target); middle = render_stats(editor);
    CHECK(middle.iMeasuredBlocks > first.iMeasuredBlocks && middle.iMeasuredBlocks < 300);
    CHECK(xuiDocumentViewSetScroll(editor, 0, size.height * .96) == XUI_OK);
    frame(context, target); bottom = render_stats(editor);
    CHECK(bottom.iMeasuredBlocks > middle.iMeasuredBlocks && bottom.iMeasuredBlocks < 450);
    CHECK(xuiWidgetInvalidate(editor, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) == XUI_OK);
    frame(context, target); warm = render_stats(editor);
    CHECK(warm.iLayoutPasses == bottom.iLayoutPasses && warm.iShapedBytes == bottom.iShapedBytes);
    CHECK(xuiInputViewport(context, 520, 640) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 520, 616}) == XUI_OK);
    frame(context, target); narrow = render_stats(editor);
    CHECK(narrow.iLayoutPasses > warm.iLayoutPasses &&
        narrow.iLayoutPasses - warm.iLayoutPasses < 200);

    selection.tAnchor = selection.tCaret = position(document, bodies[PARAGRAPHS / 2], COLUMNS / 2);
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    memory_before.iSize = sizeof(memory_before); memory_after.iSize = sizeof(memory_after);
    stats_before.iSize = sizeof(stats_before); stats_after.iSize = sizeof(stats_after);
    CHECK(xuiDocumentGetMemoryStats(document, &memory_before) == XUI_OK &&
        xuiDocumentGetStats(document, &stats_before) == XUI_OK &&
        memory_before.iHistoryBytes == 0);
    start = now_ms(); CHECK(xuiDocumentEditorInsertText(editor, "X", 1) == XUI_OK);
    insert_ms = now_ms() - start;
    CHECK(xuiDocumentGetMemoryStats(document, &memory_after) == XUI_OK &&
        xuiDocumentGetStats(document, &stats_after) == XUI_OK);
    CHECK(stats_after.iUndoCount == 1 && memory_after.iHistoryBytes > 0 &&
        memory_after.iHistoryBytes < memory_before.iCurrentBytes / 100 &&
        stats_after.iAllocations >= stats_before.iAllocations &&
        stats_after.iAllocations - stats_before.iAllocations < 1000 &&
        memory_after.iLiveBytes >= memory_before.iLiveBytes &&
        memory_after.iLiveBytes - memory_before.iLiveBytes <
            memory_before.iCurrentBytes / 100 &&
        memory_after.iLiveBytes == memory_after.iCurrentBytes +
            memory_after.iHistoryBytes + memory_after.iSnapshotAdditionalBytes +
            memory_after.iOtherBytes);
    printf("Document first undoable edit: nodes=%llu current_before=%llu current_after=%llu history=%llu live_delta=%lld allocations=%llu prepared_accounting_visits=%llu\n",
        (unsigned long long)stats_before.iNodes,
        (unsigned long long)memory_before.iCurrentBytes,
        (unsigned long long)memory_after.iCurrentBytes,
        (unsigned long long)memory_after.iHistoryBytes,
        (long long)(memory_after.iLiveBytes - memory_before.iLiveBytes),
        (unsigned long long)(stats_after.iAllocations - stats_before.iAllocations),
        (unsigned long long)(stats_after.iPreparedAccountingVisits -
            stats_before.iPreparedAccountingVisits));
    copy_check(document, bodies[PARAGRAPHS / 2], COLUMNS / 2, COLUMNS / 2 + 1, "X");
    start = now_ms(); CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    undo_ms = now_ms() - start;
    copy_check(document, bodies[PARAGRAPHS / 2], COLUMNS / 2, COLUMNS / 2 + 1, original_middle);
    start = now_ms(); CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    redo_ms = now_ms() - start;
    copy_check(document, bodies[PARAGRAPHS / 2], COLUMNS / 2, COLUMNS / 2 + 1, "X");
    selection.tAnchor = position(document, bodies[PARAGRAPHS / 2], COLUMNS - 2);
    selection.tCaret = position(document, bodies[PARAGRAPHS / 2 + 1], 3);
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    start = now_ms(); CHECK(xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    join_ms = now_ms() - start;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "JOIN", 4, NULL, NULL, 0, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    frame(context, target);
    printf("DocumentEditor scale: paragraphs=%d bytes=%d build=%.2fms first_frame=%.2fms measured_first=%llu measured_after_scroll=%llu layout_passes_after_width=%llu local_insert=%.2fms undo=%.2fms redo=%.2fms cross_paragraph=%.2fms\n",
        PARAGRAPHS, PARAGRAPHS * (COLUMNS + 1) - 1, build_ms, first_ms,
        (unsigned long long)first.iMeasuredBlocks, (unsigned long long)bottom.iMeasuredBlocks,
        (unsigned long long)narrow.iLayoutPasses, insert_ms, undo_ms, redo_ms, join_ms);
    CHECK(xuiSetRootWidget(context, NULL) == XUI_OK);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document); free(bodies);
    CHECK(xuiInputViewport(context, 960, 640) == XUI_OK);
}
static void single_large_paragraph(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    static const char pattern[] = "Large styled paragraph. ";
    const size_t bytes = 100000; char* text = malloc(bytes + 1);
    xui_document document; xui_document_transaction txn; xui_widget view;
    xui_doc_node_desc_t styled = {0};
    xui_doc_view_desc_t desc = {0}; xui_doc_renderer_stats_t first, warm, narrow, wide_again;
    xui_doc_rect_t wide_size, narrow_size; xui_doc_position_t hit;
    xui_doc_renderer_stats_t after_hit; int exact;
    double cold_start, cold_ms;
    uint64_t paragraph, text_id, styled_id, caret_visits = 0; size_t i;
    CHECK(text != NULL);
    for (i = 0; i < bytes; i++) text[i] = pattern[i % (sizeof(pattern) - 1)];
    text[bytes] = 0;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = insert(txn, 1, XUI_DOC_PARAGRAPH, NULL, 0);
    text_id = insert(txn, paragraph, XUI_DOC_TEXT, text, bytes);
    styled.iSize = sizeof(styled); styled.iKind = XUI_DOC_TEXT;
    styled.tAttributes.iMarks = XUI_DOC_BOLD;
    styled.sText = " bold-end"; styled.iTextBytes = strlen(styled.sText);
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND, &styled, &styled_id) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    desc.iSize = sizeof(desc); desc.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &desc, &view) == XUI_OK &&
        xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 936, 616}) == XUI_OK);
    cold_start = now_ms(); frame(context, target); cold_ms = now_ms() - cold_start;
    first = render_stats(view);
    CHECK(first.iBlocks == 1 && first.iMeasuredBlocks == 1 &&
        first.iShapedBytes > 0 && first.iShapedBytes < bytes / 4);
    CHECK(xuiDocumentViewGetContentSize(view, &wide_size, &exact) == XUI_OK && !exact);
    CHECK(xuiWidgetInvalidate(view, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) == XUI_OK);
    frame(context, target); warm = render_stats(view);
    CHECK(warm.iShapedBytes == first.iShapedBytes && warm.iLayoutPasses == first.iLayoutPasses);
    CHECK(warm.iDrawFragmentsExamined - first.iDrawFragmentsExamined < 20000);
    CHECK(xuiDocumentViewHitTest(view, 100, 100, &hit) == XUI_OK && hit.iNodeId == text_id);
    after_hit = render_stats(view);
    CHECK(after_hit.iHitFragmentsExamined - warm.iHitFragmentsExamined < 1000 &&
        after_hit.iShapedBytes == warm.iShapedBytes);
    CHECK(xuiInputViewport(context, 520, 640) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 520, 616}) == XUI_OK);
    frame(context, target); narrow = render_stats(view);
    CHECK(narrow.iLayoutPasses == first.iLayoutPasses + 1);
    CHECK(narrow.iShapedBytes > warm.iShapedBytes &&
        narrow.iShapedBytes - warm.iShapedBytes < bytes / 4);
    CHECK(xuiDocumentViewGetContentSize(view, &narrow_size, &exact) == XUI_OK && !exact &&
        narrow_size.height > wide_size.height);
    CHECK(xuiInputViewport(context, 960, 640) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 936, 616}) == XUI_OK);
    frame(context, target); wide_again = render_stats(view);
    CHECK(wide_again.iLayoutPasses == narrow.iLayoutPasses + 1 &&
        wide_again.iShapedBytes > narrow.iShapedBytes &&
        wide_again.iShapedBytes - narrow.iShapedBytes < bytes / 4);
    {
        xui_document_snapshot snapshot = NULL;
        xui_document_renderer reflow = NULL, fresh = NULL;
        xui_doc_rect_t reflow_size, fresh_size, shallow_before;
        static const uint64_t offsets[] = {0, 1000, 50000, 99999};
        size_t k;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentRendererCreate(context, NULL, &reflow) == XUI_OK);
        CHECK(xuiDocumentRendererCreate(context, NULL, &fresh) == XUI_OK);
        CHECK(xuiDocumentRendererSetSnapshot(reflow, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererSetSnapshot(fresh, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(reflow, 936, 0, 616) == XUI_OK);
        {
            xui_doc_position_t p = position(document, text_id, 1000);
            xui_doc_rect_t shallow;
            xui_doc_renderer_stats_t before = {0}, after = {0};
            before.iSize = after.iSize = sizeof(before);
            CHECK(xuiDocumentRendererGetStats(reflow, &before) == XUI_OK);
            CHECK(xuiDocumentRendererGetCaretRect(reflow, &p, &shallow) == XUI_OK);
            shallow_before = shallow;
            CHECK(xuiDocumentRendererGetStats(reflow, &after) == XUI_OK &&
                after.iShapedBytes == before.iShapedBytes);
        }
        {
            xui_doc_position_t p = position(document, text_id, bytes - 1);
            xui_doc_rect_t deep;
            CHECK(xuiDocumentRendererGetCaretRect(reflow, &p, &deep) == XUI_OK);
            p = position(document, text_id, 1000);
            CHECK(xuiDocumentRendererGetCaretRect(reflow, &p, &deep) == XUI_OK &&
                fabs(deep.x - shallow_before.x) < .01 &&
                fabs(deep.y - shallow_before.y) < .01);
        }
        CHECK(xuiDocumentRendererLayout(reflow, 520, 0, 616) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(fresh, 520, 0, 616) == XUI_OK);
        {
            xui_doc_position_t p = position(document, text_id, bytes - 1);
            xui_doc_rect_t deep;
            CHECK(xuiDocumentRendererGetCaretRect(fresh, &p, &deep) == XUI_OK);
        }
        CHECK(xuiDocumentRendererGetSize(reflow, &reflow_size, &exact) == XUI_OK && exact);
        CHECK(xuiDocumentRendererGetSize(fresh, &fresh_size, &exact) == XUI_OK && exact);
        CHECK(fabs(reflow_size.height - fresh_size.height) < .01);
        for (k = 0; k < sizeof(offsets) / sizeof(offsets[0]); k++) {
            xui_doc_position_t p = position(document, text_id, offsets[k]);
            xui_doc_rect_t a, b;
            xui_doc_renderer_stats_t before_caret = {0}, after_caret = {0};
            before_caret.iSize = after_caret.iSize = sizeof(before_caret);
            CHECK(xuiDocumentRendererGetStats(reflow, &before_caret) == XUI_OK);
            CHECK(xuiDocumentRendererGetCaretRect(reflow, &p, &a) == XUI_OK);
            CHECK(xuiDocumentRendererGetStats(reflow, &after_caret) == XUI_OK);
            if (k == 2) {
                caret_visits = after_caret.iCaretFragmentsExamined - before_caret.iCaretFragmentsExamined;
                CHECK(caret_visits < 100);
            }
            CHECK(xuiDocumentRendererGetCaretRect(fresh, &p, &b) == XUI_OK);
            CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
        }
        {
            xui_doc_position_t p = position(document, styled_id, 5);
            xui_doc_rect_t a, b;
            CHECK(xuiDocumentRendererGetCaretRect(reflow, &p, &a) == XUI_OK);
            CHECK(xuiDocumentRendererGetCaretRect(fresh, &p, &b) == XUI_OK);
            CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
        }
        xuiDocumentRendererRelease(reflow);
        xuiDocumentRendererRelease(fresh);
        xuiDocumentSnapshotRelease(snapshot);
    }
    {
        xui_document_snapshot snapshot = NULL;
        xui_document_renderer renderer = NULL, fresh = NULL;
        xui_doc_renderer_desc_t renderer_desc = {0};
        xui_doc_renderer_stats_t before_font = {0}, after_font = {0};
        xui_doc_position_t p = position(document, text_id, bytes - 1);
        xui_doc_rect_t before_rect, after_rect, expected;
        scale_font_switch switcher = {0};
        xui_font alternate = NULL;
        CHECK(proxy->tProxy.fontLoadMemory(&proxy->tProxy, &alternate, NULL, 0, 20, 0) == XUI_OK);
        switcher.current = xuiGetDefaultFont(context);
        renderer_desc.iSize = sizeof(renderer_desc);
        renderer_desc.onFont = scale_select_font;
        renderer_desc.pUser = &switcher;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &renderer) == XUI_OK);
        CHECK(xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(renderer, 936, 0, 616) == XUI_OK);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &p, &before_rect) == XUI_OK);
        before_font.iSize = after_font.iSize = sizeof(before_font);
        CHECK(xuiDocumentRendererGetStats(renderer, &before_font) == XUI_OK);
        switcher.current = alternate;
        CHECK(xuiDocumentRendererInvalidateFonts(renderer) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(renderer, 936, 0, 616) == XUI_OK);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &p, &after_rect) == XUI_OK);
        CHECK(xuiDocumentRendererGetStats(renderer, &after_font) == XUI_OK &&
            after_font.iShapedBytes - before_font.iShapedBytes >= bytes);
        CHECK(fabs(after_rect.y - before_rect.y) > 100);
        CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &fresh) == XUI_OK);
        CHECK(xuiDocumentRendererSetSnapshot(fresh, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(fresh, 936, 0, 616) == XUI_OK);
        CHECK(xuiDocumentRendererGetCaretRect(fresh, &p, &expected) == XUI_OK);
        CHECK(fabs(after_rect.y - expected.y) < .01);
        xuiDocumentRendererRelease(fresh);
        xuiDocumentRendererRelease(renderer);
        xuiDocumentSnapshotRelease(snapshot);
        proxy->tProxy.fontDestroy(&proxy->tProxy, alternate);
    }
    printf("DocumentView large paragraph: bytes=%llu cold_frame=%.2fms cold_shape=%llu width_switch_shape=%llu warm_draw_visits=%llu hit_visits=%llu caret_visits=%llu\n",
        (unsigned long long)bytes, cold_ms,
        (unsigned long long)first.iShapedBytes,
        (unsigned long long)(narrow.iShapedBytes - warm.iShapedBytes),
        (unsigned long long)(warm.iDrawFragmentsExamined - first.iDrawFragmentsExamined),
        (unsigned long long)(after_hit.iHitFragmentsExamined - warm.iHitFragmentsExamined),
        (unsigned long long)caret_visits);
    CHECK(xuiSetRootWidget(context, NULL) == XUI_OK);
    xuiWidgetDestroy(view); xuiDocumentRelease(document); free(text);
}
static void paragraph_prefix_geometry(xui_context context, const char* pattern,
    int expect_prefix)
{
    const size_t unit = strlen(pattern), first_bytes = unit * 128,
        second_bytes = unit * 1800;
    char *first = malloc(first_bytes + 1), *second = malloc(second_bytes + 1);
    xui_document document; xui_document_transaction txn;
    xui_document_snapshot snapshot; xui_document_renderer renderer;
    xui_doc_node_desc_t styled = {0}; xui_doc_renderer_stats_t stats = {0};
    xui_doc_rect_t before[2], after, size;
    xui_doc_rect_t lazy_before;
    xui_doc_position_t lazy_position;
    xui_doc_range_t lazy_range = {0};
    xui_doc_position_t hit_before, hit_after;
    uint64_t paragraph, first_id, second_id, prefix_rects = 0, full_rects = 0;
    size_t i; int exact;
    CHECK(first && second);
    for (i = 0; i < first_bytes; i += unit) memcpy(first + i, pattern, unit);
    for (i = 0; i < second_bytes; i += unit) memcpy(second + i, pattern, unit);
    first[first_bytes] = second[second_bytes] = 0;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = insert(txn, 1, XUI_DOC_PARAGRAPH, NULL, 0);
    first_id = insert(txn, paragraph, XUI_DOC_TEXT, first, first_bytes);
    styled.iSize = sizeof(styled); styled.iKind = XUI_DOC_TEXT;
    styled.tAttributes.iMarks = XUI_DOC_ITALIC;
    styled.sText = second; styled.iTextBytes = second_bytes;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
        &styled, &second_id) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(renderer, 936, 0, 616) == XUI_OK);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK);
    printf("Paragraph prefix case: unit=%llu bytes=%llu initial_shaped=%llu run_input=%llu expect=%d\n",
        (unsigned long long)unit,
        (unsigned long long)(first_bytes + second_bytes),
        (unsigned long long)stats.iShapedBytes, (unsigned long long)stats.iTextRunBytes, expect_prefix);
    CHECK((expect_prefix ? stats.iTextRunBytes < (first_bytes + second_bytes) / 3 :
            stats.iTextRunBytes >= first_bytes + second_bytes));
    CHECK(stats.iShapedBytes <= stats.iTextRunBytes * 4);
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK &&
        exact == !expect_prefix);
    {
        xui_doc_position_t p = position(document, first_id, unit * 40);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &p, &before[0]) == XUI_OK);
        p = position(document, second_id, unit * 20);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &p, &before[1]) == XUI_OK);
    }
    CHECK(xuiDocumentRendererHitTest(renderer, before[1].x + .1,
        before[1].y + before[1].height / 2, &hit_before) == XUI_OK);
    lazy_position = position(document, second_id, unit * 400);
    lazy_range.tAnchor = position(document, first_id, unit * 127);
    lazy_range.tCaret = lazy_position;
    if (expect_prefix) {
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &lazy_position,
            &lazy_before) == XUI_OK);
        CHECK(xuiDocumentRendererGetRangeRects(renderer, &lazy_range,
            NULL, 0, &prefix_rects) == XUI_OK && prefix_rects > 0);
        CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && !exact);
    }
    if (expect_prefix)
        CHECK(xuiDocumentRendererLayout(renderer, 936, size.height * .5, 616) == XUI_OK);
    {
        xui_doc_position_t p = position(document, second_id, second_bytes - unit);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &p, &after) == XUI_OK);
    }
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact);
    {
        xui_doc_position_t p = position(document, first_id, unit * 40);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &p, &after) == XUI_OK &&
            fabs(after.x - before[0].x) < .01 && fabs(after.y - before[0].y) < .01);
        p = position(document, second_id, unit * 20);
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &p, &after) == XUI_OK &&
            fabs(after.x - before[1].x) < .01 && fabs(after.y - before[1].y) < .01);
    }
    CHECK(xuiDocumentRendererHitTest(renderer, before[1].x + .1,
        before[1].y + before[1].height / 2, &hit_after) == XUI_OK &&
        hit_after.iNodeId == hit_before.iNodeId &&
        hit_after.iOffset == hit_before.iOffset);
    if (expect_prefix)
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &lazy_position,
            &after) == XUI_OK && fabs(after.x - lazy_before.x) < .01 &&
            fabs(after.y - lazy_before.y) < .01);
    if (expect_prefix)
        CHECK(xuiDocumentRendererGetRangeRects(renderer, &lazy_range,
            NULL, 0, &full_rects) == XUI_OK && full_rects == prefix_rects);
    xuiDocumentRendererRelease(renderer); xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document); free(first); free(second);
}
static void unicode_run_metrics_prefix(void)
{
    static const char pattern[] = "Alpha beta gamma. ";
    const size_t unit = sizeof(pattern) - 1, lead = unit * 4000,
        suffix = unit * 1000, bytes = lead + 3 + suffix;
    char* text = malloc(bytes + 1);
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_document document; xui_document_transaction txn;
    xui_document_snapshot snapshot; xui_document_renderer renderer;
    xui_doc_position_t near_pos, middle_pos, later_pos, deep;
    xui_doc_rect_t before, middle_before, later_before, after, size;
    xui_doc_renderer_stats_t stats = {0}, extended = {0}, extended2 = {0};
    uint64_t paragraph, text_id; size_t i; int exact;
    double full_height;
    CHECK(text != NULL);
    for (i = 0; i < lead; i += unit) memcpy(text + i, pattern, unit);
    memcpy(text + lead, "\xe4\xb8\xad", 3);
    for (i = 0; i < suffix; i += unit) memcpy(text + lead + 3 + i, pattern, unit);
    text[bytes] = 0;
    xuiTestProxyInit(&proxy);
    scale_base_shape = proxy.tProxy.textShape;
    proxy.tProxy.textShape = scale_late_tall_shape;
    CHECK(xuiCreate(&context) == XUI_OK &&
        xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "scale.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = insert(txn, 1, XUI_DOC_PARAGRAPH, NULL, 0);
    text_id = insert(txn, paragraph, XUI_DOC_TEXT, text, bytes);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(renderer, 936, 0, 616) == XUI_OK);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK);
    if(stats.iShapedBytes>=bytes/4)fprintf(stderr,"Unicode metrics prefix: source=%zu shaped=%llu run=%llu limit=%zu\n",bytes,
        (unsigned long long)stats.iShapedBytes,(unsigned long long)stats.iTextRunBytes,bytes/4);
    CHECK(stats.iShapedBytes < bytes / 4);
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && !exact);
    near_pos = position(document, text_id, unit * 30);
    middle_pos = position(document, text_id, unit * 500);
    later_pos = position(document, text_id, unit * 1500);
    deep = position(document, text_id, bytes - unit);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &near_pos, &before) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(renderer, 936, 900, 616) == XUI_OK);
    extended.iSize = sizeof(extended);
    CHECK(xuiDocumentRendererGetStats(renderer, &extended) == XUI_OK &&
        extended.iShapedBytes > stats.iShapedBytes &&
        extended.iTextRunBytes - stats.iTextRunBytes < bytes / 4 &&
        extended.iShapedBytes - stats.iShapedBytes < (extended.iTextRunBytes - stats.iTextRunBytes) * 4);
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && !exact);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &near_pos, &after) == XUI_OK &&
        fabs(after.x - before.x) < .01 && fabs(after.y - before.y) < .01);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &middle_pos, &middle_before) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && !exact);
    scale_fail_shape_after = 2;
    CHECK(xuiDocumentRendererLayout(renderer, 936, 2000, 616) ==
        XUI_ERROR_OUT_OF_MEMORY);
    scale_fail_shape_after = 0;
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && !exact);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &near_pos, &after) == XUI_OK &&
        fabs(after.x - before.x) < .01 && fabs(after.y - before.y) < .01);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &middle_pos, &after) == XUI_OK &&
        fabs(after.x - middle_before.x) < .01 &&
        fabs(after.y - middle_before.y) < .01);
    CHECK(xuiDocumentRendererLayout(renderer, 936, 2000, 616) == XUI_OK);
    extended2.iSize = sizeof(extended2);
    CHECK(xuiDocumentRendererGetStats(renderer, &extended2) == XUI_OK &&
        extended2.iShapedBytes > extended.iShapedBytes &&
        extended2.iTextRunBytes - extended.iTextRunBytes < bytes / 2 &&
        extended2.iShapedBytes - extended.iShapedBytes < (extended2.iTextRunBytes - extended.iTextRunBytes) * 4);
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && !exact);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &near_pos, &after) == XUI_OK &&
        fabs(after.x - before.x) < .01 && fabs(after.y - before.y) < .01);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &middle_pos, &after) == XUI_OK &&
        fabs(after.x - middle_before.x) < .01 &&
        fabs(after.y - middle_before.y) < .01);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &later_pos, &later_before) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && !exact);
    CHECK(xuiDocumentRendererLayout(renderer, 936, 0, 1000000) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &deep, &after) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact);
    full_height = size.height;
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &near_pos, &after) == XUI_OK &&
        fabs(after.x - before.x) < .01 && fabs(after.y - before.y) < .01);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &middle_pos, &after) == XUI_OK &&
        fabs(after.x - middle_before.x) < .01 &&
        fabs(after.y - middle_before.y) < .01);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &later_pos, &after) == XUI_OK &&
        fabs(after.x - later_before.x) < .01 &&
        fabs(after.y - later_before.y) < .01);
    {
        xui_document_renderer cold = NULL;
        xui_doc_renderer_stats_t cold_stats = {0}, caret_stats = {0};
        xui_doc_position_t probe = position(document, text_id, unit * 1800);
        xui_doc_rect_t expected;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &probe, &expected) == XUI_OK);
        CHECK(xuiDocumentRendererCreate(context, NULL, &cold) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(cold, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(cold, 936, 0, 2000) == XUI_OK);
        cold_stats.iSize = sizeof(cold_stats);
        CHECK(xuiDocumentRendererGetStats(cold, &cold_stats) == XUI_OK &&
            cold_stats.iShapedBytes > stats.iShapedBytes &&
            cold_stats.iShapedBytes < bytes);
        CHECK(xuiDocumentRendererGetSize(cold, &size, &exact) == XUI_OK && !exact);
        CHECK(xuiDocumentRendererGetCaretRect(cold, &near_pos, &after) == XUI_OK &&
            fabs(after.x - before.x) < .01 && fabs(after.y - before.y) < .01);
        scale_fail_shape_after = 2;
        CHECK(xuiDocumentRendererGetCaretRect(cold, &probe, &after) ==
            XUI_ERROR_OUT_OF_MEMORY);
        scale_fail_shape_after = 0;
        CHECK(xuiDocumentRendererGetSize(cold, &size, &exact) == XUI_OK && !exact);
        CHECK(xuiDocumentRendererGetCaretRect(cold, &near_pos, &after) == XUI_OK &&
            fabs(after.x - before.x) < .01 && fabs(after.y - before.y) < .01);
        CHECK(xuiDocumentRendererGetCaretRect(cold, &probe, &after) == XUI_OK &&
            fabs(after.x - expected.x) < .01 &&
            fabs(after.y - expected.y) < .01);
        caret_stats.iSize = sizeof(caret_stats);
        CHECK(xuiDocumentRendererGetStats(cold, &caret_stats) == XUI_OK &&
            caret_stats.iShapedBytes > cold_stats.iShapedBytes &&
            caret_stats.iTextRunBytes < bytes * 2 / 3 && caret_stats.iShapedBytes < caret_stats.iTextRunBytes * 4);
        CHECK(xuiDocumentRendererGetSize(cold, &size, &exact) == XUI_OK && !exact);
        CHECK(xuiDocumentRendererGetCaretRect(cold, &deep, &after) == XUI_OK);
        CHECK(xuiDocumentRendererGetSize(cold, &size, &exact) == XUI_OK && exact);
        xuiDocumentRendererRelease(cold);
    }
    {
        xui_document_renderer direct = NULL;
        xui_doc_renderer_stats_t direct_stats = {0};
        xui_doc_position_t probe = position(document, text_id, unit * 1800);
        xui_doc_rect_t expected;
        CHECK(xuiDocumentRendererGetCaretRect(renderer, &probe, &expected) == XUI_OK);
        CHECK(xuiDocumentRendererCreate(context, NULL, &direct) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(direct, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(direct, 936, 0, 0) == XUI_OK);
        CHECK(xuiDocumentRendererGetCaretRect(direct, &probe, &after) == XUI_OK &&
            fabs(after.x - expected.x) < .01 &&
            fabs(after.y - expected.y) < .01);
        direct_stats.iSize = sizeof(direct_stats);
        CHECK(xuiDocumentRendererGetStats(direct, &direct_stats) == XUI_OK &&
            direct_stats.iTextRunBytes < bytes * 2 / 3 && direct_stats.iShapedBytes < direct_stats.iTextRunBytes * 4);
        CHECK(xuiDocumentRendererGetSize(direct, &size, &exact) == XUI_OK &&
            !exact);
        xuiDocumentRendererRelease(direct);
    }
    {
        xui_document_renderer lazy = NULL;
        xui_doc_range_t range = {0};
        xui_doc_rect_t *expected = NULL, *actual = NULL;
        xui_doc_renderer_stats_t range_stats = {0};
        uint64_t expected_count = 0, actual_count = 0, returned = 0, j;
        range.tAnchor = position(document, text_id, unit * 1780);
        range.tCaret = position(document, text_id, unit * 1800);
        CHECK(xuiDocumentRendererGetRangeRects(renderer, &range, NULL, 0,
            &expected_count) == XUI_OK && expected_count > 0);
        CHECK(xuiDocumentRendererCreate(context, NULL, &lazy) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(lazy, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(lazy, 936, 0, 0) == XUI_OK);
        CHECK(xuiDocumentRendererGetRangeRects(lazy, &range, NULL, 0,
            &actual_count) == XUI_OK && actual_count == expected_count);
        range_stats.iSize = sizeof(range_stats);
        CHECK(xuiDocumentRendererGetStats(lazy, &range_stats) == XUI_OK &&
            range_stats.iTextRunBytes < bytes * 2 / 3 && range_stats.iShapedBytes < range_stats.iTextRunBytes * 4);
        CHECK(xuiDocumentRendererGetSize(lazy, &size, &exact) == XUI_OK && !exact);
        expected = malloc((size_t)expected_count * sizeof(*expected));
        actual = malloc((size_t)actual_count * sizeof(*actual));
        CHECK(expected && actual);
        CHECK(xuiDocumentRendererGetRangeRects(renderer, &range, expected,
            expected_count, &returned) == XUI_OK && returned == expected_count);
        CHECK(xuiDocumentRendererGetRangeRects(lazy, &range, actual,
            actual_count, &returned) == XUI_OK && returned == actual_count);
        for (j = 0; j < actual_count; j++)
            CHECK(fabs(actual[j].x - expected[j].x) < .01 &&
                fabs(actual[j].y - expected[j].y) < .01 &&
                fabs(actual[j].width - expected[j].width) < .01 &&
                fabs(actual[j].height - expected[j].height) < .01);
        free(expected); free(actual); xuiDocumentRendererRelease(lazy);
    }
    {
        xui_document_renderer cold_full = NULL;
        CHECK(xuiDocumentRendererCreate(context, NULL, &cold_full) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(cold_full, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(cold_full, 936, 0, 1000000) == XUI_OK);
        CHECK(xuiDocumentRendererGetSize(cold_full, &size, &exact) == XUI_OK &&
            exact && fabs(size.height - full_height) < .01);
        xuiDocumentRendererRelease(cold_full);
    }
    xuiDocumentRendererRelease(renderer); xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document); xuiDestroy(context);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font); free(text);
    printf("Unicode long paragraph: initial=%llu first_extend=%llu second_extend=%llu, late tall glyph keeps prefix geometry\n",
        (unsigned long long)stats.iShapedBytes,
        (unsigned long long)(extended.iShapedBytes - stats.iShapedBytes),
        (unsigned long long)(extended2.iShapedBytes - extended.iShapedBytes));
}
static void inline_object_geometry_continue(xui_context context)
{
    static const char pattern[] = "Alpha beta gamma. ";
    const size_t unit = sizeof(pattern) - 1;
    const size_t lead_bytes = unit * 1800, tail_bytes = unit * 3000;
    char *lead = malloc(lead_bytes + 1), *tail = malloc(tail_bytes + 1);
    xui_document document; xui_document_transaction txn;
    xui_document_snapshot snapshot; xui_document_renderer lazy, full;
    xui_doc_rect_t expected, actual, size;
    xui_doc_renderer_stats_t stats = {0};
    uint64_t paragraph, first_id, image_id, text_rect_shaped = 0; size_t i; int exact;
    CHECK(lead && tail);
    for (i = 0; i < lead_bytes; i += unit) memcpy(lead + i, pattern, unit);
    for (i = 0; i < tail_bytes; i += unit) memcpy(tail + i, pattern, unit);
    lead[lead_bytes] = tail[tail_bytes] = 0;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = insert(txn, 1, XUI_DOC_PARAGRAPH, NULL, 0);
    first_id = insert(txn, paragraph, XUI_DOC_TEXT, lead, lead_bytes);
    image_id = insert(txn, paragraph, XUI_DOC_IMAGE, "alt", 3);
    insert(txn, paragraph, XUI_DOC_TEXT, tail, tail_bytes);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &lazy) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(lazy, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &full) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(full, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(lazy, 936, 0, 616) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(full, 936, 0, 1000000) == XUI_OK);
    {
        xui_document_renderer cold_text;
        CHECK(xuiDocumentRendererGetNodeRect(full, first_id, &expected) == XUI_OK);
        CHECK(xuiDocumentRendererCreate(context, NULL, &cold_text) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(cold_text, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(cold_text, 936, 0, 0) == XUI_OK);
        CHECK(xuiDocumentRendererGetNodeRect(cold_text, first_id, &actual) == XUI_OK &&
            fabs(actual.x - expected.x) < .01 &&
            fabs(actual.y - expected.y) < .01 &&
            fabs(actual.width - expected.width) < .01 &&
            fabs(actual.height - expected.height) < .01);
        stats.iSize = sizeof(stats);
        CHECK(xuiDocumentRendererGetStats(cold_text, &stats) == XUI_OK &&
            stats.iTextRunBytes < (lead_bytes + tail_bytes) * 2 / 3 && stats.iShapedBytes < stats.iTextRunBytes * 4);
        text_rect_shaped = stats.iShapedBytes;
        CHECK(xuiDocumentRendererGetSize(cold_text, &size, &exact) == XUI_OK && !exact);
        xuiDocumentRendererRelease(cold_text);
    }
    CHECK(xuiDocumentRendererGetNodeRect(full, image_id, &expected) == XUI_OK);
    CHECK(xuiDocumentRendererGetNodeRect(lazy, image_id, &actual) == XUI_OK &&
        fabs(actual.x - expected.x) < .01 &&
        fabs(actual.y - expected.y) < .01 &&
        fabs(actual.width - expected.width) < .01 &&
        fabs(actual.height - expected.height) < .01);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(lazy, &stats) == XUI_OK &&
        stats.iTextRunBytes < (lead_bytes + tail_bytes) * 2 / 3 && stats.iShapedBytes < stats.iTextRunBytes * 4);
    CHECK(xuiDocumentRendererGetSize(lazy, &size, &exact) == XUI_OK && !exact);
    {
        xui_document_renderer cold;
        CHECK(xuiDocumentRendererCreate(context, NULL, &cold) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(cold, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(cold, 936, 0, 0) == XUI_OK);
        CHECK(xuiDocumentRendererGetNodeRect(cold, image_id, &actual) == XUI_OK &&
            fabs(actual.x - expected.x) < .01 &&
            fabs(actual.y - expected.y) < .01 &&
            fabs(actual.width - expected.width) < .01 &&
            fabs(actual.height - expected.height) < .01);
        stats.iSize = sizeof(stats);
        CHECK(xuiDocumentRendererGetStats(cold, &stats) == XUI_OK &&
            stats.iTextRunBytes < (lead_bytes + tail_bytes) * 2 / 3 && stats.iShapedBytes < stats.iTextRunBytes * 4);
        CHECK(xuiDocumentRendererGetSize(cold, &size, &exact) == XUI_OK &&
            !exact);
        xuiDocumentRendererRelease(cold);
    }
    for (i = 1; i <= 2; i++) {
        xui_document_renderer range_renderer;
        xui_doc_range_t range = {0};
        xui_doc_rect_t *expected_rects, *actual_rects;
        uint64_t expected_count = 0, actual_count = 0, returned = 0, j;
        range.tAnchor = position(document, first_id, lead_bytes - unit * 5);
        range.tCaret = position(document, paragraph, i);
        range.tCaret.iKind = XUI_DOC_POSITION_GAP;
        CHECK(xuiDocumentRendererGetRangeRects(full, &range, NULL, 0,
            &expected_count) == XUI_OK && expected_count > 0);
        CHECK(xuiDocumentRendererCreate(context, NULL, &range_renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(range_renderer, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(range_renderer, 936, 0,
            i == 2 ? 616 : 0) == XUI_OK);
        CHECK(xuiDocumentRendererGetRangeRects(range_renderer, &range,
            NULL, 0, &actual_count) == XUI_OK && actual_count == expected_count);
        stats.iSize = sizeof(stats);
        CHECK(xuiDocumentRendererGetStats(range_renderer, &stats) == XUI_OK &&
            stats.iTextRunBytes < (lead_bytes + tail_bytes) * 2 / 3 && stats.iShapedBytes < stats.iTextRunBytes * 4);
        CHECK(xuiDocumentRendererGetSize(range_renderer, &size, &exact) ==
            XUI_OK && !exact);
        expected_rects = malloc((size_t)expected_count * sizeof(*expected_rects));
        actual_rects = malloc((size_t)actual_count * sizeof(*actual_rects));
        CHECK(expected_rects && actual_rects);
        CHECK(xuiDocumentRendererGetRangeRects(full, &range,
            expected_rects, expected_count, &returned) == XUI_OK &&
            returned == expected_count);
        CHECK(xuiDocumentRendererGetRangeRects(range_renderer, &range,
            actual_rects, actual_count, &returned) == XUI_OK &&
            returned == actual_count);
        for (j = 0; j < actual_count; j++)
            CHECK(fabs(actual_rects[j].x - expected_rects[j].x) < .01 &&
                fabs(actual_rects[j].y - expected_rects[j].y) < .01 &&
                fabs(actual_rects[j].width - expected_rects[j].width) < .01 &&
                fabs(actual_rects[j].height - expected_rects[j].height) < .01);
        free(actual_rects); free(expected_rects);
        xuiDocumentRendererRelease(range_renderer);
    }
    xuiDocumentRendererRelease(full); xuiDocumentRendererRelease(lazy);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(document);
    free(tail); free(lead);
    printf("Long paragraph text-node rect: %llu/%llu bytes shaped; cold rectangle matches full layout\n",
        (unsigned long long)text_rect_shaped,
        (unsigned long long)(lead_bytes + tail_bytes));
}
static void unbreakable_paragraph(xui_context context)
{
    const size_t bytes = 128 * 1024;
    char* text = malloc(bytes + 1);
    xui_document document; xui_document_transaction txn;
    xui_document_snapshot snapshot; xui_document_renderer renderer, full;
    xui_doc_renderer_stats_t stats = {0}, extended = {0};
    xui_doc_rect_t size, near_rect, late_rect, expected;
    xui_doc_position_t near_pos, late_pos, hit;
    int exact;
    uint64_t paragraph, text_id;
    CHECK(text != NULL);
    memset(text, 'A', bytes); text[bytes] = 0;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = insert(txn, 1, XUI_DOC_PARAGRAPH, NULL, 0);
    text_id = insert(txn, paragraph, XUI_DOC_TEXT, text, bytes);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(renderer, 936, 0, 616) == XUI_OK);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
        stats.iShapedBytes > 0 && stats.iShapedBytes < bytes / 4);
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && !exact);
    near_pos = position(document, text_id, 128);
    late_pos = position(document, text_id, bytes - 128);
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &near_pos, &near_rect) == XUI_OK &&
        xuiDocumentRendererGetCaretRect(renderer, &late_pos, &late_rect) == XUI_OK &&
        xuiDocumentRendererHitTest(renderer, late_rect.x + .1,
            late_rect.y + late_rect.height / 2, &hit) == XUI_OK &&
        hit.iNodeId == text_id && hit.iOffset == late_pos.iOffset);
    extended.iSize = sizeof(extended);
    CHECK(xuiDocumentRendererGetStats(renderer, &extended) == XUI_OK &&
        extended.iShapedBytes > stats.iShapedBytes &&
        xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact);
    CHECK(xuiDocumentRendererCreate(context, NULL, &full) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(full, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererLayout(full, 936, 0, 1000000) == XUI_OK &&
        xuiDocumentRendererGetCaretRect(full, &near_pos, &expected) == XUI_OK &&
        fabs(expected.x - near_rect.x) < .01 &&
        fabs(expected.y - near_rect.y) < .01 &&
        xuiDocumentRendererGetCaretRect(full, &late_pos, &expected) == XUI_OK &&
        fabs(expected.x - late_rect.x) < .01 &&
        fabs(expected.y - late_rect.y) < .01);
    xuiDocumentRendererRelease(full); xuiDocumentRendererRelease(renderer);
    xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document); free(text);
    printf("Unbreakable paragraph: initial=%llu/%llu shaped bytes; deep caret, hit and full geometry passed\n",
        (unsigned long long)stats.iShapedBytes, (unsigned long long)bytes);
}
static void underestimated_prefix_scroll(xui_context context)
{
    static const char pattern[] = "Alpha beta gamma delta. ";
    const size_t unit = sizeof(pattern) - 1, first_bytes = unit * 700,
        second_bytes = unit * 1700, tail_bytes = unit * 200;
    char *first = malloc(first_bytes + 1), *second = malloc(second_bytes + 1),
        *tail = malloc(tail_bytes + 1);
    xui_document document; xui_document_transaction txn;
    xui_document_snapshot snapshot; xui_document_renderer renderer;
    xui_doc_node_desc_t styled = {0}; xui_doc_renderer_stats_t stats = {0};
    xui_doc_rect_t size, tail_rect; xui_doc_position_t hit;
    uint64_t paragraph, tail_paragraph, first_id, styled_id, tail_id;
    size_t i; int exact; double target;
    CHECK(first && second && tail);
    for (i = 0; i < first_bytes; i += unit) memcpy(first + i, pattern, unit);
    for (i = 0; i < second_bytes; i += unit) memcpy(second + i, pattern, unit);
    for (i = 0; i < tail_bytes; i += unit) memcpy(tail + i, pattern, unit);
    first[first_bytes] = second[second_bytes] = tail[tail_bytes] = 0;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = insert(txn, 1, XUI_DOC_PARAGRAPH, NULL, 0);
    first_id = insert(txn, paragraph, XUI_DOC_TEXT, first, first_bytes);
    styled.iSize = sizeof(styled); styled.iKind = XUI_DOC_TEXT;
    styled.tAttributes.fFontSize = 60;
    styled.sText = second; styled.iTextBytes = second_bytes;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
        &styled, &styled_id) == XUI_OK);
    tail_paragraph = insert(txn, 1, XUI_DOC_PARAGRAPH, NULL, 0);
    tail_id = insert(txn, tail_paragraph, XUI_DOC_TEXT, tail, tail_bytes);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(renderer, 936, 0, 616) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && !exact);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
        stats.iShapedBytes < first_bytes + second_bytes);
    CHECK(xuiDocumentRendererGetNodeRect(renderer, tail_paragraph, &tail_rect) == XUI_OK);
    target = tail_rect.y + 200;
    CHECK(target < tail_rect.y + tail_rect.height);
    CHECK(xuiDocumentRendererLayout(renderer, 936, target, 616) == XUI_OK);
    CHECK(xuiDocumentRendererHitTest(renderer, 100, target + 1, &hit) == XUI_OK &&
        (hit.iNodeId == first_id || hit.iNodeId == styled_id) &&
        hit.iNodeId != tail_id);
    CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
        stats.iShapedBytes >= first_bytes + second_bytes);
    xuiDocumentRendererRelease(renderer); xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document); free(first); free(second); free(tail);
}
static void overestimated_prefix_hit(xui_context context)
{
    static const char pattern[] = "Alpha beta gamma delta. ";
    const size_t unit = sizeof(pattern) - 1, first_bytes = unit * 700,
        second_bytes = unit * 1700, tail_bytes = unit * 200;
    char *first = malloc(first_bytes + 1), *second = malloc(second_bytes + 1),
        *tail = malloc(tail_bytes + 1);
    xui_document document; xui_document_transaction txn;
    xui_document_snapshot snapshot; xui_document_renderer lazy, oracle;
    xui_doc_node_desc_t large = {0}; xui_doc_renderer_stats_t stats = {0};
    xui_doc_rect_t estimated, actual, deep; xui_doc_position_t hit;
    uint64_t paragraph, first_id, second_id, tail_paragraph, tail_id;
    size_t i; double target;
    CHECK(first && second && tail);
    for (i = 0; i < first_bytes; i += unit) memcpy(first + i, pattern, unit);
    for (i = 0; i < second_bytes; i += unit) memcpy(second + i, pattern, unit);
    for (i = 0; i < tail_bytes; i += unit) memcpy(tail + i, pattern, unit);
    first[first_bytes] = second[second_bytes] = tail[tail_bytes] = 0;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    paragraph = insert(txn, 1, XUI_DOC_PARAGRAPH, NULL, 0);
    large.iSize = sizeof(large); large.iKind = XUI_DOC_TEXT;
    large.tAttributes.fFontSize = 60;
    large.sText = first; large.iTextBytes = first_bytes;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
        &large, &first_id) == XUI_OK && first_id != 0);
    second_id = insert(txn, paragraph, XUI_DOC_TEXT, second, second_bytes);
    tail_paragraph = insert(txn, 1, XUI_DOC_PARAGRAPH, NULL, 0);
    tail_id = insert(txn, tail_paragraph, XUI_DOC_TEXT, tail, tail_bytes);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &lazy) == XUI_OK &&
        xuiDocumentRendererCreate(context, NULL, &oracle) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(lazy, snapshot, NULL) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(oracle, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(lazy, 936, 0, 616) == XUI_OK);
    CHECK(xuiDocumentRendererGetNodeRect(lazy, tail_paragraph, &estimated) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(oracle, 936, 0, 616) == XUI_OK);
    {
        xui_doc_position_t p = position(document, second_id, second_bytes - 1);
        CHECK(xuiDocumentRendererGetCaretRect(oracle, &p, &deep) == XUI_OK);
    }
    CHECK(xuiDocumentRendererGetNodeRect(oracle, tail_paragraph, &actual) == XUI_OK &&
        estimated.y > actual.y + 300);
    target = actual.y + 200;
    CHECK(target < actual.y + actual.height && target < estimated.y);
    CHECK(xuiDocumentRendererHitTest(lazy, 100, target, &hit) == XUI_OK &&
        hit.iNodeId == tail_id);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(lazy, &stats) == XUI_OK &&
        stats.iShapedBytes >= first_bytes + second_bytes);
    xuiDocumentRendererRelease(lazy); xuiDocumentRendererRelease(oracle);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(document);
    free(first); free(second); free(tail);
}
static void renderer_cache_budget(xui_context context)
{
    static const char line[] = "A cached Markdown paragraph with enough glyphs to exercise shaping, wrapping, and fragment storage.";
    char* source = malloc(256 * (sizeof(line) + 16));
    size_t capacity = 256 * (sizeof(line) + 16), bytes = 0;
    xui_doc_desc_t document_desc = {0};
    xui_document document = NULL;
    xui_document_snapshot snapshot = NULL;
    int mode, i;
    CHECK(source != NULL);
    for (i = 0; i < 256; i++) {
        int added = snprintf(source + bytes, capacity - bytes, "%s %03d\n\n", line, i);
        CHECK(added > 0 && (size_t)added < capacity - bytes);
        bytes += (size_t)added;
    }
    document_desc.iSize = sizeof(document_desc); document_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&document_desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, source, bytes) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    for (mode = XUI_DOC_VISUAL; mode <= XUI_DOC_SOURCE_TEXT; mode++) {
        xui_doc_renderer_desc_t renderer_desc = {0};
        xui_document_renderer renderer = NULL;
        xui_doc_renderer_stats_t stats = {0};
        xui_doc_rect_t size;
        uint64_t shaped_after_scroll;
        int exact;
        renderer_desc.iSize = sizeof(renderer_desc);
        renderer_desc.iLayoutCacheBudgetBytes = 65536;
        CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &renderer) == XUI_OK);
        if (mode == XUI_DOC_SOURCE_TEXT) CHECK(xuiDocumentRendererSetMode(renderer, mode) == XUI_OK);
        CHECK(xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(renderer, 640, 0, 24) == XUI_OK);
        CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK);
        CHECK(size.height > 1000);
        CHECK(xuiDocumentRendererLayout(renderer, 640, 0, 1000000) == XUI_OK);
        CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact);
        stats.iSize = sizeof(stats);
        CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
            stats.iLayoutCacheBytes > stats.iLayoutCacheBudgetBytes);
        CHECK(xuiDocumentRendererLayout(renderer, 640, 0, 24) == XUI_OK);
        CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && !exact);
        for (i = 1; i <= 3; i++) {
            CHECK(xuiDocumentRendererLayout(renderer, 640, size.height * i / 5, 24) == XUI_OK);
            stats.iSize = sizeof(stats);
            CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK);
            CHECK(stats.iLayoutCacheBudgetBytes == renderer_desc.iLayoutCacheBudgetBytes);
            CHECK(stats.iLayoutCacheBytes <= stats.iLayoutCacheBudgetBytes);
        }
        CHECK(stats.iLayoutCacheEvictions > 0);
        shaped_after_scroll = stats.iShapedBytes;
        CHECK(xuiDocumentRendererLayout(renderer, 640, 0, 24) == XUI_OK);
        CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK);
        CHECK(stats.iLayoutCacheBytes <= stats.iLayoutCacheBudgetBytes);
        CHECK(stats.iShapedBytes > shaped_after_scroll);
        shaped_after_scroll = stats.iShapedBytes;
        CHECK(xuiDocumentRendererLayout(renderer, 640, 0, 24) == XUI_OK);
        CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK);
        CHECK(stats.iShapedBytes == shaped_after_scroll);
        CHECK(xuiDocumentRendererLayout(renderer, 520, 0, 24) == XUI_OK);
        CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK);
        CHECK(stats.iLayoutCacheBytes <= stats.iLayoutCacheBudgetBytes);
        if (mode == XUI_DOC_VISUAL) {
            uint64_t cache_bytes = stats.iLayoutCacheBytes;
            uint64_t shaped_bytes = stats.iShapedBytes;
            CHECK(xuiDocumentRendererInvalidateObjects(renderer) == XUI_OK);
            CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
                stats.iLayoutCacheBytes == cache_bytes && stats.iShapedBytes == shaped_bytes);
        }
        if (mode == XUI_DOC_SOURCE_TEXT) {
            xui_doc_source_patch_t patch = {0};
            xui_document_prepare prepare = NULL;
            patch.iSize = sizeof(patch);
            patch.iStart = patch.iEnd = bytes * 3 / 5;
            patch.sText = "Z"; patch.iTextBytes = 1;
            CHECK(xuiDocumentPrepareSource(document, NULL, &patch, 1, &prepare) == XUI_OK);
            CHECK(xuiDocumentRendererSetSourceInput(renderer, prepare) == XUI_OK);
            CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK);
            CHECK(stats.iSourceIncrementalUpdates > 0 && stats.iLayoutCacheBytes <= stats.iLayoutCacheBudgetBytes);
            CHECK(xuiDocumentRendererLayout(renderer, 640, size.height * 3 / 5, 24) == XUI_OK);
            CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK);
            CHECK(stats.iLayoutCacheBytes <= stats.iLayoutCacheBudgetBytes);
            CHECK(xuiDocumentRendererSetSourceInput(renderer, NULL) == XUI_OK);
            CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK && stats.iLayoutCacheBytes == 0);
            CHECK(xuiDocumentCancelPrepare(document) == XUI_OK);
            xuiDocumentPrepareRelease(prepare);
        }
        xuiDocumentRendererRelease(renderer);
    }
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(document); free(source);
    puts("Renderer VISUAL/SOURCE layout cache budgets and offscreen eviction passed");
}
static void long_code_block(xui_context context, xui_surface target, xui_test_proxy_state_t* proxy)
{
    const int lines = 2048;
    char* text = malloc((size_t)lines * 80);
    size_t bytes = 0, first_boundary = 0;
    uint64_t code, middle = 0, last = 0;
    xui_document document = NULL;
    xui_document_transaction txn = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_document_change_set changes = NULL;
    xui_document_renderer sparse = NULL, full = NULL;
    xui_doc_renderer_desc_t renderer_desc = {0};
    xui_doc_renderer_stats_t stats = {0};
    xui_doc_rect_t a, b, node_rect, sparse_size, full_size;
    xui_doc_position_t p, hit;
    int exact, i;
    CHECK(text != NULL);
    for (i = 0; i < lines; i++) {
        int added;
        if (i == 1500) middle = bytes + 10;
        if (i == lines - 1) last = bytes + 10;
        added = snprintf(text + bytes, (size_t)lines * 80 - bytes,
            "row-%04d: abcdefghijklmnopqrstuvwxyz0123456789\n", i);
        CHECK(added > 0 && (size_t)added < (size_t)lines * 80 - bytes);
        bytes += (size_t)added;
        if (!first_boundary && bytes >= 8192) first_boundary = bytes;
    }
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    code = insert(txn, 1, XUI_DOC_CODE_BLOCK, text, bytes);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn); txn = NULL;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    renderer_desc.iSize = sizeof(renderer_desc); renderer_desc.fLineGap = 2;
    CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &sparse) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &full) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(sparse, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(full, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(sparse, 640, 0, 60) == XUI_OK);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(sparse, &stats) == XUI_OK);
    CHECK(stats.iBlocks > 10 && stats.iMeasuredBlocks < 4 && stats.iShapedBytes < bytes / 4);
    CHECK(xuiDocumentRendererGetSize(sparse, &sparse_size, &exact) == XUI_OK && !exact);
    p = position(document, code, bytes);
    CHECK(xuiDocumentRendererGetCaretRect(sparse, &p, &a) == XUI_OK &&
        fabs(a.x - 8) < .01 && fabs(a.y - lines * 16) < .01 && fabs(a.height - 14) < .01);
    CHECK(xuiDocumentRendererGetStats(sparse, &stats) == XUI_OK &&
        stats.iMeasuredBlocks < 4 && stats.iShapedBytes < bytes / 3);
    CHECK(xuiDocumentRendererHitTest(sparse, 13, a.y + 7, &hit) == XUI_OK &&
        hit.iNodeId == code && hit.iOffset == bytes);
    CHECK(xuiDocumentRendererGetNodeRect(sparse, code, &node_rect) == XUI_OK && node_rect.height > 20000);
    CHECK(xuiDocumentRendererLayout(full, 640, 0, 1000000) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(full, &full_size, &exact) == XUI_OK && exact);
    CHECK(fabs(full_size.height - ((lines + 1) * 14 + lines * 2 + 8)) < .01);
    CHECK(xuiDocumentRendererGetCaretRect(full, &p, &b) == XUI_OK &&
        fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01 && fabs(a.height - b.height) < .01);
    CHECK(fabs(sparse_size.height - full_size.height) < .01);
    {
        xui_doc_renderer_stats_t before = {0}, after = {0};
        before.iSize = after.iSize = sizeof(before);
        CHECK(xuiDocumentRendererGetStats(full, &before) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(full, 520, 0, 1000000) == XUI_OK);
        CHECK(xuiDocumentRendererGetStats(full, &after) == XUI_OK &&
            after.iShapedBytes == before.iShapedBytes);
        CHECK(xuiDocumentRendererLayout(full, 640, 0, 1000000) == XUI_OK);
        CHECK(xuiDocumentRendererGetStats(full, &after) == XUI_OK &&
            after.iShapedBytes == before.iShapedBytes);
    }
    {
        xui_doc_position_t before = position(document, code, first_boundary);
        xui_doc_position_t after = before;
        xui_doc_rect_t before_rect, after_rect;
        xui_doc_range_t selection;
        xui_draw_context draw = NULL;
        uint32_t selection_color = XUI_COLOR_RGBA(245, 20, 30, 255);
        before.iAffinity = XUI_DOC_BEFORE;
        after.iAffinity = XUI_DOC_AFTER;
        CHECK(xuiDocumentRendererGetCaretRect(sparse, &before, &before_rect) == XUI_OK);
        CHECK(xuiDocumentRendererGetCaretRect(sparse, &after, &after_rect) == XUI_OK);
        CHECK(after_rect.y > before_rect.y);
        selection.tAnchor = position(document, code, first_boundary - 2);
        selection.tCaret = position(document, code, first_boundary + 2);
        xuiTestSurfaceReset(target);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
        CHECK(xuiDocumentRendererDraw(sparse, draw, 0, 20 - before_rect.y,
            (xui_rect_t){0, 0, 640, 80}, &selection, selection_color) == XUI_OK);
        CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
        CHECK(xuiTestSurfaceGetRectFillColorCount(target, selection_color) > 0);
        CHECK(xuiTestSurfaceGetTextDrawCount(target) > 0);
    }
    p = position(document, code, middle);
    CHECK(xuiDocumentRendererGetCaretRect(sparse, &p, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(full, &p, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    CHECK(xuiDocumentRendererHitTest(sparse, a.x + .1, a.y + a.height / 2, &hit) == XUI_OK &&
        hit.iNodeId == code && hit.iOffset == middle);
    {
        xui_draw_context draw = NULL;
        xuiTestSurfaceReset(target);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
        CHECK(xuiDocumentRendererDraw(sparse, draw, 0, 20 - a.y,
            (xui_rect_t){0, 0, 640, 80}, NULL, 0) == XUI_OK);
        CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
        CHECK(xuiTestSurfaceGetTextDrawCount(target) > 0);
    }
    p = position(document, code, last);
    CHECK(xuiDocumentRendererGetCaretRect(sparse, &p, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(full, &p, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(txn, code, 0, 0, "X\n", 2) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, &changes) == XUI_OK); xuiDocumentTxnRelease(txn); txn = NULL;
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(sparse, snapshot, changes) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(full, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(full, 640, 0, 1000000) == XUI_OK);
    p = position(document, code, middle + 2);
    CHECK(xuiDocumentRendererGetCaretRect(sparse, &p, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(full, &p, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    xuiDocumentRendererRelease(sparse); xuiDocumentRendererRelease(full);
    xuiDocumentChangeSetRelease(changes); xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    {
        char* markdown = malloc(bytes + 10);
        xui_doc_desc_t document_desc = {0};
        xui_doc_node_info_t info = {0};
        xui_doc_node_id markdown_code = 0;
        CHECK(markdown != NULL);
        memcpy(markdown, "```c\n", 5);
        memcpy(markdown + 5, text, bytes);
        memcpy(markdown + 5 + bytes, "```\n", 4);
        markdown[bytes + 9] = 0;
        document_desc.iSize = sizeof(document_desc);
        document_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&document_desc, &document) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(document, markdown, bytes + 9) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &markdown_code) == XUI_OK);
        info.iSize = sizeof(info);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, markdown_code, &info) == XUI_OK &&
            info.iKind == XUI_DOC_CODE_BLOCK && info.iTextBytes == bytes);
        CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &sparse) == XUI_OK);
        CHECK(xuiDocumentRendererSetSnapshot(sparse, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(sparse, 640, 0, 60) == XUI_OK);
        stats.iSize = sizeof(stats);
        CHECK(xuiDocumentRendererGetStats(sparse, &stats) == XUI_OK &&
            stats.iBlocks > 10 && stats.iShapedBytes < bytes / 4);
        p = position(document, markdown_code, middle);
        CHECK(xuiDocumentRendererGetCaretRect(sparse, &p, &a) == XUI_OK);
        CHECK(xuiDocumentRendererHitTest(sparse, a.x + .1, a.y + a.height / 2, &hit) == XUI_OK &&
            hit.iNodeId == markdown_code && hit.iOffset == middle);
        xuiDocumentRendererRelease(sparse); xuiDocumentSnapshotRelease(snapshot);
        xuiDocumentRelease(document); free(markdown);
    }
    free(text);
    printf("Long code block: bytes=%llu initial_shaped=%llu slices=%llu, deep caret and edit rebuild passed\n",
        (unsigned long long)bytes, (unsigned long long)stats.iShapedBytes,
        (unsigned long long)stats.iBlocks);
}
static void unbroken_code_line(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    static const char pattern[] = "longIdentifier12345 = alpha + beta; ";
    const size_t bytes = 128 * 1024;
    char* text = malloc(bytes + 1);
    xui_document document = NULL;
    xui_document_transaction txn = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_document_change_set changes = NULL;
    xui_document_renderer sparse = NULL, full = NULL, cold = NULL;
    xui_widget view = NULL;
    xui_doc_view_desc_t view_desc = {0};
    xui_doc_renderer_stats_t stats = {0};
    xui_doc_rect_t a, b, size;
    xui_doc_range_t all = {0};
    xui_doc_position_t p, hit;
    uint64_t code, count;
    size_t i;
    int exact;
    double scroll_x, scroll_y;
    CHECK(text != NULL);
    for (i = 0; i < bytes; i++) text[i] = pattern[i % (sizeof(pattern) - 1)];
    text[bytes] = 0;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    code = insert(txn, 1, XUI_DOC_CODE_BLOCK, text, bytes);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &sparse) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &full) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(sparse, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(full, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(sparse, 640, 0, 60) == XUI_OK);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(sparse, &stats) == XUI_OK &&
        stats.iShapedBytes == 8192 && stats.iBlocks == 1);
    CHECK(xuiDocumentRendererGetSize(sparse, &size, &exact) == XUI_OK &&
        !exact && size.width > 640);
    all.tAnchor = position(document, code, 0);
    all.tCaret = position(document, code, bytes);
    CHECK(xuiDocumentRendererGetRangeRects(full, &all, NULL, 0, &count) == XUI_OK &&
        count == 1);
    CHECK(xuiDocumentRendererGetSize(full, &size, &exact) == XUI_OK && exact);
    {
        xui_doc_range_t short_range = {0};
        xui_doc_rect_t sparse_rect[2], full_rect[2];
        uint64_t sparse_count = 0, full_count = 0;
        short_range.tAnchor = position(document, code, 1000);
        short_range.tCaret = position(document, code, 16384);
        CHECK(xuiDocumentRendererGetRangeRects(sparse, &short_range,
            sparse_rect, 2, &sparse_count) == XUI_OK);
        CHECK(xuiDocumentRendererGetRangeRects(full, &short_range,
            full_rect, 2, &full_count) == XUI_OK);
        CHECK(sparse_count == 1 && full_count == sparse_count &&
            fabs(sparse_rect[0].x - full_rect[0].x) < .01 &&
            fabs(sparse_rect[0].y - full_rect[0].y) < .01 &&
            fabs(sparse_rect[0].width - full_rect[0].width) < .01 &&
            fabs(sparse_rect[0].height - full_rect[0].height) < .01);
        CHECK(xuiDocumentRendererGetStats(sparse, &stats) == XUI_OK &&
            stats.iShapedBytes < bytes / 4);
        CHECK(xuiDocumentRendererGetSize(sparse, &size, &exact) == XUI_OK &&
            !exact);
        short_range.tAnchor = position(document, code, 16384);
        short_range.tCaret = position(document, code, 1000);
        CHECK(xuiDocumentRendererGetRangeRects(sparse, &short_range,
            sparse_rect, 2, &sparse_count) == XUI_OK &&
            sparse_count == full_count &&
            fabs(sparse_rect[0].x - full_rect[0].x) < .01 &&
            fabs(sparse_rect[0].width - full_rect[0].width) < .01);
        CHECK(xuiDocumentRendererGetStats(sparse, &stats) == XUI_OK &&
            stats.iShapedBytes < bytes / 4);
        CHECK(xuiDocumentRendererCreate(context, NULL, &cold) == XUI_OK);
        CHECK(xuiDocumentRendererSetSnapshot(cold, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(cold, 640, 0, 0) == XUI_OK);
        CHECK(xuiDocumentRendererGetRangeRects(cold, &short_range,
            sparse_rect, 2, &sparse_count) == XUI_OK &&
            sparse_count == full_count &&
            fabs(sparse_rect[0].x - full_rect[0].x) < .01 &&
            fabs(sparse_rect[0].width - full_rect[0].width) < .01);
        CHECK(xuiDocumentRendererGetStats(cold, &stats) == XUI_OK &&
            stats.iShapedBytes < bytes / 4);
        CHECK(xuiDocumentRendererGetSize(cold, &size, &exact) == XUI_OK &&
            !exact);
    }
    p = position(document, code, 32768);
    CHECK(xuiDocumentRendererGetCaretRect(sparse, &p, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(full, &p, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    CHECK(xuiDocumentRendererGetStats(sparse, &stats) == XUI_OK &&
        stats.iShapedBytes < bytes / 2);
    CHECK(xuiDocumentRendererHitTest(sparse, a.x + .1,
        a.y + a.height / 2, &hit) == XUI_OK &&
        hit.iNodeId == code && hit.iOffset == p.iOffset);
    {
        xui_doc_renderer_stats_t after = {0};
        after.iSize = sizeof(after);
        CHECK(xuiDocumentRendererGetStats(sparse, &after) == XUI_OK &&
            after.iHitFragmentsExamined - stats.iHitFragmentsExamined < 10);
    }
    {
        xui_draw_context draw = NULL;
        xuiTestSurfaceReset(target);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
        CHECK(xuiDocumentRendererDraw(sparse, draw, 100 - a.x, 0,
            (xui_rect_t){0, 0, 640, 60}, NULL, 0) == XUI_OK);
        CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
        CHECK(xuiTestSurfaceGetTextDrawCount(target) > 0);
    }
    view_desc.iSize = sizeof(view_desc);
    view_desc.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 640, 240}) == XUI_OK);
    frame(context, target);
    CHECK(xuiDocumentViewSetScroll(view, a.x - 100, 0) == XUI_OK);
    CHECK(xuiDocumentViewGetScroll(view, &scroll_x, &scroll_y) == XUI_OK &&
        fabs(scroll_x - (a.x - 100)) < 1 && scroll_y == 0);
    frame(context, target);
    CHECK(xuiDocumentViewHitTest(view, 100, (float)(a.y + a.height / 2),
        &hit) == XUI_OK && hit.iNodeId == code &&
        hit.iOffset == p.iOffset);
    CHECK(xuiSetRootWidget(context, NULL) == XUI_OK);
    xuiWidgetDestroy(view);
    {
        xui_doc_renderer_stats_t before = {0}, after = {0};
        before.iSize = after.iSize = sizeof(before);
        CHECK(xuiDocumentRendererGetStats(sparse, &before) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(sparse, 520, 0, 60) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(full, 520, 0, 60) == XUI_OK);
        CHECK(xuiDocumentRendererGetStats(sparse, &after) == XUI_OK &&
            after.iShapedBytes - before.iShapedBytes == 8192);
        CHECK(xuiDocumentRendererGetSize(sparse, &size, &exact) == XUI_OK &&
            !exact);
        p = position(document, code, 32768);
        CHECK(xuiDocumentRendererGetCaretRect(sparse, &p, &a) == XUI_OK);
        CHECK(xuiDocumentRendererGetCaretRect(full, &p, &b) == XUI_OK);
        CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    }
    p = position(document, code, bytes - 1);
    CHECK(xuiDocumentRendererGetCaretRect(sparse, &p, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(full, &p, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    CHECK(xuiDocumentRendererGetSize(sparse, &size, &exact) == XUI_OK && exact);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(txn, code, 0, 0, "ABC", 3) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, &changes) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(sparse, snapshot, changes) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(full, snapshot, NULL) == XUI_OK);
    {
        xui_doc_renderer_stats_t before = {0}, after = {0};
        before.iSize = after.iSize = sizeof(before);
        CHECK(xuiDocumentRendererGetStats(sparse, &before) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(sparse, 520, 0, 60) == XUI_OK);
        CHECK(xuiDocumentRendererGetStats(sparse, &after) == XUI_OK &&
            after.iShapedBytes - before.iShapedBytes == 8192);
        all.tAnchor = position(document, code, 0);
        all.tCaret = position(document, code, bytes + 3);
        CHECK(xuiDocumentRendererGetRangeRects(full, &all, NULL, 0,
            &count) == XUI_OK && count == 1);
        p = position(document, code, 32771);
        CHECK(xuiDocumentRendererGetCaretRect(sparse, &p, &a) == XUI_OK);
        CHECK(xuiDocumentRendererGetCaretRect(full, &p, &b) == XUI_OK);
        CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    }
    xuiDocumentRendererRelease(sparse);
    xuiDocumentRendererRelease(full);
    xuiDocumentRendererRelease(cold);
    xuiDocumentChangeSetRelease(changes);
    xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    free(text);
    puts("Unbroken ASCII code line horizontal prefix, scroll, hit and caret passed");
}
static xui_text_shape_proc protected_shape;
static unsigned protected_shape_calls, protected_rtl_calls;
static int protected_whole_shape(xui_proxy proxy, const xui_text_item_t* pTextItem, xui_text_shape_t* shape)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;
    int bytes = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iTextSize : 0;
    uint32_t flags = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->iFlags : 0;

    CHECK(bytes == 16 * 1024 && (unsigned char)text[0] == 0xd8);
    protected_shape_calls++;
    if (flags & XUI_TEXT_SHAPE_RTL) protected_rtl_calls++;
    return protected_shape(proxy, pTextItem, shape);
}
static void protected_code_line(xui_context original_context)
{
    const size_t bytes = 16 * 1024;
    char* text = malloc(bytes + 1);
    xui_document document = NULL;
    xui_document_transaction txn = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_document_renderer renderer = NULL;
    xui_doc_renderer_stats_t stats = {0};
    xui_test_proxy_state_t proxy;
    xui_context context;
    size_t i;
    xuiTestProxyInit(&proxy);protected_shape=proxy.tProxy.textShape;
    proxy.tProxy.textShape=protected_whole_shape;
    protected_shape_calls=protected_rtl_calls=0;
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy.tProxy)==XUI_OK &&
        xuiSetDefaultFont(context,xuiGetDefaultFont(original_context))==XUI_OK);
    CHECK(text != NULL);
    for (i = 0; i < bytes; i += 2) {
        text[i] = (char)0xd8;
        text[i + 1] = (char)0xa8;
    }
    text[bytes] = 0;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    insert(txn, 1, XUI_DOC_CODE_BLOCK, text, bytes);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(renderer, 640, 0, 60) == XUI_OK);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
        stats.iTextRunBytes == bytes && stats.iShapedBytes <= bytes * 3 &&
        protected_shape_calls > 0 && protected_shape_calls <= 3 && protected_rtl_calls > 0);
    xuiDocumentRendererRelease(renderer);
    xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    free(text);
    xuiDestroy(context);
    printf("Protected non-ASCII code shaping remains whole-line: %u passes, %u RTL, no artificial chunks\n",
        protected_shape_calls,protected_rtl_calls);
}
static void unbroken_source_line(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    const size_t bytes = 128 * 1024;
    char* source = malloc(bytes + 1);
    xui_doc_desc_t document_desc = {0};
    xui_document document = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_document_renderer sparse = NULL, full = NULL, live = NULL;
    xui_widget view = NULL;
    xui_doc_view_desc_t view_desc = {0};
    xui_doc_renderer_stats_t stats = {0};
    xui_doc_rect_t a, b, size;
    xui_doc_range_t range = {0};
    xui_doc_position_t p, hit;
    uint64_t count = 0;
    int exact;
    double scroll_x, scroll_y;
    CHECK(source != NULL);
    memset(source, 'A', bytes); source[bytes] = 0;
    document_desc.iSize = sizeof(document_desc);
    document_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&document_desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, source, bytes) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &sparse) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &full) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(sparse, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(full, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(sparse, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(full, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(sparse, 640, 0, 60) == XUI_OK);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(sparse, &stats) == XUI_OK &&
        stats.iShapedBytes == 8192 && stats.iSourceBlocks == 1);
    CHECK(xuiDocumentRendererGetSize(sparse, &size, &exact) == XUI_OK &&
        !exact && size.width > 640);
    range.tAnchor = source_position(document, 0);
    range.tCaret = source_position(document, bytes);
    CHECK(xuiDocumentRendererGetRangeRects(full, &range, NULL, 0,
        &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentRendererGetSize(full, &size, &exact) == XUI_OK && exact);
    range.tAnchor = source_position(document, 1000);
    range.tCaret = source_position(document, 16000);
    {
        xui_doc_rect_t lazy[2], complete[2];
        uint64_t lazy_count = 0, complete_count = 0;
        CHECK(xuiDocumentRendererGetRangeRects(sparse, &range,
            lazy, 2, &lazy_count) == XUI_OK);
        CHECK(xuiDocumentRendererGetRangeRects(full, &range,
            complete, 2, &complete_count) == XUI_OK);
        CHECK(lazy_count == 1 && complete_count == lazy_count &&
            fabs(lazy[0].x - complete[0].x) < .01 &&
            fabs(lazy[0].width - complete[0].width) < .01);
        CHECK(xuiDocumentRendererGetStats(sparse, &stats) == XUI_OK &&
            stats.iShapedBytes < bytes / 4);
        CHECK(xuiDocumentRendererGetSize(sparse, &size, &exact) == XUI_OK &&
            !exact);
    }
    p = source_position(document, bytes / 2);
    CHECK(xuiDocumentRendererGetCaretRect(sparse, &p, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(full, &p, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    CHECK(xuiDocumentRendererHitTest(sparse, a.x + .1,
        a.y + a.height / 2, &hit) == XUI_OK &&
        hit.iKind == XUI_DOC_POSITION_SOURCE && hit.iOffset == p.iOffset);
    {
        uint64_t start = 0, end = 0;
        xui_doc_renderer_stats_t live_stats = {0};
        xui_doc_rect_t live_caret;
        CHECK(xuiDocumentRendererCreate(context, NULL, &live) == XUI_OK);
        CHECK(xuiDocumentRendererSetMode(live, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
        CHECK(xuiDocumentRendererSetSnapshot(live, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererSetActivePosition(live, &p) == XUI_OK);
        CHECK(xuiDocumentRendererGetActiveSourceRange(live, &start,
            &end) == XUI_OK && start == 0 && end == bytes);
        CHECK(xuiDocumentRendererLayout(live, 640, 0, 60) == XUI_OK);
        live_stats.iSize = sizeof(live_stats);
        CHECK(xuiDocumentRendererGetStats(live, &live_stats) == XUI_OK &&
            live_stats.iSourceBlocks == 1 && !live_stats.bLiveSourceFallback &&
            live_stats.iShapedBytes < bytes / 4);
        CHECK(xuiDocumentRendererGetCaretRect(live, &p,
            &live_caret) == XUI_OK);
        CHECK(xuiDocumentRendererHitTest(live, live_caret.x + .1,
            live_caret.y + live_caret.height / 2, &hit) == XUI_OK &&
            hit.iKind == XUI_DOC_POSITION_SOURCE &&
            hit.iOffset == p.iOffset);
    }
    view_desc.iSize = sizeof(view_desc);
    view_desc.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 640, 240}) == XUI_OK);
    frame(context, target);
    CHECK(xuiDocumentViewSetScroll(view, a.x - 100, 0) == XUI_OK);
    CHECK(xuiDocumentViewGetScroll(view, &scroll_x, &scroll_y) == XUI_OK &&
        fabs(scroll_x - (a.x - 100)) < 1 && scroll_y == 0);
    frame(context, target);
    CHECK(xuiDocumentViewHitTest(view, 100,
        (float)(a.y + a.height / 2), &hit) == XUI_OK &&
        hit.iKind == XUI_DOC_POSITION_SOURCE && hit.iOffset == p.iOffset);
    CHECK(xuiSetRootWidget(context, NULL) == XUI_OK);
    xuiWidgetDestroy(view);
    {
        xui_draw_context draw = NULL;
        xuiTestSurfaceReset(target);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
        CHECK(xuiDocumentRendererDraw(sparse, draw, 100 - a.x, 0,
            (xui_rect_t){0, 0, 640, 60}, NULL, 0) == XUI_OK);
        CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
        CHECK(xuiTestSurfaceGetTextDrawCount(target) > 0);
    }
    xuiDocumentRendererRelease(sparse);
    xuiDocumentRendererRelease(full);
    xuiDocumentRendererRelease(live);
    xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    free(source);
    puts("Unbroken SOURCE/LIVE line horizontal prefix, range, caret and hit passed");
}
static void shifted_source_line_prefix(xui_context context)
{
    static const char head[] = "one\ntwo\nthree\n";
    const size_t bytes = 48 * 1024, start = sizeof(head) - 1;
    char* source = malloc(start + bytes + 1);
    xui_doc_desc_t desc = {0};
    xui_doc_source_patch_t patch = {0};
    xui_document document = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_document_prepare prepare = NULL;
    xui_document_renderer reused = NULL, fresh = NULL;
    xui_doc_position_t p, hit;
    xui_doc_rect_t a, b, size;
    xui_doc_renderer_stats_t before = {0}, after = {0};
    int exact;
    CHECK(source != NULL);
    memcpy(source, head, start);
    memset(source + start, 'A', bytes);
    source[start + bytes] = 0;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, source,
        start + bytes) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &reused) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &fresh) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(reused, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(fresh, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(reused, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(fresh, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(reused, 640, 0, 120) == XUI_OK);
    p = source_position(document, start + 16000);
    CHECK(xuiDocumentRendererGetCaretRect(reused, &p, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(reused, &size, &exact) == XUI_OK &&
        !exact);
    patch.iSize = sizeof(patch); patch.iStart = patch.iEnd = 0;
    patch.sText = "X"; patch.iTextBytes = 1;
    CHECK(xuiDocumentPrepareSource(document, NULL, &patch, 1,
        &prepare) == XUI_OK);
    before.iSize = after.iSize = sizeof(before);
    CHECK(xuiDocumentRendererGetStats(reused, &before) == XUI_OK);
    CHECK(xuiDocumentRendererSetSourceInput(reused, prepare) == XUI_OK);
    CHECK(xuiDocumentRendererSetSourceInput(fresh, prepare) == XUI_OK);
    CHECK(xuiDocumentPrepareSourcePosition(prepare, start + 16001,
        XUI_DOC_AFTER, &p) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(reused, &p, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(fresh, &p, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    CHECK(xuiDocumentRendererLayout(reused, 640, 0, 120) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(fresh, 640, 0, 120) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(reused, &p, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(fresh, &p, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    CHECK(xuiDocumentRendererGetStats(reused, &after) == XUI_OK &&
        after.iSourceReusedBlocks >= 1 &&
        after.iShapedBytes - before.iShapedBytes < 64);
    CHECK(xuiDocumentRendererHitTest(reused, a.x + .1,
        a.y + a.height / 2, &hit) == XUI_OK &&
        hit.iKind == XUI_DOC_POSITION_SOURCE &&
        hit.iOffset == p.iOffset);
    CHECK(xuiDocumentRendererGetSize(reused, &size, &exact) == XUI_OK &&
        !exact);
    xuiDocumentRendererRelease(reused);
    xuiDocumentRendererRelease(fresh);
    CHECK(xuiDocumentCancelPrepare(document) == XUI_OK);
    xuiDocumentPrepareRelease(prepare);
    xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    free(source);
    puts("SOURCE partial long-line cache survives preceding source splice");
}
static void protected_source_line(xui_context original_context)
{
    const size_t bytes = 16 * 1024;
    char* source = malloc(bytes + 1);
    xui_doc_desc_t desc = {0};
    xui_document document = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_document_renderer renderer = NULL;
    xui_doc_renderer_stats_t stats = {0};
    xui_test_proxy_state_t proxy;
    xui_context context;
    size_t i;
    xuiTestProxyInit(&proxy);protected_shape=proxy.tProxy.textShape;
    proxy.tProxy.textShape=protected_whole_shape;
    protected_shape_calls=protected_rtl_calls=0;
    CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy.tProxy)==XUI_OK &&
        xuiSetDefaultFont(context,xuiGetDefaultFont(original_context))==XUI_OK);
    CHECK(source != NULL);
    for (i = 0; i < bytes; i += 2) {
        source[i] = (char)0xd8;
        source[i + 1] = (char)0xa8;
    }
    source[bytes] = 0;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, source, bytes) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(renderer, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(renderer, 640, 0, 60) == XUI_OK);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(renderer, &stats) == XUI_OK &&
        stats.iTextRunBytes == bytes && stats.iShapedBytes <= bytes * 3 &&
        protected_shape_calls > 0 && protected_shape_calls <= 3 && protected_rtl_calls > 0);
    xuiDocumentRendererRelease(renderer);
    xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    free(source);
    xuiDestroy(context);
    printf("Protected non-ASCII SOURCE shaping remains whole-line: %u passes, %u RTL, no artificial chunks\n",
        protected_shape_calls,protected_rtl_calls);
}
static void source_callback_cold_height(xui_context context,
    xui_test_proxy_state_t* proxy)
{
    static const char head[] = "one\ntwo\nthree\n";
    const size_t bytes = 48 * 1024, prefix = sizeof(head) - 1;
    char* source = malloc(prefix + bytes + 1);
    scale_font_switch switcher = {0};
    xui_doc_desc_t document_desc = {0};
    xui_doc_renderer_desc_t renderer_desc = {0};
    xui_doc_source_patch_t patch = {0};
    xui_document document = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_document_prepare prepare = NULL;
    xui_document_renderer cold = NULL, warm = NULL, fresh = NULL, live = NULL;
    xui_doc_position_t p, original;
    xui_doc_rect_t a, b;
    xui_doc_renderer_stats_t stats = {0};
    uint64_t live_start = 0, live_end = 0;
    xui_font alternate = NULL;
    CHECK(source != NULL);
    memcpy(source, head, prefix);
    memset(source + prefix, 'A', bytes);
    source[prefix + bytes] = 0;
    CHECK(proxy->tProxy.fontLoadMemory(&proxy->tProxy, &alternate,
        NULL, 0, 24, 0) == XUI_OK);
    switcher.current = alternate;
    document_desc.iSize = sizeof(document_desc);
    document_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    renderer_desc.iSize = sizeof(renderer_desc);
    renderer_desc.onFont = scale_select_font;
    renderer_desc.pUser = &switcher;
    CHECK(xuiDocumentCreate(&document_desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, source,
        prefix + bytes) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &cold) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &warm) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(cold, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(warm, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(cold, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(warm, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(cold, 640, 0, 0) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(warm, 640, 0, 120) == XUI_OK);
    p = source_position(document, prefix + 16000);
    CHECK(xuiDocumentRendererGetCaretRect(cold, &p, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(warm, &p, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(cold, &stats) == XUI_OK &&
        stats.iShapedBytes < bytes / 2);
    patch.iSize = sizeof(patch); patch.iStart = patch.iEnd = 0;
    patch.sText = "X"; patch.iTextBytes = 1;
    CHECK(xuiDocumentPrepareSource(document, NULL, &patch, 1,
        &prepare) == XUI_OK);
    CHECK(xuiDocumentRendererSetSourceInput(cold, prepare) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &fresh) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(fresh, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(fresh, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSourceInput(fresh, prepare) == XUI_OK);
    CHECK(xuiDocumentPrepareSourcePosition(prepare, prefix + 16001,
        XUI_DOC_AFTER, &p) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(cold, &p, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(fresh, &p, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    CHECK(xuiDocumentRendererGetStats(cold, &stats) == XUI_OK &&
        stats.iSourceIncrementalUpdates > 0);
    original = source_position(document, prefix + 16000);
    CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &live) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(live, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(live, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetActivePosition(live, &original) == XUI_OK);
    CHECK(xuiDocumentRendererGetActiveSourceRange(live, &live_start,
        &live_end) == XUI_OK && live_start == 0 && live_end == prefix + bytes);
    CHECK(xuiDocumentRendererSetSourceInput(live, prepare) == XUI_OK);
    CHECK(xuiDocumentRendererGetStats(live, &stats) == XUI_OK &&
        stats.iSourceIncrementalUpdates > 0 && !stats.bLiveSourceFallback);
    CHECK(xuiDocumentRendererGetCaretRect(live, &p, &a) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    xuiDocumentRendererRelease(fresh);
    fresh = NULL;
    switcher.current = xuiGetDefaultFont(context);
    CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &fresh) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(fresh, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(fresh, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSourceInput(fresh, prepare) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(cold, &p, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(fresh, &p, &b) == XUI_OK);
    CHECK(fabs(a.y - b.y) > 10);
    CHECK(xuiDocumentRendererInvalidateFonts(cold) == XUI_OK);
    CHECK(xuiDocumentRendererInvalidateFonts(live) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(cold, &p, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(fresh, &p, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    CHECK(xuiDocumentRendererGetCaretRect(live, &p, &a) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    CHECK(xuiDocumentRendererLayout(cold, 600, 0, 0) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(cold, &p, &a) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    xuiDocumentRendererRelease(live);
    xuiDocumentRendererRelease(fresh);
    xuiDocumentRendererRelease(cold);
    xuiDocumentRendererRelease(warm);
    CHECK(xuiDocumentCancelPrepare(document) == XUI_OK);
    xuiDocumentPrepareRelease(prepare);
    xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    proxy->tProxy.fontDestroy(&proxy->tProxy, alternate);
    free(source);
    puts("SOURCE/LIVE cold, incremental, invalidated and resized caret use callback-selected font line height");
}
static void view_callback_font_invalidation(xui_context context,
    xui_surface target, xui_test_proxy_state_t* proxy)
{
    char source[1201];
    scale_font_switch switcher = {0};
    xui_doc_desc_t document_desc = {0};
    xui_doc_view_desc_t view_desc = {0};
    xui_document document = NULL;
    xui_widget view = NULL;
    xui_font alternate = NULL;
    xui_doc_rect_t before_size, after_size;
    xui_doc_position_t before, after;
    int exact, mode, i;
    for (i = 0; i < 300; i++) memcpy(source + i * 4, "row\n", 4);
    source[1200] = 0;
    CHECK(proxy->tProxy.fontLoadMemory(&proxy->tProxy, &alternate,
        NULL, 0, 24, 0) == XUI_OK);
    switcher.current = xuiGetDefaultFont(context);
    document_desc.iSize = sizeof(document_desc);
    document_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&document_desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, source, 1200) == XUI_OK);
    view_desc.iSize = sizeof(view_desc);
    view_desc.pDocument = document;
    view_desc.tRenderer.iSize = sizeof(view_desc.tRenderer);
    view_desc.tRenderer.onFont = scale_select_font;
    view_desc.tRenderer.pUser = &switcher;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 640, 180}) == XUI_OK);
    for (mode = XUI_DOC_SOURCE_TEXT; mode <= XUI_DOC_LIVE_MARKDOWN; mode++) {
        switcher.current = xuiGetDefaultFont(context);
        CHECK(xuiDocumentViewSetMode(view, mode) == XUI_OK);
        CHECK(xuiDocumentViewInvalidateFonts(view) == XUI_OK);
        frame(context, target);
        CHECK(xuiDocumentViewSetScroll(view, 0, 800) == XUI_OK);
        frame(context, target);
        CHECK(xuiDocumentViewHitTest(view, 8, 20, &before) == XUI_OK);
        CHECK(xuiDocumentViewGetContentSize(view, &before_size, &exact) == XUI_OK);
        switcher.current = alternate;
        CHECK(xuiDocumentViewInvalidateFonts(view) == XUI_OK);
        frame(context, target);
        CHECK(xuiDocumentViewHitTest(view, 8, 20, &after) == XUI_OK);
        CHECK(xuiDocumentViewGetContentSize(view, &after_size, &exact) == XUI_OK);
        CHECK(before.iKind == XUI_DOC_POSITION_SOURCE &&
            after.iKind == XUI_DOC_POSITION_SOURCE &&
            before.iOffset / 4 == after.iOffset / 4);
        CHECK(after_size.height > before_size.height * 1.2);
    }
    CHECK(xuiSetRootWidget(context, NULL) == XUI_OK);
    xuiWidgetDestroy(view);
    xuiDocumentRelease(document);
    proxy->tProxy.fontDestroy(&proxy->tProxy, alternate);
    puts("DocumentView SOURCE/LIVE external font invalidation preserves visible source row");
}
static void source_ascii_content_height(void)
{
    enum { PREFIX_ROWS = 12, LONG_BYTES = 9000, SUFFIX_ROWS = 40 };
    char* source = malloc(PREFIX_ROWS * 3 + LONG_BYTES + 1 + SUFFIX_ROWS * 3 + 1);
    char* plain = NULL;
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_font font = NULL;
    xui_doc_desc_t desc = {0};
    xui_doc_renderer_desc_t renderer_desc = {0};
    xui_document document = NULL;
    xui_document plain_document = NULL;
    xui_document_snapshot snapshot = NULL, plain_snapshot = NULL;
    xui_document_renderer cold = NULL, warm = NULL, plain_renderer = NULL;
    xui_doc_position_t p, plain_position, hit;
    xui_doc_rect_t a, b, normal;
    size_t bytes = 0, i;
    int mode;
    CHECK(source != NULL);
    for (i = 0; i < PREFIX_ROWS; i++) {
        memcpy(source + bytes, "ok\n", 3);
        bytes += 3;
    }
    memset(source + bytes, 'a', LONG_BYTES);
    source[bytes + LONG_BYTES - 1] = '~';
    bytes += LONG_BYTES;
    source[bytes++] = '\n';
    for (i = 0; i < SUFFIX_ROWS; i++) {
        memcpy(source + bytes, "ok\n", 3);
        bytes += 3;
    }
    source[bytes] = 0;
    source[5 * 3 + 1] = '~';
    plain = malloc(bytes + 1);
    CHECK(plain != NULL);
    memcpy(plain, source, bytes + 1);
    plain[PREFIX_ROWS * 3 + LONG_BYTES - 1] = 'a';
    xuiTestProxyInit(&proxy);
    scale_base_shape = proxy.tProxy.textShape;
    scale_ascii_tall_char = '~';
    proxy.tProxy.textShape = scale_late_tall_shape;
    CHECK(xuiCreate(&context) == XUI_OK &&
        xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font,
        "scale.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, source, bytes) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentCreate(&desc, &plain_document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(plain_document, plain, bytes) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(plain_document, &plain_snapshot) == XUI_OK);
    renderer_desc.iSize = sizeof(renderer_desc);
    renderer_desc.bSourceLineHeightMayVary = 1;
    p = source_position(document, PREFIX_ROWS * 3 + LONG_BYTES + 1 + 25 * 3);
    plain_position = source_position(plain_document, p.iOffset);
    for (mode = XUI_DOC_SOURCE_TEXT; mode <= XUI_DOC_LIVE_MARKDOWN; mode++) {
        CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &cold) == XUI_OK);
        CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &warm) == XUI_OK);
        CHECK(xuiDocumentRendererCreate(context, &renderer_desc, &plain_renderer) == XUI_OK);
        CHECK(xuiDocumentRendererSetMode(cold, mode) == XUI_OK);
        CHECK(xuiDocumentRendererSetMode(warm, mode) == XUI_OK);
        CHECK(xuiDocumentRendererSetMode(plain_renderer, mode) == XUI_OK);
        CHECK(xuiDocumentRendererSetSnapshot(cold, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererSetSnapshot(warm, snapshot, NULL) == XUI_OK);
        CHECK(xuiDocumentRendererSetSnapshot(plain_renderer, plain_snapshot, NULL) == XUI_OK);
        if (mode == XUI_DOC_LIVE_MARKDOWN) {
            CHECK(xuiDocumentRendererSetActivePosition(cold, &p) == XUI_OK);
            CHECK(xuiDocumentRendererSetActivePosition(warm, &p) == XUI_OK);
            CHECK(xuiDocumentRendererSetActivePosition(plain_renderer, &plain_position) == XUI_OK);
        }
        CHECK(xuiDocumentRendererLayout(cold, 640, 0, 0) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(warm, 640, 0, 10000) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(plain_renderer, 640, 0, 10000) == XUI_OK);
        CHECK(xuiDocumentRendererGetCaretRect(warm, &p, &b) == XUI_OK);
        CHECK(xuiDocumentRendererGetCaretRect(plain_renderer, &plain_position, &normal) == XUI_OK);
        CHECK(b.y - normal.y > 10);
        CHECK(xuiDocumentRendererGetCaretRect(cold, &p, &a) == XUI_OK);
        CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
        CHECK(xuiDocumentRendererHitTest(cold, b.x + .1,
            b.y + b.height / 2, &hit) == XUI_OK &&
            hit.iKind == XUI_DOC_POSITION_SOURCE && hit.iOffset == p.iOffset);
        CHECK(xuiDocumentRendererLayout(cold, 600, 0, 0) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(warm, 600, 0, 10000) == XUI_OK);
        CHECK(xuiDocumentRendererGetCaretRect(warm, &p, &b) == XUI_OK);
        CHECK(xuiDocumentRendererGetCaretRect(cold, &p, &a) == XUI_OK);
        CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
        xuiDocumentRendererRelease(cold); cold = NULL;
        xuiDocumentRendererRelease(warm); warm = NULL;
        xuiDocumentRendererRelease(plain_renderer); plain_renderer = NULL;
    }
    xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentSnapshotRelease(plain_snapshot);
    xuiDocumentRelease(document);
    xuiDocumentRelease(plain_document);
    xuiDestroy(context);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    scale_ascii_tall_char = 0;
    free(plain);
    free(source);
    puts("SOURCE/LIVE content-dependent ASCII row height resolves cold caret and hit, including long lines");
}
static void source_unicode_cold_height(void)
{
    static const char head[] = "\xe4\xb8\xad\n";
    enum { ROWS = 100 };
    char source[sizeof(head) + ROWS * 3];
    xui_test_proxy_state_t proxy;
    xui_context context = NULL;
    xui_font font = NULL;
    xui_doc_desc_t desc = {0};
    xui_doc_source_patch_t patch = {0};
    xui_document document = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_document_prepare prepare = NULL;
    xui_document_renderer cold = NULL, warm = NULL, cold_hit = NULL,
        cold_range = NULL, live_cold = NULL, live_warm = NULL,
        live_hit = NULL, live_fresh = NULL, source_fresh = NULL,
        live_resized = NULL, source_resized = NULL;
    xui_doc_position_t p, hit, candidate;
    xui_doc_range_t range = {0};
    xui_doc_rect_t a, b, actual[2], expected[2];
    xui_doc_renderer_stats_t stats = {0};
    uint64_t actual_count = 0, expected_count = 0;
    size_t i, bytes = sizeof(head) - 1;
    memcpy(source, head, bytes);
    for (i = 0; i < ROWS; i++) {
        memcpy(source + bytes, "ok\n", 3);
        bytes += 3;
    }
    source[bytes] = 0;
    xuiTestProxyInit(&proxy);
    scale_base_shape = proxy.tProxy.textShape;
    scale_fail_shape_after = 0;
    scale_late_tall_factor = 3;
    proxy.tProxy.textShape = scale_late_tall_shape;
    CHECK(xuiCreate(&context) == XUI_OK &&
        xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font,
        "scale.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, source, bytes) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &cold) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &warm) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &cold_hit) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &cold_range) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(cold, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(warm, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(cold_hit, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(cold_range, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(cold, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(warm, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(cold_hit, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(cold_range, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(cold, 640, 0, 0) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(warm, 640, 0, 120) == XUI_OK);
    p = source_position(document, sizeof(head) - 1 + 80 * 3 + 1);
    CHECK(xuiDocumentRendererGetCaretRect(warm, &p, &b) == XUI_OK);
    range.tAnchor = p; range.tCaret = p; range.tCaret.iOffset++;
    CHECK(xuiDocumentRendererGetRangeRects(warm, &range, expected, 2,
        &expected_count) == XUI_OK && expected_count == 1);
    CHECK(xuiDocumentRendererGetRangeRects(cold_range, &range, actual, 2,
        &actual_count) == XUI_OK && actual_count == expected_count &&
        fabs(actual[0].y - expected[0].y) < .01);
    CHECK(xuiDocumentRendererHitTest(cold_hit, b.x + .1,
        b.y + b.height / 2, &hit) == XUI_OK &&
        hit.iKind == XUI_DOC_POSITION_SOURCE && hit.iOffset == p.iOffset);
    scale_fail_shape_after = 1;
    CHECK(xuiDocumentRendererGetCaretRect(cold, &p, &a) ==
        XUI_ERROR_OUT_OF_MEMORY);
    scale_fail_shape_after = 0;
    CHECK(xuiDocumentRendererGetCaretRect(cold, &p, &a) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    stats.iSize = sizeof(stats);
    CHECK(xuiDocumentRendererGetStats(cold, &stats) == XUI_OK &&
        stats.iShapedBytes < bytes / 4);
    CHECK(xuiDocumentRendererCreate(context, NULL, &live_cold) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &live_warm) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &live_hit) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(live_cold, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(live_warm, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(live_hit, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(live_cold, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(live_warm, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(live_hit, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetActivePosition(live_cold, &p) == XUI_OK);
    CHECK(xuiDocumentRendererSetActivePosition(live_warm, &p) == XUI_OK);
    CHECK(xuiDocumentRendererSetActivePosition(live_hit, &p) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(live_cold, 640, 0, 0) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(live_warm, 640, 0, 120) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(live_warm, &p, &b) == XUI_OK);
    CHECK(xuiDocumentRendererHitTest(live_hit, b.x + .1,
        b.y + b.height / 2, &hit) == XUI_OK &&
        hit.iKind == XUI_DOC_POSITION_SOURCE && hit.iOffset == p.iOffset);
    CHECK(xuiDocumentRendererGetCaretRect(live_cold, &p, &a) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    CHECK(xuiDocumentRendererGetStats(live_cold, &stats) == XUI_OK &&
        stats.iShapedBytes < bytes / 4);
    patch.iSize = sizeof(patch); patch.iStart = patch.iEnd = 0;
    patch.sText = head; patch.iTextBytes = sizeof(head) - 1;
    CHECK(xuiDocumentPrepareSource(document, NULL, &patch, 1,
        &prepare) == XUI_OK);
    CHECK(xuiDocumentRendererSetSourceInput(live_cold, prepare) == XUI_OK);
    CHECK(xuiDocumentRendererGetStats(live_cold, &stats) == XUI_OK &&
        stats.iSourceIncrementalUpdates > 0 && !stats.bLiveSourceFallback);
    CHECK(xuiDocumentRendererCreate(context, NULL, &live_fresh) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(live_fresh, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(live_fresh, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetActivePosition(live_fresh, &p) == XUI_OK);
    CHECK(xuiDocumentRendererSetSourceInput(live_fresh, prepare) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(live_fresh, 640, 0, 120) == XUI_OK);
    CHECK(xuiDocumentPrepareSourcePosition(prepare,
        p.iOffset + sizeof(head) - 1, XUI_DOC_AFTER,
        &candidate) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(live_cold,
        &candidate, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(live_fresh,
        &candidate, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    CHECK(xuiDocumentRendererSetSourceInput(cold, prepare) == XUI_OK);
    CHECK(xuiDocumentRendererGetStats(cold, &stats) == XUI_OK &&
        stats.iSourceIncrementalUpdates > 0);
    CHECK(xuiDocumentRendererCreate(context, NULL, &source_fresh) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(source_fresh, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(source_fresh, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSourceInput(source_fresh, prepare) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(source_fresh, 640, 0, 120) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(cold, &candidate, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(source_fresh,
        &candidate, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    scale_late_tall_factor = 2;
    CHECK(xuiDocumentRendererLayout(cold, 600, 0, 0) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(live_cold, 600, 0, 0) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &source_resized) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(source_resized, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(source_resized, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetSourceInput(source_resized, prepare) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(source_resized, 600, 0, 120) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(cold, &candidate, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(source_resized,
        &candidate, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    CHECK(xuiDocumentRendererCreate(context, NULL, &live_resized) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(live_resized, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(live_resized, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererSetActivePosition(live_resized, &p) == XUI_OK);
    CHECK(xuiDocumentRendererSetSourceInput(live_resized, prepare) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(live_resized, 600, 0, 120) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(live_cold,
        &candidate, &a) == XUI_OK);
    CHECK(xuiDocumentRendererGetCaretRect(live_resized,
        &candidate, &b) == XUI_OK);
    CHECK(fabs(a.x - b.x) < .01 && fabs(a.y - b.y) < .01);
    xuiDocumentRendererRelease(source_resized);
    xuiDocumentRendererRelease(live_resized);
    xuiDocumentRendererRelease(source_fresh);
    xuiDocumentRendererRelease(live_fresh);
    xuiDocumentRendererRelease(live_cold);
    xuiDocumentRendererRelease(live_warm);
    xuiDocumentRendererRelease(live_hit);
    xuiDocumentRendererRelease(cold_hit);
    xuiDocumentRendererRelease(cold_range);
    xuiDocumentRendererRelease(cold);
    xuiDocumentRendererRelease(warm);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentCancelPrepare(document) == XUI_OK);
    xuiDocumentPrepareRelease(prepare);
    xuiDocumentRelease(document);
    xuiDestroy(context);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    scale_late_tall_factor = 3;
    puts("SOURCE/LIVE Unicode predecessor height resolves cold range, caret, hit, splice and resize");
}
int main(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; xui_surface target;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "scale.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK && xuiInputViewport(context, 960, 640) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&proxy, &target, 960, 640, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    many_paragraphs(context, target); single_large_paragraph(context, target, &proxy);
    paragraph_prefix_geometry(context, "Alpha beta gamma delta. ", 1);
    paragraph_prefix_geometry(context, "Cafe\xcc\x81 "
        "\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x92\xbb "
        "\xd8\xa7\xd9\x84\xd8\xb9\xd8\xb1\xd8\xa8\xd9\x8a\xd8\xa9 "
        "\xe4\xb8\xad\xe6\x96\x87 ", 1);
    paragraph_prefix_geometry(context, "AAAAAAAAAAAAAAAAAA", 1);
    paragraph_prefix_geometry(context, "A\xcc\x81" "A\xcc\x81"
        "A\xcc\x81" "A\xcc\x81" "A\xcc\x81" "A\xcc\x81"
        "A\xcc\x81" "A\xcc\x81" "A\xcc\x81" "A\xcc\x81", 1);
    paragraph_prefix_geometry(context, "\xd8\xa8\xd8\xa8\xd8\xa8"
        "\xd8\xa8\xd8\xa8\xd8\xa8\xd8\xa8\xd8\xa8\xd8\xa8", 0);
    unicode_run_metrics_prefix();
    inline_object_geometry_continue(context);
    unbreakable_paragraph(context);
    underestimated_prefix_scroll(context);
    overestimated_prefix_hit(context);
    renderer_cache_budget(context);
    long_code_block(context, target, &proxy);
    unbroken_code_line(context, target, &proxy);
    protected_code_line(context);
    unbroken_source_line(context, target, &proxy);
    shifted_source_line_prefix(context);
    protected_source_line(context);
    source_callback_cold_height(context, &proxy);
    view_callback_font_invalidation(context, target, &proxy);
    source_ascii_content_height();
    source_unicode_cold_height();
    xuiDestroy(context); proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Unified Document visible-range and Editor scale regression passed");
    return 0;
}
