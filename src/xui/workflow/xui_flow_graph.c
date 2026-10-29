#include "xui_config.h"
#if XUI_ENABLE_FLOW_GRAPH
#include "xui.h"
#include "xui_xrt_port.h"

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define XUI_FLOW_COMMAND_HISTORY_DEFAULT_LIMIT 256

typedef struct xui_flow_port_model_t {
	char* sId;
	char* sTitle;
	char* sDataType;
	int iDirection;
	int iKind;
	int bRequired;
	int bMulti;
	int bDynamic;
} xui_flow_port_model_t;

typedef struct xui_flow_node_model_t {
	char* sId;
	char* sType;
	char* sTitle;
	char* sSummary;
	float fX;
	float fY;
	float fW;
	float fH;
	xvalue* pConfig;
	int iRunState;
	char* sRunPreview;
	xarray arrPorts;
	xmap mapPorts;
} xui_flow_node_model_t;

typedef struct xui_flow_edge_model_t {
	char* sId;
	int iKind;
	char* sFromNode;
	char* sFromPort;
	char* sToNode;
	char* sToPort;
	int iRouteStyle;
	float fRouteBias;
	float fRouteSourceOffset;
	float fRouteTargetOffset;
	int iFromNode;
	int iFromPort;
	int iToNode;
	int iToPort;
	int bInvalid;
	int iRunState;
	char* sRunPreview;
} xui_flow_edge_model_t;

typedef struct xui_flow_diagnostic_model_t {
	int iSeverity;
	char* sCode;
	char* sMessage;
	char* sNode;
	char* sEdge;
	char* sPath;
} xui_flow_diagnostic_model_t;

typedef struct xui_flow_edge_snapshot_t {
	xui_flow_edge_desc_t tEdge;
	int iRunState;
	char* sRunPreview;
} xui_flow_edge_snapshot_t;

typedef struct xui_flow_command_model_t {
	int iType;
	xui_flow_node_desc_t tNode;
	xui_flow_edge_desc_t tEdge;
	xarray arrPorts;
	xarray arrEdges;
	xarray arrMoves;
	xarray arrCommands;
	char* sNodeId;
	int iNodeRunState;
	char* sNodeRunPreview;
	int iEdgeRunState;
	char* sEdgeRunPreview;
	float fOldX;
	float fOldY;
	float fNewX;
	float fNewY;
	xvalue* pOldConfig;
	xvalue* pNewConfig;
} xui_flow_command_model_t;

struct xui_flow_graph_t {
	xarray arrNodes;
	xarray arrEdges;
	xmap mapNodes;
	xmap mapEdges;
	xarray arrSelectedNodes;
	xarray arrSelectedEdges;
	xmap mapSelectedNodes;
	xmap mapSelectedEdges;
	xarray arrDiagnostics;
	xarray arrUndo;
	xarray arrRedo;
	xui_flow_viewport_t tViewport;
	uint32_t iNextNodeId;
	uint32_t iNextEdgeId;
	uint32_t iRevision;
	uint32_t iGeometryRevision;
	uint32_t iCleanRevision;
	int iCommandHistoryLimit;
};

static int __xuiFlowPortDescValid(const xui_flow_port_desc_t* pDesc);
static int __xuiFlowPortFind(const xui_flow_node_model_t* pNode, const char* sId);
static void __xuiFlowTrimHistory(xui_flow_graph pGraph);
static void __xuiFlowClearRedo(xui_flow_graph pGraph);
static int __xuiFlowApplyDeleteSelectionCommand(xui_flow_graph pGraph,
	const xui_flow_command_model_t* pCommand, int bDelete);

static int __xuiFlowFinite(float fValue)
{
	return isfinite((double)fValue) ? 1 : 0;
}

static void __xuiFlowBumpRevision(xui_flow_graph pGraph)
{
	if ( pGraph != NULL ) {
		pGraph->iRevision++;
		if ( pGraph->iRevision == 0u ) {
			pGraph->iRevision = 1u;
		}
	}
}

static void __xuiFlowBumpGeometryRevision(xui_flow_graph pGraph)
{
	if ( pGraph == NULL ) {
		return;
	}
	pGraph->iGeometryRevision++;
	if ( pGraph->iGeometryRevision == 0u ) {
		pGraph->iGeometryRevision = 1u;
	}
	__xuiFlowBumpRevision(pGraph);
}

uint32_t xuiInternalFlowGraphGetGeometryRevision(xui_flow_graph pGraph)
{
	return (pGraph != NULL) ? pGraph->iGeometryRevision : 0u;
}

void xuiInternalFlowGraphSwapContents(xui_flow_graph pFirst, xui_flow_graph pSecond)
{
	struct xui_flow_graph_t tGraph;

	if ( pFirst == NULL || pSecond == NULL || pFirst == pSecond ) {
		return;
	}
	tGraph = *pFirst;
	*pFirst = *pSecond;
	*pSecond = tGraph;
	pFirst->iRevision = tGraph.iRevision;
	pFirst->iGeometryRevision = tGraph.iGeometryRevision;
	__xuiFlowBumpGeometryRevision(pFirst);
}

static char* __xuiFlowCopyString(const char* sText)
{
	char* sCopy;
	size_t iSize;

	if ( sText == NULL ) {
		return NULL;
	}
	iSize = strlen(sText);
	sCopy = (char*)xrtMalloc(iSize + 1u);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iSize + 1u);
	return sCopy;
}

static int __xuiFlowStringEqual(const char* sA, const char* sB)
{
	if ( sA == sB ) {
		return 1;
	}
	if ( sA == NULL || sB == NULL ) {
		return 0;
	}
	return strcmp(sA, sB) == 0;
}

static void __xuiFlowFreePort(xui_flow_port_model_t* pPort)
{
	if ( pPort == NULL ) {
		return;
	}
	xrtFree(pPort->sId);
	xrtFree(pPort->sTitle);
	xrtFree(pPort->sDataType);
	memset(pPort, 0, sizeof(*pPort));
}

static void __xuiFlowFreePortArray(xarray* pPorts)
{
	uint32_t i;

	if ( pPorts == NULL ) {
		return;
	}
	for ( i = 1u; i <= pPorts->Count; ++i ) {
		__xuiFlowFreePort((xui_flow_port_model_t*)xuiXrtArrayGet(pPorts, i));
	}
	xrtArrayUnit(pPorts);
}

static void __xuiFlowFreeNode(xui_flow_node_model_t* pNode)
{
	if ( pNode == NULL ) {
		return;
	}
	__xuiFlowFreePortArray(&pNode->arrPorts);
	xrtMapUnit(&pNode->mapPorts);
	if ( pNode->pConfig != NULL ) {
		xrtValueRelease(pNode->pConfig);
	}
	xrtFree(pNode->sId);
	xrtFree(pNode->sType);
	xrtFree(pNode->sTitle);
	xrtFree(pNode->sSummary);
	xrtFree(pNode->sRunPreview);
	memset(pNode, 0, sizeof(*pNode));
}

static void __xuiFlowFreeEdge(xui_flow_edge_model_t* pEdge)
{
	if ( pEdge == NULL ) {
		return;
	}
	xrtFree(pEdge->sId);
	xrtFree(pEdge->sFromNode);
	xrtFree(pEdge->sFromPort);
	xrtFree(pEdge->sToNode);
	xrtFree(pEdge->sToPort);
	xrtFree(pEdge->sRunPreview);
	memset(pEdge, 0, sizeof(*pEdge));
}

static void __xuiFlowFreeDiagnostic(xui_flow_diagnostic_model_t* pDiagnostic)
{
	if ( pDiagnostic == NULL ) {
		return;
	}
	xrtFree(pDiagnostic->sCode);
	xrtFree(pDiagnostic->sMessage);
	xrtFree(pDiagnostic->sNode);
	xrtFree(pDiagnostic->sEdge);
	xrtFree(pDiagnostic->sPath);
	memset(pDiagnostic, 0, sizeof(*pDiagnostic));
}

static void __xuiFlowFreePortDesc(xui_flow_port_desc_t* pPort)
{
	if ( pPort == NULL ) {
		return;
	}
	xrtFree((void*)pPort->sId);
	xrtFree((void*)pPort->sTitle);
	xrtFree((void*)pPort->sDataType);
	memset(pPort, 0, sizeof(*pPort));
}

static void __xuiFlowFreeEdgeDesc(xui_flow_edge_desc_t* pEdge)
{
	if ( pEdge == NULL ) {
		return;
	}
	xrtFree((void*)pEdge->sId);
	xrtFree((void*)pEdge->sFromNode);
	xrtFree((void*)pEdge->sFromPort);
	xrtFree((void*)pEdge->sToNode);
	xrtFree((void*)pEdge->sToPort);
	memset(pEdge, 0, sizeof(*pEdge));
}

static void __xuiFlowFreeCommand(xui_flow_command_model_t* pCommand)
{
	uint32_t i;
	xui_flow_edge_snapshot_t* pEdge;
	xui_flow_move_node_record_t* pMove;

	if ( pCommand == NULL ) {
		return;
	}
	xrtFree((void*)pCommand->tNode.sId);
	xrtFree((void*)pCommand->tNode.sType);
	xrtFree((void*)pCommand->tNode.sTitle);
	xrtFree((void*)pCommand->tNode.sSummary);
	__xuiFlowFreeEdgeDesc(&pCommand->tEdge);
	for ( i = 1u; i <= pCommand->arrPorts.Count; ++i ) {
		__xuiFlowFreePortDesc((xui_flow_port_desc_t*)xuiXrtArrayGet(&pCommand->arrPorts, i));
	}
	xrtArrayUnit(&pCommand->arrPorts);
	for ( i = 1u; i <= pCommand->arrEdges.Count; ++i ) {
		pEdge = (xui_flow_edge_snapshot_t*)xuiXrtArrayGet(&pCommand->arrEdges, i);
		if ( pEdge == NULL ) continue;
		__xuiFlowFreeEdgeDesc(&pEdge->tEdge);
		xrtFree(pEdge->sRunPreview);
		memset(pEdge, 0, sizeof(*pEdge));
	}
	xrtArrayUnit(&pCommand->arrEdges);
	for ( i = 1u; i <= pCommand->arrMoves.Count; ++i ) {
		pMove = (xui_flow_move_node_record_t*)xuiXrtArrayGet(&pCommand->arrMoves, i);
		if ( pMove == NULL ) {
			continue;
		}
		xrtFree((void*)pMove->sId);
		memset(pMove, 0, sizeof(*pMove));
	}
	xrtArrayUnit(&pCommand->arrMoves);
	for ( i = 1u; i <= pCommand->arrCommands.Count; ++i ) {
		__xuiFlowFreeCommand((xui_flow_command_model_t*)xuiXrtArrayGet(&pCommand->arrCommands, i));
	}
	xrtArrayUnit(&pCommand->arrCommands);
	if ( pCommand->pOldConfig != NULL ) {
		xrtValueRelease(pCommand->pOldConfig);
	}
	if ( pCommand->pNewConfig != NULL ) {
		xrtValueRelease(pCommand->pNewConfig);
	}
	xrtFree(pCommand->sNodeId);
	xrtFree(pCommand->sNodeRunPreview);
	xrtFree(pCommand->sEdgeRunPreview);
	memset(pCommand, 0, sizeof(*pCommand));
}

