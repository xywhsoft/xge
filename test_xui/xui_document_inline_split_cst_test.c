#define XUI_DOC_INLINE_SPLIT_HELPERS_ONLY
#include "xui_document_inline_split_diagnostic_test.c"
#undef XUI_DOC_INLINE_SPLIT_HELPERS_ONLY

/* Concrete source goldens, independent of serializer output. Context and
 * line-ending expansion changes only framing, never the edited body. */
static const struct split_format {
    const char *before, *split, *replace;
} split_formats[] = {
    {"__abcd__ &amp;\n", "__ab__\n\n__cd__ &amp;\n", "__aX__\n\n__Yd__ &amp;\n"},
    {"_abcd_ &amp;\n", "_ab_\n\n_cd_ &amp;\n", "_aX_\n\n_Yd_ &amp;\n"},
    {"***abcd*** &amp;\n", "***ab***\n\n***cd*** &amp;\n", "***aX***\n\n***Yd*** &amp;\n"},
    {"~~abcd~~ &amp;\n", "~~ab~~\n\n~~cd~~ &amp;\n", "~~aX~~\n\n~~Yd~~ &amp;\n"},
    {"``abcd`` &amp;\n", "``ab``\n\n``cd`` &amp;\n", "``aX``\n\n``Yd`` &amp;\n"},
    {"[abcd](<raw> 'Title') &amp;\n", "[ab](<raw> 'Title')\n\n[cd](<raw> 'Title') &amp;\n",
     "[aX](<raw> 'Title')\n\n[Yd](<raw> 'Title') &amp;\n"},
    {"__*abcd*__ &amp;\n", "__*ab*__\n\n__*cd*__ &amp;\n", "__*aX*__\n\n__*Yd*__ &amp;\n"},
    {"[__abcd__](<raw> 'Title') &amp;\n", "[__ab__](<raw> 'Title')\n\n[__cd__](<raw> 'Title') &amp;\n",
     "[__aX__](<raw> 'Title')\n\n[__Yd__](<raw> 'Title') &amp;\n"},
    {"[abcd][Ref] &amp;\n", "[ab][Ref]\n\n[cd][Ref] &amp;\n",
     "[aX][Ref]\n\n[Yd][Ref] &amp;\n"}
};

/* Resolve the full reference in an explicit global scope for every container.
 * A local definition inside a footnote is a separate parser capability case. */
static char* split_global_reference(char* source, const char* ending, int final)
{
    size_t bytes = strlen(source), e = strlen(ending);
    char* extended = malloc(bytes + 3 * e + 64);
    TABLE_CHECK(extended);
    memcpy(extended, source, bytes);
    snprintf(extended + bytes, 3 * e + 64, "%s%s[Ref]: <raw> 'Title'%s", ending, ending, final ? ending : "");
    free(source); return extended;
}

/* Inserted separators keep quote markers with no trailing horizontal space.
 * Only this new line is normalized; all pre-existing blank lines stay raw. */
static void split_inserted_separator(char* source, const char* ending, int replacement)
{
    char* content = strstr(source, replacement ? "aX" : "ab");
    char *first, *blank, *end, *trim;
    TABLE_CHECK(content);
    first = strstr(content, ending); TABLE_CHECK(first);
    blank = first + strlen(ending); end = strstr(blank, ending); TABLE_CHECK(end);
    for (trim = blank; trim < end; trim++)
        TABLE_CHECK(*trim == '>' || *trim == ' ' || *trim == '\t');
    trim = end;
    while (trim > blank && (trim[-1] == ' ' || trim[-1] == '\t')) trim--;
    memmove(trim, end, strlen(end) + 1);
}

static void split_matrix_faults(void)
{
    unsigned mode;
    for (mode = 0; mode < 2; mode++) {
        const struct formatted_split_case* fixture = &formatted_split_cases[mode ? 5 : 1];
        char *before = delete_ending(fixture->before, "\r\n", 1),
             *after = delete_ending(fixture->after, "\r\n", 1);
        boundary_case operation = {before, after, "\r\n", 2, 2, 2};
        long point;
        for (point = 0; point < 4096; point++) {
            table_allocator allocator = {-1, 0};
            xui_document document = table_open(before, &allocator);
            xui_document_snapshot old, current, restored;
            xui_doc_range_t range;
            xui_doc_position_t caret = {0};
            uint64_t revision;
            int result;
            TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK &&
                xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
                xuiDocumentMarkSaved(document, old) == XUI_OK);
            range = boundary_selection(document, old, before, &operation, (int)mode);
            revision = xuiDocumentGetRevision(document);
            allocator.remaining = point;
            result = boundary_command(document, &range, &operation, &caret, NULL);
            allocator.remaining = -1;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == XUI_ERROR_OUT_OF_MEMORY &&
                    xuiDocumentGetRevision(document) == revision &&
                    !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(boundary_command(document, &range, &operation, &caret, NULL) == XUI_OK);
                table_source(current, before); xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
            }
            TABLE_CHECK(xuiDocumentGetRevision(document) == revision + 1 &&
                xuiDocumentIsDirty(document) && xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
            table_source(current, after); integration_equivalent(current, after);
            boundary_caret(document, current, &caret);
            TABLE_CHECK(xuiDocumentUndo(document, NULL) == XUI_OK && !xuiDocumentIsDirty(document) &&
                !xuiDocumentCanUndo(document) && xuiDocumentCanRedo(document) &&
                xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(document, NULL) == XUI_OK &&
                xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            table_source(restored, after); integration_equivalent(restored, after); xuiDocumentSnapshotRelease(restored);
            table_source(current, after); table_source(old, before);
            xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(document);
            TABLE_CHECK(!allocator.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 4096);
        printf("Formatted split mode %u: %ld OOM points atomic/poisoned commit/same-document retry/one Undo/zero leaks\n", mode, point);
        free(before); free(after);
    }
}

static int direct_split_command(xui_document document, const xui_doc_position_t* at,
    xui_doc_position_t* caret, xui_document_change_set* change)
{
    xui_document_transaction transaction;
    int result = xuiDocumentBeginTransaction(document, NULL, &transaction);
    if (result != XUI_OK) return result;
    result = xuiDocumentTxnSplitBlock(transaction, at, caret);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, change);
    else TABLE_CHECK(xuiDocumentTxnCommit(transaction, NULL) == result);
    xuiDocumentTxnRelease(transaction);
    return result;
}

