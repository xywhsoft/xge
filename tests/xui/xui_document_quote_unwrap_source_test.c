#define XUI_TAB_PREFIX_NO_MAIN
#include "xui_document_tab_prefix_test.c"

typedef struct unwrap_source_case { const char* source; const char* expected; } unwrap_source_case;
#define UNWRAP_EIGHT(prefix) prefix "continuation &amp; raw\n" prefix "continuation &amp; raw\n" \
    prefix "continuation &amp; raw\n" prefix "continuation &amp; raw\n" prefix "continuation &amp; raw\n" \
    prefix "continuation &amp; raw\n" prefix "continuation &amp; raw\n" prefix "continuation &amp; raw\n"
#define UNWRAP_EIGHTY(prefix) UNWRAP_EIGHT(prefix) UNWRAP_EIGHT(prefix) UNWRAP_EIGHT(prefix) UNWRAP_EIGHT(prefix) \
    UNWRAP_EIGHT(prefix) UNWRAP_EIGHT(prefix) UNWRAP_EIGHT(prefix) UNWRAP_EIGHT(prefix) UNWRAP_EIGHT(prefix) UNWRAP_EIGHT(prefix)
static const unwrap_source_case unwrap_source_cases[] = {
    { "pre &amp; raw\n\n> take &amp; raw\n>\n> ~~~~ lang opaque &amp; info\n> literal <&> \\raw\n> ~~~~\n\npost &amp; raw\n",
      "pre &amp; raw\n\ntake &amp; raw\n\n~~~~ lang opaque &amp; info\nliteral <&> \\raw\n~~~~\n\npost &amp; raw\n" },
    { "pre &amp; raw\n\n> take [a]\n>\n> [a]: /u 'raw &amp;'\n>\n> | a | b |\n> | :-- | --: |\n> | ~~s~~ | `c|d` |\n\npost &amp; raw\n",
      "pre &amp; raw\n\ntake [a]\n\n[a]: /u 'raw &amp;'\n\n| a | b |\n| :-- | --: |\n| ~~s~~ | `c|d` |\n\npost &amp; raw\n" },
    { "> > take &amp; raw\n> >\n> > [a]: /u 'raw &amp;'\n> >\n> > also [a]\n",
      "> take &amp; raw\n> \n> [a]: /u 'raw &amp;'\n> \n> also [a]\n" },
    { "- > take &amp; raw\n  >\n  > [a]: /u 'raw &amp;'\n  >\n  > also [a]\n- last &amp; raw\n",
      "- take &amp; raw\n  \n  [a]: /u 'raw &amp;'\n  \n  also [a]\n- last &amp; raw\n" },
    { "  > take &amp; raw\n >\n   > ~~~~~ lang opaque\n > literal &amp; \\raw\n   > ~~~~~\n",
      "take &amp; raw\n\n~~~~~ lang opaque\nliteral &amp; \\raw\n~~~~~\n" },
    { "> take &amp; raw\nlazy &amp; continuation\n>\n> also &amp; raw\n",
      "take &amp; raw\nlazy &amp; continuation\n\nalso &amp; raw\n" },
    { "> take\n>\n>\t\tcode\t<&> raw\n", "take\n\n      code\t<&> raw\n" },
    { "> take\n>\n> ~~~~ lang opaque\n>\tcode\t<&> raw\n> ~~~~\n",
      "take\n\n~~~~ lang opaque\n  code\t<&> raw\n~~~~\n" },
    { "pre [a]\n\n[a]: /first 'first wins'\n\n> take [a]\n>\n> [a]: /duplicate 'raw &amp;'\n>\n> [unused]: /opaque 'one\n> two'\n>\n> also [a]\n",
      "pre [a]\n\n[a]: /first 'first wins'\n\ntake [a]\n\n[a]: /duplicate 'raw &amp;'\n\n[unused]: /opaque 'one\ntwo'\n\nalso [a]\n" },
    { "> take [^n]\n>\n> [^n]: foot &amp; body\n>     continuation &amp;  \n>\n> [^unused]: raw unused\n>     continuation\\\n>     ending\n>\n> also [^n]\n",
      "take [^n]\n\n[^n]: foot &amp; body\n    continuation &amp;  \n\n[^unused]: raw unused\n    continuation\\\n    ending\n\nalso [^n]\n" },
    { "1. > take &amp; raw\n   >\n   > ~~~~~ lang opaque\n   > code\t<&> raw\n   > ~~~~~\n2. last\n",
      "1. take &amp; raw\n   \n   ~~~~~ lang opaque\n   code\t<&> raw\n   ~~~~~\n2. last\n" },
    { "100. before\n\n\t > take &amp; raw\n\t >\n\t > also &amp; raw\n101. last\n",
      "100. before\n\n\t take &amp; raw\n\t \n\t also &amp; raw\n101. last\n" },
    { "  > take [^n]\n  >\n  > [^n]:\n  > \t\tcode\t<&> raw\n  >\n  > also [^n]\n",
      "take [^n]\n\n[^n]:\n\t\tcode\t<&> raw\n\nalso [^n]\n" },
    { ">take &amp; raw  \n>hard break\n>\n><div>raw &amp;</div>\n",
      "take &amp; raw  \nhard break\n\n<div>raw &amp;</div>\n" },
    { "> take &amp; raw\n>\n> also &amp; raw\n>\n>\n", "take &amp; raw\n\nalso &amp; raw\n\n\n" },
    { "> take &amp; raw", "take &amp; raw" },
    { "> take &amp; raw\n>\n> > ~~~~ lang opaque\n> >\tcode\t<&> raw\n> > ~~~~\n",
      "take &amp; raw\n\n> ~~~~ lang opaque\n> code\t<&> raw\n> ~~~~\n" },
    { "> take &amp; raw\n>\n> -\titem &amp; raw\n> -\tlast\n",
      "take &amp; raw\n\n- item &amp; raw\n- last\n" },
    { "> take 中文 🧡 &amp; raw\n>\n> ~~~~ lang opaque\n> >\tcontent &amp; raw\n> ~~~~\n",
      "take 中文 🧡 &amp; raw\n\n~~~~ lang opaque\n>\tcontent &amp; raw\n~~~~\n" },
    { "- > take &amp; raw\n  > ~~~~ lang opaque\n  > code\t<&>\n  > ~~~~\n- last\n",
      "- take &amp; raw\n  ~~~~ lang opaque\n  code\t<&>\n  ~~~~\n- last\n" },
    { "> take\n>\n> [^unused]: raw &amp;\n" UNWRAP_EIGHTY(">     ") ">\n> also\n",
      "take\n\n[^unused]: raw &amp;\n" UNWRAP_EIGHTY("    ") "\nalso\n" }
};
#undef UNWRAP_EIGHTY
#undef UNWRAP_EIGHT

