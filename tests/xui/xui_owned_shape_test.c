/* Actual Native owned Shape contract. CPU-only, no engine/window/GPU. */
#define XUI_OWNED_LAYOUT_HELPERS 1
#include "xui_owned_layout_test.c"
#undef XUI_OWNED_LAYOUT_HELPERS

static xui_text_item_t ownedShapeItem(xui_font font)
{return (xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,.sText="a\0b",.iTextSize=3,.iFlags=XUI_TEXT_SHAPE_RETAIN_PAINT};}
static bool ownedShapeVisit(xrtownershipref ref,void* raw)
{assert(ref.Data && ref.Ops);++*(unsigned*)raw;return true;}
static void ownedShapeBusy(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);p.ready=false;
    xui_text_item_t item=ownedShapeItem(font);const xui_text_shape_t* shape=NULL;
    assert(xuiTextShapeCreateOwned(p.proxy,&item,&shape)==XUI_OK && shape && shape->iTextSize==3 && shape->iClusterCount==3);
    xrtownershipref ref=xuiTextShapeOwnership(shape);
    assert((const unsigned char*)shape-(const unsigned char*)ref.Data==sizeof(xui_owned_resource_t));
    assert(sizeof(xui_owned_resource_t)==80 && xuiTextShapeGetProxy(shape)==p.proxy && refs(ref)==1);
    assert(refs(xuiFontOwnership(font))==2);
    bool consumed=true;assert(xuiTextShapeReleaseOwnerChecked(shape,&consumed)==XUI_STATUS_DEFERRED && !consumed && !p.paintDrops);
    assert(xuiTextShapeAddRef(shape)==XUI_OK && refs(ref)==2);
    assert(xuiTextShapeReleaseOwnerChecked(shape,&consumed)==XUI_OK && consumed && refs(ref)==1);
    unsigned edges=0;xrtownershipscope frozen={0};assert(xrtOwnershipFreezeTryBegin(&frozen));
    assert(ref.Ops->Trace(ref.Data,ownedShapeVisit,&edges) && edges==2);assert(xrtOwnershipScopeEnd(&frozen));
    assert(xuiDestroyChecked(p.context)==XUI_OK);
    assert(xuiFontReleaseChecked(font)==XUI_OK && !p.fontDrops);
    p.ready=true;assert(xuiTextShapeReleaseOwnerChecked(shape,&consumed)==XUI_OK && consumed && p.paintDrops==1 && p.fontDrops==1);
    assert(xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner));
}
static int ownedShapePartial(xui_proxy proxy,const xui_text_item_t* item,xui_text_shape_t* shape)
{int status=layoutShape(proxy,item,shape);return status==XUI_OK ? XUI_ERROR_BACKEND_FAILED:status;}
static void ownedShapePartialCarrier(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);p.ready=false;
    /* The capability table is immutable; create a second exact provider. */
    xui_test_proxy_state_t state=p.state;state.tProxy.textShape=ownedShapePartial;state.tProxy.pUser=&p;
    xui_callback_owner_t binding=ownerRef(&p.owner);
    xui_proxy partial=NULL;assert(xuiProxyCreateOwned(&state.tProxy,&binding,&partial)==XUI_OK);
    unsigned finalized=0;xui_font partialFont=fontCreate(partial,&finalized,XUI_OK);
    xui_text_item_t item=ownedShapeItem(partialFont);const xui_text_shape_t* shape=NULL;
    assert(xuiTextShapeCreateOwned(partial,&item,&shape)==XUI_ERROR_BACKEND_FAILED && shape && xrtGetError()==NULL);
    bool consumed=true;assert(xuiTextShapeReleaseOwnerChecked(shape,&consumed)==XUI_STATUS_DEFERRED && !consumed);
    p.ready=true;assert(xuiTextShapeReleaseChecked(shape)==XUI_OK && p.paintDrops==1);
    assert(xuiFontReleaseChecked(partialFont)==XUI_OK && finalized==1 && xuiProxyReleaseChecked(partial)==XUI_OK);
    assert(xuiDestroyChecked(p.context)==XUI_OK && xuiFontReleaseChecked(font)==XUI_OK && xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner));
}
static void ownedShapeOomAndNullCleanup(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);xui_text_item_t item=ownedShapeItem(font);
    unsigned failures=0,successes=0;
    for(int budget=0;budget<12;++budget){
        const xui_text_shape_t* shape=NULL;assert(xrtMemDebugFailAfter(budget));
        int status=xuiTextShapeCreateOwned(p.proxy,&item,&shape);xrtMemDebugFailClear();
        if(status!=XUI_OK)++failures;else ++successes;
        if(shape)assert(xuiTextShapeReleaseChecked(shape)==XUI_OK);
        xrtClearError();assert(refs(xuiFontOwnership(font))==1);
    }
    assert(failures && successes);
    const xui_text_shape_t* shape=NULL;assert(xuiTextShapeCreateOwned(p.proxy,&item,&shape)==XUI_OK);
    p.paintStatus=XUI_ERROR_BACKEND_FAILED;bool consumed=false;
    assert(xuiTextShapeReleaseOwnerChecked(shape,&consumed)==XUI_ERROR_BACKEND_FAILED && consumed && xrtGetError()==NULL);
    assert(xuiDestroyChecked(p.context)==XUI_OK && xuiFontReleaseChecked(font)==XUI_OK && xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner));
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    ownedShapeBusy();ownedShapePartialCarrier();ownedShapeOomAndNullCleanup();
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("owned immutable Shape: direct payload handle, true cap/paint graph, busy success and explicit failed carrier, OOM rollback and consumed NULL failure passed");return 0;
}
