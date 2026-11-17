#include "xui_config.h"
#if XUI_ENABLE_WORKFLOW
#include "xui_internal.h"
#include "xui_xrt_port.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "xge_data_lifetime_internal.h"
#include "xui_callback_hold_internal.h"

typedef struct xui_workflow_type_callbacks_t {
	xui_workflow_write_ports_proc onPorts;
	xui_workflow_write_diagnostics_proc onValidate;
	void* pUser;
	xui_callback_owner_t tOwner;
} xui_workflow_type_callbacks_t;

typedef struct xui_workflow_type_model_t {
	char* sId;
	int iVersion;
	char* sTitle;
	char* sCategory;
	char* sDescription;
	char* sIcon;
	xarray arrInputs;
	xarray arrOutputs;
	xvalue* pConfigSchema;
	xui_workflow_dynamic_ports_proc onDynamicPorts;
	void* pDynamicPortsUser;
	xui_workflow_validate_proc onValidate;
	void* pValidateUser;
	xui_workflow_type_callbacks_t tCallbacks;
} xui_workflow_type_model_t;

typedef struct xui_workflow_callback_input_t {
	xui_workflow_callback_node_t tNode;
	xvalue* pConfig;
} xui_workflow_callback_input_t;

struct xui_workflow_port_output_t {
	xarray arrPorts;
	int iStatus;
};

struct xui_workflow_diagnostic_output_t {
	xarray arrDiagnostics;
	const char* sNodeId;
	int iStatus;
};

typedef struct xui_workflow_port_result_t {
	xui_flow_port_desc_t* pPorts;
	void* pAllocation;
	int iCount;
} xui_workflow_port_result_t;

typedef struct xui_workflow_variable_model_t {
	char* sId;
	char* sTitle;
	char* sType;
	char* sScope;
	xvalue* pDefaultValue;
} xui_workflow_variable_model_t;

typedef struct xui_workflow_owned_t {
    xui_workflow pWorkflow;
    atomic_uint iRefs;
    unsigned iDepth;
} xui_workflow_owned_t;

typedef struct xui_workflow_operation_t {
    xui_workflow_owned_t* pOwner;
    xerror* pAmbient;
    bool bOuter;
} xui_workflow_operation_t;

struct xui_workflow_t {
	xui_flow_graph pGraph;
	xarray arrTypes;
	xarray arrVariables;
	xmap mapTypes;
	xmap mapVariables;
	uint32_t iNextVariableId;
    xui_workflow_owned_t* pOwned;
};

static xge_data_lifetime_t* __xuiWorkflowOwnerRoot(xui_workflow_owned_t* owner)
{return owner ? (xge_data_lifetime_t*)(owner+1):NULL;}
static int __xuiWorkflowOwnedOperationBegin(xui_workflow_owned_t*,xui_workflow_operation_t*);
static int __xuiWorkflowOwnedOperationEnd(xui_workflow_operation_t*,int,int);
static bool __xuiWorkflowLegacyReadable(xui_workflow);
static inline int __xuiWorkflowOperationBegin(xui_workflow workflow,xui_workflow_operation_t* operation)
{
    operation->pOwner=workflow ? workflow->pOwned:NULL;
    return operation->pOwner ? __xuiWorkflowOwnedOperationBegin(operation->pOwner,operation):XUI_OK;
}
static inline int __xuiWorkflowOperationEnd(xui_workflow_operation_t* operation,int result,int minimum)
{return operation->pOwner ? __xuiWorkflowOwnedOperationEnd(operation,result,minimum):result;}

static int __xuiWorkflowModelBusy(xui_workflow pWorkflow)
{
    if(!pWorkflow)return 0;
    if(pWorkflow->pOwned && __xuiWorkflowOwnerRoot(pWorkflow->pOwned)->iCreator!=xrtThreadCurrentId())return 1;
    return xuiInternalFlowGraphIsValueCallbackActive(pWorkflow->pGraph);
}

static void __xuiWorkflowReportBusy(void)
{
    if(!xrtGetError())xrtSetErrorInfo(XERR_STATE,"xui.flow_graph",XUI_ERROR_BUSY,
        "Workflow model is busy during a configuration value callback");
}

static int __xuiWorkflowDropCallbackOwner(xui_workflow workflow, const xui_callback_owner_t* owner)
{
	if (!owner->onDrop) return XUI_OK;
	xerror* first = xrtTakeError();
	xuiInternalFlowGraphValueCallbackEnter(workflow->pGraph);
	bool ok = owner->onDrop(owner->tReference.Data);
	xuiInternalFlowGraphValueCallbackLeave(workflow->pGraph);
	int status = ok && !xrtGetError() ? XUI_OK : XUI_ERROR_CALLBACK_FAILED;
	if (first) {
		xrtClearError();
		xrtSetErrorTake(first);
	}
	return status;
}

/* Only the actual Clone/Drop boundary is Busy. Normal internal calls
 * between boundaries remain valid, and independent models are unaffected. */
static xvalue* __xuiWorkflowValueClone(xui_workflow workflow,xvalue* value)
{
    xui_flow_graph graph=workflow ? workflow->pGraph:NULL;
    xuiInternalFlowGraphValueCallbackEnter(graph);
    xvalue* copy=xrtValueDeepClone(value);
    xuiInternalFlowGraphValueCallbackLeave(graph);
    return copy;
}
static xvalue* __xuiWorkflowValueRef(xui_workflow workflow,xvalue* value)
{
    xui_flow_graph graph=workflow ? workflow->pGraph:NULL;
    xuiInternalFlowGraphValueCallbackEnter(graph);
    xvalue* result=xrtValueRetain(value);
    xuiInternalFlowGraphValueCallbackLeave(graph);
    return result;
}
static bool __xuiWorkflowValueObjectSetTake(xui_workflow workflow,xvalue* object,
    const char* key,size_t size,xvalue* value)
{
    xui_flow_graph graph=workflow ? workflow->pGraph:NULL;
    xuiInternalFlowGraphValueCallbackEnter(graph);
    bool ok=xuiXrtValueObjectSetTake(object,key,size,value);
    xuiInternalFlowGraphValueCallbackLeave(graph);
    return ok;
}
static void __xuiWorkflowValueRelease(xui_workflow workflow,xvalue* value)
{
    if(!value)return;
    xui_flow_graph graph=workflow ? workflow->pGraph:NULL;
    xuiInternalFlowGraphValueCallbackEnter(graph);
    xerror* first=xrtTakeError();
    xrtValueRelease(value);
    if(first){xrtClearError();xrtSetErrorTake(first);}
    xuiInternalFlowGraphValueCallbackLeave(graph);
}

static int __xuiWorkflowBuildDynamicPorts(xui_workflow pWorkflow, const xui_workflow_type_model_t* pType, const char* sNodeId, xvalue* pConfig,
	xui_workflow_port_result_t* result);

static char* __xuiWorkflowCopyString(const char* sText)
{
	return xuiInternalStringDuplicateNullable(sText);
}

static void __xuiWorkflowFreePortDesc(xui_flow_port_desc_t* pPort)
{
	if ( pPort == NULL ) {
		return;
	}
	xrtFree((void*)pPort->sId);
	xrtFree((void*)pPort->sTitle);
	xrtFree((void*)pPort->sDataType);
	memset(pPort, 0, sizeof(*pPort));
}

