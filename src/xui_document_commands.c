#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"
#include "xui_document_md4c.h"
#include <math.h>
#include <stdio.h>

static int doc_command_position(xui_document_transaction t, const xui_doc_position_t* p)
{
    if (!p || p->iDocumentId != t->document->identity || p->iRevision != t->base_revision) return XUI_DOC_ERROR_STALE;
    return doc_position_valid(t->draft, p) ? XUI_OK : XUI_ERROR_INVALID_ARGUMENT;
}
static xui_doc_position_t doc_command_caret(xui_document_transaction t, uint64_t id, uint64_t offset, unsigned kind)
{
    xui_doc_position_t p = {0}; p.iSize = sizeof(p); p.iDocumentId = t->document->identity; p.iRevision = t->base_revision;
    p.iNodeId = id; p.iOffset = offset; p.iKind = kind; p.iAffinity = XUI_DOC_AFTER; return p;
}
static int doc_paragraph(doc_node* n) { return n && (n->kind == XUI_DOC_PARAGRAPH || n->kind == XUI_DOC_HEADING); }
static void doc_command_relocations(xui_document_transaction t, uint64_t start)
{
    for (; start < t->count; start++) if (t->ops[start].iKind == XUI_DOC_OP_MOVE) t->ops[start].iMappingFlags |= XUI_DOC_OP_ANCESTRY_ONLY;
}
static int doc_insert_empty(xui_document_transaction t, uint64_t parent, uint64_t index, unsigned kind, uint64_t* id)
{
    xui_doc_node_desc_t desc = {0}; desc.iSize = sizeof(desc); desc.iKind = kind;
    return doc_txn_insert(t, parent, index, &desc, id);
}
/* Convert a structural insertion gap to a text caret without flattening styles. */
static int doc_text_caret(xui_document_transaction t, xui_doc_position_t* p)
{
    doc_node* n = doc_index_get(t->draft->index, p->iNodeId); uint64_t id; int result;
    if (p->iKind == XUI_DOC_POSITION_TEXT) return XUI_OK;
    if (p->iKind != XUI_DOC_POSITION_GAP) return XUI_DOC_ERROR_DOMAIN;
    if (!doc_paragraph(n)) {
        result = doc_insert_empty(t, n->id, p->iOffset, XUI_DOC_PARAGRAPH, &id);
        if (result != XUI_OK) return result;
        *p = doc_command_caret(t, id, 0, XUI_DOC_POSITION_GAP); n = doc_index_get(t->draft->index, id);
    }
    if (p->iOffset) {
        doc_node* previous = doc_index_get(t->draft->index, doc_seq_get_id(n->children, p->iOffset - 1));
        if (previous->kind == XUI_DOC_TEXT) { *p = doc_command_caret(t, previous->id, doc_seq_size(previous->text), XUI_DOC_POSITION_TEXT); return XUI_OK; }
    }
    result = doc_insert_empty(t, n->id, p->iOffset, XUI_DOC_TEXT, &id);
    if (result == XUI_OK) *p = doc_command_caret(t, id, 0, XUI_DOC_POSITION_TEXT);
    return result;
}
static int doc_image_desc_valid(const xui_doc_image_desc_t* image)
{
    const char *title, *link_title;
    if (!image || image->iSize != sizeof(*image) || !image->sResource || !*image->sResource ||
        (!image->sAlt && image->iAltBytes) || image->iAltBytes >= SIZE_MAX ||
        !isfinite(image->fWidth) || !isfinite(image->fHeight) ||
        image->fWidth < 0 || image->fWidth > DOC_MAX_LAYOUT_VALUE ||
        image->fHeight < 0 || image->fHeight > DOC_MAX_LAYOUT_VALUE)
        return 0;
    title = image->sTitle ? image->sTitle : "";
    link_title = image->sLinkTitle ? image->sLinkTitle : "";
    return doc_utf8(image->sResource, strlen(image->sResource)) &&
        doc_utf8(image->sAlt, image->iAltBytes) && doc_utf8(title, strlen(title)) &&
        (!image->sLinkTitle || image->sLinkTarget) &&
        (!image->sLinkTarget || doc_utf8(image->sLinkTarget, strlen(image->sLinkTarget))) &&
        doc_utf8(link_title, strlen(link_title)) &&
        !memchr(image->sResource, '\n', strlen(image->sResource)) &&
        !memchr(image->sResource, '\r', strlen(image->sResource)) &&
        !memchr(title, '\n', strlen(title)) && !memchr(title, '\r', strlen(title)) &&
        (!image->sLinkTarget || (!strchr(image->sLinkTarget, '\n') && !strchr(image->sLinkTarget, '\r'))) &&
        !strchr(link_title, '\n') && !strchr(link_title, '\r') &&
        (!image->iAltBytes || (!memchr(image->sAlt, 0, (size_t)image->iAltBytes) &&
            !memchr(image->sAlt, '\n', (size_t)image->iAltBytes) &&
            !memchr(image->sAlt, '\r', (size_t)image->iAltBytes)));
}
static int doc_image_link_set(xui_document_transaction t, xui_doc_node_id image_id,
    const xui_doc_image_desc_t* image)
{
    doc_node *node = doc_index_get(t->draft->index, image_id), *copy;
    doc_blob *target = NULL, *title = NULL;
    xui_doc_operation_t op = {0};
    const char *next_target = image->sLinkTarget ? image->sLinkTarget : "",
        *next_title = image->sLinkTitle ? image->sLinkTitle : "";
    int result;
    if (!node || node->kind != XUI_DOC_IMAGE) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    if (!strcmp(doc_string(node->link_target), next_target) &&
        !strcmp(doc_string(node->link_title), next_title)) return XUI_OK;
    if (*next_target && !(target = doc_blob_new(t->draft->allocator, next_target, strlen(next_target))))
        goto failed;
    if (*next_title && !(title = doc_blob_new(t->draft->allocator, next_title, strlen(next_title))))
        goto failed;
    copy = doc_node_clone(t->draft->allocator, node);
    if (!copy) goto failed;
    doc_blob_release(copy->link_target); copy->link_target = target;
    doc_blob_release(copy->link_title); copy->link_title = title;
    result = doc_state_set(t->draft, copy); doc_node_release(copy);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    op.iKind = XUI_DOC_OP_ATTRIBUTES; op.iFlags = XUI_DOC_CHANGE_RESOURCE;
    op.iNodeId = image_id;
    return doc_txn_op(t, &op);
failed:
    doc_blob_release(target); doc_blob_release(title);
    return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
}
static int doc_image_copy(xui_document_transaction t, const xui_doc_image_desc_t* image,
    doc_blob** resource, doc_blob** alt, doc_blob** title)
{
    *resource = doc_blob_new(t->draft->allocator, image->sResource, strlen(image->sResource));
    if (*resource && image->iAltBytes) *alt = doc_blob_new(t->draft->allocator, image->sAlt, image->iAltBytes);
    if (*resource && (!image->iAltBytes || *alt) && image->sTitle && *image->sTitle)
        *title = doc_blob_new(t->draft->allocator, image->sTitle, strlen(image->sTitle));
    if (!*resource || (image->iAltBytes && !*alt) || (image->sTitle && *image->sTitle && !*title))
        return XUI_ERROR_OUT_OF_MEMORY;
    return XUI_OK;
}
XUI_API int xuiDocumentTxnInsertImage(xui_document_transaction t, const xui_doc_range_t* range,
    const xui_doc_image_desc_t* image, xui_doc_node_id* image_id, xui_doc_position_t* caret)
{
    doc_blob *resource = NULL, *alt = NULL, *title = NULL;
    xui_doc_node_desc_t node_desc = {0}; xui_doc_position_t at;
    doc_node *node, *parent; uint64_t parent_id, index, tail, id = 0; int result;
    if (image_id) *image_id = 0;
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!range || !image_id || !caret || !doc_image_desc_valid(image))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE || range->tCaret.iKind == XUI_DOC_POSITION_SOURCE)
        return doc_txn_fail(t, XUI_DOC_ERROR_DOMAIN);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && (image->fWidth > 0 || image->fHeight > 0))
        return doc_txn_fail(t, XUI_DOC_ERROR_UNREPRESENTABLE);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_position_t target;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnInsertImage(&shadow, range, image, &id, &target);
        result = doc_markdown_shadow_end_insert_image(t, &shadow, result, range, id, &target, caret);
        if (result == XUI_OK) *image_id = id;
        return result;
    }
    result = doc_image_copy(t, image, &resource, &alt, &title);
    if (result != XUI_OK) goto done;
    result = xuiDocumentTxnReplaceRange(t, range, "", 0, &at);
    if (result != XUI_OK) goto done;
    node = doc_index_get(t->draft->index, at.iNodeId);
    if (at.iKind == XUI_DOC_POSITION_TEXT && node && node->kind == XUI_DOC_TEXT) {
        parent_id = node->parent;
        parent = doc_index_get(t->draft->index, parent_id);
        if (!doc_paragraph(parent)) { result = XUI_ERROR_UNSUPPORTED; goto done; }
        index = doc_child_index(parent, node->id);
        if (at.iOffset == doc_seq_size(node->text)) index++;
        else if (at.iOffset) {
            result = doc_split_text(t, node->id, at.iOffset, &tail);
            if (result != XUI_OK) goto done;
            index++;
        }
    } else if (at.iKind == XUI_DOC_POSITION_GAP && node) {
        if (doc_paragraph(node)) { parent_id = node->id; index = at.iOffset; }
        else if (doc_schema_child(node->kind, XUI_DOC_PARAGRAPH)) {
            result = doc_insert_empty(t, node->id, at.iOffset, XUI_DOC_PARAGRAPH, &parent_id);
            if (result != XUI_OK) goto done;
            index = 0;
        } else { result = XUI_ERROR_UNSUPPORTED; goto done; }
    } else { result = XUI_ERROR_UNSUPPORTED; goto done; }
    node_desc.iSize = sizeof(node_desc); node_desc.iKind = XUI_DOC_IMAGE;
    node_desc.sText = doc_string(alt); node_desc.iTextBytes = image->iAltBytes;
    node_desc.sResource = doc_string(resource); node_desc.sTitle = doc_string(title);
    node_desc.sLinkTarget = image->sLinkTarget; node_desc.sLinkTitle = image->sLinkTitle;
    if (image->sLinkTarget) node_desc.tAttributes.iMarks |= XUI_DOC_LINK;
    node_desc.tAttributes.fWidth = image->fWidth; node_desc.tAttributes.fHeight = image->fHeight;
    result = doc_txn_insert(t, parent_id, index, &node_desc, &id);
    if (result == XUI_OK) {
        *image_id = id;
        *caret = doc_command_caret(t, parent_id, index + 1, XUI_DOC_POSITION_GAP);
    }
