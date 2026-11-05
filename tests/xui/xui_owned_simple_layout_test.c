/* Real Label/Hyperlink owned layout slots, CPU-only; never initialize XGE. */
#define XUI_OWNED_LAYOUT_HELPERS
#include "xui_owned_layout_test.c"
static unsigned invalidations;
static void gone(void* raw){++*(unsigned*)raw;}
static xrtownershipref wanted;
static unsigned actualEdges;
static bool visitFont(xrtownershipref ref,void* user)
{(void)user;if(ref.Data==wanted.Data && ref.Ops==wanted.Ops)++actualEdges;return true;}
static void actualSimpleWidgets(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);
    xui_widget label=NULL,link=NULL;
    xui_label_desc_t ld={.iSize=sizeof(ld),.sText="label",.pFont=font};
    xui_hyperlink_desc_t hd={.iSize=sizeof(hd),.sText="link",.pFont=font};
    assert(xuiLabelCreate(p.context,&label,&ld)==XUI_OK && xuiHyperlinkCreate(p.context,&link,&hd)==XUI_OK);
    xui_vec2_t size={0};assert(xuiWidgetMeasureContent(label,(xui_vec2_t){300,50},&size)==XUI_OK && size.fX>0);
    assert(xuiWidgetMeasureContent(link,(xui_vec2_t){300,50},&size)==XUI_OK && size.fX>0);
    assert(p.shapes==2 && refs(xuiFontOwnership(font))==7 && !p.context->pMeasureLayout);
    wanted=xuiFontOwnership(font);actualEdges=0;
    assert(xuiContextTraceOwnership(p.context,visitFont,NULL) && actualEdges==6);
    xui_lifetime_link_t left={0},right={0};assert(xuiWidgetLifetimeWatch(label,&left,gone,&invalidations)==XUI_OK);
    assert(xuiWidgetLifetimeWatch(link,&right,gone,&invalidations)==XUI_OK);
    p.ready=false;bool consumed=true;
    assert(xuiDestroyOwnerChecked(p.context,&consumed)==XUI_STATUS_DEFERRED && !consumed && !p.paintDrops && !invalidations && !xrtGetError());
    p.ready=true;assert(xuiDestroyChecked(p.context)==XUI_OK && invalidations==2 && p.paintDrops==2);
    assert(xuiFontReleaseChecked(font)==XUI_OK && p.fontDrops==1 && xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner));
}
static void actualPartialSlot(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);
    xui_widget label=NULL;xui_label_desc_t desc={.iSize=sizeof(desc),.sText="partial",.pFont=font};
    assert(xuiLabelCreate(p.context,&label,&desc)==XUI_OK);
    p.busyNextShape=p.oomAfterShape=true;xui_vec2_t size={0};
    assert(xuiWidgetMeasureContent(label,(xui_vec2_t){300,50},&size)==XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();xrtClearError();
    assert(p.shapes==1 && !p.paintDrops);
    assert(xuiDestroyChecked(p.context)==XUI_STATUS_DEFERRED && !p.paintDrops);
    p.ready=true;assert(xuiDestroyChecked(p.context)==XUI_OK && p.paintDrops==1);
    assert(xuiFontReleaseChecked(font)==XUI_OK && p.fontDrops==1 && xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner));
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);actualSimpleWidgets();actualPartialSlot();
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("actual Label/Hyperlink: owned paint+font slots, exact graph refs, cached Font keys, failed producer carrier and deferred Context close balanced");return 0;
}
