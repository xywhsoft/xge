#ifndef XUI_DOCUMENT_CONTAINER_MOVE_CASES_H
#define XUI_DOCUMENT_CONTAINER_MOVE_CASES_H
#include "xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct container_move_case {
    const char *name, *before, *after, *moving, *target;
    uint64_t index;
} container_move_case;
static const container_move_case container_cases[] = {
    {"list-first-paragraph", "+ alpha &amp; __one__\n\n  BETA &#60;\n\n  gamma\n",
        "+ BETA &#60;\n\n  alpha &amp; __one__\n\n  gamma\n", "BETA", "alpha", 0},
    {"task-first-paragraph", "+ [X] alpha &amp; __one__\n\n  BETA &#60;\n\n  gamma\n",
        "+ [X] BETA &#60;\n\n  alpha &amp; __one__\n\n  gamma\n", "BETA", "alpha", 0},
    {"footnote-first-paragraph", "BETA [^f]\n\n[^f]: alpha &amp; __one__\n\n    SECOND &#60;\n\n    gamma\n",
        "BETA [^f]\n\n[^f]: SECOND &#60;\n\n    alpha &amp; __one__\n\n    gamma\n", "SECOND", "alpha", 0},
    {"root-to-quote", "alpha &amp; __one__\n\n>anchor &lt;\n>\n> tail __same__\n\nBETA &#60;\n",
        "alpha &amp; __one__\n\n>anchor &lt;\n>\n> BETA &#60;\n>\n> tail __same__\n\n", "BETA", "anchor", 1},
    {"quote-to-root", "alpha &amp; __one__\n\n>anchor &lt;\n>\n> BETA &#60;\n>\n> tail __same__\n\n",
        "alpha &amp; __one__\n\nBETA &#60;\n\n>anchor &lt;\n>\n> tail __same__\n\n", "BETA", "alpha", 1},
    {"list-item-to-list-item", "+ alpha &amp; __one__\n\n  BETA &#60;\n\n  tail __same__\n\n+ anchor &lt;\n\n  end __other__\n",
        "+ alpha &amp; __one__\n\n  tail __same__\n\n+ anchor &lt;\n\n  BETA &#60;\n\n  end __other__\n", "BETA", "anchor", 1}
};
static int container_open(const char* source, xui_document* d)
{
    xui_doc_desc_t desc = {0}; desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = XUI_MD_EXTENDED;
    int result = xuiDocumentCreate(&desc, d);
    if (result == XUI_OK) result = xuiDocumentLoadMarkdown(*d, source, strlen(source));
    return result;
}
static int container_source(xui_document_snapshot s, const char* expected)
{
    char raw[4096]; uint64_t bytes;
    return xuiDocumentSnapshotCopySource(s, raw, sizeof(raw), &bytes) == XUI_OK &&
        bytes == strlen(expected) && !memcmp(raw, expected, (size_t)bytes);
}
static int container_paragraph(xui_document_snapshot s, const char* before, const char* needle, uint64_t* out)
{
    xui_doc_position_t at; xui_doc_node_info_t info = {0}; int quality;
    info.iSize = sizeof(info);
    const char* found = strstr(before, needle);
    if (!found || xuiDocumentSourceToPosition(s, found - before, &at, &quality) != XUI_OK ||
        xuiDocumentSnapshotGetNode(s, at.iNodeId, &info) != XUI_OK) return -1;
    if (info.iKind == XUI_DOC_TEXT && xuiDocumentSnapshotGetNode(s, info.iParentId, &info) != XUI_OK) return -1;
    if (info.iKind != XUI_DOC_PARAGRAPH) return -1;
    *out = info.iId; return XUI_OK;
}

#endif
