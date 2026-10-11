#define BLOCK_KINDS_MATRIX_NO_MAIN
#include "xui_document_block_kind_move_test.c"
#undef BLOCK_KINDS_MATRIX_NO_MAIN
#include "xui_document_ui.h"
#include "test_xui/xui_test_proxy.h"
#define BLOCK_EDITOR_CHECK(x) do { if (!(x)) { fprintf(stderr, "Block Editor line %d: %s\n", __LINE__, #x); exit(2); } } while (0)

static int block_editor_source(xui_document d, const char* wanted)
{
    xui_document_snapshot s = NULL; int equal;
    BLOCK_EDITOR_CHECK(xuiDocumentAcquireSnapshot(d, &s) == XUI_OK);
    equal = source_equal(s, wanted); xuiDocumentSnapshotRelease(s); return equal;
}
int main(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    unsigned total = 0; uint32_t modes[] = {XUI_DOC_SOURCE_TEXT, XUI_DOC_LIVE_MARKDOWN, XUI_DOC_VISUAL};
    xuiTestProxyInit(&proxy);
    BLOCK_EDITOR_CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK &&
        proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK && xuiSetDefaultFont(context, font) == XUI_OK);
    for (unsigned fixture = 0; fixture < sizeof(cases) / sizeof(*cases); fixture++)
    for (unsigned mode = 0; mode < 3; mode++) for (unsigned selection = 0; selection < 3; selection++) {
        const block_case* c = &cases[fixture]; char before[2048], wanted[2048];
        xui_document d = NULL, oracle = NULL; xui_document_snapshot old = NULL, now = NULL, expected = NULL;
        xui_document_transaction t = NULL; xui_widget editor; xui_doc_editor_desc_t desc = {0};
        xui_doc_node_info_t moving_info = {0}; xui_doc_command_state_t command = {0};
        xui_doc_range_t range = {0}, selected; uint64_t moving, parent, child, old_index = 0, revision, offset; int quality;
        source_variant(c->before, before, 1, 1, 0);
        source_variant(c->after, wanted, 1, 1, fixture == 26 ? 1 : 0);
        moving_info.iSize = sizeof(moving_info);
        BLOCK_EDITOR_CHECK(open_document(before, &d) == XUI_OK && xuiDocumentClearHistory(d) == XUI_OK &&
            xuiDocumentAcquireSnapshot(d, &old) == XUI_OK && xuiDocumentMarkSaved(d, old) == XUI_OK &&
            open_document(wanted, &oracle) == XUI_OK && xuiDocumentAcquireSnapshot(oracle, &expected) == XUI_OK &&
            moving_node(old, before, c->moving_kind, &moving) == XUI_OK && find_kind(old, 1, c->target_kind, &parent) == XUI_OK &&
            xuiDocumentSnapshotGetNode(old, moving, &moving_info) == XUI_OK &&
            xuiDocumentSourceToPosition(old, (uint64_t)(strstr(before, "BETA") - before), &range.tAnchor, &quality) == XUI_OK);
        for (;;) {
            BLOCK_EDITOR_CHECK(xuiDocumentSnapshotGetChild(old, moving_info.iParentId, old_index, &child) == XUI_OK);
            if (child == moving) break;
            old_index++;
        }
        range.tCaret = range.tAnchor;
        if (selection) {
            range.tAnchor.iKind = XUI_DOC_POSITION_GAP; range.tAnchor.iNodeId = moving_info.iParentId;
            range.tAnchor.iOffset = old_index + (selection == 2);
            range.tCaret = range.tAnchor; range.tCaret.iOffset = old_index + (selection == 1);
        }
        desc.iSize = sizeof(desc); desc.iMode = XUI_DOC_VISUAL; desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d;
        BLOCK_EDITOR_CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 400}) == XUI_OK && xuiInputViewport(context, 640, 400) == XUI_OK &&
            xuiSetFocusWidget(context, editor) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
            xuiDocumentViewSetSelection(editor, &range) == XUI_OK && xuiDocumentViewSetMode(editor, modes[mode]) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        BLOCK_EDITOR_CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK &&
            xuiDocumentTxnMoveNode(t, moving, parent, c->index) == XUI_OK && xuiDocumentTxnCommit(t, NULL) == XUI_OK);
        xuiDocumentTxnRelease(t);
        BLOCK_EDITOR_CHECK(xuiUpdate(context, .016f) == XUI_OK && xuiDocumentGetRevision(d) == revision + 1 &&
            block_editor_source(d, wanted) && xuiDocumentAcquireSnapshot(d, &now) == XUI_OK && source_equal(old, before) &&
            all_ids(old, now, 1, moving, parent) && xuiDocumentSnapshotGetChild(now, parent, c->index, &child) == XUI_OK && child == moving &&
            xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
            xuiDocumentPositionToSource(now, &selected.tAnchor, &offset, &quality) == XUI_OK &&
            xuiDocumentPositionToSource(now, &selected.tCaret, &offset, &quality) == XUI_OK);
        char *actual_html = NULL, *expected_html = NULL; uint64_t na, nb;
        BLOCK_EDITOR_CHECK(xuiDocumentExportHtml(now, &actual_html, &na) == XUI_OK &&
            xuiDocumentExportHtml(expected, &expected_html, &nb) == XUI_OK && na == nb && !memcmp(actual_html, expected_html, (size_t)na));
        xuiDocumentFreeBuffer(actual_html); xuiDocumentFreeBuffer(expected_html);
        command.iSize = sizeof(command);
        BLOCK_EDITOR_CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_UNDO, &command) == XUI_OK && command.bEnabled &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
            block_editor_source(d, before) && !xuiDocumentIsDirty(d) && !xuiDocumentCanUndo(d) &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK && block_editor_source(d, wanted));
        revision = xuiDocumentGetRevision(d);
        for (unsigned switched = 0; switched < 3; switched++)
            BLOCK_EDITOR_CHECK(xuiDocumentViewSetMode(editor, modes[switched]) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
                xuiDocumentGetRevision(d) == revision && block_editor_source(d, wanted) && source_equal(old, before));
        xuiDocumentSnapshotRelease(now); xuiDocumentSnapshotRelease(expected); xuiDocumentSnapshotRelease(old);
        xuiDocumentRelease(oracle); xuiWidgetDestroy(editor); xuiDocumentRelease(d); total++;
    }
    BLOCK_EDITOR_CHECK(total == 243);
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Block Editor: 243 actual DLL external-Document-move/selection/source/HTML/all-NodeId/shared-history/three-mode cases passed; proxy backend.");
    return 0;
}