static int __xuiWorkflowCopyPortDesc(xui_flow_port_desc_t* dst, const xui_flow_port_desc_t* src)
{
	*dst = *src;
	dst->sId = __xuiWorkflowCopyString(src->sId);
	dst->sTitle = __xuiWorkflowCopyString(src->sTitle);
	dst->sDataType = __xuiWorkflowCopyString(src->sDataType);
	if (!dst->sId || (src->sTitle && !dst->sTitle) || (src->sDataType && !dst->sDataType)) {
		__xuiWorkflowFreePortDesc(dst);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	return XUI_OK;
}

XUI_API int xuiWorkflowCallbackConfig(const xui_workflow_callback_node_t* pNode, xui_workflow_callback_config_t* pConfig)
{
	if (!pNode || !pConfig) return XUI_ERROR_INVALID_ARGUMENT;
	const xui_workflow_callback_input_t* input = (const xui_workflow_callback_input_t*)pNode;
	pConfig->pConfig = input->pConfig;
	return XUI_OK;
}

XUI_API int xuiWorkflowPortOutputAppend(xui_workflow_port_output_t* output, const xui_flow_port_desc_t* port)
{
	if (!output) return XUI_ERROR_INVALID_ARGUMENT;
	if (output->iStatus != XUI_OK) return output->iStatus;
	if (!port || port->iSize < sizeof(*port) || !port->sId || !port->sId[0] ||
	    (port->iDirection != XUI_FLOW_PORT_INPUT && port->iDirection != XUI_FLOW_PORT_OUTPUT) ||
	    (port->iKind != XUI_FLOW_PORT_CONTROL && port->iKind != XUI_FLOW_PORT_DATA))
		return output->iStatus = XUI_ERROR_INVALID_ARGUMENT;
	if (output->arrPorts.Count >= INT_MAX) return output->iStatus = XUI_ERROR_LIMIT_EXCEEDED;
	xui_flow_port_desc_t* dst = xrtArrayAdd(&output->arrPorts, 1);
	if (!dst) return output->iStatus = XUI_ERROR_OUT_OF_MEMORY;
	output->iStatus = __xuiWorkflowCopyPortDesc(dst, port);
	dst->bDynamic = 1;
	return output->iStatus;
}

static void __xuiWorkflowFreeDiagnosticDesc(xui_flow_diagnostic_desc_t* diagnostic)
{
	xrtFree((void*)diagnostic->sCode);
	xrtFree((void*)diagnostic->sMessage);
	xrtFree((void*)diagnostic->sNode);
	xrtFree((void*)diagnostic->sEdge);
	xrtFree((void*)diagnostic->sPath);
}

XUI_API int xuiWorkflowDiagnosticOutputAppend(xui_workflow_diagnostic_output_t* output,
	const xui_flow_diagnostic_desc_t* diagnostic)
{
	if (!output) return XUI_ERROR_INVALID_ARGUMENT;
	if (output->iStatus != XUI_OK) return output->iStatus;
	if (!diagnostic || diagnostic->iSize < sizeof(*diagnostic) ||
	    diagnostic->iSeverity < XUI_CODE_DIAGNOSTIC_ERROR || diagnostic->iSeverity > XUI_CODE_DIAGNOSTIC_HINT)
		return output->iStatus = XUI_ERROR_INVALID_ARGUMENT;
	if (output->arrDiagnostics.Count >= INT_MAX) return output->iStatus = XUI_ERROR_LIMIT_EXCEEDED;
	xui_flow_diagnostic_desc_t* dst = xrtArrayAdd(&output->arrDiagnostics, 1);
	if (!dst) return output->iStatus = XUI_ERROR_OUT_OF_MEMORY;
	const char* node = diagnostic->sNode ? diagnostic->sNode : output->sNodeId;
	*dst = *diagnostic;
	dst->sCode = __xuiWorkflowCopyString(diagnostic->sCode);
	dst->sMessage = __xuiWorkflowCopyString(diagnostic->sMessage);
	dst->sNode = __xuiWorkflowCopyString(node);
	dst->sEdge = __xuiWorkflowCopyString(diagnostic->sEdge);
	dst->sPath = __xuiWorkflowCopyString(diagnostic->sPath);
	if ((diagnostic->sCode && !dst->sCode) || (diagnostic->sMessage && !dst->sMessage) ||
	    (node && !dst->sNode) || (diagnostic->sEdge && !dst->sEdge) ||
	    (diagnostic->sPath && !dst->sPath))
		output->iStatus = XUI_ERROR_OUT_OF_MEMORY;
	return output->iStatus;
}

static int __xuiWorkflowCopyPortArray(xarray* pDst, const xui_flow_port_desc_t* pSrc, int iCount, int iExpectedDirection)
{
	xui_flow_port_desc_t* pPort;
	xui_flow_port_desc_t* pExisting;
	uint32_t iPos;
	uint32_t j;
	int i;

	if ( iCount < 0 || (iCount > 0 && pSrc == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < iCount; ++i ) {
		if ( pSrc[i].iSize < sizeof(pSrc[i]) || pSrc[i].sId == NULL || pSrc[i].sId[0] == 0 ||
		     pSrc[i].iDirection != iExpectedDirection ||
		     (pSrc[i].iKind != XUI_FLOW_PORT_CONTROL && pSrc[i].iKind != XUI_FLOW_PORT_DATA) ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		for ( j = 1u; j <= pDst->Count; ++j ) {
			pExisting = (xui_flow_port_desc_t*)xuiXrtArrayGet(pDst, j);
			if ( pExisting->sId != NULL && strcmp(pExisting->sId, pSrc[i].sId) == 0 ) {
				return XUI_ERROR_ALREADY_INITIALIZED;
			}
		}
		iPos = xuiXrtArrayAppendSpace(pDst, 1u);
		if ( iPos == 0u ) {
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		pPort = (xui_flow_port_desc_t*)xuiXrtArrayGet(pDst, iPos);
		int status = __xuiWorkflowCopyPortDesc(pPort, &pSrc[i]);
		if (status != XUI_OK) {
			xuiXrtArrayRemove(pDst, iPos, 1u);
			return status;
		}
	}
	return XUI_OK;
}

static int __xuiWorkflowFreeType(xui_workflow pWorkflow, xui_workflow_type_model_t* pType)
{
	uint32_t i;

	if ( pType == NULL ) {
		return XUI_OK;
	}
	for ( i = 1u; i <= pType->arrInputs.Count; ++i ) {
		__xuiWorkflowFreePortDesc((xui_flow_port_desc_t*)xuiXrtArrayGet(&pType->arrInputs, i));
	}
	for ( i = 1u; i <= pType->arrOutputs.Count; ++i ) {
		__xuiWorkflowFreePortDesc((xui_flow_port_desc_t*)xuiXrtArrayGet(&pType->arrOutputs, i));
	}
	xrtArrayUnit(&pType->arrInputs);
	xrtArrayUnit(&pType->arrOutputs);
	xrtFree(pType->sId);
	xrtFree(pType->sTitle);
	xrtFree(pType->sCategory);
	xrtFree(pType->sDescription);
	xrtFree(pType->sIcon);
	if ( pType->pConfigSchema != NULL ) {
		__xuiWorkflowValueRelease(pWorkflow, pType->pConfigSchema);
	}
	xui_callback_owner_t owner = pType->tCallbacks.tOwner;
	memset(pType, 0, sizeof(*pType));
	return __xuiWorkflowDropCallbackOwner(pWorkflow, &owner);
}

static void __xuiWorkflowFreeVariable(xui_workflow pWorkflow, xui_workflow_variable_model_t* pVariable)
{
	if ( pVariable == NULL ) {
		return;
	}
	xrtFree(pVariable->sId);
	xrtFree(pVariable->sTitle);
	xrtFree(pVariable->sType);
	xrtFree(pVariable->sScope);
	if ( pVariable->pDefaultValue != NULL ) {
		__xuiWorkflowValueRelease(pWorkflow, pVariable->pDefaultValue);
	}
	memset(pVariable, 0, sizeof(*pVariable));
}

static int __xuiWorkflowDictFindIndex(xmap* pDict, const char* sId)
{
	int* pValue;

	if ( (pDict == NULL) || (sId == NULL) || (sId[0] == 0) ) {
		return -1;
	}
	pValue = (int*)xuiXrtMapGet(pDict, (ptr)(void*)sId, (uint32)strlen(sId));
	return (pValue != NULL) ? *pValue : -1;
}

static int __xuiWorkflowDictSetIndex(xmap* pDict, const char* sId, int iIndex)
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

static int __xuiWorkflowConfigKindToXvoType(int iKind)
{
	switch ( iKind ) {
		case XUI_WORKFLOW_CONFIG_FIELD_STRING:
		case XUI_WORKFLOW_CONFIG_FIELD_TEXTAREA:
		case XUI_WORKFLOW_CONFIG_FIELD_EXPRESSION:
		case XUI_WORKFLOW_CONFIG_FIELD_SELECT:
		case XUI_WORKFLOW_CONFIG_FIELD_VARIABLE_REF:
		case XUI_WORKFLOW_CONFIG_FIELD_NODE_OUTPUT_REF:
			return XVALUE_STRING;
		case XUI_WORKFLOW_CONFIG_FIELD_MULTI_SELECT:
		case XUI_WORKFLOW_CONFIG_FIELD_ARRAY:
			return XVALUE_ARRAY;
		case XUI_WORKFLOW_CONFIG_FIELD_OBJECT:
		case XUI_WORKFLOW_CONFIG_FIELD_GROUP:
		case XUI_WORKFLOW_CONFIG_FIELD_TABS:
		case XUI_WORKFLOW_CONFIG_FIELD_CONDITION_BUILDER:
		case XUI_WORKFLOW_CONFIG_FIELD_MAPPING_BUILDER:
			return XVALUE_OBJECT;
		case XUI_WORKFLOW_CONFIG_FIELD_INT:
			return XVALUE_INT;
		case XUI_WORKFLOW_CONFIG_FIELD_FLOAT:
			return XVALUE_FLOAT;
		case XUI_WORKFLOW_CONFIG_FIELD_BOOL:
			return XVALUE_BOOL;
		default:
			return XVALUE_NULL;
	}
}

static int __xuiWorkflowConfigDefaultForKind(int iKind, xvalue** ppValue)
{
	if ( ppValue == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppValue = NULL;
	switch ( iKind ) {
		case XUI_WORKFLOW_CONFIG_FIELD_STRING:
		case XUI_WORKFLOW_CONFIG_FIELD_TEXTAREA:
		case XUI_WORKFLOW_CONFIG_FIELD_EXPRESSION:
		case XUI_WORKFLOW_CONFIG_FIELD_SELECT:
		case XUI_WORKFLOW_CONFIG_FIELD_VARIABLE_REF:
		case XUI_WORKFLOW_CONFIG_FIELD_NODE_OUTPUT_REF:
			*ppValue = xuiXrtValueCreateText("", 0, FALSE);
			break;
		case XUI_WORKFLOW_CONFIG_FIELD_MULTI_SELECT:
		case XUI_WORKFLOW_CONFIG_FIELD_ARRAY:
			*ppValue = xrtValueArray();
			break;
		case XUI_WORKFLOW_CONFIG_FIELD_OBJECT:
		case XUI_WORKFLOW_CONFIG_FIELD_GROUP:
		case XUI_WORKFLOW_CONFIG_FIELD_TABS:
		case XUI_WORKFLOW_CONFIG_FIELD_CONDITION_BUILDER:
		case XUI_WORKFLOW_CONFIG_FIELD_MAPPING_BUILDER:
			*ppValue = xrtValueObject();
			break;
		case XUI_WORKFLOW_CONFIG_FIELD_INT:
			*ppValue = xrtValueInt(0);
			break;
		case XUI_WORKFLOW_CONFIG_FIELD_FLOAT:
			*ppValue = xrtValueFloat(0.0);
			break;
		case XUI_WORKFLOW_CONFIG_FIELD_BOOL:
			*ppValue = xrtValueBool(FALSE);
			break;
		default:
			return XUI_ERROR_INVALID_ARGUMENT;
	}
	return (*ppValue != NULL) ? XUI_OK : XUI_ERROR_OUT_OF_MEMORY;
}

static xvalue* __xuiWorkflowConfigFields(xvalue* pSchema)
{
	xvalue* pFields;

	if ( (pSchema == NULL) || (xuiXrtValueType(pSchema) != XVALUE_OBJECT) ) {
		return NULL;
	}
	pFields = xuiXrtValueObjectGet(pSchema, "fields", 6);
	return (xuiXrtValueType(pFields) == XVALUE_ARRAY) ? pFields : NULL;
}

static xui_workflow_type_model_t* __xuiWorkflowFindTypeModel(xui_workflow pWorkflow, const char* sType)
{
	int iType;

	if ( (pWorkflow == NULL) || (sType == NULL) ) {
		return NULL;
	}
	iType = xuiWorkflowFindNodeType(pWorkflow, sType);
	return (iType >= 0) ? (xui_workflow_type_model_t*)xuiXrtArrayGet(&pWorkflow->arrTypes, (uint32_t)iType + 1u) : NULL;
}

static int __xuiWorkflowGetNodeType(xui_workflow pWorkflow, const char* sNodeId, xui_flow_node_info_t* pInfo)
{
	int iNode;
	int iRet;

	if ( (pWorkflow == NULL) || (sNodeId == NULL) || (sNodeId[0] == 0) || (pInfo == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNode = xuiFlowGraphFindNode(pWorkflow->pGraph, sNodeId);
	if ( iNode < 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiFlowGraphGetNode(pWorkflow->pGraph, iNode, pInfo);
	if ( iRet != XUI_OK || pInfo->sType == NULL || pInfo->sType[0] == 0 ) {
		return (iRet == XUI_OK) ? XUI_ERROR_INVALID_ARGUMENT : iRet;
	}
	return XUI_OK;
}

static void __xuiWorkflowSummaryAppend(char* sBuffer, int iCapacity, int* pOffset, const char* sText)
{
	int iWritten;

	if ( (sBuffer == NULL) || (iCapacity <= 0) || (pOffset == NULL) || (*pOffset >= iCapacity - 1) || (sText == NULL) ) {
		return;
	}
	iWritten = snprintf(sBuffer + *pOffset, (size_t)(iCapacity - *pOffset), "%s", sText);
	if ( iWritten < 0 ) {
		return;
	}
	*pOffset += iWritten;
	if ( *pOffset >= iCapacity ) {
		*pOffset = iCapacity - 1;
		sBuffer[iCapacity - 1] = '\0';
	}
}

static int __xuiWorkflowSummaryValue(char* sBuffer, int iCapacity, const char* sId, xvalue* pValue)
{
	const char* sText;

	if ( (sBuffer == NULL) || (iCapacity <= 0) || (sId == NULL) || (pValue == NULL) ) {
		return 0;
	}
	switch ( xuiXrtValueType(pValue) ) {
		case XVALUE_STRING:
			sText = (const char*)xuiXrtValueGetText(pValue);
			if ( sText == NULL || sText[0] == 0 ) return 0;
			snprintf(sBuffer, (size_t)iCapacity, "%s=%s", sId, sText);
			return 1;
		case XVALUE_INT:
			snprintf(sBuffer, (size_t)iCapacity, "%s=%lld", sId, (long long)xuiXrtValueGetInt(pValue));
			return 1;
		case XVALUE_FLOAT:
			snprintf(sBuffer, (size_t)iCapacity, "%s=%.2f", sId, xuiXrtValueGetFloat(pValue));
			return 1;
		case XVALUE_BOOL:
			snprintf(sBuffer, (size_t)iCapacity, "%s=%s", sId, xuiXrtValueGetBool(pValue) ? "true" : "false");
			return 1;
		default:
			break;
	}
	return 0;
}

static int __xuiWorkflowConfigFieldKind(xui_workflow pWorkflow, const char* sType, const char* sField)
{
	xui_workflow_type_model_t* pType;
	xvalue* pFields;
	xvalue* pField;
	const char* sId;
	uint32_t i;
	uint32_t iCount;

	if ( (pWorkflow == NULL) || (sType == NULL) || (sField == NULL) ) {
		return 0;
	}
	pType = __xuiWorkflowFindTypeModel(pWorkflow, sType);
	if ( pType == NULL ) {
		return 0;
	}
	pFields = __xuiWorkflowConfigFields(pType->pConfigSchema);
	if ( pFields == NULL ) {
		return 0;
	}
	iCount = xrtValueCount(pFields);
	for ( i = 0u; i < iCount; ++i ) {
		pField = xuiXrtValueArrayGet(pFields, i);
		if ( xuiXrtValueType(pField) != XVALUE_OBJECT ) {
			continue;
		}
		sId = (const char*)xuiXrtValueObjectGetText(pField, "id", 2);
		if ( sId != NULL && strcmp(sId, sField) == 0 ) {
			return (int)xuiXrtValueObjectGetInt(pField, "kind", 4);
		}
	}
	return 0;
}

static int __xuiWorkflowBuildNodeSummary(xui_workflow pWorkflow, const char* sType,
	xvalue* pConfig, char* sSummary, int iSummaryCapacity)
{
	xui_workflow_type_model_t* pType;
	xvalue* pFields;
	xvalue* pField;
	xvalue* pValue;
	const char* sId;
	char sPart[96];
	int iOffset;
	int iParts;
	uint32_t i;
	uint32_t iCount;

	if ( pWorkflow == NULL || sType == NULL || sSummary == NULL || iSummaryCapacity <= 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = __xuiWorkflowFindTypeModel(pWorkflow, sType);
	if ( pType == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	sSummary[0] = '\0';
	iOffset = 0;
	iParts = 0;
	pFields = __xuiWorkflowConfigFields(pType->pConfigSchema);
	if ( pFields != NULL && pConfig != NULL && xuiXrtValueType(pConfig) == XVALUE_OBJECT ) {
		iCount = xrtValueCount(pFields);
		for ( i = 0u; i < iCount && iParts < 3; ++i ) {
			pField = xuiXrtValueArrayGet(pFields, i);
			if ( xuiXrtValueType(pField) != XVALUE_OBJECT ) {
				continue;
			}
			sId = (const char*)xuiXrtValueObjectGetText(pField, "id", 2);
			if ( sId == NULL || sId[0] == 0 ) {
				continue;
			}
			pValue = xuiXrtValueObjectGet(pConfig, sId, (uint32_t)strlen(sId));
			if ( !__xuiWorkflowSummaryValue(sPart, (int)sizeof(sPart), sId, pValue) ) {
				continue;
			}
			if ( iParts > 0 ) {
				__xuiWorkflowSummaryAppend(sSummary, iSummaryCapacity, &iOffset, ", ");
			}
			__xuiWorkflowSummaryAppend(sSummary, iSummaryCapacity, &iOffset, sPart);
			iParts++;
		}
	}
	return XUI_OK;
}

static int __xuiWorkflowRefreshNodeSummary(xui_workflow pWorkflow, const char* sNodeId, const char* sType, xvalue* pConfig)
{
	char sSummary[256];
	int iRet;

	if ( sNodeId == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiWorkflowBuildNodeSummary(pWorkflow, sType, pConfig,
		sSummary, (int)sizeof(sSummary));
	if ( iRet != XUI_OK ) return iRet;
	return xuiFlowGraphSetNodeSummary(pWorkflow->pGraph, sNodeId,
		(sSummary[0] != 0) ? sSummary : NULL);
}

static void __xuiWorkflowConfigAddDiagnostic(xui_workflow_config_diagnostic_t* pDiagnostics, int iDiagnosticCapacity, int* pDiagnosticCount, const char* sPath, const char* sCode, const char* sMessage)
{
	xui_workflow_config_diagnostic_t* pDiagnostic;
	int iIndex;

	if ( pDiagnosticCount == NULL ) {
		return;
	}
	iIndex = *pDiagnosticCount;
	(*pDiagnosticCount)++;
	if ( (pDiagnostics == NULL) || (iIndex < 0) || (iIndex >= iDiagnosticCapacity) ) {
		return;
	}
	pDiagnostic = &pDiagnostics[iIndex];
	memset(pDiagnostic, 0, sizeof(*pDiagnostic));
	pDiagnostic->iSize = sizeof(*pDiagnostic);
	pDiagnostic->sPath = sPath;
	pDiagnostic->sCode = sCode;
	pDiagnostic->sMessage = sMessage;
}

XUI_API int xuiWorkflowCreate(xui_workflow* ppWorkflow)
{
	xui_workflow pWorkflow;
	int iRet;

	if ( ppWorkflow == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppWorkflow = NULL;
	pWorkflow = (xui_workflow)xrtCalloc(1u, sizeof(*pWorkflow));
	if ( pWorkflow == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = xuiFlowGraphCreate(&pWorkflow->pGraph);
	if ( iRet != XUI_OK ) {
		xrtFree(pWorkflow);
		return iRet;
	}
	xuiXrtArrayInit(&pWorkflow->arrTypes, sizeof(xui_workflow_type_model_t));
	xuiXrtArrayInit(&pWorkflow->arrVariables, sizeof(xui_workflow_variable_model_t));
	xuiXrtMapInit(&pWorkflow->mapTypes, sizeof(int));
	xuiXrtMapInit(&pWorkflow->mapVariables, sizeof(int));
	*ppWorkflow = pWorkflow;
	return XUI_OK;
}

static void __xuiWorkflowDestroyTables(xui_workflow pWorkflow)
{
    uint32_t i;
	for ( i = 1u; i <= pWorkflow->arrTypes.Count; ++i ) {
		__xuiWorkflowFreeType(pWorkflow, (xui_workflow_type_model_t*)xuiXrtArrayGet(&pWorkflow->arrTypes, i));
	}
	for ( i = 1u; i <= pWorkflow->arrVariables.Count; ++i ) {
		__xuiWorkflowFreeVariable(pWorkflow, (xui_workflow_variable_model_t*)xuiXrtArrayGet(&pWorkflow->arrVariables, i));
	}
	xrtArrayUnit(&pWorkflow->arrTypes);
	xrtArrayUnit(&pWorkflow->arrVariables);
	xrtMapUnit(&pWorkflow->mapTypes);
	xrtMapUnit(&pWorkflow->mapVariables);
}

XUI_API void xuiWorkflowDestroy(xui_workflow pWorkflow)
{
    if(xuiWorkflowDestroyChecked(pWorkflow)!=XUI_OK)__xuiWorkflowReportBusy();
}

XUI_API int xuiWorkflowDestroyChecked(xui_workflow pWorkflow)
{
    if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    if(!pWorkflow)return XUI_OK;
    if(pWorkflow->pOwned){bool consumed=false;return xuiWorkflowReleaseOwnerChecked(pWorkflow,&consumed);}
    __xuiWorkflowDestroyTables(pWorkflow);
    xuiFlowGraphDestroy(pWorkflow->pGraph);
    xrtFree(pWorkflow);
    return XUI_OK;
}

XUI_API xui_flow_graph xuiWorkflowGetGraph(xui_workflow pWorkflow)
{
	if(__xuiWorkflowModelBusy(pWorkflow)){__xuiWorkflowReportBusy();return NULL;}
    if(!__xuiWorkflowLegacyReadable(pWorkflow))return NULL;
    return pWorkflow->pGraph;
}

static bool __xuiWorkflowTraceValues(xui_workflow pWorkflow, xrtownershipvisitor onVisit, void* pUser)
{
    if(!pWorkflow || !onVisit)return false;
    if(xuiFlowGraphIsOwned(pWorkflow->pGraph)){
        xrtownershipref ref=xuiFlowGraphOwnership(pWorkflow->pGraph);
        if(!ref.Data || !onVisit(ref,pUser))return false;
    }else if(!xuiFlowGraphTraceOwnedValues(pWorkflow->pGraph,onVisit,pUser))return false;
	for ( size_t i = 0; i < pWorkflow->arrTypes.Count; ++i ) {
		const xui_workflow_type_model_t* pType = xrtArrayGet(&pWorkflow->arrTypes, i);
		if ( pType->pConfigSchema != NULL && !onVisit(xrtValueOwnership(pType->pConfigSchema), pUser) ) return false;
		xrtownershipref callback = pType->tCallbacks.tOwner.tReference;
		if (callback.Data && !onVisit(callback, pUser)) return false;
	}
	for ( size_t i = 0; i < pWorkflow->arrVariables.Count; ++i ) {
		const xui_workflow_variable_model_t* pVariable = xrtArrayGet(&pWorkflow->arrVariables, i);
		if ( pVariable->pDefaultValue != NULL && !onVisit(xrtValueOwnership(pVariable->pDefaultValue), pUser) ) return false;
	}
	return true;
}

XUI_API bool xuiWorkflowTraceOwnedValues(xui_workflow workflow,xrtownershipvisitor visitor,void* user)
{
    if(__xuiWorkflowModelBusy(workflow)){__xuiWorkflowReportBusy();return false;}
    if(workflow && workflow->pOwned && !xuiWorkflowOwnershipReadReady(xuiWorkflowOwnership(workflow)))return false;
    return __xuiWorkflowTraceValues(workflow,visitor,user);
}

XUI_API int xuiWorkflowConfigSchemaCreate(xvalue** ppSchema)
{
	xvalue* pSchema;
	xvalue* pFields;

	if ( ppSchema == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*ppSchema = NULL;
	pSchema = xrtValueObject();
	if ( pSchema == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pFields = xrtValueArray();
	if ( pFields == NULL ) {
		xrtValueRelease(pSchema);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( !xuiXrtValueObjectSetTake(pSchema, "fields", 6, pFields, TRUE) ) {
		xrtValueRelease(pFields);
		xrtValueRelease(pSchema);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	*ppSchema = pSchema;
	return XUI_OK;
}

static int __xuiWorkflowConfigSchemaAddFieldValue(xvalue* pSchema, const xui_workflow_config_field_desc_t* pDesc,bool shared)
{
	xvalue* pFields;
	xvalue* pField;
	xvalue* pExisting;
	xvalue* pDefaultCopy;
	xvalue* pMetaCopy;
	const char* sExistingId;
	int iExpectedType;
	int iDefaultType;
	double fDefault;
	uint32_t i;

	if ( (pSchema == NULL) || (pDesc == NULL) || (pDesc->iSize < sizeof(*pDesc)) ||
	     (pDesc->sId == NULL) || (pDesc->sId[0] == 0) ||
	     (__xuiWorkflowConfigKindToXvoType(pDesc->iKind) == XVALUE_NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pDesc->bHasMin || pDesc->bHasMax) &&
	     (pDesc->iKind != XUI_WORKFLOW_CONFIG_FIELD_INT) && (pDesc->iKind != XUI_WORKFLOW_CONFIG_FIELD_FLOAT) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pDesc->bHasMin && !isfinite(pDesc->fMin)) || (pDesc->bHasMax && !isfinite(pDesc->fMax)) ||
	     (pDesc->bHasMin && pDesc->bHasMax && pDesc->fMin > pDesc->fMax) ||
	     (pDesc->bHasDefault && pDesc->pDefaultValue == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pDesc->bHasDefault ) {
		iExpectedType = __xuiWorkflowConfigKindToXvoType(pDesc->iKind);
		iDefaultType = xuiXrtValueType(pDesc->pDefaultValue);
		if ( iDefaultType != XVALUE_NULL && iDefaultType != iExpectedType &&
		     !(iExpectedType == XVALUE_FLOAT && iDefaultType == XVALUE_INT) ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		if ( iDefaultType != XVALUE_NULL &&
		     (pDesc->iKind == XUI_WORKFLOW_CONFIG_FIELD_INT || pDesc->iKind == XUI_WORKFLOW_CONFIG_FIELD_FLOAT) ) {
			fDefault = (iDefaultType == XVALUE_INT) ? (double)xuiXrtValueGetInt(pDesc->pDefaultValue) : xuiXrtValueGetFloat(pDesc->pDefaultValue);
			if ( !isfinite(fDefault) || (pDesc->bHasMin && fDefault < pDesc->fMin) || (pDesc->bHasMax && fDefault > pDesc->fMax) ) {
				return XUI_ERROR_INVALID_ARGUMENT;
			}
		}
	}
	pFields = __xuiWorkflowConfigFields(pSchema);
	if ( pFields == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0u; i < xrtValueCount(pFields); ++i ) {
		pExisting = xuiXrtValueArrayGet(pFields, i);
		sExistingId = (const char*)xuiXrtValueObjectGetText(pExisting, "id", 2);
		if ( sExistingId != NULL && strcmp(sExistingId, pDesc->sId) == 0 ) {
			return XUI_ERROR_ALREADY_INITIALIZED;
		}
	}
	pField = xrtValueObject();
	if ( pField == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( !xuiXrtValueObjectSetText(pField, "id", 2, (ptr)(void*)pDesc->sId, (uint32_t)strlen(pDesc->sId), FALSE) ||
	     !xuiXrtValueObjectSetInt(pField, "kind", 4, pDesc->iKind) ||
	     !xuiXrtValueObjectSetBool(pField, "required", 8, pDesc->bRequired ? TRUE : FALSE) ) {
		xrtValueRelease(pField);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( pDesc->sTitle != NULL && !xuiXrtValueObjectSetText(pField, "title", 5, (ptr)(void*)pDesc->sTitle, (uint32_t)strlen(pDesc->sTitle), FALSE) ) {
		xrtValueRelease(pField);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( pDesc->sExpressionLanguage != NULL && !xuiXrtValueObjectSetText(pField, "expressionLanguage", 18, (ptr)(void*)pDesc->sExpressionLanguage, (uint32_t)strlen(pDesc->sExpressionLanguage), FALSE) ) {
		xrtValueRelease(pField);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( pDesc->sRefScope != NULL && !xuiXrtValueObjectSetText(pField, "refScope", 8, (ptr)(void*)pDesc->sRefScope, (uint32_t)strlen(pDesc->sRefScope), FALSE) ) {
		xrtValueRelease(pField);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( pDesc->sRefType != NULL && !xuiXrtValueObjectSetText(pField, "refType", 7, (ptr)(void*)pDesc->sRefType, (uint32_t)strlen(pDesc->sRefType), FALSE) ) {
		xrtValueRelease(pField);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( pDesc->sGroup != NULL && !xuiXrtValueObjectSetText(pField, "group", 5, (ptr)(void*)pDesc->sGroup, (uint32_t)strlen(pDesc->sGroup), FALSE) ) {
		xrtValueRelease(pField);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( pDesc->sTab != NULL && !xuiXrtValueObjectSetText(pField, "tab", 3, (ptr)(void*)pDesc->sTab, (uint32_t)strlen(pDesc->sTab), FALSE) ) {
		xrtValueRelease(pField);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( pDesc->pOptions != NULL ) {
		pMetaCopy = (shared ? xrtValueRetain(pDesc->pOptions) : xrtValueDeepClone(pDesc->pOptions));
		if ( pMetaCopy == NULL ) {
			xrtValueRelease(pField);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		if ( !xuiXrtValueObjectSetTake(pField, "options", 7, pMetaCopy, TRUE) ) {
			xrtValueRelease(pMetaCopy);
			xrtValueRelease(pField);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( pDesc->pChildren != NULL ) {
		pMetaCopy = (shared ? xrtValueRetain(pDesc->pChildren) : xrtValueDeepClone(pDesc->pChildren));
		if ( pMetaCopy == NULL ) {
			xrtValueRelease(pField);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		if ( !xuiXrtValueObjectSetTake(pField, "children", 8, pMetaCopy, TRUE) ) {
			xrtValueRelease(pMetaCopy);
			xrtValueRelease(pField);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( pDesc->bHasMin && !xuiXrtValueObjectSetFloat(pField, "min", 3, pDesc->fMin) ) {
		xrtValueRelease(pField);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( pDesc->bHasMax && !xuiXrtValueObjectSetFloat(pField, "max", 3, pDesc->fMax) ) {
		xrtValueRelease(pField);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( pDesc->bHasDefault ) {
		pDefaultCopy = (shared ? xrtValueRetain(pDesc->pDefaultValue) : xrtValueDeepClone(pDesc->pDefaultValue));
		if ( pDefaultCopy == NULL ) {
			xrtValueRelease(pField);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		if ( !xuiXrtValueObjectSetTake(pField, "default", 7, pDefaultCopy, TRUE) ) {
			xrtValueRelease(pDefaultCopy);
			xrtValueRelease(pField);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( !xuiXrtValueArrayAppendTake(pFields, pField, TRUE) ) {
		xrtValueRelease(pField);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	return XUI_OK;
}

XUI_API int xuiWorkflowConfigSchemaAddField(xvalue* pSchema,const xui_workflow_config_field_desc_t* pDesc)
{return __xuiWorkflowConfigSchemaAddFieldValue(pSchema,pDesc,false);}
XUI_API int xuiWorkflowConfigSchemaAddFieldShared(xvalue* pSchema,const xui_workflow_config_field_desc_t* pDesc)
{return __xuiWorkflowConfigSchemaAddFieldValue(pSchema,pDesc,true);}

static int __xuiWorkflowRegisterNodeTypeValue(xui_workflow pWorkflow, const xui_workflow_node_type_desc_t* pDesc,
	int* pIndex, bool shared, const xui_workflow_type_callbacks_t* callbacks)
{
	xui_workflow_type_model_t* pType;
	xui_flow_port_desc_t* pInput;
	xui_flow_port_desc_t* pOutput;
	uint32_t iPos;
	uint32_t i;
	uint32_t j;
	int iIndex;
	int iRet;

	if ( pIndex != NULL ) {
		*pIndex = -1;
	}
	if ( (pWorkflow == NULL) || (pDesc == NULL) || (pDesc->iSize < sizeof(*pDesc)) ||
	     (pDesc->sId == NULL) || (pDesc->sId[0] == 0) ) {
		return (XUI_ERROR_INVALID_ARGUMENT);
	}
	if ( xuiWorkflowFindNodeType(pWorkflow, pDesc->sId) >= 0 ) {
		return (XUI_ERROR_ALREADY_INITIALIZED);
	}
	iPos = xuiXrtArrayAppendSpace(&pWorkflow->arrTypes, 1u);
	if ( iPos == 0u ) {
		return (XUI_ERROR_OUT_OF_MEMORY);
	}
	iIndex = (int)iPos - 1;
	pType = (xui_workflow_type_model_t*)xuiXrtArrayGet(&pWorkflow->arrTypes, iPos);
	memset(pType, 0, sizeof(*pType));
	xuiXrtArrayInit(&pType->arrInputs, sizeof(xui_flow_port_desc_t));
	xuiXrtArrayInit(&pType->arrOutputs, sizeof(xui_flow_port_desc_t));
	pType->sId = __xuiWorkflowCopyString(pDesc->sId);
	pType->iVersion = pDesc->iVersion;
	pType->sTitle = __xuiWorkflowCopyString(pDesc->sTitle);
	pType->sCategory = __xuiWorkflowCopyString(pDesc->sCategory);
	pType->sDescription = __xuiWorkflowCopyString(pDesc->sDescription);
	pType->sIcon = __xuiWorkflowCopyString(pDesc->sIcon);
	pType->pConfigSchema = (pDesc->pConfigSchema != NULL) ? (shared ? __xuiWorkflowValueRef(pWorkflow,pDesc->pConfigSchema) : __xuiWorkflowValueClone(pWorkflow,pDesc->pConfigSchema)) : NULL;
	pType->onDynamicPorts = pDesc->onDynamicPorts;
	pType->pDynamicPortsUser = pDesc->pDynamicPortsUser;
	pType->onValidate = pDesc->onValidate;
	pType->pValidateUser = pDesc->pValidateUser;
	if ( pType->sId == NULL ||
	     (pDesc->sTitle != NULL && pType->sTitle == NULL) ||
	     (pDesc->sCategory != NULL && pType->sCategory == NULL) ||
	     (pDesc->sDescription != NULL && pType->sDescription == NULL) ||
	     (pDesc->sIcon != NULL && pType->sIcon == NULL) ||
	     (pDesc->pConfigSchema != NULL && pType->pConfigSchema == NULL) ) {
		__xuiWorkflowFreeType(pWorkflow, pType);
		xuiXrtArrayRemove(&pWorkflow->arrTypes, iPos, 1u);
		return (XUI_ERROR_OUT_OF_MEMORY);
	}
	iRet = __xuiWorkflowCopyPortArray(&pType->arrInputs, pDesc->pInputs, pDesc->iInputCount, XUI_FLOW_PORT_INPUT);
	if ( iRet == XUI_OK ) {
		iRet = __xuiWorkflowCopyPortArray(&pType->arrOutputs, pDesc->pOutputs, pDesc->iOutputCount, XUI_FLOW_PORT_OUTPUT);
	}
	if ( iRet == XUI_OK ) {
		for ( i = 1u; i <= pType->arrInputs.Count && iRet == XUI_OK; ++i ) {
			pInput = (xui_flow_port_desc_t*)xuiXrtArrayGet(&pType->arrInputs, i);
			for ( j = 1u; j <= pType->arrOutputs.Count; ++j ) {
				pOutput = (xui_flow_port_desc_t*)xuiXrtArrayGet(&pType->arrOutputs, j);
				if ( strcmp(pInput->sId, pOutput->sId) == 0 ) {
					iRet = XUI_ERROR_ALREADY_INITIALIZED;
					break;
				}
			}
		}
	}
	if ( iRet != XUI_OK ) {
		__xuiWorkflowFreeType(pWorkflow, pType);
		xuiXrtArrayRemove(&pWorkflow->arrTypes, iPos, 1u);
		return (iRet);
	}
	xrtownershipscope mutation = {0};
	if (callbacks && !xrtOwnershipMutationBegin(&mutation)) {
		iRet = XUI_ERROR_INVALID_STATE;
	} else {
		iRet = __xuiWorkflowDictSetIndex(&pWorkflow->mapTypes, pType->sId, iIndex);
		if (callbacks) {
			if (iRet == XUI_OK) pType->tCallbacks = *callbacks;
			if (!xrtOwnershipScopeEnd(&mutation)) abort();
		}
	}
	if ( iRet != XUI_OK ) {
		__xuiWorkflowFreeType(pWorkflow, pType);
		xuiXrtArrayRemove(&pWorkflow->arrTypes, iPos, 1u);
		return (iRet);
	}
	if ( pIndex != NULL ) {
		*pIndex = iIndex;
	}
	return (XUI_OK);
}

XUI_API int xuiWorkflowRegisterNodeType(xui_workflow pWorkflow, const xui_workflow_node_type_desc_t* pDesc, int* pIndex)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
    return __xuiWorkflowOperationEnd(&__ownedCall,__xuiWorkflowRegisterNodeTypeValue(pWorkflow,pDesc,pIndex,false,NULL),0);
}
XUI_API int xuiWorkflowRegisterNodeTypeShared(xui_workflow pWorkflow, const xui_workflow_node_type_desc_t* pDesc, int* pIndex)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
    return __xuiWorkflowOperationEnd(&__ownedCall,__xuiWorkflowRegisterNodeTypeValue(pWorkflow,pDesc,pIndex,true,NULL),0);
}

XUI_API int xuiWorkflowRegisterNodeTypeOwned(xui_workflow workflow, const xui_workflow_owned_node_type_desc_t* desc,
	const xui_callback_owner_t* owner, int* index)
{
	if (index) *index = -1;
	if (__xuiWorkflowModelBusy(workflow)) return XUI_ERROR_BUSY;
	if (!workflow || !workflow->pOwned || !desc || desc->iSize < sizeof(*desc) ||
	    !xuiInternalCallbackOwnerRawValid(owner) ||
	    (owner && owner->onHold && !desc->onDynamicPorts && !desc->onValidate))
		return XUI_ERROR_INVALID_ARGUMENT;
	xui_workflow_operation_t operation = {0};
	int status = __xuiWorkflowOperationBegin(workflow, &operation);
	if (status != XUI_OK) return status;
	xui_workflow_node_type_desc_t metadata = {
		.iSize = sizeof(metadata), .sId = desc->sId, .iVersion = desc->iVersion,
		.sTitle = desc->sTitle, .sCategory = desc->sCategory, .sDescription = desc->sDescription,
		.sIcon = desc->sIcon, .pInputs = desc->pInputs, .iInputCount = desc->iInputCount,
		.pOutputs = desc->pOutputs, .iOutputCount = desc->iOutputCount, .pConfigSchema = desc->pConfigSchema,
	};
	xui_workflow_type_callbacks_t callbacks = {
		.onPorts = desc->onDynamicPorts, .onValidate = desc->onValidate, .pUser = desc->pUser,
	};
	if (owner) callbacks.tOwner = *owner;
	bool held = false;
	xuiInternalFlowGraphValueCallbackEnter(workflow->pGraph);
	status = xuiInternalCallbackHoldFresh(owner, &held);
	xuiInternalFlowGraphValueCallbackLeave(workflow->pGraph);
	if (status == XUI_OK)
		status = __xuiWorkflowRegisterNodeTypeValue(workflow, &metadata, index, true, &callbacks);
	if (status != XUI_OK && held) {
		xerror* first = xrtTakeError();
		(void)__xuiWorkflowDropCallbackOwner(workflow, &callbacks.tOwner);
		xrtClearError();
		xrtSetErrorTake(first);
	}
	return __xuiWorkflowOperationEnd(&operation, status, 0);
}

XUI_API int xuiWorkflowFindNodeType(xui_workflow pWorkflow, const char* sId)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	if ( pWorkflow == NULL ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (-1), -1);
	}
	return __xuiWorkflowOperationEnd(&__ownedCall, (__xuiWorkflowDictFindIndex(&pWorkflow->mapTypes, sId)), -1);
}

XUI_API int xuiWorkflowGetNodeTypeCount(xui_workflow pWorkflow)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	return __xuiWorkflowOperationEnd(&__ownedCall, ((pWorkflow != NULL) ? (int)pWorkflow->arrTypes.Count : 0), 0);
}

static bool __xuiWorkflowHasDynamicPorts(const xui_workflow_type_model_t* type)
{
	return type->onDynamicPorts || type->tCallbacks.onPorts;
}

static void __xuiWorkflowFreeDynamicPorts(const xui_workflow_type_model_t* type, xui_workflow_port_result_t* result)
{
	if (type->tCallbacks.onPorts)
		for (int i = 0; i < result->iCount; ++i) __xuiWorkflowFreePortDesc(&result->pPorts[i]);
	xrtFree(result->pAllocation);
	*result = (xui_workflow_port_result_t){0};
}

static int __xuiWorkflowAddDynamicPorts(xui_workflow pWorkflow, const xui_workflow_type_model_t* pType, const char* sNodeId, int iNode, xvalue* pConfig)
{
	xui_workflow_port_result_t result = {0};
	int status = __xuiWorkflowBuildDynamicPorts(pWorkflow, pType, sNodeId, pConfig, &result);
	for (int i = 0; i < result.iCount && status == XUI_OK; ++i)
		status = xuiFlowGraphAddPort(pWorkflow->pGraph, iNode, &result.pPorts[i], NULL);
	__xuiWorkflowFreeDynamicPorts(pType, &result);
	return status;
}

static int __xuiWorkflowCallbackStatus(int status, int outputStatus)
{
	if (status != XUI_OK) return status < XUI_OK ? status : XUI_ERROR_CALLBACK_FAILED;
	if (outputStatus != XUI_OK) return outputStatus;
	return xrtGetError() ? XUI_ERROR_CALLBACK_FAILED : XUI_OK;
}

static int __xuiWorkflowInvokeValidator(xui_workflow workflow, const xui_workflow_type_model_t* type,
	const xui_flow_node_info_t* node, xvalue* config, int* diagnosticCount)
{
	xui_workflow_callback_input_t input = {{node->sId, node->sType}, config};
	xui_workflow_diagnostic_output_t output = {.sNodeId = node->sId};
	xrtArrayInit(&output.arrDiagnostics, sizeof(xui_flow_diagnostic_desc_t));
	xuiInternalFlowGraphValueCallbackEnter(workflow->pGraph);
	int status = type->tCallbacks.onValidate(&input.tNode, &output, type->tCallbacks.pUser);
	xuiInternalFlowGraphValueCallbackLeave(workflow->pGraph);
	status = __xuiWorkflowCallbackStatus(status, output.iStatus);
	xui_flow_diagnostic_desc_t* diagnostics = (xui_flow_diagnostic_desc_t*)output.arrDiagnostics.Data;
	size_t count = output.arrDiagnostics.Count;
	if (status == XUI_OK) {
		status = xuiInternalFlowGraphAppendDiagnosticsTake(workflow->pGraph, diagnostics, count);
		if (status == XUI_OK) {
			*diagnosticCount += (int)count;
			count = 0;
		}
	}
	for (size_t i = 0; i < count; ++i) __xuiWorkflowFreeDiagnosticDesc(&diagnostics[i]);
	xrtArrayUnit(&output.arrDiagnostics);
	return status;
}

static int __xuiWorkflowBuildDynamicPorts(xui_workflow pWorkflow, const xui_workflow_type_model_t* pType, const char* sNodeId, xvalue* pConfig,
	xui_workflow_port_result_t* result)
{
	xui_flow_port_desc_t* pPorts;
	int iCapacity;
	int iNextCapacity;
	int iPortCount;
	int i;
	int iRet;

	if (pType->tCallbacks.onPorts) {
		xui_workflow_callback_input_t input = {{sNodeId, pType->sId}, pConfig};
		xui_workflow_port_output_t output = {0};
		xrtArrayInit(&output.arrPorts, sizeof(xui_flow_port_desc_t));
		xuiInternalFlowGraphValueCallbackEnter(pWorkflow->pGraph);
		iRet = pType->tCallbacks.onPorts(&input.tNode, &output, pType->tCallbacks.pUser);
		xuiInternalFlowGraphValueCallbackLeave(pWorkflow->pGraph);
		iRet = __xuiWorkflowCallbackStatus(iRet, output.iStatus);
		*result = (xui_workflow_port_result_t){
			(xui_flow_port_desc_t*)output.arrPorts.Data, output.arrPorts.Allocation, (int)output.arrPorts.Count
		};
		if (iRet != XUI_OK) __xuiWorkflowFreeDynamicPorts(pType, result);
		return iRet;
	}
	if ( pType->onDynamicPorts == NULL ) {
		return XUI_OK;
	}
	iCapacity = 32;
	for ( ;; ) {
		pPorts = (xui_flow_port_desc_t*)xrtCalloc((size_t)iCapacity, sizeof(*pPorts));
		if ( pPorts == NULL ) {
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		iPortCount = 0;
		iRet = pType->onDynamicPorts(pWorkflow, sNodeId, pType->sId, pConfig,
			pPorts, iCapacity, &iPortCount, pType->pDynamicPortsUser);
		if ( iRet != XUI_OK ) {
			xrtFree(pPorts);
			return iRet;
		}
		if ( iPortCount < 0 ) {
			xrtFree(pPorts);
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		if ( iPortCount < iCapacity ) {
			break;
		}
		xrtFree(pPorts);
		if ( iPortCount > iCapacity ) {
			iNextCapacity = iPortCount;
		} else {
			if ( iCapacity > INT_MAX / 2 ) {
				return XUI_ERROR_LIMIT_EXCEEDED;
			}
			iNextCapacity = iCapacity * 2;
		}
		if ( iNextCapacity <= iCapacity ) {
			return XUI_ERROR_LIMIT_EXCEEDED;
		}
		iCapacity = iNextCapacity;
	}
	for ( i = 0; i < iPortCount; ++i ) {
		if ( pPorts[i].iSize < sizeof(pPorts[i]) ) {
			pPorts[i].iSize = sizeof(pPorts[i]);
		}
		pPorts[i].bDynamic = 1;
	}
	*result = (xui_workflow_port_result_t){pPorts, pPorts, iPortCount};
	return XUI_OK;
}

XUI_API int xuiWorkflowAddNode(xui_workflow pWorkflow, const char* sType, const char* sId, const char* sTitle, float fX, float fY, int* pIndex)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	xui_workflow_type_model_t* pType;
	xui_flow_node_desc_t tNode;
	xui_flow_port_desc_t* pPort;
	xvalue* pDefaultConfig;
	uint32_t i;
	int iType;
	int iNode;
	int iRet;

	if ( pIndex != NULL ) {
		*pIndex = -1;
	}
	if ( (pWorkflow == NULL) || (sType == NULL) || (sId == NULL) || (sId[0] == 0) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	iType = xuiWorkflowFindNodeType(pWorkflow, sType);
	if ( iType < 0 ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	pType = (xui_workflow_type_model_t*)xuiXrtArrayGet(&pWorkflow->arrTypes, (uint32_t)iType + 1u);
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	tNode.sId = sId;
	tNode.sType = sType;
	tNode.sTitle = (sTitle != NULL) ? sTitle : pType->sTitle;
	tNode.fX = fX;
	tNode.fY = fY;
	tNode.fW = 220.0f;
	tNode.fH = 120.0f;
	iRet = xuiFlowGraphAddNode(pWorkflow->pGraph, &tNode, &iNode);
	if ( iRet != XUI_OK ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
	}
	pDefaultConfig = NULL;
	for ( i = 1u; i <= pType->arrInputs.Count; ++i ) {
		pPort = (xui_flow_port_desc_t*)xuiXrtArrayGet(&pType->arrInputs, i);
		iRet = xuiFlowGraphAddPort(pWorkflow->pGraph, iNode, pPort, NULL);
		if ( iRet != XUI_OK ) {
			(void)xuiFlowGraphRemoveNode(pWorkflow->pGraph, sId);
			return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
		}
	}
	for ( i = 1u; i <= pType->arrOutputs.Count; ++i ) {
		pPort = (xui_flow_port_desc_t*)xuiXrtArrayGet(&pType->arrOutputs, i);
		iRet = xuiFlowGraphAddPort(pWorkflow->pGraph, iNode, pPort, NULL);
		if ( iRet != XUI_OK ) {
			(void)xuiFlowGraphRemoveNode(pWorkflow->pGraph, sId);
			return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
		}
	}
	iRet = xuiWorkflowCreateDefaultConfig(pWorkflow, sType, &pDefaultConfig);
	if ( iRet != XUI_OK ) {
		(void)xuiFlowGraphRemoveNode(pWorkflow->pGraph, sId);
		return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
	}
	iRet = __xuiWorkflowAddDynamicPorts(pWorkflow, pType, sId, iNode, pDefaultConfig);
	if ( iRet != XUI_OK ) {
		xerror* first = xrtTakeError();
		__xuiWorkflowValueRelease(pWorkflow, pDefaultConfig);
		(void)xuiFlowGraphRemoveNode(pWorkflow->pGraph, sId);
		xrtClearError();
		xrtSetErrorTake(first);
		return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
	}
	iRet = xuiFlowGraphSetNodeConfig(pWorkflow->pGraph, sId, pDefaultConfig);
	if ( iRet == XUI_OK ) {
		iRet = __xuiWorkflowRefreshNodeSummary(pWorkflow, sId, sType, pDefaultConfig);
	}
	__xuiWorkflowValueRelease(pWorkflow, pDefaultConfig);
	if ( iRet != XUI_OK ) {
		(void)xuiFlowGraphRemoveNode(pWorkflow->pGraph, sId);
		return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
	}
	if ( pIndex != NULL ) {
		*pIndex = iNode;
	}
	return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_OK), 0);
}

XUI_API int xuiWorkflowConnect(xui_workflow pWorkflow, const char* sEdgeId, const char* sFromNode, const char* sFromPort, const char* sToNode, const char* sToPort, int* pIndex)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	xui_flow_edge_desc_t tEdge;
	xui_flow_port_info_t tPort;
	int iNode;
	int i;
	int iRet;

	if ( pIndex != NULL ) {
		*pIndex = -1;
	}
	if ( (pWorkflow == NULL) || (sEdgeId == NULL) || (sEdgeId[0] == 0) ||
	     (sFromNode == NULL) || (sFromPort == NULL) || (sToNode == NULL) || (sToPort == NULL) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	iNode = xuiFlowGraphFindNode(pWorkflow->pGraph, sFromNode);
	if ( iNode < 0 ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	memset(&tPort, 0, sizeof(tPort));
	for ( i = 0; i < xuiFlowGraphGetNodePortCount(pWorkflow->pGraph, iNode); ++i ) {
		iRet = xuiFlowGraphGetPort(pWorkflow->pGraph, iNode, i, &tPort);
		if ( iRet != XUI_OK ) {
			return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
		}
		if ( tPort.sId != NULL && strcmp(tPort.sId, sFromPort) == 0 ) {
			break;
		}
	}
	if ( i >= xuiFlowGraphGetNodePortCount(pWorkflow->pGraph, iNode) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	memset(&tEdge, 0, sizeof(tEdge));
	tEdge.iSize = sizeof(tEdge);
	tEdge.sId = sEdgeId;
	tEdge.iKind = tPort.iKind;
	tEdge.sFromNode = sFromNode;
	tEdge.sFromPort = sFromPort;
	tEdge.sToNode = sToNode;
	tEdge.sToPort = sToPort;
	return __xuiWorkflowOperationEnd(&__ownedCall, (xuiFlowGraphAddEdge(pWorkflow->pGraph, &tEdge, pIndex)), 0);
}

XUI_API int xuiWorkflowRemoveNode(xui_workflow pWorkflow, const char* sId)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	if ( pWorkflow == NULL ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	return __xuiWorkflowOperationEnd(&__ownedCall, (xuiFlowGraphRemoveNode(pWorkflow->pGraph, sId)), 0);
}

XUI_API int xuiWorkflowSelectNode(xui_workflow pWorkflow, const char* sId, int bSelected)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	if ( pWorkflow == NULL ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	return __xuiWorkflowOperationEnd(&__ownedCall, (xuiFlowGraphSelectNode(pWorkflow->pGraph, sId, bSelected)), 0);
}

XUI_API int xuiWorkflowGetSelectedNode(xui_workflow pWorkflow, xui_flow_node_info_t* pInfo)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	xui_flow_node_info_t tInfo;
	int i;
	int iCount;
	int iRet;

	if ( (pWorkflow == NULL) || (pInfo == NULL) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	iCount = xuiFlowGraphGetNodeCount(pWorkflow->pGraph);
	for ( i = 0; i < iCount; ++i ) {
		iRet = xuiFlowGraphGetNode(pWorkflow->pGraph, i, &tInfo);
		if ( iRet != XUI_OK ) {
			return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
		}
		if ( xuiFlowGraphIsNodeSelected(pWorkflow->pGraph, tInfo.sId) ) {
			*pInfo = tInfo;
			return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_OK), 0);
		}
	}
	return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
}

XUI_API int xuiWorkflowGetNodeCount(xui_workflow pWorkflow)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	return __xuiWorkflowOperationEnd(&__ownedCall, ((pWorkflow != NULL) ? xuiFlowGraphGetNodeCount(pWorkflow->pGraph) : 0), 0);
}

XUI_API int xuiWorkflowGetNodeLibraryCount(xui_workflow pWorkflow)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	return __xuiWorkflowOperationEnd(&__ownedCall, ((pWorkflow != NULL) ? (int)pWorkflow->arrTypes.Count : 0), 0);
}

XUI_API int xuiWorkflowGetNodeLibraryItem(xui_workflow pWorkflow, int iIndex, xui_workflow_node_library_item_t* pItem)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	xui_workflow_type_model_t* pType;

	if ( (pWorkflow == NULL) || (pItem == NULL) || (iIndex < 0) || ((uint32_t)iIndex >= pWorkflow->arrTypes.Count) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	pType = (xui_workflow_type_model_t*)xuiXrtArrayGet(&pWorkflow->arrTypes, (uint32_t)iIndex + 1u);
	memset(pItem, 0, sizeof(*pItem));
	pItem->iSize = sizeof(*pItem);
	pItem->sId = pType->sId;
	pItem->iVersion = pType->iVersion;
	pItem->sTitle = pType->sTitle;
	pItem->sCategory = pType->sCategory;
	pItem->sDescription = pType->sDescription;
	pItem->sIcon = pType->sIcon;
	pItem->iInputCount = (int)pType->arrInputs.Count;
	pItem->iOutputCount = (int)pType->arrOutputs.Count;
	return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_OK), 0);
}

XUI_API int xuiWorkflowCreateDefaultConfig(xui_workflow pWorkflow, const char* sType, xvalue** ppConfig)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	xui_workflow_type_model_t* pType;
	xvalue* pFields;
	xvalue* pConfig;
	xvalue* pField;
	xvalue* pDefault;
	xvalue* pValue;
	const char* sId;
	uint32_t i;
	uint32_t iCount;
	int iKind;
	int iRet;

	if ( ppConfig == NULL ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	*ppConfig = NULL;
	pType = __xuiWorkflowFindTypeModel(pWorkflow, sType);
	if ( pType == NULL ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	pConfig = xrtValueObject();
	if ( pConfig == NULL ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_OUT_OF_MEMORY), 0);
	}
	pFields = __xuiWorkflowConfigFields(pType->pConfigSchema);
	if ( pFields == NULL ) {
		*ppConfig = pConfig;
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_OK), 0);
	}
	iCount = xrtValueCount(pFields);
	for ( i = 0u; i < iCount; ++i ) {
		pField = xuiXrtValueArrayGet(pFields, i);
		if ( xuiXrtValueType(pField) != XVALUE_OBJECT ) {
			continue;
		}
		sId = (const char*)xuiXrtValueObjectGetText(pField, "id", 2);
		iKind = (int)xuiXrtValueObjectGetInt(pField, "kind", 4);
		if ( (sId == NULL) || (sId[0] == 0) ) {
			continue;
		}
		pDefault = xuiXrtValueObjectGet(pField, "default", 7);
		if ( xuiXrtValueType(pDefault) != XVALUE_NULL ) {
			pValue = __xuiWorkflowValueClone(pWorkflow, pDefault);
			if ( pValue == NULL ) {
				__xuiWorkflowValueRelease(pWorkflow, pConfig);
				return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_OUT_OF_MEMORY), 0);
			}
		} else {
			iRet = __xuiWorkflowConfigDefaultForKind(iKind, &pValue);
			if ( iRet != XUI_OK ) {
				__xuiWorkflowValueRelease(pWorkflow, pConfig);
				return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
			}
		}
		if ( !__xuiWorkflowValueObjectSetTake(pWorkflow, pConfig, sId, strlen(sId), pValue) ) {
			__xuiWorkflowValueRelease(pWorkflow, pValue);
			__xuiWorkflowValueRelease(pWorkflow, pConfig);
			return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_OUT_OF_MEMORY), 0);
		}
	}
	*ppConfig = pConfig;
	return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_OK), 0);
}

XUI_API int xuiWorkflowValidateConfig(xui_workflow pWorkflow, const char* sType, xvalue* pConfig, int* pDiagnosticCount)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	return __xuiWorkflowOperationEnd(&__ownedCall, (xuiWorkflowValidateConfigEx(pWorkflow, sType, pConfig, NULL, 0, pDiagnosticCount)), 0);
}

XUI_API int xuiWorkflowValidateConfigEx(xui_workflow pWorkflow, const char* sType, xvalue* pConfig, xui_workflow_config_diagnostic_t* pDiagnostics, int iDiagnosticCapacity, int* pDiagnosticCount)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	xui_workflow_type_model_t* pType;
	xvalue* pFields;
	xvalue* pField;
	xvalue* pValue;
	const char* sId;
	int iDiagnostics;
	int iKind;
	int iExpectedType;
	int iValueType;
	int bRequired;
	double fValue;
	uint32_t i;
	uint32_t iCount;

	if ( (pDiagnosticCount == NULL) || (iDiagnosticCapacity < 0) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	*pDiagnosticCount = 0;
	pType = __xuiWorkflowFindTypeModel(pWorkflow, sType);
	if ( pType == NULL || (pConfig != NULL && xuiXrtValueType(pConfig) != XVALUE_OBJECT) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	pFields = __xuiWorkflowConfigFields(pType->pConfigSchema);
	if ( pFields == NULL ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_OK), 0);
	}
	iDiagnostics = 0;
	iCount = xrtValueCount(pFields);
	for ( i = 0u; i < iCount; ++i ) {
		pField = xuiXrtValueArrayGet(pFields, i);
		if ( xuiXrtValueType(pField) != XVALUE_OBJECT ) {
			continue;
		}
		sId = (const char*)xuiXrtValueObjectGetText(pField, "id", 2);
		iKind = (int)xuiXrtValueObjectGetInt(pField, "kind", 4);
		bRequired = xuiXrtValueObjectGetBool(pField, "required", 8) ? 1 : 0;
		if ( (sId == NULL) || (sId[0] == 0) ) {
			continue;
		}
		pValue = (pConfig != NULL) ? xuiXrtValueObjectGet(pConfig, sId, (uint32_t)strlen(sId)) : NULL;
		iValueType = xuiXrtValueType(pValue);
		if ( bRequired && (iValueType == XVALUE_NULL || ((iValueType == XVALUE_STRING) && xuiXrtValueSize(pValue) == 0u)) ) {
			__xuiWorkflowConfigAddDiagnostic(pDiagnostics, iDiagnosticCapacity, &iDiagnostics, sId, "required", "Required config field is missing.");
			continue;
		}
		if ( iValueType == XVALUE_NULL ) {
			continue;
		}
		iExpectedType = __xuiWorkflowConfigKindToXvoType(iKind);
		if ( iExpectedType == XVALUE_FLOAT ) {
			if ( (iValueType != XVALUE_FLOAT) && (iValueType != XVALUE_INT) ) {
				__xuiWorkflowConfigAddDiagnostic(pDiagnostics, iDiagnosticCapacity, &iDiagnostics, sId, "type", "Config field has the wrong value type.");
				continue;
			}
		} else if ( iValueType != iExpectedType ) {
			__xuiWorkflowConfigAddDiagnostic(pDiagnostics, iDiagnosticCapacity, &iDiagnostics, sId, "type", "Config field has the wrong value type.");
			continue;
		}
		if ( (iKind == XUI_WORKFLOW_CONFIG_FIELD_INT) || (iKind == XUI_WORKFLOW_CONFIG_FIELD_FLOAT) ) {
			fValue = (iValueType == XVALUE_INT) ? (double)xuiXrtValueGetInt(pValue) : xuiXrtValueGetFloat(pValue);
			if ( !isfinite(fValue) ) {
				__xuiWorkflowConfigAddDiagnostic(pDiagnostics, iDiagnosticCapacity, &iDiagnostics, sId, "number.finite", "Config field must be a finite number.");
				continue;
			}
			if ( xuiXrtValueType(xuiXrtValueObjectGet(pField, "min", 3)) != XVALUE_NULL && fValue < xuiXrtValueObjectGetFloat(pField, "min", 3) ) {
				__xuiWorkflowConfigAddDiagnostic(pDiagnostics, iDiagnosticCapacity, &iDiagnostics, sId, "range.min", "Config field is below minimum.");
			}
			if ( xuiXrtValueType(xuiXrtValueObjectGet(pField, "max", 3)) != XVALUE_NULL && fValue > xuiXrtValueObjectGetFloat(pField, "max", 3) ) {
				__xuiWorkflowConfigAddDiagnostic(pDiagnostics, iDiagnosticCapacity, &iDiagnostics, sId, "range.max", "Config field is above maximum.");
			}
		}
	}
	*pDiagnosticCount = iDiagnostics;
	return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_OK), 0);
}

static int __xuiWorkflowAnalyzeControlGraph(xui_workflow pWorkflow, int* pReachable, int* pCycleEdge)
{
	xui_flow_node_info_t tNode;
	xui_flow_edge_info_t tEdge;
	int* pHeads;
	int* pNext;
	int* pTo;
	int* pEdgeIndices;
	int* pInDegree;
	int* pQueue;
	int iNodeCount;
	int iEdgeCount;
	int iControlEdgeCount;
	int iQueueHead;
	int iQueueTail;
	int iVisited;
	int i;
	int iEntry;
	int iRet;

	if ( (pWorkflow == NULL) || (pReachable == NULL) || (pCycleEdge == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pCycleEdge = -1;
	iNodeCount = xuiFlowGraphGetNodeCount(pWorkflow->pGraph);
	iEdgeCount = xuiFlowGraphGetEdgeCount(pWorkflow->pGraph);
	memset(pReachable, 0, (size_t)iNodeCount * sizeof(*pReachable));
	if ( iNodeCount == 0 ) {
		return XUI_OK;
	}
	pHeads = (int*)xrtMalloc((uint32_t)iNodeCount * sizeof(*pHeads));
	pInDegree = (int*)xrtCalloc((uint32_t)iNodeCount, sizeof(*pInDegree));
	pQueue = (int*)xrtMalloc((uint32_t)iNodeCount * sizeof(*pQueue));
	pNext = (iEdgeCount > 0) ? (int*)xrtMalloc((uint32_t)iEdgeCount * sizeof(*pNext)) : NULL;
	pTo = (iEdgeCount > 0) ? (int*)xrtMalloc((uint32_t)iEdgeCount * sizeof(*pTo)) : NULL;
	pEdgeIndices = (iEdgeCount > 0) ? (int*)xrtMalloc((uint32_t)iEdgeCount * sizeof(*pEdgeIndices)) : NULL;
	if ( pHeads == NULL || pInDegree == NULL || pQueue == NULL ||
	     (iEdgeCount > 0 && (pNext == NULL || pTo == NULL || pEdgeIndices == NULL)) ) {
		iRet = XUI_ERROR_OUT_OF_MEMORY;
		goto cleanup;
	}
	for ( i = 0; i < iNodeCount; ++i ) {
		pHeads[i] = -1;
	}
	iControlEdgeCount = 0;
	for ( i = 0; i < iEdgeCount; ++i ) {
		iRet = xuiFlowGraphGetEdge(pWorkflow->pGraph, i, &tEdge);
		if ( iRet != XUI_OK ) goto cleanup;
		if ( tEdge.bInvalid || tEdge.iKind != XUI_FLOW_PORT_CONTROL ||
		     tEdge.iFromNode < 0 || tEdge.iFromNode >= iNodeCount || tEdge.iToNode < 0 || tEdge.iToNode >= iNodeCount ) {
			continue;
		}
		pNext[iControlEdgeCount] = pHeads[tEdge.iFromNode];
		pTo[iControlEdgeCount] = tEdge.iToNode;
		pEdgeIndices[iControlEdgeCount] = i;
		pHeads[tEdge.iFromNode] = iControlEdgeCount;
		pInDegree[tEdge.iToNode]++;
		iControlEdgeCount++;
	}
	iQueueHead = 0;
	iQueueTail = 0;
	for ( i = 0; i < iNodeCount; ++i ) {
		if ( pInDegree[i] == 0 ) pQueue[iQueueTail++] = i;
	}
	iVisited = 0;
	while ( iQueueHead < iQueueTail ) {
		i = pQueue[iQueueHead++];
		iVisited++;
		for ( iEntry = pHeads[i]; iEntry >= 0; iEntry = pNext[iEntry] ) {
			if ( --pInDegree[pTo[iEntry]] == 0 ) pQueue[iQueueTail++] = pTo[iEntry];
		}
	}
	if ( iVisited < iNodeCount ) {
		for ( i = 0; i < iControlEdgeCount; ++i ) {
			iRet = xuiFlowGraphGetEdge(pWorkflow->pGraph, pEdgeIndices[i], &tEdge);
			if ( iRet != XUI_OK ) goto cleanup;
			if ( pInDegree[tEdge.iFromNode] > 0 && pInDegree[tEdge.iToNode] > 0 ) {
				*pCycleEdge = pEdgeIndices[i];
				break;
			}
		}
	}
	iQueueHead = 0;
	iQueueTail = 0;
	for ( i = 0; i < iNodeCount; ++i ) {
		iRet = xuiFlowGraphGetNode(pWorkflow->pGraph, i, &tNode);
		if ( iRet != XUI_OK ) goto cleanup;
		if ( tNode.sType != NULL && strcmp(tNode.sType, "start") == 0 ) {
			pReachable[i] = 1;
			pQueue[iQueueTail++] = i;
		}
	}
	while ( iQueueHead < iQueueTail ) {
		i = pQueue[iQueueHead++];
		for ( iEntry = pHeads[i]; iEntry >= 0; iEntry = pNext[iEntry] ) {
			if ( !pReachable[pTo[iEntry]] ) {
				pReachable[pTo[iEntry]] = 1;
				pQueue[iQueueTail++] = pTo[iEntry];
			}
		}
	}
	iRet = XUI_OK;

cleanup:
	xrtFree(pEdgeIndices);
	xrtFree(pTo);
	xrtFree(pNext);
	xrtFree(pQueue);
	xrtFree(pInDegree);
	xrtFree(pHeads);
	return iRet;
}

static int __xuiWorkflowValidatePortConnections(xui_workflow pWorkflow, int* pDiagnosticCount)
{
	xui_flow_diagnostic_desc_t tDiagnostic;
	xui_flow_edge_info_t tEdge;
	xui_flow_node_info_t tNode;
	xui_flow_port_info_t tFromPort;
	xui_flow_port_info_t tToPort;
	uint32_t* pConnectionCounts;
	int* pPortOffsets;
	int iNodeCount;
	int iEdgeCount;
	int iPortCount;
	int iTotalPorts;
	int i;
	int j;
	int iRet;

	if ( (pWorkflow == NULL) || (pDiagnosticCount == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iNodeCount = xuiFlowGraphGetNodeCount(pWorkflow->pGraph);
	pPortOffsets = (int*)xrtMalloc((size_t)(iNodeCount + 1) * sizeof(*pPortOffsets));
	if ( pPortOffsets == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pPortOffsets[0] = 0;
	for ( i = 0; i < iNodeCount; ++i ) {
		iPortCount = xuiFlowGraphGetNodePortCount(pWorkflow->pGraph, i);
		if ( iPortCount > INT_MAX - pPortOffsets[i] ) {
			xrtFree(pPortOffsets);
			return XUI_ERROR_LIMIT_EXCEEDED;
		}
		pPortOffsets[i + 1] = pPortOffsets[i] + iPortCount;
	}
	iTotalPorts = pPortOffsets[iNodeCount];
	pConnectionCounts = (iTotalPorts > 0) ? (uint32_t*)xrtCalloc((size_t)iTotalPorts, sizeof(*pConnectionCounts)) : NULL;
	if ( iTotalPorts > 0 && pConnectionCounts == NULL ) {
		xrtFree(pPortOffsets);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iEdgeCount = xuiFlowGraphGetEdgeCount(pWorkflow->pGraph);
	for ( i = 0; i < iEdgeCount; ++i ) {
		iRet = xuiFlowGraphGetEdge(pWorkflow->pGraph, i, &tEdge);
		if ( iRet != XUI_OK ) goto cleanup;
		if ( tEdge.bInvalid || tEdge.iFromNode < 0 || tEdge.iFromNode >= iNodeCount ||
		     tEdge.iToNode < 0 || tEdge.iToNode >= iNodeCount ||
		     tEdge.iFromPort < 0 || tEdge.iFromPort >= pPortOffsets[tEdge.iFromNode + 1] - pPortOffsets[tEdge.iFromNode] ||
		     tEdge.iToPort < 0 || tEdge.iToPort >= pPortOffsets[tEdge.iToNode + 1] - pPortOffsets[tEdge.iToNode] ) {
			continue;
		}
		iRet = xuiFlowGraphGetPort(pWorkflow->pGraph, tEdge.iFromNode, tEdge.iFromPort, &tFromPort);
		if ( iRet != XUI_OK ) goto cleanup;
		iRet = xuiFlowGraphGetPort(pWorkflow->pGraph, tEdge.iToNode, tEdge.iToPort, &tToPort);
		if ( iRet != XUI_OK ) goto cleanup;
		if ( tFromPort.iDirection == XUI_FLOW_PORT_OUTPUT && tToPort.iDirection == XUI_FLOW_PORT_INPUT &&
		     tFromPort.iKind == tEdge.iKind && tToPort.iKind == tEdge.iKind ) {
			pConnectionCounts[pPortOffsets[tEdge.iToNode] + tEdge.iToPort]++;
		}
	}
	for ( i = 0; i < iNodeCount; ++i ) {
		iRet = xuiFlowGraphGetNode(pWorkflow->pGraph, i, &tNode);
		if ( iRet != XUI_OK ) goto cleanup;
		iPortCount = pPortOffsets[i + 1] - pPortOffsets[i];
		for ( j = 0; j < iPortCount; ++j ) {
			iRet = xuiFlowGraphGetPort(pWorkflow->pGraph, i, j, &tToPort);
			if ( iRet != XUI_OK ) goto cleanup;
			if ( tToPort.iDirection != XUI_FLOW_PORT_INPUT ) {
				continue;
			}
			if ( tToPort.bRequired && pConnectionCounts[pPortOffsets[i] + j] == 0u ) {
				memset(&tDiagnostic, 0, sizeof(tDiagnostic));
				tDiagnostic.iSize = sizeof(tDiagnostic);
				tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
				tDiagnostic.sCode = "workflow.port.required";
				tDiagnostic.sMessage = "Required workflow input port is not connected.";
				tDiagnostic.sNode = tNode.sId;
				tDiagnostic.sPath = tToPort.sId;
				iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &tDiagnostic, NULL);
				if ( iRet != XUI_OK ) goto cleanup;
				(*pDiagnosticCount)++;
			}
			if ( !tToPort.bMulti && pConnectionCounts[pPortOffsets[i] + j] > 1u ) {
				memset(&tDiagnostic, 0, sizeof(tDiagnostic));
				tDiagnostic.iSize = sizeof(tDiagnostic);
				tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
				tDiagnostic.sCode = "workflow.port.multiple";
				tDiagnostic.sMessage = "Workflow input port does not accept multiple connections.";
				tDiagnostic.sNode = tNode.sId;
				tDiagnostic.sPath = tToPort.sId;
				iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &tDiagnostic, NULL);
				if ( iRet != XUI_OK ) goto cleanup;
				(*pDiagnosticCount)++;
			}
		}
	}
	iRet = XUI_OK;

cleanup:
	xrtFree(pConnectionCounts);
	xrtFree(pPortOffsets);
	return iRet;
}

static int __xuiWorkflowExpressionReferencesClosed(const char* sExpression)
{
	const char* sOpen;
	const char* sClose;

	if ( sExpression == NULL ) return 1;
	sOpen = sExpression;
	while ( (sOpen = strstr(sOpen, "${")) != NULL ) {
		sClose = strchr(sOpen + 2, '}');
		if ( sClose == NULL ) return 0;
		sOpen = sClose + 1;
	}
	return 1;
}

XUI_API int xuiWorkflowValidateGraph(xui_workflow pWorkflow, int* pDiagnosticCount)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	xui_flow_edge_info_t tEdge;
	xui_flow_node_info_t tNode;
	xui_flow_port_info_t tFromPort;
	xui_flow_port_info_t tToPort;
	xui_flow_diagnostic_desc_t tDiagnostic;
	xui_workflow_type_model_t* pType;
	xui_workflow_config_diagnostic_t arrConfigDiagnostics[16];
	xui_workflow_config_diagnostic_t* pConfigDiagnostics;
	xui_flow_diagnostic_desc_t arrHostDiagnostics[16];
	xvalue* pConfig;
	xvalue* pFields;
	xvalue* pField;
	xvalue* pValue;
	const char* sField;
	const char* sRef;
	int* pReachable;
	int i;
	int j;
	int iCount;
	int iNodeCount;
	int iDiagnostics;
	int iConfigDiagnostics;
	int iHostDiagnostics;
	int iStartCount;
	int iEndCount;
	int iCycleEdge;
	int iKind;
	int iRet;

	if ( (pWorkflow == NULL) || (pDiagnosticCount == NULL) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	*pDiagnosticCount = 0;
	iRet = xuiFlowGraphClearDiagnostics(pWorkflow->pGraph);
	if ( iRet != XUI_OK ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
	}
	iDiagnostics = 0;
	iStartCount = 0;
	iEndCount = 0;
	pReachable = NULL;
	pConfig = NULL;
	iNodeCount = xuiFlowGraphGetNodeCount(pWorkflow->pGraph);
	for ( i = 0; i < iNodeCount; ++i ) {
		iRet = xuiFlowGraphGetNode(pWorkflow->pGraph, i, &tNode);
		if ( iRet != XUI_OK ) {
			return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
		}
		if ( tNode.sType == NULL || xuiWorkflowFindNodeType(pWorkflow, tNode.sType) < 0 ) {
			memset(&tDiagnostic, 0, sizeof(tDiagnostic));
			tDiagnostic.iSize = sizeof(tDiagnostic);
			tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
			tDiagnostic.sCode = "workflow.node.missing_type";
			tDiagnostic.sMessage = "Workflow node references an unregistered node type.";
			tDiagnostic.sNode = tNode.sId;
			tDiagnostic.sPath = "nodes";
			iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &tDiagnostic, NULL);
			if ( iRet != XUI_OK ) {
				if ( pConfig != NULL ) __xuiWorkflowValueRelease(pWorkflow, pConfig);
				return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
			}
			iDiagnostics++;
			continue;
		}
		pType = __xuiWorkflowFindTypeModel(pWorkflow, tNode.sType);
		pConfig = NULL;
		iRet = xuiFlowGraphGetNodeConfig(pWorkflow->pGraph, tNode.sId, &pConfig);
		if ( iRet != XUI_OK ) {
			return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
		}
		pConfigDiagnostics = arrConfigDiagnostics;
		iRet = xuiWorkflowValidateConfigEx(pWorkflow, tNode.sType, pConfig, pConfigDiagnostics, (int)(sizeof(arrConfigDiagnostics) / sizeof(arrConfigDiagnostics[0])), &iConfigDiagnostics);
		if ( iRet != XUI_OK ) {
			if ( pConfig != NULL ) __xuiWorkflowValueRelease(pWorkflow, pConfig);
			return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
		}
		if ( iConfigDiagnostics > (int)(sizeof(arrConfigDiagnostics) / sizeof(arrConfigDiagnostics[0])) ) {
			pConfigDiagnostics = (xui_workflow_config_diagnostic_t*)xrtMalloc((size_t)iConfigDiagnostics * sizeof(*pConfigDiagnostics));
			if ( pConfigDiagnostics == NULL ) {
				if ( pConfig != NULL ) __xuiWorkflowValueRelease(pWorkflow, pConfig);
				return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_OUT_OF_MEMORY), 0);
			}
			iRet = xuiWorkflowValidateConfigEx(pWorkflow, tNode.sType, pConfig, pConfigDiagnostics, iConfigDiagnostics, &iConfigDiagnostics);
			if ( iRet != XUI_OK ) {
				xrtFree(pConfigDiagnostics);
				if ( pConfig != NULL ) __xuiWorkflowValueRelease(pWorkflow, pConfig);
				return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
			}
		}
		for ( j = 0; j < iConfigDiagnostics; ++j ) {
			memset(&tDiagnostic, 0, sizeof(tDiagnostic));
			tDiagnostic.iSize = sizeof(tDiagnostic);
			tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
			tDiagnostic.sCode = pConfigDiagnostics[j].sCode;
			tDiagnostic.sMessage = pConfigDiagnostics[j].sMessage;
			tDiagnostic.sNode = tNode.sId;
			tDiagnostic.sPath = pConfigDiagnostics[j].sPath;
			iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &tDiagnostic, NULL);
			if ( iRet != XUI_OK ) {
				if ( pConfigDiagnostics != arrConfigDiagnostics ) xrtFree(pConfigDiagnostics);
				if ( pConfig != NULL ) __xuiWorkflowValueRelease(pWorkflow, pConfig);
				return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
			}
			iDiagnostics++;
		}
		if ( pConfigDiagnostics != arrConfigDiagnostics ) xrtFree(pConfigDiagnostics);
		pFields = (pType != NULL) ? __xuiWorkflowConfigFields(pType->pConfigSchema) : NULL;
		if ( pFields != NULL && pConfig != NULL && xuiXrtValueType(pConfig) == XVALUE_OBJECT ) {
			iCount = xrtValueCount(pFields);
			for ( j = 0; j < iCount; ++j ) {
				pField = xuiXrtValueArrayGet(pFields, (uint32_t)j);
				if ( xuiXrtValueType(pField) != XVALUE_OBJECT ) {
					continue;
				}
				sField = (const char*)xuiXrtValueObjectGetText(pField, "id", 2);
				iKind = (int)xuiXrtValueObjectGetInt(pField, "kind", 4);
				if ( sField == NULL || (iKind != XUI_WORKFLOW_CONFIG_FIELD_VARIABLE_REF && iKind != XUI_WORKFLOW_CONFIG_FIELD_EXPRESSION) ) {
					continue;
				}
				pValue = xuiXrtValueObjectGet(pConfig, sField, (uint32_t)strlen(sField));
				if ( xuiXrtValueType(pValue) != XVALUE_STRING || xuiXrtValueSize(pValue) == 0u ) {
					continue;
				}
				sRef = (const char*)xuiXrtValueGetText(pValue);
				if ( iKind == XUI_WORKFLOW_CONFIG_FIELD_EXPRESSION ) {
					if ( strstr(sRef, "${") == NULL || __xuiWorkflowExpressionReferencesClosed(sRef) ) {
						continue;
					}
					memset(&tDiagnostic, 0, sizeof(tDiagnostic));
					tDiagnostic.iSize = sizeof(tDiagnostic);
					tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
					tDiagnostic.sCode = "workflow.config.expression";
					tDiagnostic.sMessage = "Workflow config expression reference is not closed.";
					tDiagnostic.sNode = tNode.sId;
					tDiagnostic.sPath = sField;
					iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &tDiagnostic, NULL);
					if ( iRet != XUI_OK ) {
						if ( pConfig != NULL ) __xuiWorkflowValueRelease(pWorkflow, pConfig);
						return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
					}
					iDiagnostics++;
					continue;
				}
				if ( xuiWorkflowFindVariable(pWorkflow, sRef) >= 0 ) {
					continue;
				}
				memset(&tDiagnostic, 0, sizeof(tDiagnostic));
				tDiagnostic.iSize = sizeof(tDiagnostic);
				tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
				tDiagnostic.sCode = "workflow.config.variable_ref";
				tDiagnostic.sMessage = "Workflow config references a missing variable.";
				tDiagnostic.sNode = tNode.sId;
				tDiagnostic.sPath = sField;
				iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &tDiagnostic, NULL);
				if ( iRet != XUI_OK ) {
					if ( pConfig != NULL ) __xuiWorkflowValueRelease(pWorkflow, pConfig);
					return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
				}
				iDiagnostics++;
			}
		}
		if (pType && pType->tCallbacks.onValidate) {
			iRet = __xuiWorkflowInvokeValidator(pWorkflow, pType, &tNode, pConfig, &iDiagnostics);
			if (iRet != XUI_OK) {
				__xuiWorkflowValueRelease(pWorkflow, pConfig);
				return __xuiWorkflowOperationEnd(&__ownedCall, iRet, 0);
			}
		} else if ( pType != NULL && pType->onValidate != NULL ) {
			memset(arrHostDiagnostics, 0, sizeof(arrHostDiagnostics));
			iHostDiagnostics = 0;
			iRet = pType->onValidate(pWorkflow, tNode.sId, tNode.sType, pConfig, arrHostDiagnostics, (int)(sizeof(arrHostDiagnostics) / sizeof(arrHostDiagnostics[0])), &iHostDiagnostics, pType->pValidateUser);
			if ( iRet != XUI_OK ) {
				if ( pConfig != NULL ) __xuiWorkflowValueRelease(pWorkflow, pConfig);
				return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
			}
			if ( iHostDiagnostics < 0 || iHostDiagnostics > (int)(sizeof(arrHostDiagnostics) / sizeof(arrHostDiagnostics[0])) ) {
				if ( pConfig != NULL ) __xuiWorkflowValueRelease(pWorkflow, pConfig);
				return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
			}
			for ( j = 0; j < iHostDiagnostics && j < (int)(sizeof(arrHostDiagnostics) / sizeof(arrHostDiagnostics[0])); ++j ) {
				if ( arrHostDiagnostics[j].iSize < sizeof(arrHostDiagnostics[j]) ) {
					arrHostDiagnostics[j].iSize = sizeof(arrHostDiagnostics[j]);
				}
				if ( arrHostDiagnostics[j].sNode == NULL ) {
					arrHostDiagnostics[j].sNode = tNode.sId;
				}
				iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &arrHostDiagnostics[j], NULL);
				if ( iRet != XUI_OK ) {
					if ( pConfig != NULL ) __xuiWorkflowValueRelease(pWorkflow, pConfig);
					return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
				}
				iDiagnostics++;
			}
		}
		if ( pConfig != NULL ) {
			__xuiWorkflowValueRelease(pWorkflow, pConfig);
			pConfig = NULL;
		}
		if ( tNode.sType != NULL && strcmp(tNode.sType, "start") == 0 ) {
			iStartCount++;
		}
		if ( tNode.sType != NULL && strcmp(tNode.sType, "end") == 0 ) {
			iEndCount++;
		}
	}
	if ( iStartCount == 0 ) {
		memset(&tDiagnostic, 0, sizeof(tDiagnostic));
		tDiagnostic.iSize = sizeof(tDiagnostic);
		tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
		tDiagnostic.sCode = "workflow.start.missing";
		tDiagnostic.sMessage = "Workflow requires at least one Start node.";
		tDiagnostic.sPath = "nodes";
		iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &tDiagnostic, NULL);
		if ( iRet != XUI_OK ) return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
		iDiagnostics++;
	}
	if ( iEndCount == 0 ) {
		memset(&tDiagnostic, 0, sizeof(tDiagnostic));
		tDiagnostic.iSize = sizeof(tDiagnostic);
		tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
		tDiagnostic.sCode = "workflow.end.missing";
		tDiagnostic.sMessage = "Workflow requires at least one End node.";
		tDiagnostic.sPath = "nodes";
		iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &tDiagnostic, NULL);
		if ( iRet != XUI_OK ) return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
		iDiagnostics++;
	}
	iCount = xuiFlowGraphGetEdgeCount(pWorkflow->pGraph);
	for ( i = 0; i < iCount; ++i ) {
		iRet = xuiFlowGraphGetEdge(pWorkflow->pGraph, i, &tEdge);
		if ( iRet != XUI_OK ) {
			return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
		}
		if ( tEdge.iFromNode >= 0 && tEdge.iFromPort >= 0 && tEdge.iToNode >= 0 && tEdge.iToPort >= 0 ) {
			iRet = xuiFlowGraphGetPort(pWorkflow->pGraph, tEdge.iFromNode, tEdge.iFromPort, &tFromPort);
			if ( iRet != XUI_OK ) return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
			iRet = xuiFlowGraphGetPort(pWorkflow->pGraph, tEdge.iToNode, tEdge.iToPort, &tToPort);
			if ( iRet != XUI_OK ) return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
			if ( tFromPort.iDirection != XUI_FLOW_PORT_OUTPUT || tToPort.iDirection != XUI_FLOW_PORT_INPUT ) {
				memset(&tDiagnostic, 0, sizeof(tDiagnostic));
				tDiagnostic.iSize = sizeof(tDiagnostic);
				tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
				tDiagnostic.sCode = "workflow.edge.direction";
				tDiagnostic.sMessage = "Workflow edge must connect an output port to an input port.";
				tDiagnostic.sEdge = tEdge.sId;
				tDiagnostic.sPath = "edges";
				iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &tDiagnostic, NULL);
				if ( iRet != XUI_OK ) return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
				iDiagnostics++;
				continue;
			}
			if ( tFromPort.iKind != tEdge.iKind || tToPort.iKind != tEdge.iKind ) {
				memset(&tDiagnostic, 0, sizeof(tDiagnostic));
				tDiagnostic.iSize = sizeof(tDiagnostic);
				tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
				tDiagnostic.sCode = "workflow.edge.kind";
				tDiagnostic.sMessage = "Workflow edge kind must match both connected ports.";
				tDiagnostic.sEdge = tEdge.sId;
				tDiagnostic.sPath = "edges";
				iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &tDiagnostic, NULL);
				if ( iRet != XUI_OK ) return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
				iDiagnostics++;
				continue;
			}
			if ( !tEdge.bInvalid ) continue;
		}
		memset(&tDiagnostic, 0, sizeof(tDiagnostic));
		tDiagnostic.iSize = sizeof(tDiagnostic);
		tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
		tDiagnostic.sCode = "workflow.edge.invalid_ref";
		tDiagnostic.sMessage = "Workflow edge references a missing node or port.";
		tDiagnostic.sEdge = tEdge.sId;
		tDiagnostic.sNode = (tEdge.iFromNode < 0) ? tEdge.sFromNode : ((tEdge.iToNode < 0) ? tEdge.sToNode : NULL);
		tDiagnostic.sPath = "edges";
		iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &tDiagnostic, NULL);
		if ( iRet != XUI_OK ) {
			return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
		}
		iDiagnostics++;
	}
	iRet = __xuiWorkflowValidatePortConnections(pWorkflow, &iDiagnostics);
	if ( iRet != XUI_OK ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
	}
	if ( iNodeCount > 0 ) {
		pReachable = (int*)xrtCalloc((uint32_t)iNodeCount, sizeof(int));
		if ( pReachable == NULL ) {
			return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_OUT_OF_MEMORY), 0);
		}
		iRet = __xuiWorkflowAnalyzeControlGraph(pWorkflow, pReachable, &iCycleEdge);
		if ( iRet != XUI_OK ) {
			xrtFree(pReachable);
			return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
		}
		if ( iCycleEdge >= 0 ) {
			iRet = xuiFlowGraphGetEdge(pWorkflow->pGraph, iCycleEdge, &tEdge);
			if ( iRet == XUI_OK ) {
				memset(&tDiagnostic, 0, sizeof(tDiagnostic));
				tDiagnostic.iSize = sizeof(tDiagnostic);
				tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
				tDiagnostic.sCode = "workflow.control.cycle";
				tDiagnostic.sMessage = "Workflow control flow contains a cycle.";
				tDiagnostic.sEdge = tEdge.sId;
				tDiagnostic.sPath = "edges";
				iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &tDiagnostic, NULL);
			}
			if ( iRet != XUI_OK ) {
				xrtFree(pReachable);
				return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
			}
			iDiagnostics++;
		}
		if ( iStartCount > 0 ) {
			for ( i = 0; i < iNodeCount; ++i ) {
				if ( pReachable[i] ) continue;
				iRet = xuiFlowGraphGetNode(pWorkflow->pGraph, i, &tNode);
				if ( iRet != XUI_OK ) {
					xrtFree(pReachable);
					return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
				}
				memset(&tDiagnostic, 0, sizeof(tDiagnostic));
				tDiagnostic.iSize = sizeof(tDiagnostic);
				tDiagnostic.iSeverity = XUI_CODE_DIAGNOSTIC_ERROR;
				tDiagnostic.sCode = "workflow.node.unreachable";
				tDiagnostic.sMessage = "Workflow node is not reachable from a Start node.";
				tDiagnostic.sNode = tNode.sId;
				tDiagnostic.sPath = "nodes";
				iRet = xuiFlowGraphAddDiagnostic(pWorkflow->pGraph, &tDiagnostic, NULL);
				if ( iRet != XUI_OK ) {
					xrtFree(pReachable);
					return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
				}
				iDiagnostics++;
			}
		}
		xrtFree(pReachable);
	}
	*pDiagnosticCount = iDiagnostics;
	return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_OK), 0);
}

static int __xuiWorkflowSetNodeConfigValue(xui_workflow pWorkflow,const char* sId,xvalue* pConfig,bool shared)
{
	xui_flow_node_info_t tInfo;
	xui_workflow_type_model_t* pType;
	xui_workflow_port_result_t ports = {0};
	char sSummary[256];
	int iDiagnostics;
	int iRet;

	iRet = __xuiWorkflowGetNodeType(pWorkflow, sId, &tInfo);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = xuiWorkflowValidateConfig(pWorkflow, tInfo.sType, pConfig, &iDiagnostics);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	if ( iDiagnostics > 0 ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = __xuiWorkflowFindTypeModel(pWorkflow, tInfo.sType);
	if ( pType == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiWorkflowBuildDynamicPorts(pWorkflow, pType, sId, pConfig, &ports);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	iRet = __xuiWorkflowBuildNodeSummary(pWorkflow, tInfo.sType, pConfig,
		sSummary, (int)sizeof(sSummary));
	if ( iRet == XUI_OK ) {
        iRet = shared ? xuiInternalFlowGraphUpdateNodeConfigurationShared(pWorkflow->pGraph,sId,
            pConfig,ports.pPorts,ports.iCount,__xuiWorkflowHasDynamicPorts(pType),(sSummary[0]!=0) ? sSummary:NULL) :
            xuiInternalFlowGraphUpdateNodeConfiguration(pWorkflow->pGraph,sId,
            pConfig,ports.pPorts,ports.iCount,__xuiWorkflowHasDynamicPorts(pType),(sSummary[0]!=0) ? sSummary:NULL);
	}
	__xuiWorkflowFreeDynamicPorts(pType, &ports);
	return iRet;
}

XUI_API int xuiWorkflowSetNodeConfig(xui_workflow pWorkflow,const char* sId,xvalue* pConfig)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
    return __xuiWorkflowOperationEnd(&__ownedCall,__xuiWorkflowSetNodeConfigValue(pWorkflow,sId,pConfig,false),0);
}
XUI_API int xuiWorkflowSetNodeConfigShared(xui_workflow pWorkflow,const char* sId,xvalue* pConfig)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
    return __xuiWorkflowOperationEnd(&__ownedCall,__xuiWorkflowSetNodeConfigValue(pWorkflow,sId,pConfig,true),0);
}

XUI_API int xuiWorkflowGetNodeConfig(xui_workflow pWorkflow, const char* sId, xvalue** ppConfig)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	if ( pWorkflow == NULL ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	return __xuiWorkflowOperationEnd(&__ownedCall, (xuiFlowGraphGetNodeConfig(pWorkflow->pGraph, sId, ppConfig)), 0);
}

XUI_API int xuiWorkflowGetNodeConfigShared(xui_workflow pWorkflow, const char* sId, xvalue** ppConfig)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	if ( pWorkflow == NULL ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	return __xuiWorkflowOperationEnd(&__ownedCall, (xuiFlowGraphGetNodeConfigShared(pWorkflow->pGraph, sId, ppConfig)), 0);
}

XUI_API int xuiWorkflowMakeVariableId(xui_workflow pWorkflow, char* sBuffer, int iCapacity)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	char sId[32];
	uint32_t iNext;
	int iLength;

	if ( (pWorkflow == NULL) || (sBuffer == NULL) || (iCapacity <= 0) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	sBuffer[0] = '\0';
	iNext = pWorkflow->iNextVariableId;
	do {
		if ( iNext == UINT32_MAX ) {
			return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_LIMIT_EXCEEDED), 0);
		}
		iNext++;
		iLength = snprintf(sId, sizeof(sId), "var_%u", iNext);
		if ( iLength < 0 || iLength >= (int)sizeof(sId) || iLength >= iCapacity ) {
			return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_LIMIT_EXCEEDED), 0);
		}
	} while ( __xuiWorkflowDictFindIndex(&pWorkflow->mapVariables, sId) >= 0 );
	memcpy(sBuffer, sId, (size_t)iLength + 1u);
	pWorkflow->iNextVariableId = iNext;
	return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_OK), 0);
}

