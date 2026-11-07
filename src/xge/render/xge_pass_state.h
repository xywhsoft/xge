#ifndef XGE_PASS_STATE_INTERNAL_H
#define XGE_PASS_STATE_INTERNAL_H
/* The weak chain names actual caller-owned Pass records. It owns no target,
 * allocation or code. Its short lock never spans Flush, GL or user callbacks. */
typedef struct xge_pass_state_t {
    xatomic32 lock;
    xge_pass current;
    uint64_t thread;
} xge_pass_state_t;
enum {XGE_PASS_STARTING=-1,XGE_PASS_ACTIVE=1,XGE_PASS_ENDING=-2};
static xge_pass_state_t g_xgePassState;
static bool __xgePassStateTryLock(xge_pass_state_t* state)
{
    uint32_t expected=0;
    return xrtAtomic32CompareExchange(&state->lock,&expected,1,XMEMORY_ACQUIRE,XMEMORY_RELAXED);
}
static void __xgePassStateLock(xge_pass_state_t* state)
{while(!__xgePassStateTryLock(state)){} }
static void __xgePassStateUnlock(xge_pass_state_t* state)
{xrtAtomic32Store(&state->lock,0,XMEMORY_RELEASE);}
static bool __xgePassStateIdle(xge_pass_state_t* state)
{
    if(!__xgePassStateTryLock(state))return false;
    bool result=state->current==NULL;__xgePassStateUnlock(state);return result;
}
static bool __xgePassStateCurrent(xge_pass_state_t* state,const xge_pass_t* pass)
{
    if(!pass || !__xgePassStateTryLock(state))return false;
    bool result=state->thread==xrtThreadCurrentId() && state->current==pass && pass->bActive==XGE_PASS_ACTIVE;
    __xgePassStateUnlock(state);return result;
}
static bool __xgePassStateMayInit(xge_pass_state_t* state,const xge_pass_t* pass)
{
    __xgePassStateLock(state);bool result=true;
    if(state->current && state->thread!=xrtThreadCurrentId())result=false;
    else for(xge_pass current=state->current;current;current=current->_pPrevious)
        if(current==pass){result=false;break;}
    __xgePassStateUnlock(state);return result;
}
static bool __xgePassStateReserve(xge_pass_state_t* state,xge_pass pass)
{
    __xgePassStateLock(state);uint64_t thread=xrtThreadCurrentId();
    if((state->current && (state->thread!=thread || state->current->bActive!=XGE_PASS_ACTIVE)) || pass->bActive){
        __xgePassStateUnlock(state);return false;
    }
    for(xge_pass current=state->current;current;current=current->_pPrevious)
        if(current==pass){__xgePassStateUnlock(state);return false;}
    pass->_pPrevious=state->current;pass->bActive=XGE_PASS_STARTING;
    state->current=pass;state->thread=thread;__xgePassStateUnlock(state);return true;
}
static void __xgePassStateActivate(xge_pass_state_t* state,xge_pass pass)
{
    __xgePassStateLock(state);
    if(state->thread!=xrtThreadCurrentId() || state->current!=pass || pass->bActive!=XGE_PASS_STARTING)abort();
    pass->bActive=XGE_PASS_ACTIVE;__xgePassStateUnlock(state);
}
static void __xgePassStateAbort(xge_pass_state_t* state,xge_pass pass)
{
    __xgePassStateLock(state);
    if(state->thread!=xrtThreadCurrentId() || state->current!=pass || pass->bActive!=XGE_PASS_STARTING)abort();
    state->current=pass->_pPrevious;if(!state->current)state->thread=0;
    pass->_pPrevious=NULL;pass->bActive=0;__xgePassStateUnlock(state);
}
static bool __xgePassStateBeginEnd(xge_pass_state_t* state,xge_pass pass)
{
    __xgePassStateLock(state);
    if(state->thread!=xrtThreadCurrentId() || state->current!=pass || pass->bActive!=XGE_PASS_ACTIVE){
        __xgePassStateUnlock(state);return false;
    }
    pass->bActive=XGE_PASS_ENDING;__xgePassStateUnlock(state);return true;
}
static void __xgePassStateFinishEnd(xge_pass_state_t* state,xge_pass pass)
{
    __xgePassStateLock(state);
    if(state->thread!=xrtThreadCurrentId() || state->current!=pass || pass->bActive!=XGE_PASS_ENDING)abort();
    state->current=pass->_pPrevious;if(!state->current)state->thread=0;
    pass->_pPrevious=NULL;pass->bActive=0;__xgePassStateUnlock(state);
}
#endif
