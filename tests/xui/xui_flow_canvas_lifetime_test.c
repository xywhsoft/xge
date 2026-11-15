#define XRT_MODULE_MEMORY_DEBUG
#include "xui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

enum { QUERY, REPLACE, CLOSE_WIDGET, CLOSE_CONTEXT };
typedef struct canvas_probe {
    xui_context context;
    xui_widget widget,canvas,peer,peerWorkflow;
    xui_flow_graph graph,replacement;
    xui_workflow workflow,replacementWorkflow;
    int action,legacy,retiring,callerDrop,replacing,clones,drops,probes;
} canvas_probe;

static void add_node(xui_flow_graph graph,const char* id)
{
    xui_flow_node_desc_t node={0};node.iSize=sizeof(node);node.sId=id;
    assert(xuiFlowGraphAddNode(graph,&node,NULL)==XUI_OK);
}
static xui_widget canvas(xui_context context,xui_flow_graph graph,int own)
{
    xui_flow_graph_desc_t desc={0};xui_widget widget=NULL;
    desc.iSize=sizeof(desc);desc.pGraph=graph;desc.bOwnGraph=own;
    assert(xuiFlowGraphWidgetCreate(context,&widget,&desc)==XUI_OK && widget);return widget;
}
static xui_widget workflow_canvas(xui_context context,xui_workflow workflow,int own)
{
    xui_workflow_desc_t desc={0};xui_widget widget=NULL;
    desc.iSize=sizeof(desc);desc.pWorkflow=workflow;desc.bOwnWorkflow=own;
    assert(xuiWorkflowWidgetCreate(context,&widget,&desc)==XUI_OK && widget);return widget;
}
static void take_busy(void)
{
    xerror* error=xrtTakeError();assert(error && xrtErrorCode(error)==XUI_ERROR_BUSY);
    assert(strcmp(xrtErrorDomain(error),"xui.flow_graph")==0);xrtErrorFree(error);
}
static void busy_queries(canvas_probe* p)
{
    const xerror* ambient=xrtGetError();xmemdebugsnapshot before,after;
    xui_flow_hit_t hit,copy;xui_widget output=(xui_widget)(uintptr_t)1;
    xui_flow_graph_desc_t desc={0};desc.iSize=sizeof(desc);desc.pGraph=p->graph;desc.bOwnGraph=1;
    ++p->probes;xrtMemDebugSnapshot(&before);
    assert(xuiContextOwnershipBusy(p->context));
    assert(xuiFlowGraphGetNodeCount(p->graph)==XUI_ERROR_BUSY);
    assert(xuiFlowGraphGetNodeCount(p->replacement)==1);
    assert(xuiFlowGraphWidgetSetGraph(p->peer,p->graph,1)==XUI_ERROR_BUSY);
    assert(xuiFlowGraphWidgetCreate(p->context,&output,&desc)==XUI_ERROR_BUSY && output==(xui_widget)(uintptr_t)1);
    if(!p->workflow || !p->replacing){
        memset(&hit,0x5a,sizeof(hit));copy=hit;
        assert(xuiFlowGraphWidgetHitTest(p->canvas,0,0,&hit)==XUI_ERROR_BUSY && memcmp(&hit,&copy,sizeof(hit))==0);
        assert(xuiFlowGraphWidgetGetHoverHit(p->canvas,&hit)==XUI_ERROR_BUSY && memcmp(&hit,&copy,sizeof(hit))==0);
        assert(xuiFlowGraphWidgetSelectAt(p->canvas,0,0,&hit)==XUI_ERROR_BUSY && memcmp(&hit,&copy,sizeof(hit))==0);
    }
    if(p->workflow){
        xui_workflow_desc_t workflowDesc={0};workflowDesc.iSize=sizeof(workflowDesc);
        workflowDesc.pWorkflow=p->workflow;workflowDesc.bOwnWorkflow=1;
        assert(xuiWorkflowWidgetSetWorkflow(p->widget,p->replacementWorkflow,1)==XUI_ERROR_BUSY);
        assert(xuiWorkflowWidgetSetWorkflow(p->peerWorkflow,p->workflow,1)==XUI_ERROR_BUSY);
        assert(xuiWorkflowWidgetCreate(p->context,&output,&workflowDesc)==XUI_ERROR_BUSY && output==(xui_widget)(uintptr_t)1);
        /* A composite's canvas cannot be rebound by an arbitrary public caller. */
        assert(xuiFlowGraphWidgetSetGraph(p->canvas,p->replacement,0)==XUI_ERROR_INVALID_STATE);
    }else assert(xuiFlowGraphWidgetSetGraph(p->canvas,p->replacement,1)==XUI_ERROR_BUSY);
    xrtMemDebugSnapshot(&after);
    assert(after.AllocCount==before.AllocCount && after.FreeCount==before.FreeCount && after.ReallocCount==before.ReallocCount);
    assert(xrtGetError()==ambient);
    if(p->legacy){
        if(!p->workflow || !p->replacing){assert(!xuiFlowGraphWidgetGetGraph(p->canvas));if(ambient)assert(xrtGetError()==ambient);else take_busy();}
        else assert(xuiFlowGraphWidgetGetGraph(p->canvas)==p->replacement);
        if(p->workflow){assert(!xuiWorkflowWidgetGetWorkflow(p->widget));if(ambient)assert(xrtGetError()==ambient);else take_busy();}
    }
}
static void callback(canvas_probe* p)
{
    assert(xuiFlowGraphGetNodeCount(p->graph)==XUI_ERROR_BUSY);
    if(p->retiring)return;
    busy_queries(p);
    if(p->action==CLOSE_WIDGET){
        p->retiring=1;assert(xuiWidgetDestroyChecked(p->widget)==XUI_OK);
        assert(xuiContextOwnershipBusy(p->context));
    }else if(p->action==CLOSE_CONTEXT){
        bool consumed=true;p->retiring=1;
        assert(xuiDestroyOwnerChecked(p->context,&consumed)==XUI_STATUS_DEFERRED && !consumed);
        assert(xuiContextOwnershipBusy(p->context));
    }
}
static bool clone_value(void* raw,void** output,void* user)
{
    canvas_probe* p=user;(void)raw;callback(p);++p->clones;*output=(void*)2;return true;
}
static void drop_value(void* raw,void* user)
{
    canvas_probe* p=user;
    if(raw==(void*)2){if(!p->callerDrop)callback(p);++p->drops;}
}
static xvalue* value(canvas_probe* p)
{
    static const xvaluehandleops ops={clone_value,drop_value,NULL,NULL};void* raw=(void*)1;
    xvalue* result=xrtValueHandleTake(&raw,&ops,p);assert(result && !raw);return result;
}
static void create(canvas_probe* p,int workflow,int legacy)
{
    memset(p,0,sizeof(*p));p->legacy=legacy;assert(xuiCreate(&p->context)==XUI_OK);
    if(workflow){
        assert(xuiWorkflowCreate(&p->workflow)==XUI_OK && xuiWorkflowCreate(&p->replacementWorkflow)==XUI_OK);
        p->graph=xuiWorkflowGetGraph(p->workflow);p->replacement=xuiWorkflowGetGraph(p->replacementWorkflow);
        p->widget=workflow_canvas(p->context,p->workflow,1);p->canvas=xuiWorkflowWidgetGetCanvas(p->widget);
        p->peerWorkflow=workflow_canvas(p->context,p->replacementWorkflow,0);
    }else{
        assert(xuiFlowGraphCreate(&p->graph)==XUI_OK && xuiFlowGraphCreate(&p->replacement)==XUI_OK);
        p->widget=p->canvas=canvas(p->context,p->graph,1);
    }
    add_node(p->graph,"one");add_node(p->replacement,"replacement");p->peer=canvas(p->context,p->replacement,0);
}
static void finish(canvas_probe* p)
{
    bool consumed=false;p->retiring=1;
    assert(xuiDestroyOwnerChecked(p->context,&consumed)==XUI_OK && consumed);
    assert(p->clones==p->drops && !xrtGetError());
    if(p->workflow)xuiWorkflowDestroy(p->replacementWorkflow);
    else assert(xuiFlowGraphDestroyChecked(p->replacement)==XUI_OK);
    assert(!xrtGetError());
}
static void run_case(int workflow,int action)
{
    canvas_probe p;xvalue *input,*copy=NULL;create(&p,workflow,1);p.action=action;input=value(&p);
    assert(xuiFlowGraphSetNodeConfig(p.graph,"one",input)==XUI_OK);
    assert(p.clones==1 && p.drops==0 && p.probes>0);
    if(action==QUERY || action==REPLACE){
        assert(xuiFlowGraphSetNodeConfig(p.graph,"one",input)==XUI_OK);
        assert(xuiFlowGraphGetNodeConfig(p.graph,"one",&copy)==XUI_OK && copy);
        p.callerDrop=1;xrtValueRelease(copy);p.callerDrop=0;
        if(action==REPLACE){
            p.replacing=1;
            if(workflow)assert(xuiWorkflowWidgetSetWorkflow(p.widget,p.replacementWorkflow,0)==XUI_OK);
            else assert(xuiFlowGraphWidgetSetGraph(p.widget,p.replacement,0)==XUI_OK);
            assert(p.clones==p.drops);
        }
    }
    xrtValueRelease(input);finish(&p);
}
static void allocation_failures(int workflow)
{
    int failures=0,successes=0;
    for(int budget=0;budget<32;budget++){
        canvas_probe p;create(&p,workflow,0);xvalue* input=value(&p);
        assert(xrtMemDebugFailAfter(budget));int status=xuiFlowGraphSetNodeConfig(p.graph,"one",input);xrtMemDebugFailClear();
        if(xrtGetError())xrtErrorFree(xrtTakeError());
        if(status==XUI_OK)++successes;else{assert(status==XUI_ERROR_OUT_OF_MEMORY);++failures;}
        assert(xuiFlowGraphGetNodeCount(p.graph)==1);xrtValueRelease(input);finish(&p);
    }
    assert(failures && successes);
}
static void close_during_retired_drop(int workflow,int action)
{
    canvas_probe p;create(&p,workflow,1);xvalue* input=value(&p);
    assert(xuiFlowGraphSetNodeConfig(p.graph,"one",input)==XUI_OK);
    p.action=action;p.replacing=1;
    int status=workflow ? xuiWorkflowWidgetSetWorkflow(p.widget,p.replacementWorkflow,0):
        xuiFlowGraphWidgetSetGraph(p.widget,p.replacement,0);
    /* Legacy setters can report expiry after committing the borrowed input.
     * No input ownership is transferred in this case. The caller retains it. */
    if(action==CLOSE_WIDGET)assert(status<0);
    else assert(status==XUI_OK || status==XUI_ERROR_INVALID_STATE);
    assert(p.clones==p.drops);
    if(xrtGetError())xrtErrorFree(xrtTakeError());
    xrtValueRelease(input);finish(&p);
}
int main(void)
{
    xmemdebugsnapshot before,after;assert(xrtMemDebugEnable(true));xrtMemDebugSnapshot(&before);
    for(int repeat=0;repeat<3;repeat++)for(int workflow=0;workflow<2;workflow++)for(int action=QUERY;action<=CLOSE_CONTEXT;action++)run_case(workflow,action);
    allocation_failures(0);allocation_failures(1);
    for(int workflow=0;workflow<2;workflow++){
        close_during_retired_drop(workflow,CLOSE_WIDGET);
        close_during_retired_drop(workflow,CLOSE_CONTEXT);
    }
    xrtMemDebugSnapshot(&after);
    assert(after.LiveCount==before.LiveCount && after.LiveBytes==before.LiveBytes);
    assert(after.InvalidFreeCount==before.InvalidFreeCount && after.DoubleFreeCount==before.DoubleFreeCount);
    puts("Flow canvas: owning Graph/Workflow, Busy outputs, replacement, checked owner retry, 64 OOM prefixes and heap balance passed");return 0;
}
