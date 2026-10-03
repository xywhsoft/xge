#define main quote_nested_main
#include "xui_document_quote_nested_test.c"
#undef main

typedef struct source_quote_case {
    const char* source;
    const char* left;
    const char* right;
} source_quote_case;
static const source_quote_case source_cases[] = {
    { "pre &amp; untouched\n\n> > left &amp; raw\n> >\n> > take [a]\n> >\n> > [a]: <https://one.test/a?b=&amp;> 'raw title'\n>\n> tail [a]\n\noutside [a]\n\npost &amp; untouched\n", "take", "outside" },
    { "pre &amp; untouched\n\n> > left &amp; raw\n> >\n> > take [a]\n> >\n> > [a]: /first \"line one\n> > line two\"\n>\n> tail [a]\n\nmid [a]\n\n> before\n>\n> > also [a]\n> >\n> > [a]: /unused 'duplicate'\n> >\n> > right &amp; raw\n\npost &amp; untouched\n", "take", "also" },
    { "pre &amp; untouched\n\n> > left &amp; raw\n> >\n> > [a]: /first\n> >\n> > take [a]\n>\n> tail [a]\n\noutside [a]\n\n[a]: /second 'unused duplicate'\n\npost &amp; untouched\n", "take", "outside" },
    { "pre &amp; untouched\n\n> left &amp; raw\n>\n> take [a]\n>\n> [a]: /first\n>\n> also [a]\n>\n> right &amp; raw\n\npost &amp; untouched\n", "take", "also" },
    { "pre &amp; untouched\n\ntake [a]\n\n[a]: /first 'title'\n\n> > also [a]\n> >\n> > right &amp; raw\n\npost &amp; untouched\n", "take", "also" },
    { "pre &amp; untouched\n\n> left &amp; raw\n>\n> take\n>\n> [unused]: /url 'unused'\n>\n> - [x] full item [unused]\n> - last\n\noutside [unused]\n\npost &amp; untouched\n", "take", "outside" },
    { "pre &amp; untouched\n\n> > left &amp; raw\n> >\n> > take [a]\n> >\n> > [a]:\n> >   <https://one.test/a>\n> >   'raw title'\n> >\n> > ~~~~ lang opaque &amp; info\n> > literal <&> \\raw\n> > ~~~~\n>\n> | raw &amp; | escaped \\| pipe |\n> | :-- | --: |\n> | `code|pipe` | cell [a] |\n\noutside [a]\n\npost &amp; untouched\n", "take", "outside" },
    { "pre &amp; untouched\n\n> > left &amp; raw\n> >\n> > take [a]\n> >\n> > [a]: /first 'title'\n> >\n> > lazy &amp; raw\nlazy continuation\n>\n> ## heading &amp; raw ##\n\noutside [a]\n\npost &amp; untouched\n", "take", "outside" },
    { "pre &amp; untouched\n\n> > left &amp; raw\n> >\n> > take [a]\n> >\n> > [a]: /first 'title'\n> >\n> > [Unused\\!]: /opaque 'untouched'\n>\n> - item [a]\n> - second\n\noutside [a]\n\n[^unused]: opaque &amp; body\n\npost &amp; untouched\n", "take", "outside" }
};
static const char* source_gold_first =
    "pre &amp; untouched\n\n> > left &amp; raw\n> >\n\n"
    "> > > take [a]\n> > >\n> > > [a]: <https://one.test/a?b=&amp;> 'raw title'\n"
    "> >\n> > tail [a]\n> \n> outside [a]\n\n\npost &amp; untouched\n";
static const char* source_gold_multiline =
    "pre &amp; untouched\n\n> > left &amp; raw\n> >\n\n"
    "> > > take [a]\n> > >\n> > > [a]: /first \"line one\n> > > line two\"\n"
    "> >\n> > tail [a]\n> \n> mid [a]\n> \n> > before\n> >\n> > > also [a]\n\n"
    "> >\n> > [a]: /unused 'duplicate'\n> >\n> > right &amp; raw\n\npost &amp; untouched\n";