static int __xuiWorkflowAddVariableValue(xui_workflow pWorkflow, const xui_workflow_variable_desc_t* pDesc, int* pIndex,bool shared)
{
	xui_workflow_variable_model_t* pVariable;
	uint32_t iPos;
	int iIndex;
	int iRet;

	if ( pIndex != NULL ) {
		*pIndex = -1;
	}
	if ( (pWorkflow == NULL) || (pDesc == NULL) || (pDesc->iSize < sizeof(*pDesc)) || (pDesc->sId == NULL) || (pDesc->sId[0] == 0) ) {
		return (XUI_ERROR_INVALID_ARGUMENT);
	}
	if ( xuiWorkflowFindVariable(pWorkflow, pDesc->sId) >= 0 ) {
		return (XUI_ERROR_ALREADY_INITIALIZED);
	}
	iPos = xuiXrtArrayAppendSpace(&pWorkflow->arrVariables, 1u);
	if ( iPos == 0u ) {
		return (XUI_ERROR_OUT_OF_MEMORY);
	}
	iIndex = (int)iPos - 1;
	pVariable = (xui_workflow_variable_model_t*)xuiXrtArrayGet(&pWorkflow->arrVariables, iPos);
	memset(pVariable, 0, sizeof(*pVariable));
	pVariable->sId = __xuiWorkflowCopyString(pDesc->sId);
	pVariable->sTitle = __xuiWorkflowCopyString(pDesc->sTitle);
	pVariable->sType = __xuiWorkflowCopyString(pDesc->sType);
	pVariable->sScope = __xuiWorkflowCopyString(pDesc->sScope);
	pVariable->pDefaultValue = (pDesc->pDefaultValue != NULL) ? (shared ? __xuiWorkflowValueRef(pWorkflow,pDesc->pDefaultValue) : __xuiWorkflowValueClone(pWorkflow,pDesc->pDefaultValue)) : NULL;
	if ( pVariable->sId == NULL ||
	     (pDesc->sTitle != NULL && pVariable->sTitle == NULL) ||
	     (pDesc->sType != NULL && pVariable->sType == NULL) ||
	     (pDesc->sScope != NULL && pVariable->sScope == NULL) ||
	     (pDesc->pDefaultValue != NULL && pVariable->pDefaultValue == NULL) ) {
		__xuiWorkflowFreeVariable(pWorkflow, pVariable);
		xuiXrtArrayRemove(&pWorkflow->arrVariables, iPos, 1u);
		return (XUI_ERROR_OUT_OF_MEMORY);
	}
	iRet = __xuiWorkflowDictSetIndex(&pWorkflow->mapVariables, pVariable->sId, iIndex);
	if ( iRet != XUI_OK ) {
		__xuiWorkflowFreeVariable(pWorkflow, pVariable);
		xuiXrtArrayRemove(&pWorkflow->arrVariables, iPos, 1u);
		return (iRet);
	}
	if ( pIndex != NULL ) {
		*pIndex = iIndex;
	}
	return (XUI_OK);
}

