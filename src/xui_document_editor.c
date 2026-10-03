#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT_EDITOR
#include "xui_document_view_internal.h"
#include "xui_internal.h"
#include <math.h>
#include "xui_document_worker.h"
#include "xui_document_image_resource_internal.h"
#include "xui_document_find_ui.h"
#include "../xge.h"
#include <stdio.h>

typedef struct doc_editor_bookmark {
    uint64_t before, after;
    xui_doc_range_t start, end;
    xui_doc_table_selection_t start_table;
    xui_doc_cell_hit_t start_table_anchor, start_table_focus;
    struct doc_editor_bookmark* next;
} doc_editor_bookmark;
typedef struct doc_editor_event_item {
    struct doc_editor_event_item* next;
    uint64_t received;
    xui_event_t event;
} doc_editor_event_item;
enum doc_editor_input_kind { DOC_EDITOR_PROGRAM, DOC_EDITOR_TYPING, DOC_EDITOR_BACKSPACE, DOC_EDITOR_DELETE, DOC_EDITOR_ISOLATED };
typedef struct doc_editor_data {
    doc_view_data view;
    xui_doc_editor_desc_t desc;
    xui_widget menu;
    char* menu_text[16];
    doc_find_ui* find_ui;
    uint64_t origin;
    doc_editor_bookmark* bookmarks;
    unsigned bookmark_count;
    uint32_t pending_set, pending_clear;
    uint32_t pending_style_fields;
    xui_doc_text_style_t pending_style;
    int has_pending_marks, pending_clear_format, composing, committing, caret_visible;
    double blink;
    xui_doc_range_t composition_range;
    xui_doc_position_t composition_caret;
    xui_document_renderer projection;
    doc_input_worker* worker;
    doc_input_job *input, *retired;
    xui_doc_range_t input_start, input_end, publish_selection;
    uint64_t visual_source_caret;
    uint64_t next_group, group, group_time, group_saved, action_time;
    uint64_t stream_group;
    int stream_open;
    unsigned group_kind;
    xui_doc_range_t group_end;
    int publishing, visual_input;
    int input_error;
    doc_editor_event_item *events, *events_tail;
    uint64_t event_count, input_epoch;
    int draining, event_error, event_dragging, in_clipboard;
    int resizing_column, resize_pointer_type;
    uint64_t resize_pointer_id, resize_table, resize_group;
    uint32_t resize_column;
    double resize_start_x, resize_start_width, resize_current_width;
    uint64_t task_pressed, task_pointer_id;
    int task_pointer_type;
    double vertical_goal_x, vertical_layout_width;
    int vertical_goal_active;
} doc_editor_data;
static void doc_editor_retire_inputs(doc_editor_data*);
static void doc_editor_forget_input(doc_editor_data*);
static int doc_editor_handle_event(doc_editor_data*, const xui_event_t*, uint64_t);
static int doc_editor_flush_input(doc_editor_data*);
static xui_rect_t doc_editor_caret_rect(doc_editor_data*);
static atomic_uint_fast64_t doc_editor_sequence = 1;
static atomic_uint_fast64_t doc_editor_clipboard_image_sequence = 1;
static void doc_editor_break_group(doc_editor_data* e) { e->group = 0; e->group_kind = 0; }
static doc_editor_data* doc_editor_get(xui_widget w)
{
    xui_widget_type type = w ? xuiWidgetFindType(xuiWidgetGetContext(w), "document-editor") : NULL;
    return type && xuiWidgetIsType(w, type) ? xuiWidgetGetTypeData(w) : NULL;
}
static int doc_editor_blocked(doc_editor_data* e)
{
    return e->view.document->input_barrier && e->view.document->input_barrier != e->origin;
}
static int doc_editor_api_busy(doc_editor_data* e)
{
    /* Private event handlers have their own entry points. A host callback
     * must never inherit a replay flag that authorizes public mutations. */
    return e->events || e->draining || e->in_clipboard || e->committing || e->stream_open;
}
static int doc_editor_before_selection(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w);
    return e && (e->events || e->draining || e->in_clipboard || e->committing) ?
        XUI_DOC_ERROR_BUSY : XUI_OK;
}
static void doc_editor_cancel_resize(doc_editor_data* e, int release_capture)
{
    xui_widget w = e->view.widget;
    uint64_t pointer_id = e->resize_pointer_id;
    int pointer_type = e->resize_pointer_type;
    int active = e->resizing_column;
    e->resizing_column = 0;
    if (active && release_capture && xuiInternalWidgetIsValid(w)) {
        xui_context context = xuiWidgetGetContext(w);
        if (xuiGetPointerCaptureEx(context, pointer_id, pointer_type) == w)
            xuiReleasePointerCaptureEx(context, pointer_id, pointer_type, w);
    }
}
static void doc_editor_cancel_task_marker(doc_editor_data* e, int release_capture)
{
    xui_widget w = e->view.widget;
    uint64_t pointer_id = e->task_pointer_id;
    int pointer_type = e->task_pointer_type;
    int active = e->task_pressed != 0;
    e->task_pressed = 0;
    if (active && release_capture && xuiInternalWidgetIsValid(w)) {
        xui_context context = xuiWidgetGetContext(w);
        if (xuiGetPointerCaptureEx(context, pointer_id, pointer_type) == w)
            xuiReleasePointerCaptureEx(context, pointer_id, pointer_type, w);
    }
}
static void doc_editor_clear_events(doc_editor_data* e)
{
    e->input_epoch++;
    doc_editor_break_group(e);
    while (e->events) { doc_editor_event_item* item = e->events; e->events = item->next; doc_free(item); }
    e->events_tail = NULL; e->event_count = 0; e->event_error = e->event_dragging = 0;
    doc_editor_cancel_resize(e, 1);
    doc_editor_cancel_task_marker(e, 1);
    if (!e->stream_open && e->view.document &&
        e->view.document->input_barrier == e->origin) e->view.document->input_barrier = 0;
}
static int doc_editor_ordered_event(const xui_event_t* event)
{
    switch (event->iType) {
    case XUI_EVENT_KEY_DOWN:
        switch (event->iKey) {
        case XUI_KEY_ESCAPE: case XUI_KEY_BACKSPACE: case XUI_KEY_DELETE: case XUI_KEY_ENTER:
        case XUI_KEY_LEFT: case XUI_KEY_RIGHT: case XUI_KEY_UP: case XUI_KEY_DOWN:
        case XUI_KEY_HOME: case XUI_KEY_END: case XUI_KEY_PAGE_UP: case XUI_KEY_PAGE_DOWN:
        case XUI_KEY_TAB: return 1;
        default: break;
        }
        if (event->iModifiers & XUI_MOD_CTRL) switch (event->iKey >= 'a' && event->iKey <= 'z' ? event->iKey - 'a' + 'A' : event->iKey) {
        case 'A': case 'C': case 'X': case 'V': case 'Z': case 'Y': case 'B': case 'I': case 'U':
        case 'F': case 'H': return 1;
        default: break;
        }
        return 0;
    case XUI_EVENT_TEXT: case XUI_EVENT_IME_COMPOSITION:
    case XUI_EVENT_POINTER_DOWN: case XUI_EVENT_POINTER_UP: case XUI_EVENT_POINTER_MOVE:
    case XUI_EVENT_POINTER_WHEEL: case XUI_EVENT_POINTER_CAPTURE_LOST:
    case XUI_EVENT_CONTEXT_MENU: case XUI_EVENT_BLUR: return 1;
    default: return 0;
    }
}
static int doc_editor_enqueue(doc_editor_data* e, const xui_event_t* event, uint64_t received)
{
    doc_editor_event_item* item; xui_widget w = e->view.widget;
    if (event->iType == XUI_EVENT_POINTER_MOVE && e->events_tail && e->events_tail->event.iType == XUI_EVENT_POINTER_MOVE &&
        e->events_tail->event.iPointerId == event->iPointerId && e->events_tail->event.iPointerType == event->iPointerType) {
        e->events_tail->event.fX = event->fX; e->events_tail->event.fY = event->fY;
        e->events_tail->event.iModifiers = event->iModifiers; e->events_tail->event.iButtons = event->iButtons; return XUI_OK;
    }
    if (e->event_count >= 4096) return XUI_DOC_ERROR_LIMIT;
    item = doc_alloc(e->view.document->allocator, sizeof(*item)); if (!item) return XUI_ERROR_OUT_OF_MEMORY;
    item->received = received;
    item->event = *event; item->event.pTarget = item->event.pCurrentTarget = w;
    /* Only inline payloads of the event types above are retained. Borrowed
     * command/user/related-widget pointers have no meaning for this queue. */
    item->event.sCommand = NULL; item->event.pData = NULL; item->event.pRelated = NULL;
    if (e->events_tail) e->events_tail->next = item; else { e->events = item; e->event_dragging = e->view.dragging; }
    e->events_tail = item; e->event_count++; e->view.document->input_barrier = e->origin;
    xuiWidgetInvalidate(w, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
    /* Platform focus/capture follow arrival time, while selection/navigation
     * follows document action order. Replay must not steal focus back later. */
    if (event->iType == XUI_EVENT_POINTER_DOWN && (event->iButton == XUI_POINTER_BUTTON_LEFT || !event->iButton)) {
        e->event_dragging = !e->view.desc.bDisableSelection;
        xuiSetFocusWidget(xuiWidgetGetContext(w), w);
        if (xuiInternalWidgetIsValid(w) && !e->view.desc.bDisableSelection)
            xuiSetPointerCaptureEx(xuiWidgetGetContext(w), event->iPointerId, event->iPointerType, w);
    } else if (event->iType == XUI_EVENT_POINTER_UP) {
        e->event_dragging = 0;
        xuiReleasePointerCaptureEx(xuiWidgetGetContext(w), event->iPointerId, event->iPointerType, w);
    } else if (event->iType == XUI_EVENT_BLUR || event->iType == XUI_EVENT_POINTER_CAPTURE_LOST) e->event_dragging = 0;
    return XUI_OK;
}
static void doc_editor_clear_bookmarks(doc_editor_data* e)
{
    while (e->bookmarks) { doc_editor_bookmark* b = e->bookmarks; e->bookmarks = b->next; free(b); }
    e->bookmark_count = 0;
}
static void doc_editor_trim_bookmarks(doc_editor_data* e)
{
    doc_editor_bookmark* last = e->bookmarks; unsigned i, limit = e->view.document->history_limit;
    if (!limit || e->view.document->disable_history) { doc_editor_clear_bookmarks(e); return; }
    if (e->bookmark_count <= limit || !last) return;
    for (i = 1; i < limit && last->next; i++) last = last->next;
    while (last->next) { doc_editor_bookmark* old = last->next; last->next = old->next; free(old); }
    e->bookmark_count = limit;
}
static void doc_editor_keep_bookmark(doc_editor_data* e, doc_editor_bookmark* b)
{
    doc_history* h = e->view.document->undo;
    if (!h || h->origin != e->origin || h->after->content_id != b->after) {
        e->bookmarks = b->next; free(b); return;
    }
    if (h->before->content_id != b->before) {
        doc_editor_bookmark** link = &b->next;
        while (*link && ((*link)->before != h->before->content_id || (*link)->after != b->before)) link = &(*link)->next;
        if (!*link) { e->bookmarks = b->next; free(b); return; }
        { doc_editor_bookmark* old = *link; b->before = old->before; b->start = old->start;
          b->start_table = old->start_table;
          b->start_table_anchor = old->start_table_anchor;
          b->start_table_focus = old->start_table_focus;
          *link = old->next; free(old); e->bookmark_count--; }
    }
    e->bookmark_count++; doc_editor_trim_bookmarks(e);
}
static int doc_editor_cancel_composition(doc_editor_data* e)
{
    e->composing = 0; xuiDocumentRendererRelease(e->projection); e->projection = NULL;
    return xuiWidgetInvalidate(e->view.widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API int xuiDocumentEditorCancelComposition(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w); if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    return doc_editor_api_busy(e) ? XUI_DOC_ERROR_BUSY : doc_editor_cancel_composition(e);
}
XUI_API int xuiDocumentEditorIsComposing(xui_widget w) { doc_editor_data* e = doc_editor_get(w); return e && e->composing; }
static void doc_editor_projection_changed(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w);
    if (!e) return;
    doc_editor_cancel_composition(e);
    doc_editor_break_group(e);
    doc_editor_cancel_resize(e, 1);
    doc_editor_cancel_task_marker(e, 1);
    e->vertical_goal_active = 0;
    e->pending_set = e->pending_clear = 0; e->has_pending_marks = 0;
    e->pending_style_fields = 0; e->pending_clear_format = 0;
    e->blink = 0; e->caret_visible = 1;
}
static void doc_editor_changed(xui_document d, xui_document_change_set c, void* user)
{
    doc_editor_data* e = user; doc_editor_bookmark* b;
    e->vertical_goal_active = 0;
    if (!e->committing) e->stream_open = 0;
    if (!e->committing) doc_editor_clear_events(e);
    if (e->input && !e->committing) {
        xui_doc_position_t at; int mapping;
        if (c && xuiDocumentMapPosition(c, &e->input_start.tAnchor, &at, &mapping) == XUI_OK) e->view.selection.tAnchor = at;
        if (c && xuiDocumentMapPosition(c, &e->input_start.tCaret, &at, &mapping) == XUI_OK) e->view.selection.tCaret = at;
        doc_editor_forget_input(e);
    }
    if (!c) { doc_editor_clear_bookmarks(e); doc_editor_cancel_composition(e); return; }
    if (!e->committing) doc_editor_cancel_composition(e);
    if (e->publishing && c->origin == e->origin) e->view.selection = e->publish_selection;
    else if (c->origin == e->origin) for (b = e->bookmarks; b; b = b->next) {
        if (c->before->content_id == b->before && c->after->content_id == b->after) {
            e->view.selection = b->end; break;
        }
        if (c->before->content_id == b->after && c->after->content_id == b->before) {
            e->view.selection = b->start;
            if (e->view.renderer->mode == XUI_DOC_VISUAL) {
                e->view.table_selection = b->start_table;
                e->view.table_anchor = b->start_table_anchor;
                e->view.table_focus = b->start_table_focus;
            }
            break;
        }
    }
    e->view.selection.tAnchor.iRevision = e->view.selection.tCaret.iRevision = xuiDocumentGetRevision(d);
    if (e->publishing) e->view.selection.tAnchor.iInputGeneration = e->view.selection.tCaret.iInputGeneration = 0;
    if (!e->view.needs_sync) {
        unsigned i;
        for (i = 0; i < 2; i++) {
            xui_doc_position_t* p = i ? &e->view.selection.tCaret : &e->view.selection.tAnchor; int mapping;
            if (e->view.renderer->mode != XUI_DOC_VISUAL && p->iKind != XUI_DOC_POSITION_SOURCE) {
                uint64_t offset;
                if (xuiDocumentPositionToSource(e->view.renderer->snapshot, p, &offset, &mapping) == XUI_OK) { p->iKind = XUI_DOC_POSITION_SOURCE; p->iNodeId = DOC_ROOT; p->iOffset = offset; }
            } else if (e->view.renderer->mode == XUI_DOC_VISUAL && p->iKind == XUI_DOC_POSITION_SOURCE)
                xuiDocumentSourceToPositionEx(e->view.renderer->snapshot, p->iOffset, p->iAffinity, p, &mapping);
        }
    }
    e->blink = 0; e->caret_visible = 1;
}
static int doc_editor_sync(doc_editor_data* e)
{
    int result = doc_view_sync(&e->view);
    if (result == XUI_OK) doc_editor_retire_inputs(e);
    return result;
}
static void doc_editor_reveal(doc_editor_data* e)
{
    xui_doc_rect_t caret, size; int exact; xui_rect_t content = xuiWidgetGetContentRect(e->view.widget);
    if (content.fW <= 0 || content.fH <= 0) return;
    if (doc_view_layout(&e->view) != XUI_OK || xuiDocumentRendererGetCaretRect(e->view.renderer, &e->view.selection.tCaret, &caret) != XUI_OK) return;
    if (caret.y < e->view.scroll_y) e->view.scroll_y = caret.y;
    else if (caret.y + caret.height > e->view.scroll_y + content.fH) e->view.scroll_y = fmax(0, caret.y + caret.height - content.fH);
    if (caret.x < e->view.scroll_x) e->view.scroll_x = caret.x;
    else if (caret.x + 2 > e->view.scroll_x + content.fW) e->view.scroll_x = fmax(0, caret.x + 2 - content.fW);
    /* A delete/Undo may make the entire content fit again. Keeping the old
     * offset at the new caret would hide its prefix and misplace clicks. */
    if (xuiDocumentRendererGetSize(e->view.renderer, &size, &exact) == XUI_OK) {
        e->view.scroll_x = fmin(e->view.scroll_x, fmax(0, size.width - content.fW));
        e->view.scroll_y = fmin(e->view.scroll_y, fmax(0, size.height - content.fH));
    }
    e->blink = 0; e->caret_visible = 1;
    xuiWidgetInvalidate(e->view.widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER |
        (e->view.renderer->mode == XUI_DOC_LIVE_MARKDOWN ? XUI_WIDGET_DIRTY_LAYOUT : 0));
}
static int doc_editor_commit_ex(doc_editor_data* e, xui_document_transaction t, const xui_doc_range_t* after, int reveal)
{
    xui_widget w = e->view.widget; doc_editor_bookmark* b; int result;
    if (!t->count) return xuiDocumentTxnCommit(t, NULL);
    b = calloc(1, sizeof(*b)); if (!b) return XUI_ERROR_OUT_OF_MEMORY;
    b->start = e->view.selection; b->end = *after;
    b->start_table = e->view.table_selection;
    b->start_table_anchor = e->view.table_anchor;
    b->start_table_focus = e->view.table_focus;
    b->before = t->base->content_id; b->after = t->document->next_state + 1;
    b->next = e->bookmarks; e->bookmarks = b; e->committing = e->publishing = 1; e->publish_selection = *after;
    result = xuiDocumentTxnCommit(t, NULL);
    if (!xuiInternalWidgetIsValid(w)) return result;
    e->committing = e->publishing = 0;
    if (result != XUI_OK) { e->bookmarks = b->next; free(b); return result; }
    doc_editor_keep_bookmark(e, b);
    if (reveal) doc_editor_reveal(e);
    return XUI_OK;
}
static int doc_editor_commit(doc_editor_data* e, xui_document_transaction t, const xui_doc_range_t* after)
{
    return doc_editor_commit_ex(e, t, after, 1);
}
static void doc_editor_retire_inputs(doc_editor_data* e)
{
    doc_input_job** link = &e->retired;
    while (*link) {
        doc_input_job* job = *link;
        if (job->prepare == e->view.renderer->input) { link = &job->next; continue; }
        *link = job->next; doc_input_retire(e->worker, job);
    }
}
static void doc_editor_forget_input(doc_editor_data* e)
{
    if (e->input) { e->input->next = e->retired; e->retired = e->input; e->input = NULL; }
    if (e->visual_input) {
        e->visual_input = 0; e->view.needs_sync = 1;
        doc_view_find_invalidate(&e->view); doc_view_accessible_clear(&e->view);
        if (xuiInternalWidgetIsValid(e->view.widget))
            xuiInternalAccessibilityQueue(e->view.widget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
    }
    e->view.input = NULL; e->input_error = 0;
    doc_view_invalidate_edit(&e->view); e->view.pending_edit_events |= 3;
    doc_editor_retire_inputs(e);
}
static void doc_editor_accept_input(doc_editor_data* e, doc_input_job* job)
{
    if (e->input) { e->input->next = e->retired; e->retired = e->input; }
    else e->input_start = e->view.selection;
    e->input = job; e->view.input = job->prepare; e->input_error = 0;
    doc_view_invalidate_edit(&e->view); e->view.pending_edit_events |= 3;
    e->view.error = doc_editor_sync(e);
    /* Creation accepted the input. Renderer OOM is retried by sync/update;
     * retain the old renderer's job until that renderer releases it. */
    xuiWidgetInvalidate(e->view.widget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
static int doc_editor_prepare_replace(doc_editor_data* e, const xui_doc_range_t* range, const char* text, uint64_t bytes, uint64_t group)
{
    doc_input_job* job; xui_doc_source_patch_t patch = {0}; xui_doc_txn_desc_t desc = {0}; int order, result;
    if (e->desc.bReadOnly) return XUI_ERROR_INVALID_STATE;
    if (doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->view.document->pending && (!e->input || e->view.document->pending != e->input->prepare)) return XUI_DOC_ERROR_BUSY;
    result = doc_render_compare(e->view.renderer, &range->tAnchor, &range->tCaret, &order); if (result != XUI_OK) return result;
    if (e->view.renderer->mode == XUI_DOC_LIVE_MARKDOWN) {
        result = xuiDocumentRendererSetActivePosition(e->view.renderer, &range->tCaret);
        if (result != XUI_OK) return result;
    }
    if (!e->worker) { e->worker = doc_input_worker_create(); if (!e->worker) return XUI_ERROR_OUT_OF_MEMORY; }
    job = calloc(1, sizeof(*job)); if (!job) return XUI_ERROR_OUT_OF_MEMORY;
    patch.iSize = sizeof(patch); patch.iStart = (order < 0 ? range->tAnchor : range->tCaret).iOffset;
    patch.iEnd = (order < 0 ? range->tCaret : range->tAnchor).iOffset; patch.sText = text; patch.iTextBytes = bytes;
    desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE; desc.iOrigin = e->origin; desc.iGroup = group; job->group = group;
    result = e->input ? xuiDocumentPrepareContinueSource(e->view.document, e->input->prepare, &patch, 1, &job->prepare) :
        xuiDocumentPrepareSource(e->view.document, &desc, &patch, 1, &job->prepare);
    if (result != XUI_OK) { free(job); return result; }
    doc_editor_accept_input(e, job);
    xuiDocumentPrepareSourcePosition(job->prepare, patch.iStart + bytes, XUI_DOC_AFTER, &e->view.selection.tCaret);
    e->view.selection.tAnchor = e->view.selection.tCaret; e->input_end = e->view.selection;
    doc_editor_reveal(e); return XUI_OK;
}
static int doc_editor_prepare_visual_text(doc_editor_data* e, const xui_doc_range_t* range,
    const char* text, uint64_t bytes, uint64_t group)
{
    doc_input_job* job; xui_document_snapshot preview = NULL;
    xui_doc_txn_desc_t desc = {0}; struct xui_doc_change_set_t change = {0};
    xui_doc_operation_t op = {0}; xui_doc_position_t a = range->tAnchor, b = range->tCaret, preview_caret;
    uint64_t source_caret; int result, order, same_node;
    if (e->input || e->view.document->pending) return XUI_DOC_ERROR_BUSY;
    if (a.iKind != XUI_DOC_POSITION_TEXT || b.iKind != XUI_DOC_POSITION_TEXT)
        return XUI_ERROR_UNSUPPORTED;
    result = doc_render_compare(e->view.renderer, &a, &b, &order);
    if (result != XUI_OK) return result;
    if (order > 0) { xui_doc_position_t swap = a; a = b; b = swap; }
    same_node = a.iNodeId == b.iNodeId;
    if (!e->worker) { e->worker = doc_input_worker_create(); if (!e->worker) return XUI_ERROR_OUT_OF_MEMORY; }
    job = calloc(1, sizeof(*job)); if (!job) return XUI_ERROR_OUT_OF_MEMORY;
    desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE; desc.iOrigin = e->origin; desc.iGroup = group;
    if (same_node) {
        result = doc_prepare_visual_text(e->view.document, &desc, a.iNodeId,
            a.iOffset, b.iOffset, text, bytes, &job->prepare, &preview, &source_caret);
        preview_caret = a; preview_caret.iOffset += bytes;
    } else result = doc_prepare_visual_span(e->view.document, &desc, range,
        text, bytes, &job->prepare, &preview, &preview_caret, &source_caret);
    if (result != XUI_OK) { free(job); return result; }
    job->group = group;
    op.iKind = XUI_DOC_OP_TEXT; op.iFlags = XUI_DOC_CHANGE_TEXT;
    op.iNodeId = a.iNodeId; op.iOffset = a.iOffset;
    op.iOldLength = same_node ? b.iOffset - a.iOffset : 0;
    op.iNewLength = same_node ? bytes : 0;
    change.before = e->view.renderer->snapshot->state; change.after = preview->state;
    change.identity = preview->identity;
    change.before_revision = change.after_revision = preview->revision;
    change.flags = XUI_DOC_CHANGE_TEXT; change.ops = &op; change.count = 1;
    result = xuiDocumentRendererSetSnapshot(e->view.renderer, preview, &change);
    xuiDocumentSnapshotRelease(preview);
    if (result != XUI_OK) {
        doc_input_retire(e->worker, job); return result;
    }
    result = doc_prepare_visual_attach(e->view.document, NULL, job->prepare);
    if (result != XUI_OK) {
        e->view.needs_sync = 1; (void)doc_editor_sync(e);
        doc_input_retire(e->worker, job); return result;
    }
    e->input_start = e->view.selection; e->input = job; e->visual_input = 1;
    e->visual_source_caret = source_caret;
    e->view.input = NULL; e->input_error = 0;
    e->view.selection.tAnchor = e->view.selection.tCaret = preview_caret;
    e->input_end = e->view.selection;
    doc_view_find_invalidate(&e->view); doc_view_accessible_clear(&e->view);
    xuiInternalAccessibilityQueue(e->view.widget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
    doc_view_invalidate_edit(&e->view); e->view.pending_edit_events |= 3;
    xuiWidgetInvalidate(e->view.widget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
    return XUI_OK;
}
static int doc_editor_continue_visual_text(doc_editor_data* e, const xui_doc_range_t* range,
    const char* text, uint64_t bytes, uint64_t group)
{
    doc_input_job* job; xui_document_snapshot preview = NULL;
    struct xui_doc_change_set_t change = {0}; xui_doc_operation_t op = {0};
    xui_doc_position_t a = range->tAnchor, b = range->tCaret;
    xui_doc_position_t current = e->view.selection.tCaret;
    uint64_t source_caret; int result;
    if (!e->visual_input || !e->input || e->input->group != group ||
        a.iKind != XUI_DOC_POSITION_TEXT || b.iKind != a.iKind || a.iNodeId != b.iNodeId ||
        a.iDocumentId != current.iDocumentId || b.iDocumentId != current.iDocumentId ||
        a.iRevision != current.iRevision || b.iRevision != current.iRevision ||
        a.iNodeId != current.iNodeId ||
        e->pending_clear_format || e->has_pending_marks || e->pending_style_fields)
        return XUI_DOC_ERROR_BUSY;
    if (a.iOffset > b.iOffset) { xui_doc_position_t swap = a; a = b; b = swap; }
    if (!bytes && a.iOffset == b.iOffset) return XUI_DOC_ERROR_BUSY;
    job = calloc(1, sizeof(*job)); if (!job) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_prepare_visual_continue(e->view.document, e->input->prepare,
        a.iNodeId, a.iOffset, b.iOffset, text, bytes, e->visual_source_caret,
        &job->prepare, &preview, &source_caret);
    if (result != XUI_OK) { free(job); return result == XUI_ERROR_UNSUPPORTED ? XUI_DOC_ERROR_BUSY : result; }
    job->group = group;
    op.iKind = XUI_DOC_OP_TEXT; op.iFlags = XUI_DOC_CHANGE_TEXT;
    op.iNodeId = a.iNodeId; op.iOffset = a.iOffset;
    op.iOldLength = b.iOffset - a.iOffset; op.iNewLength = bytes;
    change.before = e->view.renderer->snapshot->state; change.after = preview->state;
    change.identity = preview->identity;
    change.before_revision = change.after_revision = preview->revision;
    change.flags = XUI_DOC_CHANGE_TEXT; change.ops = &op; change.count = 1;
    result = xuiDocumentRendererSetSnapshot(e->view.renderer, preview, &change);
    xuiDocumentSnapshotRelease(preview);
    if (result != XUI_OK) { doc_input_retire(e->worker, job); return result; }
    result = doc_prepare_visual_attach(e->view.document, e->input->prepare, job->prepare);
    if (result != XUI_OK) {
        e->view.needs_sync = 1; (void)doc_editor_sync(e);
        doc_input_retire(e->worker, job); return result;
    }
    e->input->next = e->retired; e->retired = e->input; e->input = job;
    e->visual_source_caret = source_caret; e->input_error = 0;
    e->view.selection.tAnchor = e->view.selection.tCaret = a;
    e->view.selection.tCaret.iOffset += bytes;
    e->view.selection.tAnchor = e->view.selection.tCaret;
    e->input_end = e->view.selection;
    doc_view_find_invalidate(&e->view); doc_view_accessible_clear(&e->view);
    xuiInternalAccessibilityQueue(e->view.widget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
    doc_view_invalidate_edit(&e->view); e->view.pending_edit_events |= 3;
    doc_editor_retire_inputs(e);
    xuiWidgetInvalidate(e->view.widget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
    return XUI_OK;
}
XUI_API int xuiDocumentEditorGetPendingInput(xui_widget w, xui_doc_prepare_info_t* info)
{
    doc_editor_data* e = doc_editor_get(w);
    if (!e || !info || info->iSize != sizeof(*info)) return XUI_ERROR_INVALID_ARGUMENT;
    return e->input ? xuiDocumentPrepareGetInfo(e->input->prepare, info) : XUI_ERROR_NOT_FOUND;
}
XUI_API int xuiDocumentEditorAppendStreamSource(xui_widget w,
    const char* text, uint64_t bytes, int final_chunk)
{
    doc_editor_data* e = doc_editor_get(w);
    doc_input_job* job; xui_doc_txn_desc_t desc = {0};
    xui_doc_prepare_info_t info = {0};
    uint64_t group; int result;
    if (!e || (!text && bytes) || bytes > SIZE_MAX || (final_chunk != 0 && final_chunk != 1))
        return XUI_ERROR_INVALID_ARGUMENT;
    if ((e->events && !e->stream_open) || e->draining || e->in_clipboard || e->committing || e->composing ||
        e->desc.bReadOnly || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if ((e->view.renderer->mode != XUI_DOC_SOURCE_TEXT &&
         e->view.renderer->mode != XUI_DOC_LIVE_MARKDOWN) ||
        xuiDocumentGetProfile(e->view.document) != XUI_DOCUMENT_MARKDOWN)
        return XUI_ERROR_UNSUPPORTED;
    if (e->input && !e->stream_open) return XUI_DOC_ERROR_BUSY;
    if (e->view.document->pending &&
        (!e->input || e->view.document->pending != e->input->prepare)) return XUI_DOC_ERROR_BUSY;
    if (!e->stream_open && !e->input && !bytes && final_chunk) return XUI_OK;
    if (!e->stream_open && e->next_group == UINT64_MAX) return XUI_DOC_ERROR_LIMIT;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    if (!e->stream_open && e->view.renderer->mode == XUI_DOC_LIVE_MARKDOWN) {
        xui_doc_position_t tail = {0};
        tail.iSize = sizeof(tail); tail.iKind = XUI_DOC_POSITION_SOURCE; tail.iNodeId = DOC_ROOT;
        tail.iDocumentId = e->view.renderer->snapshot->identity;
        tail.iRevision = e->view.renderer->snapshot->revision;
        tail.iOffset = doc_seq_size(doc_render_source(e->view.renderer)); tail.iAffinity = XUI_DOC_AFTER;
        result = xuiDocumentRendererSetActivePosition(e->view.renderer, &tail);
        if (result != XUI_OK) return result;
    }
    if (!e->worker) { e->worker = doc_input_worker_create(); if (!e->worker) return XUI_ERROR_OUT_OF_MEMORY; }
    job = calloc(1, sizeof(*job)); if (!job) return XUI_ERROR_OUT_OF_MEMORY;
    group = e->stream_open ? e->stream_group : e->next_group + 1;
    desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
    desc.iOrigin = e->origin; desc.iGroup = group;
    result = xuiDocumentPrepareStreamSource(e->view.document,
        e->input ? e->input->prepare : NULL, e->input ? NULL : &desc,
        text, bytes, final_chunk, &job->prepare);
    if (result != XUI_OK) { free(job); return result; }
    job->group = group;
    if (!e->stream_open) {
        e->next_group = e->stream_group = group; e->stream_open = 1;
        doc_editor_break_group(e);
    }
    e->view.document->input_barrier = e->origin;
    doc_editor_accept_input(e, job);
    info.iSize = sizeof(info);
    result = xuiDocumentPrepareGetInfo(job->prepare, &info);
    if (result == XUI_OK) result = xuiDocumentPrepareSourcePosition(job->prepare,
        info.iSourceBytes, XUI_DOC_AFTER, &e->view.selection.tCaret);
    if (result == XUI_OK) {
        e->view.selection.tAnchor = e->view.selection.tCaret;
        e->input_end = e->view.selection;
    }
    if (final_chunk) {
        e->stream_open = 0;
        if (!e->events && e->view.document->input_barrier == e->origin)
            e->view.document->input_barrier = 0;
    }
    if (result == XUI_OK) doc_editor_reveal(e);
    return result;
}
static int doc_editor_flush_input(doc_editor_data* e)
{
    xui_widget w = e->view.widget; xui_doc_prepare_info_t info = {0}; doc_editor_bookmark* b;
    xui_document_change_set change = NULL; uint64_t visual_anchor = 0, visual_caret = 0;
    int visual_selection_exact = 0, result;
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (!e->input) return XUI_OK;
    if (e->composing || e->committing) return XUI_DOC_ERROR_BUSY;
    info.iSize = sizeof(info); xuiDocumentPrepareGetInfo(e->input->prepare, &info);
    if (info.bCancellationRequested) {
        e->stream_open = 0;
        e->view.selection = e->input_start; doc_editor_clear_events(e); doc_editor_forget_input(e); doc_editor_sync(e);
        return info.iResult == XUI_DOC_ERROR_BUSY ? XUI_DOC_ERROR_STALE : info.iResult;
    }
    if (info.iState == XUI_DOC_PREPARE_FAILED) return info.iResult;
    if (info.iState != XUI_DOC_PREPARE_READY) { doc_input_schedule(e->worker, e->input); return XUI_DOC_ERROR_BUSY; }
    if (e->visual_input)
        visual_selection_exact =
            doc_prepare_visual_position_source(e->view.document, e->input->prepare,
                &e->view.selection.tAnchor, &visual_anchor) == XUI_OK &&
            doc_prepare_visual_position_source(e->view.document, e->input->prepare,
                &e->view.selection.tCaret, &visual_caret) == XUI_OK;
    b = calloc(1, sizeof(*b)); if (!b) return XUI_ERROR_OUT_OF_MEMORY;
    b->start = e->input_start; b->end = e->input_end;
    b->end.tAnchor.iInputGeneration = b->end.tCaret.iInputGeneration = 0;
    b->before = e->view.document->state->content_id; b->after = e->view.document->next_state + 1;
    b->next = e->bookmarks; e->bookmarks = b; e->committing = e->publishing = 1; e->publish_selection = e->view.selection;
    result = xuiDocumentPreparePublish(e->view.document, e->input->prepare, &change);
    if (!xuiInternalWidgetIsValid(w)) { xuiDocumentChangeSetRelease(change); return result; }
    e->committing = e->publishing = 0;
    if (result != XUI_OK || !change) { e->bookmarks = b->next; free(b); b = NULL; }
    if (result == XUI_OK) {
        unsigned i;
        if (e->visual_input) {
            xui_document_snapshot published = NULL;
            xui_doc_position_t mapped; int mapping;
            if (xuiDocumentAcquireSnapshot(e->view.document, &published) == XUI_OK) {
                if (xuiDocumentSourceToPositionEx(published, e->visual_source_caret,
                    XUI_DOC_AFTER, &mapped, &mapping) == XUI_OK) {
                    e->view.selection.tAnchor = e->view.selection.tCaret = mapped;
                    e->input_end = e->view.selection;
                    if (b) b->end = e->view.selection;
                }
                if (visual_selection_exact) {
                    xui_doc_range_t selected;
                    int anchor_mapping, caret_mapping;
                    if (xuiDocumentSourceToPositionEx(published, visual_anchor,
                            e->publish_selection.tAnchor.iAffinity,
                            &selected.tAnchor, &anchor_mapping) == XUI_OK &&
                        anchor_mapping == XUI_DOC_MAP_EXACT &&
                        xuiDocumentSourceToPositionEx(published, visual_caret,
                            e->publish_selection.tCaret.iAffinity,
                            &selected.tCaret, &caret_mapping) == XUI_OK &&
                        caret_mapping == XUI_DOC_MAP_EXACT)
                        e->view.selection = selected;
                }
                xuiDocumentSnapshotRelease(published);
            }
        }
        if (b) doc_editor_keep_bookmark(e, b);
        e->view.selection.tAnchor.iInputGeneration = e->view.selection.tCaret.iInputGeneration = 0;
        e->view.selection.tAnchor.iRevision = e->view.selection.tCaret.iRevision = xuiDocumentGetRevision(e->view.document);
        for (i = 0; i < 2; i++) {
            xui_doc_position_t* p = i ? &e->composition_range.tCaret : &e->composition_range.tAnchor;
            if (p->iInputGeneration == info.iGeneration) {
                p->iInputGeneration = 0; p->iRevision = xuiDocumentGetRevision(e->view.document);
            }
        }
        if (e->group == e->input->group) {
            if (e->visual_input) e->group_end = e->view.selection;
            else {
                e->group_end.tAnchor.iInputGeneration = e->group_end.tCaret.iInputGeneration = 0;
                e->group_end.tAnchor.iRevision = e->group_end.tCaret.iRevision = xuiDocumentGetRevision(e->view.document);
            }
        }
        doc_editor_forget_input(e); result = doc_editor_sync(e);
        if (result == XUI_OK) doc_editor_reveal(e);
    }
    xuiDocumentChangeSetRelease(change); return result;
}
XUI_API int xuiDocumentEditorGetPendingWork(xui_widget w, xui_doc_editor_pending_work_t* info)
{
    doc_editor_data* e = doc_editor_get(w); xui_doc_prepare_info_t input = {0};
    if (!e || !info || info->iSize != sizeof(*info)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(info, 0, sizeof(*info)); info->iSize = sizeof(*info);
    info->bHasInput = e->input != NULL; info->iQueuedEvents = e->event_count;
    info->iResult = e->event_error ? e->event_error :
        (e->input || e->events || e->stream_open ? XUI_DOC_ERROR_BUSY : XUI_OK);
    if (e->input) {
        input.iSize = sizeof(input); xuiDocumentPrepareGetInfo(e->input->prepare, &input);
        if (input.iState == XUI_DOC_PREPARE_FAILED) info->iResult = input.iResult;
    }
    return XUI_OK;
}
XUI_API int xuiDocumentEditorFlush(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w); unsigned budget = 64; int result = XUI_OK;
    uint64_t started = xrtClock();
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (e->draining || e->in_clipboard || e->committing || e->view.document->writer || e->view.document->notifying) return XUI_DOC_ERROR_BUSY;
    if (e->event_error) return e->event_error;
    e->draining = 1;
    if (e->stream_open) {
        result = doc_editor_flush_input(e);
        if (!xuiInternalWidgetIsValid(w)) return result;
        e->draining = 0;
        return result == XUI_OK ? XUI_DOC_ERROR_BUSY : result;
    }
    while (e->events && budget--) {
        doc_editor_event_item* item = e->events; xui_event_t event = item->event; uint64_t epoch = e->input_epoch;
        e->view.replaying_input = 1;
        result = doc_editor_handle_event(e, &event, item->received);
        if (!xuiInternalWidgetIsValid(w)) return result < 0 ? result : XUI_OK;
        e->view.replaying_input = 0;
        /* Cancellation/external replacement wins over this action. A host
         * may also enqueue fresh input, reusing the freed item's address. */
        if (epoch != e->input_epoch) result = XUI_OK;
        if (result == XUI_DOC_ERROR_BUSY) break;
        if (result == XUI_ERROR_OUT_OF_MEMORY || result == XUI_DOC_ERROR_LIMIT) { e->event_error = result; break; }
        /* An external edit or explicit cancellation from a callback may have
         * cleared this queue. Never dereference the old item in that case. */
        if (epoch == e->input_epoch && e->events == item) {
            e->events = item->next; if (!e->events) e->events_tail = NULL;
            e->event_count--; doc_free(item);
        }
        if (result < 0 && result != XUI_ERROR_NOT_FOUND && e->desc.onError) {
            e->desc.onError(w, result, e->desc.pUser); if (!xuiInternalWidgetIsValid(w)) return result;
        }
        result = XUI_OK;
        if (xrtClock() - started >= 4000) break;
    }
    if (!e->events && !e->stream_open &&
        e->view.document->input_barrier == e->origin) e->view.document->input_barrier = 0;
    if (result == XUI_OK) {
        if (e->events) result = XUI_DOC_ERROR_BUSY;
        else if (e->input && xrtClock() - started >= 4000) { doc_input_schedule(e->worker, e->input); result = XUI_DOC_ERROR_BUSY; }
        else result = doc_editor_flush_input(e);
    }
    if (!xuiInternalWidgetIsValid(w)) return result;
    if (result == XUI_OK && e->events) result = XUI_DOC_ERROR_BUSY;
    e->draining = 0; return result;
}
XUI_API int xuiDocumentEditorRetryInput(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w); doc_input_job* job; xui_doc_range_t range; int result;
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (e->draining || e->in_clipboard || e->committing) return XUI_DOC_ERROR_BUSY;
    e->event_error = e->input_error = 0;
    if (!e->input) return XUI_OK;
    if (e->composing) return XUI_DOC_ERROR_BUSY;
    job = calloc(1, sizeof(*job)); if (!job) return XUI_ERROR_OUT_OF_MEMORY;
    result = xuiDocumentPrepareContinueSource(e->view.document, e->input->prepare, NULL, 0, &job->prepare);
    if (result != XUI_OK) { free(job); return result; }
    job->group = e->input->group;
    if (e->visual_input) {
        e->input->next = e->retired; e->retired = e->input;
        e->input = job; e->input_error = 0;
        doc_editor_retire_inputs(e);
        doc_input_schedule(e->worker, job); return XUI_OK;
    }
    range = e->view.selection; doc_editor_accept_input(e, job);
    xuiDocumentPrepareSourcePosition(job->prepare, range.tAnchor.iOffset, range.tAnchor.iAffinity, &e->view.selection.tAnchor);
    xuiDocumentPrepareSourcePosition(job->prepare, range.tCaret.iOffset, range.tCaret.iAffinity, &e->view.selection.tCaret);
    e->input_end.tAnchor.iInputGeneration = e->input_end.tCaret.iInputGeneration = e->view.selection.tCaret.iInputGeneration;
    e->group_end.tAnchor.iInputGeneration = e->group_end.tCaret.iInputGeneration = e->view.selection.tCaret.iInputGeneration;
    doc_input_schedule(e->worker, job); return XUI_OK;
}
XUI_API int xuiDocumentEditorCancelInput(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w); int result = XUI_OK;
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (e->view.document->writer || e->view.document->notifying) return XUI_DOC_ERROR_BUSY;
    e->stream_open = 0;
    doc_editor_clear_events(e);
    doc_editor_cancel_composition(e);
    if (!e->input) return XUI_OK;
    if (e->view.document->pending == e->input->prepare) result = xuiDocumentCancelPrepare(e->view.document);
    else xuiDocumentPrepareCancel(e->input->prepare);
    if (result == XUI_OK) {
        e->view.selection = e->input_start; doc_editor_forget_input(e); result = doc_editor_sync(e);
    }
    return result;
}
static int doc_editor_group_position_equal(const xui_doc_position_t* a, const xui_doc_position_t* b)
{
    return a->iDocumentId == b->iDocumentId && a->iKind == b->iKind && a->iNodeId == b->iNodeId && a->iOffset == b->iOffset;
}
static int doc_editor_input_group(doc_editor_data* e, unsigned kind, xui_doc_range_t* range, uint64_t* group)
{
    int result; xui_widget w = e->view.widget; uint64_t now = e->action_time;
    int merge = kind && kind != DOC_EDITOR_ISOLATED && e->group && e->group_kind == kind &&
        e->desc.iUndoGroupTimeoutMs != UINT32_MAX && now >= e->group_time &&
        now - e->group_time <= (uint64_t)e->desc.iUndoGroupTimeoutMs * 1000 &&
        e->group_saved == e->view.document->saved_state &&
        (e->input || e->view.document->saved_state != e->view.document->state->content_id) &&
        doc_editor_group_position_equal(&e->group_end.tAnchor, &e->view.selection.tAnchor) &&
        doc_editor_group_position_equal(&e->group_end.tCaret, &e->view.selection.tCaret);
    if (kind && !merge && e->next_group == UINT64_MAX) return XUI_DOC_ERROR_LIMIT;
    *group = !kind ? 0 : merge ? e->group : e->next_group + 1;
    if (e->input && e->input->group != *group) {
        result = doc_editor_flush_input(e); if (result != XUI_OK) return result;
        if (!xuiInternalWidgetIsValid(w)) return XUI_ERROR_INVALID_STATE;
        /* Publication preserves the candidate's exact source byte offsets.
         * The edit range may be a local deletion/IME range, not selection. */
        if (range->tAnchor.iInputGeneration) { range->tAnchor.iInputGeneration = 0; range->tAnchor.iRevision = xuiDocumentGetRevision(e->view.document); }
        if (range->tCaret.iInputGeneration) { range->tCaret.iInputGeneration = 0; range->tCaret.iRevision = xuiDocumentGetRevision(e->view.document); }
    }
    return XUI_OK;
}
static int doc_editor_begin(doc_editor_data* e, xui_document_transaction* t, uint64_t group)
{
    xui_widget w = e->view.widget; xui_doc_txn_desc_t desc = {0}; int result;
    if (e->desc.bReadOnly) return XUI_ERROR_INVALID_STATE;
    if (e->stream_open) return XUI_DOC_ERROR_BUSY;
    if (doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    result = doc_editor_flush_input(e); if (result != XUI_OK) return result;
    if (!xuiInternalWidgetIsValid(w)) return XUI_ERROR_INVALID_STATE;
    if (e->view.document->pending) return XUI_DOC_ERROR_BUSY;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    desc.iSize = sizeof(desc); desc.iOrigin = e->origin; desc.iGroup = group; desc.iBaseRevision = xuiDocumentGetRevision(e->view.document);
    desc.iDomain = e->view.selection.tAnchor.iKind == XUI_DOC_POSITION_SOURCE ? XUI_DOC_SOURCE : XUI_DOC_SEMANTIC;
    return xuiDocumentBeginTransaction(e->view.document, &desc, t);
}
static int doc_editor_display_column_width(doc_editor_data* e, doc_state* state,
    uint64_t table, uint32_t column, float* out)
{
    doc_node* node = doc_index_get(state->index, table);
    doc_table_cell_slot first, last; xui_doc_cell_hit_t a = {0}, b = {0};
    uint32_t columns, automatic = 0, i; double fixed = 0, width, zoom;
    int result;
    if (!node || node->kind != XUI_DOC_TABLE) return XUI_ERROR_NOT_FOUND;
    columns = doc_table_column_count(state, node);
    if (column >= columns) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_table_column_width(node, column) > 0) {
        *out = doc_table_column_width(node, column); return XUI_OK;
    }
    for (i = 0; i < columns; i++) {
        float preferred = doc_table_column_width(node, i);
        if (preferred > 0) fixed += preferred;
        else automatic++;
    }
    if (!automatic) return XUI_DOC_ERROR_SCHEMA;
    result = doc_table_cell_at(state, table, 0, 0, &first);
    if (result == XUI_OK) result = doc_table_cell_at(state, table, 0, columns - 1, &last);
    a.iSize = b.iSize = sizeof(a);
    if (result == XUI_OK) result = xuiDocumentViewGetCellRect(e->view.widget, first.cell, &a);
    if (result == XUI_OK) result = xuiDocumentViewGetCellRect(e->view.widget, last.cell, &b);
    if (result != XUI_OK) return result;
    zoom = e->view.renderer->desc.fZoom;
    if (zoom <= 0) return XUI_ERROR_INVALID_STATE;
    width = ((b.tBounds.x + b.tBounds.width - a.tBounds.x) / zoom - fixed) / automatic;
    if (!isfinite(width) || width <= 0 || width > 1000000) return XUI_DOC_ERROR_SCHEMA;
    *out = (float)width; return XUI_OK;
}
static int doc_editor_pin_outer_auto_columns(doc_editor_data* e,
    xui_document_transaction t, uint64_t table, uint32_t column, int pin)
{
    doc_node* node = doc_index_get(t->base->index, table);
    uint32_t columns, i, automatic = UINT32_MAX; float width; int result;
    if (!pin || !node || node->kind != XUI_DOC_TABLE) return XUI_OK;
    columns = doc_table_column_count(t->base, node);
    if (!columns || column != columns - 1) return XUI_OK;
    for (i = 0; i < columns; i++) if (i != column &&
        doc_table_column_width(node, i) == 0) { automatic = i; break; }
    if (automatic == UINT32_MAX) return XUI_OK;
    result = doc_editor_display_column_width(e, t->base, table, automatic, &width);
    if (result != XUI_OK) return result;
    for (i = 0; i < columns; i++) if (i != column && doc_table_column_width(node, i) == 0) {
        result = xuiDocumentTxnSetTableColumnWidth(t, table, i, width);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
static int doc_editor_apply_column_width(doc_editor_data* e, uint64_t table, uint32_t column,
    float width, uint64_t group, int pin_outer_auto)
{
    xui_document_transaction t = NULL; xui_doc_range_t after; int result;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL ||
        xuiDocumentGetProfile(e->view.document) != XUI_DOCUMENT_RICH) return XUI_ERROR_UNSUPPORTED;
    after = e->view.selection;
    result = doc_editor_begin(e, &t, group); if (result != XUI_OK) return result;
    result = doc_editor_pin_outer_auto_columns(e, t, table, column, pin_outer_auto);
    if (result == XUI_OK) result = xuiDocumentTxnSetTableColumnWidth(t, table, column, width);
    if (result == XUI_OK) result = doc_editor_commit_ex(e, t, &after, 0);
    xuiDocumentTxnRelease(t); return result;
}
static int doc_editor_toggle_task_item(doc_editor_data* e, xui_doc_node_id item_id)
{
    xui_document_transaction transaction = NULL;
    xui_doc_range_t after; xui_doc_attributes_t attrs;
    doc_node* item; int result;
    if (e->desc.bReadOnly) return XUI_ERROR_INVALID_STATE;
    if (e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    item = doc_index_get(e->view.document->state->index, item_id);
    if (!item || item->kind != XUI_DOC_LIST_ITEM ||
        !(item->attrs->iFlags & XUI_DOC_TASK)) return XUI_ERROR_NOT_FOUND;
    doc_editor_cancel_task_marker(e, 1);
    if (!xuiInternalWidgetIsValid(e->view.widget)) return XUI_DOC_ERROR_STALE;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &transaction, 0);
    if (result != XUI_OK) return result;
    item = doc_index_get(transaction->draft->index, item_id);
    if (!item || item->kind != XUI_DOC_LIST_ITEM ||
        !(item->attrs->iFlags & XUI_DOC_TASK)) result = XUI_DOC_ERROR_STALE;
    else {
        attrs = *item->attrs; attrs.iFlags ^= XUI_DOC_CHECKED;
        after = e->view.selection;
        result = xuiDocumentTxnSetAttributes(transaction, item_id, &attrs);
        if (result == XUI_OK) result = doc_editor_commit_ex(e, transaction, &after, 0);
    }
    xuiDocumentTxnRelease(transaction);
    return result;
}
XUI_API int xuiDocumentEditorToggleTaskItem(xui_widget w, xui_doc_node_id item_id)
{
    doc_editor_data* e = doc_editor_get(w);
    if (!e || !item_id) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e)) return XUI_DOC_ERROR_BUSY;
    return doc_editor_toggle_task_item(e, item_id);
}
XUI_API int xuiDocumentEditorSetTableColumnWidth(xui_widget w, xui_doc_node_id table,
    uint32_t column, float width)
{
    doc_editor_data* e = doc_editor_get(w);
    if (!e || !isfinite(width) || width < 0 || width > 1000000) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e)) return XUI_DOC_ERROR_BUSY;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    doc_editor_cancel_resize(e, 1); e->action_time = xrtClock();
    return doc_editor_apply_column_width(e, table, column, width, 0, 0);
}
static int doc_editor_resize_hit(doc_editor_data* e, double x, double y,
    uint64_t* table_id, uint32_t* column, double* width)
{
    xui_widget widget = e->view.widget; xui_rect_t world = xuiWidgetGetWorldRect(widget);
    xui_doc_cell_hit_t hit = {0}; doc_node* table; uint32_t columns, edge, i, automatic = 0;
    double local_x = x - world.fX, local_y = y - world.fY, fixed = 0, zoom;
    float preferred; int result;
    if (e->desc.bReadOnly || e->view.desc.bDisableSelection ||
        e->view.renderer->mode != XUI_DOC_VISUAL ||
        xuiDocumentGetProfile(e->view.document) != XUI_DOCUMENT_RICH) return XUI_ERROR_NOT_FOUND;
    hit.iSize = sizeof(hit);
    result = xuiDocumentViewHitTestCell(widget, local_x - 5, local_y, &hit);
    if (result != XUI_OK) return result;
    if (fabs(hit.tBounds.x + hit.tBounds.width - local_x) > 4) return XUI_ERROR_NOT_FOUND;
    table = doc_index_get(e->view.renderer->snapshot->state->index, hit.iTableId);
    columns = doc_table_column_count(e->view.renderer->snapshot->state, table);
    edge = hit.iColumn + hit.iColumnSpan;
    if (!edge || edge > columns) return XUI_ERROR_NOT_FOUND;
    zoom = e->view.renderer->desc.fZoom;
    if (zoom <= 0) return XUI_ERROR_INVALID_STATE;
    result = xuiDocumentSnapshotGetTableColumnWidth(e->view.renderer->snapshot, hit.iTableId, edge - 1, &preferred);
    if (result != XUI_OK) return result;
    if (preferred > 0) *width = preferred;
    else {
        for (i = hit.iColumn; i < edge; i++) {
            result = xuiDocumentSnapshotGetTableColumnWidth(e->view.renderer->snapshot, hit.iTableId, i, &preferred);
            if (result != XUI_OK) return result;
            if (preferred > 0) fixed += preferred * zoom;
            else automatic++;
        }
        if (!automatic) return XUI_DOC_ERROR_SCHEMA;
        *width = (hit.tBounds.width - fixed) / automatic / zoom;
    }
    if (!isfinite(*width) || *width <= 0) return XUI_DOC_ERROR_SCHEMA;
    *table_id = hit.iTableId; *column = edge - 1; return XUI_OK;
}
static int doc_editor_resize_cursor(xui_widget w, int x, int y, void* user)
{
    doc_editor_data* e = doc_editor_get(w); uint64_t table; uint32_t column; double width; (void)user;
    if (!e) return XUI_CURSOR_INHERIT;
    if (e->resizing_column) return XUI_CURSOR_RESIZE_EW;
    return doc_editor_resize_hit(e, x, y, &table, &column, &width) == XUI_OK ?
        XUI_CURSOR_RESIZE_EW : XUI_CURSOR_INHERIT;
}
static int doc_editor_resize_event(doc_editor_data* e, const xui_event_t* event)
{
    xui_widget w = e->view.widget; int result;
    if (event->iType == XUI_EVENT_POINTER_DOWN &&
        (event->iButton == XUI_POINTER_BUTTON_LEFT || !event->iButton)) {
        uint64_t table; uint32_t column; double width;
        if (e->resizing_column) return XUI_EVENT_DISPATCH_STOP;
        if (event->iModifiers & XUI_MOD_ALT) return XUI_ERROR_NOT_FOUND;
        result = doc_editor_resize_hit(e, event->fX, event->fY, &table, &column, &width);
        if (result != XUI_OK) return result;
        if (e->next_group == UINT64_MAX) return XUI_DOC_ERROR_LIMIT;
        doc_editor_break_group(e);
        e->resize_group = ++e->next_group; e->resize_table = table; e->resize_column = column;
        e->resize_start_x = event->fX; e->resize_start_width = e->resize_current_width = width;
        e->resize_pointer_id = event->iPointerId; e->resize_pointer_type = event->iPointerType;
        e->resizing_column = 1;
        if (!e->view.replaying_input) {
            xuiSetFocusWidget(xuiWidgetGetContext(w), w);
            if (!xuiInternalWidgetIsValid(w)) return XUI_EVENT_DISPATCH_STOP;
            xuiSetPointerCaptureEx(xuiWidgetGetContext(w), event->iPointerId, event->iPointerType, w);
        }
        return XUI_EVENT_DISPATCH_STOP;
    }
    if (!e->resizing_column) return XUI_ERROR_NOT_FOUND;
    if (event->iType == XUI_EVENT_BLUR) {
        doc_editor_cancel_resize(e, 1); return XUI_ERROR_NOT_FOUND;
    }
    if (event->iType == XUI_EVENT_POINTER_CAPTURE_LOST) {
        if (event->iPointerId == e->resize_pointer_id && event->iPointerType == e->resize_pointer_type) {
            doc_editor_cancel_resize(e, 0); return XUI_EVENT_DISPATCH_STOP;
        }
        return XUI_ERROR_NOT_FOUND;
    }
    if ((event->iType == XUI_EVENT_POINTER_MOVE || event->iType == XUI_EVENT_POINTER_UP) &&
        (event->iPointerId != e->resize_pointer_id || event->iPointerType != e->resize_pointer_type))
        return XUI_EVENT_DISPATCH_STOP;
    if (event->iType == XUI_EVENT_POINTER_UP) {
        doc_editor_cancel_resize(e, !e->view.replaying_input);
        return XUI_EVENT_DISPATCH_STOP;
    }
    if (event->iType == XUI_EVENT_POINTER_MOVE) {
        double zoom = e->view.renderer->desc.fZoom;
        float width = (float)fmin(1000000, fmax(24,
            e->resize_start_width + (event->fX - e->resize_start_x) / zoom));
        if (fabs(width - e->resize_current_width) < .001) return XUI_EVENT_DISPATCH_STOP;
        result = doc_editor_apply_column_width(e, e->resize_table, e->resize_column,
            width, e->resize_group, 1);
        if (result != XUI_OK) return result;
        e->resize_current_width = width; return XUI_EVENT_DISPATCH_STOP;
    }
    doc_editor_cancel_resize(e, 1); return XUI_ERROR_NOT_FOUND;
}
static int doc_editor_list_context(doc_state* state, const xui_doc_position_t* at, int* empty, uint64_t* item_id)
{
    doc_node *n = doc_index_get(state->index, at->iNodeId), *item, *list;
    uint64_t i;
    if (at->iKind == XUI_DOC_POSITION_TEXT && n && n->kind == XUI_DOC_TEXT)
        n = doc_index_get(state->index, n->parent);
    if (n && (n->kind == XUI_DOC_PARAGRAPH || n->kind == XUI_DOC_HEADING)) {
        item = doc_index_get(state->index, n->parent);
    } else if (at->iKind == XUI_DOC_POSITION_GAP && n && n->kind == XUI_DOC_LIST_ITEM) item = n;
    else return 0;
    list = item && item->kind == XUI_DOC_LIST_ITEM ? doc_index_get(state->index, item->parent) : NULL;
    if (!list || list->kind != XUI_DOC_LIST) return 0;
    if (item_id) *item_id = item->id;
    *empty = 1;
    for (i = 0; i < doc_seq_size(item->children); i++) {
        doc_node* child = doc_index_get(state->index, doc_seq_get_id(item->children, i));
        if (!doc_semantic_empty_paragraph(state, child)) { *empty = 0; break; }
    }
    return *empty || (n && (n->kind == XUI_DOC_PARAGRAPH || n->kind == XUI_DOC_HEADING));
}
static doc_node* doc_editor_task_context(doc_state* state,
    const xui_doc_position_t* at)
{
    doc_node* node;
    if (at->iKind == XUI_DOC_POSITION_SOURCE) return NULL;
    node = doc_index_get(state->index, at->iNodeId);
    while (node && node->kind != XUI_DOC_LIST_ITEM)
        node = node->parent ? doc_index_get(state->index, node->parent) : NULL;
    return node && (node->attrs->iFlags & XUI_DOC_TASK) ? node : NULL;
}
static void doc_editor_retarget_draft_caret(xui_document_transaction t,
    xui_doc_position_t* caret)
{
    uint64_t i;
    if (doc_position_valid(t->draft, caret)) return;
    for (i = 0; i < t->count; i++) if (t->ops[i].iKind == XUI_DOC_OP_SPLIT &&
        caret->iNodeId == t->ops[i].iNodeId && caret->iOffset > t->ops[i].iOffset) {
        caret->iNodeId = t->ops[i].iOtherNodeId;
        caret->iOffset -= t->ops[i].iOffset;
    }
}
static int doc_editor_replace(doc_editor_data* e, const xui_doc_range_t* range, const char* text, uint64_t bytes, unsigned kind)
{
    xui_widget w = e->view.widget; xui_document_transaction t = NULL; xui_doc_position_t caret = {0};
    xui_doc_range_t after, edit = *range; uint64_t group, received = e->action_time; int result, order;
    if (e->stream_open) return XUI_DOC_ERROR_BUSY;
    result = doc_render_compare(e->view.renderer, &edit.tAnchor, &edit.tCaret, &order); if (result != XUI_OK) return result;
    if (!bytes && !order) return XUI_OK;
    result = doc_editor_input_group(e, kind, &edit, &group); if (result != XUI_OK) return result;
    range = &edit;
    if (e->view.renderer->mode == XUI_DOC_SOURCE_TEXT ||
        e->view.renderer->mode == XUI_DOC_LIVE_MARKDOWN) {
        uint64_t source_bytes = doc_seq_size(doc_render_source(e->view.renderer)), limit = e->desc.iAsyncSourceThresholdBytes;
        if (e->input || (limit != UINT64_MAX && (source_bytes >= limit || bytes >= limit - source_bytes))) {
            result = doc_editor_prepare_replace(e, range, text, bytes, group); goto done;
        }
    }
    if (e->view.renderer->mode == XUI_DOC_VISUAL &&
        xuiDocumentGetProfile(e->view.document) == XUI_DOCUMENT_MARKDOWN) {
        uint64_t limit = e->desc.iAsyncSourceThresholdBytes;
        uint64_t source_bytes = doc_seq_size(e->view.renderer->snapshot->state->source);
        if (e->visual_input) {
            result = doc_editor_continue_visual_text(e, range, text, bytes, group); goto done;
        }
        if (limit != UINT64_MAX && (source_bytes >= limit || bytes >= limit - source_bytes) &&
            !e->pending_clear_format && !e->has_pending_marks && !e->pending_style_fields) {
            result = doc_editor_prepare_visual_text(e, range, text, bytes, group);
            if (result != XUI_ERROR_UNSUPPORTED) goto done;
        }
    }
    result = doc_editor_begin(e, &t, group); if (result != XUI_OK) return result;
    if (!order && bytes == 1 && text[0] == '\n' && e->view.renderer->mode == XUI_DOC_VISUAL) {
        int empty;
        if (doc_editor_list_context(t->draft, &range->tCaret, &empty, NULL))
            result = empty ? xuiDocumentTxnExitListItem(t, &range->tCaret, &caret) :
                xuiDocumentTxnSplitListItem(t, &range->tCaret, &caret);
        else result = xuiDocumentTxnReplaceRange(t, range, text, bytes, &caret);
    } else result = xuiDocumentTxnReplaceRange(t, range, text, bytes, &caret);
    if (result == XUI_OK && bytes &&
        (e->pending_clear_format || e->has_pending_marks || e->pending_style_fields) &&
        caret.iKind == XUI_DOC_POSITION_TEXT) {
        if (memchr(text, '\n', (size_t)bytes) || memchr(text, '\r', (size_t)bytes)) {
            uint64_t i;
            for (i = 0; i < bytes && (text[i] == '\n' || text[i] == '\r'); i++) {}
            if (i < bytes) {
                struct xui_doc_change_set_t changes = {0};
                xui_doc_range_t inserted; int mapping;
                changes.before = t->base; changes.after = t->draft;
                changes.identity = t->document->identity;
                changes.before_revision = changes.after_revision = t->base_revision;
                changes.ops = t->ops; changes.count = t->count;
                inserted.tAnchor = range->tAnchor;
                inserted.tAnchor.iAffinity = XUI_DOC_BEFORE;
                inserted.tCaret = caret;
                result = xuiDocumentMapPosition(&changes, &inserted.tAnchor,
                    &inserted.tAnchor, &mapping);
                if (result == XUI_OK && e->pending_clear_format)
                    result = xuiDocumentTxnClearFormatting(t, &inserted);
                if (result == XUI_OK) doc_editor_retarget_draft_caret(t, &caret);
                if (result == XUI_OK && e->has_pending_marks) {
                    doc_editor_retarget_draft_caret(t, &inserted.tAnchor);
                    inserted.tCaret = caret;
                    result = xuiDocumentTxnSetMarks(t, &inserted,
                        e->pending_set, e->pending_clear);
                    if (result == XUI_OK) doc_editor_retarget_draft_caret(t, &caret);
                }
                if (result == XUI_OK && e->pending_style_fields) {
                    doc_editor_retarget_draft_caret(t, &inserted.tAnchor);
                    inserted.tCaret = caret;
                    result = xuiDocumentTxnSetTextStyle(t, &inserted,
                        e->pending_style_fields, &e->pending_style);
                    if (result == XUI_OK) doc_editor_retarget_draft_caret(t, &caret);
                }
            }
        }
        if (caret.iOffset >= bytes && !memchr(text, '\n', (size_t)bytes) && !memchr(text, '\r', (size_t)bytes)) {
            if (e->pending_clear_format) {
                xui_doc_range_t plain; plain.tAnchor = plain.tCaret = caret;
                plain.tAnchor.iOffset -= bytes;
                result = xuiDocumentTxnClearFormatting(t, &plain);
                if (result == XUI_OK) doc_editor_retarget_draft_caret(t, &caret);
            }
            if (result == XUI_OK && e->has_pending_marks) {
                xui_doc_range_t marked; marked.tAnchor = marked.tCaret = caret; marked.tAnchor.iOffset -= bytes;
                result = xuiDocumentTxnSetMarks(t, &marked, e->pending_set, e->pending_clear);
            }
            /* Formatting may split the inserted run. Follow its final boundary
             * before applying another format or publishing the caret. */
            if (result == XUI_OK) doc_editor_retarget_draft_caret(t, &caret);
            if (result == XUI_OK && e->pending_style_fields) {
                xui_doc_range_t styled; styled.tAnchor = styled.tCaret = caret; styled.tAnchor.iOffset -= bytes;
                result = xuiDocumentTxnSetTextStyle(t, &styled, e->pending_style_fields, &e->pending_style);
                if (result == XUI_OK) doc_editor_retarget_draft_caret(t, &caret);
            }
        }
    }
    after.tAnchor = after.tCaret = caret;
    if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    xuiDocumentTxnRelease(t);
done:
    if (result == XUI_OK && xuiInternalWidgetIsValid(w)) {
        if (group > e->next_group) e->next_group = group;
        e->group = kind == DOC_EDITOR_ISOLATED ? 0 : group; e->group_kind = kind;
        e->group_time = received; e->group_saved = e->view.document->saved_state; e->group_end = e->view.selection;
    }
    return result;
}
static int doc_editor_paste_matrix(doc_editor_data*, const char*, uint64_t);
static int doc_editor_clear_table_selection(doc_editor_data*);
static int doc_editor_insert_text(doc_editor_data* e, const char* text, uint64_t bytes, unsigned kind)
{
    int result;
    if (!doc_utf8(text, bytes)) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    if (e->desc.bReadOnly) return XUI_ERROR_INVALID_STATE;
    if (e->view.table_selection.iTableId) {
        if (!bytes) return doc_editor_clear_table_selection(e);
        if (memchr(text, '\n', (size_t)bytes) || memchr(text, '\r', (size_t)bytes))
            return XUI_ERROR_UNSUPPORTED;
        return doc_editor_paste_matrix(e, text, bytes);
    }
    if (kind && (memchr(text, '\n', (size_t)bytes) || memchr(text, '\r', (size_t)bytes))) kind = DOC_EDITOR_ISOLATED;
    doc_editor_cancel_composition(e); return doc_editor_replace(e, &e->view.selection, text, bytes, kind);
}
XUI_API int xuiDocumentEditorInsertText(xui_widget w, const char* text, uint64_t bytes)
{
    doc_editor_data* e = doc_editor_get(w);
    if (!e || !doc_utf8(text, bytes)) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e)) return XUI_DOC_ERROR_BUSY;
    e->action_time = xrtClock(); return doc_editor_insert_text(e, text, bytes, DOC_EDITOR_PROGRAM);
}
static int doc_editor_set_marks(doc_editor_data* e, uint32_t set, uint32_t clear)
{
    xui_document_transaction t = NULL; xui_doc_range_t after; int order, result;
    if ((set | clear) & XUI_DOC_LINK) return XUI_ERROR_UNSUPPORTED;
    if (doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    result = doc_render_compare(e->view.renderer, &e->view.selection.tAnchor, &e->view.selection.tCaret, &order);
    if (result != XUI_OK) return result;
    doc_editor_break_group(e);
    if (!order) {
        xui_doc_attributes_t attrs = {0}; attrs.iMarks = set;
        if (!doc_schema_attrs(XUI_DOC_TEXT, &attrs)) return XUI_DOC_ERROR_SCHEMA;
        e->pending_set = (e->pending_set | set) & ~clear; e->pending_clear = (e->pending_clear | clear) & ~set; e->has_pending_marks = 1; return XUI_OK;
    }
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
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
XUI_API int xuiDocumentEditorSetMarks(xui_widget w, uint32_t set, uint32_t clear)
{
    doc_editor_data* e = doc_editor_get(w); if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    return doc_editor_api_busy(e) ? XUI_DOC_ERROR_BUSY : doc_editor_set_marks(e, set, clear);
}
static void doc_editor_style_overlay(xui_doc_text_style_t* target, uint32_t fields,
    const xui_doc_text_style_t* value)
{
    target->iSize = sizeof(*target);
    target->iExplicitFields = (target->iExplicitFields & ~fields) |
        (value->iExplicitFields & fields &
            (XUI_DOC_TEXT_STYLE_COLOR | XUI_DOC_TEXT_STYLE_BACKGROUND));
    target->iCurrentColorFields = (target->iCurrentColorFields & ~fields) |
        (value->iCurrentColorFields & fields &
            (XUI_DOC_TEXT_STYLE_COLOR | XUI_DOC_TEXT_STYLE_BACKGROUND));
    if (fields & XUI_DOC_TEXT_STYLE_COLOR) target->iTextColor = value->iTextColor;
    if (fields & XUI_DOC_TEXT_STYLE_BACKGROUND) target->iBackgroundColor = value->iBackgroundColor;
    if (fields & XUI_DOC_TEXT_STYLE_FONT_SIZE) target->fFontSize = value->fFontSize;
    if (fields & XUI_DOC_TEXT_STYLE_LANGUAGE)
        strcpy(target->sLanguage, value->sLanguage);
    if (fields & XUI_DOC_TEXT_STYLE_FONT_FAMILY) {
        size_t bytes = strlen(value->sFontFamily);
        memset(target->sFontFamily, 0, sizeof(target->sFontFamily));
        memcpy(target->sFontFamily, value->sFontFamily, bytes);
    }
}
XUI_API int xuiDocumentEditorQueryTextStyle(xui_widget w, xui_doc_editor_text_style_state_t* state)
{
    doc_editor_data* e = doc_editor_get(w); int result, order;
    if (!e || !state || state->iSize != sizeof(*state)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(state, 0, sizeof(*state)); state->iSize = sizeof(*state);
    state->tQuery.iSize = sizeof(state->tQuery);
    state->tQuery.tStyle.iSize = sizeof(state->tQuery.tStyle);
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) {
        state->iDisabledReason = XUI_DOC_ERROR_BUSY; return XUI_OK;
    }
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    if (e->desc.bReadOnly) state->iDisabledReason = XUI_ERROR_INVALID_STATE;
    else if (e->view.renderer->mode != XUI_DOC_VISUAL) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
    else if (e->view.document->state->profile != XUI_DOCUMENT_RICH)
        state->iDisabledReason = XUI_DOC_ERROR_UNREPRESENTABLE;
    if (state->iDisabledReason) return XUI_OK;
    result = xuiDocumentSnapshotQueryTextStyle(e->view.renderer->snapshot, &e->view.selection, &state->tQuery);
    if (result != XUI_OK) return result;
    result = doc_render_compare(e->view.renderer, &e->view.selection.tAnchor, &e->view.selection.tCaret, &order);
    if (result != XUI_OK) return result;
    if (!order && e->pending_clear_format) {
        char language[256];
        strcpy(language, state->tQuery.tStyle.sLanguage);
        memset(&state->tQuery.tStyle, 0, sizeof(state->tQuery.tStyle));
        strcpy(state->tQuery.tStyle.sLanguage, language);
        state->tQuery.tStyle.iSize = sizeof(state->tQuery.tStyle);
        state->tQuery.iMixedFields = 0;
        state->tQuery.bHasText = 1;
    }
    if (!order && e->pending_style_fields) {
        doc_editor_style_overlay(&state->tQuery.tStyle, e->pending_style_fields, &e->pending_style);
        state->tQuery.bHasText = 1;
    }
    state->bEnabled = !order || state->tQuery.bHasText;
    if (!state->bEnabled) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
    return XUI_OK;
}
XUI_API int xuiDocumentEditorSetTextStyle(xui_widget w, uint32_t fields,
    const xui_doc_text_style_t* style)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    xui_doc_range_t after; int result, order;
    if (!e || !doc_text_style_valid(fields, style)) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    if (e->view.document->state->profile != XUI_DOCUMENT_RICH) return XUI_DOC_ERROR_UNREPRESENTABLE;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    result = doc_render_compare(e->view.renderer, &e->view.selection.tAnchor, &e->view.selection.tCaret, &order);
    if (result != XUI_OK) return result;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    if (!order) {
        doc_editor_style_overlay(&e->pending_style, fields, style);
        e->pending_style_fields |= fields;
        return XUI_OK;
    }
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    after = e->view.selection;
    result = xuiDocumentTxnSetTextStyle(t, &after, fields, style);
    if (result == XUI_OK) {
        struct xui_doc_change_set_t changes = {0}; int mapping;
        changes.before = t->base; changes.after = t->draft; changes.identity = t->document->identity;
        changes.before_revision = changes.after_revision = t->base_revision;
        changes.ops = t->ops; changes.count = t->count;
        result = xuiDocumentMapPosition(&changes, &e->view.selection.tAnchor, &after.tAnchor, &mapping);
        if (result == XUI_OK) result = xuiDocumentMapPosition(&changes, &e->view.selection.tCaret, &after.tCaret, &mapping);
        if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorInsertLink(xui_widget w, const char* label, uint64_t label_bytes,
    const char* uri, const char* title)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    xui_doc_position_t caret; xui_doc_range_t after; int result;
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnInsertLink(t, &e->view.selection, label, label_bytes, uri, title, &caret);
    if (result == XUI_OK) {
        after.tAnchor = after.tCaret = caret;
        result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorSetLink(xui_widget w, const char* uri, const char* title)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    xui_doc_range_t after; int order, result;
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    result = doc_render_compare(e->view.renderer, &e->view.selection.tAnchor, &e->view.selection.tCaret, &order);
    if (result != XUI_OK) return result;
    if (!order) return uri && *uri ? xuiDocumentEditorInsertLink(w, uri, strlen(uri), uri, title) : XUI_ERROR_UNSUPPORTED;
    doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    after = e->view.selection; result = xuiDocumentTxnSetLink(t, &after, uri, title);
    if (result == XUI_OK) {
        struct xui_doc_change_set_t changes = {0}; int mapping;
        changes.before = t->base; changes.after = t->draft; changes.identity = t->document->identity;
        changes.before_revision = changes.after_revision = t->base_revision;
        changes.ops = t->ops; changes.count = t->count;
        result = xuiDocumentMapPosition(&changes, &e->view.selection.tAnchor, &after.tAnchor, &mapping);
        if (result == XUI_OK) result = xuiDocumentMapPosition(&changes, &e->view.selection.tCaret, &after.tCaret, &mapping);
        if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorInsertImage(xui_widget w, const xui_doc_image_desc_t* image)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    xui_doc_position_t caret; xui_doc_range_t after; xui_doc_node_id image_id; int result;
    if (!e || !image) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnInsertImage(t, &e->view.selection, image, &image_id, &caret);
    if (result == XUI_OK) {
        after.tAnchor = after.tCaret = caret;
        result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorInsertObject(xui_widget w, uint32_t kind,
    uint32_t flags, const char* utf8, uint64_t bytes)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    xui_doc_position_t caret; xui_doc_range_t after; xui_doc_node_id object_id;
    int result;
    if (!e || (!utf8 && bytes) || bytes >= SIZE_MAX ||
        !doc_utf8(utf8, bytes) || (bytes && memchr(utf8, 0, (size_t)bytes)))
        return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnInsertObject(t, &e->view.selection,
        kind, flags, utf8, bytes, &object_id, &caret);
    if (result == XUI_OK) {
        after.tAnchor = after.tCaret = caret;
        result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorInsertCodeBlock(xui_widget w,
    const char* language, const char* utf8, uint64_t bytes)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    xui_doc_position_t caret; xui_doc_range_t after; xui_doc_node_id code_id;
    int result;
    if (!e || (!utf8 && bytes) || bytes >= SIZE_MAX || !doc_utf8(utf8, bytes) ||
        (bytes && memchr(utf8, 0, (size_t)bytes))) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnInsertCodeBlock(t, &e->view.selection,
        language, utf8, bytes, &code_id, &caret);
    if (result == XUI_OK) {
        after.tAnchor = after.tCaret = caret;
        result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorSetCodeBlockLanguage(xui_widget w,
    xui_doc_node_id code_id, const char* language)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    xui_doc_range_t after; int mapping, result;
    if (!e || !code_id) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    after = e->view.selection;
    result = xuiDocumentTxnSetCodeBlockLanguage(t, code_id, language);
    if (result == XUI_OK) {
        struct xui_doc_change_set_t changes = {0};
        changes.before = t->base; changes.after = t->draft;
        changes.identity = t->document->identity;
        changes.before_revision = changes.after_revision = t->base_revision;
        changes.ops = t->ops; changes.count = t->count;
        result = xuiDocumentMapPosition(&changes, &e->view.selection.tAnchor,
            &after.tAnchor, &mapping);
        if (result == XUI_OK) result = xuiDocumentMapPosition(&changes,
            &e->view.selection.tCaret, &after.tCaret, &mapping);
        if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorInsertRule(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    xui_doc_position_t caret; xui_doc_range_t after; xui_doc_node_id rule_id;
    int result;
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnInsertRule(t, &e->view.selection, &rule_id, &caret);
    if (result == XUI_OK) {
        after.tAnchor = after.tCaret = caret;
        result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorWrapQuote(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    xui_doc_range_t after; xui_doc_node_id quote_id; int result;
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    if (e->view.table_selection.iTableId) return XUI_ERROR_UNSUPPORTED;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnWrapQuoteRange(t, &e->view.selection, &quote_id, &after);
    if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorUnwrapQuote(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    xui_doc_position_t caret; xui_doc_range_t after; int order, result;
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    if (e->view.table_selection.iTableId) return XUI_ERROR_UNSUPPORTED;
    result = doc_render_compare(e->view.renderer, &e->view.selection.tAnchor,
        &e->view.selection.tCaret, &order);
    if (result != XUI_OK) return result;
    if (order) return XUI_ERROR_UNSUPPORTED;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnUnwrapQuote(t, &e->view.selection.tCaret, &caret);
    if (result == XUI_OK) {
        after.tAnchor = after.tCaret = caret;
        result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorInsertFootnote(xui_widget w,
    const char* label, const char* initial_utf8, uint64_t initial_bytes)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    xui_doc_position_t reference_caret, body_caret; xui_doc_range_t after;
    xui_doc_node_id reference_id, footnote_id; int result;
    if (!e || (!initial_utf8 && initial_bytes) || initial_bytes >= SIZE_MAX ||
        !doc_utf8(initial_utf8, initial_bytes) ||
        (initial_bytes && memchr(initial_utf8, 0, (size_t)initial_bytes)))
        return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnInsertFootnote(t, &e->view.selection,
        label, initial_utf8, initial_bytes,
        &reference_id, &footnote_id, &reference_caret, &body_caret);
    if (result == XUI_OK) {
        after.tAnchor = after.tCaret = body_caret;
        result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorRemoveFootnoteReference(xui_widget w,
    xui_doc_node_id reference_id)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    xui_doc_position_t caret; xui_doc_range_t after; int result;
    if (!e || !reference_id) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnRemoveFootnoteReference(t, reference_id, &caret);
    if (result == XUI_OK) {
        after.tAnchor = after.tCaret = caret;
        result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorUpdateImage(xui_widget w, xui_doc_node_id image_id,
    const xui_doc_image_desc_t* image)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    xui_doc_range_t after; int mapping, result;
    if (!e || !image) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    after = e->view.selection;
    result = xuiDocumentTxnUpdateImage(t, image_id, image);
    if (result == XUI_OK) {
        struct xui_doc_change_set_t changes = {0};
        changes.before = t->base; changes.after = t->draft; changes.identity = t->document->identity;
        changes.before_revision = changes.after_revision = t->base_revision;
        changes.ops = t->ops; changes.count = t->count;
        result = xuiDocumentMapPosition(&changes, &e->view.selection.tAnchor, &after.tAnchor, &mapping);
        if (result == XUI_OK) result = xuiDocumentMapPosition(&changes, &e->view.selection.tCaret, &after.tCaret, &mapping);
        if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorUpdateObjectSource(xui_widget w,
    xui_doc_node_id object_id, const char* utf8, uint64_t bytes)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    doc_node* object; xui_doc_range_t after; int mapping, result;
    if (!e || !object_id || (!utf8 && bytes) || bytes >= SIZE_MAX ||
        !doc_utf8(utf8, bytes) || (bytes && memchr(utf8, 0, (size_t)bytes)))
        return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    object = doc_index_get(t->draft->index, object_id);
    if (!object || (object->kind != XUI_DOC_MATH && object->kind != XUI_DOC_DIAGRAM &&
        object->kind != XUI_DOC_HTML && object->kind != XUI_DOC_CODE_BLOCK)) {
        xuiDocumentTxnRelease(t); return XUI_ERROR_INVALID_ARGUMENT;
    }
    after = e->view.selection;
    result = xuiDocumentTxnReplaceText(t, object_id, 0, doc_seq_size(object->text),
        utf8 ? utf8 : "", bytes);
    if (result == XUI_OK) {
        struct xui_doc_change_set_t changes = {0};
        changes.before = t->base; changes.after = t->draft; changes.identity = t->document->identity;
        changes.before_revision = changes.after_revision = t->base_revision;
        changes.ops = t->ops; changes.count = t->count;
        result = xuiDocumentMapPosition(&changes, &e->view.selection.tAnchor, &after.tAnchor, &mapping);
        if (result == XUI_OK) result = xuiDocumentMapPosition(&changes, &e->view.selection.tCaret, &after.tCaret, &mapping);
        if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
XUI_API int xuiDocumentEditorSetReadOnly(xui_widget w, int read_only)
{
    doc_editor_data* e = doc_editor_get(w); int result; if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (e->desc.bReadOnly == !!read_only) return XUI_OK;
    result = xuiDocumentEditorFlush(w); if (result != XUI_OK || !xuiInternalWidgetIsValid(w)) return result;
    doc_editor_break_group(e);
    if (read_only) {
        doc_editor_cancel_resize(e, 1);
        doc_editor_cancel_task_marker(e, 1);
    }
    e->desc.bReadOnly = !!read_only; if (read_only) doc_editor_cancel_composition(e);
    return xuiWidgetSetImeMode(w, read_only ? XUI_IME_DISABLED : XUI_IME_ENABLED);
}
XUI_API int xuiDocumentEditorReplaceCurrentEx(xui_widget w,
    const char* pattern, uint64_t pattern_bytes,
    const char* replacement, uint64_t replacement_bytes, uint32_t flags)
{
    doc_editor_data* e = doc_editor_get(w);
    xui_doc_range_t selected;
    doc_find_expansion* expansions = NULL;
    doc_allocator* expansion_allocator;
    uint32_t search_flags = flags & ~XUI_DOC_REPLACE_EXPAND;
    uint64_t i;
    int result;
    if (!e || !pattern || !pattern_bytes || (!replacement && replacement_bytes))
        return XUI_ERROR_INVALID_ARGUMENT;
    if ((flags & ~(XUI_DOC_FIND_REGEX | XUI_DOC_FIND_IGNORE_CASE |
            XUI_DOC_FIND_WHOLE_WORD | XUI_DOC_REPLACE_EXPAND)) ||
        ((flags & XUI_DOC_REPLACE_EXPAND) && !(flags & XUI_DOC_FIND_REGEX)))
        return XUI_ERROR_INVALID_ARGUMENT;
    if (e->desc.bReadOnly) return XUI_ERROR_INVALID_STATE;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    result = xuiDocumentViewSetFindQueryEx(w, pattern, pattern_bytes, search_flags);
    if (result != XUI_OK) return result;
    selected = e->view.selection;
    for (i = 0; i < e->view.find_count; i++) {
        const xui_doc_range_t* match = &e->view.find_matches[i];
        if (selected.tAnchor.iKind == match->tAnchor.iKind &&
            selected.tAnchor.iNodeId == match->tAnchor.iNodeId &&
            selected.tAnchor.iOffset == match->tAnchor.iOffset &&
            selected.tCaret.iKind == match->tCaret.iKind &&
            selected.tCaret.iNodeId == match->tCaret.iNodeId &&
            selected.tCaret.iOffset == match->tCaret.iOffset) {
            if (flags & XUI_DOC_REPLACE_EXPAND) {
                uint32_t domain = e->view.renderer->mode == XUI_DOC_VISUAL ?
                    XUI_DOC_SEMANTIC : XUI_DOC_SOURCE;
                uint64_t expansion_count = e->view.find_count;
                result = doc_find_expand_matches(e->view.renderer->snapshot,
                    domain, pattern, pattern_bytes, search_flags,
                    e->view.find_scope_active ? &e->view.find_scope : NULL,
                    e->view.find_matches, expansion_count,
                    replacement, replacement_bytes, &expansions);
                if (result != XUI_OK) return result;
                expansion_allocator = e->view.renderer->snapshot->state->allocator;
                doc_allocator_retain(expansion_allocator);
                doc_editor_break_group(e);
                result = doc_editor_replace(e, &selected, expansions[i].text,
                    expansions[i].bytes, DOC_EDITOR_PROGRAM);
                doc_find_expansions_free(expansions, expansion_count);
                doc_allocator_release(expansion_allocator);
                return result;
            }
            doc_editor_break_group(e);
            return doc_editor_replace(e, &selected, replacement ? replacement : "",
                replacement_bytes, DOC_EDITOR_PROGRAM);
        }
    }
    return XUI_ERROR_NOT_FOUND;
}
XUI_API int xuiDocumentEditorReplaceCurrent(xui_widget w,
    const char* pattern, uint64_t pattern_bytes,
    const char* replacement, uint64_t replacement_bytes)
{
    return xuiDocumentEditorReplaceCurrentEx(w, pattern, pattern_bytes,
        replacement, replacement_bytes, 0);
}
XUI_API int xuiDocumentEditorReplaceAllEx(xui_widget w,
    const char* pattern, uint64_t pattern_bytes,
    const char* replacement, uint64_t replacement_bytes, uint32_t flags,
    const xui_doc_range_t* scope, uint64_t* replaced)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL; xui_doc_range_t after; uint64_t count; int result, mapping;
    if (replaced) *replaced = 0;
    if (!e || !replaced) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    doc_editor_break_group(e); doc_editor_cancel_composition(e); result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnReplaceAllEx(t, pattern, pattern_bytes,
        replacement, replacement_bytes, flags, scope, &count);
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
XUI_API int xuiDocumentEditorReplaceAll(xui_widget w, const char* pattern,
    uint64_t pattern_bytes, const char* replacement,
    uint64_t replacement_bytes, const xui_doc_range_t* scope,
    uint64_t* replaced)
{
    return xuiDocumentEditorReplaceAllEx(w, pattern, pattern_bytes,
        replacement, replacement_bytes, 0, scope, replaced);
}
XUI_API int xuiDocumentEditorOpenFind(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w);
    return e ? doc_find_ui_open(w, &e->find_ui, 0) : XUI_ERROR_INVALID_ARGUMENT;
}
XUI_API int xuiDocumentEditorOpenReplace(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w);
    return e ? doc_find_ui_open(w, &e->find_ui, 1) : XUI_ERROR_INVALID_ARGUMENT;
}
XUI_API xui_widget xuiDocumentEditorGetFindWindow(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w);
    return e ? doc_find_ui_window(e->find_ui) : NULL;
}
XUI_API int xuiDocumentEditorGetReadOnly(xui_widget w) { doc_editor_data* e = doc_editor_get(w); return !e || e->desc.bReadOnly; }

typedef struct doc_edit_span { uint64_t id; int start, end; unsigned kind; } doc_edit_span;
typedef struct doc_edit_projection { char* text; doc_edit_span* spans; size_t count; int length, offset; uint64_t block; } doc_edit_projection;
typedef struct doc_edit_source_reader {
    doc_sequence* source;
    int length, start, size, error;
    unsigned char bytes[1024];
} doc_edit_source_reader;
static int doc_editor_source_read(void* user, int offset, unsigned char* byte)
{
    doc_edit_source_reader* reader = user;
    if (offset < 0 || offset >= reader->length || !byte || reader->error) return 0;
    if (offset < reader->start || offset - reader->start >= reader->size) {
        reader->start = offset & ~1023;
        reader->size = reader->length - reader->start;
        if (reader->size > (int)sizeof(reader->bytes)) reader->size = (int)sizeof(reader->bytes);
        reader->error = doc_seq_read(reader->source, (uint64_t)reader->start, reader->bytes,
            (uint64_t)reader->size);
        if (reader->error != XUI_OK) return 0;
    }
    *byte = reader->bytes[offset - reader->start];
    return 1;
}
static int doc_editor_project(doc_editor_data* e, const xui_doc_position_t* p, doc_edit_projection* out)
{
    doc_state* s = e->view.renderer->snapshot->state; doc_node* n = doc_index_get(s->index, p->iNodeId); uint64_t total = 0, i, count;
    memset(out, 0, sizeof(*out));
    if (!n) return XUI_ERROR_NOT_FOUND;
    if (n->kind == XUI_DOC_TEXT || doc_selectable_object_kind(n->kind))
        n = doc_index_get(s->index, n->parent);
    if (!n) return XUI_DOC_ERROR_SCHEMA;
    out->block = n->id; count = doc_text_kind(n->kind) ? 1 : doc_seq_size(n->children);
    if (count > SIZE_MAX / sizeof(*out->spans)) return XUI_DOC_ERROR_LIMIT;
    out->spans = calloc((size_t)count + 1, sizeof(*out->spans)); if (!out->spans) return XUI_ERROR_OUT_OF_MEMORY;
    for (i = 0; i < count; i++) {
        doc_node* child = doc_text_kind(n->kind) ? n : doc_index_get(s->index, doc_seq_get_id(n->children, i));
        uint64_t length = doc_selectable_object_kind(child->kind) ? 3 :
            doc_text_kind(child->kind) ? doc_seq_size(child->text) : 1;
        if (length > INT_MAX || total > INT_MAX - length) { free(out->spans); return XUI_DOC_ERROR_LIMIT; }
        out->spans[i] = (doc_edit_span){child->id, (int)total, (int)(total + length), child->kind}; total += length;
    }
    out->text = malloc((size_t)total + 1); if (!out->text) { free(out->spans); return XUI_ERROR_OUT_OF_MEMORY; }
    out->length = (int)total; out->count = (size_t)count;
    for (i = 0; i < count; i++) {
        doc_edit_span* span = &out->spans[i]; doc_node* child = doc_index_get(s->index, span->id);
        if (doc_selectable_object_kind(child->kind))
            memcpy(out->text + span->start, "\xef\xbf\xbc", 3);
        else if (doc_text_kind(child->kind))
            doc_seq_read(child->text, 0, out->text + span->start,
                (uint64_t)(span->end - span->start));
        else out->text[span->start] = child->kind == XUI_DOC_SOFT_BREAK || child->kind == XUI_DOC_HARD_BREAK ? '\n' : ' ';
        if (p->iKind == XUI_DOC_POSITION_TEXT && span->id == p->iNodeId)
            out->offset = span->start +
                (doc_selectable_object_kind(child->kind) ? (p->iOffset ? 3 : 0) :
                    (int)p->iOffset);
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
            if (!doc_text_kind(span->kind) || doc_selectable_object_kind(span->kind)) {
                p.iNodeId = projection->block; p.iKind = XUI_DOC_POSITION_GAP;
                p.iOffset = i + (offset >= span->end || (after && offset > span->start));
            }
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
XUI_API int xuiDocumentEditorInsertTable(xui_widget w, uint32_t rows, uint32_t columns)
{
    doc_editor_data* e = doc_editor_get(w); xui_document_transaction t = NULL;
    doc_state* s; doc_node *n, *block = NULL, *parent_node;
    xui_doc_position_t position, edge, split; xui_doc_range_t after;
    uint64_t parent = 0, index = 0, table = 0; int order, comparison, split_block = 0, result;
    if (!e || !rows || !columns || columns > 1024) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e) || doc_editor_blocked(e)) return XUI_DOC_ERROR_BUSY;
    if (e->desc.bReadOnly) return XUI_ERROR_INVALID_STATE;
    if (e->view.renderer->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    if (xuiDocumentGetProfile(e->view.document) == XUI_DOCUMENT_MARKDOWN &&
        xuiDocumentGetMarkdownDialect(e->view.document) == XUI_MD_COMMONMARK)
        return XUI_DOC_ERROR_UNREPRESENTABLE;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    result = doc_render_compare(e->view.renderer, &e->view.selection.tAnchor,
        &e->view.selection.tCaret, &order);
    if (result != XUI_OK) return result;
    if (order || e->view.table_selection.iTableId) return XUI_ERROR_UNSUPPORTED;
    s = e->view.renderer->snapshot->state; position = e->view.selection.tCaret;
    n = doc_index_get(s->index, position.iNodeId);
    if (position.iKind == XUI_DOC_POSITION_TEXT) {
        if (!n || n->kind != XUI_DOC_TEXT) return XUI_ERROR_UNSUPPORTED;
        while (n && n->kind != XUI_DOC_PARAGRAPH && n->kind != XUI_DOC_HEADING)
            n = doc_index_get(s->index, n->parent);
        block = n;
    } else if (position.iKind == XUI_DOC_POSITION_GAP && n &&
        (n->kind == XUI_DOC_PARAGRAPH || n->kind == XUI_DOC_HEADING)) block = n;
    else if (position.iKind == XUI_DOC_POSITION_GAP && n &&
        doc_schema_child(n->kind, XUI_DOC_TABLE)) {
        parent = n->id; index = position.iOffset;
    } else return XUI_ERROR_UNSUPPORTED;
    if (block) {
        parent_node = doc_index_get(s->index, block->parent);
        if (!parent_node || !doc_schema_child(parent_node->kind, XUI_DOC_TABLE))
            return XUI_ERROR_UNSUPPORTED;
        parent = parent_node->id; index = doc_child_index(parent_node, block->id);
        edge = doc_editor_edge(s, position, block->id, 0);
        result = doc_position_compare(s, &position, &edge, &comparison);
        if (result != XUI_OK) return result;
        if (comparison) {
            edge = doc_editor_edge(s, position, block->id, 1);
            result = doc_position_compare(s, &position, &edge, &comparison);
            if (result != XUI_OK) return result;
            if (!comparison) index++;
            else split_block = 1;
        }
    }
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    if (split_block) {
        result = xuiDocumentTxnSplitBlock(t, &position, &split);
        if (result == XUI_OK) {
            n = doc_index_get(t->draft->index, split.iNodeId);
            block = n && n->kind == XUI_DOC_TEXT ?
                doc_index_get(t->draft->index, n->parent) : n;
            parent_node = block ? doc_index_get(t->draft->index, block->parent) : NULL;
            if (!parent_node || !doc_schema_child(parent_node->kind, XUI_DOC_TABLE))
                result = XUI_DOC_ERROR_SCHEMA;
            else { parent = parent_node->id; index = doc_child_index(parent_node, block->id); }
        }
    }
    if (result == XUI_OK) result = xuiDocumentTxnInsertTable(t, parent, index,
        rows, columns, 1, &table);
    if (result == XUI_OK) {
        after.tAnchor = after.tCaret = doc_editor_edge(t->draft, position, table, 0);
        result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
static int doc_editor_horizontal(doc_editor_data* e, xui_doc_position_t p, int right, int word, xui_doc_position_t* out)
{
    doc_edit_projection projection; int offset, result;
    if (p.iKind == XUI_DOC_POSITION_SOURCE) {
        doc_edit_source_reader reader = {0}; uint64_t length;
        reader.source = doc_render_source(e->view.renderer);
        length = doc_seq_size(reader.source);
        if (length > INT_MAX) return XUI_DOC_ERROR_LIMIT;
        if (p.iOffset > length) return XUI_ERROR_INVALID_ARGUMENT;
        reader.length = (int)length;
        offset = word ? (right ? xuiInternalTextWordNextRead(doc_editor_source_read, &reader,
            reader.length, (int)p.iOffset, XUI_INTERNAL_WORD_NATURAL) :
            xuiInternalTextWordPrevRead(doc_editor_source_read, &reader,
            reader.length, (int)p.iOffset, XUI_INTERNAL_WORD_NATURAL)) :
            (right ? xuiInternalTextGraphemeNextRead(doc_editor_source_read, &reader,
            reader.length, (int)p.iOffset) :
            xuiInternalTextGraphemePrevRead(doc_editor_source_read, &reader,
            reader.length, (int)p.iOffset));
        if (reader.error != XUI_OK) return reader.error;
        *out = p; out->iOffset = (uint64_t)offset; return XUI_OK;
    }
    result = doc_editor_project(e, &p, &projection);
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
static int doc_editor_clipboard_result(doc_editor_data* e, xui_widget w, uint64_t epoch, int result)
{
    if (!xuiInternalWidgetIsValid(w)) return XUI_DOC_ERROR_STALE;
    e->in_clipboard = 0;
    return epoch == e->input_epoch ? result : XUI_DOC_ERROR_STALE;
}
static int doc_editor_copy(doc_editor_data* e)
{
    char* text; uint64_t bytes, epoch = e->input_epoch; int result;
    char* native = NULL; uint64_t native_bytes = 0;
    char* html = NULL; uint64_t html_bytes = 0;
    void* image_png = NULL; size_t image_png_bytes = 0;
    xui_document_fragment fragment = NULL;
    int order = 0, semantic_selection = 0;
    xui_widget w = e->view.widget; xui_proxy proxy = e->view.renderer->proxy;
    if (!proxy->clipboardSetText) return XUI_ERROR_UNSUPPORTED;
    if (e->view.table_selection.iTableId) {
        const xui_doc_table_selection_t* selected = &e->view.table_selection;
        result = xuiDocumentSnapshotCopyTableMatrix(e->view.renderer->snapshot,
            selected->iTableId, selected->iRow, selected->iColumn,
            selected->iRows, selected->iColumns, &text, &bytes);
    } else result = doc_render_copy_range(e->view.renderer, &e->view.selection, &text, &bytes);
    if (result == XUI_OK) {
        if (!e->view.table_selection.iTableId &&
            e->view.renderer->mode == XUI_DOC_VISUAL &&
            doc_render_compare(e->view.renderer, &e->view.selection.tAnchor,
                &e->view.selection.tCaret, &order) == XUI_OK && order)
            semantic_selection = 1;
        if (memchr(text, 0, (size_t)bytes)) {
            xuiDocumentFreeBuffer(text); return XUI_DOC_ERROR_UNREPRESENTABLE;
        }
        if (bytes || e->view.table_selection.iTableId || semantic_selection) {
            if (semantic_selection && proxy->clipboardSetItems) {
                int capture = xuiDocumentFragmentCreateRange(
                    e->view.renderer->snapshot, &e->view.selection, &fragment);
                int fatal_image = 0;
                if (capture == XUI_OK) capture = xuiDocumentFragmentSerialize(
                    fragment, &native, &native_bytes);
                if (capture == XUI_OK) capture = xuiDocumentFragmentExportHtml(
                    fragment, &html, &html_bytes);
                if (capture == XUI_OK) {
                    const char* image_resource = doc_fragment_single_image_resource(fragment);
                    if (image_resource) {
                        e->in_clipboard = 1;
                        int image_result = doc_image_resource_export_png(
                            xuiWidgetGetContext(w), image_resource, &image_png, &image_png_bytes);
                        image_result = doc_editor_clipboard_result(e, w, epoch, image_result);
                        if (image_result != XUI_OK && image_result != XUI_ERROR_NOT_FOUND &&
                            image_result != XUI_ERROR_UNSUPPORTED) {
                            capture = image_result;
                            fatal_image = 1;
                        }
                    }
                }
                xuiDocumentFragmentRelease(fragment);
                if (capture == XUI_ERROR_OUT_OF_MEMORY || capture == XUI_DOC_ERROR_LIMIT ||
                    capture == XUI_DOC_ERROR_STALE || fatal_image) {
                    xuiDocumentFreeBuffer(html); xuiDocumentFreeBuffer(native);
                    xrtFree(image_png);
                    xuiDocumentFreeBuffer(text); return capture;
                }
            }
            if (!native && !bytes && !e->view.table_selection.iTableId) {
                xuiDocumentFreeBuffer(text); return XUI_ERROR_UNSUPPORTED;
            }
            e->in_clipboard = 1;
            if (native) {
                xui_clipboard_item_t items[4] = {
                    {XUI_CLIPBOARD_FORMAT_TEXT_UTF8, text, (size_t)bytes},
                    {XUI_CLIPBOARD_FORMAT_DOCUMENT_FRAGMENT, native, (size_t)native_bytes},
                    {XUI_CLIPBOARD_FORMAT_HTML, html, (size_t)html_bytes},
                    {XUI_CLIPBOARD_FORMAT_IMAGE_PNG, image_png, image_png_bytes}
                };
                result = proxy->clipboardSetItems(proxy, items,
                    image_png ? 4 : (html ? 3 : 2));
            } else result = proxy->clipboardSetText(proxy, text);
            result = doc_editor_clipboard_result(e, w, epoch, result);
        }
        xuiDocumentFreeBuffer(native);
        xuiDocumentFreeBuffer(html);
        xrtFree(image_png);
        xuiDocumentFreeBuffer(text);
    }
    return result;
}
static int doc_editor_paste_fragment(doc_editor_data* e, int html)
{
    xui_proxy proxy = e->view.renderer->proxy;
    xui_widget w = e->view.widget;
    uint64_t epoch = e->input_epoch;
    xui_document_fragment fragment = NULL;
    xui_document_transaction t = NULL;
    xui_doc_position_t caret;
    xui_doc_range_t after;
    const char* format = html ? XUI_CLIPBOARD_FORMAT_HTML :
        XUI_CLIPBOARD_FORMAT_DOCUMENT_FRAGMENT;
    char* payload;
    int bytes, read, result;
    if (e->view.renderer->mode != XUI_DOC_VISUAL ||
        e->view.table_selection.iTableId || !proxy->clipboardGetData)
        return XUI_ERROR_NOT_FOUND;
    e->in_clipboard = 1;
    bytes = proxy->clipboardGetData(proxy, format, NULL, 0);
    bytes = doc_editor_clipboard_result(e, w, epoch, bytes);
    if (bytes == XUI_ERROR_FILE_NOT_FOUND || bytes == XUI_ERROR_UNSUPPORTED || !bytes)
        return XUI_ERROR_NOT_FOUND;
    if (bytes < 0) return bytes;
    if ((uint64_t)bytes > UINT64_C(256) * 1024 * 1024)
        return XUI_DOC_ERROR_LIMIT;
    payload = malloc((size_t)bytes);
    if (!payload) return XUI_ERROR_OUT_OF_MEMORY;
    e->in_clipboard = 1;
    read = proxy->clipboardGetData(proxy, format, payload, (size_t)bytes);
    read = doc_editor_clipboard_result(e, w, epoch, read);
    if (read != bytes) {
        free(payload);
        return read < 0 ? read : XUI_DOC_ERROR_STALE;
    }
    result = html ? xuiDocumentFragmentImportHtml(payload, (uint64_t)bytes,
        &fragment) : xuiDocumentFragmentDeserialize(payload, (uint64_t)bytes,
        &fragment);
    free(payload);
    if (result == XUI_ERROR_INVALID_ARGUMENT || result == XUI_DOC_ERROR_FORMAT ||
        result == XUI_DOC_ERROR_SCHEMA || result == XUI_DOC_ERROR_UTF8 ||
        result == XUI_ERROR_UNSUPPORTED) return XUI_ERROR_NOT_FOUND;
    if (result != XUI_OK) return result;
    doc_editor_break_group(e); doc_editor_cancel_composition(e);
    result = doc_editor_begin(e, &t, 0);
    if (result == XUI_OK) {
        result = xuiDocumentTxnReplaceRangeWithFragment(t,
            &e->view.selection, fragment, &caret);
        if (result == XUI_OK) {
            after.tAnchor = after.tCaret = caret;
            result = doc_editor_commit(e, t, &after);
        }
    }
    xuiDocumentTxnRelease(t);
    xuiDocumentFragmentRelease(fragment);
    return result;
}
static int doc_editor_paste_png(doc_editor_data* e)
{
    xui_proxy proxy = e->view.renderer->proxy;
    xui_widget w = e->view.widget;
    xui_context context = xuiWidgetGetContext(w);
    uint64_t epoch = e->input_epoch;
    static const unsigned char signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    xui_document_transaction t = NULL;
    xui_resource resource = NULL;
    xui_doc_image_desc_t image = {0};
    xui_doc_range_t after;
    xui_doc_position_t caret;
    xui_doc_node_id image_id;
    unsigned char* payload;
    char name[80];
    int bytes, read, width, height, result, attempt;
    if (e->view.renderer->mode != XUI_DOC_VISUAL ||
        e->view.table_selection.iTableId || !proxy->clipboardGetData)
        return XUI_ERROR_NOT_FOUND;
    e->in_clipboard = 1;
    bytes = proxy->clipboardGetData(proxy, XUI_CLIPBOARD_FORMAT_IMAGE_PNG, NULL, 0);
    bytes = doc_editor_clipboard_result(e, w, epoch, bytes);
    if (bytes == XUI_ERROR_FILE_NOT_FOUND || bytes == XUI_ERROR_UNSUPPORTED || !bytes)
        return XUI_ERROR_NOT_FOUND;
    if (bytes < 0) return bytes;
    if ((uint64_t)bytes > UINT64_C(16) * 1024 * 1024) return XUI_DOC_ERROR_LIMIT;
    payload = malloc((size_t)bytes);
    if (!payload) return XUI_ERROR_OUT_OF_MEMORY;
    e->in_clipboard = 1;
    read = proxy->clipboardGetData(proxy, XUI_CLIPBOARD_FORMAT_IMAGE_PNG,
        payload, (size_t)bytes);
    read = doc_editor_clipboard_result(e, w, epoch, read);
    if (read != bytes) { free(payload); return read < 0 ? read : XUI_DOC_ERROR_STALE; }
    if (bytes < (int)sizeof(signature) || memcmp(payload, signature, sizeof(signature)) ||
        xgeImageInfoMemory(payload, bytes, &width, &height) != XGE_OK) {
        free(payload); return XUI_DOC_ERROR_FORMAT;
    }
    if ((uint64_t)width > UINT64_C(16000000) / (uint64_t)height) {
        free(payload); return XUI_DOC_ERROR_LIMIT;
    }
    for (attempt = 0; attempt < 16; attempt++) {
        unsigned long long sequence = (unsigned long long)atomic_fetch_add(
            &doc_editor_clipboard_image_sequence, 1);
        snprintf(name, sizeof(name), "xui-clipboard-image-%016llx.png", sequence);
        if (!xuiResourceFind(context, name)) break;
    }
    if (attempt == 16) { free(payload); return XUI_DOC_ERROR_LIMIT; }
    e->in_clipboard = 1;
    result = xuiDocumentImageResourceLoadMemory(context, name,
        payload, (uint64_t)bytes, NULL, &resource);
    free(payload);
    result = doc_editor_clipboard_result(e, w, epoch, result);
    if (result != XUI_OK) {
        if (resource && xuiInternalWidgetIsValid(w) &&
            xuiResourceFind(context, name) == resource) xuiResourceRemove(resource);
        return result;
    }
    image.iSize = sizeof(image); image.sResource = name;
    doc_editor_break_group(e); doc_editor_cancel_composition(e);
    result = doc_editor_begin(e, &t, 0);
    if (result == XUI_OK) {
        result = xuiDocumentTxnInsertImage(t, &e->view.selection,
            &image, &image_id, &caret);
        if (result == XUI_OK) {
            after.tAnchor = after.tCaret = caret;
            result = doc_editor_commit(e, t, &after);
        }
    }
    xuiDocumentTxnRelease(t);
    if (result != XUI_OK && xuiInternalWidgetIsValid(w) &&
        xuiResourceFind(context, name) == resource) xuiResourceRemove(resource);
    return result;
}
static int doc_editor_paste(doc_editor_data* e, unsigned kind)
{
    xui_proxy proxy = e->view.renderer->proxy; xui_widget w = e->view.widget; uint64_t epoch = e->input_epoch;
    char* text; char query[1]; int bytes, read, result;
    result = doc_editor_paste_fragment(e, 0);
    if (result == XUI_ERROR_NOT_FOUND)
        result = doc_editor_paste_fragment(e, 1);
    if (result == XUI_ERROR_NOT_FOUND)
        result = doc_editor_paste_png(e);
    if (result != XUI_ERROR_NOT_FOUND) return result;
    if (!proxy->clipboardGetText) return XUI_ERROR_UNSUPPORTED;
    e->in_clipboard = 1; bytes = proxy->clipboardGetText(proxy, query, sizeof(query));
    bytes = doc_editor_clipboard_result(e, w, epoch, bytes); if (bytes < 0) return bytes;
    if ((uint64_t)bytes > e->view.document->max_bytes || bytes == INT_MAX) return XUI_DOC_ERROR_LIMIT;
    text = malloc((size_t)bytes + 1); if (!text) return XUI_ERROR_OUT_OF_MEMORY;
    e->in_clipboard = 1; read = proxy->clipboardGetText(proxy, text, bytes + 1);
    read = doc_editor_clipboard_result(e, w, epoch, read);
    if (read < 0) result = read;
    else if (read > bytes) result = XUI_DOC_ERROR_STALE;
    else {
        result = doc_editor_paste_matrix(e, text, (uint64_t)read);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_editor_replace(e, &e->view.selection, text, (uint64_t)read, kind);
    }
    free(text); return result;
}
static uint32_t doc_editor_command_mark(uint32_t command)
{
    switch (command) {
    case XUI_DOC_EDIT_BOLD: return XUI_DOC_BOLD;
    case XUI_DOC_EDIT_ITALIC: return XUI_DOC_ITALIC;
    case XUI_DOC_EDIT_UNDERLINE: return XUI_DOC_UNDERLINE;
    case XUI_DOC_EDIT_STRIKE: return XUI_DOC_STRIKE;
    case XUI_DOC_EDIT_CODE: return XUI_DOC_CODE;
    case XUI_DOC_EDIT_SUBSCRIPT: return XUI_DOC_SUBSCRIPT;
    case XUI_DOC_EDIT_SUPERSCRIPT: return XUI_DOC_SUPERSCRIPT;
    case XUI_DOC_EDIT_HIGHLIGHT: return XUI_DOC_HIGHLIGHT;
    default: return 0;
    }
}
static uint32_t doc_editor_heading_level(uint32_t command)
{
    return command >= XUI_DOC_EDIT_PARAGRAPH && command <= XUI_DOC_EDIT_HEADING_6 ?
        command - XUI_DOC_EDIT_PARAGRAPH : UINT32_MAX;
}
static uint32_t doc_editor_alignment(uint32_t command)
{
    return command >= XUI_DOC_EDIT_ALIGN_LEFT && command <= XUI_DOC_EDIT_ALIGN_JUSTIFY ?
        command - XUI_DOC_EDIT_ALIGN_LEFT : UINT32_MAX;
}
static int doc_editor_style_insertable(doc_state* s, const xui_doc_range_t* range, uint32_t kind)
{
    doc_node* parent;
    if (range->tAnchor.iKind != XUI_DOC_POSITION_GAP || range->tCaret.iKind != XUI_DOC_POSITION_GAP ||
        range->tAnchor.iNodeId != range->tCaret.iNodeId || range->tAnchor.iOffset != range->tCaret.iOffset)
        return 0;
    parent = doc_index_get(s->index, range->tAnchor.iNodeId);
    return parent && doc_schema_child(parent->kind, kind);
}
static int doc_editor_paragraph(const doc_node* node)
{
    return node && (node->kind == XUI_DOC_PARAGRAPH || node->kind == XUI_DOC_HEADING);
}
static int doc_editor_block_insertable(doc_state* s,
    const xui_doc_position_t* at, uint32_t kind)
{
    doc_node* node = doc_index_get(s->index, at->iNodeId);
    doc_node *block, *parent;
    if (!node) return 0;
    if (at->iKind == XUI_DOC_POSITION_GAP && !doc_editor_paragraph(node) &&
        at->iOffset <= doc_seq_size(node->children) &&
        doc_schema_child(node->kind, kind)) return 1;
    if (at->iKind == XUI_DOC_POSITION_TEXT && node->kind == XUI_DOC_TEXT)
        block = doc_index_get(s->index, node->parent);
    else if (at->iKind == XUI_DOC_POSITION_GAP) block = node;
    else return 0;
    if (!doc_editor_paragraph(block)) return 0;
    parent = doc_index_get(s->index, block->parent);
    return parent && doc_schema_child(parent->kind, kind);
}
static doc_node* doc_editor_quote_ancestor(doc_state* s,
    const xui_doc_position_t* at)
{
    doc_node* node = doc_index_get(s->index, at->iNodeId);
    for (; node && node->kind != XUI_DOC_QUOTE;
         node = node->parent ? doc_index_get(s->index, node->parent) : NULL) {}
    return node;
}
static int doc_editor_quote_range_can(doc_state* s, const xui_doc_range_t* range)
{
    return doc_quote_range_can(s, range);
}
static int doc_editor_table_context(doc_state* s, const xui_doc_position_t* position,
    doc_table_cell_slot* slot)
{
    doc_node* cell;
    if (position->iKind == XUI_DOC_POSITION_SOURCE) return XUI_ERROR_NOT_FOUND;
    cell = doc_index_get(s->index, position->iNodeId);
    while (cell && cell->kind != XUI_DOC_CELL)
        cell = cell->parent ? doc_index_get(s->index, cell->parent) : NULL;
    return cell ? doc_table_locate_cell(s, cell->id, slot) : XUI_ERROR_NOT_FOUND;
}
static int doc_editor_paste_matrix(doc_editor_data* e, const char* text, uint64_t bytes)
{
    doc_table_cell_slot slot; xui_document_transaction t = NULL;
    xui_doc_range_t after = e->view.selection; xui_doc_node_id last;
    int order, result, rectangular = e->view.table_selection.iTableId != 0;
    if (e->view.renderer->mode != XUI_DOC_VISUAL ||
        (!rectangular && !memchr(text, '\t', (size_t)bytes)))
        return XUI_ERROR_NOT_FOUND;
    if (rectangular) {
        const xui_doc_table_selection_t* selected = &e->view.table_selection;
        result = doc_table_cell_at(e->view.renderer->snapshot->state,
            selected->iTableId, selected->iRow, selected->iColumn, &slot);
        if (result != XUI_OK) return result;
    } else {
        result = doc_render_compare(e->view.renderer, &after.tAnchor, &after.tCaret, &order);
        if (result != XUI_OK) return result;
        if (order || doc_editor_table_context(e->view.renderer->snapshot->state,
            &after.tCaret, &slot) != XUI_OK) return XUI_ERROR_NOT_FOUND;
    }
    doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnPasteTableMatrix(t, slot.table, slot.row, slot.column,
        text, bytes, &last);
    if (result == XUI_OK)
        after.tAnchor = after.tCaret = doc_editor_edge(t->draft, after.tCaret, last, 1);
    if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    xuiDocumentTxnRelease(t); return result;
}
static int doc_editor_clear_table_selection(doc_editor_data* e)
{
    xui_doc_table_selection_t selected = e->view.table_selection;
    xui_doc_range_t after = e->view.selection;
    xui_document_transaction t = NULL; xui_doc_node_id first;
    int result;
    if (e->desc.bReadOnly) return XUI_ERROR_INVALID_STATE;
    if (!selected.iTableId || e->view.renderer->mode != XUI_DOC_VISUAL)
        return XUI_ERROR_NOT_FOUND;
    doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnClearTableMatrix(t, selected.iTableId,
        selected.iRow, selected.iColumn, selected.iRows, selected.iColumns, &first);
    if (result == XUI_OK)
        after.tAnchor = after.tCaret = doc_editor_edge(t->draft, after.tCaret, first, 0);
    if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    xuiDocumentTxnRelease(t); return result;
}
static uint64_t doc_editor_table_neighbor(doc_state* s, const xui_doc_position_t* position, int forward)
{
    doc_node *cell, *row, *table;
    uint64_t cell_index, row_index, rows;
    if (position->iKind == XUI_DOC_POSITION_SOURCE) return 0;
    cell = doc_index_get(s->index, position->iNodeId);
    while (cell && cell->kind != XUI_DOC_CELL)
        cell = cell->parent ? doc_index_get(s->index, cell->parent) : NULL;
    if (!cell || !(row = doc_index_get(s->index, cell->parent)) || row->kind != XUI_DOC_ROW ||
        !(table = doc_index_get(s->index, row->parent)) || table->kind != XUI_DOC_TABLE) return 0;
    cell_index = doc_child_index(row, cell->id);
    row_index = doc_child_index(table, row->id);
    if (cell_index == DOC_NONE || row_index == DOC_NONE) return 0;
    if (forward && cell_index + 1 < doc_seq_size(row->children))
        return doc_seq_get_id(row->children, cell_index + 1);
    if (!forward && cell_index) return doc_seq_get_id(row->children, cell_index - 1);
    rows = doc_seq_size(table->children);
    while (forward ? ++row_index < rows : row_index-- > 0) {
        row = doc_index_get(s->index, doc_seq_get_id(table->children, row_index));
        if (row && doc_seq_size(row->children))
            return doc_seq_get_id(row->children, forward ? 0 : doc_seq_size(row->children) - 1);
    }
    return 0;
}
static int doc_editor_table_vertical(doc_state* s, const xui_doc_position_t* position,
    int down, xui_doc_position_t* out)
{
    doc_table_cell_slot slot, target;
    uint64_t row;
    if (doc_editor_table_context(s, position, &slot) != XUI_OK) return 0;
    if (!down && !slot.row) return 0;
    row = down ? (uint64_t)slot.row + slot.row_span : (uint64_t)slot.row - 1;
    if (row >= slot.rows || doc_table_cell_at(s, slot.table, (uint32_t)row,
        slot.column, &target) != XUI_OK || target.cell == slot.cell) return 0;
    *out = doc_editor_edge(s, *position, target.cell, !down);
    return 1;
}
/* Mirror doc_editor_project's span lengths without allocating paragraph text. */
static int doc_editor_projected_span_nonempty(doc_node* node)
{
    return doc_selectable_object_kind(node->kind) ||
        !doc_text_kind(node->kind) || doc_seq_size(node->text) != 0;
}
static int doc_editor_projected_neighbor(doc_state* s, doc_node* parent,
    uint64_t index, int right)
{
    uint64_t count = doc_seq_size(parent->children), i;
    if (right) {
        for (i = index; i < count; i++) {
            doc_node* child = doc_index_get(s->index, doc_seq_get_id(parent->children, i));
            if (child && doc_editor_projected_span_nonempty(child)) return 1;
        }
    } else {
        for (i = index; i > 0; i--) {
            doc_node* child = doc_index_get(s->index, doc_seq_get_id(parent->children, i - 1));
            if (child && doc_editor_projected_span_nonempty(child)) return 1;
        }
    }
    return 0;
}
static int doc_editor_has_horizontal_neighbor(doc_state* s,
    const xui_doc_position_t* position, int right)
{
    doc_node* node;
    if (position->iKind == XUI_DOC_POSITION_SOURCE)
        return right ? position->iOffset < doc_seq_size(s->source) : position->iOffset > 0;
    node = doc_index_get(s->index, position->iNodeId);
    if (!node) return 0;
    if (position->iKind == XUI_DOC_POSITION_TEXT) {
        if (doc_selectable_object_kind(node->kind) ?
            (right ? position->iOffset == 0 : position->iOffset != 0) :
            (right ? position->iOffset < doc_seq_size(node->text) : position->iOffset > 0))
            return 1;
        if (node->kind == XUI_DOC_TEXT || doc_selectable_object_kind(node->kind)) {
            doc_node* parent = doc_index_get(s->index, node->parent);
            uint64_t index;
            if (!parent) return 0;
            index = doc_child_index(parent, node->id);
            if (index == DOC_NONE) return 0;
            if (doc_editor_projected_neighbor(s, parent, index + !!right, right)) return 1;
            node = parent;
        }
    } else if (doc_editor_projected_neighbor(s, node, position->iOffset, right)) return 1;
    while (node->parent) {
        doc_node* parent = doc_index_get(s->index, node->parent);
        uint64_t index;
        if (!parent) return 0;
        index = doc_child_index(parent, node->id);
        if (index == DOC_NONE) return 0;
        if (right ? index + 1 < doc_seq_size(parent->children) : index > 0) return 1;
        node = parent;
    }
    return 0;
}
static int doc_editor_query_command(doc_editor_data* e, uint32_t command, xui_doc_command_state_t* state)
{
    uint32_t mark = doc_editor_command_mark(command), heading = doc_editor_heading_level(command);
    uint32_t alignment = doc_editor_alignment(command), common = 0, mixed = 0;
    int order, result;
    memset(state, 0, sizeof(*state)); state->iSize = sizeof(*state);
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    result = doc_render_compare(e->view.renderer, &e->view.selection.tAnchor, &e->view.selection.tCaret, &order);
    if (result != XUI_OK) return result;
    state->bEnabled = 1;
    if (doc_editor_blocked(e) && command != XUI_DOC_EDIT_COPY && command != XUI_DOC_EDIT_SELECT_ALL) state->iDisabledReason = XUI_DOC_ERROR_BUSY;
    else if (command < XUI_DOC_EDIT_UNDO || command > XUI_DOC_EDIT_TOGGLE_TASK) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
    else if (command != XUI_DOC_EDIT_COPY && command != XUI_DOC_EDIT_SELECT_ALL &&
        command != XUI_DOC_EDIT_TABLE_NEXT_CELL && command != XUI_DOC_EDIT_TABLE_PREVIOUS_CELL &&
        e->desc.bReadOnly) state->iDisabledReason = XUI_ERROR_INVALID_STATE;
    else if (e->view.table_selection.iTableId && command == XUI_DOC_EDIT_ENTER)
        state->iDisabledReason = e->view.renderer->mode == XUI_DOC_VISUAL &&
            e->view.renderer->snapshot->state->profile == XUI_DOCUMENT_RICH ?
            XUI_OK : XUI_DOC_ERROR_UNREPRESENTABLE;
    else if ((command == XUI_DOC_EDIT_COPY || command == XUI_DOC_EDIT_CUT) &&
        ((!order && !e->view.table_selection.iTableId) ||
            !e->view.renderer->proxy->clipboardSetText)) state->iDisabledReason = XUI_ERROR_INVALID_STATE;
    else if (command == XUI_DOC_EDIT_PASTE && !e->view.renderer->proxy->clipboardGetText) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
    else if ((command == XUI_DOC_EDIT_UNDO && !e->input && !xuiDocumentCanUndo(e->view.document)) || (command == XUI_DOC_EDIT_REDO && !xuiDocumentCanRedo(e->view.document))) state->iDisabledReason = XUI_ERROR_INVALID_STATE;
    else if (command != XUI_DOC_EDIT_COPY && command != XUI_DOC_EDIT_SELECT_ALL && e->view.document->pending &&
        (!e->input || e->input->prepare != e->view.document->pending)) state->iDisabledReason = XUI_DOC_ERROR_BUSY;
    else if ((command == XUI_DOC_EDIT_BACKSPACE || command == XUI_DOC_EDIT_DELETE) &&
        !order && !e->view.table_selection.iTableId &&
        !doc_editor_has_horizontal_neighbor(e->view.renderer->snapshot->state,
            &e->view.selection.tCaret, command == XUI_DOC_EDIT_DELETE))
        state->iDisabledReason = XUI_ERROR_INVALID_STATE;
    else if (command == XUI_DOC_EDIT_CLEAR_FORMATTING) {
        xui_doc_text_style_query_t style = {0};
        style.iSize = sizeof(style);
        if (e->view.renderer->mode != XUI_DOC_VISUAL || e->view.table_selection.iTableId)
            state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
        else if (order) {
            result = xuiDocumentSnapshotQueryTextStyle(e->view.renderer->snapshot,
                &e->view.selection, &style);
            if (result != XUI_OK) return result;
            if (!style.bHasText) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
        }
        if (!order) state->bActive = e->pending_clear_format;
    }
    else if (command >= XUI_DOC_EDIT_TABLE_NEXT_CELL && command <= XUI_DOC_EDIT_TABLE_COLUMN_AUTO) {
        doc_table_cell_slot slot, anchor;
        uint64_t neighbor = 0;
        if (e->view.renderer->mode != XUI_DOC_VISUAL || e->view.desc.bDisableSelection)
            state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
        else if (doc_editor_table_context(e->view.renderer->snapshot->state,
            &e->view.selection.tCaret, &slot) != XUI_OK) state->iDisabledReason = XUI_ERROR_INVALID_STATE;
        else if (command >= XUI_DOC_EDIT_TABLE_COLUMN_NARROWER) {
            doc_node* table = doc_index_get(e->view.renderer->snapshot->state->index, slot.table);
            uint32_t column = slot.column + slot.column_span - 1;
            float preferred = doc_table_column_width(table, column), displayed = 0;
            if (e->view.renderer->snapshot->state->profile != XUI_DOCUMENT_RICH)
                state->iDisabledReason = XUI_DOC_ERROR_UNREPRESENTABLE;
            else if (order || e->view.table_selection.iTableId)
                state->iDisabledReason = XUI_ERROR_INVALID_STATE;
            else if (command == XUI_DOC_EDIT_TABLE_COLUMN_AUTO) {
                state->bActive = preferred == 0;
                if (state->bActive) state->iDisabledReason = XUI_ERROR_INVALID_STATE;
            } else {
                result = doc_editor_display_column_width(e,
                    e->view.renderer->snapshot->state, slot.table, column, &displayed);
                if (result != XUI_OK) state->iDisabledReason = result;
                else if ((command == XUI_DOC_EDIT_TABLE_COLUMN_NARROWER && displayed <= 24) ||
                    (command == XUI_DOC_EDIT_TABLE_COLUMN_WIDER && displayed >= 1000000))
                    state->iDisabledReason = XUI_ERROR_INVALID_STATE;
            }
        }
        else if (command == XUI_DOC_EDIT_TABLE_NEXT_CELL || command == XUI_DOC_EDIT_TABLE_PREVIOUS_CELL) {
            neighbor = doc_editor_table_neighbor(e->view.renderer->snapshot->state,
                &e->view.selection.tCaret, command == XUI_DOC_EDIT_TABLE_NEXT_CELL);
            if (!neighbor && (command != XUI_DOC_EDIT_TABLE_NEXT_CELL || e->desc.bReadOnly || order ||
                slot.rows == UINT32_MAX)) state->iDisabledReason = XUI_ERROR_INVALID_STATE;
        } else if (command == XUI_DOC_EDIT_TABLE_MERGE_CELLS) {
            if (e->view.renderer->snapshot->state->profile != XUI_DOCUMENT_RICH)
                state->iDisabledReason = XUI_DOC_ERROR_UNREPRESENTABLE;
            else if (e->view.table_selection.iTableId) {
                const xui_doc_table_selection_t* selected = &e->view.table_selection;
                if (selected->iTableId != slot.table || !selected->iRows ||
                    !selected->iColumns)
                    state->iDisabledReason = XUI_ERROR_INVALID_STATE;
                else state->iDisabledReason = doc_table_can_merge(
                    e->view.renderer->snapshot->state, selected->iTableId,
                    selected->iRow, selected->iColumn, selected->iRows,
                    selected->iColumns);
            }
            else if (!order || doc_editor_table_context(e->view.renderer->snapshot->state,
                &e->view.selection.tAnchor, &anchor) != XUI_OK ||
                anchor.table != slot.table || anchor.cell == slot.cell)
                state->iDisabledReason = XUI_ERROR_INVALID_STATE;
            else {
                uint32_t first_row = anchor.row < slot.row ? anchor.row : slot.row;
                uint32_t first_column = anchor.column < slot.column ? anchor.column : slot.column;
                uint32_t last_row = anchor.row + anchor.row_span;
                uint32_t last_column = anchor.column + anchor.column_span;
                if (slot.row + slot.row_span > last_row) last_row = slot.row + slot.row_span;
                if (slot.column + slot.column_span > last_column)
                    last_column = slot.column + slot.column_span;
                state->iDisabledReason = doc_table_can_merge(e->view.renderer->snapshot->state,
                    slot.table, first_row, first_column,
                    last_row - first_row, last_column - first_column);
            }
        } else if (order) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
        else if (command == XUI_DOC_EDIT_TABLE_SPLIT_CELL &&
            e->view.renderer->snapshot->state->profile != XUI_DOCUMENT_RICH)
            state->iDisabledReason = XUI_DOC_ERROR_UNREPRESENTABLE;
        else if (command == XUI_DOC_EDIT_TABLE_SPLIT_CELL &&
            slot.row_span == 1 && slot.column_span == 1)
            state->iDisabledReason = XUI_ERROR_INVALID_STATE;
        else if ((command == XUI_DOC_EDIT_TABLE_INSERT_COLUMN_BEFORE ||
            command == XUI_DOC_EDIT_TABLE_INSERT_COLUMN_AFTER) && slot.columns >= 1024)
            state->iDisabledReason = XUI_DOC_ERROR_LIMIT;
        else if ((command == XUI_DOC_EDIT_TABLE_INSERT_ROW_BEFORE ||
            command == XUI_DOC_EDIT_TABLE_INSERT_ROW_AFTER) && slot.rows == UINT32_MAX)
            state->iDisabledReason = XUI_DOC_ERROR_LIMIT;
    }
    else if (mark && (e->view.renderer->mode != XUI_DOC_VISUAL || (xuiDocumentGetProfile(e->view.document) == XUI_DOCUMENT_MARKDOWN &&
        (mark == XUI_DOC_UNDERLINE ||
        (mark == XUI_DOC_STRIKE && xuiDocumentGetMarkdownDialect(e->view.document) == XUI_MD_COMMONMARK) ||
        ((mark == XUI_DOC_SUBSCRIPT || mark == XUI_DOC_SUPERSCRIPT || mark == XUI_DOC_HIGHLIGHT) &&
            xuiDocumentGetMarkdownDialect(e->view.document) != XUI_MD_EXTENDED))))) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
    else if (command == XUI_DOC_EDIT_INDENT_LIST || command == XUI_DOC_EDIT_OUTDENT_LIST) {
        doc_state* s = e->view.renderer->snapshot->state; doc_node *item, *list;
        uint64_t item_id = 0, other_id = 0; int empty = 0;
        if (e->view.renderer->mode != XUI_DOC_VISUAL ||
            !doc_editor_list_context(s, &e->view.selection.tCaret, &empty, &item_id) ||
            !doc_editor_list_context(s, &e->view.selection.tAnchor, &empty, &other_id)) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
        else {
            item = doc_index_get(s->index, item_id); list = doc_index_get(s->index, item->parent);
            if (order) state->iDisabledReason = doc_list_range_can(s, &e->view.selection,
                command == XUI_DOC_EDIT_OUTDENT_LIST);
            else if (other_id != item_id) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
            else if (command == XUI_DOC_EDIT_INDENT_LIST && !doc_child_index(list, item_id))
                state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
        }
    }
    else if (command == XUI_DOC_EDIT_MOVE_BLOCK_UP || command == XUI_DOC_EDIT_MOVE_BLOCK_DOWN) {
        if (e->view.renderer->mode != XUI_DOC_VISUAL || e->view.table_selection.iTableId)
            state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
        else state->iDisabledReason = doc_move_block_range_can(
            e->view.renderer->snapshot->state, &e->view.selection,
            command == XUI_DOC_EDIT_MOVE_BLOCK_DOWN);
    }
    else if (command == XUI_DOC_EDIT_BLOCK_QUOTE) {
        doc_state* s = e->view.renderer->snapshot->state;
        doc_node* quote = !order ? doc_editor_quote_ancestor(s,
            &e->view.selection.tCaret) : NULL;
        if (e->view.renderer->mode != XUI_DOC_VISUAL || e->view.table_selection.iTableId)
            state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
        else if (quote) {
            doc_node* parent = doc_index_get(s->index, quote->parent);
            uint64_t i;
            state->bActive = 1;
            if (!parent) state->iDisabledReason = XUI_DOC_ERROR_SCHEMA;
            for (i = 0; parent && i < doc_seq_size(quote->children); i++) {
                doc_node* child = doc_index_get(s->index,
                    doc_seq_get_id(quote->children, i));
                if (!child || !doc_schema_child(parent->kind, child->kind)) {
                    state->iDisabledReason = XUI_ERROR_UNSUPPORTED; break;
                }
            }
        } else state->iDisabledReason = doc_editor_quote_range_can(s,
            &e->view.selection);
    }
    else if (command == XUI_DOC_EDIT_TOGGLE_TASK) {
        doc_node* item = NULL;
        if (e->view.renderer->mode == XUI_DOC_VISUAL &&
            !e->view.table_selection.iTableId && !order)
            item = doc_editor_task_context(e->view.renderer->snapshot->state,
                &e->view.selection.tCaret);
        if (!item) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
        else state->bActive = (item->attrs->iFlags & XUI_DOC_CHECKED) != 0;
    }
    else if (command == XUI_DOC_EDIT_INSERT_RULE ||
        command == XUI_DOC_EDIT_INSERT_CODE_BLOCK) {
        uint32_t kind = command == XUI_DOC_EDIT_INSERT_RULE ?
            XUI_DOC_RULE : XUI_DOC_CODE_BLOCK;
        if (e->view.renderer->mode != XUI_DOC_VISUAL ||
            e->view.table_selection.iTableId || order ||
            !doc_editor_block_insertable(e->view.renderer->snapshot->state,
                &e->view.selection.tCaret, kind))
            state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
    }
    else if (command >= XUI_DOC_EDIT_BULLET_LIST && command <= XUI_DOC_EDIT_TASK_LIST) {
        uint32_t flags = command == XUI_DOC_EDIT_NUMBER_LIST ? XUI_DOC_ORDERED :
            command == XUI_DOC_EDIT_TASK_LIST ? XUI_DOC_TASK : 0;
        if (e->view.renderer->mode != XUI_DOC_VISUAL || e->view.table_selection.iTableId)
            state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
        else {
            doc_state* s = e->view.renderer->snapshot->state;
            result = doc_list_style_range_can(s, &e->view.selection, flags,
                flags & XUI_DOC_ORDERED ? 1 : 0, &state->bActive, &state->bMixed);
            if (result == XUI_ERROR_UNSUPPORTED)
                result = doc_list_create_range_can(s, &e->view.selection,
                    flags, flags & XUI_DOC_ORDERED ? 1 : 0);
            else if (result == XUI_OK && state->bActive)
                result = doc_unlist_range_can(s, &e->view.selection);
            state->iDisabledReason = result;
        }
    }
    else if (heading != UINT32_MAX) {
        uint64_t *blocks = NULL, count = 0, matches = 0, i;
        int insertion = 0;
        if (e->view.renderer->mode != XUI_DOC_VISUAL) state->iDisabledReason = XUI_ERROR_UNSUPPORTED;
        else {
            result = doc_block_range_ids(e->view.renderer->snapshot->state, &e->view.selection, &blocks, &count);
            if (result == XUI_ERROR_UNSUPPORTED && heading && !order &&
                doc_editor_style_insertable(e->view.renderer->snapshot->state, &e->view.selection, XUI_DOC_HEADING))
                { result = XUI_OK; insertion = 1; }
            if (result == XUI_ERROR_UNSUPPORTED) state->iDisabledReason = result;
            else if (result != XUI_OK) return result;
            else if (!insertion) {
                for (i = 0; i < count; i++) {
                    doc_node* n = doc_index_get(e->view.renderer->snapshot->state->index, blocks[i]);
                    if (heading ? n->kind == XUI_DOC_HEADING && n->attrs->iHeadingLevel == heading :
                        n->kind == XUI_DOC_PARAGRAPH) matches++;
                }
                state->bActive = matches == count;
                state->bMixed = matches && matches < count;
            }
            doc_free(blocks);
        }
    }
    else if (alignment != UINT32_MAX) {
        uint64_t *blocks = NULL, count = 0, matches = 0, i;
        int insertion = 0;
        doc_state* s = e->view.renderer->snapshot->state;
        if (e->view.renderer->mode != XUI_DOC_VISUAL || s->profile != XUI_DOCUMENT_RICH)
            state->iDisabledReason = XUI_DOC_ERROR_UNREPRESENTABLE;
        else {
            result = doc_block_range_ids(s, &e->view.selection, &blocks, &count);
            if (result == XUI_ERROR_UNSUPPORTED && !order &&
                doc_editor_style_insertable(s, &e->view.selection, XUI_DOC_PARAGRAPH))
                { result = XUI_OK; insertion = 1; }
            if (result == XUI_ERROR_UNSUPPORTED) state->iDisabledReason = result;
            else if (result != XUI_OK) return result;
            else if (!insertion) {
                for (i = 0; i < count; i++) {
                    doc_node* n = doc_index_get(s->index, blocks[i]);
                    if (doc_effective_alignment(s, n) == alignment) matches++;
                }
                state->bActive = matches == count;
                state->bMixed = matches && matches < count;
            }
            doc_free(blocks);
        }
    }
    if (mark && e->view.selection.tAnchor.iKind != XUI_DOC_POSITION_SOURCE) {
        result = xuiDocumentSnapshotQueryMarks(e->view.renderer->snapshot, &e->view.selection, &common, &mixed);
        if (result != XUI_OK) return result;
        if (!order && e->pending_clear_format) common = mixed = 0;
        if (!order && e->has_pending_marks) common = (common | e->pending_set) & ~e->pending_clear;
        state->bActive = !!(common & mark); state->bMixed = !!(mixed & mark);
    }
    state->bEnabled = !state->iDisabledReason; return XUI_OK;
}
XUI_API int xuiDocumentEditorQueryCommand(xui_widget w, uint32_t command, xui_doc_command_state_t* state)
{
    doc_editor_data* e = doc_editor_get(w);
    if (!e || !state || state->iSize != sizeof(*state)) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e)) {
        memset(state, 0, sizeof(*state)); state->iSize = sizeof(*state); state->iDisabledReason = XUI_DOC_ERROR_BUSY; return XUI_OK;
    }
    return doc_editor_query_command(e, command, state);
}
XUI_API int xuiDocumentEditorCanExecute(xui_widget w, uint32_t command)
{
    xui_doc_command_state_t state = {0}; state.iSize = sizeof(state);
    return xuiDocumentEditorQueryCommand(w, command, &state) == XUI_OK && state.bEnabled;
}
XUI_API int xuiDocumentEditorQueryParagraphSpacing(xui_widget w, xui_doc_paragraph_spacing_state_t* state)
{
    doc_editor_data* e = doc_editor_get(w); xui_doc_command_state_t command = {0};
    uint64_t *blocks = NULL, count = 0, i; int result;
    doc_state* s;
    if (!e || !state || state->iSize != sizeof(*state)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(state, 0, sizeof(*state)); state->iSize = sizeof(*state);
    if (doc_editor_api_busy(e)) { state->iDisabledReason = XUI_DOC_ERROR_BUSY; return XUI_OK; }
    result = doc_editor_query_command(e, XUI_DOC_EDIT_ALIGN_LEFT, &command);
    if (result != XUI_OK) return result;
    state->bEnabled = command.bEnabled; state->iDisabledReason = command.iDisabledReason;
    if (!state->bEnabled) return XUI_OK;
    s = e->view.renderer->snapshot->state;
    result = doc_block_range_ids(s, &e->view.selection, &blocks, &count);
    if (result == XUI_ERROR_UNSUPPORTED &&
        doc_editor_style_insertable(s, &e->view.selection, XUI_DOC_PARAGRAPH)) {
        state->bUseDefault = 1; return XUI_OK;
    }
    if (result != XUI_OK) return result;
    for (i = 0; i < count; i++) {
        doc_node* n = doc_index_get(s->index, blocks[i]);
        int use_default = !(n->attrs->iFlags & XUI_DOC_SPACING_EXPLICIT) && n->attrs->fParagraphSpacing == 0;
        if (!i) { state->bUseDefault = use_default; state->fValue = n->attrs->fParagraphSpacing; }
        else if (state->bUseDefault != use_default || (!use_default && state->fValue != n->attrs->fParagraphSpacing))
            state->bMixed = 1;
    }
    doc_free(blocks); return XUI_OK;
}
XUI_API int xuiDocumentEditorSetParagraphSpacing(xui_widget w, float spacing, int use_default)
{
    doc_editor_data* e = doc_editor_get(w); xui_doc_paragraph_spacing_state_t state = {0};
    xui_document_transaction t = NULL; xui_doc_block_style_t style = {0}; xui_doc_range_t range, after;
    int result;
    if (!e || (use_default != 0 && use_default != 1) ||
        (!use_default && (!isfinite(spacing) || spacing < 0))) return XUI_ERROR_INVALID_ARGUMENT;
    state.iSize = sizeof(state);
    result = xuiDocumentEditorQueryParagraphSpacing(w, &state);
    if (result != XUI_OK) return result;
    if (!state.bEnabled) return state.iDisabledReason;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    e->action_time = xrtClock(); range = e->view.selection;
    style.iSize = sizeof(style); style.fParagraphSpacing = use_default ? 0 : spacing;
    style.bSpacingExplicit = !use_default;
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnSetBlockStyleRange(t, &range, XUI_DOC_BLOCK_STYLE_SPACING, &style, &after);
    if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    xuiDocumentTxnRelease(t); return result;
}
static int doc_editor_table_change(doc_editor_data* e, uint32_t command,
    const doc_table_cell_slot* slot)
{
    xui_document_transaction t = NULL; xui_doc_range_t after = e->view.selection;
    doc_table_cell_slot target; doc_node* table;
    uint32_t row = slot->row, column = slot->column;
    struct xui_doc_change_set_t changes = {0}; int result, mapping;
    doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    switch (command) {
    case XUI_DOC_EDIT_TABLE_NEXT_CELL: /* Tab at the final Cell appends a row. */
        row = slot->rows; column = 0;
        result = xuiDocumentTxnInsertTableRow(t, slot->table, row); break;
    case XUI_DOC_EDIT_TABLE_INSERT_ROW_BEFORE:
        result = xuiDocumentTxnInsertTableRow(t, slot->table, row); break;
    case XUI_DOC_EDIT_TABLE_INSERT_ROW_AFTER:
        row += slot->row_span;
        result = xuiDocumentTxnInsertTableRow(t, slot->table, row); break;
    case XUI_DOC_EDIT_TABLE_DELETE_ROW:
        result = xuiDocumentTxnDeleteTableRow(t, slot->table, row); break;
    case XUI_DOC_EDIT_TABLE_INSERT_COLUMN_BEFORE:
        result = xuiDocumentTxnInsertTableColumn(t, slot->table, column); break;
    case XUI_DOC_EDIT_TABLE_INSERT_COLUMN_AFTER:
        column += slot->column_span;
        result = xuiDocumentTxnInsertTableColumn(t, slot->table, column); break;
    case XUI_DOC_EDIT_TABLE_DELETE_COLUMN:
        result = xuiDocumentTxnDeleteTableColumn(t, slot->table, column); break;
    case XUI_DOC_EDIT_TABLE_SPLIT_CELL:
        result = xuiDocumentTxnSplitCell(t, slot->cell); break;
    case XUI_DOC_EDIT_TABLE_MERGE_CELLS: {
        doc_table_cell_slot anchor; uint64_t merged;
        uint32_t row_end, column_end;
        if (e->view.table_selection.iTableId) {
            const xui_doc_table_selection_t* selected = &e->view.table_selection;
            row = selected->iRow; column = selected->iColumn;
            result = xuiDocumentTxnMergeCells(t, selected->iTableId,
                row, column, selected->iRows, selected->iColumns, &merged);
            break;
        }
        result = doc_editor_table_context(t->draft, &e->view.selection.tAnchor, &anchor);
        if (result != XUI_OK) break;
        row = row < anchor.row ? row : anchor.row;
        column = column < anchor.column ? column : anchor.column;
        row_end = slot->row + slot->row_span;
        if (anchor.row + anchor.row_span > row_end) row_end = anchor.row + anchor.row_span;
        column_end = slot->column + slot->column_span;
        if (anchor.column + anchor.column_span > column_end)
            column_end = anchor.column + anchor.column_span;
        result = xuiDocumentTxnMergeCells(t, slot->table, row, column,
            row_end - row, column_end - column, &merged);
        break;
    }
    default: result = XUI_ERROR_UNSUPPORTED; break;
    }
    if (result == XUI_OK) {
        table = doc_index_get(t->draft->index, slot->table);
        if (table && table->kind == XUI_DOC_TABLE) {
            uint32_t rows = (uint32_t)doc_seq_size(table->children);
            uint32_t columns = doc_table_column_count(t->draft, table);
            if (rows && columns) {
                if (row >= rows) row = rows - 1;
                if (column >= columns) column = columns - 1;
                result = doc_table_cell_at(t->draft, slot->table, row, column, &target);
                if (result == XUI_OK)
                    after.tAnchor = after.tCaret = doc_editor_edge(t->draft, after.tCaret, target.cell, 0);
            }
        }
        if (!table || table->kind != XUI_DOC_TABLE || result == XUI_ERROR_NOT_FOUND) {
            changes.before = t->base; changes.after = t->draft; changes.identity = t->document->identity;
            changes.before_revision = changes.after_revision = t->base_revision;
            changes.ops = t->ops; changes.count = t->count;
            result = xuiDocumentMapPosition(&changes, &e->view.selection.tCaret,
                &after.tCaret, &mapping);
            if (result == XUI_OK) after.tAnchor = after.tCaret;
        }
    }
    if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
    xuiDocumentTxnRelease(t); return result;
}
static int doc_editor_enter_table_selection(doc_editor_data* e)
{
    xui_doc_command_state_t state = {0}; xui_doc_table_selection_t selected = e->view.table_selection;
    doc_table_cell_slot slot; xui_document_transaction t = NULL;
    xui_doc_range_t range, after; xui_doc_position_t caret; uint64_t first;
    int result;
    result = doc_editor_query_command(e, XUI_DOC_EDIT_ENTER, &state);
    if (result != XUI_OK) return result;
    if (!state.bEnabled) return state.iDisabledReason;
    result = doc_table_cell_at(e->view.renderer->snapshot->state,
        selected.iTableId, selected.iRow, selected.iColumn, &slot);
    if (result != XUI_OK) return result;
    doc_editor_cancel_composition(e); doc_editor_break_group(e);
    result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
    result = xuiDocumentTxnClearTableMatrix(t, slot.table, slot.row,
        slot.column, slot.row_span, slot.column_span, &first);
    if (result == XUI_OK) {
        caret = doc_editor_edge(t->draft, e->view.selection.tCaret, first, 0);
        range.tAnchor = range.tCaret = caret;
        result = xuiDocumentTxnReplaceRange(t, &range, "\n", 1, &caret);
    }
    if (result == XUI_OK) {
        after.tAnchor = after.tCaret = caret;
        result = doc_editor_commit(e, t, &after);
    }
    xuiDocumentTxnRelease(t); return result;
}
static int doc_editor_execute(doc_editor_data* e, uint32_t command, int interactive)
{
    xui_widget w = e->view.widget; xui_doc_range_t range; int order, result; uint32_t mark;
    if (doc_editor_blocked(e) && command != XUI_DOC_EDIT_COPY && command != XUI_DOC_EDIT_SELECT_ALL) return XUI_DOC_ERROR_BUSY;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    if (command == XUI_DOC_EDIT_COPY) return doc_editor_copy(e);
    if (e->view.table_selection.iTableId && command == XUI_DOC_EDIT_ENTER)
        return doc_editor_enter_table_selection(e);
    doc_editor_cancel_composition(e);
    if (command == XUI_DOC_EDIT_SELECT_ALL) {
        range = e->view.selection; range.tAnchor.iNodeId = range.tCaret.iNodeId = DOC_ROOT; range.tAnchor.iOffset = 0;
        range.tAnchor.iKind = range.tCaret.iKind = e->view.renderer->mode != XUI_DOC_VISUAL ? XUI_DOC_POSITION_SOURCE : XUI_DOC_POSITION_GAP;
        range.tCaret.iOffset = range.tAnchor.iKind == XUI_DOC_POSITION_SOURCE ? doc_seq_size(doc_render_source(e->view.renderer)) : doc_seq_size(doc_index_get(e->view.renderer->snapshot->state->index, DOC_ROOT)->children);
        return doc_view_set_selection(&e->view, &range);
    }
    if (command == XUI_DOC_EDIT_TABLE_NEXT_CELL || command == XUI_DOC_EDIT_TABLE_PREVIOUS_CELL) {
        xui_doc_command_state_t state = {0}; doc_table_cell_slot slot; uint64_t target;
        result = doc_editor_query_command(e, command, &state); if (result != XUI_OK) return result;
        if (!state.bEnabled) return state.iDisabledReason;
        target = doc_editor_table_neighbor(e->view.renderer->snapshot->state,
            &e->view.selection.tCaret, command == XUI_DOC_EDIT_TABLE_NEXT_CELL);
        if (!target) {
            result = doc_editor_table_context(e->view.renderer->snapshot->state,
                &e->view.selection.tCaret, &slot);
            return result == XUI_OK ? doc_editor_table_change(e, command, &slot) : result;
        }
        range = e->view.selection;
        range.tAnchor = range.tCaret = doc_editor_edge(e->view.renderer->snapshot->state,
            range.tCaret, target, 0);
        range.tAnchor.iAffinity = range.tCaret.iAffinity = XUI_DOC_AFTER;
        doc_editor_break_group(e);
        e->has_pending_marks = e->pending_clear_format = 0;
        e->pending_set = e->pending_clear = e->pending_style_fields = 0;
        result = doc_view_set_selection(&e->view, &range);
        if (result == XUI_OK) doc_editor_reveal(e);
        return result;
    }
    if (e->desc.bReadOnly) return XUI_ERROR_INVALID_STATE;
    if (e->view.table_selection.iTableId &&
        (command == XUI_DOC_EDIT_CUT || command == XUI_DOC_EDIT_BACKSPACE ||
            command == XUI_DOC_EDIT_DELETE)) {
        if (command == XUI_DOC_EDIT_CUT) {
            result = doc_editor_copy(e); if (result != XUI_OK) return result;
        }
        return doc_editor_clear_table_selection(e);
    }
    if (command >= XUI_DOC_EDIT_TABLE_COLUMN_NARROWER &&
        command <= XUI_DOC_EDIT_TABLE_COLUMN_AUTO) {
        xui_doc_command_state_t state = {0}; doc_table_cell_slot slot;
        uint32_t column; float displayed = 0, width = 0;
        result = doc_editor_query_command(e, command, &state);
        if (result != XUI_OK) return result;
        if (!state.bEnabled) return state.iDisabledReason;
        result = doc_editor_table_context(e->view.renderer->snapshot->state,
            &e->view.selection.tCaret, &slot);
        if (result != XUI_OK) return result;
        column = slot.column + slot.column_span - 1;
        if (command != XUI_DOC_EDIT_TABLE_COLUMN_AUTO) {
            result = doc_editor_display_column_width(e,
                e->view.renderer->snapshot->state, slot.table, column, &displayed);
            if (result != XUI_OK) return result;
            width = (float)fmin(1000000, fmax(24, displayed +
                (command == XUI_DOC_EDIT_TABLE_COLUMN_WIDER ? 8 : -8)));
        }
        doc_editor_break_group(e);
        return doc_editor_apply_column_width(e, slot.table, column, width, 0,
            command != XUI_DOC_EDIT_TABLE_COLUMN_AUTO);
    }
    if (command >= XUI_DOC_EDIT_TABLE_INSERT_ROW_BEFORE && command <= XUI_DOC_EDIT_TABLE_MERGE_CELLS) {
        xui_doc_command_state_t state = {0}; doc_table_cell_slot slot;
        result = doc_editor_query_command(e, command, &state); if (result != XUI_OK) return result;
        if (!state.bEnabled) return state.iDisabledReason;
        result = doc_editor_table_context(e->view.renderer->snapshot->state,
            &e->view.selection.tCaret, &slot);
        return result == XUI_OK ? doc_editor_table_change(e, command, &slot) : result;
    }
    if (command == XUI_DOC_EDIT_UNDO || command == XUI_DOC_EDIT_REDO) {
        doc_editor_break_group(e);
        result = doc_editor_flush_input(e); if (result != XUI_OK) return result;
        if (!xuiInternalWidgetIsValid(w)) return XUI_OK;
        e->committing = 1;
        result = command == XUI_DOC_EDIT_UNDO ? xuiDocumentUndo(e->view.document, NULL) : xuiDocumentRedo(e->view.document, NULL);
        if (xuiInternalWidgetIsValid(w)) { e->committing = 0; doc_editor_reveal(e); }
        return result;
    }
    switch (command) {
    case XUI_DOC_EDIT_CLEAR_FORMATTING: {
        xui_doc_command_state_t state = {0}; xui_document_transaction t = NULL;
        xui_doc_range_t after; struct xui_doc_change_set_t changes = {0}; int mapping;
        result = doc_editor_query_command(e, command, &state);
        if (result != XUI_OK) return result;
        if (!state.bEnabled) return state.iDisabledReason;
        doc_editor_break_group(e);
        result = doc_render_compare(e->view.renderer, &e->view.selection.tAnchor,
            &e->view.selection.tCaret, &order);
        if (result != XUI_OK) return result;
        if (!order) {
            e->pending_clear_format = 1;
            e->has_pending_marks = 0;
            e->pending_set = e->pending_clear = 0;
            e->pending_style_fields &= XUI_DOC_TEXT_STYLE_LANGUAGE;
            return XUI_OK;
        }
        result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
        result = xuiDocumentTxnClearFormatting(t, &e->view.selection);
        if (result == XUI_OK) {
            changes.before = t->base; changes.after = t->draft;
            changes.identity = t->document->identity;
            changes.before_revision = changes.after_revision = t->base_revision;
            changes.ops = t->ops; changes.count = t->count;
            result = xuiDocumentMapPosition(&changes, &e->view.selection.tAnchor,
                &after.tAnchor, &mapping);
            if (result == XUI_OK) result = xuiDocumentMapPosition(&changes,
                &e->view.selection.tCaret, &after.tCaret, &mapping);
            if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
        }
        xuiDocumentTxnRelease(t);
        if (result == XUI_OK && xuiInternalWidgetIsValid(w)) {
            e->has_pending_marks = e->pending_clear_format = 0;
            e->pending_set = e->pending_clear = e->pending_style_fields = 0;
        }
        return result;
    }
    case XUI_DOC_EDIT_CUT: result = doc_editor_copy(e); return result == XUI_OK ? doc_editor_replace(e, &e->view.selection, "", 0, interactive ? DOC_EDITOR_ISOLATED : DOC_EDITOR_PROGRAM) : result;
    case XUI_DOC_EDIT_PASTE: return doc_editor_paste(e, interactive ? DOC_EDITOR_ISOLATED : DOC_EDITOR_PROGRAM);
    case XUI_DOC_EDIT_ENTER: return doc_editor_replace(e, &e->view.selection, "\n", 1, interactive ? DOC_EDITOR_ISOLATED : DOC_EDITOR_PROGRAM);
    case XUI_DOC_EDIT_INDENT_LIST: case XUI_DOC_EDIT_OUTDENT_LIST: {
        xui_doc_command_state_t state = {0}; xui_document_transaction t = NULL; xui_doc_position_t caret;
        xui_doc_range_t after;
        result = doc_editor_query_command(e, command, &state); if (result != XUI_OK) return result;
        if (!state.bEnabled) return state.iDisabledReason;
        range = e->view.selection;
        result = doc_render_compare(e->view.renderer, &range.tAnchor, &range.tCaret, &order);
        if (result != XUI_OK) return result;
        doc_editor_break_group(e);
        result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
        if (order) result = command == XUI_DOC_EDIT_INDENT_LIST ?
            xuiDocumentTxnIndentListRange(t, &range, &after) :
            xuiDocumentTxnOutdentListRange(t, &range, &after);
        else {
            result = command == XUI_DOC_EDIT_INDENT_LIST ?
                xuiDocumentTxnIndentListItem(t, &range.tCaret, &caret) :
                xuiDocumentTxnOutdentListItem(t, &range.tCaret, &caret);
            after.tAnchor = after.tCaret = caret;
        }
        if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
        xuiDocumentTxnRelease(t); return result;
    }
    case XUI_DOC_EDIT_BULLET_LIST: case XUI_DOC_EDIT_NUMBER_LIST:
    case XUI_DOC_EDIT_TASK_LIST: {
        xui_doc_command_state_t state = {0}; xui_document_transaction t = NULL;
        xui_doc_range_t after; xui_doc_node_id list_id;
        uint32_t flags = command == XUI_DOC_EDIT_NUMBER_LIST ? XUI_DOC_ORDERED :
            command == XUI_DOC_EDIT_TASK_LIST ? XUI_DOC_TASK : 0;
        int in_list;
        result = doc_editor_query_command(e, command, &state);
        if (result != XUI_OK) return result;
        if (!state.bEnabled) return state.iDisabledReason;
        range = e->view.selection;
        doc_editor_break_group(e);
        result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
        in_list = doc_list_style_range_can(t->draft, &range, flags,
            flags & XUI_DOC_ORDERED ? 1 : 0, NULL, NULL) == XUI_OK;
        if (in_list && state.bActive)
            result = xuiDocumentTxnUnlistRange(t, &range, &after);
        else if (in_list)
            result = xuiDocumentTxnSetListStyleRange(t, &range, flags,
                flags & XUI_DOC_ORDERED ? 1 : 0, &after);
        else
            result = xuiDocumentTxnCreateListRange(t, &range, flags,
                flags & XUI_DOC_ORDERED ? 1 : 0, &list_id, &after);
        if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
        xuiDocumentTxnRelease(t); return result;
    }
    case XUI_DOC_EDIT_MOVE_BLOCK_UP: case XUI_DOC_EDIT_MOVE_BLOCK_DOWN: {
        xui_doc_command_state_t state = {0}; xui_document_transaction t = NULL;
        xui_doc_range_t after;
        result = doc_editor_query_command(e, command, &state);
        if (result != XUI_OK) return result;
        if (!state.bEnabled) return state.iDisabledReason;
        range = e->view.selection;
        doc_editor_break_group(e);
        result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
        result = xuiDocumentTxnMoveBlockRange(t, &range,
            command == XUI_DOC_EDIT_MOVE_BLOCK_DOWN, &after);
        if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
        xuiDocumentTxnRelease(t); return result;
    }
    case XUI_DOC_EDIT_BLOCK_QUOTE:
    case XUI_DOC_EDIT_INSERT_RULE:
    case XUI_DOC_EDIT_INSERT_CODE_BLOCK: {
        xui_doc_command_state_t state = {0};
        result = doc_editor_query_command(e, command, &state);
        if (result != XUI_OK) return result;
        if (!state.bEnabled) return state.iDisabledReason;
        if (command == XUI_DOC_EDIT_BLOCK_QUOTE)
            return state.bActive ? xuiDocumentEditorUnwrapQuote(w) :
                xuiDocumentEditorWrapQuote(w);
        if (command == XUI_DOC_EDIT_INSERT_RULE)
            return xuiDocumentEditorInsertRule(w);
        return xuiDocumentEditorInsertCodeBlock(w, NULL, "", 0);
    }
    case XUI_DOC_EDIT_TOGGLE_TASK: {
        xui_doc_command_state_t state = {0}; doc_node* item;
        result = doc_editor_query_command(e, command, &state);
        if (result != XUI_OK) return result;
        if (!state.bEnabled) return state.iDisabledReason;
        item = doc_editor_task_context(e->view.renderer->snapshot->state,
            &e->view.selection.tCaret);
        return item ? doc_editor_toggle_task_item(e, item->id) : XUI_ERROR_UNSUPPORTED;
    }
    case XUI_DOC_EDIT_PARAGRAPH: case XUI_DOC_EDIT_HEADING_1: case XUI_DOC_EDIT_HEADING_2:
    case XUI_DOC_EDIT_HEADING_3: case XUI_DOC_EDIT_HEADING_4: case XUI_DOC_EDIT_HEADING_5:
    case XUI_DOC_EDIT_HEADING_6: {
        xui_doc_command_state_t state = {0}; xui_document_transaction t = NULL; xui_doc_range_t after;
        result = doc_editor_query_command(e, command, &state); if (result != XUI_OK) return result;
        if (!state.bEnabled) return state.iDisabledReason;
        range = e->view.selection;
        doc_editor_break_group(e);
        result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
        result = xuiDocumentTxnSetHeading(t, &range, doc_editor_heading_level(command), &after);
        if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
        xuiDocumentTxnRelease(t); return result;
    }
    case XUI_DOC_EDIT_ALIGN_LEFT: case XUI_DOC_EDIT_ALIGN_CENTER:
    case XUI_DOC_EDIT_ALIGN_RIGHT: case XUI_DOC_EDIT_ALIGN_JUSTIFY: {
        xui_doc_command_state_t state = {0}; xui_document_transaction t = NULL;
        xui_doc_block_style_t style = {0}; xui_doc_range_t after;
        result = doc_editor_query_command(e, command, &state); if (result != XUI_OK) return result;
        if (!state.bEnabled) return state.iDisabledReason;
        range = e->view.selection; style.iSize = sizeof(style); style.iAlignment = doc_editor_alignment(command);
        doc_editor_break_group(e);
        result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
        result = xuiDocumentTxnSetBlockStyleRange(t, &range, XUI_DOC_BLOCK_STYLE_ALIGNMENT, &style, &after);
        if (result == XUI_OK) result = doc_editor_commit(e, t, &after);
        xuiDocumentTxnRelease(t); return result;
    }
    case XUI_DOC_EDIT_BACKSPACE: case XUI_DOC_EDIT_DELETE:
        range = e->view.selection; result = doc_render_compare(e->view.renderer, &range.tAnchor, &range.tCaret, &order);
        if (result != XUI_OK) return result;
        if (!order) { result = doc_editor_horizontal(e, range.tCaret, command == XUI_DOC_EDIT_DELETE, 0, &range.tCaret); if (result != XUI_OK) return result; }
        return doc_editor_replace(e, &range, "", 0, !interactive ? DOC_EDITOR_PROGRAM : command == XUI_DOC_EDIT_BACKSPACE ? DOC_EDITOR_BACKSPACE : DOC_EDITOR_DELETE);
    case XUI_DOC_EDIT_BOLD: case XUI_DOC_EDIT_ITALIC: case XUI_DOC_EDIT_UNDERLINE: case XUI_DOC_EDIT_STRIKE:
    case XUI_DOC_EDIT_CODE: case XUI_DOC_EDIT_SUBSCRIPT: case XUI_DOC_EDIT_SUPERSCRIPT:
    case XUI_DOC_EDIT_HIGHLIGHT:
        mark = doc_editor_command_mark(command);
        { xui_doc_command_state_t state = {0}; state.iSize = sizeof(state);
          result = doc_editor_query_command(e, command, &state); if (result != XUI_OK) return result;
          if (!state.bEnabled) return state.iDisabledReason;
          { uint32_t clear = state.bActive ? mark :
                mark == XUI_DOC_SUBSCRIPT ? XUI_DOC_SUPERSCRIPT :
                mark == XUI_DOC_SUPERSCRIPT ? XUI_DOC_SUBSCRIPT : 0;
            if (!state.bActive && mark == XUI_DOC_CODE &&
                xuiDocumentGetProfile(e->view.document) == XUI_DOCUMENT_MARKDOWN)
                clear |= XUI_DOC_BOLD | XUI_DOC_ITALIC | XUI_DOC_STRIKE |
                    XUI_DOC_SUBSCRIPT | XUI_DOC_SUPERSCRIPT | XUI_DOC_HIGHLIGHT;
            return doc_editor_set_marks(e, state.bActive ? 0 : mark, clear); } }
    default: return XUI_ERROR_UNSUPPORTED;
    }
}
XUI_API int xuiDocumentEditorExecute(xui_widget w, uint32_t command)
{
    doc_editor_data* e = doc_editor_get(w); if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (doc_editor_api_busy(e)) return XUI_DOC_ERROR_BUSY;
    if (command == XUI_DOC_EDIT_BACKSPACE || command == XUI_DOC_EDIT_DELETE) {
        xui_doc_command_state_t state = {0};
        int result;
        state.iSize = sizeof(state);
        result = doc_editor_query_command(e, command, &state);
        if (result != XUI_OK) return result;
        if (!state.bEnabled) return state.iDisabledReason;
    }
    e->action_time = xrtClock(); return doc_editor_execute(e, command, 0);
}
static int doc_editor_composition(doc_editor_data* e, const xui_event_t* event)
{
    xui_document_transaction t = NULL; xui_document_renderer renderer = NULL; xui_document_snapshot snapshot = NULL;
    xui_document_prepare preview = NULL;
    xui_doc_position_t caret; int result; xui_rect_t content = xuiWidgetGetContentRect(e->view.widget);
    if (e->desc.bReadOnly) return XUI_OK;
    if (e->view.table_selection.iTableId) return XUI_ERROR_UNSUPPORTED;
    if (event->iTextSize < 0 || event->iTextSize >= XUI_EVENT_TEXT_CAPACITY || !doc_utf8(event->sText, (uint64_t)event->iTextSize)) return XUI_DOC_ERROR_UTF8;
    if (!e->composing) { e->composition_range = e->view.selection; doc_editor_break_group(e); }
    /* Replacement offsets are UTF-8 offsets in the active paragraph/source
     * projection. Never interpret them as whole-document flat rich offsets. */
    if (event->bCompositionReplacementRange) {
        if (e->view.selection.tCaret.iKind == XUI_DOC_POSITION_SOURCE) {
            uint64_t length = doc_seq_size(doc_render_source(e->view.renderer));
            if (length > INT_MAX) return XUI_DOC_ERROR_LIMIT;
            if (event->iCompositionReplacementStart < 0 ||
                event->iCompositionReplacementEnd < event->iCompositionReplacementStart ||
                (uint64_t)event->iCompositionReplacementEnd > length) return XUI_ERROR_INVALID_ARGUMENT;
            e->composition_range.tAnchor = e->view.selection.tCaret;
            e->composition_range.tCaret = e->view.selection.tCaret;
            e->composition_range.tAnchor.iOffset = (uint64_t)event->iCompositionReplacementStart;
            e->composition_range.tCaret.iOffset = (uint64_t)event->iCompositionReplacementEnd;
        } else {
            doc_edit_projection projection;
            result = doc_editor_project(e, &e->view.selection.tCaret, &projection); if (result != XUI_OK) return result;
            if (event->iCompositionReplacementStart < 0 || event->iCompositionReplacementEnd < event->iCompositionReplacementStart || event->iCompositionReplacementEnd > projection.length) {
                free(projection.text); free(projection.spans); return XUI_ERROR_INVALID_ARGUMENT;
            }
            e->composition_range.tAnchor = doc_editor_unproject(&projection, e->view.selection.tCaret, event->iCompositionReplacementStart, 1);
            e->composition_range.tCaret = doc_editor_unproject(&projection, e->view.selection.tCaret, event->iCompositionReplacementEnd, 0);
            free(projection.text); free(projection.spans);
        }
    }
    if (!event->bCompositionActive) {
        xui_doc_range_t range = e->composition_range; xui_document_renderer projection = e->projection;
        xui_widget w = e->view.widget; uint64_t epoch = e->input_epoch; int composing = e->composing;
        /* Keep preedit/range until the preceding undo unit can publish. A
         * deferred confirmation must not pick up a different replacement. */
        e->composing = 0; e->projection = NULL;
        result = event->iTextSize ? doc_editor_replace(e, &range, event->sText, (uint64_t)event->iTextSize, DOC_EDITOR_ISOLATED) : XUI_OK;
        if (xuiInternalWidgetIsValid(w) && result != XUI_OK && epoch == e->input_epoch) {
            e->composing = composing; e->projection = projection;
        } else xuiDocumentRendererRelease(projection);
        return result;
    }
    if (e->view.renderer->mode == XUI_DOC_SOURCE_TEXT) {
        xui_doc_source_patch_t patch = {0}; int order;
        result = doc_render_compare(e->view.renderer, &e->composition_range.tAnchor, &e->composition_range.tCaret, &order);
        if (result != XUI_OK) return result;
        patch.iSize = sizeof(patch); patch.iStart = (order < 0 ? e->composition_range.tAnchor : e->composition_range.tCaret).iOffset;
        patch.iEnd = (order < 0 ? e->composition_range.tCaret : e->composition_range.tAnchor).iOffset;
        patch.sText = event->sText; patch.iTextBytes = (uint64_t)event->iTextSize;
        result = doc_prepare_preview(e->view.document, e->input ? e->input->prepare : NULL, &patch, &preview);
        if (result == XUI_OK) {
            snapshot = e->view.renderer->snapshot; xuiDocumentSnapshotRetain(snapshot);
            result = xuiDocumentPrepareSourcePosition(preview, patch.iStart + patch.iTextBytes, XUI_DOC_AFTER, &caret);
        }
    } else {
        result = doc_editor_begin(e, &t, 0); if (result != XUI_OK) return result;
        result = xuiDocumentTxnReplaceRange(t, &e->composition_range, event->sText, (uint64_t)event->iTextSize, &caret);
    }
    if (result == XUI_OK && !preview) {
        doc_state* state = doc_state_clone(t->draft);
        if (!state) result = XUI_ERROR_OUT_OF_MEMORY;
        else { result = doc_snapshot_create(state, t->document->identity, t->base_revision, &snapshot); doc_state_release(state); }
    }
    if (result == XUI_OK) result = xuiDocumentRendererCreate(e->view.renderer->context, &e->view.renderer->desc, &renderer);
    if (result == XUI_OK) result = xuiDocumentRendererSetMode(renderer, e->view.renderer->mode);
    if (result == XUI_OK) result = xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL);
    if (result == XUI_OK && preview) result = xuiDocumentRendererSetSourceInput(renderer, preview);
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
    xuiDocumentPrepareRelease(preview); xuiDocumentSnapshotRelease(snapshot); xuiDocumentRendererRelease(renderer); xuiDocumentTxnRelease(t); return result;
}
static int doc_editor_vertical_probe(xui_document_renderer renderer, double x,
    double y, const xui_doc_rect_t* origin, int down,
    xui_doc_position_t* position, int* moved)
{
    xui_doc_rect_t rect;
    double overlap, center = origin->y + origin->height * .5;
    int result = xuiDocumentRendererHitTest(renderer, x, fmax(0, y), position);
    if (result != XUI_OK) return result;
    result = xuiDocumentRendererGetCaretRect(renderer, position, &rect);
    if (result != XUI_OK) return result;
    overlap = fmin(origin->y + origin->height, rect.y + rect.height) -
        fmax(origin->y, rect.y);
    *moved = overlap <= .001 &&
        (down ? rect.y + rect.height * .5 > center :
                rect.y + rect.height * .5 < center);
    return XUI_OK;
}
/* Find the first rendered row past the caret. A fixed one-line step can land
 * inside paragraph spacing, while a larger step can skip a short next row. */
static int doc_editor_vertical_line(xui_document_renderer renderer,
    double x, const xui_doc_rect_t* caret, int down,
    const xui_doc_position_t* before, xui_doc_position_t* after)
{
    double center = caret->y + caret->height * .5;
    double step = fmax(1, caret->height * .5), same = 0;
    unsigned attempt;
    *after = *before;
    for (attempt = 0; attempt < 24; attempt++) {
        xui_doc_position_t candidate;
        double y = down ? center + step : fmax(0, center - step);
        int moved = 0, result = doc_editor_vertical_probe(renderer,
            x, y, caret, down, &candidate, &moved);
        if (result != XUI_OK) return result;
        if (moved) {
            double different = step;
            unsigned refine = 0;
            *after = candidate;
            while (same && different - same > .25 && refine++ < 32) {
                double middle = same + (different - same) * .5;
                y = down ? center + middle : fmax(0, center - middle);
                result = doc_editor_vertical_probe(renderer,
                    x, y, caret, down, &candidate, &moved);
                if (result != XUI_OK) return result;
                if (moved) { different = middle; *after = candidate; }
                else same = middle;
            }
            return XUI_OK;
        }
        if (!down && y <= 0) break;
        same = step;
        step *= 2;
    }
    return XUI_OK;
}
static int doc_editor_key(doc_editor_data* e, const xui_event_t* event)
{
    xui_widget w = e->view.widget; int key = event->iKey, ctrl = (event->iModifiers & XUI_MOD_CTRL) != 0, shift = (event->iModifiers & XUI_MOD_SHIFT) != 0;
    uint32_t command = 0; xui_doc_position_t p; int result, order;
    if (ctrl && !(event->iModifiers & XUI_MOD_ALT)) {
        int letter = key >= 'a' && key <= 'z' ? key - 'a' + 'A' : key;
        if (letter == 'F' || letter == 'H')
            return doc_find_ui_open(w, &e->find_ui, letter == 'H');
    }
    if (ctrl) switch (key >= 'a' && key <= 'z' ? key - 'a' + 'A' : key) {
    case 'A': command = XUI_DOC_EDIT_SELECT_ALL; break; case 'C': command = XUI_DOC_EDIT_COPY; break;
    case 'X': command = XUI_DOC_EDIT_CUT; break; case 'V': command = XUI_DOC_EDIT_PASTE; break;
    case 'Z': command = shift ? XUI_DOC_EDIT_REDO : XUI_DOC_EDIT_UNDO; break; case 'Y': command = XUI_DOC_EDIT_REDO; break;
    case 'B': command = XUI_DOC_EDIT_BOLD; break; case 'I': command = XUI_DOC_EDIT_ITALIC; break; case 'U': command = XUI_DOC_EDIT_UNDERLINE; break;
    }
    if ((event->iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT | XUI_MOD_SHIFT)) ==
        (XUI_MOD_CTRL | XUI_MOD_ALT | XUI_MOD_SHIFT)) {
        if (key == XUI_KEY_LEFT) command = XUI_DOC_EDIT_TABLE_COLUMN_NARROWER;
        else if (key == XUI_KEY_RIGHT) command = XUI_DOC_EDIT_TABLE_COLUMN_WIDER;
        else if (key == '0') command = XUI_DOC_EDIT_TABLE_COLUMN_AUTO;
    }
    if ((event->iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT | XUI_MOD_SHIFT)) == XUI_MOD_ALT &&
        (key == XUI_KEY_UP || key == XUI_KEY_DOWN)) {
        xui_doc_command_state_t state = {0};
        command = key == XUI_KEY_UP ? XUI_DOC_EDIT_MOVE_BLOCK_UP : XUI_DOC_EDIT_MOVE_BLOCK_DOWN;
        result = doc_editor_query_command(e, command, &state);
        if (result != XUI_OK) return result;
        if (!state.bEnabled) return XUI_OK;
    }
    if (key == XUI_KEY_ESCAPE && e->view.table_selection.iTableId) {
        return doc_view_set_table_selection(&e->view, NULL);
    }
    if (key == XUI_KEY_ESCAPE) return doc_editor_cancel_composition(e);
    if (key == XUI_KEY_BACKSPACE) command = XUI_DOC_EDIT_BACKSPACE;
    if (key == XUI_KEY_DELETE) command = XUI_DOC_EDIT_DELETE;
    if (key == XUI_KEY_ENTER) command = XUI_DOC_EDIT_ENTER;
    if (key == XUI_KEY_TAB && !ctrl && !(event->iModifiers & XUI_MOD_ALT)) {
        xui_doc_command_state_t state = {0};
        command = shift ? XUI_DOC_EDIT_TABLE_PREVIOUS_CELL : XUI_DOC_EDIT_TABLE_NEXT_CELL;
        result = doc_editor_query_command(e, command, &state); if (result != XUI_OK) return result;
        if (state.bEnabled) return doc_editor_execute(e, command, 1);
        command = shift ? XUI_DOC_EDIT_OUTDENT_LIST : XUI_DOC_EDIT_INDENT_LIST;
        result = doc_editor_query_command(e, command, &state); if (result != XUI_OK) return result;
        if (!state.bEnabled) return XUI_ERROR_NOT_FOUND;
    }
    if (command) {
        return doc_editor_execute(e, command, 1);
    }
    if (e->composing) return XUI_OK;
    if ((event->iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT | XUI_MOD_SHIFT)) ==
        (XUI_MOD_ALT | XUI_MOD_SHIFT) &&
        (key == XUI_KEY_LEFT || key == XUI_KEY_RIGHT ||
         key == XUI_KEY_UP || key == XUI_KEY_DOWN)) {
        result = doc_view_extend_table_selection(&e->view, key);
        if (result != XUI_ERROR_NOT_FOUND) return result;
    }
    p = e->view.selection.tCaret;
    if (key == XUI_KEY_LEFT || key == XUI_KEY_RIGHT) {
        result = doc_render_compare(e->view.renderer, &e->view.selection.tAnchor, &p, &order);
        if (result != XUI_OK) return result;
        if (!shift && order) {
            xui_doc_rect_t anchor, caret;
            result = doc_view_layout(&e->view); if (result != XUI_OK) return result;
            if (!doc_render_position_may_bidi(e->view.renderer, &e->view.selection.tAnchor) &&
                !doc_render_position_may_bidi(e->view.renderer, &p)) {
                if ((key == XUI_KEY_LEFT) == (order < 0)) p = e->view.selection.tAnchor;
            } else {
                result = xuiDocumentRendererGetCaretRect(e->view.renderer, &e->view.selection.tAnchor, &anchor);
                if (result == XUI_OK) result = xuiDocumentRendererGetCaretRect(e->view.renderer, &p, &caret);
                if (result != XUI_OK) return result;
                if (fabs(anchor.y - caret.y) < .001) {
                    if ((key == XUI_KEY_LEFT && anchor.x < caret.x) ||
                        (key == XUI_KEY_RIGHT && anchor.x > caret.x)) p = e->view.selection.tAnchor;
                } else if ((key == XUI_KEY_LEFT) == (order < 0)) p = e->view.selection.tAnchor;
            }
        } else {
            int logical_right = key == XUI_KEY_RIGHT;
            result = doc_view_layout(&e->view); if (result != XUI_OK) return result;
            if (!ctrl) result = doc_render_visual_move(e->view.renderer, &p, logical_right, &p, &logical_right);
            else result = XUI_ERROR_NOT_FOUND;
            if (result == XUI_ERROR_NOT_FOUND) {
                result = doc_editor_horizontal(e, p, logical_right, ctrl, &p);
                p.iAffinity = XUI_DOC_AFTER;
            }
            if (result != XUI_OK) return result;
        }
    } else if (key == XUI_KEY_UP || key == XUI_KEY_DOWN || key == XUI_KEY_HOME || key == XUI_KEY_END || key == XUI_KEY_PAGE_UP || key == XUI_KEY_PAGE_DOWN) {
        xui_doc_position_t before = p;
        xui_doc_rect_t caret; xui_rect_t content = xuiWidgetGetContentRect(w); double x, y;
        result = doc_view_layout(&e->view); if (result != XUI_OK) return result;
        result = xuiDocumentRendererGetCaretRect(e->view.renderer, &p, &caret); if (result != XUI_OK) return result;
        x = caret.x; y = caret.y + caret.height * .5;
        if (key == XUI_KEY_HOME || key == XUI_KEY_END) {
            if (ctrl) {
                doc_state* s = e->view.renderer->snapshot->state;
                if (p.iKind == XUI_DOC_POSITION_SOURCE) p.iOffset = key == XUI_KEY_END ? doc_seq_size(doc_render_source(e->view.renderer)) : 0;
                else p = doc_editor_edge(s, p, DOC_ROOT, key == XUI_KEY_END);
                p.iAffinity = XUI_DOC_AFTER;
                goto select;
            }
            x = key == XUI_KEY_HOME ? 0 : 1e20;
        } else {
            if (!e->vertical_goal_active || e->vertical_layout_width != content.fW) {
                e->vertical_goal_x = caret.x;
                e->vertical_layout_width = content.fW;
                e->vertical_goal_active = 1;
            }
            x = e->vertical_goal_x;
            y += (key == XUI_KEY_UP || key == XUI_KEY_PAGE_UP ? -1 : 1) *
                (key == XUI_KEY_PAGE_UP || key == XUI_KEY_PAGE_DOWN ?
                    fmax(1, content.fH) : caret.height);
        }
        if (key == XUI_KEY_UP || key == XUI_KEY_DOWN)
            result = doc_editor_vertical_line(e->view.renderer, x, &caret,
                key == XUI_KEY_DOWN, &before, &p);
        else result = xuiDocumentRendererHitTest(e->view.renderer,
            x, fmax(0, y), &p);
        if (result != XUI_OK) return result;
        if ((key == XUI_KEY_UP || key == XUI_KEY_DOWN) && !ctrl &&
            e->view.renderer->mode == XUI_DOC_VISUAL &&
            p.iKind == before.iKind && p.iNodeId == before.iNodeId && p.iOffset == before.iOffset)
            doc_editor_table_vertical(e->view.renderer->snapshot->state, &before,
                key == XUI_KEY_DOWN, &p);
    } else return XUI_ERROR_NOT_FOUND;
select:
    doc_editor_break_group(e);
    memset(&e->view.table_selection, 0, sizeof(e->view.table_selection));
    /* Visual navigation keeps the hit's upstream affinity at a wrapped row
     * end. Logical horizontal/document-edge moves choose AFTER above. */
    e->view.selection.tCaret = p; if (!shift) e->view.selection.tAnchor = p;
    e->view.pending_edit_events |= 2;
    e->has_pending_marks = e->pending_clear_format = 0;
    e->pending_set = e->pending_clear = 0;
    e->pending_style_fields = 0; doc_editor_reveal(e); return XUI_OK;
}
typedef struct doc_editor_menu_command {
    int text_id;
    uint32_t command;
    const char* shortcut;
    int table_only;
} doc_editor_menu_command;
static const doc_editor_menu_command doc_editor_menu_commands[] = {
    {XUI_TR_EDIT_UNDO, XUI_DOC_EDIT_UNDO, "Ctrl+Z", 0},
    {XUI_TR_EDIT_REDO, XUI_DOC_EDIT_REDO, "Ctrl+Y", 0},
    {0, 0, NULL, 0},
    {XUI_TR_EDIT_CUT, XUI_DOC_EDIT_CUT, "Ctrl+X", 0},
    {XUI_TR_EDIT_COPY, XUI_DOC_EDIT_COPY, "Ctrl+C", 0},
    {XUI_TR_EDIT_PASTE, XUI_DOC_EDIT_PASTE, "Ctrl+V", 0},
    {XUI_TR_EDIT_DELETE, XUI_DOC_EDIT_DELETE, NULL, 0},
    {0, 0, NULL, 0},
    {XUI_TR_EDIT_SELECT_ALL, XUI_DOC_EDIT_SELECT_ALL, "Ctrl+A", 0},
    {0, 0, NULL, 1},
    {XUI_TR_RICH_INSERT_ROW_BEFORE, XUI_DOC_EDIT_TABLE_INSERT_ROW_BEFORE, NULL, 1},
    {XUI_TR_RICH_INSERT_ROW_AFTER, XUI_DOC_EDIT_TABLE_INSERT_ROW_AFTER, NULL, 1},
    {XUI_TR_RICH_DELETE_ROW, XUI_DOC_EDIT_TABLE_DELETE_ROW, NULL, 1},
    {XUI_TR_RICH_INSERT_COLUMN_BEFORE, XUI_DOC_EDIT_TABLE_INSERT_COLUMN_BEFORE, NULL, 1},
    {XUI_TR_RICH_INSERT_COLUMN_AFTER, XUI_DOC_EDIT_TABLE_INSERT_COLUMN_AFTER, NULL, 1},
    {XUI_TR_RICH_DELETE_COLUMN, XUI_DOC_EDIT_TABLE_DELETE_COLUMN, NULL, 1},
    {XUI_TR_RICH_MERGE_CELLS, XUI_DOC_EDIT_TABLE_MERGE_CELLS, NULL, 1},
    {XUI_TR_RICH_SPLIT_CELL, XUI_DOC_EDIT_TABLE_SPLIT_CELL, NULL, 1}
};
static void doc_editor_menu_select(xui_widget menu, int index, int value, void* user)
{
    xui_widget w = user;
    doc_editor_data* e;
    int result;
    (void)menu; (void)index;
    if (value <= 0 || !doc_editor_get(w)) return;
    result = xuiDocumentEditorExecute(w, (uint32_t)value);
    e = xuiInternalWidgetIsValid(w) ? doc_editor_get(w) : NULL;
    if (result < 0 && e && e->desc.onError)
        e->desc.onError(w, result, e->desc.pUser);
}
static int doc_editor_open_menu(doc_editor_data* e, float x, float y)
{
    xui_widget w = e->view.widget;
    xui_context context = xuiWidgetGetContext(w);
    xui_menu_item_t items[sizeof(doc_editor_menu_commands) / sizeof(doc_editor_menu_commands[0])] = {0};
    char* labels[sizeof(e->menu_text) / sizeof(e->menu_text[0])] = {0};
    doc_table_cell_slot table_slot;
    size_t i, label_count = 0;
    int result, item_count = 0, in_table;
    if (e->input || e->stream_open) return XUI_DOC_ERROR_BUSY;
    result = doc_editor_sync(e);
    if (result != XUI_OK) return result;
    in_table = e->view.table_selection.iTableId != 0 ||
        doc_editor_table_context(e->view.renderer->snapshot->state,
            &e->view.selection.tCaret, &table_slot) == XUI_OK;
    if (!e->menu) {
        xui_menu_desc_t desc = {0};
        desc.iSize = sizeof(desc); desc.pOwner = w;
        desc.pFont = xuiGetDefaultFont(context);
        result = xuiMenuCreate(context, &e->menu, &desc);
        if (result != XUI_OK) return result;
        result = xuiMenuSetSelect(e->menu, doc_editor_menu_select, w);
        if (result != XUI_OK) return result;
    }
    for (i = 0; i < sizeof(doc_editor_menu_commands) / sizeof(doc_editor_menu_commands[0]); i++) {
        const doc_editor_menu_command* command = &doc_editor_menu_commands[i];
        xui_doc_command_state_t state = {0};
        const char* translated;
        size_t bytes;
        if (command->table_only && !in_table) continue;
        if (!command->command) {
            items[item_count++].iType = XUI_MENU_ITEM_SEPARATOR;
            continue;
        }
        state.iSize = sizeof(state);
        result = doc_editor_query_command(e, command->command, &state);
        if (result != XUI_OK) goto failed;
        translated = xuiTranslate(context, command->text_id);
        if (!translated) translated = "";
        bytes = strlen(translated);
        if (label_count >= sizeof(labels) / sizeof(labels[0])) {
            result = XUI_ERROR_LIMIT_EXCEEDED; goto failed;
        }
        labels[label_count] = malloc(bytes + 1);
        if (!labels[label_count]) { result = XUI_ERROR_OUT_OF_MEMORY; goto failed; }
        memcpy(labels[label_count], translated, bytes + 1);
        items[item_count].sText = labels[label_count++];
        items[item_count].sShortcut = command->shortcut;
        items[item_count].iType = XUI_MENU_ITEM_NORMAL;
        items[item_count].iState = state.bEnabled ? XUI_MENU_ITEM_ENABLED : 0;
        items[item_count++].iValue = (int)command->command;
    }
    result = xuiMenuSetItems(e->menu, items, item_count);
    /* Menu items borrow their labels. Replace every old label only after the
     * menu has switched to the newly allocated set, including size errors. */
    for (i = 0; i < sizeof(e->menu_text) / sizeof(e->menu_text[0]); i++) {
        free(e->menu_text[i]); e->menu_text[i] = labels[i];
    }
    if (result != XUI_OK) return result;
    return xuiMenuOpenAt(e->menu, w, x, y);
failed:
    for (i = 0; i < sizeof(labels) / sizeof(labels[0]); i++) free(labels[i]);
    return result;
}
XUI_API xui_widget xuiDocumentEditorGetMenuWidget(xui_widget w)
{
    doc_editor_data* e = doc_editor_get(w);
    return e ? e->menu : NULL;
}
XUI_API int xuiDocumentEditorOpenMenu(xui_widget w, float x, float y)
{
    doc_editor_data* e = doc_editor_get(w);
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    return doc_editor_api_busy(e) ? XUI_DOC_ERROR_BUSY : doc_editor_open_menu(e, x, y);
}
static int doc_editor_context_menu(doc_editor_data* e, const xui_event_t* event)
{
    xui_widget w = e->view.widget;
    xui_rect_t world = xuiWidgetGetWorldRect(w), anchor = {0};
    float x, y;
    int result;
    if (e->input || e->stream_open) return XUI_DOC_ERROR_BUSY;
    if (event->iKey == XUI_KEY_CONTEXT_MENU) {
        anchor = doc_editor_caret_rect(e);
        anchor.fX += world.fX; anchor.fY += world.fY;
    } else if (!e->view.desc.bDisableSelection) {
        xui_doc_position_t hit, left = e->view.selection.tAnchor, right = e->view.selection.tCaret;
        xui_doc_range_t range;
        int order, from_left, to_right, inside = 0;
        result = xuiDocumentViewHitTest(w, event->fX - world.fX,
            event->fY - world.fY, &hit);
        if (result != XUI_OK) return result;
        result = doc_render_compare(e->view.renderer, &left, &right, &order);
        if (result != XUI_OK) return result;
        if (e->view.table_selection.iTableId) {
            xui_doc_cell_hit_t cell = {0};
            const xui_doc_table_selection_t* selected = &e->view.table_selection;
            cell.iSize = sizeof(cell);
            if (xuiDocumentViewHitTestCell(w, event->fX - world.fX,
                event->fY - world.fY, &cell) == XUI_OK &&
                cell.iTableId == selected->iTableId &&
                cell.iRow >= selected->iRow && cell.iColumn >= selected->iColumn &&
                (uint64_t)cell.iRow + cell.iRowSpan <= (uint64_t)selected->iRow + selected->iRows &&
                (uint64_t)cell.iColumn + cell.iColumnSpan <= (uint64_t)selected->iColumn + selected->iColumns)
                inside = 1;
        } else if (order) {
            if (order > 0) { xui_doc_position_t swap = left; left = right; right = swap; }
            result = doc_render_compare(e->view.renderer, &left, &hit, &from_left);
            if (result != XUI_OK) return result;
            result = doc_render_compare(e->view.renderer, &hit, &right, &to_right);
            if (result != XUI_OK) return result;
            inside = from_left <= 0 && to_right < 0;
        }
        if (!inside) {
            range = e->view.selection;
            if (e->view.renderer->mode != XUI_DOC_VISUAL ||
                doc_view_object_range(&e->view, hit.iNodeId, &range) != XUI_OK)
                range.tAnchor = range.tCaret = hit;
            result = doc_view_set_selection(&e->view, &range);
            if (result != XUI_OK) return result;
            e->has_pending_marks = e->pending_clear_format = 0;
            e->pending_set = e->pending_clear = e->pending_style_fields = 0;
        }
    }
    doc_editor_break_group(e);
    result = xuiSetFocusWidget(xuiWidgetGetContext(w), w);
    if (result != XUI_OK) return result;
    if (!xuiInternalWidgetIsValid(w)) return (int)XUI_EVENT_DISPATCH_STOP;
    xuiInternalContextMenuPoint(event, anchor, &x, &y);
    result = doc_editor_open_menu(e, x, y);
    return result == XUI_OK ? (int)XUI_EVENT_DISPATCH_STOP : result;
}
static int doc_editor_task_marker_hit(doc_editor_data* e,
    const xui_event_t* event, xui_doc_node_id* item)
{
    xui_rect_t world = xuiWidgetGetWorldRect(e->view.widget);
    xui_rect_t content = xuiWidgetGetContentRect(e->view.widget);
    int result = doc_view_layout(&e->view);
    if (result != XUI_OK) return result;
    return xuiDocumentRendererHitTaskMarker(e->view.renderer,
        event->fX - world.fX - content.fX + e->view.scroll_x,
        event->fY - world.fY - content.fY + e->view.scroll_y, item);
}
static int doc_editor_task_marker_event(doc_editor_data* e,
    const xui_event_t* event)
{
    xui_widget w = e->view.widget;
    xui_context context = xuiWidgetGetContext(w);
    xui_doc_node_id item = 0, pressed;
    int result;
    if (event->iType == XUI_EVENT_BLUR ||
        event->iType == XUI_EVENT_POINTER_CAPTURE_LOST) {
        if (event->iType == XUI_EVENT_BLUR) {
            doc_editor_cancel_task_marker(e, !e->view.replaying_input);
            return XUI_ERROR_NOT_FOUND;
        }
        if (event->iPointerId == e->task_pointer_id &&
            event->iPointerType == e->task_pointer_type && e->task_pressed) {
            doc_editor_cancel_task_marker(e, 0);
            return XUI_EVENT_DISPATCH_STOP;
        }
        return XUI_ERROR_NOT_FOUND;
    }
    if (event->iType == XUI_EVENT_POINTER_DOWN) {
        if (e->task_pressed) {
            doc_editor_cancel_task_marker(e, !e->view.replaying_input);
            if (!xuiInternalWidgetIsValid(w)) return XUI_EVENT_DISPATCH_STOP;
        }
        if (event->iButton && event->iButton != XUI_POINTER_BUTTON_LEFT)
            return XUI_ERROR_NOT_FOUND;
        if (e->desc.bReadOnly || e->view.renderer->mode != XUI_DOC_VISUAL ||
            (event->iModifiers & (XUI_MOD_ALT | XUI_MOD_SHIFT)))
            return XUI_ERROR_NOT_FOUND;
        result = doc_editor_task_marker_hit(e, event, &item);
        if (result != XUI_OK) return result;
        e->task_pressed = item;
        e->task_pointer_id = event->iPointerId;
        e->task_pointer_type = event->iPointerType;
        if (!e->view.replaying_input) {
            result = xuiSetFocusWidget(context, w);
            if (result != XUI_OK) { e->task_pressed = 0; return result; }
            if (!xuiInternalWidgetIsValid(w)) return XUI_EVENT_DISPATCH_STOP;
            result = xuiSetPointerCaptureEx(context, event->iPointerId,
                event->iPointerType, w);
            if (result != XUI_OK) { e->task_pressed = 0; return result; }
        }
        return XUI_EVENT_DISPATCH_STOP;
    }
    if (!e->task_pressed) return XUI_ERROR_NOT_FOUND;
    if (event->iType != XUI_EVENT_POINTER_MOVE &&
        event->iType != XUI_EVENT_POINTER_UP) return XUI_ERROR_NOT_FOUND;
    if (event->iPointerId != e->task_pointer_id ||
        event->iPointerType != e->task_pointer_type)
        return XUI_EVENT_DISPATCH_STOP;
    if (!e->view.replaying_input &&
        xuiGetPointerCaptureEx(context, e->task_pointer_id,
            e->task_pointer_type) != w) {
        doc_editor_cancel_task_marker(e, 0);
        return XUI_EVENT_DISPATCH_STOP;
    }
    if (event->iType == XUI_EVENT_POINTER_MOVE)
        return XUI_EVENT_DISPATCH_STOP;
    if (event->iButton && event->iButton != XUI_POINTER_BUTTON_LEFT)
        return XUI_EVENT_DISPATCH_STOP;
    pressed = e->task_pressed;
    doc_editor_cancel_task_marker(e, !e->view.replaying_input);
    if (!xuiInternalWidgetIsValid(w) || e->desc.bReadOnly ||
        e->view.renderer->mode != XUI_DOC_VISUAL)
        return XUI_EVENT_DISPATCH_STOP;
    result = doc_editor_task_marker_hit(e, event, &item);
    if (result == XUI_ERROR_NOT_FOUND || item != pressed)
        return XUI_EVENT_DISPATCH_STOP;
    if (result != XUI_OK) return result;
    result = doc_editor_toggle_task_item(e, pressed);
    return result == XUI_OK ? (int)XUI_EVENT_DISPATCH_STOP : result;
}
static int doc_editor_handle_event(doc_editor_data* e, const xui_event_t* event, uint64_t received)
{
    xui_widget w = e->view.widget; int result;
    e->action_time = received;
    result = doc_editor_sync(e); if (result != XUI_OK) return result;
    if ((event->iType == XUI_EVENT_KEY_DOWN &&
         ((event->iKey != XUI_KEY_UP && event->iKey != XUI_KEY_DOWN &&
           event->iKey != XUI_KEY_PAGE_UP && event->iKey != XUI_KEY_PAGE_DOWN) ||
          (event->iModifiers & XUI_MOD_ALT))) ||
        event->iType == XUI_EVENT_POINTER_DOWN ||
        event->iType == XUI_EVENT_TEXT ||
        event->iType == XUI_EVENT_IME_COMPOSITION ||
        event->iType == XUI_EVENT_BLUR)
        e->vertical_goal_active = 0;
    if (e->resizing_column && (event->iType == XUI_EVENT_KEY_DOWN || event->iType == XUI_EVENT_TEXT ||
        event->iType == XUI_EVENT_IME_COMPOSITION)) {
        doc_editor_cancel_resize(e, !e->view.replaying_input);
    }
    if (event->iType == XUI_EVENT_CONTEXT_MENU) result = doc_editor_context_menu(e, event);
    else if (event->iType == XUI_EVENT_KEY_DOWN) result = doc_editor_key(e, event);
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
                result = doc_editor_insert_text(e, utf8, bytes, DOC_EDITOR_TYPING);
            }
        } else {
            if (event->iTextSize <= 0 || event->iTextSize >= XUI_EVENT_TEXT_CAPACITY) return XUI_OK;
            result = doc_editor_insert_text(e, event->sText, (uint64_t)event->iTextSize, DOC_EDITOR_TYPING);
        }
    } else if (event->iType == XUI_EVENT_IME_COMPOSITION) result = doc_editor_composition(e, event);
    else {
        if (event->iType == XUI_EVENT_BLUR || event->iType == XUI_EVENT_POINTER_DOWN) { doc_editor_cancel_composition(e); doc_editor_break_group(e); }
        if (event->iType == XUI_EVENT_POINTER_DOWN) {
            e->has_pending_marks = e->pending_clear_format = 0;
            e->pending_set = e->pending_clear = 0; e->pending_style_fields = 0;
        }
        if (event->iType == XUI_EVENT_POINTER_DOWN || event->iType == XUI_EVENT_POINTER_MOVE ||
            event->iType == XUI_EVENT_POINTER_UP || event->iType == XUI_EVENT_POINTER_CAPTURE_LOST ||
            event->iType == XUI_EVENT_BLUR) {
            result = doc_editor_resize_event(e, event);
            if (result != XUI_ERROR_NOT_FOUND) return result;
            result = doc_editor_task_marker_event(e, event);
            if (result != XUI_ERROR_NOT_FOUND) return result;
        }
        e->blink = 0; e->caret_visible = 1; return doc_view_event(w, event, NULL);
    }
    return result;
}
static int doc_editor_event(xui_widget w, const xui_event_t* event, void* user)
{
    doc_editor_data* e = doc_editor_get(w); int result, ordered, queued = 0; uint64_t received = xrtClock(); (void)user;
    if (!e || !event) return XUI_ERROR_INVALID_ARGUMENT;
    ordered = doc_editor_ordered_event(event);
    if (e->events && event->iType == XUI_EVENT_POINTER_MOVE && !e->event_dragging) return XUI_OK;
    if (event->iType == XUI_EVENT_POINTER_WHEEL && e->view.desc.bAutoHeight && !(event->iModifiers & XUI_MOD_CTRL)) return XUI_OK;
    if ((e->events || e->draining || e->in_clipboard || e->committing || e->stream_open) && ordered) { queued = 1; result = doc_editor_enqueue(e, event, received); }
    else {
        result = doc_editor_handle_event(e, event, received);
        if (!xuiInternalWidgetIsValid(w)) return XUI_EVENT_DISPATCH_STOP;
        if (result == XUI_DOC_ERROR_BUSY && e->input && ordered) { queued = 1; result = doc_editor_enqueue(e, event, received); }
    }
    if (!xuiInternalWidgetIsValid(w)) return XUI_EVENT_DISPATCH_STOP;
    if (result < 0 && result != XUI_ERROR_NOT_FOUND && e->desc.onError) e->desc.onError(w, result, e->desc.pUser);
    if (result == XUI_ERROR_NOT_FOUND || !ordered) return XUI_OK;
    if (!queued && result >= 0 && event->iType != XUI_EVENT_KEY_DOWN && event->iType != XUI_EVENT_TEXT && event->iType != XUI_EVENT_IME_COMPOSITION) return result;
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
    doc_view_resolve_style(&e->view);
    if (e->composing && e->projection) {
        e->projection->desc.iTextColor = e->view.renderer->desc.iTextColor;
        e->projection->desc.iBorderColor = e->view.renderer->desc.iBorderColor;
        e->projection->desc.iCodeBackground = e->view.renderer->desc.iCodeBackground;
        e->projection->desc.iHighlightColor = e->view.renderer->desc.iHighlightColor;
        e->projection->desc.iLinkColor = e->view.renderer->desc.iLinkColor;
        e->projection->desc.iQuoteBorderColor = e->view.renderer->desc.iQuoteBorderColor;
        e->projection->desc.iRuleColor = e->view.renderer->desc.iRuleColor;
        e->projection->desc.iParagraphBackgroundColor = e->view.renderer->desc.iParagraphBackgroundColor;
        e->projection->desc.iTableBorderColor = e->view.renderer->desc.iTableBorderColor;
        e->projection->desc.iTableHeaderColor = e->view.renderer->desc.iTableHeaderColor;
        e->projection->desc.iTableCellColor = e->view.renderer->desc.iTableCellColor;
        e->projection->desc.iImagePlaceholderColor = e->view.renderer->desc.iImagePlaceholderColor;
        e->projection->desc.iImageBorderColor = e->view.renderer->desc.iImageBorderColor;
        e->projection->desc.iImageTextColor = e->view.renderer->desc.iImageTextColor;
        result = doc_view_paint_background(&e->view, draw, content);
        if (result != XUI_OK) return result;
        result = xuiDocumentRendererLayout(e->projection, fmax(1, content.fW), e->view.scroll_y, fmax(1, content.fH));
        if (result == XUI_OK) result = xuiDocumentRendererDraw(e->projection, draw, content.fX - e->view.scroll_x, content.fY - e->view.scroll_y, content, NULL, 0);
        if (result == XUI_OK) result = doc_view_paint_border(&e->view, draw, content);
    } else result = doc_view_render(w, draw, state, user);
    if (result != XUI_OK) return result;
    if (!e->desc.bReadOnly && (e->caret_visible || e->composing) && xuiGetFocusWidget(xuiWidgetGetContext(w)) == w) {
        caret = doc_editor_caret_rect(e);
        if (caret.fH && caret.fX >= content.fX && caret.fX < content.fX + content.fW) {
            int bottom = caret.fY + caret.fH; caret.fY = caret.fY < content.fY ? content.fY : caret.fY;
            if (bottom > content.fY + content.fH) bottom = content.fY + content.fH;
            caret.fH = bottom > caret.fY ? bottom - caret.fY : 0;
            if (caret.fH && e->view.renderer->proxy->drawRectFill) e->view.renderer->proxy->drawRectFill(e->view.renderer->proxy, draw, caret,
                doc_view_style_color(w, "document.caret.color", e->desc.iCaretColor));
        }
    }
    if (xuiGetFocusWidget(xuiWidgetGetContext(w)) == w &&
        e->view.renderer->proxy->drawRectStroke) {
        uint32_t focus_border = doc_view_style_color(w,
            "document.border.focus_color", 0);
        if (focus_border) return e->view.renderer->proxy->drawRectStroke(
            e->view.renderer->proxy, draw, content, 1, focus_border);
    }
    return XUI_OK;
}
static int doc_editor_update(xui_widget w, float dt, void* user)
{
    doc_editor_data* e = doc_editor_get(w); (void)user;
    if (!e) return XUI_ERROR_INVALID_ARGUMENT;
    if (e->events || (e->input && !e->composing)) {
        int result = doc_editor_sync(e);
        if (result == XUI_OK) result = xuiDocumentEditorFlush(w);
        if (!xuiInternalWidgetIsValid(w)) return XUI_OK;
        if (result != XUI_OK && result != XUI_DOC_ERROR_BUSY && result != e->input_error) {
            e->input_error = result;
            if (e->desc.onError) e->desc.onError(w, result, e->desc.pUser);
            if (!xuiInternalWidgetIsValid(w)) return XUI_OK;
        }
    }
    if (xuiGetFocusWidget(xuiWidgetGetContext(w)) == w && !e->desc.bReadOnly && !e->composing) {
        e->blink += dt; if (e->blink >= .5) { e->blink = fmod(e->blink, .5); e->caret_visible = !e->caret_visible; xuiWidgetInvalidate(w, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER); }
    }
    if (e->find_ui) {
        doc_find_ui_update(e->find_ui);
        if (!xuiInternalWidgetIsValid(w)) return XUI_OK;
    }
    return doc_view_update(w, dt, user);
}
static int doc_editor_init(xui_widget w, void* data, const void* create, void* user)
{
    doc_editor_data* e = data; const xui_doc_editor_desc_t* desc = create; int result; (void)user;
    if (!desc || desc->iSize != sizeof(*desc)) return XUI_ERROR_INVALID_ARGUMENT;
    e->desc = *desc; e->origin = atomic_fetch_add(&doc_editor_sequence, 1); e->caret_visible = 1;
    if (!e->desc.iAsyncSourceThresholdBytes) e->desc.iAsyncSourceThresholdBytes = 100 * 1024;
    if (!e->desc.iUndoGroupTimeoutMs) e->desc.iUndoGroupTimeoutMs = 1000;
    e->view.onChanged = doc_editor_changed;
    e->view.onProjectionChanged = doc_editor_projection_changed;
    e->view.onBeforeTransition = xuiDocumentEditorFlush;
    e->view.onBeforeSelection = doc_editor_before_selection;
    if (!e->desc.iCaretColor) e->desc.iCaretColor = XUI_COLOR_RGBA(30, 30, 30, 255);
    result = desc->iMode ? xuiDocumentViewSetMode(w, desc->iMode) : XUI_OK;
    if (result == XUI_OK) result = doc_editor_sync(e);
    if (result != XUI_OK) return result;
    xuiWidgetSetFocusable(w, 1); xuiWidgetSetTabStop(w, 1); xuiWidgetSetImeMode(w, desc->bReadOnly ? XUI_IME_DISABLED : XUI_IME_ENABLED);
    xuiWidgetSetImeCandidateRect(w, doc_editor_ime_rect, NULL);
    result = doc_view_register_edit(w, 1);
    if (result == XUI_OK) result = xuiWidgetSetEventCallback(w, doc_editor_event, NULL);
    if (result == XUI_OK) result = xuiWidgetSetCursorQueryCallback(w, doc_editor_resize_cursor, NULL);
    return result;
}
static void doc_editor_destroy(xui_widget w, void* data, void* user)
{
    doc_editor_data* e = data; size_t i; (void)w; (void)user;
    doc_find_ui_destroy(e->find_ui); e->find_ui = NULL;
    if (e->menu) {
        xui_widget popup = xuiMenuGetPopupWidget(e->menu);
        (void)xuiMenuClear(e->menu);
        if (popup) xuiWidgetDestroy(popup);
        else xuiWidgetDestroy(e->menu);
        e->menu = NULL;
    }
    for (i = 0; i < sizeof(e->menu_text) / sizeof(e->menu_text[0]); i++) {
        free(e->menu_text[i]); e->menu_text[i] = NULL;
    }
    e->view.onChanged = NULL;
    e->view.onProjectionChanged = NULL;
    e->view.onBeforeTransition = NULL;
    e->view.onBeforeSelection = NULL;
    e->stream_open = 0;
    doc_editor_clear_events(e);
    xuiDocumentRendererRelease(e->projection); e->projection = NULL;
    if (e->input) {
        if (e->view.document->pending == e->input->prepare) (void)xuiDocumentCancelPrepare(e->view.document);
        else xuiDocumentPrepareCancel(e->input->prepare);
    }
    /* No rendering can follow subtype teardown. Drop its projection reference
     * before transferring the final candidate reference to the worker. */
    xuiDocumentPrepareRelease(e->view.renderer->input); e->view.renderer->input = NULL;
    doc_editor_forget_input(e); doc_input_worker_destroy(e->worker); doc_editor_clear_bookmarks(e);
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

#endif
