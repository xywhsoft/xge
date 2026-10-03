#include "../xui_config.h"
#if XUI_ENABLE_DOCUMENT
#include "xui_document_internal.h"

static int doc_semantic_attrs(const doc_node* a, const doc_node* b)
{
    uint64_t payload_a = a->extension_payload ? a->extension_payload->size : 0;
    uint64_t payload_b = b->extension_payload ? b->extension_payload->size : 0;
    return doc_attributes_equal(a->attrs, b->attrs) && !strcmp(doc_string(a->resource), doc_string(b->resource)) &&
        !strcmp(doc_string(a->info), doc_string(b->info)) && !strcmp(doc_string(a->title), doc_string(b->title)) &&
        !strcmp(doc_string(a->link_target), doc_string(b->link_target)) &&
        !strcmp(doc_string(a->link_title), doc_string(b->link_title)) &&
        a->extension_version == b->extension_version && a->extension_required == b->extension_required &&
        payload_a == payload_b && (!payload_a || !memcmp(a->extension_payload->data,
            b->extension_payload->data, (size_t)payload_a));
}
static int doc_semantic_text(doc_sequence* a, uint64_t ao, doc_sequence* b, uint64_t bo, uint64_t bytes)
{
    char left[512], right[512]; uint64_t offset = 0;
    while (offset < bytes) {
        uint64_t n = bytes - offset < sizeof(left) ? bytes - offset : sizeof(left);
        if (doc_seq_read(a, ao + offset, left, n) != XUI_OK || doc_seq_read(b, bo + offset, right, n) != XUI_OK || memcmp(left, right, (size_t)n)) return 0;
        offset += n;
    }
    return 1;
}
int doc_semantic_empty_paragraph(doc_state* state, const doc_node* n)
{
    uint64_t i;
    if (!n || n->kind != XUI_DOC_PARAGRAPH) return 0;
    for (i = 0; i < doc_seq_size(n->children); i++) {
        doc_node* child = doc_index_get(state->index, doc_seq_get_id(n->children, i));
        if (child->kind != XUI_DOC_TEXT || doc_seq_size(child->text)) return 0;
    }
    return 1;
}
static int doc_semantic_node(doc_state* a, doc_node* an, doc_state* b, doc_node* bn)
{
    uint64_t ai = 0, bi = 0, ao = 0, bo = 0, ac = doc_seq_size(an->children), bc = doc_seq_size(bn->children);
    if (an->kind != bn->kind || !doc_semantic_attrs(an, bn) || doc_seq_size(an->text) != doc_seq_size(bn->text) ||
        !doc_semantic_text(an->text, 0, bn->text, 0, doc_seq_size(an->text))) return 0;
    while (ai < ac || bi < bc) {
        doc_node *left = ai < ac ? doc_index_get(a->index, doc_seq_get_id(an->children, ai)) : NULL;
        doc_node *right = bi < bc ? doc_index_get(b->index, doc_seq_get_id(bn->children, bi)) : NULL;
        if (left && left->kind == XUI_DOC_TEXT && !doc_seq_size(left->text)) { ai++; continue; }
        if (right && right->kind == XUI_DOC_TEXT && !doc_seq_size(right->text)) { bi++; continue; }
        if (a->profile == XUI_DOCUMENT_MARKDOWN && doc_semantic_empty_paragraph(a, left)) { ai++; continue; }
        if (b->profile == XUI_DOCUMENT_MARKDOWN && doc_semantic_empty_paragraph(b, right)) { bi++; continue; }
        if (!left || !right) return 0;
        if (left->kind == XUI_DOC_TEXT && right->kind == XUI_DOC_TEXT) {
            uint64_t l = doc_seq_size(left->text) - ao, r = doc_seq_size(right->text) - bo, n = l < r ? l : r;
            if (!doc_semantic_attrs(left, right) || !doc_semantic_text(left->text, ao, right->text, bo, n)) return 0;
            ao += n; bo += n;
            if (ao == doc_seq_size(left->text)) { ai++; ao = 0; }
            if (bo == doc_seq_size(right->text)) { bi++; bo = 0; }
        } else {
            if (ao || bo || !doc_semantic_node(a, left, b, right)) return 0;
            ai++; bi++;
        }
    }
    return !ao && !bo;
}
int doc_semantic_subtree_equal(doc_state* a, uint64_t an, doc_state* b, uint64_t bn)
{
    doc_node *left = doc_index_get(a->index, an), *right = doc_index_get(b->index, bn);
    return left && right && doc_semantic_node(a, left, b, right);
}
int doc_semantic_equal(doc_state* a, doc_state* b)
{
    return doc_semantic_node(a, doc_index_get(a->index, DOC_ROOT), b, doc_index_get(b->index, DOC_ROOT));
}

#endif
