/* Actual CodeEdit TU and its privately-owned CodeDocument; CPU-only. */
#define XUI_OWNED_LAYOUT_HELPERS
#include "xui_owned_layout_test.c"
#include "../../src/xui/core/xui_text_internal.h"
#include "../../src/xui/code/xui_code_edit.c"
static xui_widget editor;
static bool destroyOnDrop;
static bool destroyOnShape,tryCodeReentry;
static int codePaintDrop(void* raw)
{
    if(destroyOnDrop){destroyOnDrop=false;assert(xuiWidgetDestroyChecked(editor)==XUI_OK);}
    return layoutPaintDrop(raw);
}
static const xui_text_paint_ownership_v1_t codePaintOps={sizeof(codePaintOps),XUI_TEXT_PAINT_OWNERSHIP_V1_TAG,layoutPaintReady,layoutPaintTrace,codePaintDrop};
static int codeShape(xui_proxy proxy,const xui_text_item_t* item,xui_text_shape_t* shape)
{
    int status=layoutShape(proxy,item,shape);if(shape->pPaint)shape->pPaintOwnership=&codePaintOps;
    if(tryCodeReentry) {
        tryCodeReentry=false;xui_rect_t unchanged={7,8,9,10},rect=unchanged;
        assert(xuiCodeEditGetOffsetRect(editor,1,&rect)==XUI_ERROR_BUSY && !memcmp(&rect,&unchanged,sizeof(rect)));
        xui_code_document document=xuiCodeEditGetDocument(editor);int bytes=xuiCodeDocumentGetLength(document);
        assert(xuiCodeDocumentReplace(document,0,1,"x")==XUI_ERROR_BUSY && xuiCodeDocumentGetLength(document)==bytes);
    }
    if(destroyOnShape){destroyOnShape=false;assert(xuiWidgetDestroyChecked(editor)==XUI_OK);}
    return status;
}
static void setupCode(LayoutProvider* p,xui_font* font,const char* text)
{
    layoutSetup(p,font);assert(xuiDestroyChecked(p->context)==XUI_OK && xuiProxyReleaseChecked(p->proxy)==XUI_OK);
    p->state.tProxy.textShape=codeShape;xui_callback_owner_t binding=ownerRef(&p->owner);
    assert(xuiProxyCreateOwned(&p->state.tProxy,&binding,&p->proxy)==XUI_OK && xuiCreateOwned(p->proxy,&p->context)==XUI_OK);
    assert(xuiFontReleaseChecked(*font)==XUI_OK);*font=fontCreate(p->proxy,&p->fontDrops,XUI_OK);
    xui_code_edit_desc_t desc={.iSize=sizeof(desc),.pFont=*font,.sText=text,.iTabColumns=4,.iIndentColumns=4};
    assert(xuiCodeEditCreate(p->context,&editor,&desc)==XUI_OK);
}
static void finishCode(LayoutProvider* p,xui_font font)
{
    assert(xuiDestroyChecked(p->context)==XUI_OK && p->paintDrops==p->shapes);
    assert(refs(xuiFontOwnership(font))==1 && xuiFontReleaseChecked(font)==XUI_OK);
    assert(xuiProxyReleaseChecked(p->proxy)==XUI_OK && drop(&p->owner));editor=NULL;
}
static xrtownershipref targetFont;static unsigned fontEdges;
static bool codeVisit(xrtownershipref ref,void* user)
{(void)user;if(ref.Data==targetFont.Data && ref.Ops==targetFont.Ops)++fontEdges;return true;}
static void actualImeCarrier(bool oom)
{
    LayoutProvider p={0};xui_font font;setupCode(&p,&font,"source");xui_code_edit_data_t* data=__xuiCodeEditGetData(editor);
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(p.context,&scope)==XUI_OK);data->iCacheActivity=2;
    unsigned before=p.shapes;p.busyNextShape=true;p.oomAfterShape=oom;
    float x=__xuiCodeEditShapedTextOffsetX(p.proxy,font,data,"abcdef",6,3,8,24);
    xrtMemDebugFailClear();xrtClearError();assert(x==24 && data->iLineLayoutCount>0 && p.shapes==before+1);
    if(!oom) {
        assert(__xuiCodeEditShapedTextOffsetX(p.proxy,font,data,"abcdef",6,2,8,16)==16);
        assert(__xuiCodeEditShapedTextOffsetX(p.proxy,font,data,"abcdef",6,6,8,48)==48 && p.shapes==before+1);
    }
    data->iCacheActivity=0;assert(xuiCallScopeEnd(&scope)==XUI_OK);
    targetFont=xuiFontOwnership(font);fontEdges=0;
    assert(xuiContextOwnershipBusy(p.context) && !xuiContextTraceOwnership(p.context,codeVisit,NULL));
    p.ready=true;bool traced=xuiContextTraceOwnership(p.context,codeVisit,NULL);
    if(!traced || fontEdges+1!=refs(targetFont))fprintf(stderr,"code graph oom=%d traced=%d edges=%u refs=%zu entries=%d activity=%d\n",oom,traced,fontEdges,refs(targetFont),data->iLineLayoutCount,data->iCacheActivity);
    assert(traced && fontEdges+1==refs(targetFont));
    p.ready=false;
    unsigned dropped=p.paintDrops;assert(xuiDestroyChecked(p.context)==XUI_STATUS_DEFERRED && p.paintDrops==dropped && !xrtGetError());
    p.ready=true;finishCode(&p,font);
}
static void actualBorrowedEditWindow(void)
{
    const int bytes=48*1024;char* text=xrtMalloc(bytes+1);assert(text);memset(text,'a',bytes);text[bytes]=0;
    LayoutProvider p={0};xui_font font;setupCode(&p,&font,text);xui_code_document document=xuiCodeEditGetDocument(editor);
    xui_rect_t rect={0};assert(xuiCodeEditGetOffsetRect(editor,20000,&rect)==XUI_OK);
    unsigned shapes=p.shapes,drops=p.paintDrops;p.ready=false;
    assert(xuiCodeDocumentReplace(document,20000,20001,"b")==XUI_ERROR_BUSY && xuiCodeDocumentGetLength(document)==bytes && p.paintDrops==drops);
    p.ready=true;assert(xuiCodeDocumentReplace(document,20000,20001,"b")==XUI_OK);
    assert(xuiCodeEditGetOffsetRect(editor,20000,&rect)==XUI_OK && p.shapes-shapes<=3 && p.paintDrops-drops<=3);
    assert(xuiCodeDocumentDestroyChecked(document)==XUI_ERROR_INVALID_STATE && xuiCodeDocumentGetLength(document)==bytes);
    finishCode(&p,font);xrtFree(text);
}
static void docGone(void* raw){++*(unsigned*)raw;}
static void actualBorrowedEditDestroyTail(void)
{
    LayoutProvider p={0};xui_font font;setupCode(&p,&font,"before");xui_code_document document=xuiCodeEditGetDocument(editor);
    xui_rect_t rect={0};assert(xuiCodeEditGetOffsetRect(editor,3,&rect)==XUI_OK && p.shapes>0);
    xui_lifetime_link_t watch={0};unsigned invalidations=0;assert(xuiCodeDocumentLifetimeWatch(document,&watch,docGone,&invalidations)==XUI_OK);
    destroyOnDrop=true;int status=xuiCodeDocumentReplace(document,1,2,"x");
    assert(status==XUI_ERROR_INVALID_STATE && !destroyOnDrop && invalidations==1 && !watch.ppPrevious);
    finishCode(&p,font);
}
static void actualGeometryPhysicalTail(void)
{
    LayoutProvider p={0};xui_font font;setupCode(&p,&font,"before");
    xui_code_document document=xuiCodeEditGetDocument(editor);xui_lifetime_link_t watch={0};unsigned invalidations=0;
    assert(xuiCodeDocumentLifetimeWatch(document,&watch,docGone,&invalidations)==XUI_OK);
    tryCodeReentry=destroyOnShape=true;xui_rect_t rect={0};
    assert(xuiCodeEditGetOffsetRect(editor,3,&rect)==XUI_OK);
    assert(!tryCodeReentry && !destroyOnShape && invalidations==1 && !watch.ppPrevious);
    finishCode(&p,font);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    actualImeCarrier(false);actualImeCarrier(true);actualBorrowedEditWindow();actualBorrowedEditDestroyTail();actualGeometryPhysicalTail();
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("CodeEdit: actual owned IME cache/Font graph, partial OOM, private Document owner guard, incremental borrowed edit window and destruction physical tail balanced");return 0;
}
