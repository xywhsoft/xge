#include "xui.h"

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

int main(void)
{
	xui_flow_graph pGraph;
	xui_flow_node_desc_t tNode;
	xui_flow_port_desc_t tPort;
	xui_flow_edge_desc_t tEdge;
	xui_flow_move_node_desc_t tMove;
	xui_flow_viewport_t tViewport;
	xui_flow_diagnostic_desc_t tDiagnostic;
	xui_flow_node_info_t tNodeInfo;
	xvalue pConfig;
	xvalue pConfig2;
	xvalue pGotConfig;
	int iStart;
	int iEnd;
	int iPort;
	int iEdge;
	int iRet;
	int iFailed;
	uint32_t iRevision;
	char sId[64];
	float fX;
	float fY;

	pGraph = NULL;
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
	pConfig = xvoCreateTable();
	XUI_TEST_CHECK(pConfig != NULL, "create node config");
	xvoTableSetText(pConfig, "prompt", 6, "hello", 5, FALSE);
	iRet = xuiFlowGraphCommandSetNodeConfig(pGraph, "n_start", pConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphCanUndo(pGraph), "command set node config");
	iRet = xuiFlowGraphGetNodeConfig(pGraph, "n_start", &pGotConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && pGotConfig != NULL && strcmp((const char*)xvoTableGetText(pGotConfig, "prompt", 6), "hello") == 0, "get node config");
	xvoUnref(pGotConfig);
	pGotConfig = NULL;
	pConfig2 = xvoCreateTable();
	XUI_TEST_CHECK(pConfig2 != NULL, "create second node config");
	xvoTableSetText(pConfig2, "prompt", 6, "world", 5, FALSE);
	iRet = xuiFlowGraphCommandSetNodeConfig(pGraph, "n_start", pConfig2);
	XUI_TEST_CHECK(iRet == XUI_OK, "command replace node config");
	iRet = xuiFlowGraphGetNodeConfig(pGraph, "n_start", &pGotConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && pGotConfig != NULL && strcmp((const char*)xvoTableGetText(pGotConfig, "prompt", 6), "world") == 0, "replace node config applied");
	xvoUnref(pGotConfig);
	pGotConfig = NULL;
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK, "undo node config");
	iRet = xuiFlowGraphGetNodeConfig(pGraph, "n_start", &pGotConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && pGotConfig != NULL && strcmp((const char*)xvoTableGetText(pGotConfig, "prompt", 6), "hello") == 0, "undo node config restores previous value");
	xvoUnref(pGotConfig);
	pGotConfig = NULL;
	iRet = xuiFlowGraphRedo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK, "redo node config");
	iRet = xuiFlowGraphGetNodeConfig(pGraph, "n_start", &pGotConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && pGotConfig != NULL && strcmp((const char*)xvoTableGetText(pGotConfig, "prompt", 6), "world") == 0, "redo node config restores new value");
	xvoUnref(pGotConfig);
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

	tPort.sId = "in";
	tPort.sTitle = "In";
	tPort.iDirection = XUI_FLOW_PORT_INPUT;
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
	iRet = xuiFlowGraphSelectEdge(pGraph, "e_start_end", 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphIsEdgeSelected(pGraph, "e_start_end"), "select edge");
	XUI_TEST_CHECK(xuiFlowGraphGetSelectedNodeCount(pGraph) == 1, "selected node count");
	XUI_TEST_CHECK(xuiFlowGraphGetSelectedEdgeCount(pGraph) == 1, "selected edge count");
	iRet = xuiFlowGraphSelectNode(pGraph, "n_start", 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiFlowGraphIsNodeSelected(pGraph, "n_start"), "deselect node");

	iRet = xuiFlowGraphAddEdge(pGraph, &tEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_ALREADY_INITIALIZED, "duplicate edge rejected");

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
	iRet = xuiFlowGraphDeleteSelection(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "n_delete") < 0 && xuiFlowGraphFindEdge(pGraph, "e_delete") < 0, "delete selection removes selected graph items");
	XUI_TEST_CHECK(xuiFlowGraphGetSelectedNodeCount(pGraph) == 0 && xuiFlowGraphGetSelectedEdgeCount(pGraph) == 0, "delete selection clears selection");

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
	iRet = xuiFlowGraphCommandRemoveEdge(pGraph, "e_cmd");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindEdge(pGraph, "e_cmd") < 0, "command remove edge");
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindEdge(pGraph, "e_cmd") >= 0, "undo remove edge");
	iRet = xuiFlowGraphRedo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindEdge(pGraph, "e_cmd") < 0, "redo remove edge");
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindEdge(pGraph, "e_cmd") >= 0, "restore edge before remove node command");
	iRet = xuiFlowGraphCommandRemoveNode(pGraph, "n_cmd");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "n_cmd") < 0 && xuiFlowGraphFindEdge(pGraph, "e_cmd") < 0, "command remove node removes incident edge");
	iRet = xuiFlowGraphUndo(pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphFindNode(pGraph, "n_cmd") >= 0 && xuiFlowGraphFindEdge(pGraph, "e_cmd") >= 0, "undo remove node restores edge");
	XUI_TEST_CHECK(xuiFlowGraphGetNodePortCount(pGraph, xuiFlowGraphFindNode(pGraph, "n_cmd")) == 1, "undo remove node restores ports");
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

cleanup:
	if ( pGotConfig != NULL ) {
		xvoUnref(pGotConfig);
	}
	if ( pConfig2 != NULL ) {
		xvoUnref(pConfig2);
	}
	if ( pConfig != NULL ) {
		xvoUnref(pConfig);
	}
	xuiFlowGraphDestroy(pGraph);
	if ( iFailed ) {
		return 1;
	}
	printf("xui_flow_graph_test passed\n");
	return 0;
}
