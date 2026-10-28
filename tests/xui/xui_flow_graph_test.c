#include "xui.h"
#include "src/xui/core/xui_xrt_port.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_flow_graph_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiFlowGraphTestHasPort(xui_flow_graph pGraph, int iNode, const char* sId)
{
	xui_flow_port_info_t tPort;
	int i;

	for ( i = 0; i < xuiFlowGraphGetNodePortCount(pGraph, iNode); ++i ) {
		memset(&tPort, 0, sizeof(tPort));
		tPort.iSize = sizeof(tPort);
		if ( xuiFlowGraphGetPort(pGraph, iNode, i, &tPort) == XUI_OK &&
		     tPort.sId != NULL && strcmp(tPort.sId, sId) == 0 ) {
			return 1;
		}
	}
	return 0;
}

int main(void)
{
	xui_flow_graph pGraph;
	xui_flow_graph pAuxGraph;
	xui_flow_node_desc_t tNode;
	xui_flow_port_desc_t tPort;
	xui_flow_port_desc_t arrDynamicPorts[2];
	xui_flow_edge_desc_t tEdge;
	xui_flow_move_node_desc_t tMove;
	xui_flow_move_node_record_t tMoveRecord;
	xui_flow_viewport_t tViewport;
	xui_flow_diagnostic_desc_t tDiagnostic;
	xui_flow_node_info_t tNodeInfo;
	xui_flow_edge_info_t tEdgeInfo;
	xvalue* pConfig;
	xvalue* pConfig2;
	xvalue* pGotConfig;
	int iStart;
	int iEnd;
	int iPort;
	int iEdge;
	int iRet;
	int iFailed;
	uint32_t iRevision;
	char sId[64];
	char sTinyId[4];
	float fX;
	float fY;

	pGraph = NULL;
	pAuxGraph = NULL;
	iStart = -1;
	iEnd = -1;
	iFailed = 0;
	pConfig = NULL;
	pConfig2 = NULL;
	pGotConfig = NULL;

	iRet = xuiFlowGraphCreate(&pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && pGraph != NULL, "create graph");
	iRevision = xuiFlowGraphGetRevision(pGraph);
	XUI_TEST_CHECK(!xuiFlowGraphIsDirty(pGraph), "new graph is clean");
	iRet = xuiFlowGraphMarkClean(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetCleanRevision(pGraph) == iRevision, "mark clean");
	XUI_TEST_CHECK(xuiFlowGraphGetNodeCount(pGraph) == 0, "empty node count");
	XUI_TEST_CHECK(xuiFlowGraphGetEdgeCount(pGraph) == 0, "empty edge count");
	memset(&tViewport, 0, sizeof(tViewport));
	tViewport.iSize = sizeof(tViewport);
	tViewport.fPanX = 10.0f;
	tViewport.fPanY = 20.0f;
	tViewport.fZoom = 2.0f;
	tViewport.fWidth = 800.0f;
	tViewport.fHeight = 600.0f;
	iRet = xuiFlowGraphSetViewport(pGraph, &tViewport);
	XUI_TEST_CHECK(iRet == XUI_OK, "set viewport");
	XUI_TEST_CHECK(xuiFlowGraphGetRevision(pGraph) > iRevision, "viewport bumps revision");
	XUI_TEST_CHECK(xuiFlowGraphIsDirty(pGraph), "viewport marks graph dirty");
	iRevision = xuiFlowGraphGetRevision(pGraph);
	iRet = xuiFlowGraphMarkClean(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiFlowGraphIsDirty(pGraph), "mark viewport revision clean");
	iRet = xuiFlowGraphWorldToScreen(pGraph, 5.0f, 7.0f, &fX, &fY);
	XUI_TEST_CHECK(iRet == XUI_OK && fX == 30.0f && fY == 54.0f, "world to screen");
	iRet = xuiFlowGraphScreenToWorld(pGraph, fX, fY, &fX, &fY);
	XUI_TEST_CHECK(iRet == XUI_OK && fX == 5.0f && fY == 7.0f, "screen to world");

	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	tNode.sId = "n_start";
	tNode.sType = "start";
	tNode.sTitle = "Start";
	tNode.fX = 80.0f;
	tNode.fY = 120.0f;
	iRet = xuiFlowGraphAddNode(pGraph, &tNode, &iStart);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == 0, "add start node");
	XUI_TEST_CHECK(xuiFlowGraphGetRevision(pGraph) > iRevision, "add node bumps revision");
	iRevision = xuiFlowGraphGetRevision(pGraph);
	XUI_TEST_CHECK(xuiFlowGraphFindNode(pGraph, "n_start") == iStart, "find start node");
	iRet = xuiFlowGraphSetNodePosition(pGraph, "n_start", 96.0f, 144.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set node position");
	XUI_TEST_CHECK(xuiFlowGraphGetRevision(pGraph) > iRevision, "move node bumps revision");
	iRevision = xuiFlowGraphGetRevision(pGraph);
	iRet = xuiFlowGraphGetNode(pGraph, iStart, &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 96.0f && tNodeInfo.fY == 144.0f, "node position updated");

	iRet = xuiFlowGraphAddNode(pGraph, &tNode, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_ALREADY_INITIALIZED, "duplicate node rejected");
	pConfig = xrtValueObject();
	XUI_TEST_CHECK(pConfig != NULL, "create node config");
	xuiXrtValueObjectSetText(pConfig, "prompt", 6, "hello", 5);
	iRet = xuiFlowGraphCommandSetNodeConfig(pGraph, "n_start", pConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphCanUndo(pGraph), "command set node config");
	iRet = xuiFlowGraphGetNodeConfig(pGraph, "n_start", &pGotConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && pGotConfig != NULL && strcmp(xuiXrtValueObjectGetText(pGotConfig, "prompt", 6), "hello") == 0, "get node config");
	xuiXrtValueObjectSetText(pGotConfig, "prompt", 6, "mutated", 7);
	xrtValueRelease(pGotConfig);
	pGotConfig = NULL;
	iRet = xuiFlowGraphGetNodeConfig(pGraph, "n_start", &pGotConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && pGotConfig != NULL && strcmp(xuiXrtValueObjectGetText(pGotConfig, "prompt", 6), "hello") == 0,
		"node config getter returns isolated copy");
	xrtValueRelease(pGotConfig);
	pGotConfig = NULL;
	pConfig2 = xrtValueObject();
	XUI_TEST_CHECK(pConfig2 != NULL, "create second node config");
	xuiXrtValueObjectSetText(pConfig2, "prompt", 6, "world", 5);
	iRet = xuiFlowGraphCommandSetNodeConfig(pGraph, "n_start", pConfig2);
	XUI_TEST_CHECK(iRet == XUI_OK, "command replace node config");
	iRet = xuiFlowGraphGetNodeConfig(pGraph, "n_start", &pGotConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && pGotConfig != NULL && strcmp(xuiXrtValueObjectGetText(pGotConfig, "prompt", 6), "world") == 0, "replace node config applied");
	xrtValueRelease(pGotConfig);
	pGotConfig = NULL;
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK, "undo node config");
	iRet = xuiFlowGraphGetNodeConfig(pGraph, "n_start", &pGotConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && pGotConfig != NULL && strcmp(xuiXrtValueObjectGetText(pGotConfig, "prompt", 6), "hello") == 0, "undo node config restores previous value");
	xrtValueRelease(pGotConfig);
	pGotConfig = NULL;
	iRet = xuiFlowGraphRedo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK, "redo node config");
	iRet = xuiFlowGraphGetNodeConfig(pGraph, "n_start", &pGotConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && pGotConfig != NULL && strcmp(xuiXrtValueObjectGetText(pGotConfig, "prompt", 6), "world") == 0, "redo node config restores new value");
	xrtValueRelease(pGotConfig);
	pGotConfig = NULL;
	iRet = xuiFlowGraphCommandSetNodeConfig(pGraph, "n_start", NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "command clear node config");
	iRet = xuiFlowGraphGetNodeConfig(pGraph, "n_start", &pGotConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && pGotConfig == NULL, "clear node config applied");

	tNode.sId = "n_end";
	tNode.sType = "end";
	tNode.sTitle = "End";
	tNode.fX = 360.0f;
	iRet = xuiFlowGraphAddNode(pGraph, &tNode, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iEnd == 1, "add end node");
	XUI_TEST_CHECK(xuiFlowGraphGetNodeCount(pGraph) == 2, "node count");

	memset(&tPort, 0, sizeof(tPort));
	tPort.iSize = sizeof(tPort);
	tPort.sId = "out";
	tPort.sTitle = "Out";
	tPort.iDirection = XUI_FLOW_PORT_OUTPUT;
	tPort.iKind = XUI_FLOW_PORT_CONTROL;
	iRet = xuiFlowGraphAddPort(pGraph, iStart, &tPort, &iPort);
	XUI_TEST_CHECK(iRet == XUI_OK && iPort == 0, "add output port");
	XUI_TEST_CHECK(xuiFlowGraphGetRevision(pGraph) > iRevision, "add port bumps revision");
	iRevision = xuiFlowGraphGetRevision(pGraph);
	XUI_TEST_CHECK(xuiFlowGraphGetNodePortCount(pGraph, iStart) == 1, "start port count");

	iRet = xuiFlowGraphAddPort(pGraph, iStart, &tPort, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_ALREADY_INITIALIZED, "duplicate port rejected");
	tPort.sId = "dyn_old";
	tPort.sTitle = "Old dynamic";
	tPort.bDynamic = 1;
	iRet = xuiFlowGraphAddPort(pGraph, iStart, &tPort, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add initial dynamic port");
	memset(arrDynamicPorts, 0, sizeof(arrDynamicPorts));
	arrDynamicPorts[0] = tPort;
	arrDynamicPorts[0].iSize = sizeof(arrDynamicPorts[0]);
	arrDynamicPorts[0].sId = "dyn_new";
	arrDynamicPorts[1] = tPort;
	arrDynamicPorts[1].iSize = sizeof(arrDynamicPorts[1]);
	arrDynamicPorts[1].sId = "out";
	iRet = xuiFlowGraphRebuildNodeDynamicPorts(pGraph, "n_start", arrDynamicPorts, 2);
	XUI_TEST_CHECK(iRet == XUI_ERROR_ALREADY_INITIALIZED, "dynamic port rebuild rejects duplicate atomically");
	XUI_TEST_CHECK(__xuiFlowGraphTestHasPort(pGraph, iStart, "dyn_old") &&
		!__xuiFlowGraphTestHasPort(pGraph, iStart, "dyn_new") &&
		xuiFlowGraphGetNodePortCount(pGraph, iStart) == 2, "failed dynamic port rebuild preserves old ports");
	iRet = xuiFlowGraphRebuildNodeDynamicPorts(pGraph, "n_start", arrDynamicPorts, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && !__xuiFlowGraphTestHasPort(pGraph, iStart, "dyn_old") &&
		__xuiFlowGraphTestHasPort(pGraph, iStart, "dyn_new"), "successful dynamic port rebuild commits staged ports");

	tPort.sId = "in";
	tPort.sTitle = "In";
	tPort.iDirection = XUI_FLOW_PORT_INPUT;
	tPort.bDynamic = 0;
	iRet = xuiFlowGraphAddPort(pGraph, iEnd, &tPort, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add input port");
	XUI_TEST_CHECK(xuiFlowGraphGetNodePortCount(pGraph, iEnd) == 1, "end port count");

	memset(&tEdge, 0, sizeof(tEdge));
	tEdge.iSize = sizeof(tEdge);
	tEdge.sId = "e_start_end";
	tEdge.iKind = XUI_FLOW_PORT_CONTROL;
	tEdge.sFromNode = "n_start";
	tEdge.sFromPort = "out";
	tEdge.sToNode = "n_end";
	tEdge.sToPort = "in";
	iRet = xuiFlowGraphAddEdge(pGraph, &tEdge, &iEdge);
	XUI_TEST_CHECK(iRet == XUI_OK && iEdge == 0, "add edge");
	XUI_TEST_CHECK(xuiFlowGraphGetRevision(pGraph) > iRevision, "add edge bumps revision");
	iRevision = xuiFlowGraphGetRevision(pGraph);
	XUI_TEST_CHECK(xuiFlowGraphGetEdgeCount(pGraph) == 1, "edge count");
	XUI_TEST_CHECK(xuiFlowGraphFindEdge(pGraph, "e_start_end") == iEdge, "find edge");
	memset(sTinyId, 'x', sizeof(sTinyId));
	iRet = xuiFlowGraphMakeNodeId(pGraph, sTinyId, (int)sizeof(sTinyId));
	XUI_TEST_CHECK(iRet == XUI_ERROR_LIMIT_EXCEEDED && sTinyId[0] == 0, "short generated id buffer rejected");
	iRet = xuiFlowGraphMakeNodeId(pGraph, sId, (int)sizeof(sId));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, sId) < 0, "make node id");
	iRet = xuiFlowGraphMakeEdgeId(pGraph, sId, (int)sizeof(sId));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindEdge(pGraph, sId) < 0, "make edge id");
	iRet = xuiFlowGraphMakePortId(pGraph, iStart, sId, (int)sizeof(sId));
	XUI_TEST_CHECK(iRet == XUI_OK, "make port id");

	iRet = xuiFlowGraphSelectNode(pGraph, "n_start", 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphIsNodeSelected(pGraph, "n_start"), "select node");
	XUI_TEST_CHECK(xuiFlowGraphGetRevision(pGraph) > iRevision, "selection bumps revision");
	iRevision = xuiFlowGraphGetRevision(pGraph);
	iRet = xuiFlowGraphSelectNode(pGraph, "n_start", 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetRevision(pGraph) == iRevision, "unchanged selection does not bump revision");
	iRet = xuiFlowGraphSelectEdge(pGraph, "e_start_end", 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphIsEdgeSelected(pGraph, "e_start_end"), "select edge");
	XUI_TEST_CHECK(xuiFlowGraphGetSelectedNodeCount(pGraph) == 1, "selected node count");
	XUI_TEST_CHECK(xuiFlowGraphGetSelectedEdgeCount(pGraph) == 1, "selected edge count");
	iRet = xuiFlowGraphSelectNode(pGraph, "n_end", 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetSelectedNodeCount(pGraph) == 2,
		"select second node for indexed selection removal");
	iRet = xuiFlowGraphSelectNode(pGraph, "n_start", 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiFlowGraphIsNodeSelected(pGraph, "n_start") &&
		xuiFlowGraphIsNodeSelected(pGraph, "n_end") && xuiFlowGraphGetSelectedNodeCount(pGraph) == 1,
		"swap removal preserves moved selection index");
	iRet = xuiFlowGraphSelectNode(pGraph, "n_end", 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetSelectedNodeCount(pGraph) == 0,
		"clear remaining indexed node selection");

	iRet = xuiFlowGraphAddEdge(pGraph, &tEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_ALREADY_INITIALIZED, "duplicate edge rejected");
	tEdge.sId = "e_reverse";
	tEdge.sFromNode = "n_end";
	tEdge.sFromPort = "in";
	tEdge.sToNode = "n_start";
	tEdge.sToPort = "out";
	iRet = xuiFlowGraphAddEdge(pGraph, &tEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "edge direction contract enforced");
	iRet = xuiFlowGraphAddEdgePreserveInvalid(pGraph, &tEdge, &iEdge);
	XUI_TEST_CHECK(iRet == XUI_OK, "preserve invalid edge for deserialization");
	memset(&tEdgeInfo, 0, sizeof(tEdgeInfo));
	tEdgeInfo.iSize = sizeof(tEdgeInfo);
	iRet = xuiFlowGraphGetEdge(pGraph, iEdge, &tEdgeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tEdgeInfo.bInvalid, "preserved direction mismatch remains marked invalid");
	iRet = xuiFlowGraphRemoveEdge(pGraph, "e_reverse");
	XUI_TEST_CHECK(iRet == XUI_OK, "remove preserved invalid edge");
	tEdge.sId = "e_start_end";
	tEdge.sFromNode = "n_start";
	tEdge.sFromPort = "out";
	tEdge.sToNode = "n_end";
	tEdge.sToPort = "in";

	tEdge.sId = "e_bad";
	tEdge.sToPort = "missing";
	iRet = xuiFlowGraphAddEdge(pGraph, &tEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "invalid edge rejected");
	tEdge.sToPort = "in";

	iRet = xuiFlowGraphRemoveEdge(pGraph, "e_start_end");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetEdgeCount(pGraph) == 0, "remove edge");
	XUI_TEST_CHECK(xuiFlowGraphFindEdge(pGraph, "e_start_end") < 0, "edge removed from map");
	XUI_TEST_CHECK(!xuiFlowGraphIsEdgeSelected(pGraph, "e_start_end"), "removed edge deselected");

	tEdge.sId = "e_start_end_2";
	iRet = xuiFlowGraphAddEdge(pGraph, &tEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetEdgeCount(pGraph) == 1, "add edge for node removal");
	iRet = xuiFlowGraphSelectNode(pGraph, "n_end", 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "select end before remove");
	iRet = xuiFlowGraphRemoveNode(pGraph, "n_end");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetNodeCount(pGraph) == 1, "remove node");
	XUI_TEST_CHECK(xuiFlowGraphGetEdgeCount(pGraph) == 0, "node removal removes edges");
	XUI_TEST_CHECK(xuiFlowGraphFindNode(pGraph, "n_end") < 0, "node removed from map");
	XUI_TEST_CHECK(!xuiFlowGraphIsNodeSelected(pGraph, "n_end"), "removed node deselected");
	iRet = xuiFlowGraphClearSelection(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetSelectedNodeCount(pGraph) == 0 && xuiFlowGraphGetSelectedEdgeCount(pGraph) == 0, "clear selection");
	memset(&tDiagnostic, 0, sizeof(tDiagnostic));
	tDiagnostic.iSize = sizeof(tDiagnostic);
	tDiagnostic.iSeverity = 2;
	tDiagnostic.sCode = "missing-end";
	tDiagnostic.sMessage = "End node is missing.";
	tDiagnostic.sPath = "workflow.nodes";
	iRet = xuiFlowGraphAddDiagnostic(pGraph, &tDiagnostic, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetDiagnosticCount(pGraph) == 1, "add diagnostic");
	memset(&tDiagnostic, 0, sizeof(tDiagnostic));
	iRet = xuiFlowGraphGetDiagnostic(pGraph, 0, &tDiagnostic);
	XUI_TEST_CHECK(iRet == XUI_OK && tDiagnostic.iSeverity == 2 && strcmp(tDiagnostic.sCode, "missing-end") == 0 &&
	               strcmp(tDiagnostic.sMessage, "End node is missing.") == 0 && strcmp(tDiagnostic.sPath, "workflow.nodes") == 0,
	               "read diagnostic details");
	iRet = xuiFlowGraphGetDiagnostic(pGraph, 1, &tDiagnostic);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject invalid diagnostic index");
	iRet = xuiFlowGraphClearDiagnostics(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetDiagnosticCount(pGraph) == 0, "clear diagnostics");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	tNode.sId = "n_delete";
	tNode.sType = "end";
	tNode.sTitle = "Delete";
	tNode.fX = 280.0f;
	tNode.fY = 160.0f;
	iRet = xuiFlowGraphAddNode(pGraph, &tNode, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK, "add node for delete selection");
	memset(&tPort, 0, sizeof(tPort));
	tPort.iSize = sizeof(tPort);
	tPort.sId = "in";
	tPort.sTitle = "In";
	tPort.iDirection = XUI_FLOW_PORT_INPUT;
	tPort.iKind = XUI_FLOW_PORT_CONTROL;
	iRet = xuiFlowGraphAddPort(pGraph, iEnd, &tPort, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add delete input");
	memset(&tEdge, 0, sizeof(tEdge));
	tEdge.iSize = sizeof(tEdge);
	tEdge.sId = "e_delete";
	tEdge.iKind = XUI_FLOW_PORT_CONTROL;
	tEdge.sFromNode = "n_start";
	tEdge.sFromPort = "out";
	tEdge.sToNode = "n_delete";
	tEdge.sToPort = "in";
	iRet = xuiFlowGraphAddEdge(pGraph, &tEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add edge for delete selection");
	iRet = xuiFlowGraphSelectNode(pGraph, "n_delete", 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "select node for delete selection");
	iRet = xuiFlowGraphSelectEdge(pGraph, "e_delete", 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "select edge for delete selection");
	iRet = xuiFlowGraphSetNodeRunState(pGraph, "n_delete", XUI_WORKFLOW_NODE_RUN_SUCCESS, "node result");
	XUI_TEST_CHECK(iRet == XUI_OK, "set deleted node run state");
	iRet = xuiFlowGraphSetEdgeRunState(pGraph, "e_delete", XUI_WORKFLOW_EDGE_RUN_ACTIVE, "edge result");
	XUI_TEST_CHECK(iRet == XUI_OK, "set deleted edge run state");
	iRet = xuiFlowGraphDeleteSelection(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "n_delete") < 0 && xuiFlowGraphFindEdge(pGraph, "e_delete") < 0, "delete selection removes selected graph items");
	XUI_TEST_CHECK(xuiFlowGraphGetSelectedNodeCount(pGraph) == 0 && xuiFlowGraphGetSelectedEdgeCount(pGraph) == 0, "delete selection clears selection");
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "n_delete") >= 0 &&
		xuiFlowGraphFindEdge(pGraph, "e_delete") >= 0 && xuiFlowGraphCanRedo(pGraph),
		"delete selection restores all items in one undo");
	memset(&tNodeInfo, 0, sizeof(tNodeInfo));
	tNodeInfo.iSize = sizeof(tNodeInfo);
	iRet = xuiFlowGraphGetNode(pGraph, xuiFlowGraphFindNode(pGraph, "n_delete"), &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.iRunState == XUI_WORKFLOW_NODE_RUN_SUCCESS &&
		tNodeInfo.sRunPreview != NULL && strcmp(tNodeInfo.sRunPreview, "node result") == 0,
		"delete undo restores node run state");
	memset(&tEdgeInfo, 0, sizeof(tEdgeInfo));
	tEdgeInfo.iSize = sizeof(tEdgeInfo);
	iRet = xuiFlowGraphGetEdge(pGraph, xuiFlowGraphFindEdge(pGraph, "e_delete"), &tEdgeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tEdgeInfo.iRunState == XUI_WORKFLOW_EDGE_RUN_ACTIVE &&
		tEdgeInfo.sRunPreview != NULL && strcmp(tEdgeInfo.sRunPreview, "edge result") == 0,
		"delete undo restores edge run state");
	iRet = xuiFlowGraphRedo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "n_delete") < 0 &&
		xuiFlowGraphFindEdge(pGraph, "e_delete") < 0,
		"redo repeats complete selected deletion in one step");

	iRevision = xuiFlowGraphGetRevision(pGraph);
	iRet = xuiFlowGraphSetNodePosition(pGraph, "n_start", NAN, 1.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiFlowGraphGetRevision(pGraph) == iRevision, "non-finite node position rejected");
	tViewport.fZoom = NAN;
	iRet = xuiFlowGraphSetViewport(pGraph, &tViewport);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiFlowGraphGetRevision(pGraph) == iRevision, "non-finite viewport rejected");

	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	tNode.sId = "n_cmd";
	tNode.sType = "action";
	tNode.sTitle = "Command";
	tNode.fX = 300.0f;
	tNode.fY = 180.0f;
	iRet = xuiFlowGraphCommandAddNode(pGraph, &tNode, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "n_cmd") == iEnd && xuiFlowGraphCanUndo(pGraph), "command add node");
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "n_cmd") < 0 && xuiFlowGraphCanRedo(pGraph), "undo add node");
	iRet = xuiFlowGraphRedo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "n_cmd") >= 0 && xuiFlowGraphCanUndo(pGraph), "redo add node");

	memset(&tMove, 0, sizeof(tMove));
	tMove.iSize = sizeof(tMove);
	tMove.sId = "n_cmd";
	tMove.fX = 340.0f;
	tMove.fY = 220.0f;
	iRet = xuiFlowGraphCommandMoveNode(pGraph, &tMove);
	XUI_TEST_CHECK(iRet == XUI_OK, "command move node");
	iRet = xuiFlowGraphGetNode(pGraph, xuiFlowGraphFindNode(pGraph, "n_cmd"), &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 340.0f && tNodeInfo.fY == 220.0f, "command move applied");
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK, "undo move node");
	iRet = xuiFlowGraphGetNode(pGraph, xuiFlowGraphFindNode(pGraph, "n_cmd"), &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 300.0f && tNodeInfo.fY == 180.0f, "undo move restored");
	iRet = xuiFlowGraphRedo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK, "redo move node");
	iRet = xuiFlowGraphGetNode(pGraph, xuiFlowGraphFindNode(pGraph, "n_cmd"), &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.fX == 340.0f && tNodeInfo.fY == 220.0f, "redo move applied");

	memset(&tPort, 0, sizeof(tPort));
	tPort.iSize = sizeof(tPort);
	tPort.sId = "in";
	tPort.sTitle = "In";
	tPort.iDirection = XUI_FLOW_PORT_INPUT;
	tPort.iKind = XUI_FLOW_PORT_CONTROL;
	iRet = xuiFlowGraphAddPort(pGraph, xuiFlowGraphFindNode(pGraph, "n_cmd"), &tPort, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add command target port");
	memset(&tEdge, 0, sizeof(tEdge));
	tEdge.iSize = sizeof(tEdge);
	tEdge.sId = "e_cmd";
	tEdge.iKind = XUI_FLOW_PORT_CONTROL;
	tEdge.sFromNode = "n_start";
	tEdge.sFromPort = "out";
	tEdge.sToNode = "n_cmd";
	tEdge.sToPort = "in";
	iRet = xuiFlowGraphCommandAddEdge(pGraph, &tEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindEdge(pGraph, "e_cmd") >= 0, "command add edge");
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindEdge(pGraph, "e_cmd") < 0 && xuiFlowGraphCanRedo(pGraph), "undo add edge");
	iRet = xuiFlowGraphRedo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindEdge(pGraph, "e_cmd") >= 0, "redo add edge");
	iRet = xuiFlowGraphSetEdgeRunState(pGraph, "e_cmd", XUI_WORKFLOW_EDGE_RUN_TAKEN, "taken");
	XUI_TEST_CHECK(iRet == XUI_OK, "set command edge run state");
	iRet = xuiFlowGraphCommandRemoveEdge(pGraph, "e_cmd");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindEdge(pGraph, "e_cmd") < 0, "command remove edge");
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindEdge(pGraph, "e_cmd") >= 0, "undo remove edge");
	memset(&tEdgeInfo, 0, sizeof(tEdgeInfo));
	tEdgeInfo.iSize = sizeof(tEdgeInfo);
	iRet = xuiFlowGraphGetEdge(pGraph, xuiFlowGraphFindEdge(pGraph, "e_cmd"), &tEdgeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tEdgeInfo.iRunState == XUI_WORKFLOW_EDGE_RUN_TAKEN &&
		tEdgeInfo.sRunPreview != NULL && strcmp(tEdgeInfo.sRunPreview, "taken") == 0,
		"undo remove edge restores run state");
	iRet = xuiFlowGraphRedo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindEdge(pGraph, "e_cmd") < 0, "redo remove edge");
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindEdge(pGraph, "e_cmd") >= 0, "restore edge before remove node command");
	iRet = xuiFlowGraphSetNodeConfig(pGraph, "n_cmd", pConfig);
	XUI_TEST_CHECK(iRet == XUI_OK, "set config before remove node command");
	iRet = xuiFlowGraphSetNodeRunState(pGraph, "n_cmd", XUI_WORKFLOW_NODE_RUN_WARNING, "warning");
	XUI_TEST_CHECK(iRet == XUI_OK, "set command node run state");
	iRet = xuiFlowGraphCommandRemoveNode(pGraph, "n_cmd");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "n_cmd") < 0 && xuiFlowGraphFindEdge(pGraph, "e_cmd") < 0, "command remove node removes incident edge");
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "n_cmd") >= 0 && xuiFlowGraphFindEdge(pGraph, "e_cmd") >= 0, "undo remove node restores edge");
	XUI_TEST_CHECK(xuiFlowGraphGetNodePortCount(pGraph, xuiFlowGraphFindNode(pGraph, "n_cmd")) == 1, "undo remove node restores ports");
	memset(&tNodeInfo, 0, sizeof(tNodeInfo));
	tNodeInfo.iSize = sizeof(tNodeInfo);
	iRet = xuiFlowGraphGetNode(pGraph, xuiFlowGraphFindNode(pGraph, "n_cmd"), &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.iRunState == XUI_WORKFLOW_NODE_RUN_WARNING &&
		tNodeInfo.sRunPreview != NULL && strcmp(tNodeInfo.sRunPreview, "warning") == 0,
		"undo remove node restores run state");
	memset(&tEdgeInfo, 0, sizeof(tEdgeInfo));
	tEdgeInfo.iSize = sizeof(tEdgeInfo);
	iRet = xuiFlowGraphGetEdge(pGraph, xuiFlowGraphFindEdge(pGraph, "e_cmd"), &tEdgeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tEdgeInfo.iRunState == XUI_WORKFLOW_EDGE_RUN_TAKEN,
		"undo remove node restores incident edge run state");
	iRet = xuiFlowGraphGetNodeConfig(pGraph, "n_cmd", &pGotConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && pGotConfig != NULL &&
		strcmp(xuiXrtValueObjectGetText(pGotConfig, "prompt", 6), "hello") == 0,
		"undo remove node restores isolated config snapshot");
	xrtValueRelease(pGotConfig);
	pGotConfig = NULL;
	iRet = xuiFlowGraphRedo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "n_cmd") < 0 && xuiFlowGraphFindEdge(pGraph, "e_cmd") < 0, "redo remove node");
	iRet = xuiFlowGraphSetCommandHistoryLimit(pGraph, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetCommandHistoryLimit(pGraph) == 1, "set command history limit");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	tNode.sId = "n_cap1";
	tNode.sType = "action";
	tNode.sTitle = "Cap1";
	tNode.fX = 420.0f;
	tNode.fY = 160.0f;
	iRet = xuiFlowGraphCommandAddNode(pGraph, &tNode, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "history limit add first");
	tNode.sId = "n_cap2";
	tNode.sTitle = "Cap2";
	tNode.fX = 460.0f;
	iRet = xuiFlowGraphCommandAddNode(pGraph, &tNode, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "history limit add second");
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "n_cap2") < 0 && xuiFlowGraphFindNode(pGraph, "n_cap1") >= 0, "history limit keeps newest command");
	XUI_TEST_CHECK(!xuiFlowGraphCanUndo(pGraph), "history limit trims oldest command");

	iRet = xuiFlowGraphCreate(&pAuxGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && pAuxGraph != NULL, "create auxiliary graph");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	tNode.sType = "action";
	tNode.sTitle = "Aux";
	tNode.fW = 120.0f;
	tNode.fH = 64.0f;
	tNode.sId = "aux_a";
	iRet = xuiFlowGraphAddNode(pAuxGraph, &tNode, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add auxiliary first node");
	tNode.sId = "aux_b";
	iRet = xuiFlowGraphAddNode(pAuxGraph, &tNode, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add auxiliary middle node");
	tNode.sId = "aux_c";
	iRet = xuiFlowGraphAddNode(pAuxGraph, &tNode, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add auxiliary last node");
	iRet = xuiFlowGraphRemoveNode(pAuxGraph, "aux_b");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pAuxGraph, "aux_a") == 0 &&
		xuiFlowGraphFindNode(pAuxGraph, "aux_c") == 1, "node removal updates shifted map indexes");
	memset(&tPort, 0, sizeof(tPort));
	tPort.iSize = sizeof(tPort);
	tPort.sId = "out";
	tPort.iDirection = XUI_FLOW_PORT_OUTPUT;
	tPort.iKind = XUI_FLOW_PORT_CONTROL;
	iRet = xuiFlowGraphAddPort(pAuxGraph, 0, &tPort, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add auxiliary output port");
	tPort.sId = "in";
	tPort.iDirection = XUI_FLOW_PORT_INPUT;
	tPort.bMulti = 1;
	iRet = xuiFlowGraphAddPort(pAuxGraph, 1, &tPort, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add auxiliary input port");
	memset(&tEdge, 0, sizeof(tEdge));
	tEdge.iSize = sizeof(tEdge);
	tEdge.iKind = XUI_FLOW_PORT_CONTROL;
	tEdge.sFromNode = "aux_a";
	tEdge.sFromPort = "out";
	tEdge.sToNode = "aux_c";
	tEdge.sToPort = "in";
	tEdge.sId = "aux_edge_a";
	iRet = xuiFlowGraphAddEdge(pAuxGraph, &tEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add auxiliary first edge");
	tEdge.sId = "aux_edge_b";
	iRet = xuiFlowGraphAddEdge(pAuxGraph, &tEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add auxiliary second edge");
	iRet = xuiFlowGraphRemoveEdge(pAuxGraph, "aux_edge_a");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindEdge(pAuxGraph, "aux_edge_b") == 0,
		"edge removal updates shifted map indexes");
	memset(&tMoveRecord, 0, sizeof(tMoveRecord));
	tMoveRecord.iSize = sizeof(tMoveRecord);
	tMoveRecord.sId = "aux_a";
	tMoveRecord.fOldX = 0.0f;
	tMoveRecord.fOldY = 0.0f;
	tMoveRecord.fNewX = 0.0f;
	tMoveRecord.fNewY = 0.0f;
	iRet = xuiFlowGraphCommandRecordMoveNodes(pAuxGraph, &tMoveRecord, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiFlowGraphCanUndo(pAuxGraph), "no-op drag record does not create history");
	tMoveRecord.fNewX = 1.0f;
	iRet = xuiFlowGraphCommandRecordMoveNodes(pAuxGraph, &tMoveRecord, 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && !xuiFlowGraphCanUndo(pAuxGraph),
		"drag record must match the already-applied position");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	tNode.sId = "aux_history";
	tNode.sType = "action";
	tNode.sTitle = "History";
	iStart = 99;
	iRet = xuiFlowGraphCommandAddNode(NULL, &tNode, &iStart);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iStart == -1, "failed command add clears output index");
	iRet = xuiFlowGraphCommandAddNode(pAuxGraph, &tNode, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphCanUndo(pAuxGraph), "add auxiliary history command");
	iRet = xuiFlowGraphUndo(pAuxGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphCanRedo(pAuxGraph), "prepare auxiliary redo command");
	iRet = xuiFlowGraphAddNode(pAuxGraph, &tNode, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "create redo conflict");
	iRet = xuiFlowGraphRedo(pAuxGraph);
	XUI_TEST_CHECK(iRet == XUI_ERROR_ALREADY_INITIALIZED && xuiFlowGraphCanRedo(pAuxGraph),
		"failed redo preserves history entry");
	iRet = xuiFlowGraphRemoveNode(pAuxGraph, "aux_history");
	XUI_TEST_CHECK(iRet == XUI_OK, "remove redo conflict");
	iRet = xuiFlowGraphRedo(pAuxGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiFlowGraphCanRedo(pAuxGraph) && xuiFlowGraphFindNode(pAuxGraph, "aux_history") >= 0,
		"preserved redo succeeds after conflict clears");
	memset(&tDiagnostic, 0, sizeof(tDiagnostic));
	tDiagnostic.iSize = sizeof(tDiagnostic);
	tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_HINT + 1;
	iRevision = xuiFlowGraphGetRevision(pAuxGraph);
	iPort = 99;
	iRet = xuiFlowGraphAddDiagnostic(pAuxGraph, &tDiagnostic, &iPort);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iPort == -1 &&
		xuiFlowGraphGetRevision(pAuxGraph) == iRevision, "invalid diagnostic is rejected without mutation");
	iRet = xuiFlowGraphSelectNode(pAuxGraph, "aux_a", 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "select first connected batch node");
	iRet = xuiFlowGraphSelectNode(pAuxGraph, "aux_c", 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "select second connected batch node");
	iRet = xuiFlowGraphDeleteSelection(pAuxGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pAuxGraph, "aux_a") < 0 &&
		xuiFlowGraphFindNode(pAuxGraph, "aux_c") < 0 && xuiFlowGraphFindEdge(pAuxGraph, "aux_edge_b") < 0,
		"batch delete removes two connected nodes and their edge");
	iRet = xuiFlowGraphUndo(pAuxGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pAuxGraph, "aux_a") >= 0 &&
		xuiFlowGraphFindNode(pAuxGraph, "aux_c") >= 0 && xuiFlowGraphFindEdge(pAuxGraph, "aux_edge_b") >= 0,
		"one undo restores connected batch without duplicate edges");
	iRet = xuiFlowGraphRedo(pAuxGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pAuxGraph, "aux_a") < 0 &&
		xuiFlowGraphFindNode(pAuxGraph, "aux_c") < 0 && xuiFlowGraphFindEdge(pAuxGraph, "aux_edge_b") < 0,
		"one redo removes connected batch again");

cleanup:
	if ( pGotConfig != NULL ) {
		xrtValueRelease(pGotConfig);
	}
	if ( pConfig2 != NULL ) {
		xrtValueRelease(pConfig2);
	}
	if ( pConfig != NULL ) {
		xrtValueRelease(pConfig);
	}
	xuiFlowGraphDestroy(pAuxGraph);
	xuiFlowGraphDestroy(pGraph);
	if ( iFailed ) {
		return 1;
	}
	printf("xui_flow_graph_test passed\n");
	return 0;
}
