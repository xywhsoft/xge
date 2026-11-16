#define XRT_MODULE_ALL
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static xui_test_proxy_state_t proxy;
static size_t refs(xui_flow_graph graph)
{
    xrtownershipref ref=xuiFlowGraphOwnership(graph);size_t count=0;
    xrtownershipscope freeze={0};assert(ref.Data && xrtOwnershipFreezeTryBegin(&freeze));
    assert(ref.Ops->Count(ref.Data,&count));assert(xrtOwnershipScopeEnd(&freeze));return count;
}
static xui_context context(void)
{
    xui_context ctx=NULL;assert(xuiCreate(&ctx)==XUI_OK);
    assert(xuiSetProxy(ctx,&proxy.tProxy)==XUI_OK);return ctx;
}
static xui_widget canvas(xui_context ctx)
{xui_widget widget=NULL;assert(xuiFlowGraphWidgetCreate(ctx,&widget,NULL)==XUI_OK);return widget;}
static void node(xui_flow_graph graph)
{xui_flow_node_desc_t desc={0};desc.iSize=sizeof(desc);desc.sId="one";assert(xuiFlowGraphAddNode(graph,&desc,NULL)==XUI_OK);}
static xrtownershipref expected;
static unsigned matches;
static bool edge(xrtownershipref ref,void* user)
{(void)user;if(ref.Data==expected.Data && ref.Ops==expected.Ops)++matches;return true;}
static void trace(xui_context ctx,xui_flow_graph graph,unsigned count)
{
    expected=xuiFlowGraphOwnership(graph);matches=0;xrtownershipscope freeze={0};
    assert(xrtOwnershipFreezeTryBegin(&freeze) && xuiContextTraceOwnership(ctx,edge,NULL));
    assert(xrtOwnershipScopeEnd(&freeze) && matches==count);
}
static void aliases(void)
{
    xui_context ctx=context();xui_widget first=canvas(ctx),second=canvas(ctx);
    xui_flow_graph graph=(xui_flow_graph)(uintptr_t)1,raw=xuiFlowGraphWidgetGetGraph(first);
    assert(raw && !xuiFlowGraphIsOwned(raw));
    assert(xrtMemDebugFailAfter(0));
    assert(xuiFlowGraphWidgetGetGraphOwned(first,&graph)==XUI_ERROR_OUT_OF_MEMORY);
    assert(graph==(xui_flow_graph)(uintptr_t)1 && xrtMemDebugFailTriggered());
    xrtMemDebugFailClear();xrtClearError();assert(!xuiFlowGraphIsOwned(raw));
    assert(xuiFlowGraphWidgetGetGraphOwned(first,&graph)==XUI_OK && graph==raw && refs(graph)==2);
    node(graph);
    xrtSetErrorKind(XERR_RANGE);const xerror* ambient=xrtGetError();
    assert(xuiFlowGraphWidgetSetGraphShared(second,graph)==XUI_OK && xrtGetError()==ambient);
    assert(xuiFlowGraphWidgetSetGraphShared(second,graph)==XUI_OK && xrtGetError()==ambient);
    xrtClearError();assert(refs(graph)==3);trace(ctx,graph,2);
    assert(xuiFlowGraphReleaseChecked(graph)==XUI_OK && refs(raw)==2);
    assert(xuiWidgetDestroyChecked(first)==XUI_OK && refs(raw)==1);
    assert(xuiFlowGraphWidgetGetGraphOwned(second,&graph)==XUI_OK && refs(graph)==2);
    assert(xuiWidgetDestroyChecked(second)==XUI_OK && refs(graph)==1);
    assert(xuiFlowGraphGetNodeCount(graph)==1);
    assert(xuiDestroyChecked(ctx)==XUI_OK && xuiFlowGraphReleaseChecked(graph)==XUI_OK);
}
typedef struct cleanup_test {xui_widget widget;xui_flow_graph old,source;int clones,drops,closeWidget;} cleanup_test;
typedef struct payload {cleanup_test* test;int copy;} payload;
static bool clone(void* raw,void** output,void* user)
{
    (void)user;payload* value=raw;*output=NULL;payload* copy=xrtMalloc(sizeof(*copy));if(!copy)return false;
    *copy=(payload){value->test,1};++value->test->clones;*output=copy;return true;
}
static void drop(void* raw,void* user)
{
    (void)user;payload* value=raw;if(!value->copy)return;
    cleanup_test* test=value->test;++test->drops;
    xui_flow_graph output=(xui_flow_graph)(uintptr_t)1;
    assert(xuiFlowGraphWidgetGetGraphOwned(test->widget,&output)==XUI_ERROR_BUSY && output==(xui_flow_graph)(uintptr_t)1);
    assert(xuiFlowGraphWidgetSetGraphShared(test->widget,test->source)==XUI_ERROR_BUSY);
    assert(xuiFlowGraphGetNodeCount(test->old)==XUI_ERROR_BUSY);
    /* Logical retirement may be accepted while physical free waits for this
     * native call. The incoming shared slot must then be released exactly once. */
    if(test->closeWidget)assert(xuiWidgetDestroyChecked(test->widget)==XUI_OK);
    xrtSetErrorInfo(XERR_STATE,"flow.widget.test",701,"old graph drop failed");xrtFree(value);
}
static void consuming_failure(int closeWidget)
{
    xui_context ctx=context();cleanup_test test={0};test.widget=canvas(ctx);
    test.closeWidget=closeWidget;
    assert(xuiFlowGraphWidgetGetGraphOwned(test.widget,&test.old)==XUI_OK);node(test.old);
    assert(xuiFlowGraphCreateOwned(&test.source)==XUI_OK);node(test.source);
    static const xvaluehandleops ops={clone,drop,NULL,NULL};payload data={&test,0};void* raw=&data;
    xvalue* value=xrtValueHandleTake(&raw,&ops,NULL);assert(value && !raw);
    assert(xuiFlowGraphSetNodeConfig(test.old,"one",value)==XUI_OK);xrtValueRelease(value);
    assert(xuiFlowGraphReleaseChecked(test.old)==XUI_OK);
    assert(xuiFlowGraphWidgetSetGraphShared(test.widget,test.source)==XUI_ERROR_CALLBACK_FAILED);
    assert(xrtGetError() && xrtErrorCode(xrtGetError())==701);xrtClearError();
    assert(test.clones==1 && test.drops==1 && refs(test.source)==(size_t)(closeWidget ? 1:2));
    if(!closeWidget)assert(xuiFlowGraphWidgetGetGraph(test.widget)==test.source);
    trace(ctx,test.source,closeWidget ? 0:1);
    assert(xuiFlowGraphGetNodeCount(test.source)==1 && xuiFlowGraphReleaseChecked(test.source)==XUI_OK);
    assert(xuiDestroyChecked(ctx)==XUI_OK);
}
static void borrowed_slots(void)
{
    xui_context ctx=context();xui_widget widget=canvas(ctx);xui_flow_graph raw=NULL,owned=NULL,alias=NULL;
    assert(xuiFlowGraphCreate(&raw)==XUI_OK);
    assert(xuiFlowGraphWidgetSetGraph(widget,raw,0)==XUI_OK);
    alias=(xui_flow_graph)(uintptr_t)1;
    assert(xuiFlowGraphWidgetGetGraphOwned(widget,&alias)==XUI_ERROR_INVALID_STATE && alias==(xui_flow_graph)(uintptr_t)1);
    assert(xuiFlowGraphCreateOwned(&owned)==XUI_OK);
    assert(xuiFlowGraphWidgetSetGraph(widget,owned,0)==XUI_OK);
    assert(xuiFlowGraphWidgetGetGraphOwned(widget,&alias)==XUI_OK && alias==owned && refs(owned)==2);
    trace(ctx,owned,0);
    assert(xuiDestroyChecked(ctx)==XUI_OK && refs(owned)==2);
    assert(xuiFlowGraphReleaseChecked(alias)==XUI_OK && xuiFlowGraphReleaseChecked(owned)==XUI_OK);
    assert(xuiFlowGraphDestroyChecked(raw)==XUI_OK);
}
int main(void)
{
    assert(xrtMemDebugEnable(true));xuiTestProxyInit(&proxy);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    aliases();consuming_failure(0);consuming_failure(1);borrowed_slots();
    assert(!xrtGetError());xrtMemDebugSnapshot(&after);
    assert(after.LiveCount==before.LiveCount && after.LiveBytes==before.LiveBytes);
    assert(!after.InvalidFreeCount && !after.DoubleFreeCount && !after.UseAfterFreeCount);
    puts("Shared FlowGraph canvas references, handoff failure and borrowed slots passed");return 0;
}
