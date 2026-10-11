#include "xui_document_move_cases.h"
#include "xui_document_ui.h"
#include "test_xui/xui_test_proxy.h"
#define EDITOR_MOVE_CHECK(x) do { if (!(x)) { fprintf(stderr, "Move Editor line %d: %s\n", __LINE__, #x); exit(2); } } while (0)
static int editor_move_source(xui_document d, const char* expected)
{
    xui_document_snapshot s; int result;
    EDITOR_MOVE_CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    result = move_source_equal(s, expected); xuiDocumentSnapshotRelease(s); return result;
}
int main(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    const char* endings[] = {"\n", "\r\n", "\r"}; unsigned total = 0;
    xuiTestProxyInit(&proxy);
    EDITOR_MOVE_CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    for (unsigned fixture = 0; fixture < 14; fixture++) for (unsigned eol = 0; eol < 3; eol++)
    for (unsigned bom = 0; bom < 2; bom++) for (unsigned selection = 0; selection < 3; selection++) {
        const sample* c = fixture < 8 ? &samples[fixture] :
            (fixture < 11 ? &eof_samples[fixture - 8] : &definition_eof_samples[fixture - 11]);
        char *before = move_source_variant(c->before, endings[eol], bom),
            *wanted = move_source_variant(c->after, endings[eol], bom);
        xui_document d, oracle; xui_document_snapshot old, now, expected;
        xui_widget editor; xui_doc_editor_desc_t desc = {0}; xui_doc_command_state_t command = {0};
        xui_doc_range_t range = {0}, selected;
        uint64_t block, parent, index, moved, revision, offset; int quality;
        EDITOR_MOVE_CHECK(before && wanted && open_document(before, &d) == XUI_OK &&
            xuiDocumentClearHistory(d) == XUI_OK && xuiDocumentAcquireSnapshot(d, &old) == XUI_OK &&
            xuiDocumentMarkSaved(d, old) == XUI_OK && open_document(wanted, &oracle) == XUI_OK &&
            xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK);
        EDITOR_MOVE_CHECK(xuiDocumentSourceToPosition(old, strstr(before, c->needle) - before,
            &range.tAnchor, &quality) == XUI_OK &&
            move_target(old, c, &range.tAnchor, &block, &parent, &index) == XUI_OK);
        range.tCaret = range.tAnchor;
        if (selection || c->root_gap) {
            range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tAnchor.iNodeId = parent;
            range.tAnchor.iOffset = selection == 2 ? index + 1 : index;
            range.tCaret = range.tAnchor; range.tCaret.iOffset = selection == 2 ? index : index + 1;
        }
        desc.iSize = sizeof(desc); desc.iMode = XUI_DOC_VISUAL; desc.tView.iSize = sizeof(desc.tView);
        desc.tView.pDocument = d;
        EDITOR_MOVE_CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 400}) == XUI_OK &&
            xuiInputViewport(context, 640, 400) == XUI_OK && xuiSetFocusWidget(context, editor) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK && xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        uint32_t action = c->down ? XUI_DOC_EDIT_MOVE_BLOCK_DOWN : XUI_DOC_EDIT_MOVE_BLOCK_UP;
        command.iSize = sizeof(command); revision = xuiDocumentGetRevision(d);
        EDITOR_MOVE_CHECK(xuiDocumentEditorQueryCommand(editor, action, &command) == XUI_OK &&
            command.bEnabled && xuiDocumentEditorCanExecute(editor, action) &&
            xuiDocumentEditorExecute(editor, action) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
            xuiDocumentGetRevision(d) == revision + 1 && editor_move_source(d, wanted) &&
            xuiDocumentAcquireSnapshot(d, &now) == XUI_OK &&
            xuiDocumentSnapshotGetChild(now, parent, c->down ? index + 1 : index - 1, &moved) == XUI_OK && moved == block &&
            xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
            xuiDocumentPositionToSource(now, &selected.tAnchor, &offset, &quality) == XUI_OK &&
            xuiDocumentPositionToSource(now, &selected.tCaret, &offset, &quality) == XUI_OK &&
            move_source_equal(old, before));
        char *a = NULL, *b = NULL; uint64_t na, nb;
        EDITOR_MOVE_CHECK(xuiDocumentExportHtml(now, &a, &na) == XUI_OK &&
            xuiDocumentExportHtml(expected, &b, &nb) == XUI_OK && na == nb && !memcmp(a, b, (size_t)na));
        xuiDocumentFreeBuffer(a); xuiDocumentFreeBuffer(b);
        EDITOR_MOVE_CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK &&
            editor_move_source(d, before) && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK && editor_move_source(d, wanted));
        uint32_t modes[] = {XUI_DOC_SOURCE_TEXT, XUI_DOC_LIVE_MARKDOWN, XUI_DOC_VISUAL};
        revision = xuiDocumentGetRevision(d);
        for (unsigned mode = 0; mode < 3; mode++)
            EDITOR_MOVE_CHECK(xuiDocumentViewSetMode(editor, modes[mode]) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
                xuiDocumentGetRevision(d) == revision && editor_move_source(d, wanted) && move_source_equal(old, before));
        xuiDocumentSnapshotRelease(expected); xuiDocumentSnapshotRelease(now); xuiDocumentSnapshotRelease(old);
        xuiDocumentRelease(oracle); xuiWidgetDestroy(editor); xuiDocumentRelease(d); free(before); free(wanted); total++;
    }
    EDITOR_MOVE_CHECK(total == 252);
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Move Editor: 252 actual DLL Query/Execute/source/HTML/StableId/selection/Undo/Redo/three-mode cases passed; proxy backend.");
    return 0;
}
