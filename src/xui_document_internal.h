#ifndef XUI_DOCUMENT_INTERNAL_H
#define XUI_DOCUMENT_INTERNAL_H
#include "../xui_document.h"
#include <stdatomic.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define DOC_NONE UINT64_MAX
#define DOC_ROOT UINT64_C(1)
#define DOC_INDEX_BITS 4
#define DOC_INDEX_SIZE 16
#define DOC_MAX_DEPTH 128

typedef struct doc_allocator {
    atomic_uint refs;
    xui_doc_alloc_proc alloc;
    xui_doc_free_proc free;
    void* user;
    atomic_uint_fast64_t live, peak, allocations, sequence;
    atomic_uint_fast64_t markdown_parses, markdown_parsed_bytes;
} doc_allocator;
typedef struct doc_blob {
    atomic_uint refs;
    uint64_t size;
    char data[];
} doc_blob;
/* Persistent implicit treap: item lengths are bytes for text or 1 for IDs. */
typedef struct doc_sequence {
    atomic_uint refs;
    uint64_t priority, length, total;
    struct doc_sequence *left, *right;
    doc_blob* blob;
    uint64_t offset;
    xui_doc_node_id id;
} doc_sequence;
typedef struct doc_node {
    atomic_uint refs;
    xui_doc_node_id id, parent;
    uint32_t kind;
    xui_doc_attributes_t attrs;
    doc_sequence *text, *children;
    doc_blob *resource, *info, *title;
    uint64_t source_start, source_end;
    int source_exact;
} doc_node;
typedef struct doc_index {
    atomic_uint refs;
    unsigned level;
    void* slots[DOC_INDEX_SIZE];
} doc_index;
typedef struct doc_state {
    atomic_uint refs;
    doc_allocator* allocator;
    doc_index* index;
    doc_sequence* source;
    uint64_t node_count, text_bytes, content_id;
    uint32_t profile, dialect;
} doc_state;
typedef struct doc_observer {
    uint64_t token;
    xui_doc_change_proc callback;
    void* user;
    struct doc_observer* next;
    int removed;
} doc_observer;
typedef struct doc_history {
    doc_state *before, *after;
    xui_doc_operation_t* ops;
    uint64_t count, origin, group;
    uint32_t flags;
    struct doc_history* next;
} doc_history;
struct xui_document_t {
    atomic_uint refs;
    doc_allocator* allocator;
    doc_state* state;
    uint64_t identity, revision, next_id, next_state, next_observer, saved_state;
    uint64_t max_bytes, max_nodes;
    unsigned history_limit, undo_count, redo_count;
    int disable_history, notifying;
    struct xui_doc_transaction_t* writer;
    doc_history *undo, *redo;
    doc_observer* observers;
};
struct xui_doc_snapshot_t {
    atomic_uint refs;
    doc_state* state;
    uint64_t identity, revision;
};
struct xui_doc_transaction_t {
    xui_document document;
    doc_state *base, *draft;
    uint64_t base_revision, origin, group;
    uint32_t domain, flags;
    xui_doc_operation_t* ops;
    uint64_t count, capacity, parse_op_start;
    int error, closed, parsing;
};
struct xui_doc_change_set_t {
    atomic_uint refs;
    doc_state *before, *after;
    uint64_t identity, before_revision, after_revision, origin;
    uint32_t flags;
    xui_doc_operation_t* ops;
    uint64_t count;
    int undo;
};

