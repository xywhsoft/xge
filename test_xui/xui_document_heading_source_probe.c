#include "../xui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); exit(1); } } while (0)
#include "xui_document_heading_source_samples.h"
static uint64_t heading_find(xui_document_snapshot s, uint64_t id)
{
    xui_doc_node_info_t info = {0}; uint64_t i; info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(s, id, &info) == XUI_OK);
    if (info.iKind == XUI_DOC_HEADING) return id;
    for (i = 0; i < info.iChildCount; i++) { uint64_t child, found;
        CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK);
        found = heading_find(s, child); if (found) return found;
    }
    return 0;
}
int main(void)
{
    unsigned sample;
    for (sample = 0; sample < sizeof(heading_source_samples) / sizeof(*heading_source_samples); sample++) {
        xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot s; xui_document_transaction t;
        xui_doc_range_t range = {0}, after; uint64_t count, bytes; char source[2048]; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, heading_source_samples[sample].input,
            strlen(heading_source_samples[sample].input)) == XUI_OK && xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
        if (sample == 19 || sample == 20) {
            range.tAnchor.iSize = sizeof(range.tAnchor);
            range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
            range.tAnchor.iNodeId = heading_find(s, 1); range.tAnchor.iKind = XUI_DOC_POSITION_GAP;
            range.tAnchor.iAffinity = XUI_DOC_AFTER; CHECK(range.tAnchor.iNodeId); range.tCaret = range.tAnchor;
        } else CHECK(xuiDocumentSnapshotFind(s, XUI_DOC_SEMANTIC, "abcd", 4, NULL, &range, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(s);
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        result = xuiDocumentTxnSetHeading(t, &range, heading_source_samples[sample].level, &after);
        if (result != XUI_OK) fprintf(stderr, "Heading source sample=%u result=%d\n", sample, result);
        CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentSnapshotCopySource(s, source, sizeof(source), &bytes) == XUI_OK);
        if (strcmp(source, heading_source_samples[sample].expected))
            fprintf(stderr, "Heading source sample=%u expected=[%s]\nactual=[%s]\n", sample, heading_source_samples[sample].expected, source);
        CHECK(!strcmp(source, heading_source_samples[sample].expected)); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    }
    puts("Heading source public DLL probe: all samples passed"); return 0;
}
