#include "uid_internal.h"
#include "lib/xrt/xrt_config.h"
#include "lib/xrt/xrt.h"
#include <stdarg.h>
#include <limits.h>

void uid_append(uid_builder *b, const char *format, ...) {
    va_list a, c;
    int n;
    size_t need, capacity;
    char *p;
    if (b->error)
        return;
    va_start(a, format);
    va_copy(c, a);
    n = vsnprintf(NULL, 0, format, c);
    va_end(c);
    if (n < 0 || (size_t)n > UID_MAX_DOCUMENT_BYTES - b->size) {
        b->error = UID_LIMIT;
        va_end(a);
        return;
    }
    need = b->size + (size_t)n + 1;
    if (need > b->capacity) {
        capacity = b->capacity ? b->capacity : 1024;
        while (capacity < need)
            capacity *= 2;
        p = realloc(b->text, capacity);
        if (!p) {
            b->error = UID_ERROR;
            va_end(a);
            return;
        }
        b->text = p;
        b->capacity = capacity;
    }
    vsnprintf(b->text + b->size, b->capacity - b->size, format, a);
    va_end(a);
    b->size += (size_t)n;
}
void uid_quote(uid_builder *b, const char *s) {
    const unsigned char *p = (const unsigned char *)(s ? s : "");
    uid_append(b, "\"");
    for (; *p; ++p) {
        if (*p == '"' || *p == '\\')
            uid_append(b, "\\%c", *p);
        else if (*p < 32)
            uid_append(b, "\\u%04x", *p);
        else
            uid_append(b, "%c", *p);
    }
    uid_append(b, "\"");
}
int uidDocumentSerialize(const uid_document *d, char **out, size_t *size) {
    uid_builder b = {0};
    int i, j;
    const uid_node_i *n;
    if (!d || !out)
        return UID_INVALID;
    *out = NULL;
    if (size)
        *size = 0;
    uid_append(&b, "{\n  \"format\":\"xui.uidesign.component\",\"version\":1,\n  \"nodes\":[");
    for (i = 0; i < d->count; ++i) {
        n = &d->nodes[i];
        uid_append(&b, "%s\n    {\"id\":%llu,\"parent\":%llu,\"type\":", i ? "," : "",
                   (unsigned long long)n->id, (unsigned long long)n->parent);
        uid_quote(&b, n->type);
        uid_append(&b, ",\"name\":");
        uid_quote(&b, n->name);
        uid_append(&b, ",\"slot\":");
        uid_quote(&b, n->slot);
        uid_append(&b, ",\"properties\":[");
        for (j = 0; j < n->property_count; ++j) {
            const uid_prop_i *p = &n->properties[j];
            uid_append(&b, "%s{\"id\":", j ? "," : "");
            uid_quote(&b, p->id);
            uid_append(&b, ",\"kind\":%d,\"value\":", p->value.kind);
            if (p->value.kind == UID_STRING)
                uid_quote(&b, p->value.text);
            else if (!isfinite(p->value.number))
                b.error = UID_INVALID;
            else
                uid_append(&b, "%.17g", p->value.number);
            uid_append(&b, "}");
        }
        uid_append(&b, "],\"bindings\":[");
        for (j = 0; j < n->binding_count; ++j) {
            uid_append(&b, "%s{\"event\":", j ? "," : "");
            uid_quote(&b, n->bindings[j].event_id);
            uid_append(&b, ",\"handler\":");
            uid_quote(&b, n->bindings[j].handler);
            uid_append(&b, "}");
        }
        uid_append(&b, "]}");
    }
    uid_append(&b, "\n  ]\n}\n");
    if (b.error) {
        free(b.text);
        return b.error;
    }
    *out = b.text;
    if (size)
        *size = b.size;
    return UID_OK;
}
static xvalue *get(const xvalue *v, const char *key) {
    return xrtValueObjectGet(v, (xstrview){key, strlen(key)});
}
static int string_copy(const xvalue *v, char *s, size_t cap) {
    xstrview view;
    if (!v || !xrtValueGetString(v, &view) || view.Size >= cap || memchr(view.Data, 0, view.Size))
        return 0;
    memcpy(s, view.Data, view.Size);
    s[view.Size] = 0;
    return 1;
}
static int number(const xvalue *v, double *out) {
    int64 i;
    if (!v)
        return 0;
    if (xrtValueType(v) == XVALUE_INT) {
        if (!xrtValueGetInt(v, &i))
            return 0;
        *out = (double)i;
        return 1;
    }
    return xrtValueType(v) == XVALUE_FLOAT && xrtValueGetFloat(v, out) && isfinite(*out);
}
static int integer(const xvalue *v, uint64_t *out) {
    double n;
    if (!number(v, &n) || n < 0 || n > INT32_MAX || floor(n) != n)
        return 0;
    *out = (uint64_t)n;
    return 1;
}
static int bounded_depth(const char *s, size_t n) {
    size_t i;
    int depth = 0, quoted = 0, escape = 0;
    for (i = 0; i < n; ++i) {
        if (!s[i])
            return 0;
        if (quoted) {
            if (escape)
                escape = 0;
            else if (s[i] == '\\')
                escape = 1;
            else if (s[i] == '"')
                quoted = 0;
        } else if (s[i] == '"')
            quoted = 1;
        else if (s[i] == '[' || s[i] == '{') {
            if (++depth > 64)
                return 0;
        } else if (s[i] == ']' || s[i] == '}') {
            if (--depth < 0)
                return 0;
        }
    }
    return depth == 0 && !quoted;
}
int uid_parse(uid_document *d, const char *text, size_t bytes) {
    xvalue *root = NULL, *nodes, *item, *props, *p, *bindings, *b, *v;
    uid_document temp = {0};
    uid_node_i *n;
    uint64_t version, k;
    int i, j, h, parents;
    char format[64], value[UID_MAX_TEXT + 1];
    int result = UID_INVALID;
    if (!d || !text || !bytes || bytes > UID_MAX_DOCUMENT_BYTES || !bounded_depth(text, bytes))
        return uid_fail(d ? d->context : NULL, UID_INVALID, "Invalid or oversized design document");
    root = xrtJsonParse((xstrview){text, bytes});
    if (!root)
        goto fail;
    if (!string_copy(get(root, "format"), format, sizeof(format)) ||
        strcmp(format, "xui.uidesign.component") || !integer(get(root, "version"), &version) ||
        version != 1) {
        uid_fail(d->context, UID_UNSUPPORTED,
                 "Expected UIDesign component format v1; legacy documents are not overwritten or "
                 "implicitly converted");
        result = UID_UNSUPPORTED;
        goto fail;
    }
    nodes = get(root, "nodes");
    if (!nodes || xrtValueType(nodes) != XVALUE_ARRAY || xrtValueCount(nodes) > UID_MAX_NODES)
        goto fail;
    temp.context = d->context;
    temp.count = (int)xrtValueCount(nodes);
    temp.nodes = calloc((size_t)temp.count + 1, sizeof(*temp.nodes));
    if (!temp.nodes) {
        result = UID_ERROR;
        goto fail;
    }
    for (i = 0; i < temp.count; ++i) {
        n = &temp.nodes[i];
        item = xrtValueArrayAt(nodes, i);
        if (!integer(get(item, "id"), &n->id) || !n->id ||
            !integer(get(item, "parent"), &n->parent) ||
            !string_copy(get(item, "type"), n->type, sizeof(n->type)) || !*n->type ||
            !string_copy(get(item, "name"), n->name, sizeof(n->name)) ||
            !uidIdentifierValid(n->name) ||
            !string_copy(get(item, "slot"), n->slot, sizeof(n->slot)) || !*n->slot)
            goto fail;
        if (n->id > temp.next_node)
            temp.next_node = n->id;
        for (j = 0; j < i; ++j)
            if (temp.nodes[j].id == n->id || !strcmp(temp.nodes[j].name, n->name))
                goto fail;
        props = get(item, "properties");
        if (!props || xrtValueType(props) != XVALUE_ARRAY ||
            xrtValueCount(props) > UID_MAX_PROPERTIES)
            goto fail;
        n->property_count = (int)xrtValueCount(props);
        n->properties = calloc((size_t)n->property_count + 1, sizeof(*n->properties));
        if (!n->properties) {
            result = UID_ERROR;
            goto fail;
        }
        for (j = 0; j < n->property_count; ++j) {
            uid_prop_i *q = &n->properties[j];
            const uid_property_desc *schema;
            p = xrtValueArrayAt(props, j);
            if (!string_copy(get(p, "id"), q->id, sizeof(q->id)) || !*q->id ||
                !strcmp(q->id, "name") || !integer(get(p, "kind"), &k) || k < UID_STRING ||
                k > UID_COLOR)
                goto fail;
            for (h = 0; h < j; ++h)
                if (!strcmp(q->id, n->properties[h].id))
                    goto fail;
            q->value.kind = (int)k;
            v = get(p, "value");
            if (k == UID_STRING) {
                if (!string_copy(v, value, sizeof(value)))
                    goto fail;
                q->value.text = uid_dup(value);
                if (!q->value.text) {
                    result = UID_ERROR;
                    goto fail;
                }
            } else {
                if (!number(v, &q->value.number) ||
                    (k == UID_BOOL && q->value.number != 0 && q->value.number != 1) ||
                    (k == UID_COLOR && (q->value.number < 0 || q->value.number > 4294967295.0 ||
                                        floor(q->value.number) != q->value.number)))
                    goto fail;
            }
            schema = uidFindProperty(d->context, n->type, q->id);
            if (schema && (schema->initial.kind != q->value.kind ||
                           (k != UID_STRING && (q->value.number < schema->minimum ||
                                                q->value.number > schema->maximum))))
                goto fail;
        }
        bindings = get(item, "bindings");
        if (!bindings || xrtValueType(bindings) != XVALUE_ARRAY ||
            xrtValueCount(bindings) > UID_MAX_EVENTS)
            goto fail;
        n->binding_count = (int)xrtValueCount(bindings);
        for (j = 0; j < n->binding_count; ++j) {
            b = xrtValueArrayAt(bindings, j);
            if (!string_copy(get(b, "event"), n->bindings[j].event_id, UID_ID_CAPACITY) ||
                !*n->bindings[j].event_id ||
                !string_copy(get(b, "handler"), n->bindings[j].handler, UID_ID_CAPACITY) ||
                !uidIdentifierValid(n->bindings[j].handler))
                goto fail;
            for (h = 0; h < j; ++h)
                if (!strcmp(n->bindings[j].event_id, n->bindings[h].event_id))
                    goto fail;
        }
    }
    for (i = 0; i < temp.count; ++i) {
        const uid_control_desc *schema;
        n = &temp.nodes[i];
        if ((i == 0 && n->parent) || (i > 0 && !n->parent))
            goto fail;
        schema = uidFindControl(d->context, n->type);
        if (schema &&
            ((i == 0 && !(schema->flags & UID_ROOT)) || (i > 0 && (schema->flags & UID_ROOT))))
            goto fail;
        if (i == 0)
            continue;
        n = uid_node(&temp, n->parent);
        if (!n)
            goto fail;
        schema = uidFindControl(d->context, n->type);
        if (schema && !(schema->flags & UID_CONTAINER))
            goto fail;
        parents = 0;
        for (; n; n = uid_node(&temp, n->parent))
            if (++parents > temp.count)
                goto fail;
    }
    for (i = 0; i < temp.count; ++i) {
        n = &temp.nodes[i];
        for (j = 0; j < n->property_count; ++j) {
            const uid_property_desc *schema =
                uidFindProperty(d->context, n->type, n->properties[j].id);
            if (schema && schema->validate) {
                ++d->context->callbacks;
                result = schema->validate(&temp, n->id, &n->properties[j].value, schema->user);
                --d->context->callbacks;
                if (result)
                    goto fail;
                result = UID_INVALID;
            }
        }
    }
    uid_clear_nodes(d);
    d->nodes = temp.nodes;
    d->count = temp.count;
    if (d->next_node < temp.next_node)
        d->next_node = temp.next_node;
    temp.nodes = NULL;
    temp.count = 0;
    xrtValueRelease(root);
    return UID_OK;
fail:
    uid_clear_nodes(&temp);
    if (root)
        xrtValueRelease(root);
    return result == UID_INVALID
               ? uid_fail(
                     d->context, result,
                     "Malformed document: check ids, names, tree, types, properties and limits")
               : result;
}
