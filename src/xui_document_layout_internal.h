#ifndef XUI_DOCUMENT_LAYOUT_INTERNAL_H
#define XUI_DOCUMENT_LAYOUT_INTERNAL_H
#include "xui_xrt_port.h"
#include "xui_document_internal.h"
#include "../xui_document_ui.h"
#include "xui_text_bidi.h"

/* The following fragment belongs to the same paragraph grapheme. Text-node
 * boundaries and a scalar-only shaper must not expose an interior caret. */
#define DOC_GRAPHEME_JOIN UINT32_C(0x10000000)
#define DOC_GRAPHEME_MEMBER UINT32_C(0x08000000)
#define DOC_LINE_CONTROL UINT32_C(0x04000000)
#define DOC_TRAILING_LINE UINT32_C(0x02000000)
#define DOC_FORCED_BREAK UINT32_C(0x80000000)
#define DOC_FORMATTED UINT32_C(0x01000000)
#define DOC_INVISIBLE UINT32_C(0x00800000)
#define DOC_SOFT_HYPHEN UINT32_C(0x00400000)
#define DOC_HYPHEN_USED UINT32_C(0x00200000)
#define DOC_CONTEXT_LAYOUT UINT32_C(0x00100000)
#define DOC_CONTEXT_LINE_END UINT32_C(0x00080000)

typedef struct doc_render_run {
    uint64_t node;
    char* text;
    uint64_t bytes;
    uint64_t base_offset;
    size_t first_fragment, end_fragment;
    xui_doc_attributes_t attrs;
    xui_font font;
    xui_text_shape_t shape;
    double hyphen_width;
    int hyphen_measured;
} doc_render_run;
typedef struct doc_fragment {
    uint64_t node, start, end;
    uint32_t kind, run, flags;
    uint32_t break_style_run; /* Only explicit SoftBreak/HardBreak atoms use this font/marks run. */
    double x, y, width, height, baseline;
    double hyphen_width;
    size_t bidi_start, bidi_end; /* Private paragraph UTF-8 projection. */
    uint8_t bidi_level, bidi_active, bidi_base;
} doc_fragment;
static inline uint32_t doc_fragment_paint_run(const doc_fragment* fragment)
{
    return fragment->kind == XUI_DOC_SOFT_BREAK ? fragment->break_style_run : fragment->run;
}
static inline uint64_t doc_fragment_paint_bytes(const doc_fragment* fragment)
{
    return fragment->kind == XUI_DOC_SOFT_BREAK ? 1 : fragment->end - fragment->start;
}
typedef struct doc_render_line {
    size_t first, end; /* Contiguous fragment range on one visual line. */
    double top, bottom;
} doc_render_line;
typedef struct doc_shape_context {
    uint64_t node, source_bytes;
    char* text;
    uint32_t* source_map; /* Source-stream boundary -> displayed UTF-8 boundary. */
    unsigned char* scripts; /* One cached Script_Extensions ID per displayed byte. */
    uint32_t bytes;
    char* hyphen_text; /* One reusable insertion buffer; base text stays immutable. */
    uint32_t hyphen_at, hyphen_borrows;
    uint64_t hyphen_copied; /* Bytes copied/moved, for internal complexity checks. */
    int context_supported;
} doc_shape_context;
typedef struct doc_hyphen_lease {
    doc_shape_context* context;
    char* owned; /* A nested different insertion cannot mutate an active borrow. */
    const char* text;
} doc_hyphen_lease;
int doc_context_hyphen_borrow(doc_shape_context*, uint32_t, doc_hyphen_lease*);
void doc_context_hyphen_release(doc_hyphen_lease*);
typedef struct doc_render_paint_variant doc_render_paint_variant;
typedef struct doc_render_paint_group {
    size_t first_fragment, end_fragment;
    size_t origin_fragment; /* First actual text; empty styled nodes retain their caret metrics. */
    char* text;
    doc_shape_context* context;
    uint32_t hyphen_offset;
    int has_hyphen_context; /* Recipe, not a retained full-paragraph copy. */
    uint32_t context_offset, context_bytes, script, input_caps;
    uint32_t* display_ends; /* Optional projected byte ends, one per fragment. */
    uint64_t bytes;
    xui_font font;
    double paint_width;
    double line_height, ascent;
    uint32_t decorations, vertical_marks;
    uint8_t bidi_level;
    size_t shared_seed; /* Seed index + 1; zero uses synchronous text painting. */
    doc_render_paint_variant* variant; /* Borrowed complete insertion context. */
    size_t variant_span; /* Variant seed + 1, or zero for an owned row result. */
    int range_input; /* Basic text callbacks select the complete variant span. */
    uint32_t shape_start, shape_end; /* Byte range in the seed's shaping input. */
    xui_text_shape_t shape; /* Owns a final row result when not sharing a seed. */
} doc_render_paint_group;
/* Width-independent seed advances for one exact, uniformly styled span.
 * Retained run/fragment identities make these reusable when only width changes. */
