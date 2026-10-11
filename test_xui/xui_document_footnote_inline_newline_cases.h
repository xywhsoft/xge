#define XUI_DOC_INLINE_SPLIT_HELPERS_ONLY
#include "test_xui/xui_document_inline_split_diagnostic_test.c"
#undef XUI_DOC_INLINE_SPLIT_HELPERS_ONLY

static const struct newline_fixture { const char *name, *before, *after; } newline_fixtures[] = {
    {"root-control", "abcd &amp;\n\nend &amp;\n", "ab\n\ncd &amp;\n\nend &amp;\n"},
    {"footnote-plain-control",
     "use[^n]\n\n[^n]: abcd &amp;\n\n    tail &amp;\n\n    [unused]: <raw> 'Title'\n\nend &amp;\n",
     "use[^n]\n\n[^n]: ab\n\n    cd &amp;\n\n    tail &amp;\n\n    [unused]: <raw> 'Title'\n\nend &amp;\n"},
    {"footnote-inline-quote-tab",
     "use[^n]\n\n[^n]: >\tabcd &amp;\n    >\n    >\ttail &amp;\n\n    [unused]: <raw> 'Title'\n\nend &amp;\n",
     "use[^n]\n\n[^n]: >\tab\n    >\n    >\tcd &amp;\n    >\n    >\ttail &amp;\n\n    [unused]: <raw> 'Title'\n\nend &amp;\n"},
    {"footnote-inline-ordered-tab",
     "use[^n]\n\n[^n]: 007)\tabcd &amp;\n\n            tail &amp;\n\n    [unused]: <raw> 'Title'\n\nend &amp;\n",
     "use[^n]\n\n[^n]: 007)\tab\n\n            cd &amp;\n\n            tail &amp;\n\n    [unused]: <raw> 'Title'\n\nend &amp;\n"},
    {"footnote-inline-task",
     "use[^n]\n\n[^n]: + [X] abcd &amp;\n\n      tail &amp;\n\n    [unused]: <raw> 'Title'\n\nend &amp;\n",
     "use[^n]\n\n[^n]: + [X] ab\n\n      cd &amp;\n\n      tail &amp;\n\n    [unused]: <raw> 'Title'\n\nend &amp;\n"}
};

static int newline_execute(xui_document d, const xui_doc_range_t* r, unsigned mode,
    xui_doc_position_t* caret, xui_document_change_set* changes)
{
    xui_document_transaction t; int result = xuiDocumentBeginTransaction(d, NULL, &t);
    if (result != XUI_OK) return result;
    result = mode ? xuiDocumentTxnSplitBlock(t, &r->tAnchor, caret) :
        xuiDocumentTxnReplaceRange(t, r, "\n", 1, caret);
    if (result == XUI_OK) result = xuiDocumentTxnCommit(t, changes);
    if (result != XUI_OK) TABLE_CHECK(xuiDocumentTxnCommit(t, NULL) == result);
    xuiDocumentTxnRelease(t); return result;
}

static char* newline_label(const char* source, const char* label)
{
    const char *p = source, *at; size_t count = 0, length = strlen(label);
    while ((at = strstr(p, "[^n]")) != NULL) { count++; p = at + 4; }
    size_t bytes = strlen(source) + count * (length - 1);
    char *out = malloc(bytes + 1), *write = out; TABLE_CHECK(out);
    p = source;
    while ((at = strstr(p, "[^n]")) != NULL) {
        memcpy(write, p, (size_t)(at-p)); write += at-p;
        memcpy(write, "[^", 2); write += 2;
        memcpy(write, label, length); write += length; *write++ = ']'; p = at + 4;
    }
    strcpy(write, p); return out;
}
static void newline_footnote_scope(xui_document_snapshot snapshot, const char* source, const char* word)
{
    xui_doc_position_t position; xui_doc_node_info_t node = {0}; int quality, found = 0;
    const char* p = strstr(source, word); TABLE_CHECK(p);
    TABLE_CHECK(xuiDocumentSourceToPosition(snapshot, (uint64_t)(p-source)+1, &position, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
    node.iSize = sizeof(node);
    for (uint64_t id = position.iNodeId; id != XUI_DOCUMENT_ROOT;) {
        TABLE_CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &node) == XUI_OK);
        if (node.iKind == XUI_DOC_FOOTNOTE) found = 1;
        id = node.iParentId;
    }
    TABLE_CHECK(found); /* Reject an invalid long-label oracle instead of counting a literal paragraph as a footnote. */
}

