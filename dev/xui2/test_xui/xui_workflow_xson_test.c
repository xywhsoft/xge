#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_workflow_xson_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static void make_control_port(xui_flow_port_desc_t* pPort, const char* sId, int iDirection)
{
	memset(pPort, 0, sizeof(*pPort));
	pPort->iSize = sizeof(*pPort);
	pPort->sId = sId;
	pPort->sTitle = sId;
	pPort->iDirection = iDirection;
	pPort->iKind = XUI_FLOW_PORT_CONTROL;
}

static int register_type(xui_workflow pWorkflow, const char* sId, const char* sTitle, const xui_flow_port_desc_t* pInputs, int iInputCount, const xui_flow_port_desc_t* pOutputs, int iOutputCount)
{
	xui_workflow_node_type_desc_t tType;

	memset(&tType, 0, sizeof(tType));
	tType.iSize = sizeof(tType);
	tType.sId = sId;
	tType.iVersion = 1;
	tType.sTitle = sTitle;
	tType.sCategory = "Flow";
	tType.pInputs = pInputs;
	tType.iInputCount = iInputCount;
	tType.pOutputs = pOutputs;
	tType.iOutputCount = iOutputCount;
	return xuiWorkflowRegisterNodeType(pWorkflow, &tType, NULL);
}

static int register_test_types(xui_workflow pWorkflow)
{
	xui_flow_port_desc_t tStartOut;
	xui_flow_port_desc_t tMidIn;
	xui_flow_port_desc_t tMidOut;
	xui_flow_port_desc_t tConditionIn;
	xui_flow_port_desc_t tConditionTrue;
	xui_flow_port_desc_t tConditionFalse;
	xui_flow_port_desc_t tEndIn;
	int iRet;

	make_control_port(&tStartOut, "out", XUI_FLOW_PORT_OUTPUT);
	iRet = register_type(pWorkflow, "start", "Start", NULL, 0, &tStartOut, 1);
	if ( iRet != XUI_OK ) return iRet;
	make_control_port(&tMidIn, "in", XUI_FLOW_PORT_INPUT);
	make_control_port(&tMidOut, "out", XUI_FLOW_PORT_OUTPUT);
	iRet = register_type(pWorkflow, "llm", "LLM", &tMidIn, 1, &tMidOut, 1);
	if ( iRet != XUI_OK ) return iRet;
	make_control_port(&tConditionIn, "in", XUI_FLOW_PORT_INPUT);
	make_control_port(&tConditionTrue, "true", XUI_FLOW_PORT_OUTPUT);
	make_control_port(&tConditionFalse, "false", XUI_FLOW_PORT_OUTPUT);
	{
		xui_flow_port_desc_t arrConditionOut[2];
		arrConditionOut[0] = tConditionTrue;
		arrConditionOut[1] = tConditionFalse;
		iRet = register_type(pWorkflow, "condition", "Condition", &tConditionIn, 1, arrConditionOut, 2);
	}
	if ( iRet != XUI_OK ) return iRet;
	make_control_port(&tEndIn, "in", XUI_FLOW_PORT_INPUT);
	return register_type(pWorkflow, "end", "End", &tEndIn, 1, NULL, 0);
}

