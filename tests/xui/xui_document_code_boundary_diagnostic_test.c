#define XUI_DOC_IMPLICIT_REFERENCE_SPLIT_HELPERS_ONLY
#include "xui_document_implicit_reference_split_diagnostic_test.c"
#undef XUI_DOC_IMPLICIT_REFERENCE_SPLIT_HELPERS_ONLY

static const struct reference_text_case {
    const char *name, *before, *after, *target, *replacement;
    uint64_t left, right;
} reference_text_cases[] = {
    {"shortcut-code-retick", "[``abcd``] &amp;\n\n[``abcd``]: <raw> 'Title'\n\nend &amp;\n",
     "[```aX``Yd```][``abcd``] &amp;\n\n[``abcd``]: <raw> 'Title'\n\nend &amp;\n", "abcd", "X``Y", 1, 3},
    {"collapsed-code-retick", "[``abcd``][] &amp;\n\n[``abcd``]: <raw> 'Title'\n\nend &amp;\n",
     "[```aX``Yd```][``abcd``] &amp;\n\n[``abcd``]: <raw> 'Title'\n\nend &amp;\n", "abcd", "X``Y", 1, 3},
    {"nested-code-retick", "- > + > __[``abcd``][]__ &amp;\n\n[``abcd``]: <raw> 'Title'\n\nend &amp;\n",
     "- > + > __[```aX``Yd```][``abcd``]__ &amp;\n\n[``abcd``]: <raw> 'Title'\n\nend &amp;\n", "abcd", "X``Y", 1, 3},
    {"code-boundary-spaces", "[`abcd`][] &amp;\n\n[`abcd`]: <raw> 'Title'\n\nend &amp;\n",
     "[`  xyz  `][`abcd`] &amp;\n\n[`abcd`]: <raw> 'Title'\n\nend &amp;\n", "abcd", " xyz ", 0, 4},
    {"full-reference-code-retick", "[``abcd``][Ref] &amp;\n\n[Ref]: <raw> 'Title'\n\nend &amp;\n",
     "[```aX``Yd```][Ref] &amp;\n\n[Ref]: <raw> 'Title'\n\nend &amp;\n", "abcd", "X``Y", 1, 3},
    {"direct-code-retick", "[``abcd``](<raw> 'Title') &amp;\n\n[unused]: /retain\n\nend &amp;\n",
     "[```aX``Yd```](<raw> 'Title') &amp;\n\n[unused]: /retain\n\nend &amp;\n", "abcd", "X``Y", 1, 3},
    {"plain-code-retick", "``abcd`` &amp;\n\n[unused]: /retain\n\nend &amp;\n",
     "```aX``Yd``` &amp;\n\n[unused]: /retain\n\nend &amp;\n", "abcd", "X``Y", 1, 3},
    {"plain-code-boundary-spaces", "`abcd` &amp;\n\n[unused]: /retain\n\nend &amp;\n",
     "`  xyz  ` &amp;\n\n[unused]: /retain\n\nend &amp;\n", "abcd", " xyz ", 0, 4},
    {"padded-code-to-all-spaces", "` abcd ` &amp;\n\n[unused]: /retain\n\nend &amp;\n",
     "`  ` &amp;\n\n[unused]: /retain\n\nend &amp;\n", "abcd", "  ", 0, 4},
    {"code-leading-tick", "`abcd` &amp;\n\n[unused]: /retain\n\nend &amp;\n",
     "`` `x `` &amp;\n\n[unused]: /retain\n\nend &amp;\n", "abcd", "`x", 0, 4},
    {"code-trailing-tick", "`abcd` &amp;\n\n[unused]: /retain\n\nend &amp;\n",
     "`` x` `` &amp;\n\n[unused]: /retain\n\nend &amp;\n", "abcd", "x`", 0, 4},
    {"code-noncolliding-tick-control", "``abcd`` &amp;\n\n[unused]: /retain\n\nend &amp;\n",
     "``aX```Yd`` &amp;\n\n[unused]: /retain\n\nend &amp;\n", "abcd", "X```Y", 1, 3},
    {"code-delete-tick-seam", "``a`b`c`` &amp;\n\n[unused]: /retain\n\nend &amp;\n",
     "```a``c``` &amp;\n\n[unused]: /retain\n\nend &amp;\n", "a`b`c", "", 2, 3}

};
static const char* code_expected_words[] = {"aX``Yd", "aX``Yd", "aX``Yd", " xyz ", "aX``Yd", "aX``Yd", "aX``Yd", " xyz ", "  ", "`x", "x`", "aX```Yd", "a``c"};

static const char* code_probe_words[] = {"aX", "aX", "aX", "xyz", "aX", "aX", "aX", "xyz", "  ", "`x", "x`", "aX", "a``"};

static void code_expected_word(xui_document_snapshot snapshot, uint64_t node, const char* expected)
{
    xui_doc_node_info_t selected = {0}, parent = {0}; uint64_t i, id, bytes, used = 0; char word[512];
    selected.iSize = parent.iSize = sizeof(selected);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(snapshot, node, &selected) == XUI_OK &&
        xuiDocumentSnapshotGetNode(snapshot, selected.iParentId, &parent) == XUI_OK);
    for (i = 0; i < parent.iChildCount; i++) {
        xui_doc_node_info_t part = {0}; part.iSize = sizeof(part);
        TABLE_CHECK(xuiDocumentSnapshotGetChild(snapshot, selected.iParentId, i, &id) == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, id, &part) == XUI_OK);
        if (part.iKind != XUI_DOC_TEXT || part.tAttributes.iMarks != selected.tAttributes.iMarks ||
            strcmp(part.sResource, selected.sResource) || strcmp(part.sTitle, selected.sTitle)) continue;
        TABLE_CHECK(used + part.iTextBytes < sizeof(word) &&
            xuiDocumentSnapshotCopyText(snapshot, id, word + used, sizeof(word) - used, &bytes) == XUI_OK);
        used += bytes;
    }
    word[used] = 0;
    printf("Code independent oracle word=[%s] expected=[%s]\n", word, expected);
    TABLE_CHECK(used == strlen(expected) && !memcmp(word, expected, (size_t)used));
}

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
            TABLE_CHECK(xuiDocumentSnapshotGetNode(old, text.iNodeId, &info) == XUI_OK && (fixture >= 6 || (info.tAttributes.iMarks & XUI_DOC_LINK)) &&
                info.iTextBytes == strlen(c->target) &&
                xuiDocumentSnapshotCopyText(old, text.iNodeId, source, sizeof(source), &bytes) == XUI_OK &&
                bytes == strlen(c->target) && !memcmp(source, c->target, (size_t)bytes));
            TABLE_CHECK(info.tAttributes.iMarks & XUI_DOC_CODE);
            {
                xui_doc_position_t ep; int eq;
                const char* probe = strstr(c->after, code_probe_words[fixture]);
                TABLE_CHECK(probe && xuiDocumentSourceToPosition(expected, (uint64_t)(probe-c->after)+1, &ep, &eq) == XUI_OK && ep.iKind == XUI_DOC_POSITION_TEXT);
                code_expected_word(expected, ep.iNodeId, code_expected_words[fixture]);
            }
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
