#define XUI_DOC_INLINE_SPLIT_HELPERS_ONLY
#include "xui_document_inline_split_diagnostic_test.c"
#undef XUI_DOC_INLINE_SPLIT_HELPERS_ONLY

static const struct implicit_case {
    const char *name, *before, *after, *target;
    uint64_t offset;
} implicit_cases[] = {
    {"root-shortcut", "[abcd] &amp;\n\n[abcd]: <raw> 'Title'\n\nend &amp;\n",
     "[ab][abcd]\n\n[cd][abcd] &amp;\n\n[abcd]: <raw> 'Title'\n\nend &amp;\n", "abcd", 2},
    {"root-collapsed", "[abcd][] &amp;\n\n[abcd]: <raw> 'Title'\n\nend &amp;\n",
     "[ab][abcd]\n\n[cd][abcd] &amp;\n\n[abcd]: <raw> 'Title'\n\nend &amp;\n", "abcd", 2},
    {"nested-shortcut", "- > + > __[abcd]__ &amp;\n\n[abcd]: <raw> 'Title'\n\nend &amp;\n",
     "- > + > __[ab][abcd]__\n  >   >\n  >   > __[cd][abcd]__ &amp;\n\n[abcd]: <raw> 'Title'\n\nend &amp;\n", "abcd", 2},
    {"nested-collapsed-formatted-label", "- > + > [__abcd__][] &amp;\n\n[__abcd__]: <raw> 'Title'\n\nend &amp;\n",
     "- > + > [__ab__][__abcd__]\n  >   >\n  >   > [__cd__][__abcd__] &amp;\n\n[__abcd__]: <raw> 'Title'\n\nend &amp;\n", "abcd", 2},
    {"casefold-duplicate-first-wins", "[AbCd][] &amp;\n\n[abcd]: <raw> 'Title'\n[ABCD]: <ignored> 'Unused'\n\nend &amp;\n",
     "[Ab][abcd]\n\n[Cd][abcd] &amp;\n\n[abcd]: <raw> 'Title'\n[ABCD]: <ignored> 'Unused'\n\nend &amp;\n", "AbCd", 2},
    {"multiline-display-label", "[ab\ncd][] &amp;\n\n[ab cd]: <raw> 'Title'\n\nend &amp;\n",
     "[a][ab cd]\n\n[b\ncd][ab cd] &amp;\n\n[ab cd]: <raw> 'Title'\n\nend &amp;\n", "ab\ncd", 1}
};

#ifndef XUI_DOC_IMPLICIT_REFERENCE_SPLIT_HELPERS_ONLY
int main(void)
{
    unsigned sample; int failures = 0;
    for (sample = 0; sample < sizeof(implicit_cases)/sizeof(*implicit_cases); sample++) {
        const struct implicit_case* c = &implicit_cases[sample];
        table_allocator allocator = {-1, 0};
        xui_document document = table_open(c->before, &allocator), oracle = table_open(c->after, NULL);
        xui_document_snapshot old, actual, expected, restored;
        xui_doc_position_t caret = {0}; xui_doc_range_t range;
        xui_doc_node_info_t node = {0}; int quality, equal;
        char source[4096]; uint64_t bytes, i;
        boundary_case operation = {c->before, c->after, "\n", 2, 0, 0};
        const char* content = strstr(c->before, c->target); TABLE_CHECK(content);
        TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK &&
            xuiDocumentAcquireSnapshot(document, &old) == XUI_OK && xuiDocumentMarkSaved(document, old) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        memset(&range, 0, sizeof(range)); range.tAnchor.iSize = sizeof(range.tAnchor);
        TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(content-c->before)+c->offset,
            &range.tAnchor, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && range.tAnchor.iKind == XUI_DOC_POSITION_TEXT);
        node.iSize = sizeof(node);
        TABLE_CHECK(xuiDocumentSnapshotGetNode(old, range.tAnchor.iNodeId, &node) == XUI_OK &&
            (node.tAttributes.iMarks & XUI_DOC_LINK));
        range.tCaret = range.tAnchor;
        TABLE_CHECK(boundary_command(document, &range, &operation, &caret, NULL) == XUI_OK &&
            xuiDocumentAcquireSnapshot(document, &actual) == XUI_OK);
        formatted_split_same_semantics(actual, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
        boundary_caret(document, actual, &caret);
        TABLE_CHECK(xuiDocumentSnapshotCopySource(actual, source, sizeof(source), &bytes) == XUI_OK);
        integration_equivalent(actual, source);
        equal = bytes == strlen(c->after) && !memcmp(source, c->after, (size_t)bytes);
        failures += !equal;
        printf("IMPLICIT REF sample=%u name=%s semantic=1 full_reparse=1 raw_equal=%d actual_hex=", sample, c->name, equal);
        for (i = 0; i < bytes; i++) printf("%02x", (unsigned char)source[i]);
        puts("");
        TABLE_CHECK(xuiDocumentUndo(document, NULL) == XUI_OK && !xuiDocumentIsDirty(document) &&
            !xuiDocumentCanUndo(document) && xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
        table_source(restored, c->before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(document, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
        table_source(restored, source); integration_equivalent(restored, source); xuiDocumentSnapshotRelease(restored);
        table_source(old, c->before); xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(old);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle); xuiDocumentRelease(document); TABLE_CHECK(!allocator.live);
    }
    printf("Implicit reference source mismatches=%d; all independent semantics/caret/reparse/history/cleanup checks completed\n", failures);
    return failures ? 1 : 0;
}

#endif
