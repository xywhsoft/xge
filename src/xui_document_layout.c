#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT_VIEW
#include "xui_document_layout_internal.h"
#include "xui_text_internal.h"
#include "xui_unicode_core.h"
#include <float.h>
#include <math.h>
#include <stdio.h>

#define DOC_NORMAL_BREAK UINT32_C(0x40000000)
#define DOC_EMERGENCY_BREAK UINT32_C(0x20000000)
/* SOURCE rows use a temporary, nonpersistent node; it never enters a state. */
static const xui_doc_attributes_t doc_source_line_attrs = { .iMarks = XUI_DOC_CODE };
typedef struct doc_paragraph_span {
    uint64_t node, start, end;
    uint32_t kind;
    size_t first_fragment, end_fragment;
} doc_paragraph_span;
struct doc_paragraph_projection {
    char* text;
    unsigned char* breaks;
    doc_paragraph_span* spans;
    uint64_t* cuts;
    size_t cut_count, cut_capacity;
    uint64_t bytes, count;
    xui_text_bidi bidi;
    doc_shape_context* shaping; /* Borrowed from the block's retained contexts. */
};
static void doc_layout_projection_free(doc_paragraph_projection* projection)
{
    free(projection->text); free(projection->breaks);
    free(projection->spans); free(projection->cuts);
    xuiInternalTextBidiFree(projection->bidi);
    memset(projection, 0, sizeof(*projection));
}
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
#include "xui_document_shape_context.inl"
static int doc_layout_context_acquire(xui_document_renderer r,doc_render_block* block,uint64_t node,
    const char* source,size_t bytes,const doc_paragraph_span* spans,size_t count,doc_shape_context** out)
{
    xui_proxy_caps_t caps;int result=xuiGetProxyCaps(r->context,&caps);
    if(result!=XUI_OK)return result;
    return doc_context_acquire(block,node,source,bytes,spans,count,!!(caps.iCaps & XUI_PROXY_CAP_TEXT_CONTEXT),out);
}
static void doc_layout_variant_free(doc_render_paint_variant* variant)
{
    size_t i;
    if(!variant)return;
    for(i=0;i<variant->span_count;i++){
        free(variant->spans[i].widths);free(variant->spans[i].offsets);
        xuiTextShapeFree(&variant->spans[i].shape);
    }
    free(variant->spans);free(variant->span_map);free(variant->hyphens);
    free(variant->offsets);free(variant->bidi_offsets);free(variant->text);
    xuiInternalTextBidiFree(variant->bidi);free(variant);
}
static void doc_layout_variants_free(doc_render_paint_variant* variants)
{
    while(variants){doc_render_paint_variant* next=variants->next;doc_layout_variant_free(variants);variants=next;}
}
static const doc_render_paint_variant* doc_layout_line_variant(const doc_render_block* block,size_t first,size_t end)
{
    const doc_render_paint_variant* variant;
    for(variant=block->variants;variant;variant=variant->next)
        if(first>=variant->first_fragment && end<=variant->end_fragment)return variant;
    return NULL;
}
void doc_render_block_free(doc_render_block* b)
{
    size_t i;
    for (i = 0; i < b->run_count; i++) {
        xuiTextShapeFree(&b->runs[i].shape);
        free(b->runs[i].text);
    }
    for (i = 0; i < b->paint_group_count; i++) {
        free(b->paint_groups[i].text); free(b->paint_groups[i].display_ends);
        xuiTextShapeFree(&b->paint_groups[i].shape);
    }
    for (i = 0; i < b->paint_seed_count; i++) {
        free(b->paint_seeds[i].widths);free(b->paint_seeds[i].offsets);
        xuiTextShapeFree(&b->paint_seeds[i].shape);
    }
    free(b->runs); free(b->fragments); free(b->lines); free(b->boxes);
    free(b->paint_groups);
    free(b->paint_seeds);
    doc_layout_variants_free(b->variants);b->variants=NULL;
    free(b->visual_fragments);
    for(i=0;i<b->context_count;i++)doc_context_free(b->contexts[i]);
    free(b->contexts);b->contexts=NULL;
    b->context_count=b->context_capacity=b->context_cursor=0;b->layout_context=NULL;
    b->visual_fragments = NULL; b->visual_capacity = 0;
    b->layout_bidi = NULL; b->layout_context = NULL; b->layout_error = XUI_OK;
    if (b->continuation) {
        doc_layout_projection_free(b->continuation);
        free(b->continuation);
    }
    b->runs = NULL; b->fragments = NULL; b->lines = NULL; b->boxes = NULL;
    b->paint_groups = NULL;
    b->paint_seeds = NULL;
    b->continuation = NULL;
    b->run_count = b->run_capacity = b->fragment_count = b->fragment_capacity =
        b->line_count = b->line_capacity = b->box_count = b->box_capacity =
        b->paint_group_count = b->paint_group_capacity = 0;
    b->paint_seed_count = b->paint_seed_capacity = 0;
    b->lines_sorted = 0;
    b->run_cursor = 0;
    b->measured = b->reflow = b->partial = b->line_partial = 0;
    b->partial_bottom = 0;
    b->partial_committed_width = 0;
    b->partial_cutoff = 0;
    b->line_cutoff = 0;
    b->line_exact_width = 0;
    b->shaped_revision = 0;
    b->admonition_font = NULL;
    b->cache_bytes = 0;
}
size_t doc_render_block_cache_bytes(const doc_render_block* b)
{
    size_t i, bytes = b->run_capacity * sizeof(*b->runs) +
        b->fragment_capacity * sizeof(*b->fragments) +
        b->line_capacity * sizeof(*b->lines) + b->box_capacity * sizeof(*b->boxes) +
        b->paint_group_capacity * sizeof(*b->paint_groups);
    bytes += b->paint_seed_capacity * sizeof(*b->paint_seeds);
    bytes += b->visual_capacity * sizeof(*b->visual_fragments);
    for (i = 0; i < b->run_count; i++) {
        bytes += b->runs[i].shape.iPaintBytes;
        if (b->runs[i].text) bytes += (size_t)b->runs[i].bytes + 1;
        if (b->runs[i].shape.pClusters)
            bytes += (size_t)b->runs[i].shape.iClusterCount * sizeof(*b->runs[i].shape.pClusters);
        if (b->runs[i].shape.pCarets)
            bytes += (size_t)b->runs[i].shape.iCaretCount * sizeof(*b->runs[i].shape.pCarets);
    }
    for (i = 0; i < b->paint_group_count; i++) {
        const xui_text_shape_t* shape=&b->paint_groups[i].shape;
        bytes += (size_t)b->paint_groups[i].bytes + 1;
        bytes+=shape->iPaintBytes;
        if(shape->pClusters)bytes+=(size_t)shape->iClusterCount*sizeof(*shape->pClusters);
        if(shape->pCarets)bytes+=(size_t)shape->iCaretCount*sizeof(*shape->pCarets);
        if (b->paint_groups[i].display_ends) bytes +=
            (b->paint_groups[i].end_fragment - b->paint_groups[i].first_fragment) * sizeof(uint32_t);
    }
    for (i = 0; i < b->paint_seed_count; i++) {
        const doc_render_paint_seed* seed=&b->paint_seeds[i];
        size_t count=seed->end_fragment-seed->first_fragment;
        bytes+=count*sizeof(double);
        if(seed->offsets)bytes+=(count+1)*sizeof(*seed->offsets);
        bytes+=seed->shape.iPaintBytes;
        if(seed->shape.pClusters)bytes+=(size_t)seed->shape.iClusterCount*sizeof(*seed->shape.pClusters);
        if(seed->shape.pCarets)bytes+=(size_t)seed->shape.iCaretCount*sizeof(*seed->shape.pCarets);
    }
    {const doc_render_paint_variant* variant;
        for(variant=b->variants;variant;variant=variant->next){
            size_t count=variant->end_fragment-variant->first_fragment;
            bytes+=sizeof(*variant)+(size_t)variant->bytes+1+count*sizeof(*variant->hyphens)+
                (count+1)*sizeof(*variant->offsets)+count*sizeof(*variant->span_map)+
                variant->span_capacity*sizeof(*variant->spans);
            if(variant->bidi_offsets)bytes+=(count+1)*sizeof(*variant->bidi_offsets);
            bytes+=xuiInternalTextBidiRetainedBytes(variant->bidi);
            for(i=0;i<variant->span_count;i++){
                const doc_render_paint_seed* span=&variant->spans[i];const xui_text_shape_t* shape=&span->shape;
                size_t fragments=span->end_fragment-span->first_fragment;
                bytes+=fragments*sizeof(*span->widths)+(fragments+1)*sizeof(*span->offsets)+shape->iPaintBytes;
                if(shape->pClusters)bytes+=(size_t)shape->iClusterCount*sizeof(*shape->pClusters);
                if(shape->pCarets)bytes+=(size_t)shape->iCaretCount*sizeof(*shape->pCarets);
            }
        }
    }
    if (b->continuation) {
        const doc_paragraph_projection* p = b->continuation;
        bytes += sizeof(*p) + (size_t)p->bytes * 2 + 2 +
            (size_t)p->count * sizeof(*p->spans) +
            p->cut_capacity * sizeof(*p->cuts);
        {
            size_t retained = xuiInternalTextBidiRetainedBytes(p->bidi);
            bytes = retained > SIZE_MAX - bytes ? SIZE_MAX : bytes + retained;
        }
    }
    bytes+=b->context_capacity*sizeof(*b->contexts);
    for(i=0;i<b->context_count;i++){
        const doc_shape_context* context=b->contexts[i];
        bytes+=sizeof(*context)+(size_t)context->source_bytes+1;
        if(context->source_map)bytes+=((size_t)context->source_bytes+1)*sizeof(*context->source_map);
        if(context->scripts)bytes+=context->bytes;
        if(context->hyphen_text)bytes+=(size_t)context->bytes+2;
    }
    return bytes;
}
void doc_render_block_reflow(doc_render_block* b)
{
    size_t i;
    for (i = 0; i < b->paint_group_count; i++) {
        free(b->paint_groups[i].text); free(b->paint_groups[i].display_ends);
        xuiTextShapeFree(&b->paint_groups[i].shape);
    }
    free(b->paint_groups);
    doc_layout_variants_free(b->variants);b->variants=NULL;
    b->paint_groups = NULL;
    b->paint_group_count = b->paint_group_capacity = 0;
    free(b->fragments); free(b->lines); free(b->boxes);
    free(b->visual_fragments);
    b->visual_fragments = NULL; b->visual_capacity = 0;
    b->layout_bidi = NULL; b->layout_context = NULL; b->layout_error = XUI_OK;
    b->fragments = NULL; b->lines = NULL; b->boxes = NULL;
    b->fragment_count = b->fragment_capacity = b->line_count = b->line_capacity =
        b->box_count = b->box_capacity = 0;
    b->lines_sorted = 0;
    b->run_cursor = 0;
    b->measured = 0;
    b->reflow = 1;
    b->cache_bytes = doc_render_block_cache_bytes(b);
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
    if (result == XUI_OK) b->boxes[b->box_count++] = (doc_box){.node = node, .kind = kind,
        .color = color, .x = x, .y = y, .width = w, .height = h};
    return result;
}
/* Only a complete logical block owns the blank line after its final forced
 * break. Code slices and paragraph prefixes must not duplicate the next row.
 * The marker keeps source offsets/font metrics and never becomes content. */
static int doc_layout_trailing_line(xui_document_renderer r, doc_render_block* b,
    size_t first)
{
    doc_fragment tail; size_t count = b->fragment_count; int result;
    if (count <= first || !(b->fragments[count - 1].flags & DOC_FORCED_BREAK))
        return XUI_OK;
    tail = b->fragments[count - 1];
    tail.start = tail.end; tail.width = 0;
    tail.bidi_start = tail.bidi_end;
    tail.flags = DOC_LINE_CONTROL | DOC_TRAILING_LINE;
    if (tail.kind == XUI_DOC_HARD_BREAK) {
        doc_node* node = doc_index_get(r->snapshot->state->index, tail.node);
        doc_node* parent = node ? doc_index_get(r->snapshot->state->index, node->parent) : NULL;
        if (!parent) return XUI_DOC_ERROR_STALE;
        tail.node = parent->id; tail.kind = parent->kind;
        tail.start = tail.end = doc_seq_size(parent->children);
    }
    result = doc_layout_fragment(b, &tail);
    if (result == XUI_OK && tail.run != UINT32_MAX)
        b->runs[tail.run].end_fragment = b->fragment_count;
    return result;
}
/* Logical line baselines include the script shift; the physical glyph ascent
 * remains unchanged for painting and font decoration metrics. */
