#ifndef XGE_GRAPHICS_LIFETIME_H
#define XGE_GRAPHICS_LIFETIME_H
#include <assert.h>
/* Cold admission for native owned graphics records. The lock is never held
 * across allocation, a graphics call or a user callback. Kept outside g_xge
 * so initialization cannot erase a live resource's ownership obligation. */
typedef enum xge_graphics_phase {
    XGE_GRAPHICS_COLD, XGE_GRAPHICS_READY, XGE_GRAPHICS_ATTACHING, XGE_GRAPHICS_RUNNING,
    XGE_GRAPHICS_ENDED, XGE_GRAPHICS_CLOSED
} xge_graphics_phase;
typedef struct xge_graphics_lifetime {
    xspinlock lock;
    uint64_t owners;
    uint64_t creatorThread;
    xge_graphics_phase phase;
    bool closing;
    bool initializing;
} xge_graphics_lifetime;
#define XGE_GRAPHICS_LIFETIME_INIT {XRT_SPIN_INIT,0,0,XGE_GRAPHICS_COLD,false,false}

static int __xgeGraphicsOwnerAcquire(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);
    int result=XGE_OK;
    if(state->closing || state->phase==XGE_GRAPHICS_ATTACHING || state->phase==XGE_GRAPHICS_ENDED || state->phase==XGE_GRAPHICS_CLOSED)result=XGE_ERROR_BUSY;
    else if(state->creatorThread && state->creatorThread!=xrtThreadCurrentId())result=XGE_ERROR_INVALID_STATE;
    else if(state->owners==UINT64_MAX)result=XGE_ERROR_OUT_OF_MEMORY;
    else {if(!state->creatorThread)state->creatorThread=xrtThreadCurrentId();++state->owners;}
    (void)xrtSpinUnlock(&state->lock);return result;
}
static int __xgeGraphicsOwnerRelease(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);
    int result=state->owners ? (--state->owners,XGE_OK):XGE_ERROR_INVALID_STATE;
    if(!state->owners && state->phase==XGE_GRAPHICS_COLD && !state->initializing)state->creatorThread=0;
    (void)xrtSpinUnlock(&state->lock);return result;
}
static uint64_t __xgeGraphicsOwnerCount(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);uint64_t result=state->owners;
    (void)xrtSpinUnlock(&state->lock);return result;
}
static int __xgeGraphicsCloseBegin(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);
    int result=state->closing || state->initializing || state->owners ? XGE_ERROR_BUSY:XGE_OK;
    if(state->phase!=XGE_GRAPHICS_COLD && state->creatorThread && state->creatorThread!=xrtThreadCurrentId())result=XGE_ERROR_INVALID_STATE;
    if(result==XGE_OK)state->closing=true;
    (void)xrtSpinUnlock(&state->lock);return result;
}
static void __xgeGraphicsCloseAbort(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);state->closing=false;
    (void)xrtSpinUnlock(&state->lock);
}
static void __xgeGraphicsCloseFinish(xge_graphics_lifetime* state,bool backendRunning)
{
    (void)xrtSpinLock(&state->lock);
    state->phase=backendRunning ? XGE_GRAPHICS_CLOSED:XGE_GRAPHICS_COLD;state->closing=false;
    if(!backendRunning)state->creatorThread=0;
    (void)xrtSpinUnlock(&state->lock);
}
static int __xgeGraphicsInitCheck(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);
    int result=state->closing || state->initializing || state->phase==XGE_GRAPHICS_CLOSED ||
        (state->phase==XGE_GRAPHICS_ENDED && state->owners) ? XGE_ERROR_BUSY:XGE_OK;
    if(state->creatorThread && state->creatorThread!=xrtThreadCurrentId())result=XGE_ERROR_INVALID_STATE;
    (void)xrtSpinUnlock(&state->lock);return result;
}
static int __xgeGraphicsInitBegin(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);int result=XGE_OK;
    if(state->closing || state->initializing || state->phase==XGE_GRAPHICS_CLOSED)result=XGE_ERROR_BUSY;
    else if(state->creatorThread && state->creatorThread!=xrtThreadCurrentId())result=XGE_ERROR_INVALID_STATE;
    else if(state->phase==XGE_GRAPHICS_ENDED && state->owners)result=XGE_ERROR_BUSY;
    else if(state->phase!=XGE_GRAPHICS_COLD)result=XGE_ERROR_ALREADY_INITIALIZED;
    else {state->initializing=true;state->creatorThread=xrtThreadCurrentId();}
    (void)xrtSpinUnlock(&state->lock);return result;
}
static void __xgeGraphicsInitAbort(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);state->initializing=false;
    if(state->phase==XGE_GRAPHICS_COLD && !state->owners)state->creatorThread=0;
    (void)xrtSpinUnlock(&state->lock);
}
static void __xgeGraphicsInitialized(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);state->phase=XGE_GRAPHICS_READY;state->creatorThread=xrtThreadCurrentId();state->initializing=false;
    (void)xrtSpinUnlock(&state->lock);
}
static int __xgeGraphicsBackendStart(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);
    int result=!state->closing && state->phase==XGE_GRAPHICS_READY ? XGE_OK:XGE_ERROR_BUSY;
    if(state->creatorThread!=xrtThreadCurrentId())result=XGE_ERROR_INVALID_STATE;
    if(result==XGE_OK)state->phase=XGE_GRAPHICS_RUNNING;
    (void)xrtSpinUnlock(&state->lock);return result;
}
static int __xgeGraphicsRunCheck(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);
    int result=state->initializing || state->closing ? XGE_ERROR_BUSY:
        state->phase==XGE_GRAPHICS_COLD ? XGE_ERROR_NOT_INITIALIZED:
        state->phase==XGE_GRAPHICS_READY ? XGE_OK:XGE_ERROR_BUSY;
    if(state->creatorThread && state->creatorThread!=xrtThreadCurrentId())result=XGE_ERROR_INVALID_STATE;
    (void)xrtSpinUnlock(&state->lock);return result;
}
/* Creator-thread window configuration, valid both before and during Run. */
static int __xgeGraphicsWindowAccess(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);
    int result=state->initializing || state->closing ? XGE_ERROR_BUSY:
        state->phase==XGE_GRAPHICS_COLD ? XGE_ERROR_NOT_INITIALIZED:
        state->phase==XGE_GRAPHICS_READY || state->phase==XGE_GRAPHICS_RUNNING ? XGE_OK:XGE_ERROR_BUSY;
    if(state->creatorThread && state->creatorThread!=xrtThreadCurrentId())result=XGE_ERROR_INVALID_STATE;
    (void)xrtSpinUnlock(&state->lock);return result;
}
static void __xgeGraphicsBackendEnding(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);state->closing=true;
    (void)xrtSpinUnlock(&state->lock);
}
static void __xgeGraphicsBackendEnded(xge_graphics_lifetime* state,bool initialized)
{
    (void)xrtSpinLock(&state->lock);
    state->phase=initialized ? XGE_GRAPHICS_ENDED:XGE_GRAPHICS_COLD;state->closing=false;
    if(!initialized)state->creatorThread=0;
    (void)xrtSpinUnlock(&state->lock);
}
/* A context owns one reservation from admission through its final code Drop.
 * Ordinary engine initialization retains its existing preload/adoption rule. */
