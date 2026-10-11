#include "test_xui/xui_document_deep_quote_samples.h"
#include "xui_document_ui.h"
#include "test_xui/xui_test_proxy.h"

static void scaffold_editor_source(xui_document document, const char* source)
{
    xui_document_snapshot snapshot;
    TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    table_source(snapshot, source);
    xuiDocumentSnapshotRelease(snapshot);
}

static char* scaffold_editor_typed(const char* source)
{
    const char* at = strstr(source, "le");
    size_t bytes = strlen(source), offset;
    TABLE_CHECK(at);
    offset = (size_t)(at - source) + 2;
    char* typed = malloc(bytes + 2);
    TABLE_CHECK(typed);
    memcpy(typed, source, offset); typed[offset] = 'Z';
    memcpy(typed + offset + 1, source + offset, bytes - offset + 1);
    return typed;
}

static int scaffold_editor_external(xui_document document, const xui_doc_range_t* range,
    unsigned compound, xui_document_change_set* changes)
{
    xui_document_transaction transaction;
    xui_doc_position_t caret;
    int result = xuiDocumentBeginTransaction(document, NULL, &transaction);
    TABLE_CHECK(result == XUI_OK);
    result = xuiDocumentTxnReplaceRange(transaction, range, "", 0, &caret);
    if (result == XUI_OK && compound) {
        xui_doc_range_t insert = {caret, caret};
        result = xuiDocumentTxnReplaceRange(transaction, &insert, "Z", 1, &caret);
    }
    if (result == XUI_OK) result = xuiDocumentTxnCommit(transaction, changes);
    xuiDocumentTxnRelease(transaction);
    return result;
}

int main(void)
{
    xui_test_proxy_state_t proxy;
    xui_context context;
    xui_font font;
    const uint32_t modes[] = {XUI_DOC_SOURCE_TEXT, XUI_DOC_LIVE_MARKDOWN, XUI_DOC_VISUAL};
    const unsigned depths[] = {63, 64, 95, 120};
    const char* endings[] = {"\n", "\r\n", "\r"};
    unsigned total = 0;
    xuiTestProxyInit(&proxy);
    TABLE_CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    for (unsigned fixture = 0; fixture < 4; fixture++)
    for (unsigned wide = 0; wide < 2; wide++)
    for (unsigned ending = 0; ending < 3; ending++)
    for (unsigned reverse = 0; reverse < 2; reverse++)
    for (unsigned mode = 0; mode < 3; mode++)
    for (unsigned route = 0; route < (mode == 2 ? 5u : 2u); route++) {
        char* input_lf = scaffold_source(depths[fixture], wide, 0);
        char* expected_lf = scaffold_source(depths[fixture], wide, 1);
        char* before = delete_ending(input_lf, endings[ending], 1);
        char* plain = delete_ending(expected_lf, endings[ending], 1);
        char* after = route == 1 ? scaffold_editor_typed(plain) : plain;
        xui_document document = table_open(before, NULL), oracle = table_open(after, NULL);
        xui_document_snapshot old, current, expected;
        xui_document_change_set changes = NULL;
        xui_doc_range_t range, selection;
        xui_doc_editor_desc_t desc = {0};
        xui_doc_command_state_t state = {0};
        xui_widget editor;
        uint64_t revision, offset;
        unsigned quotes = 0, lists = 0, items = 0;
        int quality;
        printf("Deep quote Editor depth=%u wide=%u eol=%u reverse=%u mode=%u route=%u\n",
            depths[fixture], wide, ending, reverse, mode, route);
        fflush(stdout);
        TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK &&
            xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
            xuiDocumentMarkSaved(document, old) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        range.tAnchor = scaffold_position(old, before, "left", 0, 2);
        range.tCaret = scaffold_position(old, before, "abcd", depths[fixture], 4);
        range.tAnchor.iAffinity = XUI_DOC_BEFORE; range.tCaret.iAffinity = XUI_DOC_AFTER;
        if (reverse) { xui_doc_position_t swap = range.tAnchor; range.tAnchor = range.tCaret; range.tCaret = swap; }
        (void)scaffold_position(expected, after, "le", 0, 2);
        scaffold_counts(expected, XUI_DOCUMENT_ROOT, &quotes, &lists, &items);
        TABLE_CHECK(quotes == depths[fixture] && lists == 1 && items == 1);
        desc.iSize = sizeof(desc); desc.iMode = XUI_DOC_VISUAL;
        desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document;
        TABLE_CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 400}) == XUI_OK &&
            xuiInputViewport(context, 640, 400) == XUI_OK &&
            xuiSetFocusWidget(context, editor) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &range) == XUI_OK &&
            xuiDocumentViewSetMode(editor, modes[mode]) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        revision = xuiDocumentGetRevision(document);
        if (route < 2) TABLE_CHECK(scaffold_editor_external(document, &range, route, &changes) == XUI_OK);
        else if (route == 4) TABLE_CHECK(xuiDocumentEditorInsertText(editor, "", 0) == XUI_OK);
        else {
            uint32_t command = route == 2 ? XUI_DOC_EDIT_DELETE : XUI_DOC_EDIT_BACKSPACE;
            state.iSize = sizeof(state);
            TABLE_CHECK(xuiDocumentEditorQueryCommand(editor, command, &state) == XUI_OK && state.bEnabled &&
                xuiDocumentEditorExecute(editor, command) == XUI_OK);
        }
        TABLE_CHECK(xuiDocumentEditorFlush(editor) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
            xuiDocumentGetRevision(document) == revision + 1 &&
            xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
        table_source(current, after); table_source(old, before);
        integration_equivalent(current, after);
        formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
        TABLE_CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentPositionToSource(current, &selection.tAnchor, &offset, &quality) == XUI_OK &&
            xuiDocumentPositionToSource(current, &selection.tCaret, &offset, &quality) == XUI_OK);
        state.iSize = sizeof(state);
        TABLE_CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_UNDO, &state) == XUI_OK && state.bEnabled &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
            !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document));
        scaffold_editor_source(document, before);
        TABLE_CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        scaffold_editor_source(document, after);
        revision = xuiDocumentGetRevision(document);
        for (unsigned switched = 0; switched < 3; switched++) {
            TABLE_CHECK(xuiDocumentViewSetMode(editor, modes[switched]) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
                xuiDocumentGetRevision(document) == revision);
            scaffold_editor_source(document, after); table_source(old, before);
        }
        if (route >= 2) {
            char* typed = scaffold_editor_typed(after);
            TABLE_CHECK(xuiDocumentEditorInsertText(editor, "Z", 1) == XUI_OK &&
                xuiDocumentEditorFlush(editor) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
            scaffold_editor_source(document, typed);
            TABLE_CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
            scaffold_editor_source(document, after);
            TABLE_CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
                !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document));
            scaffold_editor_source(document, before);
            TABLE_CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
            scaffold_editor_source(document, after);
            TABLE_CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
            scaffold_editor_source(document, typed);
            free(typed);
        }
        if (changes) xuiDocumentChangeSetRelease(changes);
        xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
        free(input_lf); free(expected_lf); free(before);
        if (after != plain) free(after);
        free(plain); total++;
    }
    TABLE_CHECK(total == 432);
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Deep quote Editor: 432 external-single/compound three-mode and visual Delete/Backspace/empty-replacement/continued-input shared-history cases passed; proxy backend.");
    return 0;
}
