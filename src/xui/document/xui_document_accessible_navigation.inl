/* Range operations use the View's existing committed plaintext projection.
 * Only endpoints are retained; query results follow normal Document changes. */
static int doc_text_peer(doc_accessible_text_range* range,
    xui_internal_accessible_text_range* other, uint64_t* a, uint64_t* b,
    uint64_t* c, uint64_t* d)
{
    doc_accessible_text_range* peer; xui_doc_range_t mapped; int result;
    if (!other || other->ops != &doc_text_ops) return XUI_ERROR_INVALID_ARGUMENT;
    peer = doc_text_cast(other);
    if (!peer->owner || peer->stale) return XUI_DOC_ERROR_STALE;
    if (range->owner != peer->owner) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_text_project(range, &mapped, a, b);
    if (result == XUI_OK) result = doc_text_project(peer, &mapped, c, d);
    return result;
}
static int doc_text_compare(xui_internal_accessible_text_range* base,
    xui_internal_accessible_text_range* other, int* equal)
{
    doc_accessible_text_range* range = doc_text_cast(base); xui_context context;
    uint64_t a, b, c, d; int result;
    if (!equal) return XUI_ERROR_INVALID_ARGUMENT;
    *equal = 0; result = doc_text_begin(range, &context, 0);
    if (result == XUI_OK) result = doc_text_peer(range, other, &a, &b, &c, &d);
    if (result == XUI_OK) *equal = a == c && b == d;
    return doc_text_end(context, result);
}
static int doc_text_endpoint_valid(int endpoint)
{ return endpoint == XUI_INTERNAL_TEXT_START || endpoint == XUI_INTERNAL_TEXT_END; }
static int doc_text_compare_endpoints(xui_internal_accessible_text_range* base, int endpoint,
    xui_internal_accessible_text_range* other, int other_endpoint, int* order)
{
    doc_accessible_text_range* range = doc_text_cast(base); xui_context context;
    uint64_t a, b, c, d, left, right; int result;
    if (!order) return XUI_ERROR_INVALID_ARGUMENT;
    *order = 0;
    if (!doc_text_endpoint_valid(endpoint) || !doc_text_endpoint_valid(other_endpoint))
        return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_text_begin(range, &context, 0);
    if (result == XUI_OK) result = doc_text_peer(range, other, &a, &b, &c, &d);
    if (result == XUI_OK) {
        left = endpoint ? b : a; right = other_endpoint ? d : c;
        *order = (left > right) - (left < right);
    }
    return doc_text_end(context, result);
}
static void doc_text_assign(doc_accessible_text_range* range, uint64_t first, uint64_t last)
{
    doc_plain_projection* projection = &range->owner->edit_projection;
    range->positions.tAnchor = doc_plain_unproject(projection, first, 1);
    range->positions.tCaret = first == last ? range->positions.tAnchor :
        doc_plain_unproject(projection, last, 0);
    doc_text_tag_positions(range->owner, &range->positions);
}
static int doc_text_set_endpoint(xui_internal_accessible_text_range* base, int endpoint,
    xui_internal_accessible_text_range* other, int other_endpoint)
{
    doc_accessible_text_range* range = doc_text_cast(base); xui_context context;
    uint64_t a, b, c, d, target; int result;
    if (!doc_text_endpoint_valid(endpoint) || !doc_text_endpoint_valid(other_endpoint))
        return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_text_begin(range, &context, 0);
    if (result == XUI_OK) result = doc_text_peer(range, other, &a, &b, &c, &d);
    if (result == XUI_OK) {
        target = other_endpoint ? d : c;
        if (endpoint) { b = target; if (a > b) a = b; }
        else { a = target; if (b < a) b = a; }
        doc_text_assign(range, a, b);
    }
    return doc_text_end(context, result);
}
static int doc_text_find_error(int fallback)
{
    const xerror* error = xrtGetError();
    if (error && xrtErrorIs(error, XERR_MEMORY)) return XUI_ERROR_OUT_OF_MEMORY;
    if (error && xrtErrorFind(error, "xrt.regex", XREGEX_ERROR_LIMIT)) return XUI_DOC_ERROR_LIMIT;
    return fallback;
}
static int doc_text_find(xui_internal_accessible_text_range* base, const char* needle,
    uint64_t bytes, int backward, int ignore_case, xui_internal_accessible_text_range** out)
{
    doc_accessible_text_range* range = doc_text_cast(base); xui_context context;
    xui_doc_range_t mapped, found; uint64_t a, b, start = 0, end = 0;
    xregexconfig config; xregex* regex = NULL; xregexmatcher* matcher = NULL;
    xstrview text; char* escaped = NULL; size_t escaped_bytes = 0, offset = 0;
    xregexresult state; int result, matched = 0;
    if (!out) return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    if (!bytes || bytes > SIZE_MAX || !doc_utf8(needle, bytes) ||
        (backward != 0 && backward != 1) || (ignore_case != 0 && ignore_case != 1))
        return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_text_begin(range, &context, 0);
    if (result == XUI_OK) result = doc_text_project(range, &mapped, &a, &b);
    if (result != XUI_OK) goto done;
    escaped = xrtRegexEscape((xstrview){needle, (size_t)bytes}, &escaped_bytes);
    if (!escaped) { result = doc_text_find_error(XUI_ERROR_OUT_OF_MEMORY); goto done; }
    xrtRegexConfigInit(&config);
    if (ignore_case) config.Flags |= XREGEX_IGNORE_CASE;
    regex = xrtRegexCompileConfig((xstrview){escaped, escaped_bytes}, &config);
    if (!regex) { result = doc_text_find_error(XUI_ERROR_INVALID_ARGUMENT); goto done; }
    matcher = xrtRegexMatcherCreate(regex);
    if (!matcher) { result = doc_text_find_error(XUI_ERROR_OUT_OF_MEMORY); goto done; }
    text = (xstrview){range->owner->edit_projection.text + a, (size_t)(b - a)};
    state = xrtRegexMatcherFind(matcher, text, 0);
    while (state == XREGEX_MATCH) {
        xregexcapture capture = {0};
        if (!xrtRegexMatcherCapture(matcher, 0, &capture) || !capture.Matched ||
            capture.Span.Begin >= capture.Span.End || capture.Span.End > text.Size) {
            result = XUI_ERROR_INVALID_STATE; goto done;
        }
        matched = 1; start = a + capture.Span.Begin; end = a + capture.Span.End;
        if (!backward) break;
        /* Restart after the first scalar, rather than after the match: the
         * last occurrence can overlap a previous occurrence ("aba"/"ababa"). */
        offset = capture.Span.Begin + 1;
        while (offset < text.Size && ((unsigned char)text.Data[offset] & 0xc0) == 0x80) offset++;
        state = xrtRegexMatcherFind(matcher, text, offset);
    }
    if (state == XREGEX_ERROR) { result = doc_text_find_error(XUI_ERROR_INVALID_STATE); goto done; }
    if (matched) {
        found.tAnchor = doc_plain_unproject(&range->owner->edit_projection, start, 1);
        found.tCaret = doc_plain_unproject(&range->owner->edit_projection, end, 0);
        result = doc_text_new(range->owner, &found, out);
    }
done:
    xrtRegexMatcherFree(matcher); xrtRegexRelease(regex); xrtFree(escaped);
    return doc_text_end(context, result);
}
static int doc_text_read64(void* user, int64_t offset, unsigned char* byte)
{
    doc_plain_projection* projection = user;
    if (offset < 0 || (uint64_t)offset >= projection->bytes) return 0;
    *byte = (unsigned char)projection->text[offset]; return 1;
}
#include "xui_document_accessible_lines.inl"
#include "xui_document_accessible_paragraphs.inl"
#include "xui_document_accessible_formats.inl"
static int doc_text_unit_supported(int unit)
{
    if (unit < XUI_INTERNAL_TEXT_GRAPHEME || unit > XUI_INTERNAL_TEXT_ALL)
        return XUI_ERROR_INVALID_ARGUMENT;
    return unit == XUI_INTERNAL_TEXT_GRAPHEME || unit == XUI_INTERNAL_TEXT_FORMAT || unit == XUI_INTERNAL_TEXT_WORD ||
        unit == XUI_INTERNAL_TEXT_LINE || unit == XUI_INTERNAL_TEXT_PARAGRAPH ||
        unit == XUI_INTERNAL_TEXT_ALL ? XUI_OK : XUI_ERROR_UNSUPPORTED;
}
static uint64_t doc_text_simple_step(doc_plain_projection* p, int unit, uint64_t at, int right)
{
    if (unit == XUI_INTERNAL_TEXT_ALL) return right ? p->bytes : 0;
    if (unit == XUI_INTERNAL_TEXT_GRAPHEME)
        return (uint64_t)(right ? xuiInternalTextGraphemeNext64Read(doc_text_read64, p, (int64_t)p->bytes, (int64_t)at) :
            xuiInternalTextGraphemePrev64Read(doc_text_read64, p, (int64_t)p->bytes, (int64_t)at));
    return (uint64_t)(right ? xuiInternalTextWordNext64Read(doc_text_read64, p, (int64_t)p->bytes, (int64_t)at,
        XUI_INTERNAL_WORD_NATURAL) : xuiInternalTextWordPrev64Read(doc_text_read64, p, (int64_t)p->bytes,
        (int64_t)at, XUI_INTERNAL_WORD_NATURAL));
}
static int doc_text_step(doc_view_data* view, int unit, uint64_t at, int right, uint64_t* next)
{
    doc_plain_projection* p = &view->edit_projection; uint64_t a, b; int result;
    if (unit != XUI_INTERNAL_TEXT_LINE && unit != XUI_INTERNAL_TEXT_PARAGRAPH && unit != XUI_INTERNAL_TEXT_FORMAT) {
        *next = doc_text_simple_step(p, unit, at, right); return XUI_OK;
    }
    if ((right && at == p->bytes) || (!right && !at)) { *next = at; return XUI_OK; }
    result = unit == XUI_INTERNAL_TEXT_LINE ? doc_text_line_span(view, at, &a, &b) :
        unit == XUI_INTERNAL_TEXT_FORMAT ? doc_text_format_span(view, at, &a, &b) : doc_text_paragraph_span(view, at, &a, &b);
    if (result != XUI_OK) return result;
    if (!right && a == at) {
        result = unit == XUI_INTERNAL_TEXT_LINE ?
            doc_text_line_span(view, doc_text_scalar_before(p, at), &a, &b) :
            unit == XUI_INTERNAL_TEXT_FORMAT ? doc_text_format_span(view, doc_text_scalar_before(p, at), &a, &b) :
            doc_text_paragraph_span(view, doc_text_scalar_before(p, at), &a, &b);
        if (result != XUI_OK) return result;
    }
    *next = right ? b : a; return XUI_OK;
}
static int doc_text_unit_span(doc_view_data* view, int unit, uint64_t at,
    uint64_t* first, uint64_t* last)
{
    doc_plain_projection* p = &view->edit_projection;
    if (unit == XUI_INTERNAL_TEXT_LINE) return doc_text_line_span(view, at, first, last);
    if (unit == XUI_INTERNAL_TEXT_PARAGRAPH) return doc_text_paragraph_span(view, at, first, last);
    if (unit == XUI_INTERNAL_TEXT_FORMAT) return doc_text_format_span(view, at, first, last);
    if (!p->bytes || unit == XUI_INTERNAL_TEXT_ALL) { *first = 0; *last = p->bytes; return XUI_OK; }
    if (unit == XUI_INTERNAL_TEXT_GRAPHEME) {
        *first = at == p->bytes ? doc_text_simple_step(p, unit, at, 0) :
            (uint64_t)xuiInternalTextGraphemeClamp64Read(doc_text_read64, p, (int64_t)p->bytes, (int64_t)at);
    } else {
        int64_t a, b;
        xui_internal_word_kind_t kind = xuiInternalTextWordRange64Read(doc_text_read64, p,
            (int64_t)p->bytes, (int64_t)at, XUI_INTERNAL_WORD_NATURAL, &a, &b);
        *first = kind == XUI_INTERNAL_WORD_TEXT ? (uint64_t)a : doc_text_simple_step(p, unit, at, 0);
    }
    *last = doc_text_simple_step(p, unit, *first, 1); return XUI_OK;
}
static int doc_text_expand(xui_internal_accessible_text_range* base, int unit)
{
    doc_accessible_text_range* range = doc_text_cast(base); xui_context context;
    xui_doc_range_t mapped; uint64_t a, b; int result = doc_text_unit_supported(unit);
    if (result != XUI_OK) return result;
    result = doc_text_begin(range, &context, 0);
    if (result == XUI_OK && unit == XUI_INTERNAL_TEXT_FORMAT) {
        result = doc_view_refresh_resources(range->owner);
        if (result == XUI_OK) doc_view_resolve_style(range->owner);
    }
    if (result == XUI_OK) result = doc_text_project(range, &mapped, &a, &b);
    if (result == XUI_OK && range->owner->edit_projection.bytes > INT64_MAX) result = XUI_DOC_ERROR_LIMIT;
    if (result == XUI_OK && (unit == XUI_INTERNAL_TEXT_LINE ||
        (unit == XUI_INTERNAL_TEXT_FORMAT && range->owner->renderer->mode == XUI_DOC_LIVE_MARKDOWN)))
        result = doc_view_layout(range->owner);
    if (result == XUI_OK) {
        result = doc_text_unit_span(range->owner, unit, a, &a, &b);
        if (result == XUI_OK) doc_text_assign(range, a, b);
    }
    return doc_text_end(context, result);
}
static int doc_text_move_common(xui_internal_accessible_text_range* base, int endpoint,
    int unit, int count, int* actual)
{
    doc_accessible_text_range* range = doc_text_cast(base); xui_context context;
    doc_plain_projection* p; xui_doc_range_t mapped; uint64_t a, b, at, next, stop;
    int result, moved = 0, right = count > 0;
    if (!actual) return XUI_ERROR_INVALID_ARGUMENT;
    *actual = 0; result = doc_text_unit_supported(unit); if (result != XUI_OK) return result;
    result = doc_text_begin(range, &context, 0);
    if (result == XUI_OK && unit == XUI_INTERNAL_TEXT_FORMAT && count) {
        result = doc_view_refresh_resources(range->owner);
        if (result == XUI_OK) doc_view_resolve_style(range->owner);
    }
    if (result == XUI_OK) result = doc_text_project(range, &mapped, &a, &b);
    if (result != XUI_OK) return doc_text_end(context, result);
    p = &range->owner->edit_projection;
    if (p->bytes > INT64_MAX) return doc_text_end(context, XUI_DOC_ERROR_LIMIT);
    if (!count) return doc_text_end(context, XUI_OK);
    if (unit == XUI_INTERNAL_TEXT_LINE || (unit == XUI_INTERNAL_TEXT_FORMAT &&
        range->owner->renderer->mode == XUI_DOC_LIVE_MARKDOWN)) {
        result = doc_view_layout(range->owner);
        if (result != XUI_OK) return doc_text_end(context, result);
    }
    at = endpoint == XUI_INTERNAL_TEXT_END ? b : a;
    if (endpoint < 0 && a != b) {
        result = doc_text_unit_span(range->owner, unit, at, &at, &stop);
        if (result != XUI_OK) return doc_text_end(context, result);
    }
    while (right ? moved < count : moved > count) {
        result = doc_text_step(range->owner, unit, at, right, &next);
        if (result != XUI_OK) return doc_text_end(context, result);
        if (next == at || (endpoint < 0 && a != b && next == p->bytes)) break;
        at = next; moved += right ? 1 : -1;
    }
    if (moved) {
        if (endpoint < 0) {
            if (a == b) a = b = at;
            else {
                a = at; result = doc_text_step(range->owner, unit, at, 1, &b);
                if (result != XUI_OK) return doc_text_end(context, result);
            }
        } else if (endpoint == XUI_INTERNAL_TEXT_START) { a = at; if (b < a) b = a; }
        else { b = at; if (a > b) a = b; }
        doc_text_assign(range, a, b);
    }
    *actual = moved; return doc_text_end(context, XUI_OK);
}
static int doc_text_move(xui_internal_accessible_text_range* base, int unit, int count, int* actual)
{ return doc_text_move_common(base, -1, unit, count, actual); }
static int doc_text_move_endpoint(xui_internal_accessible_text_range* base, int endpoint,
    int unit, int count, int* actual)
{
    if (actual) *actual = 0;
    if (!doc_text_endpoint_valid(endpoint)) return XUI_ERROR_INVALID_ARGUMENT;
    return doc_text_move_common(base, endpoint, unit, count, actual);
}
