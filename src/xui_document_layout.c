#include "xui_document_layout_internal.h"
#include "xui_text_internal.h"
#include <math.h>
#include <stdio.h>

#define DOC_FORCED_BREAK UINT32_C(0x80000000)
#define DOC_NORMAL_BREAK UINT32_C(0x40000000)
#define DOC_EMERGENCY_BREAK UINT32_C(0x20000000)
int doc_render_reserve(void** data, size_t* capacity, size_t count, size_t item)
{
    size_t n; void* p;
    if (count <= *capacity) return XUI_OK;
    n = *capacity ? *capacity : 16;
    while (n < count) { if (n > SIZE_MAX / 2) { n = count; break; } n *= 2; }
    if (n > SIZE_MAX / item) return XUI_DOC_ERROR_LIMIT;
    p = realloc(*data, n * item); if (!p) return XUI_ERROR_OUT_OF_MEMORY;
    *data = p; *capacity = n; return XUI_OK;
}
void doc_render_block_free(doc_render_block* b)
{
    size_t i;
    for (i = 0; i < b->run_count; i++) free(b->runs[i].text);
    free(b->runs); free(b->fragments); free(b->boxes);
    b->runs = NULL; b->fragments = NULL; b->boxes = NULL;
    b->run_count = b->run_capacity = b->fragment_count = b->fragment_capacity = b->box_count = b->box_capacity = 0;
    b->measured = 0;
}
static xui_font doc_layout_font(xui_document_renderer r, const xui_doc_attributes_t* a, uint32_t heading)
{
    xui_font source = r->desc.tFonts.normal, font = NULL; xui_font_metrics_t m = {0};
    float size = a->fFontSize; uint32_t marks = a->iMarks; size_t i;
    if (heading) marks |= XUI_DOC_BOLD;
    if (marks & XUI_DOC_CODE && r->desc.tFonts.monospace) source = r->desc.tFonts.monospace;
    else if ((marks & (XUI_DOC_BOLD | XUI_DOC_ITALIC)) == (XUI_DOC_BOLD | XUI_DOC_ITALIC) && r->desc.tFonts.boldItalic) source = r->desc.tFonts.boldItalic;
    else if (marks & XUI_DOC_BOLD && r->desc.tFonts.bold) source = r->desc.tFonts.bold;
    else if (marks & XUI_DOC_ITALIC && r->desc.tFonts.italic) source = r->desc.tFonts.italic;
    if (r->proxy->fontGetMetrics) r->proxy->fontGetMetrics(r->proxy, source, &m);
    if (size <= 0) size = m.fSize > 0 ? m.fSize : 16;
    if (heading && a->fFontSize <= 0) size *= heading == 1 ? 1.8f : heading == 2 ? 1.5f : heading == 3 ? 1.3f : 1.1f;
    if (marks & (XUI_DOC_SUBSCRIPT | XUI_DOC_SUPERSCRIPT)) size *= .75f;
    size *= r->desc.fZoom;
    if (r->desc.onFont) font = r->desc.onFont(r->context, a->sFontFamily, marks, size, r->desc.pUser);
    if (font) return font;
    if (fabsf(m.fSize - size) < .01f) return source;
    for (i = 0; i < r->font_count; i++) if (r->fonts[i].source == source && fabsf(r->fonts[i].size - size) < .01f) return r->fonts[i].font;
    if (!r->proxy->fontCreateSized || r->proxy->fontCreateSized(r->proxy, &font, source, size) != XUI_OK) return source;
    if (doc_render_reserve((void**)&r->fonts, &r->font_capacity, r->font_count + 1, sizeof(*r->fonts)) != XUI_OK) {
        if (r->proxy->fontDestroy) r->proxy->fontDestroy(r->proxy, font);
        return NULL;
    }
    r->fonts[r->font_count++] = (doc_sized_font){source, font, size}; return font;
}
static int doc_layout_fragment(doc_render_block* b, const doc_fragment* f)
{
    int result = doc_render_reserve((void**)&b->fragments, &b->fragment_capacity, b->fragment_count + 1, sizeof(*b->fragments));
    if (result == XUI_OK) b->fragments[b->fragment_count++] = *f;
    return result;
}
static int doc_layout_box(doc_render_block* b, uint64_t node, uint32_t kind, uint32_t color, double x, double y, double w, double h)
{
    int result = doc_render_reserve((void**)&b->boxes, &b->box_capacity, b->box_count + 1, sizeof(*b->boxes));
    if (result == XUI_OK) b->boxes[b->box_count++] = (doc_box){node, kind, color, x, y, w, h};
    return result;
}
static int doc_layout_run(xui_document_renderer r, doc_render_block* b, doc_node* n, uint32_t heading, const char* generated, uint64_t generated_bytes)
{
    doc_render_run* run; xui_text_shape_t shape = {0}; doc_fragment f = {0};
    uint64_t bytes = generated ? generated_bytes : doc_seq_size(n->text); int result, i;
    if (bytes > INT_MAX || b->run_count >= UINT32_MAX) return XUI_DOC_ERROR_LIMIT;
    result = doc_render_reserve((void**)&b->runs, &b->run_capacity, b->run_count + 1, sizeof(*b->runs));
    if (result != XUI_OK) return result;
    run = &b->runs[b->run_count]; memset(run, 0, sizeof(*run));
    run->node = n->id; run->attrs = n->attrs; run->bytes = bytes;
    if (n->kind == XUI_DOC_CODE_BLOCK || n->kind == XUI_DOC_DIAGRAM || n->kind == XUI_DOC_FRONT_MATTER) run->attrs.iMarks |= XUI_DOC_CODE;
    run->font = doc_layout_font(r, &run->attrs, heading);
    if (!run->font) return XUI_ERROR_UNSUPPORTED;
    run->text = malloc((size_t)bytes + 1); if (!run->text) return XUI_ERROR_OUT_OF_MEMORY;
    if (generated) memcpy(run->text, generated, (size_t)bytes); else doc_seq_read(n->text, 0, run->text, bytes);
    run->text[bytes] = 0; f.run = (uint32_t)b->run_count++; f.node = n->id; f.kind = n->kind;
    result = xuiTextShape(r->context, run->font, run->text, (int)bytes, XUI_TEXT_SHAPE_DEFAULT, &shape);
    if (result != XUI_OK) return result;
    r->stats.iShapedBytes += bytes;
    if (!shape.iClusterCount) {
        f.height = shape.fLineHeight > 0 ? shape.fLineHeight : 20 * r->desc.fZoom;
        f.baseline = shape.fAscent > 0 ? shape.fAscent : f.height * .8;
        result = doc_layout_fragment(b, &f);
    }
    for (i = 0; i < shape.iClusterCount && result == XUI_OK; i++) {
        xui_text_cluster_t* cluster = &shape.pClusters[i];
        f.start = (uint64_t)cluster->iTextStart; f.end = (uint64_t)cluster->iTextEnd;
        f.width = cluster->fAdvance; f.height = shape.fLineHeight; f.baseline = shape.fAscent;
        f.flags = cluster->iFlags;
        if (cluster->iTextStart < (int)bytes && (run->text[cluster->iTextStart] == '\n' || run->text[cluster->iTextStart] == '\r')) { f.flags |= DOC_FORCED_BREAK; f.width = 0; }
        result = doc_layout_fragment(b, &f);
    }
    xuiTextShapeFree(&shape); return result;
}
static int doc_layout_inline(xui_document_renderer r, doc_render_block* b, doc_node* n, uint32_t heading)
{
    doc_fragment f = {0};
    if (n->kind == XUI_DOC_TEXT || n->kind == XUI_DOC_FOOTNOTE_REF || n->kind == XUI_DOC_CODE_BLOCK || n->kind == XUI_DOC_FRONT_MATTER) return doc_layout_run(r, b, n, heading, NULL, 0);
    f.node = n->id; f.kind = n->kind; f.run = UINT32_MAX;
    f.height = 20 * r->desc.fZoom; f.baseline = f.height * .8;
    if (n->kind == XUI_DOC_HARD_BREAK) f.flags = DOC_FORCED_BREAK;
    else if (n->kind == XUI_DOC_SOFT_BREAK) { f.width = 6 * r->desc.fZoom; f.flags = XUI_TEXT_CLUSTER_LINE_BREAK; }
    else {
        xui_vec2_t size = {0}; float baseline = 0;
        int result = r->desc.onObjectMeasure ? r->desc.onObjectMeasure(r->snapshot, n->id, (float)r->width, r->desc.fZoom, &size, &baseline, r->desc.pUser) : XUI_ERROR_UNSUPPORTED;
        if (result != XUI_OK && result != XUI_ERROR_UNSUPPORTED) return result;
        f.width = result == XUI_OK ? size.fX : n->attrs.fWidth > 0 ? n->attrs.fWidth * r->desc.fZoom : 96 * r->desc.fZoom;
        f.height = result == XUI_OK ? size.fY : n->attrs.fHeight > 0 ? n->attrs.fHeight * r->desc.fZoom : 32 * r->desc.fZoom;
        f.baseline = baseline > 0 ? baseline : f.height;
        if (!isfinite(f.width) || !isfinite(f.height) || f.width < 0 || f.height < 0) return XUI_ERROR_INVALID_ARGUMENT;
        f.end = doc_seq_size(n->text);
    }
    return doc_layout_fragment(b, &f);
}
static int doc_layout_breaks(doc_render_block* b, size_t start)
{
    size_t i; uint64_t bytes = 0, at = 0; char* text; unsigned char* map; int result;
    for (i = start; i < b->fragment_count; i++) bytes += b->fragments[i].run != UINT32_MAX ? b->fragments[i].end - b->fragments[i].start : 3;
    if (bytes > INT_MAX) return XUI_DOC_ERROR_LIMIT;
    text = malloc((size_t)bytes + 1); map = calloc((size_t)bytes + 1, 1);
    if (!text || !map) { free(text); free(map); return XUI_ERROR_OUT_OF_MEMORY; }
    for (i = start; i < b->fragment_count; i++) {
        doc_fragment* f = &b->fragments[i]; uint64_t length = f->run != UINT32_MAX ? f->end - f->start : 3;
        if (f->run != UINT32_MAX) memcpy(text + at, b->runs[f->run].text + f->start - b->runs[f->run].base_offset, (size_t)length);
        else if (f->kind == XUI_DOC_SOFT_BREAK) memcpy(text + at, "   ", 3);
        else if (f->kind == XUI_DOC_HARD_BREAK) memcpy(text + at, "\n\n\n", 3);
        else memcpy(text + at, "\xef\xbf\xbc", 3);
        at += length;
    }
    text[at] = 0; result = xuiInternalTextBreakMap(text, (int)bytes, map);
    if (result == XUI_OK) for (i = start, at = 0; i < b->fragment_count; i++) {
        doc_fragment* f = &b->fragments[i];
        at += f->run != UINT32_MAX ? f->end - f->start : 3;
        f->flags &= ~(DOC_NORMAL_BREAK | DOC_EMERGENCY_BREAK);
        if (map[at] & XUI_LB_NORMAL) f->flags |= DOC_NORMAL_BREAK;
        if (map[at] & XUI_LB_EMERGENCY) f->flags |= DOC_EMERGENCY_BREAK;
    }
    free(text); free(map); return result;
}
static double doc_layout_lines(xui_document_renderer r, doc_render_block* b, size_t start, double x, double y, double width, uint32_t alignment, int wrap)
{
    size_t end = b->fragment_count, first = start;
    double top = y;
    while (first < end) {
        size_t at = first, stop, last_break = first, emergency = first, i; double advance = 0, ascent = 0, descent = 0, total = 0;
        while (at < end) {
            doc_fragment* f = &b->fragments[at];
            if (wrap && at > first && advance + f->width > width && (last_break > first || emergency > first)) break;
            advance += f->width; at++;
            if (f->flags & DOC_NORMAL_BREAK) last_break = at;
            if (f->flags & DOC_EMERGENCY_BREAK) emergency = at;
            if (f->flags & DOC_FORCED_BREAK) break;
            if (wrap && advance > width && (last_break > first || emergency > first)) break;
        }
        stop = at;
        if (wrap && at < end && at > first && !(b->fragments[at - 1].flags & DOC_FORCED_BREAK)) {
            if (last_break > first) stop = last_break;
            else if (emergency > first) stop = emergency;
        }
        if (stop == first) stop++;
        for (i = first; i < stop; i++) {
            doc_fragment* f = &b->fragments[i]; total += f->width;
            if (f->baseline > ascent) ascent = f->baseline;
            if (f->height - f->baseline > descent) descent = f->height - f->baseline;
        }
        advance = x + (alignment == 1 ? fmax(0, width - total) / 2 : alignment == 2 ? fmax(0, width - total) : 0);
        for (i = first; i < stop; i++) {
            doc_fragment* f = &b->fragments[i];
            f->x = advance; f->y = top + ascent - f->baseline; advance += f->width;
        }
        if (advance > b->width) b->width = advance;
        top += fmax(1, ascent + descent) + r->desc.fLineGap * r->desc.fZoom; first = stop;
    }
    if (start == end) top += 20 * r->desc.fZoom;
    return top - y;
}
static int doc_layout_node(xui_document_renderer, doc_render_block*, uint64_t, double, double, double, double*);
typedef struct doc_cell_layout {
    doc_node* node; size_t fragment_start, fragment_end, box_start, box_end;
    uint64_t row; unsigned column; double height;
} doc_cell_layout;
static int doc_layout_table(xui_document_renderer r, doc_render_block* b, doc_node* table, double x, double y, double width, double* height)
{
    doc_state* s = r->snapshot->state; uint16_t occupied[1024] = {0};
    uint64_t row, rows = doc_seq_size(table->children), i; unsigned columns = 0;
    doc_cell_layout* cells = NULL; size_t count = 0, capacity = 0; double *heights = NULL, *tops = NULL;
    int result = XUI_OK; double padding = 6 * r->desc.fZoom, column_width;
    if (rows > SIZE_MAX / sizeof(double) - 1) return XUI_DOC_ERROR_LIMIT;
    heights = calloc((size_t)rows + 1, sizeof(*heights)); tops = calloc((size_t)rows + 1, sizeof(*tops));
    if (!heights || !tops) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    for (row = 0; row < rows; row++) {
        doc_node* row_node = doc_index_get(s->index, doc_seq_get_id(table->children, row)); unsigned column = 0, k;
        for (i = 0; i < doc_seq_size(row_node->children); i++) {
            doc_node* cell = doc_index_get(s->index, doc_seq_get_id(row_node->children, i)); doc_cell_layout* c;
            while (column < 1024 && occupied[column]) column++;
            if (cell->attrs.iColumnSpan > 1024 - column || cell->attrs.iRowSpan > rows - row) { result = XUI_DOC_ERROR_SCHEMA; goto done; }
            result = doc_render_reserve((void**)&cells, &capacity, count + 1, sizeof(*cells)); if (result != XUI_OK) goto done;
            c = &cells[count++]; memset(c, 0, sizeof(*c)); c->node = cell; c->row = row; c->column = column;
            for (k = 0; k < cell->attrs.iColumnSpan; k++) occupied[column + k] = (uint16_t)cell->attrs.iRowSpan;
            column += cell->attrs.iColumnSpan; if (column > columns) columns = column;
        }
        for (k = 0; k < 1024; k++) if (occupied[k]) occupied[k]--;
    }
    column_width = columns ? width / columns : width;
    for (i = 0; i < count; i++) {
        doc_cell_layout* c = &cells[i]; double cy = padding; uint64_t child;
        c->fragment_start = b->fragment_count; c->box_start = b->box_count;
        if (!doc_seq_size(c->node->children)) {
            doc_fragment empty = {0}; empty.node = c->node->id; empty.kind = XUI_DOC_CELL; empty.run = UINT32_MAX;
            empty.x = x + c->column * column_width + padding; empty.y = padding;
            empty.height = 20 * r->desc.fZoom; empty.baseline = empty.height * .8;
            result = doc_layout_fragment(b, &empty); if (result != XUI_OK) goto done;
        }
        for (child = 0; child < doc_seq_size(c->node->children); child++) {
            double h;
            result = doc_layout_node(r, b, doc_seq_get_id(c->node->children, child), x + c->column * column_width + padding, cy,
                fmax(1, c->node->attrs.iColumnSpan * column_width - padding * 2), &h);
            if (result != XUI_OK) goto done;
            cy += h;
        }
        c->height = fmax(20 * r->desc.fZoom, cy) + padding;
        c->fragment_end = b->fragment_count; c->box_end = b->box_count;
        if (c->node->attrs.iRowSpan == 1 && c->height > heights[c->row]) heights[c->row] = c->height;
    }
    for (i = 0; i < count; i++) {
        doc_cell_layout* c = &cells[i]; double sum = 0; unsigned k;
        for (k = 0; k < c->node->attrs.iRowSpan; k++) sum += heights[c->row + k];
        if (sum < c->height) heights[c->row + c->node->attrs.iRowSpan - 1] += c->height - sum;
    }
    for (row = 0; row < rows; row++) tops[row + 1] = tops[row] + heights[row];
    for (i = 0; i < count; i++) {
        doc_cell_layout* c = &cells[i]; size_t f; double top = y + tops[c->row];
        for (f = c->fragment_start; f < c->fragment_end; f++) b->fragments[f].y += top;
        for (f = c->box_start; f < c->box_end; f++) b->boxes[f].y += top;
        result = doc_layout_box(b, c->node->id, XUI_DOC_CELL, c->node->attrs.iBackgroundColor, x + c->column * column_width, top,
            column_width * c->node->attrs.iColumnSpan, tops[c->row + c->node->attrs.iRowSpan] - tops[c->row]);
        if (result != XUI_OK) goto done;
    }
    *height = tops[rows]; if (x + width > b->width) b->width = x + width;
done:
    free(cells); free(heights); free(tops); return result;
}
static int doc_layout_node(xui_document_renderer r, doc_render_block* b, uint64_t id, double x, double y, double width, double* height)
{
    doc_node* n = doc_index_get(r->snapshot->state->index, id); uint64_t i; int result = XUI_OK;
    size_t first = b->fragment_count; double h = 0;
    if (!n) return XUI_ERROR_NOT_FOUND;
    if (n->kind == XUI_DOC_TABLE) return doc_layout_table(r, b, n, x, y, width, height);
    if (n->kind == XUI_DOC_RULE) { *height = 16 * r->desc.fZoom; return doc_layout_box(b, id, n->kind, r->desc.iBorderColor, x, y + *height / 2, width, 1); }
    if (n->kind == XUI_DOC_PARAGRAPH || n->kind == XUI_DOC_HEADING) {
        for (i = 0; i < doc_seq_size(n->children); i++) {
            result = doc_layout_inline(r, b, doc_index_get(r->snapshot->state->index, doc_seq_get_id(n->children, i)),
                n->kind == XUI_DOC_HEADING ? n->attrs.iHeadingLevel : 0);
            if (result != XUI_OK) return result;
        }
        if (first == b->fragment_count) {
            doc_fragment empty = {0}; empty.node = id; empty.kind = n->kind; empty.run = UINT32_MAX;
            empty.height = 20 * r->desc.fZoom; empty.baseline = empty.height * .8;
            result = doc_layout_fragment(b, &empty); if (result != XUI_OK) return result;
        }
        result = doc_layout_breaks(b, first); if (result != XUI_OK) return result;
        h = doc_layout_lines(r, b, first, x, y, width, n->attrs.iAlignment, 1);
    } else if (doc_text_kind(n->kind)) {
        result = doc_layout_inline(r, b, n, 0); if (result != XUI_OK) return result;
        result = doc_layout_breaks(b, first); if (result != XUI_OK) return result;
        h = doc_layout_lines(r, b, first, x + (n->kind == XUI_DOC_CODE_BLOCK ? 8 : 0), y, width, 0, n->kind != XUI_DOC_CODE_BLOCK);
        if (n->kind == XUI_DOC_CODE_BLOCK) result = doc_layout_box(b, id, n->kind, r->desc.iCodeBackground, x, y, width, h);
    } else {
        double indent = n->kind == XUI_DOC_QUOTE || n->kind == XUI_DOC_LIST_ITEM ? r->desc.fIndent * r->desc.fZoom : 0;
        for (i = 0; i < doc_seq_size(n->children); i++) {
            double child_height;
            result = doc_layout_node(r, b, doc_seq_get_id(n->children, i), x + indent, y + h, fmax(1, width - indent), &child_height);
            if (result != XUI_OK) return result;
            h += child_height;
        }
        if (n->kind == XUI_DOC_QUOTE) result = doc_layout_box(b, id, n->kind, r->desc.iBorderColor, x + 3, y, 2, h);
    }
    *height = h + (n->attrs.fParagraphSpacing > 0 ? n->attrs.fParagraphSpacing : r->desc.fParagraphGap) * r->desc.fZoom;
    return result;
}
int doc_layout_block(xui_document_renderer r, doc_render_block* b, double width)
{
    int result;
    doc_render_block_free(b); b->width = 0;
    if (b->virtual_gap) {
        doc_fragment f = {0}; f.node = DOC_ROOT; f.kind = XUI_DOC_ROOT; f.run = UINT32_MAX;
        f.start = f.end = doc_seq_size(doc_index_get(r->snapshot->state->index, DOC_ROOT)->children);
        f.height = b->height = 20 * r->desc.fZoom; f.baseline = f.height * .8;
        result = doc_layout_fragment(b, &f);
    } else if (b->source_line) {
        doc_node source_node = {0}; uint64_t bytes = b->source_end - b->source_start; size_t i; char* text;
        if (bytes > INT_MAX) return XUI_DOC_ERROR_LIMIT;
        text = malloc((size_t)bytes + 1); if (!text) return XUI_ERROR_OUT_OF_MEMORY;
        doc_seq_read(r->snapshot->state->source, b->source_start, text, bytes); text[bytes] = 0;
        source_node.id = DOC_ROOT; source_node.kind = XUI_DOC_TEXT; source_node.attrs.iMarks = XUI_DOC_CODE;
        result = doc_layout_run(r, b, &source_node, 0, text, bytes); free(text);
        if (result == XUI_OK) {
            b->height = doc_layout_lines(r, b, 0, 0, 0, width, 0, 0);
            for (i = 0; i < b->fragment_count; i++) { b->fragments[i].start += b->source_start; b->fragments[i].end += b->source_start; }
            for (i = 0; i < b->run_count; i++) b->runs[i].base_offset = b->source_start;
        }
    } else result = doc_layout_node(r, b, b->node, b->indent, 0, fmax(1, width - b->indent), &b->height);
    if (result != XUI_OK) { doc_render_block_free(b); return result; }
    b->measured = 1; r->stats.iMeasuredBlocks++; r->stats.iLayoutPasses++;
    r->stats.iFragments += b->fragment_count; return XUI_OK;
}
