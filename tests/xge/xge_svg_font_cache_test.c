/* Real aggregate, real project-owned embedded SFNT. No renderer/window/device
 * or global fallback font is initialized. Old controls stop before stale use. */
#define XRT_MODULE_ALL
#include "single/xge.c"
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static char document[65536];
static int documentBytes;
static xge_svg create(void)
{
    xge_svg svg=NULL;assert(xgeSvgCreate(&svg)==XGE_OK);
    assert(xgeSvgLoadMemory(svg,document,documentBytes)==XGE_OK);
    return svg;
}
static xge_font load(xge_svg svg,float size)
{
    xge_font font=NULL;assert(__xgeSvgFontCacheGet(svg,"Local",size,&font)==XGE_OK && font);
    return font;
}
static void equalHeap(const xmemdebugsnapshot* before)
{
    xmemdebugsnapshot after;xrtMemDebugSnapshot(&after);
    assert(after.LiveCount==before->LiveCount && after.LiveBytes==before->LiveBytes);
    assert(after.DoubleFreeCount==before->DoubleFreeCount && after.InvalidFreeCount==before->InvalidFreeCount);
}
static void retained(int mode)
{
    xge_svg svg=create();xge_font first=load(svg,10);
    assert(xgeFontAddRef(first)==2);uintptr_t address=(uintptr_t)first;
    xge_font_metrics_t original={0};assert(xgeFontGetMetrics(first,&original)==XGE_OK);
    xge_glyph_run_t* run=NULL;
    if(mode==1){
        xge_text_shape_desc_t shape={0};shape.iSize=sizeof(shape);shape.pFont=first;
        shape.sText="abc";shape.iTextSize=3;
        assert(xgeTextShapeOwned(&run,&shape)==XGE_OK && run && run->iGlyphCount>0);
    }
    for(int i=11;i<35;++i)(void)load(svg,(float)i);
    assert(address==(uintptr_t)load(svg,10) && "retained Font record moved with the SVG cache array");
    assert(xgeFontIsOwned(first) && !xgeGraphicsOwnedResourceCount());
    xmemdebugsnapshot hot,after;xrtMemDebugSnapshot(&hot);
    assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i)assert(load(svg,10)==first);
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtMemDebugSnapshot(&after);
    assert(hot.AllocCount==after.AllocCount && hot.FreeCount==after.FreeCount && hot.ReallocCount==after.ReallocCount);
    assert(xgeSvgClear(svg)==XGE_OK);xgeSvgDestroy(svg);
    xge_font_metrics_t surviving={0};assert(xgeFontGetMetrics(first,&surviving)==XGE_OK);
    assert(!memcmp(&original,&surviving,sizeof(original)) && first->iRefCount==(mode==1 ? 2:1));
    xgeFontFree(first);
    if(mode==1){
        /* The real GlyphRun, not a fake hold, retains the stable Font address
         * after both the SVG and explicit Font reference have been released. */
        xge_font fromRun=NULL;assert(xgeGlyphRunGetFont(run,0,&fromRun)==XGE_OK && fromRun);
        assert((uintptr_t)fromRun==address && xgeFontGetMetrics(fromRun,&surviving)==XGE_OK);
        xgeFontFree(fromRun);xgeGlyphRunFree(run);
    }
}
static void allocationPrefixes(void)
{
    unsigned failed=0,passed=0;
    for(int budget=0;budget<96;++budget){
        xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
        xge_svg svg=create();xge_font font=NULL;
        assert(xrtMemDebugFailAfter(budget));
        int result=__xgeSvgFontCacheGet(svg,"Local",10,&font);
        xrtMemDebugFailClear();xrtClearError();
        if(result==XGE_OK){assert(font && xgeFontIsOwned(font));++passed;}
        else{
            assert(result==XGE_ERROR_OUT_OF_MEMORY && !font);++failed;
            /* OOM must not permanently discard the embedded font bytes or
             * turn a retry into an unavailable global fallback font. */
            assert(load(svg,10));
        }
        xgeSvgDestroy(svg);equalHeap(&before);
    }
    assert(failed && passed);
    printf("SVG font allocation prefixes: %u failed, %u passed\n",failed,passed);
}
static int32_t worker(void* unused)
{
    (void)unused;
    for(int round=0;round<12;++round){
        xge_svg svg=create();
        for(int i=0;i<8;++i){
            xge_font font=load(svg,(float)i+10);xge_font_metrics_t metrics={0};
            assert(xgeFontGetMetrics(font,&metrics)==XGE_OK && metrics.fPixelSize==(float)i+10);
        }
        xgeSvgDestroy(svg);
    }
    return 0;
}
static void threads(void)
{
    xthread* workers[4];
    for(unsigned i=0;i<4;++i){workers[i]=xrtThreadCreate(worker,NULL,0);assert(workers[i]);}
    for(unsigned i=0;i<4;++i){assert(xrtThreadWaitFor(workers[i],30000)==XWAIT_OK && !xrtThreadExitCode(workers[i]));xrtThreadDestroy(workers[i]);}
}
static void engineIndependent(void)
{
#if !defined(XGE_TEST_SVG_FONT_BASELINE)
    xge_svg svg=create();xge_font font=load(svg,10);
    assert(!xgeGraphicsOwnedResourceCount() && xrtAtomic64Load(&g_xgeFontCount,XMEMORY_RELAXED)==1);
    for(int cycle=0;cycle<2;++cycle){
        assert(xgeInit(NULL)==XGE_OK);
        assert(xrtAtomic64Load(&g_xgeFontCount,XMEMORY_RELAXED)==1);
#if XGE_HAS_DEBUGMODE
        xge_debug_stats_t stats={0};assert(xgeDebugGetStats(&stats)==XGE_OK && stats.iFontCount==1);
#endif
        assert(xgeUnitChecked()==XGE_OK);
        xge_font_metrics_t metrics={0};assert(xgeFontGetMetrics(font,&metrics)==XGE_OK && metrics.fPixelSize==10);
    }
    xgeSvgDestroy(svg);
#endif
}
static void textFailures(void)
{
    char textDocument[65536];const char* close=strstr(document,"</svg>");assert(close);
    size_t prefix=(size_t)(close-document);
    int length=snprintf(textDocument,sizeof(textDocument),"%.*s<text font-family='Local' font-size='12' x='1' y='18'>abc</text></svg>",
        (int)prefix,document);assert(length>0 && (size_t)length<sizeof(textDocument));
    unsigned loadFailures=0,loadSuccesses=0,boundsFailures=0,boundsSuccesses=0;
    for(int budget=0;budget<128;++budget){
        xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
        xge_svg svg=NULL;assert(xgeSvgCreate(&svg)==XGE_OK);
        assert(xrtMemDebugFailAfter(budget));int status=xgeSvgLoadMemory(svg,textDocument,length);
        xrtMemDebugFailClear();xrtClearError();
        if(status!=XGE_OK){
            assert(status==XGE_ERROR_OUT_OF_MEMORY);++loadFailures;
            assert(xgeSvgLoadMemory(svg,textDocument,length)==XGE_OK);
        }else ++loadSuccesses;
        xge_rect_t bounds={0};assert(xgeSvgGetBounds(svg,0.25f,&bounds)==XGE_OK);
        assert(isfinite(bounds.fW) && isfinite(bounds.fH) && bounds.fW>0 && bounds.fH>0);
        xgeSvgDestroy(svg);equalHeap(&before);
    }
    for(int budget=0;budget<32;++budget){
        xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
        xge_svg svg=NULL;assert(xgeSvgCreate(&svg)==XGE_OK && xgeSvgLoadMemory(svg,textDocument,length)==XGE_OK);
        xge_rect_t bounds={0};assert(xrtMemDebugFailAfter(budget));
        int status=xgeSvgGetBounds(svg,0.25f,&bounds);xrtMemDebugFailClear();xrtClearError();
        if(status!=XGE_OK){
            assert(status==XGE_ERROR_OUT_OF_MEMORY);++boundsFailures;
            assert(xgeSvgGetBounds(svg,0.25f,&bounds)==XGE_OK);
        }else ++boundsSuccesses;
        xgeSvgDestroy(svg);equalHeap(&before);
    }
    assert(loadFailures && loadSuccesses && boundsFailures && boundsSuccesses);
    printf("SVG text prefixes: load %u failed/%u passed; bounds %u failed/%u passed\n",
        loadFailures,loadSuccesses,boundsFailures,boundsSuccesses);
}
int main(int argc,char** argv)
{
    assert(argc==3);int mode=atoi(argv[1]);assert(mode>=0 && mode<=5);
    FILE* input=fopen(argv[2],"rb");assert(input);
    size_t bytes=fread(document,1,sizeof(document)-1,input);assert(bytes && bytes<sizeof(document)-1 && !ferror(input));
    assert(!fclose(input));document[bytes]=0;documentBytes=(int)bytes;
    assert(xrtMemDebugEnable(true));
    if(mode==3)threads(); /* Warm platform thread-local bookkeeping once. */
    xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
    if(mode<2)retained(mode);
    if(mode==2)allocationPrefixes();
    if(mode==3)threads();
    if(mode==4)engineIndependent();
    if(mode==5)textFailures();
#if !defined(XGE_TEST_SVG_FONT_BASELINE)
    assert(!xrtAtomic64Load(&g_xgeFontCount,XMEMORY_RELAXED) && !xgeGraphicsOwnedResourceCount());
#endif
    xrtClearError();equalHeap(&before);
    printf("SVG stable font mode %d passed; cache entry %zu, Font %zu, raster %zu bytes\n",mode,
        sizeof(xge_svg_font_cache_t),sizeof(xge_font_t),sizeof(xge_svg_raster_image_t));
    return 0;
}
