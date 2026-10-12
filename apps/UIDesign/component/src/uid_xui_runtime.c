#include "uid_xui_internal.h"

const uid_xui_adapter *uid_adapter(const uid_context *c, const char *type) {
    int i;
    for (i = 0; c && i < c->control_count; ++i)
        if (!strcmp(c->controls[i].id, type))
            return c->adapters[i];
    return NULL;
}
static void adapter_free(void *p) {
    uid_xui_adapter *a = p;
    free((void *)a->type);
    free((void *)a->c_runtime_type);
    free(a);
}
int uidXuiRegisterAdapter(uid_context *c, const uid_xui_adapter *a) {
    int i;
    uid_xui_adapter *copy;
    if (!c || !a || a->size != sizeof(*a) || !a->type || !a->create || !a->apply || c->callbacks)
        return UID_INVALID;
    for (i = 0; i < c->control_count; ++i)
        if (!strcmp(c->controls[i].id, a->type))
            break;
    if (i == c->control_count)
        return UID_NOT_FOUND;
    if (c->adapters[i])
        return UID_CONFLICT;
    copy = malloc(sizeof(*copy));
    if (!copy)
        return UID_ERROR;
    *copy = *a;
    copy->type = uid_dup(a->type);
    copy->c_runtime_type = uid_dup(a->c_runtime_type);
    if (!copy->type || !copy->c_runtime_type) {
        adapter_free(copy);
        return UID_ERROR;
    }
    c->adapters[i] = copy;
    c->free_adapter = adapter_free;
    return UID_OK;
}
static int box_paint(xui_widget w, xui_draw_context draw, uint32_t state, void *user) {
    xui_proxy_t p;
    xui_rect_t r = xuiWidgetGetContentRect(w);
    (void)state;
    if (xuiGetProxy(xuiWidgetGetContext(w), &p))
        return XUI_ERROR;
    p.drawRectFill(&p, draw, r, (uint32_t)(uintptr_t)user);
    return p.drawRectStroke(&p, draw, r, 1, 0xB9C7D8FFu);
}
static int builtin_create(xui_context xui, xui_font font, const uid_document *d, uid_id id,
                          xui_widget *out, void *user) {
    int kind = (int)(intptr_t)user, r;
    const char *text = uid_text(d, id, "text", "");
    uint32_t color = (uint32_t)uid_num(d, id, "textColor", 0x233246FFu);
    if (kind <= 1) {
        xui_cache_policy_t policy = {0};
        r = xuiWidgetCreate(xui, out);
        if (r)
            return r;
        policy.iSize = sizeof(policy);
        policy.iPolicy = XUI_CACHE_POLICY_SELF;
        policy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
        xuiWidgetSetCachePolicy(*out, &policy);
        xuiWidgetSetCacheRenderCallback(*out, box_paint,
                                        (void *)(uintptr_t)(kind ? 0xF3F6FAFFu : 0xFFFFFFFFu));
        xuiWidgetSetLayoutType(*out, XUI_LAYOUT_MANUAL);
        return XUI_OK;
    }
    if (kind == 2) {
        xui_label_desc_t a = {0};
        a.iSize = sizeof(a);
        a.pFont = font;
        a.sText = text;
        a.iTextColor = color;
        a.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
        return xuiLabelCreate(xui, out, &a);
    }
    if (kind == 3) {
        xui_input_desc_t a = {0};
        a.iSize = sizeof(a);
        a.pFont = font;
        a.sText = text;
        a.sPlaceholder = "Type here";
        a.iTextColor = color;
        a.iBackgroundColor = 0xFFFFFFFFu;
        return xuiInputCreate(xui, out, &a);
    }
    if (kind == 4) {
        xui_button_desc_t a = {0};
        a.iSize = sizeof(a);
        a.pFont = font;
        a.sText = text;
        a.iTextColor = color;
        return xuiButtonCreate(xui, out, &a);
    }
    if (kind == 5) {
        xui_checkbox_desc_t a = {0};
        a.iSize = sizeof(a);
        a.pFont = font;
        a.sText = text;
        a.iTextColor = color;
        a.bChecked = (int)uid_num(d, id, "checked", 0);
        return xuiCheckBoxCreate(xui, out, &a);
    }
    return UID_UNSUPPORTED;
}
static int builtin_apply(xui_widget w, const uid_document *d, uid_id id, float zoom, void *u) {
    (void)u;
    return xuiWidgetSetRect(w, (xui_rect_t){(int)lround(uid_num(d, id, "x", 0) * zoom),
                                            (int)lround(uid_num(d, id, "y", 0) * zoom),
                                            (int)lround(uid_num(d, id, "width", 160) * zoom),
                                            (int)lround(uid_num(d, id, "height", 36) * zoom)});
}
static xui_widget builtin_child(xui_widget w, const char *slot, void *u) {
    (void)u;
    return slot && !strcmp(slot, "content") ? w : NULL;
}
int uidXuiRegisterBuiltins(uid_context *c) {
    int i, r;
    uid_xui_adapter a;
    static const char *types[] = {"xui.form",  "xui.panel",  "xui.label",
                                  "xui.input", "xui.button", "xui.checkbox"};
    if (!uidFindControl(c, types[0]) && (r = uidRegisterBuiltins(c)))
        return r;
    for (i = 0; i < 6; ++i) {
        a = (uid_xui_adapter){sizeof(a),
                              types[i],
                              builtin_create,
                              builtin_apply,
                              i <= 1 ? builtin_child : NULL,
                              (void *)(intptr_t)i,
                              types[i],
                              NULL};
        if ((r = uidXuiRegisterAdapter(c, &a)))
            return r;
    }
    return UID_OK;
}
static void dispatch(uid_instance *i, const char *event, uid_value value) {
    uid_binding binding;
    uid_node_info node;
    uid_runtime_event e;
    uid_preview *p = i->scene;
    if (!p || p->design || !p->host.dispatch ||
        uidGetBinding(p->document, i->id, event, &binding) || uidNodeGet(p->document, i->id, &node))
        return;
    e = (uid_runtime_event){i->id, node.name, event, binding.handler, value, i->widget};
    p->host.dispatch(&e, p->host.user);
}
static void custom_emit(const char *event, uid_value value, void *u) {
    uid_instance *i = u;
    uid_node_info n;
    const uid_event_desc *e;
    if (!i || uidNodeGet(i->scene->document, i->id, &n))
        return;
    e = uidFindEvent(i->scene->context, n.type, event);
    if (e && e->argument_kind == value.kind)
        dispatch(i, event, value);
}
static void click(xui_widget w, void *u) {
    (void)w;
    dispatch(u, "click", (uid_value){0});
}
static void input(xui_widget w, const char *s, void *u) {
    (void)w;
    dispatch(u, "textChanged", uidString(s));
}
static void checked(xui_widget w, int b, void *u) {
    (void)w;
    dispatch(u, "checkedChanged", uidBool(b));
}
xui_widget uidPreviewNodeWidget(const uid_preview *p, uid_id id) {
    int i;
    for (i = 0; p && i < p->count; ++i)
        if (p->items[i].id == id)
            return p->items[i].widget;
    return NULL;
}
xui_widget uidPreviewRoot(const uid_preview *p) {
    return p ? p->root : NULL;
}
int uid_scene_create(uid_context *c, const uid_document *d, xui_context xui, xui_widget parent,
                     xui_font font, const uid_runtime_host *host, int design, float zoom,
                     uid_preview **out) {
    uid_preview *p;
    uid_node_info n, pn;
    const uid_xui_adapter *a, *pa;
    uid_instance *item;
    xui_widget target, w;
    xui_font f;
    unsigned char done[UID_MAX_NODES] = {0};
    int i, pass, progress, r;
    if (!c || !d || d->context != c || !xui || !parent || !out || !d->count ||
        (host && host->size != sizeof(*host)))
        return UID_INVALID;
    *out = NULL;
    p = calloc(1, sizeof(*p));
    if (!p)
        return UID_ERROR;
    p->context = c;
    p->document = d;
    p->xui = xui;
    p->font = font;
    p->design = design;
    p->zoom = zoom;
    if (host)
        p->host = *host;
    for (pass = 0; pass < d->count; ++pass) {
        progress = 0;
        for (i = 0; i < d->count; ++i) {
            if (done[i])
                continue;
            uidNodeAt(d, i, &n);
            target = parent;
            if (n.parent) {
                target = uidPreviewNodeWidget(p, n.parent);
                if (!target)
                    continue;
                uidNodeGet(d, n.parent, &pn);
                pa = uid_adapter(c, pn.type);
                if ((!pa || !pa->child_host) && !design) {
                    r = uid_fail(c, UID_UNSUPPORTED,
                                 "Container has no registered child-slot adapter");
                    goto fail;
                }
                if (pa && pa->child_host)
                    target = pa->child_host(target, n.slot, pa->user);
                if (!target) {
                    r = uid_fail(c, UID_UNSUPPORTED, "Unknown container slot");
                    goto fail;
                }
            }
            w = NULL;
            a = uid_adapter(c, n.type);
            f = host && host->font
                    ? host->font((float)(uid_num(d, n.id, "fontSize", 18) * zoom), host->user)
                    : font;
            if (!f)
                f = font;
            if (!a) {
                if (!design) {
                    r = uid_fail(c, UID_UNSUPPORTED,
                                 "Preview needs a registered adapter for every control");
                    goto fail;
                }
                xui_label_desc_t desc = {0};
                desc.iSize = sizeof(desc);
                desc.pFont = f;
                desc.sText = "[Missing control extension]";
                desc.iTextColor = 0xBB3333FFu;
                r = xuiLabelCreate(xui, &w, &desc);
                if (!r)
                    r = builtin_apply(w, d, n.id, zoom, NULL);
            } else {
                r = a->create(xui, f, d, n.id, &w, a->user);
                if (!r)
                    r = a->apply(w, d, n.id, zoom, a->user);
            }
            if (r) {
                if (w)
                    xuiWidgetDestroy(w);
                goto fail;
            }
            r = xuiWidgetAddChild(target, w);
            if (r) {
                xuiWidgetDestroy(w);
                goto fail;
            }
            item = &p->items[p->count++];
            *item = (uid_instance){p, n.id, w};
            if (!n.parent)
                p->root = w;
            if (design)
                xuiWidgetSetHitTestVisible(w, 0);
            else {
                xuiWidgetSetVisible(w, (int)uid_num(d, n.id, "visible", 1));
                xuiWidgetSetEnabled(w, (int)uid_num(d, n.id, "enabled", 1));
                if (a && a->bind_events) {
                    r = a->bind_events(w, custom_emit, item, a->user);
                    if (r)
                        goto fail;
                } else if (a) {
                    const char *runtime =
                        a->c_runtime_type && *a->c_runtime_type ? a->c_runtime_type : n.type;
                    if (!strcmp(runtime, "xui.button"))
                        xuiButtonSetClick(w, click, item);
                    else if (!strcmp(runtime, "xui.input"))
                        xuiInputSetChange(w, input, item);
                    else if (!strcmp(runtime, "xui.checkbox"))
                        xuiCheckBoxSetChange(w, checked, item);
                }
            }
            done[i] = 1;
            ++progress;
        }
        if (p->count == d->count) {
            *out = p;
            return UID_OK;
        }
        if (!progress)
            break;
    }
    r = UID_INVALID;
fail:
    uid_scene_destroy(p);
    return r;
}
void uid_scene_destroy(uid_preview *p) {
    if (!p)
        return;
    if (p->root)
        xuiWidgetDestroy(p->root);
    free(p);
}
int uidPreviewCreate(uid_context *c, const uid_snapshot *s, xui_context xui, xui_widget parent,
                     xui_font font, const uid_runtime_host *host, uid_preview **out) {
    uid_snapshot *copy;
    int r;
    if (!s || !out || s->data.context != c)
        return UID_INVALID;
    *out = NULL;
    r = uidSnapshotCreate(&s->data, &copy);
    if (r)
        return r;
    r = uid_scene_create(c, &copy->data, xui, parent, font, host, 0, 1, out);
    if (r) {
        uidSnapshotDestroy(copy);
        return r;
    }
    (*out)->snapshot = copy;
    ++c->views;
    return UID_OK;
}
void uidPreviewDestroy(uid_preview *p) {
    uid_snapshot *s;
    uid_context *c;
    if (!p)
        return;
    s = p->snapshot;
    c = p->context;
    uid_scene_destroy(p);
    if (s) {
        uidSnapshotDestroy(s);
        --c->views;
    }
}