static int __xuiFlowCopyNodeDesc(xui_flow_node_desc_t* pDst, const xui_flow_node_desc_t* pSrc)
{
	if ( (pDst == NULL) || (pSrc == NULL) || (pSrc->iSize < sizeof(*pSrc)) ||
	     (pSrc->sId == NULL) || (pSrc->sId[0] == 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pDst, 0, sizeof(*pDst));
	pDst->iSize = sizeof(*pDst);
	pDst->sId = __xuiFlowCopyString(pSrc->sId);
	pDst->sType = __xuiFlowCopyString(pSrc->sType);
	pDst->sTitle = __xuiFlowCopyString(pSrc->sTitle);
	pDst->sSummary = __xuiFlowCopyString(pSrc->sSummary);
	pDst->fX = pSrc->fX;
	pDst->fY = pSrc->fY;
	pDst->fW = pSrc->fW;
	pDst->fH = pSrc->fH;
	if ( (pDst->sId == NULL) || ((pSrc->sType != NULL) && (pDst->sType == NULL)) ||
	     ((pSrc->sTitle != NULL) && (pDst->sTitle == NULL)) ||
	     ((pSrc->sSummary != NULL) && (pDst->sSummary == NULL)) ) {
		xrtFree((void*)pDst->sId);
		xrtFree((void*)pDst->sType);
		xrtFree((void*)pDst->sTitle);
		xrtFree((void*)pDst->sSummary);
		memset(pDst, 0, sizeof(*pDst));
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	return XUI_OK;
}

static int __xuiFlowCopyPortDesc(xui_flow_port_desc_t* pDst, const xui_flow_port_desc_t* pSrc)
{
	if ( (pDst == NULL) || !__xuiFlowPortDescValid(pSrc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pDst, 0, sizeof(*pDst));
	pDst->iSize = sizeof(*pDst);
	pDst->sId = __xuiFlowCopyString(pSrc->sId);
	pDst->sTitle = __xuiFlowCopyString(pSrc->sTitle);
	pDst->sDataType = __xuiFlowCopyString(pSrc->sDataType);
	pDst->iDirection = pSrc->iDirection;
	pDst->iKind = pSrc->iKind;
	pDst->bRequired = pSrc->bRequired;
	pDst->bMulti = pSrc->bMulti;
	pDst->bDynamic = pSrc->bDynamic;
	if ( (pDst->sId == NULL) || ((pSrc->sTitle != NULL) && (pDst->sTitle == NULL)) ||
	     ((pSrc->sDataType != NULL) && (pDst->sDataType == NULL)) ) {
		__xuiFlowFreePortDesc(pDst);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	return XUI_OK;
}

static int __xuiFlowCopyEdgeDesc(xui_flow_edge_desc_t* pDst, const xui_flow_edge_desc_t* pSrc)
{
	if ( (pDst == NULL) || (pSrc == NULL) || (pSrc->iSize < sizeof(*pSrc)) ||
	     (pSrc->sId == NULL) || (pSrc->sId[0] == 0) || (pSrc->sFromNode == NULL) ||
	     (pSrc->sFromPort == NULL) || (pSrc->sToNode == NULL) || (pSrc->sToPort == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pDst, 0, sizeof(*pDst));
	pDst->iSize = sizeof(*pDst);
	pDst->sId = __xuiFlowCopyString(pSrc->sId);
	pDst->sFromNode = __xuiFlowCopyString(pSrc->sFromNode);
	pDst->sFromPort = __xuiFlowCopyString(pSrc->sFromPort);
	pDst->sToNode = __xuiFlowCopyString(pSrc->sToNode);
	pDst->sToPort = __xuiFlowCopyString(pSrc->sToPort);
	pDst->iKind = pSrc->iKind;
	pDst->iRouteStyle = pSrc->iRouteStyle;
	pDst->fRouteBias = pSrc->fRouteBias;
	pDst->fRouteSourceOffset = pSrc->fRouteSourceOffset;
	pDst->fRouteTargetOffset = pSrc->fRouteTargetOffset;
	if ( (pDst->sId == NULL) || (pDst->sFromNode == NULL) || (pDst->sFromPort == NULL) ||
	     (pDst->sToNode == NULL) || (pDst->sToPort == NULL) ) {
		__xuiFlowFreeEdgeDesc(pDst);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	return XUI_OK;
}

static int __xuiFlowRouteStyleValid(int iRouteStyle)
{
	return iRouteStyle >= XUI_FLOW_ROUTE_AUTO && iRouteStyle <= XUI_FLOW_ROUTE_BEZIER;
}

static int __xuiFlowCopyValue(xvalue* pSrc, xvalue** ppDst)
{
	if ( ppDst == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppDst = NULL;
	if ( pSrc == NULL ) {
		return XUI_OK;
	}
	*ppDst = xrtValueDeepClone(pSrc);
	return (*ppDst != NULL) ? XUI_OK : XUI_ERROR_OUT_OF_MEMORY;
}

static void __xuiFlowFreeDiagnostics(xui_flow_graph pGraph)
{
	uint32_t i;

	if ( pGraph == NULL ) {
		return;
	}
	for ( i = 1u; i <= pGraph->arrDiagnostics.Count; ++i ) {
		__xuiFlowFreeDiagnostic((xui_flow_diagnostic_model_t*)xuiXrtArrayGet(&pGraph->arrDiagnostics, i));
	}
	xrtArrayUnit(&pGraph->arrDiagnostics);
}

static void __xuiFlowFreeCommandArray(xarray* pArray)
{
	uint32_t i;

	if ( pArray == NULL ) {
		return;
	}
	for ( i = 1u; i <= pArray->Count; ++i ) {
		__xuiFlowFreeCommand((xui_flow_command_model_t*)xuiXrtArrayGet(pArray, i));
	}
	xrtArrayUnit(pArray);
}

static int __xuiFlowPushCommand(xarray* pArray, const xui_flow_command_model_t* pCommand)
{
	xui_flow_command_model_t* pDst;
	uint32_t iPos;

	if ( (pArray == NULL) || (pCommand == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iPos = xuiXrtArrayAppendSpace(pArray, 1u);
	if ( iPos == 0u ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDst = (xui_flow_command_model_t*)xuiXrtArrayGet(pArray, iPos);
	*pDst = *pCommand;
	return XUI_OK;
}

static int __xuiFlowReserveUndoCommand(xui_flow_graph pGraph, uint32_t* pPosition)
{
	xui_flow_command_model_t* pSlot;
	uint32_t iPos;

	if ( pGraph == NULL || pPosition == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*pPosition = 0u;
	iPos = xuiXrtArrayAppendSpace(&pGraph->arrUndo, 1u);
	if ( iPos == 0u ) return XUI_ERROR_OUT_OF_MEMORY;
	pSlot = (xui_flow_command_model_t*)xuiXrtArrayGet(&pGraph->arrUndo, iPos);
	if ( pSlot == NULL ) {
		xuiXrtArrayRemove(&pGraph->arrUndo, iPos, 1u);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	memset(pSlot, 0, sizeof(*pSlot));
	*pPosition = iPos;
	return XUI_OK;
}

static void __xuiFlowCancelReservedUndo(xui_flow_graph pGraph, uint32_t iPosition)
{
	if ( pGraph != NULL && iPosition > 0u && iPosition <= pGraph->arrUndo.Count ) {
		xuiXrtArrayRemove(&pGraph->arrUndo, iPosition, 1u);
	}
}

static void __xuiFlowCommitReservedUndo(xui_flow_graph pGraph, uint32_t iPosition,
	xui_flow_command_model_t* pCommand)
{
	xui_flow_command_model_t* pSlot;

	if ( pGraph == NULL || pCommand == NULL || iPosition == 0u ||
	     iPosition > pGraph->arrUndo.Count ) return;
	pSlot = (xui_flow_command_model_t*)xuiXrtArrayGet(&pGraph->arrUndo, iPosition);
	*pSlot = *pCommand;
	memset(pCommand, 0, sizeof(*pCommand));
	__xuiFlowTrimHistory(pGraph);
	__xuiFlowClearRedo(pGraph);
}

static void __xuiFlowTrimCommandArray(xarray* pArray, int iLimit)
{
	xui_flow_command_model_t* pCommand;

	if ( (pArray == NULL) || (iLimit < 0) ) {
		return;
	}
	while ( (int)pArray->Count > iLimit ) {
		pCommand = (xui_flow_command_model_t*)xuiXrtArrayGet(pArray, 1u);
		__xuiFlowFreeCommand(pCommand);
		xuiXrtArrayRemove(pArray, 1u, 1u);
	}
}

static void __xuiFlowTrimHistory(xui_flow_graph pGraph)
{
	if ( pGraph == NULL ) {
		return;
	}
	__xuiFlowTrimCommandArray(&pGraph->arrUndo, pGraph->iCommandHistoryLimit);
	__xuiFlowTrimCommandArray(&pGraph->arrRedo, pGraph->iCommandHistoryLimit);
}

static int __xuiFlowPrepareCommandTransfer(xarray* pSource, xarray* pDestination,
	xui_flow_command_model_t** ppCommand, uint32_t* pDestinationPos)
{
	xui_flow_command_model_t* pDestinationCommand;
	uint32_t iPos;

	if ( (pSource == NULL) || (pDestination == NULL) || (ppCommand == NULL) ||
	     (pDestinationPos == NULL) || (pSource->Count == 0u) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iPos = xuiXrtArrayAppendSpace(pDestination, 1u);
	if ( iPos == 0u ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDestinationCommand = (xui_flow_command_model_t*)xuiXrtArrayGet(pDestination, iPos);
	memset(pDestinationCommand, 0, sizeof(*pDestinationCommand));
	*ppCommand = (xui_flow_command_model_t*)xuiXrtArrayGet(pSource, pSource->Count);
	*pDestinationPos = iPos;
	return XUI_OK;
}

static void __xuiFlowCancelCommandTransfer(xarray* pDestination, uint32_t iDestinationPos)
{
	if ( pDestination != NULL && iDestinationPos > 0u && iDestinationPos <= pDestination->Count ) {
		xuiXrtArrayRemove(pDestination, iDestinationPos, 1u);
	}
}

static void __xuiFlowCommitCommandTransfer(xarray* pSource, xarray* pDestination, uint32_t iDestinationPos)
{
	xui_flow_command_model_t* pSourceCommand;
	xui_flow_command_model_t* pDestinationCommand;

	if ( (pSource == NULL) || (pDestination == NULL) || (pSource->Count == 0u) ||
	     (iDestinationPos == 0u) || (iDestinationPos > pDestination->Count) ) {
		return;
	}
	pSourceCommand = (xui_flow_command_model_t*)xuiXrtArrayGet(pSource, pSource->Count);
	pDestinationCommand = (xui_flow_command_model_t*)xuiXrtArrayGet(pDestination, iDestinationPos);
	*pDestinationCommand = *pSourceCommand;
	memset(pSourceCommand, 0, sizeof(*pSourceCommand));
	xuiXrtArrayRemove(pSource, pSource->Count, 1u);
}

static void __xuiFlowClearRedo(xui_flow_graph pGraph)
{
	if ( pGraph == NULL ) {
		return;
	}
	__xuiFlowFreeCommandArray(&pGraph->arrRedo);
	xuiXrtArrayInit(&pGraph->arrRedo, sizeof(xui_flow_command_model_t));
}

static void __xuiFlowInitCommand(xui_flow_command_model_t* pCommand)
{
	if ( pCommand == NULL ) {
		return;
	}
	memset(pCommand, 0, sizeof(*pCommand));
	xuiXrtArrayInit(&pCommand->arrPorts, sizeof(xui_flow_port_desc_t));
	xuiXrtArrayInit(&pCommand->arrEdges, sizeof(xui_flow_edge_snapshot_t));
	xuiXrtArrayInit(&pCommand->arrMoves, sizeof(xui_flow_move_node_record_t));
	xuiXrtArrayInit(&pCommand->arrCommands, sizeof(xui_flow_command_model_t));
}

static int __xuiFlowDictFindIndex(xmap* pDict, const char* sId)
{
	int* pValue;

	if ( (pDict == NULL) || (sId == NULL) || (sId[0] == 0) ) {
		return -1;
	}
	pValue = (int*)xuiXrtMapGet(pDict, (ptr)(void*)sId, (uint32)strlen(sId));
	return (pValue != NULL) ? *pValue : -1;
}

static int __xuiFlowDictSetIndex(xmap* pDict, const char* sId, int iIndex)
{
	int* pValue;
	bool bNew;

	if ( (pDict == NULL) || (sId == NULL) || (sId[0] == 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pValue = (int*)xuiXrtMapGetOrAdd(pDict, (ptr)(void*)sId, (uint32)strlen(sId), &bNew);
	if ( pValue == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	*pValue = iIndex;
	return bNew ? XUI_OK : XUI_ERROR_ALREADY_INITIALIZED;
}

static void __xuiFlowFreeSelectionArray(xarray* pArray)
{
	uint32_t i;
	char** ppId;

	if ( pArray == NULL ) {
		return;
	}
	for ( i = 1u; i <= pArray->Count; ++i ) {
		ppId = (char**)xuiXrtArrayGet(pArray, i);
		xrtFree(*ppId);
		*ppId = NULL;
	}
	xrtArrayUnit(pArray);
}

static int __xuiFlowSelectionFind(xmap* pMap, const char* sId)
{
	int* pIndex;

	if ( (pMap == NULL) || (sId == NULL) || (sId[0] == 0) ) {
		return -1;
	}
	pIndex = (int*)xuiXrtMapGet(pMap, (ptr)(void*)sId, (uint32)strlen(sId));
	return (pIndex != NULL) ? *pIndex : -1;
}

static int __xuiFlowSelectionSet(xarray* pArray, xmap* pMap, const char* sId,
	int bSelected, int* pChanged)
{
	char** ppLastId;
	char** ppId;
	int* pMapIndex;
	bool bNew;
	uint32_t iPos;
	int iIndex;

	if ( (pArray == NULL) || (pMap == NULL) || (sId == NULL) || (sId[0] == 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pChanged != NULL ) {
		*pChanged = 0;
	}
	iIndex = __xuiFlowSelectionFind(pMap, sId);
	if ( bSelected ) {
		if ( iIndex >= 0 ) {
			return XUI_OK;
		}
		iPos = xuiXrtArrayAppendSpace(pArray, 1u);
		if ( iPos == 0u ) {
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		ppId = (char**)xuiXrtArrayGet(pArray, iPos);
		*ppId = __xuiFlowCopyString(sId);
		if ( *ppId == NULL ) {
			xuiXrtArrayRemove(pArray, iPos, 1u);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		pMapIndex = (int*)xuiXrtMapGetOrAdd(pMap, (ptr)(void*)*ppId,
			(uint32)strlen(*ppId), &bNew);
		if ( pMapIndex == NULL || !bNew ) {
			xrtFree(*ppId);
			*ppId = NULL;
			xuiXrtArrayRemove(pArray, iPos, 1u);
			return (pMapIndex == NULL) ? XUI_ERROR_OUT_OF_MEMORY : XUI_ERROR_INVALID_STATE;
		}
		*pMapIndex = (int)iPos - 1;
		if ( pChanged != NULL ) {
			*pChanged = 1;
		}
		return XUI_OK;
	}
	if ( iIndex < 0 ) {
		return XUI_OK;
	}
	ppId = (char**)xuiXrtArrayGet(pArray, (uint32_t)iIndex + 1u);
	(void)xuiXrtMapRemove(pMap, *ppId, strlen(*ppId));
	xrtFree(*ppId);
	if ( (uint32_t)iIndex + 1u < pArray->Count ) {
		ppLastId = (char**)xuiXrtArrayGet(pArray, pArray->Count);
		*ppId = *ppLastId;
		*ppLastId = NULL;
		pMapIndex = (int*)xuiXrtMapGet(pMap, (ptr)(void*)*ppId, (uint32)strlen(*ppId));
		if ( pMapIndex != NULL ) *pMapIndex = iIndex;
	} else {
		*ppId = NULL;
	}
	xuiXrtArrayRemove(pArray, pArray->Count, 1u);
	if ( pChanged != NULL ) {
		*pChanged = 1;
	}
	return XUI_OK;
}

static void __xuiFlowRefreshEdgeRef(xui_flow_graph pGraph, xui_flow_edge_model_t* pEdge)
{
	xui_flow_node_model_t* pFromNode;
	xui_flow_node_model_t* pToNode;
	xui_flow_port_model_t* pFromPort;
	xui_flow_port_model_t* pToPort;

	if ( pGraph == NULL || pEdge == NULL ) {
		return;
	}
	pEdge->iFromNode = xuiFlowGraphFindNode(pGraph, pEdge->sFromNode);
	pEdge->iToNode = xuiFlowGraphFindNode(pGraph, pEdge->sToNode);
	pEdge->iFromPort = -1;
	pEdge->iToPort = -1;
	pFromPort = NULL;
	pToPort = NULL;
	if ( pEdge->iFromNode >= 0 ) {
		pFromNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)pEdge->iFromNode + 1u);
		pEdge->iFromPort = __xuiFlowPortFind(pFromNode, pEdge->sFromPort);
		if ( pEdge->iFromPort >= 0 ) {
			pFromPort = (xui_flow_port_model_t*)xuiXrtArrayGet(&pFromNode->arrPorts, (uint32_t)pEdge->iFromPort + 1u);
		}
	}
	if ( pEdge->iToNode >= 0 ) {
		pToNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)pEdge->iToNode + 1u);
		pEdge->iToPort = __xuiFlowPortFind(pToNode, pEdge->sToPort);
		if ( pEdge->iToPort >= 0 ) {
			pToPort = (xui_flow_port_model_t*)xuiXrtArrayGet(&pToNode->arrPorts, (uint32_t)pEdge->iToPort + 1u);
		}
	}
	pEdge->bInvalid = (pFromPort == NULL || pToPort == NULL ||
		pFromPort->iDirection != XUI_FLOW_PORT_OUTPUT || pToPort->iDirection != XUI_FLOW_PORT_INPUT ||
		pFromPort->iKind != pEdge->iKind || pToPort->iKind != pEdge->iKind) ? 1 : 0;
}

static void __xuiFlowRefreshEdgeRefs(xui_flow_graph pGraph)
{
	uint32_t i;
	xui_flow_edge_model_t* pEdge;

	if ( pGraph == NULL ) {
		return;
	}
	for ( i = 1u; i <= pGraph->arrEdges.Count; ++i ) {
		pEdge = (xui_flow_edge_model_t*)xuiXrtArrayGet(&pGraph->arrEdges, i);
		__xuiFlowRefreshEdgeRef(pGraph, pEdge);
	}
}

static void __xuiFlowUpdateMapIndexes(xmap* pMap, xarray* pItems, uint32_t iFirst, size_t iIdOffset)
{
	uint32_t i;
	void* pItem;
	char** ppId;
	int* pIndex;

	if ( (pMap == NULL) || (pItems == NULL) ) {
		return;
	}
	for ( i = iFirst; i <= pItems->Count; ++i ) {
		pItem = xuiXrtArrayGet(pItems, i);
		if ( pItem == NULL ) {
			continue;
		}
		ppId = (char**)((unsigned char*)pItem + iIdOffset);
		if ( *ppId == NULL ) {
			continue;
		}
		pIndex = (int*)xuiXrtMapGet(pMap, (ptr)(void*)*ppId, (uint32)strlen(*ppId));
		if ( pIndex != NULL ) {
			*pIndex = (int)i - 1;
		}
	}
}

static int __xuiFlowRemoveEdgeAt(xui_flow_graph pGraph, int iEdge)
{
	xui_flow_edge_model_t* pEdge;
	uint32_t iPos;

	if ( (pGraph == NULL) || (iEdge < 0) || ((uint32_t)iEdge >= pGraph->arrEdges.Count) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iPos = (uint32_t)iEdge + 1u;
	pEdge = (xui_flow_edge_model_t*)xuiXrtArrayGet(&pGraph->arrEdges, iPos);
	if ( pEdge == NULL ) {
		return XUI_ERROR_INVALID_STATE;
	}
	__xuiFlowSelectionSet(&pGraph->arrSelectedEdges, &pGraph->mapSelectedEdges,
		pEdge->sId, 0, NULL);
	(void)xuiXrtMapRemove(&pGraph->mapEdges, pEdge->sId, strlen(pEdge->sId));
	__xuiFlowFreeEdge(pEdge);
	xuiXrtArrayRemove(&pGraph->arrEdges, iPos, 1u);
	__xuiFlowUpdateMapIndexes(&pGraph->mapEdges, &pGraph->arrEdges, iPos,
		offsetof(xui_flow_edge_model_t, sId));
	__xuiFlowRefreshEdgeRefs(pGraph);
	return XUI_OK;
}

static int __xuiFlowCommandAppendPort(xui_flow_command_model_t* pCommand, const xui_flow_port_model_t* pPort)
{
	xui_flow_port_desc_t tPort;
	xui_flow_port_desc_t* pDst;
	uint32_t iPos;
	int iRet;

	if ( (pCommand == NULL) || (pPort == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tPort, 0, sizeof(tPort));
	tPort.iSize = sizeof(tPort);
	tPort.sId = pPort->sId;
	tPort.sTitle = pPort->sTitle;
	tPort.sDataType = pPort->sDataType;
	tPort.iDirection = pPort->iDirection;
	tPort.iKind = pPort->iKind;
	tPort.bRequired = pPort->bRequired;
	tPort.bMulti = pPort->bMulti;
	tPort.bDynamic = pPort->bDynamic;
	iPos = xuiXrtArrayAppendSpace(&pCommand->arrPorts, 1u);
	if ( iPos == 0u ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDst = (xui_flow_port_desc_t*)xuiXrtArrayGet(&pCommand->arrPorts, iPos);
	iRet = __xuiFlowCopyPortDesc(pDst, &tPort);
	if ( iRet != XUI_OK ) {
		xuiXrtArrayRemove(&pCommand->arrPorts, iPos, 1u);
	}
	return iRet;
}

static int __xuiFlowCommandAppendEdge(xui_flow_command_model_t* pCommand, const xui_flow_edge_model_t* pEdge)
{
	xui_flow_edge_desc_t tEdge;
	xui_flow_edge_snapshot_t* pDst;
	uint32_t iPos;
	int iRet;

	if ( (pCommand == NULL) || (pEdge == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tEdge, 0, sizeof(tEdge));
	tEdge.iSize = sizeof(tEdge);
	tEdge.sId = pEdge->sId;
	tEdge.iKind = pEdge->iKind;
	tEdge.sFromNode = pEdge->sFromNode;
	tEdge.sFromPort = pEdge->sFromPort;
	tEdge.sToNode = pEdge->sToNode;
	tEdge.sToPort = pEdge->sToPort;
	tEdge.iRouteStyle = pEdge->iRouteStyle;
	tEdge.fRouteBias = pEdge->fRouteBias;
	tEdge.fRouteSourceOffset = pEdge->fRouteSourceOffset;
	tEdge.fRouteTargetOffset = pEdge->fRouteTargetOffset;
	iPos = xuiXrtArrayAppendSpace(&pCommand->arrEdges, 1u);
	if ( iPos == 0u ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDst = (xui_flow_edge_snapshot_t*)xuiXrtArrayGet(&pCommand->arrEdges, iPos);
	if ( pDst == NULL ) {
		xuiXrtArrayRemove(&pCommand->arrEdges, iPos, 1u);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	memset(pDst, 0, sizeof(*pDst));
	iRet = __xuiFlowCopyEdgeDesc(&pDst->tEdge, &tEdge);
	if ( iRet == XUI_OK ) {
		pDst->iRunState = pEdge->iRunState;
		pDst->sRunPreview = __xuiFlowCopyString(pEdge->sRunPreview);
		if ( pEdge->sRunPreview != NULL && pDst->sRunPreview == NULL ) {
			iRet = XUI_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeEdgeDesc(&pDst->tEdge);
		xrtFree(pDst->sRunPreview);
		xuiXrtArrayRemove(&pCommand->arrEdges, iPos, 1u);
	}
	return iRet;
}

static int __xuiFlowCommandAppendMove(xui_flow_command_model_t* pCommand, const xui_flow_move_node_record_t* pRecord)
{
	xui_flow_move_node_record_t* pDst;
	uint32_t iPos;

	if ( (pCommand == NULL) || (pRecord == NULL) || (pRecord->iSize < sizeof(*pRecord)) ||
	     (pRecord->sId == NULL) || (pRecord->sId[0] == 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pRecord->fOldX == pRecord->fNewX) && (pRecord->fOldY == pRecord->fNewY) ) {
		return XUI_OK;
	}
	iPos = xuiXrtArrayAppendSpace(&pCommand->arrMoves, 1u);
	if ( iPos == 0u ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDst = (xui_flow_move_node_record_t*)xuiXrtArrayGet(&pCommand->arrMoves, iPos);
	if ( pDst == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pDst, 0, sizeof(*pDst));
	pDst->iSize = sizeof(*pDst);
	pDst->sId = __xuiFlowCopyString(pRecord->sId);
	if ( pDst->sId == NULL ) {
		xuiXrtArrayRemove(&pCommand->arrMoves, iPos, 1u);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDst->fOldX = pRecord->fOldX;
	pDst->fOldY = pRecord->fOldY;
	pDst->fNewX = pRecord->fNewX;
	pDst->fNewY = pRecord->fNewY;
	return XUI_OK;
}

static int __xuiFlowCommandSnapshotRemoveEdge(xui_flow_graph pGraph, const char* sId, xui_flow_command_model_t* pCommand)
{
	xui_flow_edge_model_t* pEdge;
	int iEdge;
	int iRet;

	if ( (pGraph == NULL) || (sId == NULL) || (pCommand == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iEdge = xuiFlowGraphFindEdge(pGraph, sId);
	if ( iEdge < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pEdge = (xui_flow_edge_model_t*)xuiXrtArrayGet(&pGraph->arrEdges, (uint32_t)iEdge + 1u);
	if ( pEdge == NULL ) {
		return XUI_ERROR_INVALID_STATE;
	}
	pCommand->iType = XUI_FLOW_COMMAND_REMOVE_EDGE;
	iRet = __xuiFlowCopyEdgeDesc(&pCommand->tEdge, &(xui_flow_edge_desc_t){
		sizeof(xui_flow_edge_desc_t), pEdge->sId, pEdge->iKind, pEdge->sFromNode, pEdge->sFromPort, pEdge->sToNode, pEdge->sToPort, pEdge->iRouteStyle, pEdge->fRouteBias, pEdge->fRouteSourceOffset, pEdge->fRouteTargetOffset
	});
	if ( iRet != XUI_OK ) return iRet;
	pCommand->iEdgeRunState = pEdge->iRunState;
	pCommand->sEdgeRunPreview = __xuiFlowCopyString(pEdge->sRunPreview);
	return (pEdge->sRunPreview == NULL || pCommand->sEdgeRunPreview != NULL) ? XUI_OK : XUI_ERROR_OUT_OF_MEMORY;
}

static int __xuiFlowCommandSnapshotRemoveNodeEx(xui_flow_graph pGraph, const char* sId,
	xui_flow_command_model_t* pCommand, int bIncludeEdges)
{
	xui_flow_node_model_t* pNode;
	xui_flow_edge_model_t* pEdge;
	uint32_t i;
	int iNode;
	int iRet;

	if ( (pGraph == NULL) || (sId == NULL) || (pCommand == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNode = xuiFlowGraphFindNode(pGraph, sId);
	if ( iNode < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	if ( pNode == NULL ) {
		return XUI_ERROR_INVALID_STATE;
	}
	pCommand->iType = XUI_FLOW_COMMAND_REMOVE_NODE;
	iRet = __xuiFlowCopyNodeDesc(&pCommand->tNode, &(xui_flow_node_desc_t){
		sizeof(xui_flow_node_desc_t), pNode->sId, pNode->sType, pNode->sTitle, pNode->fX, pNode->fY, pNode->fW, pNode->fH, pNode->sSummary
	});
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = __xuiFlowCopyValue(pNode->pConfig, &pCommand->pOldConfig);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pCommand->iNodeRunState = pNode->iRunState;
	pCommand->sNodeRunPreview = __xuiFlowCopyString(pNode->sRunPreview);
	if ( pNode->sRunPreview != NULL && pCommand->sNodeRunPreview == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 1u; i <= pNode->arrPorts.Count; ++i ) {
		iRet = __xuiFlowCommandAppendPort(pCommand, (xui_flow_port_model_t*)xuiXrtArrayGet(&pNode->arrPorts, i));
		if ( iRet != XUI_OK ) {
			return iRet;
		}
	}
	for ( i = 1u; bIncludeEdges && i <= pGraph->arrEdges.Count; ++i ) {
		pEdge = (xui_flow_edge_model_t*)xuiXrtArrayGet(&pGraph->arrEdges, i);
		if ( strcmp(pEdge->sFromNode, sId) == 0 || strcmp(pEdge->sToNode, sId) == 0 ) {
			iRet = __xuiFlowCommandAppendEdge(pCommand, pEdge);
			if ( iRet != XUI_OK ) {
				return iRet;
			}
		}
	}
	return XUI_OK;
}

static int __xuiFlowCommandSnapshotRemoveNode(xui_flow_graph pGraph, const char* sId,
	xui_flow_command_model_t* pCommand)
{
	return __xuiFlowCommandSnapshotRemoveNodeEx(pGraph, sId, pCommand, 1);
}

static int __xuiFlowCommandRestoreRemovedEdge(xui_flow_graph pGraph,
	const xui_flow_edge_desc_t* pEdge, int iRunState, const char* sRunPreview)
{
	int iRet;

	iRet = xuiFlowGraphAddEdgePreserveInvalid(pGraph, pEdge, NULL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiFlowGraphSetEdgeRunState(pGraph, pEdge->sId, iRunState, sRunPreview);
	if ( iRet != XUI_OK ) (void)xuiFlowGraphRemoveEdge(pGraph, pEdge->sId);
	return iRet;
}

static int __xuiFlowCommandRestoreRemovedNode(xui_flow_graph pGraph, const xui_flow_command_model_t* pCommand)
{
	xui_flow_edge_snapshot_t* pEdge;
	uint32_t i;
	int iNode;
	int iRet;

	if ( (pGraph == NULL) || (pCommand == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiFlowGraphAddNode(pGraph, &pCommand->tNode, &iNode);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	for ( i = 1u; i <= pCommand->arrPorts.Count; ++i ) {
		iRet = xuiFlowGraphAddPort(pGraph, iNode, (xui_flow_port_desc_t*)xuiXrtArrayGet((xarray*)&pCommand->arrPorts, i), NULL);
		if ( iRet != XUI_OK ) {
			(void)xuiFlowGraphRemoveNode(pGraph, pCommand->tNode.sId);
			return iRet;
		}
	}
	if ( pCommand->pOldConfig != NULL ) {
		iRet = xuiFlowGraphSetNodeConfig(pGraph, pCommand->tNode.sId, pCommand->pOldConfig);
		if ( iRet != XUI_OK ) {
			(void)xuiFlowGraphRemoveNode(pGraph, pCommand->tNode.sId);
			return iRet;
		}
	}
	iRet = xuiFlowGraphSetNodeRunState(pGraph, pCommand->tNode.sId,
		pCommand->iNodeRunState, pCommand->sNodeRunPreview);
	if ( iRet != XUI_OK ) {
		(void)xuiFlowGraphRemoveNode(pGraph, pCommand->tNode.sId);
		return iRet;
	}
	for ( i = 1u; i <= pCommand->arrEdges.Count; ++i ) {
		pEdge = (xui_flow_edge_snapshot_t*)xuiXrtArrayGet((xarray*)&pCommand->arrEdges, i);
		iRet = (pEdge != NULL) ? __xuiFlowCommandRestoreRemovedEdge(pGraph,
			&pEdge->tEdge, pEdge->iRunState, pEdge->sRunPreview) : XUI_ERROR_INVALID_STATE;
		if ( iRet != XUI_OK ) {
			(void)xuiFlowGraphRemoveNode(pGraph, pCommand->tNode.sId);
			return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiFlowPortFind(const xui_flow_node_model_t* pNode, const char* sId)
{
	if ( pNode == NULL ) {
		return -1;
	}
	return __xuiFlowDictFindIndex((xmap*)(void*)&pNode->mapPorts, sId);
}

static int __xuiFlowPortDescValid(const xui_flow_port_desc_t* pDesc)
{
	if ( (pDesc == NULL) || (pDesc->iSize < sizeof(*pDesc)) || (pDesc->sId == NULL) || (pDesc->sId[0] == 0) ) {
		return 0;
	}
	if ( (pDesc->iDirection != XUI_FLOW_PORT_INPUT) && (pDesc->iDirection != XUI_FLOW_PORT_OUTPUT) ) {
		return 0;
	}
	if ( (pDesc->iKind != XUI_FLOW_PORT_CONTROL) && (pDesc->iKind != XUI_FLOW_PORT_DATA) ) {
		return 0;
	}
	return 1;
}

XUI_API int xuiFlowGraphCreate(xui_flow_graph* ppGraph)
{
	xui_flow_graph pGraph;

	if ( ppGraph == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppGraph = NULL;
	pGraph = (xui_flow_graph)xrtCalloc(1u, sizeof(*pGraph));
	if ( pGraph == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	xuiXrtArrayInit(&pGraph->arrNodes, sizeof(xui_flow_node_model_t));
	xuiXrtArrayInit(&pGraph->arrEdges, sizeof(xui_flow_edge_model_t));
	xuiXrtMapInit(&pGraph->mapNodes, sizeof(int));
	xuiXrtMapInit(&pGraph->mapEdges, sizeof(int));
	xuiXrtArrayInit(&pGraph->arrSelectedNodes, sizeof(char*));
	xuiXrtArrayInit(&pGraph->arrSelectedEdges, sizeof(char*));
	xuiXrtMapInit(&pGraph->mapSelectedNodes, sizeof(int));
	xuiXrtMapInit(&pGraph->mapSelectedEdges, sizeof(int));
	xuiXrtArrayInit(&pGraph->arrDiagnostics, sizeof(xui_flow_diagnostic_model_t));
	xuiXrtArrayInit(&pGraph->arrUndo, sizeof(xui_flow_command_model_t));
	xuiXrtArrayInit(&pGraph->arrRedo, sizeof(xui_flow_command_model_t));
	pGraph->tViewport.iSize = sizeof(pGraph->tViewport);
	pGraph->tViewport.fZoom = 1.0f;
	pGraph->iCommandHistoryLimit = XUI_FLOW_COMMAND_HISTORY_DEFAULT_LIMIT;
	*ppGraph = pGraph;
	return XUI_OK;
}

XUI_API void xuiFlowGraphDestroy(xui_flow_graph pGraph)
{
	uint32_t i;

	if ( pGraph == NULL ) {
		return;
	}
	for ( i = 1u; i <= pGraph->arrNodes.Count; ++i ) {
		__xuiFlowFreeNode((xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, i));
	}
	for ( i = 1u; i <= pGraph->arrEdges.Count; ++i ) {
		__xuiFlowFreeEdge((xui_flow_edge_model_t*)xuiXrtArrayGet(&pGraph->arrEdges, i));
	}
	xrtArrayUnit(&pGraph->arrNodes);
	xrtArrayUnit(&pGraph->arrEdges);
	xrtMapUnit(&pGraph->mapNodes);
	xrtMapUnit(&pGraph->mapEdges);
	__xuiFlowFreeSelectionArray(&pGraph->arrSelectedNodes);
	__xuiFlowFreeSelectionArray(&pGraph->arrSelectedEdges);
	xrtMapUnit(&pGraph->mapSelectedNodes);
	xrtMapUnit(&pGraph->mapSelectedEdges);
	__xuiFlowFreeDiagnostics(pGraph);
	__xuiFlowFreeCommandArray(&pGraph->arrUndo);
	__xuiFlowFreeCommandArray(&pGraph->arrRedo);
	xrtFree(pGraph);
}

XUI_API int xuiFlowGraphAddNode(xui_flow_graph pGraph, const xui_flow_node_desc_t* pDesc, int* pIndex)
{
	xui_flow_node_model_t* pNode;
	uint32_t iPos;
	int iIndex;
	int iRet;

	if ( pIndex != NULL ) {
		*pIndex = -1;
	}
	if ( (pGraph == NULL) || (pDesc == NULL) || (pDesc->iSize < sizeof(*pDesc)) || (pDesc->sId == NULL) || (pDesc->sId[0] == 0) ||
	     !__xuiFlowFinite(pDesc->fX) || !__xuiFlowFinite(pDesc->fY) ||
	     !__xuiFlowFinite(pDesc->fW) || !__xuiFlowFinite(pDesc->fH) ||
	     pDesc->fW < 0.0f || pDesc->fH < 0.0f ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( xuiFlowGraphFindNode(pGraph, pDesc->sId) >= 0 ) {
		return XUI_ERROR_ALREADY_INITIALIZED;
	}
	iPos = xuiXrtArrayAppendSpace(&pGraph->arrNodes, 1u);
	if ( iPos == 0u ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iIndex = (int)iPos - 1;
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, iPos);
	memset(pNode, 0, sizeof(*pNode));
	xuiXrtArrayInit(&pNode->arrPorts, sizeof(xui_flow_port_model_t));
	xuiXrtMapInit(&pNode->mapPorts, sizeof(int));
	pNode->sId = __xuiFlowCopyString(pDesc->sId);
	pNode->sType = __xuiFlowCopyString(pDesc->sType);
	pNode->sTitle = __xuiFlowCopyString(pDesc->sTitle);
	pNode->sSummary = __xuiFlowCopyString(pDesc->sSummary);
	pNode->fX = pDesc->fX;
	pNode->fY = pDesc->fY;
	pNode->fW = pDesc->fW;
	pNode->fH = pDesc->fH;
	if ( pNode->sId == NULL ||
	     (pDesc->sType != NULL && pNode->sType == NULL) ||
	     (pDesc->sTitle != NULL && pNode->sTitle == NULL) ||
	     (pDesc->sSummary != NULL && pNode->sSummary == NULL) ) {
		__xuiFlowFreeNode(pNode);
		xuiXrtArrayRemove(&pGraph->arrNodes, iPos, 1u);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xuiFlowDictSetIndex(&pGraph->mapNodes, pNode->sId, iIndex);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeNode(pNode);
		xuiXrtArrayRemove(&pGraph->arrNodes, iPos, 1u);
		return iRet;
	}
	if ( pIndex != NULL ) {
		*pIndex = iIndex;
	}
	__xuiFlowBumpGeometryRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphRemoveNode(xui_flow_graph pGraph, const char* sId)
{
	xui_flow_node_model_t* pNode;
	xui_flow_edge_model_t* pEdge;
	uint32_t iPos;
	int iNode;
	int i;
	int iRet;

	if ( (pGraph == NULL) || (sId == NULL) || (sId[0] == 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNode = xuiFlowGraphFindNode(pGraph, sId);
	if ( iNode < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( i = (int)pGraph->arrEdges.Count - 1; i >= 0; --i ) {
		pEdge = (xui_flow_edge_model_t*)xuiXrtArrayGet(&pGraph->arrEdges, (uint32_t)i + 1u);
		if ( strcmp(pEdge->sFromNode, sId) == 0 || strcmp(pEdge->sToNode, sId) == 0 ) {
			iRet = __xuiFlowRemoveEdgeAt(pGraph, i);
			if ( iRet != XUI_OK ) {
				return iRet;
			}
		}
	}
	iPos = (uint32_t)iNode + 1u;
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, iPos);
	__xuiFlowSelectionSet(&pGraph->arrSelectedNodes, &pGraph->mapSelectedNodes,
		pNode->sId, 0, NULL);
	(void)xuiXrtMapRemove(&pGraph->mapNodes, pNode->sId, strlen(pNode->sId));
	__xuiFlowFreeNode(pNode);
	xuiXrtArrayRemove(&pGraph->arrNodes, iPos, 1u);
	__xuiFlowUpdateMapIndexes(&pGraph->mapNodes, &pGraph->arrNodes, iPos,
		offsetof(xui_flow_node_model_t, sId));
	__xuiFlowRefreshEdgeRefs(pGraph);
	__xuiFlowBumpGeometryRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphRemoveEdge(xui_flow_graph pGraph, const char* sId)
{
	int iEdge;

	if ( (pGraph == NULL) || (sId == NULL) || (sId[0] == 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iEdge = xuiFlowGraphFindEdge(pGraph, sId);
	if ( iEdge < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iEdge = __xuiFlowRemoveEdgeAt(pGraph, iEdge);
	if ( iEdge == XUI_OK ) {
		__xuiFlowBumpGeometryRevision(pGraph);
	}
	return iEdge;
}

static int __xuiFlowAppendPortModel(xarray* pPorts, xmap* pPortMap,
	const xui_flow_port_desc_t* pDesc, int bDynamic, int* pIndex)
{
	xui_flow_port_model_t* pPort;
	uint32_t iPos;
	int iRet;

	if ( (pPorts == NULL) || (pPortMap == NULL) || !__xuiFlowPortDescValid(pDesc) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( __xuiFlowDictFindIndex(pPortMap, pDesc->sId) >= 0 ) {
		return XUI_ERROR_ALREADY_INITIALIZED;
	}
	iPos = xuiXrtArrayAppendSpace(pPorts, 1u);
	if ( iPos == 0u ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pPort = (xui_flow_port_model_t*)xuiXrtArrayGet(pPorts, iPos);
	memset(pPort, 0, sizeof(*pPort));
	pPort->sId = __xuiFlowCopyString(pDesc->sId);
	pPort->sTitle = __xuiFlowCopyString(pDesc->sTitle);
	pPort->sDataType = __xuiFlowCopyString(pDesc->sDataType);
	pPort->iDirection = pDesc->iDirection;
	pPort->iKind = pDesc->iKind;
	pPort->bRequired = pDesc->bRequired;
	pPort->bMulti = pDesc->bMulti;
	pPort->bDynamic = bDynamic ? 1 : 0;
	if ( pPort->sId == NULL || (pDesc->sTitle != NULL && pPort->sTitle == NULL) ||
	     (pDesc->sDataType != NULL && pPort->sDataType == NULL) ) {
		__xuiFlowFreePort(pPort);
		xuiXrtArrayRemove(pPorts, iPos, 1u);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xuiFlowDictSetIndex(pPortMap, pPort->sId, (int)iPos - 1);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreePort(pPort);
		xuiXrtArrayRemove(pPorts, iPos, 1u);
		return iRet;
	}
	if ( pIndex != NULL ) {
		*pIndex = (int)iPos - 1;
	}
	return XUI_OK;
}

static int __xuiFlowBuildReplacedDynamicPorts(const xui_flow_node_model_t* pNode,
	const xui_flow_port_desc_t* pPorts, int iPortCount, xarray* pBuiltPorts,
	xmap* pBuiltPortMap)
{
	xui_flow_port_model_t* pPort;
	xui_flow_port_desc_t tPort;
	int i;
	int iRet;

	if ( pNode == NULL || iPortCount < 0 || (iPortCount > 0 && pPorts == NULL) ||
	     pBuiltPorts == NULL || pBuiltPortMap == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(pBuiltPorts, 0, sizeof(*pBuiltPorts));
	memset(pBuiltPortMap, 0, sizeof(*pBuiltPortMap));
	xuiXrtArrayInit(pBuiltPorts, sizeof(xui_flow_port_model_t));
	xuiXrtMapInit(pBuiltPortMap, sizeof(int));
	for ( i = 0; i < (int)pNode->arrPorts.Count; ++i ) {
		pPort = (xui_flow_port_model_t*)xuiXrtArrayGet((xarray*)&pNode->arrPorts, (uint32_t)i + 1u);
		if ( pPort->bDynamic ) {
			continue;
		}
		memset(&tPort, 0, sizeof(tPort));
		tPort.iSize = sizeof(tPort);
		tPort.sId = pPort->sId;
		tPort.sTitle = pPort->sTitle;
		tPort.sDataType = pPort->sDataType;
		tPort.iDirection = pPort->iDirection;
		tPort.iKind = pPort->iKind;
		tPort.bRequired = pPort->bRequired;
		tPort.bMulti = pPort->bMulti;
		iRet = __xuiFlowAppendPortModel(pBuiltPorts, pBuiltPortMap, &tPort, 0, NULL);
		if ( iRet != XUI_OK ) {
			goto failed;
		}
	}
	for ( i = 0; i < iPortCount; ++i ) {
		tPort = pPorts[i];
		tPort.iSize = sizeof(tPort);
		iRet = __xuiFlowAppendPortModel(pBuiltPorts, pBuiltPortMap, &tPort, 1, NULL);
		if ( iRet != XUI_OK ) {
			goto failed;
		}
	}
	return XUI_OK;

failed:
	__xuiFlowFreePortArray(pBuiltPorts);
	xrtMapUnit(pBuiltPortMap);
	return iRet;
}

XUI_API int xuiFlowGraphAddPort(xui_flow_graph pGraph, int iNode, const xui_flow_port_desc_t* pDesc, int* pIndex)
{
	xui_flow_node_model_t* pNode;
	int iRet;

	if ( pIndex != NULL ) {
		*pIndex = -1;
	}
	if ( (pGraph == NULL) || !__xuiFlowPortDescValid(pDesc) || (iNode < 0) || ((uint32_t)iNode >= pGraph->arrNodes.Count) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	iRet = __xuiFlowAppendPortModel(&pNode->arrPorts, &pNode->mapPorts, pDesc, pDesc->bDynamic, pIndex);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	__xuiFlowBumpGeometryRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphRebuildNodeDynamicPorts(xui_flow_graph pGraph, const char* sNodeId, const xui_flow_port_desc_t* pPorts, int iPortCount)
{
	xui_flow_node_model_t* pNode;
	xarray arrPorts;
	xmap mapPorts;
	int iNode;
	int iRet;

	if ( (pGraph == NULL) || (sNodeId == NULL) || (sNodeId[0] == 0) || (iPortCount < 0) || (iPortCount > 0 && pPorts == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNode = xuiFlowGraphFindNode(pGraph, sNodeId);
	if ( iNode < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	iRet = __xuiFlowBuildReplacedDynamicPorts(pNode, pPorts, iPortCount,
		&arrPorts, &mapPorts);
	if ( iRet != XUI_OK ) return iRet;
	__xuiFlowFreePortArray(&pNode->arrPorts);
	xrtMapUnit(&pNode->mapPorts);
	pNode->arrPorts = arrPorts;
	pNode->mapPorts = mapPorts;
	__xuiFlowRefreshEdgeRefs(pGraph);
	__xuiFlowBumpGeometryRevision(pGraph);
	return XUI_OK;
}

int xuiInternalFlowGraphUpdateNodeConfiguration(xui_flow_graph pGraph,
	const char* sNodeId, xvalue* pConfig, const xui_flow_port_desc_t* pPorts,
	int iPortCount, int bReplaceDynamicPorts, const char* sSummary)
{
	xui_flow_node_model_t* pNode;
	xarray arrPorts;
	xmap mapPorts;
	xvalue* pConfigCopy;
	char* sSummaryCopy;
	int iNode;
	int iRet;

	if ( pGraph == NULL || sNodeId == NULL || sNodeId[0] == 0 ||
	     (bReplaceDynamicPorts && (iPortCount < 0 || (iPortCount > 0 && pPorts == NULL))) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNode = xuiFlowGraphFindNode(pGraph, sNodeId);
	if ( iNode < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	pConfigCopy = NULL;
	sSummaryCopy = NULL;
	memset(&arrPorts, 0, sizeof(arrPorts));
	memset(&mapPorts, 0, sizeof(mapPorts));
	iRet = __xuiFlowCopyValue(pConfig, &pConfigCopy);
	if ( iRet != XUI_OK ) goto failed;
	sSummaryCopy = __xuiFlowCopyString(sSummary);
	if ( sSummary != NULL && sSummaryCopy == NULL ) {
		iRet = XUI_ERROR_OUT_OF_MEMORY;
		goto failed;
	}
	if ( bReplaceDynamicPorts ) {
		iRet = __xuiFlowBuildReplacedDynamicPorts(pNode, pPorts, iPortCount,
			&arrPorts, &mapPorts);
		if ( iRet != XUI_OK ) goto failed;
	}
	if ( pNode->pConfig != NULL ) xrtValueRelease(pNode->pConfig);
	xrtFree(pNode->sSummary);
	pNode->pConfig = pConfigCopy;
	pNode->sSummary = sSummaryCopy;
	pConfigCopy = NULL;
	sSummaryCopy = NULL;
	if ( bReplaceDynamicPorts ) {
		__xuiFlowFreePortArray(&pNode->arrPorts);
		xrtMapUnit(&pNode->mapPorts);
		pNode->arrPorts = arrPorts;
		pNode->mapPorts = mapPorts;
		memset(&arrPorts, 0, sizeof(arrPorts));
		memset(&mapPorts, 0, sizeof(mapPorts));
		__xuiFlowRefreshEdgeRefs(pGraph);
		__xuiFlowBumpGeometryRevision(pGraph);
	} else {
		__xuiFlowBumpRevision(pGraph);
	}
	return XUI_OK;

failed:
	if ( pConfigCopy != NULL ) xrtValueRelease(pConfigCopy);
	xrtFree(sSummaryCopy);
	__xuiFlowFreePortArray(&arrPorts);
	xrtMapUnit(&mapPorts);
	return iRet;
}

XUI_API int xuiFlowGraphAddEdge(xui_flow_graph pGraph, const xui_flow_edge_desc_t* pDesc, int* pIndex)
{
	xui_flow_edge_model_t* pEdge;
	xui_flow_node_model_t* pFromNode;
	xui_flow_node_model_t* pToNode;
	xui_flow_port_model_t* pFromPort;
	xui_flow_port_model_t* pToPort;
	xui_flow_edge_model_t* pExistingEdge;
	xui_flow_node_model_t* pExistingFromNode;
	xui_flow_port_model_t* pExistingFromPort;
	uint32_t iPos;
	uint32_t i;
	int iIndex;
	int iFromNode;
	int iToNode;
	int iFromPort;
	int iToPort;
	int iRet;

	if ( pIndex != NULL ) {
		*pIndex = -1;
	}
	if ( (pGraph == NULL) || (pDesc == NULL) || (pDesc->iSize < sizeof(*pDesc)) ||
	     (pDesc->sId == NULL) || (pDesc->sId[0] == 0) ||
	     (pDesc->sFromNode == NULL) || (pDesc->sFromPort == NULL) ||
	     (pDesc->sToNode == NULL) || (pDesc->sToPort == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pDesc->iKind != XUI_FLOW_PORT_CONTROL) && (pDesc->iKind != XUI_FLOW_PORT_DATA) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xuiFlowRouteStyleValid(pDesc->iRouteStyle) || !__xuiFlowFinite(pDesc->fRouteBias) ||
	     !__xuiFlowFinite(pDesc->fRouteSourceOffset) || !__xuiFlowFinite(pDesc->fRouteTargetOffset) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( xuiFlowGraphFindEdge(pGraph, pDesc->sId) >= 0 ) {
		return XUI_ERROR_ALREADY_INITIALIZED;
	}
	iFromNode = xuiFlowGraphFindNode(pGraph, pDesc->sFromNode);
	iToNode = xuiFlowGraphFindNode(pGraph, pDesc->sToNode);
	if ( (iFromNode < 0) || (iToNode < 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pFromNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iFromNode + 1u);
	pToNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iToNode + 1u);
	iFromPort = __xuiFlowPortFind(pFromNode, pDesc->sFromPort);
	iToPort = __xuiFlowPortFind(pToNode, pDesc->sToPort);
	if ( (iFromPort < 0) || (iToPort < 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pFromPort = (xui_flow_port_model_t*)xuiXrtArrayGet(&pFromNode->arrPorts, (uint32_t)iFromPort + 1u);
	pToPort = (xui_flow_port_model_t*)xuiXrtArrayGet(&pToNode->arrPorts, (uint32_t)iToPort + 1u);
	if ( (pFromPort->iDirection != XUI_FLOW_PORT_OUTPUT) || (pToPort->iDirection != XUI_FLOW_PORT_INPUT) ||
	     (pFromPort->iKind != pDesc->iKind) || (pToPort->iKind != pDesc->iKind) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !pToPort->bMulti ) {
		for ( i = 1u; i <= pGraph->arrEdges.Count; ++i ) {
			pExistingEdge = (xui_flow_edge_model_t*)xuiXrtArrayGet(&pGraph->arrEdges, i);
			if ( pExistingEdge->bInvalid || pExistingEdge->iToNode != iToNode || pExistingEdge->iToPort != iToPort ||
			     pExistingEdge->iKind != pToPort->iKind || pExistingEdge->iFromNode < 0 ||
			     (uint32_t)pExistingEdge->iFromNode >= pGraph->arrNodes.Count ) continue;
			pExistingFromNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)pExistingEdge->iFromNode + 1u);
			if ( pExistingEdge->iFromPort < 0 || (uint32_t)pExistingEdge->iFromPort >= pExistingFromNode->arrPorts.Count ) continue;
			pExistingFromPort = (xui_flow_port_model_t*)xuiXrtArrayGet(&pExistingFromNode->arrPorts, (uint32_t)pExistingEdge->iFromPort + 1u);
			if ( pExistingFromPort->iDirection == XUI_FLOW_PORT_OUTPUT && pExistingFromPort->iKind == pExistingEdge->iKind ) {
				return XUI_ERROR_INVALID_ARGUMENT;
			}
		}
	}
	iPos = xuiXrtArrayAppendSpace(&pGraph->arrEdges, 1u);
	if ( iPos == 0u ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iIndex = (int)iPos - 1;
	pEdge = (xui_flow_edge_model_t*)xuiXrtArrayGet(&pGraph->arrEdges, iPos);
	memset(pEdge, 0, sizeof(*pEdge));
	pEdge->sId = __xuiFlowCopyString(pDesc->sId);
	pEdge->sFromNode = __xuiFlowCopyString(pDesc->sFromNode);
	pEdge->sFromPort = __xuiFlowCopyString(pDesc->sFromPort);
	pEdge->sToNode = __xuiFlowCopyString(pDesc->sToNode);
	pEdge->sToPort = __xuiFlowCopyString(pDesc->sToPort);
	pEdge->iKind = pDesc->iKind;
	pEdge->iRouteStyle = pDesc->iRouteStyle;
	pEdge->fRouteBias = pDesc->fRouteBias;
	pEdge->fRouteSourceOffset = pDesc->fRouteSourceOffset;
	pEdge->fRouteTargetOffset = pDesc->fRouteTargetOffset;
	pEdge->iFromNode = iFromNode;
	pEdge->iFromPort = iFromPort;
	pEdge->iToNode = iToNode;
	pEdge->iToPort = iToPort;
	if ( pEdge->sId == NULL || pEdge->sFromNode == NULL || pEdge->sFromPort == NULL || pEdge->sToNode == NULL || pEdge->sToPort == NULL ) {
		__xuiFlowFreeEdge(pEdge);
		xuiXrtArrayRemove(&pGraph->arrEdges, iPos, 1u);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xuiFlowDictSetIndex(&pGraph->mapEdges, pEdge->sId, iIndex);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeEdge(pEdge);
		xuiXrtArrayRemove(&pGraph->arrEdges, iPos, 1u);
		return iRet;
	}
	if ( pIndex != NULL ) {
		*pIndex = iIndex;
	}
	__xuiFlowBumpGeometryRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphAddEdgePreserveInvalid(xui_flow_graph pGraph, const xui_flow_edge_desc_t* pDesc, int* pIndex)
{
	xui_flow_edge_model_t* pEdge;
	uint32_t iPos;
	int iIndex;
	int iRet;

	if ( pIndex != NULL ) {
		*pIndex = -1;
	}
	if ( (pGraph == NULL) || (pDesc == NULL) || (pDesc->iSize < sizeof(*pDesc)) ||
	     (pDesc->sId == NULL) || (pDesc->sId[0] == 0) ||
	     (pDesc->sFromNode == NULL) || (pDesc->sFromPort == NULL) ||
	     (pDesc->sToNode == NULL) || (pDesc->sToPort == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pDesc->iKind != XUI_FLOW_PORT_CONTROL) && (pDesc->iKind != XUI_FLOW_PORT_DATA) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xuiFlowRouteStyleValid(pDesc->iRouteStyle) || !__xuiFlowFinite(pDesc->fRouteBias) ||
	     !__xuiFlowFinite(pDesc->fRouteSourceOffset) || !__xuiFlowFinite(pDesc->fRouteTargetOffset) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( xuiFlowGraphFindEdge(pGraph, pDesc->sId) >= 0 ) {
		return XUI_ERROR_ALREADY_INITIALIZED;
	}
	iPos = xuiXrtArrayAppendSpace(&pGraph->arrEdges, 1u);
	if ( iPos == 0u ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iIndex = (int)iPos - 1;
	pEdge = (xui_flow_edge_model_t*)xuiXrtArrayGet(&pGraph->arrEdges, iPos);
	memset(pEdge, 0, sizeof(*pEdge));
	pEdge->sId = __xuiFlowCopyString(pDesc->sId);
	pEdge->sFromNode = __xuiFlowCopyString(pDesc->sFromNode);
	pEdge->sFromPort = __xuiFlowCopyString(pDesc->sFromPort);
	pEdge->sToNode = __xuiFlowCopyString(pDesc->sToNode);
	pEdge->sToPort = __xuiFlowCopyString(pDesc->sToPort);
	pEdge->iKind = pDesc->iKind;
	pEdge->iRouteStyle = pDesc->iRouteStyle;
	pEdge->fRouteBias = pDesc->fRouteBias;
	pEdge->fRouteSourceOffset = pDesc->fRouteSourceOffset;
	pEdge->fRouteTargetOffset = pDesc->fRouteTargetOffset;
	if ( pEdge->sId == NULL || pEdge->sFromNode == NULL || pEdge->sFromPort == NULL || pEdge->sToNode == NULL || pEdge->sToPort == NULL ) {
		__xuiFlowFreeEdge(pEdge);
		xuiXrtArrayRemove(&pGraph->arrEdges, iPos, 1u);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xuiFlowDictSetIndex(&pGraph->mapEdges, pEdge->sId, iIndex);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeEdge(pEdge);
		xuiXrtArrayRemove(&pGraph->arrEdges, iPos, 1u);
		return iRet;
	}
	__xuiFlowRefreshEdgeRef(pGraph, pEdge);
	if ( pIndex != NULL ) {
		*pIndex = iIndex;
	}
	__xuiFlowBumpGeometryRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphFindNode(xui_flow_graph pGraph, const char* sId)
{
	if ( pGraph == NULL ) {
		return -1;
	}
	return __xuiFlowDictFindIndex(&pGraph->mapNodes, sId);
}

XUI_API int xuiFlowGraphFindEdge(xui_flow_graph pGraph, const char* sId)
{
	if ( pGraph == NULL ) {
		return -1;
	}
	return __xuiFlowDictFindIndex(&pGraph->mapEdges, sId);
}

XUI_API int xuiFlowGraphGetNodeCount(xui_flow_graph pGraph)
{
	return (pGraph != NULL) ? (int)pGraph->arrNodes.Count : 0;
}

XUI_API int xuiFlowGraphGetEdgeCount(xui_flow_graph pGraph)
{
	return (pGraph != NULL) ? (int)pGraph->arrEdges.Count : 0;
}

XUI_API int xuiFlowGraphGetNodePortCount(xui_flow_graph pGraph, int iNode)
{
	xui_flow_node_model_t* pNode;

	if ( (pGraph == NULL) || (iNode < 0) || ((uint32_t)iNode >= pGraph->arrNodes.Count) ) {
		return 0;
	}
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	return (int)pNode->arrPorts.Count;
}

XUI_API uint32_t xuiFlowGraphGetRevision(xui_flow_graph pGraph)
{
	return (pGraph != NULL) ? pGraph->iRevision : 0u;
}

XUI_API int xuiFlowGraphGetNode(xui_flow_graph pGraph, int iNode, xui_flow_node_info_t* pInfo)
{
	xui_flow_node_model_t* pNode;

	if ( (pGraph == NULL) || (pInfo == NULL) || (iNode < 0) || ((uint32_t)iNode >= pGraph->arrNodes.Count) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	memset(pInfo, 0, sizeof(*pInfo));
	pInfo->iSize = sizeof(*pInfo);
	pInfo->sId = pNode->sId;
	pInfo->sType = pNode->sType;
	pInfo->sTitle = pNode->sTitle;
	pInfo->sSummary = pNode->sSummary;
	pInfo->fX = pNode->fX;
	pInfo->fY = pNode->fY;
	pInfo->fW = pNode->fW;
	pInfo->fH = pNode->fH;
	pInfo->iRunState = pNode->iRunState;
	pInfo->sRunPreview = pNode->sRunPreview;
	return XUI_OK;
}

XUI_API int xuiFlowGraphGetPort(xui_flow_graph pGraph, int iNode, int iPort, xui_flow_port_info_t* pInfo)
{
	xui_flow_node_model_t* pNode;
	xui_flow_port_model_t* pPort;

	if ( (pGraph == NULL) || (pInfo == NULL) || (iNode < 0) || ((uint32_t)iNode >= pGraph->arrNodes.Count) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	if ( (iPort < 0) || ((uint32_t)iPort >= pNode->arrPorts.Count) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pPort = (xui_flow_port_model_t*)xuiXrtArrayGet(&pNode->arrPorts, (uint32_t)iPort + 1u);
	memset(pInfo, 0, sizeof(*pInfo));
	pInfo->iSize = sizeof(*pInfo);
	pInfo->sId = pPort->sId;
	pInfo->sTitle = pPort->sTitle;
	pInfo->sDataType = pPort->sDataType;
	pInfo->iDirection = pPort->iDirection;
	pInfo->iKind = pPort->iKind;
	pInfo->bRequired = pPort->bRequired;
	pInfo->bMulti = pPort->bMulti;
	pInfo->bDynamic = pPort->bDynamic;
	return XUI_OK;
}

XUI_API int xuiFlowGraphGetEdge(xui_flow_graph pGraph, int iEdge, xui_flow_edge_info_t* pInfo)
{
	xui_flow_edge_model_t* pEdge;

	if ( (pGraph == NULL) || (pInfo == NULL) || (iEdge < 0) || ((uint32_t)iEdge >= pGraph->arrEdges.Count) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pEdge = (xui_flow_edge_model_t*)xuiXrtArrayGet(&pGraph->arrEdges, (uint32_t)iEdge + 1u);
	memset(pInfo, 0, sizeof(*pInfo));
	pInfo->iSize = sizeof(*pInfo);
	pInfo->sId = pEdge->sId;
	pInfo->iKind = pEdge->iKind;
	pInfo->sFromNode = pEdge->sFromNode;
	pInfo->sFromPort = pEdge->sFromPort;
	pInfo->sToNode = pEdge->sToNode;
	pInfo->sToPort = pEdge->sToPort;
	pInfo->iFromNode = pEdge->iFromNode;
	pInfo->iFromPort = pEdge->iFromPort;
	pInfo->iToNode = pEdge->iToNode;
	pInfo->iToPort = pEdge->iToPort;
	pInfo->bInvalid = pEdge->bInvalid;
	pInfo->iRunState = pEdge->iRunState;
	pInfo->sRunPreview = pEdge->sRunPreview;
	pInfo->iRouteStyle = pEdge->iRouteStyle;
	pInfo->fRouteBias = pEdge->fRouteBias;
	pInfo->fRouteSourceOffset = pEdge->fRouteSourceOffset;
	pInfo->fRouteTargetOffset = pEdge->fRouteTargetOffset;
	return XUI_OK;
}

XUI_API int xuiFlowGraphSetNodePosition(xui_flow_graph pGraph, const char* sId, float fX, float fY)
{
	xui_flow_node_model_t* pNode;
	int iNode;

	if ( (pGraph == NULL) || (sId == NULL) || !__xuiFlowFinite(fX) || !__xuiFlowFinite(fY) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNode = xuiFlowGraphFindNode(pGraph, sId);
	if ( iNode < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	if ( (pNode->fX == fX) && (pNode->fY == fY) ) {
		return XUI_OK;
	}
	pNode->fX = fX;
	pNode->fY = fY;
	__xuiFlowBumpGeometryRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphSetNodeBounds(xui_flow_graph pGraph, const char* sId, float fX, float fY, float fW, float fH)
{
	xui_flow_node_model_t* pNode;
	int iNode;

	if ( (pGraph == NULL) || (sId == NULL) || !__xuiFlowFinite(fX) || !__xuiFlowFinite(fY) ||
	     !__xuiFlowFinite(fW) || !__xuiFlowFinite(fH) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNode = xuiFlowGraphFindNode(pGraph, sId);
	if ( iNode < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( fW < 24.0f ) fW = 24.0f;
	if ( fH < 24.0f ) fH = 24.0f;
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	if ( pNode->fX == fX && pNode->fY == fY && pNode->fW == fW && pNode->fH == fH ) {
		return XUI_OK;
	}
	pNode->fX = fX;
	pNode->fY = fY;
	pNode->fW = fW;
	pNode->fH = fH;
	__xuiFlowBumpGeometryRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphSetNodeSummary(xui_flow_graph pGraph, const char* sId, const char* sSummary)
{
	xui_flow_node_model_t* pNode;
	char* sCopy;
	int iNode;

	if ( (pGraph == NULL) || (sId == NULL) || (sId[0] == 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNode = xuiFlowGraphFindNode(pGraph, sId);
	if ( iNode < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	if ( __xuiFlowStringEqual(pNode->sSummary, sSummary) ) {
		return XUI_OK;
	}
	sCopy = __xuiFlowCopyString(sSummary);
	if ( sSummary != NULL && sCopy == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	xrtFree(pNode->sSummary);
	pNode->sSummary = sCopy;
	__xuiFlowBumpRevision(pGraph);
	return XUI_OK;
}

static int __xuiFlowRunStateValid(int iState, int bEdge)
{
	if ( bEdge ) {
		return iState >= XUI_WORKFLOW_EDGE_RUN_IDLE && iState <= XUI_WORKFLOW_EDGE_RUN_INVALID;
	}
	return iState >= XUI_WORKFLOW_NODE_RUN_IDLE && iState <= XUI_WORKFLOW_NODE_RUN_WARNING;
}

XUI_API int xuiFlowGraphSetNodeRunState(xui_flow_graph pGraph, const char* sId, int iState, const char* sPreview)
{
	xui_flow_node_model_t* pNode;
	char* sCopy;
	int iNode;

	if ( (pGraph == NULL) || (sId == NULL) || (sId[0] == 0) || !__xuiFlowRunStateValid(iState, 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNode = xuiFlowGraphFindNode(pGraph, sId);
	if ( iNode < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	if ( pNode->iRunState == iState && __xuiFlowStringEqual(pNode->sRunPreview, sPreview) ) {
		return XUI_OK;
	}
	sCopy = __xuiFlowCopyString(sPreview);
	if ( sPreview != NULL && sCopy == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	xrtFree(pNode->sRunPreview);
	pNode->sRunPreview = sCopy;
	pNode->iRunState = iState;
	__xuiFlowBumpRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphSetEdgeRunState(xui_flow_graph pGraph, const char* sId, int iState, const char* sPreview)
{
	xui_flow_edge_model_t* pEdge;
	char* sCopy;
	int iEdge;

	if ( (pGraph == NULL) || (sId == NULL) || (sId[0] == 0) || !__xuiFlowRunStateValid(iState, 1) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iEdge = xuiFlowGraphFindEdge(pGraph, sId);
	if ( iEdge < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pEdge = (xui_flow_edge_model_t*)xuiXrtArrayGet(&pGraph->arrEdges, (uint32_t)iEdge + 1u);
	if ( pEdge->iRunState == iState && __xuiFlowStringEqual(pEdge->sRunPreview, sPreview) ) {
		return XUI_OK;
	}
	sCopy = __xuiFlowCopyString(sPreview);
	if ( sPreview != NULL && sCopy == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	xrtFree(pEdge->sRunPreview);
	pEdge->sRunPreview = sCopy;
	pEdge->iRunState = iState;
	__xuiFlowBumpRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphSetEdgeRoute(xui_flow_graph pGraph, const char* sId, int iRouteStyle, float fRouteBias, float fSourceOffset, float fTargetOffset)
{
	xui_flow_edge_model_t* pEdge;
	int iEdge;

	if ( (pGraph == NULL) || (sId == NULL) || (sId[0] == 0) || !__xuiFlowRouteStyleValid(iRouteStyle) ||
	     !__xuiFlowFinite(fRouteBias) || !__xuiFlowFinite(fSourceOffset) || !__xuiFlowFinite(fTargetOffset) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iEdge = xuiFlowGraphFindEdge(pGraph, sId);
	if ( iEdge < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pEdge = (xui_flow_edge_model_t*)xuiXrtArrayGet(&pGraph->arrEdges, (uint32_t)iEdge + 1u);
	if ( pEdge->iRouteStyle == iRouteStyle && pEdge->fRouteBias == fRouteBias &&
	     pEdge->fRouteSourceOffset == fSourceOffset && pEdge->fRouteTargetOffset == fTargetOffset ) {
		return XUI_OK;
	}
	pEdge->iRouteStyle = iRouteStyle;
	pEdge->fRouteBias = fRouteBias;
	pEdge->fRouteSourceOffset = fSourceOffset;
	pEdge->fRouteTargetOffset = fTargetOffset;
	__xuiFlowBumpGeometryRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphSetNodeConfig(xui_flow_graph pGraph, const char* sId, xvalue* pConfig)
{
	xui_flow_node_model_t* pNode;
	xvalue* pCopy;
	int iNode;
	int iRet;

	if ( (pGraph == NULL) || (sId == NULL) || (sId[0] == 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNode = xuiFlowGraphFindNode(pGraph, sId);
	if ( iNode < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiFlowCopyValue(pConfig, &pCopy);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	if ( pNode->pConfig != NULL ) {
		xrtValueRelease(pNode->pConfig);
	}
	pNode->pConfig = pCopy;
	__xuiFlowBumpRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphGetNodeConfig(xui_flow_graph pGraph, const char* sId, xvalue** ppConfig)
{
	xui_flow_node_model_t* pNode;
	int iNode;

	if ( (pGraph == NULL) || (sId == NULL) || (sId[0] == 0) || (ppConfig == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppConfig = NULL;
	iNode = xuiFlowGraphFindNode(pGraph, sId);
	if ( iNode < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	return __xuiFlowCopyValue(pNode->pConfig, ppConfig);
}

XUI_API int xuiFlowGraphCommandAddNode(xui_flow_graph pGraph, const xui_flow_node_desc_t* pDesc, int* pIndex)
{
	xui_flow_command_model_t tCommand;
	uint32_t iUndoPos;
	int iRet;

	if ( pIndex != NULL ) {
		*pIndex = -1;
	}
	if ( pGraph == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiFlowInitCommand(&tCommand);
	tCommand.iType = XUI_FLOW_COMMAND_ADD_NODE;
	iRet = __xuiFlowCopyNodeDesc(&tCommand.tNode, pDesc);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	iRet = __xuiFlowReserveUndoCommand(pGraph, &iUndoPos);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	iRet = xuiFlowGraphAddNode(pGraph, pDesc, pIndex);
	if ( iRet != XUI_OK ) {
		__xuiFlowCancelReservedUndo(pGraph, iUndoPos);
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	__xuiFlowCommitReservedUndo(pGraph, iUndoPos, &tCommand);
	return XUI_OK;
}

XUI_API int xuiFlowGraphCommandRemoveNode(xui_flow_graph pGraph, const char* sId)
{
	xui_flow_command_model_t tCommand;
	uint32_t iUndoPos;
	int iRet;

	if ( pGraph == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiFlowInitCommand(&tCommand);
	iRet = __xuiFlowCommandSnapshotRemoveNode(pGraph, sId, &tCommand);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	iRet = __xuiFlowReserveUndoCommand(pGraph, &iUndoPos);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	iRet = xuiFlowGraphRemoveNode(pGraph, sId);
	if ( iRet != XUI_OK ) {
		__xuiFlowCancelReservedUndo(pGraph, iUndoPos);
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	__xuiFlowCommitReservedUndo(pGraph, iUndoPos, &tCommand);
	return XUI_OK;
}

XUI_API int xuiFlowGraphCommandMoveNode(xui_flow_graph pGraph, const xui_flow_move_node_desc_t* pDesc)
{
	xui_flow_command_model_t tCommand;
	xui_flow_node_info_t tNode;
	uint32_t iUndoPos;
	int iNode;
	int iRet;

	if ( (pGraph == NULL) || (pDesc == NULL) || (pDesc->iSize < sizeof(*pDesc)) || (pDesc->sId == NULL) ||
	     !__xuiFlowFinite(pDesc->fX) || !__xuiFlowFinite(pDesc->fY) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNode = xuiFlowGraphFindNode(pGraph, pDesc->sId);
	if ( iNode < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	iRet = xuiFlowGraphGetNode(pGraph, iNode, &tNode);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( (tNode.fX == pDesc->fX) && (tNode.fY == pDesc->fY) ) {
		return XUI_OK;
	}
	__xuiFlowInitCommand(&tCommand);
	tCommand.iType = XUI_FLOW_COMMAND_MOVE_NODE;
	tCommand.sNodeId = __xuiFlowCopyString(pDesc->sId);
	if ( tCommand.sNodeId == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	tCommand.fOldX = tNode.fX;
	tCommand.fOldY = tNode.fY;
	tCommand.fNewX = pDesc->fX;
	tCommand.fNewY = pDesc->fY;
	iRet = __xuiFlowReserveUndoCommand(pGraph, &iUndoPos);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	iRet = xuiFlowGraphSetNodePosition(pGraph, pDesc->sId, pDesc->fX, pDesc->fY);
	if ( iRet != XUI_OK ) {
		__xuiFlowCancelReservedUndo(pGraph, iUndoPos);
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	__xuiFlowCommitReservedUndo(pGraph, iUndoPos, &tCommand);
	return XUI_OK;
}

XUI_API int xuiFlowGraphCommandRecordMoveNode(xui_flow_graph pGraph, const char* sId, float fOldX, float fOldY, float fNewX, float fNewY)
{
	xui_flow_move_node_record_t tRecord;

	memset(&tRecord, 0, sizeof(tRecord));
	tRecord.iSize = sizeof(tRecord);
	tRecord.sId = sId;
	tRecord.fOldX = fOldX;
	tRecord.fOldY = fOldY;
	tRecord.fNewX = fNewX;
	tRecord.fNewY = fNewY;
	return xuiFlowGraphCommandRecordMoveNodes(pGraph, &tRecord, 1);
}

XUI_API int xuiFlowGraphCommandRecordMoveNodes(xui_flow_graph pGraph, const xui_flow_move_node_record_t* pRecords, int iRecordCount)
{
	xui_flow_command_model_t tCommand;
	xui_flow_move_node_record_t* pExisting;
	xui_flow_node_model_t* pNode;
	int i;
	int j;
	int iNode;
	int iRet;

	if ( (pGraph == NULL) || (pRecords == NULL) || (iRecordCount <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiFlowInitCommand(&tCommand);
	tCommand.iType = XUI_FLOW_COMMAND_MOVE_NODE;
	for ( i = 0; i < iRecordCount; ++i ) {
		if ( (pRecords[i].iSize < sizeof(pRecords[i])) || (pRecords[i].sId == NULL) ||
		     !__xuiFlowFinite(pRecords[i].fOldX) || !__xuiFlowFinite(pRecords[i].fOldY) ||
		     !__xuiFlowFinite(pRecords[i].fNewX) || !__xuiFlowFinite(pRecords[i].fNewY) ) {
			__xuiFlowFreeCommand(&tCommand);
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		iNode = xuiFlowGraphFindNode(pGraph, pRecords[i].sId);
		if ( iNode < 0 ) {
			__xuiFlowFreeCommand(&tCommand);
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
		if ( pNode->fX != pRecords[i].fNewX || pNode->fY != pRecords[i].fNewY ) {
			__xuiFlowFreeCommand(&tCommand);
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		if ( pRecords[i].fOldX == pRecords[i].fNewX && pRecords[i].fOldY == pRecords[i].fNewY ) {
			continue;
		}
		for ( j = 1; j <= (int)tCommand.arrMoves.Count; ++j ) {
			pExisting = (xui_flow_move_node_record_t*)xuiXrtArrayGet(&tCommand.arrMoves, (uint32_t)j);
			if ( strcmp(pExisting->sId, pRecords[i].sId) == 0 ) {
				__xuiFlowFreeCommand(&tCommand);
				return XUI_ERROR_INVALID_ARGUMENT;
			}
		}
		iRet = __xuiFlowCommandAppendMove(&tCommand, &pRecords[i]);
		if ( iRet != XUI_OK ) {
			__xuiFlowFreeCommand(&tCommand);
			return iRet;
		}
	}
	if ( tCommand.arrMoves.Count == 0u ) {
		__xuiFlowFreeCommand(&tCommand);
		return XUI_OK;
	}
	if ( __xuiFlowPushCommand(&pGraph->arrUndo, &tCommand) != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	__xuiFlowTrimHistory(pGraph);
	memset(&tCommand, 0, sizeof(tCommand));
	__xuiFlowClearRedo(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphCommandSetNodeConfig(xui_flow_graph pGraph, const char* sId, xvalue* pConfig)
{
	xui_flow_command_model_t tCommand;
	xui_flow_node_model_t* pNode;
	uint32_t iUndoPos;
	int iNode;
	int iRet;

	if ( (pGraph == NULL) || (sId == NULL) || (sId[0] == 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNode = xuiFlowGraphFindNode(pGraph, sId);
	if ( iNode < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	__xuiFlowInitCommand(&tCommand);
	tCommand.iType = XUI_FLOW_COMMAND_SET_NODE_CONFIG;
	tCommand.sNodeId = __xuiFlowCopyString(sId);
	if ( tCommand.sNodeId == NULL ) {
		__xuiFlowFreeCommand(&tCommand);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xuiFlowCopyValue(pNode->pConfig, &tCommand.pOldConfig);
	if ( iRet == XUI_OK ) {
		iRet = __xuiFlowCopyValue(pConfig, &tCommand.pNewConfig);
	}
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	iRet = __xuiFlowReserveUndoCommand(pGraph, &iUndoPos);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	iRet = xuiFlowGraphSetNodeConfig(pGraph, sId, pConfig);
	if ( iRet != XUI_OK ) {
		__xuiFlowCancelReservedUndo(pGraph, iUndoPos);
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	__xuiFlowCommitReservedUndo(pGraph, iUndoPos, &tCommand);
	return XUI_OK;
}

XUI_API int xuiFlowGraphCommandAddEdge(xui_flow_graph pGraph, const xui_flow_edge_desc_t* pDesc, int* pIndex)
{
	xui_flow_command_model_t tCommand;
	uint32_t iUndoPos;
	int iRet;

	if ( pIndex != NULL ) {
		*pIndex = -1;
	}
	if ( pGraph == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiFlowInitCommand(&tCommand);
	tCommand.iType = XUI_FLOW_COMMAND_ADD_EDGE;
	iRet = __xuiFlowCopyEdgeDesc(&tCommand.tEdge, pDesc);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	iRet = __xuiFlowReserveUndoCommand(pGraph, &iUndoPos);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	iRet = xuiFlowGraphAddEdge(pGraph, pDesc, pIndex);
	if ( iRet != XUI_OK ) {
		__xuiFlowCancelReservedUndo(pGraph, iUndoPos);
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	__xuiFlowCommitReservedUndo(pGraph, iUndoPos, &tCommand);
	return XUI_OK;
}

XUI_API int xuiFlowGraphCommandRemoveEdge(xui_flow_graph pGraph, const char* sId)
{
	xui_flow_command_model_t tCommand;
	uint32_t iUndoPos;
	int iRet;

	if ( pGraph == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiFlowInitCommand(&tCommand);
	iRet = __xuiFlowCommandSnapshotRemoveEdge(pGraph, sId, &tCommand);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	iRet = __xuiFlowReserveUndoCommand(pGraph, &iUndoPos);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	iRet = xuiFlowGraphRemoveEdge(pGraph, sId);
	if ( iRet != XUI_OK ) {
		__xuiFlowCancelReservedUndo(pGraph, iUndoPos);
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	__xuiFlowCommitReservedUndo(pGraph, iUndoPos, &tCommand);
	return XUI_OK;
}

static xui_flow_command_model_t* __xuiFlowAppendChildCommand(xui_flow_command_model_t* pCommand)
{
	xui_flow_command_model_t* pChild;
	uint32_t iPos;

	if ( pCommand == NULL ) return NULL;
	iPos = xuiXrtArrayAppendSpace(&pCommand->arrCommands, 1u);
	if ( iPos == 0u ) return NULL;
	pChild = (xui_flow_command_model_t*)xuiXrtArrayGet(&pCommand->arrCommands, iPos);
	if ( pChild == NULL ) {
		xuiXrtArrayRemove(&pCommand->arrCommands, iPos, 1u);
		return NULL;
	}
	__xuiFlowInitCommand(pChild);
	return pChild;
}

static int __xuiFlowSnapshotDeleteSelection(xui_flow_graph pGraph,
	xui_flow_command_model_t* pCommand)
{
	xui_flow_command_model_t* pChild;
	xui_flow_node_model_t* pNode;
	xui_flow_edge_model_t* pEdge;
	uint32_t i;
	int iRet;

	if ( pGraph == NULL || pCommand == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pCommand->iType = XUI_FLOW_COMMAND_DELETE_SELECTION;
	for ( i = 1u; i <= pGraph->arrNodes.Count; ++i ) {
		pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, i);
		if ( pNode == NULL || !xuiFlowGraphIsNodeSelected(pGraph, pNode->sId) ) continue;
		pChild = __xuiFlowAppendChildCommand(pCommand);
		if ( pChild == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		iRet = __xuiFlowCommandSnapshotRemoveNodeEx(pGraph, pNode->sId, pChild, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	for ( i = 1u; i <= pGraph->arrEdges.Count; ++i ) {
		pEdge = (xui_flow_edge_model_t*)xuiXrtArrayGet(&pGraph->arrEdges, i);
		if ( pEdge == NULL ) continue;
		if ( !xuiFlowGraphIsEdgeSelected(pGraph, pEdge->sId) &&
		     !xuiFlowGraphIsNodeSelected(pGraph, pEdge->sFromNode) &&
		     !xuiFlowGraphIsNodeSelected(pGraph, pEdge->sToNode) ) continue;
		pChild = __xuiFlowAppendChildCommand(pCommand);
		if ( pChild == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		iRet = __xuiFlowCommandSnapshotRemoveEdge(pGraph, pEdge->sId, pChild);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiFlowApplyDeleteSelectionCommand(xui_flow_graph pGraph,
	const xui_flow_command_model_t* pCommand, int bDelete)
{
	xui_flow_command_model_t* pChild;
	uint32_t i;
	int iRet;

	if ( pGraph == NULL || pCommand == NULL ||
	     pCommand->iType != XUI_FLOW_COMMAND_DELETE_SELECTION ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 1u; i <= pCommand->arrCommands.Count; ++i ) {
		pChild = (xui_flow_command_model_t*)xuiXrtArrayGet((xarray*)&pCommand->arrCommands, i);
		if ( pChild == NULL ||
		     (pChild->iType != XUI_FLOW_COMMAND_REMOVE_NODE && pChild->iType != XUI_FLOW_COMMAND_REMOVE_EDGE) )
			return XUI_ERROR_INVALID_STATE;
		if ( pChild->iType == XUI_FLOW_COMMAND_REMOVE_NODE ) {
			if ((xuiFlowGraphFindNode(pGraph, pChild->tNode.sId) >= 0) != !!bDelete)
				return XUI_ERROR_INVALID_STATE;
		} else if ((xuiFlowGraphFindEdge(pGraph, pChild->tEdge.sId) >= 0) != !!bDelete) {
			return XUI_ERROR_INVALID_STATE;
		}
	}
	if ( bDelete ) {
		for ( i = 1u; i <= pCommand->arrCommands.Count; ++i ) {
			pChild = (xui_flow_command_model_t*)xuiXrtArrayGet((xarray*)&pCommand->arrCommands, i);
			if ( pChild->iType != XUI_FLOW_COMMAND_REMOVE_EDGE ) continue;
			iRet = xuiFlowGraphRemoveEdge(pGraph, pChild->tEdge.sId);
			if ( iRet != XUI_OK ) return iRet;
		}
		for ( i = 1u; i <= pCommand->arrCommands.Count; ++i ) {
			pChild = (xui_flow_command_model_t*)xuiXrtArrayGet((xarray*)&pCommand->arrCommands, i);
			if ( pChild->iType != XUI_FLOW_COMMAND_REMOVE_NODE ) continue;
			iRet = xuiFlowGraphRemoveNode(pGraph, pChild->tNode.sId);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_OK;
	}
	for ( i = 1u; i <= pCommand->arrCommands.Count; ++i ) {
		pChild = (xui_flow_command_model_t*)xuiXrtArrayGet((xarray*)&pCommand->arrCommands, i);
		if ( pChild->iType != XUI_FLOW_COMMAND_REMOVE_NODE ) continue;
		iRet = __xuiFlowCommandRestoreRemovedNode(pGraph, pChild);
		if ( iRet != XUI_OK ) goto restore_failed;
	}
	for ( i = 1u; i <= pCommand->arrCommands.Count; ++i ) {
		pChild = (xui_flow_command_model_t*)xuiXrtArrayGet((xarray*)&pCommand->arrCommands, i);
		if ( pChild->iType != XUI_FLOW_COMMAND_REMOVE_EDGE ) continue;
		iRet = __xuiFlowCommandRestoreRemovedEdge(pGraph, &pChild->tEdge,
			pChild->iEdgeRunState, pChild->sEdgeRunPreview);
		if ( iRet != XUI_OK ) goto restore_failed;
	}
	return XUI_OK;

restore_failed:
	/* All restored IDs were absent before the operation, so removing them restores the deleted state. */
	for ( i = 1u; i <= pCommand->arrCommands.Count; ++i ) {
		pChild = (xui_flow_command_model_t*)xuiXrtArrayGet((xarray*)&pCommand->arrCommands, i);
		if ( pChild->iType == XUI_FLOW_COMMAND_REMOVE_EDGE &&
		     xuiFlowGraphFindEdge(pGraph, pChild->tEdge.sId) >= 0 )
			(void)xuiFlowGraphRemoveEdge(pGraph, pChild->tEdge.sId);
	}
	for ( i = 1u; i <= pCommand->arrCommands.Count; ++i ) {
		pChild = (xui_flow_command_model_t*)xuiXrtArrayGet((xarray*)&pCommand->arrCommands, i);
		if ( pChild->iType == XUI_FLOW_COMMAND_REMOVE_NODE &&
		     xuiFlowGraphFindNode(pGraph, pChild->tNode.sId) >= 0 )
			(void)xuiFlowGraphRemoveNode(pGraph, pChild->tNode.sId);
	}
	return iRet;
}

static int __xuiFlowApplyCommandMove(xui_flow_graph pGraph,
	const xui_flow_command_model_t* pCommand, int bUseNewPosition)
{
	xui_flow_move_node_record_t* pMove;
	xui_flow_node_model_t* pNode;
	float fX;
	float fY;
	uint32_t i;
	int iNode;
	int bChanged;

	if ( (pGraph == NULL) || (pCommand == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pCommand->arrMoves.Count > 0u ) {
		for ( i = 1u; i <= pCommand->arrMoves.Count; ++i ) {
			pMove = (xui_flow_move_node_record_t*)xuiXrtArrayGet((xarray*)&pCommand->arrMoves, i);
			fX = bUseNewPosition ? pMove->fNewX : pMove->fOldX;
			fY = bUseNewPosition ? pMove->fNewY : pMove->fOldY;
			if ( pMove->sId == NULL || !__xuiFlowFinite(fX) || !__xuiFlowFinite(fY) ||
			     xuiFlowGraphFindNode(pGraph, pMove->sId) < 0 ) {
				return XUI_ERROR_INVALID_ARGUMENT;
			}
		}
	} else {
		fX = bUseNewPosition ? pCommand->fNewX : pCommand->fOldX;
		fY = bUseNewPosition ? pCommand->fNewY : pCommand->fOldY;
		if ( pCommand->sNodeId == NULL || !__xuiFlowFinite(fX) || !__xuiFlowFinite(fY) ||
		     xuiFlowGraphFindNode(pGraph, pCommand->sNodeId) < 0 ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
	}
	bChanged = 0;
	if ( pCommand->arrMoves.Count > 0u ) {
		for ( i = 1u; i <= pCommand->arrMoves.Count; ++i ) {
			pMove = (xui_flow_move_node_record_t*)xuiXrtArrayGet((xarray*)&pCommand->arrMoves, i);
			iNode = xuiFlowGraphFindNode(pGraph, pMove->sId);
			pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
			fX = bUseNewPosition ? pMove->fNewX : pMove->fOldX;
			fY = bUseNewPosition ? pMove->fNewY : pMove->fOldY;
			if ( pNode->fX != fX || pNode->fY != fY ) {
				pNode->fX = fX;
				pNode->fY = fY;
				bChanged = 1;
			}
		}
	} else {
		iNode = xuiFlowGraphFindNode(pGraph, pCommand->sNodeId);
		pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
		fX = bUseNewPosition ? pCommand->fNewX : pCommand->fOldX;
		fY = bUseNewPosition ? pCommand->fNewY : pCommand->fOldY;
		if ( pNode->fX != fX || pNode->fY != fY ) {
			pNode->fX = fX;
			pNode->fY = fY;
			bChanged = 1;
		}
	}
	if ( bChanged ) {
		__xuiFlowBumpGeometryRevision(pGraph);
	}
	return XUI_OK;
}

XUI_API int xuiFlowGraphCanUndo(xui_flow_graph pGraph)
{
	return (pGraph != NULL && pGraph->arrUndo.Count > 0u) ? 1 : 0;
}

XUI_API int xuiFlowGraphCanRedo(xui_flow_graph pGraph)
{
	return (pGraph != NULL && pGraph->arrRedo.Count > 0u) ? 1 : 0;
}

XUI_API int xuiFlowGraphUndo(xui_flow_graph pGraph)
{
	xui_flow_command_model_t* pCommand;
	uint32_t iDestinationPos;
	int iRet;

	if ( pGraph == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiFlowPrepareCommandTransfer(&pGraph->arrUndo, &pGraph->arrRedo,
		&pCommand, &iDestinationPos);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( pCommand->iType == XUI_FLOW_COMMAND_ADD_NODE ) {
		iRet = xuiFlowGraphRemoveNode(pGraph, pCommand->tNode.sId);
	} else if ( pCommand->iType == XUI_FLOW_COMMAND_REMOVE_NODE ) {
		iRet = __xuiFlowCommandRestoreRemovedNode(pGraph, pCommand);
	} else if ( pCommand->iType == XUI_FLOW_COMMAND_MOVE_NODE ) {
		iRet = __xuiFlowApplyCommandMove(pGraph, pCommand, 0);
	} else if ( pCommand->iType == XUI_FLOW_COMMAND_ADD_EDGE ) {
		iRet = xuiFlowGraphRemoveEdge(pGraph, pCommand->tEdge.sId);
	} else if ( pCommand->iType == XUI_FLOW_COMMAND_REMOVE_EDGE ) {
		iRet = __xuiFlowCommandRestoreRemovedEdge(pGraph, &pCommand->tEdge,
			pCommand->iEdgeRunState, pCommand->sEdgeRunPreview);
	} else if ( pCommand->iType == XUI_FLOW_COMMAND_SET_NODE_CONFIG ) {
		iRet = xuiFlowGraphSetNodeConfig(pGraph, pCommand->sNodeId, pCommand->pOldConfig);
	} else if ( pCommand->iType == XUI_FLOW_COMMAND_DELETE_SELECTION ) {
		iRet = __xuiFlowApplyDeleteSelectionCommand(pGraph, pCommand, 0);
	} else {
		iRet = XUI_ERROR_UNSUPPORTED;
	}
	if ( iRet != XUI_OK ) {
		__xuiFlowCancelCommandTransfer(&pGraph->arrRedo, iDestinationPos);
		return iRet;
	}
	__xuiFlowCommitCommandTransfer(&pGraph->arrUndo, &pGraph->arrRedo, iDestinationPos);
	__xuiFlowTrimHistory(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphRedo(xui_flow_graph pGraph)
{
	xui_flow_command_model_t* pCommand;
	uint32_t iDestinationPos;
	int iRet;

	if ( pGraph == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiFlowPrepareCommandTransfer(&pGraph->arrRedo, &pGraph->arrUndo,
		&pCommand, &iDestinationPos);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( pCommand->iType == XUI_FLOW_COMMAND_ADD_NODE ) {
		iRet = xuiFlowGraphAddNode(pGraph, &pCommand->tNode, NULL);
	} else if ( pCommand->iType == XUI_FLOW_COMMAND_REMOVE_NODE ) {
		iRet = xuiFlowGraphRemoveNode(pGraph, pCommand->tNode.sId);
	} else if ( pCommand->iType == XUI_FLOW_COMMAND_MOVE_NODE ) {
		iRet = __xuiFlowApplyCommandMove(pGraph, pCommand, 1);
	} else if ( pCommand->iType == XUI_FLOW_COMMAND_ADD_EDGE ) {
		iRet = xuiFlowGraphAddEdge(pGraph, &pCommand->tEdge, NULL);
	} else if ( pCommand->iType == XUI_FLOW_COMMAND_REMOVE_EDGE ) {
		iRet = xuiFlowGraphRemoveEdge(pGraph, pCommand->tEdge.sId);
	} else if ( pCommand->iType == XUI_FLOW_COMMAND_SET_NODE_CONFIG ) {
		iRet = xuiFlowGraphSetNodeConfig(pGraph, pCommand->sNodeId, pCommand->pNewConfig);
	} else if ( pCommand->iType == XUI_FLOW_COMMAND_DELETE_SELECTION ) {
		iRet = __xuiFlowApplyDeleteSelectionCommand(pGraph, pCommand, 1);
	} else {
		iRet = XUI_ERROR_UNSUPPORTED;
	}
	if ( iRet != XUI_OK ) {
		__xuiFlowCancelCommandTransfer(&pGraph->arrUndo, iDestinationPos);
		return iRet;
	}
	__xuiFlowCommitCommandTransfer(&pGraph->arrRedo, &pGraph->arrUndo, iDestinationPos);
	__xuiFlowTrimHistory(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphSetCommandHistoryLimit(xui_flow_graph pGraph, int iLimit)
{
	if ( (pGraph == NULL) || (iLimit < 1) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pGraph->iCommandHistoryLimit = iLimit;
	__xuiFlowTrimHistory(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphGetCommandHistoryLimit(xui_flow_graph pGraph)
{
	return (pGraph != NULL) ? pGraph->iCommandHistoryLimit : 0;
}

XUI_API int xuiFlowGraphMarkClean(xui_flow_graph pGraph)
{
	if ( pGraph == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pGraph->iCleanRevision = pGraph->iRevision;
	return XUI_OK;
}

XUI_API int xuiFlowGraphIsDirty(xui_flow_graph pGraph)
{
	return (pGraph != NULL && pGraph->iRevision != pGraph->iCleanRevision) ? 1 : 0;
}

XUI_API uint32_t xuiFlowGraphGetCleanRevision(xui_flow_graph pGraph)
{
	return (pGraph != NULL) ? pGraph->iCleanRevision : 0u;
}

static int __xuiFlowCopyGeneratedId(char* sBuffer, int iCapacity, const char* sPrefix, uint32_t iValue)
{
	char sId[64];
	int iLength;

	if ( (sBuffer == NULL) || (iCapacity <= 0) || (sPrefix == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iLength = snprintf(sId, sizeof(sId), "%s%u", sPrefix, iValue);
	if ( iLength < 0 || iLength >= (int)sizeof(sId) || iLength >= iCapacity ) {
		sBuffer[0] = 0;
		return XUI_ERROR_LIMIT_EXCEEDED;
	}
	memcpy(sBuffer, sId, (size_t)iLength + 1u);
	return XUI_OK;
}

XUI_API int xuiFlowGraphMakeNodeId(xui_flow_graph pGraph, char* sBuffer, int iCapacity)
{
	char sId[64];
	uint32_t iNext;
	int iRet;

	if ( (pGraph == NULL) || (sBuffer == NULL) || (iCapacity <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNext = pGraph->iNextNodeId;
	do {
		iNext++;
		if ( iNext == 0u ) {
			sBuffer[0] = 0;
			return XUI_ERROR_LIMIT_EXCEEDED;
		}
		(void)snprintf(sId, sizeof(sId), "node_%u", iNext);
	} while ( xuiFlowGraphFindNode(pGraph, sId) >= 0 );
	iRet = __xuiFlowCopyGeneratedId(sBuffer, iCapacity, "node_", iNext);
	if ( iRet == XUI_OK ) {
		pGraph->iNextNodeId = iNext;
	}
	return iRet;
}

XUI_API int xuiFlowGraphMakeEdgeId(xui_flow_graph pGraph, char* sBuffer, int iCapacity)
{
	char sId[64];
	uint32_t iNext;
	int iRet;

	if ( (pGraph == NULL) || (sBuffer == NULL) || (iCapacity <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNext = pGraph->iNextEdgeId;
	do {
		iNext++;
		if ( iNext == 0u ) {
			sBuffer[0] = 0;
			return XUI_ERROR_LIMIT_EXCEEDED;
		}
		(void)snprintf(sId, sizeof(sId), "edge_%u", iNext);
	} while ( xuiFlowGraphFindEdge(pGraph, sId) >= 0 );
	iRet = __xuiFlowCopyGeneratedId(sBuffer, iCapacity, "edge_", iNext);
	if ( iRet == XUI_OK ) {
		pGraph->iNextEdgeId = iNext;
	}
	return iRet;
}

XUI_API int xuiFlowGraphMakePortId(xui_flow_graph pGraph, int iNode, char* sBuffer, int iCapacity)
{
	xui_flow_node_model_t* pNode;
	uint32_t iNext;
	char sId[64];
	int iRet;

	if ( (pGraph == NULL) || (sBuffer == NULL) || (iCapacity <= 0) || (iNode < 0) || ((uint32_t)iNode >= pGraph->arrNodes.Count) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pNode = (xui_flow_node_model_t*)xuiXrtArrayGet(&pGraph->arrNodes, (uint32_t)iNode + 1u);
	iNext = pNode->arrPorts.Count + 1u;
	if ( iNext == 0u ) {
		sBuffer[0] = 0;
		return XUI_ERROR_LIMIT_EXCEEDED;
	}
	do {
		(void)snprintf(sId, sizeof(sId), "port_%u", iNext);
		if ( __xuiFlowPortFind(pNode, sId) < 0 ) {
			break;
		}
		iNext++;
		if ( iNext == 0u ) {
			sBuffer[0] = 0;
			return XUI_ERROR_LIMIT_EXCEEDED;
		}
	} while ( 1 );
	iRet = __xuiFlowCopyGeneratedId(sBuffer, iCapacity, "port_", iNext);
	return iRet;
}

XUI_API int xuiFlowGraphClearSelection(xui_flow_graph pGraph)
{
	if ( pGraph == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pGraph->arrSelectedNodes.Count == 0u && pGraph->arrSelectedEdges.Count == 0u ) {
		return XUI_OK;
	}
	__xuiFlowFreeSelectionArray(&pGraph->arrSelectedNodes);
	__xuiFlowFreeSelectionArray(&pGraph->arrSelectedEdges);
	xrtMapUnit(&pGraph->mapSelectedNodes);
	xrtMapUnit(&pGraph->mapSelectedEdges);
	xuiXrtArrayInit(&pGraph->arrSelectedNodes, sizeof(char*));
	xuiXrtArrayInit(&pGraph->arrSelectedEdges, sizeof(char*));
	xuiXrtMapInit(&pGraph->mapSelectedNodes, sizeof(int));
	xuiXrtMapInit(&pGraph->mapSelectedEdges, sizeof(int));
	__xuiFlowBumpRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphSelectNode(xui_flow_graph pGraph, const char* sId, int bSelected)
{
	int bChanged;
	int iRet;

	if ( (pGraph == NULL) || (sId == NULL) || (xuiFlowGraphFindNode(pGraph, sId) < 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiFlowSelectionSet(&pGraph->arrSelectedNodes, &pGraph->mapSelectedNodes,
		sId, bSelected, &bChanged);
	if ( iRet == XUI_OK && bChanged ) {
		__xuiFlowBumpRevision(pGraph);
	}
	return iRet;
}

XUI_API int xuiFlowGraphSelectEdge(xui_flow_graph pGraph, const char* sId, int bSelected)
{
	int bChanged;
	int iRet;

	if ( (pGraph == NULL) || (sId == NULL) || (xuiFlowGraphFindEdge(pGraph, sId) < 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiFlowSelectionSet(&pGraph->arrSelectedEdges, &pGraph->mapSelectedEdges,
		sId, bSelected, &bChanged);
	if ( iRet == XUI_OK && bChanged ) {
		__xuiFlowBumpRevision(pGraph);
	}
	return iRet;
}

XUI_API int xuiFlowGraphIsNodeSelected(xui_flow_graph pGraph, const char* sId)
{
	return (pGraph != NULL && __xuiFlowSelectionFind(&pGraph->mapSelectedNodes, sId) >= 0) ? 1 : 0;
}

XUI_API int xuiFlowGraphIsEdgeSelected(xui_flow_graph pGraph, const char* sId)
{
	return (pGraph != NULL && __xuiFlowSelectionFind(&pGraph->mapSelectedEdges, sId) >= 0) ? 1 : 0;
}

XUI_API int xuiFlowGraphGetSelectedNodeCount(xui_flow_graph pGraph)
{
	return (pGraph != NULL) ? (int)pGraph->arrSelectedNodes.Count : 0;
}

XUI_API int xuiFlowGraphGetSelectedEdgeCount(xui_flow_graph pGraph)
{
	return (pGraph != NULL) ? (int)pGraph->arrSelectedEdges.Count : 0;
}

XUI_API int xuiFlowGraphDeleteSelection(xui_flow_graph pGraph)
{
	xui_flow_command_model_t tCommand;
	uint32_t iUndoPos;
	int iRet;

	if ( pGraph == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pGraph->arrSelectedNodes.Count == 0u && pGraph->arrSelectedEdges.Count == 0u ) {
		return XUI_OK;
	}
	__xuiFlowInitCommand(&tCommand);
	iRet = __xuiFlowSnapshotDeleteSelection(pGraph, &tCommand);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	if ( tCommand.arrCommands.Count == 0u ) {
		__xuiFlowFreeCommand(&tCommand);
		return xuiFlowGraphClearSelection(pGraph);
	}
	iRet = __xuiFlowReserveUndoCommand(pGraph, &iUndoPos);
	if ( iRet != XUI_OK ) {
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	iRet = __xuiFlowApplyDeleteSelectionCommand(pGraph, &tCommand, 1);
	if ( iRet != XUI_OK ) {
		__xuiFlowCancelReservedUndo(pGraph, iUndoPos);
		__xuiFlowFreeCommand(&tCommand);
		return iRet;
	}
	__xuiFlowCommitReservedUndo(pGraph, iUndoPos, &tCommand);
	return XUI_OK;
}

XUI_API int xuiFlowGraphSetViewport(xui_flow_graph pGraph, const xui_flow_viewport_t* pViewport)
{
	xui_flow_viewport_t tViewport;

	if ( (pGraph == NULL) || (pViewport == NULL) || (pViewport->iSize < sizeof(*pViewport)) ||
	     !__xuiFlowFinite(pViewport->fPanX) || !__xuiFlowFinite(pViewport->fPanY) ||
	     !__xuiFlowFinite(pViewport->fZoom) || !__xuiFlowFinite(pViewport->fWidth) ||
	     !__xuiFlowFinite(pViewport->fHeight) || pViewport->fWidth < 0.0f ||
	     pViewport->fHeight < 0.0f ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tViewport = *pViewport;
	tViewport.iSize = sizeof(tViewport);
	if ( tViewport.fZoom <= 0.001f ) {
		tViewport.fZoom = 0.001f;
	}
	if ( memcmp(&pGraph->tViewport, &tViewport, sizeof(tViewport)) == 0 ) {
		return XUI_OK;
	}
	pGraph->tViewport = tViewport;
	__xuiFlowBumpGeometryRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphGetViewport(xui_flow_graph pGraph, xui_flow_viewport_t* pViewport)
{
	if ( (pGraph == NULL) || (pViewport == NULL) || (pViewport->iSize < sizeof(*pViewport)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pViewport = pGraph->tViewport;
	pViewport->iSize = sizeof(*pViewport);
	return XUI_OK;
}

XUI_API int xuiFlowGraphWorldToScreen(xui_flow_graph pGraph, float fWorldX, float fWorldY, float* pScreenX, float* pScreenY)
{
	float fResultX;
	float fResultY;

	if ( (pGraph == NULL) || (pScreenX == NULL) || (pScreenY == NULL) ||
	     !__xuiFlowFinite(fWorldX) || !__xuiFlowFinite(fWorldY) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	fResultX = (fWorldX + pGraph->tViewport.fPanX) * pGraph->tViewport.fZoom;
	fResultY = (fWorldY + pGraph->tViewport.fPanY) * pGraph->tViewport.fZoom;
	if ( !__xuiFlowFinite(fResultX) || !__xuiFlowFinite(fResultY) ) {
		return XUI_ERROR_LIMIT_EXCEEDED;
	}
	*pScreenX = fResultX;
	*pScreenY = fResultY;
	return XUI_OK;
}

XUI_API int xuiFlowGraphScreenToWorld(xui_flow_graph pGraph, float fScreenX, float fScreenY, float* pWorldX, float* pWorldY)
{
	float fZoom;
	float fResultX;
	float fResultY;

	if ( (pGraph == NULL) || (pWorldX == NULL) || (pWorldY == NULL) ||
	     !__xuiFlowFinite(fScreenX) || !__xuiFlowFinite(fScreenY) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	fZoom = pGraph->tViewport.fZoom;
	if ( fZoom <= 0.001f ) {
		fZoom = 0.001f;
	}
	fResultX = (fScreenX / fZoom) - pGraph->tViewport.fPanX;
	fResultY = (fScreenY / fZoom) - pGraph->tViewport.fPanY;
	if ( !__xuiFlowFinite(fResultX) || !__xuiFlowFinite(fResultY) ) {
		return XUI_ERROR_LIMIT_EXCEEDED;
	}
	*pWorldX = fResultX;
	*pWorldY = fResultY;
	return XUI_OK;
}

XUI_API int xuiFlowGraphAddDiagnostic(xui_flow_graph pGraph, const xui_flow_diagnostic_desc_t* pDesc, int* pIndex)
{
	xui_flow_diagnostic_model_t* pDiagnostic;
	uint32_t iPos;

	if ( pIndex != NULL ) {
		*pIndex = -1;
	}
	if ( (pGraph == NULL) || (pDesc == NULL) || (pDesc->iSize < sizeof(*pDesc)) ||
	     (pDesc->iSeverity < XUI_CODE_DIAGNOSTIC_ERROR) || (pDesc->iSeverity > XUI_CODE_DIAGNOSTIC_HINT) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iPos = xuiXrtArrayAppendSpace(&pGraph->arrDiagnostics, 1u);
	if ( iPos == 0u ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pDiagnostic = (xui_flow_diagnostic_model_t*)xuiXrtArrayGet(&pGraph->arrDiagnostics, iPos);
	memset(pDiagnostic, 0, sizeof(*pDiagnostic));
	pDiagnostic->iSeverity = pDesc->iSeverity;
	pDiagnostic->sCode = __xuiFlowCopyString(pDesc->sCode);
	pDiagnostic->sMessage = __xuiFlowCopyString(pDesc->sMessage);
	pDiagnostic->sNode = __xuiFlowCopyString(pDesc->sNode);
	pDiagnostic->sEdge = __xuiFlowCopyString(pDesc->sEdge);
	pDiagnostic->sPath = __xuiFlowCopyString(pDesc->sPath);
	if ( (pDesc->sCode != NULL && pDiagnostic->sCode == NULL) ||
	     (pDesc->sMessage != NULL && pDiagnostic->sMessage == NULL) ||
	     (pDesc->sNode != NULL && pDiagnostic->sNode == NULL) ||
	     (pDesc->sEdge != NULL && pDiagnostic->sEdge == NULL) ||
	     (pDesc->sPath != NULL && pDiagnostic->sPath == NULL) ) {
		__xuiFlowFreeDiagnostic(pDiagnostic);
		xuiXrtArrayRemove(&pGraph->arrDiagnostics, iPos, 1u);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( pIndex != NULL ) {
		*pIndex = (int)iPos - 1;
	}
	__xuiFlowBumpRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphClearDiagnostics(xui_flow_graph pGraph)
{
	if ( pGraph == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pGraph->arrDiagnostics.Count == 0u ) {
		return XUI_OK;
	}
	__xuiFlowFreeDiagnostics(pGraph);
	xuiXrtArrayInit(&pGraph->arrDiagnostics, sizeof(xui_flow_diagnostic_model_t));
	__xuiFlowBumpRevision(pGraph);
	return XUI_OK;
}

XUI_API int xuiFlowGraphGetDiagnosticCount(xui_flow_graph pGraph)
{
	return (pGraph != NULL) ? (int)pGraph->arrDiagnostics.Count : 0;
}

XUI_API int xuiFlowGraphGetDiagnostic(xui_flow_graph pGraph, int iIndex, xui_flow_diagnostic_desc_t* pDesc)
{
	xui_flow_diagnostic_model_t* pDiagnostic;

	if ( (pGraph == NULL) || (pDesc == NULL) || (iIndex < 0) || ((uint32_t)iIndex >= pGraph->arrDiagnostics.Count) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDiagnostic = (xui_flow_diagnostic_model_t*)xuiXrtArrayGet(&pGraph->arrDiagnostics, (uint32_t)iIndex + 1u);
	if ( pDiagnostic == NULL ) {
		return XUI_ERROR_INVALID_STATE;
	}
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iSize = sizeof(*pDesc);
	pDesc->iSeverity = pDiagnostic->iSeverity;
	pDesc->sCode = pDiagnostic->sCode;
	pDesc->sMessage = pDiagnostic->sMessage;
	pDesc->sNode = pDiagnostic->sNode;
	pDesc->sEdge = pDiagnostic->sEdge;
	pDesc->sPath = pDiagnostic->sPath;
	return XUI_OK;
}

XUI_API int xuiFlowGraphGetNodeDiagnosticCount(xui_flow_graph pGraph, const char* sNodeId)
{
	xui_flow_diagnostic_model_t* pDiagnostic;
	uint32_t i;
	int iCount;

	if ( (pGraph == NULL) || (sNodeId == NULL) ) {
		return 0;
	}
	iCount = 0;
	for ( i = 1u; i <= pGraph->arrDiagnostics.Count; ++i ) {
		pDiagnostic = (xui_flow_diagnostic_model_t*)xuiXrtArrayGet(&pGraph->arrDiagnostics, i);
		if ( pDiagnostic != NULL && pDiagnostic->sNode != NULL && strcmp(pDiagnostic->sNode, sNodeId) == 0 ) {
			iCount++;
		}
	}
	return iCount;
}

XUI_API int xuiFlowGraphGetEdgeDiagnosticCount(xui_flow_graph pGraph, const char* sEdgeId)
{
	xui_flow_diagnostic_model_t* pDiagnostic;
	uint32_t i;
	int iCount;

	if ( (pGraph == NULL) || (sEdgeId == NULL) ) {
		return 0;
	}
	iCount = 0;
	for ( i = 1u; i <= pGraph->arrDiagnostics.Count; ++i ) {
		pDiagnostic = (xui_flow_diagnostic_model_t*)xuiXrtArrayGet(&pGraph->arrDiagnostics, i);
		if ( pDiagnostic != NULL && pDiagnostic->sEdge != NULL && strcmp(pDiagnostic->sEdge, sEdgeId) == 0 ) {
			iCount++;
		}
	}
	return iCount;
}

#endif
