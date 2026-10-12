#ifndef UIDESIGN_XUI_H
#define UIDESIGN_XUI_H
#include "uidesign.h"
#include "xui.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct uid_editor uid_editor;
typedef struct uid_preview uid_preview;
typedef struct uid_runtime_event {
    uid_id node;
    const char *name, *event_id, *handler;
    uid_value value;
    xui_widget sender;
} uid_runtime_event;
/* The runtime API is also used by generated code; it contains no designer. */
typedef struct uid_runtime_host {
    size_t size;
    void *user;
    void (*dispatch)(const uid_runtime_event *, void *user);
    xui_font (*font)(float size, void *user);
} uid_runtime_host;
typedef void (*uid_emit_event_proc)(const char *event_id, uid_value, void *emitter);
typedef struct uid_xui_adapter {
    size_t size;
    const char *type;
    int (*create)(xui_context, xui_font, const uid_document *, uid_id, xui_widget *, void *);
    int (*apply)(xui_widget, const uid_document *, uid_id, float zoom, void *);
    xui_widget (*child_host)(xui_widget, const char *slot, void *);
    void *user;
    /* A custom adapter has no C export unless it explicitly opts into one of
       the documented built-in runtime contracts. Otherwise export fails. */
    const char *c_runtime_type;
    /* Optional custom runtime wiring. Store emitter only until widget destroy.
       No business code is executed on the design surface. */
    int (*bind_events)(xui_widget, uid_emit_event_proc, void *emitter, void *user);
} uid_xui_adapter;
int uidXuiRegisterAdapter(uid_context *, const uid_xui_adapter *);
int uidXuiRegisterBuiltins(uid_context *);
typedef struct uid_editor_desc {
    size_t size;
    uint32_t version;
    xui_context xui;
    xui_widget parent;
    xui_font font;
    float font_size;
    void *user;
    void (*invalidate)(void *);
    /* Called outside a document transaction. Must return an idempotent,
       compatible handler identifier; empty output cancels without binding. */
    int (*ensure_handler)(uid_document *, uid_id, const uid_event_desc *, const char *existing,
                          char *out, size_t capacity, void *);
    void (*show_code)(const char *handler, void *);
    int (*validate_handler)(const uid_event_desc *, const char *handler, void *);
    void (*status)(const char *, void *);
    xui_font (*resolve_font)(float, void *);
    /* Coalesced view-only notification on uidEditorUpdate, never dirty/undo. */
    void (*selection_changed)(uid_editor *, const uid_id *, int count, void *);
    /* Optional synchronous custom picker for UID_CUSTOM_EDITOR properties.
       A zero-kind output cancels. String output must remain valid until the
       API returns; it is immediately copied into the model. */
    int (*edit_property)(uid_document *, const uid_id *, int count, const uid_property_desc *,
                         xui_rect_t anchor, uid_value *out, void *);
} uid_editor_desc;
int uidEditorCreate(uid_context *, uid_document *, const uid_editor_desc *, uid_editor **out);
void uidEditorDestroy(uid_editor *);
int uidEditorUpdate(uid_editor *); /* host calls before its ONE XUI layout/render */
int uidEditorFlushPendingEdits(uid_editor *);
xui_widget uidEditorRoot(const uid_editor *);
xui_widget uidEditorCanvas(const uid_editor *);
xui_widget uidEditorPropertyGrid(const uid_editor *);
xui_widget uidEditorTree(const uid_editor *);
xui_widget uidEditorNodeWidget(const uid_editor *, uid_id);
int uidEditorSelect(uid_editor *, uid_id, int extend);
int uidEditorSelection(const uid_editor *, uid_id *out, int capacity);
int uidEditorSetTool(uid_editor *, const char *type); /* NULL = pointer */
int uidEditorEditDefaultEvent(uid_editor *, uid_id);
int uidEditorShowEvents(uid_editor *, int events);
int uidEditorSetZoom(uid_editor *, float zoom);
int uidEditorAlign(uid_editor *, int vertical); /* left/top; one transaction */
int uidEditorSetSelectionProperty(uid_editor *, const char *, uid_value);
int uidEditorEditProperty(uid_editor *, const char *property);
int uidPreviewCreate(uid_context *, const uid_snapshot *, xui_context, xui_widget parent, xui_font,
                     const uid_runtime_host *, uid_preview **out);
void uidPreviewDestroy(uid_preview *);
xui_widget uidPreviewRoot(const uid_preview *);
xui_widget uidPreviewNodeWidget(const uid_preview *, uid_id);
#ifdef __cplusplus
}
#endif
#endif
