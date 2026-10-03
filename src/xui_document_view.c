#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT_VIEW
#include "xui_document_view_internal.h"
#include "xui_document_accessible_internal.h"
#include "xui_internal.h"
#include <math.h>

uint32_t doc_view_style_color(xui_widget widget, const char* name, uint32_t fallback)
{
    xui_style_property_t property = {0};
    property.iSize = sizeof(property);
    return xuiWidgetGetResolvedStyleProperty(widget, name, &property) == XUI_OK &&
        property.tValue.iType == XUI_STYLE_VALUE_COLOR ? property.tValue.iColor : fallback;
}
void doc_view_resolve_style(doc_view_data* v)
{
    xui_doc_renderer_desc_t* colors = &v->renderer->desc;
    colors->iTextColor = doc_view_style_color(v->widget, "document.text.color", v->base_renderer.iTextColor);
    colors->iBorderColor = doc_view_style_color(v->widget, "document.border.color", v->base_renderer.iBorderColor);
    colors->iCodeBackground = doc_view_style_color(v->widget, "document.code.background_color", v->base_renderer.iCodeBackground);
    colors->iHighlightColor = doc_view_style_color(v->widget, "document.highlight.color", v->base_renderer.iHighlightColor);
    colors->iLinkColor = doc_view_style_color(v->widget, "document.link.color", v->base_renderer.iLinkColor);
    colors->iQuoteBorderColor = doc_view_style_color(v->widget, "document.quote.border_color", v->base_renderer.iQuoteBorderColor);
    colors->iRuleColor = doc_view_style_color(v->widget, "document.rule.color", v->base_renderer.iRuleColor);
    colors->iParagraphBackgroundColor = doc_view_style_color(v->widget, "document.paragraph.background_color", v->base_renderer.iParagraphBackgroundColor);
    colors->iTableBorderColor = doc_view_style_color(v->widget, "document.table.border_color", v->base_renderer.iTableBorderColor);
    colors->iTableHeaderColor = doc_view_style_color(v->widget, "document.table.header_color", v->base_renderer.iTableHeaderColor);
    colors->iTableCellColor = doc_view_style_color(v->widget, "document.table.cell_color", v->base_renderer.iTableCellColor);
    colors->iImagePlaceholderColor = doc_view_style_color(v->widget, "document.image.placeholder_color", v->base_renderer.iImagePlaceholderColor);
    colors->iImageBorderColor = doc_view_style_color(v->widget, "document.image.border_color", v->base_renderer.iImageBorderColor);
    colors->iImageTextColor = doc_view_style_color(v->widget, "document.image.text_color", v->base_renderer.iImageTextColor);
}
int doc_view_paint_background(doc_view_data* v, xui_draw_context draw, xui_rect_t content)
{
    uint32_t color = doc_view_style_color(v->widget, "document.background.color", v->desc.iBackgroundColor);
    return color && v->renderer->proxy->drawRectFill ?
        v->renderer->proxy->drawRectFill(v->renderer->proxy, draw, content, color) : XUI_OK;
}
int doc_view_paint_border(doc_view_data* v, xui_draw_context draw, xui_rect_t content)
{
    uint32_t color = v->renderer->desc.iBorderColor;
    return color && v->renderer->proxy->drawRectStroke ?
        v->renderer->proxy->drawRectStroke(v->renderer->proxy, draw,
            content, 1, color) : XUI_OK;
}
static void doc_view_register_styles(xui_context context, xui_widget_type type)
{
    static const char* const names[] = {
        "document.text.color", "document.background.color", "document.border.color",
        "document.code.background_color", "document.highlight.color", "document.link.color",
        "document.selection.color", "document.caret.color",
        "document.find.result_color", "document.find.active_color",
        "document.border.focus_color", "document.quote.border_color",
        "document.rule.color", "document.paragraph.background_color",
        "document.table.border_color", "document.table.header_color",
        "document.table.cell_color", "document.image.placeholder_color",
        "document.image.border_color", "document.image.text_color"
    };
    size_t i;
    for (i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        xui_style_property_info_t info = {0};
        if (xuiStyleFindProperty(context, names[i])) continue;
        info.iSize = sizeof(info); info.sName = names[i]; info.pWidgetType = type;
        info.iValueType = XUI_STYLE_VALUE_COLOR;
        info.iDirtyFlags = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
        (void)xuiStyleRegisterProperty(context, &info, NULL);
    }
}

