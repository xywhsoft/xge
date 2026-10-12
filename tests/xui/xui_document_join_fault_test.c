#include "xui_document_partial_entity_cases.h"
#include "xui_document_partial_cross_cases.h"
#include "xui_document_nonempty_seam_cases.h"
#include "../src/xui/document/xui_document_internal.h"
#include "xui_document_join_source_cases.h"
typedef struct join_cancel_allocator {
    table_allocator base;
    long cancel_after;
    atomic_int cancellation;
} join_cancel_allocator;
static void* join_cancel_alloc(void* user, size_t bytes)
{
    join_cancel_allocator* a = user;
    if (!a->cancel_after) atomic_store(&a->cancellation, XUI_DOC_ERROR_CANCELLED);
    else if (a->cancel_after > 0) a->cancel_after--;
    return table_alloc(&a->base, bytes);
}
static void join_cancel_free(void* user, void* p)
{ join_cancel_allocator* a = user; table_free(&a->base, p); }
static xui_document join_cancel_open(const char* before, join_cancel_allocator* a)
{
    xui_doc_desc_t desc = {0}; xui_document d;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
    desc.onAlloc = join_cancel_alloc; desc.onFree = join_cancel_free; desc.pAllocatorUser = a;
    TABLE_CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, before, strlen(before)) == XUI_OK);
    return d;
}
static int join_cancel_command(xui_document d, xui_document_snapshot old, uint64_t left, uint64_t right,
    unsigned mode, unsigned cancel, long point, join_cancel_allocator* a, xui_doc_position_t* caret)
{
    xui_document_transaction t; int result;
    if (!cancel) a->base.remaining = point;
    result = xuiDocumentBeginTransaction(d, NULL, &t); if (result != XUI_OK) return result;
    if (cancel) { t->cancellation = &a->cancellation; a->cancel_after = point; }
    if (!mode) result = xuiDocumentTxnJoinBlocks(t, left, right, caret);
    else {
        xui_doc_range_t range = {0}; xui_doc_node_info_t n = {0}; n.iSize = sizeof(n);
        TABLE_CHECK(xuiDocumentSnapshotGetNode(old, left, &n) == XUI_OK);
        range.tAnchor.iSize = sizeof(range.tAnchor); range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d);
        range.tAnchor.iRevision = xuiDocumentGetRevision(d); range.tAnchor.iNodeId = left;
        range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tAnchor.iAffinity = XUI_DOC_AFTER;
        range.tAnchor.iOffset = n.iChildCount; range.tCaret = range.tAnchor;
        range.tCaret.iNodeId = right; range.tCaret.iOffset = 0;
        result = xuiDocumentTxnReplaceRange(t, &range, "", 0, caret);
    }
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
    if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
    xuiDocumentTxnRelease(t); return result;
}
static int join_existing_faults(void)
{
    unsigned scans = 0;
    for (unsigned fixture = 0; fixture < 7; fixture++) for (unsigned cancel = 0; cancel < 2; cancel++)
    for (unsigned mode = 0; mode < 2; mode++) {
        if (!cancel && fixture != 6) continue;
        unsigned sample = fixture == 6 ? 5 : fixture;
        char *before = delete_ending(join_samples[sample].before, "\r\n", 1),
            *after = delete_ending(join_samples[sample].after, "\r\n", 1); long point;
        if (fixture == 6) {
            before[strlen(before) - 2] = 0;
            char* boundary = strstr(after, "\r\n"); TABLE_CHECK(boundary);
            memmove(boundary + 1, boundary + 2, strlen(boundary + 2) + 1); *boundary = '\n';
        }
        for (point = 0; point < 4096; point++) {
            join_cancel_allocator a = {{-1, 0}, -1, 0}; atomic_init(&a.cancellation, 0);
            xui_document d = join_cancel_open(before, &a); xui_document_snapshot old, current, restored;
            xui_doc_position_t tail, caret; uint64_t left, right, revision; int result;
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
            join_positions(old, before, &left, &right, &tail); revision = xuiDocumentGetRevision(d);
            result = join_cancel_command(d, old, left, right, mode, cancel, point, &a, &caret);
            a.base.remaining = -1; a.cancel_after = -1; atomic_store(&a.cancellation, 0);
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY) &&
                    xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(join_execute(d, old, left, right, mode, &caret, NULL) == XUI_OK); table_source(current, before);
                xuiDocumentSnapshotRelease(current); TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentIsDirty(d) && xuiDocumentCanUndo(d));
            table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
            xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d); TABLE_CHECK(!a.base.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 4096); printf("Join source faults fixture=%u cancel=%u mode=%u: %ld points atomic/poisoned-commit/retry/history/zero-leaks passed\n", fixture, cancel, mode, point); fflush(stdout);
        free(before); free(after); scans++;
    }
    TABLE_CHECK(scans == 16); puts("Join source faults: 14 cooperative-cancel scans and2 EOF OOM scans passed"); return 0;
}


