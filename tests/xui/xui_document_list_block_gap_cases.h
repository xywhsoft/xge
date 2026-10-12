/* Independent source goldens for middle and terminal block frontiers.
 * Complete semantic attrs/resources are compared, with stable indented code. */
static const struct block_gap_case { const char *before, *typed; unsigned slot; } block_gap_cases[] = {
    {"- *A* &amp;\n\n      code &amp;\n", "- *A* &amp;\n\n  Z\n\n      code &amp;\n", 1},
    {"9) *A* &amp;\n\n       code &amp;\n", "9) *A* &amp;\n\n   Z\n\n       code &amp;\n", 1},
    {"use[^n]\n\n[^n]: - *A* &amp;\n\n          code &amp;\n", "use[^n]\n\n[^n]: - *A* &amp;\n\n      Z\n\n          code &amp;\n", 1},
    {"> - *A* &amp;\n>\n>       code &amp;\n", "> - *A* &amp;\n>\n>   Z\n>\n>       code &amp;\n", 1},
    {"- *A* &amp;\n\n      code &amp;\n", "- *A* &amp;\n\n      code &amp;\n\n  Z\n", 2},
    {"9) *A* &amp;\n\n       code &amp;\n", "9) *A* &amp;\n\n       code &amp;\n\n   Z\n", 2},
    {"use[^n]\n\n[^n]: - *A* &amp;\n\n          code &amp;\n", "use[^n]\n\n[^n]: - *A* &amp;\n\n          code &amp;\n\n      Z\n", 2},
    {"> - *A* &amp;\n>\n>       code &amp;\n", "> - *A* &amp;\n>\n>       code &amp;\n>\n>   Z\n", 2}
};
static uint64_t block_gap_item(xui_document_snapshot s, const char* source)
{
    xui_doc_position_t position; xui_doc_node_info_t node = {0}; int quality;
    TABLE_CHECK(xuiDocumentSourceToPosition(s, (uint64_t)(strstr(source, "code") - source) + 2,
        &position, &quality) == XUI_OK);
    uint64_t id = position.iNodeId; node.iSize = sizeof(node);
    for (;;) {
        TABLE_CHECK(xuiDocumentSnapshotGetNode(s, id, &node) == XUI_OK);
        if (node.iKind == XUI_DOC_LIST_ITEM) return id;
        id = node.iParentId; TABLE_CHECK(id);
    }
}
static int block_gap_ancestor(xui_document_snapshot s, uint64_t item, uint64_t id)
{
    xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
    for (;;) {
        TABLE_CHECK(xuiDocumentSnapshotGetNode(s, item, &node) == XUI_OK);
        if (item == id) return 1;
        item = node.iParentId; if (!item) return 0;
    }
}
static void block_gap_change(xui_document_change_set change, xui_document_snapshot old,
    uint64_t item, unsigned expected_attributes)
{
    xui_doc_change_info_t info = {0}; unsigned attributes = 0, patches = 0;
    info.iSize = sizeof(info); TABLE_CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
    TABLE_CHECK(info.iDomain == XUI_DOC_SEMANTIC && info.iAfterRevision == info.iBeforeRevision + 1);
    for (uint64_t i = 0; i < info.iOperationCount; i++) {
        const xui_doc_operation_t* op = info.pOperations + i;
        if (op->iKind == XUI_DOC_OP_SOURCE) patches++;
        if (op->iKind == XUI_DOC_OP_ATTRIBUTES) {
            xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
            if (xuiDocumentSnapshotGetNode(old, op->iNodeId, &node) == XUI_OK && node.iKind == XUI_DOC_LIST) {
                TABLE_CHECK(block_gap_ancestor(old, item, op->iNodeId)); attributes++;
            }
        }
    }
    TABLE_CHECK(patches && attributes == expected_attributes);
}
static void block_gap_notify(xui_document d, xui_document_change_set change, void* user)
{
    xui_doc_change_info_t info = {0}; info.iSize = sizeof(info);
    TABLE_CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK &&
    info.iAfterRevision == xuiDocumentGetRevision(d)); (*(unsigned*)user)++;
}
static void block_gap_equivalent(xui_document_snapshot current, const char* source)
{
    xui_document oracle = table_open(source, NULL); xui_document_snapshot full;
    TABLE_CHECK(xuiDocumentAcquireSnapshot(oracle, &full) == XUI_OK);
    formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, full, XUI_DOCUMENT_ROOT);
    xuiDocumentSnapshotRelease(full); xuiDocumentRelease(oracle);
    integration_equivalent(current, source);
}
static char* block_gap_source(unsigned fixture, unsigned eol, unsigned bom, unsigned eof, int after)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    char* source = delete_ending(after ? block_gap_cases[fixture].typed : block_gap_cases[fixture].before, endings[eol], bom);
    if (eof) source[strlen(source) - strlen(endings[eol])] = 0;
    return source;
}
static void list_block_gap_goldens(void)
{
    unsigned total = 0;
    for (unsigned fixture = 0; fixture < sizeof(block_gap_cases)/sizeof(*block_gap_cases); fixture++)
    for (unsigned eol = 0; eol < 3; eol++) for (unsigned bom = 0; bom < 2; bom++)
    for (unsigned eof = 0; eof < 2; eof++) for (unsigned continuation = 0; continuation < 3; continuation++) {
        char* before = block_gap_source(fixture, eol, bom, eof, 0);
        char* typed = block_gap_source(fixture, eol, bom, eof, 1);
        char* final = malloc(strlen(typed) + 2); TABLE_CHECK(final); strcpy(final, typed);
        if (continuation) { char* z = strchr(final, 'Z'); TABLE_CHECK(z); memmove(z+2, z+1, strlen(z+1)+1); z[1] = 'Q'; }
        table_allocator a = {-1, 0}; xui_document d = table_open(before, &a);
        xui_document_snapshot old, first, current, restored; xui_document_transaction t;
        xui_document_change_set change; xui_doc_position_t at = {0}, caret; uint64_t token, offset;
        unsigned notifications = 0; int quality;
        printf("Middle gap fixture=%u eol=%u bom=%u eof=%u continuation=%u\n", fixture, eol, bom, eof, continuation); fflush(stdout);
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
            xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, block_gap_notify, &notifications, &token) == XUI_OK);
        uint64_t item = block_gap_item(old, before); at.iSize = sizeof(at); at.iDocumentId = xuiDocumentGetIdentity(d);
        at.iKind = XUI_DOC_POSITION_GAP; at.iNodeId = item; at.iOffset = block_gap_cases[fixture].slot;
        at.iRevision = xuiDocumentGetRevision(d); at.iAffinity = XUI_DOC_AFTER; xui_doc_range_t range = {at, at};
        TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnReplaceRange(t, &range, "Z", 1, &caret) == XUI_OK && !notifications);
        if (continuation == 1) { range.tAnchor = range.tCaret = caret;
            TABLE_CHECK(xuiDocumentTxnReplaceRange(t, &range, "Q", 1, &caret) == XUI_OK && !notifications); }
        TABLE_CHECK(xuiDocumentTxnCommit(t, &change) == XUI_OK && notifications == 1); xuiDocumentTxnRelease(t);
        block_gap_change(change, old, item, 0); xuiDocumentChangeSetRelease(change);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &first) == XUI_OK); table_source(first, continuation == 1 ? final : typed);
        if (continuation == 2) {
            caret.iRevision = xuiDocumentGetRevision(d); range.tAnchor = range.tCaret = caret;
            TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
                xuiDocumentTxnReplaceRange(t, &range, "Q", 1, &caret) == XUI_OK && notifications == 1 &&
                xuiDocumentTxnCommit(t, &change) == XUI_OK && notifications == 2); xuiDocumentTxnRelease(t);
            block_gap_change(change, first, item, 0); xuiDocumentChangeSetRelease(change);
        }
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK); table_source(current, final); block_gap_equivalent(current, final);
        /* Public syntax and semantic identity of the untouched code both survive. */
        uint64_t code; xui_doc_node_info_t n = {0}; xui_doc_block_syntax_t syntax = {0}; char ac[1024], bc[1024]; uint64_t an, bn;
        n.iSize = sizeof(n); syntax.iSize = sizeof(syntax);
        TABLE_CHECK(xuiDocumentSnapshotGetNode(old, item, &n) == XUI_OK);
        uint64_t list = n.iParentId;
        TABLE_CHECK(xuiDocumentSnapshotGetNode(old, list, &n) == XUI_OK && !(n.tAttributes.iFlags & XUI_DOC_TIGHT) &&
            xuiDocumentSnapshotGetNode(current, list, &n) == XUI_OK && !(n.tAttributes.iFlags & XUI_DOC_TIGHT));
        TABLE_CHECK(xuiDocumentSnapshotGetChild(old, item, 1, &code) == XUI_OK &&
            xuiDocumentSnapshotGetBlockSyntax(old, code, &syntax) == XUI_OK && syntax.iKind == XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE &&
            xuiDocumentSnapshotGetNode(current, code, &n) == XUI_OK && n.iKind == XUI_DOC_CODE_BLOCK &&
            xuiDocumentSnapshotGetBlockSyntax(current, code, &syntax) == XUI_OK && syntax.iKind == XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE &&
            xuiDocumentSnapshotCopyText(old, code, ac, sizeof(ac), &an) == XUI_OK &&
            xuiDocumentSnapshotCopyText(current, code, bc, sizeof(bc), &bn) == XUI_OK && an == bn && !memcmp(ac, bc, (size_t)an));
        caret.iRevision = xuiDocumentGetRevision(d); TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &offset, &quality) == XUI_OK &&
            quality == XUI_DOC_MAP_EXACT && offset == (uint64_t)(strchr(final, 'Z') - final) + 1 + !!continuation);
        unsigned steps = continuation == 2 ? 2 : 1;
        for (unsigned i = 0; i < steps; i++) TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
        TABLE_CHECK(!xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
        for (unsigned i = 0; i < steps; i++) TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
        TABLE_CHECK(!xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, final); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(notifications == steps * 3); table_source(old, before); table_source(first, continuation == 1 ? final : typed);
        xuiDocumentUnsubscribe(d, token); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(first); xuiDocumentSnapshotRelease(current);
        xuiDocumentRelease(d); TABLE_CHECK(!a.live); free(before); free(typed); free(final); total++;
    }
    TABLE_CHECK(total == 288); printf("Middle gap CST: %u independent raw/full-semantic/index/stable-code/exact-caret/attribute-operation/notification/history/zero-leak goldens passed\n", total);
}

