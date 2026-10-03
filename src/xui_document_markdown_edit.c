#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include "xui_document_md4c.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct doc_md_output { doc_allocator* allocator; char* data; uint64_t size, capacity; int error; char list_marker; } doc_md_output;
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
static void doc_md_image_token(doc_md_output* b, doc_node* n,
    const char* text, uint64_t length, int include_link)
{
    const char *uri = doc_string(n->resource), *title = doc_string(n->title);
    unsigned alt_boundary = (doc_md_text_edge_space(n->text, 0) ? DOC_MD_ENTITY_FIRST : 0) |
        (doc_md_text_edge_space(n->text, 1) ? DOC_MD_ENTITY_LAST : 0);
    if (include_link && (n->attrs->iMarks & XUI_DOC_LINK)) doc_md_write(b, "[", 1);
    doc_md_write(b, "![", 2);
    doc_md_escape_boundary(b, text, length, alt_boundary);
    doc_md_write(b, "](<", 3); doc_md_escape(b, uri, strlen(uri));
    doc_md_write(b, ">", 1);
    if (*title) {
        doc_md_write(b, " \"", 2); doc_md_escape(b, title, strlen(title));
        doc_md_write(b, "\"", 1);
    }
    doc_md_write(b, ")", 1);
    if (include_link && (n->attrs->iMarks & XUI_DOC_LINK)) {
        const char *target = doc_string(n->link_target), *link_title = doc_string(n->link_title);
        doc_md_write(b, "](<", 3); doc_md_escape(b, target, strlen(target));
        doc_md_write(b, ">", 1);
        if (*link_title) {
            doc_md_write(b, " \"", 2); doc_md_escape(b, link_title, strlen(link_title));
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
        if (*title) { doc_md_write(b, " \"", 2); doc_md_escape(b, title, strlen(title)); doc_md_write(b, "\"", 1); }
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
            else if (n->kind == XUI_DOC_CODE_BLOCK) doc_md_write(b, doc_string(n->info), strlen(doc_string(n->info)));
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
static int doc_md_reference_spelling_equal_except_footnote(
    doc_sequence* old_refs, doc_sequence* old_source,
    doc_sequence* new_refs, doc_sequence* new_source,
    uint64_t editable_start, uint64_t editable_end)
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
            a.iSourceStart == editable_start && a.iSourceEnd == editable_end) {
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
 * instead of mistaking a newly inserted quote marker for title content.
 * Footnotes do not have value blobs: keep their complete spelling unchanged. */
static int doc_md_quote_prefix_references(xui_document_transaction t,
    doc_state* old, doc_state* actual, int* equal)
{
    uint64_t i; int result = doc_txn_check(t, 0);
    *equal = 0;
    if (result != XUI_OK) return result;
    result = doc_reference_link_values_equal(old, actual, t->cancellation, equal);
    if (result != XUI_OK || !*equal) return result;
    for (i = 0; i < doc_seq_size(old->references); i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t a, b;
        result = doc_txn_check(t, 0); if (result != XUI_OK) return result;
        result = doc_seq_read(old->references, i, &a, sizeof(a));
        if (result == XUI_OK) result = doc_seq_read(actual->references, i, &b, sizeof(b));
        if (result != XUI_OK) return result;
        if (a.iKind != b.iKind || (a.iKind == XUI_DOC_REFERENCE_FOOTNOTE &&
            !doc_md_source_spelling_equal(old->source, a.iSourceStart, a.iSourceEnd,
                actual->source, b.iSourceStart, b.iSourceEnd))) {
            *equal = 0; break;
        }
    }
    return XUI_OK;
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
    enum { prefix_quote = 1, prefix_list = 2, prefix_task = 3 };
    struct xui_doc_snapshot_t snapshot = {0};
    unsigned char* tokens = NULL;
    doc_node* ancestor = doc_index_get(t->draft->index, paragraph->parent);
    uint64_t i, end = 0, column = 0, continued_column = 0;
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
        if (ancestor->kind == XUI_DOC_LIST_ITEM || ancestor->kind == XUI_DOC_QUOTE) {
            result = xuiDocumentSnapshotGetBlockSyntax(&snapshot, ancestor->id, &syntax);
            if (result != XUI_OK) goto done;
            if (ancestor->kind == XUI_DOC_LIST_ITEM) {
                start = syntax.iPrimaryStart;
                stop = syntax.iTaskMarkerGapEnd == DOC_NONE ? syntax.iListMarkerGapEnd : syntax.iTaskMarkerGapEnd;
                if (start >= range->syntax_start && start < range->source_start) {
                    if (stop < start || stop > range->source_start ||
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
        uint64_t width = raw[i] == '\t' ? 4 - column % 4 : 1;
        column += width;
        if (tokens[i] == prefix_task) continue;
        if (tokens[i] == prefix_list && raw[i] != ' ' && raw[i] != '\t') {
            doc_md_write(continuation, " ", 1); continued_column++;
        } else if (raw[i] == '\t' && 4 - continued_column % 4 != width) {
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
                uint64_t width;
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
    return doc_reference_link_values_equal(old, actual, t->cancellation, equal);
}
static int doc_md_list_split_references(xui_document_transaction t, doc_state* old, doc_state* actual,
    uint64_t tail_start, uint64_t tail_end, int reindented, int* equal)
{
    uint64_t i; int needs_values = 0; *equal = 0;
    if (!reindented) { *equal = doc_md_reference_spelling_equal(old->references, old->source, actual->references, actual->source); return XUI_OK; }
    if (doc_seq_size(old->references) != doc_seq_size(actual->references)) return XUI_OK;
    for (i = 0; i < doc_seq_size(old->references); i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t a, b;
        if (doc_seq_read(old->references, i, &a, sizeof(a)) != XUI_OK ||
            doc_seq_read(actual->references, i, &b, sizeof(b)) != XUI_OK || a.iKind != b.iKind) return XUI_OK;
        if (a.iKind == XUI_DOC_REFERENCE_LINK && a.iSourceStart >= tail_start && a.iSourceEnd <= tail_end) {
            if (!doc_md_reference_fields_equal(old->source, a.iLabelStart, a.iLabelEnd, actual->source, b.iLabelStart, b.iLabelEnd) ||
                !doc_md_reference_fields_equal(old->source, a.iDestinationStart, a.iDestinationEnd, actual->source, b.iDestinationStart, b.iDestinationEnd) ||
                !doc_md_reference_fields_equal(old->source, a.iTitleStart, a.iTitleEnd, actual->source, b.iTitleStart, b.iTitleEnd)) needs_values = 1;
        } else if (!doc_md_source_spelling_equal(old->source, a.iSourceStart, a.iSourceEnd, actual->source, b.iSourceStart, b.iSourceEnd)) return XUI_OK;
    }
    if (needs_values) return doc_md_reference_values_equal(t, old, actual, equal);
    *equal = 1; return XUI_OK;
}
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
    struct xui_doc_transaction_t candidate = {0};
    doc_md_output first_prefix = {0}, continuation = {0}, outer = {0},
        next_prefix = {0}, next_continuation = {0}, left_body = {0}, right_body = {0}, output = {0};
    char *raw = NULL, *gap = NULL, *task_gap = NULL, marker[32], ending[2] = {'\n', 0}, check[2];
    uint64_t prefix_bytes, marker_bytes, gap_bytes, task_gap_bytes = 0,
        outer_column = DOC_NONE, i, item_index, ending_bytes = 1;
    uint64_t old_width, new_width, column;
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
    for (i = 0; i < doc_seq_size(old->references); i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t reference;
        doc_seq_read(old->references, i, &reference, sizeof(reference));
        if (reference.iSourceStart < range.syntax_end && reference.iSourceEnd > range.syntax_start) return result;
    }
    snapshot.state = old; snapshot.identity = t->document->identity; snapshot.revision = t->base_revision;
    syntax.iSize = sizeof(syntax);
    result = xuiDocumentSnapshotGetBlockSyntax(&snapshot, item->id, &syntax);
    if (result != XUI_OK) return result;
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
    column = outer_column + marker_bytes;
    for (i = 0; i < gap_bytes; i++) column += gap[i] == '\t' ? 4 - column % 4 : 1;
    old_width = column - outer_column;
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
    column = 0;
    for (i = 0; i < outer.size; i++) column += outer.data[i] == '\t' ? 4 - column % 4 : 1;
    new_width = column; column += marker_bytes;
    for (i = 0; i < gap_bytes; i++) column += gap[i] == '\t' ? 4 - column % 4 : 1;
    new_width = column - new_width;
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
            range.syntax_end, item_range.syntax_end, indent_delta != 0, &references_equal);
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
int doc_markdown_link_patch(xui_document_transaction t, const xui_doc_range_t* range,
    const char* uri, const char* title, doc_state* desired)
{
    struct xui_doc_transaction_t candidate = {0}; doc_md_output output = {0};
    doc_node* node; doc_node_source_range source_range;
    uint64_t start, end, a, b, length, i; char* raw = NULL;
    int result = XUI_ERROR_NOT_FOUND;
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
    if (title && *title) { doc_md_write(&output, " \"", 2); doc_md_escape(&output, title, strlen(title)); doc_md_write(&output, "\"", 1); }
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
int doc_markdown_image_patch(xui_document_transaction t, uint64_t image_id, doc_state* desired)
{
    struct xui_doc_transaction_t candidate = {0}; doc_md_output output = {0};
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
        if (range.source_start == range.source_end &&
            (syntax.iContentStart != range.source_start ||
             syntax.iContentEnd != range.source_end)) continue;
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
    if (alt_only) {
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
        (source[selected_end - 1 - source_base] != '\n' &&
            source[selected_end - 1 - source_base] != '\r') ||
        old_quote->marker_kind != XUI_DOC_BLOCK_SYNTAX_QUOTE_OPEN ||
        (old_quote->quote_prefixes &&
            old_quote->quote_prefixes->size % sizeof(uint32_t)))
        return XUI_DOC_ERROR_UNREPRESENTABLE;
    marker_count = 1 + (old_quote->quote_prefixes ?
        old_quote->quote_prefixes->size / sizeof(uint32_t) : 0);
    cursor = start;
    for (at = selected_start; at < selected_end;) {
        uint64_t line_end = at, marker, next;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        while (line_end < selected_end && source[line_end - source_base] != '\n' &&
            source[line_end - source_base] != '\r') line_end++;
        if (line_end == selected_end) return XUI_DOC_ERROR_UNREPRESENTABLE;
        if (source[line_end++ - source_base] == '\r' &&
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
        doc_md_write(output, source + cursor - source_base, marker + 1 - cursor);
        doc_md_write(output, " >", 2);
        if (output->error) return output->error;
        cursor = marker + 1; marker_index++;
        at = line_end;
    }
    doc_md_write(output, source + cursor - source_base, end - cursor);
    if (output->error) return output->error;
    *raw_suffix = 1;
    return XUI_OK;
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
    const char* inferred_prefix = NULL;
    uint64_t inferred_size = 0;
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
            uint64_t indent_start, indent_end, line_start;
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
            blank_prefix_start = line_start;
            blank_prefix_end = indent_end;
            break;
        }
    }
    if (blank_prefix_start != DOC_NONE) {
        inferred_prefix = source + blank_prefix_start - source_base;
        inferred_size = blank_prefix_end - blank_prefix_start;
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
                insert = indent_end;
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
                doc_md_write(output, source + cursor - source_base,
                    line_end - cursor);
                doc_md_write(output, inferred_prefix + existing,
                    prefix_bytes - existing);
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
            doc_md_write(output, source + cursor - source_base,
                at + existing - cursor);
            doc_md_write(output, inferred_prefix + existing,
                prefix_bytes - existing);
            doc_md_write(output, "> ", 2);
            if (output->error) return output->error;
            cursor = at + existing;
            at = next;
            continue;
        }
        if (insert == DOC_NONE || insert < cursor || insert > line_end)
            return XUI_DOC_ERROR_UNREPRESENTABLE;
        doc_md_write(output, source + cursor - source_base, insert - cursor);
        doc_md_write(output, "> ", 2);
        if (output->error) return output->error;
        cursor = insert;
        at = next;
    }
    doc_md_write(output, source + cursor - source_base, end - cursor);
    if (output->error) return output->error;
    *raw_suffix = 1;
    return XUI_OK;
}
int doc_markdown_apply_tree(xui_document_transaction t, doc_state* desired, const xui_doc_position_t* target, xui_doc_position_t* caret)
{
    doc_state* old = t->draft; doc_node *before, *after;
    doc_state* reference_state = NULL;
    doc_md_output output = {0}; char* source = NULL;
    uint64_t prefix = 0, suffix = 0, ac, bc, i, start, end, source_base = 0, unused;
    unsigned char* referenced_items = NULL;
    doc_sequence *saved_references = NULL, *saved_source = NULL;
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
    while (prefix < ac && prefix < bc && doc_semantic_subtree_equal(old, doc_seq_get_id(before->children, prefix), desired, doc_seq_get_id(after->children, prefix))) prefix++;
    while (suffix < ac - prefix && suffix < bc - prefix && doc_semantic_subtree_equal(old, doc_seq_get_id(before->children, ac - suffix - 1), desired, doc_seq_get_id(after->children, bc - suffix - 1))) suffix++;
    if (prefix == ac && prefix == bc) { if (caret && target) *caret = doc_markdown_resolve_caret(t, desired, *target); goto done; }
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
            if (reference.iKind != XUI_DOC_REFERENCE_LINK ||
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
    result = doc_txn_source_patch(t, start, end, output.data ? output.data : "",
        output.size, immediate_single_block);
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
    if (result == XUI_OK && !doc_semantic_equal(desired, t->draft))
        result = XUI_DOC_ERROR_UNREPRESENTABLE;
    if (result == XUI_OK && target && caret) *caret = doc_markdown_resolve_caret(t, desired, *target);
done:
    doc_seq_release(saved_references); doc_seq_release(saved_source);
    doc_state_release(reference_state);
    doc_free(referenced_items); doc_free(source); doc_free(output.data);
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
static int doc_md_removed_item_paragraph_body(xui_document_transaction t,
    doc_state* old, doc_node* item, doc_node* paragraph,
    uint64_t* body_start, uint64_t* body_end)
{
    doc_node_source_range range;
    char prefix[256], ending;
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
    bytes = range.source_start - range.syntax_start;
    if (bytes > sizeof(prefix)) bytes = sizeof(prefix);
    result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    result = doc_seq_read(old->source, range.syntax_start, prefix, bytes);
    if (result != XUI_OK) return result;
    while (marker_start < bytes &&
        (prefix[marker_start] == ' ' || prefix[marker_start] == '\t'))
        marker_start++;
    marker_end = doc_md_list_marker_prefix(prefix, bytes, marker_start,
        !!(item->attrs->iFlags & XUI_DOC_TASK), &continuation);
    if (marker_end == marker_start) return XUI_ERROR_NOT_FOUND;
    result = doc_seq_read(old->source, range.syntax_end - 1, &ending, 1);
    if (result != XUI_OK) return result;
    if (ending == '\n' && range.syntax_end > 1) {
        char before;
        result = doc_seq_read(old->source, range.syntax_end - 2, &before, 1);
        if (result != XUI_OK) return result;
        if (before == '\r') ending_bytes = 2;
    } else if (ending != '\r') return XUI_ERROR_NOT_FOUND;
    *body_start = range.syntax_start + marker_end;
    *body_end = range.syntax_end - ending_bytes;
    if (*body_start > range.source_start || *body_end < range.source_end ||
        *body_start > *body_end) return XUI_ERROR_NOT_FOUND;
    return XUI_OK;
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
    uint64_t item_start, item_end;
    char line[256], ending[2];
    uint64_t bytes, header_end = 0, marker_start = 0, marker_end;
    uint64_t continuation, at, ending_bytes = 0;
    int result;
    if (doc_md_bounds(old, item->id, &item_start, &item_end) != XUI_OK ||
        copy_start > item_start || item_start > block_start ||
        block_end > item_end) return XUI_ERROR_NOT_FOUND;
    if (item_start < block_start)
        return doc_md_write_source_range(t, old, copy_start,
            block_start, moved);
    bytes = block_end - block_start;
    if (bytes > sizeof(line)) bytes = sizeof(line);
    result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    result = doc_seq_read(old->source, block_start, line, bytes);
    if (result != XUI_OK) return result;
    while (header_end < bytes && line[header_end] != '\n' &&
        line[header_end] != '\r') header_end++;
    while (marker_start < header_end &&
        (line[marker_start] == ' ' || line[marker_start] == '\t'))
        marker_start++;
    marker_end = doc_md_list_marker_prefix(line, header_end, marker_start,
        !!(item->attrs->iFlags & XUI_DOC_TASK), &continuation);
    if (marker_end == marker_start || marker_end >= header_end)
        return XUI_ERROR_NOT_FOUND;
    if (block->kind == XUI_DOC_CODE_BLOCK ||
        block->kind == XUI_DOC_DIAGRAM) {
        char fence = line[marker_end];
        if ((fence != '`' && fence != '~') || header_end - marker_end < 3 ||
            line[marker_end + 1] != fence ||
            line[marker_end + 2] != fence) return XUI_ERROR_NOT_FOUND;
    } else if (block->kind == XUI_DOC_QUOTE) {
        if (line[marker_end] != '>') return XUI_ERROR_NOT_FOUND;
    } else if (block->kind == XUI_DOC_HEADING) {
        if (line[marker_end] != '#') return XUI_ERROR_NOT_FOUND;
    } else if (block->kind == XUI_DOC_RULE) {
        if (line[marker_end] != '*' && line[marker_end] != '-' &&
            line[marker_end] != '_') return XUI_ERROR_NOT_FOUND;
    } else if (block->kind == XUI_DOC_HTML) {
        if (line[marker_end] != '<') return XUI_ERROR_NOT_FOUND;
    } else if (block->kind != XUI_DOC_TABLE)
        return XUI_ERROR_NOT_FOUND;
    at = block_start + marker_end;
    while (at < block_end) {
        char chunk[4096];
        uint64_t count = block_end - at < sizeof(chunk) ?
            block_end - at : sizeof(chunk), i;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(old->source, at, chunk, count);
        if (result != XUI_OK) return result;
        for (i = 0; i < count; i++) {
            char next;
            if (chunk[i] != '\n' && chunk[i] != '\r') continue;
            ending[0] = chunk[i]; ending_bytes = 1;
            if (chunk[i] == '\r' && at + i + 1 < block_end) {
                if (i + 1 < count) next = chunk[i + 1];
                else {
                    result = doc_seq_read(old->source, at + i + 1, &next, 1);
                    if (result != XUI_OK) return result;
                }
                if (next == '\n') ending[1] = '\n', ending_bytes = 2;
            }
            break;
        }
        if (ending_bytes) break;
        at += count;
    }
    if (!ending_bytes) return XUI_ERROR_NOT_FOUND;
    result = doc_md_write_source_range(t, old, copy_start,
        block_start + marker_end, moved);
    if (result != XUI_OK) return result;
    doc_md_write(moved, ending, ending_bytes);
    return moved->error ? moved->error : XUI_OK;
}
/* A bare quote tail needs the original list/quote opener after its only visible
 * paragraph moves out. Copy source-only list markers and whitespace along its
 * path, omitting selected parent blocks but keeping the item's marker and
 * line ending. The full candidate parse proves the semantics. */
static int doc_md_nested_empty_quote_scaffold(xui_document_transaction t,
    doc_state* old, doc_state* desired, const doc_range_branch_plan* plan,
    doc_node* last_block, doc_node* last, doc_md_output* moved,
    doc_md_output* prelude)
{
    doc_node *quote, *child, *item, *list, *remaining, *left_root;
    doc_node_source_range text_range;
    uint64_t block_start, block_end, quote_start, quote_end, length = 0, p = 0;
    uint64_t quote_count = 0, marker_count = 0, list_count = 0;
    uint64_t list_start = 0, list_end = 0, left_start, left_end, copy_start = 0;
    char prefix[256], ending[2];
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
        length = text_range.source_start - block_start;
        if (length >= sizeof(prefix)) return XUI_OK;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(old->source, block_start, prefix, length);
        if (result != XUI_OK) return result;
        while (p < length && p < 3 && prefix[p] == ' ') p++;
        if (p == length) return XUI_OK;
        if (prefix[p] == '-' || prefix[p] == '+' || prefix[p] == '*') p++;
        else {
            uint64_t digits = 0;
            while (p < length && prefix[p] >= '0' && prefix[p] <= '9' && digits < 9) {
                p++; digits++;
            }
            if (!digits || p == length || (prefix[p] != '.' && prefix[p] != ')'))
                return XUI_OK;
            p++;
        }
        if (p == length || (prefix[p] != ' ' && prefix[p] != '\t')) return XUI_OK;
        while (p < length && (prefix[p] == ' ' || prefix[p] == '\t')) p++;
        while (p < length && prefix[p] == '>') {
            p++; marker_count++;
            while (p < length && (prefix[p] == ' ' || prefix[p] == '\t')) p++;
        }
        if (marker_count != quote_count || p != length) return XUI_OK;
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
    if (list_count == 1) doc_md_write(moved, prefix, length);
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
/* Exact text endpoints can keep the spelling outside a replacement even when
 * it merges paragraphs. The same-parent path crosses sibling blocks separated
 * by blank container trivia or complete source-only definitions. The
 * cross-parent path uses the semantic branch plan; all source bytes outside
 * its text endpoints stay untouched. A private parse proves the full semantic
 * result before the patch reaches the transaction. */
static int doc_markdown_range_source_patch(xui_document_transaction t,
    const xui_doc_range_t* range, const char* text, uint64_t bytes, doc_state* desired)
{
    doc_state* old = t->draft;
    xui_doc_position_t a = range->tAnchor, b = range->tCaret;
    doc_node *first, *last, *first_block, *last_block, *parent;
    struct xui_doc_transaction_t candidate = {0};
    doc_md_output output = {0}, moved = {0}, prelude = {0};
    xui_doc_source_segment_t first_entity = {0}, last_entity = {0};
    uint64_t start, end, i, first_index, last_index, move_at = 0;
    uint64_t editable_footnote_start = DOC_NONE, editable_footnote_end = DOC_NONE;
    const char* line_ending = "\n";
    uint64_t line_ending_bytes = 1;
    char container_prefix[256];
    uint64_t container_prefix_bytes = 0, container_blank_bytes = 0;
    int order, left_quality, right_quality, cross_block, first_inside = 0,
        last_inside = 0, result = XUI_ERROR_NOT_FOUND;
    if (doc_position_compare(old, &a, &b, &order) != XUI_OK) return result;
    if (order > 0) { xui_doc_position_t swap = a; a = b; b = swap; }
    if (a.iKind != XUI_DOC_POSITION_TEXT || b.iKind != XUI_DOC_POSITION_TEXT ||
        a.iNodeId == b.iNodeId) return result;
    first = doc_index_get(old->index, a.iNodeId);
    last = doc_index_get(old->index, b.iNodeId);
    if (!first || !last || first->kind != XUI_DOC_TEXT || last->kind != XUI_DOC_TEXT)
        return result;
    first_block = doc_index_get(old->index, first->parent);
    last_block = doc_index_get(old->index, last->parent);
    if (!first_block || !last_block ||
        (first_block->kind != XUI_DOC_PARAGRAPH && first_block->kind != XUI_DOC_HEADING) ||
        (last_block->kind != XUI_DOC_PARAGRAPH && last_block->kind != XUI_DOC_HEADING))
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
    if (bytes && (memchr(text, '\n', (size_t)bytes) ||
        memchr(text, '\r', (size_t)bytes))) {
        uint64_t unused, block_end;
        uint64_t quote_depth = 0;
        doc_node* ancestor = first_block;
        doc_node* list_item = NULL;
        doc_node* footnote = NULL;
        char ending;
        result = doc_md_bounds(old, first_block->id, &unused, &block_end);
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
            if (!list_item && ancestor->kind == XUI_DOC_LIST_ITEM)
                list_item = ancestor;
            if (!footnote && ancestor->kind == XUI_DOC_FOOTNOTE)
                footnote = ancestor;
            ancestor = doc_index_get(old->index, ancestor->parent);
        }
        if (quote_depth || list_item || footnote) {
            /* Reuse parser-owned quote markers and the nearest list item's
             * content indent. Complex openers take the checked fallback. */
            doc_node_source_range block_range;
            char raw_prefix[256];
            uint64_t line_start, raw_bytes, at, markers = 0;
            doc_node_source_range_get(old, first_block, &block_range);
            if (block_range.source_start == DOC_NONE ||
                block_range.source_start > doc_seq_size(old->source)) {
                result = XUI_ERROR_NOT_FOUND; goto done;
            }
            line_start = block_range.source_start;
            while (line_start && block_range.source_start - line_start <
                sizeof(raw_prefix)) {
                char previous;
                result = doc_seq_read(old->source, line_start - 1,
                    &previous, 1);
                if (result != XUI_OK) goto done;
                if (previous == '\r' || previous == '\n') break;
                line_start--;
            }
            raw_bytes = block_range.source_start - line_start;
            if (!raw_bytes || raw_bytes >= sizeof(raw_prefix)) {
                result = XUI_ERROR_NOT_FOUND; goto done;
            }
            result = doc_seq_read(old->source, line_start, raw_prefix,
                raw_bytes);
            if (result != XUI_OK) goto done;
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
                if (at + marker_width >= sizeof(container_prefix) ||
                    at + marker_width + raw_bytes - marker_end >=
                        sizeof(container_prefix)) {
                    result = XUI_ERROR_NOT_FOUND; goto done;
                }
                memcpy(container_prefix, raw_prefix, (size_t)at);
                memset(container_prefix + at, ' ', (size_t)marker_width);
                memcpy(container_prefix + at + marker_width,
                    raw_prefix + marker_end,
                    (size_t)(raw_bytes - marker_end));
                container_prefix_bytes = at + marker_width + raw_bytes - marker_end;
            } else if (footnote && !list_item && at + 2 < raw_bytes &&
                raw_prefix[at] == '[' && raw_prefix[at + 1] == '^') {
                uint64_t close = at + 2, j;
                while (close + 1 < raw_bytes &&
                    (raw_prefix[close] != ']' || raw_prefix[close + 1] != ':'))
                    close++;
                if (close + 2 >= raw_bytes || at + 4 >= sizeof(container_prefix)) {
                    result = XUI_ERROR_NOT_FOUND; goto done;
                }
                for (j = close + 2; j < raw_bytes; j++)
                    if (raw_prefix[j] != ' ' && raw_prefix[j] != '\t') {
                        result = XUI_ERROR_NOT_FOUND; goto done;
                    }
                memcpy(container_prefix, raw_prefix, (size_t)at);
                memset(container_prefix + at, ' ', 4);
                container_prefix_bytes = at + 4;
            } else {
                memcpy(container_prefix, raw_prefix, (size_t)raw_bytes);
                container_prefix_bytes = raw_bytes;
            }
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
            doc_md_write(&output, line_ending, line_ending_bytes);
            if (container_blank_bytes)
                doc_md_write(&output, container_prefix, container_blank_bytes);
            doc_md_write(&output, line_ending, line_ending_bytes);
            if (container_prefix_bytes)
                doc_md_write(&output, container_prefix, container_prefix_bytes);
            i += width;
            if (scalar == '\r' && i < bytes && text[i] == '\n') i++;
            continue;
        }
        if ((i == 0 || i + width == bytes || text[i - 1] == '\n' ||
                text[i - 1] == '\r' || text[i + width] == '\n' ||
                text[i + width] == '\r') &&
            doc_md4c_unicode_whitespace(scalar))
            doc_md_write_entity(&output, text + i, width);
        else doc_md_escape(&output, text + i, width);
        i += width;
    }
    if (last_inside) {
        result = doc_md_write_entity_slice(&output, last,
            b.iOffset, last_entity.iTextEnd);
        if (result != XUI_OK) goto done;
    }
    if (output.error) { result = output.error; goto done; }
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK && moved.size)
        result = doc_txn_source_patch(&candidate, move_at, move_at,
            moved.data, moved.size, 0);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, start, end,
        output.data ? output.data : "", output.size, 0);
    if (result == XUI_OK) result = doc_markdown_parse(&candidate);
    if (result == XUI_OK && !doc_semantic_equal(desired, candidate.draft))
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
    doc_free(output.data); doc_free(moved.data); doc_free(prelude.data);
    return result;
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
int doc_markdown_range(xui_document_transaction t, const xui_doc_range_t* range, const char* text, uint64_t bytes, xui_doc_position_t* caret)
{
    struct xui_doc_transaction_t expected = {0}; xui_doc_position_t target = {0}; int result;
    result = doc_markdown_shadow_begin(t, &expected); if (result != XUI_OK) return result;
    result = xuiDocumentTxnReplaceRange(&expected, range, text, bytes, &target);
    if (result == XUI_OK && expected.count) {
        result = doc_markdown_cell_gap_source_patch(t, range, text, bytes,
            expected.draft);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_range_source_patch(t, range, text, bytes, expected.draft);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_apply_tree(t, expected.draft, NULL, NULL);
        if (result == XUI_OK) result = doc_markdown_accept(t, &expected);
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
    for (i = 0; language[i]; i++) if ((unsigned char)language[i] <= ' ' || language[i] == '~' || language[i] == '`') return result;
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
        if (language[i] == '&') doc_md_write(&output, "&amp;", 5);
        else if (language[i] == '\\') doc_md_write(&output, "&#92;", 5);
        else doc_md_write(&output, language + i, 1);
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
int doc_markdown_shadow_end(xui_document_transaction t, struct xui_doc_transaction_t* shadow, int result,
    const xui_doc_position_t* target, xui_doc_position_t* caret)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_markdown_code_language_source_patch(t, shadow);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL);
    }
    if (result == XUI_OK && shadow->count) result = doc_markdown_accept(t, shadow);
    if (result == XUI_OK && target && caret) *caret = doc_markdown_resolve_caret(t, shadow->draft, *target);
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
int doc_markdown_shadow_end_list_split(xui_document_transaction t, struct xui_doc_transaction_t* shadow,
    int result, const xui_doc_position_t* at, const xui_doc_position_t* target, xui_doc_position_t* caret)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_md_list_split_source_patch(t, shadow->draft, at, target);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL);
        if (result == XUI_OK) result = doc_markdown_accept(t, shadow);
    }
    if (result == XUI_OK && target && caret) *caret = doc_markdown_resolve_caret(t, shadow->draft, *target);
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
/* Keep the untouched spelling of a root GFM table when a plain data row is
 * inserted or removed. Structural cases such as header changes and nested
 * tables still use the general serializer. */
static int doc_markdown_table_row_source_patch(xui_document_transaction t,
    doc_state* desired, uint64_t table_id, uint32_t index, int insert)
{
    doc_state* old = t->draft;
    doc_node* table = doc_index_get(old->index, table_id);
    doc_node* row;
    doc_node_source_range table_range, row_range;
    struct xui_doc_transaction_t candidate = {0};
    char line[1024 * 3 + 5], ending[2], prior_ending[2], previous;
    uint64_t rows, at, source_bytes, row_end, i;
    uint32_t columns, column;
    size_t line_bytes = 0, ending_bytes = 0, prior_bytes = 1;
    int result = XUI_ERROR_NOT_FOUND;
    if (!table || table->kind != XUI_DOC_TABLE || table->parent != DOC_ROOT)
        return result;
    rows = doc_seq_size(table->children);
    if (index == 0 || index > rows || (!insert && index == rows) ||
        (insert && index == rows && rows < 2)) return result;
    columns = doc_table_column_count(old, table);
    if (!columns || columns > 1024) return result;
    row = doc_index_get(old->index, doc_seq_get_id(table->children,
        insert && index == rows ? rows - 1 : index));
    if (!row || row->kind != XUI_DOC_ROW) return result;
    doc_node_source_range_get(old, table, &table_range);
    doc_node_source_range_get(old, row, &row_range);
    source_bytes = doc_seq_size(old->source);
    if (table_range.syntax_start == DOC_NONE ||
        table_range.syntax_end == DOC_NONE ||
        row_range.syntax_start == DOC_NONE ||
        row_range.syntax_end == DOC_NONE ||
        table_range.syntax_start > row_range.syntax_start ||
        row_range.syntax_start >= row_range.syntax_end ||
        row_range.syntax_end > table_range.syntax_end ||
        table_range.syntax_end > source_bytes || !row_range.syntax_start)
        return result;
    result = doc_seq_read(old->source, row_range.syntax_start - 1,
        &previous, 1);
    if (result != XUI_OK) return result;
    if (previous != '\r' && previous != '\n') return XUI_ERROR_NOT_FOUND;
    prior_ending[0] = previous;
    if (previous == '\n' && row_range.syntax_start >= 2) {
        result = doc_seq_read(old->source, row_range.syntax_start - 2,
            &prior_ending[1], 1);
        if (result != XUI_OK) return result;
        if (prior_ending[1] == '\r') {
            prior_ending[0] = '\r'; prior_ending[1] = '\n'; prior_bytes = 2;
        }
    }
    if (row_range.syntax_end < table_range.syntax_end) {
        result = doc_seq_read(old->source, row_range.syntax_end,
            ending, 1);
        if (result != XUI_OK) return result;
        if (ending[0] != '\r' && ending[0] != '\n') return XUI_ERROR_NOT_FOUND;
        ending_bytes = 1;
        if (ending[0] == '\r' && row_range.syntax_end + 1 < table_range.syntax_end) {
            result = doc_seq_read(old->source, row_range.syntax_end + 1,
                &ending[1], 1);
            if (result != XUI_OK) return result;
            if (ending[1] == '\n') ending_bytes = 2;
        }
    }
    row_end = row_range.syntax_end + ending_bytes;
    if (row_end > table_range.syntax_end) return XUI_ERROR_NOT_FOUND;
    if (insert) {
        if (index == rows && !ending_bytes) {
            memcpy(line + line_bytes, prior_ending, prior_bytes);
            line_bytes += prior_bytes;
        }
        for (column = 0; column < columns; column++) {
            memcpy(line + line_bytes, "|  ", 3); line_bytes += 3;
        }
        line[line_bytes++] = '|';
        if (index < rows || ending_bytes) {
            const char* suffix = ending_bytes ? ending : prior_ending;
            size_t suffix_bytes = ending_bytes ? ending_bytes : prior_bytes;
            memcpy(line + line_bytes, suffix, suffix_bytes);
            line_bytes += suffix_bytes;
        }
    }
    at = insert && index == rows ? row_end : row_range.syntax_start;
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, at,
        insert ? at : row_end, insert ? line : "",
        insert ? line_bytes : 0, 0);
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
    doc_state_release(candidate.draft); doc_free(candidate.ops);
    return result;
}
int doc_markdown_shadow_end_table_row(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow, int result, uint64_t table,
    uint32_t index, int insert)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_markdown_table_row_source_patch(t, shadow->draft,
            table, index, insert);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL);
        if (result == XUI_OK) result = doc_markdown_accept(t, shadow);
    }
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
/* Each fully delimited GFM row has one real pipe on either side of every
 * Cell. Rewrite only the selected pipe interval in every source row, including
 * the alignment row; all other cell spelling stays in the SourceStore. */
static int doc_markdown_table_column_source_patch(xui_document_transaction t,
    doc_state* desired, uint64_t table_id, uint32_t column, int insert)
{
    doc_state* old = t->draft;
    doc_node* table = doc_index_get(old->index, table_id);
    doc_node_source_range table_range, row_range;
    struct xui_doc_transaction_t candidate = {0};
    uint64_t pipes[1025], token_count, token_index, rows, source_row, i;
    uint32_t columns;
    int result = XUI_ERROR_NOT_FOUND;
    if (!table || table->kind != XUI_DOC_TABLE || table->parent != DOC_ROOT ||
        !table->syntax_aux ||
        table->syntax_aux->size % sizeof(doc_table_token_relative)) return result;
    columns = doc_table_column_count(old, table);
    rows = doc_seq_size(table->children);
    if (!columns || columns > 1024 || !rows || rows >= UINT32_MAX ||
        column > columns || (!insert && (column >= columns || columns == 1)))
        return result;
    doc_node_source_range_get(old, table, &table_range);
    if (table_range.syntax_start == DOC_NONE ||
        table_range.syntax_end == DOC_NONE ||
        table_range.syntax_end <= table_range.syntax_start ||
        table_range.syntax_end > doc_seq_size(old->source)) return result;
    token_count = table->syntax_aux->size / sizeof(doc_table_token_relative);
    token_index = token_count;
    result = doc_markdown_shadow_begin(t, &candidate);
    if (result != XUI_OK) return result;
    for (source_row = rows; ; source_row--) {
        uint32_t expected = columns + 1;
        uint64_t start, end;
        char edge;
        const char* added = source_row == 1 ? " --- |" : "  |";
        size_t added_bytes = source_row == 1 ? 6 : 3;
        result = doc_txn_check(t, 0);
        if (result != XUI_OK) break;
        while (token_index) {
            doc_table_token_relative token;
            memcpy(&token, table->syntax_aux->data +
                (token_index - 1) * sizeof(token), sizeof(token));
            if (token.row < source_row) break;
            if (token.row != source_row ||
                token.end > table_range.syntax_end - table_range.syntax_start) {
                result = XUI_ERROR_NOT_FOUND; break;
            }
            token_index--;
            if (token.kind == XUI_DOC_TABLE_TOKEN_PIPE) {
                if (!expected || token.ordinal != expected - 1 ||
                    token.end != token.start + 1) {
                    result = XUI_ERROR_NOT_FOUND; break;
                }
                pipes[--expected] = table_range.syntax_start + token.start;
            } else if (token.kind != XUI_DOC_TABLE_TOKEN_UNDERLINE || source_row != 1) {
                result = XUI_ERROR_NOT_FOUND; break;
            }
        }
        if (result != XUI_OK) break;
        if (expected || pipes[0] >= pipes[columns] ||
            pipes[columns] >= table_range.syntax_end) {
            result = XUI_ERROR_NOT_FOUND; break;
        }
        if (source_row == 1) {
            if (pipes[0] == table_range.syntax_start) {
                result = XUI_ERROR_NOT_FOUND; break;
            }
            result = doc_seq_read(old->source, pipes[0] - 1, &edge, 1);
            if (result != XUI_OK) break;
            if (edge != '\r' && edge != '\n') {
                result = XUI_ERROR_NOT_FOUND; break;
            }
            if (pipes[columns] + 1 < table_range.syntax_end) {
                result = doc_seq_read(old->source, pipes[columns] + 1, &edge, 1);
                if (result != XUI_OK) break;
                if (edge != '\r' && edge != '\n') {
                    result = XUI_ERROR_NOT_FOUND; break;
                }
            }
        } else {
            doc_node* row = doc_index_get(old->index, doc_seq_get_id(table->children,
                source_row ? source_row - 1 : 0));
            if (!row || row->kind != XUI_DOC_ROW ||
                doc_seq_size(row->children) != columns) {
                result = XUI_ERROR_NOT_FOUND; break;
            }
            doc_node_source_range_get(old, row, &row_range);
            if (row_range.syntax_start != pipes[0] ||
                row_range.syntax_end != pipes[columns] + 1) {
                result = XUI_ERROR_NOT_FOUND; break;
            }
        }
        if (insert) start = end = pipes[column] + 1;
        else if (column == columns - 1) {
            start = pipes[column]; end = pipes[column + 1];
        } else {
            start = pipes[column] + 1; end = pipes[column + 1] + 1;
        }
        result = doc_txn_source_patch(&candidate, start, end,
            insert ? added : "", insert ? added_bytes : 0, 0);
        if (result != XUI_OK || source_row == 0) break;
    }
    if (result == XUI_OK && token_index) result = XUI_ERROR_NOT_FOUND;
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
    doc_state_release(candidate.draft); doc_free(candidate.ops);
    return result;
}
int doc_markdown_shadow_end_table_column(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow, int result, uint64_t table,
    uint32_t column, int insert)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_markdown_table_column_source_patch(t, shadow->draft,
            table, column, insert);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL);
        if (result == XUI_OK) result = doc_markdown_accept(t, shadow);
    }
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
int doc_markdown_shadow_end_image(xui_document_transaction t, struct xui_doc_transaction_t* shadow,
    int result, uint64_t image_id)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_markdown_image_patch(t, image_id, shadow->draft);
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL);
        if (result == XUI_OK) result = doc_markdown_accept(t, shadow);
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
        if (result == XUI_ERROR_NOT_FOUND) result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL);
        if (result == XUI_OK) result = doc_markdown_accept(t, shadow);
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
 * link-definition values, including unused duplicates and multiline titles. */