static uint32_t doc_layout_vertical_marks(uint32_t marks)
{
    return marks & XUI_DOC_SUPERSCRIPT ? XUI_DOC_SUPERSCRIPT :
        marks & XUI_DOC_SUBSCRIPT;
}
static double doc_layout_baseline(double ascent, double height, uint32_t marks)
{
    if (marks & XUI_DOC_SUPERSCRIPT) return ascent + height * .3;
    if (marks & XUI_DOC_SUBSCRIPT) return fmax(0, ascent - height * .3);
    return ascent;
}
static int doc_layout_run(xui_document_renderer r, doc_render_block* b, doc_node* n,
    uint32_t heading, const char* generated, uint64_t generated_bytes,
    uint64_t base_offset)
{
    doc_render_run* run; xui_text_shape_t* shape; doc_fragment f = {0};
    uint64_t bytes = generated ? generated_bytes : doc_seq_size(n->text); int result, i;
    if (bytes > INT_MAX || b->run_cursor >= UINT32_MAX) return XUI_DOC_ERROR_LIMIT;
    if (b->reflow) {
        xui_doc_attributes_t attrs = doc_effective_text_attrs(r->snapshot->state, n);
        if (b->source_line) attrs.sLanguage = doc_index_get(r->snapshot->state->index, DOC_ROOT)->attrs->sLanguage;
        xui_font font;
        if (b->run_cursor >= b->run_count) return XUI_DOC_ERROR_STALE;
        run = &b->runs[b->run_cursor];
        if (run->node != n->id || run->bytes != bytes ||
            run->base_offset != base_offset) return XUI_DOC_ERROR_STALE;
        if (n->kind == XUI_DOC_CODE_BLOCK || n->kind == XUI_DOC_DIAGRAM || n->kind == XUI_DOC_FRONT_MATTER)
            attrs.iMarks |= XUI_DOC_CODE;
        font = doc_layout_font(r, &attrs, heading);
        if (!font) return XUI_ERROR_UNSUPPORTED;
        if (font != run->font || !doc_language_equal(attrs.sLanguage, run->attrs.sLanguage)) return XUI_DOC_ERROR_STALE;
        run->attrs = attrs;
    } else {
        result = doc_render_reserve((void**)&b->runs, &b->run_capacity, b->run_count + 1, sizeof(*b->runs));
        if (result != XUI_OK) return result;
        run = &b->runs[b->run_count]; memset(run, 0, sizeof(*run));
        run->node = n->id; run->attrs = doc_effective_text_attrs(r->snapshot->state, n);
        if (b->source_line) run->attrs.sLanguage = doc_index_get(r->snapshot->state->index, DOC_ROOT)->attrs->sLanguage;
        run->bytes = bytes;
        run->base_offset = base_offset;
        if (n->kind == XUI_DOC_CODE_BLOCK || n->kind == XUI_DOC_DIAGRAM || n->kind == XUI_DOC_FRONT_MATTER) run->attrs.iMarks |= XUI_DOC_CODE;
        run->font = doc_layout_font(r, &run->attrs, heading);
        if (!run->font) return XUI_ERROR_UNSUPPORTED;
        run->text = malloc((size_t)bytes + 1); if (!run->text) return XUI_ERROR_OUT_OF_MEMORY;
        if (generated) memcpy(run->text, generated, (size_t)bytes); else doc_seq_read(n->text, 0, run->text, bytes);
        run->text[bytes] = 0;
        b->run_count++;
        result = xuiInternalTextShapeProjection(r->context, run->font, run->text, (int)bytes, run->attrs.sLanguage, &run->shape);
        if (result != XUI_OK) return result;
        r->stats.iShapedBytes += bytes;
        r->stats.iTextRunBytes += bytes;
    }
    f.run = (uint32_t)b->run_cursor++; f.node = n->id; f.kind = n->kind;
    run->first_fragment = b->fragment_count;
    shape = &run->shape;
    result = XUI_OK;
    if (n->kind == XUI_DOC_SOFT_BREAK || n->kind == XUI_DOC_HARD_BREAK) {
        /* Shape one space for the break's effective inline font. Keep the
         * public atom at offset zero: the measuring text is private and does
         * not become editable content or a grapheme in the projection. */
        f.break_style_run = f.run; f.run = UINT32_MAX;
        f.height = shape->fLineHeight > 0 ? shape->fLineHeight : 20 * r->desc.fZoom;
        f.baseline = doc_layout_baseline(shape->fAscent > 0 ? shape->fAscent : f.height * .8,
            f.height, run->attrs.iMarks);
        f.width = n->kind == XUI_DOC_SOFT_BREAK ? shape->fWidth : 0;
        f.flags = n->kind == XUI_DOC_HARD_BREAK ? DOC_FORCED_BREAK : XUI_TEXT_CLUSTER_LINE_BREAK;
        if (!isfinite(f.width) || f.width < 0) return XUI_ERROR_INVALID_ARGUMENT;
        result = doc_layout_fragment(b, &f);
        run->end_fragment = b->fragment_count;
        return result;
    }
    if (!shape->iClusterCount) {
        f.start = f.end = base_offset;
        f.height = shape->fLineHeight > 0 ? shape->fLineHeight : 20 * r->desc.fZoom;
        f.baseline = shape->fAscent > 0 ? shape->fAscent : f.height * .8;
        f.baseline = doc_layout_baseline(f.baseline, f.height, run->attrs.iMarks);
        result = doc_layout_fragment(b, &f);
    }
    for (i = 0; i < shape->iClusterCount && result == XUI_OK; i++) {
        xui_text_cluster_t* cluster = &shape->pClusters[i];
        f.start = base_offset + (uint64_t)cluster->iTextStart;
        f.end = base_offset + (uint64_t)cluster->iTextEnd;
        f.width = cluster->fAdvance; f.height = shape->fLineHeight; f.baseline = shape->fAscent;
        f.baseline = doc_layout_baseline(f.baseline, f.height, run->attrs.iMarks);
        f.flags = cluster->iFlags;
        f.hyphen_width = 0;
        if (cluster->iTextEnd - cluster->iTextStart == 2 &&
            !memcmp(run->text + cluster->iTextStart, "\xc2\xad", 2)) {
            if (!run->hyphen_measured) {
                xui_text_shape_t hyphen = {0};
                result = xuiTextShape(r->context, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=run->font, .sText="-", .iTextSize=1, .sLanguage=run->attrs.sLanguage, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &hyphen);
                if (result == XUI_OK) {
                    run->hyphen_width = hyphen.fWidth;
                    if (!isfinite(run->hyphen_width) || run->hyphen_width < 0) result = XUI_ERROR_INVALID_ARGUMENT;
                    if (result == XUI_OK) run->hyphen_measured = 1;
                    r->stats.iShapedBytes++;
                }
                xuiTextShapeFree(&hyphen);
                if (result != XUI_OK) return result;
            }
            f.hyphen_width = run->hyphen_width;
        }
        if (cluster->iTextStart < (int)bytes &&
            (run->text[cluster->iTextStart] == '\n' || run->text[cluster->iTextStart] == '\r')) {
            /* Some shapers emit CR and LF separately. A CR immediately
             * followed by LF in this run belongs to the LF break. A CRLF
             * split across runs is resolved by the paragraph projection. */
            if (!(run->text[cluster->iTextStart] == '\r' &&
                cluster->iTextEnd == cluster->iTextStart + 1 &&
                cluster->iTextEnd < (int)bytes && run->text[cluster->iTextEnd] == '\n'))
                f.flags |= DOC_FORCED_BREAK;
            f.width = 0;
        }
        result = doc_layout_fragment(b, &f);
    }
    run->end_fragment = b->fragment_count;
    return result;
}
static int doc_layout_inline_is_run(uint32_t kind)
{
    return kind == XUI_DOC_TEXT || kind == XUI_DOC_FOOTNOTE_REF ||
        kind == XUI_DOC_CODE_BLOCK || kind == XUI_DOC_FRONT_MATTER;
}
static int doc_layout_inline(xui_document_renderer r, doc_render_block* b, doc_node* n, uint32_t heading, double width)
{
    doc_fragment f = {0};
    if (doc_layout_inline_is_run(n->kind)) return doc_layout_run(r, b, n, heading, NULL, 0, 0);
    if (n->kind == XUI_DOC_SOFT_BREAK || n->kind == XUI_DOC_HARD_BREAK)
        return doc_layout_run(r, b, n, heading, " ", 1, 0);
    f.node = n->id; f.kind = n->kind; f.run = UINT32_MAX;
    f.height = 20 * r->desc.fZoom; f.baseline = f.height * .8;
    {
        xui_vec2_t size = {0}; float baseline = 0;
        int result = r->desc.onObjectMeasure ? r->desc.onObjectMeasure(r->snapshot, n->id, (float)width, r->desc.fZoom, &size, &baseline, r->desc.pUser) : XUI_ERROR_UNSUPPORTED;
        if (result == XUI_ERROR_UNSUPPORTED && n->kind == XUI_DOC_IMAGE)
            result = doc_render_image_measure(r, n, width, &size);
        if (result != XUI_OK && result != XUI_ERROR_UNSUPPORTED) return result;
        f.width = result == XUI_OK ? size.fX : n->attrs->fWidth > 0 ? n->attrs->fWidth * r->desc.fZoom : 96 * r->desc.fZoom;
        f.height = result == XUI_OK ? size.fY : n->attrs->fHeight > 0 ? n->attrs->fHeight * r->desc.fZoom : 32 * r->desc.fZoom;
        f.baseline = baseline > 0 ? baseline : f.height;
        if (!isfinite(f.width) || !isfinite(f.height) || f.width < 0 || f.height < 0) return XUI_ERROR_INVALID_ARGUMENT;
        f.end = doc_seq_size(n->text);
        b->object_dependent = 1;
    }
    return doc_layout_fragment(b, &f);
}
static int doc_layout_ascii_word(unsigned char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9');
}
static int doc_layout_emergency_cut(const char* text, uint64_t bytes,
    uint64_t at)
{
    int previous;
    if (!at || at >= bytes) return 0;
    if (!doc_layout_ascii_word((unsigned char)text[at])) return 0;
    previous = xuiInternalTextGraphemePrev(text, (int)bytes, (int)at);
    /* Emergency cuts inside joining scripts can change glyph shaping.
     * Only ASCII-based graphemes are segmented until full paragraph shaping
     * can preserve joining context across a continuation boundary. */
    return previous >= 0 && previous < (int)at &&
        doc_layout_ascii_word((unsigned char)text[previous]) &&
        doc_layout_ascii_word((unsigned char)text[at]);
}
/* Build the paragraph's line-break stream before shaping any style run. Each
 * child keeps its source span for future continuation planning. Cuts keep a
 * grapheme intact; paragraph-level Bidi shaping remains separate work. */
static int doc_layout_projection_build(xui_document_renderer r, doc_node* paragraph,
    doc_paragraph_projection* projection)
{
    uint64_t i, bytes = 0, count = doc_seq_size(paragraph->children);
    int result;
    if (count > SIZE_MAX / sizeof(*projection->spans)) return XUI_DOC_ERROR_LIMIT;
    if (count) {
        projection->spans = calloc((size_t)count, sizeof(*projection->spans));
        if (!projection->spans) return XUI_ERROR_OUT_OF_MEMORY;
    }
    projection->count = count;
    for (i = 0; i < count; i++) {
        doc_node* child = doc_index_get(r->snapshot->state->index,
            doc_seq_get_id(paragraph->children, i));
        uint64_t length;
        if (!child) return XUI_DOC_ERROR_SCHEMA;
        length = doc_layout_inline_is_run(child->kind) ? doc_seq_size(child->text) : 3;
        if (length > INT_MAX - bytes) return XUI_DOC_ERROR_LIMIT;
        projection->spans[i] = (doc_paragraph_span){.node = child->id,
            .start = bytes, .end = bytes + length, .kind=child->kind};
        bytes += length;
    }
    projection->bytes = bytes;
    projection->text = malloc((size_t)bytes + 1);
    projection->breaks = calloc((size_t)bytes + 1, 1);
    if (!projection->text || !projection->breaks) return XUI_ERROR_OUT_OF_MEMORY;
    for (i = 0; i < count; i++) {
        doc_paragraph_span* span = &projection->spans[i];
        doc_node* child = doc_index_get(r->snapshot->state->index, span->node);
        if (doc_layout_inline_is_run(child->kind)) {
            result = doc_seq_read(child->text, 0,
                projection->text + span->start, span->end - span->start);
            if (result != XUI_OK) return result;
        } else if (child->kind == XUI_DOC_SOFT_BREAK)
            memcpy(projection->text + span->start, "   ", 3);
        else if (child->kind == XUI_DOC_HARD_BREAK)
            memcpy(projection->text + span->start, "\n\n\n", 3);
        else memcpy(projection->text + span->start, "\xef\xbf\xbc", 3);
    }
    projection->text[bytes] = 0;
    if (xuiInternalTextBidiNeedsAnalysis(projection->text, (size_t)bytes)) {
        char* bidi_text = malloc((size_t)bytes + 1);
        if (!bidi_text) return XUI_ERROR_OUT_OF_MEMORY;
        memcpy(bidi_text, projection->text, (size_t)bytes + 1);
        /* A semantic HardBreak is a line separator inside this paragraph.
         * Keep its three-byte source span, and preserve paragraph direction. */
        for (i = 0; i < count; i++) {
            doc_node* child = doc_index_get(r->snapshot->state->index, projection->spans[i].node);
            if (child->kind == XUI_DOC_HARD_BREAK)
                memcpy(bidi_text + projection->spans[i].start, "\xe2\x80\xa8", 3);
        }
        result = xuiInternalTextBidiCreate(bidi_text, (size_t)bytes,
            XUI_BIDI_AUTO_LTR, &projection->bidi);
        free(bidi_text);
        if (result != XUI_OK) return result;
    }
    result = xuiInternalTextBreakMap(projection->text, (int)bytes,
        projection->breaks);
    if (result == XUI_OK) result = xuiInternalTextDisplayGraphemes(projection->text,
        (int)bytes, projection->breaks);
    if (result != XUI_OK) return result;
    if (bytes >= 32768) {
        uint64_t target;
        for (target = 8192; target < bytes; target += 8192) {
            uint64_t scan, normal_limit = bytes - target < 256 ? bytes : target + 256;
            if (projection->cut_count &&
                projection->cuts[projection->cut_count - 1] >= target) continue;
            /* Prefer a nearby word/hard boundary to preserve run shaping.
             * A long unbroken word may otherwise force full-block shaping;
             * Unicode emergency boundaries still keep a grapheme intact. */
            for (scan = target; scan < normal_limit; scan++)
                if ((projection->breaks[scan] & XUI_LB_GRAPHEME) &&
                    (projection->breaks[scan] & (XUI_LB_NORMAL | XUI_LB_HARD)))
                    break;
            if (scan == normal_limit)
                for (scan = target; scan < bytes; scan++)
                    if ((projection->breaks[scan] & XUI_LB_GRAPHEME) &&
                        ((projection->breaks[scan] & (XUI_LB_NORMAL | XUI_LB_HARD)) ||
                         ((projection->breaks[scan] & XUI_LB_EMERGENCY) &&
                          doc_layout_emergency_cut(projection->text, bytes, scan))))
                        break;
            if (scan >= bytes) break;
            if (projection->cut_count &&
                projection->cuts[projection->cut_count - 1] >= scan) continue;
            result = doc_render_reserve((void**)&projection->cuts,
                &projection->cut_capacity, projection->cut_count + 1,
                sizeof(*projection->cuts));
            if (result != XUI_OK) return result;
            projection->cuts[projection->cut_count++] = scan;
        }
    }
    return XUI_OK;
}
static void doc_layout_control_flags(doc_render_block* block, doc_fragment* fragment,
    const unsigned char* map, uint64_t begin, uint64_t end)
{
    const doc_render_run* run; uint64_t bytes; int control_start, control_end;
    if (fragment->run == UINT32_MAX || begin == end) return;
    run = &block->runs[fragment->run]; bytes = fragment->end - fragment->start;
    if (bytes <= INT_MAX && xuiInternalTextCopyDisplay(
        run->text + fragment->start - run->base_offset, (int)bytes, NULL) < (int)bytes) {
        fragment->flags |= DOC_FORMATTED;
        if (!xuiInternalTextCopyDisplay(run->text + fragment->start - run->base_offset, (int)bytes, NULL)) {
            fragment->flags |= DOC_INVISIBLE; fragment->width = 0;
            if (map[end] & XUI_LB_SOFT_HYPHEN) fragment->flags |= DOC_SOFT_HYPHEN;
        }
    }
    if (!(map[begin] & XUI_LB_HARD)) return;
    if (bytes > INT_MAX || xuiInternalTextNextHardLine(run->text + fragment->start - run->base_offset,
        (int)bytes, 0, &control_start, &control_end) != XUI_OK || control_start ||
        control_end != (int)bytes) return;
    fragment->flags |= DOC_LINE_CONTROL;
    fragment->width = 0;
    fragment->flags &= ~DOC_FORCED_BREAK;
    if (map[end] & XUI_LB_HARD_END) fragment->flags |= DOC_FORCED_BREAK;
}
static int doc_layout_projection_apply(doc_render_block* block,
    const doc_paragraph_projection* projection)
{
    uint64_t i;
    int previous_join = 0;
    for (i = 0; i < projection->count; i++) {
        const doc_paragraph_span* span = &projection->spans[i];
        size_t j;
        for (j = span->first_fragment; j < span->end_fragment; j++) {
            doc_fragment* fragment = &block->fragments[j];
            uint64_t at = span->start + (fragment->run == UINT32_MAX ? 3 : fragment->end);
            if (at > span->end) return XUI_DOC_ERROR_SCHEMA;
            doc_layout_control_flags(block, fragment, projection->breaks,
                span->start + fragment->start, at);
            /* A shaper can emit CR and LF as separate clusters, including
             * when a style boundary falls between them. The paragraph stream
             * still treats CRLF as one mandatory break. */
            if (fragment->run != UINT32_MAX && fragment->end == fragment->start + 1 &&
                at < projection->bytes && projection->text[at - 1] == '\r' &&
                projection->text[at] == '\n')
                fragment->flags &= ~DOC_FORCED_BREAK;
            fragment->flags &= ~(DOC_NORMAL_BREAK | DOC_EMERGENCY_BREAK |
                DOC_GRAPHEME_JOIN | DOC_GRAPHEME_MEMBER);
            /* The break map stores boundaries after bytes; zero has no entry
             * and is always the paragraph's initial grapheme boundary. */
            if (fragment->run != UINT32_MAX && at > 0 && at < projection->bytes &&
                !(projection->breaks[at] & XUI_LB_GRAPHEME))
                fragment->flags |= DOC_GRAPHEME_JOIN;
            if (previous_join || (fragment->flags & DOC_GRAPHEME_JOIN))
                fragment->flags |= DOC_GRAPHEME_MEMBER;
            previous_join = !!(fragment->flags & DOC_GRAPHEME_JOIN);
            if (projection->breaks[at] & XUI_LB_NORMAL) fragment->flags |= DOC_NORMAL_BREAK;
            if (projection->breaks[at] & XUI_LB_EMERGENCY) fragment->flags |= DOC_EMERGENCY_BREAK;
            fragment->bidi_start = (size_t)(span->start + fragment->start);
            fragment->bidi_end = (size_t)at;
            fragment->bidi_active = projection->bidi != NULL;
            fragment->bidi_level = 0;
            if (projection->bidi && fragment->bidi_start < projection->bytes) {
                int result = xuiInternalTextBidiLevel(projection->bidi,
                    fragment->bidi_start, &fragment->bidi_level);
                if (result != XUI_OK) return result;
            }
        }
    }
    return XUI_OK;
}
/* L1 is line-specific. Query it before shaping a candidate, so trailing
 * whitespace uses the same direction in measurement, painting and geometry.
 * A grapheme spanning source styles always takes its head's direction. */
static int doc_layout_bidi_levels(const doc_render_block* block, size_t first, size_t end, size_t hyphen, const doc_render_paint_variant* variant)
{
    xui_bidi_line_t line = {0}; size_t i;
    xui_text_bidi bidi=variant?variant->bidi:block->layout_bidi;
    size_t start,stop;
    int result;
    if (!bidi || first == end) return XUI_OK;
    start=variant?variant->bidi_offsets[first-variant->first_fragment]:block->fragments[first].bidi_start;
    stop=variant?variant->bidi_offsets[end-variant->first_fragment]:block->fragments[end-1].bidi_end;
    result = xuiInternalTextBidiLine(bidi,start,stop,&line);
    if (result != XUI_OK) return result;
    if(!variant && hyphen>=first && hyphen<end){
        result=xuiInternalTextBidiRestoreHyphenLevels(block->layout_bidi,line.start,
            block->fragments[hyphen].bidi_start,line.levels);
        if(result!=XUI_OK){xuiInternalTextBidiLineFree(&line);return result;}
    }
    for (i = first; i < end; i++) {
        doc_fragment* fragment = &block->fragments[i];
        size_t offset=variant?variant->bidi_offsets[i-variant->first_fragment]:fragment->bidi_start;
        fragment->bidi_base = line.base;
        if (i > first && (block->fragments[i - 1].flags & DOC_GRAPHEME_JOIN)) {
            fragment->bidi_level = block->fragments[i - 1].bidi_level;
            continue;
        }
        if (offset < line.end)
            fragment->bidi_level = line.levels[offset - line.start];
        else if (line.count) fragment->bidi_level = line.levels[line.end - line.start - 1];
    }
    /* A displayed discretionary hyphen belongs to the preceding word's
     * directional item, rather than the invisible BN reset by L1. */
    if (!variant && hyphen > first && hyphen < end)
        block->fragments[hyphen].bidi_level = block->fragments[hyphen - 1].bidi_level;
    xuiInternalTextBidiLineFree(&line); return XUI_OK;
}
static int doc_layout_run_slice(xui_document_renderer r, doc_render_block* block,
    doc_node* child, uint32_t heading, uint64_t start, uint64_t bytes)
{
    uint64_t total = doc_seq_size(child->text);
    char* text; int result;
    if (start > total || bytes > total - start || bytes > INT_MAX)
        return XUI_DOC_ERROR_LIMIT;
    if (block->reflow) {
        if (block->run_cursor >= block->run_count) return XUI_DOC_ERROR_STALE;
        /* doc_layout_run validates the exact node/range/font before reusing
         * this text; width changes need no source copy or new shaping. */
        return doc_layout_run(r, block, child, heading,
            block->runs[block->run_cursor].text, bytes, start);
    }
    if (!start && bytes == total)
        return doc_layout_run(r, block, child, heading, NULL, 0, 0);
    text = malloc((size_t)bytes + 1);
    if (!text) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_read(child->text, start, text, bytes);
    if (result == XUI_OK) {
        text[bytes] = 0;
        result = doc_layout_run(r, block, child, heading, text, bytes, start);
    }
    free(text);
    return result;
}
/* A paragraph's shaping cuts are derived once from its full Unicode break
 * stream. Prefix and complete layouts use the same cuts, so run-wide shaper
 * metrics cannot change the geometry of an already committed segment. */
