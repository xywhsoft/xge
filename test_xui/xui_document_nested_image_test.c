#define main xui_document_core_uninvoked
#include "xui_document_test.c"
#undef main

typedef struct image_fixture {
    const char* source;
    const char* alt;
    const char* title;
    int linked, images;
} image_fixture;
static const image_fixture fixtures[] = {
    {"![foo ![bar](in)](out)\n", "foo bar", "", 0, 1},
    {"![![bar](in)](out)\n", "bar", "", 0, 1},
    {"![![](in)](out)\n", "", "", 0, 1},
    {"![foo ![bar](in \"inner\") baz](out \"outer\")\n", "foo bar baz", "outer", 0, 1},
    {"[![foo ![bar](in)](out)](target \"tip\")\n", "foo bar", "", 1, 1},
    {"![foo ![bar ![baz](third)](in)](out)\n", "foo bar baz", "", 0, 1},
    {"![foo ![bar &amp; baz](in)](out)\n", "foo bar & baz", "", 0, 1},
    {"![foo ![bar \\*baz\\*](in)](out)\n", "foo bar *baz*", "", 0, 1},
    {"![foo ![**bar** *baz*](in)](out)\n", "foo bar baz", "", 0, 1},
    {"![foo ![`bar\n baz`](in)](out)\n", "foo bar  baz", "", 0, 1},
    {"![foo ![bar][inner]][outer]\n\n[inner]: in \"inner title\"\n[outer]: out\n", "foo bar", "", 0, 1},
    {"![foo ![[bar](target)](in)](out)\n", "foo bar", "", 0, 1},
    {"![foo ![](in) baz](out)\n", "foo  baz", "", 0, 1},
    {"![foo ![bar](in)](out) ![qux](second)\n", "foo bar", "", 0, 2}
};
typedef struct image_case { char source[4096]; uint64_t size, body; const image_fixture* fixture; } image_case;
static void build(image_case* c, unsigned fixture, unsigned eol, unsigned container, int native)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    const char* head = native ? "\xef\xbb\xbf---\rname: image\r\n...\n\nprior `span`\n\n" : "prior `span`\n\n";
    const char *p = fixtures[fixture].source, *line = p;
    size_t used = strlen(head); unsigned index = 0;
    memset(c, 0, sizeof(*c)); memcpy(c->source, head, used); c->body = used; c->fixture = &fixtures[fixture];
    while (*p) {
        const char* prefix = container == 1 ? "> " : container == 2 ? (index ? "  " : "- ") : "";
        while (*p && *p != '\n') p++;
        memcpy(c->source + used, prefix, strlen(prefix)); used += strlen(prefix);
        memcpy(c->source + used, line, p - line); used += (size_t)(p - line);
        memcpy(c->source + used, endings[eol], strlen(endings[eol])); used += strlen(endings[eol]);
        if (*p) p++;
        line = p; index++;
    }
    strcpy(c->source + used, "\nfollowing  &amp; untouched  \n");
    c->size = strlen(c->source);
}
static uint64_t images(xui_document_snapshot s, uint64_t id, uint64_t* first)
{
    xui_doc_node_info_t n = {0}; uint64_t i, child, count = 0; n.iSize = sizeof(n);
    CHECK(xuiDocumentSnapshotGetNode(s, id, &n) == XUI_OK);
    if (n.iKind == XUI_DOC_IMAGE) { if (!*first) *first = id; count++; }
    for (i = 0; i < n.iChildCount; i++) { CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK); count += images(s, child, first); }
    return count;
}
static uint64_t golden(xui_document_snapshot s, const image_case* c)
{
    xui_doc_node_info_t n = {0}; char raw[4096], text[512]; uint64_t id = 0, bytes, i, count;
    CHECK(xuiDocumentSnapshotCopySource(s, raw, sizeof(raw), &bytes) == XUI_OK && bytes == c->size && !memcmp(raw, c->source, bytes));
    count = images(s, 1, &id);
    if (count != (uint64_t)c->fixture->images) fprintf(stderr, "nested image fixture %llu: objects %llu expected %d\n",
        (unsigned long long)(c->fixture - fixtures), (unsigned long long)count, c->fixture->images);
    CHECK(count == (uint64_t)c->fixture->images && id);
    n.iSize = sizeof(n); CHECK(xuiDocumentSnapshotGetNode(s, id, &n) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopyText(s, id, text, sizeof(text), &bytes) == XUI_OK);
    if (bytes != strlen(c->fixture->alt) || memcmp(text, c->fixture->alt, bytes)) fprintf(stderr, "nested image fixture %llu: alt '%s' expected '%s'\n",
        (unsigned long long)(c->fixture - fixtures), text, c->fixture->alt);
    CHECK(bytes == strlen(c->fixture->alt) && !memcmp(text, c->fixture->alt, bytes));
    CHECK(n.sResource && !strcmp(n.sResource, "out") && !strcmp(n.sTitle ? n.sTitle : "", c->fixture->title));
    CHECK(!!(n.tAttributes.iMarks & XUI_DOC_LINK) == c->fixture->linked);
    if (c->fixture->linked) CHECK(n.sLinkTarget && !strcmp(n.sLinkTarget, "target") && n.sLinkTitle && !strcmp(n.sLinkTitle, "tip"));
    for (i = 0; i < n.iSourceSegmentCount; i++) {
        xui_doc_source_segment_t seg = {0}; seg.iSize = sizeof(seg);
        CHECK(xuiDocumentSnapshotGetSourceSegment(s, id, i, &seg) == XUI_OK && seg.iSourceEnd <= c->size && seg.iTextEnd <= bytes);
        if (seg.iKind == XUI_DOC_SOURCE_DIRECT) {
            CHECK(seg.iSourceEnd - seg.iSourceStart == seg.iTextEnd - seg.iTextStart &&
                !memcmp(c->source + seg.iSourceStart, text + seg.iTextStart, seg.iTextEnd - seg.iTextStart));
            if (seg.iTextEnd > seg.iTextStart + 1) expect_source_position(s, seg.iSourceStart + 1, XUI_DOC_BEFORE,
                id, seg.iTextStart + 1, XUI_DOC_MAP_EXACT);
        }
    }
    return id;
}
static xui_doc_source_patch_t patch(uint64_t start, uint64_t end, const char* text, uint64_t bytes)
{
    xui_doc_source_patch_t p = {0}; p.iSize = sizeof(p); p.iStart = start; p.iEnd = end; p.sText = text; p.iTextBytes = bytes; return p;
}
static void matrix(void)
{
    unsigned fixture, eol, container, dialect, native, mode, cases = 0;
    for (fixture = 0; fixture < sizeof(fixtures) / sizeof(*fixtures); fixture++) for (eol = 0; eol < 3; eol++) for (container = 0; container < 3; container++)
    for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) for (native = 0; native < 2; native++) for (mode = 0; mode < 4; mode++) {
        image_case c; char initial[4096]; uint64_t bytes; xui_doc_desc_t desc = {0};
        xui_document d, oracle; xui_document_snapshot before, after, expected, restored;
        build(&c, fixture, eol, container, native); memcpy(initial, c.source, c.body); strcpy(initial + c.body, "placeholder\n"); bytes = c.body + 12;
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
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        inc_snapshot_equal(after, expected); xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
        if (mode) {
            CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            inc_snapshot_equal(before, restored); expect_identity_tree(before, restored, 1); xuiDocumentSnapshotRelease(restored);
            CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            inc_snapshot_equal(after, restored); expect_identity_tree(after, restored, 1); xuiDocumentSnapshotRelease(restored);
        }
        xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); golden(after, &c); xuiDocumentSnapshotRelease(after); cases++;
    }
    printf("Nested image golden: %u exact-source/alt/metadata/source-position cases with load, SOURCE, Prepare/Continue, native, retained snapshots and Undo/Redo.\n", cases);
}
static void image_update(void)
{
    const char* sources[] = {"__outside__  ![![bar](in)](out)  \\! tail\r\n", "__outside__  ![![](in)](out)  \\! tail\r\n",
        "__outside__  [![![bar](in)](out)](target \"tip\")  \\! tail\r\n", "__outside__  [![![](in)](out)](target \"tip\")  \\! tail\r\n"};
    unsigned fixture, alt, container, native, dialect, cases = 0;
    for (fixture = 0; fixture < 4; fixture++) for (alt = 0; alt < 2; alt++) for (container = 0; container < 3; container++)
    for (native = 0; native < 2; native++) for (dialect = XUI_MD_COMMONMARK; dialect <= XUI_MD_EXTENDED; dialect++) {
        char raw[1024], expected[1024], actual[1024]; const char* old = raw;
        const char* head = native ? "\xef\xbb\xbf---\rname: edit\r\n...\n\nprior `span`\n\n" : "prior `span`\n\n";
        const char* prefix = container == 1 ? "> " : container == 2 ? "- " : "";
        const char *start, *end;
        xui_doc_desc_t desc = {0};
        snprintf(raw, sizeof(raw), "%s%s%s", head, prefix, sources[fixture]);
        start = strstr(old, "!["); end = strstr(old, "](out)") + 6;
        xui_document d; xui_document_snapshot before, after, restored; xui_document_transaction t; xui_doc_image_desc_t image = {0}; uint64_t id = 0, bytes;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = dialect;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, old, strlen(old)) == XUI_OK &&
            xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && images(before, 1, &id) == 1);
        image.iSize = sizeof(image); image.sResource = alt ? "out" : "new";
        image.sAlt = alt ? "replacement" : fixture & 1 ? "" : "bar"; image.iAltBytes = strlen(image.sAlt);
        image.sTitle = ""; image.sLinkTarget = fixture >= 2 ? "target" : NULL; image.sLinkTitle = fixture >= 2 ? "tip" : NULL;
        if (alt)
            snprintf(expected, sizeof(expected), "%.*s![%s](%s)%s", (int)(start - old), old, image.sAlt, image.sResource, end);
        else {
            const char* suffix = strstr(start, "](out)") + 1;
            snprintf(expected, sizeof(expected), "%.*s(<%s>)%s", (int)(suffix - old), old, image.sResource, end);
        }
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnUpdateImage(t, id, &image) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t); CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        CHECK(xuiDocumentSnapshotCopySource(after, actual, sizeof(actual), &bytes) == XUI_OK);
        if (strcmp(actual, expected)) fprintf(stderr, "nested image update: source '%s' expected '%s'\n", actual, expected);
        CHECK(!strcmp(actual, expected));
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(before, restored); expect_identity_tree(before, restored, 1); xuiDocumentSnapshotRelease(restored);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        inc_snapshot_equal(after, restored); expect_identity_tree(after, restored, 1); xuiDocumentSnapshotRelease(restored);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); cases++;
    }
    printf("Nested image update: %u alt/destination changes preserve surrounding bytes and enclosing links, quote/list/native/dialects and Undo/Redo identity.\n", cases);
}
static void boundaries(void)
{
    const char* edits[] = {"", "\t", "`", "\0"}; const uint64_t sizes[] = {0, 1, 1, 1};
    unsigned fixture, eol, edit, cases = 0; uint64_t incremental = 0;
    for (fixture = 0; fixture < sizeof(fixtures) / sizeof(*fixtures); fixture++) for (eol = 0; eol < 3; eol++) {
        image_case c; uint64_t at; build(&c, fixture, eol, 0, 0);
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
    printf("Nested image differential: %u cases, %llu incremental; byte edits, NUL, full semantic/source oracle and history passed\n", cases, (unsigned long long)incremental);
}
static void faults(void)
{
    const unsigned selected[] = {1, 2, 4, 5, 10}; unsigned f, mode; long oom = 0, cancellations = 0;
    for (f = 0; f < sizeof(selected) / sizeof(*selected); f++) for (mode = 0; mode < 2; mode++) {
        image_case c; char initial[2048]; uint64_t bytes; long point; int success = 0;
        build(&c, selected[f], 1, 1, 0); memcpy(initial, c.source, c.body); strcpy(initial + c.body, "placeholder\n"); bytes = c.body + 12;
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
        image_case c; char initial[2048]; uint64_t bytes; long point; int success = 0;
        build(&c, selected[f], 1, 1, 0); memcpy(initial, c.source, c.body); strcpy(initial + c.body, "placeholder\n"); bytes = c.body + 12;
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
    printf("Nested image faults: %ld SOURCE/Prepare OOM, %ld Prepare cancellation checkpoints; nested/empty/linked/reference image state/identity/history/savepoint and zero leaks passed\n", oom, cancellations);
}
int main(void)
{
    matrix(); image_update(); boundaries(); faults(); return 0;
}
