#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"

typedef struct doc_md_loss_scan {
    doc_state* state;
    uint32_t dialect;
    xui_doc_markdown_loss_t* items;
    uint64_t capacity, count;
    uint32_t reasons_union;
} doc_md_loss_scan;

static int doc_md_has_line_break(doc_sequence* text)
{
    char block[256];
    uint64_t at = 0, size = doc_seq_size(text);
    while (at < size) {
        uint64_t take = size - at < sizeof(block) ? size - at : sizeof(block);
        if (doc_seq_read(text, at, block, take) != XUI_OK) return 1;
        if (memchr(block, '\n', (size_t)take) || memchr(block, '\r', (size_t)take)) return 1;
        at += take;
    }
    return 0;
}

static void doc_md_loss_append(doc_md_loss_scan* scan, const doc_node* node, uint32_t reasons)
{
    if (!reasons) return;
    scan->reasons_union |= reasons;
    if (scan->count < scan->capacity) {
        xui_doc_markdown_loss_t* item = &scan->items[scan->count];
        *item = (xui_doc_markdown_loss_t){0};
        item->iSize = sizeof(*item);
        item->iNodeKind = node->kind;
        item->iNodeId = node->id;
        item->iReasons = reasons;
    }
    scan->count++;
}

static uint32_t doc_md_loss_node(doc_md_loss_scan* scan, const doc_node* n)
{
    const xui_doc_attributes_t* a = n->attrs;
    uint32_t reasons = 0, allowed_flags = 0, allowed_marks = 0;
    int resource = n->resource && n->resource->size;
    int info = n->info && n->info->size;
    int title = n->title && n->title->size;
    uint64_t i;

    switch (n->kind) {
    case XUI_DOC_ROOT: case XUI_DOC_PARAGRAPH: case XUI_DOC_HEADING:
    case XUI_DOC_TEXT: case XUI_DOC_QUOTE: case XUI_DOC_LIST:
    case XUI_DOC_LIST_ITEM: case XUI_DOC_CODE_BLOCK: case XUI_DOC_TABLE:
    case XUI_DOC_ROW: case XUI_DOC_CELL: case XUI_DOC_IMAGE:
    case XUI_DOC_RULE: case XUI_DOC_SOFT_BREAK: case XUI_DOC_HARD_BREAK:
    case XUI_DOC_HTML: case XUI_DOC_MATH: case XUI_DOC_DIAGRAM:
    case XUI_DOC_FOOTNOTE: case XUI_DOC_FOOTNOTE_REF:
    case XUI_DOC_FRONT_MATTER: break;
    default: reasons |= XUI_DOC_MD_LOSS_NODE; break;
    }
    if ((n->kind == XUI_DOC_IMAGE || n->kind == XUI_DOC_MATH ||
         n->kind == XUI_DOC_FOOTNOTE_REF) && n->parent) {
        doc_node* parent = doc_index_get(scan->state->index, n->parent);
        if (parent && parent->kind != XUI_DOC_PARAGRAPH && parent->kind != XUI_DOC_HEADING)
            reasons |= XUI_DOC_MD_LOSS_NODE;
    }
    if (scan->dialect == XUI_MD_COMMONMARK &&
        (n->kind == XUI_DOC_TABLE || (n->kind == XUI_DOC_LIST_ITEM &&
        (a->iFlags & (XUI_DOC_TASK | XUI_DOC_CHECKED)))))
        reasons |= XUI_DOC_MD_LOSS_DIALECT;
    if (scan->dialect != XUI_MD_EXTENDED &&
        (n->kind == XUI_DOC_MATH || n->kind == XUI_DOC_DIAGRAM ||
         n->kind == XUI_DOC_FOOTNOTE || n->kind == XUI_DOC_FOOTNOTE_REF ||
         n->kind == XUI_DOC_FRONT_MATTER || (n->kind == XUI_DOC_QUOTE && info)))
        reasons |= XUI_DOC_MD_LOSS_DIALECT;

    if (n->kind == XUI_DOC_TEXT) {
        allowed_marks = XUI_DOC_BOLD | XUI_DOC_ITALIC | XUI_DOC_CODE | XUI_DOC_LINK;
        if (scan->dialect != XUI_MD_COMMONMARK) allowed_marks |= XUI_DOC_STRIKE;
        if (scan->dialect == XUI_MD_EXTENDED)
            allowed_marks |= XUI_DOC_HIGHLIGHT | XUI_DOC_SUBSCRIPT | XUI_DOC_SUPERSCRIPT;
    }
    if (n->kind == XUI_DOC_IMAGE) allowed_marks = XUI_DOC_LINK;
    if (a->iMarks & ~allowed_marks) reasons |= XUI_DOC_MD_LOSS_MARK;
    if (n->kind != XUI_DOC_TEXT && n->kind != XUI_DOC_IMAGE &&
        (a->iMarks & XUI_DOC_LINK))
        reasons |= XUI_DOC_MD_LOSS_METADATA;
    if (doc_text_color_set(a) || doc_background_color_set(a) ||
        (a->iFlags & XUI_DOC_TEXT_COLOR_CURRENT) ||
        a->fFontSize || a->sFontFamily[0])
        reasons |= XUI_DOC_MD_LOSS_TEXT_STYLE;
    if (a->sLanguage) reasons |= XUI_DOC_MD_LOSS_LANGUAGE;
    if (a->fParagraphSpacing || (a->iFlags & XUI_DOC_SPACING_EXPLICIT) ||
        (a->iFlags & XUI_DOC_ALIGNMENT_EXPLICIT_LEFT) ||
        (a->iAlignment && n->kind != XUI_DOC_CELL) ||
        (n->kind == XUI_DOC_CELL && a->iAlignment > 2))
        reasons |= XUI_DOC_MD_LOSS_BLOCK_STYLE;
    if (a->fWidth || a->fHeight || a->iRowSpan != 1 || a->iColumnSpan != 1)
        reasons |= XUI_DOC_MD_LOSS_GEOMETRY;
    if (a->iHeadingLevel && n->kind != XUI_DOC_HEADING) reasons |= XUI_DOC_MD_LOSS_METADATA;
    if (a->iListStart && (n->kind != XUI_DOC_LIST || !(a->iFlags & XUI_DOC_ORDERED)))
        reasons |= XUI_DOC_MD_LOSS_METADATA;

    switch (n->kind) {
    case XUI_DOC_LIST: allowed_flags = XUI_DOC_ORDERED | XUI_DOC_TIGHT; break;
    case XUI_DOC_LIST_ITEM: allowed_flags = XUI_DOC_TASK | XUI_DOC_CHECKED; break;
    case XUI_DOC_CELL: allowed_flags = XUI_DOC_HEADER; break;
    case XUI_DOC_MATH: allowed_flags = XUI_DOC_BLOCK; break;
    case XUI_DOC_HTML: allowed_flags = XUI_DOC_BLOCK; break;
    default: break;
    }
    if (a->iFlags & ~(allowed_flags | XUI_DOC_SPACING_EXPLICIT |
        XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO | XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO |
        XUI_DOC_TEXT_COLOR_CURRENT | XUI_DOC_BACKGROUND_COLOR_CURRENT |
        XUI_DOC_ALIGNMENT_EXPLICIT_LEFT))
        reasons |= XUI_DOC_MD_LOSS_METADATA;
    if (n->kind == XUI_DOC_LIST_ITEM && (a->iFlags & XUI_DOC_CHECKED) &&
        !(a->iFlags & XUI_DOC_TASK)) reasons |= XUI_DOC_MD_LOSS_METADATA;
    if (resource && !(n->kind == XUI_DOC_IMAGE ||
        (n->kind == XUI_DOC_TEXT && (a->iMarks & XUI_DOC_LINK))))
        reasons |= XUI_DOC_MD_LOSS_METADATA;
    if (title && !(n->kind == XUI_DOC_IMAGE ||
        (n->kind == XUI_DOC_TEXT && (a->iMarks & XUI_DOC_LINK))))
        reasons |= XUI_DOC_MD_LOSS_METADATA;
    if (info && !(n->kind == XUI_DOC_CODE_BLOCK || n->kind == XUI_DOC_QUOTE ||
        n->kind == XUI_DOC_FOOTNOTE || n->kind == XUI_DOC_FOOTNOTE_REF))
        reasons |= XUI_DOC_MD_LOSS_METADATA;

    if (n->kind == XUI_DOC_PARAGRAPH && doc_semantic_empty_paragraph(scan->state, n))
        reasons |= XUI_DOC_MD_LOSS_EMPTY_BLOCK;
    if (n->kind == XUI_DOC_TABLE) {
        if (!doc_seq_size(n->children)) reasons |= XUI_DOC_MD_LOSS_TABLE_CONTENT;
        if (n->column_widths)
            for (i = 0; i < n->column_widths->size / sizeof(float); i++)
                if (doc_table_column_width(n, (uint32_t)i)) {
                    reasons |= XUI_DOC_MD_LOSS_GEOMETRY; break;
                }
    }
    if (n->kind == XUI_DOC_CELL) {
        doc_node* row = doc_index_get(scan->state->index, n->parent);
        doc_node* table = row ? doc_index_get(scan->state->index, row->parent) : NULL;
        if (doc_seq_size(n->children) > 1 || (doc_seq_size(n->children) &&
            doc_index_get(scan->state->index, doc_seq_get_id(n->children, 0))->kind != XUI_DOC_PARAGRAPH))
            reasons |= XUI_DOC_MD_LOSS_TABLE_CONTENT;
        if (a->iRowSpan != 1 || a->iColumnSpan != 1)
            reasons |= XUI_DOC_MD_LOSS_TABLE_CONTENT;
        if (doc_seq_size(n->children) == 1) {
            doc_node* paragraph = doc_index_get(scan->state->index, doc_seq_get_id(n->children, 0));
            if (paragraph->kind == XUI_DOC_PARAGRAPH) {
                for (i = 0; i < doc_seq_size(paragraph->children); i++) {
                    doc_node* inline_node = doc_index_get(scan->state->index,
                        doc_seq_get_id(paragraph->children, i));
                    if (inline_node->kind == XUI_DOC_SOFT_BREAK ||
                        inline_node->kind == XUI_DOC_HARD_BREAK ||
                        doc_md_has_line_break(inline_node->text)) {
                        reasons |= XUI_DOC_MD_LOSS_TABLE_CONTENT; break;
                    }
                }
            }
        }
        if (table) {
            uint64_t row_index = doc_child_index(table, row->id);
            uint64_t column = doc_child_index(row, n->id);
            doc_node* header = doc_index_get(scan->state->index, doc_seq_get_id(table->children, 0));
            doc_node* head_cell = header && column < doc_seq_size(header->children) ?
                doc_index_get(scan->state->index, doc_seq_get_id(header->children, column)) : NULL;
            if (!head_cell || a->iAlignment != head_cell->attrs->iAlignment ||
                !!(a->iFlags & XUI_DOC_HEADER) != !row_index)
                reasons |= XUI_DOC_MD_LOSS_TABLE_CONTENT;
        }
    }
    return reasons;
}

