#include <math.h>

static xui_draw_text_proc projection_draw_text;
static uint64_t projection_draw_hash;
static void projection_hash_bytes(const void* data, size_t bytes)
{
    const unsigned char* p = data;
    while (bytes--) { projection_draw_hash ^= *p++; projection_draw_hash *= UINT64_C(1099511628211); }
}
static int projection_capture_text(xui_proxy proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t rect, uint32_t color, uint32_t flags)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

    projection_hash_bytes(text, strlen(text) + 1); projection_hash_bytes(&rect, sizeof(rect));
    projection_hash_bytes(&color, sizeof(color)); projection_hash_bytes(&flags, sizeof(flags));
    return projection_draw_text(proxy, draw, pTextItem, rect, color, flags);
}
static void projection_same_rect(xui_doc_rect_t a, xui_doc_rect_t b)
{
    CHECK(fabs(a.x - b.x) < .00001 && fabs(a.y - b.y) < .00001 &&
        fabs(a.width - b.width) < .00001 && fabs(a.height - b.height) < .00001);
}
static int projection_boundary(const char* source, size_t offset)
{
    return ((unsigned char)source[offset] & 0xc0) != 0x80;
}
/* The oracle is loaded independently and starts with a full source directory.
 * Compare every legal position, both affinities, hit tests and actual text
 * draw calls. No internal renderer structures are inspected by this test. */
