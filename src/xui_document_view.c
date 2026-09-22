#include "xui_document_view_internal.h"
#include <math.h>

doc_view_data* doc_view_get(xui_widget widget)
{
    xui_widget_type type;
    if (!widget) return NULL;
    type = xuiWidgetFindType(xuiWidgetGetContext(widget), "document-view");
    return type && xuiWidgetIsType(widget, type) ? xuiWidgetGetTypeData(widget) : NULL;
}
static void doc_view_reset_selection(doc_view_data* v)
{
    xui_doc_position_t p = {0}; p.iSize = sizeof(p); p.iKind = XUI_DOC_POSITION_GAP;
    if (v->renderer && v->renderer->mode != XUI_DOC_VISUAL) p.iKind = XUI_DOC_POSITION_SOURCE;
    p.iDocumentId = xuiDocumentGetIdentity(v->document); p.iRevision = xuiDocumentGetRevision(v->document);
    p.iNodeId = DOC_ROOT; p.iAffinity = XUI_DOC_AFTER; v->selection.tAnchor = v->selection.tCaret = p;
}
int doc_view_sync(doc_view_data* v)
{
    xui_document_snapshot snapshot; int result;
    if (!v->needs_sync) return XUI_OK;
    result = xuiDocumentAcquireSnapshot(v->document, &snapshot);
    if (result != XUI_OK) return result;
    result = xuiDocumentRendererSetSnapshot(v->renderer, snapshot, NULL); xuiDocumentSnapshotRelease(snapshot);
    if (result == XUI_OK) v->needs_sync = 0;
    return result;
}
static void doc_view_changed(xui_document document, xui_document_change_set changes, void* user)
{
    doc_view_data* v = user; xui_document_snapshot snapshot = NULL; xui_doc_position_t p; int mapping;
    (void)document;
    if (xuiDocumentMapPosition(changes, &v->selection.tAnchor, &p, &mapping) == XUI_OK) v->selection.tAnchor = p;
    else doc_view_reset_selection(v);
    if (xuiDocumentMapPosition(changes, &v->selection.tCaret, &p, &mapping) == XUI_OK) v->selection.tCaret = p;
    else doc_view_reset_selection(v);
    v->error = xuiDocumentAcquireSnapshot(v->document, &snapshot);
    if (v->error == XUI_OK) v->error = xuiDocumentRendererSetSnapshot(v->renderer, snapshot, changes);
    xuiDocumentSnapshotRelease(snapshot); v->needs_sync = v->error != XUI_OK;
    doc_view_invalidate_edit(v); v->pending_edit_events |= 3;
    if (v->onChanged) v->onChanged(document, changes, v);
    xuiWidgetInvalidate(v->widget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API int xuiDocumentViewSetDocument(xui_widget widget, xui_document document)
{
    doc_view_data* v = doc_view_get(widget); xui_document_snapshot snapshot = NULL; uint64_t subscription; int result;
    if (!v || !document) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->document == document) return XUI_OK;
    result = xuiDocumentAcquireSnapshot(document, &snapshot);
    if (result != XUI_OK) return result;
    result = xuiDocumentSubscribe(document, doc_view_changed, v, &subscription);
    if (result == XUI_OK) {
        result = xuiDocumentRendererSetSnapshot(v->renderer, snapshot, NULL);
        if (result != XUI_OK) xuiDocumentUnsubscribe(document, subscription);
    }
    xuiDocumentSnapshotRelease(snapshot);
    if (result != XUI_OK) return result;
    xuiDocumentRetain(document);
    if (v->document) { xuiDocumentUnsubscribe(v->document, v->subscription); xuiDocumentRelease(v->document); }
    v->document = document; v->subscription = subscription; v->scroll_x = v->scroll_y = 0; v->needs_sync = 0;
    doc_view_invalidate_edit(v); v->pending_edit_events |= 3;
    doc_view_reset_selection(v);
    if (v->onChanged) v->onChanged(document, NULL, v);
    return xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API xui_document xuiDocumentViewGetDocument(xui_widget widget)
{
    doc_view_data* v = doc_view_get(widget); return v ? v->document : NULL;
}
int doc_view_layout(doc_view_data* v)
{
    xui_rect_t content = xuiWidgetGetContentRect(v->widget); int result = doc_view_sync(v);
    if (result == XUI_OK && v->renderer->mode == XUI_DOC_LIVE_MARKDOWN) {
        xui_doc_rect_t before, after; uint64_t start = v->renderer->live_start, end = v->renderer->live_end;
        int anchored = xuiDocumentRendererGetCaretRect(v->renderer, &v->selection.tCaret, &before) == XUI_OK &&
            before.y >= v->scroll_y && before.y < v->scroll_y + content.fH;
        result = xuiDocumentRendererSetActivePosition(v->renderer, &v->selection.tCaret);
        if (result == XUI_OK && anchored && (start != v->renderer->live_start || end != v->renderer->live_end)) {
            result = xuiDocumentRendererLayout(v->renderer, fmax(1, content.fW), v->scroll_y, fmax(1, content.fH));
            if (result == XUI_OK && xuiDocumentRendererGetCaretRect(v->renderer, &v->selection.tCaret, &after) == XUI_OK)
                v->scroll_y = fmax(0, v->scroll_y + after.y - before.y);
        }
    }
    return result == XUI_OK ? xuiDocumentRendererLayout(v->renderer, fmax(1, content.fW), v->scroll_y, fmax(1, content.fH)) : result;
}
static int doc_view_map_range(doc_view_data* v, uint32_t mode, xui_doc_range_t* range)
{
    unsigned i; int result, mapping;
    if (mode != XUI_DOC_VISUAL && v->renderer->snapshot->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    for (i = 0; i < 2; i++) {
        xui_doc_position_t* p = i ? &range->tCaret : &range->tAnchor;
        if (mode != XUI_DOC_VISUAL && p->iKind != XUI_DOC_POSITION_SOURCE) {
            uint64_t offset;
            result = xuiDocumentPositionToSource(v->renderer->snapshot, p, &offset, &mapping);
            if (result != XUI_OK) return result;
            p->iNodeId = DOC_ROOT; p->iKind = XUI_DOC_POSITION_SOURCE; p->iOffset = offset;
        } else if (mode == XUI_DOC_VISUAL && p->iKind == XUI_DOC_POSITION_SOURCE) {
            result = xuiDocumentSourceToPosition(v->renderer->snapshot, p->iOffset, p, &mapping);
            if (result != XUI_OK) return result;
        }
    }
    return XUI_OK;
}
XUI_API int xuiDocumentViewSetSelection(xui_widget widget, const xui_doc_range_t* range)
{
    doc_view_data* v = doc_view_get(widget); xui_doc_range_t mapped; int order, result;
    if (!v || !range) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    result = xuiDocumentSnapshotComparePositions(v->renderer->snapshot, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    mapped = *range; result = doc_view_map_range(v, v->renderer->mode, &mapped);
    if (result != XUI_OK) return result;
    v->selection = mapped;
    v->pending_edit_events |= 2;
    if (v->onProjectionChanged) v->onProjectionChanged(widget);
    return xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API int xuiDocumentViewGetSelection(xui_widget widget, xui_doc_range_t* range)
{
    doc_view_data* v = doc_view_get(widget); if (!v || !range) return XUI_ERROR_INVALID_ARGUMENT; *range = v->selection; return XUI_OK;
}
XUI_API int xuiDocumentViewSetScroll(xui_widget widget, double x, double y)
{
    doc_view_data* v = doc_view_get(widget); xui_doc_rect_t size; int exact, result; xui_rect_t content;
    if (!v || !isfinite(x) || !isfinite(y)) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_layout(v); if (result != XUI_OK) return result;
    content = xuiWidgetGetContentRect(widget); xuiDocumentRendererGetSize(v->renderer, &size, &exact);
    v->scroll_x = fmin(fmax(0, x), fmax(0, size.width - content.fW)); v->scroll_y = fmin(fmax(0, y), fmax(0, size.height - content.fH));
    return xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API int xuiDocumentViewGetScroll(xui_widget widget, double* x, double* y)
{
    doc_view_data* v = doc_view_get(widget); if (!v || !x || !y) return XUI_ERROR_INVALID_ARGUMENT; *x = v->scroll_x; *y = v->scroll_y; return XUI_OK;
}
XUI_API int xuiDocumentViewSetZoom(xui_widget widget, float zoom)
{
    doc_view_data* v = doc_view_get(widget); xui_document_renderer renderer = NULL; xui_doc_renderer_desc_t desc; int result;
    if (!v || !isfinite(zoom) || zoom < .1f || zoom > 10) return XUI_ERROR_INVALID_ARGUMENT;
    desc = v->renderer->desc; desc.fZoom = zoom;
    result = xuiDocumentRendererCreate(xuiWidgetGetContext(widget), &desc, &renderer);
    if (result == XUI_OK) result = xuiDocumentRendererSetMode(renderer, v->renderer->mode);
    if (result == XUI_OK) result = xuiDocumentRendererSetSnapshot(renderer, v->renderer->snapshot, NULL);
    if (result != XUI_OK) { xuiDocumentRendererRelease(renderer); return result; }
    xuiDocumentRendererRelease(v->renderer); v->renderer = renderer; v->desc.tRenderer = desc;
    if (v->onProjectionChanged) v->onProjectionChanged(widget);
    return xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API int xuiDocumentViewSetMode(xui_widget widget, uint32_t mode)
{
    doc_view_data* v = doc_view_get(widget); xui_doc_range_t mapped; int result;
    if (!v || (mode != XUI_DOC_VISUAL && mode != XUI_DOC_SOURCE_TEXT && mode != XUI_DOC_LIVE_MARKDOWN)) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->renderer->mode == mode) return XUI_OK;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    mapped = v->selection;
    result = doc_view_map_range(v, mode, &mapped); if (result != XUI_OK) return result;
    result = xuiDocumentRendererSetMode(v->renderer, mode);
    if (result != XUI_OK) return result;
    v->selection = mapped; v->scroll_x = v->scroll_y = 0;
    doc_view_invalidate_edit(v); v->pending_edit_events |= 3;
    if (v->onProjectionChanged) v->onProjectionChanged(widget);
    return xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API uint32_t xuiDocumentViewGetMode(xui_widget widget)
{
    doc_view_data* v = doc_view_get(widget); return v ? v->renderer->mode : 0;
}
XUI_API int xuiDocumentViewGetContentSize(xui_widget widget, xui_doc_rect_t* size, int* exact)
{
    doc_view_data* v = doc_view_get(widget); int result;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_layout(v); return result == XUI_OK ? xuiDocumentRendererGetSize(v->renderer, size, exact) : result;
}
XUI_API int xuiDocumentViewHitTest(xui_widget widget, double x, double y, xui_doc_position_t* position)
{
    doc_view_data* v = doc_view_get(widget); xui_rect_t content; int result;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_layout(v); if (result != XUI_OK) return result;
    content = xuiWidgetGetContentRect(widget);
    return xuiDocumentRendererHitTest(v->renderer, x - content.fX + v->scroll_x, y - content.fY + v->scroll_y, position);
}
XUI_API int xuiDocumentViewFind(xui_widget widget, const char* pattern, uint64_t bytes, int backward, int wrap, xui_doc_range_t* match)
{
    doc_view_data* v = doc_view_get(widget); xui_doc_range_t* matches; uint64_t count, i, chosen = DOC_NONE; unsigned domain; int result, order;
    xui_doc_position_t boundary; xui_doc_rect_t caret;
    if (!v || !match) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    domain = v->renderer->mode != XUI_DOC_VISUAL ? XUI_DOC_SOURCE : XUI_DOC_SEMANTIC;
    result = xuiDocumentSnapshotFind(v->renderer->snapshot, domain, pattern, bytes, NULL, NULL, 0, &count);
    if (result != XUI_OK || !count) return result != XUI_OK ? result : XUI_ERROR_NOT_FOUND;
    if (count > SIZE_MAX / sizeof(*matches)) return XUI_DOC_ERROR_LIMIT;
    matches = malloc((size_t)count * sizeof(*matches)); if (!matches) return XUI_ERROR_OUT_OF_MEMORY;
    result = xuiDocumentSnapshotFind(v->renderer->snapshot, domain, pattern, bytes, NULL, matches, count, &count);
    if (result != XUI_OK) { free(matches); return result; }
    doc_position_compare(v->renderer->snapshot->state, &v->selection.tAnchor, &v->selection.tCaret, &order);
    boundary = (backward ? order > 0 : order < 0) ? v->selection.tCaret : v->selection.tAnchor;
    for (i = 0; i < count; i++) {
        uint64_t index = backward ? count - 1 - i : i;
        doc_position_compare(v->renderer->snapshot->state, backward ? &matches[index].tCaret : &matches[index].tAnchor, &boundary, &order);
        if (backward ? order <= 0 : order >= 0) { chosen = index; break; }
    }
    if (chosen == DOC_NONE && wrap) chosen = backward ? count - 1 : 0;
    if (chosen == DOC_NONE) { free(matches); return XUI_ERROR_NOT_FOUND; }
    *match = matches[chosen]; free(matches); result = xuiDocumentViewSetSelection(widget, match);
    if (result == XUI_OK && doc_view_layout(v) == XUI_OK && xuiDocumentRendererGetCaretRect(v->renderer, &match->tAnchor, &caret) == XUI_OK)
        result = xuiDocumentViewSetScroll(widget, v->scroll_x, caret.y);
    return result;
}
int doc_view_measure(xui_widget widget, xui_vec2_t constraint, xui_vec2_t* size, void* user)
{
    doc_view_data* v = doc_view_get(widget); xui_doc_rect_t bounds; int exact, result;
    double width = constraint.fX > 0 && constraint.fX < 1000000 ? constraint.fX : 480;
    (void)user;
    if (!v || !size) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    if (v->renderer->mode == XUI_DOC_LIVE_MARKDOWN) {
        result = xuiDocumentRendererSetActivePosition(v->renderer, &v->selection.tCaret);
        if (result != XUI_OK) return result;
    }
    result = xuiDocumentRendererLayout(v->renderer, width, 0, v->desc.bAutoHeight ? 1e30 : 240);
    if (result != XUI_OK) return result;
    xuiDocumentRendererGetSize(v->renderer, &bounds, &exact);
    if (v->desc.bAutoHeight && bounds.height > 1000000000) return XUI_DOC_ERROR_LIMIT;
    size->fX = (float)width; size->fY = v->desc.bAutoHeight ? (float)fmax(20, bounds.height) : 240;
    return XUI_OK;
}
int doc_view_render(xui_widget widget, xui_draw_context draw, uint32_t state, void* user)
{
    doc_view_data* v = doc_view_get(widget); xui_rect_t content; int result;
    (void)state; (void)user;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_layout(v); if (result != XUI_OK) return result;
    content = xuiWidgetGetContentRect(widget);
    return xuiDocumentRendererDraw(v->renderer, draw, content.fX - v->scroll_x, content.fY - v->scroll_y, content,
        v->desc.bDisableSelection ? NULL : &v->selection, v->desc.iSelectionColor);
}
int doc_view_event(xui_widget widget, const xui_event_t* event, void* user)
{
    doc_view_data* v = doc_view_get(widget); xui_rect_t world; xui_doc_position_t hit; int result;
    (void)user;
    if (!v || !event) return XUI_ERROR_INVALID_ARGUMENT;
    world = xuiWidgetGetWorldRect(widget);
    if (event->iType == XUI_EVENT_KEY_DOWN && event->iModifiers & XUI_MOD_CTRL && !v->desc.bDisableSelection) {
        if (event->iKey == 'C' || event->iKey == 'c') { xuiEditCopy(widget); return XUI_EVENT_DISPATCH_STOP; }
        if (event->iKey == 'A' || event->iKey == 'a') { xuiEditSelectAll(widget); return XUI_EVENT_DISPATCH_STOP; }
    }
    if (event->iType == XUI_EVENT_POINTER_WHEEL) {
        if (event->iModifiers & XUI_MOD_CTRL) xuiDocumentViewSetZoom(widget, fminf(10, fmaxf(.1f, v->renderer->desc.fZoom * (event->fWheelY > 0 ? 1.1f : 1 / 1.1f))));
        else if (v->desc.bAutoHeight) return XUI_OK; /* Let the containing scroll host handle it. */
        else xuiDocumentViewSetScroll(widget, v->scroll_x - event->fWheelX * 48, v->scroll_y - event->fWheelY * 48);
        return XUI_EVENT_DISPATCH_STOP;
    }
    if (event->iType == XUI_EVENT_BLUR || event->iType == XUI_EVENT_POINTER_CAPTURE_LOST) { v->dragging = 0; v->pressed_node = 0; return XUI_OK; }
    if (event->iType == XUI_EVENT_POINTER_DOWN && (event->iButton == XUI_POINTER_BUTTON_LEFT || !event->iButton)) {
        result = xuiDocumentViewHitTest(widget, event->fX - world.fX, event->fY - world.fY, &hit);
        if (result != XUI_OK) return result;
        xuiSetFocusWidget(xuiWidgetGetContext(widget), widget); v->pressed_node = hit.iNodeId;
        if (!v->desc.bDisableSelection) {
            if (!(event->iModifiers & XUI_MOD_SHIFT)) v->selection.tAnchor = hit;
            v->selection.tCaret = hit; v->dragging = 1;
            v->pending_edit_events |= 2;
            xuiSetPointerCaptureEx(xuiWidgetGetContext(widget), event->iPointerId, event->iPointerType, widget);
        }
        xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER |
            (v->renderer->mode == XUI_DOC_LIVE_MARKDOWN ? XUI_WIDGET_DIRTY_LAYOUT : 0)); return XUI_EVENT_DISPATCH_STOP;
    }
    if (event->iType == XUI_EVENT_POINTER_MOVE && v->dragging) {
        result = xuiDocumentViewHitTest(widget, event->fX - world.fX, event->fY - world.fY, &hit);
        if (result == XUI_OK) { v->selection.tCaret = hit; v->pending_edit_events |= 2; }
        v->pressed_node = 0; xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER |
            (v->renderer->mode == XUI_DOC_LIVE_MARKDOWN ? XUI_WIDGET_DIRTY_LAYOUT : 0)); return XUI_EVENT_DISPATCH_STOP;
    }
    if (event->iType == XUI_EVENT_POINTER_UP) {
        uint64_t pressed = v->pressed_node;
        v->dragging = 0; v->pressed_node = 0;
        xuiReleasePointerCaptureEx(xuiWidgetGetContext(widget), event->iPointerId, event->iPointerType, widget);
        if (pressed && v->desc.onActivate) {
            xui_doc_node_info_t info = {0}; info.iSize = sizeof(info);
            result = xuiDocumentViewHitTest(widget, event->fX - world.fX, event->fY - world.fY, &hit);
            if (result == XUI_OK && hit.iNodeId == pressed && xuiDocumentSnapshotGetNode(v->renderer->snapshot, pressed, &info) == XUI_OK) {
                xui_document_snapshot retained = v->renderer->snapshot;
                xuiDocumentSnapshotRetain(retained);
                v->desc.onActivate(widget, pressed, info.sResource, v->desc.pUser);
                xuiDocumentSnapshotRelease(retained);
            }
        }
        return XUI_EVENT_DISPATCH_STOP;
    }
    return XUI_OK;
}
int doc_view_init(xui_widget widget, void* data, const void* create, void* user)
{
    doc_view_data* v = data; const xui_doc_view_desc_t* desc = create; int result;
    (void)user;
    if (!desc || desc->iSize != sizeof(*desc) || !desc->pDocument) return XUI_ERROR_INVALID_ARGUMENT;
    v->widget = widget; v->desc = *desc;
    if (!v->desc.iSelectionColor) v->desc.iSelectionColor = XUI_COLOR_RGBA(80, 140, 230, 100);
    result = xuiDocumentRendererCreate(xuiWidgetGetContext(widget), desc->tRenderer.iSize ? &desc->tRenderer : NULL, &v->renderer);
    if (result != XUI_OK) return result;
    result = xuiDocumentViewSetDocument(widget, desc->pDocument);
    if (result != XUI_OK) { xuiDocumentRendererRelease(v->renderer); v->renderer = NULL; return result; }
    xuiWidgetSetFocusable(widget, !desc->bDisableSelection); xuiWidgetSetTabStop(widget, !desc->bDisableSelection);
    result = doc_view_register_edit(widget, 0);
    if (result == XUI_OK) result = xuiWidgetSetEventCallback(widget, doc_view_event, NULL);
    if (result != XUI_OK) doc_view_destroy(widget, data, user);
    return result;
}
void doc_view_destroy(xui_widget widget, void* data, void* user)
{
    doc_view_data* v = data; (void)widget; (void)user;
    if (v->document) { xuiDocumentUnsubscribe(v->document, v->subscription); xuiDocumentRelease(v->document); }
    doc_view_invalidate_edit(v);
    xuiDocumentRendererRelease(v->renderer); memset(v, 0, sizeof(*v));
}
XUI_API xui_widget_type xuiDocumentViewGetType(xui_context context)
{
    xui_widget_type type = xuiWidgetFindType(context, "document-view"); xui_widget_type_desc_t desc = {0};
    if (type) return type;
    desc.iSize = sizeof(desc); desc.sName = "document-view"; desc.pParent = xuiWidgetGetBaseType(); desc.iTypeDataSize = sizeof(doc_view_data);
    desc.onInit = doc_view_init; desc.onDestroy = doc_view_destroy; desc.onContentMeasure = doc_view_measure; desc.onCacheRender = doc_view_render;
    desc.onUpdate = doc_view_update;
    desc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
    desc.tLayout.iLayoutType = XUI_LAYOUT_MANUAL; desc.tLayout.iWidthMode = XUI_SIZE_FILL; desc.tLayout.iHeightMode = XUI_SIZE_CONTENT;
    desc.tLayout.iFlowMode = XUI_FLOW_BLOCK; desc.tLayout.iOverflow = XUI_OVERFLOW_HIDDEN;
    desc.tLayout.fMaxWidth = desc.tLayout.fMaxHeight = XUI_LAYOUT_UNBOUNDED; desc.tLayout.fShrink = 1;
    desc.tLayout.iTableRowSpan = desc.tLayout.iTableColumnSpan = desc.tLayout.iGridColumnCount = 1;
    desc.tCachePolicy.iSize = sizeof(desc.tCachePolicy); desc.tCachePolicy.iPolicy = XUI_CACHE_POLICY_SELF; desc.tCachePolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
    return xuiWidgetRegisterType(context, &type, &desc) == XUI_OK ? type : NULL;
}
XUI_API int xuiDocumentViewCreate(xui_context context, const xui_doc_view_desc_t* desc, xui_widget* out)
{
    xui_widget_type type;
    if (out) *out = NULL;
    if (!out || !desc || desc->iSize != sizeof(*desc) || !desc->pDocument) return XUI_ERROR_INVALID_ARGUMENT;
    type = xuiDocumentViewGetType(context); return type ? xuiWidgetCreateTyped(context, type, out, desc) : XUI_ERROR_OUT_OF_MEMORY;
}
