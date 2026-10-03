#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"

static int doc_copy_node_apply(xui_document_transaction t, doc_state* source, const doc_node* original,
    uint64_t parent, uint64_t index, uint64_t text_start, uint64_t text_end,
    uint64_t* copied)
{
    xui_doc_node_desc_t desc = {0};
    uint64_t text_bytes;
    char* text = NULL;
    int result;
    if (!original || text_start > text_end ||
        text_end > doc_seq_size(original->text))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    text_bytes = text_end - text_start;
    if (text_bytes >= SIZE_MAX) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    if (text_bytes) {
        text = doc_alloc(t->draft->allocator, (size_t)text_bytes + 1);
        if (!text) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
        result = doc_seq_read(original->text, text_start, text, text_bytes);
        if (result != XUI_OK) {
            doc_free(text); return doc_txn_fail(t, result);
        }
    }
    desc.iSize = sizeof(desc); desc.iKind = original->kind;
    desc.tAttributes = *original->attrs;
    if (!desc.tAttributes.sLanguage) {
        const char* language = doc_effective_text_attrs(source, original).sLanguage;
        doc_node* target = doc_index_get(t->draft->index, parent);
        const char* inherited = target ? doc_effective_text_attrs(t->draft, target).sLanguage : NULL;
        /* A copied root may lose its original ancestors or acquire new ones. */
        if (!doc_language_equal(language, inherited))
            desc.tAttributes.sLanguage = language ? language : "und";
    }
    desc.sText = text; desc.iTextBytes = text_bytes;
    desc.sResource = doc_string(original->resource);
    desc.sInfo = doc_string(original->info);
    desc.sTitle = doc_string(original->title);
    desc.sLinkTarget = original->link_target ? doc_string(original->link_target) : NULL;
    desc.sLinkTitle = original->link_title ? doc_string(original->link_title) : NULL;
    if (original->kind == XUI_DOC_EXTENSION) {
        desc.pExtensionPayload = original->extension_payload ? original->extension_payload->data : NULL;
        desc.iExtensionPayloadBytes = original->extension_payload ? original->extension_payload->size : 0;
        desc.iExtensionVersion = original->extension_version;
        desc.bExtensionRequired = original->extension_required;
    }
    result = doc_txn_insert(t, parent, index, &desc, copied);
    doc_free(text);
    return result;
}

int doc_copy_subtree_apply(xui_document_transaction t, doc_state* source,
    uint64_t source_id, uint64_t parent, uint64_t index, uint64_t* copied)
{
    doc_node* original = doc_index_get(source->index, source_id);
    uint64_t id = 0, i;
    int result;
    if (!original) return doc_txn_fail(t, XUI_ERROR_NOT_FOUND);
    result = doc_copy_node_apply(t, source, original, parent, index, 0,
        doc_seq_size(original->text), &id);
    if (result != XUI_OK) return result;
    for (i = 0; i < doc_seq_size(original->children); i++) {
        uint64_t child = doc_seq_get_id(original->children, i), inserted;
        result = doc_copy_subtree_apply(t, source, child, id, DOC_NONE, &inserted);
        if (result != XUI_OK) return result;
    }
    if (original->kind == XUI_DOC_TABLE && original->column_widths) {
        uint32_t column, columns = doc_table_column_count(source, original);
        for (column = 0; column < columns; column++) {
            float width = doc_table_column_width(original, column);
            if (!width) continue;
            result = xuiDocumentTxnSetTableColumnWidth(t, id, column, width);
            if (result != XUI_OK) return result;
        }
    }
    *copied = id;
    return XUI_OK;
}

static xui_doc_position_t doc_fragment_edge(const xui_doc_position_t* template,
    const doc_node* node, int end)
{
    xui_doc_position_t edge = *template;
    edge.iNodeId = node->id;
    edge.iKind = doc_text_kind(node->kind) ?
        XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
    edge.iOffset = end ? (edge.iKind == XUI_DOC_POSITION_TEXT ?
        doc_seq_size(node->text) : doc_seq_size(node->children)) : 0;
    return edge;
}

