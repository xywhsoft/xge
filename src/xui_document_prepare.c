#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"

/* A detached transaction is a builder over retained immutable inputs. It has
 * no live Document pointer, observers, saved state or private undo history. */
struct xui_doc_prepare_t {
    atomic_uint refs;
    atomic_int state, cancellation, result;
    atomic_uint_fast64_t committed_revision;
    doc_allocator* allocator;
    struct xui_doc_transaction_t transaction;
    doc_sequence *source, *operations; /* Immutable input, even while Run mutates the builder. */
    doc_state* visual_expected; /* The provisional VISUAL tree must match the parsed result. */
    doc_sequence *visual_edits, *visual_text; /* Persistent semantic/source edit map. */
    uint64_t visual_node, visual_parent, visual_caret_text, visual_caret_source;
    int visual_delete_chain; /* Remaining prefix/suffix still use base source positions. */
    uint64_t visual_delete_left, visual_delete_right;
    doc_publish_plan* publication;
    uint64_t identity, generation, patch_count, source_open_brackets;
    uint64_t previous_generation, patch_start, patch_end, patch_bytes;
    int single_patch; /* Immutable delta from the exact preceding projection. */
    unsigned utf8_bytes;
    char utf8[3]; /* A validated, incomplete final code point; never source/tree. */
};
static atomic_uint_fast64_t doc_prepare_sequence = 0;
static uint64_t doc_prepare_generation(void)
{
    uint_fast64_t value = atomic_load(&doc_prepare_sequence);
    do { if (value == UINT64_MAX) return 0; } while (!atomic_compare_exchange_weak(&doc_prepare_sequence, &value, value + 1));
    return value + 1;
}

XUI_API void xuiDocumentPrepareRetain(xui_document_prepare p)
{
    if (p) atomic_fetch_add(&p->refs, 1);
}
XUI_API void xuiDocumentPrepareRelease(xui_document_prepare p)
{
    doc_allocator* a;
    if (!p || atomic_fetch_sub(&p->refs, 1) != 1) return;
    a = p->allocator;
    doc_publish_plan_release(p->publication);
    doc_state_release(p->transaction.base); doc_state_release(p->transaction.draft);
    doc_state_release(p->visual_expected);
    doc_free(p->transaction.ops); doc_seq_release(p->source); doc_seq_release(p->operations);
    doc_seq_release(p->visual_edits); doc_seq_release(p->visual_text);
    doc_free(p); doc_allocator_release(a);
}
static void doc_prepare_cancel(xui_document_prepare p, int reason)
{
    int previous = 0;
    if (p) (void)atomic_compare_exchange_strong(&p->cancellation, &previous, reason);
}
XUI_API void xuiDocumentPrepareCancel(xui_document_prepare p)
{
    doc_prepare_cancel(p, XUI_DOC_ERROR_CANCELLED);
}
void doc_prepare_invalidate(xui_document d, xui_document_prepare except, int reason)
{
    xui_document_prepare p = d->pending;
    d->pending = NULL;
    if (p != except) doc_prepare_cancel(p, reason);
    xuiDocumentPrepareRelease(p);
}
XUI_API int xuiDocumentCancelPrepare(xui_document d)
{
    if (!d) return XUI_ERROR_INVALID_ARGUMENT;
    if (d->writer || d->notifying) return XUI_DOC_ERROR_BUSY;
    doc_prepare_invalidate(d, NULL, XUI_DOC_ERROR_CANCELLED); return XUI_OK;
}
XUI_API int xuiDocumentHasPrepare(xui_document d) { return d && d->pending; }

