#include "xui_config.h"
#if XUI_ENABLE_CODE_EDIT
#include "xui_internal.h"
#include "xui_callback_hold_internal.h"
#include "xui_code_document_internal.h"
#include "xui_code_language_internal.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct xui_code_language_callbacks_t {
	xui_code_language_registry pRegistry;
	xui_callback_owner_t tOwner;
	xui_code_lexer_proc onLex;
	xui_code_fold_proc onFold;
	xui_code_lex_output_proc onTokenOutput;
	xui_code_fold_output_proc onFoldOutput;
	void* pUser;
} xui_code_language_callbacks_t;

typedef struct xui_code_language_entry_t {
	xui_code_language_t tLanguage;
	char* sId;
	char* sName;
	char** pExtensions;
	char* sLineComment;
	char* sBlockCommentStart;
	char* sBlockCommentEnd;
	xui_code_regex_rule_t* pRegexRules;
	char** pRuleNames;
	char** pRulePatterns;
	xui_code_language_callbacks_t* pCallbacks;
} xui_code_language_entry_t;

struct xui_code_language_registry_t {
	xui_code_language_entry_t* pEntries;
	xui_widget pOwner;
	const xui_code_language_owner_ops_t* pOwnerOps;
	xui_code_document pExecutingDocument;
	uint64_t iOwnerThread;
	uint64_t iCallbackThread;
	int iCount;
	int iCapacity;
	bool bExecuting;
	bool bChanging;
	bool bInvokingOwned;
};

