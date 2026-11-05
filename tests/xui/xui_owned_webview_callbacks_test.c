/* Actual WebView publisher/consumer, no WebView2/browser/engine initialization. */
#define XUI_OWNED_LAYOUT_HELPERS
#include "xui_owned_layout_test.c"
#define XUI_WEBVIEW_TEST_EXPORTS
#include "../../src/xui/document/xui_webview.c"

typedef struct WebOwner {
    size_t refs;
    unsigned holds,drops,calls;
    bool clear,destroy,requestOnHold,failDrop;
    xui_widget widget;
} WebOwner;
static bool webHold(const void* raw)
{
    WebOwner* owner=(WebOwner*)raw;assert(owner->refs);++owner->refs;++owner->holds;
    if(owner->requestOnHold)assert(xuiRequestClose(owner->widget->pContext)==XUI_OK);
    return true;
}
static bool webDrop(const void* raw)
{
    WebOwner* owner=(WebOwner*)raw;assert(owner->refs);--owner->refs;++owner->drops;
    bool failed=owner->failDrop;owner->failDrop=false;return !failed;
}
static bool webCount(const void* raw,size_t* count){*count=((const WebOwner*)raw)->refs;return *count!=0;}
static const xrtownershipops webOps={webCount,trace};
static xui_callback_owner_t webOwnerRef(WebOwner* owner){return(xui_callback_owner_t){{owner,&webOps},webHold,webDrop};}
static void webEvent(xui_widget widget,int event,int32_t error,void* raw)
{
    WebOwner* owner=raw;(void)event;(void)error;assert(widget==owner->widget && owner->refs>=3);++owner->calls;
    if(owner->clear)assert(xuiWebViewSetEvent(widget,NULL,NULL)==XUI_OK && owner->refs==2);
    if(owner->destroy)assert(xuiWidgetDestroyChecked(widget)==XUI_OK);
}
static xui_widget createWeb(xui_context* context)
{
    assert(xuiCreate(context)==XUI_OK);xui_widget widget=NULL;
    xui_webview_desc_t desc={.iSize=sizeof(desc)};assert(xuiWebViewCreate(*context,&widget,&desc)==XUI_OK);return widget;
}
static void ownedEvent(bool destroy)
{
    xui_context context=NULL;xui_widget widget=createWeb(&context);
    WebOwner owner={.refs=1,.widget=widget,.clear=!destroy,.destroy=destroy};xui_callback_owner_t ref=webOwnerRef(&owner);
    assert(xuiWebViewSetEventOwned(widget,webEvent,&owner,&ref)==XUI_OK && owner.refs==2);
    xui_webview_data* data=webview_get(widget);webview_emit(data,XUI_WEBVIEW_EVENT_READY,0);
    assert(owner.calls==1 && owner.refs==1 && owner.holds==2 && owner.drops==2);
    assert(xuiDestroyChecked(context)==XUI_OK && webDrop(&owner));
}
static void requestedHold(void)
{
    xui_context context=NULL;xui_widget widget=createWeb(&context);
    WebOwner owner={.refs=1,.widget=widget,.requestOnHold=true};xui_callback_owner_t ref=webOwnerRef(&owner);
    assert(xuiWebViewSetEventOwned(widget,webEvent,&owner,&ref)==XUI_ERROR_INVALID_STATE);
    assert(owner.refs==1 && owner.holds==1 && owner.drops==1 && !xrtGetError());
    assert(xuiDestroyChecked(context)==XUI_OK && webDrop(&owner));
}
static void failedDrop(void)
{
    xui_context context=NULL;xui_widget widget=createWeb(&context);
    WebOwner owner={.refs=1,.widget=widget};xui_callback_owner_t ref=webOwnerRef(&owner);
    assert(xuiWebViewSetEventOwned(widget,webEvent,&owner,&ref)==XUI_OK);
    owner.failDrop=true;xui_call_scope_t outer={0};assert(xuiCallScopeBegin(context,&outer)==XUI_OK);
    webview_emit(webview_get(widget),XUI_WEBVIEW_EVENT_READY,0);
    assert(xuiCallScopeEnd(&outer)==XUI_ERROR_CALLBACK_FAILED && !xrtGetError() && owner.calls==1);
    assert(xuiDestroyChecked(context)==XUI_OK && webDrop(&owner));
}
static void rawMessage(xui_widget widget,const char* source,const char* json,size_t bytes,void* raw)
{
    unsigned* count=raw;assert(!strcmp(source,"about:blank") && bytes==2 && !memcmp(json,"{}",bytes));++*count;
    assert(xuiWidgetDestroyChecked(widget)==XUI_OK);
}
static void queuedDelegate(void)
{
    xui_context context=NULL;xui_widget widget=createWeb(&context);xui_webview_data* data=webview_get(widget);
    /* This is the exact physical reference retained by an outstanding COM
     * delegate. Its payload outlives the Widget, its callback userdata does not. */
    webview_retain(data);data->state=XUI_WEBVIEW_READY;unsigned calls=0;
    data->on_message=rawMessage;data->message_user=&calls;
    webview_deliver_message(data,"about:blank","{}",2);
    assert(calls==1 && data->widget==widget && data->retiring && data->state==XUI_WEBVIEW_CLOSED);
    data->user=data->message_user=(void*)(uintptr_t)1; /* inaccessible late userdata */
    webview_emit(data,XUI_WEBVIEW_EVENT_READY,0);
    webview_deliver_message(data,"about:blank","{}",2);assert(calls==1);
    webview_release(data);assert(xuiDestroyChecked(context)==XUI_OK);
}
static int32 wrongThreadDispatch(ptr raw)
{
    xui_webview_data* data=raw;
    webview_emit(data,XUI_WEBVIEW_EVENT_READY,0);
    webview_deliver_message(data,"about:blank","{}",2);
    xrtClearError();return 0;
}
static void creatorAdmission(void)
{
    xui_test_proxy_state_t state;xuiTestProxyInit(&state);Owner backend={.refs=1};xui_proxy proxy=NULL;
    xui_callback_owner_t backendRef=ownerRef(&backend);assert(xuiProxyCreateOwned(&state.tProxy,&backendRef,&proxy)==XUI_OK);
    xui_context context=NULL;assert(xuiCreateOwned(proxy,&context)==XUI_OK);
    xui_widget widget=NULL;xui_webview_desc_t desc={.iSize=sizeof(desc)};assert(xuiWebViewCreate(context,&widget,&desc)==XUI_OK);
    WebOwner owner={.refs=1,.widget=widget};xui_callback_owner_t ref=webOwnerRef(&owner);
    assert(xuiWebViewSetEventOwned(widget,webEvent,&owner,&ref)==XUI_OK);
    xui_webview_data* data=webview_get(widget);unsigned messages=0;
    data->state=XUI_WEBVIEW_READY;data->on_message=rawMessage;data->message_user=&messages;
    xthread* thread=xrtThreadCreate(wrongThreadDispatch,data,0);assert(thread);
    assert(xrtThreadWaitFor(thread,10000)==XWAIT_OK && xrtThreadExitCode(thread)==0);xrtThreadDestroy(thread);
    assert(!owner.calls && !messages && owner.refs==2 && owner.holds==1);
    assert(xuiDestroyChecked(context)==XUI_OK && webDrop(&owner));
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&backend));
}
static void contextGone(void* raw){++*(unsigned*)raw;}
static void delegateKeepsContextCarrier(void)
{
    xui_context context=NULL;xui_widget widget=createWeb(&context);xui_webview_data* data=webview_get(widget);
    unsigned destroyed=0;xui_lifetime_link_t watch={0};assert(xuiContextLifetimeWatch(context,&watch,contextGone,&destroyed)==XUI_OK);
    webview_retain(data);bool consumed=true;
    assert(xuiDestroyOwnerChecked(context,&consumed)==XUI_STATUS_DEFERRED && !consumed);
    assert(!destroyed && data->widget==widget && data->retiring && data->refs==2);
    webview_emit(data,XUI_WEBVIEW_EVENT_READY,0);webview_deliver_message(data,"about:blank","{}",2);
    webview_release(data);
    /* Delegate Release never runs the last Widget/Context destructor from a
     * frame whose own COM-vtable code must remain resident. */
    assert(!destroyed && data->refs==1);
    consumed=false;assert(xuiDestroyOwnerChecked(context,&consumed)==XUI_OK && consumed && destroyed==1 && !watch.ppPrevious);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    ownedEvent(false);ownedEvent(true);requestedHold();failedDrop();queuedDelegate();creatorAdmission();delegateKeepsContextCarrier();
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("WebView typed Event owner snapshots and real message physical tail, self-clear/destroy, requested Hold, NULL cleanup and queued delegate suppression balanced");return 0;
}
