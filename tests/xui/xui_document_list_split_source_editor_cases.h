#include "xui_document_list_split_source_samples.h"
static void document_list_split_source_editor(xui_context context)
{
    unsigned sample;
    for (sample = 0; sample < sizeof(document_list_split_source_samples) / sizeof(*document_list_split_source_samples); sample++) {
        xui_doc_desc_t profile = {0}; xui_doc_editor_desc_t desc = {0};
        xui_document document; xui_widget editor; xui_doc_range_t range, selected;
        xui_document_snapshot snapshot; int order;
        profile.iSize = sizeof(profile); profile.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK && xuiDocumentLoadMarkdown(document,
            document_list_split_source_samples[sample].input, strlen(document_list_split_source_samples[sample].input)) == XUI_OK);
        desc.iSize = sizeof(desc); desc.iMode = XUI_DOC_VISUAL;
        desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 240}) == XUI_OK && xuiInputViewport(context, 640, 240) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK);
        range = nested_prefix_editor_find(document, "abcd");
        if (document_list_split_source_samples[sample].offset == 2) range = nested_prefix_editor_find(document, "cd");
        range.tAnchor = range.tCaret = document_list_split_source_samples[sample].offset == 4 ? range.tCaret : range.tAnchor;
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        if (sample & 1) {
            xui_event_t key = {0}; key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor; key.iKey = XUI_KEY_ENTER;
            CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        } else CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ENTER) == XUI_OK);
        CHECK(xuiUpdate(context, .016f) == XUI_OK);
        nested_prefix_editor_source(document, document_list_split_source_samples[sample].expected);
        CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK &&
            xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotComparePositions(snapshot, &selected.tCaret, &selected.tCaret, &order) == XUI_OK && !order);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorInsertText(editor, "Z", 1) == XUI_OK);
        range = nested_prefix_editor_find(document, document_list_split_source_samples[sample].offset == 4 ? "Z" :
            document_list_split_source_samples[sample].offset == 0 ? "Zabcd" : "Zcd"); (void)range;
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        nested_prefix_editor_source(document, document_list_split_source_samples[sample].expected);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        nested_prefix_editor_source(document, document_list_split_source_samples[sample].input);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        nested_prefix_editor_source(document, document_list_split_source_samples[sample].expected);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
            xuiEditGetText(editor) && !strcmp(xuiEditGetText(editor), document_list_split_source_samples[sample].expected));
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("List split source editor: nested/ordered/task/empty/heading/link items, actual Enter, continued caret input, exact history and source-mode projection passed");
}
