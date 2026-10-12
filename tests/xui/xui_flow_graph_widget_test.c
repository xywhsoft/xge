#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_flow_graph_widget_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_flow_context_test_t {
	int iCount;
	int iType;
	int iNode;
	float fX;
	float fY;
} xui_flow_context_test_t;

static int __xuiFlowContext(xui_widget pWidget, const xui_flow_hit_t* pHit,
	float fX, float fY, void* pUser)
{
	xui_flow_context_test_t* pData = (xui_flow_context_test_t*)pUser;
	(void)pWidget;
	if ( pData != NULL && pHit != NULL ) {
		pData->iCount++;
		pData->iType = pHit->iType;
		pData->iNode = pHit->iNode;
		pData->fX = fX;
		pData->fY = fY;
	}
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiFlowContextClick(xui_context pContext, float fX, float fY)
{
	int iRet;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pWidget;
	xui_flow_graph pGraph;
	xui_flow_graph_desc_t tWidgetDesc;
	xui_flow_node_desc_t tNode;
	xui_flow_port_desc_t tPort;
	xui_flow_edge_desc_t tEdge;
	xui_flow_diagnostic_desc_t tDiagnostic;
	xui_flow_viewport_t tViewport;
	xui_flow_hit_t tHit;
	xui_style_property_t arrStyle[2];
	xui_flow_context_test_t tContextMenu;
	xui_surface pCache;
	int iStart;
	int iTool;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pWidget = NULL;
	pGraph = NULL;
	iStart = -1;
	iTool = -1;
	iFailed = 0;
	memset(arrStyle, 0, sizeof(arrStyle));
	memset(&tContextMenu, 0, sizeof(tContextMenu));
	tContextMenu.iType = XUI_FLOW_HIT_NONE;
	tContextMenu.iNode = -1;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 640.0f, 360.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 640.0f, 360.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect");

	iRet = xuiFlowGraphCreate(&pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && pGraph != NULL, "graph create");
	memset(&tViewport, 0, sizeof(tViewport));
	tViewport.iSize = sizeof(tViewport);
	tViewport.fPanX = 8.0f;
	tViewport.fPanY = 12.0f;
	tViewport.fZoom = 1.0f;
	tViewport.fWidth = 640.0f;
	tViewport.fHeight = 360.0f;
	iRet = xuiFlowGraphSetViewport(pGraph, &tViewport);
	XUI_TEST_CHECK(iRet == XUI_OK, "graph viewport");

	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	tNode.sId = "start";
	tNode.sType = "event.start";
	tNode.sTitle = "Start";
	tNode.fX = 40.0f;
	tNode.fY = 50.0f;
	tNode.fW = 150.0f;
	tNode.fH = 80.0f;
	iRet = xuiFlowGraphAddNode(pGraph, &tNode, &iStart);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == 0, "start node");
	tNode.sId = "tool";
	tNode.sType = "action.tool";
	tNode.sTitle = "Tool";
	tNode.fX = 280.0f;
	tNode.fY = 70.0f;
	iRet = xuiFlowGraphAddNode(pGraph, &tNode, &iTool);
	XUI_TEST_CHECK(iRet == XUI_OK && iTool == 1, "tool node");

	memset(&tPort, 0, sizeof(tPort));
	tPort.iSize = sizeof(tPort);
	tPort.sId = "out";
	tPort.sTitle = "out";
	tPort.iDirection = XUI_FLOW_PORT_OUTPUT;
	tPort.iKind = XUI_FLOW_PORT_CONTROL;
	iRet = xuiFlowGraphAddPort(pGraph, iStart, &tPort, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "start output");
	tPort.sId = "in";
	tPort.sTitle = "in";
	tPort.iDirection = XUI_FLOW_PORT_INPUT;
	iRet = xuiFlowGraphAddPort(pGraph, iTool, &tPort, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "tool input");

	memset(&tEdge, 0, sizeof(tEdge));
	tEdge.iSize = sizeof(tEdge);
	tEdge.sId = "edge_1";
	tEdge.iKind = XUI_FLOW_PORT_CONTROL;
	tEdge.sFromNode = "start";
	tEdge.sFromPort = "out";
	tEdge.sToNode = "tool";
	tEdge.sToPort = "in";
	iRet = xuiFlowGraphAddEdge(pGraph, &tEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "edge add");
	iRet = xuiFlowGraphSelectNode(pGraph, "tool", 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "select node");
	iRet = xuiFlowGraphSelectEdge(pGraph, "edge_1", 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "select edge");
	iRet = xuiFlowGraphSetNodeRunState(pGraph, "tool", XUI_WORKFLOW_NODE_RUN_SUCCESS, "done");
	XUI_TEST_CHECK(iRet == XUI_OK, "node result badge state");
	memset(&tDiagnostic, 0, sizeof(tDiagnostic));
	tDiagnostic.iSize = sizeof(tDiagnostic);
	tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
	tDiagnostic.sCode = "test.badge";
	tDiagnostic.sMessage = "Badge diagnostic";
	tDiagnostic.sNode = "tool";
	iRet = xuiFlowGraphAddDiagnostic(pGraph, &tDiagnostic, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "node diagnostic badge state");

	memset(&tWidgetDesc, 0, sizeof(tWidgetDesc));
	tWidgetDesc.iSize = sizeof(tWidgetDesc);
	tWidgetDesc.pGraph = pGraph;
	tWidgetDesc.bOwnGraph = 0;
	iRet = xuiFlowGraphWidgetCreate(pContext, &pWidget, &tWidgetDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pWidget != NULL, "widget create");
	XUI_TEST_CHECK(xuiFlowGraphWidgetGetGraph(pWidget) == pGraph, "widget graph");
	iRet = xuiWidgetSetRect(pWidget, (xui_rect_t){24.0f, 58.0f, 600.0f, 280.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "widget rect");
	iRet = xuiWidgetAddChild(pRoot, pWidget);
	XUI_TEST_CHECK(iRet == XUI_OK, "widget add");
	iRet = xuiFlowGraphWidgetSetContextMenu(pWidget, __xuiFlowContext, &tContextMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "context callback");
	arrStyle[0].iSize = sizeof(arrStyle[0]);
	arrStyle[0].sName = "flowgraph.background.color";
	arrStyle[0].tValue.iSize = sizeof(arrStyle[0].tValue);
	arrStyle[0].tValue.iType = XUI_STYLE_VALUE_COLOR;
	arrStyle[0].tValue.iColor = XUI_COLOR_RGBA(9, 18, 27, 255);
	arrStyle[1].iSize = sizeof(arrStyle[1]);
	arrStyle[1].sName = "flowgraph.grid.color";
	arrStyle[1].tValue.iSize = sizeof(arrStyle[1].tValue);
	arrStyle[1].tValue.iType = XUI_STYLE_VALUE_COLOR;
	arrStyle[1].tValue.iColor = XUI_COLOR_RGBA(44, 55, 66, 120);
	iRet = xuiWidgetSetInlineStyle(pWidget, arrStyle, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "widget inline style");

	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");
	pCache = xuiWidgetGetCacheSurface(pWidget, xuiWidgetGetStateId(pWidget));
	XUI_TEST_CHECK(pCache != NULL, "cache surface");
	XUI_TEST_CHECK(xuiTestSurfaceGetDrawCount(pCache) >= 8, "draw count");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pCache) >= 7, "badge rect fill count");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillColorCount(pCache, XUI_COLOR_RGBA(9, 18, 27, 255)) > 0, "style background draw");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillColorCount(pCache, XUI_COLOR_RGBA(44, 55, 66, 120)) > 0, "style grid draw");
	memset(&tHit, 0, sizeof(tHit));
	tHit.iSize = sizeof(tHit);
	iRet = xuiFlowGraphWidgetHitTest(pWidget, 222.0f, 170.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_PORT && tHit.iNode == iStart, "port hit");
	iRet = xuiFlowGraphWidgetHitTest(pWidget, 334.0f, 158.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_NODE && tHit.iNode == iTool, "node hit");
	iRet = xuiFlowGraphWidgetHitTest(pWidget, 267.0f, 190.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_EDGE && tHit.iEdge == 0, "edge hit");
	iRet = xuiFlowGraphSetEdgeRoute(pGraph, "edge_1", XUI_FLOW_ROUTE_STRAIGHT, 0.0f, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "straight edge route");
	iRet = xuiFlowGraphWidgetHitTest(pWidget, 267.0f, 190.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType != XUI_FLOW_HIT_EDGE, "straight edge route misses orthogonal bend");
	iRet = xuiFlowGraphWidgetHitTest(pWidget, 267.0f, 180.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_EDGE && tHit.iEdge == 0, "straight edge hit");
	iRet = xuiFlowGraphSetEdgeRoute(pGraph, "edge_1", XUI_FLOW_ROUTE_BEZIER, 0.0f, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "bezier edge route");
	iRet = xuiFlowGraphWidgetHitTest(pWidget, 267.0f, 180.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_EDGE && tHit.iEdge == 0, "bezier edge hit");
	iRet = xuiFlowGraphSetEdgeRoute(pGraph, "edge_1", XUI_FLOW_ROUTE_AUTO, 0.0f, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore auto edge route");
	iRet = xuiFlowGraphWidgetHitTest(pWidget, 44.0f, 78.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_BACKGROUND, "background hit");
	iRet = xuiFlowGraphWidgetSelectAt(pWidget, 334.0f, 158.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_NODE && xuiFlowGraphIsNodeSelected(pGraph, "tool"), "select node at point");
	iRet = xuiFlowGraphWidgetSelectAt(pWidget, 267.0f, 190.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_EDGE && xuiFlowGraphIsEdgeSelected(pGraph, "edge_1") && xuiFlowGraphGetSelectedNodeCount(pGraph) == 0, "select edge at point");
	iRet = xuiFlowGraphWidgetGetHoverHit(pWidget, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_NONE, "initial hover none");
	iRet = xuiInputPointerMove(pContext, 334.0f, 158.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hover node move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "hover node dispatch");
	iRet = xuiFlowGraphWidgetGetHoverHit(pWidget, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_NODE && tHit.iNode == iTool, "hover node hit");
	iRet = xuiInputPointerMove(pContext, 267.0f, 190.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hover edge move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "hover edge dispatch");
	iRet = xuiFlowGraphWidgetGetHoverHit(pWidget, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_EDGE && tHit.iEdge == 0, "hover edge hit");
	iRet = xuiInputPointerMove(pContext, 44.0f, 78.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hover background move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "hover background dispatch");
	iRet = xuiFlowGraphWidgetGetHoverHit(pWidget, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_BACKGROUND, "hover background hit");
	iRet = __xuiFlowContextClick(pContext, 334.0f, 158.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && tContextMenu.iCount == 1,
		"node context callback");
	XUI_TEST_CHECK(tContextMenu.iType == XUI_FLOW_HIT_NODE && tContextMenu.iNode == iTool,
		"node context semantics");
	XUI_TEST_CHECK(xuiFlowGraphIsNodeSelected(pGraph, "tool") &&
		tContextMenu.fX == 334.0f && tContextMenu.fY == 158.0f,
		"node context selects target and preserves world anchor");
	xuiTestSurfaceReset(pCache);
	iRet = xuiFlowGraphSelectNode(pGraph, "start", 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "external graph mutation");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "widget revision update");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "rerender after external mutation");
	XUI_TEST_CHECK(xuiTestSurfaceGetDrawCount(pCache) >= 8, "external mutation invalidates cache");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pGraph != NULL ) {
		xuiFlowGraphDestroy(pGraph);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_flow_graph_widget_test passed\n");
	return 0;
}