done:
    doc_blob_release(resource); doc_blob_release(alt); doc_blob_release(title);
    return result == XUI_OK ? XUI_OK : doc_txn_fail(t, result);
}
static int doc_command_block_object_gap(xui_document_transaction t,
    const xui_doc_position_t* at, uint32_t kind, uint64_t* parent_id, uint64_t* index)
{
    doc_node *node = doc_index_get(t->draft->index, at->iNodeId), *block, *parent;
    xui_doc_position_t split; uint64_t child_index, children; int result;
    if (at->iKind == XUI_DOC_POSITION_GAP && node &&
        node->kind != XUI_DOC_PARAGRAPH && node->kind != XUI_DOC_HEADING &&
        doc_schema_child(node->kind, kind)) {
        *parent_id = node->id; *index = at->iOffset; return XUI_OK;
    }
    if (at->iKind == XUI_DOC_POSITION_TEXT && node && node->kind == XUI_DOC_TEXT)
        block = doc_index_get(t->draft->index, node->parent);
    else if (at->iKind == XUI_DOC_POSITION_GAP) block = node;
    else return XUI_ERROR_UNSUPPORTED;
    if (!doc_paragraph(block)) return XUI_ERROR_UNSUPPORTED;
    parent = doc_index_get(t->draft->index, block->parent);
    if (!parent || !doc_schema_child(parent->kind, kind)) return XUI_ERROR_UNSUPPORTED;
    *parent_id = parent->id; *index = doc_child_index(parent, block->id);
    if (*index == DOC_NONE) return XUI_DOC_ERROR_SCHEMA;
    children = doc_seq_size(block->children);
    if (at->iKind == XUI_DOC_POSITION_GAP) {
        if (!at->iOffset) return XUI_OK;
        if (at->iOffset == children) { (*index)++; return XUI_OK; }
    } else {
        child_index = doc_child_index(block, node->id);
        if (child_index == DOC_NONE) return XUI_DOC_ERROR_SCHEMA;
        if (!child_index && !at->iOffset) return XUI_OK;
        if (child_index + 1 == children && at->iOffset == doc_seq_size(node->text)) {
            (*index)++; return XUI_OK;
        }
    }
    result = xuiDocumentTxnSplitBlock(t, at, &split);
    if (result != XUI_OK) return result;
    node = doc_index_get(t->draft->index, split.iNodeId);
    block = node && node->kind == XUI_DOC_TEXT ? doc_index_get(t->draft->index, node->parent) : node;
    parent = block ? doc_index_get(t->draft->index, block->parent) : NULL;
    if (!doc_paragraph(block) || !parent || !doc_schema_child(parent->kind, kind))
        return XUI_DOC_ERROR_SCHEMA;
    *parent_id = parent->id; *index = doc_child_index(parent, block->id);
    return *index == DOC_NONE ? XUI_DOC_ERROR_SCHEMA : XUI_OK;
}
XUI_API int xuiDocumentTxnInsertObject(xui_document_transaction t,
    const xui_doc_range_t* range, uint32_t kind, uint32_t flags,
    const char* utf8, uint64_t bytes, xui_doc_node_id* object_id,
    xui_doc_position_t* caret)
{
    xui_doc_node_desc_t desc = {0}; xui_doc_position_t at;
    doc_node *node, *parent; uint64_t parent_id = 0, index = 0, tail, id = 0;
    char* normalized = NULL;
    int block, order, result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (object_id) *object_id = 0;
    if (result != XUI_OK) return result;
    if (!range || !object_id || !caret ||
        (kind != XUI_DOC_MATH && kind != XUI_DOC_DIAGRAM && kind != XUI_DOC_HTML) ||
        (flags & ~XUI_DOC_BLOCK) || (kind == XUI_DOC_DIAGRAM && flags) ||
        (!utf8 && bytes) || bytes >= SIZE_MAX || !doc_utf8(utf8, bytes) ||
        (bytes && memchr(utf8, 0, (size_t)bytes)))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result == XUI_OK) result = doc_position_compare(t->draft,
        &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE ||
        range->tCaret.iKind == XUI_DOC_POSITION_SOURCE)
        return doc_txn_fail(t, XUI_DOC_ERROR_DOMAIN);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN &&
        t->draft->dialect != XUI_MD_EXTENDED &&
        (kind == XUI_DOC_MATH || kind == XUI_DOC_DIAGRAM))
        return doc_txn_fail(t, XUI_DOC_ERROR_UNREPRESENTABLE);
    block = kind == XUI_DOC_DIAGRAM || (kind == XUI_DOC_HTML && (flags & XUI_DOC_BLOCK));
    if (block && order) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_position_t target;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnInsertObject(&shadow, range, kind, flags,
            utf8, bytes, &id, &target);
        result = doc_markdown_shadow_end(t, &shadow, result, &target, caret);
        if (result == XUI_OK) *object_id = id;
        return result;
    }
    desc.iSize = sizeof(desc); desc.iKind = kind; desc.tAttributes.iFlags = flags;
    desc.sText = utf8 ? utf8 : ""; desc.iTextBytes = bytes;
    if (kind == XUI_DOC_DIAGRAM) desc.sInfo = "mermaid";
    if (block) {
        result = doc_command_block_object_gap(t, &range->tCaret, kind, &parent_id, &index);
        if (result != XUI_OK) return doc_txn_fail(t, result);
    } else {
        result = xuiDocumentTxnReplaceRange(t, range, "", 0, &at);
        if (result != XUI_OK) return result;
        node = doc_index_get(t->draft->index, at.iNodeId);
        if (at.iKind == XUI_DOC_POSITION_TEXT && node && node->kind == XUI_DOC_TEXT) {
            parent_id = node->parent; parent = doc_index_get(t->draft->index, parent_id);
            if (!doc_paragraph(parent)) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
            index = doc_child_index(parent, node->id);
            if (at.iOffset == doc_seq_size(node->text)) index++;
            else if (at.iOffset) {
                result = doc_split_text(t, node->id, at.iOffset, &tail);
                if (result != XUI_OK) return result;
                index++;
            }
        } else if (at.iKind == XUI_DOC_POSITION_GAP && node) {
            if (doc_paragraph(node)) { parent_id = node->id; index = at.iOffset; }
            else if (doc_schema_child(node->kind, XUI_DOC_PARAGRAPH)) {
                result = doc_insert_empty(t, node->id, at.iOffset, XUI_DOC_PARAGRAPH, &parent_id);
                if (result != XUI_OK) return result;
                index = 0;
            } else return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
        } else return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    }
    if (block && t->draft->profile == XUI_DOCUMENT_MARKDOWN &&
        (kind == XUI_DOC_DIAGRAM || kind == XUI_DOC_HTML) &&
        (!bytes || utf8[bytes - 1] != '\n')) {
        if (bytes > SIZE_MAX - 2) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
        normalized = doc_alloc(t->draft->allocator, (size_t)bytes + 1);
        if (!normalized) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
        if (bytes) memcpy(normalized, utf8, (size_t)bytes);
        normalized[bytes] = '\n';
        desc.sText = normalized; desc.iTextBytes = bytes + 1;
    }
    result = doc_txn_insert(t, parent_id, index, &desc, &id);
    doc_free(normalized);
    if (result != XUI_OK) return result;
    *object_id = id;
    *caret = doc_command_caret(t, parent_id, index + 1, XUI_DOC_POSITION_GAP);
    return XUI_OK;
}
static int doc_command_code_language_valid(const char* language)
{
    const unsigned char* p = (const unsigned char*)(language ? language : "");
    if (!doc_utf8((const char*)p, strlen((const char*)p))) return 0;
    for (; *p; p++) if (*p <= ' ' || *p == '~' || *p == '`') return 0;
    return 1;
}
static int doc_command_insert_simple_block(xui_document_transaction t,
    const xui_doc_range_t* range, uint32_t kind, const char* language,
    const char* utf8, uint64_t bytes, xui_doc_node_id* block_id,
    xui_doc_position_t* caret)
{
    xui_doc_node_desc_t desc = {0}; uint64_t parent_id, index, id = 0;
    char* normalized = NULL; int order, result;
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result == XUI_OK) result = doc_position_compare(t->draft,
        &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE ||
        range->tCaret.iKind == XUI_DOC_POSITION_SOURCE)
        return doc_txn_fail(t, XUI_DOC_ERROR_DOMAIN);
    if (order) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_position_t target;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = doc_command_insert_simple_block(&shadow, range, kind, language,
            utf8, bytes, &id, &target);
        result = doc_markdown_shadow_end(t, &shadow, result, &target, caret);
        if (result == XUI_OK) *block_id = id;
        return result;
    }
    result = doc_command_block_object_gap(t, &range->tCaret, kind, &parent_id, &index);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    desc.iSize = sizeof(desc); desc.iKind = kind;
    if (kind == XUI_DOC_CODE_BLOCK) {
        desc.sInfo = language;
        desc.sText = utf8 ? utf8 : ""; desc.iTextBytes = bytes;
        if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && bytes && utf8[bytes - 1] != '\n') {
            if (bytes > SIZE_MAX - 2) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
            normalized = doc_alloc(t->draft->allocator, (size_t)bytes + 1);
            if (!normalized) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
            memcpy(normalized, utf8, (size_t)bytes); normalized[bytes] = '\n';
            desc.sText = normalized; desc.iTextBytes = bytes + 1;
        }
    }
    result = doc_txn_insert(t, parent_id, index, &desc, &id);
    doc_free(normalized);
    if (result != XUI_OK) return result;
    *block_id = id;
    *caret = kind == XUI_DOC_CODE_BLOCK ?
        doc_command_caret(t, id, desc.iTextBytes, XUI_DOC_POSITION_TEXT) :
        doc_command_caret(t, parent_id, index + 1, XUI_DOC_POSITION_GAP);
    return XUI_OK;
}
XUI_API int xuiDocumentTxnInsertCodeBlock(xui_document_transaction t,
    const xui_doc_range_t* range, const char* language, const char* utf8,
    uint64_t bytes, xui_doc_node_id* code_id, xui_doc_position_t* caret)
{
    int result;
    if (code_id) *code_id = 0;
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!range || !code_id || !caret || (!utf8 && bytes) || bytes >= SIZE_MAX ||
        !doc_utf8(utf8, bytes) || (bytes && memchr(utf8, 0, (size_t)bytes)) ||
        !doc_command_code_language_valid(language))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    return doc_command_insert_simple_block(t, range, XUI_DOC_CODE_BLOCK,
        language, utf8, bytes, code_id, caret);
}
XUI_API int xuiDocumentTxnSetCodeBlockLanguage(xui_document_transaction t,
    xui_doc_node_id code_id, const char* language)
{
    doc_node* code; int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (!code_id || !doc_command_code_language_valid(language))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    code = doc_index_get(t->draft->index, code_id);
    if (!code || code->kind != XUI_DOC_CODE_BLOCK)
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    return xuiDocumentTxnSetResource(t, code_id,
        doc_string(code->resource), language, doc_string(code->title));
}
XUI_API int xuiDocumentTxnInsertRule(xui_document_transaction t,
    const xui_doc_range_t* range, xui_doc_node_id* rule_id,
    xui_doc_position_t* caret)
{
    int result;
    if (rule_id) *rule_id = 0;
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!range || !rule_id || !caret) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    return doc_command_insert_simple_block(t, range, XUI_DOC_RULE,
        NULL, NULL, 0, rule_id, caret);
}
static int doc_command_footnote_label_valid(const char* label)
{
    size_t i, length = strlen(label);
    if (!doc_md4c_footnote_label_valid(label) || !doc_utf8(label, length)) return 0;
    for (i = 0; i < length; i++) {
        unsigned char c = (unsigned char)label[i];
        if (c < 32 || c == 127) return 0;
    }
    return 1;
}
static int doc_command_footnote_label_equal(const char* a, const char* b)
{
    return doc_md4c_label_equal(a, b);
}
static int doc_command_footnote_label_in_source(doc_state* s, const char* label)
{
    uint64_t i;
    char candidate[1025];
    /* An unused Markdown definition has no semantic footnote node. It still
     * reserves its label and would take precedence if a new reference used it. */
    for (i = 0; i < doc_seq_size(s->references); i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t reference; uint64_t label_bytes;
        if (doc_seq_read(s->references, i, &reference, sizeof(reference)) != XUI_OK) return 1;
        if (reference.iKind != XUI_DOC_REFERENCE_FOOTNOTE ||
            reference.iLabelStart > reference.iLabelEnd ||
            reference.iLabelEnd > doc_seq_size(s->source)) continue;
        label_bytes = reference.iLabelEnd - reference.iLabelStart;
        if (label_bytes >= sizeof(candidate)) continue;
        if (doc_seq_read(s->source, reference.iLabelStart, candidate, label_bytes) != XUI_OK) return 1;
        candidate[label_bytes] = 0;
        if (doc_command_footnote_label_equal(candidate, label)) return 1;
    }
    return 0;
}
static int doc_command_footnote_label_used(doc_state* s, uint64_t id, const char* label)
{
    doc_node* node = doc_index_get(s->index, id); uint64_t i;
    if (!node) return 0;
    if (id == DOC_ROOT && s->profile == XUI_DOCUMENT_MARKDOWN &&
        doc_command_footnote_label_in_source(s, label)) return 1;
    if (node->kind == XUI_DOC_FOOTNOTE &&
        doc_command_footnote_label_equal(doc_string(node->info), label)) return 1;
    for (i = 0; i < doc_seq_size(node->children); i++)
        if (doc_command_footnote_label_used(s, doc_seq_get_id(node->children, i), label)) return 1;
    return 0;
}
typedef struct doc_command_footnote_order_t {
    doc_state* state;
    doc_node* root;
    unsigned char* seen;
    uint64_t reference_id;
    uint64_t insert_index;
    int after_new;
} doc_command_footnote_order_t;
static void doc_command_footnote_order_visit(doc_command_footnote_order_t* order, uint64_t id)
{
    doc_node* node = doc_index_get(order->state->index, id); uint64_t i;
    if (!node || order->insert_index != DOC_NONE || node->kind == XUI_DOC_FOOTNOTE) return;
    if (node->kind == XUI_DOC_FOOTNOTE_REF) {
        if (id == order->reference_id) { order->after_new = 1; return; }
        for (i = 0; i < doc_seq_size(order->root->children); i++) {
            doc_node* note = doc_index_get(order->state->index,
                doc_seq_get_id(order->root->children, i));
            if (note && note->kind == XUI_DOC_FOOTNOTE &&
                doc_command_footnote_label_equal(doc_string(node->info),
                    doc_string(note->info))) {
                if (!order->seen[i]) {
                    order->seen[i] = 1;
                    if (order->after_new) order->insert_index = i;
                }
                return;
            }
        }
    }
    for (i = 0; i < doc_seq_size(node->children) && order->insert_index == DOC_NONE; i++)
        doc_command_footnote_order_visit(order, doc_seq_get_id(node->children, i));
}
static int doc_command_footnote_insert_index(xui_document_transaction t,
    uint64_t reference_id, uint64_t* index)
{
    doc_command_footnote_order_t order = {0}; uint64_t i, count;
    order.state = t->draft; order.root = doc_index_get(t->draft->index, DOC_ROOT);
    if (!order.root) return XUI_DOC_ERROR_SCHEMA;
    count = doc_seq_size(order.root->children);
    order.reference_id = reference_id; order.insert_index = DOC_NONE;
    if (count) {
        if (count > SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
        order.seen = doc_alloc(t->draft->allocator, (size_t)count);
        if (!order.seen) return XUI_ERROR_OUT_OF_MEMORY;
        memset(order.seen, 0, (size_t)count);
    }
    for (i = 0; i < count && order.insert_index == DOC_NONE; i++)
        doc_command_footnote_order_visit(&order, doc_seq_get_id(order.root->children, i));
    doc_free(order.seen);
    *index = order.insert_index;
    return XUI_OK;
}
XUI_API int xuiDocumentTxnInsertFootnote(xui_document_transaction t,
    const xui_doc_range_t* range, const char* label,
    const char* initial_utf8, uint64_t initial_bytes,
    xui_doc_node_id* reference_id, xui_doc_node_id* footnote_id,
    xui_doc_position_t* reference_caret, xui_doc_position_t* body_caret)
{
    xui_doc_node_desc_t desc = {0}; xui_doc_position_t at;
    doc_node *node, *parent; uint64_t parent_id, index, note_index, tail, ref = 0, note = 0;
    char generated[32]; uint32_t serial; size_t label_bytes; int result;
    if (reference_id) *reference_id = 0;
    if (footnote_id) *footnote_id = 0;
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!range || !reference_id || !footnote_id || !reference_caret || !body_caret ||
        (!initial_utf8 && initial_bytes) || initial_bytes >= SIZE_MAX ||
        !doc_utf8(initial_utf8, initial_bytes) ||
        (initial_bytes && memchr(initial_utf8, 0, (size_t)initial_bytes)))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE ||
        range->tCaret.iKind == XUI_DOC_POSITION_SOURCE)
        return doc_txn_fail(t, XUI_DOC_ERROR_DOMAIN);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && t->draft->dialect != XUI_MD_EXTENDED)
        return doc_txn_fail(t, XUI_DOC_ERROR_UNREPRESENTABLE);
    if (!label || !*label) {
        for (serial = 1; serial; serial++) {
            snprintf(generated, sizeof(generated), "fn%u", serial);
            if (!doc_command_footnote_label_used(t->draft, DOC_ROOT, generated)) break;
        }
        if (!serial) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
        label = generated;
    }
    if (!doc_command_footnote_label_valid(label) ||
        doc_command_footnote_label_used(t->draft, DOC_ROOT, label))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    label_bytes = strlen(label);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_range_t target, after;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnInsertFootnote(&shadow, range, label,
            initial_utf8, initial_bytes, &ref, &note,
            &target.tAnchor, &target.tCaret);
        result = doc_markdown_shadow_end_range(t, &shadow, result, &target, &after);
        if (result == XUI_OK) {
            *reference_id = ref; *footnote_id = note;
            *reference_caret = after.tAnchor; *body_caret = after.tCaret;
        }
        return result;
    }
    result = xuiDocumentTxnReplaceRange(t, range, "", 0, &at);
    if (result != XUI_OK) return result;
    node = doc_index_get(t->draft->index, at.iNodeId);
    if (at.iKind == XUI_DOC_POSITION_TEXT && node && node->kind == XUI_DOC_TEXT) {
        parent_id = node->parent;
        parent = doc_index_get(t->draft->index, parent_id);
        if (!doc_paragraph(parent)) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
        index = doc_child_index(parent, node->id);
        if (at.iOffset == doc_seq_size(node->text)) index++;
        else if (at.iOffset) {
            result = doc_split_text(t, node->id, at.iOffset, &tail);
            if (result != XUI_OK) return result;
            index++;
        }
    } else if (at.iKind == XUI_DOC_POSITION_GAP && node) {
        if (doc_paragraph(node)) { parent_id = node->id; index = at.iOffset; }
        else if (doc_schema_child(node->kind, XUI_DOC_PARAGRAPH)) {
            result = doc_insert_empty(t, node->id, at.iOffset, XUI_DOC_PARAGRAPH, &parent_id);
            if (result != XUI_OK) return result;
            index = 0;
        } else return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    } else return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_FOOTNOTE_REF;
    desc.sText = label; desc.iTextBytes = label_bytes; desc.sInfo = label;
    result = doc_txn_insert(t, parent_id, index, &desc, &ref);
    if (result != XUI_OK) return result;
    *reference_caret = doc_command_caret(t, parent_id, index + 1, XUI_DOC_POSITION_GAP);
    memset(&desc, 0, sizeof(desc));
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_FOOTNOTE; desc.sInfo = label;
    note_index = DOC_NONE;
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN) {
        result = doc_command_footnote_insert_index(t, ref, &note_index);
        if (result != XUI_OK) return doc_txn_fail(t, result);
    }
    result = doc_txn_insert(t, DOC_ROOT, note_index, &desc, &note);
    if (result != XUI_OK) return result;
    *body_caret = doc_command_caret(t, note, 0, XUI_DOC_POSITION_GAP);
    if (initial_bytes) {
        xui_doc_range_t body = {*body_caret, *body_caret};
        result = xuiDocumentTxnReplaceRange(t, &body, initial_utf8, initial_bytes, body_caret);
        if (result != XUI_OK) return result;
    }
    *reference_id = ref; *footnote_id = note;
    return XUI_OK;
}
typedef struct doc_command_footnote_reach_t {
    uint64_t id;
    const char* label;
    unsigned char before, after;
} doc_command_footnote_reach_t;
static void doc_command_mark_footnote_reach(doc_state* s, uint64_t id,
    uint64_t excluded, doc_command_footnote_reach_t* notes, uint64_t count,
    int before)
{
    doc_node* node = doc_index_get(s->index, id); uint64_t i;
    if (!node || id == excluded) return;
    if (node->kind == XUI_DOC_FOOTNOTE_REF) {
        for (i = 0; i < count; i++) {
            unsigned char* marked = before ? &notes[i].before : &notes[i].after;
            if (doc_command_footnote_label_equal(doc_string(node->info), notes[i].label)) {
                if (!*marked) *marked = 1;
                break;
            }
        }
    }
    for (i = 0; i < doc_seq_size(node->children); i++)
        doc_command_mark_footnote_reach(s, doc_seq_get_id(node->children, i),
            excluded, notes, count, before);
}
static void doc_command_calculate_footnote_reach(doc_state* s, doc_node* root,
    uint64_t excluded, doc_command_footnote_reach_t* notes, uint64_t count,
    int before)
{
    uint64_t i;
    for (i = 0; i < doc_seq_size(root->children); i++) {
        doc_node* block = doc_index_get(s->index, doc_seq_get_id(root->children, i));
        if (block && block->kind != XUI_DOC_FOOTNOTE)
            doc_command_mark_footnote_reach(s, block->id, excluded, notes, count, before);
    }
    /* Expand graph edges iteratively. Tree recursion is bounded by the schema;
     * a chain of footnotes is not, and must not consume one C frame per note. */
    for (;;) {
        unsigned char* marked = NULL;
        for (i = 0; i < count; i++) {
            marked = before ? &notes[i].before : &notes[i].after;
            if (*marked == 1) break;
        }
        if (i == count) break;
        *marked = 2;
        doc_command_mark_footnote_reach(s, notes[i].id, excluded, notes, count, before);
    }
}
XUI_API int xuiDocumentTxnRemoveFootnoteReference(xui_document_transaction t,
    xui_doc_node_id reference_id, xui_doc_position_t* caret)
{
    doc_node *reference, *parent, *root, *note;
    doc_command_footnote_reach_t* notes = NULL;
    uint64_t parent_id, index, count = 0, i, j;
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (!reference_id || !caret) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    reference = doc_index_get(t->draft->index, reference_id);
    if (!reference || reference->kind != XUI_DOC_FOOTNOTE_REF)
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_position_t target;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnRemoveFootnoteReference(&shadow, reference_id, &target);
        return doc_markdown_shadow_end(t, &shadow, result, &target, caret);
    }
    parent_id = reference->parent;
    parent = doc_index_get(t->draft->index, parent_id);
    index = parent ? doc_child_index(parent, reference_id) : DOC_NONE;
    if (!doc_paragraph(parent) || index == DOC_NONE)
        return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    root = doc_index_get(t->draft->index, DOC_ROOT);
    if (!root) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    for (i = 0; i < doc_seq_size(root->children); i++) {
        note = doc_index_get(t->draft->index, doc_seq_get_id(root->children, i));
        if (note && note->kind == XUI_DOC_FOOTNOTE) count++;
    }
    if (count) {
        if (count > SIZE_MAX / sizeof(*notes))
            return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
        notes = doc_alloc(t->draft->allocator, (size_t)count * sizeof(*notes));
        if (!notes) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
        memset(notes, 0, (size_t)count * sizeof(*notes));
        count = 0;
        for (i = 0; i < doc_seq_size(root->children); i++) {
            note = doc_index_get(t->draft->index, doc_seq_get_id(root->children, i));
            if (!note || note->kind != XUI_DOC_FOOTNOTE) continue;
            notes[count].id = note->id; notes[count].label = doc_string(note->info);
            for (j = 0; j < count; j++) {
                if (doc_command_footnote_label_equal(notes[j].label, notes[count].label)) {
                    result = XUI_DOC_ERROR_SCHEMA; goto done;
                }
            }
            count++;
        }
        doc_command_calculate_footnote_reach(t->draft, root, 0, notes, count, 1);
        doc_command_calculate_footnote_reach(t->draft, root, reference_id, notes, count, 0);
    }
    result = doc_txn_delete(t, reference_id); if (result != XUI_OK) goto done;
    for (i = 0; i < count; i++) {
        if (!notes[i].before || notes[i].after) continue;
        result = doc_txn_delete(t, notes[i].id); if (result != XUI_OK) goto done;
    }
    *caret = doc_command_caret(t, parent_id, index, XUI_DOC_POSITION_GAP);
done:
    doc_free(notes);
    return result == XUI_OK ? XUI_OK : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnUpdateImage(xui_document_transaction t, xui_doc_node_id image_id,
    const xui_doc_image_desc_t* image)
{
    doc_blob *resource = NULL, *alt = NULL, *title = NULL;
    doc_node* node; xui_doc_attributes_t attrs; uint64_t old_alt_bytes; int result;
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!doc_image_desc_valid(image)) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    node = doc_index_get(t->draft->index, image_id);
    if (!node || node->kind != XUI_DOC_IMAGE) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && (image->fWidth > 0 || image->fHeight > 0))
        return doc_txn_fail(t, XUI_DOC_ERROR_UNREPRESENTABLE);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnUpdateImage(&shadow, image_id, image);
        return doc_markdown_shadow_end_image(t, &shadow, result, image_id);
    }
    result = doc_image_copy(t, image, &resource, &alt, &title);
    if (result != XUI_OK) goto done;
    old_alt_bytes = doc_seq_size(node->text);
    result = xuiDocumentTxnReplaceText(t, image_id, 0, old_alt_bytes, doc_string(alt), image->iAltBytes);
    if (result != XUI_OK) goto done;
    node = doc_index_get(t->draft->index, image_id);
    result = xuiDocumentTxnSetResource(t, image_id, doc_string(resource), doc_string(node->info), doc_string(title));
    if (result != XUI_OK) goto done;
    node = doc_index_get(t->draft->index, image_id);
    attrs = *node->attrs; attrs.fWidth = image->fWidth; attrs.fHeight = image->fHeight;
    if (image->sLinkTarget) attrs.iMarks |= XUI_DOC_LINK;
    else attrs.iMarks &= ~XUI_DOC_LINK;
    result = xuiDocumentTxnSetAttributes(t, image_id, &attrs);
    if (result == XUI_OK) result = doc_image_link_set(t, image_id, image);
