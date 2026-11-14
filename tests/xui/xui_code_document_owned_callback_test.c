/* Actual standalone and CodeEdit-owned documents; pure CPU backend. */
#define XUI_OWNED_FONTS_HELPERS
#ifdef XUI_TEST_STATIC_ENGINE
#define XUI_OWNED_CODE_DOCUMENT_HELPERS
#include "xui_owned_code_document_test.c"
#else
#define main xui_unused_owned_fonts_main
#include "xui_owned_fonts_test.c"
#undef main
#endif
#include <stdlib.h>

typedef struct ChangeOwner {size_t refs;bool deny,failDrop;xerror* holdError; xerror* dropError;} ChangeOwner;
static bool changeHold(const void* raw)
{ChangeOwner* owner=(void*)raw;if(owner->deny)return false;++owner->refs;if(owner->holdError)xrtSetErrorTake(xrtErrorRef(owner->holdError));return true;}
static bool changeDrop(const void* raw)
{ChangeOwner* owner=(void*)raw;assert(owner->refs>1);--owner->refs;if(owner->dropError)xrtSetErrorTake(xrtErrorRef(owner->dropError));return !owner->failDrop;}
static bool changeCount(const void* raw,size_t* output){*output=((const ChangeOwner*)raw)->refs;return true;}
static bool changeTrace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops changeOps={changeCount,changeTrace};
static xui_callback_owner_t changeRef(ChangeOwner* owner){return(xui_callback_owner_t){{owner,&changeOps},changeHold,changeDrop};}
typedef struct ChangeEvent {unsigned calls,gone;ChangeOwner* owner;struct ChangeEvent* replacement;bool close;bool clear;const char* savePath;} ChangeEvent;
static void changed(xui_code_document document,xui_code_range_t range,uint32_t version,void* raw)
{
    ChangeEvent* event=raw;++event->calls;assert(event->owner->refs==3 && range.iStart>=0 && version>0);
    xstrview view={0};assert(xuiCodeDocumentGetTextView(document,&view)==XUI_OK && view.Data);
    if(event->savePath)assert(xuiCodeDocumentSaveTextFile(document,event->savePath,XENCODING_UTF8)==XUI_OK);
    if(event->replacement){ChangeEvent* next=event->replacement;event->replacement=NULL;xui_callback_owner_t owner=changeRef(next->owner);
        assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,next,&owner)==XUI_OK && event->owner->refs==2);}
    if(event->clear)assert(xuiCodeDocumentSetChangeCallback(document,NULL,NULL)==XUI_OK && event->owner->refs==2);
    if(event->close){bool consumed=false;assert(xuiCodeDocumentDestroyOwnerChecked(document,&consumed)==XUI_OK && consumed && !event->gone);
        assert(xuiCodeDocumentGetTextView(document,&view)==XUI_ERROR_INVALID_STATE && !view.Data && !view.Size);}
}
static void changeGone(void* raw){++((ChangeEvent*)raw)->gone;}
static bool changeVisit(xrtownershipref ref,void* raw)
{if(ref.Data==raw){assert(ref.Ops==&changeOps);++matchingEdges;return true;}return false;}
static int32 wrongDocumentThread(ptr raw)
{
    xui_code_document document=raw;xui_code_document_scope_t scope={0};xstrview text={"sentinel",8};
    assert(xuiCodeDocumentCallBegin(document,&scope)==XUI_ERROR_BUSY && !scope.pSelf);
    assert(xuiCodeDocumentGetTextView(document,&text)==XUI_ERROR_BUSY && !text.Data && !text.Size);
    assert(xuiCodeDocumentOwnershipBusy(document) && !xuiCodeDocumentTraceOwnership(document,changeVisit,NULL));
    assert(xuiCodeDocumentSetChangeCallback(document,NULL,NULL)==XUI_ERROR_BUSY);
    bool consumed=true;assert(xuiCodeDocumentDestroyOwnerChecked(document,&consumed)==XUI_ERROR_BUSY && !consumed);
    return 0;
}
static void callbackTransactions(void)
{
    xui_code_document document=NULL;assert(xuiCodeDocumentCreate(&document)==XUI_OK);
    ChangeOwner a={.refs=1},b={.refs=1},bad={.refs=1,.deny=true};ChangeEvent first={.owner=&a},second={.owner=&b};
    xui_callback_owner_t oa=changeRef(&a),ob=changeRef(&b),rejected=changeRef(&bad);
    assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,&first,&oa)==XUI_OK && a.refs==2);
    xthread* worker=xrtThreadCreate(wrongDocumentThread,document,0);assert(worker && xrtThreadWait(worker)==XWAIT_OK);xrtThreadDestroy(worker);
    assert(a.refs==2 && !first.calls);
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    matchingEdges=0;assert(xuiCodeDocumentTraceOwnership(document,changeVisit,&a) && matchingEdges==1);
    assert(xrtOwnershipScopeEnd(&freeze));
    assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,&second,&rejected)==XUI_ERROR_CALLBACK_FAILED && bad.refs==1 && a.refs==2);
    assert(!xrtGetError());
    xrtSetErrorKind(XERR_ARGUMENT);xerror* primary=xrtTakeError();xrtSetErrorKind(XERR_STATE);xerror* secondary=xrtTakeError();assert(primary && secondary);
    b.holdError=primary;b.dropError=secondary;
    assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,&second,&ob)==XUI_ERROR_CALLBACK_FAILED && b.refs==1 && a.refs==2);
    assert(xrtGetError()==primary);xrtClearError();b.holdError=b.dropError=NULL;xrtErrorFree(primary);xrtErrorFree(secondary);
    first.replacement=&second;
    assert(xuiCodeDocumentSetTextLength(document,"a\0b",3)==XUI_OK && first.calls==1 && !second.calls && a.refs==1 && b.refs==2);
    second.clear=true;assert(xuiCodeDocumentSetText(document,"second")==XUI_OK && second.calls==1 && b.refs==1);
    assert(xuiCodeDocumentSetText(document,"no callback")==XUI_OK && second.calls==1);
    assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,&first,&oa)==XUI_OK);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i){xui_code_document_scope_t scope={0};assert(xuiCodeDocumentCallBegin(document,&scope)==XUI_OK);
        assert(xuiCodeDocumentOwnershipBusy(document));assert(xuiCodeDocumentCallEnd(&scope)==XUI_OK);
        assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,&first,&oa)==XUI_OK);}
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount && a.refs==2);
    a.failDrop=true;bool consumed=false;
    assert(xuiCodeDocumentDestroyOwnerChecked(document,&consumed)==XUI_ERROR_CALLBACK_FAILED && consumed && a.refs==1 && !xrtGetError());
}
static void scopesAndClose(void)
{
    xui_code_document document=NULL;assert(xuiCodeDocumentCreate(&document)==XUI_OK);
    ChangeOwner owner={.refs=1};ChangeEvent event={.owner=&owner};xui_callback_owner_t binding=changeRef(&owner);xui_lifetime_link_t watch={0};
    assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,&event,&binding)==XUI_OK);
    assert(xuiCodeDocumentLifetimeWatch(document,&watch,changeGone,&event)==XUI_OK);
    assert(xuiCodeDocumentSetTextLength(document,"a\0b",3)==XUI_OK);
    xui_code_document_scope_t scope={0};assert(xuiCodeDocumentCallBegin(document,&scope)==XUI_OK);
    xstrview text={0};assert(xuiCodeDocumentGetTextView(document,&text)==XUI_OK && text.Size==3);
    bool consumed=false;assert(xuiCodeDocumentDestroyOwnerChecked(document,&consumed)==XUI_OK && consumed && !event.gone);
    assert(!memcmp(text.Data,"a\0b",3) && owner.refs==2);
    assert(xuiCodeDocumentCallEnd(&scope)==XUI_ERROR_INVALID_STATE && event.gone==1 && owner.refs==1 && !watch.ppPrevious);
    assert(xuiCodeDocumentCallEnd(&scope)==XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiCodeDocumentCreate(&document)==XUI_OK);event=(ChangeEvent){.owner=&owner,.close=true};
    assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,&event,&binding)==XUI_OK);
    assert(xuiCodeDocumentLifetimeWatch(document,&watch,changeGone,&event)==XUI_OK);
    assert(xuiCodeDocumentSetText(document,"close in callback")==XUI_ERROR_INVALID_STATE && event.calls==1 && event.gone==1 && owner.refs==1);
}
static bool ownedDocumentEdge(xrtownershipref ref,void* raw)
{if(ref.Ops==&changeOps && ref.Data==raw)++matchingEdges;return true;}
static void saveDuringChange(void)
{
    const char* path=getenv("XUI_CODE_DOCUMENT_SAVE_TEST");assert(path && *path);
    xui_code_document document=NULL;assert(xuiCodeDocumentCreate(&document)==XUI_OK);
    ChangeOwner owner={.refs=1};ChangeEvent event={.owner=&owner,.savePath=path};xui_callback_owner_t binding=changeRef(&owner);
    assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,&event,&binding)==XUI_OK);
    assert(xuiCodeDocumentSetTextLength(document,"a\0b",3)==XUI_OK && event.calls==1 && !xuiCodeDocumentGetDirty(document));
    size_t size=0;bytes content=xrtFileReadAll(path,&size);assert(content && size==3 && !memcmp(content,"a\0b",3));xrtFree(content);
    assert(xuiCodeDocumentDestroyChecked(document)==XUI_OK && owner.refs==1);
}
static void editorOwnedGraph(void)
{
    xui_test_proxy_state_t state;Owner backendOwner={.refs=1};xui_proxy proxy=proxyCreate(&state,&backendOwner);
    xui_context context=NULL;assert(xuiCreate(&context)==XUI_OK && xuiSetProxyOwned(context,proxy)==XUI_OK);
    unsigned finalized=0;xui_font font=fontCreate(proxy,&finalized,XUI_OK);assert(xuiSetDefaultFont(context,font)==XUI_OK);
    xui_widget editor=NULL;assert(xuiWidgetCreateTyped(context,xuiCodeEditGetType(context),&editor,NULL)==XUI_OK);
    xui_code_document document=xuiCodeEditGetDocument(editor);assert(document);
    ChangeOwner owner={.refs=1};ChangeEvent event={.owner=&owner};xui_callback_owner_t binding=changeRef(&owner);
    assert(xuiCodeDocumentSetChangeCallbackOwned(document,changed,&event,&binding)==XUI_OK);
    matchingEdges=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(context,ownedDocumentEdge,&owner) && matchingEdges==1);assert(xrtOwnershipScopeEnd(&freeze));
    bool consumed=true;assert(xuiCodeDocumentDestroyOwnerChecked(document,&consumed)==XUI_ERROR_INVALID_STATE && !consumed);
    assert(xuiDestroyChecked(context)==XUI_OK && owner.refs==1);
    assert(xuiFontReleaseChecked(font)==XUI_OK && finalized==1 && xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&backendOwner));
}
int main(void)
{
#ifndef XUI_TEST_STATIC_ENGINE
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
    assert(xrtMemDebugEnabled()||xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    callbackTransactions();scopesAndClose();editorOwnedGraph();saveDuringChange();
#ifdef XUI_TEST_STATIC_ENGINE
    codeDocumentPrepareAndHistory();codeDocumentOwnerCloseInPrepare();codeDocumentDestroyInsideNotification();
#endif
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("CodeDocument owned callbacks: graph, replacement/clear/close reentry, Hold/Drop failures, scoped NUL reads and 1000 zero-heap updates passed");return 0;
}