XUI_API int xuiWorkflowAddVariable(xui_workflow pWorkflow, const xui_workflow_variable_desc_t* pDesc, int* pIndex)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
    return __xuiWorkflowOperationEnd(&__ownedCall,__xuiWorkflowAddVariableValue(pWorkflow,pDesc,pIndex,false),0);
}
XUI_API int xuiWorkflowAddVariableShared(xui_workflow pWorkflow, const xui_workflow_variable_desc_t* pDesc, int* pIndex)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
    return __xuiWorkflowOperationEnd(&__ownedCall,__xuiWorkflowAddVariableValue(pWorkflow,pDesc,pIndex,true),0);
}

XUI_API int xuiWorkflowFindVariable(xui_workflow pWorkflow, const char* sId)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	if ( pWorkflow == NULL ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (-1), -1);
	}
	return __xuiWorkflowOperationEnd(&__ownedCall, (__xuiWorkflowDictFindIndex(&pWorkflow->mapVariables, sId)), -1);
}

XUI_API int xuiWorkflowGetVariableCount(xui_workflow pWorkflow)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	return __xuiWorkflowOperationEnd(&__ownedCall, ((pWorkflow != NULL) ? (int)pWorkflow->arrVariables.Count : 0), 0);
}

XUI_API int xuiWorkflowGetVariable(xui_workflow pWorkflow, int iIndex, xui_workflow_variable_desc_t* pDesc)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	xui_workflow_variable_model_t* pVariable;

	if ( (pWorkflow == NULL) || (pDesc == NULL) || (iIndex < 0) || ((uint32_t)iIndex >= pWorkflow->arrVariables.Count) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	pVariable = (xui_workflow_variable_model_t*)xuiXrtArrayGet(&pWorkflow->arrVariables, (uint32_t)iIndex + 1u);
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iSize = sizeof(*pDesc);
	pDesc->sId = pVariable->sId;
	pDesc->sTitle = pVariable->sTitle;
	pDesc->sType = pVariable->sType;
	pDesc->sScope = pVariable->sScope;
	pDesc->pDefaultValue = pVariable->pDefaultValue;
	return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_OK), 0);
}

