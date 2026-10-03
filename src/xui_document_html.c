#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include <stdio.h>
#include <ctype.h>

typedef struct doc_html { char* text; size_t size, capacity; int error; } doc_html;
static void doc_html_add(doc_html* b, const char* text, size_t size)
{
    size_t capacity; char* p;
    if (b->error) return;
    if (size > SIZE_MAX - b->size - 1) { b->error = XUI_DOC_ERROR_LIMIT; return; }
    if (b->capacity < b->size + size + 1) {
        capacity = b->capacity ? b->capacity : 1024;
        while (capacity < b->size + size + 1) {
            if (capacity > SIZE_MAX / 2) { capacity = b->size + size + 1; break; }
            capacity *= 2;
        }
        p = realloc(b->text, capacity);
        if (!p) { b->error = XUI_ERROR_OUT_OF_MEMORY; return; }
        b->text = p; b->capacity = capacity;
    }
    if (size) memcpy(b->text + b->size, text, size);
    b->size += size; b->text[b->size] = 0;
}
static void doc_html_put(doc_html* b, const char* text) { doc_html_add(b, text, strlen(text)); }
static void doc_html_escape(doc_html* b, const char* text, uint64_t size)
{
    uint64_t i, begin = 0;
    for (i = 0; i < size; i++) {
        const char* escape = text[i] == '&' ? "&amp;" : text[i] == '<' ? "&lt;" : text[i] == '>' ? "&gt;" :
            text[i] == '"' ? "&quot;" : text[i] == '\'' ? "&#39;" : text[i] == 0 ? "&#xfffd;" : NULL;
        if (escape) { doc_html_add(b, text + begin, (size_t)(i - begin)); doc_html_put(b, escape); begin = i + 1; }
    }
    doc_html_add(b, text + begin, (size_t)(size - begin));
}
static void doc_html_text(doc_html* b, doc_sequence* sequence)
{
    char part[1024]; uint64_t start = 0, size = doc_seq_size(sequence);
    while (start < size && !b->error) {
        uint64_t n = size - start < sizeof(part) ? size - start : sizeof(part);
        doc_seq_read(sequence, start, part, n); doc_html_escape(b, part, n); start += n;
    }
}
static void doc_html_attr(doc_html* b, const char* key, const char* value)
{
    doc_html_put(b, " "); doc_html_put(b, key); doc_html_put(b, "=\"");
    doc_html_escape(b, value, strlen(value)); doc_html_put(b, "\"");
}
static void doc_html_number(doc_html* b, const char* key, uint64_t n)
{
    char number[32]; snprintf(number, sizeof(number), "%llu", (unsigned long long)n); doc_html_attr(b, key, number);
}
static void doc_html_css_px(doc_html* b, const char* property, float value)
{
    char number[64]; size_t bytes = 0;
    if (!xrtNumWrite((double)value, number, sizeof(number), &bytes,
        XNUMBER_FLOAT_COMPACT)) { b->error = XUI_DOC_ERROR_SCHEMA; return; }
    doc_html_put(b, property);
    doc_html_add(b, number, bytes);
    doc_html_put(b, "px;");
}
int doc_html_safe_url(const char* s, int image)
{
    char scheme[16]; size_t i;
    for (i = 0; s[i] && s[i] != ':' && s[i] != '/' && s[i] != '#' && s[i] != '?'; i++) {
        if ((unsigned char)s[i] <= 32 || i >= sizeof(scheme) - 1) return 0;
        scheme[i] = (char)tolower((unsigned char)s[i]);
    }
    if (s[i] != ':') return 1;
    scheme[i] = 0;
    return strcmp(scheme, "http") == 0 || strcmp(scheme, "https") == 0 || (!image && strcmp(scheme, "mailto") == 0);
}
static void doc_html_node(doc_html* b, doc_state* s, uint64_t id)
{
    static const struct { uint32_t bit; const char *open, *close; } marks[] = {
        {XUI_DOC_BOLD, "<strong>", "</strong>"}, {XUI_DOC_ITALIC, "<em>", "</em>"},
        {XUI_DOC_UNDERLINE, "<u>", "</u>"}, {XUI_DOC_STRIKE, "<del>", "</del>"},
        {XUI_DOC_CODE, "<code>", "</code>"}, {XUI_DOC_SUBSCRIPT, "<sub>", "</sub>"},
        {XUI_DOC_SUPERSCRIPT, "<sup>", "</sup>"}, {XUI_DOC_HIGHLIGHT, "<mark>", "</mark>"}
    };
    doc_node* n = doc_index_get(s->index, id);
    const char* tag = NULL; char heading[16], style[80]; uint64_t i; int link = 0;
    if (!n || b->error) return;
    switch (n->kind) {
    case XUI_DOC_ROOT: if (n->attrs->sLanguage) tag = "div"; break;
    case XUI_DOC_PARAGRAPH: tag = "p"; break;
    case XUI_DOC_HEADING: snprintf(heading, sizeof(heading), "h%u", n->attrs->iHeadingLevel ? n->attrs->iHeadingLevel : 1); tag = heading; break;
    case XUI_DOC_QUOTE: tag = "blockquote"; break;
    case XUI_DOC_LIST: tag = n->attrs->iFlags & XUI_DOC_ORDERED ? "ol" : "ul"; break;
    case XUI_DOC_LIST_ITEM: tag = "li"; break;
    case XUI_DOC_TABLE: tag = "table"; break;
    case XUI_DOC_ROW: tag = "tr"; break;
    case XUI_DOC_CELL: tag = n->attrs->iFlags & XUI_DOC_HEADER ? "th" : "td"; break;
    case XUI_DOC_FOOTNOTE: tag = "aside"; break;
    case XUI_DOC_TEXT: tag = "span"; break;
    case XUI_DOC_SOFT_BREAK: doc_html_put(b, "\n"); return;
    case XUI_DOC_HARD_BREAK: doc_html_put(b, "<br>\n"); return;
    case XUI_DOC_RULE: doc_html_put(b, "<hr>\n"); return;
    case XUI_DOC_IMAGE:
        {
        int image_link = (n->attrs->iMarks & XUI_DOC_LINK) &&
            doc_html_safe_url(doc_string(n->link_target), 0);
        if (image_link) {
            doc_html_put(b, "<a"); doc_html_attr(b, "href", doc_string(n->link_target));
            if (n->link_title) doc_html_attr(b, "title", doc_string(n->link_title));
            doc_html_put(b, ">");
        }
        doc_html_put(b, "<img");
        if (n->attrs->sLanguage) doc_html_attr(b, "lang", n->attrs->sLanguage);
        if (doc_html_safe_url(doc_string(n->resource), 1)) doc_html_attr(b, "src", doc_string(n->resource));
        doc_html_put(b, " alt=\""); doc_html_text(b, n->text); doc_html_put(b, "\"");
        if (n->title) doc_html_attr(b, "title", doc_string(n->title));
        if (n->attrs->fWidth > 0 || n->attrs->fHeight > 0) {
            doc_html_put(b, " style=\"");
            if (n->attrs->fWidth > 0) doc_html_css_px(b, "width:", n->attrs->fWidth);
            if (n->attrs->fHeight > 0) doc_html_css_px(b, "height:", n->attrs->fHeight);
            doc_html_put(b, "\"");
        }
        doc_html_put(b, ">");
        if (image_link) doc_html_put(b, "</a>");
        return;
        }
    case XUI_DOC_CODE_BLOCK:
        doc_html_put(b, "<pre");
        if (n->attrs->sLanguage) doc_html_attr(b, "lang", n->attrs->sLanguage);
        doc_html_put(b, "><code");
        if (n->info) doc_html_attr(b, "data-language", doc_string(n->info));
        doc_html_put(b, ">"); doc_html_text(b, n->text); doc_html_put(b, "</code></pre>\n"); return;
    case XUI_DOC_MATH: tag = n->attrs->iFlags & XUI_DOC_BLOCK ? "div" : "span"; break;
    case XUI_DOC_DIAGRAM: case XUI_DOC_FRONT_MATTER: tag = "pre"; break;
    case XUI_DOC_HTML: case XUI_DOC_EXTENSION:
        tag = n->attrs->iFlags & XUI_DOC_BLOCK ? "pre" : "span"; break;
    case XUI_DOC_FOOTNOTE_REF: tag = "sup"; break;
    default: b->error = XUI_DOC_ERROR_SCHEMA; return;
    }
    if (tag) {
        doc_html_put(b, "<"); doc_html_put(b, tag);
        if (n->kind == XUI_DOC_ROOT) doc_html_attr(b, "data-xui-document", "true");
        if (n->attrs->sLanguage) doc_html_attr(b, "lang", n->attrs->sLanguage);
        if (n->kind == XUI_DOC_CELL) {
            if (n->attrs->iRowSpan > 1) doc_html_number(b, "rowspan", n->attrs->iRowSpan);
            if (n->attrs->iColumnSpan > 1) doc_html_number(b, "colspan", n->attrs->iColumnSpan);
        }
        if (n->kind == XUI_DOC_LIST && n->attrs->iFlags & XUI_DOC_ORDERED && n->attrs->iListStart > 1) doc_html_number(b, "start", n->attrs->iListStart);
        if (n->kind == XUI_DOC_LIST_ITEM && n->attrs->iFlags & XUI_DOC_TASK) {
            doc_html_attr(b, "data-xui-task", "true");
            if (n->attrs->iFlags & XUI_DOC_CHECKED)
                doc_html_attr(b, "data-xui-checked", "true");
        }
        if (n->kind == XUI_DOC_QUOTE && n->info)
            doc_html_attr(b, "data-xui-info", doc_string(n->info));
        if (doc_text_color_set(n->attrs) ||
            (n->attrs->iFlags & XUI_DOC_TEXT_COLOR_CURRENT) ||
            doc_background_color_set(n->attrs) ||
            n->attrs->fFontSize > 0 || n->attrs->sFontFamily[0] ||
            (((n->kind == XUI_DOC_PARAGRAPH || n->kind == XUI_DOC_HEADING ||
                n->kind == XUI_DOC_CELL) &&
                (n->attrs->iAlignment ||
                    (n->attrs->iFlags & XUI_DOC_ALIGNMENT_EXPLICIT_LEFT))) ||
            ((n->kind == XUI_DOC_PARAGRAPH || n->kind == XUI_DOC_HEADING) &&
                (n->attrs->iFlags & XUI_DOC_SPACING_EXPLICIT)))) {
            doc_html_put(b, " style=\"");
            if ((n->kind == XUI_DOC_PARAGRAPH || n->kind == XUI_DOC_HEADING ||
                n->kind == XUI_DOC_CELL) &&
                (n->attrs->iAlignment ||
                    (n->attrs->iFlags & XUI_DOC_ALIGNMENT_EXPLICIT_LEFT))) {
                static const char* align[] = {"left", "center", "right", "justify"};
                doc_html_put(b, "text-align:");
                doc_html_put(b, align[n->attrs->iAlignment]);
                doc_html_put(b, ";");
            }
            if ((n->kind == XUI_DOC_PARAGRAPH || n->kind == XUI_DOC_HEADING) &&
                n->attrs->iFlags & XUI_DOC_SPACING_EXPLICIT) {
                doc_html_css_px(b, "margin-bottom:", n->attrs->fParagraphSpacing);
            }
            if (n->attrs->iFlags & XUI_DOC_TEXT_COLOR_CURRENT)
                doc_html_put(b, "color:currentColor;");
            else if (doc_text_color_set(n->attrs)) {
                snprintf(style, sizeof(style), "color:#%08x;", n->attrs->iTextColor);
                doc_html_put(b, style);
            }
            if (n->attrs->iFlags & XUI_DOC_BACKGROUND_COLOR_CURRENT)
                doc_html_put(b, "background-color:currentColor;");
            else if (doc_background_color_set(n->attrs)) {
                snprintf(style, sizeof(style), "background-color:#%08x;", n->attrs->iBackgroundColor);
                doc_html_put(b, style);
            }
            if (n->attrs->fFontSize > 0) {
                doc_html_css_px(b, "font-size:", n->attrs->fFontSize);
            }
            if (n->attrs->sFontFamily[0]) {
                const unsigned char* family = (const unsigned char*)n->attrs->sFontFamily;
                doc_html_put(b, "font-family:&quot;");
                for (; *family; family++) {
                    if (*family == '"' || *family == '\\' || *family < 32 || *family == 127) {
                        snprintf(style, sizeof(style), "\\%x ", *family);
                        doc_html_put(b, style);
                    } else doc_html_escape(b, (const char*)family, 1);
                }
                doc_html_put(b, "&quot;;");
            }
            doc_html_put(b, "\"");
        }
        if (n->kind == XUI_DOC_MATH) doc_html_attr(b, "data-xui-object", "math");
        if (n->kind == XUI_DOC_DIAGRAM) doc_html_attr(b, "data-xui-object", "diagram");
        if (n->kind == XUI_DOC_HTML) doc_html_attr(b, "data-xui-object", "html-source");
        if (n->kind == XUI_DOC_FRONT_MATTER) doc_html_attr(b, "data-xui-object", "front-matter");
        if (n->kind == XUI_DOC_EXTENSION) doc_html_attr(b, "data-xui-object", "extension");
        if (n->kind == XUI_DOC_FOOTNOTE) doc_html_attr(b, "data-xui-object", "footnote");
        if (n->kind == XUI_DOC_FOOTNOTE_REF) doc_html_attr(b, "data-xui-object", "footnote-ref");
        if (n->kind == XUI_DOC_MATH || n->kind == XUI_DOC_DIAGRAM ||
            n->kind == XUI_DOC_HTML || n->kind == XUI_DOC_FRONT_MATTER ||
            n->kind == XUI_DOC_EXTENSION || n->kind == XUI_DOC_FOOTNOTE ||
            n->kind == XUI_DOC_FOOTNOTE_REF) {
            if (n->resource) doc_html_attr(b, "data-xui-resource", doc_string(n->resource));
            if (n->info) doc_html_attr(b, "data-xui-info", doc_string(n->info));
            if (n->title) doc_html_attr(b, "data-xui-title", doc_string(n->title));
        }
        if (n->kind == XUI_DOC_EXTENSION) {
            doc_html_number(b, "data-xui-version", n->extension_version);
            if (n->extension_required) doc_html_attr(b, "data-xui-required", "1");
            if (n->extension_payload) {
                char* encoded = xrtBase64EncodeNew(n->extension_payload->data,
                    (size_t)n->extension_payload->size, NULL);
                if (!encoded) { b->error = XUI_ERROR_OUT_OF_MEMORY; return; }
                doc_html_attr(b, "data-xui-payload-base64", encoded);
                xrtFree(encoded);
            }
        }
        doc_html_put(b, ">");
    }
    if (n->kind == XUI_DOC_TABLE && n->column_widths) {
        uint32_t column, columns = (uint32_t)(n->column_widths->size / sizeof(float));
        doc_html_put(b, "<colgroup>");
        for (column = 0; column < columns; column++) {
            float width = doc_table_column_width(n, column);
            doc_html_put(b, "<col");
            if (width > 0) {
                doc_html_put(b, " style=\"");
                doc_html_css_px(b, "width:", width);
                doc_html_put(b, "\"");
            }
            doc_html_put(b, ">");
        }
        doc_html_put(b, "</colgroup>");
    }
    if (n->kind == XUI_DOC_LIST_ITEM && (n->attrs->iFlags & XUI_DOC_TASK)) doc_html_put(b,
        n->attrs->iFlags & XUI_DOC_CHECKED ? "<input type=\"checkbox\" disabled checked> " : "<input type=\"checkbox\" disabled> ");
    if (n->attrs->iMarks & XUI_DOC_LINK && doc_html_safe_url(doc_string(n->resource), 0)) {
        doc_html_put(b, "<a"); doc_html_attr(b, "href", doc_string(n->resource));
        if (n->title) doc_html_attr(b, "title", doc_string(n->title));
        doc_html_put(b, ">"); link = 1;
    }
    for (i = 0; i < sizeof(marks) / sizeof(marks[0]); i++) if (n->attrs->iMarks & marks[i].bit) doc_html_put(b, marks[i].open);
    doc_html_text(b, n->text);
    for (i = 0; i < doc_seq_size(n->children); i++) doc_html_node(b, s, doc_seq_get_id(n->children, i));
    for (i = sizeof(marks) / sizeof(marks[0]); i > 0; i--) if (n->attrs->iMarks & marks[i - 1].bit) doc_html_put(b, marks[i - 1].close);
    if (link) doc_html_put(b, "</a>");
    if (tag) { doc_html_put(b, "</"); doc_html_put(b, tag); doc_html_put(b, ">"); }
    if (!doc_inline_kind(n->kind)) doc_html_put(b, "\n");
}
int doc_html_export_children(xui_document_snapshot s, uint64_t parent_id,
    char** out, uint64_t* bytes)
{
    doc_html b = {0};
    doc_node* parent;
    uint64_t i;
    if (out) *out = NULL;
    if (bytes) *bytes = 0;
    if (!s || !out || !bytes) return XUI_ERROR_INVALID_ARGUMENT;
    parent = doc_index_get(s->state->index, parent_id);
    if (!parent) return XUI_ERROR_NOT_FOUND;
    if (parent_id == DOC_ROOT && parent->attrs->sLanguage)
        doc_html_node(&b, s->state, DOC_ROOT);
    else for (i = 0; i < doc_seq_size(parent->children); i++)
        doc_html_node(&b, s->state, doc_seq_get_id(parent->children, i));
    if (!b.error && !b.text) doc_html_put(&b, "");
    if (b.error) { free(b.text); return b.error; }
    *out = b.text; *bytes = b.size; return XUI_OK;
}
XUI_API int xuiDocumentExportHtml(xui_document_snapshot s, char** out, uint64_t* bytes)
{
    return doc_html_export_children(s, DOC_ROOT, out, bytes);
}

#endif
