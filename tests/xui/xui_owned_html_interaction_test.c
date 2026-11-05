/* Actual HTML manager + Window + WebView lifetime, CPU-only browser I/O.
 * No browser initialization, window/device, network, GPU or security probe. */
#define XUI_OWNED_LAYOUT_HELPERS
#include "xui_owned_layout_test.c"
#include "../../include/xui_document_ui.h"
#define XUI_WEBVIEW_TEST_EXPORTS
#include "../../src/xui/document/xui_webview.c"
static bool closeDuringLoad;
static unsigned loaded;
static int cpuHtmlStart(xui_widget widget)
{assert(webview_get(widget));return XUI_OK;}
static int cpuHtmlBlock(xui_widget widget)
{assert(webview_get(widget));return XUI_OK;}
static int cpuHtmlHandler(xui_widget widget,const char* source,xui_webview_message_proc callback,void* user)
{
    xui_webview_data* data=webview_get(widget);assert(data && !strcmp(source,"about:blank"));
    data->on_message=callback;data->message_user=user;return XUI_OK;
}
static int cpuHtmlLoad(xui_widget widget,const char* html,size_t bytes)
{
    assert(webview_get(widget) && html && bytes && strstr(html,"iframe"));++loaded;
    if(closeDuringLoad){closeDuringLoad=false;assert(xuiRequestClose(widget->pContext)==XUI_OK);}
    assert(html[bytes]==0);return XUI_OK;
}
static int cpuHtmlFocus(xui_widget widget){assert(webview_get(widget));return XUI_OK;}
static int htmlHeapBudget=-1;
static void* cpuHtmlMalloc(size_t bytes)
{if(htmlHeapBudget==0)return NULL;if(htmlHeapBudget>0)--htmlHeapBudget;return malloc(bytes);}
static void* cpuHtmlCalloc(size_t count,size_t bytes)
{if(htmlHeapBudget==0)return NULL;if(htmlHeapBudget>0)--htmlHeapBudget;return calloc(count,bytes);}
#define malloc cpuHtmlMalloc
#define calloc cpuHtmlCalloc
#define xuiWebViewInitializeAsync cpuHtmlStart
#define xuiWebViewBlockExternalResources cpuHtmlBlock
#define xuiWebViewSetMessageHandler cpuHtmlHandler
#define xuiWebViewLoadHtml cpuHtmlLoad
#define xuiWebViewFocus cpuHtmlFocus
#include "../../src/xui/document/xui_document_html_interaction.c"
#undef xuiWebViewInitializeAsync
#undef xuiWebViewBlockExternalResources
#undef xuiWebViewSetMessageHandler
#undef xuiWebViewLoadHtml
#undef xuiWebViewFocus
#undef malloc
#undef calloc