static void list_block_gap_faults(void)
{
    unsigned modes = 1;
#ifndef XUI_DLL
    modes = 2;
#endif
    for (unsigned fixture = 0; fixture < 8; fixture++)
    for (unsigned continued = 0; continued < 2; continued++) for (unsigned cancel = 0; cancel < modes; cancel++) {
        unsigned failures = 0; long point;
        const char* before = block_gap_cases[fixture].before;
        char expected[4096]; strcpy(expected, block_gap_cases[fixture].typed);
        if (continued) { char* z = strchr(expected, 'Z'); TABLE_CHECK(z); memmove(z+2, z+1, strlen(z+1)+1); z[1] = 'Q'; }
        for (point = 0; point < 10000; point++) {
            list_edge_allocator a = {0}; a.allocation.remaining = -1;
#ifndef XUI_DLL
            a.cancel_after = -1; atomic_init(&a.cancellation, 0);
#endif
            xui_doc_desc_t desc = {0}; desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.iMarkdownDialect = XUI_MD_EXTENDED; desc.onAlloc = list_edge_alloc; desc.onFree = list_edge_free; desc.pAllocatorUser = &a;
            xui_document d; xui_document_snapshot old, current, restored; xui_document_transaction t;
            xui_document_change_set change = NULL; xui_doc_position_t at = {0}, caret;
            unsigned notifications = 0; uint64_t token, revision; int result;
            TABLE_CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, before, strlen(before)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
                xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, block_gap_notify, &notifications, &token) == XUI_OK);
            uint64_t item = block_gap_item(old, before); revision = xuiDocumentGetRevision(d);
            at.iSize = sizeof(at); at.iDocumentId = xuiDocumentGetIdentity(d); at.iRevision = revision;
            at.iKind = XUI_DOC_POSITION_GAP; at.iNodeId = item; at.iOffset = block_gap_cases[fixture].slot; at.iAffinity = XUI_DOC_AFTER;
            xui_doc_range_t range = {at, at}; TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
            if (cancel) {
#ifndef XUI_DLL
                t->cancellation = &a.cancellation; a.cancel_after = point;
#endif
            } else a.allocation.remaining = point;
            result = xuiDocumentTxnReplaceRange(t, &range, "Z", 1, &caret);
            if (result == XUI_OK && continued) { range.tAnchor = range.tCaret = caret;
                result = xuiDocumentTxnReplaceRange(t, &range, "Q", 1, &caret); }
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, &change);
            a.allocation.remaining = -1;
