#ifndef XUI_DOCUMENT_INTERNAL_H
#define XUI_DOCUMENT_INTERNAL_H
#include "../xui_document.h"
#include <stdatomic.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "xge_text_input.h"

static inline const char* doc_language(const char* language)
{ return language ? language : ""; }
static inline int doc_language_valid(const char* language)
{ return !language || !*language || __xgeTextLanguageValid(language); }
static inline int doc_language_equal(const char* a, const char* b)
{
    a = doc_language(a); b = doc_language(b);
    while (*a && *b && __xgeLanguageLower((unsigned char)*a) ==
        __xgeLanguageLower((unsigned char)*b)) { a++; b++; }
    return !*a && !*b;
}

int doc_text_style_valid(uint32_t fields, const xui_doc_text_style_t* style);

#define DOC_NONE UINT64_MAX
#define DOC_ROOT UINT64_C(1)
#define DOC_INDEX_BITS 4
#define DOC_INDEX_SIZE 16
#define DOC_MAX_DEPTH 128
#define DOC_MAX_LAYOUT_VALUE 1000000.0f
#define DOC_BUILD_PARSE 1
#define DOC_BUILD_SEMANTIC 2
#define DOC_DEFAULT_HISTORY_BYTES (UINT64_C(64) * 1024 * 1024)
#define DOC_ATTR_BUCKETS 256

typedef struct doc_attribute {
    atomic_uint refs;
    struct doc_attribute *left, *right, *next; /* AVL bucket tree; equal hashes chain. */
    uint64_t hash;
    unsigned height;
    xui_doc_attributes_t value;
    char language[]; /* Canonical lower-case, owned by this interned entry. */
} doc_attribute;

