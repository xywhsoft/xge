#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_workflow_widget_test failed: %s\n", (msg)); \
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

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget_type pType;
	xui_widget pRoot;
	xui_widget pWidget;
	xui_widget pCanvas;
	xui_workflow pWorkflow;
	xui_workflow pOtherWorkflow;
	xui_workflow_desc_t tDesc;
	xui_workflow_node_type_desc_t tType;
	xui_flow_port_desc_t tPort;
	xui_surface pCache;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pWidget = NULL;
	pWorkflow = NULL;
	pOtherWorkflow = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 640.0f, 360.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	pType = xuiWorkflowGetType(pContext);
	XUI_TEST_CHECK(pType != NULL && strcmp(xuiWidgetTypeGetName(pType), "workflow") == 0, "workflow type register");
	XUI_TEST_CHECK(xuiWorkflowGetType(pContext) == pType, "workflow type reuse");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 640.0f, 360.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	iRet = xuiWorkflowCreate(&pWorkflow);
	XUI_TEST_CHECK(iRet == XUI_OK && pWorkflow != NULL, "workflow create");
	make_control_port(&tPort, "out", XUI_FLOW_PORT_OUTPUT);
	memset(&tType, 0, sizeof(tType));
	tType.iSize = sizeof(tType);
	tType.sId = "start";
	tType.iVersion = 1;
	tType.sTitle = "Start";
	tType.sCategory = "Flow";
	tType.pOutputs = &tPort;
	tType.iOutputCount = 1;
	iRet = xuiWorkflowRegisterNodeType(pWorkflow, &tType, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "register node type");
	iRet = xuiWorkflowAddNode(pWorkflow, "start", "n_start", NULL, 64.0f, 80.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "workflow add node");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pWorkflow = pWorkflow;
	tDesc.bOwnWorkflow = 0;
	tDesc.tGraph.iSize = sizeof(tDesc.tGraph);
	iRet = xuiWorkflowWidgetCreate(pContext, &pWidget, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pWidget != NULL, "workflow widget create");
	XUI_TEST_CHECK(xuiWorkflowWidgetGetWorkflow(pWidget) == pWorkflow, "workflow widget model");
	pCanvas = xuiWorkflowWidgetGetCanvas(pWidget);
	XUI_TEST_CHECK(pCanvas != NULL && xuiFlowGraphWidgetGetGraph(pCanvas) == xuiWorkflowGetGraph(pWorkflow), "workflow widget canvas graph");
	iRet = xuiWidgetSetRect(pWidget, (xui_rect_t){0.0f, 0.0f, 640.0f, 360.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "workflow widget rect");
	iRet = xuiWidgetAddChild(pRoot, pWidget);
	XUI_TEST_CHECK(iRet == XUI_OK, "workflow widget add");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");
	pCache = xuiWidgetGetCacheSurface(pCanvas, xuiWidgetGetStateId(pCanvas));
	XUI_TEST_CHECK(pCache != NULL && xuiTestSurfaceGetDrawCount(pCache) > 0, "workflow canvas render");

	iRet = xuiWorkflowCreate(&pOtherWorkflow);
	XUI_TEST_CHECK(iRet == XUI_OK && pOtherWorkflow != NULL, "other workflow create");
	iRet = xuiWorkflowWidgetSetWorkflow(pWidget, pOtherWorkflow, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWorkflowWidgetGetWorkflow(pWidget) == pOtherWorkflow, "workflow widget set model");
	XUI_TEST_CHECK(xuiFlowGraphWidgetGetGraph(pCanvas) == xuiWorkflowGetGraph(pOtherWorkflow), "workflow canvas graph swapped");
	XUI_TEST_CHECK((xuiWidgetGetDirtyFlags(pWidget) & XUI_WIDGET_DIRTY_CACHE) != 0, "workflow widget cache dirty after model swap");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pWorkflow != NULL ) {
		xuiWorkflowDestroy(pWorkflow);
	}
	if ( pOtherWorkflow != NULL ) {
		xuiWorkflowDestroy(pOtherWorkflow);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_workflow_widget_test passed\n");
	return 0;
}
