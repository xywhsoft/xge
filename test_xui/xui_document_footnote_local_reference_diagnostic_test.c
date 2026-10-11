#define XUI_DOC_IMPLICIT_REFERENCE_SPLIT_HELPERS_ONLY
#include "xui_document_implicit_reference_split_diagnostic_test.c"
#undef XUI_DOC_IMPLICIT_REFERENCE_SPLIT_HELPERS_ONLY

static const struct local_reference_case {
    const char *name, *source, *target, *destination, *title;
    unsigned marks, definitions;
} local_reference_cases[] = {
    {"local-shortcut-code", "use[^n]\n\n[^n]:\n    [``abcd``] &amp;\n    \n    [``abcd``]: <raw> 'Title'\n    \n    end &amp;\n", "abcd", "raw", "Title", XUI_DOC_CODE | XUI_DOC_LINK, 2},
    {"local-full-reference", "use[^n]\n\n[^n]:\n    [inside][r]\n    \n    [r]: /local 'Title'\n", "inside", "/local", "Title", XUI_DOC_LINK, 2},
    {"hidden-local-global-use", "[outside][r]\n\n[^n]:\n    [r]: /hidden 'Title'\n", "outside", "/hidden", "Title", XUI_DOC_LINK, 2},
    {"local-before-root-duplicate", "[outside][R] use[^n]\n\n[^n]:\n    [inside][r]\n    \n    [r]: /first 'First'\n\n[R]: /second 'Second'\n", "outside", "/first", "First", XUI_DOC_LINK, 3},
    {"root-before-local-duplicate", "[r]: /first 'First'\n\nuse[^n]\n\n[^n]:\n    [inside][R]\n    \n    [R]: /second 'Second'\n", "inside", "/first", "First", XUI_DOC_LINK, 3},
    {"local-quote-definition", "use[^n]\n\n[^n]:\n    [inside][r]\n    \n    > [r]: /quoted 'Title'\n", "inside", "/quoted", "Title", XUI_DOC_LINK, 2},
    {"local-code-definition-control", "[outside][r] use[^n]\n\n[^n]:\n    ```\n    [r]: /fake\n    ```\n", "outside", "", "", 0, 1},
    {"local-html-definition-control", "[outside][r] use[^n]\n\n[^n]:\n    <div>\n    [r]: /fake\n    </div>\n", "outside", "", "", 0, 1},
    {"definition-on-first-footnote-line", "use[^n]\n\n[^n]: [r]: /firstline 'Title'\n    \n    [inside][r]\n", "inside", "/firstline", "Title", XUI_DOC_LINK, 2},
    {"multiline-local-label-title", "use[^n]\n\n[^n]:\n    [inside][r s]\n    \n    [r\n    s]: /multi 'Title\n    continued'\n", "inside", "/multi", "Title\ncontinued", XUI_DOC_LINK, 2},
    {"tab-local-entities", "use[^n]\n\n[^n]:\n\t[inside][r]\n\t\n\t[r]: <a&amp;b> '&lt;Title&gt;'\n", "inside", "a&b", "<Title>", XUI_DOC_LINK, 2},
    {"nested-footnote-declaration-control", "[outside][^r] use[^n]\n\n[^n]:\n    [^r]: /fake\n", "outside", "", "", 0, 1},
    {"duplicate-across-visible-hidden-footnotes", "[outside][R] use[^n]\n\n[^n]:\n    [r]: /first 'First'\n\n[^m]:\n    [R]: /second 'Second'\n", "outside", "/first", "First", XUI_DOC_LINK, 4}
};

#ifndef XUI_DOC_FOOTNOTE_LOCAL_REFERENCE_HELPERS_ONLY
int main(void)
{
    unsigned fixture;
    for (fixture = 0; fixture < sizeof(local_reference_cases) / sizeof(*local_reference_cases); fixture++) {
        const struct local_reference_case* c = local_reference_cases + fixture;
        table_allocator allocator = {-1, 0}; xui_doc_desc_t desc = {0};
        xui_document d; xui_document_snapshot old; xui_doc_node_info_t n = {0};
        xui_doc_position_t position; xui_doc_source_info_t info = {0};
        uint64_t bytes, index; int quality, result; char text[4096];
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = table_alloc; desc.onFree = table_free; desc.pAllocatorUser = &allocator;
        TABLE_CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        result = xuiDocumentLoadMarkdown(d, c->source, strlen(c->source));
        printf("Local reference fixture=%u name=%s load=%d\n", fixture, c->name, result); fflush(stdout);
        TABLE_CHECK(result == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK);
        table_source(old, c->source); integration_equivalent(old, c->source);
        TABLE_CHECK(xuiDocumentSourceToPosition(old, (uint64_t)(strstr(c->source, c->target) - c->source) + 1, &position, &quality) == XUI_OK && position.iKind == XUI_DOC_POSITION_TEXT);
        n.iSize = sizeof(n); TABLE_CHECK(xuiDocumentSnapshotGetNode(old, position.iNodeId, &n) == XUI_OK);
        printf("Local reference fixture=%u marks=%u expected=%u URI=[%s] title=[%s]\n", fixture, n.tAttributes.iMarks, c->marks, n.sResource, n.sTitle); fflush(stdout);
        TABLE_CHECK(n.tAttributes.iMarks == c->marks && !strcmp(n.sResource, c->destination) && !strcmp(n.sTitle, c->title));
        TABLE_CHECK(xuiDocumentSnapshotCopyText(old, position.iNodeId, text, sizeof(text), &bytes) == XUI_OK && bytes < sizeof(text)); text[bytes] = 0;
        TABLE_CHECK(strstr(text, c->target));
        info.iSize = sizeof(info); TABLE_CHECK(xuiDocumentSnapshotGetSourceInfo(old, &info) == XUI_OK && info.iReferenceDefinitionCount == c->definitions);
        for (index = 0; index < info.iReferenceDefinitionCount; index++) {
            xui_doc_reference_definition_t definition = {0}; definition.iSize = sizeof(definition);
            TABLE_CHECK(xuiDocumentSnapshotGetReferenceDefinition(old, index, &definition) == XUI_OK && definition.iSourceStart <= definition.iLabelStart && definition.iLabelEnd <= definition.iSourceEnd && definition.iSourceEnd <= strlen(c->source));
        }
        xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d); TABLE_CHECK(!allocator.live);
    }
    puts("Footnote local references: independent link/code/URI/title/count/physical-order/opaque-control/source/reparse/cleanup checks passed");
    return 0;
}
#endif
