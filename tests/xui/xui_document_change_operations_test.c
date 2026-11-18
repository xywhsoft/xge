#define XRT_MODULE_ALL
#include "xui.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
	xui_document document = NULL;
	xui_document_transaction transaction = NULL;
	xui_document_change_set change = NULL;
	xui_doc_node_id paragraph = 0;
	xui_doc_node_id text = 0;
	assert(xuiDocumentCreateOwned(NULL, &document) == XUI_OK);
	assert(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
	xui_doc_node_desc_t node = {.iSize = sizeof(node), .iKind = XUI_DOC_PARAGRAPH};
	assert(xuiDocumentTxnInsertNode(transaction, XUI_DOCUMENT_ROOT, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
	node = (xui_doc_node_desc_t){.iSize = sizeof(node), .iKind = XUI_DOC_TEXT, .sText = "hello", .iTextBytes = 5};
	assert(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &text) == XUI_OK);
	assert(xuiDocumentTxnCommit(transaction, &change) == XUI_OK);

	const xui_doc_operation_t* operations = NULL;
	size_t count = 0;
	xui_doc_change_info_t info = {.iSize = sizeof(info)};
	assert(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
	assert(xuiDocumentChangeSetGetOperations(NULL, &operations, &count) == XUI_ERROR_INVALID_ARGUMENT && !operations && !count);
	assert(xuiDocumentChangeSetGetOperations(change, NULL, &count) == XUI_ERROR_INVALID_ARGUMENT && !count);
	assert(xuiDocumentChangeSetGetOperations(change, &operations, NULL) == XUI_ERROR_INVALID_ARGUMENT && !operations);
	assert(xuiDocumentChangeSetAddRef(change) == XUI_OK);
	assert(xuiDocumentChangeSetGetOperations(change, &operations, &count) == XUI_OK);
	assert(operations == info.pOperations && count == info.iOperationCount && count >= 2);
	assert(xuiDocumentChangeSetReleaseChecked(change) == XUI_OK);
	assert(xuiDocumentTxnReleaseChecked(transaction) == XUI_OK);
	assert(xuiDocumentReleaseChecked(document) == XUI_OK);
	bool found = false;
	for (size_t i = 0; i < count; ++i) {
		if (operations[i].iNodeId == text) found = true;
	}
	assert(found);
	assert(xuiDocumentChangeSetReleaseChecked(change) == XUI_OK);
	puts("document-change-operations-ok");
	return 0;
}