done:
    doc_blob_release(resource); doc_blob_release(alt); doc_blob_release(title);
    return result == XUI_OK ? XUI_OK : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnSplitBlock(xui_document_transaction t, const xui_doc_position_t* at, xui_doc_position_t* caret)
{
    doc_node *n, *block; xui_doc_position_t p; uint64_t index, block_id, parent, next, tail = 0, new_text = 0, moves; int result;
    xui_doc_node_desc_t desc = {0};
    xui_doc_operation_t split = {0};
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!caret || (result = doc_command_position(t, at)) != XUI_OK) return doc_txn_fail(t, caret ? result : XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_position_t target;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnSplitBlock(&shadow, at, &target);
        return doc_markdown_shadow_end(t, &shadow, result, &target, caret);
    }
    p = *at; n = doc_index_get(t->draft->index, p.iNodeId);
    if (p.iKind == XUI_DOC_POSITION_TEXT) {
        block = doc_index_get(t->draft->index, n->parent);
        if (n->kind != XUI_DOC_TEXT || !doc_paragraph(block)) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
        block_id = block->id; parent = block->parent; index = doc_child_index(block, n->id);
        if (p.iOffset == 0) tail = n->id;
        else if (p.iOffset < doc_seq_size(n->text)) {
            result = doc_split_text(t, n->id, p.iOffset, &tail); if (result != XUI_OK) return result;
            index++;
        } else index++;
    } else if (p.iKind == XUI_DOC_POSITION_GAP && doc_paragraph(n)) {
        block_id = n->id; parent = n->parent; index = p.iOffset;
    } else return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    block = doc_index_get(t->draft->index, block_id);
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_PARAGRAPH; desc.tAttributes = *block->attrs; desc.tAttributes.iHeadingLevel = 0;
    result = doc_txn_insert(t, parent, doc_child_index(doc_index_get(t->draft->index, parent), block_id) + 1, &desc, &next);
    if (result != XUI_OK) return result;
    moves = t->count;
    while (index < doc_seq_size((block = doc_index_get(t->draft->index, block_id))->children)) {
        uint64_t child = doc_seq_get_id(block->children, index);
        if (!tail) tail = child;
        result = xuiDocumentTxnMoveNode(t, child, next, DOC_NONE); if (result != XUI_OK) return result;
    }
    doc_command_relocations(t, moves);
    split.iKind = XUI_DOC_OP_SPLIT; split.iFlags = XUI_DOC_CHANGE_STRUCTURE;
    split.iNodeId = block_id; split.iOtherNodeId = next; split.iParentId = parent; split.iOffset = index;
    result = doc_txn_op(t, &split); if (result != XUI_OK) return result;
    n = doc_index_get(t->draft->index, tail);
    if (!n || n->kind != XUI_DOC_TEXT) {
        result = doc_insert_empty(t, next, 0, XUI_DOC_TEXT, &new_text); if (result != XUI_OK) return result;
    }
    n = doc_index_get(t->draft->index, parent);
    if (n && n->kind == XUI_DOC_LIST_ITEM) {
        doc_node* list = doc_index_get(t->draft->index, n->parent);
        xui_doc_attributes_t attrs = *list->attrs; attrs.iFlags &= ~XUI_DOC_TIGHT;
        result = xuiDocumentTxnSetAttributes(t, list->id, &attrs); if (result != XUI_OK) return result;
    }
    *caret = doc_command_caret(t, new_text ? new_text : tail, 0, XUI_DOC_POSITION_TEXT); return XUI_OK;
}
XUI_API int xuiDocumentTxnSplitListItem(xui_document_transaction t, const xui_doc_position_t* at, xui_doc_position_t* caret)
{
    doc_node *n, *block, *item, *list; xui_doc_position_t target;
    xui_doc_node_desc_t desc = {0}; xui_doc_attributes_t list_attrs;
    uint64_t item_id, list_id, new_item, new_block, index; int result;
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!caret || (result = doc_command_position(t, at)) != XUI_OK)
        return doc_txn_fail(t, caret ? result : XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnSplitListItem(&shadow, at, &target);
        return doc_markdown_shadow_end_list_split(t, &shadow, result, at, &target, caret);
    }
    n = doc_index_get(t->draft->index, at->iNodeId);
    block = at->iKind == XUI_DOC_POSITION_TEXT && n && n->kind == XUI_DOC_TEXT ?
        doc_index_get(t->draft->index, n->parent) :
        at->iKind == XUI_DOC_POSITION_GAP ? n : NULL;
    if (!doc_paragraph(block)) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    item = doc_index_get(t->draft->index, block->parent);
    list = item && item->kind == XUI_DOC_LIST_ITEM ? doc_index_get(t->draft->index, item->parent) : NULL;
    if (!list || list->kind != XUI_DOC_LIST) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    item_id = item->id; list_id = list->id; list_attrs = *list->attrs;
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_LIST_ITEM; desc.tAttributes = *item->attrs;
    desc.tAttributes.iFlags &= ~XUI_DOC_CHECKED;
    result = xuiDocumentTxnSplitBlock(t, at, &target); if (result != XUI_OK) return result;
    n = doc_index_get(t->draft->index, target.iNodeId);
    new_block = n ? n->parent : 0;
    item = doc_index_get(t->draft->index, item_id);
    list = doc_index_get(t->draft->index, list_id);
    if (!item || !list || !new_block || (index = doc_child_index(item, new_block)) == DOC_NONE)
        return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    result = doc_txn_insert(t, list_id, doc_child_index(list, item_id) + 1, &desc, &new_item);
    if (result != XUI_OK) return result;
    while ((item = doc_index_get(t->draft->index, item_id)) && index < doc_seq_size(item->children)) {
        uint64_t child = doc_seq_get_id(item->children, index);
        result = xuiDocumentTxnMoveNode(t, child, new_item, DOC_NONE);
        if (result != XUI_OK) return result;
    }
    list = doc_index_get(t->draft->index, list_id);
    if (!doc_attributes_equal(list->attrs, &list_attrs)) {
        result = xuiDocumentTxnSetAttributes(t, list_id, &list_attrs);
        if (result != XUI_OK) return result;
    }
    *caret = target; return XUI_OK;
}
XUI_API int xuiDocumentTxnExitListItem(xui_document_transaction t, const xui_doc_position_t* at, xui_doc_position_t* caret)
{
    doc_node *n, *item, *list, *parent; xui_doc_position_t target;
    xui_doc_node_desc_t desc = {0}; uint64_t item_id, list_id, parent_id;
    uint64_t item_index, list_index, count, tail_list = 0, paragraph, text_id, i;
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (!caret || (result = doc_command_position(t, at)) != XUI_OK)
        return doc_txn_fail(t, caret ? result : XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnExitListItem(&shadow, at, &target);
        return doc_markdown_shadow_end(t, &shadow, result, &target, caret);
    }
    n = doc_index_get(t->draft->index, at->iNodeId);
    if (at->iKind == XUI_DOC_POSITION_TEXT && n && n->kind == XUI_DOC_TEXT)
        n = doc_index_get(t->draft->index, n->parent);
    if (n && doc_paragraph(n)) n = doc_index_get(t->draft->index, n->parent);
    item = at->iKind != XUI_DOC_POSITION_SOURCE && n && n->kind == XUI_DOC_LIST_ITEM ? n : NULL;
    list = item ? doc_index_get(t->draft->index, item->parent) : NULL;
    if (!list || list->kind != XUI_DOC_LIST) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    for (i = 0; i < doc_seq_size(item->children); i++) {
        n = doc_index_get(t->draft->index, doc_seq_get_id(item->children, i));
        if (!doc_semantic_empty_paragraph(t->draft, n)) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    }
    parent = doc_index_get(t->draft->index, list->parent);
    item_id = item->id; list_id = list->id; parent_id = parent->id;
    item_index = doc_child_index(list, item_id); list_index = doc_child_index(parent, list_id);
    count = doc_seq_size(list->children);
    if (item_index != DOC_NONE && item_index && item_index + 1 < count) {
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_LIST; desc.tAttributes = *list->attrs;
        if (desc.tAttributes.iFlags & XUI_DOC_ORDERED) {
            if (desc.tAttributes.iListStart > UINT64_MAX - item_index - 1)
                return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
            desc.tAttributes.iListStart += item_index + 1;
        }
        result = doc_txn_insert(t, parent_id, list_index + 1, &desc, &tail_list);
        if (result != XUI_OK) return result;
        while ((list = doc_index_get(t->draft->index, list_id)) && item_index + 1 < doc_seq_size(list->children)) {
            uint64_t child = doc_seq_get_id(list->children, item_index + 1);
            result = xuiDocumentTxnMoveNode(t, child, tail_list, DOC_NONE);
            if (result != XUI_OK) return result;
        }
    }
    result = doc_txn_delete(t, item_id); if (result != XUI_OK) return result;
    if (count == 1) {
        result = doc_txn_delete(t, list_id); if (result != XUI_OK) return result;
    }
    memset(&desc, 0, sizeof(desc)); desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_PARAGRAPH;
    result = doc_txn_insert(t, parent_id, list_index + (item_index ? 1 : 0), &desc, &paragraph);
    if (result != XUI_OK) return result;
    result = doc_insert_empty(t, paragraph, 0, XUI_DOC_TEXT, &text_id);
    if (result == XUI_OK) *caret = doc_command_caret(t, text_id, 0, XUI_DOC_POSITION_TEXT);
    return result;
}
static doc_node* doc_command_list_item(doc_state* state, const xui_doc_position_t* at)
{
    doc_node* n;
    if (at->iKind == XUI_DOC_POSITION_SOURCE) return NULL;
    n = doc_index_get(state->index, at->iNodeId);
    while (n && n->kind != XUI_DOC_LIST_ITEM) n = doc_index_get(state->index, n->parent);
    return n;
}
static int doc_command_list_compatible(const doc_node* a, const doc_node* b)
{
    return a && b && a->kind == XUI_DOC_LIST && b->kind == XUI_DOC_LIST &&
        !!(a->attrs->iFlags & XUI_DOC_ORDERED) == !!(b->attrs->iFlags & XUI_DOC_ORDERED) &&
        !!(a->attrs->iFlags & XUI_DOC_TIGHT) == !!(b->attrs->iFlags & XUI_DOC_TIGHT);
}
static int doc_command_unlist_item(xui_document_transaction t, const xui_doc_position_t* at,
    xui_doc_position_t* caret, doc_node* item, doc_node* list)
{
    doc_node* parent = doc_index_get(t->draft->index, list->parent);
    xui_doc_node_desc_t desc = {0}; xui_doc_attributes_t attrs;
    uint64_t item_id = item->id, list_id = list->id, parent_id = parent->id;
    uint64_t item_index = doc_child_index(list, item_id), list_index = doc_child_index(parent, list_id);
    uint64_t count = doc_seq_size(list->children), insert_index = list_index + !!item_index;
    uint64_t first_child = doc_seq_size(item->children) ? doc_seq_get_id(item->children, 0) : 0;
    uint64_t i, tail = 0; int result;
    for (i = 0; i < doc_seq_size(item->children); i++) {
        doc_node* child = doc_index_get(t->draft->index, doc_seq_get_id(item->children, i));
        if (!doc_semantic_empty_paragraph(t->draft, child)) break;
    }
    if (i == doc_seq_size(item->children)) return xuiDocumentTxnExitListItem(t, at, caret);
    if (item_index && item_index + 1 < count) {
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_LIST; desc.tAttributes = *list->attrs;
        if (desc.tAttributes.iFlags & XUI_DOC_ORDERED) {
            if (desc.tAttributes.iListStart > UINT64_MAX - item_index - 1)
                return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
            desc.tAttributes.iListStart += item_index + 1;
        }
        result = doc_txn_insert(t, parent_id, list_index + 1, &desc, &tail);
        if (result != XUI_OK) return result;
        while ((list = doc_index_get(t->draft->index, list_id)) && item_index + 1 < doc_seq_size(list->children)) {
            uint64_t sibling = doc_seq_get_id(list->children, item_index + 1);
            result = xuiDocumentTxnMoveNode(t, sibling, tail, DOC_NONE);
            if (result != XUI_OK) return result;
        }
    }
    while ((item = doc_index_get(t->draft->index, item_id)) && doc_seq_size(item->children)) {
        uint64_t child = doc_seq_get_id(item->children, 0);
        result = xuiDocumentTxnMoveNode(t, child, parent_id, insert_index++);
        if (result != XUI_OK) return result;
    }
    result = doc_txn_delete(t, item_id); if (result != XUI_OK) return result;
    if (count == 1) {
        result = doc_txn_delete(t, list_id); if (result != XUI_OK) return result;
    } else if (!item_index && (list = doc_index_get(t->draft->index, list_id))->attrs->iFlags & XUI_DOC_ORDERED) {
        attrs = *list->attrs;
        if (attrs.iListStart == UINT64_MAX) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
        attrs.iListStart++;
        result = xuiDocumentTxnSetAttributes(t, list_id, &attrs); if (result != XUI_OK) return result;
    }
    *caret = *at;
    if (at->iNodeId == item_id) {
        doc_node* first = doc_index_get(t->draft->index, first_child);
        if (first && doc_paragraph(first)) {
            caret->iNodeId = first_child; caret->iKind = XUI_DOC_POSITION_GAP; caret->iOffset = 0;
        } else {
            caret->iNodeId = parent_id; caret->iKind = XUI_DOC_POSITION_GAP;
            caret->iOffset = insert_index - 1;
        }
    }
    return XUI_OK;
}
XUI_API int xuiDocumentTxnIndentListItem(xui_document_transaction t, const xui_doc_position_t* at, xui_doc_position_t* caret)
{
    doc_node *item, *list, *previous, *nested = NULL; xui_doc_node_desc_t desc = {0};
    uint64_t item_id, previous_id, nested_id, index, count; int result;
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!caret || (result = doc_command_position(t, at)) != XUI_OK)
        return doc_txn_fail(t, caret ? result : XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_position_t target;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnIndentListItem(&shadow, at, &target);
        return doc_markdown_shadow_end(t, &shadow, result, &target, caret);
    }
    item = doc_command_list_item(t->draft, at);
    list = item ? doc_index_get(t->draft->index, item->parent) : NULL;
    if (!list || list->kind != XUI_DOC_LIST || !(index = doc_child_index(list, item->id)) || index == DOC_NONE)
        return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    item_id = item->id;
    previous_id = doc_seq_get_id(list->children, index - 1);
    previous = doc_index_get(t->draft->index, previous_id);
    count = doc_seq_size(previous->children);
    if (count) nested = doc_index_get(t->draft->index, doc_seq_get_id(previous->children, count - 1));
    if (doc_command_list_compatible(list, nested)) nested_id = nested->id;
    else {
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_LIST; desc.tAttributes = *list->attrs;
        if (desc.tAttributes.iFlags & XUI_DOC_ORDERED) desc.tAttributes.iListStart = 1;
        result = doc_txn_insert(t, previous_id, DOC_NONE, &desc, &nested_id);
        if (result != XUI_OK) return result;
    }
    result = xuiDocumentTxnMoveNode(t, item_id, nested_id, DOC_NONE);
    if (result == XUI_OK) *caret = *at;
    return result;
}
XUI_API int xuiDocumentTxnOutdentListItem(xui_document_transaction t, const xui_doc_position_t* at, xui_doc_position_t* caret)
{
    doc_node *item, *list, *parent_item, *outer, *last = NULL;
    xui_doc_node_desc_t desc = {0};
    uint64_t item_id, list_id, outer_id, item_index, parent_index, tail_count, tail_list = 0, count;
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (!caret || (result = doc_command_position(t, at)) != XUI_OK)
        return doc_txn_fail(t, caret ? result : XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_position_t target;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnOutdentListItem(&shadow, at, &target);
        return doc_markdown_shadow_end(t, &shadow, result, &target, caret);
    }
    item = doc_command_list_item(t->draft, at);
    list = item ? doc_index_get(t->draft->index, item->parent) : NULL;
    parent_item = list && list->kind == XUI_DOC_LIST ? doc_index_get(t->draft->index, list->parent) : NULL;
    outer = parent_item && parent_item->kind == XUI_DOC_LIST_ITEM ? doc_index_get(t->draft->index, parent_item->parent) : NULL;
    if (!outer || outer->kind != XUI_DOC_LIST) {
        if (list && list->kind == XUI_DOC_LIST && parent_item && parent_item->kind != XUI_DOC_LIST_ITEM)
            return doc_command_unlist_item(t, at, caret, item, list);
        return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    }
    item_id = item->id; list_id = list->id; outer_id = outer->id;
    item_index = doc_child_index(list, item_id); parent_index = doc_child_index(outer, parent_item->id);
    tail_count = doc_seq_size(list->children) - item_index - 1;
    if (tail_count) {
        count = doc_seq_size(item->children);
        if (count) last = doc_index_get(t->draft->index, doc_seq_get_id(item->children, count - 1));
        if (doc_command_list_compatible(list, last)) tail_list = last->id;
        else {
            desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_LIST; desc.tAttributes = *list->attrs;
            if (desc.tAttributes.iFlags & XUI_DOC_ORDERED) {
                if (desc.tAttributes.iListStart > UINT64_MAX - item_index - 1)
                    return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
                desc.tAttributes.iListStart += item_index + 1;
            }
            result = doc_txn_insert(t, item_id, DOC_NONE, &desc, &tail_list);
            if (result != XUI_OK) return result;
        }
        while ((list = doc_index_get(t->draft->index, list_id)) && item_index + 1 < doc_seq_size(list->children)) {
            uint64_t sibling = doc_seq_get_id(list->children, item_index + 1);
            result = xuiDocumentTxnMoveNode(t, sibling, tail_list, DOC_NONE);
            if (result != XUI_OK) return result;
        }
    }
    result = xuiDocumentTxnMoveNode(t, item_id, outer_id, parent_index + 1);
    if (result != XUI_OK) return result;
    list = doc_index_get(t->draft->index, list_id);
    if (!doc_seq_size(list->children)) {
        result = doc_txn_delete(t, list_id); if (result != XUI_OK) return result;
    }
    *caret = *at; return XUI_OK;
}
int doc_list_range_can(doc_state* state, const xui_doc_range_t* range, int outdent)
{
    doc_node *a, *b, *first, *last, *first_list, *last_list, *parent, *candidate;
    uint64_t first_index, last_index, i; int order, result;
    if (!state || !range) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_position_compare(state, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    a = doc_command_list_item(state, &range->tAnchor);
    b = doc_command_list_item(state, &range->tCaret);
    if (!a || !b) return XUI_ERROR_UNSUPPORTED;
    first = order <= 0 ? a : b; last = order <= 0 ? b : a;
    first_list = doc_index_get(state->index, first->parent);
    last_list = doc_index_get(state->index, last->parent);
    if (!first_list || !last_list || first_list->kind != XUI_DOC_LIST || last_list->kind != XUI_DOC_LIST)
        return XUI_DOC_ERROR_SCHEMA;
    first_index = doc_child_index(first_list, first->id);
    last_index = doc_child_index(last_list, last->id);
    if (first_index == DOC_NONE || last_index == DOC_NONE) return XUI_DOC_ERROR_SCHEMA;
    if (first_list->id == last_list->id) return !outdent && !first_index ? XUI_ERROR_UNSUPPORTED : XUI_OK;
    if (!order || first_list->parent != last_list->parent) return XUI_ERROR_UNSUPPORTED;
    parent = doc_index_get(state->index, first_list->parent);
    if (!parent) return XUI_ERROR_UNSUPPORTED;
    first_index = doc_child_index(parent, first_list->id);
    last_index = doc_child_index(parent, last_list->id);
    if (first_index == DOC_NONE || last_index == DOC_NONE || first_index >= last_index)
        return XUI_DOC_ERROR_SCHEMA;
    for (i = first_index; i <= last_index; i++) {
        candidate = doc_index_get(state->index, doc_seq_get_id(parent->children, i));
        if (!candidate || candidate->kind != XUI_DOC_LIST || !doc_seq_size(candidate->children))
            return XUI_ERROR_UNSUPPORTED;
    }
    if (!outdent && !doc_child_index(first_list, first->id)) {
        if (!first_index) return XUI_ERROR_UNSUPPORTED;
        candidate = doc_index_get(state->index, doc_seq_get_id(parent->children, first_index - 1));
        if (!candidate || candidate->kind != XUI_DOC_LIST || !doc_seq_size(candidate->children))
            return XUI_ERROR_UNSUPPORTED;
    }
    return XUI_OK;
}
typedef struct doc_list_range_group {
    uint64_t list, offset, count;
} doc_list_range_group;
static int doc_command_indent_list_group(xui_document_transaction t,
    const doc_list_range_group* group, const uint64_t* ids)
{
    doc_node *list = doc_index_get(t->draft->index, group->list), *parent, *previous, *nested = NULL;
    xui_doc_node_desc_t desc = {0}; xui_doc_attributes_t attrs;
    uint64_t index, previous_id, nested_id, i, count; int result;
    if (!list || list->kind != XUI_DOC_LIST || !group->count) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    index = doc_child_index(list, ids[group->offset]);
    if (index == DOC_NONE) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    if (index) previous_id = doc_seq_get_id(list->children, index - 1);
    else {
        uint64_t list_index;
        parent = doc_index_get(t->draft->index, list->parent);
        list_index = parent ? doc_child_index(parent, list->id) : DOC_NONE;
        if (!parent || !list_index || list_index == DOC_NONE) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
        previous = doc_index_get(t->draft->index, doc_seq_get_id(parent->children, list_index - 1));
        if (!previous || previous->kind != XUI_DOC_LIST ||
            !(count = doc_seq_size(previous->children))) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
        previous_id = doc_seq_get_id(previous->children, count - 1);
    }
    previous = doc_index_get(t->draft->index, previous_id);
    count = doc_seq_size(previous->children);
    if (count) nested = doc_index_get(t->draft->index, doc_seq_get_id(previous->children, count - 1));
    if (doc_command_list_compatible(list, nested)) nested_id = nested->id;
    else {
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_LIST; desc.tAttributes = *list->attrs;
        if (desc.tAttributes.iFlags & XUI_DOC_ORDERED) desc.tAttributes.iListStart = 1;
        result = doc_txn_insert(t, previous_id, DOC_NONE, &desc, &nested_id);
        if (result != XUI_OK) return result;
    }
    for (i = 0; i < group->count; i++) {
        result = xuiDocumentTxnMoveNode(t, ids[group->offset + i], nested_id, DOC_NONE);
        if (result != XUI_OK) return result;
    }
    list = doc_index_get(t->draft->index, group->list);
    if (!list) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    if (!doc_seq_size(list->children)) return doc_txn_delete(t, list->id);
    if (!index && (list->attrs->iFlags & XUI_DOC_ORDERED)) {
        attrs = *list->attrs;
        if (attrs.iListStart > UINT64_MAX - group->count)
            return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
        attrs.iListStart += group->count;
        return xuiDocumentTxnSetAttributes(t, list->id, &attrs);
    }
    return XUI_OK;
}
static int doc_command_cross_list_range(xui_document_transaction t,
    const xui_doc_range_t* range, xui_doc_range_t* after, int outdent, int order,
    const doc_node* a, const doc_node* b)
{
    doc_node *first = order < 0 ? (doc_node*)a : (doc_node*)b;
    doc_node *last = order < 0 ? (doc_node*)b : (doc_node*)a;
    doc_node *first_list = doc_index_get(t->draft->index, first->parent);
    doc_node *last_list = doc_index_get(t->draft->index, last->parent);
    doc_node *parent = doc_index_get(t->draft->index, first_list->parent), *list;
    doc_list_range_group* groups = NULL; uint64_t* ids = NULL;
    uint64_t first_list_index = doc_child_index(parent, first_list->id);
    uint64_t last_list_index = doc_child_index(parent, last_list->id);
    uint64_t group_count = last_list_index - first_list_index + 1;
    uint64_t count = 0, i, j, k, item_index, end_index; int result = XUI_OK;
    if (group_count > SIZE_MAX / sizeof(*groups)) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    groups = doc_alloc(t->document->allocator, (size_t)group_count * sizeof(*groups));
    if (!groups) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    for (i = 0; i < group_count; i++) {
        list = doc_index_get(t->draft->index, doc_seq_get_id(parent->children, first_list_index + i));
        item_index = i ? 0 : doc_child_index(list, first->id);
        end_index = i + 1 == group_count ? doc_child_index(list, last->id) + 1 : doc_seq_size(list->children);
        if (end_index <= item_index || end_index - item_index > SIZE_MAX / sizeof(*ids) - count) {
            result = XUI_DOC_ERROR_LIMIT; goto done;
        }
        groups[i] = (doc_list_range_group){list->id, count, end_index - item_index};
        count += end_index - item_index;
    }
    ids = doc_alloc(t->document->allocator, (size_t)count * sizeof(*ids));
    if (!ids) { result = XUI_ERROR_OUT_OF_MEMORY; goto done; }
    for (i = 0; i < group_count; i++) {
        list = doc_index_get(t->draft->index, groups[i].list);
        item_index = i ? 0 : doc_child_index(list, first->id);
        for (j = 0; j < groups[i].count; j++)
            ids[groups[i].offset + j] = doc_seq_get_id(list->children, item_index + j);
    }
    *after = *range;
    if (outdent) {
        /* Each nested group inserts immediately after its owning item.
         * Process sibling groups backwards to keep their document order. */
        for (i = group_count; i-- > 0 && result == XUI_OK;) {
            for (j = 0; j < groups[i].count; j++) {
                doc_node* anchor_item;
                doc_node* caret_item;
                xui_doc_position_t at, moved;
                k = groups[i].offset + j;
                anchor_item = doc_command_list_item(t->draft, &after->tAnchor);
                caret_item = doc_command_list_item(t->draft, &after->tCaret);
                at = anchor_item && anchor_item->id == ids[k] ? after->tAnchor :
                    caret_item && caret_item->id == ids[k] ? after->tCaret :
                    doc_command_caret(t, ids[k], 0, XUI_DOC_POSITION_GAP);
                result = xuiDocumentTxnOutdentListItem(t, &at, &moved);
                if (result != XUI_OK) break;
                if (!doc_position_valid(t->draft, &after->tAnchor)) after->tAnchor = moved;
                if (!doc_position_valid(t->draft, &after->tCaret)) after->tCaret = moved;
            }
        }
    } else for (i = 0; i < group_count && result == XUI_OK; i++)
        result = doc_command_indent_list_group(t, &groups[i], ids);
done:
    doc_free(ids); doc_free(groups);
    return result == XUI_OK ? XUI_OK : doc_txn_fail(t, result);
}
static int doc_command_list_range(xui_document_transaction t, const xui_doc_range_t* range,
    xui_doc_range_t* after, int outdent)
{
    doc_node *a, *b, *list; xui_doc_position_t at, moved;
    uint64_t *ids = NULL, first, last, count, i; int result, order;
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!range || !after) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result == XUI_OK) result = doc_position_compare(t->draft, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_range_t target;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = doc_command_list_range(&shadow, range, &target, outdent);
        return doc_markdown_shadow_end_range(t, &shadow, result, &target, after);
    }
    a = doc_command_list_item(t->draft, &range->tAnchor);
    b = doc_command_list_item(t->draft, &range->tCaret);
    list = a ? doc_index_get(t->draft->index, a->parent) : NULL;
    result = doc_list_range_can(t->draft, range, outdent);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (a->parent != b->parent)
        return doc_command_cross_list_range(t, range, after, outdent, order, a, b);
    first = doc_child_index(list, a->id); last = doc_child_index(list, b->id);
    if (first == DOC_NONE || last == DOC_NONE) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    if (first > last) { uint64_t swap = first; first = last; last = swap; }
    if (!outdent && !first) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    count = last - first + 1;
    if (count > SIZE_MAX / sizeof(*ids)) return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
    ids = doc_alloc(t->document->allocator, (size_t)count * sizeof(*ids));
    if (!ids) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    for (i = 0; i < count; i++) ids[i] = doc_seq_get_id(list->children, first + i);
    *after = *range;
    for (i = 0; i < count; i++) {
        doc_node* anchor_item = doc_command_list_item(t->draft, &after->tAnchor);
        doc_node* caret_item = doc_command_list_item(t->draft, &after->tCaret);
        at = anchor_item && anchor_item->id == ids[i] ? after->tAnchor :
            caret_item && caret_item->id == ids[i] ? after->tCaret :
            doc_command_caret(t, ids[i], 0, XUI_DOC_POSITION_GAP);
        result = outdent ? xuiDocumentTxnOutdentListItem(t, &at, &moved) :
            xuiDocumentTxnIndentListItem(t, &at, &moved);
        if (result != XUI_OK) break;
        if (!doc_position_valid(t->draft, &after->tAnchor)) after->tAnchor = moved;
        if (!doc_position_valid(t->draft, &after->tCaret)) after->tCaret = moved;
    }
    doc_free(ids);
    return result == XUI_OK ? XUI_OK : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnIndentListRange(xui_document_transaction t, const xui_doc_range_t* range, xui_doc_range_t* after)
{
    return doc_command_list_range(t, range, after, 0);
}
XUI_API int xuiDocumentTxnOutdentListRange(xui_document_transaction t, const xui_doc_range_t* range, xui_doc_range_t* after)
{
    return doc_command_list_range(t, range, after, 1);
}
typedef struct doc_block_collect {
    doc_state* state;
    uint64_t first, last, *ids, count, capacity;
    int started, done, error;
} doc_block_collect;
static doc_node* doc_command_block(doc_state* state, const xui_doc_position_t* at)
{
    doc_node *n, *parent;
    if (at->iKind == XUI_DOC_POSITION_SOURCE) return NULL;
    n = doc_index_get(state->index, at->iNodeId);
    while (n && !doc_paragraph(n)) n = doc_index_get(state->index, n->parent);
    if (n && state->profile == XUI_DOCUMENT_MARKDOWN) {
        parent = doc_index_get(state->index, n->parent);
        while (parent) {
            if (parent->kind == XUI_DOC_TABLE) return NULL;
            parent = doc_index_get(state->index, parent->parent);
        }
    }
    return n;
}
static void doc_command_append_block(doc_block_collect* c, uint64_t id)
{
    if (c->count == c->capacity) {
        uint64_t capacity = c->capacity ? c->capacity * 2 : 8; uint64_t* next;
        if (capacity < c->capacity || capacity > SIZE_MAX / sizeof(*c->ids)) {
            c->error = XUI_DOC_ERROR_LIMIT; return;
        }
        next = doc_realloc(c->state->allocator, c->ids, (size_t)capacity * sizeof(*c->ids));
        if (!next) { c->error = XUI_ERROR_OUT_OF_MEMORY; return; }
        c->ids = next; c->capacity = capacity;
    }
    c->ids[c->count++] = id;
}
static void doc_command_collect_blocks(doc_block_collect* c, uint64_t id)
{
    doc_node* n = doc_index_get(c->state->index, id); uint64_t i;
    if (c->error || c->done) return;
    if (c->state->profile == XUI_DOCUMENT_MARKDOWN && n->kind == XUI_DOC_TABLE) return;
    if (doc_paragraph(n)) {
        if (id == c->first) c->started = 1;
        if (c->started) doc_command_append_block(c, id);
        if (id == c->last) c->done = 1;
        return;
    }
    for (i = 0; i < doc_seq_size(n->children) && !c->error && !c->done; i++)
        doc_command_collect_blocks(c, doc_seq_get_id(n->children, i));
}
static void doc_command_collect_all_blocks(doc_block_collect* c, uint64_t id)
{
    doc_node* n = doc_index_get(c->state->index, id); uint64_t i;
    if (c->error) return;
    if (c->state->profile == XUI_DOCUMENT_MARKDOWN && n->kind == XUI_DOC_TABLE) return;
    if (doc_paragraph(n)) { doc_command_append_block(c, id); return; }
    for (i = 0; i < doc_seq_size(n->children) && !c->error; i++)
        doc_command_collect_all_blocks(c, doc_seq_get_id(n->children, i));
}
int doc_block_range_ids(doc_state* state, const xui_doc_range_t* range, uint64_t** ids, uint64_t* count)
{
    doc_block_collect c = {0}; doc_node *a, *b; int order, result;
    if (!state || !range || !ids || !count) return XUI_ERROR_INVALID_ARGUMENT;
    *ids = NULL; *count = 0;
    result = doc_position_compare(state, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    if (order && range->tAnchor.iKind == XUI_DOC_POSITION_GAP &&
        range->tCaret.iKind == XUI_DOC_POSITION_GAP && range->tAnchor.iNodeId == range->tCaret.iNodeId) {
        doc_node* container = doc_index_get(state->index, range->tAnchor.iNodeId);
        if (!doc_paragraph(container)) {
            uint64_t start = order < 0 ? range->tAnchor.iOffset : range->tCaret.iOffset;
            uint64_t end = order < 0 ? range->tCaret.iOffset : range->tAnchor.iOffset, i;
            c.state = state;
            for (i = start; i < end && !c.error; i++)
                doc_command_collect_all_blocks(&c, doc_seq_get_id(container->children, i));
            if (c.error || !c.count) { result = c.error ? c.error : XUI_ERROR_UNSUPPORTED; doc_free(c.ids); return result; }
            *ids = c.ids; *count = c.count; return XUI_OK;
        }
    }
    a = doc_command_block(state, &range->tAnchor);
    b = doc_command_block(state, &range->tCaret);
    if (!a || !b) return XUI_ERROR_UNSUPPORTED;
    c.state = state; c.first = order <= 0 ? a->id : b->id; c.last = order <= 0 ? b->id : a->id;
    doc_command_collect_blocks(&c, DOC_ROOT);
    if (!c.error && (!c.done || !c.count)) c.error = XUI_DOC_ERROR_SCHEMA;
    if (c.error) { doc_free(c.ids); return c.error; }
    *ids = c.ids; *count = c.count; return XUI_OK;
}
typedef struct doc_list_create_plan {
    uint64_t* blocks;
    uint64_t count, parent, index;
    int empty;
} doc_list_create_plan;
static int doc_command_list_create_plan(doc_state* s, const xui_doc_range_t* range,
    uint32_t flags, uint64_t start, doc_list_create_plan* plan)
{
    doc_node *first, *parent, *block; uint64_t i;
    int order, result;
    memset(plan, 0, sizeof(*plan));
    if (!range || flags & ~(XUI_DOC_ORDERED | XUI_DOC_TASK) ||
        ((flags & XUI_DOC_ORDERED) ? !start : !!start)) return XUI_ERROR_INVALID_ARGUMENT;
    if ((flags & XUI_DOC_TASK) && s->profile == XUI_DOCUMENT_MARKDOWN &&
        s->dialect == XUI_MD_COMMONMARK) return XUI_DOC_ERROR_UNREPRESENTABLE;
    result = doc_position_compare(s, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE ||
        range->tCaret.iKind == XUI_DOC_POSITION_SOURCE) return XUI_DOC_ERROR_DOMAIN;
    result = doc_block_range_ids(s, range, &plan->blocks, &plan->count);
    if (result == XUI_ERROR_UNSUPPORTED && !order &&
        range->tAnchor.iKind == XUI_DOC_POSITION_GAP &&
        range->tCaret.iKind == XUI_DOC_POSITION_GAP &&
        range->tAnchor.iNodeId == range->tCaret.iNodeId &&
        range->tAnchor.iOffset == range->tCaret.iOffset) {
        parent = doc_index_get(s->index, range->tAnchor.iNodeId);
        if (parent && doc_schema_child(parent->kind, XUI_DOC_LIST)) {
            plan->parent = parent->id;
            plan->index = range->tAnchor.iOffset;
            plan->count = 1; plan->empty = 1;
            return XUI_OK;
        }
    }
    if (result != XUI_OK) return result;
    first = doc_index_get(s->index, plan->blocks[0]);
    parent = first ? doc_index_get(s->index, first->parent) : NULL;
    plan->index = parent ? doc_child_index(parent, first->id) : DOC_NONE;
    if (!parent || !doc_schema_child(parent->kind, XUI_DOC_LIST) ||
        plan->index == DOC_NONE) return XUI_ERROR_UNSUPPORTED;
    plan->parent = parent->id;
    for (i = 0; i < plan->count; i++) {
        block = doc_index_get(s->index, plan->blocks[i]);
        if (!block || !doc_paragraph(block) || block->parent != plan->parent ||
            doc_child_index(parent, block->id) != plan->index + i)
            return XUI_ERROR_UNSUPPORTED;
    }
    if ((flags & XUI_DOC_ORDERED) && start > UINT64_MAX - (plan->count - 1))
        return XUI_DOC_ERROR_LIMIT;
    return XUI_OK;
}
int doc_list_create_range_can(doc_state* state, const xui_doc_range_t* range,
    uint32_t flags, uint64_t start)
{
    doc_list_create_plan plan; int result;
    if (!state) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_command_list_create_plan(state, range, flags, start, &plan);
    doc_free(plan.blocks); return result;
}
XUI_API int xuiDocumentTxnCreateListRange(xui_document_transaction t,
    const xui_doc_range_t* range, uint32_t flags, uint64_t start,
    xui_doc_node_id* list_id, xui_doc_range_t* after)
{
    doc_list_create_plan plan; xui_doc_node_desc_t desc = {0};
    uint64_t id = 0, item = 0, paragraph = 0, i; int result, order;
    if (list_id) *list_id = 0;
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!range || !list_id || !after) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_range_t target;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnCreateListRange(&shadow, range, flags, start, &id, &target);
        result = doc_markdown_shadow_end_range(t, &shadow, result, &target, after);
        if (result == XUI_OK) *list_id = id;
        return result;
    }
    result = doc_command_list_create_plan(t->draft, range, flags, start, &plan);
    if (result != XUI_OK) { doc_free(plan.blocks); return doc_txn_fail(t, result); }
    result = doc_position_compare(t->draft, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) { doc_free(plan.blocks); return doc_txn_fail(t, result); }
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_LIST;
    desc.tAttributes.iFlags = (flags & XUI_DOC_ORDERED) | XUI_DOC_TIGHT;
    desc.tAttributes.iListStart = start;
    result = doc_txn_insert(t, plan.parent, plan.index, &desc, &id);
    for (i = 0; i < plan.count && result == XUI_OK; i++) {
        memset(&desc, 0, sizeof(desc)); desc.iSize = sizeof(desc);
        desc.iKind = XUI_DOC_LIST_ITEM;
        desc.tAttributes.iFlags = flags & XUI_DOC_TASK;
        result = doc_txn_insert(t, id, DOC_NONE, &desc, &item);
        if (result != XUI_OK) break;
        if (!plan.empty) result = xuiDocumentTxnMoveNode(t, plan.blocks[i], item, DOC_NONE);
        else {
            desc.iKind = XUI_DOC_PARAGRAPH; desc.tAttributes.iFlags = 0;
            result = doc_txn_insert(t, item, DOC_NONE, &desc, &paragraph);
        }
    }
    doc_free(plan.blocks);
    if (result != XUI_OK) return result;
    *list_id = id; *after = *range;
    if (plan.empty)
        after->tAnchor = after->tCaret = doc_command_caret(t, paragraph, 0, XUI_DOC_POSITION_GAP);
    else if (order && range->tAnchor.iKind == XUI_DOC_POSITION_GAP &&
        range->tCaret.iKind == XUI_DOC_POSITION_GAP &&
        range->tAnchor.iNodeId == plan.parent && range->tCaret.iNodeId == plan.parent) {
        after->tAnchor = doc_command_caret(t, id, order < 0 ? 0 : plan.count, XUI_DOC_POSITION_GAP);
        after->tCaret = doc_command_caret(t, id, order < 0 ? plan.count : 0, XUI_DOC_POSITION_GAP);
    }
    return XUI_OK;
}
typedef struct doc_list_edit_plan {
    uint64_t list, parent, list_index, first, last, count;
    int list_gap, reverse;
} doc_list_edit_plan;
static int doc_command_list_edit_plan(doc_state* s, const xui_doc_range_t* range,
    doc_list_edit_plan* plan)
{
    doc_node *a, *b, *list, *parent; int order, result;
    memset(plan, 0, sizeof(*plan));
    if (!s || !range) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_position_compare(s, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    if (order && range->tAnchor.iKind == XUI_DOC_POSITION_GAP &&
        range->tCaret.iKind == XUI_DOC_POSITION_GAP &&
        range->tAnchor.iNodeId == range->tCaret.iNodeId) {
        doc_node* gap_parent = doc_index_get(s->index, range->tAnchor.iNodeId);
        if (gap_parent && gap_parent->kind != XUI_DOC_LIST &&
            doc_schema_child(gap_parent->kind, XUI_DOC_LIST)) {
            uint64_t begin = order < 0 ? range->tAnchor.iOffset : range->tCaret.iOffset;
            uint64_t end = order < 0 ? range->tCaret.iOffset : range->tAnchor.iOffset;
            uint64_t i;
            for (i = begin; i < end; i++) {
                doc_node* child = doc_index_get(s->index,
                    doc_seq_get_id(gap_parent->children, i));
                if (!child || child->kind != XUI_DOC_LIST) break;
            }
            if (i == end) return XUI_ERROR_UNSUPPORTED;
        }
    }
    a = doc_command_list_item(s, &range->tAnchor);
    b = doc_command_list_item(s, &range->tCaret);
    if (range->tAnchor.iKind == XUI_DOC_POSITION_GAP &&
        range->tCaret.iKind == XUI_DOC_POSITION_GAP &&
        range->tAnchor.iNodeId == range->tCaret.iNodeId) {
        list = doc_index_get(s->index, range->tAnchor.iNodeId);
        if (list && list->kind == XUI_DOC_LIST) {
            if (!order) return XUI_ERROR_UNSUPPORTED;
            plan->list_gap = 1;
        }
        else if (a && b && a->parent == b->parent)
            list = doc_index_get(s->index, a->parent);
        else return XUI_ERROR_UNSUPPORTED;
    } else if (a && b && a->parent == b->parent)
        list = doc_index_get(s->index, a->parent);
    else return XUI_ERROR_UNSUPPORTED;
    parent = list ? doc_index_get(s->index, list->parent) : NULL;
    if (!list || list->kind != XUI_DOC_LIST || !parent ||
        !doc_schema_child(parent->kind, XUI_DOC_LIST)) return XUI_ERROR_UNSUPPORTED;
    plan->list_index = doc_child_index(parent, list->id);
    plan->first = plan->list_gap ?
        (order < 0 ? range->tAnchor.iOffset : range->tCaret.iOffset) :
        doc_child_index(list, order <= 0 ? a->id : b->id);
    plan->last = plan->list_gap ?
        (order < 0 ? range->tCaret.iOffset : range->tAnchor.iOffset) - 1 :
        doc_child_index(list, order <= 0 ? b->id : a->id);
    plan->count = doc_seq_size(list->children);
    if (plan->list_index == DOC_NONE || plan->first == DOC_NONE ||
        plan->last == DOC_NONE || plan->first > plan->last)
        return XUI_DOC_ERROR_SCHEMA;
    plan->list = list->id; plan->parent = parent->id; plan->reverse = order > 0;
    return XUI_OK;
}
typedef struct doc_list_cross_groups {
    uint64_t* lists;
    uint64_t count, first_item, last_item;
    uint64_t parent, parent_start, parent_end;
    int reverse, parent_gap;
} doc_list_cross_groups;
static int doc_command_list_cross_groups(doc_state* s, const xui_doc_range_t* range,
    doc_list_cross_groups* groups)
{
    doc_node *a, *b, *first, *last, *first_list, *last_list, *parent, *list;
    uint64_t start, end, i; int order, result;
    memset(groups, 0, sizeof(*groups));
    if (!s || !range) return XUI_ERROR_INVALID_ARGUMENT;
    result = doc_position_compare(s, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    if (!order) return XUI_ERROR_UNSUPPORTED;
    a = doc_command_list_item(s, &range->tAnchor);
    b = doc_command_list_item(s, &range->tCaret);
    if (range->tAnchor.iKind == XUI_DOC_POSITION_GAP &&
        range->tCaret.iKind == XUI_DOC_POSITION_GAP &&
        range->tAnchor.iNodeId == range->tCaret.iNodeId) {
        parent = doc_index_get(s->index, range->tAnchor.iNodeId);
        if (!parent || !doc_schema_child(parent->kind, XUI_DOC_LIST))
            return XUI_ERROR_UNSUPPORTED;
        start = order < 0 ? range->tAnchor.iOffset : range->tCaret.iOffset;
        end = (order < 0 ? range->tCaret.iOffset : range->tAnchor.iOffset) - 1;
        if (start > end || end >= doc_seq_size(parent->children))
            return XUI_ERROR_UNSUPPORTED;
        groups->parent_gap = 1;
        groups->parent = parent->id;
        groups->parent_start = start;
        groups->parent_end = end + 1;
    } else {
        if (!a || !b || a->parent == b->parent) return XUI_ERROR_UNSUPPORTED;
        first = order < 0 ? a : b; last = order < 0 ? b : a;
        first_list = doc_index_get(s->index, first->parent);
        last_list = doc_index_get(s->index, last->parent);
        if (!first_list || !last_list || first_list->kind != XUI_DOC_LIST ||
            last_list->kind != XUI_DOC_LIST || first_list->parent != last_list->parent)
            return XUI_ERROR_UNSUPPORTED;
        parent = doc_index_get(s->index, first_list->parent);
        start = parent ? doc_child_index(parent, first_list->id) : DOC_NONE;
        end = parent ? doc_child_index(parent, last_list->id) : DOC_NONE;
        if (start == DOC_NONE || end == DOC_NONE || start >= end ||
            !doc_schema_child(parent->kind, XUI_DOC_LIST))
            return XUI_ERROR_UNSUPPORTED;
        groups->first_item = doc_child_index(first_list, first->id);
        groups->last_item = doc_child_index(last_list, last->id);
        if (groups->first_item == DOC_NONE || groups->last_item == DOC_NONE)
            return XUI_DOC_ERROR_SCHEMA;
    }
    groups->parent = parent->id;
    groups->count = end - start + 1;
    if (groups->count > SIZE_MAX / sizeof(*groups->lists)) return XUI_DOC_ERROR_LIMIT;
    groups->lists = doc_alloc(s->allocator,
        (size_t)groups->count * sizeof(*groups->lists));
    if (!groups->lists) return XUI_ERROR_OUT_OF_MEMORY;
    for (i = 0; i < groups->count; i++) {
        list = doc_index_get(s->index, doc_seq_get_id(parent->children, start + i));
        if (!list || list->kind != XUI_DOC_LIST || !doc_seq_size(list->children)) {
            doc_free(groups->lists); groups->lists = NULL;
            return XUI_ERROR_UNSUPPORTED;
        }
        groups->lists[i] = list->id;
    }
    if (groups->parent_gap) {
        list = doc_index_get(s->index, groups->lists[groups->count - 1]);
        groups->last_item = doc_seq_size(list->children) - 1;
    }
    groups->reverse = order > 0;
    return XUI_OK;
}
static void doc_command_list_group_range(doc_state* s, const xui_doc_range_t* range,
    const doc_list_cross_groups* groups, uint64_t index, xui_doc_range_t* part)
{
    doc_node* list = doc_index_get(s->index, groups->lists[index]);
    uint64_t first = index ? 0 : groups->first_item;
    uint64_t end = index + 1 == groups->count ?
        groups->last_item + 1 : doc_seq_size(list->children);
    part->tAnchor = part->tCaret = range->tAnchor;
    part->tAnchor.iKind = part->tCaret.iKind = XUI_DOC_POSITION_GAP;
    part->tAnchor.iNodeId = part->tCaret.iNodeId = list->id;
    part->tAnchor.iOffset = first; part->tCaret.iOffset = end;
}
static int doc_command_list_style_validate(doc_state* s, const doc_list_edit_plan* plan,
    uint32_t flags, uint64_t start)
{
    doc_node* list; uint64_t suffix;
    if (flags & ~(XUI_DOC_ORDERED | XUI_DOC_TASK) ||
        ((flags & XUI_DOC_ORDERED) ? !start : !!start)) return XUI_ERROR_INVALID_ARGUMENT;
    if ((flags & XUI_DOC_TASK) && s->profile == XUI_DOCUMENT_MARKDOWN &&
        s->dialect == XUI_MD_COMMONMARK) return XUI_DOC_ERROR_UNREPRESENTABLE;
    if ((flags & XUI_DOC_ORDERED) && start > UINT64_MAX - (plan->last - plan->first))
        return XUI_DOC_ERROR_LIMIT;
    list = doc_index_get(s->index, plan->list);
    suffix = plan->count - plan->last - 1;
    if (suffix && (list->attrs->iFlags & XUI_DOC_ORDERED) &&
        list->attrs->iListStart > UINT64_MAX - plan->last - 1)
        return XUI_DOC_ERROR_LIMIT;
    return XUI_OK;
}
int doc_list_style_range_can(doc_state* s, const xui_doc_range_t* range,
    uint32_t flags, uint64_t start, int* active, int* mixed)
{
    doc_list_edit_plan plan; doc_list_cross_groups groups;
    doc_node* list; uint64_t i, matches = 0;
    int result;
    if (active) *active = 0;
    if (mixed) *mixed = 0;
    result = doc_command_list_edit_plan(s, range, &plan);
    if (result == XUI_ERROR_UNSUPPORTED) {
        int any = 0, all = 1;
        result = doc_command_list_cross_groups(s, range, &groups);
        if (result != XUI_OK) return result;
        for (i = 0; i < groups.count; i++) {
            xui_doc_range_t part; int group_active = 0, group_mixed = 0;
            doc_command_list_group_range(s, range, &groups, i, &part);
            result = doc_list_style_range_can(s, &part, flags, start,
                &group_active, &group_mixed);
            if (result != XUI_OK) break;
            any |= group_active || group_mixed;
            all &= group_active;
        }
        doc_free(groups.lists);
        if (result == XUI_OK) {
            if (active) *active = all;
            if (mixed) *mixed = any && !all;
        }
        return result;
    }
    if (result != XUI_OK) return result;
    result = doc_command_list_style_validate(s, &plan, flags, start);
    if (result != XUI_OK) return result;
    list = doc_index_get(s->index, plan.list);
    for (i = plan.first; i <= plan.last; i++) {
        doc_node* item = doc_index_get(s->index, doc_seq_get_id(list->children, i));
        if (!!(list->attrs->iFlags & XUI_DOC_ORDERED) == !!(flags & XUI_DOC_ORDERED) &&
            !!(item->attrs->iFlags & XUI_DOC_TASK) == !!(flags & XUI_DOC_TASK))
            matches++;
    }
    if (active) *active = matches == plan.last - plan.first + 1;
    if (mixed) *mixed = matches && matches < plan.last - plan.first + 1;
    return XUI_OK;
}
int doc_unlist_range_can(doc_state* s, const xui_doc_range_t* range)
{
    doc_list_edit_plan plan; doc_list_cross_groups groups;
    doc_node *list, *parent, *item, *child;
    uint64_t i, j; int result = doc_command_list_edit_plan(s, range, &plan);
    if (result == XUI_ERROR_UNSUPPORTED) {
        result = doc_command_list_cross_groups(s, range, &groups);
        if (result != XUI_OK) return result;
        for (i = 0; i < groups.count; i++) {
            xui_doc_range_t part;
            doc_command_list_group_range(s, range, &groups, i, &part);
            result = doc_unlist_range_can(s, &part);
            if (result != XUI_OK) break;
        }
        doc_free(groups.lists); return result;
    }
    if (result != XUI_OK) return result;
    list = doc_index_get(s->index, plan.list);
    parent = doc_index_get(s->index, plan.parent);
    if (plan.last + 1 < plan.count && (list->attrs->iFlags & XUI_DOC_ORDERED) &&
        list->attrs->iListStart > UINT64_MAX - plan.last - 1) return XUI_DOC_ERROR_LIMIT;
    for (i = plan.first; i <= plan.last; i++) {
        item = doc_index_get(s->index, doc_seq_get_id(list->children, i));
        if (!doc_seq_size(item->children) && s->profile != XUI_DOCUMENT_MARKDOWN &&
            !doc_schema_child(parent->kind, XUI_DOC_PARAGRAPH))
            return XUI_DOC_ERROR_UNREPRESENTABLE;
        for (j = 0; j < doc_seq_size(item->children); j++) {
            child = doc_index_get(s->index, doc_seq_get_id(item->children, j));
            if (!child || !doc_schema_child(parent->kind, child->kind))
                return XUI_ERROR_UNSUPPORTED;
        }
    }
    return XUI_OK;
}
XUI_API int xuiDocumentTxnSetListStyleRange(xui_document_transaction t,
    const xui_doc_range_t* range, uint32_t flags, uint64_t start,
    xui_doc_range_t* after)
{
    doc_list_edit_plan plan; xui_doc_node_desc_t desc = {0};
    doc_node* list; xui_doc_attributes_t attrs;
    uint64_t target, tail = 0, i, selected, suffix; int result;
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!range || !after || flags & ~(XUI_DOC_ORDERED | XUI_DOC_TASK) ||
        ((flags & XUI_DOC_ORDERED) ? !start : !!start))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_range_t mapped;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnSetListStyleRange(&shadow, range, flags, start, &mapped);
        return doc_markdown_shadow_end_range(t, &shadow, result, &mapped, after);
    }
    result = doc_command_list_edit_plan(t->draft, range, &plan);
    if (result == XUI_ERROR_UNSUPPORTED) {
        doc_list_cross_groups groups; uint64_t group;
        result = doc_command_list_cross_groups(t->draft, range, &groups);
        if (result != XUI_OK) return doc_txn_fail(t, result);
        result = doc_list_style_range_can(t->draft, range, flags, start, NULL, NULL);
        if (result != XUI_OK) { doc_free(groups.lists); return doc_txn_fail(t, result); }
        *after = *range;
        for (group = 0; group < groups.count; group++) {
            xui_doc_range_t part, mapped;
            doc_command_list_group_range(t->draft, range, &groups, group, &part);
            result = xuiDocumentTxnSetListStyleRange(t, &part, flags, start, &mapped);
            if (result != XUI_OK) break;
        }
        doc_free(groups.lists);
        return result == XUI_OK ? XUI_OK : doc_txn_fail(t, result);
    }
    if (result == XUI_OK) result = doc_command_list_style_validate(t->draft, &plan, flags, start);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    list = doc_index_get(t->draft->index, plan.list);
    selected = plan.last - plan.first + 1;
    suffix = plan.count - plan.last - 1;
    target = plan.list;
    if (plan.first) {
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_LIST;
        desc.tAttributes = *list->attrs;
        result = doc_txn_insert(t, plan.parent, plan.list_index + 1, &desc, &target);
        if (result != XUI_OK) return result;
    }
    if (suffix) {
        list = doc_index_get(t->draft->index, plan.list);
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_LIST;
        desc.tAttributes = *list->attrs;
        if (desc.tAttributes.iFlags & XUI_DOC_ORDERED)
            desc.tAttributes.iListStart += plan.last + 1;
        result = doc_txn_insert(t, plan.parent,
            plan.list_index + (plan.first ? 2 : 1), &desc, &tail);
        if (result != XUI_OK) return result;
        for (i = 0; i < suffix; i++) {
            list = doc_index_get(t->draft->index, plan.list);
            result = xuiDocumentTxnMoveNode(t,
                doc_seq_get_id(list->children, plan.last + 1), tail, DOC_NONE);
            if (result != XUI_OK) return result;
        }
    }
    if (plan.first) for (i = 0; i < selected; i++) {
        list = doc_index_get(t->draft->index, plan.list);
        result = xuiDocumentTxnMoveNode(t,
            doc_seq_get_id(list->children, plan.first), target, DOC_NONE);
        if (result != XUI_OK) return result;
    }
    list = doc_index_get(t->draft->index, target);
    attrs = *list->attrs;
    attrs.iFlags = (attrs.iFlags & ~XUI_DOC_ORDERED) | (flags & XUI_DOC_ORDERED);
    attrs.iListStart = start;
    result = xuiDocumentTxnSetAttributes(t, target, &attrs);
    if (result != XUI_OK) return result;
    for (i = 0; i < selected; i++) {
        doc_node* item;
        list = doc_index_get(t->draft->index, target);
        item = doc_index_get(t->draft->index, doc_seq_get_id(list->children, i));
        attrs = *item->attrs;
        if (!(flags & XUI_DOC_TASK)) attrs.iFlags &= ~(XUI_DOC_TASK | XUI_DOC_CHECKED);
        else if (!(attrs.iFlags & XUI_DOC_TASK)) {
            attrs.iFlags |= XUI_DOC_TASK;
            attrs.iFlags &= ~XUI_DOC_CHECKED;
        }
        result = xuiDocumentTxnSetAttributes(t, item->id, &attrs);
        if (result != XUI_OK) return result;
    }
    *after = *range;
    if (plan.list_gap) {
        after->tAnchor = doc_command_caret(t, target,
            plan.reverse ? selected : 0, XUI_DOC_POSITION_GAP);
        after->tCaret = doc_command_caret(t, target,
            plan.reverse ? 0 : selected, XUI_DOC_POSITION_GAP);
    }
    return XUI_OK;
}
XUI_API int xuiDocumentTxnUnlistRange(xui_document_transaction t,
    const xui_doc_range_t* range, xui_doc_range_t* after)
{
    doc_list_edit_plan plan; xui_doc_node_desc_t desc = {0};
    doc_node *list, *item; uint64_t tail = 0, i, insert_at, selected, moved;
    int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (!range || !after) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_range_t mapped;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnUnlistRange(&shadow, range, &mapped);
        return doc_markdown_shadow_end_range(t, &shadow, result, &mapped, after);
    }
    result = doc_command_list_edit_plan(t->draft, range, &plan);
    if (result == XUI_ERROR_UNSUPPORTED) {
        doc_list_cross_groups groups; uint64_t group, first_id, last_id, successor = 0;
        uint64_t next[2] = {0}, previous[2] = {0};
        result = doc_command_list_cross_groups(t->draft, range, &groups);
        if (result != XUI_OK) return doc_txn_fail(t, result);
        result = doc_unlist_range_can(t->draft, range);
        if (result != XUI_OK) { doc_free(groups.lists); return doc_txn_fail(t, result); }
        first_id = doc_seq_get_id(doc_index_get(t->draft->index,
            groups.lists[0])->children, groups.first_item);
        last_id = doc_seq_get_id(doc_index_get(t->draft->index,
            groups.lists[groups.count - 1])->children, groups.last_item);
        if (groups.parent_gap) {
            doc_node* parent = doc_index_get(t->draft->index, groups.parent);
            if (groups.parent_end < doc_seq_size(parent->children))
                successor = doc_seq_get_id(parent->children, groups.parent_end);
        }
        /* Bookmark item boundaries by surviving child IDs before any group
         * moves. Later groups can shift the final parent's gaps, and an
         * endpoint can be inside a multi-block item rather than at its edge. */
        for (i = 0; i < 2 && !groups.parent_gap; i++) {
            const xui_doc_position_t* p = i ? &range->tCaret : &range->tAnchor;
            doc_node* endpoint = doc_index_get(t->draft->index, p->iNodeId);
            uint64_t j;
            if (p->iKind != XUI_DOC_POSITION_GAP || endpoint->kind != XUI_DOC_LIST_ITEM) continue;
            for (j = p->iOffset; j < doc_seq_size(endpoint->children); j++) {
                doc_node* child = doc_index_get(t->draft->index, doc_seq_get_id(endpoint->children, j));
                if (t->draft->profile != XUI_DOCUMENT_MARKDOWN || !doc_semantic_empty_paragraph(t->draft, child)) {
                    next[i] = child->id; break;
                }
            }
            for (j = p->iOffset; j-- > 0;) {
                doc_node* child = doc_index_get(t->draft->index, doc_seq_get_id(endpoint->children, j));
                if (t->draft->profile != XUI_DOCUMENT_MARKDOWN || !doc_semantic_empty_paragraph(t->draft, child)) {
                    previous[i] = child->id; break;
                }
            }
        }
        *after = *range;
        for (group = 0; group < groups.count; group++) {
            xui_doc_range_t part, mapped;
            doc_command_list_group_range(t->draft, range, &groups, group, &part);
            result = xuiDocumentTxnUnlistRange(t, &part, &mapped);
            if (result != XUI_OK) break;
            if (!groups.reverse && group == 0 &&
                range->tAnchor.iKind == XUI_DOC_POSITION_GAP &&
                range->tAnchor.iNodeId == first_id) after->tAnchor = mapped.tAnchor;
            if (groups.reverse && group == 0 &&
                range->tCaret.iKind == XUI_DOC_POSITION_GAP &&
                range->tCaret.iNodeId == first_id) after->tCaret = mapped.tAnchor;
            if (!groups.reverse && group + 1 == groups.count &&
                range->tCaret.iKind == XUI_DOC_POSITION_GAP &&
                range->tCaret.iNodeId == last_id)
                after->tCaret = mapped.tCaret;
            if (groups.reverse && group + 1 == groups.count &&
                range->tAnchor.iKind == XUI_DOC_POSITION_GAP &&
                range->tAnchor.iNodeId == last_id)
                after->tAnchor = mapped.tCaret;
        }
        if (result == XUI_OK && groups.parent_gap) {
            doc_node* parent = doc_index_get(t->draft->index, groups.parent);
            uint64_t end = successor ? doc_child_index(parent, successor) :
                doc_seq_size(parent->children);
            if (end == DOC_NONE || end < groups.parent_start)
                result = XUI_DOC_ERROR_SCHEMA;
            else {
                after->tAnchor = doc_command_caret(t, groups.parent,
                    groups.reverse ? end : groups.parent_start, XUI_DOC_POSITION_GAP);
                after->tCaret = doc_command_caret(t, groups.parent,
                    groups.reverse ? groups.parent_start : end, XUI_DOC_POSITION_GAP);
            }
        }
        if (result == XUI_OK) for (i = 0; i < 2; i++) if (next[i] || previous[i]) {
            xui_doc_position_t* p = i ? &after->tCaret : &after->tAnchor;
            doc_node* parent = doc_index_get(t->draft->index, groups.parent);
            uint64_t at = doc_child_index(parent, next[i] ? next[i] : previous[i]);
            if (at == DOC_NONE) { result = XUI_DOC_ERROR_SCHEMA; break; }
            *p = i ? range->tCaret : range->tAnchor;
            p->iNodeId = groups.parent;
            p->iOffset = at + !next[i];
        }
        after->tAnchor.iAffinity = range->tAnchor.iAffinity;
        after->tCaret.iAffinity = range->tCaret.iAffinity;
        doc_free(groups.lists);
        return result == XUI_OK ? XUI_OK : doc_txn_fail(t, result);
    }
    if (result == XUI_OK) result = doc_unlist_range_can(t->draft, range);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    list = doc_index_get(t->draft->index, plan.list);
    selected = plan.last - plan.first + 1;
    *after = *range;
    if (plan.last + 1 < plan.count) {
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_LIST;
        desc.tAttributes = *list->attrs;
        if (desc.tAttributes.iFlags & XUI_DOC_ORDERED)
            desc.tAttributes.iListStart += plan.last + 1;
        result = doc_txn_insert(t, plan.parent, plan.list_index + 1, &desc, &tail);
        if (result != XUI_OK) return result;
        for (i = plan.last + 1; i < plan.count; i++) {
            list = doc_index_get(t->draft->index, plan.list);
            result = xuiDocumentTxnMoveNode(t,
                doc_seq_get_id(list->children, plan.last + 1), tail, DOC_NONE);
            if (result != XUI_OK) return result;
        }
    }
    insert_at = plan.list_index + 1;
    for (i = 0; i < selected; i++) {
        uint64_t item_id, child_index = 0, empty_id;
        uint64_t anchor_offset = insert_at, caret_offset = insert_at;
        list = doc_index_get(t->draft->index, plan.list);
        item_id = doc_seq_get_id(list->children, plan.first);
        item = doc_index_get(t->draft->index, item_id);
        if (!doc_seq_size(item->children) &&
            t->draft->profile != XUI_DOCUMENT_MARKDOWN) {
            memset(&desc, 0, sizeof(desc)); desc.iSize = sizeof(desc);
            desc.iKind = XUI_DOC_PARAGRAPH;
            result = doc_txn_insert(t, plan.parent, insert_at++, &desc, &empty_id);
            if (result != XUI_OK) return result;
        }
        while ((item = doc_index_get(t->draft->index, item_id)) &&
            doc_seq_size(item->children)) {
            uint64_t child = doc_seq_get_id(item->children, 0);
            /* Empty Markdown paragraphs vanish on reparse; do not count them
             * as promoted blocks in the returned structural selection. */
            if (t->draft->profile == XUI_DOCUMENT_MARKDOWN &&
                doc_semantic_empty_paragraph(t->draft,
                    doc_index_get(t->draft->index, child))) {
                result = doc_txn_delete(t, child);
                if (result != XUI_OK) return result;
                child_index++;
                continue;
            }
            if (after->tAnchor.iNodeId == item_id && child_index < after->tAnchor.iOffset)
                anchor_offset++;
            if (after->tCaret.iNodeId == item_id && child_index < after->tCaret.iOffset)
                caret_offset++;
            child_index++;
            result = xuiDocumentTxnMoveNode(t, child, plan.parent, insert_at++);
            if (result != XUI_OK) return result;
        }
        /* Item gaps describe boundaries between its blocks, not gaps inside
         * the first promoted block (which can be a text-bearing CodeBlock).
         * Count only surviving children and account for removal of the head
         * wrapper. Keep endpoint affinity and direction from the input. */
        if (after->tAnchor.iNodeId == item_id) {
            after->tAnchor.iNodeId = plan.parent;
            after->tAnchor.iOffset = anchor_offset - !plan.first;
        }
        if (after->tCaret.iNodeId == item_id) {
            after->tCaret.iNodeId = plan.parent;
            after->tCaret.iOffset = caret_offset - !plan.first;
        }
        result = doc_txn_delete(t, item_id);
        if (result != XUI_OK) return result;
    }
    moved = insert_at - plan.list_index - 1;
    if (!plan.first) {
        result = doc_txn_delete(t, plan.list);
        if (result != XUI_OK) return result;
    }
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && moved) {
        doc_node* parent = doc_index_get(t->draft->index, plan.parent);
        if (parent && parent->kind == XUI_DOC_LIST_ITEM && doc_seq_size(parent->children) > 1) {
            doc_node* outer = doc_index_get(t->draft->index, parent->parent);
            xui_doc_attributes_t attrs;
            if (!outer || outer->kind != XUI_DOC_LIST) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
            attrs = *outer->attrs; attrs.iFlags &= ~XUI_DOC_TIGHT;
            result = xuiDocumentTxnSetAttributes(t, outer->id, &attrs);
            if (result != XUI_OK) return result;
        }
    }
    if (plan.list_gap) {
        uint64_t start = plan.list_index + !!plan.first;
        after->tAnchor = doc_command_caret(t, plan.parent,
            plan.reverse ? start + moved : start, XUI_DOC_POSITION_GAP);
        after->tCaret = doc_command_caret(t, plan.parent,
            plan.reverse ? start : start + moved, XUI_DOC_POSITION_GAP);
    }
    after->tAnchor.iAffinity = range->tAnchor.iAffinity;
    after->tCaret.iAffinity = range->tCaret.iAffinity;
    return XUI_OK;
}
typedef struct doc_move_block_plan {
    uint64_t parent, first, last, neighbor;
    int reverse, parent_gap;
} doc_move_block_plan;
static int doc_move_block_parent(uint32_t kind)
{
    return kind == XUI_DOC_ROOT || kind == XUI_DOC_QUOTE ||
        kind == XUI_DOC_LIST || kind == XUI_DOC_LIST_ITEM ||
        kind == XUI_DOC_CELL || kind == XUI_DOC_FOOTNOTE;
}
static doc_node* doc_move_block_target(doc_state* s, const xui_doc_position_t* at)
{
    doc_node *n, *cur, *parent;
    if (at->iKind == XUI_DOC_POSITION_SOURCE) return NULL;
    n = doc_index_get(s->index, at->iNodeId);
    if (!n || (at->iKind == XUI_DOC_POSITION_GAP &&
        (n->kind == XUI_DOC_ROOT || n->kind == XUI_DOC_QUOTE ||
         n->kind == XUI_DOC_LIST || n->kind == XUI_DOC_CELL ||
         n->kind == XUI_DOC_FOOTNOTE))) return NULL;
    for (cur = n; cur; cur = parent) {
        parent = doc_index_get(s->index, cur->parent);
        if (parent && parent->kind == XUI_DOC_LIST_ITEM) {
            cur = parent;
            parent = doc_index_get(s->index, cur->parent);
        }
        if (parent && doc_move_block_parent(parent->kind)) return cur;
    }
    return NULL;
}
static int doc_move_block_plan_make(doc_state* s, const xui_doc_range_t* range,
    int down, doc_move_block_plan* plan)
{
    doc_node *a, *b, *parent; uint64_t children;
    int order, result;
    memset(plan, 0, sizeof(*plan));
    if (!s || !range || (down != 0 && down != 1)) return XUI_ERROR_INVALID_ARGUMENT;
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE ||
        range->tCaret.iKind == XUI_DOC_POSITION_SOURCE) return XUI_DOC_ERROR_DOMAIN;
    result = doc_position_compare(s, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    plan->reverse = order > 0;
    if (order && range->tAnchor.iKind == XUI_DOC_POSITION_GAP &&
        range->tCaret.iKind == XUI_DOC_POSITION_GAP &&
        range->tAnchor.iNodeId == range->tCaret.iNodeId &&
        (parent = doc_index_get(s->index, range->tAnchor.iNodeId)) != NULL &&
        doc_move_block_parent(parent->kind)) {
        plan->parent_gap = 1;
        plan->parent = parent->id;
        plan->first = order < 0 ? range->tAnchor.iOffset : range->tCaret.iOffset;
        plan->last = (order < 0 ? range->tCaret.iOffset : range->tAnchor.iOffset) - 1;
    } else {
        a = doc_move_block_target(s, &range->tAnchor);
        b = doc_move_block_target(s, &range->tCaret);
        if (!a || !b || a->parent != b->parent) return XUI_ERROR_UNSUPPORTED;
        parent = doc_index_get(s->index, a->parent);
        if (!parent || !doc_move_block_parent(parent->kind)) return XUI_ERROR_UNSUPPORTED;
        plan->parent = parent->id;
        plan->first = doc_child_index(parent, order <= 0 ? a->id : b->id);
        plan->last = doc_child_index(parent, order <= 0 ? b->id : a->id);
    }
    parent = doc_index_get(s->index, plan->parent);
    children = doc_seq_size(parent->children);
    if (plan->first == DOC_NONE || plan->last == DOC_NONE ||
        plan->first > plan->last || plan->last >= children) return XUI_ERROR_UNSUPPORTED;
    if ((!down && !plan->first) || (down && plan->last + 1 >= children))
        return XUI_ERROR_UNSUPPORTED;
    plan->neighbor = doc_seq_get_id(parent->children,
        down ? plan->last + 1 : plan->first - 1);
    return XUI_OK;
}
int doc_move_block_range_can(doc_state* s, const xui_doc_range_t* range, int down)
{
    doc_move_block_plan plan;
    return doc_move_block_plan_make(s, range, down, &plan);
}
XUI_API int xuiDocumentTxnMoveBlockRange(xui_document_transaction t,
    const xui_doc_range_t* range, int down, xui_doc_range_t* after)
{
    doc_move_block_plan plan; int result;
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!range || !after || (down != 0 && down != 1))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_range_t mapped;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnMoveBlockRange(&shadow, range, down, &mapped);
        return doc_markdown_shadow_end_range(t, &shadow, result, &mapped, after);
    }
    result = doc_move_block_plan_make(t->draft, range, down, &plan);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    result = xuiDocumentTxnMoveNode(t, plan.neighbor, plan.parent,
        down ? plan.first : plan.last + 1);
    if (result != XUI_OK) return result;
    *after = *range;
    if (plan.parent_gap) {
        uint64_t first = down ? plan.first + 1 : plan.first - 1;
        uint64_t end = plan.last + (down ? 2 : 0);
        after->tAnchor = doc_command_caret(t, plan.parent,
            plan.reverse ? end : first, XUI_DOC_POSITION_GAP);
        after->tCaret = doc_command_caret(t, plan.parent,
            plan.reverse ? first : end, XUI_DOC_POSITION_GAP);
    }
    return XUI_OK;
}
static int doc_quote_child_supported(const doc_state* state, const doc_node* node)
{
    if (!doc_schema_child(XUI_DOC_QUOTE, node->kind)) return XUI_ERROR_UNSUPPORTED;
    if (state->profile != XUI_DOCUMENT_MARKDOWN) return XUI_OK;
    /* These are the block forms that can reparse at quote depth. Front matter
     * is root-only; footnote definitions are hoisted; the remaining schema
     * kinds have no standalone Markdown block representation. The adapter
     * still checks the exact candidate source before publication. */
    switch (node->kind) {
    case XUI_DOC_PARAGRAPH: case XUI_DOC_HEADING: case XUI_DOC_QUOTE:
    case XUI_DOC_LIST: case XUI_DOC_CODE_BLOCK: case XUI_DOC_RULE:
    case XUI_DOC_HTML: return XUI_OK;
    case XUI_DOC_TABLE:
        return state->dialect == XUI_MD_COMMONMARK ?
            XUI_DOC_ERROR_UNREPRESENTABLE : XUI_OK;
    case XUI_DOC_DIAGRAM:
        return state->dialect == XUI_MD_EXTENDED ?
            XUI_OK : XUI_DOC_ERROR_UNREPRESENTABLE;
    default: return XUI_DOC_ERROR_UNREPRESENTABLE;
    }
}
static int doc_quote_edge_gap(doc_state* state, const xui_doc_position_t* at,
    int end, uint64_t* parent_id, uint64_t* offset)
{
    doc_node *node = doc_index_get(state->index, at->iNodeId), *parent;
    uint64_t index;
    if (at->iKind == XUI_DOC_POSITION_GAP && node && !doc_paragraph(node)) {
        *parent_id = node->id; *offset = at->iOffset; return XUI_OK;
    }
    node = doc_command_block(state, at);
    parent = node ? doc_index_get(state->index, node->parent) : NULL;
    index = parent ? doc_child_index(parent, node->id) : DOC_NONE;
    if (index == DOC_NONE) return XUI_ERROR_UNSUPPORTED;
    *parent_id = parent->id; *offset = index + (uint64_t)end;
    return XUI_OK;
}
static unsigned doc_quote_parent_depth(doc_state* state, uint64_t id)
{
    doc_node* node = doc_index_get(state->index, id);
    unsigned depth = 0;
    while (node && depth < DOC_MAX_DEPTH) {
        depth++;
        if (node->id == DOC_ROOT) return depth;
        node = doc_index_get(state->index, node->parent);
    }
    return 0;
}
static int doc_quote_promote_gap(doc_state* state, uint64_t* parent_id,
    uint64_t* offset, int end)
{
    doc_node* node = doc_index_get(state->index, *parent_id);
    doc_node* parent = node && node->parent ?
        doc_index_get(state->index, node->parent) : NULL;
    uint64_t index = parent ? doc_child_index(parent, node->id) : DOC_NONE;
    uint64_t children = node ? doc_seq_size(node->children) : 0;
    if (index == DOC_NONE || (*offset && *offset != children))
        return XUI_ERROR_UNSUPPORTED;
    *parent_id = parent->id;
    *offset = index + (uint64_t)(children ? *offset == children : end);
    return XUI_OK;
}
int doc_quote_range_ids(doc_state* state, const xui_doc_range_t* range,
    uint64_t** ids, uint64_t* count)
{
    doc_node *parent, *node;
    const xui_doc_position_t *left, *right;
    uint64_t start_parent, end_parent, start, end, i;
    unsigned start_depth, end_depth;
    int order, result;
    if (!state || !range || !ids || !count) return XUI_ERROR_INVALID_ARGUMENT;
    *ids = NULL; *count = 0;
    result = doc_position_compare(state, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    left = order <= 0 ? &range->tAnchor : &range->tCaret;
    right = order <= 0 ? &range->tCaret : &range->tAnchor;
    result = doc_quote_edge_gap(state, left, 0, &start_parent, &start);
    if (result == XUI_OK)
        result = doc_quote_edge_gap(state, right, 1, &end_parent, &end);
    if (result != XUI_OK) return result;
    while (start_parent != end_parent) {
        start_depth = doc_quote_parent_depth(state, start_parent);
        end_depth = doc_quote_parent_depth(state, end_parent);
        if (!start_depth || !end_depth) return XUI_ERROR_UNSUPPORTED;
        if (start_depth >= end_depth) {
            result = doc_quote_promote_gap(state, &start_parent, &start, 0);
            if (result != XUI_OK) return result;
        }
        if (end_depth >= start_depth) {
            result = doc_quote_promote_gap(state, &end_parent, &end, 1);
            if (result != XUI_OK) return result;
        }
    }
    parent = doc_index_get(state->index, start_parent);
    while (parent && !doc_schema_child(parent->kind, XUI_DOC_QUOTE)) {
        if (start || end != doc_seq_size(parent->children)) return XUI_ERROR_UNSUPPORTED;
        result = doc_quote_promote_gap(state, &start_parent, &start, 0);
        if (result == XUI_OK)
            result = doc_quote_promote_gap(state, &end_parent, &end, 1);
        if (result != XUI_OK) return result;
        parent = doc_index_get(state->index, start_parent);
    }
    if (!parent || start >= end || end > doc_seq_size(parent->children))
        return XUI_ERROR_UNSUPPORTED;
    if (state->profile == XUI_DOCUMENT_MARKDOWN) {
        /* A task marker is parsed through the item's opening paragraph.
         * Moving that first child under a Quote leaves an extra paragraph
         * in every Markdown spelling, so the requested tree cannot reload. */
        if (parent->kind == XUI_DOC_LIST_ITEM && !start &&
            (parent->attrs->iFlags & XUI_DOC_TASK))
            return XUI_DOC_ERROR_UNREPRESENTABLE;
        for (node = parent; node; node = doc_index_get(state->index, node->parent))
            if (node->kind == XUI_DOC_TABLE) return XUI_DOC_ERROR_UNREPRESENTABLE;
    }
    if (end - start > SIZE_MAX / sizeof(**ids)) return XUI_DOC_ERROR_LIMIT;
    *count = end - start;
    *ids = doc_alloc(state->allocator, (size_t)*count * sizeof(**ids));
    if (!*ids) { *count = 0; return XUI_ERROR_OUT_OF_MEMORY; }
    for (i = 0; i < *count; i++) (*ids)[i] = doc_seq_get_id(parent->children, start + i);
    result = XUI_OK;
    for (i = 0; i < *count && result == XUI_OK; i++) {
        node = doc_index_get(state->index, (*ids)[i]);
        if (!node || node->parent != parent->id ||
            doc_child_index(parent, node->id) != start + i)
            result = XUI_ERROR_UNSUPPORTED;
        else result = doc_quote_child_supported(state, node);
    }
    if (result != XUI_OK) { doc_free(*ids); *ids = NULL; *count = 0; }
    return result;
}
/* Keep the child on one side of each boundary, rather than a container
 * offset: splitting the first edge can move the second edge into a clone. */
typedef struct doc_quote_edge_anchor {
    uint64_t child, empty_parent;
    int after, end;
} doc_quote_edge_anchor;
typedef struct doc_quote_split_plan {
    uint64_t parent;
    doc_quote_edge_anchor left, right;
    uint64_t left_item_lists[DOC_MAX_DEPTH];
    unsigned left_item_count;
} doc_quote_split_plan;
static int doc_quote_anchor_get(doc_state* state, const xui_doc_position_t* at,
    int end, doc_quote_edge_anchor* anchor, uint64_t* parent_id, uint64_t* offset)
{
    doc_node* parent; uint64_t count;
    int result = doc_quote_edge_gap(state, at, end, parent_id, offset);
    if (result != XUI_OK) return result;
    parent = doc_index_get(state->index, *parent_id);
    count = parent ? doc_seq_size(parent->children) : 0;
    if (!parent || *offset > count) return XUI_DOC_ERROR_SCHEMA;
    memset(anchor, 0, sizeof(*anchor)); anchor->end = end;
    if (!count) anchor->empty_parent = parent->id;
    else {
        anchor->after = *offset == count;
        anchor->child = doc_seq_get_id(parent->children,
            anchor->after ? count - 1 : *offset);
    }
    return XUI_OK;
}
static int doc_quote_anchor_resolve(doc_state* state,
    const doc_quote_edge_anchor* anchor, uint64_t* parent_id, uint64_t* offset)
{
    doc_node *child, *parent; uint64_t index;
    if (anchor->empty_parent) {
        parent = doc_index_get(state->index, anchor->empty_parent);
        if (!parent || doc_seq_size(parent->children)) return XUI_DOC_ERROR_SCHEMA;
        *parent_id = parent->id; *offset = 0; return XUI_OK;
    }
    child = doc_index_get(state->index, anchor->child);
    parent = child ? doc_index_get(state->index, child->parent) : NULL;
    index = parent ? doc_child_index(parent, child->id) : DOC_NONE;
    if (index == DOC_NONE) return XUI_DOC_ERROR_SCHEMA;
    *parent_id = parent->id; *offset = index + (uint64_t)anchor->after;
    return XUI_OK;
}
static int doc_quote_container_splittable(const doc_node* node)
{
    return node && (node->kind == XUI_DOC_QUOTE || node->kind == XUI_DOC_LIST ||
        node->kind == XUI_DOC_LIST_ITEM);
}
/* Dry-run the same upward walk as execution. Once a child is partially
 * selected, every enclosing container below the target must also split.
 * In the original tree that boundary overlaps the child; in the draft it
 * lies between its prefix and suffix clones. */
static int doc_quote_edge_plan(doc_state* state, uint64_t current, uint64_t edge,
    int end, uint64_t target, doc_quote_split_plan* plan, uint64_t* target_offset)
{
    unsigned depth; int partial = 0;
    for (depth = 0; depth < DOC_MAX_DEPTH; depth++) {
        doc_node *node = doc_index_get(state->index, current), *parent;
        uint64_t count, index;
        if (!node) return XUI_DOC_ERROR_SCHEMA;
        count = doc_seq_size(node->children);
        if (edge > count) return XUI_DOC_ERROR_SCHEMA;
        if (current == target) {
            *target_offset = partial && !end ? edge - 1 : edge;
            return XUI_OK;
        }
        if (partial || (edge && edge < count)) {
            if (!doc_quote_container_splittable(node)) return XUI_ERROR_UNSUPPORTED;
            if (node->kind == XUI_DOC_LIST &&
                (node->attrs->iFlags & XUI_DOC_ORDERED)) {
                uint64_t added = 0; unsigned k;
                if (end) for (k = 0; k < plan->left_item_count; k++)
                    if (plan->left_item_lists[k] == node->id) added++;
                if (edge > UINT64_MAX - added ||
                    node->attrs->iListStart > UINT64_MAX - edge - added)
                    return XUI_DOC_ERROR_LIMIT;
            }
            if (!end && node->kind == XUI_DOC_LIST_ITEM) {
                if (plan->left_item_count >= DOC_MAX_DEPTH) return XUI_DOC_ERROR_LIMIT;
                plan->left_item_lists[plan->left_item_count++] = node->parent;
            }
            partial = 1;
        }
        parent = doc_index_get(state->index, node->parent);
        index = parent ? doc_child_index(parent, node->id) : DOC_NONE;
        if (index == DOC_NONE) return XUI_ERROR_UNSUPPORTED;
        current = parent->id;
        edge = index + (uint64_t)(partial || (count ? edge == count : end));
    }
    return XUI_ERROR_UNSUPPORTED;
}
static int doc_quote_split_plan_get(doc_state* state,
    const xui_doc_range_t* range, doc_quote_split_plan* plan)
{
    const xui_doc_position_t *left, *right;
    doc_node *parent, *node;
    uint64_t start_parent, end_parent, start, end, a, b, i;
    unsigned da, db; int order, result;
    memset(plan, 0, sizeof(*plan));
    result = doc_position_compare(state, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return result;
    left = order <= 0 ? &range->tAnchor : &range->tCaret;
    right = order <= 0 ? &range->tCaret : &range->tAnchor;
    result = doc_quote_anchor_get(state, left, 0, &plan->left, &start_parent, &start);
    if (result == XUI_OK)
        result = doc_quote_anchor_get(state, right, 1, &plan->right, &end_parent, &end);
    if (result != XUI_OK) return result;
    a = start_parent; b = end_parent;
    da = doc_quote_parent_depth(state, a); db = doc_quote_parent_depth(state, b);
    if (!da || !db) return XUI_DOC_ERROR_SCHEMA;
    while (a != b) {
        if (da >= db) { node = doc_index_get(state->index, a); a = node->parent; da--; }
        if (db > da) { node = doc_index_get(state->index, b); b = node->parent; db--; }
        if (!a || !b) return XUI_DOC_ERROR_SCHEMA;
    }
    parent = doc_index_get(state->index, a);
    while (parent && !doc_schema_child(parent->kind, XUI_DOC_QUOTE))
        parent = doc_index_get(state->index, parent->parent);
    if (!parent) return XUI_ERROR_UNSUPPORTED;
    plan->parent = parent->id;
    result = doc_quote_edge_plan(state, start_parent, start, 0, parent->id, plan, &start);
    if (result == XUI_OK)
        result = doc_quote_edge_plan(state, end_parent, end, 1, parent->id, plan, &end);
    if (result != XUI_OK) return result;
    if (start >= end || end > doc_seq_size(parent->children)) return XUI_ERROR_UNSUPPORTED;
    if (state->profile == XUI_DOCUMENT_MARKDOWN) {
        if (parent->kind == XUI_DOC_LIST_ITEM && !start &&
            (parent->attrs->iFlags & XUI_DOC_TASK)) return XUI_DOC_ERROR_UNREPRESENTABLE;
        for (node = parent; node; node = doc_index_get(state->index, node->parent))
            if (node->kind == XUI_DOC_TABLE) return XUI_DOC_ERROR_UNREPRESENTABLE;
    }
    for (i = start; i < end; i++) {
        node = doc_index_get(state->index, doc_seq_get_id(parent->children, i));
        if (!node) return XUI_DOC_ERROR_SCHEMA;
        result = doc_quote_child_supported(state, node);
        if (result != XUI_OK) return result;
    }
    return XUI_OK;
}
int doc_quote_range_can(doc_state* state, const xui_doc_range_t* range)
{
    doc_quote_split_plan plan; uint64_t *ids = NULL, count = 0;
    int result = doc_quote_range_ids(state, range, &ids, &count);
    doc_free(ids);
    return result == XUI_ERROR_UNSUPPORTED ?
        doc_quote_split_plan_get(state, range, &plan) : result;
}
/* A split can remove the only item separator that made the original list
 * loose. Inspect the retained raw lines between simple items before copying
 * that list's tightness into the desired Markdown tree. */
static int doc_quote_items_have_blank_line(doc_state* state,
    uint64_t start, uint64_t end, int* blank)
{
    char buffer[256]; uint64_t at;
    int line_seen = 0, content = 0, cr = 0;
    *blank = 0;
    if (start == DOC_NONE || end == DOC_NONE || start >= end ||
        end > doc_seq_size(state->source)) return XUI_ERROR_UNSUPPORTED;
    for (at = start; at < end;) {
        uint64_t n = end - at, i;
        int result;
        if (n > sizeof(buffer)) n = sizeof(buffer);
        result = doc_seq_read(state->source, at, buffer, n);
        if (result != XUI_OK) return result;
        for (i = 0; i < n; i++) {
            char c = buffer[i];
            if (c == '\n' && cr) { cr = 0; continue; }
            cr = c == '\r';
            if (c == '\n' || c == '\r') {
                if (line_seen && !content) { *blank = 1; return XUI_OK; }
                line_seen = 1; content = 0;
            } else if (c != ' ' && c != '\t' && c != '>') content = 1;
        }
        at += n;
    }
    return XUI_OK;
}
static int doc_quote_normalize_simple_list(xui_document_transaction t, uint64_t id)
{
    doc_node *list, *item, *block, *previous = NULL;
    doc_node_source_range left, right;
    uint64_t i, count;
    xui_doc_attributes_t attrs;
    if (t->draft->profile != XUI_DOCUMENT_MARKDOWN) return XUI_OK;
    list = doc_index_get(t->draft->index, id);
    if (!list || list->kind != XUI_DOC_LIST ||
        (list->attrs->iFlags & XUI_DOC_TIGHT)) return XUI_OK;
    count = doc_seq_size(list->children);
    if (!count) return XUI_OK;
    for (i = 0; i < count; i++) {
        int blank, result;
        item = doc_index_get(t->draft->index, doc_seq_get_id(list->children, i));
        if (!item || item->kind != XUI_DOC_LIST_ITEM ||
            doc_seq_size(item->children) != 1) return XUI_OK;
        block = doc_index_get(t->draft->index, doc_seq_get_id(item->children, 0));
        if (!block || block->kind != XUI_DOC_PARAGRAPH) return XUI_OK;
        if (previous) {
            doc_node_source_range_get(t->draft, previous, &left);
            doc_node_source_range_get(t->draft, item, &right);
            result = doc_quote_items_have_blank_line(t->draft,
                left.syntax_start, right.syntax_start, &blank);
            if (result == XUI_ERROR_UNSUPPORTED || blank) return XUI_OK;
            if (result != XUI_OK) return result;
        }
        previous = item;
    }
    attrs = *list->attrs; attrs.iFlags |= XUI_DOC_TIGHT;
    return xuiDocumentTxnSetAttributes(t, id, &attrs);
}
static int doc_quote_split_container(xui_document_transaction t, uint64_t container_id,
    uint64_t offset, int selected_prefix, uint64_t* selected_id)
{
    doc_node* container = doc_index_get(t->draft->index, container_id);
    doc_node* parent = container ? doc_index_get(t->draft->index, container->parent) : NULL;
    xui_doc_node_desc_t desc = {0}; uint64_t index, id = 0, i, child;
    xui_doc_attributes_t attrs;
    int result;
    index = parent ? doc_child_index(parent, container_id) : DOC_NONE;
    if (!container || (container->kind != XUI_DOC_QUOTE &&
        container->kind != XUI_DOC_LIST && container->kind != XUI_DOC_LIST_ITEM) || index == DOC_NONE ||
        !offset || offset >= doc_seq_size(container->children))
        return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    desc.iSize = sizeof(desc); desc.iKind = container->kind;
    desc.tAttributes = *container->attrs; desc.sInfo = doc_string(container->info);
    desc.sResource = doc_string(container->resource); desc.sTitle = doc_string(container->title);
    if (container->kind == XUI_DOC_LIST_ITEM && !selected_prefix)
        desc.tAttributes.iFlags &= ~(XUI_DOC_TASK | XUI_DOC_CHECKED);
    if (container->kind == XUI_DOC_LIST &&
        (container->attrs->iFlags & XUI_DOC_ORDERED)) {
        if (container->attrs->iListStart > UINT64_MAX - offset)
            return doc_txn_fail(t, XUI_DOC_ERROR_LIMIT);
        if (!selected_prefix) desc.tAttributes.iListStart += offset;
    }
    result = doc_txn_insert(t, parent->id, index + (uint64_t)!selected_prefix,
        &desc, &id);
    if (result != XUI_OK) return result;
    if (selected_prefix) {
        for (i = 0; i < offset; i++) {
            container = doc_index_get(t->draft->index, container_id);
            child = doc_seq_get_id(container->children, 0);
            result = xuiDocumentTxnMoveNode(t, child, id, DOC_NONE);
            if (result != XUI_OK) return result;
        }
        container = doc_index_get(t->draft->index, container_id);
        if (container->kind == XUI_DOC_LIST &&
            (container->attrs->iFlags & XUI_DOC_ORDERED)) {
            attrs = *container->attrs; attrs.iListStart += offset;
            result = xuiDocumentTxnSetAttributes(t, container_id, &attrs);
            if (result != XUI_OK) return result;
        } else if (container->kind == XUI_DOC_LIST_ITEM &&
            (container->attrs->iFlags & XUI_DOC_TASK)) {
            attrs = *container->attrs; attrs.iFlags &= ~(XUI_DOC_TASK | XUI_DOC_CHECKED);
            result = xuiDocumentTxnSetAttributes(t, container_id, &attrs);
            if (result != XUI_OK) return result;
        }
    } else {
        while (offset < doc_seq_size((container = doc_index_get(t->draft->index,
            container_id))->children)) {
            child = doc_seq_get_id(container->children, offset);
            result = xuiDocumentTxnMoveNode(t, child, id, DOC_NONE);
            if (result != XUI_OK) return result;
        }
    }
    result = doc_quote_normalize_simple_list(t, container_id);
    if (result == XUI_OK) result = doc_quote_normalize_simple_list(t, id);
    if (result != XUI_OK) return result;
    *selected_id = id;
    return XUI_OK;
}
static int doc_quote_edge_normalize(xui_document_transaction t,
    const doc_quote_edge_anchor* anchor, uint64_t target, int allow_split,
    uint64_t* target_offset)
{
    uint64_t current, edge; unsigned depth; int result;
    result = doc_quote_anchor_resolve(t->draft, anchor, &current, &edge);
    if (result != XUI_OK) return result;
    for (depth = 0; depth < DOC_MAX_DEPTH; depth++) {
        doc_node *node = doc_index_get(t->draft->index, current), *parent;
        uint64_t count, selected, index;
        if (!node) return XUI_DOC_ERROR_SCHEMA;
        if (current == target) { *target_offset = edge; return XUI_OK; }
        count = doc_seq_size(node->children);
        if (edge && edge < count) {
            if (!allow_split) return XUI_DOC_ERROR_SCHEMA;
            result = doc_quote_split_container(t, current, edge, anchor->end, &selected);
            if (result != XUI_OK) return result;
            node = doc_index_get(t->draft->index, selected);
            parent = node ? doc_index_get(t->draft->index, node->parent) : NULL;
            index = parent ? doc_child_index(parent, selected) : DOC_NONE;
            if (index == DOC_NONE) return XUI_DOC_ERROR_SCHEMA;
            current = parent->id; edge = index + (uint64_t)anchor->end;
        } else {
            result = doc_quote_promote_gap(t->draft, &current, &edge, anchor->end);
            if (result != XUI_OK) return result;
        }
    }
    return XUI_ERROR_UNSUPPORTED;
}
XUI_API int xuiDocumentTxnWrapQuoteRange(xui_document_transaction t,
    const xui_doc_range_t* range, xui_doc_node_id* quote_id,
    xui_doc_range_t* after)
{
    xui_doc_node_desc_t desc = {0}; doc_node *first, *parent, *item;
    doc_quote_split_plan split_plan;
    xui_doc_range_t normalized;
    uint64_t *ids = NULL, count = 0, index, parent_id, id = 0, i;
    uint64_t first_index, end_index;
    int order, result, split = 0;
    if (quote_id) *quote_id = 0;
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!range || !quote_id || !after) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result == XUI_OK) result = doc_position_compare(t->draft,
        &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (range->tAnchor.iKind == XUI_DOC_POSITION_SOURCE ||
        range->tCaret.iKind == XUI_DOC_POSITION_SOURCE)
        return doc_txn_fail(t, XUI_DOC_ERROR_DOMAIN);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_range_t target;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnWrapQuoteRange(&shadow, range, &id, &target);
        result = doc_markdown_shadow_end_quote_range(t, &shadow, result, id, &target, after);
        if (result == XUI_OK) *quote_id = id;
        return result;
    }
    result = doc_quote_range_ids(t->draft, range, &ids, &count);
    if (result == XUI_ERROR_UNSUPPORTED) {
        result = doc_quote_split_plan_get(t->draft, range, &split_plan);
        if (result == XUI_OK) {
            split = 1;
            result = doc_quote_edge_normalize(t, &split_plan.left,
                split_plan.parent, 1, &first_index);
            if (result == XUI_OK) result = doc_quote_edge_normalize(t, &split_plan.right,
                split_plan.parent, 1, &end_index);
            /* The second split may clone the first selected ancestor. Resolve
             * both stable boundaries again after all splits have completed. */
            if (result == XUI_OK) result = doc_quote_edge_normalize(t, &split_plan.left,
                split_plan.parent, 0, &first_index);
            if (result == XUI_OK) result = doc_quote_edge_normalize(t, &split_plan.right,
                split_plan.parent, 0, &end_index);
            if (result != XUI_OK) return doc_txn_fail(t, result);
            if (first_index >= end_index) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
            normalized = *range;
            normalized.tAnchor = doc_command_caret(t, split_plan.parent,
                order < 0 ? first_index : end_index, XUI_DOC_POSITION_GAP);
            normalized.tCaret = doc_command_caret(t, split_plan.parent,
                order < 0 ? end_index : first_index, XUI_DOC_POSITION_GAP);
            result = doc_quote_range_ids(t->draft, &normalized, &ids, &count);
        }
    }
    if (result != XUI_OK) return doc_txn_fail(t, result);
    first = doc_index_get(t->draft->index, ids[0]);
    parent = first ? doc_index_get(t->draft->index, first->parent) : NULL;
    index = parent ? doc_child_index(parent, first->id) : DOC_NONE;
    if (!parent || !doc_schema_child(parent->kind, XUI_DOC_QUOTE) || index == DOC_NONE) {
        doc_free(ids); return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    }
    parent_id = parent->id;
    for (i = 0; i < count; i++) {
        item = doc_index_get(t->draft->index, ids[i]);
        if (!item || !doc_schema_child(XUI_DOC_QUOTE, item->kind) ||
            item->parent != parent_id ||
            doc_child_index(parent, item->id) != index + i) {
            doc_free(ids); return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
        }
    }
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_QUOTE;
    result = doc_txn_insert(t, parent_id, index, &desc, &id);
    for (i = 0; i < count && result == XUI_OK; i++)
        result = xuiDocumentTxnMoveNode(t, ids[i], id, DOC_NONE);
    doc_free(ids);
    if (result != XUI_OK) return result;
    *quote_id = id; *after = *range;
    if (order) {
        /* A container gap selects an edge of the moved blocks. Its original
         * offset can remain numerically valid while pointing at a different
         * child, then become invalid on a subsequent wrap. Rebase each gap
         * into the new quote; preserve ordinary text endpoints. Split ranges
         * already select complete blocks on both sides. */
        if (range->tAnchor.iKind == XUI_DOC_POSITION_GAP ||
            (split && range->tCaret.iKind == XUI_DOC_POSITION_GAP))
            after->tAnchor = doc_command_caret(t, id, order < 0 ? 0 : count, XUI_DOC_POSITION_GAP);
        if (range->tCaret.iKind == XUI_DOC_POSITION_GAP ||
            (split && range->tAnchor.iKind == XUI_DOC_POSITION_GAP))
            after->tCaret = doc_command_caret(t, id, order < 0 ? count : 0, XUI_DOC_POSITION_GAP);
    }
    return XUI_OK;
}
XUI_API int xuiDocumentTxnUnwrapQuote(xui_document_transaction t,
    const xui_doc_position_t* at, xui_doc_position_t* caret)
{
    doc_node *quote, *parent, *child; uint64_t id, parent_id, index, count, i;
    uint32_t parent_kind; int result, loosen = 0;
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (!at || !caret) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, at);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (at->iKind == XUI_DOC_POSITION_SOURCE) return doc_txn_fail(t, XUI_DOC_ERROR_DOMAIN);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_position_t target;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnUnwrapQuote(&shadow, at, &target);
        return doc_markdown_shadow_end_unwrap_quote(t, &shadow, result, at, &target, caret);
    }
    quote = doc_index_get(t->draft->index, at->iNodeId);
    for (; quote && quote->kind != XUI_DOC_QUOTE;
         quote = doc_index_get(t->draft->index, quote->parent)) {}
    if (!quote) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    id = quote->id; parent = doc_index_get(t->draft->index, quote->parent);
    index = parent ? doc_child_index(parent, id) : DOC_NONE;
    if (!parent || index == DOC_NONE) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    parent_id = parent->id; parent_kind = parent->kind;
    count = doc_seq_size(quote->children);
    /* Blank separators hidden inside a quote become direct item separators
     * after promotion. Reflect their list spacing in the desired tree before
     * comparing it with Markdown; otherwise an ordinary unwrap is rejected. */
    if (parent_kind == XUI_DOC_LIST_ITEM && count > 1) {
        if (t->draft->profile == XUI_DOCUMENT_MARKDOWN) {
            for (i = 1; i < count && !loosen; i++) {
                doc_node_source_range left, right;
                doc_node_source_range_get(t->draft, doc_index_get(t->draft->index,
                    doc_seq_get_id(quote->children, i - 1)), &left);
                doc_node_source_range_get(t->draft, doc_index_get(t->draft->index,
                    doc_seq_get_id(quote->children, i)), &right);
                result = doc_quote_items_have_blank_line(t->draft, left.syntax_start, right.syntax_start, &loosen);
                if (result != XUI_OK && result != XUI_ERROR_UNSUPPORTED) return doc_txn_fail(t, result);
            }
        } else {
            for (i = 1; i < count && !loosen; i++) {
                doc_node* left = doc_index_get(t->draft->index, doc_seq_get_id(quote->children, i - 1));
                doc_node* right = doc_index_get(t->draft->index, doc_seq_get_id(quote->children, i));
                loosen = left && right && left->kind == XUI_DOC_PARAGRAPH && right->kind == XUI_DOC_PARAGRAPH;
            }
        }
    }
    for (i = 0; i < count; i++) {
        quote = doc_index_get(t->draft->index, id);
        child = doc_index_get(t->draft->index, doc_seq_get_id(quote->children, 0));
        if (!child || !doc_schema_child(parent_kind, child->kind))
            return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
        result = xuiDocumentTxnMoveNode(t, child->id, parent_id, index + 1 + i);
        if (result != XUI_OK) return result;
    }
    result = xuiDocumentTxnDeleteNode(t, id);
    if (result != XUI_OK) return result;
    /* Promotion can put the first/last paragraph directly beside an existing
     * paragraph even when the quote had just one child. Those seams require
     * paragraph separators and hence a loose Markdown list. */
    if (parent_kind == XUI_DOC_LIST_ITEM && !loosen) {
        parent = doc_index_get(t->draft->index, parent_id);
        if (!parent) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
        for (i = index ? index : 1; i < doc_seq_size(parent->children) && i <= index + count; i++) {
            doc_node* left = doc_index_get(t->draft->index, doc_seq_get_id(parent->children, i - 1));
            doc_node* right = doc_index_get(t->draft->index, doc_seq_get_id(parent->children, i));
            if (left && right && left->kind == XUI_DOC_PARAGRAPH && right->kind == XUI_DOC_PARAGRAPH) {
                loosen = 1; break;
            }
        }
    }
    if (loosen) {
        parent = doc_index_get(t->draft->index, parent_id);
        doc_node* list = parent ? doc_index_get(t->draft->index, parent->parent) : NULL;
        if (!list || list->kind != XUI_DOC_LIST) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
        xui_doc_attributes_t attrs = *list->attrs; attrs.iFlags &= ~XUI_DOC_TIGHT;
        result = xuiDocumentTxnSetAttributes(t, list->id, &attrs);
        if (result != XUI_OK) return result;
    }
    *caret = at->iNodeId == id ?
        doc_command_caret(t, parent_id, index, XUI_DOC_POSITION_GAP) : *at;
    return XUI_OK;
}
static int doc_command_set_heading_node(xui_document_transaction t, uint64_t id, uint32_t level)
{
    doc_node *n = doc_index_get(t->draft->index, id), *copy;
    xui_doc_operation_t op = {0}; xui_doc_attributes_t attrs;
    uint32_t kind = level ? XUI_DOC_HEADING : XUI_DOC_PARAGRAPH; int result;
    if (!doc_paragraph(n)) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    attrs = *n->attrs; attrs.iHeadingLevel = level;
    if (!doc_schema_attrs(kind, &attrs)) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    if (n->kind == kind && doc_attributes_equal(n->attrs, &attrs)) return XUI_OK;
    copy = doc_node_clone(t->draft->allocator, n);
    if (!copy) return doc_txn_fail(t, XUI_ERROR_OUT_OF_MEMORY);
    copy->kind = kind;
    result = doc_node_set_attrs(t->draft->allocator, copy, &attrs);
    if (result != XUI_OK) { doc_node_release(copy); return doc_txn_fail(t, result); }
    result = doc_state_set(t->draft, copy); doc_node_release(copy);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    op.iKind = XUI_DOC_OP_ATTRIBUTES; op.iFlags = XUI_DOC_CHANGE_STRUCTURE | XUI_DOC_CHANGE_STYLE; op.iNodeId = id;
    return doc_txn_op(t, &op);
}
XUI_API int xuiDocumentTxnSetHeading(xui_document_transaction t, const xui_doc_range_t* range,
    uint32_t level, xui_doc_range_t* after)
{
    uint64_t *ids = NULL, count = 0, i; int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (!range || !after || level > 6) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_range_t target;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnSetHeading(&shadow, range, level, &target);
        return doc_markdown_shadow_end_range(t, &shadow, result, &target, after);
    }
    result = doc_block_range_ids(t->draft, range, &ids, &count);
    if (result == XUI_ERROR_UNSUPPORTED && level && range->tAnchor.iKind == XUI_DOC_POSITION_GAP &&
        range->tCaret.iKind == XUI_DOC_POSITION_GAP && range->tAnchor.iNodeId == range->tCaret.iNodeId &&
        range->tAnchor.iOffset == range->tCaret.iOffset) {
        doc_node* parent = doc_index_get(t->draft->index, range->tAnchor.iNodeId);
        if (parent && doc_schema_child(parent->kind, XUI_DOC_HEADING)) {
            xui_doc_node_desc_t desc = {0}; uint64_t id;
            desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_HEADING; desc.tAttributes.iHeadingLevel = level;
            result = doc_txn_insert(t, parent->id, range->tAnchor.iOffset, &desc, &id);
            if (result == XUI_OK) after->tAnchor = after->tCaret = doc_command_caret(t, id, 0, XUI_DOC_POSITION_GAP);
            return result;
        }
    }
    if (result != XUI_OK) return doc_txn_fail(t, result);
    *after = *range;
    for (i = 0; i < count && result == XUI_OK; i++) result = doc_command_set_heading_node(t, ids[i], level);
    doc_free(ids);
    return result == XUI_OK ? XUI_OK : doc_txn_fail(t, result);
}
static void doc_command_apply_block_style(xui_doc_attributes_t* attrs, uint32_t fields,
    const xui_doc_block_style_t* style)
{
    if (fields & XUI_DOC_BLOCK_STYLE_ALIGNMENT) {
        attrs->iAlignment = style->iAlignment;
        attrs->iFlags = (attrs->iFlags & ~XUI_DOC_ALIGNMENT_EXPLICIT_LEFT) |
            (!style->iAlignment && !style->bAlignmentInherited ?
                XUI_DOC_ALIGNMENT_EXPLICIT_LEFT : 0);
    }
    if (fields & XUI_DOC_BLOCK_STYLE_SPACING) {
        attrs->fParagraphSpacing = style->bSpacingExplicit ? style->fParagraphSpacing : 0;
        attrs->iFlags = (attrs->iFlags & ~XUI_DOC_SPACING_EXPLICIT) |
            (style->bSpacingExplicit ? XUI_DOC_SPACING_EXPLICIT : 0);
    }
}
XUI_API int xuiDocumentTxnSetBlockStyleRange(xui_document_transaction t, const xui_doc_range_t* range,
    uint32_t fields, const xui_doc_block_style_t* style, xui_doc_range_t* after)
{
    uint64_t *ids = NULL, count = 0, i; int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    if (result != XUI_OK) return result;
    if (!range || !style || style->iSize != sizeof(*style) || !after || !fields ||
        fields & ~(XUI_DOC_BLOCK_STYLE_ALIGNMENT | XUI_DOC_BLOCK_STYLE_SPACING) ||
        ((fields & XUI_DOC_BLOCK_STYLE_ALIGNMENT) &&
            (style->iAlignment > 3 ||
                (style->bAlignmentInherited != 0 && style->bAlignmentInherited != 1) ||
                (style->bAlignmentInherited && style->iAlignment))) ||
        ((fields & XUI_DOC_BLOCK_STYLE_SPACING) && (style->bSpacingExplicit != 0 && style->bSpacingExplicit != 1)) ||
        ((fields & XUI_DOC_BLOCK_STYLE_SPACING) && style->bSpacingExplicit &&
            (!isfinite(style->fParagraphSpacing) || style->fParagraphSpacing < 0 ||
                style->fParagraphSpacing > DOC_MAX_LAYOUT_VALUE)))
        return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN)
        return doc_txn_fail(t, XUI_DOC_ERROR_UNREPRESENTABLE);
    result = doc_block_range_ids(t->draft, range, &ids, &count);
    if (result == XUI_ERROR_UNSUPPORTED && range->tAnchor.iKind == XUI_DOC_POSITION_GAP &&
        range->tCaret.iKind == XUI_DOC_POSITION_GAP && range->tAnchor.iNodeId == range->tCaret.iNodeId &&
        range->tAnchor.iOffset == range->tCaret.iOffset) {
        doc_node* parent = doc_index_get(t->draft->index, range->tAnchor.iNodeId);
        if (parent && doc_schema_child(parent->kind, XUI_DOC_PARAGRAPH)) {
            xui_doc_node_desc_t desc = {0}; uint64_t id;
            desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_PARAGRAPH;
            doc_command_apply_block_style(&desc.tAttributes, fields, style);
            result = doc_txn_insert(t, parent->id, range->tAnchor.iOffset, &desc, &id);
            if (result == XUI_OK) after->tAnchor = after->tCaret = doc_command_caret(t, id, 0, XUI_DOC_POSITION_GAP);
            return result;
        }
    }
    if (result != XUI_OK) return doc_txn_fail(t, result);
    *after = *range;
    for (i = 0; i < count && result == XUI_OK; i++) {
        doc_node* n = doc_index_get(t->draft->index, ids[i]);
        xui_doc_attributes_t attrs = *n->attrs;
        doc_command_apply_block_style(&attrs, fields, style);
        result = xuiDocumentTxnSetAttributes(t, n->id, &attrs);
    }
    doc_free(ids);
    return result == XUI_OK ? XUI_OK : doc_txn_fail(t, result);
}
XUI_API int xuiDocumentTxnJoinBlocks(xui_document_transaction t, uint64_t left, uint64_t right, xui_doc_position_t* caret)
{
    doc_node *a, *b, *parent; uint64_t offset, child, moves; int result = doc_txn_check(t, XUI_DOC_SEMANTIC);
    xui_doc_operation_t merge = {0};
    if (result != XUI_OK) return result;
    if (!caret) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        struct xui_doc_transaction_t shadow; xui_doc_position_t target;
        result = doc_markdown_shadow_begin(t, &shadow); if (result != XUI_OK) return result;
        result = xuiDocumentTxnJoinBlocks(&shadow, left, right, &target);
        return doc_markdown_shadow_end(t, &shadow, result, &target, caret);
    }
    a = doc_index_get(t->draft->index, left); b = doc_index_get(t->draft->index, right);
    if (!doc_paragraph(a) || !doc_paragraph(b) || a->parent != b->parent) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    parent = doc_index_get(t->draft->index, a->parent);
    if (doc_child_index(parent, right) != doc_child_index(parent, left) + 1) return doc_txn_fail(t, XUI_DOC_ERROR_SCHEMA);
    offset = doc_seq_size(a->children); *caret = doc_command_caret(t, left, offset, XUI_DOC_POSITION_GAP);
    if (offset) {
        doc_node* last = doc_index_get(t->draft->index, doc_seq_get_id(a->children, offset - 1));
        if (last->kind == XUI_DOC_TEXT) *caret = doc_command_caret(t, last->id, doc_seq_size(last->text), XUI_DOC_POSITION_TEXT);
    }
    merge.iKind = XUI_DOC_OP_MERGE; merge.iFlags = XUI_DOC_CHANGE_STRUCTURE;
    merge.iNodeId = left; merge.iOtherNodeId = right; merge.iParentId = a->parent;
    merge.iOffset = offset; merge.iOldLength = doc_seq_size(b->children);
    result = doc_txn_op(t, &merge); if (result != XUI_OK) return result;
    moves = t->count;
    while (doc_seq_size((b = doc_index_get(t->draft->index, right))->children)) {
        child = doc_seq_get_id(b->children, 0);
        result = xuiDocumentTxnMoveNode(t, child, left, DOC_NONE); if (result != XUI_OK) return result;
    }
    doc_command_relocations(t, moves);
    return doc_txn_delete(t, right);
}
static int doc_range_branch_container(uint32_t kind)
{
    return kind == XUI_DOC_ROOT || kind == XUI_DOC_QUOTE ||
        kind == XUI_DOC_LIST || kind == XUI_DOC_LIST_ITEM ||
        kind == XUI_DOC_CELL || kind == XUI_DOC_FOOTNOTE;
}
/* Find the closest common block container. Table rows and columns need their
 * own deletion semantics, so a text range cannot cross those structures. */
