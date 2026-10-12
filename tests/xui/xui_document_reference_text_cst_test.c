#define XUI_DOC_REFERENCE_TEXT_HELPERS_ONLY
#include "xui_document_reference_text_diagnostic_test.c"
#undef XUI_DOC_REFERENCE_TEXT_HELPERS_ONLY

/* The edited bytes and selected definition spelling are concrete independent
 * goldens. Framing changes only containers, physical endings and BOM/EOF. */
static const struct reference_text_format {
    const char *before, *after, *definitions, *target, *text, *replacement;
    uint64_t left, right;
} reference_text_formats[] = {
    {"[abcd] &amp;\n", "[abEcd][abcd] &amp;\n", "[abcd]: <raw> 'Title'\n", "abcd", "abcd", "E", 2, 2},
    {"[abcd][] &amp;\n", "[ad][abcd] &amp;\n", "[abcd]: <raw> 'Title'\n", "abcd", "abcd", "", 1, 3},
    {"__[abcd]__ &amp;\n", "__[aXYd][abcd]__ &amp;\n", "[abcd]: <raw> 'Title'\n", "abcd", "abcd", "XY", 1, 3},
    {"[**abcd**][] &amp;\n", "[**aXYd**][**abcd**] &amp;\n", "[**abcd**]: <raw> 'Title'\n", "abcd", "abcd", "XY", 1, 3},
    {"[**ab**cd][] &amp;\n", "[cd][**ab**cd] &amp;\n", "[**ab**cd]: <raw> 'Title'\n", "ab", "ab", "", 0, 2},
    {"[abcd][] &amp;\n", "&#32;&amp;\n", "[abcd]: <raw> 'Title'\n", "abcd", "abcd", "", 0, 4},
    {"[ab\ncd][] &amp;\n", "[ab\ncEd][ab cd] &amp;\n", "[ab cd]: <raw> 'Title'\n", "cd", "cd", "E", 1, 1},
    {"[abcd][Ref] &amp;\n", "[abEcd][Ref] &amp;\n", "[Ref]: <raw> 'Title'\n", "abcd", "abcd", "E", 2, 2},
    {"[abcd](<raw> 'Title') &amp;\n", "[aXYd](<raw> 'Title') &amp;\n", "[unused]: /retain\n", "abcd", "abcd", "XY", 1, 3},
    {"[ABCD][] &amp;\n", "[ABECD][abcd] &amp;\n", "[abcd]: <raw> 'Title'\n[ABCD]: <ignored> 'Unused'\n", "ABCD", "ABCD", "E", 2, 2},
    {"[abcd][] &amp;\n", "[abEcd][abcd] &amp;\n", "[ \tabcd \t]: <raw> 'Title'\n", "abcd", "abcd", "E", 2, 2},
    {"[ab cd][] &amp;\n", "[abE cd][ab cd] &amp;\n", "[ab\ncd]: <raw> 'Title'\n", "ab cd", "ab cd", "E", 2, 2},
    {"[``abcd``][] &amp;\n", "[``aX*Yd``][``abcd``] &amp;\n", "[``abcd``]: <raw> 'Title'\n", "abcd", "abcd", "X*Y", 1, 3},
    {"[a&amp;bc][] &amp;\n", "[a&amp;bEc][a&amp;bc] &amp;\n", "[a&amp;bc]: <raw> 'Title'\n", "bc", "bc", "E", 1, 1},
    {"[stra\xc3\x9f" "e][] &amp;\n", "[sXYa\xc3\x9f" "e][STRASSE] &amp;\n", "[STRASSE]: <raw> 'Title'\n", "stra\xc3\x9f" "e", "stra\xc3\x9f" "e", "XY", 1, 3},
    {"__[abcd][]__ &amp;\n", "&#32;&amp;\n", "[\xe2\x80\x83" "abcd\xc2\xa0]: <raw> 'Title'\n", "abcd", "abcd", "", 0, 4}
};

static char* reference_text_definitions(char* source, const char* definitions,
    const char* ending, int final)
{
    char* expanded = delete_ending(definitions, ending, 0);
    size_t bytes = strlen(source), d = strlen(expanded), e = strlen(ending);
    char* result = malloc(bytes + 2 * e + d + 1);
    TABLE_CHECK(result && d >= e);
    if (!final) { d -= e; expanded[d] = 0; }
    memcpy(result, source, bytes); memcpy(result + bytes, ending, e);
    memcpy(result + bytes + e, ending, e); memcpy(result + bytes + 2 * e, expanded, d + 1);
    free(expanded); free(source); return result;
}

