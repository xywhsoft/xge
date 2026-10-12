#define XUI_TAB_PREFIX_NO_MAIN
#include "xui_document_tab_prefix_test.c"

#ifndef XUI_DLL
static int source_has_blob(doc_sequence* source, doc_blob* blob)
{
    return source && (source->blob == blob || source_has_blob(source->left, blob) || source_has_blob(source->right, blob));
}
static uint64_t source_shared_bytes(doc_sequence* before, doc_sequence* after)
{
    if (!after) return 0;
    return (source_has_blob(before, after->blob) ? after->length : 0) +
        source_shared_bytes(before, after->left) + source_shared_bytes(before, after->right);
}
#endif
/* The operation list is observable API, independent of private source plans.
 * A quote prefix may replace a single boundary Tab, never a retained body.
 * Check byte-position mapping against an independent descending-patch oracle. */
static uint64_t source_patch_proof(const char* original, const char* output,
    xui_document_snapshot before, xui_document_change_set change)
{
    xui_doc_change_info_t info = {0}; uint64_t i, added = 0, removed = 0, last = UINT64_MAX, count = 0, at;
    info.iSize = sizeof(info); CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
    for (i = 0; i < info.iOperationCount; i++) {
        const xui_doc_operation_t* op = &info.pOperations[i];
        if (op->iKind != XUI_DOC_OP_SOURCE) continue;
        if (op->iOldLength > 1 || op->iNewLength > 32)
            fprintf(stderr, "Source rewrite still replaces body: offset=%llu old=%llu new=%llu\n",
                (unsigned long long)op->iOffset, (unsigned long long)op->iOldLength, (unsigned long long)op->iNewLength);
        CHECK(op->iOffset <= last && op->iOldLength <= 1 && op->iNewLength <= 32 &&
            (!op->iOldLength || original[op->iOffset] == '\t'));
        last = op->iOffset; added += op->iNewLength; removed += op->iOldLength; count++;
    }
    CHECK(count && strlen(original) - removed + added == strlen(output));
    for (at = 0; at <= strlen(original); at++) {
        unsigned affinity;
        if (((unsigned char)original[at] & 0xc0) == 0x80) continue;
        for (affinity = 0; affinity < 2; affinity++) {
            xui_doc_position_t position = {0}, mapped; uint64_t expected = at; int mapping;
            position.iSize = sizeof(position); position.iKind = XUI_DOC_POSITION_SOURCE;
            position.iNodeId = 1; position.iDocumentId = xuiDocumentSnapshotGetIdentity(before);
            position.iRevision = xuiDocumentSnapshotGetRevision(before); position.iOffset = at; position.iAffinity = affinity;
            for (i = 0; i < info.iOperationCount; i++) {
                const xui_doc_operation_t* op = &info.pOperations[i];
                if (op->iKind != XUI_DOC_OP_SOURCE) continue;
                if (expected < op->iOffset) continue;
                if (expected == op->iOffset && affinity == XUI_DOC_BEFORE) continue;
                if (expected >= op->iOffset + op->iOldLength) expected = expected - op->iOldLength + op->iNewLength;
                else expected = op->iOffset + (affinity == XUI_DOC_AFTER ? op->iNewLength : 0);
            }
            CHECK(xuiDocumentMapPosition(change, &position, &mapped, &mapping) == XUI_OK &&
                mapped.iOffset == expected && mapped.iRevision == info.iAfterRevision);
        }
    }
    return removed;
}
static void source_sharing_matrix(void)
{
    unsigned ci, reverse, gaps, ending, total = 0;
    for (ci = 0; ci < sizeof(tab_cases) / sizeof(tab_cases[0]); ci++)
    for (reverse = 0; reverse < 2; reverse++)
    for (gaps = 0; gaps < 4; gaps++)
    for (ending = 0; ending < 3; ending++) {
        char source[8192], expected[8192], actual[8192]; xui_document d, reload;
        xui_document_snapshot before, after, full; xui_document_transaction t; xui_document_change_set change;
        xui_doc_range_t range, selected; uint64_t quote, removed;
        prefix_variant(tab_cases[ci].source, ending, source, sizeof(source));
        tab_expected_variant(ci, ending, expected, sizeof(expected));
        d = open_md(source); range = prefix_selection(d, reverse, gaps);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK &&
            xuiDocumentTxnCommit(t, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        removed = source_patch_proof(source, actual, before, change);
#ifndef XUI_DLL
        CHECK(source_shared_bytes(before->state->source, after->state->source) == strlen(source) - removed &&
            before->state->reference_values == after->state->reference_values);
#else
        (void)removed;
#endif
        reload = open_md(actual); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full);
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload); xuiDocumentChangeSetRelease(change);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        xuiDocumentRelease(d);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(before, after);
#endif
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); total++;
    }
    printf("Source sharing: %u literal goldens, sparse public patches, both-affinity all-byte maps, shared source/definitions, Undo/Redo and full reload passed\n", total);
}
/* Long hidden bodies have no visible semantic leaf; a tiny prefix edit must
 * not retain another body-sized allocation in current/history storage. */
