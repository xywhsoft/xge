#define XRT_MODULE_ALL
#include "xui.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct callback_state {
	xui_workflow workflow;
	xui_flow_graph graph;
	int refs, holds, drops, edges, portCalls, validationCalls;
	bool failHold, failDrop, failPorts, failDiagnostic, ignoreBadPort;
	bool exhaustPorts, exhaustDiagnostics;
} callback_state;

static void check_reentry(callback_state* state)
{
	assert(xuiWorkflowGetNodeTypeCount(state->workflow) == XUI_ERROR_BUSY);
	assert(xuiWorkflowAddNode(state->workflow, "custom", "reentrant", NULL, 0, 0, NULL) == XUI_ERROR_BUSY);
	assert(xuiFlowGraphGetNodeCount(state->graph) == XUI_ERROR_BUSY);
	assert(xuiFlowGraphRemoveNode(state->graph, "node") == XUI_ERROR_BUSY);
}

static bool hold(const void* user)
{
	callback_state* state = (callback_state*)user;
	check_reentry(state);
	++state->refs;
	++state->holds;
	if (state->failHold) xrtSetErrorKind(XERR_RANGE);
	return true;
}

static bool drop(const void* user)
{
	callback_state* state = (callback_state*)user;
	check_reentry(state);
	assert(state->refs > 0);
	--state->refs;
	++state->drops;
	return !state->failDrop;
}

static bool count(const void* user, size_t* result)
{
	*result = (size_t)((const callback_state*)user)->refs;
	return true;
}

static bool trace(const void* user, xrtownershipvisitor visitor, void* context)
{
	(void)user;
	(void)visitor;
	(void)context;
	return true;
}

static const xrtownershipops operations = {count, trace};

static bool find_edge(xrtownershipref edge, void* user)
{
	callback_state* state = user;
	if (edge.Data != user) return true;
	assert(edge.Ops == &operations);
	++state->edges;
	return true;
}

static int write_ports(const xui_workflow_callback_node_t* node, xui_workflow_port_output_t* output, void* user)
{
	callback_state* state = user;
	check_reentry(state);
	assert(state->refs == 1);
	assert(strcmp(node->sType, "custom") == 0);
	xui_workflow_callback_config_t config = {0};
	assert(xuiWorkflowCallbackConfig(node, &config) == XUI_OK && config.pConfig);
	++state->portCalls;
	char id[24];
	xui_flow_port_desc_t port = {
		.iSize = sizeof(port), .sId = id,
		.iDirection = XUI_FLOW_PORT_OUTPUT, .iKind = XUI_FLOW_PORT_DATA,
	};
	/* More than the legacy initial buffer, using a reused stack string. */
	for (int i = 0; i < 40; ++i) {
		snprintf(id, sizeof(id), "result-%d", i);
		int status = xuiWorkflowPortOutputAppend(output, &port);
		if (status != XUI_OK) return status;
	}
	if (state->exhaustPorts) {
		snprintf(id, sizeof(id), "uncommitted");
		assert(xrtMemDebugFailAfter(0));
		assert(xuiWorkflowPortOutputAppend(output, &port) == XUI_ERROR_OUT_OF_MEMORY);
		xrtMemDebugFailClear();
	}
	memset(id, 'X', sizeof(id));
	if (state->ignoreBadPort) {
		assert(xuiWorkflowPortOutputAppend(output, NULL) == XUI_ERROR_INVALID_ARGUMENT);
		assert(xuiWorkflowPortOutputAppend(output, &port) == XUI_ERROR_INVALID_ARGUMENT);
	}
	if (state->failPorts) {
		xrtSetErrorKind(XERR_RANGE);
		return XUI_ERROR_CALLBACK_FAILED;
	}
	return XUI_OK;
}

static int write_diagnostics(const xui_workflow_callback_node_t* node, xui_workflow_diagnostic_output_t* output, void* user)
{
	callback_state* state = user;
	check_reentry(state);
	assert(state->refs == 1);
	assert(strcmp(node->sId, "node") == 0);
	++state->validationCalls;
	char message[] = "generated diagnostic";
	xui_flow_diagnostic_desc_t diagnostic = {
		.iSize = sizeof(diagnostic), .iSeverity = XUI_CODE_DIAGNOSTIC_WARNING,
		.sCode = "FROM_CALLBACK", .sMessage = message,
	};
	int status = xuiWorkflowDiagnosticOutputAppend(output, &diagnostic);
	memset(message, 'X', sizeof(message));
	if (status != XUI_OK) return status;
	if (state->failDiagnostic) {
		assert(xuiWorkflowDiagnosticOutputAppend(output, NULL) == XUI_ERROR_INVALID_ARGUMENT);
		assert(xuiWorkflowDiagnosticOutputAppend(output, &diagnostic) == XUI_ERROR_INVALID_ARGUMENT);
	}
	if (state->exhaustDiagnostics) {
		diagnostic.sMessage = "uncommitted";
		assert(xrtMemDebugFailAfter(1));
		assert(xuiWorkflowDiagnosticOutputAppend(output, &diagnostic) == XUI_ERROR_OUT_OF_MEMORY);
		xrtMemDebugFailClear();
	}
	return XUI_OK;
}