static xui_doc_range_t reference_text_selection(xui_document_snapshot old,
    const char* before, const struct reference_text_format* f, int reverse)
{
    xui_doc_range_t range = {0}; xui_doc_node_info_t info = {0};
    char text[256]; uint64_t bytes; int quality;
    const char* target = strstr(before, f->target); TABLE_CHECK(target);
    TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(target - before) + 1,
        &range.tAnchor, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
        range.tAnchor.iKind == XUI_DOC_POSITION_TEXT);
    info.iSize = sizeof(info);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(old, range.tAnchor.iNodeId, &info) == XUI_OK &&
        (info.tAttributes.iMarks & XUI_DOC_LINK) && info.iTextBytes == strlen(f->text) &&
        xuiDocumentSnapshotCopyText(old, range.tAnchor.iNodeId, text, sizeof(text), &bytes) == XUI_OK &&
        bytes == strlen(f->text) && !memcmp(text, f->text, (size_t)bytes));
    range.tCaret = range.tAnchor; range.tAnchor.iOffset = f->left; range.tCaret.iOffset = f->right;
    if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
    return range;
}

static int reference_text_command(xui_document document, const xui_doc_range_t* range,
    const struct reference_text_format* f, unsigned api, xui_doc_position_t* caret,
    xui_document_change_set* change)
{
    xui_document_transaction transaction; int result = xuiDocumentBeginTransaction(document, NULL, &transaction);
    if (result != XUI_OK) return result;
    result = api ? xuiDocumentTxnReplaceText(transaction, range->tAnchor.iNodeId,
        f->left, f->right, f->replacement, strlen(f->replacement)) :
        xuiDocumentTxnReplaceRange(transaction, range, f->replacement, strlen(f->replacement), caret);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, change);
    if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(transaction, NULL) == result);
    xuiDocumentTxnRelease(transaction); return result;
}

