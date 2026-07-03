#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_SCALE_NODE_COUNT 500
#define XUI_SCALE_EDGE_COUNT 2000

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_flow_graph_scale_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int add_port(xui_flow_graph pGraph, int iNode, const char* sId, int iDirection)
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

int main(void)
{
	xui_flow_graph pGraph;
	xui_flow_node_desc_t tNode;
	xui_flow_edge_desc_t tEdge;
	xui_flow_node_info_t tNodeInfo;
	xui_flow_edge_info_t tEdgeInfo;
	int iFailed;
	int iRet;
	int i;
	int iNode;
	char sNodeId[64];
	char sEdgeId[64];
	char sFromNode[64];
	char sToNode[64];

	pGraph = NULL;
	iFailed = 0;
	iRet = xuiFlowGraphCreate(&pGraph);
	XUI_TEST_CHECK(iRet == XUI_OK && pGraph != NULL, "create graph");

	for ( i = 0; i < XUI_SCALE_NODE_COUNT; ++i ) {
		snprintf(sNodeId, sizeof(sNodeId), "n_%03d", i);
		memset(&tNode, 0, sizeof(tNode));
		tNode.iSize = sizeof(tNode);
		tNode.sId = sNodeId;
		tNode.sType = "scale.node";
		tNode.sTitle = "Scale Node";
		tNode.fX = (float)((i % 25) * 260);
		tNode.fY = (float)((i / 25) * 150);
		tNode.fW = 220.0f;
		tNode.fH = 110.0f;
		iRet = xuiFlowGraphAddNode(pGraph, &tNode, &iNode);
		XUI_TEST_CHECK(iRet == XUI_OK && iNode == i, "add scale node");
		iRet = add_port(pGraph, iNode, "in", XUI_FLOW_PORT_INPUT);
		XUI_TEST_CHECK(iRet == XUI_OK, "add input port");
		iRet = add_port(pGraph, iNode, "out", XUI_FLOW_PORT_OUTPUT);
		XUI_TEST_CHECK(iRet == XUI_OK, "add output port");
	}

	for ( i = 0; i < XUI_SCALE_EDGE_COUNT; ++i ) {
		snprintf(sEdgeId, sizeof(sEdgeId), "e_%04d", i);
		snprintf(sFromNode, sizeof(sFromNode), "n_%03d", i % XUI_SCALE_NODE_COUNT);
		snprintf(sToNode, sizeof(sToNode), "n_%03d", (i * 7 + 1) % XUI_SCALE_NODE_COUNT);
		memset(&tEdge, 0, sizeof(tEdge));
		tEdge.iSize = sizeof(tEdge);
		tEdge.sId = sEdgeId;
		tEdge.iKind = XUI_FLOW_PORT_CONTROL;
		tEdge.sFromNode = sFromNode;
		tEdge.sFromPort = "out";
		tEdge.sToNode = sToNode;
		tEdge.sToPort = "in";
		iRet = xuiFlowGraphAddEdge(pGraph, &tEdge, NULL);
		XUI_TEST_CHECK(iRet == XUI_OK, "add scale edge");
	}

	XUI_TEST_CHECK(xuiFlowGraphGetNodeCount(pGraph) == XUI_SCALE_NODE_COUNT, "scale node count");
	XUI_TEST_CHECK(xuiFlowGraphGetEdgeCount(pGraph) == XUI_SCALE_EDGE_COUNT, "scale edge count");
	XUI_TEST_CHECK(xuiFlowGraphFindNode(pGraph, "n_499") == 499, "find last node");
	XUI_TEST_CHECK(xuiFlowGraphFindEdge(pGraph, "e_1999") == 1999, "find last edge");

	for ( i = 0; i < XUI_SCALE_NODE_COUNT; i += 37 ) {
		memset(&tNodeInfo, 0, sizeof(tNodeInfo));
		tNodeInfo.iSize = sizeof(tNodeInfo);
		iRet = xuiFlowGraphGetNode(pGraph, i, &tNodeInfo);
		XUI_TEST_CHECK(iRet == XUI_OK && tNodeInfo.sId != NULL, "read scale node");
	}
	for ( i = 0; i < XUI_SCALE_EDGE_COUNT; i += 113 ) {
		memset(&tEdgeInfo, 0, sizeof(tEdgeInfo));
		tEdgeInfo.iSize = sizeof(tEdgeInfo);
		iRet = xuiFlowGraphGetEdge(pGraph, i, &tEdgeInfo);
		XUI_TEST_CHECK(iRet == XUI_OK && !tEdgeInfo.bInvalid, "read scale edge");
	}

cleanup:
	xuiFlowGraphDestroy(pGraph);
	return iFailed ? 1 : 0;
}
