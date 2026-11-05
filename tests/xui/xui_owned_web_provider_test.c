/* Actual Provider + WebView + Context manager directory, CPU fake render I/O.
 * No InitializeAsync, browser, engine, window, GPU or network is started. */
#define XUI_OWNED_LAYOUT_HELPERS
#include "xui_owned_layout_test.c"
#include "../../include/xui_document_ui.h"
#include "xui_proxy_resource.h"
#define XUI_WEBVIEW_TEST_EXPORTS
#include "../../src/xui/document/xui_webview.c"
static int cpuWorkerStart(xui_widget widget){assert(webview_get(widget));return XUI_OK;}
#define xuiWebViewInitializeAsync cpuWorkerStart
#include "../../src/xui/document/xui_document_web_provider.c"
#undef xuiWebViewInitializeAsync

typedef struct ProviderTest {
    LayoutProvider base;
    xui_doc_web_provider provider;
    unsigned surfaceDrops,invalidations;
    int surfaceStatus,measureStatus;
    bool closeOnLoad,failLoad,nestedDrop,swapFont;
    xui_font nextFont;
} ProviderTest;
typedef struct CpuSurface {ProviderTest* test;xui_surface_desc_t desc;int status;} CpuSurface;
static ProviderTest* current;
static int cpuSurfaceDrop(xui_proxy proxy,void* raw)
{
    CpuSurface* surface=raw;ProviderTest* test=surface->test;(void)proxy;++test->surfaceDrops;
    if(test->nestedDrop){
        test->nestedDrop=false;
        assert(xuiDocumentWebProviderSetPalette(test->provider,XUI_COLOR_RGBA(0,0,0,255),XUI_COLOR_WHITE)==XUI_ERROR_BUSY);
    }
    return surface->status;
}
static int cpuSurfaceCreate(xui_proxy proxy,xui_surface* output,const xui_surface_desc_t* desc)
{
    void* raw=NULL;int status=xuiSurfaceCreateOwned(proxy,sizeof(CpuSurface),cpuSurfaceDrop,output,&raw);
    if(status==XUI_OK)*(CpuSurface*)raw=(CpuSurface){current,*desc,current->surfaceStatus};
    return status;
}
static int cpuSurfaceLoad(xui_proxy proxy,xui_surface* output,const void* encoded,int bytes,uint32_t flags)
{
    (void)flags;assert(encoded && bytes==1 && *(const unsigned char*)encoded==0x7f);
    xui_surface_desc_t desc={.iKind=XUI_SURFACE_KIND_TEXTURE,.iFormat=XUI_SURFACE_FORMAT_RGBA8,.iWidth=512,.iHeight=350};
    int status=cpuSurfaceCreate(proxy,output,&desc);
    if(current->closeOnLoad){current->closeOnLoad=false;bool consumed=false;assert(xuiDocumentWebProviderReleaseOwnerChecked(current->provider,&consumed)==XUI_OK && consumed);}
    /* Reentrant close cannot free the borrowed request buffer mid decoder. */
    assert(*(const unsigned char*)encoded==0x7f);
    return status==XUI_OK && current->failLoad ? XUI_ERROR_OUT_OF_MEMORY:status;
}
static int cpuSurfaceDesc(xui_proxy proxy,xui_surface surface,xui_surface_desc_t* output)
{assert(xuiSurfaceGetProxy(surface)==proxy);*output=((CpuSurface*)xuiInternalProxyResourcePayload(surface))->desc;return XUI_OK;}
static int cpuDrawSurface(xui_proxy proxy,xui_draw_context draw,xui_surface surface,xui_rect_t from,xui_rect_t to,uint32_t color,uint32_t flags)
{(void)draw;(void)from;(void)to;(void)color;(void)flags;assert(xuiSurfaceGetProxy(surface)==proxy);return XUI_OK;}
static int cpuSurfaceDrawTo(xui_proxy proxy,xui_surface target,xui_surface source,xui_rect_t from,xui_rect_t to,uint32_t color,uint32_t flags)
{assert(xuiSurfaceGetProxy(target)==proxy);return cpuDrawSurface(proxy,NULL,source,from,to,color,flags);}
static int cpuMeasure(xui_proxy proxy,const xui_text_item_t* item,xui_vec2_t* size)
{
    if(current->swapFont){
        current->swapFont=false;assert(xuiSetDefaultFont(current->base.context,current->nextFont)==XUI_OK);
        assert(current->base.fontDrops==0 && refs(xuiFontOwnership(item->pFont))==1 && xuiFontGetProxy(item->pFont)==proxy);
    }
    if(current->measureStatus)return current->measureStatus;
    size->fX=(item->iTextSize<0 ? (float)strlen(item->sText):(float)item->iTextSize)*8;size->fY=16;return XUI_OK;
}
static void providerSetup(ProviderTest* test)
{
    memset(test,0,sizeof(*test));current=test;test->base.owner.refs=1;
    xuiTestProxyInit(&test->base.state);xui_proxy_t* table=&test->base.state.tProxy;
    table->surfaceLoadMemory=cpuSurfaceLoad;table->surfaceCreate=cpuSurfaceCreate;
    table->textMeasure=cpuMeasure;table->fontGetMetrics=metrics;table->surfaceGetDesc=cpuSurfaceDesc;table->surfaceDrawTo=cpuSurfaceDrawTo;table->drawSurface=cpuDrawSurface;
    xui_callback_owner_t owner=ownerRef(&test->base.owner);
    assert(xuiProxyCreateOwned(table,&owner,&test->base.proxy)==XUI_OK && xuiCreateOwned(test->base.proxy,&test->base.context)==XUI_OK);
    xui_widget parent=NULL;assert(xuiWidgetCreate(test->base.context,&parent)==XUI_OK);
    xui_doc_web_provider_desc_t desc={.iSize=sizeof(desc),.pContext=test->base.context,.pParent=parent,.sAssetsFolder="cpu-local-assets"};
    assert(xuiDocumentWebProviderCreate(&desc,&test->provider)==XUI_OK);
    assert(test->provider->proxy==test->base.context->pOwnedProxy && test->provider->proxy==test->base.proxy);
}
static void providerFinish(ProviderTest* test)
{
    assert(xuiDestroyChecked(test->base.context)==XUI_OK);
    assert(xuiProxyReleaseChecked(test->base.proxy)==XUI_OK && drop(&test->base.owner));
}
static void queuedCapture(ProviderTest* test,bool crop)
{
    xui_doc_web_provider provider=test->provider;provider->page_ready=1;provider->stage=DOC_WEB_WAIT_CAPTURE;
    provider->entry_count=1;provider->active=&provider->entries[0];
    provider->active->state=DOC_WEB_RENDERING;provider->active->width=crop ? 32:512;provider->active->height=crop ? 24:350;
    xui_web_request request=calloc(1,sizeof(*request));assert(request);request->refs=1;request->kind=WEB_REQUEST_PNG;
    request->state=XUI_WEB_REQUEST_COMPLETED;request->result=malloc(1);assert(request->result);*(unsigned char*)request->result=0x7f;request->bytes=1;
    provider->capture=request;
}
static xrtownershipref surfaceTarget;static unsigned surfaceEdges;
static bool providerVisit(xrtownershipref ref,void* user)
{(void)user;if(ref.Data==surfaceTarget.Data && ref.Ops==surfaceTarget.Ops)++surfaceEdges;return true;}
static void actualSurface(bool crop)
{
    ProviderTest test;providerSetup(&test);queuedCapture(&test,crop);
    int updated=xuiDocumentWebProviderUpdate(test.provider);
    assert(updated==XUI_OK && test.provider->entries[0].surface);
    xui_surface surface=test.provider->entries[0].surface;surfaceTarget=xuiSurfaceOwnership(surface);surfaceEdges=0;
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceOwnership(test.base.context,providerVisit,NULL) && surfaceEdges==1);assert(xrtOwnershipScopeEnd(&freeze));
    unsigned before=test.surfaceDrops;test.nestedDrop=true;
    assert(xuiDocumentWebProviderSetPalette(test.provider,XUI_COLOR_RGBA(0,0,0,255),XUI_COLOR_WHITE)==XUI_OK && !test.nestedDrop && test.surfaceDrops==before+1);
    assert(xuiDocumentWebProviderReleaseChecked(test.provider)==XUI_OK);providerFinish(&test);
    assert(test.surfaceDrops==(crop ? 2u:1u));
}
static void partialOutputFailure(void)
{
    ProviderTest test;providerSetup(&test);queuedCapture(&test,false);test.failLoad=true;test.surfaceStatus=XUI_ERROR_BACKEND_FAILED;
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(test.base.context,&outer)==XUI_OK);
    assert(xuiDocumentWebProviderUpdate(test.provider)==XUI_ERROR_OUT_OF_MEMORY && !xrtGetError() && test.surfaceDrops==1 && !test.provider->pending_surface);
    assert(xuiCallScopeEnd(&outer)==XUI_ERROR_OUT_OF_MEMORY && !xrtGetError());
    assert(xuiDocumentWebProviderReleaseChecked(test.provider)==XUI_OK);providerFinish(&test);
}
static void providerGone(void* raw){++*(unsigned*)raw;}
static void errorCardFontTail(void)
{
    ProviderTest test;providerSetup(&test);queuedCapture(&test,false);
    xui_font old=fontCreate(test.base.proxy,&test.base.fontDrops,XUI_ERROR_BACKEND_FAILED);
    test.nextFont=fontCreate(test.base.proxy,&test.base.fontDrops,XUI_OK);
    assert(xuiSetDefaultFont(test.base.context,old)==XUI_OK && xuiFontReleaseChecked(old)==XUI_OK);
    test.failLoad=test.swapFont=true;xui_call_scope_t outer={0};assert(xuiCallScopeBegin(test.base.context,&outer)==XUI_OK);
    assert(xuiDocumentWebProviderUpdate(test.provider)==XUI_ERROR_OUT_OF_MEMORY && !test.swapFont && test.base.fontDrops==1 && !xrtGetError());
    assert(xuiCallScopeEnd(&outer)==XUI_ERROR_OUT_OF_MEMORY && !xrtGetError());
    assert(xuiDocumentWebProviderReleaseChecked(test.provider)==XUI_OK);
    assert(xuiDestroyChecked(test.base.context)==XUI_OK && xuiFontReleaseChecked(test.nextFont)==XUI_OK && test.base.fontDrops==2);
    assert(xuiProxyReleaseChecked(test.base.proxy)==XUI_OK && drop(&test.base.owner));
}
static void errorCardFailurePrefix(void)
{
    ProviderTest test;providerSetup(&test);
    xui_font old=fontCreate(test.base.proxy,&test.base.fontDrops,XUI_ERROR_BACKEND_FAILED);
    test.nextFont=fontCreate(test.base.proxy,&test.base.fontDrops,XUI_OK);
    assert(xuiSetDefaultFont(test.base.context,old)==XUI_OK && xuiFontReleaseChecked(old)==XUI_OK);
    test.swapFont=true;test.measureStatus=XUI_ERROR_UNSUPPORTED;
    doc_web_entry entry={.available=200,.zoom=1};strcpy(entry.error_text,"error");
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(test.base.context,&outer)==XUI_OK);
    xui_manager_scope_t manager={0};assert(xuiInternalManagerScopeBegin(&test.provider->owned,&manager)==XUI_OK);
    int status=doc_web_prepare_error_card(test.provider,&entry);
    assert(status==XUI_ERROR_UNSUPPORTED && !entry.height && test.base.fontDrops==1 && !xrtGetError());
    assert(xuiInternalManagerScopeEnd(&manager,status)==XUI_ERROR_UNSUPPORTED && !xrtGetError());
    assert(xuiCallScopeEnd(&outer)==XUI_ERROR_UNSUPPORTED && !xrtGetError());
    test.measureStatus=0;assert(xuiInternalManagerScopeBegin(&test.provider->owned,&manager)==XUI_OK);
    assert(doc_web_prepare_error_card(test.provider,&entry)==XUI_OK && entry.height>0);
    assert(xuiInternalManagerScopeEnd(&manager,XUI_OK)==XUI_OK);
    assert(xuiDocumentWebProviderReleaseChecked(test.provider)==XUI_OK);
    assert(xuiDestroyChecked(test.base.context)==XUI_OK && xuiFontReleaseChecked(test.nextFont)==XUI_OK && test.base.fontDrops==2);
    assert(xuiProxyReleaseChecked(test.base.proxy)==XUI_OK && drop(&test.base.owner));
}
static void soleCleanupFailure(void)
{
    ProviderTest test;providerSetup(&test);queuedCapture(&test,false);test.surfaceStatus=XUI_ERROR_BACKEND_FAILED;
    assert(xuiDocumentWebProviderUpdate(test.provider)==XUI_OK);
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(test.base.context,&outer)==XUI_OK);
    assert(xuiDocumentWebProviderSetPalette(test.provider,XUI_COLOR_RGBA(0,0,0,255),XUI_COLOR_WHITE)==XUI_ERROR_BACKEND_FAILED && !xrtGetError());
    assert(xuiCallScopeEnd(&outer)==XUI_ERROR_BACKEND_FAILED && !xrtGetError() && test.surfaceDrops==1);
    assert(xuiDocumentWebProviderReleaseChecked(test.provider)==XUI_OK);providerFinish(&test);
}
static void constructorOom(void)
{
    for(size_t budget=0;budget<32;++budget){
        ProviderTest test;providerSetup(&test);xui_widget parent=test.provider->parent;
        assert(xuiDocumentWebProviderReleaseChecked(test.provider)==XUI_OK);
        xui_doc_web_provider_desc_t desc={.iSize=sizeof(desc),.pContext=test.base.context,.pParent=parent,.sAssetsFolder="cpu-local-assets"};
        xui_doc_web_provider candidate=(void*)(uintptr_t)1;
        assert(xrtMemDebugFailAfter(budget));int status=xuiDocumentWebProviderCreate(&desc,&candidate);xrtMemDebugFailClear();xrtClearError();
        if(status==XUI_OK)assert(candidate && xuiDocumentWebProviderReleaseChecked(candidate)==XUI_OK);
        else assert(candidate==NULL);
        assert(!test.base.context->pOwnedManagers && !test.base.context->pPendingManagers && !test.base.context->pDeferredDestroyHead);
        providerFinish(&test);
    }
}
static void destroyedDuringDecode(void)
{
    ProviderTest test;providerSetup(&test);queuedCapture(&test,false);
    unsigned invalidations=0;xui_lifetime_link_t watch={0};
    assert(xuiDocumentWebProviderLifetimeWatch(test.provider,&watch,providerGone,&invalidations)==XUI_OK);
    test.closeOnLoad=true;assert(xuiDocumentWebProviderUpdate(test.provider)==XUI_ERROR_INVALID_STATE && !test.closeOnLoad && test.surfaceDrops==1);
    assert(invalidations==1 && !watch.ppPrevious && !test.base.context->pOwnedManagers && !test.base.context->pPendingManagers);providerFinish(&test);
}
static void invalidateClose(void* raw)
{
    ProviderTest* test=raw;++test->invalidations;assert(xuiDocumentWebProviderReleaseChecked(test->provider)==XUI_OK);
}
static void callbackAndQueuedWorker(void)
{
    ProviderTest test;providerSetup(&test);
    xui_callback_owner_t owner=ownerRef(&test.base.owner);
    assert(xuiDocumentWebProviderSetInvalidateOwned(test.provider,invalidateClose,&test,&owner)==XUI_OK);
    xui_webview_data* data=webview_get(test.provider->worker);webview_retain(data);
    assert(xuiDocumentWebProviderSetPalette(test.provider,XUI_COLOR_RGBA(0,0,0,255),XUI_COLOR_WHITE)==XUI_ERROR_INVALID_STATE && test.invalidations==1);
    assert(test.base.context->pOwnedManagers && test.base.context->pPendingManagers &&
        data->widget && data->retiring && data->state==XUI_WEBVIEW_CLOSED);
    /* The actual late delegate carries a stale provider userdata pointer.
     * Dispatch must reject its dead Widget before reading that pointer. */
    webview_emit(data,XUI_WEBVIEW_EVENT_READY,0);webview_deliver_message(data,"about:blank","{}",2);
    webview_release(data);
    assert(test.base.context->pOwnedManagers); /* no finalize from delegate tail */
    xui_call_scope_t boundary={0};assert(xuiCallScopeBegin(test.base.context,&boundary)==XUI_OK && xuiCallScopeEnd(&boundary)==XUI_OK);
    assert(!test.base.context->pOwnedManagers);providerFinish(&test);
}
static void parentClose(void)
{
    ProviderTest test;providerSetup(&test);xui_widget parent=test.provider->parent;
    assert(xuiWidgetDestroyChecked(test.provider->worker)==XUI_ERROR_INVALID_STATE);
    assert(xuiWidgetDestroyChecked(parent)==XUI_OK && !test.base.context->pOwnedManagers);
    providerFinish(&test);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    actualSurface(false);actualSurface(true);partialOutputFailure();errorCardFontTail();errorCardFailurePrefix();soleCleanupFailure();constructorOom();destroyedDuringDecode();callbackAndQueuedWorker();parentClose();
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("WebProvider actual Context-owned manager, canonical Proxy and Surface graph, partial output NULL failure, reentrant close and late COM-delegate suppression balanced");return 0;
}



