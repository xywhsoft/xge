#include "xui_document_view_internal.h"
#include "xui_internal.h"
#include <math.h>

typedef struct doc_editor_bookmark {
    uint64_t before, after;
    xui_doc_range_t start, end;
    struct doc_editor_bookmark* next;
} doc_editor_bookmark;
typedef struct doc_editor_data {
    doc_view_data view;
    xui_doc_editor_desc_t desc;
    uint64_t origin;
    doc_editor_bookmark* bookmarks;
    unsigned bookmark_count;
    uint32_t pending_set, pending_clear;
    int has_pending_marks, composing, committing, caret_visible;
    double blink;
    xui_doc_range_t composition_range;
    xui_doc_position_t composition_caret;
    xui_document_renderer projection;
} doc_editor_data;
static atomic_uint_fast64_t doc_editor_sequence = 1;
static doc_editor_data* doc_editor_get(xui_widget w)
{
    xui_widget_type type = w ? xuiWidgetFindType(xuiWidgetGetContext(w), "document-editor") : NULL;
    return type && xuiWidgetIsType(w, type) ? xuiWidgetGetTypeData(w) : NULL;
}
static void doc_editor_clear_bookmarks(doc_editor_data* e)
{
    while (e->bookmarks) { doc_editor_bookmark* b = e->bookmarks; e->bookmarks = b->next; free(b); }
    e->bookmark_count = 0;
}
XUI_API int xuiDocumentEditorCancelComposition(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w); if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    e->composing = 0; xuiDocumentRendererRelease(e->projection); e->projection = NULL;
    return xuiWidgetInvalidate(w, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API int xuiDocumentEditorIsComposing(xui_widget w) { doc_editor_data* e = doc_editor_get(w); return e && e->composing; }
static void doc_editor_projection_changed(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w);
    if (!e) return;
    xuiDocumentEditorCancelComposition(w);
    e->pending_set = e->pending_clear = 0; e->has_pending_marks = 0;
    e->blink = 0; e->caret_visible = 1;
}
static void doc_editor_changed(xui_document d, xui_document_change_set c, void* user)
{
    doc_editor_data* e = user; doc_editor_bookmark* b;
    if (!c) { doc_editor_clear_bookmarks(e); xuiDocumentEditorCancelComposition(e->view.widget); return; }
    if (!e->committing) xuiDocumentEditorCancelComposition(e->view.widget);
    if (c->origin == e->origin) for (b = e->bookmarks; b; b = b->next) {
        if (c->before->content_id == b->before && c->after->content_id == b->after) { e->view.selection = b->end; break; }
        if (c->before->content_id == b->after && c->after->content_id == b->before) { e->view.selection = b->start; break; }
    }
    e->view.selection.tAnchor.iRevision = e->view.selection.tCaret.iRevision = xuiDocumentGetRevision(d);
    if (!e->view.needs_sync) {
        unsigned i;
        for (i = 0; i < 2; i++) {
            xui_doc_position_t* p = i ? &e->view.selection.tCaret : &e->view.selection.tAnchor; int mapping;
            if (e->view.renderer->mode != XUI_DOC_VISUAL && p->iKind != XUI_DOC_POSITION_SOURCE) {
                uint64_t offset;
                if (xuiDocumentPositionToSource(e->view.renderer->snapshot, p, &offset, &mapping) == XUI_OK) { p->iKind = XUI_DOC_POSITION_SOURCE; p->iNodeId = DOC_ROOT; p->iOffset = offset; }
            } else if (e->view.renderer->mode == XUI_DOC_VISUAL && p->iKind == XUI_DOC_POSITION_SOURCE)
                xuiDocumentSourceToPosition(e->view.renderer->snapshot, p->iOffset, p, &mapping);
        }
    }
    e->blink = 0; e->caret_visible = 1;
}
static int doc_editor_sync(doc_editor_data* e)
{
    return doc_view_sync(&e->view);
}
static void doc_editor_reveal(doc_editor_data* e)
{
    xui_doc_rect_t caret; xui_rect_t content = xuiWidgetGetContentRect(e->view.widget);
    if (content.fW <= 0 || content.fH <= 0) return;
    if (doc_view_layout(&e->view) != XUI_OK || xuiDocumentRendererGetCaretRect(e->view.renderer, &e->view.selection.tCaret, &caret) != XUI_OK) return;
    if (caret.y < e->view.scroll_y) e->view.scroll_y = caret.y;
    else if (caret.y + caret.height > e->view.scroll_y + content.fH) e->view.scroll_y = fmax(0, caret.y + caret.height - content.fH);
    if (caret.x < e->view.scroll_x) e->view.scroll_x = caret.x;
    else if (caret.x + 2 > e->view.scroll_x + content.fW) e->view.scroll_x = fmax(0, caret.x + 2 - content.fW);
    e->blink = 0; e->caret_visible = 1;
    xuiWidgetInvalidate(e->view.widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER |
        (e->view.renderer->mode == XUI_DOC_LIVE_MARKDOWN ? XUI_WIDGET_DIRTY_LAYOUT : 0));
}
static int doc_editor_commit(doc_editor_data* e, xui_document_transaction t, const xui_doc_range_t* after)
{
    doc_editor_bookmark* b; int result;
    if (!t->count) return xuiDocumentTxnCommit(t, NULL);
    b = calloc(1, sizeof(*b)); if (!b) return XUI_ERROR_OUT_OF_MEMORY;
    b->start = e->view.selection; b->end = *after; b->before = t->base->content_id; b->after = t->document->next_state + 1;
    b->next = e->bookmarks; e->bookmarks = b; e->committing = 1;
    result = xuiDocumentTxnCommit(t, NULL); e->committing = 0;
    if (result != XUI_OK) { e->bookmarks = b->next; free(b); return result; }
    if (++e->bookmark_count > t->document->history_limit) {
        doc_editor_bookmark* last = e->bookmarks; unsigned i;
        for (i = 1; i < t->document->history_limit && last->next; i++) last = last->next;
        while (last->next) { b = last->next; last->next = b->next; free(b); e->bookmark_count--; }
    }
    doc_editor_reveal(e); return XUI_OK;
}
static int doc_editor_begin(doc_editor_data* e, xui_document_transaction* t)
{
    xui_doc_txn_desc_t desc = {0}; int result;
    if (e->desc.bReadOnly) return XUI_ERROR_INVALID_STATE;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    desc.iSize = sizeof(desc); desc.iOrigin = e->origin; desc.iBaseRevision = xuiDocumentGetRevision(e->view.document);
    desc.iDomain = e->view.selection.tAnchor.iKind == XUI_DOC_POSITION_SOURCE ? XUI_DOC_SOURCE : XUI_DOC_SEMANTIC;
    return xuiDocumentBeginTransaction(e->view.document, &desc, t);
}
static int doc_editor_replace(doc_editor_data* e, const xui_doc_range_t* range, const char* text, uint64_t bytes)
{
    xui_document_transaction t = NULL; xui_doc_position_t caret = {0}; xui_doc_range_t after; int result;
    result = doc_editor_begin(e, &t); if (result != XUI_OK) return result;
    result = xuiDocumentTxnReplaceRange(t, range, text, bytes, &caret);
    if (result == XUI_OK && bytes && e->has_pending_marks && caret.iKind == XUI_DOC_POSITION_TEXT) {
        if (caret.iOffset >= bytes && !memchr(text, '\n', (size_t)bytes) && !memchr(text, '\r', (size_t)bytes)) {
            xui_doc_range_t marked; marked.tAnchor = marked.tCaret = caret; marked.tAnchor.iOffset -= bytes;
            result = xuiDocumentTxnSetMarks(t, &marked, e->pending_set, e->pending_clear);
            /* Marks can split the inserted run. Locate its final boundary with
             * the operation map instead of retaining a now-shortened node. */
            if (result == XUI_OK && !doc_position_valid(t->draft, &caret)) {
                uint64_t i;
                for (i = 0; i < t->count; i++) if (t->ops[i].iKind == XUI_DOC_OP_SPLIT && caret.iNodeId == t->ops[i].iNodeId && caret.iOffset > t->ops[i].iOffset) {
                    caret.iNodeId = t->ops[i].iOtherNodeId; caret.iOffset -= t->ops[i].iOffset;
                }
            }
        }
    }
    after.tAnchor = after.tCaret = caret;
    if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorInsertText(xui_widget w, const char* text, uint64_t bytes)
{
    doc_editor_data* e = doc_editor_get(w); int result;
    if (!e || !doc_utf8(text, bytes)) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    xuiDocumentEditorCancelComposition(w); return doc_editor_replace(e, &e->view.selection, text, bytes);
}
XUI_API int xuiDocumentEditorSetMarks(xui_widget w, uint32_t set, uint32_t clear)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL; xui_doc_range_t after; int order, result;
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    result = doc_position_compare(e->view.renderer->snapshot->state, &e->view.selection.tAnchor, &e->view.selection.tCaret, &order);
    if (result != XUI_OK) return result;
    if (!order) {
        xui_doc_attributes_t attrs = {0}; attrs.iMarks = set;
        if (!doc_schema_attrs(XUI_DOC_TEXT, &attrs)) return XUI_DOC_ERROR_SCHEMA;
        e->pending_set = (e->pending_set | set) & ~clear; e->pending_clear = (e->pending_clear | clear) & ~set; e->has_pending_marks = 1; return XUI_OK;
    }
    result = doc_editor_begin(e, &t); if (result != XUI_OK) return result;
    after = e->view.selection; result = xuiDocumentTxnSetMarks(t, &after, set, clear);
    if (result == XUI_OK) {
        struct xui_doc_change_set_t changes = {0}; int mapping;
        changes.before = t->base; changes.after = t->draft; changes.identity = t->document->identity;
        changes.before_revision = changes.after_revision = t->base_revision; changes.ops = t->ops; changes.count = t->count;
        result = xuiDocumentMapPosition(&changes, &e->view.selection.tAnchor, &after.tAnchor, &mapping);
        if (result == XUI_OK) result = xuiDocumentMapPosition(&changes, &e->view.selection.tCaret, &after.tCaret, &mapping);
        if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorSetReadOnly(xui_widget w, int read_only)
{
    doc_editor_data* e = doc_editor_get(w); if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    e->desc.bReadOnly = !!read_only; if (read_only) xuiDocumentEditorCancelComposition(w);
    return xuiWidgetSetImeMode(w, read_only ? XUI_IME_DISABLED : XUI_IME_ENABLED);
}
XUI_API int xuiDocumentEditorReplaceAll(xui_widget w, const char* pattern, uint64_t pattern_bytes,
    const char* replacement, uint64_t replacement_bytes, const xui_doc_range_t* scope, uint64_t* replaced)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL; xui_doc_range_t after; uint64_t count; int result, mapping;
    if (replaced) *replaced = 0;
    if (!e || !replaced) return XUI_ERROR_INVALID_ARGUMENT;
    xuiDocumentEditorCancelComposition(w); result = doc_editor_begin(e, &t); if (result != XUI_OK) return result;
    result = xuiDocumentTxnReplaceAll(t, pattern, pattern_bytes, replacement, replacement_bytes, scope, &count);
    if (result == XUI_OK) {
        struct xui_doc_change_set_t changes = {0};
        changes.before = t->base; changes.after = t->draft; changes.identity = t->document->identity;
        changes.before_revision = changes.after_revision = t->base_revision; changes.ops = t->ops; changes.count = t->count;
        result = xuiDocumentMapPosition(&changes, &e->view.selection.tAnchor, &after.tAnchor, &mapping);
        if (result == XUI_OK) result = xuiDocumentMapPosition(&changes, &e->view.selection.tCaret, &after.tCaret, &mapping);
        if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    }
    if (result == XUI_OK) *replaced = count;
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorGetReadOnly(xui_widget w) { doc_editor_data* e = doc_editor_get(w); return !e || e->desc.bReadOnly; }

typedef struct doc_edit_span { uint64_t id; int start, end; unsigned kind; } doc_edit_span;
typedef struct doc_edit_projection { char* text; doc_edit_span* spans; size_t count; int length, offset; uint64_t block; } doc_edit_projection;
static int doc_editor_project(doc_editor_data* e, const xui_doc_position_t* p, doc_edit_projection* out)
{
    doc_state* s = e->view.renderer->snapshot->state; doc_node* n = doc_index_get(s->index, p->iNodeId); uint64_t total = 0, i, count;
    memset(out, 0, sizeof(*out));
    if (p->iKind == XUI_DOC_POSITION_SOURCE) {
        total = doc_seq_size(s->source); if (total > INT_MAX) return XUI_DOC_ERROR_LIMIT;
        out->text = malloc((size_t)total + 1); if (!out->text) return XUI_ERROR_OUT_OF_MEMORY;
        doc_seq_read(s->source, 0, out->text, total); out->text[total] = 0; out->length = (int)total; out->offset = (int)p->iOffset; return XUI_OK;
    }
    if (!n) return XUI_ERROR_NOT_FOUND;
    if (n->kind == XUI_DOC_TEXT) n = doc_index_get(s->index, n->parent);
    out->block = n->id; count = doc_text_kind(n->kind) ? 1 : doc_seq_size(n->children);
    if (count > SIZE_MAX / sizeof(*out->spans)) return XUI_DOC_ERROR_LIMIT;
    out->spans = calloc((size_t)count + 1, sizeof(*out->spans)); if (!out->spans) return XUI_ERROR_OUT_OF_MEMORY;
    for (i = 0; i < count; i++) {
        doc_node* child = doc_text_kind(n->kind) ? n : doc_index_get(s->index, doc_seq_get_id(n->children, i));
        uint64_t length = doc_text_kind(child->kind) ? doc_seq_size(child->text) : 1;
        if (length > INT_MAX || total > INT_MAX - length) { free(out->spans); return XUI_DOC_ERROR_LIMIT; }
        out->spans[i] = (doc_edit_span){child->id, (int)total, (int)(total + length), child->kind}; total += length;
    }
    out->text = malloc((size_t)total + 1); if (!out->text) { free(out->spans); return XUI_ERROR_OUT_OF_MEMORY; }
    out->length = (int)total; out->count = (size_t)count;
    for (i = 0; i < count; i++) {
        doc_edit_span* span = &out->spans[i]; doc_node* child = doc_index_get(s->index, span->id);
        if (doc_text_kind(child->kind)) doc_seq_read(child->text, 0, out->text + span->start, (uint64_t)(span->end - span->start));
        else out->text[span->start] = child->kind == XUI_DOC_SOFT_BREAK || child->kind == XUI_DOC_HARD_BREAK ? '\n' : ' ';
        if (p->iKind == XUI_DOC_POSITION_TEXT && span->id == p->iNodeId) out->offset = span->start + (int)p->iOffset;
    }
    if (p->iKind == XUI_DOC_POSITION_GAP) out->offset = p->iOffset < count ? out->spans[p->iOffset].start : out->length;
    out->text[total] = 0; return XUI_OK;
}
static xui_doc_position_t doc_editor_unproject(doc_edit_projection* projection, xui_doc_position_t p, int offset, int after)
{
    size_t i;
    if (p.iKind == XUI_DOC_POSITION_SOURCE) { p.iOffset = (uint64_t)offset; return p; }
    for (i = 0; i < projection->count; i++) {
        doc_edit_span* span = &projection->spans[i];
        if (offset < span->end || (offset == span->end && (!after || i + 1 == projection->count))) {
            p.iNodeId = span->id; p.iKind = XUI_DOC_POSITION_TEXT; p.iOffset = (uint64_t)(offset - span->start);
            if (!doc_text_kind(span->kind)) { p.iNodeId = projection->block; p.iKind = XUI_DOC_POSITION_GAP; p.iOffset = i + (offset == span->end); }
            return p;
        }
    }
    p.iNodeId = projection->block; p.iKind = XUI_DOC_POSITION_GAP; p.iOffset = after ? projection->count : 0; return p;
}
static xui_doc_position_t doc_editor_edge(doc_state* s, xui_doc_position_t p, uint64_t id, int end)
{
    doc_node* n = doc_index_get(s->index, id);
    while (n && doc_seq_size(n->children)) n = doc_index_get(s->index, doc_seq_get_id(n->children, end ? doc_seq_size(n->children) - 1 : 0));
    if (!n) return p;
    p.iNodeId = n->id; p.iKind = doc_text_kind(n->kind) ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
    p.iOffset = end ? (doc_text_kind(n->kind) ? doc_seq_size(n->text) : doc_seq_size(n->children)) : 0;
    return p;
}
static int doc_editor_horizontal(doc_editor_data* e, xui_doc_position_t p, int right, int word, xui_doc_position_t* out)
{
    doc_edit_projection projection; int offset, result = doc_editor_project(e, &p, &projection);
    if (result != XUI_OK) return result;
    offset = word ? (right ? xuiInternalTextWordNext(projection.text, projection.length, projection.offset, XUI_INTERNAL_WORD_NATURAL) : xuiInternalTextWordPrev(projection.text, projection.length, projection.offset, XUI_INTERNAL_WORD_NATURAL)) :
        (right ? xuiInternalTextGraphemeNext(projection.text, projection.length, projection.offset) : xuiInternalTextGraphemePrev(projection.text, projection.length, projection.offset));
    *out = doc_editor_unproject(&projection, p, offset, right);
    if (offset == projection.offset && p.iKind != XUI_DOC_POSITION_SOURCE) {
        doc_state* s = e->view.renderer->snapshot->state; doc_node* n = doc_index_get(s->index, projection.block);
        while (n && n->parent) {
            doc_node* parent = doc_index_get(s->index, n->parent); uint64_t i = doc_child_index(parent, n->id);
            if ((!right && i) || (right && i + 1 < doc_seq_size(parent->children))) {
                *out = doc_editor_edge(s, p, doc_seq_get_id(parent->children, right ? i + 1 : i - 1), !right); break;
            }
            n = parent;
        }
    }
    free(projection.text); free(projection.spans); return XUI_OK;
}
static int doc_editor_copy(doc_editor_data* e)
{
    char* text; uint64_t bytes; int result; xui_proxy proxy = e->view.renderer->proxy;
    if (!proxy->clipboardSetText) return XUI_ERROR_UNSUPPORTED;
    result = xuiDocumentSnapshotCopyRange(e->view.renderer->snapshot, &e->view.selection, &text, &bytes);
    if (result == XUI_OK) { if (bytes) result = proxy->clipboardSetText(proxy, text); xuiDocumentFreeBuffer(text); }
    return result;
}
static int doc_editor_paste(doc_editor_data* e)
{
    xui_proxy proxy = e->view.renderer->proxy; char* text; char query[1]; int bytes, read, result;
    if (!proxy->clipboardGetText) return XUI_ERROR_UNSUPPORTED;
    bytes = proxy->clipboardGetText(proxy, query, sizeof(query)); if (bytes < 0) return bytes;
    if ((uint64_t)bytes > e->view.document->max_bytes || bytes == INT_MAX) return XUI_DOC_ERROR_LIMIT;
    text = malloc((size_t)bytes + 1); if (!text) return XUI_ERROR_OUT_OF_MEMORY;
    read = proxy->clipboardGetText(proxy, text, bytes + 1);
    result = read < 0 ? read : (read > bytes ? XUI_DOC_ERROR_STALE : doc_editor_replace(e, &e->view.selection, text, (uint64_t)read)); free(text); return result;
}
static uint32_t doc_editor_command_mark(uint32_t command)
{
    switch (command) {
    case XUI_DOC_EDIT_BOLD: return XUI_DOC_BOLD;
    case XUI_DOC_EDIT_ITALIC: return XUI_DOC_ITALIC;
    case XUI_DOC_EDIT_UNDERLINE: return XUI_DOC_UNDERLINE;
    case XUI_DOC_EDIT_STRIKE: return XUI_DOC_STRIKE;
    default: return 0;
    }
}
XUI_API int xuiDocumentEditorQueryCommand(xui_widget w, uint32_t command, xui_doc_command_state_t* state)
{
    doc_editor_data* e = doc_editor_get(w); uint32_t mark = doc_editor_command_mark(command), common = 0, mixed = 0; int order, result;
    if (!e || !state || state->iSize != sizeof(*state)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(state, 0, sizeof(*state)); state->iSize = sizeof(*state);
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    result = doc_position_compare(e->view.renderer->snapshot->state, &e->view.selection.tAnchor, &e->view.selection.tCaret, &order);
    if (result != XUI_OK) return result;
    state->bEnabled = 1;
    if (command < XUI_DOC_EDIT_UNDO || command > XUI_DOC_EDIT_STRIKE) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
    else if (command != XUI_DOC_EDIT_COPY && command != XUI_DOC_EDIT_SELECT_ALL && e->desc.bReadOnly) state->iDisabledReason = XUI_ERROR_INVALID_STATE;
    else if ((command == XUI_DOC_EDIT_COPY || command == XUI_DOC_EDIT_CUT) && (!order || !e->view.renderer->proxy->clipboardSetText)) state->iDisabledReason = XUI_ERROR_INVALID_STATE;
    else if (command == XUI_DOC_EDIT_PASTE && !e->view.renderer->proxy->clipboardGetText) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
    else if ((command == XUI_DOC_EDIT_UNDO && !xuiDocumentCanUndo(e->view.document)) || (command == XUI_DOC_EDIT_REDO && !xuiDocumentCanRedo(e->view.document))) state->iDisabledReason = XUI_ERROR_INVALID_STATE;
    else if (mark && (e->view.renderer->mode != XUI_DOC_VISUAL || (xuiDocumentGetProfile(e->view.document) == XUI_DOCUMENT_MARKDOWN &&
        (mark == XUI_DOC_UNDERLINE || (mark == XUI_DOC_STRIKE && xuiDocumentGetMarkdownDialect(e->view.document) == XUI_MD_COMMONMARK))))) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
    if (mark && e->view.selection.tAnchor.iKind != XUI_DOC_POSITION_SOURCE) {
        result = xuiDocumentSnapshotQueryMarks(e->view.renderer->snapshot, &e->view.selection, &common, &mixed);
        if (result != XUI_OK) return result;
        if (!order && e->has_pending_marks) common = (common | e->pending_set) & ~e->pending_clear;
        state->bActive = !!(common & mark); state->bMixed = !!(mixed & mark);
    }
    state->bEnabled = !state->iDisabledReason; return XUI_OK;
}
XUI_API int xuiDocumentEditorCanExecute(xui_widget w, uint32_t command)
{
    xui_doc_command_state_t state = {0}; state.iSize = sizeof(state);
    return xuiDocumentEditorQueryCommand(w, command, &state) == XUI_OK && state.bEnabled;
}
XUI_API int xuiDocumentEditorExecute(xui_widget w, uint32_t command)
{
    doc_editor_data* e = doc_editor_get(w); xui_doc_range_t range; int order, result; uint32_t mark;
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    xuiDocumentEditorCancelComposition(w);
    if (command == XUI_DOC_EDIT_COPY) return doc_editor_copy(e);
    if (command == XUI_DOC_EDIT_SELECT_ALL) {
        range = e->view.selection; range.tAnchor.iNodeId = range.tCaret.iNodeId = DOC_ROOT; range.tAnchor.iOffset = 0;
        range.tAnchor.iKind = range.tCaret.iKind = e->view.renderer->mode != XUI_DOC_VISUAL ? XUI_DOC_POSITION_SOURCE : XUI_DOC_POSITION_GAP;
        range.tCaret.iOffset = range.tAnchor.iKind == XUI_DOC_POSITION_SOURCE ? doc_seq_size(e->view.renderer->snapshot->state->source) : doc_seq_size(doc_index_get(e->view.renderer->snapshot->state->index, DOC_ROOT)->children);
        return xuiDocumentViewSetSelection(w, &range);
    }
    if (e->desc.bReadOnly) return XUI_ERROR_INVALID_STATE;
    switch (command) {
    case XUI_DOC_EDIT_UNDO: result = xuiDocumentUndo(e->view.document, NULL); doc_editor_reveal(e); return result;
    case XUI_DOC_EDIT_REDO: result = xuiDocumentRedo(e->view.document, NULL); doc_editor_reveal(e); return result;
    case XUI_DOC_EDIT_CUT: result = doc_editor_copy(e); return result == XUI_OK ? doc_editor_replace(e, &e->view.selection, "", 0) : result;
    case XUI_DOC_EDIT_PASTE: return doc_editor_paste(e);
    case XUI_DOC_EDIT_ENTER: return doc_editor_replace(e, &e->view.selection, "\n", 1);
    case XUI_DOC_EDIT_BACKSPACE: case XUI_DOC_EDIT_DELETE:
        range = e->view.selection; result = doc_position_compare(e->view.renderer->snapshot->state, &range.tAnchor, &range.tCaret, &order);
        if (result != XUI_OK) return result;
        if (!order) { result = doc_editor_horizontal(e, range.tCaret, command == XUI_DOC_EDIT_DELETE, 0, &range.tCaret); if (result != XUI_OK) return result; }
        return doc_editor_replace(e, &range, "", 0);
    case XUI_DOC_EDIT_BOLD: case XUI_DOC_EDIT_ITALIC: case XUI_DOC_EDIT_UNDERLINE: case XUI_DOC_EDIT_STRIKE:
        mark = doc_editor_command_mark(command);
        { xui_doc_command_state_t state = {0}; state.iSize = sizeof(state);
          result = xuiDocumentEditorQueryCommand(w, command, &state); if (result != XUI_OK) return result;
          if (!state.bEnabled) return state.iDisabledReason;
          return xuiDocumentEditorSetMarks(w, state.bActive ? 0 : mark, state.bActive ? mark : 0); }
    default: return XUI_ERROR_UNSUPPORTED;
    }
}
static int doc_editor_composition(doc_editor_data* e, const xui_event_t* event)
{
    xui_document_transaction t = NULL; xui_document_renderer renderer = NULL; xui_document_snapshot snapshot = NULL;
    xui_doc_position_t caret; int result; xui_rect_t content = xuiWidgetGetContentRect(e->view.widget);
    if (e->desc.bReadOnly) return XUI_OK;
    if (event->iTextSize < 0 || event->iTextSize >= XUI_EVENT_TEXT_CAPACITY || !doc_utf8(event->sText, (uint64_t)event->iTextSize)) return XUI_DOC_ERROR_UTF8;
    if (!e->composing) e->composition_range = e->view.selection;
    /* Replacement offsets are UTF-8 offsets in the active paragraph/source
     * projection. Never interpret them as whole-document flat rich offsets. */
    if (event->bCompositionReplacementRange) {
        doc_edit_projection projection;
        result = doc_editor_project(e, &e->view.selection.tCaret, &projection); if (result != XUI_OK) return result;
        if (event->iCompositionReplacementStart < 0 || event->iCompositionReplacementEnd < event->iCompositionReplacementStart || event->iCompositionReplacementEnd > projection.length) {
            free(projection.text); free(projection.spans); return XUI_ERROR_INVALID_ARGUMENT;
        }
        e->composition_range.tAnchor = doc_editor_unproject(&projection, e->view.selection.tCaret, event->iCompositionReplacementStart, 1);
        e->composition_range.tCaret = doc_editor_unproject(&projection, e->view.selection.tCaret, event->iCompositionReplacementEnd, 0);
        free(projection.text); free(projection.spans);
    }
    if (!event->bCompositionActive) {
        xui_doc_range_t range = e->composition_range;
        xuiDocumentEditorCancelComposition(e->view.widget);
        return event->iTextSize ? doc_editor_replace(e, &range, event->sText, (uint64_t)event->iTextSize) : XUI_OK;
    }
    result = doc_editor_begin(e, &t); if (result != XUI_OK) return result;
    result = xuiDocumentTxnReplaceRange(t, &e->composition_range, event->sText, (uint64_t)event->iTextSize, &caret);
    if (result == XUI_OK) {
        snapshot = doc_alloc(t->document->allocator, sizeof(*snapshot));
        if (!snapshot) result = XUI_ERROR_OUT_OF_MEMORY;
        else { atomic_init(&snapshot->refs, 1); snapshot->identity = t->document->identity; snapshot->revision = t->base_revision; snapshot->state = doc_state_clone(t->draft); if (!snapshot->state) result = XUI_ERROR_OUT_OF_MEMORY; }
    }
    if (result == XUI_OK) result = xuiDocumentRendererCreate(e->view.renderer->context, &e->view.renderer->desc, &renderer);
    if (result == XUI_OK) result = xuiDocumentRendererSetMode(renderer, e->view.renderer->mode);
    if (result == XUI_OK) result = xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL);
    if (result == XUI_OK && e->view.renderer->mode == XUI_DOC_LIVE_MARKDOWN) result = xuiDocumentRendererSetActivePosition(renderer, &caret);
    if (result == XUI_OK) result = xuiDocumentRendererLayout(renderer, fmax(1, content.fW), e->view.scroll_y, fmax(1, content.fH));
    if (result == XUI_OK) {
        int cursor = event->iCompositionCursor;
        if (cursor < 0) cursor = 0;
        if (cursor > event->iTextSize) cursor = event->iTextSize;
        cursor = xuiInternalTextGraphemeClamp(event->sText, event->iTextSize, cursor);
        if (caret.iOffset >= (uint64_t)(event->iTextSize - cursor)) caret.iOffset -= (uint64_t)(event->iTextSize - cursor);
        xuiDocumentRendererRelease(e->projection); e->projection = renderer; renderer = NULL; e->composition_caret = caret; e->composing = 1;
        xuiWidgetInvalidate(e->view.widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
    }
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentRendererRelease(renderer); xuiDocumentTxnRelease(t); return result;
}
static int doc_editor_key(doc_editor_data* e, const xui_event_t* event)
{
    xui_widget w = e->view.widget; int key = event->iKey, ctrl = (event->iModifiers & XUI_MOD_CTRL) != 0, shift = (event->iModifiers & XUI_MOD_SHIFT) != 0;
    uint32_t command = 0; xui_doc_position_t p; int result, order;
    if (ctrl) switch (key >= 'a' && key <= 'z' ? key - 'a' + 'A' : key) {
    case 'A': command = XUI_DOC_EDIT_SELECT_ALL; break; case 'C': command = XUI_DOC_EDIT_COPY; break;
    case 'X': command = XUI_DOC_EDIT_CUT; break; case 'V': command = XUI_DOC_EDIT_PASTE; break;
    case 'Z': command = shift ? XUI_DOC_EDIT_REDO : XUI_DOC_EDIT_UNDO; break; case 'Y': command = XUI_DOC_EDIT_REDO; break;
    case 'B': command = XUI_DOC_EDIT_BOLD; break; case 'I': command = XUI_DOC_EDIT_ITALIC; break; case 'U': command = XUI_DOC_EDIT_UNDERLINE; break;
    }
    if (key == XUI_KEY_ESCAPE) return xuiDocumentEditorCancelComposition(w);
    if (key == XUI_KEY_BACKSPACE) command = XUI_DOC_EDIT_BACKSPACE;
    if (key == XUI_KEY_DELETE) command = XUI_DOC_EDIT_DELETE;
    if (key == XUI_KEY_ENTER) command = XUI_DOC_EDIT_ENTER;
    if (command) return xuiDocumentEditorExecute(w, command);
    if (e->composing) return XUI_OK;
    p = e->view.selection.tCaret;
    if (key == XUI_KEY_LEFT || key == XUI_KEY_RIGHT) {
        result = doc_position_compare(e->view.renderer->snapshot->state, &e->view.selection.tAnchor, &p, &order);
        if (result != XUI_OK) return result;
        if (!shift && order) p = (key == XUI_KEY_LEFT) == (order < 0) ? e->view.selection.tAnchor : p;
        else { result = doc_editor_horizontal(e, p, key == XUI_KEY_RIGHT, ctrl, &p); if (result != XUI_OK) return result; }
    } else if (key == XUI_KEY_UP || key == XUI_KEY_DOWN || key == XUI_KEY_HOME || key == XUI_KEY_END || key == XUI_KEY_PAGE_UP || key == XUI_KEY_PAGE_DOWN) {
        xui_doc_rect_t caret; xui_rect_t content = xuiWidgetGetContentRect(w); double x, y;
        result = doc_view_layout(&e->view); if (result != XUI_OK) return result;
        result = xuiDocumentRendererGetCaretRect(e->view.renderer, &p, &caret); if (result != XUI_OK) return result;
        x = caret.x; y = caret.y + caret.height * .5;
        if (key == XUI_KEY_HOME || key == XUI_KEY_END) {
            if (ctrl) {
                doc_state* s = e->view.renderer->snapshot->state;
                if (p.iKind == XUI_DOC_POSITION_SOURCE) p.iOffset = key == XUI_KEY_END ? doc_seq_size(s->source) : 0;
                else p = doc_editor_edge(s, p, DOC_ROOT, key == XUI_KEY_END);
                goto select;
            }
            x = key == XUI_KEY_HOME ? 0 : 1e20;
        } else y += (key == XUI_KEY_UP || key == XUI_KEY_PAGE_UP ? -1 : 1) * (key == XUI_KEY_PAGE_UP || key == XUI_KEY_PAGE_DOWN ? fmax(1, content.fH) : caret.height);
        result = xuiDocumentRendererHitTest(e->view.renderer, x, fmax(0, y), &p); if (result != XUI_OK) return result;
    } else return XUI_ERROR_NOT_FOUND;
select:
    p.iAffinity = XUI_DOC_AFTER; e->view.selection.tCaret = p; if (!shift) e->view.selection.tAnchor = p;
    e->view.pending_edit_events |= 2;
    e->has_pending_marks = 0; e->pending_set = e->pending_clear = 0; doc_editor_reveal(e); return XUI_OK;
}
static int doc_editor_event(xui_widget w, const xui_event_t* event, void* user)
{
    doc_editor_data* e = doc_editor_get(w); int result; (void)user;
    if (!e || !event) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    if (event->iType == XUI_EVENT_KEY_DOWN) result = doc_editor_key(e, event);
    else if (event->iType == XUI_EVENT_TEXT) {
        if (event->iModifiers & XUI_MOD_CTRL || e->desc.bReadOnly) return XUI_EVENT_DISPATCH_STOP;
        if (!event->iTextSize && event->iCodepoint) {
            char utf8[4]; uint32_t cp = event->iCodepoint; unsigned bytes;
            if (cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff)) result = XUI_DOC_ERROR_UTF8;
            else {
                if (cp < 0x80) { utf8[0] = (char)cp; bytes = 1; }
                else if (cp < 0x800) { utf8[0] = (char)(0xc0 | (cp >> 6)); utf8[1] = (char)(0x80 | (cp & 63)); bytes = 2; }
                else if (cp < 0x10000) { utf8[0] = (char)(0xe0 | (cp >> 12)); utf8[1] = (char)(0x80 | ((cp >> 6) & 63)); utf8[2] = (char)(0x80 | (cp & 63)); bytes = 3; }
                else { utf8[0] = (char)(0xf0 | (cp >> 18)); utf8[1] = (char)(0x80 | ((cp >> 12) & 63)); utf8[2] = (char)(0x80 | ((cp >> 6) & 63)); utf8[3] = (char)(0x80 | (cp & 63)); bytes = 4; }
                result = xuiDocumentEditorInsertText(w, utf8, bytes);
            }
        } else {
            if (event->iTextSize <= 0 || event->iTextSize >= XUI_EVENT_TEXT_CAPACITY) return XUI_OK;
            result = xuiDocumentEditorInsertText(w, event->sText, (uint64_t)event->iTextSize);
        }
    } else if (event->iType == XUI_EVENT_IME_COMPOSITION) result = doc_editor_composition(e, event);
    else {
        if (event->iType == XUI_EVENT_BLUR || event->iType == XUI_EVENT_POINTER_DOWN) xuiDocumentEditorCancelComposition(w);
        if (event->iType == XUI_EVENT_POINTER_DOWN) { e->has_pending_marks = 0; e->pending_set = e->pending_clear = 0; }
        result = doc_view_event(w, event, NULL); e->blink = 0; e->caret_visible = 1; return result;
    }
    if (result == XUI_ERROR_NOT_FOUND && event->iType == XUI_EVENT_KEY_DOWN) return XUI_OK;
    if (result != XUI_OK && e->desc.onError) e->desc.onError(w, result, e->desc.pUser);
    return XUI_EVENT_DISPATCH_STOP;
}
static xui_rect_t doc_editor_caret_rect(doc_editor_data* e)
{
    xui_doc_rect_t caret = {0}; xui_rect_t rect = {0}, content = xuiWidgetGetContentRect(e->view.widget);
    xui_document_renderer renderer = e->composing ? e->projection : e->view.renderer;
    const xui_doc_position_t* p = e->composing ? &e->composition_caret : &e->view.selection.tCaret;
    if (renderer && xuiDocumentRendererGetCaretRect(renderer, p, &caret) == XUI_OK) {
        rect.fX = (int)floor(content.fX + caret.x - e->view.scroll_x); rect.fY = (int)floor(content.fY + caret.y - e->view.scroll_y);
        rect.fW = 1; rect.fH = (int)ceil(caret.height);
    }
    return rect;
}
static xui_rect_t doc_editor_ime_rect(xui_widget w, void* user)
{
    doc_editor_data* e = doc_editor_get(w); xui_rect_t r = {0}, world = xuiWidgetGetWorldRect(w); (void)user;
    if (e) { doc_view_layout(&e->view); r = doc_editor_caret_rect(e); r.fX += world.fX; r.fY += world.fY; } return r;
}
static int doc_editor_render(xui_widget w, xui_draw_context draw, uint32_t state, void* user)
{
    doc_editor_data* e = doc_editor_get(w); xui_rect_t content = xuiWidgetGetContentRect(w), caret; int result;
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    if (e->composing && e->projection) {
        result = xuiDocumentRendererLayout(e->projection, fmax(1, content.fW), e->view.scroll_y, fmax(1, content.fH));
        if (result == XUI_OK) result = xuiDocumentRendererDraw(e->projection, draw, content.fX - e->view.scroll_x, content.fY - e->view.scroll_y, content, NULL, 0);
    } else result = doc_view_render(w, draw, state, user);
    if (result != XUI_OK) return result;
    if (!e->desc.bReadOnly && (e->caret_visible || e->composing) && xuiGetFocusWidget(xuiWidgetGetContext(w)) == w) {
        caret = doc_editor_caret_rect(e);
        if (caret.fH && caret.fX >= content.fX && caret.fX < content.fX + content.fW) {
            int bottom = caret.fY + caret.fH; caret.fY = caret.fY < content.fY ? content.fY : caret.fY;
            if (bottom > content.fY + content.fH) bottom = content.fY + content.fH;
            caret.fH = bottom > caret.fY ? bottom - caret.fY : 0;
            if (caret.fH && e->view.renderer->proxy->drawRectFill) e->view.renderer->proxy->drawRectFill(e->view.renderer->proxy, draw, caret, e->desc.iCaretColor);
        }
    }
    return XUI_OK;
}
static int doc_editor_update(xui_widget w, float dt, void* user)
{
    doc_editor_data* e = doc_editor_get(w); (void)user;
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (xuiGetFocusWidget(xuiWidgetGetContext(w)) == w && !e->desc.bReadOnly && !e->composing) {
        e->blink += dt; if (e->blink >= .5) { e->blink = fmod(e->blink, .5); e->caret_visible = !e->caret_visible; xuiWidgetInvalidate(w, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER); }
    }
    return doc_view_update(w, dt, user);
}
static int doc_editor_init(xui_widget w, void* data, const void* create, void* user)
{
    doc_editor_data* e = data; const xui_doc_editor_desc_t* desc = create; int result; (void)user;
    if (!desc || desc->iSize != sizeof(*desc)) return XUI_ERROR_INVALID_ARGUMENT;
    e->desc = *desc; e->origin = atomic_fetch_add(&doc_editor_sequence, 1); e->caret_visible = 1;
    e->view.onChanged = doc_editor_changed;
    e->view.onProjectionChanged = doc_editor_projection_changed;
    if (!e->desc.iCaretColor) e->desc.iCaretColor = XUI_COLOR_RGBA(30, 30, 30, 255);
    result = desc->iMode ? xuiDocumentViewSetMode(w, desc->iMode) : XUI_OK;
    if (result == XUI_OK) result = doc_editor_sync(e);
    if (result != XUI_OK) return result;
    xuiWidgetSetFocusable(w, 1); xuiWidgetSetTabStop(w, 1); xuiWidgetSetImeMode(w, desc->bReadOnly ? XUI_IME_DISABLED : XUI_IME_ENABLED);
    xuiWidgetSetImeCandidateRect(w, doc_editor_ime_rect, NULL);
    result = doc_view_register_edit(w, 1);
    return result == XUI_OK ? xuiWidgetSetEventCallback(w, doc_editor_event, NULL) : result;
}
static void doc_editor_destroy(xui_widget w, void* data, void* user)
{
    doc_editor_data* e = data; (void)w; (void)user;
    e->view.onChanged = NULL;
    e->view.onProjectionChanged = NULL;
    xuiDocumentRendererRelease(e->projection); doc_editor_clear_bookmarks(e);
}
XUI_API xui_widget_type xuiDocumentEditorGetType(xui_context context)
{
    xui_widget_type type = xuiWidgetFindType(context, "document-editor"); xui_widget_type_desc_t desc = {0};
    if (type) return type;
    desc.iSize = sizeof(desc); desc.sName = "document-editor"; desc.pParent = xuiDocumentViewGetType(context); desc.iTypeDataSize = sizeof(doc_editor_data);
    if (!desc.pParent) return NULL;
    desc.onInit = doc_editor_init; desc.onDestroy = doc_editor_destroy; desc.onCacheRender = doc_editor_render; desc.onUpdate = doc_editor_update;
    return xuiWidgetRegisterType(context, &type, &desc) == XUI_OK ? type : NULL;
}
XUI_API int xuiDocumentEditorCreate(xui_context context, const xui_doc_editor_desc_t* desc, xui_widget* out)
{
    xui_widget_type type;
    if (out) *out = NULL;
    if (!out || !desc || desc->iSize != sizeof(*desc) || desc->tView.iSize != sizeof(desc->tView) || !desc->tView.pDocument) return XUI_ERROR_INVALID_ARGUMENT;
    type = xuiDocumentEditorGetType(context); return type ? xuiWidgetCreateTyped(context, type, out, desc) : XUI_ERROR_OUT_OF_MEMORY;
}
