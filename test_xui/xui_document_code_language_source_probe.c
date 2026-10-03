#include "../xui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do { if (!(e)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); exit(1); } } while (0)
#include "xui_document_code_language_source_samples.h"
static uint64_t code_find(xui_document_snapshot s, uint64_t id)
{
    xui_doc_node_info_t info = {0}; uint64_t i; info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(s, id, &info) == XUI_OK);
    if (info.iKind == XUI_DOC_CODE_BLOCK) return id;
    for (i = 0; i < info.iChildCount; i++) { uint64_t child, found;
        CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK);
        found = code_find(s, child); if (found) return found;
    }
    return 0;
}
int main(void)
{
    unsigned sample;
    for (sample = 0; sample < sizeof(code_language_source_samples) / sizeof(*code_language_source_samples); sample++) {
        xui_doc_desc_t desc = {0}; xui_document d; xui_document_snapshot s; xui_document_transaction t;
        uint64_t code, bytes; char source[1024]; int result;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, code_language_source_samples[sample].input,
            strlen(code_language_source_samples[sample].input)) == XUI_OK && xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
        code = code_find(s, 1); CHECK(code); xuiDocumentSnapshotRelease(s);
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        result = xuiDocumentTxnSetCodeBlockLanguage(t, code, code_language_source_samples[sample].language);
        if (result != XUI_OK) fprintf(stderr, "Code language source sample=%u result=%d\n", sample, result);
        CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentSnapshotCopySource(s, source, sizeof(source), &bytes) == XUI_OK);
        if (strcmp(source, code_language_source_samples[sample].expected))
            fprintf(stderr, "Code language source sample=%u expected=[%s]\nactual=[%s]\n", sample, code_language_source_samples[sample].expected, source);
        CHECK(!strcmp(source, code_language_source_samples[sample].expected)); xuiDocumentSnapshotRelease(s); xuiDocumentRelease(d);
    }
    puts("Code language source public DLL probe: all samples passed"); return 0;
}