static void variant_source(const char* source, unsigned endings, char* out, size_t capacity)
{
    size_t i, at = 0; unsigned line = 0;
    if (endings == 2) { CHECK(capacity > 3); memcpy(out, "\xef\xbb\xbf", 3); at = 3; }
    for (i = 0; source[i]; i++) {
        CHECK(at + 2 < capacity);
        if (source[i] == '\n' && (endings == 1 || (endings == 2 && !(line++ & 1))))
            out[at++] = '\r';
        out[at++] = source[i];
    }
    out[at] = 0;
}
static xui_doc_range_t source_selection(xui_document d, size_t i, unsigned reverse, unsigned gaps)
{
    xui_doc_range_t left = find_text(d, source_cases[i].left), right = find_text(d, source_cases[i].right), range;
    xui_document_snapshot s;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    range.tAnchor = gaps & 1 ? block_gap(s, left.tAnchor, 0) : left.tAnchor;
    range.tCaret = gaps & 2 ? block_gap(s, right.tCaret, 1) : right.tCaret;
    if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
    xuiDocumentSnapshotRelease(s); return range;
}
static void source_outside_equal(const char* original, const char* output)
{
    const char* take = strstr(original, "take");
    const char* line = take; const char* post = strstr(original, "post &amp; untouched");
    CHECK(take && post);
    while (line > original && line[-1] != '\n' && line[-1] != '\r') line--;
    CHECK(!memcmp(original, output, (size_t)(line - original)) &&
        strlen(output) >= strlen(post) && !strcmp(output + strlen(output) - strlen(post), post));
}
static int copy_source_range(xui_document_snapshot s, uint64_t start, uint64_t end,
    char* out, uint64_t capacity, uint64_t* bytes)
{
    if (end < start || end - start >= capacity) return XUI_ERROR_INVALID_ARGUMENT;
    *bytes = end - start; return xuiDocumentSnapshotReadSource(s, start, out, *bytes);
}
static void source_references(xui_document_snapshot before, xui_document_snapshot after)
{
    xui_doc_source_info_t a = {0}, b = {0}; uint64_t i, n, m; char x[2048], y[2048];
    a.iSize = b.iSize = sizeof(a);
    CHECK(xuiDocumentSnapshotGetSourceInfo(before, &a) == XUI_OK &&
        xuiDocumentSnapshotGetSourceInfo(after, &b) == XUI_OK &&
        a.iReferenceDefinitionCount && a.iReferenceDefinitionCount == b.iReferenceDefinitionCount);
    for (i = 0; i < a.iReferenceDefinitionCount; i++) {
        xui_doc_reference_definition_t u = {0}, v = {0}; u.iSize = v.iSize = sizeof(u);
        CHECK(xuiDocumentSnapshotGetReferenceDefinition(before, i, &u) == XUI_OK &&
            xuiDocumentSnapshotGetReferenceDefinition(after, i, &v) == XUI_OK && u.iKind == v.iKind);
        CHECK(copy_source_range(before, u.iLabelStart, u.iLabelEnd, x, sizeof(x), &n) == XUI_OK &&
            copy_source_range(after, v.iLabelStart, v.iLabelEnd, y, sizeof(y), &m) == XUI_OK &&
            n == m && !memcmp(x, y, (size_t)n));
        if (u.iKind == XUI_DOC_REFERENCE_FOOTNOTE) {
            CHECK(copy_source_range(before, u.iSourceStart, u.iSourceEnd, x, sizeof(x), &n) == XUI_OK &&
                copy_source_range(after, v.iSourceStart, v.iSourceEnd, y, sizeof(y), &m) == XUI_OK &&
                n == m && !memcmp(x, y, (size_t)n));
        }
    }
}
static void source_cases_run(void)
{
    size_t i; unsigned reverse, gaps, endings;
    for (i = 0; i < sizeof(source_cases) / sizeof(source_cases[0]); i++)
    for (reverse = 0; reverse < 2; reverse++)
    for (gaps = 0; gaps < 4; gaps++)
    for (endings = 0; endings < 3; endings++) {
        char original[8192], output[8192], restored[8192]; uint64_t revision;
        xui_document d, reload; xui_document_snapshot before, after, full;
        xui_document_transaction t; xui_doc_node_id id; xui_doc_range_t selected;
        xui_doc_range_t range, left, right; int result;
        variant_source(source_cases[i].source, endings, original, sizeof(original));
        d = open_md(original); range = source_selection(d, i, reverse, gaps);
        left = find_text(d, source_cases[i].left); right = find_text(d, source_cases[i].right);
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnWrapQuoteRange(t, &range, &id, &selected) == XUI_OK);
        xuiDocumentTxnRelease(t);
        copy_source(d, output, sizeof(output)); CHECK(!strcmp(original, output) &&
            xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        result = xuiDocumentTxnWrapQuoteRange(t, &range, &id, &selected);
        if (result != XUI_OK) fprintf(stderr, "source quote case %zu/%u/%u/%u: %d\n", i, reverse, gaps, endings, result);
        CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, output, sizeof(output));
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK &&
            below(after, left.tAnchor.iNodeId, id) && below(after, right.tCaret.iNodeId, id) &&
            count_tasks(before, 1) == count_tasks(after, 1));
        source_outside_equal(original, output); source_references(before, after);
        if (!endings && i == 0) CHECK(!strcmp(output, source_gold_first));
        if (!endings && i == 1) CHECK(!strcmp(output, source_gold_multiline));
        if (getenv("XUI_QUOTE_SOURCE_DUMP") && !reverse && !gaps && !endings) printf("case %zu:\n%s\n", i, output);
        reload = open_md(output); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full);
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, restored, sizeof(restored));
        CHECK(!strcmp(original, restored) && xuiDocumentAcquireSnapshot(d, &full) == XUI_OK);
        same_tree(before, 1, full, 1); same_syntax(before, full); xuiDocumentSnapshotRelease(full);
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); copy_source(d, restored, sizeof(restored));
        CHECK(!strcmp(output, restored) && xuiDocumentAcquireSnapshot(d, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full); xuiDocumentSnapshotRelease(full);
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after);
        xuiDocumentRelease(d);
    }
    puts("Quote source: 216 forward/reverse TEXT/GAP LF/CRLF/BOM cases, definitions, abort, full reload, exact Undo/Redo");
}
static void source_allocation_failures(void)
{
    size_t ci; long total = 0;
    for (ci = 0; ci < sizeof(source_cases) / sizeof(source_cases[0]); ci++) {
        long point; int success = 0;
        for (point = 0; point < 4000 && !success; point++) {
            failing_allocator a = {-1, 0}; xui_doc_desc_t desc = {0};
            xui_document d; xui_document_transaction t = NULL;
            xui_document_snapshot before, after; xui_doc_range_t range, selected;
            uint64_t quote = 0, revision; int result; char output[8192];
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = fail_alloc; desc.onFree = fail_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, source_cases[ci].source, strlen(source_cases[ci].source)) == XUI_OK &&
                xuiDocumentClearHistory(d) == XUI_OK &&
                xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
            range = source_selection(d, ci, (unsigned)(ci & 1), (unsigned)(ci % 4));
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
            copy_source(d, output, sizeof(output)); CHECK(!strcmp(output, source_cases[ci].source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after);
            xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success); total += point - 1;
        printf("Quote source allocation sweep %zu: %ld failure points, atomic history/source/tree, no leaks\n", ci, point - 1);
    }
    printf("Quote source allocation failure points total: %ld\n", total);
}
static void source_depth_cases(void)
{
    static const unsigned depths[] = {1, 2, 8, 32, 64, 124};
    size_t ci; unsigned reverse;
    for (ci = 0; ci < sizeof(depths) / sizeof(depths[0]); ci++)
    for (reverse = 0; reverse < 2; reverse++) {
        char original[8192] = "pre &amp; untouched\n\n", output[16384], restored[8192];
        const char* lines[] = {"left &amp; raw", "", "take [a]", "", "[a]: /first \"line one", "line two\""};
        size_t at = strlen(original), line; unsigned level;
        xui_document d, reload; xui_document_transaction t;
        xui_document_snapshot before, after, full; xui_doc_range_t range, selected;
        uint64_t quote;
        for (line = 0; line < sizeof(lines) / sizeof(lines[0]); line++) {
            for (level = 0; level < depths[ci]; level++) { original[at++] = '>'; original[at++] = ' '; }
            at += (size_t)sprintf(original + at, "%s\n", lines[line]);
        }
        strcpy(original + at, "\noutside [a]\n\npost &amp; untouched\n");
        d = open_md(original); range = find_text(d, "take"); range.tCaret = find_text(d, "outside").tCaret;
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK &&
            xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, output, sizeof(output));
        source_outside_equal(original, output);
        CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); source_references(before, after);
        reload = open_md(output); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
        same_tree(after, 1, full, 1); same_syntax(after, full);
        xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(original, restored));
        CHECK(xuiDocumentRedo(d, NULL) == XUI_OK); copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(output, restored));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    }
    puts("Quote source definitions: 12 depth cases, 1/2/8/32/64/124 quotes, multiline title, full reload, exact history");
}
static void source_bom_eof_cases(void)
{
    unsigned bom, eol;
    const char* source = "take [a]\n\n> outside [a]\n>\n> [a]: /first 'title'";
    const char* expected = "> take [a]\n> \n> > outside [a]\n> >\n> > [a]: /first 'title'";
    for (bom = 0; bom < 2; bom++)
    for (eol = 0; eol < 2; eol++) {
        char original[1024], output[1024], gold[1024];
        xui_document d, reload; xui_document_transaction t;
        xui_document_snapshot a, b; xui_doc_range_t range, selected; uint64_t id;
        variant_source(source, eol, original + (bom ? 3 : 0), sizeof(original) - 3);
        variant_source(expected, eol, gold + (bom ? 3 : 0), sizeof(gold) - 3);
        if (bom) { memcpy(original, "\xef\xbb\xbf", 3); memcpy(gold, "\xef\xbb\xbf", 3); }
        d = open_md(original); range = find_text(d, "take"); range.tCaret = find_text(d, "outside").tCaret;
        CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnWrapQuoteRange(t, &range, &id, &selected) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, output, sizeof(output)); CHECK(!strcmp(output, gold));
        reload = open_md(output); CHECK(xuiDocumentAcquireSnapshot(d, &a) == XUI_OK &&
            xuiDocumentAcquireSnapshot(reload, &b) == XUI_OK);
        same_tree(a, 1, b, 1); same_syntax(a, b);
        xuiDocumentSnapshotRelease(a); xuiDocumentSnapshotRelease(b); xuiDocumentRelease(reload);
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, output, sizeof(output)); CHECK(!strcmp(original, output));
        xuiDocumentRelease(d);
    }
    puts("Quote source BOM/EOF: exact four golden LF/CRLF files, terminal definition without newline");
}
static void source_hidden_quote_case(void)
{
    const char* original = "pre\n\n> [a]: /first 'unused'\n\npost\n";
    const char* gold = "pre\n\n\n> > [a]: /first 'unused'\n\n\npost\n";
    xui_document d = open_md(original), reload; xui_document_snapshot s, after, full;
    xui_document_transaction t; xui_doc_range_t range = find_text(d, "pre"), selected;
    xui_doc_node_info_t root = {0}, node = {0}; uint64_t i, child, quote; char output[1024];
    root.iSize = node.iSize = sizeof(root);
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK && xuiDocumentSnapshotGetNode(s, 1, &root) == XUI_OK);
    for (i = 0; i < root.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(s, 1, i, &child) == XUI_OK &&
            xuiDocumentSnapshotGetNode(s, child, &node) == XUI_OK);
        if (node.iKind == XUI_DOC_QUOTE) break;
    }
    CHECK(i < root.iChildCount && !node.iChildCount);
    range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tAnchor.iNodeId = 1; range.tAnchor.iOffset = i;
    range.tCaret = range.tAnchor; range.tCaret.iOffset++;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
        xuiDocumentTxnWrapQuoteRange(t, &range, &quote, &selected) == XUI_OK &&
        xuiDocumentTxnCommit(t, NULL) == XUI_OK);
    xuiDocumentTxnRelease(t); copy_source(d, output, sizeof(output)); CHECK(!strcmp(output, gold));
    CHECK(xuiDocumentAcquireSnapshot(d, &after) == XUI_OK); source_references(s, after);
    reload = open_md(output); CHECK(xuiDocumentAcquireSnapshot(reload, &full) == XUI_OK);
    same_tree(after, 1, full, 1); same_syntax(after, full);
    xuiDocumentSnapshotRelease(full); xuiDocumentRelease(reload);
    CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, output, sizeof(output)); CHECK(!strcmp(original, output));
    xuiDocumentSnapshotRelease(s); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d);
    puts("Quote source hidden-only container: no visible child, full definition retained and exact Undo");
}
#ifdef XUI_QUOTE_TEST_EDITOR
static void source_editor_cases(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    size_t ci; unsigned reverse, gaps;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    for (ci = 0; ci < sizeof(source_cases) / sizeof(source_cases[0]); ci++)
    for (reverse = 0; reverse < 2; reverse++)
    for (gaps = 0; gaps < 4; gaps++) {
        xui_document d = open_md(source_cases[ci].source);
        xui_doc_editor_desc_t desc = {0}; xui_widget editor;
        xui_doc_range_t range = source_selection(d, ci, reverse, gaps), after;
        xui_doc_command_state_t state = {0}; uint64_t revision;
        char output[8192], restored[8192];
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
        desc.tView.pDocument = d; desc.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 480}) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        revision = xuiDocumentGetRevision(d); state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK &&
            state.bEnabled && !state.iDisabledReason &&
            xuiDocumentEditorCanExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) &&
            xuiDocumentGetRevision(d) == revision);
        copy_source(d, output, sizeof(output)); CHECK(!strcmp(output, source_cases[ci].source));
        /* The toolbar command toggles an already active quote; the explicit
         * Wrap API adds another level for a selection within that quote. */
        CHECK((state.bActive ? xuiDocumentEditorWrapQuote(editor) :
                xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE)) == XUI_OK &&
            xuiDocumentViewGetSelection(editor, &after) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &after) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK);
        copy_source(d, output, sizeof(output)); source_outside_equal(source_cases[ci].source, output);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
            xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
            xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
        copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, output));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, source_cases[ci].source));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        copy_source(d, restored, sizeof(restored)); CHECK(!strcmp(restored, output));
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Quote source editor: 72 Query/CanExecute/Wrap/Execute cases, valid selection, three views, shared Undo/Redo");
}
#endif
int main(void)
{
    source_cases_run(); source_depth_cases(); source_bom_eof_cases(); source_hidden_quote_case(); source_allocation_failures();
#ifdef XUI_QUOTE_TEST_EDITOR
    source_editor_cases();
#endif
    puts("Document quote source tests passed.");
    return 0;
}
