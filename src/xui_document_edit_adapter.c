#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT_EDITOR
#include "xui_document_view_internal.h"
#include "xui_document_accessible_internal.h"
#include "xui_internal.h"
#include "xui_document_image_resource_internal.h"
#include <limits.h>
#include <math.h>

void doc_view_invalidate_edit(doc_view_data* v)
{
    doc_plain_projection_free(&v->edit_projection);
    xuiDocumentSnapshotRelease(v->edit_snapshot); v->edit_snapshot = NULL;
}
static int doc_view_project_edit(doc_view_data* v)
{
    int result = doc_view_sync(v); unsigned domain = v->renderer->mode != XUI_DOC_VISUAL ? XUI_DOC_SOURCE : XUI_DOC_SEMANTIC;
    if (result != XUI_OK) return result;
    if (v->edit_snapshot && v->edit_snapshot->revision == v->renderer->snapshot->revision &&
        v->edit_projection.origin.iInputGeneration == (v->input ? v->selection.tCaret.iInputGeneration : 0) &&
        (v->edit_projection.origin.iKind == XUI_DOC_POSITION_SOURCE) == (domain == XUI_DOC_SOURCE)) return XUI_OK;
    doc_view_invalidate_edit(v);
    if (v->input) {
        doc_plain_projection* p = &v->edit_projection; uint64_t bytes = doc_seq_size(doc_render_source(v->renderer));
        if (bytes >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
        p->state = v->renderer->snapshot->state; p->allocator = p->state->allocator;
        p->text = doc_alloc(p->allocator, (size_t)bytes + 1); if (!p->text) return XUI_ERROR_OUT_OF_MEMORY;
        p->bytes = p->capacity = bytes;
        result = xuiDocumentPrepareCopySource(v->input, p->text, bytes + 1, &bytes);
        if (result == XUI_OK) result = xuiDocumentPrepareSourcePosition(v->input, 0, XUI_DOC_AFTER, &p->origin);
    } else result = doc_plain_project(v->renderer->snapshot, domain, &v->edit_projection);
    if (result == XUI_OK) { v->edit_snapshot = v->renderer->snapshot; xuiDocumentSnapshotRetain(v->edit_snapshot); }
    return result;
}
static const char* doc_view_edit_text(xui_widget w)
{
    doc_view_data* v = doc_view_get(w);
    return v && doc_view_project_edit(v) == XUI_OK && v->edit_projection.bytes <= INT_MAX ? v->edit_projection.text : NULL;
}
static int doc_view_edit_get_selection(xui_widget w, int* first, int* last)
{
    doc_view_data* v = doc_view_get(w); uint64_t a, b; int result;
    if (!v || !first || !last) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    if (v->renderer->mode != XUI_DOC_VISUAL &&
        v->selection.tAnchor.iKind == XUI_DOC_POSITION_SOURCE &&
        v->selection.tCaret.iKind == XUI_DOC_POSITION_SOURCE) {
        int order;
        result = doc_render_compare(v->renderer, &v->selection.tAnchor, &v->selection.tCaret, &order);
        if (result != XUI_OK) return result;
        a = v->selection.tAnchor.iOffset; b = v->selection.tCaret.iOffset;
        if (a > INT_MAX || b > INT_MAX) return XUI_DOC_ERROR_LIMIT;
        *first = (int)a; *last = (int)b; return XUI_OK;
    }
    result = doc_view_project_edit(v); if (result != XUI_OK) return result;
    if (v->input) { a = v->selection.tAnchor.iOffset; b = v->selection.tCaret.iOffset; }
    else {
        result = doc_plain_project_position(&v->edit_projection, &v->selection.tAnchor, &a);
        if (result == XUI_OK) result = doc_plain_project_position(&v->edit_projection, &v->selection.tCaret, &b);
    }
    if (result != XUI_OK) return result;
    if (a > INT_MAX || b > INT_MAX) return XUI_DOC_ERROR_LIMIT;
    *first = (int)a; *last = (int)b; return XUI_OK;
}
static int doc_view_edit_set_selection(xui_widget w, int first, int last)
{
    doc_view_data* v = doc_view_get(w); xui_doc_range_t range; int result;
    if (!v || first < 0 || last < 0) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->onBeforeSelection && (result = v->onBeforeSelection(w)) != XUI_OK) return result;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    if (v->renderer->mode != XUI_DOC_VISUAL) {
        doc_sequence* source = doc_render_source(v->renderer);
        uint64_t length = doc_seq_size(source);
        if ((uint64_t)first > length || (uint64_t)last > length) return XUI_ERROR_INVALID_ARGUMENT;
        if (!doc_seq_boundary(source, (uint64_t)first) || !doc_seq_boundary(source, (uint64_t)last))
            return XUI_DOC_ERROR_UTF8;
        if (v->input) {
            result = xuiDocumentPrepareSourcePosition(v->input, (uint64_t)first, XUI_DOC_AFTER, &range.tAnchor);
            if (result == XUI_OK) result = xuiDocumentPrepareSourcePosition(v->input, (uint64_t)last,
                first <= last ? XUI_DOC_BEFORE : XUI_DOC_AFTER, &range.tCaret);
            if (result != XUI_OK) return result;
        } else {
            memset(&range, 0, sizeof(range));
            range.tAnchor.iSize = sizeof(range.tAnchor);
            range.tAnchor.iDocumentId = v->renderer->snapshot->identity;
            range.tAnchor.iRevision = v->renderer->snapshot->revision;
            range.tAnchor.iNodeId = DOC_ROOT;
            range.tAnchor.iKind = XUI_DOC_POSITION_SOURCE;
            range.tAnchor.iOffset = (uint64_t)first;
            range.tAnchor.iAffinity = XUI_DOC_AFTER;
            range.tCaret = range.tAnchor;
            range.tCaret.iOffset = (uint64_t)last;
            range.tCaret.iAffinity = first <= last ? XUI_DOC_BEFORE : XUI_DOC_AFTER;
        }
        if (first == last) range.tCaret = range.tAnchor;
        return xuiDocumentViewSetSelection(w, &range);
    }
    result = doc_view_project_edit(v); if (result != XUI_OK) return result;
    if ((uint64_t)first > v->edit_projection.bytes || (uint64_t)last > v->edit_projection.bytes) return XUI_ERROR_INVALID_ARGUMENT;
    if (((uint64_t)first < v->edit_projection.bytes && ((unsigned char)v->edit_projection.text[first] & 0xc0) == 0x80) ||
        ((uint64_t)last < v->edit_projection.bytes && ((unsigned char)v->edit_projection.text[last] & 0xc0) == 0x80)) return XUI_DOC_ERROR_UTF8;
    if (v->input) {
        xuiDocumentPrepareSourcePosition(v->input, (uint64_t)first, XUI_DOC_AFTER, &range.tAnchor);
        xuiDocumentPrepareSourcePosition(v->input, (uint64_t)last, first <= last ? XUI_DOC_BEFORE : XUI_DOC_AFTER, &range.tCaret);
    } else {
        range.tAnchor = doc_plain_unproject(&v->edit_projection, (uint64_t)first, 1);
        range.tCaret = doc_plain_unproject(&v->edit_projection, (uint64_t)last, first <= last ? 0 : 1);
    }
    if (first == last) range.tCaret = range.tAnchor;
    return xuiDocumentViewSetSelection(w, &range);
}
static int doc_view_edit_has_selection(xui_widget w)
{
    doc_view_data* v = doc_view_get(w); int order;
    return v && doc_view_sync(v) == XUI_OK && (v->table_selection.iTableId ||
        (doc_render_compare(v->renderer, &v->selection.tAnchor, &v->selection.tCaret, &order) == XUI_OK && order));
}
static int doc_view_edit_select_all(xui_widget w)
{
    doc_view_data* v = doc_view_get(w); xui_doc_range_t range;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    range = v->selection; range.tAnchor.iNodeId = range.tCaret.iNodeId = DOC_ROOT;
    range.tAnchor.iKind = range.tCaret.iKind = v->renderer->mode != XUI_DOC_VISUAL ? XUI_DOC_POSITION_SOURCE : XUI_DOC_POSITION_GAP;
    range.tAnchor.iOffset = 0; range.tCaret.iOffset = v->renderer->mode != XUI_DOC_VISUAL ? doc_seq_size(doc_render_source(v->renderer)) : doc_seq_size(doc_index_get(v->document->state->index, DOC_ROOT)->children);
    return xuiDocumentViewSetSelection(w, &range);
}
static int doc_view_edit_copy(xui_widget w)
{
    doc_view_data* v = doc_view_get(w); char* text; uint64_t bytes; int result;
    char* native = NULL; uint64_t native_bytes = 0;
    char* html = NULL; uint64_t html_bytes = 0;
    void* image_png = NULL; size_t image_png_bytes = 0;
    xui_document_fragment fragment = NULL;
    int order = 0, semantic_selection = 0;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    if (!v->renderer->proxy->clipboardSetText) return XUI_ERROR_UNSUPPORTED;
    if (v->onBeforeSelection && (result = v->onBeforeSelection(w)) != XUI_OK) return result;
    result = doc_view_sync(v);
    if (result == XUI_OK) {
        if (v->table_selection.iTableId) {
            const xui_doc_table_selection_t* selected = &v->table_selection;
            result = xuiDocumentSnapshotCopyTableMatrix(v->renderer->snapshot,
                selected->iTableId, selected->iRow, selected->iColumn,
                selected->iRows, selected->iColumns, &text, &bytes);
        } else result = doc_render_copy_range(v->renderer, &v->selection, &text, &bytes);
        if (result == XUI_OK) {
            if (!v->table_selection.iTableId && v->renderer->mode == XUI_DOC_VISUAL &&
                doc_render_compare(v->renderer, &v->selection.tAnchor,
                    &v->selection.tCaret, &order) == XUI_OK && order)
                semantic_selection = 1;
            if (memchr(text, 0, (size_t)bytes)) result = XUI_DOC_ERROR_UNREPRESENTABLE;
            else if (bytes || v->table_selection.iTableId || semantic_selection) {
                if (semantic_selection && v->renderer->proxy->clipboardSetItems) {
                    int capture = xuiDocumentFragmentCreateRange(
                        v->renderer->snapshot, &v->selection, &fragment);
                    int fatal_image = 0;
                    if (capture == XUI_OK) capture = xuiDocumentFragmentSerialize(
                        fragment, &native, &native_bytes);
                    if (capture == XUI_OK) capture = xuiDocumentFragmentExportHtml(
                        fragment, &html, &html_bytes);
                    if (capture == XUI_OK) {
                        const char* image_resource = doc_fragment_single_image_resource(fragment);
                        if (image_resource) {
                            int image_result = doc_image_resource_export_png(
                                xuiWidgetGetContext(w), image_resource, &image_png, &image_png_bytes);
                            if (!xuiInternalWidgetIsValid(w)) image_result = XUI_DOC_ERROR_STALE;
                            if (image_result != XUI_OK && image_result != XUI_ERROR_NOT_FOUND &&
                                image_result != XUI_ERROR_UNSUPPORTED) {
                                capture = image_result;
                                fatal_image = 1;
                            }
                        }
                    }
                    xuiDocumentFragmentRelease(fragment);
                    if (capture == XUI_ERROR_OUT_OF_MEMORY || capture == XUI_DOC_ERROR_LIMIT ||
                        capture == XUI_DOC_ERROR_STALE || fatal_image)
                        result = capture;
                }
                if (result == XUI_OK && native) {
                    xui_clipboard_item_t items[4] = {
                        {XUI_CLIPBOARD_FORMAT_TEXT_UTF8, text, (size_t)bytes},
                        {XUI_CLIPBOARD_FORMAT_DOCUMENT_FRAGMENT, native, (size_t)native_bytes},
                        {XUI_CLIPBOARD_FORMAT_HTML, html, (size_t)html_bytes},
                        {XUI_CLIPBOARD_FORMAT_IMAGE_PNG, image_png, image_png_bytes}
                    };
                    result = v->renderer->proxy->clipboardSetItems(v->renderer->proxy,
                        items, image_png ? 4 : (html ? 3 : 2));
                } else if (result == XUI_OK && (bytes || v->table_selection.iTableId))
                    result = v->renderer->proxy->clipboardSetText(v->renderer->proxy, text);
                else if (result == XUI_OK && !v->table_selection.iTableId)
                    result = XUI_ERROR_UNSUPPORTED;
            }
            xuiDocumentFreeBuffer(native);
            xuiDocumentFreeBuffer(html);
            xrtFree(image_png);
            xuiDocumentFreeBuffer(text);
        }
    }
    return result;
}
static int doc_view_read_only(xui_widget w) { (void)w; return 1; }
static int doc_editor_edit_set_text(xui_widget w, const char* text)
{
    xui_doc_range_t before; int result;
    if (!text) return XUI_ERROR_INVALID_ARGUMENT;
    if (xuiDocumentEditorGetReadOnly(w)) return XUI_ERROR_INVALID_STATE;
    result = xuiDocumentViewGetSelection(w, &before);
    if (result == XUI_OK) result = doc_view_edit_select_all(w);
    if (result == XUI_OK) {
        result = xuiDocumentEditorInsertText(w, text, strlen(text));
        if (result != XUI_OK) xuiDocumentViewSetSelection(w, &before);
    }
    return result;
}
static int doc_editor_edit_delete(xui_widget w)
{
    doc_view_data* v = doc_view_get(w);
    if (v && v->table_selection.iTableId)
        return xuiDocumentEditorExecute(w, XUI_DOC_EDIT_DELETE);
    return doc_view_edit_has_selection(w) ? xuiDocumentEditorInsertText(w, "", 0) : XUI_OK;
}
static int doc_editor_edit_cut(xui_widget w) { return xuiDocumentEditorExecute(w, XUI_DOC_EDIT_CUT); }
static int doc_editor_edit_copy(xui_widget w) { return xuiDocumentEditorExecute(w, XUI_DOC_EDIT_COPY); }
static int doc_editor_edit_paste(xui_widget w) { return xuiDocumentEditorExecute(w, XUI_DOC_EDIT_PASTE); }
static int doc_editor_edit_undo(xui_widget w) { return xuiDocumentEditorExecute(w, XUI_DOC_EDIT_UNDO); }
static int doc_editor_edit_redo(xui_widget w) { return xuiDocumentEditorExecute(w, XUI_DOC_EDIT_REDO); }
static int doc_editor_edit_can_undo(xui_widget w) { return xuiDocumentEditorCanExecute(w, XUI_DOC_EDIT_UNDO); }
static int doc_editor_edit_can_redo(xui_widget w) { return xuiDocumentEditorCanExecute(w, XUI_DOC_EDIT_REDO); }
static xui_rect_t doc_view_edit_caret(xui_widget w)
{
    doc_view_data* v = doc_view_get(w); xui_doc_rect_t caret; xui_rect_t rect = {0}, content = xuiWidgetGetContentRect(w);
    if (v && doc_view_layout(v) == XUI_OK && xuiDocumentRendererGetCaretRect(v->renderer, &v->selection.tCaret, &caret) == XUI_OK) {
        rect.fX = (int)floor(content.fX + caret.x - v->scroll_x); rect.fY = (int)floor(content.fY + caret.y - v->scroll_y);
        rect.fW = 1; rect.fH = (int)ceil(caret.height);
    }
    return rect;
}
typedef struct doc_accessible_frame {
    doc_node* node;
    uint64_t next_child;
} doc_accessible_frame;
void doc_view_accessible_clear(doc_view_data* v)
{
    uint64_t i;
    if (!v) return;
    if (v->accessible_text)
        for (i = 0; i < v->accessible_count; i++) free(v->accessible_text[i]);
    free(v->accessible_text); free(v->accessible_ids);
    v->accessible_text = NULL; v->accessible_ids = NULL;
    v->accessible_count = v->accessible_identity = v->accessible_revision = 0;
}
int doc_accessible_snapshot_ids(xui_document_snapshot snapshot,
    xui_doc_node_id** ids, uint64_t* out_count)
{
    doc_accessible_frame* frames = NULL;
    doc_node* root; size_t depth = 0, capacity = 0; uint64_t count = 0, limit;
    xui_doc_node_id* result_ids = NULL;
    int result = XUI_OK;
    if (ids) *ids = NULL;
    if (out_count) *out_count = 0;
    if (!snapshot || !ids || !out_count) return XUI_ERROR_INVALID_ARGUMENT;
    limit = snapshot->state->node_count;
    if (!limit || limit > INT_MAX || limit > SIZE_MAX / sizeof(*result_ids))
        return XUI_DOC_ERROR_LIMIT;
    result_ids = malloc((size_t)limit * sizeof(*result_ids));
    if (!result_ids) return XUI_ERROR_OUT_OF_MEMORY;
    root = doc_index_get(snapshot->state->index, DOC_ROOT);
    if (!root) { result = XUI_DOC_ERROR_SCHEMA; goto fail; }
    while (root || depth) {
        doc_accessible_frame* frame;
        if (root) {
            if (depth == capacity) {
                size_t next = capacity ? capacity * 2 : 16;
                doc_accessible_frame* grown;
                if (next < capacity || next > SIZE_MAX / sizeof(*frames)) { result = XUI_DOC_ERROR_LIMIT; goto fail; }
                grown = realloc(frames, next * sizeof(*frames));
                if (!grown) { result = XUI_ERROR_OUT_OF_MEMORY; goto fail; }
                frames = grown; capacity = next;
            }
            frames[depth++] = (doc_accessible_frame){root, 0};
            if (count >= limit) { result = XUI_DOC_ERROR_SCHEMA; goto fail; }
            result_ids[count++] = root->id; root = NULL;
        }
        frame = &frames[depth - 1];
        if (frame->next_child < doc_seq_size(frame->node->children)) {
            uint64_t child = doc_seq_get_id(frame->node->children, frame->next_child++);
            root = doc_index_get(snapshot->state->index, child);
            if (!root) { result = XUI_DOC_ERROR_SCHEMA; goto fail; }
        } else depth--;
    }
    free(frames); frames = NULL;
    if (count != limit) { result = XUI_DOC_ERROR_SCHEMA; goto fail; }
    *ids = result_ids; *out_count = count;
    return XUI_OK;
fail:
    free(frames); free(result_ids); return result;
}
static int doc_view_accessible_index(doc_view_data* v)
{
    xui_document_snapshot snapshot;
    int result = doc_view_sync(v);
    if (result != XUI_OK) return result;
    snapshot = v->renderer->snapshot;
    if (v->accessible_ids && v->accessible_identity == snapshot->identity &&
        v->accessible_revision == snapshot->revision) return XUI_OK;
    doc_view_accessible_clear(v);
    result = doc_accessible_snapshot_ids(snapshot,
        &v->accessible_ids, &v->accessible_count);
    if (result != XUI_OK) return result;
    v->accessible_text = calloc((size_t)v->accessible_count, sizeof(*v->accessible_text));
    if (!v->accessible_text) {
        doc_view_accessible_clear(v);
        return XUI_ERROR_OUT_OF_MEMORY;
    }
    v->accessible_identity = snapshot->identity;
    v->accessible_revision = snapshot->revision;
    return XUI_OK;
}
static const char* doc_view_accessible_text(doc_view_data* v, uint64_t index, doc_node* node)
{
    uint64_t bytes = doc_seq_size(node->text); char* text;
    if (!bytes) return "";
    if (v->accessible_text[index]) return v->accessible_text[index];
    if (bytes >= SIZE_MAX) return NULL;
    text = malloc((size_t)bytes + 1); if (!text) return NULL;
    if (doc_seq_read(node->text, 0, text, bytes) != XUI_OK) { free(text); return NULL; }
    text[bytes] = 0; v->accessible_text[index] = text; return text;
}
static int doc_accessible_inline_container(uint32_t kind)
{
    return kind == XUI_DOC_PARAGRAPH || kind == XUI_DOC_HEADING;
}
static int doc_accessible_text_leaf(uint32_t kind)
{
    return kind == XUI_DOC_TEXT || kind == XUI_DOC_CODE_BLOCK ||
        kind == XUI_DOC_FOOTNOTE_REF;
}
/* A paragraph's accessible value has one replacement character per inline
 * object. Its child node retains the alt/source value and semantic role. */
