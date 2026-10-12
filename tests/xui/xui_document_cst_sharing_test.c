#define main uninvoked_core_main
#include "xui_document_test.c"
#undef main

static uint64_t exclusive_syntax_blobs(doc_state* before, doc_state* after)
{
    memory_oracle graph = {0}; uint64_t bytes = 0; size_t i;
    memory_oracle_visit(&graph, before->reference_syntax, 2);
    memory_oracle_visit(&graph, after->reference_syntax, 1);
    for (i = 0; i < graph.count; i++) if (graph.entries[i].mask == 2) {
        doc_allocation* a = (doc_allocation*)graph.entries[i].pointer - 1;
        if (a->value.kind == DOC_MEMORY_BLOB) bytes += a->value.bytes;
    }
    free(graph.entries); return bytes;
}
static void exact_syntax(doc_state* a, doc_state* b)
{
    uint64_t i, j, count = doc_seq_size(a->reference_syntax);
    CHECK(count == doc_seq_size(b->reference_syntax));
    for (i = 0; i < count; i++) {
        doc_source_cst_header left, right; uint64_t id = doc_seq_get_id(a->reference_syntax, i);
        CHECK(id == doc_seq_get_id(b->reference_syntax, i));
        CHECK(doc_source_cst_info(a, id - 1, &left) == XUI_OK && doc_source_cst_info(b, id - 1, &right) == XUI_OK);
        CHECK(!memcmp(&left, &right, sizeof(left)));
        for (j = 0; j < left.count; j++) {
            doc_source_cst_record x, y;
            CHECK(doc_source_cst_read(a, id - 1, j, &x) == XUI_OK && doc_source_cst_read(b, id - 1, j, &y) == XUI_OK);
            CHECK(!memcmp(&x, &y, sizeof(x)));
        }
    }
}
int main(void)
{
    static const unsigned sizes[] = {64, 256, 1024}; unsigned s, mode;
    const char* extra = "[same]: /shared 'Title'\n\n[^same]: **body**\n\n";
    for (s = 0; s < sizeof(sizes) / sizeof(*sizes); s++) for (mode = 0; mode < 2; mode++) {
        size_t capacity = sizes[s] * 80 + 512, a, z, n; unsigned i;
        char* source = malloc(capacity), *edited = malloc(capacity);
        fail_allocator allocator = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d, full;
        xui_document_snapshot before, after, expected; xui_document_source_index index;
        xui_doc_txn_desc_t td = {0}; xui_doc_source_patch_t patches[3] = {{0}};
        xui_document_prepare prepare = NULL; xui_document_transaction t = NULL;
        CHECK(source && edited); strcpy(source, "begin\n\n[^start]: first\n\n");
        for (i = 0; i < sizes[s]; i++) strcat(source, extra);
        strcat(source, "[^end]: last\n\n"); n = strlen(source);
        strcpy(edited, extra); strcat(edited, source);
        a = (size_t)(strstr(source, "first") - source); z = (size_t)(strstr(source, "last") - source);
        memcpy(edited + strlen(extra) + a, "FIRST", 5); memcpy(edited + strlen(extra) + z, "LAST", 4);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &allocator;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, n) == XUI_OK);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
        patches[0].iStart = z; patches[0].iEnd = z + 4; patches[0].sText = "LAST"; patches[0].iTextBytes = 4;
        patches[1].iStart = a; patches[1].iEnd = a + 5; patches[1].sText = "FIRST"; patches[1].iTextBytes = 5;
        patches[2].sText = extra; patches[2].iTextBytes = strlen(extra);
        for (i = 0; i < 3; i++) patches[i].iSize = sizeof(patches[i]);
        if (mode) {
            CHECK(xuiDocumentPrepareSource(d, &td, patches, 3, &prepare) == XUI_OK);
            CHECK(xuiDocumentPrepareRun(prepare) == XUI_OK && xuiDocumentPreparePublish(d, prepare, NULL) == XUI_OK);
            xuiDocumentPrepareRelease(prepare);
        } else {
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
            for (i = 0; i < 3; i++) CHECK(xuiDocumentTxnReplaceSource(t, patches[i].iStart, patches[i].iEnd,
                patches[i].sText, patches[i].iTextBytes) == XUI_OK);
            CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        }
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        CHECK(exclusive_syntax_blobs(before->state, after->state) == 0);
        memory_oracle_check(d);
        if (sizes[s] == 64) CHECK(document_memory(d).iHistoryBytes < 65536);
        full = test_markdown_open(edited); CHECK(xuiDocumentAcquireSnapshot(full, &expected) == XUI_OK);
        inc_snapshot_equal(after, expected); exact_syntax(after->state, expected->state);
        CHECK(xuiDocumentSnapshotCreateSourceIndex(after, NULL, &index) == XUI_OK);
        xuiDocumentSourceIndexRelease(index); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(full);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); memory_oracle_check(d);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); memory_oracle_check(d);
        printf("CST sharing definitions=%u mode=%u exclusive-old-arena-bytes=0 history=%llu\n",
            sizes[s] * 2 + 2, mode, (unsigned long long)document_memory(d).iHistoryBytes);
        xuiDocumentRelease(d); exact_syntax(before->state, before->state); exact_syntax(after->state, after->state);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); CHECK(!allocator.live);
        free(source); free(edited);
    }
    puts("CST sharing: six shifted mixed/duplicate definition cohorts; all original authoritative arena blobs retained, exact full parse, independent memory graph, Undo/Redo and zero leaks");
    return 0;
}
