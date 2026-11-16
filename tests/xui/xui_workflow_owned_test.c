#define XRT_MODULE_ALL
#include "xui.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static size_t references(xrtownershipref ref)
{
    size_t count=0;xrtownershipscope freeze={0};
    assert(ref.Data && ref.Ops && xrtOwnershipFreezeTryBegin(&freeze));
    assert(ref.Ops->Count(ref.Data,&count));assert(xrtOwnershipScopeEnd(&freeze));
    return count;
}
static bool edge(xrtownershipref ref,void* raw)
{assert(ref.Data && ref.Ops);++*(int*)raw;return true;}
static bool unexpected_edge(xrtownershipref ref,void* raw)
{(void)ref;(void)raw;assert(!"Unavailable model called visitor");return false;}
static void add_node(xui_flow_graph graph,const char* id)
{xui_flow_node_desc_t desc={0};desc.iSize=sizeof(desc);desc.sId=id;assert(xuiFlowGraphAddNode(graph,&desc,NULL)==XUI_OK);}
static void add_type(xui_workflow workflow,const char* id)
{xui_workflow_node_type_desc_t desc={0};desc.iSize=sizeof(desc);desc.sId=id;assert(xuiWorkflowRegisterNodeType(workflow,&desc,NULL)==XUI_OK);}

static void aliases_and_reload(void)
{
    xui_workflow workflow=NULL,alias=NULL,raw=NULL;
    assert(xuiWorkflowCreate(&raw)==XUI_OK && !xuiWorkflowIsOwned(raw));
    alias=(xui_workflow)(uintptr_t)1;
    assert(xuiWorkflowShareOwned(raw,&alias)==XUI_ERROR_INVALID_ARGUMENT && alias==(xui_workflow)(uintptr_t)1);
    assert(xuiWorkflowCreateOwned(&workflow)==XUI_OK && xuiWorkflowIsOwned(workflow));
    assert(xuiWorkflowShareOwned(workflow,&alias)==XUI_OK && alias==workflow);
    xrtownershipref identity=xuiWorkflowOwnership(workflow);assert(references(identity)==2);
    xui_flow_graph graph=NULL;assert(xuiWorkflowGetGraphOwned(workflow,&graph)==XUI_OK);
    xrtownershipref graph_id=xuiFlowGraphOwnership(graph);assert(references(graph_id)==2);
    add_type(workflow,"task");
    assert(xuiWorkflowAddNode(workflow,"task","original",NULL,1,2,NULL)==XUI_OK);
    xvalue* value=NULL;assert(xuiWorkflowToXValue(workflow,&value)==XUI_OK && value);
    add_node(graph,"temporary");assert(xuiWorkflowGetNodeCount(alias)==2);
    assert(xuiWorkflowLoadXValue(workflow,value)==XUI_OK);xrtValueRelease(value);
    assert(xuiWorkflowOwnership(workflow).Data==identity.Data);
    assert(xuiFlowGraphOwnership(graph).Data==graph_id.Data);
    assert(xuiFlowGraphGetNodeCount(graph)==1 && xuiFlowGraphFindNode(graph,"original")==0);
    assert(xuiWorkflowGetNodeTypeCount(alias)==1 && xuiWorkflowFindNodeType(alias,"missing")==-1);
    xrtSetErrorKind(XERR_RANGE);const xerror* ambient=xrtGetError();
    assert(xuiWorkflowFindVariable(alias,"missing")==-1 && xrtGetError()==ambient);
    assert(xuiWorkflowGetNodeCount(alias)==1 && xrtGetError()==ambient);xrtClearError();
    int edges=0;xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(identity.Ops->Trace(identity.Data,edge,&edges) && edges==1);
    assert(xrtOwnershipScopeEnd(&freeze));
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(int i=0;i<1000;++i)assert(xuiWorkflowGetNodeCount(alias)==1);
    xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.ReallocCount==after.ReallocCount && before.FreeCount==after.FreeCount);
    assert(xuiWorkflowDestroyChecked(workflow)==XUI_OK && references(identity)==1);
    assert(xuiWorkflowGetNodeCount(alias)==1 && xuiWorkflowReleaseChecked(alias)==XUI_OK);
    assert(references(graph_id)==1 && xuiFlowGraphFindNode(graph,"original")==0);
    add_node(graph,"after-workflow");assert(xuiFlowGraphReleaseChecked(graph)==XUI_OK);
    assert(xuiWorkflowDestroyChecked(raw)==XUI_OK && xuiWorkflowDestroyChecked(NULL)==XUI_OK);
}

