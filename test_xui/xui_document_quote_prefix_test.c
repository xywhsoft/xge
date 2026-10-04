#define main quote_nested_previous_main
#include "xui_document_quote_nested_test.c"
#undef main
#ifndef XUI_DLL
#include "xui_document_reference_cache_oracle.h"
#endif

typedef struct prefix_case { const char* source; } prefix_case;
static const prefix_case prefix_cases[] = {
    { "pre &amp; untouched\n\n> before &amp; raw\n>\n> take [a]\n>\n> [a]: /first \"one\n> two\"\n>\n> also [a]\n>\n> after &amp; raw\n\npost &amp; untouched\n" },
    { "pre &amp; untouched\n\n> > before &amp; raw\n> >\n> > take [a]\n> >\n> > [a]: /first \"one\n> > two\"\n> >\n> > also [a]\n> >\n> > after &amp; raw\n\npost &amp; untouched\n" },
    { "pre &amp; untouched\n\n- before &amp; raw\n\n  take [a]\n\n  [a]: /first \"one\n  two\"\n\n  also [a]\n\n  after &amp; raw\n- last [a]\n\npost &amp; untouched\n" },
    { "pre &amp; untouched\n\n> before &amp; raw\n>\n> take [multi label]\n>\n> [multi\n> label]:\n>   <https://one.test/a?b=&amp;>\n>   'raw &amp;\n> title'\n>\n> also [multi label]\n>\n> after &amp; raw\n\npost &amp; untouched\n" },
    { "pre &amp; untouched\n\n> - [x] before &amp; raw\n>\n>   take [a]\n>\n>   [a]: /first \"one\n>   two\"\n>\n>   also [a]\n>\n>   after &amp; raw\n> - last [a]\n\npost &amp; untouched\n" },
    { "pre &amp; untouched\n\n> before &amp; raw\n>\n> take [a]\n>\n> [a]: /first \"one\n> two\"\n>\n> ~~~~ lang opaque &amp; info\n> literal <&> \\raw\n> ~~~~\n>\n> | raw &amp; | escaped \\| pipe |\n> | :-- | --: |\n> | `code|pipe` | cell [a] |\n>\n> also [a]\n>\n> after &amp; raw\n\npost &amp; untouched\n" },
    { "pre &amp; untouched [a]\n\n[a]: /global 'first wins'\n\n> before &amp; raw [a]\n>\n> take [a]\n>\n> [a]: /unused \"one\n> two\"\n>\n> [Unused\\!]: /opaque 'raw &amp;'\n>\n> also [a]\n>\n> after &amp; raw\n\npost &amp; untouched [a]\n" },
    { "pre &amp; untouched [^n]\n\n> before &amp; raw\n>\n> take [a]\n>\n> [a]: /first \"one\n> two\"\n>\n> also [a]\n>\n> after &amp; raw\n\n[^n]: foot &amp; body\n\n[^unused]: opaque untouched\n\npost &amp; untouched\n" },
    { "pre &amp; untouched\n\n> before &amp; raw\n>\n> take\n>\n> [unused]: /first \"one\n> two\"\n>\n> also\n>\n> after &amp; raw\n\npost &amp; untouched\n" },
    { "pre &amp; untouched\n\n> [leading]: /lead 'outside inner quote'\n>\n> take [a]\n>\n> [a]: /first \"one\n> two\"\n>\n> also [a]\n>\n> [trailing]: /tail 'outside inner quote'\n\nafter &amp; raw\n\npost &amp; untouched\n" }
};
static const char* prefix_golden =
    "pre &amp; untouched\n\n> before &amp; raw\n>\n"
    "> > take [a]\n> >\n> > [a]: /first \"one\n> > two\"\n> >\n> > also [a]\n"
    ">\n> after &amp; raw\n\npost &amp; untouched\n";
