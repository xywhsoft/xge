#define XRT_MODULE_ALL
#include "xui.h"
#include "src/xui/core/xui_xrt_port.h"

#include <assert.h>
#include <stdio.h>

static xui_workflow make_workflow(void)
{
	xui_workflow workflow = NULL;
	assert(xuiWorkflowCreateOwned(&workflow) == XUI_OK);
	xui_workflow_node_type_desc_t type = {.iSize = sizeof(type), .sId = "step", .iVersion = 1};
	assert(xuiWorkflowRegisterNodeType(workflow, &type, NULL) == XUI_OK);
	return workflow;
}

static xvalue* lookup_shared(xvalue* tree, const char* array, const char* key)
{
	xvalue* rows = xuiXrtValueObjectGet(tree, array, 0);
	assert(rows && xrtValueCount(rows) == 1);
	return xuiXrtValueObjectGet(xuiXrtValueArrayGet(rows, 0), key, 0);
}

static void failure_prefixes(xui_workflow workflow, xvalue* tree)
{
	unsigned writeFailures = 0, readFailures = 0;
	for (unsigned i = 0; i < 256; ++i) {
		xvalue* output = NULL;
		assert(xrtMemDebugFailAfter(i));
		int status = xuiWorkflowToXValueShared(workflow, &output);
		bool triggered = xrtMemDebugFailTriggered();
		xrtMemDebugFailClear();
		xrtClearError();
		if (!triggered) {
			assert(status == XUI_OK && output);
			xrtValueRelease(output);
			break;
		}
		assert(status != XUI_OK && !output);
		++writeFailures;
	}
	for (unsigned i = 0; i < 256; ++i) {
		xui_workflow receiver = make_workflow();
		assert(xuiWorkflowAddNode(receiver, "step", "old-node", NULL, 0, 0, NULL) == XUI_OK);
		assert(xrtMemDebugFailAfter(i));
		int status = xuiWorkflowLoadXValueShared(receiver, tree);
		bool triggered = xrtMemDebugFailTriggered();
		xrtMemDebugFailClear();
		xrtClearError();
		xui_flow_graph graph = xuiWorkflowGetGraph(receiver);
		assert(xuiFlowGraphGetNodeCount(graph) == 1);
		assert(xuiFlowGraphFindNode(graph, triggered ? "old-node" : "node") == 0);
		assert(triggered ? status != XUI_OK : status == XUI_OK);
		assert(xuiWorkflowReleaseChecked(receiver) == XUI_OK);
		if (!triggered) break;
		++readFailures;
	}
	assert(writeFailures > 0 && writeFailures < 256);
	assert(readFailures > 0 && readFailures < 256);
	printf("XSON shared OOM prefixes %u/%u\n", writeFailures, readFailures);
}

int main(void)
{
	xui_workflow workflow = make_workflow();
	xui_workflow receiver = make_workflow();
	xvalue* config = xrtValueObject();
	assert(config && xuiXrtValueObjectSetInt(config, "count", 0, 5));
	assert(xuiWorkflowAddNode(workflow, "step", "node", NULL, 0, 0, NULL) == XUI_OK);
	assert(xuiWorkflowSetNodeConfigShared(workflow, "node", config) == XUI_OK);
	xui_workflow_variable_desc_t variable = {.iSize = sizeof(variable), .sId = "variable", .pDefaultValue = config};
	assert(xuiWorkflowAddVariableShared(workflow, &variable, NULL) == XUI_OK);

	xvalue *cloned = NULL, *shared = NULL;
	assert(xuiWorkflowToXValue(workflow, &cloned) == XUI_OK);
	assert(lookup_shared(cloned, "nodes", "config") != config);
	assert(lookup_shared(cloned, "variables", "default") != config);
	assert(xuiWorkflowToXValueShared(workflow, &shared) == XUI_OK);
	assert(lookup_shared(shared, "nodes", "config") == config);
	assert(lookup_shared(shared, "variables", "default") == config);

	xui_flow_graph receiverGraph = NULL;
	assert(xuiWorkflowGetGraphOwned(receiver, &receiverGraph) == XUI_OK);
	assert(xuiWorkflowLoadXValueShared(receiver, shared) == XUI_OK);
	assert(xuiWorkflowGetGraph(receiver) == receiverGraph && xuiWorkflowGetNodeTypeCount(receiver) == 1);
	xvalue* loaded = NULL;
	assert(xuiWorkflowGetNodeConfigShared(receiver, "node", &loaded) == XUI_OK && loaded == config);
	assert(xuiWorkflowGetVariable(receiver, 0, &variable) == XUI_OK && variable.pDefaultValue == config);
	xrtValueRelease(loaded);
	assert(xuiWorkflowLoadXValue(receiver, shared) == XUI_OK);
	assert(xuiWorkflowGetNodeConfigShared(receiver, "node", &loaded) == XUI_OK && loaded != config);
	xrtValueRelease(loaded);

	failure_prefixes(workflow, shared);
	xrtValueRelease(cloned);
	assert(xuiWorkflowReleaseChecked(workflow) == XUI_OK);
	assert(xuiWorkflowReleaseChecked(receiver) == XUI_OK);
	assert(xuiFlowGraphReleaseChecked(receiverGraph) == XUI_OK);
	assert(lookup_shared(shared, "nodes", "config") == config);
	xrtValueRelease(shared);
	xrtValueRelease(config);
	puts("workflow-xson-shared-ok");
	return 0;
}
