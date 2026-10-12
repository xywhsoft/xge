#include "xui_document_move_cases.h"
#include "src/xui/document/xui_document_internal.h"
#define MOVE_CHECK(x) do { if (!(x)) { fprintf(stderr, "Move fault line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
typedef struct move_fault_allocator {
    long remaining, cancel_after;
    unsigned live;
    atomic_int cancellation;
} move_fault_allocator;
static void* move_fault_alloc(void* user, size_t size)
{
    move_fault_allocator* a = user; void* p;
    if (!a->cancel_after) atomic_store(&a->cancellation, XUI_DOC_ERROR_CANCELLED);
    else if (a->cancel_after > 0) a->cancel_after--;
    if (!a->remaining) return NULL;
    if (a->remaining > 0) a->remaining--;
    p = malloc(size); if (p) a->live++; return p;
}
static void move_fault_free(void* user, void* p)
{
    move_fault_allocator* a = user; MOVE_CHECK(a->live); a->live--; free(p);
}
static void move_fault_notify(xui_document d, xui_document_change_set change, void* user)
{
    (void)d; (void)change; (*(unsigned*)user)++;
}
static int move_blob_found(doc_sequence* before, doc_blob* blob)
{
    return before && (before->blob == blob || move_blob_found(before->left, blob) ||
        move_blob_found(before->right, blob));
}
static int move_blobs_shared(doc_sequence* now, doc_sequence* before)
{
    return !now || ((!now->blob || move_blob_found(before, now->blob)) &&
        move_blobs_shared(now->left, before) && move_blobs_shared(now->right, before));
}
static void move_fault_html(xui_document_snapshot now, xui_document_snapshot expected)
{
    char *a = NULL, *b = NULL; uint64_t na, nb;
    MOVE_CHECK(xuiDocumentExportHtml(now, &a, &na) == XUI_OK &&
        xuiDocumentExportHtml(expected, &b, &nb) == XUI_OK && na == nb && !memcmp(a, b, (size_t)na));
    xuiDocumentFreeBuffer(a); xuiDocumentFreeBuffer(b);
}
static int move_fault_attempt(xui_document d, const xui_doc_range_t* range,
    uint64_t block, uint64_t parent, uint64_t index, int down,
    unsigned direct, unsigned cancel, long point, move_fault_allocator* a)
{
    xui_document_transaction t = NULL; xui_doc_range_t after;
    int result;
    if (!cancel) a->remaining = point;
    result = xuiDocumentBeginTransaction(d, NULL, &t);
    if (result != XUI_OK) return result;
    if (cancel) { t->cancellation = &a->cancellation; a->cancel_after = point; }
    result = direct ? xuiDocumentTxnMoveNode(t, block, parent, down ? index + 2 : index - 1) :
        xuiDocumentTxnMoveBlockRange(t, range, down, &after);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
    if (result != XUI_OK) MOVE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
    xuiDocumentTxnRelease(t); return result;
}
int main(void)
{
    unsigned scans = 0; uint64_t failed_points = 0;
    for (unsigned fixture = 0; fixture < 14; fixture++)
    for (unsigned direct = 0; direct < 2; direct++) for (unsigned cancel = 0; cancel < 2; cancel++) {
        const sample* c = fixture < 8 ? &samples[fixture] : (fixture < 11 ? &eof_samples[fixture - 8] : &definition_eof_samples[fixture - 11]);
        char *before = move_source_variant(c->before, "\r\n", 1),
            *wanted = move_source_variant(c->after, "\r\n", 1);
        xui_document oracle; xui_document_snapshot expected; long point;
        MOVE_CHECK(before && wanted && open_document(wanted, &oracle) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        for (point = 0; point < 4096; point++) {
            move_fault_allocator a = {-1, -1, 0, ATOMIC_VAR_INIT(0)};
            xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_snapshot old, now, restored;
            xui_doc_range_t range = {0};
            uint64_t block, parent, index, moved, revision, token;
            unsigned notifications = 0; int quality, result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = move_fault_alloc; desc.onFree = move_fault_free; desc.pAllocatorUser = &a;
            MOVE_CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, before, strlen(before)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
                xuiDocumentMarkSaved(d, old) == XUI_OK &&
                xuiDocumentSubscribe(d, move_fault_notify, &notifications, &token) == XUI_OK);
            revision = xuiDocumentGetRevision(d);
            MOVE_CHECK(xuiDocumentSourceToPosition(old, strstr(before, c->needle) - before,
                &range.tAnchor, &quality) == XUI_OK);
            range.tCaret = range.tAnchor;
            MOVE_CHECK(move_target(old, c, &range.tAnchor, &block, &parent, &index) == XUI_OK);
            if (c->root_gap) {
                range.tAnchor.iNodeId = parent; range.tAnchor.iKind = XUI_DOC_POSITION_GAP;
                range.tAnchor.iOffset = index; range.tCaret = range.tAnchor; range.tCaret.iOffset++;
            }
            result = move_fault_attempt(d, &range, block, parent, index, c->down,
                direct, cancel, point, &a);
            a.remaining = a.cancel_after = -1; atomic_store(&a.cancellation, 0);
            MOVE_CHECK(xuiDocumentAcquireSnapshot(d, &now) == XUI_OK);
            if (result != XUI_OK) {
                MOVE_CHECK(result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY) &&
                    !notifications && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) &&
                    !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && now->state == old->state &&
                    move_source_equal(now, before));
                MOVE_CHECK(move_fault_attempt(d, &range, block, parent, index, c->down,
                    direct, 0, -1, &a) == XUI_OK && move_source_equal(now, before));
                xuiDocumentSnapshotRelease(now); MOVE_CHECK(xuiDocumentAcquireSnapshot(d, &now) == XUI_OK);
            }
            MOVE_CHECK(notifications == 1 && xuiDocumentGetRevision(d) == revision + 1 &&
                xuiDocumentIsDirty(d) && xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) &&
                move_source_equal(now, wanted) && move_source_equal(old, before) &&
                xuiDocumentSnapshotGetChild(now, parent, c->down ? index + 1 : index - 1, &moved) == XUI_OK && moved == block);
            move_fault_html(now, expected);
            MOVE_CHECK(move_blobs_shared(now->state->source, old->state->source) &&
                now->state->source_storage_bytes == old->state->source_storage_bytes &&
                now->state->source_open_brackets == old->state->source_open_brackets);
            MOVE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && notifications == 2 &&
                !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentCanRedo(d) &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK && move_source_equal(restored, before) &&
                xuiDocumentSnapshotGetChild(restored, parent, index, &moved) == XUI_OK && moved == block);
            xuiDocumentSnapshotRelease(restored);
            MOVE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && notifications == 3 &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK && move_source_equal(restored, wanted) &&
                move_source_equal(old, before));
            xuiDocumentSnapshotRelease(restored); xuiDocumentSnapshotRelease(now);
            xuiDocumentUnsubscribe(d, token); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d);
            MOVE_CHECK(!a.live);
            if (result == XUI_OK) break;
        }
        MOVE_CHECK(point > 0 && point < 4096); scans++; failed_points += (uint64_t)point;
        printf("Move fault fixture=%u direct=%u cancel=%u: %ld points; atomic state/notifications/poisoned commit/retry/StableId/history/shared source/zero leaks passed\n",
            fixture, direct, cancel, point); fflush(stdout);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle); free(before); free(wanted);
    }
    MOVE_CHECK(scans == 56);
    printf("Move faults: %u exhaustive OOM/cancel scans; %llu unpublished failure points; all invariants passed\n",
        scans, (unsigned long long)failed_points);
    return 0;
}