#ifndef XUI_DLL
            a.cancel_after = -1;
#endif
            TABLE_CHECK(result == XUI_OK || result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY));
            if (result != XUI_OK) TABLE_CHECK(!change && xuiDocumentTxnCommit(t, NULL) == result);
            xuiDocumentTxnRelease(t);
#ifndef XUI_DLL
            atomic_store(&a.cancellation, 0);
#endif
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                failures++; TABLE_CHECK(xuiDocumentGetRevision(d) == revision && !notifications && !xuiDocumentIsDirty(d) &&
                    !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                table_source(current, before); table_equivalent(current, old); xuiDocumentSnapshotRelease(current);
                range.tAnchor = range.tCaret = at;
                TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
                    xuiDocumentTxnReplaceRange(t, &range, "Z", 1, &caret) == XUI_OK);
                if (continued) { range.tAnchor = range.tCaret = caret;
                    TABLE_CHECK(xuiDocumentTxnReplaceRange(t, &range, "Q", 1, &caret) == XUI_OK); }
                TABLE_CHECK(xuiDocumentTxnCommit(t, &change) == XUI_OK); xuiDocumentTxnRelease(t);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(notifications == 1 && xuiDocumentGetRevision(d) == revision + 1);
            block_gap_change(change, old, item, 0); xuiDocumentChangeSetRelease(change);
            table_source(current, expected); block_gap_equivalent(current, expected);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && !xuiDocumentCanRedo(d) &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, expected); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(notifications == 3); table_source(old, before);
            xuiDocumentUnsubscribe(d, token); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current);
            xuiDocumentRelease(d); TABLE_CHECK(!a.allocation.live); if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 10000 && failures); printf("Middle gap fixture=%u continued=%u cancel=%u fault points=%u atomic/poisoned-commit/retry/attribute-op/notification/shared-history/zero-leaks passed\n",
            fixture, continued, cancel, failures); fflush(stdout);
    }
}

/* Independent source goldens. The expected loose list is parsed separately;
 * no semantic attribute is masked, and indented code is required to survive. */
