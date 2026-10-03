#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include "xui_unicode_core.h"

static xui_doc_position_t doc_plain_edge(doc_plain_projection* p, uint64_t id, int end)
{
    doc_node* n = doc_index_get(p->state->index, id); xui_doc_position_t result = p->origin;
    while (doc_seq_size(n->children)) n = doc_index_get(p->state->index, doc_seq_get_id(n->children, end ? doc_seq_size(n->children) - 1 : 0));
    result.iNodeId = n->id; result.iKind = doc_text_kind(n->kind) ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
    result.iOffset = end ? doc_seq_size(n->text) : 0; return result;
}
static void doc_plain_span_add(doc_plain_projection* p, doc_sequence* text, const char* separator, uint64_t length,
    xui_doc_position_t first, xui_doc_position_t last, int literal)
{
    doc_plain_span* span;
    if (p->error || !length) return;
    if (length > p->capacity - p->bytes) { p->error = XUI_DOC_ERROR_LIMIT; return; }
    if (p->count == p->span_capacity) {
        uint64_t capacity = p->span_capacity ? p->span_capacity * 2 : 32; doc_plain_span* spans;
        if (capacity < p->span_capacity || capacity > SIZE_MAX / sizeof(*spans)) { p->error = XUI_DOC_ERROR_LIMIT; return; }
        spans = doc_realloc(p->allocator, p->spans, (size_t)capacity * sizeof(*spans));
        if (!spans) { p->error = XUI_ERROR_OUT_OF_MEMORY; return; }
        p->spans = spans; p->span_capacity = capacity;
    }
    span = &p->spans[p->count++]; span->start = p->bytes; span->length = length;
    span->first = first; span->last = last; span->literal = literal;
    if (separator) memcpy(p->text + p->bytes, separator, (size_t)length);
    else p->error = doc_seq_read(text, 0, p->text + p->bytes, length);
    p->bytes += length;
}
static void doc_plain_project_node(doc_plain_projection* p, uint64_t id)
{
    doc_node* n = doc_index_get(p->state->index, id); doc_node* parent = doc_index_get(p->state->index, n->parent);
    uint64_t i, index = parent ? doc_child_index(parent, id) : 0; xui_doc_position_t a = p->origin, b = p->origin;
    if (p->error) return;
    if (doc_text_kind(n->kind)) {
        a.iKind = b.iKind = XUI_DOC_POSITION_TEXT; a.iNodeId = b.iNodeId = id; a.iOffset = 0; b.iOffset = doc_seq_size(n->text);
        doc_plain_span_add(p, n->text, NULL, b.iOffset, a, b, 1);
    }
    for (i = 0; i < doc_seq_size(n->children); i++) {
        if (i && n->kind == XUI_DOC_ROW) {
            a = doc_plain_edge(p, doc_seq_get_id(n->children, i - 1), 1); b = doc_plain_edge(p, doc_seq_get_id(n->children, i), 0);
            doc_plain_span_add(p, NULL, "\t", 1, a, b, 0);
        }
        doc_plain_project_node(p, doc_seq_get_id(n->children, i));
    }
    if (n->kind == XUI_DOC_SOFT_BREAK || n->kind == XUI_DOC_HARD_BREAK) {
        a = b = p->origin; a.iKind = b.iKind = XUI_DOC_POSITION_GAP;
        a.iNodeId = b.iNodeId = n->parent; a.iOffset = index; b.iOffset = index + 1;
        doc_plain_span_add(p, NULL, "\n", 1, a, b, 0);
    }
    if (n->kind == XUI_DOC_PARAGRAPH || n->kind == XUI_DOC_HEADING || n->kind == XUI_DOC_CODE_BLOCK || n->kind == XUI_DOC_ROW) {
        if (parent && parent->kind == XUI_DOC_CELL && index + 1 == doc_seq_size(parent->children)) return;
        a = doc_plain_edge(p, id, 1); b = p->origin; b.iNodeId = n->parent; b.iKind = XUI_DOC_POSITION_GAP; b.iOffset = index + 1;
        if (parent && index + 1 < doc_seq_size(parent->children)) b = doc_plain_edge(p, doc_seq_get_id(parent->children, index + 1), 0);
        doc_plain_span_add(p, NULL, "\n", 1, a, b, 0);
    }
}
void doc_plain_projection_free(doc_plain_projection* p)
{
    doc_free(p->text); doc_free(p->spans); memset(p, 0, sizeof(*p));
}
int doc_plain_project(xui_document_snapshot snapshot, uint32_t domain, doc_plain_projection* p)
{
    uint64_t capacity; int result;
    memset(p, 0, sizeof(*p));
    if (!snapshot || (domain != XUI_DOC_SEMANTIC && domain != XUI_DOC_SOURCE)) return XUI_ERROR_INVALID_ARGUMENT;
    if (domain == XUI_DOC_SOURCE && snapshot->state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_DOC_ERROR_DOMAIN;
    p->state = snapshot->state; p->allocator = snapshot->state->allocator;
    p->origin.iSize = sizeof(p->origin); p->origin.iDocumentId = snapshot->identity; p->origin.iRevision = snapshot->revision;
    p->origin.iNodeId = DOC_ROOT; p->origin.iKind = domain == XUI_DOC_SOURCE ? XUI_DOC_POSITION_SOURCE : XUI_DOC_POSITION_GAP;
    if (p->state->node_count > (UINT64_MAX - p->state->text_bytes - 1) / 2) return XUI_DOC_ERROR_LIMIT;
    capacity = domain == XUI_DOC_SOURCE ? doc_seq_size(p->state->source) : p->state->text_bytes + p->state->node_count * 2;
    if (capacity >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    p->text = doc_alloc(p->allocator, (size_t)capacity + 1); if (!p->text) return XUI_ERROR_OUT_OF_MEMORY;
    p->capacity = capacity;
    if (domain == XUI_DOC_SOURCE) {
        xui_doc_position_t end = p->origin; end.iOffset = capacity;
        doc_plain_span_add(p, p->state->source, NULL, capacity, p->origin, end, 1);
    } else doc_plain_project_node(p, DOC_ROOT);
    p->text[p->bytes] = 0; result = p->error;
    if (result) doc_plain_projection_free(p);
    return result;
}
xui_doc_position_t doc_plain_unproject(const doc_plain_projection* p, uint64_t offset, int after)
{
    uint64_t lo = 0, hi = p->count; xui_doc_position_t result = p->origin;
    while (lo < hi) { uint64_t mid = lo + (hi - lo) / 2; if (p->spans[mid].start < offset || (after && p->spans[mid].start == offset)) lo = mid + 1; else hi = mid; }
    if (lo) {
        const doc_plain_span* span = &p->spans[lo - 1];
        result = offset <= span->start ? span->first : offset >= span->start + span->length ? span->last : span->first;
        if (span->literal) result.iOffset = span->first.iOffset + (offset - span->start > span->length ? span->length : offset - span->start);
    } else if (p->count) result = p->spans[0].first;
    result.iAffinity = after ? XUI_DOC_AFTER : XUI_DOC_BEFORE; return result;
}
int doc_plain_project_position(const doc_plain_projection* p, const xui_doc_position_t* position, uint64_t* out)
{
    uint64_t i; int order;
    if (!position || position->iDocumentId != p->origin.iDocumentId || position->iRevision != p->origin.iRevision) return XUI_DOC_ERROR_STALE;
    if (!doc_position_valid(p->state, position)) return XUI_ERROR_INVALID_ARGUMENT;
    if ((position->iKind == XUI_DOC_POSITION_SOURCE) != (p->origin.iKind == XUI_DOC_POSITION_SOURCE)) return XUI_DOC_ERROR_DOMAIN;
    for (i = 0; i < p->count; i++) {
        const doc_plain_span* span = &p->spans[i];
        if (span->literal && position->iNodeId == span->first.iNodeId && position->iKind == span->first.iKind &&
            position->iOffset >= span->first.iOffset && position->iOffset <= span->last.iOffset) { *out = span->start + position->iOffset - span->first.iOffset; return XUI_OK; }
        doc_position_compare(p->state, position, &span->first, &order);
        if (order <= 0) { *out = span->start; return XUI_OK; }
    }
    *out = p->bytes; return XUI_OK;
}
XUI_API int xuiDocumentSnapshotFind(xui_document_snapshot snapshot, uint32_t domain, const char* pattern, uint64_t length,
    const xui_doc_range_t* scope, xui_doc_range_t* matches, uint64_t capacity, uint64_t* total)
{
    doc_plain_projection p; uint64_t *prefix = NULL, first = 0, last, i, j = 0, count = 0; int result;
    if (total) *total = 0;
    if (!snapshot || !total || (!matches && capacity) || !length || !doc_utf8(pattern, length)) return XUI_ERROR_INVALID_ARGUMENT;
    if (length > SIZE_MAX / sizeof(*prefix)) return XUI_DOC_ERROR_LIMIT;
    result = doc_plain_project(snapshot, domain, &p); if (result != XUI_OK) return result;
    last = p.bytes;
    if (scope) {
        result = doc_plain_project_position(&p, &scope->tAnchor, &first);
        if (result == XUI_OK) result = doc_plain_project_position(&p, &scope->tCaret, &last);
        if (result != XUI_OK) goto done;
        if (first > last) { uint64_t swap = first; first = last; last = swap; }
    }
    if (length > last - first) goto done;
    prefix = doc_alloc(p.allocator, (size_t)length * sizeof(*prefix)); if (!prefix) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    prefix[0] = 0;
    for (i = 1; i < length; i++) { while (j && pattern[i] != pattern[j]) j = prefix[j - 1]; if (pattern[i] == pattern[j]) j++; prefix[i] = j; }
    j = 0;
    for (i = first; i < last; i++) {
        while (j && p.text[i] != pattern[j]) j = prefix[j - 1];
        if (p.text[i] == pattern[j]) j++;
        if (j == length) {
            if (count < capacity) { matches[count].tAnchor = doc_plain_unproject(&p, i + 1 - length, 1); matches[count].tCaret = doc_plain_unproject(&p, i + 1, 0); }
            count++; j = 0;
        }
    }
    *total = count;
done:
    doc_free(prefix); doc_plain_projection_free(&p); return result;
}
static int doc_regex_error(int fallback)
{
    const xerror* error = xrtGetError();
    if (error && xrtErrorIs(error, XERR_MEMORY)) return XUI_ERROR_OUT_OF_MEMORY;
    if (error && xrtErrorFind(error, "xrt.regex", XREGEX_ERROR_LIMIT))
        return XUI_DOC_ERROR_LIMIT;
    return fallback;
}
XUI_API int xuiDocumentSnapshotFindEx(xui_document_snapshot snapshot, uint32_t domain,
    const char* pattern, uint64_t pattern_bytes, uint32_t flags,
    const xui_doc_range_t* scope, xui_doc_range_t* matches,
    uint64_t capacity, uint64_t* total)
{
    doc_plain_projection projection;
    xregexconfig config;
    xregex* regex = NULL;
    xregexmatcher* matcher = NULL;
    xstrview expression, text;
    char* escaped = NULL;
    uint64_t first = 0, last, count = 0;
    size_t escaped_bytes = 0;
    xregexresult state;
    int result;
    if (total) *total = 0;
    if (flags & ~(XUI_DOC_FIND_REGEX | XUI_DOC_FIND_IGNORE_CASE |
            XUI_DOC_FIND_WHOLE_WORD))
        return XUI_ERROR_INVALID_ARGUMENT;
    if (!flags) return xuiDocumentSnapshotFind(snapshot, domain, pattern,
        pattern_bytes, scope, matches, capacity, total);
    if (!snapshot || !total || (!matches && capacity) || !pattern_bytes ||
            pattern_bytes > SIZE_MAX || !doc_utf8(pattern, pattern_bytes))
        return XUI_ERROR_INVALID_ARGUMENT;
    expression = (xstrview){pattern, (size_t)pattern_bytes};
    if (!(flags & XUI_DOC_FIND_REGEX)) {
        escaped = xrtRegexEscape(expression, &escaped_bytes);
        if (!escaped) return doc_regex_error(XUI_ERROR_OUT_OF_MEMORY);
        expression = (xstrview){escaped, escaped_bytes};
    }
    xrtRegexConfigInit(&config);
    if (flags & XUI_DOC_FIND_IGNORE_CASE) config.Flags |= XREGEX_IGNORE_CASE;
    regex = xrtRegexCompileConfig(expression, &config);
    xrtFree(escaped);
    if (!regex) return doc_regex_error(XUI_ERROR_INVALID_ARGUMENT);
    matcher = xrtRegexMatcherCreate(regex);
    xrtRegexRelease(regex);
    if (!matcher) return doc_regex_error(XUI_ERROR_OUT_OF_MEMORY);
    result = doc_plain_project(snapshot, domain, &projection);
    if (result != XUI_OK) { xrtRegexMatcherFree(matcher); return result; }
    if ((flags & XUI_DOC_FIND_WHOLE_WORD) && projection.bytes > INT_MAX) {
        result = XUI_DOC_ERROR_LIMIT; goto done_regex;
    }
    last = projection.bytes;
    if (scope) {
        result = doc_plain_project_position(&projection, &scope->tAnchor,
            &first);
        if (result == XUI_OK)
            result = doc_plain_project_position(&projection, &scope->tCaret,
                &last);
        if (result != XUI_OK) goto done_regex;
        if (first > last) { uint64_t swap = first; first = last; last = swap; }
    }
    text = (xstrview){projection.text + first, (size_t)(last - first)};
    state = xrtRegexMatcherFind(matcher, text, 0);
    while (state == XREGEX_MATCH) {
        xregexcapture capture = {0};
        uint64_t begin, end;
        if (!xrtRegexMatcherCapture(matcher, 0, &capture) ||
                !capture.Matched || capture.Span.Begin > capture.Span.End ||
                capture.Span.End > text.Size) {
            result = XUI_ERROR_INVALID_STATE; goto done_regex;
        }
        begin = first + capture.Span.Begin;
        end = first + capture.Span.End;
        if ((flags & XUI_DOC_FIND_WHOLE_WORD) &&
            (!xuiInternalTextWordBoundary(projection.text, (int)projection.bytes,
                (int)begin, XUI_INTERNAL_WORD_NATURAL) ||
             !xuiInternalTextWordBoundary(projection.text, (int)projection.bytes,
                (int)end, XUI_INTERNAL_WORD_NATURAL))) {
            state = xrtRegexMatcherNext(matcher); continue;
        }
        if (count == UINT64_MAX) { result = XUI_DOC_ERROR_LIMIT; goto done_regex; }
        if (count < capacity) {
            matches[count].tAnchor = doc_plain_unproject(&projection, begin, 1);
            matches[count].tCaret = begin == end ? matches[count].tAnchor :
                doc_plain_unproject(&projection, end, 0);
        }
        count++;
        state = xrtRegexMatcherNext(matcher);
    }
    if (state == XREGEX_ERROR) result = doc_regex_error(XUI_ERROR_INVALID_STATE);
    else *total = count;
done_regex:
    xrtRegexMatcherFree(matcher);
    doc_plain_projection_free(&projection);
    return result;
}
static int doc_template_append(doc_allocator* allocator, char** output,
    size_t* length, size_t* capacity, const char* data, size_t bytes)
{
    size_t needed, next;
    char* grown;
    if (bytes > SIZE_MAX - *length - 1) return XUI_DOC_ERROR_LIMIT;
    needed = *length + bytes + 1;
    if (needed > *capacity) {
        next = *capacity ? *capacity : 32;
        while (next < needed) {
            if (next > SIZE_MAX / 2) { next = needed; break; }
            next *= 2;
        }
        grown = doc_realloc(allocator, *output, next);
        if (!grown) return XUI_ERROR_OUT_OF_MEMORY;
        *output = grown; *capacity = next;
    }
    if (bytes) memcpy(*output + *length, data, bytes);
    *length += bytes; (*output)[*length] = 0;
    return XUI_OK;
}
static int doc_template_expand(doc_allocator* allocator, const xregex* regex,
    const xregexmatcher* matcher, const char* replacement, size_t bytes,
    doc_find_expansion* out)
{
    size_t i = 0, length = 0, capacity = 0;
    char* output = NULL;
    int result = XUI_OK;
    while (i < bytes && result == XUI_OK) {
        size_t start = i, index = 0;
        xregexcapture capture = {0};
        if (replacement[i] != '$') {
            while (i < bytes && replacement[i] != '$') i++;
            if (matcher) result = doc_template_append(allocator, &output,
                &length, &capacity, replacement + start, i - start);
            continue;
        }
        i++;
        if (i == bytes) { result = XUI_ERROR_INVALID_ARGUMENT; break; }
        if (replacement[i] == '$') {
            if (matcher) result = doc_template_append(allocator, &output,
                &length, &capacity, "$", 1);
            i++; continue;
        }
        if (replacement[i] == '{') {
            size_t name_begin = ++i;
            while (i < bytes && replacement[i] != '}') i++;
            if (i == name_begin || i == bytes) {
                result = XUI_ERROR_INVALID_ARGUMENT; break;
            }
            index = xrtRegexCaptureIndex(regex,
                (xstrview){replacement + name_begin, i - name_begin});
            if (index == XRT_NPOS) { result = XUI_ERROR_INVALID_ARGUMENT; break; }
            i++;
        } else if (replacement[i] >= '0' && replacement[i] <= '9') {
            do {
                size_t digit = (size_t)(replacement[i] - '0');
                if (index > (SIZE_MAX - digit) / 10) {
                    result = XUI_ERROR_INVALID_ARGUMENT; break;
                }
                index = index * 10 + digit;
                i++;
            } while (i < bytes && replacement[i] >= '0' && replacement[i] <= '9');
            if (result != XUI_OK) break;
            if (index >= xrtRegexCaptureCount(regex)) {
                result = XUI_ERROR_INVALID_ARGUMENT; break;
            }
        } else { result = XUI_ERROR_INVALID_ARGUMENT; break; }
        if (matcher) {
            if (!xrtRegexMatcherCapture(matcher, index, &capture)) {
                result = XUI_ERROR_INVALID_STATE; break;
            }
            if (capture.Matched) result = doc_template_append(allocator,
                &output, &length, &capacity, capture.Text.Data,
                capture.Text.Size);
        }
    }
    if (result == XUI_OK && matcher && !output)
        result = doc_template_append(allocator, &output,
            &length, &capacity, "", 0);
    if (result == XUI_OK && out) { out->text = output; out->bytes = length; }
    else doc_free(output);
    return result;
}
void doc_find_expansions_free(doc_find_expansion* values, uint64_t count)
{
    uint64_t i;
    if (!values) return;
    for (i = 0; i < count; i++) doc_free(values[i].text);
    doc_free(values);
}
static int doc_find_range_equal(const xui_doc_range_t* a,
    const xui_doc_range_t* b)
{
    return a->tAnchor.iKind == b->tAnchor.iKind &&
        a->tAnchor.iNodeId == b->tAnchor.iNodeId &&
        a->tAnchor.iOffset == b->tAnchor.iOffset &&
        a->tCaret.iKind == b->tCaret.iKind &&
        a->tCaret.iNodeId == b->tCaret.iNodeId &&
        a->tCaret.iOffset == b->tCaret.iOffset;
}
int doc_find_expand_matches(xui_document_snapshot snapshot, uint32_t domain,
    const char* pattern, uint64_t pattern_bytes, uint32_t flags,
    const xui_doc_range_t* scope, const xui_doc_range_t* matches,
    uint64_t count, const char* replacement, uint64_t replacement_bytes,
    doc_find_expansion** out)
{
    doc_plain_projection projection;
    xregexconfig config;
    xregex *regex = NULL;
    xregexmatcher* matcher = NULL;
    doc_find_expansion* values = NULL;
    xstrview text;
    xregexresult state;
    uint64_t first = 0, last, index = 0;
    int result, projected = 0;
    if (out) *out = NULL;
    if (!snapshot || !out || (!matches && count) || !pattern ||
        !pattern_bytes || pattern_bytes > SIZE_MAX ||
        (!replacement && replacement_bytes) || replacement_bytes > SIZE_MAX ||
        !doc_utf8(replacement, replacement_bytes) ||
        !(flags & XUI_DOC_FIND_REGEX) ||
        (flags & ~(XUI_DOC_FIND_REGEX | XUI_DOC_FIND_IGNORE_CASE |
            XUI_DOC_FIND_WHOLE_WORD))) return XUI_ERROR_INVALID_ARGUMENT;
    xrtRegexConfigInit(&config);
    if (flags & XUI_DOC_FIND_IGNORE_CASE) config.Flags |= XREGEX_IGNORE_CASE;
    regex = xrtRegexCompileConfig(
        (xstrview){pattern, (size_t)pattern_bytes}, &config);
    if (!regex) return doc_regex_error(XUI_ERROR_INVALID_ARGUMENT);
    result = doc_template_expand(snapshot->state->allocator, regex, NULL,
        replacement ? replacement : "", (size_t)replacement_bytes, NULL);
    if (result != XUI_OK) goto done_expand;
    matcher = xrtRegexMatcherCreate(regex);
    if (!matcher) { result = doc_regex_error(XUI_ERROR_OUT_OF_MEMORY); goto done_expand; }
    result = doc_plain_project(snapshot, domain, &projection);
    if (result != XUI_OK) goto done_expand;
    projected = 1; last = projection.bytes;
    if ((flags & XUI_DOC_FIND_WHOLE_WORD) && projection.bytes > INT_MAX) {
        result = XUI_DOC_ERROR_LIMIT; goto done_expand;
    }
    if (scope) {
        result = doc_plain_project_position(&projection, &scope->tAnchor, &first);
        if (result == XUI_OK)
            result = doc_plain_project_position(&projection, &scope->tCaret, &last);
        if (result != XUI_OK) goto done_expand;
        if (first > last) { uint64_t swap = first; first = last; last = swap; }
    }
    if (count > SIZE_MAX / sizeof(*values)) {
        result = XUI_DOC_ERROR_LIMIT; goto done_expand;
    }
    if (count) {
        values = doc_alloc(projection.allocator, (size_t)count * sizeof(*values));
        if (!values) { result = XUI_ERROR_OUT_OF_MEMORY; goto done_expand; }
    }
    text = (xstrview){projection.text + first, (size_t)(last - first)};
    state = xrtRegexMatcherFind(matcher, text, 0);
    while (state == XREGEX_MATCH) {
        xregexcapture capture = {0};
        uint64_t begin, end;
        xui_doc_range_t range;
        if (!xrtRegexMatcherCapture(matcher, 0, &capture) ||
            !capture.Matched || capture.Span.Begin > capture.Span.End ||
            capture.Span.End > text.Size) {
            result = XUI_ERROR_INVALID_STATE; goto done_expand;
        }
        begin = first + capture.Span.Begin; end = first + capture.Span.End;
        if ((flags & XUI_DOC_FIND_WHOLE_WORD) &&
            (!xuiInternalTextWordBoundary(projection.text, (int)projection.bytes,
                (int)begin, XUI_INTERNAL_WORD_NATURAL) ||
             !xuiInternalTextWordBoundary(projection.text, (int)projection.bytes,
                (int)end, XUI_INTERNAL_WORD_NATURAL))) {
            state = xrtRegexMatcherNext(matcher); continue;
        }
        range.tAnchor = doc_plain_unproject(&projection, begin, 1);
        range.tCaret = begin == end ? range.tAnchor :
            doc_plain_unproject(&projection, end, 0);
        if (index >= count || !doc_find_range_equal(&range, &matches[index])) {
            result = XUI_ERROR_INVALID_STATE; goto done_expand;
        }
        result = doc_template_expand(projection.allocator, regex, matcher,
            replacement ? replacement : "", (size_t)replacement_bytes,
            &values[index]);
        if (result != XUI_OK) goto done_expand;
        index++; state = xrtRegexMatcherNext(matcher);
    }
    if (state == XREGEX_ERROR) result = doc_regex_error(XUI_ERROR_INVALID_STATE);
    else if (index != count) result = XUI_ERROR_INVALID_STATE;
    else { *out = values; values = NULL; result = XUI_OK; }
done_expand:
    doc_find_expansions_free(values, count);
    if (projected) doc_plain_projection_free(&projection);
    xrtRegexMatcherFree(matcher);
    xrtRegexRelease(regex);
    return result;
}
XUI_API int xuiDocumentTxnReplaceAllEx(xui_document_transaction t,
    const char* pattern, uint64_t pattern_bytes,
    const char* replacement, uint64_t replacement_bytes, uint32_t flags,
    const xui_doc_range_t* scope, uint64_t* replaced)
{
    struct xui_doc_snapshot_t snapshot = {0}; xui_doc_range_t* matches = NULL;
    doc_find_expansion* expansions = NULL;
    uint64_t count = 0, i;
    struct xui_doc_transaction_t native = {0}; xui_document_transaction target = t; int result = doc_txn_check(t, 0);
    if (replaced) *replaced = 0;
    if (result != XUI_OK) return result;
    if (!replaced || !doc_utf8(replacement, replacement_bytes) ||
        (flags & ~(XUI_DOC_FIND_REGEX | XUI_DOC_FIND_IGNORE_CASE |
            XUI_DOC_FIND_WHOLE_WORD | XUI_DOC_REPLACE_EXPAND)) ||
        ((flags & XUI_DOC_REPLACE_EXPAND) && !(flags & XUI_DOC_FIND_REGEX)))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    snapshot.identity = t->document->identity; snapshot.revision = t->base_revision; snapshot.state = t->draft;
    result = xuiDocumentSnapshotFindEx(&snapshot, t->domain, pattern,
        pattern_bytes, flags & ~XUI_DOC_REPLACE_EXPAND, scope, NULL, 0, &count);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (!count) {
        if (flags & XUI_DOC_REPLACE_EXPAND) {
            result = doc_find_expand_matches(&snapshot, t->domain, pattern,
                pattern_bytes, flags & ~XUI_DOC_REPLACE_EXPAND, scope,
                NULL, 0, replacement, replacement_bytes, &expansions);
            doc_find_expansions_free(expansions, 0);
        }
        return result == XUI_OK ? XUI_OK : doc_txn_fail(t, result);
    }
    if (count > SIZE_MAX / sizeof(*matches)) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    matches = doc_alloc(t->document->allocator, (size_t)count * sizeof(*matches)); if (!matches) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    result = xuiDocumentSnapshotFindEx(&snapshot, t->domain, pattern,
        pattern_bytes, flags & ~XUI_DOC_REPLACE_EXPAND, scope, matches, count, &count);
    if (result == XUI_OK && (flags & XUI_DOC_REPLACE_EXPAND))
        result = doc_find_expand_matches(&snapshot, t->domain, pattern,
            pattern_bytes, flags & ~XUI_DOC_REPLACE_EXPAND, scope,
            matches, count, replacement, replacement_bytes, &expansions);
    /* Semantic Markdown replacements share one shadow tree and one source patch,
     * so an early reparse cannot invalidate later match node identities. */
    if (result == XUI_OK && t->draft->profile == XUI_DOCUMENT_MARKDOWN && t->domain == XUI_DOC_SEMANTIC) {
        result = doc_markdown_shadow_begin(t, &native);
        if (result == XUI_OK) target = &native;
    }
    for (i = count; i > 0 && result == XUI_OK; i--) {
        xui_doc_position_t caret;
        const char* value = expansions ? expansions[i - 1].text : replacement;
        uint64_t value_bytes = expansions ? expansions[i - 1].bytes : replacement_bytes;
        if (t->domain == XUI_DOC_SOURCE)
            result = doc_txn_source_patch(t, matches[i - 1].tAnchor.iOffset, matches[i - 1].tCaret.iOffset, value, value_bytes, 0);
        else result = xuiDocumentTxnReplaceRange(target, &matches[i - 1], value, value_bytes, &caret);
    }
    if (result == XUI_OK && t->domain == XUI_DOC_SOURCE && t->parse_op_start < t->count) result = doc_markdown_parse(t);
    if (target == &native) result = doc_markdown_shadow_end(t, &native, result, NULL, NULL);
    if (result == XUI_OK) *replaced = count;
    doc_find_expansions_free(expansions, count);
    doc_free(matches); return result == XUI_OK ? result : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnReplaceAll(xui_document_transaction t,
    const char* pattern, uint64_t pattern_bytes,
    const char* replacement, uint64_t replacement_bytes,
    const xui_doc_range_t* scope, uint64_t* replaced)
{
    return xuiDocumentTxnReplaceAllEx(t, pattern, pattern_bytes,
        replacement, replacement_bytes, 0, scope, replaced);
}

#endif
