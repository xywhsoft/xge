#define XUI_QUOTE_UNWRAP_SOURCE_NO_MAIN
#include "xui_document_quote_unwrap_source_test.c"

static const unwrap_source_case boundary_cases[] = {
    { "before &amp; raw\n> take &amp; raw\n>\n> ~~~~ lang opaque &amp; info\n> code\\raw\n> ~~~~\nafter &amp; raw\n",
      "before &amp; raw\n\ntake &amp; raw\n\n~~~~ lang opaque &amp; info\ncode\\raw\n~~~~\nafter &amp; raw\n" },
    { "before &amp; raw\n> take &amp; raw\n>\n> <div>raw &amp;</div>\nafter &amp; raw\n",
      "before &amp; raw\n\ntake &amp; raw\n\n<div>raw &amp;</div>\n\nafter &amp; raw\n" },
    { "> before &amp; raw\n> > take &amp; raw\n>\n> after &amp; raw\n",
      "> before &amp; raw\n> \n> take &amp; raw\n>\n> after &amp; raw\n" },
    { "- before &amp; raw\n  > take &amp; raw\n- last &amp; raw\n",
      "- before &amp; raw\n  \n  take &amp; raw\n- last &amp; raw\n" },
    { "- before &amp; raw\n  > take &amp; raw\n  >\n  > ~~~~ lang opaque\n  > code\\raw\n  > ~~~~\n- last &amp; raw\n",
      "- before &amp; raw\n  \n  take &amp; raw\n  \n  ~~~~ lang opaque\n  code\\raw\n  ~~~~\n- last &amp; raw\n" },
    { "- > take &amp; raw\n  >\n  > <div>raw &amp;</div>\n- last &amp; raw\n",
      "- take &amp; raw\n  \n  <div>raw &amp;</div>\n- last &amp; raw\n" },
    { "before &amp; raw\r\n> take &amp; raw\n>\r\n> <div>raw &amp;</div>\r\nafter &amp; raw\n",
      "before &amp; raw\r\n\r\ntake &amp; raw\n\r\n<div>raw &amp;</div>\r\n\r\nafter &amp; raw\n" },
    { "- > take &amp; raw\n  >\n  > <div>raw &amp;</div>\n  after &amp; raw\n- last &amp; raw\n",
      "- take &amp; raw\n  \n  <div>raw &amp;</div>\n  \n  after &amp; raw\n- last &amp; raw\n" },
    { "> take &amp; raw\n>\n> <div>raw &amp;</div>\nafter &amp; raw\n",
      "take &amp; raw\n\n<div>raw &amp;</div>\n\nafter &amp; raw\n" },
    { "before &amp; raw\n> take 中文 🧡 &amp; raw", "before &amp; raw\n\ntake 中文 🧡 &amp; raw" },
    { "before &amp; raw\n> take &amp; raw\nlazy &amp; raw\n", "before &amp; raw\n\ntake &amp; raw\nlazy &amp; raw\n" },
    { "# before &amp; raw\n> take &amp; raw\n# after &amp; raw\n", "# before &amp; raw\ntake &amp; raw\n# after &amp; raw\n" },
    { "before &amp; raw\n> # take &amp; raw\nafter &amp; raw\n", "before &amp; raw\n# take &amp; raw\nafter &amp; raw\n" },
    { "> before &amp; raw\n> > take &amp; raw\n> >\n> > <div>raw &amp;</div>\n> after &amp; raw\n",
      "> before &amp; raw\n> \n> take &amp; raw\n> \n> <div>raw &amp;</div>\n> \n> after &amp; raw\n" },
    { "- [x] before &amp; raw\n  > take &amp; raw\n- last &amp; raw\n",
      "- [x] before &amp; raw\n  \n  take &amp; raw\n- last &amp; raw\n" },
    { "100. > take &amp; raw\n     >\n     > <div>raw &amp;</div>\n     after &amp; raw\n101. last &amp; raw\n",
      "100. take &amp; raw\n     \n     <div>raw &amp;</div>\n     \n     after &amp; raw\n101. last &amp; raw\n" },
    { "> - [x] before &amp; raw\n>   > take &amp; raw\n>   >\n>   > <div>raw &amp;</div>\n>   after &amp; raw\n> - last &amp; raw\n",
      "> - [x] before &amp; raw\n>   \n>   take &amp; raw\n>   \n>   <div>raw &amp;</div>\n>   \n>   after &amp; raw\n> - last &amp; raw\n" },
    { "before &amp; raw\n> take [a]\n>\n> [a]: /raw 'one\n> two'\n>\n> <div>raw &amp;</div>\nafter &amp; raw\n",
      "before &amp; raw\n\ntake [a]\n\n[a]: /raw 'one\ntwo'\n\n<div>raw &amp;</div>\n\nafter &amp; raw\n" },
    { "before &amp; raw\n> take [^n]\n>\n> [^n]: body &amp;\n>     continuation &amp;\n>\n> [^unused]: raw &amp;\n>     continuation &amp;\n>\n> <div>raw &amp;</div>\nafter &amp; raw\n",
      "before &amp; raw\n\ntake [^n]\n\n[^n]: body &amp;\n    continuation &amp;\n\n[^unused]: raw &amp;\n    continuation &amp;\n\n<div>raw &amp;</div>\n\nafter &amp; raw\n" },
    { "before &amp; raw\r> take &amp; raw\r>\r> <div>raw &amp;</div>\rafter &amp; raw\r",
      "before &amp; raw\r\rtake &amp; raw\r\r<div>raw &amp;</div>\r\rafter &amp; raw\r" }
};
static void boundary_variant(const char* text, unsigned mode, char* output, size_t capacity)
{
    size_t at = 0, i;
    if (mode == 2) { memcpy(output, "\xef\xbb\xbf", 3); at = 3; }
    for (i = 0; text[i]; i++) {
        CHECK(at + 2 < capacity);
        if (mode == 1 && text[i] == '\n') output[at++] = '\r';
        output[at++] = text[i];
    }
    output[at] = 0;
}
static uint64_t boundary_patch_proof(const char* source, const char* output,
    xui_document_snapshot before, xui_document_change_set change)
{
    xui_doc_change_info_t info = {0}; uint64_t i, at, last = UINT64_MAX, removed = 0, added = 0, count = 0;
    info.iSize = sizeof(info); CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
    for (i = 0; i < info.iOperationCount; i++) {
        const xui_doc_operation_t* op = &info.pOperations[i];
        if (op->iKind != XUI_DOC_OP_SOURCE) continue;
        CHECK(op->iOffset <= last && op->iOldLength <= 32 && op->iNewLength <= 2048);
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
static void boundary_child_ids(xui_document_snapshot before, xui_document_snapshot after, uint64_t id)
{
    xui_doc_node_info_t node = {0}; uint64_t i, a, b; node.iSize = sizeof(node);
    CHECK(xuiDocumentSnapshotGetNode(before, id, &node) == XUI_OK);
    for (i = 0; i < node.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(before, id, i, &a) == XUI_OK &&
            xuiDocumentSnapshotGetChild(after, id, i, &b) == XUI_OK && a == b);
        boundary_child_ids(before, after, a);
    }
}
static void boundary_matrix(void)
{
    unsigned ci, mode, gap, affinity, count = 0;
    for (ci = 0; ci < sizeof(boundary_cases) / sizeof(boundary_cases[0]); ci++)
    for (mode = 0; mode < 3; mode++)
    for (gap = 0; gap < 3; gap++)
    for (affinity = 0; affinity < 2; affinity++) {
        char source[8192], expected[8192], actual[8192]; xui_document d, reload;
        xui_document_snapshot before, after, full; xui_document_transaction t;
        xui_doc_position_t caret, at; xui_doc_range_t found; xui_doc_node_info_t quote;
        xui_document_change_set change; uint64_t i, removed; int result;
        if (mode == 1 && strchr(boundary_cases[ci].source, '\r')) continue;
        boundary_variant(boundary_cases[ci].source, mode, source, sizeof(source));
        boundary_variant(boundary_cases[ci].expected, mode, expected, sizeof(expected));
        d = open_md(source); found = find_text(d, "take");
        if (getenv("XUI_BOUNDARY_TRACE")) fprintf(stderr, "boundary trace %u/%u/%u/%u\n", ci, mode, gap, affinity);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        at = unwrap_position(before, found.tAnchor, gap);
        at.iAffinity = affinity; if (!gap) at.iOffset += 2;
        quote = unwrap_quote_node(before, at);
        result = xuiDocumentTxnUnwrapQuote(t, &at, &caret);
        if (result) fprintf(stderr, "boundary %u/%u/%u: %d\n", ci, mode, gap, result);
        CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, sizeof(actual));
        if (strcmp(actual, expected)) fprintf(stderr, "boundary %u/%u/%u respelled:\n%s\n", ci, mode, gap, actual);
        CHECK(!strcmp(actual, expected));
        removed = boundary_patch_proof(source, actual, before, change); xuiDocumentChangeSetRelease(change);
        for (i = 0; i < quote.iChildCount; i++) {
            uint64_t child; xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
            CHECK(xuiDocumentSnapshotGetChild(before, quote.iId, i, &child) == XUI_OK &&
                xuiDocumentSnapshotGetNode(after, child, &node) == XUI_OK && node.iParentId == quote.iParentId);
            same_tree(before, child, after, child); boundary_child_ids(before, after, child);
        }
#ifndef XUI_DLL
        CHECK(unwrap_shared_bytes(before->state->source, after->state->source) == strlen(source) - removed &&
            before->state->reference_values == after->state->reference_values);
#else
        (void)removed;
#endif
        reload = open_md(actual); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(before, after);
#endif
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); count++;
    }
    printf("Quote boundary: %u exact adjacent-block source cases, TEXT/GAP, LF/CRLF/BOM/mixed, full reload and history\n", count);
}
static void boundary_failures(void)
{
    const unsigned samples[] = {0, 1, 3, 7, 13, 15, 17, 18}; unsigned sample; long total = 0;
    for (sample = 0; sample < sizeof(samples) / sizeof(samples[0]); sample++) {
        unsigned ci = samples[sample]; long point; int success = 0;
        for (point = 0; point < 10000 && !success; point++) {
            failing_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_transaction t = NULL; xui_document_snapshot before, after; xui_doc_range_t found;
            xui_doc_position_t caret; char actual[8192]; unsigned live; uint64_t revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = fail_alloc; desc.onFree = fail_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, boundary_cases[ci].source, strlen(boundary_cases[ci].source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            found = find_text(d, "take"); live = a.live; revision = xuiDocumentGetRevision(d); a.remaining = point;
            result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (!result) result = xuiDocumentTxnUnwrapQuote(t, &found.tAnchor, &caret);
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            xuiDocumentTxnRelease(t); a.remaining = -1;
            if (result) CHECK(result == XUI_ERROR_OUT_OF_MEMORY && a.live == live &&
                xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) {
                success = 1; copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, boundary_cases[ci].expected));
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            } else { same_tree(before, 1, after, 1); same_syntax(before, after); }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, boundary_cases[ci].source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success); total += point - 1; printf("Quote boundary OOM %u: %ld points, all failed attempts unpublished and no leaks\n", ci, point - 1);
    }
    printf("Quote boundary OOM total: %ld\n", total);
}
#ifndef XUI_DLL
static void boundary_cancellation(void)
{
    const unsigned samples[] = {0, 1, 3, 7, 13, 15, 17, 18}; unsigned sample; long total = 0;
    for (sample = 0; sample < sizeof(samples) / sizeof(samples[0]); sample++) {
        unsigned ci = samples[sample]; long point; int success = 0;
        for (point = 0; point < 10000 && !success; point++) {
            prefix_cancel_allocator a = {{-1, 0}, -1, ATOMIC_VAR_INIT(0)}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_transaction t; xui_document_snapshot before, after; xui_doc_range_t found;
            xui_doc_position_t caret; char actual[8192]; unsigned live; uint64_t revision; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = prefix_cancel_alloc; desc.onFree = prefix_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, boundary_cases[ci].source, strlen(boundary_cases[ci].source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            found = find_text(d, "take"); live = a.allocation.live; revision = xuiDocumentGetRevision(d);
            CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); t->cancellation = &a.cancellation; a.remaining = point;
            result = xuiDocumentTxnUnwrapQuote(t, &found.tAnchor, &caret);
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            a.remaining = -1; xuiDocumentTxnRelease(t); atomic_store(&a.cancellation, 0);
            if (result) CHECK(result == XUI_DOC_ERROR_CANCELLED && a.allocation.live == live &&
                xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) {
                success = 1; copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, boundary_cases[ci].expected));
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            } else { same_tree(before, 1, after, 1); same_syntax(before, after); reference_cache_snapshot_equal(before, after); }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, boundary_cases[ci].source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.allocation.live);
        }
        CHECK(success); total += point - 1; printf("Quote boundary cancellation %u: %ld checkpoints, atomic state and no leaks\n", ci, point - 1);
    }
    printf("Quote boundary cancellation total: %ld\n", total);
}
#endif
static void boundary_depth_and_compound(void)
{
    const unsigned depths[] = {0, 1, 8, 32, 64, 123}; unsigned di;
    const char* lines[] = {"before &amp; raw", "take &amp; raw", "", "<div>raw &amp;</div>", "after &amp; raw"};
    for (di = 0; di < sizeof(depths) / sizeof(depths[0]); di++) {
        char source[8192], expected[8192], actual[8192]; size_t a = 0, b = 0; unsigned line, depth;
        xui_document d; xui_document_transaction t; xui_doc_range_t found; xui_doc_position_t caret;
        for (line = 0; line < sizeof(lines) / sizeof(lines[0]); line++) {
            if (line == 1 || line == 4) {
                for (depth = 0; depth < depths[di]; depth++) b += (size_t)sprintf(expected + b, "> ");
                b += (size_t)sprintf(expected + b, "\n");
            }
            for (depth = 0; depth < depths[di] + (line > 0 && line < 4); depth++) a += (size_t)sprintf(source + a, "> ");
            a += (size_t)sprintf(source + a, "%s\n", lines[line]);
            for (depth = 0; depth < depths[di]; depth++) b += (size_t)sprintf(expected + b, "> ");
            b += (size_t)sprintf(expected + b, "%s\n", lines[line]);
        }
        CHECK(a < sizeof(source) && b < sizeof(expected)); d = open_md(source); found = find_text(d, "take");
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnUnwrapQuote(t, &found.tAnchor, &caret) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        xuiDocumentRelease(d);
    }
    {
        const char* source = "before &amp; raw\n> take &amp; raw\n\nbetween &amp; raw\n> second &amp; raw\n>\n> <div>raw &amp;</div>\nafter &amp; raw\n";
        const char* expected = "before &amp; raw\n\ntake &amp; raw\n\nbetween &amp; raw\n\nsecond &amp; raw\n\n<div>raw &amp;</div>\n\nafter &amp; raw\n";
        xui_document d = open_md(source); xui_document_transaction t; xui_doc_range_t first = find_text(d, "take"), second = find_text(d, "second");
        xui_doc_position_t caret; char actual[8192]; uint64_t revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnUnwrapQuote(t, &first.tAnchor, &caret) == XUI_OK && xuiDocumentTxnUnwrapQuote(t, &second.tAnchor, &caret) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source) && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentGetRevision(d) == revision + 1); xuiDocumentTxnRelease(t);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d)); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        xuiDocumentRelease(d);
    }
    puts("Quote boundary: root/1/8/32/64/123 parent depths and a two-quote transaction, exact source and single shared Undo");
}
static void boundary_rich(void)
{
    const unsigned samples[] = {2, 3, 9, 10, 11, 12, 14}; unsigned si;
    for (si = 0; si < sizeof(samples) / sizeof(samples[0]); si++) {
        unsigned ci = samples[si]; xui_document md, rich, expected_md, expected_rich;
        xui_document_snapshot source, before, after, full; xui_doc_rich_conversion_report_t report = {0};
        xui_document_transaction t; xui_doc_range_t at; xui_doc_position_t caret;
        report.iSize = sizeof(report); md = open_md(boundary_cases[ci].source);
        CHECK(xuiDocumentAcquireSnapshot(md, &source) == XUI_OK && xuiDocumentSnapshotAnalyzeRichConversion(source, &report) == XUI_OK &&
            xuiDocumentSnapshotConvertToRich(source, report.iReasons, &rich) == XUI_OK);
        xuiDocumentSnapshotRelease(source); xuiDocumentRelease(md);
        expected_md = open_md(boundary_cases[ci].expected);
        CHECK(xuiDocumentAcquireSnapshot(expected_md, &source) == XUI_OK && xuiDocumentSnapshotAnalyzeRichConversion(source, &report) == XUI_OK &&
            xuiDocumentSnapshotConvertToRich(source, report.iReasons, &expected_rich) == XUI_OK);
        xuiDocumentSnapshotRelease(source); xuiDocumentRelease(expected_md); at = find_text(rich, "take");
        CHECK(xuiDocumentClearHistory(rich) == XUI_OK && xuiDocumentAcquireSnapshot(rich, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(rich, NULL, &t) == XUI_OK && xuiDocumentTxnUnwrapQuote(t, &at.tAnchor, &caret) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(rich, &after) == XUI_OK &&
            xuiDocumentAcquireSnapshot(expected_rich, &full) == XUI_OK);
        xuiDocumentTxnRelease(t); same_tree(after, 1, full, 1);
        xuiDocumentSnapshotRelease(after); xuiDocumentSnapshotRelease(full);
        CHECK(xuiDocumentUndo(rich, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(rich, &after) == XUI_OK);
        same_tree(before, 1, after, 1); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after);
        CHECK(!xuiDocumentCanUndo(rich) && xuiDocumentCanRedo(rich));
        xuiDocumentRelease(rich); xuiDocumentRelease(expected_rich);
    }
    puts("Quote boundary: 7 Rich equivalents, paragraph seams, ordered children and shared Undo");
}
static void boundary_large_source(void)
{
    const unsigned sizes[] = {4096, 131072, 1048576}; unsigned si; uint64_t history = 0;
    for (si = 0; si < sizeof(sizes) / sizeof(sizes[0]); si++) {
        const char* head = "before &amp; raw\n> take\n>\n> [^unused]: ";
        const char* expected_head = "before &amp; raw\n\ntake\n\n[^unused]: ";
        const char* tail = "\n>\n> <div>raw &amp;</div>\nafter &amp; raw\n";
        const char* expected_tail = "\n\n<div>raw &amp;</div>\n\nafter &amp; raw\n";
        size_t capacity = sizes[si] + 256, length; char* source = malloc(capacity);
        char* expected = malloc(capacity); char* actual = malloc(capacity);
        xui_document d, reload; xui_document_snapshot before, after, full; xui_document_transaction t;
        xui_document_change_set change; xui_doc_change_info_t info = {0}; uint64_t i, removed = 0;
        xui_doc_range_t at; xui_doc_position_t caret; xui_doc_memory_stats_t stats = {0};
        CHECK(source && expected && actual);
        strcpy(source, head); length = strlen(head); memset(source + length, 'q', sizes[si]); strcpy(source + length + sizes[si], tail);
        strcpy(expected, expected_head); length = strlen(expected_head); memset(expected + length, 'q', sizes[si]);
        strcpy(expected + length + sizes[si], expected_tail); d = open_md(source); at = find_text(d, "take");
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK && xuiDocumentTxnUnwrapQuote(t, &at.tAnchor, &caret) == XUI_OK &&
            xuiDocumentTxnCommit(t, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, capacity); CHECK(!strcmp(actual, expected));
        info.iSize = sizeof(info); CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
        for (i = 0; i < info.iOperationCount; i++) if (info.pOperations[i].iKind == XUI_DOC_OP_SOURCE) {
            CHECK(info.pOperations[i].iOldLength <= 16 && info.pOperations[i].iNewLength <= 2);
            removed += info.pOperations[i].iOldLength;
        }
        xuiDocumentChangeSetRelease(change);
#ifndef XUI_DLL
        CHECK(unwrap_shared_bytes(before->state->source, after->state->source) == strlen(source) - removed &&
            before->state->reference_values == after->state->reference_values);
        reference_cache_snapshot_equal(before, after);
#else
        (void)removed;
#endif
        stats.iSize = sizeof(stats); CHECK(xuiDocumentGetMemoryStats(d, &stats) == XUI_OK && stats.iHistoryBytes < 65536);
        if (history) CHECK(stats.iHistoryBytes == history);
        history = stats.iHistoryBytes;
        reload = open_md(actual); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full); xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        printf("Quote boundary %u-byte hidden body: HistoryBytes=%llu\n", sizes[si], (unsigned long long)history);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, capacity); CHECK(!strcmp(actual, source));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); copy_source(d, actual, capacity); CHECK(!strcmp(actual, expected));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
        free(source); free(expected); free(actual);
    }
}
#ifdef XUI_QUOTE_TEST_EDITOR
static void boundary_editor(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    unsigned ci, gap, mode, affinity, total = 0, refused = 0;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    for (ci = 0; ci < sizeof(boundary_cases) / sizeof(boundary_cases[0]); ci++)
    for (gap = 0; gap < 3; gap++)
    for (mode = 0; mode < 3; mode++)
    for (affinity = 0; affinity < 2; affinity++) {
        xui_document d = open_md(boundary_cases[ci].source); xui_document_snapshot before;
        xui_doc_range_t range = find_text(d, "take"), selected; xui_widget editor;
        xui_doc_editor_desc_t desc = {0}; xui_doc_command_state_t state = {0}; char actual[8192]; uint64_t revision;
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        range.tAnchor = unwrap_position(before, range.tAnchor, gap);
        range.tAnchor.iAffinity = affinity; if (!gap) range.tAnchor.iOffset += 2;
        range.tCaret = range.tAnchor; xuiDocumentSnapshotRelease(before); revision = xuiDocumentGetRevision(d);
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d;
        desc.iMode = mode == 2 ? XUI_DOC_SOURCE_TEXT : mode == 1 ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_VISUAL;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK && xuiWidgetSetRect(editor, (xui_rect_t){0,0,640,480}) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        if (mode) {
            state.iSize = sizeof(state);
            CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && !state.bEnabled &&
                state.iDisabledReason == XUI_ERROR_UNSUPPORTED && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_ERROR_UNSUPPORTED &&
                xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, boundary_cases[ci].source)); refused++;
            CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        }
        state.iSize = sizeof(state); CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && state.bEnabled && state.bActive);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK && xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &selected) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, boundary_cases[ci].expected));
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
            xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, boundary_cases[ci].source));
        CHECK(!xuiDocumentCanUndo(d) && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, boundary_cases[ci].expected));
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); total++;
    }
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Quote boundary Editor: %u actual DLL VISUAL Query/Execute/Caret cases, %u LIVE/SOURCE atomic refusals and three-mode history\n", total, refused);
}
#endif
#ifndef XUI_QUOTE_BOUNDARY_NO_MAIN
int main(void)
{
    boundary_matrix(); boundary_depth_and_compound(); boundary_rich(); boundary_large_source(); boundary_failures();
#ifndef XUI_DLL
    boundary_cancellation();
#endif
#ifdef XUI_QUOTE_TEST_EDITOR
    boundary_editor();
#endif
    return 0;
}
#endif
