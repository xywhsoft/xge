#define XUI_QUOTE_BOUNDARY_NO_MAIN
#include "xui_document_quote_boundary_test.c"

typedef struct unlist_source_case { const char* source; const char* expected; int item; } unlist_source_case;
static const unlist_source_case unlist_cases[] = {
    {"- take &amp; raw\n", "take &amp; raw\n", 0},
    {"- take &amp; raw\n\n  ~~~~ lang opaque &amp; info\n  code\\raw\n  ~~~~\n", "take &amp; raw\n\n~~~~ lang opaque &amp; info\ncode\\raw\n~~~~\n", 0},
    {"before &amp; raw\n\n- take &amp; raw\n- last &amp; raw\n\nafter &amp; raw\n", "before &amp; raw\n\ntake &amp; raw\n\nlast &amp; raw\n\nafter &amp; raw\n", 0},
    {"- before &amp; raw\n- take &amp; raw\n- last &amp; raw\n", "- before &amp; raw\n\ntake &amp; raw\n\n- last &amp; raw\n", 1},
    {"> - [x] take &amp; raw\n>\n>   ~~~~ lang opaque\n>   code\\raw\n>   ~~~~\n", "> take &amp; raw\n>\n> ~~~~ lang opaque\n> code\\raw\n> ~~~~\n", 0},
    {"use[^n]\n\n[^n]:\n    - take &amp; raw\n    - last &amp; raw\n\noutside &amp; raw\n", "use[^n]\n\n[^n]:\n    take &amp; raw\n    \n    last &amp; raw\n\noutside &amp; raw\n", 0},
    {"99. before &amp; raw\n100. take &amp; raw\n101. last &amp; raw\n", "99. before &amp; raw\n\ntake &amp; raw\n\n101. last &amp; raw\n", 1},
    {"- take [a]\n\n  [a]: /target 'one\n  two &amp;'\n\n  also &amp; raw\n", "take [a]\n\n[a]: /target 'one\ntwo &amp;'\n\nalso &amp; raw\n", 0},
{"* take &amp; raw", "take &amp; raw", 0},
    {"   + take &amp; raw\n", "take &amp; raw\n", 0},
    {"-\ttake &amp; raw\n", "take &amp; raw\n", 0},
    {"-     take &amp; raw\n", "    take &amp; raw\n", 0},
    {"-\t\ttake &amp; raw\n", "      take &amp; raw\n", 0},
    {"- [X]   take &amp; raw\n", "take &amp; raw\n", 0},
    {"- [ ]\ttake &amp; raw\n  continuation &amp; raw\n", "take &amp; raw\ncontinuation &amp; raw\n", 0},
    {"- take &amp; raw\nlazy &amp; raw\n", "take &amp; raw\nlazy &amp; raw\n", 0},
    {"- take &amp; raw\n\n      code\t<&> raw\n", "take &amp; raw\n\n    code\t<&> raw\n", 0},
    {"- take &amp; raw\n\n  | a | b |\n  | :-- | --: |\n  | &amp; | `x|y` |\n", "take &amp; raw\n\n| a | b |\n| :-- | --: |\n| &amp; | `x|y` |\n", 0},
    {"> > - take &amp; raw\n> >   continuation &amp; raw\n", "> > take &amp; raw\n> > continuation &amp; raw\n", 0},
    {"> before &amp; raw\n> - take &amp; raw\n> after &amp; raw\n", "> before &amp; raw\n> \n> take &amp; raw\n> after &amp; raw\n", 0},
    {"- before &amp; raw\n  - take &amp; raw\n  - last &amp; raw\n- after &amp; raw\n", "- before &amp; raw\n  \n  take &amp; raw\n  \n  last &amp; raw\n- after &amp; raw\n", 0},
    {"- - take &amp; raw\n  - last &amp; raw\n- after &amp; raw\n", "- take &amp; raw\n  \n  last &amp; raw\n- after &amp; raw\n", 0},
    {"- parent &amp; raw\n  - before &amp; raw\n  - take &amp; raw\n  - last &amp; raw\n", "- parent &amp; raw\n  - before &amp; raw\n  \n  take &amp; raw\n  \n  - last &amp; raw\n", 1},
    {"use[^n]\n\n[^n]: - take &amp; raw\n    - last &amp; raw\n", "use[^n]\n\n[^n]: take &amp; raw\n    \n    last &amp; raw\n", 0},
    {"- take [^n]\n\n  [^n]: body &amp; raw\n      continuation &amp; raw\n", "take [^n]\n\n[^n]: body &amp; raw\n    continuation &amp; raw\n", 0},
    {"[a]: /first 'wins'\n\n- take [a]\n\n  [a]: /duplicate 'raw &amp;'\n\n  [unused]: /opaque 'one\n  two &amp;'\n\n  also [a]\n", "[a]: /first 'wins'\n\ntake [a]\n\n[a]: /duplicate 'raw &amp;'\n\n[unused]: /opaque 'one\ntwo &amp;'\n\nalso [a]\n", 0},
    {"7) before &amp; raw\n8) take &amp; raw\n9) last &amp; raw\n", "7) before &amp; raw\n\ntake &amp; raw\n\n9) last &amp; raw\n", 1},
    {"- take &amp; raw\n\n  <div>HTML &amp; raw</div>\n\n  after &amp; raw\n", "take &amp; raw\n\n<div>HTML &amp; raw</div>\n\nafter &amp; raw\n", 0},
    {"- take 中文 🧡 &amp; raw\n", "take 中文 🧡 &amp; raw\n", 0},
    {"- take &amp; raw\n\n  > [!nOtE]\n  > body &amp; raw\n", "take &amp; raw\n\n> [!nOtE]\n> body &amp; raw\n", 0},
    {"- before &amp; raw\n- take &amp; raw\n\n  ~~~~ lang opaque\n  code\\raw\n  ~~~~\n\n- last &amp; raw\n",
     "- before &amp; raw\n\ntake &amp; raw\n\n~~~~ lang opaque\ncode\\raw\n~~~~\n\n- last &amp; raw\n", 1},
};
static xui_doc_node_info_t unlist_item_node(xui_document_snapshot snapshot, uint64_t id)
{
    xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
    do { CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &node) == XUI_OK); id = node.iParentId; }
    while (id && node.iKind != XUI_DOC_LIST_ITEM);
    CHECK(node.iKind == XUI_DOC_LIST_ITEM); return node;
}
static uint64_t unlist_leaf(xui_document_snapshot snapshot, uint64_t id, int last)
{
    xui_doc_node_info_t node = {0}; uint64_t i, child, found;
    node.iSize = sizeof(node); CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &node) == XUI_OK);
    if (node.iTextBytes) return id;
    for (i = 0; i < node.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(snapshot, id, last ? node.iChildCount - i - 1 : i, &child) == XUI_OK);
        found = unlist_leaf(snapshot, child, last); if (found) return found;
    }
    return 0;
}
static xui_doc_range_t unlist_selection(xui_document d, xui_document_snapshot before,
    const unlist_source_case* fixture, unsigned kind, unsigned affinity, unsigned reverse,
    xui_doc_node_info_t* list, uint64_t* first, uint64_t* end)
{
    xui_doc_range_t range = find_text(d, "take");
    xui_doc_node_info_t item = unlist_item_node(before, range.tAnchor.iNodeId), node = {0};
    uint64_t i, id, first_id, last_id;
    list->iSize = sizeof(*list); node.iSize = sizeof(node);
    CHECK(xuiDocumentSnapshotGetNode(before, item.iParentId, list) == XUI_OK && list->iKind == XUI_DOC_LIST);
    *first = 0; *end = list->iChildCount;
    if (fixture->item) {
        for (i = 0; i < list->iChildCount; i++) { CHECK(xuiDocumentSnapshotGetChild(before, list->iId, i, &id) == XUI_OK); if (id == item.iId) break; }
        CHECK(i < list->iChildCount); *first = i; *end = i + 1;
    }
    range.tAnchor.iAffinity = range.tCaret.iAffinity = affinity;
    if (!kind) {
        range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_GAP;
        range.tAnchor.iNodeId = range.tCaret.iNodeId = list->iId;
        range.tAnchor.iOffset = *first; range.tCaret.iOffset = *end;
    } else {
        CHECK(xuiDocumentSnapshotGetChild(before, list->iId, *first, &first_id) == XUI_OK &&
            xuiDocumentSnapshotGetChild(before, list->iId, *end - 1, &last_id) == XUI_OK);
        if (kind == 1) {
            range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_GAP;
            range.tAnchor.iNodeId = first_id; range.tAnchor.iOffset = 0;
            range.tCaret.iNodeId = last_id;
            CHECK(xuiDocumentSnapshotGetNode(before, last_id, &node) == XUI_OK); range.tCaret.iOffset = node.iChildCount;
        } else if (kind == 2) {
            range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_TEXT;
            range.tAnchor.iNodeId = unlist_leaf(before, first_id, 0); range.tAnchor.iOffset = 0;
            range.tCaret.iNodeId = unlist_leaf(before, last_id, 1);
            CHECK(range.tAnchor.iNodeId && range.tCaret.iNodeId && xuiDocumentSnapshotGetNode(before, range.tCaret.iNodeId, &node) == XUI_OK);
            range.tCaret.iOffset = node.iTextBytes;
        } else { CHECK(*end == *first + 1); range.tAnchor.iOffset++; range.tCaret = range.tAnchor; }
    }
    if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
    return range;
}
static uint64_t unlist_patch_proof(const char* source, const char* output,
    xui_document_snapshot before, xui_document_change_set change,
    const xui_doc_node_info_t* list, uint64_t first, uint64_t end)
{
    xui_doc_change_info_t info = {0}; uint64_t i, at, last = UINT64_MAX, removed = 0, added = 0, count = 0;
    info.iSize = sizeof(info); CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
    for (i = 0; i < info.iOperationCount; i++) {
        const xui_doc_operation_t* op = &info.pOperations[i];
        if (op->iKind != XUI_DOC_OP_SOURCE) continue;
        CHECK(op->iOffset <= last && op->iOldLength <= 64 && op->iNewLength <= 128);
        for (at = op->iOffset; at < op->iOffset + op->iOldLength; at++) {
            uint64_t j; int owned = 0;
            if (source[at] == ' ' || source[at] == '\t' || source[at] == '\r' || source[at] == '\n') continue;
            for (j = first; j < end; j++) {
                uint64_t item; xui_doc_block_syntax_t syntax = {0}; syntax.iSize = sizeof(syntax);
                CHECK(xuiDocumentSnapshotGetChild(before, list->iId, j, &item) == XUI_OK &&
                    xuiDocumentSnapshotGetBlockSyntax(before, item, &syntax) == XUI_OK);
                if ((at >= syntax.iPrimaryStart && at < syntax.iPrimaryEnd) ||
                    (syntax.iSecondaryEnd && at >= syntax.iSecondaryStart && at < syntax.iSecondaryEnd)) owned = 1;
            }
            CHECK(owned);
        }
        removed += op->iOldLength; added += op->iNewLength; last = op->iOffset; count++;
    }
    CHECK(count && strlen(source) - removed + added == strlen(output));
    for (at = 0; at <= strlen(source); at++) {
        unsigned affinity;
        if (((unsigned char)source[at] & 0xc0) == 0x80) continue;
        for (affinity = 0; affinity < 2; affinity++) {
            xui_doc_position_t p = {0}, mapped; uint64_t expected = at; int mapping;
            p.iSize = sizeof(p); p.iKind = XUI_DOC_POSITION_SOURCE; p.iNodeId = 1;
            p.iDocumentId = xuiDocumentSnapshotGetIdentity(before); p.iRevision = xuiDocumentSnapshotGetRevision(before);
            p.iOffset = at; p.iAffinity = affinity;
            for (i = 0; i < info.iOperationCount; i++) {
                const xui_doc_operation_t* op = &info.pOperations[i];
                if (op->iKind != XUI_DOC_OP_SOURCE || expected < op->iOffset ||
                    (expected == op->iOffset && affinity == XUI_DOC_BEFORE)) continue;
                if (expected >= op->iOffset + op->iOldLength) expected = expected - op->iOldLength + op->iNewLength;
                else expected = op->iOffset + (affinity == XUI_DOC_AFTER ? op->iNewLength : 0);
            }
            CHECK(xuiDocumentMapPosition(change, &p, &mapped, &mapping) == XUI_OK && mapped.iOffset == expected);
        }
    }
    return removed;
}
static void unlist_source_matrix(void)
{
    unsigned ci, mode, kind, affinity, reverse, count = 0;
    for (ci = 0; ci < sizeof(unlist_cases) / sizeof(unlist_cases[0]); ci++)
    for (mode = 0; mode < 3; mode++)
    for (kind = 0; kind < 4; kind++)
    for (affinity = 0; affinity < 2; affinity++)
    for (reverse = 0; reverse < 2; reverse++) {
        char source[8192], expected[8192], actual[8192]; xui_document d, reload;
        xui_document_snapshot before, after, full; xui_document_transaction t; xui_document_change_set change;
        xui_doc_range_t range, mapped; xui_doc_node_info_t list = {0}; uint64_t first, end, i, removed; int result;
        boundary_variant(unlist_cases[ci].source, mode, source, sizeof(source));
        boundary_variant(unlist_cases[ci].expected, mode, expected, sizeof(expected));
        d = open_md(source); CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        if (kind == 3 && !unlist_cases[ci].item) {
            xui_doc_node_info_t item = unlist_item_node(before, find_text(d, "take").tAnchor.iNodeId);
            list.iSize = sizeof(list); CHECK(xuiDocumentSnapshotGetNode(before, item.iParentId, &list) == XUI_OK);
            if (list.iChildCount != 1) { xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); continue; }
        }
        range = unlist_selection(d, before, &unlist_cases[ci], kind, affinity, reverse, &list, &first, &end);
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        result = xuiDocumentTxnUnlistRange(t, &range, &mapped);
        if (result) fprintf(stderr, "unlist %u/%u/%u/%u/%u: %d\n", ci, mode, kind, affinity, reverse, result);
        CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t);
        mapped.tAnchor.iRevision = mapped.tCaret.iRevision = xuiDocumentGetRevision(d);
        {
            int order;
            CHECK(xuiDocumentSnapshotComparePositions(after, &mapped.tAnchor, &mapped.tCaret, &order) == XUI_OK &&
                order == (kind == 3 ? 0 : reverse ? 1 : -1) &&
                mapped.tAnchor.iAffinity == range.tAnchor.iAffinity && mapped.tCaret.iAffinity == range.tCaret.iAffinity);
        }
        copy_source(d, actual, sizeof(actual));
        if (strcmp(actual, expected)) fprintf(stderr, "unlist %u/%u/%u/%u/%u source:\n%s\nEXPECTED\n%s\n", ci, mode, kind, affinity, reverse, actual, expected);
        CHECK(!strcmp(actual, expected)); removed = unlist_patch_proof(source, actual, before, change, &list, first, end);
        xuiDocumentChangeSetRelease(change);
        for (i = 0; i < list.iChildCount; i++) {
            uint64_t item, j; xui_doc_node_info_t old = {0}, now = {0}; old.iSize = now.iSize = sizeof(old);
            CHECK(xuiDocumentSnapshotGetChild(before, list.iId, i, &item) == XUI_OK && xuiDocumentSnapshotGetNode(before, item, &old) == XUI_OK);
            if (i < first || i >= end) { same_tree(before, item, after, item); boundary_child_ids(before, after, item); continue; }
            CHECK(xuiDocumentSnapshotGetNode(after, item, &now) == XUI_ERROR_NOT_FOUND);
            for (j = 0; j < old.iChildCount; j++) {
                uint64_t child; CHECK(xuiDocumentSnapshotGetChild(before, item, j, &child) == XUI_OK &&
                    xuiDocumentSnapshotGetNode(after, child, &now) == XUI_OK && now.iParentId == list.iParentId);
                same_tree(before, child, after, child); boundary_child_ids(before, after, child);
            }
        }
