/* Public format command contract and GFM strike flanking regressions. */
#include "../xui_document_ui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e) do { checks++; if (!(e)) { fprintf(stderr,"format command line %d: %s\n",__LINE__,#e); exit(1); } } while(0)
static unsigned checks;
static xui_doc_range_t find(xui_document document,const char* text)
{
    xui_document_snapshot snapshot = NULL; xui_doc_range_t range; uint64_t count;
    CHECK(xuiDocumentAcquireSnapshot(document,&snapshot) == XUI_OK);
    { int result = xuiDocumentSnapshotFind(snapshot,XUI_DOC_SEMANTIC,text,strlen(text),NULL,&range,1,&count);
      if (result != XUI_OK || !count) fprintf(stderr,"find [%s], profile=%u: result=%d count=%llu\n",text,xuiDocumentGetProfile(document),result,(unsigned long long)count);
      CHECK(result == XUI_OK && count >= 1); }
    xuiDocumentSnapshotRelease(snapshot); return range;
}
static void exact_source(xui_document document,const char* expected)
{
    xui_document_snapshot snapshot = NULL; char source[2048]; uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(document,&snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(snapshot,source,sizeof(source),&bytes) == XUI_OK);
    CHECK(bytes == strlen(expected) && !strcmp(source,expected)); xuiDocumentSnapshotRelease(snapshot);
}
static void marks(xui_document document,const char* text,uint32_t expected)
{
    xui_document_snapshot snapshot = NULL; xui_doc_range_t range = find(document,text); uint32_t common,mixed;
    CHECK(xuiDocumentAcquireSnapshot(document,&snapshot) == XUI_OK);
    CHECK(xuiDocumentSnapshotQueryMarks(snapshot,&range,&common,&mixed) == XUI_OK);
    CHECK(common == expected && !mixed); xuiDocumentSnapshotRelease(snapshot);
}
static xui_widget editor_create(xui_context context,xui_document document,uint32_t mode)
{
    xui_doc_editor_desc_t desc = {0}; xui_widget editor = NULL;
    desc.iSize = sizeof(desc); desc.tView.iSize = sizeof(desc.tView); desc.tView.pDocument = document; desc.iMode = mode;
    CHECK(xuiDocumentEditorCreate(context,&desc,&editor) == XUI_OK);
    CHECK(xuiSetRootWidget(context,editor) == XUI_OK);
    CHECK(xuiWidgetSetRect(editor,(xui_rect_t){0,0,600,400}) == XUI_OK); return editor;
}
static xui_doc_node_id rich_node(xui_document_transaction txn,xui_doc_node_id parent,uint32_t kind,const char* text)
{
    xui_doc_node_desc_t desc = {0}; xui_doc_node_id id;
    desc.iSize = sizeof(desc); desc.iKind = kind; desc.sText = text; desc.iTextBytes = text ? strlen(text) : 0;
    if (kind == XUI_DOC_IMAGE) desc.sResource = "local.image";
    CHECK(xuiDocumentTxnInsertNode(txn,parent,XUI_DOCUMENT_APPEND,&desc,&id) == XUI_OK); return id;
}
static void unavailable_payloads(xui_context context)
{
    const uint32_t commands[] = {XUI_DOC_EDIT_BOLD,XUI_DOC_EDIT_ITALIC,XUI_DOC_EDIT_UNDERLINE,XUI_DOC_EDIT_STRIKE,
        XUI_DOC_EDIT_CODE,XUI_DOC_EDIT_SUBSCRIPT,XUI_DOC_EDIT_SUPERSCRIPT,XUI_DOC_EDIT_HIGHLIGHT};
    for (unsigned profile = 0; profile < 4; profile++) {
        xui_doc_desc_t desc = {.iSize=sizeof(desc),.iProfile=XUI_DOCUMENT_MARKDOWN,
            .iMarkdownDialect=profile == 0 ? XUI_MD_COMMONMARK : profile == 1 ? XUI_MD_GFM : XUI_MD_EXTENDED};
        xui_document document = NULL; xui_widget editor; xui_document_snapshot saved = NULL; uint64_t revision;
        CHECK(xuiDocumentCreate(profile == 3 ? NULL : &desc,&document) == XUI_OK);
        if (profile == 3) {
            xui_document_transaction txn = NULL; xui_doc_node_id paragraph;
            CHECK(xuiDocumentBeginTransaction(document,NULL,&txn) == XUI_OK);
            paragraph = rich_node(txn,1,XUI_DOC_PARAGRAPH,NULL);
            rich_node(txn,paragraph,XUI_DOC_TEXT,"editable"); rich_node(txn,paragraph,XUI_DOC_IMAGE,"alt-field");
            rich_node(txn,1,XUI_DOC_CODE_BLOCK,"literal-payload");
            CHECK(xuiDocumentTxnCommit(txn,NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
        } else {
            const char* source = "editable ![alt-field](local.image)\n\n```c\nliteral-payload\n```\n";
            CHECK(xuiDocumentLoadMarkdown(document,source,strlen(source)) == XUI_OK);
        }
        CHECK(xuiDocumentAcquireSnapshot(document,&saved) == XUI_OK);
        CHECK(xuiDocumentMarkSaved(document,saved) == XUI_OK); xuiDocumentSnapshotRelease(saved);
        revision = xuiDocumentGetRevision(document); editor = editor_create(context,document,XUI_DOC_VISUAL);
        for (unsigned payload = 0; payload < 2; payload++) for (int caret = 0; caret < 2; caret++) {
            xui_doc_range_t selection = find(document,payload ? "literal-payload" : "alt-field");
            if (caret) selection.tAnchor = selection.tCaret;
            CHECK(xuiDocumentViewSetSelection(editor,&selection) == XUI_OK);
            for (unsigned c = 0; c < sizeof(commands)/sizeof(commands[0]); c++) {
                xui_doc_command_state_t state = {.iSize=sizeof(state)};
                CHECK(xuiDocumentEditorQueryCommand(editor,commands[c],&state) == XUI_OK && !state.bEnabled);
                CHECK(!xuiDocumentEditorCanExecute(editor,commands[c]));
                CHECK(xuiDocumentEditorExecute(editor,commands[c]) == state.iDisabledReason);
                CHECK(xuiDocumentGetRevision(document) == revision && !xuiDocumentIsDirty(document));
            }
            CHECK(xuiDocumentEditorSetMarks(editor,XUI_DOC_BOLD,0) == XUI_ERROR_UNSUPPORTED);
        }
        {
            xui_doc_range_t selection = find(document,"editable"); xui_doc_command_state_t state = {.iSize=sizeof(state)};
            CHECK(xuiDocumentViewSetSelection(editor,&selection) == XUI_OK);
            CHECK(xuiDocumentEditorQueryCommand(editor,XUI_DOC_EDIT_BOLD,&state) == XUI_OK && state.bEnabled);
            CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_BOLD) == XUI_OK); marks(document,"editable",XUI_DOC_BOLD);
            CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO) == XUI_OK && !xuiDocumentIsDirty(document));
            CHECK(xuiDocumentEditorSetReadOnly(editor,1) == XUI_OK);
            CHECK(xuiDocumentEditorQueryCommand(editor,XUI_DOC_EDIT_BOLD,&state) == XUI_OK && !state.bEnabled);
        }
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
    /* Empty roots/paragraph gaps still support pending style and insertion. */
    for (int paragraph = 0; paragraph < 2; paragraph++) {
        xui_document document = NULL; xui_widget editor; xui_doc_command_state_t state = {.iSize=sizeof(state)};
        CHECK(xuiDocumentCreate(NULL,&document) == XUI_OK);
        if (paragraph) {
            xui_document_transaction txn = NULL; CHECK(xuiDocumentBeginTransaction(document,NULL,&txn) == XUI_OK);
            rich_node(txn,1,XUI_DOC_PARAGRAPH,NULL); CHECK(xuiDocumentTxnCommit(txn,NULL) == XUI_OK); xuiDocumentTxnRelease(txn);
        }
        editor = editor_create(context,document,XUI_DOC_VISUAL);
        CHECK(xuiDocumentEditorQueryCommand(editor,XUI_DOC_EDIT_BOLD,&state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_BOLD) == XUI_OK);
        CHECK(xuiDocumentEditorInsertText(editor,"typed",5) == XUI_OK); marks(document,"typed",XUI_DOC_BOLD);
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
}
static void strike_flanking(void)
{
    const struct { const char* source; const char* match; uint32_t marks; } cases[] = {
        {"x~~a~~nd\n","a",XUI_DOC_STRIKE}, {"x~a~nd\n","a",XUI_DOC_STRIKE},
        {"中~~文~~本\n","文",XUI_DOC_STRIKE}, {"(~~!~~)\n","!",XUI_DOC_STRIKE},
        {"x~~!~~y\n","!",0}, {"~~ space~~\n","space",0}, {"~~space ~~\n","space",0},
        {"x~~~plain~~~y\n","plain",0}, {"~~Hi~~ Hello, ~there~ world!\n","there",XUI_DOC_STRIKE},
        {"This ~~has a\n\nnew paragraph~~.\n","has",0},
        {"~~first\nsecond~~\n","second",XUI_DOC_STRIKE}, {"\\~~escaped~~\n","escaped",0}
    };
    for (unsigned c = 0; c < sizeof(cases)/sizeof(cases[0]); c++) {
        xui_doc_desc_t desc = {.iSize=sizeof(desc),.iProfile=XUI_DOCUMENT_MARKDOWN,.iMarkdownDialect=XUI_MD_GFM}; xui_document document = NULL;
        CHECK(xuiDocumentCreate(&desc,&document) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(document,cases[c].source,strlen(cases[c].source)) == XUI_OK);
        marks(document,cases[c].match,cases[c].marks); exact_source(document,cases[c].source); xuiDocumentRelease(document);
    }
    {
        xui_doc_desc_t desc = {.iSize=sizeof(desc),.iProfile=XUI_DOCUMENT_MARKDOWN,.iMarkdownDialect=XUI_MD_EXTENDED}; xui_document document = NULL;
        CHECK(xuiDocumentCreate(&desc,&document) == XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(document,"x~q~r\n",6) == XUI_OK); marks(document,"q",XUI_DOC_SUBSCRIPT);
        exact_source(document,"x~q~r\n"); xuiDocumentRelease(document);
    }
}
static void letter_edit(xui_context context)
{
    const char* initial = "before `code` and **bold** [link](url).\n\n```c\nliteral\n```\n";
    const char* expected = "before `code` ~~a~~nd **bold** [link](url).\n\n```c\nliteral\n```\n";
    for (int reverse = 0; reverse < 2; reverse++) {
        xui_doc_desc_t desc = {.iSize=sizeof(desc),.iProfile=XUI_DOCUMENT_MARKDOWN,.iMarkdownDialect=XUI_MD_GFM};
        xui_document document = NULL; xui_widget editor; xui_doc_range_t selection; xui_doc_command_state_t state = {.iSize=sizeof(state)};
        CHECK(xuiDocumentCreate(&desc,&document) == XUI_OK); CHECK(xuiDocumentLoadMarkdown(document,initial,strlen(initial)) == XUI_OK);
        editor = editor_create(context,document,XUI_DOC_VISUAL); selection = find(document,"and"); selection.tCaret.iOffset = selection.tAnchor.iOffset + 1;
        if (reverse) { xui_doc_position_t swap=selection.tAnchor;selection.tAnchor=selection.tCaret;selection.tCaret=swap; }
        CHECK(xuiDocumentViewSetSelection(editor,&selection) == XUI_OK);
        CHECK(xuiDocumentEditorQueryCommand(editor,XUI_DOC_EDIT_STRIKE,&state) == XUI_OK && state.bEnabled);
        CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_STRIKE) == XUI_OK); exact_source(document,expected); marks(document,"a",XUI_DOC_STRIKE);
        CHECK(xuiDocumentEditorQueryCommand(editor,XUI_DOC_EDIT_STRIKE,&state) == XUI_OK && state.bActive);
        CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO) == XUI_OK); exact_source(document,initial);
        CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_REDO) == XUI_OK); exact_source(document,expected);
        for (int mode = XUI_DOC_SOURCE_TEXT; mode <= XUI_DOC_LIVE_MARKDOWN; mode++) {
            CHECK(xuiDocumentViewSetMode(editor,mode) == XUI_OK);
            CHECK(xuiDocumentEditorQueryCommand(editor,XUI_DOC_EDIT_BOLD,&state) == XUI_OK && !state.bEnabled);
            exact_source(document,expected);
        }
        xuiWidgetDestroy(editor); xuiDocumentRelease(document);
    }
}
typedef struct fail_allocator { long remaining; unsigned live; } fail_allocator;
static void* failing_alloc(void* user, size_t bytes)
{
    fail_allocator* allocator = user; void* pointer;
    if (!allocator->remaining) return NULL;
    if (allocator->remaining > 0) allocator->remaining--;
    pointer = malloc(bytes); if (pointer) allocator->live++; return pointer;
}
static void failing_free(void* user, void* pointer)
{
    fail_allocator* allocator = user;
    CHECK(allocator->live > 0); allocator->live--; free(pointer);
}
static void count_change(xui_document document, xui_document_change_set change, void* user)
{
    (void)document; (void)change; (*(unsigned*)user)++;
}
static void marks_atomicity(void)
{
    const struct { const char* source; const char* selected; const char* expected; uint32_t mark; } cases[] = {
        {"and\n","a","~~a~~nd\n",XUI_DOC_STRIKE},
        {"left a`b right\n","a`b","left ``a`b`` right\n",XUI_DOC_CODE}
    };
    for (unsigned c = 0; c < sizeof(cases)/sizeof(cases[0]); c++) {
        int success = 0; long point;
        for (point = 0; point < 1024 && !success; point++) {
            fail_allocator allocator = {-1,0}; unsigned changes = 0; uint64_t revision,token;
            xui_doc_desc_t desc = {.iSize=sizeof(desc),.iProfile=XUI_DOCUMENT_MARKDOWN,.iMarkdownDialect=XUI_MD_GFM,
                .onAlloc=failing_alloc,.onFree=failing_free,.pAllocatorUser=&allocator};
            xui_document document = NULL; xui_document_snapshot before = NULL; xui_document_transaction txn = NULL;
            xui_doc_range_t range; int result;
            CHECK(xuiDocumentCreate(&desc,&document) == XUI_OK);
            CHECK(xuiDocumentLoadMarkdown(document,cases[c].source,strlen(cases[c].source)) == XUI_OK);
            CHECK(xuiDocumentClearHistory(document) == XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(document,&before) == XUI_OK);
            CHECK(xuiDocumentMarkSaved(document,before) == XUI_OK);
            CHECK(xuiDocumentSubscribe(document,count_change,&changes,&token) == XUI_OK);
            range = find(document,cases[c].selected); revision = xuiDocumentGetRevision(document);
            allocator.remaining = point;
            result = xuiDocumentBeginTransaction(document,NULL,&txn);
            if (result == XUI_OK) result = xuiDocumentTxnSetMarks(txn,&range,cases[c].mark,0);
            if (result == XUI_OK) result = xuiDocumentTxnCommit(txn,NULL);
            xuiDocumentTxnRelease(txn); allocator.remaining = -1;
            if (result != XUI_OK) {
                if (result != XUI_ERROR_OUT_OF_MEMORY) fprintf(stderr,"marks atomic case %u point %ld: result %d\n",c,point,result);
                CHECK(result == XUI_ERROR_OUT_OF_MEMORY);
                CHECK(xuiDocumentGetRevision(document) == revision && !xuiDocumentIsDirty(document));
                CHECK(!xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document) && !changes);
                exact_source(document,cases[c].source); marks(document,cases[c].selected,0);
            } else {
                success = 1;
                CHECK(xuiDocumentGetRevision(document) == revision + 1 && xuiDocumentIsDirty(document) && changes == 1);
                CHECK(xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
                exact_source(document,cases[c].expected); marks(document,cases[c].selected,cases[c].mark);
                CHECK(xuiDocumentUndo(document,NULL) == XUI_OK && !xuiDocumentIsDirty(document) && changes == 2);
                exact_source(document,cases[c].source);
                CHECK(!xuiDocumentCanUndo(document) && xuiDocumentCanRedo(document));
                CHECK(xuiDocumentRedo(document,NULL) == XUI_OK && xuiDocumentIsDirty(document) && changes == 3);
                exact_source(document,cases[c].expected); marks(document,cases[c].selected,cases[c].mark);
            }
            { char retained[64]; uint64_t bytes;
              CHECK(xuiDocumentSnapshotCopySource(before,retained,sizeof(retained),&bytes) == XUI_OK);
              CHECK(bytes == strlen(cases[c].source) && !strcmp(retained,cases[c].source)); }
            xuiDocumentUnsubscribe(document,token);
            xuiDocumentSnapshotRelease(before); xuiDocumentRelease(document); CHECK(!allocator.live);
        }
        CHECK(success); printf("Document format atomicity case %u: %ld allocation points passed.\n",c,point);
    }
}
int main(void)
{
    xui_test_proxy_state_t proxy; xui_context context = NULL;
    xuiTestProxyInit(&proxy); CHECK(xuiCreate(&context) == XUI_OK); CHECK(xuiSetProxy(context,&proxy.tProxy) == XUI_OK);
    unavailable_payloads(context); strike_flanking(); letter_edit(context); xuiDestroy(context); marks_atomicity();
    printf("Document format commands: payload/caret capability, empty gap insertion, GFM flanking, exact source, Undo/Redo, mode preservation and allocation atomicity passed (%u checks).\n",checks); return 0;
}