static int doc_layout_paragraph_inlines(xui_document_renderer r,
    doc_render_block* block, doc_node* paragraph,
    doc_paragraph_projection* projection, uint64_t start,
    uint64_t cutoff, double width)
{
    uint64_t i;
    size_t cut_index = 0;
    uint32_t heading = paragraph->kind == XUI_DOC_HEADING ?
        paragraph->attrs->iHeadingLevel : 0;
    for (i = 0; i < projection->count; i++) {
        doc_paragraph_span* span = &projection->spans[i];
        doc_node* child; int result;
        span->first_fragment = span->end_fragment = 0;
        if (span->end <= start && span->start < start) continue;
        if (cutoff < projection->bytes && span->start >= cutoff) break;
        child = doc_index_get(r->snapshot->state->index, span->node);
        if (!child) return XUI_DOC_ERROR_SCHEMA;
        span->first_fragment = block->fragment_count;
        if (doc_layout_inline_is_run(child->kind)) {
            uint64_t at = span->start > start ? span->start : start;
            uint64_t end = span->end < cutoff ? span->end : cutoff;
            if (at == end)
                result = doc_layout_run_slice(r, block, child, heading,
                    at - span->start, 0);
            else result = XUI_OK;
            while (result == XUI_OK && at < end) {
                uint64_t next = end;
                while (cut_index < projection->cut_count &&
                    projection->cuts[cut_index] <= at) cut_index++;
                if (cut_index < projection->cut_count &&
                    projection->cuts[cut_index] < next)
                    next = projection->cuts[cut_index];
                result = doc_layout_run_slice(r, block, child, heading,
                    at - span->start, next - at);
                at = next;
            }
        } else if (span->end <= cutoff)
            result = doc_layout_inline(r, block, child, heading, width);
        else result = XUI_DOC_ERROR_SCHEMA;
        if (result != XUI_OK) return result;
        span->end_fragment = block->fragment_count;
    }
    return XUI_OK;
}
static int doc_layout_paintable(const doc_render_block*, const doc_fragment*);
static uint64_t doc_layout_display_bytes(const doc_render_block*, const doc_fragment*);
typedef struct doc_line_plan_context {
    xui_document_renderer renderer;
    const doc_render_block* block;
    double* scratch;
    size_t base;
    int result;
    size_t end;
    const size_t* shared;
    const uint8_t* selected;
    const uint8_t* forbidden;
    doc_render_paint_variant* variant;
} doc_line_plan_context;
static int doc_layout_variant_probe(doc_line_plan_context*,size_t,size_t,double*,doc_render_paint_variant**);
static int doc_layout_hyphen_fits(doc_line_plan_context*, size_t, size_t,
    const double*, size_t, double);
static int doc_layout_hyphen_prefix(const doc_render_block* block,size_t first,size_t hyphen)
{
    size_t at;
    for(at=first;at<hyphen;at++)if(doc_layout_display_bytes(block,&block->fragments[at]))return 1;
    return 0;
}
static size_t doc_layout_line_hyphen(const doc_render_block* block,size_t first,size_t end,size_t paragraph_end)
{
    return end<paragraph_end && end>first && (block->fragments[end-1].flags & DOC_SOFT_HYPHEN) &&
        doc_layout_hyphen_prefix(block,first,end-1)?end-1:SIZE_MAX;
}
/* A terminal hyphen may change the preceding glyph advances enough to fit
 * even when the unbroken prefix overflows. Look through deletion gaps and
 * empty styled text, without crossing another visible glyph or control. */
static size_t doc_layout_hyphen_after(doc_line_plan_context* context,
    const doc_fragment* fragments, size_t first, size_t at, size_t end,
    const double* widths, size_t width_base, double width)
{
    size_t next, candidate = first;
    if (!context || !doc_layout_paintable(context->block, &fragments[at])) return first;
    for (next = at + 1; next < end; next++) {
        const doc_fragment* fragment = &fragments[next];
        if (!doc_layout_paintable(context->block, fragment) ||
            doc_layout_display_bytes(context->block, fragment)) break;
        if (next + 1 < end &&
            (fragment->flags & (DOC_NORMAL_BREAK | DOC_SOFT_HYPHEN)) ==
                (DOC_NORMAL_BREAK | DOC_SOFT_HYPHEN) &&
            !(fragment->flags & DOC_GRAPHEME_JOIN) &&
            doc_layout_hyphen_fits(context, first, next + 1, widths, width_base, width))
            candidate = next + 1;
        if (context->result != XUI_OK) break;
    }
    return candidate;
}
static size_t doc_layout_line_stop(const doc_render_block* block,
    size_t first, size_t end, double width, int wrap,
    const double* widths, size_t width_base, doc_line_plan_context* context)
{
    const doc_fragment* fragments=block->fragments;
    size_t at = first, stop, last_break = first, emergency = first;
    double advance = 0;
    /* Exact line strings have already been shaped. Reborrowing advances from
     * the next line can recreate a context-dependent cut cycle. */
    if (wrap && !widths && (fragments[first].flags & DOC_CONTEXT_LAYOUT)) {
        for (stop = first; stop < end; stop++)
            if (fragments[stop].flags & DOC_CONTEXT_LINE_END) return stop + 1;
        return end;
    }
    while (at < end) {
        const doc_fragment* fragment = &fragments[at];
        double fragment_width = fragment->flags & DOC_SOFT_HYPHEN ? 0 :
            widths ? widths[at - width_base] : fragment->width;
        double hyphen = widths && widths[at - width_base] > 0 ?
            widths[at - width_base] : fragment->hyphen_width;
        if (wrap && at > first && advance + fragment_width > width &&
            (last_break > first || emergency > first)) {
            size_t candidate = doc_layout_hyphen_after(context, fragments,
                first, at, end, widths, width_base, width);
            if (candidate > first) return candidate;
            break;
        }
        advance += fragment_width; at++;
        if (!(fragment->flags & DOC_GRAPHEME_JOIN)) {
            if ((fragment->flags & DOC_NORMAL_BREAK) &&
                (!(fragment->flags & DOC_SOFT_HYPHEN) ||
                 (doc_layout_hyphen_prefix(block,first,at-1) && (context || advance + hyphen <= width)))) last_break = at;
            if ((fragment->flags & DOC_EMERGENCY_BREAK) &&
                (!context || !(fragment->flags & DOC_SOFT_HYPHEN))) emergency = at;
        }
        if (fragment->flags & DOC_FORCED_BREAK) break;
        if (wrap && advance > width &&
            (last_break > first || emergency > first)) {
            size_t candidate = doc_layout_hyphen_after(context, fragments,
                first, at - 1, end, widths, width_base, width);
            if (candidate > first) return candidate;
            break;
        }
    }
    stop = at;
    if (wrap && at < end && at > first &&
        !(fragments[at - 1].flags & DOC_FORCED_BREAK)) {
        /* Evaluate only candidates that can actually end this line. Search
         * backward on rejection so an earlier valid break is retained. */
        while (context && last_break > first &&
            (fragments[last_break - 1].flags & DOC_SOFT_HYPHEN) &&
            !doc_layout_hyphen_fits(context, first, last_break, widths, width_base, width)) {
            size_t previous = last_break - 1;
            if (context->result != XUI_OK) return first + 1;
            last_break = first;
            while (previous > first) {
                const doc_fragment* fragment = &fragments[previous - 1];
                if ((fragment->flags & DOC_NORMAL_BREAK) &&
                    !(fragment->flags & DOC_GRAPHEME_JOIN)) { last_break = previous; break; }
                previous--;
            }
        }
        if (last_break > first) stop = last_break;
        else if (emergency > first) stop = emergency;
        else if(stop>first+1 && (fragments[stop-1].flags & DOC_SOFT_HYPHEN) &&
            !(fragments[stop-2].flags & DOC_GRAPHEME_JOIN))stop--;
    }
    return stop == first ? first + 1 : stop;
}
static size_t doc_layout_plan_lines(const doc_render_block* block,
    size_t first, size_t end, double width, const double* widths,
    size_t* cuts, size_t capacity, doc_line_plan_context* context)
{
    size_t count = 0, at = first;
    while (at < end) {
        if (count == capacity) return 0;
        at = doc_layout_line_stop(block, at, end, width, 1,
            widths, first, context);
        if (context && context->result != XUI_OK) return 0;
        cuts[count++] = at;
    }
    return count;
}
static void doc_layout_pending_paint_free(doc_render_paint_group* groups,
    size_t count)
{
    size_t i;
    if (!groups) return;
    for (i = 0; i < count; i++) {
        free(groups[i].text); free(groups[i].display_ends);xuiTextShapeFree(&groups[i].shape);
    }
    free(groups);
}
/* A full-line shaper can merge an emoji sequence that was split into several
 * Text nodes, or split a run's original cluster. Account for every advance
 * once at its ending fragment. Only Unicode grapheme interiors may cross an
 * original fragment boundary here. The caller has already subdivided merged
 * multi-grapheme clusters at provider or conventional caret stops. */
static int doc_layout_shape_widths(const doc_render_block* block,
    size_t first, size_t end, const xui_text_shape_t* shape,
    double* widths, size_t width_base, const uint32_t* display_ends)
{
    size_t fragment = first;
    uint64_t boundary, bytes = 0;
    int i, previous_end = 0;
    if (first == end) return 0;
    for (fragment = first; fragment < end; fragment++) {
        bytes += display_ends ? display_ends[fragment - first] -
            (fragment == first ? 0 : display_ends[fragment - first - 1]) : doc_fragment_paint_bytes(&block->fragments[fragment]);
        widths[fragment - width_base] = 0;
    }
    if (!bytes) return shape->iClusterCount == 0;
    if (shape->iClusterCount <= 0 || !shape->pClusters) return 0;
    fragment = first;
    boundary = display_ends ? display_ends[0] : doc_fragment_paint_bytes(&block->fragments[fragment]);
    for (i = 0; i < shape->iClusterCount; i++) {
        const xui_text_cluster_t* cluster = &shape->pClusters[i];
        if (cluster->iTextStart != previous_end ||
            cluster->iTextEnd <= cluster->iTextStart ||
            (uint64_t)cluster->iTextEnd > bytes ||
            !isfinite(cluster->fAdvance) || cluster->fAdvance < 0) return 0;
        while (boundary <= (uint64_t)cluster->iTextStart && fragment + 1 < end) {
            fragment++;
            boundary = display_ends ? display_ends[fragment - first] :
                boundary + doc_fragment_paint_bytes(&block->fragments[fragment]);
        }
        while (boundary < (uint64_t)cluster->iTextEnd && fragment + 1 < end) {
            if (!(block->fragments[fragment].flags & DOC_GRAPHEME_JOIN)) return 0;
            fragment++;
            boundary = display_ends ? display_ends[fragment - first] :
                boundary + doc_fragment_paint_bytes(&block->fragments[fragment]);
        }
        if (boundary < (uint64_t)cluster->iTextEnd) return 0;
        widths[fragment - width_base] += cluster->fAdvance;
        previous_end = cluster->iTextEnd;
    }
    return (uint64_t)previous_end == bytes;
}
int doc_render_fragment_text(const doc_render_block* block,
    const doc_fragment* fragment)
{
    const doc_render_run* run; const char* text; uint64_t bytes, i;
    if (fragment->run == UINT32_MAX || fragment->run >= block->run_count) return 0;
    if (fragment->flags & DOC_LINE_CONTROL) return 0;
    if ((fragment->flags & DOC_INVISIBLE) && !(fragment->flags & DOC_HYPHEN_USED)) return 0;
    run = &block->runs[fragment->run];
    if (fragment->start < run->base_offset || fragment->end < fragment->start ||
        fragment->end - run->base_offset > run->bytes) return 0;
    text = run->text + fragment->start - run->base_offset;
    bytes = fragment->end - fragment->start;
    if (!bytes || (*text != '\r' && *text != '\n')) return 1;
    for (i = 0; i < bytes; i++) if (text[i] != '\r' && text[i] != '\n') return 1;
    return 0;
}
static int doc_layout_paintable(const doc_render_block* block, const doc_fragment* fragment)
{
    return fragment->kind == XUI_DOC_SOFT_BREAK ||
        (fragment->flags & DOC_INVISIBLE) || doc_render_fragment_text(block, fragment);
}
static uint64_t doc_layout_display_bytes(const doc_render_block* block, const doc_fragment* fragment)
{
    uint64_t bytes = doc_fragment_paint_bytes(fragment);
    if (fragment->flags & DOC_FORMATTED) {
        const doc_render_run* run = &block->runs[doc_fragment_paint_run(fragment)];
        return (uint64_t)xuiInternalTextCopyDisplay(run->text + fragment->start - run->base_offset, (int)bytes, NULL);
    }
    return bytes;
}
/* A grapheme uses its first visible fragment's resolved font and physical
 * baseline, even when its remaining scalars have different source styles.
 * Those styles take effect at the next grapheme. Empty carriers choose no
 * glyph style. Color and decoration remain paint attributes. */
