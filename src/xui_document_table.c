#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include <math.h>

typedef struct doc_grid_cell { uint64_t id; uint32_t row, column, rows, columns; } doc_grid_cell;
typedef struct doc_grid { doc_grid_cell* cells; uint64_t count; uint32_t rows, columns; } doc_grid;
static int doc_table_grid(xui_document_transaction, uint64_t, doc_grid*);
uint32_t doc_table_column_count(const doc_state* state, const doc_node* table)
{
    doc_node* row; uint64_t i; uint32_t columns = 0;
    if (!state || !table || table->kind != XUI_DOC_TABLE || !doc_seq_size(table->children)) return 0;
    row = doc_index_get(state->index, doc_seq_get_id(table->children, 0));
    if (!row || row->kind != XUI_DOC_ROW) return 0;
    for (i = 0; i < doc_seq_size(row->children); i++) {
        doc_node* cell = doc_index_get(state->index, doc_seq_get_id(row->children, i));
        if (!cell || cell->kind != XUI_DOC_CELL || cell->attrs->iColumnSpan > 1024 - columns) return 0;
        columns += cell->attrs->iColumnSpan;
    }
    return columns;
}
float doc_table_column_width(const doc_node* table, uint32_t column)
{
    float width = 0;
    if (table && table->column_widths &&
        column < table->column_widths->size / sizeof(width))
        memcpy(&width, table->column_widths->data + (size_t)column * sizeof(width), sizeof(width));
    return width;
}
static int doc_table_walk_cell(const doc_state* state, uint64_t table_id, uint64_t wanted_id,
    uint32_t wanted_row, uint32_t wanted_column, doc_table_cell_slot* out)
{
    doc_node* table = doc_index_get(state->index, table_id);
    uint32_t occupied[1024] = {0}, rows, columns, r, c;
    doc_table_cell_slot covering[1024] = {{0}};
    if (!table || table->kind != XUI_DOC_TABLE || !out ||
        doc_seq_size(table->children) > UINT32_MAX) return XUI_ERROR_INVALID_ARGUMENT;
    rows = (uint32_t)doc_seq_size(table->children);
    columns = doc_table_column_count(state, table);
    if (!columns || columns > 1024) return XUI_DOC_ERROR_SCHEMA;
    if (!wanted_id && (wanted_row >= rows || wanted_column >= columns)) return XUI_ERROR_INVALID_ARGUMENT;
    for (r = 0; r < rows; r++) {
        doc_node* row = doc_index_get(state->index, doc_seq_get_id(table->children, r));
        uint32_t column = 0; uint64_t i;
        if (!row || row->kind != XUI_DOC_ROW) return XUI_DOC_ERROR_SCHEMA;
        if (!wanted_id && r == wanted_row && occupied[wanted_column]) {
            *out = covering[wanted_column]; return XUI_OK;
        }
        for (i = 0; i < doc_seq_size(row->children); i++) {
            doc_node* cell = doc_index_get(state->index, doc_seq_get_id(row->children, i));
            uint32_t k, w, h;
            while (column < columns && occupied[column]) column++;
            if (!cell || cell->kind != XUI_DOC_CELL) return XUI_DOC_ERROR_SCHEMA;
            w = cell->attrs->iColumnSpan; h = cell->attrs->iRowSpan;
            if (!w || !h || column >= columns || w > columns - column || h > rows - r)
                return XUI_DOC_ERROR_SCHEMA;
            if ((wanted_id && cell->id == wanted_id) ||
                (!wanted_id && r == wanted_row && wanted_column >= column && wanted_column - column < w)) {
                *out = (doc_table_cell_slot){table_id, cell->id, r, column, h, w, rows, columns};
                return XUI_OK;
            }
            for (k = 0; k < w; k++) {
                if (occupied[column + k]) return XUI_DOC_ERROR_SCHEMA;
                occupied[column + k] = h;
                covering[column + k] = (doc_table_cell_slot){table_id, cell->id, r, column,
                    h, w, rows, columns};
            }
            column += w;
        }
        for (c = 0; c < columns; c++) if (occupied[c]) occupied[c]--;
    }
    return XUI_ERROR_NOT_FOUND;
}
int doc_table_locate_cell(const doc_state* state, uint64_t cell_id, doc_table_cell_slot* out)
{
    doc_node *cell, *row;
    if (!state || !out) return XUI_ERROR_INVALID_ARGUMENT;
    cell = doc_index_get(state->index, cell_id);
    if (!cell || cell->kind != XUI_DOC_CELL) return XUI_ERROR_NOT_FOUND;
    row = doc_index_get(state->index, cell->parent);
    if (!row || row->kind != XUI_DOC_ROW) return XUI_DOC_ERROR_SCHEMA;
    return doc_table_walk_cell(state, row->parent, cell_id, 0, 0, out);
}
int doc_table_cell_at(const doc_state* state, uint64_t table, uint32_t row, uint32_t column,
    doc_table_cell_slot* out)
{
    if (!state || !out) return XUI_ERROR_INVALID_ARGUMENT;
    return doc_table_walk_cell(state, table, 0, row, column, out);
}
int doc_table_can_merge(const doc_state* state, uint64_t table_id, uint32_t row,
    uint32_t column, uint32_t rows, uint32_t columns)
{
    doc_node* table; uint32_t occupied[1024] = {0}, grid_rows, grid_columns, r, c;
    uint64_t included = 0;
    if (!state || !(table = doc_index_get(state->index, table_id)) ||
        table->kind != XUI_DOC_TABLE || !rows || !columns) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_seq_size(table->children) > UINT32_MAX) return XUI_DOC_ERROR_LIMIT;
    grid_rows = (uint32_t)doc_seq_size(table->children);
    grid_columns = doc_table_column_count(state, table);
    if (row >= grid_rows || rows > grid_rows - row || column >= grid_columns ||
        columns > grid_columns - column) return XUI_ERROR_INVALID_ARGUMENT;
    for (r = 0; r < grid_rows; r++) {
        doc_node* row_node = doc_index_get(state->index, doc_seq_get_id(table->children, r));
        uint32_t current = 0; uint64_t i;
        if (!row_node || row_node->kind != XUI_DOC_ROW) return XUI_DOC_ERROR_SCHEMA;
        for (i = 0; i < doc_seq_size(row_node->children); i++) {
            doc_node* cell = doc_index_get(state->index, doc_seq_get_id(row_node->children, i));
            uint32_t w, h, k;
            while (current < grid_columns && occupied[current]) current++;
            if (!cell || cell->kind != XUI_DOC_CELL) return XUI_DOC_ERROR_SCHEMA;
            w = cell->attrs->iColumnSpan; h = cell->attrs->iRowSpan;
            if (!w || !h || current >= grid_columns || w > grid_columns - current || h > grid_rows - r)
                return XUI_DOC_ERROR_SCHEMA;
            if (r < row + rows && r + h > row && current < column + columns &&
                current + w > column) {
                if (r < row || current < column || r + h > row + rows ||
                    current + w > column + columns) return XUI_DOC_ERROR_SCHEMA;
                included++;
            }
            for (k = 0; k < w; k++) {
                if (occupied[current + k]) return XUI_DOC_ERROR_SCHEMA;
                occupied[current + k] = h;
            }
            current += w;
        }
        for (c = 0; c < grid_columns; c++) if (occupied[c]) occupied[c]--;
    }
    return included > 1 ? XUI_OK : XUI_ERROR_INVALID_STATE;
}
static int doc_table_set_widths(xui_document_transaction t, uint64_t id, const float* widths, uint32_t columns)
{
    doc_node* table = doc_index_get(t->draft->index, id); doc_node* copy;
    doc_blob* blob = NULL; xui_doc_operation_t op = {0}; uint32_t i; int any = 0, result;
    if (!table || table->kind != XUI_DOC_TABLE || !widths || !columns || columns > 1024)
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    for (i = 0; i < columns; i++) {
        if (!isfinite(widths[i]) || widths[i] < 0 || widths[i] > 1000000) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
        if (widths[i] > 0) any = 1;
    }
    if ((!any && !table->column_widths) ||
        (any && table->column_widths && table->column_widths->size == (uint64_t)columns * sizeof(float) &&
            !memcmp(table->column_widths->data, widths, (size_t)columns * sizeof(float)))) return XUI_OK;
    if (any) {
        blob = doc_blob_new(t->draft->allocator, (const char*)widths, (uint64_t)columns * sizeof(float));
        if (!blob) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    }
    copy = doc_node_clone(t->draft->allocator, table);
    if (!copy) { doc_blob_release(blob); return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY); }
    doc_blob_release(copy->column_widths); copy->column_widths = blob;
    result = doc_state_set(t->draft, copy); doc_node_release(copy);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    op.iKind = XUI_DOC_OP_ATTRIBUTES; op.iFlags = XUI_DOC_CHANGE_STYLE; op.iNodeId = id;
    return doc_txn_op(t, &op);
}
XUI_API int xuiDocumentSnapshotGetTableColumnWidth(xui_document_snapshot snapshot,
    xui_doc_node_id id, uint32_t column, float* out)
{
    doc_node* table; uint32_t columns;
    if (out) *out = 0;
    if (!snapshot || !out) return XUI_ERROR_INVALID_ARGUMENT;
    table = doc_index_get(snapshot->state->index, id);
    if (!table || table->kind != XUI_DOC_TABLE) return XUI_ERROR_NOT_FOUND;
    columns = doc_table_column_count(snapshot->state, table);
    if (column >= columns) return XUI_ERROR_INVALID_ARGUMENT;
    *out = doc_table_column_width(table, column); return XUI_OK;
}
XUI_API int xuiDocumentTxnSetTableColumnWidth(xui_document_transaction t,
    xui_doc_node_id id, uint32_t column, float width)
{
    doc_grid grid; float widths[1024]; uint32_t i; int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (t->draft->profile != XUI_DOCUMENT_RICH) return doc_txn_fail(t, XUI_DOC_ERROR_UNREPRESENTABLE);
    if (!isfinite(width) || width < 0 || width > 1000000) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_table_grid(t, id, &grid); if (result != XUI_OK) return result;
    if (column >= grid.columns) { doc_free(grid.cells); return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT); }
    for (i = 0; i < grid.columns; i++) widths[i] = doc_table_column_width(doc_index_get(t->draft->index, id), i);
    widths[column] = width;
    result = doc_table_set_widths(t, id, widths, grid.columns);
    doc_free(grid.cells); return result;
}
static int doc_table_grid_state(doc_state* state, uint64_t id, doc_grid* grid)
{
    doc_node* table; uint32_t occupied[1024] = {0}; uint64_t r, i, count = 0; uint32_t j;
    memset(grid, 0, sizeof(*grid));
    table = doc_index_get(state->index, id);
    if (!table || table->kind != XUI_DOC_TABLE) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_seq_size(table->children) > UINT32_MAX) return XUI_DOC_ERROR_LIMIT;
    grid->rows = (uint32_t)doc_seq_size(table->children);
    for (r = 0; r < grid->rows; r++) {
        doc_node* row = doc_index_get(state->index, doc_seq_get_id(table->children, r));
        if (!row || row->kind != XUI_DOC_ROW || doc_seq_size(row->children) > UINT64_MAX - count)
            return XUI_DOC_ERROR_SCHEMA;
        count += doc_seq_size(row->children);
    }
    if (count > SIZE_MAX / sizeof(*grid->cells)) return XUI_DOC_ERROR_LIMIT;
    grid->cells = doc_alloc(state->allocator, (size_t)(count ? count : 1) * sizeof(*grid->cells));
    if (!grid->cells) return XUI_ERROR_OUT_OF_MEMORY;
    for (r = 0; r < grid->rows; r++) {
        doc_node* row = doc_index_get(state->index, doc_seq_get_id(table->children, r)); uint32_t column = 0, width = 0;
        for (i = 0; i < doc_seq_size(row->children); i++) {
            doc_node* cell = doc_index_get(state->index, doc_seq_get_id(row->children, i));
            uint32_t w, h;
            if (!cell || cell->kind != XUI_DOC_CELL) goto invalid;
            w = cell->attrs->iColumnSpan; h = cell->attrs->iRowSpan;
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
    doc_free(grid->cells); grid->cells = NULL; return XUI_DOC_ERROR_SCHEMA;
}
static int doc_table_grid(xui_document_transaction t, uint64_t id, doc_grid* grid)
{
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing)
        return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    result = doc_table_grid_state(t->draft, id, grid);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
static uint64_t doc_table_row(xui_document_transaction t, uint64_t table, uint32_t row)
{
    return doc_seq_get_id(doc_index_get(t->draft->index, table)->children, row);
}
static int doc_table_span(xui_document_transaction t, uint64_t id, uint32_t rows, uint32_t columns)
{
    xui_doc_attributes_t attrs = *doc_index_get(t->draft->index, id)->attrs; attrs.iRowSpan = rows; attrs.iColumnSpan = columns;
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
static int doc_table_insert(xui_document_transaction t, uint64_t parent, uint64_t index, uint32_t rows, uint32_t columns, int header, uint64_t* out)
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
static int doc_table_insert_row(xui_document_transaction t, uint64_t table, uint32_t row)
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
static int doc_table_delete_row(xui_document_transaction t, uint64_t table, uint32_t row)
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
static int doc_table_insert_column(xui_document_transaction t, uint64_t table, uint32_t column)
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
                flags |= doc_index_get(t->draft->index, cell->id)->attrs->iFlags & XUI_DOC_HEADER;
            }
        }
        if (!covered) { result = doc_table_empty_cell(t, doc_table_row(t, table, row), index, flags); if (result != XUI_OK) goto done; }
    }
    if (doc_index_get(t->draft->index, table)->column_widths) {
        float widths[1024]; doc_node* node = doc_index_get(t->draft->index, table); uint32_t c;
        for (c = 0; c < g.columns + 1; c++)
            widths[c] = c == column ? 0 : doc_table_column_width(node, c < column ? c : c - 1);
        result = doc_table_set_widths(t, table, widths, g.columns + 1);
    }
