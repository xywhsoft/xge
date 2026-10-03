#include "../xui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); exit(1); } } while (0)
#include "xui_document_list_split_source_samples.h"
int main(void)
{
    unsigned sample;
    for (sample = 0; sample < sizeof(document_list_split_source_samples) / sizeof(*document_list_split_source_samples); sample++) {
    const char* input = document_list_split_source_samples[sample].input;
    xui_doc_desc_t desc = {0}; xui_document document; xui_document_snapshot snapshot;
    xui_document_transaction transaction; xui_doc_range_t range; xui_doc_position_t caret;
    uint64_t count, bytes; char source[1024]; int result;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK && xuiDocumentLoadMarkdown(document, input, strlen(input)) == XUI_OK &&
        xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcd", 4, NULL, &range, 1, &count) == XUI_OK && count == 1);
    {
        xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, range.tAnchor.iNodeId, &node) == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, node.iParentId, &node) == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, node.iParentId, &node) == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, node.iParentId, &node) == XUI_OK);
        fprintf(stderr, "sample=%u original list flags=%u\n", sample, node.tAttributes.iFlags);
        if (document_list_split_source_samples[sample].offset == 2)
            CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "cd", 2, NULL, &range, 1, &count) == XUI_OK && count == 1);
        else if (document_list_split_source_samples[sample].offset == 4) range.tAnchor = range.tCaret;
    }
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    result = xuiDocumentTxnSplitListItem(transaction, &range.tAnchor, &caret);
    fprintf(stderr, "Nested task item split result=%d\n", result);
    CHECK(result == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotCopySource(snapshot, source, sizeof(source), &bytes) == XUI_OK);
    if (strcmp(source, document_list_split_source_samples[sample].expected))
        fprintf(stderr, "sample=%u expected=[%s]\nactual=[%s]\n", sample, document_list_split_source_samples[sample].expected, source);
    CHECK(!strcmp(source, document_list_split_source_samples[sample].expected));
    xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(document);
    }
    puts("List split source public DLL probe: all samples passed");
    return 0;
}
