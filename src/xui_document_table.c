#include "xui_document_internal.h"

typedef struct doc_grid_cell { uint64_t id; uint32_t row, column, rows, columns; } doc_grid_cell;
typedef struct doc_grid { doc_grid_cell* cells; uint64_t count; uint32_t rows, columns; } doc_grid;
static int doc_table_grid(xui_document_transaction t, uint64_t id, doc_grid* grid)
{
    doc_node* table; uint32_t occupied[1024] = {0}; uint64_t r, i, count = 0; uint32_t j; int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    memset(grid, 0, sizeof(*grid)); if (result != XUI_OK) return result;
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    table = doc_index_get(t->draft->index, id);
    if (!table || table->kind != XUI_DOC_TABLE) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (doc_seq_size(table->children) > UINT32_MAX) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    grid->rows = (uint32_t)doc_seq_size(table->children);
    for (r = 0; r < grid->rows; r++) count += doc_seq_size(doc_index_get(t->draft->index, doc_seq_get_id(table->children, r))->children);
    if (count > SIZE_MAX / sizeof(*grid->cells)) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    grid->cells = doc_alloc(t->document->allocator, (size_t)(count ? count : 1) * sizeof(*grid->cells));
    if (!grid->cells) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    for (r = 0; r < grid->rows; r++) {
        doc_node* row = doc_index_get(t->draft->index, doc_seq_get_id(table->children, r)); uint32_t column = 0, width = 0;
        for (i = 0; i < doc_seq_size(row->children); i++) {
            doc_node* cell = doc_index_get(t->draft->index, doc_seq_get_id(row->children, i)); uint32_t w = cell->attrs.iColumnSpan, h = cell->attrs.iRowSpan;
            while (column < 1024 && occupied[column]) column++;
            if (!w || !h || w > 1024 - column || h > grid->rows - r) goto invalid;
            for (j = 0; j < w; j++) { if (occupied[column + j]) goto invalid; occupied[column + j] = h; }
            grid->cells[grid->count++] = (doc_grid_cell){cell->id, (uint32_t)r, column, h, w}; column += w;
        }
        for (j = 0; j < 1024; j++) if (occupied[j]) width = j + 1;
        if (!r) grid->columns = width;
        if (!width || width != grid->columns) goto invalid;
        for (j = 0; j < width; j++) { if (!occupied[j]) goto invalid; occupied[j]--; }
    }
    return XUI_OK;
invalid:
    doc_free(grid->cells); grid->cells = NULL; return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
}
static uint64_t doc_table_row(xui_document_transaction t, uint64_t table, uint32_t row)
{
    return doc_seq_get_id(doc_index_get(t->draft->index, table)->children, row);
}
static int doc_table_span(xui_document_transaction t, uint64_t id, uint32_t rows, uint32_t columns)
{
    xui_doc_attributes_t attrs = doc_index_get(t->draft->index, id)->attrs; attrs.iRowSpan = rows; attrs.iColumnSpan = columns;
    return xuiDocumentTxnSetAttributes(t, id, &attrs);
}
static int doc_table_empty_cell(xui_document_transaction t, uint64_t row, uint64_t index, uint32_t flags)
{
    xui_doc_node_desc_t desc = {0}; uint64_t cell, paragraph, text; int result;
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_CELL; desc.tAttributes.iFlags = flags;
    result = doc_txn_insert(t, row, index, &desc, &cell); if (result != XUI_OK) return result;
    desc.iKind = XUI_DOC_PARAGRAPH; desc.tAttributes.iFlags = 0;
    result = doc_txn_insert(t, cell, DOC_NONE, &desc, &paragraph); if (result != XUI_OK) return result;
    desc.iKind = XUI_DOC_TEXT; return doc_txn_insert(t, paragraph, DOC_NONE, &desc, &text);
}
XUI_API int xuiDocumentTxnInsertTable(xui_document_transaction t, uint64_t parent, uint64_t index, uint32_t rows, uint32_t columns, int header, uint64_t* out)
{
    xui_doc_node_desc_t desc = {0}; uint64_t table, row; uint32_t r, c; int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (out) *out = 0;
    if (result != XUI_OK) return result;
    if (!out || !rows || !columns || columns > 1024) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if ((uint64_t)rows * columns > (t->document->max_nodes - t->draft->node_count) / 3) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_TABLE; result = doc_txn_insert(t, parent, index, &desc, &table);
    for (r = 0; r < rows && result == XUI_OK; r++) {
        desc.iKind = XUI_DOC_ROW; result = doc_txn_insert(t, table, DOC_NONE, &desc, &row);
        for (c = 0; c < columns && result == XUI_OK; c++) result = doc_table_empty_cell(t, row, DOC_NONE, header && !r ? XUI_DOC_HEADER : 0);
    }
    if (result == XUI_OK) *out = table;
    return result;
}
XUI_API int xuiDocumentTxnInsertTableRow(xui_document_transaction t, uint64_t table, uint32_t row)
{
    doc_grid g; uint64_t i, id; uint32_t c, covered[1024] = {0}; xui_doc_node_desc_t desc = {0}; int result = doc_table_grid(t, table, &g);
    if (result != XUI_OK) return result;
    if (row > g.rows || !g.columns) { result = XUI_ERROR_INVALID_ARGUMENT; goto done; }
    for (i = 0; i < g.count; i++) {
        doc_grid_cell* cell = &g.cells[i];
        if (cell->row < row && cell->row + cell->rows > row) {
            result = doc_table_span(t, cell->id, cell->rows + 1, cell->columns); if (result != XUI_OK) goto done;
            for (c = cell->column; c < cell->column + cell->columns; c++) covered[c] = 1;
        }
    }
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_ROW; result = doc_txn_insert(t, table, row, &desc, &id);
    for (c = 0; c < g.columns && result == XUI_OK; c++) if (!covered[c]) result = doc_table_empty_cell(t, id, DOC_NONE, 0);
done:
    doc_free(g.cells); return result == XUI_OK ? result : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnDeleteTableRow(xui_document_transaction t, uint64_t table, uint32_t row)
{
    doc_grid g; uint64_t i, j, row_id; int result = doc_table_grid(t, table, &g);
    if (result != XUI_OK) return result;
    if (row >= g.rows) { result = XUI_ERROR_INVALID_ARGUMENT; goto done; }
    if (g.rows == 1) { result = doc_txn_delete(t, table); goto done; }
    row_id = doc_table_row(t, table, row);
    /* Reverse order means each moved cell is inserted before already moved
     * cells to its right. Existing cells retain their original ordering. */
    for (i = g.count; i > 0; i--) {
        doc_grid_cell* cell = &g.cells[i - 1];
        if (cell->row <= row && cell->row + cell->rows > row && cell->rows > 1) {
            result = doc_table_span(t, cell->id, cell->rows - 1, cell->columns); if (result != XUI_OK) goto done;
            if (cell->row == row) {
                uint64_t index = 0;
                for (j = 0; j < g.count; j++) if (g.cells[j].row == row + 1 && g.cells[j].column < cell->column) index++;
                result = xuiDocumentTxnMoveNode(t, cell->id, doc_table_row(t, table, row + 1), index); if (result != XUI_OK) goto done;
            }
        }
    }
    result = doc_txn_delete(t, row_id);
done:
    doc_free(g.cells); return result == XUI_OK ? result : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnInsertTableColumn(xui_document_transaction t, uint64_t table, uint32_t column)
{
    doc_grid g; uint64_t i; uint32_t row; int result = doc_table_grid(t, table, &g);
    if (result != XUI_OK) return result;
    if (column > g.columns || g.columns == 1024) { result = XUI_ERROR_INVALID_ARGUMENT; goto done; }
    for (i = 0; i < g.count; i++) if (g.cells[i].column < column && g.cells[i].column + g.cells[i].columns > column) {
        result = doc_table_span(t, g.cells[i].id, g.cells[i].rows, g.cells[i].columns + 1); if (result != XUI_OK) goto done;
    }
    for (row = 0; row < g.rows; row++) {
        int covered = 0; uint64_t index = 0; uint32_t flags = 0;
        for (i = 0; i < g.count; i++) {
            doc_grid_cell* cell = &g.cells[i];
            if (cell->row <= row && cell->row + cell->rows > row && cell->column < column && cell->column + cell->columns > column) covered = 1;
            if (cell->row == row) {
                if (cell->column < column) index++;
                flags |= doc_index_get(t->draft->index, cell->id)->attrs.iFlags & XUI_DOC_HEADER;
            }
        }
        if (!covered) { result = doc_table_empty_cell(t, doc_table_row(t, table, row), index, flags); if (result != XUI_OK) goto done; }
    }
done:
    doc_free(g.cells); return result == XUI_OK ? result : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnDeleteTableColumn(xui_document_transaction t, uint64_t table, uint32_t column)
{
    doc_grid g; uint64_t i; int result = doc_table_grid(t, table, &g);
    if (result != XUI_OK) return result;
    if (column >= g.columns) { result = XUI_ERROR_INVALID_ARGUMENT; goto done; }
    if (g.columns == 1) { result = doc_txn_delete(t, table); goto done; }
    for (i = 0; i < g.count; i++) if (g.cells[i].column <= column && g.cells[i].column + g.cells[i].columns > column) {
        result = g.cells[i].columns > 1 ? doc_table_span(t, g.cells[i].id, g.cells[i].rows, g.cells[i].columns - 1) : doc_txn_delete(t, g.cells[i].id);
        if (result != XUI_OK) goto done;
    }
done:
    doc_free(g.cells); return result == XUI_OK ? result : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnMergeCells(xui_document_transaction t, uint64_t table, uint32_t row, uint32_t column, uint32_t rows, uint32_t columns, uint64_t* out)
{
    doc_grid g; uint64_t i, anchor = 0; int result = doc_table_grid(t, table, &g);
    if (out) *out = 0;
    if (result != XUI_OK) return result;
    if (!out || !rows || !columns || row >= g.rows || column >= g.columns || rows > g.rows - row || columns > g.columns - column) { result = XUI_ERROR_INVALID_ARGUMENT; goto done; }
    for (i = 0; i < g.count; i++) {
        doc_grid_cell* c = &g.cells[i];
        if (c->row < row + rows && c->row + c->rows > row && c->column < column + columns && c->column + c->columns > column) {
            if (c->row < row || c->column < column || c->row + c->rows > row + rows || c->column + c->columns > column + columns) { result = XUI_DOC_ERROR_SCHEMA; goto done; }
            if (c->row == row && c->column == column) anchor = c->id;
        }
    }
    if (!anchor) { result = XUI_DOC_ERROR_SCHEMA; goto done; }
    for (i = 0; i < g.count; i++) {
        doc_grid_cell* c = &g.cells[i]; doc_node* n;
        if (c->id == anchor || c->row < row || c->row >= row + rows || c->column < column || c->column >= column + columns) continue;
        while (doc_seq_size((n = doc_index_get(t->draft->index, c->id))->children)) {
            result = xuiDocumentTxnMoveNode(t, doc_seq_get_id(n->children, 0), anchor, DOC_NONE); if (result != XUI_OK) goto done;
        }
        result = doc_txn_delete(t, c->id); if (result != XUI_OK) goto done;
    }
    result = doc_table_span(t, anchor, rows, columns); if (result == XUI_OK) *out = anchor;
done:
    doc_free(g.cells); return result == XUI_OK ? result : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnSplitCell(xui_document_transaction t, uint64_t cell)
{
    doc_grid g; doc_grid_cell original = {0}; doc_node *n, *row; uint64_t table, i; uint32_t r, c, flags; int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    n = doc_index_get(t->draft->index, cell); if (!n || n->kind != XUI_DOC_CELL) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    flags = n->attrs.iFlags; row = doc_index_get(t->draft->index, n->parent); table = row->parent;
    result = doc_table_grid(t, table, &g); if (result != XUI_OK) return result;
    for (i = 0; i < g.count; i++) if (g.cells[i].id == cell) original = g.cells[i];
    result = doc_table_span(t, cell, 1, 1); if (result != XUI_OK) goto done;
    for (r = original.row; r < original.row + original.rows; r++) for (c = original.column + original.columns; c > original.column; c--) {
        uint64_t index = 0; uint32_t column = c - 1;
        if (r == original.row && column == original.column) continue;
        for (i = 0; i < g.count; i++) if (g.cells[i].row == r && g.cells[i].column < column) index++;
        result = doc_table_empty_cell(t, doc_table_row(t, table, r), index, flags); if (result != XUI_OK) goto done;
    }
done:
    doc_free(g.cells); return result == XUI_OK ? result : doc_txn_fail(t, result);
}