#ifndef XUI_DLL
static int unwrap_has_blob(doc_sequence* source, doc_blob* blob)
{
    return source && (source->blob == blob || unwrap_has_blob(source->left, blob) || unwrap_has_blob(source->right, blob));
}
static uint64_t unwrap_shared_bytes(doc_sequence* before, doc_sequence* after)
{
    if (!after) return 0;
    return (unwrap_has_blob(before, after->blob) ? after->length : 0) +
        unwrap_shared_bytes(before, after->left) + unwrap_shared_bytes(before, after->right);
}
#endif

static xui_doc_node_info_t unwrap_quote_node(xui_document_snapshot snapshot, xui_doc_position_t position)
{
    xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
    do {
        CHECK(xuiDocumentSnapshotGetNode(snapshot, position.iNodeId, &node) == XUI_OK);
        position.iNodeId = node.iParentId;
    } while (node.iKind != XUI_DOC_QUOTE && position.iNodeId);
    CHECK(node.iKind == XUI_DOC_QUOTE); return node;
}
static xui_doc_position_t unwrap_position(xui_document_snapshot before, xui_doc_position_t text, unsigned gap)
{
    if (gap) {
        xui_doc_node_info_t quote = unwrap_quote_node(before, text);
        text.iKind = XUI_DOC_POSITION_GAP; text.iNodeId = quote.iId;
        text.iOffset = gap == 1 ? 0 : quote.iChildCount;
    }
    return text;
}
static uint64_t unwrap_patch_proof(const char* source, const char* output,
    xui_document_snapshot before, xui_document_change_set change)
{
    xui_doc_change_info_t info = {0}; uint64_t i, at, last = UINT64_MAX, removed = 0, added = 0, count = 0;
    info.iSize = sizeof(info); CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
    for (i = 0; i < info.iOperationCount; i++) {
        const xui_doc_operation_t* op = &info.pOperations[i];
        if (op->iKind != XUI_DOC_OP_SOURCE) continue;
        CHECK(op->iOffset <= last && op->iOldLength <= 32 && op->iNewLength <= 16);
        for (at = op->iOffset; at < op->iOffset + op->iOldLength; at++)
            CHECK(source[at] == '>' || source[at] == ' ' || source[at] == '\t');
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

static void unwrap_source_matrix(void)
{
    unsigned ci, ending, gap, total = 0;
    for (ci = 0; ci < sizeof(unwrap_source_cases) / sizeof(unwrap_source_cases[0]); ci++)
    for (ending = 0; ending < 3; ending++)
    for (gap = 0; gap < 3; gap++) {
        char source[8192], expected[8192], actual[8192]; xui_document d, reload;
        xui_document_snapshot before, after, full; xui_document_transaction t; xui_doc_position_t caret;
        xui_doc_range_t at; xui_doc_position_t position; xui_document_change_set change; int result;
        prefix_variant(unwrap_source_cases[ci].source, ending, source, sizeof(source));
        prefix_variant(unwrap_source_cases[ci].expected, ending, expected, sizeof(expected));
        d = open_md(source); at = find_text(d, "take");
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        position = unwrap_position(before, at.tAnchor, gap);
        result = xuiDocumentTxnUnwrapQuote(t, &position, &caret);
        if (result) fprintf(stderr, "unwrap source case %u/%u: %d\n", ci, ending, result);
        CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, sizeof(actual));
        if (strcmp(actual, expected)) fprintf(stderr, "unwrap source case %u/%u respelled:\n%s\n", ci, ending, actual);
        CHECK(!strcmp(actual, expected));
        {
            uint64_t removed = unwrap_patch_proof(source, actual, before, change);
#ifndef XUI_DLL
            CHECK(unwrap_shared_bytes(before->state->source, after->state->source) == strlen(source) - removed &&
                before->state->reference_values == after->state->reference_values);
#else
            (void)removed;
#endif
        }
        xuiDocumentChangeSetRelease(change);
        reload = open_md(actual); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(before, after); reference_cache_snapshot_equal(after, full);
#endif
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); total++;
    }
    printf("Quote unwrap source: %u literal TEXT/GAP LF/CRLF/BOM cases, hidden definitions, exact patches and position maps, full reload and history\n", total);
}