static void source_sharing_scale(void)
{
    const size_t sizes[] = {4096, 131072, 1048576}; unsigned si, variant;
    for (si = 0; si < sizeof(sizes) / sizeof(sizes[0]); si++)
    for (variant = 0; variant < 3; variant++) {
        const char* head = variant == 0 ? "take\n\n[^unused]: " : variant == 1 ?
            "> take\n>\n> [^unused]: " : "- before\n\n  take\n\n  [^unused]: ";
        const char* tail = variant == 0 ? "\n\nalso\n" : variant == 1 ? "\n>\n> also\n" : "\n\n  also\n\n  after\n";
        size_t size = strlen(head) + sizes[si] + strlen(tail); char* source = malloc(size + 1), *actual = malloc(size + 128);
        xui_document d, reload; xui_document_snapshot before, after, full; xui_document_transaction t;
        xui_doc_range_t range, selected; xui_doc_memory_stats_t memory = {0}; uint64_t quote;
        CHECK(source && actual); strcpy(source, head); memset(source + strlen(head), 'x', sizes[si]); strcpy(source + strlen(head) + sizes[si], tail);
        d = open_md(source); range = prefix_selection(d, 0, 0);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, size + 128);
        memory.iSize = sizeof(memory); CHECK(xuiDocumentGetMemoryStats(d, &memory) == XUI_OK);
        printf("Source sharing scale: body=%zu variant=%u history=%llu snapshot-extra=%llu\n", sizes[si], variant,
            (unsigned long long)memory.iHistoryBytes, (unsigned long long)memory.iSnapshotAdditionalBytes);
        CHECK(memory.iHistoryBytes < 32768 && memory.iSnapshotAdditionalBytes < 32768);
#ifndef XUI_DLL
        CHECK(source_shared_bytes(before->state->source, after->state->source) == size &&
            before->state->reference_values == after->state->reference_values);
#endif
        reload = open_md(actual); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full);
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, size + 128); CHECK(!strcmp(actual, source));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); free(source); free(actual);
    }
}
static void source_sharing_many_lines(void)
{
    char source[8192], output[16384]; unsigned i, count = 80; size_t at = 0;
    xui_document d, reload; xui_document_snapshot before, after, full; xui_document_transaction t; xui_document_change_set change;
    xui_doc_range_t range, selected; uint64_t quote;
    at += (size_t)sprintf(source + at, "> take\n>\n> [^unused]: first\n");
    for (i = 0; i < count; i++) at += (size_t)sprintf(source + at, ">     continuation-%u &amp; raw\n", i);
    at += (size_t)sprintf(source + at, ">\n> also\n");
    CHECK(at < sizeof(source)); d = open_md(source); range = prefix_selection(d, 0, 0);
    CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
        xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK && xuiDocumentTxnCommit(t, &change) == XUI_OK &&
        xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
    xuiDocumentTxnRelease(t); copy_source(d, output, sizeof(output));
    CHECK(!source_patch_proof(source, output, before, change));
#ifndef XUI_DLL
    CHECK(source_shared_bytes(before->state->source, after->state->source) == at &&
        before->state->reference_values == after->state->reference_values);
#endif
    reload = open_md(output); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
    same_tree(after, 1, full, 1); same_syntax(after, full);
    xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload); xuiDocumentChangeSetRelease(change);
    xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    puts("Source plan growth: 80 continuation lines retain all original payload bytes, exact maps and ordered definitions");
}

