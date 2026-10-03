#include "xui_document_code_language_source_samples.h"
static void markdown_code_language_source(void)
{
    unsigned sample;
    for (sample = 0; sample < sizeof(code_language_source_samples) / sizeof(*code_language_source_samples); sample++) {
        xui_document d = test_markdown_open(code_language_source_samples[sample].input), oracle;
        xui_document_snapshot before, after, loaded; xui_document_transaction t; xui_document_change_set change;
        uint64_t code, i, count; xui_doc_node_info_t info = {0}; xui_doc_block_syntax_t syntax = {0};
        xui_doc_change_info_t changed = {0}; changed.iSize = sizeof(changed);
        int source_ops = 0; info.iSize = sizeof(info); syntax.iSize = sizeof(syntax);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK); code = find_kind(before, 1, XUI_DOC_CODE_BLOCK); CHECK(code);
        CHECK(xuiDocumentSnapshotGetBlockSyntax(before, code, &syntax) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnSetCodeBlockLanguage(t, code, code_language_source_samples[sample].language) == XUI_OK &&
            xuiDocumentTxnCommit(t, &change) == XUI_OK);
        xuiDocumentTxnRelease(t); test_source(d, code_language_source_samples[sample].expected, 1);
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK && xuiDocumentSnapshotGetNode(after, code, &info) == XUI_OK &&
            !strcmp(info.sInfo, code_language_source_samples[sample].language));
        CHECK(xuiDocumentChangeSetGetInfo(change, &changed) == XUI_OK); count = changed.iOperationCount;
        for (i = 0; i < count; i++) {
            xui_doc_operation_t op = changed.pOperations[i];
            if (op.iKind == XUI_DOC_OP_SOURCE) {
                CHECK(op.iOffset == syntax.iFenceLanguageStart && op.iOldLength == syntax.iFenceLanguageEnd - syntax.iFenceLanguageStart);
                source_ops++;
            }
        }
        CHECK(source_ops == 1);
        oracle = test_markdown_open(code_language_source_samples[sample].expected);
        CHECK(xuiDocumentAcquireSnapshot(oracle, &loaded) == XUI_OK); inc_snapshot_equal(after, loaded);
        xuiDocumentSnapshotRelease(loaded); xuiDocumentRelease(oracle); xuiDocumentChangeSetRelease(change);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); test_source(d, code_language_source_samples[sample].input, 1);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); test_source(d, code_language_source_samples[sample].expected, 1);
        {
            char* original; uint64_t bytes = strlen(code_language_source_samples[sample].input), copied;
            original = malloc((size_t)bytes + 1); CHECK(original && xuiDocumentSnapshotCopySource(before, original, bytes + 1, &copied) == XUI_OK &&
                copied == bytes && !strcmp(original, code_language_source_samples[sample].input)); free(original);
        }
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    }
    {
        const char* source = "~~~~c opaque=1\r\nabc\r\n~~~~~\r\n";
        xui_document d = test_markdown_open(source); xui_document_snapshot before, after; xui_document_transaction t;
        uint64_t code, revision = xuiDocumentGetRevision(d); int can_undo = xuiDocumentCanUndo(d);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK); code = find_kind(before, 1, XUI_DOC_CODE_BLOCK);
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnSetCodeBlockLanguage(t, code, "") == XUI_DOC_ERROR_UNREPRESENTABLE);
        xuiDocumentTxnRelease(t); CHECK(xuiDocumentGetRevision(d) == revision && xuiDocumentCanUndo(d) == can_undo);
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); expect_identity_tree(before, after, 1); expect_same_syntax(before, after);
        test_source(d, source, 1); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    }
    puts("Code language source: 13 root/quote/list/footnote/BOM/CR/LF/CRLF/EOF/empty/entity/backslash/Unicode cases, one field patch, stable code ID, full-load oracle, snapshot/history and metadata-clear rejection passed");
}
static void markdown_code_language_source_failures(void)
{
    unsigned fixture; long total = 0;
    for (fixture = 0; fixture < 2; fixture++) {
        const unsigned sample = fixture ? 2 : 0; long point; int success = 0;
        for (point = 0; point < 2048 && !success; point++) {
            fail_allocator allocation = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t = NULL;
            xui_document_snapshot before, after; uint64_t code, revision; int result, can_undo; unsigned baseline;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &allocation;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, code_language_source_samples[sample].input,
                strlen(code_language_source_samples[sample].input)) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            code = find_kind(before, 1, XUI_DOC_CODE_BLOCK); revision = xuiDocumentGetRevision(d); can_undo = xuiDocumentCanUndo(d); baseline = allocation.live;
            allocation.remaining = point; result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (result == XUI_OK) result = xuiDocumentTxnSetCodeBlockLanguage(t, code, code_language_source_samples[sample].language);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            xuiDocumentTxnRelease(t); allocation.remaining = -1;
            if (result == XUI_OK) { success = 1; test_source(d, code_language_source_samples[sample].expected, 1); }
            else {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && xuiDocumentCanUndo(d) == can_undo && allocation.live == baseline &&
                    xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
                expect_identity_tree(before, after, 1); expect_same_syntax(before, after); xuiDocumentSnapshotRelease(after);
                test_source(d, code_language_source_samples[sample].input, 1);
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); CHECK(!allocation.live);
        }
        CHECK(success); total += point;
    }
    printf("Code language source OOM: root/nested %ld attempts, atomic source/tree/syntax/history and leak-free cleanup passed\n", total);
}
