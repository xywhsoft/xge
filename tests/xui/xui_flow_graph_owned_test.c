#define XRT_MODULE_ALL
#include "xui.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdatomic.h>
#include <string.h>

int xuiInternalFlowGraphPromoteOwned(xui_flow_graph);
int xuiInternalFlowGraphSwapContents(xui_flow_graph,xui_flow_graph);
int xuiInternalFlowGraphUpdateNodeConfiguration(xui_flow_graph,const char*,xvalue*,const xui_flow_port_desc_t*,int,int,const char*);

static size_t references(xrtownershipref ref)
{
    size_t count=0;xrtownershipscope freeze={0};
    assert(ref.Data && ref.Ops && xrtOwnershipFreezeTryBegin(&freeze));
    assert(ref.Ops->Count(ref.Data,&count));assert(xrtOwnershipScopeEnd(&freeze));
    return count;
}
static void node(xui_flow_graph graph,const char* id)
{xui_flow_node_desc_t desc={0};desc.iSize=sizeof(desc);desc.sId=id;assert(xuiFlowGraphAddNode(graph,&desc,NULL)==XUI_OK);}
static bool count_edge(xrtownershipref ref,void* raw)
{assert(ref.Data && ref.Ops);++*(int*)raw;return true;}
static bool unreachable_visit(xrtownershipref ref,void* raw)
{(void)ref;(void)raw;assert(!"Busy graph invoked visitor");return false;}

static void aliases_and_swap(void)
{
    xui_flow_graph graph=NULL,alias=NULL,raw=NULL,second=NULL;
    assert(xuiFlowGraphCreate(&raw)==XUI_OK && !xuiFlowGraphIsOwned(raw));
    alias=(xui_flow_graph)(uintptr_t)1;
    assert(xuiFlowGraphShareOwned(raw,&alias)==XUI_ERROR_INVALID_ARGUMENT && alias==(xui_flow_graph)(uintptr_t)1);
    assert(xuiFlowGraphCreateOwned(&graph)==XUI_OK && xuiFlowGraphIsOwned(graph));
    node(graph,"old");node(raw,"loaded");
    xrtownershipref identity=xuiFlowGraphOwnership(graph);
    assert(references(identity)==1);
    assert(xuiFlowGraphShareOwned(graph,&alias)==XUI_OK && alias==graph && references(identity)==2);
    assert(xuiInternalFlowGraphSwapContents(graph,raw)==XUI_OK);
    assert(xuiFlowGraphOwnership(alias).Data==identity.Data && !xuiFlowGraphIsOwned(raw));
    assert(xuiFlowGraphFindNode(alias,"loaded")==0 && xuiFlowGraphFindNode(raw,"old")==0);
    assert(xuiFlowGraphCreateOwned(&second)==XUI_OK);node(second,"second");
    xrtownershipref other=xuiFlowGraphOwnership(second);
    assert(xuiInternalFlowGraphSwapContents(graph,second)==XUI_OK);
    assert(xuiFlowGraphOwnership(graph).Data==identity.Data && xuiFlowGraphOwnership(second).Data==other.Data);
    assert(xuiFlowGraphFindNode(alias,"second")==0 && xuiFlowGraphFindNode(second,"loaded")==0);
    xrtSetErrorKind(XERR_RANGE);const xerror* ambient=xrtGetError();
    assert(xuiFlowGraphFindNode(alias,"missing")==-1 && xrtGetError()==ambient);
    assert(xuiFlowGraphGetNodeCount(alias)==1 && xrtGetError()==ambient);xrtClearError();
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(int i=0;i<1000;++i)assert(xuiFlowGraphGetNodeCount(alias)==1);
    xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.ReallocCount==after.ReallocCount);
    assert(xuiFlowGraphDestroyChecked(graph)==XUI_OK && references(identity)==1);
    assert(xuiFlowGraphSetNodePosition(alias,"second",1,2)==XUI_OK);
    assert(xuiFlowGraphReleaseChecked(alias)==XUI_OK);
    assert(xuiFlowGraphDestroyChecked(raw)==XUI_OK && xuiFlowGraphReleaseChecked(second)==XUI_OK);
}