typedef struct doc_allocator {
    atomic_uint refs;
    xui_doc_alloc_proc alloc;
    xui_doc_free_proc free;
    void* user;
    atomic_uint_fast64_t live, peak, allocations, sequence;
    atomic_uint_fast64_t markdown_parses, markdown_parsed_bytes, markdown_incremental_parses;
    atomic_uint_fast64_t prepared_publishes, prepared_storage_updates;
    atomic_uint_fast64_t prepared_accounting_visits;
    atomic_uint_fast64_t next_node_id;
    uint64_t max_bytes, max_nodes; /* Immutable for every snapshot/candidate. */
    atomic_bool memory_lock;
    atomic_bool attribute_lock;
    doc_attribute* attributes[DOC_ATTR_BUCKETS]; /* Weak entries; nodes own refs. */
    uint64_t memory_buckets[8], history_record_bytes;
    uint64_t snapshot_count, snapshot_handle_bytes;
    struct xui_doc_snapshot_t* snapshots;
} doc_allocator;
typedef struct doc_find_expansion {
    char* text;
    uint64_t bytes;
} doc_find_expansion;
enum doc_memory_kind { DOC_MEMORY_BLOB = 1, DOC_MEMORY_SEQUENCE, DOC_MEMORY_NODE, DOC_MEMORY_INDEX, DOC_MEMORY_STATE, DOC_MEMORY_ATTRIBUTE };
typedef union doc_allocation {
    struct {
        doc_allocator* allocator;
        size_t bytes;
        uint64_t owners[3]; /* Current, history and transient snapshot-union reachability. */
        unsigned kind;
    } value;
    max_align_t alignment;
} doc_allocation;
typedef struct doc_blob {
    atomic_uint refs;
    uint64_t size;
    char data[];
} doc_blob;
#define DOC_EXTENSION_MAX_PAYLOAD_BYTES (UINT64_C(16) * 1024 * 1024)
/* Persistent implicit treap: item lengths are bytes for text or 1 for IDs. */
typedef struct doc_sequence {
    atomic_uint refs;
    uint64_t priority, length, total;
    struct doc_sequence *left, *right;
    doc_blob* blob;
    struct doc_sequence* value; /* Ordinal metadata owns an immutable byte rope. */
    uint64_t offset;
    xui_doc_node_id id;
    /* Packed inline syntax and top-level Markdown IDs use persistent source
     * deltas. Self applies to this piece; child applies to both child trees. */
    int64_t syntax_self_source, syntax_self_index;
    int64_t syntax_child_source, syntax_child_index;
} doc_sequence;
typedef struct doc_node {
    atomic_uint refs;
    xui_doc_node_id id, parent;
    uint32_t kind;
    const xui_doc_attributes_t* attrs;
    doc_sequence *text, *children, *provenance; /* Packed xui_doc_source_segment_t. */
    doc_blob *resource, *info, *title, *link_target, *link_title;
    doc_blob *column_widths; /* Table-only packed float widths; zero is automatic. */
    doc_blob *extension_payload; /* Opaque bytes; never interpreted by core. */
    doc_blob *quote_prefixes; /* Packed relative uint32 offsets after the opening '>'. */
    doc_blob *syntax_aux; /* Kind-specific table, fence, heading, code, quote or list-first-line metadata. */
    doc_blob *list_indents; /* Packed parser-confirmed relative ranges and logical columns. */
    uint32_t extension_version;
    int extension_required;
    uint64_t source_start, source_end;
    uint64_t syntax_start, syntax_end;
    /* MD4C offsets fit 32 bits. Relative positions use the existing root
     * source shift and avoid widening every text node for sparse markers. */
    uint32_t marker_primary_start, marker_primary_end;
    uint32_t marker_secondary_start, marker_secondary_end, marker_kind;
    /* Admonition quote: header line ending. Fence: opening info tail.
     * List: post-marker gap, or post-task gap when
     * a checkbox is present (the first gap then ends at its '[' marker). */
    uint32_t marker_tail_end;
    /* Provenance remains shared when an unchanged Markdown block moves in the
     * source. DOC_NONE means the packed segments already contain final offsets. */
    uint64_t provenance_base, provenance_current;
    /* Top-level Markdown child ordinal locates the versioned source shift.
     * Parsed coordinates are relative to the shift recorded when this node
     * was created; DOC_NONE marks root/rich nodes without a block anchor. */
    uint64_t block_ordinal;
    int64_t block_shift_base;
    int source_exact;
} doc_node;
typedef struct doc_table_token_relative {
    uint32_t start, end, row, ordinal, kind, flags;
} doc_table_token_relative;
/* LIST_ITEM syntax_aux holds one first-line record, including the marker and
 * post-marker/task gap; list_indents holds only whitespace continuation records.
 * Logical columns belong to the parser input, including dedented footnote bodies. */
typedef struct doc_list_indent_relative {
    uint32_t start, end, start_column, content_column, end_column;
} doc_list_indent_relative;
/* Quote-owned indentation and the one optional logical space after '>'.
 * Source offsets are relative to syntax_start; columns belong to the parser's
 * current input line (which may start after a footnote's dedentation). */
