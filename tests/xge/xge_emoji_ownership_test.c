/* CPU-only aggregate tests. No renderer, device, network or OS font needed. */
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
    xge_emoji_cache_t* cache=NULL;assert(__xgeEmojiEntryAcquire(pack,entry,0,&cache)==XGE_OK && cache);
    xge_svg svg=cache->pSvg;
    __xgeEmojiEntryRelease(pack,cache);
    return svg;
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
                assert(pack->pEntries[0].pCache->pSvg==old);
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
        assert(xrtMemDebugFailAfter(budget));xge_emoji_cache_t* cache=NULL;
        int status=__xgeEmojiEntryAcquire(pack,0,0,&cache);
        xrtMemDebugFailClear();xrtClearError();
        if(cache)__xgeEmojiEntryRelease(pack,cache);
        if(status==XGE_OK)++passed;
        else{++failed;assert(status==XGE_ERROR_OUT_OF_MEMORY && !pack->pEntries[0].bLoadFailed && !pack->pEntries[0].pCache);}
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
        if(pack->pEntries[entry].pCache){__xgeEmojiCacheDrop(pack->pEntries[entry].pCache);pack->pEntries[entry].pCache=NULL;--pack->iCachedSvgCount;pack->iCachedSourceBytes-=pack->pEntries[entry].iSvgSize;}
        xrtMutexUnlock(pack->pCacheMutex);
        assert(xrtMemDebugFailAfter(budget));xge_emoji_cache_t* cache=NULL;
        int status=__xgeEmojiEntryAcquire(pack,entry,0,&cache);
        xrtMemDebugFailClear();xrtClearError();
        if(cache)__xgeEmojiEntryRelease(pack,cache);
        if(status==XGE_OK)++passed;
        else{++failed;assert(status==XGE_ERROR_OUT_OF_MEMORY && !pack->pEntries[entry].bLoadFailed && !pack->pEntries[entry].pCache);}
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

typedef struct publication_test_t { xge_emoji_pack pack; bool write; } publication_test_t;
static int32_t publicationWorker(void* raw)
{
    publication_test_t* test=raw;
    xrtAtomic32FetchAdd(&ready,1,XMEMORY_RELEASE);
    while(!xrtAtomic32Load(&go,XMEMORY_ACQUIRE))xrtThreadYield();
    for(unsigned i=0;i<1000;++i){
        if(test->write){
            char sequence[32];
            snprintf(sequence,sizeof(sequence),"B%u",i);
            assert(xgeEmojiPackAddSvgMemory(test->pack,sequence,replacement,sizeof(replacement)-1,NULL,NULL)==XGE_OK);
            xge_emoji_metrics_t metrics=__xgeEmojiMetricsDefault();
            metrics.fAdvanceEm=metrics.fWidthEm=(float)(1+i%2);
            assert(xgeEmojiPackAddSvgMemory(test->pack,"A",original,sizeof(original)-1,&metrics,NULL)==XGE_OK);
        }else{
            xge_emoji_match_t match={0};
            assert(xgeEmojiPackMatch(test->pack,"A",1,&match)==XGE_OK && match.iEmojiId==1);
            assert(match.tMetrics.fAdvanceEm==match.tMetrics.fWidthEm);
        }
    }
    return 0;
}
static void parallelPublication(void)
{
    xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
    xge_emoji_pack pack=make(),shared=NULL;
    assert(xgeEmojiPackAddSvgMemory(pack,"A",original,sizeof(original)-1,NULL,NULL)==XGE_OK);
    assert(xgeEmojiPackSetDefault(pack)==XGE_OK);
    assert(xgeEmojiPackGetDefault(&shared)==XGE_OK && shared==pack);
    xrtAtomic32Store(&ready,0,XMEMORY_RELAXED);xrtAtomic32Store(&go,0,XMEMORY_RELAXED);
    publication_test_t args[2]={{pack,true},{shared,false}};
    xthread* jobs[2]={xrtThreadCreate(publicationWorker,&args[0],0),xrtThreadCreate(publicationWorker,&args[1],0)};
    assert(jobs[0] && jobs[1]);
    while(xrtAtomic32Load(&ready,XMEMORY_ACQUIRE)!=2)xrtThreadYield();
    xrtAtomic32Store(&go,1,XMEMORY_RELEASE);
    for(unsigned i=0;i<2;++i){assert(xrtThreadWaitFor(jobs[i],30000)==XWAIT_OK && !xrtThreadExitCode(jobs[i]));xrtThreadDestroy(jobs[i]);}
    xgeEmojiPackClearDefault();xgeEmojiPackFree(shared);xgeEmojiPackFree(pack);
    equalHeap(&before);
}