static void projection_compare(xui_context context, xui_test_proxy_state_t* proxy, xui_surface surface,
    xui_document_renderer r, xui_document_prepare input, const char* source, double width)
{
    xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot snapshot;
    xui_document_renderer full; xui_doc_rect_t a, b; xui_doc_position_t p, q, ah, bh;
    xui_doc_renderer_stats_t as = {0}, bs = {0}; xui_draw_context draw; uint64_t hash; size_t i; unsigned affinity;
    int exact; desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK && xuiDocumentRendererCreate(context, NULL, &full) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(full, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiDocumentRendererSetSnapshot(full, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererLayout(r, width, 0, 1000000) == XUI_OK && xuiDocumentRendererLayout(full, width, 0, 1000000) == XUI_OK);
    CHECK(xuiDocumentRendererGetSize(r, &a, &exact) == XUI_OK && exact);
    CHECK(xuiDocumentRendererGetSize(full, &b, &exact) == XUI_OK && exact); projection_same_rect(a, b);
    as.iSize = bs.iSize = sizeof(as);
    CHECK(xuiDocumentRendererGetStats(r, &as) == XUI_OK && xuiDocumentRendererGetStats(full, &bs) == XUI_OK);
    CHECK(as.iBlocks == bs.iBlocks && as.iSourceBlocks == bs.iSourceBlocks);
    for (i = 0; i <= strlen(source); i++) if (projection_boundary(source, i)) for (affinity = 0; affinity < 2; affinity++) {
        CHECK(xuiDocumentPrepareSourcePosition(input, i, affinity, &p) == XUI_OK);
        q = position(d, 1, i); q.iKind = XUI_DOC_POSITION_SOURCE; q.iAffinity = affinity;
        CHECK(xuiDocumentRendererGetCaretRect(r, &p, &a) == XUI_OK && xuiDocumentRendererGetCaretRect(full, &q, &b) == XUI_OK);
        projection_same_rect(a, b);
        CHECK(xuiDocumentRendererHitTest(r, a.x + .1, a.y + a.height / 2, &ah) == XUI_OK);
        CHECK(xuiDocumentRendererHitTest(full, b.x + .1, b.y + b.height / 2, &bh) == XUI_OK);
        CHECK(ah.iOffset == bh.iOffset && ah.iKind == bh.iKind && ah.iInputGeneration == p.iInputGeneration);
    }
    CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, surface) == XUI_OK);
    projection_draw_hash = UINT64_C(14695981039346656037);
    CHECK(xuiDocumentRendererDraw(r, draw, 0, 0, (xui_rect_t){0, 0, (float)width, 1000000}, NULL, 0) == XUI_OK);
    hash = projection_draw_hash; projection_draw_hash = UINT64_C(14695981039346656037);
    CHECK(xuiDocumentRendererDraw(full, draw, 0, 0, (xui_rect_t){0, 0, (float)width, 1000000}, NULL, 0) == XUI_OK);
    CHECK(hash == projection_draw_hash && proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK);
    xuiDocumentRendererRelease(full); xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d);
}
static void projection_replace_model(char* source, size_t start, size_t end, const char* insert)
{
    size_t n = strlen(insert); memmove(source + start + n, source + end, strlen(source + end) + 1); memcpy(source + start, insert, n);
}
static unsigned projection_random(unsigned* state)
{
    *state = *state * 1664525u + 1013904223u; return *state;
}
static void source_projection_scale(xui_context context)
{
    unsigned mib;
    for (mib = 1; mib <= 10; mib += 9) {
        xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot snapshot; xui_document_renderer r;
        xui_document_prepare input, next; xui_doc_source_patch_t patch = {0}; xui_doc_renderer_stats_t before = {0}, after = {0};
        xui_doc_stats_t cb = {0}, ca = {0}; xui_doc_position_t p; xui_doc_rect_t caret;
        size_t bytes = (size_t)mib * 1024 * 1024, i; char* source = malloc(bytes); CHECK(source);
        memset(source, 'a', bytes); for (i = 511; i < bytes; i += 512) source[i] = '\n';
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "base\n", 5) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK && xuiDocumentRendererCreate(context, NULL, &r) == XUI_OK);
        CHECK(xuiDocumentRendererSetMode(r, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiDocumentRendererSetSnapshot(r, snapshot, NULL) == XUI_OK);
        patch.iSize = sizeof(patch); patch.iEnd = 5; patch.sText = source; patch.iTextBytes = bytes;
        CHECK(xuiDocumentPrepareSource(d, NULL, &patch, 1, &input) == XUI_OK && xuiDocumentRendererSetSourceInput(r, input) == XUI_OK);
        CHECK(xuiDocumentRendererLayout(r, 640, 0, 400) == XUI_OK);
        /* Materialize distant caches, so source offset adjustment includes
         * measured fragments/runs rather than only unmeasured directory rows. */
        CHECK(xuiDocumentPrepareSourcePosition(input, bytes / 2, XUI_DOC_AFTER, &p) == XUI_OK && xuiDocumentRendererGetCaretRect(r, &p, &caret) == XUI_OK);
        CHECK(xuiDocumentPrepareSourcePosition(input, bytes - 8, XUI_DOC_AFTER, &p) == XUI_OK && xuiDocumentRendererGetCaretRect(r, &p, &caret) == XUI_OK);
        before.iSize = after.iSize = sizeof(before); cb.iSize = ca.iSize = sizeof(cb);
        CHECK(xuiDocumentRendererGetStats(r, &before) == XUI_OK && xuiDocumentGetStats(d, &cb) == XUI_OK);
        for (i = 0; i < 128; i++) {
            size_t at = (i / 2) % 3 == 0 ? 3 : (i / 2) % 3 == 1 ? bytes / 2 + 3 : bytes - 1024 + 3;
            patch.iStart = at; patch.iEnd = i & 1 ? at + 2 : at; patch.sText = i & 1 ? "" : "x\n"; patch.iTextBytes = i & 1 ? 0 : 2;
            CHECK(xuiDocumentPrepareContinueSource(d, input, &patch, 1, &next) == XUI_OK && xuiDocumentRendererSetSourceInput(r, next) == XUI_OK);
            xuiDocumentPrepareRelease(input); input = next;
            CHECK(xuiDocumentRendererLayout(r, 640, 0, 400) == XUI_OK);
            CHECK(xuiDocumentPrepareSourcePosition(input, bytes - 8 + (i & 1 ? 0 : 2), XUI_DOC_AFTER, &p) == XUI_OK);
            CHECK(xuiDocumentRendererGetCaretRect(r, &p, &caret) == XUI_OK);
        }
        CHECK(xuiDocumentRendererGetStats(r, &after) == XUI_OK && after.iSourceIncrementalUpdates - before.iSourceIncrementalUpdates == 128);
        CHECK(after.iSourceBytesScanned - before.iSourceBytesScanned <= 128 * 4096);
        CHECK(after.iSourceRowsCreated - before.iSourceRowsCreated <= 128 * 8);
        CHECK(after.iShapedBytes - before.iShapedBytes <= 128 * 4096 && after.iSourceReusedBlocks - before.iSourceReusedBlocks >= 128 * (bytes / 512 - 4));
        CHECK(xuiDocumentGetStats(d, &ca) == XUI_OK && ca.iMarkdownParses == cb.iMarkdownParses);
        printf("Source projection %u MiB, 128 first/middle/tail newline edits: %llu scanned bytes, %llu new directory rows, %llu shaped bytes, %llu reused line blocks; no parser runs.\n", mib,
            (unsigned long long)(after.iSourceBytesScanned - before.iSourceBytesScanned),
            (unsigned long long)(after.iSourceRowsCreated - before.iSourceRowsCreated),
            (unsigned long long)(after.iShapedBytes - before.iShapedBytes), (unsigned long long)(after.iSourceReusedBlocks - before.iSourceReusedBlocks));
        CHECK(xuiDocumentCancelPrepare(d) == XUI_OK); xuiDocumentPrepareRelease(input);
        xuiDocumentRendererRelease(r); xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d); free(source);
    }
}
static void source_projection_cases_impl(xui_context context, xui_test_proxy_state_t* proxy, xui_surface surface)
{
    const char* base = "a\r\nb\rc\n\nz\r\n";
    const char* inserts[] = {"", "x", "\r", "\n", "\r\n", "\n\r", "\xe4\xb8\xad", "e\xcc\x81", "xx\n\n"};
    xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot snapshot; xui_document_renderer r;
    xui_document_prepare input = NULL, next; xui_doc_source_patch_t patch[2] = {{0}};
    xui_doc_renderer_stats_t before = {0}, after = {0}; xui_doc_stats_t cb = {0}, ca = {0};
    char model[1024]; size_t start, end, i, cases = 0; unsigned seed = 817123; int exact;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, base, strlen(base)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK && xuiDocumentRendererCreate(context, NULL, &r) == XUI_OK);
    CHECK(xuiDocumentRendererSetMode(r, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiDocumentRendererSetSnapshot(r, snapshot, NULL) == XUI_OK);
    before.iSize = after.iSize = sizeof(before); patch[0].iSize = patch[1].iSize = sizeof(*patch);
    cb.iSize = ca.iSize = sizeof(cb); CHECK(xuiDocumentGetStats(d, &cb) == XUI_OK);
    for (start = 0; start <= strlen(base); start++) for (end = start; end <= strlen(base); end++)
        for (i = 0; i < sizeof(inserts) / sizeof(*inserts); i++) {
            CHECK(xuiDocumentRendererSetSourceInput(r, NULL) == XUI_OK && xuiDocumentRendererLayout(r, 160, 0, 10000) == XUI_OK);
            xuiDocumentPrepareRelease(input); input = NULL; strcpy(model, base);
            patch[0].iStart = start; patch[0].iEnd = end; patch[0].sText = inserts[i]; patch[0].iTextBytes = strlen(inserts[i]);
            CHECK(xuiDocumentPrepareSource(d, NULL, patch, 1, &input) == XUI_OK);
            CHECK(xuiDocumentRendererGetStats(r, &before) == XUI_OK && xuiDocumentRendererSetSourceInput(r, input) == XUI_OK);
            CHECK(xuiDocumentRendererGetStats(r, &after) == XUI_OK && after.iSourceIncrementalUpdates == before.iSourceIncrementalUpdates + 1);
            projection_replace_model(model, start, end, inserts[i]); projection_compare(context, proxy, surface, r, input, model, 160); cases++;
        }
    /* Continue one immutable input chain; skip displaying some generations,
     * batch two patches, retry with no patch, and invalidate wrap caches. */
    for (i = 0; i < 600; i++) {
        const char* insert = inserts[projection_random(&seed) % (sizeof(inserts) / sizeof(*inserts))]; size_t n = strlen(model);
        unsigned count = i % 17 == 0 ? 2 : i % 23 == 0 ? 0 : 1; int skip = i % 19 == 0;
        start = projection_random(&seed) % (n + 1); while (!projection_boundary(model, start)) start--;
        end = start + projection_random(&seed) % (n - start + 1); while (!projection_boundary(model, end)) end++;
        patch[0].iStart = start; patch[0].iEnd = end; patch[0].sText = insert; patch[0].iTextBytes = strlen(insert);
        if (count) projection_replace_model(model, start, end, insert);
        patch[1].iStart = patch[1].iEnd = strlen(model); patch[1].sText = "\r\n"; patch[1].iTextBytes = 2;
        if (count == 2) strcat(model, "\r\n");
        CHECK(xuiDocumentPrepareContinueSource(d, input, patch, count, &next) == XUI_OK);
        xuiDocumentPrepareRelease(input); input = next;
        if (skip) {
            patch[0].iStart = patch[0].iEnd = 0; patch[0].sText = "!"; patch[0].iTextBytes = 1;
            CHECK(xuiDocumentPrepareContinueSource(d, input, patch, 1, &next) == XUI_OK);
            xuiDocumentPrepareRelease(input); input = next; projection_replace_model(model, 0, 0, "!");
        }
        CHECK(xuiDocumentRendererGetStats(r, &before) == XUI_OK && xuiDocumentRendererSetSourceInput(r, input) == XUI_OK);
        CHECK(xuiDocumentRendererGetStats(r, &after) == XUI_OK);
        CHECK(after.iSourceIncrementalUpdates == before.iSourceIncrementalUpdates + (count <= 1 && !skip));
        if (count > 1 || skip) CHECK(after.iSourceBytesScanned - before.iSourceBytesScanned == strlen(model));
        projection_compare(context, proxy, surface, r, input, model, i % 37 < 18 ? 80 : 320); cases++;
    }
    CHECK(xuiDocumentGetStats(d, &ca) == XUI_OK && ca.iMarkdownParses == cb.iMarkdownParses);
    CHECK(xuiDocumentPrepareRun(input) == XUI_OK && xuiDocumentPreparePublish(d, input, NULL) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot); CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentRendererGetStats(r, &before) == XUI_OK && xuiDocumentRendererSetSnapshot(r, snapshot, NULL) == XUI_OK);
    CHECK(xuiDocumentRendererGetStats(r, &after) == XUI_OK && after.iSourceBytesScanned == before.iSourceBytesScanned);
    CHECK(after.iShapedBytes == before.iShapedBytes && xuiDocumentRendererGetSize(r, &(xui_doc_rect_t){0}, &exact) == XUI_OK && exact);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK && xuiDocumentRendererSetSnapshot(r, snapshot, NULL) == XUI_OK);
    xuiDocumentPrepareRelease(input); xuiDocumentRendererRelease(r); xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d);
    source_projection_scale(context);
    printf("Source projection: %llu CR/LF/UTF-8 and continued/batched/skipped-generation edits match full layout/caret/hit/draw; no input parse, publication reuses layout, Undo passed.\n", (unsigned long long)cases);
}
static void source_projection_cases(void)
{
    xui_context context; xui_test_proxy_state_t proxy; xui_font font; xui_surface surface;
    xuiTestProxyInit(&proxy); projection_draw_text = proxy.tProxy.drawText; proxy.tProxy.drawText = projection_capture_text;
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&proxy, &surface, 640, 800, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    source_projection_cases_impl(context, &proxy, surface);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, surface); xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
}
