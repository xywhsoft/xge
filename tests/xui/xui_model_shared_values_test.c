#define XRT_MODULE_ALL
#include "xui.h"
#include "src/xui/core/xui_xrt_port.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static xvalue* object(int count)
{
    xvalue* value=xrtValueObject();assert(value);
    assert(xuiXrtValueObjectSetInt(value,"count",0,count));return value;
}
static int count(xvalue* value)
{return (int)xuiXrtValueObjectGetInt(value,"count",0);}
static xvalue* config(xui_flow_graph graph,int shared)
{
    xvalue* value=NULL;
    assert((shared ? xuiFlowGraphGetNodeConfigShared(graph,"node",&value):
        xuiFlowGraphGetNodeConfig(graph,"node",&value))==XUI_OK && value);
    return value;
}
static xui_flow_graph make_graph(int owned)
{
    xui_flow_graph graph=NULL;
    assert((owned ? xuiFlowGraphCreateOwned(&graph):xuiFlowGraphCreate(&graph))==XUI_OK);
    xui_flow_node_desc_t node={0};node.iSize=sizeof(node);node.sId="node";
    assert(xuiFlowGraphAddNode(graph,&node,NULL)==XUI_OK);return graph;
}
static void graph_values(int owned)
{
    xui_flow_graph graph=make_graph(owned);xvalue* source=object(1);
    assert(xuiFlowGraphSetNodeConfig(graph,"node",source)==XUI_OK);
    assert(xuiXrtValueObjectSetInt(source,"count",0,2));
    xvalue* copy=config(graph,1);assert(copy!=source && count(copy)==1);xrtValueRelease(copy);
    assert(xuiFlowGraphSetNodeConfigShared(graph,"node",source)==XUI_OK);
    assert(xuiXrtValueObjectSetInt(source,"count",0,3));
    copy=config(graph,1);assert(copy==source && count(copy)==3);xrtValueRelease(copy);
    copy=config(graph,0);assert(copy!=source && count(copy)==3);
    assert(xuiXrtValueObjectSetInt(copy,"count",0,4));xrtValueRelease(copy);
    copy=config(graph,1);assert(count(copy)==3);xrtValueRelease(copy);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    assert(xrtMemDebugFailAfter(0));
    for(int i=0;i<1000;++i){
        assert(xuiFlowGraphSetNodeConfigShared(graph,"node",source)==XUI_OK);
        copy=config(graph,1);assert(copy==source);xrtValueRelease(copy);
    }
    assert(!xrtMemDebugFailTriggered());xrtMemDebugFailClear();xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.ReallocCount==after.ReallocCount && before.FreeCount==after.FreeCount);
    xvalue* incoming=object(7);
    assert(xuiFlowGraphCommandSetNodeConfigShared(graph,"node",incoming)==XUI_OK);
    copy=config(graph,1);assert(copy==incoming);xrtValueRelease(copy);
    assert(xuiXrtValueObjectSetInt(incoming,"count",0,99));
    assert(xuiFlowGraphUndo(graph)==XUI_OK);copy=config(graph,1);assert(count(copy)==3);xrtValueRelease(copy);
    assert(xuiFlowGraphRedo(graph)==XUI_OK);copy=config(graph,1);assert(count(copy)==7 && copy!=incoming);xrtValueRelease(copy);
    xrtValueRelease(incoming);xrtValueRelease(source);
    copy=config(graph,1);assert(count(copy)==7);xrtValueRelease(copy);
    assert(xuiFlowGraphSetNodeConfigShared(graph,"node",NULL)==XUI_OK);
    copy=(xvalue*)(uintptr_t)1;assert(xuiFlowGraphGetNodeConfigShared(graph,"node",&copy)==XUI_OK && !copy);
    assert(xuiFlowGraphDestroyChecked(graph)==XUI_OK);
}
static xvalue* schema(int initial,xvalue** field)
{
    xvalue *value=xrtValueObject(),*fields=xrtValueArray();*field=xrtValueObject();
    assert(value && fields && *field);
    assert(xuiXrtValueObjectSetText(*field,"id",0,"answer",0));
    assert(xuiXrtValueObjectSetInt(*field,"kind",0,XUI_WORKFLOW_CONFIG_FIELD_INT));
    assert(xuiXrtValueObjectSetInt(*field,"default",0,initial));
    assert(xuiXrtValueArrayAppendTake(fields,*field));
    assert(xuiXrtValueObjectSetTake(value,"fields",0,fields));return value;
}
static void workflow_values(int owned)
{
    xui_workflow workflow=NULL;
    assert((owned ? xuiWorkflowCreateOwned(&workflow):xuiWorkflowCreate(&workflow))==XUI_OK);
    xvalue* field=NULL; xvalue* input=schema(1,&field);
    xui_workflow_node_type_desc_t type={0};type.iSize=sizeof(type);type.sId="clone";type.pConfigSchema=input;
    assert(xuiWorkflowRegisterNodeType(workflow,&type,NULL)==XUI_OK);
    type.sId="shared";assert(xuiWorkflowRegisterNodeTypeShared(workflow,&type,NULL)==XUI_OK);
    assert(xuiXrtValueObjectSetInt(field,"default",0,2));xrtValueRelease(input);
    xvalue* value=NULL;assert(xuiWorkflowCreateDefaultConfig(workflow,"clone",&value)==XUI_OK);
    assert(xuiXrtValueObjectGetInt(value,"answer",0)==1);xrtValueRelease(value);
    assert(xuiWorkflowCreateDefaultConfig(workflow,"shared",&value)==XUI_OK);
    assert(xuiXrtValueObjectGetInt(value,"answer",0)==2);xrtValueRelease(value);
    input=object(3);
    xui_workflow_variable_desc_t variable={0};variable.iSize=sizeof(variable);variable.sId="clone";variable.pDefaultValue=input;
    assert(xuiWorkflowAddVariable(workflow,&variable,NULL)==XUI_OK);
    variable.sId="shared";assert(xuiWorkflowAddVariableShared(workflow,&variable,NULL)==XUI_OK);
    assert(xuiXrtValueObjectSetInt(input,"count",0,4));
    xui_workflow_variable_desc_t result={0};
    assert(xuiWorkflowGetVariable(workflow,0,&result)==XUI_OK && result.pDefaultValue!=input && count(result.pDefaultValue)==3);
    assert(xuiWorkflowGetVariable(workflow,1,&result)==XUI_OK && result.pDefaultValue==input && count(result.pDefaultValue)==4);
    xrtValueRelease(input);assert(count(result.pDefaultValue)==4);
    assert(xuiWorkflowDestroyChecked(workflow)==XUI_OK);
}
typedef struct drop_context {xui_flow_graph graph;xui_workflow workflow;int clones,drops;} drop_context;
static bool forbidden_clone(void* raw,void** output,void* user)
{(void)user;drop_context* ctx=*(drop_context**)raw;++ctx->clones;*output=NULL;return false;}
static void shared_drop(void* raw,void* user)
{
    (void)user;drop_context* ctx=*(drop_context**)raw;
    xui_flow_graph graph=ctx->graph;
    #include "xui_flow_graph_busy_probes.inc"
    if(ctx->workflow){
        xui_workflow workflow=ctx->workflow;
        #include "xui_workflow_busy_probes.inc"
    }
    ++ctx->drops;xrtFree(raw);
}
static xvalue* drop_value(drop_context* ctx)
{
    static const xvaluehandleops ops={forbidden_clone,shared_drop,NULL,NULL};
    drop_context** payload=xrtMalloc(sizeof(*payload));assert(payload);*payload=ctx;
    void* raw=payload;xvalue* value=xrtValueHandleTake(&raw,&ops,NULL);assert(value && !raw);return value;
}
static void shared_callbacks(int owned)
{
    drop_context ctx={0};ctx.graph=make_graph(owned);xvalue* value=drop_value(&ctx);
    assert(xuiFlowGraphSetNodeConfigShared(ctx.graph,"node",value)==XUI_OK);
    xvalue* alias=config(ctx.graph,1);assert(alias==value);xrtValueRelease(alias);xrtValueRelease(value);
    assert(!ctx.clones && !ctx.drops);
    assert(xuiFlowGraphSetNodeConfigShared(ctx.graph,"node",NULL)==XUI_OK);
    assert(ctx.drops==1 && !ctx.clones);assert(xuiFlowGraphDestroyChecked(ctx.graph)==XUI_OK);
    memset(&ctx,0,sizeof(ctx));
    assert((owned ? xuiWorkflowCreateOwned(&ctx.workflow):xuiWorkflowCreate(&ctx.workflow))==XUI_OK);
    ctx.graph=xuiWorkflowGetGraph(ctx.workflow);xui_flow_graph graphAlias=NULL;
    if(owned)assert(xuiWorkflowGetGraphOwned(ctx.workflow,&graphAlias)==XUI_OK);
    value=drop_value(&ctx);
    xui_workflow_node_type_desc_t type={0};type.iSize=sizeof(type);type.sId="type";type.pConfigSchema=value;
    assert(xuiWorkflowRegisterNodeTypeShared(ctx.workflow,&type,NULL)==XUI_OK);xrtValueRelease(value);
    value=drop_value(&ctx);
    xui_workflow_variable_desc_t variable={0};variable.iSize=sizeof(variable);variable.sId="variable";variable.pDefaultValue=value;
    assert(xuiWorkflowAddVariableShared(ctx.workflow,&variable,NULL)==XUI_OK);xrtValueRelease(value);
    assert(!ctx.clones && !ctx.drops);assert(xuiWorkflowDestroyChecked(ctx.workflow)==XUI_OK);
    assert(!ctx.clones && ctx.drops==2);
    if(graphAlias)assert(xuiFlowGraphReleaseChecked(graphAlias)==XUI_OK);
}
static void command_failures(int owned)
{
    int rejected=0,accepted=0;
    for(int budget=0;budget<48;++budget){
        xui_flow_graph graph=make_graph(owned);xvalue *old=object(1),*next=object(2);
        assert(xuiFlowGraphSetNodeConfigShared(graph,"node",old)==XUI_OK);
        assert(xrtMemDebugFailAfter(budget));int status=xuiFlowGraphCommandSetNodeConfigShared(graph,"node",next);
        bool failed=xrtMemDebugFailTriggered();xrtMemDebugFailClear();
        assert(status==(failed ? XUI_ERROR_OUT_OF_MEMORY:XUI_OK));
        if(failed)++rejected;else ++accepted;
        if(xrtGetError()){
            assert(failed && xrtErrorKind(xrtGetError())==XERR_MEMORY);
            xrtClearError();
        }
        xvalue* stored=config(graph,1);assert(count(stored)==(failed ? 1:2));xrtValueRelease(stored);
        assert(xuiFlowGraphCanUndo(graph)==(failed ? 0:1));
        xrtValueRelease(old);xrtValueRelease(next);assert(xuiFlowGraphDestroyChecked(graph)==XUI_OK);
    }
    assert(rejected>0 && accepted>0);
}
static void workflow_failures(int owned)
{
    for(int variable=0;variable<2;++variable){
        int rejected=0,accepted=0;
        for(int budget=0;budget<32;++budget){
            xui_workflow workflow=NULL;
            assert((owned ? xuiWorkflowCreateOwned(&workflow):xuiWorkflowCreate(&workflow))==XUI_OK);
            xvalue* value=object(1);int index=-2;
            xui_workflow_node_type_desc_t type={0};type.iSize=sizeof(type);type.sId="type";
            type.sTitle="title";type.sCategory="category";type.sDescription="description";type.sIcon="icon";type.pConfigSchema=value;
            xui_workflow_variable_desc_t desc={0};desc.iSize=sizeof(desc);desc.sId="variable";
            desc.sTitle="title";desc.sType="object";desc.sScope="scope";desc.pDefaultValue=value;
            assert(xrtMemDebugFailAfter(budget));
            int status=variable ? xuiWorkflowAddVariableShared(workflow,&desc,&index):xuiWorkflowRegisterNodeTypeShared(workflow,&type,&index);
            bool failed=xrtMemDebugFailTriggered();xrtMemDebugFailClear();
            assert(status==(failed ? XUI_ERROR_OUT_OF_MEMORY:XUI_OK));
            if(failed)++rejected;else ++accepted;
            if(xrtGetError()){assert(failed && xrtErrorKind(xrtGetError())==XERR_MEMORY);xrtClearError();}
            assert(index==(failed ? -1:0));
            assert((variable ? xuiWorkflowGetVariableCount(workflow):xuiWorkflowGetNodeTypeCount(workflow))==(failed ? 0:1));
            assert(xuiWorkflowDestroyChecked(workflow)==XUI_OK && count(value)==1);xrtValueRelease(value);
        }
        assert(rejected>0 && accepted>0);
    }
}
int main(void)
{
    assert(xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    for(int owned=0;owned<2;++owned){graph_values(owned);workflow_values(owned);shared_callbacks(owned);command_failures(owned);workflow_failures(owned);}
    assert(!xrtGetError());xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes);
    assert(!after.DoubleFreeCount && !after.InvalidFreeCount && !after.UseAfterFreeCount);
    puts("Model Shared values: exact refs, independent native Clone, retained lifetimes, history snapshots and OOM passed");
    puts("MODEL_SHARED_WARM_XRT_ALLOCATIONS=0");return 0;
}