done:
    doc_free(g.cells); return result == XUI_OK ? result : doc_txn_fail(t, result);
}
static int doc_table_delete_column(xui_document_transaction t, uint64_t table, uint32_t column)
{
    doc_grid g; uint64_t i; int result = doc_table_grid(t, table, &g);
    if (result != XUI_OK) return result;
    if (column >= g.columns) { result = XUI_ERROR_INVALID_ARGUMENT; goto done; }
    if (g.columns == 1) { result = doc_txn_delete(t, table); goto done; }
    for (i = 0; i < g.count; i++) if (g.cells[i].column <= column && g.cells[i].column + g.cells[i].columns > column) {
        result = g.cells[i].columns > 1 ? doc_table_span(t, g.cells[i].id, g.cells[i].rows, g.cells[i].columns - 1) : doc_txn_delete(t, g.cells[i].id);
        if (result != XUI_OK) goto done;
    }
    if (doc_index_get(t->draft->index, table)->column_widths) {
        float widths[1024]; doc_node* node = doc_index_get(t->draft->index, table); uint32_t c;
        for (c = 0; c < g.columns - 1; c++) widths[c] = doc_table_column_width(node, c < column ? c : c + 1);
        result = doc_table_set_widths(t, table, widths, g.columns - 1);
    }
done:
    doc_free(g.cells); return result == XUI_OK ? result : doc_txn_fail(t, result);
}
/* GFM always has one header row and one alignment per column. Row/column
 * commands preserve those invariants, including replacing the first row. */
