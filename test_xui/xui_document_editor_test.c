#include "../xui_document_ui.h"
#include "../xge.h"
#include "../src/xui_internal.h"
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
static void deletion_command_boundary_cases(xui_context context)
{
    xui_doc_editor_desc_t desc = {0};
    xui_doc_desc_t md = {0};
    xui_doc_command_state_t state = {0};
    xui_doc_range_t selection;
    xui_document_snapshot snapshot;
    xui_document document;
    xui_widget editor;
    uint64_t found;
    int profile;
    for (profile = 0; profile < 4; profile++) {
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(profile == 3 ? NULL : &md, &document) == XUI_OK);
        if (profile != 3)
            CHECK(xuiDocumentLoadMarkdown(document, "ab\n", 3) == XUI_OK);
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
        desc.tView.pDocument = document;
        desc.iMode = profile == 0 ? XUI_DOC_SOURCE_TEXT :
            profile == 1 ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 160}) == XUI_OK);
        if (profile == 3) CHECK(xuiDocumentEditorInsertText(editor, "ab", 2) == XUI_OK);
        if (profile < 2)
            CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
        else {
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
            CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                "ab", 2, NULL, &selection, 1, &found) == XUI_OK && found == 1);
            xuiDocumentSnapshotRelease(snapshot);
        }
        state.iSize = sizeof(state);
        selection.tAnchor.iOffset = selection.tCaret.iOffset = 0;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BACKSPACE, &state) == XUI_OK &&
            !state.bEnabled && state.iDisabledReason == XUI_ERROR_INVALID_STATE);
        CHECK(!xuiDocumentEditorCanExecute(editor, XUI_DOC_EDIT_BACKSPACE));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_ERROR_INVALID_STATE);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_DELETE, &state) == XUI_OK && state.bEnabled);
        selection.tAnchor.iOffset = selection.tCaret.iOffset = profile < 2 ? 3 : 2;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BACKSPACE, &state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_DELETE, &state) == XUI_OK &&
            !state.bEnabled && state.iDisabledReason == XUI_ERROR_INVALID_STATE);
        CHECK(!xuiDocumentEditorCanExecute(editor, XUI_DOC_EDIT_DELETE));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_DELETE) == XUI_ERROR_INVALID_STATE);
        selection.tAnchor.iOffset = 0;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BACKSPACE, &state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_DELETE, &state) == XUI_OK && state.bEnabled);
        selection.tAnchor.iOffset = profile < 2 ? 3 : 2;
        selection.tCaret.iOffset = 0;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BACKSPACE, &state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_DELETE, &state) == XUI_OK && state.bEnabled);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    for (profile = 0; profile < 3; profile++) {
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(profile == 2 ? NULL : &md, &document) == XUI_OK);
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
        desc.tView.pDocument = document;
        desc.iMode = profile == 0 ? XUI_DOC_SOURCE_TEXT :
            profile == 1 ? XUI_DOC_LIVE_MARKDOWN : XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BACKSPACE, &state) == XUI_OK && !state.bEnabled);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_DELETE, &state) == XUI_OK && !state.bEnabled);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    {
        xui_document_transaction transaction;
        xui_doc_node_desc_t node = {0};
        xui_doc_node_id paragraph, empty, body;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
        CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND,
            &node, &paragraph) == XUI_OK);
        node.iKind = XUI_DOC_TEXT; node.sText = ""; node.iTextBytes = 0;
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND,
            &node, &empty) == XUI_OK);
        node.sText = "ab"; node.iTextBytes = 2;
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND,
            &node, &body) == XUI_OK);
        node.sText = ""; node.iTextBytes = 0;
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND,
            &node, &empty) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
        desc.tView.pDocument = document; desc.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
        selection.tAnchor.iNodeId = selection.tCaret.iNodeId = body;
        selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_TEXT;
        selection.tAnchor.iOffset = selection.tCaret.iOffset = 0;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BACKSPACE, &state) == XUI_OK &&
            !state.bEnabled);
        selection.tAnchor.iOffset = selection.tCaret.iOffset = 2;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_DELETE, &state) == XUI_OK &&
            !state.bEnabled);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("DocumentEditor deletion command boundaries: SOURCE/LIVE/VISUAL, Rich/Markdown, empty documents/nodes and bidirectional selections passed");
}
static void send_text(xui_context context, xui_widget w, const char* text, int ime, int active)
{
    xui_event_t e = {0}; e.iSize = sizeof(e); e.iType = ime ? XUI_EVENT_IME_COMPOSITION : XUI_EVENT_TEXT; e.pTarget = w;
    strcpy(e.sText, text); e.iTextSize = (int)strlen(text); e.bCompositionActive = active; e.iCompositionCursor = e.iTextSize;
    CHECK(xuiDispatchEvent(context, &e) == XUI_OK);
}
#include "xui_document_async_editor_cases.h"
#include "xui_document_ordered_editor_cases.h"
#include "xui_document_group_editor_cases.h"
#include "xui_document_source_editor_cases.h"
static xui_doc_node_id editor_image_find(xui_document_snapshot snapshot, xui_doc_node_id parent)
{
    xui_doc_node_info_t info = {0}; xui_doc_node_id child, found; uint64_t i;
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, parent, &info) == XUI_OK);
    if (info.iKind == XUI_DOC_IMAGE) return parent;
    for (i = 0; i < info.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(snapshot, parent, i, &child) == XUI_OK);
        found = editor_image_find(snapshot, child); if (found) return found;
    }
    return 0;
}
static void embedded_editor_cases(xui_context context, xui_surface target)
{
    static const struct { float x, y; uint64_t offset; } clicks[] = {
        {220, 8, 5}, {220, 100, 5}, {2, 100, 0}
    };
    xui_document document; xui_document_transaction txn; xui_doc_node_desc_t node = {0};
    xui_doc_editor_desc_t editor_desc = {0}; xui_doc_view_desc_t view_desc = {0};
    xui_doc_range_t selection; xui_doc_rect_t size; xui_rect_t world;
    xui_event_t pointer = {0};
    xui_widget root, host, editor, sibling; xui_doc_node_id paragraph, body;
    xui_rect_i_t damage = {0, 0, 640, 480}; int exact; size_t i;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(txn, 1, XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
    node.iKind = XUI_DOC_TEXT; node.sText = "alpha"; node.iTextBytes = 5;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND, &node, &body) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiInputViewport(context, 640, 480) == XUI_OK &&
        xuiWidgetCreate(context, &root) == XUI_OK && xuiSetRootWidget(context, root) == XUI_OK &&
        xuiWidgetSetRect(root, (xui_rect_t){0, 0, 640, 480}) == XUI_OK);
    CHECK(xuiWidgetCreate(context, &host) == XUI_OK && xuiWidgetAddChild(root, host) == XUI_OK &&
        xuiWidgetSetRect(host, (xui_rect_t){20, 30, 320, 180}) == XUI_OK);
    editor_desc.iSize = sizeof(editor_desc); editor_desc.tView.iSize = sizeof(editor_desc.tView);
    editor_desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK &&
        xuiWidgetAddChild(host, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){10, 15, 240, 120}) == XUI_OK);
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &sibling) == XUI_OK &&
        xuiWidgetAddChild(root, sibling) == XUI_OK &&
        xuiWidgetSetRect(sibling, (xui_rect_t){360, 40, 240, 120}) == XUI_OK);
    CHECK(xuiLayout(context) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
        xuiRender(context, target, &damage, 1) == XUI_OK);
    world = xuiWidgetGetWorldRect(editor);
    CHECK(world.fX == 30 && world.fY == 45 && world.fW == 240 && world.fH == 120);
    pointer.iSize = sizeof(pointer); pointer.iType = XUI_EVENT_POINTER_DOWN;
    pointer.pTarget = editor; pointer.fX = world.fX + 4; pointer.fY = world.fY + 8;
    pointer.iPointerId = 27; pointer.iPointerType = XUI_POINTER_TYPE_MOUSE;
    pointer.iButton = pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButtons = 0;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    CHECK(xuiGetFocusWidget(context) == editor &&
        xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        selection.tCaret.iNodeId == body);
    for (i = 0; i < sizeof(clicks) / sizeof(clicks[0]); i++) {
        pointer.iType = XUI_EVENT_POINTER_DOWN; pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
        pointer.fX = world.fX + clicks[i].x; pointer.fY = world.fY + clicks[i].y;
        CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
        pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButtons = 0;
        CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
        CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
            selection.tCaret.iNodeId == body && selection.tCaret.iOffset == clicks[i].offset);
    }
    memset(&selection, 0, sizeof(selection)); selection.tAnchor.iSize = sizeof(selection.tAnchor);
    selection.tAnchor.iDocumentId = xuiDocumentGetIdentity(document);
    selection.tAnchor.iRevision = xuiDocumentGetRevision(document);
    selection.tAnchor.iNodeId = body; selection.tAnchor.iKind = XUI_DOC_POSITION_TEXT;
    selection.tAnchor.iOffset = 5; selection.tCaret = selection.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "!", 1) == XUI_OK);
    plain(document, "alpha!\n");
    CHECK(xuiDocumentViewGetContentSize(sibling, &size, &exact) == XUI_OK && size.height > 0);
    CHECK(xuiRender(context, target, &damage, 1) == XUI_OK);
    xuiWidgetDestroy(host);
    CHECK(xuiGetFocusWidget(context) != editor && xuiDocumentViewGetDocument(sibling) == document);
    CHECK(xuiDocumentViewGetContentSize(sibling, &size, &exact) == XUI_OK && size.height > 0);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK); plain(document, "alpha\n");
    CHECK(xuiRender(context, target, &damage, 1) == XUI_OK);
    xuiWidgetDestroy(root); xuiDocumentRelease(document);
    puts("Embedded DocumentEditor: nested geometry, focus, shared View updates and subtree teardown passed");
}
static void image_editor_cases(xui_context context)
{
    xui_doc_editor_desc_t desc = {0}; xui_doc_image_desc_t image = {0};
    xui_document d; xui_widget editor; xui_document_snapshot snapshot;
    xui_doc_node_info_t info = {0}; xui_doc_range_t selection; xui_doc_node_id id;
    char source_text[1024]; uint64_t bytes, found;
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.iMode = XUI_DOC_VISUAL;
    image.iSize = sizeof(image); image.sResource = "editor.image";
    image.sAlt = "one"; image.iAltBytes = 3; image.fWidth = 90; image.fHeight = 45;
    image.sLinkTarget = "/editor-link"; image.sLinkTitle = "Editor link";
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK); desc.tView.pDocument = d;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentEditorInsertImage(editor, &image) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    id = editor_image_find(snapshot, 1); CHECK(id != 0); xuiDocumentSnapshotRelease(snapshot);
    image.sResource = "editor.updated"; image.sAlt = "two"; image.iAltBytes = 3;
    image.fWidth = 60; image.fHeight = 30;
    image.sLinkTarget = "/updated-link"; image.sLinkTitle = "Updated link";
    CHECK(xuiDocumentEditorUpdateImage(editor, id, &image) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &info) == XUI_OK &&
        !strcmp(info.sResource, "editor.updated") && info.tAttributes.fWidth == 60 &&
        !strcmp(info.sLinkTarget, "/updated-link") &&
        !strcmp(info.sLinkTitle, "Updated link"));
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &info) == XUI_OK &&
        !strcmp(info.sResource, "editor.image") &&
        !strcmp(info.sLinkTarget, "/editor-link"));
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(editor_image_find(snapshot, 1) == 0); xuiDocumentSnapshotRelease(snapshot);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);

    {
        xui_doc_desc_t md = {0}; md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK);
    }
    CHECK(xuiDocumentLoadMarkdown(d, "hello\n", 6) == XUI_OK);
    desc.tView.pDocument = d; image.sResource = "markdown.image";
    image.sLinkTarget = "/markdown-link"; image.sLinkTitle = "Markdown link";
    image.sAlt = " inline\xC2\xA0"; image.iAltBytes = strlen(image.sAlt);
    image.fWidth = image.fHeight = 0;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "hello", 5,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = selection.tCaret; selection.tAnchor.iOffset = 2;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorInsertImage(editor, &image) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    id = editor_image_find(snapshot, 1); CHECK(id != 0);
    CHECK(xuiDocumentSnapshotCopySource(snapshot, source_text, sizeof(source_text), &bytes) == XUI_OK &&
        strstr(source_text, "[![&#32;inline&#160;]") != NULL &&
        strstr(source_text, "markdown\\-link") != NULL);
    xuiDocumentSnapshotRelease(snapshot);
    image.sResource = "markdown.updated"; image.sAlt = " changed";
    image.iAltBytes = strlen(image.sAlt);
    CHECK(xuiDocumentEditorUpdateImage(editor, id, &image) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(snapshot, source_text, sizeof(source_text), &bytes) == XUI_OK &&
        strstr(source_text, "[![&#32;changed]") != NULL);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &info) == XUI_OK &&
        !strcmp(info.sResource, "markdown.image"));
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    CHECK(xuiDocumentEditorInsertImage(editor, &image) == XUI_ERROR_UNSUPPORTED);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentEditorUpdateImage(editor, id, &image) == XUI_ERROR_UNSUPPORTED);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    puts("Document image Editor: rich/Markdown insert, update, Undo and mode/read-only policy passed");
}
static void object_selection_cases(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy)
{
    const uint32_t selection_color = XUI_COLOR_RGBA(44, 132, 241, 111);
    xui_doc_editor_desc_t desc = {0}; xui_doc_node_desc_t node = {0};
    xui_document document; xui_document_transaction txn;
    xui_document_snapshot snapshot; xui_widget editor;
    xui_doc_range_t range; xui_doc_position_t hit;
    xui_event_t pointer = {0}; xui_rect_t world;
    xui_rect_i_t damage = {0, 0, 640, 480};
    uint64_t paragraph, left, image, right, math, mermaid, html;
    int x, y, found = 0;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(txn, 1, XUI_DOCUMENT_APPEND,
        &node, &paragraph) == XUI_OK);
    node.iKind = XUI_DOC_TEXT; node.sText = "L"; node.iTextBytes = 1;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
        &node, &left) == XUI_OK);
    node.iKind = XUI_DOC_IMAGE; node.sText = ""; node.iTextBytes = 0;
    node.sResource = "empty-alt-image";
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
        &node, &image) == XUI_OK);
    node.iKind = XUI_DOC_TEXT; node.sText = "R"; node.iTextBytes = 1;
    node.sResource = NULL;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
        &node, &right) == XUI_OK);
    node.iKind = XUI_DOC_MATH; node.sText = "x+y"; node.iTextBytes = 3;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
        &node, &math) == XUI_OK);
    node.iKind = XUI_DOC_DIAGRAM; node.sText = "graph TD;A-->B";
    node.iTextBytes = strlen(node.sText);
    CHECK(xuiDocumentTxnInsertNode(txn, 1, XUI_DOCUMENT_APPEND,
        &node, &mermaid) == XUI_OK);
    node.iKind = XUI_DOC_HTML; node.sText = "<div>content</div>";
    node.iTextBytes = strlen(node.sText);
    CHECK(xuiDocumentTxnInsertNode(txn, 1, XUI_DOCUMENT_APPEND,
        &node, &html) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = document; desc.iMode = XUI_DOC_VISUAL;
    desc.tView.iSelectionColor = selection_color;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 180}) == XUI_OK);
    CHECK(xuiLayout(context) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
        xuiRender(context, target, &damage, 1) == XUI_OK);
    CHECK(xuiDocumentViewSelectObject(editor, left) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiDocumentViewSelectObject(editor, image) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == paragraph && range.tAnchor.iKind == XUI_DOC_POSITION_GAP &&
        range.tAnchor.iOffset == 1 && range.tCaret.iNodeId == paragraph &&
        range.tCaret.iOffset == 2);
    {
        xui_document_renderer renderer; xui_draw_context draw;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentRendererCreate(context, NULL, &renderer) == XUI_OK &&
            xuiDocumentRendererSetSnapshot(renderer, snapshot, NULL) == XUI_OK &&
            xuiDocumentRendererLayout(renderer, 320, 0, 180) == XUI_OK);
        xuiTestSurfaceReset(target);
        CHECK(proxy->tProxy.drawBegin(&proxy->tProxy, &draw, target) == XUI_OK);
        CHECK(xuiDocumentRendererDraw(renderer, draw, 0, 0,
            (xui_rect_t){0, 0, 320, 180}, &range, selection_color) == XUI_OK);
        CHECK(proxy->tProxy.drawEnd(&proxy->tProxy, draw) == XUI_OK &&
            xuiTestSurfaceGetRectFillColorCount(target, selection_color) > 0);
        xuiDocumentRendererRelease(renderer); xuiDocumentSnapshotRelease(snapshot);
    }
    for (y = 2; y < 90 && !found; y += 4) for (x = 2; x < 300 && !found; x += 4)
        if (xuiDocumentViewHitTest(editor, x, y, &hit) == XUI_OK &&
            hit.iNodeId == image) found = 1;
    CHECK(found);
    world = xuiWidgetGetWorldRect(editor);
    pointer.iSize = sizeof(pointer); pointer.pTarget = editor;
    pointer.iPointerId = 72; pointer.iPointerType = XUI_POINTER_TYPE_MOUSE;
    pointer.iButton = pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
    pointer.fX = world.fX + x - 4; pointer.fY = world.fY + y - 4;
    pointer.iType = XUI_EVENT_POINTER_DOWN;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButtons = 0;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == paragraph && range.tAnchor.iOffset == 1 &&
        range.tCaret.iNodeId == paragraph && range.tCaret.iOffset == 2);
    proxy->iClipboardDocumentSize = proxy->iClipboardHtmlSize = 0;
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_COPY) == XUI_OK &&
        proxy->iClipboardDocumentSize > 0 && proxy->iClipboardHtmlSize > 0);
    range.tCaret = range.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    {
        xui_event_t key = {0}; key.iSize = sizeof(key);
        key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor;
        key.iKey = XUI_KEY_RIGHT; key.iModifiers = XUI_MOD_SHIFT;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    }
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == paragraph && range.tAnchor.iOffset == 1 &&
        range.tCaret.iNodeId == right && range.tCaret.iOffset == 0);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_DELETE) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    { xui_doc_node_info_t info = {0}; info.iSize = sizeof(info);
      CHECK(xuiDocumentSnapshotGetNode(snapshot, image, &info) == XUI_ERROR_NOT_FOUND); }
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentViewSelectObject(editor, image) == XUI_OK &&
        xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CUT) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    { xui_doc_node_info_t info = {0}; info.iSize = sizeof(info);
      CHECK(xuiDocumentSnapshotGetNode(snapshot, image, &info) == XUI_ERROR_NOT_FOUND); }
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentViewSelectObject(editor, math) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iOffset == 3 && range.tCaret.iOffset == 4);
    range.tCaret = range.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    {
        xui_event_t key = {0}; key.iSize = sizeof(key);
        key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor;
        key.iKey = XUI_KEY_RIGHT; key.iModifiers = XUI_MOD_SHIFT;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    }
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_DELETE) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    { xui_doc_node_info_t info = {0}; info.iSize = sizeof(info);
      CHECK(xuiDocumentSnapshotGetNode(snapshot, math, &info) == XUI_ERROR_NOT_FOUND); }
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentViewSelectObject(editor, mermaid) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == 1 && range.tAnchor.iOffset == 1 &&
        range.tCaret.iOffset == 2);
    CHECK(xuiDocumentViewSelectObject(editor, html) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == 1 && range.tAnchor.iOffset == 2 &&
        range.tCaret.iOffset == 3);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    {
        xui_doc_desc_t md = {0};
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &document) == XUI_OK);
    }
    CHECK(xuiDocumentLoadMarkdown(document, "A ![](/img) B\n",
        strlen("A ![](/img) B\n")) == XUI_OK);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 180}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    image = editor_image_find(snapshot, 1); CHECK(image != 0);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewSelectObject(editor, image) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iKind == XUI_DOC_POSITION_GAP &&
        range.tAnchor.iNodeId == range.tCaret.iNodeId &&
        range.tCaret.iOffset == range.tAnchor.iOffset + 1);
    range.tCaret = range.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    {
        xui_event_t key = {0}; key.iSize = sizeof(key);
        key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor;
        key.iKey = XUI_KEY_RIGHT; key.iModifiers = XUI_MOD_SHIFT;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    }
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_DELETE) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        editor_image_find(snapshot, 1) == 0);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, "A ![](/img) B\n");
    CHECK(xuiDocumentViewSelectObject(editor, image) == XUI_OK &&
        xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CUT) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        editor_image_find(snapshot, 1) == 0);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, "A ![](/img) B\n");
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
        xuiDocumentViewSelectObject(editor, image) == XUI_ERROR_UNSUPPORTED);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    puts("Document object selection: empty-alt image pointer/API/keyboard, highlight, native copy, Rich/Markdown cut/delete and Undo, formula/Mermaid/HTML and mode policy passed");
}
static void clear_formatting_editor_cases(xui_context context)
{
    xui_doc_editor_desc_t desc = {0}; xui_doc_command_state_t command = {0};
    xui_doc_editor_text_style_state_t style_state = {0};
    xui_doc_text_style_t style = {0}; xui_doc_range_t selection;
    xui_doc_node_info_t info = {0}; xui_document d; xui_widget editor;
    xui_document_snapshot snapshot; uint64_t found, revision;
    uint32_t common, mixed;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = d; desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "abcde", 5) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcde", 5,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorSetMarks(editor, XUI_DOC_BOLD, 0) == XUI_OK);
    style.iSize = sizeof(style); style.iTextColor = 0x335577ffu;
    style.fFontSize = 22; strcpy(style.sFontFamily, "Test family");
    CHECK(xuiDocumentEditorSetTextStyle(editor,
        XUI_DOC_TEXT_STYLE_COLOR | XUI_DOC_TEXT_STYLE_FONT_SIZE |
        XUI_DOC_TEXT_STYLE_FONT_FAMILY, &style) == XUI_OK);
    CHECK(xuiDocumentEditorSetLink(editor, "/target", "Title") == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "bc", 2,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    command.iSize = sizeof(command);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_CLEAR_FORMATTING,
        &command) == XUI_OK && command.bEnabled);
    revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK);
    CHECK(xuiDocumentGetRevision(d) == revision + 1);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "bc", 2,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        !info.tAttributes.iMarks && !info.tAttributes.iTextColor &&
        !info.tAttributes.fFontSize && !*info.tAttributes.sFontFamily && !*info.sResource);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "bc", 2,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotQueryMarks(snapshot, &selection, &common, &mixed) == XUI_OK &&
        (common & (XUI_DOC_BOLD | XUI_DOC_LINK)) == (XUI_DOC_BOLD | XUI_DOC_LINK));
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_CLEAR_FORMATTING,
        &command) == XUI_OK && !command.bEnabled && command.iDisabledReason == XUI_ERROR_INVALID_STATE);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_ERROR_INVALID_STATE);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    selection.tAnchor = selection.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_CLEAR_FORMATTING,
        &command) == XUI_OK && command.bEnabled);
    revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK &&
        xuiDocumentGetRevision(d) == revision);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_CLEAR_FORMATTING,
        &command) == XUI_OK && command.bActive);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BOLD,
        &command) == XUI_OK && !command.bActive);
    style_state.iSize = sizeof(style_state);
    CHECK(xuiDocumentEditorQueryTextStyle(editor, &style_state) == XUI_OK &&
        style_state.bEnabled && !style_state.tQuery.tStyle.iTextColor &&
        !style_state.tQuery.tStyle.fFontSize &&
        !*style_state.tQuery.tStyle.sFontFamily);
    CHECK(xuiDocumentEditorInsertText(editor, "X", 1) == XUI_OK &&
        xuiDocumentGetRevision(d) == revision + 1);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "X", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        !info.tAttributes.iMarks && !info.tAttributes.iTextColor &&
        !info.tAttributes.fFontSize && !*info.tAttributes.sFontFamily && !*info.sResource);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "b", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        (info.tAttributes.iMarks & (XUI_DOC_BOLD | XUI_DOC_LINK)) ==
        (XUI_DOC_BOLD | XUI_DOC_LINK));
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "bc", 2,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = selection.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK);
    CHECK(xuiDocumentEditorSetMarks(editor, XUI_DOC_ITALIC, 0) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_ITALIC,
        &command) == XUI_OK && command.bActive);
    CHECK(xuiDocumentEditorInsertText(editor, "Y", 1) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "Y", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        info.tAttributes.iMarks == XUI_DOC_ITALIC && !*info.sResource);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "bc", 2,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = selection.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "U\nV", 3) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "U", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        !info.tAttributes.iMarks && !*info.sResource);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "V", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        !info.tAttributes.iMarks && !*info.sResource);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "b", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        (info.tAttributes.iMarks & (XUI_DOC_BOLD | XUI_DOC_LINK)) ==
        (XUI_DOC_BOLD | XUI_DOC_LINK));
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "bc", 2,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = selection.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "\n", 1) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_CLEAR_FORMATTING,
        &command) == XUI_OK && command.bActive);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "bc", 2,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = selection.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK);
    CHECK(xuiDocumentEditorSetMarks(editor, XUI_DOC_ITALIC, 0) == XUI_OK);
    style.iTextColor = 0x44aa66ffu;
    CHECK(xuiDocumentEditorSetTextStyle(editor, XUI_DOC_TEXT_STYLE_COLOR, &style) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "U\nV", 3) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "U", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        info.tAttributes.iMarks == XUI_DOC_ITALIC &&
        info.tAttributes.iTextColor == style.iTextColor && !*info.sResource);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "V", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        info.tAttributes.iMarks == XUI_DOC_ITALIC &&
        info.tAttributes.iTextColor == style.iTextColor && !*info.sResource);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "d", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        (info.tAttributes.iMarks & (XUI_DOC_BOLD | XUI_DOC_LINK)) ==
        (XUI_DOC_BOLD | XUI_DOC_LINK) &&
        !(info.tAttributes.iMarks & XUI_DOC_ITALIC) &&
        info.tAttributes.iTextColor == 0x335577ffu);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "bc", 2,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = selection.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorSetMarks(editor, XUI_DOC_ITALIC, 0) == XUI_OK);
    style.iTextColor = 0x1188ccffu;
    CHECK(xuiDocumentEditorSetTextStyle(editor, XUI_DOC_TEXT_STYLE_COLOR, &style) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "U\nV", 3) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "U", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        (info.tAttributes.iMarks & XUI_DOC_ITALIC) &&
        info.tAttributes.iTextColor == style.iTextColor);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "V", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        (info.tAttributes.iMarks & XUI_DOC_ITALIC) &&
        info.tAttributes.iTextColor == style.iTextColor);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);

    { xui_doc_desc_t md = {0};
      md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
      CHECK(xuiDocumentCreate(&md, &d) == XUI_OK &&
          xuiDocumentLoadMarkdown(d, "PRE **[word](/target)** POST\n", 29) == XUI_OK); }
    desc.tView.pDocument = d;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "word", 4,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_CLEAR_FORMATTING,
        &command) == XUI_OK && command.bEnabled);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK);
    source(d, "PRE word POST\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "PRE **[word](/target)** POST\n");
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "word", 4,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tCaret = selection.tAnchor;
    selection.tCaret.iOffset += 2;
    selection.tAnchor = selection.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_CLEAR_FORMATTING,
        &command) == XUI_OK && command.bEnabled);
    revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK &&
        xuiDocumentGetRevision(d) == revision);
    CHECK(xuiDocumentEditorInsertText(editor, "X", 1) == XUI_OK &&
        xuiDocumentGetRevision(d) == revision + 1);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "X", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        !info.tAttributes.iMarks && !*info.sResource);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "wo", 2,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotQueryMarks(snapshot, &selection, &common, &mixed) == XUI_OK &&
        (common & (XUI_DOC_BOLD | XUI_DOC_LINK)) == (XUI_DOC_BOLD | XUI_DOC_LINK));
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "PRE **[word](/target)** POST\n");
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "word", 4,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = selection.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "\n", 1) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_CLEAR_FORMATTING,
        &command) == XUI_OK && command.bActive);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "PRE **[word](/target)** POST\n");
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "word", 4,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = selection.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "U\nV", 3) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "U", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        !info.tAttributes.iMarks && !*info.sResource);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "V", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        !info.tAttributes.iMarks && !*info.sResource);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "PRE **[word](/target)** POST\n");
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "word", 4,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = selection.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK);
    CHECK(xuiDocumentEditorSetMarks(editor, XUI_DOC_ITALIC, 0) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "U\nV", 3) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "U", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        info.tAttributes.iMarks == XUI_DOC_ITALIC && !*info.sResource);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "V", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, selection.tAnchor.iNodeId, &info) == XUI_OK &&
        info.tAttributes.iMarks == XUI_DOC_ITALIC && !*info.sResource);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "POST", 4,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotQueryMarks(snapshot, &selection, &common, &mixed) == XUI_OK &&
        !(common & XUI_DOC_ITALIC));
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "PRE **[word](/target)** POST\n");
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_CLEAR_FORMATTING,
        &command) == XUI_OK && !command.bEnabled);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    puts("DocumentEditor clear formatting: Rich/Markdown selection, pending caret style, multiline input, one Undo and mode policy passed");
}

typedef struct editor_fail_allocator { long remaining; unsigned live; } editor_fail_allocator;
static void* editor_failing_alloc(void* user, size_t bytes)
{
    editor_fail_allocator* a = user; void* p;
    if (!a->remaining) return NULL;
    if (a->remaining > 0) a->remaining--;
    p = malloc(bytes); if (p) a->live++; return p;
}
static void editor_failing_free(void* user, void* pointer)
{
    editor_fail_allocator* a = user;
    CHECK(a->live > 0); a->live--; free(pointer);
}
static void clear_formatting_editor_failures(xui_context context)
{
    unsigned mode;
    for (mode = 0; mode < 2; mode++) {
        long point; int success = 0;
        for (point = 0; point < 4096 && !success; point++) {
            editor_fail_allocator allocator = {-1, 0};
            xui_doc_desc_t document_desc = {0}; xui_doc_editor_desc_t editor_desc = {0};
            xui_doc_node_desc_t node = {0}; xui_doc_text_style_t style = {0};
            xui_doc_stats_t before = {0}, after = {0};
            xui_doc_range_t selection; xui_document d; xui_document_transaction t = NULL;
            xui_document_snapshot snapshot; xui_widget editor; uint64_t paragraph, id, found, revision;
            int can_undo, result;
            document_desc.iSize = sizeof(document_desc);
            document_desc.iProfile = mode ? XUI_DOCUMENT_MARKDOWN : XUI_DOCUMENT_RICH;
            document_desc.onAlloc = editor_failing_alloc;
            document_desc.onFree = editor_failing_free;
            document_desc.pAllocatorUser = &allocator;
            CHECK(xuiDocumentCreate(&document_desc, &d) == XUI_OK);
            if (mode) {
                const char* original = "PRE **[word](/target)** POST\n";
                CHECK(xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
            } else {
                CHECK(xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
                node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
                CHECK(xuiDocumentTxnInsertNode(t, 1, XUI_DOCUMENT_APPEND,
                    &node, &paragraph) == XUI_OK);
                node.iKind = XUI_DOC_TEXT; node.sText = "abcde"; node.iTextBytes = 5;
                node.sResource = "/target"; node.tAttributes.iMarks = XUI_DOC_BOLD | XUI_DOC_LINK;
                node.tAttributes.iTextColor = 0x335577ffu;
                CHECK(xuiDocumentTxnInsertNode(t, paragraph, XUI_DOCUMENT_APPEND,
                    &node, &id) == XUI_OK);
                CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK);
                xuiDocumentTxnRelease(t); t = NULL;
            }
            editor_desc.iSize = sizeof(editor_desc);
            editor_desc.tView.iSize = sizeof(editor_desc.tView);
            editor_desc.tView.pDocument = d; editor_desc.iMode = XUI_DOC_VISUAL;
            CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK);
            CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
            CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
            CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                mode ? "word" : "bc", mode ? 4 : 2,
                NULL, &selection, 1, &found) == XUI_OK && found == 1);
            xuiDocumentSnapshotRelease(snapshot);
            if (mode) { selection.tCaret = selection.tAnchor; selection.tCaret.iOffset += 2; }
            selection.tAnchor = selection.tCaret;
            CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CLEAR_FORMATTING) == XUI_OK);
            CHECK(xuiDocumentEditorSetMarks(editor, XUI_DOC_ITALIC, 0) == XUI_OK);
            if (!mode) {
                style.iSize = sizeof(style); style.iTextColor = 0x44aa66ffu;
                CHECK(xuiDocumentEditorSetTextStyle(editor,
                    XUI_DOC_TEXT_STYLE_COLOR, &style) == XUI_OK);
            }
            CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
            CHECK(xuiDocumentMarkSaved(d, snapshot) == XUI_OK);
            xuiDocumentSnapshotRelease(snapshot);
            before.iSize = after.iSize = sizeof(before);
            CHECK(xuiDocumentGetStats(d, &before) == XUI_OK && !xuiDocumentIsDirty(d));
            revision = xuiDocumentGetRevision(d); can_undo = xuiDocumentCanUndo(d);
            allocator.remaining = point;
            result = xuiDocumentEditorInsertText(editor, "U\nV", 3);
            allocator.remaining = -1;
            if (result == XUI_OK) {
                success = 1;
                CHECK(xuiDocumentGetRevision(d) == revision + 1 && xuiDocumentIsDirty(d));
                CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
                CHECK(!xuiDocumentIsDirty(d));
                if (mode) source(d, "PRE **[word](/target)** POST\n");
                else plain(d, "abcde\n");
            } else {
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY &&
                    xuiDocumentGetRevision(d) == revision &&
                    xuiDocumentCanUndo(d) == can_undo && !xuiDocumentIsDirty(d));
                CHECK(xuiDocumentGetStats(d, &after) == XUI_OK &&
                    after.iUndoCount == before.iUndoCount &&
                    after.iRedoCount == before.iRedoCount &&
                    after.iHistoryBytes == before.iHistoryBytes);
                if (mode) source(d, "PRE **[word](/target)** POST\n");
                else plain(d, "abcde\n");
            }
            xuiWidgetDestroy(editor); xuiDocumentRelease(d);
            CHECK(!allocator.live);
        }
        CHECK(success);
        printf("DocumentEditor multiline pending style %s allocation-failure sweep: %ld points; atomic content/history and no leaks passed\n",
            mode ? "Markdown" : "Rich", point);
    }
}

