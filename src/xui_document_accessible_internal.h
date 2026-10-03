#ifndef XUI_DOCUMENT_ACCESSIBLE_INTERNAL_H
#define XUI_DOCUMENT_ACCESSIBLE_INTERNAL_H

#include "xui_document_internal.h"
#include "../xui.h"

/* UI-owned semantic projection shared by DocumentView and embedded Documents.
 * The caller frees ids and value with free(). */
int doc_accessible_snapshot_ids(xui_document_snapshot snapshot,
	xui_doc_node_id** ids, uint64_t* count);
int doc_accessible_role(const doc_node* node);
int doc_accessible_text_selectable_kind(uint32_t kind);
int doc_accessible_snapshot_value(xui_document_snapshot snapshot,
	doc_node* node, char** value, uint64_t* bytes);
/* Convert offsets in a semantic node's accessible value to Document positions. */
int doc_accessible_snapshot_selection_range(xui_document_snapshot snapshot,
	doc_node* node, const xui_accessible_selection_t* selected,
	xui_doc_range_t* range);
int doc_accessible_snapshot_selection_offsets(xui_document_snapshot snapshot,
	doc_node* node, const xui_doc_range_t* range,
	int* anchor, int* caret, int* selected);
int doc_accessible_snapshot_object_range(xui_document_snapshot snapshot,
	doc_node* node, xui_doc_range_t* range);
int doc_accessible_snapshot_object_selected(xui_document_snapshot snapshot,
	doc_node* node, const xui_doc_range_t* selection, int* selected);

#endif
