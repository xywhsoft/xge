#define XUI_DOC_REFERENCE_TEXT_HELPERS_ONLY
#include "xui_document_footnote_local_code_cst_test.c"
#undef XUI_DOC_REFERENCE_TEXT_HELPERS_ONLY

int main(void)
{
    unsigned selected[] = {0, 3, 8, 12}, choice, api;
    for (choice = 0; choice < sizeof(selected)/sizeof(*selected); choice++) for (api = 0; api < 2; api++) {
        unsigned fixture = selected[choice], failures = 0; long budget;
        struct reference_text_case local=reference_text_cases[fixture];
        char* before=table_wrap(local.before,4,"\r\n",1,1);
        char* after=table_wrap(local.after,4,"\r\n",1,1);
        const struct reference_text_case* c=&local;local.before=before;local.after=after;
        for (budget = 0; budget < 10000; budget++) {
            table_allocator allocator = {-1, 0}; xui_document d = table_open(c->before, &allocator);
            xui_document_snapshot old, current, restored; xui_document_transaction t; xui_doc_position_t position, caret;
            xui_doc_range_t range = {0}; uint64_t revision; int quality, result;
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
                xuiDocumentMarkSaved(d, old) == XUI_OK &&
                xuiDocumentSourceToPosition(old, (uint64_t)(strstr(c->before,c->target)-c->before)+1, &position, &quality) == XUI_OK &&
                quality == XUI_DOC_MAP_EXACT && position.iKind == XUI_DOC_POSITION_TEXT &&
                xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
            revision = xuiDocumentGetRevision(d); range.tAnchor = range.tCaret = position;
            range.tAnchor.iOffset = c->left; range.tCaret.iOffset = c->right;
            allocator.remaining = budget;
            result = api ? xuiDocumentTxnReplaceText(t, position.iNodeId, c->left, c->right, c->replacement, strlen(c->replacement)) :
                xuiDocumentTxnReplaceRange(t, &range, c->replacement, strlen(c->replacement), &caret);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            allocator.remaining = -1;
            TABLE_CHECK(result == XUI_OK || result == XUI_ERROR_OUT_OF_MEMORY);
            if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
            xuiDocumentTxnRelease(t);
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                failures++; TABLE_CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
                table_source(current, c->before); table_equivalent(current, old); xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
                result = api ? xuiDocumentTxnReplaceText(t, position.iNodeId, c->left, c->right, c->replacement, strlen(c->replacement)) :
                    xuiDocumentTxnReplaceRange(t, &range, c->replacement, strlen(c->replacement), &caret);
                TABLE_CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
                result = XUI_ERROR_OUT_OF_MEMORY;
            }
            table_source(current, c->after); integration_equivalent(current, c->after);
            {
                const char* probe = strstr(c->after, code_probe_words[fixture]);
                TABLE_CHECK(probe && xuiDocumentSourceToPosition(current, (uint64_t)(probe-c->after)+1, &position, &quality) == XUI_OK && position.iKind == XUI_DOC_POSITION_TEXT);
                code_expected_word(current, position.iNodeId, code_expected_words[fixture]);
            }
            if (!api) boundary_caret(d, current, &caret);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, c->before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, c->after); integration_equivalent(restored, c->after); xuiDocumentSnapshotRelease(restored);
            table_source(old, c->before); xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d);
            TABLE_CHECK(!allocator.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(budget < 10000 && failures);
        free(before);free(after);
        printf("Code boundary fixture=%u api=%u OOM points=%u atomic/poisoned commit/retry/one Undo/zero leaks passed\n", fixture, api, failures);
    }
    return 0;
}