static int doc_fragment_overlap(doc_state* source, const doc_node* node,
    const xui_doc_position_t* first, const xui_doc_position_t* last,
    int* overlap, int* whole)
{
    xui_doc_position_t start = doc_fragment_edge(first, node, 0);
    xui_doc_position_t end = doc_fragment_edge(first, node, 1);
    int a, b, result;
    *overlap = *whole = 0;
    result = doc_position_compare(source, &end, first, &a);
    if (result != XUI_OK) return result;
    result = doc_position_compare(source, &start, last, &b);
    if (result != XUI_OK) return result;
    if (a <= 0 || b >= 0) return XUI_OK;
    *overlap = 1;
    result = doc_position_compare(source, first, &start, &a);
    if (result != XUI_OK) return result;
    result = doc_position_compare(source, last, &end, &b);
    if (result == XUI_OK) *whole = a <= 0 && b >= 0;
    return result;
}

static int doc_copy_intersection_apply(xui_document_transaction t, doc_state* source,
    uint64_t source_id, const xui_doc_position_t* first,
    const xui_doc_position_t* last, uint64_t parent, uint64_t index,
    uint64_t* copied)
{
    doc_node* original = doc_index_get(source->index, source_id);
    uint64_t id = 0, i;
    int overlap, whole, result;
    *copied = 0;
    if (!original) return doc_txn_fail(t, XUI_ERROR_NOT_FOUND);
    result = doc_fragment_overlap(source, original, first, last,
        &overlap, &whole);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (!overlap) return XUI_OK;
    if (whole) return doc_copy_subtree_apply(t, source, source_id,
        parent, index, copied);
    if (doc_text_kind(original->kind)) {
        uint64_t start = first->iNodeId == original->id ? first->iOffset : 0;
        uint64_t end = last->iNodeId == original->id ? last->iOffset :
            doc_seq_size(original->text);
        if (original->kind != XUI_DOC_TEXT || start >= end)
            return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
        return doc_copy_node_apply(t, source, original, parent, index,
            start, end, copied);
    }
    if (original->kind == XUI_DOC_TABLE || original->kind == XUI_DOC_ROW ||
        original->kind == XUI_DOC_CELL)
        return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    result = doc_copy_node_apply(t, source, original, parent, index, 0, 0, &id);
    if (result != XUI_OK) return result;
    for (i = 0; i < doc_seq_size(original->children); i++) {
        uint64_t child = doc_seq_get_id(original->children, i), inserted;
        result = doc_copy_intersection_apply(t, source, child, first, last,
            id, DOC_NONE, &inserted);
        if (result != XUI_OK) return result;
    }
    *copied = id;
    return XUI_OK;
}

static uint64_t doc_fragment_common_ancestor(doc_state* source,
    uint64_t first, uint64_t last)
{
    doc_node* a = doc_index_get(source->index, first);
    for (; a; a = doc_index_get(source->index, a->parent)) {
        doc_node* b = doc_index_get(source->index, last);
        for (; b; b = doc_index_get(source->index, b->parent))
            if (a->id == b->id) return a->id;
    }
    return 0;
}

static int doc_copy_range_apply(xui_document_transaction t, doc_state* source,
    const xui_doc_position_t* first, const xui_doc_position_t* last,
    uint64_t parent, uint64_t index, uint64_t* copied_first,
    uint64_t* copied_count)
{
    doc_node* common;
    uint64_t ancestor = doc_fragment_common_ancestor(source,
        first->iNodeId, last->iNodeId);
    uint64_t i, count = 0, initial_index = index, initial_id = 0;
    int result = XUI_OK;
    if (!ancestor || !(common = doc_index_get(source->index, ancestor)))
        return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    if (doc_text_kind(common->kind)) {
        result = doc_copy_intersection_apply(t, source, ancestor, first, last,
            parent, index, &initial_id);
        if (result == XUI_OK && initial_id) count = 1;
    } else for (i = 0; i < doc_seq_size(common->children) && result == XUI_OK; i++) {
        uint64_t child = doc_seq_get_id(common->children, i), inserted = 0;
        uint64_t at = initial_index == DOC_NONE ? DOC_NONE : initial_index + count;
        if (initial_index != DOC_NONE && at < initial_index) {
            result = doc_txn_fail(t, XUI_DOC_ERROR_LIMIT); break;
        }
        result = doc_copy_intersection_apply(t, source, child, first, last,
            parent, at, &inserted);
        if (result == XUI_OK && inserted) {
            if (!count) initial_id = inserted;
            count++;
        }
    }
    if (result == XUI_OK && !count) result = doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    if (result == XUI_OK) {
        *copied_first = initial_id; *copied_count = count;
    }
    return result;
}

