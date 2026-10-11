/* Public deletion APIs must preserve literal boundary whitespace and source. */
#include "../xui_document_ui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif
#define CHECK(e) do { checks++; if (!(e)) { fprintf(stderr,"boundary delete line %d case %u: %s\n",__LINE__,current_case,#e); exit(1); } } while(0)
static unsigned checks, current_case;
typedef struct deletion_case { const char *source, *selected, *expected, *remaining; } deletion_case;
static const deletion_case cases[] = {
    {"keep 中文 😀 tail\n","keep 中文 😀","&#32;tail\n"," tail"},
    {"> keep tail\r\n","keep","> &#32;tail\r\n"," tail"},
    {"- keep tail\n","keep","- &#32;tail\n"," tail"},
    {"## keep tail ##\r\n","keep","## &#32;tail ##\r\n"," tail"},
    {"**keep tail**\n","keep","**&#32;tail**\n"," tail"},
    {"[keep tail](local.path \"title\")\n","keep","[ tail](local.path \"title\")\n"," tail"},
    {"[keep tail][KEY]\r\n\r\n[KEY]: <local.path> 'title'\r\n","keep","[ tail][KEY]\r\n\r\n[KEY]: <local.path> 'title'\r\n"," tail"},
    {"keep    tail\n","keep","&#32;   tail\n","    tail"},
    {"keep&#32;tail\n","keep","&#32;tail\n"," tail"},
    {"before keep tail\n","keep","before  tail\n","before  tail"},
    {"`keep tail`\n","keep","` tail`\n"," tail"},
    {"\xef\xbb\xbfkeep tail\r\n\r\n[unused]: <untouched> 'title'\r\n","keep","\xef\xbb\xbf&#32;tail\r\n\r\n[unused]: <untouched> 'title'\r\n"," tail"},
    {"keep\ttail\n","keep","&#9;tail\n","\ttail"},
    {"head keep\n","keep","head&#32;\n","head "},
    {"head  keep\n","keep","head &#32;\n","head  "},
    {"head\tkeep\n","keep","head&#9;\n","head\t"},
    {"**keep\ttail**\n","keep","**&#9;tail**\n","\ttail"},
    {"**head keep**\n","keep","**head&#32;**\n","head "},
    {"[head keep](url)\n","keep","[head ](url)\n","head "},
    {"**keep tail**\n","keep","**&#160;tail**\n"," tail"},
    {"**keep　tail**\n","keep","**&#12288;tail**\n","　tail"},
    {"**head keep**\n","keep","**head&#160;**\n","head "},
    {"before **keep tail** after\n","keep","before **&#32;tail** after\n"," tail"},
    {"before **head keep** after\n","keep","before **head&#32;** after\n","head "},
    {"> head keep\r\n","keep","> head&#32;\r\n","head "},
    {"- head keep\n","keep","- head&#32;\n","head "},
    {"## head keep ##\r\n","keep","## head&#32; ##\r\n","head "},
    {"keep tail\n","keep"," tail\n"," tail"},
    {"head&#32;keep\n","keep","head&#32;\n","head "},
    {"`head keep`\n","keep","`head `\n","head "},
    {"[keep tail]\n\n[keep tail]: <local.path> 'title'\n","keep","[ tail][keep tail]\n\n[keep tail]: <local.path> 'title'\n"," tail"},
    {"[**keep tail**]\r\n\r\n[**keep tail**]: <local.path> 'title'\r\n","keep","[**&#32;tail**][**keep tail**]\r\n\r\n[**keep tail**]: <local.path> 'title'\r\n"," tail"},
    {"[**keep tail**][KEY]\n\n[KEY]: <local.path> 'title'\n","keep","[**&#32;tail**][KEY]\n\n[KEY]: <local.path> 'title'\n"," tail"}
};
static xui_doc_range_t find(xui_document document,const char* text)
{
    xui_document_snapshot snapshot=NULL; xui_doc_range_t range; uint64_t count;
    CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    CHECK(xuiDocumentSnapshotFind(snapshot,XUI_DOC_SEMANTIC,text,strlen(text),NULL,&range,1,&count)==XUI_OK && count==1);
    xuiDocumentSnapshotRelease(snapshot); return range;
}
static void source_is(xui_document_snapshot snapshot,const char* expected)
{
    char buffer[1024]; uint64_t bytes;
    CHECK(xuiDocumentSnapshotCopySource(snapshot,buffer,sizeof(buffer),&bytes)==XUI_OK);
    if (bytes!=strlen(expected) || strcmp(buffer,expected)) fprintf(stderr,"actual source [%s], expected [%s]\n",buffer,expected);
    CHECK(bytes==strlen(expected) && !strcmp(buffer,expected));
}
static void document_is(xui_document document,const char* source,const char* remaining)
{
    xui_document_snapshot snapshot=NULL; xui_doc_range_t range=find(document,remaining); char* text=NULL; uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(document,&snapshot)==XUI_OK);
    source_is(snapshot,source);
    CHECK(xuiDocumentSnapshotCopyRange(snapshot,&range,&text,&bytes)==XUI_OK);
    CHECK(bytes==strlen(remaining) && !memcmp(text,remaining,(size_t)bytes));
    xuiDocumentFreeBuffer(text); xuiDocumentSnapshotRelease(snapshot);
}
static void text_before_caret(xui_document_snapshot snapshot,xui_doc_position_t caret,char** text,uint64_t* bytes)
{
    xui_doc_range_t prefix; xui_doc_node_info_t node={.iSize=sizeof(node)};
    CHECK(xuiDocumentSnapshotGetNode(snapshot,caret.iNodeId,&node)==XUI_OK);
    prefix.tCaret=caret;prefix.tAnchor=caret;
    prefix.tAnchor.iNodeId=caret.iKind==XUI_DOC_POSITION_TEXT ? node.iParentId : node.iId;
    prefix.tAnchor.iKind=XUI_DOC_POSITION_GAP;prefix.tAnchor.iOffset=0;prefix.tAnchor.iAffinity=XUI_DOC_BEFORE;
    CHECK(xuiDocumentSnapshotCopyRange(snapshot,&prefix,text,bytes)==XUI_OK);
}
static void perform_delete(xui_document document,const xui_doc_range_t* range,unsigned api)
{
    xui_document_transaction txn=NULL; xui_doc_position_t caret;
    CHECK(xuiDocumentBeginTransaction(document,NULL,&txn)==XUI_OK);
    if (api==1) CHECK(xuiDocumentTxnReplaceRange(txn,range,"",0,&caret)==XUI_OK);
    else {
        uint64_t a=range->tAnchor.iOffset,b=range->tCaret.iOffset;
        CHECK(range->tAnchor.iNodeId==range->tCaret.iNodeId);
        CHECK(xuiDocumentTxnReplaceText(txn,range->tAnchor.iNodeId,a<b?a:b,a<b?b:a,"",0)==XUI_OK);
    }
    CHECK(xuiDocumentTxnCommit(txn,NULL)==XUI_OK); xuiDocumentTxnRelease(txn);
}
static void deletion_matrix(void)
{
    for (unsigned c=0;c<sizeof(cases)/sizeof(cases[0]);c++)
    for (unsigned dialect=XUI_MD_COMMONMARK;dialect<=XUI_MD_EXTENDED;dialect++)
    for (unsigned api=0;api<3;api++) for (int reverse=0;reverse<2;reverse++) {
        xui_doc_desc_t desc={.iSize=sizeof(desc),.iProfile=XUI_DOCUMENT_MARKDOWN,.iMarkdownDialect=dialect};
        xui_document document=NULL; xui_document_snapshot before=NULL; xui_doc_range_t range;
        xui_context context=NULL; xui_widget editor=NULL; xui_test_proxy_state_t proxy; uint64_t revision;
        current_case=c;
        CHECK(xuiDocumentCreate(&desc,&document)==XUI_OK);
        CHECK(xuiDocumentLoadMarkdown(document,cases[c].source,strlen(cases[c].source))==XUI_OK);
        CHECK(xuiDocumentClearHistory(document)==XUI_OK);
        CHECK(xuiDocumentAcquireSnapshot(document,&before)==XUI_OK);
        CHECK(xuiDocumentMarkSaved(document,before)==XUI_OK);
        range=find(document,cases[c].selected); revision=xuiDocumentGetRevision(document);
        if (reverse) { xui_doc_position_t swap=range.tAnchor;range.tAnchor=range.tCaret;range.tCaret=swap; }
        if (!api) {
            xui_doc_editor_desc_t editor_desc={0}; xui_doc_command_state_t state={.iSize=sizeof(state)};
            xuiTestProxyInit(&proxy);
            CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy.tProxy)==XUI_OK);
            editor_desc.iSize=sizeof(editor_desc);editor_desc.tView.iSize=sizeof(editor_desc.tView);
            editor_desc.tView.pDocument=document;editor_desc.iMode=XUI_DOC_VISUAL;
            CHECK(xuiDocumentEditorCreate(context,&editor_desc,&editor)==XUI_OK);
            CHECK(xuiSetRootWidget(context,editor)==XUI_OK);
            CHECK(xuiDocumentViewSetSelection(editor,&range)==XUI_OK);
            CHECK(xuiDocumentEditorQueryCommand(editor,XUI_DOC_EDIT_CUT,&state)==XUI_OK && state.bEnabled);
            CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_CUT)==XUI_OK);
            CHECK(!strcmp(xuiTestProxyGetClipboardText(&proxy),cases[c].selected));
            {
                xui_doc_range_t actual; xui_document_snapshot committed=NULL; int order;
                xui_doc_position_t original=reverse ? range.tCaret : range.tAnchor;
                char *expected_prefix=NULL,*actual_prefix=NULL;uint64_t expected_bytes,actual_bytes;
                CHECK(xuiDocumentViewGetSelection(editor,&actual)==XUI_OK);
                CHECK(xuiDocumentAcquireSnapshot(document,&committed)==XUI_OK);
                CHECK(xuiDocumentSnapshotComparePositions(committed,&actual.tAnchor,&actual.tCaret,&order)==XUI_OK && !order);
                /* Entities can have separate Text nodes. Compare the complete
                 * unchanged semantic prefix, not node-local byte offsets. */
                text_before_caret(before,original,&expected_prefix,&expected_bytes);
                text_before_caret(committed,actual.tCaret,&actual_prefix,&actual_bytes);
                CHECK(actual_bytes==expected_bytes && !memcmp(actual_prefix,expected_prefix,(size_t)actual_bytes));
                xuiDocumentFreeBuffer(expected_prefix);xuiDocumentFreeBuffer(actual_prefix);
                xuiDocumentSnapshotRelease(committed);
            }
        } else perform_delete(document,&range,api);
        CHECK(xuiDocumentGetRevision(document)==revision+1 && xuiDocumentIsDirty(document));
        CHECK(xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document));
        document_is(document,cases[c].expected,cases[c].remaining);
        CHECK(xuiDocumentUndo(document,NULL)==XUI_OK && !xuiDocumentIsDirty(document));
        document_is(document,cases[c].source,cases[c].selected);
        CHECK(!xuiDocumentCanUndo(document) && xuiDocumentCanRedo(document));
        CHECK(xuiDocumentRedo(document,NULL)==XUI_OK && xuiDocumentIsDirty(document));
        document_is(document,cases[c].expected,cases[c].remaining);
        if (editor) for (unsigned mode=XUI_DOC_SOURCE_TEXT;mode<=XUI_DOC_LIVE_MARKDOWN;mode++) {
            CHECK(xuiDocumentViewSetMode(editor,mode)==XUI_OK);
            document_is(document,cases[c].expected,cases[c].remaining);
        }
        source_is(before,cases[c].source); xuiDocumentSnapshotRelease(before);
        if (context) xuiDestroy(context);
        xuiDocumentRelease(document);
    }
}
typedef struct fail_allocator { long remaining; unsigned live; } fail_allocator;
static void* failing_alloc(void* user,size_t bytes)
{
    fail_allocator* a=user;void* pointer;
    if (!a->remaining) return NULL;
    if (a->remaining>0) a->remaining--;
    pointer=malloc(bytes);if(pointer)a->live++;return pointer;
}
static void failing_free(void* user,void* pointer)
{
    fail_allocator* a=user;CHECK(a->live>0);a->live--;free(pointer);
}
static void count_change(xui_document document,xui_document_change_set change,void* user)
{
    (void)document;(void)change;(*(unsigned*)user)++;
}
static void deletion_atomicity(void)
{
    const unsigned fault_cases[]={0,13,19,31};
    for (unsigned f=0;f<sizeof(fault_cases)/sizeof(fault_cases[0]);f++)
    for (unsigned api=1;api<=2;api++) {
        const deletion_case* test=&cases[fault_cases[f]];
        int success=0;long point;
        for(point=0;point<1024 && !success;point++) {
            fail_allocator a={-1,0};unsigned changes=0;uint64_t token,revision;
            xui_doc_desc_t desc={.iSize=sizeof(desc),.iProfile=XUI_DOCUMENT_MARKDOWN,.iMarkdownDialect=XUI_MD_GFM,
                .onAlloc=failing_alloc,.onFree=failing_free,.pAllocatorUser=&a};
            xui_document document=NULL;xui_document_snapshot before=NULL;xui_document_transaction txn=NULL;
            xui_doc_range_t range;xui_doc_position_t caret;int result;
            current_case=fault_cases[f];
            CHECK(xuiDocumentCreate(&desc,&document)==XUI_OK);
            CHECK(xuiDocumentLoadMarkdown(document,test->source,strlen(test->source))==XUI_OK);
            CHECK(xuiDocumentClearHistory(document)==XUI_OK);
            CHECK(xuiDocumentAcquireSnapshot(document,&before)==XUI_OK && xuiDocumentMarkSaved(document,before)==XUI_OK);
            CHECK(xuiDocumentSubscribe(document,count_change,&changes,&token)==XUI_OK);
            range=find(document,test->selected);revision=xuiDocumentGetRevision(document);a.remaining=point;
            result=xuiDocumentBeginTransaction(document,NULL,&txn);
            if(result==XUI_OK) result=api==1 ? xuiDocumentTxnReplaceRange(txn,&range,"",0,&caret) :
                xuiDocumentTxnReplaceText(txn,range.tAnchor.iNodeId,range.tAnchor.iOffset,range.tCaret.iOffset,"",0);
            if(result==XUI_OK)result=xuiDocumentTxnCommit(txn,NULL);
            xuiDocumentTxnRelease(txn);a.remaining=-1;
            if(result!=XUI_OK) {
                CHECK(result==XUI_ERROR_OUT_OF_MEMORY);
                CHECK(xuiDocumentGetRevision(document)==revision && !xuiDocumentIsDirty(document));
                CHECK(!xuiDocumentCanUndo(document) && !xuiDocumentCanRedo(document) && !changes);
                document_is(document,test->source,test->selected);
            } else {
                success=1;CHECK(changes==1 && xuiDocumentGetRevision(document)==revision+1);
                document_is(document,test->expected,test->remaining);
                CHECK(xuiDocumentUndo(document,NULL)==XUI_OK && !xuiDocumentIsDirty(document) && changes==2);
                document_is(document,test->source,test->selected);
                CHECK(xuiDocumentRedo(document,NULL)==XUI_OK && changes==3);
                document_is(document,test->expected,test->remaining);
            }
            source_is(before,test->source);xuiDocumentUnsubscribe(document,token);
            xuiDocumentSnapshotRelease(before);xuiDocumentRelease(document);CHECK(!a.live);
        }
        CHECK(success);printf("Document boundary deletion case %u API %u: %ld allocation points passed.\n",fault_cases[f],api,point);
    }
}
int main(void)
{
#ifdef _WIN32
    { char path[32768]; CHECK(GetModuleFileNameA(GetModuleHandleA("xge.dll"),path,sizeof(path))>0);
      printf("Document boundary deletion runtime: %s\n",path); fflush(stdout); }
#endif
    deletion_matrix();deletion_atomicity();
    printf("Document boundary deletion: %u sources x 3 dialects x 3 APIs x 2 selection directions, exact source/text, collapsed cut caret, retained snapshots, Undo/Redo, mode preservation and allocation atomicity passed (%u checks).\n",(unsigned)(sizeof(cases)/sizeof(cases[0])),checks);
    return 0;
}
