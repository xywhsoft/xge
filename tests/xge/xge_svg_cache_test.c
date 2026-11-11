/* Explicit caches: identity, independent ownership and callback return tails.
 * CPU-only, with a local immutable provider and no files/devices/network. */
#define XRT_MODULE_ALL
#include "single/xge.c"
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#if !defined(_WIN32)
#include <pthread.h>
#endif
static const char document[]="<svg width='32' height='24'><rect id='box' width='8' height='6'/></svg>";
typedef struct State {xge_svg_cache cache;int mode;unsigned invalidations;} State;
static _Atomic unsigned reads;
static int supply(const xstrview* path,xge_resource_output_t* output,void* raw)
{
    State* state=raw;assert(path && path->Data);atomic_fetch_add(&reads,1);
    if(state->mode==1){
        assert(state->cache->iRefCount==2);xgeSvgCacheDestroy(state->cache);state->cache=NULL;
    }else if(state->mode==2){
        xge_svg forbidden=(xge_svg)(uintptr_t)1;
        assert(xrtMemDebugFailAfter(0));
        assert(xgeSvgCacheLoad(state->cache,"cache-test://nested",&forbidden)==XGE_ERROR_BUSY);
        assert(forbidden==(xge_svg)(uintptr_t)1);
        assert(xgeSvgCacheReset(state->cache)==XGE_ERROR_BUSY);
        assert(xgeSvgCacheRemove(state->cache,"cache-test://source")==XGE_ERROR_BUSY);
        assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    }
    return xgeResourceOutputSet(output,document,sizeof(document)-1);
}
static void invalidated(void* raw)
{
    State* state=raw;++state->invalidations;
    if(state->mode==3){
        assert(state->cache->iRefCount==2 && state->cache->bActive);
        assert(xgeSvgCacheReset(state->cache)==XGE_ERROR_BUSY);
        xgeSvgCacheDestroy(state->cache);state->cache=NULL;
    }else if(state->mode==4){
        /* Bounded control: old global reset fails here before reentrant free. */
        assert(g_xgeSvgCacheHead==NULL);xgeSvgCacheClear();
        xge_svg fresh=NULL;assert(xgeSvgLoadCached("cache-test://new",&fresh)==XGE_OK);
        xgeSvgDestroy(fresh);
    }
}
static xge_svg_cache cache(void)
{xge_svg_cache value=NULL;assert(xgeSvgCacheCreate(&value)==XGE_OK && value);return value;}
static xge_svg load(xge_svg_cache value,const char* uri)
{xge_svg svg=NULL;assert(xgeSvgCacheLoad(value,uri,&svg)==XGE_OK && svg);return svg;}
static void alive(xge_svg svg,int expected)
{int refs=0;assert(xgeSvgRefCountGet(svg,&refs)==XGE_OK && refs==expected);}
static void watch(xge_svg svg,xge_svg_paint_watch_t* link,State* state)
{xge_svg_paint paint=NULL;assert(xgeSvgPaintGetPicture(svg,&paint)==XGE_OK);assert(xgeSvgPaintLifetimeWatch(paint,link,invalidated,state)==XGE_OK);}
typedef struct Worker {xge_svg result;} Worker;
static void independent(Worker* work)
{
    xge_svg_cache own=cache();xge_svg first=load(own,"cache-test://thread");
    for(int i=0;i<100;++i){xge_svg next=load(own,"cache-test://thread");assert(first==next);xgeSvgDestroy(next);}
    xgeSvgCacheDestroy(own);alive(first,1);work->result=first;
}
#if defined(_WIN32)
static DWORD WINAPI threadRun(LPVOID raw){independent(raw);return 0;}
#else
static void* threadRun(void* raw){independent(raw);return NULL;}
#endif
int main(int argc,char** argv)
{
    assert(argc==2);int mode=atoi(argv[1]);assert(xrtMemDebugEnable(true));
    State state={0};xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    assert(xgeResourceProviderAddOwned("cache-test",supply,&state,NULL)==XGE_OK);
    if(mode==0){
        xge_svg_cache a=cache(),b=cache();
        xge_svg first=load(a,"cache-test://same"),alias=load(a,"cache-test://same"),other=load(b,"cache-test://same");
        assert(first==alias && first!=other && atomic_load(&reads)==2);alive(first,3);
        assert(xgeSvgSetOrigin(first,3,7)==XGE_OK);float x=0,y=0;
        assert(xgeSvgGetOrigin(alias,&x,&y)==XGE_OK && x==3 && y==7);
        assert(xgeSvgGetOrigin(other,&x,&y)==XGE_OK && x==0 && y==0);
        assert(xgeSvgCacheReset(a)==XGE_OK);alive(first,2);
        xge_svg replaced=load(a,"cache-test://same");assert(replaced!=first);
        assert(xgeSvgCacheRemove(a,"cache-test://same")==XGE_OK);alive(replaced,1);
        assert(xgeSvgCacheRemove(a,"cache-test://same")==XGE_ERROR_NOT_FOUND);
        xgeSvgCacheDestroy(a);xgeSvgCacheDestroy(b);alive(other,1);
        xgeSvgDestroy(first);xgeSvgDestroy(alias);xgeSvgDestroy(other);xgeSvgDestroy(replaced);
    }else if(mode==1 || mode==2){
        state.cache=cache();state.mode=mode;
        xge_svg svg=load(state.cache,"cache-test://source");
        if(mode==1){assert(!state.cache);alive(svg,1);}
        else{xgeSvgCacheDestroy(state.cache);state.cache=NULL;alive(svg,1);}
        xgeSvgDestroy(svg);
    }else if(mode==3 || mode==4){
        state.cache=cache();xge_svg svg=load(state.cache,"cache-test://source");
        xge_svg_paint_watch_t link={0};state.mode=3;watch(svg,&link,&state);xgeSvgDestroy(svg);
        if(mode==3)assert(xgeSvgCacheReset(state.cache)==XGE_OK);
        else assert(xgeSvgCacheRemove(state.cache,"cache-test://source")==XGE_OK);
        assert(!state.cache && state.invalidations==1 && !link.ppPrevious);
    }else if(mode==5){
        state.cache=cache();xge_svg saved=load(state.cache,"cache-test://saved");
        unsigned failures=0,successes=0;
        for(int budget=0;budget<96;++budget){
            xmemdebugsnapshot prefixBefore,prefixAfter;xrtMemDebugSnapshot(&prefixBefore);
            xge_svg svg=(xge_svg)(uintptr_t)1;assert(xrtMemDebugFailAfter(budget));
            int status=xgeSvgCacheLoad(state.cache,"cache-test://target",&svg);xrtMemDebugFailClear();
            if(status==XGE_OK){++successes;assert(svg!=(xge_svg)(uintptr_t)1);xgeSvgDestroy(svg);assert(xgeSvgCacheRemove(state.cache,"cache-test://target")==XGE_OK);}
            else{++failures;assert(svg==(xge_svg)(uintptr_t)1 && state.cache->pEntries && !state.cache->pEntries->pNext);}
            xrtClearError();xrtMemDebugSnapshot(&prefixAfter);
            assert(prefixBefore.LiveCount==prefixAfter.LiveCount && prefixBefore.LiveBytes==prefixAfter.LiveBytes);
            assert(prefixBefore.InvalidFreeCount==prefixAfter.InvalidFreeCount && prefixBefore.DoubleFreeCount==prefixAfter.DoubleFreeCount);
        }
        assert(failures && successes);printf("cache allocation prefixes: %u failed, %u passed\n",failures,successes);
        assert(xrtMemDebugFailAfter(0));xge_svg hit=load(state.cache,"cache-test://saved");
        assert(hit==saved && !xrtMemDebugFailTriggered());xrtMemDebugFailClear();xgeSvgDestroy(hit);
        xgeSvgCacheDestroy(state.cache);state.cache=NULL;xgeSvgDestroy(saved);
    }else if(mode==6){
        xge_svg svg=NULL;assert(xgeSvgLoadCached("cache-test://old",&svg)==XGE_OK);
        xge_svg_paint_watch_t link={0};state.mode=4;watch(svg,&link,&state);xgeSvgDestroy(svg);
        xgeSvgCacheClear();assert(state.invalidations==1 && g_xgeSvgCacheHead && !strcmp(g_xgeSvgCacheHead->sURI,"cache-test://new"));
        assert(xgeSvgCacheInvalidate("cache-test://new")==XGE_OK && !g_xgeSvgCacheHead);
    }else if(mode==7){
        xge_svg_cache out=(xge_svg_cache)(uintptr_t)1;
        assert(xgeSvgCacheCreate(NULL)==XGE_ERROR_INVALID_ARGUMENT);
        assert(xrtMemDebugFailAfter(0));assert(xgeSvgCacheCreate(&out)==XGE_ERROR_OUT_OF_MEMORY);
        xrtMemDebugFailClear();assert(out==(xge_svg_cache)(uintptr_t)1);
        out=cache();xge_svg untouched=(xge_svg)(uintptr_t)1;
        assert(xgeSvgCacheLoad(NULL,"cache-test://source",&untouched)==XGE_ERROR_INVALID_ARGUMENT);
        assert(xgeSvgCacheLoad(out,NULL,&untouched)==XGE_ERROR_INVALID_ARGUMENT);
        assert(xgeSvgCacheLoad(out,"cache-test://source",NULL)==XGE_ERROR_INVALID_ARGUMENT);
        assert(untouched==(xge_svg)(uintptr_t)1);
        out->iRefCount=INT_MAX;assert(xgeSvgCacheReset(out)==XGE_ERROR_OUT_OF_MEMORY && !out->bActive);out->iRefCount=1;
        assert(xgeSvgCacheReset(NULL)==XGE_ERROR_INVALID_ARGUMENT);xgeSvgCacheDestroy(NULL);xgeSvgCacheDestroy(out);
    }else if(mode==8){
        Worker work[2]={{0},{0}};
#if defined(_WIN32)
        HANDLE threads[2];for(int i=0;i<2;++i){threads[i]=CreateThread(NULL,0,threadRun,&work[i],0,NULL);assert(threads[i]);}
        for(int i=0;i<2;++i){assert(WaitForSingleObject(threads[i],INFINITE)==WAIT_OBJECT_0);DWORD result=1;assert(GetExitCodeThread(threads[i],&result) && !result);assert(CloseHandle(threads[i]));}
#else
        pthread_t threads[2];for(int i=0;i<2;++i)assert(!pthread_create(&threads[i],NULL,threadRun,&work[i]));
        for(int i=0;i<2;++i)assert(!pthread_join(threads[i],NULL));
#endif
        assert(work[0].result && work[1].result && work[0].result!=work[1].result && atomic_load(&reads)==2);
        xgeSvgDestroy(work[0].result);xgeSvgDestroy(work[1].result);
    }else assert(!"unknown cache mode");
    assert(xgeResourceProviderClearChecked()==XGE_OK);xgeSvgCacheClear();xrtClearError();
    xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    printf("SVG cache mode %d: %zu-byte cache, shared identity and exact heap baseline\n",mode,sizeof(struct xge_svg_cache_t));
    return 0;
}