static int doc_accessible_inline_part(doc_state* state, doc_node* parent,
    uint64_t index, doc_node** child, uint64_t* bytes, const char** substitute)
{
    *child = doc_index_get(state->index, doc_seq_get_id(parent->children, index));
    if (!*child) return XUI_DOC_ERROR_SCHEMA;
    if ((*child)->kind == XUI_DOC_TEXT || (*child)->kind == XUI_DOC_FOOTNOTE_REF) {
        *bytes = doc_seq_size((*child)->text); *substitute = NULL;
    } else if ((*child)->kind == XUI_DOC_SOFT_BREAK ||
        (*child)->kind == XUI_DOC_HARD_BREAK) {
        *bytes = 1; *substitute = "\n";
    } else if (doc_inline_kind((*child)->kind)) {
        *bytes = 3; *substitute = "\xef\xbf\xbc";
    } else return XUI_DOC_ERROR_SCHEMA;
    return XUI_OK;
}
static int doc_accessible_inline_measure(doc_state* state, doc_node* parent,
    uint64_t* bytes)
{
    uint64_t i, count = doc_seq_size(parent->children), total = 0;
    for (i = 0; i < count; i++) {
        doc_node* child; uint64_t part; const char* substitute;
        int result = doc_accessible_inline_part(state, parent, i,
            &child, &part, &substitute);
        if (result != XUI_OK) return result;
        if (part > INT_MAX - total) return XUI_DOC_ERROR_LIMIT;
        total += part;
    }
    *bytes = total; return XUI_OK;
}
static int doc_view_accessible_inline_text(doc_view_data* v, uint64_t index,
    doc_node* parent, const char** out, uint64_t* bytes)
{
    char* text;
    int result;
    if (v->accessible_text[index]) {
        result = doc_accessible_inline_measure(v->renderer->snapshot->state,
            parent, bytes);
        if (result == XUI_OK) *out = v->accessible_text[index];
        return result;
    }
    result = doc_accessible_snapshot_value(v->renderer->snapshot,
        parent, &text, bytes);
    if (result != XUI_OK) return result;
    v->accessible_text[index] = text; *out = text;
    return XUI_OK;
}
static int doc_accessible_inline_boundary(doc_state* state, doc_node* parent,
    uint64_t offset)
{
    uint64_t i, count = doc_seq_size(parent->children), at = 0;
    for (i = 0; i < count; i++) {
        doc_node* child; uint64_t part; const char* substitute;
        if (doc_accessible_inline_part(state, parent, i,
                &child, &part, &substitute) != XUI_OK) return 0;
        if (offset > at && offset < at + part)
            return substitute ? 0 : doc_seq_boundary(child->text, offset - at);
        at += part;
    }
    return offset <= at;
}
static int doc_accessible_inline_unproject(xui_document_snapshot snapshot,
    doc_node* parent, uint64_t offset, int prefer_after,
    xui_doc_position_t* out)
{
    doc_state* state = snapshot->state;
    uint64_t i, count = doc_seq_size(parent->children), at = 0;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    out->iDocumentId = snapshot->identity; out->iRevision = snapshot->revision;
    out->iNodeId = parent->id; out->iKind = XUI_DOC_POSITION_GAP;
    out->iAffinity = prefer_after ? XUI_DOC_AFTER : XUI_DOC_BEFORE;
    if (!count) return offset ? XUI_ERROR_INVALID_ARGUMENT : XUI_OK;
    for (i = 0; i < count; i++) {
        doc_node* child; uint64_t part; const char* substitute; int result;
        result = doc_accessible_inline_part(state, parent, i,
            &child, &part, &substitute);
        if (result != XUI_OK) return result;
        if (!part) continue;
        if (offset > at + part || (offset == at + part && prefer_after && i + 1 < count)) {
            at += part; continue;
        }
        if (offset < at || offset > at + part) return XUI_ERROR_INVALID_ARGUMENT;
        if (!substitute) {
            out->iNodeId = child->id; out->iKind = XUI_DOC_POSITION_TEXT;
            out->iOffset = offset - at;
        } else out->iOffset = offset == at ? i : i + 1;
        return XUI_OK;
    }
    if (offset != at) return XUI_ERROR_INVALID_ARGUMENT;
    out->iOffset = count; return XUI_OK;
}
static int doc_accessible_inline_project(doc_state* state, doc_node* parent,
    const xui_doc_position_t* position, uint64_t total, uint64_t* out)
{
    xui_doc_position_t start = *position, end = *position;
    uint64_t i, count = doc_seq_size(parent->children), at = 0;
    int order, result;
    start.iNodeId = end.iNodeId = parent->id;
    start.iKind = end.iKind = XUI_DOC_POSITION_GAP;
    start.iOffset = 0; end.iOffset = count;
    result = doc_position_compare(state, position, &start, &order);
    if (result != XUI_OK) return result;
    if (order <= 0) { *out = 0; return XUI_OK; }
    result = doc_position_compare(state, position, &end, &order);
    if (result != XUI_OK) return result;
    if (order >= 0) { *out = total; return XUI_OK; }
    for (i = 0; i < count; i++) {
        doc_node* child; uint64_t part; const char* substitute;
        result = doc_accessible_inline_part(state, parent, i,
            &child, &part, &substitute);
        if (result != XUI_OK) return result;
        if (position->iNodeId == parent->id && position->iKind == XUI_DOC_POSITION_GAP &&
            position->iOffset == i) { *out = at; return XUI_OK; }
        if (position->iNodeId == child->id) {
            *out = at + (!substitute ? position->iOffset : position->iOffset ? part : 0);
            return XUI_OK;
        }
        at += part;
    }
    return XUI_ERROR_INVALID_ARGUMENT;
}
static int doc_accessible_structured_container(uint32_t kind)
{
    return kind == XUI_DOC_LIST_ITEM || kind == XUI_DOC_CELL ||
        kind == XUI_DOC_ROW || kind == XUI_DOC_TABLE ||
        kind == XUI_DOC_LIST || kind == XUI_DOC_QUOTE ||
        kind == XUI_DOC_FOOTNOTE;
}
int doc_accessible_text_selectable_kind(uint32_t kind)
{
    return doc_accessible_text_leaf(kind) ||
        doc_accessible_inline_container(kind) ||
        doc_accessible_structured_container(kind);
}
/* These spans are UI-owned. doc_plain_project_position/unproject can read them,
 * but doc_plain_projection_free must not release their malloc storage. */
