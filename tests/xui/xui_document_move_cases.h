#ifndef XUI_DOCUMENT_MOVE_CASES_H
#define XUI_DOCUMENT_MOVE_CASES_H
#include "xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct sample {
    const char *name, *before, *after, *needle;
    int down, root_gap;
    uint64_t root_index;
} sample;

static const sample samples[] = {
    {"root-inline-crlf", "alpha &amp; __one__\r\n\r\nBETA &#x3C; *two*\r\n\r\ngamma\r\n",
        "BETA &#x3C; *two*\r\n\r\nalpha &amp; __one__\r\n\r\ngamma\r\n", "BETA", 0, 0, 0},
    {"root-setext", "alpha &amp; __one__\n===\n\nBETA &#60;\n\ngamma\n",
        "BETA &#60;\n\nalpha &amp; __one__\n===\n\ngamma\n", "BETA", 0, 0, 0},
    {"root-fence", "~~~~ c \nraw &amp; __literal__\n~~~~\n\nBETA &#x3c;\n\ngamma\n",
        "BETA &#x3c;\n\n~~~~ c \nraw &amp; __literal__\n~~~~\n\ngamma\n", "BETA", 0, 0, 0},
    {"quote-paragraphs", ">alpha &amp; __one__\n>\n> BETA &#60;\n>\n> gamma\n",
        "> BETA &#60;\n>\n>alpha &amp; __one__\n>\n> gamma\n", "BETA", 0, 0, 0},
    {"whole-quote", ">alpha &amp; __one__\n>\n> beta &lt;\n\nBETA &#60;\n\ngamma\n",
        "BETA &#60;\n\n>alpha &amp; __one__\n>\n> beta &lt;\n\ngamma\n", "BETA", 1, 1, 0},
    {"hidden-definition", "alpha &amp; __one__\n\n[r]: <raw> 'T'\n\nBETA [two][r]\n\ngamma\n",
        "BETA [two][r]\n\nalpha &amp; __one__\n\n[r]: <raw> 'T'\n\ngamma\n", "BETA", 0, 0, 0},
    {"table-block", "| A | B |\n| :--- | ---: |\n| &amp; | __one__ |\n\nBETA &#60;\n\ngamma\n",
        "BETA &#60;\n\n| A | B |\n| :--- | ---: |\n| &amp; | __one__ |\n\ngamma\n", "BETA", 0, 0, 0},
    {"nested-list-item", "+ alpha &amp; __one__\n\n+ BETA &#60;\n\n+ gamma\n",
        "+ BETA &#60;\n\n+ alpha &amp; __one__\n\n+ gamma\n", "BETA", 0, 0, 0}
};

static int open_document(const char *source, xui_document *out)
{
    xui_doc_desc_t desc = {0};
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = XUI_MD_EXTENDED;
    int result = xuiDocumentCreate(&desc, out);
    if (result == XUI_OK) result = xuiDocumentLoadMarkdown(*out, source, strlen(source));
    return result;
}

static const sample eof_samples[] = {
    {"root-eof", "alpha &amp; __one__\n\nBETA &#60;",
        "BETA &#60;\n\nalpha &amp; __one__\n\n", "BETA", 0, 0, 0},
    {"quote-eof", ">alpha &amp; __one__\n>\n> BETA &#60;",
        "> BETA &#60;\n>\n>alpha &amp; __one__\n>\n", "BETA", 0, 0, 0},
    {"list-eof", "+ alpha &amp; __one__\n\n+ BETA &#60;",
        "+ BETA &#60;\n\n+ alpha &amp; __one__\n\n", "BETA", 0, 0, 0}
};


static const sample definition_eof_samples[] = {
    {"root-definition-eof", "alpha &amp; __one__\n\n[r]: <raw> 'T'\n\nBETA [two][r]",
        "BETA [two][r]\n\nalpha &amp; __one__\n\n[r]: <raw> 'T'\n\n", "BETA", 0, 0, 0},
    {"quote-definition-eof", ">alpha &amp; __one__\n>\n> [r]: <raw> 'T'\n>\n> BETA [two][r]",
        "> BETA [two][r]\n>\n>alpha &amp; __one__\n>\n> [r]: <raw> 'T'\n>\n", "BETA", 0, 0, 0},
    {"list-definition-eof", "+ alpha &amp; __one__\n\n  [r]: <raw> 'T'\n\n+ BETA [two][r]",
        "+ BETA [two][r]\n\n+ alpha &amp; __one__\n\n  [r]: <raw> 'T'\n\n", "BETA", 0, 0, 0}
};

static char* move_source_variant(const char* text, const char* ending, unsigned bom)
{
    char* out = malloc(strlen(text) * 2 + 8);
    size_t count = 0;
    if (!out) return NULL;
    if (bom) { memcpy(out, "\xef\xbb\xbf", 3); count = 3; }
    for (size_t i = 0; text[i]; i++) {
        if (text[i] == '\r' || text[i] == '\n') {
            if (text[i] == '\r' && text[i + 1] == '\n') i++;
            memcpy(out + count, ending, strlen(ending)); count += strlen(ending);
        } else out[count++] = text[i];
    }
    out[count] = 0;
    return out;
}
static int move_source_equal(xui_document_snapshot snapshot, const char* expected)
{
    char source[4096]; uint64_t bytes = 0;
    return xuiDocumentSnapshotCopySource(snapshot, source, sizeof(source), &bytes) == XUI_OK &&
        bytes == strlen(expected) && !memcmp(source, expected, (size_t)bytes);
}
static int move_target(xui_document_snapshot snapshot, const sample* c,
    xui_doc_position_t* at, uint64_t* block, uint64_t* parent, uint64_t* index)
{
    xui_doc_node_info_t info = {0}, owner = {0};
    info.iSize = owner.iSize = sizeof(info);
    if (c->root_gap) {
        *parent = 1; *index = c->root_index;
        return xuiDocumentSnapshotGetChild(snapshot, 1, *index, block);
    }
    if (xuiDocumentSnapshotGetNode(snapshot, at->iNodeId, &info) != XUI_OK) return -1;
    *block = info.iKind == XUI_DOC_TEXT ? info.iParentId : info.iId;
    if (xuiDocumentSnapshotGetNode(snapshot, *block, &info) != XUI_OK ||
        xuiDocumentSnapshotGetNode(snapshot, info.iParentId, &owner) != XUI_OK) return -1;
    if (owner.iKind == XUI_DOC_LIST_ITEM) {
        *block = owner.iId;
        if (xuiDocumentSnapshotGetNode(snapshot, *block, &info) != XUI_OK ||
            xuiDocumentSnapshotGetNode(snapshot, info.iParentId, &owner) != XUI_OK) return -1;
    }
    *parent = owner.iId;
    for (*index = 0; *index < owner.iChildCount; (*index)++) {
        uint64_t id;
        if (xuiDocumentSnapshotGetChild(snapshot, *parent, *index, &id) != XUI_OK) return -1;
        if (id == *block) return XUI_OK;
    }
    return -1;
}

#endif
