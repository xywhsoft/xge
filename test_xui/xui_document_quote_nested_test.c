#include "../xui_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef XUI_QUOTE_TEST_EDITOR
#include "../xui_document_ui.h"
#include "../xge.h"
#include "../src/xui_internal.h"
#include "xui_test_proxy.h"
#endif

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); exit(1); } } while (0)
static xui_document open_md(const char* source)
{
    xui_doc_desc_t desc = {0}; xui_document d;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK);
    return d;
}
static xui_doc_range_t find_text(xui_document d, const char* text)
{
    xui_document_snapshot s; xui_doc_range_t range; uint64_t count;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(s, XUI_DOC_SEMANTIC, text, strlen(text), NULL,
        &range, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(s); return range;
}
static void copy_source(xui_document d, char* output, size_t capacity)
{
    xui_document_snapshot s; uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(s, output, capacity, &bytes) == XUI_OK &&
        bytes < capacity);
    xuiDocumentSnapshotRelease(s);
}
static int below(xui_document_snapshot s, uint64_t id, uint64_t ancestor)
{
    unsigned depth;
    for (depth = 0; id && depth < 130; depth++) {
        xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
        if (id == ancestor) return 1;
        CHECK(xuiDocumentSnapshotGetNode(s, id, &node) == XUI_OK);
        id = node.iParentId;
    }
    return 0;
}
static void same_tree(xui_document_snapshot a, uint64_t ai,
    xui_document_snapshot b, uint64_t bi)
{
    xui_doc_node_info_t x = {0}, y = {0}; uint64_t i;
    char tx[1024], ty[1024]; uint64_t nx, ny;
    x.iSize = y.iSize = sizeof(x);
    CHECK(xuiDocumentSnapshotGetNode(a, ai, &x) == XUI_OK &&
        xuiDocumentSnapshotGetNode(b, bi, &y) == XUI_OK);
    CHECK(x.iKind == y.iKind && x.iChildCount == y.iChildCount &&
        x.tAttributes.iFlags == y.tAttributes.iFlags &&
        x.tAttributes.iMarks == y.tAttributes.iMarks &&
        x.tAttributes.iListStart == y.tAttributes.iListStart &&
        x.tAttributes.iHeadingLevel == y.tAttributes.iHeadingLevel &&
        !strcmp(x.sInfo, y.sInfo) && !strcmp(x.sResource, y.sResource) && !strcmp(x.sTitle, y.sTitle));
    CHECK(xuiDocumentSnapshotCopyText(a, ai, tx, sizeof(tx), &nx) == XUI_OK &&
        xuiDocumentSnapshotCopyText(b, bi, ty, sizeof(ty), &ny) == XUI_OK &&
        nx == ny && !memcmp(tx, ty, (size_t)nx));
    for (i = 0; i < x.iChildCount; i++) {
        uint64_t ac, bc;
        CHECK(xuiDocumentSnapshotGetChild(a, ai, i, &ac) == XUI_OK &&
            xuiDocumentSnapshotGetChild(b, bi, i, &bc) == XUI_OK);
        same_tree(a, ac, b, bc);
    }
}
static unsigned count_tasks(xui_document_snapshot s, uint64_t id)
{
    xui_doc_node_info_t node = {0}; uint64_t i, child; unsigned count;
    node.iSize = sizeof(node); CHECK(xuiDocumentSnapshotGetNode(s, id, &node) == XUI_OK);
    count = !!(node.tAttributes.iFlags & XUI_DOC_TASK);
    for (i = 0; i < node.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK);
        count += count_tasks(s, child);
    }
    return count;
}
static void container_metadata(xui_document_snapshot s, uint64_t id,
    xui_document_transaction t, uint64_t wrapper)
{
    xui_doc_node_info_t node = {0}; uint64_t i, child;
    node.iSize = sizeof(node); CHECK(xuiDocumentSnapshotGetNode(s, id, &node) == XUI_OK);
    if ((node.iKind == XUI_DOC_QUOTE || node.iKind == XUI_DOC_LIST ||
        node.iKind == XUI_DOC_LIST_ITEM) && id != wrapper) {
        if (t) CHECK(xuiDocumentTxnSetResource(t, id, "/container-resource", "container-info", "container-title") == XUI_OK);
        else CHECK(!strcmp(node.sResource, "/container-resource") &&
            !strcmp(node.sInfo, "container-info") && !strcmp(node.sTitle, "container-title"));
    }
    for (i = 0; i < node.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(s, id, i, &child) == XUI_OK);
        container_metadata(s, child, t, wrapper);
    }
}
typedef struct quote_case {
    const char* source;
    const char* left;
    const char* right;
} quote_case;
static const quote_case cases[] = {
    { "pre &amp; untouched\n\n> > left &amp; raw\n> >\n> > take\n>\n> outertail\n\noutside\n\npost &amp; untouched\n", "take", "outside" },
    { "pre &amp; untouched\n\n> > left &amp; raw\n> >\n> > take\n>\n> outertail\n\nmid\n\n> before\n>\n> > also\n> >\n> > right &amp; raw\n\npost &amp; untouched\n", "take", "also" },
    { "pre &amp; untouched\n\n- left &amp; raw\n\n  take\n\n  itemtail\n- second\n\noutside\n\npost &amp; untouched\n", "take", "outside" },
    { "pre &amp; untouched\n\n7. left &amp; raw\n\n   take\n\n   also\n\n   right &amp; raw\n8. last\n\npost &amp; untouched\n", "take", "also" },
    { "pre &amp; untouched\n\n- [x] left &amp; raw\n\n  take\n\n  also\n\n  right &amp; raw\n- last\n\noutside\n\npost &amp; untouched\n", "take", "outside" },
    { "pre &amp; untouched\n\n> - parent\n>   > > left &amp; raw\n>   > >\n>   > > take\n>   >\n>   > outertail\n> - cousin\n\noutside\n\npost &amp; untouched\n", "take", "outside" },
    { "pre &amp; untouched\n\n3. left &amp; raw\n\n   take\n4. also\n\n   right &amp; raw\n\npost &amp; untouched\n", "take", "also" },
    { "pre &amp; untouched\n\n3. left &amp; raw\n\n   take\n4. also\n5. right &amp; raw\n\npost &amp; untouched\n", "take", "also" },
    { "pre &amp; untouched\n\n3. left &amp; raw\n\n   take\n4. also\n77. right &amp; raw\n\npost &amp; untouched\n", "take", "also" }
};
static xui_doc_position_t block_gap(xui_document_snapshot s,
    xui_doc_position_t position, int end)
{
    xui_doc_node_info_t node = {0}, parent = {0}; uint64_t i, child;
    node.iSize = parent.iSize = sizeof(node);
    CHECK(xuiDocumentSnapshotGetNode(s, position.iNodeId, &node) == XUI_OK &&
        xuiDocumentSnapshotGetNode(s, node.iParentId, &parent) == XUI_OK);
    if (node.iKind == XUI_DOC_TEXT) node = parent;
    CHECK(xuiDocumentSnapshotGetNode(s, node.iParentId, &parent) == XUI_OK);
    for (i = 0; i < parent.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(s, parent.iId, i, &child) == XUI_OK);
        if (child == node.iId) break;
    }
    CHECK(i < parent.iChildCount);
    position.iKind = XUI_DOC_POSITION_GAP;
    position.iNodeId = parent.iId; position.iOffset = i + (uint64_t)end;
    return position;
}
static void same_syntax(xui_document_snapshot a, xui_document_snapshot b)
{
    xui_doc_source_info_t x = {0}, y = {0}; uint64_t i;
    x.iSize = y.iSize = sizeof(x);
    CHECK(xuiDocumentSnapshotGetSourceInfo(a, &x) == XUI_OK &&
        xuiDocumentSnapshotGetSourceInfo(b, &y) == XUI_OK &&
        x.iInlineSyntaxCount == y.iInlineSyntaxCount &&
        x.iReferenceCandidateCount == y.iReferenceCandidateCount &&
        x.iReferenceDefinitionCount == y.iReferenceDefinitionCount);
    for (i = 0; i < x.iInlineSyntaxCount + x.iReferenceCandidateCount; i++) {
        xui_doc_inline_syntax_t u = {0}, v = {0}; u.iSize = v.iSize = sizeof(u);
        if (i < x.iInlineSyntaxCount) {
            CHECK(xuiDocumentSnapshotGetInlineSyntax(a, i, &u) == XUI_OK &&
                xuiDocumentSnapshotGetInlineSyntax(b, i, &v) == XUI_OK);
        } else {
            CHECK(xuiDocumentSnapshotGetReferenceCandidate(a, i - x.iInlineSyntaxCount, &u) == XUI_OK &&
                xuiDocumentSnapshotGetReferenceCandidate(b, i - x.iInlineSyntaxCount, &v) == XUI_OK);
        }
        CHECK(!memcmp(&u, &v, sizeof(u)));
    }
    for (i = 0; i < x.iReferenceDefinitionCount; i++) {
        xui_doc_reference_definition_t u = {0}, v = {0}; u.iSize = v.iSize = sizeof(u);
        CHECK(xuiDocumentSnapshotGetReferenceDefinition(a, i, &u) == XUI_OK &&
            xuiDocumentSnapshotGetReferenceDefinition(b, i, &v) == XUI_OK &&
            !memcmp(&u, &v, sizeof(u)));
    }
}
static void nested_cases(void)
{
    size_t ci; unsigned reverse, gaps, rich, endings;
    for (ci = 0; ci < sizeof(cases) / sizeof(cases[0]); ci++)
    for (reverse = 0; reverse < 2; reverse++)
    for (gaps = 0; gaps < 4; gaps++)
    for (rich = 0; rich < 2; rich++)
    for (endings = 0; endings < 3; endings++) {
        char original[8192]; size_t at = 0, j; unsigned line = 0;
        xui_document d, reloaded;
        xui_document_transaction t;
        xui_document_snapshot before, after, full;
        xui_doc_range_t left, right, range, selected, keep_left;
        xui_doc_node_id quote = 0; char output[8192], restored[8192]; int result;
        if (endings == 2) { memcpy(original, "\xef\xbb\xbf", 3); at = 3; }
        for (j = 0; cases[ci].source[j]; j++) {
            if (cases[ci].source[j] == '\n' && (endings == 1 || (endings == 2 && !(line++ & 1))))
                original[at++] = '\r';
            original[at++] = cases[ci].source[j];
        }
        original[at] = 0; d = open_md(original);
        if (rich) {
            xui_doc_rich_conversion_report_t report = {0}; report.iSize = sizeof(report);
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
                xuiDocumentSnapshotAnalyzeRichConversion(before, &report) == XUI_OK &&
                xuiDocumentSnapshotConvertToRich(before, report.iReasons, &reloaded) == XUI_OK);
            xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); d = reloaded;
            CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
                xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
            container_metadata(before, 1, t, 0);
            CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK);
            xuiDocumentTxnRelease(t); xuiDocumentSnapshotRelease(before);
        }
        left = find_text(d, cases[ci].left); right = find_text(d, cases[ci].right);
        keep_left = find_text(d, "left & raw");
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        range.tAnchor = gaps & 1 ? block_gap(before, left.tAnchor, 0) : left.tAnchor;
        range.tCaret = gaps & 2 ? block_gap(before, right.tCaret, 1) : right.tCaret;
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        {
            xui_document_snapshot aborted; uint64_t revision = xuiDocumentGetRevision(d);
            CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
                xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK);
            xuiDocumentTxnRelease(t);
            CHECK(xuiDocumentGetRevision(d) == revision && xuiDocumentAcquireSnapshot(d, &aborted) == XUI_OK);
            same_tree(before, 1, aborted, 1);
            if (!rich) { copy_source(d, output, sizeof(output)); CHECK(!strcmp(output, original)); same_syntax(before, aborted); }
            xuiDocumentSnapshotRelease(aborted);
        }
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
        if (result != XUI_OK) fprintf(stderr, "quote case %zu reverse %u: %d\n", ci, reverse, result);
        CHECK(result == XUI_OK && quote);
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t);
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        CHECK(below(after, left.tAnchor.iNodeId, quote) &&
            below(after, right.tCaret.iNodeId, quote) &&
            !below(after, keep_left.tAnchor.iNodeId, quote) &&
            count_tasks(before, 1) == count_tasks(after, 1));
        same_tree(before, keep_left.tAnchor.iNodeId, after, keep_left.tAnchor.iNodeId);
        if (ci == 1 || ci == 3 || ci >= 6) {
            xui_doc_range_t tail = find_text(d, "right & raw");
            CHECK(!below(after, tail.tAnchor.iNodeId, quote));
        }
        if (ci >= 6) {
            xui_doc_node_info_t list = {0}; uint64_t id; list.iSize = sizeof(list);
            CHECK(xuiDocumentSnapshotGetChild(after, quote, 0, &id) == XUI_OK &&
                xuiDocumentSnapshotGetNode(after, id, &list) == XUI_OK &&
                list.iKind == XUI_DOC_LIST && list.iChildCount == 2 && list.tAttributes.iListStart == 4 &&
                xuiDocumentSnapshotGetChild(after, 1, 3, &id) == XUI_OK &&
                xuiDocumentSnapshotGetNode(after, id, &list) == XUI_OK &&
                list.iKind == XUI_DOC_LIST && list.tAttributes.iListStart == 6);
        }
        if (rich) {
            container_metadata(after, 1, NULL, quote);
            CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(d, &full) == XUI_OK);
            same_tree(before, 1, full, 1); xuiDocumentSnapshotRelease(full);
            CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(d, &full) == XUI_OK);
            same_tree(after, 1, full, 1); xuiDocumentSnapshotRelease(full);
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after);
            xuiDocumentRelease(d); continue;
        }
        copy_source(d, output, sizeof(output));
        if (getenv("XUI_QUOTE_TEST_DUMP")) printf("case %zu reverse %u:\n%s\n", ci, reverse, output);
        {
            const char* left_end = strstr(original, "left &amp; raw") + strlen("left &amp; raw");
            const char* post = strstr(original, "post &amp; untouched");
            if (*left_end == '\r') left_end++;
            if (*left_end == '\n') left_end++;
            CHECK(!memcmp(output, original, (size_t)(left_end - original)) &&
                strlen(output) >= strlen(post) &&
                !strcmp(output + strlen(output) - strlen(post), post));
            if (ci == 1) {
                const char* tail = strstr(original, "> > right &amp; raw");
                CHECK(strlen(output) >= strlen(tail) && !strcmp(output + strlen(output) - strlen(tail), tail));
            }
        }
        reloaded = open_md(output);
        CHECK(xuiDocumentAcquireSnapshot(reloaded, &full) == XUI_OK);
        same_tree(after, XUI_DOCUMENT_ROOT, full, XUI_DOCUMENT_ROOT);
        same_syntax(after, full);
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reloaded);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
        copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, original));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
        copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, output));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after);
        xuiDocumentRelease(d);
    }
    printf("Nested quote range: %zu Rich/Markdown cases; TEXT/GAP, reverse, LF/CRLF/mixed+BOM, Abort, reload and Undo/Redo\n",
        sizeof(cases) / sizeof(cases[0]) * 48);
}
static void deep_cases(void)
{
    static const unsigned depths[] = {1, 2, 8, 32, 64, 124};
    size_t di; unsigned reverse;
    for (di = 0; di < sizeof(depths) / sizeof(depths[0]); di++)
    for (reverse = 0; reverse < 2; reverse++) {
        char original[4096], output[131072]; size_t at = 0; unsigned i, line;
        xui_document d, reloaded; xui_document_snapshot s, full;
        xui_document_transaction t; xui_doc_range_t range, end, selected;
        uint64_t quote;
        const char* texts[] = {"left &amp; raw", "", "take"};
        at += (size_t)sprintf(original + at, "pre &amp; untouched\n\n");
        for (line = 0; line < 3; line++) {
            for (i = 0; i < depths[di]; i++) {
                original[at++] = '>'; original[at++] = ' ';
            }
            at += (size_t)sprintf(original + at, "%s\n", texts[line]);
        }
        at += (size_t)sprintf(original + at, "\noutside\n\npost &amp; untouched\n");
        CHECK(at < sizeof(original)); d = open_md(original);
        range = find_text(d, "take"); end = find_text(d, "outside"); range.tCaret = end.tCaret;
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, output, sizeof(output));
        CHECK(strstr(output, "left &amp; raw"));
        reloaded = open_md(output);
        CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK &&
            xuiDocumentAcquireSnapshot(reloaded, &full) == XUI_OK);
        same_tree(s, 1, full, 1); same_syntax(s, full);
        xuiDocumentSnapshotRelease(s); xuiDocumentSnapshotRelease(full);
        xuiDocumentRelease(reloaded); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
        copy_source(d, output, sizeof(output)); CHECK(!strcmp(output, original));
        xuiDocumentRelease(d);
    }
    puts("Nested quote depth: 1/2/8/32/64/124 layers, forward/reverse, full reload and exact Undo");
}
static xui_document ordered_limit_document(void)
{
    xui_document md = open_md(cases[6].source), rich;
    xui_document_snapshot s; xui_document_transaction t;
    xui_doc_rich_conversion_report_t report = {0}; xui_doc_node_info_t list = {0};
    uint64_t id;
    report.iSize = sizeof(report); list.iSize = sizeof(list);
    CHECK(xuiDocumentAcquireSnapshot(md, &s) == XUI_OK &&
        xuiDocumentSnapshotAnalyzeRichConversion(s, &report) == XUI_OK &&
        xuiDocumentSnapshotConvertToRich(s, report.iReasons, &rich) == XUI_OK);
    xuiDocumentSnapshotRelease(s); xuiDocumentRelease(md);
    CHECK(xuiDocumentAcquireSnapshot(rich, &s) == XUI_OK &&
        xuiDocumentSnapshotGetChild(s, 1, 1, &id) == XUI_OK &&
        xuiDocumentSnapshotGetNode(s, id, &list) == XUI_OK && list.iKind == XUI_DOC_LIST);
    list.tAttributes.iListStart = UINT64_MAX - 2;
    CHECK(xuiDocumentBeginTransaction(rich, NULL, &t) == XUI_OK &&
        xuiDocumentTxnSetAttributes(t, id, &list.tAttributes) == XUI_OK &&
        xuiDocumentTxnCommit(t, NULL) == XUI_OK);
    xuiDocumentTxnRelease(t); xuiDocumentSnapshotRelease(s);
    CHECK(xuiDocumentClearHistory(rich) == XUI_OK); return rich;
}
static void ordered_limit_case(void)
{
    xui_document d = ordered_limit_document(); xui_document_snapshot before, after;
    xui_document_transaction t; xui_doc_range_t range = find_text(d, "take"), selected;
    uint64_t quote = 0, revision = xuiDocumentGetRevision(d);
    range.tCaret = find_text(d, "also").tCaret;
    CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
        xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
        xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_DOC_ERROR_LIMIT &&
        !quote && xuiDocumentTxnCommit(t, NULL) == XUI_DOC_ERROR_LIMIT);
    xuiDocumentTxnRelease(t);
    CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d) &&
        xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
    same_tree(before, 1, after, 1);
    xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    puts("Nested quote ordered overflow: both item splits included in preflight; atomic rejection");
}
typedef struct failing_allocator { long remaining; unsigned live; } failing_allocator;
static void* fail_alloc(void* user, size_t bytes)
{
    failing_allocator* a = user; void* result;
    if (!a->remaining) return NULL;
    if (a->remaining > 0) a->remaining--;
    result = malloc(bytes); if (result) a->live++; return result;
}
static void fail_free(void* user, void* pointer)
{
    failing_allocator* a = user; CHECK(a->live); a->live--; free(pointer);
}
static void allocation_failures(void)
{
    size_t ci;
    for (ci = 0; ci < sizeof(cases) / sizeof(cases[0]); ci++) {
        long point; int success = 0;
        for (point = 0; point < 4000 && !success; point++) {
            failing_allocator a = {-1, 0}; xui_doc_desc_t desc = {0};
            xui_document d; xui_document_transaction t = NULL;
            xui_document_snapshot before, after; xui_doc_range_t range, selected;
            uint64_t quote = 0, revision; int result; char output[8192];
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = fail_alloc; desc.onFree = fail_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, cases[ci].source, strlen(cases[ci].source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK &&
                xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = find_text(d, cases[ci].left);
            range.tCaret = find_text(d, cases[ci].right).tCaret;
            if (ci & 1) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
            if (ci == 1) {
                range.tAnchor = block_gap(before, range.tAnchor, 1);
                range.tCaret = block_gap(before, range.tCaret, 0);
            }
            revision = xuiDocumentGetRevision(d); a.remaining = point;
            result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (result == XUI_OK) result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            xuiDocumentTxnRelease(t); a.remaining = -1;
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result == XUI_OK) {
                success = 1; CHECK(quote && xuiDocumentGetRevision(d) == revision + 1);
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            } else {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY && xuiDocumentGetRevision(d) == revision);
                same_tree(before, 1, after, 1); same_syntax(before, after);
                CHECK(xuiDocumentUndo(d, NULL) == XUI_ERROR_NOT_FOUND);
            }
            copy_source(d, output, sizeof(output)); CHECK(!strcmp(output, cases[ci].source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after);
            xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success);
        printf("Nested quote allocation sweep %zu: %ld failure points and successful retry, no leaks\n", ci, point - 1);
    }
}
#ifdef XUI_QUOTE_TEST_EDITOR
static void editor_cases(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    size_t ci; unsigned reverse, gaps;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    for (ci = 0; ci < sizeof(cases) / sizeof(cases[0]); ci++)
    for (reverse = 0; reverse < 2; reverse++)
    for (gaps = 0; gaps < 4; gaps++) {
        xui_document d = open_md(cases[ci].source);
        xui_doc_editor_desc_t desc = {0}; xui_widget editor;
        xui_doc_range_t left = find_text(d, cases[ci].left), right = find_text(d, cases[ci].right);
        xui_doc_range_t range, after;
        xui_document_snapshot s; xui_doc_command_state_t state = {0};
        char output[8192], restored[8192]; uint64_t revision;
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
        desc.tView.pDocument = d; desc.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 480}) == XUI_OK &&
            xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
        range.tAnchor = gaps & 1 ? block_gap(s, left.tAnchor, 0) : left.tAnchor;
        range.tCaret = gaps & 2 ? block_gap(s, right.tCaret, 1) : right.tCaret;
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        xuiDocumentSnapshotRelease(s);
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        revision = xuiDocumentGetRevision(d); state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK &&
            state.bEnabled && !state.bActive && !state.iDisabledReason &&
            xuiDocumentEditorCanExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) &&
            xuiDocumentGetRevision(d) == revision);
        copy_source(d, output, sizeof(output)); CHECK(!strcmp(output, cases[ci].source));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK &&
            xuiDocumentViewGetSelection(editor, &after) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &after) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK);
        copy_source(d, output, sizeof(output));
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
            xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK &&
            !state.bEnabled && state.iDisabledReason == XUI_ERROR_UNSUPPORTED &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_ERROR_UNSUPPORTED &&
            xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
            xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
        copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, output));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, cases[ci].source));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, output));
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        xui_document d = ordered_limit_document(); xui_widget editor;
        xui_doc_editor_desc_t desc = {0}; xui_doc_command_state_t state = {0};
        xui_doc_range_t range = find_text(d, "take"); uint64_t revision = xuiDocumentGetRevision(d);
        range.tCaret = find_text(d, "also").tCaret;
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
        desc.tView.pDocument = d; desc.iMode = XUI_DOC_VISUAL; state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &range) == XUI_OK &&
            xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK &&
            !state.bEnabled && state.iDisabledReason == XUI_DOC_ERROR_LIMIT &&
            !xuiDocumentEditorCanExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_DOC_ERROR_LIMIT &&
            xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Nested quote editor: Query/CanExecute/Execute, mixed GAP/TEXT, reverse, three-mode views and shared history");
}
#endif
int main(void)
{
    nested_cases(); deep_cases(); ordered_limit_case(); allocation_failures();
#ifdef XUI_QUOTE_TEST_EDITOR
    editor_cases();
#endif
    return 0;
}
