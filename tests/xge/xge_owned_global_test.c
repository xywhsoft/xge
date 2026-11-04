/* CPU-only global publishers. Never initialize a window, renderer or device. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct Owner {
    xatomic32 refs,calls;
    int failHold,failDrop,diagnostic,reenter;
} Owner;
static bool count(const void* raw,size_t* output)
{*output=xrtAtomic32Load(&((Owner*)raw)->refs,XMEMORY_RELAXED);return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user)
{(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ownerOps={count,trace};
static void error_callback(const xge_error_info_t*,void*);
static int quit_callback(void*);
static uint32_t drag_callback(const xge_drag_event_t*,void*);
static bool hold(const void* raw)
{
    Owner* owner=(Owner*)raw;
    if(owner->failHold){--owner->failHold;return false;}
    (void)xrtAtomic32FetchAdd(&owner->refs,1,XMEMORY_RELAXED);
    if(owner->reenter==2)assert(xgeUnitChecked()==XGE_ERROR_INVALID_STATE);
    if(owner->reenter==4)assert(xgeSetQuitRequestCallbackOwned(NULL,NULL,NULL)==XGE_OK);
    return true;
}
static bool drop(const void* raw)
{
    Owner* owner=(Owner*)raw;
    assert(xrtAtomic32FetchSub(&owner->refs,1,XMEMORY_RELAXED)>1);
    if(owner->reenter==1){
        assert(xgeUnitChecked()==XGE_ERROR_INVALID_STATE);
        assert(xgeInit(NULL)==XGE_ERROR_INVALID_STATE);
        assert(xgeSetQuitRequestCallbackOwned(quit_callback,owner,NULL)==XGE_ERROR_INVALID_STATE);
        assert(xgeDragEventCallbackSetOwned(drag_callback,owner,NULL)==XGE_ERROR_INVALID_STATE);
    }
    if(owner->failDrop){
        --owner->failDrop;
        if(owner->diagnostic)xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"later-drop",17,"later cleanup must not replace NULL first failure"));
        return false;
    }
    return true;
}
static xge_callback_owner_t descriptor(Owner* value)
{return (xge_callback_owner_t){{value,&ownerOps},hold,drop};}
static void initialize(Owner* value)
{memset(value,0,sizeof(*value));xrtAtomic32Init(&value->refs,1);xrtAtomic32Init(&value->calls,0);}
static void error_callback(const xge_error_info_t* error,void* user)
{
    Owner* owner=user;assert(error->iCode==37);
    (void)xrtAtomic32FetchAdd(&owner->calls,1,XMEMORY_RELAXED);
    if(owner->reenter==3){
        assert(xgeSetErrorCallbackOwned(NULL,NULL,NULL)==XGE_OK);
        assert(xrtAtomic32Load(&owner->refs,XMEMORY_RELAXED)==2); /* external + snapshot */
        assert(error->iCode==37);assert(xgeUnitChecked()==XGE_ERROR_INVALID_STATE);
    }
}
static int quit_callback(void* user)
{
    Owner* owner=user;(void)xrtAtomic32FetchAdd(&owner->calls,1,XMEMORY_RELAXED);
    if(owner->reenter==3){
        int nested=99;assert(xgeQuitRequestDispatch(&nested)==XGE_OK && nested==0);
        assert(xgeSetQuitRequestCallbackOwned(NULL,NULL,NULL)==XGE_OK);
        assert(xrtAtomic32Load(&owner->refs,XMEMORY_RELAXED)==2);
        assert(xgeUnitChecked()==XGE_ERROR_INVALID_STATE);
    }
    return 1;
}
static uint32_t drag_callback(const xge_drag_event_t* event,void* user)
{
    Owner* owner=user;(void)xrtAtomic32FetchAdd(&owner->calls,1,XMEMORY_RELAXED);
    if(owner->reenter==3){
        assert(xgeDragEventCallbackSetOwned(NULL,NULL,NULL)==XGE_OK);
        assert(xrtAtomic32Load(&owner->refs,XMEMORY_RELAXED)==2);
        assert(xgeUnitChecked()==XGE_ERROR_INVALID_STATE);
        assert(event->iType==XGE_DRAG_EVENT_ENTER);
    }
    return XGE_DRAG_EFFECT_COPY;
}
static xge_error_info_t error={sizeof(error),37,1,"test","owned","event"};
static xge_drag_event_t event;
static int32 report_thread(void* unused)
{(void)unused;for(unsigned i=0;i<2000;++i)assert(xgeReportError(&error)==XGE_OK);return 0;}
typedef struct Symbol {const char* name;xge_host_xrt_proc address;} Symbol;
static const Symbol symbols[]={
#define XGE_HOST_XRT_SLOT(Name) {#Name,(xge_host_xrt_proc)(Name)},
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
};
static xge_host_xrt_proc resolve(void* unused,const char* name)
{
    (void)unused;size_t low=0,high=sizeof(symbols)/sizeof(*symbols);
    while(low<high){size_t middle=low+(high-low)/2;int order=strcmp(name,symbols[middle].name);
        if(order<0)high=middle;else if(order>0)low=middle+1;else return symbols[middle].address;}
    return NULL;
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;const char* missing=NULL;
    assert(xgeHostXrtBind(&contract,resolve,NULL,&missing)==XGE_HOST_XRT_OK);
    memset(&event,0,sizeof(event));event.iSize=sizeof(event);event.iType=XGE_DRAG_EVENT_ENTER;
    event.iAllowedEffects=XGE_DRAG_EFFECT_COPY;
    Owner a,b,c;initialize(&a);initialize(&b);initialize(&c);
    xge_callback_owner_t ao=descriptor(&a),bo=descriptor(&b),co=descriptor(&c);
    /* Failed admission and allocation preserve the old raw slot. */
    assert(xgeSetErrorCallback(error_callback,&a)==XGE_OK);
    b.failHold=1;assert(xgeSetErrorCallbackOwned(error_callback,&b,&bo)==XGE_ERROR_INVALID_STATE);
    assert(xrtAtomic32Load(&b.refs,XMEMORY_RELAXED)==1);
    assert(xgeReportError(&error)==XGE_OK && xrtAtomic32Load(&a.calls,XMEMORY_RELAXED)==1);
    assert(xrtMemDebugFailAfter(0));
    assert(xgeSetErrorCallbackOwned(error_callback,&b,&bo)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtClearError();
    assert(xrtAtomic32Load(&b.refs,XMEMORY_RELAXED)==1);
    assert(xgeReportError(&error)==XGE_OK && xrtAtomic32Load(&a.calls,XMEMORY_RELAXED)==2);
    assert(xgeSetErrorCallback(NULL,NULL)==XGE_OK);
    /* Each real callback can clear its registration; snapshot owns the tail. */
    a.reenter=3;assert(xgeSetErrorCallbackOwned(error_callback,&a,&ao)==XGE_OK);
    assert(xgeReportError(&error)==XGE_OK && xrtAtomic32Load(&a.refs,XMEMORY_RELAXED)==1);
    b.reenter=3;int allowed=77;
    assert(xgeSetQuitRequestCallbackOwned(quit_callback,&b,&bo)==XGE_OK);
    assert(xgeQuitRequestDispatch(&allowed)==XGE_OK && allowed==1);
    assert(xrtAtomic32Load(&b.refs,XMEMORY_RELAXED)==1);
    c.reenter=3;uint32_t effect=77;
    assert(xgeDragEventCallbackSetOwned(drag_callback,&c,&co)==XGE_OK);
    assert(xgeDragEventDispatchChecked(&event,&effect)==XGE_OK && effect==XGE_DRAG_EFFECT_COPY);
    assert(xrtAtomic32Load(&c.refs,XMEMORY_RELAXED)==1);
    /* Snapshot Hold refusal leaves output untouched, without invoking code. */
    b.reenter=0;assert(xgeSetQuitRequestCallbackOwned(quit_callback,&b,&bo)==XGE_OK);
    b.failHold=1;allowed=77;assert(xgeQuitRequestDispatch(&allowed)==XGE_ERROR_INVALID_STATE && allowed==77);
    assert(xgeSetQuitRequestCallbackOwned(NULL,NULL,NULL)==XGE_OK);
    /* Hold reentry retires the registration while a reader pins its node.
     * Its last-reader Drop fails before invocation; the new snapshot must
     * still be released and the output/callback count left unchanged. */
    b.reenter=4;assert(xgeSetQuitRequestCallbackOwned(quit_callback,&b,&bo)==XGE_OK);
    uint32 before=xrtAtomic32Load(&b.calls,XMEMORY_RELAXED);b.failDrop=1;allowed=77;
    assert(xgeQuitRequestDispatch(&allowed)==XGE_ERROR_INVALID_STATE && allowed==77);
    assert(xrtAtomic32Load(&b.calls,XMEMORY_RELAXED)==before && xrtAtomic32Load(&b.refs,XMEMORY_RELAXED)==1);
    b.reenter=0;
    /* Unit clears pre-init registrations and preserves NULL first failure.
     * Drop reentry cannot initialize or republish half-torn-down engine state. */
    a.reenter=0;b.reenter=1;c.reenter=1;b.failDrop=1;c.failDrop=1;c.diagnostic=1;
    assert(xgeSetErrorCallbackOwned(error_callback,&a,&ao)==XGE_OK);
    assert(xgeSetQuitRequestCallbackOwned(quit_callback,&b,&bo)==XGE_OK);
    assert(xgeDragEventCallbackSetOwned(drag_callback,&c,&co)==XGE_OK);
    assert(xgeUnitChecked()==XGE_ERROR_INVALID_STATE && !xrtGetError());
    assert(xrtAtomic32Load(&b.refs,XMEMORY_RELAXED)==1 && xrtAtomic32Load(&c.refs,XMEMORY_RELAXED)==1);
    assert(xgeReportError(&error)==XGE_OK); /* Error remains for shutdown logging. */
    assert(xgeSetErrorCallbackOwned(NULL,NULL,NULL)==XGE_OK);
    /* Warm native dispatch itself performs no XRT allocation. */
    assert(xgeSetErrorCallbackOwned(error_callback,&a,&ao)==XGE_OK);
    assert(xrtMemDebugFailAfter(0));
    for(unsigned i=0;i<100;++i)assert(xgeReportError(&error)==XGE_OK);
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    assert(xgeSetErrorCallbackOwned(NULL,NULL,NULL)==XGE_OK);
    /* Cold Hold itself may reenter, but cannot tear down an active admission. */
    b.reenter=2;assert(xgeSetQuitRequestCallbackOwned(quit_callback,&b,&bo)==XGE_OK);
    assert(xgeSetQuitRequestCallbackOwned(NULL,NULL,NULL)==XGE_OK);b.reenter=0;c.reenter=0;
    /* Concurrent error dispatch and replacement stress reader node pins. */
    assert(xgeSetErrorCallbackOwned(error_callback,&a,&ao)==XGE_OK);
    xthread* worker=xrtThreadCreate(report_thread,NULL,0);assert(worker);
    for(unsigned i=0;i<2000;++i)assert(xgeSetErrorCallbackOwned(error_callback,i&1?&a:&b,i&1?&ao:&bo)==XGE_OK);
    assert(xrtThreadWait(worker)==XWAIT_OK);xrtThreadDestroy(worker);
    assert(xgeSetErrorCallbackOwned(NULL,NULL,NULL)==XGE_OK);
    assert(xrtAtomic32Load(&a.refs,XMEMORY_RELAXED)==1 && xrtAtomic32Load(&b.refs,XMEMORY_RELAXED)==1 && xrtAtomic32Load(&c.refs,XMEMORY_RELAXED)==1);
    assert(xgeUnitChecked()==XGE_OK && !xrtGetError());
    puts("XGE global Owned: admission/OOM, self-clear snapshots, checked outputs, NULL-first shutdown, reentry and concurrent replacement passed");
    return 0;
}