static const char* __xuiWorkflowXsonText(xvalue* pValue)
{
	return (pValue != NULL && xuiXrtValueType(pValue) == XVALUE_STRING) ? (const char*)xuiXrtValueGetText(pValue) : NULL;
}

static int __xuiWorkflowXsonOptionalType(xvalue* pValue, int iType)
{
	int iValueType = xuiXrtValueType(pValue);
	return iValueType == XVALUE_NULL || iValueType == iType;
}

static int __xuiWorkflowXsonOptionalNumber(xvalue* pValue)
{
	int iType = xuiXrtValueType(pValue);
	return iType == XVALUE_NULL || iType == XVALUE_INT || iType == XVALUE_FLOAT;
}

/* Missing/null optional coordinates retain the zero-initialized descriptor.
 * Do not call a checked numeric getter on NULL and leave a spurious XRT error. */
static float __xuiWorkflowXsonOptionalFloat(xvalue* pValue)
{
    return xuiXrtValueType(pValue)==XVALUE_NULL ? 0.0f : (float)xuiXrtValueGetFloat(pValue);
}

static int __xuiWorkflowXsonInt(xvalue* pValue, int* pResult)
{
	int64 iValue;

	if ( pResult == NULL || xuiXrtValueType(pValue) != XVALUE_INT ) {
		return 0;
	}
	iValue = xuiXrtValueGetInt(pValue);
	if ( iValue < INT_MIN || iValue > INT_MAX ) {
		return 0;
	}
	*pResult = (int)iValue;
	return 1;
}