int main(void)
{
	xui_workflow pWorkflow;
	xui_workflow pLoaded;
	xui_flow_graph pGraph;
	xui_flow_viewport_t tViewport;
	xui_flow_viewport_t tLoadedViewport;
	xui_flow_node_info_t tNode;
	xui_flow_edge_desc_t tBrokenEdge;
	xui_flow_edge_info_t tEdge;
	xui_workflow_variable_desc_t tVariable;
	xui_workflow_node_run_state_t tNodeRun;
	xui_workflow_edge_run_state_t tEdgeRun;
	xvalue pConfig;
	xvalue pDefault;
	int iDiagnostics;
	int iRet;
	int iFailed;

	pWorkflow = NULL;
	pLoaded = NULL;
	pConfig = NULL;
	pDefault = NULL;
	iFailed = 0;

	iRet = xuiWorkflowCreate(&pWorkflow);
	XUI_TEST_CHECK(iRet == XUI_OK && pWorkflow != NULL, "create workflow");
	iRet = register_test_types(pWorkflow);
	XUI_TEST_CHECK(iRet == XUI_OK, "register test types");
	iRet = xuiWorkflowAddNode(pWorkflow, "start", "n_start", "Start", 20.0f, 40.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add start");
	iRet = xuiWorkflowAddNode(pWorkflow, "llm", "n_llm", "LLM", 220.0f, 40.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add llm");
	iRet = xuiWorkflowAddNode(pWorkflow, "condition", "n_condition", "Condition", 420.0f, 40.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add condition");
	iRet = xuiWorkflowAddNode(pWorkflow, "end", "n_end", "End", 620.0f, 40.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add end");
	iRet = xuiWorkflowConnect(pWorkflow, "e_start_llm", "n_start", "out", "n_llm", "in", NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "connect start to llm");
	iRet = xuiWorkflowConnect(pWorkflow, "e_llm_condition", "n_llm", "out", "n_condition", "in", NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "connect llm to condition");
	iRet = xuiWorkflowConnect(pWorkflow, "e_condition_end", "n_condition", "true", "n_end", "in", NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "connect condition to end");

	pConfig = xvoCreateTable();
	XUI_TEST_CHECK(pConfig != NULL, "create config");
	XUI_TEST_CHECK(xvoTableSetText(pConfig, "prompt", 0, "hello", 5, FALSE), "set config prompt");
	pGraph = xuiWorkflowGetGraph(pWorkflow);
	iRet = xuiFlowGraphSetEdgeRoute(pGraph, "e_condition_end", XUI_FLOW_ROUTE_ORTHOGONAL, 0.65f, 12.0f, -8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set edge route hints");
	iRet = xuiFlowGraphSetNodeConfig(pGraph, "n_llm", pConfig);
	XUI_TEST_CHECK(iRet == XUI_OK, "set node config");
	xvoUnref(pConfig);
	pConfig = NULL;

	pDefault = xvoCreateText("seed", 4, FALSE);
	XUI_TEST_CHECK(pDefault != NULL, "create variable default");
	memset(&tVariable, 0, sizeof(tVariable));
	tVariable.iSize = sizeof(tVariable);
	tVariable.sId = "var_prompt";
	tVariable.sTitle = "Prompt";
	tVariable.sType = "string";
	tVariable.sScope = "workflow";
	tVariable.pDefaultValue = pDefault;
	iRet = xuiWorkflowAddVariable(pWorkflow, &tVariable, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add variable");
	xvoUnref(pDefault);
	pDefault = NULL;

	memset(&tViewport, 0, sizeof(tViewport));
	tViewport.iSize = sizeof(tViewport);
	tViewport.fPanX = 12.0f;
	tViewport.fPanY = 24.0f;
	tViewport.fZoom = 1.25f;
	tViewport.fWidth = 800.0f;
	tViewport.fHeight = 600.0f;
	iRet = xuiFlowGraphSetViewport(pGraph, &tViewport);
	XUI_TEST_CHECK(iRet == XUI_OK, "set viewport");

	memset(&tBrokenEdge, 0, sizeof(tBrokenEdge));
	tBrokenEdge.iSize = sizeof(tBrokenEdge);
	tBrokenEdge.sId = "e_broken";
	tBrokenEdge.iKind = XUI_FLOW_PORT_CONTROL;
	tBrokenEdge.sFromNode = "n_condition";
	tBrokenEdge.sFromPort = "missing_out";
	tBrokenEdge.sToNode = "n_end";
	tBrokenEdge.sToPort = "in";
	iRet = xuiFlowGraphAddEdgePreserveInvalid(pGraph, &tBrokenEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add preserved invalid edge");
	memset(&tNodeRun, 0, sizeof(tNodeRun));
	tNodeRun.iSize = sizeof(tNodeRun);
	tNodeRun.sNodeId = "n_llm";
	tNodeRun.iState = XUI_WORKFLOW_NODE_RUN_RUNNING;
	tNodeRun.sPreview = "streaming";
	iRet = xuiWorkflowSetNodeRunState(pWorkflow, &tNodeRun);
	XUI_TEST_CHECK(iRet == XUI_OK, "set node run state");
	memset(&tNodeRun, 0, sizeof(tNodeRun));
	iRet = xuiWorkflowGetNodeRunState(pWorkflow, "n_llm", &tNodeRun);
	XUI_TEST_CHECK(iRet == XUI_OK && tNodeRun.iState == XUI_WORKFLOW_NODE_RUN_RUNNING && strcmp(tNodeRun.sPreview, "streaming") == 0, "get node run state");
	memset(&tEdgeRun, 0, sizeof(tEdgeRun));
	tEdgeRun.iSize = sizeof(tEdgeRun);
	tEdgeRun.sEdgeId = "e_condition_end";
	tEdgeRun.iState = XUI_WORKFLOW_EDGE_RUN_TAKEN;
	tEdgeRun.sPreview = "true";
	iRet = xuiWorkflowSetEdgeRunState(pWorkflow, &tEdgeRun);
	XUI_TEST_CHECK(iRet == XUI_OK, "set edge run state");
	memset(&tEdgeRun, 0, sizeof(tEdgeRun));
	iRet = xuiWorkflowGetEdgeRunState(pWorkflow, "e_condition_end", &tEdgeRun);
	XUI_TEST_CHECK(iRet == XUI_OK && tEdgeRun.iState == XUI_WORKFLOW_EDGE_RUN_TAKEN && strcmp(tEdgeRun.sPreview, "true") == 0, "get edge run state");

	iRet = xuiWorkflowSaveXSONFile(pWorkflow, "build\\workflow_roundtrip.xson");
	XUI_TEST_CHECK(iRet == XUI_OK, "save xson");
	iRet = xuiWorkflowCreate(&pLoaded);
	XUI_TEST_CHECK(iRet == XUI_OK && pLoaded != NULL, "create loaded workflow");
	iRet = register_test_types(pLoaded);
	XUI_TEST_CHECK(iRet == XUI_OK, "register loaded types");
	iRet = xuiWorkflowLoadXSONFile(pLoaded, "build\\workflow_roundtrip.xson");
	XUI_TEST_CHECK(iRet == XUI_OK, "load xson");
	XUI_TEST_CHECK(xuiWorkflowGetNodeCount(pLoaded) == 4, "loaded node count");
	XUI_TEST_CHECK(xuiFlowGraphGetEdgeCount(xuiWorkflowGetGraph(pLoaded)) == 4, "loaded edge count");
	XUI_TEST_CHECK(xuiWorkflowGetVariableCount(pLoaded) == 1, "loaded variable count");

	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	iRet = xuiFlowGraphGetNode(xuiWorkflowGetGraph(pLoaded), xuiFlowGraphFindNode(xuiWorkflowGetGraph(pLoaded), "n_llm"), &tNode);
	XUI_TEST_CHECK(iRet == XUI_OK && tNode.fX == 220.0f && tNode.fY == 40.0f, "loaded node position");
	XUI_TEST_CHECK(tNode.iRunState == XUI_WORKFLOW_NODE_RUN_RUNNING && tNode.sRunPreview != NULL && strcmp(tNode.sRunPreview, "streaming") == 0, "loaded node visual state");
	memset(&tLoadedViewport, 0, sizeof(tLoadedViewport));
	tLoadedViewport.iSize = sizeof(tLoadedViewport);
	iRet = xuiFlowGraphGetViewport(xuiWorkflowGetGraph(pLoaded), &tLoadedViewport);
	XUI_TEST_CHECK(iRet == XUI_OK && tLoadedViewport.fPanX == 12.0f && tLoadedViewport.fZoom == 1.25f, "loaded viewport");
	memset(&tEdge, 0, sizeof(tEdge));
	tEdge.iSize = sizeof(tEdge);
	iRet = xuiFlowGraphGetEdge(xuiWorkflowGetGraph(pLoaded), xuiFlowGraphFindEdge(xuiWorkflowGetGraph(pLoaded), "e_broken"), &tEdge);
	XUI_TEST_CHECK(iRet == XUI_OK && tEdge.bInvalid == 1, "loaded invalid edge preserved");
	iRet = xuiFlowGraphGetEdge(xuiWorkflowGetGraph(pLoaded), xuiFlowGraphFindEdge(xuiWorkflowGetGraph(pLoaded), "e_condition_end"), &tEdge);
	XUI_TEST_CHECK(iRet == XUI_OK && tEdge.iRunState == XUI_WORKFLOW_EDGE_RUN_TAKEN && tEdge.sRunPreview != NULL && strcmp(tEdge.sRunPreview, "true") == 0, "loaded edge visual state");
	XUI_TEST_CHECK(tEdge.iRouteStyle == XUI_FLOW_ROUTE_ORTHOGONAL && tEdge.fRouteBias > 0.64f && tEdge.fRouteBias < 0.66f && tEdge.fRouteSourceOffset == 12.0f && tEdge.fRouteTargetOffset == -8.0f, "loaded edge route hints");
	iRet = xuiWorkflowValidateGraph(pLoaded, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK && iDiagnostics > 0, "loaded invalid edge diagnostics");

cleanup:
	if ( pConfig != NULL ) xvoUnref(pConfig);
	if ( pDefault != NULL ) xvoUnref(pDefault);
	xuiWorkflowDestroy(pLoaded);
	xuiWorkflowDestroy(pWorkflow);
	return iFailed ? 1 : 0;
}
