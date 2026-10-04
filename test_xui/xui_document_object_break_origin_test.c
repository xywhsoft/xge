#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main

typedef struct alt_case { const char *first, *second; unsigned text_offset, marker_bytes; } alt_case;
static const alt_case alt_cases[] = {
    {"![*word*", "next](asset.png)", 4, 0},
    {"![`word`", "next](asset.png)", 4, 0},
    {"![_word_\\", "next](asset.png)", 4, 1},
    {"![", "body](asset.png)", 0, 0},
    {"![word  ", "next](asset.png)", 4, 2},
    {"[![*word*", "next](asset.png)](link)", 4, 0},
    {"> ![*word*", "> next](asset.png)", 4, 0},
    {"- ![*word*", "  next](asset.png)", 4, 0}
};
static void alt_golden(xui_document_snapshot s, const char* source, unsigned fixture, uint64_t newline, unsigned ending)
{
    const alt_case* c = &alt_cases[fixture]; xui_doc_node_info_t node = {0};
    xui_doc_source_segment_t segment = {0}; uint64_t i, id = find_kind(s, 1, XUI_DOC_IMAGE);
    int matched = 0;
    node.iSize = sizeof(node); CHECK(id && xuiDocumentSnapshotGetNode(s, id, &node) == XUI_OK);
    expect_text(s, id, c->text_offset ? "word\nnext" : "\nbody");
    for (i = 0; i < node.iSourceSegmentCount; i++) {
        segment.iSize = sizeof(segment);
        CHECK(xuiDocumentSnapshotGetSourceSegment(s, id, i, &segment) == XUI_OK);
        if (segment.iTextStart != c->text_offset || segment.iTextEnd != c->text_offset + 1) continue;
        CHECK(segment.iKind == XUI_DOC_SOURCE_NORMALIZED && segment.iSourceStart == newline - c->marker_bytes &&
            segment.iSourceEnd == newline + ending);
        CHECK((source[newline] == '\r' || source[newline] == '\n') &&
            (ending != 2 || (source[newline] == '\r' && source[newline + 1] == '\n')));
        matched = 1; break;
    }
    CHECK(matched);
    if (ending == 2) {
        expect_source_position(s, newline + 1, XUI_DOC_BEFORE, id, c->text_offset, XUI_DOC_MAP_COLLAPSED);
        expect_source_position(s, newline + 1, XUI_DOC_AFTER, id, c->text_offset + 1, XUI_DOC_MAP_COLLAPSED);
    }
}
static void alt_matrix(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"}; unsigned fixture, dialect, eol, native, mode, cases = 0;
    for (fixture = 0; fixture < sizeof(alt_cases) / sizeof(*alt_cases); fixture++)
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++)
    for (eol = 0; eol < 3; eol++) for (native = 0; native < 2; native++) for (mode = 0; mode < 4; mode++) {
        const char* prefix = native ? "\xef\xbb\xbf---\rname: alt\r\n...\n\nprior `span`\n\n" : "prior `span`\n\n";
        char source[1024], initial[1024]; uint64_t at = strlen(prefix) + strlen(alt_cases[fixture].first);
        xui_doc_desc_t desc = {0}; xui_document d, oracle; xui_document_snapshot before, after, expected, restored;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        snprintf(source, sizeof(source), "%s%s%s%s\n", prefix, alt_cases[fixture].first, endings[eol], alt_cases[fixture].second);
        memcpy(initial, source, at); strcpy(initial + at, source + at + strlen(endings[eol]));
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, mode ? initial : source,
            strlen(mode ? initial : source)) == XUI_OK);
        if (native) {
            char* wire; uint64_t bytes; xui_document loaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &wire, &bytes) == XUI_OK &&
                xuiDocumentDeserialize(NULL, wire, bytes, &loaded) == XUI_OK);
            xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = loaded;
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        if (mode == 1) {
            xui_doc_txn_desc_t td = {0}; xui_document_transaction t; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK &&
                xuiDocumentTxnReplaceSource(t, at, at, endings[eol], strlen(endings[eol])) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
            xuiDocumentTxnRelease(t);
        } else if (mode >= 2) {
            xui_doc_source_patch_t patch = prepare_patch(at, at, endings[eol]); xui_document_prepare p, next;
            CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK);
            if (mode == 3) {
                patch = prepare_patch(strlen(source), strlen(source), "\ncontinued\n");
                CHECK(xuiDocumentPrepareContinueSource(d, p, &patch, 1, &next) == XUI_OK);
                xuiDocumentPrepareRelease(p); p = next; strcat(source, "\ncontinued\n"); CHECK(xuiDocumentPrepareRun(p) == XUI_OK);
            }
            CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
        }
        test_source(d, source, 1); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        alt_golden(after, source, fixture, at, (unsigned)strlen(endings[eol]));
        CHECK(xuiDocumentCreate(&desc, &oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle, source, strlen(source)) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK); inc_snapshot_equal(after, expected);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
        if (mode) {
            CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            inc_snapshot_equal(before, restored); expect_identity_tree(before, restored, 1); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            inc_snapshot_equal(after, restored); expect_identity_tree(after, restored, 1); xuiDocumentSnapshotRelease(restored);
        }
        xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d);
        alt_golden(after, source, fixture, at, (unsigned)strlen(endings[eol])); xuiDocumentSnapshotRelease(after); cases++;
    }
    CHECK(cases == 576);
    printf("Image-alt origin golden: %u independent raw-byte/text/collapsed-position cases; three dialects/endings, native reload, SOURCE/Prepare/READY Continue, history and retained snapshots passed\n", cases);
}
static void alt_boundaries(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"}; const char* edits[] = {"", "\n", "*", "![x](u)"};
    unsigned fixture, eol, edit, cases = 0; uint64_t incremental = 0;
    for (fixture = 0; fixture < sizeof(alt_cases) / sizeof(*alt_cases); fixture++) for (eol = 0; eol < 3; eol++) {
        char source[512]; uint64_t at;
        snprintf(source, sizeof(source), "prior `span`\n\n%s%s%s\n", alt_cases[fixture].first, endings[eol], alt_cases[fixture].second);
        for (at = 0; at <= strlen(source); at++) for (edit = 0; edit < 4; edit++) {
            char result[1024]; uint64_t removed = edit ? 0 : at < strlen(source) ? 1 : 0;
            xui_doc_desc_t desc = {0}; xui_document d, oracle; xui_document_snapshot before, after, expected, restored;
            xui_document_transaction t; xui_doc_txn_desc_t td = {0}; xui_doc_stats_t stats;
            memcpy(result, source, at); strcpy(result + at, edits[edit]); strcat(result, source + at + removed);
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK &&
                xuiDocumentTxnReplaceSource(t, at, at + removed, edits[edit], strlen(edits[edit])) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
            xuiDocumentTxnRelease(t); test_source(d, result, 1); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            stats = inc_stats(d); incremental += stats.iMarkdownIncrementalParses;
            CHECK(xuiDocumentCreate(&desc, &oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle, result, strlen(result)) == XUI_OK &&
                xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK); inc_snapshot_equal(after, expected);
            xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
            if (removed || edits[edit][0]) {
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                inc_snapshot_equal(before, restored); xuiDocumentSnapshotRelease(restored);
                CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                inc_snapshot_equal(after, restored); xuiDocumentSnapshotRelease(restored);
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); cases++;
        }
    }
    printf("Image-alt origin differential: %u cases, %llu incremental; every byte, syntax/containers/link/object edits and complete full-parse/history oracle passed\n",
        cases, (unsigned long long)incremental);
}
static void alt_faults(void)
{
    const char* original[] = {"prior `span`\n\n![*word*next](asset.png)\n", "prior `span`\n\n![body](asset.png)\n"};
    const char* edited[] = {"prior `span`\n\n![*word*\r\nnext](asset.png)\n", "prior `span`\n\n![\r\nbody](asset.png)\n"};
    uint64_t start[] = {22, 16}; unsigned fixture, mode; long oom = 0, cancel = 0;
    for (fixture = 0; fixture < 2; fixture++) for (mode = 0; mode < 2; mode++) {
        long point; int success = 0;
        for (point = 0; point < 4096 && !success; point++) {
            fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot before, after;
            uint64_t revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, original[fixture], strlen(original[fixture])) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
            revision = xuiDocumentGetRevision(d); a.remaining = point;
            if (mode) {
                xui_document_prepare p = NULL; xui_doc_source_patch_t patch = prepare_patch(start[fixture], start[fixture], "\r\n");
                result = xuiDocumentPrepareSource(d, NULL, &patch, 1, &p);
                if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
                if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
                a.remaining = -1; xuiDocumentPrepareRelease(p);
            } else {
                xui_document_transaction t = NULL; xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                result = xuiDocumentBeginTransaction(d, &td, &t);
                if (result == XUI_OK) result = xuiDocumentTxnReplaceSource(t, start[fixture], start[fixture], "\r\n", 2);
                if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
                a.remaining = -1; xuiDocumentTxnRelease(t);
            }
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result != XUI_OK) {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) &&
                    !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                inc_snapshot_equal(before, after); expect_identity_tree(before, after, 1); test_source(d, original[fixture], 1); oom++;
            } else {
                test_source(d, edited[fixture], 1); alt_golden(after, edited[fixture], fixture ? 3 : 0, start[fixture], 2); success = 1;
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success);
    }
    for (fixture = 0; fixture < 2; fixture++) {
        long point; int success = 0;
        for (point = 0; point < 4096 && !success; point++) {
            prepare_cancel_allocator a = {0, -1, 0, NULL}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_prepare p; xui_document_snapshot before, after; uint64_t revision; int result;
            xui_doc_source_patch_t patch = prepare_patch(start[fixture], start[fixture], "\r\n");
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, original[fixture], strlen(original[fixture])) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
            revision = xuiDocumentGetRevision(d); CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK);
            a.prepare = p; a.remaining = point; result = xuiDocumentPrepareRun(p); a.prepare = NULL;
            if (a.fired) {
                CHECK(result == XUI_DOC_ERROR_CANCELLED && xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED &&
                    xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
                CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); inc_snapshot_equal(before, after);
                expect_identity_tree(before, after, 1); test_source(d, original[fixture], 1); xuiDocumentSnapshotRelease(after); cancel++;
            } else {
                CHECK(result == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); test_source(d, edited[fixture], 1); success = 1;
            }
            xuiDocumentPrepareRelease(p); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success);
    }
    printf("Image-alt origin faults: %ld SOURCE/Prepare OOM, %ld Prepare cancellation checkpoints; empty/format closing, state/identity/history/savepoint and zero leaks passed\n", oom, cancel);
}
int main(void) { alt_matrix(); alt_boundaries(); alt_faults(); return 0; }
