#define XUI_QUOTE_PREFIX_NO_MAIN
#include "xui_document_quote_prefix_test.c"

typedef struct foot_prefix_case {
    unsigned container;
    const char* middle;
    const char* use;
    int eof;
} foot_prefix_case;
static const foot_prefix_case foot_cases[] = {
    {1, "take\n\n[^unused]: foot &amp; body\n\nalso\n", "", 0},
    {1, "take\n\n[^unused]: **raw** \\*literal\\*  \n    second &amp; line\n    lazy continuation\n\nalso\n", "", 0},
    {1, "take\n\n[^n]: first &amp; body\n\n[^n]: unused duplicate\n\n[^Other]: unused case\n\nalso\n", " [^n]", 0},
    {1, "take\n\n[^n]: used **body** [r]\n\n[r]: /raw 'first\nsecond'\n\nalso\n", " [^n]", 0},
    {1, "take\n\n[^unused]: intro\n\n    - one\n    - two\n\n    ~~~~c opaque\n    code <&> raw\n    ~~~~\n\n    > nested\n\nalso\n", "", 0},
    {2, "take\n\n[^unused]:\n        code &amp; raw\n\nalso\n", "", 0},
    {3, "take\n\n[^unused]: list body\n    continuation\n\nalso\n", "", 0},
    {4, "take\n\n[^unused]: task body\n    continuation\n\nalso\n", "", 0},
    {0, "take\n\n[^unused]: root body\n    continuation\n\nalso\n", "", 0},
    {0, "take\n\n[^n]: root used body\n\n[^unused]: root hidden body\n\nalso\n", " [^n]", 0},
    {1, "take\n\n[^unused]: blank\n\nalso", "", 1},
    {0, "take\n\n[^unused]: blank\n\nalso", "", 1}
};
static const char* foot_prefixes[] = {"", "> ", "> > ", "  ", ">   "};
static void foot_append(char* output, size_t capacity, size_t* at, const char* text, size_t bytes)
{
    CHECK(bytes < capacity - *at); memcpy(output + *at, text, bytes); *at += bytes; output[*at] = 0;
}
/* Independent byte oracle: the only selected-line edit is one literal quote
 * prefix after the known outer prefix. Root wrapping also adds boundary EOLs. */
