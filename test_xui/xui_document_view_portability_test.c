#include "../xui_document_ui.h"
#include "../xge.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); exit(1); \
} } while (0)

/* The headless UI test excludes PNG clipboard paths. Fail loudly if either
 * path becomes reachable instead of silently replacing XGE image behavior. */
int xgeImageInfoMemory(const void* data, int size, int* width, int* height)
{
    (void)data; (void)size; (void)width; (void)height;
    fprintf(stderr, "unexpected XGE image probe in headless DocumentView test\n");
    abort();
}
int xgeImageEncodePNGEx(int width, int height, const void* pixels,
    int stride, uint32_t flags, void** data, size_t* size)
{
    (void)width; (void)height; (void)pixels; (void)stride;
    (void)flags; (void)data; (void)size;
    fprintf(stderr, "unexpected XGE PNG encode in headless DocumentView test\n");
    abort();
}

static void render_view(xui_context context, xui_surface target,
    xui_test_proxy_state_t* proxy, xui_widget view)
{
    xui_rect_t rect = {0, 0, 320, 200};
    xui_rect_i_t damage = {0, 0, 320, 200};
    xui_doc_rect_t size = {0};
    xui_doc_position_t hit = {0};
    int exact = 0;
    CHECK(xuiSetRootWidget(context, view) == XUI_OK);
    CHECK(xuiWidgetSetRect(view, rect) == XUI_OK);
    CHECK(xuiInputViewport(context, 320, 200) == XUI_OK);
    CHECK(xuiUpdate(context, .016f) == XUI_OK);
    CHECK(xuiDocumentViewGetContentSize(view, &size, &exact) == XUI_OK &&
        exact && size.height > 0);
    CHECK(xuiDocumentViewHitTest(view, 20, 20, &hit) == XUI_OK &&
        hit.iDocumentId != 0);
    xuiTestSurfaceReset(target);
    CHECK(xuiRender(context, target, &damage, 1) == XUI_OK);
    {
        xui_surface cache = xuiWidgetGetCacheSurface(view, xuiWidgetGetStateId(view));
        CHECK(xuiTestSurfaceGetTextDrawCount(target) > 0 ||
            (cache && xuiTestSurfaceGetTextDrawCount(cache) > 0));
    }
    (void)proxy;
}

