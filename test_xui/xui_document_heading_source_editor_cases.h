#include "xui_document_heading_source_samples.h"
static uint64_t heading_source_editor_find(xui_document_snapshot snapshot, uint64_t id)
{
    xui_doc_node_info_t node = {0}; uint64_t i; node.iSize = sizeof(node);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &node) == XUI_OK);
    if (node.iKind == XUI_DOC_HEADING) return id;
    for (i = 0; i < node.iChildCount; i++) {
        uint64_t child, heading; CHECK(xuiDocumentSnapshotGetChild(snapshot, id, i, &child) == XUI_OK);
        heading = heading_source_editor_find(snapshot, child); if (heading) return heading;
    }
    return 0;
}
static void document_heading_source_editor(xui_context context)
{
    unsigned sample;
    for (sample = 0; sample < sizeof(heading_source_samples) / sizeof(*heading_source_samples); sample++) {
        xui_document d; xui_widget editor; xui_doc_desc_t profile = {0}; xui_doc_editor_desc_t desc = {0};
        xui_doc_range_t range = {0}; unsigned command;
        profile.iSize = sizeof(profile); profile.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&profile, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, heading_source_samples[sample].input,
            strlen(heading_source_samples[sample].input)) == XUI_OK);
        desc.iSize = sizeof(desc); desc.iMode = XUI_DOC_VISUAL; desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 240}) == XUI_OK && xuiInputViewport(context, 640, 240) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK);
        if (sample == 19 || sample == 20) {
            range.tAnchor.iSize = sizeof(range.tAnchor);
            xui_document_snapshot snapshot; CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
            range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d); range.tAnchor.iRevision = xuiDocumentGetRevision(d);
            range.tAnchor.iNodeId = heading_source_editor_find(snapshot, 1); range.tAnchor.iKind = XUI_DOC_POSITION_GAP;
            range.tAnchor.iAffinity = XUI_DOC_AFTER; range.tCaret = range.tAnchor; xuiDocumentSnapshotRelease(snapshot);
        } else { range = nested_prefix_editor_find(d, "abcd"); range.tAnchor.iOffset++; range.tCaret = range.tAnchor; }
        command = heading_source_samples[sample].level ? XUI_DOC_EDIT_HEADING_1 + heading_source_samples[sample].level - 1 : XUI_DOC_EDIT_PARAGRAPH;
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK && xuiDocumentEditorExecute(editor, command) == XUI_OK &&
            xuiUpdate(context, .016f) == XUI_OK); nested_prefix_editor_source(d, heading_source_samples[sample].expected);
        if (sample != 19 && sample != 20) {
            CHECK(xuiDocumentEditorInsertText(editor, "Z", 1) == XUI_OK); range = nested_prefix_editor_find(d, "aZbcd"); (void)range;
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); nested_prefix_editor_source(d, heading_source_samples[sample].expected);
        }
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); nested_prefix_editor_source(d, heading_source_samples[sample].input);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK); nested_prefix_editor_source(d, heading_source_samples[sample].expected);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
            xuiEditGetText(editor) && !strcmp(xuiEditGetText(editor), heading_source_samples[sample].expected));
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    puts("Heading source editor: 24 marker edits, continued published caret, exact undo/redo and source-mode projection passed");
}