typedef struct doc_quote_indent_relative {
    uint32_t marker, end, start_column, marker_column, content_column, end_column;
} doc_quote_indent_relative;
typedef struct doc_code_indent_relative {
    uint32_t start, end, start_column, content_column, end_column, flags;
} doc_code_indent_relative;
typedef struct doc_fence_info_relative {
    uint32_t info_start, info_end, language_end;
} doc_fence_info_relative;
typedef struct doc_heading_content_relative {
    uint32_t start, end;
} doc_heading_content_relative;
typedef struct doc_node_source_range {
    uint64_t source_start, source_end, syntax_start, syntax_end;
} doc_node_source_range;
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
    doc_sequence* references; /* Packed xui_doc_reference_definition_t records. */
    doc_sequence* reference_values; /* Ordered immutable link fields or footnote label/dedented body. */
    doc_sequence* inline_syntax; /* Packed xui_doc_inline_syntax_t records. */
    doc_sequence* reference_candidates; /* Packed xui_doc_inline_syntax_t lookup attempts. */
    uint64_t node_count, text_bytes, payload_bytes, content_id;
    uint64_t source_storage_bytes; /* Conservative retained source-Blob payload bound; scan only after sufficient churn. */
    uint64_t source_open_brackets; /* Exact raw '[' count for the MD4C reference-expansion guard. */
    uint32_t profile, dialect;
    int markdown_footnotes; /* Includes unused definitions, absent from the rendered tree. */
    int block_shifts; /* Root child source offsets require normalization on full projection. */
    int source_blocks_indexed; /* Every source-position candidate fits ordered root block bounds. */
} doc_state;
typedef struct doc_range_branch_plan {
    uint64_t ancestor, left_child, right_child;
    uint64_t target_parent, target_index;
} doc_range_branch_plan;
typedef struct doc_observer {
    uint64_t token;
    xui_doc_change_proc callback;
    void* user;
    struct doc_observer* next;
    int removed;
} doc_observer;
typedef struct doc_history {
    atomic_uint refs;
    doc_state *before, *after;
    xui_doc_operation_t* ops;
    uint64_t count, origin, group;
    uint32_t flags, domain;
    int accounted;
    struct doc_history* next;
} doc_history;
typedef struct doc_publish_plan doc_publish_plan;
typedef struct doc_memory_delta { doc_allocation* allocation; uint64_t current, history; } doc_memory_delta;
struct xui_document_t {
    atomic_uint refs;
    doc_allocator* allocator;
    doc_state* state;
    uint64_t identity, revision, next_state, next_observer, saved_state, next_prepare;
    uint64_t max_bytes, max_nodes, history_max_bytes;
    unsigned history_limit, undo_count, redo_count;
    int disable_history, standby_history, notifying; /* Standby owns current root only while both history stacks are empty. */
    struct xui_doc_transaction_t* writer;
    xui_document_prepare pending;
    uint64_t input_barrier; /* Owner token for accepted, ordered Editor events. */
    doc_history *undo, *redo;
    doc_observer* observers;
};
struct xui_doc_snapshot_t {
    atomic_uint refs;
    doc_state* state;
    uint64_t identity, revision;
    struct xui_doc_snapshot_t *memory_previous, *memory_next;
};
struct xui_doc_transaction_t {
    xui_document document;
    doc_state *base, *draft;
    uint64_t base_revision, origin, group;
    uint32_t domain, flags;
    xui_doc_operation_t* ops;
    uint64_t count, capacity, parse_op_start;
    doc_sequence* source_before_patch; /* Borrowed only during a parsed source patch. */
    const atomic_int* cancellation;
    xui_document_prepare preparation;
    int error, closed, parsing;
};
struct xui_doc_change_set_t {
    atomic_uint refs;
    doc_state *before, *after;
    uint64_t identity, before_revision, after_revision, origin;
    uint32_t flags, domain;
    xui_doc_operation_t* ops;
    uint64_t count;
    int undo;
};

void* doc_alloc(doc_allocator*, size_t);
void* doc_realloc(doc_allocator*, void*, size_t);
void doc_free(void*);
void doc_memory_tag(void*, unsigned);
void doc_memory_current(doc_state*, doc_state*);
void doc_memory_history(doc_history*, int);
void doc_memory_standby(doc_state*, int);
void doc_memory_apply_prepared(doc_allocator*, const doc_memory_delta*, uint64_t, const uint64_t*, const uint64_t*, uint64_t);
void doc_memory_owner_counts(const doc_allocation*, uint64_t*, uint64_t*);
uint64_t doc_memory_storage_history_bytes(doc_allocator*);
uint64_t doc_memory_history_bytes(doc_allocator*);
void doc_memory_stats(doc_allocator*, xui_doc_stats_t*);
int doc_snapshot_create(doc_state*, uint64_t, uint64_t, xui_document_snapshot*);
void doc_snapshot_unregister(xui_document_snapshot);
void doc_allocator_retain(doc_allocator*);
void doc_allocator_release(doc_allocator*);
uint64_t doc_node_id_next(doc_allocator*);
void doc_prepare_invalidate(xui_document, xui_document_prepare, int);
doc_sequence* doc_prepare_source_store(xui_document_prepare);
int doc_prepare_source_delta(xui_document_prepare, xui_document_prepare, uint64_t*, uint64_t*, uint64_t*);
int doc_prepare_position_valid(xui_document_prepare, const xui_doc_position_t*);
int doc_prepare_preview(xui_document, xui_document_prepare, const xui_doc_source_patch_t*, xui_document_prepare*);
void doc_history_retain(doc_history*);
void doc_history_release(doc_history*);
doc_history* doc_history_prepare(xui_document_transaction, doc_history*);
xui_document_change_set doc_change_create(doc_allocator*, uint64_t, uint64_t, doc_state*, doc_state*,
    const xui_doc_operation_t*, uint64_t, uint32_t, uint32_t, uint64_t, int);
