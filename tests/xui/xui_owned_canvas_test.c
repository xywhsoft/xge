/* Pure CPU providers. No XGE initialization, device, window, GPU or I/O. */
#define main owned_surface_baseline_main
#include "xui_owned_surfaces_test.c"
#undef main

typedef struct CanvasTest {
    xui_test_proxy_state_t base;
    unsigned created,freed,begun,ended,drawn,rawEnd,rawDestroy;
    int failCreate,failClear,failBegin,failEnd,failSurfaceFree;
    bool oomAfterCreate,clearFallback,canEnd,keepDraw,closeContext,reenter,queueDestroy;
    xui_widget canvas; xui_context context; xui_draw_context lastDraw;
    int reenterStatus;
} CanvasTest;
typedef struct CanvasSurface {CanvasTest* test;xui_surface_desc_t desc;} CanvasSurface;
typedef struct CanvasDraw {CanvasTest* test;} CanvasDraw;
static int canvasSurfaceFree(xui_proxy proxy,void* raw)
{(void)proxy;CanvasSurface* value=raw;++value->test->freed;return value->test->failSurfaceFree;}
static int canvasSurfaceCreate(xui_proxy proxy,xui_surface* out,const xui_surface_desc_t* desc)
{
    CanvasTest* t=proxy->pUser;void* raw=NULL;
    int status=xuiSurfaceCreateOwned(proxy,sizeof(CanvasSurface),canvasSurfaceFree,out,&raw);
    if(status!=XUI_OK)return status;
    *(CanvasSurface*)raw=(CanvasSurface){t,*desc};++t->created;
    if(t->reenter)t->reenterStatus=xuiCanvasClear(t->canvas,XUI_COLOR_WHITE);
    if(t->closeContext){t->closeContext=false;assert(xuiDestroyChecked(t->context)==XUI_STATUS_DEFERRED);}
    if(t->queueDestroy)assert(xuiWidgetSetEnabled(t->canvas,0)==XUI_OK);
    if(t->oomAfterCreate)assert(xrtMemDebugFailAfter(0));
    return t->failCreate;
}
static int canvasSurfaceDesc(xui_proxy proxy,xui_surface surface,xui_surface_desc_t* out)
{(void)proxy;*out=((CanvasSurface*)xuiInternalProxyResourcePayload(surface))->desc;return XUI_OK;}
static void canvasSurfaceDestroy(xui_proxy proxy,xui_surface surface)
{CanvasTest* t=proxy->pUser;++t->rawDestroy;assert(xuiSurfaceReleaseChecked(surface)==XUI_OK);}
static int canvasClear(xui_proxy proxy,xui_surface surface,uint32_t color)
{(void)surface;(void)color;CanvasTest* t=proxy->pUser;return t->clearFallback ? XUI_ERROR_UNSUPPORTED:t->failClear;}
static int canvasDrawEnd(xui_proxy proxy,void* raw)
{(void)proxy;CanvasTest* t=((CanvasDraw*)raw)->test;++t->ended;return t->failEnd;}
static bool canvasCanEnd(const void* raw)
{return ((const CanvasDraw*)raw)->test->canEnd;}
static int canvasDrawBegin(xui_proxy proxy,xui_draw_context* out,xui_surface target)
{
    CanvasTest* t=proxy->pUser;void* raw=NULL;
    int status=xuiDrawCreateOwnedEx(proxy,target,sizeof(CanvasDraw),canvasDrawEnd,canvasCanEnd,out,&raw);
    if(status!=XUI_OK)return status;
    *(CanvasDraw*)raw=(CanvasDraw){t};++t->begun;t->lastDraw=*out;
    if(t->keepDraw)assert(xuiDrawAddRef(*out)==XUI_OK);
    return t->failBegin;
}
static int canvasRawDrawEnd(xui_proxy proxy,xui_draw_context draw)
{CanvasTest* t=proxy->pUser;++t->rawEnd;return xuiDrawReleaseChecked(draw);}
static int canvasDrawClear(xui_proxy proxy,xui_draw_context draw,xui_rect_t rectangle,uint32_t color)
{(void)draw;(void)rectangle;(void)color;CanvasTest* t=proxy->pUser;++t->drawn;return t->failClear;}
static int canvasDrawPoint(xui_proxy proxy,xui_draw_context draw,float x,float y,float width,uint32_t color)
{(void)draw;(void)x;(void)y;(void)width;(void)color;CanvasTest* t=proxy->pUser;++t->drawn;return XUI_OK;}
static xui_proxy canvasProxy(CanvasTest* t,Owner* owner)
{
    xuiTestProxyInit(&t->base);t->canEnd=true;
    t->base.tProxy.surfaceCreate=canvasSurfaceCreate;t->base.tProxy.surfaceDestroy=canvasSurfaceDestroy;
    t->base.tProxy.surfaceGetDesc=canvasSurfaceDesc;t->base.tProxy.surfaceGetGeneration=generation;
    t->base.tProxy.surfaceClear=canvasClear;t->base.tProxy.drawBegin=canvasDrawBegin;
    t->base.tProxy.drawEnd=canvasRawDrawEnd;t->base.tProxy.drawClearRect=canvasDrawClear;
    t->base.tProxy.drawPoint=canvasDrawPoint;
    xui_callback_owner_t binding=ownerRef(owner);xui_proxy proxy=NULL;
    assert(xuiProxyCreateOwned(&t->base.tProxy,&binding,&proxy)==XUI_OK);
    assert(xuiCreateOwned(proxy,&t->context)==XUI_OK);
    assert(xuiCanvasCreate(t->context,&t->canvas,NULL)==XUI_OK);
    assert(!t->created);return proxy;
}
static void finishCanvas(CanvasTest* t,xui_proxy proxy,Owner* owner)
{
    assert(xuiDestroyChecked(t->context)==XUI_OK);t->context=NULL;
    assert(t->created==t->freed && t->begun==t->ended && !t->rawEnd && !t->rawDestroy);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && drop(owner));
}
static void basics(void)
{
    CanvasTest t={0};Owner owner={.refs=1};xui_proxy proxy=canvasProxy(&t,&owner);
    xui_surface initial=xuiCanvasGetSurface(t.canvas);assert(initial && t.created==1 && refs(initial)==1);
    assert(xuiSurfaceAddRef(initial)==XUI_OK);
    matching=xuiSurfaceOwnership(initial);matchingEdges=0;xrtownershipscope freeze={0};
    assert(xrtOwnershipFreezeTryBegin(&freeze) && xuiContextTraceOwnership(t.context,edge,NULL));
    assert(xrtOwnershipScopeEnd(&freeze) && matchingEdges==1);
    assert(xuiCanvasClear(t.canvas,XUI_COLOR_WHITE)==XUI_OK && t.created==1);
    t.keepDraw=true;assert(xuiCanvasDrawPoint(t.canvas,1,2,1,XUI_COLOR_WHITE)==XUI_OK);
    assert(t.ended==1 && xuiDrawEnded(t.lastDraw) && refs(initial)==2);
    assert(xuiDrawReleaseChecked(t.lastDraw)==XUI_OK);t.keepDraw=false;
    assert(xuiCanvasSetCanvasSize(t.canvas,32,48)==XUI_OK && t.created==2 && refs(initial)==1);
    float w=0,h=0;assert(xuiCanvasGetCanvasSize(t.canvas,&w,&h)==XUI_OK && w==32 && h==48);
    assert(xuiSurfaceReleaseChecked(initial)==XUI_OK && t.freed==1);
    finishCanvas(&t,proxy,&owner);
}
static void partialAndPrimary(void)
{
    for(unsigned mode=0;mode<4;++mode){
        CanvasTest t={0};Owner owner={.refs=1};xui_proxy proxy=canvasProxy(&t,&owner);
        if(mode==0){t.failCreate=-41;t.oomAfterCreate=true;}
        if(mode==1)t.failClear=-42;
        if(mode==2){t.clearFallback=true;t.failBegin=-44;}
        if(mode==3){t.clearFallback=true;t.failClear=-42;t.failEnd=-45;t.failSurfaceFree=-46;}
        int expected=mode==0 ? -41:mode==2 ? -44:-42;
        xui_call_scope_t outer={0};assert(xuiCallScopeBegin(t.context,&outer)==XUI_OK);
        assert(xuiCanvasClear(t.canvas,XUI_COLOR_WHITE)==expected);
        int tail=xuiCallScopeEnd(&outer);assert(tail==(mode==3 ? expected:XUI_OK));
        assert(xrtMemDebugFailAfter(-1));xrtClearError();
        assert(t.created==1 && t.freed==1 && t.begun==t.ended);
        t.failCreate=t.failClear=t.failBegin=t.failEnd=t.failSurfaceFree=0;t.oomAfterCreate=false;
        assert(xuiCanvasGetSurface(t.canvas));finishCanvas(&t,proxy,&owner);
    }
}
static void pendingDraw(void)
{
    CanvasTest t={0};Owner owner={.refs=1};xui_proxy proxy=canvasProxy(&t,&owner);
    xui_surface surface=xuiCanvasGetSurface(t.canvas);assert(surface);
    t.canEnd=false;
    assert(xuiCanvasDrawPoint(t.canvas,1,2,1,XUI_COLOR_WHITE)==XUI_ERROR_BUSY);
    assert(t.begun==1 && !t.ended && refs(surface)==2);
    matching=xuiDrawOwnership(t.lastDraw);matchingEdges=0;xrtownershipscope freeze={0};
    assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextOwnershipBusy(t.context) && !xuiContextTraceOwnership(t.context,edge,NULL));
    assert(t.canvas->pType->onTraceOwnership(t.canvas,t.canvas->pTypeData,edge,NULL));
    assert(xrtOwnershipScopeEnd(&freeze) && matchingEdges==1);
    assert(xuiCanvasClear(t.canvas,XUI_COLOR_WHITE)==XUI_ERROR_BUSY && t.begun==1);
    xrtClearError();t.canEnd=true;
    assert(xuiCanvasClear(t.canvas,XUI_COLOR_WHITE)==XUI_OK && t.ended==1 && refs(surface)==1);
    finishCanvas(&t,proxy,&owner);
}
static void reentryAndClose(void)
{
    CanvasTest t={0};Owner owner={.refs=1};xui_proxy proxy=canvasProxy(&t,&owner);
    t.reenter=true;assert(xuiCanvasGetSurface(t.canvas) && t.reenterStatus==XUI_ERROR_BUSY);
    t.reenter=false;finishCanvas(&t,proxy,&owner);
    CanvasTest closing={0};Owner other={.refs=1};proxy=canvasProxy(&closing,&other);
    closing.closeContext=true;
    assert(!xuiCanvasGetSurface(closing.canvas));xrtClearError();
    assert(xuiDestroyChecked(closing.context)==XUI_OK);
    assert(closing.created==closing.freed && xuiProxyReleaseChecked(proxy)==XUI_OK && drop(&other));
}
static int destroyOnDisabled(xui_widget widget,const xui_event_t* event,void* user)
{(void)user;if(event->iType==XUI_EVENT_ENABLED_CHANGED)xuiWidgetDestroy(widget);return XUI_OK;}
static void markInvalid(void* raw){++*(unsigned*)raw;}
static void scopeTailInvalidation(void)
{
    CanvasTest t={0};Owner owner={.refs=1};xui_proxy proxy=canvasProxy(&t,&owner);
    unsigned invalidated=0;xui_lifetime_link_t watch={0};
    assert(xuiWidgetLifetimeWatch(t.canvas,&watch,markInvalid,&invalidated)==XUI_OK);
    assert(xuiWidgetSetEventCallback(t.canvas,destroyOnDisabled,NULL)==XUI_OK);
    t.queueDestroy=true;assert(xuiCanvasGetSurface(t.canvas)==NULL);
    assert(invalidated==1 && t.created==1 && t.freed==1);
    xuiLifetimeUnwatch(&watch);finishCanvas(&t,proxy,&owner);
}
static int32 wrongThreadCanvas(ptr raw)
{
    CanvasTest* t=raw;assert(xrtMemDebugFailAfter(0));
    assert(xuiCanvasGetSurface(t->canvas)==NULL);
    assert(xuiCanvasClear(t->canvas,XUI_COLOR_WHITE)==XUI_ERROR_BUSY);
    assert(xuiCanvasSetCanvasSize(t->canvas,20,30)==XUI_ERROR_BUSY);
    assert(xuiCanvasDrawPoint(t->canvas,1,2,1,XUI_COLOR_WHITE)==XUI_ERROR_BUSY);
    assert(!xrtMemDebugFailTriggered() && !xrtGetError());xrtMemDebugFailClear();return 0;
}
static void creatorAffinity(void)
{
    CanvasTest t={0};Owner owner={.refs=1};xui_proxy proxy=canvasProxy(&t,&owner);
    xthread* worker=xrtThreadCreate(wrongThreadCanvas,&t,0);assert(worker);
    assert(xrtThreadWaitFor(worker,10000)==XWAIT_OK);xrtThreadDestroy(worker);
    assert(!t.created && !t.begun && !t.context->iOperationDepth && !t.context->pCallScopes);
    finishCanvas(&t,proxy,&owner);
}
static void resizeOom(void)
{
    int accepted=0,rejected=0;
    for(int budget=0;budget<48;++budget){
        CanvasTest t={0};Owner owner={.refs=1};xui_proxy proxy=canvasProxy(&t,&owner);
        xui_surface old=xuiCanvasGetSurface(t.canvas);assert(old && xuiSurfaceAddRef(old)==XUI_OK);
        assert(xrtMemDebugFailAfter(budget));int status=xuiCanvasSetCanvasSize(t.canvas,23,47);
        assert(xrtMemDebugFailAfter(-1));xrtClearError();float w=0,h=0;
        assert(xuiCanvasGetCanvasSize(t.canvas,&w,&h)==XUI_OK);
        if(status==XUI_OK){++accepted;assert(w==23 && h==47 && refs(old)==1);}
        else {++rejected;assert(w==640 && h==480 && xuiCanvasGetSurface(t.canvas)==old && refs(old)==2);}
        assert(xuiSurfaceReleaseChecked(old)==XUI_OK);finishCanvas(&t,proxy,&owner);
    }
    assert(accepted && rejected);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    basics();partialAndPrimary();pendingDraw();reentryAndClose();scopeTailInvalidation();creatorAffinity();resizeOom();
    xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes && before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("Canvas actual owning slots: creation/resize/partial/OOM/Draw End/retained aliases/busy retry/trace/reentry/requested close passed");return 0;
}
