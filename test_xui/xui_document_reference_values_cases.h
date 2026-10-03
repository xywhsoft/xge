#ifndef XUI_DLL
#include "../src/xui_document_md4c.h"
typedef struct reference_values_cancel_allocator {
    fail_allocator allocator;
    long remaining;
    atomic_int cancellation;
} reference_values_cancel_allocator;
static void* reference_values_cancel_alloc(void* user, size_t bytes)
{
    reference_values_cancel_allocator* allocator = user;
    if (allocator->remaining >= 0) {
        if (!allocator->remaining) atomic_store(&allocator->cancellation, XUI_DOC_ERROR_CANCELLED);
        else allocator->remaining--;
    }
    return failing_alloc(&allocator->allocator, bytes);
}
static void reference_values_cancel_free(void* user, void* pointer)
{ reference_values_cancel_allocator* allocator = user; failing_free(&allocator->allocator, pointer); }
static void markdown_reference_values_guard(void)
{
    static const struct { const char* before; const char* after; int equal; } samples[] = {
        {"- [r]: /same 'First\n  Second'\n", "> - [r]: /same 'First\n>   Second'\n", 1},
        {"- [r\n  a]: /same 'First\n  Second'\n", "> - [r\n>   a]: /same 'First\n>   Second'\n", 1},
        {"99) [r]: /same 'First\r\n    Second'\r\n", "100) [r]: /same 'First\r\n     Second'\r\n", 1},
        {"99) [r]: /same 'First\n\tSecond'\n", "100) [r]: /same 'First\n\t Second'\n", 1},
        {"[r]: /same 'First\nSecond'\n", "[r]: /same 'First\nChanged'\n", 0},
        {"[r]: /same 'First'\n", "[r]: /changed 'First'\n", 0},
        {"[r]: /same 'First'\n", "[changed]: /same 'First'\n", 0},
        {"[r]: /same\n", "[r]: /same ''\n", 0},
        {"[r]: /same ''\n", "[r]: /same\n", 0},
        {"[r]: /same ''\n", "[r]: /same ''\n", 1},
        {"[r]: /first\n[r]: /second 'Unused'\n", "[r]: /first\n[r]: /changed 'Unused'\n", 0},
        {"[r]: /first\n[r]: /second 'Unused'\n", "[r]: /first\n[r]: /second 'Changed'\n", 0},
        {"[r]: /same\n", "", 0},
        {"", "[r]: /same\n", 0},
        {"[r]: /same\n[s]: /same\n", "[s]: /same\n[r]: /same\n", 0},
        {"[r]: /same '&amp;'\n", "[r]: /same '&'\n", 0},
        {"[r]: /same\n", "[R]: /same\n", 0},
        {"ordinary\n", "other ordinary\n", 1}
    };
    fail_allocator allocation = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document document;
    unsigned sample, baseline; long point; int result, equal, succeeded = 0; atomic_int cancellation;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &allocation;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK); baseline = allocation.live;
    for (sample = 0; sample < sizeof(samples) / sizeof(*samples); sample++) {
        xui_document left, right; unsigned cached_live; uint64_t parses;
        CHECK(xuiDocumentCreate(&desc, &left) == XUI_OK && xuiDocumentCreate(&desc, &right) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(left, samples[sample].before, strlen(samples[sample].before)) == XUI_OK &&
            xuiDocumentLoadMarkdown(right, samples[sample].after, strlen(samples[sample].after)) == XUI_OK);
        reference_cache_valid(left->state); reference_cache_valid(right->state);
        cached_live = allocation.live; allocation.remaining = 0; equal = -1;
        parses = atomic_load(&left->state->allocator->markdown_parses);
        CHECK(doc_reference_link_values_equal(left->state, right->state, NULL, &equal) == XUI_OK && equal == samples[sample].equal &&
            allocation.live == cached_live && atomic_load(&left->state->allocator->markdown_parses) == parses);
        allocation.remaining = -1; xuiDocumentRelease(left); xuiDocumentRelease(right); CHECK(allocation.live == baseline);
        equal = -1;
        result = doc_md4c_reference_values_equal(document->state->allocator,
            samples[sample].before, (MD_SIZE)strlen(samples[sample].before),
            samples[sample].after, (MD_SIZE)strlen(samples[sample].after), doc_md4c_dialect_flags(XUI_MD_EXTENDED), NULL, &equal);
        if (result != XUI_OK || equal != samples[sample].equal)
            fprintf(stderr, "Reference values guard sample=%u result=%d equal=%d expected=%d\n", sample, result, equal, samples[sample].equal);
        CHECK(result == XUI_OK && equal == samples[sample].equal && allocation.live == baseline);
    }
    for (point = 0; point < 2048 && !succeeded; point++) {
        allocation.remaining = point; equal = -1;
        result = doc_md4c_reference_values_equal(document->state->allocator,
            samples[1].before, (MD_SIZE)strlen(samples[1].before), samples[1].after, (MD_SIZE)strlen(samples[1].after),
            doc_md4c_dialect_flags(XUI_MD_EXTENDED), NULL, &equal);
        allocation.remaining = -1;
        CHECK(allocation.live == baseline);
        if (result == XUI_OK) { CHECK(equal); succeeded = 1; }
        else CHECK(result == XUI_ERROR_OUT_OF_MEMORY && !equal);
    }
    CHECK(succeeded); atomic_init(&cancellation, XUI_DOC_ERROR_CANCELLED); equal = -1;
    CHECK(doc_md4c_reference_values_equal(document->state->allocator,
        samples[1].before, (MD_SIZE)strlen(samples[1].before), samples[1].after, (MD_SIZE)strlen(samples[1].after),
        doc_md4c_dialect_flags(XUI_MD_EXTENDED), &cancellation, &equal) == XUI_DOC_ERROR_CANCELLED && !equal && allocation.live == baseline);
    {
        static const char prefix[] = "[r]: /same '";
        const size_t title_bytes = 65536, prefix_bytes = sizeof(prefix) - 1, bytes = title_bytes + prefix_bytes + 2;
        char* large = malloc(bytes + 1); char* altered = malloc(bytes + 1);
        CHECK(large && altered); memcpy(large, prefix, prefix_bytes); memset(large + prefix_bytes, 'q', title_bytes);
        memcpy(large + prefix_bytes + title_bytes, "'\n", 2); large[bytes] = 0; memcpy(altered, large, bytes + 1);
        CHECK(doc_md4c_reference_values_equal(document->state->allocator, large, (MD_SIZE)bytes, altered, (MD_SIZE)bytes,
            doc_md4c_dialect_flags(XUI_MD_EXTENDED), NULL, &equal) == XUI_OK && equal && allocation.live == baseline);
        altered[prefix_bytes + title_bytes - 1] = 'z';
        CHECK(doc_md4c_reference_values_equal(document->state->allocator, large, (MD_SIZE)bytes, altered, (MD_SIZE)bytes,
            doc_md4c_dialect_flags(XUI_MD_EXTENDED), NULL, &equal) == XUI_OK && !equal && allocation.live == baseline);
        {
            xui_document left, right;
            CHECK(xuiDocumentCreate(&desc, &left) == XUI_OK && xuiDocumentCreate(&desc, &right) == XUI_OK &&
                xuiDocumentLoadMarkdown(left, large, bytes) == XUI_OK && xuiDocumentLoadMarkdown(right, altered, bytes) == XUI_OK);
            reference_cache_valid(left->state); reference_cache_valid(right->state);
            CHECK(doc_reference_link_values_equal(left->state, right->state, NULL, &equal) == XUI_OK && !equal);
            CHECK(doc_reference_link_values_equal(left->state, right->state, &cancellation, &equal) == XUI_DOC_ERROR_CANCELLED && !equal);
            xuiDocumentRelease(left); xuiDocumentRelease(right); CHECK(allocation.live == baseline);
        }
        free(large); free(altered);
    }
    xuiDocumentRelease(document); CHECK(!allocation.live);
    printf("Reference values guard: 18 normalized/raw payload pairs, duplicate/unused/order/title-presence rejection, 64 KiB payload, %ld OOM attempts and cancellation passed\n", point);
    {
        reference_values_cancel_allocator allocator = {{-1, 0}, -1, ATOMIC_VAR_INIT(0)};
        long checkpoint; succeeded = 0;
        desc.onAlloc = reference_values_cancel_alloc; desc.onFree = reference_values_cancel_free; desc.pAllocatorUser = &allocator;
        CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK); baseline = allocator.allocator.live;
        for (checkpoint = 0; checkpoint < 2048 && !succeeded; checkpoint++) {
            allocator.remaining = checkpoint; atomic_store(&allocator.cancellation, 0); equal = -1;
            result = doc_md4c_reference_values_equal(document->state->allocator,
                samples[1].before, (MD_SIZE)strlen(samples[1].before), samples[1].after, (MD_SIZE)strlen(samples[1].after),
                doc_md4c_dialect_flags(XUI_MD_EXTENDED), &allocator.cancellation, &equal);
            allocator.remaining = -1; CHECK(allocator.allocator.live == baseline);
            if (result == XUI_OK) { CHECK(equal); succeeded = 1; }
            else CHECK(result == XUI_DOC_ERROR_CANCELLED && !equal);
        }
        CHECK(succeeded); xuiDocumentRelease(document); CHECK(!allocator.allocator.live);
        printf("Reference values guard mid-parse cancellation: %ld allocator checkpoints, both parser passes and leak-free cleanup passed\n", checkpoint);
    }
}
static void reference_cache_apply(xui_document d, uint64_t offset, uint64_t removed, const char* text)
{
    xui_document_transaction t; xui_doc_txn_desc_t desc = {0};
    desc.iSize = sizeof(desc); desc.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentBeginTransaction(d, &desc, &t) == XUI_OK &&
        xuiDocumentTxnReplaceSource(t, offset, offset + removed, text, strlen(text)) == XUI_OK &&
        xuiDocumentTxnCommit(t, NULL) == XUI_OK);
    xuiDocumentTxnRelease(t);
}
static void markdown_reference_cache_persistence(void)
{
    static const char source[] = "[r]: /same 'Title'\n\n[s]: /stable\n\n[r]: /unused\n\nplain body\n\nuse [r] and [s]\n";
    static const struct { const char* needle; unsigned offset, removed; const char* text; int changed; } cases[] = {
        {"plain", 0, 5, "PLAIN", -1}, {"body", 0, 4, "long body", -1},
        {"/same", 0, 5, "/next", 0}, {"/same", 0, 5, "/longer-target", 0},
        {"Title", 0, 5, "Other", 0}, {"Title", 0, 5, "Longer title", 0},
        {"[r]:", 1, 1, "x", 0}, {"[r]:", 1, 1, "long-label", 0},
        {"/unused", 0, 7, "/unused-changed", 2}
    };
    unsigned sample;
    for (sample = 0; sample < sizeof(cases) / sizeof(*cases); sample++) {
        xui_document d = test_markdown_open(source), oracle; xui_document_snapshot before, after, loaded, restored;
        xui_doc_stats_t first = inc_stats(d), last; doc_sequence* saved;
        uint64_t at = (uint64_t)(strstr(source, cases[sample].needle) - source) + cases[sample].offset, i;
        char edited[256]; size_t added = strlen(cases[sample].text); int equal;
        memcpy(edited, source, (size_t)at); memcpy(edited + at, cases[sample].text, added);
        strcpy(edited + at + added, source + at + cases[sample].removed);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK); saved = before->state->reference_values;
        reference_cache_apply(d, at, cases[sample].removed, cases[sample].text); last = inc_stats(d);
        if (last.iMarkdownIncrementalParses != first.iMarkdownIncrementalParses + 1)
            fprintf(stderr, "Reference cache persistence sample=%u incremental=%llu before=%llu\n", sample,
                (unsigned long long)last.iMarkdownIncrementalParses, (unsigned long long)first.iMarkdownIncrementalParses);
        CHECK(last.iMarkdownIncrementalParses == first.iMarkdownIncrementalParses + 1 && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        CHECK((saved == after->state->reference_values) == (cases[sample].changed < 0));
        for (i = 0; i < 3; i++) CHECK((doc_seq_get_blob_item(saved, i) == doc_seq_get_blob_item(after->state->reference_values, i)) ==
            ((int)i != cases[sample].changed));
        oracle = test_markdown_open(edited); CHECK(xuiDocumentAcquireSnapshot(oracle, &loaded) == XUI_OK);
        inc_snapshot_equal(after, loaded); xuiDocumentSnapshotRelease(loaded); xuiDocumentRelease(oracle);
        CHECK(doc_reference_link_values_equal(before->state, after->state, NULL, &equal) == XUI_OK && equal == (cases[sample].changed < 0));
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        reference_cache_snapshot_equal(before, restored); CHECK(restored->state->reference_values == saved); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        reference_cache_snapshot_equal(after, restored); CHECK(restored->state->reference_values == after->state->reference_values); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK);
        if (cases[sample].changed >= 0) {
            doc_blob* old_blob = doc_seq_get_blob_item(saved, (uint64_t)cases[sample].changed);
            doc_allocation* owner = (doc_allocation*)old_blob - 1;
            xui_doc_memory_stats_t memory = {0}; memory.iSize = sizeof(memory);
            /* Snapshot reachability marks exist only while diagnostics run. */
            CHECK(!owner->value.owners[0] && !owner->value.owners[1] && !owner->value.owners[2] &&
                xuiDocumentGetMemoryStats(d, &memory) == XUI_OK && memory.iSnapshotAdditionalBytes >= owner->value.bytes &&
                !owner->value.owners[2]);
        }
        xuiDocumentRelease(d); reference_cache_snapshot_equal(before, before); reference_cache_snapshot_equal(after, after);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after);
    }
    {
        const char* footnote = "plain body\n\n[^r]: note body\n\nuse [^r]\n";
        xui_document d = test_markdown_open(footnote), oracle; xui_document_snapshot before, after, loaded;
        uint64_t at = (uint64_t)(strstr(footnote, "note body") - footnote);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        reference_cache_apply(d, at, 9, "longer note body");
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK && before->state->reference_values == after->state->reference_values);
        oracle = test_markdown_open("plain body\n\n[^r]: longer note body\n\nuse [^r]\n");
        CHECK(xuiDocumentAcquireSnapshot(oracle, &loaded) == XUI_OK); inc_snapshot_equal(after, loaded);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentSnapshotRelease(loaded);
        xuiDocumentRelease(d); xuiDocumentRelease(oracle);
    }
    puts("Reference cache persistence: 9 body/offset/label/destination/title/unused edits, field-only blob replacement, full-load oracle, snapshot/undo/redo/ownership and footnote placeholder passed");
}
#else
static void markdown_reference_values_guard(void) {}
static void markdown_reference_cache_persistence(void) {}
#endif
