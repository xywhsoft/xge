#define XRT_MODULE_MEMORY_DEBUG
#include "xui.h"
#include "src/xui/core/xui_xrt_port.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int xuiInternalFlowGraphGetGeometryRevisionChecked(xui_flow_graph,uint32_t*);
int xuiInternalFlowGraphSwapContents(xui_flow_graph,xui_flow_graph);
int xuiInternalFlowGraphUpdateNodeConfiguration(xui_flow_graph,const char*,xvalue*,const xui_flow_port_desc_t*,int,int,const char*);
typedef struct context {
    xui_flow_graph graph,peer,alsoBusy;
    xui_workflow workflow;
    struct context* nested;
    xvalue* nestedValue;
    int legacy,failClone,nesting,callerDrop,clones,drops,probes,cloneCalls;
} context;
typedef struct payload {context* ctx;int copy;} payload;
static void add_node(xui_flow_graph graph,const char* id)
{xui_flow_node_desc_t desc={0};desc.iSize=sizeof(desc);desc.sId=id;assert(xuiFlowGraphAddNode(graph,&desc,NULL)==XUI_OK);}
static bool unexpected_visit(xrtownershipref ref,void* user)
{(void)ref;(void)user;assert(!"Busy ownership trace invoked a visitor");return false;}
static void take_busy(void)
{xerror* error=xrtTakeError();assert(error && xrtErrorCode(error)==XUI_ERROR_BUSY);assert(strcmp(xrtErrorDomain(error),"xui.flow_graph")==0);xrtErrorFree(error);}
static void probe(context* ctx)
{
    xui_flow_graph graph=ctx->graph;
    const xerror* ambient=xrtGetError();
    uint32_t revision=1234,clean=5678,geometry=9012;
    xui_flow_node_info_t info,before;
    xvalue* config=(xvalue*)(uintptr_t)1;
    ++ctx->probes;
    #include "xui_flow_graph_busy_probes.inc"
    assert(xuiFlowGraphGetRevisionChecked(graph,&revision)==XUI_ERROR_BUSY && revision==1234);
    assert(xuiFlowGraphGetCleanRevisionChecked(graph,&clean)==XUI_ERROR_BUSY && clean==5678);
    assert(xuiInternalFlowGraphGetGeometryRevisionChecked(graph,&geometry)==XUI_ERROR_BUSY && geometry==9012);
    assert(xuiInternalFlowGraphSwapContents(graph,ctx->peer)==XUI_ERROR_BUSY);
    assert(xuiInternalFlowGraphSwapContents(ctx->peer,graph)==XUI_ERROR_BUSY);
    assert(xuiInternalFlowGraphUpdateNodeConfiguration(graph,"one",NULL,NULL,0,0,NULL)==XUI_ERROR_BUSY);
    memset(&info,0x5a,sizeof(info));before=info;
    assert(xuiFlowGraphGetNode(graph,0,&info)==XUI_ERROR_BUSY && memcmp(&info,&before,sizeof(info))==0);
    assert(xuiFlowGraphGetNodeConfig(graph,"one",&config)==XUI_ERROR_BUSY && config==(xvalue*)(uintptr_t)1);
    assert(xuiFlowGraphGetNodeCount(ctx->peer)==1);
    assert(xuiFlowGraphSetNodePosition(ctx->peer,"peer",(float)ctx->probes,0)==XUI_OK);
    if(ctx->workflow){
        xui_workflow workflow=ctx->workflow;
        #include "xui_workflow_busy_probes.inc"
        assert(xuiWorkflowGetNodeConfig(workflow,"one",&config)==XUI_ERROR_BUSY && config==(xvalue*)(uintptr_t)1);
    }
    assert(xrtGetError()==ambient);
    if(ctx->legacy){
        assert(xuiFlowGraphGetRevision(graph)==0);if(ambient)assert(xrtGetError()==ambient);else take_busy();
        assert(xuiFlowGraphGetCleanRevision(graph)==0);if(ambient)assert(xrtGetError()==ambient);else take_busy();
        xuiFlowGraphDestroy(graph);if(ambient)assert(xrtGetError()==ambient);else take_busy();
        assert(!xuiFlowGraphTraceOwnedValues(graph,unexpected_visit,NULL));if(ambient)assert(xrtGetError()==ambient);else take_busy();
        if(ctx->workflow){
            assert(!xuiWorkflowGetGraph(ctx->workflow));if(ambient)assert(xrtGetError()==ambient);else take_busy();
            xuiWorkflowDestroy(ctx->workflow);if(ambient)assert(xrtGetError()==ambient);else take_busy();
            assert(!xuiWorkflowTraceOwnedValues(ctx->workflow,unexpected_visit,NULL));if(ambient)assert(xrtGetError()==ambient);else take_busy();
        }
    }
}
static bool clone_config(void* raw,void** output,void* user)
{
    payload* source=raw;context* ctx=source->ctx;payload* copy;(void)user;*output=NULL;
    ++ctx->cloneCalls;probe(ctx);
    if(ctx->alsoBusy)assert(xuiFlowGraphGetNodeCount(ctx->alsoBusy)==XUI_ERROR_BUSY);
    if(ctx->nested && !ctx->nesting){
        ctx->nesting=1;
        assert(xuiFlowGraphSetNodeConfig(ctx->nested->graph,"one",ctx->nestedValue)==XUI_OK);
        ctx->nesting=0;
        assert(xuiFlowGraphGetNodeCount(ctx->nested->graph)==1);
        assert(xuiFlowGraphGetNodeCount(ctx->graph)==XUI_ERROR_BUSY);
    }
    if(ctx->failClone)return false;
    copy=xrtMalloc(sizeof(*copy));if(!copy)return false;
    copy->ctx=ctx;copy->copy=1;++ctx->clones;*output=copy;return true;
}
static void drop_config(void* raw,void* user)
{
    payload* value=raw;(void)user;
    if(value->copy){
        if(value->ctx->callerDrop)assert(xuiFlowGraphGetNodeCount(value->ctx->graph)==1);
        else probe(value->ctx);
        ++value->ctx->drops;xrtFree(value);
    }
}
static xvalue* handle(payload* data)
{static const xvaluehandleops ops={clone_config,drop_config,NULL,NULL};void* raw=data;xvalue* value=xrtValueHandleTake(&raw,&ops,NULL);assert(value && !raw);return value;}
static void make(context* ctx,int legacy)
{memset(ctx,0,sizeof(*ctx));ctx->legacy=legacy;assert(xuiFlowGraphCreate(&ctx->graph)==0);assert(xuiFlowGraphCreate(&ctx->peer)==0);add_node(ctx->graph,"one");add_node(ctx->peer,"peer");}
static void finish(context* ctx)
{assert(xuiFlowGraphDestroyChecked(ctx->graph)==0);assert(ctx->clones==ctx->drops && !xrtGetError());assert(xuiFlowGraphDestroyChecked(ctx->peer)==0);ctx->graph=ctx->peer=NULL;}
static void run_history(void)
{
    context ctx;payload data;xvalue *value,*nothing,*copy=NULL;uint32_t before;
    make(&ctx,1);data=(payload){&ctx,0};value=handle(&data);
    assert(xuiFlowGraphSetNodeConfig(ctx.graph,"one",value)==0);
    before=xuiFlowGraphGetRevision(ctx.graph);
    ctx.failClone=1;assert(xuiFlowGraphSetNodeConfig(ctx.graph,"one",value)==XUI_ERROR_OUT_OF_MEMORY);ctx.failClone=0;
    if(xrtGetError())xrtErrorFree(xrtTakeError());
    assert(xuiFlowGraphGetRevision(ctx.graph)==before && xuiFlowGraphGetNodeCount(ctx.graph)==1);
    assert(xuiInternalFlowGraphUpdateNodeConfiguration(ctx.graph,"one",value,NULL,0,0,"summary")==0);
    assert(xuiFlowGraphCommandSetNodeConfig(ctx.graph,"one",value)==0);
    assert(xuiFlowGraphUndo(ctx.graph)==0 && xuiFlowGraphRedo(ctx.graph)==0);
    assert(xuiFlowGraphCommandRemoveNode(ctx.graph,"one")==0 && xuiFlowGraphGetNodeCount(ctx.graph)==0);
    assert(xuiFlowGraphUndo(ctx.graph)==0 && xuiFlowGraphGetNodeCount(ctx.graph)==1);
    assert(xuiFlowGraphRedo(ctx.graph)==0 && xuiFlowGraphUndo(ctx.graph)==0);
    assert(xuiFlowGraphSelectNode(ctx.graph,"one",1)==0);
    assert(xuiFlowGraphDeleteSelection(ctx.graph)==0 && xuiFlowGraphGetNodeCount(ctx.graph)==0);
    assert(xuiFlowGraphUndo(ctx.graph)==0 && xuiFlowGraphGetNodeCount(ctx.graph)==1);
    assert(xuiFlowGraphSetCommandHistoryLimit(ctx.graph,1)==0 && xuiFlowGraphGetCommandHistoryLimit(ctx.graph)==1);
    assert(xuiFlowGraphCommandSetNodeConfig(ctx.graph,"one",value)==0 && xuiFlowGraphCanRedo(ctx.graph)==0);
    assert(xuiFlowGraphGetNodeConfig(ctx.graph,"one",&copy)==0 && copy);
    /* Independent caller-owned output Drop has no model gate. */
    ctx.callerDrop=1;xrtValueRelease(copy);ctx.callerDrop=0;
    nothing=xrtValueNull();assert(nothing);assert(xuiFlowGraphSetNodeConfig(ctx.graph,"one",nothing)==0);xrtValueRelease(nothing);
    assert(xuiFlowGraphSetNodeConfig(ctx.graph,"one",value)==0);xrtValueRelease(value);
    assert(ctx.probes>20 && ctx.cloneCalls>10);finish(&ctx);
}
static void nested_models(void)
{
    context a,b;payload pa,pb;xvalue *va,*vb;
    make(&a,1);make(&b,1);pa=(payload){&a,0};pb=(payload){&b,0};va=handle(&pa);vb=handle(&pb);
    a.nested=&b;a.nestedValue=vb;b.alsoBusy=a.graph;
    assert(xuiFlowGraphSetNodeConfig(a.graph,"one",va)==0);
    a.nested=NULL;b.alsoBusy=NULL;xrtValueRelease(va);xrtValueRelease(vb);finish(&a);finish(&b);
}
static void allocation_failures(void)
{
    int failures=0,successes=0;
    for(int budget=0;budget<48;budget++){
        context ctx;payload data;xvalue *value,*found=(xvalue*)(uintptr_t)1;uint32_t revision;int status;
        make(&ctx,0);data=(payload){&ctx,0};value=handle(&data);revision=xuiFlowGraphGetRevision(ctx.graph);
        assert(xrtMemDebugFailAfter(budget));status=xuiFlowGraphCommandSetNodeConfig(ctx.graph,"one",value);xrtMemDebugFailClear();
        if(xrtGetError())xrtErrorFree(xrtTakeError());
        if(status==0){++successes;assert(xuiFlowGraphUndo(ctx.graph)==0 && xuiFlowGraphRedo(ctx.graph)==0);}
        else {++failures;assert(status==XUI_ERROR_OUT_OF_MEMORY);assert(xuiFlowGraphGetRevision(ctx.graph)==revision && !xuiFlowGraphCanUndo(ctx.graph));assert(xuiFlowGraphGetNodeConfig(ctx.graph,"one",&found)==0 && !found);}
        assert(xuiFlowGraphGetNodeCount(ctx.graph)==1);xrtValueRelease(value);finish(&ctx);
    }
    assert(failures>0 && successes>0);
}
static xvalue* workflow_document(payload* data)
{
    xvalue *root=xrtValueObject(),*nodes=xrtValueArray(),*edges=xrtValueArray();
    assert(root && nodes && edges);
    if(data){
        xvalue *node=xrtValueObject(),*config=xrtValueObject();assert(node && config);
        assert(xuiXrtValueObjectSetText(node,"id",0,(ptr)"one",0,FALSE));
        assert(xuiXrtValueObjectSetText(node,"type",0,(ptr)"test",0,FALSE));
        assert(xuiXrtValueObjectSetTake(config,"custom",0,handle(data)));
        assert(xuiXrtValueObjectSetTake(node,"config",0,config));
        assert(xuiXrtValueArrayAppendTake(nodes,node));
    }
    assert(xuiXrtValueObjectSetTake(root,"nodes",0,nodes));
    assert(xuiXrtValueObjectSetTake(root,"edges",0,edges));
    return root;
}
static void workflow_replacement(void)
{
    context ctx;payload data;xvalue *document,*empty;int prior;
    memset(&ctx,0,sizeof(ctx));ctx.legacy=1;
    assert(xuiWorkflowCreate(&ctx.workflow)==0);ctx.graph=xuiWorkflowGetGraph(ctx.workflow);
    assert(ctx.graph && xuiFlowGraphCreate(&ctx.peer)==0);add_node(ctx.peer,"peer");
    data=(payload){&ctx,0};document=workflow_document(&data);empty=workflow_document(NULL);
    assert(xuiWorkflowLoadXValue(ctx.workflow,document)==0 && ctx.clones>0);
    assert(ctx.graph==xuiWorkflowGetGraph(ctx.workflow) && xuiWorkflowGetNodeCount(ctx.workflow)==1);
    prior=ctx.probes;
    assert(xuiWorkflowLoadXValue(ctx.workflow,document)==0 && ctx.probes>prior);
    prior=ctx.drops;
    assert(xuiWorkflowLoadXValue(ctx.workflow,empty)==0 && ctx.drops>prior);
    assert(xuiWorkflowGetNodeCount(ctx.workflow)==0 && ctx.clones==ctx.drops);
    assert(xuiWorkflowLoadXValue(ctx.workflow,document)==0);
    xrtValueRelease(document);xrtValueRelease(empty);
    /* Destroying the owner must also keep it alive throughout model Drop. */
    xuiWorkflowDestroy(ctx.workflow);ctx.workflow=NULL;ctx.graph=NULL;
    assert(ctx.clones==ctx.drops);
    if(xrtGetError())fprintf(stderr,"workflow error %d: %s\n",xrtErrorCode(xrtGetError()),xrtErrorMessage(xrtGetError()));
    assert(!xrtGetError());assert(xuiFlowGraphDestroyChecked(ctx.peer)==0);
}
static void workflow_allocation_failures(void)
{
    int failures=0,successes=0;
    for(int budget=0;budget<128;budget++){
        context ctx;payload data;xvalue* document;uint32_t revision;int status;
        memset(&ctx,0,sizeof(ctx));assert(xuiWorkflowCreate(&ctx.workflow)==0);
        ctx.graph=xuiWorkflowGetGraph(ctx.workflow);add_node(ctx.graph,"sentinel");
        assert(xuiFlowGraphCreate(&ctx.peer)==0);add_node(ctx.peer,"peer");
        data=(payload){&ctx,0};document=workflow_document(&data);revision=xuiFlowGraphGetRevision(ctx.graph);
        assert(xrtMemDebugFailAfter(budget));status=xuiWorkflowLoadXValue(ctx.workflow,document);xrtMemDebugFailClear();
        if(xrtGetError())xrtErrorFree(xrtTakeError());
        assert(xuiWorkflowGetNodeCount(ctx.workflow)==1);
        if(status==0){++successes;assert(xuiFlowGraphFindNode(ctx.graph,"one")==0);}
        else{++failures;assert(status==XUI_ERROR_OUT_OF_MEMORY);assert(xuiFlowGraphFindNode(ctx.graph,"sentinel")==0 && xuiFlowGraphGetRevision(ctx.graph)==revision);}
        xrtValueRelease(document);xuiWorkflowDestroy(ctx.workflow);
        assert(ctx.clones==ctx.drops && !xrtGetError());assert(xuiFlowGraphDestroyChecked(ctx.peer)==0);
    }
    assert(failures>0 && successes>0);
}
int main(void)
{
    xmemdebugsnapshot before,after;assert(xrtMemDebugEnable(true));xrtMemDebugSnapshot(&before);
    for(int i=0;i<4;i++){run_history();nested_models();workflow_replacement();}allocation_failures();workflow_allocation_failures();
    assert(xuiFlowGraphDestroyChecked(NULL)==0 && xuiFlowGraphGetRevision(NULL)==0 && xuiFlowGraphGetCleanRevision(NULL)==0);
    xrtMemDebugSnapshot(&after);assert(after.LiveCount==before.LiveCount && after.LiveBytes==before.LiveBytes);
    assert(after.InvalidFreeCount==before.InvalidFreeCount && after.DoubleFreeCount==before.DoubleFreeCount);
    puts("FlowGraph Busy: 63 model and 30 workflow entries, outputs, history/undo/redo/trim/destroy, logical replacement, independent models, OOM and heap balance passed");return 0;
}