static void foot_source(unsigned sample, char* output, size_t capacity)
{
    const foot_prefix_case* c = &foot_cases[sample]; const char* prefix = foot_prefixes[c->container];
    static const char* before[] = {"before &amp; raw\n\n", "> before &amp; raw\n>\n",
        "> > before &amp; raw\n> >\n", "- before &amp; raw\n\n", "> - [x] before &amp; raw\n>\n"};
    static const char* after[] = {"\nafter &amp; raw\n\npost &amp; untouched\n", ">\n> after &amp; raw\n\npost &amp; untouched\n",
        "> >\n> > after &amp; raw\n\npost &amp; untouched\n", "\n  after &amp; raw\n- last\n\npost &amp; untouched\n",
        ">\n>   after &amp; raw\n> - last\n\npost &amp; untouched\n"};
    size_t at = 0; const char* line = c->middle;
    output[0] = 0; foot_append(output, capacity, &at, "pre &amp; untouched", strlen("pre &amp; untouched"));
    foot_append(output, capacity, &at, c->use, strlen(c->use)); foot_append(output, capacity, &at, "\n\n", 2);
    foot_append(output, capacity, &at, before[c->container], strlen(before[c->container]));
    while (*line) {
        const char* end = strchr(line, '\n'); size_t bytes = end ? (size_t)(end - line + 1) : strlen(line);
        foot_append(output, capacity, &at, prefix, strlen(prefix));
        foot_append(output, capacity, &at, line, bytes); line += bytes;
    }
    if (!c->eof) {
        foot_append(output, capacity, &at, after[c->container], strlen(after[c->container]));
    }
}
static void foot_equal_source(xui_document d, const char* expected)
{
    char actual[32768]; copy_source(d, actual, sizeof(actual));
    if (strcmp(actual, expected)) fprintf(stderr, "Footnote prefix source mismatch\nEXPECTED:\n%s\nACTUAL:\n%s\n", expected, actual);
    CHECK(!strcmp(actual, expected));
}
static void foot_expected(unsigned sample, const char* source, char* output, size_t capacity)
{
    const char* start = strstr(source, "take"); const char* end = strstr(source, "also");
    const char* line; const char* ending = "\n"; size_t at = 0, eol = 1, prefix = strlen(foot_prefixes[foot_cases[sample].container]);
    CHECK(start && end); while (start > source && start[-1] != '\n' && start[-1] != '\r') start--;
    while (*end && *end != '\n' && *end != '\r') end++;
    if (*end == '\r') end++;
    if (*end == '\n') end++;
    if (end > start && end[-1] == '\r') ending = "\r";
    else if (end - start >= 2 && end[-2] == '\r' && end[-1] == '\n') { ending = "\r\n"; eol = 2; }
    output[0] = 0; foot_append(output, capacity, &at, source, (size_t)(start - source));
    if (!foot_cases[sample].container) foot_append(output, capacity, &at, ending, eol);
    for (line = start; line < end; ) {
        const char* next = line; while (next < end && *next != '\r' && *next != '\n') next++;
        if (next < end && *next == '\r') next++;
        if (next < end && *next == '\n') next++;
        CHECK((size_t)(next - line) >= prefix && !memcmp(line, foot_prefixes[foot_cases[sample].container], prefix));
        foot_append(output, capacity, &at, line, prefix); foot_append(output, capacity, &at, "> ", 2);
        foot_append(output, capacity, &at, line + prefix, (size_t)(next - line) - prefix); line = next;
    }
    if (*end && !foot_cases[sample].container) foot_append(output, capacity, &at, ending, eol);
    foot_append(output, capacity, &at, end, strlen(end));
}
static void foot_matrix(void)
{
    unsigned ci, reverse, gaps, ending, total = 0;
    for (ci = 0; ci < sizeof(foot_cases) / sizeof(foot_cases[0]); ci++)
    for (reverse = 0; reverse < 2; reverse++)
    for (gaps = 0; gaps < 4; gaps++)
    for (ending = 0; ending < 3; ending++) {
        char source[32768], expected[32768], raw[8192]; xui_document d, reload;
        xui_document_snapshot before, after, full; xui_document_transaction t; xui_doc_range_t range, selected;
        uint64_t quote, revision; int result;
        foot_source(ci, raw, sizeof(raw)); prefix_variant(raw, ending, source, sizeof(source));
        foot_expected(ci, source, expected, sizeof(expected));
        d = open_md(source); range = prefix_selection(d, reverse, gaps); revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
        if (result) fprintf(stderr, "foot prefix %u/%u/%u/%u: %d\n", ci, reverse, gaps, ending, result);
        CHECK(result == XUI_OK); xuiDocumentTxnRelease(t); foot_equal_source(d, source);
        CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d) &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK &&
            xuiDocumentGetRevision(d) == revision + 1);
        xuiDocumentTxnRelease(t); foot_equal_source(d, expected); reload = open_md(expected);
        CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK); same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(before, after); reference_cache_snapshot_equal(after, full);
#endif
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &full) == XUI_OK);
        foot_equal_source(d, source); same_tree(before, 1, full, 1); same_syntax(before, full);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(before, full);
