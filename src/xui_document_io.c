#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include <stdio.h>
#include <math.h>

#define DOC_NATIVE_SCHEMA_VERSION 7u

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
static int doc_json_column_width(const xvalue* value, float* out)
{
    uint64_t integer; double number;
    if (xrtValueIs(value, XVALUE_FLOAT)) {
        if (!xrtValueGetFloat(value, &number)) return 0;
    } else {
        if (!doc_json_u64(value, &integer)) return 0;
        number = (double)integer;
    }
    if (!isfinite(number) || number < 0 || number > DOC_MAX_LAYOUT_VALUE) return 0;
    *out = (float)number; return 1;
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
    if (!doc_json_string(w, "language", doc_language(a->sLanguage),
        strlen(doc_language(a->sLanguage)))) return 0;
    return xrtJsonWriterEnd(w);
#undef UINT_FIELD
#undef FLOAT_FIELD
}
static int doc_json_language(const xvalue* v, const char** out)
{
    xstrview text;
    if (!doc_json_text(v, &text, 1) || text.Size > 255 ||
        !doc_language_valid(text.Data)) return 0;
    *out = text.Size ? text.Data : NULL;
    return 1;
}
static int doc_read_attrs(const xvalue* v, xui_doc_attributes_t* a, uint64_t version)
{
    uint64_t n; double f; xstrview text;
#define UINT_FIELD(key, field, max) if (!doc_json_u64(doc_json_get(v, key), &n) || n > (max)) return 0; a->field = n
#define FLOAT_FIELD(key, field) do { const xvalue* value = doc_json_get(v, key); if (xrtValueIs(value, XVALUE_FLOAT)) { if (!xrtValueGetFloat(value, &f)) return 0; } else { if (!doc_json_u64(value, &n)) return 0; f = (double)n; } if (!isfinite(f) || f < 0 || f > DOC_MAX_LAYOUT_VALUE) return 0; a->field = (float)f; } while (0)
    if (!xrtValueIs(v, XVALUE_OBJECT)) return 0;
    UINT_FIELD("marks", iMarks, UINT32_MAX); UINT_FIELD("flags", iFlags, UINT32_MAX); UINT_FIELD("heading", iHeadingLevel, 6);
    UINT_FIELD("alignment", iAlignment, 3); UINT_FIELD("rowSpan", iRowSpan, 1024); UINT_FIELD("columnSpan", iColumnSpan, 1024);
    UINT_FIELD("listStart", iListStart, UINT64_MAX); UINT_FIELD("textColor", iTextColor, UINT32_MAX); UINT_FIELD("backgroundColor", iBackgroundColor, UINT32_MAX);
    FLOAT_FIELD("fontSize", fFontSize); FLOAT_FIELD("width", fWidth); FLOAT_FIELD("height", fHeight); FLOAT_FIELD("paragraphSpacing", fParagraphSpacing);
    if (!doc_json_text(doc_json_get(v, "fontFamily"), &text, 1) || text.Size >= sizeof(a->sFontFamily)) return 0;
    memcpy(a->sFontFamily, text.Data, text.Size); a->sFontFamily[text.Size] = 0;
    if (version >= 7) {
        if (!doc_json_language(doc_json_get(v, "language"), &a->sLanguage)) return 0;
    } else if (doc_json_get(v, "language")) return 0;
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
        !xrtJsonWriterUInt(w, n->kind) || !doc_write_attrs(w, n->attrs)) return 0;
    if (n->column_widths) {
        uint32_t column, count = (uint32_t)(n->column_widths->size / sizeof(float));
        if (!doc_json_name(w, "columnWidths") || !xrtJsonWriterArray(w)) return 0;
        for (column = 0; column < count; column++)
            if (!xrtJsonWriterFloat(w, doc_table_column_width(n, column))) return 0;
        if (!xrtJsonWriterEnd(w)) return 0;
    }
    text = doc_seq_string(s->allocator, n->text);
    if (!text) return 0;
    ok = doc_json_string(w, "text", text, (size_t)doc_seq_size(n->text)); doc_free(text);
    if (!ok || !doc_json_string(w, "resource", doc_string(n->resource), n->resource ? (size_t)n->resource->size : 0) ||
        !doc_json_string(w, "info", doc_string(n->info), n->info ? (size_t)n->info->size : 0) ||
        !doc_json_string(w, "title", doc_string(n->title), n->title ? (size_t)n->title->size : 0)) return 0;
    if (n->kind == XUI_DOC_IMAGE &&
        (!doc_json_string(w, "linkTarget", doc_string(n->link_target),
            n->link_target ? (size_t)n->link_target->size : 0) ||
         !doc_json_string(w, "linkTitle", doc_string(n->link_title),
            n->link_title ? (size_t)n->link_title->size : 0))) return 0;
    if (n->kind == XUI_DOC_EXTENSION) {
        if (!doc_json_name(w, "extensionVersion") || !xrtJsonWriterUInt(w, n->extension_version) ||
            !doc_json_name(w, "extensionRequired") || !xrtJsonWriterBool(w, !!n->extension_required)) return 0;
        if (n->extension_payload) {
            char* encoded = xrtBase64EncodeNew(n->extension_payload->data,
                (size_t)n->extension_payload->size, NULL);
            if (!encoded) return 0;
            ok = doc_json_string(w, "extensionPayloadBase64", encoded, strlen(encoded));
            xrtFree(encoded);
            if (!ok) return 0;
        }
    }
    if (!doc_json_name(w, "children") || !xrtJsonWriterArray(w)) return 0;
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
        doc_json_name(w, "schemaVersion") && xrtJsonWriterUInt(w, DOC_NATIVE_SCHEMA_VERSION) && doc_json_name(w, "profile") &&
        xrtJsonWriterUInt(w, s->state->profile);
    if (ok && s->state->profile == XUI_DOCUMENT_MARKDOWN) {
        ok = doc_json_name(w, "dialect") && xrtJsonWriterUInt(w, s->state->dialect);
        const char* language = doc_language(doc_index_get(s->state->index, DOC_ROOT)->attrs->sLanguage);
        ok = ok && doc_json_string(w, "language", language, strlen(language));
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
static int doc_read_extension_fields(const xvalue* value, uint64_t kind,
    xui_doc_node_desc_t* desc, void** payload)
{
    const xvalue *version_value = doc_json_get(value, "extensionVersion"),
        *required_value = doc_json_get(value, "extensionRequired"),
        *payload_value = doc_json_get(value, "extensionPayloadBase64");
    xstrview encoded;
    uint64_t version;
    size_t bytes = 0;
    _Bool required;
    *payload = NULL;
    if (kind != XUI_DOC_EXTENSION)
        return version_value || required_value || payload_value ? XUI_DOC_ERROR_FORMAT : XUI_OK;
    desc->iExtensionVersion = 1;
    if (version_value) {
        if (!doc_json_u64(version_value, &version) || !version || version > UINT32_MAX)
            return XUI_DOC_ERROR_FORMAT;
        desc->iExtensionVersion = (uint32_t)version;
    }
    if (required_value) {
        if (!xrtValueGetBool(required_value, &required)) return XUI_DOC_ERROR_FORMAT;
        desc->bExtensionRequired = required;
    }
    if (!payload_value) return XUI_OK;
    if (!doc_json_text(payload_value, &encoded, 1) ||
        !xrtBase64Decode(encoded.Data, encoded.Size, NULL, 0, &bytes, NULL))
        return XUI_DOC_ERROR_FORMAT;
    if (bytes > DOC_EXTENSION_MAX_PAYLOAD_BYTES) return XUI_DOC_ERROR_LIMIT;
    if (bytes && (!desc->sInfo || !*desc->sInfo)) return XUI_DOC_ERROR_SCHEMA;
    if (bytes) {
        *payload = xrtBase64DecodeNew(encoded.Data, encoded.Size, &bytes, NULL);
        if (!*payload) return XUI_ERROR_OUT_OF_MEMORY;
        desc->pExtensionPayload = *payload;
        desc->iExtensionPayloadBytes = bytes;
    }
    return XUI_OK;
}
static int doc_read_node(xui_document d, const xvalue* value, uint64_t version)
{
    xui_doc_node_desc_t desc = {0};
    xstrview text, resource, info, title, link_target = {0}, link_title = {0};
    uint64_t id, parent, kind, child; size_t i, count;
    const xvalue* children; const xvalue* column_widths;
    doc_node* n; void* payload = NULL; int result = XUI_OK;
    if (!xrtValueIs(value, XVALUE_OBJECT) || !doc_json_read_id(doc_json_get(value, "id"), &id) || !id || id == UINT64_MAX ||
        !doc_json_read_id(doc_json_get(value, "parent"), &parent) || !doc_json_u64(doc_json_get(value, "kind"), &kind) ||
        kind < XUI_DOC_ROOT || kind > XUI_DOC_EXTENSION || !doc_read_attrs(doc_json_get(value, "attributes"), &desc.tAttributes, version) ||
        !doc_json_text(doc_json_get(value, "text"), &text, 0) || !doc_json_text(doc_json_get(value, "resource"), &resource, 1) ||
        !doc_json_text(doc_json_get(value, "info"), &info, 1) || !doc_json_text(doc_json_get(value, "title"), &title, 1)) return XUI_DOC_ERROR_FORMAT;
    /* Before v3, zero RGBA meant "unset". Reject the new presence bits in
     * older envelopes so downgrading a file cannot silently change color. */
    if (version < 3 && (desc.tAttributes.iFlags &
        (XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO | XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO)))
        return XUI_DOC_ERROR_FORMAT;
    if (version < 4 && (desc.tAttributes.iFlags &
        (XUI_DOC_TEXT_COLOR_CURRENT | XUI_DOC_BACKGROUND_COLOR_CURRENT)))
        return XUI_DOC_ERROR_FORMAT;
    if (version < 5 && (desc.tAttributes.iFlags & XUI_DOC_ALIGNMENT_EXPLICIT_LEFT))
        return XUI_DOC_ERROR_FORMAT;
    {
        const xvalue *target_value = doc_json_get(value, "linkTarget"),
            *title_value = doc_json_get(value, "linkTitle");
        if (kind == XUI_DOC_IMAGE && version >= 6) {
            if (!doc_json_text(target_value, &link_target, 1) ||
                !doc_json_text(title_value, &link_title, 1)) return XUI_DOC_ERROR_FORMAT;
            if (!(desc.tAttributes.iMarks & XUI_DOC_LINK) &&
                (link_target.Size || link_title.Size)) return XUI_DOC_ERROR_FORMAT;
        } else if (target_value || title_value) return XUI_DOC_ERROR_FORMAT;
    }
    if (doc_index_get(d->state->index, id)) return XUI_DOC_ERROR_SCHEMA;
    if (!doc_schema_attrs((uint32_t)kind, &desc.tAttributes) || (!doc_text_kind((uint32_t)kind) && text.Size)) return XUI_DOC_ERROR_SCHEMA;
    if (text.Size > d->max_bytes || d->state->text_bytes > d->max_bytes - text.Size) return XUI_DOC_ERROR_LIMIT;
    children = doc_json_get(value, "children");
    if (!xrtValueIs(children, XVALUE_ARRAY)) return XUI_DOC_ERROR_FORMAT;
    count = xrtValueCount(children); if (count > d->max_nodes) return XUI_DOC_ERROR_LIMIT;
    desc.iSize = sizeof(desc); desc.iKind = (uint32_t)kind; desc.sText = text.Data; desc.iTextBytes = text.Size;
    desc.sResource = resource.Data; desc.sInfo = info.Data; desc.sTitle = title.Data;
    if (kind == XUI_DOC_IMAGE && (desc.tAttributes.iMarks & XUI_DOC_LINK)) {
        desc.sLinkTarget = link_target.Data;
        desc.sLinkTitle = link_title.Data;
    }
    result = doc_read_extension_fields(value, kind, &desc, &payload);
    if (result != XUI_OK) return result;
    n = doc_node_new(d->allocator, id, parent, &desc);
    xrtFree(payload);
    if (!n) return XUI_ERROR_OUT_OF_MEMORY;
    column_widths = doc_json_get(value, "columnWidths");
    if (column_widths) {
        float widths[1024]; size_t columns = xrtValueCount(column_widths);
        if (kind != XUI_DOC_TABLE || !xrtValueIs(column_widths, XVALUE_ARRAY) || !columns || columns > 1024)
            result = XUI_DOC_ERROR_FORMAT;
        for (i = 0; result == XUI_OK && i < columns; i++)
            if (!doc_json_column_width(xrtValueArrayGet(column_widths, i), &widths[i])) result = XUI_DOC_ERROR_FORMAT;
        if (result == XUI_OK) {
            n->column_widths = doc_blob_new(d->allocator, (const char*)widths, columns * sizeof(float));
            if (!n->column_widths) result = XUI_ERROR_OUT_OF_MEMORY;
        }
    }
    for (i = 0; result == XUI_OK && i < count; i++) {
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
    if (result == XUI_OK && id >= atomic_load(&d->allocator->next_node_id)) atomic_store(&d->allocator->next_node_id, id + 1);
    return result;
}
XUI_API int xuiDocumentDeserialize(const xui_doc_desc_t* desc, const char* data, uint64_t bytes, xui_document* out)
{
    xjsonreadconfig config; xvalue* json; const xvalue* nodes;
    xui_doc_desc_t options = {0}; xui_document d = NULL;
    xstrview format, source; uint64_t version, profile, max_string; size_t i, count; int result;
    if (out) *out = NULL;
    if (!out || (!data && bytes) || bytes > SIZE_MAX || (desc && desc->iSize != sizeof(*desc))) return XUI_ERROR_INVALID_ARGUMENT;
    if (desc) options = *desc;
    options.iSize = sizeof(options);
    if (bytes > UINT64_C(1024) * 1024 * 1024) return XUI_DOC_ERROR_LIMIT;
    xrtJsonReadConfigInit(&config); config.MaxDepth = 8;
    config.MaxInputBytes = (size_t)bytes;
    max_string = options.iMaxTextBytes ? options.iMaxTextBytes : UINT64_C(256) * 1024 * 1024;
    /* Base64 is larger than the decoded payload, and fixed metadata such as
     * the format name must remain readable even under a small text budget. */
    if (max_string < (DOC_EXTENSION_MAX_PAYLOAD_BYTES + 2) / 3 * 4)
        max_string = (DOC_EXTENSION_MAX_PAYLOAD_BYTES + 2) / 3 * 4;
    config.MaxStringBytes = (size_t)(max_string < bytes ? max_string : bytes);
    config.MaxValues = (size_t)bytes + 1; config.MaxContainerItems = (size_t)(options.iMaxNodes ? options.iMaxNodes : 1000000);
    if (config.MaxContainerItems < 64) config.MaxContainerItems = 64;
    json = xrtJsonRead(doc_json_view(data, (size_t)bytes), &config);
    if (!json) return XUI_DOC_ERROR_FORMAT;
    if (!xrtValueIs(json, XVALUE_OBJECT) || !doc_json_text(doc_json_get(json, "format"), &format, 1) ||
        format.Size != 12 || memcmp(format.Data, "xui-document", 12) || !doc_json_u64(doc_json_get(json, "schemaVersion"), &version) ||
        (version < 1 || version > DOC_NATIVE_SCHEMA_VERSION) ||
        !doc_json_u64(doc_json_get(json, "profile"), &profile) ||
        (profile != XUI_DOCUMENT_RICH && profile != XUI_DOCUMENT_MARKDOWN)) { xrtValueRelease(json); return XUI_DOC_ERROR_FORMAT; }
    options.iProfile = (uint32_t)profile;
    if (profile == XUI_DOCUMENT_MARKDOWN) {
        uint64_t dialect;
        if (!doc_json_u64(doc_json_get(json, "dialect"), &dialect) || dialect < XUI_MD_COMMONMARK || dialect > XUI_MD_EXTENDED) { xrtValueRelease(json); return XUI_DOC_ERROR_FORMAT; }
        options.iMarkdownDialect = (uint32_t)dialect;
    }
    result = xuiDocumentCreate(&options, &d);
    if (result != XUI_OK) { xrtValueRelease(json); return result; }
    /* Import builds a private, unpublished state in place. Classify its final
     * roots only after construction, including cleanup of an invalid import. */
    if (d->standby_history) { doc_memory_standby(d->state, 0); d->standby_history = 0; }
    doc_memory_current(d->state, NULL);
    if (profile == XUI_DOCUMENT_MARKDOWN) {
        struct xui_doc_transaction_t t = {0};
        const char* language = NULL;
        if ((version >= 7 && !doc_json_language(doc_json_get(json, "language"), &language)) ||
            (version < 7 && doc_json_get(json, "language"))) result = XUI_DOC_ERROR_FORMAT;
        else if (!doc_json_text(doc_json_get(json, "source"), &source, 0)) result = XUI_DOC_ERROR_FORMAT;
        else if (source.Size > d->max_bytes) result = XUI_DOC_ERROR_LIMIT;
        else {
            doc_node* root = doc_index_get(d->state->index, DOC_ROOT);
            xui_doc_attributes_t attrs = *root->attrs;
            attrs.sLanguage = language;
            result = doc_node_set_attrs(d->allocator, root, &attrs);
            if (result != XUI_OK) goto imported;
            t.document = d; t.domain = XUI_DOC_SOURCE; t.draft = d->state; doc_state_retain(t.draft);
            t.draft->source = doc_seq_text(d->allocator, source.Data, source.Size);
            t.draft->source_open_brackets = doc_source_open_bracket_count(source.Data, source.Size);
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
            for (i = 0; i < count && result == XUI_OK; i++)
                result = doc_read_node(d, xrtValueArrayGet(nodes, i), version);
            if (result == XUI_OK) result = doc_schema_validate(d->state);
        }
    }
imported:
    doc_memory_current(NULL, d->state);
    if (result == XUI_OK && !d->disable_history) {
        doc_memory_standby(d->state, 1); d->standby_history = 1;
    }
    xrtValueRelease(json);
    if (result != XUI_OK) { xuiDocumentRelease(d); return result; }
    d->state->content_id = d->saved_state = d->next_state = 1;
    *out = d; return XUI_OK;
}

#endif
