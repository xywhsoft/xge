#define XRT_MODULE_ALL
#include "xui.h"
#include "src/xui/document/xui_document_internal.h"

#include <assert.h>
#include <stdio.h>

static void* (*original_alloc)(void*, size_t);
static int allocation_budget;

static void* fail_alloc(void* user, size_t bytes)
{
    if (allocation_budget-- == 0) return NULL;
    return original_alloc(user, bytes);
}

static int matches_oom(xui_document_snapshot snapshot)
{
    doc_allocator* allocator = snapshot->state->allocator;
    uint64_t baseline = atomic_load(&allocator->live);
    original_alloc = allocator->alloc;
    for (int budget = 0; budget < 64; ++budget) {
        xui_document_matches matches = NULL;
        allocation_budget = budget;
        allocator->alloc = fail_alloc;
        int status = xuiDocumentSnapshotFindOwned(snapshot, XUI_DOC_SEMANTIC,
            "needle", 6, 0, NULL, &matches);
        allocator->alloc = original_alloc;
        if (status == XUI_OK) {
            assert(matches && xuiDocumentMatchesReleaseChecked(matches) == XUI_OK);
            assert(atomic_load(&allocator->live) == baseline);
            return budget;
        }
        assert(status == XUI_ERROR_OUT_OF_MEMORY && !matches);
        assert(atomic_load(&allocator->live) == baseline);
    }
    return -1;
}

static int losses_oom(xui_document_snapshot snapshot)
{
    doc_allocator* allocator = snapshot->state->allocator;
    uint64_t baseline = atomic_load(&allocator->live);
    original_alloc = allocator->alloc;
    for (int budget = 0; budget < 64; ++budget) {
        xui_document_markdown_losses losses = NULL;
        allocation_budget = budget;
        allocator->alloc = fail_alloc;
        int status = xuiDocumentSnapshotCollectMarkdownLosses(snapshot, XUI_MD_COMMONMARK, &losses);
        allocator->alloc = original_alloc;
        if (status == XUI_OK) {
            assert(losses && xuiDocumentMarkdownLossesReleaseChecked(losses) == XUI_OK);
            assert(atomic_load(&allocator->live) == baseline);
            return budget;
        }
        assert(status == XUI_ERROR_OUT_OF_MEMORY && !losses);
        assert(atomic_load(&allocator->live) == baseline);
    }
    return -1;
}

int main(void)
{
    xui_document document = NULL;
    xui_document_transaction transaction = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_doc_node_id nodes[20];
    assert(xuiDocumentCreateOwned(NULL, &document) == XUI_OK);
    assert(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    for (size_t i = 0; i < 20; ++i) {
        xui_doc_node_id paragraph;
        xui_doc_node_desc_t node = {.iSize = sizeof(node), .iKind = XUI_DOC_PARAGRAPH};
        assert(xuiDocumentTxnInsertNode(transaction, XUI_DOCUMENT_ROOT, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
        node = (xui_doc_node_desc_t){.iSize = sizeof(node), .iKind = XUI_DOC_TEXT, .sText = "needle", .iTextBytes = 6};
        assert(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &nodes[i]) == XUI_OK);
        xui_doc_attributes_t attributes = {.iMarks = XUI_DOC_HIGHLIGHT};
        assert(xuiDocumentTxnSetAttributes(transaction, nodes[i], &attributes) == XUI_OK);
    }
    assert(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    assert(xuiDocumentTxnReleaseChecked(transaction) == XUI_OK);
    assert(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);

    xui_doc_range_t prefix;
    uint64_t total = 0;
    assert(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "needle", 6, NULL, &prefix, 1, &total) == XUI_OK && total == 20);
    xui_document_matches matches = NULL;
    assert(xuiDocumentSnapshotFindOwned(snapshot, XUI_DOC_SEMANTIC,
        "NEEDLE", 6, XUI_DOC_FIND_IGNORE_CASE | XUI_DOC_FIND_WHOLE_WORD, NULL, &matches) == XUI_OK);
    const xui_doc_range_t* ranges = NULL;
    size_t count = 0;
    assert(xuiDocumentMatchesGetRanges(matches, &ranges, &count) == XUI_OK && count == 20);
    assert(ranges[0].tAnchor.iNodeId == prefix.tAnchor.iNodeId && ranges[0].tCaret.iOffset == prefix.tCaret.iOffset);
    xui_document_matches scoped = NULL;
    assert(xuiDocumentSnapshotFindOwned(snapshot, XUI_DOC_SEMANTIC,
        "needle", 6, XUI_DOC_FIND_REGEX, &prefix, &scoped) == XUI_OK);
    const xui_doc_range_t* scoped_ranges = NULL;
    assert(xuiDocumentMatchesGetRanges(scoped, &scoped_ranges, &count) == XUI_OK && count == 1);
    assert(scoped_ranges[0].tAnchor.iNodeId == nodes[0]);
    assert(xuiDocumentMatchesReleaseChecked(scoped) == XUI_OK);
    xui_document_matches empty = NULL;
    assert(xuiDocumentSnapshotFindOwned(snapshot, XUI_DOC_SEMANTIC, "missing", 7, 0, NULL, &empty) == XUI_OK);
    const xui_doc_range_t* none = ranges;
    assert(xuiDocumentMatchesGetRanges(empty, &none, &count) == XUI_OK && !count && !none);
    assert(xuiDocumentMatchesReleaseChecked(empty) == XUI_OK);

    xui_doc_markdown_loss_t first;
    assert(xuiDocumentSnapshotAnalyzeMarkdownConversion(snapshot, XUI_MD_COMMONMARK, &first, 1, &total)
        == XUI_ERROR_BUFFER_TOO_SMALL && total == 20);
    xui_document_markdown_losses losses = NULL;
    assert(xuiDocumentSnapshotCollectMarkdownLosses(snapshot, XUI_MD_COMMONMARK, &losses) == XUI_OK);
    const xui_doc_markdown_loss_t* entries = NULL;
    assert(xuiDocumentMarkdownLossesGetEntries(losses, &entries, &count) == XUI_OK && count == 20);
    assert(entries[0].iNodeId == first.iNodeId && entries[0].iReasons == first.iReasons);
    int search_failures = matches_oom(snapshot);
    int loss_failures = losses_oom(snapshot);
    assert(search_failures >= 4 && loss_failures >= 4);

    xui_document_matches shared_matches = NULL;
    xui_document_markdown_losses shared_losses = NULL;
    assert(xuiDocumentMatchesShareOwned(matches, &shared_matches) == XUI_OK);
    assert(xuiDocumentMarkdownLossesShareOwned(losses, &shared_losses) == XUI_OK);
    assert(xuiDocumentMatchesReleaseChecked(matches) == XUI_OK);
    assert(xuiDocumentMarkdownLossesReleaseChecked(losses) == XUI_OK);
    assert(xuiDocumentSnapshotReleaseChecked(snapshot) == XUI_OK);
    assert(xuiDocumentReleaseChecked(document) == XUI_OK);
    for (size_t i = 0; i < 20; ++i) {
        assert(ranges[i].tAnchor.iNodeId == nodes[i] && ranges[i].tCaret.iOffset == 6);
        assert(entries[i].iNodeId == nodes[i] && (entries[i].iReasons & XUI_DOC_MD_LOSS_MARK));
    }
    assert(xuiDocumentMatchesReleaseChecked(shared_matches) == XUI_OK);
    assert(xuiDocumentMarkdownLossesReleaseChecked(shared_losses) == XUI_OK);
    printf("document-query-results-ok search-oom=%d losses-oom=%d\n", search_failures, loss_failures);
    return 0;
}
