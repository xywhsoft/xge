#include "../xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RUN_CHECK(x) do { if (!(x)) { fprintf(stderr, "HTML runs line %d: %s\n", __LINE__, #x); exit(1); } } while (0)

static void html_expect(xui_document_snapshot snapshot, const char* expected)
{
    char* html; uint64_t bytes;
    RUN_CHECK(xuiDocumentExportHtml(snapshot, &html, &bytes) == XUI_OK);
    if (bytes != strlen(expected) || memcmp(html, expected, (size_t)bytes))
        fprintf(stderr, "actual=[%s]\nexpected=[%s]\n", html, expected);
    RUN_CHECK(bytes == strlen(expected) && !memcmp(html, expected, (size_t)bytes));
    xuiDocumentFreeBuffer(html);
}
static uint64_t html_add(xui_document_transaction t, uint64_t parent, uint64_t at,
    unsigned kind, const char* text, unsigned marks, const char* uri, const char* title)
{
    xui_doc_node_desc_t node = {0}; uint64_t id;
    node.iSize = sizeof(node); node.iKind = kind; node.sText = text; node.iTextBytes = text ? strlen(text) : 0;
    node.tAttributes.iMarks = marks; node.sResource = uri; node.sTitle = title;
    RUN_CHECK(xuiDocumentTxnInsertNode(t, parent, at, &node, &id) == XUI_OK); return id;
}
int main(void)
{
    xui_document d; xui_document_transaction t; xui_document_snapshot s;
    xui_doc_desc_t desc = {0}; xui_doc_node_info_t p = {0}; uint64_t paragraph, ids[8], i, id;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_RICH;
    RUN_CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
    paragraph = html_add(t, XUI_DOCUMENT_ROOT, 0, XUI_DOC_PARAGRAPH, NULL, 0, NULL, NULL);
    ids[0] = html_add(t, paragraph, 0, XUI_DOC_TEXT, "a", XUI_DOC_LINK | XUI_DOC_CODE, "/raw", "Title");
    ids[1] = html_add(t, paragraph, 1, XUI_DOC_TEXT, "", XUI_DOC_BOLD, NULL, NULL);
    ids[2] = html_add(t, paragraph, 2, XUI_DOC_TEXT, "|b", XUI_DOC_LINK | XUI_DOC_CODE, "/raw", "Title");
    ids[3] = html_add(t, paragraph, 3, XUI_DOC_TEXT, "c", XUI_DOC_LINK | XUI_DOC_CODE, "/other", "Title");
    ids[4] = html_add(t, paragraph, 4, XUI_DOC_TEXT, "d", XUI_DOC_LINK | XUI_DOC_CODE, "/other", "Different");
    ids[5] = html_add(t, paragraph, 5, XUI_DOC_HARD_BREAK, NULL, 0, NULL, NULL);
    ids[6] = html_add(t, paragraph, 6, XUI_DOC_TEXT, "e", XUI_DOC_LINK | XUI_DOC_CODE, "/other", "Different");
    ids[7] = html_add(t, paragraph, 7, XUI_DOC_TEXT, "f", XUI_DOC_LINK | XUI_DOC_BOLD, "/other", "Different");
    RUN_CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    RUN_CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    for (i = 0; i < 3; i++) html_expect(s,
        "<p><span><a href=\"/raw\" title=\"Title\"><code>a|b</code></a></span>"
        "<span><a href=\"/other\" title=\"Title\"><code>c</code></a></span>"
        "<span><a href=\"/other\" title=\"Different\"><code>d</code></a></span><br>\n"
        "<span><a href=\"/other\" title=\"Different\"><code>e</code></a></span>"
        "<span><a href=\"/other\" title=\"Different\"><strong>f</strong></a></span></p>\n");
    p.iSize = sizeof(p);
    RUN_CHECK(xuiDocumentSnapshotGetNode(s, paragraph, &p) == XUI_OK && p.iChildCount == 8);
    for (i = 0; i < 8; i++) RUN_CHECK(xuiDocumentSnapshotGetChild(s, paragraph, i, &id) == XUI_OK && id == ids[i]);
    xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    puts("HTML equal runs / URI-title-format-object boundaries / empty runs / immutable identities passed");
    return 0;
}
