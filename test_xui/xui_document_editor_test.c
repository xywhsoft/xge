#include "../xui_document_ui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #c); exit(1); } } while (0)
static void plain(xui_document d, const char* expected)
{
    xui_document_snapshot s; char* text; uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentSnapshotCopyPlainText(s, &text, &bytes) == XUI_OK);
    if (strcmp(text, expected)) fprintf(stderr, "Expected [%s], got [%s]\n", expected, text);
    CHECK(!strcmp(text, expected)); xuiDocumentFreeBuffer(text); xuiDocumentSnapshotRelease(s);
}
static void source(xui_document d, const char* expected)
{
    xui_document_snapshot s; char text[1024]; uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK); CHECK(xuiDocumentSnapshotCopySource(s, text, sizeof(text), &bytes) == XUI_OK);
    if (strcmp(text, expected)) fprintf(stderr, "Expected source [%s], got [%s]\n", expected, text);
    CHECK(!strcmp(text, expected)); xuiDocumentSnapshotRelease(s);
}
static void send_text(xui_context context, xui_widget w, const char* text, int ime, int active)
{
    xui_event_t e = {0}; e.iSize = sizeof(e); e.iType = ime ? XUI_EVENT_IME_COMPOSITION : XUI_EVENT_TEXT; e.pTarget = w;
    strcpy(e.sText, text); e.iTextSize = (int)strlen(text); e.bCompositionActive = active; e.iCompositionCursor = e.iTextSize;
    CHECK(xuiDispatchEvent(context, &e) == XUI_OK);
}
int main(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; xui_surface target; xui_widget editor, view;
    xui_doc_editor_desc_t ed = {0}; xui_doc_view_desc_t vd = {0}; xui_document d; xui_doc_range_t selection, before;
    xui_rect_i_t damage = {0, 0, 640, 480}; xui_doc_stats_t stats = {0}; uint64_t revision, count;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK); CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK); CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&proxy, &target, 640, 480, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = d;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
    CHECK(xuiWidgetIsType(editor, xuiDocumentViewGetType(context)));
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK); CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiInputViewport(context, 320, 240) == XUI_OK); CHECK(xuiSetFocusWidget(context, editor) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "one\ntwo", 7) == XUI_OK); plain(d, "one\ntwo\n");
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iOffset == 3);
    before = selection; CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK); plain(d, "one\ntw\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); plain(d, "one\ntwo\n");
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == before.tCaret.iNodeId && selection.tCaret.iOffset == 3);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK); plain(d, "one\ntw\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_SELECT_ALL) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_COPY) == XUI_OK); CHECK(!strcmp(proxy.sClipboard, "one\ntw\n"));
    CHECK(xuiTestProxySetClipboardText(&proxy, "A\r\nB") == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_OK); plain(d, "A\nB\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK); plain(d, "A\n\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK); plain(d, "A\n");
    CHECK(xuiDocumentEditorInsertText(editor, "e\xcc\x81", 3) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK); plain(d, "A\n");
    CHECK(xuiDocumentEditorInsertText(editor, "\xf0\x9f\x91\xa8\xe2\x80\x8d\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x91\xa7", 18) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK); plain(d, "A\n");
    CHECK(xuiDocumentEditorInsertText(editor, "bc", 2) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK); selection.tAnchor = selection.tCaret; selection.tAnchor.iOffset = 1;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK); CHECK(xuiDocumentEditorSetMarks(editor, XUI_DOC_BOLD, 0) == XUI_OK);
    plain(d, "Abc\n"); CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK); selection.tAnchor = selection.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    vd.iSize = sizeof(vd); vd.pDocument = d; CHECK(xuiDocumentViewCreate(context, &vd, &view) == XUI_OK);
    CHECK(xuiDocumentViewSetSelection(view, &selection) == XUI_OK);
    revision = xuiDocumentGetRevision(d); stats.iSize = sizeof(stats); CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK); count = stats.iUndoCount;
    send_text(context, editor, "\xe4\xb8\xad", 1, 1); CHECK(xuiDocumentEditorIsComposing(editor));
    CHECK(xuiDocumentGetRevision(d) == revision); plain(d, "Abc\n");
    CHECK(xuiUpdate(context, .016f) == XUI_OK); CHECK(xuiRender(context, target, &damage, 1) == XUI_OK);
    send_text(context, editor, "", 1, 0); CHECK(!xuiDocumentEditorIsComposing(editor)); CHECK(xuiDocumentGetRevision(d) == revision);
    send_text(context, editor, "\xe4\xb8\xad", 1, 1); send_text(context, editor, "\xe4\xb8\xad\xe6\x96\x87", 1, 0);
    CHECK(!xuiDocumentEditorIsComposing(editor)); CHECK(xuiDocumentGetRevision(d) == revision + 1); plain(d, "Abc\xe4\xb8\xad\xe6\x96\x87\n");
    CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK && stats.iUndoCount == count + 1);
    CHECK(xuiDocumentViewGetSelection(view, &before) == XUI_OK && before.tCaret.iRevision == xuiDocumentGetRevision(d));
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); plain(d, "Abc\n");
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK); revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentEditorInsertText(editor, "bad", 3) == XUI_ERROR_INVALID_STATE); send_text(context, editor, "bad", 0, 0); CHECK(xuiDocumentGetRevision(d) == revision);
    {
        xui_doc_command_state_t command = {0}; xui_doc_range_t match; xui_event_t event = {0}; int start, end;
        xui_accessible_node_t accessible = {0}; uint64_t replaced;
        CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
        CHECK((xuiEditGetCapabilities(editor) & (XUI_EDIT_CAP_TEXT | XUI_EDIT_CAP_UNDO | XUI_EDIT_CAP_STRUCTURED)) == (XUI_EDIT_CAP_TEXT | XUI_EDIT_CAP_UNDO | XUI_EDIT_CAP_STRUCTURED));
        CHECK(xuiEditSetText(editor, "abCD\nabCD") == XUI_OK); CHECK(!strcmp(xuiEditGetText(editor), "abCD\nabCD\n"));
        CHECK(xuiEditSetSelection(editor, 0, 2) == XUI_OK); CHECK(xuiEditGetSelection(editor, &start, &end) == XUI_OK && start == 0 && end == 2);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BOLD) == XUI_OK); CHECK(xuiEditSelectAll(editor) == XUI_OK);
        command.iSize = sizeof(command); CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BOLD, &command) == XUI_OK && command.bEnabled && !command.bActive && command.bMixed);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BOLD) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BOLD, &command) == XUI_OK && command.bActive && !command.bMixed);
        CHECK(xuiEditSetSelection(editor, 0, 0) == XUI_OK);
        CHECK(xuiDocumentViewFind(editor, "CD", 2, 0, 0, &match) == XUI_OK);
        CHECK(xuiEditGetSelection(editor, &start, &end) == XUI_OK && start == 2 && end == 4);
        CHECK(xuiDocumentViewFind(editor, "CD", 2, 0, 0, &match) == XUI_OK);
        CHECK(xuiEditGetSelection(editor, &start, &end) == XUI_OK && start == 7 && end == 9);
        CHECK(xuiDocumentEditorReplaceAll(editor, "CD", 2, "_", 1, NULL, &replaced) == XUI_OK && replaced == 2); plain(d, "ab_\nab_\n");
        CHECK(xuiEditUndo(editor) == XUI_OK); plain(d, "abCD\nabCD\n");
        CHECK(xuiEditSelectAll(view) == XUI_OK); CHECK(xuiEditCopy(view) == XUI_OK && !strcmp(proxy.sClipboard, "abCD\nabCD\n"));
        CHECK(xuiEditIsReadonly(view));
        CHECK(xuiWidgetGetAccessibleNode(editor, 0, &accessible) == XUI_OK && accessible.iRole == XUI_ACCESSIBLE_ROLE_TEXTBOX);
        CHECK(xuiEditSetSelection(editor, 2, 2) == XUI_OK);
        event.iSize = sizeof(event); event.iType = XUI_EVENT_IME_COMPOSITION; event.pTarget = editor;
        event.bCompositionReplacementRange = 1; event.iCompositionReplacementStart = 0; event.iCompositionReplacementEnd = 2;
        event.iTextSize = 1; strcpy(event.sText, "Z"); CHECK(xuiDispatchEvent(context, &event) == XUI_OK); plain(d, "ZCD\nabCD\n");
        CHECK(xuiEditUndo(editor) == XUI_OK);
        memset(&event, 0, sizeof(event)); event.iSize = sizeof(event); event.pTarget = editor;
        event.iType = XUI_EVENT_TEXT; event.iCodepoint = 0x4e2d;
        CHECK(xuiDispatchEvent(context, &event) == XUI_OK); plain(d, "ab\xe4\xb8\xad" "CD\nabCD\n");
        CHECK(xuiEditUndo(editor) == XUI_OK);
    }
    xuiWidgetDestroy(view); xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    {
        xui_doc_desc_t md = {0}; md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(d, "# Title\n", 8) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_SOURCE_TEXT;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK); CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiDocumentEditorInsertText(editor, "intro\n\n", 7) == XUI_OK); source(d, "intro\n\n# Title\n");
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, "# Title\n");
        CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iKind != XUI_DOC_POSITION_SOURCE);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_SELECT_ALL) == XUI_OK); CHECK(xuiDocumentEditorInsertText(editor, "hello", 5) == XUI_OK);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK); CHECK(xuiDocumentEditorInsertText(editor, " world", 6) == XUI_OK); source(d, "hello world");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, "hello");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ENTER) == XUI_OK);
        CHECK(xuiDocumentEditorInsertText(editor, "second", 6) == XUI_OK); plain(d, "hello\nsecond\n");
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK); plain(d, "hello\nsecon\n");
        revision = xuiDocumentGetRevision(d); send_text(context, editor, "x", 1, 1); CHECK(xuiDocumentEditorIsComposing(editor));
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK); CHECK(!xuiDocumentEditorIsComposing(editor) && xuiDocumentGetRevision(d) == revision);
        CHECK(!xuiDocumentEditorCanExecute(editor, XUI_DOC_EDIT_BOLD));
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK); CHECK(!xuiDocumentEditorCanExecute(editor, XUI_DOC_EDIT_UNDERLINE));
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        const char* original = "# Title\n\nA **bold** line.\n\nTail\n";
        xui_doc_desc_t md = {0}; int start, end;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_LIVE_MARKDOWN;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
        CHECK(xuiSetRootWidget(context, editor) == XUI_OK); CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiSetFocusWidget(context, editor) == XUI_OK);
        CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK); count = stats.iUndoCount; revision = xuiDocumentGetRevision(d);
        CHECK(xuiEditSetSelection(editor, 13, 17) == XUI_OK);
        CHECK(xuiUpdate(context, .016f) == XUI_OK); CHECK(xuiRender(context, target, &damage, 1) == XUI_OK);
        CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iKind == XUI_DOC_POSITION_SOURCE);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiEditGetSelection(editor, &start, &end) == XUI_OK && start == 13 && end == 17);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
        CHECK(xuiDocumentGetRevision(d) == revision && xuiDocumentViewGetDocument(editor) == d);
        CHECK(xuiDocumentGetStats(d, &stats) == XUI_OK && stats.iUndoCount == count);
        CHECK(xuiDocumentEditorInsertText(editor, "new", 3) == XUI_OK); source(d, "# Title\n\nA **new** line.\n\nTail\n");
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
        CHECK(xuiDocumentEditorInsertText(editor, "X", 1) == XUI_OK); source(d, "# Title\n\nA **newX** line.\n\nTail\n");
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK); CHECK(xuiEditUndo(editor) == XUI_OK);
        source(d, "# Title\n\nA **new** line.\n\nTail\n");
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK); CHECK(xuiEditUndo(editor) == XUI_OK); source(d, original);
        CHECK(xuiEditGetSelection(editor, &start, &end) == XUI_OK && start == 13 && end == 17);
        CHECK(xuiEditSetSelection(editor, 13, 17) == XUI_OK);
        revision = xuiDocumentGetRevision(d); send_text(context, editor, "\xe4\xb8\xad", 1, 1);
        CHECK(xuiDocumentEditorIsComposing(editor) && xuiDocumentGetRevision(d) == revision);
        CHECK(xuiUpdate(context, .016f) == XUI_OK); CHECK(xuiRender(context, target, &damage, 1) == XUI_OK);
        send_text(context, editor, "\xe4\xb8\xad\xe6\x96\x87", 1, 0);
        source(d, "# Title\n\nA **\xe4\xb8\xad\xe6\x96\x87** line.\n\nTail\n");
        CHECK(xuiDocumentGetRevision(d) == revision + 1);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK); CHECK(xuiEditUndo(editor) == XUI_OK); source(d, original);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
        CHECK(xuiEditSelectAll(editor) == XUI_OK); CHECK(xuiEditCopy(editor) == XUI_OK && !strcmp(proxy.sClipboard, original));
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target); xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("DocumentEditor: structural input, clipboard, grapheme deletion, marks, selection undo, read-only, IME projection and three-mode Markdown history passed."); return 0;
}
