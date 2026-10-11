#include "xui_document_deep_quote_samples.h"
int main(void)
{
    const unsigned depths[] = {1, 2, 8, 32, 63, 64, 65, 87, 95, 120};
    const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned count = 0, rejected = 0, differences = 0;
    for (unsigned shape = 0; shape < 2; shape++)
    for (unsigned fixture = 0; fixture < sizeof(depths)/sizeof(depths[0]); fixture++) {
        char* input_lf = scaffold_source(depths[fixture], shape, 0);
        char* expected_lf = scaffold_source(depths[fixture], shape, 1);
        for (unsigned eol = 0; eol < 3; eol++) for (unsigned bom = 0; bom < 2; bom++)
        for (unsigned eof = 0; eof < 2; eof++) for (unsigned reverse = 0; reverse < 2; reverse++) {
            char* before = delete_ending(input_lf, endings[eol], (int)bom);
            char* after = delete_ending(expected_lf, endings[eol], (int)bom);
            table_allocator allocator = {-1, 0};
            xui_document d, oracle; xui_document_snapshot old, expected, current, restored;
            xui_document_transaction t; xui_document_change_set changes = NULL;
            xui_doc_range_t range; xui_doc_position_t caret;
            uint64_t bytes, revision; unsigned quotes = 0, lists = 0, items = 0;
            char* raw = NULL; int result, exact;
            if (eof) { before[strlen(before) - strlen(endings[eol])] = 0; after[strlen(after) - strlen(endings[eol])] = 0; }
            d = table_open(before, &allocator); oracle = table_open(after, NULL);
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
                xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
            range.tAnchor = scaffold_position(old, before, "left", 0, 2);
            range.tCaret = scaffold_position(old, before, "abcd", depths[fixture], 4);
            range.tAnchor.iAffinity = XUI_DOC_BEFORE; range.tCaret.iAffinity = XUI_DOC_AFTER;
            (void)scaffold_position(expected, after, "le", 0, 2);
            scaffold_counts(expected, XUI_DOCUMENT_ROOT, &quotes, &lists, &items);
            TABLE_CHECK(quotes == depths[fixture] && lists == 1 && items == 1);
            if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
            revision = xuiDocumentGetRevision(d);
            printf("VALID SCAFFOLD shape=%u depth=%u prefix=%u eol=%u bom=%u eof=%u reverse=%u\n",
                shape, depths[fixture], depths[fixture] * (shape ? 4 : 2) + 2, eol, bom, eof, reverse);
            fflush(stdout);
            TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
            result = xuiDocumentTxnReplaceRange(t, &range, "", 0, &caret);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, &changes);
            if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
            xuiDocumentTxnRelease(t);
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            count++;
            if (result != XUI_OK) {
                rejected++; printf("REJECTED %d\n", result);
                TABLE_CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d));
                table_source(current, before); table_equivalent(current, old);
            } else {
                formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
                TABLE_CHECK(xuiDocumentSnapshotCopySource(current, NULL, 0, &bytes) == XUI_OK && bytes < SIZE_MAX);
                raw = malloc((size_t)bytes + 1); TABLE_CHECK(raw);
                TABLE_CHECK(xuiDocumentSnapshotCopySource(current, raw, bytes + 1, &bytes) == XUI_OK);
                raw[bytes] = 0; integration_equivalent(current, raw); boundary_caret(d, current, &caret);
                exact = bytes == strlen(after) && !memcmp(raw, after, (size_t)bytes); differences += !exact;
                if (!exact) { printf("RAW DIFFERENCE actual_hex="); for (uint64_t i=0;i<bytes;i++) printf("%02x", (unsigned char)raw[i]); puts(""); }
                TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) &&
                    xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
                TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                table_source(restored, raw); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
                xuiDocumentChangeSetRelease(changes);
            }
            table_source(old, before); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current);
            xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle); xuiDocumentRelease(d);
            TABLE_CHECK(!allocator.live); free(raw); free(before); free(after);
        }
        free(input_lf); free(expected_lf);
    }
    TABLE_CHECK(count == 480);
    printf("Deep quote scaffold join: %u valid source/scope goldens, %u rejected, %u raw differences; full reparse/history/atomicity/zero leaks checked\n", count, rejected, differences);
    return rejected || differences ? 1 : 0;
}
