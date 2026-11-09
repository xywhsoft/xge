/* The production admission gate, using real XRT synchronization. No device. */
#define XRT_MODULE_SPIN
#define XRT_IMPLEMENTATION
#include "extlibs/xrt/xrt.h"
#include "xge.h"
#include <assert.h>
#include <stdatomic.h>
#include "src/xge/core/xge_graphics_lifetime.h"
#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif
static xge_graphics_lifetime state=XGE_GRAPHICS_LIFETIME_INIT;
static atomic_int failures;
static void exercise(void)
{
    for(int i=0;i<10000;++i){
        int result=__xgeGraphicsOwnerAcquire(&state);
        if(result==XGE_OK){
            if(__xgeGraphicsOwnerRelease(&state)!=XGE_OK)atomic_fetch_add(&failures,1);
        }else if(result!=XGE_ERROR_BUSY)atomic_fetch_add(&failures,1);
    }
}
#if defined(_WIN32)
static DWORD WINAPI worker(void* ignored){(void)ignored;exercise();return 0;}
#else
static void* worker(void* ignored){(void)ignored;exercise();return NULL;}
#endif
int main(void)
{
    assert(__xgeGraphicsOwnerAcquire(&state)==XGE_OK);
    assert(__xgeGraphicsOwnerAcquire(&state)==XGE_OK);
    assert(__xgeGraphicsCloseBegin(&state)==XGE_ERROR_BUSY);
    assert(state.phase==XGE_GRAPHICS_COLD && !state.closing && state.owners==2);
    assert(__xgeGraphicsOwnerRelease(&state)==XGE_OK);
    assert(__xgeGraphicsOwnerRelease(&state)==XGE_OK);
    assert(__xgeGraphicsOwnerRelease(&state)==XGE_ERROR_INVALID_STATE);
    assert(__xgeGraphicsCloseBegin(&state)==XGE_OK);
    assert(__xgeGraphicsOwnerAcquire(&state)==XGE_ERROR_BUSY);
    __xgeGraphicsCloseAbort(&state);assert(__xgeGraphicsInitCheck(&state)==XGE_OK);
    __xgeGraphicsInitialized(&state);assert(__xgeGraphicsRunCheck(&state)==XGE_OK);
    assert(__xgeGraphicsBackendStart(&state)==XGE_OK);
    assert(__xgeGraphicsOwnerAcquire(&state)==XGE_OK);
    __xgeGraphicsBackendEnding(&state);assert(__xgeGraphicsOwnerAcquire(&state)==XGE_ERROR_BUSY);
    __xgeGraphicsBackendEnded(&state,true);
    assert(__xgeGraphicsInitCheck(&state)==XGE_ERROR_BUSY);
    assert(__xgeGraphicsRunCheck(&state)==XGE_ERROR_BUSY);
    assert(__xgeGraphicsBackendStart(&state)==XGE_ERROR_BUSY);
    assert(__xgeGraphicsOwnerRelease(&state)==XGE_OK);
    assert(__xgeGraphicsRunCheck(&state)==XGE_ERROR_BUSY); /* Unit first. */
    assert(__xgeGraphicsCloseBegin(&state)==XGE_OK);
    __xgeGraphicsCloseFinish(&state,false);assert(state.phase==XGE_GRAPHICS_COLD);
    __xgeGraphicsInitialized(&state);assert(__xgeGraphicsBackendStart(&state)==XGE_OK);
    assert(__xgeGraphicsCloseBegin(&state)==XGE_OK);__xgeGraphicsCloseFinish(&state,true);
    assert(__xgeGraphicsOwnerAcquire(&state)==XGE_ERROR_BUSY);
    assert(__xgeGraphicsInitCheck(&state)==XGE_ERROR_BUSY); /* Old context still live. */
    __xgeGraphicsBackendEnding(&state);__xgeGraphicsBackendEnded(&state,false);
    state.owners=UINT64_MAX;
    assert(__xgeGraphicsOwnerAcquire(&state)==XGE_ERROR_OUT_OF_MEMORY && state.owners==UINT64_MAX);
    state.owners=0;
#if defined(_WIN32)
    HANDLE threads[8];for(int i=0;i<8;++i){threads[i]=CreateThread(NULL,0,worker,NULL,0,NULL);assert(threads[i]);}
#else
    pthread_t threads[8];for(int i=0;i<8;++i)assert(!pthread_create(threads+i,NULL,worker,NULL));
#endif
    for(int i=0;i<5000;++i){
        int result=__xgeGraphicsCloseBegin(&state);
        if(result==XGE_OK){
            assert(__xgeGraphicsOwnerCount(&state)==0);
            assert(__xgeGraphicsOwnerAcquire(&state)==XGE_ERROR_BUSY);
            __xgeGraphicsCloseAbort(&state);
        }else assert(result==XGE_ERROR_BUSY);
    }
#if defined(_WIN32)
    assert(WaitForMultipleObjects(8,threads,TRUE,INFINITE)==WAIT_OBJECT_0);
    for(int i=0;i<8;++i)CloseHandle(threads[i]);
#else
    for(int i=0;i<8;++i)assert(!pthread_join(threads[i],NULL));
#endif
    assert(!atomic_load(&failures) && !__xgeGraphicsOwnerCount(&state));
    return 0;
}
