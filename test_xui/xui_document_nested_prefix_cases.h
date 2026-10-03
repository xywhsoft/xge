#include "xui_document_nested_prefix_samples.h"
#ifndef NESTED_PREFIX_BASELINE_ONLY
static void nested_prefix_same_attributes(const xui_doc_node_info_t* a, const xui_doc_node_info_t* b)
{
    CHECK(a->iKind == b->iKind && !memcmp(&a->tAttributes, &b->tAttributes, sizeof(a->tAttributes)) &&
        !strcmp(a->sResource, b->sResource) && !strcmp(a->sInfo, b->sInfo) && !strcmp(a->sTitle, b->sTitle) &&
        !strcmp(a->sLinkTarget, b->sLinkTarget) && !strcmp(a->sLinkTitle, b->sLinkTitle));
}
static void nested_prefix_same_semantics(xui_document_snapshot actual, uint64_t id,
    xui_document_snapshot full, uint64_t other)
{
    xui_doc_node_info_t a = {0}, b = {0}; uint64_t i = 0, j = 0, ao = 0, bo = 0, bytes;
    char text[512], match[512];
    a.iSize = b.iSize = sizeof(a);
    CHECK(xuiDocumentSnapshotGetNode(actual, id, &a) == XUI_OK && xuiDocumentSnapshotGetNode(full, other, &b) == XUI_OK);
    nested_prefix_same_attributes(&a, &b);
    CHECK(a.iTextBytes == b.iTextBytes && a.iTextBytes < sizeof(text) &&
        xuiDocumentSnapshotCopyText(actual, id, text, sizeof(text), &bytes) == XUI_OK &&
        xuiDocumentSnapshotCopyText(full, other, match, sizeof(match), &bytes) == XUI_OK &&
        !memcmp(text, match, (size_t)a.iTextBytes));
    while (i < a.iChildCount || j < b.iChildCount) {
        xui_doc_node_info_t left = {0}, right = {0}; uint64_t child = 0, twin = 0;
        left.iSize = right.iSize = sizeof(left);
        if (i < a.iChildCount) CHECK(xuiDocumentSnapshotGetChild(actual, id, i, &child) == XUI_OK &&
            xuiDocumentSnapshotGetNode(actual, child, &left) == XUI_OK);
        if (j < b.iChildCount) CHECK(xuiDocumentSnapshotGetChild(full, other, j, &twin) == XUI_OK &&
            xuiDocumentSnapshotGetNode(full, twin, &right) == XUI_OK);
        if (child && left.iKind == XUI_DOC_TEXT && !left.iTextBytes) { i++; continue; }
        if (twin && right.iKind == XUI_DOC_TEXT && !right.iTextBytes) { j++; continue; }
        CHECK(child && twin);
        if (left.iKind == XUI_DOC_TEXT && right.iKind == XUI_DOC_TEXT) {
            uint64_t remaining = left.iTextBytes - ao, other_remaining = right.iTextBytes - bo;
            uint64_t count = remaining < other_remaining ? remaining : other_remaining;
            nested_prefix_same_attributes(&left, &right);
            CHECK(left.iTextBytes < sizeof(text) && right.iTextBytes < sizeof(match) &&
                xuiDocumentSnapshotCopyText(actual, child, text, sizeof(text), &bytes) == XUI_OK &&
                xuiDocumentSnapshotCopyText(full, twin, match, sizeof(match), &bytes) == XUI_OK &&
                !memcmp(text + ao, match + bo, (size_t)count));
            ao += count; bo += count;
            if (ao == left.iTextBytes) { i++; ao = 0; }
            if (bo == right.iTextBytes) { j++; bo = 0; }
        } else {
            CHECK(!ao && !bo); nested_prefix_same_semantics(actual, child, full, twin); i++; j++;
        }
    }
    CHECK(!ao && !bo);
}
static void nested_prefix_same_block_tokens(xui_document_snapshot actual, uint64_t id,
    xui_document_snapshot full, uint64_t other)
{
    xui_doc_node_info_t a = {0}, b = {0};
    xui_doc_block_syntax_t left = {0}, right = {0};
    uint64_t i, j, child, twin; int result;
    a.iSize = b.iSize = sizeof(a); left.iSize = right.iSize = sizeof(left);
    CHECK(xuiDocumentSnapshotGetNode(actual, id, &a) == XUI_OK &&
        xuiDocumentSnapshotGetNode(full, other, &b) == XUI_OK && a.iKind == b.iKind);
    CHECK(a.iSourceStart == b.iSourceStart && a.iSourceEnd == b.iSourceEnd &&
        a.iSyntaxStart == b.iSyntaxStart && a.iSyntaxEnd == b.iSyntaxEnd && a.bSourceExact == b.bSourceExact);
    result = xuiDocumentSnapshotGetBlockSyntax(actual, id, &left);
    CHECK(result == xuiDocumentSnapshotGetBlockSyntax(full, other, &right));
    if (result == XUI_OK) {
        CHECK(!memcmp(&left, &right, sizeof(left)));
        for (i = 0; i < left.iQuotePrefixCount; i++) {
            uint64_t start, end, other_start, other_end;
            CHECK(xuiDocumentSnapshotGetQuotePrefix(actual, id, i, &start, &end) == XUI_OK &&
                xuiDocumentSnapshotGetQuotePrefix(full, other, i, &other_start, &other_end) == XUI_OK &&
                start == other_start && end == other_end);
        }
        for (i = 0; i < left.iListIndentCount; i++) {
            xui_doc_list_indent_t indent = {0}, match = {0};
            indent.iSize = match.iSize = sizeof(indent);
            CHECK(xuiDocumentSnapshotGetListContinuationIndent(actual, id, i, &indent) == XUI_OK &&
                xuiDocumentSnapshotGetListContinuationIndent(full, other, i, &match) == XUI_OK &&
                !memcmp(&indent, &match, sizeof(indent)));
        }
    } else CHECK(result == XUI_ERROR_NOT_FOUND);
    /* A minimal merge can retain two adjacent, identically marked Text
     * leaves where a full parse emits one. Semantic equality checks their
     * text/attributes; compare block tokens independently of run boundaries. */
    for (i = j = 0; i < a.iChildCount || j < b.iChildCount;) {
        xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
        while (i < a.iChildCount) {
            CHECK(xuiDocumentSnapshotGetChild(actual, id, i, &child) == XUI_OK &&
                xuiDocumentSnapshotGetNode(actual, child, &node) == XUI_OK);
            if (node.iKind != XUI_DOC_TEXT) break;
            i++;
        }
        while (j < b.iChildCount) {
            CHECK(xuiDocumentSnapshotGetChild(full, other, j, &twin) == XUI_OK &&
                xuiDocumentSnapshotGetNode(full, twin, &node) == XUI_OK);
            if (node.iKind != XUI_DOC_TEXT) break;
            j++;
        }
        if (i == a.iChildCount && j == b.iChildCount) break;
        CHECK(i < a.iChildCount && j < b.iChildCount);
        nested_prefix_same_block_tokens(actual, child, full, twin);
        i++; j++;
    }
}
static void nested_prefix_oracle(xui_document document, const char* expected)
{
    xui_document oracle = test_markdown_open(expected);
    xui_document_snapshot actual, full;
    CHECK(xuiDocumentAcquireSnapshot(document, &actual) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &full) == XUI_OK);
    nested_prefix_same_semantics(actual, XUI_DOCUMENT_ROOT, full, XUI_DOCUMENT_ROOT);
    nested_prefix_same_block_tokens(actual, XUI_DOCUMENT_ROOT, full, XUI_DOCUMENT_ROOT);
    expect_same_syntax(actual, full);
    xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(full); xuiDocumentRelease(oracle);
}
#endif
static void nested_prefix_caret(xui_document document, xui_doc_position_t caret)
{
    xui_document_snapshot snapshot; int order; char* text; uint64_t bytes;
    xui_doc_range_t gap;
    caret.iRevision = xuiDocumentGetRevision(document);
    gap = (xui_doc_range_t){caret, test_find(document, "cd").tAnchor};
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotComparePositions(snapshot, &caret, &caret, &order) == XUI_OK && !order &&
        xuiDocumentSnapshotCopyRange(snapshot, &gap, &text, &bytes) == XUI_OK && !bytes);
    xuiDocumentFreeBuffer(text); xuiDocumentSnapshotRelease(snapshot);
}
/* Paragraph edits below multiple source containers must preserve their actual
 * marker chain, including hidden definition lines outside the edited body. */