XUI_API int xuiDocumentTxnCopySubtree(xui_document_transaction t,
    xui_document_snapshot source, xui_doc_node_id source_node,
    xui_doc_node_id target_parent, uint64_t child_index,
    xui_doc_node_id* copied_root)
{
    struct xui_doc_transaction_t shadow;
    uint64_t copied = 0;
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (copied_root) *copied_root = 0;
    if (result != XUI_OK) return result;
    if (!source || !copied_root || source_node == DOC_ROOT)
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (!doc_index_get(source->state->index, source_node))
        return doc_txn_fail(t, XUI_ERROR_NOT_FOUND);
    if (t->draft->profile != XUI_DOCUMENT_MARKDOWN || t->parsing)
        result = doc_copy_subtree_apply(t, source->state, source_node,
            target_parent, child_index, &copied);
    else {
        result = doc_markdown_shadow_begin(t, &shadow);
        if (result != XUI_OK) return result;
        result = doc_copy_subtree_apply(&shadow, source->state, source_node,
            target_parent, child_index, &copied);
        result = doc_markdown_shadow_end(t, &shadow, result, NULL, NULL);
    }
    if (result == XUI_OK) *copied_root = copied;
    return result;
}

XUI_API int xuiDocumentTxnCopyRange(xui_document_transaction t,
    xui_document_snapshot source, const xui_doc_range_t* range,
    xui_doc_node_id target_parent, uint64_t child_index,
    xui_doc_node_id* copied_first, uint64_t* copied_count)
{
    struct xui_doc_transaction_t shadow;
    const xui_doc_position_t *first, *last;
    uint64_t inserted = 0, count = 0;
    int order, result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (copied_first) *copied_first = 0;
    if (copied_count) *copied_count = 0;
    if (result != XUI_OK) return result;
    if (!source || !range || !copied_first || !copied_count)
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = xuiDocumentSnapshotComparePositions(source,
        &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE ||
        range->tCaret.iKind == XUI_DOC_POSITION_SOURCE)
        return doc_txn_fail(t, XUI_DOC_ERROR_DOMAIN);
    if (!order) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    first = order < 0 ? &range->tAnchor : &range->tCaret;
    last = order < 0 ? &range->tCaret : &range->tAnchor;
    if (t->draft->profile != XUI_DOCUMENT_MARKDOWN || t->parsing)
        result = doc_copy_range_apply(t, source->state, first, last,
            target_parent, child_index, &inserted, &count);
    else {
        result = doc_markdown_shadow_begin(t, &shadow);
        if (result != XUI_OK) return result;
        result = doc_copy_range_apply(&shadow, source->state, first, last,
            target_parent, child_index, &inserted, &count);
        result = doc_markdown_shadow_end(t, &shadow, result, NULL, NULL);
    }
    if (result == XUI_OK) {
        *copied_first = inserted; *copied_count = count;
    }
    return result;
}

struct xui_doc_fragment_t {
    atomic_uint refs;
    xui_document_snapshot snapshot;
    unsigned depth; /* Root, one wrapper, or List -> ListItem. */
};

