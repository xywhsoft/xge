#define XUI_QUOTE_PREFIX_NO_MAIN
#include "xui_document_quote_prefix_test.c"

typedef struct tab_prefix_case { const char* source; const char* expected; } tab_prefix_case;
static const tab_prefix_case tab_cases[] = {
    {"pre &amp; untouched\n\n- before &amp; raw\n\n\ttake\n\n\talso\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n- before &amp; raw\n\n  >   take\n  >\n  >   also\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\n1. before &amp; raw\n\n\ttake\n\n\talso\n\n   after &amp; raw\n2. last\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n1. before &amp; raw\n\n   >  take\n   >\n   >  also\n\n   after &amp; raw\n2. last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\n100. before &amp; raw\n\n\t\ttake\n\n\t\talso\n\n     after &amp; raw\n101. last\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n100. before &amp; raw\n\n\t >    take\n\t >\n\t >    also\n\n     after &amp; raw\n101. last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\n> > - [x] before &amp; raw\n> >\n> > \ttake\n> >\n> > \talso\n> >\n> >   after &amp; raw\n> > - last\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n> > - [x] before &amp; raw\n> >\n> >   >   take\n> >   >\n> >   >   also\n> >\n> >   after &amp; raw\n> > - last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\n- before &amp; raw\n\n\ttake [a]\n\n\t[a]: /raw 'first\n\tsecond'\n\n\talso [a]\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n- before &amp; raw\n\n  >   take [a]\n  >\n  >   [a]: /raw 'first\n  >   second'\n  >\n  >   also [a]\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\n- before &amp; raw\n\n\ttake\n\n\t[^unused]: raw &amp; body\n\t\tcontinuation\n\n\talso\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n- before &amp; raw\n\n    >   take\n    >\n    >   [^unused]: raw &amp; body\n    >   \tcontinuation\n    >\n    >   also\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched [^n]\n\n- before &amp; raw\n\n\ttake\n\n\t[^n]: raw &amp; body\n\t\tcontinuation\n\n\talso\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n",
     "pre &amp; untouched [^n]\n\n- before &amp; raw\n\n    >   take\n    >\n    >   [^n]: raw &amp; body\n    >   \tcontinuation\n    >\n    >   also\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\n- before &amp; raw\n\n\ttake\n\n\t~~~ raw opaque\n\tcode\t<&>\n\t~~~\n\n\talso\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n- before &amp; raw\n\n  >   take\n  >\n  >   ~~~ raw opaque\n  >   code\t<&>\n  >   ~~~\n  >\n  >   also\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\n- take\n\n\talso\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n- > take\n  >\n  >   also\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\n- before &amp; raw\n\n\ttake\n\t\n\talso\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n- before &amp; raw\n\n  >   take\n  >   \n  >   also\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\n- before &amp; raw\n\n\ttake\n\n\t[^unused]:\n\t\t\tcode\t<&> raw\n\n\talso\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n- before &amp; raw\n\n    >   take\n    >\n    >   [^unused]:\n    >   \t\tcode\t<&> raw\n    >\n    >   also\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched [^n]\n\n- before &amp; raw\n\n\ttake\n\n\t[^n]:\n\t\t\tcode\t<&> raw\n\n\talso\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n",
     "pre &amp; untouched [^n]\n\n- before &amp; raw\n\n    >   take\n    >\n    >   [^n]:\n    >   \t\tcode\t<&> raw\n    >\n    >   also\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\n10. before &amp; raw\n\n\ttake\n\n\t\tcode\t<&> raw\n\n\talso\n\n    after &amp; raw\n11. last\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n10. before &amp; raw\n\n\t  > take\n\t  >\n\t  > \tcode\t<&> raw\n\t  >\n\t  > also\n\n    after &amp; raw\n11. last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\n1. outer &amp; raw\n   1. before &amp; raw\n\n\t\ttake\n\n\t\talso\n\n      after &amp; raw\n   2. last\n2. outer last\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n1. outer &amp; raw\n   1. before &amp; raw\n\n\t  >   take\n\t  >\n\t  >   also\n\n      after &amp; raw\n   2. last\n2. outer last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\n- before &amp; raw\n\n\ttake\nlazy continuation\n\n\talso\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n- before &amp; raw\n\n  >   take\n  > lazy continuation\n  >\n  >   also\n\n  after &amp; raw\n- last\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\nbefore &amp; raw\n\ntake\n\n[^unused]:\n\t\tcode\t<&> raw\n\nalso\n\nafter &amp; raw\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\nbefore &amp; raw\n\n\n  > take\n  > \n  > [^unused]:\n  > \t\tcode\t<&> raw\n  > \n  > also\n\n\nafter &amp; raw\n\npost &amp; untouched\n"},
    {"pre &amp; untouched\n\n> before &amp; raw\n>\n> take\n>\n> [^unused]:\n> \t\t\tcode\t<&> raw\n>\n> also\n>\n> after &amp; raw\n\npost &amp; untouched\n",
     "pre &amp; untouched\n\n> before &amp; raw\n>\n>   > take\n>   >\n>   > [^unused]:\n>   > \t\t\tcode\t<&> raw\n>   >\n>   > also\n>\n> after &amp; raw\n\npost &amp; untouched\n"}
};
/* The root fixture adds two boundary lines. Their EOL is the selected last
 * line's EOL; all original EOLs follow their original line ordinal. Applying
 * alternating EOLs to the final golden directly would change untouched EOLs. */
