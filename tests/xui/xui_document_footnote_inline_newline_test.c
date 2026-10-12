#include "xui_document_footnote_inline_newline_cases.h"

int main(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"}; unsigned count = 0, rejected = 0, differences = 0;
    char label75[76], label76[77];
    memset(label75, 'q', 75); label75[75] = 0;
    memset(label76, 'r', 76); label76[76] = 0;
    const char* labels[] = {"n", "note", "longer_footnote_key", "脚注", label75, label76};
    for (unsigned fixture = 0; fixture < sizeof(newline_fixtures)/sizeof(*newline_fixtures); fixture++)
    for (unsigned label = 0; label < (fixture ? 6u : 1u); label++)
    for (unsigned eol = 0; eol < 3; eol++) for (unsigned bom = 0; bom < 2; bom++)
    for (unsigned eof = 0; eof < 2; eof++) for (unsigned mode = 0; mode < 2; mode++) {
        const struct newline_fixture* f = &newline_fixtures[fixture];
        char *raw_before = newline_label(f->before, labels[label]), *raw_after = newline_label(f->after, labels[label]);
        char *before = delete_ending(raw_before, endings[eol], bom), *after = delete_ending(raw_after, endings[eol], bom);
        free(raw_before); free(raw_after);
        if (eof) { before[strlen(before)-strlen(endings[eol])] = 0; after[strlen(after)-strlen(endings[eol])] = 0; }
        table_allocator allocator = {-1, 0}; xui_document d = table_open(before, &allocator), oracle = table_open(after, NULL);
        xui_document_snapshot old, current, expected, restored; xui_doc_position_t caret = {0};
        xui_document_change_set changes; xui_doc_range_t range; uint64_t bytes, revision; char raw[4096];
        boundary_case operation = {before, after, "\n", 2, 2, 2}; int result, exact;
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
            xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        range = boundary_selection(d, old, before, &operation, 0); revision = xuiDocumentGetRevision(d);
        printf("INLINE FOOTNOTE NEWLINE fixture=%u name=%s label=%s eol=%u bom=%u eof=%u command=%u\n", fixture, f->name, labels[label], eol, bom, eof, mode); fflush(stdout);
        if (fixture) { newline_footnote_scope(old, before, "abcd"); newline_footnote_scope(expected, after, "cd"); }
        result = newline_execute(d, &range, mode, &caret, &changes);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
        count++;
        if (result != XUI_OK) {
            rejected++;
            printf("REJECTED result=%d\n", result);
            TABLE_CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d));
            table_source(current, before); table_equivalent(current, old);
        } else {
            formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
            boundary_caret(d, current, &caret);
            TABLE_CHECK(xuiDocumentSnapshotCopySource(current, raw, sizeof(raw), &bytes) == XUI_OK && bytes < sizeof(raw));
            raw[bytes] = 0; integration_equivalent(current, raw);
            exact = bytes == strlen(after) && !memcmp(raw, after, (size_t)bytes);
            differences += !exact;
            if (!exact) { printf("RAW DIFFERENCE actual_hex="); for (uint64_t i = 0; i < bytes; i++) printf("%02x", (unsigned char)raw[i]); puts(""); }
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, raw); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
            xuiDocumentChangeSetRelease(changes);
        }
        table_source(old, before); xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle); xuiDocumentRelease(d); TABLE_CHECK(!allocator.live);
        free(before); free(after);
    }
    TABLE_CHECK(count == 600);
    printf("Inline footnote newline: %u command scenarios, %u rejections, %u raw differences;independent semantics/reparse/history/atomic rejection/zero leaks checked\n", count, rejected, differences);
    return rejected || differences ? 1 : 0;
}