static void prefix_variant(const char* input, unsigned mode, char* out, size_t capacity)
{
    size_t i, at = 0; unsigned line = 0;
    if (mode == 2) { CHECK(capacity > 3); memcpy(out, "\xef\xbb\xbf", 3); at = 3; }
    for (i = 0; input[i]; i++) {
        CHECK(at + 2 < capacity);
        if (input[i] == '\n' && (mode == 1 || (mode == 2 && !(line++ & 1)))) out[at++] = '\r';
        out[at++] = input[i];
    }
    out[at] = 0;
}
static xui_doc_range_t prefix_selection(xui_document d, unsigned reverse, unsigned gaps)
{
    xui_doc_range_t left = find_text(d, "take"), right = find_text(d, "also"), range;
    xui_document_snapshot s;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    range.tAnchor = gaps & 1 ? block_gap(s, left.tAnchor, 0) : left.tAnchor;
    range.tCaret = gaps & 2 ? block_gap(s, right.tCaret, 1) : right.tCaret;
    if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
    xuiDocumentSnapshotRelease(s); return range;
}
static uint64_t prefix_paragraph(xui_document_snapshot s, xui_doc_position_t position)
{
    xui_doc_node_info_t n = {0}; n.iSize = sizeof(n);
    CHECK(xuiDocumentSnapshotGetNode(s, position.iNodeId, &n) == XUI_OK);
    if (n.iKind == XUI_DOC_TEXT) return n.iParentId;
    CHECK(n.iKind == XUI_DOC_PARAGRAPH); return n.iId;
}
static void prefix_outside_equal(const char* original, const char* output)
{
    const char* left = strstr(original, "take"); const char* right = strstr(original, "after &amp; raw");
    CHECK(left && right);
    while (left > original && left[-1] != '\n' && left[-1] != '\r') left--;
    while (right > original && right[-1] != '\n' && right[-1] != '\r') right--;
    CHECK(!memcmp(original, output, (size_t)(left - original)) && strlen(output) >= strlen(right) &&
        !strcmp(output + strlen(output) - strlen(right), right));
}
static void prefix_reference_records(xui_document_snapshot before, xui_document_snapshot after)
{
    xui_doc_source_info_t a = {0}, b = {0}; uint64_t i; a.iSize = b.iSize = sizeof(a);
    CHECK(xuiDocumentSnapshotGetSourceInfo(before, &a) == XUI_OK &&
        xuiDocumentSnapshotGetSourceInfo(after, &b) == XUI_OK &&
        a.iReferenceDefinitionCount && a.iReferenceDefinitionCount == b.iReferenceDefinitionCount);
    for (i = 0; i < a.iReferenceDefinitionCount; i++) {
        xui_doc_reference_definition_t x = {0}, y = {0}; x.iSize = y.iSize = sizeof(x);
        CHECK(xuiDocumentSnapshotGetReferenceDefinition(before, i, &x) == XUI_OK &&
            xuiDocumentSnapshotGetReferenceDefinition(after, i, &y) == XUI_OK && x.iKind == y.iKind);
        if (x.iKind == XUI_DOC_REFERENCE_FOOTNOTE) {
            char u[2048], v[2048]; uint64_t n = x.iSourceEnd - x.iSourceStart;
            CHECK(n == y.iSourceEnd - y.iSourceStart && n < sizeof(u) &&
                xuiDocumentSnapshotReadSource(before, x.iSourceStart, u, n) == XUI_OK &&
                xuiDocumentSnapshotReadSource(after, y.iSourceStart, v, n) == XUI_OK && !memcmp(u, v, (size_t)n));
        }
    }
}
static void prefix_matrix(void)
{
    size_t ci; unsigned reverse, gaps, ending;
    for (ci = 0; ci < sizeof(prefix_cases) / sizeof(prefix_cases[0]); ci++)
    for (reverse = 0; reverse < 2; reverse++)
    for (gaps = 0; gaps < 4; gaps++)
    for (ending = 0; ending < 3; ending++) {
        char original[8192], output[8192], restored[8192]; uint64_t quote, revision, left, right;
        xui_document d, reload; xui_document_transaction t;
        xui_document_snapshot before, after, full; xui_doc_range_t range, selected;
        int result;
        prefix_variant(prefix_cases[ci].source, ending, original, sizeof(original));
        d = open_md(original); range = prefix_selection(d, reverse, gaps);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK);
        left = prefix_paragraph(before, find_text(d, "take").tAnchor);
        right = prefix_paragraph(before, find_text(d, "also").tCaret);
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
        if (result != XUI_OK) fprintf(stderr, "prefix case %zu/%u/%u/%u: %d\n", ci, reverse, gaps, ending, result);
        CHECK(result == XUI_OK); xuiDocumentTxnRelease(t);
        copy_source(d, output, sizeof(output)); CHECK(!strcmp(output, original) &&
            xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, output, sizeof(output));
        CHECK(below(after, left, quote) && below(after, right, quote) && count_tasks(before, 1) == count_tasks(after, 1));
        same_tree(before, left, after, left); same_tree(before, right, after, right);
        prefix_outside_equal(original, output); prefix_reference_records(before, after);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(before, after);
#endif
        if (!ci && !ending) CHECK(!strcmp(output, prefix_golden));
        if (getenv("XUI_QUOTE_PREFIX_DUMP") && !reverse && !gaps && !ending) printf("case %zu:\n%s\n", ci, output);
        reload = open_md(output); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(after, full);
#endif
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, restored, sizeof(restored));
        CHECK(!strcmp(original, restored) && xuiDocumentAcquireSnapshot(d, &full) == XUI_OK);
        same_tree(before, 1, full, 1); same_syntax(before, full); xuiDocumentSnapshotRelease(full);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); copy_source(d, restored, sizeof(restored));
        CHECK(!strcmp(output, restored) && xuiDocumentAcquireSnapshot(d, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full); xuiDocumentSnapshotRelease(full);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    }
    puts("Quote prefix values: 240 common-quote/list-item/all-visible-child cases, forward/reverse TEXT/GAP LF/CRLF/BOM; raw outer source, full reload, shared history");
}
static void prefix_failures(void)
{
    size_t ci; long total = 0;
    for (ci = 0; ci < sizeof(prefix_cases) / sizeof(prefix_cases[0]); ci++) {
        long point; int success = 0;
        for (point = 0; point < 4000 && !success; point++) {
            failing_allocator a = {-1, 0}; xui_doc_desc_t desc = {0};
            xui_document d; xui_document_transaction t = NULL;
            xui_document_snapshot before, after; xui_doc_range_t range, selected;
            uint64_t quote = 0, revision; unsigned live; int result; char output[8192];
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = fail_alloc; desc.onFree = fail_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, prefix_cases[ci].source, strlen(prefix_cases[ci].source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = prefix_selection(d, (unsigned)(ci & 1), (unsigned)(ci % 4));
            revision = xuiDocumentGetRevision(d); live = a.live; a.remaining = point;
            result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (result == XUI_OK) result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            xuiDocumentTxnRelease(t); a.remaining = -1;
            if (result != XUI_OK) CHECK(result == XUI_ERROR_OUT_OF_MEMORY && a.live == live &&
                xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result == XUI_OK) { success = 1; CHECK(quote && xuiDocumentGetRevision(d) == revision + 1); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); }
            else { same_tree(before, 1, after, 1); same_syntax(before, after); }
            copy_source(d, output, sizeof(output)); CHECK(!strcmp(output, prefix_cases[ci].source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success); total += point - 1;
        printf("Quote prefix allocation sweep %zu: %ld failure points, old source/tree/history/live count exact, zero leaks\n", ci, point - 1);
    }
    printf("Quote prefix allocation failure points total: %ld\n", total);
}
#ifndef XUI_DLL
typedef struct prefix_cancel_allocator {
    failing_allocator allocation;
    long remaining;
    atomic_int cancellation;
} prefix_cancel_allocator;
static void* prefix_cancel_alloc(void* user, size_t bytes)
{
    prefix_cancel_allocator* a = user;
    if (a->remaining == 0) atomic_store(&a->cancellation, XUI_DOC_ERROR_CANCELLED);
    else if (a->remaining > 0) a->remaining--;
    return fail_alloc(&a->allocation, bytes);
}
static void prefix_cancel_free(void* user, void* pointer)
{
    prefix_cancel_allocator* a = user; fail_free(&a->allocation, pointer);
}
static void prefix_cancellation(void)
{
    const unsigned cases[] = {0, 3, 4, 7, 8}; size_t ci; long total = 0;
    for (ci = 0; ci < sizeof(cases) / sizeof(cases[0]); ci++) {
        unsigned sample = cases[ci]; long point; int success = 0;
        for (point = 0; point < 4000 && !success; point++) {
            prefix_cancel_allocator a = {{-1,0}, -1, ATOMIC_VAR_INIT(0)};
            xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t;
            xui_document_snapshot before, after; xui_doc_range_t range, selected;
            uint64_t quote, revision; unsigned live; int result; char output[8192];
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = prefix_cancel_alloc; desc.onFree = prefix_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, prefix_cases[sample].source, strlen(prefix_cases[sample].source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = prefix_selection(d, sample & 1, sample % 4); revision = xuiDocumentGetRevision(d); live = a.allocation.live;
            CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); t->cancellation = &a.cancellation;
            a.remaining = point;
            result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            a.remaining = -1; xuiDocumentTxnRelease(t); atomic_store(&a.cancellation, 0);
            if (result != XUI_OK) CHECK(result == XUI_DOC_ERROR_CANCELLED &&
                a.allocation.live == live && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (result == XUI_OK) { success = 1; CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); }
            else { same_tree(before, 1, after, 1); same_syntax(before, after); }
            copy_source(d, output, sizeof(output)); CHECK(!strcmp(output, prefix_cases[sample].source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.allocation.live);
        }
        CHECK(success); total += point - 1;
        printf("Quote prefix cancellation sweep %u: %ld checkpoints, atomic published state, zero leaks\n", sample, point - 1);
    }
    printf("Quote prefix cancellation checkpoints total: %ld\n", total);
}
#endif
static void prefix_depth_and_compound(void)
{
    const unsigned depths[] = {1, 2, 8, 32, 64, 124};
    const char* lines[] = {"before &amp; raw", "", "take [a]", "",
        "[a]: /first \"one", "two\"", "", "also [a]", "", "after &amp; raw"};
    size_t di; unsigned reverse;
    for (di = 0; di < sizeof(depths) / sizeof(depths[0]); di++)
    for (reverse = 0; reverse < 2; reverse++) {
        char original[32768], output[32768]; size_t at = 0, line; unsigned i;
        xui_document d, reload; xui_document_transaction t;
        xui_document_snapshot before, after, full; xui_doc_range_t range, selected; uint64_t quote;
        at += (size_t)sprintf(original + at, "pre &amp; untouched\n\n");
        for (line = 0; line < sizeof(lines) / sizeof(lines[0]); line++) {
            for (i = 0; i < depths[di]; i++) { original[at++] = '>'; original[at++] = ' '; }
            at += (size_t)sprintf(original + at, "%s\n", lines[line]);
        }
        at += (size_t)sprintf(original + at, "\npost &amp; untouched\n");
        CHECK(at < sizeof(original)); d = open_md(original); range = prefix_selection(d, reverse, 3);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, output, sizeof(output)); prefix_outside_equal(original, output);
        prefix_reference_records(before, after); reload = open_md(output);
        CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(before, after); reference_cache_snapshot_equal(after, full);
#endif
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, output, sizeof(output)); CHECK(!strcmp(output, original));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    }
    for (di = 0; di < sizeof(prefix_cases) / sizeof(prefix_cases[0]); di++) {
        xui_document d = open_md(prefix_cases[di].source), reload; xui_document_transaction t;
        xui_document_snapshot before, after, full; xui_doc_range_t range = prefix_selection(d, di & 1, di % 4), selected;
        char output[8192], restored[8192]; uint64_t quote, revision = xuiDocumentGetRevision(d); unsigned layer;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        for (layer = 0; layer < 5; layer++) {
            int result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
            if (result != XUI_OK) fprintf(stderr, "compound prefix %zu layer %u: %d\n", di, layer, result);
            CHECK(result == XUI_OK); range = selected;
            copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, prefix_cases[di].source) &&
                xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
        }
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentGetRevision(d) == revision + 1 &&
            xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, output, sizeof(output)); prefix_outside_equal(prefix_cases[di].source, output);
        prefix_reference_records(before, after); reload = open_md(output);
        CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK); same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(before, after); reference_cache_snapshot_equal(after, full);
