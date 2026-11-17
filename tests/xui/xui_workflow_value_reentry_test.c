#define XRT_MODULE_ALL
#include "xui.h"
#include "src/xui/core/xui_xrt_port.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct context {
    xui_workflow workflow,peer;
    xui_flow_graph graph,peerGraph;
    int busy,clones,drops,probes,failClone;
} context;
typedef struct payload { context* ctx; int copy; } payload;

static void probe(context* ctx)
{
    if(!ctx->busy)return;
    xui_workflow workflow=ctx->workflow;
    xui_flow_graph graph=ctx->graph;
    const xerror* ambient=xrtGetError();
    #include "xui_workflow_busy_probes.inc"
    #include "xui_flow_graph_busy_probes.inc"
    int index=8192;
    xvalue* output=(xvalue*)(uintptr_t)1;
    xui_flow_node_info_t info,before;
    memset(&info,0x5a,sizeof(info));before=info;
    assert(xuiWorkflowAddVariable(workflow,NULL,&index)==XUI_ERROR_BUSY && index==8192);
    assert(xuiWorkflowCreateDefaultConfig(workflow,NULL,&output)==XUI_ERROR_BUSY && output==(xvalue*)(uintptr_t)1);
    assert(xuiFlowGraphGetNode(graph,0,&info)==XUI_ERROR_BUSY && !memcmp(&info,&before,sizeof(info)));
    assert(xuiWorkflowGetNodeCount(ctx->peer)==1);
    assert(xuiFlowGraphSetNodePosition(ctx->peerGraph,"peer",(float)ctx->probes,0)==XUI_OK);
    assert(xrtGetError()==ambient);
    ++ctx->probes;
}
static bool clone_value(void* raw,void** output,void* user)
{
    payload* original=raw;context* ctx=original->ctx;(void)user;*output=NULL;
    probe(ctx);if(ctx->failClone)return false;
    payload* copy=xrtMalloc(sizeof(*copy));if(!copy)return false;
    *copy=(payload){ctx,1};++ctx->clones;*output=copy;return true;
}
static void drop_value(void* raw,void* user)
{
    payload* value=raw;(void)user;
    if(value->copy){probe(value->ctx);++value->ctx->drops;xrtFree(value);}
}
static xvalue* custom(payload* data)
{
    static const xvaluehandleops ops={clone_value,drop_value,NULL,NULL};
    void* raw=data;xvalue* value=xrtValueHandleTake(&raw,&ops,NULL);assert(value && !raw);return value;
}
static xvalue* schema(payload* data)
{
    xvalue *root=xrtValueObject(),*fields=xrtValueArray(),*field=xrtValueObject(),*value=xrtValueObject();
    assert(root && fields && field && value);
    assert(xuiXrtValueObjectSetText(field,"id",0,"options",0));
    assert(xuiXrtValueObjectSetInt(field,"kind",0,XUI_WORKFLOW_CONFIG_FIELD_OBJECT));
    assert(xuiXrtValueObjectSetTake(value,"custom",0,custom(data)));
    assert(xuiXrtValueObjectSetTake(field,"default",0,value));
    assert(xuiXrtValueArrayAppendTake(fields,field));
    /* Native schema input can repeat an id. Replacing the prior default then
     * executes Drop inside ObjectSetTake, not an explicit ValueRelease. */
    xvalue* repeated=xrtValueClone(field);assert(repeated);
    assert(xuiXrtValueArrayAppendTake(fields,repeated));
    assert(xuiXrtValueObjectSetTake(root,"fields",0,fields));return root;
}
static void make(context* ctx,int owned)
{
    memset(ctx,0,sizeof(*ctx));
    assert((owned ? xuiWorkflowCreateOwned(&ctx->workflow):xuiWorkflowCreate(&ctx->workflow))==XUI_OK);
    assert(xuiWorkflowCreate(&ctx->peer)==XUI_OK);
    ctx->graph=xuiWorkflowGetGraph(ctx->workflow);ctx->peerGraph=xuiWorkflowGetGraph(ctx->peer);
    xui_flow_node_desc_t node={0};node.iSize=sizeof(node);node.sId="peer";
    assert(xuiFlowGraphAddNode(ctx->peerGraph,&node,NULL)==XUI_OK);
}
static void finish(context* ctx,int owned)
{
    xui_flow_graph alias=NULL;
    if(owned)assert(xuiWorkflowGetGraphOwned(ctx->workflow,&alias)==XUI_OK && alias==ctx->graph);
    ctx->busy=1;assert(xuiWorkflowDestroyChecked(ctx->workflow)==XUI_OK);ctx->busy=0;
    if(alias){assert(xuiFlowGraphGetNodeCount(alias)>=0);assert(xuiFlowGraphReleaseChecked(alias)==XUI_OK);}
    assert(ctx->clones==ctx->drops && !xrtGetError());
    assert(xuiWorkflowDestroyChecked(ctx->peer)==XUI_OK);
}
static void scenarios(int owned)
{
    context ctx;make(&ctx,owned);payload data={&ctx,0};xvalue* input=schema(&data);
    xui_workflow_node_type_desc_t type={0};type.iSize=sizeof(type);type.sId="task";type.pConfigSchema=input;
    ctx.busy=1;assert(xuiWorkflowRegisterNodeType(ctx.workflow,&type,NULL)==XUI_OK);ctx.busy=0;
    xrtValueRelease(input);
    xvalue* config=NULL;ctx.busy=1;
    assert(xuiWorkflowCreateDefaultConfig(ctx.workflow,"task",&config)==XUI_OK);ctx.busy=0;
    assert(config);xrtValueRelease(config);
    xvalue* value=custom(&data);
    xui_workflow_variable_desc_t variable={0};variable.iSize=sizeof(variable);variable.sId="variable";variable.pDefaultValue=value;
    ctx.busy=1;assert(xuiWorkflowAddVariable(ctx.workflow,&variable,NULL)==XUI_OK);ctx.busy=0;
    xvalue* output=NULL;ctx.busy=1;assert(xuiWorkflowToXValue(ctx.workflow,&output)==XUI_OK);ctx.busy=0;
    assert(output);xrtValueRelease(output);
    /* Imported variable Clone and replaced-table Drop must protect the target,
     * even though the transaction builds an independent temporary Workflow. */
    xvalue *root=xrtValueObject(),*nodes=xrtValueArray(),*edges=xrtValueArray(),*vars=xrtValueArray(),*var=xrtValueObject();
    assert(root && nodes && edges && vars && var);
    assert(xuiXrtValueObjectSetText(var,"id",0,"imported",0));
    assert(xuiXrtValueObjectSetTake(var,"default",0,custom(&data)));
    assert(xuiXrtValueArrayAppendTake(vars,var));
    assert(xuiXrtValueObjectSetTake(root,"nodes",0,nodes));
    assert(xuiXrtValueObjectSetTake(root,"edges",0,edges));
    assert(xuiXrtValueObjectSetTake(root,"variables",0,vars));
    ctx.busy=1;assert(xuiWorkflowLoadXValue(ctx.workflow,root)==XUI_OK);ctx.busy=0;
    xrtValueRelease(root);xrtValueRelease(value);
    assert(xuiWorkflowFindVariable(ctx.workflow,"imported")==0);
    int probes=ctx.probes;assert(probes>=6);finish(&ctx,owned);
    assert(ctx.probes>=probes+2);
}
static void failures(int owned)
{
    for(int budget=0;budget<90;++budget){
        context ctx;make(&ctx,owned);payload data={&ctx,0};xvalue* input=schema(&data);
        xui_workflow_node_type_desc_t type={0};type.iSize=sizeof(type);type.sId="task";type.pConfigSchema=input;
        ctx.busy=1;assert(xrtMemDebugFailAfter(budget));
        int status=xuiWorkflowRegisterNodeType(ctx.workflow,&type,NULL);
        bool triggered=xrtMemDebugFailTriggered();xrtMemDebugFailClear();ctx.busy=0;
        assert(status==(triggered ? XUI_ERROR_OUT_OF_MEMORY:XUI_OK));
        assert(xuiWorkflowGetNodeTypeCount(ctx.workflow)==(triggered ? 0:1));
        if(xrtGetError())xrtClearError();
        xrtValueRelease(input);finish(&ctx,owned);
    }
    context ctx;make(&ctx,owned);payload data={&ctx,0};xvalue* value=custom(&data);
    xui_workflow_variable_desc_t variable={0};variable.iSize=sizeof(variable);variable.sId="bad";variable.pDefaultValue=value;
    ctx.busy=ctx.failClone=1;assert(xuiWorkflowAddVariable(ctx.workflow,&variable,NULL)==XUI_ERROR_OUT_OF_MEMORY);
    ctx.busy=ctx.failClone=0;xrtClearError();assert(xuiWorkflowGetVariableCount(ctx.workflow)==0);
    xrtValueRelease(value);finish(&ctx,owned);
}
int main(void)
{
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(int owned=0;owned<2;++owned){scenarios(owned);failures(owned);}
    assert(!xrtGetError());xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(!after.DoubleFreeCount && !after.InvalidFreeCount && !after.UseAfterFreeCount);
    puts("Workflow schema/default Clone/Drop: same models Busy, independent models usable, transactional OOM passed");
    return 0;
}
