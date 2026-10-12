#ifndef XUI_DOCUMENT_CONTAINER_NEWLINE_HELPERS_H
#define XUI_DOCUMENT_CONTAINER_NEWLINE_HELPERS_H
#define XUI_DOC_INLINE_SPLIT_HELPERS_ONLY
#include "xui_document_inline_split_diagnostic_test.c"
#undef XUI_DOC_INLINE_SPLIT_HELPERS_ONLY

static int newline_execute(xui_document d, const xui_doc_range_t* r, unsigned mode,
    xui_doc_position_t* caret, xui_document_change_set* changes)
{
    xui_document_transaction t; int result = xuiDocumentBeginTransaction(d, NULL, &t);
    if (result != XUI_OK) return result;
    result = mode ? xuiDocumentTxnSplitBlock(t, &r->tAnchor, caret) :
        xuiDocumentTxnReplaceRange(t, r, "\n", 1, caret);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, changes);
    if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
    xuiDocumentTxnRelease(t); return result;
}


#endif