#define CACHE_BASE "[a]: /one\n\n[^n]: body\n\n[b]: /two\n\n[a]: /shadow\n\nplain\n"
typedef struct cache_reuse_case { const char* source; unsigned count; int old[5]; } cache_reuse_case;
static const cache_reuse_case cache_cases[] = {
    {"\n" CACHE_BASE, 4, {0,1,2,3}},
    {"[a]: /one\n\n[^n]: changed\n\n[b]: /two\n\n[a]: /shadow\n\nplain\n", 4, {0,-1,2,3}},
    {"[a]: /different\n\n[^n]: body\n\n[b]: /two\n\n[a]: /shadow\n\nplain\n", 4, {-1,1,2,3}},
    {"[^a]: /one\n\n[^n]: body\n\n[b]: /two\n\n[a]: /shadow\n\nplain\n", 4, {-1,1,2,3}},
    {"[x]: /new\n\n" CACHE_BASE, 5, {-1,0,1,2,3}},
    {"[a]: /one\n\n[^n]: body\n\n[x]: /new\n\n[b]: /two\n\n[a]: /shadow\n\nplain\n", 5, {0,1,-1,2,3}},
    {CACHE_BASE "\n[^new]: next\n", 5, {0,1,2,3,-1}},
    {"[^n]: body\n\n[b]: /two\n\n[a]: /shadow\n\nplain\n", 3, {1,2,3}},
    {"[a]: /one\n\n[b]: /two\n\n[a]: /shadow\n\nplain\n", 3, {0,2,3}},
    {"[a]: /one\n\n[^n]: body\n\n[b]: /two\n\nplain\n", 3, {0,1,2}},
    {"[^n]: body\n\n[a]: /one\n\n[b]: /two\n\n[a]: /shadow\n\nplain\n", 4, {-1,-1,2,3}},
    {"[a]: /one\n\n[^n]: body\n\n[b]: /two\n\n[a]: /different\n\nplain\n", 4, {0,1,2,-1}}
};
static void cache_reuse_matrix(void)
{
    unsigned ci, mode, total = 0;
    for (ci = 0; ci < sizeof(cache_cases) / sizeof(cache_cases[0]); ci++)
    for (mode = 0; mode < 3; mode++) {
        xui_document d = open_md(CACHE_BASE), reload; xui_document_snapshot before, after, full;
        xui_document_transaction t; xui_document_prepare p = NULL, continued = NULL;
        xui_doc_txn_desc_t desc = {0}; xui_doc_source_patch_t patch = {0}; char actual[512];
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
        if (!mode) {
            CHECK(xuiDocumentBeginTransaction(d, &desc, &t) == XUI_OK &&
                xuiDocumentTxnReplaceSource(t, 0, strlen(CACHE_BASE), cache_cases[ci].source, strlen(cache_cases[ci].source)) == XUI_OK &&
                xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        } else {
            patch.iSize = sizeof(patch); patch.iStart = 0; patch.iEnd = strlen(CACHE_BASE);
            patch.sText = cache_cases[ci].source; patch.iTextBytes = strlen(cache_cases[ci].source);
            if (mode == 2) {
                xui_doc_source_patch_t first = patch; first.sText = "temporary\n"; first.iTextBytes = 10;
                CHECK(xuiDocumentPrepareSource(d, NULL, &first, 1, &p) == XUI_OK);
                patch.iEnd = 10;
                CHECK(xuiDocumentPrepareContinueSource(d, p, &patch, 1, &continued) == XUI_OK);
            } else CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK);
            CHECK(xuiDocumentPrepareRun(continued ? continued : p) == XUI_OK &&
                xuiDocumentPreparePublish(d, continued ? continued : p, NULL) == XUI_OK);
            xuiDocumentPrepareRelease(continued); xuiDocumentPrepareRelease(p);
        }
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, cache_cases[ci].source));
        reload = open_md(actual); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
        {
            unsigned i; CHECK(doc_seq_size(after->state->reference_values) == cache_cases[ci].count);
            reference_cache_snapshot_equal(after, full);
            for (i = 0; i < cache_cases[ci].count; i++) if (cache_cases[ci].old[i] >= 0)
                CHECK(doc_seq_get_value_item(after->state->reference_values, i) ==
                    doc_seq_get_value_item(before->state->reference_values, (uint64_t)cache_cases[ci].old[i]));
            if (!ci) CHECK(after->state->reference_values == before->state->reference_values);
        }
