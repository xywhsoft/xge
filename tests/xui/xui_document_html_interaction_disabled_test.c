#include <stdio.h>
#include "xui_document_ui.h"

int main(void)
{
    xui_context context = NULL;
    xui_widget root = NULL;
    xui_document document = NULL;
    xui_document_transaction transaction = NULL;
    xui_doc_node_desc_t node = {0};
    xui_doc_node_id id = 0;
    xui_doc_html_interaction_desc_t desc = {0};
    xui_doc_html_interaction panel = NULL;
    int status = 1;
    if (xuiCreate(&context) != XUI_OK ||
        xuiWidgetCreate(context, &root) != XUI_OK ||
        xuiSetRootWidget(context, root) != XUI_OK ||
        xuiWidgetSetRect(root, (xui_rect_t){0, 0, 640, 480}) != XUI_OK ||
        xuiDocumentCreate(NULL, &document) != XUI_OK)
        goto done;
    node.iSize = sizeof(node);
    node.iKind = XUI_DOC_HTML;
    node.tAttributes.iFlags = XUI_DOC_BLOCK;
    node.sText = "<p>source</p>";
    node.iTextBytes = 13;
    if (xuiDocumentBeginTransaction(document, NULL, &transaction) != XUI_OK ||
        xuiDocumentTxnInsertNode(transaction, XUI_DOCUMENT_ROOT, 0,
            &node, &id) != XUI_OK ||
        xuiDocumentTxnCommit(transaction, NULL) != XUI_OK)
        goto done;
    xuiDocumentTxnRelease(transaction); transaction = NULL;
    desc.iSize = sizeof(desc);
    desc.pContext = context;
    desc.pDocument = document;
    desc.iNodeId = id;
    if (xuiDocumentHtmlInteractionCreate(&desc, &panel) !=
            XUI_ERROR_INVALID_STATE || panel != NULL ||
        xuiInputViewport(context, 640, 480) != XUI_OK)
        goto done;
    if (xuiDocumentHtmlInteractionCreate(&desc, &panel) !=
            XUI_ERROR_UNSUPPORTED || panel != NULL)
        goto done;
    status = 0;
done:
    if (panel) xuiDocumentHtmlInteractionRelease(panel);
    xuiDocumentTxnRelease(transaction);
    xuiDocumentRelease(document);
    if (context) xuiDestroy(context);
    if (status == 0)
        puts("Document HTML interaction: disabled WebView returns UNSUPPORTED");
    else
        fputs("Document HTML interaction disabled test failed\n", stderr);
    return status;
}