int doc_range_branch_plan_get(doc_state* state, const doc_node* left,
    const doc_node* right, doc_range_branch_plan* plan)
{
    uint64_t left_path[DOC_MAX_DEPTH], right_path[DOC_MAX_DEPTH];
    const doc_node* node;
    doc_node *ancestor, *target;
    unsigned left_count = 0, right_count = 0, li, ri, i;
    if (!state || !left || !right || !plan) return XUI_ERROR_UNSUPPORTED;
    for (node = left; node; node = doc_index_get(state->index, node->parent)) {
        if (left_count == DOC_MAX_DEPTH) return XUI_ERROR_UNSUPPORTED;
        left_path[left_count++] = node->id;
        if (node->id == DOC_ROOT) break;
    }
    for (node = right; node; node = doc_index_get(state->index, node->parent)) {
        if (right_count == DOC_MAX_DEPTH) return XUI_ERROR_UNSUPPORTED;
        right_path[right_count++] = node->id;
        if (node->id == DOC_ROOT) break;
    }
    if (!left_count || !right_count || left_path[left_count - 1] != DOC_ROOT ||
        right_path[right_count - 1] != DOC_ROOT) return XUI_ERROR_UNSUPPORTED;
    li = left_count; ri = right_count;
    while (li && ri && left_path[li - 1] == right_path[ri - 1]) { li--; ri--; }
    if (!li || !ri || li >= left_count || ri >= right_count)
        return XUI_ERROR_UNSUPPORTED;
    for (i = 1; i <= li; i++) {
        node = doc_index_get(state->index, left_path[i]);
        if (!node || !doc_range_branch_container(node->kind)) return XUI_ERROR_UNSUPPORTED;
    }
    for (i = 1; i <= ri; i++) {
        node = doc_index_get(state->index, right_path[i]);
        if (!node || !doc_range_branch_container(node->kind)) return XUI_ERROR_UNSUPPORTED;
    }
    ancestor = doc_index_get(state->index, left_path[li]);
    target = doc_index_get(state->index, left->parent);
    plan->ancestor = left_path[li];
    plan->left_child = left_path[li - 1];
    plan->right_child = right_path[ri - 1];
    plan->target_parent = left->parent;
    plan->target_index = target ? doc_child_index(target, left->id) : DOC_NONE;
    if (!ancestor || !target || plan->target_index == DOC_NONE ||
        !doc_schema_child(target->kind, right->kind) ||
        doc_child_index(ancestor, plan->left_child) == DOC_NONE ||
        doc_child_index(ancestor, plan->right_child) == DOC_NONE ||
        doc_child_index(ancestor, plan->left_child) >=
            doc_child_index(ancestor, plan->right_child))
        return XUI_ERROR_UNSUPPORTED;
    return XUI_OK;
}
/* Link definitions and trailing bare quote markers have no visible child, but
 * either still parses as an empty quote after its selected paragraph moves out.
 * Keep that container so the semantic shadow matches a source-preserving parse. */
