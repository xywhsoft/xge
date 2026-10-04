#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main

static void fm_golden(xui_document_snapshot s, const char* source, uint64_t opening,
    uint64_t content, uint64_t close_start, uint64_t close_end, const char* body)
{
    uint64_t first; xui_doc_node_info_t info = {0}; xui_doc_source_segment_t segment = {0};
    xui_doc_block_syntax_t syntax = {0}; xui_doc_source_line_t raw = {0};
    xui_doc_position_t position; int mapping;
    CHECK(xuiDocumentSnapshotGetChild(s, 1, 0, &first) == XUI_OK);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(s, first, &info) == XUI_OK && info.iKind == XUI_DOC_FRONT_MATTER);
    CHECK(info.iSourceStart == content && info.iSourceEnd == close_start && info.bSourceExact &&
        info.iSyntaxStart == opening && info.iSyntaxEnd == close_end && info.iSourceSegmentCount == 1);
    CHECK(info.iTextBytes == strlen(body)); expect_text(s, first, body);
    syntax.iSize = sizeof(syntax);
    CHECK(xuiDocumentSnapshotGetBlockSyntax(s, first, &syntax) == XUI_OK &&
        syntax.iKind == XUI_DOC_BLOCK_SYNTAX_FRONT_MATTER && syntax.iPrimaryStart == opening &&
        syntax.iPrimaryEnd == opening + 3 && syntax.iSecondaryStart == close_start && syntax.iSecondaryEnd == close_start + 3 &&
        syntax.iFenceTailStart == UINT64_MAX && syntax.iFenceTailEnd == UINT64_MAX &&
        syntax.iFenceInfoStart == UINT64_MAX && syntax.iHeadingContentStart == UINT64_MAX);
    CHECK(!memcmp(source + syntax.iPrimaryStart, "---", 3) &&
        (!memcmp(source + syntax.iSecondaryStart, "---", 3) || !memcmp(source + syntax.iSecondaryStart, "...", 3)));
    raw.iSize = sizeof(raw);
    CHECK(xuiDocumentSnapshotGetSourceLine(s, opening, &raw) == XUI_OK && raw.iLineStart == 0 &&
        raw.iContentEnd == opening + 3 && raw.iLineEnd == content);
    CHECK(xuiDocumentSnapshotGetSourceLine(s, close_start, &raw) == XUI_OK && raw.iLineStart == close_start &&
        raw.iContentEnd == close_start + 3 && raw.iLineEnd == close_end);
    CHECK(xuiDocumentSourceToPositionEx(s, opening + 1, XUI_DOC_AFTER, &position, &mapping) == XUI_OK &&
        position.iNodeId == first && position.iKind == XUI_DOC_POSITION_TEXT && position.iOffset == 0 && mapping == XUI_DOC_MAP_SYNTAX);
    CHECK(xuiDocumentSourceToPositionEx(s, close_start + 1, XUI_DOC_BEFORE, &position, &mapping) == XUI_OK &&
        position.iNodeId == first && position.iKind == XUI_DOC_POSITION_TEXT && position.iOffset == strlen(body) && mapping == XUI_DOC_MAP_SYNTAX);
    segment.iSize = sizeof(segment);
    CHECK(xuiDocumentSnapshotGetSourceSegment(s, first, 0, &segment) == XUI_OK &&
        segment.iTextStart == 0 && segment.iTextEnd == strlen(body) && segment.iSourceStart == content &&
        segment.iSourceEnd == close_start && segment.iKind == XUI_DOC_SOURCE_DIRECT);
    CHECK(!memcmp(source + content, body, strlen(body)));
}

