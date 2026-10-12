#define XUI_DOC_INLINE_BOUNDARY_HELPERS_ONLY
#include "xui_document_inline_boundary_cst_test.c"
#undef XUI_DOC_INLINE_BOUNDARY_HELPERS_ONLY
static const boundary_case prefix_matrix_cases[] = {
    {"- > + > abcd\n  >   >\n  >   > [unused]: /raw 'Title'\n\nend &amp;\n",
     "- > + > ab\n  >   >\n  >   > cd\n  >   >\n  >   > [unused]: /raw 'Title'\n\nend &amp;\n", "\n", 2, 2, 2},
    {"- > 7) [x] abcd\n  >    \n  >    [unused]: /raw 'Title'\n\nend &amp;\n",
     "- > 7) [x] ab\n  >\n  >    cd\n  >    \n  >    [unused]: /raw 'Title'\n\nend &amp;\n", "\n", 2, 2, 2},
    {"- + abcd\n    \n    [unused]: /raw 'Title'\n\nend &amp;\n",
     "- + ab\n\n    cd\n    \n    [unused]: /raw 'Title'\n\nend &amp;\n", "\n", 2, 2, 2},
    {"12) > 7. > abcd\n    >    >\n    >    > [unused]: /raw 'Title'\n\nend &amp;\n",
     "12) > 7. > ab\n    >    >\n    >    > cd\n    >    >\n    >    > [unused]: /raw 'Title'\n\nend &amp;\n", "\n", 2, 2, 2},
    {"-\t>\t+\t>\tabcd\n \t>\t \t>\n \t>\t \t>\t[unused]: /raw 'Title'\n\nend &amp;\n",
     "-\t>\t+\t>\tab\n \t>\t \t>\n \t>\t \t>\tcd\n \t>\t \t>\n \t>\t \t>\t[unused]: /raw 'Title'\n\nend &amp;\n", "\n", 2, 2, 2},
    {"> + > 12) > abcd\n>   >     >\n>   >     > [unused]: /raw 'Title'\n\nend &amp;\n",
     "> + > 12) > ab\n>   >     >\n>   >     > cd\n>   >     >\n>   >     > [unused]: /raw 'Title'\n\nend &amp;\n", "\n", 2, 2, 2},
    {"- > + > 7) > abcd\n  >   >    >\n  >   >    > [unused]: /raw 'Title'\n\nend &amp;\n",
     "- > + > 7) > ab\n  >   >    >\n  >   >    > cd\n  >   >    >\n  >   >    > [unused]: /raw 'Title'\n\nend &amp;\n", "\n", 2, 2, 2}
};
static void prefix_faults(void)
{
    unsigned mode;
    for (mode = 0; mode < 2; mode++) {
        boundary_case selected = prefix_matrix_cases[mode];
        const boundary_case* c = &selected;
        selected.replacement = "\r\n";
        char* before = delete_ending(c->before, "\r\n", 1),
            *after = delete_ending(c->after, "\r\n", 1); long point;
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
        TABLE_CHECK(point < 4096); printf("Nested prefix mode %u: %ld OOM points atomic/poisoned commit/same-document retry/Undo/Redo/zero leaks\n", mode, point); free(before); free(after);
    }
}

int main(void)
{
    static const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned sample, ending, bom, reverse, total = 0;
    for (sample = 0; sample < sizeof(prefix_matrix_cases) / sizeof(*prefix_matrix_cases); sample++)
        for (ending = 0; ending < 3; ending++) for (bom = 0; bom < 2; bom++) for (reverse = 0; reverse < 2; reverse++) {
            boundary_case c = prefix_matrix_cases[sample];
            char* before = delete_ending(c.before, endings[ending], (int)bom);
            char* after = delete_ending(c.after, endings[ending], (int)bom);
            c.replacement = endings[ending];
            boundary_golden(before, after, &c, (int)reverse);
            free(before); free(after); total++;
        }
    printf("Nested parser-owned prefixes: %u independent raw/source-index/stable-tail/caret/one-Undo goldens passed\n", total);
    prefix_faults();
    return 0;
}
