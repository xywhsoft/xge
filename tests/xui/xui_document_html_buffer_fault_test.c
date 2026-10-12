#include "../src/xui/document/xui_document_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HTML_CHECK(x) do { if (!(x)) { fprintf(stderr, "HTML buffer fault line %d: %s\n", __LINE__, #x); exit(1); } } while (0)

static long html_budget = -1;
static void* html_owned[32];
static unsigned html_live, html_attempts;
static void* html_fault_malloc(size_t bytes)
{
    unsigned i; void* p;
    html_attempts++;
    if (!html_budget) return NULL;
    if (html_budget > 0) html_budget--;
    p = malloc(bytes);
    HTML_CHECK(p);
    for (i = 0; i < 32 && html_owned[i]; i++) {}
    HTML_CHECK(i < 32); html_owned[i] = p; html_live++;
    return p;
}
static void html_fault_free(void* p)
{
    unsigned i;
    if (!p) return;
    for (i = 0; i < 32 && html_owned[i] != p; i++) {}
    HTML_CHECK(i < 32 && html_live); html_owned[i] = NULL; html_live--; free(p);
}

/* Compile the production exporter with a failing libc allocation boundary.
 * Renamed entry points let the normal kernel archive keep its own exporter. */
#define malloc html_fault_malloc
#define free html_fault_free
#define doc_html_safe_url html_fault_safe_url
#define doc_html_export_children html_fault_export_children
#define xuiDocumentExportHtml html_fault_export
#include "../src/xui/document/xui_document_html.c"
#undef xuiDocumentExportHtml
#undef doc_html_export_children
#undef doc_html_safe_url
#undef free
#undef malloc

