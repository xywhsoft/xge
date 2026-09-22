#include "xui_document_view_internal.h"
#include "xui_internal.h"
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
        (v->edit_projection.origin.iKind == XUI_DOC_POSITION_SOURCE) == (domain == XUI_DOC_SOURCE)) return XUI_OK;
    doc_view_invalidate_edit(v);
    result = doc_plain_project(v->renderer->snapshot, domain, &v->edit_projection);
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
    result = doc_view_project_edit(v); if (result != XUI_OK) return result;
    result = doc_plain_project_position(&v->edit_projection, &v->selection.tAnchor, &a);
    if (result == XUI_OK) result = doc_plain_project_position(&v->edit_projection, &v->selection.tCaret, &b);
    if (result != XUI_OK) return result;
    if (a > INT_MAX || b > INT_MAX) return XUI_DOC_ERROR_LIMIT;
    *first = (int)a; *last = (int)b; return XUI_OK;
}
static int doc_view_edit_set_selection(xui_widget w, int first, int last)
{
    doc_view_data* v = doc_view_get(w); xui_doc_range_t range; int result;
    if (!v || first < 0 || last < 0) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_project_edit(v); if (result != XUI_OK) return result;
    if ((uint64_t)first > v->edit_projection.bytes || (uint64_t)last > v->edit_projection.bytes) return XUI_ERROR_INVALID_ARGUMENT;
    if (((uint64_t)first < v->edit_projection.bytes && ((unsigned char)v->edit_projection.text[first] & 0xc0) == 0x80) ||
        ((uint64_t)last < v->edit_projection.bytes && ((unsigned char)v->edit_projection.text[last] & 0xc0) == 0x80)) return XUI_DOC_ERROR_UTF8;
    range.tAnchor = doc_plain_unproject(&v->edit_projection, (uint64_t)first, 1);
    range.tCaret = doc_plain_unproject(&v->edit_projection, (uint64_t)last, first <= last ? 0 : 1);
    if (first == last) range.tCaret = range.tAnchor;
    return xuiDocumentViewSetSelection(w, &range);
}
static int doc_view_edit_has_selection(xui_widget w)
{
    doc_view_data* v = doc_view_get(w); int order;
    return v && doc_view_sync(v) == XUI_OK && doc_position_compare(v->renderer->snapshot->state, &v->selection.tAnchor, &v->selection.tCaret, &order) == XUI_OK && order;
}
static int doc_view_edit_select_all(xui_widget w)
{
    doc_view_data* v = doc_view_get(w); xui_doc_range_t range;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    range = v->selection; range.tAnchor.iNodeId = range.tCaret.iNodeId = DOC_ROOT;
    range.tAnchor.iKind = range.tCaret.iKind = v->renderer->mode != XUI_DOC_VISUAL ? XUI_DOC_POSITION_SOURCE : XUI_DOC_POSITION_GAP;
    range.tAnchor.iOffset = 0; range.tCaret.iOffset = v->renderer->mode != XUI_DOC_VISUAL ? doc_seq_size(v->document->state->source) : doc_seq_size(doc_index_get(v->document->state->index, DOC_ROOT)->children);
    return xuiDocumentViewSetSelection(w, &range);
}
static int doc_view_edit_copy(xui_widget w)
{
    doc_view_data* v = doc_view_get(w); char* text; uint64_t bytes; int result;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    if (!v->renderer->proxy->clipboardSetText) return XUI_ERROR_UNSUPPORTED;
    result = doc_view_sync(v);
    if (result == XUI_OK) {
        result = xuiDocumentSnapshotCopyRange(v->renderer->snapshot, &v->selection, &text, &bytes);
        if (result == XUI_OK) { if (bytes) result = v->renderer->proxy->clipboardSetText(v->renderer->proxy, text); xuiDocumentFreeBuffer(text); }
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
static int doc_editor_edit_delete(xui_widget w) { return doc_view_edit_has_selection(w) ? xuiDocumentEditorInsertText(w, "", 0) : XUI_OK; }
static int doc_editor_edit_cut(xui_widget w) { return xuiDocumentEditorExecute(w, XUI_DOC_EDIT_CUT); }
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
        .hasSelection = doc_view_edit_has_selection, .selectAll = doc_view_edit_select_all, .copy = doc_view_edit_copy,
        .cut = doc_editor_edit_cut, .paste = doc_editor_edit_paste, .deleteSelection = doc_editor_edit_delete,
        .undo = doc_editor_edit_undo, .redo = doc_editor_edit_redo, .canUndo = doc_editor_edit_can_undo, .canRedo = doc_editor_edit_can_redo,
        .setReadonly = xuiDocumentEditorSetReadOnly, .isReadonly = xuiDocumentEditorGetReadOnly, .getCaretRect = doc_view_edit_caret
    };
    w->pType->pAccessibleAdapter = &accessible;
    return xuiInternalEditRegister(w, editable ? &editor : &view, NULL);
}
int doc_view_update(xui_widget w, float dt, void* user)
{
    doc_view_data* v = doc_view_get(w); unsigned events; const char* text; char* retained = NULL; int a = 0, b = 0; xui_widget host;
    (void)dt; (void)user;
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