#endif
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, CACHE_BASE));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload); xuiDocumentRelease(d);
#ifndef XUI_DLL
        reference_cache_valid(before->state); reference_cache_valid(after->state);
#endif
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); total++;
    }
    printf("Definition reuse: %u full source/Prepare/continued Prepare, mixed kinds, duplicates, insert/delete/reorder and snapshots after Document release passed\n", total);
}
static void cache_reuse_definitions_only(void)
{
    const char* source = "[a]: /one\n\n[^n]: body\n\n[b]: /two\n";
    const char* edited[] = {"\n[a]: /one\n\n[^n]: body\n\n[b]: /two\n",
        "[a]: /one\n\n[x]: /new\n\n[^n]: body\n\n[b]: /two\n"};
    unsigned ci;
    for (ci = 0; ci < 2; ci++) {
        xui_document d = open_md(source); xui_document_snapshot before, after; xui_document_transaction t;
        xui_doc_txn_desc_t desc = {0}; char actual[256];
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK); desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
        CHECK(xuiDocumentBeginTransaction(d, &desc, &t) == XUI_OK &&
            xuiDocumentTxnReplaceSource(t, 0, strlen(source), edited[ci], strlen(edited[ci])) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, edited[ci]));
#ifndef XUI_DLL
        CHECK(before->state->node_count == 1 && after->state->node_count == 1);
        if (!ci) CHECK(before->state->reference_values == after->state->reference_values);
        else {
            CHECK(doc_seq_get_value_item(before->state->reference_values, 0) == doc_seq_get_value_item(after->state->reference_values, 0) &&
                doc_seq_get_value_item(before->state->reference_values, 1) == doc_seq_get_value_item(after->state->reference_values, 2) &&
                doc_seq_get_value_item(before->state->reference_values, 2) == doc_seq_get_value_item(after->state->reference_values, 3));
        }