static int __xuiWorkflowXsonHeaderValid(xvalue* pRoot)
{
	xvalue* pKind;
	xvalue* pVersion;
	xvalue* pMetadata;
	xvalue* pFormat;
	xvalue* pSchemaVersion;
	const char* sText;

	pKind = xuiXrtValueObjectGet(pRoot, "kind", 0);
	if ( xuiXrtValueType(pKind) != XVALUE_NULL ) {
		sText = __xuiWorkflowXsonText(pKind);
		if ( sText == NULL || strcmp(sText, "xui.workflow") != 0 ) return 0;
	}
	pVersion = xuiXrtValueObjectGet(pRoot, "version", 0);
	if ( xuiXrtValueType(pVersion) != XVALUE_NULL &&
	     (xuiXrtValueType(pVersion) != XVALUE_INT || xuiXrtValueGetInt(pVersion) != 1) ) return 0;
	pMetadata = xuiXrtValueObjectGet(pRoot, "metadata", 0);
	if ( xuiXrtValueType(pMetadata) == XVALUE_NULL ) return 1;
	if ( xuiXrtValueType(pMetadata) != XVALUE_OBJECT ) return 0;
	pFormat = xuiXrtValueObjectGet(pMetadata, "format", 0);
	if ( xuiXrtValueType(pFormat) != XVALUE_NULL ) {
		sText = __xuiWorkflowXsonText(pFormat);
		if ( sText == NULL || strcmp(sText, "xui.workflow.v1") != 0 ) return 0;
	}
	pSchemaVersion = xuiXrtValueObjectGet(pMetadata, "schemaVersion", 0);
	if ( xuiXrtValueType(pSchemaVersion) != XVALUE_NULL &&
	     (xuiXrtValueType(pSchemaVersion) != XVALUE_INT || xuiXrtValueGetInt(pSchemaVersion) != 1) ) return 0;
	return 1;
}

static int __xuiWorkflowXsonSetText(xvalue* pTable, const char* sKey, const char* sValue)
{
	if ( sValue == NULL ) {
		return XUI_OK;
	}
	return xuiXrtValueObjectSetText(pTable, sKey, 0, (ptr)(void*)sValue, 0, FALSE) ? XUI_OK : XUI_ERROR_OUT_OF_MEMORY;
}

