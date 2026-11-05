/* CPU-only actual native unique Layout lifetime and resource ownership. */
#define main xui_unused_owned_fonts_main
#include "xui_owned_fonts_test.c"
#undef main

typedef struct LayoutProvider {
    xui_test_proxy_state_t state;
    Owner owner;
    xui_proxy proxy;
    xui_context context;
    xui_text_layout layout;
    unsigned fontDrops,paintDrops,shapes,draws,invalidations;
    bool ready,closeDuringDraw,busyNextShape,oomAfterShape,dropMakesBusy;
    int paintStatus;
    xerror* paintError;
} LayoutProvider;
typedef struct LayoutPaint {xui_font font;LayoutProvider* provider;} LayoutPaint;
static bool layoutPaintReady(const void* raw){return ((const LayoutPaint*)raw)->provider->ready;}
static bool layoutPaintTrace(const void* raw,xrtownershipvisitor visit,void* user)
{return visit(xuiFontOwnership(((const LayoutPaint*)raw)->font),user);}
static int layoutPaintDrop(void* raw)
{
    LayoutPaint* paint=raw;LayoutProvider* p=paint->provider;++p->paintDrops;
    assert(xuiFontReleaseChecked(paint->font)==XUI_OK);xrtFree(paint);
    if(p->dropMakesBusy){p->dropMakesBusy=false;p->ready=false;}
    if(p->paintError)xrtSetErrorTake(xrtErrorRef(p->paintError));
    return p->paintStatus;
}
static const xui_text_paint_ownership_v1_t layoutPaintOps={sizeof(layoutPaintOps),XUI_TEXT_PAINT_OWNERSHIP_V1_TAG,layoutPaintReady,layoutPaintTrace,layoutPaintDrop};
static int layoutShape(xui_proxy proxy,const xui_text_item_t* item,xui_text_shape_t* shape)
{
    LayoutProvider* p=proxy->pUser;int n=item->iTextSize<0 ? (int)strlen(item->sText):item->iTextSize;
    *shape=(xui_text_shape_t){.iSize=sizeof(*shape),.iTextSize=n,.iFlags=item->iFlags,.fAscent=12,.fDescent=4,.fLineHeight=16,.fHeight=16,.fWidth=n*8};
    if(n){shape->pClusters=xrtCalloc(n,sizeof(*shape->pClusters));if(!shape->pClusters)return XUI_ERROR_OUT_OF_MEMORY;}
    shape->iClusterCount=n;
    for(int i=0;i<n;++i)shape->pClusters[i]=(xui_text_cluster_t){.iSize=sizeof(xui_text_cluster_t),.iTextStart=i,.iTextEnd=i+1,.fAdvance=8};
    LayoutPaint* paint=xrtMalloc(sizeof(*paint));if(!paint){xuiTextShapeFree(shape);return XUI_ERROR_OUT_OF_MEMORY;}
    int status=xuiFontAddRef(item->pFont);if(status!=XUI_OK){xrtFree(paint);xuiTextShapeFree(shape);return status;}
    *paint=(LayoutPaint){item->pFont,p};shape->pPaint=paint;shape->pPaintOwnership=&layoutPaintOps;++p->shapes;
    if(p->busyNextShape){p->busyNextShape=false;p->ready=false;}
    if(p->oomAfterShape){p->oomAfterShape=false;assert(xrtMemDebugFailAfter(0));}
    return XUI_OK;
}
static int layoutDraw(xui_proxy proxy,xui_surface target,const xui_text_item_t* item,xui_rect_t rect,uint32_t color,uint32_t flags)
{
    (void)rect;(void)color;(void)flags;LayoutProvider* p=proxy->pUser;
    assert(xuiSurfaceGetProxy(target) && item->sText && item->pFont);++p->draws;
    assert(!strcmp(xuiTextLayoutGetText(p->layout),"abc") && xuiTextLayoutGetSize(p->layout).fX==24);
    if(p->closeDuringDraw){
        bool consumed=true;assert(xuiTextLayoutDestroyOwnerChecked(p->layout,&consumed)==XUI_STATUS_DEFERRED && !consumed);
        assert(xuiDestroyChecked(p->context)==XUI_STATUS_DEFERRED);
        assert(xuiTextLayoutGetContext(p->layout)==p->context);
    }
    return XUI_OK;
}
static void layoutInvalidated(void* raw)
{
    LayoutProvider* p=raw;++p->invalidations;bool consumed=true;
    assert(xuiTextLayoutDestroyOwnerChecked(p->layout,&consumed)==XUI_ERROR_INVALID_STATE && !consumed);
}
static void layoutSetup(LayoutProvider* p,xui_font* font)
{
    p->ready=true;p->owner.refs=1;xuiTestProxyInit(&p->state);
    p->state.tProxy.pUser=p;p->state.tProxy.fontGetMetrics=metrics;p->state.tProxy.textMeasure=measure;
    p->state.tProxy.textShape=layoutShape;p->state.tProxy.textDraw=layoutDraw;
    xui_callback_owner_t binding=ownerRef(&p->owner);
    assert(xuiProxyCreateOwned(&p->state.tProxy,&binding,&p->proxy)==XUI_OK);
    assert(xuiCreate(&p->context)==XUI_OK && xuiSetProxyOwned(p->context,p->proxy)==XUI_OK);
    *font=fontCreate(p->proxy,&p->fontDrops,XUI_OK);
}
static xui_text_layout_desc_t layoutDesc(xui_font font,const char* text)
{return (xui_text_layout_desc_t){.iSize=sizeof(xui_text_layout_desc_t),.pFont=font,.sText=text,.iTextSize=-1,.iWrapMode=XUI_TEXT_WRAP_NONE,.fMaxWidth=1000,.fMaxHeight=1000};}
static bool layoutVisit(xrtownershipref ref,void* raw){assert(ref.Data && ref.Ops);++*(unsigned*)raw;return true;}
static void layoutBasicAndReset(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);xui_text_layout_desc_t desc=layoutDesc(font,"abcde");
    assert(xuiTextLayoutCreate(p.context,&p.layout,&desc)==XUI_OK && refs(xuiFontOwnership(font))==3);
    unsigned edges=0;xrtownershipscope frozen={0};assert(xrtOwnershipFreezeTryBegin(&frozen));
    assert(xuiTextLayoutTraceOwnership(p.layout,layoutVisit,&edges) && edges==2);assert(xrtOwnershipScopeEnd(&frozen));
    const char* previous=xuiTextLayoutGetText(p.layout);assert(!strcmp(previous,"abcde"));
    p.ready=false;desc.sText="uncommitted";assert(xuiTextLayoutReset(p.layout,&desc)==XUI_ERROR_BUSY);
    assert(xuiTextLayoutGetText(p.layout)==previous && !p.paintDrops && p.shapes==1);p.ready=true;
    desc.sText=previous+1;assert(xuiTextLayoutReset(p.layout,&desc)==XUI_OK && !strcmp(xuiTextLayoutGetText(p.layout),"bcde"));
    assert(p.paintDrops==1 && refs(xuiFontOwnership(font))==3);
    unsigned failures=0,successes=0;
    for(int budget=0;budget<16;++budget){
        const char* before=xuiTextLayoutGetText(p.layout);desc.sText="0123456789";
        assert(xrtMemDebugFailAfter(budget));int status=xuiTextLayoutReset(p.layout,&desc);xrtMemDebugFailClear();xrtClearError();
        if(status!=XUI_OK){++failures;assert(xuiTextLayoutGetText(p.layout)==before);}else{++successes;assert(!strcmp(xuiTextLayoutGetText(p.layout),desc.sText));}
        assert(refs(xuiFontOwnership(font))==3);
    }
    assert(failures && successes);
    xui_lifetime_link_t watch={0};assert(xuiTextLayoutLifetimeWatch(p.layout,&watch,layoutInvalidated,&p)==XUI_OK);
    assert(xuiDestroyChecked(p.context)==XUI_OK && xuiTextLayoutGetContext(p.layout)==NULL && !p.invalidations);
    assert(xuiTextLayoutGetText(p.layout)==NULL && xuiTextLayoutReset(p.layout,&desc)==XUI_ERROR_INVALID_STATE);
    assert(xuiTextLayoutDestroyChecked(p.layout)==XUI_OK && p.invalidations==1 && !watch.ppPrevious);
    assert(refs(xuiFontOwnership(font))==1 && p.paintDrops==p.shapes);
    assert(xuiFontReleaseChecked(font)==XUI_OK && p.fontDrops==1 && xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner) && !p.owner.refs);
}
static int layoutSurfaceFinalize(xui_proxy proxy,void* raw){(void)proxy;(void)raw;return XUI_OK;}
static void layoutDrawReentry(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);xui_text_layout_desc_t desc=layoutDesc(font,"abc");
    assert(xuiTextLayoutCreate(p.context,&p.layout,&desc)==XUI_OK);
    xui_surface target=NULL;void* payload=NULL;assert(xuiSurfaceCreateOwned(p.proxy,0,layoutSurfaceFinalize,&target,&payload)==XUI_OK);
    assert(xuiTextLayoutDraw(p.layout,target,(xui_rect_t){0,0,200,40},0,0)==XUI_OK && p.draws==1);
    p.closeDuringDraw=true;assert(xuiTextLayoutDraw(p.layout,target,(xui_rect_t){0,0,200,40},0,0)==XUI_ERROR_INVALID_STATE && p.draws==2);
    xrtClearError();assert(xuiDestroyChecked(p.context)==XUI_OK && xuiTextLayoutGetContext(p.layout)==NULL);
    assert(xuiTextLayoutDestroyChecked(p.layout)==XUI_OK);
    assert(xuiSurfaceReleaseChecked(target)==XUI_OK && xuiFontReleaseChecked(font)==XUI_OK && xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner));
}
#ifndef XUI_OWNED_LAYOUT_HELPERS
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);layoutBasicAndReset();layoutDrawReentry();xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("unique Layout: true Font/paint ownership, alias-safe/OOM reset, BUSY preflight, context expiry, draw/close reentry passed");return 0;
}

#endif /* XUI_OWNED_LAYOUT_HELPERS */
