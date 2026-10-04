static void dependency_unresolved_caret(void)
{
    static const struct { const char *source, *text; uint32_t kind; } cases[] = {
        {"use[^b][r]\n\n[r]: /target\n", "^b", XUI_DOC_TEXT},
        {"use![^b][r]\n\n[r]: /target\n", "^b", XUI_DOC_IMAGE},
        {"use[^bad label][r]\n\n[r]: /target\n", "^bad label", XUI_DOC_TEXT},
        {"use![^bad label][r]\n\n[r]: /target\n", "^bad label", XUI_DOC_IMAGE},
        {"use[^b](/target)\n", "^b", XUI_DOC_TEXT},
        {"use![^b](/target)\n", "^b", XUI_DOC_IMAGE},
    };
    unsigned dialect, i, total = 0;
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++)
    for (i = 0; i < sizeof(cases) / sizeof(*cases); i++) {
        xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot snapshot;
        uint64_t paragraph, child, at; xui_doc_node_info_t parent = {0}; unsigned found = 0;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, cases[i].source, strlen(cases[i].source)) == XUI_OK &&
            xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK && xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK);
        parent.iSize = sizeof(parent); CHECK(xuiDocumentSnapshotGetNode(snapshot, paragraph, &parent) == XUI_OK);
        for (at = 0; at < parent.iChildCount; at++) {
            xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
            CHECK(xuiDocumentSnapshotGetChild(snapshot, paragraph, at, &child) == XUI_OK &&
                xuiDocumentSnapshotGetNode(snapshot, child, &node) == XUI_OK);
            if (node.iKind == cases[i].kind && !strcmp(node.sResource, "/target")) {
                if (node.iKind == XUI_DOC_TEXT) CHECK(node.tAttributes.iMarks & XUI_DOC_LINK);
                expect_text(snapshot, child, cases[i].text); found++;
            }
        }
        CHECK(found == 1); xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d); total++;
    }
    CHECK(inc_case("use target\n\n[r]: /target\n", XUI_MD_EXTENDED, 4, 6, "[^b][r]", 0) == 0);
    CHECK(inc_case("use target\n\n[r]: /target\n", XUI_MD_EXTENDED, 4, 6, "![^b][r]", 1) == 0);
    printf("Unresolved footnote caret: %u literal text/image labels in three dialects, no false wiki callback; SOURCE/Prepare and Undo passed\n", total);
}
static void dependency_differential(void)
{
    static const unsigned fixtures[] = {0, 4, 5, 8, 9, 10};
    static const char* edits[] = {"x", "[", "[^b]", "", "\n", "```\n", "\n    ", "\r"};
    unsigned f, e; uint64_t cases = 0, incremental = 0;
    for (f = 0; f < sizeof(fixtures) / sizeof(*fixtures); f++) {
        unsigned fixture = fixtures[f]; char* source = dependency_source(fixture, 8, 1);
        char* changed = strstr(source, dependency_cases[fixture].source); size_t at;
        CHECK(changed);
        for (at = 0; at < strlen(dependency_cases[fixture].source); at++)
        for (e = 0; e < sizeof(edits) / sizeof(*edits); e++) {
            incremental += inc_case(source, XUI_MD_EXTENDED, (uint64_t)(changed - source) + at,
                1, edits[e], (int)(cases & 1)); cases++;
        }
        free(source);
    }
    CHECK(incremental > cases / 3 && incremental <= cases);
    printf("Dependency grammar differential: %llu byte/markup/newline/container edits, %llu incremental paths, complete source/semantic/block and inline syntax/definitions/Undo oracle passed\n",
        (unsigned long long)cases, (unsigned long long)incremental);
}
static void dependency_faults(void)
{
    static const unsigned fixtures[] = {0, 5, 9, 10};
    unsigned mode, sample; long totals[3] = {0};
    for (mode = 0; mode < 3; mode++) for (sample = 0; sample < sizeof(fixtures) / sizeof(*fixtures); sample++) {
        unsigned fixture = fixtures[sample];
        char* source = dependency_source(fixture, 4, 0); char* found = strstr(source, dependency_cases[fixture].needle);
        size_t at, removed = strlen(dependency_cases[fixture].needle), added = strlen(dependency_cases[fixture].replacement);
        char* edited = malloc(strlen(source) + added + 1); long point; int success = 0;
        CHECK(edited && found); at = (size_t)(found - source);
        memcpy(edited, source, at); memcpy(edited + at, dependency_cases[fixture].replacement, added);
        strcpy(edited + at + added, source + at + removed);
        for (point = 0; point < 6000 && !success; point++) {
            fail_allocator failing = {-1, 0}; prepare_cancel_allocator cancelling = {0, -1, 0, NULL};
            xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot before, after;
            xui_document_prepare p = NULL; xui_document_transaction t = NULL; uint64_t revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = mode == 2 ? prepare_cancel_alloc : failing_alloc;
            desc.onFree = mode == 2 ? prepare_cancel_free : failing_free;
            desc.pAllocatorUser = mode == 2 ? (void*)&cancelling : (void*)&failing;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
                xuiDocumentMarkSaved(d, before) == XUI_OK);
            revision = xuiDocumentGetRevision(d);
            if (mode != 2) failing.remaining = point;
            if (!mode) {
                xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                result = xuiDocumentBeginTransaction(d, &td, &t);
                if (result == XUI_OK) result = xuiDocumentTxnReplaceSource(t, at, at + removed,
                    dependency_cases[fixture].replacement, added);
            } else {
                xui_doc_source_patch_t patch = prepare_patch(at, at + removed, dependency_cases[fixture].replacement);
                result = xuiDocumentPrepareSource(d, NULL, &patch, 1, &p);
                if (mode == 2) { cancelling.prepare = p; cancelling.remaining = point; }
                if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
            }
            cancelling.prepare = NULL; cancelling.remaining = failing.remaining = -1;
            if (result == XUI_OK) result = mode ? xuiDocumentPreparePublish(d, p, NULL) : xuiDocumentTxnCommit(t, NULL);
            xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result != XUI_OK) {
                CHECK(result == (mode == 2 ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY) &&
                    xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d));
                expect_identity_tree(before, after, 1); inc_snapshot_equal(before, after); dependency_snapshot(after, source);
            } else {
                success = 1; dependency_snapshot(after, edited);
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d));
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentPrepareRelease(p);
            CHECK(xuiDocumentCancelPrepare(d) == XUI_OK);
