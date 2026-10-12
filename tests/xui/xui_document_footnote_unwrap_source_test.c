#define XUI_QUOTE_BOUNDARY_NO_MAIN
#include "xui_document_quote_boundary_test.c"

static const unwrap_source_case inner_cases[] = {
    {"use[^n]\n\n[^n]:\n    > take &amp; raw\n    >\n    > ~~~~ lang opaque &amp; info\n    > code\\raw\n    > ~~~~\n\noutside &amp; raw\n",
     "use[^n]\n\n[^n]:\n    take &amp; raw\n    \n    ~~~~ lang opaque &amp; info\n    code\\raw\n    ~~~~\n\noutside &amp; raw\n"},
    {"use[^n]\n\n[^n]: > take &amp; raw\n    > continuation &amp; raw\n\noutside &amp; raw\n",
     "use[^n]\n\n[^n]: take &amp; raw\n    continuation &amp; raw\n\noutside &amp; raw\n"},
    {"use[^n]\n\n[^n]:\n    > [!nOtE]\n    > take &amp; raw\n    >\n    > ~~~~ lang opaque &amp; info\n    > code\\raw\n    > ~~~~\n\noutside &amp; raw\n",
     "use[^n]\n\n[^n]:\n    take &amp; raw\n    \n    ~~~~ lang opaque &amp; info\n    code\\raw\n    ~~~~\n\noutside &amp; raw\n"},
    {"use[^n]\n\n[^n]: > [!TIP]\n    > take &amp; raw\n    > continuation &amp; raw\n\noutside &amp; raw\n",
     "use[^n]\n\n[^n]: take &amp; raw\n    continuation &amp; raw\n\noutside &amp; raw\n"},
    {"use[^n]\n\n[^n]: before &amp; raw\n    > take &amp; raw\n    >\n    > <div>raw &amp;</div>\n    after &amp; raw\n\noutside &amp; raw\n",
     "use[^n]\n\n[^n]: before &amp; raw\n    \n    take &amp; raw\n    \n    <div>raw &amp;</div>\n    \n    after &amp; raw\n\noutside &amp; raw\n"},
    {"use[^n]\n\n[^n]:\n    - > [!warning]\n      > take &amp; raw\n    - last &amp; raw\n\noutside &amp; raw\n",
     "use[^n]\n\n[^n]:\n    - take &amp; raw\n    - last &amp; raw\n\noutside &amp; raw\n"},
    {"use[^n]\n\n[^n]:\n    > before &amp; raw\n    > > take &amp; raw\n    >\n    > after &amp; raw\n\noutside &amp; raw\n",
     "use[^n]\n\n[^n]:\n    > before &amp; raw\n    > \n    > take &amp; raw\n    >\n    > after &amp; raw\n\noutside &amp; raw\n"},
    {"> use[^n]\n>\n> [^n]:\n>     > take &amp; raw\n>     > continuation &amp; raw\n\noutside &amp; raw\n",
     "> use[^n]\n>\n> [^n]:\n>     take &amp; raw\n>     continuation &amp; raw\n\noutside &amp; raw\n"},
    {"use[^n]\n\n[^n]:\n\t> take &amp; raw\n\t>\n\t> ~~~~ lang opaque\n\t>\tcode\t<&> raw\n\t> ~~~~\n\noutside &amp; raw\n",
     "use[^n]\n\n[^n]:\n\ttake &amp; raw\n\t\n\t~~~~ lang opaque\n\t  code\t<&> raw\n\t~~~~\n\noutside &amp; raw\n"},
    {"use[^n] and [a]\n\n[^n]:\n    > take &amp; raw\n\n[^n]: duplicate &amp; raw\n    ~~~~ lang opaque\n    duplicate\\raw\n    ~~~~\n\n[^unused]: unused &amp; raw\n\n[a]: /outside 'raw &amp;'\n",
     "use[^n] and [a]\n\n[^n]:\n    take &amp; raw\n\n[^n]: duplicate &amp; raw\n    ~~~~ lang opaque\n    duplicate\\raw\n    ~~~~\n\n[^unused]: unused &amp; raw\n\n[a]: /outside 'raw &amp;'\n"},
    {"use[^n]\n\n[^n]:\n    > take [a]\n    >\n    > [a]: /inside 'one\n    > two &amp;'\n    >\n    > | a | b |\n    > | :-- | --: |\n    > | [a] | &amp; |\n\noutside &amp; raw\n",
     "use[^n]\n\n[^n]:\n    take [a]\n    \n    [a]: /inside 'one\n    two &amp;'\n    \n    | a | b |\n    | :-- | --: |\n    | [a] | &amp; |\n\noutside &amp; raw\n"},
    {"use[^n]\n\n[^n]:\n    > [!CaUtIoN]\n    > take 中文 🧡 &amp; raw",
     "use[^n]\n\n[^n]:\n    take 中文 🧡 &amp; raw"}
};