static int doc_table_markdown_normalize(xui_document_transaction t, uint64_t id)
{
    doc_node* table = doc_index_get(t->draft->index, id); uint64_t r, c; uint32_t align[1024] = {0}; int result;
    if (!table) return XUI_OK;
    for (r = 0; r < doc_seq_size(table->children); r++) {
        doc_node* row = doc_index_get(t->draft->index, doc_seq_get_id(table->children, r));
        for (c = 0; c < doc_seq_size(row->children); c++) {
            doc_node* cell = doc_index_get(t->draft->index, doc_seq_get_id(row->children, c));
            if (c >= 1024) return XUI_DOC_ERROR_LIMIT;
            if (cell->attrs->iAlignment) align[c] = cell->attrs->iAlignment;
        }
    }
    for (r = 0; r < doc_seq_size(table->children); r++) {
        doc_node* row = doc_index_get(t->draft->index, doc_seq_get_id(table->children, r));
        uint64_t row_id = row->id, count = doc_seq_size(row->children);
        for (c = 0; c < count; c++) {
            doc_node* cell; xui_doc_attributes_t attrs;
            row = doc_index_get(t->draft->index, row_id);
            cell = doc_index_get(t->draft->index, doc_seq_get_id(row->children, c)); attrs = *cell->attrs;
            attrs.iFlags = (attrs.iFlags & ~XUI_DOC_HEADER) | (!r ? XUI_DOC_HEADER : 0); attrs.iAlignment = align[c];
            result = xuiDocumentTxnSetAttributes(t, cell->id, &attrs); if (result != XUI_OK) return result;
        }
        table = doc_index_get(t->draft->index, id);
    }
    return XUI_OK;
}
XUI_API int xuiDocumentTxnInsertTable(xui_document_transaction t, uint64_t parent, uint64_t index, uint32_t rows, uint32_t columns, int header, uint64_t* out)
{
    struct xui_doc_transaction_t shadow; uint64_t created; int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (out) *out = 0;
    if (result != XUI_OK) return result;
    if (t->draft->profile != XUI_DOCUMENT_MARKDOWN || t->parsing) return doc_table_insert(t, parent, index, rows, columns, header, out);
    if (!out) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
    result = doc_table_insert(&shadow, parent, index, rows, columns, header, &created);
    result = doc_markdown_shadow_end(t, &shadow, result, NULL, NULL);
    if (result == XUI_OK) *out = created;
    return result;
}
typedef int (*doc_table_command_proc)(xui_document_transaction, uint64_t, uint32_t);
static int doc_table_command(xui_document_transaction t, uint64_t table, uint32_t index, doc_table_command_proc command)
{
    struct xui_doc_transaction_t shadow; int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (t->draft->profile != XUI_DOCUMENT_MARKDOWN || t->parsing) return command(t, table, index);
    result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
    result = command(&shadow, table, index);
    if (result == XUI_OK) result = doc_table_markdown_normalize(&shadow, table);
    if (command == doc_table_insert_row || command == doc_table_delete_row)
        return doc_markdown_shadow_end_table_row(t, &shadow, result, table,
            index, command == doc_table_insert_row);
    if (command == doc_table_insert_column || command == doc_table_delete_column)
        return doc_markdown_shadow_end_table_column(t, &shadow, result, table,
            index, command == doc_table_insert_column);
    return doc_markdown_shadow_end(t, &shadow, result, NULL, NULL);
}
XUI_API int xuiDocumentTxnInsertTableRow(xui_document_transaction t, uint64_t table, uint32_t row) { return doc_table_command(t, table, row, doc_table_insert_row); }
XUI_API int xuiDocumentTxnDeleteTableRow(xui_document_transaction t, uint64_t table, uint32_t row) { return doc_table_command(t, table, row, doc_table_delete_row); }
XUI_API int xuiDocumentTxnInsertTableColumn(xui_document_transaction t, uint64_t table, uint32_t column) { return doc_table_command(t, table, column, doc_table_insert_column); }
XUI_API int xuiDocumentTxnDeleteTableColumn(xui_document_transaction t, uint64_t table, uint32_t column) { return doc_table_command(t, table, column, doc_table_delete_column); }

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
    flags = n->attrs->iFlags; row = doc_index_get(t->draft->index, n->parent); table = row->parent;
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