typedef struct callback_context {
    xui_flow_graph graph,peer;
    int clones,drops,probes,errors;
} callback_context;
typedef struct payload {callback_context* context;int copy;} payload;
static void probe(callback_context* ctx)
{
    xui_flow_graph graph=ctx->graph;const xerror* ambient=xrtGetError();
    ++ctx->probes;
    #include "xui_flow_graph_busy_probes.inc"
    xui_flow_graph alias=(xui_flow_graph)(uintptr_t)1;bool consumed=true;
    assert(xuiFlowGraphShareOwned(graph,&alias)==XUI_ERROR_BUSY && alias==(xui_flow_graph)(uintptr_t)1);
    assert(xuiFlowGraphReleaseOwnerChecked(graph,&consumed)==XUI_ERROR_BUSY && !consumed);
    xrtownershipref ref=xuiFlowGraphOwnership(graph);
    assert(!xuiFlowGraphOwnershipReadReady(ref));
    assert(xuiFlowGraphOwnershipQuiescenceV1(ref)->Busy(ref.Data));
    assert(!xuiFlowGraphOwnershipPreparationV1(ref)->Ready(ref.Data));
    assert(!xuiFlowGraphOwnershipCompletionV2(ref)->DropReady(ref.Data));
    assert(!ref.Ops->Trace(ref.Data,unreachable_visit,NULL));
    assert(xuiFlowGraphGetNodeCount(ctx->peer)==1);
    assert(xuiFlowGraphSetNodePosition(ctx->peer,"peer",(float)ctx->probes,0)==XUI_OK);
    assert(xrtGetError()==ambient);
}
static bool clone_value(void* raw,void** output,void* user)
{
    (void)user;payload* source=raw;callback_context* ctx=source->context;
    *output=NULL;probe(ctx);payload* copy=xrtMalloc(sizeof(*copy));if(!copy)return false;
    *copy=(payload){ctx,1};++ctx->clones;*output=copy;return true;
}
static void drop_value(void* raw,void* user)
{
    (void)user;payload* value=raw;
    if(value->copy){callback_context* ctx=value->context;probe(ctx);++ctx->drops;
        if(ctx->errors)xrtSetErrorInfo(XERR_STATE,"flow.test",700+ctx->drops,"config drop failed");
        xrtFree(value);
    }
}
static xvalue* config(payload* data)
{
    static const xvaluehandleops ops={clone_value,drop_value,NULL,NULL};
    void* raw=data;xvalue* value=xrtValueHandleTake(&raw,&ops,NULL);assert(value && !raw);return value;
}
static void callbacks_and_cleanup(void)
{
    callback_context ctx={0};
    assert(xuiFlowGraphCreateOwned(&ctx.graph)==XUI_OK && xuiFlowGraphCreateOwned(&ctx.peer)==XUI_OK);
    node(ctx.graph,"one");node(ctx.graph,"two");node(ctx.peer,"peer");
    payload data={&ctx,0};xvalue* value=config(&data);
    assert(xuiFlowGraphSetNodeConfig(ctx.graph,"one",value)==XUI_OK);
    assert(xuiInternalFlowGraphUpdateNodeConfiguration(ctx.graph,"two",value,NULL,0,0,"second")==XUI_OK);
    assert(xuiFlowGraphCommandSetNodeConfig(ctx.graph,"one",value)==XUI_OK);
    assert(xuiFlowGraphUndo(ctx.graph)==XUI_OK && xuiFlowGraphRedo(ctx.graph)==XUI_OK);
    int edges=0;xrtownershipref ref=xuiFlowGraphOwnership(ctx.graph);
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(ref.Ops->Trace(ref.Data,count_edge,&edges) && edges==4);
    assert(xrtOwnershipScopeEnd(&freeze));
    xrtValueRelease(value);ctx.errors=1;int drops=ctx.drops;bool consumed=false;
    assert(xuiFlowGraphReleaseOwnerChecked(ctx.graph,&consumed)==XUI_ERROR_CALLBACK_FAILED && consumed);
    assert(xrtGetError() && xrtErrorCode(xrtGetError())==701+drops);xrtClearError();
    assert(ctx.clones==ctx.drops && ctx.probes>4);
    assert(xuiFlowGraphReleaseChecked(ctx.peer)==XUI_OK);
}