static const struct block_gap_extended_case { const char *before, *typed; unsigned slot; } block_gap_extended_cases[] = {
    {"- [x] *A* &amp;\n\n      code &amp;\n", "- [x] *A* &amp;\n\n  Z\n\n      code &amp;\n", 1},
    {"- parent\n  - *A* &amp;\n\n        code &amp;\n", "- parent\n  - *A* &amp;\n\n    Z\n\n        code &amp;\n", 1},
    {"> - parent\n>   - *A* &amp;\n>\n>         code &amp;\n", "> - parent\n>   - *A* &amp;\n>\n>     Z\n>\n>         code &amp;\n", 1},
    {"use[^n]\n\n[^n]: - parent\n      - *A* &amp;\n\n            code &amp;\n", "use[^n]\n\n[^n]: - parent\n      - *A* &amp;\n\n        Z\n\n            code &amp;\n", 1},
    {"-\t*A* &amp;\n\n\t    code &amp;\n", "-\t*A* &amp;\n\n \tZ\n\n\t    code &amp;\n", 1},
    {"009)\t*A* &amp;\n\n\t\t    code &amp;\n", "009)\t*A* &amp;\n\n    \tZ\n\n\t\t    code &amp;\n", 1},
    {"- [a][r] &amp;\n\n  [r]: <raw> 'Keep'\n\n      code &amp;\n\n[unused]: /hold 'U'\n", "- [a][r] &amp;\n\n  [r]: <raw> 'Keep'\n\n  Z\n\n      code &amp;\n\n[unused]: /hold 'U'\n", 1}
};
static uint64_t block_gap_extended_item(xui_document_snapshot s, const char* source)
{
    xui_doc_position_t position; xui_doc_node_info_t node = {0}; int quality;
    TABLE_CHECK(xuiDocumentSourceToPosition(s, (uint64_t)(strstr(source, "code") - source) + 2,
        &position, &quality) == XUI_OK);
    uint64_t id = position.iNodeId; node.iSize = sizeof(node);
    for (;;) {
        TABLE_CHECK(xuiDocumentSnapshotGetNode(s, id, &node) == XUI_OK);
        if (node.iKind == XUI_DOC_LIST_ITEM) return id;
        id = node.iParentId; TABLE_CHECK(id);
    }
}
static int block_gap_extended_ancestor(xui_document_snapshot s, uint64_t item, uint64_t id)
{
    xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
    for (;;) {
        TABLE_CHECK(xuiDocumentSnapshotGetNode(s, item, &node) == XUI_OK);
        if (item == id) return 1;
        item = node.iParentId; if (!item) return 0;
    }
}
static void block_gap_extended_change(xui_document_change_set change, xui_document_snapshot old,
    uint64_t item, unsigned expected_attributes)
{
    xui_doc_change_info_t info = {0}; unsigned attributes = 0, patches = 0;
    info.iSize = sizeof(info); TABLE_CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
    TABLE_CHECK(info.iDomain == XUI_DOC_SEMANTIC && info.iAfterRevision == info.iBeforeRevision + 1);
    for (uint64_t i = 0; i < info.iOperationCount; i++) {
        const xui_doc_operation_t* op = info.pOperations + i;
        if (op->iKind == XUI_DOC_OP_SOURCE) patches++;
        if (op->iKind == XUI_DOC_OP_ATTRIBUTES) {
            xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
            if (xuiDocumentSnapshotGetNode(old, op->iNodeId, &node) == XUI_OK && node.iKind == XUI_DOC_LIST) {
                TABLE_CHECK(block_gap_extended_ancestor(old, item, op->iNodeId)); attributes++;
            }
        }
    }
    TABLE_CHECK(patches && attributes == expected_attributes);
}
static void block_gap_extended_notify(xui_document d, xui_document_change_set change, void* user)
{
    xui_doc_change_info_t info = {0}; info.iSize = sizeof(info);
    TABLE_CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK &&
    info.iAfterRevision == xuiDocumentGetRevision(d)); (*(unsigned*)user)++;
}
static void block_gap_extended_equivalent(xui_document_snapshot current, const char* source)
{
    xui_document oracle = table_open(source, NULL); xui_document_snapshot full;
    TABLE_CHECK(xuiDocumentAcquireSnapshot(oracle, &full) == XUI_OK);
    formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, full, XUI_DOCUMENT_ROOT);
    xuiDocumentSnapshotRelease(full); xuiDocumentRelease(oracle);
    integration_equivalent(current, source);
}
static char* block_gap_extended_source(unsigned fixture, unsigned eol, unsigned bom, unsigned eof, int after)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    char* source = delete_ending(after ? block_gap_extended_cases[fixture].typed : block_gap_extended_cases[fixture].before, endings[eol], bom);
    if (eof) source[strlen(source) - strlen(endings[eol])] = 0;
    return source;
}
static void list_block_gap_extended_goldens(void)
{
    unsigned total = 0;
    for (unsigned fixture = 0; fixture < sizeof(block_gap_extended_cases)/sizeof(*block_gap_extended_cases); fixture++)
    for (unsigned eol = 0; eol < 3; eol++) for (unsigned bom = 0; bom < 2; bom++)
    for (unsigned eof = 0; eof < 2; eof++) for (unsigned continuation = 0; continuation < 3; continuation++) {
        char* before = block_gap_extended_source(fixture, eol, bom, eof, 0);
        char* typed = block_gap_extended_source(fixture, eol, bom, eof, 1);
        char* final = malloc(strlen(typed) + 2); TABLE_CHECK(final); strcpy(final, typed);
        if (continuation) { char* z = strchr(final, 'Z'); TABLE_CHECK(z); memmove(z+2, z+1, strlen(z+1)+1); z[1] = 'Q'; }
        table_allocator a = {-1, 0}; xui_document d = table_open(before, &a);
        xui_document_snapshot old, first, current, restored; xui_document_transaction t;
        xui_document_change_set change; xui_doc_position_t at = {0}, caret; uint64_t token, offset;
        unsigned notifications = 0; int quality;
        printf("Middle gap fixture=%u eol=%u bom=%u eof=%u continuation=%u\n", fixture, eol, bom, eof, continuation); fflush(stdout);
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
            xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, block_gap_extended_notify, &notifications, &token) == XUI_OK);
        uint64_t item = block_gap_extended_item(old, before); at.iSize = sizeof(at); at.iDocumentId = xuiDocumentGetIdentity(d);
        at.iKind = XUI_DOC_POSITION_GAP; at.iNodeId = item; at.iOffset = block_gap_extended_cases[fixture].slot;
        at.iRevision = xuiDocumentGetRevision(d); at.iAffinity = XUI_DOC_AFTER; xui_doc_range_t range = {at, at};
        TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnReplaceRange(t, &range, "Z", 1, &caret) == XUI_OK && !notifications);
        if (continuation == 1) { range.tAnchor = range.tCaret = caret;
            TABLE_CHECK(xuiDocumentTxnReplaceRange(t, &range, "Q", 1, &caret) == XUI_OK && !notifications); }
        TABLE_CHECK(xuiDocumentTxnCommit(t, &change) == XUI_OK && notifications == 1); xuiDocumentTxnRelease(t);
        block_gap_extended_change(change, old, item, 0); xuiDocumentChangeSetRelease(change);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &first) == XUI_OK); table_source(first, continuation == 1 ? final : typed);
        if (continuation == 2) {
            caret.iRevision = xuiDocumentGetRevision(d); range.tAnchor = range.tCaret = caret;
            TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
                xuiDocumentTxnReplaceRange(t, &range, "Q", 1, &caret) == XUI_OK && notifications == 1 &&
                xuiDocumentTxnCommit(t, &change) == XUI_OK && notifications == 2); xuiDocumentTxnRelease(t);
            block_gap_extended_change(change, first, item, 0); xuiDocumentChangeSetRelease(change);
        }
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK); table_source(current, final); block_gap_extended_equivalent(current, final);
        /* Public syntax and semantic identity of the untouched code both survive. */
        uint64_t code; xui_doc_node_info_t n = {0}; xui_doc_block_syntax_t syntax = {0}; char ac[1024], bc[1024]; uint64_t an, bn;
        n.iSize = sizeof(n); syntax.iSize = sizeof(syntax);
        TABLE_CHECK(xuiDocumentSnapshotGetNode(old, item, &n) == XUI_OK);
        uint64_t list = n.iParentId;
        TABLE_CHECK(xuiDocumentSnapshotGetNode(old, list, &n) == XUI_OK && !(n.tAttributes.iFlags & XUI_DOC_TIGHT) &&
            xuiDocumentSnapshotGetNode(current, list, &n) == XUI_OK && !(n.tAttributes.iFlags & XUI_DOC_TIGHT));
        TABLE_CHECK(xuiDocumentSnapshotGetChild(old, item, 1, &code) == XUI_OK &&
            xuiDocumentSnapshotGetBlockSyntax(old, code, &syntax) == XUI_OK && syntax.iKind == XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE &&
            xuiDocumentSnapshotGetNode(current, code, &n) == XUI_OK && n.iKind == XUI_DOC_CODE_BLOCK &&
            xuiDocumentSnapshotGetBlockSyntax(current, code, &syntax) == XUI_OK && syntax.iKind == XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE &&
            xuiDocumentSnapshotCopyText(old, code, ac, sizeof(ac), &an) == XUI_OK &&
            xuiDocumentSnapshotCopyText(current, code, bc, sizeof(bc), &bn) == XUI_OK && an == bn && !memcmp(ac, bc, (size_t)an));
        caret.iRevision = xuiDocumentGetRevision(d); TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &offset, &quality) == XUI_OK &&
            quality == XUI_DOC_MAP_EXACT && offset == (uint64_t)(strchr(final, 'Z') - final) + 1 + !!continuation);
        unsigned steps = continuation == 2 ? 2 : 1;
        for (unsigned i = 0; i < steps; i++) TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
        TABLE_CHECK(!xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
        for (unsigned i = 0; i < steps; i++) TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
        TABLE_CHECK(!xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, final); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(notifications == steps * 3); table_source(old, before); table_source(first, continuation == 1 ? final : typed);
        xuiDocumentUnsubscribe(d, token); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(first); xuiDocumentSnapshotRelease(current);
        xuiDocumentRelease(d); TABLE_CHECK(!a.live); free(before); free(typed); free(final); total++;
    }
    TABLE_CHECK(total == 252); printf("Extended block gap CST: %u independent raw/full-semantic/index/stable-code/exact-caret/attribute-operation/notification/history/zero-leak goldens passed\n", total);
}

