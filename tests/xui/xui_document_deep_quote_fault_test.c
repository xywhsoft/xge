#include "xui_document_deep_quote_samples.h"
#ifndef XUI_DLL
#include "src/xui/document/xui_document_internal.h"
#endif

typedef struct scaffold_allocator {
    table_allocator base;
#ifndef XUI_DLL
    long cancel_after;
    atomic_int cancellation;
#endif
} scaffold_allocator;

static void* scaffold_alloc(void* user, size_t bytes)
{
    scaffold_allocator* a = user;
#ifndef XUI_DLL
    if (!a->cancel_after) atomic_store(&a->cancellation, XUI_DOC_ERROR_CANCELLED);
    else if (a->cancel_after > 0) a->cancel_after--;
#endif
    return table_alloc(&a->base, bytes);
}

static void scaffold_free(void* user, void* pointer)
{
    scaffold_allocator* a = user;
    table_free(&a->base, pointer);
}

static int scaffold_fault_command(xui_document document, const xui_doc_range_t* range,
    unsigned command, unsigned compound, scaffold_allocator* allocation, long point,
    unsigned cancel, xui_doc_position_t* caret, xui_document_change_set* changes)
{
    xui_document_transaction transaction;
    int result = xuiDocumentBeginTransaction(document, NULL, &transaction);
    TABLE_CHECK(result == XUI_OK);
#ifndef XUI_DLL
    if (cancel) {
        transaction->cancellation = &allocation->cancellation;
        allocation->cancel_after = point;
    } else
#else
    TABLE_CHECK(!cancel);
#endif
        allocation->base.remaining = point;
    TABLE_CHECK(command == 0);
    result = xuiDocumentTxnReplaceRange(transaction, range, "", 0, caret);
    if (result == XUI_OK && compound) {
        xui_doc_range_t insert = {*caret, *caret};
        result = xuiDocumentTxnReplaceRange(transaction, &insert, "Z", 1, caret);
    }
    if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, changes);
    allocation->base.remaining = -1;
#ifndef XUI_DLL
    allocation->cancel_after = -1;
#endif
    if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(transaction, NULL) == result);
    xuiDocumentTxnRelease(transaction);
#ifndef XUI_DLL
    atomic_store(&allocation->cancellation, 0);
#endif
    return result;
}

static char* scaffold_compound_source(char* source, unsigned compound)
{
    if (!compound) return source;
    char* at = strstr(source, "le");
    size_t bytes = strlen(source), offset;
    TABLE_CHECK(at);
    offset = (size_t)(at - source) + 2;
    char* result = malloc(bytes + 2);
    TABLE_CHECK(result);
    memcpy(result, source, offset);
    result[offset] = 'Z';
    memcpy(result + offset + 1, source + offset, bytes - offset + 1);
    free(source);
    return result;
}

