#include "../xui_document_ui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(e) do { if (!(e)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #e); exit(1); } } while (0)

static xui_doc_node_id find_kind(xui_document_snapshot snapshot, xui_doc_node_id parent, uint32_t kind)
{
    xui_doc_node_info_t info = {0}; xui_doc_node_id child, found; uint64_t i;
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, parent, &info) == XUI_OK);
    if (info.iKind == kind) return parent;
    for (i = 0; i < info.iChildCount; i++) {
        CHECK(xuiDocumentSnapshotGetChild(snapshot, parent, i, &child) == XUI_OK);
        found = find_kind(snapshot, child, kind); if (found) return found;
    }
    return 0;
}
static void expect_node(xui_document document, xui_doc_node_id id, uint32_t kind, const char* text)
{
    xui_document_snapshot snapshot; xui_doc_node_info_t info = {0}; char bytes[256]; uint64_t size;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &info) == XUI_OK && info.iKind == kind);
    CHECK(xuiDocumentSnapshotCopyText(snapshot, id, bytes, sizeof(bytes), &size) == XUI_OK &&
        size == strlen(text) && !memcmp(bytes, text, (size_t)size));
    xuiDocumentSnapshotRelease(snapshot);
}
static void expect_code_language(xui_document document, xui_doc_node_id id, const char* language)
{
    xui_document_snapshot snapshot; xui_doc_node_info_t info = {0};
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    info.iSize = sizeof(info);
    CHECK(xuiDocumentSnapshotGetNode(snapshot, id, &info) == XUI_OK &&
        info.iKind == XUI_DOC_CODE_BLOCK &&
        strcmp(info.sInfo ? info.sInfo : "", language) == 0);
    xuiDocumentSnapshotRelease(snapshot);
}
static void expect_source(xui_document document, const char* part)
{
    xui_document_snapshot snapshot; char source[2048]; uint64_t size;
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(snapshot, source, sizeof(source), &size) == XUI_OK &&
        size < sizeof(source) && strstr(source, part));
    xuiDocumentSnapshotRelease(snapshot);
}
static xui_widget editor_for(xui_context context, xui_document document)
{
    xui_doc_editor_desc_t desc = {0}; xui_widget editor;
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView);
    desc.tView.pDocument = document; desc.iMode = XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context, &desc, &editor) == XUI_OK &&
        xuiSetRootWidget(context, editor) == XUI_OK &&
        xuiWidgetSetRect(editor, (xui_rect_t){0, 0, 640, 480}) == XUI_OK);
    return editor;
}
static void rich_case(xui_context context)
{
    xui_document document; xui_document_transaction txn; xui_widget editor;
    xui_doc_node_desc_t desc = {0}; xui_doc_node_id paragraph, math, diagram, html;
    uint64_t revision;
    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
        xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(txn, 1, XUI_DOCUMENT_APPEND, &desc, &paragraph) == XUI_OK);
    desc.iKind = XUI_DOC_MATH; desc.sText = "x^2"; desc.iTextBytes = 3;
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND, &desc, &math) == XUI_OK);
    desc.iKind = XUI_DOC_DIAGRAM; desc.sText = "graph TD"; desc.iTextBytes = 8;
    desc.sInfo = "mermaid"; desc.tAttributes.iFlags = XUI_DOC_BLOCK;
    CHECK(xuiDocumentTxnInsertNode(txn, 1, XUI_DOCUMENT_APPEND, &desc, &diagram) == XUI_OK);
    desc.iKind = XUI_DOC_HTML; desc.sText = "<div>old</div>";
    desc.iTextBytes = strlen(desc.sText); desc.sInfo = NULL;
    CHECK(xuiDocumentTxnInsertNode(txn, 1, XUI_DOCUMENT_APPEND, &desc, &html) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
    editor = editor_for(context, document); revision = xuiDocumentGetRevision(document);
    CHECK(xuiDocumentEditorUpdateObjectSource(editor, math, "z^2", 3) == XUI_OK);
    CHECK(xuiDocumentEditorUpdateObjectSource(editor, diagram, "graph LR", 8) == XUI_OK);
    CHECK(xuiDocumentEditorUpdateObjectSource(editor, html, "<div>new</div>", 14) == XUI_OK);
    CHECK(xuiDocumentGetRevision(document) == revision + 3);
    expect_node(document, math, XUI_DOC_MATH, "z^2");
    expect_node(document, diagram, XUI_DOC_DIAGRAM, "graph LR");
    expect_node(document, html, XUI_DOC_HTML, "<div>new</div>");
    CHECK(xuiDocumentEditorUpdateObjectSource(editor, paragraph, "bad", 3) == XUI_ERROR_INVALID_ARGUMENT &&
        xuiDocumentGetRevision(document) == revision + 3);
    CHECK(xuiDocumentEditorUpdateObjectSource(editor, math, "\xc0\xaf", 2) == XUI_ERROR_INVALID_ARGUMENT &&
        xuiDocumentGetRevision(document) == revision + 3);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK &&
        xuiDocumentEditorUpdateObjectSource(editor, math, "a", 1) == XUI_ERROR_UNSUPPORTED);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK); expect_node(document, html, XUI_DOC_HTML, "<div>old</div>");
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK); expect_node(document, diagram, XUI_DOC_DIAGRAM, "graph TD");
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK); expect_node(document, math, XUI_DOC_MATH, "x^2");
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
}
static void markdown_case(xui_context context)
{
    const char* source = "Inline $x$.\n\n~~~mermaid\ngraph TD\n~~~\n\n<div>old</div>\n";
    xui_doc_desc_t desc = {0}; xui_document document; xui_document_snapshot snapshot;
    xui_doc_node_id math, diagram, html; xui_widget editor; uint64_t revision;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN; desc.iMarkdownDialect = XUI_MD_EXTENDED;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK &&
        xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    math = find_kind(snapshot, 1, XUI_DOC_MATH);
    diagram = find_kind(snapshot, 1, XUI_DOC_DIAGRAM);
    html = find_kind(snapshot, 1, XUI_DOC_HTML);
    CHECK(math && diagram && html); xuiDocumentSnapshotRelease(snapshot);
    editor = editor_for(context, document); revision = xuiDocumentGetRevision(document);
    CHECK(xuiDocumentEditorUpdateObjectSource(editor, math, "y", 1) == XUI_OK);
    expect_node(document, math, XUI_DOC_MATH, "y"); expect_source(document, "$y$");
    CHECK(xuiDocumentEditorUpdateObjectSource(editor, math, "a$ b", 4) != XUI_OK &&
        xuiDocumentGetRevision(document) == revision + 1);
    expect_node(document, math, XUI_DOC_MATH, "y"); expect_source(document, "$y$");
    CHECK(xuiDocumentEditorUpdateObjectSource(editor, diagram, "graph LR\n", 9) == XUI_OK);
    expect_node(document, diagram, XUI_DOC_DIAGRAM, "graph LR\n"); expect_source(document, "graph LR");
    CHECK(xuiDocumentEditorUpdateObjectSource(editor, diagram,
        "graph LR\nA~~~B\n", strlen("graph LR\nA~~~B\n")) == XUI_OK);
    expect_node(document, diagram, XUI_DOC_DIAGRAM, "graph LR\nA~~~B\n");
    expect_source(document, "~~~mermaid\ngraph LR\nA~~~B\n~~~");
    CHECK(xuiDocumentEditorUpdateObjectSource(editor, diagram,
        "graph LR\n~~~\nA-->B\n", strlen("graph LR\n~~~\nA-->B\n")) == XUI_OK);
    expect_node(document, diagram, XUI_DOC_DIAGRAM, "graph LR\n~~~\nA-->B\n");
    expect_source(document, "~~~~mermaid");
    CHECK(xuiDocumentEditorUpdateObjectSource(editor, html, "<div>new</div>\n", 15) == XUI_OK);
    expect_node(document, html, XUI_DOC_HTML, "<div>new</div>\n"); expect_source(document, "<div>new</div>");
    CHECK(xuiDocumentGetRevision(document) == revision + 5);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK &&
        xuiDocumentEditorUpdateObjectSource(editor, math, "z", 1) == XUI_ERROR_UNSUPPORTED &&
        xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK);
    expect_node(document, math, XUI_DOC_MATH, "x");
    expect_node(document, diagram, XUI_DOC_DIAGRAM, "graph TD\n");
    expect_node(document, html, XUI_DOC_HTML, "<div>old</div>\n");
    expect_source(document, "Inline $x$.");
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
}
static void markdown_backtick_case(xui_context context)
{
    const char* source = "```mermaid\ngraph TD\n```\n";
    xui_doc_desc_t desc = {0}; xui_document document; xui_document_snapshot snapshot;
    xui_doc_node_id diagram; xui_widget editor; uint64_t revision;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = XUI_MD_EXTENDED;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK &&
        xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    diagram = find_kind(snapshot, 1, XUI_DOC_DIAGRAM);
    CHECK(diagram); xuiDocumentSnapshotRelease(snapshot);
    editor = editor_for(context, document); revision = xuiDocumentGetRevision(document);
    CHECK(xuiDocumentEditorUpdateObjectSource(editor, diagram,
        "graph LR\nA```B\n", strlen("graph LR\nA```B\n")) == XUI_OK);
    expect_node(document, diagram, XUI_DOC_DIAGRAM, "graph LR\nA```B\n");
    expect_source(document, "```mermaid\ngraph LR\nA```B\n```");
    CHECK(xuiDocumentEditorUpdateObjectSource(editor, diagram,
        "graph LR\n```\nA-->B\n", strlen("graph LR\n```\nA-->B\n")) == XUI_OK);
    expect_node(document, diagram, XUI_DOC_DIAGRAM, "graph LR\n```\nA-->B\n");
    expect_source(document, "~~~mermaid\ngraph LR\n```\nA-->B\n~~~");
    CHECK(xuiDocumentGetRevision(document) == revision + 2);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK);
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK);
    expect_node(document, diagram, XUI_DOC_DIAGRAM, "graph TD\n");
    expect_source(document, source);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
}
static void markdown_code_fence_case(void)
{
    const char* source = "```c\nold\n```\n";
    const char* changed = "new\n```\nrest\n";
    xui_doc_desc_t desc = {0}; xui_document document; xui_document_snapshot snapshot;
    xui_document_transaction txn; xui_doc_node_id code; uint64_t revision;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = XUI_MD_GFM;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK &&
        xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    code = find_kind(snapshot, 1, XUI_DOC_CODE_BLOCK);
    CHECK(code); xuiDocumentSnapshotRelease(snapshot);
    revision = xuiDocumentGetRevision(document);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(txn, code, 0, 4, changed, strlen(changed)) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    CHECK(xuiDocumentGetRevision(document) == revision + 1);
    expect_node(document, code, XUI_DOC_CODE_BLOCK, changed);
    expect_source(document, "~~~c\nnew\n```\nrest\n~~~");
    CHECK(xuiDocumentUndo(document, NULL) == XUI_OK);
    expect_node(document, code, XUI_DOC_CODE_BLOCK, "old\n");
    expect_source(document, source);
    xuiDocumentRelease(document);
}
static void markdown_long_fence_case(void)
{
    const char* source = "lead\n\n~~~~mermaid  \n~~~\nold\n~~~~\n";
    xui_doc_desc_t desc = {0}; xui_document document; xui_document_snapshot snapshot;
    xui_doc_txn_desc_t source_edit = {0};
    xui_document_transaction txn; xui_doc_node_id diagram;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = XUI_MD_EXTENDED;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK &&
        xuiDocumentLoadMarkdown(document, source, strlen(source)) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    diagram = find_kind(snapshot, 1, XUI_DOC_DIAGRAM);
    CHECK(diagram); xuiDocumentSnapshotRelease(snapshot);
    source_edit.iSize = sizeof(source_edit); source_edit.iDomain = XUI_DOC_SOURCE;
    CHECK(xuiDocumentBeginTransaction(document, &source_edit, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceSource(txn, 0, 4, "longer lead", 11) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    expect_node(document, diagram, XUI_DOC_DIAGRAM, "~~~\nold\n");
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(txn, diagram, 4, 7, "new", 3) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    expect_node(document, diagram, XUI_DOC_DIAGRAM, "~~~\nnew\n");
    expect_source(document, "longer lead\n\n~~~~mermaid  \n~~~\nnew\n~~~~");
    xuiDocumentRelease(document);
}
static void object_insert_editor_case(xui_context context)
{
    {
        xui_document document; xui_document_snapshot snapshot; xui_widget editor;
        xui_doc_node_id object;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
        editor = editor_for(context, document);
        CHECK(xuiDocumentEditorInsertObject(editor, XUI_DOC_MATH, 0, "x+y", 3) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        object = find_kind(snapshot, 1, XUI_DOC_MATH);
        CHECK(object); xuiDocumentSnapshotRelease(snapshot);
        expect_node(document, object, XUI_DOC_MATH, "x+y");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(!find_kind(snapshot, 1, XUI_DOC_MATH)); xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorInsertObject(editor, XUI_DOC_DIAGRAM, 0,
            "graph TD", 8) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        object = find_kind(snapshot, 1, XUI_DOC_DIAGRAM);
        CHECK(object); xuiDocumentSnapshotRelease(snapshot);
        expect_node(document, object, XUI_DOC_DIAGRAM, "graph TD");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorInsertObject(editor, XUI_DOC_HTML, XUI_DOC_BLOCK,
            "<div>raw</div>", 14) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        object = find_kind(snapshot, 1, XUI_DOC_HTML);
        CHECK(object); xuiDocumentSnapshotRelease(snapshot);
        expect_node(document, object, XUI_DOC_HTML, "<div>raw</div>");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    {
        const char* original = "a  b\n";
        xui_doc_desc_t desc = {0}; xui_document document; xui_document_snapshot snapshot;
        xui_doc_range_t range; xui_widget editor; xui_doc_node_id object; uint64_t found, revision;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_EXTENDED;
        CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, original, strlen(original)) == XUI_OK);
        editor = editor_for(context, document);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "a  b", 4,
            NULL, &range, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snapshot);
        range.tAnchor.iOffset = range.tCaret.iOffset = 2;
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        CHECK(xuiDocumentEditorInsertObject(editor, XUI_DOC_MATH, 0, "x+y", 3) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        object = find_kind(snapshot, 1, XUI_DOC_MATH);
        CHECK(object); xuiDocumentSnapshotRelease(snapshot);
        expect_node(document, object, XUI_DOC_MATH, "x+y"); expect_source(document, "$x+y$");
        CHECK(xuiDocumentEditorUpdateObjectSource(editor, object, "z", 1) == XUI_OK);
        expect_node(document, object, XUI_DOC_MATH, "z");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        expect_node(document, object, XUI_DOC_MATH, "x+y");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        expect_source(document, original);
        CHECK(xuiDocumentEditorInsertObject(editor, XUI_DOC_HTML, 0, "<br>", 4) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        object = find_kind(snapshot, 1, XUI_DOC_HTML);
        CHECK(object); xuiDocumentSnapshotRelease(snapshot);
        expect_node(document, object, XUI_DOC_HTML, "<br>");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorInsertObject(editor, XUI_DOC_DIAGRAM, 0,
            "graph TD", 8) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        object = find_kind(snapshot, 1, XUI_DOC_DIAGRAM);
        CHECK(object); xuiDocumentSnapshotRelease(snapshot);
        expect_node(document, object, XUI_DOC_DIAGRAM, "graph TD\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
        revision = xuiDocumentGetRevision(document);
        CHECK(xuiDocumentEditorInsertObject(editor, XUI_DOC_MATH, 0, "x", 1) == XUI_ERROR_UNSUPPORTED &&
            xuiDocumentGetRevision(document) == revision);
        CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiDocumentEditorInsertObject(editor, XUI_DOC_MATH, 0, "x", 1) == XUI_ERROR_UNSUPPORTED);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("Document object insertion: Rich/Markdown math, Mermaid, HTML, Undo and editor mode policy passed");
}
static void code_and_rule_editor_case(xui_context context)
{
    {
        xui_document document; xui_document_snapshot snapshot; xui_widget editor;
        xui_doc_node_id code;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
        editor = editor_for(context, document);
        CHECK(xuiDocumentEditorInsertCodeBlock(editor, "c", "", 0) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        code = find_kind(snapshot, 1, XUI_DOC_CODE_BLOCK);
        CHECK(code); xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorSetCodeBlockLanguage(editor, code, "cpp") == XUI_OK);
        expect_code_language(document, code, "cpp");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        expect_code_language(document, code, "c");
        CHECK(xuiDocumentEditorUpdateObjectSource(editor, code, "new", 3) == XUI_OK);
        expect_node(document, code, XUI_DOC_CODE_BLOCK, "new");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        expect_node(document, code, XUI_DOC_CODE_BLOCK, "");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorInsertRule(editor) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(find_kind(snapshot, 1, XUI_DOC_RULE)); xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    {
        const char* original = "beforeafter\n";
        xui_doc_desc_t desc = {0}; xui_document document; xui_document_snapshot snapshot;
        xui_doc_range_t range; xui_widget editor; xui_doc_node_id code; uint64_t found, revision;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_EXTENDED;
        CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, original, strlen(original)) == XUI_OK);
        editor = editor_for(context, document);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "beforeafter", 11,
            NULL, &range, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snapshot);
        range.tAnchor.iOffset = range.tCaret.iOffset = 6;
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        CHECK(xuiDocumentEditorInsertCodeBlock(editor, "c", "x", 1) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        code = find_kind(snapshot, 1, XUI_DOC_CODE_BLOCK);
        CHECK(code); xuiDocumentSnapshotRelease(snapshot);
        expect_node(document, code, XUI_DOC_CODE_BLOCK, "x\n");
        CHECK(xuiDocumentEditorSetCodeBlockLanguage(editor, code, "cpp") == XUI_OK);
        expect_code_language(document, code, "cpp");
        revision = xuiDocumentGetRevision(document);
        CHECK(xuiDocumentEditorSetCodeBlockLanguage(editor, code, "bad lang") == XUI_ERROR_INVALID_ARGUMENT &&
            xuiDocumentGetRevision(document) == revision);
        CHECK(xuiDocumentEditorSetCodeBlockLanguage(editor, code, "mermaid") == XUI_DOC_ERROR_UNREPRESENTABLE &&
            xuiDocumentGetRevision(document) == revision);
        CHECK(xuiDocumentEditorUpdateObjectSource(editor, code, "y\n", 2) == XUI_OK);
        expect_node(document, code, XUI_DOC_CODE_BLOCK, "y\n");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        expect_node(document, code, XUI_DOC_CODE_BLOCK, "x\n");
        expect_code_language(document, code, "cpp");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        expect_code_language(document, code, "c");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        expect_source(document, original);
        CHECK(xuiDocumentEditorInsertRule(editor) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(find_kind(snapshot, 1, XUI_DOC_RULE)); xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        expect_source(document, original);
        CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
        revision = xuiDocumentGetRevision(document);
        CHECK(xuiDocumentEditorInsertCodeBlock(editor, "c", "x", 1) == XUI_ERROR_UNSUPPORTED &&
            xuiDocumentGetRevision(document) == revision);
        CHECK(xuiDocumentEditorSetCodeBlockLanguage(editor, code, "cpp") == XUI_ERROR_UNSUPPORTED);
        CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiDocumentEditorInsertRule(editor) == XUI_ERROR_UNSUPPORTED);
        CHECK(xuiDocumentEditorSetCodeBlockLanguage(editor, code, "cpp") == XUI_ERROR_UNSUPPORTED);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("Document code/rule insertion: Rich/Markdown, source editing, Undo and mode policy passed");
}
static void quote_editor_case(xui_context context)
{
    {
        xui_document document; xui_document_transaction txn; xui_widget editor;
        xui_document_snapshot snapshot; xui_doc_node_desc_t desc = {0};
        xui_doc_range_t range; xui_doc_node_id paragraph, text_id, quote; uint64_t found;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK &&
            xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
        desc.iSize = sizeof(desc); desc.iKind = XUI_DOC_PARAGRAPH;
        CHECK(xuiDocumentTxnInsertNode(txn, 1, XUI_DOCUMENT_APPEND, &desc, &paragraph) == XUI_OK);
        desc.iKind = XUI_DOC_TEXT; desc.sText = "alpha"; desc.iTextBytes = 5;
        CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND, &desc, &text_id) == XUI_OK);
        CHECK(text_id);
        CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
        editor = editor_for(context, document);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "alpha", 5,
            NULL, &range, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snapshot);
        range.tAnchor.iOffset = range.tCaret.iOffset = 2;
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        CHECK(xuiDocumentEditorWrapQuote(editor) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        quote = find_kind(snapshot, 1, XUI_DOC_QUOTE); CHECK(quote);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorUnwrapQuote(editor) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(!find_kind(snapshot, 1, XUI_DOC_QUOTE)); xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(find_kind(snapshot, 1, XUI_DOC_QUOTE) == quote); xuiDocumentSnapshotRelease(snapshot);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    {
        const char* original = "alpha\n\nbeta\n";
        xui_doc_desc_t desc = {0}; xui_document document; xui_document_snapshot snapshot;
        xui_doc_range_t range; xui_widget editor; uint64_t found, revision;
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_EXTENDED;
        CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, original, strlen(original)) == XUI_OK);
        editor = editor_for(context, document);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "alpha", 5,
            NULL, &range, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snapshot);
        range.tAnchor.iOffset = range.tCaret.iOffset = 2;
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        CHECK(xuiDocumentEditorWrapQuote(editor) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(find_kind(snapshot, 1, XUI_DOC_QUOTE)); xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorUnwrapQuote(editor) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(!find_kind(snapshot, 1, XUI_DOC_QUOTE)); xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        expect_source(document, original);
        CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
        revision = xuiDocumentGetRevision(document);
        CHECK(xuiDocumentEditorWrapQuote(editor) == XUI_ERROR_UNSUPPORTED &&
            xuiDocumentGetRevision(document) == revision);
        CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiDocumentEditorWrapQuote(editor) == XUI_ERROR_UNSUPPORTED);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("Document quote wrap/unwrap: Rich/Markdown, caret, Undo and mode policy passed");
}
static void footnote_editor_case(xui_context context)
{
    {
        xui_document document; xui_document_snapshot snapshot; xui_widget editor;
        xui_doc_range_t range; xui_doc_node_id reference, footnote, text_id;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
        editor = editor_for(context, document);
        CHECK(xuiDocumentEditorInsertFootnote(editor, "n", "body", 4) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        reference = find_kind(snapshot, 1, XUI_DOC_FOOTNOTE_REF);
        footnote = find_kind(snapshot, 1, XUI_DOC_FOOTNOTE);
        CHECK(reference && footnote);
        text_id = find_kind(snapshot, footnote, XUI_DOC_TEXT); CHECK(text_id);
        xuiDocumentSnapshotRelease(snapshot);
        expect_node(document, reference, XUI_DOC_FOOTNOTE_REF, "n");
        expect_node(document, text_id, XUI_DOC_TEXT, "body");
        CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
            range.tCaret.iNodeId == text_id && range.tCaret.iKind == XUI_DOC_POSITION_TEXT);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(!find_kind(snapshot, 1, XUI_DOC_FOOTNOTE_REF) &&
            !find_kind(snapshot, 1, XUI_DOC_FOOTNOTE));
        xuiDocumentSnapshotRelease(snapshot);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    {
        const char* original = "beforeafter\n";
        xui_doc_desc_t desc = {0}; xui_document document; xui_document_snapshot snapshot;
        xui_doc_range_t range; xui_widget editor;
        xui_doc_node_id footnote, paragraph, text_id; uint64_t found, revision;
        xui_doc_node_info_t info = {0};
        desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
        desc.iMarkdownDialect = XUI_MD_EXTENDED;
        CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, original, strlen(original)) == XUI_OK);
        editor = editor_for(context, document);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "beforeafter", 11,
            NULL, &range, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snapshot);
        range.tAnchor.iOffset = range.tCaret.iOffset = 6;
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        CHECK(xuiDocumentEditorInsertFootnote(editor, NULL, NULL, 0) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        footnote = find_kind(snapshot, 1, XUI_DOC_FOOTNOTE);
        info.iSize = sizeof(info);
        CHECK(footnote && xuiDocumentSnapshotGetNode(snapshot, footnote, &info) == XUI_OK &&
            !strcmp(info.sInfo, "fn1") && !info.iChildCount);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
            range.tCaret.iNodeId == footnote && range.tCaret.iKind == XUI_DOC_POSITION_GAP);
        CHECK(xuiDocumentEditorInsertText(editor, "note", 4) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        text_id = find_kind(snapshot, footnote, XUI_DOC_TEXT);
        CHECK(text_id); xuiDocumentSnapshotRelease(snapshot);
        expect_node(document, text_id, XUI_DOC_TEXT, "note");
        expect_source(document, "[^fn1]: note");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        expect_source(document, original);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC, "beforeafter", 11,
            NULL, &range, 1, &found) == XUI_OK && found == 1);
        xuiDocumentSnapshotRelease(snapshot);
        range.tAnchor.iOffset = range.tCaret.iOffset = 6;
        CHECK(xuiDocumentViewSetSelection(editor, &range) == XUI_OK);
        CHECK(xuiDocumentEditorInsertFootnote(editor, "m", "One\nTwo", 7) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        footnote = find_kind(snapshot, 1, XUI_DOC_FOOTNOTE); CHECK(footnote);
        CHECK(xuiDocumentSnapshotGetNode(snapshot, footnote, &info) == XUI_OK &&
            info.iChildCount == 2);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, footnote, 1, &paragraph) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &text_id) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentViewGetSelection(editor, &range) == XUI_OK &&
            range.tCaret.iNodeId == text_id && range.tCaret.iKind == XUI_DOC_POSITION_TEXT);
        expect_node(document, text_id, XUI_DOC_TEXT, "Two");
        expect_source(document, "[^m]: One\n    \n    Two");
        CHECK(xuiDocumentEditorInsertText(editor, "x", 1) == XUI_OK);
        expect_node(document, text_id, XUI_DOC_TEXT, "Twox");
        expect_source(document, "    Twox");
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        expect_source(document, original);
        CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
        revision = xuiDocumentGetRevision(document);
        CHECK(xuiDocumentEditorInsertFootnote(editor, "x", "body", 4) == XUI_ERROR_UNSUPPORTED &&
            xuiDocumentGetRevision(document) == revision);
        CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
        CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
        CHECK(xuiDocumentEditorInsertFootnote(editor, "x", "body", 4) == XUI_ERROR_UNSUPPORTED);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    puts("Document footnote insertion: Rich/Markdown, body focus/input, Undo and mode policy passed");
}
static void footnote_remove_editor_case(xui_context context)
{
    const char* original = "a[^n]\n\n[^n]: body\n";
    xui_doc_desc_t desc = {0}; xui_document document; xui_document_snapshot snapshot;
    xui_doc_range_t selection; xui_widget editor;
    xui_doc_node_id reference, footnote; uint64_t revision;
    desc.iSize = sizeof(desc); desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    desc.iMarkdownDialect = XUI_MD_EXTENDED;
    CHECK(xuiDocumentCreate(&desc, &document) == XUI_OK &&
        xuiDocumentLoadMarkdown(document, original, strlen(original)) == XUI_OK);
    editor = editor_for(context, document);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    reference = find_kind(snapshot, 1, XUI_DOC_FOOTNOTE_REF);
    footnote = find_kind(snapshot, 1, XUI_DOC_FOOTNOTE);
    CHECK(reference && footnote); xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentEditorRemoveFootnoteReference(editor, reference) == XUI_OK);
    CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
    CHECK(!find_kind(snapshot, 1, XUI_DOC_FOOTNOTE_REF) &&
        !find_kind(snapshot, 1, XUI_DOC_FOOTNOTE));
    xuiDocumentSnapshotRelease(snapshot);
    CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
        selection.tCaret.iKind == XUI_DOC_POSITION_GAP);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    expect_source(document, original);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 1) == XUI_OK);
    revision = xuiDocumentGetRevision(document);
    CHECK(xuiDocumentEditorRemoveFootnoteReference(editor, reference) == XUI_ERROR_UNSUPPORTED &&
        xuiDocumentGetRevision(document) == revision);
    CHECK(xuiDocumentEditorSetReadOnly(editor, 0) == XUI_OK);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    CHECK(xuiDocumentEditorRemoveFootnoteReference(editor, reference) == XUI_ERROR_UNSUPPORTED);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    puts("Document footnote removal: caret, Undo, read-only and mode policy passed");
}
int main(void)
{
    xui_test_proxy_state_t proxy; xui_context context; xui_font font;
    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK && xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "object-source.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK && xuiInputViewport(context, 640, 480) == XUI_OK);
    rich_case(context); markdown_case(context); markdown_backtick_case(context);
    object_insert_editor_case(context);
    code_and_rule_editor_case(context);
    quote_editor_case(context);
    footnote_editor_case(context);
    footnote_remove_editor_case(context);
    markdown_code_fence_case(); markdown_long_fence_case();
    xuiDestroy(context); proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Document object source editing: Rich/Markdown math, Mermaid, HTML, code fences, atomic rejection and Undo passed");
    return 0;
}
