#include "xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include "xui_document_md4c.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct doc_md_source_copy { uint64_t source, output, bytes; } doc_md_source_copy;
typedef struct doc_md_source_plan { doc_md_source_copy* copies; uint64_t count, capacity; } doc_md_source_plan;
typedef struct doc_md_output {
    doc_allocator* allocator; char* data; uint64_t size, capacity;
    int error; char list_marker; doc_md_source_plan* plan;
} doc_md_output;
static void doc_md_write(doc_md_output* b, const char* text, uint64_t bytes)
{
    uint64_t required, capacity; char* next;
    if (b->error) return;
    if (bytes >= SIZE_MAX || b->size >= SIZE_MAX - bytes) { b->error = XUI_DOC_ERROR_LIMIT; return; }
    required = b->size + bytes + 1;
    if (required > b->capacity) {
        capacity = b->capacity ? b->capacity : 128;
        while (capacity < required) { if (capacity > SIZE_MAX / 2) { capacity = required; break; } capacity *= 2; }
        next = doc_realloc(b->allocator, b->data, (size_t)capacity);
        if (!next) { b->error = XUI_ERROR_OUT_OF_MEMORY; return; }
        b->data = next; b->capacity = capacity;
    }
    if (bytes) memcpy(b->data + b->size, text, (size_t)bytes);
    b->size += bytes; b->data[b->size] = 0;
}
/* Only writers with parser-confirmed source spans use this operation. The
 * normal serializer has no retained spans and keeps its explicit rewrite.
 * Ordered copies prove which source bytes must remain in the persistent
 * SourceStore; this is not a text diff and never infers syntax from spelling. */
static void doc_md_copy_source(doc_md_output* b, const char* text,
    uint64_t source, uint64_t bytes)
{
    uint64_t output = b->size; doc_md_source_plan* plan = b->plan;
    doc_md_write(b, text, bytes);
    if (b->error || !bytes || !plan) return;
    if (plan->count) {
        doc_md_source_copy* previous = &plan->copies[plan->count - 1];
        if (previous->source + previous->bytes == source &&
            previous->output + previous->bytes == output) { previous->bytes += bytes; return; }
    }
    if (plan->count == plan->capacity) {
        uint64_t capacity = plan->capacity ? plan->capacity * 2 : 16;
        doc_md_source_copy* next;
        if (capacity < plan->capacity || capacity > SIZE_MAX / sizeof(*next)) { b->error = XUI_DOC_ERROR_LIMIT; return; }
        next = doc_realloc(b->allocator, plan->copies, (size_t)capacity * sizeof(*next));
        if (!next) { b->error = XUI_ERROR_OUT_OF_MEMORY; return; }
        plan->copies = next; plan->capacity = capacity;
    }
    plan->copies[plan->count++] = (doc_md_source_copy){source, output, bytes};
}
static int doc_md_apply_source_plan(xui_document_transaction t,
    uint64_t start, uint64_t end, doc_md_output* output, int parse)
{
    doc_md_source_plan* plan = output->plan;
    uint64_t i, old_cursor = start, new_cursor = 0;
    int result;
    if (output->error) return output->error;
    if (!plan || !plan->count) return doc_txn_source_patch(t, start, end,
        output->data ? output->data : "", output->size, parse);
    /* Validate the complete proof before issuing any patches. Recheck copies
     * in bounded chunks so cancellation cannot wait on an arbitrarily long
     * preserved code line or hidden footnote. */
    for (i = 0; i < plan->count; i++) {
        const doc_md_source_copy* copy = &plan->copies[i]; uint64_t at;
        if (copy->source < old_cursor || copy->source > end || copy->bytes > end - copy->source ||
            copy->output < new_cursor || copy->output > output->size || copy->bytes > output->size - copy->output)
            return XUI_DOC_ERROR_SCHEMA;
        for (at = 0; at < copy->bytes; ) {
            uint64_t bytes = copy->bytes - at > 16384 ? 16384 : copy->bytes - at;
            result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
            if (!doc_seq_equal_bytes(t->draft->source, copy->source + at,
                output->data + copy->output + at, bytes)) return XUI_DOC_ERROR_SCHEMA;
            at += bytes;
        }
        old_cursor = copy->source + copy->bytes; new_cursor = copy->output + copy->bytes;
    }
    /* Descending offsets keep every edit in the original coordinate system.
     * Only gaps between retained copies allocate new source payloads. Parse
     * once, after the entire private candidate has been constructed. */
    old_cursor = end; new_cursor = output->size;
    for (i = plan->count; i; i--) {
        const doc_md_source_copy* copy = &plan->copies[i - 1];
        uint64_t old_end = copy->source + copy->bytes, new_end = copy->output + copy->bytes;
        result = doc_txn_source_patch(t, old_end, old_cursor,
            output->data + new_end, new_cursor - new_end, 0);
        if (result != XUI_OK) return result;
        old_cursor = copy->source; new_cursor = copy->output;
    }
    result = doc_txn_source_patch(t, start, old_cursor, output->data, new_cursor, 0);
    if (result == XUI_OK && parse) result = doc_markdown_parse(t);
    return result;
}
static void doc_md_escape(doc_md_output* b, const char* text, uint64_t bytes)
{
    uint64_t i;
    for (i = 0; i < bytes && !b->error; i++) {
        unsigned char c = (unsigned char)text[i];
        if ((c >= 33 && c <= 47) || (c >= 58 && c <= 64) || (c >= 91 && c <= 96) || (c >= 123 && c <= 126)) doc_md_write(b, "\\", 1);
        doc_md_write(b, text + i, 1);
    }
}
enum {
    DOC_MD_ENTITY_FIRST = 1, DOC_MD_ENTITY_LAST = 2,
    DOC_MD_ALT_STRONG = 4, DOC_MD_ALT_ITALIC = 8
};
/* Attribute line breaks use entities so writing a title cannot create a new
 * block, lose container prefixes, or normalize the requested CR/LF value. */
static void doc_md_title(doc_md_output* b, const char* title)
{
    uint64_t i, bytes = strlen(title);
    for (i = 0; i < bytes && !b->error; i++) {
        if (title[i] == '\n') doc_md_write(b, "&#10;", 5);
        else if (title[i] == '\r') doc_md_write(b, "&#13;", 5);
        else doc_md_escape(b, title + i, 1);
    }
}
static uint64_t doc_md_scalar_end(const char* text, uint64_t bytes)
{
    unsigned char first = (unsigned char)text[0];
    uint64_t length = first < 0x80 ? 1 : first < 0xe0 ? 2 : first < 0xf0 ? 3 : 4;
    return length <= bytes ? length : bytes;
}
static uint64_t doc_md_scalar_start(const char* text, uint64_t bytes)
{
    uint64_t at = bytes - 1;
    while (at && ((unsigned char)text[at] & 0xc0) == 0x80) at--;
    return at;
}
static uint32_t doc_md_scalar_value(const char* text, uint64_t bytes)
{
    uint32_t scalar = bytes == 1 ? (unsigned char)text[0] :
        (unsigned char)text[0] & (0x7fu >> bytes);
    uint64_t i;
    for (i = 1; i < bytes; i++)
        scalar = (scalar << 6) | ((unsigned char)text[i] & 0x3fu);
    return scalar;
}
static void doc_md_write_entity(doc_md_output* b, const char* text, uint64_t bytes)
{
    uint32_t scalar = doc_md_scalar_value(text, bytes);
    char entity[16]; int length;
    length = snprintf(entity, sizeof(entity), "&#%u;", scalar);
    if (length <= 0 || (size_t)length >= sizeof(entity)) { b->error = XUI_DOC_ERROR_LIMIT; return; }
    doc_md_write(b, entity, (uint64_t)length);
}

/* Fence language is an attribute, not code-body text. Escape characters that
 * the parser decodes, trims, treats as a new line, or consumes as fence bytes.
 * Numeric whitespace also keeps the complete language in MD4C's first word.
 * https://spec.commonmark.org/0.31.2/#entity-and-numeric-character-references */
static void doc_md_fence_info_char(doc_md_output* b, char c)
{
    if (c == '&') doc_md_write(b, "&amp;", 5);
    else if (c == '\\') doc_md_write(b, "&#92;", 5);
    else if ((unsigned char)c <= ' ' || c == '~' || c == '`')
        doc_md_write_entity(b, &c, 1);
    else doc_md_write(b, &c, 1);
}

static void doc_md_escape_boundary(doc_md_output* b, const char* text, uint64_t bytes, unsigned boundary)
{
    uint64_t first_end = 0, last_start = bytes;
    if (!bytes || !(boundary & (DOC_MD_ENTITY_FIRST | DOC_MD_ENTITY_LAST))) { doc_md_escape(b, text, bytes); return; }
    if (boundary & DOC_MD_ENTITY_FIRST) first_end = doc_md_scalar_end(text, bytes);
    if (boundary & DOC_MD_ENTITY_LAST) last_start = doc_md_scalar_start(text, bytes);
    if (first_end) doc_md_write_entity(b, text, first_end);
    if (boundary & DOC_MD_ENTITY_LAST) {
        if (last_start > first_end) doc_md_escape(b, text + first_end, last_start - first_end);
        if (last_start >= first_end) doc_md_write_entity(b, text + last_start, bytes - last_start);
    } else doc_md_escape(b, text + first_end, bytes - first_end);
}
static int doc_md_text_edge_scalar(doc_sequence* text, int last, unsigned* scalar)
{
    char bytes[4]; uint64_t length = doc_seq_size(text), count, start;
    if (!length) return 0;
    count = length < sizeof(bytes) ? length : sizeof(bytes);
    if (doc_seq_read(text, last ? length - count : 0, bytes, count) != XUI_OK) return 0;
    start = last ? doc_md_scalar_start(bytes, count) : 0;
    count = last ? count - start : doc_md_scalar_end(bytes, count);
    *scalar = doc_md_scalar_value(bytes + start, count);
    return 1;
}
static int doc_md_text_edge_space(doc_sequence* text, int last)
{
    unsigned scalar;
    return doc_md_text_edge_scalar(text, last, &scalar) &&
        doc_md4c_unicode_whitespace(scalar);
}
static int doc_md_text_edge_punct(doc_sequence* text, int last)
{
    unsigned scalar;
    return doc_md_text_edge_scalar(text, last, &scalar) &&
        doc_md4c_unicode_punct(scalar);
}
static void doc_md_destination(doc_md_output* b, const char* uri, const char* title)
{
    doc_md_write(b, "(<", 2); doc_md_escape(b, uri, strlen(uri));
    doc_md_write(b, ">", 1);
    if (*title) {
        doc_md_write(b, " \"", 2); doc_md_title(b, title);
        doc_md_write(b, "\"", 1);
    }
    doc_md_write(b, ")", 1);
}
static void doc_md_image_token(doc_md_output* b, doc_node* n,
    const char* text, uint64_t length, int include_link)
{
    const char *uri = doc_string(n->resource), *title = doc_string(n->title);
    unsigned alt_boundary = (doc_md_text_edge_space(n->text, 0) ? DOC_MD_ENTITY_FIRST : 0) |
        (doc_md_text_edge_space(n->text, 1) ? DOC_MD_ENTITY_LAST : 0);
    if (include_link && (n->attrs->iMarks & XUI_DOC_LINK)) doc_md_write(b, "[", 1);
    doc_md_write(b, "![", 2);
    doc_md_escape_boundary(b, text, length, alt_boundary);
    doc_md_write(b, "]", 1); doc_md_destination(b, uri, title);
    if (include_link && (n->attrs->iMarks & XUI_DOC_LINK)) {
        const char *target = doc_string(n->link_target), *link_title = doc_string(n->link_title);
        doc_md_write(b, "](<", 3); doc_md_escape(b, target, strlen(target));
        doc_md_write(b, ">", 1);
        if (*link_title) {
            doc_md_write(b, " \"", 2); doc_md_title(b, link_title);
            doc_md_write(b, "\"", 1);
        }
        doc_md_write(b, ")", 1);
    }
}
static void doc_md_inline_with_boundary(doc_state* s, doc_node* n, doc_md_output* b, unsigned boundary)
{
    char* text; uint64_t length = doc_seq_size(n->text), i, fence = 1, run = 0; uint32_t marks = n->attrs->iMarks;
    uint32_t supported = XUI_DOC_BOLD | XUI_DOC_ITALIC | XUI_DOC_CODE | XUI_DOC_LINK;
    const char* strong = boundary & DOC_MD_ALT_STRONG ? "__" : "**";
    const char* italic = boundary & DOC_MD_ALT_ITALIC ? "_" : "*";
    if (n->kind == XUI_DOC_SOFT_BREAK) { doc_md_write(b, "\n", 1); return; }
    if (n->kind == XUI_DOC_HARD_BREAK) { doc_md_write(b, "\\\n", 2); return; }
    if (n->kind == XUI_DOC_IMAGE || n->kind == XUI_DOC_MATH || n->kind == XUI_DOC_HTML || n->kind == XUI_DOC_FOOTNOTE_REF) {
        text = doc_seq_string(s->allocator, n->text); if (!text) { b->error = XUI_ERROR_OUT_OF_MEMORY; return; }
        if (n->kind == XUI_DOC_IMAGE) {
            doc_md_image_token(b, n, text, length, 1);
        } else if (n->kind == XUI_DOC_MATH) {
            unsigned delimiter = n->attrs->iFlags & XUI_DOC_BLOCK ? 2 : 1;
            doc_md_write(b, "$$", delimiter); doc_md_write(b, text, length); doc_md_write(b, "$$", delimiter);
        } else if (n->kind == XUI_DOC_HTML) doc_md_write(b, text, length);
        else { doc_md_write(b, "[^", 2); doc_md_escape(b, doc_string(n->info), strlen(doc_string(n->info))); doc_md_write(b, "]", 1); }
        doc_free(text); return;
    }
    if (s->dialect != XUI_MD_COMMONMARK) supported |= XUI_DOC_STRIKE;
    if (s->dialect == XUI_MD_EXTENDED) supported |= XUI_DOC_HIGHLIGHT | XUI_DOC_SUBSCRIPT | XUI_DOC_SUPERSCRIPT;
    if (n->kind != XUI_DOC_TEXT || marks & ~supported) { b->error = XUI_DOC_ERROR_UNREPRESENTABLE; return; }
    text = doc_seq_string(s->allocator, n->text); if (!text) { b->error = XUI_ERROR_OUT_OF_MEMORY; return; }
    if (marks & XUI_DOC_LINK) doc_md_write(b, "[", 1);
    if (marks & XUI_DOC_BOLD) doc_md_write(b, strong, 2);
    if (marks & XUI_DOC_ITALIC) doc_md_write(b, italic, 1);
    if (marks & XUI_DOC_STRIKE) doc_md_write(b, "~~", 2);
    if (marks & XUI_DOC_HIGHLIGHT) doc_md_write(b, "==", 2);
    if (marks & XUI_DOC_SUBSCRIPT) doc_md_write(b, "~", 1);
    if (marks & XUI_DOC_SUPERSCRIPT) doc_md_write(b, "^", 1);
    if (marks & XUI_DOC_CODE) {
        int pad;
        for (i = 0; i < length; i++) { if (text[i] == '`') { if (++run >= fence) fence = run + 1; } else run = 0; }
        for (i = 0; i < fence; i++) doc_md_write(b, "`", 1);
        pad = length && (text[0] == '`' || text[length - 1] == '`' || (text[0] == ' ' && text[length - 1] == ' '));
        if (pad) doc_md_write(b, " ", 1);
        doc_md_write(b, text, length);
        if (pad) doc_md_write(b, " ", 1);
        for (i = 0; i < fence; i++) doc_md_write(b, "`", 1);
    } else doc_md_escape_boundary(b, text, length, boundary);
    if (marks & XUI_DOC_SUPERSCRIPT) doc_md_write(b, "^", 1);
    if (marks & XUI_DOC_SUBSCRIPT) doc_md_write(b, "~", 1);
    if (marks & XUI_DOC_HIGHLIGHT) doc_md_write(b, "==", 2);
    if (marks & XUI_DOC_STRIKE) doc_md_write(b, "~~", 2);
    if (marks & XUI_DOC_ITALIC) doc_md_write(b, italic, 1);
    if (marks & XUI_DOC_BOLD) doc_md_write(b, strong, 2);
    if (marks & XUI_DOC_LINK) {
        const char* uri = doc_string(n->resource); const char* title = doc_string(n->title);
        doc_md_write(b, "](<", 3); doc_md_escape(b, uri, strlen(uri)); doc_md_write(b, ">", 1);
        if (*title) { doc_md_write(b, " \"", 2); doc_md_title(b, title); doc_md_write(b, "\"", 1); }
        doc_md_write(b, ")", 1);
    }
    doc_free(text);
}
static void doc_md_inline(doc_state* s, doc_node* n, doc_md_output* b)
{
    doc_md_inline_with_boundary(s, n, b, 0);
}
/* Factor a mark shared by adjacent text runs before spelling their changing
 * inner mark. Reopening both marks at every run would concatenate ambiguous
 * star delimiters (for example bold+italic / bold / bold+italic). */
static uint64_t doc_md_pair_end(doc_state* s, doc_node* parent, uint64_t start, uint32_t outer)
{
    uint64_t i, count = doc_seq_size(parent->children);
    uint32_t inner = outer == XUI_DOC_BOLD ? XUI_DOC_ITALIC : XUI_DOC_BOLD;
    doc_node* first_node = doc_index_get(s->index, doc_seq_get_id(parent->children, start));
    xui_doc_attributes_t base = *first_node->attrs;
    int first = -1, changed = 0;
    base.iMarks = 0;
    for (i = start; i < count; i++) {
        doc_node* child = doc_index_get(s->index, doc_seq_get_id(parent->children, i));
        xui_doc_attributes_t attrs;
        int active;
        if (!child || child->kind != XUI_DOC_TEXT || !doc_seq_size(child->text) ||
            !(child->attrs->iMarks & outer) || (child->attrs->iMarks & ~(outer | inner)) ||
            *doc_string(child->resource) || *doc_string(child->info) || *doc_string(child->title)) break;
        attrs = *child->attrs; attrs.iMarks = 0;
        if (!doc_attributes_equal(&base, &attrs)) break;
        active = !!(child->attrs->iMarks & inner);
        if (first < 0) first = active;
        else if (active != first) changed = 1;
    }
    return changed ? i : start;
}
static void doc_md_pair_write(doc_state* s, doc_node* parent, doc_md_output* b,
    uint64_t start, uint64_t end, uint32_t outer, int alternate_outer)
{
    uint64_t i; int active = 0;
    uint32_t inner = outer == XUI_DOC_BOLD ? XUI_DOC_ITALIC : XUI_DOC_BOLD;
    const char* outer_delimiter = outer == XUI_DOC_BOLD ?
        (alternate_outer ? "__" : "**") : (alternate_outer ? "_" : "*");
    const char* inner_delimiter = inner == XUI_DOC_ITALIC ? "_" : "__";
    uint64_t outer_bytes = outer == XUI_DOC_BOLD ? 2 : 1;
    uint64_t inner_bytes = inner == XUI_DOC_BOLD ? 2 : 1;
    doc_md_write(b, outer_delimiter, outer_bytes);
    for (i = start; i < end && !b->error; i++) {
        doc_node* child = doc_index_get(s->index, doc_seq_get_id(parent->children, i));
        doc_node* previous = i > start ? doc_index_get(s->index, doc_seq_get_id(parent->children, i - 1)) : NULL;
        doc_node* next = i + 1 < end ? doc_index_get(s->index, doc_seq_get_id(parent->children, i + 1)) : NULL;
        uint64_t bytes = doc_seq_size(child->text); unsigned boundary = 0;
        int marked = !!(child->attrs->iMarks & inner);
        char* text;
        if (marked != active) { doc_md_write(b, inner_delimiter, inner_bytes); active = marked; }
        if ((i == start || marked) && doc_md_text_edge_space(child->text, 0)) boundary |= DOC_MD_ENTITY_FIRST;
        if ((i + 1 == end || marked) && doc_md_text_edge_space(child->text, 1)) boundary |= DOC_MD_ENTITY_LAST;
        if (previous && !!(previous->attrs->iMarks & inner) != marked) boundary |= DOC_MD_ENTITY_FIRST;
        if (next && !!(next->attrs->iMarks & inner) != marked) boundary |= DOC_MD_ENTITY_LAST;
        text = doc_seq_string(s->allocator, child->text);
        if (!text) { b->error = XUI_ERROR_OUT_OF_MEMORY; break; }
        doc_md_escape_boundary(b, text, bytes, boundary); doc_free(text);
    }
    if (active) doc_md_write(b, inner_delimiter, inner_bytes);
    doc_md_write(b, outer_delimiter, outer_bytes);
}
static int doc_md_visible_inlines(doc_state* s, doc_node* n, doc_md_output* b)
{
    uint64_t i; unsigned last_strong_alt = 0, last_pair_outer = 0,
        last_pair_outer_alt = 0, last_italic_alt = 0;
    const uint32_t boundary_marks = XUI_DOC_BOLD | XUI_DOC_ITALIC | XUI_DOC_STRIKE |
        XUI_DOC_HIGHLIGHT | XUI_DOC_SUBSCRIPT | XUI_DOC_SUPERSCRIPT;
    for (i = 0; i < doc_seq_size(n->children) && !b->error; i++) {
        doc_node* child = doc_index_get(s->index, doc_seq_get_id(n->children, i));
        doc_node run = {0}; uint64_t run_start = i; unsigned boundary = 0;
        if (child && child->kind == XUI_DOC_TEXT && (child->attrs->iMarks & (XUI_DOC_BOLD | XUI_DOC_ITALIC))) {
            uint64_t bold_end = doc_md_pair_end(s, n, i, XUI_DOC_BOLD);
            uint64_t italic_end = doc_md_pair_end(s, n, i, XUI_DOC_ITALIC);
            uint64_t end = bold_end > italic_end ? bold_end : italic_end;
            if (end > i) {
                uint32_t outer = bold_end >= italic_end ? XUI_DOC_BOLD : XUI_DOC_ITALIC;
                doc_node* previous = i ? doc_index_get(s->index, doc_seq_get_id(n->children, i - 1)) : NULL;
                /* Adjacent outer delimiters must use different characters;
                 * *** and ___ may otherwise be parsed as a new mark run. */
                int alternate_outer = outer == XUI_DOC_BOLD ?
                    ((last_pair_outer == XUI_DOC_ITALIC && !last_pair_outer_alt) ||
                     (!last_pair_outer && previous && previous->kind == XUI_DOC_TEXT &&
                      previous->attrs->iMarks == XUI_DOC_ITALIC && !last_italic_alt)) :
                    ((last_pair_outer == XUI_DOC_BOLD && !last_pair_outer_alt) ||
                     (!last_pair_outer && previous && previous->kind == XUI_DOC_TEXT &&
                      previous->attrs->iMarks == XUI_DOC_BOLD && !last_strong_alt));
                doc_md_pair_write(s, n, b, i, end, outer, alternate_outer);
                last_pair_outer = outer; last_pair_outer_alt = alternate_outer;
                last_italic_alt = 0;
                i = end - 1; continue;
            }
        }
        if (child->kind != XUI_DOC_TEXT) {
            doc_md_inline(s, child, b);
            last_pair_outer = last_pair_outer_alt = last_italic_alt = 0; continue;
        }
        run.kind = child->kind; run.attrs = child->attrs; run.text = child->text;
        run.resource = child->resource; run.info = child->info; run.title = child->title;
        doc_seq_retain(run.text);
        while (i + 1 < doc_seq_size(n->children)) {
            doc_node* next = doc_index_get(s->index, doc_seq_get_id(n->children, i + 1)); doc_sequence* merged;
            uint64_t size = doc_seq_size(run.text);
            if (next->kind != XUI_DOC_TEXT || !doc_attributes_equal(run.attrs, next->attrs) ||
                strcmp(doc_string(run.resource), doc_string(next->resource)) ||
                strcmp(doc_string(run.info), doc_string(next->info)) || strcmp(doc_string(run.title), doc_string(next->title))) break;
            b->error = doc_seq_replace(s->allocator, run.text, size, size, next->text, &merged);
            if (b->error) break;
            doc_seq_release(run.text); run.text = merged; i++;
        }
        if (!b->error && doc_seq_size(run.text)) {
            doc_node* previous = run_start ? doc_index_get(s->index, doc_seq_get_id(n->children, run_start - 1)) : NULL;
            doc_node* next = i + 1 < doc_seq_size(n->children) ?
                doc_index_get(s->index, doc_seq_get_id(n->children, i + 1)) : NULL;
            int preceding_underscore_strong =
                (last_pair_outer == XUI_DOC_BOLD && last_pair_outer_alt) ||
                (!last_pair_outer && previous && previous->kind == XUI_DOC_TEXT &&
                 previous->attrs->iMarks == XUI_DOC_BOLD && last_strong_alt);
            if ((run.attrs->iMarks & boundary_marks) && !(run.attrs->iMarks & XUI_DOC_CODE)) {
                if (doc_md_text_edge_space(run.text, 0)) boundary |= DOC_MD_ENTITY_FIRST;
                if (doc_md_text_edge_space(run.text, 1)) boundary |= DOC_MD_ENTITY_LAST;
                if ((run.attrs->iMarks & XUI_DOC_BOLD) && run_start >= 2 && previous && previous->kind == XUI_DOC_TEXT &&
                    !(previous->attrs->iMarks & XUI_DOC_BOLD) && doc_md_text_edge_space(run.text, 0)) {
                    uint64_t before_index = run_start; doc_node* before = NULL;
                    while (before_index) {
                        before = doc_index_get(s->index, doc_seq_get_id(n->children, before_index - 1));
                        if (!before || before->kind != XUI_DOC_TEXT || (before->attrs->iMarks & XUI_DOC_BOLD)) break;
                        before_index--;
                    }
                    if (before && before->kind == XUI_DOC_TEXT && before->attrs->iMarks == XUI_DOC_BOLD &&
                        previous->attrs->iMarks != XUI_DOC_ITALIC &&
                        doc_md_text_edge_space(before->text, 1) && !last_strong_alt) boundary |= DOC_MD_ALT_STRONG;
                }
            } else if (!(run.attrs->iMarks & XUI_DOC_CODE)) {
                if (!run_start && doc_md_text_edge_space(run.text, 0)) boundary |= DOC_MD_ENTITY_FIRST;
                if (i + 1 == doc_seq_size(n->children) && doc_md_text_edge_space(run.text, 1))
                    boundary |= DOC_MD_ENTITY_LAST;
                if (previous && previous->kind == XUI_DOC_TEXT && (previous->attrs->iMarks & boundary_marks) &&
                    !(previous->attrs->iMarks & XUI_DOC_CODE) &&
                    doc_md_text_edge_space(previous->text, 1)) boundary |= DOC_MD_ENTITY_FIRST;
                if (previous && previous->kind == XUI_DOC_TEXT &&
                    (previous->attrs->iMarks & boundary_marks) &&
                    !(previous->attrs->iMarks & XUI_DOC_CODE) &&
                    doc_md_text_edge_punct(previous->text, 1))
                    boundary |= DOC_MD_ENTITY_FIRST;
                if (next && next->kind == XUI_DOC_TEXT && (next->attrs->iMarks & boundary_marks) &&
                    !(next->attrs->iMarks & XUI_DOC_CODE) &&
                    doc_md_text_edge_space(next->text, 0)) boundary |= DOC_MD_ENTITY_LAST;
                if (next && next->kind == XUI_DOC_TEXT &&
                    (next->attrs->iMarks & boundary_marks) &&
                    !(next->attrs->iMarks & XUI_DOC_CODE) &&
                    doc_md_text_edge_punct(next->text, 0))
                    boundary |= DOC_MD_ENTITY_LAST;
                /* An entity changes the raw flanking character to ';' while
                 * preserving the decoded text. This permits an underscore
                 * delimiter beside an ordinary word character. */
                if (last_pair_outer || last_italic_alt ||
                    (last_strong_alt && previous && previous->kind == XUI_DOC_TEXT &&
                     previous->attrs->iMarks == XUI_DOC_BOLD)) boundary |= DOC_MD_ENTITY_FIRST;
                if (next && next->kind == XUI_DOC_TEXT) {
                    uint64_t next_index = i + 1;
                    if (doc_md_pair_end(s, n, next_index, XUI_DOC_BOLD) > next_index ||
                        doc_md_pair_end(s, n, next_index, XUI_DOC_ITALIC) > next_index)
                        boundary |= DOC_MD_ENTITY_LAST;
                    if (next->attrs->iMarks == XUI_DOC_ITALIC) {
                        uint64_t at = next_index + 1;
                        while (at < doc_seq_size(n->children)) {
                            doc_node* following = doc_index_get(s->index,
                                doc_seq_get_id(n->children, at));
                            if (!following || following->kind != XUI_DOC_TEXT ||
                                !doc_attributes_equal(next->attrs, following->attrs) ||
                                strcmp(doc_string(next->resource), doc_string(following->resource)) ||
                                strcmp(doc_string(next->info), doc_string(following->info)) ||
                                strcmp(doc_string(next->title), doc_string(following->title))) break;
                            at++;
                        }
                        if (at < doc_seq_size(n->children)) {
                            doc_node* after_run = doc_index_get(s->index,
                                doc_seq_get_id(n->children, at));
                            if (after_run && after_run->kind == XUI_DOC_TEXT &&
                                after_run->attrs->iMarks == XUI_DOC_BOLD)
                                boundary |= DOC_MD_ENTITY_LAST;
                        }
                    }
                }
            }
            /* Keep the delimiter character distinct from the preceding
             * strong mark and the following standalone strong mark. */
            if (run.attrs->iMarks == XUI_DOC_ITALIC &&
                ((last_pair_outer == XUI_DOC_BOLD && !last_pair_outer_alt) ||
                 (!last_pair_outer && previous && previous->kind == XUI_DOC_TEXT &&
                    previous->attrs->iMarks == XUI_DOC_BOLD && !last_strong_alt) ||
                 (next && next->kind == XUI_DOC_TEXT &&
                    next->attrs->iMarks == XUI_DOC_BOLD && !preceding_underscore_strong)))
                boundary |= DOC_MD_ALT_ITALIC;
            if (run.attrs->iMarks == XUI_DOC_BOLD &&
                ((last_pair_outer == XUI_DOC_ITALIC && !last_pair_outer_alt) ||
                 (!last_pair_outer && previous && previous->kind == XUI_DOC_TEXT &&
                  previous->attrs->iMarks == XUI_DOC_ITALIC && !last_italic_alt)))
                boundary |= DOC_MD_ALT_STRONG;
            doc_md_inline_with_boundary(s, &run, b, boundary);
            if (run.attrs->iMarks & XUI_DOC_BOLD) last_strong_alt = !!(boundary & DOC_MD_ALT_STRONG);
            last_italic_alt = run.attrs->iMarks == XUI_DOC_ITALIC && !!(boundary & DOC_MD_ALT_ITALIC);
            last_pair_outer = last_pair_outer_alt = 0;
        }
        doc_seq_release(run.text);
    }
    return b->error;
}
/* Empty Text has no Markdown token. Use a private child sequence without
 * those runs for every delimiter-neighbor, coalescing and shared-mark rule;
 * skipping only their output still makes unrelated delimiters collide.
 * Never alter the desired semantic tree or any published snapshot. */
static int doc_md_inlines(doc_state* s, doc_node* n, doc_md_output* b)
{
    uint64_t i, count = doc_seq_size(n->children);
    doc_node visible = {0};
    for (i = 0; i < count; i++) {
        doc_node* child = doc_index_get(s->index, doc_seq_get_id(n->children, i));
        if (child->kind == XUI_DOC_TEXT && !doc_seq_size(child->text)) break;
    }
    if (i == count) return doc_md_visible_inlines(s, n, b);
    visible.children = n->children; doc_seq_retain(visible.children);
    for (i = count; i > 0 && !b->error; i--) {
        doc_node* child = doc_index_get(s->index, doc_seq_get_id(n->children, i - 1));
        doc_sequence* children = NULL;
        if (child->kind != XUI_DOC_TEXT || doc_seq_size(child->text)) continue;
        b->error = doc_seq_replace(s->allocator, visible.children, i - 1, i, NULL, &children);
        if (!b->error) { doc_seq_release(visible.children); visible.children = children; }
    }
    if (!b->error) doc_md_visible_inlines(s, &visible, b);
    doc_seq_release(visible.children);
    return b->error;
}
static int doc_md_block_write(doc_state*, doc_node*, doc_md_output*);
static int doc_md_children(doc_state* s, doc_node* n, doc_md_output* b, int compact)
{
    uint64_t i;
    for (i = 0; i < doc_seq_size(n->children) && !b->error; i++) {
        doc_node* child = doc_index_get(s->index, doc_seq_get_id(n->children, i));
        if (doc_semantic_empty_paragraph(s, child)) continue;
        b->error = doc_md_block_write(s, child, b);
        if (compact) while (b->size > 1 && b->data[b->size - 1] == '\n' && b->data[b->size - 2] == '\n') b->data[--b->size] = 0;
    }
    return b->error;
}
static void doc_md_prefixed(doc_md_output* out, doc_md_output* body, const char* first, const char* rest)
{
    uint64_t start = 0, end;
    if (body->error) { out->error = body->error; return; }
    if (!body->size) { doc_md_write(out, first, strlen(first)); doc_md_write(out, "\n", 1); return; }
    while (start < body->size && !out->error) {
        const char* prefix = start ? rest : first;
        end = start; while (end < body->size && body->data[end++] != '\n') { }
        doc_md_write(out, prefix, strlen(prefix)); doc_md_write(out, body->data + start, end - start); start = end;
    }
}
static int doc_md_table(doc_state* s, doc_node* table, doc_md_output* out)
{
    uint64_t r, c, columns = 0; doc_node* header = NULL;
    if (s->dialect == XUI_MD_COMMONMARK || !doc_seq_size(table->children)) return XUI_DOC_ERROR_UNREPRESENTABLE;
    for (r = 0; r < doc_seq_size(table->children); r++) {
        doc_node* row = doc_index_get(s->index, doc_seq_get_id(table->children, r));
        if (!r) { columns = doc_seq_size(row->children); header = row; }
        if (!columns || columns != doc_seq_size(row->children)) return XUI_DOC_ERROR_UNREPRESENTABLE;
        doc_md_write(out, "|", 1);
        for (c = 0; c < columns; c++) {
            doc_node* cell = doc_index_get(s->index, doc_seq_get_id(row->children, c));
            doc_node* head = doc_index_get(s->index, doc_seq_get_id(header->children, c));
            doc_md_output body = {0}; body.allocator = out->allocator;
            if (cell->attrs->iRowSpan != 1 || cell->attrs->iColumnSpan != 1 ||
                !!(cell->attrs->iFlags & XUI_DOC_HEADER) != !r || cell->attrs->iAlignment != head->attrs->iAlignment ||
                doc_seq_size(cell->children) > 1) return XUI_DOC_ERROR_UNREPRESENTABLE;
            if (doc_seq_size(cell->children)) {
                doc_node* p = doc_index_get(s->index, doc_seq_get_id(cell->children, 0));
                if (p->kind != XUI_DOC_PARAGRAPH) return XUI_DOC_ERROR_UNREPRESENTABLE;
                doc_md_inlines(s, p, &body);
            }
            if (!body.error && body.size && (memchr(body.data, '\n', (size_t)body.size) || memchr(body.data, '\r', (size_t)body.size))) body.error = XUI_DOC_ERROR_UNREPRESENTABLE;
            if (body.error) { int error = body.error; doc_free(body.data); return error; }
            doc_md_write(out, " ", 1); doc_md_write(out, body.data, body.size); doc_md_write(out, " |", 2); doc_free(body.data);
        }
        doc_md_write(out, "\n", 1);
        if (!r) {
            doc_md_write(out, "|", 1);
            for (c = 0; c < columns; c++) {
                doc_node* cell = doc_index_get(s->index, doc_seq_get_id(header->children, c));
                const char* separator = cell->attrs->iAlignment == 1 ? " :---: |" : cell->attrs->iAlignment == 2 ? " ---: |" : " --- |";
                doc_md_write(out, separator, strlen(separator));
            }
            doc_md_write(out, "\n", 1);
        }
    }
    doc_md_write(out, "\n", 1); return out->error;
}
static int doc_md_block_write(doc_state* s, doc_node* n, doc_md_output* b)
{
    uint64_t i; doc_md_output body = {0}; int result = XUI_OK;
    body.allocator = b->allocator;
    switch (n->kind) {
    case XUI_DOC_PARAGRAPH: case XUI_DOC_HEADING:
        if (n->kind == XUI_DOC_HEADING) { for (i = 0; i < n->attrs->iHeadingLevel; i++) doc_md_write(b, "#", 1); doc_md_write(b, " ", 1); }
        doc_md_inlines(s, n, b); doc_md_write(b, "\n\n", 2); break;
    case XUI_DOC_QUOTE:
        if (n->info && n->info->size) {
            doc_md_write(&body, "[!", 2);
            for (i = 0; i < n->info->size; i++) { char ch = (char)toupper((unsigned char)n->info->data[i]); doc_md_write(&body, &ch, 1); }
            doc_md_write(&body, "]\n", 2);
        }
        doc_md_children(s, n, &body, 0); doc_md_prefixed(b, &body, "> ", "> "); doc_md_write(b, "\n", 1); break;
    case XUI_DOC_LIST:
        for (i = 0; i < doc_seq_size(n->children) && !b->error; i++) {
            doc_node* item = doc_index_get(s->index, doc_seq_get_id(n->children, i)); char first[64], rest[64]; size_t width;
            body.size = 0;
            if (n->attrs->iFlags & XUI_DOC_ORDERED)
                snprintf(first, sizeof(first), "%llu%c ", (unsigned long long)(n->attrs->iListStart + i), b->list_marker ? b->list_marker : '.');
            else { strcpy(first, "- "); if (b->list_marker) first[0] = b->list_marker; }
            width = strlen(first); memset(rest, ' ', width); rest[width] = 0;
            if (item->attrs->iFlags & XUI_DOC_TASK) strcat(first, item->attrs->iFlags & XUI_DOC_CHECKED ? "[x] " : "[ ] ");
            doc_md_children(s, item, &body, !!(n->attrs->iFlags & XUI_DOC_TIGHT));
            doc_md_prefixed(b, &body, first, rest);
        }
        doc_md_write(b, "\n", 1); break;
    case XUI_DOC_CODE_BLOCK: case XUI_DOC_DIAGRAM: case XUI_DOC_FRONT_MATTER: case XUI_DOC_HTML: {
        char* text = doc_seq_string(s->allocator, n->text); uint64_t size = doc_seq_size(n->text), fence = 3, run = 0;
        char marker = n->kind == XUI_DOC_FRONT_MATTER ? '-' : '~';
        if (!text) { result = XUI_ERROR_OUT_OF_MEMORY; break; }
        if (n->kind == XUI_DOC_HTML) doc_md_write(b, text, size);
        else {
            if (n->kind != XUI_DOC_FRONT_MATTER) for (i = 0; i < size; i++) { if (text[i] == marker) { if (++run >= fence) fence = run + 1; } else run = 0; }
            for (i = 0; i < fence; i++) doc_md_write(b, &marker, 1);
            if (n->kind == XUI_DOC_DIAGRAM) doc_md_write(b, "mermaid", 7);
            else if (n->kind == XUI_DOC_CODE_BLOCK) {
                const char* info = doc_string(n->info);
                for (i = 0; info[i] && !b->error; i++) doc_md_fence_info_char(b, info[i]);
            }
            doc_md_write(b, "\n", 1); doc_md_write(b, text, size);
            if (size && text[size - 1] != '\n') doc_md_write(b, "\n", 1);
            for (i = 0; i < fence; i++) doc_md_write(b, &marker, 1);
        }
        doc_md_write(b, "\n\n", 2); doc_free(text); break;
    }
    case XUI_DOC_RULE: doc_md_write(b, "***\n\n", 5); break;
    case XUI_DOC_TABLE: result = doc_md_table(s, n, b); break;
    case XUI_DOC_FOOTNOTE: {
        doc_md_output prefix = {0}; prefix.allocator = b->allocator;
        doc_md_write(&prefix, "[^", 2); doc_md_escape(&prefix, doc_string(n->info), strlen(doc_string(n->info))); doc_md_write(&prefix, "]: ", 3);
        doc_md_children(s, n, &body, 0);
        if (prefix.error) result = prefix.error; else doc_md_prefixed(b, &body, prefix.data, "    ");
        doc_free(prefix.data); break;
    }
    default: result = XUI_DOC_ERROR_UNREPRESENTABLE; break;
    }
    doc_free(body.data); return result != XUI_OK ? result : b->error;
}
/* An empty paragraph between two lists has no Markdown semantic node. Use a
 * different marker for the next list so a full parse does not join them again. */
static char doc_md_list_marker(doc_state* s, doc_node* root, uint64_t index)
{
    doc_node* current = doc_index_get(s->index, doc_seq_get_id(root->children, index));
    uint64_t adjacent = 0;
    if (!current || current->kind != XUI_DOC_LIST) return 0;
    while (index) {
        doc_node* previous = doc_index_get(s->index, doc_seq_get_id(root->children, --index));
        if (doc_semantic_empty_paragraph(s, previous)) continue;
        if (!previous || previous->kind != XUI_DOC_LIST ||
            !!(previous->attrs->iFlags & XUI_DOC_ORDERED) != !!(current->attrs->iFlags & XUI_DOC_ORDERED)) break;
        adjacent++;
    }
    return adjacent & 1 ? (current->attrs->iFlags & XUI_DOC_ORDERED ? ')' : '+') : 0;
}
static int doc_md_bounds(doc_state* s, uint64_t id, uint64_t* start, uint64_t* end)
{
    doc_node* n = doc_index_get(s->index, id);
    doc_node_source_range range; doc_node_source_range_get(s, n, &range);
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE || range.syntax_start > range.syntax_end || range.syntax_end > doc_seq_size(s->source)) return XUI_ERROR_UNSUPPORTED;
    *start = range.syntax_start; *end = range.syntax_end; return XUI_OK;
}
typedef struct doc_md_source_gap { uint64_t start, end; int keep; } doc_md_source_gap;
/* A container can contain source-only link definitions between its semantic
 * children. Keep each complete trivia gap containing a definition before the
 * regenerated body; a full parse decides whether this remains equivalent. */
static int doc_md_container_reference_prelude(doc_state* s, doc_node* container,
    uint64_t start, uint64_t end, const char* source, doc_md_output* out,
    int* first_gap_retained)
{
    doc_md_source_gap* gaps; uint64_t count = doc_seq_size(container->children), i, at = start;
    int result = XUI_OK;
    if (first_gap_retained) *first_gap_retained = 0;
    if (count >= SIZE_MAX / sizeof(*gaps)) return XUI_DOC_ERROR_LIMIT;
    gaps = doc_alloc(s->allocator, (size_t)(count + 1) * sizeof(*gaps));
    if (!gaps) return XUI_ERROR_OUT_OF_MEMORY;
    for (i = 0; i <= count; i++) {
        uint64_t next = end, child_end = end;
        if (i < count && doc_md_bounds(s, doc_seq_get_id(container->children, i), &next, &child_end) != XUI_OK) {
            result = XUI_DOC_ERROR_UNREPRESENTABLE; goto done;
        }
        if (next < at || child_end < next || child_end > end) {
            result = XUI_DOC_ERROR_UNREPRESENTABLE; goto done;
        }
        gaps[i] = (doc_md_source_gap){at, next, 0}; at = child_end;
    }
    for (i = 0; i < doc_seq_size(s->references); i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref; uint64_t lo = 0, hi = count + 1;
        doc_seq_read(s->references, i, &ref, sizeof(ref));
        if (ref.iSourceStart >= end || ref.iSourceEnd <= start) continue;
        if (ref.iKind != XUI_DOC_REFERENCE_LINK || ref.iSourceStart < start || ref.iSourceEnd > end) {
            result = XUI_DOC_ERROR_UNREPRESENTABLE; goto done;
        }
        while (lo < hi) {
            uint64_t mid = lo + (hi - lo) / 2;
            if (ref.iSourceStart < gaps[mid].start) hi = mid;
            else if (ref.iSourceStart >= gaps[mid].end) lo = mid + 1;
            else {
                if (ref.iSourceEnd > gaps[mid].end) { result = XUI_DOC_ERROR_UNREPRESENTABLE; goto done; }
                gaps[mid].keep = 1; break;
            }
        }
        if (lo == hi) { result = XUI_DOC_ERROR_UNREPRESENTABLE; goto done; }
    }
    for (i = 0; i <= count; i++) if (gaps[i].keep) {
        if (!gaps[i].end || (source[gaps[i].end - 1] != '\n' && source[gaps[i].end - 1] != '\r')) {
            result = XUI_DOC_ERROR_UNREPRESENTABLE; goto done;
        }
        doc_md_write(out, source + gaps[i].start, gaps[i].end - gaps[i].start);
        if (out->error) { result = out->error; goto done; }
    }
    if (first_gap_retained) *first_gap_retained = gaps[0].keep;
done:
    doc_free(gaps); return result;
}
static int doc_md_reference_list_item(doc_state* s, doc_node* list,
    const xui_doc_reference_definition_t* ref, uint64_t* index)
{
    uint64_t lo = 0, hi = doc_seq_size(list->children);
    while (lo < hi) {
        uint64_t mid = lo + (hi - lo) / 2, start, end;
        if (doc_md_bounds(s, doc_seq_get_id(list->children, mid), &start, &end) != XUI_OK)
            return XUI_DOC_ERROR_UNREPRESENTABLE;
        if (ref->iSourceStart < start) hi = mid;
        else if (ref->iSourceStart >= end) lo = mid + 1;
        else {
            if (ref->iSourceEnd > end) return XUI_DOC_ERROR_UNREPRESENTABLE;
            *index = mid; return XUI_OK;
        }
    }
    return XUI_DOC_ERROR_UNREPRESENTABLE;
}
static int doc_md_list_with_reference(doc_state* old, doc_state* desired,
    doc_node* old_list, doc_node* new_list, const unsigned char* referenced_items,
    const char* source, doc_md_output* out, int* tail_raw_retained)
{
    uint64_t i, count = doc_seq_size(new_list->children);
    int tail_has_raw_reference = 0;
    if (tail_raw_retained) *tail_raw_retained = 0;
    if (doc_seq_size(old_list->children) != count || !referenced_items)
        return XUI_DOC_ERROR_UNREPRESENTABLE;
    for (i = 0; i < count && !out->error; i++) {
        doc_node* item = doc_index_get(desired->index, doc_seq_get_id(new_list->children, i));
        doc_md_output body = {0}, refs = {0}; char first[64], rest[64]; size_t width;
        uint64_t generated_start; int first_gap = 0, result = XUI_OK;
        body.allocator = refs.allocator = out->allocator;
        if (referenced_items[i]) {
            doc_node* old_item = doc_index_get(old->index, doc_seq_get_id(old_list->children, i));
            uint64_t start, end;
            if (old_item->kind != XUI_DOC_LIST_ITEM || item->kind != XUI_DOC_LIST_ITEM ||
                doc_md_bounds(old, old_item->id, &start, &end) != XUI_OK) {
                result = XUI_DOC_ERROR_UNREPRESENTABLE; goto item_done;
            }
            result = doc_md_container_reference_prelude(old, old_item, start, end,
                source, &refs, &first_gap);
            if (result != XUI_OK) goto item_done;
            if (first_gap && ((old_item->attrs->iFlags | item->attrs->iFlags) & XUI_DOC_TASK)) {
                result = XUI_DOC_ERROR_UNREPRESENTABLE; goto item_done;
            }
            if (first_gap) doc_md_write(out, refs.data, refs.size);
            if (out->error) { result = out->error; goto item_done; }
        }
        generated_start = out->size;
        if (new_list->attrs->iFlags & XUI_DOC_ORDERED)
            snprintf(first, sizeof(first), "%llu%c ",
                (unsigned long long)(new_list->attrs->iListStart + i),
                out->list_marker ? out->list_marker : '.');
        else { strcpy(first, "- "); if (out->list_marker) first[0] = out->list_marker; }
        width = strlen(first); memset(rest, ' ', width); rest[width] = 0;
        if (item->attrs->iFlags & XUI_DOC_TASK)
            strcat(first, item->attrs->iFlags & XUI_DOC_CHECKED ? "[x] " : "[ ] ");
        result = doc_md_children(desired, item, &body,
            !!(new_list->attrs->iFlags & XUI_DOC_TIGHT));
        if (result == XUI_OK) doc_md_prefixed(out, &body, first, rest);
        if (result != XUI_OK) goto item_done;
        if (out->error) { result = out->error; goto item_done; }
        if (first_gap) {
            uint64_t marker_end = generated_start;
            while (marker_end < out->size && out->data[marker_end] != ' ' &&
                out->data[marker_end] != '\n' && marker_end - generated_start < 64)
                marker_end++;
            if (marker_end == generated_start || marker_end >= out->size ||
                out->data[marker_end] != ' ') {
                result = XUI_DOC_ERROR_UNREPRESENTABLE; goto item_done;
            }
            memset(out->data + generated_start, ' ',
                (size_t)(marker_end - generated_start + 1));
        } else if (referenced_items[i]) {
            /* Keep an existing marker (including a task checkbox) and append
             * source-only definitions before the next item. The full parse
             * rejects cases where this changes item scope or semantics. */
            doc_md_write(out, refs.data, refs.size);
            if (out->error) result = out->error;
            if (i + 1 == count) tail_has_raw_reference = 1;
        }
item_done:
        doc_free(body.data); doc_free(refs.data);
        if (result != XUI_OK) return result;
    }
    if (!tail_has_raw_reference) doc_md_write(out, "\n", 1);
    else if (tail_raw_retained) *tail_raw_retained = 1;
    return out->error;
}
static int doc_md_source_spelling_equal(doc_sequence* old_source,
    uint64_t old_start, uint64_t old_end, doc_sequence* new_source,
    uint64_t new_start, uint64_t new_end)
{
    uint64_t offset, length;
    if (old_end < old_start || new_end < new_start ||
        old_end > doc_seq_size(old_source) || new_end > doc_seq_size(new_source) ||
        old_end - old_start != new_end - new_start) return 0;
    length = old_end - old_start;
    for (offset = 0; offset < length; ) {
        char before[256], after[256]; uint64_t bytes = length - offset;
        if (bytes > sizeof(before)) bytes = sizeof(before);
        if (doc_seq_read(old_source, old_start + offset, before, bytes) != XUI_OK ||
            doc_seq_read(new_source, new_start + offset, after, bytes) != XUI_OK ||
            memcmp(before, after, (size_t)bytes)) return 0;
        offset += bytes;
    }
    return 1;
}
/* Reference definitions are not semantic nodes. A generated container must not
 * lose or silently respell any definition, including unused duplicates. The
 * edited footnote owns the selected body, so compare its unchanged prefix
 * through the label while the complete semantic parse verifies its body. */
static int doc_md_reference_spelling_equal_except_footnotes(
    doc_sequence* old_refs, doc_sequence* old_source,
    doc_sequence* new_refs, doc_sequence* new_source,
    uint64_t editable_start, uint64_t editable_end, uint64_t other_start, uint64_t other_end)
{
    uint64_t i;
    if (doc_seq_size(old_refs) != doc_seq_size(new_refs) ||
        doc_seq_size(old_refs) % sizeof(xui_doc_reference_definition_t)) return 0;
    for (i = 0; i < doc_seq_size(old_refs); i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t a, b;
        if (doc_seq_read(old_refs, i, &a, sizeof(a)) != XUI_OK ||
            doc_seq_read(new_refs, i, &b, sizeof(b)) != XUI_OK ||
            a.iKind != b.iKind) return 0;
        if (a.iKind == XUI_DOC_REFERENCE_FOOTNOTE &&
            ((a.iSourceStart == editable_start && a.iSourceEnd == editable_end) ||
             (a.iSourceStart == other_start && a.iSourceEnd == other_end))) {
            if (a.iLabelStart < a.iSourceStart || a.iLabelEnd < a.iLabelStart ||
                a.iLabelEnd > a.iSourceEnd || b.iLabelStart < b.iSourceStart ||
                b.iLabelEnd < b.iLabelStart || b.iLabelEnd > b.iSourceEnd ||
                !doc_md_source_spelling_equal(old_source,
                    a.iSourceStart, a.iLabelEnd, new_source,
                    b.iSourceStart, b.iLabelEnd)) return 0;
        } else if (!doc_md_source_spelling_equal(old_source,
            a.iSourceStart, a.iSourceEnd, new_source,
            b.iSourceStart, b.iSourceEnd)) return 0;
    }
    return 1;
}
static int doc_md_reference_spelling_equal_except_footnote(
    doc_sequence* old_refs, doc_sequence* old_source, doc_sequence* new_refs, doc_sequence* new_source,
    uint64_t editable_start, uint64_t editable_end)
{
    return doc_md_reference_spelling_equal_except_footnotes(old_refs, old_source, new_refs, new_source,
        editable_start, editable_end, DOC_NONE, DOC_NONE);
}
static int doc_md_reference_spelling_equal(doc_sequence* old_refs, doc_sequence* old_source,
    doc_sequence* new_refs, doc_sequence* new_source)
{
    return doc_md_reference_spelling_equal_except_footnote(old_refs, old_source,
        new_refs, new_source, DOC_NONE, DOC_NONE);
}
/* Compare independently addressable raw fields. Multiline fields may include
 * container prefixes and need parsed-value comparison after prefix changes. */
static int doc_md_reference_fields_equal(doc_sequence* old_source,
    uint64_t old_start, uint64_t old_end, doc_sequence* new_source,
    uint64_t new_start, uint64_t new_end)
{
    if (old_start == DOC_NONE || old_end == DOC_NONE ||
        new_start == DOC_NONE || new_end == DOC_NONE)
        return old_start == DOC_NONE && old_end == DOC_NONE &&
            new_start == DOC_NONE && new_end == DOC_NONE;
    return doc_md_source_spelling_equal(old_source, old_start, old_end,
        new_source, new_start, new_end);
}
/* Prefix writers copy every non-prefix byte verbatim. Compare all ordered
 * cached definition values, including unused duplicates and multiline fields,
 * instead of mistaking a newly inserted quote marker for content. Footnote
 * bodies use the exact dedentation used by the parser's body emission. */
static int doc_md_quote_prefix_references(xui_document_transaction t,
    doc_state* old, doc_state* actual, int* equal)
{
    int result = doc_txn_check(t, 0);
    *equal = 0;
    if (result != XUI_OK) return result;
    return doc_reference_values_equal(old, actual, t->cancellation, equal);
}
static int doc_md_container_gap_trivia(doc_sequence* source, uint64_t start, uint64_t end)
{
    while (start < end) {
        char trivia[256]; uint64_t n = end - start, i;
        if (n > sizeof(trivia)) n = sizeof(trivia);
        if (doc_seq_read(source, start, trivia, n) != XUI_OK) return 0;
        for (i = 0; i < n; i++) if (trivia[i] != ' ' && trivia[i] != '\t' &&
            trivia[i] != '>' && trivia[i] != '\r' && trivia[i] != '\n') return 0;
        start += n;
    }
    return 1;
}
static uint64_t doc_md_list_marker_prefix(const char* text, uint64_t size,
    uint64_t start, int task, uint64_t* continuation)
{
    uint64_t at = start, digits = 0;
    *continuation = start;
    if (at < size && (text[at] == '-' || text[at] == '+' || text[at] == '*')) at++;
    else {
        while (at < size && text[at] >= '0' && text[at] <= '9' && digits < 9)
            at++, digits++;
        if (!digits || at >= size || (text[at] != '.' && text[at] != ')'))
            return start;
        at++;
    }
    if (at >= size || (text[at] != ' ' && text[at] != '\t')) return start;
    while (at < size && (text[at] == ' ' || text[at] == '\t')) at++;
    *continuation = at;
    if (task) {
        if (size - at < 4 || text[at] != '[' ||
            (text[at + 1] != ' ' && text[at + 1] != 'x' && text[at + 1] != 'X') ||
            text[at + 2] != ']' || (text[at + 3] != ' ' && text[at + 3] != '\t')) {
            *continuation = start;
            return start;
        }
        at += 4;
        while (at < size && (text[at] == ' ' || text[at] == '\t')) at++;
    }
    return at;
}
typedef struct doc_md_paragraph_patch {
    uint64_t old_paragraph, first_paragraph, second_paragraph;
    uint64_t old_last_paragraph;
} doc_md_paragraph_patch;
/* Build a paragraph's actual container chain from parser-confirmed tokens.
 * Source-start can also follow inline delimiters: unconfirmed nonwhitespace
 * belongs to the inline writer. Every list opener becomes its indentation on
 * continuation lines; task syntax occurs only on the first line. */
static int doc_md_paragraph_prefixes(xui_document_transaction t, doc_node* paragraph,
    const doc_node_source_range* range, const char* raw, uint64_t bytes,
    doc_md_output* first, doc_md_output* continuation,
    uint64_t outer_column, doc_md_output* outer)
{
    enum { prefix_quote = 1, prefix_list = 2, prefix_task = 3, prefix_footnote = 4 };
    struct xui_doc_snapshot_t snapshot = {0};
    unsigned char* tokens = NULL;
    doc_node* ancestor = doc_index_get(t->draft->index, paragraph->parent);
    uint64_t i, end = 0, column = 0, continued_column = 0, input_column = 0;
    int footnote_input = 0;
    unsigned depth = 0; int result = XUI_OK;
    snapshot.state = t->draft; snapshot.identity = t->document->identity; snapshot.revision = t->base_revision;
    if (bytes) {
        tokens = doc_alloc(t->draft->allocator, (size_t)bytes);
        if (!tokens) return XUI_ERROR_OUT_OF_MEMORY;
        memset(tokens, 0, (size_t)bytes);
    }
    while (ancestor && ancestor->id != DOC_ROOT) {
        xui_doc_block_syntax_t syntax = {0}; uint64_t start, stop;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) goto done;
        if (++depth > 128) { result = XUI_ERROR_NOT_FOUND; goto done; }
        syntax.iSize = sizeof(syntax);
        if (ancestor->kind == XUI_DOC_FOOTNOTE) {
            doc_node_source_range note_range; uint64_t ordinal;
            doc_node_source_range_get(t->draft, ancestor, &note_range);
            for (ordinal = 0; ordinal < doc_seq_size(t->draft->references); ordinal += sizeof(xui_doc_reference_definition_t)) {
                xui_doc_reference_definition_t reference;
                result = doc_txn_check(t, 0); if (result != XUI_OK) goto done;
                result = doc_seq_read(t->draft->references, ordinal, &reference, sizeof(reference));
                if (result != XUI_OK) goto done;
                if (reference.iKind != XUI_DOC_REFERENCE_FOOTNOTE || reference.iSourceStart != note_range.syntax_start ||
                    reference.iSourceEnd != note_range.syntax_end) continue;
                /* The parser's label range owns this first-line opener.
                 * Its entire header/gap becomes four continuation spaces;
                 * label length must not change the list's logical indent. */
                if (reference.iLabelStart < 2 || reference.iLabelEnd > UINT64_MAX - 2) break;
                start = reference.iLabelStart - 2; stop = reference.iLabelEnd + 2;
                if (start < range->syntax_start || stop > range->source_start) break;
                start -= range->syntax_start; stop -= range->syntax_start;
                if (stop < start + 4 || stop > bytes || raw[start] != '[' || raw[start + 1] != '^' ||
                    raw[stop - 2] != ']' || raw[stop - 1] != ':') { result = XUI_ERROR_NOT_FOUND; goto done; }
                while (stop < bytes && (raw[stop] == ' ' || raw[stop] == '\t')) stop++;
                memset(tokens + start, prefix_footnote, (size_t)(stop - start)); break;
            }
        } else if (ancestor->kind == XUI_DOC_LIST_ITEM || ancestor->kind == XUI_DOC_QUOTE) {
            result = xuiDocumentSnapshotGetBlockSyntax(&snapshot, ancestor->id, &syntax);
            if (result != XUI_OK) goto done;
            if (ancestor->kind == XUI_DOC_LIST_ITEM) {
                start = syntax.iPrimaryStart;
                stop = syntax.iTaskMarkerGapEnd == DOC_NONE ? syntax.iListMarkerGapEnd : syntax.iTaskMarkerGapEnd;
                if (start >= range->syntax_start && start < range->source_start) {
                    if (stop > range->source_start) {
                        uint64_t minimum = syntax.iTaskMarkerGapStart == DOC_NONE ? syntax.iPrimaryEnd : syntax.iTaskMarkerGapStart;
                        if (range->source_start < minimum) { result = XUI_ERROR_NOT_FOUND; goto done; }
                        /* A caller may stop inside parser-owned post-marker
                         * whitespace before literal code indentation. */
                        stop = range->source_start;
                    }
                    if (stop < start ||
                        syntax.iPrimaryEnd > stop) { result = XUI_ERROR_NOT_FOUND; goto done; }
                    memset(tokens + start - range->syntax_start, prefix_list, (size_t)(stop - start));
                    if (syntax.iSecondaryStart != DOC_NONE) {
                        if (syntax.iSecondaryStart < syntax.iPrimaryEnd || syntax.iSecondaryStart > stop) {
                            result = XUI_ERROR_NOT_FOUND; goto done;
                        }
                        memset(tokens + syntax.iSecondaryStart - range->syntax_start,
                            prefix_task, (size_t)(stop - syntax.iSecondaryStart));
                    }
                }
            } else {
                uint64_t low = 0, high = syntax.iQuotePrefixCount;
                while (low < high) {
                    uint64_t middle = low + (high - low) / 2;
                    result = xuiDocumentSnapshotGetQuotePrefix(&snapshot, ancestor->id, middle, &start, &stop);
                    if (result != XUI_OK) goto done;
                    if (start < range->syntax_start) low = middle + 1; else high = middle;
                }
                if (low < syntax.iQuotePrefixCount) {
                    result = xuiDocumentSnapshotGetQuotePrefix(&snapshot, ancestor->id, low, &start, &stop);
                    if (result != XUI_OK) goto done;
                    if (start < range->source_start) {
                        if (start < range->syntax_start || stop != start + 1 ||
                            stop > range->source_start || raw[start - range->syntax_start] != '>') {
                            result = XUI_ERROR_NOT_FOUND; goto done;
                        }
                        tokens[start - range->syntax_start] = prefix_quote;
                    }
                }
            }
        }
        ancestor = doc_index_get(t->draft->index, ancestor->parent);
    }
    while (end < bytes && (raw[end] == ' ' || raw[end] == '\t' || tokens[end])) end++;
    doc_md_write(first, raw, end);
    for (i = 0; i < end && !continuation->error; i++) {
        uint64_t before = continuation->size, previous_column = column;
        uint64_t width = raw[i] == '\t' ? 4 - input_column % 4 : 1;
        input_column += width;
        column += raw[i] == '\t' ? 4 - column % 4 : 1;
        if (tokens[i] == prefix_task) continue;
        if (tokens[i] == prefix_footnote) {
            /* The footnote body is a new parser input. Printed label length
             * is unrelated to its logical Tab stops. Keep physical columns
             * separately for an optional outer-prefix boundary. */
            input_column = 0; footnote_input = 1;
            if (!i || tokens[i - 1] != prefix_footnote) {
                doc_md_write(continuation, "    ", 4); continued_column += 4;
            }
        } else if (tokens[i] == prefix_list && raw[i] != ' ' && raw[i] != '\t') {
            doc_md_write(continuation, " ", 1); continued_column++;
        } else if (raw[i] == '\t' &&
            ((footnote_input && tokens[i] == prefix_list) ||
                4 - continued_column % 4 != width)) {
            static const char spaces[] = "    ";
            doc_md_write(continuation, spaces, width); continued_column += width;
        } else { doc_md_write(continuation, raw + i, 1); continued_column += width; }
        if (outer && previous_column < outer_column && !continuation->error) {
            if (column <= outer_column)
                doc_md_write(outer, continuation->data + before, continuation->size - before);
            else {
                static const char spaces[] = "    ";
                doc_md_write(outer, spaces, outer_column - previous_column);
            }
        }
    }
    result = first->error ? first->error : continuation->error ? continuation->error : outer ? outer->error : XUI_OK;
done:
    doc_free(tokens); return result;
}
static void doc_md_split_body_write(doc_md_output* output, doc_md_output* body,
    doc_md_output* first, doc_md_output* continuation,
    const char* ending, uint64_t ending_bytes, int terminal_ending, int separator)
{
    uint64_t at = 0, end;
    if (body->size < 2 || body->data[body->size - 1] != '\n' || body->data[body->size - 2] != '\n') {
        output->error = XUI_ERROR_NOT_FOUND; return;
    }
    body->size -= 2;
    do {
        end = at; while (end < body->size && body->data[end] != '\n') end++;
        doc_md_write(output, at ? continuation->data : first->data,
            at ? continuation->size : first->size);
        doc_md_write(output, body->data + at, end - at);
        if (end < body->size || terminal_ending) doc_md_write(output, ending, ending_bytes);
        if (end == body->size) break;
        at = end + 1;
    } while (!output->error);
    if (separator && !output->error) {
        doc_md_write(output, continuation->data, continuation->size);
        doc_md_write(output, ending, ending_bytes);
    }
}
/* Parser columns can start after a footnote's dedented physical prefix.
 * Anchor a logical boundary at the confirmed whitespace end; nested list
 * records may share the same raw range and a quote's optional space. */
static int doc_md_indent_end_target(uint64_t physical_end, uint32_t logical_end,
    uint32_t logical_target, uint64_t* physical_target)
{
    if (logical_target > logical_end || physical_end < logical_end) return XUI_ERROR_NOT_FOUND;
    *physical_target = physical_end - logical_end + logical_target;
    return XUI_OK;
}
static int doc_md_list_split_tail_indent(xui_document_transaction t,
    struct xui_doc_snapshot_t* snapshot, doc_node* item,
    uint64_t from, uint64_t count, int delta)
{
    static const char spaces[] = "                ";
    uint64_t i;
    if (!delta) return XUI_OK;
    if (delta <= -(int)sizeof(spaces) || delta >= (int)sizeof(spaces)) return XUI_ERROR_NOT_FOUND;
    for (i = count; i > 0; i--) {
        xui_doc_list_indent_t indent = {0}; int result;
        indent.iSize = sizeof(indent);
        result = xuiDocumentSnapshotGetListContinuationIndent(snapshot, item->id, i - 1, &indent);
        if (result != XUI_OK) return result;
        if (indent.iSourceStart < from) break;
        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
        if (delta > 0) result = doc_txn_source_patch(t, indent.iSourceEnd, indent.iSourceEnd, spaces, (uint64_t)delta, 0);
        else {
            xui_doc_source_line_t line = {0}; uint64_t at, column = 0, cut, partial = 0;
            char byte; line.iSize = sizeof(line);
            if (indent.iIndentEndColumn < (unsigned)-delta ||
                indent.iIndentEndColumn - (unsigned)-delta <= indent.iIndentStartColumn) return XUI_ERROR_NOT_FOUND;
            cut = indent.iIndentEndColumn - (unsigned)-delta;
            result = xuiDocumentSnapshotGetSourceLine(snapshot, indent.iSourceStart, &line);
            if (result != XUI_OK) return result;
            for (at = line.iLineStart; at < indent.iSourceEnd; at++) {
                if (!(at & 16383)) { result = doc_txn_check(t, 0); if (result != XUI_OK) return result; }
                result = doc_seq_read(snapshot->state->source, at, &byte, 1);
                if (result != XUI_OK) return result;
                column += byte == '\t' ? 4 - column % 4 : 1;
            }
            result = doc_md_indent_end_target(column, indent.iIndentEndColumn, (uint32_t)cut, &cut);
            if (result != XUI_OK) return result;
            column = 0;
            for (at = line.iLineStart; at < indent.iSourceEnd; at++) {
                uint64_t width;
                if (!(at & 16383)) { result = doc_txn_check(t, 0); if (result != XUI_OK) return result; }
                result = doc_seq_read(snapshot->state->source, at, &byte, 1);
                if (result != XUI_OK) return result;
                width = byte == '\t' ? 4 - column % 4 : 1;
                if (at >= indent.iSourceStart && column + width > cut) { partial = cut - column; break; }
                column += width;
                if (at >= indent.iSourceStart && column == cut) { at++; break; }
            }
            if (at < indent.iSourceStart || at >= indent.iSourceEnd || partial >= sizeof(spaces)) return XUI_ERROR_NOT_FOUND;
            result = doc_txn_source_patch(t, at, indent.iSourceEnd, spaces, partial, 0);
        }
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
static int doc_md_reference_values_equal(xui_document_transaction t, doc_state* old, doc_state* actual, int* equal)
{
    int result = doc_txn_check(t, 0); *equal = 0;
    if (result != XUI_OK) return result;
    return doc_reference_values_equal(old, actual, t->cancellation, equal);
}
static int doc_md_list_split_references(xui_document_transaction t, doc_state* old, doc_state* actual,
    uint64_t tail_start, uint64_t tail_end, int reindented,
    uint64_t editable_start, uint64_t editable_end, int* equal)
{
    uint64_t i; int needs_values = 0; *equal = 0;
    if (!reindented) { *equal = doc_md_reference_spelling_equal_except_footnote(old->references, old->source,
        actual->references, actual->source, editable_start, editable_end); return XUI_OK; }
    if (doc_seq_size(old->references) != doc_seq_size(actual->references)) return XUI_OK;
    for (i = 0; i < doc_seq_size(old->references); i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t a, b;
        if (doc_seq_read(old->references, i, &a, sizeof(a)) != XUI_OK ||
            doc_seq_read(actual->references, i, &b, sizeof(b)) != XUI_OK || a.iKind != b.iKind) return XUI_OK;
        if (a.iKind == XUI_DOC_REFERENCE_FOOTNOTE && a.iSourceStart == editable_start && a.iSourceEnd == editable_end) {
            if (!doc_md_source_spelling_equal(old->source, a.iSourceStart, a.iLabelEnd,
                actual->source, b.iSourceStart, b.iLabelEnd)) return XUI_OK;
            continue;
        }
        if (a.iKind == XUI_DOC_REFERENCE_LINK && a.iSourceStart >= tail_start && a.iSourceEnd <= tail_end) {
            if (!doc_md_reference_fields_equal(old->source, a.iLabelStart, a.iLabelEnd, actual->source, b.iLabelStart, b.iLabelEnd) ||
                !doc_md_reference_fields_equal(old->source, a.iDestinationStart, a.iDestinationEnd, actual->source, b.iDestinationStart, b.iDestinationEnd) ||
                !doc_md_reference_fields_equal(old->source, a.iTitleStart, a.iTitleEnd, actual->source, b.iTitleStart, b.iTitleEnd)) needs_values = 1;
        } else if (!doc_md_source_spelling_equal(old->source, a.iSourceStart, a.iSourceEnd, actual->source, b.iSourceStart, b.iSourceEnd)) return XUI_OK;
    }
    if (needs_values) return doc_md_reference_values_equal(t, old, actual, equal);
    *equal = 1; return XUI_OK;
}
static int doc_md_list_split_inline_patch(xui_document_transaction, doc_state*,
    const xui_doc_position_t*, struct xui_doc_snapshot_t*, doc_node*, doc_node*,
    const doc_node_source_range*, const doc_node_source_range*, doc_md_output*, doc_md_output*, doc_md_output*,
    const char*, uint64_t, uint64_t, uint64_t, int, int, uint64_t, uint64_t);
/* Split the selected paragraph's source. Definitions and later blocks remain
 * below the new item marker. If an ordered marker changes width, adjust only
 * parser-confirmed continuation whitespace; reference payloads stay intact.
 * A complete private parse checks their scope and the entire tree. */
static int doc_md_list_split_source_patch(xui_document_transaction t, doc_state* desired,
    const xui_doc_position_t* at, const xui_doc_position_t* target)
{
    doc_state* old = t->draft; struct xui_doc_snapshot_t snapshot = {0};
    doc_node *node, *paragraph, *item, *list, *first, *second, *new_item, *new_list;
    doc_node_source_range range, item_range; xui_doc_block_syntax_t syntax = {0};
    doc_list_first_indent_relative first_indent;
    struct xui_doc_transaction_t candidate = {0};
    doc_md_output first_prefix = {0}, continuation = {0}, outer = {0},
        next_prefix = {0}, next_continuation = {0}, left_body = {0}, right_body = {0}, output = {0};
    char *raw = NULL, *gap = NULL, *task_gap = NULL, marker[32], ending[2] = {'\n', 0}, check[2];
    uint64_t prefix_bytes, marker_bytes, gap_bytes, task_gap_bytes = 0,
        outer_column = DOC_NONE, i, item_index, ending_bytes = 1;
    uint64_t old_width, new_width, column, editable_start = DOC_NONE, editable_end = DOC_NONE;
    int result = XUI_ERROR_NOT_FOUND, terminal_ending = 0, indent_delta;
    node = doc_index_get(old->index, at->iNodeId);
    paragraph = at->iKind == XUI_DOC_POSITION_TEXT && node && node->kind == XUI_DOC_TEXT ?
        doc_index_get(old->index, node->parent) : at->iKind == XUI_DOC_POSITION_GAP ? node : NULL;
    item = paragraph ? doc_index_get(old->index, paragraph->parent) : NULL;
    list = item ? doc_index_get(old->index, item->parent) : NULL;
    node = doc_index_get(desired->index, target->iNodeId);
    second = target->iKind == XUI_DOC_POSITION_TEXT && node && node->kind == XUI_DOC_TEXT ?
        doc_index_get(desired->index, node->parent) : target->iKind == XUI_DOC_POSITION_GAP ? node : NULL;
    new_item = second ? doc_index_get(desired->index, second->parent) : NULL;
    first = paragraph ? doc_index_get(desired->index, paragraph->id) : NULL;
    new_list = list ? doc_index_get(desired->index, list->id) : NULL;
    if (!paragraph || (paragraph->kind != XUI_DOC_PARAGRAPH && paragraph->kind != XUI_DOC_HEADING) ||
        !item || item->kind != XUI_DOC_LIST_ITEM || !list || list->kind != XUI_DOC_LIST ||
        !first || first->parent != item->id || !second || second->kind != XUI_DOC_PARAGRAPH ||
        !new_item || new_item->kind != XUI_DOC_LIST_ITEM || new_item->parent != list->id ||
        doc_index_get(old->index, new_item->id) || !new_list ||
        doc_seq_size(list->children) == UINT64_MAX ||
        doc_seq_size(new_list->children) != doc_seq_size(list->children) + 1) return result;
    for (item_index = 0; item_index < doc_seq_size(list->children); item_index++)
        if (doc_seq_get_id(list->children, item_index) == item->id) break;
    if (item_index == doc_seq_size(list->children) ||
        doc_seq_get_id(new_list->children, item_index) != item->id ||
        doc_seq_get_id(new_list->children, item_index + 1) != new_item->id) return result;
    doc_node_source_range_get(old, paragraph, &range);
    doc_node_source_range_get(old, item, &item_range);
    if (range.syntax_start == DOC_NONE || range.source_start == DOC_NONE || range.syntax_end == DOC_NONE ||
        range.syntax_start > range.source_start || range.source_start > range.syntax_end ||
        range.syntax_start >= range.syntax_end || range.syntax_end > doc_seq_size(old->source)) return result;
    for (node = item; node && node->id != DOC_ROOT; node = doc_index_get(old->index, node->parent)) {
        if (node->kind == XUI_DOC_FOOTNOTE) {
            doc_node_source_range note_range; doc_node_source_range_get(old, node, &note_range);
            editable_start = note_range.syntax_start; editable_end = note_range.syntax_end; break;
        }
    }
    for (i = 0; i < doc_seq_size(old->references); i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t reference;
        doc_seq_read(old->references, i, &reference, sizeof(reference));
        if (reference.iSourceStart < range.syntax_end && reference.iSourceEnd > range.syntax_start &&
            !(reference.iKind == XUI_DOC_REFERENCE_FOOTNOTE && reference.iSourceStart == editable_start &&
              reference.iSourceEnd == editable_end)) return result;
    }
    snapshot.state = old; snapshot.identity = t->document->identity; snapshot.revision = t->base_revision;
    syntax.iSize = sizeof(syntax);
    result = xuiDocumentSnapshotGetBlockSyntax(&snapshot, item->id, &syntax);
    if (result != XUI_OK) return result;
    if (!item->syntax_aux || item->syntax_aux->size != sizeof(first_indent)) return XUI_ERROR_NOT_FOUND;
    memcpy(&first_indent, item->syntax_aux->data, sizeof(first_indent));
    if (first_indent.start_column >= first_indent.content_column ||
        first_indent.content_column > first_indent.end_column) return XUI_ERROR_NOT_FOUND;
    if (syntax.iPrimaryEnd <= syntax.iPrimaryStart || syntax.iPrimaryEnd - syntax.iPrimaryStart > 10 ||
        syntax.iListMarkerGapEnd < syntax.iListMarkerGapStart || syntax.iListMarkerGapEnd > doc_seq_size(old->source))
        return XUI_ERROR_NOT_FOUND;
    prefix_bytes = range.source_start - range.syntax_start;
    gap_bytes = syntax.iListMarkerGapEnd - syntax.iListMarkerGapStart;
    if (syntax.iTaskMarkerGapStart != DOC_NONE) {
        if (syntax.iTaskMarkerGapEnd < syntax.iTaskMarkerGapStart || syntax.iTaskMarkerGapEnd > doc_seq_size(old->source))
            return XUI_ERROR_NOT_FOUND;
        task_gap_bytes = syntax.iTaskMarkerGapEnd - syntax.iTaskMarkerGapStart;
    }
    if (prefix_bytes >= SIZE_MAX || gap_bytes >= SIZE_MAX || task_gap_bytes >= SIZE_MAX) return XUI_ERROR_NOT_FOUND;
    raw = doc_alloc(old->allocator, (size_t)prefix_bytes + 1);
    gap = doc_alloc(old->allocator, (size_t)gap_bytes + 1);
    task_gap = doc_alloc(old->allocator, (size_t)task_gap_bytes + 1);
    if (!raw || !gap || !task_gap) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    result = doc_seq_read(old->source, range.syntax_start, raw, prefix_bytes);
    if (result == XUI_OK) result = doc_seq_read(old->source, syntax.iListMarkerGapStart, gap, gap_bytes);
    if (result == XUI_OK && task_gap_bytes) result = doc_seq_read(old->source, syntax.iTaskMarkerGapStart, task_gap, task_gap_bytes);
    if (result != XUI_OK) goto done;
    if (syntax.iPrimaryStart >= range.syntax_start && syntax.iPrimaryStart < range.source_start) {
        outer_column = 0;
        for (i = 0; i < syntax.iPrimaryStart - range.syntax_start; i++)
            outer_column += raw[i] == '\t' ? 4 - outer_column % 4 : 1;
    } else {
        uint64_t low = 0, high = syntax.iListIndentCount;
        xui_doc_list_indent_t indent = {0}; indent.iSize = sizeof(indent);
        while (low < high) {
            uint64_t middle = low + (high - low) / 2;
            result = xuiDocumentSnapshotGetListContinuationIndent(&snapshot, item->id, middle, &indent);
            if (result != XUI_OK) goto done;
            if (indent.iSourceEnd <= range.syntax_start) low = middle + 1; else high = middle;
        }
        if (low < syntax.iListIndentCount) {
            result = xuiDocumentSnapshotGetListContinuationIndent(&snapshot, item->id, low, &indent);
            if (result != XUI_OK) goto done;
            if (indent.iSourceStart >= range.syntax_start && indent.iSourceEnd <= range.source_start)
                outer_column = indent.iIndentStartColumn;
        }
    }
    if (outer_column == DOC_NONE) { result = XUI_ERROR_NOT_FOUND; goto done; }
    first_prefix.allocator = continuation.allocator = outer.allocator = old->allocator;
    next_prefix.allocator = next_continuation.allocator = left_body.allocator = right_body.allocator = output.allocator = old->allocator;
    result = doc_md_paragraph_prefixes(t, paragraph, &range, raw, prefix_bytes,
        &first_prefix, &continuation, outer_column, &outer);
    if (result != XUI_OK) goto done;
    marker_bytes = syntax.iPrimaryEnd - syntax.iPrimaryStart;
    /* These columns come from the parser's current input. A footnote's
     * first body line is dedented independently of its physical label, so
     * measuring raw document columns changes Tab widths and tail indents. */
    old_width = first_indent.content_column - first_indent.start_column;
    result = doc_seq_read(old->source, syntax.iPrimaryStart, marker, marker_bytes);
    if (result != XUI_OK) goto done;
    if (list->attrs->iFlags & XUI_DOC_ORDERED) {
        uint64_t number;
        char delimiter = marker[marker_bytes - 1]; int length;
        if ((delimiter != '.' && delimiter != ')') || list->attrs->iListStart > UINT64_MAX - item_index - 1) {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
        number = list->attrs->iListStart + item_index + 1;
        if (number > 999999999) { result = XUI_ERROR_NOT_FOUND; goto done; }
        length = snprintf(marker, sizeof(marker), "%llu%c", (unsigned long long)number, delimiter);
        if (length <= 0 || (size_t)length >= sizeof(marker)) { result = XUI_ERROR_NOT_FOUND; goto done; }
        marker_bytes = (uint64_t)length;
    } else if (marker_bytes != 1 || (marker[0] != '-' && marker[0] != '+' && marker[0] != '*')) {
        result = XUI_ERROR_NOT_FOUND; goto done;
    }
    column = first_indent.start_column + marker_bytes;
    for (i = 0; i < gap_bytes; i++) column += gap[i] == '\t' ? 4 - column % 4 : 1;
    if (new_item->attrs->iFlags & XUI_DOC_TASK) {
        column += 3;
        for (i = 0; i < task_gap_bytes; i++) column += task_gap[i] == '\t' ? 4 - column % 4 : 1;
    }
    new_width = column - first_indent.start_column;
    if (new_width > INT_MAX || old_width > INT_MAX) { result = XUI_ERROR_NOT_FOUND; goto done; }
    indent_delta = (int)new_width - (int)old_width;
    doc_md_write(&next_prefix, outer.data, outer.size);
    doc_md_write(&next_continuation, outer.data, outer.size);
    doc_md_write(&next_prefix, marker, marker_bytes);
    for (i = 0; i < marker_bytes; i++) doc_md_write(&next_continuation, " ", 1);
    doc_md_write(&next_prefix, gap, gap_bytes); doc_md_write(&next_continuation, gap, gap_bytes);
    if (new_item->attrs->iFlags & XUI_DOC_TASK) {
        doc_md_write(&next_prefix, "[ ]", 3); doc_md_write(&next_prefix, task_gap, task_gap_bytes);
    }
    if (next_prefix.error || next_continuation.error) {
        result = next_prefix.error ? next_prefix.error : next_continuation.error; goto done;
    }
    result = doc_seq_read(old->source, range.syntax_end - 1, check, 1);
    if (result != XUI_OK) goto done;
    if (check[0] == '\n' || check[0] == '\r') {
        terminal_ending = 1; ending[0] = check[0];
        if (check[0] == '\n' && range.syntax_end >= 2 && doc_seq_read(old->source, range.syntax_end - 2, check, 1) == XUI_OK && check[0] == '\r') {
            ending[0] = '\r'; ending[1] = '\n'; ending_bytes = 2;
        }
    }
    result = doc_md_list_split_inline_patch(t, desired, at, &snapshot, item, list,
        &range, &item_range, &first_prefix, &next_prefix, &continuation, ending, ending_bytes,
        syntax.iListIndentCount, outer.size, indent_delta, terminal_ending, editable_start, editable_end);
    if (result != XUI_ERROR_NOT_FOUND) goto done;
    result = doc_md_block_write(desired, first, &left_body);
    if (result == XUI_OK) result = doc_md_block_write(desired, second, &right_body);
    if (result != XUI_OK) goto done;
    doc_md_split_body_write(&output, &left_body, &first_prefix, &continuation, ending, ending_bytes, 1,
        !(list->attrs->iFlags & XUI_DOC_TIGHT));
    doc_md_split_body_write(&output, &right_body, &next_prefix, &next_continuation, ending, ending_bytes, terminal_ending, 0);
    if (output.error) { result = output.error; goto done; }
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_md_list_split_tail_indent(&candidate, &snapshot, item,
        range.syntax_end, syntax.iListIndentCount, indent_delta);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, range.syntax_start, range.syntax_end, output.data, output.size, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_semantic_equal(desired, candidate.draft)) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        int references_equal;
        result = doc_md_list_split_references(t, old, candidate.draft,
            range.syntax_end, item_range.syntax_end, indent_delta != 0,
            editable_start, editable_end, &references_equal);
        if (result == XUI_OK && !references_equal) result = XUI_ERROR_NOT_FOUND;
    }
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft;
            candidate.draft = NULL; t->parse_op_start = t->count;
        }
    }
done:
    doc_state_release(candidate.draft); doc_free(candidate.ops);
    doc_free(raw); doc_free(gap); doc_free(task_gap);
    doc_free(first_prefix.data); doc_free(continuation.data); doc_free(outer.data);
    doc_free(next_prefix.data); doc_free(next_continuation.data); doc_free(left_body.data); doc_free(right_body.data); doc_free(output.data);
    return result;
}
/* Find one paragraph edit below a top-level container. A source patch there can
 * leave nested list/quote trivia, including definitions, entirely untouched. */
static int doc_md_find_nested_paragraph_patch(doc_state* old, doc_state* desired,
    uint64_t old_id, uint64_t new_id, unsigned depth, doc_md_paragraph_patch* pair)
{
    doc_node *before = doc_index_get(old->index, old_id),
        *after = doc_index_get(desired->index, new_id);
    uint64_t ac, bc, i, j;
    if (!before || !after || before->id != after->id ||
        before->kind != after->kind || depth > 128) return 0;
    ac = doc_seq_size(before->children); bc = doc_seq_size(after->children);
    if (depth >= 3 && (before->kind == XUI_DOC_LIST_ITEM || before->kind == XUI_DOC_QUOTE) &&
        ac < UINT64_MAX && bc == ac + 1) {
        for (i = 0; i < ac; i++) {
            doc_node* old_child = doc_index_get(old->index, doc_seq_get_id(before->children, i));
            doc_node* first = doc_index_get(desired->index, doc_seq_get_id(after->children, i));
            doc_node* second = doc_index_get(desired->index, doc_seq_get_id(after->children, i + 1));
            int siblings_equal = 1;
            if (!old_child || !first || !second || old_child->kind != XUI_DOC_PARAGRAPH ||
                first->kind != XUI_DOC_PARAGRAPH || second->kind != XUI_DOC_PARAGRAPH ||
                old_child->id != first->id) continue;
            for (j = 0; j < ac; j++) if (j != i &&
                !doc_semantic_subtree_equal(old, doc_seq_get_id(before->children, j),
                    desired, doc_seq_get_id(after->children, j + (j > i)))) {
                siblings_equal = 0; break;
            }
            if (siblings_equal) {
                *pair = (doc_md_paragraph_patch){old_child->id, first->id, second->id, 0};
                return 1;
            }
        }
    }
    if (depth >= 3 && (before->kind == XUI_DOC_LIST_ITEM || before->kind == XUI_DOC_QUOTE) &&
        bc < UINT64_MAX && ac == bc + 1) {
        for (i = 0; i < bc; i++) {
            doc_node* first = doc_index_get(old->index, doc_seq_get_id(before->children, i));
            doc_node* second = doc_index_get(old->index, doc_seq_get_id(before->children, i + 1));
            doc_node* merged = doc_index_get(desired->index, doc_seq_get_id(after->children, i));
            int siblings_equal = 1;
            if (!first || !second || !merged || first->kind != XUI_DOC_PARAGRAPH ||
                second->kind != XUI_DOC_PARAGRAPH || merged->kind != XUI_DOC_PARAGRAPH ||
                first->id != merged->id) continue;
            for (j = 0; j < ac; j++) if (j != i && j != i + 1 &&
                !doc_semantic_subtree_equal(old, doc_seq_get_id(before->children, j),
                    desired, doc_seq_get_id(after->children, j - (j > i + 1)))) {
                siblings_equal = 0; break;
            }
            if (siblings_equal) {
                *pair = (doc_md_paragraph_patch){first->id, merged->id, 0, second->id};
                return 1;
            }
        }
    }
    if (ac != bc) return 0;
    if (depth >= 3 && (before->kind == XUI_DOC_LIST_ITEM || before->kind == XUI_DOC_QUOTE)) {
        for (i = 0; i < ac; i++) {
            doc_node* old_child = doc_index_get(old->index, doc_seq_get_id(before->children, i));
            doc_node* new_child = doc_index_get(desired->index, doc_seq_get_id(after->children, i));
            int siblings_equal = 1;
            if (!old_child || !new_child || old_child->kind != XUI_DOC_PARAGRAPH ||
                new_child->kind != XUI_DOC_PARAGRAPH || old_child->id != new_child->id ||
                doc_semantic_subtree_equal(old, old_child->id, desired, new_child->id)) continue;
            for (j = 0; j < ac; j++) if (j != i &&
                !doc_semantic_subtree_equal(old, doc_seq_get_id(before->children, j),
                    desired, doc_seq_get_id(after->children, j))) {
                siblings_equal = 0; break;
            }
            if (siblings_equal) {
                *pair = (doc_md_paragraph_patch){old_child->id, new_child->id, 0, 0};
                return 1;
            }
        }
    }
    for (i = 0; i < ac; i++) {
        uint64_t child_before = doc_seq_get_id(before->children, i);
        uint64_t child_after = doc_seq_get_id(after->children, i);
        if (doc_semantic_subtree_equal(old, child_before, desired, child_after)) continue;
        if (doc_md_find_nested_paragraph_patch(old, desired, child_before, child_after,
            depth + 1, pair)) return 1;
    }
    return 0;
}
static int doc_md_nested_paragraph_patch(xui_document_transaction t, doc_state* desired)
{
    doc_state* old = t->draft; doc_md_paragraph_patch pair = {0};
    doc_node_source_range range, last_range; doc_node* paragraph;
    doc_md_output body = {0}, output = {0}, first_prefix = {0}, continuation_prefix = {0};
    struct xui_doc_transaction_t candidate = {0};
    char *prefix = NULL, ending[2] = {'\n', 0}, check[2];
    uint64_t prefix_size, i, at, end,
        ending_size = 1, gap_start = 0, gap_end = 0;
    int result = XUI_ERROR_NOT_FOUND, preserve_gap = 0;
    if (!doc_md_find_nested_paragraph_patch(old, desired, DOC_ROOT, DOC_ROOT, 0, &pair))
        return XUI_ERROR_NOT_FOUND;
    paragraph = doc_index_get(old->index, pair.old_paragraph);
    doc_node_source_range_get(old, paragraph, &range);
    if (pair.old_last_paragraph) {
        doc_node* last = doc_index_get(old->index, pair.old_last_paragraph);
        uint64_t cursor;
        if (!last) return XUI_ERROR_NOT_FOUND;
        doc_node_source_range_get(old, last, &last_range);
        if (range.syntax_end == DOC_NONE || last_range.syntax_start == DOC_NONE ||
            last_range.syntax_end == DOC_NONE || range.syntax_end > last_range.syntax_start ||
            last_range.syntax_end > doc_seq_size(old->source)) return XUI_ERROR_NOT_FOUND;
        gap_start = range.syntax_end; gap_end = last_range.syntax_start;
        cursor = gap_start;
        /* Move only complete definitions from the removed gap. All bytes
         * outside those definitions must be blank container trivia. */
        for (i = 0; i < doc_seq_size(old->references);
            i += sizeof(xui_doc_reference_definition_t)) {
            xui_doc_reference_definition_t ref;
            doc_seq_read(old->references, i, &ref, sizeof(ref));
            if (ref.iSourceEnd <= gap_start || ref.iSourceStart >= gap_end) continue;
            if (ref.iSourceStart < cursor || ref.iSourceEnd > gap_end ||
                !doc_md_container_gap_trivia(old->source, cursor, ref.iSourceStart))
                return XUI_ERROR_NOT_FOUND;
            cursor = ref.iSourceEnd; preserve_gap = 1;
        }
        if (!doc_md_container_gap_trivia(old->source, cursor, gap_end))
            return XUI_ERROR_NOT_FOUND;
        range.syntax_end = last_range.syntax_end;
    }
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
        range.source_start == DOC_NONE || range.syntax_start >= range.syntax_end ||
        range.source_start < range.syntax_start || range.source_start > range.syntax_end ||
        range.syntax_end > doc_seq_size(old->source) ||
        (range.syntax_start && (doc_seq_read(old->source, range.syntax_start - 1,
            check, 1) != XUI_OK || (check[0] != '\n' && check[0] != '\r'))))
        return XUI_ERROR_NOT_FOUND;
    for (i = 0; i < doc_seq_size(old->references);
        i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        doc_seq_read(old->references, i, &ref, sizeof(ref));
        if (ref.iSourceStart < range.syntax_end && ref.iSourceEnd > range.syntax_start &&
            !(preserve_gap && ref.iSourceStart >= gap_start && ref.iSourceEnd <= gap_end))
            return XUI_ERROR_NOT_FOUND;
    }
    prefix_size = range.source_start - range.syntax_start;
    if (prefix_size >= SIZE_MAX) return XUI_ERROR_NOT_FOUND;
    prefix = doc_alloc(old->allocator, (size_t)prefix_size + 1);
    if (!prefix) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_read(old->source, range.syntax_start, prefix, prefix_size);
    if (result != XUI_OK) goto done;
    first_prefix.allocator = continuation_prefix.allocator = old->allocator;
    result = doc_md_paragraph_prefixes(t, paragraph, &range, prefix, prefix_size,
        &first_prefix, &continuation_prefix, DOC_NONE, NULL);
    if (result != XUI_OK) goto done;
    if (range.syntax_end >= 2 && doc_seq_read(old->source, range.syntax_end - 2,
        check, 2) == XUI_OK && check[0] == '\r' && check[1] == '\n') {
        ending[0] = '\r'; ending[1] = '\n'; ending_size = 2;
    }
    body.allocator = output.allocator = old->allocator;
    result = doc_md_block_write(desired,
        doc_index_get(desired->index, pair.first_paragraph), &body);
    if (result == XUI_OK && pair.second_paragraph) result = doc_md_block_write(desired,
        doc_index_get(desired->index, pair.second_paragraph), &body);
    if (result != XUI_OK) {
        if (result != XUI_ERROR_OUT_OF_MEMORY) result = XUI_ERROR_NOT_FOUND;
        goto done;
    }
    if (preserve_gap) {
        if (body.size < 2 || body.data[body.size - 1] != '\n' ||
            body.data[body.size - 2] != '\n') {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
        body.size--;
    }
    for (at = 0; at < body.size && !output.error; at = end + 1) {
        end = at; while (end < body.size && body.data[end] != '\n') end++;
        if (end == body.size) { result = XUI_ERROR_NOT_FOUND; goto done; }
        doc_md_write(&output, at ? continuation_prefix.data : first_prefix.data,
            at ? continuation_prefix.size : first_prefix.size);
        doc_md_write(&output, body.data + at, end - at);
        doc_md_write(&output, ending, ending_size);
    }
    if (preserve_gap) for (at = gap_start; at < gap_end && !output.error;) {
        char trivia[256]; uint64_t n = gap_end - at;
        if (n > sizeof(trivia)) n = sizeof(trivia);
        result = doc_seq_read(old->source, at, trivia, n);
        if (result != XUI_OK) goto done;
        doc_md_write(&output, trivia, n); at += n;
    }
    if (output.error) { result = output.error; goto done; }
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate,
        range.syntax_start, range.syntax_end, output.data, output.size, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && (!doc_md_reference_spelling_equal(old->references,
            old->source, candidate.draft->references, candidate.draft->source) ||
            !doc_semantic_equal(desired, candidate.draft))) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++)
            result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft;
            candidate.draft = NULL; t->parse_op_start = t->count;
        }
    }
done:
    doc_state_release(candidate.draft); doc_free(candidate.ops);
    doc_free(prefix); doc_free(body.data); doc_free(output.data);
    doc_free(first_prefix.data); doc_free(continuation_prefix.data);
    return result;
}
uint64_t doc_markdown_resolve_node(doc_state* expected, doc_state* actual, uint64_t id)
{
    doc_node *n = doc_index_get(expected->index, id), *parent, *next; uint64_t i, ordinal = 0, at = 0;
    if (!n) return 0;
    next = doc_index_get(actual->index, id);
    if (next && next->kind == n->kind) return id;
    if (id == DOC_ROOT) return DOC_ROOT;
    parent = doc_index_get(expected->index, n->parent);
    next = doc_index_get(actual->index, doc_markdown_resolve_node(expected, actual, n->parent));
    if (!next) return 0;
    for (i = 0; i < doc_seq_size(parent->children); i++) {
        doc_node* p = doc_index_get(expected->index, doc_seq_get_id(parent->children, i));
        if (p->id == id) break;
        if (!doc_semantic_empty_paragraph(expected, p) && !(p->kind == XUI_DOC_TEXT && !doc_seq_size(p->text))) ordinal++;
    }
    for (i = 0; i < doc_seq_size(next->children); i++) {
        doc_node* p = doc_index_get(actual->index, doc_seq_get_id(next->children, i));
        if (doc_semantic_empty_paragraph(actual, p) || (p->kind == XUI_DOC_TEXT && !doc_seq_size(p->text))) continue;
        if (at++ == ordinal) return p->kind == n->kind ? p->id : 0;
    }
    return 0;
}
xui_doc_position_t doc_markdown_resolve_caret(xui_document_transaction t, doc_state* expected, xui_doc_position_t caret)
{
    doc_node* n = doc_index_get(expected->index, caret.iNodeId);
    doc_node* actual_root = doc_index_get(t->draft->index, DOC_ROOT); uint64_t index = 0, offset = caret.iOffset, i;
    if (doc_position_valid(t->draft, &caret)) return caret;
    if (n && doc_text_kind(n->kind) && n->kind != XUI_DOC_CODE_BLOCK && n->kind != XUI_DOC_DIAGRAM && n->kind != XUI_DOC_FRONT_MATTER) {
        doc_node* p = doc_index_get(expected->index, n->parent);
        for (i = 0; i < doc_seq_size(p->children); i++) {
            doc_node* previous = doc_index_get(expected->index, doc_seq_get_id(p->children, i));
            if (previous->id == n->id) break;
            offset += doc_seq_size(previous->text) + (previous->kind == XUI_DOC_SOFT_BREAK || previous->kind == XUI_DOC_HARD_BREAK);
        }
        n = p;
    }
    if (n) {
        doc_node* parent = doc_index_get(expected->index, n->parent);
        for (i = 0; parent && i < doc_seq_size(parent->children); i++) {
            doc_node* previous = doc_index_get(expected->index, doc_seq_get_id(parent->children, i));
            if (previous->id == n->id) break;
            if (!doc_semantic_empty_paragraph(expected, previous)) index++;
        }
        if (doc_semantic_empty_paragraph(expected, n)) {
            uint64_t mapped = doc_markdown_resolve_node(expected, t->draft, n->parent);
            if (mapped) { caret.iNodeId = mapped; caret.iKind = XUI_DOC_POSITION_GAP; caret.iOffset = index; return caret; }
        }
        n = doc_index_get(t->draft->index, doc_markdown_resolve_node(expected, t->draft, n->id));
        if (n && doc_text_kind(n->kind)) { caret.iNodeId = n->id; caret.iOffset = offset; return caret; }
        if (n && caret.iKind == XUI_DOC_POSITION_GAP) { caret.iNodeId = n->id; return caret; }
        if (n) for (i = 0; i < doc_seq_size(n->children); i++) {
            doc_node* child = doc_index_get(t->draft->index, doc_seq_get_id(n->children, i)); uint64_t length = doc_seq_size(child->text);
            if (doc_text_kind(child->kind) && offset <= length) { caret.iNodeId = child->id; caret.iOffset = offset; caret.iKind = XUI_DOC_POSITION_TEXT; return caret; }
            length += child->kind == XUI_DOC_SOFT_BREAK || child->kind == XUI_DOC_HARD_BREAK;
            if (offset >= length) offset -= length;
        }
    }
    caret.iNodeId = DOC_ROOT; caret.iKind = XUI_DOC_POSITION_GAP; caret.iOffset = doc_seq_size(actual_root->children); return caret;
}
static int doc_md_link_field_patch(xui_document_transaction, const xui_doc_range_t*,
    const char*, const char*, doc_state*);
int doc_markdown_link_patch(xui_document_transaction t, const xui_doc_range_t* range,
    const char* uri, const char* title, doc_state* desired)
{
    struct xui_doc_transaction_t candidate = {0}; doc_md_output output = {0};
    doc_node* node; doc_node_source_range source_range;
    uint64_t start, end, a, b, length, i; char* raw = NULL;
    int result = doc_md_link_field_patch(t, range, uri, title, desired);
    if (result != XUI_ERROR_NOT_FOUND) return result;
    if (!uri || !*uri || !range || range->tAnchor.iNodeId != range->tCaret.iNodeId ||
        range->tAnchor.iKind != XUI_DOC_POSITION_TEXT || range->tCaret.iKind != XUI_DOC_POSITION_TEXT)
        return XUI_ERROR_NOT_FOUND;
    node = doc_index_get(t->draft->index, range->tAnchor.iNodeId);
    if (!node || node->kind != XUI_DOC_TEXT || !node->source_exact ||
        (node->attrs->iMarks & XUI_DOC_LINK)) return XUI_ERROR_NOT_FOUND;
    a = range->tAnchor.iOffset; b = range->tCaret.iOffset;
    if (a > b) { uint64_t swap = a; a = b; b = swap; }
    length = doc_seq_size(node->text);
    if (a == b || b > length || !doc_seq_boundary(node->text, a) || !doc_seq_boundary(node->text, b))
        return XUI_ERROR_NOT_FOUND;
    doc_node_source_range_get(t->draft, node, &source_range);
    if (source_range.source_start == UINT64_MAX || source_range.source_start > UINT64_MAX - b)
        return XUI_ERROR_NOT_FOUND;
    start = source_range.source_start + a; end = source_range.source_start + b;
    if (end > doc_seq_size(t->draft->source) || end - start >= SIZE_MAX) return XUI_ERROR_NOT_FOUND;
    raw = doc_alloc(t->draft->allocator, (size_t)(end - start) + 1);
    if (!raw) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_read(t->draft->source, start, raw, end - start);
    if (result != XUI_OK) goto done;
    output.allocator = t->draft->allocator;
    doc_md_write(&output, "[", 1); doc_md_write(&output, raw, end - start);
    doc_md_write(&output, "](<", 3); doc_md_escape(&output, uri, strlen(uri));
    doc_md_write(&output, ">", 1);
    if (title && *title) { doc_md_write(&output, " \"", 2); doc_md_title(&output, title); doc_md_write(&output, "\"", 1); }
    doc_md_write(&output, ")", 1);
    if (output.error) { result = output.error; goto done; }
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, start, end, output.data, output.size, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_semantic_equal(desired, candidate.draft)) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft; candidate.draft = NULL;
            t->parse_op_start = t->count;
        }
    }
done:
    doc_state_release(candidate.draft); doc_free(candidate.ops);
    doc_free(raw); doc_free(output.data); return result;
}
/* Keep surrounding Markdown spelling when changing one parsed image. The
 * source syntax record identifies the complete image token, while the image
 * node's source range identifies its alt content. Reparse and compare the
 * entire semantic document before accepting the local patch. */
/* An image inside another image is syntax for the outer alt string, not
 * another semantic image. Follow parser-confirmed CST ancestry when binding
 * an image command, including an empty alt made from an empty nested image. */
static int doc_md_syntax_visible(doc_state* state,
    const xui_doc_inline_syntax_t* syntax, uint64_t count)
{
    uint64_t parent = syntax->iParentIndex;
    unsigned depth = 0;
    while (parent != DOC_NONE) {
        xui_doc_inline_syntax_t ancestor;
        if (parent >= count || depth++ >= DOC_MAX_DEPTH) return 0;
        doc_seq_read_syntax(state->inline_syntax, parent, &ancestor);
        if (ancestor.iKind == XUI_DOC_SYNTAX_IMAGE) return 0;
        parent = ancestor.iParentIndex;
    }
    return 1;
}
/* A field command retains the other field and all separator trivia. The CST
 * supplies token boundaries; the writer never searches for Markdown syntax. */
static int doc_md_link_fields(doc_state* state, uint64_t parent, uint64_t count,
    const char* old_uri, const char* old_title, const char* uri, const char* next_title,
    doc_md_output* output, doc_md_source_plan* plan, uint64_t* start, uint64_t* end)
{
    xui_doc_inline_syntax_t destination = {0}, title = {0}, owner;
    uint64_t i; int has_destination = 0, has_title = 0;
    int uri_changed = strcmp(old_uri, uri) != 0;
    int title_changed = strcmp(old_title, next_title) != 0;
    doc_seq_read_syntax(state->inline_syntax, parent, &owner);
    for (i = parent + 1; i < count; i++) {
        xui_doc_inline_syntax_t field; doc_seq_read_syntax(state->inline_syntax, i, &field);
        if (field.iSourceStart >= owner.iSourceEnd) break;
        if (field.iParentIndex != parent) continue;
        if (field.iKind == XUI_DOC_SYNTAX_DESTINATION) { destination = field; has_destination++; }
        if (field.iKind == XUI_DOC_SYNTAX_TITLE) { title = field; has_title++; }
    }
    if (!has_destination && owner.iDefinitionIndex != DOC_NONE) return XUI_ERROR_NOT_FOUND;
    if (has_destination != 1 || has_title > 1 || (!uri_changed && !title_changed))
        return XUI_DOC_ERROR_FORMAT;
    *start = destination.iSourceStart; *end = destination.iSourceEnd;
    if (!uri_changed && has_title) { *start = title.iSourceStart; *end = title.iSourceEnd; }
    else if (!uri_changed) *start = *end;
    else {
        doc_md_write(output, "<", 1);
        doc_md_escape(output, uri, strlen(uri));
        doc_md_write(output, ">", 1);
    }
    if (title_changed) {
        if (has_title && uri_changed) {
            char raw[256]; uint64_t at = destination.iSourceEnd;
            output->plan = plan; *end = title.iSourceEnd;
            while (at < title.iSourceStart && !output->error) {
                uint64_t bytes = title.iSourceStart - at;
                int result;
                if (bytes > sizeof(raw)) bytes = sizeof(raw);
                result = doc_seq_read(state->source, at, raw, bytes);
                if (result != XUI_OK) return result;
                doc_md_copy_source(output, raw, at, bytes); at += bytes;
            }
        }
        if (*next_title) {
            if (!has_title) {
                /* An omitted empty destination has no token to precede a title. */
                if (!uri_changed && destination.iSourceStart == destination.iSourceEnd)
                    doc_md_write(output, "<>", 2);
                doc_md_write(output, " ", 1);
            }
            doc_md_write(output, "\"", 1);
            doc_md_title(output, next_title);
            doc_md_write(output, "\"", 1);
        }
    }
    return output->error;
}
typedef struct doc_md_link_owner {
    xui_doc_inline_syntax_t syntax;
    uint64_t index, node_id, start, end;
    const char *uri, *title;
    doc_md_output output;
    doc_md_source_plan plan;
} doc_md_link_owner;
typedef struct doc_md_link_interval {
    uint64_t start, content_start, content_end, end, index, prefix_end, selected;
} doc_md_link_interval;
static int doc_md_link_interval_order(const void* left, const void* right)
{
    const doc_md_link_interval* a = left; const doc_md_link_interval* b = right;
    return a->start < b->start ? -1 : a->start > b->start;
}
static int doc_md_link_owner_order(const void* left, const void* right)
{
    const doc_md_link_owner* a = left; const doc_md_link_owner* b = right;
    return a->syntax.iSourceStart < b->syntax.iSourceStart ? -1 : a->syntax.iSourceStart > b->syntax.iSourceStart;
}
/* A semantic selection may cover multiple syntax owners with different link
 * metadata. Bind each selected projection through parser-confirmed CST, then
 * patch all fields in original descending coordinates and parse once. Full
 * semantic comparison rejects partial-owner or additional-content changes. */
static int doc_md_link_field_patch(xui_document_transaction t, const xui_doc_range_t* range,
    const char* uri, const char* title, doc_state* desired)
{
    struct xui_doc_transaction_t candidate = {0}; doc_link_source_target* targets = NULL;
    doc_md_link_owner* owners = NULL; doc_md_link_interval* intervals = NULL;
    uint64_t i, j, k, target_count = 0, owner_count = 0, syntax_count, interval_count = 0; int result;
    if (!range || !uri) return XUI_ERROR_NOT_FOUND;
    result = doc_link_source_targets(t->draft, range, &targets, &target_count);
    if (result != XUI_OK) return result;
    if (target_count > SIZE_MAX / sizeof(*owners)) { result = XUI_DOC_ERROR_LIMIT; goto done; }
    owners = doc_alloc(t->draft->allocator, (size_t)target_count * sizeof(*owners));
    if (!owners) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    memset(owners, 0, (size_t)target_count * sizeof(*owners));
    syntax_count = doc_seq_size(t->draft->inline_syntax) / sizeof(xui_doc_inline_syntax_t);
    /* Read CST once per pass. A selected large set must not scan every syntax
     * record again for each text/escape projection or search owners linearly. */
    for (k = 0; k < 2; k++) {
        uint64_t used = 0;
        for (j = 0; j < syntax_count; j++) {
            xui_doc_inline_syntax_t syntax;
            if (!(j & 255) && (result = doc_txn_check(t, 0)) != XUI_OK) goto done;
            doc_seq_read_syntax(t->draft->inline_syntax, j, &syntax);
            if (syntax.iKind != XUI_DOC_SYNTAX_LINK || syntax.iSourceStart == DOC_NONE ||
                syntax.iSourceEnd > doc_seq_size(t->draft->source) ||
                !doc_md_syntax_visible(t->draft, &syntax, syntax_count)) continue;
            if (k) intervals[used] = (doc_md_link_interval){syntax.iSourceStart, syntax.iContentStart,
                syntax.iContentEnd, syntax.iSourceEnd, j, 0, DOC_NONE};
            used++;
        }
        if (!k) {
            interval_count = used;
            if (!used) { result = XUI_ERROR_NOT_FOUND; goto done; }
            if (used > SIZE_MAX / sizeof(*intervals)) { result = XUI_DOC_ERROR_LIMIT; goto done; }
            intervals = doc_alloc(t->draft->allocator, (size_t)used * sizeof(*intervals));
            if (!intervals) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
        }
    }
    qsort(intervals, (size_t)interval_count, sizeof(*intervals), doc_md_link_interval_order);
    for (i = 0; i < interval_count; i++) intervals[i].prefix_end =
        i && intervals[i - 1].prefix_end > intervals[i].content_end ? intervals[i - 1].prefix_end : intervals[i].content_end;
    for (i = 0; i < target_count; i++) {
        uint64_t low = 0, high = interval_count; doc_md_link_interval* interval;
        while (low < high) {
            uint64_t middle = low + (high - low) / 2;
            if (intervals[middle].start <= targets[i].source_start) low = middle + 1;
            else high = middle;
        }
        if (!low) { result = XUI_ERROR_NOT_FOUND; goto done; }
        interval = &intervals[low - 1];
        if (interval->content_start > targets[i].source_start || interval->content_end < targets[i].source_end ||
            (low > 1 && intervals[low - 2].prefix_end >= targets[i].source_end)) {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
        if (interval->selected != DOC_NONE) {
            k = interval->selected;
            if (strcmp(owners[k].uri, targets[i].uri) || strcmp(owners[k].title, targets[i].title)) {
                result = XUI_ERROR_NOT_FOUND; goto done;
            }
        } else {
            k = owner_count; interval->selected = k;
            doc_seq_read_syntax(t->draft->inline_syntax, interval->index, &owners[k].syntax);
            owners[k].index = interval->index; owners[k].node_id = targets[i].node_id;
            owners[k].uri = targets[i].uri; owners[k].title = targets[i].title; owner_count++;
        }
    }
    qsort(owners, (size_t)owner_count, sizeof(*owners), doc_md_link_owner_order);
    for (i = 0; i < owner_count; i++) {
        doc_md_link_owner* owner = &owners[i];
        owner->output.allocator = t->draft->allocator;
        if (i && owners[i - 1].syntax.iSourceEnd > owner->syntax.iSourceStart) {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
        if (*uri && !strcmp(owner->uri, uri) && !strcmp(owner->title, title)) continue;
        if (owner->syntax.iFlags & XUI_DOC_SYNTAX_AUTOLINK) {
            doc_node* node = doc_index_get(t->draft->index, owner->node_id);
            doc_node_source_range source; char* label;
            doc_node_source_range_get(t->draft, node, &source);
            /* Preserve displayed autolink text. Explicit link labels must
             * escape punctuation which was literal inside the autolink. */
            if (node->kind != XUI_DOC_TEXT || source.source_start != owner->syntax.iContentStart ||
                source.source_end != owner->syntax.iContentEnd) { result = XUI_ERROR_NOT_FOUND; goto done; }
            label = doc_seq_string(t->draft->allocator, node->text);
            if (!label) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
            owner->start = owner->syntax.iSourceStart; owner->end = owner->syntax.iSourceEnd;
            if (*uri) doc_md_write(&owner->output, "[", 1);
            doc_md_escape(&owner->output, label, doc_seq_size(node->text));
            if (*uri) { doc_md_write(&owner->output, "]", 1); doc_md_destination(&owner->output, uri, title); }
            doc_free(label);
            result = owner->output.error;
        } else if (!*uri) {
            uint64_t bytes = owner->syntax.iContentEnd - owner->syntax.iContentStart; char* label;
            if (bytes >= SIZE_MAX) { result = XUI_DOC_ERROR_LIMIT; goto done; }
            label = doc_alloc(t->draft->allocator, (size_t)bytes + 1);
            if (!label) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
            result = doc_seq_read(t->draft->source, owner->syntax.iContentStart, label, bytes);
            if (result == XUI_OK) {
                /* Retain the parser-confirmed label bytes. Removing only the
                 * surrounding link syntax also preserves nested image/format
                 * spelling and container trivia. Full parse below checks any
                 * new Markdown boundary interactions before publication. */
                owner->start = owner->syntax.iSourceStart; owner->end = owner->syntax.iSourceEnd;
                owner->output.plan = &owner->plan;
                doc_md_copy_source(&owner->output, label, owner->syntax.iContentStart, bytes);
                result = owner->output.error;
            }
            doc_free(label);
        } else {
            result = doc_md_link_fields(t->draft, owner->index, syntax_count, owner->uri, owner->title, uri, title,
                &owner->output, &owner->plan, &owner->start, &owner->end);
            if (result == XUI_ERROR_NOT_FOUND) {
                char closing;
                if (owner->syntax.iContentEnd >= owner->syntax.iSourceEnd ||
                    doc_seq_read(t->draft->source, owner->syntax.iContentEnd, &closing, 1) != XUI_OK || closing != ']') {
                    result = XUI_DOC_ERROR_FORMAT; goto done;
                }
                owner->start = owner->syntax.iContentEnd + 1; owner->end = owner->syntax.iSourceEnd;
                doc_md_destination(&owner->output, uri, title); result = owner->output.error;
            }
        }
        if (result != XUI_OK) goto done;
    }
    result = doc_markdown_shadow_begin(t, &candidate);
    for (i = owner_count; i && result == XUI_OK; i--) {
        doc_md_link_owner* owner = &owners[i - 1];
        if (*uri && !strcmp(owner->uri, uri) && !strcmp(owner->title, title)) continue;
        /* The containing owner owns the retained-copy plan. */
        if (owner->plan.count) owner->output.plan = &owner->plan;
        result = doc_md_apply_source_plan(&candidate, owner->start, owner->end, &owner->output, 0);
    }
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_semantic_equal(desired, candidate.draft)) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft; candidate.draft = NULL;
            t->parse_op_start = t->count;
        }
    }
done:
    for (i = 0; i < owner_count; i++) { doc_free(owners[i].output.data); doc_free(owners[i].plan.copies); }
    doc_free(owners); doc_free(intervals); doc_free(targets); doc_state_release(candidate.draft); doc_free(candidate.ops);
    return result;
}
int doc_markdown_image_patch(xui_document_transaction t, uint64_t image_id, doc_state* desired)
{
    struct xui_doc_transaction_t candidate = {0}; doc_md_output output = {0};
    doc_md_source_plan source_plan = {0};
    doc_node *old = doc_index_get(t->draft->index, image_id), *updated = doc_index_get(desired->index, image_id);
    doc_node_source_range range; xui_doc_inline_syntax_t syntax;
    uint64_t count, i, image_index = DOC_NONE, start = DOC_NONE, end = DOC_NONE;
    uint64_t alt_start = DOC_NONE, alt_end = DOC_NONE;
    int result = XUI_ERROR_NOT_FOUND, alt_only, same_link;
    if (!old || !updated || old->kind != XUI_DOC_IMAGE || updated->kind != XUI_DOC_IMAGE) return XUI_ERROR_NOT_FOUND;
    doc_node_source_range_get(t->draft, old, &range);
    if (range.source_start == DOC_NONE || range.source_end == DOC_NONE ||
        range.source_start > range.source_end) return XUI_ERROR_NOT_FOUND;
    count = doc_seq_size(t->draft->inline_syntax) / sizeof(syntax);
    for (i = 0; i < count; i++) {
        doc_seq_read_syntax(t->draft->inline_syntax, i, &syntax);
        if (syntax.iKind != XUI_DOC_SYNTAX_IMAGE || syntax.iSourceStart == DOC_NONE ||
            syntax.iSourceEnd == DOC_NONE || syntax.iSourceEnd > doc_seq_size(t->draft->source) ||
            syntax.iContentStart > range.source_start || syntax.iContentEnd < range.source_end) continue;
        if (!doc_md_syntax_visible(t->draft, &syntax, count)) continue;
        if (start != DOC_NONE) return XUI_ERROR_NOT_FOUND;
        image_index = i;
        start = syntax.iSourceStart; end = syntax.iSourceEnd;
        alt_start = syntax.iContentStart; alt_end = syntax.iContentEnd;
    }
    if (start == DOC_NONE || start >= end || alt_start > alt_end ||
        alt_start < start || alt_end > end) return XUI_ERROR_NOT_FOUND;
    same_link = !!(old->attrs->iMarks & XUI_DOC_LINK) ==
            !!(updated->attrs->iMarks & XUI_DOC_LINK) &&
        !strcmp(doc_string(old->link_target), doc_string(updated->link_target)) &&
        !strcmp(doc_string(old->link_title), doc_string(updated->link_title));
    alt_only = same_link &&
        !strcmp(doc_string(old->resource), doc_string(updated->resource)) &&
        !strcmp(doc_string(old->title), doc_string(updated->title)) &&
        doc_attributes_equal(old->attrs, updated->attrs);
    output.allocator = t->draft->allocator;
    if (same_link && old->text == updated->text &&
        doc_attributes_equal(old->attrs, updated->attrs)) {
        result = doc_md_link_fields(t->draft, image_index, count, doc_string(old->resource), doc_string(old->title),
            doc_string(updated->resource), doc_string(updated->title), &output, &source_plan, &start, &end);
        if (result == XUI_ERROR_NOT_FOUND) {
            char closing;
            /* Reference uses become explicit without changing their hidden
             * definition or the retained label spelling. */
            if (alt_end >= end || doc_seq_read(t->draft->source, alt_end, &closing, 1) != XUI_OK || closing != ']') {
                result = XUI_DOC_ERROR_FORMAT; goto done;
            }
            start = alt_end + 1;
            doc_md_destination(&output, doc_string(updated->resource), doc_string(updated->title));
        } else if (result != XUI_OK) goto done;
    } else if (alt_only) {
        char* alt = doc_seq_string(t->draft->allocator, updated->text);
        unsigned alt_boundary = (doc_md_text_edge_space(updated->text, 0) ? DOC_MD_ENTITY_FIRST : 0) |
            (doc_md_text_edge_space(updated->text, 1) ? DOC_MD_ENTITY_LAST : 0);
        if (!alt) return XUI_ERROR_OUT_OF_MEMORY;
        doc_md_escape_boundary(&output, alt, doc_seq_size(updated->text), alt_boundary);
        doc_free(alt);
        start = alt_start; end = alt_end;
    } else if (same_link && (old->attrs->iMarks & XUI_DOC_LINK)) {
        char* alt = doc_seq_string(t->draft->allocator, updated->text);
        if (!alt) return XUI_ERROR_OUT_OF_MEMORY;
        doc_md_image_token(&output, updated, alt, doc_seq_size(updated->text), 0);
        doc_free(alt);
    } else {
        if (old->attrs->iMarks & XUI_DOC_LINK) {
            uint64_t parent_index;
            int found_link = 0;
            doc_seq_read_syntax(t->draft->inline_syntax, image_index, &syntax);
            parent_index = syntax.iParentIndex;
            while (parent_index != DOC_NONE && parent_index < count) {
                doc_seq_read_syntax(t->draft->inline_syntax, parent_index, &syntax);
                if (syntax.iKind == XUI_DOC_SYNTAX_LINK &&
                    syntax.iSourceStart != DOC_NONE && syntax.iSourceEnd != DOC_NONE &&
                    syntax.iSourceStart <= start && syntax.iSourceEnd >= end &&
                    syntax.iSourceEnd <= doc_seq_size(t->draft->source)) {
                    start = syntax.iSourceStart; end = syntax.iSourceEnd;
                    found_link = 1; break;
                }
                if (syntax.iParentIndex == parent_index) break;
                parent_index = syntax.iParentIndex;
            }
            if (!found_link) return XUI_ERROR_NOT_FOUND;
        }
        doc_md_inline(desired, updated, &output);
    }
    if (output.error) { result = output.error; goto done; }
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_md_apply_source_plan(&candidate, start, end, &output, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_semantic_equal(desired, candidate.draft)) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft; candidate.draft = NULL;
            t->parse_op_start = t->count;
        }
    }
done:
    doc_state_release(candidate.draft); doc_free(candidate.ops); doc_free(output.data); doc_free(source_plan.copies);
    return result;
}
int doc_markdown_image_insert_patch(xui_document_transaction t, const xui_doc_range_t* range,
    uint64_t image_id, doc_state* desired)
{
    struct xui_doc_transaction_t candidate = {0}; doc_md_output output = {0};
    doc_node *old, *image; uint64_t a, b, start, end, i; int first, last, result;
    if (!range || range->tAnchor.iKind != XUI_DOC_POSITION_TEXT ||
        range->tCaret.iKind != XUI_DOC_POSITION_TEXT ||
        range->tAnchor.iNodeId != range->tCaret.iNodeId) return XUI_ERROR_NOT_FOUND;
    old = doc_index_get(t->draft->index, range->tAnchor.iNodeId);
    image = doc_index_get(desired->index, image_id);
    if (!old || old->kind != XUI_DOC_TEXT || !image || image->kind != XUI_DOC_IMAGE)
        return XUI_ERROR_NOT_FOUND;
    a = range->tAnchor.iOffset; b = range->tCaret.iOffset;
    if (a > b) { uint64_t swap = a; a = b; b = swap; }
    if (b > doc_seq_size(old->text) || !doc_seq_boundary(old->text, a) || !doc_seq_boundary(old->text, b) ||
        doc_source_map_text(t->draft, old, a, XUI_DOC_AFTER, &start, &first) != XUI_OK ||
        first != XUI_DOC_MAP_EXACT ||
        (a != b && (doc_source_map_text(t->draft, old, b, XUI_DOC_BEFORE, &end, &last) != XUI_OK ||
            last != XUI_DOC_MAP_EXACT))) return XUI_ERROR_NOT_FOUND;
    if (a == b) end = start;
    if (start > end || end > doc_seq_size(t->draft->source)) return XUI_ERROR_NOT_FOUND;
    output.allocator = t->draft->allocator;
    doc_md_inline(desired, image, &output);
    if (output.error) { result = output.error; goto done; }
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, start, end, output.data, output.size, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_semantic_equal(desired, candidate.draft)) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft; candidate.draft = NULL;
            t->parse_op_start = t->count;
        }
    }
done:
    doc_state_release(candidate.draft); doc_free(candidate.ops); doc_free(output.data);
    return result;
}
typedef struct doc_md_heading_patch {
    uint64_t start, end, marker_bytes;
    char marker[8];
    char* owned_marker;
} doc_md_heading_patch;

static int doc_markdown_heading_patch_order(const void* left, const void* right)
{
    const doc_md_heading_patch* a = left;
    const doc_md_heading_patch* b = right;
    return a->start < b->start ? -1 : a->start > b->start ? 1 : 0;
}

static int doc_markdown_heading_patch_bounds(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow, const xui_doc_operation_t* op,
    doc_md_heading_patch* patch, uint64_t* count)
{
    doc_node *old, *updated, *parent;
    doc_node_source_range range;
    xui_doc_attributes_t attrs;
    uint64_t start, i, low, high;
    int result = XUI_ERROR_NOT_FOUND;
    if (op->iKind != XUI_DOC_OP_ATTRIBUTES ||
        !(op->iFlags & XUI_DOC_CHANGE_STRUCTURE)) return result;
    old = doc_index_get(t->draft->index, op->iNodeId);
    updated = doc_index_get(shadow->draft->index, op->iNodeId);
    parent = old ? doc_index_get(t->draft->index, old->parent) : NULL;
    if (!old || !updated || old->parent != updated->parent ||
        !parent || (parent->kind != XUI_DOC_ROOT && parent->kind != XUI_DOC_QUOTE &&
            parent->kind != XUI_DOC_LIST_ITEM &&
            parent->kind != XUI_DOC_FOOTNOTE) ||
        (old->kind != XUI_DOC_PARAGRAPH && old->kind != XUI_DOC_HEADING) ||
        (updated->kind != XUI_DOC_PARAGRAPH && updated->kind != XUI_DOC_HEADING) ||
        (old->kind == updated->kind &&
            old->attrs->iHeadingLevel == updated->attrs->iHeadingLevel) ||
        (old->kind == XUI_DOC_HEADING && !old->attrs->iHeadingLevel) ||
        (updated->kind == XUI_DOC_HEADING &&
            (!updated->attrs->iHeadingLevel || updated->attrs->iHeadingLevel > 6)) ||
        old->children != updated->children) return result;
    attrs = *updated->attrs;
    attrs.iHeadingLevel = old->attrs->iHeadingLevel;
    if (!doc_attributes_equal(&attrs, old->attrs)) return result;
    doc_node_source_range_get(t->draft, old, &range);
    if (old->kind == XUI_DOC_HEADING) {
        struct xui_doc_snapshot_t snapshot = {0};
        xui_doc_block_syntax_t syntax = {0};
        xui_doc_source_line_t line = {0};
        snapshot.state = t->draft; syntax.iSize = sizeof(syntax); line.iSize = sizeof(line);
        result = xuiDocumentSnapshotGetBlockSyntax(&snapshot, old->id, &syntax);
        if (result != XUI_OK) return result;
        patch[0].start = syntax.iPrimaryStart; patch[0].end = syntax.iPrimaryEnd;
        *count = 1;
        if (syntax.iKind == XUI_DOC_BLOCK_SYNTAX_ATX_HEADING) {
            if (updated->kind == XUI_DOC_HEADING) {
                patch[0].marker_bytes = updated->attrs->iHeadingLevel;
                memset(patch[0].marker, '#', (size_t)patch[0].marker_bytes);
            } else {
                patch[0].end = syntax.iHeadingContentStart;
                if (syntax.iSecondaryStart != DOC_NONE) {
                    result = xuiDocumentSnapshotGetSourceLine(&snapshot, syntax.iSecondaryStart, &line);
                    if (result != XUI_OK) return result;
                    patch[1].start = syntax.iHeadingContentEnd; patch[1].end = line.iContentEnd;
                    *count = 2;
                }
            }
            return XUI_OK;
        }
        if (syntax.iKind != XUI_DOC_BLOCK_SYNTAX_SETEXT_HEADING) return XUI_ERROR_NOT_FOUND;
        if (updated->kind == XUI_DOC_HEADING && updated->attrs->iHeadingLevel <= 2) {
            uint64_t bytes = patch[0].end - patch[0].start;
            if (bytes > SIZE_MAX) return XUI_ERROR_UNSUPPORTED;
            patch[0].owned_marker = doc_alloc(t->draft->allocator, (size_t)bytes);
            if (!patch[0].owned_marker) return XUI_ERROR_OUT_OF_MEMORY;
            patch[0].marker_bytes = bytes;
            for (i = 0; i < bytes; ) {
                uint64_t part = bytes - i < 4096 ? bytes - i : 4096;
                result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
                memset(patch[0].owned_marker + i, updated->attrs->iHeadingLevel == 1 ? '=' : '-', (size_t)part);
                i += part;
            }
        } else {
            result = xuiDocumentSnapshotGetSourceLine(&snapshot, syntax.iPrimaryStart, &line);
            if (result != XUI_OK) return result;
            /* The confirmed underline occupies a complete physical line,
             * including its container prefix. Keep the preceding line ending. */
            patch[0].start = line.iLineStart; patch[0].end = line.iLineEnd;
            if (updated->kind == XUI_DOC_HEADING) {
                xui_doc_source_line_t body_line = {0}; body_line.iSize = sizeof(body_line);
                result = xuiDocumentSnapshotGetSourceLine(&snapshot, syntax.iHeadingContentStart, &body_line);
                if (result != XUI_OK) return result;
                if (syntax.iHeadingContentEnd > body_line.iContentEnd) return XUI_ERROR_NOT_FOUND;
                patch[1].start = patch[1].end = syntax.iHeadingContentStart;
                patch[1].marker_bytes = updated->attrs->iHeadingLevel + 1;
                memset(patch[1].marker, '#', updated->attrs->iHeadingLevel);
                patch[1].marker[updated->attrs->iHeadingLevel] = ' ';
                *count = 2;
            }
        }
        return XUI_OK;
    }
    if (range.syntax_start == DOC_NONE || range.source_start == DOC_NONE ||
        range.syntax_start > range.source_start ||
        range.source_start > range.syntax_end ||
        range.syntax_end > doc_seq_size(t->draft->source)) return result;
    start = range.source_start;
    low = 0;
    high = doc_seq_size(t->draft->inline_syntax) /
        sizeof(xui_doc_inline_syntax_t);
    while (low < high) {
        uint64_t middle = low + (high - low) / 2;
        xui_doc_inline_syntax_t syntax;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read_syntax(t->draft->inline_syntax,
            middle, &syntax);
        if (result != XUI_OK) return result;
        if (syntax.iSourceStart < range.syntax_start) low = middle + 1;
        else high = middle;
    }
    if (low < doc_seq_size(t->draft->inline_syntax) /
        sizeof(xui_doc_inline_syntax_t)) {
        xui_doc_inline_syntax_t syntax;
        result = doc_seq_read_syntax(t->draft->inline_syntax, low, &syntax);
        if (result != XUI_OK) return result;
        if (syntax.iSourceStart >= range.syntax_start &&
            syntax.iSourceStart < start &&
            syntax.iSourceEnd <= range.syntax_end)
            start = syntax.iSourceStart;
    }
    patch->start = start;
    patch->end = start;
    for (i = 0; i < updated->attrs->iHeadingLevel; i++) patch->marker[i] = '#';
    patch->marker_bytes = updated->attrs->iHeadingLevel;
    patch->marker[patch->marker_bytes++] = ' ';
    *count = 1;
    return XUI_OK;
}

/* Multiple selected headings can belong to different footnotes. Only notes
 * whose body actually contains a patch may change; their labels and every
 * other definition, including unused duplicates, keep their raw spelling. */
static int doc_md_heading_references_equal(doc_state* old, doc_state* actual,
    const doc_md_heading_patch* patches, uint64_t count)
{
    uint64_t i, j;
    if (doc_seq_size(old->references) != doc_seq_size(actual->references)) return 0;
    for (i = 0; i < doc_seq_size(old->references); i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t a, b; int edited = 0;
        if (doc_seq_read(old->references, i, &a, sizeof(a)) != XUI_OK ||
            doc_seq_read(actual->references, i, &b, sizeof(b)) != XUI_OK || a.iKind != b.iKind) return 0;
        if (a.iKind == XUI_DOC_REFERENCE_FOOTNOTE) {
            for (j = 0; j < count; j++)
                if (patches[j].start >= a.iLabelEnd && patches[j].start < a.iSourceEnd &&
                    patches[j].end <= a.iSourceEnd) { edited = 1; break; }
        }
        if (!doc_md_source_spelling_equal(old->source, a.iSourceStart,
                edited ? a.iLabelEnd : a.iSourceEnd, actual->source, b.iSourceStart,
                edited ? b.iLabelEnd : b.iSourceEnd)) return 0;
    }
    return 1;
}

/* Independent heading changes only touch their line markers. Apply from the
 * end so offsets in the original source stay valid, then verify the complete
 * desired semantic tree before publishing any candidate. */
static int doc_markdown_heading_marker_patch(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow)
{
    struct xui_doc_transaction_t candidate = {0};
    doc_md_heading_patch* patches;
    uint64_t i, count = 0, capacity;
    int result = XUI_OK;
    if (!shadow->count || shadow->count > SIZE_MAX / sizeof(*patches) / 2)
        return XUI_ERROR_NOT_FOUND;
    for (i = 0; i < shadow->count; i++)
        if (shadow->ops[i].iKind != XUI_DOC_OP_ATTRIBUTES ||
            !(shadow->ops[i].iFlags & XUI_DOC_CHANGE_STRUCTURE))
            return XUI_ERROR_NOT_FOUND;
    capacity = shadow->count * 2;
    patches = doc_alloc(t->document->allocator, (size_t)capacity * sizeof(*patches));
    if (!patches) return XUI_ERROR_OUT_OF_MEMORY;
    for (i = 0; i < shadow->count && result == XUI_OK; i++) {
        uint64_t added = 0;
        result = doc_markdown_heading_patch_bounds(t, shadow,
            &shadow->ops[i], &patches[count], &added);
        count += added;
    }
    if (result != XUI_OK) goto done;
    qsort(patches, (size_t)count, sizeof(*patches),
        doc_markdown_heading_patch_order);
    for (i = 1; i < count; i++) {
        if (patches[i - 1].start == patches[i].start ||
            patches[i - 1].end > patches[i].start) {
            result = XUI_ERROR_NOT_FOUND;
            goto done;
        }
    }
    result = doc_markdown_shadow_begin(t, &candidate);
    for (i = count; result == XUI_OK && i > 0; i--)
        result = doc_txn_source_patch(&candidate, patches[i - 1].start,
            patches[i - 1].end, patches[i - 1].owned_marker ? patches[i - 1].owned_marker : patches[i - 1].marker,
            patches[i - 1].marker_bytes, count == 1);
    if (result == XUI_OK && candidate.count > candidate.parse_op_start) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && count == 1 && !doc_semantic_equal(shadow->draft, candidate.draft))
        result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && (!doc_semantic_equal(shadow->draft, candidate.draft) ||
        !doc_md_heading_references_equal(t->draft, candidate.draft, patches, count)))
        result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_DOC_ERROR_FORMAT ||
        result == XUI_DOC_ERROR_UNREPRESENTABLE ||
        result == XUI_ERROR_UNSUPPORTED) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++)
            result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft;
            candidate.draft = NULL; t->parse_op_start = t->count;
        }
    }
done:
    doc_state_release(candidate.draft); doc_free(candidate.ops);
    for (i = 0; i < capacity; i++) doc_free(patches[i].owned_marker);
    doc_free(patches);
    return result;
}
/* Find the physical boundary of a retained prefix/suffix through any
 * number of split Quote/List/Item layers. A retained edge may have the same
 * child count as before when only its last/first nested child was split. */
static int doc_md_retained_edge_cut(doc_state* old, doc_state* desired,
    doc_node* original, doc_node* retained, int suffix, unsigned depth,
    uint64_t* cut)
{
    uint64_t oc, rc, i;
    if (!original || !retained || original->kind != retained->kind ||
        (original->kind != XUI_DOC_QUOTE && original->kind != XUI_DOC_LIST &&
            original->kind != XUI_DOC_LIST_ITEM)) return XUI_ERROR_NOT_FOUND;
    if (depth >= DOC_MAX_DEPTH) return XUI_DOC_ERROR_LIMIT;
    oc = doc_seq_size(original->children); rc = doc_seq_size(retained->children);
    if (!rc || rc > oc || (suffix && original->kind == XUI_DOC_LIST_ITEM && rc != oc))
        return XUI_ERROR_NOT_FOUND;
    for (i = 0; i < rc; i++) {
        uint64_t oi = suffix ? oc - rc + i : i;
        uint64_t old_id = doc_seq_get_id(original->children, oi);
        uint64_t new_id = doc_seq_get_id(retained->children, i);
        if (!doc_semantic_subtree_equal(old, old_id, desired, new_id)) {
            /* A partial item's suffix starts at a continuation line, without
             * an item opener. It needs a regenerated list marker; copying it
             * as a standalone raw list suffix would change the tree. */
            if ((suffix && original->kind == XUI_DOC_LIST_ITEM) ||
                i != (suffix ? 0 : rc - 1)) return XUI_ERROR_NOT_FOUND;
            {
                int result = doc_md_retained_edge_cut(old, desired,
                    doc_index_get(old->index, old_id),
                    doc_index_get(desired->index, new_id), suffix, depth + 1, cut);
                if (result != XUI_OK) return result;
            }
        } else if (i == (suffix ? 0 : rc - 1)) {
            doc_node_source_range range;
            doc_node_source_range_get(old, doc_index_get(old->index, old_id), &range);
            *cut = suffix ? range.syntax_start : range.syntax_end;
            if (*cut == DOC_NONE) return XUI_ERROR_NOT_FOUND;
        }
    }
    return XUI_OK;
}
/* A quote or list split can leave its leading children untouched while moving
 * its suffix into a new sibling. Keep the leading source bytes verbatim; the
 * generated outer quote starts after a blank line. */
static int doc_md_retained_container_prefix(doc_state* old, doc_state* desired,
    doc_node* before, doc_node* after, uint64_t prefix, uint64_t suffix,
    uint64_t* cut)
{
    doc_node *original, *retained, *next;
    uint64_t ac = doc_seq_size(before->children), bc = doc_seq_size(after->children);
    uint64_t start, end;
    char previous; int result;
    if (ac <= prefix + suffix || bc < prefix + suffix + 2)
        return XUI_ERROR_NOT_FOUND;
    original = doc_index_get(old->index, doc_seq_get_id(before->children, prefix));
    retained = doc_index_get(desired->index, doc_seq_get_id(after->children, prefix));
    next = doc_index_get(desired->index, doc_seq_get_id(after->children, prefix + 1));
    if (!original || !retained || !next ||
        (original->kind != XUI_DOC_QUOTE && original->kind != XUI_DOC_LIST) ||
        retained->kind != original->kind ||
        next->kind != XUI_DOC_QUOTE || original->id != retained->id)
        return XUI_ERROR_NOT_FOUND;
    result = doc_md_retained_edge_cut(old, desired, original, retained, 0, 0, cut);
    if (result != XUI_OK) return result;
    if (doc_md_bounds(old, original->id, &start, &end) != XUI_OK ||
        *cut <= start || *cut >= end || *cut > doc_seq_size(old->source) ||
        doc_seq_read(old->source, *cut - 1, &previous, 1) != XUI_OK ||
        (previous != '\n' && previous != '\r'))
        return XUI_DOC_ERROR_UNREPRESENTABLE;
    return XUI_OK;
}
/* The other edge keeps trailing children in the old quote or list. Replace
 * only the selected prefix and leave the trailing source span, including its
 * original entities and line endings, in place. */
static int doc_md_retained_container_suffix(doc_state* old, doc_state* desired,
    doc_node* before, doc_node* after, uint64_t prefix, uint64_t suffix,
    int allow_cloned_list_tail, uint64_t* cut)
{
    doc_node *original, *retained, *previous;
    uint64_t ac = doc_seq_size(before->children), bc = doc_seq_size(after->children);
    uint64_t start, end;
    char prior; int result;
    if (ac <= prefix + suffix || bc < prefix + suffix + 2)
        return XUI_ERROR_NOT_FOUND;
    original = doc_index_get(old->index,
        doc_seq_get_id(before->children, ac - suffix - 1));
    retained = doc_index_get(desired->index,
        doc_seq_get_id(after->children, bc - suffix - 1));
    previous = doc_index_get(desired->index,
        doc_seq_get_id(after->children, bc - suffix - 2));
    if (!original || !retained || !previous ||
        (original->kind != XUI_DOC_QUOTE && original->kind != XUI_DOC_LIST) ||
        retained->kind != original->kind ||
        previous->kind != XUI_DOC_QUOTE ||
        (original->id != retained->id &&
            !(allow_cloned_list_tail && original->kind == XUI_DOC_LIST)))
        return XUI_ERROR_NOT_FOUND;
    result = doc_md_retained_edge_cut(old, desired, original, retained, 1, 0, cut);
    if (result != XUI_OK) return result;
    if (original->kind == XUI_DOC_LIST &&
        (retained->attrs->iFlags & XUI_DOC_ORDERED)) {
        doc_node* item = doc_index_get(old->index, doc_seq_get_id(original->children,
            doc_seq_size(original->children) - doc_seq_size(retained->children)));
        doc_node_source_range range; char marker[16]; uint64_t value = 0, bytes, j;
        if (!item || item->marker_primary_end <= item->marker_primary_start)
            return XUI_ERROR_NOT_FOUND;
        doc_node_source_range_get(old, item, &range);
        bytes = item->marker_primary_end - item->marker_primary_start;
        if (range.syntax_start == DOC_NONE || bytes < 2 || bytes >= sizeof(marker) ||
            range.syntax_start > UINT64_MAX - item->marker_primary_end)
            return XUI_ERROR_NOT_FOUND;
        result = doc_seq_read(old->source, range.syntax_start + item->marker_primary_start,
            marker, bytes);
        if (result != XUI_OK) return result;
        for (j = 0; j + 1 < bytes; j++) {
            if (marker[j] < '0' || marker[j] > '9') return XUI_ERROR_NOT_FOUND;
            value = value * 10 + (uint64_t)(marker[j] - '0');
        }
        /* A split item can add a number before this tail. Markdown ignores
         * later marker numbers until that item becomes the new list opener.
         * Regenerate its list when the retained first marker would disagree. */
        if ((marker[bytes - 1] != '.' && marker[bytes - 1] != ')') ||
            value != retained->attrs->iListStart) return XUI_ERROR_NOT_FOUND;
    }
    if (doc_md_bounds(old, original->id, &start, &end) != XUI_OK ||
        *cut <= start || *cut >= end || *cut > doc_seq_size(old->source) ||
        doc_seq_read(old->source, *cut - 1, &prior, 1) != XUI_OK ||
        (prior != '\n' && prior != '\r'))
        return XUI_DOC_ERROR_UNREPRESENTABLE;
    return XUI_OK;
}
/* A list split inside a quote otherwise makes the root-block writer respell
 * untouched children. Splice its canonical middle between the original raw
 * prefix/suffix, including partially retained list items at either edge.
 * The resulting source is still reparsed against the complete desired tree. */
static int doc_md_nested_quote_retained_edges(doc_state* old, doc_state* desired,
    doc_node* before, doc_node* after, uint64_t start, uint64_t end,
    const char* source, uint64_t source_base, doc_md_output* output,
    int* raw_suffix)
{
    doc_md_output canonical = {0}, prefix_body = {0}, prefix = {0};
    doc_md_output suffix_body = {0}, suffix = {0};
    doc_node_source_range range;
    doc_node* selected;
    uint64_t cut = start, suffix_cut = end, old_count, new_count;
    uint64_t whole_prefix = 0, whole_suffix = 0, prefix_count, suffix_count, i;
    int partial_prefix = 0, partial_suffix = 0, result;
    *raw_suffix = 0;
    if (!before || !after || before->id != after->id ||
        before->kind != XUI_DOC_QUOTE || after->kind != XUI_DOC_QUOTE ||
        (before->info && before->info->size) ||
        (after->info && after->info->size)) return XUI_ERROR_NOT_FOUND;
    old_count = doc_seq_size(before->children);
    new_count = doc_seq_size(after->children);
    if (!old_count || new_count < 2) return XUI_ERROR_NOT_FOUND;
    while (whole_prefix < old_count && whole_prefix < new_count &&
        doc_semantic_subtree_equal(old,
            doc_seq_get_id(before->children, whole_prefix), desired,
            doc_seq_get_id(after->children, whole_prefix))) whole_prefix++;
    while (whole_suffix < old_count - whole_prefix &&
        whole_suffix < new_count - whole_prefix &&
        doc_semantic_subtree_equal(old,
            doc_seq_get_id(before->children, old_count - whole_suffix - 1),
            desired,
            doc_seq_get_id(after->children, new_count - whole_suffix - 1)))
        whole_suffix++;
    result = doc_md_retained_container_prefix(old, desired,
        before, after, whole_prefix, whole_suffix, &cut);
    if (result != XUI_OK && result != XUI_ERROR_NOT_FOUND) return result;
    partial_prefix = result == XUI_OK;
    result = doc_md_retained_container_suffix(old, desired,
        before, after, whole_prefix, whole_suffix, 1, &suffix_cut);
    if (result != XUI_OK && result != XUI_ERROR_NOT_FOUND) return result;
    partial_suffix = result == XUI_OK;
    if (!partial_prefix && !partial_suffix) return XUI_ERROR_NOT_FOUND;
    prefix_count = whole_prefix + (uint64_t)partial_prefix;
    suffix_count = whole_suffix + (uint64_t)partial_suffix;
    if (new_count != prefix_count + suffix_count + 1)
        return XUI_ERROR_NOT_FOUND;
    selected = doc_index_get(desired->index,
        doc_seq_get_id(after->children, prefix_count));
    if (!selected || selected->kind != XUI_DOC_QUOTE)
        return XUI_ERROR_NOT_FOUND;
    if (!partial_prefix && whole_prefix) {
        doc_node* last = doc_index_get(old->index,
            doc_seq_get_id(before->children, whole_prefix - 1));
        if (!last) return XUI_ERROR_NOT_FOUND;
        doc_node_source_range_get(old, last, &range);
        cut = range.syntax_end;
    }
    if (!partial_suffix && whole_suffix) {
        doc_node* first = doc_index_get(old->index,
            doc_seq_get_id(before->children, old_count - whole_suffix));
        if (!first) return XUI_ERROR_NOT_FOUND;
        doc_node_source_range_get(old, first, &range);
        suffix_cut = range.syntax_start;
    }
    if (start < source_base || cut == DOC_NONE || suffix_cut == DOC_NONE ||
        cut < start || suffix_cut > end || cut >= suffix_cut ||
        (prefix_count && cut == start) || (suffix_count && suffix_cut == end))
        return XUI_ERROR_NOT_FOUND;
    canonical.allocator = prefix_body.allocator = prefix.allocator =
        suffix_body.allocator = suffix.allocator = output->allocator;
    result = doc_md_block_write(desired, after, &canonical);
    if (result != XUI_OK) goto done;
    if (prefix_count) {
        for (i = 0; i < prefix_count; i++) {
            doc_node* child = doc_index_get(desired->index,
                doc_seq_get_id(after->children, i));
            if (!child || doc_semantic_empty_paragraph(desired, child)) {
                result = XUI_ERROR_NOT_FOUND; goto done;
            }
            result = doc_md_block_write(desired, child, &prefix_body);
            if (result != XUI_OK) goto done;
        }
        if (prefix_body.size < 2 ||
            prefix_body.data[prefix_body.size - 1] != '\n' ||
            prefix_body.data[prefix_body.size - 2] != '\n') {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
        prefix_body.data[--prefix_body.size] = 0;
        doc_md_prefixed(&prefix, &prefix_body, "> ", "> ");
        if (prefix.error) { result = prefix.error; goto done; }
        if (!prefix.size || prefix.size > canonical.size ||
            memcmp(prefix.data, canonical.data, (size_t)prefix.size)) {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
    }
    if (suffix_count) {
        for (i = new_count - suffix_count; i < new_count; i++) {
            doc_node* child = doc_index_get(desired->index,
                doc_seq_get_id(after->children, i));
            if (!child || doc_semantic_empty_paragraph(desired, child)) {
                result = XUI_ERROR_NOT_FOUND; goto done;
            }
            result = doc_md_block_write(desired, child, &suffix_body);
            if (result != XUI_OK) goto done;
        }
        doc_md_prefixed(&suffix, &suffix_body, "> ", "> ");
        doc_md_write(&suffix, "\n", 1);
        if (suffix.error) { result = suffix.error; goto done; }
        if (!suffix.size || suffix.size > canonical.size - prefix.size ||
            memcmp(suffix.data, canonical.data + canonical.size - suffix.size,
                (size_t)suffix.size)) {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
    }
    if (prefix_count) doc_md_write(output,
        source + start - source_base, cut - start);
    doc_md_write(output, canonical.data + prefix.size,
        canonical.size - prefix.size - suffix.size);
    if (suffix_count) doc_md_write(output,
        source + suffix_cut - source_base, end - suffix_cut);
    result = output->error;
    if (result == XUI_OK) *raw_suffix = suffix_count > 0;
done:
    doc_free(canonical.data); doc_free(prefix_body.data); doc_free(prefix.data);
    doc_free(suffix_body.data); doc_free(suffix.data);
    return result;
}
static int doc_md_quote_prefix_position(doc_node* quote,
    const doc_node_source_range* range, uint64_t index, uint64_t* offset)
{
    uint32_t relative;
    if (!quote || quote->marker_kind != XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN ||
        (quote->quote_prefixes &&
            quote->quote_prefixes->size % sizeof(relative)))
        return XUI_ERROR_NOT_FOUND;
    if (!index) relative = quote->marker_primary_start;
    else {
        if (!quote->quote_prefixes ||
            index - 1 >= quote->quote_prefixes->size / sizeof(relative))
            return XUI_ERROR_NOT_FOUND;
        memcpy(&relative, quote->quote_prefixes->data +
            (index - 1) * sizeof(relative), sizeof(relative));
    }
    if (range->syntax_start == DOC_NONE || range->syntax_end == DOC_NONE ||
        range->syntax_end < range->syntax_start ||
        relative >= range->syntax_end - range->syntax_start)
        return XUI_DOC_ERROR_FORMAT;
    *offset = range->syntax_start + relative;
    return XUI_OK;
}
/* Descend a single changed Quote/List/Item path from a root block to the
 * quote receiving a new child quote. Add its parser-recorded marker only to
 * selected physical lines; all other source bytes remain verbatim. */
/* Four physical columns preserve indentation Tab stops through nested
 * quote and footnote analysis. The two spaces are legal quote indent. */
static int doc_md_quote_tab_padding(xui_document_transaction t, const char* source,
    uint64_t source_base, uint64_t start, uint64_t end, unsigned* padding)
{
    uint64_t at; int leading = 1;
    *padding = 0;
    for (at = start; at < end; at++) {
        char c = source[at - source_base];
        if (!(at & 16383)) { int result = doc_txn_check(t, 0); if (result != XUI_OK) return result; }
        if (c == '\r' || c == '\n') leading = 1;
        else if (leading) {
            if (c == '\t') { *padding = 2; return XUI_OK; }
            if (c != ' ' && c != '>') leading = 0;
        }
    }
    return XUI_OK;
}
static int doc_md_nested_quote_prefix_patch(xui_document_transaction t,
    doc_state* old, doc_state* desired, doc_node* before, doc_node* after,
    uint64_t start, uint64_t end, const char* source,
    uint64_t source_base, doc_md_output* output, int* raw_suffix)
{
    doc_node *old_cursor = before, *new_cursor = after;
    doc_node *old_quote = NULL, *new_quote = NULL, *selected, *first, *last;
    doc_node_source_range quote_range, first_range, last_range;
    uint64_t count, i, depth, old_count, new_count, whole_prefix = 0,
        whole_suffix = 0, prefix_count, suffix_count, unused,
        selected_start, selected_end, marker_count, marker_index = 0,
        at, cursor;
    int partial_prefix, partial_suffix, result;
    if (!before || !after || before->id != after->id ||
        (before->kind != XUI_DOC_LIST && before->kind != XUI_DOC_QUOTE) ||
        before->kind != after->kind || output->size || start < source_base)
        return XUI_ERROR_NOT_FOUND;
    for (depth = 0; depth < DOC_MAX_DEPTH; depth++) {
        doc_node *old_child = NULL, *new_child = NULL;
        if (old_cursor && new_cursor && old_cursor->id == new_cursor->id &&
            old_cursor->kind == XUI_DOC_QUOTE &&
            new_cursor->kind == XUI_DOC_QUOTE) {
            for (i = 0; i < doc_seq_size(new_cursor->children); i++) {
                doc_node* candidate = doc_index_get(desired->index,
                    doc_seq_get_id(new_cursor->children, i));
                if (candidate && candidate->kind == XUI_DOC_QUOTE &&
                    !doc_index_get(old->index, candidate->id)) {
                    if (old_quote) return XUI_ERROR_NOT_FOUND;
                    old_quote = old_cursor; new_quote = new_cursor;
                }
            }
            if (old_quote) break;
        }
        if (!old_cursor || !new_cursor || old_cursor->id != new_cursor->id ||
            old_cursor->kind != new_cursor->kind ||
            doc_seq_size(old_cursor->children) != doc_seq_size(new_cursor->children))
            return XUI_ERROR_NOT_FOUND;
        count = doc_seq_size(old_cursor->children);
        for (i = 0; i < count; i++) {
            uint64_t old_id = doc_seq_get_id(old_cursor->children, i);
            uint64_t new_id = doc_seq_get_id(new_cursor->children, i);
            if (doc_semantic_subtree_equal(old, old_id, desired, new_id)) continue;
            if (old_child) return XUI_ERROR_NOT_FOUND;
            old_child = doc_index_get(old->index, old_id);
            new_child = doc_index_get(desired->index, new_id);
        }
        if (!old_child || !new_child || old_child->id != new_child->id ||
            old_child->kind != new_child->kind)
            return XUI_ERROR_NOT_FOUND;
        if (old_child->kind != XUI_DOC_QUOTE &&
            old_child->kind != XUI_DOC_LIST &&
            old_child->kind != XUI_DOC_LIST_ITEM)
            return XUI_ERROR_NOT_FOUND;
        old_cursor = old_child; new_cursor = new_child;
    }
    if (!old_quote || !new_quote || old_quote->id != new_quote->id ||
        old_quote->kind != XUI_DOC_QUOTE || new_quote->kind != XUI_DOC_QUOTE)
        return XUI_ERROR_NOT_FOUND;
    old_count = doc_seq_size(old_quote->children);
    new_count = doc_seq_size(new_quote->children);
    while (whole_prefix < old_count && whole_prefix < new_count &&
        doc_semantic_subtree_equal(old,
            doc_seq_get_id(old_quote->children, whole_prefix), desired,
            doc_seq_get_id(new_quote->children, whole_prefix))) whole_prefix++;
    while (whole_suffix < old_count - whole_prefix &&
        whole_suffix < new_count - whole_prefix &&
        doc_semantic_subtree_equal(old,
            doc_seq_get_id(old_quote->children, old_count - whole_suffix - 1),
            desired,
            doc_seq_get_id(new_quote->children, new_count - whole_suffix - 1)))
        whole_suffix++;
    result = doc_md_retained_container_prefix(old, desired, old_quote,
        new_quote, whole_prefix, whole_suffix, &unused);
    if (result != XUI_OK && result != XUI_ERROR_NOT_FOUND) return result;
    partial_prefix = result == XUI_OK;
    result = doc_md_retained_container_suffix(old, desired, old_quote,
        new_quote, whole_prefix, whole_suffix, 1, &unused);
    if (result != XUI_OK && result != XUI_ERROR_NOT_FOUND) return result;
    partial_suffix = result == XUI_OK;
    /* Wrapping all visible children also has an exact prefix patch. Leading
     * and trailing hidden definitions stay in their original outer quote.
     * Empty quotes need the generic writer to generate an actual opener. */
    if (!old_count) return XUI_ERROR_NOT_FOUND;
    prefix_count = whole_prefix + (uint64_t)partial_prefix;
    suffix_count = whole_suffix + (uint64_t)partial_suffix;
    if (new_count != prefix_count + suffix_count + 1)
        return XUI_DOC_ERROR_UNREPRESENTABLE;
    selected = doc_index_get(desired->index,
        doc_seq_get_id(new_quote->children, prefix_count));
    if (!selected || selected->kind != XUI_DOC_QUOTE ||
        !doc_seq_size(selected->children)) return XUI_DOC_ERROR_UNREPRESENTABLE;
    first = doc_index_get(desired->index,
        doc_seq_get_id(selected->children, 0));
    last = doc_index_get(desired->index,
        doc_seq_get_id(selected->children, doc_seq_size(selected->children) - 1));
    if (!first || !last) return XUI_DOC_ERROR_UNREPRESENTABLE;
    if (first->kind == XUI_DOC_LIST && doc_seq_size(first->children))
        first = doc_index_get(desired->index, doc_seq_get_id(first->children, 0));
    if (last->kind == XUI_DOC_LIST && doc_seq_size(last->children))
        last = doc_index_get(desired->index,
            doc_seq_get_id(last->children, doc_seq_size(last->children) - 1));
    if (!first || !last ||
        !(first = doc_index_get(old->index, first->id)) ||
        !(last = doc_index_get(old->index, last->id)))
        return XUI_DOC_ERROR_UNREPRESENTABLE;
    doc_node_source_range_get(old, old_quote, &quote_range);
    doc_node_source_range_get(old, first, &first_range);
    doc_node_source_range_get(old, last, &last_range);
    selected_start = first_range.syntax_start;
    selected_end = last_range.syntax_end;
    if (quote_range.syntax_start == DOC_NONE ||
        quote_range.syntax_end == DOC_NONE ||
        selected_start == DOC_NONE || selected_end == DOC_NONE ||
        selected_start < quote_range.syntax_start ||
        selected_end > quote_range.syntax_end ||
        selected_start < start || selected_end > end ||
        selected_start >= selected_end ||
        (selected_start && (selected_start <= source_base ||
            (source[selected_start - 1 - source_base] != '\n' &&
                source[selected_start - 1 - source_base] != '\r'))) ||
        (selected_end != doc_seq_size(old->source) &&
            source[selected_end - 1 - source_base] != '\n' &&
            source[selected_end - 1 - source_base] != '\r') ||
        old_quote->marker_kind != XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN ||
        (old_quote->quote_prefixes &&
            old_quote->quote_prefixes->size % sizeof(uint32_t)))
        return XUI_DOC_ERROR_UNREPRESENTABLE;
    marker_count = 1 + (old_quote->quote_prefixes ?
        old_quote->quote_prefixes->size / sizeof(uint32_t) : 0);
    unsigned quote_padding;
    result = doc_md_quote_tab_padding(t, source, source_base, selected_start,
        selected_end, &quote_padding);
    if (result != XUI_OK) return result;
    cursor = start;
    for (at = selected_start; at < selected_end;) {
        uint64_t line_end = at, marker, next;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        while (line_end < selected_end && source[line_end - source_base] != '\n' &&
            source[line_end - source_base] != '\r') line_end++;
        if (line_end == selected_end) {
            if (selected_end != doc_seq_size(old->source)) return XUI_DOC_ERROR_UNREPRESENTABLE;
        } else if (source[line_end++ - source_base] == '\r' &&
            line_end < selected_end && source[line_end - source_base] == '\n') line_end++;
        while (marker_index < marker_count) {
            result = doc_md_quote_prefix_position(old_quote, &quote_range,
                marker_index, &marker);
            if (result != XUI_OK) return XUI_DOC_ERROR_UNREPRESENTABLE;
            if (marker >= at) break;
            marker_index++;
        }
        if (marker_index >= marker_count || marker >= line_end ||
            source[marker - source_base] != '>')
            return XUI_DOC_ERROR_UNREPRESENTABLE;
        if (marker_index + 1 < marker_count) {
            result = doc_md_quote_prefix_position(old_quote, &quote_range,
                marker_index + 1, &next);
            if (result != XUI_OK || next < line_end)
                return XUI_DOC_ERROR_UNREPRESENTABLE;
        }
        doc_md_copy_source(output, source + cursor - source_base, cursor, marker + 1 - cursor);
        doc_md_write(output, quote_padding ? "   >" : " >", 2 + quote_padding);
        if (output->error) return output->error;
        cursor = marker + 1; marker_index++;
        at = line_end;
    }
    doc_md_copy_source(output, source + cursor - source_base, cursor, end - cursor);
    if (output->error) return output->error;
    *raw_suffix = 1;
    return XUI_OK;
}
typedef struct doc_md_indent_boundary {
    uint64_t byte, column;
    unsigned tab_left, tab_right;
} doc_md_indent_boundary;
/* A continuation record may include indentation belonging to a child block
 * or footnote body. Locate the item's content column, splitting one Tab into
 * its left/right logical spaces when that column has no raw byte anchor. */
static int doc_md_list_indent_content_boundary(xui_document_transaction t, const doc_list_indent_relative* indent,
    uint64_t start, uint64_t end, const char* source, uint64_t source_base,
    doc_md_indent_boundary* boundary)
{
    uint64_t line_start = start, at, column = 0, target;
    memset(boundary, 0, sizeof(*boundary));
    if (start < source_base || start >= end || indent->start_column >= indent->content_column ||
        indent->content_column > indent->end_column) return XUI_DOC_ERROR_FORMAT;
    while (line_start > source_base && source[line_start - 1 - source_base] != '\n' &&
        source[line_start - 1 - source_base] != '\r') {
        if (!(line_start & 16383)) { int result = doc_txn_check(t, 0); if (result != XUI_OK) return result; }
        line_start--;
    }
    if (!line_start && end >= 3 && !memcmp(source, "\xef\xbb\xbf", 3)) line_start = 3;
    for (at = line_start; at < end; at++) {
        char c = source[at - source_base];
        if (!(at & 16383)) { int result = doc_txn_check(t, 0); if (result != XUI_OK) return result; }
        column = c == '\t' ? (column + 4) & ~(uint64_t)3 : column + 1;
    }
    /* The range includes a quote's optional space and can be shared by
     * several list layers. start_column describes that layer, not the first
     * raw byte. Body emission may also start at a dedented physical column. */
    if (doc_md_indent_end_target(column, indent->end_column, indent->content_column, &target) != XUI_OK)
        return XUI_DOC_ERROR_FORMAT;
    column = 0;
    for (at = line_start; at < start; at++) {
        char c = source[at - source_base];
        if (!(at & 16383)) { int result = doc_txn_check(t, 0); if (result != XUI_OK) return result; }
        column = c == '\t' ? (column + 4) & ~(uint64_t)3 : column + 1;
    }
    at = start;
    while (at < end && column < target) {
        char c = source[at - source_base]; uint64_t width;
        if (!(at & 16383)) { int result = doc_txn_check(t, 0); if (result != XUI_OK) return result; }
        if (c != ' ' && c != '\t') return XUI_DOC_ERROR_FORMAT;
        width = c == '\t' ? 4 - column % 4 : 1;
        if (width > target - column) {
            boundary->byte = at; boundary->column = target;
            boundary->tab_left = (unsigned)(target - column);
            boundary->tab_right = (unsigned)(width - boundary->tab_left);
            return XUI_OK;
        }
        column += width; at++;
    }
    if (column != target) return XUI_DOC_ERROR_FORMAT;
    boundary->byte = at; boundary->column = column; return XUI_OK;
}
/* Wrap complete children of one existing list item without serializing the
 * list. Continuation-indent records locate later child lines; for the first
 * child, the recorded marker width supplies a prefix for lazy lines. */
static int doc_md_list_item_quote_patch(xui_document_transaction t,
    doc_state* old, doc_state* desired, doc_node* before, doc_node* after,
    uint64_t start, uint64_t end, const char* source,
    uint64_t source_base, doc_md_output* output, int* raw_suffix)
{
    doc_node *old_cursor = before, *new_cursor = after;
    doc_node *old_item = NULL, *new_item = NULL, *quote;
    doc_node_source_range item_range, first_range, last_range;
    uint64_t depth, i, old_count, new_count, prefix = 0, suffix = 0;
    uint64_t selected_start, selected_end, cursor, at;
    uint64_t indent_count, indent_cursor = 0;
    uint64_t blank_prefix_start = DOC_NONE, blank_prefix_end = DOC_NONE;
    doc_md_indent_boundary inferred_boundary = {0};
    const char* inferred_prefix = NULL;
    uint64_t inferred_size = 0;
    unsigned quote_padding = 0;
    char marker_prefix[4096];
    int result;
    if (!before || !after || before->id != after->id ||
        (before->kind != XUI_DOC_LIST && before->kind != XUI_DOC_QUOTE) ||
        before->kind != after->kind || output->size || start < source_base)
        return XUI_ERROR_NOT_FOUND;
    for (depth = 0; depth < DOC_MAX_DEPTH; depth++) {
        doc_node *old_child = NULL, *new_child = NULL;
        if (old_cursor->kind == XUI_DOC_LIST_ITEM) {
            old_count = doc_seq_size(old_cursor->children);
            new_count = doc_seq_size(new_cursor->children);
            while (prefix < old_count && prefix < new_count &&
                doc_semantic_subtree_equal(old,
                    doc_seq_get_id(old_cursor->children, prefix), desired,
                    doc_seq_get_id(new_cursor->children, prefix))) prefix++;
            while (suffix < old_count - prefix && suffix < new_count - prefix &&
                doc_semantic_subtree_equal(old,
                    doc_seq_get_id(old_cursor->children, old_count - suffix - 1),
                    desired,
                    doc_seq_get_id(new_cursor->children, new_count - suffix - 1)))
                suffix++;
            if (old_count > prefix + suffix &&
                new_count == prefix + suffix + 1) {
                quote = doc_index_get(desired->index,
                    doc_seq_get_id(new_cursor->children, prefix));
                if (quote && quote->kind == XUI_DOC_QUOTE &&
                    !doc_index_get(old->index, quote->id)) {
                    old_item = old_cursor; new_item = new_cursor;
                    break;
                }
            }
        }
        if (old_cursor->id != new_cursor->id ||
            old_cursor->kind != new_cursor->kind ||
            doc_seq_size(old_cursor->children) !=
                doc_seq_size(new_cursor->children))
            return XUI_ERROR_NOT_FOUND;
        for (i = 0; i < doc_seq_size(old_cursor->children); i++) {
            uint64_t old_id = doc_seq_get_id(old_cursor->children, i);
            uint64_t new_id = doc_seq_get_id(new_cursor->children, i);
            if (doc_semantic_subtree_equal(old, old_id, desired, new_id))
                continue;
            if (old_child) return XUI_ERROR_NOT_FOUND;
            old_child = doc_index_get(old->index, old_id);
            new_child = doc_index_get(desired->index, new_id);
        }
        if (!old_child || !new_child || old_child->id != new_child->id ||
            old_child->kind != new_child->kind ||
            (old_child->kind != XUI_DOC_QUOTE &&
             old_child->kind != XUI_DOC_LIST &&
             old_child->kind != XUI_DOC_LIST_ITEM))
            return XUI_ERROR_NOT_FOUND;
        old_cursor = old_child; new_cursor = new_child;
    }
    if (!old_item || !new_item || old_item->id != new_item->id)
        return XUI_ERROR_NOT_FOUND;
    quote = doc_index_get(desired->index,
        doc_seq_get_id(new_item->children, prefix));
    if (!quote || doc_seq_size(quote->children) !=
        old_count - prefix - suffix)
        return XUI_DOC_ERROR_UNREPRESENTABLE;
    for (i = 0; i < doc_seq_size(quote->children); i++) {
        uint64_t old_id = doc_seq_get_id(old_item->children, prefix + i);
        uint64_t new_id = doc_seq_get_id(quote->children, i);
        if (old_id != new_id ||
            !doc_semantic_subtree_equal(old, old_id, desired, new_id))
            return XUI_DOC_ERROR_UNREPRESENTABLE;
    }
    doc_node_source_range_get(old, old_item, &item_range);
    doc_node_source_range_get(old, doc_index_get(old->index,
        doc_seq_get_id(old_item->children, prefix)), &first_range);
    doc_node_source_range_get(old, doc_index_get(old->index,
        doc_seq_get_id(old_item->children, old_count - suffix - 1)),
        &last_range);
    selected_start = first_range.syntax_start;
    selected_end = last_range.syntax_end;
    if (item_range.syntax_start == DOC_NONE ||
        item_range.syntax_end == DOC_NONE ||
        selected_start == DOC_NONE || selected_end == DOC_NONE ||
        selected_start < item_range.syntax_start ||
        selected_end > item_range.syntax_end ||
        selected_start < start || selected_end > end ||
        selected_start >= selected_end ||
        (selected_start && (selected_start <= source_base ||
            (source[selected_start - 1 - source_base] != '\n' &&
                source[selected_start - 1 - source_base] != '\r'))) ||
        (old_item->list_indents &&
            old_item->list_indents->size % sizeof(doc_list_indent_relative)))
        return XUI_DOC_ERROR_UNREPRESENTABLE;
    indent_count = old_item->list_indents ?
        old_item->list_indents->size / sizeof(doc_list_indent_relative) : 0;
    if (old_item->list_indents) {
        uint64_t previous_end = item_range.syntax_start;
        for (i = 0; i < indent_count; i++) {
            doc_list_indent_relative indent;
            doc_md_indent_boundary boundary;
            uint64_t indent_start, indent_end, line_start, scan;
            memcpy(&indent, old_item->list_indents->data +
                i * sizeof(indent), sizeof(indent));
            indent_start = item_range.syntax_start + indent.start;
            indent_end = item_range.syntax_start + indent.end;
            if (indent.start >= indent.end ||
                indent_end > item_range.syntax_end ||
                indent_start < previous_end)
                return XUI_DOC_ERROR_UNREPRESENTABLE;
            previous_end = indent_end;
            if (indent_start < selected_start || indent_end > selected_end)
                continue;
            line_start = indent_start;
            while (line_start > selected_start &&
                source[line_start - 1 - source_base] != '\n' &&
                source[line_start - 1 - source_base] != '\r')
                line_start--;
            if (line_start < selected_start || line_start >= indent_end)
                continue;
            result = doc_md_list_indent_content_boundary(t, &indent, indent_start,
                indent_end, source, source_base, &boundary);
            if (result != XUI_OK) return result;
            if (blank_prefix_start == DOC_NONE) {
                inferred_boundary = boundary;
                blank_prefix_end = boundary.byte;
                blank_prefix_start = line_start;
            }
            /* Keep later Tabs verbatim, including indented-code bytes in a
             * hidden footnote. Two legal spaces before every new quote make
             * its physical shift four columns, preserving all Tab stops and
             * the common definition-body origin across selected lines. */
            for (scan = boundary.byte + !!boundary.tab_right; scan < indent_end; scan++) {
                if (!(scan & 16383)) { result = doc_txn_check(t, 0); if (result != XUI_OK) return result; }
                if (source[scan - source_base] == '\t') { quote_padding = 2; break; }
            }
        }
    }
    if (blank_prefix_start != DOC_NONE) {
        inferred_prefix = source + blank_prefix_start - source_base;
        inferred_size = blank_prefix_end - blank_prefix_start;
        if (inferred_boundary.tab_left) {
            if (inferred_size > sizeof(marker_prefix) - inferred_boundary.tab_left)
                return XUI_DOC_ERROR_UNREPRESENTABLE;
            memcpy(marker_prefix, inferred_prefix, (size_t)inferred_size);
            memset(marker_prefix + inferred_size, ' ', inferred_boundary.tab_left);
            inferred_size += inferred_boundary.tab_left;
            inferred_prefix = marker_prefix;
        }
    } else if (selected_start == item_range.syntax_start &&
        old_item->marker_kind == XUI_DOC_BLOCK_SYNTAX_LIST_ITEM) {
        uint64_t marker = selected_start + old_item->marker_primary_start;
        uint64_t continuation = selected_start +
            (old_item->marker_secondary_start ?
                old_item->marker_secondary_start : old_item->marker_tail_end);
        uint64_t column = 0, marker_column = 0, position, width;
        if (marker < selected_start || continuation <= marker ||
            continuation > selected_end || continuation > end)
            return XUI_DOC_ERROR_UNREPRESENTABLE;
        for (position = selected_start; position < continuation; position++) {
            char c = source[position - source_base];
            if (position == marker) marker_column = column;
            if (c == '\n' || c == '\r')
                return XUI_DOC_ERROR_UNREPRESENTABLE;
            column = c == '\t' ? (column + 4) & ~(uint64_t)3 : column + 1;
        }
        width = column - marker_column;
        inferred_size = marker - selected_start + width;
        if (!width || inferred_size > sizeof(marker_prefix))
            return XUI_DOC_ERROR_UNREPRESENTABLE;
        memcpy(marker_prefix, source + selected_start - source_base,
            (size_t)(marker - selected_start));
        memset(marker_prefix + marker - selected_start, ' ', (size_t)width);
        inferred_prefix = marker_prefix;
    }
    cursor = start;
    for (at = selected_start; at < selected_end;) {
        uint64_t line_end = at, insert = DOC_NONE, next;
        doc_md_indent_boundary boundary = {0};
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        while (line_end < selected_end &&
            source[line_end - source_base] != '\n' &&
            source[line_end - source_base] != '\r') line_end++;
        next = line_end;
        if (next < selected_end && source[next++ - source_base] == '\r' &&
            next < selected_end && source[next - source_base] == '\n')
            next++;
        if (at == item_range.syntax_start &&
            old_item->marker_kind == XUI_DOC_BLOCK_SYNTAX_LIST_ITEM &&
            old_item->marker_tail_end > old_item->marker_primary_end &&
            old_item->marker_tail_end <= line_end - item_range.syntax_start)
            insert = item_range.syntax_start + old_item->marker_tail_end;
        else if (old_item->list_indents) {
            while (indent_cursor < indent_count) {
                doc_list_indent_relative indent;
                uint64_t indent_start, indent_end;
                memcpy(&indent, old_item->list_indents->data +
                    indent_cursor * sizeof(indent), sizeof(indent));
                indent_start = item_range.syntax_start + indent.start;
                indent_end = item_range.syntax_start + indent.end;
                if (indent_end <= at) { indent_cursor++; continue; }
                if (indent_start >= next) break;
                if (indent_start < at || indent_end > line_end ||
                    insert != DOC_NONE)
                    return XUI_DOC_ERROR_UNREPRESENTABLE;
                result = doc_md_list_indent_content_boundary(t, &indent, indent_start,
                    indent_end, source, source_base, &boundary);
                if (result != XUI_OK) return result;
                insert = boundary.byte;
                indent_cursor++;
            }
        }
        if (insert == DOC_NONE && inferred_prefix) {
            uint64_t existing = 0, line_bytes = line_end - at;
            uint64_t prefix_bytes = inferred_size;
            while (existing < line_bytes && existing < prefix_bytes &&
                source[at + existing - source_base] ==
                    inferred_prefix[existing])
                existing++;
            if (existing == line_bytes) {
                doc_md_copy_source(output, source + cursor - source_base,
                    cursor, line_end - cursor);
                doc_md_write(output, inferred_prefix + existing,
                    prefix_bytes - existing);
                doc_md_write(output, "  ", quote_padding);
                doc_md_write(output, ">", 1);
                if (output->error) return output->error;
                cursor = line_end;
                at = next;
                continue;
            }
            if (existing < prefix_bytes &&
                (source[at + existing - source_base] == ' ' ||
                 source[at + existing - source_base] == '\t' ||
                 source[at + existing - source_base] == '>'))
                return XUI_DOC_ERROR_UNREPRESENTABLE;
            doc_md_copy_source(output, source + cursor - source_base,
                cursor, at + existing - cursor);
            doc_md_write(output, inferred_prefix + existing,
                prefix_bytes - existing);
            doc_md_write(output, "  ", quote_padding);
            doc_md_write(output, "> ", 2);
            if (output->error) return output->error;
            cursor = at + existing;
            at = next;
            continue;
        }
        if (insert == DOC_NONE || insert < cursor || insert > line_end)
            return XUI_DOC_ERROR_UNREPRESENTABLE;
        doc_md_copy_source(output, source + cursor - source_base, cursor, insert - cursor);
        if (boundary.tab_left) doc_md_write(output, "   ", boundary.tab_left);
        doc_md_write(output, "  ", quote_padding);
        doc_md_write(output, "> ", 2);
        cursor = insert;
        if (boundary.tab_right) {
            doc_md_write(output, "   ", boundary.tab_right);
            cursor++;
        }
        if (output->error) return output->error;
        at = next;
    }
    doc_md_copy_source(output, source + cursor - source_base, cursor, end - cursor);
    if (output->error) return output->error;
    *raw_suffix = 1;
    return XUI_OK;
}
int doc_markdown_apply_tree(xui_document_transaction t, doc_state* desired, const xui_doc_position_t* target, xui_doc_position_t* caret, unsigned flags)
{
    doc_state* old = t->draft; doc_node *before, *after;
    doc_state* reference_state = NULL;
    doc_md_output output = {0}; doc_md_source_plan source_plan = {0}; char* source = NULL;
    uint64_t prefix = 0, suffix = 0, ac, bc, i, start, end, source_base = 0, unused;
    unsigned char* referenced_items = NULL;
    doc_sequence *saved_references = NULL, *saved_source = NULL;
    doc_sequence *protected_references = NULL, *protected_source = NULL;
    uint64_t editable_start = DOC_NONE, editable_end = DOC_NONE;
    int result = XUI_OK, preserve_quote_references = 0, preserve_list_references = 0;
    int list_tail_raw_reference = 0;
    int quote_prefix_reference_patch = 0;
    int quote_prefix_prepared = 0;
    int quote_rewrite, list_rewrite, immediate_single_block;
    int retain_container_prefix = 0, retain_container_suffix = 0;
    int nested_quote_raw_suffix = 0;
    uint64_t container_prefix_cut = 0, container_suffix_cut = 0;
    before = doc_index_get(old->index, DOC_ROOT); after = doc_index_get(desired->index, DOC_ROOT);
    ac = doc_seq_size(before->children); bc = doc_seq_size(after->children);
    if (flags & DOC_MD_ACCEPT_DERIVED_FOOTNOTES) {
        protected_references = old->references; protected_source = old->source;
        doc_seq_retain(protected_references); doc_seq_retain(protected_source);
    }
    while (prefix < ac && prefix < bc && doc_semantic_subtree_equal(old, doc_seq_get_id(before->children, prefix), desired, doc_seq_get_id(after->children, prefix))) prefix++;
    while (suffix < ac - prefix && suffix < bc - prefix && doc_semantic_subtree_equal(old, doc_seq_get_id(before->children, ac - suffix - 1), desired, doc_seq_get_id(after->children, bc - suffix - 1))) suffix++;
    if (prefix == ac && prefix == bc) { if (caret && target) *caret = doc_markdown_resolve_caret(t, desired, *target); goto done; }
    /* Only a directly edited definition may change its body. All other
     * definitions, including duplicates and newly hidden ones, stay raw. */
    if ((flags & DOC_MD_ACCEPT_DERIVED_FOOTNOTES) &&
        ac - prefix - suffix == 1 && bc - prefix - suffix == 1) {
        doc_node* old_block = doc_index_get(old->index, doc_seq_get_id(before->children, prefix));
        doc_node* new_block = doc_index_get(desired->index, doc_seq_get_id(after->children, prefix));
        if (old_block->kind == XUI_DOC_FOOTNOTE && new_block->kind == XUI_DOC_FOOTNOTE &&
            old_block->id == new_block->id &&
            !strcmp(doc_string(old_block->info), doc_string(new_block->info))) {
            doc_node_source_range range; doc_node_source_range_get(old, old_block, &range);
            editable_start = range.syntax_start; editable_end = range.syntax_end;
        }
    }
    result = doc_md_retained_container_prefix(old, desired, before, after,
        prefix, suffix, &container_prefix_cut);
    if (result != XUI_OK && result != XUI_ERROR_NOT_FOUND) goto done;
    retain_container_prefix = result == XUI_OK;
    result = XUI_OK;
    result = doc_md_retained_container_suffix(old, desired, before, after,
        prefix, suffix, retain_container_prefix && ac - prefix - suffix == 1,
        &container_suffix_cut);
    if (result != XUI_OK && result != XUI_ERROR_NOT_FOUND) goto done;
    retain_container_suffix = result == XUI_OK;
    result = XUI_OK;
    if (retain_container_prefix && retain_container_suffix &&
        container_prefix_cut >= container_suffix_cut) {
        result = XUI_DOC_ERROR_UNREPRESENTABLE; goto done;
    }
    quote_rewrite = list_rewrite = 0;
    if (ac - prefix - suffix == 1 && bc - prefix - suffix == 1) {
        doc_node* old_block = doc_index_get(old->index, doc_seq_get_id(before->children, prefix));
        doc_node* new_block = doc_index_get(desired->index, doc_seq_get_id(after->children, prefix));
        quote_rewrite = old_block->kind == XUI_DOC_QUOTE && new_block->kind == XUI_DOC_QUOTE;
        list_rewrite = old_block->kind == XUI_DOC_LIST && new_block->kind == XUI_DOC_LIST &&
            doc_seq_size(old_block->children) &&
            doc_seq_size(old_block->children) == doc_seq_size(new_block->children);
    }
    /* Definitions inside one changed quote or one list item can be moved as
     * exact trivia gaps. Other containers still reject rather than dropping them. */
    for (i = prefix; i < ac - suffix; i++) {
        doc_node* n = doc_index_get(old->index, doc_seq_get_id(before->children, i)); uint64_t j;
        doc_node_source_range range; doc_node_source_range_get(old, n, &range);
        for (j = 0; j < doc_seq_size(old->references); j += sizeof(xui_doc_reference_definition_t)) {
            xui_doc_reference_definition_t reference; doc_seq_read(old->references, j, &reference, sizeof(reference));
            if (range.syntax_start < reference.iSourceEnd && range.syntax_end > reference.iSourceStart) {
                if (retain_container_prefix && i == prefix &&
                    reference.iSourceEnd <= container_prefix_cut) continue;
                if (retain_container_suffix && i == ac - suffix - 1 &&
                    reference.iSourceStart >= container_suffix_cut) continue;
                /* The footnote block itself owns its definition. Rewriting its
                 * body necessarily replaces that same source span; the parser
                 * rebuilds the definition from the new block. */
                if (n->kind == XUI_DOC_FOOTNOTE &&
                    reference.iKind == XUI_DOC_REFERENCE_FOOTNOTE &&
                    reference.iSourceStart == range.syntax_start &&
                    reference.iSourceEnd == range.syntax_end) continue;
                if (quote_rewrite) preserve_quote_references = 1;
                else if (list_rewrite) preserve_list_references = 1;
                else { result = XUI_DOC_ERROR_UNREPRESENTABLE; goto done; }
            }
        }
    }
    if (preserve_quote_references || preserve_list_references) {
        result = doc_md_nested_paragraph_patch(t, desired);
        if (result == XUI_OK) {
            if (target && caret) *caret = doc_markdown_resolve_caret(t, desired, *target);
            goto done;
        }
        if (result != XUI_ERROR_NOT_FOUND) goto done;
        result = XUI_OK;
    }
    start = end = doc_seq_size(old->source);
    if (ac > prefix + suffix) {
        result = doc_md_bounds(old, doc_seq_get_id(before->children, prefix), &start, &end); if (result != XUI_OK) goto done;
        if (retain_container_prefix) start = container_prefix_cut;
        if (retain_container_suffix && prefix == ac - suffix - 1) end = container_suffix_cut;
    } else if (prefix < ac) {
        result = doc_md_bounds(old, doc_seq_get_id(before->children, prefix), &start, &unused); if (result != XUI_OK) goto done; end = start;
    }
    if (preserve_quote_references || preserve_list_references) {
        source = doc_seq_string(t->document->allocator, old->source);
        if (!source) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    } else {
        uint64_t bytes;
        /* Only the changed block and up to three preceding bytes are read:
         * CRLF plus the byte proving whether its preceding line was blank. */
        source_base = start > 3 ? start - 3 : 0;
        bytes = end - source_base;
        if (bytes >= SIZE_MAX) { result = XUI_DOC_ERROR_LIMIT; goto done; }
        source = doc_alloc(t->document->allocator, (size_t)bytes + 1);
        if (!source) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
        result = doc_seq_read(old->source, source_base, source, bytes);
        if (result != XUI_OK) goto done;
    }
    output.allocator = t->document->allocator;
    output.plan = &source_plan;
    if (retain_container_prefix) {
        const char* ending = "\n"; uint64_t ending_bytes = 1;
        if (source[start - 1 - source_base] == '\n' && start >= 2 &&
            source[start - 2 - source_base] == '\r') {
            ending = "\r\n"; ending_bytes = 2;
        } else if (source[start - 1 - source_base] == '\r') ending = "\r";
        doc_md_write(&output, ending, ending_bytes);
    } else if (start && ac == prefix + suffix && bc > prefix + suffix) {
        /* Inserting a root block must leave a blank line after its previous
         * neighbor. One terminal LF (or CRLF) only ends that neighbor's line;
         * without another ending two paragraphs would parse as one block. */
        uint64_t before_eol = start;
        const char* ending = "\n"; uint64_t ending_bytes = 1;
        if (source[start - 1 - source_base] == '\n') {
            before_eol--;
            if (before_eol && source[before_eol - 1 - source_base] == '\r') {
                before_eol--; ending = "\r\n"; ending_bytes = 2;
            }
        } else if (source[start - 1 - source_base] == '\r') {
            before_eol--; ending = "\r";
        }
        if (before_eol == start) {
            doc_md_write(&output, ending, ending_bytes);
            doc_md_write(&output, ending, ending_bytes);
        } else if (!before_eol || (source[before_eol - 1 - source_base] != '\n' &&
                source[before_eol - 1 - source_base] != '\r'))
            doc_md_write(&output, ending, ending_bytes);
    } else if (start && source[start - 1 - source_base] != '\n' &&
        source[start - 1 - source_base] != '\r')
        doc_md_write(&output, "\n\n", 2);
    if (preserve_quote_references && quote_rewrite &&
        ac - prefix - suffix == 1 && bc - prefix - suffix == 1 &&
        !output.size) {
        result = doc_md_nested_quote_prefix_patch(t, old, desired,
            doc_index_get(old->index, doc_seq_get_id(before->children, prefix)),
            doc_index_get(desired->index, doc_seq_get_id(after->children, prefix)),
            start, end, source, source_base, &output, &nested_quote_raw_suffix);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_md_list_item_quote_patch(t, old, desired,
                doc_index_get(old->index, doc_seq_get_id(before->children, prefix)),
                doc_index_get(desired->index, doc_seq_get_id(after->children, prefix)),
                start, end, source, source_base, &output,
                &nested_quote_raw_suffix);
        if (result == XUI_OK)
            quote_prefix_prepared = quote_prefix_reference_patch = 1;
        else if (result != XUI_ERROR_NOT_FOUND) goto done;
    }
    if (preserve_quote_references && !quote_prefix_prepared) {
        result = doc_md_container_reference_prelude(old,
            doc_index_get(old->index, doc_seq_get_id(before->children, prefix)),
            start, end, source, &output, NULL);
        if (result != XUI_OK) goto done;
    } else if (preserve_list_references) {
        doc_node* list = doc_index_get(old->index, doc_seq_get_id(before->children, prefix));
        uint64_t j, item_count = doc_seq_size(list->children);
        if ((uint64_t)(size_t)item_count != item_count) {
            result = XUI_DOC_ERROR_LIMIT; goto done;
        }
        referenced_items = doc_alloc(t->document->allocator, (size_t)item_count);
        if (!referenced_items) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
        memset(referenced_items, 0, (size_t)item_count);
        for (j = 0; j < doc_seq_size(old->references); j += sizeof(xui_doc_reference_definition_t)) {
            xui_doc_reference_definition_t reference; uint64_t item;
            doc_seq_read(old->references, j, &reference, sizeof(reference));
            if (reference.iSourceStart >= end || reference.iSourceEnd <= start) continue;
            if ((reference.iKind != XUI_DOC_REFERENCE_LINK && reference.iKind != XUI_DOC_REFERENCE_FOOTNOTE) ||
                doc_md_reference_list_item(old, list, &reference, &item) != XUI_OK) {
                result = XUI_DOC_ERROR_UNREPRESENTABLE; goto done;
            }
            referenced_items[item] = 1;
        }
    }
    if (preserve_quote_references || preserve_list_references) {
        /* The transaction draft's source field changes in place before its
         * parse. Keep an independent state wrapper over the shared old roots. */
        reference_state = doc_state_clone(old);
        if (!reference_state) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
        saved_references = old->references; saved_source = old->source;
        doc_seq_retain(saved_references); doc_seq_retain(saved_source);
    }
    /* A single replacement without hidden definitions can parse immediately.
     * This preserves the old SourceStore for a validated independent-block
     * reparse; multi-block rewrites still parse after all source patches. */
    immediate_single_block = !preserve_quote_references && !preserve_list_references &&
        ac - prefix - suffix == 1 && bc - prefix - suffix == 1 &&
        t->count == t->parse_op_start;
    for (i = prefix + (uint64_t)retain_container_prefix;
        i < bc - suffix - (uint64_t)retain_container_suffix; i++) {
        if (quote_prefix_prepared) continue;
        output.list_marker = doc_md_list_marker(desired, after, i);
        if (preserve_list_references) {
            result = XUI_ERROR_NOT_FOUND;
            if (list_rewrite && i == prefix && ac - prefix - suffix == 1 &&
                bc - prefix - suffix == 1)
                result = doc_md_nested_quote_prefix_patch(t, old, desired,
                    doc_index_get(old->index, doc_seq_get_id(before->children, i)),
                    doc_index_get(desired->index, doc_seq_get_id(after->children, i)),
                    start, end, source, source_base, &output,
                    &nested_quote_raw_suffix);
            if (result == XUI_ERROR_NOT_FOUND && list_rewrite &&
                i == prefix && ac - prefix - suffix == 1 &&
                bc - prefix - suffix == 1)
                result = doc_md_list_item_quote_patch(t, old, desired,
                    doc_index_get(old->index, doc_seq_get_id(before->children, i)),
                    doc_index_get(desired->index, doc_seq_get_id(after->children, i)),
                    start, end, source, source_base, &output,
                    &nested_quote_raw_suffix);
            if (result == XUI_OK) quote_prefix_reference_patch = 1;
            if (result == XUI_ERROR_NOT_FOUND)
                result = doc_md_list_with_reference(old, desired,
                    doc_index_get(old->index, doc_seq_get_id(before->children, i)),
                    doc_index_get(desired->index, doc_seq_get_id(after->children, i)),
                    referenced_items, source, &output, &list_tail_raw_reference);
        } else {
            result = XUI_ERROR_NOT_FOUND;
            if (quote_rewrite && i == prefix && ac - prefix - suffix == 1 &&
                bc - prefix - suffix == 1)
                result = doc_md_nested_quote_retained_edges(old, desired,
                    doc_index_get(old->index, doc_seq_get_id(before->children, i)),
                    doc_index_get(desired->index, doc_seq_get_id(after->children, i)),
                    start, end, source, source_base, &output,
                    &nested_quote_raw_suffix);
            if (result == XUI_ERROR_NOT_FOUND &&
                (quote_rewrite || list_rewrite) && i == prefix &&
                ac - prefix - suffix == 1 && bc - prefix - suffix == 1)
                result = doc_md_nested_quote_prefix_patch(t, old, desired,
                    doc_index_get(old->index, doc_seq_get_id(before->children, i)),
                    doc_index_get(desired->index, doc_seq_get_id(after->children, i)),
                    start, end, source, source_base, &output,
                    &nested_quote_raw_suffix);
            if (result == XUI_ERROR_NOT_FOUND &&
                (quote_rewrite || list_rewrite) && i == prefix &&
                ac - prefix - suffix == 1 && bc - prefix - suffix == 1)
                result = doc_md_list_item_quote_patch(t, old, desired,
                    doc_index_get(old->index, doc_seq_get_id(before->children, i)),
                    doc_index_get(desired->index, doc_seq_get_id(after->children, i)),
                    start, end, source, source_base, &output,
                    &nested_quote_raw_suffix);
            if (result == XUI_ERROR_NOT_FOUND)
                result = doc_md_block_write(desired,
                    doc_index_get(desired->index, doc_seq_get_id(after->children, i)),
                    &output);
        }
        if (result != XUI_OK) goto done;
    }
    /* A one-block rewrite must keep that block's original terminal line
     * endings. The writer's canonical blank separator is only needed when
     * joining newly generated blocks; otherwise it would add a blank line
     * after a visual style command on a single Markdown paragraph. */
    if (!list_tail_raw_reference && !nested_quote_raw_suffix &&
        ac - prefix - suffix == 1 &&
        bc - prefix - suffix == 1 && output.size >= 2 &&
        output.data[output.size - 1] == '\n' && output.data[output.size - 2] == '\n') {
        uint64_t eol = end;
        while (eol > start && (source[eol - 1 - source_base] == '\n' ||
            source[eol - 1 - source_base] == '\r')) eol--;
        output.size -= 2; output.data[output.size] = 0;
        doc_md_write(&output, source + eol - source_base, end - eol);
        if (output.error) { result = output.error; goto done; }
    }
    /* Delete changed blocks backwards, retaining all inter-block trivia. */
    for (i = ac - suffix; i > prefix + 1; i--) {
        uint64_t from, to;
        result = doc_md_bounds(old, doc_seq_get_id(before->children, i - 1), &from, &to); if (result != XUI_OK) goto done;
        if (retain_container_suffix && i == ac - suffix) to = container_suffix_cut;
        if (from < end) { result = XUI_DOC_ERROR_UNREPRESENTABLE; goto done; }
        result = doc_txn_source_patch(t, from, to, "", 0, 0); if (result != XUI_OK) goto done;
    }
    result = doc_md_apply_source_plan(t, start, end, &output, immediate_single_block);
    if (result == XUI_OK && t->count > t->parse_op_start) result = doc_markdown_parse(t);
    /* A locally parsed block can be structurally valid yet differ from the
     * requested semantic edit (for example an inline image rewrite). The
     * desired tree is authoritative: retry a full parse before rejecting the
     * edit, keeping the fast path only when the complete semantics agree. */
    if (result == XUI_OK && immediate_single_block &&
        !doc_semantic_equal(desired, t->draft)) result = doc_markdown_parse(t);
    if (result == XUI_OK && (preserve_quote_references || preserve_list_references)) {
        int equal;
        if (quote_prefix_reference_patch)
            result = doc_md_quote_prefix_references(t, reference_state, t->draft, &equal);
        else equal = doc_md_reference_spelling_equal(saved_references, saved_source,
            t->draft->references, t->draft->source);
        if (result == XUI_OK && !equal) result = XUI_DOC_ERROR_UNREPRESENTABLE;
    }
    if (result == XUI_OK && !doc_semantic_equal(desired, t->draft)) {
        /* Footer membership/order is derived from references. Accept it only
         * after proving ordinary semantics and every protected definition. */
        if (!(flags & DOC_MD_ACCEPT_DERIVED_FOOTNOTES) ||
            !doc_semantic_markdown_projection_equal(desired, t->draft) ||
            !doc_md_reference_spelling_equal_except_footnote(protected_references,
                protected_source, t->draft->references, t->draft->source,
                editable_start, editable_end)) result = XUI_DOC_ERROR_UNREPRESENTABLE;
    }
    if (result == XUI_OK && target && caret) *caret = doc_markdown_resolve_caret(t, desired, *target);
done:
    doc_seq_release(saved_references); doc_seq_release(saved_source);
    doc_seq_release(protected_references); doc_seq_release(protected_source);
    doc_state_release(reference_state);
    doc_free(referenced_items); doc_free(source); doc_free(output.data); doc_free(source_plan.copies);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
static int doc_md_source_gap_trivia(xui_document_transaction t,
    doc_state* state, uint64_t start, uint64_t end, int quote_markers)
{
    char buffer[4096];
    while (start < end) {
        uint64_t bytes = end - start < sizeof(buffer) ? end - start : sizeof(buffer), i;
        int result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(state->source, start, buffer, bytes);
        if (result != XUI_OK) return result;
        for (i = 0; i < bytes; i++)
            if (buffer[i] != ' ' && buffer[i] != '\t' &&
                buffer[i] != '\r' && buffer[i] != '\n' &&
                (!quote_markers || buffer[i] != '>')) return XUI_ERROR_NOT_FOUND;
        start += bytes;
    }
    return XUI_OK;
}
/* A definition between two visible blocks has no semantic child. When those
 * blocks merge, move the complete gap after the surviving block so its raw
 * definition and whitespace remain intact. Any unrecognized source byte
 * leaves this path; the private parse below still proves the final result. */
static int doc_md_source_gap_references(xui_document_transaction t,
    doc_state* state, uint64_t start, uint64_t end,
    int quote_markers, uint64_t editable_footnote_start,
    uint64_t editable_footnote_end, doc_md_output* moved)
{
    uint64_t at = start, i; int found = 0, result;
    for (i = 0; i < doc_seq_size(state->references);
        i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(state->references, i, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if (ref.iSourceEnd <= start || ref.iSourceStart >= end) continue;
        if (ref.iKind == XUI_DOC_REFERENCE_FOOTNOTE &&
            ref.iSourceStart == editable_footnote_start &&
            ref.iSourceEnd == editable_footnote_end) continue;
        if ((ref.iKind != XUI_DOC_REFERENCE_LINK &&
                ref.iKind != XUI_DOC_REFERENCE_FOOTNOTE) ||
            ref.iSourceStart < at || ref.iSourceEnd <= ref.iSourceStart ||
            ref.iSourceEnd > end)
            return XUI_ERROR_NOT_FOUND;
        result = doc_md_source_gap_trivia(t, state, at,
            ref.iSourceStart, quote_markers);
        if (result != XUI_OK) return result;
        found = 1; at = ref.iSourceEnd;
    }
    result = doc_md_source_gap_trivia(t, state, at, end, quote_markers);
    if (result != XUI_OK) return result;
    if (!found) return XUI_ERROR_NOT_FOUND;
    while (start < end) {
        char buffer[4096]; uint64_t bytes = end - start < sizeof(buffer) ? end - start : sizeof(buffer);
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(state->source, start, buffer, bytes);
        if (result != XUI_OK) return result;
        doc_md_write(moved, buffer, bytes);
        if (moved->error) return moved->error;
        start += bytes;
    }
    return XUI_OK;
}
static int doc_md_removable_item_lead_kind(uint32_t kind)
{
    return kind == XUI_DOC_PARAGRAPH || kind == XUI_DOC_CODE_BLOCK ||
        kind == XUI_DOC_QUOTE || kind == XUI_DOC_HEADING ||
        kind == XUI_DOC_RULE || kind == XUI_DOC_HTML ||
        kind == XUI_DOC_TABLE || kind == XUI_DOC_DIAGRAM;
}
static int doc_md_nested_path_item(doc_state* old, doc_state* desired,
    doc_node* item, uint64_t child_id)
{
    doc_node* remaining = doc_index_get(desired->index, item->id);
    uint64_t count = doc_seq_size(item->children);
    if (!remaining || doc_seq_size(remaining->children) != 1 ||
        doc_seq_get_id(remaining->children, 0) != child_id)
        return 0;
    if (count == 1) return doc_seq_get_id(item->children, 0) == child_id;
    if (count >= 2 && doc_seq_get_id(item->children, count - 1) == child_id) {
        doc_node* before = doc_index_get(old->index,
            doc_seq_get_id(item->children, 0));
        return before && doc_md_removable_item_lead_kind(before->kind);
    }
    return 0;
}
static int doc_md_nested_path_list(doc_state* desired, doc_node* list,
    uint64_t item_id)
{
    doc_node* remaining = doc_index_get(desired->index, list->id);
    uint64_t old_index = doc_child_index(list, item_id), new_index;
    uint64_t old_count = doc_seq_size(list->children), new_count, i, j = 0;
    if (!remaining || old_index == DOC_NONE) return 0;
    new_index = doc_child_index(remaining, item_id);
    if (new_index == DOC_NONE) return 0;
    new_count = doc_seq_size(remaining->children);
    if (new_count - new_index != old_count - old_index) return 0;
    for (i = old_index; i < old_count; i++)
        if (doc_seq_get_id(list->children, i) !=
            doc_seq_get_id(remaining->children, new_index + i - old_index))
            return 0;
    for (i = 0; i < old_index && j < new_index; i++)
        if (doc_seq_get_id(list->children, i) ==
            doc_seq_get_id(remaining->children, j)) j++;
    return j == new_index;
}
static uint64_t doc_md_nested_path_child(doc_state* old,
    uint64_t ancestor_id, uint64_t leaf_id)
{
    doc_node* node = doc_index_get(old->index, leaf_id);
    while (node && node->id != ancestor_id) {
        if (node->parent == ancestor_id) return node->id;
        node = doc_index_get(old->index, node->parent);
    }
    return DOC_NONE;
}
/* Read the complete parser-owned prefix. Deep list indentation must not
 * truncate a marker; allocation and cancellation remain transaction errors. */
static int doc_md_write_source_range(xui_document_transaction t,
    doc_state* old, uint64_t start, uint64_t end, doc_md_output* output);
static int doc_md_removed_item_paragraph_body(xui_document_transaction t,
    doc_state* old, doc_node* item, doc_node* paragraph,
    uint64_t* body_start, uint64_t* body_end)
{
    doc_node_source_range range;
    doc_md_output storage = {0};
    const char* prefix; char ending;
    uint64_t bytes, marker_start = 0, continuation, marker_end, ending_bytes = 1;
    int result;
    doc_node_source_range_get(old, paragraph, &range);
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
        range.source_start == DOC_NONE || range.source_end == DOC_NONE ||
        range.syntax_start >= range.source_start ||
        range.source_start > range.source_end ||
        range.source_end >= range.syntax_end ||
        range.syntax_end > doc_seq_size(old->source))
        return XUI_ERROR_NOT_FOUND;
    storage.allocator = old->allocator;
    result = doc_md_write_source_range(t, old, range.syntax_start,
        range.source_start, &storage);
    if (result != XUI_OK) goto done;
    prefix = storage.data; bytes = storage.size;
    while (marker_start < bytes &&
        (prefix[marker_start] == ' ' || prefix[marker_start] == '\t'))
        marker_start++;
    marker_end = doc_md_list_marker_prefix(prefix, bytes, marker_start,
        !!(item->attrs->iFlags & XUI_DOC_TASK), &continuation);
    if (marker_end == marker_start) { result = XUI_ERROR_NOT_FOUND; goto done; }
    result = doc_seq_read(old->source, range.syntax_end - 1, &ending, 1);
    if (result != XUI_OK) goto done;
    if (ending == '\n' && range.syntax_end > 1) {
        char before;
        result = doc_seq_read(old->source, range.syntax_end - 2, &before, 1);
        if (result != XUI_OK) goto done;
        if (before == '\r') ending_bytes = 2;
    } else if (ending != '\r') { result = XUI_ERROR_NOT_FOUND; goto done; }
    *body_start = range.syntax_start + marker_end;
    *body_end = range.syntax_end - ending_bytes;
    result = *body_start > range.source_start || *body_end < range.source_end ||
        *body_start > *body_end ? XUI_ERROR_NOT_FOUND : XUI_OK;
done:
    doc_free(storage.data);
    return result;
}
/* Removing all selected visible blocks leaves a definition-only list item.
 * Preserve the first paragraph's marker and line ending, then resume at the
 * first trailing definition. Do not discard any intervening source-only
 * reference. The full candidate parse proves the new structure. */
static int doc_md_definition_item_trim(xui_document_transaction t,
    doc_state* old, doc_node* item, uint64_t* body_start,
    uint64_t* body_end, uint64_t* paragraph_end, uint64_t* definition_start)
{
    doc_node *paragraph, *last;
    doc_node_source_range item_range, paragraph_range, last_range;
    uint64_t i, children = doc_seq_size(item->children);
    uint64_t references = doc_seq_size(old->references);
    char gap_end; int result;
    if (!children) return XUI_ERROR_NOT_FOUND;
    paragraph = doc_index_get(old->index,
        doc_seq_get_id(item->children, 0));
    last = doc_index_get(old->index,
        doc_seq_get_id(item->children, children - 1));
    if (!paragraph || paragraph->kind != XUI_DOC_PARAGRAPH || !last)
        return XUI_ERROR_NOT_FOUND;
    doc_node_source_range_get(old, item, &item_range);
    doc_node_source_range_get(old, paragraph, &paragraph_range);
    doc_node_source_range_get(old, last, &last_range);
    if (item_range.syntax_end == DOC_NONE ||
        paragraph_range.syntax_end == DOC_NONE ||
        last_range.syntax_end == DOC_NONE ||
        last_range.syntax_end < paragraph_range.syntax_end ||
        last_range.syntax_end >= item_range.syntax_end)
        return XUI_ERROR_NOT_FOUND;
    *definition_start = DOC_NONE;
    for (i = 0; i < references; i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(old->references, i, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if (ref.iSourceStart < item_range.syntax_start ||
            ref.iSourceEnd > item_range.syntax_end) continue;
        if (ref.iSourceStart < last_range.syntax_end)
            return XUI_ERROR_NOT_FOUND;
        if (ref.iKind != XUI_DOC_REFERENCE_LINK ||
            ref.iSourceStart == last_range.syntax_end) continue;
        if (*definition_start == DOC_NONE || ref.iSourceStart < *definition_start)
            *definition_start = ref.iSourceStart;
    }
    if (*definition_start == DOC_NONE ||
        *definition_start <= last_range.syntax_end)
        return XUI_ERROR_NOT_FOUND;
    result = doc_md_source_gap_trivia(t, old,
        last_range.syntax_end, *definition_start, 0);
    if (result != XUI_OK) return result;
    result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    result = doc_seq_read(old->source, *definition_start - 1, &gap_end, 1);
    if (result != XUI_OK) return result;
    if (gap_end != '\r' && gap_end != '\n')
        return XUI_ERROR_NOT_FOUND;
    result = doc_md_removed_item_paragraph_body(t, old, item, paragraph,
        body_start, body_end);
    if (result != XUI_OK) return result;
    *paragraph_end = paragraph_range.syntax_end;
    return XUI_OK;
}
static int doc_md_write_source_range(xui_document_transaction t,
    doc_state* old, uint64_t start, uint64_t end, doc_md_output* output)
{
    while (start < end) {
        char buffer[4096];
        uint64_t bytes = end - start < sizeof(buffer) ? end - start : sizeof(buffer);
        int result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(old->source, start, buffer, bytes);
        if (result != XUI_OK) return result;
        doc_md_write(output, buffer, bytes);
        if (output->error) return output->error;
        start += bytes;
    }
    return XUI_OK;
}
/* Relocate only complete source-only definitions. Blank trivia around
 * selected blocks must not follow them into an otherwise empty list item:
 * keeping it there would split that item from its nested list. */
static int doc_md_write_gap_definitions(xui_document_transaction t,
    doc_state* old, uint64_t start, uint64_t end, doc_md_output* output,
    doc_md_output* prelude, const char* line_ending,
    uint64_t ending_bytes)
{
    uint64_t i, at = start, refs = doc_seq_size(old->references);
    int result, seen_footnote = 0, route_prelude = 0, first = 1;
    if (start > end || end > doc_seq_size(old->source))
        return XUI_ERROR_NOT_FOUND;
    for (i = 0; i < refs; i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(old->references, i, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if (ref.iSourceEnd <= start || ref.iSourceStart >= end) continue;
        if (ref.iKind == XUI_DOC_REFERENCE_FOOTNOTE) seen_footnote = 1;
        else if (seen_footnote && ref.iKind == XUI_DOC_REFERENCE_LINK) {
            route_prelude = 1;
            break;
        }
    }
    for (i = 0; i < refs; i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        char ending;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(old->references, i, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if (ref.iSourceEnd <= start || ref.iSourceStart >= end) continue;
        if ((ref.iKind != XUI_DOC_REFERENCE_LINK &&
                ref.iKind != XUI_DOC_REFERENCE_FOOTNOTE) ||
            ref.iSourceStart < at || ref.iSourceEnd <= ref.iSourceStart ||
            ref.iSourceEnd > end)
            return XUI_ERROR_NOT_FOUND;
        result = doc_md_source_gap_trivia(t, old, at,
            ref.iSourceStart, 0);
        if (result != XUI_OK) return result;
        /* A link immediately following a footnote needs its original blank
         * separator. Place this definition run before the emptied list item,
         * preserving the source bytes and the definition order. */
        if (route_prelude) {
            if (first) {
                doc_md_write(prelude, line_ending, ending_bytes);
                if (prelude->error) return prelude->error;
            } else {
                result = doc_md_write_source_range(t, old, at,
                    ref.iSourceStart, prelude);
                if (result != XUI_OK) return result;
            }
        }
        result = doc_seq_read(old->source, ref.iSourceEnd - 1,
            &ending, 1);
        if (result != XUI_OK) return result;
        if (ending != '\r' && ending != '\n')
            return XUI_ERROR_NOT_FOUND;
        result = doc_md_write_source_range(t, old,
            ref.iSourceStart, ref.iSourceEnd,
            route_prelude ? prelude : output);
        if (result != XUI_OK) return result;
        at = ref.iSourceEnd;
        first = 0;
    }
    return doc_md_source_gap_trivia(t, old, at, end, 0);
}
static int doc_md_write_nonparagraph_item_prefix(xui_document_transaction t,
    doc_state* old, doc_node* item, doc_node* block, uint64_t copy_start,
    uint64_t block_start, uint64_t block_end, doc_md_output* moved);
/* An emptied preceding list item may own definitions after any selected
 * visible block. Keep the original marker and complete definition lines,
 * omitting selected blocks and blank lines that would split the nested list. */
static int doc_md_definition_item_trim_gaps(xui_document_transaction t,
    doc_state* old, doc_node* item, uint64_t copy_start,
    doc_md_output* moved, doc_md_output* prelude,
    const char* line_ending, uint64_t ending_bytes, uint64_t* copy_after)
{
    doc_node* first;
    uint64_t count = doc_seq_size(item->children), item_start, item_end;
    uint64_t first_start, first_end;
    uint64_t body_start, body_end, previous_end, i;
    int has_definition = 0, result;
    if (!count || doc_md_bounds(old, item->id, &item_start, &item_end) != XUI_OK)
        return XUI_ERROR_NOT_FOUND;
    first = doc_index_get(old->index, doc_seq_get_id(item->children, 0));
    if (!first || !doc_md_removable_item_lead_kind(first->kind) ||
        doc_md_bounds(old, first->id, &first_start, &first_end) != XUI_OK ||
        copy_start > first_start || item_start > first_start ||
        first_end > item_end)
        return XUI_ERROR_NOT_FOUND;
    for (i = 0; i < doc_seq_size(old->references);
        i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(old->references, i, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if (ref.iSourceEnd <= item_start || ref.iSourceStart >= item_end) continue;
        if (ref.iKind != XUI_DOC_REFERENCE_LINK ||
            ref.iSourceStart < first_end || ref.iSourceEnd > item_end)
            return XUI_ERROR_NOT_FOUND;
        has_definition = 1;
    }
    if (!has_definition) return XUI_ERROR_NOT_FOUND;
    if (first->kind == XUI_DOC_PARAGRAPH) {
        result = doc_md_removed_item_paragraph_body(t, old, item, first,
            &body_start, &body_end);
        if (result != XUI_OK || body_start < copy_start || body_end > first_end)
            return result == XUI_OK ? XUI_ERROR_NOT_FOUND : result;
        result = doc_md_write_source_range(t, old, copy_start, body_start, moved);
        if (result != XUI_OK) return result;
        result = doc_md_write_source_range(t, old, body_end, first_end, moved);
    } else {
        result = doc_md_write_nonparagraph_item_prefix(t, old, item, first,
            copy_start, first_start, first_end, moved);
    }
    if (result != XUI_OK) return result;
    previous_end = first_end;
    for (i = 1; i < count; i++) {
        doc_node* child = doc_index_get(old->index,
            doc_seq_get_id(item->children, i));
        uint64_t child_start, child_end;
        if (!child || doc_md_bounds(old, child->id,
                &child_start, &child_end) != XUI_OK ||
            previous_end > child_start || child_end > item_end)
            return XUI_ERROR_NOT_FOUND;
        result = doc_md_write_gap_definitions(t, old, previous_end,
            child_start, moved, prelude, line_ending, ending_bytes);
        if (result != XUI_OK) return result;
        previous_end = child_end;
    }
    result = doc_md_write_gap_definitions(t, old, previous_end,
        item_end, moved, prelude, line_ending, ending_bytes);
    if (result != XUI_OK) return result;
    *copy_after = item_end;
    return XUI_OK;
}
/* A selected nonparagraph block can be the first child of a list item. If
 * its opener shares the marker line, keep only that marker and the original
 * line ending, scanning beyond the marker prefix for a long opener line.
 * Otherwise the source before the block already contains the complete
 * standalone marker line. The candidate parse checks the result. */
static int doc_md_write_nonparagraph_item_prefix(xui_document_transaction t,
    doc_state* old, doc_node* item, doc_node* block, uint64_t copy_start,
    uint64_t block_start, uint64_t block_end, doc_md_output* moved)
{
    doc_md_output storage = {0};
    const char* line; char ending[2];
    uint64_t item_start, item_end, header_end, marker_start = 0, marker_end;
    uint64_t continuation, at, ending_bytes = 0;
    int result, prefix_complete = 0;
    if (doc_md_bounds(old, item->id, &item_start, &item_end) != XUI_OK ||
        copy_start > item_start || item_start > block_start ||
        block_end > item_end) return XUI_ERROR_NOT_FOUND;
    if (item_start < block_start)
        return doc_md_write_source_range(t, old, copy_start,
            block_start, moved);
    storage.allocator = old->allocator;
    at = block_start;
    /* Retain the whole indentation/marker and enough opener bytes to prove
     * its kind. Scan the remaining header for its exact EOL without retaining
     * an arbitrarily long fence info string, HTML line or table row. */
    while (at < block_end) {
        char chunk[4096];
        uint64_t count = block_end - at < sizeof(chunk) ?
            block_end - at : sizeof(chunk), i;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) goto done;
        result = doc_seq_read(old->source, at, chunk, count);
        if (result != XUI_OK) goto done;
        for (i = 0; i < count && chunk[i] != '\n' && chunk[i] != '\r'; i++) {}
        if (!prefix_complete) {
            doc_md_write(&storage, chunk, i);
            if (storage.error) { result = storage.error; goto done; }
            line = storage.data; header_end = storage.size;
            marker_start = 0;
            while (marker_start < header_end &&
                (line[marker_start] == ' ' || line[marker_start] == '\t')) marker_start++;
            marker_end = doc_md_list_marker_prefix(line, header_end, marker_start,
                !!(item->attrs->iFlags & XUI_DOC_TASK), &continuation);
            prefix_complete = marker_end > marker_start && header_end - marker_end >= 3;
        }
        if (i < count) {
            char next;
            ending[0] = chunk[i]; ending_bytes = 1;
            if (chunk[i] == '\r' && at + i + 1 < block_end) {
                if (i + 1 < count) next = chunk[i + 1];
                else {
                    result = doc_seq_read(old->source, at + i + 1, &next, 1);
                    if (result != XUI_OK) goto done;
                }
                if (next == '\n') ending[1] = '\n', ending_bytes = 2;
            }
            break;
        }
        at += count;
    }
    if (!ending_bytes) { result = XUI_ERROR_NOT_FOUND; goto done; }
    line = storage.data; header_end = storage.size; marker_start = 0;
    while (marker_start < header_end &&
        (line[marker_start] == ' ' || line[marker_start] == '\t')) marker_start++;
    marker_end = doc_md_list_marker_prefix(line, header_end, marker_start,
        !!(item->attrs->iFlags & XUI_DOC_TASK), &continuation);
    if (marker_end == marker_start || marker_end >= header_end) {
        result = XUI_ERROR_NOT_FOUND; goto done;
    }
    if (block->kind == XUI_DOC_CODE_BLOCK || block->kind == XUI_DOC_DIAGRAM) {
        char fence = line[marker_end];
        if ((fence != '`' && fence != '~') || header_end - marker_end < 3 ||
            line[marker_end + 1] != fence || line[marker_end + 2] != fence) {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
    } else if (block->kind == XUI_DOC_QUOTE) {
        if (line[marker_end] != '>') { result = XUI_ERROR_NOT_FOUND; goto done; }
    } else if (block->kind == XUI_DOC_HEADING) {
        if (line[marker_end] != '#') { result = XUI_ERROR_NOT_FOUND; goto done; }
    } else if (block->kind == XUI_DOC_RULE) {
        if (line[marker_end] != '*' && line[marker_end] != '-' && line[marker_end] != '_') {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
    } else if (block->kind == XUI_DOC_HTML) {
        if (line[marker_end] != '<') { result = XUI_ERROR_NOT_FOUND; goto done; }
    } else if (block->kind != XUI_DOC_TABLE) {
        result = XUI_ERROR_NOT_FOUND; goto done;
    }
    result = doc_md_write_source_range(t, old, copy_start,
        block_start + marker_end, moved);
    if (result != XUI_OK) goto done;
    doc_md_write(moved, ending, ending_bytes);
    result = moved->error ? moved->error : XUI_OK;
done:
    doc_free(storage.data);
    return result;
}
/* A bare quote tail needs the original list/quote opener after its only visible
 * paragraph moves out. Copy source-only list markers and whitespace along its
 * path, omitting selected parent blocks but keeping the item's marker and
 * line ending. The full candidate parse proves the semantics. */
/* Validate the parser-owned opener without imposing a stack-buffer length
 * limit. Keep cancellation/allocation errors distinct from an unproved prefix;
 * the caller copies its original bytes after all structural checks pass. */
static int doc_md_nested_quote_prefix(xui_document_transaction t,
    doc_state* old, uint64_t start, uint64_t end, uint64_t quote_count)
{
    doc_md_output storage = {0};
    uint64_t p = 0, length, marker_count = 0;
    const char* prefix;
    int result;
    storage.allocator = old->allocator;
    result = doc_md_write_source_range(t, old, start, end, &storage);
    if (result != XUI_OK) goto done;
    prefix = storage.data; length = storage.size;
    while (p < length && p < 3 && prefix[p] == ' ') p++;
    if (p == length) goto not_found;
    if (prefix[p] == '-' || prefix[p] == '+' || prefix[p] == '*') p++;
    else {
        uint64_t digits = 0;
        while (p < length && prefix[p] >= '0' && prefix[p] <= '9' && digits < 9) {
            p++; digits++;
        }
        if (!digits || p == length || (prefix[p] != '.' && prefix[p] != ')'))
            goto not_found;
        p++;
    }
    if (p == length || (prefix[p] != ' ' && prefix[p] != '\t')) goto not_found;
    while (p < length && (prefix[p] == ' ' || prefix[p] == '\t')) p++;
    while (p < length && prefix[p] == '>') {
        p++; marker_count++;
        while (p < length && (prefix[p] == ' ' || prefix[p] == '\t')) p++;
    }
    if (marker_count != quote_count || p != length) goto not_found;
    result = XUI_OK;
    goto done;
not_found:
    result = XUI_ERROR_NOT_FOUND;
done:
    doc_free(storage.data);
    return result;
}
static int doc_md_nested_empty_quote_scaffold(xui_document_transaction t,
    doc_state* old, doc_state* desired, const doc_range_branch_plan* plan,
    doc_node* last_block, doc_node* last, doc_md_output* moved,
    doc_md_output* prelude)
{
    doc_node *quote, *child, *item, *list, *remaining, *left_root;
    doc_node_source_range text_range;
    uint64_t block_start, block_end, quote_start, quote_end;
    uint64_t quote_count = 0, list_count = 0;
    uint64_t list_start = 0, list_end = 0, left_start, left_end, copy_start = 0;
    char ending[2];
    uint64_t ending_bytes = 1;
    int result;
    if (moved->size || plan->ancestor != DOC_ROOT ||
        last_block->kind != XUI_DOC_PARAGRAPH ||
        doc_child_index(last_block, last->id) != 0) return XUI_OK;
    if (doc_md_bounds(old, last_block->id, &block_start, &block_end) != XUI_OK ||
        !block_end) return XUI_OK;
    child = last_block;
    quote = doc_index_get(old->index, child->parent);
    while (quote && quote->kind == XUI_DOC_QUOTE) {
        remaining = doc_index_get(desired->index, quote->id);
        if (!remaining ||
            doc_seq_size(remaining->children) != (child == last_block ? 0 : 1) ||
            (child != last_block &&
                doc_seq_get_id(remaining->children, 0) != child->id) ||
            doc_md_bounds(old, quote->id, &quote_start, &quote_end) != XUI_OK ||
            block_start != quote_start || block_end >= quote_end)
            return XUI_OK;
        quote_count++;
        child = quote;
        quote = doc_index_get(old->index, quote->parent);
    }
    if (!quote_count) return XUI_OK;
    for (;;) {
        item = quote;
        list = item ? doc_index_get(old->index, item->parent) : NULL;
        if (!item || item->kind != XUI_DOC_LIST_ITEM ||
            !list || list->kind != XUI_DOC_LIST)
            return XUI_OK;
        list_count++;
        if (list->id == plan->right_child) break;
        if (!doc_md_nested_path_item(old, desired, item, child->id) ||
            !doc_md_nested_path_list(desired, list, item->id))
            return XUI_OK;
        child = list;
        quote = doc_index_get(old->index, list->parent);
    }
    if (list_count > 1) {
        if (!doc_md_nested_path_item(old, desired, item, child->id) ||
            !doc_md_nested_path_list(desired, list, item->id) ||
            doc_md_bounds(old, list->id, &list_start, &list_end) != XUI_OK ||
            list_start >= block_start || list_end <= block_end)
            return XUI_OK;
    }
    doc_node_source_range_get(old, last, &text_range);
    if (text_range.source_start == DOC_NONE ||
        text_range.source_start <= block_start ||
        text_range.source_start >= block_end) return XUI_OK;
    if (list_count == 1) {
        result = doc_md_nested_quote_prefix(t, old, block_start,
            text_range.source_start, quote_count);
        if (result == XUI_ERROR_NOT_FOUND) return XUI_OK;
        if (result != XUI_OK) return result;
    } else {
        left_root = doc_index_get(old->index, plan->left_child);
        if (!left_root ||
            (left_root->kind != XUI_DOC_PARAGRAPH &&
                left_root->kind != XUI_DOC_HEADING) ||
            doc_md_bounds(old, left_root->id, &left_start, &left_end) != XUI_OK ||
            left_end > list_start || text_range.source_start <= list_start)
            return XUI_OK;
        result = doc_md_source_gap_trivia(t, old, left_end, list_start, 0);
        if (result == XUI_ERROR_NOT_FOUND) return XUI_OK;
        if (result != XUI_OK) return result;
        copy_start = left_end;
    }
    result = doc_seq_read(old->source, block_end - 1, ending, 1);
    if (result != XUI_OK) return result;
    if (ending[0] == '\n' && block_end > 1) {
        char before;
        result = doc_seq_read(old->source, block_end - 2, &before, 1);
        if (result != XUI_OK) return result;
        if (before == '\r') { ending[0] = '\r'; ending[1] = '\n'; ending_bytes = 2; }
    } else if (ending[0] != '\r') return XUI_OK;
    if (list_count == 1) {
        result = doc_md_write_source_range(t, old, block_start,
            text_range.source_start, moved);
        if (result != XUI_OK) return result;
    }
    else {
        doc_node* path_list = list;
        uint64_t depth;
        for (depth = list_count; depth; depth--) {
            doc_node* remaining_list = doc_index_get(desired->index,
                path_list->id);
            uint64_t path_id = doc_md_nested_path_child(old,
                path_list->id, last_block->id);
            uint64_t index = doc_child_index(path_list, path_id);
            doc_node* path_item = doc_index_get(old->index, path_id);
            doc_node* target = doc_index_get(desired->index, path_id);
            if (!remaining_list || path_id == DOC_NONE || index == DOC_NONE ||
                !path_item || !target) return XUI_ERROR_NOT_FOUND;
            if (index) {
                uint64_t target_start, target_end, drop_start = DOC_NONE;
                uint64_t prefix = doc_child_index(remaining_list, path_id);
                uint64_t i, retained = 0;
                if (prefix == DOC_NONE ||
                    doc_md_bounds(old, path_id, &target_start,
                        &target_end) != XUI_OK ||
                    target_start > text_range.source_start)
                    return XUI_ERROR_NOT_FOUND;
                for (i = 0; i < index; i++) {
                    uint64_t sibling_id = doc_seq_get_id(path_list->children, i);
                    uint64_t sibling_start, sibling_end;
                    int keep = retained < prefix && sibling_id ==
                        doc_seq_get_id(remaining_list->children, retained);
                    if (doc_md_bounds(old, sibling_id, &sibling_start,
                            &sibling_end) != XUI_OK ||
                        sibling_start > sibling_end ||
                        sibling_end > target_start)
                        return XUI_ERROR_NOT_FOUND;
                    if (keep) {
                        if (drop_start != DOC_NONE) {
                            if (drop_start < copy_start ||
                                sibling_start < drop_start)
                                return XUI_ERROR_NOT_FOUND;
                            result = doc_md_write_source_range(t, old,
                                copy_start, drop_start, moved);
                            if (result != XUI_OK) return result;
                            copy_start = sibling_start;
                            drop_start = DOC_NONE;
                        }
                        {
                            doc_node* sibling = doc_index_get(old->index,
                                sibling_id);
                            doc_node* retained_item = doc_index_get(desired->index,
                                sibling_id);
                            if (sibling && retained_item &&
                                doc_seq_size(sibling->children) &&
                                !doc_seq_size(retained_item->children)) {
                                uint64_t body_start, body_end, paragraph_end;
                                uint64_t definition_start;
                                result = doc_md_definition_item_trim(t, old,
                                    sibling, &body_start, &body_end,
                                    &paragraph_end, &definition_start);
                                if (result == XUI_ERROR_NOT_FOUND) {
                                    result = doc_md_definition_item_trim_gaps(t,
                                        old, sibling, copy_start, moved,
                                        prelude, ending, ending_bytes,
                                        &copy_start);
                                    if (result != XUI_OK) return result;
                                    retained++;
                                    continue;
                                }
                                if (result != XUI_OK || body_start < copy_start ||
                                    paragraph_end > definition_start)
                                    return result == XUI_OK ?
                                        XUI_ERROR_NOT_FOUND : result;
                                result = doc_md_write_source_range(t, old,
                                    copy_start, body_start, moved);
                                if (result != XUI_OK) return result;
                                result = doc_md_write_source_range(t, old,
                                    body_end, paragraph_end, moved);
                                if (result != XUI_OK) return result;
                                copy_start = definition_start;
                            }
                        }
                        retained++;
                    } else if (drop_start == DOC_NONE) {
                        if (sibling_start < copy_start)
                            return XUI_ERROR_NOT_FOUND;
                        drop_start = sibling_start;
                    }
                }
                if (retained != prefix) return XUI_ERROR_NOT_FOUND;
                if (drop_start != DOC_NONE) {
                    if (drop_start < copy_start ||
                        target_start < drop_start)
                        return XUI_ERROR_NOT_FOUND;
                    result = doc_md_write_source_range(t, old,
                        copy_start, drop_start, moved);
                    if (result != XUI_OK) return result;
                    copy_start = target_start;
                }
            }
            if (doc_seq_size(path_item->children) >= 2) {
                doc_node* removed = doc_index_get(old->index,
                    doc_seq_get_id(path_item->children, 0));
                uint64_t body_start, body_end;
                uint64_t count = doc_seq_size(path_item->children);
                if (!removed) return XUI_ERROR_NOT_FOUND;
                if (removed->kind != XUI_DOC_PARAGRAPH) {
                    uint64_t first_start, first_end, child_start, child_end;
                    uint64_t previous_end, j;
                    if (!doc_md_removable_item_lead_kind(removed->kind) ||
                        doc_md_bounds(old, removed->id, &first_start,
                            &first_end) != XUI_OK ||
                        doc_md_bounds(old,
                            doc_seq_get_id(path_item->children, count - 1),
                            &child_start, &child_end) != XUI_OK ||
                        first_end > child_start ||
                        child_start > text_range.source_start)
                        return XUI_ERROR_NOT_FOUND;
                    result = doc_md_write_nonparagraph_item_prefix(t, old,
                        path_item, removed, copy_start, first_start,
                        first_end, moved);
                    if (result != XUI_OK) return result;
                    previous_end = first_end;
                    for (j = 1; j < count - 1; j++) {
                        uint64_t block_start, block_end;
                        doc_node* block = doc_index_get(old->index,
                            doc_seq_get_id(path_item->children, j));
                        if (!block || doc_md_bounds(old, block->id,
                                &block_start, &block_end) != XUI_OK ||
                            block_start < previous_end ||
                            block_end > child_start)
                            return XUI_ERROR_NOT_FOUND;
                        result = doc_md_write_gap_definitions(t, old,
                            previous_end, block_start, moved, prelude,
                            ending, ending_bytes);
                        if (result != XUI_OK) return result;
                        previous_end = block_end;
                    }
                    result = doc_md_write_gap_definitions(t, old,
                        previous_end, child_start, moved, prelude,
                        ending, ending_bytes);
                    if (result != XUI_OK) return result;
                    copy_start = child_start;
                    goto next_path_item;
                }
                result = doc_md_removed_item_paragraph_body(t, old,
                    path_item, removed, &body_start, &body_end);
                if (result != XUI_OK) return result;
                if (body_start < copy_start ||
                    body_end > text_range.source_start)
                    return XUI_ERROR_NOT_FOUND;
                result = doc_md_write_source_range(t, old, copy_start,
                    body_start, moved);
                if (result != XUI_OK) return result;
                {
                    uint64_t paragraph_start, paragraph_end, child_start;
                    uint64_t child_end;
                    uint64_t previous_end, j;
                    int has_definition = 0;
                    if (doc_md_bounds(old, removed->id,
                            &paragraph_start, &paragraph_end) != XUI_OK ||
                        doc_md_bounds(old,
                            doc_seq_get_id(path_item->children, count - 1),
                            &child_start, &child_end) != XUI_OK ||
                        paragraph_end < body_end ||
                        child_start < paragraph_end ||
                        child_start > text_range.source_start)
                        return XUI_ERROR_NOT_FOUND;
                    if (count == 2) {
                        uint64_t refs = doc_seq_size(old->references);
                        for (j = 0; j < refs;
                                j += sizeof(xui_doc_reference_definition_t)) {
                            xui_doc_reference_definition_t ref;
                            result = doc_txn_check(t, 0);
                            if (result != XUI_OK) return result;
                            result = doc_seq_read(old->references, j,
                                &ref, sizeof(ref));
                            if (result != XUI_OK) return result;
                            if (ref.iSourceEnd > paragraph_end &&
                                ref.iSourceStart < child_start) {
                                has_definition = 1;
                                break;
                            }
                        }
                    }
                    if (count > 2 || has_definition) {
                        result = doc_md_write_source_range(t, old, body_end,
                            paragraph_end, moved);
                        if (result != XUI_OK) return result;
                        previous_end = paragraph_end;
                        for (j = 1; j < count - 1; j++) {
                            uint64_t block_start, block_end;
                            doc_node* block = doc_index_get(old->index,
                                doc_seq_get_id(path_item->children, j));
                            if (!block || doc_md_bounds(old, block->id,
                                    &block_start, &block_end) != XUI_OK ||
                                block_start < previous_end ||
                                block_end > child_start)
                                return XUI_ERROR_NOT_FOUND;
                            result = doc_md_write_gap_definitions(t, old,
                                previous_end, block_start, moved, prelude,
                                ending, ending_bytes);
                            if (result != XUI_OK) return result;
                            previous_end = block_end;
                        }
                        result = doc_md_write_gap_definitions(t, old,
                            previous_end, child_start, moved, prelude,
                            ending, ending_bytes);
                        if (result != XUI_OK) return result;
                        copy_start = child_start;
                    } else copy_start = body_end;
                }
            }
next_path_item:
            if (depth > 1) {
                path_list = doc_index_get(old->index,
                    doc_seq_get_id(target->children, 0));
                if (!path_list || path_list->kind != XUI_DOC_LIST)
                    return XUI_ERROR_NOT_FOUND;
            }
        }
        result = doc_md_write_source_range(t, old, copy_start,
            text_range.source_start, moved);
        if (result != XUI_OK) return result;
    }
    doc_md_write(moved, ending, ending_bytes);
    return moved->error ? moved->error : XUI_OK;
}
/* A decoded entity may contain two Unicode scalars. If a range ends between
 * them, replace the whole source entity and spell its unselected scalar as a
 * numeric entity. Partial provenance cannot prove ownership of the complete
 * source token and is deliberately left to the conservative fallback. */
static int doc_md_range_entity_at(doc_state* state, doc_node* node,
    uint64_t offset, xui_doc_source_segment_t* out)
{
    uint64_t i;
    for (i = 0; i < doc_seq_size(node->provenance); i += sizeof(*out)) {
        xui_doc_source_segment_t segment;
        if (doc_seq_read(node->provenance, i, &segment, sizeof(segment)) != XUI_OK)
            return XUI_ERROR_NOT_FOUND;
        doc_source_resolve_segment(state, node, &segment);
        if (segment.iKind == XUI_DOC_SOURCE_ENTITY &&
            !(segment.iFlags & (XUI_DOC_SOURCE_PARTIAL_START | XUI_DOC_SOURCE_PARTIAL_END)) &&
            segment.iTextStart < offset && offset < segment.iTextEnd &&
            segment.iTextEnd <= doc_seq_size(node->text) &&
            segment.iSourceStart < segment.iSourceEnd &&
            segment.iSourceEnd <= doc_seq_size(state->source)) {
            *out = segment;
            return XUI_OK;
        }
    }
    return XUI_ERROR_NOT_FOUND;
}
static int doc_md_write_entity_slice(doc_md_output* output, doc_node* node,
    uint64_t start, uint64_t end)
{
    uint64_t at;
    for (at = start; at < end && !output->error;) {
        char scalar[4]; unsigned char first; uint64_t width;
        if (doc_seq_read(node->text, at, &first, 1) != XUI_OK)
            return XUI_ERROR_NOT_FOUND;
        width = first < 0x80 ? 1 : first < 0xe0 ? 2 : first < 0xf0 ? 3 : 4;
        if (width > end - at || doc_seq_read(node->text, at, scalar, width) != XUI_OK)
            return XUI_ERROR_NOT_FOUND;
        doc_md_write_entity(output, scalar, width);
        at += width;
    }
    return output->error ? output->error : XUI_OK;
}
static int doc_md_inline_owned_source_range(xui_document_transaction, uint64_t, uint64_t, int, uint64_t*, uint64_t*);
/* Shortcut and collapsed references take their key from the display label.
 * A content edit needs an explicit key on both surviving pieces. Use the
 * definition selected by the same parser, including dedented multiline labels
 * and first-definition precedence. Only label whitespace is normalized, and
 * the pinned parser must prove that the normalized key still compares equal. */
static int doc_md_reference_split_close(xui_document_transaction t,
    const xui_doc_inline_syntax_t* syntax, doc_md_output* close)
{
    doc_md_output label = {0}; char* original = NULL;
    uint64_t bytes, at; int result;
    label.allocator = t->draft->allocator;
    result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
    result = doc_reference_label_copy(t->draft, syntax->iDefinitionIndex, NULL, 0, &bytes);
    if (result != XUI_OK) return result;
    if (!bytes || bytes >= SIZE_MAX) return XUI_ERROR_NOT_FOUND;
    original = doc_alloc(t->draft->allocator, (size_t)bytes + 1);
    if (!original) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_reference_label_copy(t->draft, syntax->iDefinitionIndex, original, bytes, &bytes);
    if (result != XUI_OK) goto done;
    original[bytes] = 0;
    for (at = 0; at < bytes && !label.error;) {
        uint64_t width = doc_md_scalar_end(original + at, bytes - at);
        uint32_t scalar = doc_md_scalar_value(original + at, width);
        result = doc_txn_check(t, 0); if (result != XUI_OK) goto done;
        if (doc_md4c_unicode_whitespace(scalar)) {
            if (label.size && label.data[label.size - 1] != ' ') doc_md_write(&label, " ", 1);
        } else doc_md_write(&label, original + at, width);
        at += width;
    }
    if (label.error) { result = label.error; goto done; }
    if (label.size && label.data[label.size - 1] == ' ') label.size--;
    doc_md_write(&label, "", 1);
    if (label.error) { result = label.error; goto done; }
    if (!doc_md4c_label_equal(original, label.data)) { result = XUI_ERROR_NOT_FOUND; goto done; }
    doc_md_write(close, "][", 2);
    doc_md_write(close, label.data, label.size - 1);
    doc_md_write(close, "]", 1);
    result = close->error;
done:
    doc_free(original); doc_free(label.data); return result;
}
/* A paragraph split must close and reopen the parser-owned inline chain.
 * Copy concrete delimiters and link suffixes, preserving spelling outside
 * the edit. The candidate is still reparsed against the whole desired tree. */
static int doc_md_inline_split_wrappers(xui_document_transaction t,
    uint64_t start, uint64_t end, doc_md_output* close, doc_md_output* open,
    doc_md_output* reference_close, uint64_t* reference_start, uint64_t* reference_end)
{
    doc_state* state = t->draft;
    xui_doc_inline_syntax_t owned = {0};
    doc_md_output chain = {0};
    uint64_t count = doc_seq_size(state->inline_syntax) / sizeof(owned);
    uint64_t owner = DOC_NONE, i, depth;
    int result = XUI_OK;
    chain.allocator = state->allocator;
    for (i = 0; i < count; i++) {
        xui_doc_inline_syntax_t syntax;
        result = doc_txn_check(t, 0); if (result != XUI_OK) goto done;
        result = doc_seq_read_syntax(state->inline_syntax, i, &syntax);
        if (result != XUI_OK) goto done;
        if (syntax.iKind == XUI_DOC_SYNTAX_DESTINATION || syntax.iKind == XUI_DOC_SYNTAX_TITLE ||
            syntax.iContentStart > start || syntax.iContentEnd < end) continue;
        switch (syntax.iKind) {
        case XUI_DOC_SYNTAX_EMPHASIS: case XUI_DOC_SYNTAX_STRONG:
        case XUI_DOC_SYNTAX_UNDERLINE: case XUI_DOC_SYNTAX_STRIKE:
        case XUI_DOC_SYNTAX_CODE: case XUI_DOC_SYNTAX_LINK:
        case XUI_DOC_SYNTAX_HIGHLIGHT: case XUI_DOC_SYNTAX_SUBSCRIPT:
        case XUI_DOC_SYNTAX_SUPERSCRIPT: break;
        default: result = XUI_ERROR_NOT_FOUND; goto done;
        }
        if (syntax.iFlags & XUI_DOC_SYNTAX_AUTOLINK) {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
        if (owner == DOC_NONE || (syntax.iSourceStart >= owned.iSourceStart &&
                syntax.iSourceEnd <= owned.iSourceEnd)) { owner = i; owned = syntax; }
    }
    if (owner == DOC_NONE) goto done;
    for (depth = 0; depth < count; depth++) {
        xui_doc_inline_syntax_t parent;
        if (owned.iSourceStart >= owned.iContentStart ||
            owned.iContentStart > start || end > owned.iContentEnd ||
            owned.iContentEnd >= owned.iSourceEnd ||
            owned.iSourceEnd > doc_seq_size(state->source)) {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
        doc_md_write(&chain, (const char*)&owned, sizeof(owned));
        if (chain.error) { result = chain.error; goto done; }
        if (owned.iParentIndex == DOC_NONE) break;
        if (owned.iParentIndex >= count) { result = XUI_ERROR_NOT_FOUND; goto done; }
        result = doc_seq_read_syntax(state->inline_syntax, owned.iParentIndex, &parent);
        if (result != XUI_OK) goto done;
        if (parent.iContentStart > owned.iSourceStart ||
            parent.iContentEnd < owned.iSourceEnd ||
            parent.iSourceStart > owned.iSourceStart ||
            parent.iSourceEnd < owned.iSourceEnd) {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
        owned = parent;
    }
    if (depth == count) { result = XUI_ERROR_NOT_FOUND; goto done; }
    count = chain.size / sizeof(owned);
    for (i = 0; i < count && result == XUI_OK; i++) {
        memcpy(&owned, chain.data + i * sizeof(owned), sizeof(owned));
        if (owned.iKind == XUI_DOC_SYNTAX_LINK && owned.iDefinitionIndex != DOC_NONE &&
            (owned.iSourceEnd - owned.iContentEnd == 1 || owned.iSourceEnd - owned.iContentEnd == 3)) {
            char suffix[3]; uint64_t bytes = owned.iSourceEnd - owned.iContentEnd;
            result = doc_seq_read(state->source, owned.iContentEnd, suffix, bytes);
            if (result != XUI_OK) goto done;
            if (*reference_start != DOC_NONE || suffix[0] != ']' ||
                (bytes == 3 && (suffix[1] != '[' || suffix[2] != ']'))) {
                result = XUI_ERROR_NOT_FOUND; goto done;
            }
            result = doc_md_reference_split_close(t, &owned, reference_close);
            if (result != XUI_OK) goto done;
            *reference_start = owned.iContentEnd; *reference_end = owned.iSourceEnd;
            doc_md_write(close, reference_close->data, reference_close->size);
            result = close->error;
        } else result = doc_md_write_source_range(t, state, owned.iContentEnd,
            owned.iSourceEnd, close);
    }
    for (i = count; i > 0 && result == XUI_OK; i--) {
        memcpy(&owned, chain.data + (i - 1) * sizeof(owned), sizeof(owned));
        result = doc_md_write_source_range(t, state, owned.iSourceStart,
            owned.iContentStart, open);
    }
done:
    doc_free(chain.data); return result;
}
/* List splitting shares the parser-owned inline closure with paragraph
 * splitting. Insert only the new item boundary; all surrounding spelling
 * stays in SourceStore. Existing continuation indents are adjusted from
 * the caret onward when an ordered marker changes logical width. */
static int doc_md_range_text_endpoint(doc_state*, xui_doc_position_t*, int);
/* The first surviving block can contribute whitespace after the new marker.
 * Markdown may consume that whitespace as marker padding, so the old selected
 * paragraph's marker width is not the new item's content column. Ask the same
 * parser in a private candidate; never duplicate its padding/Tab grammar. */
static int doc_md_list_split_carry_delta(xui_document_transaction t, doc_node* list,
    uint64_t start, uint64_t end, doc_md_output* carry, uint64_t marker_offset,
    uint32_t old_content_column, int* delta)
{
    struct xui_doc_transaction_t probe = {0}; doc_node* parsed_list;
    uint64_t i, marker; int result;
    if (marker_offset >= carry->size || start > UINT64_MAX - marker_offset ||
        old_content_column > INT_MAX) return XUI_ERROR_NOT_FOUND;
    marker = start + marker_offset;
    result = doc_markdown_shadow_begin(t, &probe);
    if (result == XUI_OK) result = doc_txn_source_patch(&probe, start, end, carry->data, carry->size, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&probe);
    if (result != XUI_OK) goto done;
    parsed_list = doc_index_get(probe.draft->index,
        doc_markdown_resolve_node(t->draft, probe.draft, list->id));
    if (!parsed_list || parsed_list->kind != XUI_DOC_LIST) { result = XUI_ERROR_NOT_FOUND; goto done; }
    result = XUI_ERROR_NOT_FOUND;
    for (i = 0; i < doc_seq_size(parsed_list->children); i++) {
        doc_node* item = doc_index_get(probe.draft->index, doc_seq_get_id(parsed_list->children, i));
        doc_node_source_range range; doc_list_first_indent_relative indent;
        result = doc_txn_check(t, 0); if (result != XUI_OK) goto done;
        if (!item || item->kind != XUI_DOC_LIST_ITEM ||
            item->marker_kind != XUI_DOC_BLOCK_SYNTAX_LIST_ITEM) continue;
        doc_node_source_range_get(probe.draft, item, &range);
        if (range.syntax_start == DOC_NONE || item->marker_primary_start > UINT64_MAX - range.syntax_start ||
            range.syntax_start + item->marker_primary_start != marker) continue;
        if (!item->syntax_aux || item->syntax_aux->size != sizeof(indent)) break;
        memcpy(&indent, item->syntax_aux->data, sizeof(indent));
        if (indent.start_column >= indent.content_column || indent.content_column > indent.end_column ||
            indent.content_column > INT_MAX) break;
        *delta = (int)indent.content_column - (int)old_content_column;
        result = XUI_OK; goto done;
    }
    result = XUI_ERROR_NOT_FOUND;
done:
    doc_state_release(probe.draft); doc_free(probe.ops); return result;
}
static int doc_md_list_split_inline_patch(xui_document_transaction t, doc_state* desired,
    const xui_doc_position_t* at, struct xui_doc_snapshot_t* snapshot, doc_node* item, doc_node* list,
    const doc_node_source_range* range, const doc_node_source_range* item_range,
    doc_md_output* first_prefix, doc_md_output* next_prefix, doc_md_output* continuation, const char* ending, uint64_t ending_bytes,
    uint64_t indent_count, uint64_t marker_offset, int indent_delta, int terminal_ending, uint64_t editable_start, uint64_t editable_end)
{
    struct xui_doc_transaction_t candidate = {0}; doc_state* old = t->draft;
    doc_node* node = doc_index_get(old->index, at->iNodeId);
    doc_node* paragraph = at->iKind == XUI_DOC_POSITION_TEXT && node ? doc_index_get(old->index, node->parent) : node;
    xui_doc_position_t source_at = *at;
    doc_md_output close = {0}, open = {0}, reference_close = {0}, output = {0}, carry = {0};
    uint64_t start, reference_start = DOC_NONE, reference_end = DOC_NONE, i;
    uint64_t carry_start = DOC_NONE, carry_end = DOC_NONE, carry_prefix_cut = 0, carry_extra = 0;
    uint32_t carry_content_column = 0;
    int quality, result = XUI_ERROR_NOT_FOUND, references_equal, first = 0, last = 0;
    if (!paragraph || (paragraph->kind != XUI_DOC_PARAGRAPH && paragraph->kind != XUI_DOC_HEADING)) return result;
    if (paragraph->kind == XUI_DOC_PARAGRAPH) {
        uint64_t count = doc_seq_size(paragraph->children);
        first = !at->iOffset && ((at->iKind == XUI_DOC_POSITION_GAP) ||
            (at->iKind == XUI_DOC_POSITION_TEXT && count && doc_seq_get_id(paragraph->children, 0) == node->id));
        last = (at->iKind == XUI_DOC_POSITION_GAP && at->iOffset == count) ||
            (at->iKind == XUI_DOC_POSITION_TEXT && count && doc_seq_get_id(paragraph->children, count - 1) == node->id &&
             at->iOffset == doc_seq_size(node->text));
    }
    /* A whole paragraph edge is outside its concrete inline wrappers. Moving
     * that boundary keeps the complete original payload and avoids empty
     * format/link wrappers or serialization of an unchanged later block. */
    if (first) start = range->syntax_start + first_prefix->size;
    else if (last) {
        if (terminal_ending && range->syntax_end < ending_bytes) return result;
        start = range->syntax_end - (terminal_ending ? ending_bytes : 0);
    } else {
        if (doc_md_range_text_endpoint(old, &source_at, 0) != XUI_OK) return result;
        node = doc_index_get(old->index, source_at.iNodeId);
        if (!node || node->kind != XUI_DOC_TEXT ||
            doc_source_map_text(old, node, source_at.iOffset, XUI_DOC_AFTER, &start, &quality) != XUI_OK ||
            quality != XUI_DOC_MAP_EXACT) return result;
    }
    if (start < range->syntax_start || start > range->syntax_end) return result;
    if (last && !(item->attrs->iFlags & XUI_DOC_TASK)) {
        uint64_t child_index;
        for (child_index = 0; child_index < doc_seq_size(item->children); child_index++)
            if (doc_seq_get_id(item->children, child_index) == paragraph->id) break;
        if (child_index + 1 < doc_seq_size(item->children)) {
            doc_node* next = doc_index_get(old->index, doc_seq_get_id(item->children, child_index + 1));
            doc_node_source_range next_range;
            if (!next) return result;
            doc_node_source_range_get(old, next, &next_range);
            for (i = 0; i < indent_count; i++) {
                xui_doc_list_indent_t indent = {0}; xui_doc_source_line_t line = {0}; uint64_t at, cut; char ch;
                indent.iSize = sizeof(indent);
                result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
                result = xuiDocumentSnapshotGetListContinuationIndent(snapshot, item->id, i, &indent);
                if (result != XUI_OK) return result;
                if (indent.iSourceStart < next_range.syntax_start || indent.iSourceEnd > next_range.source_start ||
                    indent.iContentColumn == UINT32_MAX || indent.iIndentEndColumn < indent.iContentColumn) continue;
                /* A blank item followed by a blank line closes in Markdown.
                 * Its first surviving block owns the new marker. Preserve the
                 * physical prefix before the parser's whitespace range; the
                 * shared prefix writer supplies the enclosing list/quote/
                 * footnote chain, while parser columns preserve excess indent
                 * (including a partly consumed Tab or an indented code block).
                 * No payload or nested block marker is serialized. */
                line.iSize = sizeof(line);
                result = xuiDocumentSnapshotGetSourceLine(snapshot, indent.iSourceStart, &line);
                if (result != XUI_OK) return result;
                if (indent.iSourceStart < line.iLineStart || indent.iSourceEnd > line.iContentEnd) continue;
                cut = indent.iSourceStart - line.iLineStart;
                if (cut > next_prefix->size) return XUI_ERROR_NOT_FOUND;
                for (at = 0; at < cut; at++) {
                    result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
                    result = doc_seq_read(old->source, line.iLineStart + at, &ch, 1);
                    if (result != XUI_OK) return result;
                    if (ch != next_prefix->data[at]) return XUI_ERROR_NOT_FOUND;
                }
                for (at = indent.iSourceStart; at < indent.iSourceEnd; at++) {
                    result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
                    result = doc_seq_read(old->source, at, &ch, 1);
                    if (result != XUI_OK) return result;
                    if (ch != ' ' && ch != '\t') return XUI_ERROR_NOT_FOUND;
                }
                carry_prefix_cut = cut; carry_extra = indent.iIndentEndColumn - indent.iContentColumn;
                carry_content_column = indent.iContentColumn;
                carry_start = indent.iSourceStart; carry_end = indent.iSourceEnd; break;
            }
            if (carry_start == DOC_NONE || marker_offset > next_prefix->size || marker_offset < carry_prefix_cut)
                return XUI_ERROR_NOT_FOUND;
        }
    }
    close.allocator = open.allocator = reference_close.allocator = output.allocator = carry.allocator = old->allocator;
    if (carry_start != DOC_NONE) {
        static const char spaces[] = "                ";
        doc_md_write(&carry, next_prefix->data + carry_prefix_cut, next_prefix->size - carry_prefix_cut);
        while (carry_extra && !carry.error) {
            uint64_t bytes = carry_extra < sizeof(spaces) - 1 ? carry_extra : sizeof(spaces) - 1;
            result = doc_txn_check(t, 0); if (result != XUI_OK) goto done;
            doc_md_write(&carry, spaces, bytes); carry_extra -= bytes;
        }
        if (carry.error) { result = carry.error; goto done; }
        result = doc_md_list_split_carry_delta(t, list, carry_start, carry_end, &carry,
            marker_offset - carry_prefix_cut, carry_content_column, &indent_delta);
        if (result != XUI_OK) goto done;
    }
    result = first || last ? XUI_OK : doc_md_inline_split_wrappers(t, start, start, &close, &open,
        &reference_close, &reference_start, &reference_end);
    if (result != XUI_OK) goto done;
    doc_md_write(&output, close.data, close.size); doc_md_write(&output, ending, ending_bytes);
    if (!(list->attrs->iFlags & XUI_DOC_TIGHT)) {
        doc_md_write(&output, continuation->data, continuation->size); doc_md_write(&output, ending, ending_bytes);
    }
    doc_md_write(&output, next_prefix->data, next_prefix->size); doc_md_write(&output, open.data, open.size);
    if (output.error) { result = output.error; goto done; }
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_md_list_split_tail_indent(&candidate, snapshot, item,
        carry_start == DOC_NONE ? start : carry_end, indent_count, indent_delta);
    /* The indent helper applies descending patches. Rebase the original
     * closing suffix through those exact operations before replacing it. */
    for (i = 0; result == XUI_OK && reference_start != DOC_NONE && i < candidate.count; i++) {
        const xui_doc_operation_t* op = candidate.ops + i;
        uint64_t old_end;
        if (op->iKind != XUI_DOC_OP_SOURCE || op->iOffset > UINT64_MAX - op->iOldLength) {
            result = XUI_ERROR_NOT_FOUND; break;
        }
        old_end = op->iOffset + op->iOldLength;
        if (reference_start < old_end && reference_end > op->iOffset) {
            result = XUI_ERROR_NOT_FOUND; break;
        }
        if (reference_start >= old_end) {
            if (reference_start - op->iOldLength > UINT64_MAX - op->iNewLength ||
                reference_end - op->iOldLength > UINT64_MAX - op->iNewLength) { result = XUI_DOC_ERROR_LIMIT; break; }
            reference_start = reference_start - op->iOldLength + op->iNewLength;
            reference_end = reference_end - op->iOldLength + op->iNewLength;
        }
    }
    if (result == XUI_OK && reference_start != DOC_NONE)
        result = doc_txn_source_patch(&candidate, reference_start, reference_end,
            reference_close.data, reference_close.size, 0);
    if (result == XUI_OK) {
        if (carry_start != DOC_NONE) result = doc_txn_source_patch(&candidate, carry_start, carry_end,
            carry.data, carry.size, 0);
        else result = doc_txn_source_patch(&candidate, start, start, output.data, output.size, 0);
    }
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_semantic_markdown_projection_equal(desired, candidate.draft)) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        result = doc_md_list_split_references(t, old, candidate.draft, start, item_range->syntax_end,
            indent_delta != 0, editable_start, editable_end, &references_equal);
        if (result == XUI_OK && !references_equal) result = XUI_ERROR_NOT_FOUND;
    }
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, candidate.ops + i);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft;
            candidate.draft = NULL; t->parse_op_start = t->count;
        }
    }
done:
    doc_state_release(candidate.draft); doc_free(candidate.ops);
    doc_free(close.data); doc_free(open.data); doc_free(reference_close.data); doc_free(output.data); doc_free(carry.data); return result;
}
/* A paragraph GAP immediately adjoining Text has the same source boundary
 * as that Text endpoint. Only the private source plan normalizes the position;
 * the semantic range, returned caret and history retain the caller's intent.
 * Object-to-object gaps remain owned by the inline deletion source planner. */
static int doc_md_range_text_endpoint(doc_state* state, xui_doc_position_t* position,
    int right_boundary)
{
    doc_node* parent; uint64_t count, option;
    if (position->iKind == XUI_DOC_POSITION_TEXT) return XUI_OK;
    if (position->iKind != XUI_DOC_POSITION_GAP) return XUI_ERROR_NOT_FOUND;
    parent = doc_index_get(state->index, position->iNodeId);
    if (!parent || (parent->kind != XUI_DOC_PARAGRAPH && parent->kind != XUI_DOC_HEADING))
        return XUI_ERROR_NOT_FOUND;
    count = doc_seq_size(parent->children);
    if (position->iOffset > count) return XUI_ERROR_NOT_FOUND;
    for (option = 0; option < 2; option++) {
        uint64_t index; doc_node* text;
        if ((right_boundary && !option) || (!right_boundary && option)) {
            if (!position->iOffset) continue;
            index = position->iOffset - 1;
        } else {
            if (position->iOffset == count) continue;
            index = position->iOffset;
        }
        text = doc_index_get(state->index, doc_seq_get_id(parent->children, index));
        if (!text || text->kind != XUI_DOC_TEXT) continue;
        position->iOffset = index == position->iOffset ? 0 : doc_seq_size(text->text);
        position->iNodeId = text->id; position->iKind = XUI_DOC_POSITION_TEXT;
        return XUI_OK;
    }
    return XUI_ERROR_NOT_FOUND;
}
/* Exact text endpoints can keep the spelling outside a replacement even when
 * it merges paragraphs. The same-parent path crosses sibling blocks separated
 * by blank container trivia or complete source-only definitions. The
 * cross-parent path uses the semantic branch plan; all source bytes outside
 * its text endpoints stay untouched. A private parse proves the full semantic
 * result before the patch reaches the transaction. */
static int doc_md_code_source_wrappers(xui_document_transaction t,
    doc_node* first, uint64_t start, uint64_t end, doc_state* desired,
    xui_doc_inline_syntax_t* owned, doc_md_output* open, doc_md_output* close)
{
    doc_state* old = t->draft; doc_node* parent = doc_index_get(old->index, first->parent);
    uint64_t count = doc_seq_size(old->inline_syntax) / sizeof(*owned), i, width = 0, closing = 0;
    uint64_t opening_end, closing_start, run = 0, maximum = 0, word_bytes = 0;
    unsigned first_char = 0, last_char = 0; int found = 0, collision = 0, non_space = 0, pad, result;
    char ch;
    for (i = 0; i < count; i++) {
        xui_doc_inline_syntax_t syntax;
        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
        result = doc_seq_read_syntax(old->inline_syntax, i, &syntax); if (result != XUI_OK) return result;
        if (syntax.iKind != XUI_DOC_SYNTAX_CODE || syntax.iContentStart > start || syntax.iContentEnd < end) continue;
        if (!found || (syntax.iSourceStart >= owned->iSourceStart && syntax.iSourceEnd <= owned->iSourceEnd)) {
            *owned = syntax; found = 1;
        }
    }
    if (!found || !parent || owned->iSourceStart >= owned->iContentStart ||
        owned->iContentEnd >= owned->iSourceEnd || owned->iSourceEnd > doc_seq_size(old->source))
        return XUI_ERROR_NOT_FOUND;
    opening_end = owned->iSourceStart;
    while (opening_end < owned->iContentStart) {
        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
        result = doc_seq_read(old->source, opening_end, &ch, 1); if (result != XUI_OK) return result;
        if (ch != '`') break;
        width++; opening_end++;
    }
    closing_start = owned->iSourceEnd;
    while (closing_start > owned->iContentEnd) {
        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
        result = doc_seq_read(old->source, closing_start - 1, &ch, 1); if (result != XUI_OK) return result;
        if (ch != '`') break;
        closing++; closing_start--;
    }
    if (!width || width != closing) return XUI_ERROR_NOT_FOUND;
    /* The CODE syntax can contain several Text callbacks. Scan the desired
     * logical word through their stable identities, including tick runs that
     * cross a callback boundary. Its existing raw payload stays in place. */
    for (i = 0; i < doc_seq_size(parent->children); i++) {
        uint64_t id = doc_seq_get_id(parent->children, i), at, size;
        doc_node* before = doc_index_get(old->index, id); doc_node* after;
        doc_node_source_range source;
        if (!before || before->kind != XUI_DOC_TEXT) continue;
        doc_node_source_range_get(old, before, &source);
        if (source.source_start < owned->iContentStart || source.source_end > owned->iContentEnd) continue;
        after = doc_index_get(desired->index, id);
        if (!after) continue;
        if (after->kind != XUI_DOC_TEXT || !(after->attrs->iMarks & XUI_DOC_CODE)) return XUI_ERROR_NOT_FOUND;
        size = doc_seq_size(after->text);
        for (at = 0; at < size;) {
            char part[256]; uint64_t n = size - at < sizeof(part) ? size - at : sizeof(part), j;
            result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
            result = doc_seq_read(after->text, at, part, n); if (result != XUI_OK) return result;
            for (j = 0; j < n; j++) {
                unsigned value = (unsigned char)part[j];
                if (!word_bytes) first_char = value;
                word_bytes++; last_char = value; non_space |= value != ' ';
                if (value == '`') run++;
                else {
                    if (run == width) collision = 1;
                    if (run > maximum) maximum = run;
                    run = 0;
                }
            }
            at += n;
        }
    }
    if (!word_bytes) return XUI_ERROR_NOT_FOUND;
    if (run == width) collision = 1;
    if (run > maximum) maximum = run;
    pad = first_char == '`' || last_char == '`' || (first_char == ' ' && last_char == ' ' && non_space);
    if (!collision && !(pad && (opening_end == owned->iContentStart || closing_start == owned->iContentEnd)) &&
        !(!non_space && (opening_end != owned->iContentStart || closing_start != owned->iContentEnd)))
        return XUI_ERROR_NOT_FOUND;
    if (collision) width = maximum + 1;
    for (i = 0; i < width; i++) doc_md_write(open, "`", 1);
    if (non_space && opening_end < owned->iContentStart) {
        result = doc_md_write_source_range(t, old, opening_end, owned->iContentStart, open);
        if (result != XUI_OK) return result;
    } else if (pad) doc_md_write(open, " ", 1);
    if (non_space && closing_start > owned->iContentEnd) {
        result = doc_md_write_source_range(t, old, owned->iContentEnd, closing_start, close);
        if (result != XUI_OK) return result;
    } else if (pad) doc_md_write(close, " ", 1);
    for (i = 0; i < width; i++) doc_md_write(close, "`", 1);
    return open->error ? open->error : close->error;
}
/* A paragraph boundary is outside its terminal/opening inline wrappers.
 * TEXT endpoint maps point inside those wrappers. Advance only through exact
 * parser-owned closing suffixes, and retreat through exact opening prefixes;
 * the complete candidate parse still decides delimiter interactions. */
static int doc_md_range_join_frontier(xui_document_transaction t,
    doc_node* first, doc_node* last, doc_node* first_block, doc_node* last_block,
    const xui_doc_position_t* a, const xui_doc_position_t* b,
    uint64_t* start, uint64_t* end)
{
    uint64_t count, pass, i;
    if (first_block == last_block || a->iOffset != doc_seq_size(first->text) || b->iOffset ||
        doc_seq_get_id(first_block->children, doc_seq_size(first_block->children) - 1) != first->id ||
        doc_seq_get_id(last_block->children, 0) != last->id) return XUI_OK;
    count = doc_seq_size(t->draft->inline_syntax) / sizeof(xui_doc_inline_syntax_t);
    for (pass = 0; pass <= count; pass++) {
        int changed = 0;
        for (i = 0; i < count; i++) {
            xui_doc_inline_syntax_t syntax;
            int result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
            result = doc_seq_read_syntax(t->draft->inline_syntax, i, &syntax);
            if (result != XUI_OK) return result;
            if (syntax.iKind == XUI_DOC_SYNTAX_DESTINATION || syntax.iKind == XUI_DOC_SYNTAX_TITLE)
                continue;
            if (syntax.iContentEnd == *start && syntax.iSourceEnd > *start && syntax.iSourceEnd <= *end) {
                *start = syntax.iSourceEnd; changed = 1;
            }
            if (syntax.iContentStart == *end && syntax.iSourceStart < *end && syntax.iSourceStart >= *start) {
                *end = syntax.iSourceStart; changed = 1;
            }
        }
        if (!changed) return XUI_OK;
    }
    return XUI_ERROR_NOT_FOUND;
}
/* Joining blocks retains the left heading's parser-owned scaffold, but the
 * surviving block ends where the right block ended, including its exact EOL
 * or lack of an EOF EOL. Internal Setext endings remain untouched. */
static int doc_md_range_heading_join_suffix(xui_document_transaction t,
    doc_node* first, doc_node* last, doc_node* first_block, doc_node* last_block,
    const xui_doc_position_t* a, const xui_doc_position_t* b,
    uint64_t start, uint64_t end, uint64_t* suffix_start, uint64_t* suffix_end,
    doc_md_output* suffix, int allow_partial)
{
    struct xui_doc_snapshot_t snapshot = {0};
    xui_doc_block_syntax_t left_syntax = {0}, right_syntax = {0};
    xui_doc_source_line_t left_line = {0}, right_line = {0};
    doc_node_source_range left_range, right_range;
    uint64_t body_end; int result;
    if (first_block == last_block || first_block->parent != last_block->parent ||
        (first_block->kind != XUI_DOC_HEADING && last_block->kind != XUI_DOC_HEADING) ||
        (!allow_partial && (a->iOffset != doc_seq_size(first->text) || b->iOffset ||
         doc_seq_get_id(first_block->children, doc_seq_size(first_block->children) - 1) != first->id ||
         doc_seq_get_id(last_block->children, 0) != last->id)))
        return XUI_ERROR_NOT_FOUND;
    snapshot.state = t->draft; left_syntax.iSize = sizeof(left_syntax);
    right_syntax.iSize = sizeof(right_syntax);
    left_line.iSize = sizeof(left_line); right_line.iSize = sizeof(right_line);
    doc_node_source_range_get(t->draft, first_block, &left_range);
    doc_node_source_range_get(t->draft, last_block, &right_range);
    if (left_range.syntax_start == DOC_NONE || left_range.syntax_end == DOC_NONE ||
        right_range.syntax_start == DOC_NONE || right_range.syntax_end == DOC_NONE ||
        left_range.syntax_start >= left_range.syntax_end ||
        left_range.syntax_end > right_range.syntax_start ||
        right_range.syntax_start >= right_range.syntax_end ||
        right_range.syntax_end > doc_seq_size(t->draft->source)) return XUI_ERROR_NOT_FOUND;
    if (first_block->kind == XUI_DOC_HEADING) {
        result = xuiDocumentSnapshotGetBlockSyntax(&snapshot, first_block->id, &left_syntax);
        if (result != XUI_OK) return result;
        if ((!allow_partial && start != left_syntax.iHeadingContentEnd) ||
            start < left_syntax.iHeadingContentStart || start > left_syntax.iHeadingContentEnd)
            return XUI_ERROR_NOT_FOUND;
    }
    result = xuiDocumentSnapshotGetSourceLine(&snapshot, right_range.syntax_end - 1, &right_line);
    if (result != XUI_OK) return result;
    if (last_block->kind == XUI_DOC_HEADING) {
        result = xuiDocumentSnapshotGetBlockSyntax(&snapshot, last_block->id, &right_syntax);
        if (result != XUI_OK) return result;
        if ((!allow_partial && end != right_syntax.iHeadingContentStart) ||
            end < right_syntax.iHeadingContentStart || end > right_syntax.iHeadingContentEnd)
            return XUI_ERROR_NOT_FOUND;
        body_end = right_syntax.iHeadingContentEnd;
    } else body_end = right_line.iContentEnd;
    if (body_end < end || body_end > right_range.syntax_end ||
        right_line.iContentEnd < body_end || right_line.iContentEnd > right_range.syntax_end)
        return XUI_ERROR_NOT_FOUND;
    *suffix_start = body_end; *suffix_end = right_range.syntax_end;
    if (first_block->kind == XUI_DOC_HEADING) {
        result = xuiDocumentSnapshotGetSourceLine(&snapshot, left_range.syntax_end - 1, &left_line);
        if (result != XUI_OK) return result;
        if (left_line.iContentEnd < left_syntax.iHeadingContentEnd ||
            left_line.iContentEnd > left_range.syntax_end) return XUI_ERROR_NOT_FOUND;
        result = doc_md_write_source_range(t, t->draft, left_syntax.iHeadingContentEnd,
            left_line.iContentEnd, suffix);
        if (result != XUI_OK) return result;
    }
    /* Closing hashes or the underline belong to the left heading. The final
     * ending belongs to the right block; a surviving paragraph has no scaffold. */
    return doc_md_write_source_range(t, t->draft, right_line.iContentEnd,
        right_range.syntax_end, suffix);
}

/* A deletion can expose literal whitespace to block trimming or delimiter
 * flanking. Only replace an exact, unchanged scalar next to the selected
 * source range. The caller reparses and proves the complete semantic intent. */
static int doc_md_delete_boundary_escape(xui_document_transaction t,
    xui_document_transaction candidate, doc_node* node, uint64_t offset,
    uint64_t source_boundary, uint64_t candidate_boundary, int before,
    int* changed)
{
    uint64_t first = offset, last = offset, raw_first, raw_last, width;
    int left_quality, right_quality, result;
    char logical[4], raw[4]; uint32_t scalar;
    doc_md_output output = {0};
    *changed = 0;
    if (!node || node->kind != XUI_DOC_TEXT ||
        (node->attrs->iMarks & XUI_DOC_CODE)) return XUI_OK;
    if (before) {
        if (!first) return XUI_OK;
        first--;
        while (first && !doc_seq_boundary(node->text, first)) first--;
    } else {
        if (last >= doc_seq_size(node->text)) return XUI_OK;
        last++;
        while (last < doc_seq_size(node->text) && !doc_seq_boundary(node->text, last)) last++;
    }
    width = last - first;
    if (!width || width > sizeof(logical)) return XUI_OK;
    result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
    result = doc_seq_read(node->text, first, logical, width);
    if (result != XUI_OK) return result;
    scalar = doc_md_scalar_value(logical, width);
    if (scalar == '\r' || scalar == '\n' || !doc_md4c_unicode_whitespace(scalar)) return XUI_OK;
    if (doc_source_map_text(t->draft, node, first, XUI_DOC_AFTER, &raw_first, &left_quality) != XUI_OK ||
        doc_source_map_text(t->draft, node, last, XUI_DOC_BEFORE, &raw_last, &right_quality) != XUI_OK ||
        left_quality != XUI_DOC_MAP_EXACT || right_quality != XUI_DOC_MAP_EXACT ||
        raw_last < raw_first || raw_last - raw_first != width ||
        (before ? raw_last != source_boundary : raw_first != source_boundary)) return XUI_OK;
    result = doc_seq_read(t->draft->source, raw_first, raw, width);
    if (result != XUI_OK) return result;
    if (memcmp(logical, raw, (size_t)width)) return XUI_OK;
    output.allocator = t->draft->allocator;
    result = doc_md_write_entity_slice(&output, node, first, last);
    if (result == XUI_OK) {
        uint64_t at = before ? raw_first : candidate_boundary;
        result = doc_txn_source_patch(candidate, at, at + width, output.data, output.size, 0);
    }
    doc_free(output.data);
    if (result == XUI_OK) *changed = 1;
    return result;
}
static int doc_markdown_range_source_patch(xui_document_transaction t,
    const xui_doc_range_t* range, const char* text, uint64_t bytes, doc_state* desired)
{
    doc_state* old = t->draft;
    xui_doc_position_t a = range->tAnchor, b = range->tCaret;
    doc_node *first, *last, *first_block, *last_block, *parent;
    struct xui_doc_transaction_t candidate = {0};
    doc_md_output output = {0}, moved = {0}, prelude = {0};
    doc_md_output inline_close = {0}, inline_open = {0}, reference_close = {0};
    doc_md_output code_open = {0}, code_close = {0}, heading_close = {0};
    doc_md_output cross_close = {0}, cross_open = {0}; int cross_inline_adjust = 0;
    xui_doc_inline_syntax_t code_syntax = {0}; int code_adjust = 0, heading_adjust = 0;
    uint64_t heading_start = DOC_NONE, heading_end = DOC_NONE, join_insert_at = DOC_NONE;
    uint64_t reference_start = DOC_NONE, reference_end = DOC_NONE;
    xui_doc_source_segment_t first_entity = {0}, last_entity = {0};
    uint64_t start, end, i, first_index, last_index, move_at = 0;
    uint64_t editable_footnote_start = DOC_NONE, editable_footnote_end = DOC_NONE;
    const char* line_ending = "\n";
    uint64_t line_ending_bytes = 1;
    doc_md_output container_storage = {0}, raw_storage = {0};
    char* container_prefix = NULL;
    uint64_t container_prefix_bytes = 0, container_blank_bytes = 0;
    int order, left_quality, right_quality, cross_block, first_inside = 0,
        last_inside = 0, result = XUI_ERROR_NOT_FOUND;
    if (doc_position_compare(old, &a, &b, &order) != XUI_OK) return result;
    if (order > 0) { xui_doc_position_t swap = a; a = b; b = swap; }
    if (doc_md_range_text_endpoint(old, &a, 0) != XUI_OK ||
        doc_md_range_text_endpoint(old, &b, 1) != XUI_OK)
        return result;
    first = doc_index_get(old->index, a.iNodeId);
    last = doc_index_get(old->index, b.iNodeId);
    if (!first || !last || first->kind != XUI_DOC_TEXT || last->kind != XUI_DOC_TEXT)
        return result;
    first_block = doc_index_get(old->index, first->parent);
    last_block = doc_index_get(old->index, last->parent);
    if (!first_block || !last_block ||
        (first_block->kind != XUI_DOC_PARAGRAPH && first_block->kind != XUI_DOC_HEADING && first_block->kind != XUI_DOC_CELL) ||
        (last_block->kind != XUI_DOC_PARAGRAPH && last_block->kind != XUI_DOC_HEADING && last_block->kind != XUI_DOC_CELL))
        return result;
    cross_block = first_block != last_block;
    if (cross_block && first_block->parent != last_block->parent) {
        doc_range_branch_plan branch_plan;
        doc_node* current;
        if (doc_range_branch_plan_get(old, first_block, last_block,
            &branch_plan) != XUI_OK)
            return result;
        moved.allocator = old->allocator;
        prelude.allocator = old->allocator;
        current = first_block;
        while (current->id != branch_plan.left_child) {
            doc_node* container = doc_index_get(old->index, current->parent);
            uint64_t unused_start, current_end, container_end;
            if (!container ||
                doc_md_bounds(old, current->id, &unused_start, &current_end) != XUI_OK ||
                doc_md_bounds(old, container->id, &unused_start, &container_end) != XUI_OK ||
                current_end > container_end) break;
            result = doc_md_source_gap_trivia(t, old, current_end,
                container_end, container->kind == XUI_DOC_QUOTE);
            if (result == XUI_ERROR_NOT_FOUND)
                result = doc_md_source_gap_references(t, old, current_end,
                    container_end, container->kind == XUI_DOC_QUOTE,
                    DOC_NONE, DOC_NONE, &moved);
            if (result != XUI_OK && result != XUI_ERROR_NOT_FOUND) goto done;
            current = container;
        }
        if (branch_plan.ancestor == DOC_ROOT) {
            parent = doc_index_get(old->index, DOC_ROOT);
            first_index = doc_child_index(parent, branch_plan.left_child);
            last_index = doc_child_index(parent, branch_plan.right_child);
            for (i = first_index; i < last_index; i++) {
                doc_node* current = doc_index_get(old->index,
                    doc_seq_get_id(parent->children, i));
                doc_node* next = doc_index_get(old->index,
                    doc_seq_get_id(parent->children, i + 1));
                uint64_t current_start, current_end, next_start, next_end;
                result = doc_txn_check(t, 0);
                if (result != XUI_OK) goto done;
                if (!current || !next ||
                    doc_md_bounds(old, current->id, &current_start, &current_end) != XUI_OK ||
                    doc_md_bounds(old, next->id, &next_start, &next_end) != XUI_OK ||
                    current_end > next_start) goto done;
                result = doc_md_source_gap_trivia(t, old, current_end,
                    next_start, 0);
                if (result == XUI_ERROR_NOT_FOUND)
                    result = doc_md_source_gap_references(t, old,
                        current_end, next_start, 0, DOC_NONE, DOC_NONE, &moved);
                if (result != XUI_OK) goto done;
            }
        }
        current = last_block;
        while (current->id != branch_plan.right_child) {
            doc_node* container = doc_index_get(old->index, current->parent);
            uint64_t container_start, unused_end, current_start;
            if (!container ||
                doc_md_bounds(old, container->id, &container_start, &unused_end) != XUI_OK ||
                doc_md_bounds(old, current->id, &current_start, &unused_end) != XUI_OK ||
                container_start > current_start) break;
            result = doc_md_source_gap_trivia(t, old, container_start,
                current_start, container->kind == XUI_DOC_QUOTE);
            if (result == XUI_ERROR_NOT_FOUND &&
                container->kind == XUI_DOC_LIST &&
                current->kind == XUI_DOC_LIST_ITEM &&
                container->id != branch_plan.right_child &&
                doc_md_nested_path_list(desired, container, current->id) &&
                doc_child_index(doc_index_get(desired->index, container->id),
                    current->id) > 0) {
                /* A retained source-only sibling belongs to the list scaffold.
                 * Copying its definition alone would consume that scaffold. */
                result = XUI_OK;
            }
            if (result == XUI_ERROR_NOT_FOUND)
                result = doc_md_source_gap_references(t, old, container_start,
                    current_start, container->kind == XUI_DOC_QUOTE,
                    DOC_NONE, DOC_NONE, &moved);
            if (result != XUI_OK && result != XUI_ERROR_NOT_FOUND) goto done;
            current = container;
        }
        result = doc_md_nested_empty_quote_scaffold(t, old, desired,
            &branch_plan, last_block, last, &moved, &prelude);
        if (result != XUI_OK) goto done;
        if (prelude.size) {
            doc_md_output combined = {0};
            if (!moved.size) { result = XUI_ERROR_NOT_FOUND; goto done; }
            combined.allocator = old->allocator;
            doc_md_write(&combined, prelude.data, prelude.size);
            doc_md_write(&combined, moved.data, moved.size);
            if (combined.error) {
                result = combined.error; doc_free(combined.data); goto done;
            }
            doc_free(moved.data);
            moved = combined;
        }
        if (moved.size) {
            uint64_t unused;
            result = doc_md_bounds(old, last_block->id, &unused, &move_at);
            if (result != XUI_OK) goto done;
        }
        result = XUI_ERROR_NOT_FOUND;
    } else if (cross_block) {
        int quote_markers;
        parent = doc_index_get(old->index, first_block->parent);
        if (!parent) return result;
        {
            doc_node* ancestor = parent;
            while (ancestor && ancestor->id != DOC_ROOT) {
                if (ancestor->kind == XUI_DOC_FOOTNOTE) {
                    doc_node_source_range footnote_range;
                    doc_node_source_range_get(old, ancestor, &footnote_range);
                    if (footnote_range.syntax_start != DOC_NONE &&
                        footnote_range.syntax_end != DOC_NONE &&
                        footnote_range.syntax_start <= footnote_range.syntax_end &&
                        footnote_range.syntax_end <= doc_seq_size(old->source)) {
                        editable_footnote_start = footnote_range.syntax_start;
                        editable_footnote_end = footnote_range.syntax_end;
                    }
                    break;
                }
                ancestor = doc_index_get(old->index, ancestor->parent);
            }
        }
        quote_markers = 0;
        {
            doc_node* ancestor = parent;
            while (ancestor && ancestor->id != DOC_ROOT) {
                if (ancestor->kind == XUI_DOC_QUOTE) quote_markers = 1;
                ancestor = doc_index_get(old->index, ancestor->parent);
            }
        }
        if (!quote_markers && parent->kind == XUI_DOC_FOOTNOTE) {
            doc_node_source_range footnote_range;
            uint64_t cursor;
            doc_node_source_range_get(old, parent, &footnote_range);
            /* A root Footnote can own an indented quote opener without a
             * semantic Quote ancestor. Its first non-space source byte tells
             * whether blank continuation lines contain an explicit marker. */
            if (footnote_range.syntax_start != DOC_NONE &&
                footnote_range.syntax_end != DOC_NONE &&
                footnote_range.syntax_start <= footnote_range.syntax_end &&
                footnote_range.syntax_end <= doc_seq_size(old->source)) {
                for (cursor = footnote_range.syntax_start;
                    cursor < footnote_range.syntax_end &&
                    cursor - footnote_range.syntax_start < 256; cursor++) {
                    char opener;
                    if (doc_seq_read(old->source, cursor, &opener, 1) != XUI_OK)
                        break;
                    if (opener == ' ' || opener == '\t') continue;
                    if (opener == '>') quote_markers = 1;
                    break;
                }
            }
        }
        moved.allocator = old->allocator;
        first_index = parent ? doc_child_index(parent, first_block->id) : DOC_NONE;
        last_index = parent ? doc_child_index(parent, last_block->id) : DOC_NONE;
        if (first_index == DOC_NONE || last_index == DOC_NONE || first_index >= last_index)
            return result;
        for (i = first_index; i < last_index; i++) {
            doc_node* current = doc_index_get(old->index,
                doc_seq_get_id(parent->children, i));
            doc_node* next = doc_index_get(old->index,
                doc_seq_get_id(parent->children, i + 1));
            uint64_t current_start, current_end, next_start, next_end;
            if (!current || !next ||
                (current->kind != XUI_DOC_PARAGRAPH && current->kind != XUI_DOC_HEADING) ||
                (next->kind != XUI_DOC_PARAGRAPH && next->kind != XUI_DOC_HEADING) ||
                doc_md_bounds(old, current->id, &current_start, &current_end) != XUI_OK ||
                doc_md_bounds(old, next->id, &next_start, &next_end) != XUI_OK ||
                current_end > next_start) { result = XUI_ERROR_NOT_FOUND; goto done; }
            result = doc_md_source_gap_trivia(t, old,
                current_end, next_start, quote_markers);
            if (result == XUI_ERROR_NOT_FOUND)
                result = doc_md_source_gap_references(t, old,
                    current_end, next_start, quote_markers,
                    editable_footnote_start, editable_footnote_end, &moved);
            if (result != XUI_OK) goto done;
        }
        if (moved.size) {
            uint64_t unused;
            if (parent->id != DOC_ROOT && parent->kind != XUI_DOC_LIST_ITEM &&
                parent->kind != XUI_DOC_QUOTE && parent->kind != XUI_DOC_FOOTNOTE) {
                result = XUI_ERROR_NOT_FOUND; goto done;
            }
            result = doc_md_bounds(old, last_block->id, &unused, &move_at);
            if (result != XUI_OK) goto done;
        }
        result = XUI_ERROR_NOT_FOUND;
    }
    if (doc_source_map_text(old, first, a.iOffset, XUI_DOC_AFTER, &start, &left_quality) != XUI_OK ||
        doc_source_map_text(old, last, b.iOffset, XUI_DOC_BEFORE, &end, &right_quality) != XUI_OK)
        goto done;
    if (left_quality == XUI_DOC_MAP_COLLAPSED) {
        if (doc_md_range_entity_at(old, first, a.iOffset, &first_entity) != XUI_OK)
            goto done;
        start = first_entity.iSourceStart; first_inside = 1;
    } else if (left_quality != XUI_DOC_MAP_EXACT) goto done;
    if (right_quality == XUI_DOC_MAP_COLLAPSED) {
        if (doc_md_range_entity_at(old, last, b.iOffset, &last_entity) != XUI_OK)
            goto done;
        end = last_entity.iSourceEnd; last_inside = 1;
    } else if (right_quality != XUI_DOC_MAP_EXACT) goto done;
    if (start > end || end > doc_seq_size(old->source)) goto done;
    if (cross_block) {
        /* Nonempty replacement at a block seam types into the left terminal
         * run. Keep this parser-mapped position inside its existing wrappers;
         * delete the structural seam outside them in a separate source op. */
        if (bytes && !memchr(text, '\n', (size_t)bytes) && !memchr(text, '\r', (size_t)bytes) &&
            a.iOffset == doc_seq_size(first->text) && !b.iOffset &&
            doc_seq_get_id(first_block->children, doc_seq_size(first_block->children) - 1) == first->id &&
            doc_seq_get_id(last_block->children, 0) == last->id)
            join_insert_at = start;
        if (join_insert_at == DOC_NONE && first_block->parent == last_block->parent &&
            (bytes || a.iOffset != doc_seq_size(first->text) || b.iOffset ||
             doc_seq_get_id(first_block->children, doc_seq_size(first_block->children) - 1) != first->id ||
             doc_seq_get_id(last_block->children, 0) != last->id) &&
            (!bytes || (!memchr(text, '\n', (size_t)bytes) && !memchr(text, '\r', (size_t)bytes)))) {
            doc_md_output discarded_close = {0}, discarded_open = {0}, discarded_reference = {0};
            uint64_t ignored_start = DOC_NONE, ignored_end = DOC_NONE;
            cross_close.allocator = cross_open.allocator = old->allocator;
            reference_close.allocator = discarded_close.allocator = discarded_open.allocator =
                discarded_reference.allocator = old->allocator;
            /* Preserve the left endpoint's concrete closing chain and the
             * right endpoint's opening chain; intervening source is selected.
             * A changed right shortcut label needs its original definition key. */
            result = doc_md_inline_split_wrappers(t, start, start, &cross_close, &discarded_open,
                &discarded_reference, &ignored_start, &ignored_end);
            if (result == XUI_OK)
                result = doc_md_inline_split_wrappers(t, end, end, &discarded_close, &cross_open,
                    &reference_close, &reference_start, &reference_end);
            doc_free(discarded_close.data); doc_free(discarded_open.data); doc_free(discarded_reference.data);
            if (result != XUI_OK) goto done;
            cross_inline_adjust = 1;
        }
        result = doc_md_range_join_frontier(t, first, last, first_block, last_block,
            &a, &b, &start, &end);
        if (result != XUI_OK) goto done;
        if (!bytes || join_insert_at != DOC_NONE || cross_inline_adjust) {
            heading_close.allocator = old->allocator;
            result = doc_md_range_heading_join_suffix(t, first, last, first_block, last_block,
                &a, &b, start, end, &heading_start, &heading_end, &heading_close, cross_inline_adjust);
            if (result == XUI_OK) heading_adjust = 1;
            else if (result != XUI_ERROR_NOT_FOUND) goto done;
            result = XUI_OK;
        }
    }

    /* An entire Text run can own format/code/link delimiters. The same parser
     * ownership used by node deletion consumes only its exclusive wrappers. */
    if (first == last && !bytes && !a.iOffset && b.iOffset == doc_seq_size(first->text)) {
        doc_node_source_range owned;
        doc_node_source_range_get(old, first, &owned);
        result = doc_md_inline_owned_source_range(t, owned.source_start, owned.source_end,
            1, &start, &end);
        if (result != XUI_OK) goto done;
    }
    inline_close.allocator = inline_open.allocator = old->allocator;
    reference_close.allocator = old->allocator;
    if (first == last && ((first->attrs->iMarks & XUI_DOC_LINK) ||
        (first->attrs->iMarks && bytes &&
            (memchr(text, '\n', (size_t)bytes) || memchr(text, '\r', (size_t)bytes))))) {
        result = doc_md_inline_split_wrappers(t, start, end, &inline_close, &inline_open,
            &reference_close, &reference_start, &reference_end);
        if (result != XUI_OK) goto done;
    }
    if (bytes && (memchr(text, '\n', (size_t)bytes) ||
        memchr(text, '\r', (size_t)bytes))) {
        uint64_t unused, block_end;
        uint64_t quote_depth = 0, list_depth = 0;
        doc_node* ancestor = first_block;
        doc_node* list_item = NULL;
        doc_node* footnote = NULL;
        char ending;
        result = doc_md_bounds(old, first_block->id, &unused, &block_end);
        /* A source-less cell paragraph needs the semantic serializer to
         * report profile representability, rather than leaking this private
         * source-plan capability result through the public command. */
        if (result == XUI_ERROR_UNSUPPORTED) result = XUI_ERROR_NOT_FOUND;
        if (result != XUI_OK) goto done;
        if (block_end) {
            result = doc_seq_read(old->source, block_end - 1, &ending, 1);
            if (result != XUI_OK) goto done;
            if (ending == '\r') line_ending = "\r";
            else if (ending == '\n' && block_end > 1) {
                result = doc_seq_read(old->source, block_end - 2, &ending, 1);
                if (result != XUI_OK) goto done;
                if (ending == '\r') {
                    line_ending = "\r\n";
                    line_ending_bytes = 2;
                }
            }
        }
        while (ancestor && ancestor->id != DOC_ROOT) {
            if (ancestor->kind == XUI_DOC_QUOTE) quote_depth++;
            if (ancestor->kind == XUI_DOC_LIST_ITEM) {
                list_depth++;
                if (!list_item) list_item = ancestor;
            }
            if (!footnote && ancestor->kind == XUI_DOC_FOOTNOTE)
                footnote = ancestor;
            ancestor = doc_index_get(old->index, ancestor->parent);
        }
        if (quote_depth || list_item || footnote) {
            /* Reuse parser-owned quote markers and the nearest list item's
             * content indent. Complex openers take the checked fallback. */
            doc_node_source_range block_range;
            char* raw_prefix;
            uint64_t line_start, raw_bytes, at, markers = 0;
            doc_node_source_range_get(old, first_block, &block_range);
            if (block_range.source_start == DOC_NONE ||
                block_range.source_start > doc_seq_size(old->source)) {
                result = XUI_ERROR_NOT_FOUND; goto done;
            }
            line_start = block_range.source_start;
            while (line_start) {
                char scan[4096];
                uint64_t count = line_start < sizeof(scan) ? line_start : sizeof(scan);
                uint64_t scan_start = line_start - count, scan_at;
                result = doc_txn_check(t, 0);
                if (result != XUI_OK) goto done;
                result = doc_seq_read(old->source, scan_start, scan, count);
                if (result != XUI_OK) goto done;
                for (scan_at = count; scan_at; scan_at--)
                    if (scan[scan_at - 1] == '\r' || scan[scan_at - 1] == '\n') break;
                line_start = scan_start + scan_at;
                if (scan_at) break;
            }
            /* A file BOM is outside the physical container prefix. Leaving
             * it in raw_prefix would reject a valid quote/list opener and
             * move untouched definitions through paragraph serialization. */
            if (!line_start && block_range.source_start >= 3) {
                char bom[3];
                result = doc_seq_read(old->source, 0, bom, 3);
                if (result != XUI_OK) goto done;
                if (!memcmp(bom, "\xef\xbb\xbf", 3)) line_start = 3;
            }
            raw_bytes = block_range.source_start - line_start;
            if (!raw_bytes) {
                result = XUI_ERROR_NOT_FOUND; goto done;
            }
            raw_storage.allocator = container_storage.allocator = old->allocator;
            result = doc_md_write_source_range(t, old, line_start,
                block_range.source_start, &raw_storage);
            if (result != XUI_OK) goto done;
            raw_prefix = raw_storage.data;
            if (list_depth > 1 || inline_open.size ||
                (footnote && (list_item || quote_depth))) {
                /* Every nested list opener is already parser-confirmed.
                 * Reuse the paragraph writer's complete container chain;
                 * recognizing only the first opener leaves later markers
                 * behind and forces an unnecessary whole-block rewrite. */
                doc_node_source_range prefix_range = block_range;
                doc_md_output first_prefix = {0}, continuation = {0};
                first_prefix.allocator = continuation.allocator = old->allocator;
                prefix_range.syntax_start = line_start;
                result = doc_md_paragraph_prefixes(t, first_block, &prefix_range,
                    raw_prefix, raw_bytes, &first_prefix, &continuation, 0, NULL);
                if (result == XUI_OK && first_prefix.size <= raw_bytes &&
                    (first_prefix.size == raw_bytes ||
                        (raw_bytes - first_prefix.size == inline_open.size &&
                         !memcmp(raw_prefix + first_prefix.size, inline_open.data,
                            (size_t)inline_open.size))) &&
                    continuation.size) {
                    container_storage = continuation;
                    continuation.data = NULL;
                } else if (result == XUI_OK) result = XUI_ERROR_NOT_FOUND;
                doc_free(first_prefix.data); doc_free(continuation.data);
                if (result != XUI_OK) goto done;
            } else {
                at = 0;
                for (;;) {
                    while (at < raw_bytes && (raw_prefix[at] == ' ' ||
                        raw_prefix[at] == '\t')) at++;
                    if (at >= raw_bytes || raw_prefix[at] != '>') break;
                    at++;
                }
                if (list_item && at < raw_bytes &&
                    (raw_prefix[at] == '-' || raw_prefix[at] == '+' ||
                        raw_prefix[at] == '*' ||
                        (raw_prefix[at] >= '0' && raw_prefix[at] <= '9'))) {
                    uint64_t continuation, marker_end, columns = 0;
                    uint64_t marker_start_column = 0, marker_width, j;
                    int task = !!(list_item->attrs->iFlags & XUI_DOC_TASK);
                    marker_end = doc_md_list_marker_prefix(raw_prefix, raw_bytes,
                        at, task, &continuation);
                    if (marker_end == at) {
                        result = XUI_ERROR_NOT_FOUND; goto done;
                    }
                    for (j = 0; j < continuation; j++) {
                        if (j == at) marker_start_column = columns;
                        columns = raw_prefix[j] == '\t' ?
                            (columns + 4) & ~(uint64_t)3 : columns + 1;
                    }
                    marker_width = columns - marker_start_column;
                    doc_md_write(&container_storage, raw_prefix, at);
                    for (j = 0; j < marker_width && !container_storage.error; j++) {
                        if (!(j & 4095)) {
                            result = doc_txn_check(t, 0);
                            if (result != XUI_OK) goto done;
                        }
                        doc_md_write(&container_storage, " ", 1);
                    }
                    doc_md_write(&container_storage,
                        raw_prefix + marker_end,
                        raw_bytes - marker_end);
                } else if (footnote && !list_item && at + 2 < raw_bytes &&
                    raw_prefix[at] == '[' && raw_prefix[at + 1] == '^') {
                    uint64_t close = at + 2, j;
                    while (close + 1 < raw_bytes &&
                        (raw_prefix[close] != ']' || raw_prefix[close + 1] != ':'))
                        close++;
                    if (close + 2 >= raw_bytes) {
                        result = XUI_ERROR_NOT_FOUND; goto done;
                    }
                    for (j = close + 2; j < raw_bytes; j++)
                        if (raw_prefix[j] != ' ' && raw_prefix[j] != '\t') {
                            result = XUI_ERROR_NOT_FOUND; goto done;
                        }
                    doc_md_write(&container_storage, raw_prefix, at);
                    doc_md_write(&container_storage, "    ", 4);
                } else {
                    doc_md_write(&container_storage, raw_prefix, raw_bytes);
                }
            }
            if (container_storage.error) { result = container_storage.error; goto done; }
            container_prefix = container_storage.data;
            container_prefix_bytes = container_storage.size;
            for (at = 0; at < container_prefix_bytes; at++) {
                if (container_prefix[at] == '>') markers++;
                else if (container_prefix[at] != ' ' &&
                    container_prefix[at] != '\t') {
                    result = XUI_ERROR_NOT_FOUND; goto done;
                }
            }
            /* MD4C can attach a quote-prefixed footnote definition directly
             * to the root Footnote node, without a Quote ancestor. Keep its
             * verified source markers and let the candidate parse decide. */
            if (markers != quote_depth &&
                !(footnote && !list_item && markers > quote_depth)) {
                result = XUI_ERROR_NOT_FOUND; goto done;
            }
            if (markers) {
                container_blank_bytes = container_prefix_bytes;
                while (container_blank_bytes &&
                    (container_prefix[container_blank_bytes - 1] == ' ' ||
                        container_prefix[container_blank_bytes - 1] == '\t'))
                    container_blank_bytes--;
            }
        }
    }
    output.allocator = old->allocator;
    if (first_inside) {
        result = doc_md_write_entity_slice(&output, first,
            first_entity.iTextStart, a.iOffset);
        if (result != XUI_OK) goto done;
    }
    for (i = 0; i < bytes && !output.error;) {
        uint64_t width = doc_md_scalar_end(text + i, bytes - i);
        uint32_t scalar = doc_md_scalar_value(text + i, width);
        if (scalar == '\r' || scalar == '\n') {
            /* ReplaceRange splits a paragraph at each input line ending.
             * Markdown needs a blank separator for the same semantic split. */
            doc_md_write(&output, inline_close.data, inline_close.size);
            doc_md_write(&output, line_ending, line_ending_bytes);
            if (container_blank_bytes)
                doc_md_write(&output, container_prefix, container_blank_bytes);
            doc_md_write(&output, line_ending, line_ending_bytes);
            if (container_prefix_bytes)
                doc_md_write(&output, container_prefix, container_prefix_bytes);
            doc_md_write(&output, inline_open.data, inline_open.size);
            i += width;
            if (scalar == '\r' && i < bytes && text[i] == '\n') i++;
            continue;
        }
        if ((first == last || join_insert_at != DOC_NONE || cross_inline_adjust) && (first->attrs->iMarks & XUI_DOC_CODE))
            doc_md_write(&output, text + i, width);
        else if ((i == 0 || i + width == bytes || text[i - 1] == '\n' ||
                text[i - 1] == '\r' || text[i + width] == '\n' ||
                text[i + width] == '\r') &&
            doc_md4c_unicode_whitespace(scalar))
            doc_md_write_entity(&output, text + i, width);
        else doc_md_escape(&output, text + i, width);
        i += width;
    }
    if (last_inside && !cross_inline_adjust) {
        result = doc_md_write_entity_slice(&output, last,
            b.iOffset, last_entity.iTextEnd);
        if (result != XUI_OK) goto done;
    }
    if (output.error) { result = output.error; goto done; }
    code_open.allocator = code_close.allocator = old->allocator;
    if ((first == last || join_insert_at != DOC_NONE || cross_inline_adjust) && (first->attrs->iMarks & XUI_DOC_CODE) &&
        (!bytes || (!memchr(text, '\n', (size_t)bytes) && !memchr(text, '\r', (size_t)bytes)))) {
        result = doc_md_code_source_wrappers(t, first,
            join_insert_at != DOC_NONE ? join_insert_at : start,
            join_insert_at != DOC_NONE ? join_insert_at : (cross_inline_adjust ? start : end),
            desired, &code_syntax, &code_open, &code_close);
        if (result == XUI_OK) code_adjust = 1;
        else if (result != XUI_ERROR_NOT_FOUND) goto done;
    }
    if (cross_inline_adjust) {
        if (code_adjust) {
            doc_md_output old_close = {0}, adjusted = {0};
            old_close.allocator = adjusted.allocator = old->allocator;
            result = doc_md_write_source_range(t, old, code_syntax.iContentEnd, code_syntax.iSourceEnd, &old_close);
            if (result == XUI_OK && (old_close.size > cross_close.size ||
                memcmp(old_close.data, cross_close.data, (size_t)old_close.size)))
                result = XUI_ERROR_NOT_FOUND;
            if (result == XUI_OK) {
                doc_md_write(&adjusted, code_close.data, code_close.size);
                doc_md_write(&adjusted, cross_close.data + old_close.size, cross_close.size - old_close.size);
                result = adjusted.error;
            }
            doc_free(old_close.data);
            if (result != XUI_OK) { doc_free(adjusted.data); goto done; }
            doc_free(cross_close.data); cross_close = adjusted;
        }
        doc_md_write(&output, cross_close.data, cross_close.size);
        doc_md_write(&output, cross_open.data, cross_open.size);
        if (output.error) { result = output.error; goto done; }
        /* The surviving suffix of a multi-scalar entity belongs to the right
         * run. Re-encode it after reopening that run's parser-owned chain. */
        if (last_inside) {
            result = doc_md_write_entity_slice(&output, last, b.iOffset, last_entity.iTextEnd);
            if (result != XUI_OK) goto done;
        }
    }
    /* A moved definition gap starts after the old paragraph ending.
     * When the surviving right paragraph has no EOF ending, add only the
     * new separating LF; every copied gap byte retains its original EOL. */
    if (moved.size && move_at == doc_seq_size(old->source) && move_at &&
        !(heading_adjust && heading_close.size &&
            (heading_close.data[heading_close.size - 1] == '\n' ||
             heading_close.data[heading_close.size - 1] == '\r'))) {
        char terminal;
        result = doc_seq_read(old->source, move_at - 1, &terminal, 1);
        if (result != XUI_OK) goto done;
        if (terminal != '\r' && terminal != '\n') {
            doc_md_output separated = {0}; separated.allocator = old->allocator;
            doc_md_write(&separated, "\n", 1);
            doc_md_write(&separated, moved.data, moved.size);
            if (separated.error) { result = separated.error; doc_free(separated.data); goto done; }
            doc_free(moved.data); moved = separated;
        }
    }
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK && moved.size)
        result = doc_txn_source_patch(&candidate, move_at, move_at,
            moved.data, moved.size, 0);
    if (result == XUI_OK && heading_adjust)
        result = doc_txn_source_patch(&candidate, heading_start, heading_end,
            heading_close.data ? heading_close.data : "", heading_close.size, 0);
    /* The original closing suffix is after the source range. Apply it first
     * so both patches keep their original coordinates and share one history. */
    if (result == XUI_OK && reference_start != DOC_NONE)
        result = doc_txn_source_patch(&candidate, reference_start, reference_end,
            reference_close.data, reference_close.size, 0);
    if (result == XUI_OK && code_adjust && join_insert_at == DOC_NONE && !cross_inline_adjust)
        result = doc_txn_source_patch(&candidate, code_syntax.iContentEnd, code_syntax.iSourceEnd,
            code_close.data, code_close.size, 0);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, start, end,
        join_insert_at != DOC_NONE ? "" : (output.data ? output.data : ""),
        join_insert_at != DOC_NONE ? 0 : output.size, 0);
    /* A seam's left CODE closer precedes the structural deletion. Apply the
     * higher seam first, then adjust the original closer, type at its content
     * boundary, and finally adjust the lower opener. All offsets stay original. */
    if (result == XUI_OK && code_adjust && join_insert_at != DOC_NONE)
        result = doc_txn_source_patch(&candidate, code_syntax.iContentEnd, code_syntax.iSourceEnd,
            code_close.data, code_close.size, 0);
    if (result == XUI_OK && join_insert_at != DOC_NONE)
        result = doc_txn_source_patch(&candidate, join_insert_at, join_insert_at,
            output.data ? output.data : "", output.size, 0);
    if (result == XUI_OK && code_adjust)
        result = doc_txn_source_patch(&candidate, code_syntax.iSourceStart, code_syntax.iContentStart,
            code_open.data, code_open.size, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && cross_inline_adjust && !bytes && !cross_close.size &&
        cross_open.size && a.iOffset && !(first->attrs->iMarks & XUI_DOC_CODE) &&
        !doc_semantic_markdown_projection_equal(desired, candidate.draft)) {
        uint64_t scalar_start = a.iOffset - 1, raw_start, raw_end, width;
        int left_map, right_map;
        char logical[4], raw[4];
        while (scalar_start && !doc_seq_boundary(first->text, scalar_start)) scalar_start--;
        width = a.iOffset - scalar_start;
        /* Joining can change delimiter flanking at the surviving left scalar.
         * Only repair its exact literal source when the first full parse fails
         * the desired projection. All earlier source edits are at higher bytes,
         * so these original lower coordinates still apply to the candidate. */
        if (width <= sizeof(logical) &&
            doc_source_map_text(old, first, scalar_start, XUI_DOC_AFTER, &raw_start, &left_map) == XUI_OK &&
            doc_source_map_text(old, first, a.iOffset, XUI_DOC_BEFORE, &raw_end, &right_map) == XUI_OK &&
            left_map == XUI_DOC_MAP_EXACT && right_map == XUI_DOC_MAP_EXACT &&
            raw_end == start && raw_end - raw_start == width &&
            doc_seq_read(first->text, scalar_start, logical, width) == XUI_OK &&
            doc_seq_read(old->source, raw_start, raw, width) == XUI_OK && !memcmp(logical, raw, (size_t)width)) {
            doc_md_output boundary = {0}; boundary.allocator = old->allocator;
            result = doc_md_write_entity_slice(&boundary, first, scalar_start, a.iOffset);
            if (result == XUI_OK) result = doc_txn_source_patch(&candidate, raw_start, raw_end,
                boundary.data, boundary.size, 0);
            doc_free(boundary.data);
            if (result == XUI_OK) result = doc_markdown_parse(&candidate);
        }
    }
    if (result == XUI_OK && !bytes && first_block == last_block &&
        !code_adjust && join_insert_at == DOC_NONE && !moved.size &&
        !doc_semantic_markdown_projection_equal(desired, candidate.draft)) {
        doc_node *left = first, *right = last;
        uint64_t left_offset = a.iOffset, right_offset = b.iOffset;
        int left_changed = 0, right_changed = 0;
        if (right_offset == doc_seq_size(last->text)) {
            uint64_t index = doc_child_index(last_block, last->id);
            right = index != DOC_NONE && index + 1 < doc_seq_size(last_block->children) ?
                doc_index_get(old->index, doc_seq_get_id(last_block->children, index + 1)) : NULL;
            right_offset = 0;
        }
        if (!left_offset) {
            uint64_t index = doc_child_index(first_block, first->id);
            left = index != DOC_NONE && index ?
                doc_index_get(old->index, doc_seq_get_id(first_block->children, index - 1)) : NULL;
            left_offset = left && left->kind == XUI_DOC_TEXT ? doc_seq_size(left->text) : 0;
        }
        /* Higher source patches and the deletion leave these two frontier
         * coordinates intact. Repair right first so the lower left byte is
         * unchanged; retain any reference-key patch already in the candidate. */
        result = doc_md_delete_boundary_escape(t, &candidate, right, right_offset,
            end, start + output.size, 0, &right_changed);
        if (result == XUI_OK) result = doc_md_delete_boundary_escape(t, &candidate,
            left, left_offset, start, 0, 1, &left_changed);
        if (result == XUI_OK && (left_changed || right_changed))
            result = doc_markdown_parse(&candidate);
    }
    if (result == XUI_OK && !doc_semantic_equal(desired, candidate.draft) &&
        (!doc_semantic_markdown_projection_equal(desired, candidate.draft) ||
         !doc_md_reference_spelling_equal_except_footnote(old->references,
            old->source, candidate.draft->references, candidate.draft->source,
            editable_footnote_start, editable_footnote_end)))
        result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK && cross_block &&
        !doc_md_reference_spelling_equal_except_footnote(old->references,
            old->source, candidate.draft->references, candidate.draft->source,
            editable_footnote_start, editable_footnote_end))
        result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_DOC_ERROR_FORMAT || result == XUI_DOC_ERROR_UNREPRESENTABLE ||
        result == XUI_ERROR_UNSUPPORTED) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++)
            result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft;
            candidate.draft = NULL; t->parse_op_start = t->count;
        }
    }
done:
    doc_state_release(candidate.draft); doc_free(candidate.ops);
    doc_free(container_storage.data); doc_free(raw_storage.data);
    doc_free(output.data); doc_free(moved.data); doc_free(prelude.data);
    doc_free(inline_close.data); doc_free(inline_open.data);
    doc_free(reference_close.data);
    doc_free(code_open.data); doc_free(code_close.data); doc_free(heading_close.data);
    doc_free(cross_close.data); doc_free(cross_open.data);
    return result;
}
/* Both public text editing APIs use one parser-owned deletion source plan.
 * Nonempty edits of stable explicit links keep the incremental text path;
 * implicit references and emptied link runs need this ownership plan too. */
int doc_markdown_text_source_patch(xui_document_transaction t, uint64_t id,
    uint64_t start, uint64_t end, const char* text, uint64_t bytes, doc_state* desired)
{
    doc_node* node = doc_index_get(t->draft->index, id);
    doc_node* changed = doc_index_get(desired->index, id);
    doc_node_source_range source; xui_doc_range_t range = {0};
    uint64_t i, count; int result, implicit = 0;
    if (!node || !changed || node->kind != XUI_DOC_TEXT ||
        (bytes && !(node->attrs->iMarks & (XUI_DOC_LINK | XUI_DOC_CODE))))
        return XUI_ERROR_NOT_FOUND;
    doc_node_source_range_get(t->draft, node, &source);
    count = doc_seq_size(t->draft->inline_syntax) / sizeof(xui_doc_inline_syntax_t);
    for (i = 0; doc_seq_size(changed->text) && i < count; i++) {
        xui_doc_inline_syntax_t syntax; uint64_t suffix;
        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
        result = doc_seq_read_syntax(t->draft->inline_syntax, i, &syntax);
        if (result != XUI_OK) return result;
        if (syntax.iKind != XUI_DOC_SYNTAX_LINK || syntax.iDefinitionIndex == DOC_NONE ||
            syntax.iContentStart > source.source_start || syntax.iContentEnd < source.source_end ||
            syntax.iContentEnd > syntax.iSourceEnd) continue;
        suffix = syntax.iSourceEnd - syntax.iContentEnd;
        if (suffix == 1 || suffix == 3) { implicit = 1; break; }
    }
    if (!implicit && bytes && doc_seq_size(changed->text) &&
        !(node->attrs->iMarks & XUI_DOC_CODE)) return XUI_ERROR_NOT_FOUND;
    range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = t->document->identity;
    range.tAnchor.iRevision = t->base_revision; range.tAnchor.iNodeId = id;
    range.tAnchor.iKind = XUI_DOC_POSITION_TEXT; range.tAnchor.iOffset = start;
    range.tAnchor.iAffinity = XUI_DOC_BEFORE;
    range.tCaret = range.tAnchor; range.tCaret.iOffset = end; range.tCaret.iAffinity = XUI_DOC_AFTER;
    return doc_markdown_range_source_patch(t, &range, text, bytes, desired);
}
/* A short GFM row has synthetic trailing cells with no source anchor. Extend
 * only that physical row, preserving the other rows and their separator
 * spelling. The parser's pipe tokens distinguish a real trailing delimiter
 * from an escaped or code-span pipe. */
static int doc_md_omitted_cell_insert(xui_document_transaction t, doc_node* cell,
    doc_node* row, doc_node* table, const char* text, uint64_t bytes,
    doc_md_output* output, uint64_t* at)
{
    doc_state* old = t->draft;
    doc_node_source_range row_range, table_range, child_range;
    uint64_t count = doc_seq_size(row->children), target, first = DOC_NONE;
    uint64_t i, end, last, token_count;
    int result, trailing_pipe = 0;
    char ch;
    if (!table->syntax_aux ||
        table->syntax_aux->size % sizeof(doc_table_token_relative))
        return XUI_ERROR_NOT_FOUND;
    target = doc_child_index(row, cell->id);
    if (target == DOC_NONE) return XUI_ERROR_NOT_FOUND;
    for (i = 0; i < count; i++) {
        doc_node* child = doc_index_get(old->index, doc_seq_get_id(row->children, i));
        if (!child || child->kind != XUI_DOC_CELL) return XUI_ERROR_NOT_FOUND;
        doc_node_source_range_get(old, child, &child_range);
        if (child_range.syntax_start == DOC_NONE) {
            if (first == DOC_NONE) first = i;
            if (doc_seq_size(child->children)) return XUI_ERROR_NOT_FOUND;
        } else if (first != DOC_NONE) return XUI_ERROR_NOT_FOUND;
    }
    if (first == DOC_NONE || target < first) return XUI_ERROR_NOT_FOUND;
    doc_node_source_range_get(old, row, &row_range);
    doc_node_source_range_get(old, table, &table_range);
    if (row_range.syntax_start == DOC_NONE ||
        row_range.syntax_end == DOC_NONE ||
        row_range.syntax_start >= row_range.syntax_end ||
        row_range.syntax_end > doc_seq_size(old->source) ||
        table_range.syntax_start == DOC_NONE ||
        table_range.syntax_start > row_range.syntax_start ||
        table_range.syntax_end < row_range.syntax_end)
        return XUI_ERROR_NOT_FOUND;
    end = row_range.syntax_end;
    result = doc_seq_read(old->source, end - 1, &ch, 1);
    if (result != XUI_OK) return result;
    if (ch == '\n') {
        end--;
        if (end > row_range.syntax_start) {
            result = doc_seq_read(old->source, end - 1, &ch, 1);
            if (result != XUI_OK) return result;
            if (ch == '\r') end--;
        }
    } else if (ch == '\r') end--;
    if (end <= row_range.syntax_start) return XUI_ERROR_NOT_FOUND;
    last = end;
    while (last > row_range.syntax_start) {
        char chunk[4096];
        uint64_t size = last - row_range.syntax_start;
        uint64_t base, j;
        if (size > sizeof(chunk)) size = sizeof(chunk);
        base = last - size;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(old->source, base, chunk, size);
        if (result != XUI_OK) return result;
        for (j = size; j > 0; j--)
            if (chunk[j - 1] != ' ' && chunk[j - 1] != '\t') break;
        if (j) { ch = chunk[j - 1]; last = base + j; break; }
        last = base;
    }
    if (last == row_range.syntax_start)
        return XUI_ERROR_NOT_FOUND;
    if (ch == '|') {
        token_count = table->syntax_aux->size / sizeof(doc_table_token_relative);
        for (i = 0; i < token_count; i++) {
            doc_table_token_relative token;
            memcpy(&token, table->syntax_aux->data + i * sizeof(token), sizeof(token));
            if (token.kind == XUI_DOC_TABLE_TOKEN_PIPE &&
                table_range.syntax_start + token.start == last - 1) {
                trailing_pipe = 1; break;
            }
        }
    }
    for (i = first; i < target && !output->error; i++)
        doc_md_write(output, trailing_pipe ? " |" : " | ", trailing_pipe ? 2 : 3);
    doc_md_write(output, trailing_pipe ? " " : " | ", trailing_pipe ? 1 : 3);
    doc_md_escape(output, text, bytes);
    if (trailing_pipe) doc_md_write(output, " |", 2);
    if (output->error) return output->error;
    *at = end;
    return XUI_OK;
}
/* Explicit empty cells insert at a parser-owned zero-width anchor; omitted
 * trailing cells use a checked row extension. A private parse must reproduce
 * the desired tree before either source operation is published. */
static int doc_markdown_cell_gap_source_patch(xui_document_transaction t,
    const xui_doc_range_t* range, const char* text, uint64_t bytes,
    doc_state* desired)
{
    const xui_doc_position_t* position = &range->tAnchor;
    doc_state* old = t->draft;
    doc_node *cell, *row, *table;
    doc_node_source_range source_range;
    struct xui_doc_transaction_t candidate = {0};
    doc_md_output output = {0};
    uint64_t i, at;
    int result = XUI_ERROR_NOT_FOUND;
    if (!bytes || position->iKind != XUI_DOC_POSITION_GAP ||
        range->tCaret.iKind != XUI_DOC_POSITION_GAP ||
        position->iNodeId != range->tCaret.iNodeId ||
        position->iOffset != 0 || range->tCaret.iOffset != 0)
        return result;
    cell = doc_index_get(old->index, position->iNodeId);
    row = cell ? doc_index_get(old->index, cell->parent) : NULL;
    table = row ? doc_index_get(old->index, row->parent) : NULL;
    if (!cell || cell->kind != XUI_DOC_CELL || doc_seq_size(cell->children) ||
        !row || row->kind != XUI_DOC_ROW || !table ||
        table->kind != XUI_DOC_TABLE) return result;
    doc_node_source_range_get(old, cell, &source_range);
    output.allocator = old->allocator;
    if (source_range.syntax_start == DOC_NONE) {
        result = doc_md_omitted_cell_insert(t, cell, row, table,
            text, bytes, &output, &at);
        if (result != XUI_OK) goto done;
    } else {
        if (source_range.syntax_start != source_range.syntax_end ||
            source_range.syntax_start > doc_seq_size(old->source))
            return result;
        at = source_range.syntax_start;
        doc_md_escape(&output, text, bytes);
        if (output.error) { result = output.error; goto done; }
    }
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate,
        at, at, output.data, output.size, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && (!doc_semantic_equal(desired, candidate.draft) ||
        !doc_md_reference_spelling_equal(old->references, old->source,
            candidate.draft->references, candidate.draft->source)))
        result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_DOC_ERROR_FORMAT || result == XUI_DOC_ERROR_UNREPRESENTABLE ||
        result == XUI_ERROR_UNSUPPORTED) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++)
            result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft;
            candidate.draft = NULL; t->parse_op_start = t->count;
        }
    }
done:
    doc_state_release(candidate.draft); doc_free(candidate.ops);
    doc_free(output.data);
    return result;
}
/* Structural range input can introduce a blank separator inside an item.
 * Tightness belongs to Markdown's parsed list wrapper, not to the text intent.
 * Plan that derived attribute only on the edited block's original ancestors,
 * in an independent semantic shadow. The complete tree and all other attrs
 * must still match before either the plan or its attribute operations survive.
 * A rejected source candidate never modifies the caller's semantic intent. */
static int doc_md_range_derive_list_tightness(xui_document_transaction t,
    xui_document_transaction shadow, doc_state* parsed, uint64_t affected_block)
{
    struct xui_doc_transaction_t planned = {0};
    doc_node* ancestor = doc_index_get(t->draft->index, affected_block);
    uint64_t i; unsigned depth = 0; int result;
    result = doc_markdown_shadow_begin(shadow, &planned);
    if (result != XUI_OK) return result;
    for (; ancestor && ancestor->id != DOC_ROOT;
        ancestor = doc_index_get(t->draft->index, ancestor->parent)) {
        doc_node *desired, *actual; xui_doc_attributes_t attrs;
        result = doc_txn_check(t, 0); if (result != XUI_OK) goto done;
        if (++depth >= DOC_MAX_DEPTH) { result = XUI_DOC_ERROR_LIMIT; goto done; }
        if (ancestor->kind != XUI_DOC_LIST) continue;
        desired = doc_index_get(planned.draft->index, ancestor->id);
        actual = doc_index_get(parsed->index,
            doc_markdown_resolve_node(planned.draft, parsed, ancestor->id));
        if (!desired || !actual || desired->kind != XUI_DOC_LIST || actual->kind != XUI_DOC_LIST) {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
        if (!((desired->attrs->iFlags ^ actual->attrs->iFlags) & XUI_DOC_TIGHT)) continue;
        attrs = *desired->attrs;
        attrs.iFlags = (attrs.iFlags & ~XUI_DOC_TIGHT) | (actual->attrs->iFlags & XUI_DOC_TIGHT);
        result = xuiDocumentTxnSetAttributes(&planned, desired->id, &attrs);
        if (result != XUI_OK) goto done;
    }
    if (!doc_semantic_markdown_projection_equal(planned.draft, parsed)) {
        result = XUI_ERROR_NOT_FOUND; goto done;
    }
    for (i = 0; i < planned.count; i++) {
        result = doc_txn_op(shadow, planned.ops + i); if (result != XUI_OK) goto done;
    }
    if (planned.count) {
        doc_state_release(shadow->draft); shadow->draft = planned.draft; planned.draft = NULL;
    }
    result = XUI_OK;
done:
    doc_state_release(planned.draft); doc_free(planned.ops); return result;
}
/* Insert a paragraph at an existing block container frontier. Keep both
 * adjacent blocks and all source-only gaps byte-for-byte; serialize only the
 * new paragraph. The parser owns the physical anchor and container prefix,
 * and a complete parse proves the resulting semantic/definition projection. */
static int doc_markdown_container_gap_source_patch(xui_document_transaction t,
    const xui_doc_range_t* range, uint64_t bytes, xui_document_transaction shadow)
{
    const xui_doc_position_t* position = &range->tAnchor;
    doc_state* old = t->draft; doc_state* desired = shadow->draft;
    doc_node *item, *after, *paragraph, *previous, *following = NULL, *ancestor;
    doc_node prefix_paragraph = {0}; doc_node_source_range prefix_range = {0};
    struct xui_doc_snapshot_t snapshot = {0};
    struct xui_doc_transaction_t candidate = {0};
    xui_doc_block_syntax_t syntax = {0};
    xui_doc_source_line_t marker_line = {0}, frontier_line = {0}, previous_line = {0};
    doc_md_output first = {0}, continuation = {0}, body = {0}, output = {0};
    char *raw = NULL, ending[2] = {'\n', 0};
    uint64_t count, slot = position->iOffset, at, start, end = 0, prefix_end, prefix_bytes;
    uint64_t i, blank_bytes, padding = 0, ending_bytes = 1, replace_end, reused_prefix = 0;
    uint64_t editable_start = DOC_NONE, editable_end = DOC_NONE;
    int result = XUI_ERROR_NOT_FOUND, blank_before = 0, terminal = 1, relocate_prefix = 0;
    if (!bytes || position->iKind != XUI_DOC_POSITION_GAP ||
        range->tCaret.iKind != XUI_DOC_POSITION_GAP ||
        position->iNodeId != range->tCaret.iNodeId || slot != range->tCaret.iOffset)
        return result;
    item = doc_index_get(old->index, position->iNodeId);
    after = doc_index_get(desired->index, position->iNodeId);
    if (!item || (item->kind != XUI_DOC_LIST_ITEM && item->kind != XUI_DOC_QUOTE &&
        item->kind != XUI_DOC_FOOTNOTE && item->kind != XUI_DOC_ROOT) || !after || after->kind != item->kind)
        return result;
    if (!slot && item->kind != XUI_DOC_QUOTE && item->kind != XUI_DOC_ROOT && item->kind != XUI_DOC_FOOTNOTE) return result;
    count = doc_seq_size(item->children);
    if (!count || count == UINT64_MAX || slot > count || doc_seq_size(after->children) != count + 1)
        return result;
    paragraph = doc_index_get(desired->index, doc_seq_get_id(after->children, slot));
    if (!paragraph || paragraph->kind != XUI_DOC_PARAGRAPH || doc_index_get(old->index, paragraph->id))
        return result;
    for (i = 0; i < count; i++)
        if (doc_seq_get_id(item->children, i) != doc_seq_get_id(after->children, i + (i >= slot)))
            return result;
    if (slot) {
        previous = doc_index_get(old->index, doc_seq_get_id(item->children, slot - 1));
        if (!previous || doc_md_bounds(old, previous->id, &start, &end) != XUI_OK || !end)
            return result;
    }
    snapshot.state = old; snapshot.identity = t->document->identity; snapshot.revision = t->base_revision;
    syntax.iSize = sizeof(syntax);
    marker_line.iSize = frontier_line.iSize = previous_line.iSize = sizeof(marker_line);
    if (slot < count) {
        uint64_t next_end;
        following = doc_index_get(old->index, doc_seq_get_id(item->children, slot));
        if (!following || doc_md_bounds(old, following->id, &at, &next_end) != XUI_OK)
            return XUI_ERROR_NOT_FOUND;
        result = xuiDocumentSnapshotGetSourceLine(&snapshot, at, &frontier_line);
        if (result != XUI_OK) return result;
        at = frontier_line.iLineStart;
        if (slot && end > at) return XUI_ERROR_NOT_FOUND;
        if (!slot && !at && doc_seq_size(old->source) >= 3) {
            char bom[3]; result = doc_seq_read(old->source, 0, bom, sizeof(bom));
            if (result != XUI_OK) return result;
            if (!memcmp(bom, "\xef\xbb\xbf", 3)) at = 3;
        }
    } else {
        result = xuiDocumentSnapshotGetSourceLine(&snapshot, end - 1, &frontier_line);
        if (result != XUI_OK) return result;
        at = frontier_line.iLineEnd;
        terminal = at != doc_seq_size(old->source) || frontier_line.iContentEnd != at;
    }
    replace_end = at;
    if (item->kind == XUI_DOC_LIST_ITEM) {
        result = xuiDocumentSnapshotGetBlockSyntax(&snapshot, item->id, &syntax);
        if (result != XUI_OK) return result;
        result = xuiDocumentSnapshotGetSourceLine(&snapshot, syntax.iPrimaryStart, &marker_line);
        if (result != XUI_OK) return result;
        prefix_end = syntax.iTaskMarkerGapEnd == DOC_NONE ? syntax.iListMarkerGapEnd : syntax.iTaskMarkerGapEnd;
        if (prefix_end == DOC_NONE || prefix_end < syntax.iPrimaryEnd || prefix_end > marker_line.iContentEnd)
            return XUI_ERROR_NOT_FOUND;
    } else {
        /* Prefer a parser-owned paragraph as a physical prefix sample. Its
         * semantic start can include inline openers; the existing prefix
         * planner consumes only whitespace and confirmed ancestor markers. */
        doc_node_source_range sample = {0}; doc_node* leaf = NULL;
        if (!slot) {
            /* The first physical frontier owns any list/footnote header.
             * A later paragraph cannot describe that first-line prefix. */
            leaf = doc_index_get(old->index, doc_seq_get_id(item->children, 0));
        } else {
            for (i = 0; i < count; i++) {
                result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
                leaf = doc_index_get(old->index, doc_seq_get_id(item->children, i));
                if (leaf && leaf->kind == XUI_DOC_PARAGRAPH) break;
            }
            if (i == count) leaf = doc_index_get(old->index, doc_seq_get_id(item->children, 0));
        }
        if (!leaf) return XUI_ERROR_NOT_FOUND;
        doc_node_source_range_get(old, leaf, &sample);
        prefix_end = sample.source_start;
        if (leaf->kind == XUI_DOC_CODE_BLOCK) {
            xui_doc_block_syntax_t code_syntax = {0}; code_syntax.iSize = sizeof(code_syntax);
            result = xuiDocumentSnapshotGetBlockSyntax(&snapshot, leaf->id, &code_syntax);
            if (result != XUI_OK) return result;
            if (code_syntax.iKind == XUI_DOC_BLOCK_SYNTAX_FENCED_CODE) {
                prefix_end = code_syntax.iPrimaryStart;
            } else if (code_syntax.iKind == XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE) {
                xui_doc_code_indent_t indent = {0};
                indent.iSize = sizeof(indent);
                result = xuiDocumentSnapshotGetCodeIndent(&snapshot, leaf->id, 0, &indent);
                if (result != XUI_OK) return result;
                if (indent.iFlags || !indent.iSourceEnd) return XUI_ERROR_NOT_FOUND;
                result = xuiDocumentSnapshotGetSourceLine(&snapshot, indent.iSourceEnd - 1, &marker_line);
                if (result != XUI_OK) return result;
                if (indent.iSourceEnd > marker_line.iContentEnd) return XUI_ERROR_NOT_FOUND;
                if (item->kind == XUI_DOC_ROOT) {
                    prefix_end = marker_line.iLineStart;
                } else if (item->kind == XUI_DOC_QUOTE) {
                    xui_doc_block_syntax_t quote = {0}; uint64_t low = 0, high, qstart = 0, qend = 0;
                    quote.iSize = sizeof(quote);
                    result = xuiDocumentSnapshotGetBlockSyntax(&snapshot, item->id, &quote);
                    if (result != XUI_OK) return result;
                    high = quote.iQuotePrefixCount;
                    while (low < high) {
                        uint64_t middle = low + (high - low) / 2;
                        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
                        result = xuiDocumentSnapshotGetQuotePrefix(&snapshot, item->id, middle, &qstart, &qend);
                        if (result != XUI_OK) return result;
                        if (qstart < marker_line.iLineStart) low = middle + 1; else high = middle;
                    }
                    if (low >= quote.iQuotePrefixCount) return XUI_ERROR_NOT_FOUND;
                    result = xuiDocumentSnapshotGetQuotePrefix(&snapshot, item->id, low, &qstart, &qend);
                    if (result != XUI_OK) return result;
                    if (qstart < marker_line.iLineStart || qend > marker_line.iContentEnd || qend != qstart + 1)
                        return XUI_ERROR_NOT_FOUND;
                    /* A shared Tab belongs partly to quote syntax and partly
                     * to literal code. Use the confirmed quote marker, then
                     * the declared one-space new paragraph prefix; leave the
                     * entire old Tab and code spelling untouched. */
                    prefix_end = qend; padding = 1;
                } else {
                    /* Footnote continuation indentation is already outside
                     * the parser-owned code indent. Preserve its raw prefix. */
                    prefix_end = indent.iSourceStart;
                }
            }
        }
        if (prefix_end == DOC_NONE || prefix_end > doc_seq_size(old->source)) return XUI_ERROR_NOT_FOUND;
        result = xuiDocumentSnapshotGetSourceLine(&snapshot, prefix_end, &marker_line);
        if (result != XUI_OK) return result;
        if (prefix_end > marker_line.iContentEnd) return XUI_ERROR_NOT_FOUND;
    }
    if (item->kind == XUI_DOC_LIST_ITEM && syntax.iTaskMarkerGapEnd == DOC_NONE && item->syntax_aux &&
        item->syntax_aux->size == sizeof(doc_list_first_indent_relative)) {
        doc_list_first_indent_relative indent;
        memcpy(&indent, item->syntax_aux->data, sizeof(indent));
        if (indent.content_column < indent.end_column) {
            uint64_t stop = prefix_end, column = (uint64_t)indent.start_column + syntax.iPrimaryEnd - syntax.iPrimaryStart;
            prefix_end = syntax.iPrimaryEnd;
            while (prefix_end < stop && column < indent.content_column) {
                char byte; uint64_t next;
                result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
                result = doc_seq_read(old->source, prefix_end, &byte, 1); if (result != XUI_OK) return result;
                if (byte != ' ' && byte != '\t') return XUI_ERROR_NOT_FOUND;
                next = column + (byte == '\t' ? 4 - column % 4 : 1);
                if (next > indent.content_column) { padding = indent.content_column - column; column = indent.content_column; break; }
                column = next; prefix_end++;
            }
            if (column != indent.content_column) return XUI_ERROR_NOT_FOUND;
        }
    }
    prefix_range.syntax_start = marker_line.iLineStart;
    if (!prefix_range.syntax_start && prefix_end >= 3) {
        char bom[3]; result = doc_seq_read(old->source, 0, bom, sizeof(bom));
        if (result != XUI_OK) return result;
        if (!memcmp(bom, "\xef\xbb\xbf", 3)) prefix_range.syntax_start = 3;
    }
    prefix_range.source_start = prefix_end; prefix_range.syntax_end = marker_line.iLineEnd;
    prefix_bytes = prefix_end - prefix_range.syntax_start;
    if (prefix_bytes >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    if (frontier_line.iLineEnd > frontier_line.iContentEnd) {
        ending_bytes = frontier_line.iLineEnd - frontier_line.iContentEnd;
        if (ending_bytes > sizeof(ending)) return XUI_ERROR_NOT_FOUND;
        result = doc_seq_read(old->source, frontier_line.iContentEnd, ending, ending_bytes);
        if (result != XUI_OK) return result;
    } else if (marker_line.iLineEnd > marker_line.iContentEnd) {
        ending_bytes = marker_line.iLineEnd - marker_line.iContentEnd;
        if (ending_bytes > sizeof(ending)) return XUI_ERROR_NOT_FOUND;
        result = doc_seq_read(old->source, marker_line.iContentEnd, ending, ending_bytes);
        if (result != XUI_OK) return result;
    }
    first.allocator = continuation.allocator = body.allocator = output.allocator = old->allocator;
    raw = doc_alloc(old->allocator, (size_t)prefix_bytes + 1);
    if (!raw) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    result = doc_seq_read(old->source, prefix_range.syntax_start, raw, prefix_bytes);
    prefix_paragraph.parent = item->id;
    if (result == XUI_OK) result = doc_md_paragraph_prefixes(t, &prefix_paragraph, &prefix_range,
        raw, prefix_bytes, &first, &continuation, DOC_NONE, NULL);
    if (result == XUI_OK && (first.size > prefix_bytes ||
        (item->kind == XUI_DOC_LIST_ITEM && first.size != prefix_bytes))) result = XUI_ERROR_NOT_FOUND;
    if (result != XUI_OK) goto done;
    if (!slot && (first.size != continuation.size ||
        (first.size && memcmp(first.data, continuation.data, (size_t)first.size)))) {
        /* Move only the parser-consumed first-line container prefix onto
         * the new paragraph; keep the old block payload and all its raw
         * spelling. Inline openers beyond that prefix are never removed. */
        if (prefix_range.syntax_start != at || first.size > prefix_bytes) {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
        relocate_prefix = 1; replace_end = at + first.size;
        reused_prefix = continuation.size;
    }
    if (!slot) doc_md_write(&first, "    ", padding);
    if (first.error) { result = first.error; goto done; }
    doc_md_write(&continuation, "    ", padding);
    if (continuation.error) { result = continuation.error; goto done; }
    blank_bytes = continuation.size;
    while (blank_bytes && (continuation.data[blank_bytes - 1] == ' ' || continuation.data[blank_bytes - 1] == '\t')) blank_bytes--;
    if (slot && at) {
        result = xuiDocumentSnapshotGetSourceLine(&snapshot, at - 1, &previous_line);
        if (result != XUI_OK) goto done;
        blank_before = previous_line.iLineEnd == at && previous_line.iContentEnd < at;
        for (i = previous_line.iLineStart; blank_before && i < previous_line.iContentEnd; i++) {
            char byte; result = doc_txn_check(t, 0); if (result != XUI_OK) goto done;
            result = doc_seq_read(old->source, i, &byte, 1); if (result != XUI_OK) goto done;
            if (byte != ' ' && byte != '\t' && byte != '>') blank_before = 0;
        }
        if (previous_line.iContentEnd == at) doc_md_write(&output, ending, ending_bytes);
    }
    if (slot && !blank_before) {
        doc_md_write(&output, continuation.data, blank_bytes);
        doc_md_write(&output, ending, ending_bytes);
    }
    result = doc_md_block_write(desired, paragraph, &body);
    if (result != XUI_OK) goto done;
    doc_md_split_body_write(&output, &body, slot ? &continuation : &first, &continuation, ending, ending_bytes, terminal, 0);
    if (following) {
        doc_md_write(&output, continuation.data, blank_bytes);
        doc_md_write(&output, ending, ending_bytes);
        if (relocate_prefix) doc_md_write(&output, continuation.data, reused_prefix);
    }
    for (ancestor = item; ancestor && ancestor->id != DOC_ROOT; ancestor = doc_index_get(old->index, ancestor->parent))
        if (ancestor->kind == XUI_DOC_FOOTNOTE) {
            doc_node_source_range note; doc_node_source_range_get(old, ancestor, &note);
            editable_start = note.syntax_start; editable_end = note.syntax_end; break;
        }
    result = output.error ? output.error : doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, at, replace_end, output.data, output.size, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_md_reference_spelling_equal_except_footnote(old->references, old->source,
        candidate.draft->references, candidate.draft->source, editable_start, editable_end)) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) result = doc_md_range_derive_list_tightness(t, shadow, candidate.draft, item->id);
    if (result == XUI_DOC_ERROR_FORMAT || result == XUI_DOC_ERROR_UNREPRESENTABLE || result == XUI_ERROR_UNSUPPORTED)
        result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, candidate.ops + i);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft; candidate.draft = NULL; t->parse_op_start = t->count;
        }
    }
done:
    doc_state_release(candidate.draft); doc_free(candidate.ops); doc_free(raw);
    doc_free(first.data); doc_free(continuation.data); doc_free(body.data); doc_free(output.data);
    return result;
}

/* Typing at a list item's block gap creates a paragraph, including the empty
 * item published by SplitListItem. Its parser-owned first-line content anchor
 * lets us write only that new paragraph instead of serializing the list and
 * its untouched inline spelling, code blocks or hidden definitions. */
static int doc_markdown_list_gap_source_patch(xui_document_transaction t,
    const xui_doc_range_t* range, uint64_t bytes, xui_document_transaction shadow)
{
    const xui_doc_position_t* position = &range->tAnchor;
    doc_state* old = t->draft; doc_state* desired = shadow->draft;
    doc_node *item, *after, *paragraph, *ancestor, prefix_paragraph = {0};
    doc_node_source_range prefix_range = {0};
    struct xui_doc_snapshot_t snapshot = {0};
    struct xui_doc_transaction_t candidate = {0};
    xui_doc_block_syntax_t syntax = {0}; xui_doc_source_line_t line = {0};
    doc_md_output body = {0}, first = {0}, continuation = {0}, output = {0}, empty = {0};
    char *raw = NULL, ending[2] = {'\n', 0};
    uint64_t at, replace_end, prefix_bytes, i, ending_bytes = 1, padding_bytes = 0, suffix_bytes = 0;
    uint64_t editable_start = DOC_NONE, editable_end = DOC_NONE;
    int result = XUI_ERROR_NOT_FOUND;
    if (!bytes || position->iKind != XUI_DOC_POSITION_GAP ||
        range->tCaret.iKind != XUI_DOC_POSITION_GAP ||
        position->iNodeId != range->tCaret.iNodeId ||
        position->iOffset || range->tCaret.iOffset) return result;
    item = doc_index_get(old->index, position->iNodeId);
    after = doc_index_get(desired->index, position->iNodeId);
    if (!item || item->kind != XUI_DOC_LIST_ITEM || !after || after->kind != item->kind ||
        doc_seq_size(item->children) == UINT64_MAX ||
        doc_seq_size(after->children) != doc_seq_size(item->children) + 1) return result;
    paragraph = doc_index_get(desired->index, doc_seq_get_id(after->children, 0));
    if (!paragraph || paragraph->kind != XUI_DOC_PARAGRAPH ||
        doc_index_get(old->index, paragraph->id)) return result;
    for (i = 0; i < doc_seq_size(item->children); i++)
        if (doc_seq_get_id(item->children, i) != doc_seq_get_id(after->children, i + 1)) return result;
    snapshot.state = old; snapshot.identity = t->document->identity; snapshot.revision = t->base_revision;
    syntax.iSize = sizeof(syntax); line.iSize = sizeof(line);
    result = xuiDocumentSnapshotGetBlockSyntax(&snapshot, item->id, &syntax);
    if (result != XUI_OK) return result;
    at = syntax.iTaskMarkerGapEnd == DOC_NONE ? syntax.iListMarkerGapEnd : syntax.iTaskMarkerGapEnd;
    result = xuiDocumentSnapshotGetSourceLine(&snapshot, syntax.iPrimaryStart, &line);
    if (result != XUI_OK) return result;
    if (at == DOC_NONE || at < syntax.iPrimaryEnd || at > line.iContentEnd ||
        line.iLineEnd < line.iContentEnd || line.iLineEnd > doc_seq_size(old->source)) return XUI_ERROR_NOT_FOUND;
    replace_end = at;
    if (at < line.iTrailingStart && syntax.iTaskMarkerGapEnd == DOC_NONE && item->syntax_aux &&
        item->syntax_aux->size == sizeof(doc_list_first_indent_relative)) {
        doc_list_first_indent_relative indent; memcpy(&indent, item->syntax_aux->data, sizeof(indent));
        if (indent.content_column < indent.end_column) {
            uint64_t end = at, column = (uint64_t)indent.start_column + syntax.iPrimaryEnd - syntax.iPrimaryStart;
            at = syntax.iPrimaryEnd;
            /* The parser already decided how many columns belong to this
             * list item. Split only that whitespace; a partly consumed Tab
             * becomes the exact spaces on either side of the new paragraph. */
            while (at < end && column < indent.content_column) {
                char byte; uint64_t next;
                result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
                result = doc_seq_read(old->source, at, &byte, 1); if (result != XUI_OK) return result;
                if (byte != ' ' && byte != '\t') return XUI_ERROR_NOT_FOUND;
                next = column + (byte == '\t' ? 4 - column % 4 : 1);
                if (next > indent.content_column) {
                    padding_bytes = indent.content_column - column; suffix_bytes = next - indent.content_column;
                    replace_end = at + 1; column = indent.content_column; break;
                }
                column = next; at++; replace_end = at;
            }
            if (column != indent.content_column) return XUI_ERROR_NOT_FOUND;
        }
    }
    prefix_range.syntax_start = line.iLineStart;
    if (!prefix_range.syntax_start && at >= 3) {
        char bom[3];
        result = doc_seq_read(old->source, 0, bom, sizeof(bom));
        if (result != XUI_OK) return result;
        if (!memcmp(bom, "\xef\xbb\xbf", sizeof(bom))) prefix_range.syntax_start = 3;
    }
    prefix_range.source_start = at; prefix_range.syntax_end = line.iLineEnd;
    prefix_bytes = at - prefix_range.syntax_start;
    if (prefix_bytes >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    if (line.iLineEnd != line.iContentEnd) {
        ending_bytes = line.iLineEnd - line.iContentEnd;
        if (ending_bytes > sizeof(ending)) return XUI_ERROR_NOT_FOUND;
        result = doc_seq_read(old->source, line.iContentEnd, ending, ending_bytes);
        if (result != XUI_OK) return result;
    }
    for (ancestor = item; ancestor && ancestor->id != DOC_ROOT;
        ancestor = doc_index_get(old->index, ancestor->parent)) {
        if (ancestor->kind == XUI_DOC_FOOTNOTE) {
            doc_node_source_range note; doc_node_source_range_get(old, ancestor, &note);
            editable_start = note.syntax_start; editable_end = note.syntax_end; break;
        }
    }
    body.allocator = first.allocator = continuation.allocator = output.allocator = old->allocator;
    raw = doc_alloc(old->allocator, (size_t)prefix_bytes + 1);
    if (!raw) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    result = doc_seq_read(old->source, prefix_range.syntax_start, raw, prefix_bytes);
    prefix_paragraph.parent = item->id;
    if (result == XUI_OK) result = doc_md_paragraph_prefixes(t, &prefix_paragraph, &prefix_range,
        raw, prefix_bytes, &first, &continuation, DOC_NONE, NULL);
    if (result == XUI_OK && first.size != prefix_bytes) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) result = doc_md_block_write(desired, paragraph, &body);
    if (result != XUI_OK) goto done;
    if (padding_bytes) {
        doc_md_write(&output, "    ", padding_bytes);
        doc_md_write(&continuation, "    ", padding_bytes);
        if (continuation.error) { result = continuation.error; goto done; }
    }
    doc_md_split_body_write(&output, &body, &empty, &continuation, ending, ending_bytes, 0, 0);
    /* An existing payload on the marker line is the first surviving block.
     * Give the new paragraph its own boundary, then reuse the exact payload. */
    if (replace_end < line.iTrailingStart) {
        doc_md_write(&output, ending, ending_bytes);
        doc_md_write(&output, continuation.data, continuation.size);
        doc_md_write(&output, ending, ending_bytes);
        doc_md_write(&output, continuation.data, continuation.size);
        doc_md_write(&output, "    ", suffix_bytes);
    }
    result = output.error ? output.error : doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, at, replace_end, output.data, output.size, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_md_reference_spelling_equal_except_footnote(old->references, old->source,
        candidate.draft->references, candidate.draft->source, editable_start, editable_end))
        result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) result = doc_md_range_derive_list_tightness(t, shadow, candidate.draft, item->id);
    if (result == XUI_DOC_ERROR_FORMAT || result == XUI_DOC_ERROR_UNREPRESENTABLE ||
        result == XUI_ERROR_UNSUPPORTED) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, candidate.ops + i);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft;
            candidate.draft = NULL; t->parse_op_start = t->count;
        }
    }
done:
    doc_state_release(candidate.draft); doc_free(candidate.ops); doc_free(raw);
    doc_free(body.data); doc_free(first.data); doc_free(continuation.data); doc_free(output.data);
    return result;
}
static int doc_markdown_inline_delete_source_patch(xui_document_transaction, struct xui_doc_transaction_t*);
int doc_markdown_range(xui_document_transaction t, const xui_doc_range_t* range, const char* text, uint64_t bytes, xui_doc_position_t* caret)
{
    struct xui_doc_transaction_t expected = {0}; xui_doc_position_t target = {0}; int result;
    result = doc_markdown_shadow_begin(t, &expected); if (result != XUI_OK) return result;
    result = xuiDocumentTxnReplaceRange(&expected, range, text, bytes, &target);
    if (result == XUI_OK && expected.count) {
        result = doc_markdown_cell_gap_source_patch(t, range, text, bytes,
            expected.draft);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_list_gap_source_patch(t, range, bytes, &expected);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_container_gap_source_patch(t, range, bytes, &expected);
        if (result == XUI_ERROR_NOT_FOUND && !bytes)
            result = doc_markdown_inline_delete_source_patch(t, &expected);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_range_source_patch(t, range, text, bytes, expected.draft);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_apply_tree(t, expected.draft, NULL, NULL, DOC_MD_ACCEPT_DERIVED_FOOTNOTES);
        if (result == XUI_OK) result = doc_markdown_accept(t, &expected,DOC_MD_ACCEPT_DERIVED_FOOTNOTES);
    }
    if (result == XUI_OK && caret)
        *caret = doc_markdown_resolve_caret(t, expected.draft, target);
    doc_state_release(expected.draft); doc_free(expected.ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
int doc_markdown_shadow_begin(xui_document_transaction t, struct xui_doc_transaction_t* shadow)
{
    memset(shadow, 0, sizeof(*shadow)); shadow->document = t->document; shadow->domain = XUI_DOC_SEMANTIC;
    shadow->parsing = DOC_BUILD_SEMANTIC; shadow->base_revision = t->base_revision;
    shadow->cancellation = t->cancellation; shadow->draft = doc_state_clone(t->draft);
    return shadow->draft ? XUI_OK : doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
}
/* A language change must not regenerate the fence, literal body or opaque
 * remainder of its info string. Parser-confirmed byte ranges work unchanged
 * inside root, quote, list and footnote containers. */
static int doc_markdown_code_language_source_patch(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow)
{
    struct xui_doc_transaction_t candidate = {0}; doc_md_output output = {0};
    doc_node *before, *after; doc_node_source_range range; doc_fence_info_relative info;
    uint64_t start, end, i, editable_start = DOC_NONE, editable_end = DOC_NONE;
    const char* language; int result = XUI_ERROR_NOT_FOUND;
    if (shadow->count != 1 || shadow->ops[0].iKind != XUI_DOC_OP_ATTRIBUTES ||
        shadow->ops[0].iFlags != XUI_DOC_CHANGE_RESOURCE) return result;
    before = doc_index_get(t->draft->index, shadow->ops[0].iNodeId);
    after = doc_index_get(shadow->draft->index, shadow->ops[0].iNodeId);
    if (!before || !after || before->kind != XUI_DOC_CODE_BLOCK || after->kind != before->kind ||
        before->parent != after->parent || before->text != after->text || before->children != after->children ||
        !doc_attributes_equal(before->attrs, after->attrs) ||
        strcmp(doc_string(before->resource), doc_string(after->resource)) ||
        strcmp(doc_string(before->title), doc_string(after->title)) ||
        before->marker_kind != XUI_DOC_BLOCK_SYNTAX_FENCED_CODE || !before->syntax_aux ||
        before->syntax_aux->size != sizeof(info)) return result;
    language = doc_string(after->info);
    doc_node_source_range_get(t->draft, before, &range);
    memcpy(&info, before->syntax_aux->data, sizeof(info));
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE || range.syntax_end < range.syntax_start ||
        range.syntax_end > doc_seq_size(t->draft->source) || info.info_start < before->marker_primary_end ||
        info.info_start > info.language_end || info.language_end > info.info_end ||
        info.info_end > before->marker_tail_end || before->marker_tail_end > range.syntax_end - range.syntax_start) return result;
    start = range.syntax_start + info.info_start; end = range.syntax_start + info.language_end;
    {
        doc_node* ancestor = doc_index_get(t->draft->index, before->parent);
        while (ancestor && ancestor->id != DOC_ROOT) {
            if (ancestor->kind == XUI_DOC_FOOTNOTE) {
                doc_node_source_range note; doc_node_source_range_get(t->draft, ancestor, &note);
                if (note.syntax_start == DOC_NONE || note.syntax_end == DOC_NONE ||
                    start < note.syntax_start || end > note.syntax_end) return XUI_ERROR_NOT_FOUND;
                editable_start = note.syntax_start; editable_end = note.syntax_end; break;
            }
            ancestor = doc_index_get(t->draft->index, ancestor->parent);
        }
    }
    output.allocator = t->draft->allocator;
    for (i = 0; language[i] && !output.error; i++) {
        if (!(i & 4095) && (output.error = doc_txn_check(t, 0)) != XUI_OK) break;
        doc_md_fence_info_char(&output, language[i]);
    }
    result = output.error ? output.error : doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, start, end, output.data, output.size, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    /* Clearing a language while retaining nonempty metadata would make its
     * next word become the language. Reject instead of silently discarding it. */
    if (result == XUI_OK && (!doc_semantic_equal(shadow->draft, candidate.draft) ||
        !doc_md_reference_spelling_equal_except_footnote(t->draft->references, t->draft->source,
            candidate.draft->references, candidate.draft->source, editable_start, editable_end))) result = XUI_DOC_ERROR_UNREPRESENTABLE;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) { doc_state_release(t->draft); t->draft = candidate.draft; candidate.draft = NULL; t->parse_op_start = t->count; }
    }
    doc_state_release(candidate.draft); doc_free(candidate.ops); doc_free(output.data); return result;
}
typedef struct doc_md_prefix_span { uint64_t start, end; } doc_md_prefix_span;
typedef struct doc_md_prefix_spans { doc_md_prefix_span* data; uint64_t count, capacity; } doc_md_prefix_spans;
static int doc_md_prefix_span_add(xui_document_transaction t, doc_md_prefix_spans* spans,
    uint64_t start, uint64_t end)
{
    if (start >= end) return XUI_DOC_ERROR_FORMAT;
    if (spans->count == spans->capacity) {
        uint64_t capacity = spans->capacity ? spans->capacity * 2 : 16;
        doc_md_prefix_span* next;
        if (capacity < spans->capacity || capacity > SIZE_MAX / sizeof(*next)) return XUI_DOC_ERROR_LIMIT;
        next = doc_realloc(t->draft->allocator, spans->data, (size_t)capacity * sizeof(*next));
        if (!next) return XUI_ERROR_OUT_OF_MEMORY;
        spans->data = next; spans->capacity = capacity;
    }
    spans->data[spans->count++] = (doc_md_prefix_span){start, end}; return XUI_OK;
}
/* Only descendant container markers can extend a line's indentation proof.
 * A literal '>' or Tab inside fenced code is content and is never scanned as
 * a prefix. Gather once, then merge the source-ordered spans in linear time. */
static int doc_md_quote_child_prefixes(xui_document_transaction t, doc_node* node,
    uint64_t excluded, doc_md_prefix_spans* spans, unsigned depth)
{
    uint64_t i; int result = doc_txn_check(t, 0); doc_node_source_range range;
    if (result != XUI_OK) return result;
    if (!node || depth >= DOC_MAX_DEPTH) return XUI_DOC_ERROR_SCHEMA;
    if (node->kind != XUI_DOC_QUOTE && node->kind != XUI_DOC_LIST && node->kind != XUI_DOC_LIST_ITEM) return XUI_OK;
    doc_node_source_range_get(t->draft, node, &range);
    if (node->kind == XUI_DOC_QUOTE && node->id != excluded) {
        if (!node->syntax_aux || node->syntax_aux->size % sizeof(doc_quote_indent_relative) ||
            range.syntax_start == DOC_NONE) return XUI_ERROR_NOT_FOUND;
        for (i = 0; i < node->syntax_aux->size / sizeof(doc_quote_indent_relative); i++) {
            doc_quote_indent_relative record;
            if (!(i & 4095u)) { result = doc_txn_check(t, 0); if (result != XUI_OK) return result; }
            memcpy(&record, node->syntax_aux->data + i * sizeof(record), sizeof(record));
            result = doc_md_prefix_span_add(t, spans, range.syntax_start + record.marker, range.syntax_start + record.end);
            if (result != XUI_OK) return result;
        }
    } else if (node->kind == XUI_DOC_LIST_ITEM) {
        if (range.syntax_start == DOC_NONE || node->marker_kind != XUI_DOC_BLOCK_SYNTAX_LIST_ITEM ||
            node->marker_tail_end <= node->marker_primary_start) return XUI_ERROR_NOT_FOUND;
        result = doc_md_prefix_span_add(t, spans, range.syntax_start + node->marker_primary_start,
            range.syntax_start + node->marker_tail_end);
        if (result != XUI_OK) return result;
    }
    for (i = 0; i < doc_seq_size(node->children); i++) {
        result = doc_md_quote_child_prefixes(t, doc_index_get(t->draft->index,
            doc_seq_get_id(node->children, i)), excluded, spans, depth + 1);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
static int doc_md_prefix_span_compare(const void* left, const void* right)
{
    const doc_md_prefix_span *a = left, *b = right;
    return a->start < b->start ? -1 : a->start > b->start ? 1 : a->end < b->end ? -1 : a->end > b->end;
}

/* A prefix edit inside one resolved footnote changes that definition's body.
 * Its ordered identity/label and every other definition still must match.
 * The sparse prefix proof bounds the changed bytes; the complete desired
 * semantic tree independently checks the newly parsed body before publish. */
static int doc_md_unwrap_references(xui_document_transaction t, doc_node* quote,
    doc_state* actual, int* equal)
{
    doc_state* old = t->draft;
    doc_node* owner = doc_index_get(old->index, quote->parent);
    doc_node_source_range range;
    uint64_t count, i; int found = 0, result;
    *equal = 0;
    for (; owner && owner->id != DOC_ROOT && owner->kind != XUI_DOC_FOOTNOTE;
        owner = doc_index_get(old->index, owner->parent)) {}
    if (!owner || owner->kind != XUI_DOC_FOOTNOTE)
        return doc_md_reference_values_equal(t, old, actual, equal);
    doc_node_source_range_get(old, owner, &range);
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE)
        return XUI_OK;
    count = doc_seq_size(old->reference_values);
    if (doc_seq_size(old->references) % sizeof(xui_doc_reference_definition_t) ||
        doc_seq_size(actual->references) % sizeof(xui_doc_reference_definition_t) ||
        count != doc_seq_size(old->references) / sizeof(xui_doc_reference_definition_t) ||
        doc_seq_size(actual->reference_values) != doc_seq_size(actual->references) / sizeof(xui_doc_reference_definition_t))
        return XUI_DOC_ERROR_SCHEMA;
    if (count != doc_seq_size(actual->reference_values)) return XUI_OK;
    for (i = 0; i < count; i++) {
        xui_doc_reference_definition_t a, b;
        doc_sequence *left = doc_seq_get_value_item(old->reference_values, i),
            *right = doc_seq_get_value_item(actual->reference_values, i);
        uint64_t kind = doc_seq_get_id(old->reference_values, i), at, end;
        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
        if (doc_seq_read(old->references, i * sizeof(a), &a, sizeof(a)) != XUI_OK ||
            doc_seq_read(actual->references, i * sizeof(b), &b, sizeof(b)) != XUI_OK ||
            !left || !right || doc_seq_size(left) < 4 * sizeof(uint64_t) || doc_seq_size(right) < 4 * sizeof(uint64_t) ||
            (kind != XUI_DOC_REFERENCE_LINK && kind != XUI_DOC_REFERENCE_FOOTNOTE) ||
            kind != a.iKind || b.iKind != doc_seq_get_id(actual->reference_values, i)) return XUI_DOC_ERROR_SCHEMA;
        if (a.iKind != b.iKind) return XUI_OK;
        at = 0; end = doc_seq_size(left);
        if (kind == XUI_DOC_REFERENCE_FOOTNOTE && a.iSourceStart == range.syntax_start && a.iSourceEnd == range.syntax_end) {
            uint64_t lh[4], rh[4];
            if (doc_seq_read(left, 0, lh, sizeof(lh)) != XUI_OK || doc_seq_read(right, 0, rh, sizeof(rh)) != XUI_OK)
                return XUI_DOC_ERROR_SCHEMA;
            if (lh[0] > doc_seq_size(left) - sizeof(lh) || rh[0] > doc_seq_size(right) - sizeof(rh) ||
                lh[1] != doc_seq_size(left) - sizeof(lh) - lh[0] || rh[1] != doc_seq_size(right) - sizeof(rh) - rh[0] ||
                lh[2] || rh[2] || lh[3] || rh[3]) return XUI_DOC_ERROR_SCHEMA;
            if (found || lh[0] != rh[0] ||
                !doc_md_source_spelling_equal(old->source, a.iSourceStart, a.iLabelEnd,
                    actual->source, b.iSourceStart, b.iLabelEnd)) return XUI_OK;
            found = 1; at = sizeof(lh); end = at + lh[0];
        } else if (doc_seq_size(left) != doc_seq_size(right)) return XUI_OK;
        if (left == right) continue;
        while (at < end) {
            uint64_t bytes = end - at > 16384 ? 16384 : end - at;
            result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
            {
                char a_bytes[16384], b_bytes[16384];
                if (doc_seq_read(left, at, a_bytes, bytes) != XUI_OK || doc_seq_read(right, at, b_bytes, bytes) != XUI_OK)
                    return XUI_DOC_ERROR_SCHEMA;
                if (memcmp(a_bytes, b_bytes, (size_t)bytes)) return XUI_OK;
            }
            at += bytes;
        }
    }
    *equal = found;
    return XUI_OK;
}

/* A separator continues the enclosing containers without starting another
 * list item. Every non-whitespace byte must be a recorded ancestor marker:
 * quote markers remain '>', while list/task markers become equal-width spaces. */
static int doc_md_unwrap_parent_marker(doc_state* state, doc_node* quote, uint64_t at)
{
    doc_node* parent = doc_index_get(state->index, quote->parent);
    for (; parent && parent->id != DOC_ROOT; parent = doc_index_get(state->index, parent->parent)) {
        doc_node_source_range range;
        doc_node_source_range_get(state, parent, &range);
        if (range.syntax_start == DOC_NONE || at < range.syntax_start) continue;
        if (parent->kind == XUI_DOC_QUOTE && parent->marker_kind == XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN) {
            uint64_t relative = at - range.syntax_start, lo = 0;
            uint64_t hi = parent->quote_prefixes ? parent->quote_prefixes->size / sizeof(uint32_t) : 0;
            if (relative == parent->marker_primary_start) return 1;
            while (lo < hi) {
                uint64_t mid = lo + (hi - lo) / 2; uint32_t value;
                memcpy(&value, parent->quote_prefixes->data + mid * sizeof(value), sizeof(value));
                if (value < relative) lo = mid + 1; else hi = mid;
            }
            if (parent->quote_prefixes && lo < parent->quote_prefixes->size / sizeof(uint32_t)) {
                uint32_t value; memcpy(&value, parent->quote_prefixes->data + lo * sizeof(value), sizeof(value));
                if (value == relative) return 1;
            }
        } else if (parent->kind == XUI_DOC_FOOTNOTE) {
            uint64_t i;
            for (i = 0; i < doc_seq_size(state->references); i += sizeof(xui_doc_reference_definition_t)) {
                xui_doc_reference_definition_t ref;
                if (doc_seq_read(state->references, i, &ref, sizeof(ref)) != XUI_OK) return 0;
                if (ref.iKind == XUI_DOC_REFERENCE_FOOTNOTE && ref.iSourceStart == range.syntax_start &&
                    ref.iSourceEnd == range.syntax_end && ref.iLabelStart >= 2 &&
                    at >= ref.iLabelStart - 2 && at < ref.iLabelEnd + 2) return 2;
            }
        } else if (parent->kind == XUI_DOC_LIST_ITEM && parent->marker_kind == XUI_DOC_BLOCK_SYNTAX_LIST_ITEM) {
            uint64_t relative = at - range.syntax_start;
            if ((relative >= parent->marker_primary_start && relative < parent->marker_primary_end) ||
                (relative >= parent->marker_secondary_start && relative < parent->marker_secondary_end)) return 2;
        }
    }
    return 0;
}
static int doc_md_unwrap_separators(xui_document_transaction t, doc_node* quote,
    uint64_t start, uint64_t cut, unsigned tab_left, uint64_t end,
    const char* source, doc_md_output* front, doc_md_output* back, uint64_t* front_at)
{
    doc_md_output prefix = {0}; uint64_t i, bom = 0;
    const char* front_eol = "\n", *back_eol = "\n"; uint64_t front_bytes = 1, back_bytes = 1;
    int result = XUI_OK;
    prefix.allocator = front->allocator = back->allocator = t->draft->allocator;
    if (!start && end >= 3 && !memcmp(source, "\xef\xbb\xbf", 3)) bom = 3;
    if (cut < start + bom) return XUI_ERROR_NOT_FOUND;
    for (i = start + bom; i < cut; i++) {
        char ch = source[i - start];
        result = doc_txn_check(t, 0); if (result != XUI_OK) goto done;
        if (ch != ' ' && ch != '\t') {
            int kind = doc_md_unwrap_parent_marker(t->draft, quote, i);
            if (kind == 1 && ch == '>') {}
            else if (kind == 2 && (unsigned char)ch < 128) ch = ' ';
            else { result = XUI_ERROR_NOT_FOUND; goto done; }
        }
        doc_md_write(&prefix, &ch, 1);
    }
    if (tab_left) doc_md_write(&prefix, "   ", tab_left);
    if (prefix.error) { result = prefix.error; goto done; }
    /* Preserve all old line endings. New blanks use the corresponding edge's
     * local ending, including lone CR and mixed-ending documents. */
    for (i = 0; i < end - start; i++) {
        if (!(i & 16383u)) { result = doc_txn_check(t, 0); if (result != XUI_OK) goto done; }
        if (source[i] == '\r' || source[i] == '\n') {
            front_eol = source + i; front_bytes = source[i] == '\r' && i + 1 < end - start && source[i + 1] == '\n' ? 2 : 1;
            break;
        }
    }
    if (start) {
        char previous[2];
        result = doc_seq_read(t->draft->source, start - 1, previous, 1); if (result != XUI_OK) goto done;
        if (previous[0] == '\r') { front_eol = "\r"; front_bytes = 1; }
        else if (previous[0] == '\n') {
            front_eol = "\n"; front_bytes = 1;
            if (start >= 2) {
                result = doc_seq_read(t->draft->source, start - 2, previous + 1, 1); if (result != XUI_OK) goto done;
                if (previous[1] == '\r') { front_eol = "\r\n"; front_bytes = 2; }
            }
        }
    }
    back_eol = front_eol; back_bytes = front_bytes;
    if (end > start && (source[end - start - 1] == '\r' || source[end - start - 1] == '\n')) {
        back_eol = source + end - start - 1; back_bytes = 1;
        if (source[end - start - 1] == '\n' && end - start > 1 && source[end - start - 2] == '\r') {
            back_eol--; back_bytes = 2;
        }
    } else doc_md_write(back, back_eol, back_bytes);
    doc_md_write(front, prefix.data, prefix.size); doc_md_write(front, front_eol, front_bytes);
    doc_md_write(back, prefix.data, prefix.size); doc_md_write(back, back_eol, back_bytes);
    *front_at = start + bom;
    result = front->error ? front->error : back->error;
done:
    doc_free(prefix.data); return result;
}
/* Remove only the parser-confirmed indentation of one whole quote. Copy
 * every child spelling and lazy continuation; split leading Tabs only when
 * their logical width would change after the prefix is removed. */
static int doc_md_unwrap_quote_source_patch(xui_document_transaction t, doc_state* desired,
    const xui_doc_position_t* at)
{
    doc_node* quote = doc_index_get(t->draft->index, at->iNodeId);
    doc_node_source_range range;
    struct xui_doc_transaction_t candidate = {0}; doc_md_source_plan plan = {0};
    doc_md_output output = {0}, front = {0}, back = {0}; doc_md_prefix_spans prefixes = {0}; char* source = NULL;
    uint64_t start, end, source_size, count, i, cursor, prefix_cursor = 0, first_cut = 0, front_at = 0;
    uint64_t header_end = 0, header_column = 0;
    unsigned first_tab_left = 0, attempt; int result, equal, carry_prefix = 0;
    for (; quote && quote->kind != XUI_DOC_QUOTE; quote = doc_index_get(t->draft->index, quote->parent)) {}
    if (!quote || quote->marker_kind != XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN || !quote->syntax_aux ||
        !quote->syntax_aux->size || quote->syntax_aux->size % sizeof(doc_quote_indent_relative))
        return XUI_ERROR_NOT_FOUND;
    count = quote->syntax_aux->size / sizeof(doc_quote_indent_relative);
    doc_node_source_range_get(t->draft, quote, &range);
    source_size = doc_seq_size(t->draft->source);
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
        range.syntax_start >= range.syntax_end || range.syntax_end > source_size) return XUI_ERROR_NOT_FOUND;
    start = range.syntax_start; end = range.syntax_end;
    if (quote->info && quote->info->size) {
        if (!quote->marker_secondary_end || quote->marker_secondary_start < quote->marker_primary_end ||
            quote->marker_tail_end < quote->marker_secondary_end || quote->marker_tail_end > end - start)
            return XUI_ERROR_NOT_FOUND;
        header_end = range.syntax_start + quote->marker_tail_end;
    }
    while (start) {
        char previous;
        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
        result = doc_seq_read(t->draft->source, start - 1, &previous, 1); if (result != XUI_OK) return result;
        if (previous == '\r' || previous == '\n') break;
        start--;
    }
    /* Explicit empty quote lines can follow the last child's syntax range. */
    for (i = 0; i < count; i++) {
        doc_quote_indent_relative record;
        memcpy(&record, quote->syntax_aux->data + i * sizeof(record), sizeof(record));
        if (record.end > source_size - range.syntax_start) return XUI_DOC_ERROR_FORMAT;
        if (range.syntax_start + record.end > end) end = range.syntax_start + record.end;
    }
    if (end - start >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    result = doc_md_quote_child_prefixes(t, quote, quote->id, &prefixes, 0);
    if (result != XUI_OK) goto done;
    if (prefixes.count) qsort(prefixes.data, (size_t)prefixes.count, sizeof(*prefixes.data), doc_md_prefix_span_compare);
    source = doc_alloc(t->draft->allocator, (size_t)(end - start) + 1);
    if (!source) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    result = doc_seq_read(t->draft->source, start, source, end - start);
    if (result != XUI_OK) goto done;
    source[end - start] = 0; cursor = start;
    output.allocator = t->draft->allocator; output.plan = &plan;
    for (i = 0; i < count; i++) {
        doc_quote_indent_relative record;
        uint64_t marker, indent_end, line, p, column = 0, marker_column, target, cut, old_column, new_column;
        unsigned tab_left = 0;
        result = doc_txn_check(t, 0); if (result != XUI_OK) goto done;
        memcpy(&record, quote->syntax_aux->data + i * sizeof(record), sizeof(record));
        marker = range.syntax_start + record.marker; indent_end = range.syntax_start + record.end;
        if (marker < cursor || marker >= indent_end || indent_end > end || source[marker - start] != '>') {
            result = XUI_DOC_ERROR_FORMAT; goto done;
        }
        line = marker;
        while (line > start && source[line - 1 - start] != '\r' && source[line - 1 - start] != '\n') {
            if (!(line & 16383u)) { result = doc_txn_check(t, 0); if (result != XUI_OK) goto done; }
            line--;
        }
        if (!line && end >= 3 && !memcmp(source, "\xef\xbb\xbf", 3)) line = 3;
        for (p = line; p < marker; p++) {
            if (!(p & 16383u)) { result = doc_txn_check(t, 0); if (result != XUI_OK) goto done; }
            column = source[p - start] == '\t' ? (column + 4) & ~(uint64_t)3 : column + 1;
        }
        marker_column = column;
        if (column < record.marker_column || record.start_column > record.marker_column ||
            record.content_column <= record.marker_column || record.content_column > record.end_column) {
            result = XUI_DOC_ERROR_FORMAT; goto done;
        }
        target = column - record.marker_column + record.start_column;
        column = 0; cut = line;
        while (cut < marker && column < target) {
            uint64_t width = source[cut - start] == '\t' ? 4 - column % 4 : 1;
            if (width > target - column) { tab_left = (unsigned)(target - column); break; }
            column += width; cut++;
        }
        if (cut < cursor || column + tab_left != target) { result = XUI_ERROR_NOT_FOUND; goto done; }
        if (!i) { first_cut = cut; first_tab_left = tab_left; }
        for (p = cut; p < marker; p++) if (source[p - start] != ' ' && source[p - start] != '\t') {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
        if (!i && header_end) {
            /* The header is owned syntax, not a child paragraph. Remove its
             * entire line. A list item opener on that line must survive and
             * becomes the first body's prefix; other ancestors continue on
             * the body's existing line. Never copy the normalized type. */
            if (header_end < indent_end || header_end > end) { result = XUI_DOC_ERROR_FORMAT; goto done; }
            for (p = start; p < cut; p++) {
                if (!(p & 16383u)) { result = doc_txn_check(t, 0); if (result != XUI_OK) goto done; }
                if (doc_md_unwrap_parent_marker(t->draft, quote, p) == 2) carry_prefix = 1;
            }
            if (count == 1 && quote->parent != DOC_ROOT) carry_prefix = 1;
            if (carry_prefix) {
                doc_md_copy_source(&output, source + cursor - start, cursor, cut - cursor);
                if (tab_left) doc_md_write(&output, "   ", tab_left);
                header_column = target;
            } else if (!start && end >= 3 && !memcmp(source, "\xef\xbb\xbf", 3)) {
                doc_md_copy_source(&output, source, 0, 3);
            }
            cursor = header_end;
            if (output.error) { result = output.error; goto done; }
            continue;
        }
        if (!carry_prefix) {
            doc_md_copy_source(&output, source + cursor - start, cursor, cut - cursor);
            if (tab_left) doc_md_write(&output, "   ", tab_left);
        }
        old_column = marker_column + 1; new_column = carry_prefix ? header_column : target;
        carry_prefix = 0;
        /* The physical line may start before a dedented parser input. */
        target = marker_column - record.marker_column + record.content_column;
        for (p = marker + 1; p < indent_end; p++) {
            char ch = source[p - start]; uint64_t width, kept;
            if (!(p & 16383u)) { result = doc_txn_check(t, 0); if (result != XUI_OK) goto done; }
            if (ch != ' ' && ch != '\t') { result = XUI_DOC_ERROR_FORMAT; goto done; }
            width = ch == '\t' ? 4 - old_column % 4 : 1;
            kept = old_column + width > target ? old_column + width - (old_column > target ? old_column : target) : 0;
            if (kept) {
                if (kept == width && (ch != '\t' || width == 4 - new_column % 4))
                    doc_md_copy_source(&output, source + p - start, p, 1);
                else doc_md_write(&output, "    ", kept);
                new_column += kept;
            }
            old_column += width;
        }
        if (old_column != marker_column - record.marker_column + record.end_column) {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
        {
            uint64_t extended = indent_end;
            while (prefix_cursor < prefixes.count && prefixes.data[prefix_cursor].start <= extended) {
                if (prefixes.data[prefix_cursor].end > extended) extended = prefixes.data[prefix_cursor].end;
                prefix_cursor++;
            }
            if (extended > end) { result = XUI_DOC_ERROR_FORMAT; goto done; }
            for (p = indent_end; p < extended; p++) {
                char ch = source[p - start]; uint64_t width = ch == '\t' ? 4 - old_column % 4 : 1;
                if (!(p & 16383u)) { result = doc_txn_check(t, 0); if (result != XUI_OK) goto done; }
                if (ch == '\r' || ch == '\n') { result = XUI_DOC_ERROR_FORMAT; goto done; }
                if (ch == '\t' && width != 4 - new_column % 4) doc_md_write(&output, "    ", width);
                else doc_md_copy_source(&output, source + p - start, p, 1);
                old_column += width; new_column += width;
            }
            indent_end = extended;
        }
        cursor = indent_end;
        if (output.error) { result = output.error; goto done; }
    }
    if (carry_prefix) {
        uint64_t ending = header_end;
        if (ending > start && source[ending - 1 - start] == '\n') ending--;
        if (ending > start && source[ending - 1 - start] == '\r') ending--;
        doc_md_copy_source(&output, source + ending - start, ending, header_end - ending);
    }
    doc_md_copy_source(&output, source + cursor - start, cursor, end - cursor);
    result = output.error;
    /* Try the original minimal prefix patch first. If its semantics merge
     * blocks, try the two edge separators independently and then together.
     * Each attempt starts from the same committed draft and is fully parsed;
     * never accept changed definitions or publish a failed attempt. */
    for (attempt = 0; result == XUI_OK && attempt < 4; attempt++) {
        result = doc_markdown_shadow_begin(t, &candidate);
        if (result == XUI_OK && (attempt & 2)) result = doc_txn_source_patch(&candidate, end, end, back.data, back.size, 0);
        if (result == XUI_OK) result = doc_md_apply_source_plan(&candidate, start, end, &output, 0);
        if (result == XUI_OK && (attempt & 1)) result = doc_txn_source_patch(&candidate, front_at, front_at, front.data, front.size, 0);
        if (result == XUI_OK) result = doc_markdown_parse(&candidate);
        if (result == XUI_OK) result = doc_md_unwrap_references(t, quote, candidate.draft, &equal);
        if (result != XUI_OK) break;
        if (!equal) { result = XUI_ERROR_NOT_FOUND; break; }
        if (doc_semantic_equal(desired, candidate.draft)) break;
        doc_state_release(candidate.draft); doc_free(candidate.ops); memset(&candidate, 0, sizeof(candidate));
        if (attempt == 3) { result = XUI_ERROR_NOT_FOUND; break; }
        if (!attempt) result = doc_md_unwrap_separators(t, quote, start, first_cut, first_tab_left,
            end, source, &front, &back, &front_at);
    }
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft;
            candidate.draft = NULL; t->parse_op_start = t->count;
        }
    }
done:
    doc_state_release(candidate.draft); doc_free(candidate.ops);
    doc_free(source); doc_free(output.data); doc_free(plan.copies); doc_free(prefixes.data);
    doc_free(front.data); doc_free(back.data); return result;
}
typedef struct doc_md_unlist_source_item {
    doc_node* node;
    doc_node_source_range range;
    uint64_t start, end;
    int empty;
} doc_md_unlist_source_item;
typedef struct doc_md_unlist_source_items {
    doc_md_unlist_source_item* data;
    uint64_t count, capacity;
} doc_md_unlist_source_items;

/* Recognize the semantic operation, rather than guessing it from text: each
 * deleted item must promote its unchanged child subtrees to its list parent.
 * The complete desired tree is independently compared after parsing. */
static int doc_md_unlist_collect(xui_document_transaction t, doc_state* desired,
    doc_node* node, doc_md_unlist_source_items* items, unsigned depth)
{
    uint64_t i, child_count; int result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    if (!node || depth >= DOC_MAX_DEPTH) return XUI_DOC_ERROR_SCHEMA;
    child_count = doc_seq_size(node->children);
    if (node->kind == XUI_DOC_LIST_ITEM && !doc_index_get(desired->index, node->id)) {
        doc_node* list = doc_index_get(t->draft->index, node->parent);
        doc_md_unlist_source_item item = {0};
        item.empty = 1;
        if (!list || list->kind != XUI_DOC_LIST || !node->syntax_aux ||
            node->syntax_aux->size != sizeof(doc_list_first_indent_relative) ||
            (node->list_indents && node->list_indents->size % sizeof(doc_list_indent_relative)))
            return XUI_ERROR_NOT_FOUND;
        for (i = 0; i < child_count; i++) {
            uint64_t id = doc_seq_get_id(node->children, i);
            doc_node *before = doc_index_get(t->draft->index, id), *after = doc_index_get(desired->index, id);
            if (doc_semantic_empty_paragraph(t->draft, before) && !after) continue;
            item.empty = 0;
            if (!after || after->parent != list->parent ||
                !doc_semantic_subtree_equal(t->draft, id, desired, id)) return XUI_ERROR_NOT_FOUND;
        }
        item.node = node; doc_node_source_range_get(t->draft, node, &item.range);
        if (item.range.syntax_start == DOC_NONE || item.range.syntax_end == DOC_NONE ||
            item.range.syntax_start >= item.range.syntax_end ||
            item.range.syntax_end > doc_seq_size(t->draft->source)) return XUI_ERROR_NOT_FOUND;
        item.start = item.range.syntax_start; item.end = item.range.syntax_end;
        if (item.empty && item.end < doc_seq_size(t->draft->source)) {
            char ch;
            result = doc_seq_read(t->draft->source, item.end, &ch, 1); if (result != XUI_OK) return result;
            if (ch == '\r' || ch == '\n') {
                item.end++;
                if (ch == '\r' && item.end < doc_seq_size(t->draft->source)) {
                    result = doc_seq_read(t->draft->source, item.end, &ch, 1); if (result != XUI_OK) return result;
                    if (ch == '\n') item.end++;
                }
            }
        }
        while (item.start) {
            char ch;
            result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
            result = doc_seq_read(t->draft->source, item.start - 1, &ch, 1); if (result != XUI_OK) return result;
            if (ch == '\r' || ch == '\n') break;
            item.start--;
        }
        if (items->count == items->capacity) {
            uint64_t capacity = items->capacity ? items->capacity * 2 : 16;
            doc_md_unlist_source_item* next;
            if (capacity < items->capacity || capacity > SIZE_MAX / sizeof(*next)) return XUI_DOC_ERROR_LIMIT;
            next = doc_realloc(t->draft->allocator, items->data, (size_t)capacity * sizeof(*next));
            if (!next) return XUI_ERROR_OUT_OF_MEMORY;
            items->data = next; items->capacity = capacity;
        }
        items->data[items->count++] = item;
        return XUI_OK;
    }
    for (i = 0; i < child_count; i++) {
        result = doc_md_unlist_collect(t, desired, doc_index_get(t->draft->index,
            doc_seq_get_id(node->children, i)), items, depth + 1);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
static int doc_md_unlist_item_compare(const void* left, const void* right)
{
    const doc_md_unlist_source_item *a = left, *b = right;
    return a->start < b->start ? -1 : a->start > b->start;
}

/* A footnote opener can share the first item line, but subsequent body lines
 * use four logical indentation columns rather than the label's byte width.
 * Build inserted blank lines from the ancestor prefix before that opener. */
static int doc_md_unlist_separators(xui_document_transaction t, doc_node* item,
    uint64_t start, uint64_t cut, unsigned tab_left, uint64_t end,
    const char* source, doc_md_output* front, doc_md_output* back, uint64_t* front_at)
{
    doc_node* owner = doc_index_get(t->draft->index, item->parent);
    doc_node_source_range range; uint64_t i; int result, padding = 0;
    for (; owner && owner->id != DOC_ROOT && owner->kind != XUI_DOC_FOOTNOTE;
        owner = doc_index_get(t->draft->index, owner->parent)) {}
    if (owner && owner->kind == XUI_DOC_FOOTNOTE) {
        doc_node_source_range_get(t->draft, owner, &range);
        for (i = 0; i < doc_seq_size(t->draft->references); i += sizeof(xui_doc_reference_definition_t)) {
            xui_doc_reference_definition_t ref;
            result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
            result = doc_seq_read(t->draft->references, i, &ref, sizeof(ref)); if (result != XUI_OK) return result;
            if (ref.iKind == XUI_DOC_REFERENCE_FOOTNOTE && ref.iSourceStart == range.syntax_start &&
                ref.iSourceEnd == range.syntax_end && ref.iLabelStart >= 2 &&
                ref.iLabelStart - 2 >= start && ref.iLabelEnd + 2 <= cut) {
                cut = ref.iLabelStart - 2; tab_left = 0; padding = 1; break;
            }
        }
    }
    result = doc_md_unwrap_separators(t, item, start, cut, tab_left, end, source, front, back, front_at);
    if (result != XUI_OK || !padding) return result;
    for (i = 0; i < 2; i++) {
        doc_md_output *old = i ? back : front, padded = {0}; uint64_t eol = 1;
        if (old->size >= 2 && old->data[old->size - 2] == '\r' && old->data[old->size - 1] == '\n') eol = 2;
        padded.allocator = t->draft->allocator;
        doc_md_write(&padded, old->data, old->size - eol); doc_md_write(&padded, "    ", 4);
        doc_md_write(&padded, old->data + old->size - eol, eol);
        doc_free(old->data); *old = padded;
        if (old->error) return old->error;
    }
    return XUI_OK;
}

static int doc_md_unlist_boundary_blank(xui_document_transaction t, doc_node* item,
    uint64_t at, int previous, int* blank)
{
    uint64_t size = doc_seq_size(t->draft->source); char ch; int result;
    *blank = 1;
    if (previous) {
        if (!at) return XUI_OK;
        result = doc_seq_read(t->draft->source, at - 1, &ch, 1); if (result != XUI_OK) return result;
        if (ch == '\n') {
            at--;
            if (at) { result = doc_seq_read(t->draft->source, at - 1, &ch, 1); if (result != XUI_OK) return result; }
        }
        if (at && ch == '\r') at--;
        while (at) {
            result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
            result = doc_seq_read(t->draft->source, at - 1, &ch, 1); if (result != XUI_OK) return result;
            if (ch == '\r' || ch == '\n') break;
            if (ch != ' ' && ch != '\t' && !(ch == '>' && doc_md_unwrap_parent_marker(t->draft, item, at - 1) == 1)) {
                *blank = 0; break;
            }
            at--;
        }
    } else {
        while (at < size) {
            result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
            result = doc_seq_read(t->draft->source, at, &ch, 1); if (result != XUI_OK) return result;
            if (ch == '\r' || ch == '\n') break;
            if (ch != ' ' && ch != '\t' && !(ch == '>' && doc_md_unwrap_parent_marker(t->draft, item, at) == 1)) {
                *blank = 0; break;
            }
            at++;
        }
    }
    return XUI_OK;
}

/* The output can already end in a separator added for the preceding selected
 * item. Coalesce only new boundary blanks; retained source trivia stays copied. */
static int doc_md_unlist_output_blank(const doc_md_output* output)
{
    uint64_t at = output->size;
    if (!at || (output->data[at - 1] != '\r' && output->data[at - 1] != '\n')) return 0;
    if (output->data[--at] == '\n' && at && output->data[at - 1] == '\r') at--;
    while (at && output->data[at - 1] != '\r' && output->data[at - 1] != '\n') {
        char ch = output->data[--at];
        if (ch != ' ' && ch != '\t' && ch != '>') return 0;
    }
    return 1;
}
static int doc_md_unlist_body_blank(xui_document_transaction t, const doc_md_output* body, int* blank)
{
    uint64_t i; int result;
    *blank = !!body->size;
    for (i = 0; i < body->size; i++) {
        char ch = body->data[i];
        if (!(i & 16383)) { result = doc_txn_check(t, 0); if (result != XUI_OK) return result; }
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' && ch != '>') { *blank = 0; break; }
    }
    return XUI_OK;
}
static int doc_md_unlist_item_write(xui_document_transaction t,
    const doc_md_unlist_source_item* item, const char* source, uint64_t source_base,
    doc_md_output* output, uint64_t* first_cut, unsigned* first_tab_left)
{
    doc_node* node = item->node;
    doc_md_prefix_spans prefixes = {0};
    uint64_t count = 1 + (node->list_indents ? node->list_indents->size / sizeof(doc_list_indent_relative) : 0);
    uint64_t i, cursor = item->start, prefix_cursor = 0; int result = XUI_OK, empty = item->empty;
    if (empty) for (i = 0; i < doc_seq_size(t->draft->references); i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        result = doc_txn_check(t, 0); if (result != XUI_OK) goto done;
        result = doc_seq_read(t->draft->references, i, &ref, sizeof(ref)); if (result != XUI_OK) goto done;
        if (ref.iSourceStart >= item->range.syntax_start && ref.iSourceEnd <= item->range.syntax_end) { empty = 0; break; }
    }
    for (i = 0; i < doc_seq_size(node->children); i++) {
        result = doc_md_quote_child_prefixes(t, doc_index_get(t->draft->index,
            doc_seq_get_id(node->children, i)), node->id, &prefixes, 0);
        if (result != XUI_OK) goto done;
    }
    if (prefixes.count) qsort(prefixes.data, (size_t)prefixes.count, sizeof(*prefixes.data), doc_md_prefix_span_compare);
    for (i = 0; i < count; i++) {
        doc_list_indent_relative record;
        uint64_t beg, end, line, p, column = 0, delta, target, cut, content, old_column, new_column;
        unsigned tab_left = 0;
        result = doc_txn_check(t, 0); if (result != XUI_OK) goto done;
        /* The first five marker/column fields have identical layouts. The
         * first-line blob's additional raw leading start is kept for source
         * indexing; this existing removal plan still needs the marker. */
        memcpy(&record, i ? node->list_indents->data + (i - 1) * sizeof(record) : node->syntax_aux->data, sizeof(record));
        beg = item->range.syntax_start + record.start; end = item->range.syntax_start + record.end;
        if (beg < cursor || beg >= end || end > item->end || record.start_column >= record.content_column ||
            record.content_column > record.end_column) { result = XUI_ERROR_NOT_FOUND; goto done; }
        line = beg;
        while (line > item->start && source[line - 1 - source_base] != '\r' && source[line - 1 - source_base] != '\n') {
            if (!(line & 16383u)) { result = doc_txn_check(t, 0); if (result != XUI_OK) goto done; }
            line--;
        }
        if (!line && item->end >= 3 && !memcmp(source, "\xef\xbb\xbf", 3)) line = 3;
        for (p = line; p < end; p++) {
            if (!(p & 16383u)) { result = doc_txn_check(t, 0); if (result != XUI_OK) goto done; }
            column = source[p - source_base] == '\t' ? (column + 4) & ~(uint64_t)3 : column + 1;
        }
        if (column < record.end_column) { result = XUI_ERROR_NOT_FOUND; goto done; }
        delta = column - record.end_column;
        target = delta + record.start_column; content = delta + record.content_column;
        column = 0; cut = line;
        while (cut < (i ? end : beg) && column < target) {
            uint64_t width = source[cut - source_base] == '\t' ? 4 - column % 4 : 1;
            if (width > target - column) { tab_left = (unsigned)(target - column); break; }
            column += width; cut++;
        }
        if (cut < cursor || column + tab_left != target) { result = XUI_ERROR_NOT_FOUND; goto done; }
        for (p = cut; p < beg; p++) if (source[p - source_base] != ' ' && source[p - source_base] != '\t') {
            result = XUI_ERROR_NOT_FOUND; goto done;
        }
        if (!i) { *first_cut = cut; *first_tab_left = tab_left; }
        doc_md_copy_source(output, source + cursor - source_base, cursor, cut - cursor);
        if (tab_left) doc_md_write(output, "   ", tab_left);
        if (empty) {
            /* An item with no body/definitions owns its otherwise empty line.
             * Retain enclosing markers and BOM, but do not leave root trivia
             * in place of a deleted empty structural block. */
            if (cut > line || tab_left) {
                uint64_t eol = end;
                while (eol < item->end && source[eol - source_base] != '\r' && source[eol - source_base] != '\n') eol++;
                if (eol < item->end) {
                    uint64_t bytes = source[eol - source_base] == '\r' && eol + 1 < item->end && source[eol + 1 - source_base] == '\n' ? 2 : 1;
                    doc_md_copy_source(output, source + eol - source_base, eol, bytes);
                }
            }
            result = output->error; goto done;
        }
        new_column = target; old_column = column;
        p = cut;
        if (!i) {
            p = item->range.syntax_start + (node->marker_secondary_end ? node->marker_secondary_end : node->marker_primary_end);
            if (p > end) { result = XUI_ERROR_NOT_FOUND; goto done; }
            old_column = 0;
            for (uint64_t k = line; k < p; k++) {
                if (!(k & 16383u)) { result = doc_txn_check(t, 0); if (result != XUI_OK) goto done; }
                old_column = source[k - source_base] == '\t' ? (old_column + 4) & ~(uint64_t)3 : old_column + 1;
            }
        }
        for (; p < end; p++) {
            char ch = source[p - source_base]; uint64_t width, kept;
            if (!(p & 16383u)) { result = doc_txn_check(t, 0); if (result != XUI_OK) goto done; }
            if (ch != ' ' && ch != '\t') { result = XUI_ERROR_NOT_FOUND; goto done; }
            width = ch == '\t' ? 4 - old_column % 4 : 1;
            kept = old_column + width > content ? old_column + width - (old_column > content ? old_column : content) : 0;
            if (kept) {
                if (kept == width && (ch != '\t' || width == 4 - new_column % 4))
                    doc_md_copy_source(output, source + p - source_base, p, 1);
                else doc_md_write(output, "    ", kept);
                new_column += kept;
            }
            old_column += width;
        }
        if (old_column != delta + record.end_column) { result = XUI_ERROR_NOT_FOUND; goto done; }
        {
            uint64_t extended = end;
            while (prefix_cursor < prefixes.count && prefixes.data[prefix_cursor].start <= extended) {
                if (prefixes.data[prefix_cursor].end > extended) extended = prefixes.data[prefix_cursor].end;
                prefix_cursor++;
            }
            if (extended > item->end) { result = XUI_ERROR_NOT_FOUND; goto done; }
            for (p = end; p < extended; p++) {
                char ch = source[p - source_base]; uint64_t width = ch == '\t' ? 4 - old_column % 4 : 1;
                if (!(p & 16383u)) { result = doc_txn_check(t, 0); if (result != XUI_OK) goto done; }
                if (ch == '\r' || ch == '\n') { result = XUI_ERROR_NOT_FOUND; goto done; }
                if (ch == '\t' && width != 4 - new_column % 4) doc_md_write(output, "    ", width);
                else doc_md_copy_source(output, source + p - source_base, p, 1);
                old_column += width; new_column += width;
            }
            end = extended;
        }
        cursor = end;
        if (output->error) { result = output->error; goto done; }
    }
    doc_md_copy_source(output, source + cursor - source_base, cursor, item->end - cursor);
    result = output->error;
done:
    doc_free(prefixes.data); return result;
}

/* Surviving lists can become adjacent after several original containers
 * disappear. Compare the final sibling wrappers, regardless of their old
 * parents; only the later wrapper's parser-owned markers may change. */
static int doc_md_unlist_empty_suffixes(xui_document_transaction t, doc_state* desired,
    const doc_md_unlist_source_items* items, xui_document_transaction candidate)
{
    uint64_t i, previous_list = 0; int result;
    for (i = items->count; i-- > 0;) {
        doc_node *item = items->data[i].node, *list = doc_index_get(t->draft->index, item->parent);
        uint64_t index = doc_child_index(list, item->id), j, suffix = index + 1, right_index;
        doc_node *tail, *first, *parent, *left, *left_item;
        doc_node_source_range range; uint64_t at; char replacement, marker, left_marker;
        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
        if (list->id == previous_list) continue;
        previous_list = list->id;
        if (suffix >= doc_seq_size(list->children)) continue;
        first = doc_index_get(desired->index, doc_seq_get_id(list->children, suffix));
        tail = first ? doc_index_get(desired->index, first->parent) : NULL;
        if (!tail || tail->kind != XUI_DOC_LIST || tail->parent != list->parent ||
            doc_child_index(tail, first->id) != 0) continue;
        parent = doc_index_get(desired->index, tail->parent);
        right_index = doc_child_index(parent, tail->id);
        if (!right_index || right_index == DOC_NONE) continue;
        left = doc_index_get(desired->index, doc_seq_get_id(parent->children, right_index - 1));
        if (!left || left->kind != XUI_DOC_LIST || !doc_seq_size(left->children) ||
            ((left->attrs->iFlags ^ tail->attrs->iFlags) & XUI_DOC_ORDERED)) continue;
        left_item = doc_index_get(t->draft->index, doc_seq_get_id(left->children, 0));
        first = doc_index_get(t->draft->index, first->id);
        if (!first || !left_item || first->kind != XUI_DOC_LIST_ITEM || left_item->kind != XUI_DOC_LIST_ITEM)
            return XUI_DOC_ERROR_UNREPRESENTABLE;
        doc_node_source_range_get(t->draft, left_item, &range);
        if (range.syntax_start == DOC_NONE || left_item->marker_primary_end <= left_item->marker_primary_start)
            return XUI_DOC_ERROR_UNREPRESENTABLE;
        at = range.syntax_start + left_item->marker_primary_end - 1;
        result = doc_seq_read(t->draft->source, at, &left_marker, 1); if (result != XUI_OK) return result;
        doc_node_source_range_get(t->draft, first, &range);
        if (range.syntax_start == DOC_NONE || first->marker_primary_end <= first->marker_primary_start)
            return XUI_DOC_ERROR_UNREPRESENTABLE;
        at = range.syntax_start + first->marker_primary_end - 1;
        result = doc_seq_read(t->draft->source, at, &marker, 1); if (result != XUI_OK) return result;
        if (marker != left_marker) continue;
        replacement = marker == '.' ? ')' : marker == ')' ? '.' : marker == '+' ? '-' : '+';
        for (j = doc_seq_size(list->children); j-- > suffix;) {
            doc_node* old = doc_index_get(t->draft->index, doc_seq_get_id(list->children, j));
            doc_node* now = doc_index_get(desired->index, old->id); char ch;
            result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
            if (!now || now->parent != tail->id) return XUI_DOC_ERROR_UNREPRESENTABLE;
            doc_node_source_range_get(t->draft, old, &range);
            if (range.syntax_start == DOC_NONE || old->marker_primary_end <= old->marker_primary_start)
                return XUI_DOC_ERROR_UNREPRESENTABLE;
            at = range.syntax_start + old->marker_primary_end - 1;
            result = doc_seq_read(t->draft->source, at, &ch, 1); if (result != XUI_OK) return result;
            if (ch != marker) return XUI_DOC_ERROR_UNREPRESENTABLE;
            result = doc_txn_source_patch(candidate, at, at + 1, &replacement, 1, 0); if (result != XUI_OK) return result;
        }
    }
    return XUI_OK;
}

/* Tightness is derived from the remaining Markdown, including blank lines
 * inside the items that survive. Unlisting the only loose item can make its
 * head or suffix tight again. Normalize only wrappers affected by this
 * promotion, then still require the complete requested semantic tree. */
static int doc_md_unlist_list_affected(doc_state* old, const doc_node* list,
    const doc_md_unlist_source_items* items, doc_state* desired)
{
    uint64_t i, j;
    for (i = 0; i < items->count; i++) {
        doc_node* ancestor = doc_index_get(old->index, items->data[i].node->parent);
        for (; ancestor; ancestor = doc_index_get(old->index, ancestor->parent))
            if (ancestor->id == list->id && ancestor->kind == XUI_DOC_LIST) return 1;
        if (!doc_index_get(old->index, list->id) && doc_seq_size(list->children)) {
            doc_node* original = doc_index_get(old->index, items->data[i].node->parent);
            if (list->parent != original->parent) continue;
            for (j = 0; j < doc_seq_size(list->children); j++) {
                doc_node* child = doc_index_get(old->index, doc_seq_get_id(list->children, j));
                if (!child || child->kind != XUI_DOC_LIST_ITEM || child->parent != original->id ||
                    !doc_semantic_subtree_equal(old, child->id, desired, child->id)) break;
            }
            if (j == doc_seq_size(list->children)) return 1;
        }
    }
    return 0;
}
static int doc_md_unlist_normalize_lists(xui_document_transaction t, xui_document_transaction shadow,
    doc_state* actual, uint64_t desired_id, uint64_t actual_id, const doc_md_unlist_source_items* items, unsigned depth)
{
    doc_node* desired = doc_index_get(shadow->draft->index, desired_id);
    doc_node* parsed = doc_index_get(actual->index, actual_id);
    uint64_t i, children; int result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    if (depth >= DOC_MAX_DEPTH) return XUI_DOC_ERROR_LIMIT;
    if (!desired || !parsed || desired->kind != parsed->kind) return XUI_ERROR_NOT_FOUND;
    if (desired->kind == XUI_DOC_LIST && ((desired->attrs->iFlags ^ parsed->attrs->iFlags) & XUI_DOC_TIGHT)) {
        xui_doc_attributes_t attrs;
        if (!doc_md_unlist_list_affected(t->draft, desired, items, shadow->draft)) return XUI_ERROR_NOT_FOUND;
        attrs = *desired->attrs;
        attrs.iFlags = (attrs.iFlags & ~XUI_DOC_TIGHT) | (parsed->attrs->iFlags & XUI_DOC_TIGHT);
        result = xuiDocumentTxnSetAttributes(shadow, desired_id, &attrs);
        if (result != XUI_OK) return result;
        desired = doc_index_get(shadow->draft->index, desired_id);
    }
    /* Inline segmentation need not match to reach the nested block wrappers.
     * Full semantic equality below validates all inline data and attributes. */
    if (desired->kind == XUI_DOC_PARAGRAPH || desired->kind == XUI_DOC_HEADING || doc_text_kind(desired->kind)) return XUI_OK;
    children = doc_seq_size(desired->children);
    if (children != doc_seq_size(parsed->children)) return XUI_ERROR_NOT_FOUND;
    for (i = 0; i < children; i++) {
        result = doc_md_unlist_normalize_lists(t, shadow, actual, doc_seq_get_id(desired->children, i),
            doc_seq_get_id(parsed->children, i), items, depth + 1);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}

static int doc_md_unlist_source_patch(xui_document_transaction t, xui_document_transaction shadow)
{
    doc_state* desired = shadow->draft;
    doc_md_unlist_source_items items = {0};
    doc_md_source_plan plan = {0}; doc_md_output output = {0};
    struct xui_doc_transaction_t candidate = {0};
    char* source = NULL; uint64_t start, end, cursor, i; int result, equal;
    result = doc_md_unlist_collect(t, desired, doc_index_get(t->draft->index, DOC_ROOT), &items, 0);
    if (result != XUI_OK) goto done;
    if (!items.count) { result = XUI_ERROR_NOT_FOUND; goto done; }
    qsort(items.data, (size_t)items.count, sizeof(*items.data), doc_md_unlist_item_compare);
    start = items.data[0].start; end = items.data[items.count - 1].end;
    if (end <= start || end - start >= SIZE_MAX) { result = XUI_DOC_ERROR_LIMIT; goto done; }
    source = doc_alloc(t->draft->allocator, (size_t)(end - start) + 1);
    if (!source) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    result = doc_seq_read(t->draft->source, start, source, end - start); if (result != XUI_OK) goto done;
    source[end - start] = 0; cursor = start; output.allocator = t->draft->allocator; output.plan = &plan;
    for (i = 0; i < items.count; i++) {
        doc_md_unlist_source_item* item = &items.data[i];
        doc_node *list = doc_index_get(t->draft->index, item->node->parent),
            *parent = doc_index_get(t->draft->index, list->parent);
        doc_md_source_plan item_plan = {0};
        doc_md_output body = {0}, front = {0}, back = {0};
        uint64_t first_cut = 0, front_at = 0, index = doc_child_index(list, item->node->id);
        uint64_t list_index = doc_child_index(parent, list->id);
        unsigned tab_left = 0; int blank = 0, blank_body = 0;
        if (item->start < cursor || index == DOC_NONE || list_index == DOC_NONE) { result = XUI_ERROR_NOT_FOUND; break; }
        doc_md_copy_source(&output, source + cursor - start, cursor, item->start - cursor);
        body.allocator = t->draft->allocator; body.plan = &item_plan;
        result = doc_md_unlist_item_write(t, item, source, start, &body, &first_cut, &tab_left);
        if (result == XUI_OK && item->empty) result = doc_md_unlist_body_blank(t, &body, &blank_body);
        if (result == XUI_OK) result = doc_md_unlist_separators(t, item->node, item->start, first_cut,
            tab_left, item->end, source + item->start - start, &front, &back, &front_at);
        if (result == XUI_OK && (index || list_index) &&
            !(i && items.data[i - 1].node->parent == item->node->parent &&
                doc_child_index(list, items.data[i - 1].node->id) + 1 == index)) {
            result = doc_md_unlist_boundary_blank(t, item->node, item->start, 1, &blank);
            if (result == XUI_OK && !blank && !doc_md_unlist_output_blank(&output) &&
                !blank_body) doc_md_write(&output, front.data, front.size);
        }
        if (result == XUI_OK) {
            uint64_t at = 0, p;
            for (p = 0; p < item_plan.count; p++) {
                doc_md_source_copy* copy = &item_plan.copies[p];
                doc_md_write(&output, body.data + at, copy->output - at);
                doc_md_copy_source(&output, body.data + copy->output, copy->source, copy->bytes);
                at = copy->output + copy->bytes;
            }
            doc_md_write(&output, body.data ? body.data + at : "", body.size - at);
        }
        if (result == XUI_OK && body.size && !doc_md_unlist_output_blank(&output) &&
            (index + 1 < doc_seq_size(list->children) || list_index + 1 < doc_seq_size(parent->children))) {
            result = doc_md_unlist_boundary_blank(t, item->node, item->end, 1, &blank);
            if (result == XUI_OK && !blank) result = doc_md_unlist_boundary_blank(t, item->node, item->end, 0, &blank);
            if (result == XUI_OK && !blank) doc_md_write(&output, back.data, back.size);
        }
        doc_free(body.data); doc_free(item_plan.copies); doc_free(front.data); doc_free(back.data);
        if (result != XUI_OK) break;
        cursor = item->end;
    }
    if (result == XUI_OK) doc_md_copy_source(&output, source + cursor - start, cursor, end - cursor);
    if (result == XUI_OK) result = output.error;
    if (result == XUI_OK) result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_md_unlist_empty_suffixes(t, desired, &items, &candidate);
    if (result == XUI_OK) result = doc_md_apply_source_plan(&candidate, start, end, &output, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK) result = doc_md_unlist_normalize_lists(t, shadow, candidate.draft,
        DOC_ROOT, DOC_ROOT, &items, 0);
    if (result == XUI_OK) result = doc_md_unwrap_references(t, items.data[0].node, candidate.draft, &equal);
    if (result == XUI_OK && (!equal || !doc_semantic_equal(desired, candidate.draft))) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) { doc_state_release(t->draft); t->draft = candidate.draft; candidate.draft = NULL; t->parse_op_start = t->count; }
    }
done:
    /* Once promotion has been recognized, an unproven source candidate must
     * not fall through to canonical whole-container serialization. */
    if (result == XUI_ERROR_NOT_FOUND && items.count) result = XUI_DOC_ERROR_UNREPRESENTABLE;
    doc_state_release(candidate.draft); doc_free(candidate.ops);
    doc_free(items.data); doc_free(source); doc_free(output.data); doc_free(plan.copies); return result;
}

int doc_markdown_shadow_end_unwrap_quote(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow, int result, const xui_doc_position_t* at,
    const xui_doc_position_t* target, xui_doc_position_t* caret)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_md_unwrap_quote_source_patch(t, shadow->draft, at);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL, 0);
        if (result == XUI_OK) result = doc_markdown_accept(t, shadow,0);
    }
    if (result == XUI_OK && target && caret) *caret = doc_markdown_resolve_caret(t, shadow->draft, *target);
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
/* Parser-owned inline ranges are shared by node and semantic-range deletion.
 * Only wrappers wholly occupied by the selected content are consumed. */
static int doc_md_inline_owned_source_range(xui_document_transaction t,
    uint64_t start, uint64_t end, int allow_literal,
    uint64_t* source_start, uint64_t* source_end)
{
    xui_doc_inline_syntax_t owned = {0};
    uint64_t i, count, owner = DOC_NONE;
    int result;
    if (start == DOC_NONE || end == DOC_NONE || end < start || end > doc_seq_size(t->draft->source))
        return XUI_ERROR_NOT_FOUND;
    count = doc_seq_size(t->draft->inline_syntax) / sizeof(owned);
    for (i = 0; i < count; i++) {
        xui_doc_inline_syntax_t syntax;
        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
        result = doc_seq_read_syntax(t->draft->inline_syntax, i, &syntax); if (result != XUI_OK) return result;
        if (syntax.iKind == XUI_DOC_SYNTAX_DESTINATION || syntax.iKind == XUI_DOC_SYNTAX_TITLE ||
            syntax.iContentStart != start || syntax.iContentEnd != end) continue;
        if (syntax.iSourceStart > start || syntax.iSourceEnd < end ||
            syntax.iSourceEnd > doc_seq_size(t->draft->source)) return XUI_ERROR_NOT_FOUND;
        if (owner != DOC_NONE) return XUI_ERROR_NOT_FOUND;
        owner = i; owned = syntax;
    }
    if (owner != DOC_NONE) {
        start = owned.iSourceStart; end = owned.iSourceEnd;
        for (i = 0; owned.iParentIndex != DOC_NONE && i < count; i++) {
            xui_doc_inline_syntax_t enclosing;
            if (owned.iParentIndex >= count) return XUI_ERROR_NOT_FOUND;
            result = doc_seq_read_syntax(t->draft->inline_syntax, owned.iParentIndex, &enclosing);
            if (result != XUI_OK) return result;
            if (enclosing.iContentStart != start || enclosing.iContentEnd != end) break;
            if (enclosing.iSourceStart > start || enclosing.iSourceEnd < end ||
                enclosing.iSourceEnd > doc_seq_size(t->draft->source)) return XUI_ERROR_NOT_FOUND;
            owned = enclosing; start = owned.iSourceStart; end = owned.iSourceEnd;
        }
        if (i == count && owned.iParentIndex != DOC_NONE) return XUI_ERROR_NOT_FOUND;
    } else if (!allow_literal || start == end) return XUI_ERROR_NOT_FOUND;
    if (end <= start) return XUI_ERROR_NOT_FOUND;
    *source_start = start; *source_end = end; return XUI_OK;
}
static int doc_markdown_inline_delete_source_patch(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow)
{
    struct xui_doc_transaction_t candidate = {0};
    doc_node *parent, *first, *last, *ancestor;
    doc_node_source_range range;
    uint64_t i, left_index, right_index, start, end, unused;
    uint64_t editable_start = DOC_NONE, editable_end = DOC_NONE;
    int result;
    if (!shadow->count || shadow->ops[0].iKind != XUI_DOC_OP_DELETE)
        return XUI_ERROR_NOT_FOUND;
    parent = doc_index_get(t->draft->index, shadow->ops[0].iParentId);
    right_index = shadow->ops[0].iOffset;
    if (!parent || (parent->kind != XUI_DOC_PARAGRAPH && parent->kind != XUI_DOC_HEADING) ||
        right_index >= doc_seq_size(parent->children) || right_index < shadow->count - 1)
        return XUI_ERROR_NOT_FOUND;
    left_index = right_index - (shadow->count - 1);
    for (i = 0; i < shadow->count; i++) {
        const xui_doc_operation_t* op = &shadow->ops[i];
        doc_node* node;
        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
        if (op->iKind != XUI_DOC_OP_DELETE || op->iParentId != parent->id ||
            op->iOldLength != 1 || op->iOffset != right_index - i ||
            doc_seq_get_id(parent->children, op->iOffset) != op->iNodeId)
            return XUI_ERROR_NOT_FOUND;
        node = doc_index_get(t->draft->index, op->iNodeId);
        if (!node || doc_seq_size(node->children)) return XUI_ERROR_NOT_FOUND;
    }
    first = doc_index_get(t->draft->index, doc_seq_get_id(parent->children, left_index));
    last = doc_index_get(t->draft->index, doc_seq_get_id(parent->children, right_index));
    doc_node_source_range_get(t->draft, first, &range);
    result = doc_md_inline_owned_source_range(t, range.source_start, range.source_end,
        first->kind == XUI_DOC_TEXT || first->kind == XUI_DOC_HTML ||
        first->kind == XUI_DOC_SOFT_BREAK || first->kind == XUI_DOC_HARD_BREAK, &start, &end);
    if (result != XUI_OK) return result;
    if (first != last) {
        uint64_t final_start, final_end;
        doc_node_source_range_get(t->draft, last, &range);
        result = doc_md_inline_owned_source_range(t, range.source_start, range.source_end,
            last->kind == XUI_DOC_TEXT || last->kind == XUI_DOC_HTML ||
            last->kind == XUI_DOC_SOFT_BREAK || last->kind == XUI_DOC_HARD_BREAK, &final_start, &final_end);
        if (result != XUI_OK) return result;
        if (final_start < start || final_end < end) return XUI_ERROR_NOT_FOUND;
        end = final_end;
        /* A format wrapper may cover several parser text/entity children. */
        result = doc_md_inline_owned_source_range(t, start, end, 1, &start, &unused);
        if (result != XUI_OK) return result;
        end = unused;
    }
    if ((last->kind == XUI_DOC_SOFT_BREAK || last->kind == XUI_DOC_HARD_BREAK) &&
        right_index + 1 < doc_seq_size(parent->children)) {
        doc_node* next = doc_index_get(t->draft->index, doc_seq_get_id(parent->children, right_index + 1));
        uint64_t next_start, next_end;
        int quote_markers = 0;
        if (!next) return XUI_ERROR_NOT_FOUND;
        doc_node_source_range_get(t->draft, next, &range);
        result = doc_md_inline_owned_source_range(t, range.source_start, range.source_end,
            next->kind == XUI_DOC_TEXT || next->kind == XUI_DOC_HTML, &next_start, &next_end);
        if (result != XUI_OK) return result;
        if (next_start < end) return XUI_ERROR_NOT_FOUND;
        /* Deleting a break joins physical lines. The source-confirmed gap to
         * the next inline owner may contain only continuation trivia; consume
         * it together with the break. Unrecognized delimiters are retained. */
        for (ancestor = parent; ancestor; ancestor = doc_index_get(t->draft->index, ancestor->parent))
            if (ancestor->kind == XUI_DOC_QUOTE) quote_markers = 1;
        result = doc_md_source_gap_trivia(t, t->draft, end, next_start, quote_markers);
        if (result != XUI_OK) return result;
        end = next_start;
    }
    for (ancestor = parent; ancestor; ancestor = doc_index_get(t->draft->index, ancestor->parent)) {
        if (ancestor->kind == XUI_DOC_FOOTNOTE) {
            doc_node_source_range source; doc_node_source_range_get(t->draft, ancestor, &source);
            editable_start = source.syntax_start; editable_end = source.syntax_end; break;
        }
    }
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, start, end, "", 0, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_semantic_markdown_projection_equal(shadow->draft, candidate.draft)) {
        /* Removing the first inline child can expose a literal space to the
         * block parser's leading-whitespace trimming. Escape that one proven
         * direct byte; the remainder of the container stays byte-for-byte.
         * The second full parse must still prove the complete semantic intent. */
        doc_node* next = left_index == 0 && right_index + 1 < doc_seq_size(parent->children) ?
            doc_index_get(t->draft->index, doc_seq_get_id(parent->children, right_index + 1)) : NULL;
        doc_node_source_range next_range;
        char raw = 0, text = 0;
        result = XUI_ERROR_NOT_FOUND;
        if (next && next->kind == XUI_DOC_TEXT && doc_seq_size(next->text)) {
            doc_node_source_range_get(t->draft, next, &next_range);
            if (next_range.source_start == end && end < doc_seq_size(t->draft->source) &&
                doc_seq_read(t->draft->source, end, &raw, 1) == XUI_OK && raw == ' ' &&
                doc_seq_read(next->text, 0, &text, 1) == XUI_OK && text == ' ') {
                doc_state_release(candidate.draft); doc_free(candidate.ops);
                memset(&candidate, 0, sizeof(candidate));
                result = doc_markdown_shadow_begin(t, &candidate);
                if (result == XUI_OK) result = doc_txn_source_patch(&candidate, start, end + 1, "&#32;", 5, 0);
                if (result == XUI_OK) result = doc_markdown_parse(&candidate);
            }
        }
    }
    if (result == XUI_OK && (!doc_semantic_markdown_projection_equal(shadow->draft, candidate.draft) ||
        !doc_md_reference_spelling_equal_except_footnote(t->draft->references, t->draft->source,
            candidate.draft->references, candidate.draft->source, editable_start, editable_end)))
        result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft;
            candidate.draft = NULL; t->parse_op_start = t->count;
        }
    }
    doc_state_release(candidate.draft); doc_free(candidate.ops); return result;
}
/* A direct join has the same source intent as deleting only the boundary
 * between adjacent paragraphs. Reuse the authoritative range plan so all
 * surviving inline spelling and hidden definitions remain parser-verified.
 * Other shadow commands must not accidentally enter this path. */
static int doc_markdown_join_source_patch(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow)
{
    const xui_doc_operation_t* merge = NULL;
    doc_node *left, *right, *parent;
    xui_doc_range_t range = {0};
    uint64_t i;
    for (i = 0; i < shadow->count; i++) {
        if (shadow->ops[i].iKind != XUI_DOC_OP_MERGE) continue;
        if (merge) return XUI_ERROR_NOT_FOUND;
        merge = &shadow->ops[i];
    }
    if (!merge) return XUI_ERROR_NOT_FOUND;
    left = doc_index_get(t->draft->index, merge->iNodeId);
    right = doc_index_get(t->draft->index, merge->iOtherNodeId);
    if (!left || !right ||
        (left->kind != XUI_DOC_PARAGRAPH && left->kind != XUI_DOC_HEADING) ||
        (right->kind != XUI_DOC_PARAGRAPH && right->kind != XUI_DOC_HEADING) ||
        left->parent != right->parent || doc_index_get(shadow->draft->index, right->id))
        return XUI_ERROR_NOT_FOUND;
    parent = doc_index_get(t->draft->index, left->parent);
    if (!parent || doc_child_index(parent, right->id) != doc_child_index(parent, left->id) + 1)
        return XUI_ERROR_NOT_FOUND;
    range.tAnchor.iSize = sizeof(range.tAnchor);
    range.tAnchor.iDocumentId = t->document->identity;
    range.tAnchor.iRevision = t->base_revision;
    range.tAnchor.iNodeId = left->id;
    range.tAnchor.iOffset = doc_seq_size(left->children);
    range.tAnchor.iKind = XUI_DOC_POSITION_GAP;
    range.tAnchor.iAffinity = XUI_DOC_AFTER;
    range.tCaret = range.tAnchor;
    range.tCaret.iNodeId = right->id;
    range.tCaret.iOffset = 0;
    return doc_markdown_range_source_patch(t, &range, "", 0, shadow->draft);
}
/* A same-parent move changes sibling order, not block spelling. Rotate the
 * parser-owned block ranges and their following trivia in the SourceStore.
 * Persistent sequence splices retain the original payload blobs; the whole
 * parse and definition-spelling checks below still prove the semantic intent. */
static int doc_md_source_rotate(xui_document_transaction t,
    uint64_t start, uint64_t split, uint64_t end)
{
    doc_sequence *prefix = NULL, *left = NULL, *without = NULL, *rotated = NULL;
    doc_sequence *separator_prefix = NULL, *separator = NULL, *payload = NULL;
    doc_sequence *before;
    xui_doc_operation_t operation = {0};
    uint64_t bytes, insertion, separator_start = split, separator_bytes = 0;
    char ending;
    int result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    if (start >= split || split >= end || end > doc_seq_size(t->draft->source) ||
        !doc_seq_boundary(t->draft->source, start) ||
        !doc_seq_boundary(t->draft->source, split) ||
        !doc_seq_boundary(t->draft->source, end)) return XUI_ERROR_NOT_FOUND;
    bytes = split - start; insertion = end - bytes;
    result = doc_seq_read(t->draft->source, end - 1, &ending, 1);
    if (result != XUI_OK) return result;
    if (ending != '\r' && ending != '\n') {
        /* An unterminated final block needs a separator before its new
         * following sibling. Reuse the trailing line ending and blank-line
         * suffix before the parser-owned split, including quote prefixes.
         * Definitions elsewhere in the gap move unchanged with their source
         * slice; none of their bytes enter this duplicated separator. */
        separator_start = split - 1;
        result = doc_seq_read(t->draft->source, separator_start, &ending, 1);
        if (result != XUI_OK) return result;
        if (ending != '\r' && ending != '\n') return XUI_ERROR_NOT_FOUND;
        if (ending == '\n' && separator_start) {
            result = doc_seq_read(t->draft->source, separator_start - 1, &ending, 1);
            if (result != XUI_OK) return result;
            if (ending == '\r') separator_start--;
        }
        /* Container bounds can already include their trailing blank lines
         * (a loose list item, for example). Include those complete physical
         * lines and the preceding line ending in the new separator too. Do
         * not duplicate content-line spaces, which may be a hard-break mark. */
        while (separator_start > start) {
            uint64_t line = separator_start;
            int blank = 1;
            while (line > start) {
                result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
                result = doc_seq_read(t->draft->source, line - 1, &ending, 1);
                if (result != XUI_OK) return result;
                if (ending == '\r' || ending == '\n') break;
                if (ending != ' ' && ending != '\t' && ending != '>') { blank = 0; break; }
                line--;
            }
            if (!blank || line == start) break;
            separator_start = line - 1;
            if (ending == '\n' && separator_start) {
                result = doc_seq_read(t->draft->source, separator_start - 1, &ending, 1);
                if (result != XUI_OK) return result;
                if (ending == '\r') separator_start--;
            }
        }
        separator_bytes = split - separator_start;
        if (separator_bytes > t->draft->allocator->max_bytes ||
            doc_seq_size(t->draft->source) > t->draft->allocator->max_bytes - separator_bytes)
            return XUI_DOC_ERROR_LIMIT;
    }
    result = doc_seq_replace(t->draft->allocator, t->draft->source,
        split, doc_seq_size(t->draft->source), NULL, &prefix);
    if (result == XUI_OK) result = doc_seq_replace(t->draft->allocator,
        prefix, 0, start, NULL, &left);
    if (result == XUI_OK && separator_bytes) {
        result = doc_seq_replace(t->draft->allocator, t->draft->source,
            split, doc_seq_size(t->draft->source), NULL, &separator_prefix);
        if (result == XUI_OK) result = doc_seq_replace(t->draft->allocator,
            separator_prefix, 0, separator_start, NULL, &separator);
        if (result == XUI_OK) result = doc_seq_replace(t->draft->allocator,
            left, 0, 0, separator, &payload);
    } else if (result == XUI_OK) {
        payload = left; doc_seq_retain(payload);
    }
    if (result == XUI_OK) result = doc_seq_replace(t->draft->allocator,
        t->draft->source, start, split, NULL, &without);
    if (result == XUI_OK) result = doc_seq_replace(t->draft->allocator,
        without, insertion, insertion, payload, &rotated);
    if (result == XUI_OK) {
        before = t->draft->source; doc_seq_retain(before);
        doc_seq_release(t->draft->source); t->draft->source = rotated; rotated = NULL;
        /* Both rotation and trivia duplication share source payload blobs.
         * Trivia contains no open brackets, so both counters stay unchanged. */
        operation.iKind = XUI_DOC_OP_SOURCE;
        operation.iFlags = XUI_DOC_CHANGE_SOURCE | XUI_DOC_CHANGE_TEXT | XUI_DOC_CHANGE_STRUCTURE;
        operation.iOffset = start; operation.iOldLength = bytes;
        result = doc_txn_op(t, &operation);
        if (result == XUI_OK) {
            operation.iOffset = insertion; operation.iOldLength = 0;
            operation.iNewLength = bytes + separator_bytes;
            result = doc_txn_op(t, &operation);
        }
        if (result == XUI_OK) {
            t->source_before_patch = before;
            result = doc_markdown_parse(t);
            t->source_before_patch = NULL;
        }
        doc_seq_release(before);
    }
    doc_seq_release(prefix); doc_seq_release(left);
    doc_seq_release(separator_prefix); doc_seq_release(separator); doc_seq_release(payload);
    doc_seq_release(without); doc_seq_release(rotated);
    return result;
}
/* A list/task/footnote opener belongs to its container, not to the paragraph
 * occupying its first slot. Transfer only those parser-confirmed prefixes;
 * the paragraph payload and every intervening source slice remain shared. */
/* Anchor at a parser-confirmed byte/column pair. Physical file columns
 * cannot infer the parser's columns after footnote dedentation. An explicit
 * quote/list marker or the outermost recorded continuation supplies an exact
 * anchor, including a Tab shared with inner containers. */
static int doc_md_move_indent_boundary(xui_document_transaction t,
    uint64_t start, uint64_t end, uint32_t logical_start,
    uint32_t logical_target, uint32_t logical_end, doc_md_indent_boundary* boundary)
{
    char* raw; uint64_t column = logical_start, at; int result;
    memset(boundary, 0, sizeof(*boundary));
    if (start > end || logical_start > logical_target || logical_target > logical_end || end - start >= SIZE_MAX)
        return XUI_ERROR_NOT_FOUND;
    raw = doc_alloc(t->draft->allocator, (size_t)(end - start) + 1);
    if (!raw) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_read(t->draft->source, start, raw, end - start);
    for (at = start; at < end && result == XUI_OK; at++) {
        char c = raw[at - start];
        if (!(at & 16383)) result = doc_txn_check(t, 0);
        column = c == '\t' ? (column + 4) & ~(uint64_t)3 : column + 1;
    }
    if (result == XUI_OK && column != logical_end) result = XUI_ERROR_NOT_FOUND;
    column = logical_start;
    for (at = start; at < end && result == XUI_OK && column < logical_target; at++) {
        char c = raw[at - start]; uint64_t width = c == '\t' ? 4 - column % 4 : 1;
        if (!(at & 16383)) result = doc_txn_check(t, 0);
        if (width > logical_target - column) {
            boundary->tab_left = (unsigned)(logical_target - column);
            boundary->tab_right = (unsigned)(width - boundary->tab_left);
            column = logical_target; break;
        }
        column += width;
    }
    if (result == XUI_OK && column != logical_target) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) { boundary->byte = at; boundary->column = column; }
    doc_free(raw); return result;
}
static int doc_md_move_code_boundary(xui_document_transaction t, doc_node* code,
    const xui_doc_code_indent_t* indent, uint64_t line_start, uint64_t line_end,
    uint32_t target, doc_md_indent_boundary* boundary)
{
    uint64_t anchor = indent->iSourceStart; uint32_t column = indent->iIndentStartColumn;
    doc_node* owner = doc_index_get(t->draft->index, code->parent); unsigned depth = 0;
    for (; owner && owner->id != DOC_ROOT; owner = doc_index_get(t->draft->index, owner->parent)) {
        doc_node_source_range range; int result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        if (++depth >= DOC_MAX_DEPTH) return XUI_DOC_ERROR_LIMIT;
        doc_node_source_range_get(t->draft, owner, &range);
        if (owner->kind == XUI_DOC_QUOTE && owner->syntax_aux) {
            uint64_t low = 0, high = owner->syntax_aux->size / sizeof(doc_quote_indent_relative);
            while (low < high) {
                uint64_t middle = low + (high - low) / 2; doc_quote_indent_relative r;
                memcpy(&r, owner->syntax_aux->data + middle * sizeof(r), sizeof(r));
                if (range.syntax_start + r.marker < line_start) low = middle + 1; else high = middle;
            }
            if (low < owner->syntax_aux->size / sizeof(doc_quote_indent_relative)) {
                doc_quote_indent_relative r; uint64_t at;
                memcpy(&r, owner->syntax_aux->data + low * sizeof(r), sizeof(r)); at = range.syntax_start + r.marker;
                if (at < line_end && (at < anchor || (at == anchor && r.marker_column < column))) {
                    anchor = at; column = r.marker_column;
                }
            }
        } else if (owner->kind == XUI_DOC_LIST_ITEM) {
            if (owner->syntax_aux && owner->syntax_aux->size == sizeof(doc_list_first_indent_relative)) {
                doc_list_first_indent_relative r; uint64_t at;
                memcpy(&r, owner->syntax_aux->data, sizeof(r)); at = range.syntax_start + r.start;
                if (at >= line_start && at < line_end && (at < anchor || (at == anchor && r.start_column < column))) {
                    anchor = at; column = r.start_column;
                }
            }
            if (owner->list_indents) {
                uint64_t low = 0, high = owner->list_indents->size / sizeof(doc_list_indent_relative);
                while (low < high) {
                    uint64_t middle = low + (high - low) / 2; doc_list_indent_relative r;
                    memcpy(&r, owner->list_indents->data + middle * sizeof(r), sizeof(r));
                    if (range.syntax_start + r.start < line_start) low = middle + 1; else high = middle;
                }
                if (low < owner->list_indents->size / sizeof(doc_list_indent_relative)) {
                    doc_list_indent_relative r; uint64_t at;
                    memcpy(&r, owner->list_indents->data + low * sizeof(r), sizeof(r)); at = range.syntax_start + r.start;
                    if (at < line_end && (at < anchor || (at == anchor && r.start_column < column))) {
                        anchor = at; column = r.start_column;
                    }
                }
            }
        }
        /* Footnote body coordinates restart here. Outer markers belong to a
         * different parser input and must not be mixed into this line. */
        if (owner->kind == XUI_DOC_FOOTNOTE) break;
    }
    return doc_md_move_indent_boundary(t, anchor, indent->iSourceEnd, column,
        target, indent->iIndentEndColumn, boundary);
}
static int doc_md_move_code_first_boundary(xui_document_transaction t,
    doc_node* code, doc_md_indent_boundary* boundary)
{
    struct xui_doc_snapshot_t snapshot = {0}; xui_doc_code_indent_t indent = {0};
    doc_node_source_range range; int result;
    memset(boundary, 0, sizeof(*boundary));
    snapshot.state = t->draft; snapshot.identity = t->document->identity;
    snapshot.revision = t->base_revision; indent.iSize = sizeof(indent);
    result = xuiDocumentSnapshotGetCodeIndent(&snapshot, code->id, 0, &indent);
    if (result != XUI_OK || indent.iFlags) return result != XUI_OK ? result : XUI_ERROR_NOT_FOUND;
    doc_node_source_range_get(t->draft, code, &range);
    return doc_md_move_code_boundary(t, code, &indent, range.syntax_start, indent.iSourceEnd,
        indent.iIndentStartColumn, boundary);
}
static int doc_md_move_block_prefix(xui_document_transaction t, doc_node* paragraph,
    doc_md_output* first, doc_md_output* continuation)
{
    doc_node_source_range range;
    char* raw = NULL;
    uint64_t bytes;
    int result;
    if (!paragraph) return XUI_ERROR_NOT_FOUND;
    doc_node_source_range_get(t->draft, paragraph, &range);
    if (paragraph->kind == XUI_DOC_CODE_BLOCK) {
        struct xui_doc_snapshot_t snapshot = {0}; xui_doc_block_syntax_t syntax = {0};
        snapshot.state = t->draft; snapshot.identity = t->document->identity;
        snapshot.revision = t->base_revision; syntax.iSize = sizeof(syntax);
        result = xuiDocumentSnapshotGetBlockSyntax(&snapshot, paragraph->id, &syntax);
        if (result != XUI_OK) return result;
        if (syntax.iKind == XUI_DOC_BLOCK_SYNTAX_FENCED_CODE) range.source_start = syntax.iPrimaryStart;
        else if (syntax.iKind == XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE) {
            doc_md_indent_boundary boundary;
            result = doc_md_move_code_first_boundary(t, paragraph, &boundary);
            if (result != XUI_OK) return result;
            /* Preserve the ancestor's physical Tab in its opener. The body
             * receives only the code columns sharing that Tab. */
            range.source_start = boundary.byte + (boundary.tab_right != 0);
        }
    }
    if (range.syntax_start == DOC_NONE || range.source_start == DOC_NONE ||
        range.source_start < range.syntax_start) return XUI_ERROR_NOT_FOUND;
    bytes = range.source_start - range.syntax_start;
    if (bytes >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    raw = doc_alloc(t->draft->allocator, (size_t)bytes + 1);
    if (!raw) return XUI_ERROR_OUT_OF_MEMORY;
    first->allocator = continuation->allocator = t->draft->allocator;
    result = doc_seq_read(t->draft->source, range.syntax_start, raw, bytes);
    if (result == XUI_OK) result = doc_md_paragraph_prefixes(t, paragraph, &range,
        raw, bytes, first, continuation, DOC_NONE, NULL);
    doc_free(raw);
    return result;
}
static int doc_md_move_container_opener(xui_document_transaction old,
    xui_document_transaction candidate, doc_node* parent, uint64_t promoted,
    uint64_t* split, uint64_t* end)
{
    doc_node *first_node, *next_node;
    doc_node_source_range first_range, next_range;
    doc_md_output first = {0}, continuation = {0}, next = {0}, unused = {0};
    uint64_t a, b;
    int result;
    first_node = doc_index_get(old->draft->index, doc_seq_get_id(parent->children, 0));
    next_node = doc_index_get(old->draft->index, doc_seq_get_id(parent->children, promoted));
    result = doc_md_move_block_prefix(old, first_node, &first, &continuation);
    if (result == XUI_OK) result = doc_md_move_block_prefix(old, next_node, &next, &unused);
    if (result != XUI_OK) goto done;
    doc_node_source_range_get(old->draft, first_node, &first_range);
    doc_node_source_range_get(old->draft, next_node, &next_range);
    if (!first.size || first_range.syntax_start >= next_range.syntax_start ||
        first_range.syntax_start + first.size > next_range.syntax_start) {
        result = XUI_ERROR_NOT_FOUND; goto done;
    }
    result = doc_txn_source_patch(candidate, next_range.syntax_start,
        next_range.syntax_start + next.size, first.data, first.size, 0);
    /* Reuse the actual continuation slot. Turning a bare quote's following
     * list marker into whitespace would let the quote consume one of those
     * spaces and shorten the list indentation. The existing paragraph's
     * parser-confirmed prefix already accounts for that boundary. */
    if (result == XUI_OK) result = doc_txn_source_patch(candidate, first_range.syntax_start,
        first_range.syntax_start + first.size, next.data, next.size, 0);
    if (result == XUI_OK) {
        a = first.size; b = next.size;
        if (*split > next_range.syntax_start) { a += next.size; b += first.size; }
        if (*split < a || *end < first.size + next.size) result = XUI_ERROR_NOT_FOUND;
        else {
            *split = *split - a + b;
            /* Header and continuation exchange places; total size is fixed. */
        }
    }
done:
    doc_free(first.data); doc_free(continuation.data); doc_free(next.data); doc_free(unused.data);
    return result;
}
static int doc_md_move_slice(xui_document_transaction t, doc_sequence* source,
    uint64_t start, uint64_t end, doc_sequence** out)
{
    doc_sequence* prefix = NULL;
    int result;
    *out = NULL;
    if (start > end || end > doc_seq_size(source)) return XUI_ERROR_NOT_FOUND;
    result = doc_seq_replace(t->draft->allocator, source, end, doc_seq_size(source), NULL, &prefix);
    if (result == XUI_OK) result = doc_seq_replace(t->draft->allocator, prefix, 0, start, NULL, out);
    doc_seq_release(prefix); return result;
}
static int doc_md_move_append(xui_document_transaction t, doc_sequence** output,
    doc_sequence* part)
{
    doc_sequence* next = NULL;
    uint64_t end = doc_seq_size(*output);
    int result = doc_seq_replace(t->draft->allocator, *output, end, end, part, &next);
    if (result == XUI_OK) { doc_seq_release(*output); *output = next; }
    return result;
}
/* A physical line's prefix is proved by the old container metadata. In
 * particular, do not consume an inline delimiter or a child block's indent;
 * split a shared Tab only at the parser's logical content column. */
static int doc_md_move_line_prefix(xui_document_transaction t, doc_node* paragraph,
    uint64_t start, uint64_t end, uint64_t* bytes, unsigned* padding)
{
    doc_node* owner = doc_index_get(t->draft->index, paragraph->parent);
    unsigned depth = 0;
    *bytes = 0; *padding = 0;
    for (; owner && owner->id != DOC_ROOT; owner = doc_index_get(t->draft->index, owner->parent)) {
        doc_node_source_range range;
        uint64_t low = 0, high;
        int result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        if (++depth > DOC_MAX_DEPTH) return XUI_ERROR_NOT_FOUND;
        doc_node_source_range_get(t->draft, owner, &range);
        if (range.syntax_start == DOC_NONE) return XUI_ERROR_NOT_FOUND;
        if (owner->kind == XUI_DOC_FOOTNOTE && owner->syntax_aux) {
            high = owner->syntax_aux->size / sizeof(doc_footnote_indent_relative);
            while (low < high) {
                uint64_t middle = low + (high - low) / 2;
                doc_footnote_indent_relative r;
                memcpy(&r, owner->syntax_aux->data + middle * sizeof(r), sizeof(r));
                if (range.syntax_start + r.start < start) low = middle + 1; else high = middle;
            }
            if (low < owner->syntax_aux->size / sizeof(doc_footnote_indent_relative)) {
                doc_footnote_indent_relative r;
                memcpy(&r, owner->syntax_aux->data + low * sizeof(r), sizeof(r));
                if (range.syntax_start + r.start == start) {
                    uint64_t cut = range.syntax_start + r.end;
                    if (cut < start || cut > end) return XUI_ERROR_NOT_FOUND;
                    if (cut - start > *bytes) { *bytes = cut - start; *padding = 0; }
                }
            }
        } else if (owner->kind == XUI_DOC_QUOTE && owner->syntax_aux) {
            high = owner->syntax_aux->size / sizeof(doc_quote_indent_relative);
            while (low < high) {
                uint64_t middle = low + (high - low) / 2;
                doc_quote_indent_relative r;
                memcpy(&r, owner->syntax_aux->data + middle * sizeof(r), sizeof(r));
                if (range.syntax_start + r.marker < start) low = middle + 1; else high = middle;
            }
            if (low < owner->syntax_aux->size / sizeof(doc_quote_indent_relative)) {
                doc_quote_indent_relative r;
                memcpy(&r, owner->syntax_aux->data + low * sizeof(r), sizeof(r));
                if (range.syntax_start + r.marker < end) {
                    uint64_t cut; doc_md_indent_boundary boundary;
                    result = doc_md_move_indent_boundary(t, range.syntax_start + r.marker + 1,
                        range.syntax_start + r.end, r.marker_column + 1, r.content_column, r.end_column, &boundary);
                    if (result != XUI_OK) return result;
                    cut = boundary.byte + (boundary.tab_right != 0);
                    if (cut < start || cut > end) return XUI_ERROR_NOT_FOUND;
                    if (cut - start > *bytes || (cut - start == *bytes && boundary.tab_right < *padding)) {
                        *bytes = cut - start; *padding = boundary.tab_right;
                    }
                }
            }
        } else if (owner->kind == XUI_DOC_LIST_ITEM && owner->list_indents) {
            high = owner->list_indents->size / sizeof(doc_list_indent_relative);
            while (low < high) {
                uint64_t middle = low + (high - low) / 2;
                doc_list_indent_relative r;
                memcpy(&r, owner->list_indents->data + middle * sizeof(r), sizeof(r));
                if (range.syntax_start + r.start < start) low = middle + 1; else high = middle;
            }
            if (low < owner->list_indents->size / sizeof(doc_list_indent_relative)) {
                doc_list_indent_relative r;
                doc_md_indent_boundary boundary;
                memcpy(&r, owner->list_indents->data + low * sizeof(r), sizeof(r));
                if (range.syntax_start + r.start < end) {
                    uint64_t first = range.syntax_start + r.start, last = range.syntax_start + r.end;
                    char* raw;
                    if (last > end || last < start) return XUI_ERROR_NOT_FOUND;
                    if (last - start >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
                    raw = doc_alloc(t->draft->allocator, (size_t)(last - start) + 1);
                    if (!raw) return XUI_ERROR_OUT_OF_MEMORY;
                    result = doc_seq_read(t->draft->source, start, raw, last - start);
                    if (result == XUI_OK)
                        result = doc_md_list_indent_content_boundary(t, &r, first, last, raw, start, &boundary);
                    doc_free(raw);
                    if (result != XUI_OK) return result;
                    last = boundary.byte + (boundary.tab_right != 0);
                    if (last - start > *bytes || (last - start == *bytes && boundary.tab_right < *padding)) {
                        *bytes = last - start; *padding = boundary.tab_right;
                    }
                }
            }
        }
    }
    for (uint64_t at = 0; at < *bytes;) {
        char raw[256]; uint64_t count = *bytes - at < sizeof(raw) ? *bytes - at : sizeof(raw);
        int result = doc_txn_check(t, 0);
        if (result == XUI_OK) result = doc_seq_read(t->draft->source, start + at, raw, count);
        if (result != XUI_OK) return result;
        for (uint64_t i = 0; i < count; i++)
            if (raw[i] != ' ' && raw[i] != '\t' && raw[i] != '>') return XUI_ERROR_NOT_FOUND;
        at += count;
    }
    return XUI_OK;
}
/* Prefixes keep their original markers. For an indented code payload,
 * determine how many code columns the new container prefix already supplies.
 * A quote consumes one optional space even when its old empty marker had none;
 * a first list code line uses the parser's one-space marker-gap rule. */
static int doc_md_move_code_target(xui_document_transaction t, doc_node* parent,
    doc_node* reference, int first, const doc_md_output* prefix, uint32_t* target, int* extra)
{
    uint64_t at = 0, column = 0; doc_node_source_range range;
    *target = 0; *extra = 0;
    if (parent->kind == XUI_DOC_ROOT || parent->kind == XUI_DOC_FOOTNOTE) return XUI_OK;
    doc_node_source_range_get(t->draft, parent, &range);
    if (parent->kind == XUI_DOC_QUOTE && parent->syntax_aux) {
        doc_quote_indent_relative r; uint64_t marker = DOC_NONE;
        memcpy(&r, parent->syntax_aux->data, sizeof(r));
        for (uint64_t i = 0; i < prefix->size; i++) if (prefix->data[i] == '>') marker = i;
        if (marker == DOC_NONE || r.marker_column > UINT32_MAX - 2) return XUI_ERROR_NOT_FOUND;
        at = marker; column = r.marker_column; *target = r.marker_column + 2;
    } else if (parent->kind == XUI_DOC_LIST_ITEM && parent->syntax_aux &&
        parent->syntax_aux->size == sizeof(doc_list_first_indent_relative)) {
        doc_list_first_indent_relative r; memcpy(&r, parent->syntax_aux->data, sizeof(r));
        if (first) {
            doc_node_source_range child; uint64_t marker = range.syntax_start + r.start;
            doc_node_source_range_get(t->draft, reference ? reference : parent, &child);
            if (marker < child.syntax_start || marker - child.syntax_start >= prefix->size ||
                parent->marker_primary_end < parent->marker_primary_start) return XUI_ERROR_NOT_FOUND;
            at = marker - child.syntax_start; column = r.start_column;
            if (column + parent->marker_primary_end - parent->marker_primary_start + 1 > UINT32_MAX)
                return XUI_DOC_ERROR_LIMIT;
            *target = (uint32_t)(column + parent->marker_primary_end - parent->marker_primary_start + 1);
        } else {
            doc_node* owner = doc_index_get(t->draft->index, parent->parent); unsigned depth = 0;
            *target = r.content_column;
            for (; owner && owner->id != DOC_ROOT; owner = doc_index_get(t->draft->index, owner->parent)) {
                if (++depth >= DOC_MAX_DEPTH) return XUI_DOC_ERROR_LIMIT;
                if (owner->kind == XUI_DOC_FOOTNOTE) {
                    /* Paragraph continuation prefixes use the canonical four
                     * dedentation columns before the footnote body. */
                    while (at < prefix->size && column < 4) {
                        char c = prefix->data[at++];
                        if (c != ' ' && c != '\t') return XUI_ERROR_NOT_FOUND;
                        column = c == '\t' ? (column + 4) & ~(uint64_t)3 : column + 1;
                    }
                    if (column != 4) return XUI_ERROR_NOT_FOUND;
                    column = 0; break;
                }
            }
        }
    } else return XUI_ERROR_NOT_FOUND;
    for (; at < prefix->size; at++) {
        char c = prefix->data[at];
        if (!(at & 16383)) { int result = doc_txn_check(t, 0); if (result != XUI_OK) return result; }
        column = c == '\t' ? (column + 4) & ~(uint64_t)3 : column + 1;
    }
    if (column > (uint64_t)*target + 4 || column + 1 < *target) return XUI_ERROR_NOT_FOUND;
    *extra = (int)((int64_t)column - *target); return XUI_OK;
}
/* Removing a container can change the stop of a Tab in the code's four
 * indent columns. Rebase only that confirmed indent; the literal source
 * after its content boundary remains a shared slice, including excess indent.
 * Tabs whose logical phase stays equal keep their original source spelling. */
static int doc_md_move_code_rebase(xui_document_transaction t, doc_node* code,
    doc_node* parent, doc_node* reference, int first, const doc_md_output* replacement,
    uint64_t start, uint64_t end, uint64_t* bytes, unsigned* padding)
{
    doc_node_source_range range; uint64_t low = 0, high; xui_doc_code_indent_t indent = {0};
    struct xui_doc_snapshot_t snapshot = {0}; doc_md_indent_boundary content = {0};
    uint32_t target; int extra, result, has_tab = 0;
    if (!code->syntax_aux) return XUI_ERROR_NOT_FOUND;
    doc_node_source_range_get(t->draft, code, &range);
    high = code->syntax_aux->size / sizeof(doc_code_indent_relative);
    while (low < high) {
        uint64_t middle = low + (high - low) / 2; doc_code_indent_relative r;
        memcpy(&r, code->syntax_aux->data + middle * sizeof(r), sizeof(r));
        if (range.syntax_start + r.start < start) low = middle + 1; else high = middle;
    }
    if (low >= code->syntax_aux->size / sizeof(doc_code_indent_relative)) return XUI_ERROR_NOT_FOUND;
    snapshot.state = t->draft; snapshot.identity = t->document->identity;
    snapshot.revision = t->base_revision; indent.iSize = sizeof(indent);
    result = xuiDocumentSnapshotGetCodeIndent(&snapshot, code->id, low, &indent);
    if (result != XUI_OK) return result;
    if (indent.iFlags || indent.iSourceStart >= end || indent.iSourceEnd > end) return XUI_ERROR_NOT_FOUND;
    result = doc_md_move_code_target(t, parent, reference, first, replacement, &target, &extra);
    if (result != XUI_OK) return result;
    for (uint64_t at = indent.iSourceStart; at < indent.iSourceEnd && !has_tab;) {
        char raw[256]; uint64_t count = indent.iSourceEnd - at < sizeof(raw) ? indent.iSourceEnd - at : sizeof(raw);
        result = doc_txn_check(t, 0);
        if (result == XUI_OK) result = doc_seq_read(t->draft->source, at, raw, count);
        if (result != XUI_OK) return result;
        for (uint64_t i = 0; i < count; i++) if (raw[i] == '\t') has_tab = 1;
        at += count;
    }
    if (!extra && (!has_tab || indent.iIndentStartColumn % 4 == target % 4)) return XUI_OK;
    result = doc_md_move_code_boundary(t, code, &indent, start, end, indent.iContentColumn, &content);
    if (result != XUI_OK) return result;
    if (content.byte < start || content.byte + (content.tab_right != 0) > end ||
        content.tab_right > 3 || extra < -1 || extra > 4) return XUI_ERROR_NOT_FOUND;
    *bytes = content.byte + (content.tab_right != 0) - start;
    *padding = (unsigned)(4 - extra) + content.tab_right;
    return XUI_OK;
}
/* Scan source chunks with cancellation points, without materializing a long
 * paragraph or performing one persistent-rope lookup for every payload byte. */
static int doc_md_move_line_scan(xui_document_transaction t, uint64_t start,
    uint64_t end, uint64_t* content_end, uint64_t* line_end, uint64_t* last_nonspace)
{
    *last_nonspace = start;
    for (uint64_t at = start; at < end;) {
        char raw[4096]; uint64_t count = end - at < sizeof(raw) ? end - at : sizeof(raw);
        int result = doc_txn_check(t, 0);
        if (result == XUI_OK) result = doc_seq_read(t->draft->source, at, raw, count);
        if (result != XUI_OK) return result;
        for (uint64_t i = 0; i < count; i++) {
            if (raw[i] == '\r' || raw[i] == '\n') {
                *content_end = at + i; *line_end = *content_end + 1;
                if (raw[i] == '\r' && *line_end < end &&
                    doc_seq_equal_bytes(t->draft->source, *line_end, "\n", 1)) (*line_end)++;
                return XUI_OK;
            }
            if (raw[i] != ' ' && raw[i] != '\t') *last_nonspace = at + i + 1;
        }
        at += count;
    }
    *content_end = *line_end = end; return XUI_OK;
}
/* Move intent can change edited lists' tightness and leave a task item
 * without its paragraph. A task checkbox requires that canonical inline
 * host even when empty. Derive only these edited-container properties from
 * the confirmed parse in a private shadow, then prove the whole tree. */
static int doc_md_move_derive_lists(xui_document_transaction t,
    xui_document_transaction shadow, doc_state* parsed, uint64_t old_parent, uint64_t new_parent)
{
    struct xui_doc_transaction_t planned = {0};
    uint64_t parents[2] = {old_parent, new_parent};
    int result = doc_markdown_shadow_begin(shadow, &planned);
    if (result != XUI_OK) return result;
    {
        doc_node* desired = doc_index_get(planned.draft->index, old_parent);
        if (desired && desired->kind == XUI_DOC_LIST_ITEM && (desired->attrs->iFlags & XUI_DOC_TASK) &&
            !doc_seq_size(desired->children)) {
            doc_node* actual = doc_index_get(parsed->index,
                doc_markdown_resolve_node(planned.draft, parsed, old_parent));
            doc_node* paragraph = actual && doc_seq_size(actual->children) == 1 ?
                doc_index_get(parsed->index, doc_seq_get_id(actual->children, 0)) : NULL;
            xui_doc_node_desc_t desc = {0}; uint64_t inserted;
            if (!actual || actual->kind != XUI_DOC_LIST_ITEM ||
                !doc_attributes_equal(desired->attrs, actual->attrs) ||
                !doc_semantic_empty_paragraph(parsed, paragraph)) result = XUI_ERROR_NOT_FOUND;
            else {
                desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_PARAGRAPH; desc.tAttributes = *paragraph->attrs;
                result = doc_txn_insert(&planned, old_parent, 0, &desc, &inserted);
            }
        }
    }
    for (unsigned p = 0; p < 2 && result == XUI_OK; p++) {
        doc_node* ancestor = doc_index_get(t->draft->index, parents[p]);
        unsigned depth = 0;
        for (; ancestor && ancestor->id != DOC_ROOT; ancestor = doc_index_get(t->draft->index, ancestor->parent)) {
            doc_node *desired, *actual; xui_doc_attributes_t attrs;
            result = doc_txn_check(t, 0); if (result != XUI_OK) break;
            if (++depth >= DOC_MAX_DEPTH) { result = XUI_DOC_ERROR_LIMIT; break; }
            if (ancestor->kind != XUI_DOC_LIST) continue;
            desired = doc_index_get(planned.draft->index, ancestor->id);
            actual = doc_index_get(parsed->index, doc_markdown_resolve_node(planned.draft, parsed, ancestor->id));
            if (!desired || !actual || desired->kind != XUI_DOC_LIST || actual->kind != XUI_DOC_LIST) {
                result = XUI_ERROR_NOT_FOUND; break;
            }
            if (!((desired->attrs->iFlags ^ actual->attrs->iFlags) & XUI_DOC_TIGHT)) continue;
            attrs = *desired->attrs;
            attrs.iFlags = (attrs.iFlags & ~XUI_DOC_TIGHT) | (actual->attrs->iFlags & XUI_DOC_TIGHT);
            result = xuiDocumentTxnSetAttributes(&planned, desired->id, &attrs);
            if (result != XUI_OK) break;
        }
    }
    if (result == XUI_OK && !doc_semantic_markdown_projection_equal(planned.draft, parsed)) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (uint64_t i = 0; i < planned.count && result == XUI_OK; i++) result = doc_txn_op(shadow, planned.ops + i);
        if (result == XUI_OK && planned.count) {
            doc_state_release(shadow->draft); shadow->draft = planned.draft; planned.draft = NULL;
        }
    }
    doc_state_release(planned.draft); doc_free(planned.ops); return result;
}
/* Syntax ranges may end before the physical separator that follows a
 * container. Move those plain blank lines with its tail block, or insert
 * after them. Stop before every nonblank line, including hidden definitions;
 * semantic display order cannot determine these physical source gaps. */
static int doc_md_move_plain_blank_tail(xui_document_transaction t, uint64_t* end)
{
    uint64_t limit = doc_seq_size(t->draft->source), at = *end;
    while (at < limit) {
        uint64_t content, line, nonspace;
        int result = doc_md_move_line_scan(t, at, limit, &content, &line, &nonspace);
        if (result != XUI_OK) return result;
        if (nonspace > at) break;
        at = line;
    }
    *end = at; return XUI_OK;
}
static int doc_md_move_footnote_scope(xui_document_transaction t, doc_node* parent,
    uint64_t* start, uint64_t* end)
{
    unsigned depth = 0;
    for (; parent && parent->id != DOC_ROOT; parent = doc_index_get(t->draft->index, parent->parent)) {
        int result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        if (++depth >= DOC_MAX_DEPTH) return XUI_DOC_ERROR_LIMIT;
        if (parent->kind == XUI_DOC_FOOTNOTE) {
            doc_node_source_range r; doc_node_source_range_get(t->draft, parent, &r);
            if (r.syntax_start == DOC_NONE || r.syntax_end == DOC_NONE) return XUI_ERROR_NOT_FOUND;
            *start = r.syntax_start; *end = r.syntax_end; break;
        }
    }
    return XUI_OK;
}
/* An empty container has no child whose prefix can be sampled. Its own
 * parser-owned opener supplies the first/continuation prefixes. Replace the
 * empty physical slot after inserting the shared moved payload; never emit a
 * second container or serialize the surrounding source. Full semantic and
 * reference spelling checks still decide whether the candidate can publish. */
static int doc_md_move_empty_container_prefix(xui_document_transaction t,
    doc_node* parent, uint64_t* start, uint64_t* end,
    doc_md_output* first, doc_md_output* continuation)
{
    struct xui_doc_snapshot_t snapshot = {0};
    xui_doc_block_syntax_t syntax = {0};
    xui_doc_source_line_t line = {0};
    doc_node sample = {0}; doc_node_source_range range;
    char* raw = NULL; uint64_t marker = DOC_NONE, bytes;
    int result;
    if (!parent || doc_seq_size(parent->children)) return XUI_ERROR_NOT_FOUND;
    result = doc_md_bounds(t->draft, parent->id, start, end);
    if (result != XUI_OK || *start >= *end) return XUI_ERROR_NOT_FOUND;
    snapshot.state = t->draft; snapshot.identity = t->document->identity;
    snapshot.revision = t->base_revision;
    if (parent->kind == XUI_DOC_FOOTNOTE) {
        for (uint64_t at = 0; at < doc_seq_size(t->draft->references); at += sizeof(xui_doc_reference_definition_t)) {
            xui_doc_reference_definition_t reference;
            result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
            result = doc_seq_read(t->draft->references, at, &reference, sizeof(reference));
            if (result != XUI_OK) return result;
            if (reference.iKind == XUI_DOC_REFERENCE_FOOTNOTE &&
                reference.iSourceStart == *start && reference.iSourceEnd == *end && reference.iLabelStart >= 2) {
                marker = reference.iLabelStart - 2; break;
            }
        }
    } else {
        syntax.iSize = sizeof(syntax);
        result = xuiDocumentSnapshotGetBlockSyntax(&snapshot, parent->id, &syntax);
        if (result != XUI_OK) return result;
        marker = syntax.iPrimaryStart;
    }
    if (marker == DOC_NONE) return XUI_ERROR_NOT_FOUND;
    line.iSize = sizeof(line);
    result = xuiDocumentSnapshotGetSourceLine(&snapshot, marker, &line);
    if (result != XUI_OK) return result;
    range.syntax_start = line.iLineStart; range.syntax_end = line.iLineEnd;
    range.source_start = range.source_end = line.iContentEnd;
    if (!range.syntax_start && doc_seq_equal_bytes(t->draft->source, 0, "\xef\xbb\xbf", 3)) range.syntax_start = 3;
    if (range.syntax_start < *start || range.syntax_start > range.source_start || range.syntax_end > *end)
        return XUI_ERROR_NOT_FOUND;
    *start = range.syntax_start;
    bytes = range.source_start - range.syntax_start;
    if (bytes >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    raw = doc_alloc(t->draft->allocator, (size_t)bytes + 1);
    if (!raw) return XUI_ERROR_OUT_OF_MEMORY;
    first->allocator = continuation->allocator = t->draft->allocator;
    sample.parent = parent->id;
    result = doc_seq_read(t->draft->source, range.syntax_start, raw, bytes);
    if (result == XUI_OK) result = doc_md_paragraph_prefixes(t, &sample, &range,
        raw, bytes, first, continuation, DOC_NONE, NULL);
    if (result == XUI_OK && first->size != bytes) result = XUI_ERROR_NOT_FOUND;
    /* Empty list/definition syntax may end at the opener. Adding content
     * needs one separator; existing spaces and tabs remain verbatim. Task
     * checkbox spacing never changes the list's continuation column. */
    if (result == XUI_OK && first->size &&
        first->data[first->size - 1] != ' ' && first->data[first->size - 1] != '\t' &&
        (parent->kind == XUI_DOC_LIST_ITEM || parent->kind == XUI_DOC_FOOTNOTE)) {
        doc_md_write(first, " ", 1);
        if (parent->kind == XUI_DOC_LIST_ITEM && syntax.iSecondaryStart == DOC_NONE)
            doc_md_write(continuation, " ", 1);
        result = first->error ? first->error : continuation->error;
    }
    doc_free(raw);
    if (result == XUI_OK) result = doc_md_move_plain_blank_tail(t, end);
    return result;
}
/* A container opener belongs to the container even when its last block
 * moves away. Keep a small empty physical slot at the cut, sharing all moved
 * payload slices. Its two endings separate the retained marker from either
 * the next block or the moved block at an adjacent insertion boundary. */
static int doc_md_move_retained_empty_slot(xui_document_transaction t,
    uint64_t start, uint64_t end, doc_md_output* prefix, doc_sequence** slot,
    uint64_t* fresh, uint64_t* brackets)
{
    doc_md_output output = {0};
    uint64_t content, line, nonspace, ending_bytes = 1;
    char ending[2] = {'\n', 0};
    int result = doc_md_move_line_scan(t, start, end, &content, &line, &nonspace);
    if (result != XUI_OK) return result;
    if (line > content) {
        ending_bytes = line - content;
        if (ending_bytes > sizeof(ending)) return XUI_ERROR_NOT_FOUND;
        result = doc_seq_read(t->draft->source, content, ending, ending_bytes);
        if (result != XUI_OK) return result;
    }
    output.allocator = t->draft->allocator;
    doc_md_write(&output, prefix->data, prefix->size);
    doc_md_write(&output, ending, ending_bytes);
    doc_md_write(&output, ending, ending_bytes);
    result = output.error;
    if (result == XUI_OK) {
        *slot = doc_seq_text(t->draft->allocator, output.data, output.size);
        if (!*slot) result = XUI_ERROR_OUT_OF_MEMORY;
        else {
            *fresh += output.size;
            *brackets += doc_source_open_bracket_count(output.data, output.size);
        }
    }
    doc_free(output.data);
    return result;
}
static int doc_md_move_between_parents(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow, const xui_doc_operation_t* move)
{
    doc_node *paragraph = doc_index_get(t->draft->index, move->iNodeId),
        *old_parent = doc_index_get(t->draft->index, move->iParentId),
        *new_parent = doc_index_get(t->draft->index, move->iOtherNodeId), *reference;
    doc_sequence *payload = NULL, *slice = NULL, *prefix = NULL, *blank = NULL,
        *without = NULL, *source = NULL, *before = NULL, *opener = NULL, *retained_slot = NULL;
    struct xui_doc_transaction_t candidate = {0};
    doc_md_output first = {0}, continuation = {0}, empty = {0}, old_open = {0}, old_continue = {0},
        old_next = {0}, old_unused = {0}, new_next = {0}, new_unused = {0};
    struct { uint64_t start, end; doc_md_output* prefix; } patches[2];
    unsigned patch_count = 0;
    uint64_t removed_brackets = 0, added_brackets = 0;
    uint64_t editable_start = DOC_NONE, editable_end = DOC_NONE, other_start = DOC_NONE, other_end = DOC_NONE;
    int old_first, new_first, old_empty;
    uint64_t start, end, insertion, ignored, old_count, new_count, at, fresh = 0;
    char ending[2] = {'\n', 0}; uint64_t ending_bytes = 1;
    int result = XUI_ERROR_NOT_FOUND, last_blank = 0, terminated = 0;
    /* The semantic shadow has already checked the parent/child schema.
     * Reparent the complete block by replacing only its ancestor prefixes;
     * its own markers, literal bytes and descendants remain source payload. */
    if (!paragraph || !old_parent || !new_parent ||
        (old_parent->kind != XUI_DOC_ROOT && old_parent->kind != XUI_DOC_QUOTE &&
         old_parent->kind != XUI_DOC_LIST_ITEM && old_parent->kind != XUI_DOC_FOOTNOTE) ||
        (new_parent->kind != XUI_DOC_ROOT && new_parent->kind != XUI_DOC_QUOTE &&
         new_parent->kind != XUI_DOC_LIST_ITEM && new_parent->kind != XUI_DOC_FOOTNOTE)) return result;
    old_first = !move->iOffset && (old_parent->kind == XUI_DOC_LIST_ITEM || old_parent->kind == XUI_DOC_FOOTNOTE);
    new_first = !move->iNewLength && (new_parent->kind == XUI_DOC_LIST_ITEM || new_parent->kind == XUI_DOC_FOOTNOTE);
    old_count = doc_seq_size(old_parent->children); new_count = doc_seq_size(new_parent->children);
    if (move->iOffset >= old_count || move->iNewLength > new_count ||
        doc_seq_get_id(old_parent->children, move->iOffset) != paragraph->id) return result;
    old_empty = old_count == 1 && old_parent->id != DOC_ROOT;
    if (old_empty) old_first = 1;
    result = doc_md_move_footnote_scope(t, old_parent, &editable_start, &editable_end);
    if (result == XUI_OK) result = doc_md_move_footnote_scope(t, new_parent, &other_start, &other_end);
    if (result != XUI_OK) goto done;
    result = doc_md_bounds(t->draft, paragraph->id, &start, &end);
    if (result != XUI_OK) goto done;
    if (old_parent->id == DOC_ROOT) result = doc_md_move_plain_blank_tail(t, &end);
    else if (move->iOffset + 1 < old_count)
        result = doc_md_bounds(t->draft, doc_seq_get_id(old_parent->children, move->iOffset + 1), &end, &ignored);
    else {
        result = doc_md_bounds(t->draft, old_parent->id, &ignored, &end);
        if (result == XUI_OK) result = doc_md_move_plain_blank_tail(t, &end);
    }
    if (result != XUI_OK || start >= end) { result = XUI_ERROR_NOT_FOUND; goto done; }
    if (!start && doc_seq_equal_bytes(t->draft->source, 0, "\xef\xbb\xbf", 3)) start = 3;
    if (old_first) {
        doc_node* promoted = doc_index_get(t->draft->index, doc_seq_get_id(old_parent->children, 1));
        result = doc_md_move_block_prefix(t, paragraph, &old_open, &old_continue);
        if (result == XUI_OK && !old_empty) result = doc_md_move_block_prefix(t, promoted, &old_next, &old_unused);
        if (result != XUI_OK || !old_open.size || old_open.size > end - start) {
            if (result == XUI_OK) result = XUI_ERROR_NOT_FOUND;
            goto done;
        }
        if (!old_empty) {
            patches[patch_count].start = end; patches[patch_count].end = end + old_next.size;
            patches[patch_count++].prefix = &old_open;
        }
    }
    if (!new_count) {
        uint64_t empty_end;
        result = doc_md_move_empty_container_prefix(t, new_parent, &insertion, &empty_end, &first, &continuation);
        if (result == XUI_OK && (empty_end > start && insertion < end)) result = XUI_ERROR_NOT_FOUND;
        if (result != XUI_OK) goto done;
        new_first = 1;
        new_unused.allocator = t->draft->allocator;
        patches[patch_count].start = insertion; patches[patch_count].end = empty_end;
        patches[patch_count++].prefix = &new_unused;
    } else if (move->iNewLength < new_count)
        result = doc_md_bounds(t->draft, doc_seq_get_id(new_parent->children, move->iNewLength), &insertion, &ignored);
    else if (new_parent->id == DOC_ROOT) insertion = doc_seq_size(t->draft->source);
    else {
        result = doc_md_bounds(t->draft, new_parent->id, &ignored, &insertion);
        if (result == XUI_OK) result = doc_md_move_plain_blank_tail(t, &insertion);
    }
    if (result != XUI_OK || (insertion > start && insertion < end)) { result = XUI_ERROR_NOT_FOUND; goto done; }
    if (!insertion && doc_seq_equal_bytes(t->draft->source, 0, "\xef\xbb\xbf", 3)) insertion = 3;
    reference = new_count ? doc_index_get(t->draft->index, doc_seq_get_id(new_parent->children,
        move->iNewLength < new_count ? move->iNewLength : new_count - 1)) : NULL;
    if (!new_count) { /* Prefix and empty-slot removal were planned above. */ }
    else if (new_parent->id != DOC_ROOT) {
        result = doc_md_move_block_prefix(t, reference, &first, &continuation);
        if (result != XUI_OK) goto done;
        if (new_first) {
            /* The opener stays with its container. Use the actual next slot
             * when available, including a bare quote's optional-space rule. */
            if (new_count > 1) {
                doc_node* next = doc_index_get(t->draft->index, doc_seq_get_id(new_parent->children, 1));
                result = doc_md_move_block_prefix(t, next, &new_next, &new_unused);
                if (result != XUI_OK) goto done;
                doc_free(continuation.data); continuation = new_next; memset(&new_next, 0, sizeof(new_next));
            }
            patches[patch_count].start = insertion; patches[patch_count].end = insertion + first.size;
            patches[patch_count++].prefix = &continuation;
        } else {
            doc_free(first.data); first = continuation; memset(&continuation, 0, sizeof(continuation));
        }
    } else first.allocator = t->draft->allocator;
    if (!new_first) { continuation.allocator = t->draft->allocator; doc_md_write(&continuation, first.data, first.size); }
    if (continuation.error) { result = continuation.error; goto done; }
    empty.allocator = t->draft->allocator;
    if (insertion) {
        struct xui_doc_snapshot_t snapshot = {0}; xui_doc_source_line_t line = {0};
        snapshot.state = t->draft; snapshot.identity = t->document->identity; snapshot.revision = t->base_revision;
        line.iSize = sizeof(line);
        result = xuiDocumentSnapshotGetSourceLine(&snapshot, insertion - 1, &line);
        if (result != XUI_OK) goto done;
        if (line.iLineEnd == insertion && line.iContentEnd < line.iLineEnd) {
            char* raw; uint64_t bytes = line.iContentEnd - line.iLineStart;
            ending_bytes = line.iLineEnd - line.iContentEnd;
            if (ending_bytes > sizeof(ending) || bytes >= SIZE_MAX) { result = XUI_ERROR_NOT_FOUND; goto done; }
            result = doc_seq_read(t->draft->source, line.iContentEnd, ending, ending_bytes);
            if (result != XUI_OK) goto done;
            raw = doc_alloc(t->draft->allocator, (size_t)bytes + 1);
            if (!raw) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
            result = doc_seq_read(t->draft->source, line.iLineStart, raw, bytes);
            if (result == XUI_OK) {
                uint64_t i;
                for (i = 0; i < bytes && (raw[i] == ' ' || raw[i] == '\t' || raw[i] == '>'); i++) {}
                if (i == bytes) doc_md_write(&empty, raw, bytes);
            }
            doc_free(raw); if (result != XUI_OK || empty.error) { result = result ? result : empty.error; goto done; }
        }
    }
    if (!empty.size) {
        uint64_t bytes = continuation.size;
        while (bytes && (continuation.data[bytes - 1] == ' ' || continuation.data[bytes - 1] == '\t')) bytes--;
        if (bytes) doc_md_write(&empty, continuation.data, bytes);
    }
    for (at = start; at < end && result == XUI_OK;) {
        uint64_t content_end, line_end, last_nonspace, prefix_bytes = 0; unsigned padding = 0;
        result = doc_md_move_line_scan(t, at, end, &content_end, &line_end, &last_nonspace);
        if (result != XUI_OK) break;
        if (line_end > content_end) {
            ending_bytes = line_end - content_end;
            if (ending_bytes > sizeof(ending)) { result = XUI_ERROR_NOT_FOUND; break; }
            result = doc_seq_read(t->draft->source, content_end, ending, ending_bytes);
        }
        if (result == XUI_OK) {
            if (at == start && old_first) {
                prefix_bytes = old_open.size;
                if (paragraph->kind == XUI_DOC_CODE_BLOCK &&
                    paragraph->marker_kind == XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE) {
                    doc_md_indent_boundary boundary = {0};
                    result = doc_md_move_code_first_boundary(t, paragraph, &boundary);
                    if (result == XUI_OK && boundary.byte + (boundary.tab_right != 0) - at != prefix_bytes)
                        result = XUI_ERROR_NOT_FOUND;
                    padding = boundary.tab_right;
                }
            } else result = doc_md_move_line_prefix(t, paragraph, at, content_end, &prefix_bytes, &padding);
        }
        if (prefix_bytes > content_end - at) { result = XUI_ERROR_NOT_FOUND; break; }
        last_blank = last_nonspace <= at + prefix_bytes;
        if (result == XUI_OK) {
            doc_md_output* replacement = last_blank ? &empty : at == start && new_first ? &first : &continuation;
            doc_sequence** retained = last_blank ? &blank : at == start && new_first ? &opener : &prefix;
            if (!last_blank && paragraph->kind == XUI_DOC_CODE_BLOCK &&
                paragraph->marker_kind == XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE)
                result = doc_md_move_code_rebase(t, paragraph, new_parent, reference,
                    at == start && new_first, replacement, at, content_end, &prefix_bytes, &padding);
            if (result != XUI_OK) break;
            if (at == start && old_first) removed_brackets += doc_source_open_bracket_count(old_open.data, old_open.size);
            added_brackets += doc_source_open_bracket_count(replacement->data, replacement->size);
            if (!padding && prefix_bytes == replacement->size &&
                doc_seq_equal_bytes(t->draft->source, at, replacement->data, replacement->size))
                result = doc_md_move_slice(t, t->draft->source, at, line_end, &slice);
            else {
                if (replacement->size && !*retained) {
                    *retained = doc_seq_text(t->draft->allocator, replacement->data, replacement->size);
                    if (!*retained) result = XUI_ERROR_OUT_OF_MEMORY;
                    else fresh += replacement->size;
                }
                if (result == XUI_OK) result = doc_md_move_append(t, &payload, *retained);
                if (result == XUI_OK && padding) {
                    doc_sequence* spaces;
                    if (padding > 8) { result = XUI_ERROR_NOT_FOUND; break; }
                    spaces = doc_seq_text(t->draft->allocator, "        ", padding);
                    if (!spaces) result = XUI_ERROR_OUT_OF_MEMORY;
                    else { fresh += padding; result = doc_md_move_append(t, &payload, spaces); }
                    doc_seq_release(spaces);
                }
                if (result == XUI_OK) result = doc_md_move_slice(t, t->draft->source, at + prefix_bytes, line_end, &slice);
            }
            if (result == XUI_OK) result = doc_md_move_append(t, &payload, slice);
            doc_seq_release(slice); slice = NULL;
        }
        terminated = line_end > content_end; at = line_end;
    }
    /* A moved EOF paragraph can acquire a following sibling. Its payload
     * stays verbatim; only a required line/blank separator is added. */
    if (result == XUI_OK && move->iNewLength < new_count && (!terminated || !last_blank)) {
        doc_md_output separator = {0}; separator.allocator = t->draft->allocator;
        if (!terminated) doc_md_write(&separator, ending, ending_bytes);
        doc_md_write(&separator, empty.data, empty.size); doc_md_write(&separator, ending, ending_bytes);
        if (separator.error) result = separator.error;
        else {
            slice = doc_seq_text(t->draft->allocator, separator.data, separator.size);
            if (!slice) result = XUI_ERROR_OUT_OF_MEMORY;
            else { fresh += separator.size; result = doc_md_move_append(t, &payload, slice); }
        }
        doc_seq_release(slice); slice = NULL; doc_free(separator.data);
    }
    if (result == XUI_OK && old_empty)
        result = doc_md_move_retained_empty_slot(t, start, end, &old_open, &retained_slot, &fresh, &added_brackets);
    if (result == XUI_OK && (fresh > UINT64_MAX - t->draft->source_storage_bytes ||
        doc_seq_size(payload) > t->draft->allocator->max_bytes ||
        doc_seq_size(retained_slot) > t->draft->allocator->max_bytes - doc_seq_size(payload) ||
        doc_seq_size(t->draft->source) - (end - start) >
            t->draft->allocator->max_bytes - doc_seq_size(payload) - doc_seq_size(retained_slot)))
        result = XUI_DOC_ERROR_LIMIT;
    if (result == XUI_OK) result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_seq_replace(t->draft->allocator, candidate.draft->source, start, end, retained_slot, &without);
    /* Cut is half-open. Both adjacent endpoints remain valid insertion gaps. */
    if (insertion >= end) insertion = insertion - (end - start) + doc_seq_size(retained_slot);
    if (result == XUI_OK) result = doc_seq_replace(t->draft->allocator, without, insertion, insertion, payload, &source);
    if (result == XUI_OK) {
        xui_doc_operation_t operation = {0}; before = candidate.draft->source; doc_seq_retain(before);
        doc_seq_release(candidate.draft->source); candidate.draft->source = source; source = NULL;
        candidate.draft->source_storage_bytes += fresh;
        if (removed_brackets > candidate.draft->source_open_brackets ||
            added_brackets > UINT64_MAX - (candidate.draft->source_open_brackets - removed_brackets)) {
            result = XUI_DOC_ERROR_LIMIT; goto done;
        }
        candidate.draft->source_open_brackets = candidate.draft->source_open_brackets - removed_brackets + added_brackets;
        operation.iKind = XUI_DOC_OP_SOURCE;
        operation.iFlags = XUI_DOC_CHANGE_SOURCE | XUI_DOC_CHANGE_TEXT | XUI_DOC_CHANGE_STRUCTURE;
        operation.iOffset = start; operation.iOldLength = end - start;
        operation.iNewLength = doc_seq_size(retained_slot);
        result = doc_txn_op(&candidate, &operation);
        if (result == XUI_OK) {
            operation.iOffset = insertion; operation.iOldLength = 0; operation.iNewLength = doc_seq_size(payload);
            result = doc_txn_op(&candidate, &operation);
        }
        /* Remap the two outside prefixes through removal and insertion,
         * then patch from right to left. Payload blobs never enter a buffer. */
        for (unsigned i = 0; i < patch_count; i++) {
            if (patches[i].start >= end) {
                patches[i].start = patches[i].start - (end - start) + doc_seq_size(retained_slot);
                patches[i].end = patches[i].end - (end - start) + doc_seq_size(retained_slot);
            }
            if (patches[i].start >= insertion) { patches[i].start += doc_seq_size(payload); patches[i].end += doc_seq_size(payload); }
        }
        if (patch_count == 2 && patches[0].start < patches[1].start) {
            uint64_t a = patches[0].start, b = patches[0].end; doc_md_output* p = patches[0].prefix;
            patches[0].start = patches[1].start; patches[0].end = patches[1].end; patches[0].prefix = patches[1].prefix;
            patches[1].start = a; patches[1].end = b; patches[1].prefix = p;
        }
        for (unsigned i = 0; i < patch_count && result == XUI_OK; i++)
            result = doc_txn_source_patch(&candidate, patches[i].start, patches[i].end,
                patches[i].prefix->data, patches[i].prefix->size, 0);
        if (result == XUI_OK) {
            candidate.source_before_patch = before; result = doc_markdown_parse(&candidate); candidate.source_before_patch = NULL;
        }
    }
    /* A rejected definition candidate must never change the semantic intent
     * used by the fallback. Prove untouched spelling before planning flags. */
    if (result == XUI_OK && !doc_md_reference_spelling_equal_except_footnotes(t->draft->references, t->draft->source,
        candidate.draft->references, candidate.draft->source, editable_start, editable_end, other_start, other_end))
        result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK && ((old_empty && old_parent->kind == XUI_DOC_LIST_ITEM &&
        (old_parent->attrs->iFlags & XUI_DOC_TASK)) ||
        !doc_semantic_markdown_projection_equal(shadow->draft, candidate.draft)))
        result = doc_md_move_derive_lists(t, shadow, candidate.draft, old_parent->id, new_parent->id);
    if (result == XUI_OK && (!doc_semantic_markdown_projection_equal(shadow->draft, candidate.draft) ||
        !doc_md_reference_spelling_equal_except_footnotes(t->draft->references, t->draft->source,
            candidate.draft->references, candidate.draft->source, editable_start, editable_end, other_start, other_end)))
        result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (uint64_t i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) { doc_state_release(t->draft); t->draft = candidate.draft; candidate.draft = NULL; t->parse_op_start = t->count; }
    }
done:
    doc_free(first.data); doc_free(continuation.data); doc_free(empty.data);
    doc_free(old_open.data); doc_free(old_continue.data); doc_free(old_next.data); doc_free(old_unused.data);
    doc_free(new_next.data); doc_free(new_unused.data); doc_seq_release(opener); doc_seq_release(retained_slot);
    doc_seq_release(payload); doc_seq_release(slice); doc_seq_release(prefix); doc_seq_release(blank);
    doc_seq_release(without); doc_seq_release(source); doc_seq_release(before);
    doc_state_release(candidate.draft); doc_free(candidate.ops); return result;
}
static int doc_markdown_move_source_patch(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow)
{
    const xui_doc_operation_t* move;
    doc_node* parent;
    struct xui_doc_transaction_t candidate = {0};
    uint64_t count, first, last, split_index, i, start = DOC_NONE;
    uint64_t split = DOC_NONE, end = DOC_NONE, previous_end = 0;
    uint64_t editable_start = DOC_NONE, editable_end = DOC_NONE;
    int result;
    if (shadow->count != 1 || shadow->ops[0].iKind != XUI_DOC_OP_MOVE)
        return XUI_ERROR_NOT_FOUND;
    move = &shadow->ops[0];
    if (move->iParentId != move->iOtherNodeId)
        return doc_md_move_between_parents(t, shadow, move);
    if (move->iOffset == move->iNewLength)
        return XUI_ERROR_NOT_FOUND;
    parent = doc_index_get(t->draft->index, move->iParentId);
    if (!parent) return XUI_ERROR_NOT_FOUND;
    count = doc_seq_size(parent->children);
    first = move->iOffset < move->iNewLength ? move->iOffset : move->iNewLength;
    last = move->iOffset > move->iNewLength ? move->iOffset : move->iNewLength;
    split_index = move->iOffset < move->iNewLength ? first + 1 : last;
    if (last >= count || doc_seq_get_id(parent->children, move->iOffset) != move->iNodeId)
        return XUI_ERROR_NOT_FOUND;
    for (i = first; i <= last + (last + 1 < count); i++) {
        uint64_t block_start, block_end;
        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
        result = doc_md_bounds(t->draft, doc_seq_get_id(parent->children, i), &block_start, &block_end);
        if (result != XUI_OK || block_start >= block_end || (i > first && block_start < previous_end))
            return XUI_ERROR_NOT_FOUND;
        if (i == first) start = block_start;
        if (i == split_index) split = block_start;
        if (i == last + 1) { end = block_start; break; }
        previous_end = block_end;
    }
    if (last + 1 == count) {
        if (parent->id == DOC_ROOT) end = doc_seq_size(t->draft->source);
        else {
            uint64_t ignored;
            result = doc_md_bounds(t->draft, parent->id, &ignored, &end);
            if (result != XUI_OK || end < previous_end) return XUI_ERROR_NOT_FOUND;
        }
    }
    /* The file BOM is a document prefix, never part of a moved block. */
    if (!start && parent->id == DOC_ROOT && doc_seq_equal_bytes(t->draft->source, 0, "\xef\xbb\xbf", 3))
        start = 3;
    if (start == DOC_NONE || split == DOC_NONE || start >= split || split >= end)
        return XUI_ERROR_NOT_FOUND;
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK && !first &&
        (parent->kind == XUI_DOC_LIST_ITEM || parent->kind == XUI_DOC_FOOTNOTE)) {
        uint64_t promoted = move->iOffset ? move->iOffset : 1;
        result = doc_md_move_container_opener(t, &candidate, parent, promoted, &split, &end);
        if (parent->kind == XUI_DOC_FOOTNOTE) {
            doc_node_source_range range; doc_node_source_range_get(t->draft, parent, &range);
            editable_start = range.syntax_start; editable_end = range.syntax_end;
        }
    }
    if (result == XUI_OK) result = doc_md_source_rotate(&candidate, start, split, end);
    if (result == XUI_OK && (!doc_semantic_markdown_projection_equal(shadow->draft, candidate.draft) ||
        !doc_md_reference_spelling_equal_except_footnote(t->draft->references, t->draft->source,
            candidate.draft->references, candidate.draft->source, editable_start, editable_end))) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++) result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft;
            candidate.draft = NULL; t->parse_op_start = t->count;
        }
    }
    doc_state_release(candidate.draft); doc_free(candidate.ops);
    return result;
}
int doc_markdown_shadow_end(xui_document_transaction t, struct xui_doc_transaction_t* shadow, int result,
    const xui_doc_position_t* target, xui_doc_position_t* caret)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_markdown_move_source_patch(t, shadow);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_join_source_patch(t, shadow);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_code_language_source_patch(t, shadow);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_inline_delete_source_patch(t, shadow);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL, DOC_MD_ACCEPT_DERIVED_FOOTNOTES);
    }
    if (result == XUI_OK && shadow->count) result = doc_markdown_accept(t, shadow,DOC_MD_ACCEPT_DERIVED_FOOTNOTES);
    if (result == XUI_OK && target && caret) *caret = doc_markdown_resolve_caret(t, shadow->draft, *target);
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
/* Direct block splitting has the same source intent as inserting a paragraph
 * separator at the original semantic position. Share the checked CST plan
 * with ReplaceRange so untouched entities, wrappers and hidden definitions
 * survive. The semantic shadow remains the independent acceptance oracle. */
int doc_markdown_shadow_end_block_split(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow, int result, const xui_doc_position_t* at,
    const xui_doc_position_t* target, xui_doc_position_t* caret)
{
    if (result == XUI_OK && shadow->count) {
        xui_doc_range_t range;
        range.tAnchor = range.tCaret = *at;
        result = doc_markdown_range_source_patch(t, &range, "\n", 1, shadow->draft);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL, DOC_MD_ACCEPT_DERIVED_FOOTNOTES);
        if (result == XUI_OK) result = doc_markdown_accept(t, shadow, DOC_MD_ACCEPT_DERIVED_FOOTNOTES);
    }
    if (result == XUI_OK && target && caret) *caret = doc_markdown_resolve_caret(t, shadow->draft, *target);
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
int doc_markdown_shadow_end_list_split(xui_document_transaction t, struct xui_doc_transaction_t* shadow,
    int result, const xui_doc_position_t* at, const xui_doc_position_t* target, xui_doc_position_t* caret)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_md_list_split_source_patch(t, shadow->draft, at, target);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL, 0);
        if (result == XUI_OK) result = doc_markdown_accept(t, shadow,0);
    }
    if (result == XUI_OK && target && caret) *caret = doc_markdown_resolve_caret(t, shadow->draft, *target);
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
typedef struct doc_md_physical_row {
    uint64_t start,content,end,past;
    char ending[2];unsigned ending_bytes;
} doc_md_physical_row;
static int doc_md_cursor_under(const doc_source_cursor*,uint64_t);
static int doc_md_table_row_slot(xui_document_transaction t,const doc_source_cursor* cursor,
    uint32_t ordinal,doc_source_cst_record* out)
{
    uint64_t i;int result;
    for(i=cursor->root+1;i<cursor->header.count;i++){
        result=doc_txn_check(t,0);if(result!=XUI_OK)return result;
        result=doc_source_cursor_read(cursor,i,out);if(result!=XUI_OK)return result;
        if(out->kind==DOC_CST_FIELD && out->detail==DOC_CST_FIELD_TABLE_ROW &&
            out->metadata[2]==ordinal && doc_md_cursor_under(cursor,out->parent))return XUI_OK;
    }
    return XUI_ERROR_NOT_FOUND;
}
/* The parser owns row contents. This helper only locates physical line
 * boundaries and their unchanged container prefix, without parsing Markdown. */
static int doc_md_table_physical_row(xui_document_transaction t,const doc_source_cursor* cursor,
    const doc_source_cst_record* row,doc_md_physical_row* out)
{
    uint64_t at=cursor->base+row->start,size=doc_seq_size(t->draft->source);int result;char byte;
    memset(out,0,sizeof(*out));out->content=at;
    while(at){
        result=doc_txn_check(t,0);if(result!=XUI_OK)return result;
        result=doc_seq_read(t->draft->source,at-1,&byte,1);if(result!=XUI_OK)return result;
        if(byte=='\r'||byte=='\n')break;
        at--;
    }
    out->start=at;at=cursor->base+row->end;
    while(at<size){
        result=doc_txn_check(t,0);if(result!=XUI_OK)return result;
        result=doc_seq_read(t->draft->source,at,&byte,1);if(result!=XUI_OK)return result;
        if(byte=='\r'||byte=='\n')break;
        if(byte!=' '&&byte!='\t')return XUI_ERROR_NOT_FOUND;
        at++;
    }
    out->end=out->past=at;
    if(at<size){
        out->ending[out->ending_bytes++]=byte;out->past++;
        if(byte=='\r'&&out->past<size){
            result=doc_seq_read(t->draft->source,out->past,&byte,1);if(result!=XUI_OK)return result;
            if(byte=='\n'){out->ending[out->ending_bytes++]=byte;out->past++;}
        }
    }
    return XUI_OK;
}
static int doc_md_table_source_write(xui_document_transaction t,doc_md_output* output,uint64_t start,uint64_t end)
{
    char bytes[512];int result;
    if(start>end || end>doc_seq_size(t->draft->source))return XUI_DOC_ERROR_SCHEMA;
    while(start<end){
        uint64_t n=end-start>sizeof(bytes)?sizeof(bytes):end-start;
        result=doc_txn_check(t,0);if(result!=XUI_OK)return result;
        result=doc_seq_read(t->draft->source,start,bytes,n);if(result!=XUI_OK)return result;
        doc_md_write(output,bytes,n);if(output->error)return output->error;start+=n;
    }
    return XUI_OK;
}
static int doc_md_table_blank_row(xui_document_transaction t,doc_md_output* output,uint32_t columns)
{
    uint32_t i;int result;
    for(i=0;i<columns;i++){
        result=doc_txn_check(t,0);if(result!=XUI_OK)return result;
        doc_md_write(output,"|  ",3);if(output->error)return output->error;
    }
    doc_md_write(output,"|",1);return output->error;
}
/* A data row can have implicit missing cells or ignored extra cells. Once it
 * becomes the header, its physical width must match the alignment row. Keep
 * every retained cell's raw spelling and change only the width boundary. */
static int doc_md_table_promoted_header(xui_document_transaction t,const doc_source_cursor* cursor,
    const doc_source_cst_record* row,const doc_md_physical_row* physical,uint32_t columns,doc_md_output* output)
{
    uint64_t count=row->metadata[3],i,last=DOC_NONE;uint32_t missing;int result;
    if(count==columns)return doc_md_table_source_write(t,output,physical->content,physical->end);
    if(!count){
        result=doc_md_table_blank_row(t,output,columns);if(result!=XUI_OK)return result;
    }else{
        uint64_t wanted=count<columns?count-1:columns-1;
        for(i=cursor->root+1;i<cursor->header.count;i++){
            doc_source_cst_record cell;
            result=doc_txn_check(t,0);if(result!=XUI_OK)return result;
            result=doc_source_cursor_read(cursor,i,&cell);if(result!=XUI_OK)return result;
            if(cell.kind==DOC_CST_FIELD && cell.detail==DOC_CST_FIELD_TABLE_CELL &&
                cell.metadata[2]==row->metadata[2] && cell.metadata[3]==wanted &&
                doc_md_cursor_under(cursor,cell.parent)){
                if(cell.start<row->start || cell.end<cell.start || cell.end>row->end)return XUI_ERROR_NOT_FOUND;
                last=cell.end;break;
            }
        }
        if(last==DOC_NONE)return XUI_ERROR_NOT_FOUND;
        result=doc_md_table_source_write(t,output,physical->content,cursor->base+last);if(result!=XUI_OK)return result;
        if(count>columns)doc_md_write(output,"|",1);
        else{
            for(missing=(uint32_t)count;missing<columns;missing++){
                result=doc_txn_check(t,0);if(result!=XUI_OK)return result;
                doc_md_write(output,"|  ",3);if(output->error)return output->error;
            }
            doc_md_write(output,"|",1);
        }
    }
    if(output->error)return output->error;
    return doc_md_table_source_write(t,output,cursor->base+row->end,physical->end);
}
static int doc_md_table_footnote_scope(doc_state* state,doc_node* table,
    const doc_node_source_range* range,uint64_t* start,uint64_t* end)
{
    doc_node* ancestor=table;unsigned depth=0;*start=*end=DOC_NONE;
    while(ancestor && ancestor->id!=DOC_ROOT){
        if(++depth>DOC_MAX_DEPTH)return XUI_DOC_ERROR_LIMIT;
        if(ancestor->kind==XUI_DOC_FOOTNOTE){
            doc_node_source_range note;doc_node_source_range_get(state,ancestor,&note);
            if(note.syntax_start==DOC_NONE || note.syntax_end==DOC_NONE ||
                range->syntax_start<note.syntax_start || range->syntax_end>note.syntax_end)return XUI_ERROR_NOT_FOUND;
            *start=note.syntax_start;*end=note.syntax_end;break;
        }
        ancestor=doc_index_get(state->index,ancestor->parent);
    }
    return XUI_OK;
}
static int doc_markdown_table_row_source_patch(xui_document_transaction t,
    doc_state* desired,uint64_t table_id,uint32_t index,int insert)
{
    doc_state* old=t->draft;doc_node* table=doc_index_get(old->index,table_id);
    doc_node_source_range range;doc_source_cursor cursor;doc_source_cst_record header,alignment,target;
    doc_md_physical_row h,a,r;struct xui_doc_transaction_t candidate={0};doc_md_output output={0};
    uint64_t rows,editable_start,editable_end,i,at,end,remove_start=0,remove_end=0;
    uint32_t columns;int result=XUI_ERROR_NOT_FOUND,two_patches=0;
    if(!table || table->kind!=XUI_DOC_TABLE)return result;
    rows=doc_seq_size(table->children);columns=doc_table_column_count(old,table);
    if(!rows || rows>=UINT32_MAX || !columns || index>rows || (!insert&&index==rows))return result;
    doc_node_source_range_get(old,table,&range);
    if(range.syntax_start==DOC_NONE || range.syntax_end==DOC_NONE || range.syntax_start>=range.syntax_end)return result;
    result=doc_source_cursor_block(old,MD_BLOCK_TABLE,range.syntax_start,range.syntax_end,t->cancellation,&cursor);
    if(result!=XUI_OK)return result;
    result=doc_md_table_row_slot(t,&cursor,0,&header);if(result!=XUI_OK)return result;
    result=doc_md_table_row_slot(t,&cursor,1,&alignment);if(result!=XUI_OK)return result;
    result=doc_md_table_physical_row(t,&cursor,&header,&h);if(result!=XUI_OK)return result;
    result=doc_md_table_physical_row(t,&cursor,&alignment,&a);if(result!=XUI_OK)return result;
    if(!h.ending_bytes || h.past!=a.start)return XUI_ERROR_NOT_FOUND;
    result=doc_md_table_footnote_scope(old,table,&range,&editable_start,&editable_end);if(result!=XUI_OK)return result;
    output.allocator=old->allocator;
    if(!index && insert){
        result=doc_md_table_blank_row(t,&output,columns);if(result!=XUI_OK)goto done;
        doc_md_write(&output,h.ending,h.ending_bytes);
        result=doc_md_table_source_write(t,&output,a.start,a.past);if(result!=XUI_OK)goto done;
        if(!a.ending_bytes)doc_md_write(&output,h.ending,h.ending_bytes);
        result=doc_md_table_source_write(t,&output,a.start,a.content);if(result!=XUI_OK)goto done;
        at=end=h.content;remove_start=a.ending_bytes?a.start:h.end;remove_end=a.past;two_patches=1;
    }else if(!index && rows==1){
        at=h.content;end=a.past;
        if(table->parent!=DOC_ROOT && a.ending_bytes)doc_md_write(&output,a.ending,a.ending_bytes);
    }else{
        uint32_t source_row=insert && index==rows?(uint32_t)rows:index+1;
        if(!index)source_row=2;
        result=doc_md_table_row_slot(t,&cursor,source_row,&target);if(result!=XUI_OK)goto done;
        result=doc_md_table_physical_row(t,&cursor,&target,&r);if(result!=XUI_OK)goto done;
        if(!index){
            result=doc_md_table_promoted_header(t,&cursor,&target,&r,columns,&output);if(result!=XUI_OK)goto done;
            doc_md_write(&output,r.ending_bytes?r.ending:h.ending,r.ending_bytes?r.ending_bytes:h.ending_bytes);
            result=doc_md_table_source_write(t,&output,a.start,r.ending_bytes?a.past:a.end);if(result!=XUI_OK)goto done;
            at=h.content;end=r.past;
        }else if(insert){
            int append=index==rows;
            const char* ending=r.ending_bytes?r.ending:h.ending;
            unsigned ending_bytes=r.ending_bytes?r.ending_bytes:h.ending_bytes;
            if(append && !r.ending_bytes)doc_md_write(&output,ending,ending_bytes);
            result=doc_md_table_source_write(t,&output,a.start,a.content);if(result!=XUI_OK)goto done;
            result=doc_md_table_blank_row(t,&output,columns);if(result!=XUI_OK)goto done;
            if(!append || r.ending_bytes)doc_md_write(&output,ending,ending_bytes);
            at=end=append?r.past:r.start;
        }else{at=r.start;end=r.past;}
    }
    result=output.error;if(result!=XUI_OK)goto done;
    result=doc_markdown_shadow_begin(t,&candidate);if(result!=XUI_OK)goto done;
    /* Descending patches keep source coordinates in the old snapshot. */
    if(two_patches && remove_start>at){
        result=doc_txn_source_patch(&candidate,remove_start,remove_end,"",0,0);
        if(result==XUI_OK)result=doc_txn_source_patch(&candidate,at,end,output.data,output.size,0);
    }else{
        result=doc_txn_source_patch(&candidate,at,end,output.data,output.size,0);
        if(result==XUI_OK && two_patches)result=doc_txn_source_patch(&candidate,remove_start,remove_end,"",0,0);
    }
    if(result==XUI_OK)result=doc_markdown_parse(&candidate);
    if(result==XUI_OK && (!doc_semantic_markdown_projection_equal(desired,candidate.draft) ||
        !doc_md_reference_spelling_equal_except_footnote(old->references,old->source,
            candidate.draft->references,candidate.draft->source,editable_start,editable_end)))result=XUI_ERROR_NOT_FOUND;
    if(result==XUI_DOC_ERROR_FORMAT || result==XUI_DOC_ERROR_UNREPRESENTABLE || result==XUI_ERROR_UNSUPPORTED)result=XUI_ERROR_NOT_FOUND;
    if(result==XUI_OK){
        for(i=0;i<candidate.count && result==XUI_OK;i++)result=doc_txn_op(t,&candidate.ops[i]);
        if(result==XUI_OK){doc_state_release(t->draft);t->draft=candidate.draft;candidate.draft=NULL;t->parse_op_start=t->count;}
    }
done:
    doc_state_release(candidate.draft);doc_free(candidate.ops);doc_free(output.data);return result;
}
int doc_markdown_shadow_end_table_row(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow, int result, uint64_t table,
    uint32_t index, int insert)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_markdown_table_row_source_patch(t, shadow->draft,
            table, index, insert);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL, 0);
        if (result == XUI_OK) result = doc_markdown_accept(t, shadow,DOC_MD_ACCEPT_DERIVED_FOOTNOTES);
    }
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
/* Raw table slots come from the authoritative parser's existing cell split.
 * The grammar cursor works equally for root blocks, containers and definitions. */
typedef struct doc_md_table_patch {
    uint64_t start,end;
    const char* text;
    uint64_t bytes;
} doc_md_table_patch;
static int doc_md_cursor_under(const doc_source_cursor* cursor,uint64_t parent)
{
    unsigned depth=0;
    while(parent!=DOC_NONE){
        doc_source_cst_record r;int result;
        if(parent==cursor->root)return 1;
        if(++depth>DOC_MAX_DEPTH)return 0;
        result=doc_source_cursor_read(cursor,parent,&r);
        if(result!=XUI_OK || r.parent>=parent)return 0;
        parent=r.parent;
    }
    return 0;
}
static int doc_md_table_row_column_patch(xui_document_transaction t,const doc_source_cursor* cursor,
    uint64_t record,const doc_source_cst_record* row,uint32_t columns,uint32_t column,int insert,
    doc_md_table_patch* patch,uint64_t* next)
{
    uint64_t j=record+1,count=row->metadata[3],seen=0,first=0,last=0,previous=0;
    uint64_t selected_start=0,selected_end=0,left_end=0,right_start=0;
    int leading,closing,result,previous_empty=0,left_empty=0,right_empty=0;char byte;
    memset(patch,0,sizeof(*patch));patch->text="";
    if(!count){*next=j;return row->metadata[2]>=2?XUI_OK:XUI_ERROR_NOT_FOUND;}
    for(;j<cursor->header.count && seen<count;j++){
        doc_source_cst_record cell;
        result=doc_txn_check(t,0);if(result!=XUI_OK)return result;
        result=doc_source_cursor_read(cursor,j,&cell);if(result!=XUI_OK)return result;
        if(cell.kind!=DOC_CST_FIELD || cell.detail!=DOC_CST_FIELD_TABLE_CELL)continue;
        if(cell.parent!=record || cell.metadata[2]!=row->metadata[2] || cell.metadata[3]!=seen ||
            cell.start<row->start || cell.end<cell.start || cell.end>row->end ||
            cell.start>cell.metadata[0] || cell.metadata[0]>cell.metadata[1] || cell.metadata[1]>cell.end ||
            (seen && (previous==UINT64_MAX || cell.start!=previous+1)))return XUI_ERROR_NOT_FOUND;
        if(seen){
            result=doc_seq_read(t->draft->source,cursor->base+previous,&byte,1);if(result!=XUI_OK)return result;
            if(byte!='|')return XUI_ERROR_NOT_FOUND;
        }else first=cell.start;
        if(seen==column){selected_start=cell.start;selected_end=cell.end;left_end=previous;left_empty=previous_empty;}
        if(seen==(uint64_t)column+1){right_start=cell.start;right_empty=cell.metadata[0]==cell.metadata[1];}
        previous_empty=cell.metadata[0]==cell.metadata[1];
        previous=last=cell.end;seen++;
    }
    if(seen!=count || first<row->start || first-row->start>1 || row->end<last || row->end-last>1)
        return XUI_ERROR_NOT_FOUND;
    leading=first!=row->start;closing=last!=row->end;
    if(leading){result=doc_seq_read(t->draft->source,cursor->base+row->start,&byte,1);if(result!=XUI_OK)return result;if(byte!='|')return XUI_ERROR_NOT_FOUND;}
    if(closing){result=doc_seq_read(t->draft->source,cursor->base+last,&byte,1);if(result!=XUI_OK)return result;if(byte!='|')return XUI_ERROR_NOT_FOUND;}
    *next=j;
    /* Missing trailing cells are already semantic empties. Width changes do
     * not require materializing them or touching their original source row. */
    if(column>=count && count<columns)return XUI_OK;
    if(insert){
        if(column<count){
            patch->start=patch->end=selected_start;
            patch->text=row->metadata[2]==1?" --- |":"  |";
            if(!column && !leading)patch->text=row->metadata[2]==1?"| --- |":"|  |";
        }else if(column==count){
            patch->start=patch->end=last+(closing?1:0);
            patch->text=row->metadata[2]==1?(closing?" --- |":"| --- |"):(closing?"  |":"|  |");
        }else return XUI_ERROR_NOT_FOUND;
    }else if(column<count){
        if(count==1){patch->start=first;patch->end=last;}
        else if((uint64_t)column+1<count){
            patch->start=selected_start;patch->end=right_start;
            if(!column && !leading && (right_empty || (count==2 && !closing)))patch->text="|";
        }else{
            patch->start=left_end;patch->end=selected_end;
            if(!closing && (left_empty || (count==2 && !leading)))patch->text="|";
        }
    }else return XUI_ERROR_NOT_FOUND;
    patch->start+=cursor->base;patch->end+=cursor->base;patch->bytes=strlen(patch->text);
    return XUI_OK;
}
static int doc_markdown_table_column_source_patch(xui_document_transaction t,
    doc_state* desired,uint64_t table_id,uint32_t column,int insert)
{
    doc_state* old=t->draft;doc_node* table=doc_index_get(old->index,table_id);
    doc_node_source_range range;doc_source_cursor cursor;
    struct xui_doc_transaction_t candidate={0};doc_md_table_patch* patches=NULL;
    uint64_t rows,i,seen=0,count=0,editable_start=DOC_NONE,editable_end=DOC_NONE;
    uint32_t columns;int result=XUI_ERROR_NOT_FOUND;
    if(!table || table->kind!=XUI_DOC_TABLE)return result;
    columns=doc_table_column_count(old,table);rows=doc_seq_size(table->children);
    if(!columns || !rows || rows>=UINT32_MAX || column>columns ||
        (!insert && (column>=columns || columns==1)))return result;
    doc_node_source_range_get(old,table,&range);
    if(range.syntax_start==DOC_NONE || range.syntax_end==DOC_NONE || range.syntax_end<=range.syntax_start ||
        range.syntax_end>doc_seq_size(old->source))return result;
    result=doc_source_cursor_block(old,MD_BLOCK_TABLE,range.syntax_start,range.syntax_end,t->cancellation,&cursor);
    if(result!=XUI_OK)return result;
    result=doc_md_table_footnote_scope(old,table,&range,&editable_start,&editable_end);if(result!=XUI_OK)return result;
    if(rows+1>SIZE_MAX/sizeof(*patches))return XUI_DOC_ERROR_LIMIT;
    patches=doc_alloc(old->allocator,(size_t)(rows+1)*sizeof(*patches));if(!patches)return XUI_ERROR_OUT_OF_MEMORY;
    for(i=cursor.root+1;i<cursor.header.count;i++){
        doc_source_cst_record row;doc_md_table_patch patch;uint64_t next;
        result=doc_txn_check(t,0);if(result!=XUI_OK)goto done;
        result=doc_source_cursor_read(&cursor,i,&row);if(result!=XUI_OK)goto done;
        if(row.kind!=DOC_CST_FIELD || row.detail!=DOC_CST_FIELD_TABLE_ROW || !doc_md_cursor_under(&cursor,row.parent))continue;
        if(seen>rows || row.metadata[2]!=seen || (seen<2 && row.metadata[3]!=columns)){
            result=XUI_ERROR_NOT_FOUND;goto done;
        }
        result=doc_md_table_row_column_patch(t,&cursor,i,&row,columns,column,insert,&patch,&next);
        if(result!=XUI_OK)goto done;
        if(patch.start!=patch.end || patch.bytes){
            if(count && patch.start<patches[count-1].end){result=XUI_ERROR_NOT_FOUND;goto done;}
            patches[count++]=patch;
        }
        seen++;i=next-1;
    }
    if(seen!=rows+1){result=XUI_ERROR_NOT_FOUND;goto done;}
    result=doc_markdown_shadow_begin(t,&candidate);if(result!=XUI_OK)goto done;
    for(i=count;i && result==XUI_OK;){
        const doc_md_table_patch* p=patches+--i;
        result=doc_txn_source_patch(&candidate,p->start,p->end,p->text,p->bytes,0);
    }
    if(result==XUI_OK)result=doc_markdown_parse(&candidate);
    if(result==XUI_OK && (!doc_semantic_markdown_projection_equal(desired,candidate.draft) ||
        !doc_md_reference_spelling_equal_except_footnote(old->references,old->source,
            candidate.draft->references,candidate.draft->source,editable_start,editable_end)))
        result=XUI_ERROR_NOT_FOUND;
    if(result==XUI_DOC_ERROR_FORMAT || result==XUI_DOC_ERROR_UNREPRESENTABLE || result==XUI_ERROR_UNSUPPORTED)
        result=XUI_ERROR_NOT_FOUND;
    if(result==XUI_OK){
        for(i=0;i<candidate.count && result==XUI_OK;i++)result=doc_txn_op(t,&candidate.ops[i]);
        if(result==XUI_OK){doc_state_release(t->draft);t->draft=candidate.draft;candidate.draft=NULL;t->parse_op_start=t->count;}
    }
done:
    doc_state_release(candidate.draft);doc_free(candidate.ops);doc_free(patches);return result;
}
int doc_markdown_shadow_end_table_column(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow, int result, uint64_t table,
    uint32_t column, int insert)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_markdown_table_column_source_patch(t, shadow->draft,
            table, column, insert);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL, 0);
        if (result == XUI_OK) result = doc_markdown_accept(t, shadow,DOC_MD_ACCEPT_DERIVED_FOOTNOTES);
    }
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
int doc_markdown_shadow_end_image(xui_document_transaction t, struct xui_doc_transaction_t* shadow,
    int result, uint64_t image_id)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_markdown_image_patch(t, image_id, shadow->draft);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL, 0);
        if (result == XUI_OK) result = doc_markdown_accept(t, shadow,0);
    }
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
int doc_markdown_shadow_end_insert_image(xui_document_transaction t, struct xui_doc_transaction_t* shadow,
    int result, const xui_doc_range_t* range, uint64_t image_id,
    const xui_doc_position_t* target, xui_doc_position_t* caret)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_markdown_image_insert_patch(t, range, image_id, shadow->draft);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL, 0);
        if (result == XUI_OK) result = doc_markdown_accept(t, shadow,0);
    }
    if (result == XUI_OK && target && caret) *caret = doc_markdown_resolve_caret(t, shadow->draft, *target);
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
/* Unchanged subtrees retain their complete original syntax, including hidden
 * definitions. A partially retained quote contributes its surviving children;
 * its opener is already present on their physical source lines. List/item
 * splits need new markers and continue through the general writer. */
static int doc_md_quote_source_extent(xui_document_transaction t, doc_state* desired,
    uint64_t id, unsigned depth, uint64_t* start, uint64_t* end)
{
    doc_node* node = doc_index_get(desired->index, id);
    doc_node* original = doc_index_get(t->draft->index, id);
    uint64_t i, from, to; int result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    if (!node || depth >= DOC_MAX_DEPTH) return XUI_ERROR_NOT_FOUND;
    if (original && doc_semantic_subtree_equal(t->draft, id, desired, id)) {
        result = doc_md_bounds(t->draft, id, &from, &to);
        if (result != XUI_OK || from >= to || (*start != DOC_NONE && from < *end))
            return XUI_ERROR_NOT_FOUND;
        if (*start == DOC_NONE) *start = from;
        *end = to; return XUI_OK;
    }
    if (node->kind != XUI_DOC_QUOTE || !doc_seq_size(node->children))
        return XUI_ERROR_NOT_FOUND;
    for (i = 0; i < doc_seq_size(node->children); i++) {
        result = doc_md_quote_source_extent(t, desired,
            doc_seq_get_id(node->children, i), depth + 1, start, end);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
/* Wrapping at the root only adds quote prefixes and blank boundary lines.
 * Preserve every existing byte in the selected span rather than regenerating
 * visible nodes and trying to reconstruct source-only definitions afterward.
 * A private complete parse must match both the desired tree and all ordered
 * definition values, including unused duplicates and dedented footnote bodies. */
static int doc_markdown_quote_range_source_patch(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow, uint64_t quote_id)
{
    struct xui_doc_transaction_t candidate = {0}; doc_md_output output = {0}; doc_md_source_plan source_plan = {0};
    doc_node* quote = doc_index_get(shadow->draft->index, quote_id);
    doc_state* old = t->draft; char* source = NULL;
    uint64_t start = DOC_NONE, end = 0, i, at, bytes, source_size;
    const char* ending = "\n"; uint64_t ending_bytes = 1;
    int result = XUI_ERROR_NOT_FOUND, equal, bom_start = 0;
    if (!quote || quote->kind != XUI_DOC_QUOTE || quote->parent != DOC_ROOT ||
        doc_index_get(old->index, quote_id) || !doc_seq_size(quote->children)) return result;
    for (i = 0; i < doc_seq_size(quote->children); i++) {
        result = doc_md_quote_source_extent(t, shadow->draft,
            doc_seq_get_id(quote->children, i), 0, &start, &end);
        if (result != XUI_OK) return result;
    }
    source_size = doc_seq_size(old->source);
    if (start == DOC_NONE || start >= end || end > source_size || end - start >= SIZE_MAX)
        return XUI_ERROR_NOT_FOUND;
    if (start) {
        char before[3]; uint64_t count = start > 3 ? 3 : start;
        result = doc_seq_read(old->source, start - count, before, count);
        if (result != XUI_OK) return result;
        bom_start = start == 3 && !memcmp(before, "\xef\xbb\xbf", 3);
        if (before[count - 1] != '\n' && before[count - 1] != '\r' &&
            !bom_start)
            return XUI_ERROR_NOT_FOUND;
    }
    for (i = 0; i < doc_seq_size(old->references); i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
        result = doc_seq_read(old->references, i, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        /* Never move a partial definition. Complete footnotes also have
         * parser-confirmed body values, including definitions not emitted. */
        if (ref.iSourceStart < end && ref.iSourceEnd > start &&
            (ref.iSourceStart < start || ref.iSourceEnd > end ||
                (ref.iKind != XUI_DOC_REFERENCE_LINK &&
                 ref.iKind != XUI_DOC_REFERENCE_FOOTNOTE))) return XUI_ERROR_NOT_FOUND;
    }
    bytes = end - start;
    source = doc_alloc(t->document->allocator, (size_t)bytes + 1);
    if (!source) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_read(old->source, start, source, bytes);
    if (result != XUI_OK) goto done;
    source[bytes] = 0;
    if (source[bytes - 1] == '\r') ending = "\r";
    else if (bytes >= 2 && source[bytes - 2] == '\r' && source[bytes - 1] == '\n') {
        ending = "\r\n"; ending_bytes = 2;
    }
    output.allocator = t->document->allocator;
    output.plan = &source_plan;
    unsigned quote_padding;
    result = doc_md_quote_tab_padding(t, source, 0, 0, bytes, &quote_padding);
    if (result != XUI_OK) goto done;
    if (start && !bom_start)
        doc_md_write(&output, ending, ending_bytes);
    for (at = 0; at < bytes;) {
        uint64_t next = at;
        result = doc_txn_check(t, 0); if (result != XUI_OK) goto done;
        while (next < bytes && source[next] != '\r' && source[next] != '\n') {
            if (!(next & 16383)) {
                result = doc_txn_check(t, 0); if (result != XUI_OK) goto done;
            }
            next++;
        }
        if (next < bytes && source[next++] == '\r' && next < bytes && source[next] == '\n') next++;
        doc_md_write(&output, quote_padding ? "  > " : "> ", 2 + quote_padding);
        doc_md_copy_source(&output, source + at, start + at, next - at);
        at = next;
    }
    if (end < source_size) {
        if (source[bytes - 1] != '\r' && source[bytes - 1] != '\n')
            doc_md_write(&output, ending, ending_bytes);
        doc_md_write(&output, ending, ending_bytes);
    }
    result = output.error ? output.error : doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_md_apply_source_plan(&candidate, start, end, &output, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK) result = doc_md_reference_values_equal(t, old, candidate.draft, &equal);
    if (result == XUI_OK && (!equal || !doc_semantic_equal(shadow->draft, candidate.draft)))
        result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_DOC_ERROR_FORMAT || result == XUI_DOC_ERROR_UNREPRESENTABLE ||
        result == XUI_ERROR_UNSUPPORTED) result = XUI_ERROR_NOT_FOUND;
    if (result == XUI_OK) {
        for (i = 0; i < candidate.count && result == XUI_OK; i++)
            result = doc_txn_op(t, &candidate.ops[i]);
        if (result == XUI_OK) {
            doc_state_release(t->draft); t->draft = candidate.draft;
            candidate.draft = NULL; t->parse_op_start = t->count;
        }
    }
done:
    doc_state_release(candidate.draft); doc_free(candidate.ops);
    doc_free(source); doc_free(output.data); doc_free(source_plan.copies); return result;
}
int doc_markdown_shadow_end_quote_range(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow, int result, uint64_t quote,
    const xui_doc_range_t* target, xui_doc_range_t* after)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_markdown_quote_range_source_patch(t, shadow, quote);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL, 0);
    }
    if (result == XUI_OK && shadow->count) result = doc_markdown_accept(t, shadow,0);
    if (result == XUI_OK && target && after) {
        after->tAnchor = doc_markdown_resolve_caret(t, shadow->draft, target->tAnchor);
        after->tCaret = doc_markdown_resolve_caret(t, shadow->draft, target->tCaret);
    }
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
int doc_markdown_shadow_end_range(xui_document_transaction t, struct xui_doc_transaction_t* shadow, int result,
    const xui_doc_range_t* target, xui_doc_range_t* after)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_markdown_move_source_patch(t, shadow);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_md_unlist_source_patch(t, shadow);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_heading_marker_patch(t, shadow);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL, DOC_MD_ACCEPT_DERIVED_FOOTNOTES);
    }
    if (result == XUI_OK && shadow->count) result = doc_markdown_accept(t, shadow,DOC_MD_ACCEPT_DERIVED_FOOTNOTES);
    if (result == XUI_OK && target && after) {
        after->tAnchor = doc_markdown_resolve_caret(t, shadow->draft, target->tAnchor);
        after->tCaret = doc_markdown_resolve_caret(t, shadow->draft, target->tCaret);
    }
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}

#endif
