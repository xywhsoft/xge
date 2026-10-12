#include "uid_internal.h"
#include <ctype.h>

uid_value uidString(const char *s) {
    return (uid_value){UID_STRING, 0, s ? s : ""};
}
uid_value uidNumber(double n) {
    return (uid_value){UID_NUMBER, n, NULL};
}
uid_value uidBool(int b) {
    return (uid_value){UID_BOOL, !!b, NULL};
}
uid_value uidColor(uint32_t c) {
    return (uid_value){UID_COLOR, (double)c, NULL};
}
void uidFree(void *p) {
    free(p);
}
char *uid_dup(const char *s) {
    size_t n;
    char *p;
    if (!s)
        s = "";
    n = strlen(s) + 1;
    p = malloc(n);
    if (p)
        memcpy(p, s, n);
    return p;
}
int uid_fail(uid_context *c, int code, const char *s) {
    if (c)
        snprintf(c->error, sizeof(c->error), "%s", s ? s : "");
    return code;
}
const char *uidLastError(const uid_context *c) {
    return c ? c->error : "Invalid context";
}
int uidIdentifierValid(const char *s) {
    const unsigned char *p = (const unsigned char *)s;
    if (!p || !(*p == '_' || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) ||
        strlen(s) >= UID_ID_CAPACITY)
        return 0;
    for (++p; *p; ++p)
        if (!(*p == '_' || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9')))
            return 0;
    return 1;
}
static int key_ok(const char *s) {
    return s && *s && strlen(s) < UID_ID_CAPACITY;
}
int uidGetPropertyAt(const uid_document *d, uid_id id, int index, const char **key,
                     uid_value *out) {
    uid_node_i *n = uid_node(d, id);
    if (!n || !key || !out || index < 0 || index >= n->property_count)
        return UID_NOT_FOUND;
    *key = n->properties[index].id;
    *out = n->properties[index].value;
    return UID_OK;
}
int uid_value_copy(uid_value *dst, const uid_value *src) {
    *dst = *src;
    if (src->kind == UID_STRING) {
        dst->text = uid_dup(src->text);
        if (!dst->text)
            return UID_ERROR;
    }
    return UID_OK;
}
void uid_value_clear(uid_value *v) {
    if (v->kind == UID_STRING)
        free((void *)v->text);
    memset(v, 0, sizeof(*v));
}
static void control_clear(uid_control_desc *d) {
    int i;
    uid_property_desc *p = (uid_property_desc *)d->properties;
    uid_event_desc *e = (uid_event_desc *)d->events;
    free((void *)d->id);
    free((void *)d->name);
    free((void *)d->name_prefix);
    free((void *)d->default_event);
    for (i = 0; p && i < d->property_count; ++i) {
        free((void *)p[i].id);
        free((void *)p[i].name);
        free((void *)p[i].category);
        free((void *)p[i].description);
        uid_value_clear(&p[i].initial);
    }
    for (i = 0; e && i < d->event_count; ++i) {
        free((void *)e[i].id);
        free((void *)e[i].name);
        free((void *)e[i].description);
    }
    free(p);
    free(e);
    memset(d, 0, sizeof(*d));
}
int uidContextCreate(uid_context **out) {
    if (!out)
        return UID_INVALID;
    *out = calloc(1, sizeof(**out));
    return *out ? UID_OK : UID_ERROR;
}
int uidContextDestroy(uid_context *c) {
    int i;
    if (!c)
        return UID_OK;
    if (c->documents || c->snapshots || c->views || c->callbacks)
        return uid_fail(c, UID_BUSY,
                        "Destroy documents, snapshots and editors before their context");
    for (i = 0; i < c->control_count; ++i) {
        control_clear(&c->controls[i]);
        if (c->adapters[i] && c->free_adapter)
            c->free_adapter(c->adapters[i]);
    }
    free(c);
    return UID_OK;
}
int uidControlCount(const uid_context *c) {
    return c ? c->control_count : 0;
}
const uid_control_desc *uidControlAt(const uid_context *c, int i) {
    return c && i >= 0 && i < c->control_count ? &c->controls[i] : NULL;
}
const uid_control_desc *uidFindControl(const uid_context *c, const char *type) {
    int i;
    if (!c || !type)
        return NULL;
    for (i = 0; i < c->control_count; ++i)
        if (!strcmp(c->controls[i].id, type))
            return &c->controls[i];
    return NULL;
}
const uid_property_desc *uidFindProperty(const uid_context *c, const char *type, const char *id) {
    const uid_control_desc *d = uidFindControl(c, type);
    int i;
    if (d && id)
        for (i = 0; i < d->property_count; ++i)
            if (!strcmp(d->properties[i].id, id))
                return &d->properties[i];
    return NULL;
}
const uid_event_desc *uidFindEvent(const uid_context *c, const char *type, const char *id) {
    const uid_control_desc *d = uidFindControl(c, type);
    int i;
    if (d && id)
        for (i = 0; i < d->event_count; ++i)
            if (!strcmp(d->events[i].id, id))
                return &d->events[i];
    return NULL;
}
int uidRegisterControl(uid_context *c, const uid_control_desc *d) {
    uid_control_desc t = {0};
    uid_property_desc *p;
    uid_event_desc *e;
    int i, j;
    if (!c || !d || d->size != sizeof(*d) || d->version != UID_API_VERSION || !key_ok(d->id) ||
        !uidIdentifierValid(d->name_prefix) || d->property_count < 0 ||
        d->property_count > UID_MAX_PROPERTIES || d->event_count < 0 ||
        d->event_count > UID_MAX_EVENTS || (!d->properties && d->property_count) ||
        (!d->events && d->event_count))
        return UID_INVALID;
    if (c->callbacks)
        return UID_BUSY;
    if (uidFindControl(c, d->id))
        return uid_fail(c, UID_CONFLICT, "A control type cannot be replaced while registered");
    if (c->control_count == UID_CONTROLS)
        return UID_LIMIT;
    for (i = 0; i < d->property_count; ++i) {
        const uid_property_desc *v = &d->properties[i];
        if (!key_ok(v->id) || v->initial.kind < UID_STRING || v->initial.kind > UID_COLOR ||
            !isfinite(v->minimum) || !isfinite(v->maximum) || v->maximum < v->minimum)
            return UID_INVALID;
        if (v->initial.kind == UID_STRING) {
            if (v->initial.text && strlen(v->initial.text) > UID_MAX_TEXT)
                return UID_LIMIT;
        } else if (!isfinite(v->initial.number) || v->initial.number < v->minimum ||
                   v->initial.number > v->maximum ||
                   (v->initial.kind == UID_BOOL && v->initial.number != 0 &&
                    v->initial.number != 1) ||
                   (v->initial.kind == UID_COLOR &&
                    (v->initial.number < 0 || v->initial.number > 4294967295.0 ||
                     floor(v->initial.number) != v->initial.number)))
            return UID_INVALID;
        for (j = 0; j < i; ++j)
            if (!strcmp(v->id, d->properties[j].id))
                return UID_INVALID;
    }
    for (i = 0; i < d->event_count; ++i) {
        if (!key_ok(d->events[i].id) || d->events[i].argument_kind < 0 ||
            d->events[i].argument_kind > UID_COLOR)
            return UID_INVALID;
        for (j = 0; j < i; ++j)
            if (!strcmp(d->events[i].id, d->events[j].id))
                return UID_INVALID;
    }
    if (d->default_event && *d->default_event) {
        for (i = 0; i < d->event_count; ++i)
            if (!strcmp(d->default_event, d->events[i].id))
                break;
        if (i == d->event_count)
            return UID_INVALID;
    }
    t = *d;
    t.id = uid_dup(d->id);
    t.name = uid_dup(d->name);
    t.name_prefix = uid_dup(d->name_prefix);
    t.default_event = uid_dup(d->default_event);
    p = calloc((size_t)d->property_count + 1, sizeof(*p));
    e = calloc((size_t)d->event_count + 1, sizeof(*e));
    t.properties = p;
    t.events = e;
    if (!t.id || !t.name || !t.name_prefix || !t.default_event || !p || !e)
        goto fail;
    for (i = 0; i < d->property_count; ++i) {
        p[i] = d->properties[i];
        p[i].id = uid_dup(p[i].id);
        p[i].name = uid_dup(p[i].name);
        p[i].category = uid_dup(p[i].category);
        p[i].description = uid_dup(p[i].description);
        p[i].initial = (uid_value){0};
        if (uid_value_copy(&p[i].initial, &d->properties[i].initial) || !p[i].id || !p[i].name ||
            !p[i].category || !p[i].description)
            goto fail;
    }
    for (i = 0; i < d->event_count; ++i) {
        e[i] = d->events[i];
        e[i].id = uid_dup(e[i].id);
        e[i].name = uid_dup(e[i].name);
        e[i].description = uid_dup(e[i].description);
        if (!e[i].id || !e[i].name || !e[i].description)
            goto fail;
    }
    c->controls[c->control_count++] = t;
    return UID_OK;
fail:
    control_clear(&t);
    return uid_fail(c, UID_ERROR, "Cannot allocate control metadata");
}
int uidSubscribe(uid_context *c, uid_notify_proc fn, void *u, uint64_t *token) {
    int i;
    if (!c || !fn || !token)
        return UID_INVALID;
    for (i = 0; i < 32; ++i)
        if (!c->observers[i].token) {
            c->observers[i].token = ++c->next_subscription;
            c->observers[i].fn = fn;
            c->observers[i].user = u;
            *token = c->observers[i].token;
            return UID_OK;
        }
    return UID_LIMIT;
}
void uidUnsubscribe(uid_context *c, uint64_t t) {
    int i;
    if (c)
        for (i = 0; i < 32; ++i)
            if (c->observers[i].token == t)
                memset(&c->observers[i], 0, sizeof(c->observers[i]));
}
void uid_notify(uid_document *d, const char *name) {
    uid_context *c = d->context;
    uint64_t tokens[32];
    uid_notice n;
    int i;
    if (d->snapshot || d->constructing)
        return;
    n = (uid_notice){
        d, d->id, d->revision, name, uidDocumentDirty(d), uidCanUndo(d), uidCanRedo(d)};
    for (i = 0; i < 32; ++i)
        tokens[i] = c->observers[i].token;
    ++c->callbacks;
    for (i = 0; i < 32; ++i)
        if (tokens[i] && c->observers[i].token == tokens[i])
            c->observers[i].fn(&n, c->observers[i].user);
    --c->callbacks;
}
uid_node_i *uid_node(const uid_document *d, uid_id id) {
    int i;
    if (d)
        for (i = 0; i < d->count; ++i)
            if (d->nodes[i].id == id)
                return &d->nodes[i];
    return NULL;
}
static void node_clear(uid_node_i *n) {
    int j;
    for (j = 0; n->properties && j < n->property_count; ++j)
        uid_value_clear(&n->properties[j].value);
    free(n->properties);
    memset(n, 0, sizeof(*n));
}
void uid_clear_nodes(uid_document *d) {
    int i;
    for (i = 0; d->nodes && i < d->count; ++i)
        node_clear(&d->nodes[i]);
    free(d->nodes);
    d->nodes = NULL;
    d->count = 0;
}
static void history_clear(uid_history_i *h, int *n) {
    while (*n) {
        --*n;
        free(h[*n].before);
        free(h[*n].after);
        memset(&h[*n], 0, sizeof(*h));
    }
}
static void history_push(uid_history_i *h, int *n, uid_history_i v) {
    size_t bytes = strlen(v.before) + strlen(v.after) + 2;
    int i;
    for (i = 0; i < *n; ++i)
        bytes += strlen(h[i].before) + strlen(h[i].after) + 2;
    while (*n && (*n >= UID_HISTORY || bytes > UID_HISTORY_BYTES)) {
        bytes -= strlen(h[0].before) + strlen(h[0].after) + 2;
        free(h[0].before);
        free(h[0].after);
        memmove(h, h + 1, (size_t)(--*n) * sizeof(*h));
    }
    h[(*n)++] = v;
}
static void document_release(uid_document *d) {
    uid_clear_nodes(d);
    history_clear(d->undo, &d->undo_count);
    history_clear(d->redo, &d->redo_count);
    free(d->clean);
    free(d->before);
    --d->context->documents;
    free(d);
}
int uidDocumentDestroy(uid_document *d) {
    if (!d)
        return UID_OK;
    if (d->snapshot)
        return UID_INVALID;
    if (d->views || d->context->callbacks)
        return UID_BUSY;
    document_release(d);
    return UID_OK;
}
uint64_t uidDocumentId(const uid_document *d) {
    return d ? d->id : 0;
}
uint64_t uidDocumentRevision(const uid_document *d) {
    return d ? d->revision : 0;
}
uid_id uidDocumentRoot(const uid_document *d) {
    return d && d->count ? d->nodes[0].id : 0;
}
int uidNodeCount(const uid_document *d) {
    return d ? d->count : 0;
}
static int node_info(const uid_node_i *n, uid_node_info *out) {
    if (!n || !out)
        return UID_NOT_FOUND;
    *out = (uid_node_info){n->id,   n->parent,         n->type,         n->name,
                           n->slot, n->property_count, n->binding_count};
    return UID_OK;
}
int uidNodeAt(const uid_document *d, int i, uid_node_info *out) {
    return node_info(d && i >= 0 && i < d->count ? &d->nodes[i] : NULL, out);
}
int uidNodeGet(const uid_document *d, uid_id id, uid_node_info *out) {
    return node_info(uid_node(d, id), out);
}
uid_id uidNodeFindName(const uid_document *d, const char *s) {
    int i;
    if (d && s)
        for (i = 0; i < d->count; ++i)
            if (!strcmp(s, d->nodes[i].name))
                return d->nodes[i].id;
    return 0;
}
int uidGetProperty(const uid_document *d, uid_id id, const char *key, uid_value *out) {
    uid_node_i *n = uid_node(d, id);
    const uid_property_desc *p;
    int i;
    if (!n || !key || !out)
        return UID_NOT_FOUND;
    if (!strcmp(key, "name")) {
        *out = uidString(n->name);
        return UID_OK;
    }
    for (i = 0; i < n->property_count; ++i)
        if (!strcmp(key, n->properties[i].id)) {
            *out = n->properties[i].value;
            return UID_OK;
        }
    p = uidFindProperty(d->context, n->type, key);
    if (p) {
        *out = p->initial;
        return UID_OK;
    }
    return UID_NOT_FOUND;
}
double uid_num(const uid_document *d, uid_id id, const char *key, double v) {
    uid_value x;
    if (uidGetProperty(d, id, key, &x) || x.kind == UID_STRING || !isfinite(x.number))
        return v;
    /* Unknown extension data stays lossless, but is never blindly converted
       into platform integer geometry by the placeholder renderer. */
    if ((!strcmp(key, "x") || !strcmp(key, "y")) && (x.number < -8192 || x.number > 8192))
        return v;
    if ((!strcmp(key, "width") || !strcmp(key, "height")) && (x.number < 8 || x.number > 4096))
        return v;
    if (!strcmp(key, "fontSize") && (x.number < 10 || x.number > 96))
        return v;
    return x.number;
}
const char *uid_text(const uid_document *d, uid_id id, const char *key, const char *v) {
    uid_value x;
    return uidGetProperty(d, id, key, &x) == UID_OK && x.kind == UID_STRING ? x.text : v;
}
int uidGetBinding(const uid_document *d, uid_id id, const char *event, uid_binding *out) {
    uid_node_i *n = uid_node(d, id);
    int i;
    if (!n || !event || !out)
        return UID_INVALID;
    memset(out, 0, sizeof(*out));
    for (i = 0; i < n->binding_count; ++i)
        if (!strcmp(n->bindings[i].event_id, event)) {
            *out = n->bindings[i];
            return UID_OK;
        }
    return UID_NOT_FOUND;
}
int uidGetBindingAt(const uid_document *d, uid_id id, int i, uid_binding *out) {
    uid_node_i *n = uid_node(d, id);
    if (!n || !out || i < 0 || i >= n->binding_count)
        return UID_NOT_FOUND;
    *out = n->bindings[i];
    return UID_OK;
}
static int writable(uid_document *d) {
    if (!d || d->snapshot)
        return UID_INVALID;
    if (d->context->callbacks)
        return uid_fail(d->context, UID_BUSY,
                        "Document mutation from a notification or validator is not allowed");
    return UID_OK;
}
int uidBegin(uid_document *d, const char *name) {
    int r = writable(d);
    if (r)
        return r;
    if (d->before)
        return UID_BUSY;
    r = uidDocumentSerialize(d, &d->before, NULL);
    if (!r)
        snprintf(d->command, sizeof(d->command), "%s", name ? name : "Edit");
    return r;
}
int uidCommit(uid_document *d) {
    char *after = NULL;
    int r = writable(d);
    if (r)
        return r;
    if (!d->before)
        return UID_INVALID;
    r = uidDocumentSerialize(d, &after, NULL);
    if (r)
        return r;
    if (!strcmp(after, d->before)) {
        free(after);
        free(d->before);
        d->before = NULL;
        return UID_OK;
    }
    history_clear(d->redo, &d->redo_count);
    history_push(d->undo, &d->undo_count, (uid_history_i){d->before, after});
    d->before = NULL;
    ++d->revision;
    uid_notify(d, d->command);
    return UID_OK;
}
int uidCancel(uid_document *d) {
    char *before;
    int r = writable(d);
    if (r)
        return r;
    if (!d->before)
        return UID_INVALID;
    before = d->before;
    r = uid_parse(d, before, strlen(before));
    if (!r) {
        d->before = NULL;
        free(before);
    }
    return r;
}
static int finish(uid_document *d, int own, int result) {
    if (!own)
        return result;
    if (result) {
        int r = uidCancel(d);
        return r ? r : result;
    }
    result = uidCommit(d);
    if (result)
        uidCancel(d);
    return result;
}
int uidValidateProperty(const uid_document *d, uid_id id, const char *key, const uid_value *v) {
    uid_node_i *n = uid_node(d, id);
    const uid_property_desc *p;
    uid_id other;
    int r = UID_OK;
    if (!n || !key || !v)
        return UID_INVALID;
    if (!strcmp(key, "name")) {
        if (v->kind != UID_STRING || !uidIdentifierValid(v->text))
            return uid_fail(d->context, UID_INVALID,
                            "Name must be an identifier of at most 63 ASCII characters");
        other = uidNodeFindName(d, v->text);
        return other && other != id
                   ? uid_fail(d->context, UID_CONFLICT, "A control with this name already exists")
                   : UID_OK;
    }
    p = uidFindProperty(d->context, n->type, key);
    if (!p)
        return uid_fail(d->context, UID_UNSUPPORTED, "This property is not registered");
    if (p->flags & UID_READONLY)
        return UID_INVALID;
    if (v->kind != p->initial.kind)
        return uid_fail(d->context, UID_INVALID, "Property value has the wrong type");
    if (v->kind == UID_STRING) {
        if (!v->text || strlen(v->text) > UID_MAX_TEXT)
            return UID_LIMIT;
    } else if (!isfinite(v->number) || v->number < p->minimum || v->number > p->maximum ||
               (v->kind == UID_BOOL && v->number != 0 && v->number != 1) ||
               (v->kind == UID_COLOR && floor(v->number) != v->number))
        return uid_fail(d->context, UID_INVALID, "Property value is outside its allowed range");
    if (p->validate) {
        ++d->context->callbacks;
        r = p->validate(d, id, v, p->user);
        --d->context->callbacks;
    }
    return r;
}
int uidSetProperty(uid_document *d, uid_id id, const char *key, uid_value v) {
    uid_node_i *n;
    uid_prop_i *p;
    uid_value copy;
    int i, own, r = writable(d);
    if (r)
        return r;
    r = uidValidateProperty(d, id, key, &v);
    if (r)
        return r;
    if (!strcmp(key, "name"))
        return uidRenameNode(d, id, v.text);
    own = !d->before;
    if (own && (r = uidBegin(d, "Edit property")))
        return r;
    n = uid_node(d, id);
    for (i = 0; i < n->property_count; ++i)
        if (!strcmp(key, n->properties[i].id))
            break;
    if (i == n->property_count) {
        if (i >= UID_MAX_PROPERTIES)
            return finish(d, own, UID_LIMIT);
        p = realloc(n->properties, (size_t)(i + 1) * sizeof(*p));
        if (!p)
            return finish(d, own, UID_ERROR);
        n->properties = p;
        memset(&p[i], 0, sizeof(*p));
        snprintf(p[i].id, sizeof(p[i].id), "%s", key);
        ++n->property_count;
    }
    r = uid_value_copy(&copy, &v);
    if (!r) {
        uid_value_clear(&n->properties[i].value);
        n->properties[i].value = copy;
    }
    return finish(d, own, r);
}
int uidSetProperties(uid_document *d, const uid_change *changes, int count) {
    int i, r, own;
    if (!d || !changes || count < 0 || count > UID_MAX_NODES * UID_MAX_PROPERTIES)
        return UID_INVALID;
    own = !d->before;
    if (own && (r = uidBegin(d, "Edit selection")))
        return r;
    for (i = 0; i < count; ++i)
        if ((r = uidValidateProperty(d, changes[i].node, changes[i].property, &changes[i].value)))
            return finish(d, own, r);
    for (i = 0; i < count; ++i)
        if ((r = uidSetProperty(d, changes[i].node, changes[i].property, changes[i].value)))
            return finish(d, own, r);
    return finish(d, own, UID_OK);
}
int uidResetProperty(uid_document *d, uid_id id, const char *key) {
    uid_node_i *n = uid_node(d, id);
    const uid_property_desc *p = n ? uidFindProperty(d->context, n->type, key) : NULL;
    return p && strcmp(key, "name") ? uidSetProperty(d, id, key, p->initial) : UID_INVALID;
}
int uidRenameNode(uid_document *d, uid_id id, const char *name) {
    int own, r = writable(d);
    uid_value v = uidString(name);
    if (r)
        return r;
    r = uidValidateProperty(d, id, "name", &v);
    if (r)
        return r;
    own = !d->before;
    if (own && (r = uidBegin(d, "Rename control")))
        return r;
    snprintf(uid_node(d, id)->name, UID_ID_CAPACITY, "%s", name);
    return finish(d, own, UID_OK);
}
int uidAddNode(uid_document *d, const char *type, uid_id parent, const char *slot, uid_id *out) {
    const uid_control_desc *desc, *pd;
    uid_node_i *n, *pn;
    int own, i, r = writable(d), suffix = 1;
    char name[64];
    if (r)
        return r;
    if (!out)
        return UID_INVALID;
    *out = 0;
    desc = uidFindControl(d->context, type);
    if (!desc)
        return UID_NOT_FOUND;
    if (d->count >= UID_MAX_NODES || d->next_node >= INT32_MAX)
        return UID_LIMIT;
    pn = uid_node(d, parent);
    pd = pn ? uidFindControl(d->context, pn->type) : NULL;
    if ((d->count && (!pd || !(pd->flags & UID_CONTAINER))) ||
        (!d->count && (!(desc->flags & UID_ROOT) || parent)) ||
        (d->count && (desc->flags & UID_ROOT)))
        return UID_INVALID;
    if (slot && !key_ok(slot))
        return UID_INVALID;
    do {
        snprintf(name, sizeof(name), "%.48s%d", desc->name_prefix, suffix++);
    } while (uidNodeFindName(d, name));
    own = !d->before;
    if (own && (r = uidBegin(d, "Add control")))
        return r;
    n = realloc(d->nodes, (size_t)(d->count + 1) * sizeof(*n));
    if (!n)
        return finish(d, own, UID_ERROR);
    d->nodes = n;
    n = &n[d->count++];
    memset(n, 0, sizeof(*n));
    n->id = ++d->next_node;
    n->parent = parent;
    snprintf(n->name, sizeof(n->name), "%s", name);
    snprintf(n->type, sizeof(n->type), "%s", type);
    snprintf(n->slot, sizeof(n->slot), "%s", slot ? slot : "content");
    n->properties = calloc((size_t)desc->property_count + 1, sizeof(*n->properties));
    if (!n->properties)
        return finish(d, own, UID_ERROR);
    for (i = 0; i < desc->property_count; ++i) {
        if (!strcmp(desc->properties[i].id, "name"))
            continue;
        snprintf(n->properties[n->property_count].id, UID_ID_CAPACITY, "%s",
                 desc->properties[i].id);
        r = uid_value_copy(&n->properties[n->property_count].value, &desc->properties[i].initial);
        if (r)
            return finish(d, own, r);
        ++n->property_count;
    }
    *out = n->id;
    return finish(d, own, UID_OK);
}
int uidReparentNode(uid_document *d, uid_id id, uid_id parent, const char *slot) {
    uid_node_i *n = uid_node(d, id), *p = uid_node(d, parent), *a;
    const uid_control_desc *pd;
    int own, r = writable(d);
    if (r)
        return r;
    pd = p ? uidFindControl(d->context, p->type) : NULL;
    if (!n || !p || !n->parent || !pd || !(pd->flags & UID_CONTAINER) || !key_ok(slot))
        return UID_INVALID;
    for (a = p; a; a = uid_node(d, a->parent))
        if (a == n)
            return UID_INVALID;
    own = !d->before;
    if (own && (r = uidBegin(d, "Move into container")))
        return r;
    n->parent = parent;
    snprintf(n->slot, sizeof(n->slot), "%s", slot);
    return finish(d, own, UID_OK);
}
static int descendant(const uid_document *d, uid_id id, uid_id parent) {
    uid_node_i *n;
    int limit = 0;
    for (n = uid_node(d, id); n && limit++ < UID_MAX_NODES; n = uid_node(d, n->parent))
        if (n->id == parent)
            return 1;
    return 0;
}
int uidDeleteNode(uid_document *d, uid_id id) {
    int i, own, r = writable(d);
    unsigned char remove[UID_MAX_NODES] = {0};
    if (r)
        return r;
    if (!uid_node(d, id) || id == uidDocumentRoot(d))
        return UID_INVALID;
    own = !d->before;
    if (own && (r = uidBegin(d, "Delete control")))
        return r;
    for (i = 0; i < d->count; ++i)
        remove[i] = (unsigned char)descendant(d, d->nodes[i].id, id);
    for (i = d->count - 1; i >= 0; --i)
        if (remove[i]) {
            node_clear(&d->nodes[i]);
            memmove(&d->nodes[i], &d->nodes[i + 1], (size_t)(--d->count - i) * sizeof(*d->nodes));
        }
    return finish(d, own, UID_OK);
}
int uidSetBinding(uid_document *d, uid_id id, const char *event, const char *handler) {
    uid_node_i *n = uid_node(d, id);
    int i, own, r = writable(d);
    if (r)
        return r;
    if (!n || !uidFindEvent(d->context, n->type, event) || !handler ||
        (*handler && !uidIdentifierValid(handler)))
        return UID_INVALID;
    own = !d->before;
    if (own && (r = uidBegin(d, "Bind event")))
        return r;
    for (i = 0; i < n->binding_count; ++i)
        if (!strcmp(n->bindings[i].event_id, event))
            break;
    if (!*handler) {
        if (i < n->binding_count)
            memmove(&n->bindings[i], &n->bindings[i + 1],
                    (size_t)(--n->binding_count - i) * sizeof(n->bindings[0]));
    } else {
        if (i == n->binding_count) {
            if (i == UID_MAX_EVENTS)
                return finish(d, own, UID_LIMIT);
            ++n->binding_count;
        }
        snprintf(n->bindings[i].event_id, UID_ID_CAPACITY, "%s", event);
        snprintf(n->bindings[i].handler, UID_ID_CAPACITY, "%s", handler);
    }
    return finish(d, own, UID_OK);
}
int uidCanUndo(const uid_document *d) {
    return d && !d->before && d->undo_count > 0;
}
int uidCanRedo(const uid_document *d) {
    return d && !d->before && d->redo_count > 0;
}
static int travel(uid_document *d, int redo) {
    uid_history_i h;
    int r = writable(d);
    if (r)
        return r;
    if (d->before)
        return UID_BUSY;
    if (redo ? !d->redo_count : !d->undo_count)
        return UID_NOT_FOUND;
    h = redo ? d->redo[d->redo_count - 1] : d->undo[d->undo_count - 1];
    r = uid_parse(d, redo ? h.after : h.before, strlen(redo ? h.after : h.before));
    if (r)
        return r;
    if (redo) {
        --d->redo_count;
        history_push(d->undo, &d->undo_count, h);
    } else {
        --d->undo_count;
        history_push(d->redo, &d->redo_count, h);
    }
    ++d->revision;
    uid_notify(d, redo ? "Redo" : "Undo");
    return UID_OK;
}
int uidUndo(uid_document *d) {
    return travel(d, 0);
}
int uidRedo(uid_document *d) {
    return travel(d, 1);
}
int uidDocumentDirty(const uid_document *d) {
    char *s = NULL;
    int dirty;
    if (!d)
        return 0;
    if (!d->clean)
        return 1;
    if (uidDocumentSerialize(d, &s, NULL))
        return 1;
    dirty = strcmp(d->clean, s) != 0;
    free(s);
    return dirty;
}
int uidDocumentCreate(uid_context *c, const char *name, uid_document **out) {
    uid_document *d;
    uid_id id;
    int r;
    if (!c || !out || !uidIdentifierValid(name))
        return UID_INVALID;
    *out = NULL;
    if (c->callbacks)
        return UID_BUSY;
    d = calloc(1, sizeof(*d));
    if (!d)
        return UID_ERROR;
    d->context = c;
    d->id = ++c->next_doc;
    d->revision = 1;
    d->constructing = 1;
    ++c->documents;
    r = uidAddNode(d, "xui.form", 0, "content", &id);
    if (!r)
        r = uidRenameNode(d, id, name);
    if (r) {
        document_release(d);
        return r;
    }
    history_clear(d->undo, &d->undo_count);
    r = uidDocumentSerialize(d, &d->clean, NULL);
    if (r) {
        document_release(d);
        return r;
    }
    d->constructing = 0;
    *out = d;
    return UID_OK;
}
int uidDocumentLoad(uid_context *c, const char *json, size_t size, uid_document **out) {
    uid_document *d;
    int r;
    if (!c || !out || !json)
        return UID_INVALID;
    *out = NULL;
    if (c->callbacks)
        return UID_BUSY;
    d = calloc(1, sizeof(*d));
    if (!d)
        return UID_ERROR;
    d->context = c;
    d->id = ++c->next_doc;
    d->revision = 1;
    r = uid_parse(d, json, size);
    if (!r && !d->count)
        r = UID_INVALID;
    if (r) {
        uid_clear_nodes(d);
        free(d);
        return r;
    }
    ++c->documents;
    r = uidDocumentSerialize(d, &d->clean, NULL);
    if (r) {
        document_release(d);
        return r;
    }
    *out = d;
    return UID_OK;
}
int uidSnapshotCreate(const uid_document *d, uid_snapshot **out) {
    uid_snapshot *s;
    int r;
    if (!d || !out || d->before)
        return UID_BUSY;
    *out = NULL;
    s = calloc(1, sizeof(*s));
    if (!s)
        return UID_ERROR;
    s->data.context = d->context;
    s->data.snapshot = 1;
    s->source_id = d->id;
    s->revision = d->revision;
    s->data.id = d->id;
    s->data.revision = d->revision;
    r = uidDocumentSerialize(d, &s->json, NULL);
    if (!r)
        r = uid_parse(&s->data, s->json, strlen(s->json));
    if (r) {
        uid_clear_nodes(&s->data);
        free(s->json);
        free(s);
        return r;
    }
    ++d->context->snapshots;
    *out = s;
    return UID_OK;
}
void uidSnapshotDestroy(uid_snapshot *s) {
    if (s) {
        uid_clear_nodes(&s->data);
        --s->data.context->snapshots;
        free(s->json);
        free(s);
    }
}
const uid_document *uidSnapshotDocument(const uid_snapshot *s) {
    return s ? &s->data : NULL;
}
uint64_t uidSnapshotRevision(const uid_snapshot *s) {
    return s ? s->revision : 0;
}
int uidDocumentMarkSaved(uid_document *d, const uid_snapshot *s) {
    char *copy;
    int r = writable(d);
    if (r)
        return r;
    if (!s || s->source_id != d->id || s->data.context != d->context)
        return UID_INVALID;
    copy = uid_dup(s->json);
    if (!copy)
        return UID_ERROR;
    free(d->clean);
    d->clean = copy;
    uid_notify(d, "Saved");
    return UID_OK;
}
int uidAcceptResult(const uid_document *d, uint64_t id, uint64_t rev) {
    return d && d->id == id && d->revision == rev;
}

int uidRegisterBuiltins(uid_context *c) {
    static const char *types[] = {"xui.form",  "xui.panel",  "xui.label",
                                  "xui.input", "xui.button", "xui.checkbox"};
    static const char *names[] = {"Form", "Panel", "Label", "Input", "Button", "CheckBox"};
    static const char *prefix[] = {"Form", "panel", "label", "input", "button", "checkBox"};
    uid_property_desc p[16];
    uid_control_desc d;
    uid_event_desc event;
    int i, n, r;
    for (i = 0; i < 6; ++i) {
        n = 0;
#define PROP(ID, NAME, GROUP, HELP, VALUE, MIN, MAX, FLAGS)                                        \
    p[n++] = (uid_property_desc){ID, NAME, GROUP, HELP, VALUE, FLAGS, MIN, MAX, NULL, NULL}
        PROP("name", "(Name)", "Identity", "Stable code name; display text is independent.",
             uidString(""), 0, 0, 0);
        PROP("x", "X", "Layout", "Position in the parent content slot.", uidNumber(0), -8192, 8192,
             UID_MULTIEDIT);
        PROP("y", "Y", "Layout", "Position in the parent content slot.", uidNumber(0), -8192, 8192,
             UID_MULTIEDIT);
        PROP("width", "Width", "Layout", "Design width, independent of canvas zoom.",
             uidNumber(i == 0   ? 800
                       : i == 1 ? 300
                       : i == 2 ? 140
                                : 180),
             8, 4096, UID_MULTIEDIT);
        PROP("height", "Height", "Layout", "Design height, independent of canvas zoom.",
             uidNumber(i == 0   ? 480
                       : i == 1 ? 180
                                : 36),
             8, 4096, UID_MULTIEDIT);
        PROP("text", "Text", "Appearance", "Displayed text / form title.",
             uidString(i == 0   ? "New Form"
                       : i == 2 ? "Label"
                       : i == 4 ? "Button"
                       : i == 5 ? "CheckBox"
                                : ""),
             0, 0, UID_MULTIEDIT);
        PROP("fontSize", "Font size", "Appearance", "Font size supplied by the host font resolver.",
             uidNumber(18), 10, 48, UID_MULTIEDIT);
        PROP("textColor", "Text color", "Appearance", "RGBA color (#RRGGBBAA).",
             uidColor(0x233246FFu), 0, 4294967295.0, UID_MULTIEDIT);
        PROP("visible", "Visible", "Behavior", "Initial runtime visibility.", uidBool(1), 0, 1,
             UID_MULTIEDIT);
        PROP("enabled", "Enabled", "Behavior", "Initial runtime enabled state.", uidBool(1), 0, 1,
             UID_MULTIEDIT);
        if (i == 5)
            PROP("checked", "Checked", "Behavior", "Initial checkbox state.", uidBool(0), 0, 1,
                 UID_MULTIEDIT);
#undef PROP
        event = (uid_event_desc){i == 4   ? "click"
                                 : i == 3 ? "textChanged"
                                          : "checkedChanged",
                                 i == 4   ? "Click"
                                 : i == 3 ? "Text Changed"
                                          : "Checked Changed",
                                 "Double-click the control to create or navigate to a handler.",
                                 i == 4   ? 0
                                 : i == 3 ? UID_STRING
                                          : UID_BOOL};
        d = (uid_control_desc){sizeof(d),
                               UID_API_VERSION,
                               types[i],
                               names[i],
                               prefix[i],
                               i >= 3 ? event.id : "",
                               i <= 1 ? UID_CONTAINER | (i == 0 ? UID_ROOT : 0) : 0,
                               p,
                               n,
                               i >= 3 ? &event : NULL,
                               i >= 3 ? 1 : 0};
        r = uidRegisterControl(c, &d);
        if (r)
            return r;
    }
    return UID_OK;
}
