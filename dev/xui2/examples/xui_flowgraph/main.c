#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 980
#define DEMO_H 620
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#ifndef XGE_KEY_LEFT_SHIFT
#define XGE_KEY_LEFT_SHIFT 340
#define XGE_KEY_LEFT_CONTROL 341
#define XGE_KEY_RIGHT_SHIFT 344
#define XGE_KEY_RIGHT_CONTROL 345
#endif

typedef struct xui_flowgraph_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pTitle;
	xui_widget pCanvas;
	xui_flow_graph pGraph;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bPrevLeftDown;
	int bScriptedEdit;
	int bScriptedEditDone;
	int bScriptedEditOK;
	int bCreateOK;
	int bRenderOK;
} xui_flowgraph_demo_t;

static void __xuiFlowGraphUsage(void)
{
	printf("usage: xui_flowgraph [--frames N] [--seconds N] [--scripted-edit]\n");
	printf("       no duration option means run until the window is closed.\n");
	printf("       --scripted-edit runs deterministic add/move/connect/select/delete checks.\n");
}

static int __xuiFlowGraphParseArgs(xui_flowgraph_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--scripted-edit") == 0 ) {
			pDemo->bScriptedEdit = 1;
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xuiFlowGraphUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiFlowGraphFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	FILE* pFile;
	int i;

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

static int __xuiFlowGraphAddNode(xui_flow_graph pGraph, const char* sId, const char* sType, const char* sTitle, float fX, float fY, int* pIndex)
{
	xui_flow_node_desc_t tNode;

	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	tNode.sId = sId;
	tNode.sType = sType;
	tNode.sTitle = sTitle;
	tNode.fX = fX;
	tNode.fY = fY;
	tNode.fW = 170.0f;
	tNode.fH = 96.0f;
	return xuiFlowGraphAddNode(pGraph, &tNode, pIndex);
}

static int __xuiFlowGraphAddPort(xui_flow_graph pGraph, int iNode, const char* sId, int iDirection, int iKind)
{
	xui_flow_port_desc_t tPort;

	memset(&tPort, 0, sizeof(tPort));
	tPort.iSize = sizeof(tPort);
	tPort.sId = sId;
	tPort.sTitle = sId;
	tPort.iDirection = iDirection;
	tPort.iKind = iKind;
	return xuiFlowGraphAddPort(pGraph, iNode, &tPort, NULL);
}

static int __xuiFlowGraphAddEdge(xui_flow_graph pGraph, const char* sId, const char* sFromNode, const char* sFromPort, const char* sToNode, const char* sToPort)
{
	xui_flow_edge_desc_t tEdge;

	memset(&tEdge, 0, sizeof(tEdge));
	tEdge.iSize = sizeof(tEdge);
	tEdge.sId = sId;
	tEdge.iKind = XUI_FLOW_PORT_CONTROL;
	tEdge.sFromNode = sFromNode;
	tEdge.sFromPort = sFromPort;
	tEdge.sToNode = sToNode;
	tEdge.sToPort = sToPort;
	return xuiFlowGraphAddEdge(pGraph, &tEdge, NULL);
}

static int __xuiFlowGraphBuildSample(xui_flowgraph_demo_t* pDemo)
{
	xui_flow_viewport_t tViewport;
	int iStart;
	int iLlm;
	int iCondition;
	int iEnd;
	int iFallback;
	int iRet;

	iRet = xuiFlowGraphCreate(&pDemo->pGraph);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tViewport, 0, sizeof(tViewport));
	tViewport.iSize = sizeof(tViewport);
	tViewport.fPanX = 32.0f;
	tViewport.fPanY = 30.0f;
	tViewport.fZoom = 1.0f;
	tViewport.fWidth = DEMO_W;
	tViewport.fHeight = DEMO_H;
	iRet = xuiFlowGraphSetViewport(pDemo->pGraph, &tViewport);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiFlowGraphAddNode(pDemo->pGraph, "start", "event.start", "Start", 40.0f, 210.0f, &iStart);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddNode(pDemo->pGraph, "llm", "action.llm", "LLM Prompt", 270.0f, 210.0f, &iLlm);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddNode(pDemo->pGraph, "condition", "logic.condition", "Condition", 500.0f, 210.0f, &iCondition);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddNode(pDemo->pGraph, "end", "event.end", "End", 730.0f, 140.0f, &iEnd);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddNode(pDemo->pGraph, "fallback", "event.end", "Fallback", 730.0f, 300.0f, &iFallback);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiFlowGraphAddPort(pDemo->pGraph, iStart, "out", XUI_FLOW_PORT_OUTPUT, XUI_FLOW_PORT_CONTROL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddPort(pDemo->pGraph, iLlm, "in", XUI_FLOW_PORT_INPUT, XUI_FLOW_PORT_CONTROL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddPort(pDemo->pGraph, iLlm, "out", XUI_FLOW_PORT_OUTPUT, XUI_FLOW_PORT_CONTROL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddPort(pDemo->pGraph, iCondition, "in", XUI_FLOW_PORT_INPUT, XUI_FLOW_PORT_CONTROL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddPort(pDemo->pGraph, iCondition, "true", XUI_FLOW_PORT_OUTPUT, XUI_FLOW_PORT_CONTROL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddPort(pDemo->pGraph, iCondition, "false", XUI_FLOW_PORT_OUTPUT, XUI_FLOW_PORT_CONTROL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddPort(pDemo->pGraph, iEnd, "in", XUI_FLOW_PORT_INPUT, XUI_FLOW_PORT_CONTROL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddPort(pDemo->pGraph, iFallback, "in", XUI_FLOW_PORT_INPUT, XUI_FLOW_PORT_CONTROL);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiFlowGraphAddEdge(pDemo->pGraph, "e_start_llm", "start", "out", "llm", "in");
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddEdge(pDemo->pGraph, "e_llm_condition", "llm", "out", "condition", "in");
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddEdge(pDemo->pGraph, "e_condition_end", "condition", "true", "end", "in");
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddEdge(pDemo->pGraph, "e_condition_fallback", "condition", "false", "fallback", "in");
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiFlowGraphSelectNode(pDemo->pGraph, "condition", 1);
	(void)xuiFlowGraphSelectEdge(pDemo->pGraph, "e_llm_condition", 1);
	return XUI_OK;
}

static int __xuiFlowGraphRunScriptedEdit(xui_flowgraph_demo_t* pDemo)
{
	int iScripted;
	int iRet;
	xui_flow_node_info_t tNode;
	int bMoved;
	int bEdgeSelected;
	int bNodeSelected;

	if ( (pDemo == NULL) || (pDemo->pGraph == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDemo->bScriptedEditDone ) return XUI_OK;
	pDemo->bScriptedEditDone = 1;
	pDemo->bScriptedEditOK = 0;
	iRet = __xuiFlowGraphAddNode(pDemo->pGraph, "scripted", "action.scripted", "Scripted Edit", 520.0f, 420.0f, &iScripted);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddPort(pDemo->pGraph, iScripted, "in", XUI_FLOW_PORT_INPUT, XUI_FLOW_PORT_CONTROL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiFlowGraphAddPort(pDemo->pGraph, iScripted, "out", XUI_FLOW_PORT_OUTPUT, XUI_FLOW_PORT_CONTROL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiFlowGraphSetNodePosition(pDemo->pGraph, "scripted", 560.0f, 408.0f);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	iRet = xuiFlowGraphGetNode(pDemo->pGraph, xuiFlowGraphFindNode(pDemo->pGraph, "scripted"), &tNode);
	if ( iRet != XUI_OK ) return iRet;
	bMoved = (tNode.fX == 560.0f) && (tNode.fY == 408.0f);
	iRet = __xuiFlowGraphAddEdge(pDemo->pGraph, "e_condition_scripted", "condition", "false", "scripted", "in");
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiFlowGraphClearSelection(pDemo->pGraph);
	iRet = xuiFlowGraphSelectEdge(pDemo->pGraph, "e_condition_scripted", 1);
	if ( iRet != XUI_OK ) return iRet;
	bEdgeSelected = xuiFlowGraphIsEdgeSelected(pDemo->pGraph, "e_condition_scripted");
	iRet = xuiFlowGraphDeleteSelection(pDemo->pGraph);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiFlowGraphClearSelection(pDemo->pGraph);
	iRet = xuiFlowGraphSelectNode(pDemo->pGraph, "scripted", 1);
	if ( iRet != XUI_OK ) return iRet;
	bNodeSelected = xuiFlowGraphIsNodeSelected(pDemo->pGraph, "scripted");
	iRet = xuiFlowGraphDeleteSelection(pDemo->pGraph);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bScriptedEditOK = bMoved && bEdgeSelected && bNodeSelected &&
		(xuiFlowGraphFindNode(pDemo->pGraph, "scripted") < 0) &&
		(xuiFlowGraphFindEdge(pDemo->pGraph, "e_condition_scripted") < 0) &&
		(xuiFlowGraphGetNodeCount(pDemo->pGraph) == 5) &&
		(xuiFlowGraphGetEdgeCount(pDemo->pGraph) == 4);
	return pDemo->bScriptedEditOK ? XUI_OK : XUI_ERROR_BACKEND_FAILED;
}

static int __xuiFlowGraphCreateUi(xui_flowgraph_demo_t* pDemo)
{
	xui_label_desc_t tLabel;
	xui_flow_graph_desc_t tCanvas;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H});
	if ( iRet != XUI_OK ) return iRet;

	memset(&tLabel, 0, sizeof(tLabel));
	tLabel.iSize = sizeof(tLabel);
	tLabel.sText = "FlowGraph static workflow: Start -> LLM -> Condition -> End";
	tLabel.pFont = pDemo->pFont;
	tLabel.iTextColor = XUI_COLOR_RGBA(32, 46, 66, 255);
	tLabel.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pDemo->pTitle, &tLabel);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(pDemo->pTitle, (xui_rect_t){24.0f, 16.0f, 760.0f, 28.0f});
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pTitle);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tCanvas, 0, sizeof(tCanvas));
	tCanvas.iSize = sizeof(tCanvas);
	tCanvas.pGraph = pDemo->pGraph;
	tCanvas.bOwnGraph = 0;
	iRet = xuiFlowGraphWidgetCreate(pDemo->pContext, &pDemo->pCanvas, &tCanvas);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(pDemo->pCanvas, (xui_rect_t){24.0f, 58.0f, 932.0f, 528.0f});
	if ( iRet != XUI_OK ) return iRet;
	return xuiWidgetAddChild(pDemo->pRoot, pDemo->pCanvas);
}

static int __xuiFlowGraphCreateAssets(xui_flowgraph_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_W, (float)DEMO_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_W;
	tSurfaceDesc.iHeight = DEMO_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiFlowGraphFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	iRet = __xuiFlowGraphBuildSample(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiFlowGraphCreateUi(pDemo);
}

static void __xuiFlowGraphDestroyAssets(xui_flowgraph_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) xuiDestroy(pDemo->pContext);
	if ( pDemo->pGraph != NULL ) xuiFlowGraphDestroy(pDemo->pGraph);
	if ( pDemo->pFont != NULL ) pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
	if ( pDemo->pTarget != NULL ) pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
}

static void __xuiFlowGraphHandleInput(xui_flowgraph_demo_t* pDemo)
{
	float fX;
	float fY;
	uint32_t iButtons;
	uint32_t iModifiers;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_ESCAPE, 0);
	}
	iModifiers = 0;
	if ( xgeKeyDown(XGE_KEY_LEFT_SHIFT) || xgeKeyDown(XGE_KEY_RIGHT_SHIFT) ) iModifiers |= XUI_MOD_SHIFT;
	if ( xgeKeyDown(XGE_KEY_LEFT_CONTROL) || xgeKeyDown(XGE_KEY_RIGHT_CONTROL) ) iModifiers |= XUI_MOD_CTRL;
	(void)xuiInputSetModifiers(pDemo->pContext, iModifiers);
	if ( xgeKeyPressed(XGE_KEY_DELETE) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_DELETE, iModifiers);
	if ( xgeKeyPressed(XGE_KEY_BACKSPACE) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_BACKSPACE, iModifiers);
	xgeMouseGet(&fX, &fY);
	fX -= DEMO_OFFSET_X;
	fY -= DEMO_OFFSET_Y;
	iButtons = xgeMouseDown(XGE_MOUSE_LEFT) ? XUI_POINTER_BUTTON_LEFT : 0u;
	(void)xuiInputPointerMove(pDemo->pContext, fX, fY, iButtons);
	if ( xgeMouseDown(XGE_MOUSE_LEFT) && !pDemo->bPrevLeftDown ) {
		(void)xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
	}
	if ( !xgeMouseDown(XGE_MOUSE_LEFT) && pDemo->bPrevLeftDown ) {
		(void)xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	}
	pDemo->bPrevLeftDown = xgeMouseDown(XGE_MOUSE_LEFT) ? 1 : 0;
}

static int __xuiFlowGraphFrame(void* pUser)
{
	xui_flowgraph_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;

	pDemo = (xui_flowgraph_demo_t*)pUser;
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	if ( pDemo->bScriptedEdit && !pDemo->bScriptedEditDone ) {
		iRet = __xuiFlowGraphRunScriptedEdit(pDemo);
		if ( iRet != XUI_OK ) return iRet;
	}
	__xuiFlowGraphHandleInput(pDemo);
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bCreateOK = (pDemo->pCanvas != NULL) && (xuiFlowGraphWidgetGetGraph(pDemo->pCanvas) == pDemo->pGraph) &&
	                   (xuiFlowGraphGetNodeCount(pDemo->pGraph) == 5) && (xuiFlowGraphGetEdgeCount(pDemo->pGraph) == 4) &&
	                   (!pDemo->bScriptedEdit || pDemo->bScriptedEditOK);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(224, 232, 242, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_W, DEMO_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(18, 22, 28, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_W, (float)DEMO_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XGE_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		memset(&tStats, 0, sizeof(tStats));
		tStats.iSize = sizeof(tStats);
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		pDemo->bRenderOK = (tStats.iDrawnCaches > 0);
		printf("xui_flowgraph final-summary frames=%d create=%d render=%d scriptedEdit=%d nodes=%d edges=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bRenderOK, pDemo->bScriptedEdit ? pDemo->bScriptedEditOK : 0,
			xuiFlowGraphGetNodeCount(pDemo->pGraph), xuiFlowGraphGetEdgeCount(pDemo->pGraph),
			tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_flowgraph_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiFlowGraphParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiFlowGraphUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_W + 20;
	tDesc.iHeight = DEMO_H + 40;
	tDesc.sTitle = "XUI FlowGraph";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_flowgraph: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiFlowGraphCreateAssets(&tDemo);
	if ( iRet == XGE_OK ) {
		iRet = xgeRun(__xuiFlowGraphFrame, &tDemo);
	} else {
		printf("xui_flowgraph: create failed: %d\n", iRet);
	}
	__xuiFlowGraphDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || tDemo.bRenderOK)) ? 0 : 1;
}