doc_publish_plan* doc_publish_plan_capture(xui_document);
int doc_publish_plan_build(doc_publish_plan*, xui_document_transaction, uint64_t);
void doc_publish_plan_release(doc_publish_plan*);
doc_publish_plan* doc_prepare_publish_plan(xui_document_prepare, xui_document);
int doc_publish_plan_valid(doc_publish_plan*, xui_document);
void doc_publish_plan_apply(doc_publish_plan*);
uint64_t doc_publish_plan_undo_count(doc_publish_plan*);
doc_history* doc_publish_plan_take_history(doc_publish_plan*);
xui_document_change_set doc_publish_plan_change(doc_publish_plan*);
doc_blob* doc_blob_new(doc_allocator*, const char*, uint64_t);
doc_blob* doc_blob_allocate(doc_allocator*, uint64_t);
void doc_blob_retain(doc_blob*);
void doc_blob_release(doc_blob*);
const char* doc_string(doc_blob*);
int doc_utf8(const char*, uint64_t);
uint64_t doc_source_open_bracket_count(const char*, uint64_t);
uint64_t doc_seq_size(doc_sequence*);
void doc_seq_retain(doc_sequence*);
void doc_seq_release(doc_sequence*);
doc_sequence* doc_seq_text(doc_allocator*, const char*, uint64_t);
doc_sequence* doc_seq_id(doc_allocator*, uint64_t);
doc_sequence* doc_seq_value_item(doc_allocator*, uint64_t, doc_sequence*);
doc_sequence* doc_seq_get_value_item(doc_sequence*, uint64_t);
doc_sequence* doc_seq_blob_range(doc_allocator*, doc_blob*, uint64_t, uint64_t);
int doc_seq_reuse_bytes(doc_allocator*, doc_sequence*, doc_sequence*, const atomic_int*, doc_sequence**);
int doc_seq_compact_bytes(doc_allocator*, doc_sequence**, const atomic_int*, uint64_t*);
int doc_reference_value_append(doc_state*, uint32_t, const char*, uint64_t,
    const char*, uint64_t, const char*, uint64_t, int, const atomic_int*);
int doc_reference_value_replace(doc_state*, uint64_t, doc_state*, uint64_t, const atomic_int*);
int doc_reference_values_equal(doc_state*, doc_state*, const atomic_int*, int*);
int doc_reference_values_reuse(doc_state*, doc_state*, doc_state*, const atomic_int*);
int doc_seq_replace(doc_allocator*, doc_sequence*, uint64_t, uint64_t, doc_sequence*, doc_sequence**);
int doc_seq_read(doc_sequence*, uint64_t, void*, uint64_t);
int doc_seq_read_syntax(doc_sequence*, uint64_t, xui_doc_inline_syntax_t*);
int doc_seq_syntax_replace(doc_allocator*, doc_sequence*, uint64_t, uint64_t,
    doc_sequence*, uint64_t, uint64_t, uint64_t, doc_sequence**);
int doc_seq_syntax_shift_source_range(doc_allocator*, doc_sequence*, uint64_t,
    uint64_t, int64_t, doc_sequence**);
