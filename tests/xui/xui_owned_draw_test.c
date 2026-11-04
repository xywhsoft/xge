/* CPU provider: no XGE init, window, renderer, GPU or external I/O. */
#include "include/xui.h"
#include "src/xui/core/xui_proxy_resource.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct Test {size_t refs;int holds,drops,ended,targetFreed,sequence,endOrder,targetOrder;int failEnd;bool diagnostic,failDrop;xui_draw_context active;} Test;
typedef struct End {Test* test;xui_draw_context self;bool reenter,releaseOwner;} End;
static bool count(const void* raw,size_t* out){*out=((const Test*)raw)->refs;return true;}
static bool trace(const void* raw,xrtownershipvisitor visit,void* data){(void)raw;(void)visit;(void)data;return true;}
static const xrtownershipops ops={count,trace};
static bool hold(const void* raw){Test* t=(void*)raw;++t->refs;++t->holds;return true;}
static bool drop(const void* raw)
{Test* t=(void*)raw;assert(t->refs);--t->refs;++t->drops;if(t->failDrop){xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"backend-drop",-32,"secondary"));return false;}return true;}
static int surfaceEnd(xui_proxy proxy,void* raw)
{(void)proxy;Test* t=*(Test**)raw;++t->targetFreed;t->targetOrder=++t->sequence;assert(t->ended==1);return XUI_OK;}
static int drawEnd(xui_proxy proxy,void* raw)
{
    End* value=raw;Test* t=value->test;assert(proxy && t->active==value->self);
    ++t->ended;t->endOrder=++t->sequence;assert(!t->targetFreed);
    if(value->reenter){assert(xuiDrawEndChecked(value->self)==XUI_OK);assert(xuiDrawAddRef(value->self)==XUI_ERROR_INVALID_STATE);}
    if(value->releaseOwner){
        assert(xuiDrawReleaseChecked(value->self)==XUI_OK);
        xrtownershipref ref=xuiDrawOwnership(value->self);xrtownershipscope freeze={0};size_t refs=0;
        assert(xrtOwnershipFreezeTryBegin(&freeze) && ref.Ops->Count(ref.Data,&refs) && refs);
        assert(xrtOwnershipScopeEnd(&freeze));
    }
    t->active=NULL;
    if(t->diagnostic)xrtSetErrorTake(xrtErrorCreate(XERR_STATE,"end",t->failEnd,"end"));
    return t->failEnd;
}
static xui_proxy create(Test* test)
{
    xui_proxy_t table={0};table.iSize=sizeof(table);table.iVersion=XUI_PROXY_VERSION;
    table.pUser=test;xui_callback_owner_t owner={{test,&ops},hold,drop};xui_proxy result=NULL;
    assert(xuiProxyCreateOwned(&table,&owner,&result)==XUI_OK);return result;
}
static xui_surface target(xui_proxy proxy,Test* test)
{
    xui_surface result=NULL;void* payload=NULL;
    assert(xuiSurfaceCreateOwned(proxy,sizeof(Test*),surfaceEnd,&result,&payload)==XUI_OK);
    *(Test**)payload=test;return result;
}
static int begin(xui_proxy proxy,xui_surface surface,xui_draw_context* output)
{
    Test* test=proxy->pUser;if(test->active)return XUI_ERROR_BUSY;
    xui_draw_context result=NULL;void* payload=NULL;
    int status=xuiDrawCreateOwned(proxy,surface,sizeof(End),drawEnd,&result,&payload);
    if(status==XUI_OK){*(End*)payload=(End){test,result,true,false};test->active=result;*output=result;}
    return status;
}
static size_t edgeCount;
static bool edge(xrtownershipref ref,void* data){assert(ref.Data && ref.Ops);++*(size_t*)data;return true;}
static void traceEdges(xrtownershipref ref,size_t expected)
{
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    edgeCount=0;assert(ref.Ops->Trace(ref.Data,edge,&edgeCount));assert(edgeCount==expected);
    assert(xrtOwnershipScopeEnd(&freeze));
}
static void explicitEnd(void)
{
    Test test={.refs=1};xui_proxy proxy=create(&test);xui_surface surface=target(proxy,&test);
    xui_draw_context draw=NULL;assert(begin(proxy,surface,&draw)==XUI_OK);
    xui_draw_context unchanged=(void*)(uintptr_t)1;
    assert(xrtMemDebugFailAfter(0));assert(begin(proxy,surface,&unchanged)==XUI_ERROR_BUSY);
    assert(unchanged==(void*)(uintptr_t)1 && !xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    xrtownershipref ref=xuiDrawOwnership(draw);const xrtownershipadapterv1* adapter=xuiProxyResourceOwnershipAdapterV1(ref);
    const xrtownershipfinishv2* finish=xuiProxyResourceOwnershipCompletionV2(ref);
    assert(adapter && finish && finish->DropReady(ref.Data) && adapter->Hold(ref.Data));traceEdges(ref,2);
    assert(xuiSurfaceReleaseChecked(surface)==XUI_OK);assert(xuiProxyReleaseChecked(proxy)==XUI_OK);
    assert(xuiDrawEndChecked(draw)==XUI_OK && test.ended==1 && test.targetFreed==1);
    assert(test.endOrder<test.targetOrder && test.drops==1 && test.refs==1);
    assert(xuiDrawEnded(draw) && !xuiDrawGetProxy(draw) && !xuiDrawGetTarget(draw));traceEdges(ref,0);
    assert(xuiDrawEndChecked(draw)==XUI_OK && test.ended==1);
    assert(xuiDrawAddRef(draw)==XUI_ERROR_INVALID_STATE);
    /* A terminal header remains snapshot/cleanup eligible, never a new owner. */
    assert(adapter->Hold(ref.Data));assert(finish->DropChecked(ref.Data));
    assert(xuiDrawReleaseChecked(draw)==XUI_OK);assert(finish->DropChecked(ref.Data));
}
static void pendingEnd(bool releaseOwner)
{
    Test test={.refs=1};xui_proxy proxy=create(&test);xui_surface surface=target(proxy,&test);
    xui_draw_context draw=NULL;assert(begin(proxy,surface,&draw)==XUI_OK);
    xrtownershipref ref=xuiDrawOwnership(draw);const xrtownershipadapterv1* adapter=xuiProxyResourceOwnershipAdapterV1(ref);
    const xrtownershipfinishv2* finish=xuiProxyResourceOwnershipCompletionV2(ref);
    assert(adapter->Hold(ref.Data));xui_draw_scope_t outer={0},inner={0};
    assert(xuiDrawCallScopeBegin(draw,&outer)==XUI_OK && xuiDrawCallScopeBegin(draw,&inner)==XUI_OK);
    assert(xuiSurfaceReleaseChecked(surface)==XUI_OK && xuiProxyReleaseChecked(proxy)==XUI_OK);
    if(releaseOwner)assert(xuiDrawReleaseChecked(draw)==XUI_OK);else assert(xuiDrawEndChecked(draw)==XUI_OK);
    assert(!test.ended && !test.targetFreed);
    assert(xuiDrawCallScopeEnd(&inner)==XUI_OK && !test.ended);
    assert(xuiDrawCallScopeEnd(&outer)==XUI_OK && test.ended==1 && test.targetFreed==1 && test.drops==1);
    if(!releaseOwner)assert(xuiDrawReleaseChecked(draw)==XUI_OK);
    assert(finish->DropChecked(ref.Data));
}
static void endFailure(bool diagnostic,bool oom)
{
    Test test={.refs=1,.failEnd=-23,.diagnostic=diagnostic,.failDrop=true};xui_proxy proxy=create(&test);
    xui_surface surface=target(proxy,&test);xui_draw_context draw=NULL;assert(begin(proxy,surface,&draw)==XUI_OK);
    assert(xuiSurfaceReleaseChecked(surface)==XUI_OK && xuiProxyReleaseChecked(proxy)==XUI_OK);
    if(oom)assert(xrtMemDebugFailAfter(0));
    assert(xuiDrawEndChecked(draw)==-23 && test.ended==1 && test.targetFreed==1);
    if(diagnostic && !oom)assert(xrtGetError() && xrtErrorCode(xrtGetError())==-23);
    else if(!diagnostic)assert(!xrtGetError());
    if(oom)xrtMemDebugFailClear();
    xrtClearError();
    assert(xuiDrawEndChecked(draw)==XUI_OK && xuiDrawReleaseChecked(draw)==XUI_OK && test.ended==1);
}
static void prepareFirst(void)
{
    Test test={.refs=1};xui_proxy proxy=create(&test);xui_surface surface=target(proxy,&test);
    xui_draw_context draw=NULL;assert(begin(proxy,surface,&draw)==XUI_OK);
    xrtownershipref targetRef=xuiSurfaceOwnership(surface),drawRef=xuiDrawOwnership(draw);
    const xrtownershipadapterv1* a=xuiProxyResourceOwnershipAdapterV1(drawRef);
    const xrtownershipadapterv1* targetAdapter=xuiProxyResourceOwnershipAdapterV1(targetRef);
    const xrtownershippreparationv1* prep=xuiProxyResourceOwnershipPreparationV1(drawRef);
    const xrtownershipfinishv2* f=xuiProxyResourceOwnershipCompletionV2(drawRef),*targetFinish=xuiProxyResourceOwnershipCompletionV2(targetRef);
    assert(targetAdapter->Hold(targetRef.Data) && a->Hold(drawRef.Data));
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(targetAdapter->Claim(targetRef.Data,&test) && a->Claim(drawRef.Data,&test));
    assert(xrtOwnershipScopeEnd(&freeze));assert(!prep->Ready(drawRef.Data));
    assert(prep->Prepare(drawRef.Data,&test)==XRT_OWNERSHIP_PREPARE_READY && test.ended==1);
    assert(prep->Ready(drawRef.Data));
    /* Deliberately target first, after all preparation, like the real planner. */
    assert(targetAdapter->Finalize(targetRef.Data,&test) && a->Finalize(drawRef.Data,&test));
    assert(test.endOrder<test.targetOrder);
    assert(xuiDrawReleaseChecked(draw)==XUI_OK && xuiSurfaceReleaseChecked(surface)==XUI_OK);
    freeze=(xrtownershipscope){0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    targetAdapter->Clear(targetRef.Data,&test);a->Clear(drawRef.Data,&test);assert(xrtOwnershipScopeEnd(&freeze));
    assert(targetFinish->V1.Step(targetRef.Data,&test)==XRT_OWNERSHIP_FINISH_READY);
    assert(f->V1.Step(drawRef.Data,&test)==XRT_OWNERSHIP_FINISH_READY);
    assert(targetFinish->DropChecked(targetRef.Data) && f->DropChecked(drawRef.Data));
    assert(xuiProxyReleaseChecked(proxy)==XUI_OK && test.refs==1);
}
typedef struct WrongThread {xui_draw_context draw;const xrtownershipfinishv2* completion;bool ended;} WrongThread;
static int32 worker(ptr raw)
{
    WrongThread* value=raw;bool consumed=true;xui_draw_scope_t scope={0};
    assert(xrtMemDebugFailAfter(0));xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(value->completion->DropReady(value->draw)==value->ended);
    assert(xrtOwnershipScopeEnd(&freeze) && !xrtMemDebugFailTriggered());xrtMemDebugFailClear();
    if(value->ended)assert(value->completion->DropChecked(value->draw));
    else{
        assert(xuiDrawCallScopeBegin(value->draw,&scope)==XUI_ERROR_BUSY);
        assert(xuiDrawEndChecked(value->draw)==XUI_ERROR_BUSY);
        assert(xuiDrawReleaseOwnerChecked(value->draw,&consumed)==XUI_ERROR_BUSY && !consumed);
    }
    assert(!xrtGetError());return 0;
}
static void affinityAndReentry(void)
{
    Test test={.refs=1};xui_proxy proxy=create(&test);xui_surface surface=target(proxy,&test);
    xui_draw_context draw=NULL;assert(begin(proxy,surface,&draw)==XUI_OK);
    End* payload=xuiInternalProxyResourcePayload(draw);payload->releaseOwner=true;
    xrtownershipref ref=xuiDrawOwnership(draw);const xrtownershipadapterv1* adapter=xuiProxyResourceOwnershipAdapterV1(ref);
    assert(adapter->Hold(ref.Data));WrongThread attempt={draw,xuiProxyResourceOwnershipCompletionV2(ref),false};
    xthread* thread=xrtThreadCreate(worker,&attempt,0);assert(thread);
    assert(xrtThreadWaitFor(thread,10000)==XWAIT_OK);xrtThreadDestroy(thread);assert(!test.ended);
    assert(xuiSurfaceReleaseChecked(surface)==XUI_OK && xuiProxyReleaseChecked(proxy)==XUI_OK);
    /* End's own transient physical reference covers callback-close of its
     * final language/native owner. The external snapshot remains afterwards. */
    assert(xuiDrawEndChecked(draw)==XUI_OK && test.ended==1 && test.targetFreed==1);
    attempt.ended=true;thread=xrtThreadCreate(worker,&attempt,0);assert(thread);
    assert(xrtThreadWaitFor(thread,10000)==XWAIT_OK);xrtThreadDestroy(thread);assert(test.refs==1);
}
static void callbackConsumesOnlyOwner(void)
{
    Test test={.refs=1};xui_proxy proxy=create(&test);xui_surface surface=target(proxy,&test);
    xui_draw_context draw=NULL;assert(begin(proxy,surface,&draw)==XUI_OK);
    ((End*)xuiInternalProxyResourcePayload(draw))->releaseOwner=true;
    assert(xuiSurfaceReleaseChecked(surface)==XUI_OK && xuiProxyReleaseChecked(proxy)==XUI_OK);
    /* No snapshot hides a missing native-tail pin in this case. */
    assert(xuiDrawEndChecked(draw)==XUI_OK && test.ended==1 && test.targetFreed==1 && test.refs==1);
}
int main(void)
{
    explicitEnd();pendingEnd(false);pendingEnd(true);endFailure(false,false);endFailure(true,false);endFailure(true,true);prepareFirst();affinityAndReentry();callbackConsumesOnlyOwner();
    assert(!xrtGetError());printf("owned Draw CPU lifecycle passed; prefix=%zu, common=%zu\n",sizeof(xui_owned_draw_t),sizeof(xui_owned_resource_t));return 0;
}