static xui_doc_node_info_t inner_owner(xui_document_snapshot before, uint64_t id)
{
    xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
    do { CHECK(xuiDocumentSnapshotGetNode(before, id, &node) == XUI_OK); id = node.iParentId; }
    while (node.iKind != XUI_DOC_FOOTNOTE && id);
    CHECK(node.iKind == XUI_DOC_FOOTNOTE); return node;
}
static uint64_t inner_patch_proof(const char* source, const char* output,
    xui_document_snapshot before, xui_document_change_set change, const xui_doc_block_syntax_t* header)
{
    xui_doc_change_info_t info = {0}; uint64_t i, at, last = UINT64_MAX, removed = 0, added = 0, count = 0;
    info.iSize = sizeof(info); CHECK(xuiDocumentChangeSetGetInfo(change, &info) == XUI_OK);
    for (i = 0; i < info.iOperationCount; i++) {
        const xui_doc_operation_t* op = &info.pOperations[i];
        if (op->iKind != XUI_DOC_OP_SOURCE) continue;
        CHECK(op->iOffset <= last && op->iOldLength <= 64 && op->iNewLength <= 128);
        for (at = op->iOffset; at < op->iOffset + op->iOldLength; at++)
            CHECK((header->iSecondaryEnd > header->iSecondaryStart && at >= header->iSecondaryStart && at < header->iSecondaryEnd) ||
                source[at] == '>' || source[at] == ' ' || source[at] == '\t' || source[at] == '\r' || source[at] == '\n');
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
#ifndef XUI_DLL
static void inner_reference_sharing(xui_document_snapshot before, xui_document_snapshot after,
    const xui_doc_node_info_t* owner)
{
    uint64_t i, count = doc_seq_size(before->state->reference_values), edited = 0;
    CHECK(count == doc_seq_size(after->state->reference_values));
    for (i = 0; i < count; i++) {
        xui_doc_reference_definition_t a, b;
        CHECK(doc_seq_read(before->state->references, i * sizeof(a), &a, sizeof(a)) == XUI_OK &&
            doc_seq_read(after->state->references, i * sizeof(b), &b, sizeof(b)) == XUI_OK && a.iKind == b.iKind);
        if (a.iKind == XUI_DOC_REFERENCE_FOOTNOTE && a.iSourceStart == owner->iSyntaxStart && a.iSourceEnd == owner->iSyntaxEnd) {
            CHECK(doc_seq_get_value_item(before->state->reference_values, i) != doc_seq_get_value_item(after->state->reference_values, i));
            edited++;
        } else CHECK(doc_seq_get_value_item(before->state->reference_values, i) == doc_seq_get_value_item(after->state->reference_values, i));
    }
    CHECK(edited == 1);
}
#endif
static void inner_matrix(void)
{
    unsigned ci, mode, gap, affinity, count = 0;
    for (ci = 0; ci < sizeof(inner_cases) / sizeof(inner_cases[0]); ci++)
    for (mode = 0; mode < 3; mode++)
    for (gap = 0; gap < 3; gap++)
    for (affinity = 0; affinity < 2; affinity++) {
        char source[8192], expected[8192], actual[8192]; xui_document d, reload;
        xui_document_snapshot before, after, full; xui_document_transaction t; xui_document_change_set change;
        xui_doc_range_t found; xui_doc_position_t at, caret; xui_doc_node_info_t quote, owner; xui_doc_block_syntax_t header = {0};
        uint64_t i, removed; int result;
        boundary_variant(inner_cases[ci].source, mode, source, sizeof(source));
        boundary_variant(inner_cases[ci].expected, mode, expected, sizeof(expected));
        d = open_md(source); found = find_text(d, "take");
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        at = unwrap_position(before, found.tAnchor, gap); at.iAffinity = affinity; if (!gap) at.iOffset += 2;
        quote = unwrap_quote_node(before, at); owner = inner_owner(before, quote.iParentId); header.iSize = sizeof(header);
        CHECK(xuiDocumentSnapshotGetBlockSyntax(before, quote.iId, &header) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        result = xuiDocumentTxnUnwrapQuote(t, &at, &caret);
        if (result) fprintf(stderr, "inner-footnote %u/%u/%u/%u: %d\n", ci, mode, gap, affinity, result);
        CHECK(result == XUI_OK && xuiDocumentTxnCommit(t, &change) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, sizeof(actual));
        if (strcmp(actual, expected)) fprintf(stderr, "inner-footnote %u/%u/%u/%u source:\n%s\nEXPECTED\n%s\n", ci, mode, gap, affinity, actual, expected);
        CHECK(!strcmp(actual, expected)); removed = inner_patch_proof(source, actual, before, change, &header);
        xuiDocumentChangeSetRelease(change);
        for (i = 0; i < quote.iChildCount; i++) {
            uint64_t child; xui_doc_node_info_t node = {0}; node.iSize = sizeof(node);
            CHECK(xuiDocumentSnapshotGetChild(before, quote.iId, i, &child) == XUI_OK &&
                xuiDocumentSnapshotGetNode(after, child, &node) == XUI_OK && node.iParentId == quote.iParentId);
            same_tree(before, child, after, child); boundary_child_ids(before, after, child);
        }
#ifndef XUI_DLL
        CHECK(unwrap_shared_bytes(before->state->source, after->state->source) == strlen(source) - removed);
        inner_reference_sharing(before, after, &owner);
#else
        (void)removed; (void)owner;
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
    printf("Footnote unwrap source: %u exact TEXT/GAP/affinity/EOL cases, owned header, stable children, source maps and untouched definition sharing\n", count);
}
static void inner_failures(void)
{
    const unsigned samples[] = {0, 3, 4, 7, 9, 10}; unsigned sample; long total = 0;
    for (sample = 0; sample < sizeof(samples) / sizeof(samples[0]); sample++) {
        unsigned ci = samples[sample]; long point; int success = 0;
        for (point = 0; point < 10000 && !success; point++) {
            failing_allocator a = {-1, 0}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_transaction t = NULL; xui_document_snapshot before, after; xui_doc_range_t found;
            xui_doc_position_t caret; char actual[8192], source[8192], expected[8192]; unsigned live; uint64_t revision; int result;
            strcpy(source, inner_cases[ci].source); strcpy(expected, inner_cases[ci].expected);
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = fail_alloc; desc.onFree = fail_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
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
                success = 1; copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            } else { same_tree(before, 1, after, 1); same_syntax(before, after); }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.live);
        }
        CHECK(success); total += point - 1; printf("Footnote unwrap source OOM %u: %ld points, all failed attempts unpublished and no leaks\n", ci, point - 1);
    }
    printf("Footnote unwrap source OOM total: %ld\n", total);
}
#ifndef XUI_DLL
static void inner_cancellation(void)
{
    const unsigned samples[] = {0, 3, 4, 7, 9, 10}; unsigned sample; long total = 0;
    for (sample = 0; sample < sizeof(samples) / sizeof(samples[0]); sample++) {
        unsigned ci = samples[sample]; long point; int success = 0;
        for (point = 0; point < 10000 && !success; point++) {
            prefix_cancel_allocator a = {{-1, 0}, -1, ATOMIC_VAR_INIT(0)}; xui_doc_desc_t desc = {0}; xui_document d;
            xui_document_transaction t; xui_document_snapshot before, after; xui_doc_range_t found;
            xui_doc_position_t caret; char actual[8192], source[8192], expected[8192]; unsigned live; uint64_t revision; int result;
            strcpy(source, inner_cases[ci].source); strcpy(expected, inner_cases[ci].expected);
            desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
            desc.onAlloc = prefix_cancel_alloc; desc.onFree = prefix_cancel_free; desc.pAllocatorUser = &a;
            CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
                xuiDocumentLoadMarkdown(d, source, strlen(source)) == XUI_OK &&
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
                success = 1; copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
                CHECK(xuiDocumentUndo(d, NULL) == XUI_OK);
            } else { same_tree(before, 1, after, 1); same_syntax(before, after); reference_cache_snapshot_equal(before, after); }
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
            xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(d); CHECK(!a.allocation.live);
        }
        CHECK(success); total += point - 1; printf("Footnote unwrap source cancellation %u: %ld checkpoints, atomic state and no leaks\n", ci, point - 1);
    }
    printf("Footnote unwrap source cancellation total: %ld\n", total);
}
#endif

#ifdef XUI_QUOTE_TEST_EDITOR
static void inner_editor(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    unsigned ci, gap, mode, affinity, total = 0, refused = 0;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    for (ci = 0; ci < sizeof(inner_cases) / sizeof(inner_cases[0]); ci++)
    for (gap = 0; gap < 3; gap++)
    for (mode = 0; mode < 3; mode++)
    for (affinity = 0; affinity < 2; affinity++) {
        char raw_source[8192], raw_expected[8192]; xui_document d;
        strcpy(raw_source, inner_cases[ci].source); strcpy(raw_expected, inner_cases[ci].expected); d = open_md(raw_source); xui_document_snapshot before;
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
            copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, raw_source)); refused++;
            CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        }
        state.iSize = sizeof(state); CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && state.bEnabled && state.bActive);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK && xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &selected) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, raw_expected));
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
            xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, raw_source));
        CHECK(!xuiDocumentCanUndo(d) && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, raw_expected));
        xuiWidgetDestroy(editor); xuiDocumentRelease(d); total++;
    }
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    printf("Footnote unwrap source Editor: %u actual DLL VISUAL Query/Execute/Caret cases, %u LIVE/SOURCE atomic refusals and three-mode history\n", total, refused);
}
#endif
static void inner_shift_and_native(void)
{
    unsigned ci;
    for (ci = 0; ci < sizeof(inner_cases) / sizeof(inner_cases[0]); ci++) {
        const char* prefix = "prefix &amp; raw\n\n"; char source[8192], expected[8192], shifted[8192], actual[8192];
        xui_document d, loaded; xui_document_snapshot before, after, full; xui_document_transaction t;
        xui_doc_txn_desc_t td = {0}; xui_doc_range_t found; xui_doc_position_t caret; xui_doc_node_info_t quote, owner, node = {0};
        char* data; uint64_t bytes;
        boundary_variant(inner_cases[ci].source, 1, source, sizeof(source));
        boundary_variant(inner_cases[ci].expected, 1, expected, sizeof(expected));
        d = open_md(source); found = find_text(d, "take");
        CHECK(xuiDocumentAcquireSnapshot(d, &before) == XUI_OK);
        quote = unwrap_quote_node(before, found.tAnchor); owner = inner_owner(before, quote.iParentId);
        td.iSize = sizeof(td); td.iDomain = XUI_DOC_SOURCE;
        CHECK(xuiDocumentBeginTransaction(d, &td, &t) == XUI_OK &&
            xuiDocumentTxnReplaceSource(t, 0, 0, prefix, strlen(prefix)) == XUI_OK &&
            xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentAcquireSnapshot(d, &after) == XUI_OK);
        xuiDocumentTxnRelease(t); node.iSize = sizeof(node);
        CHECK(xuiDocumentSnapshotGetNode(after, owner.iId, &node) == XUI_OK &&
            node.iSyntaxStart == owner.iSyntaxStart + strlen(prefix) && node.iSyntaxEnd == owner.iSyntaxEnd + strlen(prefix));
        same_tree(before, quote.iId, after, quote.iId); boundary_child_ids(before, after, quote.iId);
        strcpy(shifted, prefix); strcat(shifted, source); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, shifted));
        CHECK(xuiDocumentSerialize(after, &data, &bytes) == XUI_OK && xuiDocumentDeserialize(NULL, data, bytes, &loaded) == XUI_OK &&
            xuiDocumentAcquireSnapshot(loaded, &full) == XUI_OK); xuiDocumentFreeBuffer(data);
        same_tree(after, 1, full, 1); same_syntax(after, full);