static void doc_md_loss_walk(doc_md_loss_scan* scan, uint64_t id)
{
    doc_node* n = doc_index_get(scan->state->index, id);
    uint64_t i;
    doc_md_loss_append(scan, n, doc_md_loss_node(scan, n));
    for (i = 0; i < doc_seq_size(n->children); i++)
        doc_md_loss_walk(scan, doc_seq_get_id(n->children, i));
}

static int doc_md_conversion_input(xui_document_snapshot snapshot, uint32_t dialect)
{
    return snapshot && snapshot->state->profile == XUI_DOCUMENT_RICH &&
        dialect >= XUI_MD_COMMONMARK && dialect <= XUI_MD_EXTENDED;
}

static uint32_t doc_md_supported_marks(uint32_t dialect)
{
    uint32_t marks = XUI_DOC_BOLD | XUI_DOC_ITALIC | XUI_DOC_CODE | XUI_DOC_LINK;
    if (dialect != XUI_MD_COMMONMARK) marks |= XUI_DOC_STRIKE;
    if (dialect == XUI_MD_EXTENDED)
        marks |= XUI_DOC_HIGHLIGHT | XUI_DOC_SUBSCRIPT | XUI_DOC_SUPERSCRIPT;
    return marks;
}

static int doc_md_strip_presentation(xui_document_transaction t, uint64_t id,
    uint32_t dialect, uint32_t accepted)
{
    doc_node* node = doc_index_get(t->draft->index, id);
    xui_doc_attributes_t attrs = *node->attrs;
    uint64_t i, child;
    int result;
    if (accepted & XUI_DOC_MD_LOSS_MARK) {
        if (node->kind == XUI_DOC_TEXT) attrs.iMarks &= doc_md_supported_marks(dialect);
        else if (node->kind == XUI_DOC_IMAGE) attrs.iMarks &= XUI_DOC_LINK;
        else attrs.iMarks = 0;
    }
    if (accepted & XUI_DOC_MD_LOSS_TEXT_STYLE) {
        attrs.iTextColor = attrs.iBackgroundColor = 0;
        attrs.iFlags &= ~(XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO |
            XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO |
            XUI_DOC_TEXT_COLOR_CURRENT | XUI_DOC_BACKGROUND_COLOR_CURRENT);
        attrs.fFontSize = 0;
        memset(attrs.sFontFamily, 0, sizeof(attrs.sFontFamily));
    }
    if (accepted & XUI_DOC_MD_LOSS_LANGUAGE) attrs.sLanguage = NULL;
    if (accepted & XUI_DOC_MD_LOSS_BLOCK_STYLE) {
        attrs.fParagraphSpacing = 0;
        attrs.iFlags &= ~(XUI_DOC_SPACING_EXPLICIT | XUI_DOC_ALIGNMENT_EXPLICIT_LEFT);
        if (node->kind != XUI_DOC_CELL || attrs.iAlignment > 2) attrs.iAlignment = 0;
    }
    if (accepted & XUI_DOC_MD_LOSS_GEOMETRY) attrs.fWidth = attrs.fHeight = 0;
    if (!doc_attributes_equal(&attrs, node->attrs)) {
        result = xuiDocumentTxnSetAttributes(t, id, &attrs);
        if (result != XUI_OK) return result;
    }
    node = doc_index_get(t->draft->index, id);
    if (node->kind == XUI_DOC_TABLE && (accepted & XUI_DOC_MD_LOSS_GEOMETRY)) {
        uint32_t column, columns = doc_table_column_count(t->draft, node);
        for (column = 0; column < columns; column++) {
            node = doc_index_get(t->draft->index, id);
            if (!doc_table_column_width(node, column)) continue;
            result = xuiDocumentTxnSetTableColumnWidth(t, id, column, 0);
            if (result != XUI_OK) return result;
        }
    }
    node = doc_index_get(t->draft->index, id);
    for (i = 0; i < doc_seq_size(node->children); i++) {
        child = doc_seq_get_id(node->children, i);
        result = doc_md_strip_presentation(t, child, dialect, accepted);
        if (result != XUI_OK) return result;
        node = doc_index_get(t->draft->index, id);
    }
    return XUI_OK;
}