static int __xgeGraphicsContextBegin(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);int result=XGE_OK;
    if(state->creatorThread && state->creatorThread!=xrtThreadCurrentId())result=XGE_ERROR_INVALID_STATE;
    else if(state->phase==XGE_GRAPHICS_COLD)result=XGE_ERROR_NOT_INITIALIZED;
    else if(state->initializing || state->closing || state->phase!=XGE_GRAPHICS_READY)result=XGE_ERROR_BUSY;
    else if(state->owners==UINT64_MAX)result=XGE_ERROR_OUT_OF_MEMORY;
    else {++state->owners;state->phase=XGE_GRAPHICS_ATTACHING;state->initializing=true;}
    (void)xrtSpinUnlock(&state->lock);return result;
}
static void __xgeGraphicsContextCommit(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);
    assert(state->phase==XGE_GRAPHICS_ATTACHING && state->initializing && state->owners);
    state->phase=XGE_GRAPHICS_RUNNING;state->initializing=false;
    (void)xrtSpinUnlock(&state->lock);
}
static void __xgeGraphicsContextFinish(xge_graphics_lifetime* state,bool ended)
{
    (void)xrtSpinLock(&state->lock);
    assert(state->owners && state->creatorThread==xrtThreadCurrentId());
    --state->owners;state->phase=ended ? XGE_GRAPHICS_ENDED:XGE_GRAPHICS_READY;state->initializing=state->closing=false;
    (void)xrtSpinUnlock(&state->lock);
}
#endif
