#include "../xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #c); exit(1); } } while (0)
static xstrview view(const char* s) { xstrview v = {s, strlen(s)}; return v; }
static void corpus(const char* path)
{
    FILE* file = fopen(path, "rb"); long length; char* json; xvalue* cases; size_t i, count;
    CHECK(file); CHECK(fseek(file, 0, SEEK_END) == 0); length = ftell(file); CHECK(length >= 0);
    rewind(file); json = malloc((size_t)length + 1); CHECK(json);
    CHECK(fread(json, 1, (size_t)length, file) == (size_t)length); fclose(file); json[length] = 0;
    cases = xrtJsonParse(view(json)); free(json); CHECK(cases && xrtValueIs(cases, XVALUE_ARRAY)); count = xrtValueCount(cases);
    for (i = 0; i < count; i++) {
        xstrview md; xui_doc_desc_t desc = {0}; xui_document d, loaded; xui_document_snapshot s, next;
        char *source, *native; uint64_t bytes, native_bytes; int result;
        CHECK(xrtValueGetString(xrtValueObjectGet(xrtValueArrayGet(cases, i), view("markdown")), &md));
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_COMMONMARK;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        result = xuiDocumentLoadMarkdown(d, md.Data, md.Size);
        if (result != XUI_OK) fprintf(stderr, "Corpus example %llu failed: %d\n", (unsigned long long)i + 1, result);
        CHECK(result == XUI_OK); CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
        source = malloc(md.Size + 1); CHECK(source);
        CHECK(xuiDocumentSnapshotCopySource(s, source, md.Size + 1, &bytes) == XUI_OK && bytes == md.Size && !memcmp(source, md.Data, md.Size));
        CHECK(xuiDocumentSerialize(s, &native, &native_bytes) == XUI_OK);
        CHECK(xuiDocumentDeserialize(NULL, native, native_bytes, &loaded) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(loaded, &next) == XUI_OK);
        CHECK(xuiDocumentSnapshotCopySource(next, source, md.Size + 1, &bytes) == XUI_OK && bytes == md.Size && !memcmp(source, md.Data, md.Size));
        free(source); xuiDocumentFreeBuffer(native); xuiDocumentSnapshotRelease(next); xuiDocumentRelease(loaded);
        xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    }
    xrtValueRelease(cases);
    printf("CommonMark 0.31.2 corpus: %llu parse/source/native-roundtrip cases passed (not HTML conformance).\n", (unsigned long long)count);
}
static uint64_t add(xui_document_transaction t, uint64_t parent, uint32_t kind, const char* text)
{
    xui_doc_node_desc_t n = {0}; uint64_t id;
    n.iSize = sizeof(n); n.iKind = kind; n.sText = text; n.iTextBytes = text ? strlen(text) : 0;
    CHECK(xuiDocumentTxnInsertNode(t, parent, XUI_DOCUMENT_APPEND, &n, &id) == XUI_OK); return id;
}
static int compare_time(const void* a, const void* b)
{
    double x = *(const double*)a, y = *(const double*)b; return x < y ? -1 : x != y;
}
static void percentiles(const char* name, double* samples, size_t count)
{
    qsort(samples, count, sizeof(*samples), compare_time);
    printf("%s, %llu measured edits after warmup: P50 %.3f ms, P95 %.3f ms, max %.3f ms.\n", name,
        (unsigned long long)count, samples[count / 2], samples[(count * 95 + 99) / 100 - 1], samples[count - 1]);
}
static void scale(void)
{
    xui_document d; xui_document_transaction t; xui_doc_stats_t before = {0}, after = {0};
    uint64_t id = 0, i; clock_t begin; double elapsed, samples[64];
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    begin = clock();
    for (i = 0; i < 10000; i++) { uint64_t p = add(t, 1, XUI_DOC_PARAGRAPH, NULL); id = add(t, p, XUI_DOC_TEXT, "paragraph"); }
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    elapsed = 1000.0 * (clock() - begin) / CLOCKS_PER_SEC;
    before.iSize = after.iSize = sizeof(before); CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
    begin = clock(); CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(t, id, 9, 9, " edited", 7) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentGetStats(d, &after) == XUI_OK);
    CHECK(after.iAllocations - before.iAllocations < 100 && after.iLiveBytes - before.iLiveBytes < 65536);
    printf("Rich 10000 paragraphs: build %.2f ms, local commit %.2f ms, %llu allocations, %llu retained bytes.\n",
        elapsed, 1000.0 * (clock() - begin) / CLOCKS_PER_SEC, (unsigned long long)(after.iAllocations - before.iAllocations),
        (unsigned long long)(after.iLiveBytes - before.iLiveBytes));
    for (i = 0; i < 68; i++) {
        uint64_t started = xrtClock();
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceText(t, id, 0, 1, i & 1 ? "p" : "P", 1) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        if (i >= 4) samples[i - 4] = (double)(xrtClock() - started) / 1000.0;
    }
    percentiles("Rich 10000 paragraphs", samples, 64); xuiDocumentRelease(d);
    {
        const char* block = "## Heading\n\nParagraph with **bold**, [a link](https://example.org/) and ordinary text for a Markdown document.\n\n";
        size_t block_size = strlen(block), count = (100 * 1024 + block_size - 1) / block_size, size = block_size * count;
        char* source = malloc(size + 1); xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t td = {0};
        CHECK(source); for (i = 0; i < count; i++) memcpy(source + i * block_size, block, block_size); source[size] = 0;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        begin = clock(); CHECK(xuiDocumentLoadMarkdown(d, source, size) == XUI_OK);
        elapsed = 1000.0 * (clock() - begin) / CLOCKS_PER_SEC;
        td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
        CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
        begin = clock(); CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceSource(t, 3, 4, "h", 1) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentGetStats(d, &after) == XUI_OK);
        printf("Markdown %llu bytes: load %.2f ms, edit %.2f ms, %llu allocations (full parse path).\n", (unsigned long long)size,
            elapsed, 1000.0 * (clock() - begin) / CLOCKS_PER_SEC, (unsigned long long)(after.iAllocations - before.iAllocations));
        for (i = 0; i < 68; i++) {
            uint64_t started = xrtClock();
            CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK);
            CHECK(xuiDocumentTxnReplaceSource(t, 3, 4, i & 1 ? "h" : "H", 1) == XUI_OK);
            CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
            if (i >= 4) samples[i - 4] = (double)(xrtClock() - started) / 1000.0;
        }
        percentiles("Markdown 102480 bytes (full parse)", samples, 64);
        free(source); xuiDocumentRelease(d);
    }
}
int main(int argc, char** argv)
{
    if (argc > 1) corpus(argv[1]);
    scale(); return 0;
}