static size_t doc_layout_shape_stop(const doc_render_block* block, size_t first,
    size_t end, size_t hyphen, const doc_render_paint_variant* variant)
{
    xui_font font = NULL; uint32_t vertical_marks = 0;
    uint32_t script=0;
    const char* language = NULL;
    size_t at = first;
    while (at < end) {
        const doc_fragment* fragment = &block->fragments[at];
        if (!doc_layout_paintable(block, fragment)) break;
        if (at > first && !(block->fragments[at - 1].flags & DOC_GRAPHEME_JOIN) &&
            fragment->bidi_level != block->fragments[first].bidi_level) break;
        int inserted=at==hyphen || (variant && variant->hyphens[at-variant->first_fragment]);
        if (doc_layout_display_bytes(block, fragment) || inserted) {
            const doc_render_run* run = &block->runs[doc_fragment_paint_run(fragment)];
            uint32_t next_marks = doc_layout_vertical_marks(run->attrs.iMarks);
            int joined = at > first && (block->fragments[at - 1].flags & DOC_GRAPHEME_JOIN);
            uint32_t next_script=doc_context_script(block->layout_context,fragment->bidi_start,inserted);
            if (font && !joined && (run->font != font || next_marks != vertical_marks)) break;
            if(font && !joined && next_script!=script)break;
            if(font && !joined && !doc_language_equal(language, run->attrs.sLanguage)) break;
            /* Use the paragraph's resolved Script_Extensions, including
             * shared characters and paired brackets across style boundaries. */
            if (!font) { font = run->font; vertical_marks = next_marks;script=next_script; language=run->attrs.sLanguage; }
        }
        at++;
    }
    return at;
}
static int doc_layout_paint_input(xui_document_renderer r,
    const doc_render_block* block, size_t first, size_t end,
    doc_render_paint_group* group, size_t hyphen, doc_render_paint_variant* variant)
{
    size_t bytes = 0, at = 0, i; int result, projected = 0;
    for (i = first; i < end; i++) {
        size_t length = variant?variant->offsets[i+1-variant->first_fragment]-variant->offsets[i-variant->first_fragment]:
            i == hyphen ? 1 : (size_t)doc_layout_display_bytes(block, &block->fragments[i]);
        if (length > (size_t)INT_MAX - bytes) return XUI_DOC_ERROR_LIMIT;
        bytes += length;
        if (block->fragments[i].flags & DOC_FORMATTED) projected = 1;
    }
    group->text = malloc(bytes + 1);
    if (!group->text) return XUI_ERROR_OUT_OF_MEMORY;
    if (projected || variant) {
        group->display_ends = calloc(end - first, sizeof(*group->display_ends));
        if (!group->display_ends) return XUI_ERROR_OUT_OF_MEMORY;
    }
    group->origin_fragment = first;
    group->font = block->runs[doc_fragment_paint_run(&block->fragments[first])].font;
    group->vertical_marks = doc_layout_vertical_marks(
        block->runs[doc_fragment_paint_run(&block->fragments[first])].attrs.iMarks);
    for (i = first; i < end; i++) {
        const doc_fragment* fragment = &block->fragments[i];
        const doc_render_run* run = &block->runs[doc_fragment_paint_run(fragment)];
        size_t length = (size_t)doc_layout_display_bytes(block, fragment);
        if(variant)length=variant->offsets[i+1-variant->first_fragment]-variant->offsets[i-variant->first_fragment];
        else if (i == hyphen) length = 1;
        if (!at && length) {
            group->origin_fragment = i; group->font = run->font;
            group->vertical_marks = doc_layout_vertical_marks(run->attrs.iMarks);
        }
        group->decorations |= run->attrs.iMarks &
            (XUI_DOC_UNDERLINE | XUI_DOC_LINK | XUI_DOC_STRIKE);
        if(variant)memcpy(group->text+at,variant->text+variant->offsets[i-variant->first_fragment],length);
        else if (i == hyphen) group->text[at] = '-';
        else if (fragment->flags & DOC_FORMATTED)
            xuiInternalTextCopyDisplay(run->text + fragment->start - run->base_offset,
                (int)doc_fragment_paint_bytes(fragment), group->text + at);
        else memcpy(group->text + at, run->text + fragment->start - run->base_offset, length);
        at += length;
        if (group->display_ends) group->display_ends[i - first] = (uint32_t)at;
    }
    bytes = at;
    group->text[bytes] = 0;
    group->first_fragment = first; group->end_fragment = end;
    group->bytes = bytes;
    group->bidi_level = block->fragments[first].bidi_level;
    {xui_proxy_caps_t caps;
        result=xuiGetProxyCaps(r->context,&caps);
        if(result!=XUI_OK)return result;
        group->input_caps=caps.iCaps;}
    group->context=block->layout_context;
    group->variant=variant;
    if(variant){
        group->context_offset=variant->offsets[first-variant->first_fragment];
        group->context_bytes=variant->bytes;
        group->script=doc_context_script(group->context,block->fragments[group->origin_fragment].bidi_start,
            variant->hyphens[group->origin_fragment-variant->first_fragment]);
        return XUI_OK;
    }
    if(group->context){
        uint32_t end_offset;
        if(block->fragments[end-1].bidi_end>group->context->source_bytes)return XUI_ERROR_INVALID_STATE;
        group->context_offset=doc_context_offset(group->context,block->fragments[first].bidi_start);
        end_offset=doc_context_offset(group->context,block->fragments[end-1].bidi_end);
        group->context_bytes=group->context->bytes;
        group->script=doc_context_script(group->context,block->fragments[group->origin_fragment].bidi_start,group->origin_fragment==hyphen);
        if(hyphen>=first && hyphen<end){
            uint32_t insertion=doc_context_offset(group->context,block->fragments[hyphen].bidi_start);
            if(end_offset-group->context_offset+1!=bytes || group->context_bytes>=INT_MAX)return XUI_ERROR_INVALID_STATE;
            if(group->input_caps & XUI_PROXY_CAP_TEXT_CONTEXT){
                group->has_hyphen_context=1;group->hyphen_offset=insertion;
            }
            group->context_bytes++;
        }else if(end_offset-group->context_offset!=bytes)return XUI_ERROR_INVALID_STATE;
    }
    return XUI_OK;
}
static int doc_layout_shape_paint(xui_document_renderer r,
    const doc_render_block* block, size_t first, size_t end,
    double* widths, size_t width_base, doc_render_paint_group* group, int* match, size_t hyphen,
    xui_text_shape_t* captured, doc_render_paint_variant* variant)
{
    xui_text_shape_t shape = {0};
    size_t i, bytes; double advance = 0; int result;
    *match = 0;
    if(captured)memset(captured,0,sizeof(*captured));
    result=doc_layout_paint_input(r,block,first,end,group,hyphen,variant);
    if(result!=XUI_OK)return result;
    bytes=(size_t)group->bytes;
    {xui_text_item_t item;doc_hyphen_lease lease;
        result=doc_render_group_item(group,
            block->runs[doc_fragment_paint_run(&block->fragments[group->origin_fragment])].attrs.sLanguage,&item,&lease);
        if(result==XUI_OK){
            if(captured)item.iFlags|=XUI_TEXT_SHAPE_RETAIN_PAINT;
            result=xuiTextShape(r->context,&item,&shape);
        }
        doc_context_hyphen_release(&lease);}
    if (result == XUI_OK) {
        r->stats.iShapedBytes += bytes;
        result = xuiInternalTextShapeCaretFragments(group->text, (int)bytes, &shape);
    }
    if (result == XUI_OK) {
        if (shape.fLineHeight > 0 && isfinite(shape.fLineHeight) && isfinite(shape.fAscent))
            *match = doc_layout_shape_widths(block, first, end, &shape, widths, width_base, group->display_ends);
        for (i = first; *match && i < end; i++) advance += widths[i - width_base];
        group->paint_width = fmax(advance, shape.fWidth);
        group->line_height = shape.fLineHeight; group->ascent = shape.fAscent;
        if (block->layout_bidi && !*match) result = XUI_ERROR_UNSUPPORTED;
    }
    if(captured && result==XUI_OK && *match){*captured=shape;memset(&shape,0,sizeof(shape));}
    xuiTextShapeFree(&shape);
    return result;
}
#include "xui_document_hyphen_variant.inl"
/* Measure exactly the terminal strings, including the selected hyphen's
 * font and script baseline. The scratch widths never enter live geometry.
 * Unsupported cluster maps retain the existing isolated-advance fallback;
 * real shaping/allocation errors propagate to the layout caller. */
static int doc_layout_hyphen_fits(doc_line_plan_context* context,
    size_t first, size_t end, const double* widths, size_t width_base, double width)
{
    const doc_render_block* block = context->block;
    size_t at = first, hyphen = end - 1, i;
    double total = 0;
    if(!doc_layout_hyphen_prefix(block,first,hyphen))return 0;
    if(context->selected){
        doc_render_paint_variant* candidate=NULL;
        context->result=doc_layout_variant_probe(context,first,end,&total,&candidate);
        doc_layout_variant_free(candidate);return context->result==XUI_OK && total<=width;
    }
    context->result = doc_layout_bidi_levels(block, first, end, hyphen,NULL);
    if (context->result != XUI_OK) return 0;
    while (at < end) {
        doc_render_paint_group group = {0}; int match;
        size_t span_end;
        if (!doc_layout_paintable(block, &block->fragments[at])) {
            total += widths[at - width_base]; at++; continue;
        }
        span_end = doc_layout_shape_stop(block, at, end, hyphen,NULL);
        context->result = doc_layout_shape_paint(context->renderer, block,
            at, span_end, context->scratch, context->base, &group, &match, hyphen, NULL,NULL);
        free(group.text); free(group.display_ends);
        if (context->result != XUI_OK) return 0;
        if (!match) {
            total = 0;
            for (i = first; i < end; i++) if (!(block->fragments[i].flags & DOC_SOFT_HYPHEN))
                total += widths[i - width_base];
            return total + block->fragments[hyphen].hyphen_width <= width;
        }
        for (i = at; i < span_end; i++) total += context->scratch[i - context->base];
        at = span_end;
    }
    return total <= width;
}
/* Measure one candidate in its own line context. No neighbor line advances or
 * cached isolated-node widths participate in the candidate's text width. */
static int doc_layout_shared_measure(xui_document_renderer r,const doc_render_block* block,
    size_t index,size_t first,size_t end,uint32_t start,uint32_t stop,xui_vec2_t* measured)
{
    const doc_render_paint_seed* seed=&block->paint_seeds[index-1];
    doc_render_paint_group group={0};xui_text_item_t item;doc_hyphen_lease lease;xui_text_shape_t selected={0};int result;
    if(r->proxy->textShapeRangeMeasure){
        result=r->proxy->textShapeRangeMeasure(r->proxy,doc_render_seed_shape(block,seed),(int)start,(int)stop,measured);
        if(result!=XUI_ERROR_UNSUPPORTED)return result;
    }
    result=doc_layout_paint_input(r,block,first,end,&group,SIZE_MAX,NULL);
    if(result!=XUI_OK)goto done;
    if(!(group.input_caps & XUI_PROXY_CAP_TEXT_RANGE)){result=XUI_ERROR_UNSUPPORTED;goto done;}
    group.shared_seed=index;group.shape_start=start;group.shape_end=stop;
    result=doc_render_seed_range_item(block,&group,block->runs[doc_fragment_paint_run(&block->fragments[group.origin_fragment])].attrs.sLanguage,&item,&lease);
    if(result==XUI_OK){result=xuiTextShape(r->context,&item,&selected);doc_context_hyphen_release(&lease);}
    if(result==XUI_OK){measured->fX=selected.fWidth;measured->fY=selected.fHeight;}
done:
    free(group.text);free(group.display_ends);xuiTextShapeFree(&selected);return result;
}
static int doc_layout_exact_measure(xui_document_renderer r, const doc_render_block* block,
    size_t first, size_t end, size_t paragraph_end, double* scratch, size_t base,
    double* total, int* match,const size_t* shared)
{
    size_t at = first, i;
    size_t hyphen=doc_layout_line_hyphen(block,first,end,paragraph_end);
    *total = 0; *match = 1;
    {
        int result = doc_layout_bidi_levels(block, first, end, hyphen,NULL);
        if (result != XUI_OK) return result;
    }
    while (at < end) {
        doc_render_paint_group group = {0}; size_t span_end; int result;
        if (!doc_layout_paintable(block, &block->fragments[at])) {
            *total += block->fragments[at++].width; continue;
        }
        span_end = doc_layout_shape_stop(block, at, end, hyphen,NULL);
        /* Exact fallback candidates must use the same complete glyph ranges
         * as published rows. Re-shaping an intact subrange here can choose
         * a cut using isolated advances, then publish wider cached glyphs. */
        if(shared && shared[at-base] && hyphen==SIZE_MAX){
            const doc_render_paint_seed* seed=&block->paint_seeds[shared[at-base]-1];
            if(span_end<=seed->end_fragment && seed->bidi_level==block->fragments[at].bidi_level){
                uint32_t start=seed->offsets[at-seed->first_fragment];
                uint32_t stop=seed->offsets[span_end-seed->first_fragment];xui_vec2_t measured;
                result=doc_layout_shared_measure(r,block,shared[at-base],at,span_end,start,stop,&measured);
                if(result==XUI_OK){
                    if(!isfinite(measured.fX) || measured.fX<0 || !isfinite(measured.fY) || measured.fY<0)return XUI_ERROR_INVALID_STATE;
                    memcpy(scratch+at-base,seed->widths+at-seed->first_fragment,(span_end-at)*sizeof(*scratch));
                    for(i=at;i<span_end;i++)*total+=scratch[i-base];
                    at=span_end;continue;
                }
                if(result!=XUI_ERROR_UNSUPPORTED)return result;
            }
        }
        result = doc_layout_shape_paint(r, block, at, span_end, scratch, base, &group, match, hyphen, NULL,NULL);
        free(group.text); free(group.display_ends);
        if (result != XUI_OK || !*match) return result;
        for (i = at; i < span_end; i++) *total += scratch[i - base];
        at = span_end;
    }
    return XUI_OK;
}
static int doc_layout_variant_probe(doc_line_plan_context* context,size_t first,size_t end,
    double* total,doc_render_paint_variant** candidate)
{
    size_t hyphen=doc_layout_line_hyphen(context->block,first,end,context->end);
    int result;
    *candidate=NULL;
    if(hyphen!=SIZE_MAX){
        uint8_t* selected;
        if(context->forbidden[hyphen-context->base]){*total=DBL_MAX;return XUI_OK;}
        selected=malloc(context->end-context->base);if(!selected)return XUI_ERROR_OUT_OF_MEMORY;
        memcpy(selected,context->selected,context->end-context->base);selected[hyphen-context->base]=1;
        result=doc_layout_variant_build(context->renderer,context->block,context->base,context->end,selected,candidate);
        free(selected);
        if(result!=XUI_OK)return result;
    }
    if(*candidate || context->variant)
        result=doc_layout_variant_row(context->renderer,context->block,first,end,hyphen,
            *candidate?*candidate:context->variant,context->scratch,context->base,NULL,NULL,total);
    else {int match;
        result=doc_layout_exact_measure(context->renderer,context->block,first,end,context->end,
            context->scratch,context->base,total,&match,context->shared);
        if(result==XUI_OK && !match)result=XUI_ERROR_UNSUPPORTED;
    }
    if(result!=XUI_OK){doc_layout_variant_free(*candidate);*candidate=NULL;}
    return result;
}
static size_t doc_layout_exact_boundary(const doc_render_block* block,
    size_t first,size_t after, size_t end, int emergency)
{
    size_t next;
    for (next = after + 1; next < end; next++) {
        const doc_fragment* fragment = &block->fragments[next - 1];
        if (fragment->flags & DOC_FORCED_BREAK) return next;
        if((fragment->flags & DOC_SOFT_HYPHEN) && !doc_layout_hyphen_prefix(block,first,next-1))continue;
        if (!(fragment->flags & DOC_GRAPHEME_JOIN) &&
            (fragment->flags & (DOC_NORMAL_BREAK | (emergency ? DOC_EMERGENCY_BREAK : 0))))
            return next;
    }
    return end;
}
/* A finite left-to-right greedy planner for an unstable global replan. Try
 * normal boundaries in source order, keeping the last fitting candidate; an
 * overwide first word uses grapheme-safe emergency boundaries. One indivisible
 * unit may overflow. Every committed line consumes at least one fragment. */
static int doc_layout_exact_cuts(xui_document_renderer r, const doc_render_block* block,
    size_t first, size_t end, double width, double* scratch, size_t* cuts,
    size_t* count, int* match,const size_t* shared)
{
    size_t at = first;
    *count = 0; *match = 1;
    while (at < end) {
        size_t stop = at, probe = at;
        for (;;) {
            size_t candidate = doc_layout_exact_boundary(block,at,probe,end,0);
            double total; int result;
            result = doc_layout_exact_measure(r, block, at, candidate, end, scratch, first, &total, match,shared);
            if (result != XUI_OK || !*match) return result;
            if (total <= width) {
                stop = candidate;
                if (stop == end || (block->fragments[stop - 1].flags & DOC_FORCED_BREAK)) break;
                probe = candidate; continue;
            }
            if (stop == at) {
                size_t first_unit = doc_layout_exact_boundary(block,at,at,candidate,1);
                probe = at;
                while (probe < candidate) {
                    size_t next = doc_layout_exact_boundary(block,at,probe,candidate,1);
                    result = doc_layout_exact_measure(r, block, at, next, end, scratch, first, &total, match,shared);
                    if (result != XUI_OK || !*match) return result;
                    if (total > width) break;
                    stop = next; probe = next;
                }
                if (stop == at){
                    /* A rejected discretionary hyphen is not an indivisible
                     * visible unit. Consume its preceding complete grapheme
                     * without forcing the too-wide insertion. */
                    if(first_unit>at+1 && (block->fragments[first_unit-1].flags & DOC_SOFT_HYPHEN) &&
                        !(block->fragments[first_unit-2].flags & DOC_GRAPHEME_JOIN))first_unit--;
                    stop=first_unit;
                }
            }
            break;
        }
        cuts[(*count)++] = stop; at = stop;
    }
    return XUI_OK;
}
static int doc_layout_shape_lines(xui_document_renderer r, const doc_render_block* block,
    size_t first, size_t end, const size_t* cuts, size_t count,
    double* widths, doc_render_paint_group* pending, size_t* group_count, int* match,
    const size_t* shared)
{
    size_t line_first = first, line;
    xui_proxy_caps_t caps={0};int cap_result=xuiGetProxyCaps(r->context,&caps);
    if(cap_result!=XUI_OK)return cap_result;
    *group_count = 0; *match = 1;
    for (line = 0; line < count; line++) {
        size_t line_end = cuts[line];
        size_t hyphen=doc_layout_line_hyphen(block,line_first,line_end,end);
        int resolved = doc_layout_bidi_levels(block, line_first, line_end, hyphen,NULL);
        if (resolved != XUI_OK) return resolved;
        while (line_first < line_end) {
            size_t span_end; int result;
            doc_render_paint_group* group;
            if (!doc_layout_paintable(block, &block->fragments[line_first])) { line_first++; continue; }
            span_end = doc_layout_shape_stop(block, line_first, line_end, hyphen,NULL);
            group=&pending[(*group_count)++];
            result=XUI_ERROR_UNSUPPORTED;
            if(shared && shared[line_first-first] && hyphen==SIZE_MAX){
                const doc_render_paint_seed* seed=&block->paint_seeds[shared[line_first-first]-1];
                /* UBA L1 can reset trailing whitespace at this line boundary.
                 * A seed shaped with a different direction/level cannot supply
                 * this row; its intact neighbours may still share the seed. */
                if(span_end<=seed->end_fragment && seed->bidi_level==block->fragments[line_first].bidi_level){
                    const xui_text_shape_t* shape=doc_render_seed_shape(block,seed);
                    uint32_t start=seed->offsets[line_first-seed->first_fragment];
                    uint32_t stop=seed->offsets[span_end-seed->first_fragment];
                    xui_vec2_t size;
                    result=doc_layout_shared_measure(r,block,shared[line_first-first],line_first,span_end,start,stop,&size);
                    if(result==XUI_OK){
                        size_t at;double advance=0;
                        if(!isfinite(size.fX) || size.fX<0 || !isfinite(size.fY) || size.fY<0)
                            return XUI_ERROR_INVALID_STATE;
                        result=doc_layout_paint_input(r,block,line_first,span_end,group,SIZE_MAX,NULL);
                        if(result!=XUI_OK)return result;
                        memcpy(widths+line_first-first,seed->widths+line_first-seed->first_fragment,
                            (span_end-line_first)*sizeof(*widths));
                        for(at=line_first;at<span_end;at++)advance+=widths[at-first];
                        group->paint_width=fmax(advance,size.fX);
                        group->line_height=shape->fLineHeight;group->ascent=shape->fAscent;
                        group->shared_seed=shared[line_first-first];
                        group->shape_start=start;group->shape_end=stop;
                        group->range_input=!r->proxy->drawTextShapeRangeSpans || !shape->pPaint;
                    }
                }
            }
            /* A row cutting through a ligature needs a distinct shaped result.
             * An intact range shares the full span's substitutions and carets. */
            if(result==XUI_ERROR_UNSUPPORTED)
                result = doc_layout_shape_paint(r, block, line_first, span_end,
                    widths, first, group, match, hyphen,
                    (r->proxy->drawTextShapeRangeSpans || (caps.iCaps & XUI_PROXY_CAP_TEXT_RANGE))?&group->shape:NULL,NULL);
            if (result != XUI_OK || !*match) return result;
            line_first = span_end;
        }
    }
    return XUI_OK;
}
static int doc_layout_publish_lines(doc_render_block* block, size_t first, size_t end,
    const double* widths, const size_t* cuts, size_t count,
    const doc_render_paint_group* pending, size_t group_count)
{
    size_t line, i; int result;
    if (group_count > SIZE_MAX - block->paint_group_count) return XUI_DOC_ERROR_LIMIT;
    result = doc_render_reserve((void**)&block->paint_groups, &block->paint_group_capacity,
        block->paint_group_count + group_count, sizeof(*block->paint_groups));
    if (result != XUI_OK) return result;
    for (line = 0; line < group_count; line++) {
        const doc_render_paint_group* group = &pending[line];
        for (i = group->first_fragment; i < group->end_fragment; i++) {
            doc_fragment* fragment = &block->fragments[i];
            fragment->width = widths[i - first];
            if (doc_layout_display_bytes(block, fragment) ||
                ((fragment->flags & DOC_SOFT_HYPHEN) && widths[i - first] > 0) ||
                (block->runs[doc_fragment_paint_run(fragment)].font == group->font &&
                 doc_layout_vertical_marks(block->runs[doc_fragment_paint_run(fragment)].attrs.iMarks) == group->vertical_marks) ||
                (fragment->flags & DOC_GRAPHEME_MEMBER)) {
                fragment->height = group->line_height;
                fragment->baseline = doc_layout_baseline(group->ascent, group->line_height, group->vertical_marks);
            }
        }
    }
    for (i = first; i < end; i++) block->fragments[i].flags |= DOC_CONTEXT_LAYOUT;
    for (i = 0; i < count; i++) block->fragments[cuts[i] - 1].flags |= DOC_CONTEXT_LINE_END;
    if (group_count) memcpy(block->paint_groups + block->paint_group_count, pending, group_count * sizeof(*pending));
    block->paint_group_count += group_count;
    return XUI_OK;
}
static size_t doc_layout_variant_boundary(doc_line_plan_context* context,size_t first,
    size_t after,size_t end,int emergency)
{
    size_t next=after;
    do{
        next=doc_layout_exact_boundary(context->block,first,next,end,emergency);
    }while(next<end && (context->block->fragments[next-1].flags & DOC_SOFT_HYPHEN) &&
        context->forbidden[next-1-context->base]);
    return next;
}
/* Probe every alternative against its own complete insertion pattern. A
 * merely tentative marker cannot shrink an unbroken-line alternative. */