#ifndef XUI_DLL
        CHECK(unwrap_shared_bytes(before->state->source, after->state->source) == strlen(source) - removed);
#else
        (void)removed;
#endif
        reload = open_md(actual); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(after, full);
#endif
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); count++;
    }
    printf("Unlist source: %u exact source/selection/direction/affinity/EOL cases, stable promoted and untouched IDs, maps, full reload and history passed\n", count);
}
static void unlist_source_failures(void)
{
    const unsigned samples[] = {0, 3, 4, 5, 7, 20, 21, 23, 25, 30}; unsigned sample; long total = 0;
    for (sample = 0; sample < sizeof(samples) / sizeof(samples[0]); sample++) {
        unsigned ci = samples[sample]; long point; int success = 0;
        for (point = 0; point < 10000 && !success; point++) {
            failing_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_transaction t = NULL; xui_document_snapshot before, after;
            xui_doc_range_t range, mapped; xui_doc_node_info_t list = {0};
            char actual[8192]; unsigned live; uint64_t revision, first, end; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = fail_alloc; desc.onFree = fail_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, unlist_cases[ci].source, strlen(unlist_cases[ci].source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = unlist_selection(d, before, &unlist_cases[ci], 0, 0, 0, &list, &first, &end);
            live = a.live; revision = xuiDocumentGetRevision(d); a.remaining = point;
            result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (!result) result = xuiDocumentTxnUnlistRange(t, &range, &mapped);
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            xuiDocumentTxnRelease(t); a.remaining = -1;
            if (result) CHECK(result == XUI_ERROR_OUT_OF_MEMORY && a.live == live &&
                xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) {
                success = 1; copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unlist_cases[ci].expected));
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            } else { same_tree(before, 1, after, 1); same_syntax(before, after); }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unlist_cases[ci].source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success); total += point - 1; printf("Unlist source OOM %u: %ld points, atomic source/tree/history and no leaks\n", ci, point - 1);
    }
    printf("Unlist source OOM total: %ld\n", total);
}
#ifndef XUI_DLL
static void unlist_source_cancellation(void)
{
    const unsigned samples[] = {0, 3, 4, 5, 7, 20, 21, 23, 25, 30}; unsigned sample; long total = 0;
    for (sample = 0; sample < sizeof(samples) / sizeof(samples[0]); sample++) {
        unsigned ci = samples[sample]; long point; int success = 0;
        for (point = 0; point < 10000 && !success; point++) {
            prefix_cancel_allocator a = {{-1, 0}, -1, ATOMIC_VAR_INIT(0)}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_transaction t; xui_document_snapshot before, after;
            xui_doc_range_t range, mapped; xui_doc_node_info_t list = {0};
            char actual[8192]; unsigned live; uint64_t revision, first, end; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = prefix_cancel_alloc; desc.onFree = prefix_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, unlist_cases[ci].source, strlen(unlist_cases[ci].source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = unlist_selection(d, before, &unlist_cases[ci], 0, 0, 0, &list, &first, &end);
            live = a.allocation.live; revision = xuiDocumentGetRevision(d);
            CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); t->cancellation = &a.cancellation; a.remaining = point;
            result = xuiDocumentTxnUnlistRange(t, &range, &mapped);
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            a.remaining = -1; xuiDocumentTxnRelease(t); atomic_store(&a.cancellation, 0);
            if (result) CHECK(result == XUI_DOC_ERROR_CANCELLED && a.allocation.live == live &&
                xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) {
                success = 1; copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unlist_cases[ci].expected));
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            } else { same_tree(before, 1, after, 1); same_syntax(before, after); reference_cache_snapshot_equal(before, after); }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unlist_cases[ci].source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.allocation.live);
        }
        CHECK(success); total += point - 1; printf("Unlist source cancellation %u: %ld checkpoints, atomic state and no leaks\n", ci, point - 1);
    }
    printf("Unlist source cancellation total: %ld\n", total);
}
#endif

