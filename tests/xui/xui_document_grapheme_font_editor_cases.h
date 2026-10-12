#include "xui_document_grapheme_font_model.h"
static void document_grapheme_font_editor(xui_test_proxy_state_t* proxy)
{
    static const uint32_t styles[] = {0, XUI_DOC_BOLD, XUI_DOC_SUPERSCRIPT, XUI_DOC_SUBSCRIPT};
    xui_context context; unsigned form, style;
    unit_font_base_shape = proxy->tProxy.textShape; proxy->tProxy.textShape = unit_shape;
    CHECK(proxy->tProxy.fontLoadFile(&proxy->tProxy, &unit_fonts[0], "unit-editor.ttf", 20, 0) == XUI_OK &&
        proxy->tProxy.fontLoadFile(&proxy->tProxy, &unit_fonts[1], "unit-editor-bold.ttf", 40, 0) == XUI_OK &&
        xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy->tProxy) == XUI_OK &&
        xuiSetDefaultFont(context, unit_fonts[0]) == XUI_OK);
    for (unit_sample = 0; unit_sample < 3; unit_sample++) for (form = 0; form < 2; form++)
    for (style = 0; style < (form ? 2u : 4u); style++) {
        xui_document document; xui_document_snapshot snapshot; xui_widget editor;
        xui_doc_editor_desc_t desc = {0}; uint64_t first, paragraph;
        xui_doc_range_t initial, moved, range; xui_event_t key = {0}, pointer = {0};
        xui_doc_position_t hit; xui_rect_t content, caret; char source[48] = {0}, full[32];
        unsigned split = unit_prefix[unit_sample], length = (unsigned)strlen(unit_stems[unit_sample]);
        double advance = style == 1 ? 22 : 11;
        if (form) {
            xui_doc_desc_t profile = {0};
            profile.iSize = sizeof(profile); profile.iProfile = XUI_DOCUMENT_MARKDOWN;
            if (style) snprintf(source, sizeof(source), "**%.*s**%s", (int)split, unit_stems[unit_sample], unit_md_tails[unit_sample]);
            else snprintf(source, sizeof(source), "%.*s**%sXY**", (int)split, unit_stems[unit_sample], unit_stems[unit_sample] + split);
            CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK && xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK &&
                xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK &&
                xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &first) == XUI_OK); xuiDocumentSnapshotRelease(snapshot);
        } else {
            xui_document_transaction transaction; xui_doc_node_desc_t node = {0}; uint64_t unused;
            char prefix[16], suffix[24];
            CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK && xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
            node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
            memcpy(prefix, unit_stems[unit_sample], split); prefix[split] = 0;
            node.iKind = XUI_DOC_TEXT; node.sText = prefix; node.iTextBytes = split; node.tAttributes.iMarks = styles[style];
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &first) == XUI_OK);
            node.sText = ""; node.iTextBytes = 0; node.tAttributes.iMarks = XUI_DOC_BOLD | XUI_DOC_SUPERSCRIPT;
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &unused) == XUI_OK);
            snprintf(suffix, sizeof(suffix), "%sXY", unit_stems[unit_sample] + split);
            node.sText = suffix; node.iTextBytes = strlen(suffix);
            node.tAttributes.iMarks = style == 0 ? XUI_DOC_BOLD : style == 1 ? 0 : styles[style == 2 ? 3 : 2];
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND, &node, &unused) == XUI_OK &&
                xuiDocumentTxnCommit(transaction, NULL) == XUI_OK); xuiDocumentTxnRelease(transaction);
        }
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document;
        desc.tView.tRenderer.iSize = sizeof(desc.tView.tRenderer);
        desc.tView.tRenderer.tFonts = (xui_doc_font_set_t){unit_fonts[0], unit_fonts[1], unit_fonts[0], unit_fonts[1], unit_fonts[0]};
        desc.tView.tRenderer.onFont = unit_font;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 200}) == XUI_OK && xuiInputViewport(context, 320, 200) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK);
        initial.tAnchor = initial.tCaret = doc_grapheme_edit_position(document, first, 0);
        CHECK(xuiDocumentViewSetSelection(editor, &initial) == XUI_OK);
        key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor; key.iKey = XUI_KEY_RIGHT;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK && xuiDocumentViewGetSelection(editor, &moved) == XUI_OK);
        range.tAnchor = initial.tCaret; range.tCaret = moved.tCaret;
        {
            char* copy; uint64_t bytes;
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopyRange(snapshot, &range, &copy, &bytes) == XUI_OK && bytes == length && !memcmp(copy, unit_stems[unit_sample], length));
            xuiDocumentFreeBuffer(copy); xuiDocumentSnapshotRelease(snapshot);
        }
        content = xuiWidgetGetContentRect(editor); caret = xuiEditGetCaretRect(editor);
        if (fabs((double)caret.fX - content.fX - advance) >= .01) fprintf(stderr, "Cross-font editor sample=%u form=%u style=%u advance=%g\n",
            unit_sample, form, style, (double)caret.fX - content.fX);
        CHECK(fabs((double)caret.fX - content.fX - advance) < .01);
        pointer.iSize = sizeof(pointer); pointer.pTarget = editor; pointer.iPointerId = 9; pointer.iPointerType = XUI_POINTER_TYPE_MOUSE;
        pointer.iButton = XUI_POINTER_BUTTON_LEFT; pointer.fX = (float)(content.fX + advance - 1); pointer.fY = caret.fY + caret.fH * .5f;
        CHECK(xuiDocumentViewHitTest(editor, pointer.fX, pointer.fY, &hit) == XUI_OK);
        {
            /* Entity callbacks can produce adjacent Text leaves: the previous
             * leaf end and next leaf start have the same text coordinate. */
            char* copy; uint64_t bytes; xui_doc_range_t interval = {hit, moved.tCaret};
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopyRange(snapshot, &interval, &copy, &bytes) == XUI_OK && !bytes);
            xuiDocumentFreeBuffer(copy); xuiDocumentSnapshotRelease(snapshot);
        }
        pointer.iType = XUI_EVENT_POINTER_DOWN; pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
        CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK); pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButtons = 0;
        CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK && xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK);
        doc_grapheme_edit_plain(document, "XY\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        snprintf(full, sizeof(full), "%sXY\n", unit_stems[unit_sample]); doc_grapheme_edit_plain(document, full);
        if (form) {
            char restored[48]; uint64_t bytes;
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotCopySource(snapshot, restored, sizeof(restored), &bytes) == XUI_OK && !strcmp(restored, source));
            xuiDocumentSnapshotRelease(snapshot);
        }
        initial.tAnchor = initial.tCaret = doc_grapheme_edit_position(document, first, 0);
        CHECK(xuiDocumentViewSetSelection(editor, &initial) == XUI_OK && xuiDispatchEvent(context, &key) == XUI_OK);
        caret = xuiEditGetCaretRect(editor); CHECK(fabs((double)caret.fX - content.fX - advance) < .01);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    xuiDestroy(context); proxy->tProxy.textShape = unit_font_base_shape;
    proxy->tProxy.fontDestroy(&proxy->tProxy, unit_fonts[0]); proxy->tProxy.fontDestroy(&proxy->tProxy, unit_fonts[1]);
    puts("Cross-font grapheme editor: combining/ZWJ/RI, Rich font/script and Markdown strong/entity splits, arrows, pointer, whole-unit backspace and exact undo passed");
}