typedef struct doc_table_matrix_field {
    uint64_t start, end;
    int quoted, separator;
} doc_table_matrix_field;
enum { DOC_MATRIX_END, DOC_MATRIX_TAB, DOC_MATRIX_ROW };
/* Spreadsheet TSV uses CSV-style double quoting for fields containing tabs,
 * newlines or quotes. Only a quote at the beginning of a field is structural. */
static int doc_table_matrix_next(const char* text, uint64_t bytes,
    uint64_t* cursor, doc_table_matrix_field* field)
{
    uint64_t i = *cursor;
    memset(field, 0, sizeof(*field));
    if (i < bytes && text[i] == '"') {
        field->quoted = 1; field->start = ++i;
        for (;;) {
            if (i >= bytes) return XUI_DOC_ERROR_FORMAT;
            if (text[i] != '"') { i++; continue; }
            if (i + 1 < bytes && text[i + 1] == '"') { i += 2; continue; }
            field->end = i++;
            if (i < bytes && text[i] != '\t' && text[i] != '\n' && text[i] != '\r')
                return XUI_DOC_ERROR_FORMAT;
            break;
        }
    } else {
        field->start = i;
        while (i < bytes && text[i] != '\t' && text[i] != '\n' && text[i] != '\r') i++;
        field->end = i;
    }
    if (i < bytes && text[i] == '\t') { field->separator = DOC_MATRIX_TAB; i++; }
    else if (i < bytes) {
        field->separator = DOC_MATRIX_ROW;
        if (text[i] == '\r' && i + 1 < bytes && text[i + 1] == '\n') i += 2;
        else i++;
    }
    *cursor = i; return XUI_OK;
}
static int doc_table_matrix_shape(const char* text, uint64_t bytes,
    uint32_t* rows, uint32_t* columns, int* multiline)
{
    uint64_t cursor = 0; uint32_t height = 1, width = 0, widest = 0;
    int result; doc_table_matrix_field field;
    *multiline = 0;
    for (;;) {
        result = doc_table_matrix_next(text, bytes, &cursor, &field);
        if (result != XUI_OK) return result;
        if (width == 1024) return XUI_DOC_ERROR_LIMIT;
        width++;
        if (field.quoted && (memchr(text + field.start, '\r', (size_t)(field.end - field.start)) ||
            memchr(text + field.start, '\n', (size_t)(field.end - field.start)))) *multiline = 1;
        if (field.separator == DOC_MATRIX_TAB) continue;
        if (width > widest) widest = width;
        if (field.separator != DOC_MATRIX_ROW || cursor == bytes) break;
        if (height == UINT32_MAX) return XUI_DOC_ERROR_LIMIT;
        height++; width = 0;
    }
    *rows = height; *columns = widest; return XUI_OK;
}
static int doc_table_matrix_value(xui_document_transaction t, const char* text,
    const doc_table_matrix_field* field, const char** value, uint64_t* bytes, char** owned)
{
    uint64_t i, size = 0, raw = field->end - field->start;
    *value = text + field->start; *bytes = raw; *owned = NULL;
    if (!field->quoted) return XUI_OK;
    if (raw >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    *owned = doc_alloc(t->document->allocator, (size_t)raw + 1);
    if (!*owned) return XUI_ERROR_OUT_OF_MEMORY;
    for (i = field->start; i < field->end; i++) {
        (*owned)[size++] = text[i];
        if (text[i] == '"') i++;
    }
    (*owned)[size] = 0; *value = *owned; *bytes = size; return XUI_OK;
}
static int doc_table_matrix_set_cell(xui_document_transaction t, uint64_t cell,
    const char* text, uint64_t bytes)
{
    xui_doc_node_desc_t desc = {0}; doc_node* node; uint64_t paragraph, id;
    int result;
    while ((node = doc_index_get(t->draft->index, cell)) && doc_seq_size(node->children)) {
        result = doc_txn_delete(t, doc_seq_get_id(node->children, 0));
        if (result != XUI_OK) return result;
    }
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_PARAGRAPH;
    result = doc_txn_insert(t, cell, DOC_NONE, &desc, &paragraph);
    if (result != XUI_OK) return result;
    desc.iKind = XUI_DOC_TEXT; desc.sText = text; desc.iTextBytes = bytes;
    return doc_txn_insert(t, paragraph, DOC_NONE, &desc, &id);
}
static int doc_table_matrix_apply(xui_document_transaction t, uint64_t table,
    uint32_t row, uint32_t column, const char* text, uint64_t bytes,
    uint32_t rows, uint32_t columns, uint64_t* last_cell)
{
    doc_grid grid; uint64_t *targets = NULL, count, i, cursor = 0, source_at = 0;
    uint32_t r = 0, c = 0, target_rows, target_columns; int result;
    result = doc_table_grid(t, table, &grid); if (result != XUI_OK) return result;
    if (row >= grid.rows || column >= grid.columns ||
        rows > UINT32_MAX - row || columns > 1024 - column) {
        result = XUI_ERROR_INVALID_ARGUMENT; goto done;
    }
    target_rows = row + rows; target_columns = column + columns;
    for (i = grid.columns; i < target_columns && result == XUI_OK; i++)
        result = doc_table_insert_column(t, table, (uint32_t)i);
    for (i = grid.rows; i < target_rows && result == XUI_OK; i++)
        result = doc_table_insert_row(t, table, (uint32_t)i);
    doc_free(grid.cells); grid.cells = NULL;
    if (result != XUI_OK) goto done;
    result = doc_table_grid(t, table, &grid); if (result != XUI_OK) goto done;
    count = (uint64_t)rows * columns;
    if (count > SIZE_MAX / sizeof(*targets)) { result = XUI_DOC_ERROR_LIMIT; goto done; }
    targets = doc_alloc(t->document->allocator, (size_t)count * sizeof(*targets));
    if (!targets) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    for (r = 0; r < rows && result == XUI_OK; r++) for (c = 0; c < columns; c++) {
        uint32_t rr = row + r, cc = column + c; doc_grid_cell* cell;
        while (cursor < grid.count && (grid.cells[cursor].row < rr ||
            (grid.cells[cursor].row == rr && grid.cells[cursor].column < cc))) cursor++;
        if (cursor == grid.count) { result = XUI_DOC_ERROR_SCHEMA; break; }
        cell = &grid.cells[cursor];
        if (cell->row != rr || cell->column != cc || cell->rows != 1 || cell->columns != 1) {
            result = XUI_DOC_ERROR_SCHEMA; break;
        }
        targets[(uint64_t)r * columns + c] = cell->id;
    }
    if (result != XUI_OK) goto done;
    *last_cell = targets[count - 1]; r = c = 0;
    for (;;) {
        doc_table_matrix_field field; const char* value; char* owned;
        uint64_t value_bytes;
        result = doc_table_matrix_next(text, bytes, &source_at, &field);
        if (result != XUI_OK) break;
        result = doc_table_matrix_value(t, text, &field, &value, &value_bytes, &owned);
        if (result != XUI_OK) break;
        result = doc_table_matrix_set_cell(t, targets[(uint64_t)r * columns + c],
            value, value_bytes);
        doc_free(owned); if (result != XUI_OK) break;
        c++;
        if (field.separator == DOC_MATRIX_TAB) continue;
        while (c < columns && result == XUI_OK)
            result = doc_table_matrix_set_cell(t,
                targets[(uint64_t)r * columns + c++], "", 0);
        if (result != XUI_OK || field.separator != DOC_MATRIX_ROW || source_at == bytes) break;
        r++; c = 0;
    }
done:
    doc_free(targets); doc_free(grid.cells);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnPasteTableMatrix(xui_document_transaction t,
    xui_doc_node_id table, uint32_t row, uint32_t column, const char* utf8,
    uint64_t bytes, xui_doc_node_id* last_cell)
{
    struct xui_doc_transaction_t shadow; uint32_t rows, columns; int multiline;
    uint64_t target = 0; int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (last_cell) *last_cell = 0;
    if (result != XUI_OK) return result;
    if (!last_cell || (!utf8 && bytes)) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (bytes > t->document->max_bytes || bytes > SIZE_MAX)
        return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    if (!doc_utf8(utf8, bytes)) return doc_txn_fail(t, XUI_DOC_ERROR_UTF8);
    utf8 = utf8 ? utf8 : "";
    result = doc_table_matrix_shape(utf8, bytes, &rows, &columns, &multiline);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (multiline && t->draft->profile == XUI_DOCUMENT_MARKDOWN)
        return doc_txn_fail(t, XUI_DOC_ERROR_UNREPRESENTABLE);
    if (t->draft->profile != XUI_DOCUMENT_MARKDOWN || t->parsing)
        result = doc_table_matrix_apply(t, table, row, column, utf8, bytes,
            rows, columns, &target);
    else {
        result = doc_markdown_shadow_begin(t, &shadow);
        if (result != XUI_OK) return result;
        result = doc_table_matrix_apply(&shadow, table, row, column, utf8, bytes,
            rows, columns, &target);
        if (result == XUI_OK) result = doc_table_markdown_normalize(&shadow, table);
        result = doc_markdown_shadow_end(t, &shadow, result, NULL, NULL);
    }
    if (result == XUI_OK) *last_cell = target;
    return result;
}
static int doc_table_matrix_clear_apply(xui_document_transaction t, uint64_t table,
    uint32_t row, uint32_t column, uint32_t rows, uint32_t columns,
    uint64_t* first_cell)
{
    doc_grid grid; uint64_t i; doc_table_cell_slot first; int result;
    result = doc_table_grid(t, table, &grid); if (result != XUI_OK) return result;
    if (!rows || !columns || row >= grid.rows || rows > grid.rows - row ||
        column >= grid.columns || columns > grid.columns - column) {
        result = XUI_ERROR_INVALID_ARGUMENT; goto done;
    }
    result = doc_table_cell_at(t->draft, table, row, column, &first);
    if (result != XUI_OK) goto done;
    for (i = 0; i < grid.count; i++) {
        const doc_grid_cell* cell = &grid.cells[i];
        if (cell->row >= row + rows || cell->row + cell->rows <= row ||
            cell->column >= column + columns || cell->column + cell->columns <= column)
            continue;
        if (cell->row < row || cell->column < column ||
            cell->row + cell->rows > row + rows ||
            cell->column + cell->columns > column + columns) {
            result = XUI_DOC_ERROR_SCHEMA; goto done;
        }
    }
    for (i = 0; i < grid.count && result == XUI_OK; i++) {
        const doc_grid_cell* cell = &grid.cells[i];
        if (cell->row >= row && cell->row < row + rows &&
            cell->column >= column && cell->column < column + columns)
            result = doc_table_matrix_set_cell(t, cell->id, "", 0);
    }
    if (result == XUI_OK) *first_cell = first.cell;
done:
    doc_free(grid.cells);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnClearTableMatrix(xui_document_transaction t,
    xui_doc_node_id table, uint32_t row, uint32_t column, uint32_t rows,
    uint32_t columns, xui_doc_node_id* first_cell)
{
    struct xui_doc_transaction_t shadow; uint64_t target = 0;
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (first_cell) *first_cell = 0;
    if (result != XUI_OK) return result;
    if (!first_cell) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile != XUI_DOCUMENT_MARKDOWN || t->parsing)
        result = doc_table_matrix_clear_apply(t, table, row, column,
            rows, columns, &target);
    else {
        result = doc_markdown_shadow_begin(t, &shadow);
        if (result != XUI_OK) return result;
        result = doc_table_matrix_clear_apply(&shadow, table, row, column,
            rows, columns, &target);
        if (result == XUI_OK) result = doc_table_markdown_normalize(&shadow, table);
        result = doc_markdown_shadow_end(t, &shadow, result, NULL, NULL);
    }
    if (result == XUI_OK) *first_cell = target;
    return result;
}

typedef struct doc_table_copy_buffer { char* data; size_t length, capacity; } doc_table_copy_buffer;
static int doc_table_copy_reserve(doc_table_copy_buffer* b, size_t bytes)
{
    size_t wanted, capacity; char* grown;
    if (bytes > SIZE_MAX - b->length - 1) return XUI_DOC_ERROR_LIMIT;
    wanted = b->length + bytes + 1;
    if (wanted <= b->capacity) return XUI_OK;
    capacity = b->capacity ? b->capacity : 64;
    while (capacity < wanted) {
        if (capacity > SIZE_MAX / 2) { capacity = wanted; break; }
        capacity *= 2;
    }
    grown = realloc(b->data, capacity); if (!grown) return XUI_ERROR_OUT_OF_MEMORY;
    b->data = grown; b->capacity = capacity; return XUI_OK;
}
static int doc_table_copy_append(doc_table_copy_buffer* b, const char* text, size_t bytes)
{
    int result = doc_table_copy_reserve(b, bytes);
    if (result != XUI_OK) return result;
    if (bytes) memcpy(b->data + b->length, text, bytes);
    b->length += bytes; b->data[b->length] = 0; return XUI_OK;
}
static int doc_table_copy_plain_node(doc_state* state, uint64_t id,
    int last_child, doc_table_copy_buffer* b)
{
    doc_node* node = doc_index_get(state->index, id); uint64_t i, size;
    int result;
    if (!node) return XUI_DOC_ERROR_SCHEMA;
    if (doc_text_kind(node->kind)) {
        size = doc_seq_size(node->text);
        if (size > SIZE_MAX - b->length - 1) return XUI_DOC_ERROR_LIMIT;
        result = doc_table_copy_reserve(b, (size_t)size); if (result != XUI_OK) return result;
        result = doc_seq_read(node->text, 0, b->data + b->length, size);
        if (result != XUI_OK) return result;
        b->length += (size_t)size; b->data[b->length] = 0;
    }
    size = doc_seq_size(node->children);
    for (i = 0; i < size; i++) {
        result = doc_table_copy_plain_node(state, doc_seq_get_id(node->children, i),
            i + 1 == size, b);
        if (result != XUI_OK) return result;
    }
    if (node->kind == XUI_DOC_SOFT_BREAK || node->kind == XUI_DOC_HARD_BREAK)
        return doc_table_copy_append(b, "\n", 1);
    if (node->kind == XUI_DOC_CELL && !last_child)
        return doc_table_copy_append(b, "\t", 1);
    if (node->kind == XUI_DOC_PARAGRAPH || node->kind == XUI_DOC_HEADING ||
        node->kind == XUI_DOC_CODE_BLOCK || node->kind == XUI_DOC_ROW) {
        doc_node* parent = doc_index_get(state->index, node->parent);
        if (!(last_child && parent && parent->kind == XUI_DOC_CELL))
            return doc_table_copy_append(b, "\n", 1);
    }
    return XUI_OK;
}
static int doc_table_copy_cell(doc_state* state, uint64_t cell,
    doc_table_copy_buffer* scratch, doc_table_copy_buffer* output)
{
    doc_node* node = doc_index_get(state->index, cell); uint64_t i, count;
    size_t at, quotes = 0; int quote = 0, result;
    if (!node || node->kind != XUI_DOC_CELL) return XUI_DOC_ERROR_SCHEMA;
    scratch->length = 0;
    count = doc_seq_size(node->children);
    for (i = 0; i < count; i++) {
        result = doc_table_copy_plain_node(state, doc_seq_get_id(node->children, i),
            i + 1 == count, scratch);
        if (result != XUI_OK) return result;
    }
    for (at = 0; at < scratch->length; at++) {
        if (scratch->data[at] == '"') { quote = 1; quotes++; }
        else if (scratch->data[at] == '\t' || scratch->data[at] == '\n' ||
            scratch->data[at] == '\r') quote = 1;
    }
    if (!quote) return doc_table_copy_append(output, scratch->data, scratch->length);
    if (scratch->length > SIZE_MAX - quotes - 2) return XUI_DOC_ERROR_LIMIT;
    result = doc_table_copy_reserve(output, scratch->length + quotes + 2);
    if (result != XUI_OK) return result;
    output->data[output->length++] = '"';
    for (at = 0; at < scratch->length; at++) {
        output->data[output->length++] = scratch->data[at];
        if (scratch->data[at] == '"') output->data[output->length++] = '"';
    }
    output->data[output->length++] = '"'; output->data[output->length] = 0;
    return XUI_OK;
}
XUI_API int xuiDocumentSnapshotCopyTableMatrix(xui_document_snapshot snapshot,
    xui_doc_node_id table, uint32_t row, uint32_t column, uint32_t rows,
    uint32_t columns, char** out, uint64_t* bytes)
{
    doc_grid grid; const doc_grid_cell* active[1024] = {0};
    doc_table_copy_buffer scratch = {0}, output = {0};
    uint64_t next = 0; uint32_t r, c; int result;
    if (out) *out = NULL;
    if (bytes) *bytes = 0;
    if (!snapshot || !out || !bytes || !rows || !columns) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_table_grid_state(snapshot->state, table, &grid);
    if (result != XUI_OK) return result;
    if (row >= grid.rows || rows > grid.rows - row || column >= grid.columns ||
        columns > grid.columns - column) { result = XUI_ERROR_INVALID_ARGUMENT; goto done; }
    for (; next < grid.count && grid.cells[next].row < row; next++) {
        const doc_grid_cell* cell = &grid.cells[next];
        if (cell->row + cell->rows > row)
            for (c = cell->column; c < cell->column + cell->columns; c++) active[c] = cell;
    }
    for (r = row; r < row + rows && result == XUI_OK; r++) {
        for (c = 0; c < grid.columns; c++)
            if (active[c] && active[c]->row + active[c]->rows <= r) active[c] = NULL;
        while (next < grid.count && grid.cells[next].row == r) {
            const doc_grid_cell* cell = &grid.cells[next++];
            for (c = cell->column; c < cell->column + cell->columns; c++) active[c] = cell;
        }
        for (c = column; c < column + columns && result == XUI_OK; c++) {
            const doc_grid_cell* cell = active[c];
            if (!cell || cell->row < row || cell->column < column ||
                cell->row + cell->rows > row + rows ||
                cell->column + cell->columns > column + columns) { result = XUI_DOC_ERROR_SCHEMA; break; }
            if (c > column) result = doc_table_copy_append(&output, "\t", 1);
            if (result == XUI_OK && cell->row == r && cell->column == c)
                result = doc_table_copy_cell(snapshot->state, cell->id, &scratch, &output);
        }
        if (result == XUI_OK && r + 1 < row + rows)
            result = doc_table_copy_append(&output, "\n", 1);
    }
    if (result == XUI_OK) {
        result = doc_table_copy_reserve(&output, 0);
        if (result == XUI_OK) {
            *bytes = output.length; *out = output.data; output.data = NULL;
        }
    }
done:
    free(output.data); free(scratch.data); doc_free(grid.cells);
    return result;
}
int doc_table_expand_selection(doc_state* state, uint64_t table, uint32_t* row,
    uint32_t* column, uint32_t* rows, uint32_t* columns)
{
    doc_grid grid; uint32_t first_row, first_column, end_row, end_column;
    uint64_t i; int changed, result;
    if (!state || !row || !column || !rows || !columns || !*rows || !*columns)
        return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_table_grid_state(state, table, &grid);
    if (result != XUI_OK) return result;
    if (*row >= grid.rows || *rows > grid.rows - *row || *column >= grid.columns ||
        *columns > grid.columns - *column) {
        doc_free(grid.cells); return XUI_ERROR_INVALID_ARGUMENT;
    }
    first_row = *row; first_column = *column;
    end_row = first_row + *rows; end_column = first_column + *columns;
    do {
        changed = 0;
        for (i = 0; i < grid.count; i++) {
            const doc_grid_cell* cell = &grid.cells[i];
            if (cell->row >= end_row || cell->row + cell->rows <= first_row ||
                cell->column >= end_column || cell->column + cell->columns <= first_column)
                continue;
            if (cell->row < first_row) { first_row = cell->row; changed = 1; }
            if (cell->column < first_column) { first_column = cell->column; changed = 1; }
            if (cell->row + cell->rows > end_row) { end_row = cell->row + cell->rows; changed = 1; }
            if (cell->column + cell->columns > end_column) {
                end_column = cell->column + cell->columns; changed = 1;
            }
        }
    } while (changed);
    *row = first_row; *column = first_column;
    *rows = end_row - first_row; *columns = end_column - first_column;
    doc_free(grid.cells); return XUI_OK;
}

#endif
