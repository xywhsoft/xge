#include "xui_document_layout_internal.h"
#include <math.h>
#include <float.h>
#include <stdio.h>

static void doc_render_clear(xui_document_renderer r)
{
    size_t i;
    for (i = 0; i < r->count; i++) doc_render_block_free(&r->blocks[i]);
    free(r->blocks); free(r->heights); r->blocks = NULL; r->heights = NULL; r->count = r->capacity = 0;
    xrtMapClear(&r->block_index);
}
static int doc_render_collect(xui_document_renderer r, uint64_t id, double indent)
{
    doc_node* n = doc_index_get(r->snapshot->state->index, id); uint64_t i;
    size_t first = r->count; int result;
    if (!n) return XUI_ERROR_NOT_FOUND;
    if (n->kind == XUI_DOC_ROOT || n->kind == XUI_DOC_QUOTE || n->kind == XUI_DOC_LIST || n->kind == XUI_DOC_LIST_ITEM || n->kind == XUI_DOC_FOOTNOTE) {
        if (n->kind == XUI_DOC_QUOTE || n->kind == XUI_DOC_LIST_ITEM) indent += r->desc.fIndent * r->desc.fZoom;
        for (i = 0; i < doc_seq_size(n->children); i++) {
            result = doc_render_collect(r, doc_seq_get_id(n->children, i), indent);
            if (result != XUI_OK) return result;
        }
        if (n->kind == XUI_DOC_LIST_ITEM && first < r->count) {
            doc_render_block* b = &r->blocks[first]; doc_node* parent = doc_index_get(r->snapshot->state->index, n->parent);
            b->marker = id;
            if (n->attrs.iFlags & XUI_DOC_TASK) strcpy(b->marker_text, n->attrs.iFlags & XUI_DOC_CHECKED ? "[x]" : "[ ]");
            else if (parent->attrs.iFlags & XUI_DOC_ORDERED) snprintf(b->marker_text, sizeof(b->marker_text), "%llu.",
                (unsigned long long)(parent->attrs.iListStart + doc_child_index(parent, id)));
            else strcpy(b->marker_text, "\xe2\x80\xa2");
        }
        return XUI_OK;
    }
    result = doc_render_reserve((void**)&r->blocks, &r->capacity, r->count + 1, sizeof(*r->blocks));
    if (result != XUI_OK) return result;
    memset(&r->blocks[r->count], 0, sizeof(*r->blocks));
    r->blocks[r->count].node = id; r->blocks[r->count].indent = indent;
    r->blocks[r->count].height = (24 + r->desc.fParagraphGap) * r->desc.fZoom;
    if (!xrtMapSet(&r->block_index, xuiXrtBytes(&id, sizeof(id)), &r->count)) return XUI_ERROR_OUT_OF_MEMORY;
    r->count++; return XUI_OK;
}
static int doc_render_source_line(xui_document_renderer r, uint64_t start, uint64_t end)
{
    doc_render_block* b; int result;
    if (r->count >= 1000000) return XUI_DOC_ERROR_LIMIT;
    result = doc_render_reserve((void**)&r->blocks, &r->capacity, r->count + 1, sizeof(*r->blocks));
    if (result != XUI_OK) return result;
    b = &r->blocks[r->count++]; memset(b, 0, sizeof(*b)); b->node = DOC_ROOT;
    b->source_line = 1;
    b->source_start = start; b->source_end = end; b->height = 20 * r->desc.fZoom;
    return XUI_OK;
}
static int doc_render_collect_source_range(xui_document_renderer r, uint64_t first, uint64_t size)
{
    char buffer[4096]; uint64_t offset = first, line = first; int result;
    char previous = 0;
    while (offset < size) {
        uint64_t i, bytes = size - offset < sizeof(buffer) ? size - offset : sizeof(buffer);
        doc_seq_read(r->snapshot->state->source, offset, buffer, bytes);
        for (i = 0; i < bytes; i++) {
            if (buffer[i] == '\r' || buffer[i] == '\n') {
                if (buffer[i] != '\n' || previous != '\r') {
                    result = doc_render_source_line(r, line, offset + i);
                    if (result != XUI_OK) return result;
                }
                line = offset + i + 1;
            }
            previous = buffer[i];
        }
        offset += bytes;
    }
    /* Include a terminal empty line only at EOF. At an interior partition it
     * would duplicate the first line of the following rendered container. */
    return line < size || first == size || size == doc_seq_size(r->snapshot->state->source) ? doc_render_source_line(r, line, size) : XUI_OK;
}
static uint64_t doc_render_line_start(const char* source, uint64_t at)
{
    while (at && source[at - 1] != '\n' && source[at - 1] != '\r') at--;
    return at;
}
static uint64_t doc_render_previous_line(const char* source, uint64_t at)
{
    if (at && source[at - 1] == '\n') at--;
    if (at && source[at - 1] == '\r') at--;
    return doc_render_line_start(source, at);
}
static int doc_render_fence(const char* source, uint64_t start, uint64_t end)
{
    uint64_t at = start, count = 0; char marker;
    while (at < end && at - start < 3 && source[at] == ' ') at++;
    if (at == end || (source[at] != '`' && source[at] != '~')) return 0;
    marker = source[at]; while (at < end && source[at++] == marker) count++;
    return count >= 3;
}
/* Source ranges in the current parser describe semantic payloads rather than
 * a complete CST. Partition only when root ranges give unambiguous, ordered
 * boundaries. Keep all trivia in those partitions, including reference
 * definitions. Unknown/overlapping boundaries use one editable source block;
 * this is a view fallback and never serializes or changes the document. */