static int doc_prepare_source(xui_document d, xui_document_prepare previous, const xui_doc_txn_desc_t* desc,
    const xui_doc_source_patch_t* patches, uint64_t count, const char* utf8, unsigned utf8_bytes, int attach, xui_document_prepare* out)
{
    xui_document_prepare p;
    xui_document_transaction t;
    uint64_t i;
    int result = XUI_OK;
    if (!out) return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    if (!d || (!patches && count) || count > SIZE_MAX / sizeof(*patches) ||
        (desc && (desc->iSize != sizeof(*desc) || desc->iDomain != XUI_DOC_SOURCE))) return XUI_ERROR_INVALID_ARGUMENT;
    if (d->writer || d->notifying) return XUI_DOC_ERROR_BUSY;
    if (d->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    if (desc && desc->iBaseRevision && desc->iBaseRevision != d->revision) return XUI_DOC_ERROR_STALE;
    if (previous) {
        if (d->pending != previous || previous->identity != d->identity ||
            previous->transaction.base_revision != d->revision) return XUI_DOC_ERROR_STALE;
        result = atomic_load(&previous->cancellation);
        if (result) return result;
        if (previous->utf8_bytes && !utf8) return XUI_DOC_ERROR_BUSY;
        if (count > UINT64_MAX - previous->patch_count) return XUI_DOC_ERROR_LIMIT;
    }
    if (d->next_prepare == UINT64_MAX) return XUI_DOC_ERROR_LIMIT;
    p = doc_alloc(d->allocator, sizeof(*p));
    if (!p) return XUI_ERROR_OUT_OF_MEMORY;
    atomic_init(&p->refs, 1); atomic_init(&p->state, XUI_DOC_PREPARE_QUEUED);
    atomic_init(&p->cancellation, 0); atomic_init(&p->result, XUI_DOC_ERROR_BUSY);
    atomic_init(&p->committed_revision, 0);
    p->allocator = d->allocator; doc_allocator_retain(p->allocator);
    p->identity = d->identity; p->patch_count = count;
    p->previous_generation = previous ? previous->generation : 0;
    p->single_patch = count <= 1;
    if (count == 1) { p->patch_start = patches[0].iStart; p->patch_end = patches[0].iEnd; p->patch_bytes = patches[0].iTextBytes; }
    p->utf8_bytes = utf8_bytes;
    if (utf8_bytes) memcpy(p->utf8, utf8, utf8_bytes);
    t = &p->transaction;
    t->base = d->state; doc_state_retain(t->base); t->base_revision = d->revision;
    t->draft = doc_state_clone(d->state);
    if (!t->draft) { xuiDocumentPrepareRelease(p); return XUI_ERROR_OUT_OF_MEMORY; }
    t->domain = XUI_DOC_SOURCE; t->origin = desc ? desc->iOrigin : 0; t->group = desc ? desc->iGroup : 0;
    t->cancellation = &p->cancellation;
    if (previous) {
        /* Never read the running builder's draft, ops, count or error. The
         * source and packed operation log remain immutable for its lifetime.
         * Reconcile the entire input chain against the committed semantic base. */
        doc_seq_release(t->draft->source); t->draft->source = previous->source; doc_seq_retain(t->draft->source);
        t->draft->source_open_brackets = previous->source_open_brackets;
        t->origin = previous->transaction.origin; t->group = previous->transaction.group;
        t->flags = previous->transaction.flags;
        p->patch_count += previous->patch_count;
        p->operations = previous->operations; doc_seq_retain(p->operations);
        if (!count && previous->visual_expected) {
            p->visual_expected = previous->visual_expected; doc_state_retain(p->visual_expected);
            p->visual_edits = previous->visual_edits; doc_seq_retain(p->visual_edits);
            p->visual_text = previous->visual_text; doc_seq_retain(p->visual_text);
            p->visual_node = previous->visual_node;
            p->visual_parent = previous->visual_parent;
            p->visual_caret_text = previous->visual_caret_text;
            p->visual_caret_source = previous->visual_caret_source;
        }
        p->visual_delete_chain = previous->visual_delete_chain;
        p->visual_delete_left = previous->visual_delete_left;
        p->visual_delete_right = previous->visual_delete_right;
    }
    for (i = 0; i < count && result == XUI_OK; i++) {
        if (patches[i].iSize != sizeof(patches[i])) result = XUI_ERROR_INVALID_ARGUMENT;
        else result = doc_txn_source_patch(t, patches[i].iStart, patches[i].iEnd, patches[i].sText, patches[i].iTextBytes, 0);
    }
    if (result == XUI_OK && t->count) {
        doc_sequence *insert = NULL, *operations = NULL;
        uint64_t old_bytes = doc_seq_size(p->operations), bytes = t->count * sizeof(*t->ops);
        if (bytes > SIZE_MAX - old_bytes) result = XUI_DOC_ERROR_LIMIT;
        else {
            insert = doc_seq_text(p->allocator, (const char*)t->ops, bytes);
            result = insert ? doc_seq_replace(p->allocator, p->operations, old_bytes, old_bytes, insert, &operations) : XUI_ERROR_OUT_OF_MEMORY;
        }
        doc_seq_release(insert);
        if (result == XUI_OK) { doc_seq_release(p->operations); p->operations = operations; }
    }
    if (result != XUI_OK) { xuiDocumentPrepareRelease(p); return result; }
    /* Flatten only on the worker. Extending a long chain copies persistent
     * sequence paths, not all preceding operations or the complete source. */
    doc_free(t->ops); t->ops = NULL; t->count = t->capacity = 0;
    if (p->operations && attach) {
        p->publication = doc_publish_plan_capture(d);
        if (!p->publication) { xuiDocumentPrepareRelease(p); return XUI_ERROR_OUT_OF_MEMORY; }
    }
    p->source = t->draft->source; doc_seq_retain(p->source);
    p->source_open_brackets = t->draft->source_open_brackets;
    p->generation = doc_prepare_generation();
    if (!p->generation) { xuiDocumentPrepareRelease(p); return XUI_DOC_ERROR_LIMIT; }
    /* Failed creation cannot supersede the current candidate. */
    if (attach) {
        d->next_prepare = p->generation; doc_prepare_invalidate(d, NULL, XUI_DOC_ERROR_STALE);
        d->pending = p; xuiDocumentPrepareRetain(p);
    }
    *out = p;
    return XUI_OK;
}
XUI_API int xuiDocumentPrepareSource(xui_document d, const xui_doc_txn_desc_t* desc,
    const xui_doc_source_patch_t* patches, uint64_t count, xui_document_prepare* out)
{
    return doc_prepare_source(d, NULL, desc, patches, count, NULL, 0, 1, out);
}
static int doc_prepare_visual_escape(doc_allocator* allocator, const char* text,
    uint64_t bytes, char** escaped, uint64_t* used)
{
    uint64_t i;
    if (bytes > (SIZE_MAX - 1) / 2) return XUI_DOC_ERROR_LIMIT;
    *escaped = doc_alloc(allocator, (size_t)bytes * 2 + 1);
    if (!*escaped) return XUI_ERROR_OUT_OF_MEMORY;
    *used = 0;
    for (i = 0; i < bytes; i++) {
        unsigned char c = (unsigned char)text[i];
        if ((c >= 33 && c <= 47) || (c >= 58 && c <= 64) ||
            (c >= 91 && c <= 96) || (c >= 123 && c <= 126)) (*escaped)[(*used)++] = '\\';
        (*escaped)[(*used)++] = text[i];
    }
    return XUI_OK;
}
static int doc_prepare_visual_same_marked_text(doc_node* first, doc_node* other)
{
    const uint32_t allowed = XUI_DOC_BOLD | XUI_DOC_ITALIC | XUI_DOC_STRIKE |
        XUI_DOC_HIGHLIGHT | XUI_DOC_SUBSCRIPT | XUI_DOC_SUPERSCRIPT |
        XUI_DOC_LINK;
    return first && other && first->kind == XUI_DOC_TEXT &&
        other->kind == XUI_DOC_TEXT && first->attrs->iMarks &&
        !(first->attrs->iMarks & ~allowed) &&
        doc_attributes_equal(first->attrs, other->attrs) &&
        !strcmp(doc_string(first->resource), doc_string(other->resource)) &&
        !strcmp(doc_string(first->info), doc_string(other->info)) &&
        !strcmp(doc_string(first->title), doc_string(other->title));
}
/* A provisional marked range, including a link label, must not erase a
 * delimiter or a source-only inline construct. The worker still checks the
 * full semantics, including unchanged link targets. */
static int doc_prepare_visual_marked_source_safe(doc_sequence* source,
    uint64_t from, uint64_t to)
{
    uint64_t at = from;
    while (at < to) {
        unsigned char block[256]; uint64_t n = to - at, i;
        int result;
        if (n > sizeof(block)) n = sizeof(block);
        result = doc_seq_read(source, at, block, n);
        if (result != XUI_OK) return result;
        for (i = 0; i < n; i++) {
            unsigned char c = block[i];
            if (c == '*' || c == '_' || c == '~' || c == '=' || c == '^' ||
                c == '`' || c == '[' || c == ']' || c == '!' || c == '<' ||
                c == '>' || c == '$' || c == '\\' || c == '|')
                return XUI_ERROR_UNSUPPORTED;
        }
        at += n;
    }
    return XUI_OK;
}
/* A cross-paragraph preview may erase the blank line between two blocks.
 * Inspect both complete source paragraphs and their gap, so punctuation
 * outside the selected endpoints cannot gain Markdown meaning at the join. */
static int doc_prepare_visual_plain_block_source_safe(doc_sequence* source,
    uint64_t from, uint64_t to)
{
    uint64_t at = from; unsigned newlines = 0;
    if (from > to || to - from > 65536) return XUI_ERROR_UNSUPPORTED;
    while (at < to) {
        unsigned char block[256]; uint64_t n = to - at, i;
        int result;
        if (n > sizeof(block)) n = sizeof(block);
        result = doc_seq_read(source, at, block, n);
        if (result != XUI_OK) return result;
        for (i = 0; i < n; i++) {
            unsigned char c = block[i];
            if (c == '\n') newlines++;
            else if (c < 128 &&
                !(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') &&
                !(c >= '0' && c <= '9') && c != ' ' && c != '\t' && c != '\r')
                return XUI_ERROR_UNSUPPORTED;
        }
        at += n;
    }
    return newlines >= 2 ? XUI_OK : XUI_ERROR_UNSUPPORTED;
}
typedef struct doc_visual_edit_t {
    uint64_t text_start, text_old, text_new;
    uint64_t source_start, source_old, source_new;
    uint64_t raw_start;
} doc_visual_edit_t;
static int doc_prepare_visual_append(xui_document_prepare p, xui_document_prepare previous,
    uint64_t id, uint64_t start, uint64_t end, const char* text, uint64_t bytes,
    uint64_t from, uint64_t to, uint64_t used)
{
    doc_sequence *raw = NULL, *data = NULL, *record = NULL, *edits = NULL;
    doc_sequence* old_data = previous ? previous->visual_text : NULL;
    doc_sequence* old_edits = previous ? previous->visual_edits : NULL;
    doc_visual_edit_t entry = {0}; int result = XUI_OK;
    if (!p || (previous && previous->visual_node != id) || start > end ||
        from > to || start > UINT64_MAX - bytes || from > UINT64_MAX - used)
        return XUI_ERROR_INVALID_ARGUMENT;
    entry.text_start = start; entry.text_old = end - start; entry.text_new = bytes;
    entry.source_start = from; entry.source_old = to - from;
    entry.source_new = used; entry.raw_start = doc_seq_size(old_data);
    if (bytes) {
        raw = doc_seq_text(p->allocator, text, bytes);
        if (!raw) return XUI_ERROR_OUT_OF_MEMORY;
        result = doc_seq_replace(p->allocator, old_data, entry.raw_start,
            entry.raw_start, raw, &data);
    } else { data = old_data; doc_seq_retain(data); }
    doc_seq_release(raw);
    if (result != XUI_OK) goto done;
    record = doc_seq_text(p->allocator, (const char*)&entry, sizeof(entry));
    if (!record) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    result = doc_seq_replace(p->allocator, old_edits, doc_seq_size(old_edits),
        doc_seq_size(old_edits), record, &edits);
    if (result == XUI_OK) {
        p->visual_text = data; data = NULL;
        p->visual_edits = edits; edits = NULL;
        p->visual_node = id;
        p->visual_caret_text = start + bytes;
        p->visual_caret_source = from + used;
    }
done:
    doc_seq_release(record); doc_seq_release(data); doc_seq_release(edits);
    return result;
}
static int doc_prepare_visual_prefix(xui_document_prepare p, const doc_visual_edit_t* edit,
    uint64_t offset, uint64_t* source_bytes)
{
    unsigned char block[256]; uint64_t at = 0, escaped = 0;
    if (offset > edit->text_new || edit->raw_start > doc_seq_size(p->visual_text) ||
        offset > doc_seq_size(p->visual_text) - edit->raw_start ||
        !doc_seq_boundary(p->visual_text, edit->raw_start + offset))
        return XUI_ERROR_INVALID_ARGUMENT;
    while (at < offset) {
        uint64_t n = offset - at < sizeof(block) ? offset - at : sizeof(block), i;
        if (doc_seq_read(p->visual_text, edit->raw_start + at, block, n) != XUI_OK)
            return XUI_ERROR_INVALID_STATE;
        for (i = 0; i < n; i++) {
            unsigned char c = block[i];
            if ((c >= 33 && c <= 47) || (c >= 58 && c <= 64) ||
                (c >= 91 && c <= 96) || (c >= 123 && c <= 126)) escaped++;
        }
        at += n;
    }
    if (offset > UINT64_MAX - escaped) return XUI_DOC_ERROR_LIMIT;
    *source_bytes = offset + escaped;
    return XUI_OK;
}
static int doc_prepare_visual_shift(uint64_t value, uint64_t added, uint64_t removed,
    uint64_t* out)
{
    if (added >= removed) {
        if (value > UINT64_MAX - (added - removed)) return XUI_DOC_ERROR_LIMIT;
        *out = value + added - removed;
    } else {
        if (value < removed - added) return XUI_DOC_ERROR_LIMIT;
        *out = value - (removed - added);
    }
    return XUI_OK;
}
static int doc_prepare_visual_map(xui_document d, xui_document_prepare p,
    uint64_t id, uint64_t offset, unsigned affinity, uint64_t* out)
{
    uint64_t count, added = 0, removed = 0, value = 0;
    doc_node* base; int mapping, result;
    if (!p || id != p->visual_node || !p->visual_edits || !out)
        return XUI_ERROR_UNSUPPORTED;
    count = doc_seq_size(p->visual_edits) / sizeof(doc_visual_edit_t);
    while (count) {
        doc_visual_edit_t edit; uint64_t end;
        if (doc_seq_read(p->visual_edits, --count * sizeof(edit), &edit,
                sizeof(edit)) != XUI_OK || edit.text_start > UINT64_MAX - edit.text_new)
            return XUI_ERROR_INVALID_STATE;
        end = edit.text_start + edit.text_new;
        if (offset < edit.text_start ||
            (offset == edit.text_start && affinity == XUI_DOC_BEFORE)) continue;
        if (offset > end || (offset == end && affinity == XUI_DOC_AFTER)) {
            if (offset - edit.text_new > UINT64_MAX - edit.text_old)
                return XUI_DOC_ERROR_LIMIT;
            offset = offset - edit.text_new + edit.text_old;
            if (edit.source_new >= edit.source_old) {
                uint64_t change = edit.source_new - edit.source_old;
                if (added > UINT64_MAX - change) return XUI_DOC_ERROR_LIMIT;
                added += change;
            } else {
                uint64_t change = edit.source_old - edit.source_new;
                if (removed > UINT64_MAX - change) return XUI_DOC_ERROR_LIMIT;
                removed += change;
            }
            continue;
        }
        result = doc_prepare_visual_prefix(p, &edit, offset - edit.text_start, &value);
        if (result != XUI_OK) return result;
        if (edit.source_start > UINT64_MAX - value) return XUI_DOC_ERROR_LIMIT;
        value += edit.source_start;
        return doc_prepare_visual_shift(value, added, removed, out);
    }
    base = doc_index_get(d->state->index, id);
    if (!base || doc_source_map_text(d->state, base, offset, affinity,
            &value, &mapping) != XUI_OK || mapping != XUI_DOC_MAP_EXACT)
        return XUI_ERROR_UNSUPPORTED;
    return doc_prepare_visual_shift(value, added, removed, out);
}
int doc_prepare_visual_position_source(xui_document d, xui_document_prepare p,
    const xui_doc_position_t* position, uint64_t* out)
{
    doc_node *node, *base; uint64_t value, count, i; int mapping, result;
    if (!d || !p || !position || !out || !p->visual_expected ||
        position->iKind != XUI_DOC_POSITION_TEXT ||
        position->iDocumentId != d->identity || position->iRevision != d->revision)
        return XUI_ERROR_INVALID_ARGUMENT;
    node = doc_index_get(p->visual_expected->index, position->iNodeId);
    if (!node || node->kind != XUI_DOC_TEXT ||
        !doc_seq_boundary(node->text, position->iOffset)) return XUI_ERROR_UNSUPPORTED;
    if (position->iNodeId == p->visual_node) {
        if (position->iOffset == p->visual_caret_text &&
            position->iAffinity == XUI_DOC_AFTER) {
            *out = p->visual_caret_source; return XUI_OK;
        }
        if (p->visual_parent) return XUI_ERROR_UNSUPPORTED;
        return doc_prepare_visual_map(d, p, position->iNodeId,
            position->iOffset, position->iAffinity, out);
    }
    /* A provisional replacement can remove several leaves or a whole second
     * paragraph. Their old offsets no longer identify the new text. Only
     * independent leaves outside the surviving paragraph can be shifted
     * through recorded source patches without reparsing the preview. */
    if (p->visual_parent && node->parent == p->visual_parent)
        return XUI_ERROR_UNSUPPORTED;
    base = doc_index_get(d->state->index, position->iNodeId);
    if (!base || base->kind != XUI_DOC_TEXT ||
        doc_source_map_text(d->state, base, position->iOffset,
            position->iAffinity, &value, &mapping) != XUI_OK ||
        mapping != XUI_DOC_MAP_EXACT) return XUI_ERROR_UNSUPPORTED;
    count = doc_seq_size(p->visual_edits) / sizeof(doc_visual_edit_t);
    for (i = 0; i < count; i++) {
        doc_visual_edit_t edit; uint64_t end;
        if (doc_seq_read(p->visual_edits, i * sizeof(edit), &edit,
                sizeof(edit)) != XUI_OK ||
            edit.source_start > UINT64_MAX - edit.source_old)
            return XUI_ERROR_INVALID_STATE;
        end = edit.source_start + edit.source_old;
        if (value < edit.source_start ||
            (value == edit.source_start && position->iAffinity == XUI_DOC_BEFORE))
            continue;
        if (value > end || (value == end && position->iAffinity == XUI_DOC_AFTER)) {
            result = doc_prepare_visual_shift(value, edit.source_new,
                edit.source_old, &value);
            if (result != XUI_OK) return result;
        } else return XUI_ERROR_UNSUPPORTED;
    }
    if (value > doc_seq_size(p->source)) return XUI_ERROR_INVALID_STATE;
    *out = value; return XUI_OK;
}
/* A VISUAL text edit can be projected without running MD4C when both ends
 * map exactly inside one text leaf. Marked text additionally requires a
 * delimiter-free source interval; the authoritative source patch is still
 * parsed and checked on the worker. */
int doc_prepare_visual_text(xui_document d, const xui_doc_txn_desc_t* desc,
    uint64_t id, uint64_t start, uint64_t end, const char* text, uint64_t bytes,
    xui_document_prepare* out, xui_document_snapshot* preview, uint64_t* source_caret)
{
    struct xui_doc_transaction_t shadow = {0};
    xui_doc_source_patch_t patch = {0}; doc_node* node;
    char* escaped = NULL; uint64_t from, to, used = 0;
    int left, right, result;
    if (!out || !preview || !source_caret) return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL; *preview = NULL;
    if (!d || !desc || desc->iSize != sizeof(*desc) || desc->iDomain != XUI_DOC_SOURCE ||
        (!text && bytes) || bytes > (SIZE_MAX - 1) / 2 || start > end || start > UINT64_MAX - bytes)
        return XUI_ERROR_INVALID_ARGUMENT;
    if (d->state->profile != XUI_DOCUMENT_MARKDOWN || d->pending || d->writer || d->notifying ||
        (bytes && memchr(text, '\n', (size_t)bytes)) || (bytes && memchr(text, '\r', (size_t)bytes)))
        return XUI_ERROR_UNSUPPORTED;
    node = doc_index_get(d->state->index, id);
    if (!node || node->kind != XUI_DOC_TEXT || (node->attrs->iMarks & XUI_DOC_CODE) ||
        end > doc_seq_size(node->text)) return XUI_ERROR_UNSUPPORTED;
    {
        doc_node* parent = doc_index_get(d->state->index, node->parent);
        if (!parent || (parent->kind != XUI_DOC_PARAGRAPH && parent->kind != XUI_DOC_HEADING) ||
            (node->attrs->iMarks && !doc_prepare_visual_same_marked_text(node, node)) ||
            !start || end == doc_seq_size(node->text)) return XUI_ERROR_UNSUPPORTED;
    }
    if (doc_source_map_text(d->state, node, start, XUI_DOC_AFTER, &from, &left) != XUI_OK ||
        left != XUI_DOC_MAP_EXACT ||
        doc_source_map_text(d->state, node, end, XUI_DOC_BEFORE, &to, &right) != XUI_OK ||
        right != XUI_DOC_MAP_EXACT || (start != end && from > to)) return XUI_ERROR_UNSUPPORTED;
    if (start == end) to = from;
    if (node->attrs->iMarks) {
        result = doc_prepare_visual_marked_source_safe(d->state->source, from, to);
        if (result != XUI_OK) return result;
    }
    shadow.document = d; shadow.base_revision = d->revision; shadow.domain = XUI_DOC_SEMANTIC;
    shadow.parsing = DOC_BUILD_SEMANTIC; shadow.draft = doc_state_clone(d->state);
    if (!shadow.draft) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_txn_text(&shadow, id, start, end, text, bytes);
    if (result != XUI_OK) goto done;
    if (!shadow.count) { result = XUI_ERROR_UNSUPPORTED; goto done; }
    result = doc_prepare_visual_escape(d->allocator, text, bytes, &escaped, &used);
    if (result != XUI_OK) goto done;
    patch.iSize = sizeof(patch); patch.iStart = from; patch.iEnd = to;
    patch.sText = escaped; patch.iTextBytes = used;
    result = doc_snapshot_create(shadow.draft, d->identity, d->revision, preview);
    if (result != XUI_OK) goto done;
    result = doc_prepare_source(d, NULL, desc, &patch, 1, NULL, 0, 0, out);
    if (result == XUI_OK) {
        result = doc_prepare_visual_append(*out, NULL, id, start, end,
            text, bytes, from, to, used);
        if (result == XUI_OK) {
            doc_seq_release(shadow.draft->source); shadow.draft->source = (*out)->source;
            doc_seq_retain(shadow.draft->source);
            shadow.draft->source_open_brackets = (*out)->source_open_brackets;
            (*out)->visual_expected = shadow.draft; doc_state_retain(shadow.draft);
            (*out)->visual_delete_chain = !bytes;
            (*out)->visual_delete_left = start;
            (*out)->visual_delete_right = end;
            *source_caret = from + used;
            (*out)->publication = doc_publish_plan_capture(d);
            if (!(*out)->publication) result = XUI_ERROR_OUT_OF_MEMORY;
        }
        if (result != XUI_OK) {
            xuiDocumentPrepareRelease(*out); *out = NULL;
            xuiDocumentSnapshotRelease(*preview); *preview = NULL;
        }
    } else { xuiDocumentSnapshotRelease(*preview); *preview = NULL; }
done:
    doc_free(escaped); doc_free(shadow.ops); doc_state_release(shadow.draft);
    return result;
}
/* Replacing across plain text/soft-break children, same-style marked text or
 * adjacent plain root paragraphs can be previewed by a semantic transaction.
 * Marked ranges and cross-paragraph source have separate lexical guards; the
 * worker still validates the full parse. Affected leaves use only the exact
 * current caret for subsequent same-group insertions. */
int doc_prepare_visual_span(xui_document d, const xui_doc_txn_desc_t* desc,
    const xui_doc_range_t* range, const char* text, uint64_t bytes,
    xui_document_prepare* out, xui_document_snapshot* preview,
    xui_doc_position_t* caret, uint64_t* source_caret)
{
    struct xui_doc_transaction_t shadow = {0};
    xui_doc_source_patch_t patch = {0}; xui_doc_range_t ordered;
    xui_doc_position_t a, b, target = {0};
    doc_node *left, *right, *parent; char* escaped = NULL;
    uint64_t ai, bi, i, from, to, used = 0, safe_from = 0, safe_to = 0;
    int order, lmap, rmap, marked, cross_paragraph = 0, result;
    if (!out || !preview || !caret || !source_caret) return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL; *preview = NULL;
    if (!d || !desc || desc->iSize != sizeof(*desc) || desc->iDomain != XUI_DOC_SOURCE ||
        !range || (!text && bytes) || bytes > (SIZE_MAX - 1) / 2 ||
        (bytes && (memchr(text, '\n', (size_t)bytes) || memchr(text, '\r', (size_t)bytes))))
        return XUI_ERROR_INVALID_ARGUMENT;
    if (d->state->profile != XUI_DOCUMENT_MARKDOWN || d->pending || d->writer || d->notifying)
        return XUI_ERROR_UNSUPPORTED;
    a = range->tAnchor; b = range->tCaret;
    if (a.iKind != XUI_DOC_POSITION_TEXT || b.iKind != XUI_DOC_POSITION_TEXT ||
        a.iNodeId == b.iNodeId || a.iDocumentId != d->identity ||
        b.iDocumentId != d->identity || a.iRevision != d->revision ||
        b.iRevision != d->revision || a.iInputGeneration || b.iInputGeneration ||
        doc_position_compare(d->state, &a, &b, &order) != XUI_OK)
        return XUI_ERROR_UNSUPPORTED;
    if (order > 0) { xui_doc_position_t swap = a; a = b; b = swap; }
    left = doc_index_get(d->state->index, a.iNodeId);
    right = doc_index_get(d->state->index, b.iNodeId);
    parent = left ? doc_index_get(d->state->index, left->parent) : NULL;
    if (!left || !right || !parent || left->kind != XUI_DOC_TEXT ||
        right->kind != XUI_DOC_TEXT ||
        !a.iOffset || b.iOffset >= doc_seq_size(right->text) ||
        !doc_seq_boundary(left->text, a.iOffset) ||
        !doc_seq_boundary(right->text, b.iOffset)) return XUI_ERROR_UNSUPPORTED;
    if (left->parent != right->parent) {
        doc_node* other = doc_index_get(d->state->index, right->parent);
        doc_node* root = doc_index_get(d->state->index, DOC_ROOT);
        doc_node_source_range left_source, right_source;
        uint64_t left_block, right_block;
        if (!other || !root || parent->kind != XUI_DOC_PARAGRAPH ||
            other->kind != XUI_DOC_PARAGRAPH ||
            parent->parent != DOC_ROOT || other->parent != DOC_ROOT ||
            doc_seq_size(parent->children) != 1 ||
            doc_seq_size(other->children) != 1 ||
            left->attrs->iMarks || right->attrs->iMarks ||
            !doc_attributes_equal(left->attrs, right->attrs))
            return XUI_ERROR_UNSUPPORTED;
        left_block = doc_child_index(root, parent->id);
        right_block = doc_child_index(root, other->id);
        if (left_block == DOC_NONE || right_block == DOC_NONE ||
            right_block <= left_block || right_block - left_block != 1)
            return XUI_ERROR_UNSUPPORTED;
        doc_node_source_range_get(d->state, parent, &left_source);
        doc_node_source_range_get(d->state, other, &right_source);
        if (left_source.source_start == DOC_NONE ||
            left_source.source_end == DOC_NONE ||
            right_source.source_start == DOC_NONE ||
            right_source.source_end == DOC_NONE ||
            left_source.source_start >= left_source.source_end ||
            left_source.source_end > right_source.source_start ||
            right_source.source_start >= right_source.source_end ||
            right_source.source_end > doc_seq_size(d->state->source))
            return XUI_ERROR_UNSUPPORTED;
        safe_from = left_source.source_start;
        safe_to = right_source.source_end;
        cross_paragraph = 1;
        marked = 0;
    } else {
        if (parent->kind != XUI_DOC_PARAGRAPH && parent->kind != XUI_DOC_HEADING)
            return XUI_ERROR_UNSUPPORTED;
        marked = !!(left->attrs->iMarks | right->attrs->iMarks);
        if (marked && !doc_prepare_visual_same_marked_text(left, right))
            return XUI_ERROR_UNSUPPORTED;
        ai = doc_child_index(parent, a.iNodeId); bi = doc_child_index(parent, b.iNodeId);
        if (ai == DOC_NONE || bi == DOC_NONE || ai >= bi) return XUI_ERROR_UNSUPPORTED;
        for (i = ai + 1; i < bi; i++) {
            doc_node* middle = doc_index_get(d->state->index, doc_seq_get_id(parent->children, i));
            if (!middle || (marked ? !doc_prepare_visual_same_marked_text(left, middle) :
                (middle->kind != XUI_DOC_SOFT_BREAK &&
                    (middle->kind != XUI_DOC_TEXT || middle->attrs->iMarks))))
                return XUI_ERROR_UNSUPPORTED;
        }
    }
    if (doc_source_map_text(d->state, left, a.iOffset, XUI_DOC_AFTER,
            &from, &lmap) != XUI_OK || lmap != XUI_DOC_MAP_EXACT ||
        doc_source_map_text(d->state, right, b.iOffset, XUI_DOC_BEFORE,
            &to, &rmap) != XUI_OK || rmap != XUI_DOC_MAP_EXACT ||
        from >= to) return XUI_ERROR_UNSUPPORTED;
    if (cross_paragraph) {
        if (from < safe_from || from > to || to > safe_to)
            return XUI_ERROR_UNSUPPORTED;
        result = doc_prepare_visual_plain_block_source_safe(d->state->source,
            safe_from, safe_to);
        if (result != XUI_OK) return result;
    } else if (marked) {
        result = doc_prepare_visual_marked_source_safe(d->state->source, from, to);
        if (result != XUI_OK) return result;
    }
    shadow.document = d; shadow.base_revision = d->revision;
    shadow.domain = XUI_DOC_SEMANTIC; shadow.parsing = DOC_BUILD_SEMANTIC;
    shadow.draft = doc_state_clone(d->state);
    if (!shadow.draft) return XUI_ERROR_OUT_OF_MEMORY;
    ordered.tAnchor = a; ordered.tCaret = b;
    result = xuiDocumentTxnReplaceRange(&shadow, &ordered, text, bytes, &target);
    if (result != XUI_OK) goto done;
    if (!shadow.count) { result = XUI_ERROR_UNSUPPORTED; goto done; }
    if (target.iKind != XUI_DOC_POSITION_TEXT || target.iNodeId != a.iNodeId ||
        a.iOffset > UINT64_MAX - bytes || target.iOffset != a.iOffset + bytes) {
        result = XUI_ERROR_UNSUPPORTED; goto done;
    }
    result = doc_prepare_visual_escape(d->allocator, text, bytes, &escaped, &used);
    if (result != XUI_OK) goto done;
    if (from > UINT64_MAX - used) { result = XUI_DOC_ERROR_LIMIT; goto done; }
    patch.iSize = sizeof(patch); patch.iStart = from; patch.iEnd = to;
    patch.sText = escaped; patch.iTextBytes = used;
    result = doc_snapshot_create(shadow.draft, d->identity, d->revision, preview);
    if (result != XUI_OK) goto done;
    result = doc_prepare_source(d, NULL, desc, &patch, 1, NULL, 0, 0, out);
    if (result == XUI_OK) {
        result = doc_prepare_visual_append(*out, NULL, target.iNodeId,
            a.iOffset, a.iOffset, text, bytes, from, to, used);
        if (result == XUI_OK) {
            doc_seq_release(shadow.draft->source); shadow.draft->source = (*out)->source;
            doc_seq_retain(shadow.draft->source);
            shadow.draft->source_open_brackets = (*out)->source_open_brackets;
            (*out)->visual_expected = shadow.draft; doc_state_retain(shadow.draft);
            (*out)->visual_parent = parent->id;
            (*out)->publication = doc_publish_plan_capture(d);
            if (!(*out)->publication) result = XUI_ERROR_OUT_OF_MEMORY;
        }
        if (result == XUI_OK) {
            *caret = target; *source_caret = from + used;
        } else { xuiDocumentPrepareRelease(*out); *out = NULL; }
    }
    if (result != XUI_OK) { xuiDocumentSnapshotRelease(*preview); *preview = NULL; }
done:
    doc_free(escaped); doc_free(shadow.ops); doc_state_release(shadow.draft);
    return result;
}
int doc_prepare_visual_continue(xui_document d, xui_document_prepare previous,
    uint64_t id, uint64_t start, uint64_t end, const char* text, uint64_t bytes, uint64_t source_caret,
    xui_document_prepare* out, xui_document_snapshot* preview, uint64_t* next_source_caret)
{
    struct xui_doc_transaction_t shadow = {0};
    xui_doc_source_patch_t patch = {0}; doc_node *node, *parent, *base;
    char* escaped = NULL; uint64_t from, to, used = 0, left = 0, right = 0;
    uint64_t base_from, base_to; int mapping, result, fast_delete = 0;
    if (!out || !preview || !next_source_caret) return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL; *preview = NULL;
    if (!d || !previous || (!text && bytes) || bytes > (SIZE_MAX - 1) / 2 ||
        (start == end && !bytes) || start > end ||
        d->pending != previous || !previous->visual_expected ||
        previous->transaction.base_revision != d->revision ||
        (bytes && (memchr(text, '\n', (size_t)bytes) || memchr(text, '\r', (size_t)bytes))))
        return XUI_ERROR_UNSUPPORTED;
    if (source_caret > doc_seq_size(previous->source)) return XUI_ERROR_INVALID_ARGUMENT;
    if (previous->visual_parent && (!bytes || start != end ||
        id != previous->visual_node || start != previous->visual_caret_text ||
        source_caret != previous->visual_caret_source))
        return XUI_ERROR_UNSUPPORTED;
    node = doc_index_get(previous->visual_expected->index, id);
    parent = node ? doc_index_get(previous->visual_expected->index, node->parent) : NULL;
    if (!node || node->kind != XUI_DOC_TEXT || !parent ||
        (parent->kind != XUI_DOC_PARAGRAPH && parent->kind != XUI_DOC_HEADING) ||
        (node->attrs->iMarks && !doc_prepare_visual_same_marked_text(node, node)) ||
        !start || end > doc_seq_size(node->text) ||
        (!previous->visual_parent && end == doc_seq_size(node->text)))
        return XUI_ERROR_UNSUPPORTED;
    if (bytes && start == end && start == previous->visual_caret_text &&
        source_caret == previous->visual_caret_source) {
        from = to = source_caret;
    } else if (!bytes && previous->visual_delete_chain &&
        source_caret == previous->visual_caret_source) {
        /* A deletion-only chain has one contiguous removed range in the base
         * leaf. Backspace consumes its untouched prefix; Delete consumes its
         * untouched suffix. The current source joins them at source_caret. */
        base = doc_index_get(d->state->index, id);
        if (!previous->visual_delete_chain || !base || base->kind != XUI_DOC_TEXT ||
            previous->visual_delete_left >= previous->visual_delete_right ||
            previous->visual_delete_right > doc_seq_size(base->text) ||
            doc_source_map_text(d->state, base, previous->visual_delete_left,
                XUI_DOC_AFTER, &base_from, &mapping) != XUI_OK ||
            mapping != XUI_DOC_MAP_EXACT || base_from != source_caret)
            return XUI_ERROR_UNSUPPORTED;
        left = previous->visual_delete_left; right = previous->visual_delete_right;
        if (end == left && start < end) {
            if (doc_source_map_text(d->state, base, start, XUI_DOC_AFTER,
                    &from, &mapping) != XUI_OK || mapping != XUI_DOC_MAP_EXACT ||
                doc_source_map_text(d->state, base, end, XUI_DOC_BEFORE,
                    &to, &mapping) != XUI_OK || mapping != XUI_DOC_MAP_EXACT ||
                from >= to || to != source_caret) return XUI_ERROR_UNSUPPORTED;
            left = start;
        } else if (start == left && end > start &&
            end - start <= doc_seq_size(base->text) - right) {
            if (doc_source_map_text(d->state, base, right, XUI_DOC_AFTER,
                    &base_from, &mapping) != XUI_OK || mapping != XUI_DOC_MAP_EXACT ||
                doc_source_map_text(d->state, base, right + end - start, XUI_DOC_BEFORE,
                    &base_to, &mapping) != XUI_OK || mapping != XUI_DOC_MAP_EXACT ||
                base_from >= base_to || source_caret > UINT64_MAX - (base_to - base_from))
                return XUI_ERROR_UNSUPPORTED;
            from = source_caret; to = source_caret + (base_to - base_from);
            if (to > doc_seq_size(previous->source)) return XUI_ERROR_UNSUPPORTED;
            right += end - start;
        } else return XUI_ERROR_UNSUPPORTED;
        fast_delete = 1;
    } else {
        result = doc_prepare_visual_map(d, previous, id, start, XUI_DOC_AFTER, &from);
        if (result != XUI_OK) return result;
        result = start == end ? XUI_OK :
            doc_prepare_visual_map(d, previous, id, end, XUI_DOC_BEFORE, &to);
        if (result != XUI_OK) return result;
        if (start == end) to = from;
        if (from > to || (start < end && from == to) ||
            to > doc_seq_size(previous->source)) return XUI_ERROR_UNSUPPORTED;
    }
    if (node->attrs->iMarks) {
        result = doc_prepare_visual_marked_source_safe(previous->source, from, to);
        if (result != XUI_OK) return result;
    }
    if (bytes) {
        result = doc_prepare_visual_escape(d->allocator, text, bytes, &escaped, &used);
        if (result != XUI_OK) return result;
    }
    shadow.document = d; shadow.base_revision = d->revision; shadow.domain = XUI_DOC_SEMANTIC;
    shadow.parsing = DOC_BUILD_SEMANTIC; shadow.draft = doc_state_clone(previous->visual_expected);
    if (!shadow.draft) { doc_free(escaped); return XUI_ERROR_OUT_OF_MEMORY; }
    result = doc_txn_text(&shadow, id, start, end, text, bytes);
    if (result != XUI_OK) goto done;
    if (from > UINT64_MAX - used) { result = XUI_DOC_ERROR_LIMIT; goto done; }
    patch.iSize = sizeof(patch); patch.iStart = from; patch.iEnd = to;
    patch.sText = bytes ? escaped : ""; patch.iTextBytes = used;
    result = doc_snapshot_create(shadow.draft, d->identity, d->revision, preview);
    if (result != XUI_OK) goto done;
    result = doc_prepare_source(d, previous, NULL, &patch, 1, NULL, 0, 0, out);
    if (result == XUI_OK) {
        result = doc_prepare_visual_append(*out, previous, id, start, end,
            text, bytes, from, to, used);
        if (result == XUI_OK) {
            doc_seq_release(shadow.draft->source); shadow.draft->source = (*out)->source;
            doc_seq_retain(shadow.draft->source);
            shadow.draft->source_open_brackets = (*out)->source_open_brackets;
            (*out)->visual_expected = shadow.draft; doc_state_retain(shadow.draft);
            (*out)->visual_parent = previous->visual_parent;
            (*out)->visual_delete_chain = fast_delete;
            (*out)->visual_delete_left = left;
            (*out)->visual_delete_right = right;
            *next_source_caret = from + used;
            (*out)->publication = doc_publish_plan_capture(d);
            if (!(*out)->publication) result = XUI_ERROR_OUT_OF_MEMORY;
        }
        if (result != XUI_OK) {
            xuiDocumentPrepareRelease(*out); *out = NULL;
        }
    }
    if (result != XUI_OK) { xuiDocumentSnapshotRelease(*preview); *preview = NULL; }
done:
    doc_free(escaped); doc_free(shadow.ops); doc_state_release(shadow.draft);
    return result;
}
int doc_prepare_visual_attach(xui_document d, xui_document_prepare previous,
    xui_document_prepare p)
{
    if (!d || !p || d->writer || d->notifying || d->pending != previous ||
        p->identity != d->identity || p->transaction.base_revision != d->revision ||
        !p->visual_expected || !p->publication || atomic_load(&p->cancellation))
        return XUI_DOC_ERROR_STALE;
    d->next_prepare = p->generation;
    doc_prepare_invalidate(d, NULL, XUI_DOC_ERROR_STALE);
    d->pending = p; xuiDocumentPrepareRetain(p);
    return XUI_OK;
}
XUI_API int xuiDocumentPrepareContinueSource(xui_document d, xui_document_prepare previous,
    const xui_doc_source_patch_t* patches, uint64_t count, xui_document_prepare* out)
{
    if (!previous) { if (out) *out = NULL; return XUI_ERROR_INVALID_ARGUMENT; }
    return doc_prepare_source(d, previous, NULL, patches, count, NULL, 0, 1, out);
}
int doc_prepare_preview(xui_document d, xui_document_prepare previous, const xui_doc_source_patch_t* patch, xui_document_prepare* out)
{
    return doc_prepare_source(d, previous, NULL, patch, 1, NULL, 0, 0, out);
}

static unsigned doc_prepare_utf8_byte(const char* prefix, unsigned prefix_bytes, const char* text, uint64_t offset)
{
    return (unsigned char)(offset < prefix_bytes ? prefix[offset] : text[offset - prefix_bytes]);
}
/* Validate partial prefixes too: ED A0, F4 90, E0 80 and overlong leading
 * bytes are already invalid, even before their last continuation arrives. */
static int doc_prepare_utf8_prefix(const char* prefix, unsigned prefix_bytes, const char* text, uint64_t bytes, uint64_t* complete)
{
    uint64_t at = 0, total = prefix_bytes + bytes;
    while (at < total) {
        unsigned lead = doc_prepare_utf8_byte(prefix, prefix_bytes, text, at), length, i;
        if (lead < 0x80) { at++; continue; }
        if (lead >= 0xc2 && lead <= 0xdf) length = 2;
        else if (lead >= 0xe0 && lead <= 0xef) length = 3;
        else if (lead >= 0xf0 && lead <= 0xf4) length = 4;
        else return XUI_DOC_ERROR_UTF8;
        for (i = 1; i < length; i++) {
            unsigned c;
            if (i >= total - at) { *complete = at; return XUI_OK; }
            c = doc_prepare_utf8_byte(prefix, prefix_bytes, text, at + i);
            if ((c & 0xc0) != 0x80 || (i == 1 &&
                ((lead == 0xe0 && c < 0xa0) || (lead == 0xed && c > 0x9f) ||
                 (lead == 0xf0 && c < 0x90) || (lead == 0xf4 && c > 0x8f)))) return XUI_DOC_ERROR_UTF8;
        }
        at += length;
    }
    *complete = total; return XUI_OK;
}
XUI_API int xuiDocumentPrepareStreamSource(xui_document d, xui_document_prepare previous, const xui_doc_txn_desc_t* desc,
    const char* text, uint64_t bytes, int final_chunk, xui_document_prepare* out)
{
    const char* prefix = previous ? previous->utf8 : NULL;
    unsigned prefix_bytes = previous ? previous->utf8_bytes : 0, tail_bytes, i, joined_bytes = 0;
    uint64_t total, complete = 0, used = 0, source_bytes, count = 0;
    xui_doc_source_patch_t patches[2] = {{0}}; char tail[3], joined[4]; int result;
    if (!out) return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    if (!d || (!text && bytes) || bytes > SIZE_MAX || bytes > UINT64_MAX - prefix_bytes || (previous && desc) ||
        (desc && (desc->iSize != sizeof(*desc) || desc->iDomain != XUI_DOC_SOURCE))) return XUI_ERROR_INVALID_ARGUMENT;
    if (d->writer || d->notifying) return XUI_DOC_ERROR_BUSY;
    if (d->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_ERROR_UNSUPPORTED;
    if (desc && desc->iBaseRevision && desc->iBaseRevision != d->revision) return XUI_DOC_ERROR_STALE;
    if (previous) {
        if (d->pending != previous || previous->identity != d->identity || previous->transaction.base_revision != d->revision) return XUI_DOC_ERROR_STALE;
        result = atomic_load(&previous->cancellation); if (result) return result;
    }
    total = prefix_bytes + bytes;
    source_bytes = doc_seq_size(previous ? previous->source : d->state->source);
    if (total > d->max_bytes || source_bytes > d->max_bytes - total) return XUI_DOC_ERROR_LIMIT;
    result = doc_prepare_utf8_prefix(prefix, prefix_bytes, text, bytes, &complete);
    if (result != XUI_OK) return result;
    tail_bytes = (unsigned)(total - complete);
    if (final_chunk && tail_bytes) return XUI_DOC_ERROR_UTF8;
    for (i = 0; i < tail_bytes; i++) tail[i] = (char)doc_prepare_utf8_byte(prefix, prefix_bytes, text, complete + i);
    if (complete && prefix_bytes) {
        unsigned lead = (unsigned char)prefix[0]; joined_bytes = lead < 0xe0 ? 2 : lead < 0xf0 ? 3 : 4;
        for (i = 0; i < joined_bytes; i++) joined[i] = (char)doc_prepare_utf8_byte(prefix, prefix_bytes, text, i);
        patches[count].iSize = sizeof(*patches); patches[count].iStart = patches[count].iEnd = source_bytes;
        patches[count].sText = joined; patches[count++].iTextBytes = joined_bytes;
        used = joined_bytes - prefix_bytes;
    }
    if (complete > joined_bytes) {
        patches[count].iSize = sizeof(*patches); patches[count].iStart = patches[count].iEnd = source_bytes + joined_bytes;
        patches[count].sText = text + used; patches[count++].iTextBytes = complete - joined_bytes;
    }
    return doc_prepare_source(d, previous, desc, patches, count, tail, tail_bytes, 1, out);
}

XUI_API int xuiDocumentPrepareRun(xui_document_prepare p)
{
    int expected = XUI_DOC_PREPARE_QUEUED, result;
    if (!p) return XUI_ERROR_INVALID_ARGUMENT;
    if (p->utf8_bytes && !atomic_load(&p->cancellation)) return XUI_DOC_ERROR_BUSY;
    if (!atomic_compare_exchange_strong(&p->state, &expected, XUI_DOC_PREPARE_RUNNING)) return XUI_ERROR_INVALID_STATE;
    xuiDocumentPrepareRetain(p);
    result = doc_txn_check(&p->transaction, 0);
    if (result == XUI_OK && p->operations) {
        uint64_t bytes = doc_seq_size(p->operations);
        p->transaction.ops = doc_alloc(p->allocator, (size_t)bytes);
        if (!p->transaction.ops) result = XUI_ERROR_OUT_OF_MEMORY;
        else {
            result = doc_seq_read(p->operations, 0, p->transaction.ops, bytes);
            p->transaction.count = p->transaction.capacity = bytes / sizeof(*p->transaction.ops);
        }
    }
    if (result == XUI_OK && p->transaction.count) result = doc_markdown_parse(&p->transaction);
    if (result == XUI_OK && p->visual_expected &&
        !doc_semantic_equal(p->visual_expected, p->transaction.draft)) result = XUI_DOC_ERROR_UNREPRESENTABLE;
    if (result == XUI_OK && p->publication) result = doc_publish_plan_build(p->publication, &p->transaction, p->identity);
    if (result == XUI_OK) result = doc_txn_check(&p->transaction, 0);
    atomic_store(&p->result, result);
    atomic_store(&p->state, result == XUI_OK ? XUI_DOC_PREPARE_READY : XUI_DOC_PREPARE_FAILED);
    xuiDocumentPrepareRelease(p); return result;
}
doc_publish_plan* doc_prepare_publish_plan(xui_document_prepare p, xui_document d)
{
    return p && doc_publish_plan_valid(p->publication, d) ? p->publication : NULL;
}
XUI_API int xuiDocumentPrepareGetInfo(xui_document_prepare p, xui_doc_prepare_info_t* out)
{
    int state, result, cancelled;
    uint64_t revision;
    if (!p || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    do {
        state = atomic_load(&p->state); result = atomic_load(&p->result);
        revision = atomic_load(&p->committed_revision);
    } while (state != atomic_load(&p->state));
    cancelled = atomic_load(&p->cancellation);
    out->iDocumentId = p->identity; out->iBaseRevision = p->transaction.base_revision;
    out->iGeneration = p->generation; out->iSourceBytes = doc_seq_size(p->source); out->iPatchCount = p->patch_count;
    out->iBufferedUtf8Bytes = p->utf8_bytes;
    out->bCancellationRequested = cancelled != 0;
    out->iCommittedRevision = state == XUI_DOC_PREPARE_COMMITTED ? revision : 0;
    if (cancelled && (state == XUI_DOC_PREPARE_QUEUED || state == XUI_DOC_PREPARE_READY)) {
        state = XUI_DOC_PREPARE_FAILED; result = cancelled;
    } else if (state == XUI_DOC_PREPARE_QUEUED || state == XUI_DOC_PREPARE_RUNNING || state == XUI_DOC_PREPARE_PUBLISHING)
        result = XUI_DOC_ERROR_BUSY;
    out->iState = (uint32_t)state; out->iResult = result;
    return XUI_OK;
}
XUI_API int xuiDocumentPrepareCopySource(xui_document_prepare p, char* buffer, uint64_t capacity, uint64_t* length)
{
    uint64_t size;
    if (!p || !length) return XUI_ERROR_INVALID_ARGUMENT;
    *length = size = doc_seq_size(p->source);
    if (!buffer) return XUI_OK;
    if (capacity <= size || capacity > SIZE_MAX) return XUI_ERROR_BUFFER_TOO_SMALL;
    if (doc_seq_read(p->source, 0, buffer, size) != XUI_OK) return XUI_ERROR_INVALID_ARGUMENT;
    buffer[size] = 0; return XUI_OK;
}
XUI_API int xuiDocumentPrepareReadSource(xui_document_prepare p, uint64_t offset, void* buffer, uint64_t bytes)
{
    return p ? doc_seq_read(p->source, offset, buffer, bytes) : XUI_ERROR_INVALID_ARGUMENT;
}
doc_sequence* doc_prepare_source_store(xui_document_prepare p) { return p ? p->source : NULL; }
int doc_prepare_source_delta(xui_document_prepare p, xui_document_prepare previous, uint64_t* start, uint64_t* end, uint64_t* bytes)
{
    if (!p || !p->single_patch || p->previous_generation != (previous ? previous->generation : 0)) return 0;
    *start = p->patch_start; *end = p->patch_end; *bytes = p->patch_bytes; return 1;
}
int doc_prepare_position_valid(xui_document_prepare p, const xui_doc_position_t* at)
{
    return p && at && at->iSize == sizeof(*at) && at->iKind == XUI_DOC_POSITION_SOURCE && at->iNodeId == DOC_ROOT &&
        at->iDocumentId == p->identity && at->iRevision == p->transaction.base_revision && at->iInputGeneration == p->generation &&
        at->iAffinity <= XUI_DOC_AFTER && doc_seq_boundary(p->source, at->iOffset);
}
XUI_API int xuiDocumentPrepareSourcePosition(xui_document_prepare p, uint64_t offset, uint32_t affinity, xui_doc_position_t* at)
{
    if (!p || !at || affinity > XUI_DOC_AFTER || offset > doc_seq_size(p->source)) return XUI_ERROR_INVALID_ARGUMENT;
    if (!doc_seq_boundary(p->source, offset)) return XUI_DOC_ERROR_UTF8;
    memset(at, 0, sizeof(*at)); at->iSize = sizeof(*at); at->iKind = XUI_DOC_POSITION_SOURCE; at->iNodeId = DOC_ROOT;
    at->iDocumentId = p->identity; at->iRevision = p->transaction.base_revision; at->iInputGeneration = p->generation;
    at->iOffset = offset; at->iAffinity = affinity; return XUI_OK;
}
XUI_API int xuiDocumentPreparePublish(xui_document d, xui_document_prepare p, xui_document_change_set* out)
{
    struct xui_doc_transaction_t t;
    int expected = XUI_DOC_PREPARE_READY, result;
    if (out) *out = NULL;
    if (!d || !p) return XUI_ERROR_INVALID_ARGUMENT;
    if (d->identity != p->identity) return XUI_DOC_ERROR_STALE;
    if (d->writer || d->notifying) return XUI_DOC_ERROR_BUSY;
    if (!atomic_compare_exchange_strong(&p->state, &expected, XUI_DOC_PREPARE_PUBLISHING)) {
        result = expected == XUI_DOC_PREPARE_FAILED ? atomic_load(&p->result) :
            expected == XUI_DOC_PREPARE_QUEUED ? atomic_load(&p->cancellation) : 0;
        if (result) {
            if (d->pending == p && (result == XUI_DOC_ERROR_CANCELLED || result == XUI_DOC_ERROR_STALE)) doc_prepare_invalidate(d, p, result);
            return result;
        }
        return expected == XUI_DOC_PREPARE_QUEUED || expected == XUI_DOC_PREPARE_RUNNING ? XUI_DOC_ERROR_BUSY : XUI_ERROR_INVALID_STATE;
    }
    xuiDocumentPrepareRetain(p);
    xuiDocumentRetain(d);
    result = atomic_load(&p->cancellation);
    if (!result && (d->revision != p->transaction.base_revision || d->pending != p || d->next_prepare != p->generation)) result = XUI_DOC_ERROR_STALE;
    if (!result) {
        t = p->transaction; t.document = d; t.preparation = p;
        /* Publication has claimed the candidate. Late cancellation cannot
         * interrupt callbacks or undo an already-published revision. */
        t.cancellation = NULL; d->writer = &t;
        result = xuiDocumentTxnCommit(&t, out);
        xuiDocumentTxnAbort(&t);
    }
    if (result == XUI_OK) atomic_store(&p->committed_revision, d->revision);
    if (d->pending == p && (result == XUI_OK || result == XUI_DOC_ERROR_CANCELLED || result == XUI_DOC_ERROR_STALE)) doc_prepare_invalidate(d, p, result);
    atomic_store(&p->result, result);
    atomic_store(&p->state, result == XUI_OK ? XUI_DOC_PREPARE_COMMITTED : XUI_DOC_PREPARE_FAILED);
    xuiDocumentPrepareRelease(p); xuiDocumentRelease(d); return result;
}

#endif
