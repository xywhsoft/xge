#ifndef XRT_MODULE_ALL
#define XRT_MODULE_ALL
#endif
#include <xui.h>
#include "xui_test_proxy.h"
#include <assert.h>
#include <stdio.h>

static size_t references(xrtownershipref ref)
{
    xrtownershipscope freeze={0};
    size_t count=0;
    assert(ref.Data && xrtOwnershipFreezeTryBegin(&freeze));
    assert(ref.Ops->Count(ref.Data,&count));
    assert(xrtOwnershipScopeEnd(&freeze));
    return count;
}

static void render_color(xui_context context,xui_widget root,xui_widget canvas,uint32_t color)
{
    assert(xuiSetViewportSize(context,240,150)==XUI_OK);
    assert(xuiSetRootWidget(context,root)==XUI_OK);
    assert(xuiWidgetSetRect(root,(xui_rect_t){0,0,240,150})==XUI_OK);
    assert(xuiLayout(context)==XUI_OK);
    assert(xuiRenderPrepare(context)==XUI_OK);
    xui_surface cache=xuiWidgetGetCacheSurface(canvas,xuiWidgetGetStateId(canvas));
    assert(cache && xuiTestSurfaceGetRectFillColorCount(cache,color)>0);
}

static void sweep(xui_context context,const xui_flow_graph_desc_t* flow,const xui_workflow_desc_t* workflow)
{
    unsigned failures=0,successes=0;
    xrtownershipref model=flow ? xuiFlowGraphOwnership(flow->pGraph):xuiWorkflowOwnership(workflow->pWorkflow);
    for(size_t budget=0;budget<48;++budget){
        xui_widget widget=NULL;
        assert(xrtMemDebugFailAfter(budget));
        int status=flow ? xuiFlowGraphWidgetCreateShared(context,&widget,flow):
            xuiWorkflowWidgetCreateShared(context,&widget,workflow);
        xrtMemDebugFailClear();
        xrtClearError();
        if(status==XUI_OK){
            ++successes;
            assert(widget && references(model)==2);
            assert(xuiWidgetDestroyChecked(widget)==XUI_OK);
        }else{
            ++failures;
            assert(!widget);
        }
        assert(references(model)==1);
    }
    assert(failures && successes);
}

typedef struct {xui_context context; const xui_flow_graph_desc_t* flow; const xui_workflow_desc_t* workflow;} thread_inputs;

static int wrong_thread(void* raw)
{
    const thread_inputs* inputs=raw;
    xui_widget output=(xui_widget)(uintptr_t)1;
    assert(xuiFlowGraphWidgetCreateShared(inputs->context,&output,inputs->flow)==XUI_ERROR_BUSY && !output);
    output=(xui_widget)(uintptr_t)1;
    assert(xuiWorkflowWidgetCreateShared(inputs->context,&output,inputs->workflow)==XUI_ERROR_BUSY && !output);
    return 0;
}

int main(void)
{
    assert(xrtMemDebugEnable(true));
    xui_test_proxy_state_t proxy;
    xuiTestProxyInit(&proxy);
    xui_context context=NULL;
    assert(xuiCreate(&context)==XUI_OK && xuiSetProxy(context,&proxy.tProxy)==XUI_OK);
    xui_flow_graph graph=NULL;
    xui_workflow model=NULL;
    assert(xuiFlowGraphCreateOwned(&graph)==XUI_OK && xuiWorkflowCreateOwned(&model)==XUI_OK);
    xui_flow_graph_desc_t flow={.iSize=sizeof(flow),.pGraph=graph,.iBackgroundColor=0x2c1207ff};
    xui_workflow_desc_t workflow={.iSize=sizeof(workflow),.pWorkflow=model,
        .tGraph={.iSize=sizeof(workflow.tGraph),.iBackgroundColor=0x14562aff}};

    xui_widget widget=NULL;
    assert(xuiFlowGraphWidgetCreateShared(context,&widget,&flow)==XUI_OK);
    assert(xuiFlowGraphWidgetGetGraph(widget)==graph && references(xuiFlowGraphOwnership(graph))==2);
    render_color(context,widget,widget,flow.iBackgroundColor);
    assert(xuiWidgetDestroyChecked(widget)==XUI_OK && references(xuiFlowGraphOwnership(graph))==1);
    assert(xuiWorkflowWidgetCreateShared(context,&widget,&workflow)==XUI_OK);
    assert(xuiWorkflowWidgetGetWorkflow(widget)==model && references(xuiWorkflowOwnership(model))==2);
    render_color(context,widget,xuiWorkflowWidgetGetCanvas(widget),workflow.tGraph.iBackgroundColor);
    assert(xuiWidgetDestroyChecked(widget)==XUI_OK && references(xuiWorkflowOwnership(model))==1);

    sweep(context,&flow,NULL);
    sweep(context,NULL,&workflow);
    xmemdebugsnapshot before={0},after={0};
    xrtMemDebugSnapshot(&before);
    sweep(context,&flow,NULL);
    sweep(context,NULL,&workflow);
    xrtMemDebugSnapshot(&after);
    assert(after.LiveCount==before.LiveCount && after.LiveBytes==before.LiveBytes);
    assert(xuiFlowGraphWidgetCreate(context,&widget,&flow)==XUI_OK);
    assert(references(xuiFlowGraphOwnership(graph))==1 && xuiWidgetDestroyChecked(widget)==XUI_OK);
    assert(xuiWorkflowWidgetCreate(context,&widget,&workflow)==XUI_OK);
    assert(references(xuiWorkflowOwnership(model))==1 && xuiWidgetDestroyChecked(widget)==XUI_OK);
    assert(xuiFlowGraphWidgetCreateShared(context,&widget,NULL)==XUI_OK);
    assert(xuiFlowGraphIsOwned(xuiFlowGraphWidgetGetGraph(widget)) && xuiWidgetDestroyChecked(widget)==XUI_OK);
    assert(xuiWorkflowWidgetCreateShared(context,&widget,NULL)==XUI_OK);
    assert(xuiWorkflowIsOwned(xuiWorkflowWidgetGetWorkflow(widget)) && xuiWidgetDestroyChecked(widget)==XUI_OK);
    assert(xuiDestroyChecked(context)==XUI_OK);

    xui_proxy owned=NULL;
    xui_context_desc_t desc={.iSize=sizeof(desc),.iSystemFontPolicy=XUI_SYSTEM_FONT_DISABLED};
    assert(xuiProxyXgeCreate(&owned)==XUI_OK && xuiCreateOwnedEx(owned,&desc,&context)==XUI_OK);
    const thread_inputs inputs={context,&flow,&workflow};
    xthread* worker=xrtThreadCreate(wrong_thread,(void*)&inputs,0);
    assert(worker && xrtThreadWaitFor(worker,5000)==XWAIT_OK);
    xrtThreadDestroy(worker);
    assert(xuiDestroyChecked(context)==XUI_OK && xuiProxyReleaseChecked(owned)==XUI_OK);
    assert(xuiFlowGraphReleaseChecked(graph)==XUI_OK && xuiWorkflowReleaseChecked(model)==XUI_OK);
    puts("xui_canvas_create_shared_test passed");
    return 0;
}