static int doc_range_markdown_keep_empty_quote(xui_document_transaction t,
    const doc_node* node, uint64_t right_block_end, int* keep)
{
    doc_node_source_range range;
    uint64_t i, count;
    *keep = 0;
    if (t->draft->profile != XUI_DOCUMENT_MARKDOWN || node->kind != XUI_DOC_QUOTE)
        return XUI_OK;
    doc_node_source_range_get(t->draft, node, &range);
    count = doc_seq_size(t->draft->references);
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
        range.syntax_start >= range.syntax_end ||
        range.syntax_end > doc_seq_size(t->draft->source)) return XUI_OK;
    for (i = 0; i < count; i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        int result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(t->draft->references, i, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if (ref.iKind == XUI_DOC_REFERENCE_LINK &&
            ref.iSourceStart >= range.syntax_start &&
            ref.iSourceEnd <= range.syntax_end) {
            *keep = 1;
            return XUI_OK;
        }
    }
    if (right_block_end == DOC_NONE || right_block_end < range.syntax_start ||
        right_block_end >= range.syntax_end) return XUI_OK;
    for (i = right_block_end; i < range.syntax_end;) {
        char buffer[4096];
        uint64_t bytes = range.syntax_end - i < sizeof(buffer) ?
            range.syntax_end - i : sizeof(buffer), j;
        int result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(t->draft->source, i, buffer, bytes);
        if (result != XUI_OK) return result;
        for (j = 0; j < bytes; j++) {
            char c = buffer[j];
            if (c == '>') *keep = 1;
            else if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
                *keep = 0;
                return XUI_OK;
            }
        }
        i += bytes;
    }
    return XUI_OK;
}
/* A source-only link definition is outside a semantic text selection. Keep
 * its preceding item while removing the selected visible blocks. The exact
 * source patch later verifies the surviving marker and definition bytes. */