static void unwrap_depth_and_rich(void)
{
    const unsigned depths[] = {1, 8, 32, 64, 124}; unsigned di, ci;
    for (di = 0; di < sizeof(depths) / sizeof(depths[0]); di++) {
        const char* lines[] = {"take &amp; raw", "", "also &amp; raw"};
        char source[8192], actual[8192]; size_t used = 0; unsigned line, layer;
        xui_document d; xui_document_transaction t; xui_doc_position_t caret;
        xui_doc_range_t at; uint64_t revision;
        for (line = 0; line < sizeof(lines) / sizeof(lines[0]); line++) {
            for (layer = 0; layer < depths[di]; layer++) used += (size_t)sprintf(source + used, "> ");
            used += (size_t)sprintf(source + used, "%s\n", lines[line]);
        }
        CHECK(used < sizeof(source)); d = open_md(source); at = find_text(d, "take"); revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        caret = at.tAnchor;
        for (layer = 0; layer < depths[di]; layer++) {
            CHECK(xuiDocumentTxnUnwrapQuote(t, &caret, &caret) == XUI_OK && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        }
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentGetRevision(d) == revision + 1);
        xuiDocumentTxnRelease(t); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, "take &amp; raw\n\nalso &amp; raw\n"));
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d)); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        xuiDocumentRelease(d);
    }
    for (ci = 0; ci < 8; ci++) {
        xui_document md = open_md(unwrap_source_cases[ci].source), rich, expected_md, expected_rich;
        xui_document_snapshot source, before, after, full; xui_doc_rich_conversion_report_t report = {0};
        xui_document_transaction t; xui_doc_range_t at; xui_doc_position_t caret;
        report.iSize = sizeof(report);
        CHECK(xuiDocumentAcquireSnapshot(md, &source) == XUI_OK && xuiDocumentSnapshotAnalyzeRichConversion(source, &report) == XUI_OK &&
            xuiDocumentSnapshotConvertToRich(source, report.iReasons, &rich) == XUI_OK);
        xuiDocumentSnapshotRelease(source); xuiDocumentRelease(md);
        expected_md = open_md(unwrap_source_cases[ci].expected);
        CHECK(xuiDocumentAcquireSnapshot(expected_md, &source) == XUI_OK && xuiDocumentSnapshotAnalyzeRichConversion(source, &report) == XUI_OK &&
            xuiDocumentSnapshotConvertToRich(source, report.iReasons, &expected_rich) == XUI_OK);
        xuiDocumentSnapshotRelease(source); xuiDocumentRelease(expected_md);
        at = find_text(rich, "take");
        CHECK(xuiDocumentClearHistory(rich) == XUI_OK && xuiDocumentAcquireSnapshot(rich, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(rich, NULL, &t) == XUI_OK && xuiDocumentTxnUnwrapQuote(t, &at.tAnchor, &caret) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(rich, &after) == XUI_OK &&
            xuiDocumentAcquireSnapshot(expected_rich, &full) == XUI_OK);
        xuiDocumentTxnRelease(t); same_tree(after, 1, full, 1);
        xuiDocumentSnapshotRelease(after); xuiDocumentSnapshotRelease(full);
        CHECK(xuiDocumentUndo(rich, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(rich, &after) == XUI_OK);
        same_tree(before, 1, after, 1); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after);
        xuiDocumentRelease(rich); xuiDocumentRelease(expected_rich);
    }
    puts("Quote unwrap: 1/8/32/64/124-layer single transactions and 8 Rich equivalents, unpublished source and shared history");
}
static void unwrap_large_source(void)
{
    const unsigned sizes[] = {4096, 131072, 1048576}; unsigned si;
    for (si = 0; si < sizeof(sizes) / sizeof(sizes[0]); si++) {
        const char* head = "pre &amp; raw\n\n> take\n>\n> [^unused]: ";
        const char* tail = "\n>\n> also\n\npost &amp; raw\n";
        size_t capacity = sizes[si] + 256, length; char* source = malloc(capacity); char* actual = malloc(capacity);
        xui_document d, reload; xui_document_snapshot before, after, full; xui_document_transaction t;
        xui_doc_range_t at; xui_doc_position_t caret; xui_doc_memory_stats_t stats = {0};
        CHECK(source && actual); strcpy(source, head); length = strlen(head); memset(source + length, 'q', sizes[si]);
        length += sizes[si]; strcpy(source + length, tail); length += strlen(tail);
        d = open_md(source); at = find_text(d, "take");
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnUnwrapQuote(t, &at.tAnchor, &caret) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, capacity);
        CHECK(strstr(actual, "[^unused]: ") && strlen(actual) == length - 8 && strstr(actual, "pre &amp; raw") && strstr(actual, "post &amp; raw"));
        stats.iSize = sizeof(stats); CHECK(xuiDocumentGetMemoryStats(d, &stats) == XUI_OK && stats.iHistoryBytes < 32768);
#ifndef XUI_DLL
        CHECK(before->state->reference_values == after->state->reference_values);
        reference_cache_snapshot_equal(before, after);
#endif
        reload = open_md(actual); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK); same_tree(after, 1, full, 1); same_syntax(after, full);
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        printf("Quote unwrap %u-byte hidden body: HistoryBytes=%llu\n", sizes[si], (unsigned long long)stats.iHistoryBytes);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, capacity); CHECK(!strcmp(actual, source));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); free(source); free(actual);
    }
}

