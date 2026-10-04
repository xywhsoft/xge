static char* reference_sharing_source(size_t body, int used, size_t* offset)
{
    const char* prefix = used ? "use [^n] and [r]\n\n[r]: /stable 'Title'\n\n[^n]: " :
        "use [r]\n\n[r]: /stable 'Title'\n\n[^n]: ";
    const char* suffix = "\n\n[^n]: duplicate untouched\n\n[r]: /unused\n";
    char* source = malloc(strlen(prefix) + body + strlen(suffix) + 1); size_t i; uint32_t random = 12345;
    CHECK(source); *offset = strlen(prefix); memcpy(source, prefix, *offset);
    for (i = 0; i < body; i++) { random = random * 1664525u + 1013904223u; source[*offset + i] = (char)('a' + (random >> 16) % 26); }
    strcpy(source + *offset + body, suffix); return source;
}
static void reference_sharing_compare(xui_document d, const char* source)
{
    xui_document oracle = test_markdown_open(source); xui_document_snapshot actual, expected;
    CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
    inc_snapshot_equal(actual, expected); xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
}
#ifndef XUI_DLL
static uint64_t reference_sharing_cache_history(xui_document_snapshot before, xui_document_snapshot after)
{
    memory_oracle oracle = {0}; uint64_t bytes = 0; size_t i;
    memory_oracle_visit(&oracle, before->state->reference_values, 2);
    memory_oracle_visit(&oracle, after->state->reference_values, 1);
    for (i = 0; i < oracle.count; i++) if (oracle.entries[i].mask == 2)
        bytes += ((doc_allocation*)oracle.entries[i].pointer - 1)->value.bytes;
    free(oracle.entries); return bytes;
}
#endif
static void markdown_reference_range_sharing(void)
{
    static const size_t sizes[] = {4096, 131072, 1048576}; unsigned si, used, mode, cases = 0;
    for (si = 0; si < sizeof(sizes) / sizeof(*sizes); si++) for (used = 0; used < 2; used++) for (mode = 0; mode < 8; mode++) {
        size_t offset, n = sizes[si], positions[3], i; char *source = reference_sharing_source(n, (int)used, &offset), *edited;
        xui_document d = test_markdown_open(source); xui_document_snapshot before, after, restored;
        xui_document_transaction t = NULL; xui_doc_txn_desc_t td = {0}; xui_doc_source_patch_t patches[3] = {{0}};
        xui_document_prepare prepare = NULL; xui_doc_memory_stats_t memory; unsigned changes = mode < 3 ? 1 : 3;
        char replacements[3][3] = {{'X',0},{'Y',0},{'Z',0}};
        edited = malloc(strlen(source) + 4); CHECK(edited); strcpy(edited, source);
        positions[0] = offset; positions[1] = offset + n / 2; positions[2] = offset + n - 1;
        if (mode < 3) positions[0] = positions[mode];
        if (mode >= 6) { strcpy(replacements[0], "XY"); replacements[1][0] = 0; positions[1]++; }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
        for (i = 0; i < changes; i++) {
            size_t inserted = strlen(replacements[i]);
            memmove(edited + positions[i] + inserted, edited + positions[i] + 1, strlen(edited + positions[i] + 1) + 1);
            memcpy(edited + positions[i], replacements[i], inserted); patches[i].iSize = sizeof(patches[i]);
            patches[i].iStart = positions[i]; patches[i].iEnd = positions[i] + 1;
            patches[i].sText = replacements[i]; patches[i].iTextBytes = inserted;
        }
        if (mode == 4 || mode == 5 || mode == 7) {
            CHECK(xuiDocumentPrepareSource(d, &td, patches, mode == 4 ? changes : 1, &prepare) == XUI_OK);
            if (mode == 5 || mode == 7) {
                xui_document_prepare continued;
                CHECK(xuiDocumentPrepareContinueSource(d, prepare, patches + 1, changes - 1, &continued) == XUI_OK);
                xuiDocumentPrepareRelease(prepare); prepare = continued;
            }
            CHECK(xuiDocumentPrepareRun(prepare) == XUI_OK && xuiDocumentPreparePublish(d, prepare, NULL) == XUI_OK);
            xuiDocumentPrepareRelease(prepare);
        } else {
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
            for (i = 0; i < changes; i++) CHECK(xuiDocumentTxnReplaceSource(t, positions[i], positions[i] + 1, replacements[i], strlen(replacements[i])) == XUI_OK);
            CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        }
        reference_sharing_compare(d, edited); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
#ifndef XUI_DLL
        CHECK(reference_sharing_cache_history(before, after) < 32768); memory_oracle_check(d);
#endif
        memory = document_memory(d);
        CHECK(memory.iHistoryBytes < 65536);
        printf("Reference range sharing body=%zu used=%u mode=%u history=%llu", n, used, mode, (unsigned long long)memory.iHistoryBytes);
#ifndef XUI_DLL
        printf(" cache-exclusive=%llu", (unsigned long long)reference_sharing_cache_history(before, after));
#endif
        putchar('\n');
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(before, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(after, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK);
#ifndef XUI_DLL
        memory_oracle_check(d); CHECK(document_memory(d).iSnapshotAdditionalBytes < 65536);
#endif
        xuiDocumentRelease(d); inc_snapshot_equal(before, before); inc_snapshot_equal(after, after);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); free(source); free(edited); cases++;
    }
    printf("Reference range sharing: %u public SOURCE/Prepare/Continue, used/unused/duplicate definitions, head/middle/tail/multiple changes, full-load oracle, Undo/Redo and retained snapshots passed\n", cases);
}
#ifndef XUI_DLL
static void reference_sharing_text_memory(memory_oracle* memory, doc_state* state, uint64_t id)
{
    doc_node* node = doc_index_get(state->index, id); uint64_t i;
    CHECK(node); memory_oracle_visit(memory, node->text, 1);
    for (i = 0; i < doc_seq_size(node->children); i++)
        reference_sharing_text_memory(memory, state, doc_seq_get_id(node->children, i));
}
static uint64_t reference_sharing_allocation_bytes(memory_oracle* memory)
{
    uint64_t bytes = 0; size_t i;
    for (i = 0; i < memory->count; i++) bytes += ((doc_allocation*)memory->entries[i].pointer - 1)->value.bytes;
    return bytes;
}
#endif
static void markdown_reference_storage_shrink(void)
{
    unsigned used, pending;
    for (used = 0; used < 2; used++) for (pending = 0; pending < 2; pending++) {
        size_t offset; char* source = reference_sharing_source(1048576, (int)used, &offset), *edited;
        xui_document d = test_markdown_open(source); xui_document_snapshot before, after, restored;
        xui_doc_txn_desc_t td = {0}; xui_document_transaction t; xui_document_prepare p;
        xui_doc_source_patch_t patch = {0}; size_t removed = 1048576 - 64;
        edited = malloc(strlen(source) + 1); CHECK(edited); strcpy(edited, source);
        memmove(edited + offset + 64, edited + offset + 1048576, strlen(edited + offset + 1048576) + 1);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
        if (pending) {
            patch.iSize = sizeof(patch); patch.iStart = offset + 64; patch.iEnd = patch.iStart + removed; patch.sText = "";
            CHECK(xuiDocumentPrepareSource(d, &td, &patch, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK &&
                xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
        } else {
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK &&
                xuiDocumentTxnReplaceSource(t, offset + 64, offset + 1048576, "", 0) == XUI_OK &&
                xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        }
        reference_sharing_compare(d, edited); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
#ifndef XUI_DLL
        {
            memory_oracle cache = {0}, text = {0}; uint64_t cache_bytes, text_bytes;
            memory_oracle_visit(&cache, after->state->reference_values, 1);
            reference_sharing_text_memory(&text, after->state, DOC_ROOT);
            cache_bytes = reference_sharing_allocation_bytes(&cache); text_bytes = reference_sharing_allocation_bytes(&text);
            CHECK(cache_bytes < 32768 && text_bytes < 32768); memory_oracle_check(d);
            printf("Reference shrink 1MiB to 64 bytes used=%u prepare=%u: cache=%llu semantic-text=%llu\n", used, pending,
                (unsigned long long)cache_bytes, (unsigned long long)text_bytes);
            free(cache.entries); free(text.entries);
        }
#endif
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(before, restored); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(after, restored); xuiDocumentSnapshotRelease(restored);
        xuiDocumentRelease(d); inc_snapshot_equal(before, before); inc_snapshot_equal(after, after);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); free(source); free(edited);
    }
    puts("Reference storage shrink: four used/unused SOURCE/Prepare, large removed payload does not remain pinned by tiny cache/text ranges, full oracle and Undo/Redo passed");
}
#ifndef XUI_DLL
static void markdown_reference_range_sharing_failures(void)
{
    unsigned cancel, sample; long totals[2] = {0};
    for (cancel = 0; cancel < 2; cancel++) for (sample = 0; sample < 4; sample++) {
        long point; int success = 0; size_t offset; char* source = reference_sharing_source(4096, (int)(sample & 1), &offset);
        for (point = 0; point < 6000 && !success; point++) {
            reference_values_cancel_allocator allocator = {{-1,0},-1,ATOMIC_VAR_INIT(0)};
            xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t td = {0}; xui_document d; xui_document_transaction t;
            xui_document_snapshot before, after; uint64_t revision; unsigned live; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.onAlloc = reference_values_cancel_alloc;
            desc.onFree = reference_values_cancel_free; desc.pAllocatorUser = &allocator;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            revision = xuiDocumentGetRevision(d); live = allocator.allocator.live;
            td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE; CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
            if (cancel) { t->cancellation = &allocator.cancellation; allocator.remaining = point; }
            else allocator.allocator.remaining = point;
            result = sample >= 2 ? xuiDocumentTxnReplaceSource(t, offset + 64, offset + 4096, "", 0) :
                xuiDocumentTxnReplaceSource(t, offset, offset + 1, "X", 1);
            if (sample < 2 && result == XUI_OK) result = xuiDocumentTxnReplaceSource(t, offset + 2048, offset + 2049, "Y", 1);
            if (sample < 2 && result == XUI_OK) result = xuiDocumentTxnReplaceSource(t, offset + 4095, offset + 4096, "Z", 1);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            allocator.remaining = allocator.allocator.remaining = -1; atomic_store(&allocator.cancellation, 0); xuiDocumentTxnRelease(t);
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result == XUI_OK) { success = 1; CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); }
            else {
                CHECK(result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY) &&
                    xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
                inc_snapshot_equal(before, after); CHECK(before->state == after->state && allocator.allocator.live == live + 1);
            }
            memory_oracle_check(d); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after);
            xuiDocumentRelease(d); CHECK(!allocator.allocator.live);
        }
        CHECK(success); totals[cancel] += point - 1; free(source);
    }
    printf("Reference range sharing faults: %ld OOM and %ld cancellation checkpoints, multi-edit source/tree/cache/revision/history atomic and no leaks\n", totals[0], totals[1]);
}
#else
static void markdown_reference_range_sharing_failures(void) {}
#endif