XUI_API int xuiDocumentSnapshotConvertToMarkdownWithPolicy(
    xui_document_snapshot snapshot, uint32_t dialect,
    uint32_t accepted_reasons, xui_document* out)
{
    const uint32_t supported = XUI_DOC_MD_LOSS_MARK | XUI_DOC_MD_LOSS_TEXT_STYLE |
        XUI_DOC_MD_LOSS_BLOCK_STYLE | XUI_DOC_MD_LOSS_GEOMETRY | XUI_DOC_MD_LOSS_LANGUAGE;
    xui_doc_desc_t desc = {0};
    xui_document temporary = NULL;
    xui_document_transaction transaction = NULL;
    xui_document_snapshot normalized = NULL;
    doc_md_loss_scan scan = {0};
    doc_node* root;
    uint64_t i, copied;
    int result;
    if (out) *out = NULL;
    if (!out || !doc_md_conversion_input(snapshot, dialect) ||
        (accepted_reasons & ~supported)) return XUI_ERROR_INVALID_ARGUMENT;
    if (!accepted_reasons) return xuiDocumentSnapshotConvertToMarkdown(snapshot, dialect, out);
    scan.state = snapshot->state; scan.dialect = dialect;
    doc_md_loss_walk(&scan, DOC_ROOT);
    if (!scan.count) return xuiDocumentSnapshotConvertToMarkdown(snapshot, dialect, out);
    if (scan.reasons_union & ~accepted_reasons) return XUI_DOC_ERROR_UNREPRESENTABLE;

    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_RICH;
    desc.bDisableHistory = 1;
    desc.iMaxTextBytes = snapshot->state->allocator->max_bytes;
    desc.iMaxNodes = snapshot->state->allocator->max_nodes;
    result = xuiDocumentCreate(&desc, &temporary);
    if (result != XUI_OK) return result;
    result = xuiDocumentBeginTransaction(temporary, NULL, &transaction);
    if (result != XUI_OK) goto done;
    root = doc_index_get(snapshot->state->index, DOC_ROOT);
    for (i = 0; i < doc_seq_size(root->children) && result == XUI_OK; i++)
        result = doc_copy_subtree_apply(transaction, snapshot->state,
            doc_seq_get_id(root->children, i), DOC_ROOT, DOC_NONE, &copied);
    if (result == XUI_OK)
        result = doc_md_strip_presentation(transaction, DOC_ROOT, dialect, accepted_reasons);
    if (result == XUI_OK)
        result = doc_snapshot_create(transaction->draft, temporary->identity,
            temporary->revision, &normalized);
    if (result == XUI_OK)
        result = xuiDocumentSnapshotConvertToMarkdown(normalized, dialect, out);
done:
    xuiDocumentSnapshotRelease(normalized);
    xuiDocumentTxnRelease(transaction);
    xuiDocumentRelease(temporary);
    return result;
}