#include "xui_document_grapheme_editor_cases.h"
#include "xui_document_ligature_editor_cases.h"
#include "xui_document_grapheme_font_editor_cases.h"
#include "xui_document_nested_prefix_editor_cases.h"
#include "xui_document_list_split_source_editor_cases.h"
#include "xui_document_code_language_source_editor_cases.h"
#include "xui_document_heading_source_editor_cases.h"
#include "xui_document_language_editor_cases.h"
static void source_selection_collapse_prefix(xui_context context)
{
    const size_t bytes=128*1024;char* source=malloc(bytes+1);int mode;
    CHECK(source);memset(source,'a',bytes);source[bytes]=0;
    for(mode=XUI_DOC_SOURCE_TEXT;mode<=XUI_DOC_LIVE_MARKDOWN;mode++) {
        xui_doc_desc_t d={0};xui_doc_editor_desc_t desc={0};xui_document document;xui_widget editor;
        xui_doc_range_t selection={0},moved;xui_doc_renderer_stats_t before={0},after={0};xui_event_t key={0};
        xui_doc_rect_t content;int exact;
        d.iSize=sizeof(d);d.iProfile=XUI_DOCUMENT_MARKDOWN;
        CHECK(xuiDocumentCreate(&d,&document)==XUI_OK && xuiDocumentLoadMarkdown(document,source,bytes)==XUI_OK);
        desc.iSize=sizeof(desc);desc.tView.iSize=sizeof(desc.tView);desc.tView.pDocument=document;
        CHECK(xuiDocumentEditorCreate(context,&desc,&editor)==XUI_OK && xuiDocumentViewSetMode(editor,mode)==XUI_OK &&
            xuiSetRootWidget(context,editor)==XUI_OK &&
            xuiWidgetSetRect(editor,(xui_rect_t){0,0,320,200})==XUI_OK && xuiInputViewport(context,320,200)==XUI_OK &&
            xuiUpdate(context,.016f)==XUI_OK);
        CHECK(xuiDocumentViewGetContentSize(editor,&content,&exact)==XUI_OK);
        selection.tAnchor.iSize=sizeof(selection.tAnchor);selection.tAnchor.iKind=XUI_DOC_POSITION_SOURCE;
        selection.tAnchor.iDocumentId=xuiDocumentGetIdentity(document);selection.tAnchor.iRevision=xuiDocumentGetRevision(document);
        selection.tAnchor.iNodeId=1;selection.tCaret=selection.tAnchor;selection.tCaret.iOffset=bytes;
        CHECK(xuiDocumentViewSetSelection(editor,&selection)==XUI_OK);
        before.iSize=after.iSize=sizeof(before);CHECK(xuiDocumentViewGetRenderStats(editor,&before)==XUI_OK);
        CHECK(before.iTextRunBytes>0 && before.iTextRunBytes<bytes/3);
        key.iSize=sizeof(key);key.iType=XUI_EVENT_KEY_DOWN;key.pTarget=editor;key.iKey=XUI_KEY_LEFT;
        CHECK(xuiDispatchEvent(context,&key)==XUI_OK && xuiDocumentViewGetSelection(editor,&moved)==XUI_OK &&
            moved.tAnchor.iOffset==0 && moved.tCaret.iOffset==0);
        CHECK(xuiDocumentViewGetRenderStats(editor,&after)==XUI_OK);
        if(after.iShapedBytes!=before.iShapedBytes)
            fprintf(stderr,"SOURCE/LIVE collapse mode=%d before=%llu after=%llu run=%llu/%llu\n",mode,
                (unsigned long long)before.iShapedBytes,(unsigned long long)after.iShapedBytes,
                (unsigned long long)before.iTextRunBytes,(unsigned long long)after.iTextRunBytes);
        CHECK(after.iShapedBytes==before.iShapedBytes);
        CHECK(xuiSetRootWidget(context,NULL)==XUI_OK);xuiWidgetDestroy(editor);xuiDocumentRelease(document);
    }
    free(source);puts("SOURCE/LIVE select-all then Left preserves ASCII horizontal prefix without shaping the far end");
}
int main(void)
{
    xui_test_proxy_state_t proxy;
    xui_context context;
    xui_font font;
    xui_surface target;
    xui_document document;
    xui_document_transaction txn;
    xui_doc_node_desc_t node = {0};
    xui_doc_desc_t md = {0};
    xui_doc_view_desc_t desc = {0};
    xui_doc_editor_desc_t editor_desc = {0};
    xui_doc_range_t selection = {0};
    xui_widget view, editor;
    uint64_t paragraph, text_id;

    xuiTestProxyInit(&proxy);
    CHECK(xuiCreate(&context) == XUI_OK);
    CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14, 0) == XUI_OK);
    CHECK(xuiSetDefaultFont(context, font) == XUI_OK);
    CHECK(xuiTestSurfaceCreate(&proxy, &target, 320, 200,
        XUI_SURFACE_USAGE_TARGET) == XUI_OK);
    source_selection_collapse_prefix(context);
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
    language_editor_cases(context);

    CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    node.iSize = sizeof(node); node.iKind = XUI_DOC_PARAGRAPH;
    CHECK(xuiDocumentTxnInsertNode(txn, 1, XUI_DOCUMENT_APPEND,
        &node, &paragraph) == XUI_OK);
    node.iKind = XUI_DOC_TEXT; node.sText = "Hello Rich View";
    node.iTextBytes = strlen(node.sText);
    CHECK(xuiDocumentTxnInsertNode(txn, paragraph, XUI_DOCUMENT_APPEND,
        &node, &text_id) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    desc.iSize = sizeof(desc); desc.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &desc, &view) == XUI_OK);
    render_view(context, target, &proxy, view);
    CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
    CHECK(xuiDocumentTxnReplaceText(txn, text_id, 6, 10, "Linux", 5) == XUI_OK);
    CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
    xuiDocumentTxnRelease(txn);
    render_view(context, target, &proxy, view);
    xuiWidgetDestroy(view);
    editor_desc.iSize = sizeof(editor_desc);
    editor_desc.tView.iSize = sizeof(editor_desc.tView);
    editor_desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK);
    render_view(context, target, &proxy, editor);
    selection.tAnchor.iSize = sizeof(selection.tAnchor);
    selection.tAnchor.iDocumentId = xuiDocumentGetIdentity(document);
    selection.tAnchor.iRevision = xuiDocumentGetRevision(document);
    selection.tAnchor.iNodeId = text_id;
    selection.tAnchor.iKind = XUI_DOC_POSITION_TEXT;
    selection.tAnchor.iOffset = 16;
    selection.tCaret = selection.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "!", 1) == XUI_OK);
    {
        xui_document_snapshot snapshot;
        char bytes[64] = {0}; uint64_t length = 0;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotCopyText(snapshot, text_id, bytes,
            sizeof(bytes), &length) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(length == strlen("Hello Linux View!") &&
            !memcmp(bytes, "Hello Linux View!", (size_t)length));
    }
    render_view(context, target, &proxy, editor);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    { xui_accessible_selection_t local = {0}; xui_accessible_node_t node = {0};
      int i, found = 0;
      local.iSize = sizeof(local); local.iAnchor = 6; local.iCaret = 11;
      node.iSize = sizeof(node);
      CHECK(xuiWidgetGetAccessibleNode(editor, 0, &node) == XUI_OK &&
          node.iRole == XUI_ACCESSIBLE_ROLE_DOCUMENT && node.sValue &&
          !strcmp(node.sValue, "Hello Linux View\n"));
      CHECK(xuiWidgetPerformAccessibleAction(editor, XUI_DOCUMENT_ROOT,
          XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
      node.iSize = sizeof(node);
      CHECK(xuiWidgetGetAccessibleNode(editor, 0, &node) == XUI_OK &&
          node.iTextStart == 6 && node.iTextEnd == 11 &&
          (node.iState & XUI_ACCESSIBLE_STATE_SELECTED));
      CHECK(xuiWidgetPerformAccessibleAction(editor, text_id,
          XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
      CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
          selection.tAnchor.iNodeId == text_id && selection.tAnchor.iOffset == 6 &&
          selection.tCaret.iNodeId == text_id && selection.tCaret.iOffset == 11);
      for (i = 0; i < xuiWidgetGetAccessibleNodeCount(editor); i++) {
          node.iSize = sizeof(node);
          CHECK(xuiWidgetGetAccessibleNode(editor, i, &node) == XUI_OK);
          if (node.iId == text_id) {
              CHECK(node.iTextStart == 6 && node.iTextEnd == 11 &&
                  (node.iState & XUI_ACCESSIBLE_STATE_SELECTED)); found = 1;
          }
      }
      CHECK(found); }
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    {
        xui_doc_node_desc_t item = {0}; xui_doc_node_id paragraph, text_id, image;
        xui_accessible_selection_t local = {0}; xui_accessible_node_t node = {0};
        int i, found = 0;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
        CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
        item.iSize = sizeof(item); item.iKind = XUI_DOC_PARAGRAPH;
        CHECK(xuiDocumentTxnInsertNode(txn, XUI_DOCUMENT_ROOT,
            XUI_DOCUMENT_APPEND, &item, &paragraph) == XUI_OK);
        item.iKind = XUI_DOC_TEXT; item.sText = "a\xe4\xb8\xad";
        item.iTextBytes = 4;
        CHECK(xuiDocumentTxnInsertNode(txn, paragraph,
            XUI_DOCUMENT_APPEND, &item, &text_id) == XUI_OK);
        item.iKind = XUI_DOC_IMAGE; item.sText = "alt"; item.iTextBytes = 3;
        item.sResource = "/img";
        CHECK(xuiDocumentTxnInsertNode(txn, paragraph,
            XUI_DOCUMENT_APPEND, &item, &image) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
        xuiDocumentTxnRelease(txn);
        desc.pDocument = document;
        CHECK(xuiDocumentViewCreate(context, &desc, &view) == XUI_OK);
        render_view(context, target, &proxy, view);
        for (i = 0; i < xuiWidgetGetAccessibleNodeCount(view); i++) {
            node.iSize = sizeof(node);
            CHECK(xuiWidgetGetAccessibleNode(view, i, &node) == XUI_OK);
            if (node.iId == paragraph) {
                CHECK(node.sValue && !strcmp(node.sValue,
                    "a\xe4\xb8\xad\xef\xbf\xbc") &&
                    (node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
                found = 1;
            }
        }
        CHECK(found);
        local.iSize = sizeof(local); local.iAnchor = 4; local.iCaret = 7;
        CHECK(xuiWidgetPerformAccessibleAction(view, paragraph,
            XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
        CHECK(xuiDocumentViewGetSelection(view, &selection) == XUI_OK &&
            selection.tAnchor.iNodeId == paragraph && selection.tAnchor.iOffset == 1 &&
            selection.tCaret.iNodeId == paragraph && selection.tCaret.iOffset == 2);
        (void)text_id; (void)image;
        xuiWidgetDestroy(view); xuiDocumentRelease(document);
    }

    {
        xui_doc_table_selection_t request = {0}, selected = {0};
        xui_doc_node_id table, last, first_cell = 0;
        uint64_t revision; int i;
        CHECK(xuiDocumentCreate(NULL, &document) == XUI_OK);
        CHECK(xuiDocumentBeginTransaction(document, NULL, &txn) == XUI_OK);
        CHECK(xuiDocumentTxnInsertTable(txn, 1, 0, 2, 2, 0, &table) == XUI_OK);
        CHECK(xuiDocumentTxnPasteTableMatrix(txn, table, 0, 0,
            "a\tb\nc\td", 7, &last) == XUI_OK);
        CHECK(xuiDocumentTxnCommit(txn, NULL) == XUI_OK);
        xuiDocumentTxnRelease(txn);
        desc.pDocument = document;
        CHECK(xuiDocumentViewCreate(context, &desc, &view) == XUI_OK);
        render_view(context, target, &proxy, view);
        for (i = 0; i < xuiWidgetGetAccessibleNodeCount(view); i++) {
            xui_accessible_node_t node = {0}; node.iSize = sizeof(node);
            CHECK(xuiWidgetGetAccessibleNode(view, i, &node) == XUI_OK);
            if (node.iId == table)
                CHECK(node.sValue && !strcmp(node.sValue, "a\tb\nc\td"));
            if (node.iRole == XUI_ACCESSIBLE_ROLE_CELL &&
                node.iRow == 0 && node.iColumn == 0) {
                CHECK(node.iRowCount == 1 && node.iColumnCount == 1 &&
                    (node.iState & XUI_ACCESSIBLE_STATE_SELECTABLE) &&
                    (node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
                first_cell = node.iId;
            }
        }
        { xui_accessible_selection_t local = {0}; xui_document_snapshot snap;
          xui_doc_node_id paragraph, text_id;
          local.iSize = sizeof(local); local.iCaret = 1;
          CHECK(first_cell && xuiWidgetPerformAccessibleAction(view, first_cell,
              XUI_ACCESSIBLE_ACTION_SET_SELECTION, &local) == XUI_OK);
          CHECK(xuiDocumentAcquireSnapshot(document, &snap) == XUI_OK);
          CHECK(xuiDocumentSnapshotGetChild(snap, first_cell, 0, &paragraph) == XUI_OK &&
              xuiDocumentSnapshotGetChild(snap, paragraph, 0, &text_id) == XUI_OK);
          xuiDocumentSnapshotRelease(snap);
          CHECK(xuiDocumentViewGetSelection(view, &selection) == XUI_OK &&
              selection.tAnchor.iNodeId == text_id && selection.tAnchor.iOffset == 0 &&
              selection.tCaret.iNodeId == text_id && selection.tCaret.iOffset == 1); }
        CHECK(first_cell && xuiWidgetPerformAccessibleAction(view, first_cell,
            XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK);
        selected.iSize = sizeof(selected);
        CHECK(xuiDocumentViewGetTableSelection(view, &selected) == XUI_OK &&
            selected.iTableId == table && selected.iRows == 1 && selected.iColumns == 1);
        { xui_event_t key = {0}; key.iSize = sizeof(key);
          key.iType = XUI_EVENT_KEY_DOWN; key.pTarget = view;
          key.iModifiers = XUI_MOD_ALT | XUI_MOD_SHIFT;
          key.iKey = XUI_KEY_RIGHT;
          CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
          CHECK(xuiDocumentViewGetTableSelection(view, &selected) == XUI_OK &&
              selected.iRow == 0 && selected.iColumn == 0 &&
              selected.iRows == 1 && selected.iColumns == 2);
          key.iKey = XUI_KEY_DOWN;
          CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
          CHECK(xuiDocumentViewGetTableSelection(view, &selected) == XUI_OK &&
              selected.iRows == 2 && selected.iColumns == 2);
          key.iKey = XUI_KEY_LEFT;
          CHECK(xuiDispatchEvent(context, &key) == XUI_OK);
          CHECK(xuiDocumentViewGetTableSelection(view, &selected) == XUI_OK &&
              selected.iRows == 2 && selected.iColumns == 1); }
        request.iSize = selected.iSize = sizeof(request);
        request.iTableId = table; request.iRows = request.iColumns = 2;
        revision = xuiDocumentGetRevision(document);
        CHECK(xuiDocumentViewSetTableSelection(view, &request) == XUI_OK &&
            xuiDocumentViewGetTableSelection(view, &selected) == XUI_OK &&
            selected.iTableId == table && selected.iRows == 2 && selected.iColumns == 2);
        CHECK(xuiEditHasSelection(view));
        render_view(context, target, &proxy, view);
        request.iRows = 3;
        CHECK(xuiDocumentViewSetTableSelection(view, &request) == XUI_ERROR_INVALID_ARGUMENT &&
            xuiDocumentViewGetTableSelection(view, &selected) == XUI_OK &&
            selected.iRows == 2);
        CHECK(xuiDocumentViewSetTableSelection(view, NULL) == XUI_OK &&
            xuiDocumentViewGetTableSelection(view, &selected) == XUI_ERROR_NOT_FOUND &&
            xuiDocumentGetRevision(document) == revision);
        xuiWidgetDestroy(view);
        desc.bDisableSelection = 1;
        CHECK(xuiDocumentViewCreate(context, &desc, &view) == XUI_OK);
        for (i = 0; i < xuiWidgetGetAccessibleNodeCount(view); i++) {
            xui_accessible_node_t node = {0}; node.iSize = sizeof(node);
            CHECK(xuiWidgetGetAccessibleNode(view, i, &node) == XUI_OK);
            if (node.iId == first_cell)
                CHECK(!(node.iState & XUI_ACCESSIBLE_STATE_SELECTABLE) &&
                    !(node.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_SELECTION)));
        }
        CHECK(xuiWidgetPerformAccessibleAction(view, first_cell,
            XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_ERROR_UNSUPPORTED);
        xuiWidgetDestroy(view); desc.bDisableSelection = 0; xuiDocumentRelease(document);
    }

    md.iSize = sizeof(md); md.iProfile = XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&md, &document) == XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(document, "# Hello Markdown\n", 17) == XUI_OK);
    desc.pDocument = document;
    CHECK(xuiDocumentViewCreate(context, &desc, &view) == XUI_OK);
    render_view(context, target, &proxy, view);
    CHECK(xuiDocumentViewSetMode(view, XUI_DOC_SOURCE_TEXT) == XUI_OK);
    render_view(context, target, &proxy, view);
    xuiWidgetDestroy(view);
    memset(&editor_desc, 0, sizeof(editor_desc));
    editor_desc.iSize = sizeof(editor_desc);
    editor_desc.iMode = XUI_DOC_SOURCE_TEXT;
    editor_desc.tView.iSize = sizeof(editor_desc.tView);
    editor_desc.tView.pDocument = document;
    CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK);
    render_view(context, target, &proxy, editor);
    memset(&selection, 0, sizeof(selection));
    selection.tAnchor.iSize = sizeof(selection.tAnchor);
    selection.tAnchor.iDocumentId = xuiDocumentGetIdentity(document);
    selection.tAnchor.iRevision = xuiDocumentGetRevision(document);
    selection.tAnchor.iNodeId = 1;
    selection.tAnchor.iKind = XUI_DOC_POSITION_SOURCE;
    selection.tAnchor.iOffset = 17;
    selection.tCaret = selection.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "tail", 4) == XUI_OK);
    {
        xui_document_snapshot snapshot;
        char bytes[64] = {0}; uint64_t length = 0;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotCopySource(snapshot, bytes,
            sizeof(bytes), &length) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(length == strlen("# Hello Markdown\ntail") &&
            !memcmp(bytes, "# Hello Markdown\ntail", (size_t)length));
    }
    render_view(context, target, &proxy, editor);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_LIVE_MARKDOWN) == XUI_OK);
    render_view(context, target, &proxy, editor);
    selection.tAnchor.iRevision = xuiDocumentGetRevision(document);
    selection.tCaret = selection.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "tail", 4) == XUI_OK);
    {
        xui_document_snapshot snapshot;
        char bytes[64] = {0}; uint64_t length = 0;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotCopySource(snapshot, bytes,
            sizeof(bytes), &length) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(length == strlen("# Hello Markdown\ntail") &&
            !memcmp(bytes, "# Hello Markdown\ntail", (size_t)length));
    }
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    CHECK(xuiDocumentViewSetMode(editor, XUI_DOC_VISUAL) == XUI_OK);
    render_view(context, target, &proxy, editor);
    {
        xui_document_snapshot snapshot;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, 1, 0, &paragraph) == XUI_OK);
        CHECK(xuiDocumentSnapshotGetChild(snapshot, paragraph, 0, &text_id) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
    }
    selection.tAnchor.iRevision = xuiDocumentGetRevision(document);
    selection.tAnchor.iNodeId = text_id;
    selection.tAnchor.iKind = XUI_DOC_POSITION_TEXT;
    selection.tAnchor.iOffset = 5;
    selection.tCaret = selection.tAnchor;
    CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
    CHECK(xuiDocumentEditorInsertText(editor, "!", 1) == XUI_OK);
    {
        xui_document_snapshot snapshot;
        char bytes[64] = {0}; uint64_t length = 0;
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK);
        CHECK(xuiDocumentSnapshotCopySource(snapshot, bytes,
            sizeof(bytes), &length) == XUI_OK);
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(length == strlen("# Hello\\! Markdown\n") &&
            !memcmp(bytes, "# Hello\\! Markdown\n", (size_t)length));
    }
    render_view(context, target, &proxy, editor);
    CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
    xuiWidgetDestroy(editor); xuiDocumentRelease(document);

    {
        const char* original = "- [ ] portable &amp; task\n";
        const char* checked = "- [x] portable &amp; task\n";
        xui_document_snapshot snapshot;
        xui_doc_command_state_t state = {0};
        xui_doc_node_id list, item;
        xui_event_t pointer = {0};
        char bytes[64] = {0}; uint64_t length = 0;
        md.iMarkdownDialect = XUI_MD_GFM;
        CHECK(xuiDocumentCreate(&md, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, original, strlen(original)) == XUI_OK);
        memset(&editor_desc, 0, sizeof(editor_desc));
        editor_desc.iSize = sizeof(editor_desc);
        editor_desc.iMode = XUI_DOC_VISUAL;
        editor_desc.tView.iSize = sizeof(editor_desc.tView);
        editor_desc.tView.pDocument = document;
        CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK);
        render_view(context, target, &proxy, editor);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, 1, 0, &list) == XUI_OK &&
            xuiDocumentSnapshotGetChild(snapshot, list, 0, &item) == XUI_OK &&
            xuiDocumentSnapshotFind(snapshot, XUI_DOC_SEMANTIC,
                "portable", 8, NULL, &selection, 1, &length) == XUI_OK && length == 1);
        xuiDocumentSnapshotRelease(snapshot);
        selection.tAnchor = selection.tCaret;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        state.iSize = sizeof(state);
        CHECK(xuiDocumentEditorQueryCommand(editor, XUI_DOC_EDIT_TOGGLE_TASK,
            &state) == XUI_OK && state.bEnabled && !state.bActive);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_TOGGLE_TASK) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        pointer.iSize = sizeof(pointer); pointer.pTarget = editor;
        pointer.iPointerId = 7; pointer.iPointerType = XUI_POINTER_TYPE_MOUSE;
        pointer.iButton = XUI_POINTER_BUTTON_LEFT;
        pointer.fX = 8; pointer.fY = 12;
        pointer.iType = XUI_EVENT_POINTER_DOWN;
        pointer.iButtons = XUI_POINTER_BUTTON_LEFT;
        CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
        pointer.iType = XUI_EVENT_POINTER_UP; pointer.iButtons = 0;
        CHECK(xuiDispatchEvent(context, &pointer) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotCopySource(snapshot, bytes, sizeof(bytes), &length) == XUI_OK &&
            length == strlen(checked) && !memcmp(bytes, checked, (size_t)length));
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document, &snapshot) == XUI_OK &&
            xuiDocumentSnapshotCopySource(snapshot, bytes, sizeof(bytes), &length) == XUI_OK &&
            length == strlen(original) && !memcmp(bytes, original, (size_t)length));
        xuiDocumentSnapshotRelease(snapshot);
        CHECK(xuiDocumentEditorToggleTaskItem(editor, item) == XUI_OK);
        CHECK(xuiDocumentEditorExecute(editor, XUI_DOC_EDIT_UNDO) == XUI_OK);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    {
        const char* lines = "0000000000\nx\n0000000000\n";
        xui_event_t key = {0};
        CHECK(xuiDocumentCreate(&md, &document) == XUI_OK &&
            xuiDocumentLoadMarkdown(document, lines, strlen(lines)) == XUI_OK);
        editor_desc.tView.pDocument = document;
        editor_desc.iMode = XUI_DOC_SOURCE_TEXT;
        CHECK(xuiDocumentEditorCreate(context, &editor_desc, &editor) == XUI_OK);
        render_view(context, target, &proxy, editor);
        CHECK(xuiDocumentViewGetSelection(editor, &selection) == XUI_OK);
        selection.tAnchor.iOffset = selection.tCaret.iOffset = 8;
        CHECK(xuiDocumentViewSetSelection(editor, &selection) == XUI_OK);
        key.iSize = sizeof(key); key.iType = XUI_EVENT_KEY_DOWN;
        key.pTarget = editor; key.iKey = XUI_KEY_DOWN;
        CHECK(xuiDispatchEvent(context, &key) == XUI_OK &&
            xuiDispatchEvent(context, &key) == XUI_OK &&
            xuiDocumentViewGetSelection(editor, &selection) == XUI_OK &&
            selection.tCaret.iOffset >= 18 && selection.tCaret.iOffset <= 22);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }

    proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
    xuiDestroy(context);
    proxy.tProxy.fontDestroy(&proxy.tProxy, font);
    puts("Linux headless Rich/Markdown DocumentView and Editor visual/source/live input, task marker, vertical navigation, undo, hit and draw passed");
    return 0;
}
