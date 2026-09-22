#ifndef XUI_DOCUMENT_VIEW_INTERNAL_H
#define XUI_DOCUMENT_VIEW_INTERNAL_H
#include "xui_document_layout_internal.h"
typedef struct doc_view_data {
    xui_widget widget;
    xui_document document;
    xui_document_renderer renderer;
    xui_doc_view_desc_t desc;
    xui_doc_range_t selection;
    uint64_t subscription;
    double scroll_x, scroll_y;
    int dragging, needs_sync, error;
    uint64_t pressed_node;
    xui_doc_change_proc onChanged; /* Subtype hook; called after base selection/layout mapping. */
    void (*onProjectionChanged)(xui_widget);
    doc_plain_projection edit_projection;
    xui_document_snapshot edit_snapshot;
    unsigned pending_edit_events;
} doc_view_data;
doc_view_data* doc_view_get(xui_widget);
int doc_view_sync(doc_view_data*);
int doc_view_layout(doc_view_data*);
int doc_view_event(xui_widget, const xui_event_t*, void*);
int doc_view_init(xui_widget, void*, const void*, void*);
void doc_view_destroy(xui_widget, void*, void*);
int doc_view_measure(xui_widget, xui_vec2_t, xui_vec2_t*, void*);
int doc_view_render(xui_widget, xui_draw_context, uint32_t, void*);
int doc_view_update(xui_widget, float, void*);
int doc_view_register_edit(xui_widget, int);
void doc_view_invalidate_edit(doc_view_data*);
#endif
