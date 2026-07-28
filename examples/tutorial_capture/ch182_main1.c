/* ch182 — FlowGraph 流程图 */
#include "tut_capture_xui.h"

static int create_ui(tut_xui_ctx_t* ctx)
{
	xui_flow_graph_desc_t wdesc;
	xui_flow_node_desc_t ndesc;
	xui_flow_port_desc_t pdesc;
	xui_flow_graph pGraph;
	xui_widget pWidget;
	int iNode, iPort, ret;

	/* 创建图数据 */
	ret = xuiFlowGraphCreate(&pGraph);
	if (ret != XUI_OK) return ret;

	/* 节点 A */
	memset(&ndesc, 0, sizeof(ndesc));
	ndesc.iSize = sizeof(ndesc);
	ndesc.sId = "node_a";
	ndesc.sTitle = "Input";
	ndesc.fX = 60.0f; ndesc.fY = 100.0f;
	ndesc.fW = 140.0f; ndesc.fH = 60.0f;
	xuiFlowGraphAddNode(pGraph, &ndesc, &iNode);
	memset(&pdesc, 0, sizeof(pdesc));
	pdesc.iSize = sizeof(pdesc);
	pdesc.sId = "a_out"; pdesc.sTitle = "Out"; pdesc.iDirection = 1;
	xuiFlowGraphAddPort(pGraph, iNode, &pdesc, &iPort);

	/* 节点 B */
	ndesc.sId = "node_b";
	ndesc.sTitle = "Process";
	ndesc.fX = 300.0f; ndesc.fY = 100.0f;
	xuiFlowGraphAddNode(pGraph, &ndesc, &iNode);
	memset(&pdesc, 0, sizeof(pdesc));
	pdesc.iSize = sizeof(pdesc);
	pdesc.sId = "b_in"; pdesc.sTitle = "In"; pdesc.iDirection = 0;
	xuiFlowGraphAddPort(pGraph, iNode, &pdesc, &iPort);

	/* 创建流程图控件 */
	memset(&wdesc, 0, sizeof(wdesc));
	wdesc.iSize = sizeof(wdesc);
	wdesc.pGraph = pGraph;
	wdesc.bOwnGraph = 1;
	ret = xuiFlowGraphWidgetCreate(ctx->pContext, &pWidget, &wdesc);
	if (ret != XUI_OK) return ret;
	xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FILL);
	return xuiWidgetAddChild(ctx->pRoot, pWidget);
}

int main(int argc, char** argv) { return tut_xui_run(create_ui, "ch182", argc, argv); }