static doc_node* doc_fragment_container(xui_document_snapshot snapshot,
    unsigned depth)
{
    doc_node* node = doc_index_get(snapshot->state->index, DOC_ROOT);
    unsigned i;
    for (i = 0; node && i < depth; i++) {
        if (doc_seq_size(node->children) != 1 ||
            (depth == 2 && (i ? node->kind != XUI_DOC_LIST :
                node->kind != XUI_DOC_ROOT))) return NULL;
        node = doc_index_get(snapshot->state->index,
            doc_seq_get_id(node->children, 0));
    }
    if (depth == 2 && node && node->kind != XUI_DOC_LIST_ITEM) return NULL;
    return node && !doc_text_kind(node->kind) &&
        doc_seq_size(node->children) ? node : NULL;
}

const char* doc_fragment_single_image_resource(xui_document_fragment fragment)
{
    doc_node *container, *image;
    if (!fragment) return NULL;
    container = doc_fragment_container(fragment->snapshot, fragment->depth);
    if (!container || doc_seq_size(container->children) != 1) return NULL;
    image = doc_index_get(fragment->snapshot->state->index,
        doc_seq_get_id(container->children, 0));
    return image && image->kind == XUI_DOC_IMAGE ? doc_string(image->resource) : NULL;
}

static int doc_fragment_create(xui_document_snapshot snapshot,
    unsigned depth, xui_document_fragment* out)
{
    xui_document_fragment fragment;
    if (!doc_fragment_container(snapshot, depth)) return XUI_DOC_ERROR_FORMAT;
    fragment = malloc(sizeof(*fragment));
    if (!fragment) return XUI_ERROR_OUT_OF_MEMORY;
    atomic_init(&fragment->refs, 1);
    fragment->snapshot = snapshot; fragment->depth = depth;
    xuiDocumentSnapshotRetain(snapshot);
    *out = fragment;
    return XUI_OK;
}

XUI_API int xuiDocumentFragmentCreateRange(xui_document_snapshot source,
    const xui_doc_range_t* range, xui_document_fragment* out)
{
    xui_doc_desc_t desc = {0};
    xui_document temporary = NULL;
    xui_document_transaction t = NULL;
    xui_document_snapshot captured = NULL;
    const xui_doc_position_t *first, *last;
    doc_node *container, *list;
    uint64_t ancestor, parent = DOC_ROOT, copied, count;
    unsigned depth = 0;
    int order, result;
    if (out) *out = NULL;
    if (!source || !range || !out) return XUI_ERROR_INVALID_ARGUMENT;
    result = xuiDocumentSnapshotComparePositions(source,
        &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE ||
        range->tCaret.iKind == XUI_DOC_POSITION_SOURCE)
        return XUI_DOC_ERROR_DOMAIN;
    if (!order) return XUI_ERROR_UNSUPPORTED;
    first = order < 0 ? &range->tAnchor : &range->tCaret;
    last = order < 0 ? &range->tCaret : &range->tAnchor;
    ancestor = doc_fragment_common_ancestor(source->state,
        first->iNodeId, last->iNodeId);
    container = doc_index_get(source->state->index, ancestor);
    if (!container) return XUI_DOC_ERROR_SCHEMA;
    if (doc_text_kind(container->kind))
        container = doc_index_get(source->state->index, container->parent);
    if (!container) return XUI_DOC_ERROR_SCHEMA;
    if (container->kind != XUI_DOC_ROOT &&
        !doc_schema_child(XUI_DOC_ROOT, container->kind) &&
        container->kind != XUI_DOC_LIST_ITEM)
        return XUI_ERROR_UNSUPPORTED;
    if (source->state->allocator->max_nodes > UINT64_MAX - 3)
        return XUI_DOC_ERROR_LIMIT;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_RICH;
    desc.bDisableHistory = 1;
    desc.iMaxTextBytes = source->state->allocator->max_bytes;
    desc.iMaxNodes = source->state->allocator->max_nodes + 3;
    result = xuiDocumentCreate(&desc, &temporary);
    if (result != XUI_OK) return result;
    result = xuiDocumentBeginTransaction(temporary, NULL, &t);
    if (result == XUI_OK && container->kind == XUI_DOC_LIST_ITEM) {
        list = doc_index_get(source->state->index, container->parent);
        if (!list || list->kind != XUI_DOC_LIST)
            result = doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
        else {
            result = doc_copy_node_apply(t, source->state, list, DOC_ROOT, DOC_NONE,
                0, 0, &parent);
            if (result == XUI_OK) {
                uint64_t item;
                result = doc_copy_node_apply(t, source->state, container, parent,
                    DOC_NONE, 0, 0, &item);
                if (result == XUI_OK) parent = item;
            }
            depth = 2;
        }
    } else if (result == XUI_OK && container->kind != XUI_DOC_ROOT) {
        result = doc_copy_node_apply(t, source->state, container, DOC_ROOT,
            DOC_NONE, 0, 0, &parent);
        depth = 1;
    }
    if (result == XUI_OK) result = xuiDocumentTxnCopyRange(t, source,
        range, parent, DOC_NONE, &copied, &count);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
    xuiDocumentTxnRelease(t);
    if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(temporary, &captured);
    if (result == XUI_OK) result = doc_fragment_create(captured, depth, out);
    xuiDocumentSnapshotRelease(captured);
    xuiDocumentRelease(temporary);
    return result;
}

