#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"

/* A local grammar boundary is one top-level block delimited by unchanged
 * blank lines or document edges. Safe standalone link-definition value edits
 * can invalidate several independent blocks. Unused footnote-definition body
 * edits can update only source metadata; edits of footnote uses or rendered
 * transitive footnote dependencies and edits spanning blocks still use the
 * full parser.
 * Context-sensitive blocks also reparse
 * their following sibling to prove the edit did not consume it. */
static int doc_inc_kind(uint32_t kind)
{
    return kind == XUI_DOC_PARAGRAPH || kind == XUI_DOC_HEADING || kind == XUI_DOC_RULE ||
        kind == XUI_DOC_CODE_BLOCK || kind == XUI_DOC_DIAGRAM || kind == XUI_DOC_HTML ||
        kind == XUI_DOC_QUOTE || kind == XUI_DOC_LIST || kind == XUI_DOC_TABLE;
}
static int doc_inc_context_kind(uint32_t kind)
{
    return kind == XUI_DOC_CODE_BLOCK || kind == XUI_DOC_DIAGRAM || kind == XUI_DOC_HTML ||
        kind == XUI_DOC_QUOTE || kind == XUI_DOC_LIST || kind == XUI_DOC_TABLE;
}
static int doc_inc_definition_container_kind(uint32_t kind)
{
    return kind == XUI_DOC_QUOTE || kind == XUI_DOC_LIST || kind == XUI_DOC_TABLE;
}
static int doc_inc_cancel(xui_document_transaction t)
{
    return t->cancellation ? atomic_load(t->cancellation) : XUI_OK;
}
static int doc_inc_syntax_first(xui_document_transaction, doc_sequence*, uint64_t, uint64_t*);
/* Top-level Markdown block ranges are in source order. Locate the first end
 * that can contain the edit, preserving the previous linear scan's choice at
 * an inclusive boundary shared by adjacent blocks. */
static int doc_inc_find_block(xui_document_transaction t, doc_state* s, uint64_t offset,
    uint64_t* index, doc_node** block)
{
    doc_node* root = doc_index_get(s->index, DOC_ROOT);
    uint64_t count = doc_seq_size(root->children), lo = 0, hi;
    int result;
    *index = count; *block = NULL;
    /* MD4C emits referenced footnotes after ordinary blocks. Their order is
     * by first use, so their source ends cannot participate in this binary
     * search over the ordinary source-ordered prefix. */
    while (count) {
        doc_node* last = doc_index_get(s->index, doc_seq_get_id(root->children, count - 1));
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        if (!last) return XUI_ERROR_UNSUPPORTED;
        if (last->kind != XUI_DOC_FOOTNOTE) break;
        count--;
    }
    hi = count;
    while (lo < hi) {
        uint64_t mid = lo + (hi - lo) / 2;
        doc_node* n = doc_index_get(s->index, doc_seq_get_id(root->children, mid));
        doc_node_source_range range;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        if (!n) return XUI_ERROR_UNSUPPORTED;
        doc_node_source_range_get(s, n, &range);
        if (range.syntax_end == DOC_NONE) return XUI_ERROR_UNSUPPORTED;
        if (range.syntax_end < offset) lo = mid + 1;
        else hi = mid;
    }
    for (; lo < count; lo++) {
        doc_node* n = doc_index_get(s->index, doc_seq_get_id(root->children, lo));
        doc_node_source_range range;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        if (!n) return XUI_ERROR_UNSUPPORTED;
        doc_node_source_range_get(s, n, &range);
        if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE || range.syntax_start > offset)
            return XUI_ERROR_UNSUPPORTED;
        if (doc_inc_kind(n->kind) && offset <= range.syntax_end) {
            *index = lo; *block = n; return XUI_OK;
        }
    }
    return XUI_ERROR_UNSUPPORTED;
}
static unsigned char doc_inc_byte(doc_sequence* source, uint64_t at)
{
    unsigned char c = 0; (void)doc_seq_read(source, at, &c, 1); return c;
}
static uint64_t doc_inc_previous_line(doc_sequence* source, uint64_t at)
{
    unsigned char c;
    if (!at) return 0;
    c = doc_inc_byte(source, at - 1);
    if (c == '\n') { at--; if (at && doc_inc_byte(source, at - 1) == '\r') at--; }
    else if (c == '\r') at--;
    else return DOC_NONE;
    return at;
}
static int doc_inc_boundaries(doc_sequence* source, uint64_t start, uint64_t end, int consumed_blank)
{
    uint64_t at, size = doc_seq_size(source); unsigned char c;
    if (start) {
        at = doc_inc_previous_line(source, start);
        if (at == DOC_NONE || !at) return 0;
        /* The immediately preceding physical line must be blank. Conservatively
         * reject whitespace-only lines; they can carry indentation context. */
        c = doc_inc_byte(source, at - 1);
        if (c != '\r' && c != '\n') return 0;
    }
    if (end == size) return 1;
    if (!end || doc_inc_previous_line(source, end) == DOC_NONE) return 0;
    c = doc_inc_byte(source, end);
    /* A CR at the window edge can consume the following LF. That LF then
     * belongs to this line ending rather than the separating blank line. */
    if (c == '\n' && doc_inc_byte(source, end - 1) == '\r') return 0;
    if (c == '\n') return 1;
    if (c == '\r') return 1;
    /* Lists (and some other container blocks) may include the separating
     * blank physical line in their syntax range. Then end already points at
     * the next block, so verify the line just consumed really was blank. */
    if (consumed_blank) {
        at = doc_inc_previous_line(source, end);
        if (at != DOC_NONE && at) {
            c = doc_inc_byte(source, at - 1);
            if (c == '\r' || c == '\n') return 1;
        }
    }
    return 0;
}
static int doc_inc_range_equal(uint64_t old, uint64_t parsed, uint64_t removed, uint64_t added)
{
    return old == DOC_NONE ? parsed == DOC_NONE : old >= removed && old - removed + added == parsed;
}
static int doc_inc_subtree_source_equal(doc_state* old, uint64_t old_id, doc_state* parsed,
    uint64_t parsed_id, uint64_t removed, uint64_t added)
{
    doc_node *a = doc_index_get(old->index, old_id), *b = doc_index_get(parsed->index, parsed_id);
    doc_node_source_range ar, br;
    uint64_t i, children, segments;
    if (!a || !b || a->source_exact != b->source_exact ||
        doc_seq_size(a->children) != doc_seq_size(b->children) ||
        doc_seq_size(a->provenance) != doc_seq_size(b->provenance)) return 0;
    doc_node_source_range_get(old, a, &ar); doc_node_source_range_get(parsed, b, &br);
    if (!doc_inc_range_equal(ar.source_start, br.source_start, removed, added) ||
        !doc_inc_range_equal(ar.source_end, br.source_end, removed, added) ||
        !doc_inc_range_equal(ar.syntax_start, br.syntax_start, removed, added) ||
        !doc_inc_range_equal(ar.syntax_end, br.syntax_end, removed, added)) return 0;
    segments = doc_seq_size(a->provenance) / sizeof(xui_doc_source_segment_t);
    for (i = 0; i < segments; i++) {
        xui_doc_source_segment_t x, y;
        if (doc_seq_read(a->provenance, i * sizeof(x), &x, sizeof(x)) != XUI_OK ||
            doc_seq_read(b->provenance, i * sizeof(y), &y, sizeof(y)) != XUI_OK) return 0;
        doc_source_resolve_segment(old, a, &x); doc_source_resolve_segment(parsed, b, &y);
        if (!doc_inc_range_equal(x.iSourceStart, y.iSourceStart, removed, added) ||
            !doc_inc_range_equal(x.iSourceEnd, y.iSourceEnd, removed, added)) return 0;
        x.iSourceStart = y.iSourceStart; x.iSourceEnd = y.iSourceEnd;
        if (memcmp(&x, &y, sizeof(x))) return 0;
    }
    children = doc_seq_size(a->children);
    for (i = 0; i < children; i++)
        if (!doc_inc_subtree_source_equal(old, doc_seq_get_id(a->children, i), parsed,
            doc_seq_get_id(b->children, i), removed, added)) return 0;
    return 1;
}
/* A definition-only edit leaves block grammar unchanged. Prove that a
 * standalone container still has the same block skeleton and syntax bounds.
 * Inline link changes may alter the source envelope of its ancestors. */
static int doc_inc_definition_shape_equal(doc_state* old, uint64_t old_id,
    doc_state* parsed, uint64_t parsed_id, uint64_t start)
{
    doc_node *a = doc_index_get(old->index, old_id), *b = doc_index_get(parsed->index, parsed_id);
    doc_node_source_range ar, br;
    uint64_t i, count;
    if (!a || !b || a->kind != b->kind || a->source_exact != b->source_exact ||
        !doc_attributes_equal(a->attrs, b->attrs) ||
        strcmp(doc_string(a->resource), doc_string(b->resource)) ||
        strcmp(doc_string(a->info), doc_string(b->info)) ||
        strcmp(doc_string(a->title), doc_string(b->title))) return 0;
    doc_node_source_range_get(old, a, &ar); doc_node_source_range_get(parsed, b, &br);
    if (!doc_inc_range_equal(ar.syntax_start, br.syntax_start, start, 0) ||
        !doc_inc_range_equal(ar.syntax_end, br.syntax_end, start, 0)) return 0;
    if (a->kind != XUI_DOC_QUOTE && a->kind != XUI_DOC_LIST &&
        a->kind != XUI_DOC_LIST_ITEM && a->kind != XUI_DOC_TABLE &&
        a->kind != XUI_DOC_ROW && a->kind != XUI_DOC_CELL) return 1;
    count = doc_seq_size(a->children);
    if (count != doc_seq_size(b->children)) return 0;
    for (i = 0; i < count; i++)
        if (!doc_inc_definition_shape_equal(old, doc_seq_get_id(a->children, i),
            parsed, doc_seq_get_id(b->children, i), start)) return 0;
    return 1;
}
/* A fence or raw HTML block can be valid at a fragment EOF but consume later
 * siblings in the actual document. Parse the next complete sibling too; if
 * either block changes, the ordinary full parse owns the edit. */
static int doc_inc_right_context(xui_document_transaction t, doc_state* old,
    uint64_t index, uint64_t start, uint64_t old_end, uint64_t end,
    doc_state* fragment, uint64_t fresh_id)
{
    doc_node* root = doc_index_get(old->index, DOC_ROOT);
    doc_node *next, *preview_root;
    doc_node_source_range range;
    doc_state* preview = NULL;
    uint64_t next_start, next_end, first_id, second_id;
    int result;
    if (index + 1 >= doc_seq_size(root->children))
        return end == doc_seq_size(old->source) ? XUI_OK : XUI_ERROR_UNSUPPORTED;
    next = doc_index_get(old->index, doc_seq_get_id(root->children, index + 1));
    if (!next) return XUI_ERROR_UNSUPPORTED;
    doc_node_source_range_get(old, next, &range);
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
        range.syntax_start < old_end || range.syntax_end < range.syntax_start) return XUI_ERROR_UNSUPPORTED;
    next_start = range.syntax_start - old_end + end;
    next_end = range.syntax_end - old_end + end;
    if (next_start < end || next_end > doc_seq_size(old->source) ||
        next_end - start > 65536) return XUI_ERROR_UNSUPPORTED;
    result = doc_markdown_parse_window(t, start, next_end, &preview);
    if (result != XUI_OK) return result;
    preview_root = doc_index_get(preview->index, DOC_ROOT);
    result = XUI_ERROR_UNSUPPORTED;
    if (doc_seq_size(preview_root->children) != 2 || preview->references || preview->markdown_footnotes) goto done;
    first_id = doc_seq_get_id(preview_root->children, 0);
    second_id = doc_seq_get_id(preview_root->children, 1);
    {
        doc_node *first = doc_index_get(preview->index, first_id), *second = doc_index_get(preview->index, second_id);
        if (first->syntax_start != 0 || first->syntax_end != end - start ||
            second->syntax_start != next_start - start || second->syntax_end != next_end - start ||
            !doc_semantic_subtree_equal(fragment, fresh_id, preview, first_id) ||
            !doc_semantic_subtree_equal(old, next->id, preview, second_id) ||
            !doc_inc_subtree_source_equal(fragment, fresh_id, preview, first_id, 0, 0) ||
            !doc_inc_subtree_source_equal(old, next->id, preview, second_id, old_end, end - start)) goto done;
    }
    result = XUI_OK;
done:
    doc_state_release(preview); return result;
}
static int doc_inc_left_context(xui_document_transaction t, doc_state* old,
    uint64_t index, uint64_t start, uint64_t end, doc_state* fragment, uint64_t fresh_id)
{
    doc_node* root = doc_index_get(old->index, DOC_ROOT);
    doc_node *previous, *preview_root;
    doc_node_source_range range;
    doc_state* preview = NULL;
    uint64_t previous_start, first_id, second_id;
    int result;
    if (!index) return XUI_OK;
    previous = doc_index_get(old->index, doc_seq_get_id(root->children, index - 1));
    if (!previous) return XUI_ERROR_UNSUPPORTED;
    doc_node_source_range_get(old, previous, &range);
    previous_start = range.syntax_start;
    if (previous_start == DOC_NONE || range.syntax_end == DOC_NONE ||
        range.syntax_end > start || end - previous_start > 65536) return XUI_ERROR_UNSUPPORTED;
    result = doc_markdown_parse_window(t, previous_start, end, &preview);
    if (result != XUI_OK) return result;
    preview_root = doc_index_get(preview->index, DOC_ROOT);
    result = XUI_ERROR_UNSUPPORTED;
    if (doc_seq_size(preview_root->children) != 2 || preview->references || preview->markdown_footnotes) goto done;
    first_id = doc_seq_get_id(preview_root->children, 0);
    second_id = doc_seq_get_id(preview_root->children, 1);
    {
        doc_node *first = doc_index_get(preview->index, first_id), *second = doc_index_get(preview->index, second_id);
        if (first->syntax_start != 0 || first->syntax_end != range.syntax_end - previous_start ||
            second->syntax_start != start - previous_start || second->syntax_end != end - previous_start ||
            !doc_semantic_subtree_equal(old, previous->id, preview, first_id) ||
            !doc_semantic_subtree_equal(fragment, fresh_id, preview, second_id) ||
            !doc_inc_subtree_source_equal(old, previous->id, preview, first_id, previous_start, 0) ||
            !doc_inc_subtree_source_equal(fragment, fresh_id, preview, second_id, 0, start - previous_start)) goto done;
    }
    result = XUI_OK;
done:
    doc_state_release(preview); return result;
}
static uint64_t doc_inc_offset(uint64_t value, uint64_t removed, uint64_t added)
{
    return value == DOC_NONE ? DOC_NONE : value - removed + added;
}
/* Unchanged definitions must lie wholly outside the edited block. Without a
 * lookup the old table can be reused; link lookups parse against copied
 * definitions. Later definitions only need their source coordinates rebased. */
