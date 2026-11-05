/* Actual native layouts with successful busy paints; no engine/device init. */
#define XUI_OWNED_LAYOUT_HELPERS
#include "xui_owned_layout_test.c"
static void finish(LayoutProvider* p,xui_font font)
{
    p->ready=true;p->paintStatus=0;
    if(p->layout){assert(xuiTextLayoutDestroyChecked(p->layout)==XUI_OK);p->layout=NULL;}
    assert(xuiDestroyChecked(p->context)==XUI_OK);p->context=NULL;
    assert(xuiFontReleaseChecked(font)==XUI_OK && p->fontDrops==1);
    assert(xuiProxyReleaseChecked(p->proxy)==XUI_OK && drop(&p->owner));
    assert(p->paintDrops==p->shapes);
}
static void resetOOMRetainsDraft(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);
    xui_text_layout_desc_t desc=layoutDesc(font,"active");
    assert(xuiTextLayoutCreate(p.context,&p.layout,&desc)==XUI_OK);
    const char* active=xuiTextLayoutGetText(p.layout);
    p.busyNextShape=p.oomAfterShape=true;desc.sText="draft";
    assert(xuiTextLayoutReset(p.layout,&desc)==XUI_ERROR_OUT_OF_MEMORY);xrtMemDebugFailClear();xrtClearError();
    assert(xuiTextLayoutGetText(p.layout)==active && !strcmp(active,"active") && p.shapes==2 && !p.paintDrops);
    unsigned edges=0;assert(xuiTextLayoutTraceOwnership(p.layout,layoutVisit,&edges) && edges==4);
    for(int i=0;i<4;++i)assert(xuiTextLayoutReset(p.layout,&desc)==XUI_ERROR_BUSY && p.shapes==2);
    xui_call_scope_t outer={0};assert(xuiCallScopeBegin(p.context,&outer)==XUI_OK);
    bool consumed=true;assert(xuiTextLayoutDestroyOwnerChecked(p.layout,&consumed)==XUI_STATUS_DEFERRED && !consumed);
    assert(!xrtGetError() && xuiCallScopeEnd(&outer)==XUI_OK && !xrtGetError());
    finish(&p,font);
}
static void constructorKeepsOwner(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);p.busyNextShape=p.oomAfterShape=true;
    xui_text_layout_desc_t desc=layoutDesc(font,"created");
    assert(xuiTextLayoutCreate(p.context,&p.layout,&desc)==XUI_ERROR_OUT_OF_MEMORY && p.layout);
    xrtMemDebugFailClear();xrtClearError();bool consumed=true;
    assert(xuiTextLayoutDestroyOwnerChecked(p.layout,&consumed)==XUI_STATUS_DEFERRED && !consumed && !p.paintDrops);
    finish(&p,font);
}
static void lateWaitAndNullFailure(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);xui_text_layout_desc_t desc=layoutDesc(font,"active");
    assert(xuiTextLayoutCreate(p.context,&p.layout,&desc)==XUI_OK);
    p.busyNextShape=p.oomAfterShape=true;desc.sText="pending";
    assert(xuiTextLayoutReset(p.layout,&desc)==XUI_ERROR_OUT_OF_MEMORY);xrtMemDebugFailClear();xrtClearError();
    p.ready=true;p.dropMakesBusy=true;p.paintStatus=XUI_ERROR_UNSUPPORTED;
    bool consumed=true;assert(xuiTextLayoutDestroyOwnerChecked(p.layout,&consumed)==XUI_ERROR_UNSUPPORTED && !consumed && !xrtGetError());
    assert(p.paintDrops==1);unsigned edges=0;
    assert(xuiTextLayoutTraceOwnership(p.layout,layoutVisit,&edges) && edges==2);
    finish(&p,font);
}
static void measureCacheKeepsOwner(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);xui_text_layout_desc_t desc=layoutDesc(font,"measure");
    xui_vec2_t size={0};assert(xuiTextMeasureLayout(p.context,&desc,&size)==XUI_OK && size.fX==56);
    p.busyNextShape=p.oomAfterShape=true;desc.sText="bad draft";
    assert(xuiTextMeasureLayout(p.context,&desc,&size)==XUI_ERROR_OUT_OF_MEMORY);xrtMemDebugFailClear();xrtClearError();
    assert(p.shapes==2 && !p.paintDrops);
    bool consumed=true;assert(xuiDestroyOwnerChecked(p.context,&consumed)==XUI_STATUS_DEFERRED && !consumed && !xrtGetError());
    p.ready=true;assert(xuiDestroyChecked(p.context)==XUI_OK);p.context=NULL;
    assert(p.paintDrops==2 && xuiFontReleaseChecked(font)==XUI_OK && p.fontDrops==1);
    assert(xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner));
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    resetOOMRetainsDraft();constructorKeepsOwner();lateWaitAndNullFailure();measureCacheKeepsOwner();
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("layout draft: successful-busy/OOM/late-wait/NULL-failure/measurement actual owners balanced");return 0;
}
