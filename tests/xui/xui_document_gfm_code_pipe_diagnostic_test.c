#define XUI_DOC_REFERENCE_TEXT_HELPERS_ONLY
#include "xui_document_reference_text_diagnostic_test.c"
#undef XUI_DOC_REFERENCE_TEXT_HELPERS_ONLY
static const struct pipe_case { const char *source, *probe, *word; unsigned marks; } pipe_cases[] = {
    {"`a\\|b`\n", "a\\|b", "a\\|b", XUI_DOC_CODE},
    {"| H |\n| --- |\n| a\\|b |\n", "a\\|b", "a|b", 0},
    {"| H |\n| --- |\n| `a\\|b` |\n", "a\\|b", "a|b", XUI_DOC_CODE},
    {"| H |\n| --- |\n| [``a\\|b``](<raw> 'Title') |\n", "a\\|b", "a|b", XUI_DOC_CODE | XUI_DOC_LINK},
    {"| H |\n| --- |\n| b `\\|` az |\n", "\\|", "|", XUI_DOC_CODE}
};
int main(void)
{
    unsigned dialect, fixture; int failures = 0;
    for (dialect = XUI_MD_GFM; dialect <= XUI_MD_EXTENDED; dialect++)
        for (fixture = 0; fixture < sizeof(pipe_cases)/sizeof(*pipe_cases); fixture++) {
            const struct pipe_case* c = &pipe_cases[fixture]; table_allocator allocator = {-1,0};
            xui_document document; xui_document_snapshot snapshot; xui_doc_desc_t options = {0};
            xui_doc_position_t position; xui_doc_node_info_t selected = {0}, parent = {0};
            uint64_t i, id, bytes, used = 0; int quality, same; char word[256];
            options.iSize = sizeof(options); options.iProfile = XUI_DOCUMENT_MARKDOWN; options.iMarkdownDialect = dialect;
            options.onAlloc = table_alloc; options.onFree = table_free; options.pAllocatorUser = &allocator;
            TABLE_CHECK(xuiDocumentCreate(&options, &document) == XUI_OK &&
                xuiDocumentLoadMarkdown(document, c->source, strlen(c->source)) == XUI_OK &&
                xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSourceToPosition(snapshot, (uint64_t)(strstr(c->source,c->probe)-c->source)+1, &position, &quality) == XUI_OK &&
                position.iKind == XUI_DOC_POSITION_TEXT);
            selected.iSize = parent.iSize = sizeof(selected);
            TABLE_CHECK(xuiDocumentSnapshotGetNode(snapshot, position.iNodeId, &selected) == XUI_OK &&
                selected.tAttributes.iMarks == c->marks && xuiDocumentSnapshotGetNode(snapshot, selected.iParentId, &parent) == XUI_OK);
            for (i=0;i<parent.iChildCount;i++) {
                xui_doc_node_info_t part = {0};part.iSize = sizeof(part);
                TABLE_CHECK(xuiDocumentSnapshotGetChild(snapshot, selected.iParentId, i, &id) == XUI_OK &&
                    xuiDocumentSnapshotGetNode(snapshot,id,&part) == XUI_OK);
                if (part.iKind != XUI_DOC_TEXT || part.tAttributes.iMarks != c->marks ||
                    strcmp(part.sResource, selected.sResource) || strcmp(part.sTitle, selected.sTitle)) continue;
                TABLE_CHECK(used+part.iTextBytes < sizeof(word) && xuiDocumentSnapshotCopyText(snapshot,id,word+used,sizeof(word)-used,&bytes) == XUI_OK);
                used += bytes;
            }
            word[used] = 0;same=used == strlen(c->word) && !memcmp(word,c->word,(size_t)used);failures += !same;
            printf("GFM code pipe dialect=%u fixture=%u normative_equal=%d actual=[%s] expected=[%s]\n",dialect,fixture,same,word,c->word);
            table_source(snapshot,c->source);integration_equivalent(snapshot,c->source);
            xuiDocumentSnapshotRelease(snapshot);xuiDocumentRelease(document);TABLE_CHECK(!allocator.live);
        }
    printf("GFM code pipe normative mismatches=%d; all source/full-reparse/cleanup checks completed\n",failures);
    return failures ? 1 : 0;
}
