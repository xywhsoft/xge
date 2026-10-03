#include "../xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CHECK(e) do { if (!(e)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); exit(1); } } while (0)

static double now_ms(void)
{
    return 1000.0 * (double)clock() / CLOCKS_PER_SEC;
}
static int compare_double(const void* a, const void* b)
{
    double x = *(const double*)a, y = *(const double*)b;
    return (x > y) - (x < y);
}
int main(int argc, char** argv)
{
    unsigned blocks = argc > 1 ? (unsigned)strtoul(argv[1], NULL, 10) : 100000;
    size_t capacity = (size_t)blocks * 140 + 1024, used = 0;
    char* source = malloc(capacity);
    xui_doc_desc_t desc = {0}; xui_doc_txn_desc_t tx = {0};
    xui_document d; xui_document_transaction t;
    xui_doc_stats_t before = {0}, after = {0};
    double durations[65], began, loaded; uint64_t at = 0;
    unsigned i, changed = 0;
    int length;
    CHECK(source != NULL);
    length = snprintf(source, capacity, "alpha [ref] note[^n]\n\n[ref]: /u\n\n");
    CHECK(length > 0 && (size_t)length < capacity); used = (size_t)length;
    for (i = 0; i < blocks; i++) {
        if (i == blocks / 2) {
            length = snprintf(source + used, capacity - used, "[^n]: **bold** [ref] body\n\n");
            CHECK(length > 0 && (size_t)length < capacity - used);
            at = used + (uint64_t)(strstr(source + used, "body") - (source + used));
            used += (size_t)length;
        }
        length = snprintf(source + used, capacity - used,
            "padding block %06u with **bold** and [ref] plus ordinary text to make the document roughly ten megabytes.\n\n", i);
        CHECK(length > 0 && (size_t)length < capacity - used);
        used += (size_t)length;
    }
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = XUI_MD_EXTENDED; desc.bDisableHistory = 1;
    tx.iSize = sizeof(tx); tx.iDomain = XUI_DOC_SOURCE;
    began = now_ms();
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(d, source, used) == XUI_OK);
    loaded = now_ms() - began;
    before.iSize = after.iSize = sizeof(before);
    CHECK(xuiDocumentGetStats(d, &before) == XUI_OK);
    if (argc > 2 && strcmp(argv[2], "prepare-chain") == 0) {
        xui_doc_source_patch_t patches[2] = {{0}};
        xui_document_prepare p;
        patches[0].iSize = patches[1].iSize = sizeof(patches[0]);
        patches[0].iStart = at; patches[0].iEnd = at + 4;
        patches[0].sText = "longer"; patches[0].iTextBytes = 6;
        patches[1].iStart = at; patches[1].iEnd = at + 6;
        patches[1].sText = "change"; patches[1].iTextBytes = 6;
        began = now_ms();
        CHECK(xuiDocumentPrepareSource(d, NULL, patches, 2, &p) == XUI_OK);
        CHECK(xuiDocumentPrepareRun(p) == XUI_OK &&
            xuiDocumentPreparePublish(d, p, NULL) == XUI_OK);
        xuiDocumentPrepareRelease(p);
        CHECK(xuiDocumentGetStats(d, &after) == XUI_OK);
        CHECK(after.iMarkdownIncrementalParses - before.iMarkdownIncrementalParses == 1 &&
            after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 128);
        printf("prepare-chain source=%zu blocks=%u load=%.2f ms edit=%.2f ms local=%llu/1 parsed=%llu bytes\n",
            used, blocks, loaded, now_ms() - began,
            (unsigned long long)(after.iMarkdownIncrementalParses - before.iMarkdownIncrementalParses),
            (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
        xuiDocumentRelease(d); free(source); return 0;
    }
    if (argc > 2 && strcmp(argv[2], "chain") == 0) {
        began = now_ms();
        CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceSource(t, at, at + 4, "longer", 6) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceSource(t, at, at + 6, "change", 6) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentGetStats(d, &after) == XUI_OK);
        CHECK(after.iMarkdownIncrementalParses - before.iMarkdownIncrementalParses == 2 &&
            after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 256);
        printf("chain source=%zu blocks=%u load=%.2f ms edit=%.2f ms local=%llu/2 parsed=%llu bytes\n",
            used, blocks, loaded, now_ms() - began,
            (unsigned long long)(after.iMarkdownIncrementalParses - before.iMarkdownIncrementalParses),
            (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes));
        xuiDocumentRelease(d); free(source); return 0;
    }
    for (i = 0; i < 65; i++) {
        const char* next = changed ? "body" : "longer body";
        uint64_t old_length = changed ? 11 : 4;
        uint64_t new_length = changed ? 4 : 11;
        began = now_ms();
        CHECK(xuiDocumentBeginTransaction(d, &tx, &t) == XUI_OK);
        CHECK(xuiDocumentTxnReplaceSource(t, at, at + old_length,
            next, new_length) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t);
        durations[i] = now_ms() - began;
        changed = !changed;
    }
    CHECK(xuiDocumentGetStats(d, &after) == XUI_OK);
    CHECK(after.iMarkdownIncrementalParses - before.iMarkdownIncrementalParses == 65 &&
        after.iMarkdownParsedBytes - before.iMarkdownParsedBytes < 8192 &&
        after.iCurrentBytes < before.iCurrentBytes + 65536);
    qsort(durations + 1, 64, sizeof(double), compare_double);
    printf("source=%zu blocks=%u load=%.2f ms first=%.2f ms P50=%.2f ms P95=%.2f ms max=%.2f ms local=%llu/%u parsed=%llu bytes current_delta=%+lld\n",
        used, blocks, loaded, durations[0], durations[33], durations[61], durations[64],
        (unsigned long long)(after.iMarkdownIncrementalParses - before.iMarkdownIncrementalParses), 65,
        (unsigned long long)(after.iMarkdownParsedBytes - before.iMarkdownParsedBytes),
        (long long)after.iCurrentBytes - (long long)before.iCurrentBytes);
    xuiDocumentRelease(d); free(source); return 0;
}