XUI_API void xuiDocumentFragmentRetain(xui_document_fragment fragment)
{
    if (fragment) atomic_fetch_add(&fragment->refs, 1);
}

XUI_API void xuiDocumentFragmentRelease(xui_document_fragment fragment)
{
    if (!fragment || atomic_fetch_sub(&fragment->refs, 1) != 1) return;
    xuiDocumentSnapshotRelease(fragment->snapshot);
    free(fragment);
}

XUI_API int xuiDocumentFragmentExportHtml(xui_document_fragment fragment,
    char** out, uint64_t* bytes)
{
    doc_node* container;
    if (out) *out = NULL;
    if (bytes) *bytes = 0;
    if (!fragment || !out || !bytes) return XUI_ERROR_INVALID_ARGUMENT;
    container = doc_fragment_container(fragment->snapshot, fragment->depth);
    if (!container) return XUI_DOC_ERROR_SCHEMA;
    return doc_html_export_children(fragment->snapshot, container->id,
        out, bytes);
}

XUI_API int xuiDocumentFragmentImportHtml(const char* html, uint64_t bytes,
    xui_document_fragment* out)
{
    xui_document document = NULL;
    xui_document_snapshot snapshot = NULL;
    unsigned depth = 0;
    int result;
    if (out) *out = NULL;
    if (!html || !out) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_html_import_document(html, bytes, &document, &depth);
    if (result != XUI_OK) return result;
    result = xuiDocumentAcquireSnapshot(document, &snapshot);
    if (result == XUI_OK) result = doc_fragment_create(snapshot, depth, out);
    xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    return result;
}

XUI_API int xuiDocumentFragmentSerialize(xui_document_fragment fragment,
    char** out, uint64_t* bytes)
{
    static const char magic[8] = {'X','U','I','F','R','A','G','1'};
    char *native = NULL, *wire;
    uint64_t length = 0;
    int result;
    if (out) *out = NULL;
    if (bytes) *bytes = 0;
    if (!fragment || !out || !bytes) return XUI_ERROR_INVALID_ARGUMENT;
    result = xuiDocumentSerialize(fragment->snapshot, &native, &length);
    if (result != XUI_OK) return result;
    if (length >= SIZE_MAX - 16) {
        xuiDocumentFreeBuffer(native); return XUI_DOC_ERROR_LIMIT;
    }
    wire = malloc((size_t)length + 17);
    if (!wire) {
        xuiDocumentFreeBuffer(native); return XUI_ERROR_OUT_OF_MEMORY;
    }
    memcpy(wire, magic, 8); memset(wire + 8, 0, 8);
    wire[8] = (char)fragment->depth;
    memcpy(wire + 16, native, (size_t)length);
    wire[16 + (size_t)length] = 0;
    xuiDocumentFreeBuffer(native);
    *out = wire; *bytes = length + 16;
    return XUI_OK;
}

