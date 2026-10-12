XUI_API int xuiDocumentPrepareSemanticPosition(xui_document_prepare prepare,
    uint64_t node, uint32_t kind, uint64_t offset, uint32_t affinity,
    xui_doc_position_t* output)
{
    xui_doc_position_t position = {0}; doc_node* target;
    if (!prepare || !output || affinity > XUI_DOC_AFTER ||
        (kind != XUI_DOC_POSITION_TEXT && kind != XUI_DOC_POSITION_GAP)) return XUI_ERROR_INVALID_ARGUMENT;
    if (!prepare->visual_expected) return XUI_ERROR_UNSUPPORTED;
    target = doc_index_get(prepare->visual_expected->index, node);
    if (!target) return XUI_ERROR_NOT_FOUND;
    if (kind == XUI_DOC_POSITION_TEXT && doc_text_kind(target->kind) &&
        offset <= doc_seq_size(target->text) && !doc_seq_boundary(target->text, offset)) return XUI_DOC_ERROR_UTF8;
    position.iSize = sizeof(position); position.iDocumentId = prepare->identity;
    position.iRevision = prepare->transaction.base_revision; position.iNodeId = node;
    position.iKind = kind; position.iOffset = offset; position.iAffinity = affinity;
    if (!doc_position_valid(prepare->visual_expected, &position)) return XUI_ERROR_INVALID_ARGUMENT;
    position.iInputGeneration = prepare->generation; *output = position; return XUI_OK;
}
XUI_API int xuiDocumentPrepareMapPosition(xui_document_prepare from,
    xui_document_prepare to, const xui_doc_position_t* position,
    xui_doc_position_t* output, int* mapping)
{
    xui_document_prepare basis = from ? from : to, descendant;
    struct xui_doc_change_set_t change = {0}; xui_doc_position_t input, mapped;
    uint64_t source_prefix = 0, first = 0, bytes; int result, quality, reverse;
    if (!basis || !position || !output || !mapping || position->iSize != sizeof(*position))
        return XUI_ERROR_INVALID_ARGUMENT;
    if (position->iKind == XUI_DOC_POSITION_SOURCE)
        return xuiDocumentPrepareMapSourcePosition(from, to, position, output, mapping);
    if (position->iDocumentId != basis->identity || position->iRevision != basis->transaction.base_revision ||
        position->iInputGeneration != (from ? from->generation : 0)) return XUI_DOC_ERROR_STALE;
    if ((from && !from->visual_expected) || (to && !to->visual_expected)) return XUI_ERROR_UNSUPPORTED;
    if (from && to && (from->identity != to->identity ||
        from->transaction.base_revision != to->transaction.base_revision ||
        from->transaction.base != to->transaction.base)) return XUI_DOC_ERROR_STALE;
    reverse = !to || (from && from->generation > to->generation);
    descendant = reverse ? from : to;
    if (from && to) {
        xui_document_prepare ancestor = reverse ? to : from;
        result = doc_prepare_lineage_prefix(descendant, ancestor, &source_prefix);
        if (result != XUI_OK) return result;
        first = doc_seq_size(ancestor->visual_operations);
    }
    bytes = doc_seq_size(descendant->visual_operations);
    if (first > bytes || first % sizeof(xui_doc_operation_t) || bytes % sizeof(xui_doc_operation_t))
        return XUI_ERROR_INVALID_STATE;
    change.before = from ? from->visual_expected : basis->transaction.base;
    change.after = to ? to->visual_expected : basis->transaction.base;
    change.identity = basis->identity;
    change.before_revision = change.after_revision = basis->transaction.base_revision;
    change.domain = XUI_DOC_SEMANTIC; change.undo = reverse;
    change.packed_ops = descendant->visual_operations; change.packed_op_start = first;
    change.count = (bytes - first) / sizeof(xui_doc_operation_t);
    input = *position; input.iInputGeneration = 0;
    result = xuiDocumentMapPosition(&change, &input, &mapped, &quality);
    if (result != XUI_OK) return result;
    mapped.iInputGeneration = to ? to->generation : 0;
    *output = mapped; *mapping = quality; return XUI_OK;
}