void* doc_alloc(doc_allocator*, size_t);
void* doc_realloc(doc_allocator*, void*, size_t);
void doc_free(void*);
void doc_allocator_retain(doc_allocator*);
void doc_allocator_release(doc_allocator*);
doc_blob* doc_blob_new(doc_allocator*, const char*, uint64_t);
void doc_blob_retain(doc_blob*);
void doc_blob_release(doc_blob*);
const char* doc_string(doc_blob*);
int doc_utf8(const char*, uint64_t);
uint64_t doc_seq_size(doc_sequence*);
void doc_seq_retain(doc_sequence*);
void doc_seq_release(doc_sequence*);
doc_sequence* doc_seq_text(doc_allocator*, const char*, uint64_t);
doc_sequence* doc_seq_id(doc_allocator*, uint64_t);
int doc_seq_replace(doc_allocator*, doc_sequence*, uint64_t, uint64_t, doc_sequence*, doc_sequence**);
int doc_seq_read(doc_sequence*, uint64_t, void*, uint64_t);
uint64_t doc_seq_get_id(doc_sequence*, uint64_t);
int doc_seq_boundary(doc_sequence*, uint64_t);
int doc_seq_equal_bytes(doc_sequence*, uint64_t, const char*, uint64_t);
char* doc_seq_string(doc_allocator*, doc_sequence*);
doc_node* doc_node_new(doc_allocator*, uint64_t, uint64_t, const xui_doc_node_desc_t*);
doc_node* doc_node_clone(doc_allocator*, const doc_node*);
void doc_node_retain(doc_node*);
void doc_node_release(doc_node*);
doc_node* doc_index_get(doc_index*, uint64_t);
int doc_index_set(doc_allocator*, doc_index*, uint64_t, doc_node*, doc_index**);
void doc_index_retain(doc_index*);
void doc_index_release(doc_index*);
doc_state* doc_state_new(doc_allocator*, uint32_t);
doc_state* doc_state_clone(doc_state*);
void doc_state_retain(doc_state*);
void doc_state_release(doc_state*);
int doc_state_set(doc_state*, doc_node*);
int doc_state_remove(doc_state*, uint64_t);
int doc_node_info(const doc_node*, xui_doc_node_info_t*);
int doc_schema_child(uint32_t, uint32_t);
int doc_schema_attrs(uint32_t, const xui_doc_attributes_t*);
int doc_attributes_equal(const xui_doc_attributes_t*, const xui_doc_attributes_t*);
int doc_schema_validate(doc_state*);
int doc_schema_validate_transaction(xui_document_transaction);
int doc_schema_depth(doc_state*, uint64_t, unsigned);
int doc_text_kind(uint32_t);
int doc_inline_kind(uint32_t);
int doc_txn_fail(xui_document_transaction, int);
int doc_txn_op(xui_document_transaction, const xui_doc_operation_t*);
int doc_txn_check(xui_document_transaction, int);
int doc_txn_insert(xui_document_transaction, uint64_t, uint64_t, const xui_doc_node_desc_t*, uint64_t*);
int doc_txn_delete(xui_document_transaction, uint64_t);
int doc_txn_text(xui_document_transaction, uint64_t, uint64_t, uint64_t, const char*, uint64_t);
uint64_t doc_child_index(const doc_node*, uint64_t);
int doc_markdown_parse(xui_document_transaction);
int doc_markdown_reconcile(xui_document_transaction, doc_state**);
int doc_markdown_text(xui_document_transaction, uint64_t, uint64_t, uint64_t, const char*, uint64_t);
int doc_markdown_attributes(xui_document_transaction, uint64_t, const xui_doc_attributes_t*);
int doc_markdown_marks(xui_document_transaction, const xui_doc_range_t*, uint32_t, uint32_t);
int doc_markdown_range(xui_document_transaction, const xui_doc_range_t*, const char*, uint64_t, xui_doc_position_t*);
int doc_markdown_apply_paragraphs(xui_document_transaction, doc_state*, const xui_doc_position_t*, xui_doc_position_t*);
int doc_txn_source(xui_document_transaction, uint64_t, uint64_t, const char*, uint64_t);
int doc_txn_source_patch(xui_document_transaction, uint64_t, uint64_t, const char*, uint64_t, int);
int doc_position_valid(doc_state*, const xui_doc_position_t*);
int doc_position_compare(doc_state*, const xui_doc_position_t*, const xui_doc_position_t*, int*);
int doc_marks_range(xui_document_transaction, const xui_doc_range_t*, uint32_t, uint32_t);
typedef struct doc_plain_span {
    uint64_t start, length;
    xui_doc_position_t first, last;
    int literal;
} doc_plain_span;
typedef struct doc_plain_projection {
    doc_allocator* allocator;
    doc_state* state;
    char* text;
    uint64_t bytes, capacity, count, span_capacity;
    doc_plain_span* spans;
    xui_doc_position_t origin;
    int error;
} doc_plain_projection;
int doc_plain_project(xui_document_snapshot, uint32_t, doc_plain_projection*);
void doc_plain_projection_free(doc_plain_projection*);
int doc_plain_project_position(const doc_plain_projection*, const xui_doc_position_t*, uint64_t*);
xui_doc_position_t doc_plain_unproject(const doc_plain_projection*, uint64_t, int);
int doc_split_text(xui_document_transaction, uint64_t, uint64_t, uint64_t*);
int doc_semantic_equal(doc_state*, doc_state*);
int doc_semantic_subtree_equal(doc_state*, uint64_t, doc_state*, uint64_t);
int doc_semantic_empty_paragraph(doc_state*, const doc_node*);

#endif
