#ifndef XUI_DOCUMENT_MD4C_H
#define XUI_DOCUMENT_MD4C_H
#include "xui_document_internal.h"
#include "../lib/md4c/md4c.h"
typedef void (*doc_md4c_block_source_proc)(MD_BLOCKTYPE, MD_OFFSET, MD_OFFSET, int, void*);
typedef void (*doc_md4c_block_markers_proc)(int, MD_OFFSET, MD_OFFSET, MD_OFFSET, MD_OFFSET,
    MD_OFFSET, MD_OFFSET, MD_OFFSET, void*);
typedef int (*doc_md4c_fence_info_proc)(MD_OFFSET, MD_OFFSET, MD_OFFSET, void*);
typedef int (*doc_md4c_heading_content_proc)(MD_OFFSET, MD_OFFSET, void*);
typedef int (*doc_md4c_break_source_proc)(int, MD_OFFSET, MD_OFFSET,
    MD_OFFSET, MD_OFFSET, MD_OFFSET, void*);
/* Parser-confirmed origin of one normalized text callback. */
typedef int (*doc_md4c_text_source_proc)(MD_TEXTTYPE, MD_TEXTTYPE, MD_OFFSET, MD_OFFSET, MD_SIZE, void*);
typedef int (*doc_md4c_text_scope_proc)(MD_TEXTTYPE, MD_OFFSET, MD_OFFSET, int, void*);
/* Six entries per record: marker/end offsets, start/marker/content/end columns. */
typedef int (*doc_md4c_quote_prefixes_proc)(const MD_OFFSET*, MD_SIZE, void*);
typedef int (*doc_md4c_list_indents_proc)(const MD_OFFSET*, MD_SIZE, void*);
typedef int (*doc_md4c_code_indents_proc)(const MD_OFFSET*, MD_SIZE, void*);
typedef int (*doc_md4c_table_token_proc)(int, MD_SIZE, MD_SIZE, MD_OFFSET, MD_OFFSET, unsigned, void*);
typedef int (*doc_md4c_reference_source_proc)(MD_OFFSET, MD_OFFSET, MD_OFFSET, MD_OFFSET,
    MD_OFFSET, MD_OFFSET, MD_OFFSET, MD_OFFSET, int, void*);
typedef int (*doc_md4c_reference_values_proc)(const char*, MD_SIZE, const char*, MD_SIZE,
    const char*, MD_SIZE, int, void*);
typedef int (*doc_md4c_footnote_source_proc)(MD_OFFSET, MD_OFFSET, MD_OFFSET, MD_OFFSET,
    const char*, MD_SIZE, void*);
typedef int (*doc_md4c_candidate_source_proc)(int, MD_OFFSET, MD_OFFSET, MD_OFFSET, MD_OFFSET, void*);
typedef int (*doc_md4c_span_source_proc)(MD_SPANTYPE, MD_OFFSET, MD_OFFSET, int, MD_OFFSET, MD_OFFSET, void*);
int doc_md4c_parse(doc_allocator*, const char*, MD_SIZE, const MD_PARSER*, doc_md4c_block_source_proc,
    doc_md4c_block_markers_proc, doc_md4c_fence_info_proc, doc_md4c_heading_content_proc, doc_md4c_break_source_proc,
    doc_md4c_quote_prefixes_proc, doc_md4c_list_indents_proc,
    doc_md4c_code_indents_proc,
    doc_md4c_table_token_proc,
    doc_md4c_reference_source_proc, doc_md4c_reference_values_proc, doc_md4c_footnote_source_proc, doc_md4c_candidate_source_proc,
    doc_md4c_span_source_proc, doc_md4c_text_source_proc, doc_md4c_text_scope_proc,
    const atomic_int*, int*, void*);
unsigned doc_md4c_dialect_flags(uint32_t);
/* Compare every parsed link definition, including unused/duplicate definitions.
 * The parser removes only container prefixes and merges multiline fields. */
int doc_md4c_reference_values_equal(doc_allocator*, const char*, MD_SIZE,
    const char*, MD_SIZE, unsigned, const atomic_int*, int*);
/* Use the pinned parser's Unicode folding and whitespace rules for labels. */
int doc_md4c_label_equal(const char*, const char*);
int doc_md4c_footnote_label_valid(const char*);
/* Mirrors the pinned parser's Unicode delimiter-flanking classifications. */
int doc_md4c_unicode_punct(unsigned codepoint);
int doc_md4c_unicode_whitespace(unsigned codepoint);
#endif