typedef struct unlist_empty_case { const char* source; const char* expected; unsigned empty_lines; } unlist_empty_case;
static const unlist_empty_case unlist_empty_cases[] = {
    {"- before &amp; raw\n- \n+ \n* \n- \n- last &amp; raw\n", "- before &amp; raw\n\n+ last &amp; raw\n", 4},
    {"* before &amp; raw\n* \n+ \n- \n* \n* last &amp; raw", "* before &amp; raw\n\n+ last &amp; raw", 4},
    {"- [x] before &amp; raw\n- [ ]\n+ [ ]\n* [x]\n- [ ]\n- [x] last &amp; raw\n", "- [x] before &amp; raw\n\n+ [x] last &amp; raw\n", 4},
    {"9. before &amp; raw\n10. \n1) \n1. \n1) \n99. \n100. last &amp; raw\n", "9. before &amp; raw\n\n100) last &amp; raw\n", 5},
    {"- before &amp; raw\n- \n+ \n* \n- \n- last &amp; raw\n- after 中文 🧡 &amp; raw\n", "- before &amp; raw\n\n+ last &amp; raw\n+ after 中文 🧡 &amp; raw\n", 4},
    {"+ before &amp; raw\n+ \n* \n- \n+ \n+ last &amp; raw\n", "+ before &amp; raw\n\n- last &amp; raw\n", 4},
};
static void unlist_empty_wrap(const char* input, const char* header, const char* prefix,
    unsigned blanks, char* output, size_t capacity)
{
    size_t at = strlen(header), i, prefix_bytes = strlen(prefix);
    CHECK(at < capacity); memcpy(output, header, at);
    for (i = 0; input[i];) {
        unsigned repeated = input[i] == '\n' && blanks ? blanks : 1, n;
        for (n = 0; n < repeated; n++) {
            CHECK(at + prefix_bytes + 2 < capacity); memcpy(output + at, prefix, prefix_bytes); at += prefix_bytes;
            if (input[i] == '\n') output[at++] = '\n';
        }
        if (input[i] == '\n') { i++; continue; }
        while (input[i] && input[i] != '\n') { CHECK(at + 2 < capacity); output[at++] = input[i++]; }
        if (input[i]) output[at++] = input[i++];
    }
    output[at] = 0;
}
static xui_doc_range_t unlist_empty_selection(xui_document d, xui_document_snapshot before,
    unsigned affinity, unsigned reverse, xui_doc_node_info_t* first_survivor, xui_doc_node_info_t* last_survivor,
    uint64_t* parent_id)
{
    xui_doc_range_t range = find_text(d, "before"); xui_doc_node_info_t node = {0}, list = {0};
    uint64_t first_empty, last_empty, index;
    node.iSize = list.iSize = sizeof(node);
    *first_survivor = unlist_item_node(before, range.tAnchor.iNodeId);
    *last_survivor = unlist_item_node(before, find_text(d, "last").tAnchor.iNodeId);
    CHECK(xuiDocumentSnapshotGetNode(before, first_survivor->iParentId, &list) == XUI_OK);
    if (list.iChildCount != 2) { char source[8192]; copy_source(d, source, sizeof(source)); fprintf(stderr, "Cross empty first list has %llu items:\n%s\n", (unsigned long long)list.iChildCount, source); }
    CHECK(list.iChildCount == 2 && xuiDocumentSnapshotGetChild(before, list.iId, 1, &first_empty) == XUI_OK);
    *parent_id = list.iParentId;
    CHECK(xuiDocumentSnapshotGetNode(before, last_survivor->iParentId, &list) == XUI_OK && list.iParentId == *parent_id);
    for (index = 0; index < list.iChildCount; index++) {
        uint64_t id; CHECK(xuiDocumentSnapshotGetChild(before, list.iId, index, &id) == XUI_OK); if (id == last_survivor->iId) break;
    }
    CHECK(index > 0 && index < list.iChildCount && xuiDocumentSnapshotGetChild(before, list.iId, index - 1, &last_empty) == XUI_OK &&
        xuiDocumentSnapshotGetNode(before, last_empty, &node) == XUI_OK);
    range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_GAP;
    range.tAnchor.iNodeId = first_empty; range.tCaret.iNodeId = last_empty;
    range.tAnchor.iOffset = 0; range.tCaret.iOffset = node.iChildCount;
    range.tAnchor.iAffinity = affinity & 1; range.tCaret.iAffinity = affinity >> 1;
    if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
    return range;
}
static void unlist_cross_empty(int use_editor)
{
    unsigned ci, wrapper, eol, affinity, reverse, count = 0;
#ifdef XUI_QUOTE_TEST_EDITOR
    xui_test_proxy_state_t proxy; xui_context context = NULL; xui_font font = NULL;
    if (use_editor) {
        xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
            proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    }
#else
    CHECK(!use_editor);
#endif
    for (ci = 0; ci < sizeof(unlist_empty_cases) / sizeof(unlist_empty_cases[0]); ci++)
    for (wrapper = 0; wrapper < 5; wrapper++) for (eol = 0; eol < 3; eol++)
    for (affinity = 0; affinity < 4; affinity++) for (reverse = 0; reverse < 2; reverse++) {
        const char *header = wrapper == 2 ? "- parent &amp; raw\n  \n" : wrapper == 3 ? "use[^n]\n\n[^n]:\n" : "";
        char prefix[256] = "", raw[8192], raw_golden[8192], source[8192], expected[8192], actual[8192];
        xui_document d, reload; xui_document_snapshot before, after, full;
        xui_document_transaction t; xui_doc_range_t range, mapped;
        xui_doc_node_info_t first, last; uint64_t parent; int order, result;
#ifdef XUI_QUOTE_TEST_EDITOR
        xui_widget editor = NULL;
#endif
        if (wrapper == 1 || wrapper == 4) {
            unsigned n, depth = wrapper == 4 ? 123 : 1;
            for (n = 0; n < depth; n++) { prefix[n * 2] = '>'; prefix[n * 2 + 1] = ' '; } prefix[depth * 2] = 0;
        } else if (wrapper == 2 || wrapper == 3) strcpy(prefix, wrapper == 2 ? "  " : "    ");
        unlist_empty_wrap(unlist_empty_cases[ci].source, header, prefix, 0, raw, sizeof(raw));
        unlist_empty_wrap(unlist_empty_cases[ci].expected, header, prefix,
            wrapper ? unlist_empty_cases[ci].empty_lines : 0, raw_golden, sizeof(raw_golden));
        boundary_variant(raw, eol, source, sizeof(source)); boundary_variant(raw_golden, eol, expected, sizeof(expected));
        d = open_md(source); CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        range = unlist_empty_selection(d, before, affinity, reverse, &first, &last, &parent);
#ifdef XUI_QUOTE_TEST_EDITOR
        if (use_editor) {
            xui_doc_editor_desc_t desc = {0}; xui_doc_command_state_t state = {0}; xui_doc_node_info_t list = {0}; uint32_t command;
            list.iSize = sizeof(list); CHECK(xuiDocumentSnapshotGetNode(before, first.iParentId, &list) == XUI_OK);
            command = ci == 2 ? XUI_DOC_EDIT_TASK_LIST : list.tAttributes.iFlags & XUI_DOC_ORDERED ? XUI_DOC_EDIT_NUMBER_LIST : XUI_DOC_EDIT_BULLET_LIST;
            desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d; desc.iMode = XUI_DOC_VISUAL;
            state.iSize = sizeof(state);
            CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
                xuiWidgetSetRect(editor, (xui_rect_t){0,0,640,480}) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK &&
                xuiDocumentEditorQueryCommand(editor, command, &state) == XUI_OK && state.bEnabled && state.bActive);
            result = xuiDocumentEditorExecute(editor, command);
            if (!result) CHECK(xuiDocumentViewGetSelection(editor, &mapped) == XUI_OK && xuiDocumentViewSetSelection(editor, &mapped) == XUI_OK);
        } else
#endif
        {
            CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
            result = xuiDocumentTxnUnlistRange(t, &range, &mapped); if (!result) result = xuiDocumentTxnCommit(t, NULL);
            xuiDocumentTxnRelease(t); mapped.tAnchor.iRevision = mapped.tCaret.iRevision = xuiDocumentGetRevision(d);
        }
        if (result) fprintf(stderr, "Cross empty %u/%u/%u/%u/%u: %d\n", ci, wrapper, eol, affinity, reverse, result);
        CHECK(!result && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK &&
            xuiDocumentSnapshotComparePositions(after, &mapped.tAnchor, &mapped.tCaret, &order) == XUI_OK && !order &&
            mapped.tAnchor.iNodeId == parent && mapped.tCaret.iNodeId == parent &&
            mapped.tAnchor.iAffinity == range.tAnchor.iAffinity && mapped.tCaret.iAffinity == range.tCaret.iAffinity);
        copy_source(d, actual, sizeof(actual));
        if (strcmp(actual, expected)) fprintf(stderr, "Cross empty %u/%u/%u/%u/%u source:\n%s\nEXPECTED\n%s\n", ci, wrapper, eol, affinity, reverse, actual, expected);
        CHECK(!strcmp(actual, expected));
        same_tree(before, first.iId, after, first.iId); boundary_child_ids(before, after, first.iId);
        same_tree(before, last.iId, after, last.iId); boundary_child_ids(before, after, last.iId);
        reload = open_md(actual); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(after, full);
#endif
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
#ifdef XUI_QUOTE_TEST_EDITOR
        if (use_editor) xuiWidgetDestroy(editor);
#endif
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); count++;
    }
