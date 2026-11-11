/* Real font/cache paths. Documents and Font records are thread-local; only
 * the immutable SFNT source, Face outline cache and fallback publication share.
 * The TSan gate instruments the canonical XRT implementation as well, so its
 * allocator, locks and atomics remain visible to the race detector. */
#define XRT_MODULE_ALL
#include "single/xge.c"
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
static unsigned char fontData[65536];
static int fontBytes;
static xatomic32 ready=XRT_ATOMIC32_INIT(0),go=XRT_ATOMIC32_INIT(0);
static void waitStart(void)
{
    (void)xrtAtomic32FetchAdd(&ready,1,XMEMORY_RELEASE);
    while(!xrtAtomic32Load(&go,XMEMORY_ACQUIRE))xrtThreadYield();
}
static int32_t reader(void* argument)
{
    bool replacing=argument!=NULL;
    xge_svg svg=NULL;assert(xgeSvgCreate(&svg)==XGE_OK);waitStart();
    for(unsigned i=0;i<128;++i){
        xge_font font=NULL;int status=__xgeSvgFontCacheGet(svg,"",10.0f+(float)(i%32),&font);
        if(replacing && status==XGE_ERROR_RESOURCE_FAILED){assert(!font);continue;}
        assert(status==XGE_OK && font);
        const stbtt_vertex* points=NULL;int count=0;
        assert(__xgeFontGlyphOutlineGet(font,2,&points,&count)==XGE_OK && points && count>0);
        assert(xgeSvgClear(svg)==XGE_OK);
    }
    xgeSvgDestroy(svg);return 0;
}
static int32_t writer(void* argument)
{
    (void)argument;waitStart();
    for(unsigned i=0;i<128;++i){
        assert(xgeFontFallbackSetMemory(fontData,fontBytes,12)==XGE_OK);
        if(i%2)xgeFontFallbackClear();
    }
    return 0;
}
static void parallel(bool replacing)
{
    assert(xgeFontFallbackSetMemory(fontData,fontBytes,12)==XGE_OK);
    xrtAtomic32Store(&ready,0,XMEMORY_RELAXED);xrtAtomic32Store(&go,0,XMEMORY_RELAXED);
    xthread* jobs[3];unsigned count=replacing ? 3:2;
    for(unsigned i=0;i<2;++i){jobs[i]=xrtThreadCreate(reader,replacing ? &jobs:NULL,0);assert(jobs[i]);}
    if(replacing){jobs[2]=xrtThreadCreate(writer,NULL,0);assert(jobs[2]);}
    while(xrtAtomic32Load(&ready,XMEMORY_ACQUIRE)!=count)xrtThreadYield();
    xrtAtomic32Store(&go,1,XMEMORY_RELEASE);
    for(unsigned i=0;i<count;++i){assert(xrtThreadWaitFor(jobs[i],30000)==XWAIT_OK && !xrtThreadExitCode(jobs[i]));xrtThreadDestroy(jobs[i]);}
    xgeFontFallbackClear();
    assert(!xrtAtomic64Load(&g_xgeFontCount,XMEMORY_RELAXED));
}
static void sameSource(void)
{
    assert(xgeFontFallbackSetMemory(fontData,fontBytes,12)==XGE_OK);
    xge_svg first=NULL,second=NULL;assert(xgeSvgCreate(&first)==XGE_OK && xgeSvgCreate(&second)==XGE_OK);
    xge_font a=NULL,b=NULL;assert(__xgeSvgFontCacheGet(first,"",10,&a)==XGE_OK && __xgeSvgFontCacheGet(second,"",11,&b)==XGE_OK);
    assert(a!=b && __xgeFontFace(a)==__xgeFontFace(b)); /* No SFNT byte copying per document. */
    const stbtt_vertex *ap=NULL,*bp=NULL;int ac=0,bc=0;
    assert(__xgeFontGlyphOutlineGet(a,2,&ap,&ac)==XGE_OK && __xgeFontGlyphOutlineGet(b,2,&bp,&bc)==XGE_OK);
    assert(ap && ac>0 && ap==bp && ac==bc);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    for(unsigned i=0;i<1000;++i){assert(__xgeFontGlyphOutlineGet(a,2,&bp,&bc)==XGE_OK && ap==bp && ac==bc);}
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
    xgeFontFallbackClear();xgeSvgDestroy(first);
    assert(__xgeFontGlyphOutlineGet(b,2,&bp,&bc)==XGE_OK && ap==bp && ac==bc);xgeSvgDestroy(second);
}
static void equalHeap(const xmemdebugsnapshot* before)
{
    xmemdebugsnapshot after;xrtMemDebugSnapshot(&after);
    assert(before->LiveCount==after.LiveCount && before->LiveBytes==after.LiveBytes &&
        before->InvalidFreeCount==after.InvalidFreeCount && before->DoubleFreeCount==after.DoubleFreeCount);
}
static void allocationPrefixes(void)
{
    unsigned failures=0,successes=0;
    for(int budget=0;budget<24;++budget){
        xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
        assert(xgeFontFallbackSetMemory(fontData,fontBytes,12)==XGE_OK);
        xge_svg svg=NULL;assert(xgeSvgCreate(&svg)==XGE_OK);xge_font font=NULL;
        assert(__xgeSvgFontCacheGet(svg,"",10,&font)==XGE_OK && font);
        const stbtt_vertex* points=NULL;int count=0;
        assert(xrtMemDebugFailAfter(budget));int status=__xgeFontGlyphOutlineGet(font,2,&points,&count);
        xrtMemDebugFailClear();xrtClearError();
        if(status==XGE_ERROR_OUT_OF_MEMORY){++failures;assert(!points && !count);assert(__xgeFontGlyphOutlineGet(font,2,&points,&count)==XGE_OK);}
        else {assert(status==XGE_OK);++successes;}
        assert(points && count>0);
        xgeSvgDestroy(svg);xgeFontFallbackClear();equalHeap(&before);
    }
    assert(failures && successes);printf("Shared outline prefixes: %u failed, %u passed\n",failures,successes);
}
static void fallbackPrefixes(void)
{
    unsigned setFailed=0,setPassed=0,getFailed=0,getPassed=0;
    for(int budget=0;budget<24;++budget){
        xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
        assert(xgeFontFallbackSetMemory(fontData,fontBytes,12)==XGE_OK);
        xge_font_face original=__xgeFontFace(&g_xge.tFallbackFont);assert(original);
        assert(xrtMemDebugFailAfter(budget));int status=xgeFontFallbackSetMemory(fontData,fontBytes,14);
        xrtMemDebugFailClear();xrtClearError();
        if(status==XGE_ERROR_OUT_OF_MEMORY){++setFailed;assert(__xgeFontFace(&g_xge.tFallbackFont)==original);}
        else {assert(status==XGE_OK);++setPassed;}
        xgeFontFallbackClear();equalHeap(&before);
    }
    for(int budget=0;budget<16;++budget){
        xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
        assert(xgeFontFallbackSetMemory(fontData,fontBytes,12)==XGE_OK);
        xge_font_face original=__xgeFontFace(&g_xge.tFallbackFont);assert(original);
        uint32_t refs=xrtAtomic32Load(&original->refs,XMEMORY_RELAXED);
        assert(xrtMemDebugFailAfter(budget));xge_font_t output={0};int status=xgeFontFallbackGet(&output,10);
        xrtMemDebugFailClear();xrtClearError();
        if(status==XGE_ERROR_OUT_OF_MEMORY){++getFailed;assert(!output.iRefCount);}
        else {assert(status==XGE_OK);++getPassed;}
        xgeFontFree(&output);
        assert(__xgeFontFace(&g_xge.tFallbackFont)==original && xrtAtomic32Load(&original->refs,XMEMORY_RELAXED)==refs);
        xgeFontFallbackClear();equalHeap(&before);
    }
    assert(setFailed && setPassed && getFailed && getPassed);
    printf("Fallback prefixes: set %u failed/%u passed; get %u failed/%u passed\n",setFailed,setPassed,getFailed,getPassed);
}
static void reset(void)
{
    assert(xgeFontFallbackSetMemory(fontData,fontBytes,12)==XGE_OK);
    xge_svg svg=NULL;assert(xgeSvgCreate(&svg)==XGE_OK);xge_font font=NULL;
    assert(__xgeSvgFontCacheGet(svg,"",10,&font)==XGE_OK && font);
    assert(xgeInit(NULL)==XGE_OK);
    /* Init still clears the global fallback; the independent document retains
     * its old Face. Dropping the old global Font no longer leaks its backend. */
    xge_font_t missing={0};assert(xgeFontFallbackGet(&missing,12)==XGE_ERROR_RESOURCE_FAILED);
    assert(xrtAtomic64Load(&g_xgeFontCount,XMEMORY_RELAXED)==1);
    xge_font_metrics_t metrics={0};assert(xgeFontGetMetrics(font,&metrics)==XGE_OK && metrics.fPixelSize==10);
    assert(xgeUnitChecked()==XGE_OK);xgeSvgDestroy(svg);
}
int main(int argc,char** argv)
{
    assert(argc==2);FILE* input=fopen(argv[1],"rb");assert(input);
    size_t bytes=fread(fontData,1,sizeof(fontData),input);assert(bytes && bytes<sizeof(fontData) && !ferror(input));
    assert(!fclose(input));fontBytes=(int)bytes;assert(xrtMemDebugEnable(true));
    parallel(false);parallel(true); /* Warm platform/thread bookkeeping. */
    xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
    sameSource();parallel(false);parallel(true);allocationPrefixes();fallbackPrefixes();reset();xrtClearError();equalHeap(&before);
    assert(!xrtAtomic64Load(&g_xgeFontCount,XMEMORY_RELAXED) && !xgeGraphicsOwnedResourceCount());
    printf("Shared Face and fallback passed; Face %zu bytes, 1000 hot outline hits without XRT heap operations\n",sizeof(struct xge_font_face_t));
    return 0;
}
