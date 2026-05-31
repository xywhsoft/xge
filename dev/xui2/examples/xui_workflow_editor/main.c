#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 1180
#define DEMO_H 700
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define WORKFLOW_TOOL_VALIDATE 100
#define WORKFLOW_TOOL_SAVE 101
#define WORKFLOW_TOOL_LOAD 102
#define WORKFLOW_TOOL_RUN 103
#define WORKFLOW_CONTEXT_VALIDATE 200
#define WORKFLOW_CONTEXT_INSPECT 201
#define WORKFLOW_CONTEXT_DELETE 202
#define WORKFLOW_CONTEXT_CONNECT 203
#define WORKFLOW_CONTEXT_ROUTE 204
#define WORKFLOW_CONTEXT_ADD_LLM 205
#define WORKFLOW_CONTEXT_ADD_HTTP 206
#ifndef XGE_KEY_LEFT_SHIFT
#define XGE_KEY_LEFT_SHIFT 340
#define XGE_KEY_LEFT_CONTROL 341
#define XGE_KEY_RIGHT_SHIFT 344
#define XGE_KEY_RIGHT_CONTROL 345
#endif

typedef struct xui_workflow_editor_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pTitle;
	xui_widget pLibrary;
	xui_widget pToolbar;
	xui_widget pWorkflowWidget;
	xui_widget pProperties;
	xui_widget pDiagnostics;
	xui_widget pCanvasMenu;
	xui_widget pNodeMenu;
	xui_widget pEdgeMenu;
	xui_widget pPortMenu;
	xui_workflow pWorkflow;
	xui_flow_hit_t tContextHit;
	const char* sToolbarXsonPath;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bPrevLeftDown;
	int bPrevRightDown;
	int bSaveLoadOK;
	int bRunPlaybackOK;
	int bCreateOK;
	int bRenderOK;
	int bToolbarValidateOK;
	int bToolbarSaveOK;
	int bToolbarLoadOK;
	int bToolbarRunOK;
	int bToolbarSmokeDone;
	int iToolbarActionCount;
	int bContextCanvasOK;
	int bContextNodeOK;
	int bContextEdgeOK;
	int bContextPortOK;
	int bContextSmokeDone;
	int iContextMenuOpenCount;
	int iContextCommandCount;
} xui_workflow_editor_demo_t;

static int __xuiWorkflowEditorSaveXSON(xui_workflow pWorkflow, const char* sPrimaryPath, const char* sFallbackPath, const char** ppSavedPath);
static int __xuiWorkflowEditorLoadXSON(xui_workflow pWorkflow, const char* sPrimaryPath, const char* sFallbackPath);

