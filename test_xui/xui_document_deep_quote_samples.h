/* Independent raw-source goldens for deleting across a retained empty quote. */
#define XUI_DOC_INLINE_SPLIT_HELPERS_ONLY
#include "test_xui/xui_document_inline_split_diagnostic_test.c"
#undef XUI_DOC_INLINE_SPLIT_HELPERS_ONLY

static char* scaffold_source(unsigned depth, unsigned wide, int after)
{
    const char* unit = wide ? "  > " : "> ";
    size_t unit_bytes = strlen(unit), prefix_bytes = unit_bytes * depth;
    char* prefix = malloc(prefix_bytes + 1);
    char* source = malloc(prefix_bytes * 3 + 256);
    size_t at = 0;
    TABLE_CHECK(prefix && source && depth && depth <= 120);
    for (unsigned i = 0; i < depth; i++) memcpy(prefix + i * unit_bytes, unit, unit_bytes);
    prefix[prefix_bytes] = 0;
    at += (size_t)sprintf(source + at, "%s%s+ %s%s\n  ", after ? "le" : "left &amp;", after ? "\n" : "\n\n", prefix, after ? "" : "abcd");
    memcpy(source + at, prefix, prefix_bytes - 1); at += prefix_bytes - 1;
    at += (size_t)sprintf(source + at, "\n  %s[unused]: <raw> 'Title'\n\nend &amp;\n", prefix);
    TABLE_CHECK(at < prefix_bytes * 3 + 256);
    free(prefix);
    return source;
}

static void scaffold_counts(xui_document_snapshot s, uint64_t id,
    unsigned* quotes, unsigned* lists, unsigned* items)
{
    xui_doc_node_info_t node = {0}; uint64_t child;
    node.iSize = sizeof(node);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(s, id, &node) == XUI_OK);
    *quotes += node.iKind == XUI_DOC_QUOTE;
    *lists += node.iKind == XUI_DOC_LIST;
    *items += node.iKind == XUI_DOC_LIST_ITEM;
    for (uint64_t i = 0; i < node.iChildCount; i++) {
        TABLE_CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK);
        scaffold_counts(s, child, quotes, lists, items);
    }
}

static xui_doc_position_t scaffold_position(xui_document_snapshot s, const char* source,
    const char* word, unsigned quotes_expected, uint64_t offset)
{
    const char* at = strstr(source, word);
    xui_doc_position_t position; xui_doc_node_info_t node = {0}; int quality;
    unsigned quotes = 0, lists = 0, items = 0, paragraphs = 0;
    TABLE_CHECK(at && xuiDocumentSourceToPosition(s, (uint64_t)(at - source) + 1,
        &position, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
    node.iSize = sizeof(node);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(s, position.iNodeId, &node) == XUI_OK &&
        node.iKind == XUI_DOC_TEXT && node.iTextBytes >= offset);
    for (uint64_t id = node.iParentId; id != XUI_DOCUMENT_ROOT; id = node.iParentId) {
        TABLE_CHECK(xuiDocumentSnapshotGetNode(s, id, &node) == XUI_OK && node.iParentId != id);
        quotes += node.iKind == XUI_DOC_QUOTE; lists += node.iKind == XUI_DOC_LIST;
        items += node.iKind == XUI_DOC_LIST_ITEM; paragraphs += node.iKind == XUI_DOC_PARAGRAPH;
    }
    TABLE_CHECK(quotes == quotes_expected && paragraphs == 1 &&
        lists == !!quotes_expected && items == !!quotes_expected);
    position.iOffset = offset;
    return position;
}

