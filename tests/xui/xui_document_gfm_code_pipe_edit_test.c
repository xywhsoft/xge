#define XUI_DOC_REFERENCE_TEXT_HELPERS_ONLY
#include "xui_document_reference_text_diagnostic_test.c"
#undef XUI_DOC_REFERENCE_TEXT_HELPERS_ONLY

static const struct pipe_edit_case { const char* before; const char* after; } pipe_edit_cases[] = {
    {"| H |\n| --- |\n| `a\\|b` |\n", "| H |\n| --- |\n| `aWb` |\n"},
    {"| H |\n| --- |\n| [``a\\|b``](<raw> 'Title') |\n", "| H |\n| --- |\n| [``aWb``](<raw> 'Title') |\n"},
    {"| H |\n| --- |\n| [``a\\|b``][Ref] |\n\n[Ref]: <raw> 'Title'\n", "| H |\n| --- |\n| [``aWb``][Ref] |\n\n[Ref]: <raw> 'Title'\n"}
};
int main(void)
{
    unsigned fixture, api, context, eol, bom, final, reverse, checked = 0; int failures = 0;
    const char* endings[] = {"\n", "\r\n", "\r"};
    for (fixture = 0; fixture < sizeof(pipe_edit_cases)/sizeof(*pipe_edit_cases); fixture++) for (api = 0; api < 2; api++) for (context = 0; context < 5; context++)
        for (eol = 0; eol < 3; eol++) for (bom = 0; bom < 2; bom++) for (final = 0; final < 2; final++) for (reverse = 0; reverse < 2; reverse++) {
        char* before = table_wrap(pipe_edit_cases[fixture].before, context, endings[eol], bom, final);
        char* after = table_wrap(pipe_edit_cases[fixture].after, context, endings[eol], bom, final);
        struct pipe_edit_case local = {before, after}; const struct pipe_edit_case* c = &local; table_allocator allocator = {-1, 0};
        xui_document document = table_open(c->before, &allocator), oracle = table_open(c->after, NULL);
        xui_document_snapshot old, expected, current, restored; xui_document_transaction transaction;
        xui_doc_range_t range = {0}; xui_doc_position_t position, caret; int quality, result; char text[8]; uint64_t bytes;
        uint64_t source = (uint64_t)(strstr(c->before, "\\|") - c->before);
        TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK && xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
            xuiDocumentMarkSaved(document, old) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK &&
            xuiDocumentSourceToPositionEx(old, source + 1, XUI_DOC_BEFORE, &position, &quality) == XUI_OK &&
            quality == XUI_DOC_MAP_COLLAPSED && position.iKind == XUI_DOC_POSITION_TEXT &&
            xuiDocumentSnapshotCopyText(old, position.iNodeId, text, sizeof(text), &bytes) == XUI_OK && bytes == 1 && text[0] == '|');
        range.tAnchor = range.tCaret = position; range.tAnchor.iOffset = 0; range.tCaret.iOffset = 1;
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        TABLE_CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        result = api ? xuiDocumentTxnReplaceText(transaction, position.iNodeId, 0, 1, "W", 1) :
            xuiDocumentTxnReplaceRange(transaction, &range, "W", 1, &caret);
        if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, NULL);
        if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(transaction, NULL) == result);
        xuiDocumentTxnRelease(transaction);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
        printf("GFM pipe edit fixture=%u api=%u result=%d\n", fixture, api, result); fflush(stdout);
        if (result != XUI_OK) { failures++; table_source(current, c->before); table_equivalent(current, old); }
        else {
            table_source(current, c->after); formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
            integration_equivalent(current, c->after); if (!api) boundary_caret(document, current, &caret);
            {
                xui_doc_position_t mapped; uint64_t origin; int mapping; char part[256];
                uint64_t at = (uint64_t)(strstr(c->after, "W") - c->after);
                TABLE_CHECK(xuiDocumentSourceToPositionEx(current, at, XUI_DOC_AFTER, &mapped, &mapping) == XUI_OK &&
                    mapping == XUI_DOC_MAP_EXACT && mapped.iKind == XUI_DOC_POSITION_TEXT &&
                    xuiDocumentSnapshotCopyText(current, mapped.iNodeId, part, sizeof(part), &bytes) == XUI_OK &&
                    mapped.iOffset < bytes && part[mapped.iOffset] == 'W' &&
                    xuiDocumentPositionToSource(current, &mapped, &origin, &mapping) == XUI_OK && mapping == XUI_DOC_MAP_EXACT && origin == at);
                if (!api) TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &origin, &mapping) == XUI_OK &&
                    mapping == XUI_DOC_MAP_EXACT && origin == at + 1);
                else {
                    TABLE_CHECK(xuiDocumentSourceToPositionEx(current, at + 1, XUI_DOC_BEFORE, &caret, &mapping) == XUI_OK && mapping == XUI_DOC_MAP_EXACT);
                }
                {
                    char continued[4096]; xui_document_snapshot next; xui_doc_range_t insertion = {0}; xui_doc_position_t next_caret;
                    snprintf(continued, sizeof(continued), "%.*sZ%s", (int)(at + 1), c->after, c->after + at + 1);
                    insertion.tAnchor = insertion.tCaret = caret;
                    TABLE_CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK &&
                        xuiDocumentTxnReplaceRange(transaction, &insertion, "Z", 1, &next_caret) == XUI_OK && xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
                    xuiDocumentTxnRelease(transaction);
                    TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &next) == XUI_OK);
                    table_source(next, continued); integration_equivalent(next, continued); boundary_caret(document, next, &next_caret);
                    TABLE_CHECK(xuiDocumentPositionToSource(next, &next_caret, &origin, &mapping) == XUI_OK && mapping == XUI_DOC_MAP_EXACT && origin == at + 2);
                    xuiDocumentSnapshotRelease(next);
                    TABLE_CHECK(xuiDocumentUndo(document, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(document, &next) == XUI_OK);
                    table_source(next, c->after); integration_equivalent(next, c->after); xuiDocumentSnapshotRelease(next);
                }
            }
            TABLE_CHECK(xuiDocumentUndo(document, NULL) == XUI_OK && !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document) &&
                xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            table_source(restored, c->before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(document, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            table_source(restored, c->after); integration_equivalent(restored, c->after); xuiDocumentSnapshotRelease(restored);
        }
        table_source(old, c->before); xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle); xuiDocumentRelease(document); TABLE_CHECK(!allocator.live); free(before); free(after); checked++;
    }
    printf("GFM pipe caret matrix checked=%u failures=%d\n", checked, failures); TABLE_CHECK(checked == 720); return failures ? 1 : 0;
}