#endif
        xuiDocumentSnapshotRelease(full); CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); foot_equal_source(d, expected);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); total++;
    }
    printf("Footnote prefix: %u golden raw source / used-unused-duplicate / root-quote-list-task / reverse TEXT-GAP / BOM-EOL-EOF / Abort-Undo-Redo cases passed\n", total);
}
static void foot_failures(void)
{
    unsigned ci; long total = 0;
    for (ci = 0; ci < sizeof(foot_cases) / sizeof(foot_cases[0]); ci++) {
        long point; int succeeded = 0; char source[8192], expected[8192];
        foot_source(ci, source, sizeof(source)); foot_expected(ci, source, expected, sizeof(expected));
        for (point = 0; point < 8000 && !succeeded; point++) {
            failing_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t = NULL;
            xui_document_snapshot before, after; xui_doc_range_t range, selected; uint64_t revision, quote; unsigned live; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.onAlloc = fail_alloc; desc.onFree = fail_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = prefix_selection(d, ci & 1, ci % 4); revision = xuiDocumentGetRevision(d); live = a.live; a.remaining = point;
            result = xuiDocumentBeginTransaction(d, NULL, &t);
            if (result == XUI_OK) result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(t, NULL);
            xuiDocumentTxnRelease(t); a.remaining = -1;
            if (result) CHECK(result == XUI_ERROR_OUT_OF_MEMORY && a.live == live && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) { succeeded = 1; foot_equal_source(d, expected); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); }
            else {
                same_tree(before, 1, after, 1); same_syntax(before, after);
#ifndef XUI_DLL
                reference_cache_snapshot_equal(before, after);
#endif
            }
            foot_equal_source(d, source); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(succeeded); total += point - 1; printf("Footnote prefix OOM %u: %ld points passed\n", ci, point - 1);
    }
    printf("Footnote prefix OOM total: %ld points, successful retry and zero leaks\n", total);
}
static void foot_compound_and_depth(void)
{
    unsigned ci, rich, reverse, layer; const unsigned depths[] = {1, 8, 32, 64, 124};
    for (ci = 0; ci < sizeof(foot_cases) / sizeof(foot_cases[0]); ci++)
    for (rich = 0; rich < 2; rich++) {
        char source[8192], output[32768]; xui_document d, reload; xui_document_snapshot before, after, full;
        xui_document_transaction t; xui_doc_range_t range, selected; uint64_t quote, revision;
        foot_source(ci, source, sizeof(source)); d = open_md(source);
        if (rich) {
            xui_document converted; xui_doc_rich_conversion_report_t report = {0}; report.iSize = sizeof(report);
            CHECK(xuiDocumentAcquireSnapshot(d, &full) == XUI_OK && xuiDocumentSnapshotAnalyzeRichConversion(full, &report) == XUI_OK &&
                xuiDocumentSnapshotConvertToRich(full, report.iReasons, &converted) == XUI_OK);
            xuiDocumentSnapshotRelease(full); xuiDocumentRelease(d); d = converted;
        }
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        range = prefix_selection(d, ci & 1, ci % 4); revision = xuiDocumentGetRevision(d);
        for (layer = 0; layer < 5; layer++) {
            CHECK(xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK); range = selected;
            CHECK(xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            if (!rich) foot_equal_source(d, source);
        }
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t);
        if (!rich) {
            copy_source(d, output, sizeof(output)); reload = open_md(output); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
            same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
            reference_cache_snapshot_equal(before, after); reference_cache_snapshot_equal(after, full);
#endif
            xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        }
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d) && xuiDocumentAcquireSnapshot(d, &full) == XUI_OK);
        same_tree(before, 1, full, 1); xuiDocumentSnapshotRelease(full); if (!rich) foot_equal_source(d, source);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &full) == XUI_OK);
        same_tree(after, 1, full, 1); xuiDocumentSnapshotRelease(full); if (!rich) foot_equal_source(d, output);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    }
    for (ci = 0; ci < sizeof(depths) / sizeof(depths[0]); ci++)
    for (reverse = 0; reverse < 2; reverse++) {
        const char* lines[] = {"before &amp; raw", "", "take", "", "[^unused]: first", "    second &amp; body", "", "also", "", "after &amp; raw"};
        char source[32768], output[32768]; size_t at = 0, line; unsigned depth;
        xui_document d, reload; xui_document_transaction t; xui_document_snapshot before, after, full; xui_doc_range_t range, selected; uint64_t quote;
        foot_append(source, sizeof(source), &at, "pre &amp; untouched\n\n", strlen("pre &amp; untouched\n\n"));
        for (line = 0; line < sizeof(lines) / sizeof(lines[0]); line++) {
            for (depth = 0; depth < depths[ci]; depth++) foot_append(source, sizeof(source), &at, "> ", 2);
            foot_append(source, sizeof(source), &at, lines[line], strlen(lines[line])); foot_append(source, sizeof(source), &at, "\n", 1);
        }
        foot_append(source, sizeof(source), &at, "\npost &amp; untouched\n", strlen("\npost &amp; untouched\n"));
        d = open_md(source); range = prefix_selection(d, reverse, 3);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, output, sizeof(output)); reload = open_md(output);
        CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK); same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(before, after); reference_cache_snapshot_equal(after, full);
