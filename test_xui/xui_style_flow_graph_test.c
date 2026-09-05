#include "xui.h"
#include "xui_test_proxy.h"
#include <stdio.h>
#include <string.h>

#define CHECK(e,m) do { if(!(e)) { printf("flow graph style: %s (%d)\n",m,__LINE__); failed=1; goto cleanup; } } while(0)
static uint32_t seen[4096];
static int count;
static int (*baseFill)(xui_proxy,xui_draw_context,xui_rect_t,uint32_t);
static int (*baseLine)(xui_proxy,xui_draw_context,float,float,float,float,float,uint32_t);
static int (*baseText)(xui_proxy,xui_draw_context,xui_font,const char*,xui_rect_t,uint32_t,uint32_t);
static int has(uint32_t c) { int i; for(i=0;i<count;i++) if(seen[i]==c) return 1; return 0; }
static int fill(xui_proxy p,xui_draw_context d,xui_rect_t r,uint32_t c)
{ if(count<4096) seen[count++]=c; return baseFill(p,d,r,c); }
static int line(xui_proxy p,xui_draw_context d,float x,float y,float x2,float y2,float w,uint32_t c)
{ if(count<4096) seen[count++]=c; return baseLine(p,d,x,y,x2,y2,w,c); }
static int text(xui_proxy p,xui_draw_context d,xui_font f,const char* s,xui_rect_t r,uint32_t c,uint32_t flags)
{ if(count<4096) seen[count++]=c; return baseText(p,d,f,s,r,c,flags); }
int main(void)
{
	static const char* const newKeys[]={
		"flowgraph.node.hover_color",
		"flowgraph.node.running_color",
		"flowgraph.node.success_color",
		"flowgraph.node.failed_color",
		"flowgraph.node.warning_color",
		"flowgraph.node.diagnostic_border_color",
		"flowgraph.node.disabled_border_color",
		"flowgraph.node.disabled_color",
		"flowgraph.run.queued_color",
		"flowgraph.run.running_color",
		"flowgraph.run.success_color",
		"flowgraph.run.failed_color",
		"flowgraph.run.skipped_color",
		"flowgraph.run.warning_color",
		"flowgraph.badge.color",
		"flowgraph.badge.text_color",
		"flowgraph.badge.success_color",
		"flowgraph.badge.failed_color",
		"flowgraph.badge.skipped_color",
		"flowgraph.badge.warning_color",
		"flowgraph.badge.warning_text_color",
		"flowgraph.edge.active_color",
		"flowgraph.edge.taken_color",
		"flowgraph.edge.skipped_color",
		"flowgraph.edge.invalid_color",
		"flowgraph.edge.hover_color",
		"flowgraph.connection.valid_color",
		"flowgraph.connection.invalid_color",
		"flowgraph.selection.color",
		"flowgraph.selection.border_color",
		"flowgraph.port.disabled_color",
		"flowgraph.port.hover_color",
		"flowgraph.text.color",
		"flowgraph.text.disabled_color",
		"flowgraph.diagnostic.badge_color",
		"flowgraph.diagnostic.text_color",
		"flowgraph.text.summary_color",
	};
	static const char* const keys[]={"flowgraph.node.color","flowgraph.edge.color","flowgraph.node.success_color",
		"flowgraph.badge.success_color","flowgraph.text.color","flowgraph.text.summary_color",
		"flowgraph.node.disabled_color","flowgraph.text.disabled_color","flowgraph.edge.taken_color",
		"flowgraph.diagnostic.badge_color","flowgraph.diagnostic.text_color"};
	xui_test_proxy_state_t proxy;
	xui_context ctx=NULL;
	xui_flow_graph graph=NULL;
	xui_widget widget=NULL;
	xui_font font=NULL;
	xui_flow_node_desc_t node;
	xui_flow_port_desc_t port;
	xui_flow_edge_desc_t edge;
	xui_flow_graph_desc_t desc;
	xui_flow_diagnostic_desc_t diagnostic;
	xui_style_property_t props[11];
	uint32_t revision;
	int i,a,b,failed=0;
	xuiTestProxyInit(&proxy);
	baseFill=proxy.tProxy.drawRectFill; proxy.tProxy.drawRectFill=fill;
	baseLine=proxy.tProxy.drawLine; proxy.tProxy.drawLine=line;
	baseText=proxy.tProxy.drawText; proxy.tProxy.drawText=text;
	CHECK(xuiCreate(&ctx)==XUI_OK && xuiSetProxy(ctx,&proxy.tProxy)==XUI_OK,"context");
	CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy,&font,"test.ttf",14,0)==XUI_OK,"font");
	CHECK(xuiSetDefaultFont(ctx,font)==XUI_OK && xuiInputViewport(ctx,640,400)==XUI_OK,"viewport");
	CHECK(xuiFlowGraphCreate(&graph)==XUI_OK,"graph");
	memset(&node,0,sizeof(node)); node.iSize=sizeof(node); node.sId="a"; node.sType="start"; node.sTitle="Node";
	node.fX=30; node.fY=30; node.fW=160; node.fH=100;
	CHECK(xuiFlowGraphAddNode(graph,&node,&a)==XUI_OK,"node a");
	node.sId="b"; node.fX=320;
	CHECK(xuiFlowGraphAddNode(graph,&node,&b)==XUI_OK,"node b");
	memset(&port,0,sizeof(port)); port.iSize=sizeof(port); port.sId="out"; port.iDirection=XUI_FLOW_PORT_OUTPUT; port.iKind=XUI_FLOW_PORT_CONTROL;
	CHECK(xuiFlowGraphAddPort(graph,a,&port,NULL)==XUI_OK,"output");
	port.sId="in"; port.iDirection=XUI_FLOW_PORT_INPUT;
	CHECK(xuiFlowGraphAddPort(graph,b,&port,NULL)==XUI_OK,"input");
	memset(&edge,0,sizeof(edge)); edge.iSize=sizeof(edge); edge.sId="edge"; edge.sFromNode="a"; edge.sFromPort="out";
	edge.sToNode="b"; edge.sToPort="in"; edge.iKind=XUI_FLOW_PORT_CONTROL;
	CHECK(xuiFlowGraphAddEdge(graph,&edge,NULL)==XUI_OK,"edge");
	CHECK(xuiFlowGraphSetNodeSummary(graph,"b","Summary")==XUI_OK,"summary");
	memset(&desc,0,sizeof(desc)); desc.iSize=sizeof(desc); desc.pGraph=graph; desc.iNodeColor=0xa1b2c3ff;
	CHECK(xuiFlowGraphWidgetCreate(ctx,&widget,&desc)==XUI_OK,"widget");
	CHECK(xuiSetRootWidget(ctx,widget)==XUI_OK && xuiWidgetSetRect(widget,(xui_rect_t){0,0,640,400})==XUI_OK,"root");
	CHECK(xuiRenderPrepare(ctx)==XUI_OK && has(desc.iNodeColor),"initial API color");
	for(i=0;i<(int)(sizeof(newKeys)/sizeof(newKeys[0]));i++) CHECK(xuiStyleFindProperty(ctx,newKeys[i])!=0,newKeys[i]);
	for(i=0;i<11;i++) {
		memset(&props[i],0,sizeof(props[i])); props[i].iSize=sizeof(props[i]); props[i].sName=keys[i];
		props[i].tValue.iSize=sizeof(props[i].tValue); props[i].tValue.iType=XUI_STYLE_VALUE_COLOR;
		props[i].tValue.iColor=0x361725ffu+((uint32_t)i<<16);
	}
	revision=xuiFlowGraphGetRevision(graph);
	CHECK(xuiStyleSetDefault(ctx,props,11)==XUI_OK,"global stylesheet");
	count=0;
	CHECK(xuiRenderPrepare(ctx)==XUI_OK,"global render");
	CHECK(has(props[0].tValue.iColor) && has(props[1].tValue.iColor),"existing node and edge paint caches refresh");
	CHECK(has(props[4].tValue.iColor) && has(props[5].tValue.iColor),"node title and summary colors");
	CHECK(xuiFlowGraphGetRevision(graph)==revision,"theme does not alter graph revision");
	CHECK(xuiFlowGraphSetNodeRunState(graph,"b",XUI_WORKFLOW_NODE_RUN_SUCCESS,"done")==XUI_OK,"success");
	CHECK(xuiFlowGraphSetEdgeRunState(graph,"edge",XUI_WORKFLOW_EDGE_RUN_TAKEN,"")==XUI_OK,"taken");
	CHECK(xuiUpdate(ctx,0)==XUI_OK,"graph update"); count=0;
	CHECK(xuiRenderPrepare(ctx)==XUI_OK && has(props[2].tValue.iColor) && has(props[3].tValue.iColor) && has(props[8].tValue.iColor),"run and badge styles");
	props[0].tValue.iColor=0x893645ff; props[3].tValue.iColor=0x973526ff;
	CHECK(xuiStyleSetDefault(ctx,props,11)==XUI_OK,"second palette"); count=0;
	CHECK(xuiRenderPrepare(ctx)==XUI_OK && has(props[0].tValue.iColor) && has(props[3].tValue.iColor),"live run-state cache refresh");
	CHECK(xuiWidgetSetEnabled(widget,0)==XUI_OK,"disable"); count=0;
	CHECK(xuiRenderPrepare(ctx)==XUI_OK && has(props[6].tValue.iColor) && has(props[7].tValue.iColor),"disabled styles");
	CHECK(xuiWidgetSetEnabled(widget,1)==XUI_OK,"enable");
	memset(&diagnostic,0,sizeof(diagnostic)); diagnostic.iSize=sizeof(diagnostic); diagnostic.sNode="a";
	diagnostic.iSeverity=XUI_CODE_DIAGNOSTIC_ERROR; diagnostic.sMessage="Error";
	CHECK(xuiFlowGraphAddDiagnostic(graph,&diagnostic,NULL)==XUI_OK && xuiUpdate(ctx,0)==XUI_OK,"diagnostic"); count=0;
	CHECK(xuiRenderPrepare(ctx)==XUI_OK && has(props[9].tValue.iColor) && has(props[10].tValue.iColor),"diagnostic badge theme");
	props[0].tValue.iColor=0;
	CHECK(xuiStyleSetDefault(ctx,props,11)==XUI_OK,"transparent"); count=0;
	CHECK(xuiRenderPrepare(ctx)==XUI_OK && has(0) && !has(0x893645ff),"transparent is not replaced with default");
	CHECK(xuiStyleClearDefault(ctx)==XUI_OK,"clear theme"); count=0;
	CHECK(xuiRenderPrepare(ctx)==XUI_OK && has(desc.iNodeColor),"clear restores API paint and cache");
	puts("flow graph global style colors passed");
cleanup:
	if(ctx) xuiDestroy(ctx);
	if(graph) xuiFlowGraphDestroy(graph);
	if(font) proxy.tProxy.fontDestroy(&proxy.tProxy,font);
	return failed;
}