typedef struct reentrant_test_t { xge_emoji_pack pack; int calls; } reentrant_test_t;
static int replaceDuringSvgLoad(const char* uri,void** data,int* size,void* raw)
{
    (void)uri;(void)data;(void)size;
    reentrant_test_t* test=raw;
    xge_emoji_cache_t* cache=NULL;
    assert(__xgeEmojiEntryAcquire(test->pack,0,0,&cache)==XGE_ERROR_BUSY && !cache);
    xge_emoji_match_t match={0};
    assert(xgeEmojiPackMatch(test->pack,"A",1,&match)==XGE_OK && match.iEmojiId==1);
    assert(xgeEmojiPackAddSvgMemory(test->pack,"A",replacement,sizeof(replacement)-1,NULL,NULL)==XGE_OK);
    ++test->calls;
    return XGE_ERROR_NOT_FOUND;
}
static void reentrantCacheLoad(void)
{
    const char external[]="<svg width='8' height='8'><image href='emoji-test://pixel' width='8' height='8'/></svg>";
    reentrant_test_t test={make(),0};
    const xge_resource_provider_t provider={"emoji-test",replaceDuringSvgLoad,NULL,&test};
    assert(xgeResourceProviderAdd(&provider)==XGE_OK);
    assert(xgeEmojiPackAddSvgMemory(test.pack,"A",external,sizeof(external)-1,NULL,NULL)==XGE_OK);
    xge_emoji_cache_t* old=NULL;
    assert(__xgeEmojiEntryAcquire(test.pack,0,0,&old)==XGE_OK && old && test.calls==1);
    assert(!test.pack->pEntries[0].pCache && !test.pack->pEntries[0].bLoading);
    float width=0,height=0;
    assert(xgeSvgGetSize(old->pSvg,&width,&height)==XGE_OK && width==8 && height==8);
    assert(xgeSvgGetSize(loadEntry(test.pack,0),&width,&height)==XGE_OK && width==9 && height==7);
    __xgeEmojiEntryRelease(test.pack,old);
    assert(xgeResourceProviderClearChecked()==XGE_OK);
    xgeEmojiPackFree(test.pack);
}

static int32_t dropDrawnPack(void* raw)
{
    xge_emoji_pack pack=raw;
    xge_emoji_cache_t* cache=NULL;
    assert(__xgeEmojiEntryAcquire(pack,0,xrtThreadCurrentId(),&cache)==XGE_ERROR_INVALID_STATE && !cache);
    xgeEmojiPackFree(pack);
    __xgeEmojiDrainRetired();
    return 0;
}
static void deferredDrawDisposal(void)
{
    xmemdebugsnapshot before;xrtMemDebugSnapshot(&before);
    xge_emoji_pack pack=make();
    assert(xgeEmojiPackAddSvgMemory(pack,"A",original,sizeof(original)-1,NULL,NULL)==XGE_OK);
    xge_emoji_cache_t* cache=NULL;
    assert(__xgeEmojiEntryAcquire(pack,0,xrtThreadCurrentId(),&cache)==XGE_OK);
    __xgeEmojiEntryRelease(pack,cache);
    xthread* worker=xrtThreadCreate(dropDrawnPack,pack,0);assert(worker);
    assert(xrtThreadWaitFor(worker,30000)==XWAIT_OK && !xrtThreadExitCode(worker));
    xrtThreadDestroy(worker);
    assert(g_xgeEmojiRetired==cache);
    assert(xgeUnitChecked()==XGE_OK && !g_xgeEmojiRetired);
    equalHeap(&before);
}
int main(void)
{
    setvbuf(stdout,NULL,_IONBF,0);assert(xrtMemDebugEnable(true));
    invalidInput();addPrefixes();cachePrefixes();builtinPrefixes();capacityBounds();parallelReferences();
    builtinErrorReentry();parallelBuiltin();parallelPublication();reentrantCacheLoad();deferredDrawDisposal();
    assert(!xgeGraphicsOwnedResourceCount());printf("Emoji native ownership, transaction and retry passed; pack bytes %zu\n",sizeof(struct xge_emoji_pack_t));return 0;
}
