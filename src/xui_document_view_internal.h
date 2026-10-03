#ifndef XUI_DOCUMENT_VIEW_INTERNAL_H
#define XUI_DOCUMENT_VIEW_INTERNAL_H
#include "xui_document_layout_internal.h"
typedef struct doc_view_data {
    xui_widget widget;
    xui_document document;
    xui_document_renderer renderer;
    xui_document_prepare input; /* Borrowed from the editor coordinator. */
    xui_doc_view_desc_t desc;
    xui_doc_renderer_desc_t base_renderer; /* Unthemed fallback palette and metrics. */
    int uses_default_font;
    xui_doc_range_t selection;
    char* find_pattern;
    uint64_t find_pattern_bytes;
    xui_doc_range_t find_scope;
    xui_doc_range_t* find_matches;
    uint64_t find_count, find_active, find_identity, find_revision;
    uint32_t find_mode, find_flags;
    int find_valid, find_scope_active;
    xui_doc_table_selection_t table_selection;
    xui_doc_cell_hit_t table_anchor, table_focus;
    uint64_t subscription;
    double scroll_x, scroll_y;
    double drag_pointer_x, drag_pointer_y;
    int drag_pointer_active;
    int dragging, table_selecting, needs_sync, error;
    uint64_t pressed_node;
    xui_doc_change_proc onChanged; /* Subtype hook; called after base selection/layout mapping. */
    void (*onProjectionChanged)(xui_widget);
    int (*onBeforeTransition)(xui_widget);
    int (*onBeforeSelection)(xui_widget);
    int replaying_input; /* Replayed pointer events must not reacquire OS focus/capture. */
    doc_plain_projection edit_projection;
    xui_document_snapshot edit_snapshot;
    xui_doc_node_id* accessible_ids;
    char** accessible_text;
    uint64_t accessible_count, accessible_identity, accessible_revision;
    unsigned pending_edit_events;
} doc_view_data;
doc_view_data* doc_view_get(xui_widget);
int doc_view_sync(doc_view_data*);
void doc_view_find_invalidate(doc_view_data*);
void doc_view_find_query_clear(doc_view_data*);
int doc_view_refresh_resources(doc_view_data*);
uint32_t doc_view_style_color(xui_widget, const char*, uint32_t);
void doc_view_resolve_style(doc_view_data*);
int doc_view_paint_background(doc_view_data*, xui_draw_context, xui_rect_t);
int doc_view_paint_border(doc_view_data*, xui_draw_context, xui_rect_t);
int doc_view_layout(doc_view_data*);
int doc_view_set_selection(doc_view_data*, const xui_doc_range_t*);
int doc_view_set_table_selection(doc_view_data*, const xui_doc_table_selection_t*);
int doc_view_extend_table_selection(doc_view_data*, int key);
int doc_view_object_range(doc_view_data*, uint64_t, xui_doc_range_t*);
int doc_view_event(xui_widget, const xui_event_t*, void*);
int doc_view_init(xui_widget, void*, const void*, void*);
void doc_view_destroy(xui_widget, void*, void*);
int doc_view_measure(xui_widget, xui_vec2_t, xui_vec2_t*, void*);
int doc_view_render(xui_widget, xui_draw_context, uint32_t, void*);
int doc_view_update(xui_widget, float, void*);
int doc_view_drag_autoscroll(doc_view_data*, double);
int doc_view_register_edit(xui_widget, int);
void doc_view_invalidate_edit(doc_view_data*);
void doc_view_accessible_clear(doc_view_data*);
#endif