static int callback_diagnostics(xui_flow_graph graph)
{
	int count = 0;
	for (int i = 0; i < xuiFlowGraphGetDiagnosticCount(graph); ++i) {
		xui_flow_diagnostic_desc_t item = {.iSize = sizeof(item)};
		assert(xuiFlowGraphGetDiagnostic(graph, i, &item) == XUI_OK);
		if (strcmp(item.sCode, "FROM_CALLBACK") != 0) continue;
		assert(strcmp(item.sMessage, "generated diagnostic") == 0);
		assert(strcmp(item.sNode, "node") == 0);
		++count;
	}
	return count;
}

int main(void)
{
	callback_state state = {0};
	assert(xuiWorkflowCreateOwned(&state.workflow) == XUI_OK);
	assert(xuiWorkflowGetGraphOwned(state.workflow, &state.graph) == XUI_OK);
	xui_callback_owner_t owner = {{.Ops = &operations, .Data = &state}, hold, drop};
	xui_workflow_owned_node_type_desc_t desc = {
		.iSize = sizeof(desc), .sId = "custom", .iVersion = 1,
		.onDynamicPorts = write_ports, .onValidate = write_diagnostics, .pUser = &state,
	};
	int index = 0;
	state.failHold = true;
	assert(xuiWorkflowRegisterNodeTypeOwned(state.workflow, &desc, &owner, &index) == XUI_ERROR_CALLBACK_FAILED);
	assert(index == -1 && state.holds == 1 && state.drops == 1 && state.refs == 0);
	assert(xrtGetError());
	xrtClearError();
	state.failHold = false;
	assert(xuiWorkflowRegisterNodeTypeOwned(state.workflow, &desc, &owner, &index) == XUI_OK && index == 0);
	assert(state.refs == 1);
	assert(xuiWorkflowRegisterNodeTypeOwned(state.workflow, &desc, &owner, &index) == XUI_ERROR_ALREADY_INITIALIZED);
	assert(index == -1 && state.holds == 3 && state.drops == 2 && state.refs == 1);

	xrtownershipscope freeze = {0};
	xrtownershipref ref = xuiWorkflowOwnership(state.workflow);
	assert(xrtOwnershipFreezeTryBegin(&freeze));
	assert(ref.Ops->Trace(ref.Data, find_edge, &state) && state.edges == 1);
	assert(xrtOwnershipScopeEnd(&freeze));

	state.failPorts = true;
	assert(xuiWorkflowAddNode(state.workflow, "custom", "broken", NULL, 0, 0, NULL) == XUI_ERROR_CALLBACK_FAILED);
	assert(xrtGetError());
	xrtClearError();
	assert(xuiFlowGraphGetNodeCount(state.graph) == 0);
	state.failPorts = false;
	assert(xuiWorkflowAddNode(state.workflow, "custom", "node", NULL, 0, 0, &index) == XUI_OK);
	xui_flow_port_info_t port = {.iSize = sizeof(port)};
	assert(xuiFlowGraphGetPort(state.graph, index, 39, &port) == XUI_OK);
	assert(strcmp(port.sId, "result-39") == 0 && port.bDynamic);
	assert(state.portCalls == 2);

	xvalue* config = NULL;
	assert(xuiWorkflowGetNodeConfig(state.workflow, "node", &config) == XUI_OK);
	state.ignoreBadPort = true;
	assert(xuiWorkflowSetNodeConfigShared(state.workflow, "node", config) == XUI_ERROR_INVALID_ARGUMENT);
	assert(xuiFlowGraphGetPort(state.graph, index, 39, &port) == XUI_OK && strcmp(port.sId, "result-39") == 0);
	state.ignoreBadPort = false;
	state.exhaustPorts = true;
	assert(xuiWorkflowSetNodeConfigShared(state.workflow, "node", config) == XUI_ERROR_OUT_OF_MEMORY);
	xrtClearError();
	assert(xuiFlowGraphGetPort(state.graph, index, 39, &port) == XUI_OK && strcmp(port.sId, "result-39") == 0);
	state.exhaustPorts = false;
	assert(xuiWorkflowSetNodeConfigShared(state.workflow, "node", config) == XUI_OK);
	xrtValueRelease(config);
	assert(state.portCalls == 5);

	int diagnostics = 0;
	state.failDiagnostic = true;
	assert(xuiWorkflowValidateGraph(state.workflow, &diagnostics) == XUI_ERROR_INVALID_ARGUMENT);
	assert(callback_diagnostics(state.graph) == 0);
	state.failDiagnostic = false;
	state.exhaustDiagnostics = true;
	assert(xuiWorkflowValidateGraph(state.workflow, &diagnostics) == XUI_ERROR_OUT_OF_MEMORY);
	xrtClearError();
	assert(callback_diagnostics(state.graph) == 0);
	state.exhaustDiagnostics = false;
	assert(xuiWorkflowValidateGraph(state.workflow, &diagnostics) == XUI_OK);
	assert(callback_diagnostics(state.graph) == 1 && state.validationCalls == 3);

	xui_workflow alias = NULL;
	assert(xuiWorkflowShareOwned(state.workflow, &alias) == XUI_OK);
	assert(xuiWorkflowReleaseChecked(state.workflow) == XUI_OK && state.refs == 1);
	state.failDrop = true;
	bool consumed = false;
	assert(xuiWorkflowReleaseOwnerChecked(alias, &consumed) == XUI_ERROR_CALLBACK_FAILED && consumed);
	assert(state.refs == 0 && state.drops == 3);
	xrtClearError();
	assert(xuiFlowGraphGetNodeCount(state.graph) == 1);
	assert(xuiFlowGraphReleaseChecked(state.graph) == XUI_OK);
	puts("workflow-owned-callbacks-ok");
	return 0;
}
