#include "xui_document_nested_list_samples.h"
int main(void)
{
    const unsigned depths[] = {1, 2, 8, 16, 23, 24, 25, 26, 32, 48, 50, 51, 52, 53, 54, 60};
    const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned count = 0, rejected = 0, differences = 0;
    for (unsigned ordered = 0; ordered < 2; ordered++)
    for (unsigned fixture = 0; fixture < sizeof(depths) / sizeof(depths[0]); fixture++) {
        char* input_lf = deep_list_source(depths[fixture], ordered, 0);
        char* expected_lf = deep_list_source(depths[fixture], ordered, 1);
        for (unsigned eol = 0; eol < 3; eol++) for (unsigned bom = 0; bom < 2; bom++)
        for (unsigned eof = 0; eof < 2; eof++) for (unsigned reverse = 0; reverse < 2; reverse++) {
        char* before = delete_ending(input_lf, endings[eol], (int)bom);
        char* after = delete_ending(expected_lf, endings[eol], (int)bom);
        if (eof) { before[strlen(before) - strlen(endings[eol])] = 0;
                   after[strlen(after) - strlen(endings[eol])] = 0; }
        table_allocator allocator = {-1, 0};
        xui_document document, oracle; xui_document_snapshot old, expected, current, restored;
        xui_document_transaction transaction; xui_document_change_set changes = NULL;
        xui_doc_range_t range; xui_doc_position_t caret, old_tail, new_tail;
        uint64_t bytes, revision; unsigned lists = 0, items = 0, quotes = 0;
        xui_doc_source_info_t info = {0};
        char* raw = NULL; int result, exact;
        document = table_open(before, &allocator); oracle = table_open(after, NULL);
        TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK &&
            xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
            xuiDocumentMarkSaved(document, old) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        range.tAnchor = deep_list_position(old, before, "left", 0, 2);
        range.tCaret = deep_list_position(old, before, "abcd", depths[fixture], 4);
        range.tAnchor.iAffinity = XUI_DOC_BEFORE; range.tCaret.iAffinity = XUI_DOC_AFTER;
        (void)deep_list_position(expected, after, "le", 0, 2);
        deep_list_counts(expected, XUI_DOCUMENT_ROOT, &lists, &items, &quotes);
        TABLE_CHECK(lists == depths[fixture] && items == depths[fixture] && quotes == 1);
        info.iSize = sizeof(info);
        TABLE_CHECK(xuiDocumentSnapshotGetSourceInfo(old, &info) == XUI_OK && info.iReferenceDefinitionCount == 1);
        TABLE_CHECK(xuiDocumentSnapshotGetSourceInfo(expected, &info) == XUI_OK && info.iReferenceDefinitionCount == 1);
        old_tail = deep_list_position(old, before, "end", 0, 3);
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        revision = xuiDocumentGetRevision(document);
        printf("VALID LIST TRIM ordered=%u depth=%u deepest_indent=%u eol=%u bom=%u eof=%u reverse=%u\n", ordered,
            depths[fixture], (depths[fixture] - 1) * 5, eol, bom, eof, reverse); fflush(stdout);
        TABLE_CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        result = xuiDocumentTxnReplaceRange(transaction, &range, "", 0, &caret);
        if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, &changes);
        if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(transaction, NULL) == result);
        xuiDocumentTxnRelease(transaction);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
        count++;
        if (result != XUI_OK) {
            rejected++; printf("REJECTED %d\n", result);
            TABLE_CHECK(xuiDocumentGetRevision(document) == revision &&
                !xuiDocumentCanUndo(document) && !xuiDocumentIsDirty(document));
            table_source(current, before); table_equivalent(current, old);
        } else {
            formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
            TABLE_CHECK(xuiDocumentSnapshotCopySource(current, NULL, 0, &bytes) == XUI_OK && bytes < SIZE_MAX);
            raw = malloc((size_t)bytes + 1); TABLE_CHECK(raw);
            TABLE_CHECK(xuiDocumentSnapshotCopySource(current, raw, bytes + 1, &bytes) == XUI_OK);
            raw[bytes] = 0; integration_equivalent(current, raw); boundary_caret(document, current, &caret);
            TABLE_CHECK(xuiDocumentSnapshotGetSourceInfo(current, &info) == XUI_OK && info.iReferenceDefinitionCount == 1);
            new_tail = deep_list_position(current, raw, "end", 0, 3);
            TABLE_CHECK(new_tail.iNodeId == old_tail.iNodeId);
            exact = bytes == strlen(after) && !memcmp(raw, after, (size_t)bytes); differences += !exact;
            if (!exact) { printf("RAW DIFFERENCE actual_hex=");
                for (uint64_t i = 0; i < bytes; i++) { printf("%02x", (unsigned char)raw[i]); }
                puts(""); }
            TABLE_CHECK(xuiDocumentUndo(document, NULL) == XUI_OK && !xuiDocumentCanUndo(document) &&
                !xuiDocumentIsDirty(document) && xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(document, NULL) == XUI_OK &&
                xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            table_source(restored, raw); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
            xuiDocumentChangeSetRelease(changes);
        }
        table_source(old, before); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle); xuiDocumentRelease(document);
        TABLE_CHECK(!allocator.live); free(raw); free(before); free(after);
        }
        free(input_lf); free(expected_lf);
    }
    TABLE_CHECK(count == 768);
    printf("Nested-list retained quote trim: %u valid independent source/scope goldens, %u rejected, %u raw differences; full reparse/history/atomicity/zero leaks checked\n",
        count, rejected, differences);
    return rejected || differences ? 1 : 0;
}
