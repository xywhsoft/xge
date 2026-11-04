/* CPU-only retained frame publishing; no window, render or audio device. */
#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include <xge.h>
#include <xge_host_xrt.h>
#include <xge_host_xrt_contract.h>
#include "src/xge/core/xge_callback_owned.h" /* Actual first-outcome helpers. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct Owner {xatomic32 refs,calls;int value,failHold,failDrop,diagnostic,clearOnHold,clearOnCall,unitOnCall;} Owner;
static bool count(const void* raw,size_t* out){*out=xrtAtomic32Load(&((const Owner*)raw)->refs,XMEMORY_RELAXED);return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static bool hold(const void* raw)
{
    Owner* owner=(Owner*)raw;if(owner->failHold){--owner->failHold;return false;}
    (void)xrtAtomic32FetchAdd(&owner->refs,1,XMEMORY_RELAXED);
    if(owner->clearOnHold){owner->clearOnHold=0;assert(xgeSetFrameCallbackOwned(NULL,NULL,NULL)==XGE_OK);}
    return true;
}
static bool drop(const void* raw)
{
    Owner* owner=(Owner*)raw;assert(xrtAtomic32FetchSub(&owner->refs,1,XMEMORY_RELAXED)>1);
    if(owner->failDrop){--owner->failDrop;if(owner->diagnostic)xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"later frame drop",9,"later"));return false;}
    return true;
}
static int frame(void* raw)
{
    Owner* owner=raw;(void)xrtAtomic32FetchAdd(&owner->calls,1,XMEMORY_RELAXED);
    if(owner->unitOnCall){
        assert(xgeUnitChecked()==XGE_ERROR_INVALID_STATE);
        int unchanged=123;assert(xgeFrameCallbackDispatchChecked(&unchanged)==XGE_ERROR_INVALID_STATE && unchanged==123);
        xgeQuit();
    }
    if(owner->clearOnCall){assert(xgeSetFrameCallbackOwned(NULL,NULL,NULL)==XGE_OK);assert(xrtAtomic32Load(&owner->refs,XMEMORY_RELAXED)==2);}
    return owner->value;
}
static void init(Owner* owner){memset(owner,0,sizeof(*owner));xrtAtomic32Init(&owner->refs,1);xrtAtomic32Init(&owner->calls,0);}
static xge_callback_owner_t descriptor(Owner* owner){return (xge_callback_owner_t){{owner,&ops},hold,drop};}
static xge_host_xrt_proc resolve(void* ignored,const char* name)
{
    (void)ignored;
#define XGE_HOST_XRT_SLOT(Symbol) if(!strcmp(name,#Symbol))return (xge_host_xrt_proc)Symbol;
#include <xge_host_xrt_slots.inc>
#undef XGE_HOST_XRT_SLOT
    return NULL;
}
static void outcome(void)
{
    xge_callback_run_outcome_t result={0};
    __xgeCallbackRunRemember(&result,3);assert(__xgeCallbackRunTake(&result)==0);
    assert(xrtMemDebugFailAfter(0));__xgeCallbackRunRemember(&result,-7);
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    xerror* later=xrtErrorCreate(XERR_STATE,"later",9,"later");assert(later);xrtSetErrorTake(later);
    __xgeCallbackRunRemember(&result,-9);assert(__xgeCallbackRunTake(&result)==-7 && !xrtGetError());
    xerror* first=xrtErrorCreate(XERR_STATE,"first",7,"first");assert(first);xrtSetErrorTake(first);
    __xgeCallbackRunRemember(&result,-7);xrtClearError();
    later=xrtErrorCreate(XERR_STATE,"later",9,"later");assert(later);xrtSetErrorTake(later);
    __xgeCallbackRunRemember(&result,-9);assert(__xgeCallbackRunTake(&result)==-7 && xrtGetError()==first);xrtClearError();
}
int main(void)
{
    const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;const char* missing=NULL;
    assert(xgeHostXrtBind(&contract,resolve,NULL,&missing)==XGE_HOST_XRT_OK);
    Owner a,b;init(&a);init(&b);xge_callback_owner_t ao=descriptor(&a),bo=descriptor(&b);
    int output=99;assert(xgeFrameCallbackDispatchChecked(&output)==XGE_OK && output==0);
    assert(xgeRunOwned(frame,&a,&ao)==XGE_ERROR_NOT_INITIALIZED && xrtAtomic32Load(&a.refs,XMEMORY_RELAXED)==1);
    assert(xgeSetFrameCallbackOwned(frame,&a,NULL)==XGE_OK);
    assert(xrtMemDebugFailAfter(0));assert(xgeSetFrameCallbackOwned(frame,&b,&bo)==XGE_ERROR_OUT_OF_MEMORY);
    assert(xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtClearError();
    assert(xrtAtomic32Load(&b.refs,XMEMORY_RELAXED)==1);
    assert(xgeFrameCallbackDispatchChecked(&output)==XGE_OK && xrtAtomic32Load(&a.calls,XMEMORY_RELAXED)==1);
    /* Raw slots do not allocate an owner node, but their active dispatch
     * still excludes teardown of the engine underneath the callback. */
    a.unitOnCall=1;assert(xgeFrameCallbackDispatchChecked(&output)==XGE_OK);a.unitOnCall=0;
    assert(xgeSetFrameCallbackOwned(frame,&a,&ao)==XGE_OK);
    assert(xrtMemDebugFailAfter(0));for(int i=0;i<100;++i)assert(xgeFrameCallbackDispatchChecked(&output)==XGE_OK);
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    a.value=5;assert(xgeFrameCallbackDispatchChecked(&output)==XGE_OK && output==5);
    a.value=-7;a.failDrop=1;a.diagnostic=1;output=99;
    assert(xgeFrameCallbackDispatchChecked(&output)==-7 && output==99 && !xrtGetError());
    a.value=0;a.failHold=1;assert(xgeFrameCallbackDispatchChecked(&output)==XGE_ERROR_INVALID_STATE && output==99);
    a.unitOnCall=1;assert(xgeFrameCallbackDispatchChecked(&output)==XGE_OK && output==0);a.unitOnCall=0;
    a.clearOnCall=1;assert(xgeFrameCallbackDispatchChecked(&output)==XGE_OK && xrtAtomic32Load(&a.refs,XMEMORY_RELAXED)==1);
    a.clearOnCall=0;assert(xgeSetFrameCallbackOwned(frame,&a,&ao)==XGE_OK);
    a.clearOnHold=1;a.failDrop=1;a.diagnostic=0;unsigned calls=xrtAtomic32Load(&a.calls,XMEMORY_RELAXED);output=99;
    assert(xgeFrameCallbackDispatchChecked(&output)==XGE_ERROR_INVALID_STATE && output==99);
    assert(xrtAtomic32Load(&a.calls,XMEMORY_RELAXED)==calls && xrtAtomic32Load(&a.refs,XMEMORY_RELAXED)==1);
    assert(xgeSetFrameCallbackOwned(frame,&a,&ao)==XGE_OK);a.failDrop=1;
    assert(xgeUnitChecked()==XGE_ERROR_INVALID_STATE && !xrtGetError());
    assert(xrtAtomic32Load(&a.refs,XMEMORY_RELAXED)==1 && xgeUnitChecked()==XGE_OK);
    outcome();puts("Owned Frame: retention/OOM/rebind/physical snapshot/busy Unit/checked payload/first outcome: PASS");return 0;
}