XUI_API int xuiDocumentSnapshotConvertToMarkdown(
    xui_document_snapshot snapshot, uint32_t dialect, xui_document* out)
{
    xui_doc_desc_t desc = {0}; xui_document target = NULL;
    xui_document_transaction transaction = NULL;
    xui_document_snapshot saved = NULL;
    struct xui_doc_transaction_t shadow;
    doc_md_loss_scan scan = {0};
    doc_node* root;
    uint64_t i, copied;
    int result;
    if (out) *out = NULL;
    if (!out || !doc_md_conversion_input(snapshot, dialect)) return XUI_ERROR_INVALID_ARGUMENT;
    scan.state = snapshot->state; scan.dialect = dialect;
    doc_md_loss_walk(&scan, DOC_ROOT);
    if (scan.count) return XUI_DOC_ERROR_UNREPRESENTABLE;

    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = dialect; desc.bDisableHistory = 1;
    desc.iMaxTextBytes = snapshot->state->allocator->max_bytes;
    desc.iMaxNodes = snapshot->state->allocator->max_nodes;
    result = xuiDocumentCreate(&desc, &target);
    if (result != XUI_OK) return result;
    result = xuiDocumentBeginTransaction(target, NULL, &transaction);
    if (result != XUI_OK) goto done;
    result = doc_markdown_shadow_begin(transaction, &shadow);
    if (result != XUI_OK) goto done;
    root = doc_index_get(snapshot->state->index, DOC_ROOT);
    for (i = 0; i < doc_seq_size(root->children) && result == XUI_OK; i++)
        result = doc_copy_subtree_apply(&shadow, snapshot->state,
            doc_seq_get_id(root->children, i), DOC_ROOT, DOC_NONE, &copied);
    result = doc_markdown_shadow_end(transaction, &shadow, result, NULL, NULL);
    if (result == XUI_OK && !doc_semantic_equal(snapshot->state, transaction->draft))
        result = XUI_DOC_ERROR_UNREPRESENTABLE;
    if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, NULL);
    if (result == XUI_ERROR_UNSUPPORTED)
        result = XUI_DOC_ERROR_UNREPRESENTABLE;
    if (result != XUI_OK) goto done;
    result = xuiDocumentAcquireSnapshot(target, &saved);
    if (result == XUI_OK) result = xuiDocumentMarkSaved(target, saved);
    if (result == XUI_OK) {
        target->disable_history = 0;
        doc_memory_standby(target->state, 1); target->standby_history = 1;
        *out = target; target = NULL;
    }
