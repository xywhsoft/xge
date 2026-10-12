#define XUI_DOC_INLINE_BOUNDARY_HELPERS_ONLY
#include "xui_document_inline_boundary_cst_test.c"
#undef XUI_DOC_INLINE_BOUNDARY_HELPERS_ONLY

static void formatted_split_same_attributes(const xui_doc_node_info_t* a, const xui_doc_node_info_t* b)
{
    TABLE_CHECK(a->iKind == b->iKind && !memcmp(&a->tAttributes, &b->tAttributes, sizeof(a->tAttributes)) &&
        !strcmp(a->sResource, b->sResource) && !strcmp(a->sInfo, b->sInfo) && !strcmp(a->sTitle, b->sTitle) &&
        !strcmp(a->sLinkTarget, b->sLinkTarget) && !strcmp(a->sLinkTitle, b->sLinkTitle));
}
static void formatted_split_same_semantics(xui_document_snapshot actual, uint64_t id,
    xui_document_snapshot full, uint64_t other)
{
    xui_doc_node_info_t a = {0}, b = {0}; uint64_t i = 0, j = 0, ao = 0, bo = 0, bytes;
    char text[512], match[512];
    a.iSize = b.iSize = sizeof(a);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(actual, id, &a) == XUI_OK && xuiDocumentSnapshotGetNode(full, other, &b) == XUI_OK);
    formatted_split_same_attributes(&a, &b);
    TABLE_CHECK(a.iTextBytes == b.iTextBytes && a.iTextBytes < sizeof(text) &&
        xuiDocumentSnapshotCopyText(actual, id, text, sizeof(text), &bytes) == XUI_OK &&
        xuiDocumentSnapshotCopyText(full, other, match, sizeof(match), &bytes) == XUI_OK &&
        !memcmp(text, match, (size_t)a.iTextBytes));
    while (i < a.iChildCount || j < b.iChildCount) {
        xui_doc_node_info_t left = {0}, right = {0}; uint64_t child = 0, twin = 0;
        left.iSize = right.iSize = sizeof(left);
        if (i < a.iChildCount) TABLE_CHECK(xuiDocumentSnapshotGetChild(actual, id, i, &child) == XUI_OK &&
            xuiDocumentSnapshotGetNode(actual, child, &left) == XUI_OK);
        if (j < b.iChildCount) TABLE_CHECK(xuiDocumentSnapshotGetChild(full, other, j, &twin) == XUI_OK &&
            xuiDocumentSnapshotGetNode(full, twin, &right) == XUI_OK);
        if (child && left.iKind == XUI_DOC_TEXT && !left.iTextBytes) { i++; continue; }
        if (twin && right.iKind == XUI_DOC_TEXT && !right.iTextBytes) { j++; continue; }
        TABLE_CHECK(child && twin);
        if (left.iKind == XUI_DOC_TEXT && right.iKind == XUI_DOC_TEXT) {
            uint64_t remaining = left.iTextBytes - ao, other_remaining = right.iTextBytes - bo;
            uint64_t count = remaining < other_remaining ? remaining : other_remaining;
            formatted_split_same_attributes(&left, &right);
            TABLE_CHECK(left.iTextBytes < sizeof(text) && right.iTextBytes < sizeof(match) &&
                xuiDocumentSnapshotCopyText(actual, child, text, sizeof(text), &bytes) == XUI_OK &&
                xuiDocumentSnapshotCopyText(full, twin, match, sizeof(match), &bytes) == XUI_OK &&
                !memcmp(text + ao, match + bo, (size_t)count));
            ao += count; bo += count;
            if (ao == left.iTextBytes) { i++; ao = 0; }
            if (bo == right.iTextBytes) { j++; bo = 0; }
        } else {
            TABLE_CHECK(!ao && !bo); formatted_split_same_semantics(actual, child, full, twin); i++; j++;
        }
    }
    TABLE_CHECK(!ao && !bo);
}

