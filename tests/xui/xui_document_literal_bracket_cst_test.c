#define XUI_DOC_IMPLICIT_REFERENCE_SPLIT_HELPERS_ONLY
#include "xui_document_implicit_reference_split_diagnostic_test.c"
#undef XUI_DOC_IMPLICIT_REFERENCE_SPLIT_HELPERS_ONLY

static const struct reference_text_case {
    const char *name, *before, *after, *target, *replacement;
    uint64_t left, right;
} reference_text_cases[] = {
    {"shortcut-literal-brackets", "[abcd] &amp;\n\n[abcd]: <raw> 'Title'\n\nend &amp;\n",
     "[aX\\]Y\\[d][abcd] &amp;\n\n[abcd]: <raw> 'Title'\n\nend &amp;\n", "abcd", "X]Y[", 1, 3},
    {"nested-collapsed-literal-brackets", "- > + > __[abcd][]__ &amp;\n\n[abcd]: <raw> 'Title'\n\nend &amp;\n",
     "- > + > __[aX\\]Y\\[d][abcd]__ &amp;\n\n[abcd]: <raw> 'Title'\n\nend &amp;\n", "abcd", "X]Y[", 1, 3}
};

#ifndef XUI_DOC_REFERENCE_TEXT_HELPERS_ONLY
int main(void)
{
    unsigned fixture, api; int failures = 0;
    for (fixture = 0; fixture < sizeof(reference_text_cases)/sizeof(*reference_text_cases); fixture++)
        for (api = 0; api < 2; api++) {
            const struct reference_text_case* c = &reference_text_cases[fixture];
            table_allocator allocator = {-1, 0}; xui_document document = table_open(c->before, &allocator), oracle = table_open(c->after, NULL);
            xui_document_snapshot old, expected, current, restored; xui_document_transaction transaction;
            xui_doc_range_t range; xui_doc_position_t text, caret = {0}; xui_doc_node_info_t info = {0};
            const char* content = strstr(c->before, c->target); char source[4096]; uint64_t bytes; int quality, result, same;
            TABLE_CHECK(content && xuiDocumentClearHistory(document) == XUI_OK && xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
                xuiDocumentMarkSaved(document, old) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK &&
                xuiDocumentSourceToPosition(old, (uint64_t)(content - c->before) + 1, &text, &quality) == XUI_OK &&
                quality == XUI_DOC_MAP_EXACT && text.iKind == XUI_DOC_POSITION_TEXT);
            info.iSize = sizeof(info);
            TABLE_CHECK(xuiDocumentSnapshotGetNode(old, text.iNodeId, &info) == XUI_OK && (info.tAttributes.iMarks & XUI_DOC_LINK) &&
                info.iTextBytes == strlen(c->target) &&
                xuiDocumentSnapshotCopyText(old, text.iNodeId, source, sizeof(source), &bytes) == XUI_OK &&
                bytes == strlen(c->target) && !memcmp(source, c->target, (size_t)bytes));
            if (fixture == 3) TABLE_CHECK(info.tAttributes.iMarks & XUI_DOC_BOLD);
            range.tAnchor = range.tCaret = text; range.tAnchor.iOffset = c->left; range.tCaret.iOffset = c->right;
            TABLE_CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            result = api ? xuiDocumentTxnReplaceText(transaction, text.iNodeId, c->left, c->right, c->replacement, strlen(c->replacement)) :
                xuiDocumentTxnReplaceRange(transaction, &range, c->replacement, strlen(c->replacement), &caret);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, NULL);
            if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(transaction, NULL) == result);
            xuiDocumentTxnRelease(transaction);
            TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
            if (result != XUI_OK) {
                failures++; table_source(current, c->before); table_equivalent(current, old);
                TABLE_CHECK(!xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document));
                printf("Reference text fixture=%u api=%u name=%s rejected=%d atomic=1\n", fixture, api, c->name, result);
            } else {
                formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
                if (!api) boundary_caret(document, current, &caret);
                TABLE_CHECK(xuiDocumentSnapshotCopySource(current, source, sizeof(source), &bytes) == XUI_OK);
                integration_equivalent(current, source);
                same = bytes == strlen(c->after) && !memcmp(source, c->after, (size_t)bytes); failures += !same;
                printf("Reference text fixture=%u api=%u name=%s raw_equal=%d semantics=1 reparse=1\n", fixture, api, c->name, same);
                if (!same) printf("Reference text actual=[%s]\n", source);
                TABLE_CHECK(xuiDocumentUndo(document, NULL) == XUI_OK && !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document) &&
                    xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
                table_source(restored, c->before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
                TABLE_CHECK(xuiDocumentRedo(document, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
                table_source(restored, source); integration_equivalent(restored, source); xuiDocumentSnapshotRelease(restored);
            }
            table_source(old, c->before); xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old);
            xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle); xuiDocumentRelease(document); TABLE_CHECK(!allocator.live);
        }
    printf("Reference text failures=%d; all public API semantics/source/atomicity/history/cleanup checks completed\n", failures);
    return failures ? 1 : 0;
}

#endif