static int doc_layout_variant_choose(doc_line_plan_context* context,size_t first,size_t end,
    double width,size_t* stop,doc_render_paint_variant** chosen)
{
    size_t probe=first;int result;
    *stop=first;*chosen=NULL;
    for(;;){
        size_t next=doc_layout_variant_boundary(context,first,probe,end,0);
        doc_render_paint_variant* candidate=NULL;double total;
        result=doc_layout_variant_probe(context,first,next,&total,&candidate);
        if(result!=XUI_OK)goto fail;
        if(total<=width){
            *stop=next;doc_layout_variant_free(*chosen);*chosen=candidate;candidate=NULL;
            if(next==end || (context->block->fragments[next-1].flags & DOC_FORCED_BREAK))break;
            probe=next;continue;
        }
        doc_layout_variant_free(candidate);
        {size_t hyphen=doc_layout_hyphen_after(context,context->block->fragments,first,next-1,context->end,
            context->scratch,context->base,width);
            if(context->result!=XUI_OK){result=context->result;goto fail;}
            if(hyphen>first && hyphen<=end){
                result=doc_layout_variant_probe(context,first,hyphen,&total,&candidate);
                if(result!=XUI_OK)goto fail;
                *stop=hyphen;doc_layout_variant_free(*chosen);*chosen=candidate;break;
            }
        }
        if(*stop==first){
            size_t unit=doc_layout_variant_boundary(context,first,first,next,1);
            probe=first;
            while(probe<next){
                size_t boundary=doc_layout_variant_boundary(context,first,probe,next,1);
                candidate=NULL;result=doc_layout_variant_probe(context,first,boundary,&total,&candidate);
                if(result!=XUI_OK)goto fail;
                if(total>width){doc_layout_variant_free(candidate);break;}
                *stop=boundary;doc_layout_variant_free(*chosen);*chosen=candidate;probe=boundary;
            }
            if(*stop==first){
                if(unit>first+1 && (context->block->fragments[unit-1].flags & DOC_SOFT_HYPHEN) &&
                    !(context->block->fragments[unit-2].flags & DOC_GRAPHEME_JOIN))unit--;
                *stop=unit;
            }
        }
        break;
    }
    return XUI_OK;
fail:
    doc_layout_variant_free(*chosen);*chosen=NULL;return result;
}
/* Prior committed insertions feed later candidate glyphs. After all choices,
 * validate the rows against that final pattern. A future insertion that makes
 * an earlier discretionary row too wide is forbidden on retry; stricter legal
 * row bounds only decrease. This gives finite progress without cut cycles. */
static int doc_layout_variant_lines(xui_document_renderer r,doc_render_block* block,
    size_t first,size_t end,double width,const size_t* shared,double* scratch,size_t* cuts)
{
    size_t n=end-first,attempt=0;uint8_t* selected=calloc(n,1),*forbidden=calloc(n,1);
    size_t* limits=calloc(n,sizeof(*limits));double* widths=calloc(n,sizeof(*widths));
    doc_render_paint_variant* variant=NULL;doc_render_paint_group* pending=NULL;
    doc_line_plan_context context={.renderer=r,.block=block,.base=first,.end=end,.shared=shared,
        .scratch=scratch,.selected=selected,.forbidden=forbidden};
    int result=XUI_OK;
    if(!selected || !forbidden || !limits || !widths){result=XUI_ERROR_OUT_OF_MEMORY;goto done;}
    for(;;){
        size_t at=first,count=0,groups=0,line,retry=SIZE_MAX;int retry_marker=0;
        memset(selected,0,n);doc_layout_variant_free(variant);variant=NULL;context.variant=NULL;context.result=XUI_OK;
        /* Continuation may already have committed a marker before first. */
        result=doc_layout_variant_build(r,block,first,end,selected,&variant);
        if(result!=XUI_OK)goto done;
        context.variant=variant;
        while(at<end){
            doc_render_paint_variant* chosen=NULL;size_t stop,limit=limits[at-first]?limits[at-first]:end;
            result=doc_layout_variant_choose(&context,at,limit,width,&stop,&chosen);
            if(result!=XUI_OK)goto done;
            if(chosen){
                doc_layout_variant_free(variant);variant=chosen;context.variant=variant;
                memcpy(selected,variant->hyphens,n);
            }
            cuts[count++]=stop;at=stop;
        }
        if(!variant && !attempt){result=XUI_ERROR_NOT_FOUND;goto done;}
        pending=calloc(n,sizeof(*pending));if(!pending){result=XUI_ERROR_OUT_OF_MEMORY;goto done;}
        at=first;
        if(!variant){int match;size_t groups_base=0;
            result=doc_layout_shape_lines(r,block,first,end,cuts,count,widths,pending,&groups_base,&match,shared);
            groups=groups_base;
            if(result==XUI_OK && !match)result=XUI_ERROR_UNSUPPORTED;
            if(result!=XUI_OK)goto pending_done;
        }
        for(line=0;line<count;line++){
            double total=0;size_t stop=cuts[line],hyphen=doc_layout_line_hyphen(block,at,stop,end),j;
            if(variant)result=doc_layout_variant_row(r,block,at,stop,hyphen,variant,widths,first,pending,&groups,&total);
            else for(j=at;j<stop;j++)total+=widths[j-first];
            if(result!=XUI_OK)goto pending_done;
            if(total>width){
                if(hyphen!=SIZE_MAX){retry=hyphen;retry_marker=1;break;}
                /* Indivisible Unicode units may overflow. A fitting earlier
                 * legal prefix instead requires a stricter row on retry. */
                for(j=at;j<stop;){
                    doc_render_paint_variant* probe=NULL;double measured;
                    size_t boundary=doc_layout_variant_boundary(&context,at,j,stop,1);
                    if(boundary==stop)break;
                    result=doc_layout_variant_probe(&context,at,boundary,&measured,&probe);doc_layout_variant_free(probe);
                    if(result!=XUI_OK)goto pending_done;
                    if(measured<=width && (!limits[at-first] || boundary<limits[at-first])){
                        retry=at;limits[at-first]=boundary;break;
                    }
                    j=boundary;
                }
                if(retry!=SIZE_MAX)break;
            }
            at=stop;
        }
        if(retry==SIZE_MAX){
            result=doc_layout_publish_lines(block,first,end,widths,cuts,count,pending,groups);
            if(result==XUI_OK){
                free(pending);pending=NULL;
                if(variant){variant->next=block->variants;block->variants=variant;variant=NULL;}
            }
        }
pending_done:
        if(pending){doc_layout_pending_paint_free(pending,groups);pending=NULL;}
        if(result!=XUI_OK || retry==SIZE_MAX)break;
        if(retry_marker)forbidden[retry-first]=1;
        attempt++;
    }
done:
    doc_layout_variant_free(variant);free(selected);free(forbidden);free(limits);free(widths);return result;
}
/* Shape the exact strings that will be drawn on each visual line. Replan after
 * shaping because a kerning change at a style or wrap boundary may change the
 * line cuts. No fragment geometry is published until the cuts converge. */
