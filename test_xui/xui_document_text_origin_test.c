#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main

typedef struct norm_case {
    char source[2048], expected[512];
    uint64_t size, source_start, source_end, text_start, text_end, edit_start, edit_size;
    unsigned kind, segment_kind;
} norm_case;
static void norm_build(norm_case* c, unsigned fixture, unsigned eol, int native)
{
    const char* endings[] = {"\n", "\r\n", "\r"}; const char* ending = endings[eol];
    const char* prefix = native ? "\xef\xbb\xbf---\rname: origin\r\n...\n\nprior `span`\n\n" : "prior `span`\n\n";
    char body[1024]; const char* expected = NULL; uint64_t start = 0, end = 0;
    memset(c, 0, sizeof(*c)); c->kind = XUI_DOC_TEXT; c->segment_kind = XUI_DOC_SOURCE_NORMALIZED; c->text_end = 1;
    if (fixture < 4) {
        const char* first[] = {"`aa", "![`aa", "> `aa", "[`aa"};
        const char* second[] = {"bb`", "bb`](u)", "> bb`", "bb`](u)"};
        snprintf(body, sizeof(body), "%s%s%s%s", first[fixture], ending, second[fixture], ending);
        start = strlen(first[fixture]); end = start + strlen(ending); expected = fixture == 1 ? "aa bb" : " ";
        if (fixture == 1) { c->kind = XUI_DOC_IMAGE; c->text_start = 2; c->text_end = 3; }
    } else if (fixture == 4 || fixture == 6) {
        if (fixture == 4) {
            snprintf(body, sizeof(body), "> ```c%s> %s> aa%s> ```%s", ending, ending, ending, ending);
            start = 6 + strlen(ending) + 2;
        } else {
            snprintf(body, sizeof(body), "```c%s%saa%s```%s", ending, ending, ending, ending);
            start = 4 + strlen(ending);
        }
        end = start + strlen(ending); c->kind = XUI_DOC_CODE_BLOCK; expected = "\naa\n";
    } else if (fixture == 5) {
        snprintf(body, sizeof(body), "    aa%s    %s    bb%s", ending, ending, ending);
        start = 6 + strlen(ending) + 4; end = start + strlen(ending);
        c->kind = XUI_DOC_CODE_BLOCK; expected = "aa\n\nbb\n"; c->text_start = 3; c->text_end = 4;
    } else if (fixture == 7 || fixture == 14 || fixture == 15) {
        if (fixture == 7) {
            snprintf(body, sizeof(body), " ```c%s\tword%s ```%s", ending, ending, ending);
            start = 5 + strlen(ending); end = start + 1;
        } else {
            char indent[41]; unsigned count = fixture == 14 ? 4 : 40;
            memset(indent, ' ', count); indent[count] = 0;
            snprintf(body, sizeof(body), " ```c%s%sword%s ```%s", ending, indent, ending, ending);
            start = 5 + strlen(ending) + 1; end = start + (fixture == 14 ? 3 : 16);
            c->segment_kind = XUI_DOC_SOURCE_DIRECT;
        }
        c->kind = XUI_DOC_CODE_BLOCK; c->text_end = fixture == 15 ? 16 : 3;
        if (fixture == 15) { memset(c->expected, ' ', 39); strcpy(c->expected + 39, "word\n"); }
        else expected = "   word\n";
    } else if (fixture == 8 || fixture == 9 || fixture == 10 || fixture == 18) {
        if (fixture == 8) snprintf(body, sizeof(body), "aa@bb%s", ending);
        else if (fixture == 10) snprintf(body, sizeof(body), "<div>aa@bb</div>%s", ending);
        else snprintf(body, sizeof(body), fixture == 18 ? "```c%saa@@bb%s```%s" : "```c%saa@bb%s```%s", ending, ending, ending);
        start = (uint64_t)(strchr(body, '@') - body); end = start + 1;
        if (fixture == 8) { expected = "\xef\xbf\xbd"; c->text_end = 3; }
        else if (fixture == 10) {
            expected = "<div>aa\xef\xbf\xbd" "bb</div>\n"; c->kind = XUI_DOC_HTML; c->text_start = 7; c->text_end = 10;
        } else {
            expected = fixture == 18 ? "aa\xef\xbf\xbd\xef\xbf\xbd" "bb\n" : "aa\xef\xbf\xbd" "bb\n";
            c->kind = XUI_DOC_CODE_BLOCK; c->text_start = 2; c->text_end = 5;
        }
    } else if (fixture == 11) {
        snprintf(body, sizeof(body), "a <i%s title=\"x\">b</i>%s", ending, ending);
        start = 4; end = start + strlen(ending); c->kind = XUI_DOC_HTML; expected = "<i\ntitle=\"x\">";
        c->text_start = 2; c->text_end = 3;
    } else if (fixture == 12) {
        snprintf(body, sizeof(body), "$aa%sbb$%s", ending, ending);
        start = 3; end = start + strlen(ending); c->kind = XUI_DOC_MATH; expected = "aa bb"; c->text_start = 2; c->text_end = 3;
    } else if (fixture == 13) {
        snprintf(body, sizeof(body), "> <script>%s> %s> aa</script>%s", ending, ending, ending);
        start = 10 + strlen(ending) + 2; end = start + strlen(ending); c->kind = XUI_DOC_HTML;
        expected = "<script>\n\naa</script>\n"; c->text_start = 9; c->text_end = 10;
    } else if (fixture == 16) {
        snprintf(body, sizeof(body), "ref[^n]%s%s[^n]:%s    ```c%s    %s    aa%s    ```%s", ending, ending, ending, ending, ending, ending, ending);
        start = 7 + 2 * strlen(ending) + 5 + strlen(ending) + 8 + strlen(ending) + 4;
        end = start + strlen(ending); c->kind = XUI_DOC_CODE_BLOCK; expected = "\naa\n";
    } else if (fixture >= 19) {
        if (fixture == 19) snprintf(body, sizeof(body), "`aa@bb`%s", ending);
        else if (fixture == 20) snprintf(body, sizeof(body), "![`aa@bb`](u)%s", ending);
        else if (fixture == 21) snprintf(body, sizeof(body), "$aa@bb$%s", ending);
        else { CHECK(fixture == 22); snprintf(body, sizeof(body), "a <i title=\"@\">b</i>%s", ending); }
        start = (uint64_t)(strchr(body, '@') - body); end = start + 1;
        if (fixture == 20 || fixture == 21) {
            c->kind = fixture == 20 ? XUI_DOC_IMAGE : XUI_DOC_MATH;
            expected = "aa\xef\xbf\xbd" "bb"; c->text_start = 2; c->text_end = 5;
        } else if (fixture == 22) {
            expected = "<i title=\"\xef\xbf\xbd\">"; c->kind = XUI_DOC_HTML; c->text_start = 10; c->text_end = 13;
        } else { expected = "\xef\xbf\xbd"; c->text_end = 3; }
    } else {
        CHECK(fixture == 17); snprintf(body, sizeof(body), "```c%saa", ending);
        start = end = strlen(body); c->kind = XUI_DOC_CODE_BLOCK; expected = "aa\n"; c->text_start = 2; c->text_end = 3;
    }
    if (expected) strcpy(c->expected, expected);
    c->size = strlen(prefix) + strlen(body); strcpy(c->source, prefix); strcat(c->source, body);
    c->source_start = strlen(prefix) + start; c->source_end = strlen(prefix) + end;
    if (fixture == 8 || fixture == 9 || fixture == 10 || fixture >= 18) c->source[c->source_start] = 0;
    if (fixture == 18) c->source[c->source_start + 1] = 0;
    c->edit_start = c->source_start; c->edit_size = c->source_end - c->source_start;
    if (!c->edit_size) { c->edit_start--; c->edit_size = 1; }
}
static void norm_source(xui_document d, const char* source, uint64_t bytes)
{
    xui_document_snapshot s; char value[4096]; uint64_t count;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK &&
        xuiDocumentSnapshotCopySource(s, value, sizeof(value), &count) == XUI_OK && count == bytes && !memcmp(value, source, count));
    xuiDocumentSnapshotRelease(s);
}
static uint64_t norm_find(xui_document_snapshot s, uint64_t id, const norm_case* c)
{
    xui_doc_node_info_t node = {0}; uint64_t child, found, i, bytes; char value[512];
    node.iSize = sizeof(node); CHECK(xuiDocumentSnapshotGetNode(s, id, &node) == XUI_OK);
    if (node.iKind == c->kind && xuiDocumentSnapshotCopyText(s, id, value, sizeof(value), &bytes) == XUI_OK &&
        bytes == strlen(c->expected) && !memcmp(value, c->expected, bytes)) return id;
    for (i = 0; i < node.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK); found = norm_find(s, child, c); if (found) return found;
    }
    return 0;
}
static void norm_golden(xui_document_snapshot s, const norm_case* c)
{
    xui_doc_node_info_t node = {0}; xui_doc_source_segment_t segment = {0}; uint64_t i, id = norm_find(s, 1, c); int matched = 0;
    node.iSize = sizeof(node); CHECK(id && xuiDocumentSnapshotGetNode(s, id, &node) == XUI_OK);
    for (i = 0; i < node.iSourceSegmentCount; i++) {
        segment.iSize = sizeof(segment); CHECK(xuiDocumentSnapshotGetSourceSegment(s, id, i, &segment) == XUI_OK);
        if (segment.iTextStart != c->text_start || segment.iTextEnd != c->text_end) continue;
        CHECK(segment.iKind == c->segment_kind && segment.iSourceStart == c->source_start && segment.iSourceEnd == c->source_end);
        matched = 1; break;
    }
    CHECK(matched);
    if (c->source_end - c->source_start == 2 && c->source[c->source_start] == '\r') {
        expect_source_position(s, c->source_start + 1, XUI_DOC_BEFORE, id, c->text_start, XUI_DOC_MAP_COLLAPSED);
        expect_source_position(s, c->source_start + 1, XUI_DOC_AFTER, id, c->text_end, XUI_DOC_MAP_COLLAPSED);
    }
    if (c->segment_kind == XUI_DOC_SOURCE_DIRECT) {
        expect_source_position(s, c->source_start + 1, XUI_DOC_BEFORE, id, c->text_start + 1, XUI_DOC_MAP_EXACT);
    }
}
static xui_doc_source_patch_t norm_patch(uint64_t start, uint64_t end, const char* text, uint64_t bytes)
{
    xui_doc_source_patch_t patch = {0}; patch.iSize = sizeof(patch); patch.iStart = start; patch.iEnd = end;
    patch.sText = text; patch.iTextBytes = bytes; return patch;
}
static void norm_initial(const norm_case* c, char* initial, uint64_t* bytes)
{
    memcpy(initial, c->source, c->edit_start);
    memcpy(initial + c->edit_start, c->source + c->edit_start + c->edit_size, c->size - c->edit_start - c->edit_size);
    *bytes = c->size - c->edit_size; initial[*bytes] = 0;
}
static void norm_matrix(void)
{
    unsigned fixture, dialect, eol, native, mode, cases = 0;
    for (fixture = 0; fixture < 23; fixture++) for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) {
        if ((fixture == 12 || fixture == 16 || fixture == 21) && dialect != XUI_MD_EXTENDED) continue;
        for (eol = 0; eol < 3; eol++) for (native = 0; native < 2; native++) for (mode = 0; mode < 4; mode++) {
            norm_case c; char initial[2048]; uint64_t bytes; xui_doc_desc_t desc = {0};
            xui_document d, oracle; xui_document_snapshot before, after, expected, restored;
            norm_build(&c, fixture, eol, native); norm_initial(&c, initial, &bytes);
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, mode ? initial : c.source, mode ? bytes : c.size) == XUI_OK);
            if (native) {
                char* wire; uint64_t wire_bytes; xui_document loaded;
                CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &wire, &wire_bytes) == XUI_OK &&
                    xuiDocumentDeserialize(NULL, wire, wire_bytes, &loaded) == XUI_OK);
                xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = loaded;
            }
            CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            if (mode == 1) {
                xui_doc_txn_desc_t td = {0}; xui_document_transaction t; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK && xuiDocumentTxnReplaceSource(t, c.edit_start, c.edit_start,
                    c.source + c.edit_start, c.edit_size) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
            } else if (mode >= 2) {
                xui_doc_source_patch_t patch = norm_patch(c.edit_start, c.edit_start, c.source + c.edit_start, c.edit_size);
                xui_document_prepare p, next;
                CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK);
                if (mode == 3) {
                    uint64_t at = (uint64_t)(strstr(initial, "prior") - initial);
                    patch = norm_patch(at, at + 5, "later", 5); CHECK(xuiDocumentPrepareContinueSource(d, p, &patch, 1, &next) == XUI_OK);
                    xuiDocumentPrepareRelease(p); p = next; memcpy(c.source + at, "later", 5); CHECK(xuiDocumentPrepareRun(p) == XUI_OK);
                }
                CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
            }
            norm_source(d, c.source, c.size); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            norm_golden(after, &c);
            CHECK(xuiDocumentCreate(&desc, &oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle, c.source, c.size) == XUI_OK &&
                xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK); inc_snapshot_equal(after, expected);
            xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
            if (mode) {
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                inc_snapshot_equal(before, restored); expect_identity_tree(before, restored, 1); xuiDocumentSnapshotRelease(restored);
                CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                inc_snapshot_equal(after, restored); expect_identity_tree(after, restored, 1); xuiDocumentSnapshotRelease(restored);
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); norm_golden(after, &c); xuiDocumentSnapshotRelease(after); cases++;
        }
    }
    CHECK(cases == 1512);
    printf("Normalized text golden: %u independent raw-byte/semantic/position cases; code/math/HTML/NUL/Tab/blank/EOF/footnote, native/SOURCE/Prepare/Continue/history/lifetime passed\n", cases);
}
static void norm_boundaries(void)
{
    const char* edits[] = {"", "\n", "`", "\0"}; const uint64_t sizes[] = {0, 1, 1, 1};
    unsigned fixture, eol, edit, cases = 0; uint64_t incremental = 0;
    for (fixture = 0; fixture < 23; fixture++) for (eol = 0; eol < 3; eol++) {
        norm_case c; uint64_t at; norm_build(&c, fixture, eol, 0);
        for (at = 0; at <= c.size; at++) for (edit = 0; edit < 4; edit++) {
            char result[4096]; uint64_t removed = edit ? 0 : at < c.size ? 1 : 0, bytes = c.size - removed + sizes[edit];
            xui_doc_desc_t desc = {0}; xui_document d, oracle; xui_document_snapshot before, after, expected, restored;
            xui_document_transaction t; xui_doc_txn_desc_t td = {0}; xui_doc_stats_t stats;
            memcpy(result, c.source, at); memcpy(result + at, edits[edit], sizes[edit]);
            memcpy(result + at + sizes[edit], c.source + at + removed, c.size - at - removed);
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, c.source, c.size) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK && xuiDocumentTxnReplaceSource(t, at, at + removed, edits[edit], sizes[edit]) == XUI_OK &&
                xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
            norm_source(d, result, bytes); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            stats = inc_stats(d); incremental += stats.iMarkdownIncrementalParses;
            CHECK(xuiDocumentCreate(&desc, &oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle, result, bytes) == XUI_OK &&
                xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK); inc_snapshot_equal(after, expected);
            xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
            if (removed || sizes[edit]) {
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                inc_snapshot_equal(before, restored); xuiDocumentSnapshotRelease(restored);
                CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                inc_snapshot_equal(after, restored); xuiDocumentSnapshotRelease(restored);
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); cases++;
        }
    }
    printf("Normalized text differential: %u cases, %llu incremental; every byte/NUL/delimiters/containers and complete full-parse/history oracle passed\n", cases, (unsigned long long)incremental);
}
static void norm_faults(void)
{
    const unsigned fixtures[] = {0, 4, 7, 9, 11, 22}; unsigned f, mode; long oom = 0, cancel = 0;
    for (f = 0; f < sizeof(fixtures) / sizeof(*fixtures); f++) for (mode = 0; mode < 2; mode++) {
        norm_case c; char initial[2048]; uint64_t bytes; long point; int success = 0;
        norm_build(&c, fixtures[f], 1, 0); norm_initial(&c, initial, &bytes);
        for (point = 0; point < 4096 && !success; point++) {
            fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot before, after;
            uint64_t revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, initial, bytes) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
            revision = xuiDocumentGetRevision(d); a.remaining = point;
            if (mode) {
                xui_document_prepare p = NULL; xui_doc_source_patch_t patch = norm_patch(c.edit_start, c.edit_start, c.source + c.edit_start, c.edit_size);
                result = xuiDocumentPrepareSource(d, NULL, &patch, 1, &p);
                if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
                if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
                a.remaining = -1; xuiDocumentPrepareRelease(p);
            } else {
                xui_document_transaction t = NULL; xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                result = xuiDocumentBeginTransaction(d, &td, &t);
                if (result == XUI_OK) result = xuiDocumentTxnReplaceSource(t, c.edit_start, c.edit_start, c.source + c.edit_start, c.edit_size);
                if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
                a.remaining = -1; xuiDocumentTxnRelease(t);
            }
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result != XUI_OK) {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                inc_snapshot_equal(before, after); expect_identity_tree(before, after, 1); norm_source(d, initial, bytes); oom++;
            } else { norm_source(d, c.source, c.size); norm_golden(after, &c); success = 1; }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success);
    }
    for (f = 0; f < sizeof(fixtures) / sizeof(*fixtures); f++) {
        norm_case c; char initial[2048]; uint64_t bytes; long point; int success = 0;
        norm_build(&c, fixtures[f], 1, 0); norm_initial(&c, initial, &bytes);
        for (point = 0; point < 4096 && !success; point++) {
            prepare_cancel_allocator a = {0, -1, 0, NULL}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_prepare p; xui_document_snapshot before, after; uint64_t revision; int result;
            xui_doc_source_patch_t patch = norm_patch(c.edit_start, c.edit_start, c.source + c.edit_start, c.edit_size);
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, initial, bytes) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
            revision = xuiDocumentGetRevision(d); CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &p) == XUI_OK);
            a.prepare = p; a.remaining = point; result = xuiDocumentPrepareRun(p); a.prepare = NULL;
            if (a.fired) {
                CHECK(result == XUI_DOC_ERROR_CANCELLED && xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED &&
                    xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
                CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); inc_snapshot_equal(before, after); expect_identity_tree(before, after, 1);
                norm_source(d, initial, bytes); xuiDocumentSnapshotRelease(after); cancel++;
            } else { CHECK(result == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); norm_source(d, c.source, c.size); success = 1; }
            xuiDocumentPrepareRelease(p); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success);
    }
    printf("Normalized text faults: %ld SOURCE/Prepare OOM, %ld Prepare cancellation checkpoints; code/blank/Tab/NUL/HTML scope state/identity/history/savepoint and zero leaks passed\n", oom, cancel);
}
int main(void) { norm_matrix(); norm_boundaries(); norm_faults(); return 0; }
