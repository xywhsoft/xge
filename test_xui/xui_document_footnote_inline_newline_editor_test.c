#include "test_xui/xui_document_footnote_inline_newline_cases.h"
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

/* VISUAL collapsed single-newline input dispatches SplitListItem, whereas
 * both external commands below split a paragraph. Ordered marker 007) starts
 * at seven; the right item is eight, with a logical four-column content indent.
 * A new task item starts unchecked. Existing tail indentation follows its new
 * item; the hidden footnote definition remains outside that item. */
static const char* newline_editor_ordered_enter =
    "use[^n]\n\n[^n]: 007)\tab\n            \n    8)\tcd &amp;\n\n        tail &amp;\n\n    [unused]: <raw> 'Title'\n\nend &amp;\n";
static const char* newline_editor_task_enter =
    "use[^n]\n\n[^n]: + [X] ab\n      \n    + [ ] cd &amp;\n\n      tail &amp;\n\n    [unused]: <raw> 'Title'\n\nend &amp;\n";

int main(void)
{
    xui_test_proxy_state_t proxy;
    xui_context context;
    xui_font font;
    unsigned total = 0;
    const uint32_t modes[] = {XUI_DOC_SOURCE_TEXT, XUI_DOC_LIVE_MARKDOWN, XUI_DOC_VISUAL};
    const char* endings[] = {"\n", "\r\n", "\r"};
    char label76[77]; memset(label76, 'r', 76); label76[76] = 0;
    const char* labels[] = {"脚注", label76};
    xuiTestProxyInit(&proxy);
    TABLE_CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    for (unsigned fixture = 1; fixture < 5; fixture++)
    for (unsigned label = 0; label < 2; label++)
    for (unsigned ending = 0; ending < 3; ending++)
    for (unsigned mode = 0; mode < 3; mode++)
    for (unsigned route = 0; route < (mode == 2 ? 3u : 2u); route++) {
        const struct newline_fixture* f = &newline_fixtures[fixture];
        char* raw_before = newline_label(f->before, labels[label]);
        const char* expected_source = route == 2 && fixture == 3 ? newline_editor_ordered_enter :
            route == 2 && fixture == 4 ? newline_editor_task_enter : f->after;
        char* raw_after = newline_label(expected_source, labels[label]);
        char* before = delete_ending(raw_before, endings[ending], 1);
        char* after = delete_ending(raw_after, endings[ending], 1);
        free(raw_before); free(raw_after);
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
        printf("Footnote Editor fixture=%u label=%u eol=%u mode=%u route=%u\n", fixture, label, ending, mode, route);
        fflush(stdout);
        TABLE_CHECK(xuiDocumentClearHistory(document) == XUI_OK &&
            xuiDocumentAcquireSnapshot(document, &old) == XUI_OK &&
            xuiDocumentMarkSaved(document, old) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        newline_footnote_scope(old, before, "abcd");
        newline_footnote_scope(expected, after, "cd");
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
    TABLE_CHECK(total == 168);
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Footnote Editor: 168 external-command/three-mode/shared-history and visual-Enter/continued-input cases passed; proxy backend.");
    return 0;
}