int64_t doc_seq_source_shift(doc_sequence*, uint64_t);
int doc_seq_shift_source_suffix(doc_allocator*, doc_sequence*, uint64_t, int64_t, doc_sequence**);
int doc_seq_clear_source_shifts(doc_allocator*, doc_sequence*, doc_sequence**);
uint64_t doc_seq_get_id(doc_sequence*, uint64_t);
int doc_seq_boundary(doc_sequence*, uint64_t);
int doc_seq_equal_bytes(doc_sequence*, uint64_t, const char*, uint64_t);
char* doc_seq_string(doc_allocator*, doc_sequence*);
doc_node* doc_node_new(doc_allocator*, uint64_t, uint64_t, const xui_doc_node_desc_t*);
doc_node* doc_node_clone(doc_allocator*, const doc_node*);
int doc_node_set_attrs(doc_allocator*, doc_node*, const xui_doc_attributes_t*);
static inline doc_attribute* doc_attribute_owner(const xui_doc_attributes_t* value)
{
    return value ? (doc_attribute*)((char*)value - offsetof(doc_attribute, value)) : NULL;
}
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
int64_t doc_node_block_shift(const doc_state*, const doc_node*);
void doc_node_source_range_with_children(doc_sequence*, const doc_node*, doc_node_source_range*);
void doc_node_source_range_get(const doc_state*, const doc_node*, doc_node_source_range*);
int doc_node_info(const doc_state*, const doc_node*, xui_doc_node_info_t*);
int doc_source_append(doc_allocator*, doc_sequence**, const xui_doc_source_segment_t*);
int doc_source_slice(doc_state*, const doc_node*, uint64_t, uint64_t, uint64_t, doc_sequence**);
void doc_source_resolve_segment(const doc_state*, const doc_node*, xui_doc_source_segment_t*);
int doc_source_shift_provenance(doc_node*, uint64_t, uint64_t);
typedef struct doc_source_hit { uint64_t offset, distance, span; unsigned rank; int mapping; } doc_source_hit;
int doc_source_better(const doc_source_hit*, const doc_source_hit*, unsigned);
int doc_source_map_source(const doc_state*, const doc_node*, uint64_t, unsigned, doc_source_hit*);
int doc_source_map_text(const doc_state*, const doc_node*, uint64_t, unsigned, uint64_t*, int*);
int doc_source_find_position(doc_state*, uint64_t, unsigned, int, uint64_t*, doc_source_hit*);
int doc_markdown_source_blocks_ordered(doc_state*, uint64_t);
int doc_markdown_source_block_valid(doc_state*, uint64_t, uint64_t);
int doc_schema_child(uint32_t, uint32_t);
int doc_schema_attrs(uint32_t, const xui_doc_attributes_t*);
int doc_attributes_equal(const xui_doc_attributes_t*, const xui_doc_attributes_t*);
static inline int doc_text_color_set(const xui_doc_attributes_t* attrs)
{
    return attrs->iTextColor != 0 || !!(attrs->iFlags & XUI_DOC_TEXT_COLOR_EXPLICIT_ZERO);
}
static inline int doc_background_color_set(const xui_doc_attributes_t* attrs)
{
    return attrs->iBackgroundColor != 0 || !!(attrs->iFlags &
        (XUI_DOC_BACKGROUND_COLOR_EXPLICIT_ZERO | XUI_DOC_BACKGROUND_COLOR_CURRENT));
}
xui_doc_attributes_t doc_effective_text_attrs(const doc_state*, const doc_node*);
uint32_t doc_effective_alignment(const doc_state*, const doc_node*);
int doc_schema_validate(doc_state*);
int doc_schema_validate_transaction(xui_document_transaction);
int doc_schema_depth(doc_state*, uint64_t, unsigned);
uint32_t doc_table_column_count(const doc_state*, const doc_node*);
float doc_table_column_width(const doc_node*, uint32_t);
typedef struct doc_table_cell_slot {
    uint64_t table, cell;
    uint32_t row, column, row_span, column_span, rows, columns;
} doc_table_cell_slot;
int doc_table_locate_cell(const doc_state*, uint64_t cell, doc_table_cell_slot*);
int doc_table_cell_at(const doc_state*, uint64_t table, uint32_t row, uint32_t column,
    doc_table_cell_slot*);
