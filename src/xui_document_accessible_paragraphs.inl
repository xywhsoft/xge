/* Paragraph boundaries are a disposable View index over its existing text
 * projection. Semantic paragraphs preserve explicit empty nodes; raw source
 * paragraphs keep internal line breaks and attach following blank lines. */
static int doc_text_paragraph_edge(uint64_t* edges, uint64_t capacity,
    uint64_t* count, uint64_t offset)
{
    if (*count == UINT64_MAX) return XUI_DOC_ERROR_LIMIT;
    if (edges) {
        if (*count >= capacity) return XUI_ERROR_INVALID_STATE;
        edges[*count] = offset;
    }
    (*count)++; return XUI_OK;
}
static int doc_text_paragraph_owner(const doc_plain_projection* projection,
    const doc_plain_span* span, uint64_t* owner)
{
    doc_node* node = doc_index_get(projection->state->index, span->first.iNodeId);
    if (!node) return XUI_ERROR_INVALID_STATE;
    *owner = node->id;
    while (node && node->id != DOC_ROOT) {
        if (node->kind == XUI_DOC_PARAGRAPH || node->kind == XUI_DOC_HEADING || node->kind == XUI_DOC_CODE_BLOCK) {
            *owner = node->id; break;
        }
        node = doc_index_get(projection->state->index, node->parent);
    }
    return XUI_OK;
}
static int doc_text_paragraph_fill(const doc_plain_projection* projection,
    uint64_t* edges, uint64_t capacity, uint64_t* count)
{
    uint64_t i; int result;
    *count = 0; result = doc_text_paragraph_edge(edges, capacity, count, 0);
    if (result != XUI_OK || !projection->bytes) return result;
    if (projection->origin.iKind == XUI_DOC_POSITION_SOURCE) {
        uint64_t cursor = 0; int previous_blank = 0;
        while (cursor < projection->bytes) {
            uint64_t start = cursor; int blank = 1;
            while (cursor < projection->bytes && projection->text[cursor] != '\r' && projection->text[cursor] != '\n') {
                if (projection->text[cursor] != ' ' && projection->text[cursor] != '\t') blank = 0;
                cursor++;
            }
            if (cursor < projection->bytes) {
                if (projection->text[cursor++] == '\r' && cursor < projection->bytes && projection->text[cursor] == '\n') cursor++;
            }
            if (!blank && previous_blank && start) {
                result = doc_text_paragraph_edge(edges, capacity, count, start);
                if (result != XUI_OK) return result;
            }
            previous_blank = blank;
        }
    } else {
        uint64_t previous_owner = 0;
        for (i = 0; i < projection->count; i++) {
            const doc_plain_span* span = &projection->spans[i]; uint64_t owner;
            result = doc_text_paragraph_owner(projection, span, &owner);
            if (result != XUI_OK) return result;
            if (previous_owner && owner != previous_owner && span->start) {
                result = doc_text_paragraph_edge(edges, capacity, count, span->start);
                if (result != XUI_OK) return result;
            }
            previous_owner = owner;
        }
    }
    return doc_text_paragraph_edge(edges, capacity, count, projection->bytes);
}
static int doc_text_paragraph_cache(doc_view_data* view)
{
    uint64_t count, filled; uint64_t* edges; int result;
    if (view->text_paragraph_edges) return XUI_OK;
    result = doc_text_paragraph_fill(&view->edit_projection, NULL, 0, &count);
    if (result != XUI_OK) return result;
    if (count > SIZE_MAX / sizeof(*edges)) return XUI_DOC_ERROR_LIMIT;
    edges = doc_alloc(view->edit_projection.allocator, (size_t)count * sizeof(*edges));
    if (!edges) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_text_paragraph_fill(&view->edit_projection, edges, count, &filled);
    if (result != XUI_OK || filled != count) {
        doc_free(edges); return result == XUI_OK ? XUI_ERROR_INVALID_STATE : result;
    }
    view->text_paragraph_edges = edges; view->text_paragraph_edge_count = count;
    return XUI_OK;
}
static int doc_text_paragraph_span(doc_view_data* view, uint64_t at, uint64_t* first, uint64_t* last)
{
    uint64_t lo = 0, hi; int result;
    if (!view->edit_projection.bytes) { *first = *last = 0; return XUI_OK; }
    result = doc_text_paragraph_cache(view); if (result != XUI_OK) return result;
    if (at == view->edit_projection.bytes) at--;
    hi = view->text_paragraph_edge_count;
    while (lo < hi) {
        uint64_t mid = lo + (hi - lo) / 2;
        if (view->text_paragraph_edges[mid] <= at) lo = mid + 1; else hi = mid;
    }
    if (!lo || lo >= view->text_paragraph_edge_count) return XUI_ERROR_INVALID_STATE;
    *first = view->text_paragraph_edges[lo - 1]; *last = view->text_paragraph_edges[lo];
    return XUI_OK;
}
