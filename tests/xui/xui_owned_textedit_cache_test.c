/* Actual TextEdit persistent paint/layout carriers and pre-edit transactions. */
#define XUI_OWNED_LAYOUT_HELPERS
#include "xui_owned_layout_test.c"
static xui_widget actualEdit;
static bool attemptReentry,closeWidget;
static unsigned widgetInvalidations;
static void editGone(void* raw){++*(unsigned*)raw;}
static int editShape(xui_proxy proxy,const xui_text_item_t* item,xui_text_shape_t* shape)
{
    int status=layoutShape(proxy,item,shape);
    if(attemptReentry){attemptReentry=false;assert(xuiTextEditSetText(actualEdit,"reentered")==XUI_ERROR_BUSY);}
    if(closeWidget){closeWidget=false;assert(xuiWidgetDestroyChecked(actualEdit)==XUI_OK);}
    return status;
}
static void setupEdit(LayoutProvider* p,xui_font* font,const char* text)
{
    layoutSetup(p,font);
    /* Immutable table must be configured before native capability creation. */
    assert(xuiDestroyChecked(p->context)==XUI_OK && xuiProxyReleaseChecked(p->proxy)==XUI_OK);
    p->state.tProxy.textShape=editShape;xui_callback_owner_t binding=ownerRef(&p->owner);
    assert(xuiProxyCreateOwned(&p->state.tProxy,&binding,&p->proxy)==XUI_OK);
    assert(xuiCreateOwned(p->proxy,&p->context)==XUI_OK);
    assert(xuiFontReleaseChecked(*font)==XUI_OK);*font=fontCreate(p->proxy,&p->fontDrops,XUI_OK);
    xui_text_edit_desc_t desc={.iSize=sizeof(desc),.pFont=*font,.sText=text};
    assert(xuiTextEditCreate(p->context,&actualEdit,&desc)==XUI_OK);
}
static void finishEdit(LayoutProvider* p,xui_font font)
{
    assert(xuiDestroyChecked(p->context)==XUI_OK && p->paintDrops==p->shapes);
    assert(refs(xuiFontOwnership(font))==1 && xuiFontReleaseChecked(font)==XUI_OK);
    assert(xuiProxyReleaseChecked(p->proxy)==XUI_OK && drop(&p->owner));actualEdit=NULL;
}
static void editingWaitAndRetry(void)
{
    LayoutProvider p={0};xui_font font;setupEdit(&p,&font,"before");
    assert(xuiTextEditGetLineCount(actualEdit)==1 && p.shapes>0);
    unsigned produced=p.shapes,dropped=p.paintDrops;p.ready=false;
    assert(xuiTextEditSetText(actualEdit,"after")==XUI_ERROR_BUSY && !strcmp(xuiTextEditGetText(actualEdit),"before"));
    assert(xuiTextEditSetWordWrap(actualEdit,true)==XUI_ERROR_BUSY && !xuiTextEditGetWordWrap(actualEdit));
    assert(p.shapes==produced && p.paintDrops==dropped && !xrtGetError());
    p.ready=true;attemptReentry=true;assert(xuiTextEditSetText(actualEdit,"after")==XUI_OK && !attemptReentry);
    assert(!strcmp(xuiTextEditGetText(actualEdit),"after"));finishEdit(&p,font);
}
static void failedProducerCarrier(void)
{
    LayoutProvider p={0};xui_font font;setupEdit(&p,&font,"before");
    assert(xuiTextEditGetLineCount(actualEdit)==1);p.busyNextShape=p.oomAfterShape=true;
    int status=xuiTextEditSetText(actualEdit,"busy candidate");
    xrtMemDebugFailClear();xrtClearError();assert(status==XUI_OK || status==XUI_ERROR_OUT_OF_MEMORY);
    unsigned drops=p.paintDrops;assert(p.shapes>drops);
    assert(xuiDestroyChecked(p.context)==XUI_STATUS_DEFERRED && p.paintDrops==drops && !xrtGetError());
    p.ready=true;finishEdit(&p,font);
}
static void incrementalWindow(void)
{
    LayoutProvider p={0};xui_font font;const int bytes=48*1024;
    char* text=xrtMalloc(bytes+1);assert(text);memset(text,'a',bytes);text[bytes]=0;
    setupEdit(&p,&font,text);assert(xuiTextEditGetLineCount(actualEdit)==1);
    assert(xuiTextEditSetSelection(actualEdit,20000,20001)==XUI_OK);
    unsigned produced=p.shapes,dropped=p.paintDrops;p.ready=false;
    assert(xuiTextEditDeleteSelection(actualEdit)==XUI_ERROR_BUSY && (int)strlen(xuiTextEditGetText(actualEdit))==bytes);
    assert(p.shapes==produced && p.paintDrops==dropped && !xrtGetError());p.ready=true;
    assert(xuiTextEditDeleteSelection(actualEdit)==XUI_OK);
    assert((int)strlen(xuiTextEditGetText(actualEdit))==bytes-1);
    /* The original affected-window algorithm must remain incremental. */
    assert(p.shapes-produced<=3 && p.paintDrops-dropped<=3);
    assert(xuiTextEditUndo(actualEdit)==XUI_OK && (int)strlen(xuiTextEditGetText(actualEdit))==bytes);
    finishEdit(&p,font);xrtFree(text);
}
static xrtownershipref targetFont;
static unsigned fontEdges;
static bool visitActualFont(xrtownershipref ref,void* raw)
{(void)raw;if(ref.Data==targetFont.Data && ref.Ops==targetFont.Ops)++fontEdges;return true;}
static void firstNullFailureAndGraph(void)
{
    LayoutProvider p={0};xui_font font;setupEdit(&p,&font,"before");
    assert(xuiTextEditGetLineCount(actualEdit)==1);
    targetFont=xuiFontOwnership(font);fontEdges=0;
    assert(xuiContextTraceOwnership(p.context,visitActualFont,NULL) && fontEdges+1==refs(targetFont));
    p.paintStatus=XUI_ERROR_BACKEND_FAILED;p.dropMakesBusy=true;
    assert(xuiTextEditSetText(actualEdit,"not published")==XUI_ERROR_BACKEND_FAILED && !xrtGetError());
    assert(!strcmp(xuiTextEditGetText(actualEdit),"before"));
    unsigned dropped=p.paintDrops;
    assert(xuiDestroyChecked(p.context)==XUI_STATUS_DEFERRED && p.paintDrops==dropped && !xrtGetError());
    p.paintStatus=XUI_OK;p.ready=true;finishEdit(&p,font);
}
static void oomBalances(void)
{
    for(int budget=0;budget<16;++budget) {
        LayoutProvider p={0};xui_font font;setupEdit(&p,&font,"before");
        assert(xuiTextEditGetLineCount(actualEdit)==1);
        assert(xrtMemDebugFailAfter(budget));(void)xuiTextEditSetText(actualEdit,"0123456789-0123456789");
        xrtMemDebugFailClear();xrtClearError();p.ready=true;finishEdit(&p,font);
    }
}
static void nativeGetterPhysicalTail(void)
{
    for(int getter=0;getter<5;++getter) {
        LayoutProvider p={0};xui_font font;setupEdit(&p,&font,"before");
        assert(xuiTextEditSetWordWrap(actualEdit,true)==XUI_OK);
        xui_lifetime_link_t watch={0};unsigned before=widgetInvalidations;
        assert(xuiWidgetLifetimeWatch(actualEdit,&watch,editGone,&widgetInvalidations)==XUI_OK);
        closeWidget=true;
        if(getter==0)(void)xuiTextEditGetLineCount(actualEdit);
        else if(getter==1)(void)xuiTextEditGetCursorRect(actualEdit);
        else if(getter==2)assert(xuiTextEditGetScrollModel(actualEdit)==NULL);
        else if(getter==3)assert(xuiTextEditGetHScrollBarWidget(actualEdit)==NULL);
        else assert(xuiTextEditGetVScrollBarWidget(actualEdit)==NULL);
        assert(!closeWidget && widgetInvalidations==before+1 && !watch.ppPrevious);
        finishEdit(&p,font);
    }
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    editingWaitAndRetry();failedProducerCarrier();incrementalWindow();firstNullFailureAndGraph();oomBalances();nativeGetterPhysicalTail();
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("TextEdit: actual persistent carriers, BUSY before text mutation, native reentry rejection, partial OOM, incremental affected-window reuse and final balance");return 0;
}
