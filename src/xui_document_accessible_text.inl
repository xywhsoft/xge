/* Included by the Document edit adapter. This is a projection over the same
 * Document positions and changes used by the editor, not another text store. */
struct doc_accessible_text_range {
    xui_internal_accessible_text_range base;
    atomic_uint refs;
    doc_view_data* owner;
    xui_doc_range_t positions;
    int stale;
    doc_accessible_text_range* next;
};
static const xui_internal_accessible_text_ops doc_text_ops;
static void doc_text_tag_positions(doc_view_data* view, xui_doc_range_t* positions)
{
    if (view->visual_input) {
        xui_doc_prepare_info_t info = {0}; info.iSize = sizeof(info);
        if (xuiDocumentPrepareGetInfo(view->visual_input, &info) == XUI_OK)
            positions->tAnchor.iInputGeneration = positions->tCaret.iInputGeneration = info.iGeneration;
    }
}
static doc_accessible_text_range* doc_text_cast(xui_internal_accessible_text_range* range)
{ return (doc_accessible_text_range*)range; }
static void doc_text_retain(xui_internal_accessible_text_range* range)
{ atomic_fetch_add(&doc_text_cast(range)->refs, 1); }
static void doc_text_release(xui_internal_accessible_text_range* range)
{
    if (atomic_fetch_sub(&doc_text_cast(range)->refs, 1) == 1) free(range);
}
static void doc_text_prune(doc_view_data* view)
{
    doc_accessible_text_range** link = &view->accessible_ranges;
    while (*link) {
        doc_accessible_text_range* range = *link;
        if (atomic_load(&range->refs) == 1) {
            *link = range->next; range->owner = NULL;
            doc_text_release(&range->base);
        } else link = &range->next;
    }
}
void doc_view_text_ranges_clear(doc_view_data* view)
{
    doc_accessible_text_range* range = view->accessible_ranges;
    view->accessible_ranges = NULL;
    while (range) {
        doc_accessible_text_range* next = range->next;
        range->owner = NULL; range->stale = 1;
        doc_text_release(&range->base); range = next;
    }
}
void doc_view_text_ranges_changed(doc_view_data* view, xui_document_change_set changes)
{
    doc_accessible_text_range* range;
    doc_text_prune(view);
    for (range = view->accessible_ranges; range; range = range->next) {
        xui_doc_range_t before = range->positions, mapped; int mapping, result = XUI_OK; unsigned i;
        if (range->stale) continue;
        /* External writes supersede accepted input. Restore its base positions
         * before applying that write; a publication maps its exact generation
         * directly through the ChangeSet, without replaying input patches. */
        for (i = 0; changes && result == XUI_OK && i < 2; i++) {
            xui_doc_position_t* p = i ? &before.tCaret : &before.tAnchor;
            if (p->iInputGeneration && p->iInputGeneration != changes->input_generation)
                result = xuiDocumentPrepareMapPosition(view->visual_input ? view->visual_input : view->input,
                    NULL, p, p, &mapping);
        }
        if (!changes || result != XUI_OK ||
            xuiDocumentMapPosition(changes, &before.tAnchor,
                &mapped.tAnchor, &mapping) != XUI_OK ||
            xuiDocumentMapPosition(changes, &before.tCaret,
                &mapped.tCaret, &mapping) != XUI_OK) range->stale = 1;
        else range->positions = mapped;
    }
}
void doc_view_text_ranges_visual(doc_view_data* view, xui_document_prepare previous,
    xui_document_prepare next)
{
    doc_accessible_text_range* range;
    doc_text_prune(view);
    for (range = view->accessible_ranges; range; range = range->next) {
        xui_doc_range_t mapped = range->positions; unsigned i; int result = XUI_OK, quality;
        if (range->stale) continue;
        for (i = 0; i < 2 && result == XUI_OK; i++) {
            xui_doc_position_t* p = i ? &mapped.tCaret : &mapped.tAnchor;
            if (p->iInputGeneration || next)
                result = xuiDocumentPrepareMapPosition(p->iInputGeneration ? previous : NULL,
                    next, p, p, &quality);
        }
        if (result == XUI_OK) range->positions = mapped;
        else range->stale = 1;
    }
}
void doc_view_text_ranges_input(doc_view_data* view, xui_document_prepare previous,
    xui_document_prepare next)
{
    doc_accessible_text_range* range;
    doc_text_prune(view);
    for (range = view->accessible_ranges; range; range = range->next) {
        xui_doc_range_t mapped = range->positions; unsigned i; int result = XUI_OK, quality;
        if (range->stale) continue;
        for (i = 0; i < 2 && result == XUI_OK; i++) {
            xui_doc_position_t* p = i ? &mapped.tCaret : &mapped.tAnchor;
            if (p->iInputGeneration) {
                result = xuiDocumentPrepareMapSourcePosition(previous, next, p, p, &quality);
            } else if (next) {
                if (p->iKind != XUI_DOC_POSITION_SOURCE) {
                    uint64_t offset;
                    result = xuiDocumentPositionToSource(view->renderer->snapshot, p, &offset, &quality);
                    if (result != XUI_OK) break;
                    p->iKind = XUI_DOC_POSITION_SOURCE; p->iNodeId = DOC_ROOT; p->iOffset = offset;
                }
                result = xuiDocumentPrepareMapSourcePosition(NULL, next, p, p, &quality);
            }
            /* Publication/external-write callbacks have already mapped and
             * cleared generation. Forgetting the input must leave them alone. */
        }
        if (result == XUI_OK) range->positions = mapped;
        else range->stale = 1;
    }
}
static int doc_text_begin(doc_accessible_text_range* range, xui_context* context, int action)
{
    doc_view_data* view = range->owner; xui_widget widget; xui_widget modal;
    *context = NULL;
    if (!view || range->stale) return XUI_DOC_ERROR_STALE;
    widget = view->widget;
    if (!xuiInternalWidgetIsValid(widget) ||
        xuiInternalContextDestroyPending(widget->pContext) || widget->pContext->bDestroying)
        return XUI_DOC_ERROR_STALE;
    if (action) {
        modal = xuiInternalInputModalRoot(widget->pContext);
        if (widget->bAccessibleActionActive || widget->bAccessibleQueryActive ||
            !xuiWidgetGetEffectiveVisible(widget) || !xuiWidgetGetEffectiveEnabled(widget) ||
            (modal && !xuiInternalInputFocusContains(modal, widget)))
            return XUI_ERROR_INVALID_STATE;
    }
    *context = widget->pContext; xuiInternalOperationEnter(*context);
    return XUI_OK;
}
static int doc_text_end(xui_context context, int result)
{
    if (context) xuiInternalOperationLeave(context);
    return result;
}
static int doc_text_project(doc_accessible_text_range* range, xui_doc_range_t* mapped,
    uint64_t* first, uint64_t* last)
{
    doc_view_data* view = range->owner; int result;
    result = doc_view_project_edit(view); if (result != XUI_OK) return result;
    *mapped = range->positions;
    if (view->visual_input) {
        int quality;
        result = xuiDocumentPrepareMapPosition(view->visual_input, view->visual_input,
            &mapped->tAnchor, &mapped->tAnchor, &quality);
        if (result == XUI_OK) result = xuiDocumentPrepareMapPosition(view->visual_input, view->visual_input,
            &mapped->tCaret, &mapped->tCaret, &quality);
        if (result != XUI_OK) return result;
        mapped->tAnchor.iInputGeneration = mapped->tCaret.iInputGeneration = 0;
    }
    result = doc_view_map_range(view, view->renderer->mode, mapped);
    if (result != XUI_OK) return result;
    result = doc_plain_project_position(&view->edit_projection, &mapped->tAnchor, first);
    if (result == XUI_OK)
        result = doc_plain_project_position(&view->edit_projection, &mapped->tCaret, last);
    if (result == XUI_OK && *first > *last) {
        uint64_t offset = *first; xui_doc_position_t position = mapped->tAnchor;
        *first = *last; *last = offset;
        mapped->tAnchor = mapped->tCaret; mapped->tCaret = position;
    }
    return result;
}
static int doc_text_new(doc_view_data* view, const xui_doc_range_t* positions,
    xui_internal_accessible_text_range** out)
{
    doc_accessible_text_range* range;
    *out = NULL; doc_text_prune(view);
    range = calloc(1, sizeof(*range)); if (!range) return XUI_ERROR_OUT_OF_MEMORY;
    range->base.ops = &doc_text_ops; atomic_init(&range->refs, 2);
    range->owner = view; range->positions = *positions;
    doc_text_tag_positions(view, &range->positions);
    range->next = view->accessible_ranges; view->accessible_ranges = range;
    *out = &range->base; return XUI_OK;
}
static int doc_text_clone(xui_internal_accessible_text_range* base,
    xui_internal_accessible_text_range** out)
{
    doc_accessible_text_range* range = doc_text_cast(base); xui_context context; int result;
    if (!out) return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    result = doc_text_begin(range, &context, 0);
    if (result == XUI_OK) result = doc_text_new(range->owner, &range->positions, out);
    return doc_text_end(context, result);
}
static int doc_text_offsets(xui_internal_accessible_text_range* base,
    uint64_t* first, uint64_t* last, uint64_t* length)
{
    doc_accessible_text_range* range = doc_text_cast(base); xui_context context;
    xui_doc_range_t mapped; uint64_t a, b; int result;
    if (!first || !last || !length) return XUI_ERROR_INVALID_ARGUMENT;
    *first = *last = *length = 0;
    result = doc_text_begin(range, &context, 0);
    if (result == XUI_OK) result = doc_text_project(range, &mapped, &a, &b);
    if (result == XUI_OK) { *first = a; *last = b; *length = range->owner->edit_projection.bytes; }
    return doc_text_end(context, result);
}
static int doc_text_set_offsets(xui_internal_accessible_text_range* base, uint64_t first, uint64_t last)
{
    doc_accessible_text_range* range = doc_text_cast(base); xui_context context;
    xui_doc_range_t mapped; doc_plain_projection* projection; uint64_t a, b; int result;
    result = doc_text_begin(range, &context, 0);
    if (result == XUI_OK) result = doc_text_project(range, &mapped, &a, &b);
    if (result == XUI_OK) {
        projection = &range->owner->edit_projection;
        if (first > last || last > projection->bytes) result = XUI_ERROR_INVALID_ARGUMENT;
        else if ((first < projection->bytes && ((unsigned char)projection->text[first] & 0xc0) == 0x80) ||
            (last < projection->bytes && ((unsigned char)projection->text[last] & 0xc0) == 0x80))
            result = XUI_DOC_ERROR_UTF8;
        else {
            mapped.tAnchor = doc_plain_unproject(projection, first, 1);
            mapped.tCaret = first == last ? mapped.tAnchor : doc_plain_unproject(projection, last, 0);
            range->positions = mapped;
            doc_text_tag_positions(range->owner, &range->positions);
        }
    }
    return doc_text_end(context, result);
}
static int doc_text_get_text(xui_internal_accessible_text_range* base,
    char* buffer, uint64_t capacity, uint64_t* bytes)
{
    doc_accessible_text_range* range = doc_text_cast(base); xui_context context;
    xui_doc_range_t mapped; uint64_t first, last; int result;
    if (!bytes || (!buffer && capacity)) return XUI_ERROR_INVALID_ARGUMENT;
    *bytes = 0;
    result = doc_text_begin(range, &context, 0);
    if (result == XUI_OK) result = doc_text_project(range, &mapped, &first, &last);
    if (result == XUI_OK) {
        *bytes = last - first;
        if (buffer && capacity <= *bytes) result = XUI_ERROR_BUFFER_TOO_SMALL;
        else if (buffer) {
            memcpy(buffer, range->owner->edit_projection.text + first, (size_t)*bytes);
            buffer[*bytes] = 0;
        }
    }
    return doc_text_end(context, result);
}
/* Match the render tree: only attached, visible widgets paint, and ancestor
 * CLIP/HIDDEN overflow intersects the context viewport. Popup focus owners
 * are not rendering parents and must not become an extra clip. */