static const struct formatted_split_case {
    const char *name, *before, *after;
} formatted_split_cases[] = {
    {"root-strong-underscore",
     "__abcd__ &amp;\n\n[unused]: /raw 'Title'\n\nend &amp;\n",
     "__ab__\n\n__cd__ &amp;\n\n[unused]: /raw 'Title'\n\nend &amp;\n"},
    {"nested-strong-underscore",
     "- > + > __abcd__ &amp;\n  >   >\n  >   > [unused]: /raw 'Title'\n\nend &amp;\n",
     "- > + > __ab__\n  >   >\n  >   > __cd__ &amp;\n  >   >\n  >   > [unused]: /raw 'Title'\n\nend &amp;\n"},
    {"nested-emphasis-underscore",
     "- > + > _abcd_ &amp;\n  >   >\n  >   > [unused]: /raw 'Title'\n\nend &amp;\n",
     "- > + > _ab_\n  >   >\n  >   > _cd_ &amp;\n  >   >\n  >   > [unused]: /raw 'Title'\n\nend &amp;\n"},
    {"nested-code",
     "- > + > `abcd` &amp;\n  >   >\n  >   > [unused]: /raw 'Title'\n\nend &amp;\n",
     "- > + > `ab`\n  >   >\n  >   > `cd` &amp;\n  >   >\n  >   > [unused]: /raw 'Title'\n\nend &amp;\n"},
    {"nested-strike",
     "- > + > ~~abcd~~ &amp;\n  >   >\n  >   > [unused]: /raw 'Title'\n\nend &amp;\n",
     "- > + > ~~ab~~\n  >   >\n  >   > ~~cd~~ &amp;\n  >   >\n  >   > [unused]: /raw 'Title'\n\nend &amp;\n"},
    {"nested-link",
     "- > + > [abcd](<raw> 'Title') &amp;\n  >   >\n  >   > [unused]: /raw 'Title'\n\nend &amp;\n",
     "- > + > [ab](<raw> 'Title')\n  >   >\n  >   > [cd](<raw> 'Title') &amp;\n  >   >\n  >   > [unused]: /raw 'Title'\n\nend &amp;\n"}
    ,{"single-list-quote-strong-shared-golden",
     "+ > **abcd**\n  >\n  > [unused]: /raw 'Title'\n\nend &amp;\n",
     "+ > **ab**\n  >\n  > **cd**\n  >\n  > [unused]: /raw 'Title'\n\nend &amp;\n"}
};

#ifndef XUI_DOC_INLINE_SPLIT_HELPERS_ONLY
int main(void)
{
    unsigned sample;
    int mismatches = 0;
    for (sample = 0; sample < sizeof(formatted_split_cases)/sizeof(*formatted_split_cases); sample++) {
        const struct formatted_split_case *c = &formatted_split_cases[sample];
        boundary_case operation = {c->before, c->after, "\n", 2, 2, 2};
        table_allocator allocator = {-1, 0};
        xui_document document = table_open(c->before, &allocator);
        xui_document oracle = table_open(c->after, NULL);
        xui_document_snapshot old, actual, expected, restored;
        xui_doc_range_t range;
        xui_doc_position_t caret = {0};
        char raw[4096];
        uint64_t bytes, i;
        int raw_equal;
        TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK &&
            xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
            xuiDocumentMarkSaved(document, old) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        range = boundary_selection(document, old, c->before, &operation, 0);
        TABLE_CHECK(boundary_command(document, &range, &operation, &caret, NULL) == XUI_OK &&
            xuiDocumentAcquireSnapshot(document, &actual) == XUI_OK);
        formatted_split_same_semantics(actual, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
        boundary_caret(document, actual, &caret);
        TABLE_CHECK(xuiDocumentSnapshotCopySource(actual, raw, sizeof(raw), &bytes) == XUI_OK);
        integration_equivalent(actual, raw);
        raw_equal = bytes == strlen(c->after) && !memcmp(raw, c->after, (size_t)bytes);
        printf("FORMATTED SPLIT sample=%u name=%s independent_semantics=1 full_reparse=1 raw_equal=%d actual_hex=",
            sample, c->name, raw_equal);
        for (i = 0; i < bytes; i++) printf("%02x", (unsigned char)raw[i]);
        puts("");
        mismatches += !raw_equal;
        TABLE_CHECK(xuiDocumentUndo(document, NULL) == XUI_OK && !xuiDocumentCanUndo(document) &&
            !xuiDocumentIsDirty(document) && xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
        table_source(restored, c->before); table_equivalent(restored, old);
        xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(xuiDocumentRedo(document, NULL) == XUI_OK &&
            xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
        table_source(restored, raw); integration_equivalent(restored, raw);
        xuiDocumentSnapshotRelease(restored); table_source(old, c->before);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
        xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(old);
        xuiDocumentRelease(document); TABLE_CHECK(!allocator.live);
    }
    printf("Formatted split independent source mismatches=%d; all semantic/caret/history/cleanup checks completed\n", mismatches);
    return mismatches ? 1 : 0;
}

#endif