static void text_style_editor_cases(xui_context context)
{
    xui_doc_editor_desc_t desc = {0}; xui_doc_editor_text_style_state_t state = {0};
    xui_doc_text_style_t style = {0}; xui_doc_range_t selection;
    xui_document d; xui_widget editor; xui_document_snapshot snapshot;
    uint64_t found; uint32_t common, mixed;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = d; desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "abcd", 4) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "bc", 2,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    style.iSize = sizeof(style); style.iTextColor = 0x335577ffu;
    style.iBackgroundColor = 0xffeeddffu; style.fFontSize = 24;
    strcpy(style.sFontFamily, "Test family");
    CHECK(xuiDocumentEditorSetTextStyle(editor, 15, &style) == XUI_OK);
    state.iSize = sizeof(state);
    CHECK(xuiDocumentEditorQueryTextStyle(editor, &state) == XUI_OK &&
        state.bEnabled && state.tQuery.bHasText && !state.tQuery.iMixedFields &&
        state.tQuery.tStyle.iTextColor == style.iTextColor &&
        state.tQuery.tStyle.fFontSize == 24 &&
        !strcmp(state.tQuery.tStyle.sFontFamily, "Test family"));
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentEditorQueryTextStyle(editor, &state) == XUI_OK &&
        !state.tQuery.tStyle.iTextColor && !state.tQuery.tStyle.fFontSize);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "abcd", 4,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorQueryTextStyle(editor, &state) == XUI_OK &&
        state.bEnabled && state.tQuery.iMixedFields == 15);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "a", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = selection.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    style.iTextColor = 0xaa4422ffu; style.fFontSize = 18;
    CHECK(xuiDocumentEditorSetTextStyle(editor,
        XUI_DOC_TEXT_STYLE_COLOR | XUI_DOC_TEXT_STYLE_FONT_SIZE, &style) == XUI_OK);
    CHECK(xuiDocumentEditorSetMarks(editor, XUI_DOC_BOLD, 0) == XUI_OK);
    CHECK(xuiDocumentEditorQueryTextStyle(editor, &state) == XUI_OK &&
        state.tQuery.tStyle.iTextColor == 0xaa4422ffu && state.tQuery.tStyle.fFontSize == 18);
    CHECK(xuiDocumentEditorInsertText(editor, "X", 1) == XUI_OK); plain(d, "aXbcd\n");
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "X", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotQueryTextStyle(snapshot, &selection, &state.tQuery) == XUI_OK &&
        state.tQuery.tStyle.iTextColor == 0xaa4422ffu && state.tQuery.tStyle.fFontSize == 18);
    CHECK(xuiDocumentSnapshotQueryMarks(snapshot, &selection, &common, &mixed) == XUI_OK &&
        (common & XUI_DOC_BOLD) && !mixed);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorInsertText(editor, "Y", 1) == XUI_OK); plain(d, "aXYbcd\n");
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "Y", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotQueryTextStyle(snapshot, &selection, &state.tQuery) == XUI_OK &&
        state.tQuery.tStyle.iTextColor == 0xaa4422ffu && state.tQuery.tStyle.fFontSize == 18);
    CHECK(xuiDocumentSnapshotQueryMarks(snapshot, &selection, &common, &mixed) == XUI_OK &&
        (common & XUI_DOC_BOLD) && !mixed);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); plain(d, "aXbcd\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); plain(d, "abcd\n");
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "a", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = selection.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    style.iTextColor = 0; style.iBackgroundColor = 0;
    style.iExplicitFields = XUI_DOC_TEXT_STYLE_COLOR | XUI_DOC_TEXT_STYLE_BACKGROUND;
    CHECK(xuiDocumentEditorSetTextStyle(editor,
        XUI_DOC_TEXT_STYLE_COLOR | XUI_DOC_TEXT_STYLE_BACKGROUND, &style) == XUI_OK);
    CHECK(xuiDocumentEditorQueryTextStyle(editor, &state) == XUI_OK &&
        state.tQuery.tStyle.iExplicitFields ==
            (XUI_DOC_TEXT_STYLE_COLOR | XUI_DOC_TEXT_STYLE_BACKGROUND));
    CHECK(xuiDocumentEditorInsertText(editor, "Z", 1) == XUI_OK); plain(d, "aZbcd\n");
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "Z", 1,
        NULL, &selection, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotQueryTextStyle(snapshot, &selection, &state.tQuery) == XUI_OK &&
        !state.tQuery.tStyle.iTextColor && !state.tQuery.tStyle.iBackgroundColor &&
        state.tQuery.tStyle.iExplicitFields ==
            (XUI_DOC_TEXT_STYLE_COLOR | XUI_DOC_TEXT_STYLE_BACKGROUND));
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); plain(d, "abcd\n");
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    {
        xui_doc_desc_t markdown = {0};
        markdown.iSize = sizeof(markdown); markdown.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&markdown, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, "plain\n", 6) == XUI_OK);
        desc.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
        CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiDocumentEditorQueryTextStyle(editor, &state) == XUI_OK &&
            !state.bEnabled && state.iDisabledReason == XUI_DOC_ERROR_UNREPRESENTABLE);
        CHECK(xuiDocumentEditorSetTextStyle(editor, XUI_DOC_TEXT_STYLE_COLOR, &style) ==
            XUI_DOC_ERROR_UNREPRESENTABLE);
        source(d, "plain\n");
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    puts("Rich text Editor style: selection, mixed/query, pending typing with marks and explicit zero colors, Undo and Markdown capability passed");
}
static void send_pointer_mod(xui_context context, xui_widget editor,
    int kind, int x, int y, uint32_t buttons, uint32_t modifiers)
{
    xui_event_t event = {0};
    event.iSize = sizeof(event); event.iType = kind; event.pTarget = editor;
    event.fX = x; event.fY = y; event.iPointerId = 7;
    event.iPointerType = XUI_POINTER_TYPE_MOUSE;
    event.iButton = kind == XUI_EVENT_POINTER_MOVE ? 0 : XUI_POINTER_BUTTON_LEFT;
    event.iButtons = buttons; event.iModifiers = modifiers;
    CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
}
static void send_pointer(xui_context context, xui_widget editor, int kind, int x, int y, uint32_t buttons)
{
    send_pointer_mod(context, editor, kind, x, y, buttons, 0);
}
static void drag_autoscroll_cases(xui_context context)
{
    xui_doc_editor_desc_t editor_desc = {0}; xui_doc_view_desc_t view_desc = {0};
    xui_document document; xui_widget widget; xui_doc_range_t selection;
    char text[4096]; size_t bytes = 0; double x, y, after_move, after_tick;
    int i, kind;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    editor_desc.iSize = sizeof(editor_desc); editor_desc.tView.iSize = sizeof(editor_desc.tView);
    editor_desc.tView.pDocument = document; editor_desc.iMode = XUI_DOC_VISUAL;
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
    for (i = 0; i < 80; i++) {
        int written = snprintf(text + bytes, sizeof(text) - bytes, "row %02d abcdefghijklmnopqrstuvwxyz\n", i);
        CHECK(written > 0 && (size_t)written < sizeof(text) - bytes);
        bytes += (size_t)written;
    }
    for (kind = 0; kind < 2; kind++) {
        if (!kind) {
            CHECK(xuiDocumentEditorCreate(context, &editor_desc, &widget) == XUI_OK);
            CHECK(xuiDocumentEditorInsertText(widget, text, bytes) == XUI_OK);
        } else CHECK(xuiDocumentViewCreate(context, &view_desc, &widget) == XUI_OK);
        CHECK(xuiSetRootWidget(context, widget) == XUI_OK);
        CHECK(xuiWidgetSetRect(widget, (xui_rect_t){0, 0, 180, 100}) == XUI_OK);
        CHECK(xuiInputViewport(context, 180, 100) == XUI_OK);
        CHECK(xuiUpdate(context, .016f) == XUI_OK);
        CHECK(xuiDocumentViewSetScroll(widget, 0, 0) == XUI_OK);
        send_pointer(context, widget, XUI_EVENT_POINTER_DOWN, 20, 20, XUI_POINTER_BUTTON_LEFT);
        send_pointer(context, widget, XUI_EVENT_POINTER_MOVE, 20, 145, XUI_POINTER_BUTTON_LEFT);
        CHECK(xuiDocumentViewGetScroll(widget, &x, &after_move) == XUI_OK && after_move > 0);
        for (i = 0; i < 10; i++)
            send_pointer(context, widget, XUI_EVENT_POINTER_MOVE, 20, 145, XUI_POINTER_BUTTON_LEFT);
        CHECK(xuiDocumentViewGetScroll(widget, &x, &y) == XUI_OK && y == after_move);
        CHECK(xuiDocumentViewGetSelection(widget, &selection) == XUI_OK &&
            (selection.tAnchor.iNodeId != selection.tCaret.iNodeId ||
             selection.tAnchor.iOffset != selection.tCaret.iOffset));
        CHECK(xuiUpdate(context, .05f) == XUI_OK);
        CHECK(xuiDocumentViewGetScroll(widget, &x, &after_tick) == XUI_OK && after_tick > after_move);
        send_pointer(context, widget, XUI_EVENT_POINTER_MOVE, 20, 40, XUI_POINTER_BUTTON_LEFT);
        CHECK(xuiUpdate(context, .05f) == XUI_OK);
        CHECK(xuiDocumentViewGetScroll(widget, &x, &y) == XUI_OK && y == after_tick);
        send_pointer(context, widget, XUI_EVENT_POINTER_UP, 20, 40, 0);
        CHECK(xuiUpdate(context, .05f) == XUI_OK);
        CHECK(xuiDocumentViewGetScroll(widget, &x, &y) == XUI_OK && y == after_tick);
        xuiWidgetDestroy(widget);
    }
    xuiDocumentRelease(document);
    puts("Document View/Editor drag selection: viewport overflow auto-scrolls while held and stops on return/release passed");
}
typedef struct accessible_activation {
    unsigned calls;
    xui_doc_node_id node;
    char resource[64];
} accessible_activation;
static void accessible_activate(xui_widget widget, xui_doc_node_id node,
    const char* resource, void* user)
{
    accessible_activation* state = user; (void)widget;
    state->calls++; state->node = node;
    snprintf(state->resource, sizeof(state->resource), "%s", resource ? resource : "");
}
static void semantic_accessibility_cases(xui_context context)
{
    const char* markdown = "- [ ] todo\n\n[go](/go) ![alt](/img)\n\n| A | B |\n| --- | --- |\n| c | d |\n";
    const char* checked = "- [x] todo\n\n[go](/go) ![alt](/img)\n\n| A | B |\n| --- | --- |\n| c | d |\n";
    xui_doc_desc_t doc_desc = {0}; xui_doc_editor_desc_t desc = {0};
    xui_document document; xui_widget editor, view; xui_doc_view_desc_t view_desc = {0};
    xui_accessible_node_t node = {0}; xui_accessible_selection_t selection = {0};
    accessible_activation activation = {0}; uint64_t link = 0, image = 0, table = 0, task = 0;
    uint64_t first_cell = 0, second_cell = 0, revision;
    xui_doc_table_selection_t selected = {0};
    int count, i, cells = 0, start, end; double scroll_x, scroll_y;
    doc_desc.iSize = sizeof(doc_desc); doc_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    doc_desc.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&doc_desc, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, markdown, strlen(markdown)) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iMode = XUI_DOC_VISUAL;
    desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document;
    desc.tView.onActivate = accessible_activate; desc.tView.pUser = &activation;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 180, 100}) == XUI_OK);
    CHECK(xuiInputViewport(context, 180, 100) == XUI_OK);
    CHECK(xuiWidgetSetAccessibleName(editor, "Markdown document") == XUI_OK);
    count = xuiWidgetGetAccessibleNodeCount(editor); CHECK(count >= 15);
    for (i = 0; i < count; i++) {
        node.iSize = sizeof(node);
        CHECK(xuiWidgetGetAccessibleNode(editor, i, &node) == XUI_OK);
        if (!i) CHECK(node.iId == XUI_DOCUMENT_ROOT && node.iRole == XUI_ACCESSIBLE_ROLE_DOCUMENT &&
            !strcmp(node.sName, "Markdown document") && (node.iState & XUI_ACCESSIBLE_STATE_EDITABLE));
        else CHECK(node.iId > XUI_DOCUMENT_ROOT && node.iParentId != 0);
        if (node.iRole == XUI_ACCESSIBLE_ROLE_LINK) {
            CHECK(!strcmp(node.sName, "go") && !strcmp(node.sDescription, "/go")); link = node.iId;
        } else if (node.iRole == XUI_ACCESSIBLE_ROLE_IMAGE) {
            CHECK(!strcmp(node.sName, "alt")); image = node.iId;
        } else if (node.iRole == XUI_ACCESSIBLE_ROLE_CHECKBOX) {
            CHECK(!(node.iState & XUI_ACCESSIBLE_STATE_CHECKED)); task = node.iId;
        } else if (node.iRole == XUI_ACCESSIBLE_ROLE_TABLE) {
            CHECK(node.iRowCount == 2 && node.iColumnCount == 2 && node.tBounds.fH > 0);
            table = node.iId;
        } else if (node.iRole == XUI_ACCESSIBLE_ROLE_CELL) {
            CHECK(node.tBounds.fW > 0 && node.tBounds.fH > 0 &&
                node.iRowCount == 1 && node.iColumnCount == 1 &&
                (node.iState & XUI_ACCESSIBLE_STATE_SELECTABLE) &&
                (node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)) &&
                (node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW)));
            if (node.iRow == 0 && node.iColumn == 0) first_cell = node.iId;
            if (node.iRow == 0 && node.iColumn == 1) second_cell = node.iId;
            cells++;
        }
    }
    CHECK(link && image && table && task && first_cell && second_cell && cells == 4);
    CHECK(xuiWidgetPerformAccessibleAction(editor, link, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) == XUI_OK &&
        activation.calls == 1 && activation.node == link && !strcmp(activation.resource, "/go"));
    selection.iSize = sizeof(selection); selection.iAnchor = 1; selection.iCaret = 2;
    CHECK(xuiWidgetPerformAccessibleAction(editor, link,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_OK);
    { xui_doc_range_t link_range;
      CHECK(xuiDocumentViewGetSelection(editor, &link_range) == XUI_OK &&
          link_range.tAnchor.iNodeId == link && link_range.tAnchor.iOffset == 1 &&
          link_range.tCaret.iNodeId == link && link_range.tCaret.iOffset == 2); }
    CHECK(xuiWidgetPerformAccessibleAction(editor, image, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) == XUI_OK &&
        activation.calls == 2 && activation.node == image && !strcmp(activation.resource, "/img"));
    CHECK(xuiWidgetPerformAccessibleAction(editor, image,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
    { xui_doc_range_t object_selection; int selected = 0;
      CHECK(xuiDocumentViewGetSelection(editor, &object_selection) == XUI_OK &&
          object_selection.tAnchor.iKind == XUI_DOC_POSITION_GAP &&
          object_selection.tCaret.iKind == XUI_DOC_POSITION_GAP &&
          object_selection.tCaret.iOffset == object_selection.tAnchor.iOffset + 1);
      for (i = 0; i < count; i++) {
          node.iSize = sizeof(node);
          CHECK(xuiWidgetGetAccessibleNode(editor, i, &node) == XUI_OK);
          if (node.iId == image) {
              CHECK((node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)) &&
                  (node.iState & XUI_ACCESSIBLE_STATE_SELECTED));
              selected = 1;
          }
      }
      CHECK(selected); }
    selected.iSize = sizeof(selected); revision = xuiDocumentGetRevision(document);
    CHECK(xuiWidgetPerformAccessibleAction(editor, first_cell,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iTableId == table && selected.iRow == 0 && selected.iColumn == 0 &&
        selected.iRows == 1 && selected.iColumns == 1 &&
        xuiDocumentGetRevision(document) == revision);
    { int first_selected = 0, second_selected = 0;
      for (i = 0; i < count; i++) {
          node.iSize = sizeof(node);
          CHECK(xuiWidgetGetAccessibleNode(editor, i, &node) == XUI_OK);
          if (node.iId == first_cell) first_selected = !!(node.iState & XUI_ACCESSIBLE_STATE_SELECTED);
          if (node.iId == second_cell) second_selected = !!(node.iState & XUI_ACCESSIBLE_STATE_SELECTED);
      }
      CHECK(first_selected && !second_selected); }
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 180, 50}) == XUI_OK &&
        xuiInputViewport(context, 180, 50) == XUI_OK &&
        xuiDocumentViewSetScroll(editor, 0, 0) == XUI_OK);
    { int offscreen = 0;
      for (i = 0; i < count; i++) {
          node.iSize = sizeof(node);
          CHECK(xuiWidgetGetAccessibleNode(editor, i, &node) == XUI_OK);
          if (node.iId == first_cell) offscreen = !!(node.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN);
      }
      CHECK(offscreen); }
    CHECK(xuiWidgetPerformAccessibleAction(editor, first_cell,
        XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW, NULL) == XUI_OK);
    CHECK(xuiDocumentViewGetScroll(editor, &scroll_x, &scroll_y) == XUI_OK && scroll_y > 0);
    CHECK(xuiWidgetPerformAccessibleAction(editor, table, XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW, NULL) == XUI_OK);
    CHECK(xuiDocumentViewGetScroll(editor, &scroll_x, &scroll_y) == XUI_OK && scroll_y > 0);
    selection.iSize = sizeof(selection); selection.iAnchor = 0; selection.iCaret = 4;
    CHECK(xuiWidgetPerformAccessibleAction(editor, XUI_DOCUMENT_ROOT,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &selection) == XUI_OK);
    CHECK(xuiEditGetSelection(editor, &start, &end) == XUI_OK && start == 0 && end == 4);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND);
    { int selected = 0;
      for (i = 0; i < count; i++) {
          node.iSize = sizeof(node);
          CHECK(xuiWidgetGetAccessibleNode(editor, i, &node) == XUI_OK);
          if (node.iRole == XUI_ACCESSIBLE_ROLE_TEXT && node.sValue &&
              !strcmp(node.sValue, "todo") && (node.iState & XUI_ACCESSIBLE_STATE_SELECTED)) selected = 1;
      }
      CHECK(selected); }
    CHECK(xuiWidgetPerformAccessibleAction(editor, task, XUI_ACCESSIBLE_ACTION_TOGGLE, NULL) == XUI_OK);
    source(document, checked);
    for (i = 0; i < xuiWidgetGetAccessibleNodeCount(editor); i++) {
        node.iSize = sizeof(node);
        CHECK(xuiWidgetGetAccessibleNode(editor, i, &node) == XUI_OK);
        if (node.iId == task) { CHECK(node.iState & XUI_ACCESSIBLE_STATE_CHECKED); break; }
    }
    CHECK(i < count);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(document, markdown);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    CHECK(xuiWidgetPerformAccessibleAction(editor, task, XUI_ACCESSIBLE_ACTION_TOGGLE, NULL) == XUI_ERROR_INVALID_STATE);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiWidgetGetAccessibleNodeCount(editor) == 1);
    CHECK(xuiWidgetPerformAccessibleAction(editor, first_cell,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_ERROR_UNSUPPORTED);
    node.iSize = sizeof(node); CHECK(xuiWidgetGetAccessibleNode(editor, 0, &node) == XUI_OK &&
        node.iId == XUI_DOCUMENT_ROOT && node.iRole == XUI_ACCESSIBLE_ROLE_TEXTBOX);
    xuiWidgetDestroy(editor);
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetGetAccessibleNodeCount(view) == count);
    node.iSize = sizeof(node); CHECK(xuiWidgetGetAccessibleNode(view, 0, &node) == XUI_OK &&
        node.iRole == XUI_ACCESSIBLE_ROLE_DOCUMENT && (node.iState & XUI_ACCESSIBLE_STATE_READONLY));
    CHECK(xuiWidgetPerformAccessibleAction(view, second_cell,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
    CHECK(xuiDocumentViewGetTableSelection(view, &selected) == XUI_OK &&
        selected.iTableId == table && selected.iColumn == 1 && selected.iColumns == 1);
    { int seen = 0;
      for (i = 0; i < count; i++) {
          node.iSize = sizeof(node);
          CHECK(xuiWidgetGetAccessibleNode(view, i, &node) == XUI_OK);
          if (node.iId == second_cell) {
              CHECK((node.iState & XUI_ACCESSIBLE_STATE_SELECTED) &&
                  (node.iState & XUI_ACCESSIBLE_STATE_READONLY)); seen = 1;
          }
      }
      CHECK(seen); }
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    {
        xui_document_transaction transaction; xui_doc_node_desc_t item = {0};
        xui_doc_node_id paragraph, rich_link, rich_image; int link_found = 0, image_found = 0;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
        CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        item.iSize = sizeof(item); item.iKind = XUI_DOC_PARAGRAPH;
        CHECK(xuiDocumentTxnInsertNode(transaction, XUI_DOCUMENT_ROOT,
            XUI_DOCUMENT_APPEND, &item, &paragraph) == XUI_OK);
        item.iKind = XUI_DOC_TEXT; item.sText = "rich"; item.iTextBytes = 4;
        item.sResource = "/rich"; item.tAttributes.iMarks = XUI_DOC_LINK;
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph,
            XUI_DOCUMENT_APPEND, &item, &rich_link) == XUI_OK);
        item.iKind = XUI_DOC_IMAGE; item.sText = "image"; item.iTextBytes = 5;
        item.sResource = "/rich-image"; item.tAttributes.iMarks = 0;
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph,
            XUI_DOCUMENT_APPEND, &item, &rich_image) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        view_desc.pDocument = document; view_desc.onActivate = accessible_activate;
        view_desc.pUser = &activation; activation.calls = 0;
        CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
        CHECK(xuiSetRootWidget(context, view) == XUI_OK);
        CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 180, 100}) == XUI_OK);
        count = xuiWidgetGetAccessibleNodeCount(view); CHECK(count == 4);
        for (i = 0; i < count; i++) {
            node.iSize = sizeof(node); CHECK(xuiWidgetGetAccessibleNode(view, i, &node) == XUI_OK);
            if (node.iId == rich_link) {
                CHECK(node.iRole == XUI_ACCESSIBLE_ROLE_LINK && !strcmp(node.sName, "rich")); link_found = 1;
            } else if (node.iId == rich_image) {
                CHECK(node.iRole == XUI_ACCESSIBLE_ROLE_IMAGE && !strcmp(node.sName, "image")); image_found = 1;
            }
        }
        CHECK(link_found && image_found);
        CHECK(xuiWidgetPerformAccessibleAction(view, rich_image,
            XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
        for (i = 0; i < count; i++) {
            node.iSize = sizeof(node);
            CHECK(xuiWidgetGetAccessibleNode(view, i, &node) == XUI_OK);
            if (node.iId == rich_image)
                CHECK((node.iState & XUI_ACCESSIBLE_STATE_READONLY) &&
                    (node.iState & XUI_ACCESSIBLE_STATE_SELECTED));
        }
        CHECK(xuiWidgetPerformAccessibleAction(view, rich_link, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) == XUI_OK &&
            activation.calls == 1 && !strcmp(activation.resource, "/rich"));
        xuiWidgetDestroy(view); xuiDocumentRelease(document);
    }
    {
        const char* extended = "# Title\n\n- [ ] **bold**\n\nInline $x^2$ and note[^a].\n\n[^a]: Footnote\n";
        int footnotes = 0;
        doc_desc.iMarkdownDialect = XUI_MD_EXTENDED;
        CHECK(xuiDocumentCreate(&doc_desc, &document) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(document, extended, strlen(extended)) == XUI_OK);
        view_desc.pDocument = document; view_desc.onActivate = NULL; view_desc.pUser = NULL;
        CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
        CHECK(xuiSetRootWidget(context, view) == XUI_OK);
        CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 180, 100}) == XUI_OK);
        count = xuiWidgetGetAccessibleNodeCount(view); CHECK(count > 10);
        for (i = 0; i < count; i++) {
            node.iSize = sizeof(node);
            CHECK(xuiWidgetGetAccessibleNode(view, i, &node) == XUI_OK);
            if (node.iRole == XUI_ACCESSIBLE_ROLE_GENERIC && node.iParentId == XUI_DOCUMENT_ROOT)
                footnotes++;
        }
        CHECK(footnotes > 0);
        xuiWidgetDestroy(view); xuiDocumentRelease(document);
    }
    puts("Document semantic accessibility: rich node identities, link/image activation, table cells, task toggle/Undo, readonly and SOURCE role passed");
}
static void accessible_text_range_cases(xui_context context)
{
    xui_document document; xui_document_transaction txn = NULL;
    xui_doc_node_desc_t item = {0}; xui_doc_editor_desc_t ed = {0};
    xui_doc_view_desc_t vd = {0}; xui_doc_range_t range;
    xui_widget editor, view; xui_accessible_node_t node = {0};
    xui_accessible_selection_t local = {0};
    uint64_t paragraph, first, second, revision; int start, end, i;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    item.iSize = sizeof(item); item.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(txn, XUI_DOCUMENT_ROOT,
        XUI_DOCUMENT_APPEND, &item, &paragraph) == XUI_OK);
    item.iKind = XUI_DOC_TEXT; item.sText = "a\xe4\xb8\xad" "b";
    item.iTextBytes = 5; item.tAttributes.iMarks = XUI_DOC_BOLD;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph,
        XUI_DOCUMENT_APPEND, &item, &first) == XUI_OK);
    item.sText = " tail"; item.iTextBytes = 5; item.tAttributes.iMarks = 0;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph,
        XUI_DOCUMENT_APPEND, &item, &second) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    revision = xuiDocumentGetRevision(document);
    ed.iSize = sizeof(ed); ed.iMode = XUI_DOC_VISUAL;
    ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 220, 100}) == XUI_OK);
    CHECK(xuiWidgetGetAccessibleNodeCount(editor) == 4);
    for (i = 2; i < 4; i++) {
        node.iSize = sizeof(node);
        CHECK(xuiWidgetGetAccessibleNode(editor, i, &node) == XUI_OK &&
            node.iRole == XUI_ACCESSIBLE_ROLE_TEXT &&
            node.iId == (i == 2 ? first : second) &&
            (node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
    }
    local.iSize = sizeof(local); local.iAnchor = 1; local.iCaret = 4;
    CHECK(xuiWidgetPerformAccessibleAction(editor, first,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == first && range.tAnchor.iOffset == 1 &&
        range.tCaret.iNodeId == first && range.tCaret.iOffset == 4);
    node.iSize = sizeof(node);
    CHECK(xuiWidgetGetAccessibleNode(editor, 2, &node) == XUI_OK &&
        node.iTextStart == 1 && node.iTextEnd == 4 &&
        (node.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    local.iAnchor = 4; local.iCaret = 1;
    CHECK(xuiWidgetPerformAccessibleAction(editor, first,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iOffset == 4 && range.tCaret.iOffset == 1);
    node.iSize = sizeof(node);
    CHECK(xuiWidgetGetAccessibleNode(editor, 2, &node) == XUI_OK &&
        node.iTextStart == 4 && node.iTextEnd == 1);
    local.iAnchor = 2; local.iCaret = 4;
    CHECK(xuiWidgetPerformAccessibleAction(editor, first,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_DOC_ERROR_UTF8);
    local.iAnchor = 0; local.iCaret = 6;
    CHECK(xuiWidgetPerformAccessibleAction(editor, first,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_ERROR_INVALID_ARGUMENT);
    local.iSize = sizeof(local) - 1;
    CHECK(xuiWidgetPerformAccessibleAction(editor, first,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iOffset == 4 && range.tCaret.iOffset == 1);
    CHECK(xuiWidgetPerformAccessibleAction(editor, first,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
    CHECK(xuiEditGetSelection(editor, &start, &end) == XUI_OK && start == 0 && end == 5);
    CHECK(xuiEditSetSelection(editor, 1, 7) == XUI_OK);
    node.iSize = sizeof(node);
    CHECK(xuiWidgetGetAccessibleNode(editor, 2, &node) == XUI_OK &&
        node.iTextStart == 1 && node.iTextEnd == 5 &&
        (node.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    node.iSize = sizeof(node);
    CHECK(xuiWidgetGetAccessibleNode(editor, 3, &node) == XUI_OK &&
        node.iTextStart == 0 && node.iTextEnd == 2 &&
        (node.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    CHECK(xuiEditSetSelection(editor, 7, 1) == XUI_OK);
    node.iSize = sizeof(node);
    CHECK(xuiWidgetGetAccessibleNode(editor, 2, &node) == XUI_OK &&
        node.iTextStart == 5 && node.iTextEnd == 1);
    node.iSize = sizeof(node);
    CHECK(xuiWidgetGetAccessibleNode(editor, 3, &node) == XUI_OK &&
        node.iTextStart == 2 && node.iTextEnd == 0);
    CHECK(xuiDocumentGetRevision(document) == revision);
    xuiWidgetDestroy(editor);
    vd.iSize = sizeof(vd); vd.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &vd, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    local.iSize = sizeof(local); local.iAnchor = 4; local.iCaret = 1;
    CHECK(xuiWidgetPerformAccessibleAction(view, first,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    node.iSize = sizeof(node);
    CHECK(xuiWidgetGetAccessibleNode(view, 2, &node) == XUI_OK &&
        (node.iState & XUI_ACCESSIBLE_STATE_READONLY) &&
        node.iTextStart == 4 && node.iTextEnd == 1);
    xuiWidgetDestroy(view);
    vd.bDisableSelection = 1;
    CHECK(xuiDocumentViewCreate(context, &vd, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    node.iSize = sizeof(node);
    CHECK(xuiWidgetGetAccessibleNode(view, 2, &node) == XUI_OK &&
        !(node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
    node.iSize = sizeof(node);
    CHECK(xuiWidgetGetAccessibleNode(view, 0, &node) == XUI_OK &&
        !(node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
    CHECK(xuiWidgetPerformAccessibleAction(view, first,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_ERROR_UNSUPPORTED);
    CHECK(xuiWidgetPerformAccessibleAction(view, XUI_DOCUMENT_ROOT,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_ERROR_UNSUPPORTED);
    CHECK(xuiDocumentGetRevision(document) == revision);
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    puts("Document Text accessibility: local UTF-8 ranges, direction, cross-node state, readonly and disabled selection passed");
}
static xui_accessible_node_t accessible_node_by_id(xui_widget widget, uint64_t id)
{
    xui_accessible_node_t node = {0}; int i, count = xuiWidgetGetAccessibleNodeCount(widget);
    for (i = 0; i < count; i++) {
        node.iSize = sizeof(node);
        CHECK(xuiWidgetGetAccessibleNode(widget, i, &node) == XUI_OK);
        if (node.iId == id) return node;
    }
    CHECK(0); return node;
}
static void accessible_container_text_cases(xui_context context)
{
    const char* projected = "a\xe4\xb8\xad" "bc\xef\xbf\xbc" "d\ne";
    xui_document document; xui_document_transaction txn = NULL;
    xui_doc_node_desc_t item = {0}; xui_doc_editor_desc_t ed = {0};
    xui_doc_view_desc_t vd = {0}; xui_doc_range_t range;
    xui_accessible_node_t node; xui_accessible_selection_t local = {0};
    xui_widget editor, view; uint64_t paragraph, second_paragraph;
    uint64_t first, link, image, after_image, hard_break, after_break, second, code, revision;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    item.iSize = sizeof(item); item.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(txn, XUI_DOCUMENT_ROOT,
        XUI_DOCUMENT_APPEND, &item, &paragraph) == XUI_OK);
    item.iKind = XUI_DOC_TEXT; item.sText = "a\xe4\xb8\xad";
    item.iTextBytes = 4; item.tAttributes.iMarks = XUI_DOC_BOLD;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph,
        XUI_DOCUMENT_APPEND, &item, &first) == XUI_OK);
    item.sText = "bc"; item.iTextBytes = 2;
    item.tAttributes.iMarks = XUI_DOC_LINK; item.sResource = "/go";
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph,
        XUI_DOCUMENT_APPEND, &item, &link) == XUI_OK);
    memset(&item, 0, sizeof(item)); item.iSize = sizeof(item);
    item.iKind = XUI_DOC_IMAGE; item.sText = "long alt";
    item.iTextBytes = 8; item.sResource = "/img";
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph,
        XUI_DOCUMENT_APPEND, &item, &image) == XUI_OK);
    memset(&item, 0, sizeof(item)); item.iSize = sizeof(item);
    item.iKind = XUI_DOC_TEXT; item.sText = "d"; item.iTextBytes = 1;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph,
        XUI_DOCUMENT_APPEND, &item, &after_image) == XUI_OK);
    memset(&item, 0, sizeof(item)); item.iSize = sizeof(item);
    item.iKind = XUI_DOC_HARD_BREAK;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph,
        XUI_DOCUMENT_APPEND, &item, &hard_break) == XUI_OK);
    item.iKind = XUI_DOC_TEXT; item.sText = "e"; item.iTextBytes = 1;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph,
        XUI_DOCUMENT_APPEND, &item, &after_break) == XUI_OK);
    memset(&item, 0, sizeof(item)); item.iSize = sizeof(item);
    item.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(txn, XUI_DOCUMENT_ROOT,
        XUI_DOCUMENT_APPEND, &item, &second_paragraph) == XUI_OK);
    item.iKind = XUI_DOC_TEXT; item.sText = "tail"; item.iTextBytes = 4;
    CHECK(xuiDocumentTxnInsertNode(txn, second_paragraph,
        XUI_DOCUMENT_APPEND, &item, &second) == XUI_OK);
    item.iKind = XUI_DOC_CODE_BLOCK; item.sText = "aa\n\xe4\xb8\xad";
    item.iTextBytes = 6; item.sInfo = "c";
    CHECK(xuiDocumentTxnInsertNode(txn, XUI_DOCUMENT_ROOT,
        XUI_DOCUMENT_APPEND, &item, &code) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn); revision = xuiDocumentGetRevision(document);
    ed.iSize = sizeof(ed); ed.iMode = XUI_DOC_VISUAL;
    ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 240, 120}) == XUI_OK);
    node = accessible_node_by_id(editor, paragraph);
    CHECK(node.iRole == XUI_ACCESSIBLE_ROLE_PARAGRAPH && node.sValue &&
        !strcmp(node.sValue, projected) &&
        (node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
    local.iSize = sizeof(local); local.iAnchor = 6; local.iCaret = 9;
    CHECK(xuiWidgetPerformAccessibleAction(editor, paragraph,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == paragraph && range.tAnchor.iKind == XUI_DOC_POSITION_GAP &&
        range.tAnchor.iOffset == 2 && range.tCaret.iNodeId == paragraph &&
        range.tCaret.iOffset == 3);
    node = accessible_node_by_id(editor, paragraph);
    CHECK(node.iTextStart == 6 && node.iTextEnd == 9 &&
        (node.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    node = accessible_node_by_id(editor, image);
    CHECK(node.iState & XUI_ACCESSIBLE_STATE_SELECTED);
    local.iAnchor = 7;
    CHECK(xuiWidgetPerformAccessibleAction(editor, paragraph,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_DOC_ERROR_UTF8);
    local.iAnchor = 2; local.iCaret = 4;
    CHECK(xuiWidgetPerformAccessibleAction(editor, paragraph,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_DOC_ERROR_UTF8);
    local.iAnchor = 1; local.iCaret = 10;
    CHECK(xuiWidgetPerformAccessibleAction(editor, paragraph,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == first && range.tAnchor.iOffset == 1 &&
        range.tCaret.iNodeId == after_image && range.tCaret.iOffset == 1);
    node = accessible_node_by_id(editor, paragraph);
    CHECK(node.iTextStart == 1 && node.iTextEnd == 10);
    local.iAnchor = 11; local.iCaret = 4;
    CHECK(xuiWidgetPerformAccessibleAction(editor, paragraph,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == paragraph && range.tAnchor.iOffset == 5 &&
        range.tCaret.iNodeId == link && range.tCaret.iOffset == 0);
    node = accessible_node_by_id(editor, paragraph);
    CHECK(node.iTextStart == 11 && node.iTextEnd == 4);
    range.tAnchor.iNodeId = first; range.tAnchor.iKind = XUI_DOC_POSITION_TEXT;
    range.tAnchor.iOffset = 1; range.tCaret.iNodeId = second;
    range.tCaret.iKind = XUI_DOC_POSITION_TEXT; range.tCaret.iOffset = 2;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    node = accessible_node_by_id(editor, paragraph);
    CHECK(node.iTextStart == 1 && node.iTextEnd == 12);
    node = accessible_node_by_id(editor, second_paragraph);
    CHECK(node.iTextStart == 0 && node.iTextEnd == 2);
    node = accessible_node_by_id(editor, code);
    CHECK(node.iRole == XUI_ACCESSIBLE_ROLE_CODE_BLOCK &&
        node.sValue && !strcmp(node.sValue, "aa\n\xe4\xb8\xad") &&
        (node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
    local.iAnchor = 3; local.iCaret = 6;
    CHECK(xuiWidgetPerformAccessibleAction(editor, code,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == code && range.tAnchor.iOffset == 3 &&
        range.tCaret.iNodeId == code && range.tCaret.iOffset == 6);
    CHECK(xuiDocumentGetRevision(document) == revision);
    xuiWidgetDestroy(editor);
    vd.iSize = sizeof(vd); vd.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &vd, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    local.iAnchor = 6; local.iCaret = 9;
    CHECK(xuiWidgetPerformAccessibleAction(view, paragraph,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    node = accessible_node_by_id(view, paragraph);
    CHECK((node.iState & XUI_ACCESSIBLE_STATE_READONLY) &&
        node.iTextStart == 6 && node.iTextEnd == 9);
    xuiWidgetDestroy(view); vd.bDisableSelection = 1;
    CHECK(xuiDocumentViewCreate(context, &vd, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    node = accessible_node_by_id(view, paragraph);
    CHECK(!(node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
    CHECK(xuiWidgetPerformAccessibleAction(view, paragraph,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_ERROR_UNSUPPORTED);
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    {
        const char* markdown = "# A *\xe4\xb8\xad* ![alt](/img)\n";
        xui_doc_desc_t md = {0}; xui_document_snapshot snap;
        uint64_t heading;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &document) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(document, markdown, strlen(markdown)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, XUI_DOCUMENT_ROOT, 0, &heading) == XUI_OK);
        xuiDocumentSnapshotRelease(snap);
        ed.tView.pDocument = document;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
        CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
        node = accessible_node_by_id(editor, heading);
        CHECK(node.iRole == XUI_ACCESSIBLE_ROLE_HEADING && node.sValue &&
            !strcmp(node.sValue, "A \xe4\xb8\xad \xef\xbf\xbc"));
        local.iAnchor = 2; local.iCaret = 5;
        CHECK(xuiWidgetPerformAccessibleAction(editor, heading,
            XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
        node = accessible_node_by_id(editor, heading);
        CHECK(node.iTextStart == 2 && node.iTextEnd == 5);
        source(document, markdown);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiWidgetPerformAccessibleAction(editor, heading,
            XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_ERROR_UNSUPPORTED);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    (void)hard_break; (void)after_break;
    puts("Document container accessibility: paragraph/heading text ranges, inline objects, breaks, code, readonly and Markdown passed");
}
static xui_doc_node_id table_cell_text(xui_document_snapshot snapshot, xui_doc_node_id table,
    uint64_t row_index, uint64_t cell_index)
{
    xui_doc_node_id row, cell, paragraph, text;
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, row_index, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, row, cell_index, &cell) == XUI_OK);
    if (xuiDocumentSnapshotGetChild(snapshot, cell, 0, &paragraph) != XUI_OK) return cell;
    if (xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &text) != XUI_OK) return paragraph;
    return text;
}
static void accessible_structured_container_cases(xui_context context)
{
    xui_document document; xui_document_transaction txn = NULL;
    xui_document_snapshot snap; xui_doc_node_desc_t item = {0};
    xui_doc_editor_desc_t ed = {0}; xui_doc_view_desc_t vd = {0};
    xui_widget editor, view; xui_doc_range_t range;
    xui_accessible_selection_t local = {0}; xui_accessible_node_t node;
    xui_doc_table_selection_t rectangle = {0};
    uint64_t table, rows[2], cells[2][2], texts[2][2], paragraph, second_text;
    uint64_t revision, list, first_item, inner_list, inner_item, inner_paragraph, inner_text;
    int row, column;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, XUI_DOCUMENT_ROOT, 0, 2, 2, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snap) == XUI_OK);
    for (row = 0; row < 2; row++) {
        CHECK(xuiDocumentSnapshotGetChild(snap, table, (uint64_t)row, &rows[row]) == XUI_OK);
        for (column = 0; column < 2; column++) {
            CHECK(xuiDocumentSnapshotGetChild(snap, rows[row], (uint64_t)column,
                &cells[row][column]) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetChild(snap, cells[row][column], 0,
                &paragraph) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetChild(snap, paragraph, 0,
                &texts[row][column]) == XUI_OK);
        }
    }
    xuiDocumentSnapshotRelease(snap);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(txn, texts[0][0], 0, 0, "a", 1) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(txn, texts[0][1], 0, 0, "c", 1) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(txn, texts[1][0], 0, 0, "d", 1) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(txn, texts[1][1], 0, 0,
        "\xe4\xbd\xa0", 3) == XUI_OK);
    item.iSize = sizeof(item); item.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(txn, cells[0][0], XUI_DOCUMENT_APPEND,
        &item, &paragraph) == XUI_OK);
    item.iKind = XUI_DOC_TEXT; item.sText = "b"; item.iTextBytes = 1;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
        &item, &second_text) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    revision = xuiDocumentGetRevision(document);
    ed.iSize = sizeof(ed); ed.iMode = XUI_DOC_VISUAL;
    ed.tView.iSize = sizeof(ed.tView); ed.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 240, 160}) == XUI_OK);
    node = accessible_node_by_id(editor, XUI_DOCUMENT_ROOT);
    CHECK(node.iRole == XUI_ACCESSIBLE_ROLE_DOCUMENT && node.sValue &&
        !strcmp(node.sValue, "a\nb\tc\nd\t\xe4\xbd\xa0\n") &&
        (node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
    local.iSize = sizeof(local); local.iAnchor = 0; local.iCaret = 3;
    CHECK(xuiWidgetPerformAccessibleAction(editor, XUI_DOCUMENT_ROOT,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == texts[0][0] && range.tAnchor.iOffset == 0 &&
        range.tCaret.iNodeId == second_text && range.tCaret.iOffset == 1);
    node = accessible_node_by_id(editor, XUI_DOCUMENT_ROOT);
    CHECK(node.iTextStart == 0 && node.iTextEnd == 3 &&
        (node.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    node = accessible_node_by_id(editor, cells[0][0]);
    CHECK(node.iRole == XUI_ACCESSIBLE_ROLE_CELL && node.sValue &&
        !strcmp(node.sValue, "a\nb") &&
        (node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
    node = accessible_node_by_id(editor, rows[0]);
    CHECK(node.sValue && !strcmp(node.sValue, "a\nb\tc"));
    node = accessible_node_by_id(editor, table);
    CHECK(node.iRole == XUI_ACCESSIBLE_ROLE_TABLE && node.sValue &&
        !strcmp(node.sValue, "a\nb\tc\nd\t\xe4\xbd\xa0"));
    local.iSize = sizeof(local); local.iAnchor = 2; local.iCaret = 3;
    CHECK(xuiWidgetPerformAccessibleAction(editor, cells[0][0],
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == second_text && range.tAnchor.iOffset == 0 &&
        range.tCaret.iNodeId == second_text && range.tCaret.iOffset == 1);
    node = accessible_node_by_id(editor, cells[0][0]);
    CHECK(node.iTextStart == 2 && node.iTextEnd == 3 &&
        (node.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    local.iAnchor = 1; local.iCaret = 2;
    CHECK(xuiWidgetPerformAccessibleAction(editor, cells[0][0],
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == texts[0][0] && range.tAnchor.iOffset == 1 &&
        range.tCaret.iNodeId == second_text && range.tCaret.iOffset == 0);
    CHECK(xuiWidgetPerformAccessibleAction(editor, cells[0][0],
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
    rectangle.iSize = sizeof(rectangle);
    CHECK(xuiDocumentViewGetTableSelection(editor, &rectangle) == XUI_OK &&
        rectangle.iTableId == table && rectangle.iRow == 0 &&
        rectangle.iColumn == 0 && rectangle.iRows == 1 && rectangle.iColumns == 1);
    local.iAnchor = 4; local.iCaret = 5;
    CHECK(xuiWidgetPerformAccessibleAction(editor, rows[0],
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == texts[0][1] && range.tAnchor.iOffset == 0 &&
        range.tCaret.iNodeId == texts[0][1] && range.tCaret.iOffset == 1);
    CHECK(xuiDocumentViewGetTableSelection(editor, &rectangle) == XUI_ERROR_NOT_FOUND);
    local.iAnchor = 3; local.iCaret = 4;
    CHECK(xuiWidgetPerformAccessibleAction(editor, rows[0],
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == second_text && range.tAnchor.iOffset == 1 &&
        range.tCaret.iNodeId == texts[0][1] && range.tCaret.iOffset == 0);
    local.iAnchor = 6; local.iCaret = 7;
    CHECK(xuiWidgetPerformAccessibleAction(editor, table,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == texts[1][0] && range.tAnchor.iOffset == 0 &&
        range.tCaret.iNodeId == texts[1][0] && range.tCaret.iOffset == 1);
    local.iAnchor = 8; local.iCaret = 9;
    CHECK(xuiWidgetPerformAccessibleAction(editor, table,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_DOC_ERROR_UTF8);
    local.iAnchor = 11; local.iCaret = 0;
    CHECK(xuiWidgetPerformAccessibleAction(editor, table,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    node = accessible_node_by_id(editor, table);
    CHECK(node.iTextStart == 11 && node.iTextEnd == 0 &&
        (node.iState & XUI_ACCESSIBLE_STATE_SELECTED));
    CHECK(xuiDocumentGetRevision(document) == revision);
    xuiWidgetDestroy(editor);
    vd.iSize = sizeof(vd); vd.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &vd, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    local.iAnchor = 2; local.iCaret = 3;
    CHECK(xuiWidgetPerformAccessibleAction(view, cells[0][0],
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
    node = accessible_node_by_id(view, cells[0][0]);
    CHECK((node.iState & XUI_ACCESSIBLE_STATE_READONLY) &&
        node.iTextStart == 2 && node.iTextEnd == 3);
    xuiWidgetDestroy(view); xuiDocumentRelease(document);
    {
        const char* markdown = "- [ ] first\n  - inner\n- second\n";
        xui_doc_desc_t md = {0};
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        md.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&md, &document) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(document, markdown, strlen(markdown)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, XUI_DOCUMENT_ROOT, 0, &list) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, list, 0, &first_item) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, first_item, 1, &inner_list) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, inner_list, 0, &inner_item) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, inner_item, 0, &inner_paragraph) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, inner_paragraph, 0, &inner_text) == XUI_OK);
        xuiDocumentSnapshotRelease(snap);
        ed.tView.pDocument = document;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
        CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
        node = accessible_node_by_id(editor, first_item);
        CHECK(node.iRole == XUI_ACCESSIBLE_ROLE_CHECKBOX && node.sValue &&
            !strcmp(node.sValue, "first\ninner") &&
            (node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_TOGGLE)));
        node = accessible_node_by_id(editor, list);
        CHECK(node.sValue && !strcmp(node.sValue, "first\ninner\nsecond"));
        local.iAnchor = 6; local.iCaret = 11;
        CHECK(xuiWidgetPerformAccessibleAction(editor, first_item,
            XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
        CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
            range.tAnchor.iNodeId == inner_text && range.tAnchor.iOffset == 0 &&
            range.tCaret.iNodeId == inner_text && range.tCaret.iOffset == 5);
        node = accessible_node_by_id(editor, first_item);
        CHECK(node.iTextStart == 6 && node.iTextEnd == 11);
        source(document, markdown);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("Document structured accessibility: multi-block Cell, row/table TSV, nested task list, readonly and selection action coexistence passed");
}
static void select_table_text(xui_widget editor, xui_doc_node_id text)
{
    xui_doc_range_t range;
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK);
    range.tAnchor = range.tCaret;
    range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_TEXT;
    range.tAnchor.iNodeId = range.tCaret.iNodeId = text;
    range.tAnchor.iOffset = range.tCaret.iOffset = 0;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
}
static void table_navigation_editor_cases(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_doc_editor_desc_t desc = {0}; xui_widget editor; xui_doc_range_t selection;
    xui_doc_command_state_t state = {0}; xui_event_t key = {0};
    xui_doc_node_id table, merged, cells[5]; uint64_t revision; unsigned step;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 3, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnMergeCells(txn, table, 0, 0, 1, 2, &merged) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    cells[0] = table_cell_text(snapshot, table, 0, 0);
    cells[1] = table_cell_text(snapshot, table, 0, 1);
    cells[2] = table_cell_text(snapshot, table, 1, 0);
    cells[3] = table_cell_text(snapshot, table, 1, 1);
    cells[4] = table_cell_text(snapshot, table, 1, 2);
    xuiDocumentSnapshotRelease(snapshot);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = document; desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 300, 150}) == XUI_OK);
    select_table_text(editor, cells[0]); revision = xuiDocumentGetRevision(document);
    key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor; key.iKey = XUI_KEY_DOWN;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    CHECK(selection.tCaret.iNodeId != cells[2]);
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    CHECK(selection.tCaret.iNodeId == cells[2]);
    select_table_text(editor, cells[1]);
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[4]);
    select_table_text(editor, cells[0]);
    state.iSize = sizeof(state);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_PREVIOUS_CELL, &state) == XUI_OK && !state.bEnabled);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_NEXT_CELL, &state) == XUI_OK && state.bEnabled);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_NEXT_CELL) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[1]);
    key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor; key.iKey = XUI_KEY_TAB;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[2]);
    key.iModifiers = XUI_MOD_SHIFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[1]);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    CHECK(xuiDocumentEditorCanExecute(editor, XUI_DOC_EDIT_TABLE_NEXT_CELL));
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_NEXT_CELL) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[2]);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    CHECK(xuiDocumentGetRevision(document) == revision);
    select_table_text(editor, cells[4]);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_NEXT_CELL, &state) == XUI_OK && !state.bEnabled);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_NEXT_CELL, &state) == XUI_OK && state.bEnabled);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_NEXT_CELL) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    { xui_doc_node_info_t info = {0}; info.iSize = sizeof(info);
      CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK && info.iChildCount == 3);
      cells[4] = table_cell_text(snapshot, table, 2, 0); }
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[4]);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    { xui_doc_node_info_t info = {0}; info.iSize = sizeof(info);
      CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK && info.iChildCount == 2); }
    xuiDocumentSnapshotRelease(snapshot);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 3, 2, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnMergeCells(txn, table, 0, 0, 2, 2, &merged) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    cells[0] = table_cell_text(snapshot, table, 0, 0);
    cells[1] = table_cell_text(snapshot, table, 2, 0);
    xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    select_table_text(editor, cells[0]);
    key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN;
    key.pTarget = editor; key.iModifiers = 0; key.iKey = XUI_KEY_DOWN;
    for (step = 0; step < 8; step++) {
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
        if (selection.tCaret.iNodeId == cells[1]) break;
    }
    CHECK(step < 8);
    select_table_text(editor, cells[0]);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_NEXT_CELL) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[1]);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_PREVIOUS_CELL) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[0]);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    {
        xui_doc_desc_t md = {0}; md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        md.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&md, &document) == XUI_OK);
    }
    CHECK(xuiDocumentLoadMarkdown(document, "| A | B |\n| - | - |\n| x | y |\n", 30) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &table) == XUI_OK);
    cells[0] = table_cell_text(snapshot, table, 0, 0);
    cells[1] = table_cell_text(snapshot, table, 0, 1);
    cells[2] = table_cell_text(snapshot, table, 1, 1);
    cells[3] = table_cell_text(snapshot, table, 1, 0);
    xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    select_table_text(editor, cells[0]); revision = xuiDocumentGetRevision(document);
    key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor;
    key.iModifiers = 0; key.iKey = XUI_KEY_DOWN;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[3]);
    key.iKey = XUI_KEY_UP;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[0]);
    selection.tAnchor.iOffset = selection.tCaret.iOffset = 1;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    key.iKey = XUI_KEY_RIGHT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[1]);
    key.iKey = XUI_KEY_LEFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[0]);
    select_table_text(editor, cells[0]);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    key.iKey = XUI_KEY_DOWN;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[3]);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    CHECK(xuiDocumentGetRevision(document) == revision);
    select_table_text(editor, cells[0]);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_NEXT_CELL) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[1]);
    CHECK(xuiDocumentGetRevision(document) == revision);
    select_table_text(editor, cells[2]); key.pTarget = editor; key.iModifiers = 0; key.iKey = XUI_KEY_TAB;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    { xui_doc_node_info_t info = {0}; info.iSize = sizeof(info);
      CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK && info.iChildCount == 3);
      cells[2] = table_cell_text(snapshot, table, 2, 0); }
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == cells[2]);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_NEXT_CELL, &state) == XUI_OK && !state.bEnabled);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    puts("DocumentEditor table navigation: merged cells, row transitions, Tab, Shift+Tab, read-only and Markdown VISUAL passed");
}
static void table_structure_editor_cases(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_doc_editor_desc_t desc = {0}; xui_widget editor; xui_doc_range_t selection;
    xui_doc_command_state_t state = {0}; xui_doc_node_info_t info = {0};
    xui_doc_node_id table, row, first, last, inserted, merged;
    state.iSize = sizeof(state); info.iSize = sizeof(info);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 2, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    first = table_cell_text(snapshot, table, 0, 0); xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    select_table_text(editor, first);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_INSERT_ROW_AFTER, &state) == XUI_OK && state.bEnabled);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_SPLIT_CELL, &state) == XUI_OK && !state.bEnabled);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_INSERT_ROW_AFTER, &state) == XUI_OK && !state.bEnabled);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_INSERT_ROW_AFTER) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK && info.iChildCount == 3);
    inserted = table_cell_text(snapshot, table, 1, 0); xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == inserted);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK && info.iChildCount == 2);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == first);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == inserted);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_INSERT_COLUMN_AFTER) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 1, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, row, &info) == XUI_OK && info.iChildCount == 3);
    inserted = table_cell_text(snapshot, table, 1, 1); xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == inserted);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_DELETE_COLUMN) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 1, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, row, &info) == XUI_OK && info.iChildCount == 2);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_INSERT_ROW_BEFORE) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK && info.iChildCount == 4);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_DELETE_ROW) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK && info.iChildCount == 3);
    xuiDocumentSnapshotRelease(snapshot);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 2, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    first = table_cell_text(snapshot, table, 0, 0);
    last = table_cell_text(snapshot, table, 1, 1); xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    select_table_text(editor, first);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    selection.tCaret.iNodeId = last; selection.tCaret.iOffset = 0;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS, &state) == XUI_OK && state.bEnabled);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 0, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, row, &info) == XUI_OK && info.iChildCount == 1);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, row, 0, &merged) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, merged, &info) == XUI_OK &&
        info.tAttributes.iRowSpan == 2 && info.tAttributes.iColumnSpan == 2);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        selection.tAnchor.iNodeId == first && selection.tCaret.iNodeId == first);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 0, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, row, &info) == XUI_OK && info.iChildCount == 2);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        selection.tAnchor.iNodeId == first && selection.tCaret.iNodeId == last);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 3, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnMergeCells(txn, table, 0, 0, 1, 2, &merged) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    first = table_cell_text(snapshot, table, 0, 1);
    last = table_cell_text(snapshot, table, 1, 1); xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    select_table_text(editor, first);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    selection.tCaret.iNodeId = last; selection.tCaret.iOffset = 0;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS, &state) == XUI_OK &&
        !state.bEnabled && state.iDisabledReason == XUI_DOC_ERROR_SCHEMA);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS) == XUI_DOC_ERROR_SCHEMA);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 2, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnMergeCells(txn, table, 0, 0, 2, 2, &merged) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    first = table_cell_text(snapshot, table, 0, 0); xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    select_table_text(editor, first);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_SPLIT_CELL, &state) == XUI_OK && state.bEnabled);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_SPLIT_CELL) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, merged, &info) == XUI_OK &&
        info.tAttributes.iRowSpan == 1 && info.tAttributes.iColumnSpan == 1);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 1, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, row, &info) == XUI_OK && info.iChildCount == 2);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, merged, &info) == XUI_OK &&
        info.tAttributes.iRowSpan == 2 && info.tAttributes.iColumnSpan == 2);
    xuiDocumentSnapshotRelease(snapshot);
    select_table_text(editor, first);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_DELETE_ROW) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK && info.iChildCount == 1);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, merged, &info) == XUI_OK && info.tAttributes.iRowSpan == 1);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_DELETE_COLUMN) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, merged, &info) == XUI_OK && info.tAttributes.iColumnSpan == 1);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_DELETE_COLUMN) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    { int order;
      CHECK(xuiDocumentSnapshotComparePositions(snapshot, &selection.tCaret, &selection.tCaret, &order) == XUI_OK); }
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    {
        xui_doc_desc_t md = {0}; md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        md.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&md, &document) == XUI_OK);
    }
    CHECK(xuiDocumentLoadMarkdown(document, "| A | B |\n| - | - |\n| x | y |\n", 30) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &table) == XUI_OK);
    first = table_cell_text(snapshot, table, 1, 0); xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    select_table_text(editor, first);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_SPLIT_CELL, &state) == XUI_OK && !state.bEnabled &&
        state.iDisabledReason == XUI_DOC_ERROR_UNREPRESENTABLE);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    last = table_cell_text(snapshot, table, 1, 1); xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    selection.tCaret.iNodeId = last; selection.tCaret.iOffset = 0;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS, &state) == XUI_OK &&
        !state.bEnabled && state.iDisabledReason == XUI_DOC_ERROR_UNREPRESENTABLE);
    select_table_text(editor, first);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_INSERT_ROW_AFTER) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK && info.iChildCount == 3);
    inserted = table_cell_text(snapshot, table, 2, 0); xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK && selection.tCaret.iNodeId == inserted);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_INSERT_COLUMN_AFTER) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 0, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, row, &info) == XUI_OK && info.iChildCount == 3);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_DELETE_COLUMN) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 0, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, row, &info) == XUI_OK && info.iChildCount == 2);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_DELETE_ROW) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK && info.iChildCount == 2);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK && info.iChildCount == 3);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    { uint64_t offset; int mapping;
      CHECK(xuiDocumentPositionToSource(snapshot, &selection.tCaret, &offset, &mapping) == XUI_OK &&
          mapping == XUI_DOC_MAP_SYNTAX); }
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_DELETE_ROW, &state) == XUI_OK && !state.bEnabled);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    puts("DocumentEditor table structure: rich/Markdown row and column commands, rich merge/split, invalid span, selection and Undo passed");
}
static void table_matrix_editor_cases(xui_context context, xui_test_proxy_state_t* proxy)
{
    unsigned mode;
    for (mode = 0; mode < 2; mode++) {
        xui_document d; xui_document_transaction txn; xui_document_snapshot snapshot;
        xui_doc_editor_desc_t desc = {0}; xui_doc_desc_t md = {0};
        xui_doc_node_info_t info = {0}; xui_doc_range_t selection;
        xui_widget editor; uint64_t table, first, last, row, copied;
        char contents[16];
        if (!mode) {
            CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
            CHECK(xuiDocumentBeginTransaction(d, NULL, &txn) == XUI_OK);
            CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 2, 0, &table) == XUI_OK);
            CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
        } else {
            const char* source_text = "| A | B |\n| - | - |\n| x | y |\n";
            md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
            md.iMarkdownDialect = XUI_MD_GFM;
            CHECK(xuiDocumentCreate(&md, &d) == XUI_OK);
            CHECK(xuiDocumentLoadMarkdown(d, source_text, strlen(source_text)) == XUI_OK);
        }
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        if (mode) CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &table) == XUI_OK);
        first = table_cell_text(snapshot, table, mode ? 1 : 0, mode ? 0 : 1);
        xuiDocumentSnapshotRelease(snapshot);
        desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
        desc.tView.pDocument = d; desc.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        select_table_text(editor, first);
        CHECK(xuiTestProxySetClipboardText(proxy, "A\tB\nC\tD") == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK); info.iSize = sizeof(info);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK &&
            info.iChildCount == (mode ? 3 : 2));
        CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 0, &row) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, row, &info) == XUI_OK &&
            info.iChildCount == (mode ? 2 : 3));
        last = table_cell_text(snapshot, table, mode ? 2 : 1, mode ? 1 : 2);
        CHECK(xuiDocumentSnapshotCopyText(snapshot, last, contents, sizeof(contents), &copied) == XUI_OK &&
            copied == 1 && !strcmp(contents, "D"));
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
            selection.tCaret.iNodeId == last && selection.tCaret.iOffset == 1);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, table, &info) == XUI_OK && info.iChildCount == 2);
        xuiDocumentSnapshotRelease(snapshot);
        if (mode) source(d, "| A | B |\n| - | - |\n| x | y |\n");
        select_table_text(editor, first);
        CHECK(xuiTestProxySetClipboardText(proxy, mode ?
            "\"say \"\"hi\"\"\"\t\"p|q\"" : "\"up\nlow\"\tend") == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        last = table_cell_text(snapshot, table, mode ? 1 : 0, mode ? 0 : 1);
        CHECK(xuiDocumentSnapshotCopyText(snapshot, last, contents, sizeof(contents), &copied) == XUI_OK &&
            !strcmp(contents, mode ? "say \"hi\"" : "up\nlow"));
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        if (mode) {
            uint64_t revision = xuiDocumentGetRevision(d);
            select_table_text(editor, first);
            CHECK(xuiTestProxySetClipboardText(proxy, "\"line1\nline2\"\tB") == XUI_OK);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_DOC_ERROR_UNREPRESENTABLE);
            CHECK(xuiDocumentGetRevision(d) == revision);
            source(d, "| A | B |\n| - | - |\n| x | y |\n");
        }
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    puts("DocumentEditor table matrix paste: quoted TSV, rich/Markdown clipboard, growth, caret and one-step Undo passed");
}
static void expect_table_matrix_empty(xui_document document, xui_doc_node_id table,
    uint32_t rows, uint32_t columns)
{
    xui_document_snapshot snapshot; uint32_t row, column;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    for (row = 0; row < rows; row++) for (column = 0; column < columns; column++) {
        char contents[8]; uint64_t copied;
        CHECK(xuiDocumentSnapshotCopyText(snapshot,
            table_cell_text(snapshot, table, row, column), contents,
            sizeof(contents), &copied) == XUI_OK && !copied && !contents[0]);
    }
    xuiDocumentSnapshotRelease(snapshot);
}
static void table_rectangle_copy_cases(xui_context context,
    xui_test_proxy_state_t* proxy, xui_surface target)
{
    xui_document d; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_doc_editor_desc_t desc = {0}; xui_widget editor;
    xui_doc_node_id table, first_row, last_row, first_cell, last_cell, last, first_paragraph;
    xui_doc_attributes_t paragraph_style = {0};
    uint32_t paragraph_background = XUI_COLOR_RGBA(237, 94, 54, 255);
    uint32_t table_border = XUI_COLOR_RGBA(11, 142, 192, 255);
    uint32_t frame_border = XUI_COLOR_RGBA(83, 56, 173, 255);
    xui_doc_cell_hit_t first = {0}, corner = {0};
    xui_doc_table_selection_t selected = {0}; xui_doc_command_state_t state = {0};
    xui_event_t context_menu = {0}; xui_widget menu = NULL;
    xui_doc_node_info_t info = {0}; char merged_copy[128];
    xui_rect_i_t damage = {0, 0, 640, 480}; xui_surface cache;
    uint64_t revision;
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 2, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnPasteTableMatrix(txn, table, 0, 0,
        "\"A\tB\"\t\"say \"\"yes\"\"\"\nC\tD",
        strlen("\"A\tB\"\t\"say \"\"yes\"\"\"\nC\tD"), &last) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 0, &first_row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 1, &last_row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, first_row, 0, &first_cell) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, last_row, 1, &last_cell) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, first_cell, 0, &first_paragraph) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    paragraph_style.iBackgroundColor = paragraph_background;
    CHECK(xuiDocumentBeginTransaction(d, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnSetAttributes(txn, first_paragraph, &paragraph_style) == XUI_OK &&
        xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = d; desc.tView.iSelectionColor = UINT32_C(0xff42babe);
    desc.tView.tRenderer.iSize = sizeof(desc.tView.tRenderer);
    desc.tView.tRenderer.iBorderColor = frame_border;
    desc.tView.tRenderer.iTableBorderColor = table_border;
    desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiInputViewport(context, 320, 240) == XUI_OK);
    first.iSize = corner.iSize = sizeof(first);
    CHECK(xuiDocumentViewGetCellRect(editor, first_cell, &first) == XUI_OK);
    CHECK(xuiDocumentViewGetCellRect(editor, last_cell, &corner) == XUI_OK);
    revision = xuiDocumentGetRevision(d);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_MOVE,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), 0, XUI_MOD_ALT);
    selected.iSize = sizeof(selected);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iTableId == table && selected.iRow == 0 && selected.iColumn == 0 &&
        selected.iRows == 2 && selected.iColumns == 2);
    context_menu.iSize = sizeof(context_menu);
    context_menu.iType = XUI_EVENT_CONTEXT_MENU; context_menu.pTarget = editor;
    context_menu.iPointerType = XUI_POINTER_TYPE_MOUSE;
    context_menu.iButton = XUI_POINTER_BUTTON_RIGHT;
    context_menu.fX = (int)(first.tBounds.x + first.tBounds.width / 2);
    context_menu.fY = (int)(first.tBounds.y + first.tBounds.height / 2);
    CHECK(xuiDispatchEvent(context, &context_menu) == XUI_OK);
    menu = xuiDocumentEditorGetMenuWidget(editor);
    CHECK(menu && xuiMenuIsOpen(menu) && xuiMenuGetItem(menu, 4)->iValue == XUI_DOC_EDIT_COPY &&
        (xuiMenuGetItemState(menu, 4) & XUI_MENU_ITEM_ENABLED) &&
        xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iRows == 2 && selected.iColumns == 2);
    CHECK(xuiMenuGetItemCount(menu) == 18 &&
        xuiMenuGetItem(menu, 16)->iValue == XUI_DOC_EDIT_TABLE_MERGE_CELLS &&
        !strcmp(xuiMenuGetItem(menu, 16)->sText,
            xuiTranslate(context, XUI_TR_RICH_MERGE_CELLS)) &&
        (xuiMenuGetItemState(menu, 16) & XUI_MENU_ITEM_ENABLED) &&
        xuiMenuGetItem(menu, 17)->iValue == XUI_DOC_EDIT_TABLE_SPLIT_CELL &&
        !(xuiMenuGetItemState(menu, 17) & XUI_MENU_ITEM_ENABLED));
    CHECK(xuiMenuSetHoverIndex(menu, 16) == XUI_OK &&
        xuiMenuCommitHover(menu) == XUI_EVENT_DISPATCH_STOP &&
        xuiDocumentGetRevision(d) > revision &&
        xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    { char* restored; uint64_t bytes;
      CHECK(xuiDocumentSnapshotCopyTableMatrix(snapshot, table, 0, 0, 2, 2,
          &restored, &bytes) == XUI_OK &&
          !strcmp(restored, "\"A\tB\"\t\"say \"\"yes\"\"\"\nC\tD"));
      xuiDocumentFreeBuffer(restored); }
    xuiDocumentSnapshotRelease(snapshot);
    revision = xuiDocumentGetRevision(d);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_MOVE,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), 0, XUI_MOD_ALT);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iRows == 2 && selected.iColumns == 2);
    CHECK(xuiUpdate(context, .016f) == XUI_OK && xuiRender(context, target, &damage, 1) == XUI_OK);
    cache = xuiWidgetGetCacheSurface(editor, xuiWidgetGetStateId(editor));
    CHECK(cache && xuiTestSurfaceGetRectFillColorCount(cache, desc.tView.iSelectionColor) == 4 &&
        xuiTestSurfaceGetRectFillColorCount(cache, paragraph_background) > 0 &&
        xuiTestSurfaceGetRectFillColorCount(cache, table_border) == 4 &&
        xuiTestSurfaceGetRectFillColorCount(cache, frame_border) == 1 &&
        xuiTestSurfaceGetLastRectFillOrder(cache, desc.tView.iSelectionColor) >
            xuiTestSurfaceGetLastRectFillOrder(cache, paragraph_background));
    state.iSize = sizeof(state);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_COPY, &state) == XUI_OK && state.bEnabled);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_COPY) == XUI_OK);
    CHECK(xuiEditHasSelection(editor) && xuiEditCopy(editor) == XUI_OK);
    CHECK(!strcmp(xuiTestProxyGetClipboardText(proxy),
        "\"A\tB\"\t\"say \"\"yes\"\"\"\nC\tD"));
    CHECK(xuiDocumentGetRevision(d) == revision);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), 0);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), 0, XUI_MOD_ALT);
    state.iSize = sizeof(state);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_DELETE, &state) == XUI_OK &&
        state.bEnabled);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_DELETE) == XUI_OK &&
        xuiDocumentGetRevision(d) > revision);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND);
    expect_table_matrix_empty(d, table, 2, 2);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), 0, XUI_MOD_ALT);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BACKSPACE) == XUI_OK);
    expect_table_matrix_empty(d, table, 2, 2);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), 0, XUI_MOD_ALT);
    CHECK(xuiEditDeleteSelection(editor) == XUI_OK);
    expect_table_matrix_empty(d, table, 2, 2);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), 0, XUI_MOD_ALT);
    CHECK(xuiDocumentEditorInsertText(editor, "", 0) == XUI_OK);
    expect_table_matrix_empty(d, table, 2, 2);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), 0, XUI_MOD_ALT);
    CHECK(xuiEditCut(editor) == XUI_OK);
    CHECK(!strcmp(xuiTestProxyGetClipboardText(proxy),
        "\"A\tB\"\t\"say \"\"yes\"\"\"\nC\tD"));
    expect_table_matrix_empty(d, table, 2, 2);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), 0, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), 0, XUI_MOD_ALT);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_DELETE, &state) == XUI_OK &&
        !state.bEnabled && state.iDisabledReason == XUI_ERROR_INVALID_STATE);
    CHECK(xuiDocumentEditorInsertText(editor, "", 0) == XUI_ERROR_INVALID_STATE &&
        xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_DELETE) == XUI_ERROR_INVALID_STATE &&
        xuiDocumentGetRevision(d) == revision);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    CHECK(xuiTestProxySetClipboardText(proxy, "Z") == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    first_cell = table_cell_text(snapshot, table, 0, 0);
    { char text[16]; uint64_t length;
      CHECK(xuiDocumentSnapshotCopyText(snapshot, first_cell, text, sizeof(text), &length) == XUI_OK &&
          length == 1 && !strcmp(text, "Z")); }
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);

    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 3, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnPasteTableMatrix(txn, table, 0, 0,
        "a\tb\tc\nd\te\tf", 11, &last) == XUI_OK);
    CHECK(xuiDocumentTxnMergeCells(txn, table, 0, 1, 2, 1, &last) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 0, &first_row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, first_row, 0, &first_cell) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, first_row, 2, &last_cell) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = d;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 360, 240}) == XUI_OK);
    CHECK(xuiInputViewport(context, 360, 240) == XUI_OK);
    CHECK(xuiDocumentViewGetCellRect(editor, first_cell, &first) == XUI_OK);
    CHECK(xuiDocumentViewGetCellRect(editor, last_cell, &corner) == XUI_OK);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), 0, XUI_MOD_ALT);
    selected.iSize = sizeof(selected);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iRow == 0 && selected.iColumn == 0 &&
        selected.iRows == 2 && selected.iColumns == 3);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_COPY) == XUI_OK &&
        strstr(xuiTestProxyGetClipboardText(proxy), "\t\t") != NULL);
    CHECK(strlen(xuiTestProxyGetClipboardText(proxy)) < sizeof(merged_copy));
    strcpy(merged_copy, xuiTestProxyGetClipboardText(proxy));
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CUT) == XUI_OK &&
        !strcmp(xuiTestProxyGetClipboardText(proxy), merged_copy));
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, last, &info) == XUI_OK &&
        info.tAttributes.iRowSpan == 2);
    { char* cleared; uint64_t bytes;
      CHECK(xuiDocumentSnapshotCopyTableMatrix(snapshot, table, 0, 0, 2, 3,
          &cleared, &bytes) == XUI_OK && !strcmp(cleared, "\t\t\n\t\t"));
      xuiDocumentFreeBuffer(cleared); }
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    { char* restored; uint64_t bytes;
      CHECK(xuiDocumentSnapshotCopyTableMatrix(snapshot, table, 0, 0, 2, 3,
          &restored, &bytes) == XUI_OK && !strcmp(restored, merged_copy));
      xuiDocumentFreeBuffer(restored); }
    xuiDocumentSnapshotRelease(snapshot);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);

    { xui_doc_desc_t md = {0}; const char* text = "| A | B |\n| - | - |\n| x | y |\n";
      md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
      md.iMarkdownDialect = XUI_MD_GFM;
      CHECK(xuiDocumentCreate(&md, &d) == XUI_OK);
      CHECK(xuiDocumentLoadMarkdown(d, text, strlen(text)) == XUI_OK); }
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &table) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 1, &last_row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, last_row, 0, &first_cell) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, last_row, 1, &last_cell) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = d;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiInputViewport(context, 320, 240) == XUI_OK);
    CHECK(xuiDocumentViewGetCellRect(editor, first_cell, &first) == XUI_OK);
    CHECK(xuiDocumentViewGetCellRect(editor, last_cell, &corner) == XUI_OK);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), 0, XUI_MOD_ALT);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iRow == 1 && selected.iColumn == 0 && selected.iRows == 1 && selected.iColumns == 2);
    context_menu.pTarget = editor;
    context_menu.fX = (int)(first.tBounds.x + first.tBounds.width / 2);
    context_menu.fY = (int)(first.tBounds.y + first.tBounds.height / 2);
    CHECK(xuiDispatchEvent(context, &context_menu) == XUI_OK);
    menu = xuiDocumentEditorGetMenuWidget(editor);
    CHECK(menu && xuiMenuIsOpen(menu) && xuiMenuGetItemCount(menu) == 18 &&
        xuiMenuGetItem(menu, 10)->iValue == XUI_DOC_EDIT_TABLE_INSERT_ROW_BEFORE &&
        (xuiMenuGetItemState(menu, 10) & XUI_MENU_ITEM_ENABLED) &&
        xuiMenuGetItem(menu, 16)->iValue == XUI_DOC_EDIT_TABLE_MERGE_CELLS &&
        !(xuiMenuGetItemState(menu, 16) & XUI_MENU_ITEM_ENABLED) &&
        xuiMenuGetItem(menu, 17)->iValue == XUI_DOC_EDIT_TABLE_SPLIT_CELL &&
        !(xuiMenuGetItemState(menu, 17) & XUI_MENU_ITEM_ENABLED));
    CHECK(xuiMenuClose(menu) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_COPY) == XUI_OK &&
        !strcmp(xuiTestProxyGetClipboardText(proxy), "x\ty"));
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_DELETE) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    { char contents[8]; uint64_t bytes;
      CHECK(xuiDocumentSnapshotCopyText(snapshot, first_cell, contents,
          sizeof(contents), &bytes) == XUI_OK && !bytes);
      CHECK(xuiDocumentSnapshotCopyText(snapshot, last_cell, contents,
          sizeof(contents), &bytes) == XUI_OK && !bytes); }
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "| A | B |\n| - | - |\n| x | y |\n");
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND);
    xuiWidgetDestroy(editor);
    { xui_doc_view_desc_t view = {0};
      view.iSize = sizeof(view); view.pDocument = d;
      CHECK(xuiDocumentViewCreate(context, &view, &editor) == XUI_OK &&
          xuiSetRootWidget(context, editor) == XUI_OK); }
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiInputViewport(context, 320, 240) == XUI_OK);
    CHECK(xuiDocumentViewGetCellRect(editor, first_cell, &first) == XUI_OK);
    CHECK(xuiDocumentViewGetCellRect(editor, last_cell, &corner) == XUI_OK);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), 0, XUI_MOD_ALT);
    CHECK(xuiEditHasSelection(editor) && xuiEditCopy(editor) == XUI_OK &&
        !strcmp(xuiTestProxyGetClipboardText(proxy), "x\ty"));
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    puts("DocumentEditor Alt-drag rectangle: rich/Markdown TSV, merged-span expansion, fill and paste passed");
    puts("DocumentEditor table context menu: Rich merge/Undo and Markdown command states passed");
}
static void select_cell_rectangle(xui_context context, xui_widget editor,
    xui_doc_node_id first_cell, xui_doc_node_id last_cell)
{
    xui_doc_cell_hit_t first = {0}, last = {0};
    first.iSize = last.iSize = sizeof(first);
    CHECK(xuiDocumentViewGetCellRect(editor, first_cell, &first) == XUI_OK);
    CHECK(xuiDocumentViewGetCellRect(editor, last_cell, &last) == XUI_OK);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(last.tBounds.x + last.tBounds.width / 2),
        (int)(last.tBounds.y + last.tBounds.height / 2), 0, XUI_MOD_ALT);
}
static void expect_table_rectangle(xui_widget view, xui_doc_node_id table,
    uint32_t row, uint32_t column, uint32_t rows, uint32_t columns)
{
    xui_doc_table_selection_t selected = {0}; selected.iSize = sizeof(selected);
    CHECK(xuiDocumentViewGetTableSelection(view, &selected) == XUI_OK &&
        selected.iTableId == table && selected.iRow == row &&
        selected.iColumn == column && selected.iRows == rows &&
        selected.iColumns == columns);
}
static void table_rectangle_enter_cases(xui_context context)
{
    xui_document d; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_doc_editor_desc_t desc = {0}; xui_doc_command_state_t state = {0};
    xui_doc_table_selection_t selected = {0}; xui_doc_range_t selection;
    xui_doc_node_info_t info = {0}; xui_widget editor;
    xui_doc_node_id table, row, first_cell, last_cell, second_paragraph, last;
    uint64_t revision, bytes; char contents[32]; unsigned mode;
    const char* markdown = "| A | B |\n| - | - |\n| x | y |\n";
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.iMode = XUI_DOC_VISUAL; state.iSize = sizeof(state);
    selected.iSize = sizeof(selected); info.iSize = sizeof(info);
    for (mode = 0; mode < 2; mode++) {
        if (!mode) {
            CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
            CHECK(xuiDocumentBeginTransaction(d, NULL, &txn) == XUI_OK);
            CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 2, 0, &table) == XUI_OK);
            CHECK(xuiDocumentTxnPasteTableMatrix(txn, table, 0, 0,
                "A\tB\nC\tD", 7, &last) == XUI_OK);
            CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
        } else {
            xui_doc_desc_t md = {0}; md.iSize = sizeof(md);
            md.iProfile = XUI_DOCUMENT_MARKDOWN; md.iMarkdownDialect = XUI_MD_GFM;
            CHECK(xuiDocumentCreate(&md, &d) == XUI_OK);
            CHECK(xuiDocumentLoadMarkdown(d, markdown, strlen(markdown)) == XUI_OK);
        }
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        if (mode) CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &table) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, table, mode ? 1 : 0, &row) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, row, 0, &first_cell) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, row, 1, &last_cell) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        desc.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiInputViewport(context, 320, 240) == XUI_OK);
        select_cell_rectangle(context, editor, first_cell, last_cell);
        CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
            selected.iTableId == table && selected.iRow == (mode ? 1u : 0u) &&
            selected.iColumn == 0 && selected.iRows == 1 && selected.iColumns == 2);
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_ENTER, &state) == XUI_OK);
        if (!mode) {
            CHECK(state.bEnabled && state.iDisabledReason == XUI_OK);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ENTER) == XUI_OK &&
                xuiDocumentGetRevision(d) > revision);
            CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND);
            CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetNode(snapshot, first_cell, &info) == XUI_OK &&
                info.iChildCount == 2);
            CHECK(xuiDocumentSnapshotGetChild(snapshot, first_cell, 1, &second_paragraph) == XUI_OK);
            CHECK(xuiDocumentSnapshotCopyText(snapshot, table_cell_text(snapshot, table, 0, 1),
                contents, sizeof(contents), &bytes) == XUI_OK && !strcmp(contents, "B"));
            CHECK(xuiDocumentSnapshotCopyText(snapshot, table_cell_text(snapshot, table, 1, 0),
                contents, sizeof(contents), &bytes) == XUI_OK && !strcmp(contents, "C"));
            CHECK(xuiDocumentSnapshotCopyText(snapshot, table_cell_text(snapshot, table, 1, 1),
                contents, sizeof(contents), &bytes) == XUI_OK && !strcmp(contents, "D"));
            CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
                xuiDocumentSnapshotGetNode(snapshot, selection.tCaret.iNodeId, &info) == XUI_OK &&
                info.iParentId == second_paragraph && selection.tCaret.iOffset == 0);
            xuiDocumentSnapshotRelease(snapshot);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetNode(snapshot, first_cell, &info) == XUI_OK && info.iChildCount == 1);
            CHECK(xuiDocumentSnapshotCopyText(snapshot, table_cell_text(snapshot, table, 0, 0),
                contents, sizeof(contents), &bytes) == XUI_OK && !strcmp(contents, "A"));
            xuiDocumentSnapshotRelease(snapshot);
            select_cell_rectangle(context, editor, first_cell, last_cell);
            CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
            revision = xuiDocumentGetRevision(d);
            CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_ENTER, &state) == XUI_OK &&
                !state.bEnabled && state.iDisabledReason == XUI_ERROR_INVALID_STATE);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ENTER) == XUI_ERROR_INVALID_STATE &&
                xuiDocumentGetRevision(d) == revision);
        } else {
            CHECK(!state.bEnabled && state.iDisabledReason == XUI_DOC_ERROR_UNREPRESENTABLE);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ENTER) == XUI_DOC_ERROR_UNREPRESENTABLE &&
                xuiDocumentGetRevision(d) == revision);
            CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
                selected.iTableId == table);
            source(d, markdown);
        }
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 2, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnPasteTableMatrix(txn, table, 0, 0, "A\tB\nC\tD", 7, &last) == XUI_OK);
    CHECK(xuiDocumentTxnMergeCells(txn, table, 0, 0, 1, 2, &first_cell) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 1, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, row, 1, &last_cell) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = d;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiInputViewport(context, 320, 240) == XUI_OK);
    select_cell_rectangle(context, editor, first_cell, last_cell);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iRow == 0 && selected.iColumn == 0 &&
        selected.iRows == 2 && selected.iColumns == 2);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ENTER) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, first_cell, &info) == XUI_OK && info.iChildCount == 2);
    CHECK(xuiDocumentSnapshotCopyText(snapshot, table_cell_text(snapshot, table, 1, 0),
        contents, sizeof(contents), &bytes) == XUI_OK && !strcmp(contents, "C"));
    CHECK(xuiDocumentSnapshotCopyText(snapshot, table_cell_text(snapshot, table, 1, 1),
        contents, sizeof(contents), &bytes) == XUI_OK && !strcmp(contents, "D"));
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, first_cell, &info) == XUI_OK &&
        info.tAttributes.iColumnSpan == 2 && info.iChildCount == 2);
    CHECK(xuiDocumentSnapshotCopyText(snapshot, table_cell_text(snapshot, table, 0, 0),
        contents, sizeof(contents), &bytes) == XUI_OK && !strcmp(contents, "A"));
    CHECK(xuiDocumentSnapshotGetChild(snapshot, first_cell, 1, &second_paragraph) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, second_paragraph, 0, &last) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopyText(snapshot, last, contents, sizeof(contents), &bytes) == XUI_OK &&
        !strcmp(contents, "B"));
    xuiDocumentSnapshotRelease(snapshot);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    puts("DocumentEditor rectangle Enter: rich top-left split, merged span, one-step Undo, read-only and Markdown refusal passed");
}
static void table_rectangle_merge_cases(xui_context context)
{
    xui_document d; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_doc_editor_desc_t desc = {0}; xui_doc_command_state_t state = {0};
    xui_doc_table_selection_t selected = {0}; xui_doc_node_info_t info = {0};
    xui_widget editor; xui_doc_node_id table, row, first, last, text_ids[4], ignored;
    uint64_t revision; int i;
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.iMode = XUI_DOC_VISUAL; state.iSize = sizeof(state);
    selected.iSize = sizeof(selected); info.iSize = sizeof(info);
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 2, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnPasteTableMatrix(txn, table, 0, 0,
        "A\tB\nC\tD", 7, &ignored) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 0, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, row, 0, &first) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 1, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, row, 1, &last) == XUI_OK);
    for (i = 0; i < 4; i++) text_ids[i] = table_cell_text(snapshot,
        table, (uint32_t)i / 2, (uint32_t)i % 2);
    xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = d;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiInputViewport(context, 320, 240) == XUI_OK);
    select_cell_rectangle(context, editor, first, last);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iTableId == table && selected.iRow == 0 && selected.iColumn == 0 &&
        selected.iRows == 2 && selected.iColumns == 2);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS,
        &state) == XUI_OK && state.bEnabled);
    revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS) == XUI_OK &&
        xuiDocumentGetRevision(d) > revision);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, first, &info) == XUI_OK &&
        info.tAttributes.iRowSpan == 2 && info.tAttributes.iColumnSpan == 2);
    for (i = 0; i < 4; i++)
        CHECK(xuiDocumentSnapshotGetNode(snapshot, text_ids[i], &info) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    select_cell_rectangle(context, editor, first, first);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS,
        &state) == XUI_OK && !state.bEnabled &&
        state.iDisabledReason == XUI_ERROR_INVALID_STATE);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iTableId == table && selected.iRow == 0 && selected.iColumn == 0 &&
        selected.iRows == 2 && selected.iColumns == 2);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 0, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, row, &info) == XUI_OK && info.iChildCount == 2);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 1, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, row, &info) == XUI_OK && info.iChildCount == 2);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iRows == 2 && selected.iColumns == 2);
    select_cell_rectangle(context, editor, last, first);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS,
        &state) == XUI_OK && !state.bEnabled &&
        state.iDisabledReason == XUI_ERROR_INVALID_STATE);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS) ==
        XUI_ERROR_INVALID_STATE && xuiDocumentGetRevision(d) == revision);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS,
        &state) == XUI_OK && state.bEnabled);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iRows == 2 && selected.iColumns == 2);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);

    {
        static const char markdown[] = "| A | B |\n| - | - |\n| x | y |\n";
        xui_doc_desc_t md = {0}; md.iSize = sizeof(md);
        md.iProfile = XUI_DOCUMENT_MARKDOWN; md.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(d, markdown, sizeof(markdown) - 1) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &table) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 1, &row) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, row, 0, &first) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, row, 1, &last) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        desc.tView.pDocument = d;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        select_cell_rectangle(context, editor, first, last);
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS,
            &state) == XUI_OK && !state.bEnabled &&
            state.iDisabledReason == XUI_DOC_ERROR_UNREPRESENTABLE);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS) ==
            XUI_DOC_ERROR_UNREPRESENTABLE && xuiDocumentGetRevision(d) == revision);
        source(d, markdown);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    puts("DocumentEditor rectangle merge: rich Alt-drag, content, Undo, read-only and Markdown refusal passed");
}
static void table_programmatic_selection_cases(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_doc_editor_desc_t desc = {0}; xui_doc_command_state_t command = {0};
    xui_doc_table_selection_t request = {0}, selected = {0};
    xui_doc_range_t caret, after; xui_widget editor;
    xui_doc_node_id table, merged, nested; uint64_t revision;
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.iMode = XUI_DOC_VISUAL; command.iSize = sizeof(command);
    request.iSize = selected.iSize = sizeof(request);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 3, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnMergeCells(txn, table, 0, 0, 1, 2, &merged) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    revision = xuiDocumentGetRevision(document);
    request.iTableId = table; request.iRow = 0; request.iColumn = 1;
    request.iRows = 2; request.iColumns = 1;
    CHECK(xuiDocumentViewSetTableSelection(editor, &request) == XUI_OK);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iTableId == table && selected.iRow == 0 && selected.iColumn == 0 &&
        selected.iRows == 2 && selected.iColumns == 2);
    { xui_accessible_node_t cell = {0}; int i, found = 0;
      for (i = 0; i < xuiWidgetGetAccessibleNodeCount(editor); i++) {
          cell.iSize = sizeof(cell);
          CHECK(xuiWidgetGetAccessibleNode(editor, i, &cell) == XUI_OK);
          if (cell.iId == merged) {
              CHECK(cell.iRole == XUI_ACCESSIBLE_ROLE_CELL &&
                  cell.iRow == 0 && cell.iColumn == 0 &&
                  cell.iRowCount == 1 && cell.iColumnCount == 2 &&
                  (cell.iState & XUI_ACCESSIBLE_STATE_SELECTABLE) &&
                  (cell.iState & XUI_ACCESSIBLE_STATE_SELECTED) &&
                  (cell.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
              found = 1;
          }
      }
      CHECK(found); }
    CHECK(xuiWidgetPerformAccessibleAction(editor, merged,
        XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iTableId == table && selected.iRows == 1 && selected.iColumns == 2);
    CHECK(xuiDocumentViewSetTableSelection(editor, &request) == XUI_OK);
    CHECK(xuiEditHasSelection(editor));
    CHECK(xuiDocumentViewGetSelection(editor, &caret) == XUI_OK &&
        caret.tCaret.iNodeId != 1 && caret.tCaret.iRevision == revision &&
        caret.tCaret.iNodeId == caret.tAnchor.iNodeId);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS,
        &command) == XUI_OK && command.bEnabled);
    request.iColumns = 4;
    CHECK(xuiDocumentViewSetTableSelection(editor, &request) == XUI_ERROR_INVALID_ARGUMENT);
    request.iColumns = 1; request.iTableId = table + 99999;
    CHECK(xuiDocumentViewSetTableSelection(editor, &request) == XUI_ERROR_INVALID_ARGUMENT);
    request.iTableId = table; request.iSize = 0;
    CHECK(xuiDocumentViewSetTableSelection(editor, &request) == XUI_ERROR_INVALID_ARGUMENT);
    request.iSize = sizeof(request);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iTableId == table && selected.iColumn == 0 && selected.iColumns == 2);
    CHECK(xuiDocumentViewGetSelection(editor, &after) == XUI_OK &&
        after.tCaret.iNodeId == caret.tCaret.iNodeId &&
        after.tCaret.iOffset == caret.tCaret.iOffset &&
        xuiDocumentGetRevision(document) == revision);
    CHECK(xuiDocumentViewSetTableSelection(editor, NULL) == XUI_OK &&
        xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiDocumentViewGetSelection(editor, &after) == XUI_OK &&
        after.tCaret.iNodeId == caret.tCaret.iNodeId &&
        after.tCaret.iOffset == caret.tCaret.iOffset);
    request.iRow = request.iColumn = 0; request.iRows = 2; request.iColumns = 3;
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    CHECK(xuiDocumentViewSetTableSelection(editor, &request) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS,
        &command) == XUI_OK && !command.bEnabled);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS,
        &command) == XUI_OK && command.bEnabled);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS) == XUI_OK);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iTableId == table && selected.iRows == 2 && selected.iColumns == 3);
    CHECK(xuiDocumentViewGetSelection(editor, &after) == XUI_OK &&
        xuiDocumentViewSetSelection(editor, &after) == XUI_OK &&
        xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, merged, 0, 1, 1, 0, &nested) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentViewSetTableSelection(editor, &request) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &caret) == XUI_OK &&
        caret.tCaret.iNodeId == merged && caret.tCaret.iKind == XUI_DOC_POSITION_GAP);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS,
        &command) == XUI_OK && command.bEnabled);
    request.iTableId = nested; request.iRows = request.iColumns = 1;
    CHECK(xuiDocumentViewSetTableSelection(editor, &request) == XUI_OK &&
        xuiDocumentViewGetTableSelection(editor, &selected) == XUI_OK &&
        selected.iTableId == nested && selected.iRows == 1 && selected.iColumns == 1);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS,
        &command) == XUI_OK && !command.bEnabled);
    xuiWidgetDestroy(editor);
    desc.tView.bDisableSelection = 1;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiDocumentViewSetTableSelection(editor, &request) == XUI_ERROR_UNSUPPORTED);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    {
        static const char markdown[] = "| A | B |\n| - | - |\n| C | D |\n";
        xui_doc_desc_t md = {0}; md.iSize = sizeof(md);
        md.iProfile = XUI_DOCUMENT_MARKDOWN; md.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&md, &document) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(document, markdown, sizeof(markdown) - 1) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &table) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        desc.tView.pDocument = document; desc.tView.bDisableSelection = 0;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        request.iTableId = table; request.iRows = request.iColumns = 2;
        CHECK(xuiDocumentViewSetTableSelection(editor, &request) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS,
            &command) == XUI_OK && !command.bEnabled &&
            command.iDisabledReason == XUI_DOC_ERROR_UNREPRESENTABLE);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND &&
            xuiDocumentViewSetTableSelection(editor, &request) == XUI_ERROR_UNSUPPORTED);
        CHECK(xuiDocumentViewSetTableSelection(editor, NULL) == XUI_OK);
        source(document, markdown);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("DocumentEditor programmatic rectangle: span expansion, atomic validation, caret, merge Undo, mode/read-only gates passed");
}
static void table_keyboard_selection_cases(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_doc_editor_desc_t desc = {0}; xui_doc_table_selection_t request = {0};
    xui_doc_command_state_t command = {0}; xui_doc_range_t range;
    xui_event_t key = {0}; xui_widget editor;
    xui_doc_node_id table, merged, row, cell[3], text_ids[3], last;
    uint64_t revision; int i;
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.iMode = XUI_DOC_VISUAL; command.iSize = sizeof(command);
    key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN;
    key.iModifiers = XUI_MOD_ALT | XUI_MOD_SHIFT;
    request.iSize = sizeof(request);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 3, 3, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnPasteTableMatrix(txn, table, 0, 0,
        "A\tB\tC\nD\tE\tF\nG\tH\tI", 17, &last) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    text_ids[0] = table_cell_text(snapshot, table, 0, 0);
    text_ids[1] = table_cell_text(snapshot, table, 1, 1);
    text_ids[2] = table_cell_text(snapshot, table, 2, 2);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 2, &row) == XUI_OK);
    for (i = 0; i < 3; i++)
        CHECK(xuiDocumentSnapshotGetChild(snapshot, row, (uint64_t)i, &cell[i]) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK &&
        xuiInputViewport(context, 320, 240) == XUI_OK);
    key.pTarget = editor; revision = xuiDocumentGetRevision(document);
    select_table_text(editor, text_ids[1]);
    key.iKey = XUI_KEY_RIGHT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 1, 1, 1, 2);
    key.iKey = XUI_KEY_DOWN;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 1, 1, 2, 2);
    key.iKey = XUI_KEY_LEFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 1, 1, 2, 1);
    key.iKey = XUI_KEY_UP;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 1, 1, 1, 1);
    key.iKey = XUI_KEY_LEFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 1, 0, 1, 2);
    key.iKey = XUI_KEY_RIGHT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 1, 1, 1, 1);
    key.iKey = XUI_KEY_ESCAPE; key.iModifiers = 0;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    { xui_doc_table_selection_t selected = {0}; selected.iSize = sizeof(selected);
      CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND); }
    CHECK(!xuiEditHasSelection(editor));
    CHECK(xuiDocumentGetRevision(document) == revision);
    request.iTableId = table; request.iRow = 1; request.iColumn = 0;
    request.iRows = 1; request.iColumns = 3;
    CHECK(xuiDocumentViewSetTableSelection(editor, &request) == XUI_OK);
    key.iModifiers = XUI_MOD_ALT | XUI_MOD_SHIFT; key.iKey = XUI_KEY_LEFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 1, 0, 1, 2);
    select_cell_rectangle(context, editor, cell[2], cell[0]);
    expect_table_rectangle(editor, table, 2, 0, 1, 3);
    key.iKey = XUI_KEY_RIGHT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 2, 1, 1, 2);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    select_table_text(editor, text_ids[1]); key.iKey = XUI_KEY_DOWN;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 1, 1, 2, 1);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    key.iKey = XUI_KEY_ESCAPE; key.iModifiers = 0;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    select_table_text(editor, text_ids[1]);
    key.iKey = XUI_KEY_RIGHT; key.iModifiers = XUI_MOD_SHIFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iNodeId == text_ids[1] &&
        range.tCaret.iNodeId == text_ids[1] &&
        range.tCaret.iOffset != range.tAnchor.iOffset);
    { xui_doc_table_selection_t selected = {0}; selected.iSize = sizeof(selected);
      CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND); }
    select_table_text(editor, text_ids[0]);
    key.iKey = XUI_KEY_RIGHT; key.iModifiers = XUI_MOD_ALT | XUI_MOD_SHIFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 0, 0, 1, 2);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS,
        &command) == XUI_OK && command.bEnabled);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    expect_table_rectangle(editor, table, 0, 0, 1, 2);
    key.iKey = XUI_KEY_LEFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 0, 0, 1, 1);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 3, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnMergeCells(txn, table, 0, 0, 1, 2, &merged) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    text_ids[0] = table_cell_text(snapshot, table, 0, 0);
    xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    select_table_text(editor, text_ids[0]); key.pTarget = editor;
    key.iModifiers = XUI_MOD_ALT | XUI_MOD_SHIFT; key.iKey = XUI_KEY_LEFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 0, 0, 1, 2);
    key.iKey = XUI_KEY_RIGHT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 0, 0, 1, 3);
    key.iKey = XUI_KEY_DOWN;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 0, 0, 2, 3);
    key.iKey = XUI_KEY_LEFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    expect_table_rectangle(editor, table, 0, 0, 2, 2);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    {
        static const char markdown[] = "| A | B |\n| - | - |\n| C | D |\n";
        xui_doc_desc_t md = {0}; md.iSize = sizeof(md);
        md.iProfile = XUI_DOCUMENT_MARKDOWN; md.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&md, &document) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(document, markdown, sizeof(markdown) - 1) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &table) == XUI_OK);
        text_ids[0] = table_cell_text(snapshot, table, 0, 0);
        xuiDocumentSnapshotRelease(snapshot);
        desc.tView.pDocument = document;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        select_table_text(editor, text_ids[0]); key.pTarget = editor;
        key.iModifiers = XUI_MOD_ALT | XUI_MOD_SHIFT; key.iKey = XUI_KEY_RIGHT;
        revision = xuiDocumentGetRevision(document);
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        expect_table_rectangle(editor, table, 0, 0, 1, 2);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_MERGE_CELLS,
            &command) == XUI_OK && !command.bEnabled &&
            command.iDisabledReason == XUI_DOC_ERROR_UNREPRESENTABLE);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        { xui_doc_table_selection_t selected = {0}; selected.iSize = sizeof(selected);
          CHECK(xuiDocumentViewGetTableSelection(editor, &selected) == XUI_ERROR_NOT_FOUND); }
        CHECK(xuiDocumentGetRevision(document) == revision);
        source(document, markdown);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("DocumentEditor keyboard table rectangle: extend/shrink, pointer/API, merged spans, Undo, read-only and Markdown policy passed");
}
static void table_rectangle_cut_callback_case(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xui_document d; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_doc_editor_desc_t desc = {0}; xui_widget editor;
    xui_doc_cell_hit_t first = {0}, corner = {0};
    xui_doc_node_id table, row, first_cell, last_cell, last;
    uint64_t revision;
    xuiTestProxyInit(&proxy);
    ordered_clipboard_set = proxy.tProxy.clipboardSetText;
    proxy.tProxy.clipboardSetText = ordered_clipboard;
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK &&
        xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(d, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 2, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnPasteTableMatrix(txn, table, 0, 0,
        "a\tb\nc\td", 7, &last) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 0, &row) == XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot, row, 0, &first_cell) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 1, &row) == XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot, row, 1, &last_cell) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = d; desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK &&
        xuiInputViewport(context, 320, 240) == XUI_OK);
    first.iSize = corner.iSize = sizeof(first);
    CHECK(xuiDocumentViewGetCellRect(editor, first_cell, &first) == XUI_OK &&
        xuiDocumentViewGetCellRect(editor, last_cell, &corner) == XUI_OK);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_DOWN,
        (int)(first.tBounds.x + first.tBounds.width / 2),
        (int)(first.tBounds.y + first.tBounds.height / 2), XUI_POINTER_BUTTON_LEFT, XUI_MOD_ALT);
    send_pointer_mod(context, editor, XUI_EVENT_POINTER_UP,
        (int)(corner.tBounds.x + corner.tBounds.width / 2),
        (int)(corner.tBounds.y + corner.tBounds.height / 2), 0, XUI_MOD_ALT);
    ordered_callback_editor = editor; ordered_callback_action = 1;
    revision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CUT) == XUI_DOC_ERROR_STALE &&
        !ordered_callback_action && xuiDocumentGetRevision(d) == revision);
    CHECK(xuiDocumentAcquireSnapshot(d, &snapshot) == XUI_OK);
    { char* copied; uint64_t bytes;
      CHECK(xuiDocumentSnapshotCopyTableMatrix(snapshot, table, 0, 0, 2, 2,
          &copied, &bytes) == XUI_OK && !strcmp(copied, "a\tb\nc\td"));
      xuiDocumentFreeBuffer(copied); }
    xuiDocumentSnapshotRelease(snapshot);
    ordered_callback_editor = NULL; ordered_clipboard_set = NULL;
    xuiWidgetDestroy(editor); xuiDocumentRelease(d); xuiDestroy(context);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("DocumentEditor rectangle Cut: clipboard cancellation preserves all cells and revision");
}
static void table_width_editor_cases(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_doc_editor_desc_t desc = {0}; xui_widget editor; xui_widget_cursor_proc cursor;
    xui_doc_range_t before, after; xui_doc_cell_hit_t outer = {0};
    void* cursor_user; uint64_t table, row, last_cell, revision; float width; int outer_edge;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 3, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = document; desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 300, 150}) == XUI_OK &&
        xuiInputViewport(context, 300, 150) == XUI_OK && xuiLayout(context) == XUI_OK);
    CHECK(xuiWidgetGetCursorQueryCallback(editor, &cursor, &cursor_user) == XUI_OK &&
        cursor && cursor(editor, 100, 10, cursor_user) == XUI_CURSOR_RESIZE_EW &&
        cursor(editor, 20, 10, cursor_user) == XUI_CURSOR_INHERIT);
    CHECK(xuiDocumentViewGetSelection(editor, &before) == XUI_OK);
    revision = xuiDocumentGetRevision(document);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 100, 10, XUI_POINTER_BUTTON_LEFT);
    CHECK(xuiGetPointerCaptureEx(context, 7, XUI_POINTER_TYPE_MOUSE) == editor);
    CHECK(xuiDocumentGetRevision(document) == revision);
    send_pointer(context, editor, XUI_EVENT_POINTER_MOVE, 110, 10, XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_MOVE, 120, 10, XUI_POINTER_BUTTON_LEFT);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 120);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetSelection(editor, &after) == XUI_OK &&
        after.tAnchor.iNodeId == before.tAnchor.iNodeId &&
        after.tAnchor.iOffset == before.tAnchor.iOffset &&
        after.tCaret.iNodeId == before.tCaret.iNodeId && after.tCaret.iOffset == before.tCaret.iOffset);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, 120, 10, 0);
    CHECK(xuiGetPointerCaptureEx(context, 7, XUI_POINTER_TYPE_MOUSE) != editor);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 0);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 120);
    xuiDocumentSnapshotRelease(snapshot);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 120, 10, XUI_POINTER_BUTTON_LEFT);
    CHECK(xuiGetPointerCaptureEx(context, 7, XUI_POINTER_TYPE_MOUSE) == editor);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    CHECK(xuiGetPointerCaptureEx(context, 7, XUI_POINTER_TYPE_MOUSE) != editor);
    send_pointer(context, editor, XUI_EVENT_POINTER_MOVE, 140, 10, XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, 140, 10, 0);
    CHECK(cursor(editor, 120, 10, cursor_user) == XUI_CURSOR_INHERIT);
    CHECK(xuiDocumentEditorSetTableColumnWidth(editor, table, 0, 50) == XUI_ERROR_UNSUPPORTED);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    CHECK(xuiDocumentEditorSetTableColumnWidth(editor, table, 0, 50) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 50);
    xuiDocumentSnapshotRelease(snapshot);
    revision = xuiDocumentGetRevision(document);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 20, 10, XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_MOVE, 30, 10, XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, 30, 10, 0);
    CHECK(xuiDocumentGetRevision(document) == revision);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 50, 10, XUI_POINTER_BUTTON_LEFT);
    CHECK(xuiGetPointerCaptureEx(context, 7, XUI_POINTER_TYPE_MOUSE) == editor);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnSetTableColumnWidth(txn, table, 0, 60) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiGetPointerCaptureEx(context, 7, XUI_POINTER_TYPE_MOUSE) != editor);
    send_pointer(context, editor, XUI_EVENT_POINTER_MOVE, 80, 10, XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, 80, 10, 0);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 60);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorSetTableColumnWidth(editor, table, 1, 70) == XUI_OK);
    CHECK(xuiDocumentEditorSetTableColumnWidth(editor, table, 2, 80) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 0, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, row, 2, &last_cell) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    outer.iSize = sizeof(outer);
    CHECK(xuiDocumentViewGetCellRect(editor, last_cell, &outer) == XUI_OK);
    outer_edge = (int)(outer.tBounds.x + outer.tBounds.width + .5);
    CHECK(outer_edge == 210 && cursor(editor, outer_edge, 10, cursor_user) == XUI_CURSOR_RESIZE_EW);
    CHECK(xuiDocumentViewGetSelection(editor, &before) == XUI_OK);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, outer_edge, 10, XUI_POINTER_BUTTON_LEFT);
    CHECK(xuiGetPointerCaptureEx(context, 7, XUI_POINTER_TYPE_MOUSE) == editor);
    send_pointer(context, editor, XUI_EVENT_POINTER_MOVE, outer_edge + 25, 10, XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, outer_edge + 25, 10, 0);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 2, &width) == XUI_OK && width == 105);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetCellRect(editor, last_cell, &outer) == XUI_OK &&
        (int)(outer.tBounds.x + outer.tBounds.width + .5) == outer_edge + 25);
    CHECK(xuiDocumentViewGetSelection(editor, &after) == XUI_OK &&
        after.tAnchor.iNodeId == before.tAnchor.iNodeId &&
        after.tAnchor.iOffset == before.tAnchor.iOffset &&
        after.tCaret.iNodeId == before.tCaret.iNodeId && after.tCaret.iOffset == before.tCaret.iOffset);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 2, &width) == XUI_OK && width == 80);
    xuiDocumentSnapshotRelease(snapshot);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 1, 3, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 0, &row) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, row, 2, &last_cell) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 300, 150}) == XUI_OK &&
        xuiInputViewport(context, 300, 150) == XUI_OK);
    CHECK(xuiDocumentViewGetCellRect(editor, last_cell, &outer) == XUI_OK);
    outer_edge = (int)(outer.tBounds.x + outer.tBounds.width + .5);
    CHECK(outer_edge == 300 && cursor(editor, outer_edge - 1, 10, cursor_user) == XUI_CURSOR_RESIZE_EW);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, outer_edge - 1, 10, XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_MOVE, outer_edge - 21, 10, XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_MOVE, outer_edge - 31, 10, XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, outer_edge - 31, 10, 0);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 100);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 1, &width) == XUI_OK && width == 100);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 2, &width) == XUI_OK && width == 70);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetCellRect(editor, last_cell, &outer) == XUI_OK &&
        (int)(outer.tBounds.x + outer.tBounds.width + .5) == 270);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 0);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 1, &width) == XUI_OK && width == 0);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 2, &width) == XUI_OK && width == 0);
    xuiDocumentSnapshotRelease(snapshot);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    {
        xui_doc_desc_t markdown_desc = {0};
        const char* source_text = "| A | B |\n| - | - |\n| x | y |\n";
        markdown_desc.iSize = sizeof(markdown_desc);
        markdown_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        markdown_desc.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&markdown_desc, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, source_text, strlen(source_text)) == XUI_OK);
        desc.tView.pDocument = document;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 300, 150}) == XUI_OK && xuiLayout(context) == XUI_OK);
        revision = xuiDocumentGetRevision(document);
        CHECK(xuiDocumentEditorSetTableColumnWidth(editor, table, 0, 50) == XUI_ERROR_UNSUPPORTED);
        send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 150, 10, XUI_POINTER_BUTTON_LEFT);
        send_pointer(context, editor, XUI_EVENT_POINTER_MOVE, 170, 10, XUI_POINTER_BUTTON_LEFT);
        send_pointer(context, editor, XUI_EVENT_POINTER_UP, 170, 10, 0);
        CHECK(xuiDocumentGetRevision(document) == revision);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("DocumentEditor table column drag: internal/outer boundaries, cursor, capture, grouped Undo, selection, external replacement, read-only and Markdown policy passed");
}
static void table_column_keyboard_cases(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_document_snapshot snapshot;
    xui_doc_editor_desc_t desc = {0}; xui_widget editor;
    xui_doc_command_state_t state = {0}; xui_doc_cell_hit_t rect = {0};
    xui_doc_range_t before, after; xui_event_t key = {0};
    uint64_t table, first, last, row, last_cell, revision; float width;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 1, 3, 0, &table) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    first = table_cell_text(snapshot, table, 0, 0);
    last = table_cell_text(snapshot, table, 0, 2);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, table, 0, &row) == XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot, row, 2, &last_cell) == XUI_OK);
    xuiDocumentSnapshotRelease(snapshot);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = document; desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 300, 150}) == XUI_OK &&
        xuiInputViewport(context, 300, 150) == XUI_OK && xuiLayout(context) == XUI_OK);
    select_table_text(editor, first);
    CHECK(xuiDocumentViewGetSelection(editor, &before) == XUI_OK);
    state.iSize = sizeof(state);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_COLUMN_WIDER, &state) == XUI_OK && state.bEnabled);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_COLUMN_AUTO, &state) == XUI_OK &&
        !state.bEnabled && state.bActive);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_COLUMN_WIDER) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 108);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetSelection(editor, &after) == XUI_OK &&
        after.tCaret.iNodeId == before.tCaret.iNodeId && after.tCaret.iOffset == before.tCaret.iOffset);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_COLUMN_AUTO, &state) == XUI_OK &&
        state.bEnabled && !state.bActive);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_COLUMN_NARROWER) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 100);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_COLUMN_AUTO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 0);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 100);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    select_table_text(editor, last);
    rect.iSize = sizeof(rect);
    CHECK(xuiDocumentViewGetCellRect(editor, last_cell, &rect) == XUI_OK &&
        (int)(rect.tBounds.x + rect.tBounds.width + .5) == 300);
    key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor;
    key.iModifiers = XUI_MOD_CTRL | XUI_MOD_ALT | XUI_MOD_SHIFT; key.iKey = XUI_KEY_RIGHT;
    revision = xuiDocumentGetRevision(document);
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK && xuiDocumentGetRevision(document) == revision + 1);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 100);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 1, &width) == XUI_OK && width == 100);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 2, &width) == XUI_OK && width == 108);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetCellRect(editor, last_cell, &rect) == XUI_OK &&
        (int)(rect.tBounds.x + rect.tBounds.width + .5) == 308);
    key.iKey = '0'; CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 2, &width) == XUI_OK && width == 0);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 0);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 1, &width) == XUI_OK && width == 0);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 2, &width) == XUI_OK && width == 0);
    xuiDocumentSnapshotRelease(snapshot);
    key.iKey = XUI_KEY_LEFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 2, &width) == XUI_OK && width == 92);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetCellRect(editor, last_cell, &rect) == XUI_OK &&
        (int)(rect.tBounds.x + rect.tBounds.width + .5) == 292);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    select_table_text(editor, first);
    CHECK(xuiDocumentViewGetSelection(editor, &before) == XUI_OK);
    select_table_text(editor, last);
    CHECK(xuiDocumentViewGetSelection(editor, &after) == XUI_OK);
    before.tCaret = after.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &before) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_COLUMN_WIDER, &state) == XUI_OK &&
        !state.bEnabled && state.iDisabledReason == XUI_ERROR_INVALID_STATE);
    select_table_text(editor, first);
    CHECK(xuiDocumentEditorSetTableColumnWidth(editor, table, 0, 24) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_COLUMN_NARROWER, &state) == XUI_OK &&
        !state.bEnabled && state.iDisabledReason == XUI_ERROR_INVALID_STATE);
    CHECK(xuiDocumentEditorSetTableColumnWidth(editor, table, 0, 1000000) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_COLUMN_WIDER, &state) == XUI_OK &&
        !state.bEnabled && state.iDisabledReason == XUI_ERROR_INVALID_STATE);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_COLUMN_WIDER, &state) == XUI_OK &&
        !state.bEnabled && state.iDisabledReason == XUI_ERROR_INVALID_STATE);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_COLUMN_WIDER) == XUI_ERROR_INVALID_STATE);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    {
        uint64_t merged, merged_text;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
        CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
        CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 1, 3, 0, &table) == XUI_OK);
        CHECK(xuiDocumentTxnMergeCells(txn, table, 0, 0, 1, 2, &merged) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        merged_text = table_cell_text(snapshot, table, 0, 0);
        xuiDocumentSnapshotRelease(snapshot);
        desc.tView.pDocument = document;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 300, 150}) == XUI_OK &&
            xuiLayout(context) == XUI_OK);
        select_table_text(editor, merged_text);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_COLUMN_WIDER, &state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_COLUMN_WIDER) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 0, &width) == XUI_OK && width == 0);
        CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 1, &width) == XUI_OK && width == 108);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetTableColumnWidth(snapshot, table, 1, &width) == XUI_OK && width == 0);
        xuiDocumentSnapshotRelease(snapshot);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    {
        const char* markdown = "| A | B |\n| - | - |\n| x | y |\n";
        xui_doc_desc_t md = {0}; xui_doc_node_id cell_text;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        md.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&md, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, markdown, strlen(markdown)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &table) == XUI_OK);
        cell_text = table_cell_text(snapshot, table, 0, 0);
        xuiDocumentSnapshotRelease(snapshot);
        desc.tView.pDocument = document;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 300, 150}) == XUI_OK &&
            xuiLayout(context) == XUI_OK);
        select_table_text(editor, cell_text); revision = xuiDocumentGetRevision(document);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TABLE_COLUMN_WIDER, &state) == XUI_OK &&
            !state.bEnabled && state.iDisabledReason == XUI_DOC_ERROR_UNREPRESENTABLE);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TABLE_COLUMN_WIDER) ==
            XUI_DOC_ERROR_UNREPRESENTABLE && xuiDocumentGetRevision(document) == revision);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("DocumentEditor keyboard column width: commands, limits, auto width, outer edge, shortcuts, selection, Undo, read-only and Markdown policy passed");
}
static xui_clipboard_set_items_proc native_fragment_set_items;
static xui_clipboard_get_data_proc native_fragment_get_data;
static xui_widget native_fragment_callback_editor;
static int native_fragment_callback_mode;
static unsigned native_fragment_callback_calls;
static unsigned native_fragment_read_calls;
static int native_fragment_set_items_callback(xui_proxy proxy,
    const xui_clipboard_item_t* items, int count)
{
    xui_widget editor = native_fragment_callback_editor;
    native_fragment_callback_calls++;
    if (editor && native_fragment_callback_mode == 1)
        CHECK(xuiDocumentEditorInsertText(editor, "blocked", 7) == XUI_DOC_ERROR_BUSY);
    if (editor && native_fragment_callback_mode == 2)
        CHECK(xuiDocumentEditorCancelInput(editor) == XUI_OK);
    return native_fragment_set_items(proxy, items, count);
}
static int native_fragment_get_data_callback(xui_proxy proxy,
    const char* format, void* data, size_t capacity)
{
    int result = native_fragment_get_data(proxy, format, data, capacity);
    if (native_fragment_callback_editor &&
        ((!strcmp(format, XUI_CLIPBOARD_FORMAT_DOCUMENT_FRAGMENT) &&
            native_fragment_callback_mode <= 4) ||
        (!strcmp(format, XUI_CLIPBOARD_FORMAT_HTML) &&
            native_fragment_callback_mode >= 5))) {
        native_fragment_read_calls++;
        if (((native_fragment_callback_mode == 3 || native_fragment_callback_mode == 5) &&
            native_fragment_read_calls == 1) ||
            ((native_fragment_callback_mode == 4 || native_fragment_callback_mode == 6) &&
            native_fragment_read_calls == 2))
            CHECK(xuiDocumentEditorCancelInput(native_fragment_callback_editor) == XUI_OK);
    }
    return result;
}
static void native_fragment_editor_cases(xui_context context,
    xui_test_proxy_state_t* proxy)
{
    xui_doc_editor_desc_t desc = {0};
    xui_doc_node_desc_t node_desc = {0};
    xui_doc_node_info_t info = {0};
    xui_doc_desc_t md = {0};
    xui_document origin, target;
    xui_document_transaction t;
    xui_document_snapshot snapshot;
    xui_widget editor;
    xui_doc_range_t range;
    uint64_t paragraph, text_id, child, found;
    char markdown[256]; uint64_t bytes;
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentCreate(NULL, &origin) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(origin, NULL, &t) == XUI_OK);
    paragraph = 0;
    node_desc.iSize = sizeof(node_desc); node_desc.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(t, 1, 0, &node_desc, &paragraph) == XUI_OK);
    node_desc.iKind = XUI_DOC_TEXT; node_desc.sText = "ab";
    node_desc.iTextBytes = 2; node_desc.tAttributes.iMarks = XUI_DOC_BOLD;
    CHECK(xuiDocumentTxnInsertNode(t, paragraph, 0, &node_desc, &text_id) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    desc.tView.pDocument = origin;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(origin, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "ab", 2,
        NULL, &range, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_COPY) == XUI_OK &&
        proxy->iClipboardDocumentSize > 16 && !strcmp(proxy->sClipboard, "ab"));
    CHECK(proxy->iClipboardHtmlSize > 0 &&
        proxy->iClipboardHtmlSize < sizeof(proxy->arrClipboardHtml));
    proxy->arrClipboardHtml[proxy->iClipboardHtmlSize] = 0;
    CHECK(strstr((const char*)proxy->arrClipboardHtml, "<strong>ab</strong>") &&
        !strstr((const char*)proxy->arrClipboardHtml, "<p>"));
    xuiWidgetDestroy(editor); xuiDocumentRelease(origin);

    CHECK(xuiDocumentCreate(NULL, &target) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(target, NULL, &t) == XUI_OK);
    node_desc.iKind = XUI_DOC_PARAGRAPH; node_desc.sText = NULL;
    node_desc.iTextBytes = 0; node_desc.tAttributes.iMarks = 0;
    CHECK(xuiDocumentTxnInsertNode(t, 1, 0, &node_desc, &paragraph) == XUI_OK);
    node_desc.iKind = XUI_DOC_TEXT; node_desc.sText = "XY";
    node_desc.iTextBytes = 2;
    CHECK(xuiDocumentTxnInsertNode(t, paragraph, 0, &node_desc, &text_id) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    desc.tView.pDocument = target;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "XY", 2,
        NULL, &range, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    range.tAnchor = range.tCaret; range.tAnchor.iOffset = 1;
    range.tCaret = range.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_OK);
    plain(target, "XabY\n");
    CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, paragraph, 1, &child) == XUI_OK);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, child, &info) == XUI_OK &&
        info.tAttributes.iMarks == XUI_DOC_BOLD);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    plain(target, "XY\n");
    {
        static const char html_only[] = "<strong>hi &amp; β</strong>";
        size_t saved_native_bytes = proxy->iClipboardDocumentSize;
        proxy->iClipboardDocumentSize = 0;
        memcpy(proxy->arrClipboardHtml, html_only, sizeof(html_only) - 1);
        proxy->iClipboardHtmlSize = sizeof(html_only) - 1;
        CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "XY", 2,
            NULL, &range, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snapshot);
        range.tAnchor = range.tCaret; range.tAnchor.iOffset = 1;
        range.tCaret = range.tAnchor;
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_OK);
        plain(target, "Xhi & βY\n");
        CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, paragraph, 1, &child) == XUI_OK);
        info.iSize = sizeof(info);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, child, &info) == XUI_OK &&
            info.tAttributes.iMarks == XUI_DOC_BOLD);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        plain(target, "XY\n");
        proxy->iClipboardDocumentSize = saved_native_bytes;
        proxy->iClipboardHtmlSize = 0;
    }
    xuiWidgetDestroy(editor); xuiDocumentRelease(target);

    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    md.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&md, &target) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(target, "xy\n", 3) == XUI_OK);
    desc.tView.pDocument = target;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "xy", 2,
        NULL, &range, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    range.tAnchor = range.tCaret; range.tAnchor.iOffset = 1;
    range.tCaret = range.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(snapshot, markdown, sizeof(markdown), &bytes) == XUI_OK);
    if (!strstr(markdown, "x**ab**y"))
        fprintf(stderr, "Native fragment Markdown paste: [%s]\n", markdown);
    CHECK(strstr(markdown, "x**ab**y"));
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(target, "xy\n");
    {
        static const char html_only[] = "<strong>zz</strong>";
        size_t native_bytes = proxy->iClipboardDocumentSize;
        proxy->iClipboardDocumentSize = 0;
        memcpy(proxy->arrClipboardHtml, html_only, sizeof(html_only) - 1);
        proxy->iClipboardHtmlSize = sizeof(html_only) - 1;
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotCopySource(snapshot, markdown,
            sizeof(markdown), &bytes) == XUI_OK && strstr(markdown, "x**zz**y"));
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(target, "xy\n");
        proxy->iClipboardDocumentSize = native_bytes;
        proxy->iClipboardHtmlSize = 0;
    }
    proxy->arrClipboardDocument[0] = 'x';
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_OK);
    source(target, "xaby\n");
    xuiWidgetDestroy(editor); xuiDocumentRelease(target);

    CHECK(xuiDocumentCreate(NULL, &origin) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(origin, NULL, &t) == XUI_OK);
    node_desc.iKind = XUI_DOC_PARAGRAPH; node_desc.sText = NULL;
    node_desc.iTextBytes = 0; node_desc.tAttributes.iMarks = 0;
    CHECK(xuiDocumentTxnInsertNode(t, 1, UINT64_MAX, &node_desc, &paragraph) == XUI_OK);
    node_desc.iKind = XUI_DOC_TEXT; node_desc.sText = "one"; node_desc.iTextBytes = 3;
    CHECK(xuiDocumentTxnInsertNode(t, paragraph, 0, &node_desc, &text_id) == XUI_OK);
    node_desc.iKind = XUI_DOC_PARAGRAPH; node_desc.sText = NULL; node_desc.iTextBytes = 0;
    CHECK(xuiDocumentTxnInsertNode(t, 1, UINT64_MAX, &node_desc, &paragraph) == XUI_OK);
    node_desc.iKind = XUI_DOC_TEXT; node_desc.sText = "two"; node_desc.iTextBytes = 3;
    CHECK(xuiDocumentTxnInsertNode(t, paragraph, 0, &node_desc, &text_id) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    desc.tView.pDocument = origin;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    memset(&range, 0, sizeof(range));
    range.tAnchor.iSize = range.tCaret.iSize = sizeof(range.tAnchor);
    range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_GAP;
    range.tAnchor.iDocumentId = range.tCaret.iDocumentId = xuiDocumentGetIdentity(origin);
    range.tAnchor.iRevision = range.tCaret.iRevision = xuiDocumentGetRevision(origin);
    range.tAnchor.iNodeId = range.tCaret.iNodeId = 1;
    range.tCaret.iOffset = 2;
    range.tAnchor.iAffinity = range.tCaret.iAffinity = XUI_DOC_AFTER;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_COPY) == XUI_OK &&
        proxy->iClipboardDocumentSize > 16);
    CHECK(proxy->iClipboardHtmlSize > 0 &&
        proxy->iClipboardHtmlSize < sizeof(proxy->arrClipboardHtml));
    proxy->arrClipboardHtml[proxy->iClipboardHtmlSize] = 0;
    CHECK(strstr((const char*)proxy->arrClipboardHtml, "<p>") &&
        strstr((const char*)proxy->arrClipboardHtml, "one") &&
        strstr((const char*)proxy->arrClipboardHtml, "two"));
    xuiWidgetDestroy(editor); xuiDocumentRelease(origin);

    CHECK(xuiDocumentCreate(NULL, &target) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(target, NULL, &t) == XUI_OK);
    node_desc.iKind = XUI_DOC_PARAGRAPH; node_desc.sText = NULL; node_desc.iTextBytes = 0;
    CHECK(xuiDocumentTxnInsertNode(t, 1, 0, &node_desc, &paragraph) == XUI_OK);
    node_desc.iKind = XUI_DOC_TEXT; node_desc.sText = "XY"; node_desc.iTextBytes = 2;
    CHECK(xuiDocumentTxnInsertNode(t, paragraph, 0, &node_desc, &text_id) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
    desc.tView.pDocument = target;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "XY", 2,
        NULL, &range, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    range.tAnchor = range.tCaret; range.tAnchor.iOffset = 1; range.tCaret = range.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_OK);
    plain(target, "X\none\ntwo\nY\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    plain(target, "XY\n");
    xuiWidgetDestroy(editor); xuiDocumentRelease(target);
    puts("DocumentEditor native fragment clipboard: styled Rich/Markdown and multi-block paste, text fallback and one-step Undo passed");
}
static void native_fragment_image_view_cases(xui_context context,
    xui_test_proxy_state_t* proxy)
{
    xui_document origin, target;
    xui_document_transaction transaction;
    xui_document_snapshot snapshot;
    xui_doc_node_desc_t node = {0};
    xui_doc_node_info_t info = {0};
    xui_doc_view_desc_t view_desc = {0};
    xui_doc_editor_desc_t editor_desc = {0};
    xui_doc_range_t range;
    xui_widget view, editor;
    uint64_t paragraph, image, text_id, found;
    const unsigned char pixel[4] = {255, 128, 64, 64};
    void* image_bytes = NULL;
    size_t image_length = 0;
    int image_width, image_height;
    CHECK(xgeImageEncodePNG(1, 1, pixel, 4, &image_bytes, &image_length) == XGE_OK);
    CHECK(xuiDocumentImageResourceLoadMemory(context, "/empty-alt", image_bytes,
        image_length, NULL, NULL) == XUI_OK);
    xrtFree(image_bytes);
    CHECK(xuiDocumentCreate(NULL, &origin) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(origin, NULL, &transaction) == XUI_OK);
    node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(transaction, 1, 0, &node, &paragraph) == XUI_OK);
    node.iKind = XUI_DOC_IMAGE; node.sResource = "/empty-alt";
    CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, 0, &node, &image) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction);
    memset(&range, 0, sizeof(range));
    range.tAnchor.iSize = range.tCaret.iSize = sizeof(range.tAnchor);
    range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_GAP;
    range.tAnchor.iDocumentId = range.tCaret.iDocumentId = xuiDocumentGetIdentity(origin);
    range.tAnchor.iRevision = range.tCaret.iRevision = xuiDocumentGetRevision(origin);
    range.tAnchor.iNodeId = range.tCaret.iNodeId = paragraph;
    range.tCaret.iOffset = 1;
    range.tAnchor.iAffinity = range.tCaret.iAffinity = XUI_DOC_AFTER;
    view_desc.iSize = sizeof(view_desc); view_desc.pDocument = origin;
    CHECK(xuiDocumentViewCreate(context, &view_desc, &view) == XUI_OK);
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentViewSetSelection(view, &range) == XUI_OK);
    CHECK(xuiEditCopy(view) == XUI_OK && proxy->iClipboardDocumentSize > 16 &&
        !proxy->sClipboard[0]);
    CHECK(proxy->iClipboardHtmlSize > 0 &&
        proxy->iClipboardHtmlSize < sizeof(proxy->arrClipboardHtml));
    proxy->arrClipboardHtml[proxy->iClipboardHtmlSize] = 0;
    CHECK(strstr((const char*)proxy->arrClipboardHtml,
        "<img src=\"/empty-alt\" alt=\"\">"));
    CHECK(proxy->iClipboardPngSize > 8 && xgeImageInfoMemory(
        proxy->arrClipboardPng, (int)proxy->iClipboardPngSize,
        &image_width, &image_height) == XGE_OK &&
        image_width == 8 && image_height == 8);
    xuiWidgetDestroy(view);
    editor_desc.iSize = sizeof(editor_desc);
    editor_desc.tView.iSize = sizeof(editor_desc.tView);
    editor_desc.iMode = XUI_DOC_VISUAL; editor_desc.tView.pDocument = origin;
    CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_COPY) == XUI_OK &&
        proxy->iClipboardDocumentSize > 16 && !proxy->sClipboard[0]);
    CHECK(proxy->iClipboardHtmlSize > 0 &&
        proxy->iClipboardHtmlSize < sizeof(proxy->arrClipboardHtml));
    proxy->arrClipboardHtml[proxy->iClipboardHtmlSize] = 0;
    CHECK(strstr((const char*)proxy->arrClipboardHtml,
        "<img src=\"/empty-alt\" alt=\"\">"));
    CHECK(proxy->iClipboardPngSize > 8);
    xuiWidgetDestroy(editor); xuiDocumentRelease(origin);

    CHECK(xuiDocumentCreate(NULL, &target) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(target, NULL, &transaction) == XUI_OK);
    node.iKind = XUI_DOC_PARAGRAPH; node.sResource = NULL;
    CHECK(xuiDocumentTxnInsertNode(transaction, 1, 0, &node, &paragraph) == XUI_OK);
    node.iKind = XUI_DOC_TEXT; node.sText = "XY"; node.iTextBytes = 2;
    CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, 0, &node, &text_id) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction);
    editor_desc.tView.pDocument = target;
    CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "XY", 2,
        NULL, &range, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    range.tAnchor = range.tCaret; range.tAnchor.iOffset = 1; range.tCaret = range.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
    image = editor_image_find(snapshot, 1); CHECK(image);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, image, &info) == XUI_OK &&
        !strcmp(info.sResource, "/empty-alt") && !info.iTextBytes);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
    CHECK(!editor_image_find(snapshot, 1)); xuiDocumentSnapshotRelease(snapshot);
    proxy->iClipboardDocumentSize = 0;
    proxy->iClipboardHtmlSize = 0;
    proxy->sClipboard[0] = 0;
    CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "XY", 2,
        NULL, &range, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    range.tAnchor = range.tCaret; range.tAnchor.iOffset = 1; range.tCaret = range.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
    image = editor_image_find(snapshot, 1); CHECK(image);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, image, &info) == XUI_OK &&
        strncmp(info.sResource, "xui-clipboard-image-", 20) == 0 &&
        xuiResourceFind(context, info.sResource) != NULL);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
    CHECK(!editor_image_find(snapshot, 1)); xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentAcquireSnapshot(target, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "XY", 2,
        NULL, &range, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    proxy->arrClipboardPng[0] = 0;
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_DOC_ERROR_FORMAT);
    plain(target, "XY\n");
    proxy->arrClipboardPng[0] = 137;
    {
        int destroyed = proxy->iSurfaceDestroyCount;
        CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_ERROR_INVALID_STATE);
        CHECK(proxy->iSurfaceDestroyCount == destroyed);
        plain(target, "XY\n");
    }
    xuiWidgetDestroy(editor); xuiDocumentRelease(target);
    puts("Document View/Editor: image fragment, PNG bytes and PNG-only paste passed");
}
static void native_fragment_callback_case(void)
{
    xui_test_proxy_state_t proxy;
    xui_context context;
    xui_font font;
    xui_document document;
    xui_document_transaction transaction;
    xui_document_snapshot snapshot;
    xui_widget editor;
    xui_doc_editor_desc_t desc = {0};
    xui_doc_node_desc_t node = {0};
    xui_doc_range_t range;
    uint64_t paragraph, text_id, found;
    xuiTestProxyInit(&proxy);
    native_fragment_set_items = proxy.tProxy.clipboardSetItems;
    native_fragment_get_data = proxy.tProxy.clipboardGetData;
    proxy.tProxy.clipboardSetItems = native_fragment_set_items_callback;
    proxy.tProxy.clipboardGetData = native_fragment_get_data_callback;
    CHECK(xuiCreate(&context) == XUI_OK &&
        xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(transaction, 1, 0, &node, &paragraph) == XUI_OK);
    node.iKind = XUI_DOC_TEXT; node.sText = "ab"; node.iTextBytes = 2;
    CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, 0, &node, &text_id) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.iMode = XUI_DOC_VISUAL; desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "ab", 2,
        NULL, &range, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    native_fragment_callback_editor = editor;
    native_fragment_callback_mode = 1;
    native_fragment_callback_calls = 0;
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_COPY) == XUI_OK &&
        native_fragment_callback_calls == 1);
    native_fragment_callback_mode = 2;
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CUT) == XUI_DOC_ERROR_STALE &&
        native_fragment_callback_calls == 2);
    plain(document, "ab\n");
    range.tAnchor = range.tCaret; range.tAnchor.iOffset = 1; range.tCaret = range.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    native_fragment_callback_mode = 3; native_fragment_read_calls = 0;
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_DOC_ERROR_STALE &&
        native_fragment_read_calls == 1);
    plain(document, "ab\n");
    native_fragment_callback_mode = 4; native_fragment_read_calls = 0;
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) == XUI_DOC_ERROR_STALE &&
        native_fragment_read_calls == 2);
    plain(document, "ab\n");
    {
        static const char html_only[] = "<strong>z</strong>";
        proxy.iClipboardDocumentSize = 0;
        memcpy(proxy.arrClipboardHtml, html_only, sizeof(html_only) - 1);
        proxy.iClipboardHtmlSize = sizeof(html_only) - 1;
        native_fragment_callback_mode = 5; native_fragment_read_calls = 0;
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) ==
            XUI_DOC_ERROR_STALE && native_fragment_read_calls == 1);
        plain(document, "ab\n");
        native_fragment_callback_mode = 6; native_fragment_read_calls = 0;
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PASTE) ==
            XUI_DOC_ERROR_STALE && native_fragment_read_calls == 2);
        plain(document, "ab\n");
    }
    native_fragment_callback_editor = NULL; native_fragment_callback_mode = 0;
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("DocumentEditor native/HTML fragment clipboard callback: reentry is BUSY; cancelled Cut and both Paste reads leave text intact");
}
static void table_insert_editor_cases(xui_context context)
{
    unsigned profile;
    for (profile = XUI_DOCUMENT_RICH; profile <= XUI_DOCUMENT_MARKDOWN; profile++) {
        xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0};
        xui_document d; xui_document_snapshot snap; xui_widget editor;
        xui_doc_range_t match, selection; xui_doc_node_info_t info = {0};
        uint64_t found, table, row, cell, paragraph, text_id, revision;
        desc.iSize = sizeof(desc); desc.iProfile = profile; desc.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        if (profile == XUI_DOCUMENT_MARKDOWN)
            CHECK(xuiDocumentLoadMarkdown(d, "ab\n", 3) == XUI_OK);
        ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
        CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        if (profile == XUI_DOCUMENT_RICH)
            CHECK(xuiDocumentEditorInsertText(editor, "ab", 2) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "ab", 2,
            NULL, &match, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snap);
        match.tAnchor.iOffset = match.tCaret.iOffset = 1;
        CHECK(xuiDocumentViewSetSelection(editor, &match) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentEditorInsertTable(editor, 0, 2) == XUI_ERROR_INVALID_ARGUMENT);
        CHECK(xuiDocumentEditorInsertTable(editor, 2, 1025) == XUI_ERROR_INVALID_ARGUMENT);
        CHECK(xuiDocumentGetRevision(d) == revision);
        CHECK(xuiDocumentEditorInsertTable(editor, 2, 2) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        info.iSize = sizeof(info);
        CHECK(xuiDocumentSnapshotGetNode(snap, 1, &info) == XUI_OK && info.iChildCount == 3);
        CHECK(xuiDocumentSnapshotGetChild(snap, 1, 1, &table) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snap, table, &info) == XUI_OK &&
            info.iKind == XUI_DOC_TABLE && info.iChildCount == 2);
        CHECK(xuiDocumentSnapshotGetChild(snap, table, 0, &row) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snap, row, &info) == XUI_OK && info.iChildCount == 2);
        CHECK(xuiDocumentSnapshotGetChild(snap, row, 0, &cell) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snap, cell, &info) == XUI_OK);
        CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
        if (profile == XUI_DOCUMENT_RICH) {
            CHECK(info.iChildCount == 1);
            CHECK(xuiDocumentSnapshotGetChild(snap, cell, 0, &paragraph) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetChild(snap, paragraph, 0, &text_id) == XUI_OK);
            CHECK(selection.tCaret.iNodeId == text_id &&
                selection.tAnchor.iNodeId == text_id);
        } else {
            CHECK(info.iChildCount == 0 && selection.tCaret.iNodeId == cell &&
                selection.tCaret.iKind == XUI_DOC_POSITION_GAP);
        }
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorInsertText(editor, "x", 1) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        if (profile == XUI_DOCUMENT_MARKDOWN) source(d, "ab\n");
        else plain(d, "ab\n");
        CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
            selection.tCaret.iNodeId == match.tCaret.iNodeId && selection.tCaret.iOffset == 1);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, 1, 1, &table) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snap, table, &info) == XUI_OK && info.iKind == XUI_DOC_TABLE);
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
        selection.tAnchor.iOffset = selection.tCaret.iOffset = 0;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        CHECK(xuiDocumentEditorInsertTable(editor, 1, 2) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snap, 1, &info) == XUI_OK && info.iChildCount == 2);
        CHECK(xuiDocumentSnapshotGetChild(snap, 1, 0, &table) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snap, table, &info) == XUI_OK && info.iKind == XUI_DOC_TABLE);
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
        selection.tAnchor.iOffset = selection.tCaret.iOffset = 2;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        CHECK(xuiDocumentEditorInsertTable(editor, 1, 2) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snap, 1, &info) == XUI_OK && info.iChildCount == 2);
        CHECK(xuiDocumentSnapshotGetChild(snap, 1, 1, &table) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snap, table, &info) == XUI_OK && info.iKind == XUI_DOC_TABLE);
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        if (profile == XUI_DOCUMENT_MARKDOWN) source(d, "ab\n");
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "ab", 2,
            NULL, &match, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentViewSetSelection(editor, &match) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentEditorInsertTable(editor, 1, 2) == XUI_ERROR_UNSUPPORTED &&
            xuiDocumentGetRevision(d) == revision);
        if (profile == XUI_DOCUMENT_MARKDOWN) {
            CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
            CHECK(xuiDocumentEditorInsertTable(editor, 1, 2) == XUI_ERROR_UNSUPPORTED &&
                xuiDocumentGetRevision(d) == revision);
            CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
        }
        CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentEditorInsertTable(editor, 2, 2) == XUI_ERROR_INVALID_STATE &&
            xuiDocumentGetRevision(d) == revision);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0};
        xui_document d; xui_widget editor; uint64_t revision;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_COMMONMARK;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, "ab\n", 3) == XUI_OK);
        ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
        CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
        revision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentEditorInsertTable(editor, 2, 2) == XUI_DOC_ERROR_UNREPRESENTABLE);
        CHECK(xuiDocumentGetRevision(d) == revision); source(d, "ab\n");
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    for (profile = XUI_DOCUMENT_RICH; profile <= XUI_DOCUMENT_MARKDOWN; profile++) {
        xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0};
        xui_document d; xui_document_snapshot snap; xui_widget editor;
        xui_doc_node_info_t info = {0}; uint64_t table;
        desc.iSize = sizeof(desc); desc.iProfile = profile; desc.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
        CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiDocumentEditorInsertTable(editor, 1, 2) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        info.iSize = sizeof(info);
        CHECK(xuiDocumentSnapshotGetNode(snap, 1, &info) == XUI_OK && info.iChildCount == 1);
        CHECK(xuiDocumentSnapshotGetChild(snap, 1, 0, &table) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snap, table, &info) == XUI_OK && info.iKind == XUI_DOC_TABLE);
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snap, 1, &info) == XUI_OK && info.iChildCount == 0);
        xuiDocumentSnapshotRelease(snap);
        if (profile == XUI_DOCUMENT_MARKDOWN) source(d, "");
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        static const char* originals[] = {
            "| A | B |\r\n| - | - |\r\n|   |   |\r\n",
            "| A | B | C |\r\n| - | - | - |\r\n| x | y |\r\n"
        };
        static const char* updated[] = {
            "| A | B |\r\n| - | - |\r\n|   new\\|cell|   |\r\n",
            "| A | B | C |\r\n| - | - | - |\r\n| x | y | new\\|cell |\r\n"
        };
        unsigned mode;
      for (mode = 0; mode < 2; mode++) {
        const char* original = originals[mode];
        xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0};
        xui_document d; xui_document_snapshot snap; xui_widget editor;
        xui_doc_node_id table, row, cell;
        xui_doc_range_t selection = {0};
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
        ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
        CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, XUI_DOCUMENT_ROOT, 0, &table) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, table, 1, &row) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, row, mode ? 2 : 0, &cell) == XUI_OK);
        xuiDocumentSnapshotRelease(snap);
        selection.tAnchor.iSize = sizeof(selection.tAnchor);
        selection.tAnchor.iDocumentId = xuiDocumentGetIdentity(d);
        selection.tAnchor.iRevision = xuiDocumentGetRevision(d);
        selection.tAnchor.iNodeId = cell;
        selection.tAnchor.iKind = XUI_DOC_POSITION_GAP;
        selection.tAnchor.iAffinity = XUI_DOC_AFTER;
        selection.tCaret = selection.tAnchor;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        CHECK(xuiDocumentEditorInsertText(editor, "new|cell", 8) == XUI_OK);
        source(d, updated[mode]);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(d, original);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(d, updated[mode]);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
      }
    }
    puts("DocumentEditor table insertion: Rich/GFM empty and paragraph-boundary insertion, mid-paragraph split, first-cell input, exact/omitted cell source, Undo/Redo and rejection passed");
}
static void cross_list_editor_cases(xui_context context)
{
    const char* original = "- A\n- B\n\n3. C\n4. D\n5. E\n";
    xui_doc_desc_t desc = {0}; xui_doc_editor_desc_t ed = {0};
    xui_document d; xui_document_snapshot snap; xui_widget editor;
    xui_doc_range_t b, d_range, selection; xui_doc_command_state_t state = {0};
    xui_doc_node_info_t info = {0}; xui_event_t key = {0};
    uint64_t count, first_list; int direction;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&desc, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
    ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiSetFocusWidget(context, editor) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "B", 1,
        NULL, &b, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "D", 1,
        NULL, &d_range, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snap);
    selection.tAnchor = d_range.tCaret; selection.tCaret = b.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    state.iSize = sizeof(state);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INDENT_LIST, &state) == XUI_OK && state.bEnabled);
    key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor; key.iKey = XUI_KEY_TAB;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetChild(snap, 1, 0, &first_list) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snap, first_list, &info) == XUI_OK && info.iChildCount == 1);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentSnapshotComparePositions(snap, &selection.tAnchor,
        &selection.tCaret, &direction) == XUI_OK && direction > 0);
    xuiDocumentSnapshotRelease(snap);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_OUTDENT_LIST, &state) == XUI_OK && state.bEnabled);
    key.iModifiers = XUI_MOD_SHIFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snap, 1, 0, &first_list) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snap, first_list, &info) == XUI_OK && info.iChildCount == 4);
    xuiDocumentSnapshotRelease(snap);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    CHECK(selection.tAnchor.iNodeId == d_range.tAnchor.iNodeId &&
        selection.tCaret.iNodeId == b.tAnchor.iNodeId);
    key.iModifiers = XUI_MOD_SHIFT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snap, 1, &info) == XUI_OK && info.iChildCount == 5);
    xuiDocumentSnapshotRelease(snap);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INDENT_LIST, &state) == XUI_OK && !state.bEnabled);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    puts("DocumentEditor cross-list selection: Tab/Shift+Tab, nested sibling lift, selection direction and Undo passed");
}
static unsigned document_toolbar_selections;
static void document_toolbar_selected(xui_widget toolbar, int index, int value, void* user)
{
    xui_widget editor = (xui_widget)user;
    const xui_toolbar_item_t* item = xuiToolbarGetItem(toolbar, index);
    CHECK(item && item->iValue == value);
    CHECK(xuiDocumentEditorExecuteToolbarItem(editor, toolbar, index) == XUI_OK);
    document_toolbar_selections++;
}
static int document_toolbar_item(xui_widget toolbar, uint32_t command)
{
    int i;
    for (i = 0; i < xuiToolbarGetItemCount(toolbar); i++) {
        const xui_toolbar_item_t* item = xuiToolbarGetItem(toolbar, i);
        if (item && item->iType != XUI_TOOLBAR_ITEM_SEPARATOR &&
            item->iValue == (int)command) return i;
    }
    return -1;
}
static void document_toolbar_cases(xui_context context)
{
    xui_document document = NULL, markdown = NULL;
    xui_document_transaction transaction = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_doc_node_desc_t node = {0};
    xui_doc_editor_desc_t editor_desc = {0};
    xui_toolbar_desc_t toolbar_desc = {0};
    xui_doc_range_t selection = {0};
    xui_doc_node_info_t info = {0};
    xui_event_t pointer = {0};
    xui_rect_t item_rect, world;
    xui_widget root = NULL, editor = NULL, toolbar = NULL;
    xui_language custom_language = NULL;
    uint64_t paragraph, leaf, list, count = 0;
    uint32_t common = 0, mixed = 0;
    int bold, undo, center, separator, bullet, number, task, indent;
    char english[128];
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND,
        &node, &paragraph) == XUI_OK);
    node.iKind = XUI_DOC_TEXT; node.sText = "alpha"; node.iTextBytes = 5;
    CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND,
        &node, &leaf) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction); transaction = NULL;
    CHECK(xuiWidgetCreate(context, &root) == XUI_OK &&
        xuiSetRootWidget(context, root) == XUI_OK &&
        xuiWidgetSetRect(root, (xui_rect_t){0, 0, 640, 300}) == XUI_OK);
    editor_desc.iSize = sizeof(editor_desc);
    editor_desc.tView.iSize = sizeof(editor_desc.tView);
    editor_desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK &&
        xuiWidgetAddChild(root, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 52, 640, 248}) == XUI_OK);
    toolbar_desc.iSize = sizeof(toolbar_desc);
    CHECK(xuiToolbarCreate(context, &toolbar, &toolbar_desc) == XUI_OK &&
        xuiWidgetAddChild(root, toolbar) == XUI_OK &&
        xuiWidgetSetRect(toolbar, (xui_rect_t){0, 0, 640, 44}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "alpha", 5, NULL, &selection, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorSetupToolbar(editor, toolbar, 0) == XUI_OK);
    bold = document_toolbar_item(toolbar, XUI_DOC_EDIT_BOLD);
    undo = document_toolbar_item(toolbar, XUI_DOC_EDIT_UNDO);
    center = document_toolbar_item(toolbar, XUI_DOC_EDIT_ALIGN_CENTER);
    CHECK(bold >= 0 && undo >= 0 && center >= 0 &&
        xuiToolbarIsItemEnabled(toolbar, undo) &&
        xuiToolbarIsItemEnabled(toolbar, bold) &&
        !xuiToolbarGetItemChecked(toolbar, bold));
    separator = bold - 1;
    CHECK(separator >= 0 && xuiToolbarGetItem(toolbar, separator)->iType ==
        XUI_TOOLBAR_ITEM_SEPARATOR &&
        xuiDocumentEditorExecuteToolbarItem(editor, toolbar, separator) ==
        XUI_ERROR_INVALID_ARGUMENT);
    CHECK(xuiToolbarSetSelect(toolbar, document_toolbar_selected, editor) == XUI_OK);
    CHECK(xuiInputViewport(context, 640, 300) == XUI_OK && xuiLayout(context) == XUI_OK);
    item_rect = xuiToolbarGetItemRect(toolbar, bold);
    world = xuiWidgetGetWorldRect(toolbar);
    CHECK(item_rect.fW > 0 && item_rect.fH > 0);
    pointer.iSize = sizeof(pointer); pointer.pTarget = toolbar;
    pointer.iPointerId = 51; pointer.iPointerType = XUI_POINTER_TYPE_MOUSE;
    pointer.iButton = pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
    pointer.fX = world.fX + item_rect.fX + item_rect.fW * 0.5f;
    pointer.fY = world.fY + item_rect.fY + item_rect.fH * 0.5f;
    pointer.iType = XUI_EVENT_POINTER_DOWN;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
    pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButtons = 0;
    CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK &&
        document_toolbar_selections == 1 && xuiToolbarGetItemChecked(toolbar, bold));
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotQueryMarks(snapshot, &selection, &common, &mixed) == XUI_OK &&
        (common & XUI_DOC_BOLD));
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    CHECK(xuiDocumentEditorExecuteToolbarItem(editor, toolbar, undo) == XUI_OK &&
        !xuiToolbarGetItemChecked(toolbar, bold));
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK &&
        xuiDocumentEditorSyncToolbar(editor, toolbar) == XUI_OK &&
        !xuiToolbarIsItemEnabled(toolbar, bold));
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK &&
        xuiDocumentEditorSyncToolbar(editor, toolbar) == XUI_OK &&
        xuiToolbarIsItemEnabled(toolbar, bold));
    CHECK(strlen(xuiToolbarGetItemTooltip(toolbar, bold)) < sizeof(english));
    strcpy(english, xuiToolbarGetItemTooltip(toolbar, bold));
    CHECK(xuiSetLanguage(context, XUI_LANGUAGE_ZH) == XUI_OK &&
        xuiDocumentEditorSyncToolbar(editor, toolbar) == XUI_OK &&
        strcmp(english, xuiToolbarGetItemTooltip(toolbar, bold)) != 0 &&
        !strcmp(xuiToolbarGetItemTooltip(toolbar, bold),
            xuiTranslate(context, XUI_TR_RICH_BOLD)));
    custom_language = xuiCreateLanguage(context, "doc-toolbar-test",
        "Document Toolbar Test", XUI_LANGUAGE_EN);
    CHECK(custom_language &&
        xuiLanguageSetText(context, custom_language, XUI_TR_RICH_BOLD, "First") == XUI_OK &&
        xuiSetLanguage(context, xuiGetLanguageId(custom_language)) == XUI_OK &&
        xuiDocumentEditorSyncToolbar(editor, toolbar) == XUI_OK &&
        !strcmp(xuiToolbarGetItemTooltip(toolbar, bold), "First"));
    CHECK(xuiLanguageSetText(context, custom_language, XUI_TR_RICH_BOLD, "Second") == XUI_OK &&
        !strcmp(xuiToolbarGetItemTooltip(toolbar, bold), "First") &&
        xuiDocumentEditorSyncToolbar(editor, toolbar) == XUI_OK &&
        !strcmp(xuiToolbarGetItemTooltip(toolbar, bold), "Second"));
    CHECK(xuiSetLanguage(context, XUI_LANGUAGE_EN) == XUI_OK);
    CHECK(xuiDocumentEditorSetupToolbar(editor, toolbar, XUI_DOC_TOOLBAR_LISTS) == XUI_OK);
    bullet = document_toolbar_item(toolbar, XUI_DOC_EDIT_BULLET_LIST);
    number = document_toolbar_item(toolbar, XUI_DOC_EDIT_NUMBER_LIST);
    task = document_toolbar_item(toolbar, XUI_DOC_EDIT_TASK_LIST);
    indent = document_toolbar_item(toolbar, XUI_DOC_EDIT_INDENT_LIST);
    CHECK(bullet >= 0 && number >= 0 && task >= 0 && indent >= 0 &&
        xuiToolbarIsItemEnabled(toolbar, bullet) &&
        xuiToolbarIsItemEnabled(toolbar, number) &&
        xuiToolbarIsItemEnabled(toolbar, task) &&
        !xuiToolbarIsItemEnabled(toolbar, indent));
    CHECK(xuiDocumentEditorExecuteToolbarItem(editor, toolbar, bullet) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &list) == XUI_OK &&
        xuiDocumentSnapshotGetNode(snapshot, list, &info) == XUI_OK &&
        info.iKind == XUI_DOC_LIST);
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    CHECK(xuiToolbarGetItemChecked(toolbar, bullet));
    CHECK(xuiDocumentEditorExecuteToolbarItem(editor, toolbar, bullet) == XUI_OK);
    CHECK(!xuiToolbarGetItemChecked(toolbar, bullet));
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &list) == XUI_OK &&
        xuiDocumentSnapshotGetNode(snapshot, list, &info) == XUI_OK &&
        info.iKind == XUI_DOC_PARAGRAPH);
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    xuiWidgetDestroy(editor); editor = NULL;
    CHECK(xuiDocumentCreate(&(xui_doc_desc_t){.iSize=sizeof(xui_doc_desc_t),
        .iProfile=XUI_DOCUMENT_MARKDOWN}, &markdown) == XUI_OK &&
        xuiDocumentLoadMarkdown(markdown, "alpha\n", 6) == XUI_OK);
    editor_desc.tView.pDocument = markdown;
    CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK &&
        xuiWidgetAddChild(root, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 52, 640, 248}) == XUI_OK);
    CHECK(xuiDocumentEditorSetupToolbar(editor, toolbar,
        XUI_DOC_TOOLBAR_INLINE_FORMAT | XUI_DOC_TOOLBAR_ALIGNMENT) == XUI_OK);
    bold = document_toolbar_item(toolbar, XUI_DOC_EDIT_BOLD);
    center = document_toolbar_item(toolbar, XUI_DOC_EDIT_ALIGN_CENTER);
    CHECK(bold >= 0 && center >= 0 &&
        xuiToolbarIsItemEnabled(toolbar, bold) &&
        !xuiToolbarIsItemEnabled(toolbar, center));
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
        xuiDocumentEditorSyncToolbar(editor, toolbar) == XUI_OK &&
        !xuiToolbarIsItemEnabled(toolbar, bold));
    CHECK(xuiToolbarSetSelect(toolbar, NULL, NULL) == XUI_OK);
    xuiWidgetDestroy(root);
    xuiDocumentRelease(markdown); xuiDocumentRelease(document);
    puts("DocumentEditor Toolbar: pointer command, state, read-only, locale and Markdown mode passed");
}
static void document_structural_select(xui_widget editor, xui_document document,
    int whole_text)
{
    xui_document_snapshot snapshot; xui_doc_range_t range; uint64_t count = 0;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "alpha", 5, NULL, &range, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    if (!whole_text) range.tAnchor = range.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
}
static int document_structural_root_has(xui_document document, uint32_t kind)
{
    xui_document_snapshot snapshot; xui_doc_node_info_t info = {0};
    xui_doc_node_id child; uint64_t i, count; int found = 0;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, 1, &info) == XUI_OK);
    count = info.iChildCount;
    for (i = 0; i < count; i++) {
        CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, i, &child) == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, child, &info) == XUI_OK);
        if (info.iKind == kind) found = 1;
    }
    xuiDocumentSnapshotRelease(snapshot);
    return found;
}
static void document_structural_toolbar_cases(xui_context context)
{
    uint32_t profiles[] = {XUI_DOCUMENT_RICH, XUI_DOCUMENT_MARKDOWN};
    size_t p;
    for (p = 0; p < sizeof(profiles) / sizeof(profiles[0]); p++) {
        xui_document document; xui_doc_desc_t doc_desc = {0};
        xui_doc_editor_desc_t editor_desc = {0}; xui_toolbar_desc_t toolbar_desc = {0};
        xui_doc_command_state_t state = {0}; xui_doc_range_t selection;
        xui_widget editor, toolbar; uint64_t revision;
        int quote, rule, code;
        doc_desc.iSize = sizeof(doc_desc); doc_desc.iProfile = profiles[p];
        doc_desc.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&doc_desc, &document) == XUI_OK);
        if (profiles[p] == XUI_DOCUMENT_MARKDOWN)
            CHECK(xuiDocumentLoadMarkdown(document, "alpha\n", 6) == XUI_OK);
        editor_desc.iSize = sizeof(editor_desc);
        editor_desc.tView.iSize = sizeof(editor_desc.tView);
        editor_desc.tView.pDocument = document; editor_desc.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 240}) == XUI_OK);
        if (profiles[p] == XUI_DOCUMENT_RICH)
            CHECK(xuiDocumentEditorInsertText(editor, "alpha", 5) == XUI_OK);
        toolbar_desc.iSize = sizeof(toolbar_desc);
        CHECK(xuiToolbarCreate(context, &toolbar, &toolbar_desc) == XUI_OK &&
            xuiDocumentEditorSetupToolbar(editor, toolbar, XUI_DOC_TOOLBAR_BLOCKS) == XUI_OK);
        quote = document_toolbar_item(toolbar, XUI_DOC_EDIT_BLOCK_QUOTE);
        rule = document_toolbar_item(toolbar, XUI_DOC_EDIT_INSERT_RULE);
        code = document_toolbar_item(toolbar, XUI_DOC_EDIT_INSERT_CODE_BLOCK);
        CHECK(quote >= 0 && rule >= 0 && code >= 0 &&
            xuiToolbarGetItem(toolbar, quote)->iType == XUI_TOOLBAR_ITEM_TOGGLE &&
            xuiToolbarGetItem(toolbar, rule)->iType == XUI_TOOLBAR_ITEM_BUTTON &&
            xuiToolbarGetItem(toolbar, code)->iType == XUI_TOOLBAR_ITEM_BUTTON);
        CHECK(!strcmp(xuiToolbarGetItemTooltip(toolbar, rule),
            xuiTranslate(context, XUI_TR_RICH_HORIZONTAL_RULE)) &&
            !strcmp(xuiToolbarGetItemTooltip(toolbar, code),
            xuiTranslate(context, XUI_TR_RICH_CODE_BLOCK)));
        document_structural_select(editor, document, 0);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK &&
            state.bEnabled && !state.bActive);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INSERT_RULE, &state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INSERT_CODE_BLOCK, &state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorExecuteToolbarItem(editor, toolbar, quote) == XUI_OK &&
            document_structural_root_has(document, XUI_DOC_QUOTE));
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK &&
            state.bEnabled && state.bActive && xuiToolbarGetItemChecked(toolbar, quote));
        CHECK(xuiDocumentEditorExecuteToolbarItem(editor, toolbar, quote) == XUI_OK &&
            !document_structural_root_has(document, XUI_DOC_QUOTE));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK &&
            document_structural_root_has(document, XUI_DOC_QUOTE));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK &&
            !document_structural_root_has(document, XUI_DOC_QUOTE));
        document_structural_select(editor, document, 1);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INSERT_RULE, &state) == XUI_OK && !state.bEnabled);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INSERT_CODE_BLOCK, &state) == XUI_OK && !state.bEnabled);
        document_structural_select(editor, document, 0);
        CHECK(xuiDocumentEditorExecuteToolbarItem(editor, toolbar, rule) == XUI_OK &&
            document_structural_root_has(document, XUI_DOC_RULE));
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK &&
            !document_structural_root_has(document, XUI_DOC_RULE));
        document_structural_select(editor, document, 0);
        CHECK(xuiDocumentEditorExecuteToolbarItem(editor, toolbar, code) == XUI_OK &&
            document_structural_root_has(document, XUI_DOC_CODE_BLOCK));
        CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
            selection.tCaret.iKind == XUI_DOC_POSITION_TEXT);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK &&
            !document_structural_root_has(document, XUI_DOC_CODE_BLOCK));
        if (profiles[p] == XUI_DOCUMENT_MARKDOWN) source(document, "alpha\n");
        document_structural_select(editor, document, 0);
        CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
        revision = xuiDocumentGetRevision(document);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && !state.bEnabled);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INSERT_RULE, &state) == XUI_OK && !state.bEnabled);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INSERT_CODE_BLOCK, &state) == XUI_OK && !state.bEnabled);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) != XUI_OK &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_INSERT_RULE) != XUI_OK &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_INSERT_CODE_BLOCK) != XUI_OK &&
            xuiDocumentGetRevision(document) == revision);
        CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
        if (profiles[p] == XUI_DOCUMENT_MARKDOWN) {
            CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
            CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && !state.bEnabled);
            CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INSERT_RULE, &state) == XUI_OK && !state.bEnabled);
            CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INSERT_CODE_BLOCK, &state) == XUI_OK && !state.bEnabled);
            CHECK(xuiDocumentGetRevision(document) == revision);
            CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
            CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK && !state.bEnabled);
            CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INSERT_RULE, &state) == XUI_OK && !state.bEnabled);
            CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INSERT_CODE_BLOCK, &state) == XUI_OK && !state.bEnabled);
        }
        xuiWidgetDestroy(editor); xuiWidgetDestroy(toolbar); xuiDocumentRelease(document);
    }
    {
        const char* original = "alpha\n\n---\n\n- beta\n";
        xui_doc_desc_t doc_desc = {0}; xui_doc_editor_desc_t editor_desc = {0};
        xui_doc_command_state_t state = {0}; xui_doc_range_t selection;
        xui_document document; xui_document_snapshot snapshot;
        xui_doc_node_info_t info = {0}; xui_widget editor;
        uint64_t quote;
        doc_desc.iSize = sizeof(doc_desc); doc_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        doc_desc.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&doc_desc, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, original, strlen(original)) == XUI_OK);
        editor_desc.iSize = sizeof(editor_desc);
        editor_desc.tView.iSize = sizeof(editor_desc.tView);
        editor_desc.tView.pDocument = document;
        editor_desc.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 240}) == XUI_OK);
        CHECK(xuiDocumentViewFind(editor, "alpha", 5, 0, 0, &selection) == XUI_OK);
        selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_GAP;
        selection.tAnchor.iNodeId = selection.tCaret.iNodeId = 1;
        selection.tAnchor.iOffset = 0; selection.tCaret.iOffset = 3;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
            &state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, 1, 0, &quote) == XUI_OK);
        info.iSize = sizeof(info);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, quote, &info) == XUI_OK &&
            info.iKind == XUI_DOC_QUOTE && info.iChildCount == 3);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(document, original);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    {
        const char* original = "---\na: b\n---\n\nalpha\n";
        const char* footnote = "alpha[^n]\n\n[^n]: body\n";
        xui_doc_desc_t doc_desc = {0}; xui_doc_editor_desc_t editor_desc = {0};
        xui_doc_command_state_t state = {0}; xui_doc_range_t selection;
        xui_document document; xui_document_snapshot snapshot; xui_widget editor;
        uint64_t count;
        doc_desc.iSize = sizeof(doc_desc); doc_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        doc_desc.iMarkdownDialect = XUI_MD_EXTENDED;
        CHECK(xuiDocumentCreate(&doc_desc, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, original, strlen(original)) == XUI_OK);
        editor_desc.iSize = sizeof(editor_desc);
        editor_desc.tView.iSize = sizeof(editor_desc.tView);
        editor_desc.tView.pDocument = document;
        editor_desc.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 240}) == XUI_OK);
        CHECK(xuiDocumentViewFind(editor, "alpha", 5, 0, 0, &selection) == XUI_OK);
        selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_GAP;
        selection.tAnchor.iNodeId = selection.tCaret.iNodeId = 1;
        selection.tAnchor.iOffset = 0; selection.tCaret.iOffset = 1;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
            &state) == XUI_OK && !state.bEnabled &&
            state.iDisabledReason == XUI_DOC_ERROR_UNREPRESENTABLE);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) ==
            XUI_DOC_ERROR_UNREPRESENTABLE);
        source(document, original);
        CHECK(xuiDocumentLoadMarkdown(document, footnote, strlen(footnote)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                "body", 4, NULL, &selection, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_GAP;
        selection.tAnchor.iNodeId = selection.tCaret.iNodeId = 1;
        selection.tAnchor.iOffset = 1; selection.tCaret.iOffset = 2;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
            &state) == XUI_OK && !state.bEnabled &&
            state.iDisabledReason == XUI_DOC_ERROR_UNREPRESENTABLE);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) ==
            XUI_DOC_ERROR_UNREPRESENTABLE);
        source(document, footnote);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    {
        const char* original = "pre &amp; keep\n\n> inner\n\n- beta\n\npost &amp; tail\n";
        const char* partial = "> keep &amp; raw\n>\n> inner\n\noutside\n";
        const char* partial_list = "- item one &amp; raw\n\n- item two\n\noutside\n";
        const char* middle_list = "3. keep &amp; left\n\n4. take\n\n5. tail &amp; right\n";
        const char* multi_list = "- keep1 &amp; raw\n- keep2\n\n- take\n\noutside\n";
        const char* nested_list =
            "> - keep1 &amp; raw\n> - keep2\n>\n> - take\n>\n> outside\n>\n> tail &amp; raw\n";
        const char* nested_end =
            "> pre &amp; raw\n>\n> before\n>\n> - take\n>\n> - tail &amp; raw\n";
        const char* nested_middle =
            "> - keep1 &amp; raw\n> - keep2\n>\n> - take1\n> - take2\n>\n> - tail1\n> - tail2 &amp; raw\n";
        const char* list_item_quote =
            "- parent\n  > - keep &amp; raw\n  > - take\n  >\n  > outside\n  >\n  > tail &amp; raw\n- sibling &amp; raw\n";
        const char* list_item_quote_expected =
            "- parent\n  > - keep &amp; raw\n  > > - take\n  > >\n  > > outside\n  >\n  > tail &amp; raw\n- sibling &amp; raw\n";
        const char* deep_list_item_quote =
            "- great &amp; raw\n  - grand\n    - parent\n      > - keep &amp; raw\n      > - take\n      >\n      > outside\n    - sibling &amp; raw\n  - uncle &amp; raw\n";
        const char* deep_list_item_quote_expected =
            "- great &amp; raw\n  - grand\n    - parent\n      > - keep &amp; raw\n      > > - take\n      > >\n      > > outside\n    - sibling &amp; raw\n  - uncle &amp; raw\n";
        const char* reference_list_item_quote =
            "- parent [r]\n  > - keep &amp; raw\n  > - take\n  >\n  > [r]: /raw \"T\"\n  >\n  > outside\n";
        const char* reference_list_item_quote_expected =
            "- parent [r]\n  > - keep &amp; raw\n  > > - take\n  > >\n  > > [r]: /raw \"T\"\n  > >\n  > > outside\n";
        const struct { const char* input; const char* expected; } quote_ancestor_cases[] = {
            {
                "intro &amp; raw\n\n> - parent\n>   > - keep &amp; raw\n>   > - take\n>   >\n>   > outside\n>\n> tail &amp; raw\n\nafter &amp; raw\n",
                "intro &amp; raw\n\n> - parent\n>   > - keep &amp; raw\n>   > > - take\n>   > >\n>   > > outside\n>\n> tail &amp; raw\n\nafter &amp; raw\n"
            },
            {
                "> - parent [r]\n>   > - keep &amp; raw\n>   > - take\n>   >\n>   > [r]: /raw \"T\"\n>   >\n>   > outside\n>\n> tail &amp; raw\n",
                "> - parent [r]\n>   > - keep &amp; raw\n>   > > - take\n>   > >\n>   > > [r]: /raw \"T\"\n>   > >\n>   > > outside\n>\n> tail &amp; raw\n"
            }
        };
        const char* partial_item = "- item one\n\n  second block\n\n- item two\n\noutside\n";
        xui_doc_desc_t doc_desc = {0}; xui_doc_editor_desc_t editor_desc = {0};
        xui_doc_command_state_t state = {0}; xui_doc_range_t left, right, selection;
        xui_doc_node_info_t info = {0};
        xui_document document; xui_document_snapshot snapshot; xui_widget editor;
        uint64_t count, outer, bytes; char output[512];
        size_t variant;
        doc_desc.iSize = sizeof(doc_desc); doc_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        doc_desc.iMarkdownDialect = XUI_MD_EXTENDED;
        CHECK(xuiDocumentCreate(&doc_desc, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, original, strlen(original)) == XUI_OK);
        editor_desc.iSize = sizeof(editor_desc);
        editor_desc.tView.iSize = sizeof(editor_desc.tView);
        editor_desc.tView.pDocument = document;
        editor_desc.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 240}) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "inner", 5, NULL, &left, 1, &count) == XUI_OK && count == 1);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "beta", 4, NULL, &right, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
            &state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, 1, 1, &outer) == XUI_OK);
        info.iSize = sizeof(info);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, outer, &info) == XUI_OK &&
            info.iKind == XUI_DOC_QUOTE && info.iChildCount == 2);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(document, original);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(document, partial, strlen(partial)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "inner", 5, NULL, &left, 1, &count) == XUI_OK && count == 1);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "outside", 7, NULL, &right, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
            &state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotCopySource(snapshot, output, sizeof(output), &bytes) == XUI_OK &&
            bytes < sizeof(output) && strstr(output, "> keep &amp; raw\n") == output);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(document, partial);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(document, output);
        CHECK(xuiDocumentLoadMarkdown(document, partial_list, strlen(partial_list)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "item two", 8, NULL, &left, 1, &count) == XUI_OK && count == 1);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "outside", 7, NULL, &right, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
            &state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotCopySource(snapshot, output, sizeof(output), &bytes) == XUI_OK &&
            bytes < sizeof(output) &&
            strstr(output, "- item one &amp; raw\n\n") == output);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(document, partial_list);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(document, output);
        CHECK(xuiDocumentLoadMarkdown(document, middle_list, strlen(middle_list)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, 1, 0, &outer) == XUI_OK &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                "take", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection = left;
        selection.tAnchor.iKind = XUI_DOC_POSITION_GAP;
        selection.tAnchor.iNodeId = outer; selection.tAnchor.iOffset = 1;
        selection.tCaret = selection.tAnchor; selection.tCaret.iOffset = 2;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
                &state) == XUI_OK && state.bEnabled &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotCopySource(snapshot, output, sizeof(output), &bytes) == XUI_OK &&
            bytes < sizeof(output) &&
            strstr(output, "3. keep &amp; left\n") == output &&
            strstr(output, "5. tail &amp; right\n"));
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(document, middle_list);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(document, output);
        CHECK(xuiDocumentLoadMarkdown(document, multi_list, strlen(multi_list)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "take", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "outside", 7, NULL, &right, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
                &state) == XUI_OK && state.bEnabled &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, 1, 0, &outer) == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, outer, &info) == XUI_OK &&
            info.iKind == XUI_DOC_LIST && info.iChildCount == 2 &&
            (info.tAttributes.iFlags & XUI_DOC_TIGHT) &&
            xuiDocumentSnapshotCopySource(snapshot, output, sizeof(output), &bytes) == XUI_OK &&
            bytes < sizeof(output) &&
            strstr(output, "- keep1 &amp; raw\n- keep2\n") == output);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(document, multi_list);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(document, output);
        CHECK(xuiDocumentLoadMarkdown(document, nested_list, strlen(nested_list)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "take", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "outside", 7, NULL, &right, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
                &state) == XUI_OK && state.bEnabled &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, 1, 0, &outer) == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, outer, &info) == XUI_OK &&
            info.iKind == XUI_DOC_QUOTE && info.iChildCount == 3 &&
            xuiDocumentSnapshotCopySource(snapshot, output, sizeof(output), &bytes) == XUI_OK &&
            bytes < sizeof(output) &&
            strstr(output, "> - keep1 &amp; raw\n> - keep2\n") == output &&
            strstr(output, "> tail &amp; raw\n"));
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(document, nested_list);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(document, output);
        CHECK(xuiDocumentLoadMarkdown(document, nested_end, strlen(nested_end)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "before", 6, NULL, &left, 1, &count) == XUI_OK && count == 1);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "take", 4, NULL, &right, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
                &state) == XUI_OK && state.bEnabled &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotCopySource(snapshot, output, sizeof(output), &bytes) == XUI_OK &&
            bytes < sizeof(output) &&
            strstr(output, "> pre &amp; raw\n") == output &&
            strstr(output, "> - tail &amp; raw\n"));
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(document, nested_end);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(document, output);
        CHECK(xuiDocumentLoadMarkdown(document, nested_middle,
            strlen(nested_middle)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, 1, 0, &outer) == XUI_OK &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                "take1", 5, NULL, &left, 1, &count) == XUI_OK && count == 1);
        { uint64_t list_id;
          CHECK(xuiDocumentSnapshotGetChild(snapshot, outer, 0, &list_id) == XUI_OK);
          selection = left;
          selection.tAnchor.iKind = XUI_DOC_POSITION_GAP;
          selection.tAnchor.iNodeId = list_id;
          selection.tAnchor.iOffset = 2;
          selection.tCaret = selection.tAnchor;
          selection.tCaret.iOffset = 4; }
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
                &state) == XUI_OK && state.bEnabled &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotCopySource(snapshot, output, sizeof(output), &bytes) == XUI_OK &&
            bytes < sizeof(output) &&
            strstr(output, "> - keep1 &amp; raw\n> - keep2\n") == output &&
            strstr(output, "> - tail1\n> - tail2 &amp; raw\n"));
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(document, nested_middle);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(document, output);
        CHECK(xuiDocumentLoadMarkdown(document, list_item_quote,
            strlen(list_item_quote)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                "take", 4, NULL, &left, 1, &count) == XUI_OK && count == 1 &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                "outside", 7, NULL, &right, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
                &state) == XUI_OK && state.bEnabled &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
        source(document, list_item_quote_expected);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(document, list_item_quote);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(document, list_item_quote_expected);
        CHECK(xuiDocumentLoadMarkdown(document, deep_list_item_quote,
            strlen(deep_list_item_quote)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                "take", 4, NULL, &left, 1, &count) == XUI_OK && count == 1 &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                "outside", 7, NULL, &right, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
                &state) == XUI_OK && state.bEnabled &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
        source(document, deep_list_item_quote_expected);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(document, deep_list_item_quote);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(document, deep_list_item_quote_expected);
        CHECK(xuiDocumentLoadMarkdown(document, reference_list_item_quote,
            strlen(reference_list_item_quote)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                "take", 4, NULL, &left, 1, &count) == XUI_OK && count == 1 &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                "outside", 7, NULL, &right, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
            xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
                &state) == XUI_OK && state.bEnabled &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
        source(document, reference_list_item_quote_expected);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(document, reference_list_item_quote);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(document, reference_list_item_quote_expected);
        for (variant = 0; variant < sizeof(quote_ancestor_cases) /
            sizeof(quote_ancestor_cases[0]); variant++) {
            CHECK(xuiDocumentLoadMarkdown(document, quote_ancestor_cases[variant].input,
                strlen(quote_ancestor_cases[variant].input)) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                    "take", 4, NULL, &left, 1, &count) == XUI_OK && count == 1 &&
                xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                    "outside", 7, NULL, &right, 1, &count) == XUI_OK && count == 1);
            xuiDocumentSnapshotRelease(snapshot);
            selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
            CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
                xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
                    &state) == XUI_OK && state.bEnabled &&
                xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
            source(document, quote_ancestor_cases[variant].expected);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
            source(document, quote_ancestor_cases[variant].input);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
            source(document, quote_ancestor_cases[variant].expected);
        }
        {
            static const struct { const char *input, *expected; } body[] = {
                {"- first &amp; raw\n\n  second\n\n  third\n\n  tail &amp; raw\n",
                 "- first &amp; raw\n\n  > second\n  >\n  > third\n\n  tail &amp; raw\n"},
                {"- first [r] &amp; raw\n\n  second\n\n  [r]: /raw \"T\"\n\n  third\n\n  tail &amp; raw\n",
                 "- first [r] &amp; raw\n\n  > second\n  >\n  > [r]: /raw \"T\"\n  >\n  > third\n\n  tail &amp; raw\n"}
            };
            size_t body_case;
            for (body_case = 0; body_case < sizeof(body) / sizeof(body[0]);
                body_case++) {
                const char* body_input = body[body_case].input;
                const char* body_expected = body[body_case].expected;
                CHECK(xuiDocumentLoadMarkdown(document, body_input,
                    strlen(body_input)) == XUI_OK);
                CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                    xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                        "second", 6, NULL, &left, 1, &count) == XUI_OK &&
                    count == 1 &&
                    xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                        "third", 5, NULL, &right, 1, &count) == XUI_OK &&
                    count == 1);
                xuiDocumentSnapshotRelease(snapshot);
                selection.tAnchor = left.tAnchor;
                selection.tCaret = right.tCaret;
                CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
                    xuiDocumentEditorQueryCommand(editor,
                        XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK &&
                    state.bEnabled &&
                    xuiDocumentEditorExecute(editor,
                        XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
                source(document, body_expected);
                CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) ==
                    XUI_OK);
                source(document, body_input);
                CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) ==
                    XUI_OK);
                source(document, body_expected);
            }
        }
        {
            const char* lazy_input =
                "- first &amp; raw\n\n  second\nlazy continuation\n\n  tail &amp; raw\n";
            const char* lazy_expected =
                "- first &amp; raw\n\n  > second\n  > lazy continuation\n\n  tail &amp; raw\n";
            CHECK(xuiDocumentLoadMarkdown(document, lazy_input,
                strlen(lazy_input)) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                    "second", 6, NULL, &left, 1, &count) == XUI_OK &&
                count == 1);
            xuiDocumentSnapshotRelease(snapshot);
            CHECK(xuiDocumentViewSetSelection(editor, &left) == XUI_OK &&
                xuiDocumentEditorQueryCommand(editor,
                    XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK &&
                state.bEnabled &&
                xuiDocumentEditorExecute(editor,
                    XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
            source(document, lazy_expected);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) ==
                XUI_OK);
            source(document, lazy_input);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) ==
                XUI_OK);
            source(document, lazy_expected);
        }
        {
            const char* task_first =
                "- [x] first &amp; raw\nlazy continuation\n\n  tail &amp; raw\n";
            uint64_t revision;
            CHECK(xuiDocumentLoadMarkdown(document, task_first,
                strlen(task_first)) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
                xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                    "first", 5, NULL, &left, 1, &count) == XUI_OK &&
                count == 1);
            xuiDocumentSnapshotRelease(snapshot);
            CHECK(xuiDocumentViewSetSelection(editor, &left) == XUI_OK);
            revision = xuiDocumentGetRevision(document);
            CHECK(xuiDocumentEditorQueryCommand(editor,
                XUI_DOC_EDIT_BLOCK_QUOTE, &state) == XUI_OK &&
                !state.bEnabled &&
                state.iDisabledReason == XUI_DOC_ERROR_UNREPRESENTABLE &&
                xuiDocumentEditorExecute(editor,
                    XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_DOC_ERROR_UNREPRESENTABLE &&
                xuiDocumentGetRevision(document) == revision);
            source(document, task_first);
        }
        CHECK(xuiDocumentLoadMarkdown(document, partial_item, strlen(partial_item)) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "second block", 12, NULL, &left, 1, &count) == XUI_OK && count == 1);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "outside", 7, NULL, &right, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BLOCK_QUOTE,
            &state) == XUI_OK && state.bEnabled &&
            xuiDocumentEditorCanExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BLOCK_QUOTE) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, 1, &info) == XUI_OK && info.iChildCount == 2 &&
            xuiDocumentSnapshotGetChild(snapshot, 1, 0, &outer) == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, outer, &info) == XUI_OK &&
            info.iKind == XUI_DOC_LIST && info.iChildCount == 1 &&
            (info.tAttributes.iFlags & XUI_DOC_TIGHT) &&
            xuiDocumentSnapshotGetChild(snapshot, 1, 1, &outer) == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, outer, &info) == XUI_OK &&
            info.iKind == XUI_DOC_QUOTE && info.iChildCount == 2 &&
            xuiDocumentSnapshotCopySource(snapshot, output, sizeof(output), &bytes) == XUI_OK &&
            bytes < sizeof(output) && strstr(output, "- item one\n\n") == output &&
            strstr(output, "> - second block\n") && strstr(output, "> - item two\n") &&
            strstr(output, "> outside\n"));
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(document, partial_item);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(document, output);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("DocumentEditor structural toolbar: Rich/Markdown quote toggle, rule/code, Undo, disabled modes passed");
}
static void document_task_marker_editor_cases(xui_context context)
{
    const char* original = "- [ ] first &amp; raw\n- [x] tail\n";
    const char* checked = "- [x] first &amp; raw\n- [x] tail\n";
    xui_doc_desc_t md = {0}; xui_doc_editor_desc_t ed = {0};
    xui_document document; xui_document_snapshot snapshot;
    xui_doc_range_t first; xui_doc_command_state_t state = {0};
    xui_widget editor; xui_doc_node_id list, item; uint64_t found, revision;
    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    md.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&md, &document) == XUI_OK &&
        xuiDocumentLoadMarkdown(document, original, strlen(original)) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
    ed.tView.pDocument = document; ed.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 360, 180}) == XUI_OK &&
        xuiInputViewport(context, 360, 180) == XUI_OK &&
        xuiLayout(context) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 8, 12,
        XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, 8, 12, 0);
    source(document, checked);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, original);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 8, 12,
        XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_MOVE, 80, 12,
        XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, 80, 12, 0);
    source(document, original);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 8, 12,
        XUI_POINTER_BUTTON_LEFT);
    CHECK(xuiGetPointerCaptureEx(context, 7, XUI_POINTER_TYPE_MOUSE) == editor &&
        xuiReleasePointerCaptureEx(context, 7, XUI_POINTER_TYPE_MOUSE,
            editor) == XUI_OK);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, 8, 12, 0);
    source(document, original);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 80, 12,
        XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, 80, 12, 0);
    source(document, original);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot, 1, 0, &list) == XUI_OK &&
        xuiDocumentSnapshotGetChild(snapshot, list, 0, &item) == XUI_OK &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "first", 5, NULL, &first, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snapshot);
    first.tAnchor = first.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &first) == XUI_OK);
    state.iSize = sizeof(state);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TOGGLE_TASK,
        &state) == XUI_OK && state.bEnabled && !state.bActive);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TOGGLE_TASK) == XUI_OK);
    source(document, checked);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TOGGLE_TASK,
        &state) == XUI_OK && state.bEnabled && state.bActive);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, original);
    CHECK(xuiDocumentEditorToggleTaskItem(editor, item) == XUI_OK);
    source(document, checked);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, original);
    revision = xuiDocumentGetRevision(document);
    CHECK(xuiDocumentEditorToggleTaskItem(editor, UINT64_MAX) == XUI_ERROR_NOT_FOUND &&
        xuiDocumentGetRevision(document) == revision);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 8, 12,
        XUI_POINTER_BUTTON_LEFT);
    CHECK(xuiDocumentEditorToggleTaskItem(editor, item) == XUI_OK);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, 8, 12, 0);
    source(document, checked);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, original);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 8, 12,
        XUI_POINTER_BUTTON_LEFT);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK &&
        xuiGetPointerCaptureEx(context, 7, XUI_POINTER_TYPE_MOUSE) != editor);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, 8, 12, 0);
    source(document, original);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK &&
        xuiDocumentEditorToggleTaskItem(editor, item) == XUI_ERROR_INVALID_STATE &&
        xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TOGGLE_TASK,
            &state) == XUI_OK && !state.bEnabled);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 8, 12,
        XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, 8, 12, 0);
    source(document, original);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 8, 12,
        XUI_POINTER_BUTTON_LEFT);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
        xuiGetPointerCaptureEx(context, 7, XUI_POINTER_TYPE_MOUSE) != editor);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, 8, 12, 0);
    source(document, original);
    CHECK(
        xuiDocumentEditorToggleTaskItem(editor, item) == XUI_ERROR_UNSUPPORTED &&
        xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TOGGLE_TASK,
            &state) == XUI_OK && !state.bEnabled);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
        xuiDocumentEditorToggleTaskItem(editor, item) == XUI_ERROR_UNSUPPORTED &&
        xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TOGGLE_TASK,
            &state) == XUI_OK && !state.bEnabled);
    source(document, original);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK &&
        xuiDocumentLoadMarkdown(document, "- [ ]\n", 6) == XUI_OK &&
        xuiLayout(context) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 8, 12,
        XUI_POINTER_BUTTON_LEFT);
    send_pointer(context, editor, XUI_EVENT_POINTER_UP, 8, 12, 0);
    source(document, "- [x]\n");
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    {
        xui_doc_node_desc_t node = {0}; xui_document_transaction transaction;
        xui_doc_node_info_t info = {0};
        xui_doc_node_id rich_list, rich_item, paragraph, text_id;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node); node.iKind = XUI_DOC_LIST;
        CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND,
            &node, &rich_list) == XUI_OK);
        node.iKind = XUI_DOC_LIST_ITEM; node.tAttributes.iFlags = XUI_DOC_TASK;
        CHECK(xuiDocumentTxnInsertNode(transaction, rich_list,
            XUI_DOCUMENT_APPEND, &node, &rich_item) == XUI_OK);
        node.iKind = XUI_DOC_PARAGRAPH; node.tAttributes.iFlags = 0;
        CHECK(xuiDocumentTxnInsertNode(transaction, rich_item,
            XUI_DOCUMENT_APPEND, &node, &paragraph) == XUI_OK);
        node.iKind = XUI_DOC_TEXT; node.sText = "rich task";
        node.iTextBytes = strlen(node.sText);
        CHECK(xuiDocumentTxnInsertNode(transaction, paragraph,
            XUI_DOCUMENT_APPEND, &node, &text_id) == XUI_OK &&
            xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        ed.tView.pDocument = document;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 360, 180}) == XUI_OK &&
            xuiLayout(context) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
        send_pointer(context, editor, XUI_EVENT_POINTER_DOWN, 8, 12,
            XUI_POINTER_BUTTON_LEFT);
        send_pointer(context, editor, XUI_EVENT_POINTER_UP, 8, 12, 0);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        info.iSize = sizeof(info);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, rich_item, &info) == XUI_OK &&
            (info.tAttributes.iFlags & (XUI_DOC_TASK | XUI_DOC_CHECKED)) ==
                (XUI_DOC_TASK | XUI_DOC_CHECKED));
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, rich_item, &info) == XUI_OK &&
            (info.tAttributes.iFlags & (XUI_DOC_TASK | XUI_DOC_CHECKED)) ==
                XUI_DOC_TASK);
        xuiDocumentSnapshotRelease(snapshot);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("DocumentEditor task marker: Rich/Markdown pointer, cancel, command, API, Undo, read-only and modes passed");
}
static void document_vertical_goal_cases(xui_context context)
{
    const char* text = "0000000000\nx\n0000000000\n";
    xui_doc_desc_t md = {0}; xui_doc_editor_desc_t desc = {0};
    xui_document document; xui_widget editor;
    xui_doc_range_t selection; xui_event_t key = {0};
    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&md, &document) == XUI_OK &&
        xuiDocumentLoadMarkdown(document, text, strlen(text)) == XUI_OK);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = document; desc.iMode = XUI_DOC_SOURCE_TEXT;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 160}) == XUI_OK &&
        xuiInputViewport(context, 320, 160) == XUI_OK &&
        xuiLayout(context) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    selection.tAnchor.iOffset = selection.tCaret.iOffset = 8;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN;
    key.pTarget = editor; key.iKey = XUI_KEY_DOWN;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        selection.tCaret.iOffset == 12);
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        selection.tCaret.iOffset >= 18 && selection.tCaret.iOffset <= 22);
    key.iModifiers = XUI_MOD_SHIFT; key.iKey = XUI_KEY_UP;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        selection.tCaret.iOffset == 12 &&
        selection.tAnchor.iOffset >= 18);
    key.iKey = XUI_KEY_DOWN;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        selection.tCaret.iOffset >= 18);
    key.iModifiers = 0; key.iKey = XUI_KEY_HOME;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    key.iKey = XUI_KEY_UP;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    key.iKey = XUI_KEY_DOWN;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        selection.tCaret.iOffset <= 15);
    {
        const char* visual = "0000000000\n\nx\n\n0000000000\n";
        xui_document_snapshot snapshot; uint64_t count = 0;
        CHECK(xuiDocumentLoadMarkdown(document, visual, strlen(visual)) == XUI_OK &&
            xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK &&
            xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                "0000000000", 10, NULL, &selection, 1, &count) == XUI_OK &&
            count >= 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor.iOffset = selection.tCaret.iOffset = 8;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
            xuiLayout(context) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    }
    key.iKey = XUI_KEY_DOWN;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
        xuiDispatchEvent(context, &key) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    {
        xui_document_snapshot snapshot; uint64_t offset = 0; int exact = 0;
        int mapped;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        mapped = xuiDocumentPositionToSource(snapshot, &selection.tCaret,
            &offset, &exact);
        if (mapped != XUI_OK || offset < 20 || offset > 25)
            fprintf(stderr, "VISUAL vertical result=%d source=%llu node=%llu offset=%llu kind=%u\n",
                mapped, (unsigned long long)offset,
                (unsigned long long)selection.tCaret.iNodeId,
                (unsigned long long)selection.tCaret.iOffset,
                selection.tCaret.iKind);
        CHECK(mapped == XUI_OK && offset >= 20 && offset <= 25);
        xuiDocumentSnapshotRelease(snapshot);
    }
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    {
        xui_document_transaction transaction;
        xui_doc_node_desc_t node = {0};
        xui_doc_node_id paragraph, text_ids[3];
        const char* lines[3] = {"0000000000", "x", "0000000000"};
        int i;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
        node.iSize = sizeof(node);
        for (i = 0; i < 3; i++) {
            node.iKind = XUI_DOC_PARAGRAPH;
            node.tAttributes.iFlags = i == 0 ? XUI_DOC_SPACING_EXPLICIT : 0;
            node.tAttributes.fParagraphSpacing = i == 0 ? 80 : 0;
            node.sText = NULL; node.iTextBytes = 0;
            CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND,
                &node, &paragraph) == XUI_OK);
            node.iKind = XUI_DOC_TEXT; node.tAttributes.iFlags = 0;
            node.sText = lines[i]; node.iTextBytes = strlen(lines[i]);
            CHECK(xuiDocumentTxnInsertNode(transaction, paragraph,
                XUI_DOCUMENT_APPEND, &node, &text_ids[i]) == XUI_OK);
        }
        CHECK(xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
        xuiDocumentTxnRelease(transaction);
        desc.tView.pDocument = document; desc.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 300}) == XUI_OK &&
            xuiInputViewport(context, 320, 300) == XUI_OK &&
            xuiLayout(context) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK &&
            xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
        selection.tAnchor.iNodeId = selection.tCaret.iNodeId = text_ids[0];
        selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_TEXT;
        selection.tAnchor.iOffset = selection.tCaret.iOffset = 8;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        key.pTarget = editor; key.iKey = XUI_KEY_DOWN;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
            xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
            selection.tCaret.iNodeId == text_ids[1]);
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
            xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
            selection.tCaret.iNodeId == text_ids[2] &&
            selection.tCaret.iOffset >= 5);
        key.iKey = XUI_KEY_UP;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
            xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
            selection.tCaret.iNodeId == text_ids[1]);
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
            xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
            selection.tCaret.iNodeId == text_ids[0] &&
            selection.tCaret.iOffset >= 5);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("DocumentEditor vertical goal: SOURCE/VISUAL/Rich long-short-long, Shift selection, Home reset and large spacing passed");
}
static void document_cross_block_source_editor_cases(xui_context context)
{
    const char* input = "left &amp; keep\n\nsecond &amp; right\n\n[ref]: /keep\n";
    const char* expected = "left &amp; JOIN &amp; right\n\n[ref]: /keep\n";
    const char* multiline_expected = "left &amp; JOIN\n\nMORE &amp; right\n\n[ref]: /keep\n";
    const char* quote_multiline_input = "> left &amp; keep\n>\n> second &amp; right\n";
    const char* quote_multiline_expected = "> left &amp; JOIN\n>\n> MORE &amp; right\n";
    const char* list_multiline_input = "- [ ] left &amp; keep\n\n  second &amp; right\n";
    const char* list_multiline_expected = "- [ ] left &amp; JOIN\n\n  MORE &amp; right\n";
    const char* quote_list_multiline_input = "> - [ ] left &amp; keep\n>\n>   second &amp; right\n";
    const char* quote_list_multiline_expected = "> - [ ] left &amp; JOIN\n>\n>   MORE &amp; right\n";
    const char* cross_nbsp_expected = "left &amp; &#160;JOIN &amp; right\n\n[ref]: /keep\n";
    const char* entity_input = "before &amp; &fjlig; tail\n\nsecond &amp; right\n";
    const char* entity_expected = "before &amp; &#102;JOIN &amp; right\n";
    const char* tab_input = "abc &amp;\n";
    const char* tab_expected = "&#9;abc &amp;\n";
    const char* nbsp_input = "*abc*\n";
    const char* nbsp_expected = "*&#160;abc*\n";
    const char* with_definition = "left &amp; keep\n\n[unused]: /raw \"Title\"\n\nsecond &amp; right\n";
    const char* moved_definition = "left &amp; JOIN &amp; right\n\n[unused]: /raw \"Title\"\n\n";
    const char* nested_input = "> left &amp; keep\n>\n> [unused]: /raw \"T\"\n>\n> second &amp; right\n";
    const char* nested_expected = "> left &amp; JOIN &amp; right\n>\n> [unused]: /raw \"T\"\n>\n";
    const char* footnote_input = "body[^n]\n\n[^n]: left &amp; keep\n\n    second &amp; right\n\n[other]: /raw \"T\"\n";
    const char* footnote_expected = "body[^n]\n\n[^n]: left &amp; JOIN &amp; right\n\n[other]: /raw \"T\"\n";
    const char* footnote_multiline_expected = "body[^n]\n\n[^n]: left &amp; JOIN\n\n    MORE &amp; right\n\n[other]: /raw \"T\"\n";
    const char* quoted_footnote_multiline_input = "body[^n]\n\n> [^n]: left &amp; keep\n>\n>     second &amp; right\n";
    const char* quoted_footnote_multiline_expected = "body[^n]\n\n> [^n]: left &amp; JOIN\n>\n>     MORE &amp; right\n";
    const char* indented_footnote_multiline_input = "body[^n]\n\n  > [^n]: left &amp; keep\n  >\n  >     second &amp; right\n";
    const char* indented_footnote_multiline_expected = "body[^n]\n\n  > [^n]: left &amp; JOIN\n  >\n  >     MORE &amp; right\n";
    const char* cross_parent_input = "> left &amp; keep\n\nright &amp; tail\n\n[unused]: /raw \"T\"\n";
    const char* cross_parent_expected = "> left &amp; JOIN &amp; tail\n\n[unused]: /raw \"T\"\n";
    const char* cross_branch_input = "> intro\n>\n> left &amp; keep\n>\n> [unused]: /raw \"T\"\n\nright &amp; tail\n";
    const char* cross_branch_expected = "> intro\n>\n> left &amp; JOIN &amp; tail\n>\n> [unused]: /raw \"T\"\n";
    const char* bare_quote_input = "left &amp; keep\n\n> right &amp; tail\n>\n";
    const char* bare_quote_expected = "left &amp; JOIN &amp; tail\n>\n";
    const char* nested_bare_quote_input = "left &amp; keep\n\n- > right &amp; tail\n  >\n";
    const char* nested_bare_quote_expected = "left &amp; JOIN &amp; tail\n- > \n  >\n";
    const char* deep_bare_quote_input = "left &amp; keep\n\n- > > right &amp; tail\n  > >\n";
    const char* deep_bare_quote_expected = "left &amp; JOIN &amp; tail\n- > > \n  > >\n";
    const char* nested_list_quote_input = "left &amp; keep\n\n-\n  - > right &amp; tail\n    >\n";
    const char* nested_list_quote_expected = "left &amp; JOIN &amp; tail\n\n-\n  - > \n    >\n";
    const char* nested_list_parent_input = "left &amp; keep\n\n- outer &amp; gone\n  - > right &amp; tail\n    >\n  - sibling &amp; kept\n";
    const char* nested_list_parent_expected = "left &amp; JOIN &amp; tail\n\n- \n  - > \n    >\n  - sibling &amp; kept\n";
    const char* nested_list_siblings_input = "left &amp; keep\n\n- outer &amp; gone\n  - first &amp; gone\n  - second &amp; gone\n  - > right &amp; tail\n    >\n  - later &amp; kept\n";
    const char* nested_list_siblings_expected = "left &amp; JOIN &amp; tail\n\n- \n  - > \n    >\n  - later &amp; kept\n";
    const char* nested_list_definition_input = "left &amp; keep\n\n-\n  - earlier &amp; gone\n  - [unused]: /raw \"T\"\n  - > right &amp; tail\n    >\n";
    const char* nested_list_definition_expected = "left &amp; JOIN &amp; tail\n\n-\n  - [unused]: /raw \"T\"\n  - > \n    >\n";
    const char* nested_list_visible_definition_input = "left [r] &amp; keep\n\n-\n  - earlier &amp; gone\n  - body &amp; gone\n\n    [r]: /raw \"T\"\n  - > right &amp; tail\n    >\n  - later &amp; kept\n";
    const char* nested_list_visible_definition_expected = "left [r] &amp; JOIN &amp; tail\n\n-\n  - \n    [r]: /raw \"T\"\n  - > \n    >\n  - later &amp; kept\n";
    const char* nested_list_blank_definition_input = "left &amp; keep\n\n-\n  - body &amp; gone\n  \n\t\n    [unused]: /raw \"T\"\n  - > right &amp; tail\n    >\n";
    const char* nested_list_blank_definition_expected = "left &amp; JOIN &amp; tail\n\n-\n  - \n    [unused]: /raw \"T\"\n  - > \n    >\n";
    const char* nested_list_multiblock_definition_input = "left &amp; keep\n\n-\n  - first &amp; gone\n\n    ~~~c\n    int x;\n    ~~~\n\n    [unused]: /raw \"T\"\n  - > right &amp; tail\n    >\n";
    const char* nested_list_multiblock_definition_expected = "left &amp; JOIN &amp; tail\n\n-\n  - \n    [unused]: /raw \"T\"\n  - > \n    >\n";
    const char* nested_list_middle_definition_input = "left [r] &amp; keep\n\n-\n  - first &amp; gone\n\n    [r]: /raw \"T\"\n\n    second &amp; gone\n  - > right &amp; tail\n    >\n";
    const char* nested_list_middle_definition_expected = "left [r] &amp; JOIN &amp; tail\n\n-\n  - \n    [r]: /raw \"T\"\n  - > \n    >\n";
    const char* nested_list_fence_definition_input = "left [r] &amp; keep\n\n-\n  - ~~~c\n    int x;\n    ~~~\n\n    [r]: /raw \"T\"\n\n    second &amp; gone\n  - > right &amp; tail\n    >\n";
    const char* nested_list_fence_definition_expected = "left [r] &amp; JOIN &amp; tail\n\n-\n  - \n    [r]: /raw \"T\"\n  - > \n    >\n";
    const char* parent_following_input = "left &amp; keep\n\n- outer &amp; gone\n\n  ~~~c\n  int x;\n  ~~~\n\n  - > right &amp; tail\n    >\n";
    const char* parent_following_expected = "left &amp; JOIN &amp; tail\n\n- \n  - > \n    >\n";
    const char* parent_middle_definition_input = "left [r] &amp; keep\n\n- outer &amp; gone\n\n  ~~~c\n  int x;\n  ~~~\n\n  [r]: /raw \"T\"\n\n  - > right &amp; tail\n    >\n";
    const char* parent_middle_definition_expected = "left [r] &amp; JOIN &amp; tail\n\n- \n  [r]: /raw \"T\"\n  - > \n    >\n";
    const char* parent_nonparagraph_input = "left [r] &amp; keep\n\n-\n  ~~~c\n  int x;\n  ~~~\n\n  [r]: /raw \"T\"\n\n  second &amp; gone\n\n  - > right &amp; tail\n    >\n";
    const char* parent_nonparagraph_expected = "left [r] &amp; JOIN &amp; tail\n\n-\n  [r]: /raw \"T\"\n  - > \n    >\n";
    const char* parent_table_input = "left &amp; keep\n\n- | a | b |\n  | - | - |\n  | x | y |\n\n  - > right &amp; tail\n    >\n";
    const char* parent_table_expected = "left &amp; JOIN &amp; tail\n\n- \n  - > \n    >\n";
    const char* parent_footnote_input = "left [^n] &amp; keep\n\n- outer &amp; gone\n\n  [^n]: note &amp; held\n\n  - > right &amp; tail\n    >\n";
    const char* parent_footnote_expected = "left [^n] &amp; JOIN &amp; tail\n\n- \n  [^n]: note &amp; held\n  - > \n    >\n";
    const char* parent_mixed_definition_input = "left [r] [^n] &amp; keep\n\n- outer &amp; gone\n\n  [^n]: note &amp; held\n\n  [r]: /raw \"T\"\n\n  - > right &amp; tail\n    >\n  - later &amp; kept\n";
    const char* parent_mixed_definition_expected = "left [r] [^n] &amp; JOIN &amp; tail\n\n  [^n]: note &amp; held\n\n  [r]: /raw \"T\"\n\n- \n  - > \n    >\n  - later &amp; kept\n";
    const char* root_footnote_gap_input = "left [^n] &amp; keep\n\n[^n]: note &amp; held\n\nsecond &amp; right\n";
    const char* root_footnote_gap_expected = "left [^n] &amp; JOIN &amp; right\n\n[^n]: note &amp; held\n\n";
    xui_doc_desc_t markdown = {0}; xui_doc_editor_desc_t desc = {0};
    xui_document document; xui_document_snapshot snapshot;
    xui_doc_range_t left, right, selection; xui_widget editor;
    uint64_t count;
    markdown.iSize = sizeof(markdown); markdown.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&markdown, &document) == XUI_OK &&
        xuiDocumentLoadMarkdown(document, input, strlen(input)) == XUI_OK);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = document; desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 240}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, expected);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        selection.tAnchor.iNodeId == selection.tCaret.iNodeId &&
        selection.tAnchor.iOffset == selection.tCaret.iOffset);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, input, strlen(input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN\nMORE", 9) == XUI_OK);
    source(document, multiline_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, quote_multiline_input,
        strlen(quote_multiline_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN\nMORE", 9) == XUI_OK);
    source(document, quote_multiline_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, quote_multiline_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, quote_multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, quote_multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, list_multiline_input,
        strlen(list_multiline_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN\nMORE", 9) == XUI_OK);
    source(document, list_multiline_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, list_multiline_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, list_multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, list_multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, quote_list_multiline_input,
        strlen(quote_list_multiline_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN\nMORE", 9) == XUI_OK);
    source(document, quote_list_multiline_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, quote_list_multiline_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, quote_list_multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, quote_list_multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, input, strlen(input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "\xc2\xa0JOIN", 6) == XUI_OK);
    source(document, cross_nbsp_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, cross_nbsp_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, cross_nbsp_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, entity_input, strlen(entity_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "fj", 2, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tAnchor.iOffset++;
    selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, entity_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, entity_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, entity_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, entity_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, tab_input, strlen(tab_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "abc", 3, NULL, &left, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = selection.tCaret = left.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "\t", 1) == XUI_OK);
    source(document, tab_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, tab_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, tab_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, tab_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, nbsp_input, strlen(nbsp_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "abc", 3, NULL, &left, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = selection.tCaret = left.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "\xc2\xa0", 2) == XUI_OK);
    source(document, nbsp_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, nbsp_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, nbsp_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, nbsp_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, with_definition,
        strlen(with_definition)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, moved_definition);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, with_definition);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, moved_definition);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, moved_definition);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, nested_input,
        strlen(nested_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, nested_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, nested_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, nested_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, nested_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, footnote_input,
        strlen(footnote_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, footnote_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, footnote_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, footnote_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, footnote_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, footnote_input,
        strlen(footnote_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN\nMORE", 9) == XUI_OK);
    source(document, footnote_multiline_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, footnote_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, footnote_multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, footnote_multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, quoted_footnote_multiline_input,
        strlen(quoted_footnote_multiline_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN\nMORE", 9) == XUI_OK);
    source(document, quoted_footnote_multiline_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, quoted_footnote_multiline_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, quoted_footnote_multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, quoted_footnote_multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, indented_footnote_multiline_input,
        strlen(indented_footnote_multiline_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN\nMORE", 9) == XUI_OK);
    source(document, indented_footnote_multiline_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, indented_footnote_multiline_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, indented_footnote_multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, indented_footnote_multiline_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, cross_parent_input,
        strlen(cross_parent_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, cross_parent_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, cross_parent_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, cross_parent_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, cross_parent_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, cross_branch_input,
        strlen(cross_branch_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, cross_branch_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, cross_branch_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, cross_branch_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, cross_branch_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, bare_quote_input,
        strlen(bare_quote_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, bare_quote_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, bare_quote_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, bare_quote_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, bare_quote_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, nested_bare_quote_input,
        strlen(nested_bare_quote_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, nested_bare_quote_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, nested_bare_quote_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, nested_bare_quote_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, nested_bare_quote_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, deep_bare_quote_input,
        strlen(deep_bare_quote_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, deep_bare_quote_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, deep_bare_quote_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, deep_bare_quote_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, deep_bare_quote_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, nested_list_quote_input,
        strlen(nested_list_quote_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, nested_list_quote_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, nested_list_quote_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, nested_list_quote_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, nested_list_quote_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, nested_list_parent_input,
        strlen(nested_list_parent_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, nested_list_parent_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, nested_list_parent_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, nested_list_parent_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, nested_list_parent_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, nested_list_siblings_input,
        strlen(nested_list_siblings_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, nested_list_siblings_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, nested_list_siblings_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, nested_list_siblings_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, nested_list_siblings_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, nested_list_definition_input,
        strlen(nested_list_definition_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, nested_list_definition_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, nested_list_definition_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, nested_list_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, nested_list_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, nested_list_visible_definition_input,
        strlen(nested_list_visible_definition_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, nested_list_visible_definition_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, nested_list_visible_definition_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, nested_list_visible_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, nested_list_visible_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, nested_list_blank_definition_input,
        strlen(nested_list_blank_definition_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, nested_list_blank_definition_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, nested_list_blank_definition_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, nested_list_blank_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, nested_list_blank_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, nested_list_multiblock_definition_input,
        strlen(nested_list_multiblock_definition_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, nested_list_multiblock_definition_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, nested_list_multiblock_definition_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, nested_list_multiblock_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, nested_list_multiblock_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, nested_list_middle_definition_input,
        strlen(nested_list_middle_definition_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, nested_list_middle_definition_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, nested_list_middle_definition_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, nested_list_middle_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, nested_list_middle_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, nested_list_fence_definition_input,
        strlen(nested_list_fence_definition_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, nested_list_fence_definition_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, nested_list_fence_definition_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, nested_list_fence_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, nested_list_fence_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, parent_following_input,
        strlen(parent_following_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, parent_following_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, parent_following_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, parent_following_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, parent_following_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, parent_middle_definition_input,
        strlen(parent_middle_definition_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, parent_middle_definition_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, parent_middle_definition_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, parent_middle_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, parent_middle_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, parent_nonparagraph_input,
        strlen(parent_nonparagraph_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, parent_nonparagraph_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, parent_nonparagraph_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, parent_nonparagraph_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, parent_nonparagraph_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, parent_table_input,
        strlen(parent_table_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, parent_table_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, parent_table_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, parent_table_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, parent_table_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, parent_footnote_input,
        strlen(parent_footnote_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, parent_footnote_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, parent_footnote_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, parent_footnote_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, parent_footnote_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, parent_mixed_definition_input,
        strlen(parent_mixed_definition_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "right", 5, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, parent_mixed_definition_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, parent_mixed_definition_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, parent_mixed_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, parent_mixed_definition_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, root_footnote_gap_input,
        strlen(root_footnote_gap_input)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "keep", 4, NULL, &left, 1, &count) == XUI_OK && count == 1);
    CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
        "second", 6, NULL, &right, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot);
    selection.tAnchor = left.tAnchor; selection.tCaret = right.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorInsertText(editor, "JOIN", 4) == XUI_OK);
    source(document, root_footnote_gap_expected);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(document, root_footnote_gap_input);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
    source(document, root_footnote_gap_expected);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    source(document, root_footnote_gap_expected);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    puts("DocumentEditor cross-block Markdown VISUAL replacement retains root/nested/footnote/cross-branch/bare/deep-quote/nested-list sibling source with one-step Undo");
}
static void list_create_editor_cases(xui_context context)
{
    xui_doc_desc_t md = {0}; xui_doc_editor_desc_t ed = {0};
    xui_document d; xui_document_snapshot snap; xui_widget editor;
    xui_doc_range_t range; xui_doc_command_state_t state = {0}; uint64_t found;
    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    md.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&md, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, "alpha\n\nbeta\n", 12) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
    ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "alpha", 5,
        NULL, &range, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snap);
    range.tAnchor.iKind = range.tCaret.iKind = XUI_DOC_POSITION_GAP;
    range.tAnchor.iNodeId = range.tCaret.iNodeId = 1;
    range.tAnchor.iOffset = 0; range.tCaret.iOffset = 2;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    state.iSize = sizeof(state);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BULLET_LIST, &state) == XUI_OK && state.bEnabled);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_NUMBER_LIST, &state) == XUI_OK && state.bEnabled);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TASK_LIST, &state) == XUI_OK && state.bEnabled);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_NUMBER_LIST) == XUI_OK);
    source(d, "1. alpha\n2. beta\n\n\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "alpha\n\nbeta\n");
    range.tAnchor.iRevision = range.tCaret.iRevision = xuiDocumentGetRevision(d);
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TASK_LIST) == XUI_OK);
    source(d, "- [ ] alpha\n- [ ] beta\n\n\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, "alpha\n\nbeta\n");
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BULLET_LIST, &state) == XUI_OK && !state.bEnabled);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK &&
        xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BULLET_LIST, &state) == XUI_OK && !state.bEnabled);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    md.iMarkdownDialect = XUI_MD_COMMONMARK;
    CHECK(xuiDocumentCreate(&md, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, "alpha\n", 6) == XUI_OK);
    ed.tView.pDocument = d;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "alpha", 5,
        NULL, &range, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snap);
    range.tAnchor = range.tCaret;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TASK_LIST, &state) == XUI_OK &&
        !state.bEnabled && state.iDisabledReason == XUI_DOC_ERROR_UNREPRESENTABLE);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BULLET_LIST, &state) == XUI_OK && state.bEnabled);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    puts("DocumentEditor list creation: GFM commands, Undo, read-only, SOURCE and CommonMark policy passed");
}
static void list_restyle_editor_cases(xui_context context)
{
    const char* original = "- alpha\n- beta\n- gamma\n";
    xui_doc_desc_t md = {0}; xui_doc_editor_desc_t ed = {0};
    xui_document d; xui_document_snapshot snap; xui_widget editor;
    xui_doc_range_t range; xui_doc_command_state_t state = {0};
    xui_doc_node_info_t info = {0}; uint64_t found, list, item;
    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    md.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&md, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
    ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "beta", 4,
        NULL, &range, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snap);
    range.tAnchor = range.tCaret; range.tAnchor.iOffset = range.tCaret.iOffset = 2;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    state.iSize = sizeof(state); info.iSize = sizeof(info);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BULLET_LIST, &state) == XUI_OK &&
        state.bEnabled && state.bActive);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_NUMBER_LIST, &state) == XUI_OK &&
        state.bEnabled && !state.bActive);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_NUMBER_LIST) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_NUMBER_LIST, &state) == XUI_OK &&
        state.bEnabled && state.bActive);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snap, 1, 1, &list) == XUI_OK &&
        xuiDocumentSnapshotGetNode(snap, list, &info) == XUI_OK &&
        info.iKind == XUI_DOC_LIST && (info.tAttributes.iFlags & XUI_DOC_ORDERED));
    xuiDocumentSnapshotRelease(snap);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_NUMBER_LIST) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snap, 1, 1, &item) == XUI_OK &&
        xuiDocumentSnapshotGetNode(snap, item, &info) == XUI_OK &&
        info.iKind == XUI_DOC_PARAGRAPH);
    xuiDocumentSnapshotRelease(snap);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, original);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    puts("DocumentEditor list restyle/toggle: selected item, active state, unlist and Undo passed");
}
static void empty_list_unlist_editor_cases(xui_context context)
{
    const char* originals[] = {"- \n", "- [ ]\n"};
    const uint32_t commands[] = {XUI_DOC_EDIT_BULLET_LIST, XUI_DOC_EDIT_TASK_LIST};
    unsigned i;
    for (i = 0; i < 2; i++) {
        xui_doc_desc_t md = {0}; xui_doc_editor_desc_t ed = {0};
        xui_document d; xui_document_snapshot snap; xui_widget editor;
        xui_doc_range_t range; xui_doc_command_state_t state = {0};
        uint64_t list;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        md.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, originals[i], strlen(originals[i])) == XUI_OK);
        ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snap, 1, 0, &list) == XUI_OK);
        xuiDocumentSnapshotRelease(snap);
        memset(&range, 0, sizeof(range));
        range.tAnchor.iSize = sizeof(range.tAnchor);
        range.tAnchor.iDocumentId = xuiDocumentGetIdentity(d);
        range.tAnchor.iRevision = xuiDocumentGetRevision(d);
        range.tAnchor.iNodeId = list; range.tAnchor.iKind = XUI_DOC_POSITION_GAP;
        range.tCaret = range.tAnchor; range.tCaret.iOffset = 1;
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, commands[i], &state) == XUI_OK &&
            state.bEnabled && state.bActive);
        CHECK(xuiDocumentEditorExecute(editor, commands[i]) == XUI_OK);
        source(d, "");
        CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
            range.tAnchor.iKind == XUI_DOC_POSITION_GAP &&
            range.tAnchor.iNodeId == 1 && range.tAnchor.iOffset == 0 &&
            range.tCaret.iNodeId == 1 && range.tCaret.iOffset == 0);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(d, originals[i]);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    puts("DocumentEditor empty list toggle: bullet/task command, collapsed gap and Undo passed");
}
static void cross_list_restyle_editor_cases(xui_context context)
{
    const char* original = "- alpha\n- beta\n1. gamma\n2. delta\n";
    xui_doc_desc_t md = {0}; xui_doc_editor_desc_t ed = {0};
    xui_document d; xui_document_snapshot snap; xui_widget editor;
    xui_doc_range_t beta, gamma, selection;
    xui_doc_command_state_t state = {0};
    xui_doc_node_info_t info = {0}; uint64_t found, paragraph;
    int direction;
    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    md.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&md, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
    ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "beta", 4,
        NULL, &beta, 1, &found) == XUI_OK && found == 1);
    CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "gamma", 5,
        NULL, &gamma, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snap);
    selection.tAnchor = gamma.tCaret; selection.tCaret = beta.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    state.iSize = sizeof(state); info.iSize = sizeof(info);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TASK_LIST, &state) == XUI_OK &&
        state.bEnabled && !state.bActive);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_BULLET_LIST, &state) == XUI_OK &&
        state.bEnabled && state.bMixed);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TASK_LIST) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TASK_LIST, &state) == XUI_OK &&
        state.bEnabled && state.bActive);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotComparePositions(snap, &selection.tAnchor,
        &selection.tCaret, &direction) == XUI_OK && direction > 0);
    xuiDocumentSnapshotRelease(snap);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TASK_LIST) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snap, beta.tAnchor.iNodeId, &info) == XUI_OK);
    paragraph = info.iParentId;
    CHECK(xuiDocumentSnapshotGetNode(snap, paragraph, &info) == XUI_OK && info.iParentId == 1);
    CHECK(xuiDocumentSnapshotGetNode(snap, gamma.tAnchor.iNodeId, &info) == XUI_OK);
    paragraph = info.iParentId;
    CHECK(xuiDocumentSnapshotGetNode(snap, paragraph, &info) == XUI_OK && info.iParentId == 1);
    xuiDocumentSnapshotRelease(snap);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, original);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    puts("DocumentEditor cross-list style/toggle: mixed state, reverse selection, unlist and Undo passed");
}
static void list_parent_gap_editor_cases(xui_context context)
{
    const char* original = "- alpha\n- beta\n1. gamma\n2. delta\n";
    xui_doc_desc_t md = {0}; xui_doc_editor_desc_t ed = {0};
    xui_document d; xui_document_snapshot snap; xui_widget editor;
    xui_doc_range_t range; xui_doc_command_state_t state = {0};
    xui_doc_node_info_t info = {0}; uint64_t found;
    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    md.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&md, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
    ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "alpha", 5,
        NULL, &range, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snap);
    range.tAnchor.iKind = XUI_DOC_POSITION_GAP;
    range.tAnchor.iNodeId = 1; range.tAnchor.iOffset = 0;
    range.tCaret = range.tAnchor; range.tCaret.iOffset = 2;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    state.iSize = sizeof(state); info.iSize = sizeof(info);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TASK_LIST, &state) == XUI_OK &&
        state.bEnabled && !state.bActive);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TASK_LIST) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TASK_LIST, &state) == XUI_OK &&
        state.bEnabled && state.bActive);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TASK_LIST) == XUI_OK);
    CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
        range.tAnchor.iKind == XUI_DOC_POSITION_GAP &&
        range.tCaret.iKind == XUI_DOC_POSITION_GAP &&
        range.tAnchor.iNodeId == 1 && range.tCaret.iNodeId == 1 &&
        range.tAnchor.iOffset == 0 && range.tCaret.iOffset == 4);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snap, 1, &info) == XUI_OK && info.iChildCount == 4);
    xuiDocumentSnapshotRelease(snap);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, original);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    puts("DocumentEditor whole-list parent gap: style toggle, mapped selection and Undo passed");
}
static void move_block_editor_cases(xui_context context)
{
    const char* original = "alpha\n\nbeta\n\ngamma\n";
    xui_doc_desc_t md = {0}; xui_doc_editor_desc_t ed = {0};
    xui_document d; xui_document_snapshot snap; xui_widget editor;
    xui_doc_range_t range; xui_doc_command_state_t state = {0};
    xui_doc_node_info_t info = {0}; xui_event_t key = {0};
    uint64_t found, beta_id, child;
    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    md.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&md, &d) == XUI_OK &&
        xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
    ed.iSize = sizeof(ed); ed.tView.iSize = sizeof(ed.tView);
    ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
    CHECK(xuiSetFocusWidget(context, editor) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "beta", 4,
        NULL, &range, 1, &found) == XUI_OK && found == 1);
    xuiDocumentSnapshotRelease(snap);
    beta_id = range.tAnchor.iNodeId;
    range.tAnchor = range.tCaret; range.tAnchor.iOffset = range.tCaret.iOffset = 2;
    CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
    state.iSize = sizeof(state); info.iSize = sizeof(info);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_MOVE_BLOCK_UP, &state) == XUI_OK && state.bEnabled);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_MOVE_BLOCK_DOWN, &state) == XUI_OK && state.bEnabled);
    key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor;
    key.iKey = XUI_KEY_UP; key.iModifiers = XUI_MOD_ALT;
    CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snap, beta_id, &info) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snap, 1, 0, &child) == XUI_OK && child == info.iParentId);
    xuiDocumentSnapshotRelease(snap);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_MOVE_BLOCK_UP, &state) == XUI_OK && !state.bEnabled);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_MOVE_BLOCK_DOWN) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetNode(snap, beta_id, &info) == XUI_OK);
    CHECK(xuiDocumentSnapshotGetChild(snap, 1, 1, &child) == XUI_OK && child == info.iParentId);
    xuiDocumentSnapshotRelease(snap);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(d, original);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_MOVE_BLOCK_UP, &state) == XUI_OK && !state.bEnabled);
    xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    puts("DocumentEditor move block: command state, Alt+Up, Down, selection identity, Undo and SOURCE policy passed");
}
static int document_menu_item(xui_widget menu, uint32_t command)
{
    int i;
    for (i = 0; i < xuiMenuGetItemCount(menu); i++) {
        const xui_menu_item_t* item = xuiMenuGetItem(menu, i);
        if (item && item->iType != XUI_MENU_ITEM_SEPARATOR &&
            item->iValue == (int)command) return i;
    }
    return -1;
}
typedef struct document_menu_destroy_observer {
    xui_widget editor;
    unsigned calls;
} document_menu_destroy_observer;
static void document_menu_destroy_on_change(xui_document document,
    xui_document_change_set change, void* user)
{
    document_menu_destroy_observer* observer = user;
    (void)document; (void)change;
    observer->calls++;
    xuiWidgetDestroy(observer->editor);
}
static void document_menu_cases(xui_context context)
{
    xui_document document = NULL, markdown = NULL;
    xui_document_transaction transaction = NULL;
    xui_document_snapshot snapshot = NULL;
    xui_doc_node_desc_t node = {0};
    xui_doc_editor_desc_t desc = {0};
    xui_doc_range_t selection = {0}, after = {0};
    xui_event_t event = {0};
    xui_widget editor = NULL, menu = NULL;
    xui_language custom = NULL;
    document_menu_destroy_observer observer = {0};
    uint64_t paragraph, leaf, count = 0, token = 0;
    int copy, cut, paste, delete_item, undo, redo;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &transaction) == XUI_OK);
    node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(transaction, 1, XUI_DOCUMENT_APPEND,
        &node, &paragraph) == XUI_OK);
    node.iKind = XUI_DOC_TEXT; node.sText = "alpha"; node.iTextBytes = 5;
    CHECK(xuiDocumentTxnInsertNode(transaction, paragraph, XUI_DOCUMENT_APPEND,
        &node, &leaf) == XUI_OK && xuiDocumentTxnCommit(transaction, NULL) == XUI_OK);
    xuiDocumentTxnRelease(transaction); transaction = NULL;
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 180}) == XUI_OK &&
        xuiInputViewport(context, 400, 180) == XUI_OK && xuiLayout(context) == XUI_OK);
    CHECK(xuiDocumentEditorGetMenuWidget(editor) == NULL);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
            "alpha", 5, NULL, &selection, 1, &count) == XUI_OK && count == 1);
    xuiDocumentSnapshotRelease(snapshot); snapshot = NULL;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    event.iSize = sizeof(event); event.iType = XUI_EVENT_CONTEXT_MENU;
    event.pTarget = editor; event.iPointerType = XUI_POINTER_TYPE_MOUSE;
    event.iButton = XUI_POINTER_BUTTON_RIGHT; event.fX = 20; event.fY = 8;
    CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
    menu = xuiDocumentEditorGetMenuWidget(editor);
    CHECK(menu && xuiMenuIsOpen(menu) && xuiMenuGetItemCount(menu) == 9);
    CHECK(document_menu_item(menu, XUI_DOC_EDIT_TABLE_MERGE_CELLS) < 0);
    copy = document_menu_item(menu, XUI_DOC_EDIT_COPY);
    cut = document_menu_item(menu, XUI_DOC_EDIT_CUT);
    paste = document_menu_item(menu, XUI_DOC_EDIT_PASTE);
    delete_item = document_menu_item(menu, XUI_DOC_EDIT_DELETE);
    undo = document_menu_item(menu, XUI_DOC_EDIT_UNDO);
    redo = document_menu_item(menu, XUI_DOC_EDIT_REDO);
    CHECK(copy >= 0 && cut >= 0 && paste >= 0 && delete_item >= 0 &&
        undo >= 0 && redo >= 0 &&
        (xuiMenuGetItemState(menu, copy) & XUI_MENU_ITEM_ENABLED) &&
        (xuiMenuGetItemState(menu, cut) & XUI_MENU_ITEM_ENABLED) &&
        (xuiMenuGetItemState(menu, undo) & XUI_MENU_ITEM_ENABLED) &&
        !(xuiMenuGetItemState(menu, redo) & XUI_MENU_ITEM_ENABLED));
    CHECK(xuiDocumentViewGetSelection(editor, &after) == XUI_OK &&
        after.tAnchor.iNodeId == selection.tAnchor.iNodeId &&
        after.tAnchor.iOffset == selection.tAnchor.iOffset &&
        after.tCaret.iOffset == selection.tCaret.iOffset);
    CHECK(xuiMenuClose(menu) == XUI_OK);
    event.fX = 320;
    CHECK(xuiDispatchEvent(context, &event) == XUI_OK);
    CHECK(xuiMenuIsOpen(menu));
    CHECK(!(xuiMenuGetItemState(menu, copy) & XUI_MENU_ITEM_ENABLED));
    CHECK(!(xuiMenuGetItemState(menu, cut) & XUI_MENU_ITEM_ENABLED));
    CHECK(xuiDocumentViewGetSelection(editor, &after) == XUI_OK &&
        after.tAnchor.iNodeId == after.tCaret.iNodeId &&
        after.tAnchor.iOffset == after.tCaret.iOffset);
    CHECK(xuiMenuClose(menu) == XUI_OK);
    CHECK(xuiSetLanguage(context, XUI_LANGUAGE_ZH) == XUI_OK &&
        xuiDocumentEditorOpenMenu(editor, 40, 30) == XUI_OK &&
        !strcmp(xuiMenuGetItem(menu, undo)->sText,
            xuiTranslate(context, XUI_TR_EDIT_UNDO)));
    CHECK(xuiMenuClose(menu) == XUI_OK);
    custom = xuiCreateLanguage(context, "doc-menu-test", "Document Menu Test", XUI_LANGUAGE_EN);
    CHECK(custom && xuiLanguageSetText(context, custom, XUI_TR_EDIT_COPY, "First") == XUI_OK &&
        xuiSetLanguage(context, xuiGetLanguageId(custom)) == XUI_OK &&
        xuiDocumentEditorOpenMenu(editor, 40, 30) == XUI_OK &&
        !strcmp(xuiMenuGetItem(menu, copy)->sText, "First"));
    CHECK(xuiLanguageSetText(context, custom, XUI_TR_EDIT_COPY, "Second") == XUI_OK &&
        !strcmp(xuiMenuGetItem(menu, copy)->sText, "First") &&
        xuiMenuClose(menu) == XUI_OK &&
        xuiDocumentEditorOpenMenu(editor, 40, 30) == XUI_OK &&
        !strcmp(xuiMenuGetItem(menu, copy)->sText, "Second"));
    CHECK(xuiMenuClose(menu) == XUI_OK && xuiSetLanguage(context, XUI_LANGUAGE_EN) == XUI_OK);
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK &&
        xuiDocumentEditorOpenMenu(editor, 40, 30) == XUI_OK &&
        (xuiMenuGetItemState(menu, copy) & XUI_MENU_ITEM_ENABLED) &&
        !(xuiMenuGetItemState(menu, cut) & XUI_MENU_ITEM_ENABLED) &&
        !(xuiMenuGetItemState(menu, paste) & XUI_MENU_ITEM_ENABLED) &&
        !(xuiMenuGetItemState(menu, delete_item) & XUI_MENU_ITEM_ENABLED) &&
        !(xuiMenuGetItemState(menu, undo) & XUI_MENU_ITEM_ENABLED));
    CHECK(xuiMenuClose(menu) == XUI_OK &&
        xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK &&
        xuiDocumentEditorOpenMenu(editor, 40, 30) == XUI_OK &&
        xuiMenuSetHoverIndex(menu, undo) == XUI_OK &&
        xuiMenuCommitHover(menu) == XUI_EVENT_DISPATCH_STOP);
    plain(document, "");
    CHECK(xuiDocumentEditorOpenMenu(editor, 40, 30) == XUI_OK &&
        (xuiMenuGetItemState(menu, redo) & XUI_MENU_ITEM_ENABLED) &&
        xuiMenuSetHoverIndex(menu, redo) == XUI_OK &&
        xuiMenuCommitHover(menu) == XUI_EVENT_DISPATCH_STOP);
    plain(document, "alpha\n");
    event.iKey = XUI_KEY_CONTEXT_MENU; event.fX = event.fY = 0;
    CHECK(xuiDispatchEvent(context, &event) == XUI_OK && xuiMenuIsOpen(menu));
    CHECK(xuiMenuClose(menu) == XUI_OK);
    observer.editor = editor;
    CHECK(xuiDocumentSubscribe(document, document_menu_destroy_on_change,
        &observer, &token) == XUI_OK &&
        xuiDocumentEditorOpenMenu(editor, 40, 30) == XUI_OK &&
        xuiMenuSetHoverIndex(menu, undo) == XUI_OK &&
        xuiMenuCommitHover(menu) == XUI_EVENT_DISPATCH_STOP && observer.calls == 1);
    plain(document, "");
    xuiDocumentUnsubscribe(document, token);
    editor = NULL; menu = NULL;
    CHECK(xuiDocumentCreate(&(xui_doc_desc_t){.iSize=sizeof(xui_doc_desc_t),
        .iProfile=XUI_DOCUMENT_MARKDOWN}, &markdown) == XUI_OK &&
        xuiDocumentLoadMarkdown(markdown, "alpha\n", 6) == XUI_OK);
    desc.tView.pDocument = markdown;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 400, 180}) == XUI_OK &&
        xuiDocumentEditorOpenMenu(editor, 40, 30) == XUI_OK);
    menu = xuiDocumentEditorGetMenuWidget(editor);
    CHECK(menu && !(xuiMenuGetItemState(menu,
        document_menu_item(menu, XUI_DOC_EDIT_COPY)) & XUI_MENU_ITEM_ENABLED));
    CHECK(xuiMenuClose(menu) == XUI_OK &&
        xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
        xuiDocumentEditorOpenMenu(editor, 40, 30) == XUI_OK &&
        xuiMenuGetItemCount(menu) == 9);
    CHECK(xuiMenuClose(menu) == XUI_OK);
    xuiWidgetDestroy(editor); xuiDocumentRelease(markdown); xuiDocumentRelease(document);
    puts("DocumentEditor Menu: right-click selection, keyboard, commands, read-only, locale and Markdown modes passed");
}
static xui_widget document_find_child(xui_widget window, xui_widget_type type, int ordinal)
{
    xui_widget child;
    for (child = xuiWidgetGetFirstChild(xuiWindowGetClientWidget(window)); child;
         child = xuiWidgetGetNextSibling(child)) {
        if (xuiWidgetIsType(child, type) && ordinal-- == 0) return child;
    }
    return NULL;
}
static xui_widget document_find_button(xui_context context, xui_widget window, int text_id)
{
    xui_widget child;
    const char* text = xuiTranslate(context, text_id);
    for (child = xuiWidgetGetFirstChild(xuiWindowGetClientWidget(window)); child;
         child = xuiWidgetGetNextSibling(child)) {
        if (xuiWidgetIsType(child, xuiButtonGetType(context)) &&
            !strcmp(xuiButtonGetText(child), text)) return child;
    }
    return NULL;
}
static xui_widget document_find_checkbox(xui_context context,
    xui_widget window, int text_id)
{
    xui_widget child;
    const char* text = xuiTranslate(context, text_id);
    for (child = xuiWidgetGetFirstChild(xuiWindowGetClientWidget(window)); child;
         child = xuiWidgetGetNextSibling(child)) {
        if (xuiWidgetIsType(child, xuiCheckBoxGetType(context)) &&
            !strcmp(xuiCheckBoxGetText(child), text)) return child;
    }
    return NULL;
}
static void document_find_click(xui_context context, xui_widget button)
{
    xui_event_t event = {0};
    CHECK(button != NULL);
    event.iSize = sizeof(event); event.iType = XUI_EVENT_POINTER_CLICK;
    event.pTarget = button; event.iButton = XUI_POINTER_BUTTON_LEFT;
    CHECK(xuiDispatchEvent(context, &event) >= XUI_OK);
}
static void document_find_ui_cases(xui_context context, xui_surface target)
{
    xui_document rich = NULL, markdown = NULL;
    xui_doc_editor_desc_t desc = {0};
    xui_widget editor = NULL, window, input, replacement, table;
    xui_widget case_sensitive, whole_word, regex, selection_only, status;
    xui_doc_range_t match = {0}, selection = {0};
    xui_event_t event = {0};
    xui_rect_i_t damage = {0, 0, 640, 420};
    document_menu_destroy_observer observer = {0};
    uint64_t count = 0, token = 0;
    CHECK(xuiDocumentCreate(NULL, &rich) == XUI_OK);
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = rich;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 700, 420}) == XUI_OK &&
        xuiInputViewport(context, 700, 420) == XUI_OK &&
        xuiEditSetText(editor, "alpha beta alpha") == XUI_OK);
    CHECK(xuiDocumentEditorGetFindWindow(editor) == NULL &&
        xuiDocumentEditorOpenFind(editor) == XUI_OK);
    window = xuiDocumentEditorGetFindWindow(editor);
    CHECK(window && xuiWindowIsOpen(window));
    input = document_find_child(window, xuiInputGetType(context), 0);
    replacement = document_find_child(window, xuiInputGetType(context), 1);
    table = document_find_child(window, xuiTableViewGetType(context), 0);
    case_sensitive = document_find_checkbox(context, window, XUI_TR_FIND_CASE);
    whole_word = document_find_checkbox(context, window, XUI_TR_FIND_WORD);
    regex = document_find_checkbox(context, window, XUI_TR_FIND_REGEX);
    selection_only = document_find_checkbox(context, window, XUI_TR_FIND_SELECTION);
    status = document_find_child(window, xuiLabelGetType(context), 0);
    CHECK(input && replacement && table && case_sensitive && whole_word && regex &&
        selection_only && status && xuiCheckBoxGetChecked(case_sensitive) &&
        !xuiCheckBoxGetChecked(whole_word) && !xuiCheckBoxGetChecked(regex) &&
        !xuiCheckBoxGetChecked(selection_only));
    CHECK(xuiInputSetText(input, "alpha") == XUI_OK);
    document_find_click(context, document_find_button(context, window, XUI_TR_FIND_ALL));
    CHECK(xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK);
    CHECK(count == 2);
    CHECK(xuiTableViewGetRowCount(table) == 2);
    CHECK(xuiInputSetText(input, "ALPHA") == XUI_OK);
    document_find_click(context, document_find_button(context, window, XUI_TR_FIND_ALL));
    CHECK(xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 0);
    document_find_click(context, case_sensitive);
    CHECK(!xuiCheckBoxGetChecked(case_sensitive) &&
        xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 2);
    document_find_click(context, regex);
    CHECK(xuiCheckBoxGetChecked(regex) && xuiInputSetText(input, "AL.HA") == XUI_OK);
    document_find_click(context, document_find_button(context, window, XUI_TR_FIND_ALL));
    CHECK(xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 2);
    CHECK(xuiInputSetText(input, "[") == XUI_OK);
    document_find_click(context, document_find_button(context, window, XUI_TR_FIND_ALL));
    CHECK(xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 0 &&
        xuiTableViewGetRowCount(table) == 0 &&
        !strcmp(xuiLabelGetText(status), xuiTranslate(context, XUI_TR_FIND_INVALID_PATTERN)));
    CHECK(xuiSetLanguage(context, XUI_LANGUAGE_ZH) == XUI_OK &&
        xuiUpdate(context, .016f) == XUI_OK &&
        !strcmp(xuiLabelGetText(status), xuiTranslate(context, XUI_TR_FIND_INVALID_PATTERN)) &&
        xuiSetLanguage(context, XUI_LANGUAGE_EN) == XUI_OK &&
        xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiInputSetText(input, "alpha") == XUI_OK);
    document_find_click(context, regex);
    document_find_click(context, case_sensitive);
    CHECK(xuiCheckBoxGetChecked(case_sensitive) && !xuiCheckBoxGetChecked(regex) &&
        xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 2);
    CHECK(xuiInputSetText(input, "ha") == XUI_OK);
    document_find_click(context, document_find_button(context, window, XUI_TR_FIND_ALL));
    CHECK(xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 2);
    document_find_click(context, whole_word);
    CHECK(xuiCheckBoxGetChecked(whole_word) &&
        xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 0);
    document_find_click(context, whole_word);
    CHECK(xuiInputSetText(input, "alpha") == XUI_OK);
    document_find_click(context, document_find_button(context, window, XUI_TR_FIND_ALL));
    CHECK(xuiUpdate(context, .016f) == XUI_OK && xuiLayout(context) == XUI_OK &&
        xuiRender(context, target, &damage, 1) == XUI_OK);
    CHECK(xuiSetLanguage(context, XUI_LANGUAGE_ZH) == XUI_OK &&
        xuiUpdate(context, .016f) == XUI_OK &&
        !strcmp(xuiWindowGetTitle(window), xuiTranslate(context, XUI_TR_FIND_TITLE)) &&
        xuiSetLanguage(context, XUI_LANGUAGE_EN) == XUI_OK &&
        xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiDocumentViewActivateFindResult(editor, 1, &match) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        selection.tAnchor.iOffset == match.tAnchor.iOffset &&
        selection.tCaret.iOffset == match.tCaret.iOffset);
    CHECK(xuiDocumentEditorOpenReplace(editor) == XUI_OK &&
        xuiDocumentEditorGetFindWindow(editor) == window &&
        xuiInputSetText(replacement, "Z") == XUI_OK);
    document_find_click(context, selection_only);
    CHECK(xuiCheckBoxGetChecked(selection_only) &&
        xuiDocumentViewGetFindScope(editor, &selection) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 1);
    document_find_click(context, document_find_button(context, window, XUI_TR_REPLACE_ALL));
    plain(rich, "alpha beta Z\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 1);
    plain(rich, "alpha beta alpha\n");
    document_find_click(context, selection_only);
    CHECK(!xuiCheckBoxGetChecked(selection_only) &&
        xuiDocumentViewGetFindScope(editor, &selection) == XUI_ERROR_NOT_FOUND &&
        xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 2 &&
        xuiDocumentViewActivateFindResult(editor, 1, &match) == XUI_OK);
    document_find_click(context, document_find_button(context, window, XUI_TR_REPLACE_CURRENT));
    plain(rich, "alpha beta Z\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    plain(rich, "alpha beta alpha\n");
    CHECK(xuiInputSetText(replacement, "Q") == XUI_OK);
    document_find_click(context, document_find_button(context, window, XUI_TR_REPLACE_ALL));
    plain(rich, "Q beta Q\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    plain(rich, "alpha beta alpha\n");
    document_find_click(context, case_sensitive);
    document_find_click(context, regex);
    CHECK(xuiInputSetText(input, "AL.HA") == XUI_OK &&
        xuiInputSetText(replacement, "Y") == XUI_OK);
    document_find_click(context, document_find_button(context, window, XUI_TR_REPLACE_ALL));
    plain(rich, "Y beta Y\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    plain(rich, "alpha beta alpha\n");
    CHECK(xuiInputSetText(input, "(AL.HA)") == XUI_OK &&
        xuiInputSetText(replacement, "$1!") == XUI_OK);
    document_find_click(context, document_find_button(context, window, XUI_TR_REPLACE_ALL));
    plain(rich, "alpha! beta alpha!\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    plain(rich, "alpha beta alpha\n");
    CHECK(xuiInputSetText(input, "alpha") == XUI_OK);
    document_find_click(context, regex);
    document_find_click(context, case_sensitive);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK &&
        xuiDocumentViewActivateFindResult(editor, 0, NULL) == XUI_OK &&
        xuiDocumentEditorReplaceCurrent(editor, "alpha", 5, "X", 1) == XUI_ERROR_INVALID_STATE);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    CHECK(!xuiWidgetGetEnabled(document_find_button(context, window, XUI_TR_REPLACE_CURRENT)));
    CHECK(!xuiWidgetGetEnabled(document_find_button(context, window, XUI_TR_REPLACE_ALL)));
    document_find_click(context, document_find_button(context, window, XUI_TR_REPLACE_CURRENT));
    plain(rich, "alpha beta alpha\n");
    xuiWidgetDestroy(window);
    CHECK(xuiDocumentEditorGetFindWindow(editor) == NULL &&
        xuiDocumentEditorOpenFind(editor) == XUI_OK);
    window = xuiDocumentEditorGetFindWindow(editor);
    CHECK(window && xuiWindowIsOpen(window));
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK &&
        xuiSetFocusWidget(context, editor) == XUI_OK);
    event.iSize = sizeof(event); event.iType = XUI_EVENT_KEY_DOWN;
    event.pTarget = editor; event.iModifiers = XUI_MOD_CTRL; event.iKey = 'f';
    CHECK(xuiDispatchEvent(context, &event) >= XUI_OK &&
        xuiDocumentEditorGetFindWindow(editor) == window && xuiWindowIsOpen(window));
    xuiWidgetDestroy(editor); editor = NULL;
    CHECK(xuiDocumentCreate(&(xui_doc_desc_t){.iSize=sizeof(xui_doc_desc_t),
        .iProfile=XUI_DOCUMENT_MARKDOWN}, &markdown) == XUI_OK &&
        xuiDocumentLoadMarkdown(markdown, "alpha **alpha**\n", 16) == XUI_OK);
    desc.tView.pDocument = markdown;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 700, 420}) == XUI_OK &&
        xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
        xuiDocumentEditorOpenFind(editor) == XUI_OK);
    window = xuiDocumentEditorGetFindWindow(editor);
    input = document_find_child(window, xuiInputGetType(context), 0);
    CHECK(input && xuiInputSetText(input, "alpha") == XUI_OK);
    document_find_click(context, document_find_button(context, window, XUI_TR_FIND_ALL));
    CHECK(xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 2 &&
        xuiDocumentViewActivateFindResult(editor, 1, NULL) == XUI_OK &&
        xuiDocumentEditorReplaceCurrent(editor, "alpha", 5, "Z", 1) == XUI_OK);
    source(markdown, "alpha **Z**\n");
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    source(markdown, "alpha **alpha**\n");
    CHECK(xuiDocumentViewActivateFindResult(editor, 1, NULL) == XUI_OK &&
        xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentViewSetFindScope(editor, &selection) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 1 &&
        xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK &&
        xuiDocumentViewGetFindScope(editor, &selection) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 1 &&
        xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 1 &&
        xuiDocumentViewSetFindScope(editor, NULL) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 2);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK &&
        xuiUpdate(context, .016f) == XUI_OK &&
        xuiDocumentViewGetFindResultCount(editor, &count) == XUI_OK && count == 2);
    xuiWidgetDestroy(editor); editor = NULL;
    desc.tView.pDocument = rich;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 700, 420}) == XUI_OK &&
        xuiDocumentEditorOpenReplace(editor) == XUI_OK);
    window = xuiDocumentEditorGetFindWindow(editor);
    input = document_find_child(window, xuiInputGetType(context), 0);
    replacement = document_find_child(window, xuiInputGetType(context), 1);
    CHECK(input && replacement && xuiInputSetText(input, "alpha") == XUI_OK &&
        xuiInputSetText(replacement, "R") == XUI_OK);
    observer.editor = editor;
    CHECK(xuiDocumentSubscribe(rich, document_menu_destroy_on_change,
        &observer, &token) == XUI_OK);
    document_find_click(context, document_find_button(context, window, XUI_TR_REPLACE_ALL));
    CHECK(observer.calls == 1);
    plain(rich, "R beta R\n");
    xuiDocumentUnsubscribe(rich, token);
    xuiDocumentRelease(markdown); xuiDocumentRelease(rich);
    puts("DocumentEditor Find UI: list, regex/case/word/selection options, invalid pattern, replace/Undo, read-only, shortcut, Markdown SOURCE/LIVE and observer teardown passed");
}
static void cell_alignment_command_state_cases(xui_context context)
{
    xui_document document = NULL;
    xui_document_transaction txn = NULL;
    xui_widget editor = NULL;
    xui_doc_node_desc_t node = {0};
    xui_doc_editor_desc_t desc = {0};
    xui_doc_command_state_t state = {0};
    xui_doc_range_t selection = {0};
    xui_document_snapshot snapshot = NULL;
    xui_doc_node_info_t info = {0};
    uint64_t table, row, cell, paragraph, text_id;

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    node.iSize = sizeof(node);
    node.iKind = XUI_DOC_TABLE;
    CHECK(xuiDocumentTxnInsertNode(txn, 1, UINT64_MAX, &node, &table) == XUI_OK);
    node.iKind = XUI_DOC_ROW;
    CHECK(xuiDocumentTxnInsertNode(txn, table, UINT64_MAX, &node, &row) == XUI_OK);
    node.iKind = XUI_DOC_CELL;
    node.tAttributes.iAlignment = 2;
    CHECK(xuiDocumentTxnInsertNode(txn, row, UINT64_MAX, &node, &cell) == XUI_OK);
    node.iKind = XUI_DOC_PARAGRAPH;
    node.tAttributes.iAlignment = 0;
    CHECK(xuiDocumentTxnInsertNode(txn, cell, UINT64_MAX, &node, &paragraph) == XUI_OK);
    node.iKind = XUI_DOC_TEXT;
    node.sText = "right";
    node.iTextBytes = 5;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, UINT64_MAX, &node, &text_id) == XUI_OK &&
        xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);

    desc.iSize = sizeof(desc);
    desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = document;
    desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 200}) == XUI_OK);
    selection.tAnchor.iSize = sizeof(selection.tAnchor);
    selection.tAnchor.iDocumentId = xuiDocumentGetIdentity(document);
    selection.tAnchor.iRevision = xuiDocumentGetRevision(document);
    selection.tAnchor.iNodeId = text_id;
    selection.tAnchor.iKind = XUI_DOC_POSITION_TEXT;
    selection.tCaret = selection.tAnchor;
    state.iSize = sizeof(state);
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK &&
        xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_ALIGN_RIGHT, &state) == XUI_OK &&
        state.bEnabled && state.bActive &&
        xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_ALIGN_LEFT, &state) == XUI_OK &&
        !state.bActive);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ALIGN_LEFT) == XUI_OK &&
        xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_ALIGN_LEFT, &state) == XUI_OK &&
        state.bActive);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
        xuiDocumentSnapshotGetNode(snapshot, paragraph, &info) == XUI_OK &&
        (info.tAttributes.iFlags & XUI_DOC_ALIGNMENT_EXPLICIT_LEFT));
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK &&
        xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_ALIGN_RIGHT, &state) == XUI_OK &&
        state.bActive);
    xuiWidgetDestroy(editor);
    xuiDocumentRelease(document);
    puts("Editor alignment commands follow inherited Cell alignment and explicit left override");
}

