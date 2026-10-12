#define main existing_collector_main
#include "xui_document_cst_collector_test.c"
#undef main
static const char* transitions[] = {
    "head[^a]\n\n[^a]: **bold** [a](<uri> \"title\")\n[^b]: *hidden*\n",
    "head[^a] [r]\n\n[r]: target 'title'\n[^a]: [r] **body**\n[^b]: [r] [^a]\n",
    "head[^a]\n\n[^a]: first\n[^a]: *second*\n[^b]: **third**\n",
    "head[^a]\n\n[^a]: first[^b]\n[^b]: next[^a]\n[^unused]: [^b]\n",
    "[x]: first\n[^a]: *body*\n[x]: second\n[^b]: [x]\n",
    "> [^a]: > - *item*\n>     >   continuation\n>\n> [^b]: **other**\n",
    "[^a]: a | b\n    :-- | --:\n    x | y\n\n[^b]: `code`\n",
    "[^a]: **中文** &amp; \\*\n\n[^b]: <b>HTML</b>\n",
    "[^a]:\n\n\t```lang\n\tx\n\t```\n\n[^b]: more\n"
};
static void exact_forest(doc_state* state)
{
    uint64_t i, count = doc_seq_size(state->reference_syntax), definitions = doc_seq_size(state->references)/sizeof(xui_doc_reference_definition_t);
    for (i = 0; i < count; i++) {
        uint64_t key = doc_seq_get_id(state->reference_syntax, i); doc_source_cst_header h;
        CHECK(key && key <= definitions && (!i || doc_seq_get_id(state->reference_syntax, i-1) < key));
        CHECK(doc_source_cst_info(state, key-1, &h) == XUI_OK);
    }
    CHECK(check_forest(state) == count);
}
static int next_cst_binding(xui_document_source_index index, uint64_t* at, uint64_t count,
    xui_doc_source_binding_t* result, xui_doc_source_record_t* record)
{
    while (*at < count) {
        result->iSize = sizeof(*result); CHECK(xuiDocumentSourceIndexGetBinding(index, (*at)++, result) == XUI_OK);
        if (result->iOriginKind != XUI_DOC_SOURCE_BINDING_DEFINITION_SYNTAX) continue;
        record->iSize = sizeof(*record); CHECK(xuiDocumentSourceIndexGetRecord(index, result->tRange.iRecordIndex, record) == XUI_OK);
        result->tRange.iRecordIndex = 0; record->iParentIndex = 0; return 1;
    }
    return 0;
}
static void equivalent(xui_document_snapshot actual, xui_document_snapshot full)
{
    xui_document_source_index a, b; xui_doc_source_index_info_t ai = {0}, bi = {0}; uint64_t at = 0, bt = 0;
    char *ah, *bh; uint64_t an, bn;
    exact_forest(actual->state); exact_forest(full->state);
    CHECK(doc_seq_size(actual->state->reference_syntax) == doc_seq_size(full->state->reference_syntax));
    compare_cst(actual->state, full->state);
    CHECK(xuiDocumentExportHtml(actual, &ah, &an) == XUI_OK && xuiDocumentExportHtml(full, &bh, &bn) == XUI_OK);
    CHECK(an == bn && !memcmp(ah, bh, (size_t)an)); xuiDocumentFreeBuffer(ah); xuiDocumentFreeBuffer(bh);
    CHECK(xuiDocumentSnapshotCreateSourceIndex(actual, NULL, &a) == XUI_OK && xuiDocumentSnapshotCreateSourceIndex(full, NULL, &b) == XUI_OK);
    ai.iSize = bi.iSize = sizeof(ai); CHECK(xuiDocumentSourceIndexGetInfo(a, &ai) == XUI_OK && xuiDocumentSourceIndexGetInfo(b, &bi) == XUI_OK);
    for (;;) {
        xui_doc_source_binding_t av = {0}, bv = {0}; xui_doc_source_record_t ar = {0}, br = {0};
        int af = next_cst_binding(a, &at, ai.iBindingCount, &av, &ar), bf = next_cst_binding(b, &bt, bi.iBindingCount, &bv, &br);
        CHECK(af == bf); if (!af) break;
        CHECK(!memcmp(&av, &bv, sizeof(av)) && !memcmp(&ar, &br, sizeof(ar)));
    }
    xuiDocumentSourceIndexRelease(a); xuiDocumentSourceIndexRelease(b);
}
static uint64_t codepoint_end(const char* source, uint64_t at, uint64_t n)
{
    if (at == n) return at;
    do { at++; } while (at < n && ((unsigned char)source[at] & 0xc0) == 0x80);
    return at;
}
static void transition_case(const char* source, uint64_t n, uint64_t at, unsigned operation, unsigned mode)
{
    static const char* insert[] = {"x", "", "**"}; uint64_t end = operation ? codepoint_end(source, at, n) : at;
    uint64_t added = strlen(insert[operation]), expected_n = n-(end-at)+added;
    char* expected = malloc((size_t)expected_n+1); fault_allocator allocator = {-1, 0};
    xui_document d = open_doc(source, n, 3, &allocator), oracle; xui_document_snapshot before, after, full, undo;
    CHECK(expected); memcpy(expected, source, (size_t)at); memcpy(expected+at, insert[operation], (size_t)added);
    memcpy(expected+at+added, source+end, (size_t)(n-end)); expected[expected_n] = 0;
    CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
    if (!mode) {
        xui_document_transaction t; xui_doc_txn_desc_t desc = {0}; desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
        int result;
        CHECK(xuiDocumentBeginTransaction(d, &desc, &t) == XUI_OK);
        result = xuiDocumentTxnReplaceSource(t, at, end, insert[operation], added);
        if (result != XUI_OK) fprintf(stderr, "SOURCE edit returned %d at %llu\n", result, (unsigned long long)at);
        CHECK(result == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    } else {
        xui_document_prepare p; xui_doc_source_patch_t patch = {0}; patch.iSize = sizeof(patch);
        patch.iStart = at; patch.iEnd = end; patch.sText = insert[operation]; patch.iTextBytes = added;
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK);
        CHECK(d->state == before->state && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
    }
    oracle = open_doc(expected, expected_n, 3, &allocator);
    CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &full) == XUI_OK);
    equivalent(after, full);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &undo) == XUI_OK);
    equivalent(undo, before); xuiDocumentSnapshotRelease(undo);
    CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); exact_forest(before->state); exact_forest(after->state);
    xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentSnapshotRelease(full);
    xuiDocumentRelease(d); xuiDocumentRelease(oracle); CHECK(!allocator.live); free(expected);
}
#ifndef DOC_CST_TRANSITION_NO_MAIN
int main(void)
{
    unsigned fixture, eol, bom, op, mode; uint64_t total = 0;
    for (fixture = 0; fixture < sizeof(transitions)/sizeof(*transitions); fixture++)
    for (eol = 0; eol < 3; eol++) for (bom = 0; bom < 2; bom++) {
        uint64_t n, at; char* source = source_variant(transitions[fixture], eol, bom, &n);
        for (at = bom ? 3 : 0; at < n; at++) {
            if (((unsigned char)source[at] & 0xc0) == 0x80) continue;
            for (op = 0; op < 3; op++) for (mode = 0; mode < 2; mode++) {
                fprintf(stderr, "CST transition fixture=%u eol=%u bom=%u at=%llu op=%u mode=%u\n", fixture, eol, bom, (unsigned long long)at, op, mode);
                transition_case(source, n, at, op, mode); total++;
            }
        }
        free(source);
    }
    printf("CST transitions: %llu source insert/delete/replace and Prepare cases; global/duplicate definitions, nested dependencies/containers/table/UTF8, exact forest/public syntax bindings/HTML/full parse/Undo/Redo, zero leaks\n", (unsigned long long)total);
    return 0;
}
#endif