#ifndef XUI_DLL
            memory_oracle_check(d);
#endif
            xuiDocumentRelease(d); CHECK(!failing.live && !cancelling.live);
        }
        CHECK(success); totals[mode] += point - 1;
        printf("Dependency faults mode=%u fixture=%u: %ld atomic and leak-free checkpoints\n", mode, fixture, point - 1); fflush(stdout);
        free(source); free(edited);
    }
    printf("Dependency faults: %ld SOURCE OOM, %ld Prepare OOM and %ld Prepare cancellation checkpoints passed\n",
        totals[0], totals[1], totals[2]);
}
static void dependency_continue_running(void)
{
    static const unsigned fixtures[] = {0, 5, 9, 10};
    unsigned sample;
    for (sample = 0; sample < sizeof(fixtures) / sizeof(*fixtures); sample++) {
        unsigned fixture = fixtures[sample];
        char* source = dependency_source(fixture, 16, 0), *found = strstr(source, dependency_cases[fixture].needle);
        char* edited = malloc(strlen(source) + 256); size_t at, removed, added, length;
        prepare_gate_allocator a; xui_doc_desc_t desc = {0}; xui_document d; xui_document_prepare p, next;
        xui_doc_source_patch_t patches[2]; xthread* worker; xui_document_snapshot after;
        CHECK(edited && found); at = (size_t)(found - source); removed = strlen(dependency_cases[fixture].needle);
        added = strlen(dependency_cases[fixture].replacement);
        memcpy(edited, source, at); memcpy(edited + at, dependency_cases[fixture].replacement, added);
        strcpy(edited + at + added, source + at + removed); length = strlen(edited);
        strcat(edited, "\n[extra]: /unused\n");
        patches[0] = prepare_patch(at, at + removed, dependency_cases[fixture].replacement);
        patches[1] = prepare_patch(length, length, "\n[extra]: /unused\n");
        atomic_init(&a.live, 0); atomic_init(&a.gate, 0); a.owner = xrtThreadCurrentId();
        atomic_init(&a.entered, 0); atomic_init(&a.resume, 0);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.onAlloc = prepare_gate_alloc; desc.onFree = prepare_gate_free; desc.pAllocatorUser = &a;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
            xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentPrepareSource(d, NULL, patches, 1, &p) == XUI_OK);
        atomic_store(&a.gate, 1); worker = prepare_start(p); prepare_gate_wait(&a.entered);
        CHECK(prepare_info(p).iState == XUI_DOC_PREPARE_RUNNING &&
            xuiDocumentPrepareContinueSource(d, p, patches + 1, 1, &next) == XUI_OK);
        atomic_store(&a.resume, 1); CHECK(prepare_finish(worker) == XUI_DOC_ERROR_STALE);
        xuiDocumentPrepareRelease(p);
        CHECK(xuiDocumentPrepareRun(next) == XUI_OK && xuiDocumentPreparePublish(d, next, NULL) == XUI_OK);
        xuiDocumentPrepareRelease(next); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        dependency_snapshot(after, edited); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
        CHECK(!atomic_load(&a.live)); free(source); free(edited);
    }
    puts("Dependency RUNNING Continue: transitive footnotes and container-internal definitions, stale predecessor, full oracle and no leaks passed");
}