static int doc_text_visible_clip(doc_view_data* view, double* left, double* top,
    double* right, double* bottom)
{
    xui_widget widget = view->widget, parent; xui_context context = widget->pContext;
    xui_rect_t world, content;
    if (!xuiWidgetIsAttachedToContext(widget) || !xuiWidgetGetEffectiveVisible(widget) ||
        context->fViewportWidth <= 0 || context->fViewportHeight <= 0) return 0;
    world = xuiWidgetGetWorldRect(widget); content = xuiWidgetGetContentRect(widget);
    *left = fmax(0, (double)world.fX + content.fX);
    *top = fmax(0, (double)world.fY + content.fY);
    *right = fmin(context->fViewportWidth, (double)world.fX + content.fX + content.fW);
    *bottom = fmin(context->fViewportHeight, (double)world.fY + content.fY + content.fH);
    for (parent = widget->pParent; parent; parent = parent->pParent) {
        if (parent->tLayout.iOverflow == XUI_OVERFLOW_CLIP || parent->tLayout.iOverflow == XUI_OVERFLOW_HIDDEN) {
            xui_rect_t bounds = xuiWidgetGetWorldRect(parent);
            *left = fmax(*left, bounds.fX); *top = fmax(*top, bounds.fY);
            *right = fmin(*right, (double)bounds.fX + bounds.fW);
            *bottom = fmin(*bottom, (double)bounds.fY + bounds.fH);
        }
    }
    return *right > *left && *bottom > *top;
}
static int doc_text_rects(xui_internal_accessible_text_range* base,
    xui_rect_t* output, uint64_t capacity, uint64_t* total)
{
    doc_accessible_text_range* range = doc_text_cast(base); xui_context context;
    doc_view_data* view; xui_doc_range_t mapped; xui_doc_rect_t* rectangles = NULL;
    xui_rect_t world, content; uint64_t first, last, count = 0, i; int result, visible = 0;
    double clip_left = 0, clip_top = 0, clip_right = 0, clip_bottom = 0, origin_x, origin_y;
    if (!total || (capacity && !output)) return XUI_ERROR_INVALID_ARGUMENT;
    *total = 0;
    result = doc_text_begin(range, &context, 0);
    if (result != XUI_OK) return result;
    view = range->owner;
    result = doc_text_project(range, &mapped, &first, &last);
    if (result == XUI_OK) visible = doc_text_visible_clip(view, &clip_left, &clip_top, &clip_right, &clip_bottom);
    if (result == XUI_OK && visible) {
        result = doc_view_layout(view);
        if (result == XUI_OK) visible = doc_text_visible_clip(view, &clip_left, &clip_top, &clip_right, &clip_bottom);
    }
    if (result == XUI_OK && visible && first != last)
        result = xuiDocumentRendererGetRangeRects(view->renderer, &mapped, NULL, 0, &count);
    if (result == XUI_OK && count) {
        if (count > SIZE_MAX / sizeof(*rectangles)) result = XUI_DOC_ERROR_LIMIT;
        else {
            rectangles = malloc((size_t)count * sizeof(*rectangles));
            if (!rectangles) result = XUI_ERROR_OUT_OF_MEMORY;
            else result = xuiDocumentRendererGetRangeRects(view->renderer, &mapped, rectangles, count, &count);
        }
    }
    if (result == XUI_OK) {
        world = xuiWidgetGetWorldRect(view->widget); content = xuiWidgetGetContentRect(view->widget);
        origin_x = (double)world.fX + content.fX - view->scroll_x;
        origin_y = (double)world.fY + content.fY - view->scroll_y;
        for (i = 0; i < count; i++) {
            double left = fmax(clip_left, rectangles[i].x + origin_x);
            double top = fmax(clip_top, rectangles[i].y + origin_y);
            double right = fmin(clip_right, rectangles[i].x + rectangles[i].width + origin_x);
            double bottom = fmin(clip_bottom, rectangles[i].y + rectangles[i].height + origin_y);
            if (right <= left || bottom <= top) continue;
            if (*total < capacity) output[*total] = (xui_rect_t){(float)left, (float)top,
                (float)(right - left), (float)(bottom - top)};
            (*total)++;
        }
    }
    free(rectangles); return doc_text_end(context, result);
}
static int doc_text_select(xui_internal_accessible_text_range* base)
{
    doc_accessible_text_range* range = doc_text_cast(base); xui_context context;
    xui_doc_range_t mapped; uint64_t first, last; int result;
    result = doc_text_begin(range, &context, 1);
    if (result == XUI_OK && range->owner->desc.bDisableSelection) result = XUI_ERROR_UNSUPPORTED;
    if (result == XUI_OK) result = doc_text_project(range, &mapped, &first, &last);
    if (result == XUI_OK) result = xuiDocumentViewSetSelection(range->owner->widget, &mapped);
    return doc_text_end(context, result);
}
static int doc_text_scroll(xui_internal_accessible_text_range* base, int align_top)
{
    doc_accessible_text_range* range = doc_text_cast(base); xui_context context;
    doc_view_data* view; xui_doc_range_t mapped; uint64_t first, last;
    xui_doc_rect_t rect; xui_rect_t content; double x, y; int result;
    result = doc_text_begin(range, &context, 1);
    if (result != XUI_OK) return result;
    view = range->owner;
    result = doc_text_project(range, &mapped, &first, &last);
    if (result == XUI_OK) result = doc_view_layout(view);
    if (result == XUI_OK) result = xuiDocumentRendererGetCaretRect(view->renderer,
        align_top ? &mapped.tAnchor : &mapped.tCaret, &rect);
    if (result == XUI_OK) {
        content = xuiWidgetGetContentRect(view->widget); x = view->scroll_x;
        if (rect.x < x) x = rect.x;
        else if (rect.x + rect.width > x + content.fW) x = rect.x + rect.width - content.fW;
        y = align_top ? rect.y : rect.y + rect.height - content.fH;
        result = xuiDocumentViewSetScroll(view->widget, fmax(0, x), fmax(0, y));
    }
    return doc_text_end(context, result);
}
#include "xui_document_accessible_navigation.inl"
static const xui_internal_accessible_text_ops doc_text_ops = {
    .retain = doc_text_retain, .release = doc_text_release,
    .clone = doc_text_clone, .getOffsets = doc_text_offsets,
    .setOffsets = doc_text_set_offsets, .getText = doc_text_get_text,
    .getRects = doc_text_rects, .select = doc_text_select,
    .scrollIntoView = doc_text_scroll, .compare = doc_text_compare,
    .compareEndpoints = doc_text_compare_endpoints, .setEndpoint = doc_text_set_endpoint,
    .findText = doc_text_find, .expand = doc_text_expand,
    .move = doc_text_move, .moveEndpoint = doc_text_move_endpoint
};
static int doc_view_accessible_text_create(xui_widget widget, uint64_t id,
    const xui_internal_accessible_text_request* request,
    xui_internal_accessible_text_range** out)
{
    doc_view_data* view = doc_view_get(widget); xui_doc_range_t range;
    doc_plain_projection* projection; xui_rect_t world; int result;
    *out = NULL;
    if (!view || id != DOC_ROOT) return XUI_ERROR_UNSUPPORTED;
    result = doc_view_project_edit(view); if (result != XUI_OK) return result;
    projection = &view->edit_projection;
    if (request->kind == XUI_INTERNAL_TEXT_DOCUMENT) {
        range.tAnchor = doc_plain_unproject(projection, 0, 1);
        range.tCaret = doc_plain_unproject(projection, projection->bytes, 0);
        if (!projection->bytes) range.tCaret = range.tAnchor;
    } else if (request->kind == XUI_INTERNAL_TEXT_SELECTION || request->kind == XUI_INTERNAL_TEXT_CARET) {
        if (view->desc.bDisableSelection) return XUI_ERROR_UNSUPPORTED;
        range = view->selection;
        if (request->kind == XUI_INTERNAL_TEXT_CARET) range.tAnchor = range.tCaret;
    } else if (request->kind == XUI_INTERNAL_TEXT_CHILD) {
        doc_node* node = doc_index_get(view->renderer->snapshot->state->index, request->child);
        uint64_t a, b;
        if (view->renderer->mode != XUI_DOC_VISUAL || !node || node->id == DOC_ROOT)
            return XUI_ERROR_INVALID_ARGUMENT;
        result = doc_selectable_object_kind(node->kind) ?
            doc_accessible_snapshot_object_range(view->renderer->snapshot, node, &range) :
            doc_accessible_snapshot_selection_range(view->renderer->snapshot, node, NULL, &range);
        if (result != XUI_OK) return result;
        result = doc_plain_project_position(projection, &range.tAnchor, &a);
        if (result == XUI_OK) result = doc_plain_project_position(projection, &range.tCaret, &b);
        if (result != XUI_OK) return result;
    } else if (request->kind == XUI_INTERNAL_TEXT_POINT) {
        if (!isfinite(request->x) || !isfinite(request->y)) return XUI_ERROR_INVALID_ARGUMENT;
        world = xuiWidgetGetWorldRect(widget);
        result = xuiDocumentViewHitTest(widget, request->x - world.fX, request->y - world.fY, &range.tAnchor);
        if (result != XUI_OK) return result;
        range.tCaret = range.tAnchor;
    } else return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_view_map_range(view, view->renderer->mode, &range);
    if (result != XUI_OK) return result;
    return doc_text_new(view, &range, out);
}
