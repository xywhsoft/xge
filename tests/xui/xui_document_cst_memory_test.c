#define main uninvoked_core_main
#include "xui_document_test.c"
#undef main

static const char* source = "head[^u] [r]\n\n[r]: /stable 'Title'\n\n"
    "[^u]: **bold** [a](<uri> \"title\")\n"
    "[^u]: *duplicate*\n\n[^hidden]: > - 中文\n    >   `code`\n";

static void check_source_index(xui_document_snapshot snapshot)
{
    xui_document_source_index index;
    CHECK(xuiDocumentSnapshotCreateSourceIndex(snapshot, NULL, &index) == XUI_OK);
    xuiDocumentSourceIndexRelease(index);
}

int main(void)
{
    unsigned mode, operation, cases = 0;
    for (mode = 0; mode < 3; mode++) for (operation = 0; operation < 6; operation++) {
        fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d, full;
        xui_document_snapshot before, after, restored, expected;
        xui_document_source_index retained_index;
        xui_doc_txn_desc_t td = {0}; xui_doc_source_patch_t patch = {0};
        const char *needle = NULL, *replacement = ""; size_t start = 0, removed = 0, inserted, n = strlen(source);
        char edited[512], raw[512]; xui_document_prepare p = NULL; xui_document_transaction t = NULL;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, n) == XUI_OK);
        CHECK(doc_seq_size(d->state->reference_syntax) > 0);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        CHECK(xuiDocumentMarkSaved(d, before) == XUI_OK && !xuiDocumentIsDirty(d));
        CHECK(xuiDocumentSnapshotCreateSourceIndex(before, NULL, &retained_index) == XUI_OK);
        memory_oracle_check(d);
        switch (operation) {
        case 0: needle = "uri"; replacement = "new-uri"; break;
        case 1: needle = "head[^u]"; replacement = "head[^missing]"; break;
        case 2: replacement = "[^new]: **new**\n\n"; break;
        case 3: needle = "[^hidden]: > - 中文\n    >   `code`\n"; break;
        case 4: needle = "*duplicate*"; replacement = "**longer duplicate**"; break;
        case 5: removed = n; break;
        }
        if (needle) { start = (size_t)(strstr(source, needle) - source); removed = strlen(needle); }
        inserted = strlen(replacement); CHECK(n - removed + inserted < sizeof(edited));
        memcpy(edited, source, start); memcpy(edited + start, replacement, inserted);
        memcpy(edited + start + inserted, source + start + removed, n - start - removed + 1);
        td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
        patch.iSize = sizeof(patch); patch.iStart = start; patch.iEnd = start + removed;
        patch.sText = replacement; patch.iTextBytes = inserted;
        if (mode) {
            CHECK(xuiDocumentPrepareSource(d, &td, &patch, 1, &p) == XUI_OK);
            memory_oracle_check(d);
            if (mode == 2) {
                int result;
                /* Exercise the detached complete-root publication plan with
                 * real parsed CST, then restore exact ownership before Publish. */
                doc_memory_current(d->state, NULL);
                result = xuiDocumentPrepareRun(p);
                doc_memory_current(NULL, d->state);
                CHECK(result == XUI_OK);
            } else CHECK(xuiDocumentPrepareRun(p) == XUI_OK);
            memory_oracle_check(d);
            CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
        } else {
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
            CHECK(xuiDocumentTxnReplaceSource(t, start, start + removed, replacement, inserted) == XUI_OK);
            CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        }
        memory_oracle_check(d); CHECK(xuiDocumentIsDirty(d));
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); memory_oracle_check(d);
        full = test_markdown_open(edited); CHECK(xuiDocumentAcquireSnapshot(full, &expected) == XUI_OK);
        inc_snapshot_equal(after, expected); check_source_index(after);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(full);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d)); memory_oracle_check(d);
        CHECK(xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); inc_snapshot_equal(before, restored);
        xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentIsDirty(d)); memory_oracle_check(d);
        CHECK(xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); inc_snapshot_equal(after, restored);
        xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK); memory_oracle_check(d);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); memory_oracle_check(d);
        xuiDocumentRelease(d);
        CHECK(xuiDocumentSourceIndexReadSource(retained_index, 0, raw, n) == XUI_OK && !memcmp(raw, source, n));
        xuiDocumentSourceIndexRelease(retained_index); CHECK(!a.live); cases++;
    }
    printf("CST memory: %u SOURCE/Prepare/full-root-publication cases; independent current/history/snapshot unions, saved state, full reload, Undo/Redo, retained index after Document release and zero leaks\n", cases);
    return 0;
}