static int doc_inc_definition_guard(xui_document_transaction t, doc_state* old,
    uint64_t start, uint64_t old_end, uint64_t end, int* needs_definitions,
    uint64_t* brackets)
{
    uint64_t at, size = doc_seq_size(old->references);
    unsigned char bytes[512];
    unsigned char previous = 0;
    int result;
    *needs_definitions = 0; *brackets = 0;
    if (!size && !old->markdown_footnotes) return XUI_OK;
    if (size % sizeof(xui_doc_reference_definition_t)) return XUI_ERROR_UNSUPPORTED;
    for (at = start; at < end; ) {
        uint64_t count = end - at < sizeof(bytes) ? end - at : sizeof(bytes), i;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read(old->source, at, bytes, count);
        if (result != XUI_OK) return result;
        for (i = 0; i < count; i++) {
            if (old->markdown_footnotes && previous == '[' && bytes[i] == '^')
                return XUI_ERROR_UNSUPPORTED;
            if (bytes[i] == '[') { *needs_definitions = 1; (*brackets)++; }
            previous = bytes[i];
        }
        at += count;
    }
    for (at = 0; at < size; at += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read(old->references, at, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if (ref.iSourceStart == DOC_NONE || ref.iSourceEnd == DOC_NONE ||
            ref.iSourceStart > ref.iSourceEnd) return XUI_ERROR_UNSUPPORTED;
        if (ref.iSourceStart < old_end && ref.iSourceEnd > start) return XUI_ERROR_UNSUPPORTED;
    }
    return XUI_OK;
}
static int doc_inc_footnote_dependency_guard(xui_document_transaction t,
    doc_state* old, uint64_t start, uint64_t old_end)
{
    uint64_t at, count = doc_seq_size(old->inline_syntax) / sizeof(xui_doc_inline_syntax_t);
    int result;
    if (!old->markdown_footnotes) return XUI_OK;
    if (doc_seq_size(old->inline_syntax) % sizeof(xui_doc_inline_syntax_t))
        return XUI_ERROR_UNSUPPORTED;
    /* Footnote bodies are emitted after ordinary blocks and in reference
     * order. If their definitions are earlier or reordered, inline syntax
     * can cease to be source ordered. The local splice requires that order. */
    if (!old->source_blocks_indexed) {
        uint64_t previous = 0;
        for (at = 0; at < count; at++) {
            xui_doc_inline_syntax_t syntax;
            if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
            result = doc_seq_read_syntax(old->inline_syntax, at, &syntax);
            if (result != XUI_OK) return result;
            if (syntax.iSourceStart < previous) return XUI_ERROR_UNSUPPORTED;
            previous = syntax.iSourceStart;
            if (syntax.iSourceStart >= start && syntax.iSourceStart < old_end &&
                syntax.iKind == XUI_DOC_SYNTAX_FOOTNOTE_REF)
                return XUI_ERROR_UNSUPPORTED;
        }
        return XUI_OK;
    }
    result = doc_inc_syntax_first(t, old->inline_syntax, start, &at);
    if (result != XUI_OK) return result;
    for (; at < count; at++) {
        xui_doc_inline_syntax_t syntax;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read_syntax(old->inline_syntax, at, &syntax);
        if (result != XUI_OK) return result;
        if (syntax.iSourceStart >= old_end) break;
        if (syntax.iKind == XUI_DOC_SYNTAX_FOOTNOTE_REF) return XUI_ERROR_UNSUPPORTED;
    }
    return XUI_OK;
}
/* MD4C caps total reference expansion relative to its input length. Earlier
 * blocks can exhaust the full document's cap before this window is parsed,
 * so bound every possible full-document lookup as well as local lookups. */
static int doc_inc_link_budget_safe(xui_document_transaction t, doc_state* old,
    uint64_t fragment_bytes, uint64_t suffix_bytes, uint64_t brackets,
    int check_global)
{
    uint64_t full_bytes = doc_seq_size(old->source), local_bytes = fragment_bytes + suffix_bytes;
    uint64_t size = doc_seq_size(old->references), at, maximum = 0;
    doc_node* root = doc_index_get(old->index, DOC_ROOT);
    uint64_t full_budget, local_budget;
    int result;
    if (root && doc_seq_size(root->children)) {
        doc_node* first = doc_index_get(old->index, doc_seq_get_id(root->children, 0));
        doc_node_source_range range;
        if (first && first->kind == XUI_DOC_FRONT_MATTER) {
            doc_node_source_range_get(old, first, &range);
            if (range.syntax_end == DOC_NONE || range.syntax_end > full_bytes)
                return XUI_ERROR_UNSUPPORTED;
            full_bytes -= range.syntax_end;
        } else if (full_bytes > 3) full_bytes -= 3;
    }
    if (local_bytes > 3) local_bytes -= 3;
    full_budget = 16 * (full_bytes < 65536 ? full_bytes : 65536);
    local_budget = 16 * (local_bytes < 65536 ? local_bytes : 65536);
    if (!brackets || !full_budget || !local_budget) return XUI_ERROR_UNSUPPORTED;
    for (at = 0; at < size; at += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref; uint64_t expansion;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read(old->references, at, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        /* Only link definitions consume the parser's link expansion budget. */
        if (ref.iKind == XUI_DOC_REFERENCE_FOOTNOTE) continue;
        if (ref.iKind != XUI_DOC_REFERENCE_LINK ||
            ref.iLabelStart == DOC_NONE || ref.iLabelEnd < ref.iLabelStart ||
            ref.iDestinationStart == DOC_NONE || ref.iDestinationEnd < ref.iDestinationStart ||
            (ref.iTitleStart != DOC_NONE && ref.iTitleEnd < ref.iTitleStart))
            return XUI_ERROR_UNSUPPORTED;
        expansion = ref.iLabelEnd - ref.iLabelStart +
            ref.iDestinationEnd - ref.iDestinationStart +
            (ref.iTitleStart == DOC_NONE ? 0 : ref.iTitleEnd - ref.iTitleStart);
        if (expansion > maximum) maximum = expansion;
    }
    if (maximum > (local_budget - 1) / brackets) return XUI_ERROR_UNSUPPORTED;
    if (!check_global) return XUI_OK;
    if (old->source_open_brackets &&
        maximum > (full_budget - 1) / old->source_open_brackets)
        return XUI_ERROR_UNSUPPORTED;
    return XUI_OK;
}
/* Parse a changed block against unchanged link and footnote definitions. The copied
 * definition lines follow the fragment so its node/source coordinates stay
 * local; their order preserves the parser's first-definition-wins rule. */
static int doc_inc_link_suffix(xui_document_transaction t, doc_state* old,
    uint64_t old_end, uint64_t end, char** out, uint64_t* bytes)
{
    uint64_t at, used = 0, size = doc_seq_size(old->references);
    char* suffix;
    int result;
    *out = NULL; *bytes = 0;
    if (!size || size % sizeof(xui_doc_reference_definition_t)) return XUI_ERROR_UNSUPPORTED;
    for (at = 0; at < size; at += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        uint64_t length;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read(old->references, at, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if ((ref.iKind != XUI_DOC_REFERENCE_LINK &&
             ref.iKind != XUI_DOC_REFERENCE_FOOTNOTE) ||
            ref.iSourceStart == DOC_NONE ||
            ref.iSourceEnd <= ref.iSourceStart) return XUI_ERROR_UNSUPPORTED;
        length = ref.iSourceEnd - ref.iSourceStart;
        if (used > (1u << 20) - 2 || length > (1u << 20) - used - 2)
            return XUI_ERROR_UNSUPPORTED;
        used += 2 + length;
    }
    suffix = doc_alloc(old->allocator, (size_t)used);
    if (!suffix) return XUI_ERROR_OUT_OF_MEMORY;
    used = 0;
    for (at = 0; at < size; at += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        uint64_t source_start, length;
        if ((result = doc_inc_cancel(t)) != XUI_OK) goto fail;
        result = doc_seq_read(old->references, at, &ref, sizeof(ref));
        if (result != XUI_OK) goto fail;
        source_start = ref.iSourceStart < old_end ? ref.iSourceStart :
            doc_inc_offset(ref.iSourceStart, old_end, end);
        length = ref.iSourceEnd - ref.iSourceStart;
        if (source_start > doc_seq_size(old->source) ||
            length > doc_seq_size(old->source) - source_start) {
            result = XUI_ERROR_UNSUPPORTED; goto fail;
        }
        suffix[used++] = '\n'; suffix[used++] = '\n';
        result = doc_seq_read(old->source, source_start, suffix + used, length);
        if (result != XUI_OK) goto fail;
        used += length;
    }
    *out = suffix; *bytes = used; return XUI_OK;
fail:
    doc_free(suffix); return result;
}
static int doc_inc_virtual_field_equal(uint64_t original, uint64_t original_start,
    uint64_t parsed, uint64_t virtual_start)
{
    return original == DOC_NONE ? parsed == DOC_NONE :
        original >= original_start && parsed == virtual_start + original - original_start;
}
static int doc_inc_link_suffix_matches(xui_document_transaction t, doc_state* old,
    doc_state* parsed, uint64_t fragment_bytes, uint64_t first_separator)
{
    uint64_t at, virtual_start = fragment_bytes;
    uint64_t size = doc_seq_size(old->references);
    int result;
    if (doc_seq_size(parsed->references) != size) return XUI_ERROR_UNSUPPORTED;
    for (at = 0; at < size; at += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t original, fresh;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read(old->references, at, &original, sizeof(original));
        if (result == XUI_OK) result = doc_seq_read(parsed->references, at, &fresh, sizeof(fresh));
        if (result != XUI_OK) return result;
        virtual_start += at ? 2 : first_separator;
        if (fresh.iKind != original.iKind || fresh.iSourceStart != virtual_start ||
            !doc_inc_virtual_field_equal(original.iSourceEnd, original.iSourceStart,
                fresh.iSourceEnd, virtual_start) ||
            !doc_inc_virtual_field_equal(original.iLabelStart, original.iSourceStart,
                fresh.iLabelStart, virtual_start) ||
            !doc_inc_virtual_field_equal(original.iLabelEnd, original.iSourceStart,
                fresh.iLabelEnd, virtual_start) ||
            !doc_inc_virtual_field_equal(original.iDestinationStart, original.iSourceStart,
                fresh.iDestinationStart, virtual_start) ||
            !doc_inc_virtual_field_equal(original.iDestinationEnd, original.iSourceStart,
                fresh.iDestinationEnd, virtual_start) ||
            !doc_inc_virtual_field_equal(original.iTitleStart, original.iSourceStart,
                fresh.iTitleStart, virtual_start) ||
            !doc_inc_virtual_field_equal(original.iTitleEnd, original.iSourceStart,
                fresh.iTitleEnd, virtual_start)) return XUI_ERROR_UNSUPPORTED;
        virtual_start += original.iSourceEnd - original.iSourceStart;
    }
    return XUI_OK;
}
static int doc_inc_shift_definitions(xui_document_transaction t, doc_state* state,
    uint64_t old_end, uint64_t end)
{
    uint64_t at, size = doc_seq_size(state->references);
    xui_doc_reference_definition_t* refs;
    doc_sequence* next;
    int result;
    if (!size || old_end == end) return XUI_OK;
    if (size > SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    refs = doc_alloc(state->allocator, (size_t)size);
    if (!refs) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_read(state->references, 0, refs, size);
    for (at = 0; result == XUI_OK && at < size / sizeof(*refs); at++) {
        xui_doc_reference_definition_t* ref = &refs[at];
        if ((result = doc_inc_cancel(t)) != XUI_OK) break;
        if (ref->iSourceStart < old_end) continue;
        ref->iSourceStart = doc_inc_offset(ref->iSourceStart, old_end, end);
        ref->iSourceEnd = doc_inc_offset(ref->iSourceEnd, old_end, end);
        ref->iLabelStart = doc_inc_offset(ref->iLabelStart, old_end, end);
        ref->iLabelEnd = doc_inc_offset(ref->iLabelEnd, old_end, end);
        ref->iDestinationStart = doc_inc_offset(ref->iDestinationStart, old_end, end);
        ref->iDestinationEnd = doc_inc_offset(ref->iDestinationEnd, old_end, end);
        ref->iTitleStart = doc_inc_offset(ref->iTitleStart, old_end, end);
        ref->iTitleEnd = doc_inc_offset(ref->iTitleEnd, old_end, end);
    }
    next = result == XUI_OK ? doc_seq_text(state->allocator, (const char*)refs, size) : NULL;
    if (result == XUI_OK && !next) result = XUI_ERROR_OUT_OF_MEMORY;
    doc_free(refs);
    if (result == XUI_OK) { doc_seq_release(state->references); state->references = next; }
    return result;
}
static int doc_inc_shift(xui_document_transaction t, doc_state* s, uint64_t id, uint64_t removed, uint64_t added)
{
    doc_node *n = doc_index_get(s->index, id), *copy;
    uint64_t i; int result = doc_inc_cancel(t);
    if (result != XUI_OK || removed == added) return result;
    copy = doc_node_clone(s->allocator, n);
    if (!copy) return XUI_ERROR_OUT_OF_MEMORY;
    copy->source_start = doc_inc_offset(n->source_start, removed, added);
    copy->source_end = doc_inc_offset(n->source_end, removed, added);
    copy->syntax_start = doc_inc_offset(n->syntax_start, removed, added);
    copy->syntax_end = doc_inc_offset(n->syntax_end, removed, added);
    result = doc_source_shift_provenance(copy, removed, added);
    for (i = 0; result == XUI_OK && i < doc_seq_size(copy->children); i++)
        result = doc_inc_shift(t, s, doc_seq_get_id(copy->children, i), removed, added);
    if (result == XUI_OK) result = doc_state_set(s, copy);
    doc_node_release(copy); return result;
}
static int doc_inc_remove(xui_document_transaction t, doc_state* s, doc_state* old, uint64_t id)
{
    doc_node* n = doc_index_get(old->index, id); uint64_t i; int result = doc_inc_cancel(t);
    for (i = 0; result == XUI_OK && i < doc_seq_size(n->children); i++)
        result = doc_inc_remove(t, s, old, doc_seq_get_id(n->children, i));
    return result == XUI_OK ? doc_state_remove(s, id) : result;
}
static int doc_inc_syntax_first(xui_document_transaction t, doc_sequence* syntax,
    uint64_t target, uint64_t* out)
{
    uint64_t lo = 0, hi = doc_seq_size(syntax) / sizeof(xui_doc_inline_syntax_t);
    int result;
    while (lo < hi) {
        uint64_t mid = lo + (hi - lo) / 2;
        xui_doc_inline_syntax_t span;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read_syntax(syntax, mid, &span);
        if (result != XUI_OK) return result;
        if (span.iSourceStart < target) lo = mid + 1;
        else hi = mid;
    }
    *out = lo; return XUI_OK;
}
static int doc_inc_syntax_sequence(xui_document_transaction t, doc_state* s,
    doc_sequence** target, doc_sequence* insert,
    uint64_t start, uint64_t old_end, uint64_t end)
{
    uint64_t lo, hi, i;
    xui_doc_inline_syntax_t span; doc_sequence* next = NULL;
    int result = doc_inc_syntax_first(t, *target, start, &lo);
    if (result != XUI_OK) return result;
    if (lo) {
        result = doc_seq_read_syntax(*target, lo - 1, &span);
        if (result != XUI_OK) return result;
        if (span.iSourceEnd > start) return XUI_ERROR_UNSUPPORTED;
    }
    result = doc_inc_syntax_first(t, *target, old_end, &hi);
    if (result != XUI_OK) return result;
    for (i = lo; i < hi; i++) {
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read_syntax(*target, i, &span);
        if (result != XUI_OK) return result;
        if (span.iSourceEnd > old_end || (span.iParentIndex != DOC_NONE && span.iParentIndex < lo)) return XUI_ERROR_UNSUPPORTED;
    }
    /* No inline span or reference candidate can cross a parsed top-level block boundary. Keep the
     * suffix packed bytes shared and rebase only the tree's coordinate tags. */
    result = doc_seq_syntax_replace(s->allocator, *target, lo, hi,
        insert, start, old_end, end, &next);
    if (result == XUI_OK) { doc_seq_release(*target); *target = next; }
    return result;
}
static int doc_inc_root(doc_state* s, uint64_t index, uint64_t id,
    uint64_t old_end, uint64_t end, int64_t old_shift, int was_indexed)
{
    doc_node *root = doc_index_get(s->index, DOC_ROOT), *copy = doc_node_clone(s->allocator, root);
    doc_sequence *item = NULL, *children = NULL, *shifted = NULL; uint64_t i; int result;
    if (!copy) return XUI_ERROR_OUT_OF_MEMORY;
    item = doc_seq_id(s->allocator, id);
    if (item) item->syntax_self_source = old_shift;
    result = item ? doc_seq_replace(s->allocator, copy->children, index, index + 1, item, &children) : XUI_ERROR_OUT_OF_MEMORY;
    if (result == XUI_OK && old_end != end) {
        result = doc_seq_shift_source_suffix(s->allocator, children, index + 1,
            (int64_t)end - (int64_t)old_end, &shifted);
        if (result == XUI_OK) { doc_seq_release(children); children = shifted; shifted = NULL; }
    }
    /* Footnote nodes follow normal blocks in the tree even when their
     * definitions occur earlier in source. Undo the ordinal suffix shift for
     * those earlier definitions; later definitions retain it. */
    i = doc_seq_size(root->children);
    while (result == XUI_OK && old_end != end && s->markdown_footnotes && i > index + 1) {
        doc_node* note = doc_index_get(s->index, doc_seq_get_id(root->children, i - 1));
        if (!note || note->kind != XUI_DOC_FOOTNOTE) break;
        i--;
    }
    for (; result == XUI_OK && old_end != end &&
         s->markdown_footnotes && i < doc_seq_size(root->children); i++) {
        doc_node* note = doc_index_get(s->index, doc_seq_get_id(root->children, i));
        doc_node_source_range range;
        doc_sequence *item = NULL, *corrected = NULL;
        if (!note || note->kind != XUI_DOC_FOOTNOTE) continue;
        doc_node_source_range_with_children(root->children, note, &range);
        if (range.syntax_start >= old_end) continue;
        if (range.syntax_end == DOC_NONE || range.syntax_end > old_end) {
            result = XUI_ERROR_UNSUPPORTED; break;
        }
        item = doc_seq_id(s->allocator, note->id);
        if (!item) { result = XUI_ERROR_OUT_OF_MEMORY; break; }
        item->syntax_self_source = doc_seq_source_shift(root->children, i);
        result = doc_seq_replace(s->allocator, children, i, i + 1, item, &corrected);
        doc_seq_release(item);
        if (result == XUI_OK) {
            doc_seq_release(children); children = corrected;
        }
    }
    if (result == XUI_OK) {
        doc_seq_release(copy->children); copy->children = children; children = NULL;
        copy->syntax_start = 0; copy->syntax_end = doc_seq_size(s->source);
        copy->source_start = copy->source_end = DOC_NONE; copy->source_exact = 0;
        /* Referenced footnotes are appended in reference order, which need not
         * match source order. Recompute the envelope when the ordered source
         * index is unavailable; otherwise only the edge blocks can matter. */
        if (!was_indexed) {
            for (i = 0; i < doc_seq_size(copy->children); i++) {
                doc_node* n = doc_index_get(s->index, doc_seq_get_id(copy->children, i));
                doc_node_source_range range;
                doc_node_source_range_with_children(copy->children, n, &range);
                if (n->kind == XUI_DOC_FRONT_MATTER) continue;
                if (range.source_start != DOC_NONE &&
                    (copy->source_start == DOC_NONE || range.source_start < copy->source_start))
                    copy->source_start = range.source_start;
                if (range.source_end != DOC_NONE &&
                    (copy->source_end == DOC_NONE || range.source_end > copy->source_end))
                    copy->source_end = range.source_end;
            }
        } else {
            for (i = 0; i < doc_seq_size(copy->children); i++) {
                doc_node* n = doc_index_get(s->index, doc_seq_get_id(copy->children, i));
                doc_node_source_range range; doc_node_source_range_with_children(copy->children, n, &range);
                if (n->kind == XUI_DOC_FRONT_MATTER || range.source_start == DOC_NONE) continue;
                copy->source_start = range.source_start; break;
            }
            for (i = doc_seq_size(copy->children); i; i--) {
                doc_node* n = doc_index_get(s->index, doc_seq_get_id(copy->children, i - 1));
                doc_node_source_range range; doc_node_source_range_with_children(copy->children, n, &range);
                if (n->kind == XUI_DOC_FRONT_MATTER || range.source_end == DOC_NONE) continue;
                copy->source_end = range.source_end; break;
            }
        }
        result = doc_state_set(s, copy);
        if (result == XUI_OK) {
            if (old_end != end) s->block_shifts = 1;
            s->source_blocks_indexed = was_indexed &&
                doc_markdown_source_block_valid(s, index, doc_seq_size(s->source));
        }
    }
    if (result != XUI_OK) doc_seq_release(children);
    doc_seq_release(shifted); doc_seq_release(item); doc_node_release(copy); return result;
}
static uint64_t doc_inc_value_offset(uint64_t value, uint64_t old_end, uint64_t end)
{
    return value == DOC_NONE || value < old_end ? value : value - old_end + end;
}
/* A standalone definition owns no rendered block. Shift all following block
 * coordinates through the persistent root-child index, including its source
 * envelope and source-position qualification. */
static int doc_inc_root_after_definition(xui_document_transaction t, doc_state* s, uint64_t first,
    uint64_t old_end, uint64_t end)
{
    doc_node *root = doc_index_get(s->index, DOC_ROOT), *copy = doc_node_clone(s->allocator, root);
    doc_sequence* shifted = NULL;
    uint64_t i, count;
    int result = XUI_OK, was_indexed = s->source_blocks_indexed;
    if (!copy) return XUI_ERROR_OUT_OF_MEMORY;
    count = doc_seq_size(copy->children);
    if (first > count) result = XUI_ERROR_UNSUPPORTED;
    if (result == XUI_OK && first < count)
        result = doc_seq_shift_source_suffix(s->allocator, copy->children, first,
            (int64_t)end - (int64_t)old_end, &shifted);
    if (result == XUI_OK) {
        if (shifted) { doc_seq_release(copy->children); copy->children = shifted; shifted = NULL; }
        copy->syntax_start = 0; copy->syntax_end = doc_seq_size(s->source);
        copy->source_start = copy->source_end = DOC_NONE; copy->source_exact = 0;
        for (i = 0; i < count; i++) {
            doc_node* n = doc_index_get(s->index, doc_seq_get_id(copy->children, i));
            doc_node_source_range range;
            if ((result = doc_inc_cancel(t)) != XUI_OK) break;
            if (!n) { result = XUI_ERROR_UNSUPPORTED; break; }
            doc_node_source_range_with_children(copy->children, n, &range);
            if (n->kind == XUI_DOC_FRONT_MATTER || range.source_start == DOC_NONE) continue;
            copy->source_start = range.source_start; break;
        }
        for (i = count; result == XUI_OK && i; i--) {
            doc_node* n = doc_index_get(s->index, doc_seq_get_id(copy->children, i - 1));
            doc_node_source_range range;
            if ((result = doc_inc_cancel(t)) != XUI_OK) break;
            if (!n) { result = XUI_ERROR_UNSUPPORTED; break; }
            doc_node_source_range_with_children(copy->children, n, &range);
            if (n->kind == XUI_DOC_FRONT_MATTER || range.source_end == DOC_NONE) continue;
            copy->source_end = range.source_end; break;
        }
        if (result == XUI_OK) result = doc_state_set(s, copy);
        if (result == XUI_OK) {
            if (first < count) s->block_shifts = 1;
            /* The old index proves every block and candidate is bounded and
             * ordered. A uniform suffix shift preserves those properties
             * within each side; only their shared edge can change. */
            s->source_blocks_indexed = was_indexed &&
                (first == count || doc_markdown_source_block_valid(s, first,
                    doc_seq_size(s->source)));
        }
    }
    doc_seq_release(shifted); doc_node_release(copy); return result;
}
static int doc_inc_shift_definition_value(xui_document_transaction t, doc_state* s,
    uint64_t index, const xui_doc_reference_definition_t* ref,
    uint64_t old_end, uint64_t end, uint64_t first)
{
    doc_sequence *insert = NULL, *next = NULL;
    uint64_t at = index * sizeof(*ref);
    int result = doc_inc_shift_definitions(t, s, old_end, end);
    if (result != XUI_OK) return result;
    insert = doc_seq_text(s->allocator, (const char*)ref, sizeof(*ref));
    if (!insert) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_replace(s->allocator, s->references, at, at + sizeof(*ref), insert, &next);
    doc_seq_release(insert);
    if (result != XUI_OK) return result;
    doc_seq_release(s->references); s->references = next;
    result = doc_inc_syntax_sequence(t, s, &s->inline_syntax, NULL, old_end, old_end, end);
    if (result == XUI_OK)
        result = doc_inc_syntax_sequence(t, s, &s->reference_candidates,
            NULL, old_end, old_end, end);
    if (result == XUI_OK) result = doc_inc_root_after_definition(t, s, first, old_end, end);
    return result;
}
/* Length can alter MD4C's whole-document expansion cap, while a same-length
 * edit can alter the number of open brackets. Bound possible lookups under
 * both old and new caps so unrelated links cannot change without reparsing. */
static int doc_inc_definition_budget_safe(xui_document_transaction t, doc_state* old,
    uint64_t index, const xui_doc_reference_definition_t* updated,
    uint64_t removed, uint64_t added)
{
    doc_node* root = doc_index_get(old->index, DOC_ROOT);
    uint64_t new_bytes = doc_seq_size(old->source), old_bytes, offset = 0;
    uint64_t size = doc_seq_size(old->references), at, maximum = 0, brackets;
    uint64_t budget;
    int result;
    if (new_bytes < added || !root) return XUI_ERROR_UNSUPPORTED;
    old_bytes = new_bytes - added + removed;
    if (doc_seq_size(root->children)) {
        doc_node* first = doc_index_get(old->index, doc_seq_get_id(root->children, 0));
        doc_node_source_range range;
        if (first && first->kind == XUI_DOC_FRONT_MATTER) {
            doc_node_source_range_get(old, first, &range);
            if (range.syntax_end == DOC_NONE || range.syntax_end > old_bytes ||
                range.syntax_end > new_bytes) return XUI_ERROR_UNSUPPORTED;
            offset = range.syntax_end;
        } else offset = 3;
    } else offset = 3;
    old_bytes = old_bytes > offset ? old_bytes - offset : 0;
    new_bytes = new_bytes > offset ? new_bytes - offset : 0;
    budget = 16 * (old_bytes < new_bytes ? old_bytes : new_bytes);
    if (budget > (1u << 20)) budget = 1u << 20;
    if (!budget || size % sizeof(xui_doc_reference_definition_t)) return XUI_ERROR_UNSUPPORTED;
    for (at = 0; at < size; at += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        uint64_t expansion;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read(old->references, at, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        /* An unused footnote body changes no link-definition expansion. */
        if (ref.iKind == XUI_DOC_REFERENCE_FOOTNOTE) continue;
        if (ref.iKind != XUI_DOC_REFERENCE_LINK || ref.iLabelStart == DOC_NONE ||
            ref.iLabelEnd < ref.iLabelStart || ref.iDestinationStart == DOC_NONE ||
            ref.iDestinationEnd < ref.iDestinationStart ||
            (ref.iTitleStart != DOC_NONE && ref.iTitleEnd < ref.iTitleStart))
            return XUI_ERROR_UNSUPPORTED;
        expansion = ref.iLabelEnd - ref.iLabelStart +
            ref.iDestinationEnd - ref.iDestinationStart +
            (ref.iTitleStart == DOC_NONE ? 0 : ref.iTitleEnd - ref.iTitleStart);
        if (expansion > maximum) maximum = expansion;
        if (at / sizeof(ref) == index) {
            expansion = updated->iLabelEnd - updated->iLabelStart +
                updated->iDestinationEnd - updated->iDestinationStart +
                (updated->iTitleStart == DOC_NONE ? 0 : updated->iTitleEnd - updated->iTitleStart);
            if (expansion > maximum) maximum = expansion;
        }
    }
    /* The old cap can see at most every byte removed by the edit as another
     * lookup. Keep exactly the previous conservative bound without rereading
     * the complete new source. */
    if (old->source_open_brackets > UINT64_MAX - removed) return XUI_ERROR_UNSUPPORTED;
    brackets = old->source_open_brackets + removed;
    if (brackets && maximum > (budget - 1) / brackets) return XUI_ERROR_UNSUPPORTED;
    return XUI_OK;
}
static int doc_inc_reparse_changed_definition(xui_document_transaction t,
    doc_state* old, doc_state* state, uint64_t index, doc_node* block,
    const char* suffix, uint64_t suffix_bytes, int check_global)
{
    doc_state* fragment = NULL;
    doc_node *root = doc_index_get(state->index, DOC_ROOT), *parsed_root, *fresh;
    doc_node_source_range range;
    uint64_t brackets = 0, id, first_separator = 2;
    int needs = 0, result, was_indexed;
    if (doc_inc_context_kind(block->kind) &&
        !doc_inc_definition_container_kind(block->kind)) return XUI_ERROR_UNSUPPORTED;
    doc_node_source_range_get(state, block, &range);
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
        range.syntax_end <= range.syntax_start ||
        !doc_inc_boundaries(state->source, range.syntax_start, range.syntax_end,
            doc_inc_definition_container_kind(block->kind)))
        return XUI_ERROR_UNSUPPORTED;
    result = doc_inc_definition_guard(t, state, range.syntax_start, range.syntax_end,
        range.syntax_end, &needs, &brackets);
    if (result != XUI_OK || !needs) return result == XUI_OK ? XUI_ERROR_UNSUPPORTED : result;
    /* MD4C includes separator blank lines in a list's syntax range. Adding
     * the suffix's first two newlines would extend that list into synthetic
     * source. Reuse the existing blank line for the first definition. */
    if (block->kind == XUI_DOC_LIST && suffix_bytes >= 2 &&
        range.syntax_end - range.syntax_start >= 2) {
        char tail[4];
        result = doc_seq_read(state->source, range.syntax_end - 2, tail, 2);
        if (result != XUI_OK) return result;
        if (tail[0] == '\n' && tail[1] == '\n') first_separator = 0;
        else if (range.syntax_end - range.syntax_start >= 4) {
            result = doc_seq_read(state->source, range.syntax_end - 4, tail, 4);
            if (result != XUI_OK) return result;
            if (tail[0] == '\r' && tail[1] == '\n' &&
                tail[2] == '\r' && tail[3] == '\n') first_separator = 0;
        }
    }
    result = doc_inc_link_budget_safe(t, state, range.syntax_end - range.syntax_start,
        suffix_bytes - (2 - first_separator), brackets, check_global);
    if (result == XUI_OK)
        result = doc_markdown_parse_window_with_suffix(t, range.syntax_start,
            range.syntax_end, suffix + (2 - first_separator),
            suffix_bytes - (2 - first_separator), &fragment);
    if (result != XUI_OK) return result;
    parsed_root = doc_index_get(fragment->index, DOC_ROOT);
    result = XUI_ERROR_UNSUPPORTED;
    if (doc_seq_size(parsed_root->children) != 1 || fragment->markdown_footnotes) goto done;
    result = doc_inc_link_suffix_matches(t, state, fragment,
        range.syntax_end - range.syntax_start, first_separator);
    if (result != XUI_OK) goto done;
    fresh = doc_index_get(fragment->index, doc_seq_get_id(parsed_root->children, 0));
    result = XUI_ERROR_UNSUPPORTED;
    if (fresh->kind != block->kind || fresh->syntax_start != 0 ||
        fresh->syntax_end != range.syntax_end - range.syntax_start) goto done;
    if (doc_inc_definition_container_kind(block->kind) &&
        !doc_inc_definition_shape_equal(state, block->id, fragment,
            fresh->id, range.syntax_start)) goto done;
    was_indexed = state->source_blocks_indexed && fragment->source_blocks_indexed;
    id = fresh->id;
    result = doc_inc_shift(t, fragment, id, 0, range.syntax_start);
    if (result != XUI_OK) goto done;
    result = doc_inc_remove(t, state, old, block->id);
    if (result == XUI_OK)
        result = doc_markdown_reconcile_block(t, fragment, block->id, id,
            state, index, doc_seq_source_shift(root->children, index), &id);
    if (result == XUI_OK)
        result = doc_inc_syntax_sequence(t, state, &state->inline_syntax,
            fragment->inline_syntax, range.syntax_start, range.syntax_end, range.syntax_end);
    if (result == XUI_OK)
        result = doc_inc_syntax_sequence(t, state, &state->reference_candidates,
            fragment->reference_candidates, range.syntax_start, range.syntax_end, range.syntax_end);
    if (result == XUI_OK)
        result = doc_inc_root(state, index, id, range.syntax_end, range.syntax_end,
            doc_seq_source_shift(root->children, index),
            was_indexed);
done:
    doc_state_release(fragment); return result;
}
typedef struct doc_inc_dependents_t {
    doc_allocator* allocator;
    uint64_t local[32];
    uint64_t* items;
    uint64_t count, capacity;
} doc_inc_dependents_t;
static void doc_inc_dependents_init(doc_inc_dependents_t* d, doc_allocator* allocator)
{
    d->allocator = allocator; d->items = d->local;
    d->count = 0; d->capacity = sizeof(d->local) / sizeof(d->local[0]);
}
static void doc_inc_dependents_dispose(doc_inc_dependents_t* d)
{
    if (d->items != d->local) doc_free(d->items);
}
static int doc_inc_dependents_add(doc_inc_dependents_t* d, uint64_t index)
{
    if (d->count && d->items[d->count - 1] == index) return XUI_OK;
    if (d->count == d->capacity) {
        uint64_t capacity;
        uint64_t* next;
        if (d->capacity > SIZE_MAX / (2 * sizeof(*next))) return XUI_DOC_ERROR_LIMIT;
        capacity = d->capacity * 2;
        next = doc_alloc(d->allocator, (size_t)capacity * sizeof(*next));
        if (!next) return XUI_ERROR_OUT_OF_MEMORY;
        memcpy(next, d->items, (size_t)d->count * sizeof(*next));
        if (d->items != d->local) doc_free(d->items);
        d->items = next; d->capacity = capacity;
    }
    d->items[d->count++] = index; return XUI_OK;
}
/* A changed label can alter both resolved and previously unresolved links,
 * including another definition's first-wins priority. Every independent
 * top-level block containing a possible '[' lookup is therefore invalidated. */
static int doc_inc_label_blocks(xui_document_transaction t, doc_state* state,
    doc_inc_dependents_t* dependents)
{
    doc_node* root = doc_index_get(state->index, DOC_ROOT);
    unsigned char bytes[512]; uint64_t i;
    int result;
    dependents->count = 0;
    for (i = 0; i < doc_seq_size(root->children); i++) {
        doc_node* block = doc_index_get(state->index, doc_seq_get_id(root->children, i));
        doc_node_source_range range; uint64_t at; int found = 0;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        if (!block) return XUI_ERROR_UNSUPPORTED;
        if (block->kind == XUI_DOC_FRONT_MATTER) continue;
        doc_node_source_range_get(state, block, &range);
        if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
            range.syntax_end < range.syntax_start ||
            range.syntax_end > doc_seq_size(state->source)) return XUI_ERROR_UNSUPPORTED;
        for (at = range.syntax_start; at < range.syntax_end && !found; ) {
            uint64_t size = range.syntax_end - at, j;
            if (size > sizeof(bytes)) size = sizeof(bytes);
            if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
            result = doc_seq_read(state->source, at, bytes, size);
            if (result != XUI_OK) return result;
            for (j = 0; j < size; j++) if (bytes[j] == '[') { found = 1; break; }
            at += size;
        }
        if (!found) continue;
        if (!doc_inc_kind(block->kind) ||
            (doc_inc_context_kind(block->kind) &&
                !doc_inc_definition_container_kind(block->kind)))
            return XUI_ERROR_UNSUPPORTED;
        result = doc_inc_dependents_add(dependents, i);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
/* The source index proves nonoverlapping root ranges. Only the block just
 * before the first following block can overlap a standalone definition. */
static int doc_inc_definition_first_shift(xui_document_transaction t,
    doc_state* s, const xui_doc_reference_definition_t* ref, uint64_t* first)
{
    doc_node* root = doc_index_get(s->index, DOC_ROOT);
    uint64_t count = doc_seq_size(root->children), at = 0;
    int result;
    if (s->source_blocks_indexed) {
        uint64_t lo = 0, hi = count;
        while (lo < hi) {
            uint64_t mid = lo + (hi - lo) / 2;
            doc_node* block = doc_index_get(s->index, doc_seq_get_id(root->children, mid));
            doc_node_source_range range;
            if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
            if (!block) return XUI_ERROR_UNSUPPORTED;
            doc_node_source_range_get(s, block, &range);
            if (range.syntax_start < ref->iSourceEnd) lo = mid + 1;
            else hi = mid;
        }
        if (lo) {
            doc_node* block = doc_index_get(s->index, doc_seq_get_id(root->children, lo - 1));
            doc_node_source_range range;
            if (!block) return XUI_ERROR_UNSUPPORTED;
            doc_node_source_range_get(s, block, &range);
            if (ref->iSourceStart < range.syntax_end &&
                ref->iSourceEnd > range.syntax_start) return XUI_ERROR_UNSUPPORTED;
        }
        *first = lo;
        return XUI_OK;
    }
    *first = count;
    for (; at < count; at++) {
        doc_node* block = doc_index_get(s->index, doc_seq_get_id(root->children, at));
        doc_node_source_range range;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        if (!block) return XUI_ERROR_UNSUPPORTED;
        doc_node_source_range_get(s, block, &range);
        if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
            (ref->iSourceStart < range.syntax_end && ref->iSourceEnd > range.syntax_start))
            return XUI_ERROR_UNSUPPORTED;
        if (*first == count && range.syntax_start >= ref->iSourceEnd) *first = at;
    }
    return XUI_OK;
}
/* Find the first source block containing the complete inline syntax, starting
 * where the previous dependent was found. Equal block ends keep the linear
 * scan's choice at a shared boundary. */
static int doc_inc_definition_span_block(xui_document_transaction t,
    doc_state* s, uint64_t start, uint64_t end, uint64_t lower, uint64_t* found)
{
    doc_node* root = doc_index_get(s->index, DOC_ROOT);
    uint64_t count = doc_seq_size(root->children), at = lower;
    int result;
    *found = DOC_NONE;
    if (s->source_blocks_indexed) {
        uint64_t hi = count;
        while (at < hi) {
            uint64_t mid = at + (hi - at) / 2;
            doc_node* block = doc_index_get(s->index, doc_seq_get_id(root->children, mid));
            doc_node_source_range range;
            if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
            if (!block) return XUI_ERROR_UNSUPPORTED;
            doc_node_source_range_get(s, block, &range);
            if (range.syntax_end < end) at = mid + 1;
            else hi = mid;
        }
        if (at < count) {
            doc_node* block = doc_index_get(s->index, doc_seq_get_id(root->children, at));
            doc_node_source_range range;
            if (!block) return XUI_ERROR_UNSUPPORTED;
            doc_node_source_range_get(s, block, &range);
            if (range.syntax_start <= start && end <= range.syntax_end) {
                *found = at; return XUI_OK;
            }
        }
        return XUI_ERROR_UNSUPPORTED;
    }
    for (; at < count; at++) {
        doc_node* block = doc_index_get(s->index, doc_seq_get_id(root->children, at));
        doc_node_source_range range;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        if (!block) return XUI_ERROR_UNSUPPORTED;
        doc_node_source_range_get(s, block, &range);
        if (range.syntax_start <= start && end <= range.syntax_end) {
            *found = at; return XUI_OK;
        }
    }
    return XUI_ERROR_UNSUPPORTED;
}
static int doc_inc_definition_value_blocks(xui_document_transaction t,
    doc_state* old, uint64_t definition, doc_inc_dependents_t* dependents)
{
    uint64_t at, size = doc_seq_size(old->inline_syntax);
    int result;
    if (size % sizeof(xui_doc_inline_syntax_t)) return XUI_ERROR_UNSUPPORTED;
    for (at = 0; at < size; at += sizeof(xui_doc_inline_syntax_t)) {
        xui_doc_inline_syntax_t syntax;
        uint64_t found;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read_syntax(old->inline_syntax,
            at / sizeof(syntax), &syntax);
        if (result != XUI_OK) return result;
        if (syntax.iDefinitionIndex != definition) continue;
        if (syntax.iKind != XUI_DOC_SYNTAX_LINK && syntax.iKind != XUI_DOC_SYNTAX_IMAGE)
            return XUI_ERROR_UNSUPPORTED;
        result = doc_inc_definition_span_block(t, old, syntax.iSourceStart,
            syntax.iSourceEnd, dependents->count ? dependents->items[dependents->count - 1] : 0,
            &found);
        if (result != XUI_OK) return result;
        if (dependents->count && found < dependents->items[dependents->count - 1])
            return XUI_ERROR_UNSUPPORTED;
        result = doc_inc_dependents_add(dependents, found);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
/* A footnote definition without a rendered footnote has no semantic subtree.
 * A body-only edit can therefore update its source range and shift following
 * coordinates after a standalone parse proves the definition still exists.
 * The root must have no footnotes, since appended footnote nodes are ordered
 * by reference rather than by source and need a separate reconciliation. */
static int doc_inc_unused_footnote_body(xui_document_transaction t, doc_state** out)
{
    doc_state *old = t->draft, *fragment = NULL, *state = NULL;
    doc_node* root = doc_index_get(old->index, DOC_ROOT);
    const xui_doc_operation_t* op;
    xui_doc_reference_definition_t ref, parsed, expected;
    uint64_t at, count, index = DOC_NONE, old_end, end, first;
    int result;
    if (t->domain != XUI_DOC_SOURCE || t->count != t->parse_op_start + 1 ||
        !old->markdown_footnotes || !root) return XUI_ERROR_UNSUPPORTED;
    op = &t->ops[t->parse_op_start];
    if (op->iKind != XUI_DOC_OP_SOURCE || (!op->iOldLength && !op->iNewLength) ||
        op->iOffset > UINT64_MAX - op->iOldLength ||
        op->iOffset > UINT64_MAX - op->iNewLength) return XUI_ERROR_UNSUPPORTED;
    count = doc_seq_size(old->references);
    if (!count || count % sizeof(ref)) return XUI_ERROR_UNSUPPORTED;
    for (at = 0; at < count; at += sizeof(ref)) {
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read(old->references, at, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if (ref.iKind != XUI_DOC_REFERENCE_FOOTNOTE ||
            ref.iLabelEnd == DOC_NONE || ref.iLabelEnd > UINT64_MAX - 2 ||
            ref.iSourceEnd == DOC_NONE || ref.iSourceEnd <= ref.iLabelEnd + 2)
            continue;
        /* Keep the label, closing bracket, colon and final line ending out
         * of this path. The standalone parse checks the remaining grammar. */
        if (op->iOffset > ref.iLabelEnd + 2 &&
            op->iOffset + op->iOldLength < ref.iSourceEnd) {
            index = at / sizeof(ref); break;
        }
    }
    if (index == DOC_NONE) return XUI_ERROR_UNSUPPORTED;
    if (doc_seq_size(root->children)) {
        doc_node* last = doc_index_get(old->index,
            doc_seq_get_id(root->children, doc_seq_size(root->children) - 1));
        if (!last || last->kind == XUI_DOC_FOOTNOTE) return XUI_ERROR_UNSUPPORTED;
    }
    old_end = ref.iSourceEnd;
    if (old_end < op->iOldLength ||
        old_end - op->iOldLength > UINT64_MAX - op->iNewLength)
        return XUI_ERROR_UNSUPPORTED;
    end = old_end - op->iOldLength + op->iNewLength;
    if (end > doc_seq_size(old->source) ||
        !doc_inc_boundaries(old->source, ref.iSourceStart, end, 0))
        return XUI_ERROR_UNSUPPORTED;
    result = doc_inc_definition_first_shift(t, old, &ref, &first);
    if (result != XUI_OK) return result;
    result = doc_markdown_parse_window(t, ref.iSourceStart, end, &fragment);
    if (result != XUI_OK) {
        if (result != XUI_ERROR_OUT_OF_MEMORY && result != XUI_DOC_ERROR_CANCELLED &&
            result != XUI_DOC_ERROR_STALE) result = XUI_ERROR_UNSUPPORTED;
        return result;
    }
    result = XUI_ERROR_UNSUPPORTED;
    if (!fragment->markdown_footnotes ||
        doc_seq_size(fragment->references) != sizeof(parsed) ||
        doc_seq_size(doc_index_get(fragment->index, DOC_ROOT)->children) ||
        doc_seq_size(fragment->inline_syntax) ||
        doc_seq_size(fragment->reference_candidates)) goto done;
    result = doc_seq_read(fragment->references, 0, &parsed, sizeof(parsed));
    if (result != XUI_OK) goto done;
    result = XUI_ERROR_UNSUPPORTED;
    expected = ref; expected.iSourceEnd = end;
    if (parsed.iKind != expected.iKind || parsed.iSourceStart != 0 ||
        !doc_inc_virtual_field_equal(expected.iSourceEnd, ref.iSourceStart, parsed.iSourceEnd, 0) ||
        !doc_inc_virtual_field_equal(expected.iLabelStart, ref.iSourceStart, parsed.iLabelStart, 0) ||
        !doc_inc_virtual_field_equal(expected.iLabelEnd, ref.iSourceStart, parsed.iLabelEnd, 0) ||
        parsed.iDestinationStart != DOC_NONE || parsed.iDestinationEnd != DOC_NONE ||
        parsed.iTitleStart != DOC_NONE || parsed.iTitleEnd != DOC_NONE) goto done;
    if (end != old_end) {
        result = doc_inc_definition_budget_safe(t, old, index, &expected,
            op->iOldLength, op->iNewLength);
        if (result != XUI_OK) goto done;
    }
    state = doc_state_clone(old);
    if (!state) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    result = end == old_end ? XUI_OK :
        doc_inc_shift_definition_value(t, state, index, &expected, old_end, end, first);
    if (result == XUI_OK) result = doc_inc_cancel(t);
    if (result == XUI_OK) {
        atomic_fetch_add(&old->allocator->markdown_incremental_parses, 1);
        *out = state; state = NULL;
    }
done:
    doc_state_release(state); doc_state_release(fragment); return result;
}
typedef struct doc_inc_note_virtual_t {
    char* suffix;
    uint64_t bytes, use_start;
    uint64_t* definitions; /* Virtual definition index to full-document index. */
    uint64_t count;
} doc_inc_note_virtual_t;
static void doc_inc_note_virtual_dispose(doc_inc_note_virtual_t* v)
{
    doc_free(v->suffix); doc_free(v->definitions);
}
/* A referenced definition can be rendered by a synthetic use after its local
 * source window. External link definitions retain their relative order, so
 * links in the footnote body resolve exactly as in the full document. Other
 * footnotes are excluded: a body containing a footnote use takes the full
 * parser until its transitive dependency order can be updated together. */
static int doc_inc_note_virtual_build(xui_document_transaction t,
    doc_state* old, const xui_doc_reference_definition_t* changed,
    uint64_t definition, uint64_t end, doc_inc_note_virtual_t* v)
{
    uint64_t at, total = doc_seq_size(old->references) / sizeof(*changed);
    uint64_t label = changed->iLabelEnd - changed->iLabelStart;
    uint64_t capacity = 7 + label, used = 0;
    int result;
    memset(v, 0, sizeof(*v));
    if (label == 0 || label > 1024 || total > SIZE_MAX / sizeof(uint64_t) - 1 ||
        capacity > (1u << 20)) return XUI_ERROR_UNSUPPORTED;
    v->definitions = doc_alloc(old->allocator, (size_t)(total + 1) * sizeof(uint64_t));
    if (!v->definitions) return XUI_ERROR_OUT_OF_MEMORY;
    for (at = 0; at < total; at++) {
        xui_doc_reference_definition_t ref;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read(old->references, at * sizeof(ref), &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if (ref.iKind == XUI_DOC_REFERENCE_FOOTNOTE) continue;
        if (ref.iKind != XUI_DOC_REFERENCE_LINK || ref.iSourceStart == DOC_NONE ||
            ref.iSourceEnd <= ref.iSourceStart ||
            (ref.iSourceStart < changed->iSourceEnd && ref.iSourceEnd > changed->iSourceStart) ||
            capacity > (1u << 20) - 2 ||
            ref.iSourceEnd - ref.iSourceStart > (1u << 20) - capacity - 2)
            return XUI_ERROR_UNSUPPORTED;
        capacity += 2 + ref.iSourceEnd - ref.iSourceStart;
    }
    v->suffix = doc_alloc(old->allocator, (size_t)capacity);
    if (!v->suffix) return XUI_ERROR_OUT_OF_MEMORY;
    v->definitions[v->count++] = definition;
    v->suffix[used++] = '\n'; v->suffix[used++] = '\n';
    v->use_start = end - changed->iSourceStart + used;
    v->suffix[used++] = 'X'; v->suffix[used++] = '['; v->suffix[used++] = '^';
    result = doc_seq_read(old->source, changed->iLabelStart, v->suffix + used, label);
    if (result != XUI_OK) return result;
    used += label; v->suffix[used++] = ']'; v->suffix[used++] = '\n';
    for (at = 0; at < total; at++) {
        xui_doc_reference_definition_t ref;
        uint64_t start, length;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read(old->references, at * sizeof(ref), &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if (ref.iKind != XUI_DOC_REFERENCE_LINK) continue;
        start = doc_inc_value_offset(ref.iSourceStart, changed->iSourceEnd, end);
        length = ref.iSourceEnd - ref.iSourceStart;
        if (start > doc_seq_size(old->source) || length > doc_seq_size(old->source) - start)
            return XUI_ERROR_UNSUPPORTED;
        v->suffix[used++] = '\n'; v->suffix[used++] = '\n';
        result = doc_seq_read(old->source, start, v->suffix + used, length);
        if (result != XUI_OK) return result;
        used += length; v->definitions[v->count++] = at;
    }
    if (used != capacity) return XUI_ERROR_UNSUPPORTED;
    v->bytes = used; return XUI_OK;
}
static int doc_inc_note_has_nested_use(xui_document_transaction t,
    doc_sequence* source, uint64_t start, uint64_t end)
{
    unsigned char bytes[512], previous = 0;
    uint64_t at;
    int result;
    for (at = start; at < end; ) {
        uint64_t count = end - at < sizeof(bytes) ? end - at : sizeof(bytes), i;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read(source, at, bytes, count);
        if (result != XUI_OK) return result;
        for (i = 0; i < count; i++) {
            if (previous == '[' && bytes[i] == '^') return XUI_ERROR_UNSUPPORTED;
            previous = bytes[i];
        }
        at += count;
    }
    return XUI_OK;
}
static int doc_inc_note_virtual_check(xui_document_transaction t,
    doc_state* old, doc_state* fragment, const xui_doc_reference_definition_t* changed,
    uint64_t end, const doc_inc_note_virtual_t* v, uint64_t* fresh_id)
{
    doc_node* root = doc_index_get(fragment->index, DOC_ROOT);
    doc_node *use, *note;
    xui_doc_reference_definition_t parsed;
    uint64_t i, offset = v->use_start + 3 +
        changed->iLabelEnd - changed->iLabelStart + 2;
    int result;
    if (!root || !fragment->markdown_footnotes ||
        doc_seq_size(root->children) != 2 ||
        doc_seq_size(fragment->references) != v->count * sizeof(parsed))
        return XUI_ERROR_UNSUPPORTED;
    use = doc_index_get(fragment->index, doc_seq_get_id(root->children, 0));
    note = doc_index_get(fragment->index, doc_seq_get_id(root->children, 1));
    if (!use || !note || use->kind != XUI_DOC_PARAGRAPH ||
        note->kind != XUI_DOC_FOOTNOTE || note->syntax_start != 0 ||
        note->syntax_end != end - changed->iSourceStart ||
        use->syntax_start < end - changed->iSourceStart)
        return XUI_ERROR_UNSUPPORTED;
    result = doc_seq_read(fragment->references, 0, &parsed, sizeof(parsed));
    if (result != XUI_OK) return result;
    if (parsed.iKind != XUI_DOC_REFERENCE_FOOTNOTE || parsed.iSourceStart != 0 ||
        parsed.iSourceEnd != end - changed->iSourceStart ||
        !doc_inc_virtual_field_equal(changed->iLabelStart, changed->iSourceStart,
            parsed.iLabelStart, 0) ||
        !doc_inc_virtual_field_equal(changed->iLabelEnd, changed->iSourceStart,
            parsed.iLabelEnd, 0) ||
        parsed.iDestinationStart != DOC_NONE || parsed.iDestinationEnd != DOC_NONE ||
        parsed.iTitleStart != DOC_NONE || parsed.iTitleEnd != DOC_NONE)
        return XUI_ERROR_UNSUPPORTED;
    for (i = 1; i < v->count; i++) {
        xui_doc_reference_definition_t original;
        uint64_t length;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read(old->references, v->definitions[i] * sizeof(original),
            &original, sizeof(original));
        if (result == XUI_OK)
            result = doc_seq_read(fragment->references, i * sizeof(parsed), &parsed, sizeof(parsed));
        if (result != XUI_OK) return result;
        length = original.iSourceEnd - original.iSourceStart;
        offset += 2;
        if (parsed.iKind != XUI_DOC_REFERENCE_LINK || parsed.iSourceStart != offset ||
            !doc_inc_virtual_field_equal(original.iSourceEnd, original.iSourceStart,
                parsed.iSourceEnd, offset) ||
            !doc_inc_virtual_field_equal(original.iLabelStart, original.iSourceStart,
                parsed.iLabelStart, offset) ||
            !doc_inc_virtual_field_equal(original.iLabelEnd, original.iSourceStart,
                parsed.iLabelEnd, offset) ||
            !doc_inc_virtual_field_equal(original.iDestinationStart, original.iSourceStart,
                parsed.iDestinationStart, offset) ||
            !doc_inc_virtual_field_equal(original.iDestinationEnd, original.iSourceStart,
                parsed.iDestinationEnd, offset) ||
            !doc_inc_virtual_field_equal(original.iTitleStart, original.iSourceStart,
                parsed.iTitleStart, offset) ||
            !doc_inc_virtual_field_equal(original.iTitleEnd, original.iSourceStart,
                parsed.iTitleEnd, offset)) return XUI_ERROR_UNSUPPORTED;
        offset += length;
    }
    if (offset != end - changed->iSourceStart + v->bytes)
        return XUI_ERROR_UNSUPPORTED;
    *fresh_id = note->id; return XUI_OK;
}
static int doc_inc_note_location(xui_document_transaction t, doc_state* old,
    const xui_doc_reference_definition_t* ref, uint64_t* ordinal, uint64_t* first)
{
    doc_node* root = doc_index_get(old->index, DOC_ROOT);
    uint64_t count = doc_seq_size(root->children), ordinary = count, lo = 0, hi, i;
    int result;
    *ordinal = *first = DOC_NONE;
    while (ordinary) {
        doc_node* node = doc_index_get(old->index,
            doc_seq_get_id(root->children, ordinary - 1));
        if (!node) return XUI_ERROR_UNSUPPORTED;
        if (node->kind != XUI_DOC_FOOTNOTE) break;
        ordinary--;
    }
    /* The ordinary prefix is source ordered even though appended footnotes
     * follow first-use order. Find the first block ending inside/after this
     * definition; only that boundary can overlap it. */
    hi = ordinary;
    while (lo < hi) {
        uint64_t mid = lo + (hi - lo) / 2;
        doc_node* node = doc_index_get(old->index,
            doc_seq_get_id(root->children, mid));
        doc_node_source_range range;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        if (!node) return XUI_ERROR_UNSUPPORTED;
        doc_node_source_range_get(old, node, &range);
        if (range.syntax_end == DOC_NONE) return XUI_ERROR_UNSUPPORTED;
        if (range.syntax_end <= ref->iSourceStart) lo = mid + 1;
        else hi = mid;
    }
    *first = lo;
    if (lo < ordinary) {
        doc_node* node = doc_index_get(old->index,
            doc_seq_get_id(root->children, lo));
        doc_node_source_range range;
        if (!node) return XUI_ERROR_UNSUPPORTED;
        doc_node_source_range_get(old, node, &range);
        if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
            range.syntax_start < ref->iSourceEnd)
            return XUI_ERROR_UNSUPPORTED;
    }
    for (i = ordinary; i < count; i++) {
        doc_node* node = doc_index_get(old->index, doc_seq_get_id(root->children, i));
        doc_node_source_range range;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        if (!node || node->kind != XUI_DOC_FOOTNOTE) return XUI_ERROR_UNSUPPORTED;
        doc_node_source_range_get(old, node, &range);
        if (range.syntax_start == ref->iSourceStart &&
            range.syntax_end == ref->iSourceEnd) {
            if (*ordinal != DOC_NONE) return XUI_ERROR_UNSUPPORTED;
            *ordinal = i;
        } else if (range.syntax_start < ref->iSourceEnd &&
            range.syntax_end > ref->iSourceStart) return XUI_ERROR_UNSUPPORTED;
    }
    return *ordinal == DOC_NONE ? XUI_ERROR_UNSUPPORTED : XUI_OK;
}
static int doc_inc_note_syntax_run(xui_document_transaction t, doc_sequence* syntax,
    uint64_t start, uint64_t end, uint64_t* lo, uint64_t* hi)
{
    uint64_t count = doc_seq_size(syntax) / sizeof(xui_doc_inline_syntax_t), i;
    int result;
    *lo = *hi = DOC_NONE;
    if (doc_seq_size(syntax) % sizeof(xui_doc_inline_syntax_t)) return XUI_ERROR_UNSUPPORTED;
    for (i = 0; i < count; i++) {
        xui_doc_inline_syntax_t span;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read_syntax(syntax, i, &span);
        if (result != XUI_OK) return result;
        if (span.iSourceStart >= end || span.iSourceEnd <= start) continue;
        if (span.iSourceStart < start || span.iSourceEnd > end ||
            (*hi != DOC_NONE && i != *hi)) return XUI_ERROR_UNSUPPORTED;
        if (*lo == DOC_NONE) *lo = i;
        *hi = i + 1;
    }
    return XUI_OK;
}
static int doc_inc_note_shift_kept_syntax(xui_doc_inline_syntax_t* span,
    uint64_t start, uint64_t old_end, uint64_t end,
    uint64_t lo, uint64_t hi, uint64_t added)
{
    uint64_t* fields[] = {&span->iSourceStart, &span->iSourceEnd,
        &span->iContentStart, &span->iContentEnd};
    unsigned i;
    if (span->iSourceStart < old_end && span->iSourceEnd > start) return XUI_ERROR_UNSUPPORTED;
    for (i = 0; i < sizeof(fields) / sizeof(*fields); i++) {
        if (*fields[i] == DOC_NONE) continue;
        if (*fields[i] >= old_end) *fields[i] = doc_inc_offset(*fields[i], old_end, end);
        else if (*fields[i] >= start) return XUI_ERROR_UNSUPPORTED;
    }
    if (span->iParentIndex != DOC_NONE) {
        if (span->iParentIndex >= lo && span->iParentIndex < hi)
            return XUI_ERROR_UNSUPPORTED;
        if (span->iParentIndex >= hi)
            span->iParentIndex = span->iParentIndex - (hi - lo) + added;
    }
    return XUI_OK;
}
static int doc_inc_note_shift_syntax_run(doc_state* state, doc_sequence** target,
    uint64_t lo, uint64_t hi, int64_t delta)
{
    doc_sequence* next = NULL;
    int result = doc_seq_syntax_shift_source_range(state->allocator, *target,
        lo, hi, delta, &next);
    if (result == XUI_OK) {
        doc_seq_release(*target); *target = next;
    }
    return result;
}
static int doc_inc_note_syntax(xui_document_transaction t, doc_state* old,
    doc_state* state, doc_state* fragment, const xui_doc_reference_definition_t* ref,
    uint64_t end, uint64_t note_ordinal, const doc_inc_note_virtual_t* v)
{
    doc_node* root = doc_index_get(old->index, DOC_ROOT);
    uint64_t count = doc_seq_size(old->inline_syntax) / sizeof(xui_doc_inline_syntax_t);
    uint64_t parsed_count = doc_seq_size(fragment->inline_syntax) / sizeof(xui_doc_inline_syntax_t);
    uint64_t lo, hi, flo, fhi, i, j, next_count, added;
    uint64_t run_lo = DOC_NONE, run_hi = DOC_NONE;
    xui_doc_inline_syntax_t* spans = NULL;
    doc_sequence *insert = NULL, *packed = NULL;
    int result;
    result = doc_inc_note_syntax_run(t, old->inline_syntax,
        ref->iSourceStart, ref->iSourceEnd, &lo, &hi);
    if (result != XUI_OK) return result;
    result = doc_inc_note_syntax_run(t, fragment->inline_syntax,
        0, end - ref->iSourceStart, &flo, &fhi);
    if (result != XUI_OK) return result;
    for (i = 0; i < parsed_count; i++) {
        xui_doc_inline_syntax_t span;
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read_syntax(fragment->inline_syntax, i, &span);
        if (result != XUI_OK) return result;
        if (i >= flo && i < fhi) continue;
        if (i != 0 || span.iKind != XUI_DOC_SYNTAX_FOOTNOTE_REF ||
            span.iSourceStart != v->use_start + 1 ||
            span.iDefinitionIndex != 0 || span.iParentIndex != DOC_NONE)
            return XUI_ERROR_UNSUPPORTED;
    }
    if (!parsed_count) return XUI_ERROR_UNSUPPORTED;
    if (flo == DOC_NONE) flo = fhi = 0;
    if (lo == DOC_NONE) {
        lo = hi = count;
        for (i = note_ordinal + 1; i < doc_seq_size(root->children); i++) {
            doc_node* note = doc_index_get(old->index, doc_seq_get_id(root->children, i));
            doc_node_source_range range;
            uint64_t next_lo, next_hi;
            if (!note || note->kind != XUI_DOC_FOOTNOTE) return XUI_ERROR_UNSUPPORTED;
            doc_node_source_range_get(old, note, &range);
            result = doc_inc_note_syntax_run(t, old->inline_syntax,
                range.syntax_start, range.syntax_end, &next_lo, &next_hi);
            if (result != XUI_OK) return result;
            if (next_lo != DOC_NONE) { lo = hi = next_lo; break; }
        }
    }
    added = fhi - flo;
    if (added > UINT64_MAX - (count - (hi - lo))) return XUI_DOC_ERROR_LIMIT;
    next_count = count - (hi - lo) + added;
    if (added > SIZE_MAX / sizeof(*spans)) return XUI_DOC_ERROR_LIMIT;
    if (added) {
        spans = doc_alloc(state->allocator, (size_t)added * sizeof(*spans));
        if (!spans) return XUI_ERROR_OUT_OF_MEMORY;
    }
    for (j = flo; j < fhi; j++) {
        xui_doc_inline_syntax_t* span = &spans[j - flo];
        if ((result = doc_inc_cancel(t)) != XUI_OK) goto done;
        result = doc_seq_read_syntax(fragment->inline_syntax, j, span);
        if (result != XUI_OK) goto done;
        if (span->iParentIndex != DOC_NONE) {
            if (span->iParentIndex < flo || span->iParentIndex >= fhi) {
                result = XUI_ERROR_UNSUPPORTED; goto done;
            }
            span->iParentIndex -= flo;
        }
        if (span->iDefinitionIndex != DOC_NONE) {
            if (span->iDefinitionIndex >= v->count) {
                result = XUI_ERROR_UNSUPPORTED; goto done;
            }
            span->iDefinitionIndex = v->definitions[span->iDefinitionIndex];
        }
        span->iSourceStart = doc_inc_offset(span->iSourceStart, 0, ref->iSourceStart);
        span->iSourceEnd = doc_inc_offset(span->iSourceEnd, 0, ref->iSourceStart);
        span->iContentStart = doc_inc_offset(span->iContentStart, 0, ref->iSourceStart);
        span->iContentEnd = doc_inc_offset(span->iContentEnd, 0, ref->iSourceStart);
    }
    if (added) {
        insert = doc_seq_text(state->allocator, (const char*)spans,
            added * sizeof(*spans));
        if (!insert) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    }
    /* Replace the note run persistently. Its suffix needs only the parent
     * index displacement; source shifts are applied below to source-ordered
     * runs, since appended footnotes are in first-use rather than source order. */
    result = doc_seq_syntax_replace(state->allocator, old->inline_syntax,
        lo, hi, insert, 0, 0, 0, &packed);
    if (result != XUI_OK) goto done;
    if (doc_seq_size(packed) / sizeof(*spans) != next_count) {
        result = XUI_ERROR_UNSUPPORTED; goto done;
    }
    for (i = 0; i < count; i++) {
        xui_doc_inline_syntax_t original, shifted;
        uint64_t mapped;
        int needs_shift;
        if (i >= lo && i < hi) continue;
        if ((result = doc_inc_cancel(t)) != XUI_OK) goto done;
        result = doc_seq_read_syntax(old->inline_syntax, i, &original);
        if (result != XUI_OK) goto done;
        shifted = original;
        result = doc_inc_note_shift_kept_syntax(&shifted,
            ref->iSourceStart, ref->iSourceEnd, end, lo, hi, added);
        if (result != XUI_OK) goto done;
        if (original.iParentIndex != DOC_NONE &&
            ((i < lo && original.iParentIndex >= hi) ||
             (i >= hi && original.iParentIndex < hi))) {
            result = XUI_ERROR_UNSUPPORTED; goto done;
        }
        needs_shift = original.iSourceStart != shifted.iSourceStart;
        if ((original.iSourceEnd != shifted.iSourceEnd) != needs_shift ||
            (original.iContentStart != DOC_NONE &&
             (original.iContentStart != shifted.iContentStart) != needs_shift) ||
            (original.iContentEnd != DOC_NONE &&
             (original.iContentEnd != shifted.iContentEnd) != needs_shift)) {
            result = XUI_ERROR_UNSUPPORTED; goto done;
        }
        mapped = i < lo ? i : i - (hi - lo) + added;
        if (needs_shift && run_lo != DOC_NONE && mapped == run_hi) {
            run_hi++;
        } else {
            if (run_lo != DOC_NONE) {
                result = doc_inc_note_shift_syntax_run(state, &packed,
                    run_lo, run_hi, (int64_t)end - (int64_t)ref->iSourceEnd);
                if (result != XUI_OK) goto done;
                run_lo = run_hi = DOC_NONE;
            }
            if (needs_shift) { run_lo = mapped; run_hi = mapped + 1; }
        }
    }
    if (run_lo != DOC_NONE)
        result = doc_inc_note_shift_syntax_run(state, &packed,
            run_lo, run_hi, (int64_t)end - (int64_t)ref->iSourceEnd);
    if (result == XUI_OK) {
        doc_seq_release(state->inline_syntax); state->inline_syntax = packed; packed = NULL;
    }
done:
    doc_seq_release(insert); doc_seq_release(packed); doc_free(spans); return result;
}
static int doc_inc_note_root(xui_document_transaction t, doc_state* old,
    doc_state* state, uint64_t ordinal, uint64_t first, uint64_t fresh_id,
    uint64_t old_end, uint64_t end)
{
    doc_node* original = doc_index_get(old->index, DOC_ROOT);
    doc_node* copy = doc_node_clone(state->allocator, doc_index_get(state->index, DOC_ROOT));
    doc_sequence *item = NULL, *children = NULL, *shifted = NULL;
    uint64_t i, count = doc_seq_size(original->children), ordinary = count;
    int result = XUI_OK;
    if (!copy) return XUI_ERROR_OUT_OF_MEMORY;
    if (ordinal >= count || first > ordinal) result = XUI_ERROR_UNSUPPORTED;
    if (result == XUI_OK) {
        item = doc_seq_id(state->allocator, fresh_id);
        if (!item) result = XUI_ERROR_OUT_OF_MEMORY;
    }
    if (result == XUI_OK) {
        item->syntax_self_source = doc_seq_source_shift(original->children, ordinal);
        result = doc_seq_replace(state->allocator, copy->children,
            ordinal, ordinal + 1, item, &children);
    }
    if (result == XUI_OK && old_end != end)
        result = doc_seq_shift_source_suffix(state->allocator, children, first,
            (int64_t)end - (int64_t)old_end, &shifted);
    if (result == XUI_OK && shifted) {
        doc_seq_release(children); children = shifted; shifted = NULL;
    }
    /* Normal blocks form a source-ordered prefix; footnotes follow first-use
     * order. Restore the old tag on notes whose definitions precede this
     * edit, including the newly reconciled note, after shifting the suffix. */
    for (i = first; result == XUI_OK && old_end != end && i < count; i++) {
        doc_node* previous = doc_index_get(old->index,
            doc_seq_get_id(original->children, i));
        doc_node_source_range range;
        doc_sequence *corrected_item = NULL, *corrected = NULL;
        if ((result = doc_inc_cancel(t)) != XUI_OK) break;
        if (!previous) { result = XUI_ERROR_UNSUPPORTED; break; }
        if (previous->kind != XUI_DOC_FOOTNOTE) continue;
        doc_node_source_range_with_children(original->children, previous, &range);
        if (range.syntax_start >= old_end) continue;
        if (range.syntax_end > old_end) { result = XUI_ERROR_UNSUPPORTED; break; }
        corrected_item = doc_seq_id(state->allocator,
            i == ordinal ? fresh_id : previous->id);
        if (!corrected_item) { result = XUI_ERROR_OUT_OF_MEMORY; break; }
        corrected_item->syntax_self_source =
            doc_seq_source_shift(original->children, i);
        result = doc_seq_replace(state->allocator, children, i, i + 1,
            corrected_item, &corrected);
        doc_seq_release(corrected_item);
        if (result == XUI_OK) {
            doc_seq_release(children); children = corrected;
        }
    }
    if (result == XUI_OK) {
        doc_seq_release(copy->children); copy->children = children; children = NULL;
        copy->syntax_start = 0; copy->syntax_end = doc_seq_size(state->source);
        copy->source_start = copy->source_end = DOC_NONE; copy->source_exact = 0;
        while (ordinary) {
            doc_node* node = doc_index_get(state->index,
                doc_seq_get_id(copy->children, ordinary - 1));
            if ((result = doc_inc_cancel(t)) != XUI_OK) break;
            if (!node) { result = XUI_ERROR_UNSUPPORTED; break; }
            if (node->kind != XUI_DOC_FOOTNOTE) break;
            ordinary--;
        }
        /* Ordinary blocks retain source order. Their first and last
         * source-bearing entries determine the envelope; appended footnotes
         * need a separate scan because first-use order is unrelated to source. */
        for (i = 0; result == XUI_OK && i < ordinary; i++) {
            doc_node* node = doc_index_get(state->index,
                doc_seq_get_id(copy->children, i));
            doc_node_source_range range;
            if ((result = doc_inc_cancel(t)) != XUI_OK) break;
            if (!node) { result = XUI_ERROR_UNSUPPORTED; break; }
            doc_node_source_range_with_children(copy->children, node, &range);
            if (node->kind == XUI_DOC_FRONT_MATTER) continue;
            if (range.source_start != DOC_NONE) { copy->source_start = range.source_start; break; }
        }
        for (i = ordinary; result == XUI_OK && i; i--) {
            doc_node* node = doc_index_get(state->index,
                doc_seq_get_id(copy->children, i - 1));
            doc_node_source_range range;
            if ((result = doc_inc_cancel(t)) != XUI_OK) break;
            if (!node) { result = XUI_ERROR_UNSUPPORTED; break; }
            doc_node_source_range_with_children(copy->children, node, &range);
            if (node->kind == XUI_DOC_FRONT_MATTER) continue;
            if (range.source_end != DOC_NONE) { copy->source_end = range.source_end; break; }
        }
        for (i = ordinary; result == XUI_OK && i < count; i++) {
            doc_node* node = doc_index_get(state->index,
                doc_seq_get_id(copy->children, i));
            doc_node_source_range range;
            if ((result = doc_inc_cancel(t)) != XUI_OK) break;
            if (!node || node->kind != XUI_DOC_FOOTNOTE) {
                result = XUI_ERROR_UNSUPPORTED; break;
            }
            doc_node_source_range_with_children(copy->children, node, &range);
            if (range.source_start != DOC_NONE &&
                (copy->source_start == DOC_NONE || range.source_start < copy->source_start))
                copy->source_start = range.source_start;
            if (range.source_end != DOC_NONE &&
                (copy->source_end == DOC_NONE || range.source_end > copy->source_end))
                copy->source_end = range.source_end;
        }
        if (result == XUI_OK) result = doc_state_set(state, copy);
        if (result == XUI_OK) {
            if (old_end != end) state->block_shifts = 1;
            state->source_blocks_indexed = 0;
        }
    }
    doc_seq_release(item); doc_seq_release(children); doc_seq_release(shifted);
    doc_node_release(copy); return result;
}
/* Immediate patches carry the preceding source version. Prepare can apply
 * several patches before parsing, in which case the base tree/definitions
 * still describe t->base->source. All such patches must stay strictly within
 * the same definition body in their successive coordinate spaces. */
static int doc_inc_used_footnote_body(xui_document_transaction t, doc_state** out)
{
    doc_state *old = t->draft, *fragment = NULL, *state = NULL;
    doc_sequence* before_source = t->source_before_patch;
    const xui_doc_operation_t* op;
    xui_doc_reference_definition_t ref, expected;
    doc_inc_note_virtual_t virtual_note = {0};
    doc_node* root = doc_index_get(old->index, DOC_ROOT);
    doc_sequence *record = NULL, *next = NULL;
    uint64_t at, i, count, index = DOC_NONE, old_end, end;
    uint64_t total_removed = 0, total_added = 0;
    uint64_t ordinal, first, old_id, fresh_id, parsed_id, brackets = 0;
    int result = XUI_ERROR_UNSUPPORTED;
    /* Prepare can parse a batch after all its source patches were applied. */
    if (!before_source && t->parse_op_start == 0 && t->count)
        before_source = t->base->source;
    if (t->domain != XUI_DOC_SOURCE ||
        t->count <= t->parse_op_start || !before_source ||
        (t->source_before_patch && t->count != t->parse_op_start + 1) ||
        !old->markdown_footnotes || !root) return result;
    op = &t->ops[t->parse_op_start];
    if (op->iKind != XUI_DOC_OP_SOURCE || (!op->iOldLength && !op->iNewLength) ||
        op->iOffset > UINT64_MAX - op->iOldLength ||
        op->iOffset > UINT64_MAX - op->iNewLength) return result;
    count = doc_seq_size(old->references);
    if (!count || count % sizeof(ref)) return result;
    for (at = 0; at < count; at += sizeof(ref)) {
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read(old->references, at, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if (ref.iKind != XUI_DOC_REFERENCE_FOOTNOTE ||
            ref.iLabelEnd == DOC_NONE || ref.iLabelEnd > UINT64_MAX - 2 ||
            ref.iSourceEnd == DOC_NONE || ref.iSourceEnd <= ref.iLabelEnd + 2)
            continue;
        if (op->iOffset > ref.iLabelEnd + 2 &&
            op->iOffset + op->iOldLength < ref.iSourceEnd) {
            index = at / sizeof(ref); break;
        }
    }
    if (index == DOC_NONE) return XUI_ERROR_UNSUPPORTED;
    old_end = ref.iSourceEnd;
    end = old_end;
    for (i = t->parse_op_start; i < t->count; i++) {
        op = &t->ops[i];
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        if (op->iKind != XUI_DOC_OP_SOURCE ||
            (!op->iOldLength && !op->iNewLength) ||
            op->iOffset > UINT64_MAX - op->iOldLength ||
            op->iOffset > UINT64_MAX - op->iNewLength ||
            op->iOffset <= ref.iLabelEnd + 2 ||
            op->iOffset + op->iOldLength >= end ||
            end < op->iOldLength ||
            end - op->iOldLength > UINT64_MAX - op->iNewLength ||
            total_removed > UINT64_MAX - op->iOldLength ||
            total_added > UINT64_MAX - op->iNewLength)
            return XUI_ERROR_UNSUPPORTED;
        end = end - op->iOldLength + op->iNewLength;
        total_removed += op->iOldLength;
        total_added += op->iNewLength;
    }
    if (end > doc_seq_size(old->source) ||
        !doc_inc_boundaries(old->source, ref.iSourceStart, end, 0))
        return XUI_ERROR_UNSUPPORTED;
    result = doc_inc_note_location(t, old, &ref, &ordinal, &first);
    if (result != XUI_OK) return result;
    result = doc_inc_note_has_nested_use(t, before_source,
        ref.iLabelEnd + 2, old_end);
    if (result != XUI_OK) return result;
    result = doc_inc_note_has_nested_use(t, old->source,
        ref.iLabelEnd + 2, end);
    if (result != XUI_OK) return result;
    expected = ref; expected.iSourceEnd = end;
    result = doc_inc_definition_budget_safe(t, old, index, &expected,
        total_removed, total_added);
    if (result != XUI_OK) return result;
    result = doc_inc_note_virtual_build(t, old, &ref, index, end, &virtual_note);
    if (result != XUI_OK) goto done;
    for (at = ref.iSourceStart; at < end; at++) {
        if ((result = doc_inc_cancel(t)) != XUI_OK) goto done;
        if (doc_inc_byte(old->source, at) == '[') brackets++;
    }
    for (at = 0; at < virtual_note.bytes; at++)
        if (virtual_note.suffix[at] == '[') brackets++;
    result = doc_inc_link_budget_safe(t, old, end - ref.iSourceStart,
        virtual_note.bytes, brackets, 1);
    if (result != XUI_OK) goto done;
    result = doc_markdown_parse_window_with_suffix(t, ref.iSourceStart, end,
        virtual_note.suffix, virtual_note.bytes, &fragment);
    if (result != XUI_OK) {
        if (result != XUI_ERROR_OUT_OF_MEMORY && result != XUI_DOC_ERROR_CANCELLED &&
            result != XUI_DOC_ERROR_STALE) result = XUI_ERROR_UNSUPPORTED;
        goto done;
    }
    result = doc_inc_note_virtual_check(t, old, fragment, &expected,
        end, &virtual_note, &fresh_id);
    if (result != XUI_OK) goto done;
    for (at = 0; at < doc_seq_size(fragment->reference_candidates) /
        sizeof(xui_doc_inline_syntax_t); at++) {
        xui_doc_inline_syntax_t candidate;
        if ((result = doc_inc_cancel(t)) != XUI_OK) goto done;
        result = doc_seq_read_syntax(fragment->reference_candidates, at, &candidate);
        if (result != XUI_OK) goto done;
        if (candidate.iSourceEnd > end - ref.iSourceStart) {
            result = XUI_ERROR_UNSUPPORTED; goto done;
        }
    }
    old_id = doc_seq_get_id(root->children, ordinal);
    parsed_id = fresh_id;
    result = doc_inc_shift(t, fragment, fresh_id, 0, ref.iSourceStart);
    if (result != XUI_OK) goto done;
    state = doc_state_clone(old);
    if (!state) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    result = doc_inc_shift_definitions(t, state, old_end, end);
    if (result != XUI_OK) goto done;
    record = doc_seq_text(state->allocator, (const char*)&expected, sizeof(expected));
    if (!record) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    result = doc_seq_replace(state->allocator, state->references,
        index * sizeof(ref), (index + 1) * sizeof(ref), record, &next);
    if (result != XUI_OK) goto done;
    doc_seq_release(state->references); state->references = next; next = NULL;
    result = doc_inc_remove(t, state, old, old_id);
    if (result == XUI_OK)
        result = doc_markdown_reconcile_block(t, fragment, old_id, parsed_id,
            state, ordinal, doc_seq_source_shift(root->children, ordinal), &fresh_id);
    if (result == XUI_OK)
        result = doc_inc_note_syntax(t, old, state, fragment, &ref,
            end, ordinal, &virtual_note);
    if (result == XUI_OK)
        result = doc_inc_syntax_sequence(t, state, &state->reference_candidates,
            fragment->reference_candidates, ref.iSourceStart, old_end, end);
    if (result == XUI_OK)
        result = doc_inc_note_root(t, old, state, ordinal, first, fresh_id,
            old_end, end);
    if (result == XUI_OK) result = doc_inc_cancel(t);
    if (result == XUI_OK) {
        atomic_fetch_add(&old->allocator->markdown_incremental_parses, 1);
        *out = state; state = NULL;
    }
done:
    doc_seq_release(record); doc_seq_release(next);
    doc_state_release(state); doc_state_release(fragment);
    doc_inc_note_virtual_dispose(&virtual_note); return result;
}
/* A field edit in a standalone link definition preserves its physical order.
 * Reparse the definition, update source coordinates, and reparse all possible
 * dependent top-level blocks before publishing the new state. */
static int doc_inc_definition_field(xui_document_transaction t, doc_state** out)
{
    doc_state *old = t->draft, *fragment = NULL, *state = NULL;
    doc_node* root = doc_index_get(old->index, DOC_ROOT);
    const xui_doc_operation_t* op;
    xui_doc_reference_definition_t ref, parsed, expected;
    doc_inc_dependents_t dependents;
    char* suffix = NULL;
    uint64_t at, index = DOC_NONE, count;
    uint64_t old_value_end, new_value_end, new_ref_end, first_shift, suffix_bytes = 0;
    int result, label_edit = 0;
    if (t->domain != XUI_DOC_SOURCE || t->count != t->parse_op_start + 1 ||
        old->markdown_footnotes || !root) return XUI_ERROR_UNSUPPORTED;
    op = &t->ops[t->parse_op_start];
    if (op->iKind != XUI_DOC_OP_SOURCE || (!op->iOldLength && !op->iNewLength) ||
        op->iOffset > UINT64_MAX - op->iOldLength ||
        op->iOffset > UINT64_MAX - op->iNewLength) return XUI_ERROR_UNSUPPORTED;
    count = doc_seq_size(old->references);
    if (!count || count % sizeof(ref)) return XUI_ERROR_UNSUPPORTED;
    for (at = 0; at < count; at += sizeof(ref)) {
        if ((result = doc_inc_cancel(t)) != XUI_OK) return result;
        result = doc_seq_read(old->references, at, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if (ref.iKind != XUI_DOC_REFERENCE_LINK ||
            ref.iSourceStart == DOC_NONE || ref.iSourceEnd <= ref.iSourceStart) continue;
        label_edit = ref.iLabelStart != DOC_NONE &&
            (op->iOldLength ?
                op->iOffset >= ref.iLabelStart &&
                op->iOffset + op->iOldLength <= ref.iLabelEnd :
                op->iOffset >= ref.iLabelStart &&
                op->iOffset <= ref.iLabelEnd);
        if (label_edit ||
            ((ref.iDestinationStart != DOC_NONE &&
                (op->iOldLength ?
                    op->iOffset >= ref.iDestinationStart &&
                    op->iOffset + op->iOldLength <= ref.iDestinationEnd :
                    op->iOffset >= ref.iDestinationStart &&
                    op->iOffset <= ref.iDestinationEnd)) ||
             (ref.iTitleStart != DOC_NONE &&
                (op->iOldLength ?
                    op->iOffset >= ref.iTitleStart &&
                    op->iOffset + op->iOldLength <= ref.iTitleEnd :
                    op->iOffset >= ref.iTitleStart &&
                    op->iOffset <= ref.iTitleEnd)))) {
            index = at / sizeof(ref); break;
        }
    }
    if (index == DOC_NONE) return XUI_ERROR_UNSUPPORTED;
    old_value_end = op->iOffset + op->iOldLength;
    new_value_end = op->iOffset + op->iNewLength;
    if (ref.iSourceEnd < old_value_end ||
        ref.iSourceEnd - op->iOldLength > UINT64_MAX - op->iNewLength)
        return XUI_ERROR_UNSUPPORTED;
    new_ref_end = ref.iSourceEnd - op->iOldLength + op->iNewLength;
    if (new_ref_end > doc_seq_size(old->source) ||
        !doc_inc_boundaries(old->source, ref.iSourceStart, new_ref_end, 0))
        return XUI_ERROR_UNSUPPORTED;
    result = doc_inc_definition_first_shift(t, old, &ref, &first_shift);
    if (result != XUI_OK) return result;
    doc_inc_dependents_init(&dependents, old->allocator);
    if (!label_edit) {
        result = doc_inc_definition_value_blocks(t, old, index, &dependents);
        if (result != XUI_OK) goto done;
    }
    result = doc_markdown_parse_window(t, ref.iSourceStart, new_ref_end, &fragment);
    if (result != XUI_OK) {
        if (result != XUI_ERROR_OUT_OF_MEMORY && result != XUI_DOC_ERROR_CANCELLED &&
            result != XUI_DOC_ERROR_STALE) result = XUI_ERROR_UNSUPPORTED;
        goto done;
    }
    result = XUI_ERROR_UNSUPPORTED;
    if (fragment->markdown_footnotes || doc_seq_size(fragment->references) != sizeof(parsed) ||
        doc_seq_size(doc_index_get(fragment->index, DOC_ROOT)->children)) goto done;
    result = doc_seq_read(fragment->references, 0, &parsed, sizeof(parsed));
    if (result != XUI_OK) goto done;
    result = XUI_ERROR_UNSUPPORTED;
    expected = ref;
    expected.iSourceEnd = doc_inc_value_offset(ref.iSourceEnd, old_value_end, new_value_end);
    expected.iLabelStart = doc_inc_value_offset(ref.iLabelStart, old_value_end, new_value_end);
    expected.iLabelEnd = doc_inc_value_offset(ref.iLabelEnd, old_value_end, new_value_end);
    expected.iDestinationStart = doc_inc_value_offset(ref.iDestinationStart, old_value_end, new_value_end);
    expected.iDestinationEnd = doc_inc_value_offset(ref.iDestinationEnd, old_value_end, new_value_end);
    expected.iTitleStart = doc_inc_value_offset(ref.iTitleStart, old_value_end, new_value_end);
    expected.iTitleEnd = doc_inc_value_offset(ref.iTitleEnd, old_value_end, new_value_end);
    if (!op->iOldLength && op->iOffset == ref.iLabelStart)
        expected.iLabelStart = ref.iLabelStart;
    if (!op->iOldLength && op->iOffset == ref.iDestinationStart)
        expected.iDestinationStart = ref.iDestinationStart;
    if (!op->iOldLength && op->iOffset == ref.iTitleStart)
        expected.iTitleStart = ref.iTitleStart;
    if (parsed.iKind != expected.iKind || parsed.iSourceStart != 0 ||
        !doc_inc_virtual_field_equal(expected.iSourceEnd, ref.iSourceStart, parsed.iSourceEnd, 0) ||
        !doc_inc_virtual_field_equal(expected.iLabelStart, ref.iSourceStart, parsed.iLabelStart, 0) ||
        !doc_inc_virtual_field_equal(expected.iLabelEnd, ref.iSourceStart, parsed.iLabelEnd, 0) ||
        !doc_inc_virtual_field_equal(expected.iDestinationStart, ref.iSourceStart, parsed.iDestinationStart, 0) ||
        !doc_inc_virtual_field_equal(expected.iDestinationEnd, ref.iSourceStart, parsed.iDestinationEnd, 0) ||
        !doc_inc_virtual_field_equal(expected.iTitleStart, ref.iSourceStart, parsed.iTitleStart, 0) ||
        !doc_inc_virtual_field_equal(expected.iTitleEnd, ref.iSourceStart, parsed.iTitleEnd, 0)) goto done;
    if (new_ref_end != ref.iSourceEnd) {
        result = doc_inc_definition_budget_safe(t, old, index, &expected,
            op->iOldLength, op->iNewLength);
        if (result != XUI_OK) goto done;
    }
    state = doc_state_clone(old);
    if (!state) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    result = new_ref_end == ref.iSourceEnd ? XUI_OK :
        doc_inc_shift_definition_value(t, state, index, &expected,
            ref.iSourceEnd, new_ref_end, first_shift);
    if (result == XUI_OK) result = doc_reference_value_replace(state, index, fragment, 0);
    if (result == XUI_OK && label_edit)
        result = doc_inc_label_blocks(t, state, &dependents);
    if (result == XUI_OK && dependents.count)
        result = doc_inc_link_suffix(t, state, 0, 0, &suffix, &suffix_bytes);
    for (at = 0; result == XUI_OK && at < dependents.count; at++) {
        uint64_t dependent = dependents.items[at];
        doc_node* block = doc_index_get(old->index,
            doc_seq_get_id(root->children, dependent));
        result = doc_inc_reparse_changed_definition(t, old, state, dependent,
            block, suffix, suffix_bytes, at == 0);
    }
    if (result == XUI_OK) result = doc_inc_cancel(t);
    if (result == XUI_OK) {
        atomic_fetch_add(&old->allocator->markdown_incremental_parses, 1);
        *out = state; state = NULL;
    }
done:
    doc_state_release(state);
    doc_inc_dependents_dispose(&dependents);
    doc_free(suffix);
    doc_state_release(fragment); return result;
}
int doc_markdown_incremental(xui_document_transaction t, doc_state** out)
{
    doc_state *old = t->draft, *fragment = NULL, *state = NULL;
    char* suffix = NULL; uint64_t suffix_bytes = 0;
    doc_node *root = doc_index_get(old->index, DOC_ROOT), *block = NULL, *parsed_root, *fresh;
    uint64_t index, i, count = doc_seq_size(root->children), start, end, old_end, id = 0, brackets = 0;
    int64_t old_shift;
    doc_node_source_range block_range;
    int result = XUI_ERROR_UNSUPPORTED, needs_definitions = 0, fragment_indexed = 0;
    *out = NULL;
    /* An immediate VISUAL source patch retains its preceding SourceStore and
     * can use the same validated block reparse. Batched structural rewrites
     * have no single preceding source version and keep the full parse path. */
    if ((t->domain != XUI_DOC_SOURCE &&
            (t->domain != XUI_DOC_SEMANTIC || !t->source_before_patch ||
             t->count != t->parse_op_start + 1)) ||
        t->count <= t->parse_op_start) return result;
    result = doc_inc_unused_footnote_body(t, out);
    if (result != XUI_ERROR_UNSUPPORTED) return result;
    result = doc_inc_used_footnote_body(t, out);
    if (result != XUI_ERROR_UNSUPPORTED) return result;
    result = doc_inc_definition_field(t, out);
    if (result != XUI_ERROR_UNSUPPORTED) return result;
    if (count < 2) return result;
    result = doc_inc_find_block(t, old, t->ops[t->parse_op_start].iOffset, &index, &block);
    if (result != XUI_OK) return result;
    doc_node_source_range_get(old, block, &block_range);
    start = block_range.syntax_start; end = old_end = block_range.syntax_end;
    old_shift = doc_seq_source_shift(root->children, index);
    for (i = t->parse_op_start; i < t->count; i++) {
        const xui_doc_operation_t* op = &t->ops[i];
        if (op->iKind != XUI_DOC_OP_SOURCE || op->iOffset < start || op->iOffset > end || op->iOldLength > end - op->iOffset)
            return XUI_ERROR_UNSUPPORTED;
        end = end - op->iOldLength + op->iNewLength;
    }
    if (end <= start || end > doc_seq_size(old->source) ||
        !doc_inc_boundaries(old->source, start, end, doc_inc_context_kind(block->kind))) return XUI_ERROR_UNSUPPORTED;
    result = doc_inc_footnote_dependency_guard(t, old, start, old_end);
    if (result != XUI_OK) return result;
    result = doc_inc_definition_guard(t, old, start, old_end, end,
        &needs_definitions, &brackets);
    if (result != XUI_OK) return result;
    if (needs_definitions) {
        result = doc_inc_link_suffix(t, old, old_end, end, &suffix,
            &suffix_bytes);
        if (result != XUI_OK) return result;
        result = doc_inc_link_budget_safe(t, old, end - start,
            suffix_bytes, brackets, 1);
        if (result != XUI_OK) { doc_free(suffix); return result; }
    }
    result = doc_markdown_parse_window_with_suffix(t, start, end,
        suffix, suffix_bytes, &fragment);
    doc_free(suffix);
    if (result != XUI_OK) return result;
    parsed_root = doc_index_get(fragment->index, DOC_ROOT);
    if (doc_seq_size(parsed_root->children) != 1 ||
        (needs_definitions ? fragment->markdown_footnotes != old->markdown_footnotes :
            fragment->markdown_footnotes)) {
        result = XUI_ERROR_UNSUPPORTED; goto done;
    }
    if (needs_definitions)
        result = doc_inc_link_suffix_matches(t, old, fragment, end - start, 2);
    else if (fragment->references) result = XUI_ERROR_UNSUPPORTED;
    if (result != XUI_OK) goto done;
    fresh = doc_index_get(fragment->index, doc_seq_get_id(parsed_root->children, 0));
    if (!doc_inc_kind(fresh->kind) || fresh->syntax_start != 0 || fresh->syntax_end != end - start) { result = XUI_ERROR_UNSUPPORTED; goto done; }
    fragment_indexed = fragment->source_blocks_indexed;
    id = fresh->id;
    if (doc_inc_context_kind(block->kind) || doc_inc_context_kind(fresh->kind)) {
        result = doc_inc_left_context(t, old, index, start, end, fragment, id);
        if (result == XUI_OK) result = doc_inc_right_context(t, old, index, start, old_end, end, fragment, id);
        if (result != XUI_OK) goto done;
    }
    result = doc_inc_shift(t, fragment, id, 0, start);
    if (result != XUI_OK) goto done;
    state = doc_state_clone(old);
    if (!state) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    result = doc_inc_shift_definitions(t, state, old_end, end);
    if (result == XUI_OK) result = doc_inc_remove(t, state, old, block->id);
    if (result == XUI_OK) result = doc_markdown_reconcile_block(t, fragment, block->id, id,
        state, index, old_shift, &id);
    if (result == XUI_OK) result = doc_inc_syntax_sequence(t, state, &state->inline_syntax,
        fragment->inline_syntax, start, old_end, end);
    if (result == XUI_OK) result = doc_inc_syntax_sequence(t, state, &state->reference_candidates,
        fragment->reference_candidates, start, old_end, end);
    if (result == XUI_OK) result = doc_inc_root(state, index, id, old_end, end, old_shift,
        old->source_blocks_indexed && fragment_indexed);
    if (result == XUI_OK) result = doc_inc_cancel(t);
    if (result == XUI_OK) {
        atomic_fetch_add(&old->allocator->markdown_incremental_parses, 1);
        *out = state; state = NULL;
    }
done:
    doc_state_release(state); doc_state_release(fragment); return result;
}

#endif