typedef struct doc_render_paint_seed {
    size_t first_fragment, end_fragment;
    xui_font font;
    uint32_t vertical_marks;
    double* widths;
    uint8_t bidi_level;
    xui_text_shape_t shape; /* Owns a joint result when source_run is zero. */
    uint32_t source_run; /* Run index + 1 for borrowing an already cached run. */
    uint32_t* offsets; /* Fragment boundaries in the complete shape input. */
} doc_render_paint_seed;
/* One immutable selected-marker pattern owns the complete displayed context
 * and its uniformly styled glyph spans. Published rows borrow it together. */
struct doc_render_paint_variant {
    doc_render_paint_variant* next;
    doc_shape_context* context;
    size_t first_fragment,end_fragment;
    char* text;
    uint32_t bytes;
    uint8_t* hyphens;
    uint32_t* offsets; /* Global displayed byte boundaries, one per fragment. */
    uint32_t* bidi_offsets; /* Global raw variant boundaries retain controls. */
    xui_text_bidi bidi;
    doc_render_paint_seed* spans;
    size_t span_count,span_capacity;
    size_t* span_map;
};
typedef struct doc_box {
    uint64_t node, table;
    uint32_t kind, color, row, column;
    double x, y, width, height;
} doc_box;
typedef struct doc_paragraph_projection doc_paragraph_projection;
typedef struct doc_render_block {
    uint64_t node, marker, shaped_revision;
    uint64_t source_start, source_end;
    uint64_t text_start, text_end;
    double indent, height, width;
    float dpi_scale;
    char marker_text[32];
    xui_font admonition_font; /* Resolved while measuring a decorative quote title. */
    int measured, ever_measured, virtual_gap, source_line, code_slice, slice_last, reflow;
    int admonition_title;
    int line_lazy, line_partial; /* A printable ASCII code line has exact height and a horizontal prefix. */
    int height_uncertain; /* SOURCE text needing shaping to settle its row height. */
    uint64_t line_cutoff;
    double line_exact_width;
    int partial; /* Visible prefix is exact; the rest of height is estimated. */
    double partial_bottom; /* Exclusive bottom of committed complete lines. */
    double partial_committed_width;
    uint64_t partial_cutoff;
    doc_paragraph_projection* continuation; /* Cached Unicode break stream. */
    doc_shape_context** contexts; size_t context_count, context_capacity, context_cursor;
    doc_shape_context* layout_context; /* Borrowed while measuring one paragraph. */
    int object_dependent; /* Retained after cache eviction for resource-height anchoring. */
    size_t slice_count; /* Number of contiguous slices, stored on the first slice. */
    doc_render_run* runs; size_t run_count, run_capacity;
    size_t run_cursor;
    doc_fragment* fragments; size_t fragment_count, fragment_capacity;
    size_t* visual_fragments; size_t visual_capacity; /* Per-line permutation of logical fragments. */
    xui_text_bidi layout_bidi; /* Borrowed only while laying out one paragraph. */
    int layout_error;
    doc_render_line* lines; size_t line_count, line_capacity;
    doc_render_paint_group* paint_groups; size_t paint_group_count, paint_group_capacity;
    doc_render_paint_seed* paint_seeds; size_t paint_seed_count, paint_seed_capacity;
    doc_render_paint_variant* variants;
    int lines_sorted; /* False for overlapping cell lines or index OOM. */
    doc_box* boxes; size_t box_count, box_capacity;
    size_t cache_bytes;
} doc_render_block;
static inline const xui_text_shape_t* doc_render_seed_shape(const doc_render_block* block,
    const doc_render_paint_seed* seed)
{
    return seed->source_run ? &block->runs[seed->source_run-1].shape : &seed->shape;
}
static inline int doc_render_group_item(const doc_render_paint_group* group,
    const char* language, xui_text_item_t* output, doc_hyphen_lease* lease)
{
    xui_text_item_t item={0};
    int result;
    memset(lease,0,sizeof(*lease));
    item.iSize=sizeof(item);item.pFont=group->font;item.sText=group->text;
    item.iTextSize=(int)group->bytes;
    item.sLanguage=language;
    item.iFlags=XUI_TEXT_SHAPE_DEFAULT|((group->bidi_level&1)?XUI_TEXT_SHAPE_RTL:0);
    /* Context is independent of whether the paragraph needs a script map.
     * Reduced backends may omit derived inputs; explicit language/RTL remain
     * requested and must be supported or rejected. */
    if(group->context){
        if(group->input_caps & XUI_PROXY_CAP_TEXT_CONTEXT){
            item.sContext=group->variant?group->variant->text:group->context->text;
            if(!group->variant && group->has_hyphen_context){
                result=doc_context_hyphen_borrow(group->context,group->hyphen_offset,lease);
                if(result!=XUI_OK)return result;
                item.sContext=lease->text;
            }
            item.iContextSize=(int)group->context_bytes;item.iContextOffset=(int)group->context_offset;
        }
        if(group->input_caps & XUI_PROXY_CAP_TEXT_SCRIPT)item.iScript=group->script;
    }
    if(group->range_input && !group->shared_seed){
        const doc_render_paint_variant* variant=group->variant;
        const doc_render_paint_seed* span;
        uint32_t offset;
        if(!variant || !group->variant_span || group->variant_span>variant->span_count)return XUI_ERROR_INVALID_STATE;
        span=&variant->spans[group->variant_span-1];
        offset=variant->offsets[span->first_fragment-variant->first_fragment];
        item.sText=variant->text+offset;
        item.iTextSize=(int)span->offsets[span->end_fragment-span->first_fragment];
        if(item.sContext)item.iContextOffset=(int)offset;
        item.iFlags|=XUI_TEXT_SHAPE_RANGE;
        item.iRangeStart=(int)group->shape_start;item.iRangeEnd=(int)group->shape_end;
        item.pShape=span->shape.pPaint?&span->shape:NULL;
    }
    *output=item;return XUI_OK;
}
static inline int doc_render_seed_range_item(const doc_render_block* block,
    const doc_render_paint_group* group,const char* language,xui_text_item_t* item,doc_hyphen_lease* lease)
{
    const doc_render_paint_seed* seed;uint32_t offset;int result;
    result=doc_render_group_item(group,language,item,lease);if(result!=XUI_OK)return result;
    if(!group->shared_seed || group->shared_seed>block->paint_seed_count)return XUI_ERROR_INVALID_STATE;
    seed=&block->paint_seeds[group->shared_seed-1];
    if(group->context){
        size_t source=block->fragments[seed->first_fragment].bidi_start;
        if(source>group->context->source_bytes)return XUI_ERROR_INVALID_STATE;
        offset=group->context->source_map?group->context->source_map[source]:(uint32_t)source;
        item->sText=group->context->text+offset;
        if(item->sContext)item->iContextOffset=(int)offset;
    }else if(seed->source_run)item->sText=block->runs[seed->source_run-1].text;
    else return XUI_ERROR_UNSUPPORTED;
    item->iTextSize=(int)seed->offsets[seed->end_fragment-seed->first_fragment];
    item->iFlags|=XUI_TEXT_SHAPE_RANGE;item->iRangeStart=(int)group->shape_start;item->iRangeEnd=(int)group->shape_end;
    item->pShape=doc_render_seed_shape(block,seed);
    if(!item->pShape->pPaint)item->pShape=NULL;
    return XUI_OK;
}
static inline double doc_render_group_x(const doc_render_block* block,
    const doc_render_paint_group* group)
{
    size_t first = group->first_fragment;
    if (group->bidi_level & 1) {
        first = group->end_fragment - 1;
        while (first > group->first_fragment &&
            (block->fragments[first - 1].flags & DOC_GRAPHEME_JOIN)) first--;
    }
    return block->fragments[first].x;
}
typedef struct doc_sized_font { xui_font source, font; float size; } doc_sized_font;
typedef struct doc_source_row doc_source_row;
struct xui_document_renderer_t {
    xui_context context;
    uint64_t resource_registry_generation;
    float dpi_scale;
    int has_named_images;
    int uses_default_font;
    int freeze_dynamic_refresh; /* Capture the last layout during a metric transition. */
    xui_proxy_t proxy_storage;
    xui_proxy proxy;
    xui_document_snapshot snapshot;
    xui_document_prepare input;
    xui_doc_renderer_desc_t desc;
    doc_render_block* blocks;
    doc_source_row* source_rows;
    uint64_t source_row_serial;
    size_t source_height_hint;
    int source_height_hint_valid;
    size_t count, capacity;
    double* heights;
    xmap block_index;
    double width, viewport_top, viewport_height;
    double size_width;
    int size_exact, size_dirty;
    size_t partial_hint;
    int partial_hint_valid;
    uint32_t mode;
    uint64_t active_source, live_start, live_end;
    uint64_t live_active_node, live_base_end;
    size_t live_first, live_rows;
    int live_fallback, live_pending_full;
    doc_sized_font* fonts; size_t font_count, font_capacity;
    size_t cache_bytes;
    xui_doc_renderer_stats_t stats;
    xui_doc_table_selection_t table_selection;
    uint32_t table_selection_color;
    const xui_doc_range_t* find_matches; /* Borrowed for one Draw call. */
    uint64_t find_count, find_active;
    uint32_t find_result_color, find_active_color;
};
int doc_render_reserve(void**, size_t*, size_t, size_t);
void doc_render_block_free(doc_render_block*);
void doc_render_block_reflow(doc_render_block*);
size_t doc_render_block_cache_bytes(const doc_render_block*);
int doc_render_image_measure(xui_document_renderer, doc_node*, double, xui_vec2_t*);
int doc_render_image_draw(xui_document_renderer, doc_node*, xui_draw_context, xui_rect_t);
int doc_layout_block(xui_document_renderer, doc_render_block*, double);
/* Empty styled text is drawable text; standalone mandatory controls and objects
 * are layout boundaries and never enter a text paint string. */
