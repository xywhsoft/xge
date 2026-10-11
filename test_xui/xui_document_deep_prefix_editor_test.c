#include "xui_document_deep_prefix_editor_cases.h"
#include "xui_document_ui.h"
#include "test_xui/xui_test_proxy.h"

static void newline_editor_source(xui_document document, const char* source)
{
    xui_document_snapshot snapshot;
    TABLE_CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    table_source(snapshot, source);
    xuiDocumentSnapshotRelease(snapshot);
}

static char* newline_editor_typed_source(const char* source)
{
    const char* content = strstr(source, "cd &amp;");
    size_t bytes = strlen(source), offset;
    TABLE_CHECK(content);
    offset = (size_t)(content - source);
    char* typed = malloc(bytes + 2);
    TABLE_CHECK(typed);
    memcpy(typed, source, offset); typed[offset] = 'Z';
    memcpy(typed + offset + 1, source + offset, bytes - offset + 1);
    return typed;
}

int main(void)
{
    xui_test_proxy_state_t proxy;
    xui_context context;
    xui_font font;
    unsigned total = 0;
    const uint32_t modes[] = {XUI_DOC_SOURCE_TEXT, XUI_DOC_LIVE_MARKDOWN, XUI_DOC_VISUAL};
    const char* endings[] = {"\n", "\r\n", "\r"};
    xuiTestProxyInit(&proxy);
    TABLE_CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    for (unsigned fixture = 0; fixture < sizeof(prefix_fixtures)/sizeof(prefix_fixtures[0]); fixture++)
    for (unsigned ending = 0; ending < 3; ending++)
    for (unsigned mode = 0; mode < 3; mode++)
    for (unsigned route = 0; route < (mode == 2 ? 3u : 2u); route++) {
        const struct prefix_fixture* f = &prefix_fixtures[fixture];
        char* before = delete_ending(f->before, endings[ending], 1);
        char* after = delete_ending(f->after, endings[ending], 1);
        xui_document document = table_open(before, NULL), oracle = table_open(after, NULL);
        xui_document_snapshot old, current, expected;
        xui_document_change_set changes = NULL;
        xui_doc_position_t caret = {0};
        xui_doc_range_t range, selection;
        xui_doc_editor_desc_t desc = {0};
        xui_doc_command_state_t command = {0};
        xui_widget editor;
        uint64_t revision, offset;
        int quality;
        boundary_case operation = {before, after, "\n", 2, 2, 2};
        printf("Wide/deep prefix Editor fixture=%u name=%s depth=%u eol=%u mode=%u route=%u\n", fixture, f->name, f->depth, ending, mode, route);
        fflush(stdout);
        TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK &&
            xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
            xuiDocumentMarkSaved(document, old) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        prefix_scope(old, before, "abcd", f);
        prefix_scope(expected, after, "cd", f);
        range = boundary_selection(document, old, before, &operation, 0);
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
        TABLE_CHECK((route == 2 ? xuiDocumentEditorInsertText(editor, "\n", 1) :
            newline_execute(document, &range, route, &caret, &changes)) == XUI_OK);
        TABLE_CHECK(xuiDocumentEditorFlush(editor) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
            xuiDocumentGetRevision(document) == revision + 1 &&
            xuiDocumentAcquireSnapshot(document, &current) == XUI_OK);
        table_source(current, after); table_source(old, before);
        integration_equivalent(current, after);
        formatted_split_same_semantics(current, XUI_DOCUMENT_ROOT, expected, XUI_DOCUMENT_ROOT);
        TABLE_CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentPositionToSource(current, &selection.tAnchor, &offset, &quality) == XUI_OK &&
            xuiDocumentPositionToSource(current, &selection.tCaret, &offset, &quality) == XUI_OK);
        command.iSize = sizeof(command);
        TABLE_CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_UNDO, &command) == XUI_OK && command.bEnabled &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
            !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document));
        newline_editor_source(document, before);
        TABLE_CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        newline_editor_source(document, after);
        revision = xuiDocumentGetRevision(document);
        for (unsigned switched = 0; switched < 3; switched++) {
            TABLE_CHECK(xuiDocumentViewSetMode(editor, modes[switched]) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
                xuiDocumentGetRevision(document) == revision);
            newline_editor_source(document, after); table_source(old, before);
        }
        if (route == 2) {
            char* typed = newline_editor_typed_source(after);
            TABLE_CHECK(xuiDocumentEditorInsertText(editor, "Z", 1) == XUI_OK &&
                xuiDocumentEditorFlush(editor) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
            newline_editor_source(document, typed);
            TABLE_CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
            newline_editor_source(document, after);
            TABLE_CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
                !xuiDocumentIsDirty(document) && !xuiDocumentCanUndo(document));
            newline_editor_source(document, before);
            free(typed);
        }
        if (changes) xuiDocumentChangeSetRelease(changes);
        xuiDocumentSnapshotRelease(current); xuiDocumentSnapshotRelease(old);
        xuiDocumentSnapshotRelease(expected); xuiDocumentRelease(oracle);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
        free(before); free(after); total++;
    }
    TABLE_CHECK(total == 672);
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Wide/deep prefix Editor: 672 independent-source external-command/three-mode/shared-history and visual-Enter/continued-input cases passed; proxy backend.");
    return 0;
}
