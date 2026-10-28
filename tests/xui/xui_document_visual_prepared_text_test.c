#include "xui_document_ui.h"
#include "../src/xui/accessibility/xui_accessibility_text_internal.h"
#include "xui_test_proxy.h"
#include "xui_test_time.h"
#include "../extlibs/xrt/xrt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"Visual prepared range line %d: %s\n",__LINE__,#c); exit(1); } } while (0)
static void text(xui_internal_accessible_text_range* range,const char* expected)
{
    char value[128];uint64_t bytes;
    CHECK(range->ops->getText(range,value,sizeof(value),&bytes)==XUI_OK);
    if(strcmp(value,expected))fprintf(stderr,"Expected [%s], got [%s]\n",expected,value);
    CHECK(bytes==strlen(expected)&&!strcmp(value,expected));
}
typedef struct visual_allocator {
    uint64_t owner;
    atomic_uint_fast64_t calls, live, fail_at;
    atomic_int gate, entered, resume, fail_worker;
} visual_allocator;
static void wait_flag(atomic_int* flag)
{
    xui_test_deadline_t deadline=xuiTestDeadlineAfterUs(30000000);
    while(!atomic_load(flag)){CHECK(!xuiTestDeadlineExpired(deadline));xrtThreadYield();}
}
static void* allocate(void* user,size_t bytes)
{
    visual_allocator* a=user;void* p;
    if(xrtThreadCurrentId()==a->owner){
        uint64_t call=atomic_fetch_add(&a->calls,1)+1;
        if(call==atomic_load(&a->fail_at))return NULL;
    }else{
        if(atomic_exchange(&a->gate,0)){atomic_store(&a->entered,1);wait_flag(&a->resume);}
        if(atomic_load(&a->fail_worker))return NULL;
    }
    p=malloc(bytes);if(p)atomic_fetch_add(&a->live,1);return p;
}
static void deallocate(void* user,void* p)
{visual_allocator* a=user;if(p){CHECK(atomic_fetch_sub(&a->live,1));free(p);}}
typedef struct visual_fixture {
    xui_context context;xui_test_proxy_state_t proxy;xui_font font;
    xui_document document;xui_widget editor;visual_allocator allocator;
    xui_internal_accessible_text_range *whole,*retained;
    uint64_t generation,first,last;int observe,callbacks;
} visual_fixture;
static void offsets(xui_internal_accessible_text_range* range,uint64_t a,uint64_t b)
{
    uint64_t first,last,length;
    CHECK(range->ops->getOffsets(range,&first,&last,&length)==XUI_OK);
    if(first!=a||last!=b)fprintf(stderr,"Expected offsets %llu..%llu, got %llu..%llu\n",
        (unsigned long long)a,(unsigned long long)b,(unsigned long long)first,(unsigned long long)last);
    CHECK(first==a&&last==b&&b<=length);
}
static void published(xui_document document,xui_document_change_set change,void* user)
{
    visual_fixture* f=user;xui_doc_change_info_t info={0};xui_doc_prepare_info_t pending={0};
    (void)document;
    if(!f->observe)return;
    info.iSize=sizeof(info);CHECK(xuiDocumentChangeSetGetInfo(change,&info)==XUI_OK);
    pending.iSize=sizeof(pending);CHECK(xuiDocumentEditorGetPendingInput(f->editor,&pending)==XUI_OK);
    CHECK(pending.iState==XUI_DOC_PREPARE_PUBLISHING&&info.iInputGeneration==f->generation);
    text(f->retained,"cd");offsets(f->retained,f->first,f->last);f->callbacks++;
}
static xui_doc_prepare_info_t pending(visual_fixture* f)
{
    xui_doc_prepare_info_t info={0};info.iSize=sizeof(info);
    CHECK(xuiDocumentEditorGetPendingInput(f->editor,&info)==XUI_OK);return info;
}
static int flush(visual_fixture* f)
{
    xui_test_deadline_t deadline=xuiTestDeadlineAfterUs(30000000);int result;
    do{result=xuiDocumentEditorFlush(f->editor);CHECK(!xuiTestDeadlineExpired(deadline));xrtThreadYield();}
    while(result==XUI_DOC_ERROR_BUSY);return result;
}
static void source(visual_fixture* f,const char* expected)
{
    xui_document_snapshot snapshot;char buffer[256];uint64_t bytes;
    CHECK(xuiDocumentAcquireSnapshot(f->document,&snapshot)==XUI_OK);
    CHECK(xuiDocumentSnapshotCopySource(snapshot,buffer,sizeof(buffer),&bytes)==XUI_OK);
    CHECK(bytes==strlen(expected)&&!strcmp(buffer,expected));xuiDocumentSnapshotRelease(snapshot);
}
static void start(visual_fixture* f,const char* markdown)
{
    xui_doc_desc_t desc={0};xui_doc_editor_desc_t ed={0};uint64_t subscription;
    xui_internal_accessible_text_request request={XUI_INTERNAL_TEXT_DOCUMENT,0,0,0};
    xui_accessible_node_t root={0};memset(f,0,sizeof(*f));
    f->allocator.owner=xrtThreadCurrentId();atomic_init(&f->allocator.calls,0);atomic_init(&f->allocator.live,0);
    atomic_init(&f->allocator.fail_at,UINT64_MAX);atomic_init(&f->allocator.gate,0);
    atomic_init(&f->allocator.entered,0);atomic_init(&f->allocator.resume,0);atomic_init(&f->allocator.fail_worker,0);
    xuiTestProxyInit(&f->proxy);CHECK(xuiCreate(&f->context)==XUI_OK);
    CHECK(xuiSetProxy(f->context,&f->proxy.tProxy)==XUI_OK);
    CHECK(xuiSetViewportSize(f->context,800,600)==XUI_OK);
    CHECK(f->proxy.tProxy.fontLoadFile(&f->proxy.tProxy,&f->font,"test.ttf",14,0)==XUI_OK);
    CHECK(xuiSetDefaultFont(f->context,f->font)==XUI_OK);
    desc.iSize=sizeof(desc);desc.iProfile=XUI_DOCUMENT_MARKDOWN;
    desc.onAlloc=allocate;desc.onFree=deallocate;desc.pAllocatorUser=&f->allocator;
    CHECK(xuiDocumentCreate(&desc,&f->document)==XUI_OK);
    CHECK(xuiDocumentLoadMarkdown(f->document,markdown,strlen(markdown))==XUI_OK);
    CHECK(xuiDocumentClearHistory(f->document)==XUI_OK);
    CHECK(xuiDocumentSubscribe(f->document,published,f,&subscription)==XUI_OK);
    ed.iSize=sizeof(ed);ed.iMode=XUI_DOC_VISUAL;ed.iAsyncSourceThresholdBytes=1;
    ed.tView.iSize=sizeof(ed.tView);ed.tView.pDocument=f->document;
    CHECK(xuiDocumentEditorCreate(f->context,&ed,&f->editor)==XUI_OK);
    CHECK(xuiSetRootWidget(f->context,f->editor)==XUI_OK);
    CHECK(xuiWidgetSetRect(f->editor,(xui_rect_t){0,0,300,200})==XUI_OK);
    CHECK(xuiLayout(f->context)==XUI_OK);
    root.iSize=sizeof(root);CHECK(xuiWidgetGetAccessibleNode(f->editor,0,&root)==XUI_OK);
    CHECK(xuiInternalAccessibilityCreateTextRange(f->editor,root.iId,&request,&f->whole)==XUI_OK);
}
static void finish(visual_fixture* f)
{
    if(f->retained)f->retained->ops->release(f->retained);
    f->whole->ops->release(f->whole);xuiDestroy(f->context);xuiDocumentRelease(f->document);
    CHECK(!atomic_load(&f->allocator.live));
}
static void basic(void)
{
    visual_fixture f;xui_internal_accessible_text_range *range,*caret,*found,*format;
    uint64_t calls;unsigned i;start(&f,"abcd\n");
    CHECK(f.whole->ops->clone(f.whole,&range)==XUI_OK);f.retained=range;
    CHECK(range->ops->setOffsets(range,2,4)==XUI_OK);text(range,"cd");
    CHECK(f.whole->ops->clone(f.whole,&caret)==XUI_OK);
    CHECK(caret->ops->setOffsets(caret,1,1)==XUI_OK);CHECK(caret->ops->select(caret)==XUI_OK);
    CHECK(xuiDocumentEditorInsertText(f.editor,"X",1)==XUI_OK);
    CHECK(pending(&f).iGeneration&&pending(&f).iState==XUI_DOC_PREPARE_QUEUED);
    text(f.whole,"aXbcd\n");text(range,"cd");offsets(range,3,5);source(&f,"abcd\n");
    CHECK(f.whole->ops->findText(f.whole,"cd",2,0,0,&found)==XUI_OK&&found);
    text(found,"cd");offsets(found,3,5);
    CHECK(f.whole->ops->clone(f.whole,&format)==XUI_OK);
    CHECK(format->ops->setOffsets(format,0,0)==XUI_OK);
    CHECK(format->ops->expand(format,XUI_INTERNAL_TEXT_FORMAT)==XUI_OK);
    text(format,"aXbcd\n");offsets(format,0,6);
    atomic_store(&f.allocator.gate,1);
    CHECK(xuiDocumentEditorFlush(f.editor)==XUI_DOC_ERROR_BUSY);wait_flag(&f.allocator.entered);
    CHECK(pending(&f).iState==XUI_DOC_PREPARE_RUNNING);
    calls=atomic_load(&f.allocator.calls);
    for(i=0;i<5000;i++){
        text(range,"cd");offsets(range,3,5);
        CHECK(format->ops->setOffsets(format,0,0)==XUI_OK);
        CHECK(format->ops->expand(format,XUI_INTERNAL_TEXT_FORMAT)==XUI_OK);
        offsets(format,0,6);
    }
    CHECK(atomic_load(&f.allocator.calls)==calls);
    {xui_doc_range_t same;xui_doc_prepare_info_t before=pending(&f);
     CHECK(xuiDocumentViewGetSelection(f.editor,&same)==XUI_OK);
     same.tAnchor.iOffset=1;same.tCaret.iOffset=2;
     CHECK(xuiDocumentViewSetSelection(f.editor,&same)==XUI_OK);
     CHECK(xuiDocumentEditorInsertText(f.editor,"X",1)==XUI_OK);
     CHECK(pending(&f).iGeneration!=before.iGeneration);text(f.whole,"aXbcd\n");text(range,"cd");offsets(range,3,5);}
    CHECK(xuiDocumentEditorInsertText(f.editor,"Y",1)==XUI_OK);
    text(f.whole,"aXYbcd\n");text(range,"cd");offsets(range,4,6);text(found,"cd");offsets(found,4,6);
    CHECK(format->ops->expand(format,XUI_INTERNAL_TEXT_FORMAT)==XUI_OK);text(format,"aXYbcd\n");offsets(format,0,7);
    CHECK(xuiDocumentEditorCancelInput(f.editor)==XUI_OK);atomic_store(&f.allocator.resume,1);
    text(range,"cd");offsets(range,2,4);text(found,"cd");text(f.whole,"abcd\n");
    CHECK(format->ops->expand(format,XUI_INTERNAL_TEXT_FORMAT)==XUI_OK);text(format,"abcd\n");offsets(format,0,5);
    format->ops->release(format);
    found->ops->release(found);
    CHECK(xuiEditSetSelection(f.editor,1,1)==XUI_OK);
    CHECK(xuiDocumentEditorInsertText(f.editor,"Z",1)==XUI_OK);
    f.generation=pending(&f).iGeneration;f.first=3;f.last=5;f.observe=1;
    CHECK(flush(&f)==XUI_OK);f.observe=0;CHECK(f.callbacks==1);
    text(range,"cd");offsets(range,3,5);source(&f,"aZbcd\n");
    CHECK(xuiDocumentEditorExecute(f.editor,XUI_DOC_EDIT_UNDO)==XUI_OK);text(range,"cd");offsets(range,2,4);
    CHECK(xuiDocumentEditorExecute(f.editor,XUI_DOC_EDIT_REDO)==XUI_OK);text(range,"cd");offsets(range,3,5);
    CHECK(xuiDocumentEditorInsertText(f.editor,"!",1)==XUI_OK);text(range,"cd");
    {xui_document_transaction transaction;xui_doc_txn_desc_t desc={0};desc.iSize=sizeof(desc);desc.iDomain=XUI_DOC_SOURCE;
     CHECK(xuiDocumentBeginTransaction(f.document,&desc,&transaction)==XUI_OK);
     CHECK(xuiDocumentTxnReplaceSource(transaction,0,0,"Q",1)==XUI_OK);
     CHECK(xuiDocumentTxnCommit(transaction,NULL)==XUI_OK);xuiDocumentTxnRelease(transaction);}
    text(range,"cd");offsets(range,4,6);source(&f,"QaZbcd\n");
    CHECK(xuiEditSetSelection(f.editor,1,1)==XUI_OK);
    CHECK(xuiDocumentEditorInsertText(f.editor,"R",1)==XUI_OK);
    atomic_store(&f.allocator.fail_worker,1);CHECK(flush(&f)==XUI_ERROR_OUT_OF_MEMORY);
    CHECK(pending(&f).iState==XUI_DOC_PREPARE_FAILED);text(range,"cd");offsets(range,5,7);
    atomic_store(&f.allocator.fail_worker,0);
    CHECK(xuiDocumentEditorRetryInput(f.editor)==XUI_OK);text(range,"cd");offsets(range,5,7);
    CHECK(flush(&f)==XUI_OK);text(range,"cd");offsets(range,5,7);source(&f,"QRaZbcd\n");
    CHECK(xuiDocumentEditorInsertText(f.editor,"\xe4\xb8\xad",3)==XUI_OK);
    atomic_store(&f.allocator.fail_at,atomic_load(&f.allocator.calls)+1);
    {char buffer[128];uint64_t bytes=99;CHECK(range->ops->getText(range,buffer,sizeof(buffer),&bytes)==XUI_ERROR_OUT_OF_MEMORY&&bytes==0);}
    atomic_store(&f.allocator.fail_at,UINT64_MAX);text(range,"cd");offsets(range,8,10);
    CHECK(xuiDocumentEditorCancelInput(f.editor)==XUI_OK);text(range,"cd");offsets(range,5,7);
    caret->ops->release(caret);finish(&f);
    puts("VISUAL ranges: RUNNING/5000 no-allocation, continue/cancel/publication callback/history/external/failed retry/projection OOM passed.");
}
static void cross_span(int blocks)
{
    visual_fixture f;xui_internal_accessible_text_range *edit,*suffix;
    start(&f,blocks?"abcde\n\nfghij\n\ntail\n":"abcde\nfghij\n\ntail\n");
    CHECK(f.whole->ops->findText(f.whole,"hij",3,0,0,&suffix)==XUI_OK&&suffix);f.retained=suffix;
    CHECK(f.whole->ops->clone(f.whole,&edit)==XUI_OK);
    CHECK(edit->ops->setOffsets(edit,3,8)==XUI_OK);CHECK(edit->ops->select(edit)==XUI_OK);
    CHECK(xuiDocumentEditorInsertText(f.editor,"*Z",2)==XUI_OK);
    CHECK(pending(&f).iGeneration);text(f.whole,"abc*Zhij\ntail\n");text(suffix,"hij");offsets(suffix,5,8);
    CHECK(xuiDocumentEditorCancelInput(f.editor)==XUI_OK);text(suffix,"hij");offsets(suffix,8,11);
    CHECK(edit->ops->setOffsets(edit,3,8)==XUI_OK);CHECK(edit->ops->select(edit)==XUI_OK);
    CHECK(xuiDocumentEditorInsertText(f.editor,"*Z",2)==XUI_OK);
    CHECK(xuiDocumentEditorInsertText(f.editor,"!",1)==XUI_OK);
    text(f.whole,"abc*Z!hij\ntail\n");text(suffix,"hij");offsets(suffix,6,9);
    CHECK(flush(&f)==XUI_OK);text(suffix,"hij");offsets(suffix,6,9);
    source(&f,"abc\\*Z\\!hij\n\ntail\n");
    CHECK(xuiDocumentEditorExecute(f.editor,XUI_DOC_EDIT_UNDO)==XUI_OK);text(suffix,"hij");offsets(suffix,8,11);
    CHECK(xuiDocumentEditorExecute(f.editor,XUI_DOC_EDIT_REDO)==XUI_OK);text(suffix,"hij");offsets(suffix,6,9);
    edit->ops->release(edit);finish(&f);
    printf("VISUAL ranges: cross %s, parsed text coalescing and Undo/Redo passed.\n",blocks?"paragraphs":"leaves");
}
int main(void)
{
    basic();cross_span(0);cross_span(1);
    puts("Visual prepared text ranges passed.");return 0;
}
