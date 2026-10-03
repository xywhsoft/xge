#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT_VIEW
#include "xui_document_layout_internal.h"
#include "xui_text_internal.h"
#include <math.h>
#include <float.h>
#include <stdio.h>

/* Image sources may name a surface in the owning XUI context. The registry
 * owns the handle; resolve it for each operation so replacement/removal never
 * leaves a cached dangling surface in the renderer. */
static int doc_render_image_surface(xui_document_renderer r, doc_node* n,
    xui_surface* surface, xui_surface_desc_t* desc)
{
    xui_resource resource;
    const char* name = doc_string(n->resource);
    if (!name || !name[0] || !r->proxy->surfaceGetDesc) return XUI_ERROR_UNSUPPORTED;
    resource = xuiResourceFind(r->context, name);
    if (!resource || xuiResourceGetKind(resource) != XUI_RESOURCE_SURFACE) return XUI_ERROR_UNSUPPORTED;
    *surface = (xui_surface)xuiResourceGetHandle(resource);
    if (!*surface || r->proxy->surfaceGetDesc(r->proxy, *surface, desc) != XUI_OK ||
        desc->iWidth <= 0 || desc->iHeight <= 0) return XUI_ERROR_UNSUPPORTED;
    return XUI_OK;
}
int doc_render_image_measure(xui_document_renderer r, doc_node* n, double available, xui_vec2_t* size)
{
    xui_surface surface; xui_surface_desc_t desc = {0};
    double width, height, zoom = r->desc.fZoom;
    int result;
    r->has_named_images = 1;
    result = doc_render_image_surface(r, n, &surface, &desc);
    if (result != XUI_OK) return result;
    width = n->attrs->fWidth > 0 ? n->attrs->fWidth : desc.iWidth;
    height = n->attrs->fHeight > 0 ? n->attrs->fHeight : desc.iHeight;
    if (n->attrs->fWidth > 0 && n->attrs->fHeight == 0) height *= width / desc.iWidth;
    if (n->attrs->fHeight > 0 && n->attrs->fWidth == 0) width *= height / desc.iHeight;
    width *= zoom; height *= zoom;
    if (width > available) { height *= available / width; width = available; }
    if (!isfinite(width) || !isfinite(height) || width <= 0 || height <= 0 ||
        width > FLT_MAX || height > FLT_MAX) return XUI_DOC_ERROR_LIMIT;
    size->fX = (float)width; size->fY = (float)height;
    return XUI_OK;
}
int doc_render_image_draw(xui_document_renderer r, doc_node* n, xui_draw_context draw, xui_rect_t bounds)
{
    xui_surface surface; xui_surface_desc_t desc = {0};
    int result = doc_render_image_surface(r, n, &surface, &desc);
    if (result != XUI_OK || !r->proxy->drawSurface) return XUI_ERROR_UNSUPPORTED;
    return r->proxy->drawSurface(r->proxy, draw, surface,
        (xui_rect_t){0, 0, (float)desc.iWidth, (float)desc.iHeight}, bounds, XUI_COLOR_WHITE, 0);
}
static int doc_render_image_placeholder(xui_document_renderer r, doc_node* n,
    xui_draw_context draw, xui_rect_t bounds)
{
    char alt[128];
    uint64_t bytes = n ? doc_seq_size(n->text) : 0;
    int result;
    if (r->desc.iImagePlaceholderColor && r->proxy->drawRectFill) {
        result = r->proxy->drawRectFill(r->proxy, draw, bounds,
            r->desc.iImagePlaceholderColor);
        if (result != XUI_OK) return result;
    }
    if (r->desc.iImageBorderColor && r->proxy->drawRectStroke) {
        result = r->proxy->drawRectStroke(r->proxy, draw, bounds, 1,
            r->desc.iImageBorderColor);
        if (result != XUI_OK) return result;
    }
    if (!bytes || !r->proxy->drawText || !r->desc.iImageTextColor) return XUI_OK;
    if (bytes >= sizeof(alt)) bytes = sizeof(alt) - 1;
    result = doc_seq_read(n->text, 0, alt, bytes);
    if (result != XUI_OK) return result;
    while (bytes && !doc_utf8(alt, bytes)) bytes--;
    if (!bytes) return XUI_OK;
    alt[bytes] = 0;
    return r->proxy->drawText(r->proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=r->desc.tFonts.normal, .sText=alt, .iTextSize=-1, .sLanguage=doc_effective_text_attrs(r->snapshot->state,n).sLanguage, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, bounds, r->desc.iImageTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
}

/* SOURCE rows keep layout ownership in an implicit treap. A source edit only
 * replaces its neighboring rows; suffix byte positions shift lazily. VISUAL
 * and LIVE retain their separate flat directory. */
struct doc_source_row {
    struct doc_source_row *left, *right;
    doc_render_block block;
    uint64_t priority;
    size_t count, unmeasured, unsettled_height, cache_bytes;
    double height, width;
    int64_t shift;
};
static size_t doc_rows_count(doc_source_row* n) { return n ? n->count : 0; }
static double doc_rows_height(doc_source_row* n) { return n ? n->height : 0; }
static double doc_rows_width(doc_source_row* n) { return n ? n->width : 0; }
static size_t doc_rows_unmeasured(doc_source_row* n) { return n ? n->unmeasured : 0; }
static size_t doc_rows_unsettled_height(doc_source_row* n) { return n ? n->unsettled_height : 0; }
static size_t doc_rows_cache_bytes(doc_source_row* n) { return n ? n->cache_bytes : 0; }
static void doc_rows_update(doc_source_row* n)
{
    if (!n) return;
    n->count = doc_rows_count(n->left) + 1 + doc_rows_count(n->right);
    n->height = doc_rows_height(n->left) + n->block.height + doc_rows_height(n->right);
    n->width = fmax(n->block.width, fmax(doc_rows_width(n->left), doc_rows_width(n->right)));
    n->unmeasured = doc_rows_unmeasured(n->left) +
        (!n->block.measured || n->block.line_partial) +
        doc_rows_unmeasured(n->right);
    n->unsettled_height = doc_rows_unsettled_height(n->left) +
        (n->block.height_uncertain && !n->block.ever_measured) +
        doc_rows_unsettled_height(n->right);
    n->cache_bytes = doc_rows_cache_bytes(n->left) + n->block.cache_bytes + doc_rows_cache_bytes(n->right);
}
static uint64_t doc_rows_shift_offset(uint64_t value, int64_t shift)
{
    return shift >= 0 ? value + (uint64_t)shift : value - (uint64_t)(-shift);
}
static void doc_rows_apply(doc_source_row* n, int64_t shift)
{
    size_t i;
    if (!n || !shift) return;
    n->block.source_start = doc_rows_shift_offset(n->block.source_start, shift);
    n->block.source_end = doc_rows_shift_offset(n->block.source_end, shift);
    n->block.text_start = doc_rows_shift_offset(n->block.text_start, shift);
    n->block.text_end = doc_rows_shift_offset(n->block.text_end, shift);
    if (n->block.line_partial)
        n->block.line_cutoff = doc_rows_shift_offset(n->block.line_cutoff, shift);
    for (i = 0; i < n->block.fragment_count; i++) {
        n->block.fragments[i].start = doc_rows_shift_offset(n->block.fragments[i].start, shift);
        n->block.fragments[i].end = doc_rows_shift_offset(n->block.fragments[i].end, shift);
    }
    for (i = 0; i < n->block.run_count; i++)
        n->block.runs[i].base_offset = doc_rows_shift_offset(n->block.runs[i].base_offset, shift);
    n->shift += shift;
}
static void doc_rows_push(doc_source_row* n)
{
    if (!n || !n->shift) return;
    doc_rows_apply(n->left, n->shift);
    doc_rows_apply(n->right, n->shift);
    n->shift = 0;
}
static doc_source_row* doc_rows_join(doc_source_row* left, doc_source_row* right)
{
    if (!left) return right;
    if (!right) return left;
    if (left->priority <= right->priority) {
        doc_rows_push(left);
        left->right = doc_rows_join(left->right, right);
        doc_rows_update(left);
        return left;
    }
    doc_rows_push(right);
    right->left = doc_rows_join(left, right->left);
    doc_rows_update(right);
    return right;
}
static void doc_rows_split(doc_source_row* n, size_t count, doc_source_row** left, doc_source_row** right)
{
    size_t before;
    if (!n) { *left = *right = NULL; return; }
    doc_rows_push(n);
    before = doc_rows_count(n->left);
    if (count <= before) {
        doc_rows_split(n->left, count, left, &n->left);
        doc_rows_update(n); *right = n;
    } else {
        doc_rows_split(n->right, count - before - 1, &n->right, right);
        doc_rows_update(n); *left = n;
    }
}
static doc_source_row* doc_rows_at(doc_source_row* n, size_t index)
{
    while (n) {
        size_t before = doc_rows_count(n->left);
        doc_rows_push(n);
        if (index < before) n = n->left;
        else if (index == before) return n;
        else { index -= before + 1; n = n->right; }
    }
    return NULL;
}
static size_t doc_rows_source_at(doc_source_row* n, uint64_t offset)
{
    size_t base = 0, found = 0;
    while (n) {
        size_t before = doc_rows_count(n->left);
        doc_rows_push(n);
        if (n->block.source_start <= offset) {
            found = base + before;
            base += before + 1; n = n->right;
        } else n = n->left;
    }
    return found;
}
static double doc_rows_height_before(doc_source_row* n, size_t index)
{
    double height = 0;
    while (n && index) {
        size_t before = doc_rows_count(n->left);
        if (index <= before) n = n->left;
        else {
            height += doc_rows_height(n->left) + n->block.height;
            index -= before + 1; n = n->right;
        }
    }
    return height;
}
static size_t doc_rows_first_unsettled_before(doc_source_row* n,
    size_t limit, size_t base)
{
    size_t at, found;
    if (!n || !n->unsettled_height || base >= limit) return SIZE_MAX;
    at = base + doc_rows_count(n->left);
    found = doc_rows_first_unsettled_before(n->left, limit, base);
    if (found != SIZE_MAX) return found;
    if (at < limit && n->block.height_uncertain && !n->block.ever_measured)
        return at;
    return doc_rows_first_unsettled_before(n->right, limit, at + 1);
}
static size_t doc_rows_at_y(doc_source_row* n, double y)
{
    size_t base = 0, count = doc_rows_count(n);
    while (n) {
        double before = doc_rows_height(n->left);
        size_t rows = doc_rows_count(n->left);
        if (y < before) n = n->left;
        else if (y < before + n->block.height) return base + rows;
        else { y -= before + n->block.height; base += rows + 1; n = n->right; }
    }
    return count ? count - 1 : 0;
}
static void doc_rows_refresh(doc_source_row* n, size_t index)
{
    size_t before;
    if (!n) return;
    before = doc_rows_count(n->left);
    if (index < before) doc_rows_refresh(n->left, index);
    else if (index > before) doc_rows_refresh(n->right, index - before - 1);
    doc_rows_update(n);
}
static void doc_rows_unmeasure(doc_source_row* n, double line_height)
{
    if (!n) return;
    doc_rows_unmeasure(n->left, line_height);
    doc_render_block_free(&n->block);
    n->block.height = line_height;
    n->block.ever_measured = 0;
    doc_rows_unmeasure(n->right, line_height);
    doc_rows_update(n);
}
static void doc_rows_free(doc_source_row* n)
{
    if (!n) return;
    doc_rows_free(n->left); doc_rows_free(n->right);
    doc_render_block_free(&n->block); free(n);
}
static void doc_render_release_block(xui_document_renderer r, doc_render_block* b)
{
    size_t bytes = b->cache_bytes;
    doc_render_block_free(b);
    r->cache_bytes -= bytes;
    r->size_dirty = 1;
}
static void doc_rows_trim_cache(xui_document_renderer r, doc_source_row* n, double y,
    double top, double bottom, int reverse)
{
    double row_y;
    if (!n || !n->cache_bytes || r->cache_bytes <= r->desc.iLayoutCacheBudgetBytes) return;
    row_y = y + doc_rows_height(n->left);
    if (reverse) doc_rows_trim_cache(r, n->right, row_y + n->block.height, top, bottom, reverse);
    else doc_rows_trim_cache(r, n->left, y, top, bottom, reverse);
    if (n->block.cache_bytes && r->cache_bytes > r->desc.iLayoutCacheBudgetBytes &&
        (row_y + n->block.height <= top || row_y >= bottom))
        { doc_render_release_block(r, &n->block); r->stats.iLayoutCacheEvictions++; }
    if (reverse) doc_rows_trim_cache(r, n->left, y, top, bottom, reverse);
    else doc_rows_trim_cache(r, n->right, row_y + n->block.height, top, bottom, reverse);
    doc_rows_update(n);
}
static uint64_t doc_rows_priority(uint64_t serial)
{
    serial += UINT64_C(0x9e3779b97f4a7c15);
    serial = (serial ^ (serial >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    serial = (serial ^ (serial >> 27)) * UINT64_C(0x94d049bb133111eb);
    return serial ^ (serial >> 31);
}
static doc_render_block* doc_render_block_at_index(xui_document_renderer r, size_t index)
{
    doc_source_row* row;
    if (index >= r->count) return NULL;
    if (r->mode != XUI_DOC_SOURCE_TEXT) return &r->blocks[index];
    row = doc_rows_at(r->source_rows, index);
    return row ? &row->block : NULL;
}

doc_sequence* doc_render_source(xui_document_renderer r)
{
    return r->input ? doc_prepare_source_store(r->input) : r->snapshot->state->source;
}
static int doc_render_live_base_to_input(xui_document_renderer r, uint64_t base, uint64_t* input)
{
    uint64_t old_size = doc_seq_size(r->snapshot->state->source);
    uint64_t new_size = doc_seq_size(doc_render_source(r));
    if (base < r->live_base_end) { *input = base; return XUI_OK; }
    if (new_size >= old_size) {
        if (base > UINT64_MAX - (new_size - old_size)) return XUI_DOC_ERROR_LIMIT;
        *input = base + new_size - old_size;
    } else {
        if (base < old_size - new_size) return XUI_DOC_ERROR_STALE;
        *input = base - (old_size - new_size);
    }
    return XUI_OK;
}
static int doc_render_live_input_to_base(xui_document_renderer r, uint64_t input, uint64_t* base)
{
    uint64_t old_size = doc_seq_size(r->snapshot->state->source);
    uint64_t new_size = doc_seq_size(doc_render_source(r));
    if (r->live_pending_full || input < r->live_start) { *base = input; return XUI_OK; }
    if (input < r->live_end) return XUI_DOC_ERROR_STALE;
    if (new_size >= old_size) {
        if (input < new_size - old_size) return XUI_DOC_ERROR_STALE;
        *base = input - (new_size - old_size);
    } else {
        if (input > UINT64_MAX - (old_size - new_size)) return XUI_DOC_ERROR_LIMIT;
        *base = input + old_size - new_size;
    }
    return XUI_OK;
}
int doc_render_live_position_source(xui_document_renderer r,
    const xui_doc_position_t* p, uint64_t* source)
{
    int mapping, result;
    if (p->iKind == XUI_DOC_POSITION_SOURCE) {
        if (!doc_prepare_position_valid(r->input, p)) return XUI_DOC_ERROR_STALE;
        *source = p->iOffset; return XUI_OK;
    }
    if (r->live_pending_full || p->iInputGeneration ||
        p->iDocumentId != r->snapshot->identity || p->iRevision != r->snapshot->revision)
        return XUI_DOC_ERROR_STALE;
    result = xuiDocumentPositionToSource(r->snapshot, p, source, &mapping);
    if (result != XUI_OK) return result;
    if (*source >= r->live_start && *source < r->live_base_end) return XUI_DOC_ERROR_STALE;
    return doc_render_live_base_to_input(r, *source, source);
}
int doc_render_compare(xui_document_renderer r, const xui_doc_position_t* a, const xui_doc_position_t* b, int* order)
{
    uint64_t left, right; int result;
    if (!r || !r->snapshot || !a || !b || !order) return XUI_ERROR_INVALID_ARGUMENT;
    if (!r->input) return xuiDocumentSnapshotComparePositions(r->snapshot, a, b, order);
    if (r->mode == XUI_DOC_LIVE_MARKDOWN) {
        result = doc_render_live_position_source(r, a, &left); if (result != XUI_OK) return result;
        result = doc_render_live_position_source(r, b, &right); if (result != XUI_OK) return result;
        *order = left < right ? -1 : left != right; return XUI_OK;
    }
    if (!doc_prepare_position_valid(r->input, a) || !doc_prepare_position_valid(r->input, b)) return XUI_DOC_ERROR_STALE;
    *order = a->iOffset < b->iOffset ? -1 : a->iOffset != b->iOffset; return XUI_OK;
}
int doc_render_copy_range(xui_document_renderer r, const xui_doc_range_t* range, char** text, uint64_t* bytes)
{
    int order, result; uint64_t start, end;
    if (!r->input) return xuiDocumentSnapshotCopyRange(r->snapshot, range, text, bytes);
    *text = NULL; *bytes = 0;
    result = doc_render_compare(r, &range->tAnchor, &range->tCaret, &order); if (result != XUI_OK) return result;
    if (r->mode == XUI_DOC_LIVE_MARKDOWN) {
        result = doc_render_live_position_source(r, order < 0 ? &range->tAnchor : &range->tCaret, &start);
        if (result == XUI_OK) result = doc_render_live_position_source(r, order < 0 ? &range->tCaret : &range->tAnchor, &end);
        if (result != XUI_OK) return result;
    } else {
        start = (order < 0 ? range->tAnchor : range->tCaret).iOffset;
        end = (order < 0 ? range->tCaret : range->tAnchor).iOffset;
    }
    if (end - start >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    *text = malloc((size_t)(end - start) + 1); if (!*text) return XUI_ERROR_OUT_OF_MEMORY;
    *bytes = end - start; doc_seq_read(doc_render_source(r), start, *text, *bytes); (*text)[*bytes] = 0; return XUI_OK;
}
static void doc_render_clear(xui_document_renderer r)
{
    size_t i;
    if (r->blocks) for (i = 0; i < r->count; i++) doc_render_block_free(&r->blocks[i]);
    free(r->blocks); free(r->heights); doc_rows_free(r->source_rows);
    r->blocks = NULL; r->heights = NULL; r->source_rows = NULL; r->count = r->capacity = 0;
    r->cache_bytes = 0;
    r->size_width = 0; r->size_exact = 1; r->size_dirty = 1;
    r->partial_hint_valid = 0;
    r->source_height_hint_valid = 0;
    r->has_named_images = 0;
    xrtMapClear(&r->block_index);
}
static double doc_render_code_line_height(xui_document_renderer r, doc_node* node)
{
    xui_doc_attributes_t attrs = {0};
    xui_font_metrics_t metrics = {0}, selected_metrics = {0};
    xui_font font = r->desc.tFonts.monospace ? r->desc.tFonts.monospace : r->desc.tFonts.normal;
    double scale = r->desc.fZoom;
    float size;
    if (node) attrs = doc_effective_text_attrs(r->snapshot->state, node);
    attrs.iMarks |= XUI_DOC_CODE;
    if (r->proxy && r->proxy->fontGetMetrics && font)
        r->proxy->fontGetMetrics(r->proxy, font, &metrics);
    size = attrs.fFontSize > 0 ? attrs.fFontSize : metrics.fSize > 0 ? metrics.fSize : 16;
    size *= r->desc.fZoom;
    if (r->desc.onFont && r->proxy && r->proxy->fontGetMetrics) {
        xui_font selected = r->desc.onFont(r->context, attrs.sFontFamily,
            attrs.iMarks, size, r->desc.pUser);
        if (selected && r->proxy->fontGetMetrics(r->proxy, selected,
                &selected_metrics) == XUI_OK && selected_metrics.fLineHeight > 0)
            return selected_metrics.fLineHeight;
    }
    if (metrics.fLineHeight > 0) {
        if (attrs.fFontSize > 0 && metrics.fSize > 0)
            scale *= attrs.fFontSize / metrics.fSize;
        return metrics.fLineHeight * scale;
    }
    return 20 * scale;
}
static int doc_render_plain_ascii_line(doc_sequence* text,
    uint64_t start, uint64_t end, int* plain)
{
    char chunk[4096];
    uint64_t at;
    *plain = 0;
    if (end - start <= 8192) return XUI_OK;
    *plain = 1;
    for (at = start; at < end && *plain; ) {
        uint64_t n = end - at < sizeof(chunk) ? end - at :
            sizeof(chunk), i;
        int result = doc_seq_read(text, at, chunk, n);
        if (result != XUI_OK) return result;
        for (i = 0; i < n; i++)
            if ((unsigned char)chunk[i] < 0x20 ||
                (unsigned char)chunk[i] > 0x7e) {
                *plain = 0;
                break;
            }
        at += n;
    }
    return XUI_OK;
}
static int doc_render_code_slice(xui_document_renderer r, doc_node* node, double indent,
    uint64_t start, uint64_t end, uint64_t lines, double line_height)
{
    doc_render_block* b;
    int result;
    if (r->count >= 1000000 || lines > 1000000) return XUI_DOC_ERROR_LIMIT;
    result = doc_render_reserve((void**)&r->blocks, &r->capacity, r->count + 1, sizeof(*r->blocks));
    if (result != XUI_OK) return result;
    b = &r->blocks[r->count++]; memset(b, 0, sizeof(*b));
    b->node = node->id; b->indent = indent; b->code_slice = 1;
    b->text_start = start; b->text_end = end;
    b->height = lines * (line_height + r->desc.fLineGap * r->desc.fZoom);
    /* Protected scripts and tabs retain whole-line shaping. */
    xui_doc_attributes_t attrs = doc_effective_text_attrs(r->snapshot->state, node);
    return lines == 1 && !attrs.sLanguage ? doc_render_plain_ascii_line(node->text,
        start, end, &b->line_lazy) : XUI_OK;
}
static int doc_render_collect_code(xui_document_renderer r, doc_node* node, double indent)
{
    char buffer[4099];
    uint64_t bytes = doc_seq_size(node->text), offset = 0, start = 0, lines = 0, last_break = 0;
    size_t first = r->count;
    double line_height = doc_render_code_line_height(r, node);
    int result = XUI_OK;
    while (offset < bytes) {
        uint64_t n = bytes - offset < 4096 ? bytes - offset : 4096;
        uint64_t read = bytes - offset < sizeof(buffer) ? bytes - offset : sizeof(buffer), consumed = n;
        int scan = 0;
        /* Three lookahead bytes cover every mandatory UTF-8 control and a
         * CRLF crossing the chunk edge. Only controls starting in n are used. */
        result = doc_seq_read(node->text, offset, buffer, read); if (result != XUI_OK) return result;
        while ((uint64_t)scan < n) {
            int line_end, next; uint64_t end;
            result = xuiInternalTextNextHardLine(buffer, (int)read, scan, &line_end, &next);
            if (result != XUI_OK) return result;
            if ((uint64_t)line_end >= n) break;
            end = offset + (uint64_t)next; last_break = end; lines++;
            if (end - start >= 8192) {
                result = doc_render_code_slice(r, node, indent, start, end, lines, line_height);
                if (result != XUI_OK) return result;
                start = end; lines = 0;
            }
            scan = next;
            if ((uint64_t)next > consumed) consumed = (uint64_t)next;
        }
        offset += consumed;
    }
    if (start < bytes || first == r->count) {
        result = doc_render_code_slice(r, node, indent, start, bytes,
            lines + 1, line_height);
        if (result != XUI_OK) return result;
    } else if (last_break == bytes) {
        /* EOF exactly on a slice cut still owns one trailing empty line. */
        r->blocks[r->count - 1].height += line_height + r->desc.fLineGap * r->desc.fZoom;
    }
    r->blocks[first].slice_count = r->count - first;
    r->blocks[r->count - 1].slice_last = 1;
    r->blocks[r->count - 1].height -= r->desc.fLineGap * r->desc.fZoom;
    r->blocks[r->count - 1].height += ((node->attrs->iFlags & XUI_DOC_SPACING_EXPLICIT) || node->attrs->fParagraphSpacing > 0 ?
        node->attrs->fParagraphSpacing : r->desc.fParagraphGap) * r->desc.fZoom;
    return xrtMapSet(&r->block_index, xuiXrtBytes(&node->id, sizeof(node->id)), &first) ? XUI_OK : XUI_ERROR_OUT_OF_MEMORY;
}
static int doc_render_collect_object_dependencies(xui_document_renderer r, doc_node* node)
{
    uint64_t i; int dependent = 0;
    if (node->kind == XUI_DOC_IMAGE) {
        const char* resource = doc_string(node->resource);
        if (resource && resource[0]) r->has_named_images = 1;
        dependent = 1;
    } else if (node->kind == XUI_DOC_HTML || node->kind == XUI_DOC_MATH ||
        node->kind == XUI_DOC_DIAGRAM || node->kind == XUI_DOC_EXTENSION) dependent = 1;
    for (i = 0; i < doc_seq_size(node->children); i++) {
        doc_node* child = doc_index_get(r->snapshot->state->index, doc_seq_get_id(node->children, i));
        if (child && doc_render_collect_object_dependencies(r, child)) dependent = 1;
    }
    return dependent;
}
static int doc_render_collect(xui_document_renderer r, uint64_t id, double indent)
{
    doc_node* n = doc_index_get(r->snapshot->state->index, id); uint64_t i;
    size_t first = r->count; int result;
    if (!n) return XUI_ERROR_NOT_FOUND;
    if (n->kind == XUI_DOC_ROOT || n->kind == XUI_DOC_QUOTE || n->kind == XUI_DOC_LIST || n->kind == XUI_DOC_LIST_ITEM || n->kind == XUI_DOC_FOOTNOTE) {
        if (n->kind == XUI_DOC_QUOTE || n->kind == XUI_DOC_LIST_ITEM) indent += r->desc.fIndent * r->desc.fZoom;
        if (n->kind == XUI_DOC_QUOTE && n->info && n->info->size) {
            doc_render_block* title;
            result = doc_render_reserve((void**)&r->blocks, &r->capacity,
                r->count + 1, sizeof(*r->blocks));
            if (result != XUI_OK) return result;
            title = &r->blocks[r->count]; memset(title, 0, sizeof(*title));
            title->node = id; title->indent = indent; title->admonition_title = 1;
            title->height = 24 * r->desc.fZoom;
            if (!xrtMapSet(&r->block_index,
                xuiXrtBytes(&id, sizeof(id)), &r->count)) return XUI_ERROR_OUT_OF_MEMORY;
            r->count++;
        }
        for (i = 0; i < doc_seq_size(n->children); i++) {
            result = doc_render_collect(r, doc_seq_get_id(n->children, i), indent);
            if (result != XUI_OK) return result;
        }
        if (n->kind == XUI_DOC_LIST_ITEM && first < r->count) {
            doc_render_block* b = &r->blocks[first]; doc_node* parent = doc_index_get(r->snapshot->state->index, n->parent);
            b->marker = id;
            if (n->attrs->iFlags & XUI_DOC_TASK) strcpy(b->marker_text, n->attrs->iFlags & XUI_DOC_CHECKED ? "[x]" : "[ ]");
            else if (parent->attrs->iFlags & XUI_DOC_ORDERED) snprintf(b->marker_text, sizeof(b->marker_text), "%llu.",
                (unsigned long long)(parent->attrs->iListStart + doc_child_index(parent, id)));
            else strcpy(b->marker_text, "\xe2\x80\xa2");
        }
        return XUI_OK;
    }
    if (n->kind == XUI_DOC_CODE_BLOCK && doc_seq_size(n->text) > 8192)
        return doc_render_collect_code(r, n, indent);
    result = doc_render_reserve((void**)&r->blocks, &r->capacity, r->count + 1, sizeof(*r->blocks));
    if (result != XUI_OK) return result;
    memset(&r->blocks[r->count], 0, sizeof(*r->blocks));
    r->blocks[r->count].node = id; r->blocks[r->count].indent = indent;
    r->blocks[r->count].object_dependent = doc_render_collect_object_dependencies(r, n);
    r->blocks[r->count].height = (24 + r->desc.fParagraphGap) * r->desc.fZoom;
    if (!xrtMapSet(&r->block_index, xuiXrtBytes(&id, sizeof(id)), &r->count)) return XUI_ERROR_OUT_OF_MEMORY;
    r->count++; return XUI_OK;
}
static int doc_render_source_line(xui_document_renderer r, uint64_t start,
    uint64_t end, double line_height, int height_uncertain)
{
    doc_render_block* b; int result, lazy;
    if (r->count >= 1000000) return XUI_DOC_ERROR_LIMIT;
    result = doc_render_plain_ascii_line(doc_render_source(r),
        start, end, &lazy);
    if (result != XUI_OK) return result;
    if (r->desc.bSourceLineHeightMayVary ||
        doc_index_get(r->snapshot->state->index, DOC_ROOT)->attrs->sLanguage) {
        /* A content-dependent shaper can make even printable ASCII taller
         * than font metrics. Prefix layout cannot establish a long row's
         * height when the decisive glyph is in a later chunk. */
        height_uncertain = 1;
        lazy = 0;
    }
    if (r->mode == XUI_DOC_SOURCE_TEXT) {
        doc_source_row* row = calloc(1, sizeof(*row));
        if (!row) return XUI_ERROR_OUT_OF_MEMORY;
        row->priority = doc_rows_priority(++r->source_row_serial);
        b = &row->block; b->node = DOC_ROOT; b->source_line = 1;
        b->source_start = start; b->source_end = end; b->height = line_height;
        b->text_start = start; b->text_end = end; b->line_lazy = lazy;
        b->height_uncertain = height_uncertain;
        doc_rows_update(row);
        r->source_rows = doc_rows_join(r->source_rows, row); r->count++;
        r->stats.iSourceRowsCreated++;
        return XUI_OK;
    }
    result = doc_render_reserve((void**)&r->blocks, &r->capacity, r->count + 1, sizeof(*r->blocks));
    if (result != XUI_OK) return result;
    b = &r->blocks[r->count++]; memset(b, 0, sizeof(*b)); b->node = DOC_ROOT;
    b->source_line = 1;
    b->source_start = start; b->source_end = end; b->height = line_height;
    b->text_start = start; b->text_end = end; b->line_lazy = lazy;
    b->height_uncertain = height_uncertain;
    if (height_uncertain && !r->source_height_hint_valid) {
        r->source_height_hint = r->count - 1;
        r->source_height_hint_valid = 1;
    }
    return XUI_OK;
}
static int doc_render_collect_source_range(xui_document_renderer r, uint64_t first, uint64_t size)
{
    char buffer[4096]; uint64_t offset = first, line = first; int result;
    double line_height = doc_render_code_line_height(r, NULL);
    char previous = 0; int height_uncertain = 0;
    while (offset < size) {
        uint64_t i, bytes = size - offset < sizeof(buffer) ? size - offset : sizeof(buffer);
        doc_seq_read(doc_render_source(r), offset, buffer, bytes);
        r->stats.iSourceBytesScanned += bytes;
        for (i = 0; i < bytes; i++) {
            /* Fallback glyphs or control shaping can change the row height;
             * settle these rows before using their estimated prefix height. */
            if (buffer[i] == '\r' || buffer[i] == '\n') {
                if (buffer[i] != '\n' || previous != '\r') {
                    result = doc_render_source_line(r, line, offset + i,
                        line_height, height_uncertain);
                    if (result != XUI_OK) return result;
                }
                line = offset + i + 1;
                height_uncertain = 0;
            } else if ((unsigned char)buffer[i] < 0x20 ||
                (unsigned char)buffer[i] > 0x7e) height_uncertain = 1;
            previous = buffer[i];
        }
        offset += bytes;
    }
    /* Include a terminal empty line only at EOF. At an interior partition it
     * would duplicate the first line of the following rendered container. */
    return line < size || first == size || size == doc_seq_size(doc_render_source(r)) ?
        doc_render_source_line(r, line, size, line_height,
            height_uncertain) : XUI_OK;
}
static double* doc_render_build_heights(const doc_render_block* blocks, size_t count)
{
    double* heights = calloc(count + 1, sizeof(*heights)); size_t i;
    if (heights) for (i = 1; i <= count; i++) {
        size_t parent = i + (i & (~i + 1));
        heights[i] += blocks[i - 1].height;
        if (parent <= count) heights[parent] += heights[i];
    }
    return heights;
}
static size_t doc_render_source_at(xui_document_renderer r, uint64_t offset)
{
    size_t low = 0, high = r->count;
    if (r->mode == XUI_DOC_SOURCE_TEXT) return doc_rows_source_at(r->source_rows, offset);
    while (low < high) { size_t mid = low + (high - low) / 2; if (r->blocks[mid].source_start <= offset) low = mid + 1; else high = mid; }
    return low ? low - 1 : 0;
}
/* Rescan the edited lines plus context on both sides, so joining/splitting a
 * CRLF (including at EOF) cannot affect a reused line. All allocations finish
 * before changing the old directory or any of its owned layout buffers. */
static int doc_render_source_splice(xui_document_renderer r, xui_document_prepare p)
{
    struct xui_document_renderer_t part = {0};
    doc_source_row *prefix, *tail, *removed_rows, *suffix;
    doc_render_block *first_block, *last_block;
    uint64_t start, end, bytes, removed, old_size = doc_seq_size(doc_render_source(r)), old_end, new_end;
    size_t first, last, count; int result;
    int64_t shift;
    if (!r->count || !r->source_rows || !doc_prepare_source_delta(p, r->input, &start, &end, &bytes)) return XUI_ERROR_NOT_FOUND;
    if (start > end || end > old_size || old_size - (end - start) > UINT64_MAX - bytes ||
        old_size - (end - start) + bytes != doc_seq_size(doc_prepare_source_store(p))) return XUI_ERROR_NOT_FOUND;
    if (start == end && !bytes) {
        r->stats.iSourceIncrementalUpdates++; r->stats.iSourceReusedBlocks += r->count; return XUI_OK;
    }
    first = doc_render_source_at(r, start); if (first) first--;
    last = doc_render_source_at(r, end) + 2; if (last > r->count) last = r->count;
    first_block = doc_render_block_at_index(r, first);
    last_block = doc_render_block_at_index(r, last);
    /* An empty EOF line belongs to this rebuild, never also to the suffix. */
    if (last_block && last_block->source_start == old_size) { last = r->count; last_block = NULL; }
    old_end = last_block ? last_block->source_start : old_size;
    removed = end - start; new_end = old_end - removed + bytes;
    if (removed > INT64_MAX || bytes > INT64_MAX || old_size > INT64_MAX ||
        doc_seq_size(doc_prepare_source_store(p)) > INT64_MAX) return XUI_ERROR_NOT_FOUND;
    shift = (int64_t)bytes - (int64_t)removed;
    part.context = r->context; part.proxy = r->proxy;
    part.snapshot = r->snapshot; part.input = p; part.desc = r->desc;
    part.mode = XUI_DOC_SOURCE_TEXT; part.source_row_serial = r->source_row_serial;
    result = doc_render_collect_source_range(&part, first_block->source_start, new_end);
    if (result != XUI_OK) { doc_rows_free(part.source_rows); return result; }
    count = first + part.count + r->count - last;
    if (count > 1000000) { doc_rows_free(part.source_rows); return XUI_DOC_ERROR_LIMIT; }
    /* All allocations have succeeded. Splits, suffix rebasing and joins are
     * allocation-free, so a failure cannot expose a half-updated directory. */
    doc_rows_split(r->source_rows, first, &prefix, &tail);
    doc_rows_split(tail, last - first, &removed_rows, &suffix);
    r->cache_bytes -= doc_rows_cache_bytes(removed_rows);
    doc_rows_free(removed_rows);
    doc_rows_apply(suffix, shift);
    r->source_rows = doc_rows_join(doc_rows_join(prefix, part.source_rows), suffix);
    r->source_row_serial = part.source_row_serial;
    r->stats.iSourceBytesScanned += part.stats.iSourceBytesScanned;
    r->stats.iSourceRowsCreated += part.stats.iSourceRowsCreated;
    r->stats.iSourceIncrementalUpdates++; r->stats.iSourceReusedBlocks += first + r->count - last;
    r->count = count;
    r->size_dirty = 1;
    return XUI_OK;
}
/* Complete syntax extents come from the parser, including empty blocks and
 * closing fences. Assign inter-block trivia to the preceding partition. */
typedef struct doc_live_partition { uint64_t start, end, node; } doc_live_partition;
static int doc_live_order(const void* a, const void* b)
{
    const doc_live_partition *x = a, *y = b;
    return x->start < y->start ? -1 : x->start != y->start;
}
static int doc_render_collect_live(xui_document_renderer r)
{
    doc_state* s = r->snapshot->state; doc_node* root = doc_index_get(s->index, DOC_ROOT);
    uint64_t count = doc_seq_size(root->children), size = doc_seq_size(s->source);
    uint64_t input_size = doc_seq_size(doc_render_source(r)), i, active = 0, previous_end = 0;
    doc_live_partition* starts = NULL; int result = XUI_OK, fallback = 0;
    if (count > SIZE_MAX / sizeof(*starts) - 1 || size >= SIZE_MAX || input_size >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    starts = calloc((size_t)count + 1, sizeof(*starts));
    if (!starts) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    for (i = 0; i < count; i++) {
        doc_node* n = doc_index_get(s->index, doc_seq_get_id(root->children, i)); uint64_t start;
        doc_node_source_range range; doc_node_source_range_get(s, n, &range);
        if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE || range.syntax_start > range.syntax_end || range.syntax_end > size) { fallback = 1; break; }
        start = range.syntax_start;
        starts[i] = (doc_live_partition){start, range.syntax_end, n->id};
    }
    if (!fallback && count) {
        qsort(starts, (size_t)count, sizeof(*starts), doc_live_order);
        for (i = 0; i < count; i++) {
            if (i && (starts[i].start <= starts[i - 1].start || starts[i].start < previous_end)) { fallback = 1; break; }
            previous_end = starts[i].end;
        }
        starts[0].start = 0;
    }
    starts[count].start = size;
    r->active_source = r->active_source < input_size ? r->active_source : input_size;
    if (!fallback && count && r->input && !r->live_pending_full) {
        for (i = 0; i < count && starts[i].node != r->live_active_node; i++) {}
        if (i == count || starts[i + 1].start != r->live_base_end ||
            starts[i].start > r->live_end || r->live_end > input_size) fallback = 1;
        else active = i;
    }
    r->live_fallback = fallback || r->live_pending_full;
    if (r->live_fallback || !count) {
        r->live_pending_full = !!r->input;
        r->live_start = 0; r->live_end = input_size;
        result = doc_render_collect_source_range(r, 0, input_size); goto done;
    }
    if (!r->input) {
        for (i = 1; i < count; i++) if (starts[i].start <= r->active_source) active = i;
        r->live_end = starts[active + 1].start;
        r->live_base_end = r->live_end;
        r->live_active_node = starts[active].node;
    }
    r->live_start = starts[active].start;
    for (i = 0; i < count && result == XUI_OK; i++) {
        if (i == active) {
            r->live_first = r->count;
            result = doc_render_collect_source_range(r, starts[i].start, r->live_end);
            r->live_rows = r->count - r->live_first;
        }
        else result = doc_render_collect(r, starts[i].node, 0);
    }
done:
    free(starts); return result;
}
static size_t doc_render_live_row_at(xui_document_renderer r, uint64_t offset)
{
    size_t low = 0, high = r->live_rows;
    while (low < high) {
        size_t mid = low + (high - low) / 2;
        if (r->blocks[r->live_first + mid].source_start <= offset) low = mid + 1;
        else high = mid;
    }
    return low ? low - 1 : 0;
}
static void doc_render_live_shift_row(doc_render_block* b, int64_t shift)
{
    size_t i;
    b->source_start = doc_rows_shift_offset(b->source_start, shift);
    b->source_end = doc_rows_shift_offset(b->source_end, shift);
    b->text_start = doc_rows_shift_offset(b->text_start, shift);
    b->text_end = doc_rows_shift_offset(b->text_end, shift);
    if (b->line_partial)
        b->line_cutoff = doc_rows_shift_offset(b->line_cutoff, shift);
    for (i = 0; i < b->fragment_count; i++) {
        b->fragments[i].start = doc_rows_shift_offset(b->fragments[i].start, shift);
        b->fragments[i].end = doc_rows_shift_offset(b->fragments[i].end, shift);
    }
    for (i = 0; i < b->run_count; i++)
        b->runs[i].base_offset = doc_rows_shift_offset(b->runs[i].base_offset, shift);
}
static void doc_render_live_splice_height_hint(xui_document_renderer r,
    const struct xui_document_renderer_t* part, size_t first)
{
    if (r->source_height_hint_valid && r->source_height_hint >= first)
        r->source_height_hint = first;
    if (part->source_height_hint_valid) {
        size_t at = first + part->source_height_hint;
        if (!r->source_height_hint_valid || at < r->source_height_hint) {
            r->source_height_hint = at;
            r->source_height_hint_valid = 1;
        }
    }
}
/* A direct successor inside the active syntax partition only changes nearby
 * source rows. Reuse all semantic blocks and untouched source layouts. Prepare
 * replacement rows first; a row-count change also builds new indices before
 * committing. */
static int doc_render_live_splice(xui_document_renderer r, xui_document_prepare p,
    uint64_t start, uint64_t end, uint64_t bytes, uint64_t candidate_end)
{
    struct xui_document_renderer_t part = {0};
    doc_render_block *new_blocks = NULL, *old_blocks = r->blocks;
    double* new_heights = NULL;
    xmap new_index = {0}; xmapiter iter; xbytesview key;
    size_t first, last, first_abs, last_abs, new_count, i, removed_cache = 0;
    size_t old_count = r->count, old_live_rows = r->live_rows;
    uint64_t old_end, new_end, old_size = doc_seq_size(doc_render_source(r));
    int64_t shift; int result = XUI_ERROR_NOT_FOUND;
    if (!r->live_rows || !r->blocks || r->live_fallback || r->live_pending_full ||
        start < r->live_start || end > r->live_end || start > end ||
        end - start > INT64_MAX || bytes > INT64_MAX || old_size > INT64_MAX ||
        doc_seq_size(doc_prepare_source_store(p)) > INT64_MAX) return result;
    shift = (int64_t)bytes - (int64_t)(end - start);
    first = doc_render_live_row_at(r, start); if (first) first--;
    last = doc_render_live_row_at(r, end) + 2;
    if (last > old_live_rows) last = old_live_rows;
    if (last < old_live_rows && r->blocks[r->live_first + last].source_start == old_size)
        last = old_live_rows;
    first_abs = r->live_first + first; last_abs = r->live_first + last;
    old_end = last < old_live_rows ? r->blocks[last_abs].source_start : r->live_end;
    new_end = old_end - (end - start) + bytes;
    part.context = r->context; part.proxy = r->proxy;
    part.snapshot = r->snapshot; part.input = p; part.desc = r->desc;
    part.mode = r->mode; part.width = r->width;
    result = doc_render_collect_source_range(&part, r->blocks[first_abs].source_start, new_end);
    if (result != XUI_OK) goto fail;
    new_count = first_abs + part.count + old_count - last_abs;
    if (!part.count || new_count > 1000000) { result = XUI_DOC_ERROR_LIMIT; goto fail; }
    if (part.count == last - first) {
        /* Most typing changes no line count. The NodeId map and Fenwick
         * directory keep their indices, so exchange only the prepared rows.
         * Everything below is allocation-free and cannot expose a partial
         * candidate on failure. */
        for (i = 0; i < part.count; i++) {
            size_t index = first_abs + i;
            double previous_height = old_blocks[index].height;
            removed_cache += old_blocks[index].cache_bytes;
            doc_render_block_free(&old_blocks[index]);
            old_blocks[index] = part.blocks[i];
            doc_render_set_height(r, index, old_blocks[index].height - previous_height);
        }
        if (shift) for (i = last_abs; i < r->live_first + old_live_rows; i++)
            doc_render_live_shift_row(&old_blocks[i], shift);
        free(part.blocks);
        r->live_end = candidate_end;
        r->cache_bytes -= removed_cache;
        r->size_dirty = 1;
        r->stats.iSourceBytesScanned += part.stats.iSourceBytesScanned;
        r->stats.iSourceRowsCreated += part.stats.iSourceRowsCreated;
        r->stats.iSourceIncrementalUpdates++;
        r->stats.iSourceReusedBlocks += old_count - (last_abs - first_abs);
        doc_render_live_splice_height_hint(r, &part, first_abs);
        xuiDocumentPrepareRetain(p); xuiDocumentPrepareRelease(r->input); r->input = p;
        return XUI_OK;
    }
    new_blocks = calloc(new_count, sizeof(*new_blocks));
    if (!new_blocks) { result = XUI_ERROR_OUT_OF_MEMORY; goto fail; }
    memcpy(new_blocks, old_blocks, first_abs * sizeof(*new_blocks));
    memcpy(new_blocks + first_abs, part.blocks, part.count * sizeof(*new_blocks));
    memcpy(new_blocks + first_abs + part.count, old_blocks + last_abs,
        (old_count - last_abs) * sizeof(*new_blocks));
    new_heights = doc_render_build_heights(new_blocks, new_count);
    if (!new_heights || !xrtMapInit(&new_index, sizeof(size_t))) {
        result = XUI_ERROR_OUT_OF_MEMORY; goto fail;
    }
    if (!xrtMapIterBegin(&r->block_index, &iter)) { result = XUI_DOC_ERROR_STALE; goto fail; }
    for (;;) {
        const size_t* old_slot = xrtMapIterNext(&iter, &key);
        size_t index;
        if (!old_slot) break;
        if (*old_slot < first_abs) index = *old_slot;
        else if (*old_slot >= last_abs) index = first_abs + part.count + (*old_slot - last_abs);
        else { result = XUI_DOC_ERROR_STALE; break; }
        if (!xrtMapSet(&new_index, key, &index)) { result = XUI_ERROR_OUT_OF_MEMORY; break; }
    }
    xrtMapIterEnd(&iter);
    if (result != XUI_OK) goto fail;
    /* No fallible operation remains. The shallow copies now take ownership of
     * the reused layout buffers; rebase retained row geometry in place. */
    for (i = first_abs; i < last_abs; i++) {
        removed_cache += old_blocks[i].cache_bytes;
        doc_render_block_free(&old_blocks[i]);
    }
    if (shift) for (i = first_abs + part.count;
        i < first_abs + part.count + old_live_rows - last; i++)
        doc_render_live_shift_row(&new_blocks[i], shift);
    free(old_blocks); free(r->heights); xrtMapUnit(&r->block_index);
    free(part.blocks);
    r->blocks = new_blocks; r->heights = new_heights; r->block_index = new_index;
    r->count = r->capacity = new_count;
    r->live_rows = first + part.count + old_live_rows - last;
    r->live_end = candidate_end;
    r->cache_bytes -= removed_cache;
    if (r->partial_hint_valid && r->partial_hint >= last_abs)
        r->partial_hint = first_abs + part.count + (r->partial_hint - last_abs);
    r->size_dirty = 1;
    r->stats.iSourceBytesScanned += part.stats.iSourceBytesScanned;
    r->stats.iSourceRowsCreated += part.stats.iSourceRowsCreated;
    r->stats.iSourceIncrementalUpdates++;
    r->stats.iSourceReusedBlocks += old_count - (last_abs - first_abs);
    doc_render_live_splice_height_hint(r, &part, first_abs);
    xuiDocumentPrepareRetain(p); xuiDocumentPrepareRelease(r->input); r->input = p;
    return XUI_OK;
fail:
    free(new_blocks); free(new_heights); xrtMapUnit(&new_index);
    for (i = 0; i < part.count; i++) doc_render_block_free(&part.blocks[i]);
    free(part.blocks);
    return result;
}
static int doc_render_empty_tail(xui_document_renderer r)
{
    doc_state* s = r->snapshot->state; uint64_t at = doc_seq_size(s->source); unsigned breaks = 0; char previous = 0;
    if (s->profile != XUI_DOCUMENT_MARKDOWN) return XUI_OK;
    while (at && breaks < 2) {
        char c; doc_seq_read(s->source, --at, &c, 1);
        if (c == '\r' || c == '\n') { if (c != '\r' || previous != '\n') breaks++; }
        else if (c != ' ' && c != '\t') break;
        previous = c;
    }
    if (breaks >= 2) {
        uint64_t id = DOC_ROOT; int result = doc_render_reserve((void**)&r->blocks, &r->capacity, r->count + 1, sizeof(*r->blocks));
        if (result != XUI_OK) return result;
        memset(&r->blocks[r->count], 0, sizeof(*r->blocks)); r->blocks[r->count].node = DOC_ROOT;
        r->blocks[r->count].virtual_gap = 1; r->blocks[r->count].height = 20 * r->desc.fZoom;
        if (!xrtMapSet(&r->block_index, xuiXrtBytes(&id, sizeof(id)), &r->count)) return XUI_ERROR_OUT_OF_MEMORY;
        r->count++;
    }
    return XUI_OK;
}
double doc_render_height_before(xui_document_renderer r, size_t index)
{
    double height = 0;
    if (r->mode == XUI_DOC_SOURCE_TEXT) return doc_rows_height_before(r->source_rows, index);
    for (; index; index -= index & (~index + 1)) height += r->heights[index];
    return height;
}
void doc_render_set_height(xui_document_renderer r, size_t index, double delta)
{
    size_t i;
    if (r->mode == XUI_DOC_SOURCE_TEXT) { doc_rows_refresh(r->source_rows, index); return; }
    for (i = index + 1; i <= r->count; i += i & (~i + 1)) r->heights[i] += delta;
}
size_t doc_render_block_at(xui_document_renderer r, double y)
{
    size_t index = 0, bit = 1; double top = 0;
    if (r->mode == XUI_DOC_SOURCE_TEXT) return doc_rows_at_y(r->source_rows, y);
    while (bit <= r->count / 2) bit <<= 1;
    for (; bit; bit >>= 1) {
        size_t next = index + bit;
        if (next <= r->count && top + r->heights[next] <= y) { top += r->heights[next]; index = next; }
    }
    return index < r->count ? index : r->count ? r->count - 1 : 0;
}
static int doc_render_materialize_block(xui_document_renderer r, size_t index, doc_render_block* b)
{
    double previous; size_t previous_bytes; int result;
    if (b->measured && !b->partial && !b->line_partial) return XUI_OK;
    previous = b->height;
    previous_bytes = b->cache_bytes;
    if (b->partial || b->line_partial) doc_render_block_free(b);
    result = doc_layout_block(r, b, r->width);
    r->cache_bytes = r->cache_bytes - previous_bytes + b->cache_bytes;
    r->size_dirty = 1;
    if (result != XUI_OK) b->height = previous;
    doc_render_set_height(r, index, b->height - previous);
    return result;
}
static int doc_render_extend_partial(xui_document_renderer r, size_t index,
    doc_render_block* b, double required_bottom)
{
    double previous = b->height;
    size_t previous_bytes = b->cache_bytes;
    int result = doc_layout_block_extend(r, b, r->width, required_bottom);
    if (result != XUI_ERROR_NOT_FOUND) {
        r->cache_bytes = r->cache_bytes - previous_bytes + b->cache_bytes;
        if (result == XUI_OK) {
            r->size_dirty = 1;
            doc_render_set_height(r, index, b->height - previous);
        }
    }
    return result;
}
static int doc_render_materialize_visible(xui_document_renderer r, size_t index,
    doc_render_block* b, double required_bottom)
{
    double previous; size_t previous_bytes; int result;
    if (b->measured) {
        if (!b->partial || required_bottom < b->partial_bottom) return XUI_OK;
        result = doc_render_extend_partial(r, index, b, required_bottom);
        if (result != XUI_ERROR_NOT_FOUND) return result;
        return doc_render_materialize_block(r, index, b);
    }
    previous = b->height; previous_bytes = b->cache_bytes;
    if (b->line_lazy && required_bottom > 0 && !b->reflow) {
        result = doc_layout_line_prefix(r, b, r->width);
        if (result == XUI_OK) {
            r->cache_bytes = r->cache_bytes - previous_bytes + b->cache_bytes;
            r->size_dirty = 1;
            doc_render_set_height(r, index, b->height - previous);
            return XUI_OK;
        }
        if (result != XUI_ERROR_NOT_FOUND) return result;
    }
    if (r->mode == XUI_DOC_VISUAL && required_bottom > 0) {
        result = doc_layout_block_prefix(r, b, r->width, required_bottom);
        if (result == XUI_OK) {
            r->cache_bytes = r->cache_bytes - previous_bytes + b->cache_bytes;
            r->size_dirty = 1;
            doc_render_set_height(r, index, b->height - previous);
            if (!r->partial_hint_valid || index < r->partial_hint) r->partial_hint = index;
            r->partial_hint_valid = 1;
            return XUI_OK;
        }
        if (result != XUI_ERROR_NOT_FOUND) return result;
    }
    return doc_render_materialize_block(r, index, b);
}
static int doc_render_cover_line(xui_document_renderer r, size_t index,
    doc_render_block* b, double required_x, uint64_t required_offset)
{
    double previous = b->height;
    size_t previous_bytes = b->cache_bytes;
    int result;
    if (!b->line_partial || (required_x <= b->line_exact_width &&
        required_offset <= b->line_cutoff)) return XUI_OK;
    result = doc_layout_line_extend(r, b, r->width, required_x,
        required_offset);
    r->cache_bytes = r->cache_bytes - previous_bytes + b->cache_bytes;
    if (result != XUI_OK) return result;
    r->size_dirty = 1;
    doc_render_set_height(r, index, b->height - previous);
    return XUI_OK;
}
static int doc_render_resolve_partial_before_y(xui_document_renderer r,
    double y);
int doc_render_cover_x(xui_document_renderer r, double x,
    double top, double height)
{
    size_t index;
    int result;
    if (!r || !isfinite(x) || !isfinite(top) || !isfinite(height) ||
        x < 0 || top < 0 || height < 0) return XUI_ERROR_INVALID_ARGUMENT;
    if (!height || !r->count) return XUI_OK;
    result = doc_render_resolve_partial_before_y(r, top);
    if (result != XUI_OK) return result;
    index = doc_render_block_at(r, top);
    for (; index < r->count; index++) {
        doc_render_block* b = doc_render_block_at_index(r, index);
        double block_top = doc_render_height_before(r, index);
        if (block_top > top + height) break;
        result = doc_render_materialize_visible(r, index, b,
            top + height - block_top + 1);
        if (result != XUI_OK) return result;
        result = doc_render_cover_line(r, index, b, x, 0);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
int doc_render_materialize(xui_document_renderer r, size_t index)
{
    doc_render_block* b = doc_render_block_at_index(r, index);
    return b ? doc_render_materialize_block(r, index, b) : XUI_ERROR_NOT_FOUND;
}
/* Partial visual blocks and source rows with variable glyph metrics can move
 * every later block. Settle only uncertain predecessors before using their
 * accumulated height; the row-tree count and flat hint skip confirmed rows. */
static int doc_render_resolve_partial_before_index(xui_document_renderer r,
    size_t index, int include_source_row, int* changed)
{
    size_t i, source_limit = index;
    int did_change = 0, result;
    if (changed) *changed = 0;
    if (include_source_row && source_limit < r->count) source_limit++;
    if (r->mode == XUI_DOC_SOURCE_TEXT) {
        if (source_limit > r->count) source_limit = r->count;
        for (;;) {
            i = doc_rows_first_unsettled_before(r->source_rows,
                source_limit, 0);
            if (i == SIZE_MAX) break;
            result = doc_render_materialize(r, i);
            if (result != XUI_OK) return result;
            did_change = 1;
        }
        if (changed) *changed = did_change;
        return XUI_OK;
    }
    if (r->mode == XUI_DOC_LIVE_MARKDOWN && r->source_height_hint_valid &&
        source_limit > r->source_height_hint) {
        size_t end = source_limit < r->count ? source_limit : r->count;
        for (i = r->source_height_hint; i < end; i++) {
            doc_render_block* b = &r->blocks[i];
            if (!b->source_line || !b->height_uncertain || b->ever_measured)
                continue;
            result = doc_render_materialize(r, i);
            if (result != XUI_OK) return result;
            did_change = 1;
        }
        r->source_height_hint = end;
        if (end == r->count) r->source_height_hint_valid = 0;
    }
    if (!r->partial_hint_valid || index <= r->partial_hint) {
        if (changed) *changed = did_change;
        return XUI_OK;
    }
    if (index > r->count) index = r->count;
    for (i = r->partial_hint; i < index; i++) {
        if (!r->blocks[i].partial) continue;
        result = doc_render_materialize_block(r, i, &r->blocks[i]);
        if (result != XUI_OK) return result;
        did_change = 1;
    }
    i = r->partial_hint;
    while (i < r->count && !r->blocks[i].partial) i++;
    r->partial_hint_valid = i < r->count;
    if (r->partial_hint_valid) r->partial_hint = i;
    if (changed) *changed = did_change;
    return XUI_OK;
}
static int doc_render_resolve_partial_before_y(xui_document_renderer r, double y)
{
    int changed, result;
    if (r->mode == XUI_DOC_SOURCE_TEXT && r->count) {
        do {
            size_t index = doc_render_block_at(r, y);
            result = doc_render_resolve_partial_before_index(r,
                index, 1, &changed);
            if (result != XUI_OK) return result;
        } while (changed);
        return XUI_OK;
    }
    if (!r->partial_hint_valid && !r->source_height_hint_valid) return XUI_OK;
    do {
        size_t index = doc_render_block_at(r, y);
        result = doc_render_resolve_partial_before_index(r,
            index, r->mode == XUI_DOC_LIVE_MARKDOWN, &changed);
        if (result != XUI_OK) return result;
    } while (changed && (r->partial_hint_valid ||
        r->source_height_hint_valid));
    return XUI_OK;
}
size_t doc_render_find(xui_document_renderer r, uint64_t id)
{
    doc_node* n = doc_index_get(r->snapshot->state->index, id);
    for (; n; n = doc_index_get(r->snapshot->state->index, n->parent)) {
        size_t* index = xrtMapGet(&r->block_index, xuiXrtBytes(&n->id, sizeof(n->id)));
        if (index) return *index;
    }
    return SIZE_MAX;
}
/* SetTextStyle may split text runs while leaving every visual block in place.
 * Keep the block directory and unrelated measured blocks in that case. Each
 * INSERT must be the new half of the immediately following text SPLIT; other
 * structural edits still require a full rebuild. */
int doc_render_inline_style_splits(xui_document_change_set changes)
{
    uint64_t i;
    if (!changes || !(changes->flags & XUI_DOC_CHANGE_STYLE) ||
        !(changes->flags & XUI_DOC_CHANGE_STRUCTURE) ||
        (changes->flags & (XUI_DOC_CHANGE_SOURCE | XUI_DOC_CHANGE_RESET))) return 0;
    for (i = 0; i < changes->count; i++) {
        const xui_doc_operation_t* op = &changes->ops[i];
        const xui_doc_operation_t *split, *insert;
        doc_node *old_text, *left, *right, *parent;
        if (!(op->iFlags & XUI_DOC_CHANGE_STRUCTURE)) continue;
        if (op->iKind == XUI_DOC_OP_INSERT) {
            if (i + 1 >= changes->count) return 0;
            split = &changes->ops[i + 1];
            right = doc_index_get(changes->after->index, op->iNodeId);
            parent = doc_index_get(changes->before->index, op->iParentId);
            if (split->iKind != XUI_DOC_OP_SPLIT || split->iParentId ||
                split->iOtherNodeId != op->iNodeId || !right ||
                right->kind != XUI_DOC_TEXT || right->parent != op->iParentId ||
                (!parent || (parent->kind != XUI_DOC_PARAGRAPH &&
                    parent->kind != XUI_DOC_HEADING))) return 0;
        } else if (op->iKind == XUI_DOC_OP_SPLIT) {
            if (!i || op->iParentId) return 0;
            insert = &changes->ops[i - 1];
            old_text = doc_index_get(changes->before->index, op->iNodeId);
            left = doc_index_get(changes->after->index, op->iNodeId);
            right = doc_index_get(changes->after->index, op->iOtherNodeId);
            if (insert->iKind != XUI_DOC_OP_INSERT ||
                insert->iNodeId != op->iOtherNodeId || !old_text || !left || !right ||
                old_text->kind != XUI_DOC_TEXT || left->kind != XUI_DOC_TEXT ||
                right->kind != XUI_DOC_TEXT || left->parent != old_text->parent ||
                right->parent != old_text->parent) return 0;
        } else return 0;
    }
    return 1;
}
int doc_render_metric_style_change(xui_document_change_set changes)
{
    uint64_t i;
    const uint32_t color_flags = XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO |
        XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO |
        XUI_DOC_TEXT_COLOR_CURRENT | XUI_DOC_BACKGROUND_COLOR_CURRENT;
    if (!changes) return 0;
    for (i = 0; i < changes->count; i++) {
        const xui_doc_operation_t* op = &changes->ops[i];
        doc_node *a, *b;
        if (!(op->iFlags & XUI_DOC_CHANGE_STYLE)) continue;
        a = doc_index_get(changes->before->index, op->iNodeId);
        b = doc_index_get(changes->after->index, op->iNodeId);
        if (!a || !b || a->kind != b->kind ||
            a->attrs->iMarks != b->attrs->iMarks ||
            ((a->attrs->iFlags ^ b->attrs->iFlags) & ~color_flags) ||
            a->attrs->iHeadingLevel != b->attrs->iHeadingLevel ||
            a->attrs->iAlignment != b->attrs->iAlignment ||
            a->attrs->iRowSpan != b->attrs->iRowSpan ||
            a->attrs->iColumnSpan != b->attrs->iColumnSpan ||
            a->attrs->iListStart != b->attrs->iListStart ||
            a->attrs->fFontSize != b->attrs->fFontSize ||
            a->attrs->fWidth != b->attrs->fWidth ||
            a->attrs->fHeight != b->attrs->fHeight ||
            a->attrs->fParagraphSpacing != b->attrs->fParagraphSpacing ||
            strcmp(a->attrs->sFontFamily, b->attrs->sFontFamily) ||
            !doc_language_equal(a->attrs->sLanguage, b->attrs->sLanguage)) return 1;
    }
    return 0;
}
static int doc_render_paint_only_change(xui_document_change_set changes)
{
    uint64_t i;
    if (!changes || changes->flags != XUI_DOC_CHANGE_STYLE || !changes->count ||
        doc_render_metric_style_change(changes)) return 0;
    for (i = 0; i < changes->count; i++) {
        const xui_doc_operation_t* op = &changes->ops[i];
        doc_node* before = doc_index_get(changes->before->index, op->iNodeId);
        doc_node* after = doc_index_get(changes->after->index, op->iNodeId);
        if (op->iKind != XUI_DOC_OP_ATTRIBUTES || op->iFlags != XUI_DOC_CHANGE_STYLE ||
            !before || !after || before->kind != after->kind ||
            (after->kind != XUI_DOC_TEXT && after->kind != XUI_DOC_PARAGRAPH &&
             after->kind != XUI_DOC_HEADING && after->kind != XUI_DOC_CELL)) return 0;
    }
    return 1;
}
static size_t doc_render_find_text(xui_document_renderer r, uint64_t id, uint64_t offset, uint32_t affinity)
{
    size_t first = doc_render_find(r, id), low = 0, high;
    if (first == SIZE_MAX || !r->blocks[first].code_slice || r->blocks[first].node != id) return first;
    high = r->blocks[first].slice_count;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        uint64_t end = r->blocks[first + middle].text_end;
        if (offset < end || (offset == end && affinity == XUI_DOC_BEFORE)) high = middle;
        else low = middle + 1;
    }
    return first + (low < r->blocks[first].slice_count ? low : r->blocks[first].slice_count - 1);
}
XUI_API int xuiDocumentRendererCreate(xui_context context, const xui_doc_renderer_desc_t* desc, xui_document_renderer* out)
{
    xui_document_renderer r;
    if (out) *out = NULL;
    if (!context || !out || (desc && (desc->iSize != sizeof(*desc) || desc->bSourceLineHeightMayVary > 1 || !isfinite(desc->fZoom) || desc->fZoom < 0 ||
        !isfinite(desc->fParagraphGap) || desc->fParagraphGap < 0 || !isfinite(desc->fLineGap) || desc->fLineGap < 0 ||
        !isfinite(desc->fIndent) || desc->fIndent < 0))) return XUI_ERROR_INVALID_ARGUMENT;
    r = calloc(1, sizeof(*r)); if (!r) return XUI_ERROR_OUT_OF_MEMORY;
    r->context = context; r->proxy = &r->proxy_storage;
    r->resource_registry_generation = xuiResourceGetRegistryGeneration(context);
    r->dpi_scale = xuiGetVirtualDpi(context);
    r->uses_default_font = !desc || !desc->tFonts.normal;
    if (xuiGetProxy(context, &r->proxy_storage) != XUI_OK) { free(r); return XUI_ERROR_INVALID_ARGUMENT; }
    if (desc) r->desc = *desc;
    r->desc.iSize = sizeof(r->desc); r->stats.iSize = sizeof(r->stats);
    if (r->desc.fZoom == 0) r->desc.fZoom = 1;
    if (r->desc.fIndent == 0) r->desc.fIndent = 24;
    if (r->desc.fParagraphGap == 0) r->desc.fParagraphGap = 8;
    if (!r->desc.iLayoutCacheBudgetBytes) r->desc.iLayoutCacheBudgetBytes = UINT64_C(32) * 1024 * 1024;
    if (!r->desc.tFonts.normal) r->desc.tFonts.normal = xuiGetDefaultFont(context);
    if (!r->desc.iTextColor) r->desc.iTextColor = XUI_COLOR_RGBA(32, 36, 44, 255);
    if (!r->desc.iBorderColor) r->desc.iBorderColor = XUI_COLOR_RGBA(160, 167, 178, 255);
    if (!r->desc.iCodeBackground) r->desc.iCodeBackground = XUI_COLOR_RGBA(240, 242, 246, 255);
    if (!r->desc.iHighlightColor) r->desc.iHighlightColor = XUI_COLOR_RGBA(255, 235, 135, 255);
    if (!r->desc.iLinkColor) r->desc.iLinkColor = XUI_COLOR_RGBA(25, 100, 185, 255);
    if (!r->desc.iQuoteBorderColor) r->desc.iQuoteBorderColor = r->desc.iBorderColor;
    if (!r->desc.iRuleColor) r->desc.iRuleColor = r->desc.iBorderColor;
    if (!r->desc.iTableBorderColor) r->desc.iTableBorderColor = r->desc.iBorderColor;
    if (!r->desc.iImagePlaceholderColor) r->desc.iImagePlaceholderColor = XUI_COLOR_RGBA(238, 241, 245, 255);
    if (!r->desc.iImageBorderColor) r->desc.iImageBorderColor = XUI_COLOR_RGBA(170, 180, 192, 255);
    if (!r->desc.iImageTextColor) r->desc.iImageTextColor = XUI_COLOR_RGBA(90, 100, 112, 255);
    r->width = 640; r->mode = XUI_DOC_VISUAL; r->size_dirty = 1;
    if (!r->proxy || !xrtMapInit(&r->block_index, sizeof(size_t))) { free(r); return XUI_ERROR_OUT_OF_MEMORY; }
    *out = r; return XUI_OK;
}
XUI_API void xuiDocumentRendererRelease(xui_document_renderer r)
{
    size_t i;
    if (!r) return;
    doc_render_clear(r); xrtMapUnit(&r->block_index); xuiDocumentSnapshotRelease(r->snapshot); xuiDocumentPrepareRelease(r->input);
    for (i = 0; i < r->font_count; i++) if (r->proxy->fontDestroy) r->proxy->fontDestroy(r->proxy, r->fonts[i].font);
    free(r->fonts); free(r);
}
XUI_API int xuiDocumentRendererSetSnapshot(xui_document_renderer r, xui_document_snapshot s, xui_document_change_set changes)
{
    int rebuild, promote, paint_only; size_t i;
    if (!r || !s) return XUI_ERROR_INVALID_ARGUMENT;
    if (r->mode != XUI_DOC_VISUAL && s->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    if (!r->input && r->snapshot && r->snapshot->state == s->state &&
        r->snapshot->identity == s->identity && r->snapshot->revision == s->revision) return XUI_OK;
    promote = r->input && r->mode == XUI_DOC_SOURCE_TEXT && r->snapshot->identity == s->identity &&
        doc_render_source(r) == s->state->source &&
        doc_language_equal(doc_index_get(r->snapshot->state->index, DOC_ROOT)->attrs->sLanguage,
            doc_index_get(s->state->index, DOC_ROOT)->attrs->sLanguage);
    rebuild = !promote && (r->input || !r->snapshot || !changes || changes->identity != s->identity || changes->before_revision != r->snapshot->revision ||
        changes->before != r->snapshot->state || changes->after != s->state ||
        changes->after_revision != s->revision ||
        (changes->flags & (XUI_DOC_CHANGE_SOURCE | XUI_DOC_CHANGE_RESET)) ||
        ((changes->flags & XUI_DOC_CHANGE_STRUCTURE) &&
            !(r->mode == XUI_DOC_VISUAL && doc_render_inline_style_splits(changes))));
    if (!rebuild && !promote) for (i = 0; i < changes->count; i++) {
        size_t index = doc_render_find(r, changes->ops[i].iNodeId);
        doc_node* changed = doc_index_get(s->state->index, changes->ops[i].iNodeId);
        /* Root/quote/list attributes can affect many descendant blocks.
         * They have no block entry of their own; dropping just an indexed
         * ancestor would leave inherited font/language measurements stale. */
        if ((changes->ops[i].iFlags & XUI_DOC_CHANGE_STYLE) && changed &&
            (changed->kind == XUI_DOC_ROOT ||
             (index == SIZE_MAX && doc_seq_size(changed->children)))) { rebuild = 1; break; }
        if (index != SIZE_MAX && r->blocks[index].code_slice) { rebuild = 1; break; }
    }
    paint_only = !rebuild && !promote && r->mode == XUI_DOC_VISUAL &&
        doc_render_paint_only_change(changes);
    if (rebuild) {
        struct xui_document_renderer_t next = {0}; int result;
        next.context = r->context; next.proxy = r->proxy;
        next.snapshot = s; next.desc = r->desc; next.width = r->width; next.mode = r->mode; next.active_source = r->active_source;
        if (!xrtMapInit(&next.block_index, sizeof(size_t))) return XUI_ERROR_OUT_OF_MEMORY;
        if (r->mode == XUI_DOC_SOURCE_TEXT) result = doc_render_collect_source_range(&next, 0, doc_seq_size(s->state->source));
        else if (r->mode == XUI_DOC_LIVE_MARKDOWN) result = doc_render_collect_live(&next);
        else result = doc_render_collect(&next, DOC_ROOT, 0);
        if (result == XUI_OK && r->mode == XUI_DOC_VISUAL) result = doc_render_empty_tail(&next);
        if (result == XUI_OK && r->mode != XUI_DOC_SOURCE_TEXT) {
            next.heights = doc_render_build_heights(next.blocks, next.count);
            if (!next.heights) result = XUI_ERROR_OUT_OF_MEMORY;
        }
        if (result != XUI_OK) { doc_render_clear(&next); xrtMapUnit(&next.block_index); return result; }
        doc_render_clear(r); xrtMapUnit(&r->block_index);
        r->blocks = next.blocks; r->heights = next.heights; r->source_rows = next.source_rows;
        r->source_row_serial = next.source_row_serial;
        r->source_height_hint = next.source_height_hint;
        r->source_height_hint_valid = next.source_height_hint_valid;
        r->count = next.count; r->capacity = next.capacity; r->block_index = next.block_index;
        r->size_dirty = 1;
        r->has_named_images = next.has_named_images;
        r->active_source = next.active_source; r->live_start = next.live_start; r->live_end = next.live_end;
        r->live_active_node = next.live_active_node; r->live_base_end = next.live_base_end;
        r->live_first = next.live_first; r->live_rows = next.live_rows;
        r->live_fallback = next.live_fallback; r->live_pending_full = next.live_pending_full;
        r->stats.iSourceBytesScanned += next.stats.iSourceBytesScanned;
        r->stats.iSourceRowsCreated += next.stats.iSourceRowsCreated;
    } else if (!promote && !paint_only) {
        for (i = 0; i < changes->count; i++) {
            size_t index = doc_render_find(r, changes->ops[i].iNodeId);
            if (index != SIZE_MAX) doc_render_release_block(r, &r->blocks[index]);
        }
    }
    xuiDocumentSnapshotRetain(s); xuiDocumentSnapshotRelease(r->snapshot); r->snapshot = s;
    xuiDocumentPrepareRelease(r->input); r->input = NULL;
    if (paint_only) {
        /* Geometry remains valid in the successor snapshot. A later width-only
         * reflow must not discard its shared shape merely for the old revision. */
        for(i=0;i<r->count;i++)if(r->blocks[i].shaped_revision==changes->before_revision)
            r->blocks[i].shaped_revision=s->revision;
        for (i = 0; i < changes->count; i++) {
            size_t index = doc_render_find(r, changes->ops[i].iNodeId), j;
            doc_render_block* block;
            if (index == SIZE_MAX) continue;
            block = &r->blocks[index];
            for (j = 0; j < r->blocks[index].run_count; j++) {
                doc_render_run* run = &block->runs[j];
                doc_node* node = doc_index_get(s->state->index, run->node);
                if (node) run->attrs = doc_effective_text_attrs(s->state, node);
            }
            for (j = 0; j < block->box_count; j++) {
                doc_box* box = &block->boxes[j];
                doc_node* node = doc_index_get(s->state->index, box->node);
                if (node && (box->kind == XUI_DOC_PARAGRAPH ||
                    box->kind == XUI_DOC_HEADING || box->kind == XUI_DOC_CELL))
                    box->color = node->attrs->iBackgroundColor;
            }
        }
    }
    return XUI_OK;
}
static int doc_render_set_live_input(xui_document_renderer r, xui_document_prepare p, int force_full)
{
    struct xui_document_renderer_t next = {0};
    uint64_t start, end, bytes, candidate_end = r->live_end;
    uint64_t source_size = doc_seq_size(doc_prepare_source_store(p));
    int full = force_full || r->live_pending_full || r->live_fallback;
    int result;
    if (!full) {
        if (!doc_prepare_source_delta(p, r->input, &start, &end, &bytes) ||
            start < r->live_start || end > r->live_end || start > end ||
            r->live_end - (end - start) > UINT64_MAX - bytes)
            full = 1;
        else {
            candidate_end = r->live_end - (end - start) + bytes;
            if (candidate_end > source_size) full = 1;
        }
    }
    if (!full) {
        result = doc_render_live_splice(r, p, start, end, bytes, candidate_end);
        if (result == XUI_OK) return XUI_OK;
        if (result != XUI_ERROR_NOT_FOUND) return result;
    }
    next.context = r->context; next.proxy = r->proxy; next.snapshot = r->snapshot;
    next.input = p; next.desc = r->desc; next.width = r->width; next.mode = r->mode;
    next.active_source = r->active_source;
    next.live_active_node = r->live_active_node; next.live_base_end = r->live_base_end;
    next.live_pending_full = full; next.live_end = candidate_end;
    if (!xrtMapInit(&next.block_index, sizeof(size_t))) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_render_collect_live(&next);
    if (result == XUI_OK) {
        next.heights = doc_render_build_heights(next.blocks, next.count);
        if (!next.heights) result = XUI_ERROR_OUT_OF_MEMORY;
    }
    if (result != XUI_OK) { doc_render_clear(&next); xrtMapUnit(&next.block_index); return result; }
    doc_render_clear(r); xrtMapUnit(&r->block_index);
    r->blocks = next.blocks; r->heights = next.heights;
    r->count = next.count; r->capacity = next.capacity; r->block_index = next.block_index;
    r->source_height_hint = next.source_height_hint;
    r->source_height_hint_valid = next.source_height_hint_valid;
    r->size_dirty = 1; r->has_named_images = next.has_named_images;
    r->active_source = next.active_source; r->live_start = next.live_start; r->live_end = next.live_end;
    r->live_active_node = next.live_active_node; r->live_base_end = next.live_base_end;
    r->live_first = next.live_first; r->live_rows = next.live_rows;
    r->live_fallback = next.live_fallback; r->live_pending_full = next.live_pending_full;
    r->stats.iSourceBytesScanned += next.stats.iSourceBytesScanned;
    r->stats.iSourceRowsCreated += next.stats.iSourceRowsCreated;
    xuiDocumentPrepareRetain(p); xuiDocumentPrepareRelease(r->input); r->input = p;
    return XUI_OK;
}
XUI_API int xuiDocumentRendererSetSourceInput(xui_document_renderer r, xui_document_prepare p)
{
    struct xui_document_renderer_t next = {0}; xui_doc_prepare_info_t info = {0}; int result;
    if (!r || !r->snapshot) return XUI_ERROR_INVALID_ARGUMENT;
    if (!p) return xuiDocumentRendererSetSnapshot(r, r->snapshot, NULL);
    if (r->mode != XUI_DOC_SOURCE_TEXT && r->mode != XUI_DOC_LIVE_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    if (r->input == p) return XUI_OK;
    info.iSize = sizeof(info); result = xuiDocumentPrepareGetInfo(p, &info); if (result != XUI_OK) return result;
    if (info.iDocumentId != r->snapshot->identity || info.iBaseRevision != r->snapshot->revision) return XUI_DOC_ERROR_STALE;
    if (r->mode == XUI_DOC_LIVE_MARKDOWN) return doc_render_set_live_input(r, p, 0);
    result = doc_render_source_splice(r, p);
    if (result == XUI_OK) goto applied;
    if (result != XUI_ERROR_NOT_FOUND) return result;
    next.snapshot = r->snapshot; next.input = p; next.desc = r->desc; next.width = r->width; next.mode = r->mode;
    next.context = r->context; next.proxy = r->proxy;
    if (!xrtMapInit(&next.block_index, sizeof(size_t))) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_render_collect_source_range(&next, 0, info.iSourceBytes);
    if (result != XUI_OK) { doc_render_clear(&next); xrtMapUnit(&next.block_index); return result; }
    doc_render_clear(r); xrtMapUnit(&r->block_index);
    r->blocks = next.blocks; r->source_rows = next.source_rows; r->source_row_serial = next.source_row_serial;
    r->count = next.count; r->capacity = next.capacity; r->heights = next.heights; r->block_index = next.block_index;
    r->size_dirty = 1;
    r->stats.iSourceBytesScanned += next.stats.iSourceBytesScanned;
    r->stats.iSourceRowsCreated += next.stats.iSourceRowsCreated;
applied:
    xuiDocumentPrepareRetain(p); xuiDocumentPrepareRelease(r->input); r->input = p; return XUI_OK;
}
XUI_API int xuiDocumentRendererSetMode(xui_document_renderer r, uint32_t mode)
{
    xui_document_snapshot s; uint32_t previous; int result;
    if (!r || (mode != XUI_DOC_VISUAL && mode != XUI_DOC_SOURCE_TEXT && mode != XUI_DOC_LIVE_MARKDOWN)) return XUI_ERROR_INVALID_ARGUMENT;
    if (r->mode == mode) return XUI_OK;
    if (r->input) return XUI_DOC_ERROR_BUSY;
    if (mode != XUI_DOC_VISUAL && r->snapshot && r->snapshot->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    previous = r->mode; r->mode = mode;
    if (!r->snapshot) return XUI_OK;
    s = r->snapshot; r->snapshot = NULL;
    result = xuiDocumentRendererSetSnapshot(r, s, NULL);
    if (result != XUI_OK) { r->snapshot = s; r->mode = previous; }
    else xuiDocumentSnapshotRelease(s);
    return result;
}
XUI_API int xuiDocumentRendererInvalidateObjects(xui_document_renderer r)
{
    size_t i;
    if (!r) return XUI_ERROR_INVALID_ARGUMENT;
    r->resource_registry_generation = xuiResourceGetRegistryGeneration(r->context);
    if (r->mode == XUI_DOC_SOURCE_TEXT) return XUI_OK;
    for (i = 0; i < r->count; i++) {
        doc_render_block* block = &r->blocks[i];
        if (!block->object_dependent) continue;
        /* Object geometry/pixels changed, but immutable text and its fonts
         * can still seed the next layout, including a retained paragraph prefix. */
        if (r->snapshot && (block->measured || block->reflow) &&
            (!block->partial || block->continuation) && !block->line_partial &&
            block->shaped_revision == r->snapshot->revision &&
            fabsf(block->dpi_scale - xuiGetVirtualDpi(r->context)) < .001f) {
            size_t previous_bytes = block->cache_bytes;
            doc_render_block_reflow(block);
            r->cache_bytes = r->cache_bytes - previous_bytes + block->cache_bytes;
        } else doc_render_release_block(r, block);
    }
    r->size_dirty = 1;
    return XUI_OK;
}
static void doc_render_live_reset_source_heights(xui_document_renderer r)
{
    double line_height = doc_render_code_line_height(r, NULL);
    size_t i;
    r->source_height_hint_valid = 0;
    for (i = 0; i < r->count; i++) {
        doc_render_block* b = &r->blocks[i];
        if (!b->source_line) continue;
        b->height = line_height;
        b->ever_measured = 0;
        if (b->height_uncertain && !r->source_height_hint_valid) {
            r->source_height_hint = i;
            r->source_height_hint_valid = 1;
        }
    }
    if (!r->heights) return;
    memset(r->heights, 0, (r->count + 1) * sizeof(*r->heights));
    for (i = 1; i <= r->count; i++) {
        size_t parent = i + (i & (~i + 1));
        r->heights[i] += r->blocks[i - 1].height;
        if (parent <= r->count) r->heights[parent] += r->heights[i];
    }
}
static int doc_render_invalidate_fonts(xui_document_renderer r, float dpi_scale,
    xui_font default_font)
{
    struct xui_document_renderer_t next = {0};
    size_t i;
    int result = XUI_OK;
    if (r->snapshot) {
        next.context = r->context; next.proxy = r->proxy;
        next.snapshot = r->snapshot; next.input = r->input;
        next.desc = r->desc; next.desc.tFonts.normal = default_font;
        next.width = r->width; next.mode = r->mode;
        next.active_source = r->active_source;
        next.live_start = r->live_start; next.live_end = r->live_end;
        next.live_active_node = r->live_active_node;
        next.live_base_end = r->live_base_end;
        next.live_pending_full = r->live_pending_full;
        next.source_row_serial = r->source_row_serial;
        if (!xrtMapInit(&next.block_index, sizeof(size_t))) return XUI_ERROR_OUT_OF_MEMORY;
        if (r->mode == XUI_DOC_SOURCE_TEXT)
            result = doc_render_collect_source_range(&next, 0,
                doc_seq_size(doc_render_source(&next)));
        else if (r->mode == XUI_DOC_LIVE_MARKDOWN)
            result = doc_render_collect_live(&next);
        else result = doc_render_collect(&next, DOC_ROOT, 0);
        if (result == XUI_OK && r->mode == XUI_DOC_VISUAL)
            result = doc_render_empty_tail(&next);
        if (result == XUI_OK && r->mode != XUI_DOC_SOURCE_TEXT) {
            next.heights = doc_render_build_heights(next.blocks, next.count);
            if (!next.heights) result = XUI_ERROR_OUT_OF_MEMORY;
        }
        if (result != XUI_OK) {
            doc_render_clear(&next); xrtMapUnit(&next.block_index);
            return result;
        }
        doc_render_clear(r); xrtMapUnit(&r->block_index);
        r->blocks = next.blocks; r->heights = next.heights;
        r->source_rows = next.source_rows; r->source_row_serial = next.source_row_serial;
        r->source_height_hint = next.source_height_hint;
        r->source_height_hint_valid = next.source_height_hint_valid;
        r->count = next.count; r->capacity = next.capacity;
        r->block_index = next.block_index;
        r->has_named_images = next.has_named_images;
        r->active_source = next.active_source;
        r->live_start = next.live_start; r->live_end = next.live_end;
        r->live_active_node = next.live_active_node;
        r->live_base_end = next.live_base_end;
        r->live_first = next.live_first; r->live_rows = next.live_rows;
        r->live_fallback = next.live_fallback;
        r->live_pending_full = next.live_pending_full;
        r->stats.iSourceBytesScanned += next.stats.iSourceBytesScanned;
        r->stats.iSourceRowsCreated += next.stats.iSourceRowsCreated;
    }
    /* No block layout retains a sized font after clearing the old directory. */
    for (i = 0; i < r->font_count; i++)
        if (r->proxy->fontDestroy) r->proxy->fontDestroy(r->proxy, r->fonts[i].font);
    r->font_count = 0;
    r->desc.tFonts.normal = default_font;
    r->dpi_scale = dpi_scale;
    r->size_dirty = 1;
    return XUI_OK;
}
XUI_API int xuiDocumentRendererInvalidateFonts(xui_document_renderer r)
{
    if (!r) return XUI_ERROR_INVALID_ARGUMENT;
    return doc_render_invalidate_fonts(r, xuiGetVirtualDpi(r->context),
        r->uses_default_font ? xuiGetDefaultFont(r->context) : r->desc.tFonts.normal);
}
static int doc_render_refresh_resources(xui_document_renderer r)
{
    uint64_t generation = xuiResourceGetRegistryGeneration(r->context);
    float dpi_scale = xuiGetVirtualDpi(r->context);
    xui_font default_font = r->uses_default_font ? xuiGetDefaultFont(r->context) : r->desc.tFonts.normal;
    int result;
    if (r->freeze_dynamic_refresh) return XUI_OK;
    if (dpi_scale != r->dpi_scale || default_font != r->desc.tFonts.normal) {
        result = doc_render_invalidate_fonts(r, dpi_scale, default_font);
        if (result != XUI_OK) return result;
    }
    if (generation == r->resource_registry_generation) return XUI_OK;
    if (!r->has_named_images) { r->resource_registry_generation = generation; return XUI_OK; }
    return xuiDocumentRendererInvalidateObjects(r);
}
XUI_API int xuiDocumentRendererSetActivePosition(xui_document_renderer r, const xui_doc_position_t* p)
{
    xui_document_snapshot s; uint64_t offset, previous; int mapping, result;
    if (!r || !r->snapshot || !p) return XUI_ERROR_INVALID_ARGUMENT;
    if (r->mode != XUI_DOC_LIVE_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    result = r->input ? doc_render_live_position_source(r, p, &offset) :
        xuiDocumentPositionToSource(r->snapshot, p, &offset, &mapping);
    if (result != XUI_OK) return result;
    if (offset >= r->live_start && (offset < r->live_end || (offset == r->live_end && offset == doc_seq_size(doc_render_source(r))))) {
        r->active_source = offset; return XUI_OK;
    }
    previous = r->active_source; r->active_source = offset;
    if (r->input) {
        result = doc_render_set_live_input(r, r->input, 1);
        if (result != XUI_OK) r->active_source = previous;
        return result;
    }
    s = r->snapshot; r->snapshot = NULL;
    result = xuiDocumentRendererSetSnapshot(r, s, NULL);
    if (result != XUI_OK) { r->snapshot = s; r->active_source = previous; }
    else xuiDocumentSnapshotRelease(s);
    return result;
}
XUI_API int xuiDocumentRendererGetActiveSourceRange(xui_document_renderer r, uint64_t* start, uint64_t* end)
{
    if (!r || !r->snapshot || !start || !end) return XUI_ERROR_INVALID_ARGUMENT;
    if (r->mode != XUI_DOC_LIVE_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    *start = r->live_start; *end = r->live_end; return XUI_OK;
}
XUI_API int xuiDocumentRendererLayout(xui_document_renderer r, double width, double top, double height)
{
    size_t i, start, visible_bytes = 0; double y; int result, reverse;
    if (!r || !r->snapshot || !isfinite(width) || !isfinite(top) || !isfinite(height) || width <= 0 || top < 0 || height < 0) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_render_refresh_resources(r); if (result != XUI_OK) return result;
    if (fabs(r->width - width) > .001) {
        if (r->mode == XUI_DOC_SOURCE_TEXT) {
            doc_rows_unmeasure(r->source_rows, doc_render_code_line_height(r, NULL));
            r->cache_bytes = 0;
        }
        else {
            double block_top = 0, keep_top = fmax(0, r->viewport_top - 128), keep_bottom = r->viewport_top + r->viewport_height + 128;
            float dpi_scale = xuiGetVirtualDpi(r->context);
            for (i = 0; i < r->count; i++) {
                doc_render_block* block = &r->blocks[i];
                double block_bottom = block_top + block->height;
                size_t previous_bytes = block->cache_bytes;
                if (r->viewport_height > 0 && (!block->partial || block->continuation) && !block->line_partial &&
                    (block->measured || block->reflow) &&
                    block_bottom >= keep_top && block_top <= keep_bottom &&
                    block->shaped_revision == r->snapshot->revision &&
                    fabsf(block->dpi_scale - dpi_scale) < .001f)
                    doc_render_block_reflow(block);
                else doc_render_block_free(block);
                r->cache_bytes = r->cache_bytes - previous_bytes + block->cache_bytes;
                block_top = block_bottom;
            }
            if (r->mode == XUI_DOC_LIVE_MARKDOWN)
                doc_render_live_reset_source_heights(r);
        }
        r->width = width;
        r->size_dirty = 1;
    }
    reverse = top < r->viewport_top;
    r->viewport_top = top; r->viewport_height = height;
    if (height == 0) return XUI_OK;
    result = doc_render_resolve_partial_before_y(r, fmax(0, top - 128));
    if (result != XUI_OK) return result;
    start = doc_render_block_at(r, fmax(0, top - 128));
    y = doc_render_height_before(r, start);
    for (i = start; i < r->count; i++) {
        doc_render_block* block = doc_render_block_at_index(r, i);
        if (!block) return XUI_ERROR_NOT_FOUND;
        if (i > start && y > top + height + 128) break;
        /* Once cache pressure is significant, speculative offscreen layout
         * would only be discarded and shaped again on the next frame. */
        if (!block->measured && (y + block->height <= top || y >= top + height) &&
            r->cache_bytes >= r->desc.iLayoutCacheBudgetBytes / 2) {
            y += block->height;
            continue;
        }
        result = doc_render_materialize_visible(r, i, block,
            top + height + 128 - y);
        if (result != XUI_OK) return result;
        if (y + block->height > top && y < top + height) visible_bytes += block->cache_bytes;
        y += block->height;
    }
    if (r->cache_bytes > r->desc.iLayoutCacheBudgetBytes && r->cache_bytes > visible_bytes) {
        if (r->mode == XUI_DOC_SOURCE_TEXT)
            doc_rows_trim_cache(r, r->source_rows, 0, top, top + height, reverse);
        else if (reverse) {
            double y = doc_render_height_before(r, r->count);
            for (i = r->count; i && r->cache_bytes > r->desc.iLayoutCacheBudgetBytes; i--) {
                doc_render_block* b = &r->blocks[i - 1];
                y -= b->height;
                if (b->cache_bytes && (y + b->height <= top || y >= top + height)) {
                    doc_render_release_block(r, b); r->stats.iLayoutCacheEvictions++;
                }
            }
        } else {
            double y = 0;
            for (i = 0; i < r->count && r->cache_bytes > r->desc.iLayoutCacheBudgetBytes; i++) {
                doc_render_block* b = &r->blocks[i];
                if (b->cache_bytes && (y + b->height <= top || y >= top + height)) {
                    doc_render_release_block(r, b); r->stats.iLayoutCacheEvictions++;
                }
                y += b->height;
            }
        }
    }
    return XUI_OK;
}
XUI_API int xuiDocumentRendererGetSize(xui_document_renderer r, xui_doc_rect_t* size, int* exact)
{
    size_t i; int result;
    if (!r || !size || !exact) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_render_refresh_resources(r); if (result != XUI_OK) return result;
    if (r->mode == XUI_DOC_SOURCE_TEXT) {
        *size = (xui_doc_rect_t){0, 0, doc_rows_width(r->source_rows), doc_rows_height(r->source_rows)};
        *exact = !doc_rows_unmeasured(r->source_rows);
        return XUI_OK;
    }
    if (r->size_dirty) {
        r->size_width = 0; r->size_exact = 1;
        for (i = 0; i < r->count; i++) {
            if (!r->blocks[i].measured || r->blocks[i].partial ||
                r->blocks[i].line_partial) r->size_exact = 0;
            if (r->blocks[i].width > r->size_width) r->size_width = r->blocks[i].width;
        }
        r->size_dirty = 0;
    }
    *size = (xui_doc_rect_t){0, 0, r->size_width, doc_render_height_before(r, r->count)};
    *exact = r->size_exact;
    return XUI_OK;
}
static void doc_render_position(xui_document_renderer r, uint64_t id, uint64_t offset, xui_doc_position_t* out)
{
    doc_node* n = doc_index_get(r->snapshot->state->index, id);
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out); out->iDocumentId = r->snapshot->identity; out->iRevision = r->snapshot->revision;
    out->iNodeId = id; out->iKind = n && doc_text_kind(n->kind) ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
    if (r->mode == XUI_DOC_SOURCE_TEXT) out->iKind = XUI_DOC_POSITION_SOURCE;
    out->iOffset = offset; out->iAffinity = XUI_DOC_AFTER;
    if (r->input) {
        uint64_t source = offset;
        if (r->mode == XUI_DOC_LIVE_MARKDOWN && id != DOC_ROOT &&
            doc_render_live_position_source(r, out, &source) != XUI_OK) return;
        (void)xuiDocumentPrepareSourcePosition(r->input, source, XUI_DOC_AFTER, out);
        return;
    }
    if (r->mode == XUI_DOC_LIVE_MARKDOWN) {
        int mapping; uint64_t source = offset;
        if (id == DOC_ROOT || xuiDocumentPositionToSource(r->snapshot, out, &source, &mapping) == XUI_OK) {
            out->iNodeId = DOC_ROOT; out->iKind = XUI_DOC_POSITION_SOURCE; out->iOffset = source;
        }
    }
}
static int doc_render_node_descends(doc_state* state, uint64_t id, uint64_t ancestor);
static void doc_render_visible_fragments(const doc_render_block* b,
    double first_y, double last_y, size_t* first, size_t* end)
{
    size_t low = 0, high = b->line_count, begin, finish;
    *first = 0; *end = b->fragment_count;
    if (!b->lines_sorted || !b->line_count) return;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        if (b->lines[middle].bottom < first_y) low = middle + 1;
        else high = middle;
    }
    begin = low; high = b->line_count;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        if (b->lines[middle].top <= last_y) low = middle + 1;
        else high = middle;
    }
    finish = low;
    *first = begin < b->line_count ? b->lines[begin].first : b->fragment_count;
    *end = finish > begin ? b->lines[finish - 1].end : *first;
}
static void doc_render_hit_fragments(const doc_render_block* b,
    double y, size_t* first, size_t* end)
{
    size_t low = 0, high = b->line_count, line;
    *first = 0; *end = b->fragment_count;
    if (!b->lines_sorted || !b->line_count) return;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        if (b->lines[middle].bottom < y) low = middle + 1;
        else high = middle;
    }
    line = low < b->line_count ? low : b->line_count - 1;
    if (line && y < b->lines[line].top &&
        y - b->lines[line - 1].bottom < b->lines[line].top - y) line--;
    *first = b->lines[line].first; *end = b->lines[line].end;
}
static void doc_render_line_fragments_at_x(const doc_render_block* b,
    double left, double right, size_t* first, size_t* end)
{
    size_t low, high, stop;
    if (!b->line_lazy || b->line_count != 1 || *first >= *end)
        return;
    stop = *end;
    low = *first; high = stop;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        const doc_fragment* f = &b->fragments[middle];
        if (f->x + f->width < left) low = middle + 1;
        else high = middle;
    }
    *first = low < stop ? low : stop - 1;
    low = *first; high = stop;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        if (b->fragments[middle].x <= right) low = middle + 1;
        else high = middle;
    }
    *end = low > *first ? low : *first + 1;
}
static void doc_render_grapheme(const doc_render_block* block, size_t at,
    size_t* first, size_t* end)
{
    *first = at; *end = at + 1;
    if (!(block->fragments[at].flags & DOC_GRAPHEME_MEMBER)) return;
    while (*first && (block->fragments[*first - 1].flags & DOC_GRAPHEME_JOIN))
        (*first)--;
    while (*end < block->fragment_count &&
        (block->fragments[*end - 1].flags & DOC_GRAPHEME_JOIN)) (*end)++;
}
static xui_doc_rect_t doc_render_grapheme_rect(const doc_render_block* block,
    size_t first, size_t end, double top)
{
    const doc_fragment* fragment = &block->fragments[first];
    double left = fragment->x, right = left + fragment->width;
    double upper = fragment->y, lower = upper + fragment->height;
    size_t i;
    for (i = first + 1; i < end; i++) {
        fragment = &block->fragments[i];
        left = fmin(left, fragment->x);
        right = fmax(right, fragment->x + fragment->width);
        upper = fmin(upper, fragment->y);
        lower = fmax(lower, fragment->y + fragment->height);
    }
    return (xui_doc_rect_t){left, top + upper, right - left, lower - upper};
}
static uint64_t doc_render_grapheme_visits(const doc_render_block* block,
    size_t first, size_t end)
{
    /* A standalone fragment is already counted by the caller's lookup/draw
     * loop. Its MEMBER fast path performs no additional fragment traversal. */
    if (end - first == 1) return 0;
    /* Boundary scans visit every interior edge plus at most two neighbours;
     * the bounding rectangle visits each cluster fragment once more. */
    return (uint64_t)(end - first) * 2 - 1 + (first > 0) +
        (end < block->fragment_count);
}
static int doc_render_same_inline_flow(xui_document_renderer renderer,
    const doc_fragment* first, const doc_fragment* next)
{
    doc_node *a, *b, *parent;
    if (first->node == next->node) return 1;
    a = doc_index_get(renderer->snapshot->state->index, first->node);
    b = doc_index_get(renderer->snapshot->state->index, next->node);
    if (!a || !b || !a->parent || a->parent != b->parent) return 0;
    parent = doc_index_get(renderer->snapshot->state->index, a->parent);
    return parent && (parent->kind == XUI_DOC_PARAGRAPH || parent->kind == XUI_DOC_HEADING);
}
static size_t doc_render_downstream_row(xui_document_renderer renderer,
    const doc_render_block* block, size_t end, uint64_t* visits)
{
    const doc_fragment* tail = &block->fragments[end - 1];
    size_t next;
    for (next = end; next < block->fragment_count; next++) {
        const doc_fragment* fragment = &block->fragments[next];
        (*visits)++;
        /* A forced break owns an explicit next insertion carrier, including
         * a terminal empty row. Preserve its metrics exactly. */
        if (tail->flags & DOC_FORCED_BREAK) return next;
        if (!doc_render_same_inline_flow(renderer, tail, fragment)) return SIZE_MAX;
        /* Logical baselines identify rows; mixed fonts and scripts can have
         * different glyph tops while sharing the same row. */
        if (fragment->y + fragment->baseline > tail->y + tail->baseline + .001)
            return next;
        /* Zero-length Text nodes keep their own insertion metrics, but must
         * not hide a wrapped glyph following them. Do not skip invisible
         * graphemes or structural carriers that happen to have zero width. */
        if (fragment->kind != XUI_DOC_TEXT || fragment->start != fragment->end ||
            fragment->run == UINT32_MAX || (fragment->flags & DOC_TRAILING_LINE)) break;
    }
    return SIZE_MAX;
}
static int doc_render_empty_wrap_edge(xui_document_renderer renderer,
    const doc_render_block* block, size_t at, uint64_t* visits)
{
    const doc_fragment* empty = &block->fragments[at];
    while (at) {
        const doc_fragment* previous = &block->fragments[--at];
        (*visits)++;
        if (!doc_render_same_inline_flow(renderer, empty, previous) ||
            fabs(previous->y + previous->baseline - empty->y - empty->baseline) > .001)
            return 0;
        if (previous->kind == XUI_DOC_TEXT && previous->start == previous->end &&
            previous->run != UINT32_MAX && !(previous->flags & DOC_TRAILING_LINE)) continue;
        return !(previous->flags & DOC_FORCED_BREAK);
    }
    /* A paragraph start or a carrier after a forced break owns its row even
     * when the following oversized glyph wraps onto a later row. */
    return 0;
}
static xui_doc_rect_t doc_render_grapheme_caret(xui_document_renderer renderer,
    const doc_render_block* block,
    const doc_fragment* chosen, const xui_doc_position_t* position, double top)
{
    size_t first, end;
    const doc_fragment *head, *tail;
    xui_doc_rect_t rect;
    int after, empty;
    doc_render_grapheme(block, (size_t)(chosen - block->fragments), &first, &end);
    renderer->stats.iCaretFragmentsExamined += doc_render_grapheme_visits(block, first, end);
    head = &block->fragments[first]; tail = &block->fragments[end - 1];
    empty = end - first == 1 && chosen->kind == XUI_DOC_TEXT &&
        chosen->start == chosen->end && !(chosen->flags & DOC_TRAILING_LINE) &&
        position->iAffinity == XUI_DOC_AFTER && doc_render_empty_wrap_edge(renderer,
            block, first, &renderer->stats.iCaretFragmentsExamined);
    rect = doc_render_grapheme_rect(block, first, end, top);
    if (position->iNodeId == head->node && position->iOffset == head->start)
        after = 0;
    else if (position->iNodeId == tail->node && position->iOffset == tail->end)
        after = tail->end > tail->start || end - first > 1;
    else after = position->iAffinity == XUI_DOC_AFTER;
    if ((empty || (after && position->iNodeId == tail->node &&
        position->iOffset == tail->end)) && position->iAffinity == XUI_DOC_AFTER) {
        size_t next = doc_render_downstream_row(renderer, block, end,
            &renderer->stats.iCaretFragmentsExamined);
        if (next != SIZE_MAX) {
            size_t next_first, next_end;
            doc_render_grapheme(block, next, &next_first, &next_end);
            renderer->stats.iCaretFragmentsExamined +=
                doc_render_grapheme_visits(block, next_first, next_end);
            rect = doc_render_grapheme_rect(block, next_first, next_end, top);
            if (block->fragments[next_first].bidi_level & 1) rect.x += rect.width;
            if (empty) {
                /* An empty styled node still determines the input font. Only
                 * its row anchor follows downstream affinity. */
                rect.y = top + block->fragments[next].y +
                    block->fragments[next].baseline - chosen->baseline;
                rect.height = chosen->height;
            }
            rect.width = 1;
            return rect;
        }
    }
    if (after != !!(head->bidi_level & 1)) rect.x += rect.width;
    rect.width = 1;
    return rect;
}
XUI_API int xuiDocumentRendererHitTest(xui_document_renderer r, double x, double y, xui_doc_position_t* out)
{
    size_t index, i, first, end, best = SIZE_MAX; doc_render_block* b; uint64_t cell = 0;
    double best_dy = DBL_MAX, best_dx = DBL_MAX, cell_area = DBL_MAX, top; int result;
    if (!r || !r->snapshot || !out || !isfinite(x) || !isfinite(y)) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_render_refresh_resources(r); if (result != XUI_OK) return result;
    if (!r->count) { doc_render_position(r, DOC_ROOT, 0, out); return XUI_OK; }
    do {
        result = doc_render_resolve_partial_before_y(r, y);
        if (result != XUI_OK) return result;
        index = doc_render_block_at(r, y);
        b = doc_render_block_at_index(r, index); top = doc_render_height_before(r, index);
        result = doc_render_materialize_visible(r, index, b, y - top + 1);
        if (result != XUI_OK) return result;
    } while (doc_render_block_at(r, y) != index);
    result = doc_render_cover_line(r, index, b, x, 0);
    if (result != XUI_OK) return result;
    doc_render_hit_fragments(b, y - top, &first, &end);
    doc_render_line_fragments_at_x(b, x, x, &first, &end);
    for (i = 0; i < b->box_count; i++) {
        doc_box* box = &b->boxes[i]; double area;
        if (box->kind != XUI_DOC_CELL || x < box->x || x >= box->x + box->width ||
            y < top + box->y || y >= top + box->y + box->height) continue;
        area = box->width * box->height;
        if (area < cell_area) { cell = box->node; cell_area = area; }
    }
    for (i = first; i < end; i++) {
        doc_fragment* f = &b->fragments[i];
        r->stats.iHitFragmentsExamined++;
        if (cell && !doc_render_node_descends(r->snapshot->state, f->node, cell)) continue;
        double dx = x < f->x ? f->x - x : x > f->x + f->width ? x - f->x - f->width : 0;
        double dy = y < top + f->y ? top + f->y - y : y > top + f->y + f->height ? y - top - f->y - f->height : 0;
        /* Pick the visual row before the horizontal caret. A weighted 2-D
         * distance can jump to the next row when clicking far past a short
         * line inside the same paragraph. */
        if (dy < best_dy || (dy == best_dy && dx < best_dx)) {
            best = i; best_dy = dy; best_dx = dx;
        }
    }
    if (best != SIZE_MAX) {
        /* Fragments on one visual row share a baseline even when font sizes,
         * scripts or inline objects give them different bounding boxes. */
        double baseline = b->fragments[best].y + b->fragments[best].baseline;
        best_dx = DBL_MAX;
        for (i = first; i < end; i++) {
            doc_fragment* f = &b->fragments[i]; double dx;
            r->stats.iHitFragmentsExamined++;
            if (cell && !doc_render_node_descends(r->snapshot->state, f->node, cell)) continue;
            if (fabs(f->y + f->baseline - baseline) > .001) continue;
            dx = x < f->x ? f->x - x : x > f->x + f->width ? x - f->x - f->width : 0;
            /* End uses a far-right coordinate. Subtracting finite fragment
             * widths from that value can round distinct distances to the
             * same double, so break ties by the actual visual edge. */
            if (dx < best_dx || (dx == best_dx &&
                ((x >= f->x + f->width && x >= b->fragments[best].x + b->fragments[best].width &&
                  f->x + f->width > b->fragments[best].x + b->fragments[best].width) ||
                 (x <= f->x && x <= b->fragments[best].x && f->x < b->fragments[best].x)))) {
                best = i; best_dx = dx;
            }
        }
    }
    if (best == SIZE_MAX) doc_render_position(r, cell ? cell : b->node, b->source_line ? b->source_start : 0, out);
    else {
        size_t cluster_first, cluster_end;
        xui_doc_rect_t rect;
        const doc_fragment* edge;
        int after;
        doc_render_grapheme(b, best, &cluster_first, &cluster_end);
        r->stats.iHitFragmentsExamined += doc_render_grapheme_visits(b, cluster_first, cluster_end);
        rect = doc_render_grapheme_rect(b, cluster_first, cluster_end, top);
        after = x >= rect.x + rect.width / 2;
        if (b->fragments[cluster_first].bidi_level & 1) after = !after;
        edge = &b->fragments[after ? cluster_end - 1 : cluster_first];
        if ((edge->flags & DOC_LINE_CONTROL) && !(edge->flags & DOC_TRAILING_LINE)) {
            /* A zero-width control is the end of the clicked row; placing
             * its downstream end would move the visible caret to another. */
            edge = &b->fragments[cluster_first];
            doc_render_position(r, edge->node, edge->start, out);
        } else if (edge->kind == XUI_DOC_HARD_BREAK) {
            doc_node* node = doc_index_get(r->snapshot->state->index, edge->node);
            doc_node* parent = node ? doc_index_get(r->snapshot->state->index, node->parent) : NULL;
            uint64_t child = parent ? doc_child_index(parent, node->id) : DOC_NONE;
            if (child == DOC_NONE) return XUI_DOC_ERROR_STALE;
            doc_render_position(r, parent->id, child, out);
        } else if (edge->bidi_active && doc_selectable_object_kind(edge->kind)) {
            doc_node* node = doc_index_get(r->snapshot->state->index, edge->node);
            doc_node* parent = node ? doc_index_get(r->snapshot->state->index, node->parent) : NULL;
            uint64_t child = parent ? doc_child_index(parent, node->id) : DOC_NONE;
            if (child == DOC_NONE) return XUI_DOC_ERROR_STALE;
            doc_render_position(r, parent->id, child + (after ? 1 : 0), out);
            if (after) out->iAffinity = XUI_DOC_BEFORE;
        } else {
            doc_render_position(r, edge->node, after ? edge->end : edge->start, out);
            if (after && edge->bidi_active) out->iAffinity = XUI_DOC_BEFORE;
            if (after && doc_render_downstream_row(r, b, cluster_end,
                &r->stats.iHitFragmentsExamined) != SIZE_MAX)
                out->iAffinity = XUI_DOC_BEFORE;
        }
    }
    return XUI_OK;
}
XUI_API int xuiDocumentRendererHitTaskMarker(xui_document_renderer r,
    double x, double y, xui_doc_node_id* item)
{
    size_t index;
    doc_render_block* block;
    doc_node* node;
    double top, width, left, height;
    int result;
    if (!r || !r->snapshot || !item || !isfinite(x) || !isfinite(y))
        return XUI_ERROR_INVALID_ARGUMENT;
    *item = 0;
    if (r->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    result = doc_render_refresh_resources(r);
    if (result != XUI_OK) return result;
    if (!r->count || x < 0 || y < 0) return XUI_ERROR_NOT_FOUND;
    do {
        result = doc_render_resolve_partial_before_y(r, y);
        if (result != XUI_OK) return result;
        index = doc_render_block_at(r, y);
        block = doc_render_block_at_index(r, index);
        top = doc_render_height_before(r, index);
        result = doc_render_materialize_visible(r, index, block, y - top + 1);
        if (result != XUI_OK) return result;
    } while (doc_render_block_at(r, y) != index);
    node = doc_index_get(r->snapshot->state->index, block->marker);
    if (!node || node->kind != XUI_DOC_LIST_ITEM ||
        !(node->attrs->iFlags & XUI_DOC_TASK)) return XUI_ERROR_NOT_FOUND;
    width = r->desc.fIndent * r->desc.fZoom;
    height = 24 * r->desc.fZoom;
    left = fmax(0, block->indent - width);
    if (width <= 0 || height <= 0 || x < left || x >= left + width ||
        y < top || y >= top + height) return XUI_ERROR_NOT_FOUND;
    *item = node->id;
    return XUI_OK;
}
static int doc_render_cell_hit(xui_document_renderer r, const doc_box* box, double top, xui_doc_cell_hit_t* out)
{
    doc_node* cell = doc_index_get(r->snapshot->state->index, box->node);
    if (!cell || cell->kind != XUI_DOC_CELL) return XUI_DOC_ERROR_SCHEMA;
    out->iTableId = box->table; out->iCellId = box->node;
    out->iRow = box->row; out->iColumn = box->column;
    out->iRowSpan = cell->attrs->iRowSpan; out->iColumnSpan = cell->attrs->iColumnSpan;
    out->tBounds = (xui_doc_rect_t){box->x, top + box->y, box->width, box->height};
    return XUI_OK;
}
XUI_API int xuiDocumentRendererHitTestCell(xui_document_renderer r, double x, double y, xui_doc_cell_hit_t* out)
{
    size_t index, i; doc_render_block* block; double top; int result;
    if (!r || !r->snapshot || !out || out->iSize != sizeof(*out) || !isfinite(x) || !isfinite(y))
        return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_render_refresh_resources(r); if (result != XUI_OK) return result;
    if (!r->count || x < 0 || y < 0) return XUI_ERROR_NOT_FOUND;
    do {
        result = doc_render_resolve_partial_before_y(r, y);
        if (result != XUI_OK) return result;
        index = doc_render_block_at(r, y);
        result = doc_render_materialize(r, index); if (result != XUI_OK) return result;
    } while (doc_render_block_at(r, y) != index);
    block = doc_render_block_at_index(r, index); top = doc_render_height_before(r, index);
    for (i = 0; i < block->box_count; i++) {
        const doc_box* box = &block->boxes[i]; double left, right, upper, lower;
        if (box->kind != XUI_DOC_CELL) continue;
        left = box->x; right = left + box->width;
        upper = top + box->y; lower = upper + box->height;
        if (x >= left && x < right && y >= upper && y < lower)
            return doc_render_cell_hit(r, box, top, out);
    }
    return XUI_ERROR_NOT_FOUND;
}
XUI_API int xuiDocumentRendererGetCellRect(xui_document_renderer r, xui_doc_node_id cell, xui_doc_cell_hit_t* out)
{
    doc_node* node; doc_render_block* block; size_t index, i; int result;
    if (!r || !r->snapshot || !cell || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    node = doc_index_get(r->snapshot->state->index, cell);
    if (!node || node->kind != XUI_DOC_CELL) return XUI_ERROR_NOT_FOUND;
    result = doc_render_refresh_resources(r); if (result != XUI_OK) return result;
    index = doc_render_find(r, cell); if (index == SIZE_MAX) return XUI_ERROR_NOT_FOUND;
    result = doc_render_materialize(r, index); if (result != XUI_OK) return result;
    block = doc_render_block_at_index(r, index);
    for (i = 0; i < block->box_count; i++)
        if (block->boxes[i].kind == XUI_DOC_CELL && block->boxes[i].node == cell)
            return doc_render_cell_hit(r, &block->boxes[i], doc_render_height_before(r, index), out);
    return XUI_ERROR_NOT_FOUND;
}
static int doc_render_node_descends(doc_state* state, uint64_t id, uint64_t ancestor)
{
    doc_node* node;
    for (node = doc_index_get(state->index, id); node;
         node = doc_index_get(state->index, node->parent)) {
        if (node->id == ancestor) return 1;
        if (!node->parent || node->parent == node->id) break;
    }
    return 0;
}
static void doc_render_rect_union(xui_doc_rect_t* out, double x, double y,
    double width, double height, int* found)
{
    double right = x + fmax(1, width), bottom = y + fmax(1, height);
    if (!*found) {
        *out = (xui_doc_rect_t){x, y, right - x, bottom - y}; *found = 1;
    } else {
        double left = fmin(out->x, x), top = fmin(out->y, y);
        right = fmax(out->x + out->width, right);
        bottom = fmax(out->y + out->height, bottom);
        *out = (xui_doc_rect_t){left, top, right - left, bottom - top};
    }
}
static int doc_render_partial_through_object(xui_document_renderer r,
    size_t index, doc_render_block* b, uint64_t id, doc_fragment** covered)
{
    int result;
    *covered = NULL;
    if (!b || !b->partial) return XUI_OK;
    for (;;) {
        size_t i;
        for (i = 0; i < b->fragment_count; i++) {
            doc_fragment* fragment = &b->fragments[i];
            if (fragment->node == id && fragment->run == UINT32_MAX &&
                fragment->y + fragment->height < b->partial_bottom - .001) {
                *covered = fragment;
                return XUI_OK;
            }
        }
        result = doc_render_extend_partial(r, index, b, b->partial_bottom + 1);
        if (result == XUI_ERROR_NOT_FOUND) return XUI_OK;
        if (result != XUI_OK) return result;
    }
}
static int doc_render_partial_through_text(xui_document_renderer r,
    size_t index, doc_render_block* b, const xui_doc_position_t* p,
    doc_fragment** covered);
XUI_API int xuiDocumentRendererGetNodeRect(xui_document_renderer r, xui_doc_node_id id, xui_doc_rect_t* out)
{
    doc_render_block* block; doc_state* state; doc_node* node; size_t index, i; double top;
    int found = 0, result;
    if (!r || !r->snapshot || !id || !out) return XUI_ERROR_INVALID_ARGUMENT;
    if (r->mode != XUI_DOC_VISUAL) return XUI_ERROR_UNSUPPORTED;
    state = r->snapshot->state;
    node = doc_index_get(state->index, id);
    if (!node) return XUI_ERROR_NOT_FOUND;
    result = doc_render_refresh_resources(r); if (result != XUI_OK) return result;
    if (id == DOC_ROOT) {
        xui_doc_rect_t size; int exact;
        result = xuiDocumentRendererGetSize(r, &size, &exact);
        if (result == XUI_OK) *out = (xui_doc_rect_t){0, 0, size.width, size.height};
        return result;
    }
    if (node->kind == XUI_DOC_QUOTE || node->kind == XUI_DOC_LIST ||
        node->kind == XUI_DOC_LIST_ITEM || node->kind == XUI_DOC_FOOTNOTE) {
        size_t first = SIZE_MAX, last = SIZE_MAX;
        for (i = 0; i < r->count; i++) {
            block = doc_render_block_at_index(r, i);
            if (block && doc_render_node_descends(state, block->node, id)) {
                if (first == SIZE_MAX) first = i;
                last = i;
            }
        }
        if (first == SIZE_MAX) return XUI_ERROR_NOT_FOUND;
        top = doc_render_height_before(r, first);
        block = doc_render_block_at_index(r, last);
        *out = (xui_doc_rect_t){0, top, r->width,
            fmax(1, doc_render_height_before(r, last) + block->height - top)};
        return XUI_OK;
    }
    index = doc_render_find(r, id); if (index == SIZE_MAX) return XUI_ERROR_NOT_FOUND;
    if (r->blocks[index].code_slice && r->blocks[index].node == id) {
        size_t last = index + r->blocks[index].slice_count;
        double width = r->width;
        top = doc_render_height_before(r, index);
        for (i = index; i < last; i++) if (r->blocks[i].width > width) width = r->blocks[i].width;
        *out = (xui_doc_rect_t){0, top, width, fmax(1, doc_render_height_before(r, last) - top)};
        return XUI_OK;
    }
    block = doc_render_block_at_index(r, index);
    if (block && !block->measured && (doc_selectable_object_kind(node->kind) ||
        node->kind == XUI_DOC_TEXT || node->kind == XUI_DOC_FOOTNOTE_REF)) {
        result = doc_render_materialize_visible(r, index, block, 1);
        if (result != XUI_OK) return result;
    }
    if (block && block->partial && (node->kind == XUI_DOC_TEXT ||
        node->kind == XUI_DOC_FOOTNOTE_REF)) {
        xui_doc_position_t tail = {0}; doc_fragment* covered = NULL;
        tail.iKind = XUI_DOC_POSITION_TEXT;
        tail.iNodeId = id; tail.iOffset = doc_seq_size(node->text);
        tail.iAffinity = XUI_DOC_AFTER;
        result = doc_render_partial_through_text(r, index, block, &tail,
            &covered);
        if (result != XUI_OK) return result;
        if (covered) {
            top = doc_render_height_before(r, index);
            for (i = 0; i < block->fragment_count; ) {
                size_t first, end, j;
                xui_doc_rect_t rect;
                doc_render_grapheme(block, i, &first, &end);
                rect = doc_render_grapheme_rect(block, first, end, top);
                if (rect.y + rect.height < top + block->partial_bottom - .001) {
                    for (j = first; j < end; j++) if (block->fragments[j].node == id) {
                        doc_render_rect_union(out, rect.x, rect.y, rect.width, rect.height, &found);
                        break;
                    }
                }
                i = end;
            }
            if (found) return XUI_OK;
        }
    }
    if (block && block->partial && doc_selectable_object_kind(node->kind)) {
        doc_fragment* covered = NULL;
        result = doc_render_partial_through_object(r, index, block, id,
            &covered);
        if (result != XUI_OK) return result;
        if (covered) {
            top = doc_render_height_before(r, index);
            *out = (xui_doc_rect_t){covered->x, top + covered->y,
                fmax(1, covered->width), fmax(1, covered->height)};
            return XUI_OK;
        }
    }
    result = doc_render_materialize(r, index); if (result != XUI_OK) return result;
    block = doc_render_block_at_index(r, index); top = doc_render_height_before(r, index);
    if (block->node == id) {
        *out = (xui_doc_rect_t){0, top, fmax(r->width, block->width), fmax(1, block->height)};
        return XUI_OK;
    }
    for (i = 0; i < block->fragment_count; ) {
        size_t first, end, j;
        doc_render_grapheme(block, i, &first, &end);
        for (j = first; j < end; j++) {
            if (doc_render_node_descends(state, block->fragments[j].node, id)) {
                xui_doc_rect_t rect = doc_render_grapheme_rect(block, first, end, top);
                doc_render_rect_union(out, rect.x, rect.y, rect.width, rect.height, &found);
                break;
            }
        }
        i = end;
    }
    for (i = 0; i < block->box_count; i++) {
        const doc_box* box = &block->boxes[i];
        if (doc_render_node_descends(state, box->node, id))
            doc_render_rect_union(out, box->x, top + box->y,
                box->width, box->height, &found);
    }
    if (found) return XUI_OK;
    *out = (xui_doc_rect_t){0, top, fmax(r->width, block->width), fmax(1, block->height)};
    return XUI_OK;
}
static doc_fragment* doc_render_caret_fragment(xui_document_renderer r,
    doc_render_block* b, const xui_doc_position_t* p)
{
    size_t i;
    for (i = 0; i < b->run_count; i++) {
        doc_render_run* run = &b->runs[i]; size_t low, high;
        doc_fragment* chosen;
        if (run->node != p->iNodeId) continue;
        if (p->iOffset < run->base_offset ||
            p->iOffset - run->base_offset > run->bytes) continue;
        if (p->iAffinity == XUI_DOC_AFTER &&
            p->iOffset - run->base_offset == run->bytes &&
            i + 1 < b->run_count && b->runs[i + 1].node == p->iNodeId &&
            b->runs[i + 1].base_offset == p->iOffset) continue;
        low = run->first_fragment; high = run->end_fragment;
        while (low < high) {
            size_t middle = low + (high - low) / 2;
            r->stats.iCaretFragmentsExamined++;
            if (b->fragments[middle].end < p->iOffset) low = middle + 1;
            else high = middle;
        }
        if (low >= run->end_fragment) continue;
        chosen = &b->fragments[low]; r->stats.iCaretFragmentsExamined++;
        if (p->iOffset < chosen->start || p->iOffset > chosen->end) continue;
        if (p->iOffset == chosen->end && low + 1 < run->end_fragment &&
            b->fragments[low + 1].start == p->iOffset &&
            (p->iAffinity == XUI_DOC_AFTER || (b->fragments[low + 1].flags & DOC_TRAILING_LINE))) {
            chosen = &b->fragments[low + 1]; r->stats.iCaretFragmentsExamined++;
        }
        return chosen;
    }
    if (p->iKind == XUI_DOC_POSITION_TEXT) {
        doc_node* node = doc_index_get(r->snapshot->state->index,
            p->iNodeId);
        if (node && (node->kind == XUI_DOC_TEXT ||
            node->kind == XUI_DOC_FOOTNOTE_REF ||
            node->kind == XUI_DOC_CODE_BLOCK ||
            node->kind == XUI_DOC_FRONT_MATTER)) return NULL;
    }
    /* Empty structural fragments and inline objects do not have a text run. */
    for (i = 0; i < b->fragment_count; i++) {
        doc_fragment* f = &b->fragments[i];
        r->stats.iCaretFragmentsExamined++;
        if (f->node == p->iNodeId && p->iOffset >= f->start && p->iOffset <= f->end)
            return f;
    }
    return NULL;
}
/* A text boundary is final once its fragment belongs to a committed line.
 * Keep extending the saved paragraph projection until that happens; a missing
 * next cut leaves the caller to finish the block by the ordinary path. */
static int doc_render_partial_through_text(xui_document_renderer r,
    size_t index, doc_render_block* b, const xui_doc_position_t* p,
    doc_fragment** covered)
{
    doc_node* node;
    int result;
    *covered = NULL;
    if (!b || !b->partial || p->iKind != XUI_DOC_POSITION_TEXT)
        return XUI_OK;
    node = doc_index_get(r->snapshot->state->index, p->iNodeId);
    if (!node || (node->kind != XUI_DOC_TEXT &&
        node->kind != XUI_DOC_FOOTNOTE_REF)) return XUI_OK;
    for (;;) {
        doc_fragment* chosen = doc_render_caret_fragment(r, b, p);
        double caret_bottom = chosen ? chosen->y + chosen->height : b->partial_bottom;
        if (chosen && p->iAffinity == XUI_DOC_AFTER &&
            (chosen->end == p->iOffset || (chosen->flags & DOC_GRAPHEME_MEMBER))) {
            xui_doc_rect_t caret = doc_render_grapheme_caret(r, b, chosen, p, 0);
            caret_bottom = caret.y + caret.height;
        }
        if (chosen && caret_bottom < b->partial_bottom - .001) {
            *covered = chosen;
            return XUI_OK;
        }
        result = doc_render_extend_partial(r, index, b, b->partial_bottom + 1);
        if (result == XUI_ERROR_NOT_FOUND) return XUI_OK;
        if (result != XUI_OK) return result;
    }
}
static int doc_render_partial_through_range_end(xui_document_renderer r,
    size_t index, doc_render_block* b, const xui_doc_position_t* end,
    doc_fragment** covered)
{
    doc_node *parent, *child;
    uint64_t child_id;
    *covered = NULL;
    if (!b) return XUI_OK;
    if (b->line_partial) {
        uint64_t offset;
        int result;
        if ((end->iKind != XUI_DOC_POSITION_TEXT &&
            end->iKind != XUI_DOC_POSITION_SOURCE) ||
            end->iNodeId != b->node ||
            end->iOffset < b->text_start ||
            end->iOffset > b->text_end) return XUI_OK;
        offset = end->iOffset < b->text_end ? end->iOffset + 1 :
            end->iOffset;
        result = doc_render_cover_line(r, index, b, 0, offset);
        if (result != XUI_OK) return result;
        *covered = doc_render_caret_fragment(r, b, end);
        return XUI_OK;
    }
    if (!b->partial) return XUI_OK;
    if (end->iKind == XUI_DOC_POSITION_TEXT)
        return doc_render_partial_through_text(r, index, b, end, covered);
    if (end->iKind != XUI_DOC_POSITION_GAP || !end->iOffset ||
        end->iNodeId != b->node) return XUI_OK;
    parent = doc_index_get(r->snapshot->state->index, end->iNodeId);
    if (!parent || end->iOffset > doc_seq_size(parent->children))
        return XUI_OK;
    child_id = doc_seq_get_id(parent->children, end->iOffset - 1);
    child = doc_index_get(r->snapshot->state->index, child_id);
    if (!child) return XUI_OK;
    if (doc_selectable_object_kind(child->kind))
        return doc_render_partial_through_object(r, index, b,
            child_id, covered);
    if (child->kind == XUI_DOC_TEXT || child->kind == XUI_DOC_FOOTNOTE_REF) {
        xui_doc_position_t tail = *end;
        tail.iKind = XUI_DOC_POSITION_TEXT;
        tail.iNodeId = child_id;
        tail.iOffset = doc_seq_size(child->text);
        tail.iAffinity = XUI_DOC_AFTER;
        return doc_render_partial_through_text(r, index, b,
            &tail, covered);
    }
    return XUI_OK;
}
static int doc_render_object_edge_caret(xui_document_renderer r, uint64_t id,
    int after, xui_doc_rect_t* out)
{
    size_t index = doc_render_find(r, id), i;
    doc_render_block* block;
    int result;
    if (index == SIZE_MAX) return XUI_ERROR_NOT_FOUND;
    result = doc_render_materialize(r, index); if (result != XUI_OK) return result;
    block = doc_render_block_at_index(r, index);
    for (i = 0; i < block->fragment_count; i++) {
        doc_fragment* fragment = &block->fragments[i];
        if (fragment->node != id || fragment->run != UINT32_MAX) continue;
        *out = (xui_doc_rect_t){fragment->x + ((after != !!(fragment->bidi_level & 1)) ? fragment->width : 0),
            doc_render_height_before(r, index) + fragment->y, 1, fragment->height};
        return XUI_OK;
    }
    return XUI_ERROR_NOT_FOUND;
}
XUI_API int xuiDocumentRendererGetCaretRect(xui_document_renderer r, const xui_doc_position_t* p, xui_doc_rect_t* out)
{
    size_t index; doc_render_block* b; double top; int result; xui_doc_position_t clamped;
    if (!r || !r->snapshot || !p || !out) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_render_refresh_resources(r); if (result != XUI_OK) return result;
    { int order; result = doc_render_compare(r, p, p, &order); if (result != XUI_OK) return result; }
    if (p->iDocumentId != r->snapshot->identity || p->iRevision != r->snapshot->revision) return XUI_DOC_ERROR_STALE;
    if (!r->count) { *out = (xui_doc_rect_t){0, 0, 1, 20 * r->desc.fZoom}; return XUI_OK; }
    if (p->iKind == XUI_DOC_POSITION_SOURCE && r->mode == XUI_DOC_LIVE_MARKDOWN) {
        if (r->live_fallback || !r->live_rows) index = doc_render_source_at(r, p->iOffset);
        else if (r->live_rows && p->iOffset >= r->live_start && p->iOffset <= r->live_end)
            index = r->live_first + doc_render_live_row_at(r, p->iOffset);
        else index = SIZE_MAX;
        if (index != SIZE_MAX) {
            b = &r->blocks[index];
            if (b->source_line && p->iOffset >= b->source_start) {
                char crlf[2] = {0};
                if (p->iOffset <= b->source_end) goto located;
                if (p->iOffset == b->source_end + 1 && b->source_end + 1 < doc_seq_size(doc_render_source(r))) {
                    doc_seq_read(doc_render_source(r), b->source_end, crlf, 2);
                    if (crlf[0] == '\r' && crlf[1] == '\n') goto located;
                }
            }
        }
        /* Inactive source syntax projects to the nearest semantic position.
         * The owning View activates its caret before asking for geometry. */
        { xui_doc_position_t mapped; uint64_t base = p->iOffset; int mapping;
          if (r->input) { result = doc_render_live_input_to_base(r, p->iOffset, &base); if (result != XUI_OK) return result; }
          result = xuiDocumentSourceToPositionEx(r->snapshot, base, p->iAffinity, &mapped, &mapping);
          return result == XUI_OK ? xuiDocumentRendererGetCaretRect(r, &mapped, out) : result; }
    }
    if (p->iKind == XUI_DOC_POSITION_GAP) {
        doc_node* n = doc_index_get(r->snapshot->state->index, p->iNodeId);
        uint64_t count = doc_seq_size(n->children);
        if (p->iAffinity == XUI_DOC_BEFORE && p->iOffset && p->iOffset <= count) {
            doc_node* previous = doc_index_get(r->snapshot->state->index,
                doc_seq_get_id(n->children, p->iOffset - 1));
            size_t block_index = previous ? doc_render_find(r, previous->id) : SIZE_MAX;
            doc_render_block* previous_block = block_index == SIZE_MAX ? NULL : doc_render_block_at_index(r, block_index);
            if (previous && doc_selectable_object_kind(previous->kind) && previous_block && previous_block->visual_fragments) {
                result = doc_render_object_edge_caret(r, previous->id, 1, out);
                if (result != XUI_ERROR_NOT_FOUND) return result;
            }
        }
        if (count && !(p->iNodeId == DOC_ROOT && p->iOffset == count && doc_render_block_at_index(r, r->count - 1)->virtual_gap)) {
            int end = p->iOffset == count; xui_doc_position_t edge = *p;
            n = doc_index_get(r->snapshot->state->index, doc_seq_get_id(n->children, end ? count - 1 : p->iOffset));
            while (doc_seq_size(n->children)) n = doc_index_get(r->snapshot->state->index, doc_seq_get_id(n->children, end ? doc_seq_size(n->children) - 1 : 0));
            if (end && n->kind == XUI_DOC_HARD_BREAK) {
                doc_node* parent = doc_index_get(r->snapshot->state->index, n->parent);
                index = doc_render_find(r, n->id);
                if (!parent || index == SIZE_MAX) return XUI_ERROR_NOT_FOUND;
                clamped = edge; clamped.iNodeId = parent->id;
                clamped.iKind = XUI_DOC_POSITION_GAP;
                clamped.iOffset = doc_seq_size(parent->children);
                p = &clamped;
                goto located;
            }
            if (doc_selectable_object_kind(n->kind)) {
                result = doc_render_object_edge_caret(r, n->id, end, out);
                if (result != XUI_ERROR_NOT_FOUND) return result;
            }
            edge.iNodeId = n->id; edge.iKind = doc_text_kind(n->kind) ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
            edge.iOffset = end ? doc_seq_size(n->text) : 0;
            return xuiDocumentRendererGetCaretRect(r, &edge, out);
        }
    }
    if (p->iKind == XUI_DOC_POSITION_SOURCE && r->mode == XUI_DOC_SOURCE_TEXT) {
        index = doc_render_source_at(r, p->iOffset);
    } else { index = doc_render_find_text(r, p->iNodeId, p->iOffset, p->iAffinity); if (index == SIZE_MAX) return XUI_ERROR_NOT_FOUND; }
located:
    result = doc_render_resolve_partial_before_index(r, index, 0, NULL);
    if (result != XUI_OK) return result;
    b = doc_render_block_at_index(r, index);
    if (b && !b->measured && (p->iKind == XUI_DOC_POSITION_TEXT ||
        p->iKind == XUI_DOC_POSITION_SOURCE)) {
        result = doc_render_materialize_visible(r, index, b, 1);
        if (result != XUI_OK) return result;
    }
    if (b && b->line_partial && (p->iKind == XUI_DOC_POSITION_TEXT ||
        p->iKind == XUI_DOC_POSITION_SOURCE)) {
        uint64_t offset = p->iOffset < b->text_end ? p->iOffset + 1 :
            p->iOffset;
        doc_fragment* chosen;
        result = doc_render_cover_line(r, index, b, 0, offset);
        if (result != XUI_OK) return result;
        chosen = doc_render_caret_fragment(r, b, p);
        if (chosen) {
            top = doc_render_height_before(r, index);
            *out = doc_render_grapheme_caret(r, b, chosen, p, top);
            return XUI_OK;
        }
    }
    if (b && b->partial) {
        doc_fragment* chosen = NULL;
        result = doc_render_partial_through_text(r, index, b, p, &chosen);
        if (result != XUI_OK) return result;
        if (chosen) {
            top = doc_render_height_before(r, index);
            *out = doc_render_grapheme_caret(r, b, chosen, p, top);
            return XUI_OK;
        }
    }
    result = doc_render_materialize(r, index); if (result != XUI_OK) return result;
    b = doc_render_block_at_index(r, index); top = doc_render_height_before(r, index);
    if (b->source_line && p->iOffset > b->source_end) { clamped = *p; clamped.iOffset = b->source_end; p = &clamped; }
    {
        doc_fragment* chosen = doc_render_caret_fragment(r, b, p);
        if (chosen) {
            *out = doc_render_grapheme_caret(r, b, chosen, p, top);
            return XUI_OK;
        }
    }
    return XUI_ERROR_NOT_FOUND;
}
int doc_render_visual_move(xui_document_renderer r, const xui_doc_position_t* p,
    int right, xui_doc_position_t* out, int* logical_right)
{
    xui_doc_rect_t caret, candidate = {0}; doc_render_block* block;
    doc_node* node; uint64_t cell = 0;
    size_t index, i; double top, distance = DBL_MAX;
    int result, found = 0, base = 0;
    if (!out || !logical_right) return XUI_ERROR_INVALID_ARGUMENT;
    *logical_right = right;
    result = xuiDocumentRendererGetCaretRect(r, p, &caret);
    if (result != XUI_OK) return result;
    index = doc_render_block_at(r, caret.y + caret.height * .5);
    block = doc_render_block_at_index(r, index);
    if (!block || !block->visual_fragments) return XUI_ERROR_NOT_FOUND;
    top = doc_render_height_before(r, index);
    node = p->iKind == XUI_DOC_POSITION_SOURCE ? NULL :
        doc_index_get(r->snapshot->state->index, p->iNodeId);
    while (node) {
        if (node->kind == XUI_DOC_CELL) { cell = node->id; break; }
        node = node->parent ? doc_index_get(r->snapshot->state->index, node->parent) : NULL;
    }
    for (i = 0; i < block->fragment_count; ) {
        size_t head, tail; xui_doc_rect_t rect; double edge, delta;
        const doc_fragment* fragment;
        doc_render_grapheme(block, block->visual_fragments[i], &head, &tail);
        i += tail - head; fragment = &block->fragments[head];
        if (cell && !doc_render_node_descends(r->snapshot->state, fragment->node, cell)) continue;
        rect = doc_render_grapheme_rect(block, head, tail, top);
        if (fmin(rect.y + rect.height, caret.y + caret.height) - fmax(rect.y, caret.y) <= .001) continue;
        base = fragment->bidi_base;
        if (rect.width <= 0) continue;
        edge = right ? rect.x + rect.width : rect.x;
        delta = right ? edge - caret.x : caret.x - edge;
        if (delta > .001 && delta < distance) { candidate = rect; distance = delta; found = 1; }
    }
    if (found) {
        double inset = fmin(.0001, candidate.width * .25);
        return xuiDocumentRendererHitTest(r, right ? candidate.x + candidate.width - inset : candidate.x + inset,
            candidate.y + candidate.height * .5, out);
    }
    *logical_right = right != !!(base & 1);
    /* Crossing a wrapped row follows paragraph reading order. At a block or
     * table-cell edge the editor's logical projection selects the next block. */
    distance = DBL_MAX;
    for (i = 0; i < block->fragment_count; ) {
        size_t head, tail; xui_doc_rect_t rect; double delta;
        const doc_fragment* fragment;
        doc_render_grapheme(block, block->visual_fragments[i], &head, &tail);
        i += tail - head; fragment = &block->fragments[head];
        if (cell && !doc_render_node_descends(r->snapshot->state, fragment->node, cell)) continue;
        rect = doc_render_grapheme_rect(block, head, tail, top);
        if (fmin(rect.y + rect.height, caret.y + caret.height) - fmax(rect.y, caret.y) > .001) continue;
        delta = *logical_right ? rect.y + rect.height * .5 - (caret.y + caret.height * .5) :
            caret.y + caret.height * .5 - (rect.y + rect.height * .5);
        if (delta > .001 && delta < distance) {
            candidate = rect; distance = delta; found = 1; base = fragment->bidi_base;
        }
    }
    if (!found) return XUI_ERROR_NOT_FOUND;
    {
        double left = DBL_MAX, right_edge = -DBL_MAX;
        for (i = 0; i < block->fragment_count; ) {
            size_t head, tail; xui_doc_rect_t rect;
            doc_render_grapheme(block, block->visual_fragments[i], &head, &tail); i += tail - head;
            if (cell && !doc_render_node_descends(r->snapshot->state, block->fragments[head].node, cell)) continue;
            rect = doc_render_grapheme_rect(block, head, tail, top);
            if (fmin(rect.y + rect.height, candidate.y + candidate.height) - fmax(rect.y, candidate.y) <= .001) continue;
            left = fmin(left, rect.x); right_edge = fmax(right_edge, rect.x + rect.width);
        }
        if (left == DBL_MAX) return XUI_ERROR_NOT_FOUND;
        return xuiDocumentRendererHitTest(r, *logical_right != !!(base & 1) ? left : right_edge,
            candidate.y + candidate.height * .5, out);
    }
}
static xui_rect_t doc_render_rect(double x, double y, double w, double h)
{
    double left = floor(x + .5), top = floor(y + .5);
    return (xui_rect_t){(float)left, (float)top, (float)(floor(x + w + .5) - left), (float)(floor(y + h + .5) - top)};
}
static int doc_render_visible(xui_rect_t rect, xui_rect_t clip)
{
    return rect.fX <= clip.fX + clip.fW && rect.fX + rect.fW >= clip.fX && rect.fY <= clip.fY + clip.fH && rect.fY + rect.fH >= clip.fY;
}
static int doc_render_selected(xui_document_renderer r, const doc_fragment* f, const xui_doc_range_t* range)
{
    xui_doc_position_t start, end; const xui_doc_position_t *a, *b; int order, left, right;
    if (!range || range->tAnchor.iRevision != r->snapshot->revision ||
        range->tCaret.iRevision != r->snapshot->revision) return 0;
    if (doc_render_compare(r, &range->tAnchor, &range->tCaret, &order) != XUI_OK || !order) return 0;
    a = order < 0 ? &range->tAnchor : &range->tCaret; b = order < 0 ? &range->tCaret : &range->tAnchor;
    if (f->run == UINT32_MAX && doc_selectable_object_kind(f->kind)) {
        doc_node* object = doc_index_get(r->snapshot->state->index, f->node);
        doc_node* parent = object ? doc_index_get(r->snapshot->state->index, object->parent) : NULL;
        uint64_t index = parent ? doc_child_index(parent, f->node) : DOC_NONE;
        if (index == DOC_NONE) return 0;
        doc_render_position(r, parent->id, index, &start);
        doc_render_position(r, parent->id, index + 1, &end);
    } else {
        if (!f->end) return 0;
        doc_render_position(r, f->node, f->start, &start);
        doc_render_position(r, f->node, f->end, &end);
    }
    return doc_render_compare(r, a, &end, &left) == XUI_OK && left < 0 &&
        doc_render_compare(r, b, &start, &right) == XUI_OK && right > 0;
}
static int doc_render_grapheme_selected(xui_document_renderer r,
    const doc_render_block* block, size_t first, size_t end,
    const xui_doc_range_t* range)
{
    size_t i;
    for (i = first; i < end; i++)
        if (doc_render_selected(r, &block->fragments[i], range)) return 1;
    return 0;
}
static void doc_render_range_emit(xui_doc_rect_t* rects, uint64_t capacity,
    uint64_t* count, const xui_doc_rect_t* rect)
{
    if (*count < capacity) rects[*count] = *rect;
    (*count)++;
}
static size_t doc_render_range_descendant(xui_document_renderer r,
    uint64_t id, int last)
{
    doc_node* node = doc_index_get(r->snapshot->state->index, id);
    size_t index = doc_render_find(r, id);
    uint64_t count, i;
    if (index != SIZE_MAX) {
        doc_render_block* block = &r->blocks[index];
        if (last && block->admonition_title && block->node == id && node) {
            count = doc_seq_size(node->children);
            for (i = count; i > 0; i--) {
                uint64_t child = doc_seq_get_id(node->children, i - 1);
                size_t descendant = doc_render_range_descendant(r, child, 1);
                if (descendant != SIZE_MAX) return descendant;
            }
        }
        return last && block->code_slice && block->node == id ?
            index + block->slice_count - 1 : index;
    }
    if (!node) return SIZE_MAX;
    count = doc_seq_size(node->children);
    for (i = 0; i < count; i++) {
        uint64_t child = doc_seq_get_id(node->children, last ? count - 1 - i : i);
        index = doc_render_range_descendant(r, child, last);
        if (index != SIZE_MAX) return index;
    }
    return SIZE_MAX;
}
static size_t doc_render_range_bound(xui_document_renderer r,
    const xui_doc_position_t* position, int first)
{
    doc_node* node = doc_index_get(r->snapshot->state->index,
        position->iNodeId);
    uint64_t count, child;
    if (!node || position->iKind != XUI_DOC_POSITION_GAP)
        return doc_render_find_text(r, position->iNodeId,
            position->iOffset, position->iAffinity);
    count = doc_seq_size(node->children);
    if (count && (first ? position->iOffset < count : position->iOffset > 0)) {
        child = doc_seq_get_id(node->children,
            first ? position->iOffset : position->iOffset - 1);
        return doc_render_range_descendant(r, child, !first);
    }
    return doc_render_find(r, position->iNodeId);
}
int doc_render_position_may_bidi(xui_document_renderer r, const xui_doc_position_t* p)
{
    size_t index;
    doc_render_block* block;
    if (p->iKind == XUI_DOC_POSITION_SOURCE) {
        if (r->mode == XUI_DOC_SOURCE_TEXT || r->live_fallback) index = doc_render_source_at(r, p->iOffset);
        else if (r->mode == XUI_DOC_LIVE_MARKDOWN && r->live_rows &&
            p->iOffset >= r->live_start && p->iOffset <= r->live_end)
            index = r->live_first + doc_render_live_row_at(r, p->iOffset);
        else return 1;
    } else index = doc_render_range_bound(r, p, p->iAffinity != XUI_DOC_BEFORE);
    if (index == SIZE_MAX) return 1;
    block = doc_render_block_at_index(r, index);
    if (!block) return 1;
    if (block->visual_fragments) return 1;
    /* An ASCII horizontal prefix already certifies that the unseen suffix
     * cannot contain directional controls. Do not materialize its far end
     * merely to collapse a selection towards the visible start. */
    return !block->measured && !block->line_lazy && !block->virtual_gap;
}
XUI_API int xuiDocumentRendererGetRangeRects(xui_document_renderer r,
    const xui_doc_range_t* range, xui_doc_rect_t* rects,
    uint64_t capacity, uint64_t* total)
{
    const xui_doc_position_t *start, *end;
    xui_doc_rect_t pending = {0};
    size_t first = 0, last, block_index, pending_block = SIZE_MAX;
    uint64_t count = 0; int order, result, has_pending = 0;
    if (total) *total = 0;
    if (!r || !r->snapshot || !range || !total || (capacity && !rects))
        return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_render_refresh_resources(r); if (result != XUI_OK) return result;
    result = doc_render_compare(r, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    if (!order || !r->count) return XUI_OK;
    start = order < 0 ? &range->tAnchor : &range->tCaret;
    end = order < 0 ? &range->tCaret : &range->tAnchor;
    last = r->count - 1;
    if (r->mode == XUI_DOC_SOURCE_TEXT &&
        start->iKind == XUI_DOC_POSITION_SOURCE &&
        end->iKind == XUI_DOC_POSITION_SOURCE) {
        first = doc_render_source_at(r, start->iOffset);
        last = doc_render_source_at(r, end->iOffset);
    } else if (start->iKind != XUI_DOC_POSITION_SOURCE &&
        end->iKind != XUI_DOC_POSITION_SOURCE) {
        size_t a = doc_render_range_bound(r, start, 1);
        size_t b = doc_render_range_bound(r, end, 0);
        if (a != SIZE_MAX && b != SIZE_MAX && a <= b) {
            first = a; last = b;
        }
    }
    if (r->mode == XUI_DOC_SOURCE_TEXT) {
        result = doc_render_resolve_partial_before_index(r, first, 0, NULL);
        if (result != XUI_OK) return result;
    }
    for (block_index = first; block_index <= last; block_index++) {
        doc_render_block* block = doc_render_block_at_index(r, block_index);
        doc_fragment* covered = NULL;
        double top; size_t i;
        if (block_index == last && block && !block->measured &&
            ((end->iKind == XUI_DOC_POSITION_TEXT &&
                doc_render_find_text(r, end->iNodeId, end->iOffset,
                    end->iAffinity) == block_index) ||
             (end->iKind == XUI_DOC_POSITION_SOURCE &&
                block->source_line &&
                end->iOffset >= block->source_start &&
                end->iOffset <= block->source_end) ||
             (end->iKind == XUI_DOC_POSITION_GAP &&
                end->iNodeId == block->node))) {
            result = doc_render_materialize_visible(r, block_index, block, 1);
            if (result != XUI_OK) return result;
        }
        if (block_index == last)
            result = doc_render_partial_through_range_end(r, block_index,
                block, end, &covered);
        else result = XUI_OK;
        if (result != XUI_OK) return result;
        if (!covered) {
            result = doc_render_materialize(r, block_index);
            if (result != XUI_OK) return result;
        }
        block = doc_render_block_at_index(r, block_index);
        top = doc_render_height_before(r, block_index);
        for (i = 0; i < block->fragment_count; ) {
            size_t cluster_first, cluster_end;
            xui_doc_rect_t next;
            int selected;
            doc_render_grapheme(block, block->visual_fragments ? block->visual_fragments[i] : i,
                &cluster_first, &cluster_end);
            selected = doc_render_grapheme_selected(r, block, cluster_first,
                cluster_end, range);
            next = doc_render_grapheme_rect(block, cluster_first, cluster_end, top);
            i += cluster_end - cluster_first;
            /* Invisible source controls create no physical gap. Keep a
             * neighbouring visual selection connected across those units. */
            if (next.width <= 0 || next.height <= 0) continue;
            if (!selected) {
                if (has_pending) {
                    doc_render_range_emit(rects, capacity, &count, &pending);
                    has_pending = 0;
                }
                continue;
            }
            if (has_pending && pending_block == block_index &&
                fabs(pending.y - next.y) < .001 &&
                fabs(pending.height - next.height) < .001 &&
                next.x >= pending.x - .001 &&
                next.x <= pending.x + pending.width + .001) {
                double right = fmax(pending.x + pending.width, next.x + next.width);
                pending.width = right - pending.x;
            } else {
                if (has_pending) doc_render_range_emit(rects, capacity, &count, &pending);
                pending = next; pending_block = block_index; has_pending = 1;
            }
        }
    }
    if (has_pending) doc_render_range_emit(rects, capacity, &count, &pending);
    *total = count;
    return XUI_OK;
}
static uint64_t doc_render_find_hit(xui_document_renderer r, const doc_fragment* fragment)
{
    xui_doc_position_t start; uint64_t low = 0, high = r->find_count; int order;
    if (!r->find_matches || !r->find_count || !fragment->end) return DOC_NONE;
    doc_render_position(r, fragment->node, fragment->start, &start);
    while (low < high) {
        uint64_t mid = low + (high - low) / 2;
        if (doc_render_compare(r, &r->find_matches[mid].tCaret, &start, &order) != XUI_OK) return DOC_NONE;
        if (order <= 0) low = mid + 1; else high = mid;
    }
    return low < r->find_count && doc_render_selected(r, fragment, &r->find_matches[low]) ? low : DOC_NONE;
}
static int doc_render_cell_selected(const xui_document_renderer r, const doc_box* box)
{
    return box->kind == XUI_DOC_CELL && box->table == r->table_selection.iTableId &&
        box->row >= r->table_selection.iRow && box->column >= r->table_selection.iColumn &&
        box->row - r->table_selection.iRow < r->table_selection.iRows &&
        box->column - r->table_selection.iColumn < r->table_selection.iColumns;
}
static int doc_render_quote_rails(xui_document_renderer r, doc_render_block* block,
    xui_draw_context draw, double ox, double top, xui_rect_t clip)
{
    uint64_t id = block->node;
    double indent = block->indent, step = r->desc.fIndent * r->desc.fZoom;
    if (!r->desc.iQuoteBorderColor || !r->proxy->drawRectFill) return XUI_OK;
    while (id && id != DOC_ROOT) {
        doc_node* node = doc_index_get(r->snapshot->state->index, id);
        if (!node) break;
        if (node->kind == XUI_DOC_QUOTE) {
            xui_rect_t rect = doc_render_rect(ox + fmax(0, indent - step) + 3,
                top, 2, block->height);
            if (doc_render_visible(rect, clip)) {
                int result = r->proxy->drawRectFill(r->proxy, draw, rect,
                    r->desc.iQuoteBorderColor);
                if (result != XUI_OK) return result;
            }
        }
        if (node->kind == XUI_DOC_QUOTE || node->kind == XUI_DOC_LIST_ITEM)
            indent = fmax(0, indent - step);
        id = node->parent;
    }
    return XUI_OK;
}
static uint32_t doc_render_foreground(const xui_document_renderer r,
    const xui_doc_attributes_t* attrs)
{
    if (doc_text_color_set(attrs)) return attrs->iTextColor;
    if (attrs->iFlags & XUI_DOC_TEXT_COLOR_CURRENT) return r->desc.iTextColor;
    return attrs->iMarks & XUI_DOC_LINK ? r->desc.iLinkColor : r->desc.iTextColor;
}
static int doc_render_decoration_line(xui_document_renderer r,
    xui_draw_context draw, const xui_font_metrics_t* metrics,
    double left, double right, double baseline, double height,
    uint32_t color, int strike)
{
    double thickness = strike ? metrics->fStrikeThickness : metrics->fUnderlineThickness;
    double offset = strike ? metrics->fStrikePosition : metrics->fUnderlinePosition;
    if (!r->proxy->drawLine || right <= left) return XUI_OK;
    if (!isfinite(thickness) || thickness <= 0 || !isfinite(offset)) {
        thickness = 1;
        offset = strike ? -height * .28 : fmax(1, height * .08);
    }
    return r->proxy->drawLine(r->proxy, draw, (float)left, (float)(baseline + offset),
        (float)right, (float)(baseline + offset), (float)thickness, color);
}
static int doc_render_run_decorations(xui_document_renderer r,
    const doc_render_run* run, xui_draw_context draw, xui_rect_t rect,
    uint32_t color)
{
    xui_font_metrics_t metrics = {0}; int result = XUI_OK;
    uint32_t marks = run->attrs.iMarks;
    double baseline = rect.fY + (run->shape.fAscent > 0 ? run->shape.fAscent : rect.fH * .8);
    if (!(marks & (XUI_DOC_UNDERLINE | XUI_DOC_LINK | XUI_DOC_STRIKE)) ||
        !r->proxy->drawLine) return XUI_OK;
    if (r->proxy->fontGetMetrics &&
        r->proxy->fontGetMetrics(r->proxy, run->font, &metrics) != XUI_OK)
        memset(&metrics, 0, sizeof(metrics));
    if (marks & (XUI_DOC_UNDERLINE | XUI_DOC_LINK))
        result = doc_render_decoration_line(r, draw, &metrics, rect.fX,
            rect.fX + rect.fW, baseline, rect.fH, color, 0);
    if (result == XUI_OK && (marks & XUI_DOC_STRIKE))
        result = doc_render_decoration_line(r, draw, &metrics, rect.fX,
            rect.fX + rect.fW, baseline, rect.fH, color, 1);
    return result;
}
typedef struct doc_decoration_segment {
    double left, right;
    uint32_t color;
    int active;
} doc_decoration_segment;
static int doc_render_decoration_extend(xui_document_renderer r,
    xui_draw_context draw, const xui_font_metrics_t* metrics,
    doc_decoration_segment* segment, double left, double right,
    double baseline, double height, uint32_t color, int strike)
{
    int result = XUI_OK;
    if (right <= left) return XUI_OK;
    if (segment->active && (segment->color != color || left > segment->right + .001 ||
        right < segment->left - .001)) {
        result = doc_render_decoration_line(r, draw, metrics, segment->left,
            segment->right, baseline, height, segment->color, strike);
        segment->active = 0;
    }
    if (result != XUI_OK) return result;
    if (!segment->active) *segment = (doc_decoration_segment){left, right, color, 1};
    else {
        segment->left = fmin(segment->left, left);
        segment->right = fmax(segment->right, right);
    }
    return XUI_OK;
}
static size_t doc_render_paint_run_end(const doc_render_block* block,
    const doc_render_paint_group* group, size_t first)
{
    size_t end = block->runs[doc_fragment_paint_run(&block->fragments[first])].end_fragment;
    return end < group->end_fragment ? end : group->end_fragment;
}
static int doc_render_group_decorations(xui_document_renderer r,
    const doc_render_block* block, const doc_render_paint_group* group,
    xui_draw_context draw, double ox, double top)
{
    xui_font_metrics_t metrics = {0};
    doc_decoration_segment segments[2] = {{0}};
    const doc_fragment* first;
    double baseline;
    size_t i = group->first_fragment; int result = XUI_OK, type;
    if (!group->decorations || !r->proxy->drawLine) return XUI_OK;
    first = &block->fragments[group->origin_fragment];
    r->stats.iDrawFragmentsExamined++;
    baseline = floor(top + first->y + .5) + group->ascent;
    if (r->proxy->fontGetMetrics &&
        r->proxy->fontGetMetrics(r->proxy, group->font, &metrics) != XUI_OK)
        memset(&metrics, 0, sizeof(metrics));
    while (i < group->end_fragment) {
        size_t end = doc_render_paint_run_end(block, group, i), head, tail;
        const doc_fragment *a = &block->fragments[i], *z = &block->fragments[end - 1];
        const doc_render_run* run = &block->runs[doc_fragment_paint_run(a)];
        uint32_t marks = run->attrs.iMarks, color = doc_render_foreground(r, &run->attrs);
        xui_doc_rect_t left, right;
        r->stats.iDrawFragmentsExamined += end - i > 1 ? 2 : 1;
        if ((marks & (XUI_DOC_UNDERLINE | XUI_DOC_LINK | XUI_DOC_STRIKE)) &&
            (a->kind == XUI_DOC_SOFT_BREAK || z->end > a->start)) {
            doc_render_grapheme(block, i, &head, &tail);
            r->stats.iDrawFragmentsExamined += doc_render_grapheme_visits(block, head, tail);
            left = doc_render_grapheme_rect(block, head, tail, top);
            doc_render_grapheme(block, end - 1, &head, &tail);
            r->stats.iDrawFragmentsExamined += doc_render_grapheme_visits(block, head, tail);
            right = doc_render_grapheme_rect(block, head, tail, top);
            for (type = 0; type < 2; type++) {
                if (!(marks & (type ? XUI_DOC_STRIKE : XUI_DOC_UNDERLINE | XUI_DOC_LINK))) continue;
                result = doc_render_decoration_extend(r, draw, &metrics, &segments[type],
                    floor(ox + fmin(left.x, right.x) + .5),
                    floor(ox + fmax(left.x + left.width, right.x + right.width) + .5),
                    baseline, group->line_height, color, type);
                if (result != XUI_OK) return result;
            }
        }
        i = end;
    }
    for (type = 0; type < 2; type++) if (segments[type].active) {
        result = doc_render_decoration_line(r, draw, &metrics,
            segments[type].left, segments[type].right, baseline,
            group->line_height, segments[type].color, type);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
/* A drawText-only proxy can keep the complete shaping context. Uniform color
 * needs one call; different colors use disjoint visual grapheme intervals.
 * The first visible source fragment owns a grapheme spanning styled nodes.
 * This spatial fallback cannot reproduce per-glyph ink coloring at an overlap;
 * a native drawTextSpans backend remains the exact glyph-color path. */
static int doc_render_group_color_unit(const doc_render_block* block,
    const doc_render_paint_group* group, size_t at, size_t* end)
{
    size_t first;
    uint64_t bytes = 0;
    doc_render_grapheme(block, at, &first, end);
    if (first < group->first_fragment || *end > group->end_fragment) return -1;
    if (group->display_ends) {
        size_t begin = at - group->first_fragment;
        bytes = group->display_ends[*end - group->first_fragment - 1] -
            (begin ? group->display_ends[begin - 1] : 0);
    } else {
        size_t i;
        for (i = at; i < *end; i++) bytes +=
            block->fragments[i].kind == XUI_DOC_SOFT_BREAK ? 1 :
            doc_fragment_paint_bytes(&block->fragments[i]);
    }
    return bytes != 0;
}
static int doc_render_group_text(xui_document_renderer r,
    const doc_render_block* block,const doc_render_paint_group* group,xui_draw_context draw,
    xui_rect_t rect,uint32_t color,uint32_t flags,float offset,
    xui_text_paint_span_t* spans,int count,int use_spans)
{
    xui_text_item_t item;doc_hyphen_lease lease;
    int result;
    if((group->shared_seed || group->variant_span) && !group->range_input){
        const doc_render_paint_seed* seed;
        int i;
        if(!r->proxy->drawTextShapeRangeSpans || group->shared_seed>block->paint_seed_count ||
            (group->variant_span && (!group->variant || group->variant_span>group->variant->span_count)))
            return XUI_ERROR_INVALID_STATE;
        seed=group->variant_span?&group->variant->spans[group->variant_span-1]:&block->paint_seeds[group->shared_seed-1];
        for(i=0;i<count;i++){
            spans[i].iStart+=(int)group->shape_start;
            spans[i].iEnd+=(int)group->shape_start;
        }
        return r->proxy->drawTextShapeRangeSpans(r->proxy,draw,doc_render_seed_shape(block,seed),
            (int)group->shape_start,(int)group->shape_end,rect,color,flags,offset,spans,count);
    }
    if(group->shape.pPaint && r->proxy->drawTextShapeRangeSpans){
        result=r->proxy->drawTextShapeRangeSpans(r->proxy,draw,&group->shape,0,(int)group->bytes,
            rect,color,flags,offset,spans,count);
        if(result!=XUI_ERROR_UNSUPPORTED)return result;
    }
    if(group->range_input && group->shared_seed)result=doc_render_seed_range_item(block,group,
        block->runs[doc_fragment_paint_run(&block->fragments[group->origin_fragment])].attrs.sLanguage,&item,&lease);
    else result=doc_render_group_item(group,
        block->runs[doc_fragment_paint_run(&block->fragments[group->origin_fragment])].attrs.sLanguage,&item,&lease);
    if(result!=XUI_OK)return result;
    if(group->shape.pPaint && !r->proxy->drawTextShapeRangeSpans && (group->input_caps & XUI_PROXY_CAP_TEXT_RANGE)){
        item.iFlags|=XUI_TEXT_SHAPE_RANGE;item.iRangeStart=0;item.iRangeEnd=(int)group->bytes;item.pShape=&group->shape;
    }
    item.fDrawOffsetX=offset;
    if(use_spans && !r->proxy->drawTextSpans && (group->input_caps & XUI_PROXY_CAP_TEXT_RANGE)){
        if(!(item.iFlags & XUI_TEXT_SHAPE_RANGE)){
            item.iFlags|=XUI_TEXT_SHAPE_RANGE;item.iRangeStart=0;item.iRangeEnd=item.iTextSize;
            item.pShape=group->shape.pPaint?&group->shape:NULL;
        }
        item.pPaintSpans=spans;item.iPaintSpanCount=count;
        result=r->proxy->drawText(r->proxy,draw,&item,rect,color,flags);
    }
    else if(use_spans && !r->proxy->drawTextSpans)result=XUI_ERROR_UNSUPPORTED;
    else if(use_spans)result=r->proxy->drawTextSpans(r->proxy,draw,&item,rect,color,flags,spans,count);
    else result=r->proxy->drawText(r->proxy,draw,&item,rect,color,flags);
    doc_context_hyphen_release(&lease);return result;
}
static int doc_render_group_clipped_text(xui_document_renderer r,
    const doc_render_block* block, const doc_render_paint_group* group, xui_draw_context draw,
    xui_rect_t rect, xui_rect_t outer, double left, double right, uint32_t color, float offset)
{
    xui_rect_t colored = outer;
    int result, restored;
    double a = fmax(outer.fX, floor(left + .5));
    double z = fmin(outer.fX + outer.fW, floor(right + .5));
    if (z <= a || outer.fH <= 0) return XUI_OK;
    colored.fX = (float)a; colored.fW = (float)(z - a);
    result = r->proxy->drawClipSet(r->proxy, draw, colored);
    if (result == XUI_OK) result = doc_render_group_text(r,block,group,draw,rect,color,
        (r->proxy->drawClipSet ? 0 : XUI_TEXT_CLIP) | ((group->bidi_level & 1) ? XUI_TEXT_RTL : 0),offset,NULL,0,0);
    restored = r->proxy->drawClipSet(r->proxy, draw, outer);
    return result == XUI_OK ? restored : result;
}
static int doc_render_group_plain_text(xui_document_renderer r,
    const doc_render_block* block, const doc_render_paint_group* group,
    xui_draw_context draw, double ox, xui_rect_t rect, xui_rect_t clip)
{
    float offset=(float)(ox+doc_render_group_x(block,group)-rect.fX);
    size_t i, end; uint32_t color = r->desc.iTextColor;
    int found = 0, uniform = 1, result;
    double left = clip.fX;
    for (i = group->first_fragment; i < group->end_fragment; i = end) {
        int visible = doc_render_group_color_unit(block, group, i, &end);
        const doc_render_run* run;
        uint32_t next;
        r->stats.iDrawFragmentsExamined += end - i;
        if (visible < 0) return XUI_ERROR_INVALID_STATE;
        if (!visible) continue;
        run = &block->runs[doc_fragment_paint_run(&block->fragments[i])];
        next = doc_render_foreground(r, &run->attrs);
        if (!found) { color = next; found = 1; }
        else if (color != next) { uniform = 0; break; }
    }
    if (uniform) {
        return doc_render_group_text(r,block,group,draw,rect,color,
            (r->proxy->drawClipSet?0:XUI_TEXT_CLIP)|((group->bidi_level&1)?XUI_TEXT_RTL:0),offset,NULL,0,0);
    }
    if (!r->proxy->drawClipSet || !r->proxy->drawClipGet || !r->proxy->drawClipClear)
        return XUI_ERROR_UNSUPPORTED;
    found = 0;
    if (group->bidi_level & 1) {
        /* Iterate whole graphemes in physical order, retaining the complete
         * logical string for every clipped draw call. */
        i = group->end_fragment;
        while (i > group->first_fragment) {
            size_t head, tail;
            const doc_render_run* run; uint32_t next; int visible;
            doc_render_grapheme(block, i - 1, &head, &tail);
            visible = doc_render_group_color_unit(block, group, head, &tail);
            if (visible < 0) return XUI_ERROR_INVALID_STATE;
            i = head;
            r->stats.iDrawFragmentsExamined += tail - head;
            if (!visible) continue;
            run = &block->runs[doc_fragment_paint_run(&block->fragments[head])];
            next = doc_render_foreground(r, &run->attrs);
            if (!found) { color = next; found = 1; }
            else if (color != next) {
                double boundary = ox + block->fragments[head].x;
                result = doc_render_group_clipped_text(r, block, group, draw, rect, clip, left, boundary, color, offset);
                if (result != XUI_OK) return result;
                left = boundary; color = next;
            }
        }
        return doc_render_group_clipped_text(r, block, group, draw, rect, clip,
            left, clip.fX + clip.fW, color, offset);
    }
    for (i = group->first_fragment; i < group->end_fragment; i = end) {
        int visible = doc_render_group_color_unit(block, group, i, &end);
        const doc_render_run* run; uint32_t next;
        r->stats.iDrawFragmentsExamined += end - i;
        if (visible < 0) return XUI_ERROR_INVALID_STATE;
        if (!visible) continue;
        run = &block->runs[doc_fragment_paint_run(&block->fragments[i])];
        next = doc_render_foreground(r, &run->attrs);
        if (!found) { color = next; found = 1; }
        else if (color != next) {
            double boundary = ox + block->fragments[i].x;
            result = doc_render_group_clipped_text(r, block, group, draw, rect,
                clip, left, boundary, color, offset);
            if (result != XUI_OK) return result;
            left = boundary; color = next;
        }
    }
    return doc_render_group_clipped_text(r, block, group, draw, rect, clip,
        left, clip.fX + clip.fW, color, offset);
}
static int doc_render_draw_paint_group(xui_document_renderer r,
    const doc_render_block* block, const doc_render_paint_group* group,
    xui_draw_context draw, double ox, double top, xui_rect_t clip)
{
    xui_text_paint_span_t small[16], *spans = small;
    const doc_fragment* first = &block->fragments[group->first_fragment];
    const doc_fragment* origin = &block->fragments[group->origin_fragment];
    xui_rect_t rect = doc_render_rect(ox + doc_render_group_x(block, group), top + origin->y,
        group->paint_width, group->line_height);
    size_t i, at = 0, count = 0;
    int result;
    if (!group->bytes) return XUI_OK;
    if (!doc_render_visible(rect, clip)) return XUI_OK;
    r->stats.iDrawFragmentsExamined += 3;
    /* Retained colour spans are independent of the string span callback.
     * Prefer them over spatial clipping, which can recolour antialiased ink
     * extending across a neighbouring space/style boundary. */
    if (!r->proxy->drawTextSpans && !(r->proxy->drawTextShapeRangeSpans &&
        (group->shared_seed || group->variant_span || group->shape.pPaint)) &&
        !(group->input_caps & XUI_PROXY_CAP_TEXT_RANGE)) {
        result = doc_render_group_plain_text(r, block, group, draw, ox, rect, clip);
        if (result == XUI_OK) result = doc_render_group_decorations(r, block, group, draw, ox, top);
        return result;
    }
    count = (size_t)(doc_fragment_paint_run(&block->fragments[group->end_fragment - 1]) -
        doc_fragment_paint_run(first)) + 1;
    if (count > sizeof(small) / sizeof(small[0])) {
        spans = malloc(count * sizeof(*spans));
        if (!spans) return XUI_ERROR_OUT_OF_MEMORY;
    }
    count = 0;
    for (i = group->first_fragment; i < group->end_fragment; ) {
        size_t end = doc_render_paint_run_end(block, group, i);
        const doc_fragment *fragment = &block->fragments[i], *last = &block->fragments[end - 1];
        const doc_render_run* run = &block->runs[doc_fragment_paint_run(fragment)];
        size_t length = fragment->kind == XUI_DOC_SOFT_BREAK ? 1 : (size_t)(last->end - fragment->start);
        if (group->display_ends) length = group->display_ends[end - group->first_fragment - 1] - (uint32_t)at;
        r->stats.iDrawFragmentsExamined += end - i > 1 ? 2 : 1;
        if (length) spans[count++] = (xui_text_paint_span_t){sizeof(*spans),
            (int)at, (int)(at + length), doc_render_foreground(r, &run->attrs)};
        at += length; i = end;
    }
    result=doc_render_group_text(r,block,group,draw,rect,r->desc.iTextColor,
        (r->proxy->drawClipSet ? 0 : XUI_TEXT_CLIP) | ((group->bidi_level & 1) ? XUI_TEXT_RTL : 0),
        (float)(ox+doc_render_group_x(block,group)-rect.fX),spans,(int)count,1);
    if (spans != small) free(spans);
    if(result==XUI_ERROR_UNSUPPORTED && !r->proxy->drawTextSpans)
        result=doc_render_group_plain_text(r,block,group,draw,ox,rect,clip);
    if (result == XUI_OK) result = doc_render_group_decorations(r, block, group, draw, ox, top);
    return result;
}
XUI_API int xuiDocumentRendererDraw(xui_document_renderer r, xui_draw_context draw, double ox, double oy, xui_rect_t clip,
    const xui_doc_range_t* selection, uint32_t selection_color)
{
    size_t index, i; xui_rect_t saved = {0}; int had_clip = 0, result = XUI_OK;
    if (!r || !r->snapshot || !draw || !isfinite(ox) || !isfinite(oy)) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_render_refresh_resources(r); if (result != XUI_OK) return result;
    if (r->proxy->drawClipGet) {
        result = r->proxy->drawClipGet(r->proxy, draw, &saved, &had_clip);
        if (result != XUI_OK) return result;
    }
    if (had_clip) {
        float right = fminf(saved.fX + saved.fW, clip.fX + clip.fW), bottom = fminf(saved.fY + saved.fH, clip.fY + clip.fH);
        clip.fX = fmaxf(saved.fX, clip.fX); clip.fY = fmaxf(saved.fY, clip.fY); clip.fW = fmaxf(0, right - clip.fX); clip.fH = fmaxf(0, bottom - clip.fY);
    }
    result = doc_render_resolve_partial_before_y(r, fmax(0, clip.fY - oy));
    if (result != XUI_OK) return result;
    if (r->proxy->drawClipSet) {
        result = r->proxy->drawClipSet(r->proxy, draw, clip);
        if (result != XUI_OK) goto draw_done;
    }
    index = doc_render_block_at(r, fmax(0, clip.fY - oy));
    for (; index < r->count; index++) {
        doc_render_block* b; size_t first_fragment, end_fragment;
        double top = oy + doc_render_height_before(r, index);
        if (top > clip.fY + clip.fH) break;
        b = doc_render_block_at_index(r, index);
        result = doc_render_materialize_visible(r, index, b,
            clip.fY + clip.fH - top + 1);
        if (result != XUI_OK) break;
        b = doc_render_block_at_index(r, index);
        result = doc_render_cover_line(r, index, b,
            fmax(0, clip.fX + clip.fW - ox), 0);
        if (result != XUI_OK) break;
        doc_render_visible_fragments(b, clip.fY - top - 1,
            clip.fY + clip.fH - top + 1, &first_fragment, &end_fragment);
        doc_render_line_fragments_at_x(b, clip.fX - ox - 1,
            clip.fX + clip.fW - ox + 1, &first_fragment, &end_fragment);
        for (i = b->box_count; i > 0; i--) {
            doc_box* box = &b->boxes[i - 1]; xui_rect_t rect = doc_render_rect(ox + box->x, top + box->y, box->width, box->height);
            uint32_t color = box->kind == XUI_DOC_RULE ? r->desc.iRuleColor :
                box->kind == XUI_DOC_QUOTE ? r->desc.iQuoteBorderColor :
                box->kind == XUI_DOC_CODE_BLOCK ? r->desc.iCodeBackground : box->color;
            if (box->kind == XUI_DOC_PARAGRAPH || box->kind == XUI_DOC_HEADING ||
                box->kind == XUI_DOC_CELL) {
                doc_node* node = doc_index_get(r->snapshot->state->index, box->node);
                if (node && (node->attrs->iFlags & XUI_DOC_BACKGROUND_COLOR_CURRENT)) {
                    xui_doc_attributes_t attrs = doc_effective_text_attrs(r->snapshot->state, node);
                    color = doc_render_foreground(r, &attrs);
                } else if (node && !color && !doc_background_color_set(node->attrs)) {
                    if (box->kind == XUI_DOC_CELL)
                        color = node->attrs->iFlags & XUI_DOC_HEADER ?
                            r->desc.iTableHeaderColor : r->desc.iTableCellColor;
                    else color = r->desc.iParagraphBackgroundColor;
                }
            }
            if (!doc_render_visible(rect, clip)) continue;
            if (color && r->proxy->drawRectFill) r->proxy->drawRectFill(r->proxy, draw, rect, color);
            if (box->kind == XUI_DOC_CELL && r->proxy->drawRectStroke &&
                !(r->table_selection.iTableId && doc_render_cell_selected(r, box)))
                r->proxy->drawRectStroke(r->proxy, draw, rect, 1, r->desc.iTableBorderColor);
        }
        result = doc_render_quote_rails(r, b, draw, ox, top, clip);
        if (result != XUI_OK) break;
        if (b->admonition_title && r->proxy->drawText) {
            doc_node* quote = doc_index_get(r->snapshot->state->index, b->node);
            xui_rect_t rect = doc_render_rect(ox + b->indent, top,
                fmax(1, r->width - b->indent), b->height);
            if (quote && quote->info && doc_render_visible(rect, clip)) {
                result = r->proxy->drawText(r->proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=b->admonition_font, .sText=doc_string(quote->info), .iTextSize=-1, .sLanguage=doc_effective_text_attrs(r->snapshot->state,quote).sLanguage, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_CLIP | XUI_TEXT_ALIGN_MIDDLE) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, rect, r->desc.iQuoteBorderColor ? r->desc.iQuoteBorderColor : r->desc.iTextColor, XUI_TEXT_CLIP | XUI_TEXT_ALIGN_MIDDLE);
                if (result != XUI_OK) break;
            }
        }
        if (r->table_selection.iTableId && r->proxy->drawRectFill) {
            for (i = b->box_count; i > 0; i--) {
                doc_box* box = &b->boxes[i - 1]; xui_rect_t rect;
                if (!doc_render_cell_selected(r, box)) continue;
                rect = doc_render_rect(ox + box->x, top + box->y, box->width, box->height);
                if (!doc_render_visible(rect, clip)) continue;
                r->proxy->drawRectFill(r->proxy, draw, rect, r->table_selection_color);
                if (r->proxy->drawRectStroke)
                    r->proxy->drawRectStroke(r->proxy, draw, rect, 1, r->desc.iTableBorderColor);
            }
        }
        if (b->marker_text[0] && r->proxy->drawText) r->proxy->drawText(r->proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=r->desc.tFonts.normal, .sText=b->marker_text, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, doc_render_rect(ox + fmax(0, b->indent - r->desc.fIndent * r->desc.fZoom), top, r->desc.fIndent * r->desc.fZoom, 24 * r->desc.fZoom), r->desc.iTextColor, XUI_TEXT_CLIP);
        for (i = first_fragment; i < end_fragment; ) {
            size_t first, end, j;
            xui_doc_rect_t bounds;
            xui_rect_t rect;
            uint32_t background = 0;
            int found = 0, active = 0;
            doc_render_grapheme(b, i, &first, &end);
            r->stats.iDrawFragmentsExamined += doc_render_grapheme_visits(b, first, end);
            bounds = doc_render_grapheme_rect(b, first, end, top);
            rect = doc_render_rect(ox + bounds.x, bounds.y, bounds.width, bounds.height);
            i = end;
            if (!doc_render_visible(rect, clip)) continue;
            for (j = first; j < end; j++) {
                const doc_fragment* f = &b->fragments[j];
                uint64_t hit;
                r->stats.iDrawFragmentsExamined++;
                if (f->run != UINT32_MAX || f->kind == XUI_DOC_SOFT_BREAK || f->kind == XUI_DOC_HARD_BREAK) {
                    const xui_doc_attributes_t* attrs = &b->runs[f->run != UINT32_MAX ? f->run : f->break_style_run].attrs;
                    uint32_t color = attrs->iFlags & XUI_DOC_BACKGROUND_COLOR_CURRENT ?
                        doc_render_foreground(r, attrs) :
                        doc_background_color_set(attrs) ? attrs->iBackgroundColor :
                        attrs->iMarks & XUI_DOC_HIGHLIGHT ? r->desc.iHighlightColor : 0;
                    if (color) background = color;
                }
                hit = doc_render_find_hit(r, f);
                if (hit != DOC_NONE) {
                    found = 1;
                    if (hit == r->find_active) active = 1;
                }
            }
            if (background && r->proxy->drawRectFill) r->proxy->drawRectFill(r->proxy, draw, rect, background);
            if (found && r->proxy->drawRectFill)
                r->proxy->drawRectFill(r->proxy, draw, rect, r->find_result_color);
            if (doc_render_grapheme_selected(r, b, first, end, selection) && r->proxy->drawRectFill)
                r->proxy->drawRectFill(r->proxy, draw, rect, selection_color);
            if (active && r->proxy->drawRectFill)
                r->proxy->drawRectFill(r->proxy, draw, rect, r->find_active_color);
        }
        {
        size_t group_at = 0;
        for (i = first_fragment; i < end_fragment; ) {
            doc_fragment* f = &b->fragments[i]; size_t end = i + 1;
            xui_rect_t rect = doc_render_rect(ox + f->x, top + f->y, f->width, f->height);
            r->stats.iDrawFragmentsExamined++;
            while (group_at < b->paint_group_count &&
                b->paint_groups[group_at].end_fragment <= i) group_at++;
            if (group_at < b->paint_group_count &&
                b->paint_groups[group_at].first_fragment == i) {
                const doc_render_paint_group* group = &b->paint_groups[group_at++];
                result = doc_render_draw_paint_group(r, b, group, draw,
                    ox, top, clip);
                if (result != XUI_OK) break;
                i = group->end_fragment;
                continue;
            }
            if (f->run != UINT32_MAX && !doc_render_fragment_text(b, f)) {
                i++;
                continue;
            }
            if (f->run == UINT32_MAX) {
                if (f->kind == XUI_DOC_SOFT_BREAK && doc_render_visible(rect, clip)) {
                    const doc_render_run* run = &b->runs[f->break_style_run];
                    result = doc_render_run_decorations(r, run, draw, rect, doc_render_foreground(r, &run->attrs));
                    if (result != XUI_OK) break;
                }
                if (doc_render_visible(rect, clip) && f->kind != XUI_DOC_HARD_BREAK && f->kind != XUI_DOC_SOFT_BREAK && f->kind != XUI_DOC_PARAGRAPH && f->kind != XUI_DOC_HEADING && f->kind != XUI_DOC_CELL && f->kind != XUI_DOC_ROOT && f->kind != XUI_DOC_QUOTE) {
                    int object_result = r->desc.onObjectDraw ? r->desc.onObjectDraw(r->snapshot, f->node, r->proxy, draw, rect, r->desc.pUser) : XUI_ERROR_UNSUPPORTED;
                    if (object_result == XUI_ERROR_UNSUPPORTED && f->kind == XUI_DOC_IMAGE)
                        object_result = doc_render_image_draw(r, doc_index_get(r->snapshot->state->index, f->node), draw, rect);
                    if (object_result == XUI_ERROR_UNSUPPORTED && f->kind == XUI_DOC_IMAGE)
                        object_result = doc_render_image_placeholder(r,
                            doc_index_get(r->snapshot->state->index, f->node), draw, rect);
                    if (object_result == XUI_ERROR_UNSUPPORTED && r->proxy->drawRectStroke) r->proxy->drawRectStroke(r->proxy, draw, rect, 1, r->desc.iBorderColor);
                    else if (object_result != XUI_OK && object_result != XUI_ERROR_UNSUPPORTED) { result = object_result; break; }
                    if (doc_render_selected(r, f, selection)) {
                        if (r->proxy->drawRectFill) r->proxy->drawRectFill(r->proxy, draw, rect, selection_color);
                        if (r->proxy->drawRectStroke) r->proxy->drawRectStroke(r->proxy, draw, rect, 1, selection_color);
                    }
                }
            } else {
                doc_render_run* run = &b->runs[f->run]; uint64_t text_end = f->end; double width = f->width;
                int projected = !!(f->flags & DOC_FORMATTED);
                uint32_t color = doc_render_foreground(r, &run->attrs);
                while (end < end_fragment) {
                    doc_fragment* next = &b->fragments[end];
                    if (next->run != f->run || (!doc_render_fragment_text(b, next) && !(next->flags & DOC_INVISIBLE)) ||
                        next->start != text_end || fabs(next->y - f->y) > .001 ||
                        fabs(next->x - f->x - width) > .001) break;
                    r->stats.iDrawFragmentsExamined++;
                    if (next->flags & DOC_FORMATTED) projected = 1;
                    width += next->width; text_end = next->end; end++;
                }
                rect = doc_render_rect(ox + f->x, top + f->y, width, f->height);
                /* Reuse the full cached run's ink extent without reshaping.
                 * Decorations and selection still use logical advances. */
                xui_rect_t paint_rect = rect;
                if (!projected && f->start == run->base_offset &&
                    text_end - run->base_offset == run->bytes)
                    paint_rect = doc_render_rect(ox + f->x, top + f->y,
                        fmax(width, run->shape.fWidth), f->height);
                if (doc_render_visible(paint_rect, clip) && text_end > f->start) {
                    if(!projected && run->shape.pPaint && r->proxy->drawTextShapeRange){
                        result=r->proxy->drawTextShapeRange(r->proxy,draw,&run->shape,
                            (int)(f->start-run->base_offset),(int)(text_end-run->base_offset),
                            paint_rect,color,XUI_TEXT_CLIP | (!r->proxy->drawLine &&
                            (run->attrs.iMarks & (XUI_DOC_UNDERLINE | XUI_DOC_LINK)) ? XUI_TEXT_UNDERLINE : 0));
                        if(result==XUI_OK){
                            result=doc_render_run_decorations(r,run,draw,rect,color);
                            if(result!=XUI_OK)break;
                            i=end;continue;
                        }
                        if(result!=XUI_ERROR_UNSUPPORTED)break;
                    }
                    char* display = NULL; char* text = run->text + f->start - run->base_offset;
                    char terminator = run->text[text_end - run->base_offset];
                    if (projected) {
                        int bytes;
                        display = malloc((size_t)(text_end - f->start) + 2);
                        if (!display) { result = XUI_ERROR_OUT_OF_MEMORY; break; }
                        bytes = xuiInternalTextCopyDisplay(text, (int)(text_end - f->start), display);
                        if (b->fragments[end - 1].flags & DOC_HYPHEN_USED) display[bytes++] = '-';
                        display[bytes] = 0; text = display;
                    } else run->text[text_end - run->base_offset] = 0;
                    if (r->proxy->drawText) result = r->proxy->drawText(r->proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=run->font, .sText=text, .iTextSize=-1, .sLanguage=run->attrs.sLanguage, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_CLIP | (!r->proxy->drawLine &&
                            (run->attrs.iMarks & (XUI_DOC_UNDERLINE | XUI_DOC_LINK)) ? XUI_TEXT_UNDERLINE : 0)) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, paint_rect, color, XUI_TEXT_CLIP | (!r->proxy->drawLine &&
                            (run->attrs.iMarks & (XUI_DOC_UNDERLINE | XUI_DOC_LINK)) ? XUI_TEXT_UNDERLINE : 0));
                    run->text[text_end - run->base_offset] = terminator;
                    free(display);
                    if (result == XUI_OK) result = doc_render_run_decorations(r, run, draw, rect, color);
                    if (result != XUI_OK) break;
                }
            }
            i = end;
        }
        }
        if (result != XUI_OK) break;
    }
draw_done:
    {
        int restored = XUI_OK;
        if (had_clip && r->proxy->drawClipSet) restored = r->proxy->drawClipSet(r->proxy, draw, saved);
        else if (r->proxy->drawClipClear) restored = r->proxy->drawClipClear(r->proxy, draw);
        if (result == XUI_OK) result = restored;
    }
    return result;
}
XUI_API int xuiDocumentRendererGetStats(xui_document_renderer r, xui_doc_renderer_stats_t* out)
{
    size_t i;
    if (!r || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    *out = r->stats; out->iBlocks = r->count; out->iSourceBlocks = 0; out->bLiveSourceFallback = r->live_fallback;
    out->iLayoutCacheBytes = r->cache_bytes;
    out->iLayoutCacheBudgetBytes = r->desc.iLayoutCacheBudgetBytes;
    if (r->mode == XUI_DOC_SOURCE_TEXT) { out->iSourceBlocks = r->count; return XUI_OK; }
    for (i = 0; i < r->count; i++) out->iSourceBlocks += !!r->blocks[i].source_line;
    return XUI_OK;
}

#endif
