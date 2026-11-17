#define XRT_MODULE_ALL
#include "xui.h"
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

/* Linker-only fault injection covers the native unconsumed cleanup contract.
 * Ordinary lifetime, callback failure and OOM cases use actual model code. */
static xui_workflow blocked_release;
static unsigned release_blocks,blocked_calls;
static xui_widget late_canvas,late_parent;
static unsigned late_failures,late_hits;
static bool late_close;
int __real_xuiWidgetInvalidate(xui_widget,uint32_t);
int __wrap_xuiWidgetInvalidate(xui_widget widget,uint32_t flags)
{
    if(widget==late_canvas && late_failures){
        --late_failures;++late_hits;
        if(late_close)assert(xuiWidgetDestroyChecked(late_parent)==XUI_OK);
        xrtSetErrorInfo(XERR_STATE,"workflow.widget.test",901,"injected post-publication invalidate error");
        return XUI_ERROR_RESOURCE_FAILED;
    }
    return __real_xuiWidgetInvalidate(widget,flags);
}
int __real_xuiWorkflowReleaseOwnerChecked(xui_workflow,bool*);
int __wrap_xuiWorkflowReleaseOwnerChecked(xui_workflow workflow,bool* consumed)
{
    if(workflow==blocked_release && release_blocks){
        --release_blocks;++blocked_calls;*consumed=false;
        xrtSetErrorInfo(XERR_STATE,"workflow.widget.test",801,"injected unconsumed reference");
        return XUI_ERROR_BUSY;
    }
    return __real_xuiWorkflowReleaseOwnerChecked(workflow,consumed);
}