int doc_table_expand_selection(doc_state*, uint64_t table, uint32_t* row,
    uint32_t* column, uint32_t* rows, uint32_t* columns);
int doc_table_can_merge(const doc_state*, uint64_t table, uint32_t row, uint32_t column,
    uint32_t rows, uint32_t columns);
int doc_text_kind(uint32_t);
int doc_selectable_object_kind(uint32_t);
int doc_inline_kind(uint32_t);
int doc_txn_fail(xui_document_transaction, int);
int doc_txn_op(xui_document_transaction, const xui_doc_operation_t*);
int doc_txn_check(xui_document_transaction, int);
int doc_txn_insert(xui_document_transaction, uint64_t, uint64_t, const xui_doc_node_desc_t*, uint64_t*);
int doc_txn_delete(xui_document_transaction, uint64_t);
int doc_txn_text(xui_document_transaction, uint64_t, uint64_t, uint64_t, const char*, uint64_t);
int doc_range_branch_plan_get(doc_state*, const doc_node*, const doc_node*,
    doc_range_branch_plan*);
uint64_t doc_child_index(const doc_node*, uint64_t);
int doc_markdown_parse(xui_document_transaction);
int doc_markdown_parse_window(xui_document_transaction, uint64_t, uint64_t, doc_state**);
int doc_markdown_parse_input(xui_document_transaction, uint64_t, const char*, uint64_t, doc_state**, uint64_t*);
typedef struct doc_front_matter_range {
    uint64_t content_start, content_end, syntax_end;
} doc_front_matter_range;
int doc_markdown_front_matter_range(const char*, uint64_t, uint64_t, const atomic_int*, doc_front_matter_range*);
int doc_markdown_parse_window_with_suffix(xui_document_transaction, uint64_t, uint64_t,
    const char*, uint64_t, doc_state**);
int doc_markdown_incremental(xui_document_transaction, doc_state**);
int doc_markdown_reconcile(xui_document_transaction, doc_state**);
int doc_markdown_reconcile_block(xui_document_transaction, doc_state*, uint64_t, uint64_t,
    doc_state*, uint64_t, int64_t, uint64_t*);
int doc_markdown_accept(xui_document_transaction, xui_document_transaction);
int doc_markdown_text(xui_document_transaction, uint64_t, uint64_t, uint64_t, const char*, uint64_t, xui_doc_position_t*);
int doc_prepare_visual_text(xui_document, const xui_doc_txn_desc_t*, uint64_t, uint64_t, uint64_t,
    const char*, uint64_t, xui_document_prepare*, xui_document_snapshot*, uint64_t*);
int doc_prepare_visual_span(xui_document, const xui_doc_txn_desc_t*, const xui_doc_range_t*,
    const char*, uint64_t, xui_document_prepare*, xui_document_snapshot*,
    xui_doc_position_t*, uint64_t*);
int doc_prepare_visual_continue(xui_document, xui_document_prepare, uint64_t, uint64_t, uint64_t,
    const char*, uint64_t, uint64_t, xui_document_prepare*, xui_document_snapshot*, uint64_t*);
int doc_prepare_visual_position_source(xui_document, xui_document_prepare,
    const xui_doc_position_t*, uint64_t*);
int doc_prepare_visual_attach(xui_document, xui_document_prepare, xui_document_prepare);
int doc_markdown_attributes(xui_document_transaction, uint64_t, const xui_doc_attributes_t*);
int doc_markdown_marks(xui_document_transaction, const xui_doc_range_t*, uint32_t, uint32_t);
int doc_markdown_link(xui_document_transaction, const xui_doc_range_t*, const char*, const char*);
int doc_markdown_link_patch(xui_document_transaction, const xui_doc_range_t*, const char*, const char*, doc_state*);
int doc_markdown_image_patch(xui_document_transaction, uint64_t, doc_state*);
int doc_markdown_image_insert_patch(xui_document_transaction, const xui_doc_range_t*, uint64_t, doc_state*);
int doc_markdown_shadow_end_image(xui_document_transaction, struct xui_doc_transaction_t*, int, uint64_t);
int doc_markdown_shadow_end_insert_image(xui_document_transaction, struct xui_doc_transaction_t*, int,
    const xui_doc_range_t*, uint64_t, const xui_doc_position_t*, xui_doc_position_t*);
