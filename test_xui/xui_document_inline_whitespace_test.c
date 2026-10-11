#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main

typedef struct whitespace_fixture { const char* source; const char* expected; int partial_tab; } whitespace_fixture;
static const whitespace_fixture fixtures[] = {
    {"z <i\n title=\"x\">\n", "<i\n title=\"x\">", 0},
    {"z <i title=\"a\n b\">\n", "<i title=\"a\n b\">", 0},
    {"z <!-- a\n b -->\n", "<!-- a\n b -->", 0},
    {"z <?a\n b?>\n", "<?a\n b?>", 0},
    {"z <![CDATA[a\n b]]>\n", "<![CDATA[a\n b]]>", 0},
    {"z <!A\n b>\n", "<!A\n b>", 0},
    {"z <i title=\"a  \n b\">\n", "<i title=\"a  \n b\">", 0},
    {"z `aa\n bb`\n", "aa  bb", 0}, {"z `aa\n\tbb`\n", "aa \tbb", 0},
    {"> z <i title=\"a\n>  b\">\n", "<i title=\"a\n b\">", 0},
    {"- z <i title=\"a\n   b\">\n", "<i title=\"a\n b\">", 0},
    {"> z `aa\n>  bb`\n", "aa  bb", 0}, {"- z `aa\n   bb`\n", "aa  bb", 0},
    {"z ` aa\n bb `\n", "aa  bb", 0}, {"z `\naa\n bb\n`\n", "aa  bb", 0},
    {"z `\n  aa\n bb `\n", "  aa  bb", 0}, {"z `  \n aa `\n", "   aa", 0},
    {"z ` \n `\n", "   ", 0}, {"z ` \n\t `\n", " \t", 0},
    {"z `aa\n   bb`\n", "aa    bb", 0}, {"> z ` aa\n>  bb `\n", "aa  bb", 0},
    {"- z ` aa\n   bb `\n", "aa  bb", 0}, {"- z `aa\n\tbb`\n", "aa   bb", 1},
    {"> z `aa\n>\tbb`\n", "aa   bb", 1}, {"- z ` aa\n\t`\n", "aa  ", 1},
    {"- z `\n\taa `\n", "  aa", 1}, {"> z ` \n>\t`\n", "    ", 1},
    {"z <i title=\"a\n\tb\">\n", "<i title=\"a\n\tb\">", 0},
    {"> z <i title=\"a\n>\tb\">\n", "<i title=\"a\n  b\">", 1},
    {"- z <i title=\"a\n\tb\">\n", "<i title=\"a\n  b\">", 1},
    {"z `aa\n bb` and `cc\n dd`\n", "aa  bbcc  dd", 0},
    {"z <i title=\"a\n b@\">\n", "<i title=\"a\n b\xef\xbf\xbd\">", 0},
};
typedef struct whitespace_case { char source[2048]; uint64_t size, body; const whitespace_fixture* fixture; } whitespace_case;
typedef struct whitespace_capture { char value[1024]; uint64_t size, tabs; } whitespace_capture;
static void build_case(whitespace_case* c, unsigned fixture, unsigned eol, int native)
{
    const char* endings[] = {"\n", "\r\n", "\r"}; const char* p;
    strcpy(c->source, native ? "\xef\xbb\xbf---\rname: whitespace\r\n...\n\nprior\n\n" : "prior\n\n");
    c->body = c->size = strlen(c->source); c->fixture = &fixtures[fixture];
    for (p = c->fixture->source; *p; p++) {
        if (*p == '\n') { uint64_t n = strlen(endings[eol]); memcpy(c->source + c->size, endings[eol], n); c->size += n; }
        else c->source[c->size++] = *p == '@' ? 0 : *p;
    }
    c->source[c->size] = 0;
}
static void capture(xui_document_snapshot s, uint64_t id, const whitespace_case* c, whitespace_capture* out)
{
    xui_doc_node_info_t n = {0}; uint64_t i, child; n.iSize = sizeof(n);
    CHECK(xuiDocumentSnapshotGetNode(s, id, &n) == XUI_OK);
    if (n.iKind == XUI_DOC_HTML || (n.iKind == XUI_DOC_TEXT && (n.tAttributes.iMarks & XUI_DOC_CODE))) {
        char text[1024]; uint64_t bytes;
        CHECK(xuiDocumentSnapshotCopyText(s, id, text, sizeof(text), &bytes) == XUI_OK && out->size + bytes < sizeof(out->value));
        memcpy(out->value + out->size, text, bytes); out->size += bytes;
        for (i = 0; i < n.iSourceSegmentCount; i++) {
            xui_doc_source_segment_t seg = {0}; seg.iSize = sizeof(seg);
            CHECK(xuiDocumentSnapshotGetSourceSegment(s, id, i, &seg) == XUI_OK && seg.iSourceEnd <= c->size && seg.iTextEnd <= bytes);
            if (seg.iKind == XUI_DOC_SOURCE_DIRECT) {
                CHECK(seg.iSourceEnd - seg.iSourceStart == seg.iTextEnd - seg.iTextStart &&
                    !memcmp(c->source + seg.iSourceStart, text + seg.iTextStart, seg.iTextEnd - seg.iTextStart));
                if (seg.iTextEnd - seg.iTextStart > 1) expect_source_position(s, seg.iSourceStart + 1, XUI_DOC_BEFORE,
                    id, seg.iTextStart + 1, XUI_DOC_MAP_EXACT);
            } else if (seg.iKind == XUI_DOC_SOURCE_NORMALIZED && seg.iSourceEnd == seg.iSourceStart + 1 &&
                       c->source[seg.iSourceStart] == '\t' && seg.iTextEnd > seg.iTextStart) {
                uint64_t j; for (j = seg.iTextStart; j < seg.iTextEnd; j++) CHECK(text[j] == ' ');
                out->tabs++;
            }
        }
    }
    for (i = 0; i < n.iChildCount; i++) { CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK); capture(s, child, c, out); }
}
static void golden(xui_document_snapshot s, const whitespace_case* c)
{
    whitespace_capture out = {{0}, 0, 0}; char raw[2048]; uint64_t bytes;
    CHECK(xuiDocumentSnapshotCopySource(s, raw, sizeof(raw), &bytes) == XUI_OK && bytes == c->size && !memcmp(raw, c->source, bytes));
    capture(s, 1, c, &out);
    if (out.size != strlen(c->fixture->expected) || memcmp(out.value, c->fixture->expected, out.size)) {
        fprintf(stderr, "whitespace fixture %llu semantic mismatch: actual bytes %llu expected %llu\n",
            (unsigned long long)(c->fixture - fixtures), (unsigned long long)out.size, (unsigned long long)strlen(c->fixture->expected)); exit(1);
    }
    CHECK(!c->fixture->partial_tab || out.tabs);
}
static xui_doc_source_patch_t patch(uint64_t start, uint64_t end, const char* text, uint64_t bytes)
{
    xui_doc_source_patch_t p = {0}; p.iSize = sizeof(p); p.iStart = start; p.iEnd = end; p.sText = text; p.iTextBytes = bytes; return p;
}
static void matrix(void)
{
    unsigned fixture, eol, dialect, native, mode, cases = 0;
    for (fixture = 0; fixture < sizeof(fixtures) / sizeof(*fixtures); fixture++) for (eol = 0; eol < 3; eol++)
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) for (native = 0; native < 2; native++) for (mode = 0; mode < 4; mode++) {
        whitespace_case c; char initial[2048]; uint64_t bytes; xui_doc_desc_t desc = {0};
        xui_document d, oracle; xui_document_snapshot before, after, expected, restored;
        build_case(&c, fixture, eol, native); memcpy(initial, c.source, c.body); strcpy(initial + c.body, "z placeholder\n"); bytes = c.body + 14;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, mode ? initial : c.source, mode ? bytes : c.size) == XUI_OK);
        if (native) {
            char* wire; uint64_t count; xui_document loaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSerialize(before, &wire, &count) == XUI_OK &&
                xuiDocumentDeserialize(NULL, wire, count, &loaded) == XUI_OK);
            xuiDocumentFreeBuffer(wire); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = loaded;
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        if (mode == 1) {
            xui_doc_txn_desc_t td = {0}; xui_document_transaction t; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK && xuiDocumentTxnReplaceSource(t, c.body, bytes, c.source + c.body, c.size - c.body) == XUI_OK &&
                xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        } else if (mode >= 2) {
            xui_document_prepare p, next; xui_doc_source_patch_t edit = patch(c.body, bytes, c.source + c.body, c.size - c.body);
            CHECK(xuiDocumentPrepareSource(d, NULL, &edit, 1, &p) == XUI_OK && xuiDocumentPrepareRun(p) == XUI_OK);
            if (mode == 3) {
                uint64_t at = (uint64_t)(strstr(initial, "prior") - initial); edit = patch(at, at + 5, "later", 5);
                CHECK(xuiDocumentPrepareContinueSource(d, p, &edit, 1, &next) == XUI_OK); xuiDocumentPrepareRelease(p); p = next;
                memcpy(c.source + at, "later", 5); CHECK(xuiDocumentPrepareRun(p) == XUI_OK);
            }
            CHECK(xuiDocumentPreparePublish(d, p, NULL) == XUI_OK); xuiDocumentPrepareRelease(p);
        }
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); golden(after, &c);
        CHECK(xuiDocumentCreate(&desc, &oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle, c.source, c.size) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK); inc_snapshot_equal(after, expected);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
        if (mode) {
            CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            inc_snapshot_equal(before, restored); expect_identity_tree(before, restored, 1); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            inc_snapshot_equal(after, restored); expect_identity_tree(after, restored, 1); xuiDocumentSnapshotRelease(restored);
        }
        xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); golden(after, &c); xuiDocumentSnapshotRelease(after); cases++;
    }
    printf("Inline whitespace golden: %u independent semantic/source/position cases; HTML/code/trim/Tab/native/SOURCE/Prepare/Continue/history/lifetime passed\n", cases);
}
static void boundaries(void)
{
    const char* edits[] = {"", "\t", "`", "\0"}; const uint64_t sizes[] = {0, 1, 1, 1};
    unsigned fixture, eol, edit, cases = 0; uint64_t incremental = 0;
    for (fixture = 0; fixture < sizeof(fixtures) / sizeof(*fixtures); fixture++) for (eol = 0; eol < 3; eol++) {
        whitespace_case c; uint64_t at; build_case(&c, fixture, eol, 0);
        for (at = c.body; at <= c.size; at++) for (edit = 0; edit < 4; edit++) {
            char result[4096]; uint64_t removed = edit ? 0 : at < c.size ? 1 : 0, bytes = c.size - removed + sizes[edit];
            xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t td = {0}; xui_document d, oracle; xui_document_transaction t;
            xui_document_snapshot before, after, expected, restored;
            memcpy(result, c.source, at); memcpy(result + at, edits[edit], sizes[edit]); memcpy(result + at + sizes[edit], c.source + at + removed, c.size - at - removed);
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, c.source, c.size) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK && xuiDocumentTxnReplaceSource(t, at, at + removed, edits[edit], sizes[edit]) == XUI_OK &&
                xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); incremental += inc_stats(d).iMarkdownIncrementalParses;
            CHECK(xuiDocumentCreate(&desc, &oracle) == XUI_OK && xuiDocumentLoadMarkdown(oracle, result, bytes) == XUI_OK &&
                xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK); inc_snapshot_equal(after, expected);
            xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
            if (xuiDocumentCanUndo(d)) {
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                inc_snapshot_equal(before, restored); expect_identity_tree(before, restored, 1); xuiDocumentSnapshotRelease(restored);
                CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
                inc_snapshot_equal(after, restored); expect_identity_tree(after, restored, 1); xuiDocumentSnapshotRelease(restored);
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); cases++;
        }
    }
    printf("Inline whitespace differential: %u cases, %llu incremental; byte edits, NUL, full semantic/source oracle and history passed\n", cases, (unsigned long long)incremental);
}
static void faults(void)
{
    const unsigned selected[] = {1, 16, 24, 25, 31}; unsigned f, mode; long oom = 0, cancellations = 0;
    for (f = 0; f < sizeof(selected) / sizeof(*selected); f++) for (mode = 0; mode < 2; mode++) {
        whitespace_case c; char initial[2048]; uint64_t bytes; long point; int success = 0;
        build_case(&c, selected[f], 1, 0); memcpy(initial, c.source, c.body); strcpy(initial + c.body, "z placeholder\n"); bytes = c.body + 14;
        for (point = 0; point < 4096 && !success; point++) {
            fail_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_snapshot before, after; uint64_t revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = failing_alloc; desc.onFree = failing_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, initial, bytes) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
            revision = xuiDocumentGetRevision(d); a.remaining = point;
            if (mode) {
                xui_document_prepare p = NULL; xui_doc_source_patch_t edit = patch(c.body, bytes, c.source + c.body, c.size - c.body);
                result = xuiDocumentPrepareSource(d, NULL, &edit, 1, &p);
                if (result == XUI_OK) result = xuiDocumentPrepareRun(p);
                if (result == XUI_OK) result = xuiDocumentPreparePublish(d, p, NULL);
                a.remaining = -1; xuiDocumentPrepareRelease(p);
            } else {
                xui_document_transaction t = NULL; xui_doc_txn_desc_t td = {0}; td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
                result = xuiDocumentBeginTransaction(d, &td, &t);
                if (result == XUI_OK) result = xuiDocumentTxnReplaceSource(t, c.body, bytes, c.source + c.body, c.size - c.body);
                if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
                a.remaining = -1; xuiDocumentTxnRelease(t);
            }
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result != XUI_OK) {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) &&
                    !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                inc_snapshot_equal(before, after); expect_identity_tree(before, after, 1); oom++;
            } else { golden(after, &c); success = 1; }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success);
    }
    for (f = 0; f < sizeof(selected) / sizeof(*selected); f++) {
        whitespace_case c; char initial[2048]; uint64_t bytes; long point; int success = 0;
        build_case(&c, selected[f], 1, 0); memcpy(initial, c.source, c.body); strcpy(initial + c.body, "z placeholder\n"); bytes = c.body + 14;
        for (point = 0; point < 4096 && !success; point++) {
            prepare_cancel_allocator a = {0, -1, 0, NULL}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_prepare p; xui_document_snapshot before, after; uint64_t revision; int result;
            xui_doc_source_patch_t edit = patch(c.body, bytes, c.source + c.body, c.size - c.body);
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
            desc.onAlloc = prepare_cancel_alloc; desc.onFree = prepare_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, initial, bytes) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentMarkSaved(d, before) == XUI_OK);
            revision = xuiDocumentGetRevision(d); CHECK(xuiDocumentPrepareSource(d, NULL, &edit, 1, &p) == XUI_OK);
            a.prepare = p; a.remaining = point; result = xuiDocumentPrepareRun(p); a.prepare = NULL;
            if (a.fired) {
                CHECK(result == XUI_DOC_ERROR_CANCELLED && xuiDocumentPreparePublish(d, p, NULL) == XUI_DOC_ERROR_CANCELLED &&
                    xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); inc_snapshot_equal(before, after); expect_identity_tree(before, after, 1);
                xuiDocumentSnapshotRelease(after); cancellations++;
            } else {
                CHECK(result == XUI_OK && xuiDocumentPreparePublish(d, p, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
                golden(after, &c); xuiDocumentSnapshotRelease(after); success = 1;
            }
            xuiDocumentPrepareRelease(p); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success);
    }
    printf("Inline whitespace faults: %ld SOURCE/Prepare OOM, %ld Prepare cancellation checkpoints; HTML/code/trim/partial-Tab/NUL state/identity/history/savepoint and zero leaks passed\n", oom, cancellations);
}
int main(void) { matrix(); boundaries(); faults(); return 0; }
