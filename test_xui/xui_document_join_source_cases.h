#ifndef XUI_DOCUMENT_JOIN_SOURCE_CASES_H
#define XUI_DOCUMENT_JOIN_SOURCE_CASES_H
#define XUI_DOC_FOOTNOTE_LOCAL_REFERENCE_HELPERS_ONLY
#include "test_xui/xui_document_footnote_local_reference_diagnostic_test.c"
#undef XUI_DOC_FOOTNOTE_LOCAL_REFERENCE_HELPERS_ONLY

static const struct join_sample { const char *name, *before, *after; } join_samples[] = {
    {"root-entities-marks", "ab &amp; __keep__\n\n!cd &lt; **tail**\n\n[unused]: /hold 'Keep'\n",
     "ab &amp; __keep__!cd &lt; **tail**\n\n[unused]: /hold 'Keep'\n"},
    {"root-reference", "[ab][r] &amp;\n\n!cd &lt; __keep__\n\n[r]: <raw> 'Title'\n",
     "[ab][r] &amp;!cd &lt; __keep__\n\n[r]: <raw> 'Title'\n"},
    {"quote-reference", "> [ab][r] &amp;\n>\n> !cd &lt; __keep__\n\n[r]: <raw> 'Title'\n",
     "> [ab][r] &amp;!cd &lt; __keep__\n\n[r]: <raw> 'Title'\n"},
    {"footnote-reference", "use[^n]\n\n[^n]:\n    [ab][r] &amp;\n    \n    !cd &lt; __keep__\n    \n    [r]: <raw> 'Title'\n",
     "use[^n]\n\n[^n]:\n    [ab][r] &amp;!cd &lt; __keep__\n    \n    [r]: <raw> 'Title'\n"},
    {"footnote-first-line", "use[^n]\n\n[^n]: [ab][r] &amp;\n    \n    !cd &lt; __keep__\n    \n    [r]: <raw> 'Title'\n",
     "use[^n]\n\n[^n]: [ab][r] &amp;!cd &lt; __keep__\n    \n    [r]: <raw> 'Title'\n"},
    {"quote-hidden-gap", "> [ab][r] &amp;\n>\n> [r]: <raw> 'Title'\n>\n> !cd &lt; __keep__\n",
     "> [ab][r] &amp;!cd &lt; __keep__\n>\n> [r]: <raw> 'Title'\n>\n"}
};