static void doc_accessible_projection_free(doc_plain_projection* p)
{
    free(p->text); free(p->spans); memset(p, 0, sizeof(*p));
}
static int doc_accessible_projection_append(doc_plain_projection* p,
    doc_sequence* source, const char* bytes, uint64_t length,
    xui_doc_position_t first, xui_doc_position_t last, int literal)
{
    doc_plain_span* span; char* grown_text;
    if (!length) return XUI_OK;
    if (length > INT_MAX - p->bytes) return XUI_DOC_ERROR_LIMIT;
    if (p->bytes + length >= p->capacity) {
        uint64_t capacity = p->capacity ? p->capacity : 64;
        while (capacity <= p->bytes + length) {
            if (capacity > (uint64_t)INT_MAX / 2) {
                capacity = (uint64_t)INT_MAX + 1; break;
            }
            capacity *= 2;
        }
        grown_text = realloc(p->text, (size_t)capacity);
        if (!grown_text) return XUI_ERROR_OUT_OF_MEMORY;
        p->text = grown_text; p->capacity = capacity;
    }
    if (p->count == p->span_capacity) {
        uint64_t capacity = p->span_capacity ? p->span_capacity * 2 : 16;
        doc_plain_span* grown;
        if (capacity < p->span_capacity || capacity > SIZE_MAX / sizeof(*grown))
            return XUI_DOC_ERROR_LIMIT;
        grown = realloc(p->spans, (size_t)capacity * sizeof(*grown));
        if (!grown) return XUI_ERROR_OUT_OF_MEMORY;
        p->spans = grown; p->span_capacity = capacity;
    }
    if (source) {
        int result = doc_seq_read(source, 0, p->text + p->bytes, length);
        if (result != XUI_OK) return result;
    } else memcpy(p->text + p->bytes, bytes, (size_t)length);
    span = &p->spans[p->count++];
    span->start = p->bytes; span->length = length;
    span->first = first; span->last = last; span->literal = literal;
    p->bytes += length; p->text[p->bytes] = 0;
    return XUI_OK;
}
static int doc_accessible_edge(doc_plain_projection* p, doc_node* node,
    int end, xui_doc_position_t* out)
{
    uint64_t count = doc_seq_size(node->children);
    *out = p->origin;
    if (doc_accessible_text_leaf(node->kind) ||
        node->kind == XUI_DOC_FRONT_MATTER) {
        out->iNodeId = node->id; out->iKind = XUI_DOC_POSITION_TEXT;
        out->iOffset = end ? doc_seq_size(node->text) : 0;
    } else if (doc_selectable_object_kind(node->kind) ||
        node->kind == XUI_DOC_SOFT_BREAK ||
        node->kind == XUI_DOC_HARD_BREAK || node->kind == XUI_DOC_RULE) {
        doc_node* parent = doc_index_get(p->state->index, node->parent);
        uint64_t index;
        if (!parent || (index = doc_child_index(parent, node->id)) == DOC_NONE)
            return XUI_DOC_ERROR_SCHEMA;
        out->iNodeId = parent->id; out->iKind = XUI_DOC_POSITION_GAP;
        out->iOffset = index + (end ? 1 : 0);
    } else if (count) {
        doc_node* child = doc_index_get(p->state->index,
            doc_seq_get_id(node->children, end ? count - 1 : 0));
        if (!child) return XUI_DOC_ERROR_SCHEMA;
        return doc_accessible_edge(p, child, end, out);
    } else {
        out->iNodeId = node->id; out->iKind = XUI_DOC_POSITION_GAP;
        out->iOffset = 0;
    }
    out->iAffinity = end ? XUI_DOC_BEFORE : XUI_DOC_AFTER;
    return XUI_OK;
}
static int doc_accessible_projection_emit(doc_plain_projection* p,
    doc_node* node, doc_node* parent, uint64_t index)
{
    uint64_t i, count = doc_seq_size(node->children);
    xui_doc_position_t first = p->origin, last = p->origin;
    if (doc_accessible_text_leaf(node->kind) ||
        node->kind == XUI_DOC_FRONT_MATTER) {
        first.iNodeId = last.iNodeId = node->id;
        first.iKind = last.iKind = XUI_DOC_POSITION_TEXT;
        last.iOffset = doc_seq_size(node->text);
        return doc_accessible_projection_append(p, node->text, NULL,
            last.iOffset, first, last, 1);
    }
    if (doc_selectable_object_kind(node->kind) ||
        node->kind == XUI_DOC_RULE || node->kind == XUI_DOC_SOFT_BREAK ||
        node->kind == XUI_DOC_HARD_BREAK) {
        const char* replacement = node->kind == XUI_DOC_SOFT_BREAK ||
            node->kind == XUI_DOC_HARD_BREAK ? "\n" : "\xef\xbf\xbc";
        uint64_t length = replacement[0] == '\n' ? 1 : 3;
        if (!parent) return XUI_DOC_ERROR_SCHEMA;
        first.iNodeId = last.iNodeId = parent->id;
        first.iKind = last.iKind = XUI_DOC_POSITION_GAP;
        first.iOffset = index; last.iOffset = index + 1;
        return doc_accessible_projection_append(p, NULL, replacement,
            length, first, last, 0);
    }
    for (i = 0; i < count; i++) {
        doc_node* child = doc_index_get(p->state->index,
            doc_seq_get_id(node->children, i));
        int result;
        if (!child) return XUI_DOC_ERROR_SCHEMA;
        if (i && node->kind != XUI_DOC_PARAGRAPH && node->kind != XUI_DOC_HEADING) {
            doc_node* previous = doc_index_get(p->state->index,
                doc_seq_get_id(node->children, i - 1));
            const char* separator = node->kind == XUI_DOC_ROW ? "\t" : "\n";
            if (!previous) return XUI_DOC_ERROR_SCHEMA;
            result = doc_accessible_edge(p, previous, 1, &first);
            if (result == XUI_OK) result = doc_accessible_edge(p, child, 0, &last);
            if (result == XUI_OK) result = doc_accessible_projection_append(p,
                NULL, separator, 1, first, last, 0);
            if (result != XUI_OK) return result;
        }
        result = doc_accessible_projection_emit(p, child, node, i);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
static int doc_accessible_projection_build(xui_document_snapshot snapshot,
    doc_node* node, doc_plain_projection* out)
{
    int result;
    memset(out, 0, sizeof(*out)); out->state = snapshot->state;
    out->origin.iSize = sizeof(out->origin);
    out->origin.iDocumentId = snapshot->identity;
    out->origin.iRevision = snapshot->revision;
    out->origin.iNodeId = node->id;
    out->origin.iKind = XUI_DOC_POSITION_GAP;
    result = doc_accessible_projection_emit(out, node, NULL, 0);
    if (result == XUI_OK && !out->text) {
        out->text = malloc(1);
        if (!out->text) result = XUI_ERROR_OUT_OF_MEMORY;
        else out->text[0] = 0;
    }
    if (result != XUI_OK) doc_accessible_projection_free(out);
    return result;
}
int doc_accessible_snapshot_value(xui_document_snapshot snapshot,
    doc_node* node, char** value, uint64_t* bytes)
{
    char* text = NULL;
    uint64_t length = 0, at = 0, i;
    int result = XUI_OK;
    if (value) *value = NULL;
    if (bytes) *bytes = 0;
    if (!snapshot || !node || !value || !bytes ||
        doc_index_get(snapshot->state->index, node->id) != node)
        return XUI_ERROR_INVALID_ARGUMENT;
    if (node->kind == XUI_DOC_ROOT) {
        result = xuiDocumentSnapshotCopyPlainText(snapshot, &text, &length);
    } else if (doc_accessible_structured_container(node->kind)) {
        doc_plain_projection projection = {0};
        result = doc_accessible_projection_build(snapshot, node, &projection);
        if (result == XUI_OK) {
            text = projection.text; length = projection.bytes;
            projection.text = NULL;
        }
        doc_accessible_projection_free(&projection);
    } else if (doc_accessible_inline_container(node->kind)) {
        result = doc_accessible_inline_measure(snapshot->state, node, &length);
        if (result != XUI_OK) return result;
        text = malloc((size_t)length + 1);
        if (!text) return XUI_ERROR_OUT_OF_MEMORY;
        for (i = 0; i < doc_seq_size(node->children); i++) {
            doc_node* child; uint64_t part; const char* substitute;
            result = doc_accessible_inline_part(snapshot->state, node, i,
                &child, &part, &substitute);
            if (result != XUI_OK) break;
            if (substitute) memcpy(text + at, substitute, (size_t)part);
            else result = doc_seq_read(child->text, 0, text + at, part);
            if (result != XUI_OK) break;
            at += part;
        }
        if (result == XUI_OK) text[length] = 0;
    } else {
        length = doc_seq_size(node->text);
        if (length >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
        text = malloc((size_t)length + 1);
        if (!text) return XUI_ERROR_OUT_OF_MEMORY;
        if (length) result = doc_seq_read(node->text, 0, text, length);
        if (result == XUI_OK) text[length] = 0;
    }
    if (result == XUI_OK && memchr(text, 0, (size_t)length) != NULL)
        result = XUI_DOC_ERROR_UNREPRESENTABLE;
    if (result != XUI_OK) { free(text); return result; }
    *value = text; *bytes = length;
    return XUI_OK;
}
int doc_accessible_snapshot_selection_range(xui_document_snapshot snapshot,
    doc_node* node, const xui_accessible_selection_t* selected,
    xui_doc_range_t* range)
{
    uint64_t bytes, first, last;
    int result;
    if (!snapshot || !node || !range) return XUI_ERROR_INVALID_ARGUMENT;
    if (selected && (selected->iSize < sizeof(*selected) ||
        selected->iAnchor < 0 || selected->iCaret < 0))
        return XUI_ERROR_INVALID_ARGUMENT;
    memset(range, 0, sizeof(*range));
    if (doc_accessible_text_leaf(node->kind) ||
        doc_accessible_inline_container(node->kind)) {
        result = doc_accessible_inline_container(node->kind) ?
            doc_accessible_inline_measure(snapshot->state, node, &bytes) : XUI_OK;
        if (result != XUI_OK) return result;
        if (!doc_accessible_inline_container(node->kind)) bytes = doc_seq_size(node->text);
        if (bytes > INT_MAX) return XUI_DOC_ERROR_LIMIT;
        if (selected && ((uint64_t)selected->iAnchor > bytes ||
            (uint64_t)selected->iCaret > bytes)) return XUI_ERROR_INVALID_ARGUMENT;
        first = selected ? (uint64_t)selected->iAnchor : 0;
        last = selected ? (uint64_t)selected->iCaret : bytes;
        if (selected && (doc_accessible_inline_container(node->kind) ?
            (!doc_accessible_inline_boundary(snapshot->state, node, first) ||
             !doc_accessible_inline_boundary(snapshot->state, node, last)) :
            (!doc_seq_boundary(node->text, first) ||
             !doc_seq_boundary(node->text, last))))
            return XUI_DOC_ERROR_UTF8;
        if (doc_accessible_inline_container(node->kind)) {
            result = doc_accessible_inline_unproject(snapshot, node,
                first, first <= last, &range->tAnchor);
            if (result == XUI_OK) result = doc_accessible_inline_unproject(snapshot,
                node, last, last < first, &range->tCaret);
            if (result != XUI_OK) return result;
        } else {
            range->tAnchor.iSize = sizeof(range->tAnchor);
            range->tAnchor.iDocumentId = snapshot->identity;
            range->tAnchor.iRevision = snapshot->revision;
            range->tAnchor.iNodeId = node->id;
            range->tAnchor.iKind = XUI_DOC_POSITION_TEXT;
            range->tAnchor.iOffset = first;
            range->tAnchor.iAffinity = XUI_DOC_AFTER;
            range->tCaret = range->tAnchor;
            range->tCaret.iOffset = last;
            range->tCaret.iAffinity = last >= first ? XUI_DOC_BEFORE : XUI_DOC_AFTER;
        }
        if (first == last) range->tCaret = range->tAnchor;
        return XUI_OK;
    }
    if (doc_accessible_structured_container(node->kind)) {
        doc_plain_projection projection = {0};
        result = doc_accessible_projection_build(snapshot, node, &projection);
        if (result != XUI_OK) return result;
        first = selected ? (uint64_t)selected->iAnchor : 0;
        last = selected ? (uint64_t)selected->iCaret : projection.bytes;
        if (projection.bytes > INT_MAX) result = XUI_DOC_ERROR_LIMIT;
        else if (first > projection.bytes || last > projection.bytes)
            result = XUI_ERROR_INVALID_ARGUMENT;
        else if ((first < projection.bytes &&
                ((unsigned char)projection.text[first] & 0xc0) == 0x80) ||
            (last < projection.bytes &&
                ((unsigned char)projection.text[last] & 0xc0) == 0x80))
            result = XUI_DOC_ERROR_UTF8;
        if (result == XUI_OK) {
            range->tAnchor = doc_plain_unproject(&projection, first, first <= last);
            range->tCaret = doc_plain_unproject(&projection, last, last < first);
            if (first == last) range->tCaret = range->tAnchor;
        }
        doc_accessible_projection_free(&projection);
        return result;
    }
    return XUI_ERROR_UNSUPPORTED;
}
static int doc_accessible_structured_selection_offsets(
    const doc_plain_projection* projection, doc_node* node,
    const xui_doc_range_t* range, int* anchor, int* caret, int* selected)
{
    doc_state* state = projection->state;
    xui_doc_position_t start = projection->origin, end = start;
    const xui_doc_position_t *first, *last;
    uint64_t a, c;
    int direction, before_end, after_start, result;
    if (projection->bytes > INT_MAX) return XUI_DOC_ERROR_LIMIT;
    end.iOffset = doc_seq_size(node->children);
    result = doc_position_compare(state, &range->tAnchor,
        &range->tCaret, &direction);
    if (result != XUI_OK) return result;
    if (!direction) {
        result = doc_position_compare(state, &range->tCaret,
            &start, &before_end);
        if (result == XUI_OK) result = doc_position_compare(state,
            &range->tCaret, &end, &after_start);
        if (result != XUI_OK) return result;
        if (before_end >= 0 && after_start <= 0) {
            result = doc_plain_project_position(projection, &range->tCaret, &c);
            if (result != XUI_OK) return result;
            *anchor = *caret = (int)c;
        }
        return XUI_OK;
    }
    first = direction < 0 ? &range->tAnchor : &range->tCaret;
    last = direction < 0 ? &range->tCaret : &range->tAnchor;
    result = doc_position_compare(state, first, &end, &before_end);
    if (result == XUI_OK) result = doc_position_compare(state,
        last, &start, &after_start);
    if (result != XUI_OK) return result;
    if (before_end >= 0 || after_start <= 0) return XUI_OK;
    result = doc_plain_project_position(projection, &range->tAnchor, &a);
    if (result == XUI_OK) result = doc_plain_project_position(projection,
        &range->tCaret, &c);
    if (result != XUI_OK) return result;
    *anchor = (int)a;
    *caret = (int)c;
    *selected = 1;
    return XUI_OK;
}
int doc_accessible_snapshot_selection_offsets(xui_document_snapshot snapshot,
    doc_node* node, const xui_doc_range_t* range,
    int* anchor, int* caret, int* selected)
{
    xui_doc_position_t start, end;
    const xui_doc_position_t *first, *last;
    uint64_t bytes, a, c;
    int direction, before_end, after_start, result;
    if (!snapshot || !node || !range || !anchor || !caret || !selected)
        return XUI_ERROR_INVALID_ARGUMENT;
    *anchor = *caret = *selected = 0;
    if (doc_accessible_structured_container(node->kind)) {
        doc_plain_projection projection = {0};
        result = doc_accessible_projection_build(snapshot, node, &projection);
        if (result != XUI_OK) return result;
        result = doc_accessible_structured_selection_offsets(&projection,
            node, range, anchor, caret, selected);
        doc_accessible_projection_free(&projection);
        return result;
    }
    if (!doc_accessible_text_leaf(node->kind) &&
        !doc_accessible_inline_container(node->kind)) return XUI_ERROR_UNSUPPORTED;
    result = doc_accessible_inline_container(node->kind) ?
        doc_accessible_inline_measure(snapshot->state, node, &bytes) : XUI_OK;
    if (result != XUI_OK) return result;
    if (!doc_accessible_inline_container(node->kind)) bytes = doc_seq_size(node->text);
    if (bytes > INT_MAX) return XUI_DOC_ERROR_LIMIT;
    start = end = range->tAnchor;
    start.iNodeId = end.iNodeId = node->id;
    start.iKind = end.iKind = doc_accessible_text_leaf(node->kind) ?
        XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
    start.iOffset = 0;
    end.iOffset = doc_accessible_text_leaf(node->kind) ?
        bytes : doc_seq_size(node->children);
    result = doc_position_compare(snapshot->state, &range->tAnchor,
        &range->tCaret, &direction);
    if (result != XUI_OK) return result;
    if (!direction) {
        result = doc_position_compare(snapshot->state, &range->tCaret,
            &start, &before_end);
        if (result == XUI_OK) result = doc_position_compare(snapshot->state,
            &range->tCaret, &end, &after_start);
        if (result != XUI_OK) return result;
        if (before_end < 0 || after_start > 0) return XUI_OK;
        if (doc_accessible_text_leaf(node->kind)) {
            if (range->tCaret.iKind == XUI_DOC_POSITION_TEXT &&
                range->tCaret.iNodeId == node->id)
                *anchor = *caret = (int)range->tCaret.iOffset;
        } else {
            result = doc_accessible_inline_project(snapshot->state, node,
                &range->tCaret, bytes, &c);
            if (result != XUI_OK) return result;
            *anchor = *caret = (int)c;
        }
        return XUI_OK;
    }
    first = direction < 0 ? &range->tAnchor : &range->tCaret;
    last = direction < 0 ? &range->tCaret : &range->tAnchor;
    result = doc_position_compare(snapshot->state, first, &end, &before_end);
    if (result == XUI_OK) result = doc_position_compare(snapshot->state,
        last, &start, &after_start);
    if (result != XUI_OK) return result;
    if (before_end >= 0 || after_start <= 0) return XUI_OK;
    if (doc_accessible_inline_container(node->kind)) {
        result = doc_accessible_inline_project(snapshot->state, node,
            &range->tAnchor, bytes, &a);
        if (result == XUI_OK) result = doc_accessible_inline_project(snapshot->state,
            node, &range->tCaret, bytes, &c);
        if (result != XUI_OK) return result;
    } else {
        int a_start, a_end, c_start, c_end;
        result = doc_position_compare(snapshot->state, &range->tAnchor,
            &start, &a_start);
        if (result == XUI_OK) result = doc_position_compare(snapshot->state,
            &range->tAnchor, &end, &a_end);
        if (result == XUI_OK) result = doc_position_compare(snapshot->state,
            &range->tCaret, &start, &c_start);
        if (result == XUI_OK) result = doc_position_compare(snapshot->state,
            &range->tCaret, &end, &c_end);
        if (result != XUI_OK) return result;
        a = a_start <= 0 ? 0 : a_end >= 0 ? bytes : range->tAnchor.iOffset;
        c = c_start <= 0 ? 0 : c_end >= 0 ? bytes : range->tCaret.iOffset;
    }
    *anchor = (int)a;
    *caret = (int)c;
    *selected = 1;
    return XUI_OK;
}
int doc_accessible_snapshot_object_range(xui_document_snapshot snapshot,
    doc_node* node, xui_doc_range_t* range)
{
    doc_node* parent;
    uint64_t index;
    xui_doc_position_t edge = {0};
    if (!snapshot || !node || !range) return XUI_ERROR_INVALID_ARGUMENT;
    if (!doc_selectable_object_kind(node->kind)) return XUI_ERROR_UNSUPPORTED;
    parent = doc_index_get(snapshot->state->index, node->parent);
    if (!parent || (index = doc_child_index(parent, node->id)) == DOC_NONE)
        return XUI_DOC_ERROR_SCHEMA;
    edge.iSize = sizeof(edge);
    edge.iDocumentId = snapshot->identity;
    edge.iRevision = snapshot->revision;
    edge.iNodeId = parent->id;
    edge.iKind = XUI_DOC_POSITION_GAP;
    edge.iOffset = index;
    edge.iAffinity = XUI_DOC_AFTER;
    range->tAnchor = range->tCaret = edge;
    range->tCaret.iOffset++;
    return XUI_OK;
}
int doc_accessible_snapshot_object_selected(xui_document_snapshot snapshot,
    doc_node* node, const xui_doc_range_t* selection, int* selected)
{
    xui_doc_range_t object;
    const xui_doc_position_t *first, *last;
    int direction, before_end, after_start, result;
    if (!snapshot || !node || !selection || !selected)
        return XUI_ERROR_INVALID_ARGUMENT;
    *selected = 0;
    result = doc_accessible_snapshot_object_range(snapshot, node, &object);
    if (result != XUI_OK) return result;
    result = doc_position_compare(snapshot->state, &selection->tAnchor,
        &selection->tCaret, &direction);
    if (result != XUI_OK || !direction) return XUI_OK;
    first = direction < 0 ? &selection->tAnchor : &selection->tCaret;
    last = direction < 0 ? &selection->tCaret : &selection->tAnchor;
    result = doc_position_compare(snapshot->state, first,
        &object.tCaret, &before_end);
    if (result != XUI_OK) return XUI_OK;
    result = doc_position_compare(snapshot->state, last,
        &object.tAnchor, &after_start);
    if (result == XUI_OK && before_end < 0 && after_start > 0)
        *selected = 1;
    return XUI_OK;
}
static int doc_view_accessible_structured_state(doc_view_data* v,
    doc_node* node, const doc_plain_projection* projection,
    xui_accessible_node_t* out)
{
    int selected = 0, result;
    if (!v->desc.bDisableSelection) {
        out->iState |= XUI_ACCESSIBLE_STATE_SELECTABLE;
        out->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION);
    }
    result = doc_accessible_structured_selection_offsets(projection,
        node, &v->selection, &out->iTextStart, &out->iTextEnd, &selected);
    if (result == XUI_OK && selected) out->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
    return result;
}
int doc_accessible_role(const doc_node* node)
{
    switch (node->kind) {
    case XUI_DOC_ROOT: return XUI_ACCESSIBLE_ROLE_DOCUMENT;
    case XUI_DOC_PARAGRAPH: return XUI_ACCESSIBLE_ROLE_PARAGRAPH;
    case XUI_DOC_HEADING: return XUI_ACCESSIBLE_ROLE_HEADING;
    case XUI_DOC_CODE_BLOCK: return XUI_ACCESSIBLE_ROLE_CODE_BLOCK;
    case XUI_DOC_TEXT: return node->attrs->iMarks & XUI_DOC_LINK ? XUI_ACCESSIBLE_ROLE_LINK : XUI_ACCESSIBLE_ROLE_TEXT;
    case XUI_DOC_QUOTE: return XUI_ACCESSIBLE_ROLE_BLOCK_QUOTE;
    case XUI_DOC_LIST: return XUI_ACCESSIBLE_ROLE_LIST;
    case XUI_DOC_LIST_ITEM: return node->attrs->iFlags & XUI_DOC_TASK ? XUI_ACCESSIBLE_ROLE_CHECKBOX : XUI_ACCESSIBLE_ROLE_LIST_ITEM;
    case XUI_DOC_TABLE: return XUI_ACCESSIBLE_ROLE_TABLE;
    case XUI_DOC_ROW: return XUI_ACCESSIBLE_ROLE_GROUP;
    case XUI_DOC_CELL: return XUI_ACCESSIBLE_ROLE_CELL;
    case XUI_DOC_IMAGE: return XUI_ACCESSIBLE_ROLE_IMAGE;
    case XUI_DOC_RULE: return XUI_ACCESSIBLE_ROLE_SEPARATOR;
    case XUI_DOC_MATH: case XUI_DOC_DIAGRAM: case XUI_DOC_HTML:
    case XUI_DOC_EXTENSION: return XUI_ACCESSIBLE_ROLE_EMBEDDED_OBJECT;
    default: return XUI_ACCESSIBLE_ROLE_GENERIC;
    }
}
static void doc_view_accessible_cell_state(doc_view_data* v,
    xui_accessible_node_t* out, uint64_t table, uint32_t row,
    uint32_t column, uint32_t row_span, uint32_t column_span)
{
    const xui_doc_table_selection_t* selected = &v->table_selection;
    out->iRow = row > INT_MAX ? INT_MAX : (int)row;
    out->iColumn = column > INT_MAX ? INT_MAX : (int)column;
    out->iRowCount = row_span > INT_MAX ? INT_MAX : (int)row_span;
    out->iColumnCount = column_span > INT_MAX ? INT_MAX : (int)column_span;
    if (v->desc.bDisableSelection) return;
    out->iState |= XUI_ACCESSIBLE_STATE_SELECTABLE;
    out->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION);
    if (selected->iTableId == table && row >= selected->iRow &&
        column >= selected->iColumn &&
        (uint64_t)row + row_span <= (uint64_t)selected->iRow + selected->iRows &&
        (uint64_t)column + column_span <= (uint64_t)selected->iColumn + selected->iColumns)
        out->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
}
static void doc_view_accessible_text_state(doc_view_data* v,
    doc_node* node, xui_accessible_node_t* out)
{
    doc_state* state = v->renderer->snapshot->state;
    xui_doc_position_t start = v->selection.tAnchor, end = start;
    const xui_doc_position_t *first, *last;
    uint64_t bytes = doc_seq_size(node->text);
    int direction, before_end, after_start, anchor_start, anchor_end;
    int caret_start, caret_end;
    if (bytes > INT_MAX) return; /* The accessibility range ABI uses int offsets. */
    if (!v->desc.bDisableSelection) {
        out->iState |= XUI_ACCESSIBLE_STATE_SELECTABLE;
        out->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION);
    }
    start.iNodeId = end.iNodeId = node->id;
    start.iKind = end.iKind = XUI_DOC_POSITION_TEXT;
    start.iOffset = 0; end.iOffset = bytes;
    if (doc_position_compare(state, &v->selection.tAnchor,
            &v->selection.tCaret, &direction) != XUI_OK) return;
    if (!direction) {
        if (v->selection.tCaret.iKind == XUI_DOC_POSITION_TEXT &&
            v->selection.tCaret.iNodeId == node->id)
            out->iTextStart = out->iTextEnd = (int)v->selection.tCaret.iOffset;
        return;
    }
    first = direction < 0 ? &v->selection.tAnchor : &v->selection.tCaret;
    last = direction < 0 ? &v->selection.tCaret : &v->selection.tAnchor;
    if (doc_position_compare(state, first, &end, &before_end) != XUI_OK ||
        doc_position_compare(state, last, &start, &after_start) != XUI_OK ||
        before_end >= 0 || after_start <= 0) return;
    out->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
    if (doc_position_compare(state, &v->selection.tAnchor, &start, &anchor_start) != XUI_OK ||
        doc_position_compare(state, &v->selection.tAnchor, &end, &anchor_end) != XUI_OK ||
        doc_position_compare(state, &v->selection.tCaret, &start, &caret_start) != XUI_OK ||
        doc_position_compare(state, &v->selection.tCaret, &end, &caret_end) != XUI_OK) return;
    out->iTextStart = anchor_start <= 0 ? 0 : anchor_end >= 0 ? (int)bytes :
        (int)v->selection.tAnchor.iOffset;
    out->iTextEnd = caret_start <= 0 ? 0 : caret_end >= 0 ? (int)bytes :
        (int)v->selection.tCaret.iOffset;
}
static int doc_view_accessible_inline_state(doc_view_data* v,
    doc_node* parent, uint64_t bytes, xui_accessible_node_t* out)
{
    doc_state* state = v->renderer->snapshot->state;
    xui_doc_position_t start = v->selection.tAnchor, end = start;
    const xui_doc_position_t *first, *last;
    uint64_t anchor, caret; int direction, before_end, after_start, result;
    if (!v->desc.bDisableSelection) {
        out->iState |= XUI_ACCESSIBLE_STATE_SELECTABLE;
        out->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION);
    }
    start.iNodeId = end.iNodeId = parent->id;
    start.iKind = end.iKind = XUI_DOC_POSITION_GAP;
    start.iOffset = 0; end.iOffset = doc_seq_size(parent->children);
    result = doc_position_compare(state, &v->selection.tAnchor,
        &v->selection.tCaret, &direction);
    if (result != XUI_OK) return result;
    if (!direction) {
        if (doc_position_compare(state, &v->selection.tCaret,
                &start, &before_end) != XUI_OK ||
            doc_position_compare(state, &v->selection.tCaret,
                &end, &after_start) != XUI_OK) return XUI_ERROR_INVALID_ARGUMENT;
        if (before_end >= 0 && after_start <= 0) {
            result = doc_accessible_inline_project(state, parent,
                &v->selection.tCaret, bytes, &caret);
            if (result != XUI_OK) return result;
            out->iTextStart = out->iTextEnd = (int)caret;
        }
        return XUI_OK;
    }
    first = direction < 0 ? &v->selection.tAnchor : &v->selection.tCaret;
    last = direction < 0 ? &v->selection.tCaret : &v->selection.tAnchor;
    if (doc_position_compare(state, first, &end, &before_end) != XUI_OK ||
        doc_position_compare(state, last, &start, &after_start) != XUI_OK)
        return XUI_ERROR_INVALID_ARGUMENT;
    if (before_end >= 0 || after_start <= 0) return XUI_OK;
    result = doc_accessible_inline_project(state, parent,
        &v->selection.tAnchor, bytes, &anchor);
    if (result == XUI_OK) result = doc_accessible_inline_project(state, parent,
        &v->selection.tCaret, bytes, &caret);
    if (result != XUI_OK) return result;
    out->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
    out->iTextStart = (int)anchor; out->iTextEnd = (int)caret;
    return XUI_OK;
}
static int doc_view_accessible_count(xui_widget w, void* user)
{
    doc_view_data* v = user; (void)w;
    if (!v) return 0;
    if (v->renderer->mode != XUI_DOC_VISUAL) return 1;
    return doc_view_accessible_index(v) == XUI_OK ? (int)v->accessible_count : 0;
}
static int doc_view_accessible_get(xui_widget w, int index, xui_accessible_node_t* out, void* user)
{
    doc_view_data* v = user; xui_document_snapshot snapshot; doc_node* node;
    xui_rect_t world, content; xui_doc_rect_t rect; uint64_t id;
    const char* text; uint64_t inline_bytes = 0; int result, has_bounds;
    if (!v || index < 0 || !out) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->renderer->mode != XUI_DOC_VISUAL) {
        if (index != 0) return XUI_ERROR_NOT_FOUND;
        out->iId = DOC_ROOT; out->tBounds = xuiWidgetGetWorldRect(w);
        result = xuiInternalAccessibilityEditNode(w, out);
        if (result == XUI_OK && v->desc.bDisableSelection)
            out->iActions &= ~XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION);
        return result;
    }
    result = doc_view_accessible_index(v); if (result != XUI_OK) return result;
    if ((uint64_t)index >= v->accessible_count) return XUI_ERROR_NOT_FOUND;
    snapshot = v->renderer->snapshot; id = v->accessible_ids[index];
    node = doc_index_get(snapshot->state->index, id);
    if (!node) return XUI_DOC_ERROR_STALE;
    out->iId = id; out->iParentId = node->parent;
    out->iRole = doc_accessible_role(node);
    out->sName = ""; out->sDescription = doc_string(node->title);
    if (xuiEditIsReadonly(w)) out->iState |= XUI_ACCESSIBLE_STATE_READONLY;
    else out->iState |= XUI_ACCESSIBLE_STATE_EDITABLE;
    if (id == DOC_ROOT) {
        out->sName = xuiWidgetGetAccessibleName(w) ? xuiWidgetGetAccessibleName(w) : "Document";
        out->sDescription = xuiWidgetGetAccessibleDescription(w);
        out->tBounds = xuiWidgetGetWorldRect(w);
        result = doc_view_project_edit(v);
        if (result != XUI_OK) return result;
        if (v->edit_projection.bytes > INT_MAX) return XUI_DOC_ERROR_LIMIT;
        out->sValue = v->edit_projection.text;
        result = xuiEditGetSelection(w, &out->iTextStart, &out->iTextEnd);
        if (result != XUI_OK) return result;
        if (!v->desc.bDisableSelection) {
            out->iState |= XUI_ACCESSIBLE_STATE_SELECTABLE;
            out->iActions = XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION);
            if (out->iTextStart != out->iTextEnd)
                out->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
        }
        if (!xuiEditIsReadonly(w)) out->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_VALUE);
        return XUI_OK;
    }
    if (doc_accessible_structured_container(node->kind)) {
        doc_plain_projection projection = {0};
        result = doc_accessible_projection_build(snapshot, node, &projection);
        if (result != XUI_OK) return result;
        if (!v->accessible_text[index]) {
            v->accessible_text[index] = projection.text;
            projection.text = NULL;
        }
        out->sValue = v->accessible_text[index];
        result = doc_view_accessible_structured_state(v, node, &projection, out);
        doc_accessible_projection_free(&projection);
        if (result != XUI_OK) return result;
    } else {
        if (doc_accessible_inline_container(node->kind)) {
            result = doc_view_accessible_inline_text(v, (uint64_t)index,
                node, &text, &inline_bytes);
            if (result != XUI_OK) return result;
        } else {
            text = doc_view_accessible_text(v, (uint64_t)index, node);
            if (!text) return XUI_ERROR_OUT_OF_MEMORY;
        }
        if (doc_accessible_text_leaf(node->kind)) {
            out->sValue = text;
            if (node->kind == XUI_DOC_TEXT && (node->attrs->iMarks & XUI_DOC_LINK)) {
                out->sName = text; out->sDescription = doc_string(node->resource);
            }
            doc_view_accessible_text_state(v, node, out);
        } else if (doc_accessible_inline_container(node->kind)) {
            out->sValue = text;
            result = doc_view_accessible_inline_state(v, node, inline_bytes, out);
            if (result != XUI_OK) return result;
        } else if (node->kind == XUI_DOC_IMAGE) {
            out->sName = text;
            out->sDescription = node->attrs->iMarks & XUI_DOC_LINK ?
                doc_string(node->link_target) : doc_string(node->resource);
        }
        else if (doc_seq_size(node->text)) out->sValue = text;
    }
    if (doc_selectable_object_kind(node->kind)) {
        int selected = 0;
        if (!v->desc.bDisableSelection) {
            out->iState |= XUI_ACCESSIBLE_STATE_SELECTABLE;
            out->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION);
        }
        result = doc_accessible_snapshot_object_selected(snapshot,
            node, &v->selection, &selected);
        if (result != XUI_OK) return result;
        if (selected) out->iState |= XUI_ACCESSIBLE_STATE_SELECTED;
    }
    if (node->kind == XUI_DOC_HEADING) out->iLevel = (int)node->attrs->iHeadingLevel;
    if (node->kind == XUI_DOC_LIST_ITEM && (node->attrs->iFlags & XUI_DOC_TASK)) {
        if (node->attrs->iFlags & XUI_DOC_CHECKED) out->iState |= XUI_ACCESSIBLE_STATE_CHECKED;
        if (!xuiEditIsReadonly(w)) out->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE);
    }
    if (node->kind == XUI_DOC_TABLE) {
        doc_node* row = NULL; uint64_t columns = 0, i;
        uint64_t rows = doc_seq_size(node->children);
        out->iRowCount = rows > INT_MAX ? INT_MAX : (int)rows;
        if (rows) row = doc_index_get(snapshot->state->index, doc_seq_get_id(node->children, 0));
        if (row) for (i = 0; i < doc_seq_size(row->children); i++) {
            doc_node* cell = doc_index_get(snapshot->state->index, doc_seq_get_id(row->children, i));
            if (cell) columns += cell->attrs->iColumnSpan ? cell->attrs->iColumnSpan : 1;
        }
        out->iColumnCount = columns > INT_MAX ? INT_MAX : (int)columns;
    }
    result = doc_view_layout(v); if (result != XUI_OK) return result;
    result = xuiDocumentRendererGetNodeRect(v->renderer, id, &rect);
    if (result != XUI_OK && result != XUI_ERROR_NOT_FOUND) return result;
    has_bounds = result == XUI_OK;
    world = xuiWidgetGetWorldRect(w); content = xuiWidgetGetContentRect(w);
    if (result == XUI_OK) out->tBounds = (xui_rect_t){(float)(world.fX + content.fX + rect.x - v->scroll_x),
        (float)(world.fY + content.fY + rect.y - v->scroll_y), (float)rect.width, (float)rect.height};
    else out->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
    if (node->kind == XUI_DOC_CELL) {
        xui_doc_cell_hit_t cell = {0}; cell.iSize = sizeof(cell);
        int cell_result = xuiDocumentRendererGetCellRect(v->renderer, id, &cell);
        if (cell_result == XUI_OK) {
            doc_view_accessible_cell_state(v, out, cell.iTableId,
                cell.iRow, cell.iColumn, cell.iRowSpan, cell.iColumnSpan);
            out->tBounds = (xui_rect_t){(float)(world.fX + content.fX + cell.tBounds.x - v->scroll_x),
                (float)(world.fY + content.fY + cell.tBounds.y - v->scroll_y),
                (float)cell.tBounds.width, (float)cell.tBounds.height};
            out->iState &= ~XUI_ACCESSIBLE_STATE_OFFSCREEN;
            has_bounds = 1;
        } else if (cell_result == XUI_ERROR_NOT_FOUND) {
            doc_table_cell_slot slot;
            int locate = doc_table_locate_cell(snapshot->state, id, &slot);
            if (locate != XUI_OK) return locate;
            doc_view_accessible_cell_state(v, out, slot.table,
                slot.row, slot.column, slot.row_span, slot.column_span);
        } else return cell_result;
    }
    if (out->tBounds.fX + out->tBounds.fW <= world.fX + content.fX ||
        out->tBounds.fX >= world.fX + content.fX + content.fW ||
        out->tBounds.fY + out->tBounds.fH <= world.fY + content.fY ||
        out->tBounds.fY >= world.fY + content.fY + content.fH)
        out->iState |= XUI_ACCESSIBLE_STATE_OFFSCREEN;
    if (has_bounds)
        out->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW);
    if (v->desc.onActivate && (node->kind == XUI_DOC_IMAGE ||
        (node->kind == XUI_DOC_TEXT && (node->attrs->iMarks & XUI_DOC_LINK)) ||
        node->kind == XUI_DOC_MATH || node->kind == XUI_DOC_DIAGRAM || node->kind == XUI_DOC_HTML))
        out->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE);
    return XUI_OK;
}
static int doc_view_accessible_action(xui_widget w, uint64_t id, int action, const void* data, void* user)
{
    doc_view_data* v = user; doc_node* node; int result;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    if (action == XUI_ACCESSIBLE_ACTION_FOCUS)
        return xuiSetFocusWidget(xuiWidgetGetContext(w), w);
    if (action == XUI_ACCESSIBLE_ACTION_SET_SELECTION && v->desc.bDisableSelection)
        return XUI_ERROR_UNSUPPORTED;
    if ((id == 0 || id == DOC_ROOT) && (action == XUI_ACCESSIBLE_ACTION_SET_SELECTION ||
        action == XUI_ACCESSIBLE_ACTION_SET_VALUE))
        return xuiInternalAccessibilityEditAction(w, action, data);
    if (v->renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    node = doc_index_get(v->renderer->snapshot->state->index, id);
    if (!node || id == DOC_ROOT) return XUI_ERROR_NOT_FOUND;
    if (action == XUI_ACCESSIBLE_ACTION_SET_SELECTION &&
        (doc_accessible_text_leaf(node->kind) ||
         doc_accessible_inline_container(node->kind) ||
         (doc_accessible_structured_container(node->kind) &&
          (node->kind != XUI_DOC_CELL || data)))) {
        xui_doc_range_t range = {0};
        uint32_t kind = node->kind;
        const xui_accessible_selection_t* selected = data;
        if (selected && (selected->iSize < sizeof(*selected) ||
            selected->iAnchor < 0 || selected->iCaret < 0))
            return XUI_ERROR_INVALID_ARGUMENT;
        if (v->onBeforeSelection && (result = v->onBeforeSelection(w)) != XUI_OK)
            return result;
        if (!xuiInternalWidgetIsValid(w)) return XUI_ERROR_INVALID_STATE;
        result = doc_view_sync(v); if (result != XUI_OK) return result;
        node = doc_index_get(v->renderer->snapshot->state->index, id);
        if (!node || node->kind != kind) return XUI_DOC_ERROR_STALE;
        result = doc_accessible_snapshot_selection_range(v->renderer->snapshot,
            node, selected, &range);
        return result == XUI_OK ? doc_view_set_selection(v, &range) : result;
    }
    if (action == XUI_ACCESSIBLE_ACTION_SET_SELECTION &&
        doc_selectable_object_kind(node->kind))
        return xuiDocumentViewSelectObject(w, id);
    if (action == XUI_ACCESSIBLE_ACTION_SET_SELECTION && node->kind == XUI_DOC_CELL) {
        doc_table_cell_slot slot;
        xui_doc_table_selection_t selected = {0};
        result = doc_table_locate_cell(v->renderer->snapshot->state, id, &slot);
        if (result != XUI_OK) return result;
        selected.iSize = sizeof(selected); selected.iTableId = slot.table;
        selected.iRow = slot.row; selected.iColumn = slot.column;
        selected.iRows = slot.row_span; selected.iColumns = slot.column_span;
        return xuiDocumentViewSetTableSelection(w, &selected);
    }
    if (action == XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW) {
        xui_doc_rect_t rect; xui_rect_t content; double x, y;
        result = doc_view_layout(v); if (result != XUI_OK) return result;
        if (node->kind == XUI_DOC_CELL) {
            xui_doc_cell_hit_t cell = {0}; cell.iSize = sizeof(cell);
            result = xuiDocumentRendererGetCellRect(v->renderer, id, &cell);
            if (result == XUI_OK) rect = cell.tBounds;
        } else result = xuiDocumentRendererGetNodeRect(v->renderer, id, &rect);
        if (result != XUI_OK) return result;
        content = xuiWidgetGetContentRect(w); x = v->scroll_x; y = v->scroll_y;
        if (rect.x < x) x = rect.x;
        else if (rect.x + rect.width > x + content.fW) x = rect.x + rect.width - content.fW;
        if (rect.y < y) y = rect.y;
        else if (rect.y + rect.height > y + content.fH) y = rect.y + rect.height - content.fH;
        return xuiDocumentViewSetScroll(w, x, y);
    }
    if (action == XUI_ACCESSIBLE_ACTION_ACTIVATE && v->desc.onActivate &&
        (node->kind == XUI_DOC_IMAGE || (node->kind == XUI_DOC_TEXT &&
        (node->attrs->iMarks & XUI_DOC_LINK)) || node->kind == XUI_DOC_MATH ||
        node->kind == XUI_DOC_DIAGRAM || node->kind == XUI_DOC_HTML)) {
        xui_document_snapshot retained = v->renderer->snapshot;
        const char* resource = node->kind == XUI_DOC_IMAGE &&
            (node->attrs->iMarks & XUI_DOC_LINK) ?
            doc_string(node->link_target) : doc_string(node->resource);
        xuiDocumentSnapshotRetain(retained);
        v->desc.onActivate(w, id, resource, v->desc.pUser);
        xuiDocumentSnapshotRelease(retained);
        return XUI_OK;
    }
    if (action == XUI_ACCESSIBLE_ACTION_TOGGLE && node->kind == XUI_DOC_LIST_ITEM &&
        (node->attrs->iFlags & XUI_DOC_TASK)) {
        if (xuiEditIsReadonly(w)) return XUI_ERROR_INVALID_STATE;
        return xuiDocumentEditorToggleTaskItem(w, id);
    }
    return XUI_ERROR_UNSUPPORTED;
}
int doc_view_register_edit(xui_widget w, int editable)
{
    static const xui_internal_accessibility_adapter_t accessible = {xuiInternalAccessibilityEditNode, xuiInternalAccessibilityEditAction};
    static const xui_internal_edit_adapter_t view = {
        .iCapabilities = XUI_EDIT_CAP_TEXT | XUI_EDIT_CAP_SELECTION | XUI_EDIT_CAP_CLIPBOARD | XUI_EDIT_CAP_READONLY | XUI_EDIT_CAP_CARET_RECT | XUI_EDIT_CAP_MULTILINE | XUI_EDIT_CAP_STRUCTURED,
        .getText = doc_view_edit_text, .setSelection = doc_view_edit_set_selection, .getSelection = doc_view_edit_get_selection,
        .hasSelection = doc_view_edit_has_selection, .selectAll = doc_view_edit_select_all, .copy = doc_view_edit_copy,
        .isReadonly = doc_view_read_only, .getCaretRect = doc_view_edit_caret
    };
    static const xui_internal_edit_adapter_t editor = {
        .iCapabilities = XUI_EDIT_CAP_TEXT | XUI_EDIT_CAP_SELECTION | XUI_EDIT_CAP_CLIPBOARD | XUI_EDIT_CAP_UNDO | XUI_EDIT_CAP_READONLY | XUI_EDIT_CAP_CARET_RECT | XUI_EDIT_CAP_IME | XUI_EDIT_CAP_MULTILINE | XUI_EDIT_CAP_STRUCTURED,
        .setText = doc_editor_edit_set_text, .getText = doc_view_edit_text, .setSelection = doc_view_edit_set_selection, .getSelection = doc_view_edit_get_selection,
        .hasSelection = doc_view_edit_has_selection, .selectAll = doc_view_edit_select_all, .copy = doc_editor_edit_copy,
        .cut = doc_editor_edit_cut, .paste = doc_editor_edit_paste, .deleteSelection = doc_editor_edit_delete,
        .undo = doc_editor_edit_undo, .redo = doc_editor_edit_redo, .canUndo = doc_editor_edit_can_undo, .canRedo = doc_editor_edit_can_redo,
        .setReadonly = xuiDocumentEditorSetReadOnly, .isReadonly = xuiDocumentEditorGetReadOnly, .getCaretRect = doc_view_edit_caret
    };
    int result;
    w->pType->pAccessibleAdapter = &accessible;
    result = xuiInternalEditRegister(w, editable ? &editor : &view, NULL);
    if (result == XUI_OK) result = xuiWidgetSetAccessibilityProvider(w,
        doc_view_accessible_count, doc_view_accessible_get,
        doc_view_accessible_action, doc_view_get(w));
    return result;
}
int doc_view_update(xui_widget w, float dt, void* user)
{
    doc_view_data* v = doc_view_get(w); unsigned events; const char* text; char* retained = NULL; int a = 0, b = 0; xui_widget host;
    (void)user;
    if (v) {
        int result = doc_view_refresh_resources(v);
        if (result != XUI_OK) return result;
        result = doc_view_drag_autoscroll(v, fminf(fmaxf(dt, 0), .05f));
        if (result != XUI_OK) return result;
    }
    if (!v || !v->pending_edit_events) return XUI_OK;
    events = v->pending_edit_events; v->pending_edit_events = 0;
    host = xuiInternalEditHost(w);
    /* Queue accessibility notifications without flattening an unread document.
     * The compatibility text projection is built only for an actual listener. */
    text = host && host->onEditEvent ? doc_view_edit_text(w) : NULL;
    if (text) {
        size_t length = strlen(text);
        retained = malloc(length + 1); if (!retained) { v->pending_edit_events |= events; return XUI_ERROR_OUT_OF_MEMORY; }
        memcpy(retained, text, length + 1); text = retained; doc_view_edit_get_selection(w, &a, &b);
    }
    if (events & 1) xuiInternalEditEmit(w, XUI_EDIT_EVENT_TEXT_CHANGED, text, a, b, 0, 0, 1);
    if (xuiInternalWidgetIsValid(w) && events & 2) xuiInternalEditEmit(w, XUI_EDIT_EVENT_SELECTION_CHANGED, text, a, b, 0, 0, 1);
    free(retained);
    return XUI_OK;
}

#endif
