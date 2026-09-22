#include "xui_document_internal.h"

int doc_attributes_equal(const xui_doc_attributes_t* a, const xui_doc_attributes_t* b)
{
    return a->iMarks == b->iMarks && a->iFlags == b->iFlags && a->iHeadingLevel == b->iHeadingLevel &&
        a->iAlignment == b->iAlignment && a->iRowSpan == b->iRowSpan && a->iColumnSpan == b->iColumnSpan &&
        a->iListStart == b->iListStart && a->iTextColor == b->iTextColor && a->iBackgroundColor == b->iBackgroundColor &&
        a->fFontSize == b->fFontSize && a->fWidth == b->fWidth && a->fHeight == b->fHeight &&
        a->fParagraphSpacing == b->fParagraphSpacing && !strcmp(a->sFontFamily, b->sFontFamily);
}

/* A table's cells cover a rectangular grid. Spans occupy following rows;
 * cells are stored once, at their upper-left position. */
static int doc_validate_table(doc_state* s, doc_node* table)
{
    uint16_t occupied[1024] = {0};
    uint64_t row_index, row_count = doc_seq_size(table->children);
    unsigned width = 0;
    for (row_index = 0; row_index < row_count; row_index++) {
        doc_node* row = doc_index_get(s->index, doc_seq_get_id(table->children, row_index));
        uint64_t cell_index;
        unsigned column = 0, row_width = 0, k;
        if (!row || row->kind != XUI_DOC_ROW) return XUI_DOC_ERROR_SCHEMA;
        for (cell_index = 0; cell_index < doc_seq_size(row->children); cell_index++) {
            doc_node* cell = doc_index_get(s->index, doc_seq_get_id(row->children, cell_index));
            unsigned w, h;
            if (!cell || cell->kind != XUI_DOC_CELL) return XUI_DOC_ERROR_SCHEMA;
            w = cell->attrs.iColumnSpan; h = cell->attrs.iRowSpan;
            if (!w || !h || h > row_count - row_index) return XUI_DOC_ERROR_SCHEMA;
            while (column < 1024 && occupied[column]) column++;
            if (w > 1024 - column) return XUI_DOC_ERROR_LIMIT;
            for (k = 0; k < w; k++) {
                if (occupied[column + k]) return XUI_DOC_ERROR_SCHEMA;
                occupied[column + k] = (uint16_t)h;
            }
            column += w;
        }
        for (k = 0; k < 1024; k++) if (occupied[k]) row_width = k + 1;
        if (!row_width) return XUI_DOC_ERROR_SCHEMA;
        if (!row_index) width = row_width;
        if (row_width != width) return XUI_DOC_ERROR_SCHEMA;
        for (k = 0; k < width; k++) {
            if (!occupied[k]) return XUI_DOC_ERROR_SCHEMA;
            occupied[k]--;
        }
    }
    return XUI_OK;
}
static int doc_validate_tree(doc_state* s, uint64_t id, uint64_t parent,
    unsigned depth, uint64_t* count, uint64_t* visited)
{
    doc_node* n = doc_index_get(s->index, id);
    uint64_t i;
    int result;
    if (++*count > s->node_count || depth > DOC_MAX_DEPTH) return XUI_DOC_ERROR_SCHEMA;
    visited[*count - 1] = id;
    if (!n || n->parent != parent || !doc_schema_attrs(n->kind, &n->attrs) ||
        (!doc_text_kind(n->kind) && n->text)) return XUI_DOC_ERROR_SCHEMA;
    if (n->kind == XUI_DOC_TABLE && (result = doc_validate_table(s, n)) != XUI_OK) return result;
    for (i = 0; i < doc_seq_size(n->children); i++) {
        uint64_t child_id = doc_seq_get_id(n->children, i);
        doc_node* child = doc_index_get(s->index, child_id);
        if (!child || !doc_schema_child(n->kind, child->kind)) return XUI_DOC_ERROR_SCHEMA;
        result = doc_validate_tree(s, child_id, id, depth + 1, count, visited);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
static int doc_schema_id_order(const void* a, const void* b)
{
    uint64_t x = *(const uint64_t*)a, y = *(const uint64_t*)b; return x < y ? -1 : x != y;
}
int doc_schema_validate(doc_state* s)
{
    uint64_t count = 0, i, *visited;
    doc_node* root = doc_index_get(s->index, DOC_ROOT);
    int result;
    if (!root || root->kind != XUI_DOC_ROOT) return XUI_DOC_ERROR_SCHEMA;
    if (s->node_count > SIZE_MAX / sizeof(*visited)) return XUI_DOC_ERROR_LIMIT;
    visited = doc_alloc(s->allocator, (size_t)s->node_count * sizeof(*visited));
    if (!visited) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_validate_tree(s, DOC_ROOT, 0, 1, &count, visited);
    if (result == XUI_OK && count != s->node_count) result = XUI_DOC_ERROR_SCHEMA;
    if (result == XUI_OK) {
        qsort(visited, (size_t)count, sizeof(*visited), doc_schema_id_order);
        for (i = 1; i < count; i++) if (visited[i] == visited[i - 1]) { result = XUI_DOC_ERROR_SCHEMA; break; }
    }
    doc_free(visited); return result;
}
int doc_schema_validate_transaction(xui_document_transaction t)
{
    uint64_t i, last_table = 0;
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN) return XUI_OK;
    /* Local text edits do not scan the tree. Only touched tables need a grid
     * check; temporary ragged rows are allowed inside a transaction. */
    for (i = 0; i < t->count; i++) {
        const xui_doc_operation_t* op = &t->ops[i];
        uint64_t candidates[3] = {op->iNodeId, op->iParentId, op->iOtherNodeId};
        unsigned k;
        if (!(op->iFlags & (XUI_DOC_CHANGE_STRUCTURE | XUI_DOC_CHANGE_STYLE))) continue;
        for (k = 0; k < 3; k++) {
            doc_node* n = doc_index_get(t->draft->index, candidates[k]);
            for (; n; n = doc_index_get(t->draft->index, n->parent)) {
                if (n->kind == XUI_DOC_TABLE) {
                    int result;
                    if (last_table == n->id) break;
                    last_table = n->id;
                    result = doc_validate_table(t->draft, n);
                    if (result != XUI_OK) return result;
                }
            }
        }
    }
    return XUI_OK;
}
int doc_schema_depth(doc_state* s, uint64_t id, unsigned depth)
{
    doc_node* n = doc_index_get(s->index, id);
    uint64_t i;
    if (!n || depth > DOC_MAX_DEPTH) return XUI_DOC_ERROR_LIMIT;
    for (i = 0; i < doc_seq_size(n->children); i++) {
        int result = doc_schema_depth(s, doc_seq_get_id(n->children, i), depth + 1);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