static xui_test_proxy_state_t proxy;
static size_t references(xui_workflow workflow)
{
    xrtownershipref ref=xuiWorkflowOwnership(workflow);size_t count=0;xrtownershipscope freeze={0};
    assert(ref.Data && xrtOwnershipFreezeTryBegin(&freeze));
    assert(ref.Ops->Count(ref.Data,&count) && xrtOwnershipScopeEnd(&freeze));return count;
}
static xui_context context(void)
{
    xui_context ctx=NULL;assert(xuiCreate(&ctx)==XUI_OK);
    assert(xuiSetProxy(ctx,&proxy.tProxy)==XUI_OK);return ctx;
}
static xui_widget canvas(xui_context ctx)
{xui_widget widget=NULL;assert(xuiWorkflowWidgetCreate(ctx,&widget,NULL)==XUI_OK);return widget;}
static void node(xui_workflow workflow,const char* id)
{
    xui_workflow_node_type_desc_t type={0};type.iSize=sizeof(type);type.sId="task";
    assert(xuiWorkflowRegisterNodeType(workflow,&type,NULL)==XUI_OK);
    assert(xuiWorkflowAddNode(workflow,"task",id,NULL,0,0,NULL)==XUI_OK);
}
static xrtownershipref expected;
static unsigned matches;
static bool edge(xrtownershipref ref,void* user)
{(void)user;if(ref.Data==expected.Data && ref.Ops==expected.Ops)++matches;return true;}
static void trace(xui_context ctx,xui_workflow workflow,unsigned count)
{
    expected=xuiWorkflowOwnership(workflow);matches=0;xrtownershipscope freeze={0};
    assert(xrtOwnershipFreezeTryBegin(&freeze) && xuiContextTraceOwnership(ctx,edge,NULL));
    assert(xrtOwnershipScopeEnd(&freeze) && matches==count);
}
static void aliases_and_edges(void)
{
    xui_context ctx=context();xui_widget first=canvas(ctx),second=canvas(ctx);
    xui_workflow workflow=NULL;assert(xuiWorkflowWidgetGetWorkflowOwned(first,&workflow)==XUI_OK);
    assert(references(workflow)==2);node(workflow,"one");
    xui_flow_graph graph=NULL;assert(xuiWorkflowGetGraphOwned(workflow,&graph)==XUI_OK);
    xrtSetErrorKind(XERR_RANGE);const xerror* ambient=xrtGetError();
    assert(xuiWorkflowWidgetSetWorkflowShared(second,workflow)==XUI_OK && xrtGetError()==ambient);
    assert(xuiWorkflowWidgetSetWorkflowShared(second,workflow)==XUI_OK && xrtGetError()==ambient);
    xrtClearError();assert(references(workflow)==3);trace(ctx,workflow,2);
    xmemdebugsnapshot warm_before,warm_after;xrtMemDebugSnapshot(&warm_before);
    for(int i=0;i<1000;++i){
        xui_workflow view=NULL;assert(xuiWorkflowWidgetGetWorkflowOwned(second,&view)==XUI_OK);
        assert(xuiWorkflowReleaseChecked(view)==XUI_OK);
        assert(xuiWorkflowWidgetSetWorkflowShared(second,workflow)==XUI_OK);
    }
    xrtMemDebugSnapshot(&warm_after);
    assert(warm_before.AllocCount==warm_after.AllocCount && warm_before.FreeCount==warm_after.FreeCount &&
        warm_before.ReallocCount==warm_after.ReallocCount);
    assert(xuiWorkflowReleaseChecked(workflow)==XUI_OK && references(workflow)==2);
    assert(xuiWidgetDestroyChecked(first)==XUI_OK && references(workflow)==1);
    assert(xuiWorkflowWidgetGetWorkflowOwned(second,&workflow)==XUI_OK && references(workflow)==2);
    trace(ctx,workflow,1);
    assert(xuiDestroyChecked(ctx)==XUI_OK && references(workflow)==1);
    assert(xuiWorkflowGetNodeCount(workflow)==1 && xuiWorkflowReleaseChecked(workflow)==XUI_OK);
    assert(xuiFlowGraphGetNodeCount(graph)==1 && xuiFlowGraphFindNode(graph,"one")==0);
    assert(xuiFlowGraphReleaseChecked(graph)==XUI_OK);
}
static void promotion_failures(void)
{
    for(uint64 budget=0;budget<3;++budget){
        xui_context ctx=context();xui_widget widget=canvas(ctx);
        xui_workflow original=xuiWorkflowWidgetGetWorkflow(widget);
        xui_workflow result=(xui_workflow)(uintptr_t)1;
        assert(!xuiWorkflowIsOwned(original) && xrtMemDebugFailAfter(budget));
        int status=xuiWorkflowWidgetGetWorkflowOwned(widget,&result);
        bool failed=xrtMemDebugFailTriggered();xrtMemDebugFailClear();xrtClearError();
        assert(status==(budget<2 ? XUI_ERROR_OUT_OF_MEMORY:XUI_OK) && failed==(budget<2));
        if(budget<2){
            assert(result==(xui_workflow)(uintptr_t)1 && !xuiWorkflowIsOwned(original));
            assert(!xuiFlowGraphIsOwned(xuiWorkflowGetGraph(original)));
        }else assert(result==original && references(result)==2 && xuiWorkflowReleaseChecked(result)==XUI_OK);
        assert(xuiDestroyChecked(ctx)==XUI_OK);
    }
}
static void handoff_failure_prefixes(void)
{
    unsigned failures=0,successes=0;
    for(uint64 budget=0;budget<32;++budget){
        xui_context ctx=context();xui_widget widget=canvas(ctx);xui_workflow source=NULL;
        xui_workflow original=xuiWorkflowWidgetGetWorkflow(widget);
        assert(xuiWorkflowCreateOwned(&source)==XUI_OK);node(source,"new");
        assert(xrtMemDebugFailAfter(budget));
        int status=xuiWorkflowWidgetSetWorkflowShared(widget,source);
        bool failed=xrtMemDebugFailTriggered();xrtMemDebugFailClear();xrtClearError();
        assert(status==XUI_OK || status<0);
        if(failed){assert(status<0);++failures;}else{assert(status==XUI_OK);++successes;}
        xui_workflow current=xuiWorkflowWidgetGetWorkflow(widget);
        assert(current==original || current==source);
        assert(references(source)==(size_t)(current==source ? 2:1));
        assert(xuiFlowGraphWidgetGetGraph(xuiWorkflowWidgetGetCanvas(widget))==xuiWorkflowGetGraph(current));
        assert(xuiDestroyChecked(ctx)==XUI_OK && references(source)==1);
        assert(xuiWorkflowReleaseChecked(source)==XUI_OK);
    }
    assert(failures>=2 && successes>0);
    printf("Workflow handoff OOM budgets=32 triggered=%u successful=%u\n",failures,successes);
}
static void borrowed_slots(void)
{
    xui_context ctx=context();xui_widget widget=canvas(ctx);xui_workflow raw=NULL,owned=NULL,alias=NULL;
    assert(xuiWorkflowCreate(&raw)==XUI_OK && xuiWorkflowWidgetSetWorkflow(widget,raw,0)==XUI_OK);
    alias=(xui_workflow)(uintptr_t)1;
    assert(xuiWorkflowWidgetGetWorkflowOwned(widget,&alias)==XUI_ERROR_INVALID_STATE && alias==(xui_workflow)(uintptr_t)1);
    assert(!xuiWorkflowIsOwned(raw));
    assert(xuiWorkflowCreateOwned(&owned)==XUI_OK && xuiWorkflowWidgetSetWorkflow(widget,owned,0)==XUI_OK);
    assert(xuiWorkflowWidgetGetWorkflowOwned(widget,&alias)==XUI_OK && alias==owned && references(owned)==2);
    trace(ctx,owned,0);
    assert(xuiDestroyChecked(ctx)==XUI_OK && references(owned)==2);
    assert(xuiWorkflowReleaseChecked(alias)==XUI_OK && xuiWorkflowReleaseChecked(owned)==XUI_OK);
    assert(xuiWorkflowDestroyChecked(raw)==XUI_OK);
}

