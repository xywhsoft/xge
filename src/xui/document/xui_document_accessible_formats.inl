/* Disposable format boundaries over the View's existing text projection.
 * Font selection and inherited attributes are shared with actual layout.
 * LIVE source syntax which has no displayed text retains an explicit hidden
 * format; the active raw-source interval uses the renderer's source font. */
typedef struct doc_text_format_key {
    xui_font font;
    const char* language;
    uint32_t marks, foreground, background, alignment, object_kind, hidden;
} doc_text_format_key;
typedef struct doc_text_format_record {
    uint64_t first, last;
    unsigned priority;
    doc_text_format_key key;
} doc_text_format_record;
typedef struct doc_text_format_event { uint64_t at, record; } doc_text_format_event;
struct doc_format_cache {
    uint64_t count, generation, active_source;
    uint32_t mode;
    doc_state* state; /* Borrowed from the retained edit snapshot. */
    xui_doc_renderer_desc_t descriptor;
    uint64_t edges[];
};
void doc_view_format_cache_clear(doc_view_data* view)
{ doc_free(view->text_formats); view->text_formats = NULL; }
static int doc_text_format_equal(const doc_text_format_key* a, const doc_text_format_key* b)
{
    return a->font == b->font && a->marks == b->marks && a->foreground == b->foreground &&
        a->background == b->background && a->alignment == b->alignment &&
        a->object_kind == b->object_kind && a->hidden == b->hidden && doc_language_equal(a->language, b->language);
}
static int doc_text_format_key_for(doc_view_data* view, doc_node* node, int raw,
    int hidden, doc_text_format_key* key)
{
    xui_document_renderer r = view->renderer; doc_state* state = view->edit_projection.state;
    xui_doc_attributes_t attrs = {0}; doc_node* ancestor; uint32_t heading = 0;
    memset(key, 0, sizeof(*key)); key->hidden = !!hidden;
    if (raw) {
        attrs.iMarks = XUI_DOC_CODE;
        attrs.sLanguage = doc_index_get(state->index, DOC_ROOT)->attrs->sLanguage;
    } else {
        if (!node) return XUI_ERROR_INVALID_STATE;
        attrs = doc_effective_text_attrs(state, node);
        for (ancestor = node; ancestor; ancestor = doc_index_get(state->index, ancestor->parent)) {
            if (ancestor->kind == XUI_DOC_HEADING && !heading) heading = ancestor->attrs->iHeadingLevel;
        }
        if (node->kind == XUI_DOC_CODE_BLOCK || node->kind == XUI_DOC_DIAGRAM || node->kind == XUI_DOC_FRONT_MATTER)
            attrs.iMarks |= XUI_DOC_CODE;
        key->alignment = doc_effective_alignment(state, node);
        if (doc_selectable_object_kind(node->kind)) key->object_kind = node->kind;
    }
    key->font = doc_layout_text_font(r, &attrs, heading);
    if (!key->font) return XUI_ERROR_OUT_OF_MEMORY;
    key->language = attrs.sLanguage;
    key->marks = attrs.iMarks | (heading ? XUI_DOC_BOLD : 0);
    key->foreground = doc_render_text_foreground(r, &attrs);
    key->background = attrs.iFlags & XUI_DOC_BACKGROUND_COLOR_CURRENT ? key->foreground :
        doc_background_color_set(&attrs) ? attrs.iBackgroundColor :
        attrs.iMarks & XUI_DOC_HIGHLIGHT ? r->desc.iHighlightColor : 0;
    return XUI_OK;
}
static int doc_text_format_record_add(doc_text_format_record* records, uint64_t capacity,
    uint64_t* count, uint64_t first, uint64_t last, unsigned priority, const doc_text_format_key* key)
{
    if (first == last) return XUI_OK;
    if (first > last || *count >= capacity) return XUI_ERROR_INVALID_STATE;
    records[*count] = (doc_text_format_record){first, last, priority, *key}; (*count)++; return XUI_OK;
}
static int doc_text_format_live_interval(doc_view_data* view, doc_text_format_record* records,
    uint64_t capacity, uint64_t* count, uint64_t first, uint64_t last, unsigned depth, const doc_text_format_key* key)
{
    xui_document_renderer r = view->renderer; uint64_t a, b, left = r->live_start;
    uint64_t right = r->input ? r->live_base_end : r->live_end; int result;
    if (first == DOC_NONE || last == DOC_NONE || first >= last) return XUI_OK;
    if (first < left) {
        a = first; b = last < left ? last : left;
        if (r->input) {
            result = doc_render_live_base_to_input(r, a, &a);
            if (result == XUI_OK) result = doc_render_live_base_to_input(r, b, &b);
            if (result != XUI_OK) return result;
        }
        result = doc_text_format_record_add(records, capacity, count, a, b, depth, key);
        if (result != XUI_OK) return result;
    }
    if (last > right) {
        a = first > right ? first : right; b = last;
        if (r->input) {
            result = doc_render_live_base_to_input(r, a, &a);
            if (result == XUI_OK) result = doc_render_live_base_to_input(r, b, &b);
            if (result != XUI_OK) return result;
        }
        return doc_text_format_record_add(records, capacity, count, a, b, depth, key);
    }
    return XUI_OK;
}
static int doc_text_format_live_node(doc_view_data* view, uint64_t id, unsigned depth,
    doc_text_format_record* records, uint64_t capacity, uint64_t* count)
{
    doc_state* state = view->edit_projection.state; doc_node* node = doc_index_get(state->index, id);
    uint64_t i; int result;
    if (!node || depth >= DOC_MAX_DEPTH) return XUI_ERROR_INVALID_STATE;
    if (doc_text_kind(node->kind) || node->kind == XUI_DOC_SOFT_BREAK || node->kind == XUI_DOC_HARD_BREAK ||
        doc_selectable_object_kind(node->kind)) {
        doc_node_source_range bounds; doc_text_format_key key;
        doc_node_source_range_get(state, node, &bounds);
        result = doc_text_format_key_for(view, node, 0, 0, &key);
        if (result == XUI_OK) result = doc_text_format_live_interval(view, records, capacity, count,
            bounds.source_start, bounds.source_end, depth + 1, &key);
        if (result != XUI_OK) return result;
    }
    for (i = 0; i < doc_seq_size(node->children); i++) {
        result = doc_text_format_live_node(view, doc_seq_get_id(node->children, i), depth + 1, records, capacity, count);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
static int doc_text_format_records(doc_view_data* view, doc_text_format_record* records,
    uint64_t capacity, uint64_t* count)
{
    doc_plain_projection* p = &view->edit_projection; xui_document_renderer r = view->renderer;
    doc_text_format_key key; uint64_t i; int result;
    *count = 0;
    if (p->origin.iKind == XUI_DOC_POSITION_SOURCE) {
        int live = r->mode == XUI_DOC_LIVE_MARKDOWN && !r->live_fallback && r->live_rows;
        result = doc_text_format_key_for(view, NULL, 1, live, &key);
        if (result == XUI_OK) result = doc_text_format_record_add(records, capacity, count, 0, p->bytes, 0, &key);
        if (result != XUI_OK || !live) return result;
        result = doc_text_format_live_node(view, DOC_ROOT, 0, records, capacity, count);
        if (result != XUI_OK) return result;
        result = doc_text_format_key_for(view, NULL, 1, 0, &key);
        return result == XUI_OK ? doc_text_format_record_add(records, capacity, count,
            r->live_start, r->live_end, UINT_MAX, &key) : result;
    }
    for (i = 0; i < p->count; i++) {
        const doc_plain_span* span = &p->spans[i]; doc_node* node = doc_index_get(p->state->index, span->first.iNodeId);
        if (!span->literal && span->first.iKind == XUI_DOC_POSITION_GAP && node &&
            span->first.iOffset < doc_seq_size(node->children)) {
            doc_node* child = doc_index_get(p->state->index, doc_seq_get_id(node->children, span->first.iOffset));
            if (child && (child->kind == XUI_DOC_SOFT_BREAK || child->kind == XUI_DOC_HARD_BREAK)) node = child;
        }
        result = doc_text_format_key_for(view, node, 0, 0, &key);
        if (result == XUI_OK) result = doc_text_format_record_add(records, capacity, count,
            span->start, span->start + span->length, 1, &key);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
static int doc_text_format_event_compare(const void* left, const void* right)
{
    const doc_text_format_event *a = left, *b = right;
    return a->at < b->at ? -1 : a->at > b->at;
}
static int doc_text_format_higher(const doc_text_format_record* records, uint64_t a, uint64_t b)
{
    if (records[a].priority != records[b].priority) return records[a].priority > records[b].priority;
    if (records[a].last - records[a].first != records[b].last - records[b].first)
        return records[a].last - records[a].first < records[b].last - records[b].first;
    return a < b;
}
static void doc_text_format_heap_add(const doc_text_format_record* records, uint64_t* heap, uint64_t* count, uint64_t id)
{
    uint64_t at = (*count)++;
    while (at && doc_text_format_higher(records, id, heap[(at - 1) / 2])) {
        heap[at] = heap[(at - 1) / 2]; at = (at - 1) / 2;
    }
    heap[at] = id;
}
static void doc_text_format_heap_pop(const doc_text_format_record* records, uint64_t* heap, uint64_t* count)
{
    uint64_t id = heap[--(*count)], at = 0, child;
    while ((child = at * 2 + 1) < *count) {
        if (child + 1 < *count && doc_text_format_higher(records, heap[child + 1], heap[child])) child++;
        if (!doc_text_format_higher(records, heap[child], id)) break;
        heap[at] = heap[child]; at = child;
    }
    if (*count) heap[at] = id;
}
static int doc_text_format_cache(doc_view_data* view)
{
    doc_plain_projection* p = &view->edit_projection; xui_document_renderer r = view->renderer;
    doc_format_cache* cache = view->text_formats; doc_text_format_record* records = NULL;
    doc_text_format_event* events = NULL; uint64_t *heap = NULL, capacity, count, used = 0, i, next, generation = 0;
    doc_text_format_key previous = {0}; int have_previous = 0, result;
    if (r->input) { xui_doc_prepare_info_t info = {0}; info.iSize = sizeof(info);
        result = xuiDocumentPrepareGetInfo(r->input, &info); if (result != XUI_OK) return result; generation = info.iGeneration;
    }
    if (cache && cache->state == p->state && cache->mode == r->mode && cache->active_source == r->active_source &&
        cache->generation == generation && !memcmp(&cache->descriptor, &r->desc, sizeof(r->desc))) return XUI_OK;
    cache = NULL; /* A failed rebuild must keep the previous cache intact. */
    if (p->origin.iKind != XUI_DOC_POSITION_SOURCE) capacity = p->count;
    else if (r->mode != XUI_DOC_LIVE_MARKDOWN || r->live_fallback || !r->live_rows) capacity = 1;
    else {
        if (p->state->node_count > (UINT64_MAX - 3) / 2) return XUI_DOC_ERROR_LIMIT;
        capacity = p->state->node_count * 2 + 3;
    }
    if (capacity > SIZE_MAX / sizeof(*records)) return XUI_DOC_ERROR_LIMIT;
    records = doc_alloc(p->allocator, (size_t)capacity * sizeof(*records));
    if (!records) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_text_format_records(view, records, capacity, &count); if (result != XUI_OK) goto done;
    if (count > (SIZE_MAX - sizeof(*cache) - sizeof(uint64_t)) / (2 * sizeof(uint64_t)) || count > SIZE_MAX / (2 * sizeof(*events))) {
        result = XUI_DOC_ERROR_LIMIT; goto done;
    }
    events = doc_alloc(p->allocator, (size_t)count * 2 * sizeof(*events));
    heap = doc_alloc(p->allocator, (size_t)count * sizeof(*heap));
    cache = doc_alloc(p->allocator, sizeof(*cache) + (size_t)(count * 2 + 1) * sizeof(uint64_t));
    if (!events || !heap || !cache) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    for (i = 0; i < count; i++) {
        if (records[i].last > p->bytes) { result = XUI_ERROR_INVALID_STATE; goto done; }
        events[i * 2] = (doc_text_format_event){records[i].first, i};
        events[i * 2 + 1] = (doc_text_format_event){records[i].last, i};
    }
    qsort(events, (size_t)count * 2, sizeof(*events), doc_text_format_event_compare);
    cache->count = 0; cache->edges[cache->count++] = 0;
    for (i = 0; i < count * 2; i = next) {
        uint64_t at = events[i].at;
        for (next = i; next < count * 2 && events[next].at == at; next++) {
            uint64_t id = events[next].record;
            if (records[id].first == at) doc_text_format_heap_add(records, heap, &used, id);
        }
        while (used && records[heap[0]].last <= at) doc_text_format_heap_pop(records, heap, &used);
        if (at == p->bytes) break;
        if (!used) { result = XUI_ERROR_INVALID_STATE; goto done; }
        if (have_previous && !doc_text_format_equal(&previous, &records[heap[0]].key)) cache->edges[cache->count++] = at;
        previous = records[heap[0]].key; have_previous = 1;
    }
    cache->edges[cache->count++] = p->bytes; cache->state = p->state; cache->mode = r->mode;
    cache->active_source = r->active_source; cache->generation = generation;
    memcpy(&cache->descriptor, &r->desc, sizeof(r->desc));
    doc_view_format_cache_clear(view); view->text_formats = cache; cache = NULL; result = XUI_OK;
done:
    doc_free(cache); doc_free(events); doc_free(heap); doc_free(records); return result;
}
static int doc_text_format_span(doc_view_data* view, uint64_t at, uint64_t* first, uint64_t* last)
{
    uint64_t lo = 0, hi; int result;
    if (!view->edit_projection.bytes) { *first = *last = 0; return XUI_OK; }
    result = doc_text_format_cache(view); if (result != XUI_OK) return result;
    if (at == view->edit_projection.bytes) at--;
    hi = view->text_formats->count;
    while (lo < hi) {
        uint64_t mid = lo + (hi - lo) / 2;
        if (view->text_formats->edges[mid] <= at) lo = mid + 1; else hi = mid;
    }
    if (!lo || lo >= view->text_formats->count) return XUI_ERROR_INVALID_STATE;
    *first = view->text_formats->edges[lo - 1]; *last = view->text_formats->edges[lo]; return XUI_OK;
}
