#include "xui_document_nested_prefix_samples.h"
static xui_doc_range_t nested_prefix_editor_find(xui_document document, const char* text)
{
    xui_document_snapshot snapshot; xui_doc_range_t range; uint64_t count;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, text, strlen(text), NULL, &range, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot); return range;
}
static void nested_prefix_editor_source(xui_document document, const char* expected)
{
    xui_document_snapshot snapshot; char actual[1024]; uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotCopySource(snapshot, actual, sizeof(actual), &bytes) == XUI_OK &&
        bytes == strlen(expected) && !memcmp(actual, expected, (size_t)bytes));
    xuiDocumentSnapshotRelease(snapshot);
}
static void document_nested_prefix_editor(xui_context context)
{
    unsigned sample;
    for (sample = 0; sample < sizeof(document_nested_prefix_samples) / sizeof(*document_nested_prefix_samples); sample++) {
        xui_doc_desc_t profile = {0}; xui_doc_editor_desc_t desc = {0};
        xui_document document; xui_document_snapshot snapshot; xui_widget editor;
        xui_doc_range_t selected, range, gap; char* copied; uint64_t bytes;
        int result;
        profile.iSize = sizeof(profile); profile.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&profile, &document) == XUI_OK && xuiDocumentLoadMarkdown(document,
            document_nested_prefix_samples[sample].input, strlen(document_nested_prefix_samples[sample].input)) == XUI_OK);
        desc.iSize = sizeof(desc); desc.iMode = XUI_DOC_VISUAL;
        desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK && xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 240}) == XUI_OK &&
            xuiInputViewport(context, 640, 240) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        range = nested_prefix_editor_find(document, document_nested_prefix_samples[sample].merge ? "ab" : "abcd");
        if (document_nested_prefix_samples[sample].merge) { range.tAnchor = range.tCaret; range.tCaret = nested_prefix_editor_find(document, "cd").tAnchor; }
        else { range.tAnchor.iOffset += 2; range.tCaret = range.tAnchor; }
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        if (sample == 6) {
            /* Keep this fixture a paragraph split; task-item Enter is covered
             * separately by the list-item source and editor fixtures. */
            range = nested_prefix_editor_find(document, "bc");
            CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
            result = xuiDocumentEditorInsertText(editor, "b\nc", 3);
        } else result = document_nested_prefix_samples[sample].merge ? xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) :
            xuiDocumentEditorInsertText(editor, "\n", 1);
        if (result != XUI_OK) fprintf(stderr, "Nested prefix editor sample=%u result=%d\n", sample, result);
        CHECK(result == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        nested_prefix_editor_source(document, document_nested_prefix_samples[sample].expected);
        CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK);
        gap.tAnchor = selected.tCaret; gap.tCaret = nested_prefix_editor_find(document, sample == 6 ? "d" : "cd").tAnchor;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotCopyRange(snapshot, &gap, &copied, &bytes) == XUI_OK && !bytes);
        xuiDocumentFreeBuffer(copied); xuiDocumentSnapshotRelease(snapshot);
        /* Continue through the published caret, then undo that edit before
         * testing history and source/visual mode projections. */
        CHECK(xuiDocumentEditorInsertText(editor, "Z", 1) == XUI_OK);
        range = nested_prefix_editor_find(document, sample == 6 ? "cZd" : "Zcd"); (void)range;
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        nested_prefix_editor_source(document, document_nested_prefix_samples[sample].expected);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        nested_prefix_editor_source(document, document_nested_prefix_samples[sample].input);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        nested_prefix_editor_source(document, document_nested_prefix_samples[sample].expected);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK && selected.tCaret.iKind == XUI_DOC_POSITION_SOURCE);
        CHECK(xuiEditGetText(editor) && !strcmp(xuiEditGetText(editor), document_nested_prefix_samples[sample].expected));
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("Nested prefix editor: visual split/merge, published caret continuation, exact definitions/Undo/Redo and source-mode projection passed");
}