#endif
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d));
        copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, prefix_cases[di].source));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, output));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    }
    for (di = 0; di < sizeof(prefix_cases) / sizeof(prefix_cases[0]); di++) {
        xui_document md = open_md(prefix_cases[di].source), rich; xui_document_snapshot source, before, after;
        xui_doc_rich_conversion_report_t report = {0}; xui_document_transaction t;
        xui_doc_range_t range, selected; unsigned layer; uint64_t quote, revision, left, right;
        report.iSize = sizeof(report);
        CHECK(xuiDocumentAcquireSnapshot(md, &source) == XUI_OK &&
            xuiDocumentSnapshotAnalyzeRichConversion(source, &report) == XUI_OK &&
            xuiDocumentSnapshotConvertToRich(source, report.iReasons, &rich) == XUI_OK);
        xuiDocumentSnapshotRelease(source); xuiDocumentRelease(md);
        range = prefix_selection(rich, di & 1, di % 4); revision = xuiDocumentGetRevision(rich);
        CHECK(xuiDocumentClearHistory(rich) == XUI_OK && xuiDocumentAcquireSnapshot(rich, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(rich, NULL, &t) == XUI_OK);
        left = prefix_paragraph(before, find_text(rich, "take").tAnchor);
        right = prefix_paragraph(before, find_text(rich, "also").tCaret);
        for (layer = 0; layer < 5; layer++) {
            CHECK(xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK); range = selected;
            CHECK(xuiDocumentGetRevision(rich) == revision && !xuiDocumentCanUndo(rich));
        }
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentGetRevision(rich) == revision + 1 &&
            xuiDocumentAcquireSnapshot(rich, &after) == XUI_OK && below(after, left, quote) && below(after, right, quote));
        xuiDocumentTxnRelease(t); same_tree(before, left, after, left); same_tree(before, right, after, right);
        xuiDocumentSnapshotRelease(after); CHECK(xuiDocumentUndo(rich, NULL) == XUI_OK && !xuiDocumentCanUndo(rich) &&
            xuiDocumentAcquireSnapshot(rich, &after) == XUI_OK); same_tree(before, 1, after, 1);
        xuiDocumentSnapshotRelease(after); xuiDocumentSnapshotRelease(before); xuiDocumentRelease(rich);
    }
    puts("Quote prefix depth: 12 cases through 124 layers; compound: 10 Markdown + 10 Rich five-wrap transactions, one publish/Undo");
}
#ifdef XUI_QUOTE_TEST_EDITOR
static void prefix_editor(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    size_t ci; unsigned reverse, gaps;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    for (ci = 0; ci < sizeof(prefix_cases) / sizeof(prefix_cases[0]); ci++)
    for (reverse = 0; reverse < 2; reverse++)
    for (gaps = 0; gaps < 4; gaps++) {
        xui_document d = open_md(prefix_cases[ci].source); xui_doc_editor_desc_t desc = {0}; xui_widget editor;
        xui_doc_range_t range = prefix_selection(d, reverse, gaps), selected;
        xui_doc_command_state_t state = {0}; xui_document_snapshot before, after, full; xui_document reload;
        char output[8192], restored[8192]; uint64_t revision;
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d; desc.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0,0,640,480}) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        revision = xuiDocumentGetRevision(d); state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && state.bEnabled &&
            !state.bActive && !state.iDisabledReason && xuiDocumentEditorCanExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) &&
            xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
        copy_source(d, output, sizeof(output)); CHECK(!strcmp(output, prefix_cases[ci].source));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK &&
            xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &selected) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
            xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        copy_source(d, output, sizeof(output)); prefix_outside_equal(prefix_cases[ci].source, output);
        prefix_reference_records(before, after); reload = open_md(output);
        CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK); same_tree(after, 1, full, 1); same_syntax(after, full);
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
            xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK &&
            !state.bEnabled && state.iDisabledReason == XUI_ERROR_UNSUPPORTED &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_ERROR_UNSUPPORTED &&
            xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK && xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
        copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, output));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && !xuiDocumentCanUndo(d));
        copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, prefix_cases[ci].source));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, output));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Quote prefix editor: 80 actual DLL Query/CanExecute/Execute, full reload, SOURCE/LIVE/VISUAL shared history cases");
}
#endif
#ifndef XUI_QUOTE_PREFIX_NO_MAIN
int main(void)
{
    prefix_matrix(); prefix_depth_and_compound(); prefix_failures();
#ifndef XUI_DLL
    prefix_cancellation();
#endif
#ifdef XUI_QUOTE_TEST_EDITOR
    prefix_editor();
#endif
    return 0;
}
#endif