#ifdef XUI_QUOTE_TEST_EDITOR
    if (use_editor) { xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font); }
#endif
    printf("Unlist cross-empty: %u exact source/affinity/direction/EOL root/quote/list/footnote/123-depth, stable surviving IDs and history cases%s\n",
        count, use_editor ? " in actual DLL Editor" : "");
}

#ifndef XUI_DLL
static void unlist_cross_empty_faults(void)
{
    unsigned cancel, sample; long counts[2] = {0};
    for (cancel = 0; cancel < 2; cancel++) for (sample = 0; sample < 4; sample++) {
        unsigned ci = sample & 1 ? 3 : 0; const char* header = sample < 2 ? "" : "use[^n]\n\n[^n]:\n";
        const char* prefix = sample < 2 ? "" : "    "; long point; int success = 0;
        char source[8192], expected[8192];
        unlist_empty_wrap(unlist_empty_cases[ci].source, header, prefix, 0, source, sizeof(source));
        unlist_empty_wrap(unlist_empty_cases[ci].expected, header, prefix,
            sample < 2 ? 0 : unlist_empty_cases[ci].empty_lines, expected, sizeof(expected));
        for (point = 0; point < 10000 && !success; point++) {
            prefix_cancel_allocator a = {{-1,0}, -1, ATOMIC_VAR_INIT(0)}; xui_doc_desc_t desc = {0};
            xui_document d; xui_document_snapshot before, after; xui_document_transaction t = NULL;
            xui_doc_range_t range, mapped; xui_doc_node_info_t first, last;
            uint64_t parent, revision; unsigned live; int result; char actual[8192];
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = prefix_cancel_alloc; desc.onFree = prefix_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = unlist_empty_selection(d, before, 2, 1, &first, &last, &parent);
            revision = xuiDocumentGetRevision(d); live = a.allocation.live;
            if (cancel) {
                CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
                t->cancellation = &a.cancellation; a.remaining = point; result = XUI_OK;
            } else { a.allocation.remaining = point; result = xuiDocumentBeginTransaction(d, NULL, &t); }
            if (!result) result = xuiDocumentTxnUnlistRange(t, &range, &mapped);
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            a.remaining = a.allocation.remaining = -1; xuiDocumentTxnRelease(t); atomic_store(&a.cancellation, 0);
            if (result) CHECK(result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY) &&
                a.allocation.live == live && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) {
                success = 1; copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            } else {
                same_tree(before, 1, after, 1); same_syntax(before, after);
                boundary_child_ids(before, after, 1); reference_cache_snapshot_equal(before, after);
            }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.allocation.live);
        }
        CHECK(success); counts[cancel] += point - 1;
        printf("Unlist cross-empty %s %u: %ld checkpoints, atomic source/tree/cache/revision/history and no leaks\n",
            cancel ? "cancellation" : "OOM", sample, point - 1);
    }
    printf("Unlist cross-empty faults: %ld OOM, %ld cancellation checkpoints\n", counts[0], counts[1]);
}
#endif