XUI_API int xuiDocumentFragmentDeserialize(const char* data,
    uint64_t bytes, xui_document_fragment* out)
{
    static const char magic[8] = {'X','U','I','F','R','A','G','1'};
    xui_document document = NULL;
    xui_document_snapshot snapshot = NULL;
    unsigned depth;
    int result;
    if (out) *out = NULL;
    if (!data || !out || bytes <= 16 || bytes > SIZE_MAX ||
        memcmp(data, magic, 8) || (unsigned char)data[8] > 2 ||
        memcmp(data + 9, "\0\0\0\0\0\0\0", 7))
        return XUI_ERROR_INVALID_ARGUMENT;
    depth = (unsigned char)data[8];
    result = xuiDocumentDeserialize(NULL, data + 16, bytes - 16, &document);
    if (result != XUI_OK) return result;
    if (document->state->profile != XUI_DOCUMENT_RICH)
        result = XUI_DOC_ERROR_FORMAT;
    if (result == XUI_OK) result = xuiDocumentAcquireSnapshot(document, &snapshot);
    if (result == XUI_OK) result = doc_fragment_create(snapshot, depth, out);
    xuiDocumentSnapshotRelease(snapshot);
    xuiDocumentRelease(document);
    return result;
}

static int doc_fragment_insert_apply(xui_document_transaction t,
    xui_document_fragment fragment, uint64_t parent, uint64_t index,
    uint64_t* copied_first, uint64_t* copied_count)
{
    doc_node* container = doc_fragment_container(fragment->snapshot,
        fragment->depth);
    uint64_t i, count = 0, first = 0;
    int result = XUI_OK;
    if (!container) return doc_txn_fail(t, XUI_DOC_ERROR_FORMAT);
    for (i = 0; i < doc_seq_size(container->children) && result == XUI_OK; i++) {
        uint64_t child = doc_seq_get_id(container->children, i), inserted = 0;
        uint64_t at = index == DOC_NONE ? DOC_NONE : index + count;
        if (index != DOC_NONE && at < index)
            return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
        result = doc_copy_subtree_apply(t, fragment->snapshot->state,
            child, parent, at, &inserted);
        if (result == XUI_OK) {
            if (!count) first = inserted;
            count++;
        }
    }
    if (result == XUI_OK) {
        *copied_first = first; *copied_count = count;
    }
    return result;
}

XUI_API int xuiDocumentTxnInsertFragment(xui_document_transaction t,
    xui_document_fragment fragment, xui_doc_node_id target_parent,
    uint64_t child_index, xui_doc_node_id* copied_first,
    uint64_t* copied_count)
{
    struct xui_doc_transaction_t shadow;
    uint64_t first = 0, count = 0;
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (copied_first) *copied_first = 0;
    if (copied_count) *copied_count = 0;
    if (result != XUI_OK) return result;
    if (!fragment || !copied_first || !copied_count)
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile != XUI_DOCUMENT_MARKDOWN || t->parsing)
        result = doc_fragment_insert_apply(t, fragment, target_parent,
            child_index, &first, &count);
    else {
        result = doc_markdown_shadow_begin(t, &shadow);
        if (result != XUI_OK) return result;
        result = doc_fragment_insert_apply(&shadow, fragment, target_parent,
            child_index, &first, &count);
        result = doc_markdown_shadow_end(t, &shadow, result, NULL, NULL);
    }
    if (result == XUI_OK) {
        *copied_first = first; *copied_count = count;
    }
    return result;
}