static int doc_layout_paint_group(xui_document_renderer r, doc_render_block* block,
    size_t first, size_t end, double width)
{
    enum { fast_replans = 8 };
    size_t *cuts = NULL, *next_cuts = NULL;
    size_t* shared = NULL;
    double *widths = NULL, *next_widths = NULL;
    doc_line_plan_context plan_context = {0};
    doc_line_plan_context* contextual = NULL;
    doc_render_paint_group* pending = NULL;
    uint32_t previous_run = UINT32_MAX;
    size_t i, runs = 0, spans = 0, fragments = end - first;
    int iteration, result = XUI_OK, projected = 0, has_hyphen = 0, any_shared = 0, range_input=0;
    xui_proxy_caps_t range_caps={0};
    if(r->proxy->getCaps){result=r->proxy->getCaps(r->proxy,&range_caps);if(result!=XUI_OK)return result;}
    range_input=(range_caps.iCaps & XUI_PROXY_CAP_TEXT_RANGE)!=0;
    for (i = first; i < end; i++)
        block->fragments[i].flags &= ~(DOC_CONTEXT_LAYOUT | DOC_CONTEXT_LINE_END);
    if (first >= end) return XUI_OK;
    if (block->layout_bidi) {
        size_t bytes = xuiInternalTextBidiTextBytes(block->layout_bidi);
        for (i = first; i < end; i++) {
            doc_fragment* fragment = &block->fragments[i];
            if (fragment->bidi_start < bytes)
                result = xuiInternalTextBidiLevel(block->layout_bidi, fragment->bidi_start, &fragment->bidi_level);
            else {
                xui_bidi_paragraph_t paragraph;
                size_t count = xuiInternalTextBidiParagraphCount(block->layout_bidi);
                result = count ? xuiInternalTextBidiParagraph(block->layout_bidi, count - 1, &paragraph) : XUI_OK;
                fragment->bidi_level = count ? paragraph.base : 0;
            }
            if (result != XUI_OK) return result;
        }
    }
    for (i = first; i < end; i++) {
        const doc_fragment* fragment = &block->fragments[i];
        if (fragment->flags & DOC_FORMATTED) projected = 1;
        if (fragment->flags & DOC_SOFT_HYPHEN) has_hyphen = 1;
        const doc_render_run* run;
        size_t length;
        if (fragment->run == UINT32_MAX && fragment->kind != XUI_DOC_SOFT_BREAK) {
            if (fragment->kind == XUI_DOC_SOFT_BREAK || fragment->kind == XUI_DOC_HARD_BREAK) {
                runs++;
                previous_run = fragment->break_style_run;
            }
            continue;
        }
        if (doc_fragment_paint_run(fragment) >= block->run_count)
            return block->layout_bidi ? XUI_ERROR_INVALID_STATE : XUI_OK;
        run = &block->runs[doc_fragment_paint_run(fragment)];
        if ((!r->proxy->drawLine && (run->attrs.iMarks &
                (XUI_DOC_UNDERLINE | XUI_DOC_LINK | XUI_DOC_STRIKE))) ||
            fragment->start < run->base_offset ||
            fragment->end < fragment->start ||
            fragment->end - run->base_offset > run->bytes)
            return block->layout_bidi ? XUI_ERROR_UNSUPPORTED : XUI_OK;
        length = (size_t)doc_fragment_paint_bytes(fragment);
        if (doc_fragment_paint_run(fragment) != previous_run) {
            if (previous_run != UINT32_MAX && doc_fragment_paint_run(fragment) != previous_run + 1)
                return XUI_OK;
            runs++;
            previous_run = doc_fragment_paint_run(fragment);
        }
        if (!doc_layout_paintable(block, fragment)) {
            continue;
        }
        if (i > first && doc_fragment_paint_run(&block->fragments[i - 1]) == doc_fragment_paint_run(fragment) &&
            block->fragments[i - 1].end != fragment->start) return XUI_OK;
        if (memchr(run->text + fragment->start - run->base_offset,
                '\r', length) ||
            memchr(run->text + fragment->start - run->base_offset,
                '\n', length)) return XUI_OK;
    }
    if (!runs) return XUI_OK;
    widths = calloc(fragments, sizeof(*widths));
    next_widths = calloc(fragments, sizeof(*next_widths));
    cuts = calloc(fragments, sizeof(*cuts));
    next_cuts = calloc(fragments, sizeof(*next_cuts));
    if (!widths || !next_widths || !cuts || !next_cuts) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    if (has_hyphen) {
        plan_context.renderer = r; plan_context.block = block; plan_context.base = first;
        plan_context.scratch = calloc(fragments, sizeof(*plan_context.scratch));
        if (!plan_context.scratch) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
        contextual = &plan_context;
    }
    for (i = first; i < end; i++) widths[i - first] = block->fragments[i].width;
    if (!block->layout_bidi && (!block->layout_context || !block->layout_context->scripts) && !projected && runs == 1 && doc_layout_plan_lines(block, first, end, width,
        widths, cuts, fragments, NULL) == 1) goto done;
    /* A deleted, unselected SHY belongs to the base displayed span. Rows
     * selecting it still build their own insertion variant in shape_lines. */
    if((r->proxy->textShapeRangeMeasure && r->proxy->drawTextShapeRangeSpans) || range_input){
        shared=calloc(fragments,sizeof(*shared));
        if(!shared){result=XUI_ERROR_OUT_OF_MEMORY;goto done;}
    }
    for (i = first; i < end; ) {
        doc_render_paint_group seed = {0}; int match;
        xui_text_shape_t captured={0};uint32_t source_run=0;
        const doc_render_run* run;
        size_t cached;
        size_t span_end;
        if (!doc_layout_paintable(block, &block->fragments[i])) { i++; continue; }
        span_end = doc_layout_shape_stop(block, i, end, SIZE_MAX,NULL);
        /* A complete cached run is a usable first line plan. Contextual
         * advances may differ, but the final row shaping/replan below must
         * still consume the retained context before publication. Avoid an
         * extra whole-span pass solely because another script occurs later
         * in the paragraph. Cross-node and continuation spans still seed. */
        {
            size_t origin=i;
            /* Deletion projection can start with controls from another font.
             * Match the first visible font used by paint_input; source byte
             * identities stay on the fragments, displayed bytes on offsets. */
            while(origin<span_end && !doc_layout_display_bytes(block,&block->fragments[origin]))origin++;
            if(origin==span_end)origin=i;
            run=&block->runs[doc_fragment_paint_run(&block->fragments[origin])];
        }
        {
            if (!block->layout_bidi && i == run->first_fragment && span_end == run->end_fragment &&
                !shared) {
                i = span_end; spans++; continue;
            }
        }
        for (cached = 0; cached < block->paint_seed_count; cached++) {
            const doc_render_paint_seed* saved = &block->paint_seeds[cached];
            if (saved->first_fragment == i && saved->end_fragment == span_end &&
                saved->font == run->font &&
                saved->bidi_level == block->fragments[i].bidi_level &&
                saved->vertical_marks == doc_layout_vertical_marks(run->attrs.iMarks)) {
                memcpy(widths + i - first, saved->widths, (span_end - i) * sizeof(*widths));
                break;
            }
        }
        if (cached == block->paint_seed_count) {
            double* saved;
            uint32_t* offsets=NULL;
            /* Isolated ASCII in a Unicode paragraph can resolve to another
             * script (for example paired Greek brackets). Borrow its shape
             * only when the paragraph has no script-resolution map. */
            if(shared && !projected && !block->layout_bidi && i==run->first_fragment && span_end==run->end_fragment && run->shape.pPaint &&
                (!block->layout_context || !block->layout_context->scripts))
                source_run=doc_fragment_paint_run(&block->fragments[i])+1;
            else{
                result = doc_layout_shape_paint(r, block, i, span_end, widths, first, &seed, &match,
                    SIZE_MAX, shared?&captured:NULL,NULL);
                free(seed.text); free(seed.display_ends);
                if (result != XUI_OK || !match) {xuiTextShapeFree(&captured);goto done;}
            }
            saved = malloc((span_end - i) * sizeof(*saved));
            if (!saved) {xuiTextShapeFree(&captured);result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
            if(source_run || captured.pPaint || (range_input && captured.iSize)){
                size_t at;
                offsets=calloc(span_end-i+1,sizeof(*offsets));
                if(!offsets){free(saved);xuiTextShapeFree(&captured);result=XUI_ERROR_OUT_OF_MEMORY;goto done;}
                for(at=i;at<span_end;at++)offsets[at-i+1]=offsets[at-i]+
                    (uint32_t)doc_layout_display_bytes(block,&block->fragments[at]);
            }
            result = doc_render_reserve((void**)&block->paint_seeds, &block->paint_seed_capacity,
                block->paint_seed_count + 1, sizeof(*block->paint_seeds));
            if (result != XUI_OK) { free(saved);free(offsets);xuiTextShapeFree(&captured);goto done; }
            memcpy(saved, widths + i - first, (span_end - i) * sizeof(*saved));
            block->paint_seeds[block->paint_seed_count++] = (doc_render_paint_seed){
                .first_fragment = i, .end_fragment = span_end, .font = run->font,
                .vertical_marks = doc_layout_vertical_marks(run->attrs.iMarks), .widths = saved,
                .bidi_level = block->fragments[i].bidi_level,.shape=captured,
                .source_run=source_run,.offsets=offsets};
        }
        if(shared && block->paint_seeds[cached].offsets){
            size_t at;
            for(at=i;at<span_end;at++)shared[at-first]=cached+1;
            any_shared=1;
        }
        i = span_end; spans++;
    }
    if (!spans) goto done;
    if(has_hyphen && (any_shared || range_input) && block->layout_context){
        result=doc_layout_variant_lines(r,block,first,end,width,shared,plan_context.scratch,cuts);
        if(result!=XUI_ERROR_NOT_FOUND)goto done;
        result=XUI_OK;
    }
    for (iteration = 0; iteration < fast_replans; iteration++) {
        size_t count = doc_layout_plan_lines(block, first, end, width,
            widths, cuts, fragments, contextual);
        size_t group_count = 0;
        int match = 1;
        if (plan_context.result != XUI_OK) { result = plan_context.result; goto done; }
        if (!count || (!any_shared && !block->layout_bidi && (!block->layout_context || !block->layout_context->scripts) && !projected && count == 1 && runs == 1)) break;
        /* Each visual cut can also expose a differently styled discretionary
         * hyphen that was invisible in the seed span. Budget both groups. */
        if (count > (SIZE_MAX - spans) / 2 ||
            count * 2 + spans > SIZE_MAX / sizeof(*pending)) { result = XUI_DOC_ERROR_LIMIT; goto done; }
        pending = calloc(block->layout_bidi ? fragments : count * 2 + spans, sizeof(*pending));
        if (!pending) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
        /* Text shaping replaces only text advances. Measured object widths,
         * soft-break spaces and zero-width controls survive every replan. */
        memcpy(next_widths, widths, fragments * sizeof(*widths));
        result = doc_layout_shape_lines(r, block, first, end, cuts, count,
            next_widths, pending, &group_count, &match, shared);
        if (result != XUI_OK || !match) {
            doc_layout_pending_paint_free(pending, group_count);
            pending = NULL;
            if (result == XUI_OK) break;
            goto done;
        }
        {
            size_t next_count = doc_layout_plan_lines(block, first, end,
                width, next_widths, next_cuts, fragments, contextual);
            if (plan_context.result != XUI_OK) {
                result = plan_context.result;
                doc_layout_pending_paint_free(pending, group_count);
                pending = NULL;
                goto done;
            }
            if (next_count == count &&
                memcmp(cuts, next_cuts, count * sizeof(*cuts)) == 0) {
                result = doc_layout_publish_lines(block, first, end, next_widths,
                    cuts, count, pending, group_count);
                if (result != XUI_OK) {
                    doc_layout_pending_paint_free(pending, group_count);
                    pending = NULL;
                    goto done;
                }
                free(pending); pending = NULL;
                break;
            }
        }
        doc_layout_pending_paint_free(pending, group_count);
        pending = NULL;
        { double* swap = widths; widths = next_widths; next_widths = swap; }
    }
    if (iteration == fast_replans) {
        size_t count = 0, group_count = 0; int match;
        result = doc_layout_exact_cuts(r, block, first, end, width, next_widths, cuts, &count, &match,shared);
        if (result != XUI_OK || !match) goto done;
        if (count > (SIZE_MAX - spans) / 2 || count * 2 + spans > SIZE_MAX / sizeof(*pending)) {
            result = XUI_DOC_ERROR_LIMIT; goto done;
        }
        pending = calloc(block->layout_bidi ? fragments : count * 2 + spans, sizeof(*pending));
        if (!pending) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
        memcpy(next_widths, widths, fragments * sizeof(*widths));
        result = doc_layout_shape_lines(r, block, first, end, cuts, count,
            next_widths, pending, &group_count, &match, shared);
        if (result == XUI_OK && match) {
            result = doc_layout_publish_lines(block, first, end, next_widths,
                cuts, count, pending, group_count);
            if (result == XUI_OK) { free(pending); pending = NULL; }
        }
        if (pending) { doc_layout_pending_paint_free(pending, group_count); pending = NULL; }
    }
done:
    free(widths); free(next_widths); free(cuts); free(next_cuts); free(plan_context.scratch);free(shared);
    return result;
}
/* Replace provisional groups only after their new tail has been shaped.
 * Earlier committed groups keep both their text and fragment identities. */
static void doc_layout_replace_paint_tail(doc_render_block* block,
    size_t keep, size_t previous)
{
    size_t i, added = block->paint_group_count - previous;
    if (keep == previous) return;
    for (i = keep; i < previous; i++) {
        free(block->paint_groups[i].text); free(block->paint_groups[i].display_ends);
        xuiTextShapeFree(&block->paint_groups[i].shape);
    }
    if (added) memmove(block->paint_groups + keep, block->paint_groups + previous,
        added * sizeof(*block->paint_groups));
    block->paint_group_count = keep + added;
}
static int doc_layout_breaks(xui_document_renderer r, doc_render_block* b, size_t start, xui_text_bidi* bidi)
{
    size_t i; uint64_t bytes = 0, at = 0; char* text; unsigned char* map;
    int result, previous_join = 0;
    if(start>=b->fragment_count)return XUI_OK;
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
    text[at] = 0;
    {doc_shape_context* retained=NULL;
        result=doc_layout_context_acquire(r,b,b->fragments[start].node,text,(size_t)bytes,NULL,0,&retained);
        if(result!=XUI_OK){free(text);free(map);return result;}
        b->layout_context=retained;
    }
    result = xuiInternalTextBreakMap(text, (int)bytes, map);
    if (result == XUI_OK) result = xuiInternalTextDisplayGraphemes(text, (int)bytes, map);
    if (result == XUI_OK && xuiInternalTextBidiNeedsAnalysis(text, (size_t)bytes)) {
        for (i = start, at = 0; i < b->fragment_count; i++) {
            const doc_fragment* f = &b->fragments[i];
            if (f->kind == XUI_DOC_HARD_BREAK) memcpy(text + at, "\xe2\x80\xa8", 3);
            at += f->run != UINT32_MAX ? f->end - f->start : 3;
        }
        result = xuiInternalTextBidiCreate(text, (size_t)bytes, XUI_BIDI_AUTO_LTR, bidi);
    }
    if (result == XUI_OK) for (i = start, at = 0; i < b->fragment_count; i++) {
        doc_fragment* f = &b->fragments[i];
        uint64_t begin = at;
        at += f->run != UINT32_MAX ? f->end - f->start : 3;
        doc_layout_control_flags(b, f, map, begin, at);
        f->flags &= ~(DOC_NORMAL_BREAK | DOC_EMERGENCY_BREAK |
            DOC_GRAPHEME_JOIN | DOC_GRAPHEME_MEMBER);
        if (f->run != UINT32_MAX && at > 0 && at < bytes && !(map[at] & XUI_LB_GRAPHEME))
            f->flags |= DOC_GRAPHEME_JOIN;
        if (previous_join || (f->flags & DOC_GRAPHEME_JOIN))
            f->flags |= DOC_GRAPHEME_MEMBER;
        previous_join = !!(f->flags & DOC_GRAPHEME_JOIN);
        if (map[at] & XUI_LB_NORMAL) f->flags |= DOC_NORMAL_BREAK;
        if (map[at] & XUI_LB_EMERGENCY) f->flags |= DOC_EMERGENCY_BREAK;
        f->bidi_start = (size_t)begin; f->bidi_end = (size_t)at;
        f->bidi_active = *bidi != NULL; f->bidi_level = 0;
        if (*bidi && begin < bytes) {
            result = xuiInternalTextBidiLevel(*bidi, (size_t)begin, &f->bidi_level);
            if (result != XUI_OK) break;
        }
    }
    free(text); free(map); return result;
}
/* Nonwrapping SOURCE/code rows still need grapheme endpoints. Printable
 * ASCII rows, including their horizontal continuation path, have no
 * interior grapheme boundaries and retain the allocation-free fast path. */
static int doc_layout_row_graphemes(xui_document_renderer r, doc_render_block* block, xui_text_bidi* bidi)
{
    size_t i;
    for (i = 0; i < block->run_count; i++) {
        const doc_render_run* run = &block->runs[i];
        uint64_t byte;
        for (byte = 0; byte < run->bytes; byte++)
            if ((unsigned char)run->text[byte] >= 0x80 ||
                (unsigned char)run->text[byte] < 32)
                return doc_layout_breaks(r, block, 0, bidi);
    }
    return XUI_OK;
}
static double doc_layout_lines(xui_document_renderer r, doc_render_block* b, size_t start, double x, double y, double width, uint32_t alignment, int wrap)
{
    size_t end = b->fragment_count, first = start, scan;
    size_t paint_at = 0, paint_end = b->paint_group_count;
    double top = y;
    while (paint_at < paint_end) {
        size_t middle = paint_at + (paint_end - paint_at) / 2;
        if (b->paint_groups[middle].end_fragment <= start) paint_at = middle + 1;
        else paint_end = middle;
    }
    if (b->visual_fragments) {
        size_t old = b->visual_capacity;
        int result = doc_render_reserve((void**)&b->visual_fragments, &b->visual_capacity,
            end, sizeof(*b->visual_fragments));
        if (result != XUI_OK) { b->layout_error = result; return 0; }
        for (scan = old; scan < b->visual_capacity; scan++) b->visual_fragments[scan] = scan;
    }
    for (scan = start; scan < end; scan++) {
        doc_fragment* fragment = &b->fragments[scan];
        fragment->flags &= ~DOC_HYPHEN_USED;
        if (fragment->flags & DOC_SOFT_HYPHEN) {
            const doc_render_paint_variant* variant=doc_layout_line_variant(b,scan,scan+1);
            if ((variant && variant->hyphens[scan-variant->first_fragment]) || fragment->width > 0)
                fragment->hyphen_width = fragment->width;
            fragment->width = 0;
        }
    }
    while (first < end) {
        size_t stop = doc_layout_line_stop(b, first, end,
            width, wrap, NULL, 0, NULL), i;
        double advance, ascent = 0, descent = 0, total = 0;
        if (wrap && doc_layout_line_hyphen(b,first,stop,end)!=SIZE_MAX) {
            b->fragments[stop - 1].flags |= DOC_HYPHEN_USED;
            b->fragments[stop - 1].width = b->fragments[stop - 1].hyphen_width;
        }
        for (i = first; i < stop; i++) {
            doc_fragment* f = &b->fragments[i]; total += f->width;
            if (f->baseline > ascent) ascent = f->baseline;
            if (f->height - f->baseline > descent) descent = f->height - f->baseline;
        }
        advance = x + (alignment == 1 ? fmax(0, width - total) / 2 : alignment == 2 ? fmax(0, width - total) : 0);
        if (b->layout_bidi) {
            size_t old_capacity = b->visual_capacity, units = 0, k, slot;
            int maximum = 0, minimum_odd = 126, level;
            int result = doc_layout_bidi_levels(b, first, stop,
                (b->fragments[stop - 1].flags & DOC_HYPHEN_USED) ? stop - 1 : SIZE_MAX,doc_layout_line_variant(b,first,stop));
            if (result == XUI_OK) result = doc_render_reserve((void**)&b->visual_fragments,
                &b->visual_capacity, b->fragment_count, sizeof(*b->visual_fragments));
            if (result != XUI_OK) { b->layout_error = result; return 0; }
            for (i = old_capacity; i < b->visual_capacity; i++) b->visual_fragments[i] = i;
            for (i = first; i < stop; ) {
                uint8_t value = b->fragments[i].bidi_level;
                b->visual_fragments[first + units++] = i;
                if (value > maximum) maximum = value;
                if ((value & 1) && value < minimum_odd) minimum_odd = value;
                do { i++; } while (i < stop && (b->fragments[i - 1].flags & DOC_GRAPHEME_JOIN));
            }
            /* UBA L2 operates on intact graphemes. L3 keeps combining marks
             * with their base even when a style boundary splits the grapheme. */
            for (level = maximum; level >= minimum_odd; level--) {
                for (k = 0; k < units; ) {
                    size_t a, z;
                    if (b->fragments[b->visual_fragments[first + k]].bidi_level < level) { k++; continue; }
                    a = k++;
                    while (k < units && b->fragments[b->visual_fragments[first + k]].bidi_level >= level) k++;
                    z = k;
                    while (a < --z) {
                        size_t swap = b->visual_fragments[first + a];
                        b->visual_fragments[first + a++] = b->visual_fragments[first + z];
                        b->visual_fragments[first + z] = swap;
                    }
                }
            }
            for (k = 0; k < units; k++) {
                i = b->visual_fragments[first + k];
                do {
                    doc_fragment* f = &b->fragments[i];
                    f->x = advance; f->y = top + ascent - f->baseline; advance += f->width;
                    i++;
                } while (i < stop && (b->fragments[i - 1].flags & DOC_GRAPHEME_JOIN));
            }
            /* Expand packed grapheme heads backwards, preserving logical
             * order inside each unit without another allocation. */
            slot = stop;
            for (k = units; k > 0; k--) {
                size_t head = b->visual_fragments[first + k - 1], tail = head + 1;
                while (tail < stop && (b->fragments[tail - 1].flags & DOC_GRAPHEME_JOIN)) tail++;
                while (tail > head) b->visual_fragments[--slot] = --tail;
            }
        } else for (i = first; i < stop; i++) {
            doc_fragment* f = &b->fragments[i];
            f->x = advance; f->y = top + ascent - f->baseline; advance += f->width;
            if (b->visual_fragments) b->visual_fragments[i] = i;
        }
        if (advance > b->width) b->width = advance;
        /* A complete cached LTR run needs no second shape, but its ink can
         * overhang the logical advance after GPOS. Keep carets/wrapping at
         * the advance while retaining that ink in the scroll extent. */
        while (paint_at < b->paint_group_count &&
            b->paint_groups[paint_at].end_fragment <= first) paint_at++;
        if (!b->layout_bidi && b->fragments[first].run != UINT32_MAX &&
            (paint_at == b->paint_group_count || b->paint_groups[paint_at].first_fragment >= stop)) {
            const doc_render_run* run = &b->runs[b->fragments[first].run];
            if (run->first_fragment == first && run->end_fragment == stop &&
                !(b->fragments[stop - 1].flags & DOC_HYPHEN_USED))
                b->width = fmax(b->width, b->fragments[first].x + run->shape.fWidth);
        }
        if (b->lines_sorted) {
            double line_top = DBL_MAX, line_bottom = -DBL_MAX;
            for (i = first; i < stop; i++) {
                doc_fragment* f = &b->fragments[i];
                if (f->y < line_top) line_top = f->y;
                if (f->y + f->height > line_bottom) line_bottom = f->y + f->height;
            }
            if (b->line_count && line_top < b->lines[b->line_count - 1].bottom - .001)
                b->lines_sorted = 0;
            else if (doc_render_reserve((void**)&b->lines, &b->line_capacity,
                b->line_count + 1, sizeof(*b->lines)) != XUI_OK)
                b->lines_sorted = 0;
            else b->lines[b->line_count++] = (doc_render_line){first, stop, line_top, line_bottom};
        }
        top += fmax(1, ascent + descent); first = stop;
        if (first < end && !b->source_line) top += r->desc.fLineGap * r->desc.fZoom;
    }
    if (start == end) top += 20 * r->desc.fZoom;
    return top - y;
}
static double doc_layout_committed_width(const doc_render_block* b,
    size_t start, double previous)
{
    size_t i, end = b->lines[b->line_count - 1].first;
    for (i = start; i < end; i++) {
        const doc_fragment* fragment = &b->fragments[i];
        previous = fmax(previous, fragment->x + fragment->width);
    }
    return previous;
}
/* Lay out a long paragraph only through enough complete lines to cover the
 * viewport. The final shaped line is deliberately uncommitted: subsequent
 * text may still join it. Prefix and full layouts shape identical Unicode
 * segments, including their run-wide vertical metrics. */
int doc_layout_block_prefix(xui_document_renderer r, doc_render_block* b,
    double width, double required_bottom)
{
    doc_node* paragraph = doc_index_get(r->snapshot->state->index, b->node);
    doc_paragraph_projection projection = {0};
    doc_paragraph_projection* saved;
    uint64_t cutoff;
    double spacing, inner_width, shaped_height, committed;
    double previous_height = b->height;
    int result, reuse = b->reflow;
    if (!paragraph || (paragraph->kind != XUI_DOC_PARAGRAPH &&
        paragraph->kind != XUI_DOC_HEADING) ||
        (reuse && (!b->partial || !b->continuation)))
        return XUI_ERROR_NOT_FOUND;
    if (reuse) {
        projection = *b->continuation;
        free(b->continuation); b->continuation = NULL;
        cutoff = b->partial_cutoff;
    } else {
        result = doc_layout_projection_build(r, paragraph, &projection);
        if (result != XUI_OK) goto done;
        cutoff = projection.cut_count ? projection.cuts[0] : 0;
    }
    if (!projection.cut_count) { result = XUI_ERROR_NOT_FOUND; goto done; }
    /* A later discretionary insertion can change an already committed glyph
     * through arbitrary OpenType lookahead. Joint-result backends must settle
     * the complete marker pattern before publishing any paragraph prefix. */
    if(strstr(projection.text,"\xc2\xad")){
        xui_proxy_caps_t caps={0};
        if(r->proxy->getCaps){result=r->proxy->getCaps(r->proxy,&caps);if(result!=XUI_OK)goto done;}
        if((r->proxy->textShapeRangeMeasure && r->proxy->drawTextShapeRangeSpans) ||
            (caps.iCaps & XUI_PROXY_CAP_TEXT_RANGE)){result=XUI_ERROR_NOT_FOUND;goto done;}
    }
    inner_width = fmax(1, width - b->indent);
    spacing = ((paragraph->attrs->iFlags & XUI_DOC_SPACING_EXPLICIT) ||
        paragraph->attrs->fParagraphSpacing > 0 ? paragraph->attrs->fParagraphSpacing :
        r->desc.fParagraphGap) * r->desc.fZoom;
    if (!reuse) {
        doc_render_block_free(b);
        result=doc_layout_context_acquire(r,b,paragraph->id,projection.text,(size_t)projection.bytes,
            projection.spans,(size_t)projection.count,&projection.shaping);
        if(result!=XUI_OK)goto done;
    }
    b->layout_context=projection.shaping;
    b->layout_bidi = projection.bidi;
    b->lines_sorted = 1; b->width = 0; b->run_cursor = 0;
    result = doc_layout_paragraph_inlines(r, b, paragraph,
        &projection, 0, cutoff, inner_width);
    if (result != XUI_OK) goto done;
    if (reuse && b->run_cursor != b->run_count) { result = XUI_DOC_ERROR_STALE; goto done; }
    result = doc_layout_projection_apply(b, &projection);
    if (result != XUI_OK) goto done;
    result = doc_layout_paint_group(r, b, 0, b->fragment_count, inner_width);
    if (result != XUI_OK) goto done;
    shaped_height = doc_layout_lines(r, b, 0, b->indent, 0, inner_width,
        doc_effective_alignment(r->snapshot->state, paragraph), 1);
    if (b->layout_error != XUI_OK) { result = b->layout_error; goto done; }
    committed = b->lines_sorted && b->line_count > 1 ?
        b->lines[b->line_count - 1].top : 0;
    if (!committed) { result = XUI_ERROR_NOT_FOUND; goto done; }
    result = doc_layout_box(b, paragraph->id, paragraph->kind,
        paragraph->attrs->iBackgroundColor, b->indent, 0,
        inner_width, committed);
    if (result != XUI_OK) goto done;
    saved = malloc(sizeof(*saved));
    if (!saved) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    *saved = projection; memset(&projection, 0, sizeof(projection));
    b->continuation = saved;
    b->partial = 1; b->partial_bottom = committed;
    b->partial_committed_width = doc_layout_committed_width(b, 0, 0);
    b->partial_cutoff = cutoff;
    b->height = fmax(shaped_height,
        shaped_height * (double)saved->bytes / (double)cutoff) + spacing;
    b->reflow = 0; b->run_cursor = 0;
    b->dpi_scale = xuiGetVirtualDpi(r->context);
    b->shaped_revision = r->snapshot->revision;
    b->measured = b->ever_measured = 1;
    b->cache_bytes = doc_render_block_cache_bytes(b);
    r->stats.iMeasuredBlocks++; r->stats.iLayoutPasses++;
    r->stats.iFragments += b->fragment_count;
    b->layout_bidi = NULL; b->layout_context = NULL;
    result = committed > required_bottom ? XUI_OK :
        doc_layout_block_extend(r, b, width, required_bottom);
done:
    b->layout_bidi = NULL; b->layout_context = NULL;
    if (result != XUI_OK) {
        doc_render_block_free(b);
        b->height = previous_height;
    }
    doc_layout_projection_free(&projection);
    if (result == XUI_DOC_ERROR_STALE && reuse)
        return doc_layout_block_prefix(r, b, width, required_bottom);
    return result;
}
/* Advance a visible paragraph without reshaping or laying out committed
 * lines. The saved projection fixes the same cut and line-break decisions as
 * a later complete layout; only the unfinished line is laid out again. */
int doc_layout_block_extend(xui_document_renderer r, doc_render_block* b,
    double width, double required_bottom)
{
    doc_node* paragraph = doc_index_get(r->snapshot->state->index, b->node);
    doc_paragraph_projection* projection = b->continuation;
    size_t old_runs = b->run_count, old_fragments = b->fragment_count;
    size_t old_line_count = b->line_count, old_last_first;
    size_t old_paint_count = b->paint_group_count, old_paint_keep = 0;
    size_t old_seed_count = b->paint_seed_count;
    doc_render_paint_variant* old_variants=b->variants;
    doc_fragment* old_tail = NULL;
    size_t* old_visual = NULL;
    doc_render_line old_last_line;
    size_t cut_index = 0;
    uint64_t current = b->partial_cutoff;
    double old_height = b->height, old_bottom = b->partial_bottom;
    double old_width = b->width, inner_width, spacing;
    double committed_width = b->partial_committed_width;
    int old_object_dependent = b->object_dependent;
    int result = XUI_ERROR_NOT_FOUND;
    if (!b->partial || !projection || !paragraph || !b->lines_sorted ||
        old_line_count < 2 ||
        projection->bytes <= current || b->reflow ||
        b->shaped_revision != r->snapshot->revision) return XUI_ERROR_NOT_FOUND;
    old_last_first = b->lines[old_line_count - 1].first;
    old_last_line = b->lines[old_line_count - 1];
    old_tail = malloc((old_fragments - old_last_first) * sizeof(*old_tail));
    if (!old_tail) return XUI_ERROR_OUT_OF_MEMORY;
    memcpy(old_tail, b->fragments + old_last_first,
        (old_fragments - old_last_first) * sizeof(*old_tail));
    if (b->visual_fragments) {
        old_visual = malloc((old_fragments - old_last_first) * sizeof(*old_visual));
        if (!old_visual) { free(old_tail); return XUI_ERROR_OUT_OF_MEMORY; }
        memcpy(old_visual, b->visual_fragments + old_last_first,
            (old_fragments - old_last_first) * sizeof(*old_visual));
    }
    b->layout_context=projection->shaping;
    b->layout_bidi = projection->bidi;
    b->layout_error = XUI_OK;
    while (old_paint_keep < old_paint_count &&
        b->paint_groups[old_paint_keep].end_fragment <= old_last_first) old_paint_keep++;
    inner_width = fmax(1, width - b->indent);
    spacing = ((paragraph->attrs->iFlags & XUI_DOC_SPACING_EXPLICIT) ||
        paragraph->attrs->fParagraphSpacing > 0 ? paragraph->attrs->fParagraphSpacing :
        r->desc.fParagraphGap) * r->desc.fZoom;
    while (cut_index < projection->cut_count &&
        projection->cuts[cut_index] <= current) cut_index++;
    while (cut_index < projection->cut_count) {
        size_t chosen = cut_index;
        size_t resume_first = b->lines[b->line_count - 1].first;
        uint64_t target = current * 2;
        uint64_t cutoff;
        double resume_top = b->lines[b->line_count - 1].top;
        double shaped_height, committed;
        while (chosen + 1 < projection->cut_count &&
            projection->cuts[chosen] < target) chosen++;
        cutoff = projection->cuts[chosen];
        b->run_cursor = b->run_count;
        result = doc_layout_paragraph_inlines(r, b, paragraph, projection,
            current, cutoff, inner_width);
        if (result != XUI_OK) goto rollback;
        result = doc_layout_projection_apply(b, projection);
        if (result != XUI_OK) goto rollback;
        {
            size_t previous = b->paint_group_count, keep = previous;
            /* Keep the original tail until the entire extension succeeds so
             * a later shaping error can restore its old geometry and strings. */
            while (keep > old_paint_count &&
                b->paint_groups[keep - 1].end_fragment > resume_first) keep--;
            result = doc_layout_paint_group(r, b, resume_first, b->fragment_count, inner_width);
            if (result != XUI_OK) goto rollback;
            doc_layout_replace_paint_tail(b, keep, previous);
        }
        b->run_cursor = 0; b->line_count--;
        b->width = committed_width;
        shaped_height = resume_top + doc_layout_lines(r, b,
            resume_first, b->indent, resume_top, inner_width,
            doc_effective_alignment(r->snapshot->state, paragraph), 1);
        if (b->layout_error != XUI_OK) { result = b->layout_error; goto rollback; }
        if (!b->lines_sorted) goto abandoned;
        committed = b->lines_sorted && b->line_count > 1 ?
            b->lines[b->line_count - 1].top : 0;
        committed_width = doc_layout_committed_width(b,
            resume_first, committed_width);
        if (committed > required_bottom) {
            doc_layout_replace_paint_tail(b, old_paint_keep, old_paint_count);
            if (b->box_count)
                b->boxes[b->box_count - 1].height = committed;
            b->partial_bottom = committed;
            b->partial_committed_width = committed_width;
            b->partial_cutoff = cutoff;
            b->height = fmax(shaped_height,
                shaped_height * (double)projection->bytes / (double)cutoff) + spacing;
            b->cache_bytes = doc_render_block_cache_bytes(b);
            r->stats.iLayoutPasses++;
            r->stats.iFragments += b->fragment_count - old_fragments;
            free(old_tail); free(old_visual); b->layout_bidi = NULL; b->layout_context = NULL;
            return XUI_OK;
        }
        current = cutoff; cut_index = chosen + 1;
    }
    /* The caller will complete the block when no remaining cut can cover Y. */
abandoned:
    free(old_tail); free(old_visual); b->layout_bidi = NULL; b->layout_context = NULL;
    return XUI_ERROR_NOT_FOUND;
rollback:
    while (b->paint_seed_count > old_seed_count){
        doc_render_paint_seed* seed=&b->paint_seeds[--b->paint_seed_count];
        free(seed->widths);free(seed->offsets);xuiTextShapeFree(&seed->shape);
    }
    doc_layout_replace_paint_tail(b, old_paint_count, b->paint_group_count);
    while(b->variants!=old_variants){
        doc_render_paint_variant* variant=b->variants;
        b->variants=variant->next;doc_layout_variant_free(variant);
    }
    while (b->run_count > old_runs) {
        doc_render_run* run = &b->runs[--b->run_count];
        xuiTextShapeFree(&run->shape); free(run->text);
        memset(run, 0, sizeof(*run));
    }
    b->fragment_count = old_fragments; b->run_cursor = 0;
    memcpy(b->fragments + old_last_first, old_tail,
        (old_fragments - old_last_first) * sizeof(*old_tail));
    if (old_visual) memcpy(b->visual_fragments + old_last_first, old_visual,
        (old_fragments - old_last_first) * sizeof(*old_visual));
    free(old_tail); free(old_visual);
    b->line_count = old_line_count; b->lines_sorted = 1;
    b->lines[old_line_count - 1] = old_last_line;
    b->layout_bidi = NULL; b->layout_context = NULL; b->layout_error = XUI_OK;
    b->height = old_height; b->partial_bottom = old_bottom;
    b->width = old_width;
    b->object_dependent = old_object_dependent;
    b->cache_bytes = doc_render_block_cache_bytes(b);
    return result;
}
static int doc_layout_line_chunk(xui_document_renderer r, doc_render_block* b,
    doc_node* node, uint64_t start, uint64_t end)
{
    uint64_t bytes = end - start;
    doc_sequence* source = b->source_line ? doc_render_source(r) : node->text;
    char* text;
    int result;
    if (bytes > INT_MAX) return XUI_DOC_ERROR_LIMIT;
    text = malloc((size_t)bytes + 1);
    if (!text) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_read(source, start, text, bytes);
    if (result == XUI_OK) {
        text[bytes] = 0;
        result = doc_layout_run(r, b, node, 0, text, bytes, start);
    }
    free(text);
    return result;
}
static void doc_layout_line_size(xui_document_renderer r, doc_render_block* b,
    doc_node* node, double inner_width, double line_height)
{
    double origin = b->source_line ? 0 : b->indent + 8;
    double progress = (double)(b->line_cutoff - b->text_start);
    double total = (double)(b->text_end - b->text_start);
    double advance = fmax(0, b->line_exact_width - origin);
    double estimate = origin + advance * total / progress;
    double line_gap = b->source_line || b->slice_last ? 0 :
        r->desc.fLineGap * r->desc.fZoom;
    double spacing = b->source_line || !b->slice_last ? 0 :
        ((node->attrs->iFlags & XUI_DOC_SPACING_EXPLICIT) ||
        node->attrs->fParagraphSpacing > 0 ? node->attrs->fParagraphSpacing :
        r->desc.fParagraphGap) * r->desc.fZoom;
    b->line_partial = b->line_cutoff < b->text_end;
    b->width = b->line_partial ? fmax(b->line_exact_width, estimate) :
        b->line_exact_width;
    b->height = line_height + line_gap + spacing;
    if (b->box_count) {
        b->boxes[0].width = fmax(inner_width, b->width - b->indent);
        b->boxes[0].height = line_height + line_gap;
    }
}
/* A long, printable ASCII code or source line has one visual row. Shape only
 * its first chunk on the initial frame; X/offset queries append more chunks
 * to that same row. Other scripts keep whole-line shaping for correctness. */
int doc_layout_line_prefix(xui_document_renderer r, doc_render_block* b,
    double width)
{
    doc_node source_node = {0};
    doc_node* node;
    double inner_width, line_height, old_height;
    uint64_t cutoff;
    int result;
    if (!b->line_lazy || b->reflow) return XUI_ERROR_NOT_FOUND;
    if (b->source_line) {
        source_node.id = DOC_ROOT;
        source_node.kind = XUI_DOC_TEXT;
        source_node.attrs = &doc_source_line_attrs;
        node = &source_node;
    } else {
        node = doc_index_get(r->snapshot->state->index, b->node);
        if (!node || node->kind != XUI_DOC_CODE_BLOCK)
            return XUI_DOC_ERROR_STALE;
    }
    old_height = b->height;
    inner_width = fmax(1, width - b->indent);
    cutoff = b->text_start + 8192;
    doc_render_block_free(b);
    b->lines_sorted = 1;
    b->width = 0;
    result = doc_layout_line_chunk(r, b, node, b->text_start, cutoff);
    if (result != XUI_OK) goto fail;
    line_height = doc_layout_lines(r, b, 0,
        b->source_line ? 0 : b->indent + 8, 0,
        inner_width, 0, 0);
    if (!b->source_line) {
        result = doc_layout_box(b, node->id, node->kind,
            r->desc.iCodeBackground, b->indent, 0,
            inner_width, line_height);
        if (result != XUI_OK) goto fail;
    }
    b->line_cutoff = cutoff;
    b->line_exact_width = b->width;
    doc_layout_line_size(r, b, node, inner_width, line_height);
    b->dpi_scale = xuiGetVirtualDpi(r->context);
    b->shaped_revision = r->snapshot->revision;
    b->measured = b->ever_measured = 1;
    b->cache_bytes = doc_render_block_cache_bytes(b);
    r->stats.iMeasuredBlocks++;
    r->stats.iLayoutPasses++;
    r->stats.iFragments += b->fragment_count;
    return XUI_OK;
fail:
    doc_render_block_free(b);
    b->height = old_height;
    return result;
}
int doc_layout_line_extend(xui_document_renderer r, doc_render_block* b,
    double width, double required_x, uint64_t required_offset)
{
    doc_node source_node = {0};
    doc_node* node;
    size_t old_runs, old_fragments, i;
    uint64_t cutoff;
    double exact_width, old_width, inner_width, line_height;
    int result = XUI_OK;
    if (!b->line_partial || b->shaped_revision != r->snapshot->revision)
        return XUI_ERROR_NOT_FOUND;
    if (required_x <= b->line_exact_width &&
        required_offset <= b->line_cutoff) return XUI_OK;
    if (b->source_line) {
        source_node.id = DOC_ROOT;
        source_node.kind = XUI_DOC_TEXT;
        source_node.attrs = &doc_source_line_attrs;
        node = &source_node;
    } else {
        node = doc_index_get(r->snapshot->state->index, b->node);
        if (!node || node->kind != XUI_DOC_CODE_BLOCK)
            return XUI_DOC_ERROR_STALE;
    }
    old_runs = b->run_count;
    old_fragments = b->fragment_count;
    old_width = b->width;
    cutoff = b->line_cutoff;
    exact_width = b->line_exact_width;
    while (cutoff < b->text_end &&
        (required_x > exact_width || required_offset > cutoff)) {
        size_t first = b->fragment_count;
        uint64_t next = b->text_end - cutoff <= 8192 ?
            b->text_end : cutoff + 8192;
        result = doc_layout_line_chunk(r, b, node, cutoff, next);
        if (result != XUI_OK) goto rollback;
        for (i = first; i < b->fragment_count; i++)
            exact_width += b->fragments[i].width;
        cutoff = next;
    }
    if (cutoff == b->line_cutoff) return XUI_OK;
    inner_width = fmax(1, width - b->indent);
    b->line_count = 0;
    b->lines_sorted = 1;
    b->width = 0;
    line_height = doc_layout_lines(r, b, 0,
        b->source_line ? 0 : b->indent + 8, 0,
        inner_width, 0, 0);
    b->line_cutoff = cutoff;
    b->line_exact_width = b->width;
    doc_layout_line_size(r, b, node, inner_width, line_height);
    b->cache_bytes = doc_render_block_cache_bytes(b);
    r->stats.iLayoutPasses++;
    r->stats.iFragments += b->fragment_count - old_fragments;
    return XUI_OK;
rollback:
    while (b->run_count > old_runs) {
        doc_render_run* run = &b->runs[--b->run_count];
        xuiTextShapeFree(&run->shape);
        free(run->text);
        memset(run, 0, sizeof(*run));
    }
    b->fragment_count = old_fragments;
    b->run_cursor = 0;
    b->width = old_width;
    b->cache_bytes = doc_render_block_cache_bytes(b);
    return result;
}
static int doc_layout_node(xui_document_renderer, doc_render_block*, uint64_t, double, double, double, double*);
typedef struct doc_cell_layout {
    doc_node* node; size_t fragment_start, fragment_end, box_start, box_end;
    uint64_t row; unsigned column; double height;
} doc_cell_layout;
static int doc_layout_table(xui_document_renderer r, doc_render_block* b, doc_node* table, double x, double y, double width, double* height)
{
    doc_state* s = r->snapshot->state; uint32_t occupied[1024] = {0};
    uint64_t row, rows = doc_seq_size(table->children), i; unsigned columns = 0;
    doc_cell_layout* cells = NULL; size_t count = 0, capacity = 0; double *heights = NULL, *tops = NULL;
    int result = XUI_OK; double padding = 6 * r->desc.fZoom, positions[1025] = {0};
    /* Cell fragments are relocated after row heights and spans are known.
     * Their provisional line coordinates cannot serve as a visual Y index. */
    b->lines_sorted = 0;
    if (rows > SIZE_MAX / sizeof(double) - 1) return XUI_DOC_ERROR_LIMIT;
    heights = calloc((size_t)rows + 1, sizeof(*heights)); tops = calloc((size_t)rows + 1, sizeof(*tops));
    if (!heights || !tops) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    for (row = 0; row < rows; row++) {
        doc_node* row_node = doc_index_get(s->index, doc_seq_get_id(table->children, row)); unsigned column = 0, k;
        for (i = 0; i < doc_seq_size(row_node->children); i++) {
            doc_node* cell = doc_index_get(s->index, doc_seq_get_id(row_node->children, i)); doc_cell_layout* c;
            while (column < 1024 && occupied[column]) column++;
            if (cell->attrs->iColumnSpan > 1024 - column || cell->attrs->iRowSpan > rows - row) { result = XUI_DOC_ERROR_SCHEMA; goto done; }
            result = doc_render_reserve((void**)&cells, &capacity, count + 1, sizeof(*cells)); if (result != XUI_OK) goto done;
            c = &cells[count++]; memset(c, 0, sizeof(*c)); c->node = cell; c->row = row; c->column = column;
            for (k = 0; k < cell->attrs->iColumnSpan; k++) occupied[column + k] = cell->attrs->iRowSpan;
            column += cell->attrs->iColumnSpan; if (column > columns) columns = column;
        }
        for (k = 0; k < 1024; k++) if (occupied[k]) occupied[k]--;
    }
    if (table->column_widths) {
        double explicit_width = 0, auto_width; unsigned auto_columns = 0, column;
        for (column = 0; column < columns; column++) {
            float preferred = doc_table_column_width(table, column);
            if (preferred > 0) explicit_width += preferred * r->desc.fZoom;
            else auto_columns++;
        }
        auto_width = auto_columns ? fmax(24 * r->desc.fZoom,
            (width - explicit_width) / auto_columns) : 0;
        for (column = 0; column < columns; column++) {
            float preferred = doc_table_column_width(table, column);
            positions[column + 1] = positions[column] +
                (preferred > 0 ? preferred * r->desc.fZoom : auto_width);
        }
    } else {
        unsigned column;
        for (column = 0; column < columns; column++)
            positions[column + 1] = (double)(column + 1) * width / columns;
    }
    for (i = 0; i < count; i++) {
        doc_cell_layout* c = &cells[i]; double cy = padding; uint64_t child;
        c->fragment_start = b->fragment_count; c->box_start = b->box_count;
        if (!doc_seq_size(c->node->children)) {
            doc_fragment empty = {0}; empty.node = c->node->id; empty.kind = XUI_DOC_CELL; empty.run = UINT32_MAX;
            empty.x = x + positions[c->column] + padding; empty.y = padding;
            empty.height = 20 * r->desc.fZoom; empty.baseline = empty.height * .8;
            result = doc_layout_fragment(b, &empty); if (result != XUI_OK) goto done;
        }
        for (child = 0; child < doc_seq_size(c->node->children); child++) {
            double h;
            result = doc_layout_node(r, b, doc_seq_get_id(c->node->children, child), x + positions[c->column] + padding, cy,
                fmax(1, positions[c->column + c->node->attrs->iColumnSpan] - positions[c->column] - padding * 2), &h);
            if (result != XUI_OK) goto done;
            cy += h;
        }
        c->height = fmax(20 * r->desc.fZoom, cy) + padding;
        c->fragment_end = b->fragment_count; c->box_end = b->box_count;
        if (c->node->attrs->iRowSpan == 1 && c->height > heights[c->row]) heights[c->row] = c->height;
    }
    for (i = 0; i < count; i++) {
        doc_cell_layout* c = &cells[i]; double sum = 0; unsigned k;
        for (k = 0; k < c->node->attrs->iRowSpan; k++) sum += heights[c->row + k];
        if (sum < c->height) heights[c->row + c->node->attrs->iRowSpan - 1] += c->height - sum;
    }
    for (row = 0; row < rows; row++) tops[row + 1] = tops[row] + heights[row];
    for (i = 0; i < count; i++) {
        doc_cell_layout* c = &cells[i]; size_t f; double top = y + tops[c->row];
        for (f = c->fragment_start; f < c->fragment_end; f++) b->fragments[f].y += top;
        for (f = c->box_start; f < c->box_end; f++) b->boxes[f].y += top;
        result = doc_layout_box(b, c->node->id, XUI_DOC_CELL, c->node->attrs->iBackgroundColor, x + positions[c->column], top,
            positions[c->column + c->node->attrs->iColumnSpan] - positions[c->column],
            tops[c->row + c->node->attrs->iRowSpan] - tops[c->row]);
        if (result != XUI_OK) goto done;
        b->boxes[b->box_count - 1].table = table->id;
        b->boxes[b->box_count - 1].row = (uint32_t)c->row;
        b->boxes[b->box_count - 1].column = c->column;
    }
    *height = tops[rows]; if (x + positions[columns] > b->width) b->width = x + positions[columns];
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
        doc_paragraph_projection projection = {0};
        size_t first_paint_group = b->paint_group_count, paint_at;
        result = doc_layout_projection_build(r, n, &projection);
        if (result == XUI_OK) result=doc_layout_context_acquire(r,b,n->id,projection.text,(size_t)projection.bytes,
            projection.spans,(size_t)projection.count,&projection.shaping);
        if (result != XUI_OK) { doc_layout_projection_free(&projection); return result; }
        b->layout_context=projection.shaping;
        result = doc_layout_paragraph_inlines(r, b, n, &projection,
            0, projection.bytes, width);
        if (result != XUI_OK) { doc_layout_projection_free(&projection); return result; }
        if (first == b->fragment_count) {
            doc_fragment empty = {0}; empty.node = id; empty.kind = n->kind; empty.run = UINT32_MAX;
            empty.height = 20 * r->desc.fZoom; empty.baseline = empty.height * .8;
            result = doc_layout_fragment(b, &empty);
            if (result != XUI_OK) { doc_layout_projection_free(&projection); return result; }
        }
        b->layout_bidi = projection.bidi;
        result = doc_layout_projection_apply(b, &projection);
        if (result == XUI_OK) result = doc_layout_paint_group(r, b, first, b->fragment_count, width);
        if (result == XUI_OK) result = doc_layout_trailing_line(r, b, first);
        if (result == XUI_OK) {
            h = doc_layout_lines(r, b, first, x, y, width,
                doc_effective_alignment(r->snapshot->state, n), 1);
            result = b->layout_error;
        }
        b->layout_bidi = NULL; b->layout_context = NULL;
        doc_layout_projection_free(&projection);
        if (result != XUI_OK) return result;
        for (paint_at = first_paint_group; paint_at < b->paint_group_count;
            paint_at++) {
            const doc_render_paint_group* group = &b->paint_groups[paint_at];
            b->width = fmax(b->width, doc_render_group_x(b, group) + group->paint_width);
        }
        result = doc_layout_box(b, id, n->kind,
            n->attrs->iBackgroundColor, x, y, width, h);
    } else if (doc_text_kind(n->kind)) {
        xui_text_bidi bidi = NULL;
        result = doc_layout_inline(r, b, n, 0, width);
        if (result == XUI_OK) result = doc_layout_breaks(r, b, first, &bidi);
        b->layout_bidi = bidi;
        if (result == XUI_OK && (bidi || (b->layout_context && b->layout_context->scripts))) result = doc_layout_paint_group(r, b, first,
            b->fragment_count, n->kind == XUI_DOC_CODE_BLOCK ? DBL_MAX : width);
        if (result == XUI_OK) result = doc_layout_trailing_line(r, b, first);
        if (result == XUI_OK) {
            h = doc_layout_lines(r, b, first, x + (n->kind == XUI_DOC_CODE_BLOCK ? 8 : 0), y, width, 0, n->kind != XUI_DOC_CODE_BLOCK);
            result = b->layout_error;
        }
        b->layout_bidi = NULL; b->layout_context = NULL; xuiInternalTextBidiFree(bidi);
        if (result != XUI_OK) return result;
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
    *height = h + ((n->attrs->iFlags & XUI_DOC_SPACING_EXPLICIT) || n->attrs->fParagraphSpacing > 0 ?
        n->attrs->fParagraphSpacing : r->desc.fParagraphGap) * r->desc.fZoom;
    return result;
}
int doc_layout_block(xui_document_renderer r, doc_render_block* b, double width)
{
    int result, previous_object_dependent = b->object_dependent;
    xui_text_bidi row_bidi = NULL;
    if (b->reflow) { b->run_cursor = 0; b->context_cursor = 0; }
    else doc_render_block_free(b);
    b->lines_sorted = 1;
    b->object_dependent = 0;
    b->width = 0;
    if (b->virtual_gap) {
        doc_fragment f = {0}; f.node = DOC_ROOT; f.kind = XUI_DOC_ROOT; f.run = UINT32_MAX;
        f.start = f.end = doc_seq_size(doc_index_get(r->snapshot->state->index, DOC_ROOT)->children);
        f.height = b->height = 20 * r->desc.fZoom; f.baseline = f.height * .8;
        result = doc_layout_fragment(b, &f);
    } else if (b->admonition_title) {
        doc_node* quote = doc_index_get(r->snapshot->state->index, b->node);
        xui_doc_attributes_t attrs;
        xui_font_metrics_t metrics = {0};
        doc_fragment f = {0};
        if (!quote || quote->kind != XUI_DOC_QUOTE || !quote->info || !quote->info->size)
            return XUI_DOC_ERROR_STALE;
        attrs = doc_effective_text_attrs(r->snapshot->state, quote);
        attrs.iMarks |= XUI_DOC_BOLD;
        b->admonition_font = doc_layout_font(r, &attrs, 0);
        if (!b->admonition_font) return XUI_ERROR_UNSUPPORTED;
        if (r->proxy->fontGetMetrics)
            r->proxy->fontGetMetrics(r->proxy, b->admonition_font, &metrics);
        b->height = fmax(20 * r->desc.fZoom, metrics.fLineHeight) + 4 * r->desc.fZoom;
        b->width = width;
        f.node = quote->id; f.kind = XUI_DOC_QUOTE; f.run = UINT32_MAX;
        f.x = b->indent; f.height = b->height; f.baseline = f.height * .8;
        result = doc_layout_fragment(b, &f);
    } else if (b->code_slice) {
        doc_node* node = doc_index_get(r->snapshot->state->index, b->node);
        uint64_t bytes = b->text_end - b->text_start;
        double inner_width = fmax(1, width - b->indent), h, line_gap;
        char* text;
        if (!node || node->kind != XUI_DOC_CODE_BLOCK) return XUI_DOC_ERROR_STALE;
        if (bytes > INT_MAX) return XUI_DOC_ERROR_LIMIT;
        text = malloc((size_t)bytes + 1); if (!text) return XUI_ERROR_OUT_OF_MEMORY;
        result = doc_seq_read(node->text, b->text_start, text, bytes);
        if (result == XUI_OK) {
            text[bytes] = 0;
            result = doc_layout_run(r, b, node, 0, text, bytes, b->text_start);
        }
        free(text);
        if (result == XUI_OK) result = doc_layout_row_graphemes(r, b, &row_bidi);
        b->layout_bidi = row_bidi;
        if (result == XUI_OK && (row_bidi || (b->layout_context && b->layout_context->scripts))) result = doc_layout_paint_group(r, b, 0, b->fragment_count, DBL_MAX);
        if (result == XUI_OK && b->slice_last) result = doc_layout_trailing_line(r, b, 0);
        if (result == XUI_OK) {
            h = doc_layout_lines(r, b, 0, b->indent + 8, 0, inner_width, 0, 0);
            line_gap = b->slice_last ? 0 : r->desc.fLineGap * r->desc.fZoom;
            result = doc_layout_box(b, node->id, node->kind, r->desc.iCodeBackground,
                b->indent, 0, inner_width, h + line_gap);
            b->height = h + line_gap + (b->slice_last ?
                ((node->attrs->iFlags & XUI_DOC_SPACING_EXPLICIT) || node->attrs->fParagraphSpacing > 0 ?
                    node->attrs->fParagraphSpacing : r->desc.fParagraphGap) * r->desc.fZoom : 0);
        }
    } else if (b->source_line) {
        doc_node source_node = {0}; uint64_t bytes = b->source_end - b->source_start; char* text;
        if (bytes > INT_MAX) return XUI_DOC_ERROR_LIMIT;
        text = malloc((size_t)bytes + 1); if (!text) return XUI_ERROR_OUT_OF_MEMORY;
        doc_seq_read(doc_render_source(r), b->source_start, text, bytes); text[bytes] = 0;
        source_node.id = DOC_ROOT; source_node.kind = XUI_DOC_TEXT; source_node.attrs = &doc_source_line_attrs;
        result = doc_layout_run(r, b, &source_node, 0, text, bytes, b->source_start); free(text);
        if (result == XUI_OK) result = doc_layout_row_graphemes(r, b, &row_bidi);
        b->layout_bidi = row_bidi;
        if (result == XUI_OK && (row_bidi || (b->layout_context && b->layout_context->scripts))) result = doc_layout_paint_group(r, b, 0, b->fragment_count, DBL_MAX);
        if (result == XUI_OK) result = doc_layout_trailing_line(r, b, 0);
        if (result == XUI_OK) {
            b->height = doc_layout_lines(r, b, 0, 0, 0, width, 0, 0);
        }
    } else result = doc_layout_node(r, b, b->node, b->indent, 0, fmax(1, width - b->indent), &b->height);
    b->layout_bidi = NULL; b->layout_context = NULL; xuiInternalTextBidiFree(row_bidi);
    if (result == XUI_OK) result = b->layout_error;
    if (result == XUI_OK && b->reflow && (b->run_cursor != b->run_count ||
        b->context_cursor != b->context_count)) result = XUI_DOC_ERROR_STALE;
    if (result == XUI_DOC_ERROR_STALE && b->reflow) {
        doc_render_block_free(b);
        return doc_layout_block(r, b, width);
    }
    if (result != XUI_OK) {
        doc_render_block_free(b);
        b->object_dependent = previous_object_dependent;
        return result;
    }
    b->reflow = 0;
    b->run_cursor = 0;
    b->dpi_scale = xuiGetVirtualDpi(r->context);
    b->shaped_revision = r->snapshot->revision;
    b->measured = b->ever_measured = 1; r->stats.iMeasuredBlocks++; r->stats.iLayoutPasses++;
    b->cache_bytes = doc_render_block_cache_bytes(b);
    r->stats.iFragments += b->fragment_count; return XUI_OK;
}

#endif