static xui_doc_node_info_t unlist_cross_item(xui_document_snapshot s, uint64_t id)
{
    xui_doc_node_info_t info = {0}; info.iSize = sizeof(info);
    do { CHECK(xuiDocumentSnapshotGetNode(s, id, &info) == XUI_OK); id = info.iParentId; }
    while (id && info.iKind != XUI_DOC_LIST_ITEM);
    CHECK(info.iKind == XUI_DOC_LIST_ITEM); return info;
}
static void unlist_cross_partial_gaps(int use_editor)
{
    const char* source = "- before &amp; raw\n- take &amp; raw\n\n  ~~~~ lang opaque\n  code\\raw\n  ~~~~\n\n1. other &amp; raw\n\n   <div>HTML &amp; raw</div>\n\n2. last &amp; raw\n";
    const char* expected = "- before &amp; raw\n\ntake &amp; raw\n\n~~~~ lang opaque\ncode\\raw\n~~~~\n\nother &amp; raw\n\n<div>HTML &amp; raw</div>\n\n2. last &amp; raw\n";
    unsigned profile, eol, a, b, reverse, affinity, count = 0;
#ifdef XUI_QUOTE_TEST_EDITOR
    xui_test_proxy_state_t proxy; xui_context context = NULL; xui_font font = NULL;
    if (use_editor) {
        source = "- before &amp; raw\n- take &amp; raw\n\n  ~~~~ lang opaque\n  code\\raw\n  ~~~~\n\n+ other &amp; raw\n\n  <div>HTML &amp; raw</div>\n\n+ last &amp; raw\n";
        expected = "- before &amp; raw\n\ntake &amp; raw\n\n~~~~ lang opaque\ncode\\raw\n~~~~\n\nother &amp; raw\n\n<div>HTML &amp; raw</div>\n\n+ last &amp; raw\n";
        xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
            proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    }
#else
    CHECK(!use_editor);
#endif

    for (profile = 0; profile < 2; profile++) for (eol = 0; eol < 3; eol++)
    for (a = 0; a <= 2; a++) for (b = 0; b <= 2; b++)
    for (reverse = 0; reverse < 2; reverse++) for (affinity = 0; affinity < 4; affinity++) {
        xui_document d, rich, reload; xui_document_snapshot before, after, full;
        xui_document_transaction t; xui_doc_range_t range, mapped;
        xui_doc_node_info_t first, last, node = {0}; uint64_t i, child; int order;
        char original[8192], golden[8192], actual[8192];
#ifdef XUI_QUOTE_TEST_EDITOR
        xui_widget editor = NULL;
#endif
        boundary_variant(source, eol, original, sizeof(original)); boundary_variant(expected, eol, golden, sizeof(golden));
        d = open_md(original);
        if (profile) {
            xui_doc_rich_conversion_report_t report = {0}; report.iSize = sizeof(report);
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentSnapshotAnalyzeRichConversion(before, &report) == XUI_OK &&
                xuiDocumentSnapshotConvertToRich(before, report.iReasons, &rich) == XUI_OK);
            xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = rich;
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        first = unlist_cross_item(before, find_text(d, "take").tAnchor.iNodeId);
        last = unlist_cross_item(before, find_text(d, "other").tAnchor.iNodeId);
        CHECK(first.iParentId != last.iParentId && first.iChildCount == 2 && last.iChildCount == 2);
        range.tAnchor = find_text(d, "take").tAnchor; range.tCaret = find_text(d, "other").tAnchor;
        range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_GAP;
        range.tAnchor.iNodeId = first.iId; range.tCaret.iNodeId = last.iId;
        range.tAnchor.iOffset = a; range.tCaret.iOffset = b;
        range.tAnchor.iAffinity = affinity & 1; range.tCaret.iAffinity = affinity >> 1;
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
#ifdef XUI_QUOTE_TEST_EDITOR
        if (use_editor) {
            xui_doc_editor_desc_t desc = {0}; xui_doc_command_state_t state = {0};
            desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d; desc.iMode = XUI_DOC_VISUAL;
            state.iSize = sizeof(state);
            CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
                xuiWidgetSetRect(editor, (xui_rect_t){0,0,640,480}) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK &&
                xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BULLET_LIST, &state) == XUI_OK && state.bEnabled && state.bActive &&
                xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BULLET_LIST) == XUI_OK && xuiDocumentViewGetSelection(editor, &mapped) == XUI_OK &&
                xuiDocumentViewSetSelection(editor, &mapped) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        } else