#ifndef XUI_DLL
        reference_cache_snapshot_equal(after, full);
#endif
        xuiDocumentSnapshotRelease(full); found = find_text(loaded, "take");
        CHECK(xuiDocumentBeginTransaction(loaded, NULL, &t) == XUI_OK &&
            xuiDocumentTxnUnwrapQuote(t, &found.tAnchor, &caret) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t); strcpy(actual, prefix); strcat(actual, expected); strcpy(expected, actual);
        copy_source(loaded, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        CHECK(xuiDocumentUndo(loaded, NULL) == XUI_OK && !xuiDocumentCanUndo(loaded));
        copy_source(loaded, actual, sizeof(actual)); CHECK(!strcmp(actual, shifted));
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        xuiDocumentSnapshotRelease(before); xuiDocumentSnapshotRelease(after); xuiDocumentRelease(loaded); xuiDocumentRelease(d);
    }
    puts("Footnote unwrap metadata: 12 incremental source shifts, stable old snapshots and native roundtrip edits passed");
}
static void inner_depth_and_compound(void)
{
    const unsigned depths[] = {0, 1, 8, 32, 64, 123}; unsigned di;
    for (di = 0; di < sizeof(depths) / sizeof(depths[0]); di++) {
        char source[8192], expected[8192], actual[8192]; size_t a, b; unsigned layer;
        xui_document d; xui_document_transaction t; xui_doc_range_t found; xui_doc_position_t caret;
        strcpy(source, "use[^n]\n\n[^n]:\n    "); a = strlen(source);
        for (layer = 0; layer <= depths[di]; layer++) a += (size_t)sprintf(source + a, "> ");
        a += (size_t)sprintf(source + a, "[!Important]\n    ");
        for (layer = 0; layer <= depths[di]; layer++) a += (size_t)sprintf(source + a, "> ");
        a += (size_t)sprintf(source + a, "take &amp; raw\n"); CHECK(a < sizeof(source));
        strcpy(expected, "use[^n]\n\n[^n]:\n    "); b = strlen(expected);
        for (layer = 0; layer < depths[di]; layer++) b += (size_t)sprintf(expected + b, "> ");
        b += (size_t)sprintf(expected + b, "take &amp; raw\n"); CHECK(b < sizeof(expected));
        d = open_md(source); found = find_text(d, "take");
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnUnwrapQuote(t, &found.tAnchor, &caret) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d));
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source)); xuiDocumentRelease(d);
    }
    {
        const char* source = "use[^n]\n\n[^n]:\n    > [!nOtE]\n    > > [!TIP]\n    > > take &amp; raw\n";
        const char* expected = "use[^n]\n\n[^n]:\n    take &amp; raw\n";
        xui_document d = open_md(source); xui_document_transaction t; xui_doc_range_t found = find_text(d, "take");
        xui_doc_position_t caret; char actual[8192]; uint64_t revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        CHECK(xuiDocumentTxnUnwrapQuote(t, &found.tAnchor, &caret) == XUI_OK && xuiDocumentTxnUnwrapQuote(t, &caret, &caret) == XUI_OK &&
            xuiDocumentGetRevision(d) == revision && !xuiDocumentCanUndo(d));
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source));
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK && xuiDocumentGetRevision(d) == revision + 1);
        xuiDocumentTxnRelease(t); copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, expected));
        CHECK(xuiDocumentUndo(d, NULL) == XUI_OK && !xuiDocumentCanUndo(d));
        copy_source(d, actual, sizeof(actual)); CHECK(!strcmp(actual, source)); xuiDocumentRelease(d);
    }
    puts("Footnote unwrap: 0/1/8/32/64/123 parent quote depths and a two-header unpublished transaction with single Undo passed");
}
int main(void)
{
    inner_matrix(); inner_shift_and_native(); inner_depth_and_compound(); inner_failures();
#ifndef XUI_DLL
    inner_cancellation();
#endif
#ifdef XUI_QUOTE_TEST_EDITOR
    inner_editor();
#endif
    return 0;
}
