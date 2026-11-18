#define XRT_MODULE_ALL
#if defined(_WIN32)
#define XRT_USE_SHARED
#endif
#include "xui.h"
#include "xrt.h"
#include "xui_test_proxy.h"

#undef NDEBUG
#include <assert.h>
#include <stdio.h>

int main(void)
{
	xui_test_proxy_state_t proxy;
	xuiTestProxyInit(&proxy);
	xui_context context = NULL;
	xui_widget editor = NULL;
	assert(xuiCreate(&context) == XUI_OK);
	assert(xuiSetProxy(context, &proxy.tProxy) == XUI_OK);
	assert(xuiCodeEditCreate(context, &editor, NULL) == XUI_OK);
	assert(xuiCodeEditSetText(editor, "\xe4\xb8\xad su") == XUI_OK);
	xui_code_document document = xuiCodeEditGetDocument(editor);
	xui_code_selection_model selection = xuiCodeEditGetSelection(editor);
	assert(document != NULL && selection != NULL);

	uint32_t version = UINT32_MAX;
	int caret = -9;
	assert(xuiCodeEditGetCompletionPosition(editor, NULL, &caret) == XUI_ERROR_INVALID_ARGUMENT);
	assert(xuiCodeEditGetCompletionPosition(editor, &version, NULL) == XUI_ERROR_INVALID_ARGUMENT);
	assert(version == UINT32_MAX && caret == -9);
	assert(xuiCodeSelectionSetRange(selection, document, 0, 3) == XUI_OK);
	assert(xuiCodeEditGetCompletionPosition(editor, &version, &caret) == XUI_ERROR_UNSUPPORTED);
	assert(version == UINT32_MAX && caret == -9);

	assert(xuiCodeSelectionSetRange(selection, document, 6, 6) == XUI_OK);
	assert(xrtMemDebugEnabled() || xrtMemDebugEnable(true));
	assert(xrtMemDebugFailAfter(0));
	assert(xuiCodeEditGetCompletionPosition(editor, &version, &caret) == XUI_OK);
	assert(!xrtMemDebugFailTriggered());
	xrtMemDebugFailClear();
	assert(caret == 6 && version == xuiCodeDocumentGetVersion(document));
	assert(xuiCodeEditSetText(editor, "changed") == XUI_OK);
	uint32_t nextVersion = 0;
	assert(xuiCodeEditGetCompletionPosition(editor, &nextVersion, &caret) == XUI_OK);
	assert(nextVersion != version);

	xuiDestroy(context);
	puts("XUI_CODE_COMPLETION_NATIVE_OK");
	return 0;
}