#endif
        {
            CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnUnlistRange(t, &range, &mapped) == XUI_OK &&
                xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
            mapped.tAnchor.iRevision = mapped.tCaret.iRevision = xuiDocumentGetRevision(d);
        }
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK &&
            xuiDocumentSnapshotComparePositions(after, &mapped.tAnchor, &mapped.tCaret, &order) == XUI_OK &&
            order == (a == 2 && !b ? 0 : reverse ? 1 : -1) &&
            mapped.tAnchor.iNodeId == 1 && mapped.tCaret.iNodeId == 1 &&
            mapped.tAnchor.iKind == XUI_DOC_POSITION_GAP && mapped.tCaret.iKind == XUI_DOC_POSITION_GAP &&
            mapped.tAnchor.iOffset == (reverse ? 3 + b : 1 + a) && mapped.tCaret.iOffset == (reverse ? 1 + a : 3 + b) &&
            mapped.tAnchor.iAffinity == range.tAnchor.iAffinity && mapped.tCaret.iAffinity == range.tCaret.iAffinity);
        node.iSize = sizeof(node);
        CHECK(xuiDocumentSnapshotGetNode(after, first.iId, &node) == XUI_ERROR_NOT_FOUND &&
            xuiDocumentSnapshotGetNode(after, last.iId, &node) == XUI_ERROR_NOT_FOUND &&
            xuiDocumentSnapshotGetNode(after, 1, &node) == XUI_OK && node.iChildCount == 6);
        for (i = 0; i < 4; i++) {
            CHECK(xuiDocumentSnapshotGetChild(before, i < 2 ? first.iId : last.iId, i % 2, &child) == XUI_OK);
            same_tree(before, child, after, child); boundary_child_ids(before, after, child);
        }
        if (!profile) {
            copy_source(d, actual, sizeof(actual));
            if (strcmp(actual, golden)) fprintf(stderr, "Cross partial source:\n%s\nEXPECTED\n%s\n", actual, golden);
            CHECK(!strcmp(actual, golden)); reload = open_md(actual); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
            same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
            reference_cache_snapshot_equal(after, full);
#endif
            xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        }