#endif
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    }
    puts("Definition-only documents: root-only full parse retains equal values and shifted prefix/suffix blobs");
}
static void cache_reuse_failures(void)
{
    const unsigned samples[] = {1,2,3,4,5,8,10}; unsigned si; long total = 0;
    for (si = 0; si < sizeof(samples) / sizeof(samples[0]); si++) {
        unsigned ci = samples[si]; long point; int success = 0;
        for (point = 0; point < 6000 && !success; point++) {
            failing_allocator a = {-1,0}; xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t td = {0};
            xui_document d; xui_document_transaction t; xui_document_snapshot before, after; uint64_t revision; unsigned live; int result; char actual[512];
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.onAlloc = fail_alloc; desc.onFree = fail_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, CACHE_BASE, strlen(CACHE_BASE)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            revision = xuiDocumentGetRevision(d); live = a.live; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK); a.remaining = point;
            result = xuiDocumentTxnReplaceSource(t, 0, strlen(CACHE_BASE), cache_cases[ci].source, strlen(cache_cases[ci].source));
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            a.remaining = -1; xuiDocumentTxnRelease(t);
            if (result) CHECK(result == XUI_ERROR_OUT_OF_MEMORY && a.live == live && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) { success = 1; CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); }
            else { same_tree(before, 1, after, 1); same_syntax(before, after);
#ifndef XUI_DLL
                CHECK(before->state->reference_values == after->state->reference_values);
#endif
            }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, CACHE_BASE));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success); total += point - 1; printf("Definition reuse OOM sample %u: %ld points\n", ci, point - 1);
    }
    printf("Definition reuse OOM total: %ld points; source/tree/syntax/revision/history atomic and no leaks\n", total);
}
#ifndef XUI_DLL
static void cache_reuse_cancellation(void)
{
    const unsigned samples[] = {1,4,5,8}; unsigned si; long total = 0;
    for (si = 0; si < sizeof(samples) / sizeof(samples[0]); si++) {
        unsigned ci = samples[si]; long point; int success = 0;
        for (point = 0; point < 6000 && !success; point++) {
            prefix_cancel_allocator a = {{-1,0},-1,ATOMIC_VAR_INIT(0)}; xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t td = {0};
            xui_document d; xui_document_transaction t; xui_document_snapshot before, after; uint64_t revision; unsigned live; int result; char actual[512];
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.onAlloc = prefix_cancel_alloc; desc.onFree = prefix_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, CACHE_BASE, strlen(CACHE_BASE)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            revision = xuiDocumentGetRevision(d); live = a.allocation.live; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK); t->cancellation = &a.cancellation; a.remaining = point;
            result = xuiDocumentTxnReplaceSource(t, 0, strlen(CACHE_BASE), cache_cases[ci].source, strlen(cache_cases[ci].source));
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            a.remaining = -1; xuiDocumentTxnRelease(t); atomic_store(&a.cancellation, 0);
            if (result) CHECK(result == XUI_DOC_ERROR_CANCELLED && a.allocation.live == live && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) { success = 1; CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); }
            else { same_tree(before, 1, after, 1); same_syntax(before, after); CHECK(before->state->reference_values == after->state->reference_values); }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, CACHE_BASE));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.allocation.live);
        }
        CHECK(success); total += point - 1; printf("Definition reuse cancellation sample %u: %ld checkpoints\n", ci, point - 1);
    }
    printf("Definition reuse cancellation total: %ld checkpoints; unpublished state and storage unchanged\n", total);
}
static void source_plan_growth_failures(void)
{
    char source[8192], actual[16384]; unsigned i, mode; size_t at = 0;
    at += (size_t)sprintf(source + at, "> take\n>\n> [^unused]: first\n");
    for (i = 0; i < 80; i++) at += (size_t)sprintf(source + at, ">     continuation-%u &amp; raw\n", i);
    at += (size_t)sprintf(source + at, ">\n> also\n"); CHECK(at < sizeof(source));
    for (mode = 0; mode < 2; mode++) {
        long point; int success = 0;
        for (point = 0; point < 15000 && !success; point++) {
            prefix_cancel_allocator a = {{-1,0},-1,ATOMIC_VAR_INIT(0)};
            xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot before, after; xui_document_transaction t;
            xui_doc_range_t range, selected; uint64_t quote, revision; unsigned live; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.onAlloc = prefix_cancel_alloc; desc.onFree = prefix_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, at) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = prefix_selection(d, 0, 0); revision = xuiDocumentGetRevision(d); live = a.allocation.live;
            CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); t->cancellation = &a.cancellation;
            if (mode) a.remaining = point; else a.allocation.remaining = point;
            result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            a.remaining = a.allocation.remaining = -1; xuiDocumentTxnRelease(t); atomic_store(&a.cancellation, 0);
            if (result) CHECK(result == (mode ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY) &&
                a.allocation.live == live && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) { success = 1; CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); }
            else CHECK(before->state->source == after->state->source && before->state->index == after->state->index &&
                before->state->references == after->state->references && before->state->reference_values == after->state->reference_values);
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.allocation.live);
        }
        CHECK(success); printf("Source plan growth %s: %ld points; copy-vector growth, sparse patches, candidate/commit failure atomic and no leaks\n",
            mode ? "cancellation" : "OOM", point - 1);
    }
}
#endif
int main(void)
{
    if (getenv("XUI_SOURCE_SHARING_SCALE_ONLY")) { source_sharing_scale(); return 0; }
    source_sharing_matrix(); source_sharing_scale(); source_sharing_many_lines(); cache_reuse_matrix(); cache_reuse_definitions_only(); cache_reuse_failures();
#ifndef XUI_DLL
    cache_reuse_cancellation();
    source_plan_growth_failures();
#endif
    return 0;
}