static int join_continuation_attempt(xui_document d, join_cancel_allocator* a,
    unsigned cancel, unsigned compound, uint64_t left, uint64_t right,
    const xui_doc_position_t* seed, xui_doc_position_t* caret)
{
    xui_document_transaction t; xui_doc_position_t next = *seed;
    int result = xuiDocumentBeginTransaction(d, NULL, &t);
    if (result != XUI_OK) return result;
    if (cancel) t->cancellation = &a->cancellation;
    if (compound) result = xuiDocumentTxnJoinBlocks(t, left, right, &next);
    if (result == XUI_OK) {
        xui_doc_range_t selected = {next, next};
        result = xuiDocumentTxnReplaceRange(t, &selected, "Z", 1, &next);
    }
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
    if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
    xuiDocumentTxnRelease(t);
    if (result == XUI_OK) *caret = next;
    return result;
}
static void join_continuation_faults(void)
{
    unsigned scans = 0;
    for (unsigned choice = 0; choice < 7; choice++)
    for (unsigned cancel = 0; cancel < 2; cancel++) for (unsigned compound = 0; compound < 2; compound++) {
        unsigned fixture = choice < 6 ? choice : 5;
        char *before = delete_ending(join_samples[fixture].before, "\r\n", 1),
            *joined = delete_ending(join_samples[fixture].after, "\r\n", 1);
        if (choice == 6) {
            before[strlen(before) - 2] = 0;
            char* boundary = strstr(joined, "\r\n"); TABLE_CHECK(boundary);
            memmove(boundary + 1, boundary + 2, strlen(boundary + 2) + 1); *boundary = '\n';
        }
        char* junction = strstr(joined, fixture == 0 ? "__!cd" : "!cd"); TABLE_CHECK(junction);
        size_t insertion = (size_t)(junction - joined); char* continued = malloc(strlen(joined) + 2); TABLE_CHECK(continued);
        memcpy(continued, joined, insertion); continued[insertion] = 'Z'; strcpy(continued + insertion + 1, joined + insertion);
        long point;
        for (point = 0; point < 4096; point++) {
            join_cancel_allocator a = {{-1, 0}, -1, ATOMIC_VAR_INIT(0)};
            xui_document d = join_cancel_open(before, &a); xui_document_snapshot old, baseline, current, restored;
            xui_doc_position_t tail, seed = {0}, caret = {0}; uint64_t left, right, revision, at; int quality, result;
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
            join_positions(old, before, &left, &right, &tail);
            if (!compound) {
                TABLE_CHECK(join_execute(d, old, left, right, 0, &seed, NULL) == XUI_OK);
                seed.iRevision = xuiDocumentGetRevision(d);
            }
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &baseline) == XUI_OK);
            table_source(baseline, compound ? before : joined); integration_equivalent(baseline, compound ? before : joined);
            revision = xuiDocumentGetRevision(d);
            if (cancel) a.cancel_after = point; else a.base.remaining = point;
            result = join_continuation_attempt(d, &a, cancel, compound, left, right, &seed, &caret);
            a.base.remaining = -1; a.cancel_after = -1; atomic_store(&a.cancellation, 0);
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY));
                TABLE_CHECK(xuiDocumentGetRevision(d) == revision && xuiDocumentCanUndo(d) == !compound &&
                    !xuiDocumentCanRedo(d) && xuiDocumentIsDirty(d) == !compound);
                table_source(current, compound ? before : joined); table_equivalent(current, baseline);
                TABLE_CHECK(join_continuation_attempt(d, &a, 0, compound, left, right, &seed, &caret) == XUI_OK);
                table_source(current, compound ? before : joined); xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && xuiDocumentIsDirty(d));
            table_source(current, continued); integration_equivalent(current, continued); boundary_caret(d, current, &caret);
            TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == insertion + 1);
            xui_doc_node_info_t typed = {0}; typed.iSize = sizeof(typed);
            TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &typed) == XUI_OK && typed.iKind == XUI_DOC_TEXT &&
                typed.tAttributes.iMarks == (fixture == 0 ? XUI_DOC_BOLD : 0));
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, compound ? before : joined); table_equivalent(restored, baseline); xuiDocumentSnapshotRelease(restored);
            if (!compound) TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            TABLE_CHECK(!xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
            if (!compound) {
                TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK); table_source(restored, joined);
                table_equivalent(restored, baseline); xuiDocumentSnapshotRelease(restored);
                TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
            }
            TABLE_CHECK(!xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, continued); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
            table_source(old, before); table_source(baseline, compound ? before : joined); table_source(current, continued);
            xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(baseline); xuiDocumentSnapshotRelease(current);
            xuiDocumentRelease(d); TABLE_CHECK(!a.base.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 4096);
        printf("Join continuation faults fixture=%u eof=%u cancel=%u compound=%u: %ld points atomic/poisoned-commit/same-document-retry/shared-history/styles/zero-leaks passed\n",
            fixture, choice == 6, cancel, compound, point); fflush(stdout);
        free(before); free(joined); free(continued); scans++;
    }
    TABLE_CHECK(scans == 28);
    puts("Join continuation faults: 14 OOM and14 cooperative-cancel scans passed");
}