#endif
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); foot_equal_source(d, source);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload); xuiDocumentRelease(d);
    }
    puts("Footnote prefix: 12 Markdown + 12 Rich five-wrap compound transactions and 10 deep cases through 124 layers passed");
}
#ifndef XUI_DLL
static void foot_cancellation(void)
{
    const unsigned samples[] = {0, 2, 4, 7, 8}; unsigned ci; long total = 0;
    for (ci = 0; ci < sizeof(samples) / sizeof(samples[0]); ci++) {
        unsigned sample = samples[ci]; long point; int succeeded = 0; char source[8192], expected[8192];
        foot_source(sample, source, sizeof(source)); foot_expected(sample, source, expected, sizeof(expected));
        for (point = 0; point < 8000 && !succeeded; point++) {
            prefix_cancel_allocator a = {{-1,0}, -1, ATOMIC_VAR_INIT(0)}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_transaction t; xui_document_snapshot before, after; xui_doc_range_t range, selected;
            uint64_t revision, quote; unsigned live; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = prefix_cancel_alloc; desc.onFree = prefix_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = prefix_selection(d, sample & 1, sample % 4); revision = xuiDocumentGetRevision(d); live = a.allocation.live;
            CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); t->cancellation = &a.cancellation; a.remaining = point;
            result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            a.remaining = -1; xuiDocumentTxnRelease(t); atomic_store(&a.cancellation, 0);
            if (result) CHECK(result == XUI_DOC_ERROR_CANCELLED && a.allocation.live == live && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) { succeeded = 1; foot_equal_source(d, expected); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); }
            else { same_tree(before, 1, after, 1); same_syntax(before, after); reference_cache_snapshot_equal(before, after); }
            foot_equal_source(d, source); xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.allocation.live);
        }
        CHECK(succeeded); total += point - 1; printf("Footnote prefix cancellation %u: %ld checkpoints passed\n", sample, point - 1);
    }
    printf("Footnote prefix cancellation total: %ld checkpoints and zero leaks\n", total);
}
#endif
#ifdef XUI_QUOTE_TEST_EDITOR
static void foot_editor(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; unsigned ci, reverse, gaps, total = 0;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    for (ci = 0; ci < sizeof(foot_cases) / sizeof(foot_cases[0]); ci++)
    for (reverse = 0; reverse < 2; reverse++)
    for (gaps = 0; gaps < 4; gaps++) {
        char source[8192], expected[8192]; xui_document d, reload; xui_widget editor; xui_doc_editor_desc_t desc = {0};
        xui_doc_command_state_t state = {0}; xui_document_snapshot after, full; xui_doc_range_t range, selected;
        foot_source(ci, source, sizeof(source)); foot_expected(ci, source, expected, sizeof(expected)); d = open_md(source);
        range = prefix_selection(d, reverse, gaps); desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d; desc.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK && xuiWidgetSetRect(editor, (xui_rect_t){0,0,640,480}) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        state.iSize = sizeof(state); CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && state.bEnabled &&
            xuiDocumentEditorCanExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE)); foot_equal_source(d, source);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK && xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &selected) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        foot_equal_source(d, expected); reload = open_md(expected); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full); xuiDocumentSnapshotRelease(full); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(reload);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
            xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK); foot_equal_source(d, expected);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && !xuiDocumentCanUndo(d)); foot_equal_source(d, source);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK); foot_equal_source(d, expected);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); total++;
    }
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Footnote prefix editor: %u actual DLL Query-Execute-selection / full reload / three-mode shared history cases passed\n", total);
}
#endif
int main(void)
{
    foot_matrix(); foot_compound_and_depth(); foot_failures();
#ifndef XUI_DLL
    foot_cancellation();
#endif
#ifdef XUI_QUOTE_TEST_EDITOR
    foot_editor();
#endif
    return 0;
}
