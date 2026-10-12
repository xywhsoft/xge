#define XUI_DOC_INLINE_RANGE_HELPERS_ONLY
#include "xui_document_inline_range_cst_test.c"
#undef XUI_DOC_INLINE_RANGE_HELPERS_ONLY

typedef struct boundary_case {
    const char *before, *after, *replacement;
    unsigned endpoint; /* 0: TEXT/GAP; 1: GAP/TEXT; 2: TEXT/TEXT. */
    uint64_t left, right;
} boundary_case;
static const boundary_case boundary_cases[] = {
    {"abc end &amp; tail\n", "ab&amp; tail\n", "", 0, 2, 0},
    {"abc end &amp; tail\n", "bc end &amp; tail\n", "", 1, 0, 1},
    {"**abc end** &amp; tail\n", "**ab** &amp; tail\n", "", 0, 2, 0},
    {"**abc end** &amp; tail\n", "**bc end** &amp; tail\n", "", 1, 0, 1},
    {"`abc end` &amp; tail\n", "`ab**` &amp; tail\n", "**", 0, 2, 0},
    {"abc end &amp; tail\n", "&#32;end &amp; tail\n", "", 1, 0, 3},
    {"**abc** end &amp; tail\n", "&#32;end &amp; tail\n", "", 0, 0, 0},
    {"`abc end` &amp; tail\n", "`ab` &amp; tail\n", "", 0, 2, 0},
    {"abc end &amp; tail\n", "a\\* end &amp; tail\n", "*", 2, 1, 3},
    {"`abc end` &amp; tail\n", "`a* end` &amp; tail\n", "*", 2, 1, 3}
};
static const boundary_case boundary_nested = {
    "- outer\n  - [unused]: /retain \"title\"\n\n    abcd\n\nend &amp;\n",
    "- outer\n  - [unused]: /retain \"title\"\n\n    ab\n\n    cd\n\nend &amp;\n",
    "\n", 2, 2, 2
};
static const boundary_case boundary_newline_extra[] = {
    {"> [unused]: /retain \"title\"\n>\n> abcd\n\nend &amp;\n", "> [unused]: /retain \"title\"\n>\n> ab\n>\n> cd\n\nend &amp;\n", "\n", 2, 2, 2},
    {"> abcd\n>\n> [unused]: /retain \"title\"\n\nend &amp;\n", "> ab\n>\n> cd\n>\n> [unused]: /retain \"title\"\n\nend &amp;\n", "\n", 2, 2, 2},
    {"> abcd\n>\n> [unused]: /retain \"title\"\n>\n> tail\n\nend &amp;\n", "> ab\n>\n> cd\n>\n> [unused]: /retain \"title\"\n>\n> tail\n\nend &amp;\n", "\n", 2, 2, 2},
    {"- [unused]: /retain \"title\"\n\n  abcd\n\nend &amp;\n", "- [unused]: /retain \"title\"\n\n  ab\n\n  cd\n\nend &amp;\n", "\n", 2, 2, 2},
    {"1. [unused]: /retain \"title\"\n\n   abcd\n\nend &amp;\n", "1. [unused]: /retain \"title\"\n\n   ab\n\n   cd\n\nend &amp;\n", "\n", 2, 2, 2},
    {"- [unused]: /retain \"title\"\n\n  abcd\n- tail\n\nend &amp;\n", "- [unused]: /retain \"title\"\n\n  ab\n\n  cd\n- tail\n\nend &amp;\n", "\n", 2, 2, 2},
    {"- first\n- [unused]: /retain \"title\"\n\n  abcd\n\nend &amp;\n", "- first\n- [unused]: /retain \"title\"\n\n  ab\n\n  cd\n\nend &amp;\n", "\n", 2, 2, 2},
    {"- abcd\n\n  [ref]: /target\n\nend &amp;\n", "- ab\n\n  cd\n\n  [ref]: /target\n\nend &amp;\n", "\n", 2, 2, 2},
    {"+ > abcd\n  >\n  > [unused]: /raw 'Title'\n\nend &amp;\n", "+ > ab\n  >\n  > cd\n  >\n  > [unused]: /raw 'Title'\n\nend &amp;\n", "\n", 2, 2, 2},
    {"12) > > abcd\n    > >\n    > > [unused]: <raw> \"Title\"\n\nend &amp;\n", "12) > > ab\n    > >\n    > > cd\n    > >\n    > > [unused]: <raw> \"Title\"\n\nend &amp;\n", "\n", 2, 2, 2},
    {"> + > abcd\n>   >\n>   > [unused]: /raw 'Title'\n\nend &amp;\n", "> + > ab\n>   >\n>   > cd\n>   >\n>   > [unused]: /raw 'Title'\n\nend &amp;\n", "\n", 2, 2, 2},
    {"before &amp; untouched\n\n-\t>\tabcd\n \t>\t\n \t>\t[unused]: /raw 'Title'\n\nafter &amp; untouched\n\nend &amp;\n", "before &amp; untouched\n\n-\t>\tab\n    >\n    >\tcd\n \t>\t\n \t>\t[unused]: /raw 'Title'\n\nafter &amp; untouched\n\nend &amp;\n", "\n", 2, 2, 2}
};
static xui_doc_range_t boundary_selection(xui_document d, xui_document_snapshot s,
    const char* before, const boundary_case* c, int reverse)
{
    xui_doc_position_t text; xui_doc_range_t r; int quality;
    const char* content = strstr(before, "abc");
    TABLE_CHECK(content && xuiDocumentSourceToPosition(s, (uint64_t)(content-before)+1,
        &text, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && text.iKind == XUI_DOC_POSITION_TEXT);
    r = gap_selection(d, s, text.iNodeId, text.iNodeId, 0, NULL);
    if (c->endpoint != 1) { r.tAnchor = text; r.tAnchor.iOffset = c->left; r.tAnchor.iAffinity = XUI_DOC_BEFORE; }
    if (c->endpoint != 0) { r.tCaret = text; r.tCaret.iOffset = c->right; r.tCaret.iAffinity = XUI_DOC_AFTER; }
    if (reverse) { xui_doc_position_t swap = r.tAnchor; r.tAnchor = r.tCaret; r.tCaret = swap; }
    return r;
}
static int boundary_command(xui_document d, const xui_doc_range_t* range,
    const boundary_case* c, xui_doc_position_t* caret, xui_document_change_set* change)
{
    xui_document_transaction t; int result = xuiDocumentBeginTransaction(d, NULL, &t);
    if (result != XUI_OK) return result;
    result = xuiDocumentTxnReplaceRange(t, range, c->replacement, strlen(c->replacement), caret);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, change);
    if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
    xuiDocumentTxnRelease(t); return result;
}
static void boundary_caret(xui_document d, xui_document_snapshot s, xui_doc_position_t* caret)
{
    xui_doc_node_info_t n = {0}; n.iSize = sizeof(n);
    caret->iRevision = xuiDocumentGetRevision(d);
    TABLE_CHECK(caret->iDocumentId == xuiDocumentGetIdentity(d) &&
        xuiDocumentSnapshotGetNode(s, caret->iNodeId, &n) == XUI_OK);
    TABLE_CHECK((caret->iKind == XUI_DOC_POSITION_TEXT && caret->iOffset <= n.iTextBytes) ||
        (caret->iKind == XUI_DOC_POSITION_GAP && caret->iOffset <= n.iChildCount));
}
static const char* boundary_tail(const char* source)
{
    const char *p = source, *last = NULL;
    while ((p = strstr(p, "end &amp;")) != NULL) { last = p; p++; }
    TABLE_CHECK(last); return last;
}
static void boundary_golden(const char* before, const char* after, const boundary_case* c, int reverse)
{
    xui_document d = table_open(before, NULL); xui_document_snapshot old, current, restored;
    xui_document_change_set change; xui_doc_range_t range; xui_doc_position_t caret = {0}, tail, mapped;
    uint64_t source; int quality;
    TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
    range = boundary_selection(d, old, before, c, reverse);
    TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(boundary_tail(before)-before)+1, &tail, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
    TABLE_CHECK(boundary_command(d, &range, c, &caret, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
    table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
    TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset);
    TABLE_CHECK(xuiDocumentPositionToSource(current, &mapped, &source, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && source == (uint64_t)(boundary_tail(after)-after)+1);
    TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
    table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
    TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
    table_source(restored, after); integration_equivalent(restored, after); xuiDocumentSnapshotRelease(restored);
    table_source(old, before); table_source(current, after); xuiDocumentChangeSetRelease(change);
    xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d);
}
static void boundary_faults(void)
{
    const unsigned fixtures[] = {0, 4, 5, 6}, contexts[] = {1, 3, 4, 3}; unsigned mode;
    for (mode = 0; mode < 7; mode++) {
        const boundary_case* c = mode < 4 ? &boundary_cases[fixtures[mode]] : mode == 4 ? &boundary_nested : &boundary_newline_extra[mode == 5 ? 1 : 7];
        char *before = mode >= 4 ? delete_ending(c->before, "\r\n", 1) : table_wrap(c->before, contexts[mode], "\r\n", 1, 0),
            *after = mode >= 4 ? delete_ending(c->after, "\r\n", 1) : table_wrap(c->after, contexts[mode], "\r\n", 1, 0); long point;
        for (point = 0; point < 4096; point++) {
            table_allocator f = {-1, 0}; xui_document d = table_open(before, &f);
            xui_document_snapshot old, current, restored; xui_doc_range_t range; xui_doc_position_t caret = {0}; uint64_t revision; int result;
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
            range = boundary_selection(d, old, before, c, (int)(mode%2)); revision = xuiDocumentGetRevision(d);
            f.remaining = point; result = boundary_command(d, &range, c, &caret, NULL); f.remaining = -1;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(boundary_command(d, &range, c, &caret, NULL) == XUI_OK); table_source(current, before);
                xuiDocumentSnapshotRelease(current); TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(xuiDocumentGetRevision(d) == revision+1 && xuiDocumentIsDirty(d) && xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
            table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, after); integration_equivalent(restored, after); xuiDocumentSnapshotRelease(restored);
            table_source(current, after); table_source(old, before); xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d); TABLE_CHECK(!f.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 4096); printf("Inline boundary mode %u: %ld OOM points atomic/poisoned commit/same-document retry/Undo/Redo/zero leaks\n", mode, point); free(before); free(after);
    }
}
static void boundary_nested_matrix(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"}; unsigned f, e, bom, reverse, total = 0;
    for (f = 0; f < 1 + sizeof(boundary_newline_extra)/sizeof(*boundary_newline_extra); f++)
        for (e = 0; e < 3; e++) for (bom = 0; bom < 2; bom++) for (reverse = 0; reverse < 2; reverse++) {
        boundary_case c = f ? boundary_newline_extra[f-1] : boundary_nested; char *before = delete_ending(c.before, endings[e], (int)bom),
            *after = delete_ending(c.after, endings[e], (int)bom);
        c.replacement = endings[e]; boundary_golden(before, after, &c, (int)reverse); free(before); free(after); total++;
    }
    printf("Inline boundary: %u nested newline minimal-source/retained-definition/full-reparse/stable-tail/Undo goldens passed\n", total);
}
static void boundary_table_newline(void)
{
    const char* rows[] = {"| H | B |\n| --- | --- |\n| abc end | beta |\n",
        "| H | B |\n| --- | --- |\n| **abc end** | beta |\n",
        "| H | B |\n| --- | --- |\n| `abc end` | beta |\n"};
    const char* endings[] = {"\n", "\r\n", "\r"}; unsigned fixture, context, ending, reverse, total = 0;
    for (fixture = 0; fixture < 3; fixture++) for (context = 0; context < 5; context++)
        for (ending = 0; ending < 3; ending++) for (reverse = 0; reverse < 2; reverse++) {
            char* before = table_wrap(rows[fixture], context, "\r\n", 1, 0);
            xui_document d = table_open(before, NULL); xui_document_snapshot old, current;
            xui_doc_range_t range; xui_doc_position_t caret = {0}; uint64_t revision; int result;
            boundary_case c = {NULL, NULL, endings[ending], 2, 2, 2};
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
            range = boundary_selection(d, old, before, &c, reverse); revision = xuiDocumentGetRevision(d);
            result = boundary_command(d, &range, &c, &caret, NULL);
            TABLE_CHECK(result == XUI_DOC_ERROR_UNREPRESENTABLE && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK); table_source(current, before); table_equivalent(current, old);
            table_source(old, before); xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d); free(before); total++;
        }
    printf("Inline boundary: %u table newline profile rejections preserve source/revision/savepoint/history\n", total);
}
#ifndef XUI_DOC_INLINE_BOUNDARY_HELPERS_ONLY
int main(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"}; unsigned f, context, e, bom, final, reverse, total = 0;
    for (f = 0; f < sizeof(boundary_cases)/sizeof(*boundary_cases); f++)
        for (context = 0; context < 5; context++) for (e = 0; e < 3; e++)
            for (bom = 0; bom < 2; bom++) for (final = 0; final < 2; final++) for (reverse = 0; reverse < 2; reverse++) {
                const boundary_case* c = &boundary_cases[f]; char *before = table_wrap(c->before, context, endings[e], bom, final),
                    *after = table_wrap(c->after, context, endings[e], bom, final);
                fprintf(stderr, "Boundary fixture=%u context=%u ending=%u BOM=%u EOF=%u reverse=%u\n", f, context, e, bom, final, reverse);
                boundary_golden(before, after, c, reverse); free(before); free(after); total++;
            }
    boundary_nested_matrix(); boundary_table_newline(); boundary_faults(); printf("Inline boundary: %u raw goldens/full reparse/byte roles/stable exact tail/draft caret/one Undo passed\n", total); return 0;
}

#endif
