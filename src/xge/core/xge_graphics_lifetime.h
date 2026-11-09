#ifndef XGE_GRAPHICS_LIFETIME_H
#define XGE_GRAPHICS_LIFETIME_H
/* Cold admission for native owned graphics records. The lock is never held
 * across allocation, a graphics call or a user callback. Kept outside g_xge
 * so initialization cannot erase a live resource's ownership obligation. */
typedef enum xge_graphics_phase {
    XGE_GRAPHICS_COLD, XGE_GRAPHICS_READY, XGE_GRAPHICS_RUNNING,
    XGE_GRAPHICS_ENDED, XGE_GRAPHICS_CLOSED
} xge_graphics_phase;
typedef struct xge_graphics_lifetime {
    xspinlock lock;
    uint64_t owners;
    xge_graphics_phase phase;
    bool closing;
} xge_graphics_lifetime;
#define XGE_GRAPHICS_LIFETIME_INIT {XRT_SPIN_INIT,0,XGE_GRAPHICS_COLD,false}

static int __xgeGraphicsOwnerAcquire(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);
    int result=XGE_OK;
    if(state->closing || state->phase==XGE_GRAPHICS_ENDED || state->phase==XGE_GRAPHICS_CLOSED)result=XGE_ERROR_BUSY;
    else if(state->owners==UINT64_MAX)result=XGE_ERROR_OUT_OF_MEMORY;
    else ++state->owners;
    (void)xrtSpinUnlock(&state->lock);return result;
}
static int __xgeGraphicsOwnerRelease(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);
    int result=state->owners ? (--state->owners,XGE_OK):XGE_ERROR_INVALID_STATE;
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
    int result=state->closing || state->owners ? XGE_ERROR_BUSY:XGE_OK;
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
    (void)xrtSpinUnlock(&state->lock);
}
static int __xgeGraphicsInitCheck(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);
    int result=state->closing || state->phase==XGE_GRAPHICS_CLOSED ||
        (state->phase==XGE_GRAPHICS_ENDED && state->owners) ? XGE_ERROR_BUSY:XGE_OK;
    (void)xrtSpinUnlock(&state->lock);return result;
}
static void __xgeGraphicsInitialized(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);state->phase=XGE_GRAPHICS_READY;
    (void)xrtSpinUnlock(&state->lock);
}
static int __xgeGraphicsBackendStart(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);
    int result=!state->closing && state->phase==XGE_GRAPHICS_READY ? XGE_OK:XGE_ERROR_BUSY;
    if(result==XGE_OK)state->phase=XGE_GRAPHICS_RUNNING;
    (void)xrtSpinUnlock(&state->lock);return result;
}
static int __xgeGraphicsRunCheck(xge_graphics_lifetime* state)
{
    (void)xrtSpinLock(&state->lock);
    int result=!state->closing && state->phase==XGE_GRAPHICS_READY ? XGE_OK:XGE_ERROR_BUSY;
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
    (void)xrtSpinUnlock(&state->lock);
}
#endif