static void direct_split_golden(const char* before, const char* after, const boundary_case* c, int reverse)
{
    xui_document d = table_open(before, NULL); xui_document_snapshot old, current, restored;
    xui_document_change_set change; xui_doc_range_t range; xui_doc_position_t caret = {0}, tail, mapped;
    uint64_t source; int quality;
    TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
    range = boundary_selection(d, old, before, c, reverse);
    TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(boundary_tail(before)-before)+1, &tail, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
    TABLE_CHECK(direct_split_command(d, &range.tCaret, &caret, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
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
int main(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned fixture, operation, context, ending, bom, final, reverse, total = 0;
    for (fixture = 0; fixture < sizeof(split_formats)/sizeof(*split_formats); fixture++)
        for (operation = 0; operation < 3; operation++) for (context = 0; context < 5; context++)
            for (ending = 0; ending < 3; ending++) for (bom = 0; bom < 2; bom++)
                for (final = 0; final < 2; final++) for (reverse = 0; reverse < 2; reverse++) {
                    boundary_case c = {NULL, NULL, "\n", 2, 2, 2};
                    char replacement[8];
                    char* before = table_wrap(split_formats[fixture].before, context, endings[ending], (int)bom, (int)final);
                    char* after = table_wrap(operation == 1 ? split_formats[fixture].replace : split_formats[fixture].split,
                        context, endings[ending], (int)bom, (int)final);
                    if (fixture == 8) {
                        before = split_global_reference(before, endings[ending], (int)final);
                        after = split_global_reference(after, endings[ending], (int)final);
                        xui_document check = table_open(before, NULL);
                        xui_document_snapshot snapshot; xui_doc_position_t position;
                        xui_doc_node_info_t node = {0}; int quality;
                        node.iSize = sizeof(node);
                        TABLE_CHECK(xuiDocumentAcquireSnapshot(check, &snapshot) == XUI_OK &&
                            xuiDocumentSourceToPosition(snapshot, (uint64_t)(strstr(before, "abcd") - before) + 1,
                                &position, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
                            xuiDocumentSnapshotGetNode(snapshot, position.iNodeId, &node) == XUI_OK &&
                            node.iKind == XUI_DOC_TEXT && (node.tAttributes.iMarks & XUI_DOC_LINK));
                        xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(check);
                    }
                    split_inserted_separator(after, endings[ending], operation == 1);
                    if (operation == 1) {
                        snprintf(replacement, sizeof(replacement), "X%sY", endings[ending]);
                        c.replacement = replacement; c.left = 1; c.right = 3;
                    } else c.replacement = endings[ending];
                    printf("Formatted matrix fixture=%u operation=%u context=%u ending=%u BOM=%u EOF=%u reverse=%u\n",
                        fixture, operation, context, ending, bom, final, reverse); fflush(stdout);
                    if (operation == 2) direct_split_golden(before, after, &c, (int)reverse);
                    else boundary_golden(before, after, &c, (int)reverse);
                    free(before); free(after); total++;
                }
    for (fixture = 0; fixture < sizeof(formatted_split_cases)/sizeof(*formatted_split_cases); fixture++)
        for (ending = 0; ending < 3; ending++) for (bom = 0; bom < 2; bom++) for (reverse = 0; reverse < 2; reverse++) {
            const struct formatted_split_case* f = &formatted_split_cases[fixture];
            char* before = delete_ending(f->before, endings[ending], (int)bom);
            char* after = delete_ending(f->after, endings[ending], (int)bom);
            boundary_case c = {before, after, endings[ending], 2, 2, 2};
            boundary_golden(before, after, &c, (int)reverse);
            free(before); free(after); total++;
        }
    printf("Formatted split: %u independent raw/source-index/stable-tail/caret/one-Undo goldens passed\n", total);
    split_matrix_faults();
    return 0;
}