int doc_markdown_range(xui_document_transaction, const xui_doc_range_t*, const char*, uint64_t, xui_doc_position_t*);
int doc_markdown_apply_tree(xui_document_transaction, doc_state*, const xui_doc_position_t*, xui_doc_position_t*);
int doc_markdown_shadow_begin(xui_document_transaction, struct xui_doc_transaction_t*);
int doc_markdown_shadow_end(xui_document_transaction, struct xui_doc_transaction_t*, int, const xui_doc_position_t*, xui_doc_position_t*);
int doc_markdown_shadow_end_unwrap_quote(xui_document_transaction, struct xui_doc_transaction_t*, int,
    const xui_doc_position_t*, const xui_doc_position_t*, xui_doc_position_t*);
int doc_markdown_shadow_end_list_split(xui_document_transaction, struct xui_doc_transaction_t*, int,
    const xui_doc_position_t*, const xui_doc_position_t*, xui_doc_position_t*);
int doc_markdown_shadow_end_table_row(xui_document_transaction,
    struct xui_doc_transaction_t*, int, uint64_t, uint32_t, int);
int doc_markdown_shadow_end_table_column(xui_document_transaction,
    struct xui_doc_transaction_t*, int, uint64_t, uint32_t, int);
int doc_markdown_shadow_end_range(xui_document_transaction, struct xui_doc_transaction_t*, int, const xui_doc_range_t*, xui_doc_range_t*);
int doc_markdown_shadow_end_quote_range(xui_document_transaction, struct xui_doc_transaction_t*, int,
    uint64_t, const xui_doc_range_t*, xui_doc_range_t*);
uint64_t doc_markdown_resolve_node(doc_state*, doc_state*, uint64_t);
xui_doc_position_t doc_markdown_resolve_caret(xui_document_transaction, doc_state*, xui_doc_position_t);
int doc_block_range_ids(doc_state*, const xui_doc_range_t*, uint64_t**, uint64_t*);
int doc_quote_range_ids(doc_state*, const xui_doc_range_t*, uint64_t**, uint64_t*);
int doc_quote_range_can(doc_state*, const xui_doc_range_t*);
int doc_move_block_range_can(doc_state*, const xui_doc_range_t*, int down);
int doc_list_create_range_can(doc_state*, const xui_doc_range_t*, uint32_t, uint64_t);
int doc_list_style_range_can(doc_state*, const xui_doc_range_t*, uint32_t, uint64_t,
    int*, int*);
int doc_unlist_range_can(doc_state*, const xui_doc_range_t*);
int doc_list_range_can(doc_state*, const xui_doc_range_t*, int);
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
int doc_find_expand_matches(xui_document_snapshot, uint32_t, const char*,
    uint64_t, uint32_t, const xui_doc_range_t*, const xui_doc_range_t*,
    uint64_t, const char*, uint64_t, doc_find_expansion**);
void doc_find_expansions_free(doc_find_expansion*, uint64_t);
int doc_split_text(xui_document_transaction, uint64_t, uint64_t, uint64_t*);
int doc_semantic_equal(doc_state*, doc_state*);
int doc_semantic_subtree_equal(doc_state*, uint64_t, doc_state*, uint64_t);
int doc_semantic_empty_paragraph(doc_state*, const doc_node*);
int doc_copy_subtree_apply(xui_document_transaction, doc_state*, uint64_t,
    uint64_t, uint64_t, uint64_t*);
int doc_html_export_children(xui_document_snapshot, uint64_t, char**, uint64_t*);
int doc_html_safe_url(const char*, int);
int doc_html_import_document(const char*, uint64_t, xui_document*, unsigned*);
const char* doc_fragment_single_image_resource(xui_document_fragment);

#endif
