/* Matched default-backend CPU gate. No xgeInit/Run, window, GPU or audio. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xui.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static xge_host_xrt_proc resolve(void* user,const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
static unsigned fakeFinalized;
static int fakeFinalize(xui_proxy proxy,void* payload)
{(void)proxy;(void)payload;++fakeFinalized;return XUI_OK;}
static int32 paintWrongThread(ptr raw)
{
    xui_text_shape_t* shape=raw;xui_text_shape_t before=*shape;
    assert(shape->pPaintOwnership && shape->pPaintOwnership->size==sizeof(*shape->pPaintOwnership) &&
        shape->pPaintOwnership->AbiTag==XUI_TEXT_PAINT_OWNERSHIP_V1_TAG);
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(!shape->pPaintOwnership->DropReady(shape->pPaint));assert(xrtOwnershipScopeEnd(&freeze));
    assert(xrtMemDebugFailAfter(0));
    /* Since native checked paint retirement (4d5257f6), a preflight that
     * consumes nothing is DEFERRED. It must still preserve every byte and
     * avoid even attempting an allocation on this non-creator thread. */
    assert(xuiTextShapeFreeChecked(shape)==XUI_STATUS_DEFERRED && !memcmp(shape,&before,sizeof(before)));
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();return 0;
}
static void rgbaRows(xui_proxy proxy,bool owned)
{
    const unsigned char source[24]={255,0,0,128,4,8,12,255,17,17,17,17,0,0,0,0,30,60,90,255,17,17,17,17};
    const unsigned char packed[16]={128,0,0,128,4,8,12,255,0,0,0,0,30,60,90,255};
    unsigned char output[24];memset(output,0xCC,sizeof(output));
    xui_surface surface=NULL;
    assert(proxy->surfaceCreateRGBA(proxy,&surface,2,2,source,12,XUI_SURFACE_ALPHA_STRAIGHT)==XGE_OK);
    assert(proxy->surfaceReadRGBA(proxy,surface,output,12)==XGE_OK);
    assert(!memcmp(output,packed,8) && !memcmp(output+12,packed+8,8));
    for(unsigned i=8;i<12;++i)assert(output[i]==0xCC && output[i+12]==0xCC);
    assert(proxy->surfaceReadRGBA(proxy,surface,output,7)==XGE_ERROR_INVALID_ARGUMENT);
    if(owned)assert(xuiSurfaceReleaseChecked(surface)==XUI_OK);else proxy->surfaceDestroy(proxy,surface);
    /* A NULL input requests blank storage; its unused stride is irrelevant. */
    surface=NULL;assert(proxy->surfaceCreateRGBA(proxy,&surface,1,1,NULL,1,0)==XGE_OK);
    if(owned)assert(xuiSurfaceReleaseChecked(surface)==XUI_OK);else proxy->surfaceDestroy(proxy,surface);
    for(unsigned mode=0;mode<3;++mode){
        int width=mode==0 ? INT_MAX:1,height=mode==0 ? 1:INT_MAX;
        surface=(xui_surface)(uintptr_t)1;assert(xrtMemDebugFailAfter(0));
        assert(proxy->surfaceCreateRGBA(proxy,&surface,width,height,mode==2 ? NULL:(void*)(uintptr_t)1,0,XUI_SURFACE_ALPHA_STRAIGHT)==XGE_ERROR_OUT_OF_MEMORY);
        assert(!surface && !xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    }
}
static void rgbaTextureTarget(void)
{
    const unsigned char source[16]={128,0,0,128,4,8,12,255,0,0,0,0,30,60,90,255};
    unsigned char output[24];xge_texture_t texture={0};
    assert(xgeTextureCreateRGBAEx(&texture,2,2,source,XGE_TEXTURE_COMPRESS_NONE)==XGE_OK);
    memset(output,0xCC,sizeof(output));assert(xgeTextureReadPixels(&texture,output,12)==XGE_OK);
    assert(!memcmp(output,source,8) && !memcmp(output+12,source+8,8));
    for(unsigned i=8;i<12;++i)assert(output[i]==0xCC && output[i+12]==0xCC);
    xgeTextureFree(&texture);
    xge_render_target_t target={0};assert(xgeRenderTargetCreate(&target,2,2)==XGE_OK);
    assert(xgeTextureUpdateRGBA(&target.tTexture,0,0,2,2,source,8)==XGE_OK);
    /* Render-target readback requires the actual GPU contents, even if an
     * earlier CPU upload happened to populate its private shadow. */
    memset(output,0xCC,sizeof(output));assert(xgeRenderTargetReadPixels(&target,output,12)==XGE_ERROR_INVALID_STATE);
    for(unsigned i=0;i<sizeof(output);++i)assert(output[i]==0xCC);
    xgeRenderTargetFree(&target);
    for(unsigned mode=0;mode<2;++mode){
        texture=(xge_texture_t){0};texture.iWidth=mode ? 1:INT_MAX;texture.iHeight=mode ? INT_MAX:1;texture.iFormat=XGE_PIXEL_RGBA8;
        texture.pBackend=(void*)(uintptr_t)1;target=(xge_render_target_t){0};target.iWidth=texture.iWidth;target.iHeight=texture.iHeight;
        assert(xrtMemDebugFailAfter(0));
        assert(xgeTextureReadPixels(&texture,(void*)(uintptr_t)1,0)==XGE_ERROR_OUT_OF_MEMORY);
        assert(xgeRenderTargetReadPixels(&target,(void*)(uintptr_t)1,0)==XGE_ERROR_OUT_OF_MEMORY);
        assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    }
}
static void finalizedShape(const char* path)
{
    xui_proxy proxy=NULL;assert(xuiProxyXgeCreate(&proxy)==XUI_OK);
    xui_font font=NULL;assert(proxy->fontLoadFile(proxy,&font,path,16.0f,0)==XUI_OK);
    xui_text_item_t item={0};item.iSize=sizeof(item);item.pFont=font;item.sText="Retired payload";
    item.iTextSize=-1;item.iFlags=XUI_TEXT_SHAPE_RETAIN_PAINT;
    xui_text_shape_t shape={0};assert(proxy->textShape(proxy,&item,&shape)==XUI_OK);
    xrtownershipref ref=xuiFontOwnership(font);
    const xrtownershipadapterv1* adapter=xuiProxyResourceOwnershipAdapterV1(ref);
    const xrtownershipfinishv2* completion=xuiProxyResourceOwnershipCompletionV2(ref);
    assert(adapter && completion && adapter->Hold(ref.Data));
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(adapter->Claim(ref.Data,&shape));assert(xrtOwnershipScopeEnd(&freeze));
    assert(adapter->Finalize(ref.Data,&shape));
    xui_vec2_t measured={0};
    /* Other graph finalizers may still observe this retained shape after the
     * Font's semantic payload has gone. Reject before touching glyph fonts. */
    assert(proxy->textShapeRangeMeasure(proxy,&shape,0,shape.iTextSize,&measured)==XUI_ERROR_INVALID_STATE);
    freeze=(xrtownershipscope){0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    adapter->Restore(ref.Data,&shape);assert(xrtOwnershipScopeEnd(&freeze));
    assert(xuiTextShapeFreeChecked(&shape)==XUI_OK);
    assert(xuiFontReleaseChecked(font)==XUI_OK && completion->DropChecked(ref.Data));
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK);
}
int main(int argc,char** argv)
{
    if(argc!=2)return 2;
    assert(xgeCheckBuildCompatibility(XGE_BUILD_ABI_FINGERPRINT,XGE_FEATURES)==XGE_OK);
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;const char* missing=NULL;
    assert(xgeHostXrtBind(&contract,resolve,NULL,&missing)==XGE_HOST_XRT_OK);
    assert(xrtMemDebugEnable(true));
    xui_proxy proxy=NULL;assert(xuiProxyXgeCreate(&proxy)==XUI_OK && proxy && xuiProxyIsOwned(proxy));
    rgbaRows(proxy,true);rgbaTextureTarget();
    /* The common payload constructor intentionally supports other backends.
     * Default methods must refuse them before reading a larger XGE payload. */
    xui_font fakeFont=NULL;xui_surface fakeSurface=NULL;void* payload=NULL;
    assert(xuiFontCreateOwned(proxy,1,fakeFinalize,&fakeFont,&payload)==XUI_OK);
    xui_font_metrics_t metrics={0};assert(proxy->fontGetMetrics(proxy,fakeFont,&metrics)!=XUI_OK);
    assert(xuiFontReleaseChecked(fakeFont)==XUI_OK);
    assert(xuiSurfaceCreateOwned(proxy,1,fakeFinalize,&fakeSurface,&payload)==XUI_OK);
    xui_surface_desc_t desc={0};assert(proxy->surfaceGetDesc(proxy,fakeSurface,&desc)!=XUI_OK);
    assert(xuiSurfaceReleaseChecked(fakeSurface)==XUI_OK && fakeFinalized==2);
    xui_font font=NULL;assert(proxy->fontLoadFile(proxy,&font,argv[1],16.0f,0)==XUI_OK && font);
    assert(xuiFontGetProxy(font)==proxy && proxy->fontGetMetrics(proxy,font,&metrics)==XUI_OK && metrics.fLineHeight>0);
    xui_text_item_t item={0};item.iSize=sizeof(item);item.pFont=font;item.sText="Owned carrier";item.iTextSize=-1;item.iFlags=XUI_TEXT_SHAPE_RETAIN_PAINT;
    xui_text_shape_t shape={0};assert(proxy->textShape(proxy,&item,&shape)==XUI_OK && shape.pPaint && shape.paintFreeChecked && shape.pPaintOwnership);
    xthread* paintThread=xrtThreadCreate(paintWrongThread,&shape,0);assert(paintThread);
    assert(xrtThreadWaitFor(paintThread,10000)==XWAIT_OK && xrtThreadExitCode(paintThread)==0);xrtThreadDestroy(paintThread);
    size_t count=0;xrtownershipref ref=xuiFontOwnership(font);assert(ref.Ops && ref.Ops->Count(ref.Data,&count) && count==2);
    bool consumed=false;assert(xuiProxyReleaseOwnerChecked(proxy,&consumed)==XUI_OK && consumed);
    assert(xuiFontReleaseOwnerChecked(font,&consumed)==XUI_OK && consumed);
    /* The paint record still owns exactly one actual carrier reference. */
    assert(ref.Ops->Count(ref.Data,&count) && count==1);
    assert(proxy->fontGetMetrics(proxy,font,&metrics)==XUI_OK && metrics.fLineHeight>0);
    assert(xuiTextShapeFreeChecked(&shape)==XUI_OK && !shape.pPaint && !shape.pClusters && !shape.pCarets);
    xui_proxy_t raw=xuiProxyXge();assert(!raw._pCapability && !xuiProxyIsOwned(&raw));
    rgbaRows(&raw,false);
    font=NULL;assert(raw.fontLoadFile(&raw,&font,argv[1],16.0f,0)==XUI_OK && font);
    assert(!xuiFontOwnership(font).Data && raw.fontGetMetrics(&raw,font,&metrics)==XUI_OK);
    raw.fontDestroy(&raw,font);
    finalizedShape(argv[1]);
    assert(!xrtGetError());
    puts("matched native owned XGE proxy: CPU font/shared carrier/retained paint/tiny foreign payload rejection/raw C contract: PASS");return 0;
}