static void fm_matrix(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned a, b, c, bom, closer, eof, native, mode, cases = 0, rejected = 0;
    for (a = 0; a < 3; a++) for (b = 0; b < 3; b++) for (c = 0; c < 3; c++)
    for (bom = 0; bom < 2; bom++) for (closer = 0; closer < 2; closer++) for (eof = 0; eof < 2; eof++) {
        char source[256], initial[256], body[64];
        uint64_t opening = bom ? 3 : 0, content = opening + 3 + strlen(endings[a]);
        uint64_t close_start, close_end; unsigned dialect;
        snprintf(body, sizeof(body), "key: value%s", endings[b]);
        snprintf(source, sizeof(source), "%s---%s%s%s%s", bom ? "\xef\xbb\xbf" : "", endings[a], body,
            closer ? "..." : "---", eof ? "" : endings[c]);
        close_start = content + strlen(body); close_end = strlen(source);
        if (!eof) { strcat(source, endings[c]); strcat(source, "ordinary *kept*"); strcat(source, endings[c]); }
        for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_GFM; dialect++) {
            xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot s;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
                xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && !find_kind(s, 1, XUI_DOC_FRONT_MATTER));
            test_source(d, source, 1); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d); rejected++;
        }
        for (native = 0; native < 2; native++) for (mode = 0; mode < 4; mode++) {
            xui_doc_desc_t desc = {0}; xui_document d, oracle; xui_document_snapshot before, after, expected, restored;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            memcpy(initial, source, content + 5); strcpy(initial + content + 5, "old");
            strcat(initial, source + content + 10);
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, mode ? initial : source, strlen(mode ? initial : source)) == XUI_OK);
            if (native) {
                char* wire; uint64_t bytes; xui_document loaded;
                CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &wire, &bytes) == XUI_OK &&
                    xuiDocumentDeserialize(NULL, wire, bytes, &loaded) == XUI_OK);
                xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = loaded;
            }
            CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            if (mode == 1) {
                xui_doc_txn_desc_t td = {0}; xui_document_transaction t;
                td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK &&
                    xuiDocumentTxnReplaceSource(t, content + 5, content + 8, "value", 5) == XUI_OK &&
                    xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
            } else if (mode >= 2) {
                xui_doc_source_patch_t patch = prepare_patch(content + 5, content + 8, mode == 3 ? "valu" : "value");
                xui_document_prepare p, next;
                CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK);
                if (mode == 3) {
                    patch = prepare_patch(content + 9, content + 9, "e");
                    CHECK(xuiDocumentPrepareContinueSource(d, p, &patch, 1, &next) == XUI_OK);
                    xuiDocumentPrepareRelease(p); p = next; CHECK(xuiDocumentPrepareRun(p) == XUI_OK);
                }
                CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
            }
            test_source(d, source, 1); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            fm_golden(after, source, opening, content, close_start, close_end, body);
            CHECK(xuiDocumentCreate(&desc, &oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle, source, strlen(source)) == XUI_OK &&
                xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK); inc_snapshot_equal(after, expected);
            xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
            if (mode) {
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                inc_snapshot_equal(before, restored); xuiDocumentSnapshotRelease(restored);
                CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                inc_snapshot_equal(after, restored); xuiDocumentSnapshotRelease(restored);
            }
            xuiDocumentRelease(d); fm_golden(after, source, opening, content, close_start, close_end, body);
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); cases++;
        }
    }
    printf("Front matter raw-line golden: %u cases, %u dialect exclusions; all 27 LF/CRLF/CR mixtures, BOM, both closers, EOF, native/SOURCE/Prepare/Continue, raw body/ranges, Undo and retained snapshots passed\n", cases, rejected);
}

static void fm_boundaries(void)
{
    const char* bodies[] = {"---\rkey: old\r---\r\rbody\r", "---\r\nkey: old\r...\n\nbody\n",
        "\xef\xbb\xbf---\nkey: old\r---\r\n\r\nbody\n", "---\r---\r\rbody\r", "---\rkey: old\r---",
        "---\rkey: old\r", "ordinary\r---\rkey: old\r---\r", "--- \rkey: old\r---\r"};
    const char* edits[] = {"\r", "\n", "\r\n", "", "---", "...", "x", "\t"};
    unsigned f, e; uint64_t cases = 0, incremental = 0; size_t at;
    for (f = 0; f < sizeof(bodies) / sizeof(*bodies); f++)
    for (at = 0; at <= strlen(bodies[f]); at++) {
        if (f == 2 && at < 3) continue;
        for (e = 0; e < sizeof(edits) / sizeof(*edits); e++) {
            uint64_t remove = (e & 1) && at < strlen(bodies[f]) ? 1 : 0;
            incremental += inc_case(bodies[f], XUI_MD_EXTENDED, at, remove, edits[e], (int)(cases & 1)); cases++;
        }
    }
    printf("Front matter boundary differential: %llu cases, %llu incremental; delimiters, body, blank/unterminated/noninitial, mixed endings and EOF passed\n",
        (unsigned long long)cases, (unsigned long long)incremental);
}