typedef struct worker_context {xui_workflow workflow;atomic_bool started,stop;unsigned probes;int finished;} worker_context;
static void rejected(worker_context* ctx)
{
    xui_workflow workflow=ctx->workflow;
    #include "xui_workflow_busy_probes.inc"
    assert(xuiWorkflowDestroyChecked(workflow)==XUI_ERROR_BUSY);
    xui_workflow alias=(xui_workflow)(uintptr_t)1;xui_flow_graph graph=(xui_flow_graph)(uintptr_t)1;bool consumed=true;
    assert(xuiWorkflowShareOwned(workflow,&alias)==XUI_ERROR_BUSY && alias==(xui_workflow)(uintptr_t)1);
    assert(xuiWorkflowGetGraphOwned(workflow,&graph)==XUI_ERROR_BUSY && graph==(xui_flow_graph)(uintptr_t)1);
    assert(xuiWorkflowReleaseOwnerChecked(workflow,&consumed)==XUI_ERROR_BUSY && !consumed);
    xrtownershipref ref=xuiWorkflowOwnership(workflow);
    assert(!xuiWorkflowOwnershipReadReady(ref) && xuiWorkflowOwnershipQuiescenceV1(ref)->Busy(ref.Data));
    assert(!ref.Ops->Trace(ref.Data,unexpected_edge,NULL));assert(!xrtGetError());
}
static int32 worker(void* raw)
{
    worker_context* ctx=raw;
    if(ctx->finished){bool consumed=false;assert(xuiWorkflowReleaseOwnerChecked(ctx->workflow,&consumed)==XUI_OK && consumed);return 0;}
    rejected(ctx);++ctx->probes;atomic_store_explicit(&ctx->started,true,memory_order_release);
    do{rejected(ctx);++ctx->probes;}while(!atomic_load_explicit(&ctx->stop,memory_order_acquire));
    return 0;
}
static void concurrent_rejection(void)
{
    worker_context ctx={0};atomic_init(&ctx.started,false);atomic_init(&ctx.stop,false);
    assert(xuiWorkflowCreateOwned(&ctx.workflow)==XUI_OK);add_type(ctx.workflow,"task");
    assert(xuiWorkflowAddNode(ctx.workflow,"task","node",NULL,0,0,NULL)==XUI_OK);
    xui_workflow alias=NULL;assert(xuiWorkflowShareOwned(ctx.workflow,&alias)==XUI_OK);
    xthread* thread=xrtThreadCreate(worker,&ctx,0);assert(thread);
    while(!atomic_load_explicit(&ctx.started,memory_order_acquire)){}
    for(int i=0;i<1000;++i){
        assert(xuiWorkflowSelectNode(ctx.workflow,"node",i%2)==XUI_OK);
        xui_workflow_variable_desc_t desc={0};desc.iSize=sizeof(desc);desc.sId="variable";
        if(!i)assert(xuiWorkflowAddVariable(ctx.workflow,&desc,NULL)==XUI_OK);
        assert(xuiWorkflowGetVariableCount(ctx.workflow)==1);
    }
    atomic_store_explicit(&ctx.stop,true,memory_order_release);
    assert(xrtThreadWaitFor(thread,5000)==XWAIT_OK);xrtThreadDestroy(thread);assert(ctx.probes>1);
    assert(xuiWorkflowReleaseChecked(alias)==XUI_OK && xuiWorkflowReleaseChecked(ctx.workflow)==XUI_OK);
}
static void collector_tail(void)
{
    worker_context ctx={0};assert(xuiWorkflowCreateOwned(&ctx.workflow)==XUI_OK);
    add_type(ctx.workflow,"task");
    xui_workflow alias=NULL;assert(xuiWorkflowShareOwned(ctx.workflow,&alias)==XUI_OK);
    xui_flow_graph graph=NULL;assert(xuiWorkflowGetGraphOwned(ctx.workflow,&graph)==XUI_OK);add_node(graph,"retained");
    xrtownershipref ref=xuiWorkflowOwnership(ctx.workflow);
    const xrtownershipadapterv1* adapter=xuiWorkflowOwnershipAdapterV1(ref);
    const xrtownershippreparationv1* prepare=xuiWorkflowOwnershipPreparationV1(ref);
    const xrtownershipfinishv2* finish=xuiWorkflowOwnershipCompletionV2(ref);
    const xrtownershipfinalizationv1* final=xuiWorkflowOwnershipFinalizationV1(ref);
    xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(adapter->Hold(ref.Data) && adapter->Claim(ref.Data,&ctx));assert(xrtOwnershipScopeEnd(&freeze));
    assert(prepare->Prepare(ref.Data,&ctx)==XRT_OWNERSHIP_PREPARE_READY);
    assert(!xuiWorkflowOwnershipReadReady(ref) && xuiWorkflowGetNodeCount(ctx.workflow)==XUI_ERROR_INVALID_STATE);
    bool complete=false;assert(final->CanStep(ref.Data,&ctx) && final->Step(ref.Data,&ctx,&complete) && complete);
    assert(xuiFlowGraphGetNodeCount(graph)==1 && references(xuiFlowGraphOwnership(graph))==1);
    assert(xrtOwnershipFreezeTryBegin(&freeze));adapter->Clear(ref.Data,&ctx);assert(xrtOwnershipScopeEnd(&freeze));
    assert(finish->V1.Step(ref.Data,&ctx)==XRT_OWNERSHIP_FINISH_READY && references(ref)==3);
    assert(xuiWorkflowGetNodeCount(alias)==XUI_ERROR_INVALID_STATE);
    assert(xuiWorkflowReleaseChecked(alias)==XUI_OK && finish->DropChecked(ref.Data) && references(ref)==1);
    ctx.finished=1;xthread* thread=xrtThreadCreate(worker,&ctx,0);assert(thread);
    assert(xrtThreadWaitFor(thread,5000)==XWAIT_OK);xrtThreadDestroy(thread);
    assert(xuiFlowGraphReleaseChecked(graph)==XUI_OK);
}