static void join_positions(xui_document_snapshot old, const char* before,
    uint64_t* left, uint64_t* right, xui_doc_position_t* tail)
{
    xui_doc_position_t p; xui_doc_node_info_t n = {0}; int quality;
    const char *a = strstr(before, "ab"), *b = strstr(before, "!cd");
    TABLE_CHECK(a && b);
    TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(a - before) + 1, &p, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
    n.iSize = sizeof(n); TABLE_CHECK(xuiDocumentSnapshotGetNode(old, p.iNodeId, &n) == XUI_OK); *left = n.iParentId;
    TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(b - before) + 1, tail, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(old, tail->iNodeId, &n) == XUI_OK); *right = n.iParentId;
}
static int join_execute(xui_document d, xui_document_snapshot old, uint64_t left, uint64_t right,
    unsigned mode, xui_doc_position_t* caret, xui_document_change_set* change)
{
    xui_document_transaction t; int result = xuiDocumentBeginTransaction(d, NULL, &t);
    if (result != XUI_OK) return result;
    if (!mode) result = xuiDocumentTxnJoinBlocks(t, left, right, caret);
    else {
        xui_doc_range_t range = {0}; xui_doc_node_info_t n = {0}; uint64_t child;
        n.iSize = sizeof(n); TABLE_CHECK(xuiDocumentSnapshotGetNode(old, left, &n) == XUI_OK && n.iChildCount);
        range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d);
        range.tAnchor.iRevision = xuiDocumentGetRevision(d); range.tAnchor.iAffinity = XUI_DOC_AFTER;
        range.tAnchor.iNodeId = left; range.tAnchor.iOffset = n.iChildCount; range.tAnchor.iKind = XUI_DOC_POSITION_GAP;
        if (mode >= 3) {
            TABLE_CHECK(xuiDocumentSnapshotGetChild(old, left, n.iChildCount - 1, &child) == XUI_OK &&
                xuiDocumentSnapshotGetNode(old, child, &n) == XUI_OK && n.iKind == XUI_DOC_TEXT);
            range.tAnchor.iNodeId = child; range.tAnchor.iOffset = n.iTextBytes; range.tAnchor.iKind = XUI_DOC_POSITION_TEXT;
        }
        range.tCaret = range.tAnchor; range.tCaret.iNodeId = right; range.tCaret.iOffset = 0;
        range.tCaret.iKind = XUI_DOC_POSITION_GAP;
        if (mode >= 3) {
            TABLE_CHECK(xuiDocumentSnapshotGetChild(old, right, 0, &child) == XUI_OK);
            range.tCaret.iNodeId = child; range.tCaret.iKind = XUI_DOC_POSITION_TEXT;
        }
        if (mode == 2 || mode == 4) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        result = xuiDocumentTxnReplaceRange(t, &range, "", 0, caret);
    }
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, change);
    if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
    xuiDocumentTxnRelease(t); return result;
}
static void join_goldens(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"}; unsigned total = 0;
    for (unsigned fixture = 0; fixture < sizeof(join_samples) / sizeof(*join_samples); fixture++)
    for (unsigned eol = 0; eol < 3; eol++) for (unsigned bom = 0; bom < 2; bom++)
    for (unsigned eof = 0; eof < 2; eof++) for (unsigned mode = 0; mode < 5; mode++) {
        char *before = delete_ending(join_samples[fixture].before, endings[eol], bom),
            *after = delete_ending(join_samples[fixture].after, endings[eol], bom);
        if (eof) {
            before[strlen(before) - strlen(endings[eol])] = 0;
            if (fixture == 5) {
                /* The old definition gap (including its final original EOL)
                 * moves after the EOF payload. Only the new boundary is LF. */
                char* boundary = strstr(after, endings[eol]); TABLE_CHECK(boundary);
                size_t width = strlen(endings[eol]);
                memmove(boundary + 1, boundary + width, strlen(boundary + width) + 1); *boundary = '\n';
            } else after[strlen(after) - strlen(endings[eol])] = 0;
        }
        table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation);
        xui_document_snapshot old, current, restored; xui_document_change_set change;
        xui_doc_position_t tail, mapped, caret; uint64_t left, right, at; int quality;
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
        join_positions(old, before, &left, &right, &tail);
        printf("Join golden fixture=%u eol=%u bom=%u eof=%u mode=%u\n", fixture, eol, bom, eof, mode); fflush(stdout);
        TABLE_CHECK(join_execute(d, old, left, right, mode, &caret, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
        table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
        TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
            mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset &&
            xuiDocumentPositionToSource(current, &mapped, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
            at == (uint64_t)(strstr(after, "!cd") - after) + 1);
        xuiDocumentChangeSetRelease(change);
        TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
        xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentRelease(d); TABLE_CHECK(!allocation.live);
        free(before); free(after); total++;
    }
    TABLE_CHECK(total == 360);
    printf("Join source matrix: %u independent whole-source/full-semantic/index/stable-tail/caret/common-history/zero-leak goldens passed\n", total);
}
static void join_faults(void)
{
    for (unsigned fixture = 0; fixture < sizeof(join_samples) / sizeof(*join_samples); fixture++)
    for (unsigned mode = 0; mode < 2; mode++) {
        char *before = delete_ending(join_samples[fixture].before, "\r\n", 1),
            *after = delete_ending(join_samples[fixture].after, "\r\n", 1); long point;
        for (point = 0; point < 4096; point++) {
            table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation);
            xui_document_snapshot old, current, restored; xui_doc_position_t tail, caret;
            uint64_t left, right, revision; int result;
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
            join_positions(old, before, &left, &right, &tail); revision = xuiDocumentGetRevision(d);
            allocation.remaining = point; result = join_execute(d, old, left, right, mode, &caret, NULL); allocation.remaining = -1;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision &&
                    !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && !xuiDocumentIsDirty(d));
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(join_execute(d, old, left, right, mode, &caret, NULL) == XUI_OK); table_source(current, before);
                xuiDocumentSnapshotRelease(current); TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentCanUndo(d) && xuiDocumentIsDirty(d));
            table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
            xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentRelease(d); TABLE_CHECK(!allocation.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 4096); printf("Join source fixture=%u mode=%u: %ld OOM points atomic/poisoned commit/same-document retry/history/zero leaks passed\n", fixture, mode, point);
        free(before); free(after);
    }
}


