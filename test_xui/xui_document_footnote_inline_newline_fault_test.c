#include "xui_document_footnote_inline_newline_cases.h"
#ifndef XUI_DLL
#include "src/xui_document_internal.h"
#endif

typedef struct newline_allocator {
    table_allocator base;
#ifndef XUI_DLL
    long cancel_after;
    atomic_int cancellation;
#endif
} newline_allocator;

static void* newline_alloc(void* user, size_t bytes)
{
    newline_allocator* a = user;
#ifndef XUI_DLL
    if (!a->cancel_after) atomic_store(&a->cancellation, XUI_DOC_ERROR_CANCELLED);
    else if (a->cancel_after > 0) a->cancel_after--;
#endif
    return table_alloc(&a->base, bytes);
}

static void newline_free(void* user, void* pointer)
{
    newline_allocator* a = user;
    table_free(&a->base, pointer);
}

static int newline_fault_command(xui_document document, const xui_doc_range_t* range,
    unsigned command, unsigned compound, newline_allocator* allocation, long point,
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
    result = command ? xuiDocumentTxnSplitBlock(transaction, &range->tAnchor, caret) :
        xuiDocumentTxnReplaceRange(transaction, range, "\n", 1, caret);
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

static char* newline_compound_source(char* source, unsigned compound)
{
    if (!compound) return source;
    char* at = strstr(source, "cd &amp;");
    size_t bytes = strlen(source), offset;
    TABLE_CHECK(at);
    offset = (size_t)(at - source);
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
    char label76[77];
    memset(label76, 'r', 76); label76[76] = 0;
    for (unsigned fixture = 1; fixture < 5; fixture++)
    for (unsigned command = 0; command < 2; command++)
    for (unsigned compound = 0; compound < 2; compound++)
    for (unsigned cancel = 0; cancel < cancellation_modes; cancel++) {
        const struct newline_fixture* f = &newline_fixtures[fixture];
        char* raw_before = newline_label(f->before, fixture == 3 ? label76 : "脚注");
        char* raw_after = newline_label(f->after, fixture == 3 ? label76 : "脚注");
        char* before = delete_ending(raw_before, "\r\n", 1);
        char* after = newline_compound_source(delete_ending(raw_after, "\r\n", 1), compound);
        free(raw_before); free(raw_after);
        xui_document oracle = table_open(after, NULL);
        xui_document_snapshot expected;
        TABLE_CHECK(xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        newline_footnote_scope(expected, after, "cd");
        unsigned failures = 0;
        long point;
        for (point = 0; point < 8192; point++) {
            newline_allocator allocator = {0};
            allocator.base.remaining = -1;
#ifndef XUI_DLL
            allocator.cancel_after = -1;
            atomic_init(&allocator.cancellation, 0);
#endif
            xui_doc_desc_t desc = {0};
            desc.iSize = sizeof(desc);
            desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = newline_alloc;
            desc.onFree = newline_free;
            desc.pAllocatorUser = &allocator;
            xui_document document;
            xui_document_snapshot old, current, restored;
            xui_document_change_set changes = NULL;
            xui_doc_position_t caret = {0}, tail, mapped;
            xui_doc_range_t range;
            uint64_t revision, source;
            int quality, result;
            boundary_case operation = {before, after, "\n", 2, 2, 2};
            TABLE_CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK &&
                xuiDocumentLoadMarkdown(document, before, strlen(before)) == XUI_OK &&
                xuiDocumentClearHistory(document) == XUI_OK &&
                xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
                xuiDocumentMarkSaved(document, old) == XUI_OK);
            newline_footnote_scope(old, before, "abcd");
            range = boundary_selection(document, old, before, &operation, 0);
            revision = xuiDocumentGetRevision(document);
            TABLE_CHECK(xuiDocumentSourceToPosition(old,
                (uint64_t)(boundary_tail(before) - before) + 1, &tail, &quality) == XUI_OK &&
                quality == XUI_DOC_MAP_EXACT);
            result = newline_fault_command(document, &range, command, compound,
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
                TABLE_CHECK(newline_fault_command(document, &range, command, compound,
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
        TABLE_CHECK(point < 8192 && failures);
        scans++;
        printf("Footnote newline fault fixture=%u command=%u compound=%u cancel=%u: %ld points; atomic/retry/stable-tail/shared-history/zero-leaks passed\n",
            fixture, command, compound, cancel, point);
        fflush(stdout);
        xuiDocumentSnapshotRelease(expected);
        xuiDocumentRelease(oracle);
        free(before); free(after);
    }
    TABLE_CHECK(scans == 16 * cancellation_modes);
    printf("Footnote newline faults: %u exhaustive OOM/cancel scans passed\n", scans);
    return 0;
}