static void unwrap_failure_sweeps(void)
{
    const unsigned cases[] = {0, 1, 3, 6, 9, 12, 16, 17, 20}; unsigned sample; long total = 0;
    for (sample = 0; sample < sizeof(cases) / sizeof(cases[0]); sample++) {
        unsigned ci = cases[sample]; long point; int success = 0;
        for (point = 0; point < 10000 && !success; point++) {
            failing_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_transaction t = NULL; xui_document_snapshot before, after;
            xui_doc_range_t at; xui_doc_position_t caret; char actual[8192]; uint64_t revision; unsigned live; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = fail_alloc; desc.onFree = fail_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, unwrap_source_cases[ci].source, strlen(unwrap_source_cases[ci].source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            at = find_text(d, "take"); revision = xuiDocumentGetRevision(d); live = a.live; a.remaining = point;
            result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (!result) result = xuiDocumentTxnUnwrapQuote(t, &at.tAnchor, &caret);
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            xuiDocumentTxnRelease(t); a.remaining = -1;
            if (result) CHECK(result == XUI_ERROR_OUT_OF_MEMORY && a.live == live &&
                xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) {
                success = 1; copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unwrap_source_cases[ci].expected));
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            } else { same_tree(before, 1, after, 1); same_syntax(before, after); }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unwrap_source_cases[ci].source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success); total += point - 1;
        printf("Quote unwrap OOM %u: %ld failure points, atomic tree/source/history, zero leaks\n", ci, point - 1);
    }
    printf("Quote unwrap OOM total: %ld\n", total);
}
#ifndef XUI_DLL
static void unwrap_cancellation_sweeps(void)
{
    const unsigned cases[] = {1, 3, 6, 9, 12, 16, 17, 20}; unsigned sample; long total = 0;
    for (sample = 0; sample < sizeof(cases) / sizeof(cases[0]); sample++) {
        unsigned ci = cases[sample]; long point; int success = 0;
        for (point = 0; point < 10000 && !success; point++) {
            prefix_cancel_allocator a = {{-1, 0}, -1, ATOMIC_VAR_INIT(0)}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_transaction t; xui_document_snapshot before, after; xui_doc_range_t at;
            xui_doc_position_t caret; char actual[8192]; uint64_t revision; unsigned live; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = prefix_cancel_alloc; desc.onFree = prefix_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, unwrap_source_cases[ci].source, strlen(unwrap_source_cases[ci].source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            at = find_text(d, "take"); revision = xuiDocumentGetRevision(d); live = a.allocation.live;
            CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); t->cancellation = &a.cancellation; a.remaining = point;
            result = xuiDocumentTxnUnwrapQuote(t, &at.tAnchor, &caret);
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            a.remaining = -1; xuiDocumentTxnRelease(t); atomic_store(&a.cancellation, 0);
            if (result) CHECK(result == XUI_DOC_ERROR_CANCELLED && a.allocation.live == live &&
                xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) {
                success = 1; copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unwrap_source_cases[ci].expected));
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            } else { same_tree(before, 1, after, 1); same_syntax(before, after); reference_cache_snapshot_equal(before, after); }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unwrap_source_cases[ci].source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.allocation.live);
        }
        CHECK(success); total += point - 1; printf("Quote unwrap cancellation %u: %ld checkpoints, atomic failure and zero leaks\n", ci, point - 1);
    }
    printf("Quote unwrap cancellation total: %ld\n", total);
}
#endif
#ifdef XUI_QUOTE_TEST_EDITOR
static void unwrap_editor(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; unsigned ci, gap, mode, total = 0;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    for (ci = 0; ci < sizeof(unwrap_source_cases) / sizeof(unwrap_source_cases[0]); ci++)
    for (gap = 0; gap < 3; gap++)
    for (mode = 0; mode < 3; mode++) {
        xui_document d = open_md(unwrap_source_cases[ci].source); xui_document_snapshot before;
        xui_doc_range_t range = find_text(d, "take"), selected; xui_widget editor;
        xui_doc_editor_desc_t desc = {0}; xui_doc_command_state_t state = {0}; char actual[8192];
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        range.tAnchor = unwrap_position(before, range.tAnchor, gap); range.tCaret = range.tAnchor; xuiDocumentSnapshotRelease(before);
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d;
        desc.iMode = mode == 2 ? XUI_DOC_SOURCE_TEXT : mode == 1 ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_VISUAL;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK && xuiWidgetSetRect(editor, (xui_rect_t){0,0,640,480}) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        if (mode) {
            state.iSize = sizeof(state);
            CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && !state.bEnabled &&
                state.iDisabledReason == XUI_ERROR_UNSUPPORTED && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_ERROR_UNSUPPORTED &&
                !xuiDocumentCanUndo(d));
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unwrap_source_cases[ci].source));
            CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        }
        state.iSize = sizeof(state); CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && state.bEnabled && state.bActive);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK && xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &selected) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unwrap_source_cases[ci].expected));
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
            xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unwrap_source_cases[ci].source));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, unwrap_source_cases[ci].expected));
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); total++;
    }
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Quote unwrap Editor: %u actual DLL VISUAL Query/Execute/Caret cases, 126 LIVE/SOURCE atomic refusals and three-mode history\n", total);
}
#endif
#ifndef XUI_QUOTE_UNWRAP_SOURCE_NO_MAIN
int main(void)
{
    unwrap_source_matrix(); unwrap_depth_and_rich(); unwrap_large_source(); unwrap_failure_sweeps();
#ifndef XUI_DLL
    unwrap_cancellation_sweeps();
#endif
#ifdef XUI_QUOTE_TEST_EDITOR
    unwrap_editor();
#endif
    return 0;
}
#endif