#include "xui_document_join_heading_cases.h"
static int join_heading_edge_faults(void)
{
    unsigned scans = 0;
    for (unsigned fixture = 0; fixture < sizeof(heading_edge_samples) / sizeof(*heading_edge_samples); fixture++)
    for (unsigned cancel = 0; cancel < 2; cancel++) for (unsigned mode = 0; mode < 2; mode++) {
        char *before, *after; long point;
        heading_edge_sources(fixture, "\r\n", "", 1, &before, &after);
        for (point = 0; point < 4096; point++) {
            join_cancel_allocator a = {{-1, 0}, -1, ATOMIC_VAR_INIT(0)};
            xui_document d = join_cancel_open(before, &a); xui_document_snapshot old, current, restored;
            xui_doc_position_t tail, caret; uint64_t left, right, revision; int result;
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
            join_positions(old, before, &left, &right, &tail); revision = xuiDocumentGetRevision(d);
            result = join_cancel_command(d, old, left, right, mode, cancel, point, &a, &caret);
            a.base.remaining = -1; a.cancel_after = -1; atomic_store(&a.cancellation, 0);
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY) &&
                    xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(join_execute(d, old, left, right, mode, &caret, NULL) == XUI_OK); table_source(current, before);
                xuiDocumentSnapshotRelease(current); TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentIsDirty(d) && xuiDocumentCanUndo(d));
            table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
            xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentRelease(d); TABLE_CHECK(!a.base.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 4096);
        printf("Heading edge fault fixture=%u cancel=%u mode=%u: %ld points atomic/poisoned-commit/retry/old-snapshot/history/zero-leaks passed\n", fixture, cancel, mode, point);
        fflush(stdout); free(before); free(after); scans++;
    }
    TABLE_CHECK(scans == 48); puts("Heading edge faults: 24 OOM and24 cooperative-cancel scans passed"); return 0;
}