static void fm_faults(void)
{
    const char* original[] = {"---\rkey: old\r---\r\rbody\r", "--- \rkey: old\r---\r"};
    const char* edited[] = {"---\rkey: value\r---\r\rbody\r", "---\n\rkey: old\r---\r"};
    uint64_t start[] = {9, 3}, end[] = {12, 4}; const char* insert[] = {"value", "\n"};
    unsigned fixture, mode; long oom = 0, cancel = 0;
    for (fixture = 0; fixture < 2; fixture++) for (mode = 0; mode < 2; mode++) {
        long point; int success = 0;
        for (point = 0; point < 4096 && !success; point++) {
            fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_snapshot before, after; uint64_t revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, original[fixture], strlen(original[fixture])) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
                xuiDocumentMarkSaved(d, before) == XUI_OK); revision = xuiDocumentGetRevision(d);
            a.remaining = point;
            if (mode) {
                xui_document_prepare p = NULL; xui_doc_source_patch_t patch = prepare_patch(start[fixture], end[fixture], insert[fixture]);
                result = xuiDocumentPrepareSource(d, NULL, &patch, 1, &p);
                if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
                if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
                a.remaining = -1; xuiDocumentPrepareRelease(p);
            } else {
                xui_document_transaction t = NULL; xui_doc_txn_desc_t td = {0};
                td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                result = xuiDocumentBeginTransaction(d, &td, &t);
                if (result == XUI_OK) result = xuiDocumentTxnReplaceSource(t, start[fixture], end[fixture], insert[fixture], strlen(insert[fixture]));
                if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
                a.remaining = -1; xuiDocumentTxnRelease(t);
            }
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result != XUI_OK) {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) &&
                    !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                inc_snapshot_equal(before, after); expect_identity_tree(before, after, 1); test_source(d, original[fixture], 1); oom++;
            } else {
                xui_document oracle; xui_document_snapshot expected;
                test_source(d, edited[fixture], 1); CHECK(xuiDocumentIsDirty(d) && xuiDocumentCanUndo(d));
                CHECK(xuiDocumentCreate(&desc, &oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle, edited[fixture], strlen(edited[fixture])) == XUI_OK &&
                    xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK); inc_snapshot_equal(after, expected);
                xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle); success = 1;
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
            xui_doc_source_patch_t patch = prepare_patch(start[fixture], end[fixture], insert[fixture]);
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
    printf("Front matter faults: %ld SOURCE/Prepare OOM, %ld Prepare cancellation checkpoints; body edit and opener activation, complete state/history/savepoint and zero leaks passed\n", oom, cancel);
}
static void fm_global_dependencies(void)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned fixture, eol, bom, native, mode, cases = 0; uint64_t incremental = 0;
    for (fixture = 0; fixture < 6; fixture++) for (eol = 0; eol < 3; eol++)
    for (bom = 0; bom < 2; bom++) for (native = 0; native < 2; native++) for (mode = 0; mode < 5; mode++) {
        char source[8192], edited[8192], line[128]; uint64_t at, removed, added; const char* replacement;
        unsigned i; int expected_front = fixture != 1 && fixture != 3;
        prepare_gate_allocator allocator; xui_doc_desc_t desc = {0}; xui_document d, oracle;
        xui_document_snapshot before, after, expected, restored; xui_doc_source_info_t info = {0};
        xui_doc_stats_t stats_before, stats_after; xui_doc_range_t first, last; xui_doc_source_patch_t patch;
        atomic_init(&allocator.live, 0); atomic_init(&allocator.gate, 0); allocator.owner = xrtThreadCurrentId();
        atomic_init(&allocator.entered, 0); atomic_init(&allocator.resume, 0);
        snprintf(source, sizeof(source), "%s%s%s[r]: /meta%s[^n]: meta [^m]%s[^m]: nested%s%s%s%suse [r] [^n]%s%soutside [o]%s%s[o]: /outside%s%s",
            bom ? "\xef\xbb\xbf" : "", fixture == 0 ? "--- " : "---", endings[eol], endings[eol],
            endings[eol], endings[eol], fixture == 4 ? "x--" : "---", endings[eol], endings[eol],
            endings[eol], endings[eol], endings[eol], endings[eol], endings[eol], endings[eol]);
        for (i = 0; i < 32; i++) {
            snprintf(line, sizeof(line), "kept paragraph %02u *span*%s%s", i, endings[eol], endings[eol]); strcat(source, line);
        }
        if (fixture == 0 || fixture == 1) { at = (bom ? 3 : 0) + 3; removed = fixture == 0 ? 1 : 0; replacement = fixture == 0 ? "" : " "; }
        else if (fixture == 2) { at = (uint64_t)(strstr(source, "/meta") - source); removed = 5; replacement = "/changed"; }
        else if (fixture == 3 || fixture == 4) {
            const char* closer = strstr(source + (bom ? 3 : 0) + 4, fixture == 4 ? "x--" : "---");
            CHECK(closer); at = (uint64_t)(closer - source); removed = 1; replacement = fixture == 4 ? "-" : "x";
        } else { at = (uint64_t)(strstr(source, "[r]:") - source) + 1; removed = 1; replacement = "renamed"; }
        added = strlen(replacement); memcpy(edited, source, at); strcpy(edited + at, replacement); strcat(edited, source + at + removed);
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
        desc.onAlloc = prepare_gate_alloc; desc.onFree = prepare_gate_free; desc.pAllocatorUser = &allocator;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
        if (native) {
            char* wire; uint64_t bytes; xui_document loaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &wire, &bytes) == XUI_OK &&
                xuiDocumentDeserialize(&desc, wire, bytes, &loaded) == XUI_OK);
            xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = loaded;
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        first = test_find(d, "kept paragraph 00"); last = test_find(d, "kept paragraph 31"); stats_before = inc_stats(d);
        patch = prepare_patch(at, at + removed, replacement);
        if (!mode) {
            xui_doc_txn_desc_t td = {0}; xui_document_transaction t; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK &&
                xuiDocumentTxnReplaceSource(t, at, at + removed, replacement, added) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
            xuiDocumentTxnRelease(t);
        } else {
            xui_document_prepare p, next; xthread* worker = NULL;
            CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK);
            if (mode == 3) CHECK(xuiDocumentPrepareRun(p) == XUI_OK);
            if (mode == 4) {
                atomic_store(&allocator.gate, 1); worker = prepare_start(p); prepare_gate_wait(&allocator.entered);
                CHECK(prepare_info(p).iState == XUI_DOC_PREPARE_RUNNING);
            }
            if (mode >= 2) {
                patch = prepare_patch(strlen(edited), strlen(edited), "\ncontinued\n");
                CHECK(xuiDocumentPrepareContinueSource(d, p, &patch, 1, &next) == XUI_OK);
                if (worker) { atomic_store(&allocator.resume, 1); CHECK(prepare_finish(worker) == XUI_DOC_ERROR_STALE); }
                xuiDocumentPrepareRelease(p); p = next; strcat(edited, "\ncontinued\n");
            }
            CHECK(xuiDocumentPrepareRun(p) == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
        }
        test_source(d, edited, 1); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        stats_after = inc_stats(d); incremental += stats_after.iMarkdownIncrementalParses - stats_before.iMarkdownIncrementalParses;
        info.iSize = sizeof(info);
        CHECK(xuiDocumentSnapshotGetSourceInfo(after, &info) == XUI_OK && info.iReferenceDefinitionCount == (expected_front ? 1u : 4u));
        CHECK((find_kind(after, 1, XUI_DOC_FRONT_MATTER) != 0) == expected_front);
        CHECK(test_find(d, "kept paragraph 00").tAnchor.iNodeId == first.tAnchor.iNodeId &&
            test_find(d, "kept paragraph 31").tAnchor.iNodeId == last.tAnchor.iNodeId);
        CHECK(xuiDocumentCreate(&desc, &oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle, edited, strlen(edited)) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK); inc_snapshot_equal(after, expected);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(before, restored); expect_identity_tree(before, restored, 1); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(after, restored); expect_identity_tree(after, restored, 1); xuiDocumentSnapshotRelease(restored);
        xuiDocumentRelease(d); CHECK((find_kind(after, 1, XUI_DOC_FRONT_MATTER) != 0) == expected_front);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); CHECK(!atomic_load(&allocator.live)); cases++;
    }
    CHECK(incremental > cases / 2);
    printf("Front matter global dependencies: %u cases, %llu incremental; opener/closer activation, hidden links and transitive footnotes, unchanged IDs, native/SOURCE/queued/ready/RUNNING Continue, full oracle and Undo passed\n",
        cases, (unsigned long long)incremental);
}
int main(void)
{
    fm_matrix(); fm_boundaries(); fm_faults(); fm_global_dependencies(); return 0;
}
