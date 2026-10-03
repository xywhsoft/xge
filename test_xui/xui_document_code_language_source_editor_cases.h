#include "xui_document_code_language_source_samples.h"
static uint64_t code_language_editor_find(xui_document_snapshot snapshot, uint64_t id)
{
    xui_doc_node_info_t node = {0}; uint64_t i; node.iSize = sizeof(node);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &node) == XUI_OK);
    if (node.iKind == XUI_DOC_CODE_BLOCK) return id;
    for (i = 0; i < node.iChildCount; i++) {
        uint64_t child, code; CHECK(xuiDocumentSnapshotGetChild(snapshot, id, i, &child) == XUI_OK);
        code = code_language_editor_find(snapshot, child); if (code) return code;
    }
    return 0;
}
static void document_code_language_source_editor(xui_context context)
{
    unsigned sample;
    for (sample = 0; sample < sizeof(code_language_source_samples) / sizeof(*code_language_source_samples); sample++) {
        xui_document d; xui_document_snapshot snapshot; xui_widget editor; xui_doc_desc_t profile = {0};
        xui_doc_editor_desc_t desc = {0}; xui_doc_range_t range; uint64_t code; int order;
        profile.iSize = sizeof(profile); profile.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&profile, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, code_language_source_samples[sample].input,
            strlen(code_language_source_samples[sample].input)) == XUI_OK && xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        code = code_language_editor_find(snapshot, 1); CHECK(code); xuiDocumentSnapshotRelease(snapshot);
        desc.iSize = sizeof(desc); desc.iMode = XUI_DOC_VISUAL; desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 240}) == XUI_OK && xuiInputViewport(context, 640, 240) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK);
        range = nested_prefix_editor_find(d, "abc"); range.tAnchor.iOffset++; range.tCaret = range.tAnchor;
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK &&
            xuiDocumentEditorSetCodeBlockLanguage(editor, code, code_language_source_samples[sample].language) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK);
        nested_prefix_editor_source(d, code_language_source_samples[sample].expected);
        CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK && xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotComparePositions(snapshot, &range.tCaret, &range.tCaret, &order) == XUI_OK && !order && range.tCaret.iNodeId == code);
        xuiDocumentSnapshotRelease(snapshot); CHECK(xuiDocumentEditorInsertText(editor, "Z", 1) == XUI_OK);
        range = nested_prefix_editor_find(d, "aZbc"); (void)range;
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); nested_prefix_editor_source(d, code_language_source_samples[sample].expected);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); nested_prefix_editor_source(d, code_language_source_samples[sample].input);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK); nested_prefix_editor_source(d, code_language_source_samples[sample].expected);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
            xuiEditGetText(editor) && !strcmp(xuiEditGetText(editor), code_language_source_samples[sample].expected));
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    puts("Code language source editor: 13 field patches, published caret/body continuation, exact undo/redo and source-mode projection passed");
}