doc_view_data* doc_view_get(xui_widget widget)
{
    xui_widget_type type;
    if (!widget) return NULL;
    type = xuiWidgetFindType(xuiWidgetGetContext(widget), "document-view");
    return type && xuiWidgetIsType(widget, type) ? xuiWidgetGetTypeData(widget) : NULL;
}
void doc_view_find_invalidate(doc_view_data* v)
{
    free(v->find_matches); v->find_matches = NULL;
    v->find_count = 0; v->find_active = DOC_NONE; v->find_valid = 0;
}
void doc_view_find_query_clear(doc_view_data* v)
{
    doc_view_find_invalidate(v);
    free(v->find_pattern); v->find_pattern = NULL;
    v->find_pattern_bytes = 0; v->find_flags = 0;
}
static void doc_view_find_clear(doc_view_data* v)
{
    doc_view_find_query_clear(v);
    v->find_scope_active = 0;
    memset(&v->find_scope, 0, sizeof(v->find_scope));
}
static int doc_view_find_collect(doc_view_data* v, const char* pattern,
    uint64_t bytes, uint32_t flags,
    xui_doc_range_t** matches, uint64_t* count)
{
    uint32_t domain = v->renderer->mode == XUI_DOC_VISUAL ? XUI_DOC_SEMANTIC : XUI_DOC_SOURCE;
    int result;
    *matches = NULL; *count = 0;
    result = xuiDocumentSnapshotFindEx(v->renderer->snapshot, domain,
        pattern, bytes, flags, v->find_scope_active ? &v->find_scope : NULL,
        NULL, 0, count);
    if (result != XUI_OK || !*count) return result;
    if (*count > SIZE_MAX / sizeof(**matches)) return XUI_DOC_ERROR_LIMIT;
    *matches = malloc((size_t)*count * sizeof(**matches));
    if (!*matches) return XUI_ERROR_OUT_OF_MEMORY;
    result = xuiDocumentSnapshotFindEx(v->renderer->snapshot, domain,
        pattern, bytes, flags, v->find_scope_active ? &v->find_scope : NULL,
        *matches, *count, count);
    if (result != XUI_OK) { free(*matches); *matches = NULL; *count = 0; }
    return result;
}
static int doc_view_find_range_equal(const xui_doc_range_t* a, const xui_doc_range_t* b)
{
    return a->tAnchor.iKind == b->tAnchor.iKind && a->tAnchor.iNodeId == b->tAnchor.iNodeId &&
        a->tAnchor.iOffset == b->tAnchor.iOffset && a->tCaret.iKind == b->tCaret.iKind &&
        a->tCaret.iNodeId == b->tCaret.iNodeId && a->tCaret.iOffset == b->tCaret.iOffset;
}
static void doc_view_find_store(doc_view_data* v, xui_doc_range_t* matches, uint64_t count)
{
    uint64_t i;
    doc_view_find_invalidate(v);
    v->find_matches = matches; v->find_count = count;
    v->find_identity = v->renderer->snapshot->identity;
    v->find_revision = v->renderer->snapshot->revision;
    v->find_mode = v->renderer->mode; v->find_valid = 1;
    for (i = 0; i < count; i++) if (doc_view_find_range_equal(&matches[i], &v->selection)) {
        v->find_active = i; break;
    }
}
static int doc_view_find_refresh(doc_view_data* v)
{
    xui_doc_range_t* matches; uint64_t count; int result;
    if (!v->find_pattern) return XUI_OK;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    if (v->find_valid && v->find_identity == v->renderer->snapshot->identity &&
        v->find_revision == v->renderer->snapshot->revision && v->find_mode == v->renderer->mode) return XUI_OK;
    result = doc_view_find_collect(v, v->find_pattern,
        v->find_pattern_bytes, v->find_flags, &matches, &count);
    if (result == XUI_OK) doc_view_find_store(v, matches, count);
    return result;
}
static void doc_view_reset_selection(doc_view_data* v)
{
    xui_doc_position_t p = {0}; p.iSize = sizeof(p); p.iKind = XUI_DOC_POSITION_GAP;
    memset(&v->table_selection, 0, sizeof(v->table_selection));
    v->table_selecting = 0;
    if (v->renderer && v->renderer->mode != XUI_DOC_VISUAL) p.iKind = XUI_DOC_POSITION_SOURCE;
    p.iDocumentId = xuiDocumentGetIdentity(v->document); p.iRevision = xuiDocumentGetRevision(v->document);
    p.iNodeId = DOC_ROOT; p.iAffinity = XUI_DOC_AFTER; v->selection.tAnchor = v->selection.tCaret = p;
}
int doc_view_sync(doc_view_data* v)
{
    xui_document_snapshot snapshot; int result;
    if (!v->needs_sync) return v->input != v->renderer->input ? xuiDocumentRendererSetSourceInput(v->renderer, v->input) : XUI_OK;
    result = xuiDocumentAcquireSnapshot(v->document, &snapshot);
    if (result != XUI_OK) return result;
    result = xuiDocumentRendererSetSnapshot(v->renderer, snapshot, NULL); xuiDocumentSnapshotRelease(snapshot);
    if (result == XUI_OK) { v->needs_sync = 0; if (v->input) result = xuiDocumentRendererSetSourceInput(v->renderer, v->input); }
    return result;
}
static int doc_view_rebuild_renderer(doc_view_data* v, float zoom, int prepare_old_layout);
int doc_view_refresh_resources(doc_view_data* v)
{
    int result;
    xui_context context = xuiWidgetGetContext(v->widget);
    if (v->renderer->dpi_scale != xuiGetVirtualDpi(context) ||
        (v->uses_default_font && v->renderer->desc.tFonts.normal != xuiGetDefaultFont(context))) {
        result = doc_view_rebuild_renderer(v, v->renderer->desc.fZoom, 0);
        if (result != XUI_OK) return result;
    }
    uint64_t generation = xuiResourceGetRegistryGeneration(context);
    if (generation != v->renderer->resource_registry_generation && !v->renderer->has_named_images) {
        v->renderer->resource_registry_generation = generation;
        return XUI_OK;
    }
    return generation != v->renderer->resource_registry_generation ?
        xuiDocumentViewInvalidateObjects(v->widget) : XUI_OK;
}
static void doc_view_changed(xui_document document, xui_document_change_set changes, void* user)
{
    doc_view_data* v = user; xui_document_snapshot snapshot = NULL; xui_doc_position_t p; int mapping;
    xui_doc_position_t visible = {0}, mapped_visible = {0};
    xui_doc_rect_t old_rect = {0}, new_rect = {0}, size = {0};
    xui_rect_t content = xuiWidgetGetContentRect(v->widget);
    double screen_y = fmin(20, content.fH / 4), fraction = 0, next_y;
    size_t anchor_block = 0, i;
    int anchored = 0, exact;
    (void)document;
    if (changes && (changes->flags & XUI_DOC_CHANGE_STYLE) &&
        !(changes->flags & (XUI_DOC_CHANGE_SOURCE | XUI_DOC_CHANGE_RESET)) &&
        (!(changes->flags & XUI_DOC_CHANGE_STRUCTURE) ||
            doc_render_inline_style_splits(changes)) &&
        doc_render_metric_style_change(changes) &&
        !v->desc.bAutoHeight && v->scroll_y > 0 && content.fW > 0 && content.fH > 0 &&
        v->renderer->mode == XUI_DOC_VISUAL && v->renderer->count &&
        fabs(v->renderer->width - fmax(1, content.fW)) < .001 &&
        xuiDocumentRendererHitTest(v->renderer,
            v->scroll_x + fmin(45, content.fW / 3), v->scroll_y + screen_y,
            &visible) == XUI_OK &&
        xuiDocumentRendererGetCaretRect(v->renderer, &visible, &old_rect) == XUI_OK &&
        xuiDocumentMapPosition(changes, &visible, &mapped_visible, &mapping) == XUI_OK &&
        mapping != XUI_DOC_MAP_DELETED) {
        anchored = 1;
        fraction = fmin(.99, fmax(0,
            (v->scroll_y + screen_y - old_rect.y) / fmax(1, old_rect.height)));
        anchor_block = doc_render_block_at(v->renderer, v->scroll_y + screen_y);
    }
    doc_view_find_invalidate(v);
    if (v->find_scope_active) {
        xui_doc_position_t anchor, caret;
        if (xuiDocumentMapPosition(changes, &v->find_scope.tAnchor,
                &anchor, &mapping) == XUI_OK &&
            xuiDocumentMapPosition(changes, &v->find_scope.tCaret,
                &caret, &mapping) == XUI_OK) {
            v->find_scope.tAnchor = anchor; v->find_scope.tCaret = caret;
        } else doc_view_find_clear(v);
    }
    doc_view_accessible_clear(v);
    memset(&v->table_selection, 0, sizeof(v->table_selection));
    v->table_selecting = 0;
    if (xuiDocumentMapPosition(changes, &v->selection.tAnchor, &p, &mapping) == XUI_OK) v->selection.tAnchor = p;
    else doc_view_reset_selection(v);
    if (xuiDocumentMapPosition(changes, &v->selection.tCaret, &p, &mapping) == XUI_OK) v->selection.tCaret = p;
    else doc_view_reset_selection(v);
    v->error = xuiDocumentAcquireSnapshot(v->document, &snapshot);
    if (v->error == XUI_OK) v->error = xuiDocumentRendererSetSnapshot(v->renderer, snapshot, changes);
    xuiDocumentSnapshotRelease(snapshot); v->needs_sync = v->error != XUI_OK;
    if (v->error == XUI_OK && anchored) {
        /* A style change above the viewport can change its block height.
         * SetSnapshot evicts that block's layout; settle changed predecessors
         * before resolving the mapped visible caret. */
        for (i = 0; i < changes->count; i++) {
            size_t block = doc_render_find(v->renderer, changes->ops[i].iNodeId);
            if (block != SIZE_MAX && block <= anchor_block &&
                doc_render_materialize(v->renderer, block) != XUI_OK) {
                anchored = 0; break;
            }
        }
        if (anchored && xuiDocumentRendererLayout(v->renderer,
                fmax(1, content.fW), v->scroll_y, fmax(1, content.fH)) == XUI_OK &&
            xuiDocumentRendererGetCaretRect(v->renderer, &mapped_visible, &new_rect) == XUI_OK) {
            next_y = fmax(0, new_rect.y + fraction * new_rect.height - screen_y);
            if (xuiDocumentRendererLayout(v->renderer, fmax(1, content.fW),
                    next_y, fmax(1, content.fH)) == XUI_OK &&
                xuiDocumentRendererGetSize(v->renderer, &size, &exact) == XUI_OK)
                v->scroll_y = fmin(next_y, fmax(0, size.height - content.fH));
        }
    }
    doc_view_invalidate_edit(v); v->pending_edit_events |= 3;
    if (v->onChanged) v->onChanged(document, changes, v);
    xuiInternalAccessibilityQueue(v->widget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
    xuiWidgetInvalidate(v->widget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API int xuiDocumentViewSetDocument(xui_widget widget, xui_document document)
{
    doc_view_data* v = doc_view_get(widget); xui_document_snapshot snapshot = NULL; uint64_t subscription; int result;
    if (!v || !document) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->document == document) return XUI_OK;
    if (v->onBeforeTransition && (result = v->onBeforeTransition(widget)) != XUI_OK) return result;
    if (!xuiInternalWidgetIsValid(widget)) return XUI_ERROR_INVALID_STATE;
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
    doc_view_find_clear(v);
    doc_view_accessible_clear(v);
    if (v->document) { xuiDocumentUnsubscribe(v->document, v->subscription); xuiDocumentRelease(v->document); }
    v->document = document; v->subscription = subscription; v->scroll_x = v->scroll_y = 0; v->needs_sync = 0;
    doc_view_invalidate_edit(v); v->pending_edit_events |= 3;
    doc_view_reset_selection(v);
    if (v->onChanged) v->onChanged(document, NULL, v);
    xuiInternalAccessibilityQueue(widget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
    return xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API xui_document xuiDocumentViewGetDocument(xui_widget widget)
{
    doc_view_data* v = doc_view_get(widget); return v ? v->document : NULL;
}
int doc_view_layout(doc_view_data* v)
{
    xui_rect_t content = xuiWidgetGetContentRect(v->widget); int result = doc_view_refresh_resources(v);
    doc_view_resolve_style(v);
    if (result == XUI_OK) result = doc_view_sync(v);
    if (result == XUI_OK && v->renderer->mode == XUI_DOC_LIVE_MARKDOWN) {
        xui_doc_rect_t before, after; uint64_t start = v->renderer->live_start, end = v->renderer->live_end;
        /* SetActivePosition preserves the projection for a source caret in
         * the current active block. No anchor is needed in that case; asking
         * for a far caret would eagerly shape an ASCII horizontal suffix. */
        int same_active = v->selection.tCaret.iKind == XUI_DOC_POSITION_SOURCE &&
            v->selection.tCaret.iOffset >= start &&
            (v->selection.tCaret.iOffset < end ||
             (v->selection.tCaret.iOffset == end && end == doc_seq_size(doc_render_source(v->renderer))));
        int anchored = !same_active &&
            xuiDocumentRendererGetCaretRect(v->renderer, &v->selection.tCaret, &before) == XUI_OK &&
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
            result = v->renderer->input && v->renderer->mode == XUI_DOC_LIVE_MARKDOWN ?
                doc_render_live_position_source(v->renderer, p, &offset) :
                xuiDocumentPositionToSource(v->renderer->snapshot, p, &offset, &mapping);
            if (result != XUI_OK) return result;
            if (v->renderer->input && v->renderer->mode == XUI_DOC_LIVE_MARKDOWN)
                result = xuiDocumentPrepareSourcePosition(v->renderer->input, offset, p->iAffinity, p);
            else { p->iNodeId = DOC_ROOT; p->iKind = XUI_DOC_POSITION_SOURCE; p->iOffset = offset; }
            if (result != XUI_OK) return result;
        } else if (mode == XUI_DOC_VISUAL && p->iKind == XUI_DOC_POSITION_SOURCE) {
            result = xuiDocumentSourceToPosition(v->renderer->snapshot, p->iOffset, p, &mapping);
            if (result != XUI_OK) return result;
        }
    }
    return XUI_OK;
}
int doc_view_set_selection(doc_view_data* v, const xui_doc_range_t* range)
{
    xui_widget widget = v->widget; xui_doc_range_t mapped; int order, result;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    result = doc_render_compare(v->renderer, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    mapped = *range; result = doc_view_map_range(v, v->renderer->mode, &mapped);
    if (result != XUI_OK) return result;
    v->selection = mapped;
    memset(&v->table_selection, 0, sizeof(v->table_selection));
    v->table_selecting = 0;
    v->pending_edit_events |= 2;
    if (v->onProjectionChanged) v->onProjectionChanged(widget);
    return xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API int xuiDocumentViewSetSelection(xui_widget widget, const xui_doc_range_t* range)
{
    doc_view_data* v = doc_view_get(widget); int result;
    if (!v || !range) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->onBeforeSelection && (result = v->onBeforeSelection(widget)) != XUI_OK) return result;
    return doc_view_set_selection(v, range);
}
XUI_API int xuiDocumentViewGetSelection(xui_widget widget, xui_doc_range_t* range)
{
    doc_view_data* v = doc_view_get(widget); if (!v || !range) return XUI_ERROR_INVALID_ARGUMENT; *range = v->selection; return XUI_OK;
}
int doc_view_object_range(doc_view_data* v, uint64_t id, xui_doc_range_t* range)
{
    xui_document_snapshot snapshot = v->renderer->snapshot;
    doc_node* object = doc_index_get(snapshot->state->index, id);
    if (!object || !doc_selectable_object_kind(object->kind))
        return XUI_ERROR_NOT_FOUND;
    return doc_accessible_snapshot_object_range(snapshot, object, range);
}
XUI_API int xuiDocumentViewSelectObject(xui_widget widget, xui_doc_node_id node)
{
    doc_view_data* v = doc_view_get(widget); xui_doc_range_t range; int result;
    if (!v || !node) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->desc.bDisableSelection || v->renderer->mode != XUI_DOC_VISUAL)
        return XUI_ERROR_UNSUPPORTED;
    if (v->onBeforeSelection && (result = v->onBeforeSelection(widget)) != XUI_OK)
        return result;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    result = doc_view_object_range(v, node, &range);
    return result == XUI_OK ? doc_view_set_selection(v, &range) : result;
}
static xui_doc_cell_hit_t doc_view_table_slot_hit(const doc_table_cell_slot* slot)
{
    xui_doc_cell_hit_t hit = {0};
    hit.iSize = sizeof(hit); hit.iTableId = slot->table; hit.iCellId = slot->cell;
    hit.iRow = slot->row; hit.iColumn = slot->column;
    hit.iRowSpan = slot->row_span; hit.iColumnSpan = slot->column_span;
    return hit;
}
int doc_view_set_table_selection(doc_view_data* v,
    const xui_doc_table_selection_t* selection)
{
    xui_widget widget = v ? v->widget : NULL;
    xui_doc_table_selection_t expanded;
    xui_doc_position_t caret = {0};
    doc_table_cell_slot first, last;
    doc_node* node;
    int result;
    if (!v || (selection && (selection->iSize != sizeof(*selection) ||
        !selection->iTableId || !selection->iRows || !selection->iColumns)))
        return XUI_ERROR_INVALID_ARGUMENT;
    if (selection && (v->desc.bDisableSelection || v->renderer->mode != XUI_DOC_VISUAL))
        return XUI_ERROR_UNSUPPORTED;
    if (!selection && !v->table_selection.iTableId) return XUI_OK;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    if (selection) {
        expanded = *selection;
        result = doc_table_expand_selection(v->renderer->snapshot->state,
            expanded.iTableId, &expanded.iRow, &expanded.iColumn,
            &expanded.iRows, &expanded.iColumns);
        if (result != XUI_OK) return result;
        result = doc_table_cell_at(v->renderer->snapshot->state,
            expanded.iTableId, expanded.iRow, expanded.iColumn, &first);
        if (result != XUI_OK) return result;
        result = doc_table_cell_at(v->renderer->snapshot->state,
            expanded.iTableId, expanded.iRow + expanded.iRows - 1,
            expanded.iColumn + expanded.iColumns - 1, &last);
        if (result != XUI_OK) return result;
        node = doc_index_get(v->renderer->snapshot->state->index, first.cell);
        while (node && doc_seq_size(node->children)) {
            doc_node* child = doc_index_get(v->renderer->snapshot->state->index,
                doc_seq_get_id(node->children, 0));
            /* A nested table must not steal the outer table's command context. */
            if (!child || child->kind == XUI_DOC_TABLE) break;
            node = child;
        }
        if (!node) return XUI_DOC_ERROR_SCHEMA;
        caret.iSize = sizeof(caret);
        caret.iDocumentId = v->renderer->snapshot->identity;
        caret.iRevision = v->renderer->snapshot->revision;
        caret.iNodeId = node->id;
        caret.iKind = doc_text_kind(node->kind) ?
            XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
        caret.iAffinity = XUI_DOC_AFTER;
        if (!doc_position_valid(v->renderer->snapshot->state, &caret))
            return XUI_DOC_ERROR_SCHEMA;
        v->selection.tAnchor = v->selection.tCaret = caret;
        v->table_selection = expanded;
        v->table_anchor = doc_view_table_slot_hit(&first);
        v->table_focus = doc_view_table_slot_hit(&last);
    } else {
        memset(&v->table_selection, 0, sizeof(v->table_selection));
        memset(&v->table_anchor, 0, sizeof(v->table_anchor));
        memset(&v->table_focus, 0, sizeof(v->table_focus));
    }
    v->table_selecting = 0;
    v->pending_edit_events |= 2;
    if (v->onProjectionChanged) v->onProjectionChanged(widget);
    return xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API int xuiDocumentViewSetTableSelection(xui_widget widget,
    const xui_doc_table_selection_t* selection)
{
    doc_view_data* v = doc_view_get(widget); int result;
    if (!v || (selection && (selection->iSize != sizeof(*selection) ||
        !selection->iTableId || !selection->iRows || !selection->iColumns)))
        return XUI_ERROR_INVALID_ARGUMENT;
    if (v->onBeforeSelection && (result = v->onBeforeSelection(widget)) != XUI_OK)
        return result;
    return doc_view_set_table_selection(v, selection);
}
XUI_API int xuiDocumentViewGetTableSelection(xui_widget widget, xui_doc_table_selection_t* out)
{
    doc_view_data* v = doc_view_get(widget);
    if (!v || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    if (!v->table_selection.iTableId) return XUI_ERROR_NOT_FOUND;
    *out = v->table_selection; return XUI_OK;
}
static int doc_view_set_scroll(doc_view_data* v, double x, double y)
{
    xui_widget widget = v->widget; xui_doc_rect_t size; int exact, result; xui_rect_t content;
    double next_x, next_y;
    result = doc_view_layout(v); if (result != XUI_OK) return result;
    content = xuiWidgetGetContentRect(widget);
    xuiDocumentRendererGetSize(v->renderer, &size, &exact);
    next_y = fmin(fmax(0, y), fmax(0, size.height - content.fH));
    if (x > 0) {
        result = doc_render_cover_x(v->renderer, x + content.fW,
            next_y, content.fH);
        if (result != XUI_OK) return result;
        xuiDocumentRendererGetSize(v->renderer, &size, &exact);
    }
    next_x = fmin(fmax(0, x), fmax(0, size.width - content.fW));
    next_y = fmin(next_y, fmax(0, size.height - content.fH));
    if (next_x == v->scroll_x && next_y == v->scroll_y) return XUI_OK;
    v->scroll_x = next_x; v->scroll_y = next_y;
    return xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API int xuiDocumentViewSetScroll(xui_widget widget, double x, double y)
{
    doc_view_data* v = doc_view_get(widget); int result;
    if (!v || !isfinite(x) || !isfinite(y)) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->onBeforeSelection && (result = v->onBeforeSelection(widget)) != XUI_OK) return result;
    return doc_view_set_scroll(v, x, y);
}
XUI_API int xuiDocumentViewGetScroll(xui_widget widget, double* x, double* y)
{
    doc_view_data* v = doc_view_get(widget); if (!v || !x || !y) return XUI_ERROR_INVALID_ARGUMENT; *x = v->scroll_x; *y = v->scroll_y; return XUI_OK;
}
int doc_view_drag_autoscroll(doc_view_data* v, double dt)
{
    xui_rect_t content; xui_doc_position_t hit;
    double dx = 0, dy = 0, before_x, before_y, x, y; int result;
    if (!v || !v->dragging || !v->drag_pointer_active || v->table_selecting ||
        v->desc.bAutoHeight || dt <= 0) return XUI_OK;
    content = xuiWidgetGetContentRect(v->widget);
    if (content.fW <= 0 || content.fH <= 0) return XUI_OK;
    x = v->drag_pointer_x; y = v->drag_pointer_y;
    if (x < content.fX) dx = -fmin(600, fmax(80, (content.fX - x) * 6)) * dt;
    else if (x > content.fX + content.fW) dx = fmin(600, fmax(80, (x - content.fX - content.fW) * 6)) * dt;
    if (y < content.fY) dy = -fmin(600, fmax(80, (content.fY - y) * 6)) * dt;
    else if (y > content.fY + content.fH) dy = fmin(600, fmax(80, (y - content.fY - content.fH) * 6)) * dt;
    if (!dx && !dy) return XUI_OK;
    before_x = v->scroll_x; before_y = v->scroll_y;
    result = doc_view_set_scroll(v, before_x + dx, before_y + dy);
    if (result != XUI_OK || (before_x == v->scroll_x && before_y == v->scroll_y)) return result;
    x = fmin(fmax(x, content.fX), content.fX + content.fW - 1);
    y = fmin(fmax(y, content.fY), content.fY + content.fH - 1);
    result = xuiDocumentViewHitTest(v->widget, x, y, &hit);
    if (result == XUI_OK) { v->selection.tCaret = hit; v->pending_edit_events |= 2; }
    return result;
}
static int doc_view_rebuild_renderer(doc_view_data* v, float zoom, int prepare_old_layout)
{
    xui_widget widget = v->widget; xui_document_renderer renderer = NULL;
    xui_doc_renderer_desc_t desc; xui_doc_position_t anchor = {0};
    xui_doc_rect_t before = {0}, after = {0}, size; xui_rect_t content;
    double next_x, next_y, width, viewport_height, ratio, anchor_screen_y, anchor_fraction = 0;
    size_t anchor_block = 0, i;
    int anchored = 0, track_prefix = 0, exact, result; unsigned pass;
    result = prepare_old_layout ? doc_view_layout(v) : doc_view_sync(v);
    if (result != XUI_OK) return result;
    content = xuiWidgetGetContentRect(widget);
    width = fmax(1, content.fW); viewport_height = fmax(1, content.fH);
    anchor_screen_y = fmin(20, content.fH / 4);
    ratio = zoom / v->renderer->desc.fZoom;
    next_x = v->scroll_x * ratio; next_y = v->scroll_y * ratio;
    if (!prepare_old_layout) v->renderer->freeze_dynamic_refresh = 1;
    if (!v->desc.bAutoHeight && (v->scroll_x > 0 || v->scroll_y > 0) &&
        content.fW > 0 && content.fH > 0 &&
        xuiDocumentRendererHitTest(v->renderer,
            v->scroll_x + fmin(45, content.fW / 3),
            v->scroll_y + anchor_screen_y, &anchor) == XUI_OK &&
        xuiDocumentRendererGetCaretRect(v->renderer, &anchor, &before) == XUI_OK) {
        anchored = 1;
        anchor_fraction = fmin(.99, fmax(0,
            (v->scroll_y + anchor_screen_y - before.y) / fmax(1, before.height)));
        if (!prepare_old_layout && v->renderer->mode != XUI_DOC_SOURCE_TEXT && v->renderer->count) {
            anchor_block = doc_render_block_at(v->renderer, v->scroll_y + anchor_screen_y);
            track_prefix = 1;
        }
    }
    v->renderer->freeze_dynamic_refresh = 0;
    desc = v->base_renderer; desc.fZoom = zoom;
    if (v->uses_default_font) desc.tFonts.normal = xuiGetDefaultFont(xuiWidgetGetContext(widget));
    result = xuiDocumentRendererCreate(xuiWidgetGetContext(widget), &desc, &renderer);
    if (result == XUI_OK) renderer->width = width;
    if (result == XUI_OK) result = xuiDocumentRendererSetMode(renderer, v->renderer->mode);
    if (result == XUI_OK) renderer->active_source = v->renderer->active_source;
    if (result == XUI_OK) result = xuiDocumentRendererSetSnapshot(renderer, v->renderer->snapshot, NULL);
    if (result == XUI_OK && v->input) result = xuiDocumentRendererSetSourceInput(renderer, v->input);
    if (result == XUI_OK) result = xuiDocumentRendererLayout(renderer, width, 0, 0);
    if (result == XUI_OK && track_prefix && renderer->count == v->renderer->count) {
        for (i = 0; i <= anchor_block; i++) {
            const doc_render_block* old_block = &v->renderer->blocks[i];
            const doc_render_block* new_block = &renderer->blocks[i];
            if (old_block->node != new_block->node || old_block->code_slice != new_block->code_slice ||
                old_block->text_start != new_block->text_start) break;
            if (!old_block->ever_measured && !old_block->object_dependent) continue;
            result = doc_render_materialize(renderer, i);
            if (result != XUI_OK) break;
        }
    }
    if (result == XUI_OK && anchored &&
        xuiDocumentRendererGetCaretRect(renderer, &anchor, &after) == XUI_OK) {
        for (pass = 0; pass < 8; pass++) {
            double revised_y = fmax(0, after.y + anchor_fraction * after.height - anchor_screen_y);
            if (pass && fabs(revised_y - next_y) < .25) { next_y = revised_y; break; }
            next_y = revised_y;
            result = xuiDocumentRendererLayout(renderer, width, next_y, viewport_height);
            if (result != XUI_OK) break;
            if (xuiDocumentRendererGetCaretRect(renderer, &anchor, &after) != XUI_OK) {
                anchored = 0; break;
            }
        }
        if (v->scroll_x > 0) next_x = fmax(0, v->scroll_x + after.x - before.x);
    } else anchored = 0;
    if (result == XUI_OK && !anchored)
        result = xuiDocumentRendererLayout(renderer, width, next_y, viewport_height);
    if (result == XUI_OK) result = xuiDocumentRendererGetSize(renderer, &size, &exact);
    if (result != XUI_OK) { xuiDocumentRendererRelease(renderer); return result; }
    next_x = fmin(fmax(0, next_x), fmax(0, size.width - content.fW));
    next_y = fmin(fmax(0, next_y), fmax(0, size.height - content.fH));
    xuiDocumentRendererRelease(v->renderer); v->renderer = renderer; v->base_renderer = renderer->desc;
    v->scroll_x = next_x; v->scroll_y = next_y;
    v->desc.tRenderer = desc; doc_view_resolve_style(v);
    if (v->onProjectionChanged) v->onProjectionChanged(widget);
    return xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
static int doc_view_set_zoom(doc_view_data* v, float zoom)
{
    xui_context context = xuiWidgetGetContext(v->widget);
    int metrics_changed = v->renderer->dpi_scale != xuiGetVirtualDpi(context) ||
        (v->uses_default_font && v->renderer->desc.tFonts.normal != xuiGetDefaultFont(context));
    if (!metrics_changed && fabsf(zoom - v->renderer->desc.fZoom) < .0001f) return XUI_OK;
    return doc_view_rebuild_renderer(v, zoom, !metrics_changed);
}
XUI_API int xuiDocumentViewSetZoom(xui_widget widget, float zoom)
{
    doc_view_data* v = doc_view_get(widget); int result;
    if (!v || !isfinite(zoom) || zoom < .1f || zoom > 10) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->onBeforeSelection && (result = v->onBeforeSelection(widget)) != XUI_OK) return result;
    return doc_view_set_zoom(v, zoom);
}
XUI_API int xuiDocumentViewInvalidateFonts(xui_widget widget)
{
    doc_view_data* v = doc_view_get(widget);
    xui_context context;
    int metrics_changed, result;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->onBeforeSelection && (result = v->onBeforeSelection(widget)) != XUI_OK)
        return result;
    if (!xuiInternalWidgetIsValid(widget)) return XUI_ERROR_INVALID_STATE;
    context = xuiWidgetGetContext(widget);
    metrics_changed = v->renderer->dpi_scale != xuiGetVirtualDpi(context) ||
        (v->uses_default_font && v->renderer->desc.tFonts.normal != xuiGetDefaultFont(context));
    return doc_view_rebuild_renderer(v, v->renderer->desc.fZoom, !metrics_changed);
}
XUI_API int xuiDocumentViewSetMode(xui_widget widget, uint32_t mode)
{
    doc_view_data* v = doc_view_get(widget); xui_doc_range_t mapped, mapped_scope;
    int scope_result = XUI_OK, result;
    if (!v || (mode != XUI_DOC_VISUAL && mode != XUI_DOC_SOURCE_TEXT && mode != XUI_DOC_LIVE_MARKDOWN)) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->renderer->mode == mode) return XUI_OK;
    if (v->onBeforeTransition && (result = v->onBeforeTransition(widget)) != XUI_OK) return result;
    if (!xuiInternalWidgetIsValid(widget)) return XUI_ERROR_INVALID_STATE;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    mapped = v->selection;
    result = doc_view_map_range(v, mode, &mapped); if (result != XUI_OK) return result;
    if (v->find_scope_active) {
        mapped_scope = v->find_scope;
        scope_result = doc_view_map_range(v, mode, &mapped_scope);
    }
    result = xuiDocumentRendererSetMode(v->renderer, mode);
    if (result != XUI_OK) return result;
    if (scope_result == XUI_OK) {
        if (v->find_scope_active) v->find_scope = mapped_scope;
        doc_view_find_invalidate(v);
    } else doc_view_find_clear(v);
    v->selection = mapped; v->scroll_x = v->scroll_y = 0;
    memset(&v->table_selection, 0, sizeof(v->table_selection));
    v->table_selecting = 0;
    doc_view_invalidate_edit(v); v->pending_edit_events |= 3;
    if (v->onProjectionChanged) v->onProjectionChanged(widget);
    xuiInternalAccessibilityQueue(widget, XUI_ACCESSIBLE_EVENT_TREE_CHANGED);
    return xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API uint32_t xuiDocumentViewGetMode(xui_widget widget)
{
    doc_view_data* v = doc_view_get(widget); return v ? v->renderer->mode : 0;
}
XUI_API int xuiDocumentViewInvalidateObjects(xui_widget widget)
{
    doc_view_data* v = doc_view_get(widget); xui_document_renderer renderer;
    xui_rect_t content; size_t anchor = 0, i;
    uint64_t fragment_node = 0, fragment_start = 0, fragment_end = 0;
    double offset = 0, fragment_offset = 0, next_y;
    int anchored, fragment_anchored = 0, result;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    renderer = v->renderer;
    content = xuiWidgetGetContentRect(widget);
    anchored = !v->desc.bAutoHeight && v->scroll_y > 0 && content.fH > 0 &&
        renderer->mode != XUI_DOC_SOURCE_TEXT && renderer->count &&
        fabs(renderer->width - fmax(1, content.fW)) < .001;
    if (anchored) {
        anchor = doc_render_block_at(renderer, v->scroll_y);
        offset = v->scroll_y - doc_render_height_before(renderer, anchor);
        if (renderer->blocks[anchor].measured && renderer->blocks[anchor].object_dependent) {
            doc_render_block* block = &renderer->blocks[anchor];
            for (i = 0; i < block->fragment_count; i++) {
                doc_fragment* fragment = &block->fragments[i];
                if (offset < fragment->y || offset >= fragment->y + fragment->height) continue;
                fragment_node = fragment->node;
                fragment_start = fragment->start; fragment_end = fragment->end;
                fragment_offset = offset - fragment->y;
                fragment_anchored = 1;
                break;
            }
        }
    }
    result = xuiDocumentRendererInvalidateObjects(renderer);
    if (result != XUI_OK) return result;
    if (anchored) {
        for (i = 0; i <= anchor; i++) {
            if (!renderer->blocks[i].object_dependent) continue;
            result = doc_render_materialize(renderer, i);
            if (result != XUI_OK) return result;
        }
        next_y = doc_render_height_before(renderer, anchor) +
            fmin(offset, fmax(0, renderer->blocks[anchor].height - 1));
        if (fragment_anchored) {
            doc_render_block* block = &renderer->blocks[anchor];
            for (i = 0; i < block->fragment_count; i++) {
                doc_fragment* fragment = &block->fragments[i];
                if (fragment->node != fragment_node || fragment->start != fragment_start ||
                    fragment->end != fragment_end) continue;
                next_y = doc_render_height_before(renderer, anchor) + fragment->y +
                    fmin(fragment_offset, fmax(0, fragment->height - 1));
                break;
            }
        }
        v->scroll_y = fmin(fmax(0, next_y),
            fmax(0, doc_render_height_before(renderer, renderer->count) - content.fH));
    }
    return result == XUI_OK ? xuiWidgetInvalidate(widget,
        XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) : result;
}
XUI_API int xuiDocumentViewGetContentSize(xui_widget widget, xui_doc_rect_t* size, int* exact)
{
    doc_view_data* v = doc_view_get(widget); int result;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_layout(v); return result == XUI_OK ? xuiDocumentRendererGetSize(v->renderer, size, exact) : result;
}
XUI_API int xuiDocumentViewGetRenderStats(xui_widget widget, xui_doc_renderer_stats_t* stats)
{
    doc_view_data* v = doc_view_get(widget);
    return v && v->renderer ? xuiDocumentRendererGetStats(v->renderer, stats) : XUI_ERROR_INVALID_ARGUMENT;
}
XUI_API int xuiDocumentViewHitTest(xui_widget widget, double x, double y, xui_doc_position_t* position)
{
    doc_view_data* v = doc_view_get(widget); xui_rect_t content; int result;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_layout(v); if (result != XUI_OK) return result;
    content = xuiWidgetGetContentRect(widget);
    return xuiDocumentRendererHitTest(v->renderer, x - content.fX + v->scroll_x, y - content.fY + v->scroll_y, position);
}
static void doc_view_cell_local(doc_view_data* v, xui_doc_cell_hit_t* out)
{
    xui_rect_t content = xuiWidgetGetContentRect(v->widget);
    out->tBounds.x += content.fX - v->scroll_x;
    out->tBounds.y += content.fY - v->scroll_y;
}
XUI_API int xuiDocumentViewHitTestCell(xui_widget widget, double x, double y, xui_doc_cell_hit_t* out)
{
    doc_view_data* v = doc_view_get(widget); xui_rect_t content; int result;
    if (!v || !out) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_layout(v); if (result != XUI_OK) return result;
    content = xuiWidgetGetContentRect(widget);
    result = xuiDocumentRendererHitTestCell(v->renderer,
        x - content.fX + v->scroll_x, y - content.fY + v->scroll_y, out);
    if (result == XUI_OK) doc_view_cell_local(v, out);
    return result;
}
XUI_API int xuiDocumentViewGetCellRect(xui_widget widget, xui_doc_node_id cell, xui_doc_cell_hit_t* out)
{
    doc_view_data* v = doc_view_get(widget); int result;
    if (!v || !out) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_layout(v); if (result != XUI_OK) return result;
    result = xuiDocumentRendererGetCellRect(v->renderer, cell, out);
    if (result == XUI_OK) doc_view_cell_local(v, out);
    return result;
}
XUI_API int xuiDocumentViewSetFindScope(xui_widget widget,
    const xui_doc_range_t* scope)
{
    doc_view_data* v = doc_view_get(widget);
    xui_doc_range_t mapped;
    int order, result;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->onBeforeTransition && (result = v->onBeforeTransition(widget)) != XUI_OK)
        return result;
    if (!xuiInternalWidgetIsValid(widget)) return XUI_ERROR_INVALID_STATE;
    if (!scope) {
        if (!v->find_scope_active) return XUI_OK;
        v->find_scope_active = 0;
        memset(&v->find_scope, 0, sizeof(v->find_scope));
        doc_view_find_invalidate(v);
        return xuiWidgetInvalidate(widget,
            XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
    }
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    mapped = *scope;
    result = doc_view_map_range(v, v->renderer->mode, &mapped);
    if (result != XUI_OK) return result;
    result = xuiDocumentSnapshotComparePositions(v->renderer->snapshot,
        &mapped.tAnchor, &mapped.tCaret, &order);
    if (result != XUI_OK) return result;
    if (!order) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->find_scope_active && doc_view_find_range_equal(&v->find_scope, &mapped))
        return XUI_OK;
    v->find_scope = mapped; v->find_scope_active = 1;
    doc_view_find_invalidate(v);
    return xuiWidgetInvalidate(widget,
        XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API int xuiDocumentViewGetFindScope(xui_widget widget,
    xui_doc_range_t* scope)
{
    doc_view_data* v = doc_view_get(widget); int result;
    if (!v || !scope) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    if (!v->find_scope_active) return XUI_ERROR_NOT_FOUND;
    *scope = v->find_scope; return XUI_OK;
}
XUI_API int xuiDocumentViewSetFindQueryEx(xui_widget widget,
    const char* pattern, uint64_t bytes, uint32_t flags)
{
    doc_view_data* v = doc_view_get(widget); xui_doc_range_t* matches; char* copy;
    uint64_t count; int result;
    if (!v || !pattern || !bytes || bytes >= SIZE_MAX) return XUI_ERROR_INVALID_ARGUMENT;
    if (v->onBeforeTransition && (result = v->onBeforeTransition(widget)) != XUI_OK) return result;
    if (!xuiInternalWidgetIsValid(widget)) return XUI_ERROR_INVALID_STATE;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    if (v->find_pattern && v->find_pattern_bytes == bytes &&
            v->find_flags == flags &&
            !memcmp(v->find_pattern, pattern, (size_t)bytes))
        return doc_view_find_refresh(v);
    copy = malloc((size_t)bytes + 1); if (!copy) return XUI_ERROR_OUT_OF_MEMORY;
    memcpy(copy, pattern, (size_t)bytes); copy[bytes] = 0;
    result = doc_view_find_collect(v, copy, bytes, flags, &matches, &count);
    if (result != XUI_OK) { free(copy); return result; }
    doc_view_find_query_clear(v); v->find_pattern = copy;
    v->find_pattern_bytes = bytes; v->find_flags = flags;
    doc_view_find_store(v, matches, count);
    return xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API int xuiDocumentViewSetFindQuery(xui_widget widget,
    const char* pattern, uint64_t bytes)
{
    return xuiDocumentViewSetFindQueryEx(widget, pattern, bytes, 0);
}
XUI_API int xuiDocumentViewClearFind(xui_widget widget)
{
    doc_view_data* v = doc_view_get(widget);
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    doc_view_find_clear(v);
    return xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
XUI_API int xuiDocumentViewGetFindResultCount(xui_widget widget, uint64_t* count)
{
    doc_view_data* v = doc_view_get(widget); int result;
    if (!v || !count) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_find_refresh(v); if (result == XUI_OK) *count = v->find_count;
    return result;
}
XUI_API int xuiDocumentViewGetFindResult(xui_widget widget, uint64_t index, xui_doc_range_t* range, int* active)
{
    doc_view_data* v = doc_view_get(widget); int result;
    if (!v || !range) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_find_refresh(v); if (result != XUI_OK) return result;
    if (index >= v->find_count) return XUI_ERROR_NOT_FOUND;
    *range = v->find_matches[index]; if (active) *active = index == v->find_active;
    return XUI_OK;
}
XUI_API int xuiDocumentViewActivateFindResult(xui_widget widget, uint64_t index,
    xui_doc_range_t* match)
{
    doc_view_data* v = doc_view_get(widget);
    xui_doc_range_t selected;
    xui_doc_rect_t caret;
    int result;
    if (!v) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_find_refresh(v); if (result != XUI_OK) return result;
    if (index >= v->find_count) return XUI_ERROR_NOT_FOUND;
    selected = v->find_matches[index];
    result = xuiDocumentViewSetSelection(widget, &selected);
    if (result != XUI_OK) return result;
    v->find_active = index;
    if (match) *match = selected;
    if (doc_view_layout(v) == XUI_OK &&
        xuiDocumentRendererGetCaretRect(v->renderer, &selected.tAnchor, &caret) == XUI_OK)
        result = xuiDocumentViewSetScroll(widget, v->scroll_x, caret.y);
    return result;
}
XUI_API int xuiDocumentViewFindEx(xui_widget widget, const char* pattern,
    uint64_t bytes, uint32_t flags, int backward, int wrap,
    xui_doc_range_t* match)
{
    doc_view_data* v = doc_view_get(widget); uint64_t i, chosen = DOC_NONE; int result, order;
    xui_doc_position_t boundary;
    if (!v || !match) return XUI_ERROR_INVALID_ARGUMENT;
    result = xuiDocumentViewSetFindQueryEx(widget, pattern, bytes, flags);
    if (result != XUI_OK) return result;
    if (!v->find_count) return XUI_ERROR_NOT_FOUND;
    if (v->find_active != DOC_NONE && doc_view_find_range_equal(&v->selection, &v->find_matches[v->find_active])) {
        if (backward && v->find_active) chosen = v->find_active - 1;
        else if (!backward && v->find_active + 1 < v->find_count) chosen = v->find_active + 1;
    } else {
        result = doc_position_compare(v->renderer->snapshot->state, &v->selection.tAnchor, &v->selection.tCaret, &order);
        if (result != XUI_OK) return result;
        boundary = (backward ? order < 0 : order > 0) ? v->selection.tAnchor : v->selection.tCaret;
        for (i = 0; i < v->find_count; i++) {
            uint64_t index = backward ? v->find_count - 1 - i : i;
            result = doc_position_compare(v->renderer->snapshot->state,
                backward ? &v->find_matches[index].tCaret : &v->find_matches[index].tAnchor, &boundary, &order);
            if (result != XUI_OK) return result;
            if (backward ? order <= 0 : order >= 0) { chosen = index; break; }
        }
    }
    if (chosen == DOC_NONE && wrap) chosen = backward ? v->find_count - 1 : 0;
    if (chosen == DOC_NONE) return XUI_ERROR_NOT_FOUND;
    return xuiDocumentViewActivateFindResult(widget, chosen, match);
}
XUI_API int xuiDocumentViewFind(xui_widget widget, const char* pattern,
    uint64_t bytes, int backward, int wrap, xui_doc_range_t* match)
{
    return xuiDocumentViewFindEx(widget, pattern, bytes, 0, backward,
        wrap, match);
}
int doc_view_measure(xui_widget widget, xui_vec2_t constraint, xui_vec2_t* size, void* user)
{
    doc_view_data* v = doc_view_get(widget); xui_doc_rect_t bounds; int exact, result;
    double width = constraint.fX > 0 && constraint.fX < 1000000 ? constraint.fX : 480;
    (void)user;
    if (!v || !size) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_refresh_resources(v); if (result != XUI_OK) return result;
    doc_view_resolve_style(v);
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
    result = doc_view_paint_background(v, draw, content); if (result != XUI_OK) return result;
    if (!v->renderer->input) { result = doc_view_find_refresh(v); if (result != XUI_OK) return result; }
    v->renderer->table_selection = v->table_selection;
    v->renderer->table_selection_color = doc_view_style_color(widget,
        "document.selection.color", v->desc.iSelectionColor);
    v->renderer->find_matches = v->renderer->input ? NULL : v->find_matches;
    v->renderer->find_count = v->renderer->input ? 0 : v->find_count;
    v->renderer->find_active = v->find_active;
    v->renderer->find_result_color = doc_view_style_color(widget,
        "document.find.result_color", XUI_COLOR_RGBA(255, 235, 128, 150));
    v->renderer->find_active_color = doc_view_style_color(widget,
        "document.find.active_color", XUI_COLOR_RGBA(255, 183, 77, 190));
    result = xuiDocumentRendererDraw(v->renderer, draw, content.fX - v->scroll_x, content.fY - v->scroll_y, content,
        v->desc.bDisableSelection || v->table_selection.iTableId ? NULL : &v->selection,
        v->renderer->table_selection_color);
    memset(&v->renderer->table_selection, 0, sizeof(v->renderer->table_selection));
    v->renderer->find_matches = NULL; v->renderer->find_count = 0;
    return result == XUI_OK ? doc_view_paint_border(v, draw, content) : result;
}
static void doc_view_table_drag(doc_view_data* v, const xui_doc_cell_hit_t* hit)
{
    uint32_t first_row = v->table_anchor.iRow < hit->iRow ? v->table_anchor.iRow : hit->iRow;
    uint32_t first_column = v->table_anchor.iColumn < hit->iColumn ? v->table_anchor.iColumn : hit->iColumn;
    uint32_t end_row = v->table_anchor.iRow + v->table_anchor.iRowSpan;
    uint32_t end_column = v->table_anchor.iColumn + v->table_anchor.iColumnSpan;
    if (hit->iRow + hit->iRowSpan > end_row) end_row = hit->iRow + hit->iRowSpan;
    if (hit->iColumn + hit->iColumnSpan > end_column) end_column = hit->iColumn + hit->iColumnSpan;
    v->table_selection = (xui_doc_table_selection_t){sizeof(v->table_selection),
        hit->iTableId, first_row, first_column, end_row - first_row, end_column - first_column};
    v->table_focus = *hit;
}
static int doc_view_table_position_slot(doc_state* state,
    const xui_doc_position_t* position, doc_table_cell_slot* slot)
{
    doc_node* node;
    if (position->iKind == XUI_DOC_POSITION_SOURCE) return XUI_ERROR_NOT_FOUND;
    node = doc_index_get(state->index, position->iNodeId);
    while (node && node->kind != XUI_DOC_CELL)
        node = node->parent ? doc_index_get(state->index, node->parent) : NULL;
    return node ? doc_table_locate_cell(state, node->id, slot) : XUI_ERROR_NOT_FOUND;
}
int doc_view_extend_table_selection(doc_view_data* v, int key)
{
    doc_state* state; doc_table_cell_slot anchor, focus, target;
    xui_doc_table_selection_t rectangle = {0};
    uint32_t row, column, end_row, end_column;
    int active, at_edge = 0, result;
    if (!v || v->desc.bDisableSelection || v->renderer->mode != XUI_DOC_VISUAL)
        return XUI_ERROR_NOT_FOUND;
    if (key != XUI_KEY_LEFT && key != XUI_KEY_RIGHT &&
        key != XUI_KEY_UP && key != XUI_KEY_DOWN) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_sync(v); if (result != XUI_OK) return result;
    state = v->renderer->snapshot->state;
    active = v->table_selection.iTableId != 0;
    if (active && v->table_anchor.iCellId && v->table_focus.iCellId) {
        result = doc_table_locate_cell(state, v->table_anchor.iCellId, &anchor);
        if (result == XUI_OK) result = doc_table_locate_cell(state, v->table_focus.iCellId, &focus);
        if (result != XUI_OK || anchor.table != v->table_selection.iTableId ||
            focus.table != anchor.table) {
            result = XUI_ERROR_NOT_FOUND;
        }
    } else result = XUI_ERROR_NOT_FOUND;
    if (active && result != XUI_OK) {
        const xui_doc_table_selection_t* selected = &v->table_selection;
        result = doc_table_cell_at(state, selected->iTableId,
            selected->iRow, selected->iColumn, &anchor);
        if (result == XUI_OK) result = doc_table_cell_at(state, selected->iTableId,
            selected->iRow + selected->iRows - 1,
            selected->iColumn + selected->iColumns - 1, &focus);
        if (result != XUI_OK) return result;
    }
    if (!active) {
        result = doc_view_table_position_slot(state, &v->selection.tCaret, &focus);
        if (result != XUI_OK) return result;
        anchor = focus;
    }
    row = focus.row; column = focus.column;
    if (key == XUI_KEY_LEFT) {
        if (!column) at_edge = 1; else column--;
    } else if (key == XUI_KEY_RIGHT) {
        column += focus.column_span; if (column >= focus.columns) at_edge = 1;
    } else if (key == XUI_KEY_UP) {
        if (!row) at_edge = 1; else row--;
    } else {
        row += focus.row_span; if (row >= focus.rows) at_edge = 1;
    }
    if (at_edge && active) return XUI_OK;
    target = focus;
    if (!at_edge) {
        result = doc_table_cell_at(state, focus.table, row, column, &target);
        if (result != XUI_OK) return result;
    }
    rectangle.iSize = sizeof(rectangle); rectangle.iTableId = anchor.table;
    rectangle.iRow = anchor.row < target.row ? anchor.row : target.row;
    rectangle.iColumn = anchor.column < target.column ? anchor.column : target.column;
    end_row = anchor.row + anchor.row_span;
    end_column = anchor.column + anchor.column_span;
    if (target.row + target.row_span > end_row) end_row = target.row + target.row_span;
    if (target.column + target.column_span > end_column)
        end_column = target.column + target.column_span;
    rectangle.iRows = end_row - rectangle.iRow;
    rectangle.iColumns = end_column - rectangle.iColumn;
    result = doc_view_set_table_selection(v, &rectangle);
    if (result != XUI_OK) return result;
    v->table_anchor = doc_view_table_slot_hit(&anchor);
    v->table_focus = doc_view_table_slot_hit(&target);
    /* A large selection keeps its moving end visible while the ordinary
     * caret remains at the rectangle's top-left Cell for table commands. */
    if (doc_view_layout(v) == XUI_OK) {
        xui_doc_cell_hit_t cell = {0}; xui_rect_t content = xuiWidgetGetContentRect(v->widget);
        cell.iSize = sizeof(cell);
        if (content.fW > 0 && content.fH > 0 &&
            xuiDocumentRendererGetCellRect(v->renderer, target.cell, &cell) == XUI_OK) {
            double x = v->scroll_x, y = v->scroll_y;
            if (cell.tBounds.x < x) x = cell.tBounds.x;
            else if (cell.tBounds.x + cell.tBounds.width > x + content.fW)
                x = cell.tBounds.x + cell.tBounds.width - content.fW;
            if (cell.tBounds.y < y) y = cell.tBounds.y;
            else if (cell.tBounds.y + cell.tBounds.height > y + content.fH)
                y = cell.tBounds.y + cell.tBounds.height - content.fH;
            (void)xuiDocumentViewSetScroll(v->widget, x, y);
        }
    }
    return XUI_OK;
}
int doc_view_event(xui_widget widget, const xui_event_t* event, void* user)
{
    doc_view_data* v = doc_view_get(widget); xui_rect_t world; xui_doc_position_t hit; int result;
    (void)user;
    if (!v || !event) return XUI_ERROR_INVALID_ARGUMENT;
    world = xuiWidgetGetWorldRect(widget);
    if (event->iType == XUI_EVENT_KEY_DOWN &&
        (event->iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT | XUI_MOD_SHIFT)) ==
            (XUI_MOD_ALT | XUI_MOD_SHIFT) &&
        (event->iKey == XUI_KEY_LEFT || event->iKey == XUI_KEY_RIGHT ||
         event->iKey == XUI_KEY_UP || event->iKey == XUI_KEY_DOWN)) {
        result = doc_view_extend_table_selection(v, event->iKey);
        return result == XUI_ERROR_NOT_FOUND ? XUI_OK :
            result == XUI_OK ? (int)XUI_EVENT_DISPATCH_STOP : result;
    }
    if (event->iType == XUI_EVENT_KEY_DOWN && event->iModifiers & XUI_MOD_CTRL && !v->desc.bDisableSelection) {
        if (event->iKey == 'C' || event->iKey == 'c') { xuiEditCopy(widget); return XUI_EVENT_DISPATCH_STOP; }
        if (event->iKey == 'A' || event->iKey == 'a') { xuiEditSelectAll(widget); return XUI_EVENT_DISPATCH_STOP; }
    }
    if (event->iType == XUI_EVENT_POINTER_WHEEL) {
        if (event->iModifiers & XUI_MOD_CTRL) result = doc_view_set_zoom(v, fminf(10, fmaxf(.1f, v->renderer->desc.fZoom * (event->fWheelY > 0 ? 1.1f : 1 / 1.1f))));
        else if (v->desc.bAutoHeight) return XUI_OK; /* Let the containing scroll host handle it. */
        else result = doc_view_set_scroll(v, v->scroll_x - event->fWheelX * 48, v->scroll_y - event->fWheelY * 48);
        if (result != XUI_OK) return result;
        return XUI_EVENT_DISPATCH_STOP;
    }
    if (event->iType == XUI_EVENT_BLUR || event->iType == XUI_EVENT_POINTER_CAPTURE_LOST) {
        v->dragging = 0; v->drag_pointer_active = 0; v->table_selecting = 0; v->pressed_node = 0; return XUI_OK;
    }
    if (event->iType == XUI_EVENT_POINTER_DOWN && (event->iButton == XUI_POINTER_BUTTON_LEFT || !event->iButton)) {
        xui_doc_cell_hit_t cell = {0}; int table_drag = 0;
        result = xuiDocumentViewHitTest(widget, event->fX - world.fX, event->fY - world.fY, &hit);
        if (result != XUI_OK) return result;
        if (!v->desc.bDisableSelection && v->renderer->mode == XUI_DOC_VISUAL &&
            (event->iModifiers & XUI_MOD_ALT)) {
            cell.iSize = sizeof(cell);
            table_drag = xuiDocumentViewHitTestCell(widget,
                event->fX - world.fX, event->fY - world.fY, &cell) == XUI_OK;
        }
        if (!v->replaying_input) xuiSetFocusWidget(xuiWidgetGetContext(widget), widget);
        v->pressed_node = table_drag ? 0 : hit.iNodeId;
        if (!v->desc.bDisableSelection) {
            if (table_drag) {
                v->table_anchor = cell; v->table_selecting = 1;
                v->selection.tAnchor = v->selection.tCaret = hit;
                doc_view_table_drag(v, &cell);
            } else {
                xui_doc_range_t object_range;
                memset(&v->table_selection, 0, sizeof(v->table_selection));
                v->table_selecting = 0;
                if (!(event->iModifiers & XUI_MOD_SHIFT) &&
                    v->renderer->mode == XUI_DOC_VISUAL &&
                    doc_view_object_range(v, hit.iNodeId, &object_range) == XUI_OK)
                    v->selection = object_range;
                else {
                    if (!(event->iModifiers & XUI_MOD_SHIFT)) v->selection.tAnchor = hit;
                    v->selection.tCaret = hit;
                }
            }
            v->dragging = 1; v->drag_pointer_active = 0;
            v->pending_edit_events |= 2;
            if (!v->replaying_input) xuiSetPointerCaptureEx(xuiWidgetGetContext(widget), event->iPointerId, event->iPointerType, widget);
        }
        xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER |
            (v->renderer->mode == XUI_DOC_LIVE_MARKDOWN ? XUI_WIDGET_DIRTY_LAYOUT : 0)); return XUI_EVENT_DISPATCH_STOP;
    }
    if (event->iType == XUI_EVENT_POINTER_MOVE && v->dragging) {
        xui_rect_t content = xuiWidgetGetContentRect(widget);
        int was_outside = v->drag_pointer_active &&
            (v->drag_pointer_x < content.fX || v->drag_pointer_x > content.fX + content.fW ||
             v->drag_pointer_y < content.fY || v->drag_pointer_y > content.fY + content.fH);
        int is_outside;
        v->drag_pointer_x = event->fX - world.fX;
        v->drag_pointer_y = event->fY - world.fY;
        v->drag_pointer_active = 1;
        is_outside = v->drag_pointer_x < content.fX || v->drag_pointer_x > content.fX + content.fW ||
            v->drag_pointer_y < content.fY || v->drag_pointer_y > content.fY + content.fH;
        if (v->table_selecting) {
            xui_doc_cell_hit_t cell = {0}; cell.iSize = sizeof(cell);
            if (xuiDocumentViewHitTestCell(widget, event->fX - world.fX,
                event->fY - world.fY, &cell) == XUI_OK &&
                cell.iTableId == v->table_anchor.iTableId) {
                doc_view_table_drag(v, &cell); v->pending_edit_events |= 2;
            }
            xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
            return XUI_EVENT_DISPATCH_STOP;
        }
        if (is_outside && !was_outside) {
            result = doc_view_drag_autoscroll(v, 1.0 / 60.0);
            if (result != XUI_OK) return result;
        }
        {
            double x = fmin(fmax(v->drag_pointer_x, content.fX), content.fX + fmax(0, content.fW - 1));
            double y = fmin(fmax(v->drag_pointer_y, content.fY), content.fY + fmax(0, content.fH - 1));
            result = xuiDocumentViewHitTest(widget, x, y, &hit);
        }
        if (result == XUI_OK) { v->selection.tCaret = hit; v->pending_edit_events |= 2; }
        v->pressed_node = 0; xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER |
            (v->renderer->mode == XUI_DOC_LIVE_MARKDOWN ? XUI_WIDGET_DIRTY_LAYOUT : 0)); return XUI_EVENT_DISPATCH_STOP;
    }
    if (event->iType == XUI_EVENT_POINTER_UP) {
        uint64_t pressed = v->pressed_node;
        int table_result = XUI_OK;
        if (v->table_selecting && v->table_selection.iTableId) {
            xui_doc_cell_hit_t cell = {0}; cell.iSize = sizeof(cell);
            if (xuiDocumentViewHitTestCell(widget, event->fX - world.fX,
                event->fY - world.fY, &cell) == XUI_OK &&
                cell.iTableId == v->table_anchor.iTableId)
                doc_view_table_drag(v, &cell);
            table_result = doc_table_expand_selection(v->renderer->snapshot->state,
                v->table_selection.iTableId, &v->table_selection.iRow,
                &v->table_selection.iColumn, &v->table_selection.iRows,
                &v->table_selection.iColumns);
            if (table_result != XUI_OK)
                memset(&v->table_selection, 0, sizeof(v->table_selection));
            v->pending_edit_events |= 2;
            xuiWidgetInvalidate(widget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
        }
        v->dragging = 0; v->drag_pointer_active = 0; v->table_selecting = 0; v->pressed_node = 0;
        if (!v->replaying_input) xuiReleasePointerCaptureEx(xuiWidgetGetContext(widget), event->iPointerId, event->iPointerType, widget);
        if (table_result != XUI_OK) return table_result;
        if (pressed && v->desc.onActivate) {
            xui_doc_node_info_t info = {0}; info.iSize = sizeof(info);
            result = xuiDocumentViewHitTest(widget, event->fX - world.fX, event->fY - world.fY, &hit);
            if (result == XUI_OK && hit.iNodeId == pressed && xuiDocumentSnapshotGetNode(v->renderer->snapshot, pressed, &info) == XUI_OK) {
                xui_document_snapshot retained = v->renderer->snapshot;
                xuiDocumentSnapshotRetain(retained);
                v->desc.onActivate(widget, pressed,
                    info.iKind == XUI_DOC_IMAGE && (info.tAttributes.iMarks & XUI_DOC_LINK) ?
                        info.sLinkTarget : info.sResource, v->desc.pUser);
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
    v->uses_default_font = !desc->tRenderer.iSize || !desc->tRenderer.tFonts.normal;
    v->base_renderer = v->renderer->desc;
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
    doc_view_accessible_clear(v);
    doc_view_invalidate_edit(v);
    doc_view_find_clear(v);
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
    if (xuiWidgetRegisterType(context, &type, &desc) != XUI_OK) return NULL;
    doc_view_register_styles(context, type);
    return type;
}
XUI_API int xuiDocumentViewCreate(xui_context context, const xui_doc_view_desc_t* desc, xui_widget* out)
{
    xui_widget_type type;
    if (out) *out = NULL;
    if (!out || !desc || desc->iSize != sizeof(*desc) || !desc->pDocument) return XUI_ERROR_INVALID_ARGUMENT;
    type = xuiDocumentViewGetType(context); return type ? xuiWidgetCreateTyped(context, type, out, desc) : XUI_ERROR_OUT_OF_MEMORY;
}

#endif
