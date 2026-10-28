#include "xui.h"
#include "src/xui/core/xui_xrt_port.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_workflow_schema_test failed: %s\n", (msg)); \
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

static int make_dynamic_branch_ports(xui_workflow pWorkflow, const char* sNodeId, const char* sType, xvalue* pConfig, xui_flow_port_desc_t* pPorts, int iPortCapacity, int* pPortCount, void* pUser)
{
	xvalue* pBranches;
	const char* sBranch;
	uint32_t i;
	uint32_t iCount;

	(void)pWorkflow;
	(void)sNodeId;
	(void)sType;
	(void)pUser;
	if ( (pPorts == NULL) || (pPortCount == NULL) || (iPortCapacity < 2) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pConfig != NULL && xuiXrtValueGetBool(xuiXrtValueObjectGet(pConfig, "rejectPorts", 11)) ) {
		return XUI_ERROR_RESOURCE_FAILED;
	}
	*pPortCount = 0;
	pBranches = (pConfig != NULL && xuiXrtValueIsObject(pConfig)) ? xuiXrtValueObjectGet(pConfig, "branches", 8) : NULL;
	if ( xuiXrtValueIsArray(pBranches) ) {
		iCount = xrtValueCount(pBranches);
		for ( i = 0u; i < iCount && *pPortCount < iPortCapacity; ++i ) {
			sBranch = xuiXrtValueGetText(xuiXrtValueArrayGet(pBranches, i));
			if ( sBranch == NULL || sBranch[0] == 0 ) {
				continue;
			}
			make_control_port(&pPorts[*pPortCount], sBranch, XUI_FLOW_PORT_OUTPUT);
			(*pPortCount)++;
		}
	}
	if ( *pPortCount == 0 ) {
		make_control_port(&pPorts[0], "true", XUI_FLOW_PORT_OUTPUT);
		make_control_port(&pPorts[1], "false", XUI_FLOW_PORT_OUTPUT);
		*pPortCount = 2;
	}
	return XUI_OK;
}