static int doc_range_markdown_removable_item_block(uint32_t kind)
{
    return kind == XUI_DOC_PARAGRAPH || kind == XUI_DOC_CODE_BLOCK ||
        kind == XUI_DOC_QUOTE || kind == XUI_DOC_HEADING ||
        kind == XUI_DOC_RULE || kind == XUI_DOC_HTML ||
        kind == XUI_DOC_TABLE || kind == XUI_DOC_DIAGRAM;
}
static int doc_range_markdown_keep_definition_item(xui_document_transaction t,
    const doc_node* node, int* keep)
{
    doc_node_source_range range, last_range;
    doc_node *first, *last;
    uint64_t i, count, children;
    *keep = 0;
    if (t->draft->profile != XUI_DOCUMENT_MARKDOWN ||
        node->kind != XUI_DOC_LIST_ITEM)
        return XUI_OK;
    children = doc_seq_size(node->children);
    if (children) {
        first = doc_index_get(t->draft->index,
            doc_seq_get_id(node->children, 0));
        last = doc_index_get(t->draft->index,
            doc_seq_get_id(node->children, children - 1));
        if (!first || !doc_range_markdown_removable_item_block(first->kind) ||
            !last)
            return XUI_OK;
        doc_node_source_range_get(t->draft, last, &last_range);
        if (last_range.syntax_end == DOC_NONE) return XUI_OK;
    }
    doc_node_source_range_get(t->draft, node, &range);
    if (range.syntax_start == DOC_NONE || range.syntax_end == DOC_NONE ||
        range.syntax_start >= range.syntax_end ||
        range.syntax_end > doc_seq_size(t->draft->source)) return XUI_OK;
    if (children && (last_range.syntax_end <= range.syntax_start ||
        last_range.syntax_end > range.syntax_end)) return XUI_OK;
    count = doc_seq_size(t->draft->references);
    for (i = 0; i < count; i += sizeof(xui_doc_reference_definition_t)) {
        xui_doc_reference_definition_t ref;
        int result = doc_txn_check(t, 0);
        if (result != XUI_OK) return result;
        result = doc_seq_read(t->draft->references, i, &ref, sizeof(ref));
        if (result != XUI_OK) return result;
        if (ref.iSourceStart < range.syntax_start ||
            ref.iSourceEnd > range.syntax_end) continue;
        if (ref.iKind != XUI_DOC_REFERENCE_LINK) continue;
        if (children) {
            uint64_t child;
            int in_gap = 0;
            for (child = 1; child <= children; child++) {
                doc_node_source_range before, after;
                uint64_t gap_start = range.syntax_start;
                uint64_t gap_end = range.syntax_end;
                if (child) {
                    doc_node* n = doc_index_get(t->draft->index,
                        doc_seq_get_id(node->children, child - 1));
                    if (!n) { *keep = 0; return XUI_OK; }
                    doc_node_source_range_get(t->draft, n, &before);
                    gap_start = before.syntax_end;
                }
                if (child < children) {
                    doc_node* n = doc_index_get(t->draft->index,
                        doc_seq_get_id(node->children, child));
                    if (!n) { *keep = 0; return XUI_OK; }
                    doc_node_source_range_get(t->draft, n, &after);
                    gap_end = after.syntax_start;
                }
                if (gap_start != DOC_NONE && gap_end != DOC_NONE &&
                    ref.iSourceStart >= gap_start && ref.iSourceEnd <= gap_end) {
                    in_gap = 1;
                    break;
                }
            }
            if (!in_gap) { *keep = 0; return XUI_OK; }
        }
        *keep = children ? 2 : 1;
    }
    return XUI_OK;
}
static int doc_delete_range(xui_document_transaction t, xui_doc_position_t a, xui_doc_position_t b, xui_doc_position_t* caret)
{
    doc_node *na = doc_index_get(t->draft->index, a.iNodeId), *nb = doc_index_get(t->draft->index, b.iNodeId), *pa, *pb;
    uint64_t ai, bi, i, ap, bp, parent, right_block_end = DOC_NONE;
    doc_range_branch_plan branch_plan = {0};
    int result, cross_parent;
    *caret = a;
    if (a.iNodeId == b.iNodeId) {
        if (a.iKind == XUI_DOC_POSITION_TEXT) return doc_txn_text(t, a.iNodeId, a.iOffset, b.iOffset, "", 0);
        for (i = b.iOffset; i > a.iOffset; i--) {
            na = doc_index_get(t->draft->index, a.iNodeId);
            result = doc_txn_delete(t, doc_seq_get_id(na->children, i - 1)); if (result != XUI_OK) return result;
        }
        return XUI_OK;
    }
    pa = a.iKind == XUI_DOC_POSITION_TEXT ? doc_index_get(t->draft->index, na->parent) : na;
    pb = b.iKind == XUI_DOC_POSITION_TEXT ? doc_index_get(t->draft->index, nb->parent) : nb;
    if (!doc_paragraph(pa) || !doc_paragraph(pb)) return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    cross_parent = pa->id != pb->id && pa->parent != pb->parent;
    if (cross_parent && doc_range_branch_plan_get(t->draft, pa, pb,
        &branch_plan) != XUI_OK)
        return doc_txn_fail(t, XUI_ERROR_UNSUPPORTED);
    if (cross_parent && t->draft->profile == XUI_DOCUMENT_MARKDOWN) {
        doc_node_source_range source_range;
        doc_node_source_range_get(t->draft, pb, &source_range);
        right_block_end = source_range.syntax_end;
    }
    ap = pa->id; bp = pb->id; parent = pa->parent;
    ai = a.iKind == XUI_DOC_POSITION_TEXT ? doc_child_index(pa, na->id) + 1 : a.iOffset;
    bi = b.iKind == XUI_DOC_POSITION_TEXT ? doc_child_index(pb, nb->id) : b.iOffset;
    if (a.iKind == XUI_DOC_POSITION_TEXT) {
        result = doc_txn_text(t, a.iNodeId, a.iOffset, doc_seq_size(na->text), "", 0); if (result != XUI_OK) return result;
    }
    if (b.iKind == XUI_DOC_POSITION_TEXT) {
        result = doc_txn_text(t, b.iNodeId, 0, b.iOffset, "", 0); if (result != XUI_OK) return result;
    }
    if (ap == bp) {
        for (i = bi; i > ai; i--) {
            pa = doc_index_get(t->draft->index, ap);
            result = doc_txn_delete(t, doc_seq_get_id(pa->children, i - 1)); if (result != XUI_OK) return result;
        }
        return XUI_OK;
    }
    for (i = doc_seq_size(doc_index_get(t->draft->index, ap)->children); i > ai; i--) {
        pa = doc_index_get(t->draft->index, ap);
        result = doc_txn_delete(t, doc_seq_get_id(pa->children, i - 1)); if (result != XUI_OK) return result;
    }
    for (i = bi; i > 0; i--) {
        pb = doc_index_get(t->draft->index, bp);
        result = doc_txn_delete(t, doc_seq_get_id(pb->children, i - 1)); if (result != XUI_OK) return result;
    }
    if (cross_parent) {
        xui_doc_position_t unused;
        uint64_t current = ap, right_parent = doc_index_get(t->draft->index, bp)->parent;
        uint64_t next;
        doc_node *node, *container;
        while ((node = doc_index_get(t->draft->index, current))->parent != branch_plan.ancestor) {
            container = doc_index_get(t->draft->index, node->parent);
            ai = doc_child_index(container, current);
            for (i = doc_seq_size(container->children); i > ai + 1; i--) {
                container = doc_index_get(t->draft->index, node->parent);
                result = doc_txn_delete(t, doc_seq_get_id(container->children, i - 1));
                if (result != XUI_OK) return result;
            }
            current = node->parent;
        }
        current = bp;
        while ((node = doc_index_get(t->draft->index, current))->parent != branch_plan.ancestor) {
            uint64_t container_id = node->parent;
            int tighten_parent_list;
            container = doc_index_get(t->draft->index, container_id);
            bi = doc_child_index(container, current);
            tighten_parent_list = t->draft->profile == XUI_DOCUMENT_MARKDOWN &&
                container->kind == XUI_DOC_LIST_ITEM &&
                node->kind == XUI_DOC_LIST && bi > 0;
            for (i = bi; i > 0; i--) {
                int keep;
                container = doc_index_get(t->draft->index, container_id);
                result = doc_range_markdown_keep_definition_item(t,
                    doc_index_get(t->draft->index,
                        doc_seq_get_id(container->children, i - 1)), &keep);
                if (result != XUI_OK) return result;
                if (keep) {
                    if (keep == 2) {
                        uint64_t list_id = container->id;
                        doc_node* item = doc_index_get(t->draft->index,
                            doc_seq_get_id(container->children, i - 1));
                        uint64_t item_id = item->id;
                        while (doc_seq_size(item->children)) {
                            uint64_t child_id = doc_seq_get_id(item->children,
                                doc_seq_size(item->children) - 1);
                            result = doc_txn_delete(t, child_id);
                            if (result != XUI_OK) return result;
                            item = doc_index_get(t->draft->index, item_id);
                        }
                        container = doc_index_get(t->draft->index,
                            list_id);
                        if (container->kind == XUI_DOC_LIST &&
                            !(container->attrs->iFlags & XUI_DOC_TIGHT)) {
                            xui_doc_attributes_t attrs = *container->attrs;
                            attrs.iFlags |= XUI_DOC_TIGHT;
                            result = xuiDocumentTxnSetAttributes(t,
                                container->id, &attrs);
                            if (result != XUI_OK) return result;
                        }
                    }
                    continue;
                }
                result = doc_txn_delete(t, doc_seq_get_id(container->children, i - 1));
                if (result != XUI_OK) return result;
            }
            if (tighten_parent_list) {
                doc_node* item = doc_index_get(t->draft->index, container_id);
                doc_node* list = doc_index_get(t->draft->index, item->parent);
                if (list && list->kind == XUI_DOC_LIST &&
                    !(list->attrs->iFlags & XUI_DOC_TIGHT)) {
                    xui_doc_attributes_t attrs = *list->attrs;
                    attrs.iFlags |= XUI_DOC_TIGHT;
                    result = xuiDocumentTxnSetAttributes(t, list->id, &attrs);
                    if (result != XUI_OK) return result;
                }
            }
            current = container_id;
        }
        container = doc_index_get(t->draft->index, branch_plan.ancestor);
        ai = doc_child_index(container, branch_plan.left_child);
        bi = doc_child_index(container, branch_plan.right_child);
        for (i = bi; i > ai + 1; i--) {
            container = doc_index_get(t->draft->index, branch_plan.ancestor);
            result = doc_txn_delete(t, doc_seq_get_id(container->children, i - 1));
            if (result != XUI_OK) return result;
        }
        result = xuiDocumentTxnMoveNode(t, bp, branch_plan.target_parent,
            branch_plan.target_index + 1);
        if (result != XUI_OK) return result;
        result = xuiDocumentTxnJoinBlocks(t, ap, bp, &unused);
        if (result != XUI_OK) return result;
        current = right_parent;
        while (current != branch_plan.ancestor) {
            int keep;
            node = doc_index_get(t->draft->index, current);
            if (doc_seq_size(node->children)) break;
            result = doc_range_markdown_keep_empty_quote(t, node,
                right_block_end, &keep);
            if (result != XUI_OK) return result;
            if (keep) break;
            next = node->parent;
            result = doc_txn_delete(t, current);
            if (result != XUI_OK) return result;
            current = next;
        }
        return XUI_OK;
    }
    pa = doc_index_get(t->draft->index, parent); ai = doc_child_index(pa, ap); bi = doc_child_index(pa, bp);
    for (i = bi; i > ai + 1; i--) {
        pa = doc_index_get(t->draft->index, parent);
        result = doc_txn_delete(t, doc_seq_get_id(pa->children, i - 1)); if (result != XUI_OK) return result;
    }
    { xui_doc_position_t unused; return xuiDocumentTxnJoinBlocks(t, ap, bp, &unused); }
}
XUI_API int xuiDocumentTxnReplaceRange(xui_document_transaction t, const xui_doc_range_t* range,
    const char* text, uint64_t bytes, xui_doc_position_t* caret)
{
    xui_doc_position_t a, b, p; uint64_t start, i; int order, result = doc_txn_check(t, 0);
    if (result != XUI_OK) return result;
    if (!range || !caret || !doc_utf8(text, bytes)) return doc_txn_fail(t, XUI_ERROR_INVALID_ARGUMENT);
    result = doc_command_position(t, &range->tAnchor);
    if (result == XUI_OK) result = doc_command_position(t, &range->tCaret);
    if (result == XUI_OK) result = doc_position_compare(t->draft, &range->tAnchor, &range->tCaret, &order);
    if (result != XUI_OK) return doc_txn_fail(t, result);
    a = order <= 0 ? range->tAnchor : range->tCaret; b = order <= 0 ? range->tCaret : range->tAnchor;
    if (a.iKind == XUI_DOC_POSITION_SOURCE) {
        result = xuiDocumentTxnReplaceSource(t, a.iOffset, b.iOffset, text, bytes);
        if (result == XUI_OK) { *caret = a; caret->iOffset += bytes; caret->iAffinity = XUI_DOC_AFTER; }
        return result;
    }
    result = doc_txn_check(t, XUI_DOC_SEMANTIC); if (result != XUI_OK) return result;
    if (t->draft->profile == XUI_DOCUMENT_MARKDOWN && !t->parsing) {
        if (a.iKind != XUI_DOC_POSITION_TEXT || b.iKind != XUI_DOC_POSITION_TEXT || a.iNodeId != b.iNodeId ||
            (bytes && doc_index_get(t->draft->index, a.iNodeId)->kind == XUI_DOC_TEXT && (memchr(text, '\n', (size_t)bytes) || memchr(text, '\r', (size_t)bytes))))
            return doc_markdown_range(t, range, text, bytes, caret);
        return doc_markdown_text(t, a.iNodeId, a.iOffset, b.iOffset, text, bytes, caret);
    }
    result = order ? doc_delete_range(t, a, b, &p) : XUI_OK;
    if (!order) p = a;
    if (result != XUI_OK) return result;
    if (!bytes) { *caret = p; return XUI_OK; }
    result = doc_text_caret(t, &p); if (result != XUI_OK) return doc_txn_fail(t, result);
    if (doc_index_get(t->draft->index, p.iNodeId)->kind != XUI_DOC_TEXT) {
        result = doc_txn_text(t, p.iNodeId, p.iOffset, p.iOffset, text, bytes);
        if (result == XUI_OK) { p.iOffset += bytes; *caret = p; } return result;
    }
    for (start = i = 0; i <= bytes; i++) {
        if (i != bytes && text[i] != '\r' && text[i] != '\n') continue;
        result = doc_txn_text(t, p.iNodeId, p.iOffset, p.iOffset, text + start, i - start);
        if (result != XUI_OK) return result;
        p.iOffset += i - start;
        if (i < bytes) {
            result = xuiDocumentTxnSplitBlock(t, &p, &p); if (result != XUI_OK) return result;
            if (text[i] == '\r' && i + 1 < bytes && text[i + 1] == '\n') i++;
        }
        start = i + 1;
    }
    p.iAffinity = XUI_DOC_AFTER; *caret = p; return XUI_OK;
}