int doc_render_fragment_text(const doc_render_block*, const doc_fragment*);
int doc_layout_block_prefix(xui_document_renderer, doc_render_block*, double,
    double); /* NOT_FOUND when a bounded prefix cannot cover the requested Y. */
int doc_layout_block_extend(xui_document_renderer, doc_render_block*, double,
    double); /* NOT_FOUND when the remaining suffix needs complete layout. */
int doc_layout_line_prefix(xui_document_renderer, doc_render_block*, double);
int doc_layout_line_extend(xui_document_renderer, doc_render_block*, double,
    double, uint64_t); /* Cover a document X or code-byte offset. */
int doc_render_cover_x(xui_document_renderer, double, double, double);
double doc_render_height_before(xui_document_renderer, size_t);
void doc_render_set_height(xui_document_renderer, size_t, double);
size_t doc_render_block_at(xui_document_renderer, double);
size_t doc_render_find(xui_document_renderer, uint64_t);
int doc_render_inline_style_splits(xui_document_change_set);
int doc_render_metric_style_change(xui_document_change_set);
int doc_render_materialize(xui_document_renderer, size_t);
doc_sequence* doc_render_source(xui_document_renderer);
int doc_render_live_position_source(xui_document_renderer, const xui_doc_position_t*, uint64_t*);
int doc_render_compare(xui_document_renderer, const xui_doc_position_t*, const xui_doc_position_t*, int*);
int doc_render_copy_range(xui_document_renderer, const xui_doc_range_t*, char**, uint64_t*);
/* Visual arrows consume intact rendered graphemes. Logical deletion/word
 * navigation keep the original edit projection. NOT_FOUND selects that path;
 * logical_right reflects paragraph direction at a visual row boundary. */
int doc_render_visual_move(xui_document_renderer, const xui_doc_position_t*, int,
    xui_doc_position_t*, int*);
int doc_render_position_may_bidi(xui_document_renderer, const xui_doc_position_t*);
#endif
