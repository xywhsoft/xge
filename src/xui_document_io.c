#include "xui_document_internal.h"
#include <stdio.h>
#include <math.h>

static xstrview doc_json_view(const char* s, size_t n)
{
    xstrview v = {s, n}; return v;
}
static xvalue* doc_json_get(const xvalue* v, const char* key)
{
    return xrtValueObjectGet(v, doc_json_view(key, strlen(key)));
}
static int doc_json_name(xjsonwriter* w, const char* key)
{
    return xrtJsonWriterName(w, doc_json_view(key, strlen(key)));
}
static int doc_json_string(xjsonwriter* w, const char* key, const char* s, size_t n)
{
    return doc_json_name(w, key) && xrtJsonWriterString(w, doc_json_view(s, n));
}
static int doc_json_id(xjsonwriter* w, uint64_t id)
{
    char number[32]; int n = snprintf(number, sizeof(number), "%llu", (unsigned long long)id);
    return n > 0 && xrtJsonWriterString(w, doc_json_view(number, (size_t)n));
}
static int doc_json_u64(const xvalue* v, uint64_t* out)
{
    if (xrtValueIs(v, XVALUE_UINT)) { uint64 value; if (!xrtValueGetUInt(v, &value)) return 0; *out = value; return 1; }
    if (xrtValueIs(v, XVALUE_INT)) { int64 value; if (!xrtValueGetInt(v, &value) || value < 0) return 0; *out = (uint64_t)value; return 1; }
    return 0;
}
static int doc_json_read_id(const xvalue* v, uint64_t* out)
{
    xstrview s = {0}; uint64_t value = 0; size_t i;
    if (!xrtValueGetString(v, &s) || !s.Size || s.Size > 20 || (s.Size > 1 && s.Data[0] == '0')) return 0;
    for (i = 0; i < s.Size; i++) {
        unsigned digit = (unsigned char)s.Data[i] - '0';
        if (digit > 9 || value > (UINT64_MAX - digit) / 10) return 0;
        value = value * 10 + digit;
    }
    *out = value; return 1;
}
static int doc_json_text(const xvalue* v, xstrview* out, int c_string)
{
    return xrtValueGetString(v, out) && doc_utf8(out->Data, out->Size) &&
        (!c_string || !memchr(out->Data, 0, out->Size));
}
static int doc_write_attrs(xjsonwriter* w, const xui_doc_attributes_t* a)
{
#define UINT_FIELD(key, field) if (!doc_json_name(w, key) || !xrtJsonWriterUInt(w, a->field)) return 0
#define FLOAT_FIELD(key, field) if (!doc_json_name(w, key) || !xrtJsonWriterFloat(w, a->field)) return 0
    if (!doc_json_name(w, "attributes") || !xrtJsonWriterObject(w)) return 0;
    UINT_FIELD("marks", iMarks); UINT_FIELD("flags", iFlags); UINT_FIELD("heading", iHeadingLevel);
    UINT_FIELD("alignment", iAlignment); UINT_FIELD("rowSpan", iRowSpan); UINT_FIELD("columnSpan", iColumnSpan);
    UINT_FIELD("listStart", iListStart); UINT_FIELD("textColor", iTextColor); UINT_FIELD("backgroundColor", iBackgroundColor);
    FLOAT_FIELD("fontSize", fFontSize); FLOAT_FIELD("width", fWidth); FLOAT_FIELD("height", fHeight); FLOAT_FIELD("paragraphSpacing", fParagraphSpacing);
    if (!doc_json_string(w, "fontFamily", a->sFontFamily, strlen(a->sFontFamily))) return 0;
    return xrtJsonWriterEnd(w);
#undef UINT_FIELD
#undef FLOAT_FIELD
}
static int doc_read_attrs(const xvalue* v, xui_doc_attributes_t* a)
{
    uint64_t n; double f; xstrview text;
#define UINT_FIELD(key, field, max) if (!doc_json_u64(doc_json_get(v, key), &n) || n > (max)) return 0; a->field = n
#define FLOAT_FIELD(key, field) do { const xvalue* value = doc_json_get(v, key); if (xrtValueIs(value, XVALUE_FLOAT)) { if (!xrtValueGetFloat(value, &f)) return 0; } else { if (!doc_json_u64(value, &n)) return 0; f = (double)n; } if (!isfinite(f) || f < 0 || f > 1000000.0) return 0; a->field = (float)f; } while (0)
    if (!xrtValueIs(v, XVALUE_OBJECT)) return 0;
    UINT_FIELD("marks", iMarks, UINT32_MAX); UINT_FIELD("flags", iFlags, UINT32_MAX); UINT_FIELD("heading", iHeadingLevel, 6);
    UINT_FIELD("alignment", iAlignment, 3); UINT_FIELD("rowSpan", iRowSpan, 1024); UINT_FIELD("columnSpan", iColumnSpan, 1024);
    UINT_FIELD("listStart", iListStart, UINT64_MAX); UINT_FIELD("textColor", iTextColor, UINT32_MAX); UINT_FIELD("backgroundColor", iBackgroundColor, UINT32_MAX);
    FLOAT_FIELD("fontSize", fFontSize); FLOAT_FIELD("width", fWidth); FLOAT_FIELD("height", fHeight); FLOAT_FIELD("paragraphSpacing", fParagraphSpacing);
    if (!doc_json_text(doc_json_get(v, "fontFamily"), &text, 1) || text.Size >= sizeof(a->sFontFamily)) return 0;
    memcpy(a->sFontFamily, text.Data, text.Size); a->sFontFamily[text.Size] = 0;
    return 1;
#undef UINT_FIELD
#undef FLOAT_FIELD
}
static int doc_write_node(xjsonwriter* w, doc_state* s, uint64_t id)
{
    doc_node* n = doc_index_get(s->index, id);
    char* text;
    uint64_t i;
    int ok;
    if (!n || !xrtJsonWriterObject(w) || !doc_json_name(w, "id") || !doc_json_id(w, id) ||
        !doc_json_name(w, "parent") || !doc_json_id(w, n->parent) || !doc_json_name(w, "kind") ||
        !xrtJsonWriterUInt(w, n->kind) || !doc_write_attrs(w, &n->attrs)) return 0;
    text = doc_seq_string(s->allocator, n->text);
    if (!text) return 0;
    ok = doc_json_string(w, "text", text, (size_t)doc_seq_size(n->text)); doc_free(text);
    if (!ok || !doc_json_string(w, "resource", doc_string(n->resource), n->resource ? (size_t)n->resource->size : 0) ||
        !doc_json_string(w, "info", doc_string(n->info), n->info ? (size_t)n->info->size : 0) ||
        !doc_json_string(w, "title", doc_string(n->title), n->title ? (size_t)n->title->size : 0) ||
        !doc_json_name(w, "children") || !xrtJsonWriterArray(w)) return 0;
    for (i = 0; i < doc_seq_size(n->children); i++) if (!doc_json_id(w, doc_seq_get_id(n->children, i))) return 0;
    if (!xrtJsonWriterEnd(w) || !xrtJsonWriterEnd(w)) return 0;
    for (i = 0; i < doc_seq_size(n->children); i++) if (!doc_write_node(w, s, doc_seq_get_id(n->children, i))) return 0;
    return 1;
}
XUI_API int xuiDocumentSerialize(xui_document_snapshot s, char** out, uint64_t* bytes)
{
    xjsonwriteconfig config; xjsonwriter* w; char* result = NULL; char* copy = NULL; size_t size = 0; int ok;
    if (out) *out = NULL;
    if (bytes) *bytes = 0;
    if (!s || !out || !bytes) return XUI_ERROR_INVALID_ARGUMENT;
    xrtJsonWriteConfigInit(&config); config.MaxOutputBytes = SIZE_MAX; config.MaxDepth = 8;
    w = xrtJsonWriterCreate(&config); if (!w) return XUI_ERROR_OUT_OF_MEMORY;
    ok = xrtJsonWriterObject(w) && doc_json_string(w, "format", "xui-document", 12) &&
        doc_json_name(w, "schemaVersion") && xrtJsonWriterUInt(w, 1) && doc_json_name(w, "profile") &&
        xrtJsonWriterUInt(w, s->state->profile);
    if (ok && s->state->profile == XUI_DOCUMENT_MARKDOWN) {
        ok = doc_json_name(w, "dialect") && xrtJsonWriterUInt(w, s->state->dialect);
        char* source = doc_seq_string(s->state->allocator, s->state->source);
        ok = ok && source && doc_json_string(w, "source", source, (size_t)doc_seq_size(s->state->source)); doc_free(source);
    } else if (ok) {
        ok = doc_json_name(w, "nodes") && xrtJsonWriterArray(w) && doc_write_node(w, s->state, DOC_ROOT) && xrtJsonWriterEnd(w);
    }
    if (ok && xrtJsonWriterEnd(w) && xrtJsonWriterFinish(w)) result = xrtJsonWriterTake(w, &size);
    xrtJsonWriterFree(w);
    if (!result) return XUI_ERROR_OUT_OF_MEMORY;
    if (size < SIZE_MAX) copy = malloc(size + 1);
    if (copy) memcpy(copy, result, size + 1);
    xrtFree(result);
    if (!copy) return XUI_ERROR_OUT_OF_MEMORY;
    *out = copy; *bytes = size; return XUI_OK;
}
static int doc_read_node(xui_document d, const xvalue* value)
{
    xui_doc_node_desc_t desc = {0};
    xstrview text, resource, info, title;
    uint64_t id, parent, kind, child; size_t i, count;
    const xvalue* children;
    doc_node* n; int result = XUI_OK;
    if (!xrtValueIs(value, XVALUE_OBJECT) || !doc_json_read_id(doc_json_get(value, "id"), &id) || !id || id == UINT64_MAX ||
        !doc_json_read_id(doc_json_get(value, "parent"), &parent) || !doc_json_u64(doc_json_get(value, "kind"), &kind) ||
        kind < XUI_DOC_ROOT || kind > XUI_DOC_EXTENSION || !doc_read_attrs(doc_json_get(value, "attributes"), &desc.tAttributes) ||
        !doc_json_text(doc_json_get(value, "text"), &text, 0) || !doc_json_text(doc_json_get(value, "resource"), &resource, 1) ||
        !doc_json_text(doc_json_get(value, "info"), &info, 1) || !doc_json_text(doc_json_get(value, "title"), &title, 1)) return XUI_DOC_ERROR_FORMAT;
    if (doc_index_get(d->state->index, id)) return XUI_DOC_ERROR_SCHEMA;
    if (!doc_schema_attrs((uint32_t)kind, &desc.tAttributes) || (!doc_text_kind((uint32_t)kind) && text.Size)) return XUI_DOC_ERROR_SCHEMA;
    if (text.Size > d->max_bytes || d->state->text_bytes > d->max_bytes - text.Size) return XUI_DOC_ERROR_LIMIT;
    children = doc_json_get(value, "children");
    if (!xrtValueIs(children, XVALUE_ARRAY)) return XUI_DOC_ERROR_FORMAT;
    count = xrtValueCount(children); if (count > d->max_nodes) return XUI_DOC_ERROR_LIMIT;
    desc.iSize = sizeof(desc); desc.iKind = (uint32_t)kind; desc.sText = text.Data; desc.iTextBytes = text.Size;
    desc.sResource = resource.Data; desc.sInfo = info.Data; desc.sTitle = title.Data;
    n = doc_node_new(d->allocator, id, parent, &desc); if (!n) return XUI_ERROR_OUT_OF_MEMORY;
    for (i = 0; i < count; i++) {
        doc_sequence *item, *next = NULL;
        if (!doc_json_read_id(xrtValueArrayGet(children, i), &child) || !child || child == DOC_ROOT) { result = XUI_DOC_ERROR_SCHEMA; break; }
        item = doc_seq_id(d->allocator, child);
        if (!item) { result = XUI_ERROR_OUT_OF_MEMORY; break; }
        result = doc_seq_replace(d->allocator, n->children, i, i, item, &next); doc_seq_release(item);
        if (result != XUI_OK) break;
        doc_seq_release(n->children); n->children = next;
    }
    if (result == XUI_OK) result = doc_state_set(d->state, n);
    doc_node_release(n);
    if (result == XUI_OK && id >= d->next_id) d->next_id = id + 1;
    return result;
}
XUI_API int xuiDocumentDeserialize(const xui_doc_desc_t* desc, const char* data, uint64_t bytes, xui_document* out)
{
    xjsonreadconfig config; xvalue* json; const xvalue* nodes;
    xui_doc_desc_t options = {0}; xui_document d = NULL;
    xstrview format, source; uint64_t version, profile; size_t i, count; int result;
    if (out) *out = NULL;
    if (!out || (!data && bytes) || bytes > SIZE_MAX || (desc && desc->iSize != sizeof(*desc))) return XUI_ERROR_INVALID_ARGUMENT;
    if (desc) options = *desc;
    options.iSize = sizeof(options);
    xrtJsonReadConfigInit(&config); config.MaxDepth = 8;
    config.MaxInputBytes = (size_t)bytes; config.MaxStringBytes = (size_t)(options.iMaxTextBytes ? options.iMaxTextBytes : UINT64_C(256) * 1024 * 1024);
    if (bytes > UINT64_C(1024) * 1024 * 1024) return XUI_DOC_ERROR_LIMIT;
    config.MaxValues = (size_t)bytes + 1; config.MaxContainerItems = (size_t)(options.iMaxNodes ? options.iMaxNodes : 1000000);
    if (config.MaxContainerItems < 64) config.MaxContainerItems = 64;
    json = xrtJsonRead(doc_json_view(data, (size_t)bytes), &config);
    if (!json) return XUI_DOC_ERROR_FORMAT;
    if (!xrtValueIs(json, XVALUE_OBJECT) || !doc_json_text(doc_json_get(json, "format"), &format, 1) ||
        format.Size != 12 || memcmp(format.Data, "xui-document", 12) || !doc_json_u64(doc_json_get(json, "schemaVersion"), &version) ||
        version != 1 || !doc_json_u64(doc_json_get(json, "profile"), &profile) ||
        (profile != XUI_DOCUMENT_RICH && profile != XUI_DOCUMENT_MARKDOWN)) { xrtValueRelease(json); return XUI_DOC_ERROR_FORMAT; }
    options.iProfile = (uint32_t)profile;
    if (profile == XUI_DOCUMENT_MARKDOWN && doc_json_get(json, "dialect")) {
        uint64_t dialect;
        if (!doc_json_u64(doc_json_get(json, "dialect"), &dialect) || dialect < XUI_MD_COMMONMARK || dialect > XUI_MD_EXTENDED) { xrtValueRelease(json); return XUI_DOC_ERROR_FORMAT; }
        options.iMarkdownDialect = (uint32_t)dialect;
    }
    result = xuiDocumentCreate(&options, &d);
    if (result != XUI_OK) { xrtValueRelease(json); return result; }
    if (profile == XUI_DOCUMENT_MARKDOWN) {
        struct xui_doc_transaction_t t = {0};
        if (!doc_json_text(doc_json_get(json, "source"), &source, 0)) result = XUI_DOC_ERROR_FORMAT;
        else if (source.Size > d->max_bytes) result = XUI_DOC_ERROR_LIMIT;
        else {
            t.document = d; t.domain = XUI_DOC_SOURCE; t.draft = d->state; doc_state_retain(t.draft);
            t.draft->source = doc_seq_text(d->allocator, source.Data, source.Size);
            if (source.Size && !t.draft->source) result = XUI_ERROR_OUT_OF_MEMORY;
            else result = doc_markdown_parse(&t);
            if (result == XUI_OK) { doc_state_release(d->state); d->state = t.draft; doc_state_retain(d->state); }
            doc_state_release(t.draft);
        }
    } else {
        nodes = doc_json_get(json, "nodes"); count = xrtValueCount(nodes);
        if (!xrtValueIs(nodes, XVALUE_ARRAY) || !count) result = XUI_DOC_ERROR_FORMAT;
        else if (count > d->max_nodes) result = XUI_DOC_ERROR_LIMIT;
        else {
            result = doc_state_remove(d->state, DOC_ROOT);
            for (i = 0; i < count && result == XUI_OK; i++) result = doc_read_node(d, xrtValueArrayGet(nodes, i));
            if (result == XUI_OK) result = doc_schema_validate(d->state);
        }
    }
    xrtValueRelease(json);
    if (result != XUI_OK) { xuiDocumentRelease(d); return result; }
    d->state->content_id = d->saved_state = d->next_state = 1;
    *out = d; return XUI_OK;
}