int main(void)
{
    unsigned scans = 0, cancellation_modes = 1;
#ifndef XUI_DLL
    cancellation_modes = 2;
#endif
    const unsigned depths[] = {63, 64, 95, 120};
    for (unsigned fixture = 0; fixture < 4; fixture++)
    for (unsigned command = 0; command < 1; command++)
    for (unsigned compound = 0; compound < 2; compound++)
    for (unsigned cancel = 0; cancel < cancellation_modes; cancel++) {
        unsigned depth = depths[fixture];
        char* raw_before = scaffold_source(depth, 1, 0);
        char* raw_after = scaffold_source(depth, 1, 1);
        char* before = delete_ending(raw_before, "\r\n", 1);
        char* after = scaffold_compound_source(delete_ending(raw_after, "\r\n", 1), compound);
        free(raw_before); free(raw_after);
        xui_document oracle = table_open(after, NULL);
        xui_document_snapshot expected;
        TABLE_CHECK(xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        (void)scaffold_position(expected, after, "le", 0, 2);
        unsigned quotes = 0, lists = 0, items = 0;
        scaffold_counts(expected, XUI_DOCUMENT_ROOT, &quotes, &lists, &items);
        TABLE_CHECK(quotes == depth && lists == 1 && items == 1);
        unsigned failures = 0;
        long point;
        for (point = 0; point < 16384; point++) {
            scaffold_allocator allocator = {0};
            allocator.base.remaining = -1;
#ifndef XUI_DLL
            allocator.cancel_after = -1;
            atomic_init(&allocator.cancellation, 0);
#endif
            xui_doc_desc_t desc = {0};
            desc.iSize = sizeof(desc);
            desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = scaffold_alloc;
            desc.onFree = scaffold_free;
            desc.pAllocatorUser = &allocator;
            xui_document document;
            xui_document_snapshot old, current, restored;
            xui_document_change_set changes = NULL;
            xui_doc_position_t caret = {0}, tail, mapped;
            xui_doc_range_t range;
            uint64_t revision, source;
            int quality, result;
            TABLE_CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK &&
                xuiDocumentLoadMarkdown(document, before, strlen(before)) == XUI_OK &&
                xuiDocumentClearHistory(document) == XUI_OK &&
                xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
                xuiDocumentMarkSaved(document, old) == XUI_OK);
            range.tAnchor = scaffold_position(old, before, "left", 0, 2);
            range.tCaret = scaffold_position(old, before, "abcd", depth, 4);
            range.tAnchor.iAffinity = XUI_DOC_BEFORE;
            range.tCaret.iAffinity = XUI_DOC_AFTER;
            revision = xuiDocumentGetRevision(document);
            TABLE_CHECK(xuiDocumentSourceToPosition(old,
                (uint64_t)(boundary_tail(before) - before) + 1, &tail, &quality) == XUI_OK &&
                quality == XUI_DOC_MAP_EXACT);
            result = scaffold_fault_command(document, &range, command, compound,
                &allocator, point, cancel, &caret, &changes);
            TABLE_CHECK(result == XUI_OK || result ==
                (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY));
            TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
            if (result != XUI_OK) {
                failures++;
                TABLE_CHECK(!changes && xuiDocumentGetRevision(document) == revision &&
                    !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document) &&
                    !xuiDocumentCanRedo(document));
                table_source(current, before); table_equivalent(current, old);
                TABLE_CHECK(scaffold_fault_command(document, &range, command, compound,
                    &allocator, -1, 0, &caret, &changes) == XUI_OK);
                table_source(current, before);
                xuiDocumentSnapshotRelease(current);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
            }
            TABLE_CHECK(xuiDocumentGetRevision(document) == revision + 1 &&
                xuiDocumentIsDirty(document) && xuiDocumentCanUndo(document) &&
                !xuiDocumentCanRedo(document));
            table_source(current, after);
            integration_equivalent(current, after);
            formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
            boundary_caret(document, current, &caret);
            TABLE_CHECK(xuiDocumentMapPosition(changes, &tail, &mapped, &quality) == XUI_OK &&
                quality == XUI_DOC_MAP_EXACT && mapped.iNodeId == tail.iNodeId &&
                mapped.iOffset == tail.iOffset &&
                xuiDocumentPositionToSource(current, &mapped, &source, &quality) == XUI_OK &&
                quality == XUI_DOC_MAP_EXACT &&
                source == (uint64_t)(boundary_tail(after) - after) + 1);
            TABLE_CHECK(xuiDocumentUndo(document, NULL) == XUI_OK &&
                !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document) &&
                xuiDocumentCanRedo(document) &&
                xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old);
            xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(document, NULL) == XUI_OK &&
                xuiDocumentAcquireSnapshot(document, &restored) == XUI_OK);
            table_source(restored, after); table_equivalent(restored, current);
            xuiDocumentSnapshotRelease(restored);
            table_source(old, before); table_source(current, after);
            xuiDocumentChangeSetRelease(changes);
            xuiDocumentSnapshotRelease(old);
            xuiDocumentSnapshotRelease(current);
            xuiDocumentRelease(document);
            TABLE_CHECK(!allocator.base.live);
            if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 16384 && failures);
        scans++;
        printf("Deep quote scaffold fault fixture=%u command=%u compound=%u cancel=%u: %ld points; atomic/retry/stable-tail/shared-history/zero-leaks passed\n",
            fixture, command, compound, cancel, point);
        fflush(stdout);
        xuiDocumentSnapshotRelease(expected);
        xuiDocumentRelease(oracle);
        free(before); free(after);
    }
    TABLE_CHECK(scans == 8 * cancellation_modes);
    printf("Deep quote scaffold faults: %u exhaustive OOM/cancel scans passed\n", scans);
    return 0;
}
