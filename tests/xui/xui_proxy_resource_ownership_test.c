/* Component-level native unit. Default XGE backend integration is a separate
 * matched-engine gate; this test initializes no device/window/backend. */
#include "include/xui.h"
#include "src/xui/core/xui_proxy_resource.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct Owner {size_t refs,holds,drops;int failHold,failDrop;bool diagnostic;} Owner;
typedef struct Payload {int* calls;int status;bool diagnostic,reenter;xui_font self;} Payload;
static bool count(const void* raw,size_t* out){*out=((const Owner*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* user){(void)raw;(void)visit;(void)user;return true;}
static const xrtownershipops ops={count,trace};
static bool hold(const void* raw)
{Owner* owner=(void*)raw;++owner->holds;if(owner->failHold){owner->failHold=0;return false;}++owner->refs;return true;}
static bool drop(const void* raw)
{
    Owner* owner=(void*)raw;assert(owner->refs);--owner->refs;++owner->drops;
    if(owner->failDrop){owner->failDrop=0;if(owner->diagnostic)xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"drop",-9,"drop"));return false;}
    return true;
}
static xui_callback_owner_t ownerOf(Owner* owner)
{return (xui_callback_owner_t){{owner,&ops},hold,drop};}
static int finalize(xui_proxy proxy,void* raw)
{
    Payload* payload=raw;assert(proxy && xuiProxyIsOwned(proxy));++*payload->calls;
    if(payload->reenter){
        assert(xuiFontAddRef(payload->self)==XUI_ERROR_INVALID_STATE);
        assert(!xuiInternalProxyResourceReady(payload->self,XUI_OWNED_FONT));
    }
    if(payload->diagnostic)xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"payload",payload->status,"payload"));
    return payload->status;
}
static xui_proxy createFlags(Owner* owner,uint32_t flags)
{
    xui_proxy_t table={0};table.iSize=sizeof(table);table.iVersion=XUI_PROXY_VERSION;table.pUser=owner;
    xui_callback_owner_t descriptor=ownerOf(owner);xui_proxy proxy=NULL;
    assert(xuiProxyCreateOwnedEx(&table,&descriptor,flags,&proxy)==XUI_OK && proxy && proxy->pUser==owner && owner->refs==2);
    return proxy;
}
static xui_proxy create(Owner* owner){return createFlags(owner,XUI_PROXY_OWNED_ANYTHREAD_FINALIZE);}
static xui_font font(xui_proxy proxy,int* calls,int status,bool diagnostic)
{
    xui_font value=NULL;void* raw=NULL;
    assert(xuiFontCreateOwned(proxy,sizeof(Payload),finalize,&value,&raw)==XUI_OK);
    assert((uintptr_t)raw%_Alignof(max_align_t)==0);
    *(Payload*)raw=(Payload){calls,status,diagnostic,true,value};return value;
}
static void releaseOrder(void)
{
    Owner owner={.refs=1};xui_proxy proxy=create(&owner);int calls=0;
    xui_font a=font(proxy,&calls,0,false);xui_surface b=NULL;void* raw=NULL;
    assert(xuiSurfaceCreateOwned(proxy,sizeof(Payload),finalize,&b,&raw)==XUI_OK);
    *(Payload*)raw=(Payload){&calls,0,false,false,NULL};
    assert(xuiFontAddRef(a)==XUI_OK);bool consumed=false;
    assert(xuiProxyReleaseOwnerChecked(proxy,&consumed)==XUI_OK && consumed && owner.refs==2);
    assert(xuiFontReleaseOwnerChecked(a,&consumed)==XUI_OK && consumed && calls==0);
    assert(xuiFontGetProxy(a)==xuiSurfaceGetProxy(b));
    assert(xuiFontReleaseOwnerChecked(a,&consumed)==XUI_OK && consumed && calls==1 && owner.refs==2);
    assert(xuiSurfaceReleaseOwnerChecked(b,&consumed)==XUI_OK && consumed && calls==2 && owner.refs==1 && owner.drops==1);
}
static void failures(void)
{
    Owner owner={.refs=1};xui_proxy proxy=create(&owner);int calls=0;bool consumed=false;
    xui_font value=font(proxy,&calls,-8,false);
    assert(xuiProxyReleaseOwnerChecked(proxy,&consumed)==XUI_OK && consumed);
    owner.failDrop=1;owner.diagnostic=true;
    assert(xuiFontReleaseOwnerChecked(value,&consumed)==-8 && consumed && !xrtGetError());
    assert(owner.refs==1 && calls==1);
    proxy=create(&owner);value=font(proxy,&calls,-7,true);
    assert(xuiProxyReleaseOwnerChecked(proxy,&consumed)==XUI_OK && consumed);
    owner.failDrop=1;
    assert(xuiFontReleaseOwnerChecked(value,&consumed)==-7 && consumed && xrtErrorCode(xrtGetError())==-7);xrtClearError();
    assert(owner.refs==1 && calls==2);
}
static void lifecycle(void)
{
    Owner owner={.refs=1};xui_proxy proxy=create(&owner);int calls=0;bool consumed=false;
    xui_font value=font(proxy,&calls,0,false);xrtownershipref ref=xuiFontOwnership(value);
    const xrtownershipadapterv1* adapter=xuiProxyResourceOwnershipAdapterV1(ref);
    const xrtownershippreparationv1* prepare=xuiProxyResourceOwnershipPreparationV1(ref);
    const xrtownershipfinishv1* completion=xuiProxyResourceOwnershipCompletionV1(ref);
    assert(adapter && prepare && completion && prepare->Adapter==adapter && completion->Adapter==adapter);
    assert(xuiInternalProxyResourcePin(value,XUI_OWNED_FONT)==XUI_OK && !prepare->Ready(ref.Data));
    assert(!adapter->Claim(ref.Data,&owner));
    assert(xuiInternalProxyResourceUnpin(value,XUI_OWNED_FONT)==XUI_OK && prepare->Ready(ref.Data));
    assert(adapter->Hold(ref.Data));
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(adapter->Claim(ref.Data,&owner));assert(xrtOwnershipScopeEnd(&freeze));
    assert(adapter->Finalize(ref.Data,&owner) && calls==1);
    assert(!xuiInternalProxyResourceReady(value,XUI_OWNED_FONT));
    assert(xuiFontReleaseOwnerChecked(value,&consumed)==XUI_OK && consumed);
    assert(xrtMemDebugFailAfter(0));freeze=(xrtownershipscope){0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    adapter->Clear(ref.Data,&owner);assert(xrtOwnershipScopeEnd(&freeze));
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    assert(completion->Step(ref.Data,&owner)==XRT_OWNERSHIP_FINISH_READY && calls==1);
    adapter->Drop(ref.Data);
    assert(xuiProxyReleaseOwnerChecked(proxy,&consumed)==XUI_OK && consumed && owner.refs==1);
}
static void oom(void)
{
    Owner owner={.refs=1};xui_proxy proxy=create(&owner);int calls=0;bool consumed=false;
    assert(xrtMemDebugFailAfter(0));xui_font value=(xui_font)(uintptr_t)1;void* payload=(void*)(uintptr_t)1;
    assert(xuiFontCreateOwned(proxy,sizeof(Payload),finalize,&value,&payload)==XUI_ERROR_OUT_OF_MEMORY);
    assert(value==(xui_font)(uintptr_t)1 && payload==(void*)(uintptr_t)1 && xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();xrtClearError();
    value=font(proxy,&calls,0,false);assert(xrtMemDebugFailAfter(0));
    assert(xuiFontAddRef(value)==XUI_OK && xuiFontReleaseOwnerChecked(value,&consumed)==XUI_OK && consumed);
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    assert(xuiFontReleaseOwnerChecked(value,&consumed)==XUI_OK && consumed);
    assert(xuiProxyReleaseOwnerChecked(proxy,&consumed)==XUI_OK && consumed && owner.refs==1);
}
static void failedFinalizeAbort(void)
{
    Owner owner={.refs=1};xui_proxy proxy=create(&owner);int calls=0;bool consumed=false;
    xui_font value=font(proxy,&calls,-8,false);xrtownershipref ref=xuiFontOwnership(value);
    const xrtownershipadapterv1* adapter=xuiProxyResourceOwnershipAdapterV1(ref);
    assert(adapter && adapter->Hold(ref.Data));
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(adapter->Claim(ref.Data,&owner));assert(xrtOwnershipScopeEnd(&freeze));
    assert(!adapter->Finalize(ref.Data,&owner) && calls==1 && xrtErrorCode(xrtGetError())==-8);
    xrtClearError();
    assert(!xuiInternalProxyResourceReady(value,XUI_OWNED_FONT));
    assert(xuiFontAddRef(value)==XUI_ERROR_INVALID_STATE);
    /* Abort restores admission bookkeeping, never an already consumed
     * backend payload. Its semantic finalizer is not repeated by last Drop. */
    freeze=(xrtownershipscope){0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    adapter->Restore(ref.Data,&owner);assert(xrtOwnershipScopeEnd(&freeze));
    assert(xuiFontReleaseOwnerChecked(value,&consumed)==XUI_OK && consumed);
    adapter->Drop(ref.Data);assert(calls==1);
    assert(xuiProxyReleaseOwnerChecked(proxy,&consumed)==XUI_OK && consumed && owner.refs==1);
}
static void checkedSnapshotDrop(void)
{
    Owner owner={.refs=1};xui_proxy proxy=create(&owner);int calls=0;bool consumed=false;
    xui_font value=font(proxy,&calls,-8,false);xrtownershipref ref=xuiFontOwnership(value);
    const xrtownershipadapterv1* adapter=xuiProxyResourceOwnershipAdapterV1(ref);
    const xrtownershipfinishv2* completion=xuiProxyResourceOwnershipCompletionV2(ref);
    assert(completion && completion->V1.size==sizeof(*completion) && completion->AbiTag==XRT_OWNERSHIP_FINISH_V2_TAG && completion->V1.Adapter==adapter && completion->DropChecked && completion->DropReady && completion->DropReady(ref.Data));
    assert(xuiProxyResourceOwnershipCompletionV1(ref)->size==sizeof(xrtownershipfinishv1));
    assert(adapter->Hold(ref.Data));
    assert(xuiProxyReleaseOwnerChecked(proxy,&consumed)==XUI_OK && consumed);
    assert(xuiFontReleaseOwnerChecked(value,&consumed)==XUI_OK && consumed && calls==0);
    /* No semantic duty was claimed/committed. The final physical snapshot
     * reference must still report a cleanup failure after consuming itself. */
    assert(xrtMemDebugFailAfter(0));
    assert(!completion->DropChecked(ref.Data));
    assert(xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtClearError();
    assert(calls==1 && owner.refs==1 && owner.drops==1);
}
typedef struct Concurrent {xui_proxy proxy;xui_font font;xatomic32 go;} Concurrent;
static int32 worker(ptr raw)
{
    Concurrent* work=raw;
    while(!xrtAtomic32Load(&work->go,XMEMORY_ACQUIRE))xrtThreadYield();
    for(unsigned i=0;i<4000;++i){
        assert(xuiFontAddRef(work->font)==XUI_OK);
        assert(xuiProxyAddRef(work->proxy)==XUI_OK);
        assert(xuiInternalProxyResourcePin(work->font,XUI_OWNED_FONT)==XUI_OK);
        assert(xuiProxyResourceOwnershipBusy(xuiFontOwnership(work->font)));
        assert(xuiInternalProxyResourceUnpin(work->font,XUI_OWNED_FONT)==XUI_OK);
        assert(xuiFontReleaseChecked(work->font)==XUI_OK);
        assert(xuiProxyReleaseChecked(work->proxy)==XUI_OK);
    }
    assert(xuiFontReleaseChecked(work->font)==XUI_OK);
    assert(xuiProxyReleaseChecked(work->proxy)==XUI_OK);return 0;
}
static void concurrentReferences(void)
{
    Owner owner={.refs=1};int calls=0;Concurrent work={.proxy=create(&owner)};
    work.font=font(work.proxy,&calls,0,false);xrtAtomic32Init(&work.go,0);
    xthread* threads[4];
    for(unsigned i=0;i<4;++i){
        assert(xuiFontAddRef(work.font)==XUI_OK && xuiProxyAddRef(work.proxy)==XUI_OK);
        threads[i]=xrtThreadCreate(worker,&work,0);assert(threads[i]);
    }
    assert(xuiFontReleaseChecked(work.font)==XUI_OK && xuiProxyReleaseChecked(work.proxy)==XUI_OK);
    xrtAtomic32Store(&work.go,1,XMEMORY_RELEASE);
    for(unsigned i=0;i<4;++i){assert(xrtThreadWaitFor(threads[i],10000)==XWAIT_OK && xrtThreadExitCode(threads[i])==0);xrtThreadDestroy(threads[i]);}
    assert(calls==1 && owner.refs==1 && owner.drops==1);
}
typedef struct Finalizing {xui_font font;const xrtownershipadapterv1* adapter;xatomic32 entered,proceed;unsigned calls;} Finalizing;
static int concurrentFinalize(xui_proxy proxy,void* raw)
{
    Finalizing* state=*(Finalizing**)raw;(void)proxy;++state->calls;
    xrtAtomic32Store(&state->entered,1,XMEMORY_RELEASE);
    while(!xrtAtomic32Load(&state->proceed,XMEMORY_ACQUIRE))xrtThreadYield();
    return XUI_OK;
}
static int32 finalizeWorker(ptr raw)
{Finalizing* state=raw;assert(state->adapter->Finalize(state->font,state));return 0;}
static void concurrentFinalization(void)
{
    Owner owner={.refs=1};xui_proxy proxy=create(&owner);Finalizing state={0};void* payload=NULL;
    xrtAtomic32Init(&state.entered,0);xrtAtomic32Init(&state.proceed,0);
    assert(xuiFontCreateOwned(proxy,sizeof(Finalizing*),concurrentFinalize,&state.font,&payload)==XUI_OK);
    *(Finalizing**)payload=&state;state.adapter=xuiProxyResourceOwnershipAdapterV1(xuiFontOwnership(state.font));
    assert(state.adapter && state.adapter->Hold(state.font));
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(state.adapter->Claim(state.font,&state));assert(xrtOwnershipScopeEnd(&freeze));
    xthread* thread=xrtThreadCreate(finalizeWorker,&state,0);assert(thread);
    while(!xrtAtomic32Load(&state.entered,XMEMORY_ACQUIRE))xrtThreadYield();
    assert(xuiProxyResourceOwnershipBusy(xuiFontOwnership(state.font)));
    assert(!state.adapter->Finalize(state.font,&state));
    assert(xrtErrorCode(xrtGetError())==XUI_ERROR_BUSY);xrtClearError();
    xrtAtomic32Store(&state.proceed,1,XMEMORY_RELEASE);
    assert(xrtThreadWaitFor(thread,10000)==XWAIT_OK);xrtThreadDestroy(thread);
    freeze=(xrtownershipscope){0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    state.adapter->Restore(state.font,&state);assert(xrtOwnershipScopeEnd(&freeze));
    assert(xuiFontReleaseChecked(state.font)==XUI_OK);state.adapter->Drop(state.font);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && state.calls==1 && owner.refs==1 && owner.drops==1);
}
typedef struct AffinityAttempt {xui_font font;const xrtownershipfinishv2* completion;bool consumed,ready;int status;bool pin;} AffinityAttempt;
typedef struct FinishAttempt {AffinityAttempt* attempt;const void* token;xrtownershipfinishresult result;} FinishAttempt;
static int32 finishWrongThread(ptr raw)
{
    FinishAttempt* attempt=raw;assert(!xrtGetError());
    attempt->result=attempt->attempt->completion->V1.Step(attempt->attempt->font,attempt->token);
    assert(attempt->result==XRT_OWNERSHIP_FINISH_BUSY && !xrtGetError());return 0;
}
static int32 affinityWorker(ptr raw)
{
    AffinityAttempt* attempt=raw;xrtownershipscope freeze={0};
    xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"sentinel",-31,"unchanged query diagnostic"));
    assert(xrtGetError() && xrtOwnershipFreezeTryBegin(&freeze));assert(xrtMemDebugFailAfter(0));
    const xerror* prior=xrtGetError();attempt->ready=attempt->completion->DropReady(attempt->font);assert(xrtGetError()==prior);
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();assert(xrtOwnershipScopeEnd(&freeze));xrtClearError();
    attempt->status=attempt->pin ? xuiInternalProxyResourceUnpin(attempt->font,XUI_OWNED_FONT):
        xuiFontReleaseOwnerChecked(attempt->font,&attempt->consumed);return 0;
}
static int32 headerDropWorker(ptr raw)
{
    AffinityAttempt* attempt=raw;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(attempt->completion->DropReady(attempt->font));assert(xrtOwnershipScopeEnd(&freeze));
    assert(attempt->completion->DropChecked(attempt->font));return 0;
}
static void creatorAffinity(bool pin)
{
    Owner owner={.refs=1};int calls=0;xui_proxy proxy=createFlags(&owner,0);xui_font value=font(proxy,&calls,0,false);
    AffinityAttempt attempt={.font=value,.completion=xuiProxyResourceOwnershipCompletionV2(xuiFontOwnership(value)),.pin=pin};
    if(pin){assert(xuiInternalProxyResourcePin(value,XUI_OWNED_FONT)==XUI_OK);assert(xuiFontReleaseChecked(value)==XUI_OK);}
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK);
    xthread* thread=xrtThreadCreate(affinityWorker,&attempt,0);assert(thread);
    assert(xrtThreadWaitFor(thread,10000)==XWAIT_OK);xrtThreadDestroy(thread);
    assert(!attempt.ready && attempt.status==XUI_ERROR_BUSY && !attempt.consumed && calls==0);
    if(pin)assert(xuiInternalProxyResourceUnpin(value,XUI_OWNED_FONT)==XUI_OK);
    else assert(xuiFontReleaseChecked(value)==XUI_OK);
    assert(calls==1 && owner.refs==1 && owner.drops==1);
}
static void finalizedHeaderAffinity(void)
{
    Owner owner={.refs=1};int calls=0;xui_proxy proxy=createFlags(&owner,0);xui_font value=font(proxy,&calls,0,false);
    xrtownershipref ref=xuiFontOwnership(value);const xrtownershipadapterv1* adapter=xuiProxyResourceOwnershipAdapterV1(ref);
    AffinityAttempt attempt={.font=value,.completion=xuiProxyResourceOwnershipCompletionV2(ref)};
    assert(adapter->Hold(ref.Data));xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(adapter->Claim(ref.Data,&owner));assert(xrtOwnershipScopeEnd(&freeze));
    assert(adapter->Finalize(ref.Data,&owner));assert(xuiFontReleaseChecked(value)==XUI_OK);
    freeze=(xrtownershipscope){0};assert(xrtOwnershipFreezeTryBegin(&freeze));adapter->Clear(ref.Data,&owner);assert(xrtOwnershipScopeEnd(&freeze));
    FinishAttempt finishAttempt={&attempt,&owner,XRT_OWNERSHIP_FINISH_FAILED};
    xthread* workerThread=xrtThreadCreate(finishWrongThread,&finishAttempt,0);assert(workerThread);
    assert(xrtThreadWaitFor(workerThread,10000)==XWAIT_OK);xrtThreadDestroy(workerThread);
    assert(finishAttempt.result==XRT_OWNERSHIP_FINISH_BUSY);
    assert(attempt.completion->V1.Step(ref.Data,&owner)==XRT_OWNERSHIP_FINISH_READY);
    xthread* thread=xrtThreadCreate(headerDropWorker,&attempt,0);assert(thread);
    assert(xrtThreadWaitFor(thread,10000)==XWAIT_OK);xrtThreadDestroy(thread);
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && calls==1 && owner.refs==1 && owner.drops==1);
}
int main(void)
{
    releaseOrder();failures();lifecycle();oom();failedFinalizeAbort();checkedSnapshotDrop();concurrentReferences();concurrentFinalization();creatorAffinity(false);creatorAffinity(true);finalizedHeaderAffinity();
    printf("native storage bytes: header=%zu proxyTable=%zu owner=%zu\n",sizeof(xui_owned_resource_t),sizeof(xui_proxy_t),sizeof(xui_callback_owner_t));
    puts("native proxy/resource storage: shared owners, NULL first failure, at-most-once Finalize, freeze-only Clear, physical pins and OOM: PASS");return 0;
}