typedef struct seam_fault_allocator {
    table_allocator base;
    long cancel_after;
    atomic_int cancellation;
} seam_fault_allocator;
static void* seam_fault_alloc(void* user, size_t bytes)
{
    seam_fault_allocator* a = user;
    if (!a->cancel_after) atomic_store(&a->cancellation, XUI_DOC_ERROR_CANCELLED);
    else if (a->cancel_after > 0) a->cancel_after--;
    return table_alloc(&a->base, bytes);
}
static void seam_fault_free(void* user, void* p)
{ seam_fault_allocator* a = user; table_free(&a->base, p); }
static xui_document seam_fault_open(const char* source, seam_fault_allocator* a)
{
    xui_doc_desc_t desc = {0}; xui_document d;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = XUI_MD_EXTENDED; desc.onAlloc = seam_fault_alloc;
    desc.onFree = seam_fault_free; desc.pAllocatorUser = a;
    TABLE_CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
    return d;
}
static int seam_fault_attempt(xui_document d, const xui_doc_range_t* range,
    const char* text, unsigned cancel, long point, seam_fault_allocator* a,
    xui_doc_position_t* caret, xui_document_change_set* change)
{
    xui_document_transaction t = NULL; int result;
    if (!cancel) a->base.remaining = point;
    result = xuiDocumentBeginTransaction(d, NULL, &t);
    if (result != XUI_OK) return result;
    if (cancel) { t->cancellation = &a->cancellation; a->cancel_after = point; }
    result = xuiDocumentTxnReplaceRange(t, range, text, strlen(text), caret);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, change);
    if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
    xuiDocumentTxnRelease(t); return result;
}
static int nonempty_private_faults(void)
{
    unsigned scans = 0;
    for (unsigned fixture = 0; fixture < 24; fixture++) for (unsigned cancel = 0; cancel < 2; cancel++)
    for (unsigned endpoint = 0; endpoint < 2; endpoint++) {
        unsigned mode = endpoint ? 3 : 0, marks; const char* text;
        char *before, *after; uint64_t caret_at; long point;
        seam_fault_source(fixture, &before, &after, &text, &caret_at, &marks);
        for (point = 0; point < 4096; point++) {
            seam_fault_allocator a = {{-1, 0}, -1, ATOMIC_VAR_INIT(0)};
            xui_document d = seam_fault_open(before, &a); xui_document_snapshot old, current, restored;
            xui_doc_position_t tail, caret, mapped; xui_document_change_set change = NULL;
            xui_doc_node_info_t typed = {0}; uint64_t left, right, revision, token, at;
            unsigned notifications = 0; int result, quality;
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
                xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, seam_fault_changed, &notifications, &token) == XUI_OK);
            join_positions(old, before, &left, &right, &tail); revision = xuiDocumentGetRevision(d);
            xui_doc_range_t range = seam_fault_range(d, old, left, right, mode);
            result = seam_fault_attempt(d, &range, text, cancel, point, &a, &caret, &change);
            a.base.remaining = -1; a.cancel_after = -1; atomic_store(&a.cancellation, 0);
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY) && !change &&
                    xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) &&
                    !xuiDocumentCanRedo(d) && !notifications);
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(seam_fault_attempt(d, &range, text, 0, -1, &a, &caret, &change) == XUI_OK);
                table_source(current, before); xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentIsDirty(d) &&
                xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && notifications == 1 && change);
            table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
            typed.iSize = sizeof(typed);
            TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &typed) == XUI_OK && typed.tAttributes.iMarks == marks);
            TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == caret_at);
            TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
                mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset &&
                xuiDocumentPositionToSource(current, &mapped, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
                at == (uint64_t)(strstr(after, "!cd") - after) + 1);
            xuiDocumentChangeSetRelease(change);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && notifications == 2 && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && notifications == 3 && !xuiDocumentCanRedo(d) &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
            xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentUnsubscribe(d, token);
            xuiDocumentRelease(d); TABLE_CHECK(!a.base.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 4096);
        printf("Nonempty seam fault fixture=%u cancel=%u mode=%u: %ld points atomic/no-ghost-notifications/poisoned-commit/same-document-retry/source/caret/stable-tail/history/zero-leaks passed\n", fixture, cancel, mode, point);
        fflush(stdout); free(before); free(after); scans++;
    }
    TABLE_CHECK(scans == 96); puts("Nonempty seam faults: 48 OOM and48 cooperative-cancel scans passed"); return 0;
}


