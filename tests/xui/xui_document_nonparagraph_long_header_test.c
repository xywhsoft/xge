#include "xui_document_nonparagraph_prefix_samples.h"
static char* lengthen_first_header(char* source, unsigned variant, size_t target)
{
    char* start = strstr(source, "\n\n"), *end;
    size_t bytes = strlen(source), length, added, offset;
    TABLE_CHECK(start && variant < 7);
    start += 2; end = strchr(start, '\n'); TABLE_CHECK(end);
    length = (size_t)(end - start);
    TABLE_CHECK(target > length + 1 && target <= 8192);
    added = target - length; offset = (size_t)(end - source);
    char* result = malloc(bytes + added + 1); TABLE_CHECK(result);
    memcpy(result, source, offset);
    memset(result + offset, variant == 4 ? '-' : 'x', added);
    if (variant == 2 || variant == 3) result[offset] = ' ';
    memcpy(result + offset + added, end, bytes - offset + 1);
    TABLE_CHECK((size_t)(strchr(result + (start - source), '\n') - (result + (start - source))) == target);
    free(source);
    return result;
}

int main(void)
{
    const unsigned depths[] = {2, 53};
    const size_t header_lengths[] = {4094, 4095, 4096, 8190, 8191, 8192};
    const char* endings[] = {"\n", "\r\n", "\r"};
    const uint32_t kinds[] = {XUI_DOC_HEADING, XUI_DOC_QUOTE, XUI_DOC_CODE_BLOCK,
        XUI_DOC_DIAGRAM, XUI_DOC_RULE, XUI_DOC_HTML, XUI_DOC_TABLE};
    unsigned count = 0, rejected = 0, differences = 0;
    for (unsigned variant = 0; variant < 7; variant++)
    for (unsigned ordered = 0; ordered < 2; ordered++)
    for (unsigned fixture = 0; fixture < sizeof(depths) / sizeof(depths[0]); fixture++) {
        for (unsigned header = 0; header < 6; header++)
        for (unsigned eol = 0; eol < 3; eol++) for (unsigned bom = 0; bom < 2; bom++) {
        char* input_lf = lengthen_first_header(deep_list_source(depths[fixture], ordered, variant, 0), variant, header_lengths[header]);
        char* expected_lf = deep_list_source(depths[fixture], ordered, variant, 1);
        char* before = delete_ending(input_lf, endings[eol], (int)bom);
        char* after = delete_ending(expected_lf, endings[eol], (int)bom);
        free(input_lf); free(expected_lf);
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
        TABLE_CHECK(selected_block_count(old, XUI_DOCUMENT_ROOT, kinds[variant]) == depths[fixture] - 1 + (variant == 1));
        TABLE_CHECK(selected_block_count(expected, XUI_DOCUMENT_ROOT, kinds[variant]) == (variant == 1));
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
        revision = xuiDocumentGetRevision(document);
        printf("VALID NONPARAGRAPH kind=%u variant=%u ordered=%u depth=%u header_bytes=%zu eol=%u bom=%u\n",
            kinds[variant], variant, ordered, depths[fixture], header_lengths[header], eol, bom); fflush(stdout);
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
    }
    TABLE_CHECK(count == 1008);
    printf("Nonparagraph long-header trim: %u valid independent source/scope goldens, %u rejected, %u raw differences; full reparse/history/atomicity/zero leaks checked\n",
        count, rejected, differences);
    return rejected || differences ? 1 : 0;
}
