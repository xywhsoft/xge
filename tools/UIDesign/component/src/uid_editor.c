#include "uid_xui_internal.h"
#include <ctype.h>

typedef struct ui_action {
    struct uid_editor *editor;
    int command;
    const char *type;
    xui_widget widget;
} ui_action;
struct uid_editor {
    uid_context *context;
    uid_document *document;
    uid_editor_desc host;
    xui_widget root, canvas, overlay, tree, grid, heading;
    uid_preview *scene;
    uid_runtime_host runtime;
    uint64_t subscription;
    ui_action actions[80];
    int action_count, tool_count;
    xui_tree_view_node_t tree_nodes[UID_MAX_NODES];
    char tree_text[UID_MAX_NODES][128];
    uid_id selected[UID_MAX_NODES];
    int selection_count;
    uid_id reported_selection[UID_MAX_NODES];
    int reported_count;
    char tool[UID_ID_CAPACITY], property[UID_ID_CAPACITY];
    int refresh, events, syncing, drag, resize, snap, drag_changed;
    float zoom, start_x, start_y, current_x, current_y;
    xui_rect_t starts[UID_MAX_NODES];
    uid_id drag_parent;
    xui_proxy_t proxy;
};
enum {
    ACT_UNDO = 1,
    ACT_REDO,
    ACT_LEFT,
    ACT_TOP,
    ACT_ZOOM_IN,
    ACT_ZOOM_OUT,
    ACT_PROPERTIES,
    ACT_EVENTS,
    ACT_RESET,
    ACT_POINTER,
    ACT_TOOL,
    ACT_SNAP
};
static void invalidate(uid_editor *e) {
    if (e->root)
        xuiWidgetInvalidate(e->root, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE |
                                         XUI_WIDGET_DIRTY_RENDER);
    if (e->host.invalidate)
        e->host.invalidate(e->host.user);
    /* Parent cache invalidation does not invalidate independent SELF caches.
       Selection/drag/zoom must repaint the overlay even if its size is stable. */
    if (e->canvas)
        xuiWidgetInvalidate(e->canvas, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
    if (e->overlay)
        xuiWidgetInvalidate(e->overlay, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}
static int report(uid_editor *e, int r) {
    if (r && e->host.status)
        e->host.status(uidLastError(e->context), e->host.user);
    return r;
}
static int selected(const uid_editor *e, uid_id id) {
    int i;
    for (i = 0; i < e->selection_count; ++i)
        if (e->selected[i] == id)
            return 1;
    return 0;
}
static int selected_ancestor(const uid_editor *e, uid_id id) {
    uid_node_i *n = uid_node(e->document, id);
    for (n = n ? uid_node(e->document, n->parent) : NULL; n; n = uid_node(e->document, n->parent))
        if (selected(e, n->id))
            return 1;
    return 0;
}
static xui_rect_t node_rect(const uid_document *d, uid_id id) {
    return (xui_rect_t){(int)uid_num(d, id, "x", 0), (int)uid_num(d, id, "y", 0),
                        (int)uid_num(d, id, "width", 100), (int)uid_num(d, id, "height", 36)};
}
static int contains(xui_rect_t r, float x, float y) {
    return x >= r.fX && y >= r.fY && x <= r.fX + r.fW && y <= r.fY + r.fH;
}
static void on_notice(const uid_notice *n, void *u) {
    uid_editor *e = u;
    if (n->document == e->document) {
        e->refresh = 1;
        invalidate(e);
    }
}
static int make_button(uid_editor *e, const char *label, int command, const char *type,
                       void (*fn)(xui_widget, void *)) {
    xui_button_desc_t d = {0};
    ui_action *a;
    int r;
    if (e->action_count >= 80)
        return UID_LIMIT;
    a = &e->actions[e->action_count++];
    *a = (ui_action){e, command, type, NULL};
    d.iSize = sizeof(d);
    d.pFont = e->host.font;
    d.sText = label;
    d.iTextColor = 0x233246FFu;
    r = xuiButtonCreate(e->host.xui, &a->widget, &d);
    if (!r)
        r = xuiWidgetAddChild(e->root, a->widget);
    if (!r)
        r = xuiButtonSetClick(a->widget, fn, a);
    return r;
}
static int root_layout(xui_widget w, xui_rect_t r, void *u) {
    uid_editor *e = u;
    int i;
    float left = 174, right = 306, top = 42, tool_y = r.fY + top + 6, toolbar_x = r.fX + 4;
    float cw = r.fW - left - right - 16;
    if (cw < 160)
        cw = 160;
    xuiLayoutArrangeChild(w, e->canvas, (xui_rect_t){r.fX + left + 6, r.fY + top, cw, r.fH - top});
    xuiLayoutArrangeChild(w, e->heading, (xui_rect_t){r.fX + r.fW - right, r.fY + top, right, 30});
    xuiLayoutArrangeChild(
        w, e->grid, (xui_rect_t){r.fX + r.fW - right, r.fY + top + 72, right, r.fH - top - 72});
    for (i = 0; i < e->action_count; ++i) {
        ui_action *a = &e->actions[i];
        xui_rect_t b;
        if (a->command == ACT_TOOL || a->command == ACT_POINTER) {
            b = (xui_rect_t){r.fX + 4, tool_y, left - 4, 30};
            tool_y += 34;
        } else if (a->command == ACT_PROPERTIES)
            b = (xui_rect_t){r.fX + r.fW - right, r.fY + top + 34, 98, 32};
        else if (a->command == ACT_EVENTS)
            b = (xui_rect_t){r.fX + r.fW - right + 102, r.fY + top + 34, 90, 32};
        else if (a->command == ACT_RESET)
            b = (xui_rect_t){r.fX + r.fW - right + 196, r.fY + top + 34, 106, 32};
        else {
            b = (xui_rect_t){toolbar_x, r.fY + 3, 86, 32};
            toolbar_x += 90;
        }
        xuiLayoutArrangeChild(w, a->widget, b);
    }
    xuiLayoutArrangeChild(
        w, e->tree, (xui_rect_t){r.fX + 4, tool_y + 10, left - 4, r.fH - (tool_y - r.fY) - 14});
    return XUI_OK;
}
static int canvas_layout(xui_widget w, xui_rect_t r, void *u) {
    uid_editor *e = u;
    uid_id root = uidDocumentRoot(e->document);
    if (e->scene && e->scene->root)
        xuiLayoutArrangeChild(
            w, e->scene->root,
            (xui_rect_t){r.fX + 24, r.fY + 46,
                         (int)lround(uid_num(e->document, root, "width", 800) * e->zoom),
                         (int)lround(uid_num(e->document, root, "height", 480) * e->zoom)});
    return xuiLayoutArrangeChild(w, e->overlay, r);
}
static int canvas_paint(xui_widget w, xui_draw_context draw, uint32_t state, void *u) {
    uid_editor *e = u;
    xui_rect_t r = xuiWidgetGetContentRect(w);
    char title[192];
    uid_node_info n;
    (void)state;
    e->proxy.drawRectFill(&e->proxy, draw, r, 0xE7EDF5FFu);
    uidNodeGet(e->document, uidDocumentRoot(e->document), &n);
    snprintf(title, sizeof(title), "%s  /  %.0f x %.0f   |   %.0f%%", n.name,
             uid_num(e->document, n.id, "width", 800), uid_num(e->document, n.id, "height", 480),
             e->zoom * 100);
    return e->proxy.drawText(&e->proxy, draw, e->host.font, title,
                             (xui_rect_t){r.fX + 24, r.fY + 8, r.fW - 30, 30}, 0x41536AFFu,
                             XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
}
static int overlay_paint(xui_widget w, xui_draw_context draw, uint32_t state, void *u) {
    uid_editor *e = u;
    xui_rect_t origin = xuiWidgetGetWorldRect(w), r;
    int i;
    float x, y;
    (void)state;
    for (i = 0; i < e->selection_count; ++i) {
        xui_widget item = uidEditorNodeWidget(e, e->selected[i]);
        if (!item)
            continue;
        r = xuiWidgetGetWorldRect(item);
        r.fX -= origin.fX;
        r.fY -= origin.fY;
        e->proxy.drawRectStroke(&e->proxy, draw, r, 2, 0x1974D2FFu);
        e->proxy.drawRectFill(&e->proxy, draw, (xui_rect_t){r.fX + r.fW - 4, r.fY + r.fH - 4, 8, 8},
                              0x1974D2FFu);
    }
    if (e->drag == 2 || e->drag == 3) {
        x = fminf(e->start_x, e->current_x) - origin.fX;
        y = fminf(e->start_y, e->current_y) - origin.fY;
        r = (xui_rect_t){x, y, fabsf(e->current_x - e->start_x), fabsf(e->current_y - e->start_y)};
        e->proxy.drawRectStroke(&e->proxy, draw, r, 1, 0x1974D2FFu);
    }
    if (e->drag == 1 && e->snap && e->selection_count) {
        xui_widget item = uidEditorNodeWidget(e, e->selected[0]);
        if (item) {
            r = xuiWidgetGetWorldRect(item);
            x = r.fX - origin.fX;
            y = r.fY - origin.fY;
            e->proxy.drawLine(&e->proxy, draw, x, 0, x, origin.fH, 1, 0xDA7F2EFFu);
            e->proxy.drawLine(&e->proxy, draw, 0, y, origin.fW, y, 1, 0xDA7F2EFFu);
        }
    }
    return XUI_OK;
}
int uidEditorFlushPendingEdits(uid_editor *e) {
    if (!e)
        return UID_INVALID;
    if (e->drag)
        return UID_BUSY;
    if (e->grid && xuiPropertyGridIsEditing(e->grid))
        return xuiPropertyGridEndEdit(e->grid, 1)
                   ? UID_OK
                   : report(e, uid_fail(e->context, UID_INVALID,
                                        "The property editor contains an invalid value; correct it "
                                        "or press Escape."));
    return UID_OK;
}
int uidEditorSelect(uid_editor *e, uid_id id, int extend) {
    int i, r;
    if (!e || !uid_node(e->document, id))
        return UID_NOT_FOUND;
    if ((r = uidEditorFlushPendingEdits(e)))
        return r;
    if (id == uidDocumentRoot(e->document) ||
        (e->selection_count == 1 && e->selected[0] == uidDocumentRoot(e->document)))
        extend = 0;
    if (!extend)
        e->selection_count = 0;
    if (extend && selected(e, id)) {
        for (i = 0; i < e->selection_count; ++i)
            if (e->selected[i] == id) {
                memmove(&e->selected[i], &e->selected[i + 1],
                        (size_t)(--e->selection_count - i) * sizeof(uid_id));
                break;
            }
    } else if (e->selection_count < UID_MAX_NODES)
        e->selected[e->selection_count++] = id;
    if (!e->selection_count)
        e->selected[e->selection_count++] = uidDocumentRoot(e->document);
    e->refresh = 1;
    invalidate(e);
    return UID_OK;
}
int uidEditorSelection(const uid_editor *e, uid_id *out, int cap) {
    int n;
    if (!e)
        return 0;
    n = e->selection_count;
    if (out && cap > 0)
        memcpy(out, e->selected, (size_t)(n < cap ? n : cap) * sizeof(uid_id));
    return n;
}
int uidEditorSetTool(uid_editor *e, const char *type) {
    const uid_control_desc *d;
    if (!e)
        return UID_INVALID;
    if (type && *type) {
        d = uidFindControl(e->context, type);
        if (!d || (d->flags & UID_ROOT))
            return UID_INVALID;
        snprintf(e->tool, sizeof(e->tool), "%s", type);
    } else
        e->tool[0] = 0;
    invalidate(e);
    return UID_OK;
}
int uidEditorShowEvents(uid_editor *e, int b) {
    int r;
    if (!e)
        return UID_INVALID;
    if ((r = uidEditorFlushPendingEdits(e)))
        return r;
    e->events = !!b;
    e->refresh = 1;
    invalidate(e);
    return UID_OK;
}
int uidEditorSetZoom(uid_editor *e, float z) {
    if (!e || !isfinite(z) || z < 0.25f || z > 2)
        return UID_INVALID;
    e->zoom = z;
    e->refresh = 1;
    invalidate(e);
    return UID_OK;
}
int uidEditorSetSelectionProperty(uid_editor *e, const char *key, uid_value v) {
    uid_change changes[UID_MAX_NODES];
    uid_node_info n;
    const uid_property_desc *p;
    int i;
    if (!e)
        return UID_INVALID;
    for (i = 0; i < e->selection_count; ++i) {
        uidNodeGet(e->document, e->selected[i], &n);
        p = uidFindProperty(e->context, n.type, key);
        if (e->selection_count > 1 && (!p || !(p->flags & UID_MULTIEDIT)))
            return UID_INVALID;
        changes[i] = (uid_change){e->selected[i], key, v};
    }
    return report(e, uidSetProperties(e->document, changes, e->selection_count));
}
int uidEditorEditProperty(uid_editor *e, const char *key) {
    uid_node_info node;
    const uid_property_desc *p;
    uid_value value = {0};
    uid_id selection[UID_MAX_NODES];
    int count, r;
    uint64_t revision;
    if (!e || !key || !e->selection_count)
        return UID_INVALID;
    if ((r = uidEditorFlushPendingEdits(e)))
        return r;
    if (uidNodeGet(e->document, e->selected[0], &node))
        return UID_NOT_FOUND;
    p = uidFindProperty(e->context, node.type, key);
    if (!p || !(p->flags & UID_CUSTOM_EDITOR) || (p->flags & UID_READONLY))
        return UID_INVALID;
    if (!e->host.edit_property)
        return report(e, uid_fail(e->context, UID_UNSUPPORTED,
                                  "The host has not supplied a custom property editor"));
    count = e->selection_count;
    memcpy(selection, e->selected, (size_t)count * sizeof(uid_id));
    revision = uidDocumentRevision(e->document);
    r = e->host.edit_property(e->document, selection, count, p, xuiWidgetGetWorldRect(e->grid),
                              &value, e->host.user);
    if (r || !value.kind)
        return report(e, r);
    if (revision != uidDocumentRevision(e->document) || count != e->selection_count ||
        memcmp(selection, e->selected, (size_t)count * sizeof(uid_id)))
        return report(e, uid_fail(e->context, UID_CONFLICT, "Custom property result is stale"));
    return uidEditorSetSelectionProperty(e, key, value);
}
int uidEditorAlign(uid_editor *e, int vertical) {
    uid_change changes[UID_MAX_NODES];
    uid_node_info first, n;
    uid_value value;
    int i;
    if (!e || e->selection_count < 2)
        return UID_INVALID;
    uidNodeGet(e->document, e->selected[0], &first);
    uidGetProperty(e->document, first.id, vertical ? "y" : "x", &value);
    for (i = 0; i < e->selection_count; ++i) {
        uidNodeGet(e->document, e->selected[i], &n);
        if (!n.parent || n.parent != first.parent)
            return report(e, uid_fail(e->context, UID_INVALID,
                                      "Alignment requires siblings in one container"));
        changes[i] = (uid_change){n.id, vertical ? "y" : "x", value};
    }
    return report(e, uidSetProperties(e->document, changes, e->selection_count));
}
static int edit_event(uid_editor *e, uid_id id, const char *event) {
    uid_binding binding = {0};
    uid_node_info n;
    const uid_event_desc *desc;
    char handler[UID_ID_CAPACITY] = {0};
    uint64_t revision;
    int r;
    if ((r = uidEditorFlushPendingEdits(e)))
        return r;
    if (uidNodeGet(e->document, id, &n))
        return UID_NOT_FOUND;
    desc = uidFindEvent(e->context, n.type, event);
    if (!desc)
        return UID_NOT_FOUND;
    uidGetBinding(e->document, id, event, &binding);
    if (*binding.handler && !e->host.ensure_handler) {
        if (e->host.show_code)
            e->host.show_code(binding.handler, e->host.user);
        return UID_OK;
    }
    if (!e->host.ensure_handler)
        return report(
            e, uid_fail(e->context, UID_UNSUPPORTED, "The host has not supplied a code service"));
    revision = uidDocumentRevision(e->document);
    r = e->host.ensure_handler(e->document, id, desc, binding.handler, handler, sizeof(handler),
                               e->host.user);
    if (r || !*handler)
        return report(e, r);
    if (!uidAcceptResult(e->document, uidDocumentId(e->document), revision))
        return report(
            e, uid_fail(e->context, UID_CONFLICT,
                        "The document changed while creating a handler; binding was not applied"));
    r = uidSetBinding(e->document, id, event, handler);
    if (!r && e->host.show_code)
        e->host.show_code(handler, e->host.user);
    return report(e, r);
}
int uidEditorEditDefaultEvent(uid_editor *e, uid_id id) {
    uid_node_info n;
    const uid_control_desc *d;
    if (!e || uidNodeGet(e->document, id, &n))
        return UID_NOT_FOUND;
    d = uidFindControl(e->context, n.type);
    return d && *d->default_event ? edit_event(e, id, d->default_event) : UID_NOT_FOUND;
}
static void action(xui_widget w, void *u) {
    ui_action *a = u;
    uid_editor *e = a->editor;
    uid_node_info n;
    const uid_property_desc *p;
    (void)w;
    if (uidEditorFlushPendingEdits(e))
        return;
    switch (a->command) {
    case ACT_UNDO:
        report(e, uidUndo(e->document));
        break;
    case ACT_REDO:
        report(e, uidRedo(e->document));
        break;
    case ACT_LEFT:
        uidEditorAlign(e, 0);
        break;
    case ACT_TOP:
        uidEditorAlign(e, 1);
        break;
    case ACT_ZOOM_IN:
        uidEditorSetZoom(e, fminf(2, e->zoom + 0.1f));
        break;
    case ACT_ZOOM_OUT:
        uidEditorSetZoom(e, fmaxf(0.25f, e->zoom - 0.1f));
        break;
    case ACT_PROPERTIES:
        uidEditorShowEvents(e, 0);
        break;
    case ACT_EVENTS:
        uidEditorShowEvents(e, 1);
        break;
    case ACT_POINTER:
        uidEditorSetTool(e, NULL);
        break;
    case ACT_TOOL:
        uidEditorSetTool(e, a->type);
        break;
    case ACT_SNAP:
        e->snap = !e->snap;
        xuiButtonSetText(a->widget, e->snap ? "Snap: on" : "Snap: off");
        break;
    case ACT_RESET:
        if (e->events) {
            if (*e->property)
                uidSetBinding(e->document, e->selected[0], e->property, "");
        } else if (!uidNodeGet(e->document, e->selected[0], &n) &&
                   (p = uidFindProperty(e->context, n.type, e->property)) &&
                   strcmp(e->property, "name"))
            uidEditorSetSelectionProperty(e, e->property, p->initial);
        break;
    }
}
static void tree_select(xui_widget w, int id, void *u) {
    uid_editor *e = u;
    (void)w;
    if (!e->syncing)
        uidEditorSelect(e, (uid_id)id, 0);
}
static void value_text(uid_value v, char *out, size_t cap) {
    if (v.kind == UID_STRING)
        snprintf(out, cap, "%s", v.text ? v.text : "");
    else if (v.kind == UID_BOOL)
        snprintf(out, cap, "%s", v.number ? "true" : "false");
    else if (v.kind == UID_COLOR)
        snprintf(out, cap, "#%08X", (unsigned)v.number);
    else
        snprintf(out, cap, "%.10g", v.number);
}
static int parse_value(int kind, const char *s, uid_value *v) {
    char *end;
    unsigned long color;
    if (!s)
        return UID_INVALID;
    if (kind == UID_STRING) {
        *v = uidString(s);
        return UID_OK;
    }
    if (kind == UID_BOOL) {
        if (!strcmp(s, "true") || !strcmp(s, "1"))
            *v = uidBool(1);
        else if (!strcmp(s, "false") || !strcmp(s, "0"))
            *v = uidBool(0);
        else
            return UID_INVALID;
        return UID_OK;
    }
    if (kind == UID_COLOR) {
        if (*s != '#' || (strlen(s) != 7 && strlen(s) != 9))
            return UID_INVALID;
        color = strtoul(s + 1, &end, 16);
        if (*end)
            return UID_INVALID;
        if (strlen(s) == 7)
            color = (color << 8) | 255;
        *v = uidColor((uint32_t)color);
        return UID_OK;
    }
    *v = uidNumber(strtod(s, &end));
    while (*end && isspace((unsigned char)*end))
        ++end;
    return end == s || *end || !isfinite(v->number) ? UID_INVALID : UID_OK;
}
static int grid_validate_result(xui_widget w, int row, const char *key, const char *s, int type,
                                void *u) {
    uid_editor *e = u;
    uid_node_info n;
    const uid_property_desc *p;
    const uid_event_desc *ev;
    uid_value v;
    int i, r;
    (void)w;
    (void)row;
    (void)type;
    if (e->syncing)
        return XUI_OK;
    if (!e->selection_count)
        return UID_INVALID;
    uidNodeGet(e->document, e->selected[0], &n);
    if (e->events) {
        ev = uidFindEvent(e->context, n.type, key);
        if (!ev || (*s && !uidIdentifierValid(s)))
            return UID_INVALID;
        return *s && e->host.validate_handler ? e->host.validate_handler(ev, s, e->host.user)
                                              : UID_OK;
    }
    p = uidFindProperty(e->context, n.type, key);
    if (!p)
        return UID_INVALID;
    if ((r = parse_value(p->initial.kind, s, &v)))
        return r;
    for (i = 0; i < e->selection_count; ++i)
        if ((r = uidValidateProperty(e->document, e->selected[i], key, &v)))
            return report(e, r);
    return UID_OK;
}
static int grid_validate(xui_widget w, int row, const char *key, const char *s, int type, void *u) {
    return grid_validate_result(w, row, key, s, type, u) == UID_OK;
}
static void grid_change(xui_widget w, int row, const char *key, const char *s, int type, void *u) {
    uid_editor *e = u;
    uid_node_info n;
    const uid_property_desc *p;
    uid_value v;
    (void)w;
    (void)row;
    (void)type;
    if (e->syncing || !e->selection_count)
        return;
    if (grid_validate_result(w, row, key, s, type, u))
        return;
    if (e->events) {
        report(e, uidSetBinding(e->document, e->selected[0], key, s));
        return;
    }
    uidNodeGet(e->document, e->selected[0], &n);
    p = uidFindProperty(e->context, n.type, key);
    if (p && !parse_value(p->initial.kind, s, &v))
        uidEditorSetSelectionProperty(e, key, v);
}
static void grid_select(xui_widget w, int row, const char *key, void *u) {
    uid_editor *e = u;
    (void)w;
    (void)row;
    if (key)
        snprintf(e->property, sizeof(e->property), "%s", key);
}
static int grid_action(xui_widget w, int row, const char *key, xui_rect_t rect, void *u) {
    uid_editor *e = u;
    (void)w;
    (void)row;
    (void)rect;
    return (e->events ? edit_event(e, e->selected[0], key) : uidEditorEditProperty(e, key)) ==
           UID_OK;
}
static int grid_add(xui_widget grid, int category, const xui_property_grid_property_t *p) {
    xui_property_desc_t desc = {p->sId,    p->sName,         p->sDescription, p->iType,
                                p->sValue, p->sDefaultValue, p->iFlags};
    int index = xuiPropertyGridAddProperty(grid, category, &desc);
    if (index >= 0 && p->onAction)
        xuiPropertyGridSetAction(grid, index, p->onAction, p->pActionUser);
    return index;
}
static int refresh_grid(uid_editor *e) {
    uid_node_info n, other;
    const uid_control_desc *d;
    const uid_property_desc *p, *op;
    uid_value v, ov;
    xui_property_grid_property_t item;
    uid_binding binding;
    int i, j, cat, mixed, common;
    char heading[128], a[UID_MAX_TEXT + 1], b[UID_MAX_TEXT + 1];
    uidNodeGet(e->document, e->selected[0], &n);
    d = uidFindControl(e->context, n.type);
    snprintf(heading, sizeof(heading), "%s%s", n.name,
             e->selection_count > 1 ? "  (multiple)" : "");
    xuiLabelSetText(e->heading, heading);
    xuiPropertyGridClear(e->grid);
    if (!d)
        return UID_OK;
    if (e->events) {
        cat =
            xuiPropertyGridAddCategory(e->grid, "events", "Events / ... to create or navigate", 1);
        for (i = 0; i < d->event_count; ++i) {
            memset(&item, 0, sizeof(item));
            snprintf(item.sId, sizeof(item.sId), "%s", d->events[i].id);
            snprintf(item.sName, sizeof(item.sName), "%s", d->events[i].name);
            snprintf(item.sDescription, sizeof(item.sDescription), "%s", d->events[i].description);
            uidGetBinding(e->document, n.id, d->events[i].id, &binding);
            snprintf(item.sValue, sizeof(item.sValue), "%s", binding.handler);
            item.iType = XUI_TABLE_CELL_TYPE_TEXT;
            item.onAction = grid_action;
            item.pActionUser = e;
            if (e->selection_count > 1)
                item.iFlags = XUI_PROPERTY_FLAG_READONLY;
            grid_add(e->grid, cat, &item);
        }
        return UID_OK;
    }
    for (i = 0; i < d->property_count; ++i) {
        p = &d->properties[i];
        if (p->flags & UID_HIDDEN)
            continue;
        uidGetProperty(e->document, n.id, p->id, &v);
        value_text(v, a, sizeof(a));
        mixed = 0;
        common = 1;
        for (j = 1; j < e->selection_count; ++j) {
            uidNodeGet(e->document, e->selected[j], &other);
            op = uidFindProperty(e->context, other.type, p->id);
            if (!op || op->initial.kind != p->initial.kind || !(p->flags & UID_MULTIEDIT) ||
                !(op->flags & UID_MULTIEDIT)) {
                common = 0;
                break;
            }
            uidGetProperty(e->document, other.id, p->id, &ov);
            value_text(ov, b, sizeof(b));
            if (strcmp(a, b))
                mixed = 1;
        }
        if (!common)
            continue;
        cat = xuiPropertyGridFindCategory(e->grid, p->category);
        if (cat < 0)
            cat = xuiPropertyGridAddCategory(e->grid, p->category, p->category, 1);
        memset(&item, 0, sizeof(item));
        snprintf(item.sId, sizeof(item.sId), "%s", p->id);
        snprintf(item.sName, sizeof(item.sName), "%s", p->name);
        snprintf(item.sDescription, sizeof(item.sDescription), "%s%s", p->description,
                 mixed ? " Multiple values: entering a value changes the whole selection." : "");
        snprintf(item.sValue, sizeof(item.sValue), "%.*s", (int)sizeof(item.sValue) - 1,
                 mixed ? "" : a);
        if (strlen(a) >= sizeof(item.sValue)) {
            item.iFlags |= XUI_PROPERTY_FLAG_READONLY;
            snprintf(item.sDescription, sizeof(item.sDescription),
                     "Long value is preserved. Edit through the host property API.");
        }
        value_text(p->initial, b, sizeof(b));
        snprintf(item.sDefaultValue, sizeof(item.sDefaultValue), "%.*s",
                 (int)sizeof(item.sDefaultValue) - 1, b);
        item.bAutoDirty = 1;
        item.iType = mixed                           ? XUI_TABLE_CELL_TYPE_TEXT
                     : p->initial.kind == UID_STRING ? XUI_TABLE_CELL_TYPE_TEXT
                     : p->initial.kind == UID_BOOL   ? XUI_TABLE_CELL_TYPE_BOOL
                     : p->initial.kind == UID_COLOR  ? XUI_TABLE_CELL_TYPE_COLOR
                                                     : XUI_TABLE_CELL_TYPE_FLOAT;
        if (p->flags & UID_READONLY)
            item.iFlags |= XUI_PROPERTY_FLAG_READONLY;
        if (p->flags & UID_CUSTOM_EDITOR) {
            item.onAction = grid_action;
            item.pActionUser = e;
        }
        grid_add(e->grid, cat, &item);
    }
    return UID_OK;
}
static uid_id hit(uid_editor *e, float x, float y, int container) {
    int i;
    uid_node_info n;
    const uid_control_desc *d;
    for (i = uidNodeCount(e->document) - 1; i >= 0; --i) {
        uidNodeAt(e->document, i, &n);
        d = uidFindControl(e->context, n.type);
        if (container && (!d || !(d->flags & UID_CONTAINER)))
            continue;
        xui_widget w = uidEditorNodeWidget(e, n.id);
        if (w && contains(xuiWidgetGetWorldRect(w), x, y))
            return n.id;
    }
    return uidDocumentRoot(e->document);
}
static int change_rect(uid_editor *e, uid_id id, xui_rect_t r) {
    uid_change c[] = {{id, "x", uidNumber(r.fX)},
                      {id, "y", uidNumber(r.fY)},
                      {id, "width", uidNumber(r.fW)},
                      {id, "height", uidNumber(r.fH)}};
    return uidSetProperties(e->document, c, 4);
}
static void cancel_drag(uid_editor *e) {
    if (e->drag == 1)
        uidCancel(e->document);
    e->drag = 0;
    e->refresh = 1;
    xuiReleasePointerCapture(e->host.xui, e->overlay);
    invalidate(e);
}
static int canvas_event(xui_widget w, const xui_event_t *event, void *u) {
    uid_editor *e = u;
    uid_id id;
    uid_node_info n;
    int i, r = 0;
    float dx, dy;
    xui_rect_t rect, pr;
    if (event->iPhase != XUI_EVENT_PHASE_TARGET)
        return XUI_OK;
    if (event->iType == XUI_EVENT_POINTER_CAPTURE_LOST && e->drag) {
        cancel_drag(e);
        return XUI_OK;
    }
    if (event->iType == XUI_EVENT_POINTER_DOUBLE_CLICK) {
        id = hit(e, event->fX, event->fY, 0);
        if (e->drag)
            cancel_drag(e);
        uidEditorSelect(e, id, 0);
        uidEditorEditDefaultEvent(e, id);
        return XUI_EVENT_DISPATCH_STOP;
    }
    if (event->iType == XUI_EVENT_KEY_DOWN) {
        if (event->iKey == XUI_KEY_ESCAPE) {
            if (e->drag)
                cancel_drag(e);
            else
                uidEditorSetTool(e, NULL);
            return XUI_EVENT_DISPATCH_STOP;
        }
        if ((event->iModifiers & XUI_MOD_CTRL) && event->iKey == 'Z') {
            report(e, uidUndo(e->document));
            return XUI_EVENT_DISPATCH_STOP;
        }
        if ((event->iModifiers & XUI_MOD_CTRL) && event->iKey == 'Y') {
            report(e, uidRedo(e->document));
            return XUI_EVENT_DISPATCH_STOP;
        }
        if (event->iKey == XUI_KEY_DELETE) {
            if (!uidBegin(e->document, "Delete selection")) {
                for (i = e->selection_count - 1; i >= 0; --i)
                    if (e->selected[i] != uidDocumentRoot(e->document) &&
                        uid_node(e->document, e->selected[i]))
                        uidDeleteNode(e->document, e->selected[i]);
                uidCommit(e->document);
            }
            return XUI_EVENT_DISPATCH_STOP;
        }
        dx = event->iKey == XUI_KEY_LEFT ? -1 : event->iKey == XUI_KEY_RIGHT ? 1 : 0;
        dy = event->iKey == XUI_KEY_UP ? -1 : event->iKey == XUI_KEY_DOWN ? 1 : 0;
        if (dx || dy) {
            if (event->iModifiers & XUI_MOD_SHIFT) {
                dx *= 8;
                dy *= 8;
            }
            if (!uidBegin(e->document, "Nudge selection")) {
                for (i = 0; i < e->selection_count; ++i)
                    if (e->selected[i] != uidDocumentRoot(e->document) &&
                        !selected_ancestor(e, e->selected[i])) {
                        rect = node_rect(e->document, e->selected[i]);
                        rect.fX += (int)dx;
                        rect.fY += (int)dy;
                        if ((r = change_rect(e, e->selected[i], rect)))
                            break;
                    }
                if (r)
                    uidCancel(e->document);
                else
                    uidCommit(e->document);
            }
            return XUI_EVENT_DISPATCH_STOP;
        }
    }
    if (event->iType == XUI_EVENT_POINTER_DOWN && event->iButton == XUI_POINTER_BUTTON_LEFT) {
        if (uidEditorFlushPendingEdits(e))
            return XUI_EVENT_DISPATCH_STOP;
        xuiSetFocusWidget(e->host.xui, w);
        e->start_x = e->current_x = event->fX;
        e->start_y = e->current_y = event->fY;
        e->drag_changed = 0;
        e->resize = 0;
        if (*e->tool) {
            e->drag = 2;
            e->drag_parent = hit(e, event->fX, event->fY, 1);
        } else {
            id = hit(e, event->fX, event->fY, 0);
            rect = xuiWidgetGetWorldRect(uidEditorNodeWidget(e, id));
            e->resize = abs(event->fX - (rect.fX + rect.fW)) <= 9 &&
                        abs(event->fY - (rect.fY + rect.fH)) <= 9;
            if (event->iModifiers & (XUI_MOD_CTRL | XUI_MOD_SHIFT))
                uidEditorSelect(e, id, 1);
            else if (!selected(e, id))
                uidEditorSelect(e, id, 0);
            if (id == uidDocumentRoot(e->document) && !e->resize)
                e->drag = 3;
            else {
                if (uidBegin(e->document, e->resize ? "Resize" : "Move selection"))
                    return XUI_EVENT_DISPATCH_STOP;
                e->drag = 1;
                for (i = 0; i < e->selection_count; ++i)
                    e->starts[i] = node_rect(e->document, e->selected[i]);
            }
        }
        xuiSetPointerCapture(e->host.xui, w);
        invalidate(e);
        return XUI_EVENT_DISPATCH_STOP;
    }
    if (event->iType == XUI_EVENT_POINTER_MOVE && e->drag) {
        e->current_x = event->fX;
        e->current_y = event->fY;
        dx = (event->fX - e->start_x) / e->zoom;
        dy = (event->fY - e->start_y) / e->zoom;
        if (fabsf(dx) + fabsf(dy) > 2)
            e->drag_changed = 1;
        if (e->drag == 1) {
            if (e->snap) {
                dx = roundf(dx / 8) * 8;
                dy = roundf(dy / 8) * 8;
            }
            for (i = 0; i < e->selection_count; ++i) {
                id = e->selected[i];
                if (!e->resize && selected_ancestor(e, id))
                    continue;
                rect = e->starts[i];
                if (e->resize) {
                    rect.fW = (int)fmaxf(8, rect.fW + dx);
                    rect.fH = (int)fmaxf(8, rect.fH + dy);
                } else if (id != uidDocumentRoot(e->document)) {
                    rect.fX += (int)dx;
                    rect.fY += (int)dy;
                }
                if ((r = change_rect(e, id, rect)))
                    break;
            }
            if (r) {
                report(e, r);
                cancel_drag(e);
            } else
                e->refresh = 1;
        }
        invalidate(e);
        return XUI_EVENT_DISPATCH_STOP;
    }
    if (event->iType == XUI_EVENT_POINTER_UP && e->drag) {
        int drag = e->drag;
        e->current_x = event->fX;
        e->current_y = event->fY;
        e->drag = 0;
        xuiReleasePointerCapture(e->host.xui, e->overlay);
        if (drag == 1)
            report(e, uidCommit(e->document));
        else if (drag == 2) {
            pr = xuiWidgetGetWorldRect(uidEditorNodeWidget(e, e->drag_parent));
            rect = (xui_rect_t){(int)((fminf(e->start_x, event->fX) - pr.fX) / e->zoom),
                                (int)((fminf(e->start_y, event->fY) - pr.fY) / e->zoom),
                                (int)(fabsf(event->fX - e->start_x) / e->zoom),
                                (int)(fabsf(event->fY - e->start_y) / e->zoom)};
            if (!uidBegin(e->document, "Draw control")) {
                r = uidAddNode(e->document, e->tool, e->drag_parent, "content", &id);
                if (!r) {
                    xui_rect_t initial = node_rect(e->document, id);
                    if (rect.fW < 8)
                        rect.fW = initial.fW;
                    if (rect.fH < 8)
                        rect.fH = initial.fH;
                    r = change_rect(e, id, rect);
                }
                if (r)
                    uidCancel(e->document);
                else {
                    uidCommit(e->document);
                    uidEditorSelect(e, id, 0);
                }
            }
            uidEditorSetTool(e, NULL);
        } else {
            rect = (xui_rect_t){fminf(e->start_x, event->fX), fminf(e->start_y, event->fY),
                                fabsf(event->fX - e->start_x), fabsf(event->fY - e->start_y)};
            e->selection_count = 0;
            for (i = 1; i < uidNodeCount(e->document); ++i) {
                uidNodeAt(e->document, i, &n);
                xui_widget node = uidEditorNodeWidget(e, n.id);
                if (node) {
                    pr = xuiWidgetGetWorldRect(node);
                    if (contains(rect, pr.fX, pr.fY) &&
                        contains(rect, pr.fX + pr.fW, pr.fY + pr.fH))
                        e->selected[e->selection_count++] = n.id;
                }
            }
            if (!e->selection_count)
                e->selected[e->selection_count++] = uidDocumentRoot(e->document);
        }
        e->refresh = 1;
        invalidate(e);
        return XUI_EVENT_DISPATCH_STOP;
    }
    return XUI_OK;
}
int uidEditorUpdate(uid_editor *e) {
    uid_node_info n;
    xui_rect_t rect;
    int i, j, r;
    if (!e)
        return UID_INVALID;
    if (!e->refresh)
        return UID_OK;
    if (xuiPropertyGridIsEditing(e->grid))
        return UID_OK;
    e->refresh = 0;
    for (i = e->selection_count - 1; i >= 0; --i)
        if (!uid_node(e->document, e->selected[i]))
            memmove(&e->selected[i], &e->selected[i + 1],
                    (size_t)(--e->selection_count - i) * sizeof(uid_id));
    if (!e->selection_count)
        e->selected[e->selection_count++] = uidDocumentRoot(e->document);
    uid_scene_destroy(e->scene);
    e->scene = NULL;
    r = uid_scene_create(e->context, e->document, e->host.xui, e->canvas, e->host.font, &e->runtime,
                         1, e->zoom, &e->scene);
    if (r)
        return report(e, r);
    /* The overlay must be above the freshly materialized widget tree. */
    xuiWidgetRemoveFromParent(e->overlay);
    xuiWidgetAddChild(e->canvas, e->overlay);
    rect = xuiWidgetGetContentRect(e->canvas);
    canvas_layout(e->canvas, rect, e);
    e->syncing = 1;
    for (i = 0; i < uidNodeCount(e->document); ++i) {
        uidNodeAt(e->document, i, &n);
        snprintf(e->tree_text[i], sizeof(e->tree_text[i]), "%s", n.name);
        memset(&e->tree_nodes[i], 0, sizeof(e->tree_nodes[i]));
        e->tree_nodes[i].iId = (int)n.id;
        e->tree_nodes[i].iParent = n.parent ? (int)n.parent : -1;
        e->tree_nodes[i].sText = e->tree_text[i];
        {
            uid_node_i *ancestor = uid_node(e->document, n.parent);
            for (; ancestor; ancestor = uid_node(e->document, ancestor->parent))
                ++e->tree_nodes[i].iDepth;
        }
        e->tree_nodes[i].bExpanded = e->tree_nodes[i].bEnabled = 1;
        for (j = 0; j < uidNodeCount(e->document); ++j)
            if (e->document->nodes[j].parent == n.id)
                e->tree_nodes[i].bHasChildren = 1;
    }
    /* TreeView requires parents before children, unlike the document's stable
       insertion order after reparenting into a newer container. */
    for (i = 1; i < uidNodeCount(e->document); ++i) {
        xui_tree_view_node_t node = e->tree_nodes[i];
        for (j = i; j > 0 && e->tree_nodes[j - 1].iDepth > node.iDepth; --j)
            e->tree_nodes[j] = e->tree_nodes[j - 1];
        e->tree_nodes[j] = node;
    }
    r = xuiTreeViewSetNodes(e->tree, e->tree_nodes, uidNodeCount(e->document));
    if (r) {
        e->syncing = 0;
        return report(e, r);
    }
    xuiTreeViewSetSelected(e->tree, (int)e->selected[0]);
    refresh_grid(e);
    e->syncing = 0;
    if (e->reported_count != e->selection_count ||
        memcmp(e->reported_selection, e->selected, (size_t)e->selection_count * sizeof(uid_id))) {
        e->reported_count = e->selection_count;
        memcpy(e->reported_selection, e->selected, (size_t)e->selection_count * sizeof(uid_id));
        if (e->host.selection_changed) {
            ++e->context->callbacks;
            e->host.selection_changed(e, e->selected, e->selection_count, e->host.user);
            --e->context->callbacks;
        }
    }
    invalidate(e);
    return UID_OK;
}
static int cache_widget(uid_editor *e, xui_widget *out, xui_widget_cache_render_proc render) {
    xui_cache_policy_t p = {0};
    int r = xuiWidgetCreate(e->host.xui, out);
    if (r)
        return r;
    p.iSize = sizeof(p);
    p.iPolicy = XUI_CACHE_POLICY_SELF;
    p.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
    r = xuiWidgetSetCachePolicy(*out, &p);
    if (r) {
        xuiWidgetDestroy(*out);
        *out = NULL;
        return r;
    }
    if (render)
        xuiWidgetSetCacheRenderCallback(*out, render, e);
    return UID_OK;
}
int uidEditorCreate(uid_context *c, uid_document *doc, const uid_editor_desc *desc,
                    uid_editor **out) {
    uid_editor *e;
    xui_tree_view_desc_t tree = {0};
    xui_property_grid_desc_t grid = {0};
    xui_label_desc_t label = {0};
    int i, r;
    const uid_control_desc *control;
    if (!c || !doc || doc->context != c || doc->snapshot || doc->views || !desc ||
        desc->size != sizeof(*desc) || desc->version != UID_API_VERSION || !desc->xui ||
        !desc->parent || !out)
        return UID_INVALID;
    *out = NULL;
    if (c->callbacks)
        return UID_BUSY;
    e = calloc(1, sizeof(*e));
    if (!e)
        return UID_ERROR;
    e->context = c;
    e->document = doc;
    e->host = *desc;
    e->zoom = 1;
    e->snap = 1;
    e->selected[0] = uidDocumentRoot(doc);
    e->selection_count = 1;
    e->runtime = (uid_runtime_host){sizeof(e->runtime), desc->user, NULL, desc->resolve_font};
    xuiGetProxy(desc->xui, &e->proxy);
    r = xuiWidgetCreate(desc->xui, &e->root);
    if (r)
        goto fail;
    r = xuiWidgetAddChild(desc->parent, e->root);
    if (r)
        goto fail;
    xuiWidgetSetLayoutType(e->root, XUI_LAYOUT_MANUAL);
    xuiWidgetSetLayoutChildrenCallback(e->root, root_layout, e);
    r = cache_widget(e, &e->canvas, canvas_paint);
    if (r)
        goto fail;
    xuiWidgetAddChild(e->root, e->canvas);
    xuiWidgetSetLayoutChildrenCallback(e->canvas, canvas_layout, e);
    r = cache_widget(e, &e->overlay, overlay_paint);
    if (r)
        goto fail;
    xuiWidgetAddChild(e->canvas, e->overlay);
    xuiWidgetSetFocusable(e->overlay, 1);
    xuiWidgetSetTabStop(e->overlay, 1);
    xuiWidgetSetEventCallback(e->overlay, canvas_event, e);
    xuiWidgetSetEventInterest(
        e->overlay, XUI_EVENT_MASK_POINTER | XUI_EVENT_MASK_KEYBOARD | XUI_EVENT_MASK_CAPTURE, 1);
    tree.iSize = sizeof(tree);
    tree.pFont = desc->font;
    tree.fItemHeight = 30;
    tree.fIndent = 14;
    tree.fPadding = 5;
    tree.fBorderWidth = 1;
    tree.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
    tree.iBackgroundColor = 0xF8FAFDFFu;
    tree.iBorderColor = 0xC4D0DFFFu;
    tree.iSelectedColor = 0x1F65AFFFu;
    tree.iTextColor = 0x233246FFu;
    r = xuiTreeViewCreate(desc->xui, &e->tree, &tree);
    if (r)
        goto fail;
    xuiWidgetAddChild(e->root, e->tree);
    xuiTreeViewSetSelect(e->tree, tree_select, e);
    grid.iSize = sizeof(grid);
    grid.pFont = desc->font;
    grid.fNameWidth = 118;
    grid.fRowHeight = 30;
    grid.fCategoryHeight = 30;
    grid.iDescriptionMode = XUI_PROPERTY_GRID_DESCRIPTION_BOTH;
    grid.fDescriptionPanelHeight = 94;
    grid.iEditMode = XUI_TABLE_GRID_EDIT_QUICK;
    grid.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
    r = xuiPropertyGridCreate(desc->xui, &e->grid, &grid);
    if (r)
        goto fail;
    xuiWidgetAddChild(e->root, e->grid);
    xuiPropertyGridSetValidate(e->grid, grid_validate, e);
    xuiPropertyGridSetChange(e->grid, grid_change, e);
    xuiPropertyGridSetSelect(e->grid, grid_select, e);
    label.iSize = sizeof(label);
    label.pFont = desc->font;
    label.sText = "Selection";
    label.iTextColor = 0x233246FFu;
    r = xuiLabelCreate(desc->xui, &e->heading, &label);
    if (r)
        goto fail;
    xuiWidgetAddChild(e->root, e->heading);
#define BUTTON(LABEL, CMD, TYPE)                                                                   \
    if ((r = make_button(e, LABEL, CMD, TYPE, action)))                                            \
    goto fail
    BUTTON("Undo", ACT_UNDO, NULL);
    BUTTON("Redo", ACT_REDO, NULL);
    BUTTON("Align left", ACT_LEFT, NULL);
    BUTTON("Align top", ACT_TOP, NULL);
    BUTTON("Zoom +", ACT_ZOOM_IN, NULL);
    BUTTON("Zoom -", ACT_ZOOM_OUT, NULL);
    BUTTON("Snap: on", ACT_SNAP, NULL);
    BUTTON("Properties", ACT_PROPERTIES, NULL);
    BUTTON("Events", ACT_EVENTS, NULL);
    BUTTON("Reset", ACT_RESET, NULL);
    BUTTON("Pointer / select", ACT_POINTER, NULL);
    for (i = 0; i < uidControlCount(c); ++i) {
        control = uidControlAt(c, i);
        if (!(control->flags & UID_ROOT)) {
            BUTTON(control->name, ACT_TOOL, control->id);
            ++e->tool_count;
        }
    }
#undef BUTTON
    r = uidSubscribe(c, on_notice, e, &e->subscription);
    if (r)
        goto fail;
    ++c->views;
    ++doc->views;
    e->refresh = 1;
    *out = e;
    return UID_OK;
fail:
    uidUnsubscribe(c, e->subscription);
    if (e->root)
        xuiWidgetDestroy(e->root);
    free(e);
    return r;
}
void uidEditorDestroy(uid_editor *e) {
    if (!e)
        return;
    if (e->drag == 1)
        uidCancel(e->document);
    uidUnsubscribe(e->context, e->subscription);
    uid_scene_destroy(e->scene);
    if (e->root)
        xuiWidgetDestroy(e->root);
    --e->document->views;
    --e->context->views;
    free(e);
}
xui_widget uidEditorRoot(const uid_editor *e) {
    return e ? e->root : NULL;
}
xui_widget uidEditorCanvas(const uid_editor *e) {
    return e ? e->overlay : NULL;
}
xui_widget uidEditorPropertyGrid(const uid_editor *e) {
    return e ? e->grid : NULL;
}
xui_widget uidEditorTree(const uid_editor *e) {
    return e ? e->tree : NULL;
}
xui_widget uidEditorNodeWidget(const uid_editor *e, uid_id id) {
    return e ? uidPreviewNodeWidget(e->scene, id) : NULL;
}
