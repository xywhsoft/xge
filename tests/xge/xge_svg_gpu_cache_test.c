/* One bounded real renderer session, with no external resources or network.
 * Public-product and aggregate forms share exactly the same SVG/readback.
 * The aggregate additionally checks stable CPU Font -> owned GPU atlas. */
#include <string.h>
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#if defined(XGE_SVG_TEST_AGGREGATE)
#include "single/xge.c"
#else
#include <xge.h>
#if !defined(XGE_TEST_STATIC)
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
static xge_host_xrt_proc resolve(void* user,const char* name)
{
    (void)user;
#define XGE_HOST_XRT_SLOT(symbol) if(!strcmp(name,#symbol))return (xge_host_xrt_proc)&symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
#endif
#endif
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct State {xge_svg svg,clone;xge_glyph_run_t* run;int calls;const char* document;} State;
#if defined(XGE_SVG_TEST_AGGREGATE)
static int cachedRaster(xge_svg_paint paint,void* raw)
{
    if(paint->iType==XGE_SVG_DRAW_ITEM_RASTER_IMAGE)
        *(xge_texture*)raw=paint->u.tRaster.pRaster->pTexture;
    return 1;
}
#endif
static void queuedRaster(State* state)
{
    const char* image=strstr(state->document,"<image ");assert(image);
    const char* end=strstr(image,"/>");assert(end);char document[1024];
    int bytes=snprintf(document,sizeof(document),"<svg width='16' height='16' viewBox='0 0 16 16'>%.*s</svg>",
        (int)(end+2-image),image);assert(bytes>0 && (size_t)bytes<sizeof(document));
    uint64_t before=xgeGraphicsOwnedResourceCount();xge_svg queued=NULL;
    assert(xgeSvgCreate(&queued)==XGE_OK && xgeSvgLoadMemory(queued,document,bytes)==XGE_OK);
    xge_rect_t destination={0,0,16,16};assert(xgeSvgDrawPx(queued,destination,0.25f)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==before+1);
#if defined(XGE_SVG_TEST_AGGREGATE)
    /* SVG may flush its own draw while restoring clip/blend state. Force an
     * ordinary public deferred draw of this exact private cached texture. */
    xge_texture texture=NULL;xge_svg_paint picture=NULL;
    assert(xgeSvgPaintGetPicture(queued,&picture)==XGE_OK);
    assert(xgeSvgPaintTraverse(picture,cachedRaster,&texture)==XGE_OK);
    assert(texture);xgeDrawPx(texture,0,0);assert(texture->iRefCount>1);
#endif
    /* Last document and raster-image references disappear before the queued
     * draw reads its texture. The queued native owner must keep storage alive. */
    xgeSvgDestroy(queued);
#if defined(XGE_SVG_TEST_AGGREGATE)
    assert(xgeGraphicsOwnedResourceCount()==before+1);
#else
    assert(xgeGraphicsOwnedResourceCount()==before || xgeGraphicsOwnedResourceCount()==before+1);
#endif
    assert(xgeFlush()==XGE_OK && xgeGraphicsOwnedResourceCount()==before);
}
static void exercise(State* state,uint64_t base)
{
    xge_render_target target=NULL;xge_pass pass=NULL;
    assert(xgeRenderTargetCreateOwned(&target,16,16)==XGE_OK);
    assert(xgePassCreateOwned(&pass,target,XGE_PASS_CLEAR_COLOR,XGE_COLOR_RGBA(0,0,0,255))==XGE_OK);
    assert(xgePassBegin(pass)==XGE_OK);
    uint64_t active=xgeGraphicsOwnedResourceCount();assert(active>base);
#if defined(XGE_SVG_TEST_AGGREGATE)
    xge_font font=NULL;assert(__xgeSvgFontCacheGet(state->svg,"Local",12,&font)==XGE_OK && font);
    assert(xgeFontIsOwned(font));
    xge_text_shape_desc_t shape={0};shape.iSize=sizeof(shape);shape.pFont=font;
    shape.sText="abc";shape.iTextSize=3;
    assert(xgeTextShapeOwned(&state->run,&shape)==XGE_OK && state->run);
    xge_glyph_t glyph={0};assert(xgeFontGlyphAtlasGet(font,'a',&glyph)==XGE_OK && glyph.iPage>=0);
    assert(xgeGraphicsOwnedResourceCount()==active);
    /* Fail the first real atlas GPU owner's reservation/allocation, then
     * retry. CPU page pixels and the owning Font must remain usable. */
    assert(xrtMemDebugFailAfter(0));
    assert(__xgeFontAtlasUploadPage(font,glyph.iPage)==XGE_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();xrtClearError();assert(xgeGraphicsOwnedResourceCount()==active);
    assert(__xgeFontAtlasUploadPage(font,glyph.iPage)==XGE_OK);
    assert(xgeGraphicsOwnedResourceCount()==active+1);
#endif
    xgeClear(XGE_COLOR_RGBA(0,0,0,255));xge_rect_t destination={0,0,16,16};
    assert(xgeSvgDrawPx(state->svg,destination,0.25f)==XGE_OK && xgeFlush()==XGE_OK);
    queuedRaster(state);
    unsigned char pixels[16*16*4];
    assert(xgePassEnd(pass)==XGE_OK);
    assert(xgeRenderTargetReadPixelsSized(target,pixels,sizeof(pixels),16*4)==XGE_OK);
    xgePassFree(pass);xgeRenderTargetFree(target);
    unsigned red=0,white=0;
    for(unsigned i=0;i<16*16;++i){
        const unsigned char* p=pixels+4*i;
        if(p[0]>200 && p[1]<20 && p[2]<20 && p[3]>200)++red;
        if(p[0]>200 && p[1]>200 && p[2]>200 && p[3]>200)++white;
    }
    assert(red>=16 && white>0);
    assert(xgeSvgClone(state->svg,&state->clone)==XGE_OK && state->clone);
    uint64_t expected=base+1;
#if defined(XGE_SVG_TEST_AGGREGATE)
    ++expected;
#endif
    assert(xgeGraphicsOwnedResourceCount()==expected);
    printf("SVG real pixels: %u red, %u white; native GPU owners %llu\n",red,white,(unsigned long long)(expected-base));
}
static void drop(State* state,uint64_t base)
{
    xgeSvgDestroy(state->svg);state->svg=NULL;
    /* Clone shares the native raster image; releasing one document cannot
     * delete its texture while another document still holds it. */
    assert(xgeGraphicsOwnedResourceCount()>=base+1);
    xgeSvgDestroy(state->clone);state->clone=NULL;
#if defined(XGE_SVG_TEST_AGGREGATE)
    assert(xgeGraphicsOwnedResourceCount()==base+1);
    xge_font retained=NULL;assert(xgeGlyphRunGetFont(state->run,0,&retained)==XGE_OK && retained);
    xge_font_metrics_t metrics={0};assert(xgeFontGetMetrics(retained,&metrics)==XGE_OK);
    xgeFontFree(retained);xgeGlyphRunFree(state->run);state->run=NULL;
#endif
    assert(xgeGraphicsOwnedResourceCount()==base);
}
#if !XGE_ENABLE_EGL
static int frame(void* raw)
{State* state=raw;++state->calls;exercise(state,0);return 1;}
#endif
int main(int argc,char** argv)
{
    assert(argc==2);
#if !defined(XGE_TEST_STATIC) && !defined(XGE_SVG_TEST_AGGREGATE)
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_OK);
#endif
    assert(xrtMemDebugEnable(true));
    FILE* input=fopen(argv[1],"rb");assert(input);char document[65536];
    size_t bytes=fread(document,1,sizeof(document),input);assert(bytes && bytes<sizeof(document) && !ferror(input));assert(!fclose(input));
    State state={0};state.document=document;document[bytes]=0;assert(xgeSvgCreate(&state.svg)==XGE_OK);
    assert(xgeSvgLoadMemory(state.svg,document,(int)bytes)==XGE_OK);
    assert(!xgeGraphicsOwnedResourceCount());
    xge_desc_t desc={0};desc.iWidth=desc.iHeight=16;desc.sTitle="XGE SVG bounded cache test";
#if XGE_ENABLE_EGL
    desc.iRunMode=XGE_RUN_MANUAL;assert(xgeInit(&desc)==XGE_OK);
    xge_graphics_context context=NULL;assert(xgeGraphicsContextCreateEglOwned(&context,16,16,true,NULL)==XGE_OK && context);
    exercise(&state,1);
    bool consumed=true;assert(xgeGraphicsContextReleaseOwnerChecked(context,&consumed)==XGE_STATUS_DEFERRED && !consumed);
    drop(&state,1);
    assert(xgeGraphicsContextReleaseOwnerChecked(context,&consumed)==XGE_OK && consumed);
#else
    assert(xgeInit(&desc)==XGE_OK && xgeRun(frame,&state)==XGE_OK && state.calls==1);
    /* Outside the frame: Busy is caused by real cached native GPU owners,
     * not by the callback/frame scope. Ended backend cleanup must not use GL. */
    assert(xgeUnitChecked()==XGE_ERROR_BUSY);
    drop(&state,0);
#endif
    assert(!xgeGraphicsOwnedResourceCount() && xgeUnitChecked()==XGE_OK && !xrtGetError());
    puts("SVG cached raster/Font GPU ownership, shared clone lifetime and checked close: PASS");return 0;
}