/* Independent source goldens. The expected loose list is parsed separately;
 * no semantic attribute is masked, and indented code is required to survive. */
static const struct block_gap_code_tail_case { const char *before, *typed; unsigned slot; } block_gap_code_tail_cases[] = {
    {"-\t    code &amp;\n", "-\t    code &amp;\n\n  Z\n", 1},
    {"10)\t     code &amp;\n", "10)\t     code &amp;\n\n   \tZ\n", 1},
    {"use[^n]\n\n[^n]: -\t    code &amp;\n", "use[^n]\n\n[^n]: -\t    code &amp;\n\n      Z\n", 1},
    {"> -\t    code &amp;\n", "> -\t    code &amp;\n>\n>  \tZ\n", 1}
};
static uint64_t block_gap_code_tail_item(xui_document_snapshot s, const char* source)
{
    xui_doc_position_t position; xui_doc_node_info_t node = {0}; int quality;
    TABLE_CHECK(xuiDocumentSourceToPosition(s, (uint64_t)(strstr(source, "code") - source) + 2,
        &position, &quality) == XUI_OK);
    uint64_t id = position.iNodeId; node.iSize = sizeof(node);
    for (;;) {
        TABLE_CHECK(xuiDocumentSnapshotGetNode(s, id, &node) == XUI_OK);
        if (node.iKind == XUI_DOC_LIST_ITEM) return id;
        id = node.iParentId; TABLE_CHECK(id);
    }
}
static int block_gap_code_tail_ancestor(xui_document_snapshot s, uint64_t item, uint64_t id)
{
    xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
    for (;;) {
        TABLE_CHECK(xuiDocumentSnapshotGetNode(s, item, &node) == XUI_OK);
        if (item == id) return 1;
        item = node.iParentId; if (!item) return 0;
    }
}
static void block_gap_code_tail_change(xui_document_change_set change, xui_document_snapshot old,
    uint64_t item, unsigned expected_attributes)
{
    xui_doc_change_info_t info = {0}; unsigned attributes = 0, patches = 0;
    info.iSize = sizeof(info); TABLE_CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
    TABLE_CHECK(info.iDomain == XUI_DOC_SEMANTIC && info.iAfterRevision == info.iBeforeRevision + 1);
    for (uint64_t i = 0; i < info.iOperationCount; i++) {
        const xui_doc_operation_t* op = info.pOperations + i;
        if (op->iKind == XUI_DOC_OP_SOURCE) patches++;
        if (op->iKind == XUI_DOC_OP_ATTRIBUTES) {
            xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
            if (xuiDocumentSnapshotGetNode(old, op->iNodeId, &node) == XUI_OK && node.iKind == XUI_DOC_LIST) {
                TABLE_CHECK(block_gap_code_tail_ancestor(old, item, op->iNodeId)); attributes++;
            }
        }
    }
    TABLE_CHECK(patches && attributes == expected_attributes);
}
static void block_gap_code_tail_notify(xui_document d, xui_document_change_set change, void* user)
{
    xui_doc_change_info_t info = {0}; info.iSize = sizeof(info);
    TABLE_CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK &&
    info.iAfterRevision == xuiDocumentGetRevision(d)); (*(unsigned*)user)++;
}
static void block_gap_code_tail_equivalent(xui_document_snapshot current, const char* source)
{
    xui_document oracle = table_open(source, NULL); xui_document_snapshot full;
    TABLE_CHECK(xuiDocumentAcquireSnapshot(oracle, &full) == XUI_OK);
    formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, full, XUI_DOCUMENT_ROOT);
    xuiDocumentSnapshotRelease(full); xuiDocumentRelease(oracle);
    integration_equivalent(current, source);
}
static char* block_gap_code_tail_source(unsigned fixture, unsigned eol, unsigned bom, unsigned eof, int after)
{
    const char* endings[] = {"\n", "\r\n", "\r"};
    char* source = delete_ending(after ? block_gap_code_tail_cases[fixture].typed : block_gap_code_tail_cases[fixture].before, endings[eol], bom);
    if (eof) source[strlen(source) - strlen(endings[eol])] = 0;
    if (after && eof) {
        /* The unchanged marker/code line has no ending to inherit. The
         * inserted boundary uses the declared LF default; earlier source
         * lines (e.g. the footnote use) retain their original endings. */
        char* prefix = delete_ending(block_gap_code_tail_cases[fixture].before, endings[eol], bom);
        prefix[strlen(prefix) - strlen(endings[eol])] = 0;
        const char* inserted = block_gap_code_tail_cases[fixture].typed + strlen(block_gap_code_tail_cases[fixture].before) - 1;
        char* expected = malloc(strlen(prefix) + strlen(inserted) + 1); TABLE_CHECK(expected);
        strcpy(expected, prefix); strcat(expected, inserted); expected[strlen(expected) - 1] = 0;
        free(prefix); free(source); source = expected;
    }
    return source;
}
static void list_block_gap_code_tail_goldens(void)
{
    unsigned total = 0;
    for (unsigned fixture = 0; fixture < sizeof(block_gap_code_tail_cases)/sizeof(*block_gap_code_tail_cases); fixture++)
    for (unsigned eol = 0; eol < 3; eol++) for (unsigned bom = 0; bom < 2; bom++)
    for (unsigned eof = 0; eof < 2; eof++) for (unsigned continuation = 0; continuation < 3; continuation++) {
        char* before = block_gap_code_tail_source(fixture, eol, bom, eof, 0);
        char* typed = block_gap_code_tail_source(fixture, eol, bom, eof, 1);
        char* final = malloc(strlen(typed) + 2); TABLE_CHECK(final); strcpy(final, typed);
        if (continuation) { char* z = strchr(final, 'Z'); TABLE_CHECK(z); memmove(z+2, z+1, strlen(z+1)+1); z[1] = 'Q'; }
        table_allocator a = {-1, 0}; xui_document d = table_open(before, &a);
        xui_document_snapshot old, first, current, restored; xui_document_transaction t;
        xui_document_change_set change; xui_doc_position_t at = {0}, caret; uint64_t token, offset;
        unsigned notifications = 0; int quality;
        printf("Middle gap fixture=%u eol=%u bom=%u eof=%u continuation=%u\n", fixture, eol, bom, eof, continuation); fflush(stdout);
        TABLE_CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
            xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, block_gap_code_tail_notify, &notifications, &token) == XUI_OK);
        uint64_t item = block_gap_code_tail_item(old, before); at.iSize = sizeof(at); at.iDocumentId = xuiDocumentGetIdentity(d);
        at.iKind = XUI_DOC_POSITION_GAP; at.iNodeId = item; at.iOffset = block_gap_code_tail_cases[fixture].slot;
        at.iRevision = xuiDocumentGetRevision(d); at.iAffinity = XUI_DOC_AFTER; xui_doc_range_t range = {at, at};
        TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnReplaceRange(t, &range, "Z", 1, &caret) == XUI_OK && !notifications);
        if (continuation == 1) { range.tAnchor = range.tCaret = caret;
            TABLE_CHECK(xuiDocumentTxnReplaceRange(t, &range, "Q", 1, &caret) == XUI_OK && !notifications); }
        TABLE_CHECK(xuiDocumentTxnCommit(t, &change) == XUI_OK && notifications == 1); xuiDocumentTxnRelease(t);
        block_gap_code_tail_change(change, old, item, 0); xuiDocumentChangeSetRelease(change);
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &first) == XUI_OK); table_source(first, continuation == 1 ? final : typed);
        if (continuation == 2) {
            caret.iRevision = xuiDocumentGetRevision(d); range.tAnchor = range.tCaret = caret;
            TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
                xuiDocumentTxnReplaceRange(t, &range, "Q", 1, &caret) == XUI_OK && notifications == 1 &&
                xuiDocumentTxnCommit(t, &change) == XUI_OK && notifications == 2); xuiDocumentTxnRelease(t);
            block_gap_code_tail_change(change, first, item, 0); xuiDocumentChangeSetRelease(change);
        }
        TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK); table_source(current, final); block_gap_code_tail_equivalent(current, final);
        /* Public syntax and semantic identity of the untouched code both survive. */
        uint64_t code; xui_doc_node_info_t n = {0}; xui_doc_block_syntax_t syntax = {0}; char ac[1024], bc[1024]; uint64_t an, bn;
        n.iSize = sizeof(n); syntax.iSize = sizeof(syntax);
        TABLE_CHECK(xuiDocumentSnapshotGetNode(old, item, &n) == XUI_OK);
        uint64_t list = n.iParentId;
        TABLE_CHECK(xuiDocumentSnapshotGetNode(old, list, &n) == XUI_OK && (n.tAttributes.iFlags & XUI_DOC_TIGHT) &&
            xuiDocumentSnapshotGetNode(current, list, &n) == XUI_OK && (n.tAttributes.iFlags & XUI_DOC_TIGHT));
        TABLE_CHECK(xuiDocumentSnapshotGetChild(old, item, 0, &code) == XUI_OK &&
            xuiDocumentSnapshotGetBlockSyntax(old, code, &syntax) == XUI_OK && syntax.iKind == XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE &&
            xuiDocumentSnapshotGetNode(current, code, &n) == XUI_OK && n.iKind == XUI_DOC_CODE_BLOCK &&
            xuiDocumentSnapshotGetBlockSyntax(current, code, &syntax) == XUI_OK && syntax.iKind == XUI_DOC_BLOCK_SYNTAX_INDENTED_CODE &&
            xuiDocumentSnapshotCopyText(old, code, ac, sizeof(ac), &an) == XUI_OK &&
            xuiDocumentSnapshotCopyText(current, code, bc, sizeof(bc), &bn) == XUI_OK && an == bn && !memcmp(ac, bc, (size_t)an));
        caret.iRevision = xuiDocumentGetRevision(d); TABLE_CHECK(xuiDocumentPositionToSource(current, &caret, &offset, &quality) == XUI_OK &&
            quality == XUI_DOC_MAP_EXACT && offset == (uint64_t)(strchr(final, 'Z') - final) + 1 + !!continuation);
        unsigned steps = continuation == 2 ? 2 : 1;
        for (unsigned i = 0; i < steps; i++) TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
        TABLE_CHECK(!xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
        for (unsigned i = 0; i < steps; i++) TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
        TABLE_CHECK(!xuiDocumentCanRedo(d) && xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
        table_source(restored, final); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
        TABLE_CHECK(notifications == steps * 3); table_source(old, before); table_source(first, continuation == 1 ? final : typed);
        xuiDocumentUnsubscribe(d, token); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(first); xuiDocumentSnapshotRelease(current);
        xuiDocumentRelease(d); TABLE_CHECK(!a.live); free(before); free(typed); free(final); total++;
    }
    TABLE_CHECK(total == 144); printf("Code tail gap CST: %u independent raw/full-semantic/index/stable-code/exact-caret/attribute-operation/notification/history/zero-leak goldens passed\n", total);
}