static uint64_t html_insert(xui_document_transaction t, uint64_t parent, uint64_t index,
    unsigned kind, const char* text)
{
    xui_doc_node_desc_t n = {0}; uint64_t id;
    n.iSize = sizeof(n); n.iKind = kind; n.sText = text;
    n.iTextBytes = text ? strlen(text) : 0;
    if (kind == XUI_DOC_TEXT) n.tAttributes.iMarks = XUI_DOC_BOLD;
    HTML_CHECK(xuiDocumentTxnInsertNode(t, parent, index, &n, &id) == XUI_OK);
    return id;
}
static char* html_golden(int root)
{
    const char* open = root ? "<p><span><strong>" : "<span><strong>";
    const char* close = root ? "</strong></span></p>\n" : "</strong></span>";
    const char* unit = "a&amp;&lt;&gt;&quot;&#39;";
    size_t a = strlen(open), z = strlen(close), u = strlen(unit), i;
    char* result = malloc(a + 1024 * u + z + 1);
    HTML_CHECK(result); memcpy(result, open, a);
    for (i = 0; i < 1024; i++) memcpy(result + a + i * u, unit, u);
    memcpy(result + a + 1024 * u, close, z + 1); return result;
}
static void html_check_snapshot(xui_document d, xui_document_snapshot old,
    uint64_t paragraph, const uint64_t ids[2], uint64_t revision)
{
    unsigned i; uint64_t id; xui_doc_node_info_t n = {0};
    HTML_CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d));
    n.iSize = sizeof(n);
    HTML_CHECK(xuiDocumentSnapshotGetNode(old, paragraph, &n) == XUI_OK && n.iChildCount == 2);
    for (i = 0; i < 2; i++) {
        HTML_CHECK(xuiDocumentSnapshotGetChild(old, paragraph, i, &id) == XUI_OK && id == ids[i]);
        HTML_CHECK(xuiDocumentSnapshotGetNode(old, id, &n) == XUI_OK && n.iTextBytes == 3072 && n.tAttributes.iMarks == XUI_DOC_BOLD);
    }
}
int main(void)
{
    xui_doc_desc_t desc = {0}; xui_document d, empty;
    xui_document_transaction t; xui_document_snapshot old, blank;
    uint64_t paragraph, ids[2], revision; unsigned mode, i;
    char text[3073];
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_RICH;
    for (i = 0; i < 512; i++) memcpy(text + i * 6, "a&<>\"'", 6);
    text[3072] = 0;
    HTML_CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    paragraph = html_insert(t, XUI_DOCUMENT_ROOT, 0, XUI_DOC_PARAGRAPH, NULL);
    ids[0] = html_insert(t, paragraph, 0, XUI_DOC_TEXT, text);
    ids[1] = html_insert(t, paragraph, 1, XUI_DOC_TEXT, text);
    HTML_CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    HTML_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK);
    revision = xuiDocumentGetRevision(d);
    for (mode = 0; mode < 2; mode++) {
        char* expected = html_golden(!mode); long budget; unsigned failures = 0;
        for (budget = 0; budget < 32; budget++) {
            char* output = (char*)1; uint64_t bytes = UINT64_MAX; int result;
            html_budget = budget;
            result = mode ? html_fault_export_children(old, paragraph, &output, &bytes) : html_fault_export(old, &output, &bytes);
            html_budget = -1;
            if (result == XUI_ERROR_OUT_OF_MEMORY) {
                failures++; HTML_CHECK(!output && !bytes && !html_live);
                result = mode ? html_fault_export_children(old, paragraph, &output, &bytes) : html_fault_export(old, &output, &bytes);
                HTML_CHECK(result == XUI_OK);
            } else HTML_CHECK(result == XUI_OK);
            HTML_CHECK(bytes == strlen(expected) && !memcmp(output, expected, (size_t)bytes) && !output[bytes] && html_live == 1);
            html_fault_free(output); HTML_CHECK(!html_live);
            html_check_snapshot(d, old, paragraph, ids, revision);
            if (budget == (long)failures) break;
        }
        HTML_CHECK(budget < 32 && failures >= 5);
        printf("HTML buffer mode=%u OOM points=%u NULL output/retry/golden/old snapshot/history/zero leaks passed\n", mode, failures);
        free(expected);
    }
    HTML_CHECK(xuiDocumentCreate(&desc, &empty) == XUI_OK && xuiDocumentAcquireSnapshot(empty, &blank) == XUI_OK);
    for (i = 0; i < 2; i++) {
        char* output = (char*)1; uint64_t bytes = UINT64_MAX; int result;
        html_budget = i; result = html_fault_export(blank, &output, &bytes); html_budget = -1;
        if (!i) HTML_CHECK(result == XUI_ERROR_OUT_OF_MEMORY && !output && !bytes && !html_live);
        else { HTML_CHECK(result == XUI_OK && output && !output[0] && !bytes && html_live == 1); html_fault_free(output); }
    }
    {
        doc_html b = {0}; unsigned attempts; char* pointer; size_t capacity;
        doc_html_put(&b, "retain"); HTML_CHECK(!b.error && b.size == 6);
        pointer = b.text; capacity = b.capacity; html_budget = 0;
        HTML_CHECK(!doc_html_reserve(&b, capacity) && b.error == XUI_ERROR_OUT_OF_MEMORY && b.text == pointer && b.capacity == capacity && !strcmp(b.text, "retain"));
        attempts = html_attempts; doc_html_put(&b, "ignored");
        HTML_CHECK(html_attempts == attempts && b.size == 6 && !strcmp(b.text, "retain"));
        html_budget = -1; html_fault_free(b.text);
        memset(&b, 0, sizeof(b)); b.size = SIZE_MAX; attempts = html_attempts;
        HTML_CHECK(!doc_html_reserve(&b, 0) && b.error == XUI_DOC_ERROR_LIMIT && html_attempts == attempts);
        memset(&b, 0, sizeof(b)); b.size = SIZE_MAX - 8;
        HTML_CHECK(!doc_html_reserve(&b, 8) && b.error == XUI_DOC_ERROR_LIMIT && html_attempts == attempts);
    }
    HTML_CHECK(!html_live); xuiDocumentSnapshotRelease(blank); xuiDocumentRelease(empty);
    xuiDocumentSnapshotRelease(old); xuiDocumentRelease(d);
    puts("HTML empty allocation / sticky failure / overflow / complete cleanup passed");
    return 0;
}