static void tab_expected_variant(unsigned ci, unsigned ending, char* output, size_t capacity)
{
    const char* input = tab_cases[ci].expected; unsigned line = 0; size_t at = 3, i;
    if (ci != 15 || ending != 2) { prefix_variant(input, ending, output, capacity); return; }
    CHECK(capacity > 3); memcpy(output, "\xef\xbb\xbf", 3);
    for (i = 0; input[i]; i++) {
        CHECK(at + 2 < capacity);
        if (input[i] == '\n') {
            unsigned original = line == 4 || line == 11 ? 9 : line < 4 ? line : line < 11 ? line - 1 : line - 2;
            if (!(original & 1)) output[at++] = '\r';
            line++;
        }
        output[at++] = input[i];
    }
    output[at] = 0;
}
static void tab_matrix(void)
{
    unsigned ci, reverse, gaps, ending, total = 0;
    for (ci = 0; ci < sizeof(tab_cases) / sizeof(tab_cases[0]); ci++)
    for (reverse = 0; reverse < 2; reverse++)
    for (gaps = 0; gaps < 4; gaps++)
    for (ending = 0; ending < 3; ending++) {
        char source[8192], expected[8192], actual[8192]; xui_document d, reload;
        xui_document_snapshot before, after, full; xui_document_transaction t;
        xui_doc_range_t range, selected; uint64_t quote, revision; int result;
        prefix_variant(tab_cases[ci].source, ending, source, sizeof(source));
        tab_expected_variant(ci, ending, expected, sizeof(expected));
        d = open_md(source); range = prefix_selection(d, reverse, gaps); revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
        if (result) fprintf(stderr, "Tab prefix %u/%u/%u/%u: %d\n", ci, reverse, gaps, ending, result);
        CHECK(result == XUI_OK); xuiDocumentTxnRelease(t);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source) && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, sizeof(actual));
        if (strcmp(actual, expected)) fprintf(stderr, "Tab golden %u/%u/%u/%u\nEXPECTED:\n%s\nACTUAL:\n%s\n", ci, reverse, gaps, ending, expected, actual);
        CHECK(!strcmp(actual, expected)); reload = open_md(expected); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(before, after); reference_cache_snapshot_equal(after, full);