typedef struct callback_context {xui_workflow workflow;int clones,drops;} callback_context;
typedef struct payload {callback_context* context;bool copy;} payload;
static bool clone_value(void* raw,void** output,void* user)
{
    (void)user;payload* value=raw;payload* copy=xrtMalloc(sizeof(*copy));*output=NULL;if(!copy)return false;
    *copy=(payload){value->context,true};++value->context->clones;*output=copy;return true;
}
static void drop_value(void* raw,void* user)
{
    (void)user;payload* value=raw;if(!value->copy)return;
    callback_context* ctx=value->context;++ctx->drops;
    assert(xuiWorkflowGetVariableCount(ctx->workflow)==XUI_ERROR_INVALID_STATE);
    assert(!xuiWorkflowGetGraph(ctx->workflow));xrtClearError();
    xrtSetErrorInfo(XERR_STATE,"workflow.test",770+ctx->drops,"default Drop failed");xrtFree(value);
}
static void cleanup_errors(void)
{
    callback_context ctx={0};assert(xuiWorkflowCreateOwned(&ctx.workflow)==XUI_OK);
    payload data={&ctx,false};void* raw=&data;
    static const xvaluehandleops ops={clone_value,drop_value,NULL,NULL};
    xvalue* value=xrtValueHandleTake(&raw,&ops,NULL);assert(value && !raw);
    for(int i=0;i<2;++i){
        xui_workflow_variable_desc_t desc={0};desc.iSize=sizeof(desc);desc.sId=i ? "two":"one";desc.pDefaultValue=value;
        assert(xuiWorkflowAddVariable(ctx.workflow,&desc,NULL)==XUI_OK);
    }
    xrtValueRelease(value);bool consumed=false;
    assert(xuiWorkflowReleaseOwnerChecked(ctx.workflow,&consumed)==XUI_ERROR_CALLBACK_FAILED && consumed);
    assert(ctx.clones==2 && ctx.drops==2 && xrtErrorCode(xrtGetError())==771);xrtClearError();
}
static void failure_prefixes(void)
{
    for(uint64 budget=0;budget<8;++budget){
        xui_workflow workflow=(xui_workflow)(uintptr_t)1;
        assert(xrtMemDebugFailAfter(budget));int status=xuiWorkflowCreateOwned(&workflow);
        bool triggered=xrtMemDebugFailTriggered();xrtMemDebugFailClear();xrtClearError();
        assert(status==(budget<4 ? XUI_ERROR_OUT_OF_MEMORY:XUI_OK) && triggered==(budget<4));
        if(budget<4)assert(!workflow);
        else{
            xui_workflow alias=NULL;xui_flow_graph graph=NULL;
            assert(xrtMemDebugFailAfter(0));
            assert(xuiWorkflowShareOwned(workflow,&alias)==XUI_OK && xuiWorkflowGetGraphOwned(workflow,&graph)==XUI_OK);
            assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();
            assert(xuiWorkflowReleaseChecked(workflow)==XUI_OK && xuiWorkflowReleaseChecked(alias)==XUI_OK);
            assert(xuiFlowGraphReleaseChecked(graph)==XUI_OK);
        }
    }
}
int main(void)
{
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    failure_prefixes();aliases_and_reload();concurrent_rejection();collector_tail();cleanup_errors();
    assert(!xrtGetError());xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(!after.DoubleFreeCount && !after.InvalidFreeCount && !after.UseAfterFreeCount);
    puts("Workflow shared refs, graph survival/reload, checked retirement, thread rejection and OOM passed");return 0;
}
