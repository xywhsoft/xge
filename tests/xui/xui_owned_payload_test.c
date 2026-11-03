#include "src/xui/core/xui_internal.h"
#include <assert.h>
#include <stdio.h>

/* The small CPU-only lifetime gate does not link the popup implementation. */
xui_widget xuiInternalPopupPointerPassthroughOwner(xui_context c,xui_widget w,float x,float y)
{(void)c;(void)x;(void)y;return w;}
int xuiInternalPopupPrepareOwnerPress(xui_context c,xui_widget w,float x,float y)
{(void)c;(void)w;(void)x;(void)y;return XUI_OK;}

typedef struct Counter { unsigned edges, stop; } Counter;
static bool visit(xrtownershipref ref,void* raw)
{Counter* count=raw;assert(ref.Data && ref.Ops);++count->edges;return !count->stop || count->edges<count->stop;}
static unsigned graph_edges(xui_flow_graph graph)
{
    Counter count={0};xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiFlowGraphTraceOwnedValues(graph,visit,&count));assert(xrtOwnershipScopeEnd(&freeze));return count.edges;
}
static void add_node(xui_flow_graph graph,const char* id)
{xui_flow_node_desc_t node={0};node.iSize=sizeof(node);node.sId=id;assert(xuiFlowGraphAddNode(graph,&node,NULL)==XUI_OK);}
static void graph_values(void)
{
    xui_flow_graph graph=NULL;assert(xuiFlowGraphCreate(&graph)==XUI_OK);add_node(graph,"node");
    xvalue* first=xrtValueInt(111);xvalue* second=xrtValueInt(222);assert(first && second);
    assert(xuiFlowGraphSetNodeConfig(graph,"node",first)==XUI_OK && graph_edges(graph)==1);
    assert(xuiFlowGraphCommandSetNodeConfig(graph,"node",second)==XUI_OK && graph_edges(graph)==3);
    assert(xuiFlowGraphUndo(graph)==XUI_OK && graph_edges(graph)==3);
    assert(xuiFlowGraphRedo(graph)==XUI_OK && graph_edges(graph)==3);
    assert(xuiFlowGraphSelectNode(graph,"node",1)==XUI_OK && xuiFlowGraphDeleteSelection(graph)==XUI_OK);
    assert(xuiFlowGraphGetNodeCount(graph)==0 && graph_edges(graph)==3);
    assert(xuiFlowGraphUndo(graph)==XUI_OK && graph_edges(graph)==4);
    Counter stopped={0,2};xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(!xuiFlowGraphTraceOwnedValues(graph,visit,&stopped) && stopped.edges==2);
    assert(!xuiFlowGraphTraceOwnedValues(NULL,visit,&stopped) && !xuiFlowGraphTraceOwnedValues(graph,NULL,&stopped));
    assert(xrtOwnershipScopeEnd(&freeze));
    xerror* prior=xrtErrorCreate(XERR_RANGE,"test",0,"preserve");assert(prior);xrtSetErrorTake(prior);
    xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);assert(xrtMemDebugFailAfter(0));
    for(unsigned i=0;i<200;++i)assert(graph_edges(graph)==4 && xrtGetError()==prior);
    assert(xrtMemDebugFailAfter(-1));xrtMemDebugSnapshot(&after);
    assert(before.AllocCount==after.AllocCount && before.FreeCount==after.FreeCount && before.ReallocCount==after.ReallocCount);
    xrtClearError();xrtValueRelease(first);xrtValueRelease(second);xuiFlowGraphDestroy(graph);
}
static void workflow_values(void)
{
    xui_workflow workflow=NULL;assert(xuiWorkflowCreate(&workflow)==XUI_OK);
    xvalue* schema=xrtValueObject();xvalue* number=xrtValueInt(99);assert(schema && number);
    xui_workflow_node_type_desc_t type={0};type.iSize=sizeof(type);type.sId="type";type.iVersion=1;type.pConfigSchema=schema;
    assert(xuiWorkflowRegisterNodeType(workflow,&type,NULL)==XUI_OK);
    xui_workflow_variable_desc_t variable={0};variable.iSize=sizeof(variable);variable.sId="v";variable.sType="int";variable.pDefaultValue=number;
    assert(xuiWorkflowAddVariable(workflow,&variable,NULL)==XUI_OK);
    xui_flow_graph graph=xuiWorkflowGetGraph(workflow);add_node(graph,"node");
    assert(xuiFlowGraphSetNodeConfig(graph,"node",number)==XUI_OK);
    Counter count={0};xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiWorkflowTraceOwnedValues(workflow,visit,&count) && count.edges==3);
    count=(Counter){0,2};assert(!xuiWorkflowTraceOwnedValues(workflow,visit,&count) && count.edges==2);
    assert(!xuiWorkflowTraceOwnedValues(NULL,visit,&count) && !xuiWorkflowTraceOwnedValues(workflow,NULL,&count));
    assert(xrtOwnershipScopeEnd(&freeze));xrtValueRelease(schema);xrtValueRelease(number);xuiWorkflowDestroy(workflow);
}
typedef struct Payload { xvalue* base; xvalue* derived; } Payload;
static int base_init(xui_widget widget,void* data,const void* input,void* user)
{(void)widget;(void)input;(void)user;((Payload*)data)->base=xrtValueInt(11);return ((Payload*)data)->base?XUI_OK:XUI_ERROR_OUT_OF_MEMORY;}
static int derived_init(xui_widget widget,void* data,const void* input,void* user)
{(void)widget;(void)input;(void)user;((Payload*)data)->derived=xrtValueInt(22);return ((Payload*)data)->derived?XUI_OK:XUI_ERROR_OUT_OF_MEMORY;}
static void base_drop(xui_widget widget,void* data,void* user)
{(void)widget;(void)user;xrtValueRelease(((Payload*)data)->base);}
static void derived_drop(xui_widget widget,void* data,void* user)
{(void)widget;(void)user;xrtValueRelease(((Payload*)data)->derived);}
static bool base_trace(xui_widget widget,const void* data,xrtownershipvisitor visitor,void* user)
{(void)widget;return visitor(xrtValueOwnership(((const Payload*)data)->base),user);}
static bool derived_trace(xui_widget widget,const void* data,xrtownershipvisitor visitor,void* user)
{(void)widget;return visitor(xrtValueOwnership(((const Payload*)data)->derived),user);}
static void native_types(void)
{
    xui_context context=NULL;assert(xuiCreate(&context)==XUI_OK);xui_widget_type parent=NULL,child=NULL;
    xui_widget_type_desc_t desc={0};desc.iSize=sizeof(desc);desc.sName="owner-base";desc.iTypeDataSize=sizeof(Payload);
    desc.onInit=base_init;desc.onDestroy=base_drop;assert(xuiWidgetRegisterType(context,&parent,&desc)==XUI_OK);
    assert(xuiWidgetTypeSetOwnershipTrace(parent,base_trace)==XUI_OK);
    desc.sName="owner-child";desc.pParent=parent;desc.onInit=derived_init;desc.onDestroy=derived_drop;
    assert(xuiWidgetRegisterType(context,&child,&desc)==XUI_OK && xuiWidgetTypeSetOwnershipTrace(child,derived_trace)==XUI_OK);
    xui_widget widget=NULL;assert(xuiWidgetCreateTyped(context,child,&widget,NULL)==XUI_OK);
    assert(xuiWidgetGetParent(widget)==NULL); /* Unattached still belongs to context. */
    assert(xuiWidgetTypeSetOwnershipTrace(parent,NULL)==XUI_ERROR_ALREADY_INITIALIZED);
    assert(xuiWidgetTypeSetOwnershipTrace(child,NULL)==XUI_ERROR_ALREADY_INITIALIZED);
    assert(xuiWidgetTypeSetOwnershipTrace(xuiWidgetGetBaseType(),NULL)==XUI_ERROR_INVALID_ARGUMENT);
    Counter count={0};xrtownershipscope freeze={0};assert(xrtOwnershipFreezeTryBegin(&freeze));
    assert(xuiContextTraceEventOwners(context,visit,&count) && count.edges==0);
    assert(xuiContextTraceOwnership(context,visit,&count) && count.edges==2);
    count=(Counter){0,1};assert(!xuiContextTraceOwnership(context,visit,&count) && count.edges==1);
    assert(xrtOwnershipScopeEnd(&freeze));
    xui_call_scope_t call={0};assert(xuiCallScopeBegin(context,&call)==XUI_OK);
    assert(!xuiContextTraceOwnership(context,visit,&count) && count.edges==1);
    assert(xuiCallScopeEnd(&call)==XUI_OK);xuiWidgetDestroy(widget);
    assert(xuiWidgetTypeSetOwnershipTrace(parent,NULL)==XUI_OK);xuiDestroy(context);
}
int main(void)
{
    assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));xmemdebugsnapshot before,after;xrtMemDebugSnapshot(&before);
    graph_values();workflow_values();native_types();xrtClearError();xrtMemDebugSnapshot(&after);
    assert(before.LiveCount==after.LiveCount && before.LiveBytes==after.LiveBytes &&
        before.InvalidFreeCount==after.InvalidFreeCount && before.DoubleFreeCount==after.DoubleFreeCount);
    puts("native owned payload: node/history/group edges, workflow schema/defaults, type inheritance, refusal and allocation-free tracing passed");return 0;
}
