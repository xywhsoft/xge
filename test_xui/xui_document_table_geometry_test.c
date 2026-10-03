#include "../xui_document_ui.h"
#include "xui_test_proxy.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(e) do { if (!(e)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); exit(1); } } while (0)
static int (*base_fill)(xui_proxy, xui_draw_context, xui_rect_t, uint32_t);
static struct fill_record { xui_rect_t rect; uint32_t color; } fills[128];
static size_t fill_count;
static int record_fill(xui_proxy p, xui_draw_context d, xui_rect_t rect, uint32_t color)
{
    if (fill_count < sizeof(fills) / sizeof(fills[0])) fills[fill_count++] = (struct fill_record){rect, color};
    return base_fill(p, d, rect, color);
}
static uint64_t insert(xui_document_transaction t, uint64_t parent, uint32_t kind,
    uint32_t rows, uint32_t columns, uint32_t color)
{
    xui_doc_node_desc_t desc = {0}; uint64_t id = 0;
    desc.iSize = sizeof(desc); desc.iKind = kind;
    desc.tAttributes.iRowSpan = rows; desc.tAttributes.iColumnSpan = columns;
    desc.tAttributes.iBackgroundColor = color;
    {
        int result = xuiDocumentTxnInsertNode(t, parent, XUI_DOCUMENT_APPEND, &desc, &id);
        if (result != XUI_OK) fprintf(stderr,
            "insert kind=%u parent=%llu row_span=%u column_span=%u result=%d\n",
            kind, (unsigned long long)parent, rows, columns, result);
        CHECK(result == XUI_OK);
    }
    return id;
}
static int near(double a, double b) { return fabs(a - b) < .0005; }
static int pixel(double value) { return (int)floor(value + .5); }
static int has_fill(uint32_t color, double x, double y, double width, double height, double scroll)
{
    int left = pixel(x), top = pixel(y - scroll);
    int right = pixel(x + width), bottom = pixel(y + height - scroll);
    size_t i;
    for (i = 0; i < fill_count; i++) if (fills[i].color == color &&
        fills[i].rect.fX == left && fills[i].rect.fY == top &&
        fills[i].rect.fW == right - left && fills[i].rect.fH == bottom - top) return 1;
    return 0;
}
static void table_text_after_row_relocation(xui_context context)
{
    xui_document document; xui_document_transaction txn;
    xui_document_snapshot snapshot; xui_document_renderer renderer;
    xui_doc_position_t caret = {0}, hit = {0}; xui_doc_rect_t rect;
    uint64_t table, row, cell, paragraph, text_id = 0; int i;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    table = insert(txn, 1, XUI_DOC_TABLE, 1, 1, 0);
    for (i = 0; i < 2; i++) {
        xui_doc_node_desc_t text = {0};
        row = insert(txn, table, XUI_DOC_ROW, 1, 1, 0);
        cell = insert(txn, row, XUI_DOC_CELL, 1, 1, 0);
        paragraph = insert(txn, cell, XUI_DOC_PARAGRAPH, 1, 1, 0);
        text.iSize = sizeof(text); text.iKind = XUI_DOC_TEXT;
        text.sText = i ? "second" : "first"; text.iTextBytes = strlen(text.sText);
        CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
            &text, &text_id) == XUI_OK);
    }
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 160, 0, 160) == XUI_OK);
    caret.iSize = sizeof(caret); caret.iKind = XUI_DOC_POSITION_TEXT;
    caret.iDocumentId = xuiDocumentGetIdentity(document);
    caret.iRevision = xuiDocumentGetRevision(document);
    caret.iNodeId = text_id; caret.iOffset = 2;
    CHECK(xuiDocumentRendererGetCaretRect(renderer, &caret, &rect) == XUI_OK && rect.y > 25);
    CHECK(xuiDocumentRendererHitTest(renderer, rect.x + 1,
        rect.y + rect.height / 2, &hit) == XUI_OK && hit.iNodeId == text_id);
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    puts("Table text line index falls back after row relocation; second-row hit remains correct");
}
static void large_row_span(xui_context context)
{
    const uint32_t rows = 1024;
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_document_renderer renderer; xui_doc_cell_hit_t hit = {0};
    xui_doc_renderer_desc_t desc = {0}; uint64_t table, row, spanning = 0, last = 0;
    uint32_t i;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    table = insert(txn, 1, XUI_DOC_TABLE, 1, 1, 0);
    for (i = 0; i < rows; i++) {
        row = insert(txn, table, XUI_DOC_ROW, 1, 1, 0);
        if (i == 0) {
            spanning = insert(txn, row, XUI_DOC_CELL, rows, 1, 0);
            insert(txn, row, XUI_DOC_CELL, 1, 1, 0);
        } else if (i == 1) last = insert(txn, row, XUI_DOC_CELL, rows - 1, 1, 0);
    }
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    desc.iSize = sizeof(desc);
    CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 100, 0, 100) == XUI_OK);
    hit.iSize = sizeof(hit);
    CHECK(xuiDocumentRendererGetCellRect(renderer, spanning, &hit) == XUI_OK &&
        hit.iColumn == 0 && hit.iRowSpan == rows && near(hit.tBounds.x, 0));
    CHECK(xuiDocumentRendererGetCellRect(renderer, last, &hit) == XUI_OK &&
        hit.iColumn == 1 && hit.iRowSpan == rows - 1 && near(hit.tBounds.x, 50));
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    puts("Unified Document table maximum 1024-row merged span: column occupancy and cell geometry passed");
}
static void nested_cells(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0};
    xui_doc_cell_hit_t hit = {0}; uint64_t outer_table, outer_row, outer_cell;
    uint64_t inner_table, inner_row[2], inner_cell[4]; int i;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    outer_table = insert(txn, 1, XUI_DOC_TABLE, 1, 1, 0);
    outer_row = insert(txn, outer_table, XUI_DOC_ROW, 1, 1, 0);
    outer_cell = insert(txn, outer_row, XUI_DOC_CELL, 1, 1, 0);
    inner_table = insert(txn, outer_cell, XUI_DOC_TABLE, 1, 1, 0);
    for (i = 0; i < 2; i++) inner_row[i] = insert(txn, inner_table, XUI_DOC_ROW, 1, 1, 0);
    for (i = 0; i < 4; i++) inner_cell[i] = insert(txn, inner_row[i / 2], XUI_DOC_CELL, 1, 1, 0);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    desc.iSize = sizeof(desc);
    CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 100, 0, 20) == XUI_OK);
    hit.iSize = sizeof(hit);
    CHECK(xuiDocumentRendererGetCellRect(renderer, outer_cell, &hit) == XUI_OK);
    CHECK(
        hit.iTableId == outer_table && near(hit.tBounds.x, 0) && near(hit.tBounds.y, 0) &&
        near(hit.tBounds.width, 100) && near(hit.tBounds.height, 64));
    CHECK(xuiDocumentRendererGetCellRect(renderer, inner_cell[3], &hit) == XUI_OK);
    CHECK(
        hit.iTableId == inner_table && hit.iRow == 1 && hit.iColumn == 1 &&
        near(hit.tBounds.x, 50) && near(hit.tBounds.y, 32) &&
        near(hit.tBounds.width, 44) && near(hit.tBounds.height, 26));
    CHECK(xuiDocumentRendererHitTestCell(renderer, 1, 1, &hit) == XUI_OK &&
        hit.iCellId == outer_cell);
    CHECK(xuiDocumentRendererHitTestCell(renderer, 70, 50, &hit) == XUI_OK &&
        hit.iCellId == inner_cell[3]);
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    puts("Unified Document nested table cell bounds, off-viewport lookup and innermost hit passed");
}
static void preferred_column_widths(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_document_change_set changes; xui_document_renderer renderer;
    xui_doc_renderer_desc_t desc = {0}; xui_doc_cell_hit_t hit = {0};
    xui_doc_rect_t size; uint64_t table, merged, row, first, middle, last;
    int exact;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 3, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnSetTableColumnWidth(txn, table, 0, 30) == XUI_OK);
    CHECK(xuiDocumentTxnSetTableColumnWidth(txn, table, 2, 20) == XUI_OK);
    CHECK(xuiDocumentTxnMergeCells(txn, table, 0, 0, 1, 2, &merged) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 1, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, row, 0, &first) == XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot, row, 1, &middle) == XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot, row, 2, &last) == XUI_OK);
    desc.iSize = sizeof(desc); desc.fZoom = 1.5f;
    CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, 150, 0, 100) == XUI_OK);
    hit.iSize = sizeof(hit);
    CHECK(xuiDocumentRendererGetCellRect(renderer, merged, &hit) == XUI_OK &&
        near(hit.tBounds.x, 0) && near(hit.tBounds.width, 120));
    CHECK(xuiDocumentRendererGetCellRect(renderer, first, &hit) == XUI_OK &&
        near(hit.tBounds.x, 0) && near(hit.tBounds.width, 45));
    CHECK(xuiDocumentRendererGetCellRect(renderer, middle, &hit) == XUI_OK &&
        near(hit.tBounds.x, 45) && near(hit.tBounds.width, 75));
    CHECK(xuiDocumentRendererGetCellRect(renderer, last, &hit) == XUI_OK &&
        near(hit.tBounds.x, 120) && near(hit.tBounds.width, 30));
    CHECK(xuiDocumentRendererLayout(renderer, 60, 0, 100) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact && near(size.width, 111));
    CHECK(xuiDocumentRendererGetCellRect(renderer, middle, &hit) == XUI_OK &&
        near(hit.tBounds.x, 45) && near(hit.tBounds.width, 36));
    CHECK(xuiDocumentRendererGetCellRect(renderer, last, &hit) == XUI_OK &&
        near(hit.tBounds.x, 81) && near(hit.tBounds.width, 30));
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnSetTableColumnWidth(txn, table, 0, 40) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, &changes) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererSetSnapshot(renderer, snapshot, changes) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentChangeSetRelease(changes);
    CHECK(xuiDocumentRendererLayout(renderer, 150, 0, 100) == XUI_OK);
    CHECK(xuiDocumentRendererGetCellRect(renderer, first, &hit) == XUI_OK &&
        near(hit.tBounds.width, 60));
    CHECK(xuiDocumentRendererGetCellRect(renderer, middle, &hit) == XUI_OK &&
        near(hit.tBounds.x, 60) && near(hit.tBounds.width, 60));
    xuiDocumentRendererRelease(renderer); xuiDocumentRelease(document);
    puts("Unified Document table preferred column widths: spans, zoom, overflow and change-set reflow passed");
}
int main(int argc, char** argv)
{
    static const double scrolls[] = {0, .25, .5};
    const double width = 101.3, column_width = width / 3, row_height = 32.5;
    const uint32_t colors[] = {0x223344ffu, 0x334455ffu, 0x445566ffu,
        0x556677ffu, 0x667788ffu, 0x778899ffu};
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; xui_surface target;
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_document_renderer renderer; xui_doc_renderer_desc_t desc = {0};
    xui_doc_view_desc_t view_desc = {0}; xui_doc_cell_hit_t hit = {0};
    xui_widget view; xui_draw_context draw; xui_doc_rect_t size;
    uint64_t table, row[3], cells[6]; int exact, phase, i;
    xui_doc_rect_t expected[6] = {
        {0, 0, 2 * column_width, 2 * row_height},
        {2 * column_width, 0, column_width, row_height},
        {2 * column_width, row_height, column_width, row_height},
        {0, 2 * row_height, column_width, row_height},
        {column_width, 2 * row_height, column_width, row_height},
        {2 * column_width, 2 * row_height, column_width, row_height}
    };
    const uint32_t rows[] = {0, 0, 1, 2, 2, 2};
    const uint32_t columns[] = {0, 2, 2, 0, 1, 2};
    xuiTestProxyInit(&proxy); base_fill = proxy.tProxy.drawRectFill;
    proxy.tProxy.drawRectFill = record_fill;
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "table-geometry.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK &&
        xuiTestSurfaceCreate(&proxy, &target, 160, 160, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    table = insert(txn, 1, XUI_DOC_TABLE, 1, 1, 0);
    for (i = 0; i < 3; i++) row[i] = insert(txn, table, XUI_DOC_ROW, 1, 1, 0);
    cells[0] = insert(txn, row[0], XUI_DOC_CELL, 2, 2, colors[0]);
    cells[1] = insert(txn, row[0], XUI_DOC_CELL, 1, 1, colors[1]);
    cells[2] = insert(txn, row[1], XUI_DOC_CELL, 1, 1, colors[2]);
    for (i = 3; i < 6; i++) cells[i] = insert(txn, row[2], XUI_DOC_CELL, 1, 1, colors[i]);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    desc.iSize = sizeof(desc); desc.fZoom = 1.25f;
    CHECK(xuiDocumentRendererCreate(context, &desc, &renderer) == XUI_OK &&
        xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentRendererLayout(renderer, width, 0, 160) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(renderer, &size, &exact) == XUI_OK && exact &&
        near(size.height, 3 * row_height));
    for (i = 0; i < 6; i++) {
        hit.iSize = sizeof(hit);
        CHECK(xuiDocumentRendererGetCellRect(renderer, cells[i], &hit) == XUI_OK);
        CHECK(hit.iTableId == table && hit.iCellId == cells[i] &&
            hit.iRow == rows[i] && hit.iColumn == columns[i] &&
            hit.iRowSpan == (i == 0 ? 2u : 1u) && hit.iColumnSpan == (i == 0 ? 2u : 1u));
        CHECK(near(hit.tBounds.x, expected[i].x) && near(hit.tBounds.y, expected[i].y) &&
            near(hit.tBounds.width, expected[i].width) && near(hit.tBounds.height, expected[i].height));
        CHECK(xuiDocumentRendererHitTestCell(renderer,
            expected[i].x + expected[i].width / 2,
            expected[i].y + expected[i].height / 2, &hit) == XUI_OK && hit.iCellId == cells[i]);
    }
    hit.iSize = sizeof(hit);
    CHECK(xuiDocumentRendererHitTestCell(renderer, -1, 10, &hit) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiDocumentRendererHitTestCell(renderer, 10, 3 * row_height + 1, &hit) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiDocumentRendererGetCellRect(renderer, table, &hit) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiDocumentRendererHitTestCell(renderer, 2 * column_width, 10, &hit) == XUI_OK && hit.iCellId == cells[1]);
    CHECK(xuiDocumentRendererHitTestCell(renderer, column_width, row_height, &hit) == XUI_OK && hit.iCellId == cells[0]);
    for (phase = 0; phase < 3; phase++) {
        fill_count = 0;
        CHECK(proxy.tProxy.drawBegin(&proxy.tProxy, &draw, target) == XUI_OK);
        CHECK(xuiDocumentRendererDraw(renderer, draw, 0, -scrolls[phase],
            (xui_rect_t){0, 0, 160, 160}, NULL, 0) == XUI_OK);
        CHECK(proxy.tProxy.drawEnd(&proxy.tProxy, draw) == XUI_OK);
        for (i = 0; i < 6; i++) CHECK(has_fill(colors[i], expected[i].x, expected[i].y,
            expected[i].width, expected[i].height, scrolls[phase]));
    }
    CHECK(xuiInputViewport(context, 101, 50) == XUI_OK);
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
    view_desc.tRenderer.iSize = sizeof(view_desc.tRenderer); view_desc.tRenderer.fZoom = 1.25f;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK &&
        xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 101, 50}) == XUI_OK && xuiLayout(context) == XUI_OK);
    CHECK(xuiDocumentViewSetScroll(view, 0, .25) == XUI_OK);
    hit.iSize = sizeof(hit);
    CHECK(xuiDocumentViewGetCellRect(view, cells[0], &hit) == XUI_OK &&
        near(hit.tBounds.y, -.25));
    CHECK(xuiDocumentViewHitTestCell(view, 10, 10, &hit) == XUI_OK && hit.iCellId == cells[0]);
    CHECK(xuiDocumentViewHitTestCell(view, 90, 40, &hit) == XUI_OK && hit.iCellId == cells[2]);
    CHECK(xuiSetRootWidget(context, NULL) == XUI_OK);
    xuiWidgetDestroy(view); xuiDocumentRendererRelease(renderer);
    xuiDocumentRelease(document);
    table_text_after_row_relocation(context);
    nested_cells(context);
    preferred_column_widths(context);
    if (argc > 1 && !strcmp(argv[1], "span")) large_row_span(context);
    xuiDestroy(context);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Unified Document table geometry: merged spans, fractionally aligned cell paint, renderer/view rect and padded-cell hit passed");
    return 0;
}
