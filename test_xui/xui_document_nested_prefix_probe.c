#include "../xui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(condition) do { if (!(condition)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); exit(1); } } while (0)
static xui_document test_markdown_open(const char* source)
{
    xui_doc_desc_t desc = {0}; xui_document document;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK && xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK);
    return document;
}
static xui_doc_range_t test_find(xui_document document, const char* text)
{
    xui_document_snapshot snapshot; xui_doc_range_t range; uint64_t count;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, text, strlen(text), NULL, &range, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot); return range;
}
static void test_source(xui_document document, const char* expected, int equal)
{
    xui_document_snapshot snapshot; char source[2048]; uint64_t bytes; (void)equal;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotCopySource(snapshot, source, sizeof(source), &bytes) == XUI_OK);
    if (strcmp(source, expected)) fprintf(stderr, "expected=[%s]\nactual=[%s]\n", expected, source);
    CHECK(!strcmp(source, expected)); xuiDocumentSnapshotRelease(snapshot);
}
#define NESTED_PREFIX_BASELINE_ONLY
#include "xui_document_nested_prefix_cases.h"
int main(void) { markdown_nested_prefix_patch(); return 0; }
