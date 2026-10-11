/* Read only immutable source/log/lineage roots. The worker's transaction
 * draft and flattened ops are never consulted by input-position queries. */
typedef struct doc_input_generation {
    uint64_t generation, operation_bytes, source_bytes;
} doc_input_generation;
static int doc_prepare_lineage_append(xui_document_prepare prepare)
{
    doc_input_generation record = {prepare->generation, doc_seq_size(prepare->operations), doc_seq_size(prepare->source)};
    doc_sequence *insert, *lineage = NULL; uint64_t bytes = doc_seq_size(prepare->input_lineage); int result;
    if (bytes > SIZE_MAX - sizeof(record)) return XUI_DOC_ERROR_LIMIT;
    insert = doc_seq_text(prepare->allocator, (const char*)&record, sizeof(record));
    if (!insert) return XUI_ERROR_OUT_OF_MEMORY;
    result = doc_seq_replace(prepare->allocator, prepare->input_lineage, bytes, bytes, insert, &lineage);
    doc_seq_release(insert);
    if (result == XUI_OK) { doc_seq_release(prepare->input_lineage); prepare->input_lineage = lineage; }
    return result;
}
static int doc_prepare_lineage_prefix(xui_document_prepare descendant,
    xui_document_prepare ancestor, uint64_t* prefix)
{
    uint64_t low = 0, high = doc_seq_size(descendant->input_lineage) / sizeof(doc_input_generation);
    while (low < high) {
        uint64_t middle = low + (high - low) / 2; doc_input_generation record;
        int result = doc_seq_read(descendant->input_lineage, middle * sizeof(record), &record, sizeof(record));
        if (result != XUI_OK) return result;
        if (record.generation < ancestor->generation) low = middle + 1;
        else if (record.generation > ancestor->generation) high = middle;
        else {
            if (record.operation_bytes != doc_seq_size(ancestor->operations) ||
                record.source_bytes != doc_seq_size(ancestor->source)) return XUI_DOC_ERROR_STALE;
            *prefix = record.operation_bytes; return XUI_OK;
        }
    }
    return XUI_DOC_ERROR_STALE;
}
XUI_API int xuiDocumentPrepareMapSourcePosition(xui_document_prepare from,
    xui_document_prepare to, const xui_doc_position_t* position,
    xui_doc_position_t* output, int* mapping)
{
    xui_document_prepare basis = from ? from : to, descendant;
    doc_sequence *source, *target; xui_doc_position_t mapped; uint64_t prefix = 0, cursor, bytes;
    int result, deleted = 0, reverse;
    if (!basis || !position || !output || !mapping || position->iSize != sizeof(*position) ||
        position->iKind != XUI_DOC_POSITION_SOURCE || position->iNodeId != DOC_ROOT ||
        position->iAffinity > XUI_DOC_AFTER) return XUI_ERROR_INVALID_ARGUMENT;
    if (position->iDocumentId != basis->identity || position->iRevision != basis->transaction.base_revision ||
        position->iInputGeneration != (from ? from->generation : 0)) return XUI_DOC_ERROR_STALE;
    if (from && to && (from->identity != to->identity || from->transaction.base_revision != to->transaction.base_revision ||
        from->transaction.base != to->transaction.base)) return XUI_DOC_ERROR_STALE;
    source = from ? from->source : basis->transaction.base->source;
    target = to ? to->source : basis->transaction.base->source;
    if (position->iOffset > doc_seq_size(source)) return XUI_ERROR_INVALID_ARGUMENT;
    if (!doc_seq_boundary(source, position->iOffset)) return XUI_DOC_ERROR_UTF8;
    reverse = !to || (from && from->generation > to->generation);
    descendant = reverse ? from : to;
    if (from && to) {
        result = doc_prepare_lineage_prefix(descendant, reverse ? to : from, &prefix);
        if (result != XUI_OK) return result;
    }
    bytes = doc_seq_size(descendant->operations);
    if (prefix > bytes || prefix % sizeof(xui_doc_operation_t) || bytes % sizeof(xui_doc_operation_t))
        return XUI_ERROR_INVALID_STATE;
    mapped = *position;
    for (cursor = reverse ? bytes : prefix; reverse ? cursor > prefix : cursor < bytes;) {
        xui_doc_operation_t operation;
        if (reverse) cursor -= sizeof(operation);
        result = doc_seq_read(descendant->operations, cursor, &operation, sizeof(operation));
        if (result != XUI_OK) return result;
        if (operation.iKind != XUI_DOC_OP_SOURCE) return XUI_ERROR_INVALID_STATE;
        mapped.iOffset = doc_map_offset(mapped.iOffset, mapped.iAffinity, operation.iOffset,
            reverse ? operation.iNewLength : operation.iOldLength,
            reverse ? operation.iOldLength : operation.iNewLength, &deleted);
        if (!reverse) cursor += sizeof(operation);
    }
    if (mapped.iOffset > doc_seq_size(target) || !doc_seq_boundary(target, mapped.iOffset)) return XUI_ERROR_INVALID_STATE;
    mapped.iInputGeneration = to ? to->generation : 0;
    *output = mapped; *mapping = deleted ? XUI_DOC_MAP_DELETED : XUI_DOC_MAP_EXACT;
    return XUI_OK;
}
