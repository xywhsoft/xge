/* Independent valid list-path goldens with a retained empty quote and confirmed hidden reference. */
#define XUI_DOC_INLINE_SPLIT_HELPERS_ONLY
#include "test_xui/xui_document_inline_split_diagnostic_test.c"
#undef XUI_DOC_INLINE_SPLIT_HELPERS_ONLY

static char* deep_list_source(unsigned depth, unsigned ordered, unsigned variant, int after)
{
    const char* marker = ordered ? "1.   " : "+    ";
    size_t width = strlen(marker), capacity = (size_t)depth * depth * width * 4 + (size_t)depth * 1024 + 8192;
    char* source = malloc(capacity);
    size_t at = 0;
    TABLE_CHECK(source && depth && depth <= 60 && width == 5 && variant < 7);
    at += (size_t)sprintf(source + at, "%s%s", after ? "le" : "left &amp;",
        after && depth == 1 ? "\n" : "\n\n");
    for (unsigned i = 0; i < depth; i++) {
        memset(source + at, ' ', (size_t)i * width); at += (size_t)i * width;
        at += (size_t)sprintf(source + at, "%s", marker);
        if (i + 1 == depth) {
            at += (size_t)sprintf(source + at, "> %s\n", after ? "" : "abcd");
        } else if (after) {
            at += (size_t)sprintf(source + at, "\n");
        } else if (variant == 0) {
            at += (size_t)sprintf(source + at, "# remove\n");
        } else if (variant == 1) {
            at += (size_t)sprintf(source + at, "> remove\n\n");
        } else if (variant == 2 || variant == 3) {
            at += (size_t)sprintf(source + at, "```%s\n", variant == 2 ? "c" : "mermaid");
            memset(source + at, ' ', (size_t)(i + 1) * width); at += (size_t)(i + 1) * width;
            at += (size_t)sprintf(source + at, "%s\n", variant == 2 ? "remove" : "graph TD; A-->B");
            memset(source + at, ' ', (size_t)(i + 1) * width); at += (size_t)(i + 1) * width;
            at += (size_t)sprintf(source + at, "```\n\n");
        } else if (variant == 4) {
            at += (size_t)sprintf(source + at, "---\n\n");
        } else if (variant == 5) {
            at += (size_t)sprintf(source + at, "<div>remove</div>\n\n");
        } else {
            at += (size_t)sprintf(source + at, "a | b\n");
            memset(source + at, ' ', (size_t)(i + 1) * width); at += (size_t)(i + 1) * width;
            at += (size_t)sprintf(source + at, "--- | ---\n");
            memset(source + at, ' ', (size_t)(i + 1) * width); at += (size_t)(i + 1) * width;
            at += (size_t)sprintf(source + at, "remove | cell\n\n");
        }
    }
    memset(source + at, ' ', (size_t)depth * width); at += (size_t)depth * width;
    at += (size_t)sprintf(source + at, ">\n");
    memset(source + at, ' ', (size_t)depth * width); at += (size_t)depth * width;
    at += (size_t)sprintf(source + at, "> [unused]: <raw> 'Title'\n\nend &amp;\n");
    TABLE_CHECK(at < capacity);
    return source;
}

static void deep_list_counts(xui_document_snapshot snapshot, uint64_t id,
    unsigned* lists, unsigned* items, unsigned* quotes)
{
    xui_doc_node_info_t node = {0}; uint64_t child;
    node.iSize = sizeof(node);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &node) == XUI_OK);
    *lists += node.iKind == XUI_DOC_LIST;
    *items += node.iKind == XUI_DOC_LIST_ITEM;
    *quotes += node.iKind == XUI_DOC_QUOTE;
    for (uint64_t i = 0; i < node.iChildCount; i++) {
        TABLE_CHECK(xuiDocumentSnapshotGetChild(snapshot, id, i, &child) == XUI_OK);
        deep_list_counts(snapshot, child, lists, items, quotes);
    }
}

static xui_doc_position_t deep_list_position(xui_document_snapshot snapshot,
    const char* source, const char* word, unsigned expected_depth, uint64_t offset)
{
    const char* at = strstr(source, word);
    xui_doc_position_t position; xui_doc_node_info_t node = {0}; int quality;
    unsigned lists = 0, items = 0, paragraphs = 0, quotes = 0;
    TABLE_CHECK(at && xuiDocumentSourceToPosition(snapshot, (uint64_t)(at - source) + 1,
        &position, &quality) == XUI_OK && quality == XUI_DOC_MAP_EXACT);
    node.iSize = sizeof(node);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(snapshot, position.iNodeId, &node) == XUI_OK &&
        node.iKind == XUI_DOC_TEXT && node.iTextBytes >= offset);
    for (uint64_t id = node.iParentId; id != XUI_DOCUMENT_ROOT; id = node.iParentId) {
        TABLE_CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &node) == XUI_OK && node.iParentId != id);
        lists += node.iKind == XUI_DOC_LIST; items += node.iKind == XUI_DOC_LIST_ITEM;
        paragraphs += node.iKind == XUI_DOC_PARAGRAPH; quotes += node.iKind == XUI_DOC_QUOTE;
    }
    TABLE_CHECK(lists == expected_depth && items == expected_depth && paragraphs == 1 && quotes == !!expected_depth);
    position.iOffset = offset;
    return position;
}

static unsigned selected_block_count(xui_document_snapshot snapshot, uint64_t id, uint32_t kind)
{
    xui_doc_node_info_t node = {0}; uint64_t child; unsigned total;
    node.iSize = sizeof(node);
    TABLE_CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &node) == XUI_OK);
    total = node.iKind == kind;
    for (uint64_t i = 0; i < node.iChildCount; i++) {
        TABLE_CHECK(xuiDocumentSnapshotGetChild(snapshot, id, i, &child) == XUI_OK);
        total += selected_block_count(snapshot, child, kind);
    }
    return total;
}

