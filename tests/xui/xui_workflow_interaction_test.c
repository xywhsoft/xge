#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_workflow_interaction_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiWorkflowInteractionAddNode(xui_flow_graph pGraph, const char* sId, float fX, float fY, int* pIndex)
{
	xui_flow_node_desc_t tNode;

	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	tNode.sId = sId;
	tNode.sType = "mock";
	tNode.sTitle = sId;
	tNode.fX = fX;
	tNode.fY = fY;
	tNode.fW = 150.0f;
	tNode.fH = 80.0f;
	return xuiFlowGraphAddNode(pGraph, &tNode, pIndex);
}

static int __xuiWorkflowInteractionAddPort(xui_flow_graph pGraph, int iNode, const char* sId, int iDirection)
{
	xui_flow_port_desc_t tPort;

	memset(&tPort, 0, sizeof(tPort));
	tPort.iSize = sizeof(tPort);
	tPort.sId = sId;
	tPort.sTitle = sId;
	tPort.iDirection = iDirection;
	tPort.iKind = XUI_FLOW_PORT_CONTROL;
	return xuiFlowGraphAddPort(pGraph, iNode, &tPort, NULL);
}

static int __xuiWorkflowInteractionClick(xui_context pContext, float fX, float fY, uint32_t iModifiers)
{
	int iRet;
	int iReset;

	iRet = xuiInputSetModifiers(pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	iReset = xuiInputSetModifiers(pContext, 0u);
	return (iRet == XUI_OK) ? iReset : iRet;
}

static int __xuiWorkflowInteractionDragEx(xui_context pContext, float fX0, float fY0, float fX1, float fY1, uint32_t iModifiers);

static int __xuiWorkflowInteractionDrag(xui_context pContext, float fX0, float fY0, float fX1, float fY1)
{
	return __xuiWorkflowInteractionDragEx(pContext, fX0, fY0, fX1, fY1, 0u);
}

static int __xuiWorkflowInteractionDragEx(xui_context pContext, float fX0, float fY0, float fX1, float fY1, uint32_t iModifiers)
{
	int iRet;
	int iReset;

	iRet = xuiInputSetModifiers(pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pContext, fX0, fY0, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMove(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	iReset = xuiInputSetModifiers(pContext, 0u);
	return (iRet == XUI_OK) ? iReset : iRet;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pCanvas;
	xui_surface pCache;
	xui_flow_graph pGraph;
	xui_flow_graph_desc_t tCanvasDesc;
	xui_flow_edge_desc_t tEdge;
	xui_flow_edge_info_t tEdgeInfo;
	xui_flow_hit_t tHit;
	xui_flow_node_info_t tNodeInfo;
	int iA;
	int iB;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pCanvas = NULL;
	pCache = NULL;
	pGraph = NULL;
	iA = -1;
	iB = -1;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 500.0f, 260.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root create");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 500.0f, 260.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect");

	iRet = xuiFlowGraphCreate(&pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK, "graph create");
	iRet = __xuiWorkflowInteractionAddNode(pGraph, "a", 40.0f, 60.0f, &iA);
	XUI_TEST_CHECK(iRet == XUI_OK, "node a");
	iRet = __xuiWorkflowInteractionAddNode(pGraph, "b", 250.0f, 80.0f, &iB);
	XUI_TEST_CHECK(iRet == XUI_OK, "node b");
	iRet = __xuiWorkflowInteractionAddPort(pGraph, iA, "out", XUI_FLOW_PORT_OUTPUT);
	XUI_TEST_CHECK(iRet == XUI_OK, "a out");
	iRet = __xuiWorkflowInteractionAddPort(pGraph, iB, "in", XUI_FLOW_PORT_INPUT);
	XUI_TEST_CHECK(iRet == XUI_OK, "b in");
	memset(&tEdge, 0, sizeof(tEdge));
	tEdge.iSize = sizeof(tEdge);
	tEdge.sId = "a_b";
	tEdge.iKind = XUI_FLOW_PORT_CONTROL;
	tEdge.sFromNode = "a";
	tEdge.sFromPort = "out";
	tEdge.sToNode = "b";
	tEdge.sToPort = "in";
	iRet = xuiFlowGraphAddEdge(pGraph, &tEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "edge add");

	memset(&tCanvasDesc, 0, sizeof(tCanvasDesc));
	tCanvasDesc.iSize = sizeof(tCanvasDesc);
	tCanvasDesc.pGraph = pGraph;
	iRet = xuiFlowGraphWidgetCreate(pContext, &pCanvas, &tCanvasDesc);
	XUI_TEST_CHECK(iRet == XUI_OK, "canvas create");
	iRet = xuiWidgetSetRect(pCanvas, (xui_rect_t){0.0f, 0.0f, 500.0f, 260.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "canvas rect");
	iRet = xuiWidgetAddChild(pRoot, pCanvas);
	XUI_TEST_CHECK(iRet == XUI_OK, "canvas add");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial render prepare");
	pCache = xuiWidgetGetCacheSurface(pCanvas, xuiWidgetGetStateId(pCanvas));
	XUI_TEST_CHECK(pCache != NULL, "canvas cache");

	memset(&tHit, 0, sizeof(tHit));
	tHit.iSize = sizeof(tHit);
	iRet = xuiFlowGraphWidgetHitTest(pCanvas, 190.0f, 110.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_PORT && tHit.iNode == iA, "port hit");
	iRet = xuiFlowGraphWidgetHitTest(pCanvas, 280.0f, 100.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_NODE && tHit.iNode == iB, "node hit");
	iRet = xuiFlowGraphWidgetHitTest(pCanvas, 220.0f, 120.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_EDGE, "edge hit");
	iRet = xuiFlowGraphWidgetHitTest(pCanvas, 12.0f, 12.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_FLOW_HIT_BACKGROUND, "background hit");
	iRet = xuiFlowGraphWidgetSelectAt(pCanvas, 280.0f, 100.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphIsNodeSelected(pGraph, "b"), "single select node");
	iRet = xuiFlowGraphWidgetSelectAt(pCanvas, 12.0f, 12.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetSelectedNodeCount(pGraph) == 0 && xuiFlowGraphGetSelectedEdgeCount(pGraph) == 0, "background clears selection");
	iRet = __xuiWorkflowInteractionDrag(pContext, 12.0f, 12.0f, 440.0f, 210.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphIsNodeSelected(pGraph, "a") && xuiFlowGraphIsNodeSelected(pGraph, "b") && xuiFlowGraphGetSelectedNodeCount(pGraph) == 2, "marquee selects nodes");
	iRet = __xuiWorkflowInteractionDragEx(pContext, 240.0f, 70.0f, 440.0f, 190.0f, XUI_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphIsNodeSelected(pGraph, "a") && !xuiFlowGraphIsNodeSelected(pGraph, "b") && xuiFlowGraphGetSelectedNodeCount(pGraph) == 1, "shift marquee toggles nodes");
	iRet = xuiFlowGraphClearSelection(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear after marquee");
	iRet = __xuiWorkflowInteractionClick(pContext, 280.0f, 100.0f, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphIsNodeSelected(pGraph, "b") && xuiFlowGraphGetSelectedNodeCount(pGraph) == 1, "event single select");
	iRet = __xuiWorkflowInteractionClick(pContext, 80.0f, 90.0f, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphIsNodeSelected(pGraph, "a") && xuiFlowGraphIsNodeSelected(pGraph, "b") && xuiFlowGraphGetSelectedNodeCount(pGraph) == 2, "ctrl adds node selection");
	iRet = __xuiWorkflowInteractionClick(pContext, 280.0f, 100.0f, XUI_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphIsNodeSelected(pGraph, "a") && !xuiFlowGraphIsNodeSelected(pGraph, "b") && xuiFlowGraphGetSelectedNodeCount(pGraph) == 1, "shift toggles node selection");
	iRet = __xuiWorkflowInteractionDrag(pContext, 80.0f, 90.0f, 100.0f, 110.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "drag releases capture");
	iRet = xuiFlowGraphGetNode(pGraph, iA, &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 60.0f && tNodeInfo.fY == 80.0f, "node drag updates position");
	XUI_TEST_CHECK(xuiFlowGraphCanUndo(pGraph), "node drag records undo command");
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiFlowGraphCanUndo(pGraph) && xuiFlowGraphCanRedo(pGraph), "node drag undo is one step");
	iRet = xuiFlowGraphGetNode(pGraph, iA, &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 40.0f && tNodeInfo.fY == 60.0f, "node drag undo restores position");
	iRet = xuiFlowGraphRedo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK, "node drag redo");
	iRet = xuiFlowGraphGetNode(pGraph, iA, &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 60.0f && tNodeInfo.fY == 80.0f, "node drag redo restores moved position");
	iRet = __xuiWorkflowInteractionClick(pContext, 280.0f, 100.0f, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphIsNodeSelected(pGraph, "a") && xuiFlowGraphIsNodeSelected(pGraph, "b"), "select second node before multi drag");
	iRet = __xuiWorkflowInteractionDrag(pContext, 100.0f, 110.0f, 115.0f, 125.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetSelectedNodeCount(pGraph) == 2, "multi drag preserves selection");
	iRet = xuiFlowGraphGetNode(pGraph, iA, &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 75.0f && tNodeInfo.fY == 95.0f, "multi drag updates first node");
	iRet = xuiFlowGraphGetNode(pGraph, iB, &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 265.0f && tNodeInfo.fY == 95.0f, "multi drag updates second node");
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi drag undo");
	iRet = xuiFlowGraphGetNode(pGraph, iA, &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 60.0f && tNodeInfo.fY == 80.0f, "multi drag undo restores first node");
	iRet = xuiFlowGraphGetNode(pGraph, iB, &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 250.0f && tNodeInfo.fY == 80.0f, "multi drag undo restores second node");
	iRet = xuiFlowGraphRedo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK, "multi drag redo");
	iRet = xuiFlowGraphGetNode(pGraph, iA, &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 75.0f && tNodeInfo.fY == 95.0f, "multi drag redo restores first node");
	iRet = xuiFlowGraphGetNode(pGraph, iB, &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 265.0f && tNodeInfo.fY == 95.0f, "multi drag redo restores second node");
	iRet = xuiInputPointerDown(pContext, 115.0f, 125.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "cancel node drag down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "cancel node drag down dispatch");
	iRet = xuiInputPointerMove(pContext, 135.0f, 145.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "cancel node drag move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "cancel node drag move dispatch");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "cancel node drag escape");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "cancel node drag releases capture");
	iRet = xuiFlowGraphGetNode(pGraph, iA, &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 75.0f && tNodeInfo.fY == 95.0f, "cancel node drag restores first node");
	iRet = xuiFlowGraphGetNode(pGraph, iB, &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 265.0f && tNodeInfo.fY == 95.0f, "cancel node drag restores second node");
	iRet = xuiInputPointerDown(pContext, 225.0f, 145.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "connection preview down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "connection preview down dispatch");
	xuiTestSurfaceReset(pCache);
	iRet = xuiInputPointerMove(pContext, 225.0f, 145.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "invalid preview move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "invalid preview dispatch");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetLastColor(pCache) == XUI_COLOR_RGBA(214, 72, 72, 210), "invalid connection preview color");
	iRet = xuiInputPointerMove(pContext, 265.0f, 145.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "valid preview move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "valid preview dispatch");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetLastColor(pCache) == XUI_COLOR_RGBA(30, 160, 95, 230), "valid connection preview color");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "cancel connection drag escape");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiFlowGraphGetEdgeCount(pGraph) == 1, "cancel connection drag prevents edge");
	iRet = xuiInputPointerDown(pContext, 225.0f, 145.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "connection drag down after cancel");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "connection drag down after cancel dispatch");
	iRet = xuiInputPointerMove(pContext, 265.0f, 145.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "connection drag move after cancel");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "connection drag move after cancel dispatch");
	iRet = xuiInputPointerUp(pContext, 265.0f, 145.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "connection drag up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetEdgeCount(pGraph) == 2, "port drag creates edge");
	iRet = xuiFlowGraphGetEdge(pGraph, 1, &tEdgeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tEdgeInfo.sFromNode, "a") == 0 && strcmp(tEdgeInfo.sFromPort, "out") == 0 && strcmp(tEdgeInfo.sToNode, "b") == 0 && strcmp(tEdgeInfo.sToPort, "in") == 0, "port drag edge endpoints");
	iRet = xuiFlowGraphClearSelection(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear before delete edge");
	iRet = xuiFlowGraphSelectEdge(pGraph, tEdgeInfo.sId, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "select generated edge before delete");
	iRet = xuiSetFocusWidget(pContext, pCanvas);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus canvas before delete");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DELETE, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "delete edge key down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetEdgeCount(pGraph) == 1 && xuiFlowGraphGetSelectedEdgeCount(pGraph) == 0, "delete key removes selected edge");
	iRet = xuiFlowGraphSelectNode(pGraph, "b", 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "select node before delete");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DELETE, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "delete node key down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "b") < 0 && xuiFlowGraphGetNodeCount(pGraph) == 1 && xuiFlowGraphGetEdgeCount(pGraph) == 0, "delete key removes selected node and incident edges");

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
	printf("xui_workflow_interaction_test passed\n");
	return 0;
}