static int join_continuations(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"}; unsigned total = 0;
    for (unsigned fixture = 0; fixture < sizeof(join_samples) / sizeof(*join_samples); fixture++)
    for (unsigned eol = 0; eol < 3; eol++) for (unsigned bom = 0; bom < 2; bom++)
    for (unsigned eof = 0; eof < 2; eof++) for (unsigned compound = 0; compound < 2; compound++) {
        char *before = delete_ending(join_samples[fixture].before, endings[eol], bom),
            *joined = delete_ending(join_samples[fixture].after, endings[eol], bom);
        if (eof) {
            before[strlen(before) - strlen(endings[eol])] = 0;
            if (fixture == 5) {
                char* boundary = strstr(joined, endings[eol]); TABLE_CHECK(boundary);
                size_t width = strlen(endings[eol]);
                memmove(boundary + 1, boundary + width, strlen(boundary + width) + 1); *boundary = '\n';
            } else joined[strlen(joined) - strlen(endings[eol])] = 0;
        }
        /* Join returns the left run's terminal TEXT caret. The first fixture's
         * left run is bold; typing stays inside its original closing marker. */
        char* junction = strstr(joined, fixture == 0 ? "__!cd" : "!cd"); TABLE_CHECK(junction);
        size_t insertion = (size_t)(junction - joined); char* continued = malloc(strlen(joined) + 2); TABLE_CHECK(continued);
        memcpy(continued, joined, insertion); continued[insertion] = 'Z'; strcpy(continued + insertion + 1, joined + insertion);
        table_allocator allocation = {-1, 0}; xui_document d = table_open(before, &allocation);
        xui_document_snapshot old, first, current, restored; xui_document_transaction t;
        uint64_t left, right, source; xui_doc_position_t tail, caret; int quality;
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
        join_positions(old, before, &left, &right, &tail);
        printf("Join continuation fixture=%u eol=%u bom=%u eof=%u compound=%u\n", fixture, eol, bom, eof, compound); fflush(stdout);
        TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnJoinBlocks(t, left, right, &caret) == XUI_OK);
        if (compound) { xui_doc_range_t selected = {caret, caret}; TABLE_CHECK(xuiDocumentTxnReplaceRange(t, &selected, "Z", 1, &caret) == XUI_OK); }
        TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &first) == XUI_OK);
        table_source(first, compound ? continued : joined); integration_equivalent(first, compound ? continued : joined);
        caret.iRevision = xuiDocumentGetRevision(d);
        if (!compound) {
            xui_doc_range_t selected = {caret, caret};
            TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnReplaceRange(t, &selected, "Z", 1, &caret) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
            xuiDocumentTxnRelease(t); caret.iRevision = xuiDocumentGetRevision(d);
        }
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
        table_source(current, continued); integration_equivalent(current, continued); boundary_caret(d, current, &caret);
        xui_doc_node_info_t typed = {0}; typed.iSize = sizeof(typed);
        TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &typed) == XUI_OK && typed.iKind == XUI_DOC_TEXT &&
            typed.tAttributes.iMarks == (fixture == 0 ? XUI_DOC_BOLD : 0));
        TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &source, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && source == insertion + 1);
        if (!compound) {
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, joined); table_equivalent(restored, first); xuiDocumentSnapshotRelease(restored);
        }
        TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, compound ? continued : joined); table_equivalent(restored, first); xuiDocumentSnapshotRelease(restored);
        if (!compound) TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
        TABLE_CHECK(!xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, continued); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
        table_source(old, before); table_source(first, compound ? continued : joined); table_source(current, continued);
        xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(first); xuiDocumentSnapshotRelease(current); xuiDocumentRelease(d);
        TABLE_CHECK(!allocation.live); free(before); free(joined); free(continued); total++;
    }
    TABLE_CHECK(total == 144);
    printf("Join continuation: %u same-draft/post-publication/source/reparse/exact-caret/shared-history/snapshot/zero-leak goldens passed\n", total);
    return 0;
}

#endif