done:
    xuiDocumentSnapshotRelease(saved);
    xuiDocumentTxnRelease(transaction);
    xuiDocumentRelease(target);
    return result;
}

XUI_API int xuiDocumentSnapshotAnalyzeMarkdownConversion(
    xui_document_snapshot snapshot, uint32_t dialect,
    xui_doc_markdown_loss_t* losses, uint64_t capacity, uint64_t* total)
{
    doc_md_loss_scan scan = {0};
    xui_document converted = NULL;
    int result;
    if (total) *total = 0;
    if (!total || (!losses && capacity) || !doc_md_conversion_input(snapshot, dialect))
        return XUI_ERROR_INVALID_ARGUMENT;
    scan.state = snapshot->state; scan.dialect = dialect;
    scan.items = losses; scan.capacity = capacity;
    doc_md_loss_walk(&scan, DOC_ROOT);
    if (!scan.count) {
        result = xuiDocumentSnapshotConvertToMarkdown(snapshot, dialect, &converted);
        xuiDocumentRelease(converted);
        if (result == XUI_DOC_ERROR_UNREPRESENTABLE) {
            doc_md_loss_append(&scan, doc_index_get(scan.state->index, DOC_ROOT),
                XUI_DOC_MD_LOSS_ROUNDTRIP);
        } else if (result != XUI_OK) return result;
    }
    *total = scan.count;
    return capacity < scan.count && losses ? XUI_ERROR_BUFFER_TOO_SMALL : XUI_OK;
}

