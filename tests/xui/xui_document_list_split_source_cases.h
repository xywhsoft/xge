#include "xui_document_list_split_source_samples.h"
static xui_doc_position_t list_split_source_position(xui_document document, unsigned sample)
{
    xui_doc_range_t range = test_find(document, "abcd");
    return document_list_split_source_samples[sample].offset == 0 ? range.tAnchor :
        document_list_split_source_samples[sample].offset == 4 ? range.tCaret : test_find(document, "cd").tAnchor;
}
static void list_split_source_oracle(xui_document document, unsigned sample, xui_doc_position_t caret)
{
    xui_document_snapshot snapshot; int order; xui_doc_node_info_t node = {0};
    caret.iRevision = xuiDocumentGetRevision(document); node.iSize = sizeof(node);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotComparePositions(snapshot, &caret, &caret, &order) == XUI_OK && !order);
    if (document_list_split_source_samples[sample].offset == 4) {
        uint64_t child, leaf, count, i; xui_doc_node_info_t empty = {0}; empty.iSize = sizeof(empty);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, caret.iNodeId, &node) == XUI_OK &&
            caret.iKind == XUI_DOC_POSITION_GAP && node.iKind == XUI_DOC_LIST_ITEM && !caret.iOffset && node.iChildCount <= 1);
        if (node.iChildCount) {
            CHECK(xuiDocumentSnapshotGetChild(snapshot, caret.iNodeId, 0, &child) == XUI_OK &&
                xuiDocumentSnapshotGetNode(snapshot, child, &empty) == XUI_OK && empty.iKind == XUI_DOC_PARAGRAPH);
            count = empty.iChildCount;
            for (i = 0; i < count; i++) {
                CHECK(xuiDocumentSnapshotGetChild(snapshot, child, i, &leaf) == XUI_OK &&
                    xuiDocumentSnapshotGetNode(snapshot, leaf, &node) == XUI_OK &&
                    node.iKind == XUI_DOC_TEXT && !node.iTextBytes);
            }
        }
    } else {
        char* text; uint64_t bytes;
        xui_doc_range_t gap = {caret, test_find(document, document_list_split_source_samples[sample].offset ? "cd" : "abcd").tAnchor};
        CHECK(xuiDocumentSnapshotCopyRange(snapshot, &gap, &text, &bytes) == XUI_OK && !bytes);
        xuiDocumentFreeBuffer(text);
    }
    xuiDocumentSnapshotRelease(snapshot);
    nested_prefix_oracle(document, document_list_split_source_samples[sample].expected);
}
static void markdown_list_split_source(void)
{
    unsigned sample;
    for (sample = 0; sample < sizeof(document_list_split_source_samples) / sizeof(*document_list_split_source_samples); sample++) {
        xui_document document = test_markdown_open(document_list_split_source_samples[sample].input);
        xui_document_transaction transaction; xui_document_snapshot before, after;
        xui_document_change_set change; xui_doc_position_t at = list_split_source_position(document, sample), caret;
        xui_doc_stats_t stats_before = {0}, stats_after = {0};
        xui_doc_node_info_t node = {0}, item_info = {0}, list_info = {0}, new_item_info = {0};
        uint64_t paragraph, item, list, item_index, block_index, i, id, other, new_item;
        int result;
        node.iSize = item_info.iSize = list_info.iSize = new_item_info.iSize = sizeof(node);
        CHECK(xuiDocumentAcquireSnapshot(document, &before) == XUI_OK &&
            xuiDocumentSnapshotGetNode(before, at.iNodeId, &node) == XUI_OK);
        paragraph = node.iParentId;
        CHECK(xuiDocumentSnapshotGetNode(before, paragraph, &node) == XUI_OK); item = node.iParentId;
        CHECK(xuiDocumentSnapshotGetNode(before, item, &item_info) == XUI_OK); list = item_info.iParentId;
        CHECK(xuiDocumentSnapshotGetNode(before, list, &list_info) == XUI_OK);
        for (item_index = 0; item_index < list_info.iChildCount; item_index++) {
            CHECK(xuiDocumentSnapshotGetChild(before, list, item_index, &id) == XUI_OK); if (id == item) break;
        }
        for (block_index = 0; block_index < item_info.iChildCount; block_index++) {
            CHECK(xuiDocumentSnapshotGetChild(before, item, block_index, &id) == XUI_OK); if (id == paragraph) break;
        }
        CHECK(item_index < list_info.iChildCount && block_index < item_info.iChildCount &&
            xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        stats_before.iSize = stats_after.iSize = sizeof(stats_before);
        CHECK(xuiDocumentGetStats(document, &stats_before) == XUI_OK);
        result = xuiDocumentTxnSplitListItem(transaction, &at, &caret);
        if (result != XUI_OK) fprintf(stderr, "List split source sample=%u result=%d\n", sample, result);
        CHECK(result == XUI_OK && xuiDocumentTxnCommit(transaction, &change) == XUI_OK);
        CHECK(xuiDocumentGetStats(document, &stats_after) == XUI_OK);
        if (sample == 21 || sample == 29) {
            uint64_t bytes = strlen(document_list_split_source_samples[sample].expected);
            CHECK(stats_after.iMarkdownParses == stats_before.iMarkdownParses + 1);
            CHECK(stats_after.iMarkdownParsedBytes == stats_before.iMarkdownParsedBytes + bytes);
        }
        xuiDocumentTxnRelease(transaction); test_source(document, document_list_split_source_samples[sample].expected, 1);
        list_split_source_oracle(document, sample, caret);
        CHECK(xuiDocumentAcquireSnapshot(document, &after) == XUI_OK &&
            xuiDocumentSnapshotGetNode(after, list, &node) == XUI_OK && node.iChildCount == list_info.iChildCount + 1 &&
            xuiDocumentSnapshotGetNode(after, item, &node) == XUI_OK && !memcmp(&node.tAttributes, &item_info.tAttributes, sizeof(node.tAttributes)) &&
            xuiDocumentSnapshotGetChild(after, list, item_index + 1, &new_item) == XUI_OK &&
            xuiDocumentSnapshotGetNode(after, new_item, &new_item_info) == XUI_OK);
        item_info.tAttributes.iFlags &= ~XUI_DOC_CHECKED;
        CHECK(!memcmp(&new_item_info.tAttributes, &item_info.tAttributes, sizeof(item_info.tAttributes)));
        for (i = 0; i < list_info.iChildCount; i++) if (i != item_index) {
            CHECK(xuiDocumentSnapshotGetChild(before, list, i, &id) == XUI_OK &&
                xuiDocumentSnapshotGetChild(after, list, i + (i > item_index), &other) == XUI_OK && id == other);
            expect_identity_tree(before, after, id); expect_moved_text_maps(document, change, before, id);
        }
        for (i = 0; i < item_info.iChildCount; i++) if (i != block_index) {
            CHECK(xuiDocumentSnapshotGetChild(before, item, i, &id) == XUI_OK);
            expect_identity_tree(before, after, id); expect_moved_text_maps(document, change, before, id);
        }
        xuiDocumentChangeSetRelease(change); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after);
        CHECK(xuiDocumentUndo(document, NULL) == XUI_OK); test_source(document, document_list_split_source_samples[sample].input, 1);
        CHECK(xuiDocumentRedo(document, NULL) == XUI_OK); test_source(document, document_list_split_source_samples[sample].expected, 1);
        xuiDocumentRelease(document);
    }
    puts("Markdown list split source: definitions, nested markers, task reset, CR/LF/CRLF/BOM, Tab/number-width tail indent, empty/heading/link items, stable siblings and moved block maps passed");
}
static void markdown_list_split_source_failures(void)
{
    unsigned sample;
    for (sample = 0; sample < sizeof(document_list_split_source_samples) / sizeof(*document_list_split_source_samples); sample++) {
        long point; int succeeded = 0;
        for (point = 0; point < 4096 && !succeeded; point++) {
            fail_allocator allocator = {-1, 0}; xui_doc_desc_t desc = {0};
            xui_document document; xui_document_transaction transaction = NULL;
            xui_document_snapshot before, after; xui_doc_position_t at, caret;
            uint64_t revision, token; unsigned notified = notifications; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &allocator;
            CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK && xuiDocumentLoadMarkdown(document,
                document_list_split_source_samples[sample].input, strlen(document_list_split_source_samples[sample].input)) == XUI_OK &&
                xuiDocumentClearHistory(document) == XUI_OK && xuiDocumentAcquireSnapshot(document, &before) == XUI_OK &&
                xuiDocumentMarkSaved(document, before) == XUI_OK && xuiDocumentSubscribe(document, changed, NULL, &token) == XUI_OK);
            at = list_split_source_position(document, sample); revision = xuiDocumentGetRevision(document);
            allocator.remaining = point;
            result = xuiDocumentBeginTransaction(document, NULL, &transaction);
            if (result == XUI_OK) result = xuiDocumentTxnSplitListItem(transaction, &at, &caret);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, NULL);
            if (transaction) xuiDocumentTxnRelease(transaction);
            allocator.remaining = -1;
            if (result == XUI_OK) {
                succeeded = 1; CHECK(notifications == notified + 1 && xuiDocumentCanUndo(document));
                test_source(document, document_list_split_source_samples[sample].expected, 1); list_split_source_oracle(document, sample, caret);
                CHECK(xuiDocumentUndo(document, NULL) == XUI_OK); test_source(document, document_list_split_source_samples[sample].input, 1);
                CHECK(xuiDocumentRedo(document, NULL) == XUI_OK); test_source(document, document_list_split_source_samples[sample].expected, 1);
            } else {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(document) == revision &&
                    !xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document) && !xuiDocumentIsDirty(document) && notifications == notified);
                test_source(document, document_list_split_source_samples[sample].input, 1);
                CHECK(xuiDocumentAcquireSnapshot(document, &after) == XUI_OK);
                expect_identity_tree(before, after, XUI_DOCUMENT_ROOT); nested_prefix_same_semantics(before, XUI_DOCUMENT_ROOT, after, XUI_DOCUMENT_ROOT);
                expect_same_syntax(before, after); nested_prefix_same_block_tokens(before, XUI_DOCUMENT_ROOT, after, XUI_DOCUMENT_ROOT);
                xuiDocumentSnapshotRelease(after);
            }
            xuiDocumentUnsubscribe(document, token); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(document); CHECK(!allocator.live);
        }
        CHECK(succeeded); printf("Markdown list split source OOM sample=%u: %ld attempts, atomic source/tree/tokens/history/notifications and leak-free\n", sample, point);
    }
}
