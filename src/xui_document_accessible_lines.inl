/* A line is a renderer row. Plaintext separators are assigned to their
 * preceding row only when that insertion point still belongs to the row. */
static uint64_t doc_text_scalar_before(const doc_plain_projection* p, uint64_t at)
{
    if (!at) return 0;
    at--;
    while (at && ((unsigned char)p->text[at] & 0xc0) == 0x80) at--;
    return at;
}
static int doc_text_same_row(const xui_doc_range_t* a, const xui_doc_range_t* b)
{
    return a->tAnchor.iNodeId == b->tAnchor.iNodeId &&
        a->tAnchor.iKind == b->tAnchor.iKind && a->tAnchor.iOffset == b->tAnchor.iOffset &&
        a->tCaret.iNodeId == b->tCaret.iNodeId &&
        a->tCaret.iKind == b->tCaret.iKind && a->tCaret.iOffset == b->tCaret.iOffset;
}
static int doc_text_live_source_active(const doc_view_data* view, uint64_t at)
{
    xui_document_renderer r = view->renderer;
    return r->live_fallback || !r->live_rows || (at >= r->live_start &&
        (at < r->live_end || (at == r->live_end && at == view->edit_projection.bytes)));
}
static xui_doc_position_t doc_text_semantic_edge(doc_view_data* view, uint64_t id, int end)
{
    doc_state* state = view->edit_projection.state;
    doc_node* n = doc_index_get(state->index, id);
    xui_doc_position_t p = view->edit_projection.origin;
    p.iInputGeneration = 0; /* The semantic edge belongs to the base snapshot. */
    while (doc_seq_size(n->children)) n = doc_index_get(state->index,
        doc_seq_get_id(n->children, end ? doc_seq_size(n->children) - 1 : 0));
    p.iNodeId = n->id; p.iKind = doc_text_kind(n->kind) ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
    p.iOffset = end ? doc_seq_size(n->text) : 0; p.iAffinity = XUI_DOC_AFTER;
    if (n->kind == XUI_DOC_SOFT_BREAK || n->kind == XUI_DOC_HARD_BREAK || doc_selectable_object_kind(n->kind)) {
        doc_node* parent = doc_index_get(state->index, n->parent);
        p.iNodeId = parent->id; p.iKind = XUI_DOC_POSITION_GAP;
        p.iOffset = doc_child_index(parent, n->id) + (end ? 1 : 0);
    }
    return p;
}
static int doc_text_row_query(doc_view_data* view, uint64_t at, xui_doc_range_t* row)
{
    xui_document_renderer r = view->renderer;
    xui_doc_position_t p = doc_plain_unproject(&view->edit_projection, at, 1);
    int result, mapping;
    /* Querying bounds must not ask for the X of a far-away horizontal caret. */
    if (p.iKind == XUI_DOC_POSITION_SOURCE && r->mode == XUI_DOC_LIVE_MARKDOWN &&
        !doc_text_live_source_active(view, at)) {
        uint64_t owner, first, last, base = at; doc_node* n;
        result = doc_render_live_partition(r, at, &owner, &first, &last);
        if (result != XUI_OK) return result;
        if (r->input) {
            result = doc_render_live_input_to_base(r, at, &base);
            if (result != XUI_OK) return result;
        }
        result = xuiDocumentSourceToPositionEx(r->snapshot, base, XUI_DOC_AFTER, &p, &mapping);
        if (result != XUI_OK) return result;
        for (n = doc_index_get(view->edit_projection.state->index, p.iNodeId);
            n && n->id != owner; n = doc_index_get(view->edit_projection.state->index, n->parent)) {}
        if (!n) {
            doc_node_source_range bounds;
            doc_node_source_range_get(view->edit_projection.state,
                doc_index_get(view->edit_projection.state->index, owner), &bounds);
            p = doc_text_semantic_edge(view, owner, base >= bounds.source_end);
        }
    }
    return xuiDocumentRendererGetLineRange(r, &p, row);
}
static int doc_text_row_source_edge(doc_view_data* view, const xui_doc_range_t* row,
    uint64_t edge, int end, uint64_t* first, uint64_t* last)
{
    doc_plain_projection* p = &view->edit_projection; xui_doc_range_t candidate;
    uint64_t at; int result;
    if (edge > p->bytes || (end ? edge <= *last : edge >= *first)) return XUI_OK;
    if (view->renderer->mode == XUI_DOC_LIVE_MARKDOWN &&
        !doc_text_live_source_active(view, *first)) {
        uint64_t owner, start, stop;
        result = doc_render_live_partition(view->renderer, *first, &owner, &start, &stop);
        if (result != XUI_OK) return result;
        if (edge < start || edge > stop) return XUI_OK;
    }
    at = end && edge ? doc_text_scalar_before(p, edge) : edge;
    if (at < p->bytes && ((unsigned char)p->text[at] & 0xc0) == 0x80) return XUI_OK;
    result = doc_text_row_query(view, at, &candidate);
    if (result != XUI_OK) return result;
    if (doc_text_same_row(row, &candidate)) {
        if (end) *last = edge; else *first = edge;
    }
    return XUI_OK;
}
static int doc_text_line_span(doc_view_data* view, uint64_t at, uint64_t* first, uint64_t* last)
{
    doc_plain_projection* p = &view->edit_projection;
    xui_doc_range_t row, mapped, candidate; uint64_t i, a, b; int result;
    if (!p->bytes) { *first = *last = 0; return XUI_OK; }
    if (at == p->bytes) at = doc_text_scalar_before(p, at);
    result = doc_text_row_query(view, at, &row);
    if (result != XUI_OK) return result;
    mapped = row; result = doc_view_map_range(view, view->renderer->mode, &mapped);
    if (result == XUI_OK) result = doc_plain_project_position(p, &mapped.tAnchor, &a);
    if (result == XUI_OK) result = doc_plain_project_position(p, &mapped.tCaret, &b);
    if (result != XUI_OK) return result;
    if (a > b) return XUI_ERROR_INVALID_STATE;
    if (p->origin.iKind != XUI_DOC_POSITION_SOURCE) {
        /* A synthetic paragraph/cell separator has structural endpoints. A
         * hard break followed by an empty row must keep those two rows apart. */
        for (i = 0; i < p->count; i++) {
            const doc_plain_span* span = &p->spans[i]; int order;
            if (span->start < b) continue;
            if (span->start > b || span->literal) break;
            result = doc_position_compare(p->state, &span->first, &row.tCaret, &order);
            if (result != XUI_OK) return result;
            if (order) break;
            result = doc_text_row_query(view, b, &candidate);
            if (result != XUI_OK) return result;
            if (doc_text_same_row(&row, &candidate)) b += span->length;
            break;
        }
    } else if (view->renderer->mode == XUI_DOC_LIVE_MARKDOWN &&
        !doc_text_live_source_active(view, at)) {
        xui_doc_position_t semantic; int mapping;
        doc_node* node;
        uint64_t left = a, right = b, base = a;
        if (view->renderer->input) {
            result = doc_render_live_input_to_base(view->renderer, a, &base);
            if (result != XUI_OK) return result;
        }
        result = xuiDocumentSourceToPositionEx(view->renderer->snapshot, base,
            XUI_DOC_AFTER, &semantic, &mapping);
        if (result != XUI_OK) return result;
        node = doc_index_get(p->state->index, semantic.iNodeId);
        /* Physical prefixes/terminators and enclosing syntax belong to this
         * unit only if they project to this same renderer row. Never extend
         * according to the requested offset: membership must be canonical. */
        while (left && p->text[left - 1] != '\n' && p->text[left - 1] != '\r') left--;
        while (right < p->bytes && p->text[right] != '\n' && p->text[right] != '\r') right++;
        if (right < p->bytes) {
            if (p->text[right++] == '\r' && right < p->bytes && p->text[right] == '\n') right++;
        }
        result = doc_text_row_source_edge(view, &row, left, 0, &a, &b);
        if (result == XUI_OK) result = doc_text_row_source_edge(view, &row, right, 1, &a, &b);
        while (result == XUI_OK && node) {
            doc_node_source_range bounds;
            doc_node_source_range_get(p->state, node, &bounds);
            if (view->renderer->input) {
                uint64_t* edges[] = {&bounds.source_start, &bounds.syntax_start,
                    &bounds.source_end, &bounds.syntax_end}; unsigned edge;
                for (edge = 0; edge < 4 && result == XUI_OK; edge++)
                    if (*edges[edge] != DOC_NONE)
                        result = doc_render_live_base_to_input(view->renderer, *edges[edge], edges[edge]);
                if (result != XUI_OK) break;
            }
            result = doc_text_row_source_edge(view, &row, bounds.source_start, 0, &a, &b);
            if (result == XUI_OK) result = doc_text_row_source_edge(view, &row, bounds.syntax_start, 0, &a, &b);
            if (result == XUI_OK) result = doc_text_row_source_edge(view, &row, bounds.source_end, 1, &a, &b);
            if (result == XUI_OK) result = doc_text_row_source_edge(view, &row, bounds.syntax_end, 1, &a, &b);
            node = doc_index_get(p->state->index, node->parent);
        }
        if (result == XUI_OK) {
            uint64_t owner, start, stop;
            result = doc_render_live_partition(view->renderer, at, &owner, &start, &stop);
            if (result == XUI_OK) result = doc_text_row_source_edge(view, &row, start, 0, &a, &b);
            if (result == XUI_OK) result = doc_text_row_source_edge(view, &row, stop, 1, &a, &b);
        }
        if (result != XUI_OK) return result;
    }
    if (a > at || b <= at || b > p->bytes) return XUI_ERROR_INVALID_STATE;
    *first = a; *last = b; return XUI_OK;
}
