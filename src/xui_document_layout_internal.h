#ifndef XUI_DOCUMENT_LAYOUT_INTERNAL_H
#define XUI_DOCUMENT_LAYOUT_INTERNAL_H
#include "xui_xrt_port.h"
#include "xui_document_internal.h"
#include "../xui_document_ui.h"

typedef struct doc_render_run {
    uint64_t node;
    char* text;
    uint64_t bytes;
    uint64_t base_offset;
    xui_doc_attributes_t attrs;
    xui_font font;
} doc_render_run;
typedef struct doc_fragment {
    uint64_t node, start, end;
    uint32_t kind, run, flags;
    double x, y, width, height, baseline;
} doc_fragment;
typedef struct doc_box { uint64_t node; uint32_t kind, color; double x, y, width, height; } doc_box;
typedef struct doc_render_block {
    uint64_t node, marker;
    uint64_t source_start, source_end;
    double indent, height, width;
    char marker_text[32];
    int measured, virtual_gap, source_line;
    doc_render_run* runs; size_t run_count, run_capacity;
    doc_fragment* fragments; size_t fragment_count, fragment_capacity;
    doc_box* boxes; size_t box_count, box_capacity;
} doc_render_block;
typedef struct doc_sized_font { xui_font source, font; float size; } doc_sized_font;
struct xui_document_renderer_t {
    xui_context context;
    xui_proxy_t proxy_storage;
    xui_proxy proxy;
    xui_document_snapshot snapshot;
    xui_doc_renderer_desc_t desc;
    doc_render_block* blocks;
    size_t count, capacity;
    double* heights;
    xmap block_index;
    double width, viewport_top, viewport_height;
    uint32_t mode;
    uint64_t active_source, live_start, live_end;
    int live_fallback;
    doc_sized_font* fonts; size_t font_count, font_capacity;
    xui_doc_renderer_stats_t stats;
};
int doc_render_reserve(void**, size_t*, size_t, size_t);
void doc_render_block_free(doc_render_block*);
int doc_layout_block(xui_document_renderer, doc_render_block*, double);
double doc_render_height_before(xui_document_renderer, size_t);
void doc_render_set_height(xui_document_renderer, size_t, double);
size_t doc_render_block_at(xui_document_renderer, double);
int doc_render_materialize(xui_document_renderer, size_t);
#endif
