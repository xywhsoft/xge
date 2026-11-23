#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui_code_document_internal.h"
#include "xui_code_language_internal.h"

#include <limits.h>

XUI_API int xuiCodeFoldOutputGetCapacity(const xui_code_fold_output_t* output)
{
	return output != NULL ? output->iCapacity : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API int xuiCodeFoldOutputAppend(xui_code_fold_output_t* output, xui_code_fold_range_t range)
{
	if ( output == NULL || (range.iSize != 0 && range.iSize < sizeof(range)) ||
	     range.iStartLine < 0 || range.iEndLine <= range.iStartLine ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( output->iCount == INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
	if ( output->iCount < output->iCapacity ) output->pRanges[output->iCount] = range;
	++output->iCount;
	return XUI_OK;
}

typedef struct xui_code_fold_output_call_t {
	xui_code_fold_output_proc onFold;
	void* pUser;
	uint32_t iVersion;
	uint32_t iChangeVersion;
} xui_code_fold_output_call_t;

static int __xuiCodeFoldOutputCall(xui_code_document document, xui_code_fold_range_t* ranges,
	int capacity, int* count, void* user)
{
	const xui_code_fold_output_call_t* call = user;
	*count = 0;
	xui_code_fold_output_t output = {ranges, capacity, 0};
	int status = call->onFold(document, &output, call->pUser);
	if ( status == XUI_OK && xrtGetError() ) status = XUI_ERROR_CALLBACK_FAILED;
	if ( status == XUI_OK )
		status = xuiInternalCodeDocumentCheckVersions(document, call->iVersion, call->iChangeVersion);
	if ( status == XUI_OK ) *count = output.iCount;
	return status;
}

XUI_API int xuiCodeFoldStateBuildFromCallback(xui_code_fold_state state, xui_code_document document,
	xui_code_fold_output_proc callback, void* user)
{
	if ( callback == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	xerror* ambient = xrtTakeError();
	/* Enter before CodeEdit starts its guarded callback. The same document is
	   pinned for the count pass, fill passes and publication. */
	xui_code_document_scope_t documentScope = {0};
	int status = xuiCodeDocumentCallBegin(document, &documentScope);
	if ( status == XUI_OK ) {
		xui_code_fold_output_call_t call = {callback, user,
			xuiCodeDocumentGetVersion(document), xuiCodeDocumentGetChangeVersion(document)};
		status = xuiCodeFoldStateBuildFromProvider(state, document, __xuiCodeFoldOutputCall, &call);
		xui_cleanup_receipt_t receipt = {0};
		xuiInternalCleanupStart(&receipt, status);
		xuiInternalCleanupAdd(&receipt, xuiCodeDocumentCallEnd(&documentScope));
		status = xuiInternalCleanupEnd(NULL, &receipt);
	}
	if ( status == XUI_OK ) xrtSetErrorTake(ambient);
	else xrtErrorFree(ambient);
	return status;
}
#endif