static int doc_markdown_quote_range_source_patch(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow, uint64_t quote_id)
{
    struct xui_doc_transaction_t candidate = {0}; doc_md_output output = {0};
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
        /* Used footnotes have visible bodies; unused bodies need a dedicated
         * provenance comparison before moving their container prefixes. */
        if (ref.iSourceStart < end && ref.iSourceEnd > start &&
            (ref.iSourceStart < start || ref.iSourceEnd > end ||
                ref.iKind != XUI_DOC_REFERENCE_LINK)) return XUI_ERROR_NOT_FOUND;
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
        doc_md_write(&output, "> ", 2);
        doc_md_write(&output, source + at, next - at);
        at = next;
    }
    if (end < source_size) {
        if (source[bytes - 1] != '\r' && source[bytes - 1] != '\n')
            doc_md_write(&output, ending, ending_bytes);
        doc_md_write(&output, ending, ending_bytes);
    }
    result = output.error ? output.error : doc_markdown_shadow_begin(t, &candidate);
    if (result == XUI_OK) result = doc_txn_source_patch(&candidate, start, end, output.data, output.size, 0);
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
    doc_free(source); doc_free(output.data); return result;
}
int doc_markdown_shadow_end_quote_range(xui_document_transaction t,
    struct xui_doc_transaction_t* shadow, int result, uint64_t quote,
    const xui_doc_range_t* target, xui_doc_range_t* after)
{
    if (result == XUI_OK && shadow->count) {
        result = doc_markdown_quote_range_source_patch(t, shadow, quote);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL);
    }
    if (result == XUI_OK && shadow->count) result = doc_markdown_accept(t, shadow);
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
        result = doc_markdown_heading_marker_patch(t, shadow);
        if (result == XUI_ERROR_NOT_FOUND)
            result = doc_markdown_apply_tree(t, shadow->draft, NULL, NULL);
    }
    if (result == XUI_OK && shadow->count) result = doc_markdown_accept(t, shadow);
    if (result == XUI_OK && target && after) {
        after->tAnchor = doc_markdown_resolve_caret(t, shadow->draft, target->tAnchor);
        after->tCaret = doc_markdown_resolve_caret(t, shadow->draft, target->tCaret);
    }
    doc_state_release(shadow->draft); doc_free(shadow->ops);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}

#endif