typedef struct cleanup_test {xui_widget widget;xui_workflow old,source;int clones,drops,closeWidget;} cleanup_test;
typedef struct payload {cleanup_test* test;bool copy;} payload;
static bool clone(void* raw,void** output,void* user)
{
    (void)user;payload* value=raw;*output=NULL;payload* copy=xrtMalloc(sizeof(*copy));if(!copy)return false;
    *copy=(payload){value->test,true};++value->test->clones;*output=copy;return true;
}
static void drop(void* raw,void* user)
{
    (void)user;payload* value=raw;if(!value->copy)return;
    cleanup_test* test=value->test;++test->drops;
    xui_workflow output=(xui_workflow)(uintptr_t)1;
    assert(xuiWorkflowWidgetGetWorkflowOwned(test->widget,&output)==XUI_ERROR_BUSY && output==(xui_workflow)(uintptr_t)1);
    assert(xuiWorkflowWidgetSetWorkflowShared(test->widget,test->source)==XUI_ERROR_BUSY);
    assert(xuiWorkflowGetNodeCount(test->old)==XUI_ERROR_INVALID_STATE);
    if(test->closeWidget)assert(xuiWidgetDestroyChecked(test->widget)==XUI_OK);
    xrtSetErrorInfo(XERR_STATE,"workflow.widget.test",701,"old model Drop failed");xrtFree(value);
}
static void consuming_failure(int closeWidget,int late)
{
    xui_context ctx=context();cleanup_test test={0};test.widget=canvas(ctx);test.closeWidget=closeWidget;
    assert(xuiWorkflowWidgetGetWorkflowOwned(test.widget,&test.old)==XUI_OK);
    assert(xuiWorkflowCreateOwned(&test.source)==XUI_OK);node(test.source,"new");
    payload data={&test,false};void* raw=&data;static const xvaluehandleops ops={clone,drop,NULL,NULL};
    xvalue* value=xrtValueHandleTake(&raw,&ops,NULL);assert(value && !raw);
    xui_workflow_variable_desc_t desc={0};desc.iSize=sizeof(desc);desc.sId="variable";desc.pDefaultValue=value;
    assert(xuiWorkflowAddVariable(test.old,&desc,NULL)==XUI_OK);xrtValueRelease(value);
    assert(xuiWorkflowReleaseChecked(test.old)==XUI_OK);
    unsigned hits=late_hits;
    if(late){late_canvas=xuiWorkflowWidgetGetCanvas(test.widget);late_failures=1;}
    assert(xuiWorkflowWidgetSetWorkflowShared(test.widget,test.source)==(late ? XUI_ERROR_RESOURCE_FAILED:XUI_ERROR_CALLBACK_FAILED));
    assert(xrtErrorCode(xrtGetError())==(late ? 901:701) && test.clones==1 && test.drops==1);xrtClearError();
    assert(late_hits==hits+(late ? 1u:0u));late_canvas=NULL;late_failures=0;
    assert(references(test.source)==(size_t)(closeWidget ? 1:2));trace(ctx,test.source,closeWidget ? 0:1);
    if(!closeWidget){
        assert(xuiWorkflowWidgetGetWorkflow(test.widget)==test.source);
        assert(xuiFlowGraphWidgetGetGraph(xuiWorkflowWidgetGetCanvas(test.widget))==xuiWorkflowGetGraph(test.source));
    }
    assert(xuiWorkflowReleaseChecked(test.source)==XUI_OK && xuiDestroyChecked(ctx)==XUI_OK);
}
static void retirement_after_graph_publication(void)
{
    xui_context ctx=context();xui_widget widget=canvas(ctx);xui_workflow source=NULL;
    assert(xuiWorkflowCreateOwned(&source)==XUI_OK);node(source,"new");
    late_canvas=xuiWorkflowWidgetGetCanvas(widget);late_parent=widget;late_close=true;late_failures=1;
    unsigned hits=late_hits;
    assert(xuiWorkflowWidgetSetWorkflowShared(widget,source)==XUI_ERROR_RESOURCE_FAILED && late_hits==hits+1);
    assert(xrtErrorCode(xrtGetError())==901);xrtClearError();
    late_canvas=late_parent=NULL;late_close=false;late_failures=0;
    assert(references(source)==1 && xuiWorkflowGetNodeCount(source)==1);trace(ctx,source,0);
    assert(xuiWorkflowReleaseChecked(source)==XUI_OK && xuiDestroyChecked(ctx)==XUI_OK);
}
static void unconsumed_failure(int destroy)
{
    xui_context ctx=context();xui_widget widget=canvas(ctx);xui_workflow old=NULL,source=NULL,next=NULL;
    assert(xuiWorkflowWidgetGetWorkflowOwned(widget,&old)==XUI_OK && references(old)==2);
    assert(xuiWorkflowCreateOwned(&source)==XUI_OK && xuiWorkflowCreateOwned(&next)==XUI_OK);
    node(old,"old");node(source,"source");node(next,"next");
    blocked_release=old;release_blocks=1;unsigned prior=blocked_calls;
    assert(xuiWorkflowWidgetSetWorkflowShared(widget,source)==XUI_ERROR_BUSY && blocked_calls==prior+1);
    assert(xrtErrorCode(xrtGetError())==801);xrtClearError();
    assert(references(old)==2 && references(source)==2 && xuiWorkflowWidgetGetWorkflow(widget)==source);
    trace(ctx,old,1);trace(ctx,source,1);
    if(destroy){
        release_blocks=100;
        bool consumed=true;
        assert(xuiDestroyOwnerChecked(ctx,&consumed)==XUI_ERROR_BUSY && !consumed);
        assert(xrtErrorCode(xrtGetError())==801);xrtClearError();
        assert(references(old)==2 && references(source)==2);
        release_blocks=0;
        assert(xuiDestroyOwnerChecked(ctx,&consumed)==XUI_OK && consumed && references(old)==1 && references(source)==1);
    }else{
        assert(xuiWorkflowWidgetSetWorkflowShared(widget,next)==XUI_OK);
        assert(references(old)==1 && references(source)==1 && references(next)==2);
        trace(ctx,old,0);trace(ctx,next,1);
        assert(xuiDestroyChecked(ctx)==XUI_OK && references(next)==1);
    }
    blocked_release=NULL;release_blocks=0;
    assert(xuiWorkflowGetNodeCount(old)==1 && xuiWorkflowGetNodeCount(source)==1);
    assert(xuiWorkflowReleaseChecked(old)==XUI_OK && xuiWorkflowReleaseChecked(source)==XUI_OK && xuiWorkflowReleaseChecked(next)==XUI_OK);
}
static void current_release_retry(void)
{
    xui_context ctx=context();xui_widget widget=canvas(ctx);xui_workflow workflow=NULL;
    assert(xuiWorkflowWidgetGetWorkflowOwned(widget,&workflow)==XUI_OK);node(workflow,"current");
    xui_flow_graph graph=NULL;assert(xuiWorkflowGetGraphOwned(workflow,&graph)==XUI_OK);
    blocked_release=workflow;release_blocks=100;
    bool consumed=true;
    assert(xuiDestroyOwnerChecked(ctx,&consumed)==XUI_ERROR_BUSY && !consumed);
    assert(xrtErrorCode(xrtGetError())==801);xrtClearError();
    assert(references(workflow)==2 && xuiWorkflowGetNodeCount(workflow)==1 && xuiFlowGraphGetNodeCount(graph)==1);
    release_blocks=0;
    assert(xuiDestroyOwnerChecked(ctx,&consumed)==XUI_OK && consumed && references(workflow)==1);
    blocked_release=NULL;
    assert(xuiWorkflowReleaseChecked(workflow)==XUI_OK && xuiFlowGraphGetNodeCount(graph)==1);
    assert(xuiFlowGraphReleaseChecked(graph)==XUI_OK);
}
int main(void)
{
    assert(xrtMemDebugEnable(true));xuiTestProxyInit(&proxy);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    aliases_and_edges();promotion_failures();handoff_failure_prefixes();borrowed_slots();consuming_failure(0,0);consuming_failure(1,0);
    consuming_failure(0,1);retirement_after_graph_publication();unconsumed_failure(0);unconsumed_failure(1);current_release_retry();
    assert(!xrtGetError());xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(!after.DoubleFreeCount && !after.InvalidFreeCount && !after.UseAfterFreeCount);
    puts("Workflow canvas real aliases, owning edges, consuming errors, retained cleanup and OOM passed");return 0;
}