static int validate_llm_manual_mode(xui_workflow pWorkflow, const char* sNodeId, const char* sType, xvalue* pConfig, xui_flow_diagnostic_desc_t* pDiagnostics, int iDiagnosticCapacity, int* pDiagnosticCount, void* pUser)
{
	const char* sMode;

	(void)pWorkflow;
	(void)sNodeId;
	(void)sType;
	(void)pUser;
	if ( (pDiagnosticCount == NULL) || (iDiagnosticCapacity < 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pDiagnosticCount = 0;
	sMode = (pConfig != NULL && xuiXrtValueIsObject(pConfig)) ? (const char*)xuiXrtValueObjectGetText(pConfig, "mode", 4) : NULL;
	if ( sMode == NULL || strcmp(sMode, "manual") != 0 ) {
		return XUI_OK;
	}
	if ( pDiagnostics != NULL && iDiagnosticCapacity > 0 ) {
		memset(&pDiagnostics[0], 0, sizeof(pDiagnostics[0]));
		pDiagnostics[0].iSize = sizeof(pDiagnostics[0]);
		pDiagnostics[0].iSeverity = XUI_CODE_DIAGNOSTIC_WARNING;
		pDiagnostics[0].sCode = "llm.manual_mode";
		pDiagnostics[0].sMessage = "Manual mode requires host review.";
		pDiagnostics[0].sPath = "mode";
	}
	*pDiagnosticCount = 1;
	return XUI_OK;
}

int main(void)
{
	xui_workflow pWorkflow;
	xui_flow_graph pGraph;
	xui_workflow_node_type_desc_t tType;
	xui_workflow_node_library_item_t tLibraryItem;
	xui_flow_node_info_t tNodeInfo;
	xui_flow_edge_info_t tEdgeInfo;
	xui_flow_edge_desc_t tInvalidEdge;
	xui_flow_diagnostic_desc_t tDiagnostic;
	xui_flow_port_desc_t tStartOut;
	xui_flow_port_desc_t tLlmIn;
	xui_flow_port_desc_t tLlmOut;
	xui_flow_port_desc_t tBadIn;
	xui_flow_port_desc_t tBadOut;
	xui_flow_port_desc_t tConditionIn;
	xui_flow_port_desc_t tConditionTrue;
	xui_flow_port_desc_t tConditionFalse;
	xui_flow_port_desc_t tDynamicIn;
	xui_flow_port_desc_t tEndIn;
	xui_flow_port_desc_t tDataOut;
	xui_flow_port_desc_t tDataIn;
	xui_workflow_variable_desc_t tVariable;
	xui_workflow_config_field_desc_t tField;
	xui_workflow_config_diagnostic_t arrDiagnostics[4];
	xvalue* pSchema;
	xvalue* pManySchema;
	xvalue* pDefaultValue;
	xvalue* pDefaultConfig;
	xvalue* pNodeConfig;
	xvalue* pBadConfig;
	xvalue* pDynamicConfig;
	xvalue* pBranches;
	xvalue* pFieldDefault;
	xvalue* pOptions;
	xvalue* pChildren;
	int iType;
	int iStart;
	int iLlm;
	int iCondition;
	int iDynamic;
	int iEnd;
	int iVariable;
	int iDiagnostics;
	int i;
	int bFoundDirectionDiagnostic;
	int bFoundUnreachableDiagnostic;
	int bFoundCycleDiagnostic;
	int bFoundRequiredPortDiagnostic;
	int bFoundMultiplePortDiagnostic;
	int iRequiredConfigDiagnostics;
	int iRet;
	int iFailed;
	char sId[64];
	char sFieldId[32];
	char sTinyId[2];

	pWorkflow = NULL;
	pSchema = NULL;
	pManySchema = NULL;
	pDefaultValue = NULL;
	pDefaultConfig = NULL;
	pNodeConfig = NULL;
	pBadConfig = NULL;
	pDynamicConfig = NULL;
	pBranches = NULL;
	pFieldDefault = NULL;
	pOptions = NULL;
	pChildren = NULL;
	iFailed = 0;

	iRet = xuiWorkflowCreate(&pWorkflow);
	XUI_TEST_CHECK(iRet == XUI_OK && pWorkflow != NULL, "create workflow");
	XUI_TEST_CHECK(xuiWorkflowGetNodeTypeCount(pWorkflow) == 0, "empty type count");
	XUI_TEST_CHECK(xuiWorkflowGetNodeCount(pWorkflow) == 0, "empty node count");

	make_control_port(&tStartOut, "out", XUI_FLOW_PORT_OUTPUT);
	memset(&tType, 0, sizeof(tType));
	tType.iSize = sizeof(tType);
	tType.sId = "start";
	tType.iVersion = 1;
	tType.sTitle = "Start";
	tType.sCategory = "Flow";
	tType.pOutputs = &tStartOut;
	tType.iOutputCount = 1;
	iRet = xuiWorkflowRegisterNodeType(pWorkflow, &tType, &iType);
	XUI_TEST_CHECK(iRet == XUI_OK && iType == 0, "register start");
	XUI_TEST_CHECK(xuiWorkflowFindNodeType(pWorkflow, "start") == 0, "find start type");

	make_control_port(&tLlmIn, "in", XUI_FLOW_PORT_INPUT);
	tLlmIn.bMulti = 1;
	make_control_port(&tLlmOut, "out", XUI_FLOW_PORT_OUTPUT);
	memset(&tType, 0, sizeof(tType));
	tType.iSize = sizeof(tType);
	tType.sId = "bad.direction";
	tType.pInputs = &tLlmOut;
	tType.iInputCount = 1;
	iType = 99;
	iRet = xuiWorkflowRegisterNodeType(pWorkflow, &tType, &iType);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iType == -1 && xuiWorkflowGetNodeTypeCount(pWorkflow) == 1, "invalid port direction rejected at registration");
	make_control_port(&tBadIn, "same", XUI_FLOW_PORT_INPUT);
	make_control_port(&tBadOut, "same", XUI_FLOW_PORT_OUTPUT);
	memset(&tType, 0, sizeof(tType));
	tType.iSize = sizeof(tType);
	tType.sId = "bad.duplicate_port";
	tType.pInputs = &tBadIn;
	tType.iInputCount = 1;
	tType.pOutputs = &tBadOut;
	tType.iOutputCount = 1;
	iRet = xuiWorkflowRegisterNodeType(pWorkflow, &tType, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_ALREADY_INITIALIZED && xuiWorkflowGetNodeTypeCount(pWorkflow) == 1, "duplicate input output port id rejected at registration");
	iRet = xuiWorkflowConfigSchemaCreate(&pSchema);
	XUI_TEST_CHECK(iRet == XUI_OK && pSchema != NULL, "create config schema");
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "model";
	tField.sTitle = "Model";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_STRING;
	tField.bRequired = 1;
	tField.bHasDefault = 1;
	pFieldDefault = xuiXrtValueCreateText("glm", 3, FALSE);
	XUI_TEST_CHECK(pFieldDefault != NULL, "create field default");
	tField.pDefaultValue = pFieldDefault;
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add string config field");
	xrtValueRelease(pFieldDefault);
	pFieldDefault = NULL;
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "temperature";
	tField.sTitle = "Temperature";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_FLOAT;
	tField.bHasDefault = 1;
	tField.bHasMin = 1;
	tField.fMin = 0.0;
	tField.bHasMax = 1;
	tField.fMax = 2.0;
	pFieldDefault = xrtValueFloat(0.7);
	XUI_TEST_CHECK(pFieldDefault != NULL, "create float field default");
	tField.pDefaultValue = pFieldDefault;
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add float config field");
	xrtValueRelease(pFieldDefault);
	pFieldDefault = NULL;
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "systemPrompt";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_TEXTAREA;
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add textarea config field");
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "enabled";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_BOOL;
	tField.bHasDefault = 1;
	pFieldDefault = xrtValueBool(TRUE);
	XUI_TEST_CHECK(pFieldDefault != NULL, "create bool field default");
	tField.pDefaultValue = pFieldDefault;
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add bool config field");
	xrtValueRelease(pFieldDefault);
	pFieldDefault = NULL;
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "condition";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_EXPRESSION;
	tField.sExpressionLanguage = "xexpr";
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add expression config field");
	pOptions = xrtValueArray();
	XUI_TEST_CHECK(pOptions != NULL, "create select options");
	xuiXrtValueArrayAppendText(pOptions, "auto", 4, FALSE);
	xuiXrtValueArrayAppendText(pOptions, "manual", 6, FALSE);
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "mode";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_SELECT;
	tField.bHasDefault = 1;
	tField.pOptions = pOptions;
	pFieldDefault = xuiXrtValueCreateText("auto", 4, FALSE);
	XUI_TEST_CHECK(pFieldDefault != NULL, "create select default");
	tField.pDefaultValue = pFieldDefault;
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add select config field");
	xrtValueRelease(pFieldDefault);
	pFieldDefault = NULL;
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "tags";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_MULTI_SELECT;
	tField.pOptions = pOptions;
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add multi-select config field");
	xrtValueRelease(pOptions);
	pOptions = NULL;
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "inputVar";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_VARIABLE_REF;
	tField.sRefScope = "workflow";
	tField.sRefType = "string";
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add variable ref config field");
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "sourceOutput";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_NODE_OUTPUT_REF;
	tField.sRefType = "text";
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add node output ref config field");
	pChildren = xrtValueArray();
	XUI_TEST_CHECK(pChildren != NULL, "create children metadata");
	xuiXrtValueArrayAppendText(pChildren, "child", 5, FALSE);
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "payload";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_OBJECT;
	tField.pChildren = pChildren;
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add object config field");
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "items";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_ARRAY;
	tField.pChildren = pChildren;
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add array config field");
	xrtValueRelease(pChildren);
	pChildren = NULL;
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "advanced";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_GROUP;
	tField.sGroup = "advanced";
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add group config field");
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "tabs";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_TABS;
	tField.sTab = "main";
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add tabs config field");
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "branches";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_CONDITION_BUILDER;
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add condition builder config field");
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "mapping";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_MAPPING_BUILDER;
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "add mapping builder config field");
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "model";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_STRING;
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_ERROR_ALREADY_INITIALIZED, "duplicate config field rejected");
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "invalidRange";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_FLOAT;
	tField.bHasMin = 1;
	tField.fMin = 2.0;
	tField.bHasMax = 1;
	tField.fMax = 1.0;
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "inverted config range rejected");
	memset(&tType, 0, sizeof(tType));
	tType.iSize = sizeof(tType);
	tType.sId = "llm.chat";
	tType.iVersion = 1;
	tType.sTitle = "LLM Chat";
	tType.sCategory = "AI";
	tType.pInputs = &tLlmIn;
	tType.iInputCount = 1;
	tType.pOutputs = &tLlmOut;
	tType.iOutputCount = 1;
	tType.pConfigSchema = pSchema;
	tType.onValidate = validate_llm_manual_mode;
	iRet = xuiWorkflowRegisterNodeType(pWorkflow, &tType, &iType);
	XUI_TEST_CHECK(iRet == XUI_OK && iType == 1, "register llm");
	memset(&tField, 0, sizeof(tField));
	tField.iSize = sizeof(tField);
	tField.sId = "lateExternalField";
	tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_STRING;
	iRet = xuiWorkflowConfigSchemaAddField(pSchema, &tField);
	XUI_TEST_CHECK(iRet == XUI_OK, "mutate caller schema after registration");
	iRet = xuiWorkflowCreateDefaultConfig(pWorkflow, "llm.chat", &pDefaultConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && pDefaultConfig != NULL, "create default config");
	XUI_TEST_CHECK(xuiXrtValueType(xuiXrtValueObjectGet(pDefaultConfig, "lateExternalField", 17)) == XVALUE_NULL, "registered schema isolated from caller mutation");
	XUI_TEST_CHECK(strcmp((const char*)xuiXrtValueObjectGetText(pDefaultConfig, "model", 5), "glm") == 0, "default string config");
	XUI_TEST_CHECK(xuiXrtValueObjectGetFloat(pDefaultConfig, "temperature", 11) == 0.7, "default float config");
	XUI_TEST_CHECK(xuiXrtValueObjectGetBool(pDefaultConfig, "enabled", 7), "default bool config");
	XUI_TEST_CHECK(xuiXrtValueType(xuiXrtValueObjectGet(pDefaultConfig, "condition", 9)) == XVALUE_STRING, "expression stored as text");
	XUI_TEST_CHECK(strcmp((const char*)xuiXrtValueObjectGetText(pDefaultConfig, "mode", 4), "auto") == 0, "default select config");
	XUI_TEST_CHECK(xuiXrtValueType(xuiXrtValueObjectGet(pDefaultConfig, "tags", 4)) == XVALUE_ARRAY, "default multi-select config");
	XUI_TEST_CHECK(xuiXrtValueType(xuiXrtValueObjectGet(pDefaultConfig, "inputVar", 8)) == XVALUE_STRING, "default variable ref config");
	XUI_TEST_CHECK(xuiXrtValueType(xuiXrtValueObjectGet(pDefaultConfig, "sourceOutput", 12)) == XVALUE_STRING, "default output ref config");
	XUI_TEST_CHECK(xuiXrtValueType(xuiXrtValueObjectGet(pDefaultConfig, "payload", 7)) == XVALUE_OBJECT, "default object config");
	XUI_TEST_CHECK(xuiXrtValueType(xuiXrtValueObjectGet(pDefaultConfig, "items", 5)) == XVALUE_ARRAY, "default array config");
	XUI_TEST_CHECK(xuiXrtValueType(xuiXrtValueObjectGet(pDefaultConfig, "advanced", 8)) == XVALUE_OBJECT, "default group config");
	XUI_TEST_CHECK(xuiXrtValueType(xuiXrtValueObjectGet(pDefaultConfig, "tabs", 4)) == XVALUE_OBJECT, "default tabs config");
	XUI_TEST_CHECK(xuiXrtValueType(xuiXrtValueObjectGet(pDefaultConfig, "branches", 8)) == XVALUE_OBJECT, "default condition builder config");
	XUI_TEST_CHECK(xuiXrtValueType(xuiXrtValueObjectGet(pDefaultConfig, "mapping", 7)) == XVALUE_OBJECT, "default mapping builder config");
	iRet = xuiWorkflowValidateConfig(pWorkflow, "llm.chat", pDefaultConfig, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK && iDiagnostics == 0, "default config validates");
	pBadConfig = xrtValueDeepClone(pDefaultConfig);
	XUI_TEST_CHECK(pBadConfig != NULL, "copy bad config");
	xuiXrtValueObjectSetText(pBadConfig, "model", 5, "", 0, FALSE);
	xuiXrtValueObjectSetFloat(pBadConfig, "temperature", 11, 3.0);
	iRet = xuiWorkflowValidateConfigEx(pWorkflow, "llm.chat", pBadConfig, arrDiagnostics, 4, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK && iDiagnostics == 2, "required and range diagnostics");
	XUI_TEST_CHECK(strcmp(arrDiagnostics[0].sPath, "model") == 0 && strcmp(arrDiagnostics[0].sCode, "required") == 0, "required diagnostic path");
	XUI_TEST_CHECK(strcmp(arrDiagnostics[1].sPath, "temperature") == 0 && strcmp(arrDiagnostics[1].sCode, "range.max") == 0, "range diagnostic path");
	xuiXrtValueObjectSetText(pBadConfig, "model", 5, "glm", 3, FALSE);
	xuiXrtValueObjectSetText(pBadConfig, "condition", 9, "${input.user}", 13, FALSE);
	iRet = xuiWorkflowValidateConfig(pWorkflow, "llm.chat", pBadConfig, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK && iDiagnostics == 1, "expression text is not evaluated");
	xuiXrtValueObjectSetFloat(pBadConfig, "temperature", 11, 0.7);
	xuiXrtValueObjectSetText(pBadConfig, "tags", 4, "not-array", 9, FALSE);
	xuiXrtValueObjectSetText(pBadConfig, "payload", 7, "not-object", 10, FALSE);
	iRet = xuiWorkflowValidateConfig(pWorkflow, "llm.chat", pBadConfig, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK && iDiagnostics == 2, "complex field type diagnostics");
	xuiXrtValueObjectSetFloat(pBadConfig, "temperature", 11, NAN);
	iRet = xuiWorkflowValidateConfigEx(pWorkflow, "llm.chat", pBadConfig, arrDiagnostics, 4, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK && iDiagnostics == 3 && strcmp(arrDiagnostics[0].sCode, "number.finite") == 0, "non-finite config number diagnosed");
	xuiXrtValueObjectSetFloat(pBadConfig, "temperature", 11, 0.7);

	iRet = xuiWorkflowRegisterNodeType(pWorkflow, &tType, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_ALREADY_INITIALIZED, "duplicate type rejected");

	make_control_port(&tConditionIn, "in", XUI_FLOW_PORT_INPUT);
	make_control_port(&tConditionTrue, "true", XUI_FLOW_PORT_OUTPUT);
	make_control_port(&tConditionFalse, "false", XUI_FLOW_PORT_OUTPUT);
	{
		xui_flow_port_desc_t arrConditionOutputs[2];
		arrConditionOutputs[0] = tConditionTrue;
		arrConditionOutputs[1] = tConditionFalse;
		memset(&tType, 0, sizeof(tType));
		tType.iSize = sizeof(tType);
		tType.sId = "condition";
		tType.iVersion = 1;
		tType.sTitle = "Condition";
		tType.sCategory = "Flow";
		tType.pInputs = &tConditionIn;
		tType.iInputCount = 1;
		tType.pOutputs = arrConditionOutputs;
		tType.iOutputCount = 2;
		iRet = xuiWorkflowRegisterNodeType(pWorkflow, &tType, NULL);
		XUI_TEST_CHECK(iRet == XUI_OK, "register condition");
	}

	make_control_port(&tEndIn, "in", XUI_FLOW_PORT_INPUT);
	memset(&tType, 0, sizeof(tType));
	tType.iSize = sizeof(tType);
	tType.sId = "end";
	tType.iVersion = 1;
	tType.sTitle = "End";
	tType.sCategory = "Flow";
	tType.pInputs = &tEndIn;
	tType.iInputCount = 1;
	iRet = xuiWorkflowRegisterNodeType(pWorkflow, &tType, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "register end");
	make_control_port(&tDynamicIn, "in", XUI_FLOW_PORT_INPUT);
	memset(&tType, 0, sizeof(tType));
	tType.iSize = sizeof(tType);
	tType.sId = "dynamic.condition";
	tType.iVersion = 1;
	tType.sTitle = "Dynamic Condition";
	tType.sCategory = "Flow";
	tType.pInputs = &tDynamicIn;
	tType.iInputCount = 1;
	tType.onDynamicPorts = make_dynamic_branch_ports;
	iRet = xuiWorkflowRegisterNodeType(pWorkflow, &tType, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "register dynamic condition");
	XUI_TEST_CHECK(xuiWorkflowGetNodeTypeCount(pWorkflow) == 5, "type count");
	XUI_TEST_CHECK(xuiWorkflowGetNodeLibraryCount(pWorkflow) == 5, "node library count");
	memset(&tLibraryItem, 0, sizeof(tLibraryItem));
	iRet = xuiWorkflowGetNodeLibraryItem(pWorkflow, 1, &tLibraryItem);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tLibraryItem.sId, "llm.chat") == 0 && tLibraryItem.iInputCount == 1 && tLibraryItem.iOutputCount == 1, "node library item");

	iRet = xuiWorkflowAddNode(pWorkflow, "start", "n_start", NULL, 80.0f, 120.0f, &iStart);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == 0, "add start node");
	iRet = xuiWorkflowAddNode(pWorkflow, "llm.chat", "n_llm", "Ask", 320.0f, 120.0f, &iLlm);
	XUI_TEST_CHECK(iRet == XUI_OK && iLlm == 1, "add llm node");
	iRet = xuiWorkflowAddNode(pWorkflow, "condition", "n_condition", NULL, 560.0f, 120.0f, &iCondition);
	XUI_TEST_CHECK(iRet == XUI_OK && iCondition == 2, "add condition node");
	iRet = xuiWorkflowAddNode(pWorkflow, "end", "n_end", NULL, 800.0f, 120.0f, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iEnd == 3, "add end node");
	iRet = xuiWorkflowAddNode(pWorkflow, "dynamic.condition", "n_dynamic", NULL, 1040.0f, 120.0f, &iDynamic);
	XUI_TEST_CHECK(iRet == XUI_OK && iDynamic == 4, "add dynamic condition node");
	XUI_TEST_CHECK(xuiWorkflowGetNodeCount(pWorkflow) == 5, "workflow node count");
	iRet = xuiWorkflowGetNodeConfig(pWorkflow, "n_llm", &pNodeConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && pNodeConfig != NULL, "get node default config");
	XUI_TEST_CHECK(strcmp((const char*)xuiXrtValueObjectGetText(pNodeConfig, "model", 5), "glm") == 0, "node has default config");
	xrtValueRelease(pNodeConfig);
	pNodeConfig = NULL;
	iRet = xuiWorkflowSetNodeConfig(pWorkflow, "n_llm", pDefaultConfig);
	XUI_TEST_CHECK(iRet == XUI_OK, "set valid node config");
	iRet = xuiWorkflowSetNodeConfig(pWorkflow, "n_llm", pBadConfig);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject invalid node config");
	iRet = xuiWorkflowSelectNode(pWorkflow, "n_llm", 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "select workflow node");
	memset(&tNodeInfo, 0, sizeof(tNodeInfo));
	iRet = xuiWorkflowGetSelectedNode(pWorkflow, &tNodeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tNodeInfo.sId, "n_llm") == 0, "get selected workflow node");
	XUI_TEST_CHECK(tNodeInfo.sSummary != NULL && strstr(tNodeInfo.sSummary, "model=glm") != NULL, "node visual summary");
	if ( pNodeConfig != NULL ) {
		xrtValueRelease(pNodeConfig);
		pNodeConfig = NULL;
	}
	pNodeConfig = xrtValueDeepClone(pDefaultConfig);
	XUI_TEST_CHECK(pNodeConfig != NULL, "copy config for graph validation");
	xuiXrtValueObjectSetText(pNodeConfig, "model", 5, "", 0, FALSE);
	xuiXrtValueObjectSetText(pNodeConfig, "inputVar", 8, "missing_var", 11, FALSE);
	xuiXrtValueObjectSetText(pNodeConfig, "condition", 9, "${input.user", 12, FALSE);
	xuiXrtValueObjectSetText(pNodeConfig, "mode", 4, "manual", 6, FALSE);
	iRet = xuiFlowGraphSetNodeConfig(xuiWorkflowGetGraph(pWorkflow), "n_llm", pNodeConfig);
	XUI_TEST_CHECK(iRet == XUI_OK, "set invalid config directly for graph validation");
	iRet = xuiWorkflowValidateGraph(pWorkflow, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK && iDiagnostics >= 4 && xuiFlowGraphGetDiagnosticCount(xuiWorkflowGetGraph(pWorkflow)) == iDiagnostics, "graph config variable expression host diagnostics");
	iRet = xuiFlowGraphSetNodeConfig(xuiWorkflowGetGraph(pWorkflow), "n_llm", pDefaultConfig);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore valid graph config");
	xrtValueRelease(pNodeConfig);
	pNodeConfig = NULL;

	pGraph = xuiWorkflowGetGraph(pWorkflow);
	XUI_TEST_CHECK(pGraph != NULL, "workflow graph");
	XUI_TEST_CHECK(xuiFlowGraphGetNodePortCount(pGraph, iStart) == 1, "start static ports copied");
	XUI_TEST_CHECK(xuiFlowGraphGetNodePortCount(pGraph, iLlm) == 2, "llm static ports copied");
	XUI_TEST_CHECK(xuiFlowGraphGetNodePortCount(pGraph, iCondition) == 3, "condition static ports copied");
	XUI_TEST_CHECK(xuiFlowGraphGetNodePortCount(pGraph, iEnd) == 1, "end static ports copied");
	XUI_TEST_CHECK(xuiFlowGraphGetNodePortCount(pGraph, iDynamic) == 3, "dynamic ports merged");

	iRet = xuiWorkflowConnect(pWorkflow, "e_start_llm", "n_start", "out", "n_llm", "in", NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "connect start to llm");
	iRet = xuiWorkflowConnect(pWorkflow, "e_llm_condition", "n_llm", "out", "n_condition", "in", NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "connect llm to condition");
	iRet = xuiWorkflowConnect(pWorkflow, "e_condition_end", "n_condition", "true", "n_end", "in", NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "connect condition to end");
	XUI_TEST_CHECK(xuiFlowGraphGetEdgeCount(pGraph) == 3, "workflow edge count");
	iRet = xuiWorkflowRemoveNode(pWorkflow, "n_condition");
	XUI_TEST_CHECK(iRet == XUI_OK, "remove workflow node");
	XUI_TEST_CHECK(xuiWorkflowGetNodeCount(pWorkflow) == 4 && xuiFlowGraphGetEdgeCount(pGraph) == 1, "remove node cleans connected edges");
	iRet = xuiWorkflowConnect(pWorkflow, "e_dynamic_end", "n_dynamic", "true", "n_end", "in", NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "connect dynamic port");
	pDynamicConfig = xrtValueObject();
	XUI_TEST_CHECK(pDynamicConfig != NULL, "create dynamic config");
	pBranches = xrtValueArray();
	XUI_TEST_CHECK(pBranches != NULL, "create dynamic branches");
	xuiXrtValueArrayAppendText(pBranches, "true", 4, FALSE);
	xuiXrtValueArrayAppendText(pBranches, "maybe", 5, FALSE);
	XUI_TEST_CHECK(xuiXrtValueObjectSetTake(pDynamicConfig, "branches", 8, pBranches, TRUE), "set dynamic branches");
	pBranches = NULL;
	iRet = xuiWorkflowSetNodeConfig(pWorkflow, "n_dynamic", pDynamicConfig);
	iDynamic = xuiFlowGraphFindNode(pGraph, "n_dynamic");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetNodePortCount(pGraph, iDynamic) == 3, "dynamic ports rebuild from config");
	iRet = xuiFlowGraphGetEdge(pGraph, xuiFlowGraphFindEdge(pGraph, "e_dynamic_end"), &tEdgeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && !tEdgeInfo.bInvalid && tEdgeInfo.iFromPort >= 0, "stable dynamic edge preserved");
	pBranches = xrtValueArray();
	XUI_TEST_CHECK(pBranches != NULL, "create missing branch list");
	xuiXrtValueArrayAppendText(pBranches, "maybe", 5, FALSE);
	XUI_TEST_CHECK(xuiXrtValueObjectSetTake(pDynamicConfig, "branches", 8, pBranches, TRUE), "replace dynamic branches");
	pBranches = NULL;
	iRet = xuiWorkflowSetNodeConfig(pWorkflow, "n_dynamic", pDynamicConfig);
	iDynamic = xuiFlowGraphFindNode(pGraph, "n_dynamic");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiFlowGraphGetNodePortCount(pGraph, iDynamic) == 2, "dynamic ports remove missing branch");
	iRet = xuiFlowGraphGetEdge(pGraph, xuiFlowGraphFindEdge(pGraph, "e_dynamic_end"), &tEdgeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tEdgeInfo.bInvalid && tEdgeInfo.iFromPort < 0, "missing dynamic port marks edge invalid");
	pNodeConfig = xrtValueDeepClone(pDynamicConfig);
	XUI_TEST_CHECK(pNodeConfig != NULL, "clone dynamic config for callback failure");
	XUI_TEST_CHECK(xuiXrtValueObjectSetBool(pNodeConfig, "rejectPorts", 11, TRUE), "set dynamic callback rejection");
	iRet = xuiWorkflowSetNodeConfig(pWorkflow, "n_dynamic", pNodeConfig);
	XUI_TEST_CHECK(iRet == XUI_ERROR_RESOURCE_FAILED, "dynamic callback failure rejected before mutation");
	xrtValueRelease(pNodeConfig);
	pNodeConfig = NULL;
	iRet = xuiWorkflowGetNodeConfig(pWorkflow, "n_dynamic", &pNodeConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiXrtValueGetBool(xuiXrtValueObjectGet(pNodeConfig, "rejectPorts", 11)) &&
	               xrtValueCount(xuiXrtValueObjectGet(pNodeConfig, "branches", 8)) == 1, "callback failure preserves config");
	xrtValueRelease(pNodeConfig);
	pNodeConfig = xrtValueDeepClone(pDynamicConfig);
	XUI_TEST_CHECK(pNodeConfig != NULL, "clone dynamic config for duplicate ports");
	pBranches = xrtValueArray();
	XUI_TEST_CHECK(pBranches != NULL, "create duplicate dynamic branches");
	xuiXrtValueArrayAppendText(pBranches, "maybe", 5, FALSE);
	xuiXrtValueArrayAppendText(pBranches, "maybe", 5, FALSE);
	XUI_TEST_CHECK(xuiXrtValueObjectSetTake(pNodeConfig, "branches", 8, pBranches, TRUE), "set duplicate dynamic branches");
	pBranches = NULL;
	iRet = xuiWorkflowSetNodeConfig(pWorkflow, "n_dynamic", pNodeConfig);
	XUI_TEST_CHECK(iRet == XUI_ERROR_ALREADY_INITIALIZED, "duplicate dynamic ports rejected");
	xrtValueRelease(pNodeConfig);
	pNodeConfig = NULL;
	iRet = xuiWorkflowGetNodeConfig(pWorkflow, "n_dynamic", &pNodeConfig);
	XUI_TEST_CHECK(iRet == XUI_OK && xrtValueCount(xuiXrtValueObjectGet(pNodeConfig, "branches", 8)) == 1 &&
	               xuiFlowGraphGetNodePortCount(pGraph, iDynamic) == 2, "failed dynamic rebuild rolls back config and ports");
	xrtValueRelease(pNodeConfig);
	pNodeConfig = NULL;
	iRet = xuiWorkflowValidateGraph(pWorkflow, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK && iDiagnostics >= 1 && xuiFlowGraphGetDiagnosticCount(pGraph) == iDiagnostics, "missing dynamic port diagnostic");
	bFoundUnreachableDiagnostic = 0;
	for ( i = 0; i < xuiFlowGraphGetDiagnosticCount(pGraph); ++i ) {
		memset(&tDiagnostic, 0, sizeof(tDiagnostic));
		iRet = xuiFlowGraphGetDiagnostic(pGraph, i, &tDiagnostic);
		XUI_TEST_CHECK(iRet == XUI_OK, "read reachability diagnostic");
		if ( tDiagnostic.sCode != NULL && strcmp(tDiagnostic.sCode, "workflow.node.unreachable") == 0 ) {
			bFoundUnreachableDiagnostic = 1;
		}
	}
	XUI_TEST_CHECK(bFoundUnreachableDiagnostic, "disconnected workflow node diagnosed as unreachable");
	iRet = xuiWorkflowConnect(pWorkflow, "e_bad_direction", "n_llm", "in", "n_start", "out", NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject direction-invalid edge");
	memset(&tInvalidEdge, 0, sizeof(tInvalidEdge));
	tInvalidEdge.iSize = sizeof(tInvalidEdge);
	tInvalidEdge.sId = "e_bad_direction";
	tInvalidEdge.iKind = XUI_FLOW_PORT_CONTROL;
	tInvalidEdge.sFromNode = "n_llm";
	tInvalidEdge.sFromPort = "in";
	tInvalidEdge.sToNode = "n_start";
	tInvalidEdge.sToPort = "out";
	iRet = xuiFlowGraphAddEdgePreserveInvalid(pGraph, &tInvalidEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "preserve direction-invalid imported edge");
	iRet = xuiWorkflowValidateGraph(pWorkflow, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK && iDiagnostics >= 2, "edge direction diagnostic");
	bFoundDirectionDiagnostic = 0;
	for ( i = 0; i < xuiFlowGraphGetDiagnosticCount(pGraph); ++i ) {
		memset(&tDiagnostic, 0, sizeof(tDiagnostic));
		iRet = xuiFlowGraphGetDiagnostic(pGraph, i, &tDiagnostic);
		XUI_TEST_CHECK(iRet == XUI_OK, "read workflow diagnostic");
		if ( tDiagnostic.sCode != NULL && strcmp(tDiagnostic.sCode, "workflow.edge.direction") == 0 &&
		     tDiagnostic.sEdge != NULL && strcmp(tDiagnostic.sEdge, "e_bad_direction") == 0 ) {
			bFoundDirectionDiagnostic = 1;
		}
	}
	XUI_TEST_CHECK(bFoundDirectionDiagnostic, "direction-invalid edge classified accurately");
	iRet = xuiWorkflowConnect(pWorkflow, "e_cycle_a", "n_llm", "out", "n_dynamic", "in", NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "connect first cycle edge");
	iRet = xuiWorkflowConnect(pWorkflow, "e_cycle_b", "n_dynamic", "maybe", "n_llm", "in", NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "connect second cycle edge");
	iRet = xuiWorkflowValidateGraph(pWorkflow, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK, "validate control cycle");
	bFoundCycleDiagnostic = 0;
	for ( i = 0; i < xuiFlowGraphGetDiagnosticCount(pGraph); ++i ) {
		memset(&tDiagnostic, 0, sizeof(tDiagnostic));
		iRet = xuiFlowGraphGetDiagnostic(pGraph, i, &tDiagnostic);
		XUI_TEST_CHECK(iRet == XUI_OK, "read cycle diagnostic");
		if ( tDiagnostic.sCode != NULL && strcmp(tDiagnostic.sCode, "workflow.control.cycle") == 0 ) {
			bFoundCycleDiagnostic = 1;
		}
	}
	XUI_TEST_CHECK(bFoundCycleDiagnostic, "control cycle diagnosed");
	iRet = xuiWorkflowRemoveNode(pWorkflow, "n_start");
	XUI_TEST_CHECK(iRet == XUI_OK, "remove start for validation");
	iRet = xuiWorkflowRemoveNode(pWorkflow, "n_end");
	XUI_TEST_CHECK(iRet == XUI_OK, "remove end for validation");
	iRet = xuiWorkflowValidateGraph(pWorkflow, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK && iDiagnostics >= 2, "missing start end diagnostics");

	iRet = xuiWorkflowAddNode(pWorkflow, "missing", "n_missing", NULL, 0.0f, 0.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "missing type rejected");
	sTinyId[0] = 'x';
	sTinyId[1] = '\0';
	iRet = xuiWorkflowMakeVariableId(pWorkflow, sTinyId, (int)sizeof(sTinyId));
	XUI_TEST_CHECK(iRet == XUI_ERROR_LIMIT_EXCEEDED && sTinyId[0] == '\0', "small variable id buffer rejected");
	iRet = xuiWorkflowMakeVariableId(pWorkflow, sId, (int)sizeof(sId));
	XUI_TEST_CHECK(iRet == XUI_OK && strncmp(sId, "var_", 4) == 0, "make variable id");
	pDefaultValue = xrtValueObject();
	XUI_TEST_CHECK(pDefaultValue != NULL, "create variable default");
	XUI_TEST_CHECK(xuiXrtValueObjectSetText(pDefaultValue, "value", 5, "hello", 5, FALSE), "set variable default value");
	memset(&tVariable, 0, sizeof(tVariable));
	tVariable.iSize = sizeof(tVariable);
	tVariable.sId = sId;
	tVariable.sTitle = "User input";
	tVariable.sType = "string";
	tVariable.sScope = "workflow";
	tVariable.pDefaultValue = pDefaultValue;
	iRet = xuiWorkflowAddVariable(pWorkflow, &tVariable, &iVariable);
	XUI_TEST_CHECK(iRet == XUI_OK && iVariable == 0, "add variable");
	XUI_TEST_CHECK(xuiXrtValueObjectSetText(pDefaultValue, "value", 5, "changed", 7, FALSE), "mutate caller variable default");
	memset(&tVariable, 0, sizeof(tVariable));
	iRet = xuiWorkflowGetVariable(pWorkflow, iVariable, &tVariable);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp((const char*)xuiXrtValueObjectGetText(tVariable.pDefaultValue, "value", 5), "hello") == 0, "variable default isolated from caller mutation");
	XUI_TEST_CHECK(xuiWorkflowFindVariable(pWorkflow, sId) == 0, "find variable");
	XUI_TEST_CHECK(xuiWorkflowGetVariableCount(pWorkflow) == 1, "variable count");
	iVariable = 99;
	tVariable.iSize = sizeof(tVariable);
	iRet = xuiWorkflowAddVariable(pWorkflow, &tVariable, &iVariable);
	XUI_TEST_CHECK(iRet == XUI_ERROR_ALREADY_INITIALIZED && iVariable == -1, "duplicate variable rejected and output cleared");

	make_control_port(&tDataOut, "data", XUI_FLOW_PORT_OUTPUT);
	tDataOut.iKind = XUI_FLOW_PORT_DATA;
	memset(&tType, 0, sizeof(tType));
	tType.iSize = sizeof(tType);
	tType.sId = "data.source";
	tType.pOutputs = &tDataOut;
	tType.iOutputCount = 1;
	iRet = xuiWorkflowRegisterNodeType(pWorkflow, &tType, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "register data source");
	make_control_port(&tDataIn, "data", XUI_FLOW_PORT_INPUT);
	tDataIn.iKind = XUI_FLOW_PORT_DATA;
	tDataIn.bRequired = 1;
	memset(&tType, 0, sizeof(tType));
	tType.iSize = sizeof(tType);
	tType.sId = "data.target";
	tType.pInputs = &tDataIn;
	tType.iInputCount = 1;
	iRet = xuiWorkflowRegisterNodeType(pWorkflow, &tType, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "register data target");
	iRet = xuiWorkflowAddNode(pWorkflow, "data.source", "n_data_source", NULL, 0.0f, 0.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add data source");
	iRet = xuiWorkflowAddNode(pWorkflow, "data.target", "n_data_target", NULL, 0.0f, 0.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add data target");
	iRet = xuiWorkflowConnect(pWorkflow, "e_data", "n_data_source", "data", "n_data_target", "data", NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "connect data ports through workflow api");
	iRet = xuiFlowGraphGetEdge(pGraph, xuiFlowGraphFindEdge(pGraph, "e_data"), &tEdgeInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tEdgeInfo.iKind == XUI_FLOW_PORT_DATA, "workflow connection infers data edge kind");
	iRet = xuiWorkflowConnect(pWorkflow, "e_data_duplicate", "n_data_source", "data", "n_data_target", "data", NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject second connection to non-multi input");
	memset(&tInvalidEdge, 0, sizeof(tInvalidEdge));
	tInvalidEdge.iSize = sizeof(tInvalidEdge);
	tInvalidEdge.sId = "e_data_imported_duplicate";
	tInvalidEdge.iKind = XUI_FLOW_PORT_DATA;
	tInvalidEdge.sFromNode = "n_data_source";
	tInvalidEdge.sFromPort = "data";
	tInvalidEdge.sToNode = "n_data_target";
	tInvalidEdge.sToPort = "data";
	iRet = xuiFlowGraphAddEdgePreserveInvalid(pGraph, &tInvalidEdge, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "preserve imported duplicate input edge");
	iRet = xuiWorkflowValidateGraph(pWorkflow, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK, "validate imported duplicate input edge");
	bFoundMultiplePortDiagnostic = 0;
	for ( i = 0; i < xuiFlowGraphGetDiagnosticCount(pGraph); ++i ) {
		memset(&tDiagnostic, 0, sizeof(tDiagnostic));
		iRet = xuiFlowGraphGetDiagnostic(pGraph, i, &tDiagnostic);
		XUI_TEST_CHECK(iRet == XUI_OK, "read multiple input diagnostic");
		if ( tDiagnostic.sCode != NULL && strcmp(tDiagnostic.sCode, "workflow.port.multiple") == 0 &&
		     tDiagnostic.sNode != NULL && strcmp(tDiagnostic.sNode, "n_data_target") == 0 ) {
			bFoundMultiplePortDiagnostic = 1;
		}
	}
	XUI_TEST_CHECK(bFoundMultiplePortDiagnostic, "imported duplicate input edge diagnosed");
	iRet = xuiFlowGraphRemoveEdge(pGraph, "e_data_imported_duplicate");
	XUI_TEST_CHECK(iRet == XUI_OK, "remove imported duplicate input edge");
	iRet = xuiFlowGraphRemoveEdge(pGraph, "e_data");
	XUI_TEST_CHECK(iRet == XUI_OK, "remove required input connection");
	iRet = xuiWorkflowValidateGraph(pWorkflow, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK, "validate disconnected required input");
	bFoundRequiredPortDiagnostic = 0;
	for ( i = 0; i < xuiFlowGraphGetDiagnosticCount(pGraph); ++i ) {
		memset(&tDiagnostic, 0, sizeof(tDiagnostic));
		iRet = xuiFlowGraphGetDiagnostic(pGraph, i, &tDiagnostic);
		XUI_TEST_CHECK(iRet == XUI_OK, "read required port diagnostic");
		if ( tDiagnostic.sCode != NULL && strcmp(tDiagnostic.sCode, "workflow.port.required") == 0 &&
		     tDiagnostic.sNode != NULL && strcmp(tDiagnostic.sNode, "n_data_target") == 0 &&
		     tDiagnostic.sPath != NULL && strcmp(tDiagnostic.sPath, "data") == 0 ) {
			bFoundRequiredPortDiagnostic = 1;
		}
	}
	XUI_TEST_CHECK(bFoundRequiredPortDiagnostic, "required input port diagnosed when disconnected");

	iRet = xuiWorkflowConfigSchemaCreate(&pManySchema);
	XUI_TEST_CHECK(iRet == XUI_OK && pManySchema != NULL, "create large diagnostic schema");
	for ( i = 0; i < 20; ++i ) {
		snprintf(sFieldId, sizeof(sFieldId), "required_%02d", i);
		memset(&tField, 0, sizeof(tField));
		tField.iSize = sizeof(tField);
		tField.sId = sFieldId;
		tField.iKind = XUI_WORKFLOW_CONFIG_FIELD_STRING;
		tField.bRequired = 1;
		iRet = xuiWorkflowConfigSchemaAddField(pManySchema, &tField);
		XUI_TEST_CHECK(iRet == XUI_OK, "add large diagnostic schema field");
	}
	memset(&tType, 0, sizeof(tType));
	tType.iSize = sizeof(tType);
	tType.sId = "many.required";
	tType.pConfigSchema = pManySchema;
	iRet = xuiWorkflowRegisterNodeType(pWorkflow, &tType, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "register large diagnostic type");
	iRet = xuiWorkflowAddNode(pWorkflow, "many.required", "n_many_required", NULL, 0.0f, 0.0f, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "add large diagnostic node");
	iRet = xuiWorkflowValidateGraph(pWorkflow, &iDiagnostics);
	XUI_TEST_CHECK(iRet == XUI_OK, "validate more than sixteen config errors");
	iRequiredConfigDiagnostics = 0;
	for ( i = 0; i < xuiFlowGraphGetDiagnosticCount(pGraph); ++i ) {
		memset(&tDiagnostic, 0, sizeof(tDiagnostic));
		iRet = xuiFlowGraphGetDiagnostic(pGraph, i, &tDiagnostic);
		XUI_TEST_CHECK(iRet == XUI_OK, "read expanded config diagnostic");
		if ( tDiagnostic.sCode != NULL && strcmp(tDiagnostic.sCode, "required") == 0 &&
		     tDiagnostic.sNode != NULL && strcmp(tDiagnostic.sNode, "n_many_required") == 0 ) {
			iRequiredConfigDiagnostics++;
		}
	}
	XUI_TEST_CHECK(iRequiredConfigDiagnostics == 20, "graph validation preserves all config diagnostics");

cleanup:
	if ( pManySchema != NULL ) {
		xrtValueRelease(pManySchema);
	}
	if ( pChildren != NULL ) {
		xrtValueRelease(pChildren);
	}
	if ( pOptions != NULL ) {
		xrtValueRelease(pOptions);
	}
	if ( pFieldDefault != NULL ) {
		xrtValueRelease(pFieldDefault);
	}
	if ( pBadConfig != NULL ) {
		xrtValueRelease(pBadConfig);
	}
	if ( pBranches != NULL ) {
		xrtValueRelease(pBranches);
	}
	if ( pDynamicConfig != NULL ) {
		xrtValueRelease(pDynamicConfig);
	}
	if ( pNodeConfig != NULL ) {
		xrtValueRelease(pNodeConfig);
	}
	if ( pDefaultConfig != NULL ) {
		xrtValueRelease(pDefaultConfig);
	}
	if ( pDefaultValue != NULL ) {
		xrtValueRelease(pDefaultValue);
	}
	if ( pSchema != NULL ) {
		xrtValueRelease(pSchema);
	}
	xuiWorkflowDestroy(pWorkflow);
	if ( iFailed ) {
		return 1;
	}
	printf("xui_workflow_schema_test passed\n");
	return 0;
}

