/* CPU-only aggregate tests. No renderer, device, network or OS font needed.
 * Mutable packs stay caller-serialized here; publication policy is separate. */
#define XRT_MODULE_ALL
#include "single/xge.c"
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
static const char original[]="<svg width='8' height='8'><rect width='8' height='8' fill='red'/></svg>";
static const char replacement[]="<svg width='9' height='7'><rect width='9' height='7' fill='blue'/></svg>";
static const char longKey[]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static xatomic32 ready=XRT_ATOMIC32_INIT(0),go=XRT_ATOMIC32_INIT(0);
static void equalHeap(const xmemdebugsnapshot* before)
{
    xmemdebugsnapshot after;xrtMemDebugSnapshot(&after);
    assert(before->LiveCount==after.LiveCount && before->LiveBytes==after.LiveBytes &&
        before->InvalidFreeCount==after.InvalidFreeCount && before->DoubleFreeCount==after.DoubleFreeCount);
}
static xge_emoji_pack make(void)
{
    xge_emoji_pack pack=NULL;assert(xgeEmojiPackCreate(&pack)==XGE_OK && pack);return pack;
}
static xge_svg loadEntry(xge_emoji_pack pack,int entry)
{
    xrtMutexLock(pack->pCacheMutex);int status=__xgeEmojiEntryLoadLocked(pack,entry);
    xrtMutexUnlock(pack->pCacheMutex);assert(status==XGE_OK && pack->pEntries[entry].pSvg);
    return pack->pEntries[entry].pSvg;
}
static void invalidInput(void)
{
    xge_emoji_pack pack=make();xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    for(unsigned i=0;i<32;++i){
        uint32_t id=123;assert(xgeEmojiPackAddSvgMemory(pack,"Z\xff",original,sizeof(original)-1,NULL,&id)==XGE_ERROR_RESOURCE_FAILED);
        assert(id==123 && !pack->iEntryCount && pack->iNodeCount==1);
    }
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtClearError();equalHeap(&before);xgeEmojiPackFree(pack);
}
static void addPrefixes(void)
{
    for(unsigned kind=0;kind<2;++kind){
        unsigned failed=0,passed=0;
        for(int budget=0;budget<64;++budget){
            xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);xge_emoji_pack pack=make();
            uint32_t id=0;assert(xgeEmojiPackAddSvgMemory(pack,"B",original,sizeof(original)-1,NULL,&id)==XGE_OK && id==1);
            xge_svg old=loadEntry(pack,0);assert(xgeSvgAddRef(old)==XGE_OK);
            const char* key=kind ? "B":longKey;xge_emoji_metrics_t metrics=__xgeEmojiMetricsDefault();metrics.fAdvanceEm=2;
            id=123;assert(xrtMemDebugFailAfter(budget));
            int status=xgeEmojiPackAddSvgMemory(pack,key,replacement,sizeof(replacement)-1,&metrics,&id);
            xrtMemDebugFailClear();xrtClearError();
            if(status==XGE_OK){++passed;assert(id==(kind ? 1u:2u));}
            else{
                ++failed;assert(status==XGE_ERROR_OUT_OF_MEMORY && id==123 && pack->iEntryCount==1);
                xge_emoji_match_t match={0};assert(xgeEmojiPackMatch(pack,"B",1,&match)==XGE_OK && match.iEmojiId==1 && match.tMetrics.fAdvanceEm==1);
                assert(pack->pEntries[0].pSvg==old);
                if(!kind)assert(xgeEmojiPackMatch(pack,key,sizeof(longKey)-1,&match)==XGE_ERROR_NOT_FOUND);
                assert(xgeEmojiPackAddSvgMemory(pack,key,replacement,sizeof(replacement)-1,&metrics,&id)==XGE_OK && id==(kind ? 1u:2u));
            }
            assert(pack->iEntryCount==(kind ? 1:2));
            xge_emoji_match_t match={0};assert(xgeEmojiPackMatch(pack,key,(int)strlen(key),&match)==XGE_OK && match.iEmojiId==id && match.tMetrics.fAdvanceEm==2);
            float width=0,height=0;assert(xgeSvgGetSize(old,&width,&height)==XGE_OK && width==8 && height==8);
            xge_svg current=loadEntry(pack,(int)id-1);assert(xgeSvgGetSize(current,&width,&height)==XGE_OK && width==9 && height==7);
            xgeSvgDestroy(old);xgeEmojiPackFree(pack);equalHeap(&before);
        }
        assert(failed && passed);printf("Emoji add prefixes kind %u: %u failed, %u passed\n",kind,failed,passed);
    }
}
static void cachePrefixes(void)
{
    unsigned failed=0,passed=0;
    for(int budget=0;budget<96;++budget){
        xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);xge_emoji_pack pack=make();
        assert(xgeEmojiPackAddSvgMemory(pack,"A",original,sizeof(original)-1,NULL,NULL)==XGE_OK);
        xrtMutexLock(pack->pCacheMutex);assert(xrtMemDebugFailAfter(budget));
        int status=__xgeEmojiEntryLoadLocked(pack,0);
        xrtMemDebugFailClear();xrtClearError();xrtMutexUnlock(pack->pCacheMutex);
        if(status==XGE_OK)++passed;
        else{++failed;assert(status==XGE_ERROR_OUT_OF_MEMORY && !pack->pEntries[0].bLoadFailed && !pack->pEntries[0].pSvg);}
        assert(loadEntry(pack,0));xgeEmojiPackFree(pack);equalHeap(&before);
    }
    assert(failed && passed);printf("Emoji SVG cache prefixes: %u failed, %u passed\n",failed,passed);
}
static void builtinPrefixes(void)
{
    xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);xge_emoji_pack pack=NULL;
    assert(xgeEmojiPackLoadBuiltin(&pack)==XGE_OK && pack && pack->bBuiltin && pack->iEntryCount>0);
    xge_emoji_match_t match={0};const char grin[]="\xf0\x9f\x98\x80";
    assert(xgeEmojiPackMatch(pack,grin,sizeof(grin)-1,&match)==XGE_OK && match.iEmojiId>0);
    int entry=(int)match.iEmojiId-1;unsigned failed=0,passed=0;
    for(int budget=0;budget<96;++budget){
        xrtMutexLock(pack->pCacheMutex);
        if(pack->pEntries[entry].pSvg){xgeSvgDestroy(pack->pEntries[entry].pSvg);pack->pEntries[entry].pSvg=NULL;--pack->iCachedSvgCount;pack->iCachedSourceBytes-=pack->pEntries[entry].iSvgSize;}
        assert(xrtMemDebugFailAfter(budget));int status=__xgeEmojiEntryLoadLocked(pack,entry);
        xrtMemDebugFailClear();xrtClearError();xrtMutexUnlock(pack->pCacheMutex);
        if(status==XGE_OK)++passed;
        else{++failed;assert(status==XGE_ERROR_OUT_OF_MEMORY && !pack->pEntries[entry].bLoadFailed && !pack->pEntries[entry].pSvg);}
        assert(loadEntry(pack,entry));
    }
    assert(failed && passed);xmemdebugsnapshot hot,after;xrtMemDebugSnapshot(&hot);assert(xrtMemDebugFailAfter(0));
    for(unsigned i=0;i<1000;++i){match.iSize=sizeof(match);assert(xgeEmojiPackMatch(pack,grin,sizeof(grin)-1,&match)==XGE_OK);}
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtMemDebugSnapshot(&after);
    assert(hot.AllocCount==after.AllocCount && hot.FreeCount==after.FreeCount && hot.ReallocCount==after.ReallocCount);
    xgeEmojiPackFree(pack);__xgeEmojiGlobalClear();xrtClearError();equalHeap(&before);
    printf("Builtin Emoji cache prefixes: %u failed, %u passed; 1000 hot matches without XRT heap operations\n",failed,passed);
}
static void capacityBounds(void)
{
    struct xge_emoji_pack_t fake={0};fake.iNodeCapacity=fake.iEntryCapacity=INT32_MAX/2+1;
    assert(xrtMemDebugFailAfter(0));assert(__xgeEmojiEnsureNodes(&fake,INT32_MAX)==XGE_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();xrtClearError();
    assert(xrtMemDebugFailAfter(0));assert(__xgeEmojiEnsureEntries(&fake,INT32_MAX)==XGE_ERROR_OUT_OF_MEMORY);
    xrtMemDebugFailClear();xrtClearError();assert(!fake.pNodes && !fake.pEntries);
}
static void reenterError(const xge_error_info_t* error,void* raw)
{
    assert(error && error->iCode==XGE_ERROR_OUT_OF_MEMORY);++*(int*)raw;
    xgeEmojiPackClearDefault();
}
static void builtinErrorReentry(void)
{
    xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);int calls=0;
    assert(xgeSetErrorCallback(reenterError,&calls)==XGE_OK);
    assert(xrtMemDebugFailAfter(0));xge_emoji_pack pack=NULL;int status=xgeEmojiPackLoadBuiltin(&pack);
    xrtMemDebugFailClear();xrtClearError();assert(status==XGE_ERROR_OUT_OF_MEMORY && !pack && calls==1);
    assert(xgeSetErrorCallback(NULL,NULL)==XGE_OK);equalHeap(&before);
}
static int32_t builtinWorker(void* raw)
{
    xrtAtomic32FetchAdd(&ready,1,XMEMORY_RELEASE);
    while(!xrtAtomic32Load(&go,XMEMORY_ACQUIRE))xrtThreadYield();
    assert(xgeEmojiPackLoadBuiltin((xge_emoji_pack*)raw)==XGE_OK);return 0;
}
static void parallelBuiltin(void)
{
    xrtAtomic32Store(&ready,0,XMEMORY_RELAXED);xrtAtomic32Store(&go,0,XMEMORY_RELAXED);
    xge_emoji_pack packs[3]={NULL,NULL,NULL};xthread* jobs[3];
    for(unsigned i=0;i<3;++i){jobs[i]=xrtThreadCreate(builtinWorker,&packs[i],0);assert(jobs[i]);}
    while(xrtAtomic32Load(&ready,XMEMORY_ACQUIRE)!=3)xrtThreadYield();
    xrtAtomic32Store(&go,1,XMEMORY_RELEASE);
    for(unsigned i=0;i<3;++i){assert(xrtThreadWaitFor(jobs[i],30000)==XWAIT_OK && !xrtThreadExitCode(jobs[i]));xrtThreadDestroy(jobs[i]);}
    assert(packs[0] && packs[0]==packs[1] && packs[1]==packs[2]);
    assert(xrtAtomic32Load(&packs[0]->iRefCount,XMEMORY_RELAXED)==4);
    for(unsigned i=0;i<3;++i)xgeEmojiPackFree(packs[i]);
    __xgeEmojiGlobalClear();
}
static int32_t refWorker(void* raw)
{
    xge_emoji_pack pack=raw;xrtAtomic32FetchAdd(&ready,1,XMEMORY_RELEASE);
    while(!xrtAtomic32Load(&go,XMEMORY_ACQUIRE))xrtThreadYield();
    for(unsigned i=0;i<10000;++i){assert(xgeEmojiPackAddRef(pack)>1);xgeEmojiPackFree(pack);}return 0;
}
static void parallelReferences(void)
{
    xge_emoji_pack pack=make();xthread* jobs[3];
    for(unsigned i=0;i<3;++i){jobs[i]=xrtThreadCreate(refWorker,pack,0);assert(jobs[i]);}
    while(xrtAtomic32Load(&ready,XMEMORY_ACQUIRE)!=3)xrtThreadYield();
    xrtAtomic32Store(&go,1,XMEMORY_RELEASE);
    for(unsigned i=0;i<3;++i){assert(xrtThreadWaitFor(jobs[i],30000)==XWAIT_OK && !xrtThreadExitCode(jobs[i]));xrtThreadDestroy(jobs[i]);}
    assert(xrtAtomic32Load(&pack->iRefCount,XMEMORY_RELAXED)==1);
    /* Saturation rejects without wrapping an otherwise live owned reference. */
    xrtAtomic32Store(&pack->iRefCount,INT32_MAX,XMEMORY_RELAXED);assert(xgeEmojiPackAddRef(pack)==XGE_ERROR_INVALID_ARGUMENT);
    assert(xrtAtomic32Load(&pack->iRefCount,XMEMORY_RELAXED)==INT32_MAX);xrtAtomic32Store(&pack->iRefCount,1,XMEMORY_RELAXED);
    xgeEmojiPackFree(pack);
}
int main(void)
{
    setvbuf(stdout,NULL,_IONBF,0);assert(xrtMemDebugEnable(true));
    invalidInput();addPrefixes();cachePrefixes();builtinPrefixes();capacityBounds();parallelReferences();
    builtinErrorReentry();parallelBuiltin();
    assert(!xgeGraphicsOwnedResourceCount());printf("Emoji native ownership, transaction and retry passed; pack bytes %zu\n",sizeof(struct xge_emoji_pack_t));return 0;
}