static void markdown_nested_prefix_patch(void)
{
    unsigned sample;
    for (sample = 0; sample < sizeof(document_nested_prefix_samples) / sizeof(*document_nested_prefix_samples); sample++) {
        xui_document document = test_markdown_open(document_nested_prefix_samples[sample].input);
        xui_document_transaction transaction; xui_doc_range_t range = test_find(document, document_nested_prefix_samples[sample].merge ? "ab" : "abcd");
        xui_doc_position_t caret; int result;
        if (document_nested_prefix_samples[sample].merge) { range.tAnchor = range.tCaret; range.tCaret = test_find(document, "cd").tAnchor; }
        else { range.tAnchor.iOffset += 2; range.tCaret = range.tAnchor; }
        CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        result = xuiDocumentTxnReplaceRange(transaction, &range, document_nested_prefix_samples[sample].merge ? "" : "\n", document_nested_prefix_samples[sample].merge ? 0 : 1, &caret);
        if (result != XUI_OK) fprintf(stderr, "Nested prefix sample=%u split=%d\n", sample, result);
        CHECK(result == XUI_OK && xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        test_source(document, document_nested_prefix_samples[sample].expected, 1);
        nested_prefix_caret(document, caret);
#ifndef NESTED_PREFIX_BASELINE_ONLY
        nested_prefix_oracle(document, document_nested_prefix_samples[sample].expected);
#endif
        CHECK(xuiDocumentUndo(document, NULL) == XUI_OK); test_source(document, document_nested_prefix_samples[sample].input, 1);
        CHECK(xuiDocumentRedo(document, NULL) == XUI_OK); test_source(document, document_nested_prefix_samples[sample].expected, 1);
        xuiDocumentRelease(document);
    }
    puts("Markdown nested marker chain: list/quote ordering, ordered markers, CRLF, strong split and exact outside definitions/Undo/Redo passed");
}
#ifndef NESTED_PREFIX_BASELINE_ONLY
static void markdown_nested_prefix_failures(void)
{
    unsigned sample;
    for (sample = 0; sample < sizeof(document_nested_prefix_samples) / sizeof(*document_nested_prefix_samples); sample++) {
        long point; int succeeded = 0;
        for (point = 0; point < 4096 && !succeeded; point++) {
            fail_allocator allocator = {-1, 0}; xui_doc_desc_t desc = {0};
            xui_document document; xui_document_transaction transaction = NULL;
            xui_document_snapshot before, after; xui_doc_range_t range; xui_doc_position_t caret;
            uint64_t revision, token; unsigned notified = notifications; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &allocator;
            CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK && xuiDocumentLoadMarkdown(document,
                document_nested_prefix_samples[sample].input, strlen(document_nested_prefix_samples[sample].input)) == XUI_OK &&
                xuiDocumentClearHistory(document) == XUI_OK && xuiDocumentAcquireSnapshot(document, &before) == XUI_OK &&
                xuiDocumentMarkSaved(document, before) == XUI_OK && xuiDocumentSubscribe(document, changed, NULL, &token) == XUI_OK);
            range = test_find(document, document_nested_prefix_samples[sample].merge ? "ab" : "abcd");
            if (document_nested_prefix_samples[sample].merge) { range.tAnchor = range.tCaret; range.tCaret = test_find(document, "cd").tAnchor; }
            else { range.tAnchor.iOffset += 2; range.tCaret = range.tAnchor; }
            revision = xuiDocumentGetRevision(document); allocator.remaining = point;
            result = xuiDocumentBeginTransaction(document, NULL, &transaction);
            if (result == XUI_OK) result = xuiDocumentTxnReplaceRange(transaction, &range,
                document_nested_prefix_samples[sample].merge ? "" : "\n", document_nested_prefix_samples[sample].merge ? 0 : 1, &caret);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, NULL);
            if (transaction) xuiDocumentTxnRelease(transaction);
            allocator.remaining = -1;
            if (result == XUI_OK) {
                succeeded = 1; CHECK(notifications == notified + 1 && xuiDocumentCanUndo(document));
                test_source(document, document_nested_prefix_samples[sample].expected, 1); nested_prefix_caret(document, caret);
                nested_prefix_oracle(document, document_nested_prefix_samples[sample].expected);
                CHECK(xuiDocumentUndo(document, NULL) == XUI_OK); test_source(document, document_nested_prefix_samples[sample].input, 1);
                CHECK(xuiDocumentRedo(document, NULL) == XUI_OK); test_source(document, document_nested_prefix_samples[sample].expected, 1);
            } else {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(document) == revision &&
                    !xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document) && !xuiDocumentIsDirty(document) && notifications == notified);
                test_source(document, document_nested_prefix_samples[sample].input, 1);
                CHECK(xuiDocumentAcquireSnapshot(document, &after) == XUI_OK);
                expect_identity_tree(before, after, XUI_DOCUMENT_ROOT); expect_subtree_copy(before, XUI_DOCUMENT_ROOT, after, XUI_DOCUMENT_ROOT, 0);
                expect_same_syntax(before, after); nested_prefix_same_block_tokens(before, XUI_DOCUMENT_ROOT, after, XUI_DOCUMENT_ROOT);
                xuiDocumentSnapshotRelease(after);
            }
            xuiDocumentUnsubscribe(document, token); xuiDocumentSnapshotRelease(before);
            xuiDocumentRelease(document); CHECK(!allocator.live);
        }
        CHECK(succeeded);
        printf("Markdown nested prefix OOM sample=%u: %ld attempts, atomic source/tree/tokens/history/notifications and leak-free\n", sample, point);
    }
}
#endif