#endif
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); total++;
    }
    printf("Tab prefix: %u literal golden, split columns, ordered/task/quoted lists, hidden definitions, code, BOM-EOL, Abort-Undo-Redo passed\n", total);
}
static void tab_compound(void)
{
    unsigned ci, step; const unsigned depths[] = {1, 8, 32, 64, 123, 124};
    for (ci = 0; ci < sizeof(tab_cases) / sizeof(tab_cases[0]); ci++) {
        xui_document d = open_md(tab_cases[ci].source), reload; xui_document_transaction t;
        xui_document_snapshot before, after, full; xui_doc_range_t range = prefix_selection(d, ci & 1, ci % 4), selected;
        char source[32768]; uint64_t quote;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        for (step = 0; step < 5; step++) {
            int result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
            if (result) fprintf(stderr, "Tab compound %u step %u: %d\n", ci, step, result);
            CHECK(result == XUI_OK); range = selected;
        }
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, source, sizeof(source)); prefix_outside_equal(tab_cases[ci].source, source);
        reload = open_md(source); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(before, after); reference_cache_snapshot_equal(after, full);
#endif
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d)); copy_source(d, source, sizeof(source)); CHECK(!strcmp(source, tab_cases[ci].source));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    }
    for (ci = 0; ci < sizeof(depths) / sizeof(depths[0]); ci++) {
        xui_document d = open_md(tab_cases[11].source); xui_document_transaction t; xui_document_snapshot before, after;
        xui_doc_range_t range = prefix_selection(d, ci & 1, ci % 4), selected; uint64_t quote, revision = xuiDocumentGetRevision(d); int limited = 0;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        for (step = 0; step < depths[ci]; step++) {
            int result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
            if (depths[ci] == 124 && step == 123) { CHECK(result == XUI_DOC_ERROR_LIMIT); limited = 1; break; }
            if (result) fprintf(stderr, "Tab depth %u step %u: %d\n", depths[ci], step, result);
            CHECK(result == XUI_OK); range = selected;
        }
        if (limited) {
            char source[8192]; xuiDocumentTxnRelease(t); copy_source(d, source, sizeof(source));
            CHECK(!strcmp(source, tab_cases[11].source) && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            xuiDocumentSnapshotRelease(before); xuiDocumentRelease(d); continue;
        }
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t);
        {
            char source[32768]; xui_document reload; xui_document_snapshot full;
            copy_source(d, source, sizeof(source)); reload = open_md(source);
            CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
            same_tree(after, 1, full, 1); same_syntax(after, full);
            xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        }
#ifndef XUI_DLL
        reference_cache_snapshot_equal(before, after);
#endif
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    }
    puts("Tab prefix: 17 five-wrap transactions; 1/8/32/64/123 layers preserve definitions; 124-layer footnote nesting rejects at schema depth atomically");
}
static void tab_failures(void)
{
    unsigned test; long total = 0; const unsigned compound[] = {0, 5, 10, 13, 16};
    for (test = 0; test < sizeof(tab_cases) / sizeof(tab_cases[0]) + sizeof(compound) / sizeof(compound[0]); test++) {
        unsigned ci = test < sizeof(tab_cases) / sizeof(tab_cases[0]) ? test : compound[test - sizeof(tab_cases) / sizeof(tab_cases[0])];
        unsigned steps = test < sizeof(tab_cases) / sizeof(tab_cases[0]) ? 1 : 2;
        long point; int success = 0;
        for (point = 0; point < 10000 && !success; point++) {
            failing_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d; xui_document_transaction t = NULL;
            xui_document_snapshot before, after; xui_doc_range_t range, selected; uint64_t quote, revision; unsigned live, step;
            char actual[8192]; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.onAlloc = fail_alloc; desc.onFree = fail_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, tab_cases[ci].source, strlen(tab_cases[ci].source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = prefix_selection(d, ci & 1, ci % 4); revision = xuiDocumentGetRevision(d); live = a.live; a.remaining = point;
            result = xuiDocumentBeginTransaction(d, NULL, &t);
            for (step = 0; step < steps && result == XUI_OK; step++) {
                result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
                if (result == XUI_OK) range = selected;
            }
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            xuiDocumentTxnRelease(t); a.remaining = -1;
            if (result) CHECK(result == XUI_ERROR_OUT_OF_MEMORY && a.live == live && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) {
                success = 1; copy_source(d, actual, sizeof(actual));
                if (steps == 1) CHECK(!strcmp(actual, tab_cases[ci].expected));
                prefix_outside_equal(tab_cases[ci].source, actual);
#ifndef XUI_DLL
                reference_cache_snapshot_equal(before, after);
#endif
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d));
            } else {
                same_tree(before, 1, after, 1); same_syntax(before, after);
#ifndef XUI_DLL
                reference_cache_snapshot_equal(before, after);
#endif
            }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, tab_cases[ci].source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success); total += point - 1; printf("Tab prefix OOM %u (%u wraps): %ld points passed\n", ci, steps, point - 1);
    }
    printf("Tab prefix OOM total: %ld points, 17 single and 5 compound sweeps, successful retry and zero leaks\n", total);
}
#ifndef XUI_DLL
static void tab_cancellation(void)
{
    const unsigned samples[] = {0, 5, 10, 13, 15, 16}; unsigned sample; long total = 0;
    for (sample = 0; sample < sizeof(samples) / sizeof(samples[0]); sample++) {
        unsigned ci = samples[sample]; long point; int success = 0;
        for (point = 0; point < 10000 && !success; point++) {
            prefix_cancel_allocator a = {{-1,0}, -1, ATOMIC_VAR_INIT(0)}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_transaction t; xui_document_snapshot before, after; xui_doc_range_t range, selected;
            uint64_t quote, revision; unsigned live; char actual[8192]; int result;
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.onAlloc = prefix_cancel_alloc; desc.onFree = prefix_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, tab_cases[ci].source, strlen(tab_cases[ci].source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = prefix_selection(d, ci & 1, ci % 4); revision = xuiDocumentGetRevision(d); live = a.allocation.live;
            CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK); t->cancellation = &a.cancellation; a.remaining = point;
            result = xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected);
            if (!result) result = xuiDocumentTxnCommit(t, NULL);
            a.remaining = -1; xuiDocumentTxnRelease(t); atomic_store(&a.cancellation, 0);
            if (result) CHECK(result == XUI_DOC_ERROR_CANCELLED && a.allocation.live == live && xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
            CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
            if (!result) { success = 1; copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, tab_cases[ci].expected)); CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); }
            else { same_tree(before, 1, after, 1); same_syntax(before, after); reference_cache_snapshot_equal(before, after); }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, tab_cases[ci].source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.allocation.live);
        }
        CHECK(success); total += point - 1; printf("Tab prefix cancellation %u: %ld checkpoints passed\n", ci, point - 1);
    }
    printf("Tab prefix cancellation total: %ld checkpoints, successful retry and zero leaks\n", total);
}
#endif
#ifdef XUI_QUOTE_TEST_EDITOR
static void tab_editor(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; unsigned ci, reverse, gaps, total = 0;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    for (ci = 0; ci < sizeof(tab_cases) / sizeof(tab_cases[0]); ci++)
    for (reverse = 0; reverse < 2; reverse++)
    for (gaps = 0; gaps < 4; gaps++) {
        xui_document d = open_md(tab_cases[ci].source), reload; xui_widget editor; xui_doc_editor_desc_t desc = {0};
        xui_doc_command_state_t state = {0}; xui_document_snapshot after, full; xui_doc_range_t range = prefix_selection(d, reverse, gaps), selected; char actual[8192];
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d; desc.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK && xuiWidgetSetRect(editor, (xui_rect_t){0,0,640,480}) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        state.iSize = sizeof(state); CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && state.bEnabled &&
            xuiDocumentEditorCanExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK && xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &selected) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, tab_cases[ci].expected)); reload = open_md(actual);
        CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK); same_tree(after, 1, full, 1); same_syntax(after, full);
        xuiDocumentSnapshotRelease(full); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(reload);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
            xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && !xuiDocumentCanUndo(d));
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, tab_cases[ci].source));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, tab_cases[ci].expected));
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); total++;
    }
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Tab prefix Editor: %u actual DLL Query-Execute-selection, full reload, three modes and shared history passed\n", total);
}
#endif
#ifndef XUI_TAB_PREFIX_NO_MAIN
int main(void)
{
    tab_matrix(); tab_compound(); tab_failures();
#ifndef XUI_DLL
    tab_cancellation();
#endif
#ifdef XUI_QUOTE_TEST_EDITOR
    tab_editor();
#endif
    return 0;
}
#endif