static int __xuiCodeLanguageRegistryStatus(xui_code_language_registry registry, bool write)
{
	if ( registry == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( registry->pOwner != NULL && registry->iOwnerThread != xrtThreadCurrentId() ) return XUI_ERROR_BUSY;
	if ( registry->iCallbackThread != 0 && registry->iCallbackThread != xrtThreadCurrentId() ) return XUI_ERROR_BUSY;
	if ( write && (registry->bExecuting || registry->bChanging) ) return XUI_ERROR_BUSY;
	if ( registry->pOwner == NULL ) return XUI_OK;
	return registry->pOwnerOps->check(registry->pOwner, write);
}

XUI_API bool xuiCodeLanguageRegistryOwnershipBusy(xui_code_language_registry registry)
{
	return __xuiCodeLanguageRegistryStatus(registry, false) != XUI_OK ||
		registry->bExecuting || registry->bChanging;
}

XUI_API bool xuiCodeLanguageRegistryTraceOwnership(xui_code_language_registry registry,
	xrtownershipvisitor visit, void* user)
{
	if ( visit == NULL || xuiCodeLanguageRegistryOwnershipBusy(registry) ) return false;
	for ( int i = 0; i < registry->iCount; ++i ) {
		const xui_code_language_callbacks_t* callbacks = registry->pEntries[i].pCallbacks;
		if ( callbacks != NULL && callbacks->tOwner.onHold != NULL && !visit(callbacks->tOwner.tReference, user) ) return false;
	}
	return true;
}

int xuiInternalCodeLanguageRegistryClaimOwner(xui_code_language_registry registry,
	xui_widget editor, const xui_code_language_owner_ops_t* ops)
{
	if ( registry == NULL || editor == NULL || ops == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( registry->pOwner != NULL ) return XUI_ERROR_INVALID_STATE;
	registry->iOwnerThread = xrtThreadCurrentId();
	registry->pOwnerOps = ops;
	registry->pOwner = editor;
	return XUI_OK;
}

XUI_API int xuiCodeLanguageRegistryEditorLifetimeWatch(xui_code_language_registry registry,
	xui_lifetime_link_t* link, xui_lifetime_invalidate_proc invalidate, void* user)
{
	int status = __xuiCodeLanguageRegistryStatus(registry, false);
	if ( status != XUI_OK ) return status;
	if ( registry->pOwner == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return registry->pOwnerOps->watch(registry->pOwner, link, invalidate, user);
}

static void __xuiCodeLanguageRegistryChanged(xui_code_language_registry registry, const char* languageId)
{
	if ( registry->pOwner != NULL ) registry->pOwnerOps->changed(registry->pOwner, languageId);
}

static int __xuiCodeLanguageLexOwned(xui_code_document document, int firstLine, int endLine,
	xui_code_token_t* tokens, int capacity, int* count, void* user);
static int __xuiCodeLanguageFoldOwned(xui_code_document document, xui_code_fold_range_t* ranges,
	int capacity, int* count, void* user);

static char* __xuiCodeLanguageStrDup(const char* sText)
{
	return xuiInternalStringDuplicateNullable(sText);
}

static int __xuiCodeLanguageEquals(const char* a, const char* b)
{
	if ( a == NULL || b == NULL ) return 0;
	while ( *a != '\0' && *b != '\0' ) {
		if ( tolower((unsigned char)*a) != tolower((unsigned char)*b) ) return 0;
		a++;
		b++;
	}
	return *a == '\0' && *b == '\0';
}

static const char* __xuiCodeLanguageNormalizeExtension(const char* sExtension)
{
	if ( sExtension == NULL ) return NULL;
	if ( sExtension[0] == '.' ) return sExtension + 1;
	return sExtension;
}

static const char* __xuiCodeLanguageExtension(const xui_code_language_t* language,
	const xui_code_language_extension_t* extensions, int index)
{
	const char* extension = extensions != NULL ? extensions[index].sExtension : language->arrExtensions[index];
	return __xuiCodeLanguageNormalizeExtension(extension);
}

static void __xuiCodeLanguageSetError(char* sError, int iCapacity, const char* sMessage)
{
	if ( (sError == NULL) || (iCapacity <= 0) ) return;
	if ( sMessage == NULL ) sMessage = "";
	snprintf(sError, (size_t)iCapacity, "%s", sMessage);
}

static void __xuiCodeLanguageFreeEntry(xui_code_language_entry_t* pEntry)
{
	int i;

	if ( pEntry == NULL ) return;
	xrtFree(pEntry->sId);
	xrtFree(pEntry->sName);
	for ( i = 0; i < pEntry->tLanguage.iExtensionCount; i++ ) xrtFree(pEntry->pExtensions[i]);
	xrtFree(pEntry->pExtensions);
	xrtFree(pEntry->sLineComment);
	xrtFree(pEntry->sBlockCommentStart);
	xrtFree(pEntry->sBlockCommentEnd);
	for ( i = 0; i < pEntry->tLanguage.iRegexRuleCount; i++ ) {
		xrtFree(pEntry->pRuleNames[i]);
		xrtFree(pEntry->pRulePatterns[i]);
	}
	xrtFree(pEntry->pRuleNames);
	xrtFree(pEntry->pRulePatterns);
	xrtFree(pEntry->pRegexRules);
	memset(pEntry, 0, sizeof(*pEntry));
}

static void __xuiCodeLanguagePublish(xui_code_language_entry_t* pEntry)
{
	int i;

	pEntry->tLanguage.iSize = sizeof(pEntry->tLanguage);
	pEntry->tLanguage.sId = pEntry->sId;
	pEntry->tLanguage.sName = pEntry->sName;
	pEntry->tLanguage.arrExtensions = (const char* const*)pEntry->pExtensions;
	pEntry->tLanguage.sLineComment = pEntry->sLineComment;
	pEntry->tLanguage.sBlockCommentStart = pEntry->sBlockCommentStart;
	pEntry->tLanguage.sBlockCommentEnd = pEntry->sBlockCommentEnd;
	pEntry->tLanguage.pRegexRules = pEntry->pRegexRules;
	for ( i = 0; i < pEntry->tLanguage.iRegexRuleCount; i++ ) {
		pEntry->pRegexRules[i].iSize = sizeof(pEntry->pRegexRules[i]);
		pEntry->pRegexRules[i].sName = pEntry->pRuleNames[i];
		pEntry->pRegexRules[i].sPattern = pEntry->pRulePatterns[i];
	}
}

static int __xuiCodeLanguageReserve(xui_code_language_registry pRegistry, int iCapacity)
{
	xui_code_language_entry_t* pNew;
	int iNewCapacity;

	if ( pRegistry == NULL || iCapacity < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iCapacity <= pRegistry->iCapacity ) return XUI_OK;
	iNewCapacity = (pRegistry->iCapacity > 0) ? pRegistry->iCapacity : 8;
	while ( iNewCapacity < iCapacity ) {
		if ( iNewCapacity > INT_MAX / 2 ) {
			iNewCapacity = iCapacity;
			break;
		}
		iNewCapacity *= 2;
	}
	if ( (size_t)iNewCapacity > SIZE_MAX / sizeof(*pNew) ) return XUI_ERROR_LIMIT_EXCEEDED;
	pNew = (xui_code_language_entry_t*)xrtRealloc(pRegistry->pEntries,
		sizeof(*pNew) * (size_t)iNewCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pNew + pRegistry->iCapacity, 0,
		sizeof(*pNew) * (size_t)(iNewCapacity - pRegistry->iCapacity));
	pRegistry->pEntries = pNew;
	pRegistry->iCapacity = iNewCapacity;
	return XUI_OK;
}

static int __xuiCodeLanguageCopyEntry(xui_code_language_entry_t* pEntry, const xui_code_language_t* pLanguage,
	const xui_code_language_extension_t* extensions)
{
	int i;

	if ( pEntry == NULL || pLanguage == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (size_t)pLanguage->iExtensionCount > SIZE_MAX / sizeof(*pEntry->pExtensions) ||
	     (size_t)pLanguage->iRegexRuleCount > SIZE_MAX / sizeof(*pEntry->pRegexRules) ||
	     (size_t)pLanguage->iRegexRuleCount > SIZE_MAX / sizeof(*pEntry->pRuleNames) ||
	     (size_t)pLanguage->iRegexRuleCount > SIZE_MAX / sizeof(*pEntry->pRulePatterns) ) {
		return XUI_ERROR_LIMIT_EXCEEDED;
	}
	memset(pEntry, 0, sizeof(*pEntry));
	pEntry->tLanguage = *pLanguage;
	pEntry->tLanguage.iExtensionCount = 0;
	pEntry->tLanguage.iRegexRuleCount = 0;
	pEntry->sId = __xuiCodeLanguageStrDup(pLanguage->sId);
	pEntry->sName = __xuiCodeLanguageStrDup((pLanguage->sName != NULL) ? pLanguage->sName : pLanguage->sId);
	pEntry->sLineComment = __xuiCodeLanguageStrDup(pLanguage->sLineComment);
	pEntry->sBlockCommentStart = __xuiCodeLanguageStrDup(pLanguage->sBlockCommentStart);
	pEntry->sBlockCommentEnd = __xuiCodeLanguageStrDup(pLanguage->sBlockCommentEnd);
	if ( pEntry->sId == NULL || pEntry->sName == NULL ) goto oom;
	if ( pLanguage->sLineComment != NULL && pEntry->sLineComment == NULL ) goto oom;
	if ( pLanguage->sBlockCommentStart != NULL && pEntry->sBlockCommentStart == NULL ) goto oom;
	if ( pLanguage->sBlockCommentEnd != NULL && pEntry->sBlockCommentEnd == NULL ) goto oom;
	if ( pLanguage->iExtensionCount > 0 ) {
		pEntry->pExtensions = (char**)xrtMalloc(sizeof(char*) * (size_t)pLanguage->iExtensionCount);
		if ( pEntry->pExtensions == NULL ) goto oom;
		memset(pEntry->pExtensions, 0, sizeof(char*) * (size_t)pLanguage->iExtensionCount);
		for ( i = 0; i < pLanguage->iExtensionCount; i++ ) {
			pEntry->pExtensions[i] = __xuiCodeLanguageStrDup(__xuiCodeLanguageExtension(pLanguage, extensions, i));
			if ( pEntry->pExtensions[i] == NULL ) goto oom;
			pEntry->tLanguage.iExtensionCount++;
		}
	}
	if ( pLanguage->iRegexRuleCount > 0 ) {
		pEntry->pRegexRules = (xui_code_regex_rule_t*)xrtMalloc(sizeof(*pEntry->pRegexRules) * (size_t)pLanguage->iRegexRuleCount);
		pEntry->pRuleNames = (char**)xrtMalloc(sizeof(char*) * (size_t)pLanguage->iRegexRuleCount);
		pEntry->pRulePatterns = (char**)xrtMalloc(sizeof(char*) * (size_t)pLanguage->iRegexRuleCount);
		if ( pEntry->pRegexRules == NULL || pEntry->pRuleNames == NULL || pEntry->pRulePatterns == NULL ) goto oom;
		memset(pEntry->pRegexRules, 0, sizeof(*pEntry->pRegexRules) * (size_t)pLanguage->iRegexRuleCount);
		memset(pEntry->pRuleNames, 0, sizeof(char*) * (size_t)pLanguage->iRegexRuleCount);
		memset(pEntry->pRulePatterns, 0, sizeof(char*) * (size_t)pLanguage->iRegexRuleCount);
		for ( i = 0; i < pLanguage->iRegexRuleCount; i++ ) {
			pEntry->pRegexRules[i] = pLanguage->pRegexRules[i];
			/* Both pointer arrays are initialized, so rollback includes this slot. */
			pEntry->tLanguage.iRegexRuleCount++;
			pEntry->pRuleNames[i] = __xuiCodeLanguageStrDup(pLanguage->pRegexRules[i].sName);
			pEntry->pRulePatterns[i] = __xuiCodeLanguageStrDup(pLanguage->pRegexRules[i].sPattern);
			if ( (pLanguage->pRegexRules[i].sName != NULL && pEntry->pRuleNames[i] == NULL) ||
			     pEntry->pRulePatterns[i] == NULL ) goto oom;
		}
	}
	__xuiCodeLanguagePublish(pEntry);
	return XUI_OK;

oom:
	__xuiCodeLanguageFreeEntry(pEntry);
	return XUI_ERROR_OUT_OF_MEMORY;
}

static int __xuiCodeLanguageCLex(xui_code_document_t* pDocument, int iStartLine, int iEndLine, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount, void* pUser)
{
	int iStartOffset;
	int iEndOffset;
	int iLineCount;
	int iRet;

	(void)pUser;
	if ( (pDocument == NULL) || (pTokenCount == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iLineCount = xuiCodeDocumentGetLineCount(pDocument);
	if ( iStartLine < 0 ) iStartLine = 0;
	if ( iEndLine < 0 || iEndLine >= iLineCount ) iEndLine = iLineCount - 1;
	if ( iStartLine > iEndLine ) {
		*pTokenCount = 0;
		return XUI_OK;
	}
	iRet = xuiCodeDocumentGetLineRange(pDocument, iStartLine, &iStartOffset, NULL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeDocumentGetLineRange(pDocument, iEndLine, NULL, &iEndOffset);
	if ( iRet != XUI_OK ) return iRet;
	return xuiCodeLexerCTokenizeDocumentRange(pDocument, iStartOffset, iEndOffset,
		pTokens, iTokenCapacity, pTokenCount);
}

int xuiInternalCodeLanguageUsesBuiltinC(const xui_code_language_t* pLanguage)
{
	return pLanguage != NULL && pLanguage->iSize >= sizeof(*pLanguage) &&
		pLanguage->iLexerType == XUI_CODE_LEXER_BUILTIN_C &&
		pLanguage->onLex == __xuiCodeLanguageCLex;
}

static int __xuiCodeLanguageRegexLexRange(const xui_code_language_t* pLanguage,
	xui_code_document pDocument, int iStartLine, int iEndLine,
	xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount,
	char* sError, int iErrorCapacity)
{
	char* sRange;
	int iLineCount;
	int iStartOffset;
	int iEndOffset;
	int iRangeLength;
	int iStoredCount;
	int i;
	int iRet;

	iLineCount = xuiCodeDocumentGetLineCount(pDocument);
	if ( iStartLine < 0 ) iStartLine = 0;
	if ( iEndLine < 0 || iEndLine >= iLineCount ) iEndLine = iLineCount - 1;
	if ( iStartLine > iEndLine ) {
		*pTokenCount = 0;
		return XUI_OK;
	}
	iRet = xuiCodeDocumentGetLineRange(pDocument, iStartLine, &iStartOffset, NULL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiCodeDocumentGetLineRange(pDocument, iEndLine, NULL, &iEndOffset);
	if ( iRet != XUI_OK ) return iRet;
	iRangeLength = iEndOffset - iStartOffset;
	sRange = (char*)xrtMalloc((size_t)iRangeLength + 1u);
	if ( sRange == NULL ) {
		__xuiCodeLanguageSetError(sError, iErrorCapacity, "out of memory");
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	iRet = xuiCodeDocumentCopyRange(pDocument, iStartOffset, iEndOffset,
		sRange, iRangeLength + 1, NULL);
	if ( iRet == XUI_OK ) {
		iRet = xuiCodeLexerRegexTokenize(sRange, iRangeLength,
			pLanguage->pRegexRules, pLanguage->iRegexRuleCount,
			pTokens, iTokenCapacity, pTokenCount, sError, iErrorCapacity);
	}
	if ( iRet == XUI_OK && pTokens != NULL ) {
		iStoredCount = *pTokenCount;
		if ( iStoredCount > iTokenCapacity ) iStoredCount = iTokenCapacity;
		for ( i = 0; i < iStoredCount; i++ ) {
			pTokens[i].iStartOffset += iStartOffset;
			pTokens[i].iEndOffset += iStartOffset;
		}
	}
	xrtFree(sRange);
	return iRet;
}

static int __xuiCodeLanguageCFold(xui_code_document_t* pDocument, xui_code_fold_range_t* pRanges, int iRangeCapacity, int* pRangeCount, void* pUser)
{
	(void)pUser;
	if ( pDocument == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiCodeFoldCBuildDocumentRanges(pDocument, pRanges, iRangeCapacity, pRangeCount);
}

XUI_API int xuiCodeLanguageRegistryCreate(xui_code_language_registry* ppRegistry)
{
	xui_code_language_registry pRegistry;

	if ( ppRegistry == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppRegistry = NULL;
	pRegistry = (xui_code_language_registry)xrtMalloc(sizeof(*pRegistry));
	if ( pRegistry == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pRegistry, 0, sizeof(*pRegistry));
	*ppRegistry = pRegistry;
	return XUI_OK;
}

static void __xuiCodeLanguageRetireEntry(xui_code_language_entry_t* entry, xui_context context,
	xui_cleanup_receipt_t* receipt)
{
	xui_code_language_callbacks_t* callbacks = entry->pCallbacks;
	__xuiCodeLanguageFreeEntry(entry);
	if ( callbacks != NULL ) {
		xuiInternalCleanupAdd(receipt, xuiInternalCallbackOwnerDropOutcome(context,
			&callbacks->tOwner, receipt->status, receipt->error));
		xrtFree(callbacks);
	}
}

static int __xuiCodeLanguageRetireEntries(xui_code_language_registry registry, xui_context context,
	xui_cleanup_receipt_t* receipt)
{
	xrtownershipscope mutation = {0};
	if ( !xrtOwnershipMutationBegin(&mutation) ) return XUI_ERROR_INVALID_STATE;
	int count = registry->iCount;
	registry->iCount = 0;
	if ( !xrtOwnershipScopeEnd(&mutation) ) abort();
	for ( int i = 0; i < count; ++i )
		__xuiCodeLanguageRetireEntry(&registry->pEntries[i], context, receipt);
	return XUI_OK;
}

static int __xuiCodeLanguageRegistryDestroy(xui_code_language_registry registry, xui_widget editor,
	bool* consumed)
{
	registry->bChanging = true;
	xui_cleanup_receipt_t receipt = {0};
	xuiInternalCleanupStart(&receipt, XUI_OK);
	int status = __xuiCodeLanguageRetireEntries(registry, editor ? editor->pContext : NULL, &receipt);
	xuiInternalCleanupAdd(&receipt, status);
	if ( status == XUI_OK ) {
		*consumed = true;
		xrtFree(registry->pEntries);
		xrtFree(registry);
	} else registry->bChanging = false;
	return xuiInternalCleanupEnd(NULL, &receipt);
}

int xuiInternalCodeLanguageRegistryDestroyOwned(xui_code_language_registry registry, xui_widget editor)
{
	if ( registry == NULL ) return XUI_OK;
	if ( registry->bChanging || registry->bExecuting ||
	     (registry->pOwner != NULL && registry->pOwner != editor) ) return XUI_ERROR_BUSY;
	bool consumed = false;
	return __xuiCodeLanguageRegistryDestroy(registry, editor, &consumed);
}

XUI_API int xuiCodeLanguageRegistryDestroyOwnerChecked(xui_code_language_registry registry, bool* consumed)
{
	if ( consumed == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*consumed = false;
	int status = __xuiCodeLanguageRegistryStatus(registry, true);
	if ( status != XUI_OK ) return status;
	if ( registry->pOwner != NULL ) return XUI_ERROR_BUSY;
	return __xuiCodeLanguageRegistryDestroy(registry, NULL, consumed);
}

XUI_API int xuiCodeLanguageRegistryDestroyChecked(xui_code_language_registry registry)
{
	bool consumed = false;
	return xuiCodeLanguageRegistryDestroyOwnerChecked(registry, &consumed);
}

XUI_API void xuiCodeLanguageRegistryDestroy(xui_code_language_registry registry)
{
	if ( registry != NULL ) (void)xuiCodeLanguageRegistryDestroyChecked(registry);
}

XUI_API int xuiCodeLanguageRegistryClearChecked(xui_code_language_registry registry)
{
	int status = __xuiCodeLanguageRegistryStatus(registry, true);
	if ( status != XUI_OK ) return status;
	if ( registry->iCount == 0 ) return XUI_OK;
	xui_context context = registry->pOwner ? registry->pOwner->pContext : NULL;
	xui_call_scope_t scope = {0};
	if ( context != NULL && (status = xuiCallScopeBegin(context, &scope)) != XUI_OK ) return status;
	registry->bChanging = true;
	xui_cleanup_receipt_t receipt = {0};
	xuiInternalCleanupStart(&receipt, XUI_OK);
	status = __xuiCodeLanguageRetireEntries(registry, context, &receipt);
	xuiInternalCleanupAdd(&receipt, status);
	if ( status == XUI_OK ) __xuiCodeLanguageRegistryChanged(registry, NULL);
	registry->bChanging = false;
	if ( scope.pSelf != NULL ) receipt.status = xuiInternalCallScopeEndOutcome(&scope, receipt.status, &receipt.error);
	return xuiInternalCleanupEnd(NULL, &receipt);
}

XUI_API void xuiCodeLanguageRegistryClear(xui_code_language_registry pRegistry)
{
	if ( pRegistry != NULL ) (void)xuiCodeLanguageRegistryClearChecked(pRegistry);
}

XUI_API int xuiCodeLanguageRegistryLoadDefaults(xui_code_language_registry pRegistry)
{
	static const char* arrCExtensions[] = { "c", "h" };
	xui_code_language_t tLanguage;

	if ( pRegistry == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tLanguage, 0, sizeof(tLanguage));
	tLanguage.iSize = sizeof(tLanguage);
	tLanguage.sId = "c";
	tLanguage.sName = "C";
	tLanguage.arrExtensions = arrCExtensions;
	tLanguage.iExtensionCount = 2;
	tLanguage.sLineComment = "//";
	tLanguage.sBlockCommentStart = "/*";
	tLanguage.sBlockCommentEnd = "*/";
	tLanguage.iLexerType = XUI_CODE_LEXER_BUILTIN_C;
	tLanguage.onLex = __xuiCodeLanguageCLex;
	tLanguage.onFold = __xuiCodeLanguageCFold;
	return xuiCodeLanguageRegistryRegister(pRegistry, &tLanguage);
}

static int __xuiCodeLanguageRegister(xui_code_language_registry pRegistry, const xui_code_language_t* pLanguage,
	const xui_code_language_extension_t* extensions, const xui_callback_owner_t* owner,
	const xui_code_callback_language_t* typed)
{
	xui_code_language_entry_t tEntry;
	int i;
	int iRet;

	iRet = __xuiCodeLanguageRegistryStatus(pRegistry, true);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pLanguage == NULL) ||
	     (pLanguage->iSize < sizeof(*pLanguage)) ||
	     (pLanguage->sId == NULL) || (pLanguage->sId[0] == '\0') ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pLanguage->iExtensionCount < 0 ||
	     (pLanguage->iExtensionCount > 0 && pLanguage->arrExtensions == NULL && extensions == NULL) )
		return XUI_ERROR_INVALID_ARGUMENT;
	if ( pLanguage->iRegexRuleCount < 0 || (pLanguage->iRegexRuleCount > 0 && pLanguage->pRegexRules == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pLanguage->iExtensionCount; i++ ) {
		const char* sExtension = __xuiCodeLanguageExtension(pLanguage, extensions, i);
		if ( sExtension == NULL || sExtension[0] == '\0' ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	for ( i = 0; i < pLanguage->iRegexRuleCount; i++ ) {
		if ( pLanguage->pRegexRules[i].iSize < sizeof(pLanguage->pRegexRules[i]) ||
		     pLanguage->pRegexRules[i].sPattern == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = __xuiCodeLanguageCopyEntry(&tEntry, pLanguage, extensions);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < pRegistry->iCount; i++ ) {
		if ( __xuiCodeLanguageEquals(pRegistry->pEntries[i].tLanguage.sId, pLanguage->sId) ) break;
	}
	bool append = i == pRegistry->iCount;
	if ( append ) {
		iRet = i == INT_MAX ? XUI_ERROR_LIMIT_EXCEEDED : __xuiCodeLanguageReserve(pRegistry, i + 1);
		if ( iRet != XUI_OK ) {
			__xuiCodeLanguageFreeEntry(&tEntry);
			return iRet;
		}
	}
	xui_context context = pRegistry->pOwner ? pRegistry->pOwner->pContext : NULL;
	xui_call_scope_t scope = {0};
	if ( context != NULL && (iRet = xuiCallScopeBegin(context, &scope)) != XUI_OK ) {
		__xuiCodeLanguageFreeEntry(&tEntry); return iRet;
	}
	xerror* ambient = context == NULL ? xrtTakeError() : NULL;
	pRegistry->bChanging = true;
	bool held = false;
	uint64_t previousThread = pRegistry->iCallbackThread;
	if ( typed != NULL || (owner != NULL && owner->onHold != NULL) ) {
		xui_code_language_callbacks_t* callbacks = xrtMalloc(sizeof(*callbacks));
		if ( callbacks == NULL ) iRet = XUI_ERROR_OUT_OF_MEMORY;
		else {
			xui_callback_owner_t emptyOwner = {0};
			*callbacks = (xui_code_language_callbacks_t){
				.pRegistry = pRegistry, .tOwner = owner ? *owner : emptyOwner,
				.onLex = pLanguage->onLex, .onFold = pLanguage->onFold, .pUser = pLanguage->pUser,
				.onTokenOutput = typed ? typed->onLex : NULL, .onFoldOutput = typed ? typed->onFold : NULL
			};
			tEntry.pCallbacks = callbacks;
			tEntry.tLanguage.pUser = callbacks;
			tEntry.tLanguage.onLex = callbacks->onLex || callbacks->onTokenOutput ? __xuiCodeLanguageLexOwned : NULL;
			tEntry.tLanguage.onFold = callbacks->onFold || callbacks->onFoldOutput ? __xuiCodeLanguageFoldOwned : NULL;
			pRegistry->iCallbackThread = xrtThreadCurrentId();
			iRet = xuiInternalCallbackHoldFresh(&callbacks->tOwner, &held);
		}
	}
	if ( iRet == XUI_OK ) iRet = __xuiCodeLanguageRegistryStatus(pRegistry, false);
	if ( iRet == XUI_OK && context != NULL && context->bCloseRequested ) iRet = XUI_ERROR_INVALID_STATE;
	xrtownershipscope mutation = {0};
	if ( iRet == XUI_OK && !xrtOwnershipMutationBegin(&mutation) ) iRet = XUI_ERROR_INVALID_STATE;
	xui_cleanup_receipt_t receipt = {0};
	xuiInternalCleanupStart(&receipt, iRet);
	if ( iRet == XUI_OK ) {
		xui_code_language_entry_t previous = pRegistry->pEntries[i];
		pRegistry->pEntries[i] = tEntry;
		if ( append ) ++pRegistry->iCount;
		if ( !xrtOwnershipScopeEnd(&mutation) ) abort();
		__xuiCodeLanguageRegistryChanged(pRegistry, tEntry.sId);
		__xuiCodeLanguageRetireEntry(&previous, context, &receipt);
	} else {
		if ( tEntry.pCallbacks != NULL && !held ) {
			xrtFree(tEntry.pCallbacks);
			tEntry.pCallbacks = NULL;
		}
		__xuiCodeLanguageRetireEntry(&tEntry, context, &receipt);
		pRegistry->iCallbackThread = previousThread;
	}
	pRegistry->bChanging = false;
	if ( scope.pSelf != NULL ) receipt.status = xuiInternalCallScopeEndOutcome(&scope, receipt.status, &receipt.error);
	iRet = xuiInternalCleanupEnd(NULL, &receipt);
	if ( context == NULL && iRet == XUI_OK ) xrtSetErrorTake(ambient);
	else xrtErrorFree(ambient);
	return iRet;
}

XUI_API int xuiCodeLanguageRegistryRegister(xui_code_language_registry registry, const xui_code_language_t* language)
{
	return __xuiCodeLanguageRegister(registry, language, NULL, NULL, NULL);
}

XUI_API int xuiCodeLanguageRegistryRegisterOwned(xui_code_language_registry registry,
	const xui_code_language_t* language, const xui_callback_owner_t* owner)
{
	if ( !xuiInternalCallbackOwnerRawValid(owner) || (owner != NULL && owner->onHold != NULL &&
	     (language == NULL || language->iSize < sizeof(*language) ||
	      (language->onLex == NULL && language->onFold == NULL))) ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiCodeLanguageRegister(registry, language, NULL, owner, NULL);
}

XUI_API int xuiCodeLanguageRegistryRegisterCallbacks(xui_code_language_registry registry,
	const xui_code_callback_language_t* language, const xui_callback_owner_t* owner)
{
	if ( language == NULL || language->iSize < sizeof(*language) ||
	     (language->onLex == NULL && language->onFold == NULL) || !xuiInternalCallbackOwnerRawValid(owner) )
		return XUI_ERROR_INVALID_ARGUMENT;
	xui_code_language_t native = {
		.iSize = sizeof(native), .sId = language->sId, .sName = language->sName,
		.sLineComment = language->sLineComment, .sBlockCommentStart = language->sBlockCommentStart,
		.sBlockCommentEnd = language->sBlockCommentEnd, .iLexerType = XUI_CODE_LEXER_CALLBACK,
		.iExtensionCount = language->iExtensionCount, .pUser = language->pUser
	};
	return __xuiCodeLanguageRegister(registry, &native, language->pExtensions, owner, language);
}

XUI_API int xuiCodeLanguageRegistryRegisterRegex(xui_code_language_registry registry,
	const xui_code_regex_language_t* language)
{
	if ( language == NULL || language->iSize < sizeof(*language) || language->iRegexRuleCount <= 0 )
		return XUI_ERROR_INVALID_ARGUMENT;
	xui_code_language_t native = {
		.iSize = sizeof(native), .sId = language->sId, .sName = language->sName,
		.sLineComment = language->sLineComment, .sBlockCommentStart = language->sBlockCommentStart,
		.sBlockCommentEnd = language->sBlockCommentEnd, .iLexerType = XUI_CODE_LEXER_REGEX,
		.iExtensionCount = language->iExtensionCount, .pRegexRules = language->pRegexRules,
		.iRegexRuleCount = language->iRegexRuleCount
	};
	return __xuiCodeLanguageRegister(registry, &native, language->pExtensions, NULL, NULL);
}

XUI_API int xuiCodeLanguageRegistryGetCount(xui_code_language_registry pRegistry)
{
	if ( pRegistry == NULL ) return 0;
	int status = __xuiCodeLanguageRegistryStatus(pRegistry, false);
	return status == XUI_OK ? pRegistry->iCount : status;
}

XUI_API int xuiCodeLanguageRegistryGet(xui_code_language_registry pRegistry, int iIndex, xui_code_language_t* pLanguage)
{
	int status = __xuiCodeLanguageRegistryStatus(pRegistry, false);
	if ( status != XUI_OK ) return status;
	if ( (pLanguage == NULL) || (iIndex < 0) || (iIndex >= pRegistry->iCount) ) return XUI_ERROR_INVALID_ARGUMENT;
	*pLanguage = pRegistry->pEntries[iIndex].tLanguage;
	pLanguage->iSize = sizeof(*pLanguage);
	return XUI_OK;
}

XUI_API int xuiCodeLanguageRegistryFind(xui_code_language_registry pRegistry, const char* sId, xui_code_language_t* pLanguage)
{
	int i;

	int status = __xuiCodeLanguageRegistryStatus(pRegistry, false);
	if ( status != XUI_OK ) return status;
	if ( (sId == NULL) || (pLanguage == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < pRegistry->iCount; i++ ) {
		if ( __xuiCodeLanguageEquals(pRegistry->pEntries[i].tLanguage.sId, sId) ) {
			*pLanguage = pRegistry->pEntries[i].tLanguage;
			pLanguage->iSize = sizeof(*pLanguage);
			return XUI_OK;
		}
	}
	return XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiCodeLanguageRegistryFindByExtension(xui_code_language_registry pRegistry, const char* sExtension, xui_code_language_t* pLanguage)
{
	const char* sNorm;
	int i;
	int j;

	int status = __xuiCodeLanguageRegistryStatus(pRegistry, false);
	if ( status != XUI_OK ) return status;
	if ( (sExtension == NULL) || (pLanguage == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	sNorm = __xuiCodeLanguageNormalizeExtension(sExtension);
	for ( i = 0; i < pRegistry->iCount; i++ ) {
		for ( j = 0; j < pRegistry->pEntries[i].tLanguage.iExtensionCount; j++ ) {
			if ( __xuiCodeLanguageEquals(pRegistry->pEntries[i].tLanguage.arrExtensions[j], sNorm) ) {
				*pLanguage = pRegistry->pEntries[i].tLanguage;
				pLanguage->iSize = sizeof(*pLanguage);
				return XUI_OK;
			}
		}
	}
	return XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiCodeLanguageRegistryGetExtension(xui_code_language_registry registry,
	int languageIndex, int extensionIndex, const char** extension)
{
	if ( extension == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	xui_code_language_t language;
	int status = xuiCodeLanguageRegistryGet(registry, languageIndex, &language);
	if ( status != XUI_OK ) return status;
	if ( extensionIndex < 0 || extensionIndex >= language.iExtensionCount ) return XUI_ERROR_INVALID_ARGUMENT;
	*extension = language.arrExtensions[extensionIndex];
	return XUI_OK;
}

XUI_API int xuiCodeLanguageRegistryGetRegexRule(xui_code_language_registry registry,
	int languageIndex, int ruleIndex, xui_code_regex_rule_t* rule)
{
	if ( rule == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	xui_code_language_t language;
	int status = xuiCodeLanguageRegistryGet(registry, languageIndex, &language);
	if ( status != XUI_OK ) return status;
	if ( ruleIndex < 0 || ruleIndex >= language.iRegexRuleCount ) return XUI_ERROR_INVALID_ARGUMENT;
	*rule = language.pRegexRules[ruleIndex];
	return XUI_OK;
}

XUI_API int xuiCodeLanguageLex(const xui_code_language_t* pLanguage, xui_code_document pDocument, int iStartLine, int iEndLine, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount, char* sError, int iErrorCapacity)
{
	int iRet;

	if ( (pLanguage == NULL) || (pLanguage->iSize < sizeof(*pLanguage)) ||
	     (pDocument == NULL) || (pTokenCount == NULL) ||
	     iTokenCapacity < 0 || (pTokens == NULL && iTokenCapacity > 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pTokenCount = 0;
	__xuiCodeLanguageSetError(sError, iErrorCapacity, "");
	if ( pLanguage->onLex != NULL ) {
		iRet = pLanguage->onLex(pDocument, iStartLine, iEndLine, pTokens,
			iTokenCapacity, pTokenCount, pLanguage->pUser);
		if ( iRet != XUI_OK ) {
			*pTokenCount = 0;
			return iRet;
		}
		if ( *pTokenCount < 0 ) {
			*pTokenCount = 0;
			return XUI_ERROR_INVALID_STATE;
		}
		return iRet;
	}
	if ( pLanguage->iLexerType == XUI_CODE_LEXER_REGEX && pLanguage->pRegexRules != NULL ) {
		iRet = __xuiCodeLanguageRegexLexRange(pLanguage, pDocument,
			iStartLine, iEndLine, pTokens, iTokenCapacity, pTokenCount,
			sError, iErrorCapacity);
		if ( iRet != XUI_OK ) {
			*pTokenCount = 0;
			return iRet;
		}
		if ( *pTokenCount < 0 ) {
			*pTokenCount = 0;
			return XUI_ERROR_INVALID_STATE;
		}
		return iRet;
	}
	__xuiCodeLanguageSetError(sError, iErrorCapacity, "language has no lexer");
	return XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiCodeLanguageFold(const xui_code_language_t* pLanguage, xui_code_document pDocument, xui_code_fold_range_t* pRanges, int iRangeCapacity, int* pRangeCount)
{
	int iRet;

	if ( (pLanguage == NULL) || (pLanguage->iSize < sizeof(*pLanguage)) ||
	     (pDocument == NULL) || (pRangeCount == NULL) ||
	     iRangeCapacity < 0 || (pRanges == NULL && iRangeCapacity > 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	*pRangeCount = 0;
	if ( pLanguage->onFold != NULL ) {
		iRet = pLanguage->onFold(pDocument, pRanges, iRangeCapacity,
			pRangeCount, pLanguage->pUser);
		if ( iRet != XUI_OK ) {
			*pRangeCount = 0;
			return iRet;
		}
		if ( *pRangeCount < 0 ) {
			*pRangeCount = 0;
			return XUI_ERROR_INVALID_STATE;
		}
		return iRet;
	}
	return XUI_ERROR_UNSUPPORTED;
}

typedef struct xui_code_language_call_t {
	xui_call_scope_t tOwner;
	xui_code_document_scope_t tDocument;
} xui_code_language_call_t;

static int __xuiCodeLanguageCallBegin(xui_code_language_registry registry, const char* id,
	xui_code_document document, xui_code_language_t* language, xui_code_language_call_t* call)
{
	int status = xuiCodeLanguageRegistryFind(registry, id, language);
	if ( status != XUI_OK ) return status;
	if ( registry->bExecuting || registry->bChanging ) return XUI_ERROR_BUSY;
	if ( registry->pOwner != NULL ) {
		status = xuiCallScopeBegin(registry->pOwner->pContext, &call->tOwner);
		if ( status != XUI_OK ) return status;
	}
	status = xuiCodeDocumentCallBegin(document, &call->tDocument);
	if ( status != XUI_OK ) {
		if ( call->tOwner.pSelf != NULL )
			return xuiInternalCallScopeEndRestored(&call->tOwner, status, xrtTakeError());
		return status;
	}
	registry->bExecuting = true;
	registry->pExecutingDocument = document;
	return XUI_OK;
}

static int __xuiCodeLanguageCallEnd(xui_code_language_registry registry,
	xui_code_language_call_t* call, int status, int* count)
{
	xui_cleanup_receipt_t receipt = {0};
	xuiInternalCleanupStart(&receipt, status);
	xuiInternalCleanupAdd(&receipt, xuiCodeDocumentCallEnd(&call->tDocument));
	registry->pExecutingDocument = NULL;
	registry->bExecuting = false;
	if ( call->tOwner.pSelf != NULL )
		receipt.status = xuiInternalCallScopeEndOutcome(&call->tOwner, receipt.status, &receipt.error);
	status = xuiInternalCleanupEnd(NULL, &receipt);
	if ( status != XUI_OK ) *count = 0;
	return status;
}

XUI_API int xuiCodeLanguageRegistryLex(xui_code_language_registry registry, const char* id,
	xui_code_document document, int firstLine, int endLine, xui_code_token_t* tokens, int capacity,
	int* count, char* diagnostic, int diagnosticCapacity)
{
	if ( count == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*count = 0;
	__xuiCodeLanguageSetError(diagnostic, diagnosticCapacity, "");
	xui_code_language_t language;
	xui_code_language_call_t call = {0};
	int status = __xuiCodeLanguageCallBegin(registry, id, document, &language, &call);
	if ( status != XUI_OK ) return status;
	status = xuiCodeLanguageLex(&language, document, firstLine, endLine, tokens, capacity,
		count, diagnostic, diagnosticCapacity);
	return __xuiCodeLanguageCallEnd(registry, &call, status, count);
}

XUI_API int xuiCodeLanguageRegistryFold(xui_code_language_registry registry, const char* id,
	xui_code_document document, xui_code_fold_range_t* ranges, int capacity, int* count)
{
	if ( count == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*count = 0;
	xui_code_language_t language;
	xui_code_language_call_t call = {0};
	int status = __xuiCodeLanguageCallBegin(registry, id, document, &language, &call);
	if ( status != XUI_OK ) return status;
	status = xuiCodeLanguageFold(&language, document, ranges, capacity, count);
	return __xuiCodeLanguageCallEnd(registry, &call, status, count);
}

typedef struct xui_code_language_owned_call_t {
	xui_code_language_call_t tScope;
	xui_context pContext;
	xerror* pAmbient;
	uint32_t iVersion;
	uint32_t iChangeVersion;
	bool bBorrowedExecution;
	bool bWasGuarded;
} xui_code_language_owned_call_t;

struct xui_code_token_output_t {
	xui_code_token_t* pTokens;
	int iCapacity;
	int iCount;
};

XUI_API int xuiCodeTokenOutputGetCapacity(const xui_code_token_output_t* output)
{
	return output ? output->iCapacity : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API int xuiCodeTokenOutputAppend(xui_code_token_output_t* output, xui_code_token_t token)
{
	if ( output == NULL || token.iStartOffset < 0 || token.iEndOffset <= token.iStartOffset )
		return XUI_ERROR_INVALID_ARGUMENT;
	if ( output->iCount == INT_MAX ) return XUI_ERROR_LIMIT_EXCEEDED;
	if ( output->iCount < output->iCapacity ) output->pTokens[output->iCount] = token;
	++output->iCount;
	return XUI_OK;
}

static int __xuiCodeLanguageOwnedBegin(xui_code_language_registry registry, xui_code_document document,
	xui_code_language_owned_call_t* call)
{
	int status = __xuiCodeLanguageRegistryStatus(registry, false);
	if ( status != XUI_OK ) return status;
	if ( registry->bChanging || registry->bInvokingOwned ) return XUI_ERROR_BUSY;
	call->pContext = registry->pOwner ? registry->pOwner->pContext : NULL;
	call->bBorrowedExecution = registry->bExecuting;
	if ( call->bBorrowedExecution && registry->pExecutingDocument != document ) return XUI_ERROR_BUSY;
	call->bWasGuarded = call->pContext != NULL && call->pContext->bGuardedCallback;
	/* Editor rebuilds already pin their own document under the Context guard. */
	if ( call->bWasGuarded && (call->pContext->iOperationDepth == 0 ||
	     xuiCodeEditGetDocument(registry->pOwner) != document) ) return XUI_ERROR_BUSY;
	call->pAmbient = xrtTakeError();
	if ( !call->bBorrowedExecution && !call->bWasGuarded ) {
		if ( call->pContext != NULL ) status = xuiCallScopeBegin(call->pContext, &call->tScope.tOwner);
		if ( status == XUI_OK ) status = xuiCodeDocumentCallBegin(document, &call->tScope.tDocument);
		if ( status != XUI_OK ) {
			xrtErrorFree(call->pAmbient);
			if ( call->tScope.tOwner.pSelf != NULL )
				return xuiInternalCallScopeEndRestored(&call->tScope.tOwner, status, xrtTakeError());
			return status;
		}
	}
	call->iVersion = xuiCodeDocumentGetVersion(document);
	call->iChangeVersion = xuiCodeDocumentGetChangeVersion(document);
	registry->bExecuting = true;
	registry->bInvokingOwned = true;
	if ( call->pContext != NULL ) call->pContext->bGuardedCallback = true;
	return XUI_OK;
}

static int __xuiCodeLanguageOwnedEnd(xui_code_language_registry registry, xui_code_document document,
	xui_code_language_owned_call_t* call, int status, int* count)
{
	if ( status == XUI_OK && (*count < 0 || xrtGetError() != NULL) )
		status = *count < 0 ? XUI_ERROR_INVALID_STATE : XUI_ERROR_CALLBACK_FAILED;
	if ( status == XUI_OK ) status = xuiInternalCodeDocumentCheckVersions(document, call->iVersion, call->iChangeVersion);
	if ( call->pContext != NULL ) call->pContext->bGuardedCallback = call->bWasGuarded;
	xui_cleanup_receipt_t receipt = {0};
	xuiInternalCleanupStart(&receipt, status);
	if ( call->tScope.tDocument.pSelf != NULL )
		xuiInternalCleanupAdd(&receipt, xuiCodeDocumentCallEnd(&call->tScope.tDocument));
	registry->bInvokingOwned = false;
	registry->bExecuting = call->bBorrowedExecution;
	if ( call->tScope.tOwner.pSelf != NULL )
		receipt.status = xuiInternalCallScopeEndOutcome(&call->tScope.tOwner, receipt.status, &receipt.error);
	status = xuiInternalCleanupEnd(NULL, &receipt);
	if ( status == XUI_OK ) xrtSetErrorTake(call->pAmbient);
	else { xrtErrorFree(call->pAmbient); *count = 0; }
	return status;
}

static int __xuiCodeLanguageLexOwned(xui_code_document document, int firstLine, int endLine,
	xui_code_token_t* tokens, int capacity, int* count, void* user)
{
	xui_code_language_callbacks_t* callbacks = user;
	xui_code_language_owned_call_t call = {0};
	int status = __xuiCodeLanguageOwnedBegin(callbacks->pRegistry, document, &call);
	if ( status != XUI_OK ) return status;
	if ( callbacks->onTokenOutput != NULL ) {
		xui_code_token_output_t output = {tokens, capacity, 0};
		status = callbacks->onTokenOutput(document, firstLine, endLine, &output, callbacks->pUser);
		*count = output.iCount;
	} else status = callbacks->onLex(document, firstLine, endLine, tokens, capacity, count, callbacks->pUser);
	return __xuiCodeLanguageOwnedEnd(callbacks->pRegistry, document, &call, status, count);
}

static int __xuiCodeLanguageFoldOwned(xui_code_document document, xui_code_fold_range_t* ranges,
	int capacity, int* count, void* user)
{
	xui_code_language_callbacks_t* callbacks = user;
	xui_code_language_owned_call_t call = {0};
	int status = __xuiCodeLanguageOwnedBegin(callbacks->pRegistry, document, &call);
	if ( status != XUI_OK ) return status;
	if ( callbacks->onFoldOutput != NULL ) {
		xui_code_fold_output_t output = {ranges, capacity, 0};
		status = callbacks->onFoldOutput(document, &output, callbacks->pUser);
		*count = output.iCount;
	} else status = callbacks->onFold(document, ranges, capacity, count, callbacks->pUser);
	return __xuiCodeLanguageOwnedEnd(callbacks->pRegistry, document, &call, status, count);
}

#endif
