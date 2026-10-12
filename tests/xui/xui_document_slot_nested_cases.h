#include "xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct container_move_case {
    const char *name, *before, *after, *moving, *target;
    uint64_t index;
} container_move_case;
static const container_move_case container_cases[] = {
    {"task-first-to-quote", "+ [X] BETA &#60;\n\n  remain __same__\n\n>anchor &lt;\n>\n> tail &amp;\n", "+ [X] remain __same__\n\n>anchor &lt;\n>\n> BETA &#60;\n>\n> tail &amp;\n", "BETA", "anchor", 1},
    {"root-to-nested-task-first", "alpha &amp;\n\n>+ [X] anchor &lt;\n>\n>   tail __same__\n\nBETA &#60;\n", "alpha &amp;\n\n>+ [X] BETA &#60;\n>\n>   anchor &lt;\n>\n>   tail __same__\n\n", "BETA", "anchor", 0},
    {"nested-task-first-to-root", ">+ [X] BETA &#60;\n>\n>   remain __same__\n\nalpha &amp;\n", ">+ [X] remain __same__\n\nBETA &#60;\n\nalpha &amp;\n", "BETA", "alpha", 1},
    {"footnote-list-first-to-root", "anchor [^f]\n\n[^f]: + BETA &#60;\n\n      remain __same__\n", "BETA &#60;\n\nanchor [^f]\n\n[^f]: + remain __same__\n", "BETA", "anchor", 0},
    {"root-to-footnote-task-first", "anchor [^f]\n\n[^f]: + [X] target &lt;\n\n      remain __same__\n\nBETA &#60;\n", "anchor [^f]\n\n[^f]: + [X] BETA &#60;\n\n      target &lt;\n\n      remain __same__\n\n", "BETA", "target", 0},
    {"footnote-lazy-first-to-footnote", "anchor [^f] [^g]\n\n[^f]: BETA &#60;\nlazy __text__\n\n    remain __same__\n\n[^g]: target &lt;\n\n    end __other__\n", "anchor [^f] [^g]\n\n[^f]: remain __same__\n\n[^g]: target &lt;\n\n    BETA &#60;\n    lazy __text__\n\n    end __other__\n", "BETA", "target", 1}
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