typedef struct worker_context {xui_flow_graph graph;int finished;} worker_context;
static int32 worker(void* raw)
{
    worker_context* ctx=raw;bool consumed=false;
    if(ctx->finished){assert(xuiFlowGraphReleaseOwnerChecked(ctx->graph,&consumed)==XUI_OK && consumed);return 0;}
    assert(xuiFlowGraphGetNodeCount(ctx->graph)==XUI_ERROR_BUSY);
    xui_flow_graph alias=(xui_flow_graph)(uintptr_t)1;
    assert(xuiFlowGraphShareOwned(ctx->graph,&alias)==XUI_ERROR_BUSY && alias==(xui_flow_graph)(uintptr_t)1);
    assert(xuiFlowGraphReleaseOwnerChecked(ctx->graph,&consumed)==XUI_ERROR_BUSY && !consumed);
    xrtownershipref ref=xuiFlowGraphOwnership(ctx->graph);
    assert(!xuiFlowGraphOwnershipReadReady(ref) && xuiFlowGraphOwnershipQuiescenceV1(ref)->Busy(ref.Data));
    assert(!ref.Ops->Trace(ref.Data,unreachable_visit,NULL));
    assert(!xuiFlowGraphOwnershipCompletionV2(ref)->DropReady(ref.Data));assert(!xrtGetError());return 0;
}
static void run_worker(worker_context* ctx)
{xthread* thread=xrtThreadCreate(worker,ctx,0);assert(thread && xrtThreadWaitFor(thread,5000)==XWAIT_OK);xrtThreadDestroy(thread);}
typedef struct parallel_context {
    worker_context worker;
    atomic_bool started,stop;
    unsigned calls;
} parallel_context;
static int32 parallel_worker(void* raw)
{
    parallel_context* ctx=raw;
    /* A real shared reference keeps the address alive on this rejected thread. */
    assert(worker(&ctx->worker)==0);++ctx->calls;
    atomic_store_explicit(&ctx->started,true,memory_order_release);
    do{assert(worker(&ctx->worker)==0);++ctx->calls;}
    while(!atomic_load_explicit(&ctx->stop,memory_order_acquire));
    return 0;
}
static void concurrent_rejection(void)
{
    parallel_context ctx={0};xui_flow_graph raw=NULL,alias=NULL;
    atomic_init(&ctx.started,false);atomic_init(&ctx.stop,false);
    assert(xuiFlowGraphCreateOwned(&ctx.worker.graph)==XUI_OK);
    assert(xuiFlowGraphCreate(&raw)==XUI_OK);node(ctx.worker.graph,"one");node(raw,"one");
    assert(xuiFlowGraphShareOwned(ctx.worker.graph,&alias)==XUI_OK);
    xthread* thread=xrtThreadCreate(parallel_worker,&ctx,0);assert(thread);
    while(!atomic_load_explicit(&ctx.started,memory_order_acquire)){}
    for(int i=0;i<1000;++i){
        assert(xuiFlowGraphSetNodePosition(ctx.worker.graph,"one",(float)i,0)==XUI_OK);
        assert(xuiInternalFlowGraphSwapContents(ctx.worker.graph,raw)==XUI_OK);
    }
    atomic_store_explicit(&ctx.stop,true,memory_order_release);
    assert(xrtThreadWaitFor(thread,5000)==XWAIT_OK);xrtThreadDestroy(thread);assert(ctx.calls>1);
    assert(xuiFlowGraphReleaseChecked(alias)==XUI_OK);
    assert(xuiFlowGraphReleaseChecked(ctx.worker.graph)==XUI_OK && xuiFlowGraphDestroyChecked(raw)==XUI_OK);
}
static void collector_and_affinity(void)
{
    worker_context ctx={0};assert(xuiFlowGraphCreateOwned(&ctx.graph)==XUI_OK);node(ctx.graph,"one");
    xui_flow_graph alias=NULL;assert(xuiFlowGraphShareOwned(ctx.graph,&alias)==XUI_OK);run_worker(&ctx);
    xrtownershipref ref=xuiFlowGraphOwnership(ctx.graph);assert(references(ref)==2);
    const xrtownershipadapterv1* adapter=xuiFlowGraphOwnershipAdapterV1(ref);
    const xrtownershipfinalizationv1* final=xuiFlowGraphOwnershipFinalizationV1(ref);
    const xrtownershipfinishv2* finish=xuiFlowGraphOwnershipCompletionV2(ref);
    const xrtownershippreparationv1* prepare=xuiFlowGraphOwnershipPreparationV1(ref);
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(adapter->Hold(ref.Data) && adapter->Claim(ref.Data,&ctx) && prepare->Ready(ref.Data));
    assert(xrtOwnershipScopeEnd(&freeze) && prepare->Prepare(ref.Data,&ctx)==XRT_OWNERSHIP_PREPARE_READY);
    assert(references(ref)==3 && !xuiFlowGraphOwnershipReadReady(ref));
    alias=(xui_flow_graph)(uintptr_t)1;
    assert(xuiFlowGraphShareOwned(ctx.graph,&alias)==XUI_ERROR_INVALID_STATE && alias==(xui_flow_graph)(uintptr_t)1);
    assert(xuiFlowGraphGetNodeCount(ctx.graph)==XUI_ERROR_INVALID_STATE);
    bool complete=false;assert(final->CanStep(ref.Data,&ctx) && final->Step(ref.Data,&ctx,&complete) && complete);
    assert(xuiFlowGraphGetNodeCount(ctx.graph)==XUI_ERROR_INVALID_STATE);
    assert(xrtOwnershipFreezeTryBegin(&freeze));adapter->Clear(ref.Data,&ctx);assert(xrtOwnershipScopeEnd(&freeze));
    assert(finish->V1.Step(ref.Data,&ctx)==XRT_OWNERSHIP_FINISH_READY);
    assert(finish->V1.Step(ref.Data,&ctx)==XRT_OWNERSHIP_FINISH_READY);
    /* Finish is not physical free: both real views and the GC pin still exist. */
    assert(references(ref)==3 && xuiFlowGraphIsOwned(ctx.graph));
    assert(xuiFlowGraphGetNodeCount(ctx.graph)==XUI_ERROR_INVALID_STATE);
    assert(xuiFlowGraphReleaseChecked(ctx.graph)==XUI_OK && references(ref)==2);
    assert(finish->DropReady(ref.Data) && finish->DropChecked(ref.Data) && references(ref)==1);
    ctx.finished=1;run_worker(&ctx);
}
static void parent_edge(void)
{
    xui_workflow workflow=NULL;assert(xuiWorkflowCreate(&workflow)==XUI_OK);
    xui_flow_graph graph=xuiWorkflowGetGraph(workflow),alias=NULL;
    assert(xuiInternalFlowGraphPromoteOwned(graph)==XUI_OK);
    assert(xuiFlowGraphShareOwned(graph,&alias)==XUI_OK);
    xvalue* value=xrtValueInt(12);node(graph,"one");
    assert(value && xuiFlowGraphSetNodeConfig(graph,"one",value)==XUI_OK);xrtValueRelease(value);
    int edges=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiWorkflowTraceOwnedValues(workflow,count_edge,&edges) && edges==1);
    assert(xrtOwnershipScopeEnd(&freeze));
    xuiWorkflowDestroy(workflow);
    assert(xuiFlowGraphGetNodeCount(alias)==1 && references(xuiFlowGraphOwnership(alias))==1);
    assert(xuiFlowGraphReleaseChecked(alias)==XUI_OK);
}
static void create_failures(void)
{
    for(int budget=0;budget<3;++budget){
        xui_flow_graph graph=(xui_flow_graph)(uintptr_t)1;
        assert(xrtMemDebugFailAfter((uint64)budget));int status=xuiFlowGraphCreateOwned(&graph);
        bool triggered=xrtMemDebugFailTriggered();xrtMemDebugFailClear();xrtClearError();
        assert(status==(budget<2 ? XUI_ERROR_OUT_OF_MEMORY:XUI_OK));
        assert(triggered==(budget<2));
        if(budget<2)assert(!graph);else assert(xuiFlowGraphReleaseChecked(graph)==XUI_OK);
    }
}
int main(void)
{
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    create_failures();aliases_and_swap();callbacks_and_cleanup();collector_and_affinity();concurrent_rejection();parent_edge();
    assert(!xrtGetError());xrtMemDebugSnapshot(&after);
    assert(after.LiveCount==before.LiveCount && after.LiveBytes==before.LiveBytes);
    assert(!after.DoubleFreeCount && !after.InvalidFreeCount && !after.UseAfterFreeCount);
    puts("FlowGraph real shared refs, callback Busy, collector tail, thread affinity and OOM passed");return 0;
}