static int doc_fragment_replace_apply(xui_document_transaction t,
    const xui_doc_range_t* range, xui_document_fragment fragment,
    xui_doc_position_t* caret)
{
    doc_node *container = doc_fragment_container(fragment->snapshot,
        fragment->depth), *node, *parent;
    xui_doc_position_t at, split;
    uint64_t parent_id = 0, index = 0, tail = 0, first = 0, count = 0;
    int inline_roots, result;
    if (!container) return doc_txn_fail(t, XUI_DOC_ERROR_FORMAT);
    inline_roots = container->kind == XUI_DOC_PARAGRAPH ||
        container->kind == XUI_DOC_HEADING;
    result = xuiDocumentTxnReplaceRange(t, range, "", 0, &at);
    if (result != XUI_OK) return result;
    node = doc_index_get(t->draft->index, at.iNodeId);
    if (!node) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    if (inline_roots && at.iKind == XUI_DOC_POSITION_TEXT &&
        node->kind == XUI_DOC_TEXT) {
        parent_id = node->parent;
        parent = doc_index_get(t->draft->index, parent_id);
        if (!parent || (parent->kind != XUI_DOC_PARAGRAPH &&
            parent->kind != XUI_DOC_HEADING))
            return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
        index = doc_child_index(parent, node->id);
        if (at.iOffset == doc_seq_size(node->text)) index++;
        else if (at.iOffset) {
            result = doc_split_text(t, node->id, at.iOffset, &tail);
            if (result != XUI_OK) return result;
            index++;
        }
    } else if (inline_roots && at.iKind == XUI_DOC_POSITION_GAP) {
        if (node->kind == XUI_DOC_PARAGRAPH || node->kind == XUI_DOC_HEADING) {
            parent_id = node->id; index = at.iOffset;
        } else if (doc_schema_child(node->kind, XUI_DOC_PARAGRAPH)) {
            xui_doc_node_desc_t desc = {0};
            desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_PARAGRAPH;
            result = doc_txn_insert(t, node->id, at.iOffset, &desc, &parent_id);
            if (result != XUI_OK) return result;
        } else return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    } else if (!inline_roots && at.iKind == XUI_DOC_POSITION_GAP &&
        doc_schema_child(node->kind, doc_index_get(fragment->snapshot->state->index,
            doc_seq_get_id(container->children, 0))->kind)) {
        parent_id = node->id; index = at.iOffset;
    } else if (!inline_roots && (at.iKind == XUI_DOC_POSITION_TEXT ||
        at.iKind == XUI_DOC_POSITION_GAP)) {
        result = xuiDocumentTxnSplitBlock(t, &at, &split);
        if (result != XUI_OK) return result;
        node = doc_index_get(t->draft->index, split.iNodeId);
        if (node && split.iKind == XUI_DOC_POSITION_TEXT)
            node = doc_index_get(t->draft->index, node->parent);
        if (!node || node->kind != XUI_DOC_PARAGRAPH)
            return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
        parent_id = node->parent;
        parent = doc_index_get(t->draft->index, parent_id);
        if (!parent) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
        index = doc_child_index(parent, node->id);
    } else return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    if (index == DOC_NONE) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    result = doc_fragment_insert_apply(t, fragment, parent_id, index,
        &first, &count);
    if (result != XUI_OK) return result;
    if (count > UINT64_MAX - index) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    *caret = at;
    caret->iKind = XUI_DOC_POSITION_GAP;
    caret->iNodeId = parent_id; caret->iOffset = index + count;
    caret->iAffinity = XUI_DOC_AFTER;
    return XUI_OK;
}

XUI_API int xuiDocumentTxnReplaceRangeWithFragment(xui_document_transaction t,
    const xui_doc_range_t* range, xui_document_fragment fragment,
    xui_doc_position_t* caret)
{
    struct xui_doc_transaction_t shadow;
    xui_doc_position_t target = {0};
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (caret) memset(caret, 0, sizeof(*caret));
    if (result != XUI_OK) return result;
    if (!range || !fragment || !caret)
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile != XUI_DOCUMENT_MARKDOWN || t->parsing) {
        result = doc_fragment_replace_apply(t, range, fragment, &target);
        if (result == XUI_OK) *caret = target;
        return result;
    }
    result = doc_markdown_shadow_begin(t, &shadow);
    if (result != XUI_OK) return result;
    result = doc_fragment_replace_apply(&shadow, range, fragment, &target);
    return doc_markdown_shadow_end(t, &shadow, result,
        result == XUI_OK ? &target : NULL, caret);
}

#endif
