/* The worker has proved semantic equivalence. Walk the same normalization as
 * doc_semantic_node_mode: empty text/Markdown paragraphs vanish and adjacent
 * equal-style text may be repartitioned. No plain-text ordinal, source guess,
 * writable document, allocation or worker draft is involved in a query. */
static int doc_semantic_map_ignored(doc_state* state, const doc_node* node)
{
    return node && ((node->kind == XUI_DOC_TEXT && !doc_seq_size(node->text)) ||
        (state->profile == XUI_DOCUMENT_MARKDOWN && doc_semantic_empty_paragraph(state, node)));
}
static int doc_semantic_map_cursor(doc_state* state, doc_node* parent,
    uint64_t child, uint64_t offset, const xui_doc_position_t* input,
    xui_doc_position_t* output, int* quality)
{
    *output = *input;
    if (offset) {
        doc_node* node = doc_index_get(state->index, doc_seq_get_id(parent->children, child));
        if (!node || node->kind != XUI_DOC_TEXT || !doc_seq_boundary(node->text, offset))
            return XUI_DOC_ERROR_SCHEMA;
        output->iKind = XUI_DOC_POSITION_TEXT; output->iNodeId = node->id;
        output->iOffset = offset; *quality = XUI_DOC_MAP_COLLAPSED;
    } else {
        output->iKind = XUI_DOC_POSITION_GAP; output->iNodeId = parent->id;
        output->iOffset = child;
    }
    return XUI_OK;
}
static int doc_semantic_map_node(doc_state* a, doc_node* an, doc_state* b,
    doc_node* bn, const xui_doc_position_t* input, xui_doc_position_t* output,
    int* quality, unsigned depth)
{
    uint64_t ai = 0, bi = 0, ao = 0, bo = 0;
    uint64_t ac = doc_seq_size(an->children), bc = doc_seq_size(bn->children), wanted = input->iNodeId;
    doc_node* branch;
    if (depth >= DOC_MAX_DEPTH || an->kind != bn->kind) return XUI_DOC_ERROR_SCHEMA;
    if (an->id == input->iNodeId && input->iKind == XUI_DOC_POSITION_TEXT) {
        *output = *input; output->iNodeId = bn->id;
        return doc_seq_boundary(bn->text, output->iOffset) ? XUI_OK : XUI_DOC_ERROR_SCHEMA;
    }
    if (an->id != wanted) {
        branch = doc_index_get(a->index, wanted);
        while (branch && branch->parent != an->id && branch->id != DOC_ROOT)
            branch = doc_index_get(a->index, branch->parent);
        if (!branch || branch->parent != an->id) return XUI_ERROR_NOT_FOUND;
        wanted = branch->id;
    }
    while (ai < ac || bi < bc) {
        doc_node *left = ai < ac ? doc_index_get(a->index, doc_seq_get_id(an->children, ai)) : NULL;
        doc_node *right = bi < bc ? doc_index_get(b->index, doc_seq_get_id(bn->children, bi)) : NULL;
        if (an->id == input->iNodeId && input->iKind == XUI_DOC_POSITION_GAP &&
            ai == input->iOffset && !ao) {
            /* AFTER passes normalized-away target children at this boundary. */
            if (input->iAffinity == XUI_DOC_AFTER && !bo) {
                while (right && doc_semantic_map_ignored(b, right)) {
                    bi++; right = bi < bc ? doc_index_get(b->index, doc_seq_get_id(bn->children, bi)) : NULL;
                    *quality = XUI_DOC_MAP_COLLAPSED;
                }
            }
            return doc_semantic_map_cursor(b, bn, bi, bo, input, output, quality);
        }
        if (doc_semantic_map_ignored(a, left)) { ai++; continue; }
        if (doc_semantic_map_ignored(b, right)) { bi++; continue; }
        if (!left || !right) return XUI_DOC_ERROR_SCHEMA;
        if (left->kind == XUI_DOC_TEXT && right->kind == XUI_DOC_TEXT) {
            uint64_t l = doc_seq_size(left->text) - ao, r = doc_seq_size(right->text) - bo;
            uint64_t n = l < r ? l : r;
            if (left->id == wanted && input->iKind == XUI_DOC_POSITION_TEXT &&
                input->iOffset >= ao && input->iOffset <= ao + n &&
                (input->iOffset < ao + n || input->iAffinity == XUI_DOC_BEFORE ||
                    ao + n == doc_seq_size(left->text))) {
                *output = *input; output->iNodeId = right->id;
                output->iOffset = bo + input->iOffset - ao; return XUI_OK;
            }
            ao += n; bo += n;
            if (ao == doc_seq_size(left->text)) { ai++; ao = 0; }
            if (bo == doc_seq_size(right->text)) { bi++; bo = 0; }
        } else {
            if (ao || bo || left->kind != right->kind) return XUI_DOC_ERROR_SCHEMA;
            if (left->id == wanted) return doc_semantic_map_node(a, left, b, right,
                input, output, quality, depth + 1);
            ai++; bi++;
        }
    }
    if (an->id == input->iNodeId && input->iKind == XUI_DOC_POSITION_GAP && input->iOffset == ac)
        return doc_semantic_map_cursor(b, bn, bc, 0, input, output, quality);
    return XUI_ERROR_NOT_FOUND;
}
int doc_semantic_map_position(doc_state* a, doc_state* b,
    const xui_doc_position_t* input, xui_doc_position_t* output, int* mapping)
{
    xui_doc_position_t normalized, mapped; doc_node* node; unsigned depth = 0;
    int quality = XUI_DOC_MAP_EXACT, result;
    if (!a || !b || !input || !output || !mapping || input->iKind == XUI_DOC_POSITION_SOURCE ||
        !doc_position_valid(a, input)) return XUI_ERROR_INVALID_ARGUMENT;
    normalized = *input; node = doc_index_get(a->index, input->iNodeId);
    /* A skipped ancestor has no structural counterpart; its positions
     * collapse to the corresponding parent boundary, respecting affinity. */
    while (node && node->id != DOC_ROOT && depth++ < DOC_MAX_DEPTH) {
        if (doc_semantic_map_ignored(a, node)) {
            doc_node* parent = doc_index_get(a->index, node->parent);
            if (!parent) return XUI_DOC_ERROR_SCHEMA;
            normalized.iNodeId = parent->id; normalized.iKind = XUI_DOC_POSITION_GAP;
            normalized.iOffset = doc_child_index(parent, node->id) + (input->iAffinity == XUI_DOC_AFTER);
            quality = XUI_DOC_MAP_COLLAPSED;
        }
        node = doc_index_get(a->index, node->parent);
    }
    result = doc_semantic_map_node(a, doc_index_get(a->index, DOC_ROOT), b,
        doc_index_get(b->index, DOC_ROOT), &normalized, &mapped, &quality, 0);
    if (result != XUI_OK) return result;
    if (!doc_position_valid(b, &mapped)) return XUI_DOC_ERROR_SCHEMA;
    *output = mapped; *mapping = quality; return XUI_OK;
}
