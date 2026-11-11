/* The production admission gate, using real XRT synchronization. No device. */
#define XRT_MODULE_SPIN
#define XRT_MODULE_THREAD
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
static int guardOnly;
static void exercise(void)
{
    for(int i=0;i<10000;++i){
        int result=__xgeGraphicsOwnerAcquire(&state);
        if(result==XGE_OK){
            if(__xgeGraphicsOwnerRelease(&state)!=XGE_OK)atomic_fetch_add(&failures,1);
        }else if(result!=XGE_ERROR_BUSY && result!=XGE_ERROR_INVALID_STATE)atomic_fetch_add(&failures,1);
    }
}
static void guarded(void)
{
    assert(__xgeAudioOwnerAcquire(&state)==XGE_ERROR_INVALID_STATE);
    assert(__xgeGraphicsContextBegin(&state)==XGE_ERROR_INVALID_STATE);
    assert(__xgeGraphicsOwnerAcquire(&state)==XGE_ERROR_INVALID_STATE);
    assert(__xgeGraphicsInitCheck(&state)==XGE_ERROR_INVALID_STATE);
    assert(__xgeGraphicsRunCheck(&state)==XGE_ERROR_INVALID_STATE);
    assert(__xgeGraphicsBackendStart(&state)==XGE_ERROR_INVALID_STATE);
    assert(__xgeGraphicsCloseBegin(&state)==XGE_ERROR_INVALID_STATE);
}
#if defined(_WIN32)
static DWORD WINAPI worker(void* ignored){(void)ignored;if(guardOnly)guarded();else exercise();return 0;}
#else
static void* worker(void* ignored){(void)ignored;if(guardOnly)guarded();else exercise();return NULL;}
#endif
int main(void)
{
    assert(__xgeAudioOwnerAcquire(&state)==XGE_ERROR_NOT_INITIALIZED);
    assert(__xgeAudioOwnerRelease(&state)==XGE_ERROR_INVALID_STATE);
    __xgeGraphicsInitialized(&state);
    assert(__xgeAudioOwnerAcquire(&state)==XGE_OK && state.audioOwners==1);
    assert(__xgeGraphicsOwnerCount(&state)==0);
    assert(__xgeGraphicsCloseBegin(&state)==XGE_ERROR_BUSY);
    assert(__xgeGraphicsContextBegin(&state)==XGE_OK);
    __xgeGraphicsContextCommit(&state);__xgeGraphicsContextFinish(&state,false);
    assert(!state.owners && state.audioOwners==1 && state.phase==XGE_GRAPHICS_READY);
    state.audioOwners=UINT64_MAX;
    assert(__xgeAudioOwnerAcquire(&state)==XGE_ERROR_OUT_OF_MEMORY && state.audioOwners==UINT64_MAX);
    state.audioOwners=1;
    assert(__xgeGraphicsBackendStart(&state)==XGE_OK);
    __xgeGraphicsBackendEnding(&state);__xgeGraphicsBackendEnded(&state,true);
    assert(__xgeAudioOwnerAcquire(&state)==XGE_OK && state.audioOwners==2);
    assert(__xgeGraphicsCloseBegin(&state)==XGE_ERROR_BUSY);
    assert(__xgeAudioOwnerRelease(&state)==XGE_OK && __xgeAudioOwnerRelease(&state)==XGE_OK);
    assert(__xgeGraphicsCloseBegin(&state)==XGE_OK);__xgeGraphicsCloseFinish(&state,false);
    assert(__xgeGraphicsContextBegin(&state)==XGE_ERROR_NOT_INITIALIZED);
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
    assert(__xgeGraphicsInitBegin(&state)==XGE_OK);
    assert(__xgeGraphicsInitBegin(&state)==XGE_ERROR_BUSY);
    assert(__xgeGraphicsCloseBegin(&state)==XGE_ERROR_BUSY);
    assert(__xgeGraphicsRunCheck(&state)==XGE_ERROR_BUSY);
    assert(__xgeGraphicsOwnerAcquire(&state)==XGE_OK && __xgeGraphicsOwnerRelease(&state)==XGE_OK && state.creatorThread);
    __xgeGraphicsInitAbort(&state);assert(!state.initializing && !state.creatorThread);
    __xgeGraphicsInitialized(&state);assert(__xgeGraphicsRunCheck(&state)==XGE_OK);
    assert(__xgeGraphicsOwnerAcquire(&state)==XGE_OK);
    assert(__xgeGraphicsContextBegin(&state)==XGE_OK && state.phase==XGE_GRAPHICS_ATTACHING && state.initializing && state.owners==2);
    assert(__xgeGraphicsOwnerAcquire(&state)==XGE_ERROR_BUSY);
    assert(__xgeGraphicsBackendStart(&state)==XGE_ERROR_BUSY);
    assert(__xgeGraphicsWindowAccess(&state)==XGE_ERROR_BUSY);
    assert(__xgeGraphicsContextBegin(&state)==XGE_ERROR_BUSY);
    assert(__xgeGraphicsCloseBegin(&state)==XGE_ERROR_BUSY);
    __xgeGraphicsContextCommit(&state);assert(state.phase==XGE_GRAPHICS_RUNNING && !state.initializing);
    __xgeGraphicsBackendEnding(&state);__xgeGraphicsContextFinish(&state,false);
    assert(state.phase==XGE_GRAPHICS_READY && state.owners==1 && !state.initializing && !state.closing);
    assert(__xgeGraphicsOwnerRelease(&state)==XGE_OK);
    assert(__xgeGraphicsContextBegin(&state)==XGE_OK);
    __xgeGraphicsContextFinish(&state,false); /* Failed attachment restores READY. */
    assert(state.phase==XGE_GRAPHICS_READY && !state.owners);
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
    __xgeGraphicsInitialized(&state);guardOnly=1;
#if defined(_WIN32)
    HANDLE guardedThread=CreateThread(NULL,0,worker,NULL,0,NULL);assert(guardedThread);
    assert(WaitForSingleObject(guardedThread,INFINITE)==WAIT_OBJECT_0);CloseHandle(guardedThread);
#else
    pthread_t guardedThread;assert(!pthread_create(&guardedThread,NULL,worker,NULL));assert(!pthread_join(guardedThread,NULL));
#endif
    guardOnly=0;assert(!state.closing && state.phase==XGE_GRAPHICS_READY && !state.owners);
    assert(__xgeGraphicsCloseBegin(&state)==XGE_OK);__xgeGraphicsCloseFinish(&state,false);
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