#ifdef XUI_QUOTE_TEST_EDITOR
        if (use_editor) {
            xui_doc_range_t restored;
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiDocumentViewGetSelection(editor, &restored) == XUI_OK &&
                restored.tAnchor.iNodeId == range.tAnchor.iNodeId && restored.tCaret.iNodeId == range.tCaret.iNodeId &&
                restored.tAnchor.iOffset == range.tAnchor.iOffset && restored.tCaret.iOffset == range.tCaret.iOffset &&
                restored.tAnchor.iAffinity == range.tAnchor.iAffinity && restored.tCaret.iAffinity == range.tCaret.iAffinity &&
                xuiDocumentViewSetSelection(editor, &restored) == XUI_OK && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK &&
                xuiDocumentViewGetSelection(editor, &restored) == XUI_OK && xuiDocumentViewSetSelection(editor, &restored) == XUI_OK);
            CHECK(restored.tAnchor.iNodeId == mapped.tAnchor.iNodeId && restored.tCaret.iNodeId == mapped.tCaret.iNodeId &&
                restored.tAnchor.iOffset == mapped.tAnchor.iOffset && restored.tCaret.iOffset == mapped.tCaret.iOffset &&
                restored.tAnchor.iAffinity == mapped.tAnchor.iAffinity && restored.tCaret.iAffinity == mapped.tCaret.iAffinity);
            xuiWidgetDestroy(editor);
        }
#endif
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &full) == XUI_OK);
        same_tree(before, 1, full, 1); boundary_child_ids(before, full, 1); xuiDocumentSnapshotRelease(full);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &full) == XUI_OK);
        same_tree(after, 1, full, 1); boundary_child_ids(after, full, 1); xuiDocumentSnapshotRelease(full);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); count++;
    }
#ifdef XUI_QUOTE_TEST_EDITOR
    if (use_editor) { xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font); }
#endif
    printf("Unlist cross-item partial gaps: %u Rich/Markdown/EOL/direction/distinct-affinity exact boundary and subtree/history cases%s\n",
        count, use_editor ? " in actual DLL Editor" : "");
}

/* Every original item gap remains the same boundary after its children are
 * promoted. Check interior/collapsed gaps, distinct endpoint affinities, Rich
 * and Markdown, not just the first/end boundaries used by the source matrix. */
static void unlist_partial_gaps(int use_editor)
{
    unsigned profile, ci, a, b, affinity, total = 0;
#ifdef XUI_QUOTE_TEST_EDITOR
    xui_test_proxy_state_t proxy; xui_context context = NULL; xui_font font = NULL;
    if (use_editor) {
        xuiTestProxyInit(&proxy);
        CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
            proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    }
#else
    CHECK(!use_editor);
#endif
    for (profile = 0; profile < 2; profile++)
    for (ci = 0; ci < sizeof(unlist_cases) / sizeof(unlist_cases[0]); ci++) {
        xui_document sample = open_md(unlist_cases[ci].source); xui_document_snapshot snapshot;
        xui_doc_node_info_t selected, list = {0}; uint64_t children;
        CHECK(xuiDocumentAcquireSnapshot(sample, &snapshot) == XUI_OK);
        selected = unlist_item_node(snapshot, find_text(sample, "take").tAnchor.iNodeId);
        list.iSize = sizeof(list); CHECK(xuiDocumentSnapshotGetNode(snapshot, selected.iParentId, &list) == XUI_OK);
        children = selected.iChildCount;
        xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(sample);
        if (!unlist_cases[ci].item && list.iChildCount != 1) continue;
        for (a = 0; a <= children; a++) for (b = 0; b <= children; b++) for (affinity = 0; affinity < 4; affinity++) {
            xui_document d = open_md(unlist_cases[ci].source), reload;
            xui_document_snapshot before, after, full; xui_document_transaction t;
            xui_doc_range_t range, mapped, restored; xui_doc_node_info_t item, parent = {0};
            uint64_t item_index, list_index, i, child, start; char actual[8192]; int order;
#ifdef XUI_QUOTE_TEST_EDITOR
            xui_widget editor = NULL;
#else
            (void)restored;
#endif
            if (profile) {
                xui_document rich; xui_doc_rich_conversion_report_t report = {0}; report.iSize = sizeof(report);
                CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
                    xuiDocumentSnapshotAnalyzeRichConversion(snapshot, &report) == XUI_OK &&
                    xuiDocumentSnapshotConvertToRich(snapshot, report.iReasons, &rich) == XUI_OK);
                xuiDocumentSnapshotRelease(snapshot); xuiDocumentRelease(d); d = rich;
            }
            CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = find_text(d, "take"); item = unlist_item_node(before, range.tAnchor.iNodeId);
            list.iSize = parent.iSize = sizeof(list);
            CHECK(xuiDocumentSnapshotGetNode(before, item.iParentId, &list) == XUI_OK &&
                xuiDocumentSnapshotGetNode(before, list.iParentId, &parent) == XUI_OK && item.iChildCount == children);
            for (item_index = 0; item_index < list.iChildCount; item_index++) {
                CHECK(xuiDocumentSnapshotGetChild(before, list.iId, item_index, &child) == XUI_OK); if (child == item.iId) break;
            }
            for (list_index = 0; list_index < parent.iChildCount; list_index++) {
                CHECK(xuiDocumentSnapshotGetChild(before, parent.iId, list_index, &child) == XUI_OK); if (child == list.iId) break;
            }
            CHECK(item_index < list.iChildCount && list_index < parent.iChildCount);
            start = list_index + !!item_index;
            range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_GAP;
            range.tAnchor.iNodeId = range.tCaret.iNodeId = item.iId;
            range.tAnchor.iOffset = a; range.tCaret.iOffset = b;
            range.tAnchor.iAffinity = affinity & 1; range.tCaret.iAffinity = affinity >> 1;
#ifdef XUI_QUOTE_TEST_EDITOR
            if (use_editor) {
                xui_doc_editor_desc_t desc = {0}; uint32_t command = item.tAttributes.iFlags & XUI_DOC_TASK ? XUI_DOC_EDIT_TASK_LIST :
                    list.tAttributes.iFlags & XUI_DOC_ORDERED ? XUI_DOC_EDIT_NUMBER_LIST : XUI_DOC_EDIT_BULLET_LIST;
                desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d; desc.iMode = XUI_DOC_VISUAL;
                CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
                    xuiWidgetSetRect(editor, (xui_rect_t){0,0,640,480}) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK &&
                    xuiDocumentEditorExecute(editor, command) == XUI_OK && xuiDocumentViewGetSelection(editor, &mapped) == XUI_OK &&
                    xuiDocumentViewSetSelection(editor, &mapped) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
            } else
#endif
            {
                CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnUnlistRange(t, &range, &mapped) == XUI_OK &&
                    xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
                mapped.tAnchor.iRevision = mapped.tCaret.iRevision = xuiDocumentGetRevision(d);
            }
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK &&
                xuiDocumentSnapshotComparePositions(after, &mapped.tAnchor, &mapped.tCaret, &order) == XUI_OK &&
                order == (a < b ? -1 : a != b) &&
                mapped.tAnchor.iKind == XUI_DOC_POSITION_GAP && mapped.tCaret.iKind == XUI_DOC_POSITION_GAP &&
                mapped.tAnchor.iNodeId == parent.iId && mapped.tCaret.iNodeId == parent.iId &&
                mapped.tAnchor.iOffset == start + a && mapped.tCaret.iOffset == start + b &&
                mapped.tAnchor.iAffinity == range.tAnchor.iAffinity && mapped.tCaret.iAffinity == range.tCaret.iAffinity);
            for (i = 0; i < children; i++) {
                CHECK(xuiDocumentSnapshotGetChild(before, item.iId, i, &child) == XUI_OK);
                same_tree(before, child, after, child); boundary_child_ids(before, after, child);
            }
            if (!profile) {
                copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unlist_cases[ci].expected));
                reload = open_md(actual); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
                same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
                reference_cache_snapshot_equal(after, full);