static void list_block_gap_code_tail_faults(void)
{
    unsigned modes = 1;
#ifndef XUI_DLL
    modes = 2;
#endif
    for (unsigned fixture = 0; fixture < 4; fixture++)
    for (unsigned continued = 0; continued < 2; continued++) for (unsigned cancel = 0; cancel < modes; cancel++) {
        unsigned failures = 0; long point;
        const char* before = block_gap_code_tail_cases[fixture].before;
        char expected[4096]; strcpy(expected, block_gap_code_tail_cases[fixture].typed);
        if (continued) { char* z = strchr(expected, 'Z'); TABLE_CHECK(z); memmove(z+2, z+1, strlen(z+1)+1); z[1] = 'Q'; }
        for (point = 0; point < 10000; point++) {
            list_edge_allocator a = {0}; a.allocation.remaining = -1;
#ifndef XUI_DLL
            a.cancel_after = -1; atomic_init(&a.cancellation, 0);
#endif
            xui_doc_desc_t desc = {0}; desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.iMarkdownDialect = XUI_MD_EXTENDED; desc.onAlloc = list_edge_alloc; desc.onFree = list_edge_free; desc.pAllocatorUser = &a;
            xui_document d; xui_document_snapshot old, current, restored; xui_document_transaction t;
            xui_document_change_set change = NULL; xui_doc_position_t at = {0}, caret;
            unsigned notifications = 0; uint64_t token, revision; int result;
            TABLE_CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, before, strlen(before)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
                xuiDocumentMarkSaved(d, old) == XUI_OK && xuiDocumentSubscribe(d, block_gap_code_tail_notify, &notifications, &token) == XUI_OK);
            uint64_t item = block_gap_code_tail_item(old, before); revision = xuiDocumentGetRevision(d);
            at.iSize = sizeof(at); at.iDocumentId = xuiDocumentGetIdentity(d); at.iRevision = revision;
            at.iKind = XUI_DOC_POSITION_GAP; at.iNodeId = item; at.iOffset = block_gap_code_tail_cases[fixture].slot; at.iAffinity = XUI_DOC_AFTER;
            xui_doc_range_t range = {at, at}; TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
            if (cancel) {
#ifndef XUI_DLL
                t->cancellation = &a.cancellation; a.cancel_after = point;
#endif
            } else a.allocation.remaining = point;
            result = xuiDocumentTxnReplaceRange(t, &range, "Z", 1, &caret);
            if (result == XUI_OK && continued) { range.tAnchor = range.tCaret = caret;
                result = xuiDocumentTxnReplaceRange(t, &range, "Q", 1, &caret); }
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, &change);
            a.allocation.remaining = -1;
#ifndef XUI_DLL
            a.cancel_after = -1;
#endif
            TABLE_CHECK(result == XUI_OK || result == (cancel ? XUI_DOC_ERROR_CANCELLED : XUI_ERROR_OUT_OF_MEMORY));
            if (result != XUI_OK) TABLE_CHECK(!change && xuiDocumentTxnCommit(t, NULL) == result);
            xuiDocumentTxnRelease(t);
#ifndef XUI_DLL
            atomic_store(&a.cancellation, 0);
#endif
            TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            if (result != XUI_OK) {
                failures++; TABLE_CHECK(xuiDocumentGetRevision(d) == revision && !notifications && !xuiDocumentIsDirty(d) &&
                    !xuiDocumentCanUndo(d) && !xuiDocumentCanRedo(d));
                table_source(current, before); table_equivalent(current, old); xuiDocumentSnapshotRelease(current);
                range.tAnchor = range.tCaret = at;
                TABLE_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
                    xuiDocumentTxnReplaceRange(t, &range, "Z", 1, &caret) == XUI_OK);
                if (continued) { range.tAnchor = range.tCaret = caret;
                    TABLE_CHECK(xuiDocumentTxnReplaceRange(t, &range, "Q", 1, &caret) == XUI_OK); }
                TABLE_CHECK(xuiDocumentTxnCommit(t, &change) == XUI_OK); xuiDocumentTxnRelease(t);
                TABLE_CHECK(xuiDocumentAcquireSnapshot(d, &current) == XUI_OK);
            }
            TABLE_CHECK(notifications == 1 && xuiDocumentGetRevision(d) == revision + 1);
            block_gap_code_tail_change(change, old, item, 0); xuiDocumentChangeSetRelease(change);
            table_source(current, expected); block_gap_code_tail_equivalent(current, expected);
            TABLE_CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && !xuiDocumentIsDirty(d) &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, before); table_equivalent(restored, old); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && !xuiDocumentCanRedo(d) &&
                xuiDocumentAcquireSnapshot(d, &restored) == XUI_OK);
            table_source(restored, expected); table_equivalent(restored, current); xuiDocumentSnapshotRelease(restored);
            TABLE_CHECK(notifications == 3); table_source(old, before);
            xuiDocumentUnsubscribe(d, token); xuiDocumentSnapshotRelease(old); xuiDocumentSnapshotRelease(current);
            xuiDocumentRelease(d); TABLE_CHECK(!a.allocation.live); if (result == XUI_OK) break;
        }
        TABLE_CHECK(point < 10000 && failures); printf("Code tail gap fixture=%u continued=%u cancel=%u fault points=%u atomic/poisoned-commit/retry/attribute-op/notification/shared-history/zero-leaks passed\n",
            fixture, continued, cancel, failures); fflush(stdout);
    }
}
