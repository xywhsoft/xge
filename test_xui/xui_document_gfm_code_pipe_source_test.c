#define XUI_DOC_REFERENCE_TEXT_HELPERS_ONLY
#include "xui_document_reference_text_diagnostic_test.c"
#undef XUI_DOC_REFERENCE_TEXT_HELPERS_ONLY

static const struct pipe_source_case {
    const char *inline_text, *word;
    unsigned marks, pipes;
} pipe_source_cases[] = {
    {"`a\\|b`", "a|b", XUI_DOC_CODE, 1},
    {"`\\|`", "|", XUI_DOC_CODE, 1},
    {"``a\\|b\\\\q``", "a|b\\\\q", XUI_DOC_CODE, 1},
    {"`\\|x\\|`", "|x|", XUI_DOC_CODE, 2},
    {"`` \\| ``", "|", XUI_DOC_CODE, 1},
    {"[``a\\|b``](<raw> 'Title')", "a|b", XUI_DOC_CODE | XUI_DOC_LINK, 1},
    {"[``a\\|b``][keep]", "a|b", XUI_DOC_CODE | XUI_DOC_LINK, 1},
    {"**`a\\|b`**", "a|b", XUI_DOC_CODE | XUI_DOC_BOLD, 1}
};
static void pipe_source_scan(xui_document_snapshot snapshot, xui_document_source_index index,
    uint64_t id, unsigned marks, char* word, uint64_t* used, unsigned* pipes)
{
    xui_doc_node_info_t info = {0}; uint64_t i, bytes;
    info.iSize = sizeof(info);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &info) == XUI_OK);
    if (info.iKind == XUI_DOC_TEXT && info.tAttributes.iMarks == marks) {
        TABLE_CHECK(*used + info.iTextBytes < 256 &&
            xuiDocumentSnapshotCopyText(snapshot, id, word + *used, 256 - *used, &bytes) == XUI_OK);
        *used += bytes;
        for (i = 0; i < info.iSourceSegmentCount; i++) {
            xui_doc_source_segment_t segment = {0}; char raw[3] = {0};
            segment.iSize = sizeof(segment);
            TABLE_CHECK(xuiDocumentSnapshotGetSourceSegment(snapshot, id, i, &segment) == XUI_OK);
            if (segment.iSourceEnd - segment.iSourceStart != 2 ||
                segment.iTextEnd - segment.iTextStart != 1) continue;
            TABLE_CHECK(xuiDocumentSourceIndexReadSource(index, segment.iSourceStart, raw, 2) == XUI_OK);
            if (memcmp(raw, "\\|", 2)) continue;
            {
                xui_doc_position_t position = {0}, mapped; uint64_t source, token;
                xui_doc_source_token_t ownership = {0}; unsigned affinity; int quality;
                TABLE_CHECK(segment.iKind == XUI_DOC_SOURCE_NORMALIZED);
                TABLE_CHECK(xuiDocumentSourceToPosition(snapshot, segment.iSourceStart + 1, &position, &quality) == XUI_OK);
                position.iNodeId = id; position.iKind = XUI_DOC_POSITION_TEXT;
                for (affinity = XUI_DOC_BEFORE; affinity <= XUI_DOC_AFTER; affinity++) {
                    position.iAffinity = affinity; position.iOffset = segment.iTextStart;
                    TABLE_CHECK(xuiDocumentPositionToSource(snapshot, &position, &source, &quality) == XUI_OK &&
                        source == segment.iSourceStart && quality == XUI_DOC_MAP_EXACT);
                    position.iOffset = segment.iTextEnd;
                    TABLE_CHECK(xuiDocumentPositionToSource(snapshot, &position, &source, &quality) == XUI_OK &&
                        source == segment.iSourceEnd && quality == XUI_DOC_MAP_EXACT);
                    TABLE_CHECK(xuiDocumentSourceToPositionEx(snapshot, segment.iSourceStart + 1, affinity, &mapped, &quality) == XUI_OK &&
                        quality == XUI_DOC_MAP_COLLAPSED && mapped.iNodeId == id && mapped.iKind == XUI_DOC_POSITION_TEXT &&
                        mapped.iOffset == (affinity == XUI_DOC_AFTER ? segment.iTextEnd : segment.iTextStart));
                }
                ownership.iSize = sizeof(ownership);
                TABLE_CHECK(xuiDocumentSourceIndexFindToken(index, segment.iSourceStart, &token) == XUI_OK &&
                    xuiDocumentSourceIndexGetToken(index, token, &ownership) == XUI_OK &&
                    ownership.iOriginalSourceStart == segment.iSourceStart &&
                    ownership.iOriginalSourceEnd == segment.iSourceEnd && ownership.iSegmentKind == XUI_DOC_SOURCE_NORMALIZED);
                if (ownership.iNodeId == id) {
                    TABLE_CHECK(ownership.iTextStart == segment.iTextStart && ownership.iTextEnd == segment.iTextEnd);
                } else {
                    TABLE_CHECK(!ownership.iNodeId && ownership.iTextStart == UINT64_MAX && ownership.iTextEnd == UINT64_MAX);
                }
                {
                    xui_doc_source_index_info_t si = {0}; uint64_t binding; unsigned found = 0;
                    si.iSize = sizeof(si); TABLE_CHECK(xuiDocumentSourceIndexGetInfo(index, &si) == XUI_OK);
                    for (binding = 0; binding < si.iBindingCount; binding++) {
                        xui_doc_source_binding_t b = {0}; b.iSize = sizeof(b);
                        TABLE_CHECK(xuiDocumentSourceIndexGetBinding(index, binding, &b) == XUI_OK);
                        if (b.iOriginKind != XUI_DOC_SOURCE_BINDING_PROVENANCE || b.tRange.iNodeId != id ||
                            b.tRange.iOriginalSourceStart != segment.iSourceStart || b.tRange.iOriginalSourceEnd != segment.iSourceEnd) continue;
                        TABLE_CHECK(!b.iMetadata && b.tRange.iSegmentKind == XUI_DOC_SOURCE_NORMALIZED &&
                            b.tRange.iTextStart == segment.iTextStart && b.tRange.iTextEnd == segment.iTextEnd);
                        found++;
                    }
                    TABLE_CHECK(found == 1);
                }
                (*pipes)++;
            }
        }
    }
    for (i = 0; i < info.iChildCount; i++) {
        uint64_t child;
        TABLE_CHECK(xuiDocumentSnapshotGetChild(snapshot, id, i, &child) == XUI_OK);
        pipe_source_scan(snapshot, index, child, marks, word, used, pipes);
    }
}
#ifndef XUI_DOC_GFM_CODE_PIPE_HELPERS_ONLY
int main(void)
{
    unsigned dialect, fixture, context, eol, bom, final, checked = 0;
    const char* endings[] = {"\n", "\r\n", "\r"};
    for (dialect = XUI_MD_GFM; dialect <= XUI_MD_EXTENDED; dialect++)
        for (fixture = 0; fixture < sizeof(pipe_source_cases)/sizeof(*pipe_source_cases); fixture++)
            for (context = 0; context < (dialect == XUI_MD_EXTENDED ? 5u : 4u); context++)
                for (eol = 0; eol < 3; eol++) for (bom = 0; bom < 2; bom++) for (final = 0; final < 2; final++) {
                    const struct pipe_source_case* c = &pipe_source_cases[fixture];
                    char table[512], word[256]; uint64_t used = 0; unsigned pipes = 0;
                    char* source; table_allocator allocator = {-1, 0}; xui_doc_desc_t options = {0};
                    xui_document document; xui_document_snapshot snapshot; xui_document_source_index index;
                    snprintf(table, sizeof(table), "| H |\n| --- |\n| %s |", c->inline_text);
                    source = table_wrap(table, context, endings[eol], bom, final);
                    options.iSize = sizeof(options); options.iProfile = XUI_DOCUMENT_MARKDOWN; options.iMarkdownDialect = dialect;
                    options.onAlloc = table_alloc; options.onFree = table_free; options.pAllocatorUser = &allocator;
                    TABLE_CHECK(xuiDocumentCreate(&options, &document) == XUI_OK &&
                        xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK &&
                        xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                        xuiDocumentSnapshotCreateSourceIndex(snapshot, NULL, &index) == XUI_OK);
                    printf("GFM pipe source fixture=%u dialect=%u context=%u eol=%u bom=%u final=%u\n",
                        fixture, dialect, context, eol, bom, final); fflush(stdout);
                    pipe_source_scan(snapshot, index, XUI_DOCUMENT_ROOT, c->marks, word, &used, &pipes);
                    word[used] = 0;
                    TABLE_CHECK(used == strlen(c->word) && !memcmp(word, c->word, (size_t)used) && pipes == c->pipes);
                    table_source(snapshot, source); integration_equivalent(snapshot, source);
                    xuiDocumentSourceIndexRelease(index); xuiDocumentSnapshotRelease(snapshot);
                    xuiDocumentRelease(document); TABLE_CHECK(!allocator.live); free(source); checked++;
                }
    printf("GFM pipe source %u independent words/origin/bidirectional positions/index/full-reparse/cleanup goldens passed\n", checked);
    return 0;
}

#endif