#include "xui_document_grapheme_editor_cases.h"
#include "xui_document_ligature_editor_cases.h"
#include "xui_document_grapheme_font_editor_cases.h"
#include "xui_document_nested_prefix_editor_cases.h"
#include "xui_document_list_split_source_editor_cases.h"
#include "xui_document_code_language_source_editor_cases.h"
#include "xui_document_heading_source_editor_cases.h"
#include "xui_document_language_editor_cases.h"
int main(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font; xui_surface target; xui_widget editor, view;
    xui_doc_editor_desc_t ed = {0}; xui_doc_view_desc_t vd = {0}; xui_document d; xui_doc_range_t selection, before;
    xui_rect_i_t damage = {0, 0, 640, 480}; xui_doc_stats_t stats = {0}; uint64_t revision, count;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK); CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK); CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&proxy, &target, 640, 480, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    document_toolbar_cases(context);
    document_cross_node_grapheme_editor(context, &proxy);
    document_unicode_control_editor(context, &proxy);
    document_trailing_line_editor(context, &proxy);
    document_blank_row_editor(context, &proxy);
    document_soft_wrap_editor(context, &proxy);
    document_format_editor(context, &proxy);
    document_ligature_editor(&proxy);
    document_grapheme_font_editor(&proxy);
    document_nested_prefix_editor(context);
    document_list_split_source_editor(context);
    document_code_language_source_editor(context);
    document_heading_source_editor(context);
    document_structural_toolbar_cases(context);
    document_task_marker_editor_cases(context);
    document_vertical_goal_cases(context);
    document_cross_block_source_editor_cases(context);
    list_create_editor_cases(context);
    list_restyle_editor_cases(context);
    empty_list_unlist_editor_cases(context);
    cross_list_restyle_editor_cases(context);
    list_parent_gap_editor_cases(context);
    move_block_editor_cases(context);
    document_menu_cases(context);
    document_find_ui_cases(context, target);
    image_editor_cases(context); object_selection_cases(context, target, &proxy);
    clear_formatting_editor_cases(context);
    clear_formatting_editor_failures(context); table_width_editor_cases(context);
    table_insert_editor_cases(context);
    cross_list_editor_cases(context);
    table_column_keyboard_cases(context);
    table_navigation_editor_cases(context); table_structure_editor_cases(context);
    table_matrix_editor_cases(context, &proxy);
    native_fragment_editor_cases(context, &proxy);
    native_fragment_image_view_cases(context, &proxy);
    native_fragment_callback_case();
    table_rectangle_copy_cases(context, &proxy, target);
    table_rectangle_enter_cases(context);
    table_rectangle_merge_cases(context);
    table_programmatic_selection_cases(context);
    table_keyboard_selection_cases(context);
    table_rectangle_cut_callback_case();
    embedded_editor_cases(context, target);
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
        {
            xui_document_snapshot styled_snapshot;
            xui_doc_range_t styled_match;
            uint32_t common_marks, mixed_marks;
            CHECK(xuiEditSetText(editor, "Alpha alpha") == XUI_OK);
            CHECK(xuiEditSetSelection(editor, 0, 5) == XUI_OK);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_BOLD) == XUI_OK);
            CHECK(xuiEditSetSelection(editor, 0, 0) == XUI_OK);
            CHECK(xuiDocumentViewFindEx(editor, "alpha", 5,
                XUI_DOC_FIND_IGNORE_CASE, 0, 0, &styled_match) == XUI_OK);
            CHECK(xuiDocumentEditorReplaceCurrentEx(editor, "alpha", 5,
                "ONE", 3, XUI_DOC_FIND_IGNORE_CASE) == XUI_OK);
            plain(d, "ONE alpha\n");
            CHECK(xuiDocumentAcquireSnapshot(d, &styled_snapshot) == XUI_OK);
            CHECK(xuiDocumentSnapshotFind(styled_snapshot, XUI_DOC_SEMANTIC,
                "ONE", 3, NULL, &styled_match, 1, &replaced) == XUI_OK &&
                replaced == 1);
            CHECK(xuiDocumentSnapshotQueryMarks(styled_snapshot, &styled_match,
                &common_marks, &mixed_marks) == XUI_OK &&
                common_marks == XUI_DOC_BOLD && !mixed_marks);
            xuiDocumentSnapshotRelease(styled_snapshot);
            CHECK(xuiEditUndo(editor) == XUI_OK);
            plain(d, "Alpha alpha\n");
        }
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
        CHECK(xuiDocumentViewSetFindQueryEx(editor, "cd", 2,
            XUI_DOC_FIND_IGNORE_CASE) == XUI_OK);
        CHECK(xuiDocumentViewGetFindResultCount(editor, &replaced) == XUI_OK &&
            replaced == 2);
        CHECK(xuiEditSetSelection(editor, 0, 0) == XUI_OK);
        CHECK(xuiDocumentViewFindEx(editor, "a.cD", 4,
            XUI_DOC_FIND_REGEX | XUI_DOC_FIND_IGNORE_CASE,
            0, 0, &match) == XUI_OK);
        CHECK(xuiEditGetSelection(editor, &start, &end) == XUI_OK &&
            start == 0 && end == 4);
        CHECK(xuiDocumentEditorReplaceCurrentEx(editor, "a.cD", 4, "Q", 1,
            XUI_DOC_FIND_REGEX | XUI_DOC_FIND_IGNORE_CASE) == XUI_OK);
        plain(d, "Q\nabCD\n");
        CHECK(xuiEditUndo(editor) == XUI_OK); plain(d, "abCD\nabCD\n");
        CHECK(xuiDocumentEditorReplaceAllEx(editor, "a.cD", 4, "Q", 1,
            XUI_DOC_FIND_REGEX | XUI_DOC_FIND_IGNORE_CASE,
            NULL, &replaced) == XUI_OK && replaced == 2);
        plain(d, "Q\nQ\n");
        CHECK(xuiEditUndo(editor) == XUI_OK); plain(d, "abCD\nabCD\n");
        CHECK(xuiEditSetSelection(editor, 0, 0) == XUI_OK);
        CHECK(xuiDocumentViewFindEx(editor, "([a-z]+)([A-Z]+)", 16,
            XUI_DOC_FIND_REGEX, 0, 0, &match) == XUI_OK);
        CHECK(xuiDocumentEditorReplaceCurrentEx(editor,
            "([a-z]+)([A-Z]+)", 16, "$2-$1", 5,
            XUI_DOC_FIND_REGEX | XUI_DOC_REPLACE_EXPAND) == XUI_OK);
        plain(d, "CD-ab\nabCD\n");
        CHECK(xuiEditUndo(editor) == XUI_OK); plain(d, "abCD\nabCD\n");
        CHECK(xuiEditSetSelection(editor, 0, 0) == XUI_OK &&
            xuiDocumentViewFindEx(editor, "([a-z]+)([A-Z]+)", 16,
                XUI_DOC_FIND_REGEX, 0, 0, &match) == XUI_OK &&
            xuiDocumentEditorReplaceCurrentEx(editor,
                "([a-z]+)([A-Z]+)", 16, "$9", 2,
                XUI_DOC_FIND_REGEX | XUI_DOC_REPLACE_EXPAND) == XUI_ERROR_INVALID_ARGUMENT);
        plain(d, "abCD\nabCD\n");
        CHECK(xuiEditSelectAll(view) == XUI_OK); CHECK(xuiEditCopy(view) == XUI_OK && !strcmp(proxy.sClipboard, "abCD\nabCD\n"));
        CHECK(xuiEditIsReadonly(view));
        CHECK(xuiWidgetGetAccessibleNode(editor, 0, &accessible) == XUI_OK && accessible.iRole == XUI_ACCESSIBLE_ROLE_DOCUMENT);
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
    {
        const char* original = "> - **ab**cd\n>   - nested\n\nTAIL\n";
        xui_doc_desc_t md = {0}; xui_doc_range_t match;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK); CHECK(xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiDocumentViewFind(editor, "abcd", 4, 0, 0, &match) == XUI_OK);
        CHECK(xuiDocumentEditorInsertText(editor, "changed", 7) == XUI_OK); plain(d, "changed\nnested\nTAIL\n");
        CHECK(xuiDocumentEditorInsertText(editor, "!*", 2) == XUI_OK);
        CHECK(xuiDocumentEditorInsertText(editor, "X", 1) == XUI_OK); plain(d, "changed!*X\nnested\nTAIL\n");
        CHECK(xuiEditUndo(editor) == XUI_OK); CHECK(xuiEditUndo(editor) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ENTER) == XUI_OK);
        CHECK(xuiDocumentEditorInsertText(editor, "second", 6) == XUI_OK); plain(d, "changed\nsecond\nnested\nTAIL\n");
        CHECK(xuiUpdate(context, .016f) == XUI_OK); CHECK(xuiRender(context, target, &damage, 1) == XUI_OK);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
        CHECK(xuiEditUndo(editor) == XUI_OK); CHECK(xuiEditUndo(editor) == XUI_OK); CHECK(xuiEditUndo(editor) == XUI_OK);
        source(d, original); xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        const char* original = "- abcd\n- tail\n";
        xui_doc_desc_t md = {0}; xui_document_snapshot snap; xui_doc_node_info_t node = {0};
        xui_doc_range_t match; uint64_t list;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiDocumentViewFind(editor, "abcd", 4, 0, 0, &match) == XUI_OK);
        match.tAnchor = match.tCaret; match.tAnchor.iOffset = match.tCaret.iOffset = 2;
        CHECK(xuiDocumentViewSetSelection(editor, &match) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ENTER) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK && xuiDocumentSnapshotGetChild(snap, 1, 0, &list) == XUI_OK);
        node.iSize = sizeof(node); CHECK(xuiDocumentSnapshotGetNode(snap, list, &node) == XUI_OK && node.iKind == XUI_DOC_LIST && node.iChildCount == 3);
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "abcd", 4, NULL, &match, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snap);
        match.tAnchor = match.tCaret; match.tAnchor.iOffset = match.tCaret.iOffset = 4;
        CHECK(xuiDocumentViewSetSelection(editor, &match) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ENTER) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ENTER) == XUI_OK);
        CHECK(xuiDocumentEditorInsertText(editor, "between", 7) == XUI_OK); plain(d, "abcd\nbetween\ntail\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        const char* original = "- only\n- \n";
        xui_doc_desc_t md = {0}; xui_document_snapshot snap; xui_doc_node_info_t node = {0};
        uint64_t list, empty;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK && xuiDocumentSnapshotGetChild(snap, 1, 0, &list) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, list, 1, &empty) == XUI_OK); xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
        selection.tAnchor.iNodeId = selection.tCaret.iNodeId = empty;
        selection.tAnchor.iKind = selection.tCaret.iKind = XUI_DOC_POSITION_GAP;
        selection.tAnchor.iOffset = selection.tCaret.iOffset = 0;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ENTER) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK && xuiDocumentSnapshotGetChild(snap, 1, 0, &list) == XUI_OK);
        node.iSize = sizeof(node); CHECK(xuiDocumentSnapshotGetNode(snap, list, &node) == XUI_OK && node.iKind == XUI_DOC_LIST && node.iChildCount == 1);
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorInsertText(editor, "after", 5) == XUI_OK); plain(d, "only\nafter\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        const char* original = "- A\n- B\n- C\n";
        xui_doc_desc_t md = {0}; xui_doc_range_t match; xui_event_t key = {0};
        xui_doc_command_state_t state = {0}; xui_document_snapshot snap; xui_doc_node_info_t info = {0};
        uint64_t list, item, nested;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiSetFocusWidget(context, editor) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "B", 1, NULL, &match, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snap);
        match.tAnchor = match.tCaret; CHECK(xuiDocumentViewSetSelection(editor, &match) == XUI_OK);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INDENT_LIST, &state) == XUI_OK && state.bEnabled);
        key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = editor; key.iKey = XUI_KEY_TAB;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, 1, 0, &list) == XUI_OK);
        info.iSize = sizeof(info); CHECK(xuiDocumentSnapshotGetNode(snap, list, &info) == XUI_OK && info.iChildCount == 2);
        CHECK(xuiDocumentSnapshotGetChild(snap, list, 0, &item) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, item, 1, &nested) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snap, nested, &info) == XUI_OK && info.iKind == XUI_DOC_LIST && info.iChildCount == 1);
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_OUTDENT_LIST, &state) == XUI_OK && state.bEnabled);
        key.iModifiers = XUI_MOD_SHIFT; CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, 1, 0, &list) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snap, list, &info) == XUI_OK && info.iChildCount == 3);
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "B", 1, NULL, &match, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snap);
        match.tAnchor = match.tCaret; CHECK(xuiDocumentViewSetSelection(editor, &match) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_OUTDENT_LIST, &state) == XUI_OK && state.bEnabled);
        key.iModifiers = XUI_MOD_SHIFT; CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetNode(snap, 1, &info) == XUI_OK && info.iChildCount == 3);
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
        {
            xui_doc_range_t b_match, c_match, before_range; int direction;
            CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
            CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "B", 1, NULL, &b_match, 1, &count) == XUI_OK && count == 1);
            CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "C", 1, NULL, &c_match, 1, &count) == XUI_OK && count == 1);
            xuiDocumentSnapshotRelease(snap);
            match.tAnchor = c_match.tCaret; match.tCaret = b_match.tAnchor;
            CHECK(xuiDocumentViewSetSelection(editor, &match) == XUI_OK);
            before_range = match;
            CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INDENT_LIST, &state) == XUI_OK && state.bEnabled);
            key.iModifiers = 0; CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetChild(snap, 1, 0, &list) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetNode(snap, list, &info) == XUI_OK && info.iChildCount == 1);
            CHECK(xuiDocumentSnapshotGetChild(snap, list, 0, &item) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetChild(snap, item, 1, &nested) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetNode(snap, nested, &info) == XUI_OK && info.iChildCount == 2);
            CHECK(xuiDocumentViewGetSelection(editor, &match) == XUI_OK);
            CHECK(xuiDocumentSnapshotComparePositions(snap, &match.tAnchor, &match.tCaret, &direction) == XUI_OK && direction > 0);
            xuiDocumentSnapshotRelease(snap);
            CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_OUTDENT_LIST, &state) == XUI_OK && state.bEnabled);
            key.iModifiers = XUI_MOD_SHIFT; CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetChild(snap, 1, 0, &list) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetNode(snap, list, &info) == XUI_OK && info.iChildCount == 3);
            xuiDocumentSnapshotRelease(snap);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
            CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
            CHECK(xuiDocumentViewGetSelection(editor, &match) == XUI_OK);
            CHECK(match.tAnchor.iNodeId == before_range.tAnchor.iNodeId && match.tAnchor.iOffset == before_range.tAnchor.iOffset);
            CHECK(match.tCaret.iNodeId == before_range.tCaret.iNodeId && match.tCaret.iOffset == before_range.tCaret.iOffset);
        }
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_INDENT_LIST, &state) == XUI_OK && !state.bEnabled);
        revision = xuiDocumentGetRevision(d); key.iModifiers = 0;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK && xuiDocumentGetRevision(d) == revision);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        const char* original = "Intro\n\n## Middle\n\nTail\n";
        xui_doc_desc_t md = {0}; xui_document_snapshot snap; xui_doc_range_t match, first, last;
        xui_doc_command_state_t state = {0}; xui_doc_node_info_t node = {0}; uint64_t block, i;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "Middle", 6, NULL, &match, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snap);
        match.tAnchor = match.tCaret; CHECK(xuiDocumentViewSetSelection(editor, &match) == XUI_OK);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_HEADING_2, &state) == XUI_OK && state.bEnabled && state.bActive);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_HEADING_3) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, 1, 1, &block) == XUI_OK);
        node.iSize = sizeof(node); CHECK(xuiDocumentSnapshotGetNode(snap, block, &node) == XUI_OK && node.iKind == XUI_DOC_HEADING && node.tAttributes.iHeadingLevel == 3);
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "Intro", 5, NULL, &first, 1, &count) == XUI_OK && count == 1);
        CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "Tail", 4, NULL, &last, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snap);
        match.tAnchor = last.tCaret; match.tCaret = first.tAnchor;
        CHECK(xuiDocumentViewSetSelection(editor, &match) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_HEADING_2, &state) == XUI_OK && state.bEnabled && !state.bActive && state.bMixed);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_HEADING_1) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_HEADING_1, &state) == XUI_OK && state.bActive && !state.bMixed);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_PARAGRAPH) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        for (i = 0; i < 3; i++) {
            CHECK(xuiDocumentSnapshotGetChild(snap, 1, i, &block) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetNode(snap, block, &node) == XUI_OK && node.iKind == XUI_DOC_PARAGRAPH);
        }
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_SELECT_ALL) == XUI_OK);
        CHECK(xuiDocumentViewGetSelection(editor, &match) == XUI_OK &&
            match.tAnchor.iKind == XUI_DOC_POSITION_GAP && match.tAnchor.iNodeId == 1 && match.tAnchor.iOffset == 0 &&
            match.tCaret.iKind == XUI_DOC_POSITION_GAP && match.tCaret.iNodeId == 1 && match.tCaret.iOffset == 3);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_HEADING_4, &state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_HEADING_4) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        for (i = 0; i < 3; i++) {
            CHECK(xuiDocumentSnapshotGetChild(snap, 1, i, &block) == XUI_OK);
            CHECK(xuiDocumentSnapshotGetNode(snap, block, &node) == XUI_OK &&
                node.iKind == XUI_DOC_HEADING && node.tAttributes.iHeadingLevel == 4);
        }
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, original);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_HEADING_1, &state) == XUI_OK && !state.bEnabled);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        const char* original = "> [!nOtE]\r\n> **alpha**\r\n>\r\n> beta\r\n";
        const char* expected = "> [!nOtE]\r\n> ## **alpha**\r\n>\r\n> beta\r\n";
        xui_doc_desc_t md = {0}; xui_document_snapshot snap;
        xui_doc_range_t match;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK &&
            xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC,
                "alpha", 5, NULL, &match, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snap);
        match.tCaret = match.tAnchor;
        CHECK(xuiDocumentViewSetSelection(editor, &match) == XUI_OK &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_HEADING_2) == XUI_OK);
        source(d, expected);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(d, original);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(d, expected);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        const char* original = "> [!nOtE]\r\n> alpha\r\n>\r\n> beta\r\n";
        const char* expected = "> [!nOtE]\r\n> ## alpha\r\n>\r\n> ## beta\r\n";
        xui_doc_desc_t md = {0}; xui_document_snapshot snap;
        xui_doc_range_t first, last, selected;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK &&
            xuiDocumentLoadMarkdown(d, original, strlen(original)) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK &&
            xuiSetRootWidget(context, editor) == XUI_OK &&
            xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK &&
            xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC,
                "alpha", 5, NULL, &first, 1, &count) == XUI_OK && count == 1 &&
            xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC,
                "beta", 4, NULL, &last, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snap);
        selected.tAnchor = first.tAnchor; selected.tCaret = last.tCaret;
        CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_HEADING_2) == XUI_OK);
        source(d, expected);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        source(d, original);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK &&
            xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_REDO) == XUI_OK);
        source(d, expected);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        xui_doc_desc_t md = {0}; xui_doc_command_state_t state = {0};
        xui_doc_paragraph_spacing_state_t spacing = {0};
        xui_document_snapshot snap; xui_doc_node_info_t node = {0}; uint64_t block;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "", 0) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_HEADING_2, &state) == XUI_OK && state.bEnabled && !state.bActive);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_HEADING_2) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snap, 1, 0, &block) == XUI_OK);
        node.iSize = sizeof(node);
        CHECK(xuiDocumentSnapshotGetNode(snap, block, &node) == XUI_OK &&
            node.iKind == XUI_DOC_HEADING && node.tAttributes.iHeadingLevel == 2);
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, "");
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_ALIGN_CENTER, &state) == XUI_OK &&
            !state.bEnabled && state.iDisabledReason == XUI_DOC_ERROR_UNREPRESENTABLE);
        spacing.iSize = sizeof(spacing);
        CHECK(xuiDocumentEditorQueryParagraphSpacing(editor, &spacing) == XUI_OK && !spacing.bEnabled);
        CHECK(xuiDocumentEditorSetParagraphSpacing(editor, 0, 0) == XUI_DOC_ERROR_UNREPRESENTABLE);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        xui_doc_command_state_t state = {0}; xui_doc_paragraph_spacing_state_t spacing = {0};
        xui_doc_range_t selected;
        xui_document_snapshot snap; xui_doc_node_info_t node = {0}; uint64_t blocks[2], text, i;
        xui_document_transaction t; xui_doc_node_desc_t desc = {0};
        CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK && xuiDocumentBeginTransaction(d, NULL, &t) == XUI_OK);
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_PARAGRAPH;
        for (i = 0; i < 2; i++) {
            CHECK(xuiDocumentTxnInsertNode(t, 1, UINT64_MAX, &desc, &blocks[i]) == XUI_OK);
            desc.iKind = XUI_DOC_TEXT; desc.sText = i ? "Beta" : "Alpha"; desc.iTextBytes = i ? 4 : 5;
            CHECK(xuiDocumentTxnInsertNode(t, blocks[i], UINT64_MAX, &desc, &text) == XUI_OK);
            desc.iKind = XUI_DOC_PARAGRAPH; desc.sText = NULL; desc.iTextBytes = 0;
        }
        CHECK(xuiDocumentTxnCommit(t, NULL) == XUI_OK); xuiDocumentTxnRelease(t);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_SELECT_ALL) == XUI_OK);
        state.iSize = sizeof(state); spacing.iSize = sizeof(spacing);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_ALIGN_LEFT, &state) == XUI_OK && state.bEnabled && state.bActive);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ALIGN_CENTER) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_ALIGN_CENTER, &state) == XUI_OK && state.bActive);
        CHECK(xuiDocumentEditorQueryParagraphSpacing(editor, &spacing) == XUI_OK && spacing.bEnabled && spacing.bUseDefault);
        CHECK(xuiDocumentEditorSetParagraphSpacing(editor, 0, 0) == XUI_OK);
        CHECK(xuiDocumentEditorQueryParagraphSpacing(editor, &spacing) == XUI_OK &&
            spacing.bEnabled && !spacing.bUseDefault && !spacing.bMixed && spacing.fValue == 0);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        node.iSize = sizeof(node);
        for (i = 0; i < 2; i++) {
            CHECK(xuiDocumentSnapshotGetNode(snap, blocks[i], &node) == XUI_OK &&
                node.tAttributes.iAlignment == 1 && (node.tAttributes.iFlags & XUI_DOC_SPACING_EXPLICIT));
        }
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorQueryParagraphSpacing(editor, &spacing) == XUI_OK && spacing.bUseDefault);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_ALIGN_LEFT, &state) == XUI_OK && state.bActive);
        CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK);
        selected.tAnchor = selected.tCaret;
        selected.tAnchor.iNodeId = selected.tCaret.iNodeId = blocks[0];
        selected.tAnchor.iKind = selected.tCaret.iKind = XUI_DOC_POSITION_GAP;
        selected.tAnchor.iOffset = selected.tCaret.iOffset = 0;
        selected.tAnchor.iRevision = selected.tCaret.iRevision = xuiDocumentGetRevision(d);
        CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_ALIGN_RIGHT) == XUI_OK);
        CHECK(xuiDocumentEditorSetParagraphSpacing(editor, 7, 0) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_SELECT_ALL) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_ALIGN_RIGHT, &state) == XUI_OK && state.bMixed);
        CHECK(xuiDocumentEditorQueryParagraphSpacing(editor, &spacing) == XUI_OK && spacing.bMixed);
        CHECK(xuiDocumentEditorSetParagraphSpacing(editor, 0, 1) == XUI_OK);
        CHECK(xuiDocumentEditorQueryParagraphSpacing(editor, &spacing) == XUI_OK &&
            spacing.bEnabled && !spacing.bMixed && spacing.bUseDefault);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorQueryParagraphSpacing(editor, &spacing) == XUI_OK && spacing.bMixed);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        xui_document_snapshot snap; xui_doc_range_t selected; xui_doc_command_state_t state = {0};
        uint64_t found; uint32_t common, mixed;
        CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiDocumentEditorInsertText(editor, "plain", 5) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "plain", 5, NULL, &selected, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snap); CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_HIGHLIGHT) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_SUBSCRIPT) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_SUPERSCRIPT) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_SUPERSCRIPT, &state) == XUI_OK && state.bActive);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_SUBSCRIPT, &state) == XUI_OK && !state.bActive);
        CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotQueryMarks(snap, &selected, &common, &mixed) == XUI_OK &&
            (common & (XUI_DOC_HIGHLIGHT | XUI_DOC_SUPERSCRIPT)) == (XUI_DOC_HIGHLIGHT | XUI_DOC_SUPERSCRIPT) &&
            !(common & XUI_DOC_SUBSCRIPT) && !mixed);
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_SUBSCRIPT, &state) == XUI_OK && state.bActive);
        CHECK(xuiDocumentEditorSetMarks(editor, XUI_DOC_LINK, 0) == XUI_ERROR_UNSUPPORTED);
        CHECK(xuiDocumentEditorSetLink(editor, "/rich", "R") == XUI_OK);
        CHECK(xuiDocumentViewGetSelection(editor, &selected) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotQueryMarks(snap, &selected, &common, &mixed) == XUI_OK && (common & XUI_DOC_LINK));
        xuiDocumentSnapshotRelease(snap);
        CHECK(xuiDocumentEditorSetLink(editor, NULL, NULL) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorSetLink(editor, NULL, NULL) == XUI_OK);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        xui_doc_desc_t md = {0}; xui_doc_command_state_t state = {0};
        xui_document_snapshot snap; xui_doc_range_t selected; uint64_t count;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN; md.iMarkdownDialect = XUI_MD_EXTENDED;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "plain\n", 6) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "plain", 5, NULL, &selected, 1, &count) == XUI_OK && count == 1);
        xuiDocumentSnapshotRelease(snap); CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_HIGHLIGHT, &state) == XUI_OK && state.bEnabled && !state.bActive);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_HIGHLIGHT) == XUI_OK); source(d, "==plain==\n");
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_HIGHLIGHT, &state) == XUI_OK && state.bActive);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, "plain\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_SUBSCRIPT) == XUI_OK); source(d, "~plain~\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_SUPERSCRIPT) == XUI_OK); source(d, "^plain^\n");
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_SUPERSCRIPT, &state) == XUI_OK && state.bActive);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, "~plain~\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, "plain\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_CODE) == XUI_OK); source(d, "`plain`\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, "plain\n");
        CHECK(xuiDocumentEditorSetLink(editor, "/uri", "T") == XUI_OK);
        CHECK(xuiDocumentEditorSetLink(editor, NULL, NULL) == XUI_OK); source(d, "plain\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, "plain\n");
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        xui_doc_desc_t md = {0}; xui_doc_command_state_t state = {0};
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN; md.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "plain\n", 6) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_HIGHLIGHT, &state) == XUI_OK && !state.bEnabled);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_SUBSCRIPT, &state) == XUI_OK && !state.bEnabled);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_SUPERSCRIPT, &state) == XUI_OK && !state.bEnabled);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_CODE, &state) == XUI_OK && state.bEnabled);
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        xui_document_snapshot snap; xui_doc_range_t selected; uint64_t found;
        CHECK(xuiDocumentCreate(NULL, &d) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiDocumentEditorInsertText(editor, "ab", 2) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "ab", 2, NULL, &selected, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snap); selected.tAnchor.iOffset = selected.tCaret.iOffset = 1;
        CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
        CHECK(xuiDocumentEditorSetLink(editor, "/uri", NULL) == XUI_OK); plain(d, "a/urib\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); plain(d, "ab\n");
        CHECK(xuiDocumentEditorInsertLink(editor, "go", 2, "/target", "T") == XUI_OK); plain(d, "agob\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); plain(d, "ab\n");
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    {
        xui_doc_desc_t md = {0}; xui_document_snapshot snap; xui_doc_range_t selected; uint64_t found;
        md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&md, &d) == XUI_OK && xuiDocumentLoadMarkdown(d, "ab\n", 3) == XUI_OK);
        ed.tView.pDocument = d; ed.iMode = XUI_DOC_VISUAL;
        CHECK(xuiDocumentEditorCreate(context, &ed, &editor) == XUI_OK && xuiSetRootWidget(context, editor) == XUI_OK);
        CHECK(xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 320, 240}) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(d, &snap) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snap, XUI_DOC_SEMANTIC, "ab", 2, NULL, &selected, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snap); selected.tAnchor.iOffset = selected.tCaret.iOffset = 1;
        CHECK(xuiDocumentViewSetSelection(editor, &selected) == XUI_OK);
        CHECK(xuiDocumentEditorSetLink(editor, "/uri", NULL) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, "ab\n");
        CHECK(xuiDocumentEditorInsertLink(editor, "go", 2, "/target", NULL) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK); source(d, "ab\n");
        xuiWidgetDestroy(editor); xuiDocumentRelease(d);
    }
    cell_alignment_command_state_cases(context);
    text_style_editor_cases(context);
    language_editor_cases(context);
    drag_autoscroll_cases(context);
    semantic_accessibility_cases(context);
    accessible_text_range_cases(context);
    accessible_container_text_cases(context);
    accessible_structured_container_cases(context);
    deletion_command_boundary_cases(context);
    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target); xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    group_editor_cases();
    group_editor_boundary_cases();
    async_editor_cases();
    async_editor_stream_cases();
    async_editor_live_cases();
    async_editor_visual_cases();
    async_editor_visual_backspace_cases();
    async_editor_visual_selection_cases();
    async_editor_visual_oom_sweep();
    async_editor_visual_scale_cases();
    async_editor_live_scale_cases();
    ordered_editor_cases();
    async_editor_scale_cases();
    source_editor_navigation_cases();
    puts("DocumentEditor: structural input, clipboard, grapheme deletion, marks, selection undo, read-only, IME projection and three-mode Markdown history passed."); return 0;
}
