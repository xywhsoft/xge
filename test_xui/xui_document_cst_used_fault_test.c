#define DOC_CST_FAULT_HELPERS_ONLY
#include "xui_document_cst_fault_test.c"
#undef DOC_CST_FAULT_HELPERS_ONLY
static const char* used_source = "head[^u]\n\n[^u]: **bold** [a](<uri> \"title\")\n";
static xui_document used_fault_doc(cancel_allocator* a)
{
    xui_doc_desc_t desc = {0}; xui_document d;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = 3;
    desc.onAlloc = cancel_alloc; desc.onFree = cancel_free; desc.pAllocatorUser = a;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, used_source, strlen(used_source)) == XUI_OK);
    CHECK(xuiDocumentClearHistory(d) == XUI_OK); return d;
}
int main(void)
{
    long totals[3] = {0}; unsigned mode;
    for (mode = 0; mode < 3; mode++) {
        cancel_allocator a = {{-1, 0}, -1, NULL, 0}; xui_document d = used_fault_doc(&a);
        xui_document_snapshot saved; xui_document_source_index old_index; uint64_t id, revision, start;
        xui_doc_source_patch_t patch = {0}; unsigned live; long point; int success = 0;
        CHECK(xuiDocumentAcquireSnapshot(d, &saved) == XUI_OK && xuiDocumentMarkSaved(d, saved) == XUI_OK);
        CHECK(xuiDocumentSnapshotCreateSourceIndex(saved, NULL, &old_index) == XUI_OK);
        id = xuiDocumentGetIdentity(d); revision = xuiDocumentGetRevision(d); live = a.base.live;
        start = (uint64_t)(strstr(used_source, "uri")-used_source);
        patch.iSize = sizeof(patch); patch.iStart = start; patch.iEnd = start+3; patch.sText = "new-uri"; patch.iTextBytes = 7;
        for (point = 0; point < 3000; point++) {
            int result; xui_document_prepare p = NULL; xui_document_transaction t = NULL; xui_doc_txn_desc_t td = {0};
            td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            if (!mode) CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
            else CHECK(xuiDocumentPrepareSource(d, &td, &patch, 1, &p) == XUI_OK);
            if (mode == 2) { a.document = d; a.at = point; a.result = XUI_ERROR_UNSUPPORTED; }
            else a.base.remaining = point;
            if (!mode) { result = xuiDocumentTxnReplaceSource(t, start, start+3, "new-uri", 7);
                if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL); }
            else { result = xuiDocumentPrepareRun(p);
                if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL); }
            a.document = NULL; a.at = -1; a.base.remaining = -1;
            if (result == XUI_OK) { success = 1; totals[mode] = point; }
            else {
                CHECK(result == (mode == 2 ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY));
                if (mode == 2) CHECK(a.result == XUI_OK);
                if (mode) CHECK(xuiDocumentCancelPrepare(d) == XUI_OK);
            }
            xuiDocumentPrepareRelease(p); xuiDocumentTxnRelease(t);
            if (success) {
                char expected[128]; xui_document oracle; xui_document_snapshot actual, full; xui_document_source_index current;
                memcpy(expected, used_source, (size_t)start); memcpy(expected+start, "new-uri", 7);
                memcpy(expected+start+7, used_source+start+3, strlen(used_source)-(size_t)start-2);
                oracle = open_doc(expected, strlen(expected), 3, NULL);
                CHECK(xuiDocumentAcquireSnapshot(d, &actual) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &full) == XUI_OK);
                compare_cst(actual->state, full->state);
                CHECK(xuiDocumentSnapshotCreateSourceIndex(actual, NULL, &current) == XUI_OK);
                xuiDocumentSourceIndexRelease(current); xuiDocumentSnapshotRelease(actual); xuiDocumentSnapshotRelease(full); xuiDocumentRelease(oracle);
                break;
            }
            unchanged(d, saved, id, revision, live, &a);
            { char raw[3]; a.base.remaining = 0;
                CHECK(xuiDocumentSourceIndexReadSource(old_index, start, raw, 3) == XUI_OK && !memcmp(raw, "uri", 3));
                a.base.remaining = -1; CHECK(a.base.live == live); }
        }
        CHECK(success); xuiDocumentSnapshotRelease(saved); xuiDocumentRelease(d);
        { char raw[3]; CHECK(xuiDocumentSourceIndexReadSource(old_index, start, raw, 3) == XUI_OK && !memcmp(raw, "uri", 3)); }
        xuiDocumentSourceIndexRelease(old_index); CHECK(!a.base.live);
    }
    printf("Used-footnote CST faults: %ld SOURCE OOM, %ld Prepare Run/Publish OOM, %ld allocation cancellation checkpoints; nonempty old CST/index/source/identity/history preserved, actual successful records equal full parse, zero leaks\n", totals[0], totals[1], totals[2]);
    return 0;
}