static void reference_text_golden(const char* before, const char* after,
    const struct reference_text_format* f, unsigned api, int reverse)
{
    table_allocator allocator = {-1, 0}; xui_document document = table_open(before, &allocator), oracle = table_open(after, NULL);
    xui_document_snapshot old, current, expected, restored; xui_document_change_set change;
    xui_doc_range_t range; xui_doc_position_t caret = {0}, tail, mapped; uint64_t offset; int quality;
    TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK && xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
        xuiDocumentMarkSaved(document, old) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    range = reference_text_selection(old, before, f, reverse);
    TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(boundary_tail(before) - before) + 1,
        &tail, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
    TABLE_CHECK(reference_text_command(document, &range, f, api, &caret, &change) == XUI_OK &&
        xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
    table_source(current, after); integration_equivalent(current, after);
    formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
    if (!api) boundary_caret(document, current, &caret);
    TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
        mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset &&
        xuiDocumentPositionToSource(current, &mapped, &offset, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
        offset == (uint64_t)(boundary_tail(after) - after) + 1);
    TABLE_CHECK(xuiDocumentUndo(document, NULL) == XUI_OK && !xuiDocumentCanUndo(document) && !xuiDocumentIsDirty(document) &&
        xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
    table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
    TABLE_CHECK(xuiDocumentRedo(document, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
    table_source(restored, after); integration_equivalent(restored, after); xuiDocumentSnapshotRelease(restored);
    table_source(old, before); table_source(current, after); xuiDocumentChangeSetRelease(change);
    xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(expected);
    xuiDocumentRelease(oracle); xuiDocumentRelease(document); TABLE_CHECK(!allocator.live);
}

static char* reference_text_shape(const char* body, unsigned shape)
{
    size_t bytes = strlen(body); char* result = malloc(bytes + 128); TABLE_CHECK(result && bytes && body[bytes - 1] == '\n');
    if (!shape) snprintf(result, bytes + 128, "# %.*s\n", (int)(bytes - 1), body);
    else if (shape == 1) snprintf(result, bytes + 128, "| %.*s | H |\n| --- | --- |\n| one | two |\n", (int)(bytes - 1), body);
    else snprintf(result, bytes + 128, "| H | J |\n| :--: | ---: |\n| %.*s | retain &amp; |\n", (int)(bytes - 1), body);
    return result;
}

static void reference_text_faults(void)
{
    unsigned api, mode;
    for (api = 0; api < 2; api++) for (mode = 0; mode < 2; mode++) {
        const struct reference_text_format* f = &reference_text_formats[mode ? 5 : 3];
        char* before = reference_text_definitions(table_wrap(f->before, 3, "\r\n", 1, 1), f->definitions, "\r\n", 1);
        char* after = reference_text_definitions(table_wrap(f->after, 3, "\r\n", 1, 1), f->definitions, "\r\n", 1); long point;
        for (point = 0; point < 4096; point++) {
            table_allocator allocator = {-1, 0}; xui_document document = table_open(before, &allocator);
            xui_document_snapshot old, current, restored; xui_doc_range_t range;
            xui_doc_position_t caret = {0}; uint64_t revision; int result;
            TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK && xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
                xuiDocumentMarkSaved(document, old) == XUI_OK);
            range = reference_text_selection(old, before, f, !api); revision = xuiDocumentGetRevision(document);
            allocator.remaining = point; result = reference_text_command(document, &range, f, api, &caret, NULL); allocator.remaining = -1;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(document) == revision &&
                    !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(reference_text_command(document, &range, f, api, &caret, NULL) == XUI_OK);
                table_source(current, before); xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
            }
            TABLE_CHECK(xuiDocumentGetRevision(document) == revision + 1 && xuiDocumentIsDirty(document) &&
                xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
            table_source(current, after); integration_equivalent(current, after);
            if (!api) boundary_caret(document, current, &caret);
            TABLE_CHECK(xuiDocumentUndo(document, NULL) == XUI_OK && !xuiDocumentCanUndo(document) && !xuiDocumentIsDirty(document) &&
                xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(document, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            table_source(restored, after); integration_equivalent(restored, after); xuiDocumentSnapshotRelease(restored);
            table_source(old, before); table_source(current, after); xuiDocumentSnapshotRelease(current);
            xuiDocumentSnapshotRelease(old); xuiDocumentRelease(document); TABLE_CHECK(!allocator.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 4096);
        printf("Reference text API %u mode %u: %ld OOM points atomic/poisoned commit/retry/one Undo/zero leaks\n", api, mode, point);
        free(before); free(after);
    }
}

int main(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned fixture, context, ending, bom, final, api, reverse, shape, total = 0;
    for (fixture = 0; fixture < sizeof(reference_text_formats) / sizeof(*reference_text_formats); fixture++)
        for (context = 0; context < 5; context++) for (ending = 0; ending < 3; ending++)
            for (bom = 0; bom < 2; bom++) for (final = 0; final < 2; final++) for (api = 0; api < 2; api++)
                for (reverse = 0; reverse < (api ? 1u : 2u); reverse++) {
                    const struct reference_text_format* f = &reference_text_formats[fixture];
                    char* before = reference_text_definitions(table_wrap(f->before, context, endings[ending], (int)bom, (int)final), f->definitions, endings[ending], (int)final);
                    char* after = reference_text_definitions(table_wrap(f->after, context, endings[ending], (int)bom, (int)final), f->definitions, endings[ending], (int)final);
                    printf("Reference text matrix fixture=%u context=%u ending=%u BOM=%u EOF=%u api=%u reverse=%u\n", fixture, context, ending, bom, final, api, reverse); fflush(stdout);
                    reference_text_golden(before, after, f, api, (int)reverse); free(before); free(after); total++;
                }
    for (fixture = 0; fixture < sizeof(reference_text_formats) / sizeof(*reference_text_formats); fixture++) {
        if (fixture == 6) continue; /* Physical multiline labels cannot belong to a GFM table row. */
        for (shape = 0; shape < 3; shape++) for (ending = 0; ending < 3; ending++) for (bom = 0; bom < 2; bom++)
            for (final = 0; final < 2; final++) for (api = 0; api < 2; api++) for (reverse = 0; reverse < (api ? 1u : 2u); reverse++) {
                const struct reference_text_format* f = &reference_text_formats[fixture];
                char *left = reference_text_shape(f->before, shape), *right = reference_text_shape(f->after, shape);
                char* before = reference_text_definitions(table_wrap(left, 0, endings[ending], (int)bom, (int)final), f->definitions, endings[ending], (int)final);
                char* after = reference_text_definitions(table_wrap(right, 0, endings[ending], (int)bom, (int)final), f->definitions, endings[ending], (int)final);
                printf("Reference text shape fixture=%u shape=%u ending=%u BOM=%u EOF=%u api=%u reverse=%u\n", fixture, shape, ending, bom, final, api, reverse); fflush(stdout);
                reference_text_golden(before, after, f, api, (int)reverse); free(left); free(right); free(before); free(after); total++;
            }
    }
    printf("Reference text: %u independent raw/semantics/source-index/stable-tail/caret/one-Undo goldens passed\n", total);
    reference_text_faults(); return 0;
}