typedef struct doc_range_copy { doc_state* state; xui_doc_position_t a, b; char* text; uint64_t length, capacity; } doc_range_copy;
static void doc_copy_append(doc_range_copy* c, const char* s, uint64_t bytes) { if (bytes) memcpy(c->text + c->length, s, (size_t)bytes); c->length += bytes; }
static void doc_copy_range_node(doc_range_copy* c, uint64_t id)
{
    doc_node* n = doc_index_get(c->state->index, id); xui_doc_position_t first = c->a, last = c->a;
    int ca, cb; uint64_t i, size = doc_text_kind(n->kind) ? doc_seq_size(n->text) : doc_seq_size(n->children);
    first.iNodeId = last.iNodeId = id; first.iOffset = 0; last.iOffset = size;
    first.iKind = last.iKind = doc_text_kind(n->kind) ? XUI_DOC_POSITION_TEXT : XUI_DOC_POSITION_GAP;
    doc_position_compare(c->state, &last, &c->a, &ca); doc_position_compare(c->state, &first, &c->b, &cb);
    if (ca <= 0 || cb >= 0) return;
    if (doc_text_kind(n->kind)) {
        uint64_t from = c->a.iNodeId == id ? c->a.iOffset : 0, to = c->b.iNodeId == id ? c->b.iOffset : size;
        doc_seq_read(n->text, from, c->text + c->length, to - from); c->length += to - from;
    }
    for (i = 0; i < doc_seq_size(n->children); i++) doc_copy_range_node(c, doc_seq_get_id(n->children, i));
    if (n->kind == XUI_DOC_SOFT_BREAK || n->kind == XUI_DOC_HARD_BREAK) doc_copy_append(c, "\n", 1);
    doc_position_compare(c->state, &last, &c->b, &cb);
    if (cb < 0) {
        doc_node* parent = doc_index_get(c->state->index, n->parent);
        int last_child = parent && doc_seq_get_id(parent->children, doc_seq_size(parent->children) - 1) == id;
        if (n->kind == XUI_DOC_CELL) { if (!last_child) doc_copy_append(c, "\t", 1); }
        else if ((doc_paragraph(n) || n->kind == XUI_DOC_CODE_BLOCK || n->kind == XUI_DOC_ROW) &&
            !(last_child && parent->kind == XUI_DOC_CELL)) doc_copy_append(c, "\n", 1);
    }
}
XUI_API int xuiDocumentSnapshotCopyRange(xui_document_snapshot s, const xui_doc_range_t* range, char** out, uint64_t* bytes)
{
    doc_range_copy c = {0}; int order, result;
    if (out) *out = NULL;
    if (bytes) *bytes = 0;
    if (!s || !range || !out || !bytes) return XUI_ERROR_INVALID_ARGUMENT;
    result = xuiDocumentSnapshotComparePositions(s, &range->tAnchor, &range->tCaret, &order); if (result != XUI_OK) return result;
    c.state = s->state; c.a = order <= 0 ? range->tAnchor : range->tCaret; c.b = order <= 0 ? range->tCaret : range->tAnchor;
    if (s->state->node_count > (UINT64_MAX - s->state->text_bytes - 1) / 2) return XUI_DOC_ERROR_LIMIT;
    c.capacity = c.a.iKind == XUI_DOC_POSITION_SOURCE ? c.b.iOffset - c.a.iOffset : s->state->text_bytes + s->state->node_count * 2;
    if (c.capacity >= SIZE_MAX) return XUI_DOC_ERROR_LIMIT;
    c.text = malloc((size_t)c.capacity + 1); if (!c.text) return XUI_ERROR_OUT_OF_MEMORY;
    if (c.a.iKind == XUI_DOC_POSITION_SOURCE) { doc_seq_read(s->state->source, c.a.iOffset, c.text, c.capacity); c.length = c.capacity; }
    else if (order) doc_copy_range_node(&c, DOC_ROOT);
    c.text[c.length] = 0; *out = c.text; *bytes = c.length; return XUI_OK;
}

#endif
