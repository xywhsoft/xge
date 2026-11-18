#define XRT_MODULE_ALL
#include "xui.h"
#include "src/xui/core/xui_xrt_port.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void check_issue(xvalue* report, size_t index, const char* path, const char* code)
{
	xvalue* issue = xrtValueArrayGet(report, index);
	assert(issue && xrtValueCount(issue) == 3);
	assert(strcmp((const char*)xuiXrtValueObjectGetText(issue, "path", 4), path) == 0);
	assert(strcmp((const char*)xuiXrtValueObjectGetText(issue, "code", 4), code) == 0);
	assert(xuiXrtValueObjectGetText(issue, "message", 7));
}

int main(void)
{
	xui_workflow workflow = NULL;
	xvalue* schema = NULL;
	assert(xuiWorkflowCreateOwned(&workflow) == XUI_OK);
	assert(xuiWorkflowConfigSchemaCreate(&schema) == XUI_OK);
	for (int index = 0; index < 20; ++index) {
		char name[16];
		snprintf(name, sizeof(name), "field%d", index);
		xui_workflow_config_field_desc_t field = {
			.iSize = sizeof(field), .sId = name, .iKind = XUI_WORKFLOW_CONFIG_FIELD_INT,
			.bRequired = 1, .bHasMin = 1, .fMin = 10
		};
		assert(xuiWorkflowConfigSchemaAddFieldShared(schema, &field) == XUI_OK);
	}
	xui_workflow_node_type_desc_t type = {.iSize = sizeof(type), .sId = "sample", .pConfigSchema = schema};
	assert(xuiWorkflowRegisterNodeTypeShared(workflow, &type, NULL) == XUI_OK);
	xui_workflow_config_diagnostic_t summary = {0};
	int count = 0;
	assert(xuiWorkflowValidateConfigEx(workflow, "sample", NULL, &summary, 1, &count) == XUI_OK);
	assert(count == 20 && strcmp(summary.sPath, "field0") == 0);
	xvalue* config = xrtValueObject();
	assert(config && xuiXrtValueObjectSetInt(config, "field0", 6, 5));
	xvalue* report = NULL;
	assert(xuiWorkflowCollectConfigDiagnostics(workflow, "sample", config, &report) == XUI_OK);
	assert(xrtValueCount(report) == 20);
	check_issue(report, 0, "field0", "range.min");
	check_issue(report, 19, "field19", "required");

	int failures = 0;
	for (int budget = 0; budget < 512; ++budget) {
		xvalue* attempt = NULL;
		assert(xrtMemDebugFailAfter(budget));
		int status = xuiWorkflowCollectConfigDiagnostics(workflow, "sample", config, &attempt);
		bool triggered = xrtMemDebugFailTriggered();
		xrtMemDebugFailClear();
		xrtClearError();
		if (!triggered) {
			assert(status == XUI_OK && xrtValueCount(attempt) == 20);
			xrtValueRelease(attempt);
			break;
		}
		assert(status == XUI_ERROR_OUT_OF_MEMORY && !attempt);
		++failures;
	}
	assert(failures > 20 && failures < 512);
	xvalue* attempt = NULL;
	assert(xuiWorkflowCollectConfigDiagnostics(workflow, "absent", config, &attempt) == XUI_ERROR_INVALID_ARGUMENT);
	assert(!attempt);
	assert(xuiWorkflowReleaseChecked(workflow) == XUI_OK);
	xrtValueRelease(schema);
	xrtValueRelease(config);
	check_issue(report, 0, "field0", "range.min");
	check_issue(report, 19, "field19", "required");
	xrtValueRelease(report);
	printf("Workflow config report OOM prefix %d\n", failures);
	return 0;
}