XUI_API int xuiDocumentSnapshotAnalyzeRichConversion(
    xui_document_snapshot snapshot, xui_doc_rich_conversion_report_t* report)
{
    doc_state* state;
    uint64_t bytes, references, syntax;
    if (!snapshot || !report || report->iSize != sizeof(*report) ||
        snapshot->state->profile != XUI_DOCUMENT_MARKDOWN)
        return XUI_ERROR_INVALID_ARGUMENT;
    state = snapshot->state;
    bytes = doc_seq_size(state->source);
    references = doc_seq_size(state->references) / sizeof(xui_doc_reference_definition_t);
    syntax = doc_seq_size(state->inline_syntax) / sizeof(xui_doc_inline_syntax_t);
    *report = (xui_doc_rich_conversion_report_t){0};
    report->iSize = sizeof(*report);
    report->iSourceBytes = bytes;
    report->iReferenceDefinitions = references;
    report->iInlineSyntaxEntries = syntax;
    if (bytes) report->iReasons |= XUI_DOC_RICH_LOSS_SOURCE_SYNTAX;
    if (references) report->iReasons |= XUI_DOC_RICH_LOSS_REFERENCE_DEFINITIONS;
    return XUI_OK;
}

XUI_API int xuiDocumentSnapshotConvertToRich(
    xui_document_snapshot snapshot, uint32_t accepted_reasons, xui_document* out)
{
    xui_doc_rich_conversion_report_t report = {0};
    xui_doc_desc_t desc = {0};
    xui_document target = NULL;
    xui_document_transaction transaction = NULL;
    xui_document_snapshot saved = NULL;
    doc_state* comparable = NULL;
    doc_node* root;
    uint64_t i, copied;
    int result;
    if (out) *out = NULL;
    if (!out || (accepted_reasons & ~(XUI_DOC_RICH_LOSS_SOURCE_SYNTAX |
        XUI_DOC_RICH_LOSS_REFERENCE_DEFINITIONS))) return XUI_ERROR_INVALID_ARGUMENT;
    report.iSize = sizeof(report);
    result = xuiDocumentSnapshotAnalyzeRichConversion(snapshot, &report);
    if (result != XUI_OK) return result;
    if (report.iReasons & ~accepted_reasons) return XUI_DOC_ERROR_UNREPRESENTABLE;

    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_RICH;
    desc.bDisableHistory = 1;
    desc.iMaxTextBytes = snapshot->state->allocator->max_bytes;
    desc.iMaxNodes = snapshot->state->allocator->max_nodes;
    result = xuiDocumentCreate(&desc, &target);
    if (result != XUI_OK) return result;
    result = xuiDocumentBeginTransaction(target, NULL, &transaction);
    if (result != XUI_OK) goto done;
    root = doc_index_get(snapshot->state->index, DOC_ROOT);
    for (i = 0; i < doc_seq_size(root->children) && result == XUI_OK; i++)
        result = doc_copy_subtree_apply(transaction, snapshot->state,
            doc_seq_get_id(root->children, i), DOC_ROOT, DOC_NONE, &copied);
    if (result == XUI_OK) {
        comparable = doc_state_clone(transaction->draft);
        if (!comparable) result = XUI_ERROR_OUT_OF_MEMORY;
        else {
            comparable->profile = XUI_DOCUMENT_MARKDOWN;
            if (!doc_semantic_equal(snapshot->state, comparable))
                result = XUI_DOC_ERROR_UNREPRESENTABLE;
        }
    }
    doc_state_release(comparable); comparable = NULL;
    if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, NULL);
    if (result != XUI_OK) goto done;
    result = xuiDocumentAcquireSnapshot(target, &saved);
    if (result == XUI_OK) result = xuiDocumentMarkSaved(target, saved);
    if (result == XUI_OK) {
        target->disable_history = 0;
        doc_memory_standby(target->state, 1); target->standby_history = 1;
        *out = target; target = NULL;
    }
done:
    doc_state_release(comparable);
    xuiDocumentSnapshotRelease(saved);
    xuiDocumentTxnRelease(transaction);
    xuiDocumentRelease(target);
    return result;
}

#endif
