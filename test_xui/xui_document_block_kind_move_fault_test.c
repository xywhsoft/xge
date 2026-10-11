#include "src/xui_document_internal.h"
#define BLOCK_KINDS_MATRIX_NO_MAIN
#include "xui_document_block_kind_move_test.c"
#undef BLOCK_KINDS_MATRIX_NO_MAIN
#define FAULT_CHECK(x) do { if (!(x)) { fprintf(stderr, "Block kind fault line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
typedef struct container_fault_allocator {
    long remaining, cancel_after;
    unsigned live;
    atomic_int cancellation;
} container_fault_allocator;
static void* container_fault_alloc(void* user, size_t size)
{
    container_fault_allocator* a = user; void* p;
    if (!a->cancel_after) atomic_store(&a->cancellation, XUI_DOC_ERROR_CANCELLED);
    else if (a->cancel_after > 0) a->cancel_after--;
    if (!a->remaining) return NULL;
    if (a->remaining > 0) a->remaining--;
    p = malloc(size); if (p) a->live++; return p;
}
static void container_fault_free(void* user, void* p)
{
    container_fault_allocator* a = user; FAULT_CHECK(a->live); a->live--; free(p);
}
static void container_fault_notify(xui_document d, xui_document_change_set change, void* user)
{
    (void)d; (void)change; (*(unsigned*)user)++;
}
static int container_blob_found(doc_sequence* source, doc_blob* blob)
{
    return source && (source->blob == blob || container_blob_found(source->left, blob) || container_blob_found(source->right, blob));
}
static int container_old_payload_retained(doc_sequence* old, doc_sequence* now)
{
    return !old || ((!old->blob || container_blob_found(now, old->blob)) &&
        container_old_payload_retained(old->left, now) && container_old_payload_retained(old->right, now));
}
static int container_fault_attempt(xui_document d, uint64_t block, uint64_t parent,
    uint64_t index, unsigned cancel, long point, container_fault_allocator* a)
{
    xui_document_transaction t = NULL;
    int result;
    if (!cancel) a->remaining = point;
    result = xuiDocumentBeginTransaction(d, NULL, &t);
    if (result != XUI_OK) return result;
    if (cancel) { t->cancellation = &a->cancellation; a->cancel_after = point; }
    result = xuiDocumentTxnMoveNode(t, block, parent, index);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
    if (result != XUI_OK) FAULT_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
    xuiDocumentTxnRelease(t); return result;
}
int main(void)
{
    unsigned scans = 0; uint64_t failed_points = 0;
    for (unsigned fixture = 0; fixture < sizeof(cases) / sizeof(*cases); fixture++) for (unsigned cancel = 0; cancel < 2; cancel++) {
        const block_case* c = &cases[fixture];
        char before[2048], wanted[2048];
        source_variant(c->before, before, 1, 1, 0);
        source_variant(c->after, wanted, 1, 1, fixture == 26 ? 1 : 0);
        xui_document oracle; xui_document_snapshot expected; long point;
        FAULT_CHECK(open_document(wanted, &oracle) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        for (point = 0; point < 4096; point++) {
            container_fault_allocator a = {-1, -1, 0, ATOMIC_VAR_INIT(0)};
            xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_snapshot old, now, restored;
            xui_doc_node_info_t parent = {0}, moving_info = {0};
            uint64_t moving, target, moved, old_index = 0, revision, token;
            unsigned notifications = 0; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = container_fault_alloc; desc.onFree = container_fault_free; desc.pAllocatorUser = &a;
            parent.iSize = moving_info.iSize = sizeof(parent);
            FAULT_CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, before, strlen(before)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
                xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, container_fault_notify, &notifications, &token) == XUI_OK);
            revision = xuiDocumentGetRevision(d);
            FAULT_CHECK(moving_node(old, before, c->moving_kind, &moving) == XUI_OK &&
                find_kind(old, 1, c->target_kind, &target) == XUI_OK && xuiDocumentSnapshotGetNode(old, target, &parent) == XUI_OK &&
                xuiDocumentSnapshotGetNode(old, moving, &moving_info) == XUI_OK);
            for (;;) {
                FAULT_CHECK(xuiDocumentSnapshotGetChild(old, moving_info.iParentId, old_index, &moved) == XUI_OK);
                if (moved == moving) break;
                old_index++;
            }
            result = container_fault_attempt(d, moving, target, c->index, cancel, point, &a);
            a.remaining = a.cancel_after = -1; atomic_store(&a.cancellation, 0);
            FAULT_CHECK(xuiDocumentAcquireSnapshot(d, &now) == XUI_OK);
            if (result != XUI_OK) {
                FAULT_CHECK(result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY) &&
                    !notifications && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) &&
                    !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && now->state == old->state && source_equal(now, before));
                FAULT_CHECK(container_fault_attempt(d, moving, target, c->index, 0, -1, &a) == XUI_OK && source_equal(now, before));
                xuiDocumentSnapshotRelease(now); FAULT_CHECK(xuiDocumentAcquireSnapshot(d, &now) == XUI_OK);
            }
            FAULT_CHECK(notifications == 1 && xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentIsDirty(d) &&
                xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && source_equal(now, wanted) && source_equal(old, before) &&
                all_ids(old, now, 1, moving, target) &&
                xuiDocumentSnapshotGetChild(now, target, c->index, &moved) == XUI_OK && moved == moving);
            char *actual_html = NULL, *expected_html = NULL; uint64_t na, nb;
            FAULT_CHECK(xuiDocumentExportHtml(now, &actual_html, &na) == XUI_OK && xuiDocumentExportHtml(expected, &expected_html, &nb) == XUI_OK &&
                na == nb && !memcmp(actual_html, expected_html, (size_t)na));
            xuiDocumentFreeBuffer(actual_html); xuiDocumentFreeBuffer(expected_html);
            FAULT_CHECK(container_old_payload_retained(old->state->source, now->state->source) &&
                now->state->source_storage_bytes >= old->state->source_storage_bytes &&
                now->state->source_storage_bytes - old->state->source_storage_bytes <= 64 &&
                now->state->source_open_brackets == old->state->source_open_brackets);
            FAULT_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && notifications == 2 && !xuiDocumentIsDirty(d) &&
                !xuiDocumentCanUndo(d) && xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK &&
                source_equal(restored, before) && all_ids(old, restored, 1, 0, 0) &&
                xuiDocumentSnapshotGetChild(restored, moving_info.iParentId, old_index, &moved) == XUI_OK && moved == moving);
            xuiDocumentSnapshotRelease(restored);
            FAULT_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && notifications == 3 && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK &&
                source_equal(restored, wanted) && source_equal(old, before) && all_ids(old, restored, 1, moving, target));
            xuiDocumentSnapshotRelease(restored); xuiDocumentSnapshotRelease(now);
            xuiDocumentUnsubscribe(d, token); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d);
            FAULT_CHECK(!a.live);
            if (result == XUI_OK) break;
        }
        FAULT_CHECK(point > 0 && point < 4096); scans++; failed_points += (uint64_t)point;
        printf("Block kind fault fixture=%u cancel=%u: %ld points; atomic state/notifications/poisoned commit/retry/all-NodeId/history/shared payload/zero leaks passed\n", fixture, cancel, point); fflush(stdout);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
    }
    FAULT_CHECK(scans == 54);
    printf("Block kind faults: %u exhaustive OOM/cancel scans; %llu unpublished failure points; all invariants passed\n", scans, (unsigned long long)failed_points);
    return 0;
}
