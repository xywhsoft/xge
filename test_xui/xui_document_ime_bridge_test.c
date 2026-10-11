/* Drive the real TSF text store and XGE input queue without a keyboard/IME.
 * The production XUI objects are linked directly so no test-only ABI is needed.
 * Rendering uses the deterministic proxy; this is not physical IME acceptance. */
#include "xge.c"
#include "xui_document_ui.h"
#include "test_xui/xui_test_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks;
#define CHECK(e) do { checks++; if (!(e)) { fprintf(stderr,"IME bridge line %d: %s\n",__LINE__,#e); exit(1); } } while (0)
static void pump(xui_context context)
{
    CHECK(xuiProxyXgePumpInputRect(context,(xui_rect_t){0,0,800,500})==XUI_OK);
    CHECK(xuiDispatchPendingEvents(context)==XUI_OK);
}
static void flush(xui_widget editor)
{
    int result=XUI_DOC_ERROR_BUSY, i;
    for(i=0;i<2000 && result==XUI_DOC_ERROR_BUSY;i++) {
        result=xuiDocumentEditorFlush(editor);
        if(result==XUI_DOC_ERROR_BUSY) Sleep(1);
    }
    CHECK(result==XUI_OK);
}
static void store_is(const char* text, int start, int end)
{
    xge_tsf_text_store_t* store=&g_xgeWin32Ime.tTextStore;
    char* utf8=NULL;int bytes=0;
    if(!store->sText) fprintf(stderr,"TSF expected [%s], got no snapshot.\n",text);
    CHECK(store->sText);
    CHECK(__xgeTsfWideToUtf8(store->sText,store->iTextLength,&utf8,&bytes));
    if(strcmp(utf8?utf8:"",text))fprintf(stderr,"TSF expected [%s], got [%s]\n",text,utf8?utf8:"");
    CHECK(!strcmp(utf8?utf8:"",text));
    CHECK(__xgeTsfStoreDocumentUtf8Offset(store,store->iSelectionStart)==start);
    CHECK(__xgeTsfStoreDocumentUtf8Offset(store,store->iSelectionEnd)==end);
    xrtFree(utf8);
}
static void compose(xui_context context,const WCHAR* text,int cancel)
{
    xge_tsf_text_store_t* store=&g_xgeWin32Ime.tTextStore;WINBOOL ok=FALSE;
    LONG start=store->iSelectionStart,end=store->iSelectionEnd;TS_TEXTCHANGE change;
    CHECK(store->tCompositionSink.lpVtbl->OnStartComposition(&store->tCompositionSink,NULL,&ok)==S_OK && ok);
    pump(context);
    if(!cancel) {
        store->iLockType=TS_LF_READWRITE;
        CHECK(store->tStore.lpVtbl->SetText(&store->tStore,0,start,end,text,(ULONG)wcslen(text),&change)==S_OK);
        store->iLockType=0;
        __xgeTsfStoreFlushEvents(store);
        pump(context);
    }
    CHECK(store->tCompositionSink.lpVtbl->OnEndComposition(&store->tCompositionSink,NULL)==S_OK);
    pump(context);
}
static void empty_replacement(xui_context context,xui_widget editor,xui_document document)
{
    xge_tsf_text_store_t* store=&g_xgeWin32Ime.tTextStore;
    char* before=strdup(xuiEditGetText(editor));char* expected;
    int first,last;uint64_t revision;WINBOOL ok=FALSE;TS_TEXTCHANGE change;
    LONG start,end;WCHAR* original=NULL;int original_length=0;
    CHECK(before && strstr(before,"输入😀"));
    first=(int)(strstr(before,"输入😀")-before);last=first+(int)strlen("输入😀");
    expected=malloc(strlen(before)+1);CHECK(expected);
    memcpy(expected,before,(size_t)first);strcpy(expected+first,before+last);
    CHECK(xuiEditSetSelection(editor,first,last)==XUI_OK);pump(context);store_is(before,first,last);
    revision=xuiDocumentGetRevision(document);
    compose(context,L"",0);flush(editor);
    CHECK(!strcmp(xuiEditGetText(editor),expected));
    CHECK(xuiDocumentGetRevision(document)==revision+1);
    CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);
    CHECK(!strcmp(xuiEditGetText(editor),before));
    CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_REDO)==XUI_OK);
    CHECK(!strcmp(xuiEditGetText(editor),expected));
    CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);
    CHECK(!strcmp(xuiEditGetText(editor),before));
    /* Restore an empty preedit to its original text: end means cancel. */
    CHECK(xuiEditSetSelection(editor,first,last)==XUI_OK);pump(context);
    start=store->iSelectionStart;end=store->iSelectionEnd;
    CHECK(__xgeTsfUtf8ToWide("输入😀",(int)strlen("输入😀"),&original,&original_length));
    revision=xuiDocumentGetRevision(document);
    CHECK(store->tCompositionSink.lpVtbl->OnStartComposition(&store->tCompositionSink,NULL,&ok)==S_OK && ok);
    pump(context);store->iLockType=TS_LF_READWRITE;
    CHECK(store->tStore.lpVtbl->SetText(&store->tStore,0,start,end,L"",0,&change)==S_OK);
    store->iLockType=0;__xgeTsfStoreFlushEvents(store);pump(context);
    CHECK(xuiDocumentGetRevision(document)==revision && !strcmp(xuiEditGetText(editor),before));
    store->iLockType=TS_LF_READWRITE;
    CHECK(store->tStore.lpVtbl->SetText(&store->tStore,0,start,start,original,(ULONG)original_length,&change)==S_OK);
    store->iLockType=0;__xgeTsfStoreFlushEvents(store);pump(context);
    CHECK(store->tCompositionSink.lpVtbl->OnEndComposition(&store->tCompositionSink,NULL)==S_OK);
    pump(context);flush(editor);
    CHECK(xuiDocumentGetRevision(document)==revision && !strcmp(xuiEditGetText(editor),before));
    xrtFree(original);
    /* TSF also replaces ranges directly without a composition callback. */
    CHECK(xuiEditSetSelection(editor,first,last)==XUI_OK);pump(context);
    start=store->iSelectionStart;end=store->iSelectionEnd;revision=xuiDocumentGetRevision(document);
    store->iLockType=TS_LF_READWRITE;
    CHECK(store->tStore.lpVtbl->SetText(&store->tStore,0,start,end,L"",0,&change)==S_OK);
    store->iLockType=0;__xgeTsfStoreFlushEvents(store);pump(context);flush(editor);
    CHECK(!strcmp(xuiEditGetText(editor),expected) && xuiDocumentGetRevision(document)==revision+1);
    CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);
    CHECK(!strcmp(xuiEditGetText(editor),before));
    /* An empty insertion with no selected text remains a no-op. */
    CHECK(xuiEditSetSelection(editor,first,first)==XUI_OK);pump(context);
    start=store->iSelectionStart;revision=xuiDocumentGetRevision(document);store->iLockType=TS_LF_READWRITE;
    CHECK(store->tStore.lpVtbl->SetText(&store->tStore,0,start,start,L"",0,&change)==S_OK);
    store->iLockType=0;__xgeTsfStoreFlushEvents(store);pump(context);flush(editor);
    CHECK(!strcmp(xuiEditGetText(editor),before) && xuiDocumentGetRevision(document)==revision);
    free(expected);free(before);
    puts("Native IME empty commit, restored cancellation and direct TSF deletion passed");
}
static void run_mode(int mode)
{
    const char* source="first\n\n中文 **bold** target 😀\n";
    xui_context context; xui_widget editor; xui_document document;
    xui_test_proxy_state_t proxy; xui_font font; xui_doc_editor_desc_t ed={0};xui_doc_desc_t dd={0};
    char *before,*expected; const char* text; int first,last;uint64_t revision;
    dd.iSize=sizeof(dd);dd.iProfile=XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(mode==0?NULL:&dd,&document)==XUI_OK);
    if(mode)CHECK(xuiDocumentLoadMarkdown(document,source,strlen(source))==XUI_OK);
    xuiTestProxyInit(&proxy);CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy.tProxy)==XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK);
    CHECK(xuiSetDefaultFont(context,font)==XUI_OK && xuiSetViewportSize(context,800,500)==XUI_OK);
    ed.iSize=sizeof(ed);ed.tView.iSize=sizeof(ed.tView);ed.tView.pDocument=document;
    ed.iMode=mode==1?XUI_DOC_SOURCE_TEXT:mode==3?XUI_DOC_LIVE_MARKDOWN:XUI_DOC_VISUAL;
    CHECK(xuiDocumentEditorCreate(context,&ed,&editor)==XUI_OK && xuiSetRootWidget(context,editor)==XUI_OK);
    CHECK(xuiWidgetSetRect(editor,(xui_rect_t){0,0,800,500})==XUI_OK);
    if(!mode)CHECK(xuiDocumentEditorInsertText(editor,"first\n中文 bold target 😀",strlen("first\n中文 bold target 😀"))==XUI_OK);
    CHECK(xuiSetFocusWidget(context,editor)==XUI_OK);
    text=xuiEditGetText(editor);CHECK(text && strstr(text,"target"));before=strdup(text);CHECK(before);
    first=(int)(strstr(before,"target")-before);last=first+6;
    CHECK(xuiEditSetSelection(editor,last,first)==XUI_OK); /* Reverse selection must be ordered for TSF. */
    pump(context);store_is(before,first,last);
    revision=xuiDocumentGetRevision(document);
    compose(context,L"输入😀",0);flush(editor);
    expected=malloc(strlen(before)+32);CHECK(expected);
    memcpy(expected,before,(size_t)first);strcpy(expected+first,"输入😀");strcat(expected,before+last);
    text=xuiEditGetText(editor);CHECK(text && !strcmp(text,expected));
    CHECK(xuiDocumentGetRevision(document)==revision+1);
    CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);
    CHECK(!strcmp(xuiEditGetText(editor),before));
    CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_REDO)==XUI_OK);
    CHECK(!strcmp(xuiEditGetText(editor),expected));
    CHECK(xuiEditSetSelection(editor,first,first+(int)strlen("输入😀"))==XUI_OK);
    pump(context);revision=xuiDocumentGetRevision(document);
    compose(context,L"",1);flush(editor);
    CHECK(!strcmp(xuiEditGetText(editor),expected) && xuiDocumentGetRevision(document)==revision);
    CHECK(xuiEditGetSelection(editor,&first,&last)==XUI_OK);
    /* Native offsets span paragraphs and differently marked nodes. */
    free(before);before=strdup(xuiEditGetText(editor));CHECK(before);
    first=2;last=(int)(strstr(before,"输入😀")-before)+(int)strlen("输入😀");
    CHECK(xuiEditSetSelection(editor,first,last)==XUI_OK);pump(context);
    store_is(before,first,last);revision=xuiDocumentGetRevision(document);
    compose(context,L"跨段",0);flush(editor);
    memcpy(expected,before,(size_t)first);strcpy(expected+first,"跨段");strcat(expected,before+last);
    CHECK(!strcmp(xuiEditGetText(editor),expected) && xuiDocumentGetRevision(document)==revision+1);
    CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);
    CHECK(!strcmp(xuiEditGetText(editor),before));
    empty_replacement(context,editor,document);
    free(expected);free(before);xuiDestroy(context);xuiDocumentRelease(document);
    proxy.tProxy.fontDestroy(&proxy.tProxy,font);
    puts(mode==0?"Rich native IME bridge passed":mode==1?"Source native IME bridge passed":mode==2?"Visual native IME bridge passed":"Live native IME bridge passed");
}
static void focus_switch(int mode,int scenario)
{
    xui_context context;xui_widget root,editors[2];xui_document documents[2];
    xui_test_proxy_state_t proxy;xui_font font;xui_doc_editor_desc_t desc={0};
    xge_tsf_text_store_t* store=&g_xgeWin32Ime.tTextStore;
    WINBOOL ok=FALSE;TS_TEXTCHANGE change;LONG first,last;uint64_t revisions[2];int i;
    xui_doc_desc_t document_desc={0};xge_input_event_t event={0};
    xuiTestProxyInit(&proxy);CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy.tProxy)==XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK);
    CHECK(xuiSetDefaultFont(context,font)==XUI_OK && xuiSetViewportSize(context,800,500)==XUI_OK);
    CHECK(xuiPanelCreate(context,&root,NULL)==XUI_OK && xuiSetRootWidget(context,root)==XUI_OK);
    CHECK(xuiWidgetSetRect(root,(xui_rect_t){0,0,800,500})==XUI_OK);
    desc.iSize=sizeof(desc);desc.tView.iSize=sizeof(desc.tView);
    desc.iMode=mode==1?XUI_DOC_SOURCE_TEXT:mode==3?XUI_DOC_LIVE_MARKDOWN:XUI_DOC_VISUAL;
    document_desc.iSize=sizeof(document_desc);document_desc.iProfile=XUI_DOCUMENT_MARKDOWN;
    for(i=0;i<2;i++) {
        CHECK(xuiDocumentCreate(mode?&document_desc:NULL,&documents[i])==XUI_OK);desc.tView.pDocument=documents[i];
        CHECK(xuiDocumentEditorCreate(context,&desc,&editors[i])==XUI_OK);
        CHECK(xuiWidgetAddChild(root,editors[i])==XUI_OK);
        CHECK(xuiWidgetSetRect(editors[i],(xui_rect_t){(float)(i*400),0,400,500})==XUI_OK);
        if(mode)CHECK(xuiDocumentLoadMarkdown(documents[i],"target\n",7)==XUI_OK);
        else CHECK(xuiDocumentEditorInsertText(editors[i],"target",6)==XUI_OK);
        revisions[i]=xuiDocumentGetRevision(documents[i]);
    }
    CHECK(xuiSetFocusWidget(context,editors[0])==XUI_OK && xuiEditSetSelection(editors[0],0,6)==XUI_OK);
    pump(context);store_is("target\n",0,6);first=store->iSelectionStart;last=store->iSelectionEnd;
    if(scenario!=7)CHECK(store->tCompositionSink.lpVtbl->OnStartComposition(&store->tCompositionSink,NULL,&ok)==S_OK && ok);
    if(scenario!=2 && scenario!=7)pump(context);
    store->iLockType=TS_LF_READWRITE;
    CHECK(store->tStore.lpVtbl->SetText(&store->tStore,0,first,last,L"错投",2,&change)==S_OK);
    store->iLockType=0;__xgeTsfStoreFlushEvents(store);
    if(scenario!=2 && scenario!=7)pump(context);
    /* Also invalidate a complete commit that is already waiting in XGE. */
    if(scenario>=8)CHECK(store->tCompositionSink.lpVtbl->OnEndComposition(&store->tCompositionSink,NULL)==S_OK);
    if(scenario<=2 || scenario==7) {
        CHECK(xuiSetFocusWidget(context,editors[1])==XUI_OK);
        if(scenario!=2)pump(context);
        if(scenario==1) { CHECK(xuiSetFocusWidget(context,editors[0])==XUI_OK);pump(context); }
    } else if(scenario==3) {
        event.iSize=sizeof(event);event.iType=XGE_EVENT_WINDOW_BLUR;
        CHECK(__xgeInputEventQueuePush(&event));pump(context);
    } else if(scenario==4) {
        CHECK(xuiInputKeyDown(context,XUI_KEY_ESCAPE,0)==XUI_OK);
        CHECK(xuiDispatchPendingEvents(context)==XUI_OK);
    } else if(scenario==5) {
        CHECK(xuiEditSetSelection(editors[0],0,0)==XUI_OK);
    } else if(scenario==6) {
        CHECK(mode && xuiDocumentViewSetMode(editors[0],mode==1?XUI_DOC_VISUAL:XUI_DOC_SOURCE_TEXT)==XUI_OK);
    } else if(scenario==8) {
        CHECK(xuiDocumentEditorSetReadOnly(editors[0],1)==XUI_OK);
        CHECK(xuiDocumentEditorSetReadOnly(editors[0],0)==XUI_OK);
    } else if(scenario==9) {
        CHECK(xuiDocumentViewSetDocument(editors[0],documents[1])==XUI_OK);
        CHECK(!xuiDocumentEditorIsComposing(editors[0]));
        CHECK(xuiDocumentViewSetDocument(editors[0],documents[0])==XUI_OK);
    } else if(scenario==10) {
        xuiWidgetDestroy(editors[0]);
        desc.tView.pDocument=documents[0];
        CHECK(xuiDocumentEditorCreate(context,&desc,&editors[0])==XUI_OK);
        CHECK(xuiWidgetAddChild(root,editors[0])==XUI_OK);
        CHECK(xuiWidgetSetRect(editors[0],(xui_rect_t){0,0,400,500})==XUI_OK);
        CHECK(xuiSetFocusWidget(context,editors[0])==XUI_OK);
    } else {
        CHECK(scenario==11 && xuiWidgetSetEnabled(editors[0],0)==XUI_OK);
        CHECK(xuiWidgetSetEnabled(editors[0],1)==XUI_OK);
        CHECK(xuiSetFocusWidget(context,editors[0])==XUI_OK);
    }
    if(scenario<8 && scenario!=7)CHECK(store->tCompositionSink.lpVtbl->OnEndComposition(&store->tCompositionSink,NULL)==S_OK);
    pump(context);flush(editors[0]);flush(editors[1]);
    for(i=0;i<2;i++) {
        CHECK(!strcmp(xuiEditGetText(editors[i]),"target\n"));
        CHECK(xuiDocumentGetRevision(documents[i])==revisions[i]);
    }
    /* A fresh composition in the newly focused editor must still commit. */
    CHECK(xuiSetFocusWidget(context,editors[1])==XUI_OK && xuiEditSetSelection(editors[1],0,6)==XUI_OK);pump(context);
    compose(context,L"正确",0);flush(editors[1]);
    CHECK(!strcmp(xuiEditGetText(editors[1]),"正确\n") && xuiDocumentGetRevision(documents[1])==revisions[1]+1);
    CHECK(!strcmp(xuiEditGetText(editors[0]),"target\n") && xuiDocumentGetRevision(documents[0])==revisions[0]);
    xuiDestroy(context);for(i=0;i<2;i++)xuiDocumentRelease(documents[i]);
    proxy.tProxy.fontDestroy(&proxy.tProxy,font);
    puts("Native IME session change cancels stale commit and accepts fresh composition");
}
static void context_switch(int destroy_previous)
{
    xui_context contexts[2];xui_widget editors[2];xui_document documents[2];
    xui_test_proxy_state_t proxy;xui_font font;xui_doc_editor_desc_t desc={0};
    xge_tsf_text_store_t* store=&g_xgeWin32Ime.tTextStore;
    WINBOOL ok=FALSE;TS_TEXTCHANGE change;LONG first,last;uint64_t revisions[2];int i;
    xuiTestProxyInit(&proxy);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK);
    desc.iSize=sizeof(desc);desc.tView.iSize=sizeof(desc.tView);desc.iMode=XUI_DOC_VISUAL;
    for(i=0;i<2;i++) {
        CHECK(xuiCreate(&contexts[i])==XUI_OK && xuiSetProxy(contexts[i],&proxy.tProxy)==XUI_OK);
        CHECK(xuiSetDefaultFont(contexts[i],font)==XUI_OK && xuiSetViewportSize(contexts[i],800,500)==XUI_OK);
        CHECK(xuiDocumentCreate(NULL,&documents[i])==XUI_OK);desc.tView.pDocument=documents[i];
        CHECK(xuiDocumentEditorCreate(contexts[i],&desc,&editors[i])==XUI_OK);
        CHECK(xuiSetRootWidget(contexts[i],editors[i])==XUI_OK);
        CHECK(xuiWidgetSetRect(editors[i],(xui_rect_t){0,0,800,500})==XUI_OK);
        CHECK(xuiDocumentEditorInsertText(editors[i],"target",6)==XUI_OK);
        CHECK(xuiSetFocusWidget(contexts[i],editors[i])==XUI_OK && xuiEditSetSelection(editors[i],0,6)==XUI_OK);
        revisions[i]=xuiDocumentGetRevision(documents[i]);
    }
    pump(contexts[0]);store_is("target\n",0,6);first=store->iSelectionStart;last=store->iSelectionEnd;
    CHECK(store->tCompositionSink.lpVtbl->OnStartComposition(&store->tCompositionSink,NULL,&ok)==S_OK && ok);
    pump(contexts[0]);store->iLockType=TS_LF_READWRITE;
    CHECK(store->tStore.lpVtbl->SetText(&store->tStore,0,first,last,L"错投",2,&change)==S_OK);
    store->iLockType=0;__xgeTsfStoreFlushEvents(store);pump(contexts[0]);
    if(destroy_previous) { xuiDestroy(contexts[0]);contexts[0]=NULL; }
    pump(contexts[1]);
    if(!destroy_previous) {
        pump(contexts[0]);
        CHECK(!xuiDocumentEditorIsComposing(editors[0]));
    }
    CHECK(store->tCompositionSink.lpVtbl->OnEndComposition(&store->tCompositionSink,NULL)==S_OK);
    pump(contexts[1]);flush(editors[1]);
    CHECK(!strcmp(xuiEditGetText(editors[1]),"target\n") && xuiDocumentGetRevision(documents[1])==revisions[1]);
    CHECK(xuiDocumentGetRevision(documents[0])==revisions[0]);
    if(contexts[0])CHECK(!strcmp(xuiEditGetText(editors[0]),"target\n"));
    CHECK(xuiEditSetSelection(editors[1],0,6)==XUI_OK);pump(contexts[1]);
    compose(contexts[1],L"正确",0);flush(editors[1]);
    CHECK(!strcmp(xuiEditGetText(editors[1]),"正确\n") && xuiDocumentGetRevision(documents[1])==revisions[1]+1);
    CHECK(xuiDocumentGetRevision(documents[0])==revisions[0]);
    for(i=0;i<2;i++) { if(contexts[i])xuiDestroy(contexts[i]);xuiDocumentRelease(documents[i]); }
    proxy.tProxy.fontDestroy(&proxy.tProxy,font);
    puts("Native IME context switch or destruction rejects stale commit");
}
static void window_boundary(void)
{
    xui_context context; xui_document document; xui_widget editor;
    xui_test_proxy_state_t proxy; xui_font font;xui_doc_desc_t dd={0};xui_doc_editor_desc_t ed={0};
    char* source=malloc(72016);char* window=NULL;int i,bytes,start=72000;
    xge_tsf_text_store_t* store=&g_xgeWin32Ime.tTextStore;
    CHECK(source);
    for(i=0;i<24000;i++)memcpy(source+i*3,"中",3);
    strcpy(source+start,"target!\n");
    dd.iSize=sizeof(dd);dd.iProfile=XUI_DOCUMENT_MARKDOWN;
    CHECK(xuiDocumentCreate(&dd,&document)==XUI_OK && xuiDocumentLoadMarkdown(document,source,strlen(source))==XUI_OK);
    xuiTestProxyInit(&proxy);CHECK(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy.tProxy)==XUI_OK);
    CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK);
    CHECK(xuiSetDefaultFont(context,font)==XUI_OK && xuiSetViewportSize(context,800,500)==XUI_OK);
    ed.iSize=sizeof(ed);ed.tView.iSize=sizeof(ed.tView);ed.tView.pDocument=document;ed.iMode=XUI_DOC_SOURCE_TEXT;
    CHECK(xuiDocumentEditorCreate(context,&ed,&editor)==XUI_OK && xuiSetRootWidget(context,editor)==XUI_OK);
    CHECK(xuiWidgetSetRect(editor,(xui_rect_t){0,0,800,500})==XUI_OK);
    CHECK(xuiEditSetSelection(editor,start,start+6)==XUI_OK && xuiSetFocusWidget(context,editor)==XUI_OK);
    pump(context);CHECK(store->iDocumentOffset>0 && store->iDocumentOffset%3==0);
    CHECK(__xgeTsfWideToUtf8(store->sText,store->iTextLength,&window,&bytes));
    CHECK(window && bytes<=65538 && !strcmp(window,source+store->iDocumentOffset));
    CHECK(__xgeTsfStoreDocumentUtf8Offset(store,store->iSelectionStart)==start);
    CHECK(__xgeTsfStoreDocumentUtf8Offset(store,store->iSelectionEnd)==start+6);
    compose(context,L"尾部",0);flush(editor);
    strcpy(source+start,"尾部!\n");CHECK(!strcmp(xuiEditGetText(editor),source));
    CHECK(xuiDocumentEditorExecute(editor,XUI_DOC_EDIT_UNDO)==XUI_OK);
    strcpy(source+start,"target!\n");CHECK(!strcmp(xuiEditGetText(editor),source));
    xrtFree(window);free(source);xuiDestroy(context);xuiDocumentRelease(document);
    proxy.tProxy.fontDestroy(&proxy.tProxy,font);
    puts("Native IME 64KB UTF-8 snapshot window and document offset passed");
}
int main(void)
{
    xge_desc_t desc={0};int mode,scenario;
    desc.iRunMode=XGE_RUN_MANUAL;desc.iWidth=800;desc.iHeight=500;
    CHECK(xgeInit(&desc)==XGE_OK);
    __xgeTsfStoreInitialize(&g_xgeWin32Ime.tTextStore);
    g_xgeWin32Ime.bTsfInitialized=1;g_xgeWin32Ime.iMode=XGE_IME_MODE_COMPOSITION;
    for(mode=0;mode<4;mode++)run_mode(mode);
    for(mode=0;mode<4;mode++)for(scenario=0;scenario<12;scenario++)if(mode || scenario!=6)focus_switch(mode,scenario);
    context_switch(0);context_switch(1);
    window_boundary();
    g_xgeWin32Ime.bTsfInitialized=0;__xgeTsfStoreUnit(&g_xgeWin32Ime.tTextStore);xgeUnit();
    printf("Document native IME bridge: 4 modes; %u checks; no physical IME or clipboard verification.\n",checks);
    return 0;
}
