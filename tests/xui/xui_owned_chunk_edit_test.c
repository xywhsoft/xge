/* Native incremental cache transaction, CPU-only. */
#define XUI_OWNED_LAYOUT_HELPERS 1
#include "xui_owned_layout_test.c"
#undef XUI_OWNED_LAYOUT_HELPERS
#include "src/xui/core/xui_text_internal.h"
static void cacheEditWindow(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);
    const int bytes=48*1024;char* text=xrtMalloc((size_t)bytes+1);assert(text);memset(text,'a',(size_t)bytes);text[bytes]=0;
    xui_internal_text_chunk_cache_t cache={0};
    assert(xuiInternalTextChunkCacheEnsure(p.context,&cache,font,text,bytes,0)==XUI_OK && cache.iChunkCount>6);
    int count=cache.iChunkCount;void* paints[32];assert(count<32);
    for(int i=0;i<count;++i)paints[i]=cache.pChunks[i].tShape.pPaint;
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(p.context,&scope)==XUI_OK);
    xui_internal_text_chunk_cache_t original=cache;p.ready=false;
    assert(xuiInternalTextChunkCachePrepareEdit(p.context,&cache,20000,20001,20001)==XUI_ERROR_BUSY);
    assert(!memcmp(&original,&cache,sizeof(cache)) && !p.paintDrops);
    p.ready=true;assert(xuiInternalTextChunkCachePrepareEdit(p.context,&cache,20000,20001,20001)==XUI_OK);
    assert(cache.iPreparation==2 && !cache.bValid && p.paintDrops>=2 && p.paintDrops<=3);
    unsigned dropped=p.paintDrops;
    assert(xuiInternalTextChunkCacheEnsure(p.context,&cache,font,text,bytes,0)==XUI_ERROR_BUSY);
    assert(xuiInternalTextChunkCacheClearChecked(p.context,&cache)==XUI_ERROR_BUSY);
    assert(xuiInternalTextChunkCacheWidth(&cache)==0);
    text[20000]='b';assert(xuiInternalTextChunkCachePatch(p.context,&cache,text,bytes,20000,20001,20001)==XUI_OK);
    assert(cache.bValid && !cache.iPreparation && cache.iChunkCount==count && p.paintDrops==dropped);
    unsigned retained=0;for(int i=0;i<count;++i)if(cache.pChunks[i].tShape.pPaint==paints[i])++retained;
    assert(retained>=(unsigned)count-dropped && cache.pChunks[0].tShape.pPaint==paints[0] && cache.pChunks[count-1].tShape.pPaint==paints[count-1]);
    assert(xuiCallScopeEnd(&scope)==XUI_OK);
    assert(xuiInternalTextChunkCacheClearChecked(p.context,&cache)==XUI_OK && refs(xuiFontOwnership(font))==1);
    xrtFree(text);assert(xuiDestroyChecked(p.context)==XUI_OK && xuiFontReleaseChecked(font)==XUI_OK && xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner));
}
static void cachePrepareFailureAndPatchOom(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);
    const int bytes=32*1024;char* text=xrtMalloc((size_t)bytes+1);assert(text);memset(text,'a',(size_t)bytes);text[bytes]=0;
    xui_internal_text_chunk_cache_t cache={0};assert(xuiInternalTextChunkCacheEnsure(p.context,&cache,font,text,bytes,0)==XUI_OK);
    xui_call_scope_t scope={0};assert(xuiCallScopeBegin(p.context,&scope)==XUI_OK);
    p.paintStatus=XUI_ERROR_BACKEND_FAILED;
    assert(xuiInternalTextChunkCachePrepareEdit(p.context,&cache,9000,9001,9001)==XUI_ERROR_BACKEND_FAILED);
    assert(!cache.bValid && !cache.iPreparation && text[9000]=='a' && xrtGetError()==NULL);
    assert(xuiCallScopeEnd(&scope)==XUI_ERROR_BACKEND_FAILED && xrtGetError()==NULL);
    p.paintStatus=XUI_OK;assert(xuiInternalTextChunkCacheClearChecked(p.context,&cache)==XUI_OK);
    assert(xuiInternalTextChunkCacheEnsure(p.context,&cache,font,text,bytes,0)==XUI_OK);
    assert(xuiCallScopeBegin(p.context,&scope)==XUI_OK);
    assert(xuiInternalTextChunkCachePrepareEdit(p.context,&cache,9000,9001,9001)==XUI_OK);
    assert(xrtMemDebugFailAfter(0));text[9000]='b';
    assert(xuiInternalTextChunkCachePatch(p.context,&cache,text,bytes,9000,9001,9001)==XUI_ERROR_OUT_OF_MEMORY);xrtMemDebugFailClear();xrtClearError();
    assert(!cache.bValid && !cache.iPreparation && cache.iChunkCount>0);
    p.ready=false;assert(xuiInternalTextChunkCacheClearChecked(p.context,&cache)==XUI_ERROR_BUSY && cache.iChunkCount>0);
    p.ready=true;assert(xuiInternalTextChunkCacheClearChecked(p.context,&cache)==XUI_OK);
    assert(xuiCallScopeEnd(&scope)==XUI_OK);
    xrtFree(text);assert(xuiDestroyChecked(p.context)==XUI_OK && xuiFontReleaseChecked(font)==XUI_OK && xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner));
}
static void cacheSuccessfulBusyThenOOM(void)
{
    LayoutProvider p={0};xui_font font;layoutSetup(&p,&font);
    const int bytes=32*1024;char* text=xrtMalloc((size_t)bytes+1);assert(text);memset(text,'x',(size_t)bytes);text[bytes]=0;
    xui_internal_text_chunk_cache_t cache={0};xui_call_scope_t scope={0};assert(xuiCallScopeBegin(p.context,&scope)==XUI_OK);
    p.busyNextShape=p.oomAfterShape=true;
    assert(xuiInternalTextChunkCacheEnsure(p.context,&cache,font,text,bytes,0)==XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();xrtClearError();assert(cache.iChunkCount==1 && cache.pChunks[0].tShape.pPaint && !cache.bValid);
    unsigned edges=0;assert(xuiInternalTextChunkCacheTraceOwnership(&cache,layoutVisit,&edges) && edges==1);
    bool complete=true;assert(xuiInternalTextChunkCacheDestroyPrepare(p.context,&cache,&complete)==XUI_STATUS_DEFERRED && !complete && !p.paintDrops);
    assert(xuiCallScopeEnd(&scope)==XUI_OK && !xrtGetError());
    p.ready=true;assert(xuiInternalTextChunkCacheClearChecked(p.context,&cache)==XUI_OK && p.paintDrops==1);
    assert(xuiInternalTextChunkCacheEnsure(p.context,&cache,font,text,bytes,0)==XUI_OK);
    int oldCount=cache.iChunkCount;
    assert(xuiCallScopeBegin(p.context,&scope)==XUI_OK);
    assert(xuiInternalTextChunkCachePrepareEdit(p.context,&cache,16000,16001,16001)==XUI_OK);text[16000]='y';
    p.busyNextShape=p.oomAfterShape=true;
    assert(xuiInternalTextChunkCachePatch(p.context,&cache,text,bytes,16000,16001,16001)==XUI_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();xrtClearError();assert(cache.iChunkCount>oldCount && !cache.bValid && !cache.iPreparation);
    assert(xuiCallScopeEnd(&scope)==XUI_OK);
    p.ready=true;assert(xuiInternalTextChunkCacheClearChecked(p.context,&cache)==XUI_OK);
    xrtFree(text);assert(xuiDestroyChecked(p.context)==XUI_OK && xuiFontReleaseChecked(font)==XUI_OK && xuiProxyReleaseChecked(p.proxy)==XUI_OK && drop(&p.owner));
    assert(p.shapes==p.paintDrops);
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;
    assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK && xrtMemDebugEnable(true));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);cacheEditWindow();cachePrepareFailureAndPatchOom();cacheSuccessfulBusyThenOOM();
    xrtClearError();xrtMemDebugSnapshot(&after);assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    puts("chunk edit: BUSY preserves record/text, physical affected-window retirement, reentry reservation, unaffected paint reuse and OOM carrier retention passed");return 0;
}