static int __xuiWorkflowXsonSetValueCopy(xui_workflow pWorkflow, xvalue* pTable, const char* sKey, xvalue* pValue)
{
	xvalue* pCopy;

	if ( pValue == NULL ) {
		return XUI_OK;
	}
	pCopy = __xuiWorkflowValueClone(pWorkflow, pValue);
	if ( pCopy == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( !xuiXrtValueObjectSetTake(pTable, sKey, 0, pCopy, TRUE) ) {
		__xuiWorkflowValueRelease(pWorkflow, pCopy);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	return XUI_OK;
}

static int __xuiWorkflowXsonSetObject(xui_workflow pWorkflow, xvalue* pTable, const char* sKey, xvalue* pChild)
{
	if ( pChild == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	if ( !xuiXrtValueObjectSetTake(pTable, sKey, 0, pChild, TRUE) ) {
		__xuiWorkflowValueRelease(pWorkflow, pChild);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	return XUI_OK;
}

static int __xuiWorkflowAppendPortXValue(xui_workflow pWorkflow, xvalue* pPorts, const xui_flow_port_info_t* pPort)
{
	xvalue* pObj;
	int iRet;

	pObj = xrtValueObject();
	if ( pObj == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xuiWorkflowXsonSetText(pObj, "id", pPort->sId);
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "title", pPort->sTitle);
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "dataType", pPort->sDataType);
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetInt(pObj, "direction", 0, pPort->iDirection) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetInt(pObj, "kind", 0, pPort->iKind) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetBool(pObj, "required", 0, pPort->bRequired ? TRUE : FALSE) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetBool(pObj, "multi", 0, pPort->bMulti ? TRUE : FALSE) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetBool(pObj, "dynamic", 0, pPort->bDynamic ? TRUE : FALSE) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueArrayAppendTake(pPorts, pObj, TRUE) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet != XUI_OK ) {
		__xuiWorkflowValueRelease(pWorkflow, pObj);
	}
	return iRet;
}

static int __xuiWorkflowAppendNodeXValue(xui_workflow pWorkflow, xvalue* pNodes, xvalue* pTypeRefs, int iNode, bool shared)
{
	xui_flow_node_info_t tNode;
	xui_flow_port_info_t tPort;
	xvalue* pObj;
	xvalue* pPorts;
	xvalue* pConfig;
	int i;
	int iPortCount;
	int iRet;

	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	iRet = xuiFlowGraphGetNode(pWorkflow->pGraph, iNode, &tNode);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pObj = xrtValueObject();
	pPorts = xrtValueArray();
	if ( pObj == NULL || pPorts == NULL ) {
		if ( pObj != NULL ) __xuiWorkflowValueRelease(pWorkflow, pObj);
		if ( pPorts != NULL ) __xuiWorkflowValueRelease(pWorkflow, pPorts);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xuiWorkflowXsonSetText(pObj, "id", tNode.sId);
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "type", tNode.sType);
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "title", tNode.sTitle);
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "summary", tNode.sSummary);
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetFloat(pObj, "x", 0, tNode.fX) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetFloat(pObj, "y", 0, tNode.fY) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetFloat(pObj, "w", 0, tNode.fW) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetFloat(pObj, "h", 0, tNode.fH) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetInt(pObj, "runState", 0, tNode.iRunState) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "runPreview", tNode.sRunPreview);
	if ( iRet == XUI_OK && tNode.sType != NULL && !xuiXrtValueArrayAppendText(pTypeRefs, (ptr)(void*)tNode.sType, 0, FALSE) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	iPortCount = xuiFlowGraphGetNodePortCount(pWorkflow->pGraph, iNode);
	for ( i = 0; iRet == XUI_OK && i < iPortCount; ++i ) {
		memset(&tPort, 0, sizeof(tPort));
		tPort.iSize = sizeof(tPort);
		iRet = xuiFlowGraphGetPort(pWorkflow->pGraph, iNode, i, &tPort);
		if ( iRet == XUI_OK ) {
			iRet = __xuiWorkflowAppendPortXValue(pWorkflow, pPorts, &tPort);
		}
	}
	if ( iRet == XUI_OK ) {
		iRet = __xuiWorkflowXsonSetObject(pWorkflow, pObj, "ports", pPorts);
		pPorts = NULL;
	}
	pConfig = NULL;
	if ( iRet == XUI_OK ) {
		iRet = shared ? xuiFlowGraphGetNodeConfigShared(pWorkflow->pGraph, tNode.sId, &pConfig) :
			xuiFlowGraphGetNodeConfig(pWorkflow->pGraph, tNode.sId, &pConfig);
		if ( iRet == XUI_OK && pConfig != NULL ) {
			if (shared) {
				iRet = __xuiWorkflowXsonSetObject(pWorkflow, pObj, "config", pConfig);
				pConfig = NULL;
			} else {
				iRet = __xuiWorkflowXsonSetValueCopy(pWorkflow, pObj, "config", pConfig);
			}
		}
		if ( pConfig != NULL ) __xuiWorkflowValueRelease(pWorkflow, pConfig);
	}
	if ( iRet == XUI_OK && !xuiXrtValueArrayAppendTake(pNodes, pObj, TRUE) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( pPorts != NULL ) __xuiWorkflowValueRelease(pWorkflow, pPorts);
	if ( iRet != XUI_OK ) __xuiWorkflowValueRelease(pWorkflow, pObj);
	return iRet;
}

static int __xuiWorkflowAppendEdgeXValue(xui_workflow pWorkflow, xvalue* pEdges, int iEdge)
{
	xui_flow_edge_info_t tEdge;
	xvalue* pObj;
	xvalue* pRoute;
	int iRet;

	memset(&tEdge, 0, sizeof(tEdge));
	tEdge.iSize = sizeof(tEdge);
	iRet = xuiFlowGraphGetEdge(pWorkflow->pGraph, iEdge, &tEdge);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pObj = xrtValueObject();
	if ( pObj == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pRoute = NULL;
	iRet = __xuiWorkflowXsonSetText(pObj, "id", tEdge.sId);
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetInt(pObj, "kind", 0, tEdge.iKind) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "fromNode", tEdge.sFromNode);
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "fromPort", tEdge.sFromPort);
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "toNode", tEdge.sToNode);
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "toPort", tEdge.sToPort);
	if ( iRet == XUI_OK && (tEdge.iRouteStyle != XUI_FLOW_ROUTE_AUTO || tEdge.fRouteBias != 0.0f || tEdge.fRouteSourceOffset != 0.0f || tEdge.fRouteTargetOffset != 0.0f) ) {
		pRoute = xrtValueObject();
		if ( pRoute == NULL ) {
			iRet = XUI_ERROR_OUT_OF_MEMORY;
		}
		if ( iRet == XUI_OK && !xuiXrtValueObjectSetInt(pRoute, "style", 0, tEdge.iRouteStyle) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
		if ( iRet == XUI_OK && !xuiXrtValueObjectSetFloat(pRoute, "bias", 0, tEdge.fRouteBias) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
		if ( iRet == XUI_OK && !xuiXrtValueObjectSetFloat(pRoute, "sourceOffset", 0, tEdge.fRouteSourceOffset) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
		if ( iRet == XUI_OK && !xuiXrtValueObjectSetFloat(pRoute, "targetOffset", 0, tEdge.fRouteTargetOffset) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
		if ( iRet == XUI_OK ) {
			iRet = __xuiWorkflowXsonSetObject(pWorkflow, pObj, "route", pRoute);
			pRoute = NULL;
		}
	}
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetBool(pObj, "invalid", 0, tEdge.bInvalid ? TRUE : FALSE) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetInt(pObj, "runState", 0, tEdge.iRunState) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "runPreview", tEdge.sRunPreview);
	if ( iRet == XUI_OK && !xuiXrtValueArrayAppendTake(pEdges, pObj, TRUE) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( pRoute != NULL ) __xuiWorkflowValueRelease(pWorkflow, pRoute);
	if ( iRet != XUI_OK ) __xuiWorkflowValueRelease(pWorkflow, pObj);
	return iRet;
}

static int __xuiWorkflowAppendVariableXValue(xui_workflow pWorkflow, xvalue* pVariables,
	const xui_workflow_variable_model_t* pVariable, bool shared)
{
	xvalue* pObj;
	int iRet;

	pObj = xrtValueObject();
	if ( pObj == NULL ) {
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = __xuiWorkflowXsonSetText(pObj, "id", pVariable->sId);
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "title", pVariable->sTitle);
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "type", pVariable->sType);
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pObj, "scope", pVariable->sScope);
	if (iRet == XUI_OK && pVariable->pDefaultValue) {
		iRet = shared ? __xuiWorkflowXsonSetObject(pWorkflow, pObj, "default",
			__xuiWorkflowValueRef(pWorkflow, pVariable->pDefaultValue)) :
			__xuiWorkflowXsonSetValueCopy(pWorkflow, pObj, "default", pVariable->pDefaultValue);
	}
	if ( iRet == XUI_OK && !xuiXrtValueArrayAppendTake(pVariables, pObj, TRUE) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet != XUI_OK ) __xuiWorkflowValueRelease(pWorkflow, pObj);
	return iRet;
}

static int __xuiWorkflowToXValue(xui_workflow pWorkflow, xvalue** ppValue, bool shared)
{
	xui_flow_viewport_t tViewport;
	xui_workflow_variable_model_t* pVariable;
	xvalue* pRoot;
	xvalue* pMeta;
	xvalue* pTypes;
	xvalue* pVariables;
	xvalue* pNodes;
	xvalue* pEdges;
	xvalue* pViewport;
	uint32_t i;
	int iRet;

	if ( (pWorkflow == NULL) || (ppValue == NULL) ) {
		return (XUI_ERROR_INVALID_ARGUMENT);
	}
	*ppValue = NULL;
	pRoot = xrtValueObject();
	pMeta = xrtValueObject();
	pTypes = xrtValueArray();
	pVariables = xrtValueArray();
	pNodes = xrtValueArray();
	pEdges = xrtValueArray();
	pViewport = xrtValueObject();
	if ( pRoot == NULL || pMeta == NULL || pTypes == NULL || pVariables == NULL || pNodes == NULL || pEdges == NULL || pViewport == NULL ) {
		if ( pRoot != NULL ) __xuiWorkflowValueRelease(pWorkflow, pRoot);
		if ( pMeta != NULL ) __xuiWorkflowValueRelease(pWorkflow, pMeta);
		if ( pTypes != NULL ) __xuiWorkflowValueRelease(pWorkflow, pTypes);
		if ( pVariables != NULL ) __xuiWorkflowValueRelease(pWorkflow, pVariables);
		if ( pNodes != NULL ) __xuiWorkflowValueRelease(pWorkflow, pNodes);
		if ( pEdges != NULL ) __xuiWorkflowValueRelease(pWorkflow, pEdges);
		if ( pViewport != NULL ) __xuiWorkflowValueRelease(pWorkflow, pViewport);
		return (XUI_ERROR_OUT_OF_MEMORY);
	}
	iRet = __xuiWorkflowXsonSetText(pRoot, "kind", "xui.workflow");
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetInt(pRoot, "version", 0, 1) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK ) iRet = __xuiWorkflowXsonSetText(pMeta, "format", "xui.workflow.v1");
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetInt(pMeta, "schemaVersion", 0, 1) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	for ( i = 1u; iRet == XUI_OK && i <= pWorkflow->arrVariables.Count; ++i ) {
		pVariable = (xui_workflow_variable_model_t*)xuiXrtArrayGet(&pWorkflow->arrVariables, i);
		iRet = __xuiWorkflowAppendVariableXValue(pWorkflow, pVariables, pVariable, shared);
	}
	for ( i = 0u; iRet == XUI_OK && i < (uint32_t)xuiFlowGraphGetNodeCount(pWorkflow->pGraph); ++i ) {
		iRet = __xuiWorkflowAppendNodeXValue(pWorkflow, pNodes, pTypes, (int)i, shared);
	}
	for ( i = 0u; iRet == XUI_OK && i < (uint32_t)xuiFlowGraphGetEdgeCount(pWorkflow->pGraph); ++i ) {
		iRet = __xuiWorkflowAppendEdgeXValue(pWorkflow, pEdges, (int)i);
	}
	memset(&tViewport, 0, sizeof(tViewport));
	tViewport.iSize = sizeof(tViewport);
	if ( iRet == XUI_OK ) iRet = xuiFlowGraphGetViewport(pWorkflow->pGraph, &tViewport);
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetFloat(pViewport, "panX", 0, tViewport.fPanX) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetFloat(pViewport, "panY", 0, tViewport.fPanY) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetFloat(pViewport, "zoom", 0, tViewport.fZoom) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetFloat(pViewport, "width", 0, tViewport.fWidth) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK && !xuiXrtValueObjectSetFloat(pViewport, "height", 0, tViewport.fHeight) ) iRet = XUI_ERROR_OUT_OF_MEMORY;
	if ( iRet == XUI_OK ) {
		iRet = __xuiWorkflowXsonSetObject(pWorkflow, pRoot, "metadata", pMeta);
		pMeta = NULL;
	}
	if ( iRet == XUI_OK ) {
		iRet = __xuiWorkflowXsonSetObject(pWorkflow, pRoot, "nodeTypes", pTypes);
		pTypes = NULL;
	}
	if ( iRet == XUI_OK ) {
		iRet = __xuiWorkflowXsonSetObject(pWorkflow, pRoot, "variables", pVariables);
		pVariables = NULL;
	}
	if ( iRet == XUI_OK ) {
		iRet = __xuiWorkflowXsonSetObject(pWorkflow, pRoot, "nodes", pNodes);
		pNodes = NULL;
	}
	if ( iRet == XUI_OK ) {
		iRet = __xuiWorkflowXsonSetObject(pWorkflow, pRoot, "edges", pEdges);
		pEdges = NULL;
	}
	if ( iRet == XUI_OK ) {
		iRet = __xuiWorkflowXsonSetObject(pWorkflow, pRoot, "viewport", pViewport);
		pViewport = NULL;
	}
	if ( pMeta != NULL ) __xuiWorkflowValueRelease(pWorkflow, pMeta);
	if ( pTypes != NULL ) __xuiWorkflowValueRelease(pWorkflow, pTypes);
	if ( pVariables != NULL ) __xuiWorkflowValueRelease(pWorkflow, pVariables);
	if ( pNodes != NULL ) __xuiWorkflowValueRelease(pWorkflow, pNodes);
	if ( pEdges != NULL ) __xuiWorkflowValueRelease(pWorkflow, pEdges);
	if ( pViewport != NULL ) __xuiWorkflowValueRelease(pWorkflow, pViewport);
	if ( iRet != XUI_OK ) {
		__xuiWorkflowValueRelease(pWorkflow, pRoot);
		return (iRet);
	}
	*ppValue = pRoot;
	return (XUI_OK);
}

XUI_API int xuiWorkflowToXValue(xui_workflow pWorkflow, xvalue** ppValue)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
    return __xuiWorkflowOperationEnd(&__ownedCall,__xuiWorkflowToXValue(pWorkflow,ppValue,false),0);
}

XUI_API int xuiWorkflowToXValueShared(xui_workflow workflow, xvalue** output)
{
	if (__xuiWorkflowModelBusy(workflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(workflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
    return __xuiWorkflowOperationEnd(&__ownedCall,__xuiWorkflowToXValue(workflow,output,true),0);
}

static int __xuiWorkflowLoadPortXValue(xui_flow_graph pGraph, int iNode, xvalue* pObj)
{
	xui_flow_port_desc_t tPort;
	xvalue* pDirection;
	xvalue* pKind;
	int iDirection;
	int iKind;

	if ( pObj == NULL || xuiXrtValueType(pObj) != XVALUE_OBJECT ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pDirection = xuiXrtValueObjectGet(pObj, "direction", 0);
	pKind = xuiXrtValueObjectGet(pObj, "kind", 0);
	if ( xuiXrtValueType(xuiXrtValueObjectGet(pObj, "id", 0)) != XVALUE_STRING ||
	     !__xuiWorkflowXsonInt(pDirection, &iDirection) || !__xuiWorkflowXsonInt(pKind, &iKind) ||
	     !__xuiWorkflowXsonOptionalType(xuiXrtValueObjectGet(pObj, "title", 0), XVALUE_STRING) ||
	     !__xuiWorkflowXsonOptionalType(xuiXrtValueObjectGet(pObj, "dataType", 0), XVALUE_STRING) ||
	     !__xuiWorkflowXsonOptionalType(xuiXrtValueObjectGet(pObj, "required", 0), XVALUE_BOOL) ||
	     !__xuiWorkflowXsonOptionalType(xuiXrtValueObjectGet(pObj, "multi", 0), XVALUE_BOOL) ||
	     !__xuiWorkflowXsonOptionalType(xuiXrtValueObjectGet(pObj, "dynamic", 0), XVALUE_BOOL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tPort, 0, sizeof(tPort));
	tPort.iSize = sizeof(tPort);
	tPort.sId = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "id", 0));
	tPort.sTitle = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "title", 0));
	tPort.sDataType = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "dataType", 0));
	tPort.iDirection = iDirection;
	tPort.iKind = iKind;
	tPort.bRequired = xuiXrtValueGetBool(xuiXrtValueObjectGet(pObj, "required", 0)) ? 1 : 0;
	tPort.bMulti = xuiXrtValueGetBool(xuiXrtValueObjectGet(pObj, "multi", 0)) ? 1 : 0;
	tPort.bDynamic = xuiXrtValueGetBool(xuiXrtValueObjectGet(pObj, "dynamic", 0)) ? 1 : 0;
	return xuiFlowGraphAddPort(pGraph, iNode, &tPort, NULL);
}

static int __xuiWorkflowLoadNodeXValue(xui_flow_graph pGraph, xvalue* pObj, bool shared)
{
	xui_flow_node_desc_t tNode;
	xvalue* pPorts;
	xvalue* pConfig;
	xvalue* pRunPreview;
	xvalue* pRunState;
	uint32_t i;
	int iNode;
	int iRunState;
	int iRet;

	if ( pObj == NULL || xuiXrtValueType(pObj) != XVALUE_OBJECT ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( xuiXrtValueType(xuiXrtValueObjectGet(pObj, "id", 0)) != XVALUE_STRING ||
	     xuiXrtValueType(xuiXrtValueObjectGet(pObj, "type", 0)) != XVALUE_STRING ||
	     !__xuiWorkflowXsonOptionalType(xuiXrtValueObjectGet(pObj, "title", 0), XVALUE_STRING) ||
	     !__xuiWorkflowXsonOptionalType(xuiXrtValueObjectGet(pObj, "summary", 0), XVALUE_STRING) ||
	     !__xuiWorkflowXsonOptionalNumber(xuiXrtValueObjectGet(pObj, "x", 0)) ||
	     !__xuiWorkflowXsonOptionalNumber(xuiXrtValueObjectGet(pObj, "y", 0)) ||
	     !__xuiWorkflowXsonOptionalNumber(xuiXrtValueObjectGet(pObj, "w", 0)) ||
	     !__xuiWorkflowXsonOptionalNumber(xuiXrtValueObjectGet(pObj, "h", 0)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pRunState = xuiXrtValueObjectGet(pObj, "runState", 0);
	pRunPreview = xuiXrtValueObjectGet(pObj, "runPreview", 0);
	iRunState = XUI_WORKFLOW_NODE_RUN_IDLE;
	if ( (xuiXrtValueType(pRunState) != XVALUE_NULL && !__xuiWorkflowXsonInt(pRunState, &iRunState)) ||
	     !__xuiWorkflowXsonOptionalType(pRunPreview, XVALUE_STRING) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	tNode.sId = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "id", 0));
	tNode.sType = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "type", 0));
	tNode.sTitle = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "title", 0));
	tNode.sSummary = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "summary", 0));
	tNode.fX = __xuiWorkflowXsonOptionalFloat(xuiXrtValueObjectGet(pObj, "x", 0));
	tNode.fY = __xuiWorkflowXsonOptionalFloat(xuiXrtValueObjectGet(pObj, "y", 0));
	tNode.fW = __xuiWorkflowXsonOptionalFloat(xuiXrtValueObjectGet(pObj, "w", 0));
	tNode.fH = __xuiWorkflowXsonOptionalFloat(xuiXrtValueObjectGet(pObj, "h", 0));
	iRet = xuiFlowGraphAddNode(pGraph, &tNode, &iNode);
	if ( iRet != XUI_OK ) {
		return iRet;
	}
	pPorts = xuiXrtValueObjectGet(pObj, "ports", 0);
	if ( pPorts != NULL && xuiXrtValueType(pPorts) != XVALUE_NULL && xuiXrtValueType(pPorts) != XVALUE_ARRAY ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pPorts != NULL && xuiXrtValueType(pPorts) == XVALUE_ARRAY ) {
		for ( i = 0u; i < xrtValueCount(pPorts); ++i ) {
			iRet = __xuiWorkflowLoadPortXValue(pGraph, iNode, xuiXrtValueArrayGet(pPorts, i));
			if ( iRet != XUI_OK ) {
				return iRet;
			}
		}
	}
	pConfig = xuiXrtValueObjectGet(pObj, "config", 0);
	if ( pConfig != NULL && xuiXrtValueType(pConfig) != XVALUE_NULL ) {
		if ( xuiXrtValueType(pConfig) != XVALUE_OBJECT ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		iRet = shared ? xuiFlowGraphSetNodeConfigShared(pGraph, tNode.sId, pConfig) :
			xuiFlowGraphSetNodeConfig(pGraph, tNode.sId, pConfig);
	}
	if ( iRet == XUI_OK ) {
		if ( pRunState != NULL && xuiXrtValueType(pRunState) != XVALUE_NULL ) {
			iRet = xuiFlowGraphSetNodeRunState(pGraph, tNode.sId, iRunState, __xuiWorkflowXsonText(pRunPreview));
		}
	}
	return iRet;
}

