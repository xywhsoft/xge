#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include <math.h>

typedef struct doc_mark_target { uint64_t id, start, end; } doc_mark_target;
typedef struct doc_mark_collect {
    doc_mark_target* targets;
    uint64_t count, capacity;
    const xui_doc_position_t *first, *last;
    int error;
} doc_mark_collect;
static void doc_collect_marks(doc_state* s, uint64_t id, doc_mark_collect* c)
{
    doc_node* n = doc_index_get(s->index, id);
    xui_doc_position_t first = *c->first, last = *c->first; uint64_t i; int before, after;
    if (c->error) return;
    first.iNodeId = last.iNodeId = id; first.iOffset = 0;
    first.iKind = last.iKind = doc_text_kind(n->kind) ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
    last.iOffset = doc_text_kind(n->kind) ? doc_seq_size(n->text) : doc_seq_size(n->children);
    doc_position_compare(s, &last, c->first, &before); doc_position_compare(s, &first, c->last, &after);
    if (before <= 0 || after >= 0) return;
    if (n->kind == XUI_DOC_TEXT || n->kind == XUI_DOC_SOFT_BREAK || n->kind == XUI_DOC_HARD_BREAK) {
        if (c->count == c->capacity) {
            uint64_t capacity = c->capacity ? c->capacity * 2 : 16; doc_mark_target* targets;
            if (capacity < c->capacity || capacity > SIZE_MAX / sizeof(*targets)) { c->error = XUI_DOC_ERROR_LIMIT; return; }
            targets = doc_realloc(s->allocator, c->targets, (size_t)capacity * sizeof(*targets));
            if (!targets) { c->error = XUI_ERROR_OUT_OF_MEMORY; return; }
            c->targets = targets; c->capacity = capacity;
        }
        doc_mark_target* target = &c->targets[c->count++];
        target->id = id; target->start = id == c->first->iNodeId ? c->first->iOffset : 0;
        target->end = id == c->last->iNodeId ? c->last->iOffset : doc_seq_size(n->text);
    }
    for (i = 0; i < doc_seq_size(n->children) && !c->error; i++) doc_collect_marks(s, doc_seq_get_id(n->children, i), c);
}
int doc_marks_range(xui_document_transaction t, const xui_doc_range_t* range, uint32_t set, uint32_t clear)
{
    doc_mark_collect c = {0}; xui_doc_range_t part = *range;
    uint64_t i; int order, result;
    result = doc_position_compare(t->draft, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    c.first = order < 0 ? &range->tAnchor : &range->tCaret; c.last = order < 0 ? &range->tCaret : &range->tAnchor;
    doc_collect_marks(t->draft, DOC_ROOT, &c);
    result = c.error;
    part.tAnchor.iKind = part.tCaret.iKind = XUI_DOC_POSITION_TEXT;
    for (i = c.count; i > 0 && result == XUI_OK; i--) {
        doc_mark_target* target = &c.targets[i - 1];
        doc_node* node = doc_index_get(t->draft->index, target->id);
        if (node->kind != XUI_DOC_TEXT) {
            xui_doc_attributes_t attrs = *node->attrs;
            attrs.iMarks = (attrs.iMarks | set) & ~clear;
            result = xuiDocumentTxnSetAttributes(t, target->id, &attrs);
            continue;
        }
        part.tAnchor.iNodeId = part.tCaret.iNodeId = target->id;
        part.tAnchor.iOffset = target->start; part.tCaret.iOffset = target->end;
        result = xuiDocumentTxnSetMarks(t, &part, set, clear);
        if (result != XUI_OK) break;
    }
    doc_free(c.targets); return result == XUI_OK ? result : doc_txn_fail(t, result);
}
static int doc_link_range_apply(xui_document_transaction t, const xui_doc_range_t* range,
    const char* uri, const char* title)
{
    doc_mark_collect c = {0}; uint64_t i; int order, result;
    result = doc_position_compare(t->draft, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (!order) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    c.first = order < 0 ? &range->tAnchor : &range->tCaret;
    c.last = order < 0 ? &range->tCaret : &range->tAnchor;
    doc_collect_marks(t->draft, DOC_ROOT, &c);
    result = c.error;
    if (result == XUI_OK && !c.count) result = XUI_ERROR_UNSUPPORTED;
    /* Work right-to-left so splitting a run cannot invalidate the remaining
     * target IDs or offsets. All edits stay in the transaction draft. */
    for (i = c.count; i && result == XUI_OK; i--) {
        doc_mark_target* target = &c.targets[i - 1];
        doc_node* node = doc_index_get(t->draft->index, target->id);
        xui_doc_attributes_t attrs;
        uint64_t id = target->id, tail = 0, length;
        if (!node) { result = XUI_ERROR_NOT_FOUND; break; }
        if (node->kind == XUI_DOC_TEXT) {
            length = doc_seq_size(node->text);
            if (target->start > target->end || target->end > length ||
                !doc_seq_boundary(node->text, target->start) || !doc_seq_boundary(node->text, target->end)) {
                result = XUI_DOC_ERROR_UTF8; break;
            }
            if (target->start == target->end ||
                (!!(node->attrs->iMarks & XUI_DOC_LINK) == !!*uri &&
                 !strcmp(doc_string(node->resource), uri) &&
                 !strcmp(doc_string(node->title), *uri ? title : ""))) continue;
            if (target->end < length) result = doc_split_text(t, id, target->end, &tail);
            if (result == XUI_OK && target->start) {
                result = doc_split_text(t, id, target->start, &tail);
                if (result == XUI_OK) id = tail;
            }
        } else if (node->kind != XUI_DOC_SOFT_BREAK && node->kind != XUI_DOC_HARD_BREAK) {
            result = XUI_ERROR_UNSUPPORTED;
        } else if (t->draft->profile == XUI_DOCUMENT_MARKDOWN) {
            result = XUI_DOC_ERROR_UNREPRESENTABLE;
        }
        if (result != XUI_OK) break;
        node = doc_index_get(t->draft->index, id);
        attrs = *node->attrs;
        if (*uri) attrs.iMarks |= XUI_DOC_LINK;
        else attrs.iMarks &= ~XUI_DOC_LINK;
        result = xuiDocumentTxnSetAttributes(t, id, &attrs);
        if (result == XUI_OK) {
            node = doc_index_get(t->draft->index, id);
            result = xuiDocumentTxnSetResource(t, id, *uri ? uri : "",
                doc_string(node->info), *uri ? title : "");
        }
    }
    doc_free(c.targets);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnSetLink(xui_document_transaction t, const xui_doc_range_t* range,
    const char* uri, const char* title)
{
    doc_blob *uri_copy = NULL, *title_copy = NULL;
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (!range || range->tAnchor.iSize != sizeof(range->tAnchor) ||
        range->tCaret.iSize != sizeof(range->tCaret) ||
        range->tAnchor.iDocumentId != t->document->identity ||
        range->tCaret.iDocumentId != t->document->identity ||
        range->tAnchor.iRevision != t->base_revision ||
        range->tCaret.iRevision != t->base_revision)
        return doc_txn_fail(t, XUI_DOC_ERROR_STALE);
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE ||
        range->tCaret.iKind == XUI_DOC_POSITION_SOURCE)
        return doc_txn_fail(t, XUI_DOC_ERROR_DOMAIN);
    if (!uri) uri = "";
    if (!title) title = "";
    if ((!*uri && *title) || !doc_utf8(uri, strlen(uri)) || !doc_utf8(title, strlen(title)))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    /* The arguments may point into a retained node. Splitting that node can
     * release its last resource blob before every selected run is updated. */
    if (*uri && !(uri_copy = doc_blob_new(t->draft->allocator, uri, strlen(uri))))
        return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    if (*title && !(title_copy = doc_blob_new(t->draft->allocator, title, strlen(title)))) {
        doc_blob_release(uri_copy); return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    }
    uri = doc_string(uri_copy); title = doc_string(title_copy);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing)
        result = doc_markdown_link(t, range, uri, title);
    else result = doc_link_range_apply(t, range, uri, title);
    doc_blob_release(uri_copy); doc_blob_release(title_copy);
    return result;
}
XUI_API int xuiDocumentTxnInsertLink(xui_document_transaction t, const xui_doc_range_t* range,
    const char* label, uint64_t label_bytes, const char* uri, const char* title, xui_doc_position_t* caret)
{
    doc_blob *label_copy = NULL, *uri_copy = NULL, *title_copy = NULL;
    doc_state* before_link = NULL; xui_doc_position_t end, mapped;
    xui_doc_range_t inserted; uint64_t first_op; int mapping, result;
    struct xui_doc_change_set_t changes = {0};
    result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (!range || !caret || !label || !label_bytes || !uri || !*uri ||
        label_bytes >= SIZE_MAX || !doc_utf8(label, label_bytes) ||
        memchr(label, '\n', (size_t)label_bytes) || memchr(label, '\r', (size_t)label_bytes) ||
        !doc_utf8(uri, strlen(uri)) || (title && !doc_utf8(title, strlen(title))))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (!(label_copy = doc_blob_new(t->draft->allocator, label, label_bytes)) ||
        !(uri_copy = doc_blob_new(t->draft->allocator, uri, strlen(uri))) ||
        (title && *title && !(title_copy = doc_blob_new(t->draft->allocator, title, strlen(title))))) {
        result = XUI_ERROR_OUT_OF_MEMORY; goto done;
    }
    result = xuiDocumentTxnReplaceRange(t, range, doc_string(label_copy), label_bytes, &end);
    if (result != XUI_OK) goto done;
    if (end.iKind != XUI_DOC_POSITION_TEXT || end.iOffset < label_bytes) {
        result = XUI_DOC_ERROR_UNREPRESENTABLE; goto done;
    }
    inserted.tAnchor = inserted.tCaret = end;
    inserted.tAnchor.iOffset -= label_bytes;
    if (!doc_position_valid(t->draft, &inserted.tAnchor)) {
        result = XUI_DOC_ERROR_UNREPRESENTABLE; goto done;
    }
    before_link = doc_state_clone(t->draft);
    if (!before_link) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    first_op = t->count;
    result = xuiDocumentTxnSetLink(t, &inserted, doc_string(uri_copy), doc_string(title_copy));
    if (result != XUI_OK) goto done;
    changes.before = before_link; changes.after = t->draft;
    changes.identity = t->document->identity;
    changes.before_revision = changes.after_revision = t->base_revision;
    changes.domain = XUI_DOC_SEMANTIC; changes.ops = t->ops + first_op; changes.count = t->count - first_op;
    result = xuiDocumentMapPosition(&changes, &end, &mapped, &mapping);
    if (result == XUI_OK && (!doc_position_valid(t->draft, &mapped) || mapping == XUI_DOC_MAP_DELETED))
        result = XUI_DOC_ERROR_UNREPRESENTABLE;
    if (result == XUI_OK) *caret = mapped;
done:
    doc_state_release(before_link);
    doc_blob_release(label_copy); doc_blob_release(uri_copy); doc_blob_release(title_copy);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentSnapshotQueryMarks(xui_document_snapshot snapshot, const xui_doc_range_t* range, uint32_t* common, uint32_t* mixed)
{
    doc_mark_collect c = {0}; int order, result; uint64_t i; uint32_t any = 0;
    if (!snapshot || !range || !common || !mixed) return XUI_ERROR_INVALID_ARGUMENT;
    *common = *mixed = 0;
    result = xuiDocumentSnapshotComparePositions(snapshot, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE) return XUI_DOC_ERROR_DOMAIN;
    if (!order) {
        doc_node* n = doc_index_get(snapshot->state->index, range->tCaret.iNodeId);
        if (range->tCaret.iKind == XUI_DOC_POSITION_GAP && doc_seq_size(n->children)) {
            uint64_t index = range->tCaret.iOffset ? range->tCaret.iOffset - 1 : 0;
            n = doc_index_get(snapshot->state->index, doc_seq_get_id(n->children, index));
        }
        if (n && n->kind == XUI_DOC_TEXT) *common = n->attrs->iMarks;
        return XUI_OK;
    }
    c.first = order < 0 ? &range->tAnchor : &range->tCaret; c.last = order < 0 ? &range->tCaret : &range->tAnchor;
    doc_collect_marks(snapshot->state, DOC_ROOT, &c);
    if (!c.error && c.count) {
        *common = UINT32_MAX;
        for (i = 0; i < c.count; i++) {
            uint32_t marks = doc_index_get(snapshot->state->index, c.targets[i].id)->attrs->iMarks;
            any |= marks; *common &= marks;
        }
        *mixed = any & ~*common;
    }
    doc_free(c.targets); return c.error;
}
XUI_API int xuiDocumentSnapshotQueryLink(xui_document_snapshot snapshot, const xui_doc_range_t* range,
    xui_doc_link_info_t* out)
{
    doc_mark_collect c = {0}; int order, result, saw = 0, any = 0, all = 1, different = 0;
    int first_linked = 0; const char *uri = NULL, *title = NULL; uint64_t i;
    if (!snapshot || !range || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out);
    result = xuiDocumentSnapshotComparePositions(snapshot, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE || range->tCaret.iKind == XUI_DOC_POSITION_SOURCE)
        return XUI_DOC_ERROR_DOMAIN;
    if (!order) {
        doc_node* n = doc_index_get(snapshot->state->index, range->tCaret.iNodeId);
        if (n && range->tCaret.iKind == XUI_DOC_POSITION_GAP && doc_seq_size(n->children)) {
            uint64_t index = range->tCaret.iOffset ? range->tCaret.iOffset - 1 : 0;
            n = doc_index_get(snapshot->state->index, doc_seq_get_id(n->children, index));
        }
        if (n && n->kind == XUI_DOC_TEXT && (n->attrs->iMarks & XUI_DOC_LINK)) {
            out->bLinked = 1; out->sUri = doc_string(n->resource); out->sTitle = doc_string(n->title);
        }
        return XUI_OK;
    }
    c.first = order < 0 ? &range->tAnchor : &range->tCaret;
    c.last = order < 0 ? &range->tCaret : &range->tAnchor;
    doc_collect_marks(snapshot->state, DOC_ROOT, &c);
    if (c.error) { doc_free(c.targets); return c.error; }
    for (i = 0; i < c.count; i++) {
        doc_node* n = doc_index_get(snapshot->state->index, c.targets[i].id);
        int linked = !!(n->attrs->iMarks & XUI_DOC_LINK);
        if (n->kind == XUI_DOC_TEXT && c.targets[i].start == c.targets[i].end) continue;
        if (!saw) {
            first_linked = linked; uri = linked ? doc_string(n->resource) : NULL;
            title = linked ? doc_string(n->title) : NULL;
        } else if (linked != first_linked || (linked &&
            (strcmp(uri, doc_string(n->resource)) || strcmp(title, doc_string(n->title))))) different = 1;
        saw = 1; any |= linked; all &= linked;
    }
    out->bLinked = saw && any && all;
    out->bMixed = different;
    if (out->bLinked && !out->bMixed) { out->sUri = uri; out->sTitle = title; }
    doc_free(c.targets); return XUI_OK;
}
int doc_text_style_valid(uint32_t fields, const xui_doc_text_style_t* style)
{
    if (!style || style->iSize != sizeof(*style) || !fields ||
        fields & ~(XUI_DOC_TEXT_STYLE_COLOR | XUI_DOC_TEXT_STYLE_BACKGROUND |
            XUI_DOC_TEXT_STYLE_FONT_SIZE | XUI_DOC_TEXT_STYLE_FONT_FAMILY |
            XUI_DOC_TEXT_STYLE_LANGUAGE)) return 0;
    if (style->iExplicitFields & fields &
        ~(XUI_DOC_TEXT_STYLE_COLOR | XUI_DOC_TEXT_STYLE_BACKGROUND)) return 0;
    if (style->iCurrentColorFields &
        ~(fields & (XUI_DOC_TEXT_STYLE_COLOR | XUI_DOC_TEXT_STYLE_BACKGROUND))) return 0;
    if (style->iCurrentColorFields & style->iExplicitFields) return 0;
    if ((style->iCurrentColorFields & XUI_DOC_TEXT_STYLE_COLOR) && style->iTextColor) return 0;
    if ((style->iCurrentColorFields & XUI_DOC_TEXT_STYLE_BACKGROUND) && style->iBackgroundColor) return 0;
    if (fields & XUI_DOC_TEXT_STYLE_FONT_SIZE && (!isfinite(style->fFontSize) ||
        style->fFontSize < 0 || style->fFontSize > DOC_MAX_LAYOUT_VALUE)) return 0;
    if (fields & XUI_DOC_TEXT_STYLE_FONT_FAMILY) {
        const char* end = memchr(style->sFontFamily, 0, sizeof(style->sFontFamily));
        if (!end || !doc_utf8(style->sFontFamily, (uint64_t)(end - style->sFontFamily))) return 0;
    }
    if (fields & XUI_DOC_TEXT_STYLE_LANGUAGE &&
        (!memchr(style->sLanguage, 0, sizeof(style->sLanguage)) ||
         !doc_language_valid(style->sLanguage))) return 0;
    return 1;
}
static void doc_text_style_apply(xui_doc_attributes_t* attrs, uint32_t fields,
    const xui_doc_text_style_t* style)
{
    if (fields & XUI_DOC_TEXT_STYLE_COLOR) {
        attrs->iTextColor = style->iTextColor;
        attrs->iFlags &= ~(XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO | XUI_DOC_TEXT_COLOR_CURRENT);
        if (style->iCurrentColorFields & XUI_DOC_TEXT_STYLE_COLOR)
            attrs->iFlags |= XUI_DOC_TEXT_COLOR_CURRENT;
        if (!style->iTextColor && (style->iExplicitFields & XUI_DOC_TEXT_STYLE_COLOR))
            attrs->iFlags |= XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO;
    }
    if (fields & XUI_DOC_TEXT_STYLE_BACKGROUND) {
        attrs->iBackgroundColor = style->iBackgroundColor;
        attrs->iFlags &= ~(XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO |
            XUI_DOC_BACKGROUND_COLOR_CURRENT);
        if (style->iCurrentColorFields & XUI_DOC_TEXT_STYLE_BACKGROUND)
            attrs->iFlags |= XUI_DOC_BACKGROUND_COLOR_CURRENT;
        if (!style->iBackgroundColor &&
            (style->iExplicitFields & XUI_DOC_TEXT_STYLE_BACKGROUND))
            attrs->iFlags |= XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO;
    }
    if (fields & XUI_DOC_TEXT_STYLE_FONT_SIZE) attrs->fFontSize = style->fFontSize;
    if (fields & XUI_DOC_TEXT_STYLE_LANGUAGE)
        attrs->sLanguage = style->sLanguage[0] ? style->sLanguage : NULL;
    if (fields & XUI_DOC_TEXT_STYLE_FONT_FAMILY) {
        size_t bytes = strlen(style->sFontFamily);
        memset(attrs->sFontFamily, 0, sizeof(attrs->sFontFamily));
        memcpy(attrs->sFontFamily, style->sFontFamily, bytes);
    }
}
static void doc_text_style_read(xui_doc_text_style_t* style,
    const doc_state* state, const doc_node* node)
{
    xui_doc_attributes_t attrs = doc_effective_text_attrs(state, node);
    memset(style, 0, sizeof(*style));
    style->iSize = sizeof(*style);
    if (attrs.iFlags & XUI_DOC_TEXT_COLOR_CURRENT)
        style->iCurrentColorFields |= XUI_DOC_TEXT_STYLE_COLOR;
    else {
        style->iTextColor = attrs.iTextColor;
        if (doc_text_color_set(&attrs)) style->iExplicitFields |= XUI_DOC_TEXT_STYLE_COLOR;
    }
    if (attrs.iFlags & XUI_DOC_BACKGROUND_COLOR_CURRENT)
        style->iCurrentColorFields |= XUI_DOC_TEXT_STYLE_BACKGROUND;
    else {
        style->iBackgroundColor = attrs.iBackgroundColor;
        if (doc_background_color_set(&attrs)) style->iExplicitFields |= XUI_DOC_TEXT_STYLE_BACKGROUND;
    }
    style->fFontSize = attrs.fFontSize;
    memcpy(style->sFontFamily, attrs.sFontFamily, strlen(attrs.sFontFamily) + 1);
    strcpy(style->sLanguage, doc_language(attrs.sLanguage));
}
static uint32_t doc_text_style_differences(const xui_doc_text_style_t* a,
    const xui_doc_text_style_t* b)
{
    uint32_t mixed = 0;
    if (a->iTextColor != b->iTextColor ||
        !!(a->iCurrentColorFields & XUI_DOC_TEXT_STYLE_COLOR) !=
            !!(b->iCurrentColorFields & XUI_DOC_TEXT_STYLE_COLOR) ||
        !!(a->iExplicitFields & XUI_DOC_TEXT_STYLE_COLOR) !=
            !!(b->iExplicitFields & XUI_DOC_TEXT_STYLE_COLOR))
        mixed |= XUI_DOC_TEXT_STYLE_COLOR;
    if (a->iBackgroundColor != b->iBackgroundColor ||
        !!(a->iCurrentColorFields & XUI_DOC_TEXT_STYLE_BACKGROUND) !=
            !!(b->iCurrentColorFields & XUI_DOC_TEXT_STYLE_BACKGROUND) ||
        !!(a->iExplicitFields & XUI_DOC_TEXT_STYLE_BACKGROUND) !=
            !!(b->iExplicitFields & XUI_DOC_TEXT_STYLE_BACKGROUND))
        mixed |= XUI_DOC_TEXT_STYLE_BACKGROUND;
    if (a->fFontSize != b->fFontSize) mixed |= XUI_DOC_TEXT_STYLE_FONT_SIZE;
    if (strcmp(a->sFontFamily, b->sFontFamily)) mixed |= XUI_DOC_TEXT_STYLE_FONT_FAMILY;
    if (!doc_language_equal(a->sLanguage, b->sLanguage)) mixed |= XUI_DOC_TEXT_STYLE_LANGUAGE;
    return mixed;
}
XUI_API int xuiDocumentTxnSetTextStyle(xui_document_transaction t, const xui_doc_range_t* range,
    uint32_t fields, const xui_doc_text_style_t* style)
{
    doc_mark_collect c = {0}; xui_doc_text_style_t value = {0};
    uint64_t i; int order, result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (!doc_text_style_valid(fields, style)) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (!range || range->tAnchor.iSize != sizeof(range->tAnchor) ||
        range->tCaret.iSize != sizeof(range->tCaret) ||
        range->tAnchor.iDocumentId != t->document->identity ||
        range->tCaret.iDocumentId != t->document->identity ||
        range->tAnchor.iRevision != t->base_revision ||
        range->tCaret.iRevision != t->base_revision)
        return doc_txn_fail(t, XUI_DOC_ERROR_STALE);
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE ||
        range->tCaret.iKind == XUI_DOC_POSITION_SOURCE)
        return doc_txn_fail(t, XUI_DOC_ERROR_DOMAIN);
    result = doc_position_compare(t->draft, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (!order) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing)
        return doc_txn_fail(t, XUI_DOC_ERROR_UNREPRESENTABLE);
    /* Read only selected fields: callers need not initialize unrelated ones,
     * and the input may be borrowed from a node that is about to split. */
    value.iSize = sizeof(value);
    value.iExplicitFields = style->iExplicitFields & fields;
    value.iCurrentColorFields = style->iCurrentColorFields & fields;
    if (fields & XUI_DOC_TEXT_STYLE_COLOR) value.iTextColor = style->iTextColor;
    if (fields & XUI_DOC_TEXT_STYLE_BACKGROUND) value.iBackgroundColor = style->iBackgroundColor;
    if (fields & XUI_DOC_TEXT_STYLE_FONT_SIZE) value.fFontSize = style->fFontSize;
    if (fields & XUI_DOC_TEXT_STYLE_FONT_FAMILY)
        memcpy(value.sFontFamily, style->sFontFamily, strlen(style->sFontFamily) + 1);
    if (fields & XUI_DOC_TEXT_STYLE_LANGUAGE)
        strcpy(value.sLanguage, style->sLanguage);
    c.first = order < 0 ? &range->tAnchor : &range->tCaret;
    c.last = order < 0 ? &range->tCaret : &range->tAnchor;
    doc_collect_marks(t->draft, DOC_ROOT, &c);
    result = c.error;
    if (result == XUI_OK && !c.count) result = XUI_ERROR_UNSUPPORTED;
    for (i = c.count; i && result == XUI_OK; i--) {
        doc_mark_target* target = &c.targets[i - 1];
        doc_node* node = doc_index_get(t->draft->index, target->id);
        xui_doc_attributes_t attrs;
        uint64_t id = target->id, tail = 0, length;
        if (!node) { result = XUI_ERROR_NOT_FOUND; break; }
        attrs = *node->attrs; doc_text_style_apply(&attrs, fields, &value);
        if (!doc_schema_attrs(node->kind, &attrs)) { result = XUI_DOC_ERROR_SCHEMA; break; }
        if (doc_attributes_equal(&attrs, node->attrs)) continue;
        if (node->kind == XUI_DOC_TEXT) {
            length = doc_seq_size(node->text);
            if (target->start > target->end || target->end > length ||
                !doc_seq_boundary(node->text, target->start) ||
                !doc_seq_boundary(node->text, target->end)) { result = XUI_DOC_ERROR_UTF8; break; }
            if (target->start == target->end) continue;
            if (target->end < length) result = doc_split_text(t, id, target->end, &tail);
            if (result == XUI_OK && target->start) {
                result = doc_split_text(t, id, target->start, &tail);
                if (result == XUI_OK) id = tail;
            }
        } else if (node->kind != XUI_DOC_SOFT_BREAK && node->kind != XUI_DOC_HARD_BREAK)
            result = XUI_ERROR_UNSUPPORTED;
        if (result == XUI_OK) result = xuiDocumentTxnSetAttributes(t, id, &attrs);
    }
    doc_free(c.targets);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}

XUI_API int xuiDocumentTxnClearFormatting(xui_document_transaction t,
    const xui_doc_range_t* range)
{
    doc_mark_collect c = {0}; uint64_t i;
    int order, result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (!range || range->tAnchor.iSize != sizeof(range->tAnchor) ||
        range->tCaret.iSize != sizeof(range->tCaret) ||
        range->tAnchor.iDocumentId != t->document->identity ||
        range->tCaret.iDocumentId != t->document->identity ||
        range->tAnchor.iRevision != t->base_revision ||
        range->tCaret.iRevision != t->base_revision)
        return doc_txn_fail(t, XUI_DOC_ERROR_STALE);
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE ||
        range->tCaret.iKind == XUI_DOC_POSITION_SOURCE)
        return doc_txn_fail(t, XUI_DOC_ERROR_DOMAIN);
    result = doc_position_compare(t->draft, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (!order) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow = {0};
        result = doc_markdown_shadow_begin(t, &shadow);
        if (result != XUI_OK) return result;
        result = xuiDocumentTxnClearFormatting(&shadow, range);
        return doc_markdown_shadow_end(t, &shadow, result, NULL, NULL);
    }
    c.first = order < 0 ? &range->tAnchor : &range->tCaret;
    c.last = order < 0 ? &range->tCaret : &range->tAnchor;
    doc_collect_marks(t->draft, DOC_ROOT, &c);
    result = c.error;
    if (result == XUI_OK && !c.count) result = XUI_ERROR_UNSUPPORTED;
    /* Work backwards: a split at the selected edge must not invalidate a
     * later target ID or offset. All mutations stay in the transaction draft. */
    for (i = c.count; i && result == XUI_OK; i--) {
        doc_mark_target* target = &c.targets[i - 1];
        doc_node* node = doc_index_get(t->draft->index, target->id);
        xui_doc_attributes_t attrs;
        uint64_t id = target->id, tail = 0, length;
        int had_link;
        if (!node) { result = XUI_ERROR_NOT_FOUND; break; }
        had_link = !!(node->attrs->iMarks & XUI_DOC_LINK) ||
            *doc_string(node->resource) || *doc_string(node->title);
        if (!node->attrs->iMarks && !doc_text_color_set(node->attrs) &&
            !(node->attrs->iFlags & XUI_DOC_TEXT_COLOR_CURRENT) &&
            !doc_background_color_set(node->attrs) && !node->attrs->fFontSize &&
            !*node->attrs->sFontFamily && !had_link) continue;
        if (node->kind == XUI_DOC_TEXT) {
            length = doc_seq_size(node->text);
            if (target->start > target->end || target->end > length ||
                !doc_seq_boundary(node->text, target->start) ||
                !doc_seq_boundary(node->text, target->end)) {
                result = XUI_DOC_ERROR_UTF8; break;
            }
            if (target->start == target->end) continue;
            if (target->end < length) result = doc_split_text(t, id, target->end, &tail);
            if (result == XUI_OK && target->start) {
                result = doc_split_text(t, id, target->start, &tail);
                if (result == XUI_OK) id = tail;
            }
        } else if (node->kind != XUI_DOC_SOFT_BREAK && node->kind != XUI_DOC_HARD_BREAK)
            result = XUI_ERROR_UNSUPPORTED;
        if (result != XUI_OK) break;
        node = doc_index_get(t->draft->index, id);
        attrs = *node->attrs;
        attrs.iMarks = 0;
        attrs.iTextColor = attrs.iBackgroundColor = 0;
        attrs.iFlags &= ~(XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO |
            XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO |
            XUI_DOC_TEXT_COLOR_CURRENT | XUI_DOC_BACKGROUND_COLOR_CURRENT);
        attrs.fFontSize = 0; attrs.sFontFamily[0] = 0;
        if (!doc_attributes_equal(&attrs, node->attrs))
            result = xuiDocumentTxnSetAttributes(t, id, &attrs);
        if (result == XUI_OK && had_link) {
            node = doc_index_get(t->draft->index, id);
            result = xuiDocumentTxnSetResource(t, id, "", doc_string(node->info), "");
        }
    }
    doc_free(c.targets);
    return result == XUI_OK ? result : doc_txn_fail(t, result);
}
static doc_node* doc_text_style_edge(doc_state* state, uint64_t id, int last)
{
    doc_node* node = doc_index_get(state->index, id);
    uint64_t count, i;
    if (!node) return NULL;
    if (node->kind == XUI_DOC_TEXT || node->kind == XUI_DOC_SOFT_BREAK || node->kind == XUI_DOC_HARD_BREAK)
        return node;
    count = doc_seq_size(node->children);
    for (i = 0; i < count; i++) {
        uint64_t child = doc_seq_get_id(node->children, last ? count - i - 1 : i);
        doc_node* found = doc_text_style_edge(state, child, last);
        if (found) return found;
    }
    return NULL;
}
XUI_API int xuiDocumentSnapshotQueryTextStyle(xui_document_snapshot snapshot, const xui_doc_range_t* range,
    xui_doc_text_style_query_t* out)
{
    doc_mark_collect c = {0}; int order, result; uint64_t i;
    if (!snapshot || !range || !out || out->iSize != sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    memset(out, 0, sizeof(*out)); out->iSize = sizeof(*out); out->tStyle.iSize = sizeof(out->tStyle);
    result = xuiDocumentSnapshotComparePositions(snapshot, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE ||
        range->tCaret.iKind == XUI_DOC_POSITION_SOURCE) return XUI_DOC_ERROR_DOMAIN;
    if (!order) {
        doc_node* node = doc_index_get(snapshot->state->index, range->tCaret.iNodeId);
        if (node && range->tCaret.iKind == XUI_DOC_POSITION_GAP) {
            uint64_t count = doc_seq_size(node->children), index = range->tCaret.iOffset;
            if (count) node = doc_text_style_edge(snapshot->state,
                doc_seq_get_id(node->children, index ? index - 1 : 0), !!index);
            else node = NULL;
        }
        if (node && (node->kind == XUI_DOC_TEXT || node->kind == XUI_DOC_SOFT_BREAK ||
            node->kind == XUI_DOC_HARD_BREAK)) {
            doc_text_style_read(&out->tStyle, snapshot->state, node); out->bHasText = 1;
        }
        return XUI_OK;
    }
    c.first = order < 0 ? &range->tAnchor : &range->tCaret;
    c.last = order < 0 ? &range->tCaret : &range->tAnchor;
    doc_collect_marks(snapshot->state, DOC_ROOT, &c);
    if (c.error) { doc_free(c.targets); return c.error; }
    for (i = 0; i < c.count; i++) {
        doc_node* node = doc_index_get(snapshot->state->index, c.targets[i].id);
        xui_doc_text_style_t current = {0};
        if (node->kind == XUI_DOC_TEXT && c.targets[i].start == c.targets[i].end) continue;
        doc_text_style_read(&current, snapshot->state, node);
        if (!out->bHasText) { out->tStyle = current; out->bHasText = 1; }
        else out->iMixedFields |= doc_text_style_differences(&out->tStyle, &current);
    }
    doc_free(c.targets); return XUI_OK;
}
typedef struct doc_plain { char* text; uint64_t size, capacity; int error; } doc_plain;
static void doc_plain_append(doc_plain* b, const char* s, uint64_t size)
{
    if (b->error || !size) return;
    if (size > UINT64_MAX - b->size || size > b->capacity - b->size) { b->error = XUI_DOC_ERROR_LIMIT; return; }
    memcpy(b->text + b->size, s, (size_t)size); b->size += size;
}
static void doc_plain_node(doc_state* s, uint64_t id, doc_plain* b)
{
    doc_node* n = doc_index_get(s->index, id); uint64_t i;
    if (n->text && !b->error) {
        uint64_t size = doc_seq_size(n->text);
        if (size > b->capacity - b->size) { b->error = XUI_DOC_ERROR_LIMIT; return; }
        doc_seq_read(n->text, 0, b->text + b->size, size); b->size += size;
    }
    if (n->kind == XUI_DOC_SOFT_BREAK || n->kind == XUI_DOC_HARD_BREAK) doc_plain_append(b, "\n", 1);
    for (i = 0; i < doc_seq_size(n->children); i++) {
        if (i && n->kind == XUI_DOC_ROW) doc_plain_append(b, "\t", 1);
        doc_plain_node(s, doc_seq_get_id(n->children, i), b);
    }
    if (n->kind == XUI_DOC_PARAGRAPH || n->kind == XUI_DOC_HEADING || n->kind == XUI_DOC_CODE_BLOCK || n->kind == XUI_DOC_ROW) {
        doc_node* parent = doc_index_get(s->index, n->parent);
        if (parent && parent->kind == XUI_DOC_CELL && doc_seq_get_id(parent->children, doc_seq_size(parent->children) - 1) == id) return;
        doc_plain_append(b, "\n", 1);
    }
}
XUI_API int xuiDocumentSnapshotCopyPlainText(xui_document_snapshot s, char** out, uint64_t* bytes)
{
    doc_plain b = {0};
    if (out) *out = NULL;
    if (bytes) *bytes = 0;
    if (!s || !out || !bytes) return XUI_ERROR_INVALID_ARGUMENT;
    if (s->state->node_count > (UINT64_MAX - s->state->text_bytes - 1) / 2) return XUI_DOC_ERROR_LIMIT;
    b.capacity = s->state->text_bytes + 2 * s->state->node_count;
    if (b.capacity >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    b.text = malloc((size_t)b.capacity + 1); if (!b.text) return XUI_ERROR_OUT_OF_MEMORY;
    doc_plain_node(s->state, DOC_ROOT, &b);
    if (b.error) { free(b.text); return b.error; }
    b.text[b.size] = 0; *out = b.text; *bytes = b.size; return XUI_OK;
}

#endif