typedef struct HtmlTest {
    LayoutProvider base;
    xui_font font;
    xui_document document;
    xui_doc_node_id node;
    xui_doc_html_interaction panel;
} HtmlTest;
static xui_doc_html_interaction_desc_t htmlDesc(HtmlTest* test)
{return(xui_doc_html_interaction_desc_t){.iSize=sizeof(xui_doc_html_interaction_desc_t),.pContext=test->base.context,.pDocument=test->document,.iNodeId=test->node};}
static void htmlSetup(HtmlTest* test,bool create)
{
    memset(test,0,sizeof(*test));layoutSetup(&test->base,&test->font);
    assert(xuiSetDefaultFont(test->base.context,test->font)==XUI_OK);
    xui_widget root=NULL;assert(xuiWidgetCreate(test->base.context,&root)==XUI_OK && xuiSetRootWidget(test->base.context,root)==XUI_OK);
    assert(xuiSetViewportSize(test->base.context,800,600)==XUI_OK);
    assert(xuiDocumentCreate(NULL,&test->document)==XUI_OK);
    xui_document_transaction txn=NULL;assert(xuiDocumentBeginTransaction(test->document,NULL,&txn)==XUI_OK);
    const char text[]="<p>CPU-only panel</p>";
    xui_doc_node_desc_t node={.iSize=sizeof(node),.iKind=XUI_DOC_HTML,.sText=text,.iTextBytes=sizeof(text)-1};
    assert(xuiDocumentTxnInsertNode(txn,1,XUI_DOCUMENT_APPEND,&node,&test->node)==XUI_OK && xuiDocumentTxnCommit(txn,NULL)==XUI_OK);
    xuiDocumentTxnRelease(txn);
    if(create){xui_doc_html_interaction_desc_t desc=htmlDesc(test);assert(xuiDocumentHtmlInteractionCreate(&desc,&test->panel)==XUI_OK);}
}
static void htmlFinish(HtmlTest* test)
{
    assert(xuiDestroyChecked(test->base.context)==XUI_OK);
    xuiDocumentRelease(test->document);
    assert(xuiFontReleaseChecked(test->font)==XUI_OK && xuiProxyReleaseChecked(test->base.proxy)==XUI_OK && drop(&test->base.owner));
}
static void htmlGone(void* raw){++*(unsigned*)raw;}
typedef struct HtmlOwner {
    size_t refs;
    unsigned holds,drops,calls;
    xui_doc_html_interaction panel;
    const xstrview* current;
    bool clear,close,closeOnHold,failDrop,inCallback;
} HtmlOwner;
static bool htmlOwnerHold(const void* raw)
{
    HtmlOwner* owner=(HtmlOwner*)raw;assert(owner->refs);++owner->refs;++owner->holds;
    if(owner->closeOnHold){owner->closeOnHold=false;assert(xuiRequestClose(owner->panel->context)==XUI_OK);}
    return true;
}
static bool htmlOwnerDrop(const void* raw)
{
    HtmlOwner* owner=(HtmlOwner*)raw;assert(owner->refs);--owner->refs;++owner->drops;
    if(owner->current){
        assert(owner->current->Size==26 && !memcmp(owner->current->Data,"https://cpu.invalid/a?b=cc",26));
        if(!owner->inCallback)owner->current=NULL;
    }
    bool fail=owner->failDrop;owner->failDrop=false;return !fail;
}
static bool htmlOwnerCount(const void* raw,size_t* count){*count=((const HtmlOwner*)raw)->refs;return *count!=0;}
static const xrtownershipops htmlOwnerOps={htmlOwnerCount,trace};
static xui_callback_owner_t htmlOwnerRef(HtmlOwner* owner){return(xui_callback_owner_t){{owner,&htmlOwnerOps},htmlOwnerHold,htmlOwnerDrop};}
static void htmlLink(const xstrview* view,void* raw)
{
    HtmlOwner* owner=raw;assert(view && owner->refs>=3);++owner->calls;owner->current=view;owner->inCallback=true;
    assert(view->Size==26 && !memcmp(view->Data,"https://cpu.invalid/a?b=cc",26));
    if(owner->clear)assert(xuiDocumentHtmlInteractionSetOpenLinkOwned(owner->panel,NULL,NULL,NULL)==XUI_OK);
    if(owner->close)assert(xuiDocumentHtmlInteractionReleaseChecked(owner->panel)==XUI_OK);
    assert(view->Data[0]=='h');owner->inCallback=false;
}
static void htmlSend(HtmlTest* test)
{
    char json[180];int bytes=snprintf(json,sizeof(json),"{\"kind\":\"externalLink\",\"url\":\"https://cpu.invalid/a?b=cc\",\"generation\":%u}",(unsigned)test->panel->page_generation);
    assert(bytes>0 && bytes<(int)sizeof(json));
    doc_html_interaction_message(test->panel->webview,"about:blank",json,(size_t)bytes,test->panel);
}
static void htmlOwned(bool close)
{
    HtmlTest test;htmlSetup(&test,true);HtmlOwner owner={.refs=1,.panel=test.panel,.clear=!close,.close=close};
    xui_callback_owner_t ref=htmlOwnerRef(&owner);
    assert(xuiDocumentHtmlInteractionSetOpenLinkOwned(test.panel,htmlLink,&owner,&ref)==XUI_OK && owner.refs==2);
    unsigned gone=0;xui_lifetime_link_t watch={0};assert(xuiDocumentHtmlInteractionLifetimeWatch(test.panel,&watch,htmlGone,&gone)==XUI_OK);
    assert(xuiWidgetDestroyChecked(test.panel->webview)==XUI_ERROR_INVALID_STATE);
    assert(xuiDocumentHtmlInteractionGetWebView(test.panel)==test.panel->webview);
    xrtownershipscope frozen={0};unsigned edges=0;assert(xrtOwnershipFreezeTryBegin(&frozen));
    assert(doc_html_trace(test.panel,layoutVisit,&edges) && edges==1);assert(xrtOwnershipScopeEnd(&frozen));
    xrtClearError();htmlSend(&test);assert(owner.calls==1 && owner.refs==1 && !owner.current && owner.holds==2 && owner.drops==2);
    if(close)assert(gone==1 && !watch.ppPrevious);else{
        assert(!gone && xuiDocumentHtmlInteractionGetContext(test.panel)==test.base.context);
        assert(xuiDocumentHtmlInteractionReleaseChecked(test.panel)==XUI_OK && gone==1);
    }
    assert(htmlOwnerDrop(&owner));htmlFinish(&test);
}
static void htmlQueuedDelegate(void)
{
    HtmlTest test;htmlSetup(&test,true);xui_webview_data* data=webview_get(test.panel->webview);webview_retain(data);
    unsigned gone=0;xui_lifetime_link_t watch={0};assert(xuiContextLifetimeWatch(test.base.context,&watch,htmlGone,&gone)==XUI_OK);
    bool consumed=true;assert(xuiDestroyOwnerChecked(test.base.context,&consumed)==XUI_STATUS_DEFERRED && !consumed && !gone);
    assert(data->retiring && data->refs==2);webview_release(data);assert(!gone && data->refs==1);
    assert(xuiDestroyOwnerChecked(test.base.context,&consumed)==XUI_OK && consumed && gone==1);
    xuiDocumentRelease(test.document);assert(xuiFontReleaseChecked(test.font)==XUI_OK && xuiProxyReleaseChecked(test.base.proxy)==XUI_OK && drop(&test.base.owner));
}
static void htmlWindowClose(void)
{
    HtmlTest test;htmlSetup(&test,true);unsigned gone=0;xui_lifetime_link_t watch={0};assert(xuiDocumentHtmlInteractionLifetimeWatch(test.panel,&watch,htmlGone,&gone)==XUI_OK);
    assert(xuiWidgetDestroyChecked(test.panel->window)==XUI_OK && gone==1);htmlFinish(&test);
}
static void htmlReadyAndHoldClose(void)
{
    HtmlTest test;htmlSetup(&test,true);xui_webview_data* data=webview_get(test.panel->webview);data->state=XUI_WEBVIEW_READY;
    unsigned before=loaded;webview_emit(data,XUI_WEBVIEW_EVENT_READY,0);assert(loaded==before+1 && !test.panel->dirty && test.panel->page_generation==1);
    assert(xuiDocumentHtmlInteractionUpdate(test.panel)==XUI_OK && loaded==before+1);
    assert(xuiDocumentHtmlInteractionShow(test.panel)==XUI_OK && !test.panel->focus_pending);
    HtmlOwner owner={.refs=1,.panel=test.panel,.closeOnHold=true};xui_callback_owner_t ref=htmlOwnerRef(&owner);
    assert(xuiDocumentHtmlInteractionSetOpenLinkOwned(test.panel,htmlLink,&owner,&ref)==XUI_ERROR_INVALID_STATE);
    assert(owner.refs==1 && owner.holds==1 && owner.drops==1);htmlFinish(&test);assert(htmlOwnerDrop(&owner));
}
static void htmlFailureReceipt(void)
{
    HtmlTest test;htmlSetup(&test,true);HtmlOwner owner={.refs=1,.panel=test.panel};xui_callback_owner_t ref=htmlOwnerRef(&owner);
    assert(xuiDocumentHtmlInteractionSetOpenLinkOwned(test.panel,htmlLink,&owner,&ref)==XUI_OK);
    owner.failDrop=true;xui_call_scope_t scope={0};assert(xuiCallScopeBegin(test.base.context,&scope)==XUI_OK);
    htmlSend(&test);assert(xuiCallScopeEnd(&scope)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError());
    assert(xuiDocumentHtmlInteractionReleaseChecked(test.panel)==XUI_OK && owner.refs==1);htmlFinish(&test);assert(htmlOwnerDrop(&owner));
}
static void htmlConstructorOom(void)
{
    unsigned failures=0,successes=0;
    for(int budget=0;budget<128;++budget){
        HtmlTest test;htmlSetup(&test,false);xui_doc_html_interaction_desc_t desc=htmlDesc(&test);
        assert(xrtMemDebugFailAfter(budget));int status=xuiDocumentHtmlInteractionCreate(&desc,&test.panel);xrtMemDebugFailClear();xrtClearError();
        if(status==XUI_OK){++successes;assert(test.panel);}else{++failures;assert(!test.panel);}
        htmlFinish(&test);
    }
    assert(failures && successes);
}
static void htmlRawLink(const char* url,void* raw)
{unsigned* count=raw;assert(!strcmp(url,"https://cpu.invalid/a?b=cc"));++*count;}
static void htmlRawAndLocal(void)
{
    HtmlTest test;htmlSetup(&test,true);unsigned calls=0;
    assert(xuiDocumentHtmlInteractionSetOpenLink(test.panel,htmlRawLink,&calls)==XUI_OK);htmlSend(&test);assert(calls==1);
    test.panel->backend_error=XUI_ERROR_UNSUPPORTED;xui_call_scope_t scope={0};assert(xuiCallScopeBegin(test.base.context,&scope)==XUI_OK);
    assert(xuiDocumentHtmlInteractionUpdate(test.panel)==XUI_ERROR_UNSUPPORTED && !xrtGetError());
    assert(xuiCallScopeEnd(&scope)==XUI_OK);test.panel->backend_error=0;
    htmlFinish(&test);
}
static void htmlHeapFailures(void)
{
    for(int budget=0;budget<2;++budget){
        HtmlTest test;htmlSetup(&test,false);xui_doc_html_interaction_desc_t desc=htmlDesc(&test);
        htmlHeapBudget=budget;assert(xuiDocumentHtmlInteractionCreate(&desc,&test.panel)==XUI_ERROR_OUT_OF_MEMORY && !test.panel);htmlHeapBudget=-1;htmlFinish(&test);
    }
    HtmlTest test;htmlSetup(&test,true);xui_webview_data* data=webview_get(test.panel->webview);data->state=XUI_WEBVIEW_READY;
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(test.base.context,&scope)==XUI_OK);
    htmlHeapBudget=0;webview_emit(data,XUI_WEBVIEW_EVENT_READY,0);htmlHeapBudget=-1;
    assert(test.panel->backend_error==XUI_ERROR_OUT_OF_MEMORY && test.panel->dirty && !test.panel->page_generation);
    assert(xuiCallScopeEnd(&scope)==XUI_ERROR_OUT_OF_MEMORY && !xrtGetError());
    test.panel->backend_error=0;assert(xuiDocumentHtmlInteractionUpdate(test.panel)==XUI_OK && !test.panel->dirty);
    htmlFinish(&test);
}
static void htmlCloseDuringLoad(void)
{
    HtmlTest test;htmlSetup(&test,true);xui_webview_data* data=webview_get(test.panel->webview);data->state=XUI_WEBVIEW_READY;
    closeDuringLoad=true;xui_call_scope_t scope={0};assert(xuiCallScopeBegin(test.base.context,&scope)==XUI_OK);
    assert(xuiDocumentHtmlInteractionUpdate(test.panel)==XUI_ERROR_INVALID_STATE && !closeDuringLoad);
    assert(xuiCallScopeEnd(&scope)==XUI_OK);htmlFinish(&test);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    htmlOwned(false);htmlOwned(true);htmlQueuedDelegate();htmlWindowClose();htmlReadyAndHoldClose();htmlFailureReceipt();htmlConstructorOom();htmlRawAndLocal();htmlHeapFailures();htmlCloseDuringLoad();
    xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("HTML actual Node/parts/typed URL view tail/close/delegate/OOM/NULL receipt: PASS");return 0;
}