#endif
                xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
            }
#ifdef XUI_QUOTE_TEST_EDITOR
            if (use_editor) {
                CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiDocumentViewGetSelection(editor, &restored) == XUI_OK &&
                    restored.tAnchor.iNodeId == item.iId && restored.tCaret.iNodeId == item.iId &&
                    restored.tAnchor.iOffset == a && restored.tCaret.iOffset == b &&
                    restored.tAnchor.iAffinity == range.tAnchor.iAffinity && restored.tCaret.iAffinity == range.tCaret.iAffinity &&
                    xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK && xuiDocumentViewGetSelection(editor, &restored) == XUI_OK &&
                    restored.tAnchor.iNodeId == parent.iId && restored.tCaret.iNodeId == parent.iId &&
                    restored.tAnchor.iOffset == start + a && restored.tCaret.iOffset == start + b &&
                    restored.tAnchor.iAffinity == range.tAnchor.iAffinity && restored.tCaret.iAffinity == range.tCaret.iAffinity);
                xuiWidgetDestroy(editor);
            } else
#endif
            {
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &full) == XUI_OK);
                same_tree(before, 1, full, 1); boundary_child_ids(before, full, 1); xuiDocumentSnapshotRelease(full);
                CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &full) == XUI_OK);
                same_tree(after, 1, full, 1); boundary_child_ids(after, full, 1); xuiDocumentSnapshotRelease(full);
            }
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); total++;
        }
    }
#ifdef XUI_QUOTE_TEST_EDITOR
    if (use_editor) { xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font); }
#endif
    printf("Unlist partial item gaps: %u Rich/Markdown interior/collapsed/direction/distinct-affinity and stable subtree/history cases%s\n",
        total, use_editor ? " in actual DLL Editor" : "");
}
#ifdef XUI_QUOTE_TEST_EDITOR
static void unlist_source_editor(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    unsigned ci, kind, mode, affinity, reverse, total = 0, refused = 0;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    for (ci = 0; ci < sizeof(unlist_cases) / sizeof(unlist_cases[0]); ci++)
    for (kind = 0; kind < 4; kind++)
    for (mode = 0; mode < 3; mode++)
    for (affinity = 0; affinity < 2; affinity++)
    for (reverse = 0; reverse < 2; reverse++) {
        xui_document d = open_md(unlist_cases[ci].source); xui_document_snapshot before;
        xui_doc_range_t range, selected; xui_doc_node_info_t list = {0}, item = {0}; xui_widget editor;
        xui_doc_editor_desc_t desc = {0}; xui_doc_command_state_t state = {0};
        char actual[8192]; uint64_t revision, first, end, item_id; uint32_t command;
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        if (kind == 3 && !unlist_cases[ci].item) {
            item = unlist_item_node(before, find_text(d, "take").tAnchor.iNodeId);
            list.iSize = sizeof(list); CHECK(xuiDocumentSnapshotGetNode(before, item.iParentId, &list) == XUI_OK);
            if (list.iChildCount != 1) { xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); continue; }
        }
        range = unlist_selection(d, before, &unlist_cases[ci], kind, affinity, reverse, &list, &first, &end);
        item.iSize = sizeof(item); CHECK(xuiDocumentSnapshotGetChild(before, list.iId, first, &item_id) == XUI_OK &&
            xuiDocumentSnapshotGetNode(before, item_id, &item) == XUI_OK);
        command = item.tAttributes.iFlags & XUI_DOC_TASK ? XUI_DOC_EDIT_TASK_LIST :
            list.tAttributes.iFlags & XUI_DOC_ORDERED ? XUI_DOC_EDIT_NUMBER_LIST : XUI_DOC_EDIT_BULLET_LIST;
        xuiDocumentSnapshotRelease(before); revision = xuiDocumentGetRevision(d);
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d;
        desc.iMode = mode == 2 ? XUI_DOC_SOURCE_TEXT : mode == 1 ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_VISUAL;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK && xuiWidgetSetRect(editor, (xui_rect_t){0,0,640,480}) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        if (mode) {
            state.iSize = sizeof(state);
            CHECK(xuiDocumentEditorQueryCommand(editor, command, &state) == XUI_OK && !state.bEnabled &&
                state.iDisabledReason == XUI_ERROR_UNSUPPORTED && xuiDocumentEditorExecute(editor, command) == XUI_ERROR_UNSUPPORTED &&
                xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unlist_cases[ci].source)); refused++;
            CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        }
        state.iSize = sizeof(state); CHECK(xuiDocumentEditorQueryCommand(editor, command, &state) == XUI_OK && state.bEnabled && state.bActive);
        CHECK(xuiDocumentEditorExecute(editor, command) == XUI_OK && xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &selected) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unlist_cases[ci].expected));
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
            xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unlist_cases[ci].source));
        CHECK(!xuiDocumentCanUndo(d) && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unlist_cases[ci].expected));
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); total++;
    }
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Unlist source Editor: %u actual DLL VISUAL Query/Execute/Caret cases, %u LIVE/SOURCE atomic refusals and three-mode history\n", total, refused);
}
#endif

#ifndef XUI_UNLIST_SOURCE_NO_MAIN
int main(void) {
    unlist_source_matrix(); unlist_partial_gaps(0); unlist_cross_partial_gaps(0); unlist_cross_empty(0); unlist_source_failures();
#ifndef XUI_DLL
    unlist_source_cancellation(); unlist_cross_empty_faults();
#endif
#ifdef XUI_QUOTE_TEST_EDITOR
    unlist_source_editor(); unlist_partial_gaps(1); unlist_cross_partial_gaps(1); unlist_cross_empty(1);
#endif
    return 0;
}

#endif