static int __xuiWorkflowLoadEdgeXValue(xui_flow_graph pGraph, xvalue* pObj)
{
	xui_flow_edge_desc_t tEdge;
	xvalue* pRoute;
	xvalue* pRouteBias;
	xvalue* pRouteSourceOffset;
	xvalue* pRouteStyle;
	xvalue* pRouteTargetOffset;
	xvalue* pRunPreview;
	xvalue* pRunState;
	int iKind;
	int iRouteStyle;
	int iRunState;

	if ( pObj == NULL || xuiXrtValueType(pObj) != XVALUE_OBJECT ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( xuiXrtValueType(xuiXrtValueObjectGet(pObj, "id", 0)) != XVALUE_STRING ||
	     !__xuiWorkflowXsonInt(xuiXrtValueObjectGet(pObj, "kind", 0), &iKind) ||
	     xuiXrtValueType(xuiXrtValueObjectGet(pObj, "fromNode", 0)) != XVALUE_STRING ||
	     xuiXrtValueType(xuiXrtValueObjectGet(pObj, "fromPort", 0)) != XVALUE_STRING ||
	     xuiXrtValueType(xuiXrtValueObjectGet(pObj, "toNode", 0)) != XVALUE_STRING ||
	     xuiXrtValueType(xuiXrtValueObjectGet(pObj, "toPort", 0)) != XVALUE_STRING ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pRunState = xuiXrtValueObjectGet(pObj, "runState", 0);
	pRunPreview = xuiXrtValueObjectGet(pObj, "runPreview", 0);
	iRunState = XUI_WORKFLOW_EDGE_RUN_IDLE;
	if ( (xuiXrtValueType(pRunState) != XVALUE_NULL && !__xuiWorkflowXsonInt(pRunState, &iRunState)) ||
	     !__xuiWorkflowXsonOptionalType(pRunPreview, XVALUE_STRING) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tEdge, 0, sizeof(tEdge));
	tEdge.iSize = sizeof(tEdge);
	tEdge.sId = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "id", 0));
	tEdge.iKind = iKind;
	tEdge.sFromNode = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "fromNode", 0));
	tEdge.sFromPort = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "fromPort", 0));
	tEdge.sToNode = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "toNode", 0));
	tEdge.sToPort = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "toPort", 0));
	pRoute = xuiXrtValueObjectGet(pObj, "route", 0);
	if ( pRoute != NULL && xuiXrtValueType(pRoute) != XVALUE_NULL ) {
		if ( xuiXrtValueType(pRoute) != XVALUE_OBJECT ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		pRouteStyle = xuiXrtValueObjectGet(pRoute, "style", 0);
		pRouteBias = xuiXrtValueObjectGet(pRoute, "bias", 0);
		pRouteSourceOffset = xuiXrtValueObjectGet(pRoute, "sourceOffset", 0);
		pRouteTargetOffset = xuiXrtValueObjectGet(pRoute, "targetOffset", 0);
		iRouteStyle = XUI_FLOW_ROUTE_AUTO;
		if ( (xuiXrtValueType(pRouteStyle) != XVALUE_NULL && !__xuiWorkflowXsonInt(pRouteStyle, &iRouteStyle)) ||
		     !__xuiWorkflowXsonOptionalNumber(pRouteBias) ||
		     !__xuiWorkflowXsonOptionalNumber(pRouteSourceOffset) ||
		     !__xuiWorkflowXsonOptionalNumber(pRouteTargetOffset) ) {
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		if ( xuiXrtValueType(pRouteStyle) != XVALUE_NULL ) tEdge.iRouteStyle = iRouteStyle;
		tEdge.fRouteBias = __xuiWorkflowXsonOptionalFloat(pRouteBias);
		tEdge.fRouteSourceOffset = __xuiWorkflowXsonOptionalFloat(pRouteSourceOffset);
		tEdge.fRouteTargetOffset = __xuiWorkflowXsonOptionalFloat(pRouteTargetOffset);
	}
	{
		int iRet = xuiFlowGraphAddEdgePreserveInvalid(pGraph, &tEdge, NULL);
		if ( iRet == XUI_OK && pRunState != NULL && xuiXrtValueType(pRunState) != XVALUE_NULL ) {
			iRet = xuiFlowGraphSetEdgeRunState(pGraph, tEdge.sId, iRunState, __xuiWorkflowXsonText(pRunPreview));
		}
		return iRet;
	}
}

static int __xuiWorkflowLoadVariableXValue(xui_workflow pWorkflow, xvalue* pObj, bool shared)
{
	xui_workflow_variable_desc_t tVariable;
	xvalue* pDefault;

	if ( pObj == NULL || xuiXrtValueType(pObj) != XVALUE_OBJECT ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( xuiXrtValueType(xuiXrtValueObjectGet(pObj, "id", 0)) != XVALUE_STRING ||
	     !__xuiWorkflowXsonOptionalType(xuiXrtValueObjectGet(pObj, "title", 0), XVALUE_STRING) ||
	     !__xuiWorkflowXsonOptionalType(xuiXrtValueObjectGet(pObj, "type", 0), XVALUE_STRING) ||
	     !__xuiWorkflowXsonOptionalType(xuiXrtValueObjectGet(pObj, "scope", 0), XVALUE_STRING) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tVariable, 0, sizeof(tVariable));
	tVariable.iSize = sizeof(tVariable);
	tVariable.sId = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "id", 0));
	tVariable.sTitle = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "title", 0));
	tVariable.sType = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "type", 0));
	tVariable.sScope = __xuiWorkflowXsonText(xuiXrtValueObjectGet(pObj, "scope", 0));
	pDefault = xuiXrtValueObjectGet(pObj, "default", 0);
	if ( pDefault != NULL && xuiXrtValueType(pDefault) != XVALUE_NULL ) {
		tVariable.pDefaultValue = pDefault;
	}
	return shared ? xuiWorkflowAddVariableShared(pWorkflow, &tVariable, NULL) :
		xuiWorkflowAddVariable(pWorkflow, &tVariable, NULL);
}

static int __xuiWorkflowLoadXValue(xui_workflow pWorkflow, xvalue* pValue, bool shared)
{
	xui_flow_graph pGraph;
	xui_workflow pVariablesWorkflow;
	xvalue* pNodes;
	xvalue* pEdges;
	xvalue* pVariables;
	xvalue* pViewportValue;
	xui_flow_viewport_t tViewport;
	xarray arrVariables;
	xmap mapVariables;
	uint32_t iNextVariableId;
	uint32_t i;
	int iRet;

	if ( (pWorkflow == NULL) || (pValue == NULL) || (xuiXrtValueType(pValue) != XVALUE_OBJECT) ) {
		return (XUI_ERROR_INVALID_ARGUMENT);
	}
	if ( !__xuiWorkflowXsonHeaderValid(pValue) ) {
		return (XUI_ERROR_INVALID_ARGUMENT);
	}
	pNodes = xuiXrtValueObjectGet(pValue, "nodes", 0);
	pEdges = xuiXrtValueObjectGet(pValue, "edges", 0);
	pVariables = xuiXrtValueObjectGet(pValue, "variables", 0);
	if ( (pNodes == NULL) || (xuiXrtValueType(pNodes) != XVALUE_ARRAY) || (pEdges == NULL) || (xuiXrtValueType(pEdges) != XVALUE_ARRAY) ) {
		return (XUI_ERROR_INVALID_ARGUMENT);
	}
	if ( pVariables != NULL && xuiXrtValueType(pVariables) != XVALUE_ARRAY ) {
		return (XUI_ERROR_INVALID_ARGUMENT);
	}
	pVariablesWorkflow = NULL;
	iRet = xuiFlowGraphCreate(&pGraph);
	if ( iRet != XUI_OK ) {
		return (iRet);
	}
	iRet=xuiInternalFlowGraphSetCallbackOwner(pGraph,pWorkflow->pGraph);
	if(iRet!=XUI_OK){xuiFlowGraphDestroy(pGraph);return (iRet);}
	for ( i = 0u; i < xrtValueCount(pNodes); ++i ) {
		iRet = __xuiWorkflowLoadNodeXValue(pGraph, xuiXrtValueArrayGet(pNodes, i), shared);
		if ( iRet != XUI_OK ) {
			xuiFlowGraphDestroy(pGraph);
			return (iRet);
		}
	}
	for ( i = 0u; i < xrtValueCount(pEdges); ++i ) {
		iRet = __xuiWorkflowLoadEdgeXValue(pGraph, xuiXrtValueArrayGet(pEdges, i));
		if ( iRet != XUI_OK ) {
			xuiFlowGraphDestroy(pGraph);
			return (iRet);
		}
	}
	pViewportValue = xuiXrtValueObjectGet(pValue, "viewport", 0);
	if ( pViewportValue != NULL && xuiXrtValueType(pViewportValue) != XVALUE_NULL && xuiXrtValueType(pViewportValue) != XVALUE_OBJECT ) {
		xuiFlowGraphDestroy(pGraph);
		return (XUI_ERROR_INVALID_ARGUMENT);
	}
	if ( pViewportValue != NULL && xuiXrtValueType(pViewportValue) == XVALUE_OBJECT ) {
		xvalue* pField;
		if ( !__xuiWorkflowXsonOptionalNumber(xuiXrtValueObjectGet(pViewportValue, "panX", 0)) ||
		     !__xuiWorkflowXsonOptionalNumber(xuiXrtValueObjectGet(pViewportValue, "panY", 0)) ||
		     !__xuiWorkflowXsonOptionalNumber(xuiXrtValueObjectGet(pViewportValue, "zoom", 0)) ||
		     !__xuiWorkflowXsonOptionalNumber(xuiXrtValueObjectGet(pViewportValue, "width", 0)) ||
		     !__xuiWorkflowXsonOptionalNumber(xuiXrtValueObjectGet(pViewportValue, "height", 0)) ) {
			xuiFlowGraphDestroy(pGraph);
			return (XUI_ERROR_INVALID_ARGUMENT);
		}
		memset(&tViewport, 0, sizeof(tViewport));
		tViewport.iSize = sizeof(tViewport);
		iRet = xuiFlowGraphGetViewport(pGraph, &tViewport);
		if ( iRet != XUI_OK ) {
			xuiFlowGraphDestroy(pGraph);
			return (iRet);
		}
		pField = xuiXrtValueObjectGet(pViewportValue, "panX", 0);
		if ( pField != NULL && xuiXrtValueType(pField) != XVALUE_NULL ) tViewport.fPanX = (float)xuiXrtValueGetFloat(pField);
		pField = xuiXrtValueObjectGet(pViewportValue, "panY", 0);
		if ( pField != NULL && xuiXrtValueType(pField) != XVALUE_NULL ) tViewport.fPanY = (float)xuiXrtValueGetFloat(pField);
		pField = xuiXrtValueObjectGet(pViewportValue, "zoom", 0);
		if ( pField != NULL && xuiXrtValueType(pField) != XVALUE_NULL ) tViewport.fZoom = (float)xuiXrtValueGetFloat(pField);
		pField = xuiXrtValueObjectGet(pViewportValue, "width", 0);
		if ( pField != NULL && xuiXrtValueType(pField) != XVALUE_NULL ) tViewport.fWidth = (float)xuiXrtValueGetFloat(pField);
		pField = xuiXrtValueObjectGet(pViewportValue, "height", 0);
		if ( pField != NULL && xuiXrtValueType(pField) != XVALUE_NULL ) tViewport.fHeight = (float)xuiXrtValueGetFloat(pField);
		iRet = xuiFlowGraphSetViewport(pGraph, &tViewport);
		if ( iRet != XUI_OK ) {
			xuiFlowGraphDestroy(pGraph);
			return (iRet);
		}
	}
	iRet = xuiWorkflowCreate(&pVariablesWorkflow);
	if ( iRet != XUI_OK ) {
		xuiFlowGraphDestroy(pGraph);
		return (iRet);
	}
	iRet=xuiInternalFlowGraphSetCallbackOwner(pVariablesWorkflow->pGraph,pWorkflow->pGraph);
	if(iRet!=XUI_OK){
		xuiWorkflowDestroy(pVariablesWorkflow);xuiFlowGraphDestroy(pGraph);
		return iRet;
	}
	if ( pVariables != NULL ) {
		for ( i = 0u; i < xrtValueCount(pVariables); ++i ) {
			iRet = __xuiWorkflowLoadVariableXValue(pVariablesWorkflow, xuiXrtValueArrayGet(pVariables, i), shared);
			if ( iRet != XUI_OK ) {
				xuiWorkflowDestroy(pVariablesWorkflow);
				xuiFlowGraphDestroy(pGraph);
				return (iRet);
			}
		}
	}
	iRet = xuiInternalFlowGraphSwapContents(pWorkflow->pGraph, pGraph);
	if (iRet != XUI_OK) {
		xuiWorkflowDestroy(pVariablesWorkflow);
		xuiFlowGraphDestroy(pGraph);
		return (iRet);
	}
	xuiFlowGraphDestroy(pGraph);
	arrVariables = pWorkflow->arrVariables;
	pWorkflow->arrVariables = pVariablesWorkflow->arrVariables;
	pVariablesWorkflow->arrVariables = arrVariables;
	mapVariables = pWorkflow->mapVariables;
	pWorkflow->mapVariables = pVariablesWorkflow->mapVariables;
	pVariablesWorkflow->mapVariables = mapVariables;
	iNextVariableId = pWorkflow->iNextVariableId;
	pWorkflow->iNextVariableId = pVariablesWorkflow->iNextVariableId;
	pVariablesWorkflow->iNextVariableId = iNextVariableId;
	xuiWorkflowDestroy(pVariablesWorkflow);
	return (XUI_OK);
}

XUI_API int xuiWorkflowLoadXValue(xui_workflow pWorkflow, xvalue* pValue)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
    return __xuiWorkflowOperationEnd(&__ownedCall,__xuiWorkflowLoadXValue(pWorkflow,pValue,false),0);
}

XUI_API int xuiWorkflowLoadXValueShared(xui_workflow workflow, xvalue* input)
{
	if (__xuiWorkflowModelBusy(workflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(workflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
    return __xuiWorkflowOperationEnd(&__ownedCall,__xuiWorkflowLoadXValue(workflow,input,true),0);
}

XUI_API int xuiWorkflowSaveXSONFile(xui_workflow pWorkflow, const char* sPath)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	xvalue* pValue;
	int iRet;

	if ( (pWorkflow == NULL) || (sPath == NULL) || (sPath[0] == 0) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	iRet = xuiWorkflowToXValue(pWorkflow, &pValue);
	if ( iRet != XUI_OK ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
	}
	iRet = xrtXlonStringifyFile(sPath, pValue, true) ? XUI_OK : XUI_ERROR_RESOURCE_FAILED;
	__xuiWorkflowValueRelease(pWorkflow, pValue);
	return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
}

XUI_API int xuiWorkflowLoadXSONFile(xui_workflow pWorkflow, const char* sPath)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	xvalue* pValue;
	int iRet;

	if ( (pWorkflow == NULL) || (sPath == NULL) || (sPath[0] == 0) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	pValue = xrtXlonParseFile(sPath);
	if ( pValue == NULL || xuiXrtValueType(pValue) == XVALUE_NULL ) {
		if ( pValue != NULL ) __xuiWorkflowValueRelease(pWorkflow, pValue);
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_RESOURCE_FAILED), 0);
	}
	iRet = xuiWorkflowLoadXValue(pWorkflow, pValue);
	__xuiWorkflowValueRelease(pWorkflow, pValue);
	return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
}

static int __xuiWorkflowNodeRunStateValid(int iState)
{
	return (iState >= XUI_WORKFLOW_NODE_RUN_IDLE && iState <= XUI_WORKFLOW_NODE_RUN_WARNING);
}

static int __xuiWorkflowEdgeRunStateValid(int iState)
{
	return (iState >= XUI_WORKFLOW_EDGE_RUN_IDLE && iState <= XUI_WORKFLOW_EDGE_RUN_INVALID);
}

XUI_API int xuiWorkflowSetNodeRunState(xui_workflow pWorkflow, const xui_workflow_node_run_state_t* pState)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	if ( (pWorkflow == NULL) || (pState == NULL) || (pState->iSize < sizeof(*pState)) ||
	     (pState->sNodeId == NULL) || (pState->sNodeId[0] == 0) || !__xuiWorkflowNodeRunStateValid(pState->iState) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	if ( xuiFlowGraphFindNode(pWorkflow->pGraph, pState->sNodeId) < 0 ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	return __xuiWorkflowOperationEnd(&__ownedCall, (xuiFlowGraphSetNodeRunState(pWorkflow->pGraph, pState->sNodeId, pState->iState, pState->sPreview)), 0);
}

XUI_API int xuiWorkflowGetNodeRunState(xui_workflow pWorkflow, const char* sNodeId, xui_workflow_node_run_state_t* pState)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	xui_flow_node_info_t tNode;
	int iIndex;
	int iRet;

	if ( (pWorkflow == NULL) || (sNodeId == NULL) || (sNodeId[0] == 0) || (pState == NULL) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	iIndex = xuiFlowGraphFindNode(pWorkflow->pGraph, sNodeId);
	if ( iIndex < 0 ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	iRet = xuiFlowGraphGetNode(pWorkflow->pGraph, iIndex, &tNode);
	if ( iRet != XUI_OK ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
	}
	memset(pState, 0, sizeof(*pState));
	pState->iSize = sizeof(*pState);
	pState->sNodeId = tNode.sId;
	pState->iState = tNode.iRunState;
	pState->sPreview = tNode.sRunPreview;
	return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_OK), 0);
}

XUI_API int xuiWorkflowSetEdgeRunState(xui_workflow pWorkflow, const xui_workflow_edge_run_state_t* pState)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	if ( (pWorkflow == NULL) || (pState == NULL) || (pState->iSize < sizeof(*pState)) ||
	     (pState->sEdgeId == NULL) || (pState->sEdgeId[0] == 0) || !__xuiWorkflowEdgeRunStateValid(pState->iState) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	if ( xuiFlowGraphFindEdge(pWorkflow->pGraph, pState->sEdgeId) < 0 ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	return __xuiWorkflowOperationEnd(&__ownedCall, (xuiFlowGraphSetEdgeRunState(pWorkflow->pGraph, pState->sEdgeId, pState->iState, pState->sPreview)), 0);
}

XUI_API int xuiWorkflowGetEdgeRunState(xui_workflow pWorkflow, const char* sEdgeId, xui_workflow_edge_run_state_t* pState)
{
	if (__xuiWorkflowModelBusy(pWorkflow)) return XUI_ERROR_BUSY;
    xui_workflow_operation_t __ownedCall={0};
    int __ownedStatus=__xuiWorkflowOperationBegin(pWorkflow,&__ownedCall);
    if(__ownedStatus!=XUI_OK)return __ownedStatus;
	xui_flow_edge_info_t tEdge;
	int iIndex;
	int iRet;

	if ( (pWorkflow == NULL) || (sEdgeId == NULL) || (sEdgeId[0] == 0) || (pState == NULL) ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	iIndex = xuiFlowGraphFindEdge(pWorkflow->pGraph, sEdgeId);
	if ( iIndex < 0 ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_ERROR_INVALID_ARGUMENT), 0);
	}
	memset(&tEdge, 0, sizeof(tEdge));
	tEdge.iSize = sizeof(tEdge);
	iRet = xuiFlowGraphGetEdge(pWorkflow->pGraph, iIndex, &tEdge);
	if ( iRet != XUI_OK ) {
		return __xuiWorkflowOperationEnd(&__ownedCall, (iRet), 0);
	}
	memset(pState, 0, sizeof(*pState));
	pState->iSize = sizeof(*pState);
	pState->sEdgeId = tEdge.sId;
	pState->iState = tEdge.iRunState;
	pState->sPreview = tEdge.sRunPreview;
	return __xuiWorkflowOperationEnd(&__ownedCall, (XUI_OK), 0);
}

#include "xui_workflow_owned.inl"

#endif
