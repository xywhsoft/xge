#include "xui_document_ligature_model.h"
static void document_ligature_editor(xui_test_proxy_state_t* proxy)
{
    xui_context context; xui_font font; unsigned variant;
    ligature_base_shape = proxy->tProxy.textShape; proxy->tProxy.textShape = ligature_shape;
    CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &font, "ligature-editor.ttf", 20, 0) == XUI_OK &&
        xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    for (ligature_caret_mode = 0; ligature_caret_mode < 2; ligature_caret_mode++)
    for (ligature_sample = 0; ligature_sample < 4; ligature_sample++) for (variant = 0; variant < 5; variant++) {
        const char* full = ligature_sources[ligature_sample]; unsigned middle = ligature_middle[ligature_sample];
        xui_document document; xui_document_snapshot snapshot; xui_doc_desc_t profile = {0};
        xui_doc_editor_desc_t desc = {0}; xui_widget editor; uint64_t first, paragraph;
        xui_doc_range_t selection, moved, range; xui_event_t key = {0}, pointer = {0};
        xui_doc_position_t hit; xui_rect_t content, caret; char original_source[48] = {0}, expected[40];
        double advance = ligature_caret_mode ? (ligature_sample == 1 ? 4 : 8) : 6;
        profile.iSize = sizeof(profile); profile.iProfile = variant >= 2 ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK);
        if (variant >= 2) {
            uint64_t bytes; char markdown[40];
            if (variant == 2) snprintf(markdown, sizeof(markdown), "**%.*s**%s", (int)middle, full, full + middle);
            else snprintf(markdown, sizeof(markdown), "%s", full);
            CHECK(xuiDocumentLoadMarkdown(document, markdown, strlen(markdown)) == XUI_OK &&
                xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopySource(snapshot, original_source, sizeof(original_source), &bytes) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &first) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
        } else {
            xui_document_transaction transaction; xui_doc_node_desc_t node = {0}; char prefix[16];
            CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
            memcpy(prefix, full, middle); prefix[middle] = 0;
            node.iKind = XUI_DOC_TEXT; node.sText = variant ? prefix : full; node.iTextBytes = strlen(node.sText);
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &first) == XUI_OK);
            if (variant) {
                uint64_t tail;
                node.sText = full + middle; node.iTextBytes = strlen(node.sText); node.tAttributes.iTextColor = XUI_COLOR_RGBA(180, 20, 40, 255);
                CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &tail) == XUI_OK);
            }
            CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document;
        desc.tView.tRenderer.iSize = sizeof(desc.tView.tRenderer);
        desc.tView.tRenderer.tFonts = (xui_doc_font_set_t){font, font, font, font, font};
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 200}) == XUI_OK &&
            xuiInputViewport(context, 320, 200) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        if (variant >= 3) CHECK(xuiDocumentViewSetMode(editor, variant == 3 ? XUI_DOC_SOURCE_TEXT : XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
        selection.tAnchor = selection.tCaret = doc_grapheme_edit_position(document, first, 0);
        if (variant >= 3) {
            selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_SOURCE;
            selection.tAnchor.iNodeId = selection.tCaret.iNodeId = 1;
        }
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor; key.iKey = XUI_KEY_RIGHT;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK && xuiDocumentViewGetSelection(editor, &moved) == XUI_OK);
        range.tAnchor = selection.tCaret; range.tCaret = moved.tCaret;
        {
            char* copy; uint64_t bytes;
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopyRange(snapshot, &range, &copy, &bytes) == XUI_OK && bytes == middle && !memcmp(copy, full, middle));
            xuiDocumentFreeBuffer(copy); xuiDocumentSnapshotRelease(snapshot);
        }
        content = xuiWidgetGetContentRect(editor); caret = xuiEditGetCaretRect(editor);
        if (fabs((double)caret.fX - content.fX - advance) >= .01)
            fprintf(stderr, "Ligature editor mode=%u sample=%u variant=%u caret=%g content=%g expected=%g\n",
                ligature_caret_mode, ligature_sample, variant, (double)caret.fX, (double)content.fX, advance);
        CHECK(fabs((double)caret.fX - content.fX - advance) < .01);
        pointer.iSize = sizeof(pointer); pointer.pTarget = editor; pointer.iPointerId = 9; pointer.iPointerType = XUI_POINTER_TYPE_MOUSE;
        pointer.iButton = XUI_POINTER_BUTTON_LEFT; pointer.fX = (float)(content.fX + advance - 1); pointer.fY = content.fY + 10;
        CHECK(xuiDocumentViewHitTest(editor, pointer.fX, pointer.fY, &hit) == XUI_OK && hit.iOffset == middle);
        pointer.iType = XUI_EVENT_POINTER_DOWN; pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
        CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
        pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButtons = 0;
        {
            int pointer_result = xuiDispatchEvent(context, &pointer);
            int backspace_result = pointer_result == XUI_OK ? xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) : pointer_result;
            if (pointer_result != XUI_OK || backspace_result != XUI_OK)
                fprintf(stderr, "Ligature editor delete mode=%u sample=%u variant=%u pointer=%d backspace=%d hit-node=%llu hit-offset=%llu\n",
                    ligature_caret_mode, ligature_sample, variant, pointer_result, backspace_result,
                    (unsigned long long)hit.iNodeId, (unsigned long long)hit.iOffset);
            CHECK(pointer_result == XUI_OK && backspace_result == XUI_OK);
        }
        snprintf(expected, sizeof(expected), "%s\n", full + middle); doc_grapheme_edit_plain(document, expected);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        snprintf(expected, sizeof(expected), "%s\n", full); doc_grapheme_edit_plain(document, expected);
        if (variant >= 2) {
            char source[48]; uint64_t bytes;
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopySource(snapshot, source, sizeof(source), &bytes) == XUI_OK && !strcmp(source, original_source));
            xuiDocumentSnapshotRelease(snapshot);
        }
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    proxy->tProxy.textShape = ligature_base_shape;
    xuiDestroy(context); proxy->tProxy.fontDestroy(&proxy->tProxy, font);
    puts("Document ligature editor: equal/provider stops, Rich joined/split and Markdown VISUAL/SOURCE/LIVE arrows, pointer, grapheme backspace and exact undo passed");
}
