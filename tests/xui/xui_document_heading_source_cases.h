#include "xui_document_heading_source_samples.h"
static xui_doc_range_t heading_source_range(xui_document d, unsigned sample)
{
    xui_doc_range_t range = {0};
    if (sample == 19 || sample == 20) {
        range.tAnchor.iSize = sizeof(range.tAnchor);
        xui_document_snapshot s; CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
        range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
        range.tAnchor.iNodeId = find_kind(s, 1, XUI_DOC_HEADING); range.tAnchor.iKind = XUI_DOC_POSITION_GAP;
        range.tAnchor.iAffinity = XUI_DOC_AFTER; range.tCaret = range.tAnchor; xuiDocumentSnapshotRelease(s);
    } else range = test_find(d, "abcd");
    return range;
}
static void markdown_heading_source(void)
{
    unsigned sample;
    for (sample = 0; sample < sizeof(heading_source_samples) / sizeof(*heading_source_samples); sample++) {
        xui_document d = test_markdown_open(heading_source_samples[sample].input), oracle;
        xui_document_snapshot before, after, loaded; xui_document_transaction t; xui_doc_range_t range, mapped;
        uint64_t heading; xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK); heading = find_kind(before, 1, XUI_DOC_HEADING);
        if (heading_source_samples[sample].raw_content) {
            xui_doc_block_syntax_t syntax = {0}; char raw[1024]; uint64_t bytes;
            syntax.iSize = sizeof(syntax);
            CHECK(heading && xuiDocumentSnapshotGetBlockSyntax(before, heading, &syntax) == XUI_OK);
            bytes = syntax.iHeadingContentEnd - syntax.iHeadingContentStart;
            CHECK(bytes == strlen(heading_source_samples[sample].raw_content) && bytes < sizeof(raw) &&
                xuiDocumentSnapshotReadSource(before, syntax.iHeadingContentStart, raw, bytes) == XUI_OK &&
                !memcmp(raw, heading_source_samples[sample].raw_content, (size_t)bytes));
        }
        range = heading_source_range(d, sample);
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        { int result = xuiDocumentTxnSetHeading(t, &range, heading_source_samples[sample].level, &mapped);
          if (result != XUI_OK) fprintf(stderr, "Heading source sample=%u result=%d\n", sample, result);
          CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK); }
        xuiDocumentTxnRelease(t); test_source(d, heading_source_samples[sample].expected, 1);
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        if (heading) CHECK(xuiDocumentSnapshotGetNode(after, heading, &node) == XUI_OK &&
            node.iKind == (heading_source_samples[sample].level ? XUI_DOC_HEADING : XUI_DOC_PARAGRAPH));
        oracle = test_markdown_open(heading_source_samples[sample].expected);
        CHECK(xuiDocumentAcquireSnapshot(oracle, &loaded) == XUI_OK); inc_snapshot_equal(after, loaded);
        xuiDocumentSnapshotRelease(loaded); xuiDocumentRelease(oracle);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); test_source(d, heading_source_samples[sample].input, 1);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); test_source(d, heading_source_samples[sample].expected, 1);
        {
            char raw[2048]; uint64_t bytes;
            CHECK(xuiDocumentSnapshotCopySource(before, raw, sizeof(raw), &bytes) == XUI_OK &&
                !strcmp(raw, heading_source_samples[sample].input));
        }
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    }
    {
        const char* input = "## **abcd** &amp; ###\r\n\r\nefgh &amp;\r\n===== \t\r\n\r\n[r]: /unused 'Title'\r\n";
        const char* expected = "### **abcd** &amp; ###\r\n\r\n### efgh &amp;\r\n\r\n[r]: /unused 'Title'\r\n";
        xui_document d = test_markdown_open(input), oracle; xui_document_transaction t; xui_document_snapshot a, b;
        xui_doc_range_t range = test_find(d, "abcd"), last = test_find(d, "efgh"), mapped; range.tCaret = last.tCaret;
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnSetHeading(t, &range, 3, &mapped) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t); test_source(d, expected, 1);
        oracle = test_markdown_open(expected); CHECK(xuiDocumentAcquireSnapshot(d, &a) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &b) == XUI_OK); inc_snapshot_equal(a, b);
        xuiDocumentSnapshotRelease(a); xuiDocumentSnapshotRelease(b); xuiDocumentRelease(oracle); xuiDocumentRelease(d);
    }
    {
        const char* input = "[^a]: ## abcd &amp; ###\r\n\r\n[^b]: efgh &amp;\r\n    ===== \t\r\n\r\nuse [^a] [^b]\r\n\r\n[r]: /unused 'Title'\r\n";
        const char* expected = "[^a]: ### abcd &amp; ###\r\n\r\n[^b]: ### efgh &amp;\r\n\r\nuse [^a] [^b]\r\n\r\n[r]: /unused 'Title'\r\n";
        xui_document d = test_markdown_open(input), oracle; xui_document_transaction t; xui_document_snapshot a, b;
        xui_doc_range_t range = test_find(d, "abcd"), last = test_find(d, "efgh"), mapped; range.tCaret = last.tCaret;
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnSetHeading(t, &range, 3, &mapped) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t); test_source(d, expected, 1);
        oracle = test_markdown_open(expected); CHECK(xuiDocumentAcquireSnapshot(d, &a) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &b) == XUI_OK); inc_snapshot_equal(a, b);
        xuiDocumentSnapshotRelease(a); xuiDocumentSnapshotRelease(b); xuiDocumentRelease(oracle); xuiDocumentRelease(d);
    }
    {
        const char prefix[] = "abcd &amp;\r\n", tail[] = " \t\r\n"; size_t length = 70000, bytes = sizeof(prefix) - 1 + length + sizeof(tail) - 1;
        char* source = malloc(bytes + 1); xui_document d, oracle; xui_document_snapshot a, b; xui_document_transaction t;
        xui_doc_range_t range, mapped;
        CHECK(source); memcpy(source, prefix, sizeof(prefix) - 1); memset(source + sizeof(prefix) - 1, '=', length);
        memcpy(source + sizeof(prefix) - 1 + length, tail, sizeof(tail)); d = test_markdown_open(source); range = test_find(d, "abcd");
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnSetHeading(t, &range, 2, &mapped) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t); memset(source + sizeof(prefix) - 1, '-', length); test_source(d, source, 1);
        oracle = test_markdown_open(source); CHECK(xuiDocumentAcquireSnapshot(d, &a) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &b) == XUI_OK);
        inc_snapshot_equal(a, b); xuiDocumentSnapshotRelease(a); xuiDocumentSnapshotRelease(b); xuiDocumentRelease(oracle);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); memset(source + sizeof(prefix) - 1, '=', length); test_source(d, source, 1);
        xuiDocumentRelease(d); free(source);
    }
    puts("Heading source: 24 source/editor fixtures, root and multiple-footnote mixed ranges, 70,000-byte underline, parser content bounds, full-load oracle, stable IDs and snapshot/history passed");
}
static void markdown_heading_source_failures(void)
{
    const unsigned samples[] = {1, 5, 15}; unsigned fixture; long total = 0;
    for (fixture = 0; fixture < sizeof(samples) / sizeof(*samples); fixture++) {
        unsigned sample = samples[fixture]; long point; int success = 0;
        for (point = 0; point < 2048 && !success; point++) {
            fail_allocator allocation = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t = NULL;
            xui_document_snapshot before, after; xui_doc_range_t range, mapped; uint64_t revision; int result, can_undo; unsigned baseline;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &allocation;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, heading_source_samples[sample].input,
                strlen(heading_source_samples[sample].input)) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = heading_source_range(d, sample); revision = xuiDocumentGetRevision(d); can_undo = xuiDocumentCanUndo(d); baseline = allocation.live;
            allocation.remaining = point; result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (result == XUI_OK) result = xuiDocumentTxnSetHeading(t, &range, heading_source_samples[sample].level, &mapped);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            xuiDocumentTxnRelease(t); allocation.remaining = -1;
            if (result == XUI_OK) { success = 1; test_source(d, heading_source_samples[sample].expected, 1); }
            else {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && xuiDocumentCanUndo(d) == can_undo && allocation.live == baseline &&
                    xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
                expect_identity_tree(before, after, 1); expect_same_syntax(before, after); xuiDocumentSnapshotRelease(after);
                test_source(d, heading_source_samples[sample].input, 1);
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); CHECK(!allocation.live);
        }
        CHECK(success); total += point;
    }
    printf("Heading source OOM: root Setext/nested ATX clear/footnote Setext-to-ATX %ld attempts, atomic source/tree/syntax/history and leak-free cleanup passed\n", total);
}
#ifndef XUI_DLL
static void markdown_heading_source_cancellation(void)
{
    const unsigned samples[] = {1, 15}; unsigned fixture; long total = 0;
    for (fixture = 0; fixture < sizeof(samples) / sizeof(*samples); fixture++) {
        unsigned sample = samples[fixture]; long point; int success = 0;
        for (point = 0; point < 2048 && !success; point++) {
            reference_values_cancel_allocator allocation = {{-1, 0}, -1, ATOMIC_VAR_INIT(0)};
            xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t;
            xui_document_snapshot before, after; xui_doc_range_t range, mapped; uint64_t revision; int result; unsigned baseline;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = reference_values_cancel_alloc; desc.onFree = reference_values_cancel_free; desc.pAllocatorUser = &allocation;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, heading_source_samples[sample].input,
                strlen(heading_source_samples[sample].input)) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = heading_source_range(d, sample); revision = xuiDocumentGetRevision(d); baseline = allocation.allocator.live;
            CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); t->cancellation = &allocation.cancellation;
            allocation.remaining = point;
            result = xuiDocumentTxnSetHeading(t, &range, heading_source_samples[sample].level, &mapped);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            allocation.remaining = -1; xuiDocumentTxnRelease(t); atomic_store(&allocation.cancellation, 0);
            if (result == XUI_OK) { success = 1; test_source(d, heading_source_samples[sample].expected, 1); }
            else {
                CHECK(result == XUI_DOC_ERROR_CANCELLED && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) &&
                    allocation.allocator.live == baseline && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
                expect_identity_tree(before, after, 1); expect_same_syntax(before, after); xuiDocumentSnapshotRelease(after);
                test_source(d, heading_source_samples[sample].input, 1);
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); CHECK(!allocation.allocator.live);
        }
        CHECK(success); total += point;
    }
    printf("Heading source cancellation: Setext and footnote conversion %ld allocator checkpoints, atomic published state and leak-free cleanup passed\n", total);
}
#endif