typedef struct partial_fault_allocator {
    table_allocator base;
    long cancel_after;
    atomic_int cancellation;
} partial_fault_allocator;
static void* partial_fault_alloc(void* user, size_t bytes)
{
    partial_fault_allocator* a = user;
    if (!a->cancel_after) atomic_store(&a->cancellation, XUI_DOC_ERROR_CANCELLED);
    else if (a->cancel_after > 0) a->cancel_after--;
    return table_alloc(&a->base, bytes);
}
static void partial_fault_free(void* user, void* p)
{ partial_fault_allocator* a = user; table_free(&a->base, p); }
static void partial_fault_changed(xui_document d, xui_document_change_set change, void* user)
{ (void)d; TABLE_CHECK(change); (*(unsigned*)user)++; }
static int partial_fault_attempt(xui_document d, const xui_doc_range_t* range, const char* text,
    partial_fault_allocator* a, long point, xui_doc_position_t* caret, xui_document_change_set* change)
{
    xui_document_transaction t; int result = xuiDocumentBeginTransaction(d, NULL, &t);
    if (result != XUI_OK) return result;
    t->cancellation = &a->cancellation; a->cancel_after = point;
    result = xuiDocumentTxnReplaceRange(t, range, text, strlen(text), caret);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, change);
    if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
    xuiDocumentTxnRelease(t); return result;
}
static int partial_private_faults(void)
{
    const char* payloads[] = {"Z*[]", "Z``", ""}; unsigned scans = 0;
    for (unsigned fixture = 0; fixture < 15; fixture++) for (unsigned payload = 0; payload < 3; payload++)
    for (unsigned reverse = 0; reverse < 2; reverse++) {
        char *before, *after; long point;
        partial_cross_sources(fixture, payload, "\r\n", "", 1, &before, &after);
        for (point = 0; point < 4096; point++) {
            partial_fault_allocator a = {0}; a.base.remaining = -1;
            a.cancel_after = -1; atomic_init(&a.cancellation, 0);
            xui_doc_desc_t desc = {0}; xui_document d;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = partial_fault_alloc; desc.onFree = partial_fault_free; desc.pAllocatorUser = &a;
            TABLE_CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, before, strlen(before)) == XUI_OK);
            xui_document_snapshot old, current, restored; xui_document_change_set change = NULL;
            xui_doc_position_t tail, mapped, caret; xui_doc_node_info_t typed = {0};
            uint64_t revision, token, at; unsigned notifications = 0; int quality, result;
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
                xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, partial_fault_changed, &notifications, &token) == XUI_OK);
            xui_doc_range_t range = partial_cross_range(old, before, reverse, &tail);
            revision = xuiDocumentGetRevision(d);
            result = partial_fault_attempt(d, &range, payloads[payload], &a, point, &caret, &change);
            a.base.remaining = -1;
            a.cancel_after = -1; atomic_store(&a.cancellation, 0);
            const int wanted_error = XUI_DOC_ERROR_CANCELLED;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == wanted_error && !change && xuiDocumentGetRevision(d) == revision &&
                    !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && !notifications);
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(partial_fault_attempt(d, &range, payloads[payload], &a, -1, &caret, &change) == XUI_OK);
                table_source(current, before); xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(change && xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentIsDirty(d) &&
                xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && notifications == 1);
            table_source(current, after); integration_equivalent(current, after); boundary_caret(d, current, &caret);
            typed.iSize = sizeof(typed);
            TABLE_CHECK(xuiDocumentSnapshotGetNode(current, caret.iNodeId, &typed) == XUI_OK && typed.tAttributes.iMarks == partial_samples[fixture].marks);
            const char* raw_payload = payload == 2 ? "" : (typed.tAttributes.iMarks & XUI_DOC_CODE) ? payloads[payload] : payload ? "Z\\`\\`" : "Z\\*\\[\\]";
            char* word = strstr(after, payload == 2 ? "ke" : "keZ");
            uint64_t caret_at;
            if (payload == 2 && fixture == 6) {
                word = strstr(after, "&#101;"); TABLE_CHECK(word); caret_at = (uint64_t)(word - after) + strlen("&#101;");
            } else { TABLE_CHECK(word); caret_at = (uint64_t)(word - after) + 2 + strlen(raw_payload); }
            TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT && at == caret_at);
            TABLE_CHECK(xuiDocumentMapPosition(change, &tail, &mapped, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
                mapped.iNodeId == tail.iNodeId && mapped.iOffset == tail.iOffset - 2 &&
                xuiDocumentPositionToSource(current, &mapped, &at, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT &&
                at == (uint64_t)(strstr(after, "!cd") - after) + 1);
            xuiDocumentChangeSetRelease(change);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && notifications == 2 && !xuiDocumentIsDirty(d) &&
                !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && notifications == 3 && !xuiDocumentCanRedo(d) &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
            table_source(old, before); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current);
            xuiDocumentUnsubscribe(d, token); xuiDocumentRelease(d); TABLE_CHECK(!a.base.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point > 0 && point < 4096);
        printf("Partial cross fault fixture=%u payload=%u reverse=%u: %ld points atomic/no-ghost-notifications/poisoned-commit/same-document-retry/source/precise-caret/stable-tail/history/zero-leaks passed\n", fixture, payload, reverse, point);
        fflush(stdout); free(before); free(after); scans++;
    }
    TABLE_CHECK(scans == 90);
    puts("Partial cross cancellation: 90 cooperative-cancel scans passed");
    return 0;
}