static void __xuiWorkflowEditorUsage(void)
{
	printf("usage: xui_workflow_editor [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiWorkflowEditorParseArgs(xui_workflow_editor_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; ++i ) {
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
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__xuiWorkflowEditorUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiWorkflowEditorFindTtf(void)
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

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); ++i ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

static void __xuiWorkflowEditorMakeControlPort(xui_flow_port_desc_t* pPort, const char* sId, int iDirection)
{
	memset(pPort, 0, sizeof(*pPort));
	pPort->iSize = sizeof(*pPort);
	pPort->sId = sId;
	pPort->sTitle = sId;
	pPort->iDirection = iDirection;
	pPort->iKind = XUI_FLOW_PORT_CONTROL;
}

static int __xuiWorkflowEditorRegisterType(xui_workflow pWorkflow, const char* sId, const char* sTitle, const char* sCategory, const xui_flow_port_desc_t* pInputs, int iInputCount, const xui_flow_port_desc_t* pOutputs, int iOutputCount)
{
	xui_workflow_node_type_desc_t tType;

	memset(&tType, 0, sizeof(tType));
	tType.iSize = sizeof(tType);
	tType.sId = sId;
	tType.iVersion = 1;
	tType.sTitle = sTitle;
	tType.sCategory = sCategory;
	tType.pInputs = pInputs;
	tType.iInputCount = iInputCount;
	tType.pOutputs = pOutputs;
	tType.iOutputCount = iOutputCount;
	return xuiWorkflowRegisterNodeType(pWorkflow, &tType, NULL);
}

static int __xuiWorkflowEditorRegisterTypes(xui_workflow pWorkflow)
{
	xui_flow_port_desc_t tStartOut;
	xui_flow_port_desc_t tIn;
	xui_flow_port_desc_t tOut;
	xui_flow_port_desc_t tTrue;
	xui_flow_port_desc_t tFalse;
	xui_flow_port_desc_t arrConditionOut[2];
	int iRet;

	__xuiWorkflowEditorMakeControlPort(&tStartOut, "out", XUI_FLOW_PORT_OUTPUT);
	iRet = __xuiWorkflowEditorRegisterType(pWorkflow, "start", "Start", "Flow", NULL, 0, &tStartOut, 1);
	if ( iRet != XUI_OK ) return iRet;
	__xuiWorkflowEditorMakeControlPort(&tIn, "in", XUI_FLOW_PORT_INPUT);
	__xuiWorkflowEditorMakeControlPort(&tOut, "out", XUI_FLOW_PORT_OUTPUT);
	iRet = __xuiWorkflowEditorRegisterType(pWorkflow, "llm.mock", "LLM Mock", "Action", &tIn, 1, &tOut, 1);
	if ( iRet != XUI_OK ) return iRet;
	__xuiWorkflowEditorMakeControlPort(&tTrue, "true", XUI_FLOW_PORT_OUTPUT);
	__xuiWorkflowEditorMakeControlPort(&tFalse, "false", XUI_FLOW_PORT_OUTPUT);
	arrConditionOut[0] = tTrue;
	arrConditionOut[1] = tFalse;
	iRet = __xuiWorkflowEditorRegisterType(pWorkflow, "condition", "Condition", "Logic", &tIn, 1, arrConditionOut, 2);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiWorkflowEditorRegisterType(pWorkflow, "http.mock", "HTTP Mock", "Action", &tIn, 1, &tOut, 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiWorkflowEditorRegisterType(pWorkflow, "variable.mock", "Variable Mock", "Data", &tIn, 1, &tOut, 1);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiWorkflowEditorRegisterType(pWorkflow, "end", "End", "Flow", &tIn, 1, NULL, 0);
}

static int __xuiWorkflowEditorBuildSample(xui_workflow pWorkflow)
{
	xui_flow_viewport_t tViewport;
	xui_workflow_variable_desc_t tVariable;
	xvalue pDefault;
	int iDiagnostics;
	int iRet;

	memset(&tViewport, 0, sizeof(tViewport));
	tViewport.iSize = sizeof(tViewport);
	tViewport.fPanX = 20.0f;
	tViewport.fPanY = 120.0f;
	tViewport.fZoom = 1.0f;
	tViewport.fWidth = 760.0f;
	tViewport.fHeight = 560.0f;
	iRet = xuiFlowGraphSetViewport(xuiWorkflowGetGraph(pWorkflow), &tViewport);
	if ( iRet != XUI_OK ) { printf("sample stage viewport failed: %d\n", iRet); return iRet; }

	iRet = xuiWorkflowAddNode(pWorkflow, "start", "start", "Start", 30.0f, 130.0f, NULL);
	if ( iRet != XUI_OK ) { printf("sample stage add start failed: %d\n", iRet); return iRet; }
	iRet = xuiWorkflowAddNode(pWorkflow, "llm.mock", "llm", "LLM Mock", 290.0f, 130.0f, NULL);
	if ( iRet != XUI_OK ) { printf("sample stage add llm failed: %d\n", iRet); return iRet; }
	iRet = xuiWorkflowAddNode(pWorkflow, "condition", "condition", "Condition", 550.0f, 130.0f, NULL);
	if ( iRet != XUI_OK ) { printf("sample stage add condition failed: %d\n", iRet); return iRet; }
	iRet = xuiWorkflowAddNode(pWorkflow, "end", "end", "End", 810.0f, 40.0f, NULL);
	if ( iRet != XUI_OK ) { printf("sample stage add end failed: %d\n", iRet); return iRet; }
	iRet = xuiWorkflowAddNode(pWorkflow, "http.mock", "http", "HTTP Mock", 810.0f, 220.0f, NULL);
	if ( iRet != XUI_OK ) { printf("sample stage add http failed: %d\n", iRet); return iRet; }
	iRet = xuiWorkflowConnect(pWorkflow, "e_start_llm", "start", "out", "llm", "in", NULL);
	if ( iRet != XUI_OK ) { printf("sample stage connect start failed: %d\n", iRet); return iRet; }
	iRet = xuiWorkflowConnect(pWorkflow, "e_llm_condition", "llm", "out", "condition", "in", NULL);
	if ( iRet != XUI_OK ) { printf("sample stage connect llm failed: %d\n", iRet); return iRet; }
	iRet = xuiWorkflowConnect(pWorkflow, "e_condition_end", "condition", "true", "end", "in", NULL);
	if ( iRet != XUI_OK ) { printf("sample stage connect end failed: %d\n", iRet); return iRet; }
	iRet = xuiWorkflowConnect(pWorkflow, "e_condition_http", "condition", "false", "http", "in", NULL);
	if ( iRet != XUI_OK ) { printf("sample stage connect http failed: %d\n", iRet); return iRet; }
	(void)xuiFlowGraphSetEdgeRoute(xuiWorkflowGetGraph(pWorkflow), "e_start_llm", XUI_FLOW_ROUTE_STRAIGHT, 0.0f, 0.0f, 0.0f);
	(void)xuiFlowGraphSetEdgeRoute(xuiWorkflowGetGraph(pWorkflow), "e_llm_condition", XUI_FLOW_ROUTE_BEZIER, 0.0f, 0.0f, 0.0f);
	(void)xuiFlowGraphSetEdgeRoute(xuiWorkflowGetGraph(pWorkflow), "e_condition_end", XUI_FLOW_ROUTE_AUTO, 0.0f, 0.0f, 0.0f);
	(void)xuiFlowGraphSetEdgeRoute(xuiWorkflowGetGraph(pWorkflow), "e_condition_http", XUI_FLOW_ROUTE_ORTHOGONAL, 0.0f, 0.0f, 0.0f);
	(void)xuiWorkflowSelectNode(pWorkflow, "condition", 1);

	pDefault = xvoCreateText("user_prompt", 11, FALSE);
	if ( pDefault == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(&tVariable, 0, sizeof(tVariable));
	tVariable.iSize = sizeof(tVariable);
	tVariable.sId = "var_prompt";
	tVariable.sTitle = "Prompt";
	tVariable.sType = "string";
	tVariable.sScope = "workflow";
	tVariable.pDefaultValue = pDefault;
	iRet = xuiWorkflowAddVariable(pWorkflow, &tVariable, NULL);
	xvoUnref(pDefault);
	if ( iRet != XUI_OK ) { printf("sample stage add variable failed: %d\n", iRet); return iRet; }
	iDiagnostics = 0;
	iRet = xuiWorkflowValidateGraph(pWorkflow, &iDiagnostics);
	if ( iRet != XUI_OK ) { printf("sample stage validate failed: %d\n", iRet); return iRet; }
	return iRet;
}

static int __xuiWorkflowEditorCreateWorkflow(xui_workflow_editor_demo_t* pDemo)
{
	xui_workflow pInitial;
	xui_workflow pLoaded;
	const char* sSamplePath;
	int iRet;

	pInitial = NULL;
	pLoaded = NULL;
	sSamplePath = "build\\workflow_editor_sample.xson";
	iRet = xuiWorkflowCreate(&pInitial);
	if ( iRet != XUI_OK ) { printf("workflow stage create initial failed: %d\n", iRet); return iRet; }
	iRet = __xuiWorkflowEditorRegisterTypes(pInitial);
	if ( iRet != XUI_OK ) { printf("workflow stage register initial failed: %d\n", iRet); goto cleanup; }
	iRet = __xuiWorkflowEditorBuildSample(pInitial);
	if ( iRet != XUI_OK ) { printf("workflow stage build sample failed: %d\n", iRet); goto cleanup; }
	iRet = __xuiWorkflowEditorSaveXSON(pInitial, "build\\workflow_editor_sample.xson", "workflow_editor_sample.xson", &sSamplePath);
	if ( iRet != XUI_OK ) { printf("workflow stage save failed: %d\n", iRet); goto cleanup; }
	if ( iRet == XUI_OK ) {
		iRet = xuiWorkflowCreate(&pLoaded);
		if ( iRet != XUI_OK ) { printf("workflow stage create loaded failed: %d\n", iRet); goto cleanup; }
		iRet = __xuiWorkflowEditorRegisterTypes(pLoaded);
		if ( iRet != XUI_OK ) { printf("workflow stage register loaded failed: %d\n", iRet); goto cleanup; }
		iRet = xuiWorkflowLoadXSONFile(pLoaded, sSamplePath);
		if ( iRet != XUI_OK ) { printf("workflow stage load failed: %d\n", iRet); goto cleanup; }
	}
	if ( iRet == XUI_OK ) {
		pDemo->bSaveLoadOK = (xuiWorkflowGetNodeCount(pLoaded) == 5 && xuiFlowGraphGetEdgeCount(xuiWorkflowGetGraph(pLoaded)) == 4);
		pDemo->pWorkflow = pLoaded;
		pLoaded = NULL;
	}
cleanup:
	xuiWorkflowDestroy(pInitial);
	xuiWorkflowDestroy(pLoaded);
	return iRet;
}

static int __xuiWorkflowEditorCreateLabel(xui_workflow_editor_demo_t* pDemo, xui_widget* ppWidget, const char* sText, xui_rect_t tRect, uint32_t iColor)
{
	xui_label_desc_t tLabel;
	int iRet;

	memset(&tLabel, 0, sizeof(tLabel));
	tLabel.iSize = sizeof(tLabel);
	tLabel.sText = sText;
	tLabel.iTextColor = iColor;
	tLabel.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP;
	tLabel.iWrapMode = XUI_TEXT_WRAP_WORD;
	iRet = xuiLabelCreate(pDemo->pContext, ppWidget, &tLabel);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(*ppWidget, tRect);
	if ( iRet != XUI_OK ) return iRet;
	return xuiWidgetAddChild(pDemo->pRoot, *ppWidget);
}

static void __xuiWorkflowEditorSetDiagnostics(xui_workflow_editor_demo_t* pDemo, const char* sText)
{
	if ( (pDemo != NULL) && (pDemo->pDiagnostics != NULL) && (sText != NULL) ) {
		(void)xuiLabelSetText(pDemo->pDiagnostics, sText);
	}
}

static int __xuiWorkflowEditorSmokeEnabled(const xui_workflow_editor_demo_t* pDemo)
{
	return (pDemo != NULL) && ((pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0));
}

static int __xuiWorkflowEditorSaveXSON(xui_workflow pWorkflow, const char* sPrimaryPath, const char* sFallbackPath, const char** ppSavedPath)
{
	int iRet;

	if ( ppSavedPath != NULL ) {
		*ppSavedPath = sPrimaryPath;
	}
	iRet = xuiWorkflowSaveXSONFile(pWorkflow, sPrimaryPath);
	if ( (iRet != XUI_OK) && (sFallbackPath != NULL) ) {
		iRet = xuiWorkflowSaveXSONFile(pWorkflow, sFallbackPath);
		if ( (iRet == XUI_OK) && (ppSavedPath != NULL) ) {
			*ppSavedPath = sFallbackPath;
		}
	}
	return iRet;
}

static int __xuiWorkflowEditorLoadXSON(xui_workflow pWorkflow, const char* sPrimaryPath, const char* sFallbackPath)
{
	int iRet;

	iRet = xuiWorkflowLoadXSONFile(pWorkflow, sPrimaryPath);
	if ( (iRet != XUI_OK) && (sFallbackPath != NULL) ) {
		iRet = xuiWorkflowLoadXSONFile(pWorkflow, sFallbackPath);
	}
	return iRet;
}

static int __xuiWorkflowEditorDoToolbarAction(xui_workflow_editor_demo_t* pDemo, int iValue)
{
	xui_workflow pLoaded;
	char sText[160];
	int iDiagnostics;
	int iRet;

	if ( (pDemo == NULL) || (pDemo->pWorkflow == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDemo->iToolbarActionCount++;
	if ( iValue == WORKFLOW_TOOL_VALIDATE ) {
		iDiagnostics = 0;
		iRet = xuiWorkflowValidateGraph(pDemo->pWorkflow, &iDiagnostics);
		if ( iRet == XUI_OK ) {
			pDemo->bToolbarValidateOK = 1;
			snprintf(sText, sizeof(sText), "Diagnostics: validated, issues=%d", iDiagnostics);
			__xuiWorkflowEditorSetDiagnostics(pDemo, sText);
		}
		return iRet;
	}
	if ( iValue == WORKFLOW_TOOL_SAVE ) {
		iRet = __xuiWorkflowEditorSaveXSON(pDemo->pWorkflow, "build\\workflow_editor_toolbar.xson", "workflow_editor_toolbar.xson", &pDemo->sToolbarXsonPath);
		if ( iRet == XUI_OK ) {
			pDemo->bToolbarSaveOK = 1;
			__xuiWorkflowEditorSetDiagnostics(pDemo, "Diagnostics: toolbar save complete");
		}
		return iRet;
	}
	if ( iValue == WORKFLOW_TOOL_LOAD ) {
		pLoaded = NULL;
		iRet = xuiWorkflowCreate(&pLoaded);
		if ( iRet == XUI_OK ) iRet = __xuiWorkflowEditorRegisterTypes(pLoaded);
		if ( iRet == XUI_OK ) {
			if ( pDemo->sToolbarXsonPath != NULL ) {
				iRet = xuiWorkflowLoadXSONFile(pLoaded, pDemo->sToolbarXsonPath);
			} else {
				iRet = __xuiWorkflowEditorLoadXSON(pLoaded, "build\\workflow_editor_toolbar.xson", "workflow_editor_toolbar.xson");
			}
		}
		if ( iRet == XUI_OK && pDemo->pWorkflowWidget != NULL ) {
			iRet = xuiWorkflowWidgetSetWorkflow(pDemo->pWorkflowWidget, pLoaded, 0);
		}
		if ( iRet == XUI_OK ) {
			xuiWorkflowDestroy(pDemo->pWorkflow);
			pDemo->pWorkflow = pLoaded;
			pLoaded = NULL;
			pDemo->bToolbarLoadOK = (xuiWorkflowGetNodeCount(pDemo->pWorkflow) == 5);
			__xuiWorkflowEditorSetDiagnostics(pDemo, "Diagnostics: toolbar load complete");
		}
		xuiWorkflowDestroy(pLoaded);
		return iRet;
	}
	if ( iValue == WORKFLOW_TOOL_RUN ) {
		pDemo->iFrame = 0;
		pDemo->bRunPlaybackOK = 0;
		pDemo->bToolbarRunOK = 1;
		__xuiWorkflowEditorSetDiagnostics(pDemo, "Diagnostics: mock run restarted");
		return XUI_OK;
	}
	return XUI_ERROR_UNSUPPORTED;
}

static void __xuiWorkflowEditorToolbarSelect(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_workflow_editor_demo_t* pDemo;
	int iRet;

	(void)pWidget;
	(void)iIndex;
	pDemo = (xui_workflow_editor_demo_t*)pUser;
	iRet = __xuiWorkflowEditorDoToolbarAction(pDemo, iValue);
	if ( iRet != XUI_OK ) {
		__xuiWorkflowEditorSetDiagnostics(pDemo, "Diagnostics: toolbar action failed");
	}
}

static int __xuiWorkflowEditorCreateToolbar(xui_workflow_editor_demo_t* pDemo)
{
	xui_toolbar_desc_t tDesc;
	xui_toolbar_metrics_t tMetrics;
	xui_toolbar_colors_t tColors;
	xui_toolbar_item_t arrItems[4];
	int iRet;

	memset(arrItems, 0, sizeof(arrItems));
	arrItems[0] = (xui_toolbar_item_t){"Validate", "Validate workflow", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, WORKFLOW_TOOL_VALIDATE, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[1] = (xui_toolbar_item_t){"Save", "Save XSON", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, WORKFLOW_TOOL_SAVE, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[2] = (xui_toolbar_item_t){"Load", "Load XSON", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, WORKFLOW_TOOL_LOAD, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[3] = (xui_toolbar_item_t){"Run", "Run mock workflow", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, WORKFLOW_TOOL_RUN, 1, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	memset(&tMetrics, 0, sizeof(tMetrics));
	tMetrics.iSize = sizeof(tMetrics);
	tMetrics.iOrientation = XUI_ORIENTATION_HORIZONTAL;
	tMetrics.fItemWidth = 84.0f;
	tMetrics.fItemHeight = 28.0f;
	tMetrics.fSeparatorSize = 8.0f;
	tMetrics.fGroupGap = 6.0f;
	tMetrics.fPaddingX = 4.0f;
	tMetrics.fPaddingY = 3.0f;
	tMetrics.fOverflowSize = 28.0f;
	tMetrics.fRadius = 4.0f;
	tMetrics.fBorderWidth = 1.0f;
	tMetrics.fIconSize = 0.0f;
	tMetrics.fIconGap = 5.0f;
	memset(&tColors, 0, sizeof(tColors));
	tColors.iSize = sizeof(tColors);
	tColors.iBackgroundColor = XUI_COLOR_RGBA(238, 244, 250, 255);
	tColors.iBorderColor = XUI_COLOR_RGBA(190, 204, 220, 255);
	tColors.iHoverColor = XUI_COLOR_RGBA(222, 236, 250, 255);
	tColors.iActiveColor = XUI_COLOR_RGBA(200, 224, 248, 255);
	tColors.iFocusColor = XUI_COLOR_RGBA(42, 134, 230, 255);
	tColors.iDisabledColor = XUI_COLOR_RGBA(226, 230, 236, 255);
	tColors.iTextColor = XUI_COLOR_RGBA(42, 68, 96, 255);
	tColors.iDisabledTextColor = XUI_COLOR_RGBA(132, 142, 154, 255);
	tColors.iIconColor = XUI_COLOR_RGBA(42, 68, 96, 255);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pItems = arrItems;
	tDesc.iItemCount = 4;
	tDesc.tMetrics = tMetrics;
	tDesc.tColors = tColors;
	tDesc.bHasMetrics = 1;
	tDesc.bHasColors = 1;
	iRet = xuiToolbarCreate(pDemo->pContext, &pDemo->pToolbar, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(pDemo->pToolbar, (xui_rect_t){292.0f, 12.0f, 390.0f, 34.0f});
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pToolbar);
	if ( iRet != XUI_OK ) return iRet;
	return xuiToolbarSetSelect(pDemo->pToolbar, __xuiWorkflowEditorToolbarSelect, pDemo);
}

static const char* __xuiWorkflowEditorHitName(int iType)
{
	if ( iType == XUI_FLOW_HIT_PORT ) return "port";
	if ( iType == XUI_FLOW_HIT_EDGE ) return "edge";
	if ( iType == XUI_FLOW_HIT_NODE ) return "node";
	if ( iType == XUI_FLOW_HIT_BACKGROUND ) return "canvas";
	return "none";
}

static void __xuiWorkflowEditorContextSelect(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_workflow_editor_demo_t* pDemo;
	char sText[192];

	(void)pWidget;
	(void)iIndex;
	pDemo = (xui_workflow_editor_demo_t*)pUser;
	if ( pDemo == NULL ) {
		return;
	}
	pDemo->iContextCommandCount++;
	snprintf(sText, sizeof(sText), "Diagnostics: context %s command=%d node=%d port=%d edge=%d",
	         __xuiWorkflowEditorHitName(pDemo->tContextHit.iType), iValue,
	         pDemo->tContextHit.iNode, pDemo->tContextHit.iPort, pDemo->tContextHit.iEdge);
	__xuiWorkflowEditorSetDiagnostics(pDemo, sText);
}

static int __xuiWorkflowEditorCreateContextMenu(xui_workflow_editor_demo_t* pDemo, xui_widget* ppMenu, const xui_menu_item_t* pItems, int iCount)
{
	xui_menu_desc_t tDesc;
	xui_widget pOwner;
	int iRet;

	pOwner = xuiWorkflowWidgetGetCanvas(pDemo->pWorkflowWidget);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pOwner;
	iRet = xuiMenuCreate(pDemo->pContext, ppMenu, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiMenuSetItems(*ppMenu, pItems, iCount);
	if ( iRet != XUI_OK ) return iRet;
	return xuiMenuSetSelect(*ppMenu, __xuiWorkflowEditorContextSelect, pDemo);
}

static int __xuiWorkflowEditorCreateContextMenus(xui_workflow_editor_demo_t* pDemo)
{
	xui_menu_item_t arrCanvas[4];
	xui_menu_item_t arrNode[4];
	xui_menu_item_t arrEdge[4];
	xui_menu_item_t arrPort[3];
	int iRet;

	memset(arrCanvas, 0, sizeof(arrCanvas));
	arrCanvas[0] = (xui_menu_item_t){"Validate Canvas", "V", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DEFAULT, WORKFLOW_CONTEXT_VALIDATE, 0, NULL, NULL};
	arrCanvas[1] = (xui_menu_item_t){"Add LLM Mock", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, WORKFLOW_CONTEXT_ADD_LLM, 0, NULL, NULL};
	arrCanvas[2] = (xui_menu_item_t){"Add HTTP Mock", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, WORKFLOW_CONTEXT_ADD_HTTP, 0, NULL, NULL};
	arrCanvas[3] = (xui_menu_item_t){"Inspect Canvas", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, WORKFLOW_CONTEXT_INSPECT, 0, NULL, NULL};
	iRet = __xuiWorkflowEditorCreateContextMenu(pDemo, &pDemo->pCanvasMenu, arrCanvas, 4);
	if ( iRet != XUI_OK ) return iRet;

	memset(arrNode, 0, sizeof(arrNode));
	arrNode[0] = (xui_menu_item_t){"Inspect Node", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DEFAULT, WORKFLOW_CONTEXT_INSPECT, 0, NULL, NULL};
	arrNode[1] = (xui_menu_item_t){"Run From Node", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, WORKFLOW_TOOL_RUN, 0, NULL, NULL};
	arrNode[2].iType = XUI_MENU_ITEM_SEPARATOR;
	arrNode[3] = (xui_menu_item_t){"Delete Node", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DANGER, WORKFLOW_CONTEXT_DELETE, 0, NULL, NULL};
	iRet = __xuiWorkflowEditorCreateContextMenu(pDemo, &pDemo->pNodeMenu, arrNode, 4);
	if ( iRet != XUI_OK ) return iRet;

	memset(arrEdge, 0, sizeof(arrEdge));
	arrEdge[0] = (xui_menu_item_t){"Inspect Edge", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DEFAULT, WORKFLOW_CONTEXT_INSPECT, 0, NULL, NULL};
	arrEdge[1] = (xui_menu_item_t){"Toggle Route Bias", NULL, XUI_MENU_ITEM_CHECK, XUI_MENU_ITEM_ENABLED, WORKFLOW_CONTEXT_ROUTE, 0, NULL, NULL};
	arrEdge[2].iType = XUI_MENU_ITEM_SEPARATOR;
	arrEdge[3] = (xui_menu_item_t){"Delete Edge", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DANGER, WORKFLOW_CONTEXT_DELETE, 0, NULL, NULL};
	iRet = __xuiWorkflowEditorCreateContextMenu(pDemo, &pDemo->pEdgeMenu, arrEdge, 4);
	if ( iRet != XUI_OK ) return iRet;

	memset(arrPort, 0, sizeof(arrPort));
	arrPort[0] = (xui_menu_item_t){"Inspect Port", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DEFAULT, WORKFLOW_CONTEXT_INSPECT, 0, NULL, NULL};
	arrPort[1] = (xui_menu_item_t){"Start Connection", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, WORKFLOW_CONTEXT_CONNECT, 0, NULL, NULL};
	arrPort[2] = (xui_menu_item_t){"Validate Port", NULL, XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, WORKFLOW_CONTEXT_VALIDATE, 0, NULL, NULL};
	return __xuiWorkflowEditorCreateContextMenu(pDemo, &pDemo->pPortMenu, arrPort, 3);
}

static xui_widget __xuiWorkflowEditorMenuForHit(xui_workflow_editor_demo_t* pDemo, int iType)
{
	if ( iType == XUI_FLOW_HIT_PORT ) return pDemo->pPortMenu;
	if ( iType == XUI_FLOW_HIT_EDGE ) return pDemo->pEdgeMenu;
	if ( iType == XUI_FLOW_HIT_NODE ) return pDemo->pNodeMenu;
	return pDemo->pCanvasMenu;
}

static int __xuiWorkflowEditorOpenContextMenu(xui_workflow_editor_demo_t* pDemo, float fX, float fY)
{
	xui_widget pCanvas;
	xui_widget pMenu;
	xui_flow_hit_t tHit;
	int iRet;

	if ( (pDemo == NULL) || (pDemo->pWorkflowWidget == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pCanvas = xuiWorkflowWidgetGetCanvas(pDemo->pWorkflowWidget);
	if ( pCanvas == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tHit, 0, sizeof(tHit));
	tHit.iSize = sizeof(tHit);
	iRet = xuiFlowGraphWidgetHitTest(pCanvas, fX, fY, &tHit);
	if ( iRet != XUI_OK ) return iRet;
	pMenu = __xuiWorkflowEditorMenuForHit(pDemo, tHit.iType);
	if ( pMenu == NULL ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	pDemo->tContextHit = tHit;
	iRet = xuiMenuOpenAt(pMenu, pCanvas, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->iContextMenuOpenCount++;
	if ( tHit.iType == XUI_FLOW_HIT_PORT ) pDemo->bContextPortOK = 1;
	else if ( tHit.iType == XUI_FLOW_HIT_EDGE ) pDemo->bContextEdgeOK = 1;
	else if ( tHit.iType == XUI_FLOW_HIT_NODE ) pDemo->bContextNodeOK = 1;
	else pDemo->bContextCanvasOK = 1;
	return XUI_OK;
}

static int __xuiWorkflowEditorCreateUi(xui_workflow_editor_demo_t* pDemo)
{
	xui_workflow_desc_t tWorkflowDesc;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H});
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiWorkflowEditorCreateLabel(pDemo, &pDemo->pTitle, "Workflow Editor", (xui_rect_t){18.0f, 14.0f, 260.0f, 28.0f}, XUI_COLOR_RGBA(26, 36, 52, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiWorkflowEditorCreateToolbar(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiWorkflowEditorCreateLabel(pDemo, &pDemo->pLibrary, "Node Library\n\nStart\nLLM Mock\nCondition\nHTTP Mock\nVariable Mock\nEnd", (xui_rect_t){18.0f, 58.0f, 180.0f, 560.0f}, XUI_COLOR_RGBA(48, 64, 82, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiWorkflowEditorCreateLabel(pDemo, &pDemo->pProperties, "Properties\n\nSelected: Condition\nBranches: true / false\nVariable: var_prompt\nConfig: mock route", (xui_rect_t){996.0f, 58.0f, 166.0f, 560.0f}, XUI_COLOR_RGBA(48, 64, 82, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiWorkflowEditorCreateLabel(pDemo, &pDemo->pDiagnostics, "Diagnostics: ready", (xui_rect_t){18.0f, 632.0f, 1144.0f, 42.0f}, XUI_COLOR_RGBA(48, 64, 82, 255));
	if ( iRet != XUI_OK ) return iRet;

	memset(&tWorkflowDesc, 0, sizeof(tWorkflowDesc));
	tWorkflowDesc.iSize = sizeof(tWorkflowDesc);
	tWorkflowDesc.pWorkflow = pDemo->pWorkflow;
	tWorkflowDesc.bOwnWorkflow = 0;
	tWorkflowDesc.tGraph.iSize = sizeof(tWorkflowDesc.tGraph);
	tWorkflowDesc.tGraph.iBackgroundColor = XUI_COLOR_RGBA(244, 248, 252, 255);
	tWorkflowDesc.tGraph.iGridColor = XUI_COLOR_RGBA(210, 220, 232, 190);
	iRet = xuiWorkflowWidgetCreate(pDemo->pContext, &pDemo->pWorkflowWidget, &tWorkflowDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(pDemo->pWorkflowWidget, (xui_rect_t){214.0f, 58.0f, 766.0f, 560.0f});
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pWorkflowWidget);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiWorkflowEditorCreateContextMenus(pDemo);
}

static int __xuiWorkflowEditorCreateAssets(xui_workflow_editor_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) { printf("create stage xuiCreate failed: %d\n", iRet); return iRet; }
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) { printf("create stage xuiSetProxy failed: %d\n", iRet); return iRet; }
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_W, (float)DEMO_H);
	if ( iRet != XUI_OK ) { printf("create stage xuiInputViewport failed: %d\n", iRet); return iRet; }
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_W;
	tSurfaceDesc.iHeight = DEMO_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) { printf("create stage surfaceCreate failed: %d\n", iRet); return iRet; }
	sFontPath = __xuiWorkflowEditorFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) { printf("create stage fontLoadFile failed: %d\n", iRet); return iRet; }
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	iRet = __xuiWorkflowEditorCreateWorkflow(pDemo);
	if ( iRet != XUI_OK ) { printf("create stage workflow failed: %d\n", iRet); return iRet; }
	iRet = __xuiWorkflowEditorCreateUi(pDemo);
	if ( iRet != XUI_OK ) { printf("create stage ui failed: %d\n", iRet); return iRet; }
	return iRet;
}

static void __xuiWorkflowEditorDestroyAssets(xui_workflow_editor_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) xuiDestroy(pDemo->pContext);
	if ( pDemo->pWorkflow != NULL ) xuiWorkflowDestroy(pDemo->pWorkflow);
	if ( pDemo->pFont != NULL ) pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
	if ( pDemo->pTarget != NULL ) pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
}

static void __xuiWorkflowEditorHandleInput(xui_workflow_editor_demo_t* pDemo)
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
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	(void)xuiInputPointerMove(pDemo->pContext, fX, fY, iButtons);
	if ( xgeMouseDown(XGE_MOUSE_LEFT) && !pDemo->bPrevLeftDown ) {
		(void)xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
	}
	if ( !xgeMouseDown(XGE_MOUSE_LEFT) && pDemo->bPrevLeftDown ) {
		(void)xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	}
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) && !pDemo->bPrevRightDown ) {
		(void)__xuiWorkflowEditorOpenContextMenu(pDemo, fX, fY);
	}
	pDemo->bPrevLeftDown = xgeMouseDown(XGE_MOUSE_LEFT) ? 1 : 0;
	pDemo->bPrevRightDown = xgeMouseDown(XGE_MOUSE_RIGHT) ? 1 : 0;
}

static void __xuiWorkflowEditorRunToolbarSmoke(xui_workflow_editor_demo_t* pDemo)
{
	if ( (pDemo == NULL) || pDemo->bToolbarSmokeDone || (pDemo->pToolbar == NULL) ) {
		return;
	}
	if ( __xuiWorkflowEditorDoToolbarAction(pDemo, WORKFLOW_TOOL_VALIDATE) == XUI_OK &&
	     __xuiWorkflowEditorDoToolbarAction(pDemo, WORKFLOW_TOOL_SAVE) == XUI_OK &&
	     __xuiWorkflowEditorDoToolbarAction(pDemo, WORKFLOW_TOOL_LOAD) == XUI_OK &&
	     __xuiWorkflowEditorDoToolbarAction(pDemo, WORKFLOW_TOOL_RUN) == XUI_OK ) {
		pDemo->bToolbarSmokeDone = 1;
	}
}

static int __xuiWorkflowEditorFindHitPoint(xui_workflow_editor_demo_t* pDemo, int iType, float* pX, float* pY)
{
	xui_widget pCanvas;
	xui_rect_t tRect;
	xui_flow_hit_t tHit;
	float fStep;
	float fX;
	float fY;

	if ( (pDemo == NULL) || (pX == NULL) || (pY == NULL) ) {
		return 0;
	}
	pCanvas = xuiWorkflowWidgetGetCanvas(pDemo->pWorkflowWidget);
	if ( pCanvas == NULL ) {
		return 0;
	}
	tRect = xuiWidgetGetWorldRect(pCanvas);
	fStep = (iType == XUI_FLOW_HIT_EDGE || iType == XUI_FLOW_HIT_PORT) ? 4.0f : 10.0f;
	for ( fY = tRect.fY + 4.0f; fY < tRect.fY + tRect.fH - 4.0f; fY += fStep ) {
		for ( fX = tRect.fX + 4.0f; fX < tRect.fX + tRect.fW - 4.0f; fX += fStep ) {
			memset(&tHit, 0, sizeof(tHit));
			tHit.iSize = sizeof(tHit);
			if ( xuiFlowGraphWidgetHitTest(pCanvas, fX, fY, &tHit) == XUI_OK && tHit.iType == iType ) {
				*pX = fX;
				*pY = fY;
				return 1;
			}
		}
	}
	return 0;
}

static void __xuiWorkflowEditorRunContextSmoke(xui_workflow_editor_demo_t* pDemo)
{
	static const int arrTypes[4] = {XUI_FLOW_HIT_BACKGROUND, XUI_FLOW_HIT_NODE, XUI_FLOW_HIT_EDGE, XUI_FLOW_HIT_PORT};
	xui_widget pMenu;
	float fX;
	float fY;
	int i;

	if ( (pDemo == NULL) || pDemo->bContextSmokeDone ) {
		return;
	}
	for ( i = 0; i < 4; ++i ) {
		if ( !__xuiWorkflowEditorFindHitPoint(pDemo, arrTypes[i], &fX, &fY) ) {
			return;
		}
		if ( __xuiWorkflowEditorOpenContextMenu(pDemo, fX, fY) != XUI_OK ) {
			return;
		}
		pMenu = __xuiWorkflowEditorMenuForHit(pDemo, arrTypes[i]);
		if ( pMenu == NULL ) {
			return;
		}
		(void)xuiMenuSetHoverIndex(pMenu, 0);
		(void)xuiMenuCommitHover(pMenu);
	}
	pDemo->bContextSmokeDone = (pDemo->bContextCanvasOK && pDemo->bContextNodeOK && pDemo->bContextEdgeOK && pDemo->bContextPortOK && pDemo->iContextCommandCount >= 4);
}

static void __xuiWorkflowEditorApplyRunState(xui_workflow_editor_demo_t* pDemo)
{
	xui_workflow_node_run_state_t tNodeState;
	xui_workflow_edge_run_state_t tEdgeState;
	int iStep;

	if ( pDemo->pWorkflow == NULL ) return;
	iStep = pDemo->iFrame / 12;
	memset(&tNodeState, 0, sizeof(tNodeState));
	tNodeState.iSize = sizeof(tNodeState);
	tNodeState.sNodeId = "start";
	tNodeState.iState = (iStep >= 0) ? XUI_WORKFLOW_NODE_RUN_SUCCESS : XUI_WORKFLOW_NODE_RUN_IDLE;
	tNodeState.sPreview = "triggered";
	(void)xuiWorkflowSetNodeRunState(pDemo->pWorkflow, &tNodeState);
	tNodeState.sNodeId = "llm";
	tNodeState.iState = (iStep < 1) ? XUI_WORKFLOW_NODE_RUN_IDLE : ((iStep < 3) ? XUI_WORKFLOW_NODE_RUN_RUNNING : XUI_WORKFLOW_NODE_RUN_SUCCESS);
	tNodeState.sPreview = (iStep < 3) ? "drafting" : "answer ready";
	(void)xuiWorkflowSetNodeRunState(pDemo->pWorkflow, &tNodeState);
	tNodeState.sNodeId = "condition";
	tNodeState.iState = (iStep < 3) ? XUI_WORKFLOW_NODE_RUN_IDLE : ((iStep < 4) ? XUI_WORKFLOW_NODE_RUN_RUNNING : XUI_WORKFLOW_NODE_RUN_WARNING);
	tNodeState.sPreview = (iStep < 4) ? "checking" : "false branch";
	(void)xuiWorkflowSetNodeRunState(pDemo->pWorkflow, &tNodeState);
	tNodeState.sNodeId = "http";
	tNodeState.iState = (iStep < 4) ? XUI_WORKFLOW_NODE_RUN_IDLE : ((iStep < 5) ? XUI_WORKFLOW_NODE_RUN_RUNNING : XUI_WORKFLOW_NODE_RUN_FAILED);
	tNodeState.sPreview = (iStep < 5) ? "requesting" : "timeout";
	(void)xuiWorkflowSetNodeRunState(pDemo->pWorkflow, &tNodeState);
	tNodeState.sNodeId = "end";
	tNodeState.iState = XUI_WORKFLOW_NODE_RUN_SKIPPED;
	tNodeState.sPreview = "not reached";
	(void)xuiWorkflowSetNodeRunState(pDemo->pWorkflow, &tNodeState);

	memset(&tEdgeState, 0, sizeof(tEdgeState));
	tEdgeState.iSize = sizeof(tEdgeState);
	tEdgeState.sEdgeId = "e_start_llm";
	tEdgeState.iState = (iStep >= 1) ? XUI_WORKFLOW_EDGE_RUN_TAKEN : XUI_WORKFLOW_EDGE_RUN_IDLE;
	tEdgeState.sPreview = "go";
	(void)xuiWorkflowSetEdgeRunState(pDemo->pWorkflow, &tEdgeState);
	tEdgeState.sEdgeId = "e_llm_condition";
	tEdgeState.iState = (iStep >= 3) ? XUI_WORKFLOW_EDGE_RUN_TAKEN : XUI_WORKFLOW_EDGE_RUN_IDLE;
	(void)xuiWorkflowSetEdgeRunState(pDemo->pWorkflow, &tEdgeState);
	tEdgeState.sEdgeId = "e_condition_end";
	tEdgeState.iState = (iStep >= 4) ? XUI_WORKFLOW_EDGE_RUN_SKIPPED : XUI_WORKFLOW_EDGE_RUN_IDLE;
	tEdgeState.sPreview = "true";
	(void)xuiWorkflowSetEdgeRunState(pDemo->pWorkflow, &tEdgeState);
	tEdgeState.sEdgeId = "e_condition_http";
	tEdgeState.iState = (iStep >= 4) ? XUI_WORKFLOW_EDGE_RUN_TAKEN : XUI_WORKFLOW_EDGE_RUN_IDLE;
	tEdgeState.sPreview = "false";
	(void)xuiWorkflowSetEdgeRunState(pDemo->pWorkflow, &tEdgeState);
	pDemo->bRunPlaybackOK = (iStep >= 5);
}

static int __xuiWorkflowEditorFrame(void* pUser)
{
	xui_workflow_editor_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_workflow_node_run_state_t tFailedState;
	int iRet;

	pDemo = (xui_workflow_editor_demo_t*)pUser;
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	if ( __xuiWorkflowEditorSmokeEnabled(pDemo) ) {
		__xuiWorkflowEditorRunToolbarSmoke(pDemo);
	}
	__xuiWorkflowEditorApplyRunState(pDemo);
	__xuiWorkflowEditorHandleInput(pDemo);
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiWorkflowEditorSmokeEnabled(pDemo) ) {
		__xuiWorkflowEditorRunContextSmoke(pDemo);
	}
	pDemo->bCreateOK = (pDemo->pWorkflowWidget != NULL) && (xuiWorkflowWidgetGetWorkflow(pDemo->pWorkflowWidget) == pDemo->pWorkflow) &&
	                   (xuiWorkflowGetNodeCount(pDemo->pWorkflow) == 5) && (xuiFlowGraphGetEdgeCount(xuiWorkflowGetGraph(pDemo->pWorkflow)) == 4);
	if ( __xuiWorkflowEditorSmokeEnabled(pDemo) ) {
		pDemo->bCreateOK = pDemo->bCreateOK &&
		                   pDemo->bSaveLoadOK && pDemo->bToolbarValidateOK && pDemo->bToolbarSaveOK && pDemo->bToolbarLoadOK && pDemo->bToolbarRunOK &&
		                   pDemo->bContextCanvasOK && pDemo->bContextNodeOK && pDemo->bContextEdgeOK && pDemo->bContextPortOK;
	}
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
		memset(&tFailedState, 0, sizeof(tFailedState));
		(void)xuiWorkflowGetNodeRunState(pDemo->pWorkflow, "http", &tFailedState);
		pDemo->bRenderOK = (tStats.iDrawnCaches > 0);
		printf("xui_workflow_editor final-summary frames=%d create=%d render=%d saveLoad=%d toolbar=%d context=%d playback=%d failedVisible=%d nodes=%d edges=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bRenderOK, pDemo->bSaveLoadOK,
			(pDemo->bToolbarValidateOK && pDemo->bToolbarSaveOK && pDemo->bToolbarLoadOK && pDemo->bToolbarRunOK && pDemo->iToolbarActionCount >= 4),
			(pDemo->bContextSmokeDone && pDemo->iContextMenuOpenCount >= 4 && pDemo->iContextCommandCount >= 4),
			pDemo->bRunPlaybackOK,
			(tFailedState.iState == XUI_WORKFLOW_NODE_RUN_FAILED),
			xuiWorkflowGetNodeCount(pDemo->pWorkflow), xuiFlowGraphGetEdgeCount(xuiWorkflowGetGraph(pDemo->pWorkflow)),
			tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_workflow_editor_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiWorkflowEditorParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiWorkflowEditorUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_W + 20;
	tDesc.iHeight = DEMO_H + 40;
	tDesc.sTitle = "XUI Workflow Editor";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_workflow_editor: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiWorkflowEditorCreateAssets(&tDemo);
	if ( iRet == XGE_OK ) {
		iRet = xgeRun(__xuiWorkflowEditorFrame, &tDemo);
	} else {
		printf("xui_workflow_editor: create failed: %d\n", iRet);
	}
	__xuiWorkflowEditorDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || tDemo.bRenderOK)) ? 0 : 1;
}