static int doc_render_collect_live(xui_document_renderer r)
{
    doc_state* s = r->snapshot->state; doc_node* root = doc_index_get(s->index, DOC_ROOT);
    uint64_t count = doc_seq_size(root->children), size = doc_seq_size(s->source), i, active = 0, previous_end = 0;
    uint64_t* starts = NULL; char* source = NULL; int result = XUI_OK, fallback = !count;
    if (count > SIZE_MAX / sizeof(*starts) - 1 || size >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    starts = calloc((size_t)count + 1, sizeof(*starts)); source = malloc((size_t)size + 1);
    if (!starts || !source) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    doc_seq_read(s->source, 0, source, size); source[size] = 0;
    for (i = 0; i < count; i++) {
        doc_node* n = doc_index_get(s->index, doc_seq_get_id(root->children, i)); uint64_t start;
        if (n->source_start == DOC_NONE || n->source_end == DOC_NONE || n->source_start > n->source_end || n->source_end > size) { fallback = 1; break; }
        start = doc_render_line_start(source, n->source_start);
        if (n->kind == XUI_DOC_CODE_BLOCK || n->kind == XUI_DOC_DIAGRAM) {
            uint64_t previous = doc_render_previous_line(source, start);
            if (previous < start && doc_render_fence(source, previous, start)) start = previous;
        }
        if (n->kind == XUI_DOC_FRONT_MATTER) start = 0;
        if (i && (start <= starts[i - 1] || start < previous_end)) { fallback = 1; break; }
        starts[i] = i ? start : 0; previous_end = n->source_end;
    }
    r->active_source = r->active_source < size ? r->active_source : size;
    r->live_fallback = fallback;
    if (fallback) {
        r->live_start = 0; r->live_end = size;
        result = doc_render_collect_source_range(r, 0, size); goto done;
    }
    starts[count] = size;
    for (i = 1; i < count; i++) if (starts[i] <= r->active_source) active = i;
    r->live_start = starts[active]; r->live_end = starts[active + 1];
    for (i = 0; i < count && result == XUI_OK; i++) {
        if (i == active) result = doc_render_collect_source_range(r, starts[i], starts[i + 1]);
        else result = doc_render_collect(r, doc_seq_get_id(root->children, i), 0);
    }
done:
    free(starts); free(source); return result;
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
    for (; index; index -= index & (~index + 1)) height += r->heights[index];
    return height;
}
void doc_render_set_height(xui_document_renderer r, size_t index, double delta)
{
    size_t i;
    for (i = index + 1; i <= r->count; i += i & (~i + 1)) r->heights[i] += delta;
}
size_t doc_render_block_at(xui_document_renderer r, double y)
{
    size_t index = 0, bit = 1; double top = 0;
    while (bit <= r->count / 2) bit <<= 1;
    for (; bit; bit >>= 1) {
        size_t next = index + bit;
        if (next <= r->count && top + r->heights[next] <= y) { top += r->heights[next]; index = next; }
    }
    return index < r->count ? index : r->count ? r->count - 1 : 0;
}
int doc_render_materialize(xui_document_renderer r, size_t index)
{
    doc_render_block* b; double previous; int result;
    if (index >= r->count) return XUI_ERROR_NOT_FOUND;
    b = &r->blocks[index]; if (b->measured) return XUI_OK; previous = b->height;
    result = doc_layout_block(r, b, r->width);
    if (result == XUI_OK) doc_render_set_height(r, index, b->height - previous);
    else b->height = previous;
    return result;
}
static size_t doc_render_find(xui_document_renderer r, uint64_t id)
{
    doc_node* n = doc_index_get(r->snapshot->state->index, id);
    for (; n; n = doc_index_get(r->snapshot->state->index, n->parent)) {
        size_t* index = xrtMapGet(&r->block_index, xuiXrtBytes(&n->id, sizeof(n->id)));
        if (index) return *index;
    }
    return SIZE_MAX;
}
XUI_API int xuiDocumentRendererCreate(xui_context context, const xui_doc_renderer_desc_t* desc, xui_document_renderer* out)
{
    xui_document_renderer r;
    if (out) *out = NULL;
    if (!context || !out || (desc && (desc->iSize != sizeof(*desc) || !isfinite(desc->fZoom) || desc->fZoom < 0 ||
        !isfinite(desc->fParagraphGap) || desc->fParagraphGap < 0 || !isfinite(desc->fLineGap) || desc->fLineGap < 0 ||
        !isfinite(desc->fIndent) || desc->fIndent < 0))) return XUI_ERROR_INVALID_ARGUMENT;
    r = calloc(1, sizeof(*r)); if (!r) return XUI_ERROR_OUT_OF_MEMORY;
    r->context = context; r->proxy = &r->proxy_storage;
    if (xuiGetProxy(context, &r->proxy_storage) != XUI_OK) { free(r); return XUI_ERROR_INVALID_ARGUMENT; }
    if (desc) r->desc = *desc;
    r->desc.iSize = sizeof(r->desc); r->stats.iSize = sizeof(r->stats);
    if (r->desc.fZoom == 0) r->desc.fZoom = 1;
    if (r->desc.fIndent == 0) r->desc.fIndent = 24;
    if (r->desc.fParagraphGap == 0) r->desc.fParagraphGap = 8;
    if (!r->desc.tFonts.normal) r->desc.tFonts.normal = xuiGetDefaultFont(context);
    if (!r->desc.iTextColor) r->desc.iTextColor = XUI_COLOR_RGBA(32, 36, 44, 255);
    if (!r->desc.iBorderColor) r->desc.iBorderColor = XUI_COLOR_RGBA(160, 167, 178, 255);
    if (!r->desc.iCodeBackground) r->desc.iCodeBackground = XUI_COLOR_RGBA(240, 242, 246, 255);
    r->width = 640; r->mode = XUI_DOC_VISUAL;
    if (!r->proxy || !xrtMapInit(&r->block_index, sizeof(size_t))) { free(r); return XUI_ERROR_OUT_OF_MEMORY; }
    *out = r; return XUI_OK;
}
XUI_API void xuiDocumentRendererRelease(xui_document_renderer r)
{
    size_t i;
    if (!r) return;
    doc_render_clear(r); xrtMapDestroy(&r->block_index); xuiDocumentSnapshotRelease(r->snapshot);
    for (i = 0; i < r->font_count; i++) if (r->proxy->fontDestroy) r->proxy->fontDestroy(r->proxy, r->fonts[i].font);
    free(r->fonts); free(r);
}
XUI_API int xuiDocumentRendererSetSnapshot(xui_document_renderer r, xui_document_snapshot s, xui_document_change_set changes)
{
    int rebuild; size_t i;
    if (!r || !s) return XUI_ERROR_INVALID_ARGUMENT;
    if (r->mode != XUI_DOC_VISUAL && s->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    if (r->snapshot && r->snapshot->identity == s->identity && r->snapshot->revision == s->revision) return XUI_OK;
    rebuild = !r->snapshot || !changes || changes->identity != s->identity || changes->before_revision != r->snapshot->revision ||
        changes->after_revision != s->revision || changes->flags & (XUI_DOC_CHANGE_STRUCTURE | XUI_DOC_CHANGE_SOURCE | XUI_DOC_CHANGE_RESET);
    if (rebuild) {
        struct xui_document_renderer_t next = {0}; int result;
        next.snapshot = s; next.desc = r->desc; next.width = r->width; next.mode = r->mode; next.active_source = r->active_source;
        if (!xrtMapInit(&next.block_index, sizeof(size_t))) return XUI_ERROR_OUT_OF_MEMORY;
        if (r->mode == XUI_DOC_SOURCE_TEXT) result = doc_render_collect_source_range(&next, 0, doc_seq_size(s->state->source));
        else if (r->mode == XUI_DOC_LIVE_MARKDOWN) result = doc_render_collect_live(&next);
        else result = doc_render_collect(&next, DOC_ROOT, 0);
        if (result == XUI_OK && r->mode == XUI_DOC_VISUAL) result = doc_render_empty_tail(&next);
        if (result == XUI_OK) {
            next.heights = calloc(next.count + 1, sizeof(*next.heights));
            if (!next.heights) result = XUI_ERROR_OUT_OF_MEMORY;
            else for (i = 0; i < next.count; i++) doc_render_set_height(&next, i, next.blocks[i].height);
        }
        if (result != XUI_OK) { doc_render_clear(&next); xrtMapDestroy(&next.block_index); return result; }
        doc_render_clear(r); xrtMapDestroy(&r->block_index);
        r->blocks = next.blocks; r->heights = next.heights; r->count = next.count; r->capacity = next.capacity; r->block_index = next.block_index;
        r->active_source = next.active_source; r->live_start = next.live_start; r->live_end = next.live_end; r->live_fallback = next.live_fallback;
    } else {
        for (i = 0; i < changes->count; i++) {
            size_t index = doc_render_find(r, changes->ops[i].iNodeId);
            if (index != SIZE_MAX) doc_render_block_free(&r->blocks[index]);
        }
    }
    xuiDocumentSnapshotRetain(s); xuiDocumentSnapshotRelease(r->snapshot); r->snapshot = s;
    return XUI_OK;
}
XUI_API int xuiDocumentRendererSetMode(xui_document_renderer r, uint32_t mode)
{
    xui_document_snapshot s; uint32_t previous; int result;
    if (!r || (mode != XUI_DOC_VISUAL && mode != XUI_DOC_SOURCE_TEXT && mode != XUI_DOC_LIVE_MARKDOWN)) return XUI_ERROR_INVALID_ARGUMENT;
    if (r->mode == mode) return XUI_OK;
    if (mode != XUI_DOC_VISUAL && r->snapshot && r->snapshot->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    previous = r->mode; r->mode = mode;
    if (!r->snapshot) return XUI_OK;
    s = r->snapshot; r->snapshot = NULL;
    result = xuiDocumentRendererSetSnapshot(r, s, NULL);
    if (result != XUI_OK) { r->snapshot = s; r->mode = previous; }
    else xuiDocumentSnapshotRelease(s);
    return result;
}
XUI_API int xuiDocumentRendererSetActivePosition(xui_document_renderer r, const xui_doc_position_t* p)
{
    xui_document_snapshot s; uint64_t offset, previous; int mapping, result;
    if (!r || !r->snapshot || !p) return XUI_ERROR_INVALID_ARGUMENT;
    if (r->mode != XUI_DOC_LIVE_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    result = xuiDocumentPositionToSource(r->snapshot, p, &offset, &mapping);
    if (result != XUI_OK) return result;
    if (offset >= r->live_start && (offset < r->live_end || (offset == r->live_end && offset == doc_seq_size(r->snapshot->state->source)))) {
        r->active_source = offset; return XUI_OK;
    }
    previous = r->active_source; r->active_source = offset;
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
    size_t i, start; int result;
    if (!r || !r->snapshot || !isfinite(width) || !isfinite(top) || !isfinite(height) || width <= 0 || top < 0 || height < 0) return XUI_ERROR_INVALID_ARGUMENT;
    if (fabs(r->width - width) > .001) {
        for (i = 0; i < r->count; i++) doc_render_block_free(&r->blocks[i]);
        r->width = width;
    }
    r->viewport_top = top; r->viewport_height = height;
    start = doc_render_block_at(r, fmax(0, top - 128));
    for (i = start; i < r->count; i++) {
        if (i > start && doc_render_height_before(r, i) > top + height + 128) break;
        result = doc_render_materialize(r, i); if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
XUI_API int xuiDocumentRendererGetSize(xui_document_renderer r, xui_doc_rect_t* size, int* exact)
{
    size_t i;
    if (!r || !size || !exact) return XUI_ERROR_INVALID_ARGUMENT;
    *size = (xui_doc_rect_t){0, 0, 0, doc_render_height_before(r, r->count)}; *exact = 1;
    for (i = 0; i < r->count; i++) {
        if (!r->blocks[i].measured) *exact = 0;
        if (r->blocks[i].width > size->width) size->width = r->blocks[i].width;
    }
    return XUI_OK;
}
static void doc_render_position(xui_document_renderer r, uint64_t id, uint64_t offset, xui_doc_position_t* out)
{
    doc_node* n = doc_index_get(r->snapshot->state->index, id);
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out); out->iDocumentId = r->snapshot->identity; out->iRevision = r->snapshot->revision;
    out->iNodeId = id; out->iKind = n && doc_text_kind(n->kind) ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
    if (r->mode == XUI_DOC_SOURCE_TEXT) out->iKind = XUI_DOC_POSITION_SOURCE;
    out->iOffset = offset; out->iAffinity = XUI_DOC_AFTER;
    if (r->mode == XUI_DOC_LIVE_MARKDOWN) {
        int mapping; uint64_t source = offset;
        if (id == DOC_ROOT || xuiDocumentPositionToSource(r->snapshot, out, &source, &mapping) == XUI_OK) {
            out->iNodeId = DOC_ROOT; out->iKind = XUI_DOC_POSITION_SOURCE; out->iOffset = source;
        }
    }
}
XUI_API int xuiDocumentRendererHitTest(xui_document_renderer r, double x, double y, xui_doc_position_t* out)
{
    size_t index, i, best = SIZE_MAX; doc_render_block* b; double best_distance = DBL_MAX, top; int result;
    if (!r || !r->snapshot || !out || !isfinite(x) || !isfinite(y)) return XUI_ERROR_INVALID_ARGUMENT;
    if (!r->count) { doc_render_position(r, DOC_ROOT, 0, out); return XUI_OK; }
    index = doc_render_block_at(r, y); result = doc_render_materialize(r, index); if (result != XUI_OK) return result;
    b = &r->blocks[index]; top = doc_render_height_before(r, index);
    for (i = 0; i < b->fragment_count; i++) {
        doc_fragment* f = &b->fragments[i];
        double dx = x < f->x ? f->x - x : x > f->x + f->width ? x - f->x - f->width : 0;
        double dy = y < top + f->y ? top + f->y - y : y > top + f->y + f->height ? y - top - f->y - f->height : 0;
        double distance = dy * dy * 16 + dx * dx;
        if (distance < best_distance) { best = i; best_distance = distance; }
    }
    if (best == SIZE_MAX) doc_render_position(r, b->node, b->source_line ? b->source_start : 0, out);
    else { doc_fragment* f = &b->fragments[best]; doc_render_position(r, f->node, x >= f->x + f->width / 2 ? f->end : f->start, out); }
    return XUI_OK;
}
XUI_API int xuiDocumentRendererGetCaretRect(xui_document_renderer r, const xui_doc_position_t* p, xui_doc_rect_t* out)
{
    size_t index, i; doc_render_block* b; double top; int result; xui_doc_position_t clamped;
    if (!r || !r->snapshot || !p || !out || !doc_position_valid(r->snapshot->state, p)) return XUI_ERROR_INVALID_ARGUMENT;
    if (p->iDocumentId != r->snapshot->identity || p->iRevision != r->snapshot->revision) return XUI_DOC_ERROR_STALE;
    if (!r->count) { *out = (xui_doc_rect_t){0, 0, 1, 20 * r->desc.fZoom}; return XUI_OK; }
    if (p->iKind == XUI_DOC_POSITION_SOURCE && r->mode == XUI_DOC_LIVE_MARKDOWN) {
        for (index = 0; index < r->count; index++) {
            b = &r->blocks[index];
            if (b->source_line && p->iOffset >= b->source_start) {
                char crlf[2] = {0};
                if (p->iOffset <= b->source_end) goto located;
                if (p->iOffset == b->source_end + 1 && b->source_end + 1 < doc_seq_size(r->snapshot->state->source)) {
                    doc_seq_read(r->snapshot->state->source, b->source_end, crlf, 2);
                    if (crlf[0] == '\r' && crlf[1] == '\n') goto located;
                }
            }
        }
        /* Inactive source syntax projects to the nearest semantic position.
         * The owning View activates its caret before asking for geometry. */
        { xui_doc_position_t mapped; int mapping;
          result = xuiDocumentSourceToPosition(r->snapshot, p->iOffset, &mapped, &mapping);
          return result == XUI_OK ? xuiDocumentRendererGetCaretRect(r, &mapped, out) : result; }
    }
    if (p->iKind == XUI_DOC_POSITION_GAP) {
        doc_node* n = doc_index_get(r->snapshot->state->index, p->iNodeId);
        uint64_t count = doc_seq_size(n->children);
        if (count && !(p->iNodeId == DOC_ROOT && p->iOffset == count && r->blocks[r->count - 1].virtual_gap)) {
            int end = p->iOffset == count; xui_doc_position_t edge = *p;
            n = doc_index_get(r->snapshot->state->index, doc_seq_get_id(n->children, end ? count - 1 : p->iOffset));
            while (doc_seq_size(n->children)) n = doc_index_get(r->snapshot->state->index, doc_seq_get_id(n->children, end ? doc_seq_size(n->children) - 1 : 0));
            edge.iNodeId = n->id; edge.iKind = doc_text_kind(n->kind) ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
            edge.iOffset = end ? doc_seq_size(n->text) : 0;
            return xuiDocumentRendererGetCaretRect(r, &edge, out);
        }
    }
    if (p->iKind == XUI_DOC_POSITION_SOURCE && r->mode == XUI_DOC_SOURCE_TEXT) {
        size_t low = 0, high = r->count;
        while (low < high) { size_t mid = low + (high - low) / 2; if (r->blocks[mid].source_start <= p->iOffset) low = mid + 1; else high = mid; }
        index = low ? low - 1 : 0;
    } else { index = doc_render_find(r, p->iNodeId); if (index == SIZE_MAX) return XUI_ERROR_NOT_FOUND; }
located:
    result = doc_render_materialize(r, index); if (result != XUI_OK) return result;
    b = &r->blocks[index]; top = doc_render_height_before(r, index);
    if (b->source_line && p->iOffset > b->source_end) { clamped = *p; clamped.iOffset = b->source_end; p = &clamped; }
    for (i = 0; i < b->fragment_count; i++) {
        doc_fragment* f = &b->fragments[i];
        if (f->node == p->iNodeId && p->iOffset >= f->start && p->iOffset <= f->end) {
            *out = (xui_doc_rect_t){f->x + (p->iOffset == f->end && f->end > f->start ? f->width : 0), top + f->y, 1, f->height}; return XUI_OK;
        }
    }
    return XUI_ERROR_NOT_FOUND;
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
    if (!range || !f->end || range->tAnchor.iRevision != r->snapshot->revision || range->tCaret.iRevision != r->snapshot->revision) return 0;
    if (doc_position_compare(r->snapshot->state, &range->tAnchor, &range->tCaret, &order) != XUI_OK || !order) return 0;
    a = order < 0 ? &range->tAnchor : &range->tCaret; b = order < 0 ? &range->tCaret : &range->tAnchor;
    doc_render_position(r, f->node, f->start, &start); doc_render_position(r, f->node, f->end, &end);
    return doc_position_compare(r->snapshot->state, a, &end, &left) == XUI_OK && left < 0 &&
        doc_position_compare(r->snapshot->state, b, &start, &right) == XUI_OK && right > 0;
}
XUI_API int xuiDocumentRendererDraw(xui_document_renderer r, xui_draw_context draw, double ox, double oy, xui_rect_t clip,
    const xui_doc_range_t* selection, uint32_t selection_color)
{
    size_t index, i; xui_rect_t saved = {0}; int had_clip = 0, result = XUI_OK;
    if (!r || !r->snapshot || !draw || !isfinite(ox) || !isfinite(oy)) return XUI_ERROR_INVALID_ARGUMENT;
    if (r->proxy->drawClipGet) r->proxy->drawClipGet(r->proxy, draw, &saved, &had_clip);
    if (had_clip) {
        float right = fminf(saved.fX + saved.fW, clip.fX + clip.fW), bottom = fminf(saved.fY + saved.fH, clip.fY + clip.fH);
        clip.fX = fmaxf(saved.fX, clip.fX); clip.fY = fmaxf(saved.fY, clip.fY); clip.fW = fmaxf(0, right - clip.fX); clip.fH = fmaxf(0, bottom - clip.fY);
    }
    if (r->proxy->drawClipSet) r->proxy->drawClipSet(r->proxy, draw, clip);
    index = doc_render_block_at(r, fmax(0, clip.fY - oy));
    for (; index < r->count; index++) {
        doc_render_block* b; double top = oy + doc_render_height_before(r, index);
        if (top > clip.fY + clip.fH) break;
        result = doc_render_materialize(r, index); if (result != XUI_OK) break;
        b = &r->blocks[index];
        for (i = b->box_count; i > 0; i--) {
            doc_box* box = &b->boxes[i - 1]; xui_rect_t rect = doc_render_rect(ox + box->x, top + box->y, box->width, box->height);
            if (!doc_render_visible(rect, clip)) continue;
            if (box->color && r->proxy->drawRectFill) r->proxy->drawRectFill(r->proxy, draw, rect, box->color);
            if (box->kind == XUI_DOC_CELL && r->proxy->drawRectStroke) r->proxy->drawRectStroke(r->proxy, draw, rect, 1, r->desc.iBorderColor);
        }
        if (b->marker_text[0] && r->proxy->drawText) r->proxy->drawText(r->proxy, draw, r->desc.tFonts.normal, b->marker_text,
            doc_render_rect(ox + fmax(0, b->indent - r->desc.fIndent * r->desc.fZoom), top, r->desc.fIndent * r->desc.fZoom, 24 * r->desc.fZoom), r->desc.iTextColor, XUI_TEXT_CLIP);
        for (i = 0; i < b->fragment_count; i++) {
            doc_fragment* f = &b->fragments[i]; xui_rect_t rect = doc_render_rect(ox + f->x, top + f->y, f->width, f->height);
            if (!doc_render_visible(rect, clip)) continue;
            if (f->run != UINT32_MAX && b->runs[f->run].attrs.iBackgroundColor && r->proxy->drawRectFill)
                r->proxy->drawRectFill(r->proxy, draw, rect, b->runs[f->run].attrs.iBackgroundColor);
            if (doc_render_selected(r, f, selection) && r->proxy->drawRectFill) r->proxy->drawRectFill(r->proxy, draw, rect, selection_color);
        }
        for (i = 0; i < b->fragment_count; ) {
            doc_fragment* f = &b->fragments[i]; size_t end = i + 1;
            xui_rect_t rect = doc_render_rect(ox + f->x, top + f->y, f->width, f->height);
            if (f->run == UINT32_MAX) {
                if (doc_render_visible(rect, clip) && f->kind != XUI_DOC_HARD_BREAK && f->kind != XUI_DOC_SOFT_BREAK && f->kind != XUI_DOC_PARAGRAPH && f->kind != XUI_DOC_HEADING && f->kind != XUI_DOC_CELL && f->kind != XUI_DOC_ROOT) {
                    int object_result = r->desc.onObjectDraw ? r->desc.onObjectDraw(r->snapshot, f->node, r->proxy, draw, rect, r->desc.pUser) : XUI_ERROR_UNSUPPORTED;
                    if (object_result == XUI_ERROR_UNSUPPORTED && r->proxy->drawRectStroke) r->proxy->drawRectStroke(r->proxy, draw, rect, 1, r->desc.iBorderColor);
                    else if (object_result != XUI_OK && object_result != XUI_ERROR_UNSUPPORTED) { result = object_result; break; }
                }
            } else {
                doc_render_run* run = &b->runs[f->run]; uint64_t text_end = f->end; double width = f->width;
                uint32_t color = run->attrs.iTextColor ? run->attrs.iTextColor : r->desc.iTextColor;
                while (end < b->fragment_count) {
                    doc_fragment* next = &b->fragments[end];
                    if (next->run != f->run || next->start != text_end || fabs(next->y - f->y) > .001 || fabs(next->x - f->x - width) > .001) break;
                    width += next->width; text_end = next->end; end++;
                }
                rect = doc_render_rect(ox + f->x, top + f->y, width, f->height);
                if (doc_render_visible(rect, clip) && text_end > f->start) {
                    char terminator = run->text[text_end - run->base_offset]; run->text[text_end - run->base_offset] = 0;
                    if (r->proxy->drawText) result = r->proxy->drawText(r->proxy, draw, run->font, run->text + f->start - run->base_offset, rect, color,
                        XUI_TEXT_CLIP | (run->attrs.iMarks & XUI_DOC_UNDERLINE ? XUI_TEXT_UNDERLINE : 0));
                    run->text[text_end - run->base_offset] = terminator;
                    if (run->attrs.iMarks & XUI_DOC_STRIKE && r->proxy->drawLine) r->proxy->drawLine(r->proxy, draw, rect.fX, rect.fY + rect.fH * .52f, rect.fX + rect.fW, rect.fY + rect.fH * .52f, 1, color);
                    if (result != XUI_OK) break;
                }
            }
            i = end;
        }
        if (result != XUI_OK) break;
    }
    if (had_clip && r->proxy->drawClipSet) r->proxy->drawClipSet(r->proxy, draw, saved);
    else if (r->proxy->drawClipClear) r->proxy->drawClipClear(r->proxy, draw);
    return result;
}
XUI_API int xuiDocumentRendererGetStats(xui_document_renderer r, xui_doc_renderer_stats_t* out)
{
    size_t i;
    if (!r || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    *out = r->stats; out->iBlocks = r->count; out->iSourceBlocks = 0; out->bLiveSourceFallback = r->live_fallback;
    for (i = 0; i < r->count; i++) out->iSourceBlocks += !!r->blocks[i].source_line;
    return XUI_OK;
}