typedef struct entity_fault_allocator {
    table_allocator base;
    long cancel_after; atomic_int cancellation;
} entity_fault_allocator;
static void* entity_fault_alloc(void* user, size_t bytes)
{
    entity_fault_allocator* a = user;
    if (!a->cancel_after) atomic_store(&a->cancellation, XUI_DOC_ERROR_CANCELLED);
    else if (a->cancel_after > 0) a->cancel_after--;
    return table_alloc(&a->base, bytes);
}
static void entity_fault_free(void* user, void* p)
{ entity_fault_allocator* a = user; table_free(&a->base, p); }
static void entity_fault_changed(xui_document d, xui_document_change_set change, void* user)
{ (void)d; TABLE_CHECK(change); (*(unsigned*)user)++; }
static int entity_fault_attempt(xui_document d, const xui_doc_range_t* range, const char* text,
    entity_fault_allocator* a, long point, xui_doc_position_t* caret, xui_document_change_set* change)
{
    xui_document_transaction t; int result = xuiDocumentBeginTransaction(d, NULL, &t);
    if (result != XUI_OK) return result;
    t->cancellation = &a->cancellation; a->cancel_after = point;
    result = xuiDocumentTxnReplaceRange(t, range, text, strlen(text), caret);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, change);
    if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
    xuiDocumentTxnRelease(t); return result;
}
static int entity_private_faults(void)
{
    const char* texts[] = {"Z*[]", "Z``", ""}; unsigned scans = 0;
    for (unsigned fixture = 0; fixture < 9; fixture++) for (unsigned payload = 0; payload < 3; payload++) for (unsigned reverse = 0; reverse < 2; reverse++) {
        struct entity_sample sample; char *before, *after; long point;
        entity_sources(fixture, payload, "\r\n", "", 1, &sample, &before, &after);
        for (point = 0; point < 4096; point++) {
            entity_fault_allocator a = {0}; a.base.remaining = -1;
            a.cancel_after = -1; atomic_init(&a.cancellation, 0);
            xui_doc_desc_t desc = {0}; xui_document d;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = entity_fault_alloc; desc.onFree = entity_fault_free; desc.pAllocatorUser = &a;
            TABLE_CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, before, strlen(before)) == XUI_OK);
            xui_document_snapshot old, current, restored; xui_document_change_set change = NULL; xui_doc_position_t tail, caret;
            uint64_t revision, token; unsigned notifications = 0; int result;
            TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
                xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, entity_fault_changed, &notifications, &token) == XUI_OK);
            xui_doc_range_t range = entity_range(d, old, &sample, reverse, &tail); revision = xuiDocumentGetRevision(d);
            result = entity_fault_attempt(d, &range, texts[payload], &a, point, &caret, &change); a.base.remaining = -1;
            a.cancel_after = -1; atomic_store(&a.cancellation, 0); const int wanted_error = XUI_DOC_ERROR_CANCELLED;
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                TABLE_CHECK(result == wanted_error && !change && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) &&
                    !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && !notifications);
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(entity_fault_attempt(d, &range, texts[payload], &a, -1, &caret, &change) == XUI_OK);
                table_source(current, before); xuiDocumentSnapshotRelease(current); TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(change && xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentIsDirty(d) && xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d) && notifications == 1);
            entity_verify(d, current, &sample, after, payload, &range, reverse, &caret, &tail, change); xuiDocumentChangeSetRelease(change);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && notifications == 2 && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && notifications == 3 && !xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, after); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored); table_source(old, before);
            xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current); xuiDocumentUnsubscribe(d, token); xuiDocumentRelease(d); TABLE_CHECK(!a.base.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point > 0 && point < 4096); printf("Partial entity fault fixture=%u payload=%u reverse=%u: %ld points atomic/poisoned-commit/no-ghost-notifications/same-document-retry/source/exact-caret/stable-tail/history/zero-leaks passed\n", fixture, payload, reverse, point);
        fflush(stdout); free(before); free(after); scans++;
    }
    TABLE_CHECK(scans == 54);
    puts("Partial entity cancellation: 54 cooperative-cancel scans passed");
    return 0;
}

int main(void) { int result = join_existing_faults(); if (result) return result; join_continuation_faults(); TABLE_CHECK(!join_heading_edge_faults()); TABLE_CHECK(!nonempty_private_faults()); TABLE_CHECK(!partial_private_faults()); return entity_private_faults(); }
