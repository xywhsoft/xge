#include <windows.h>

#include <stdio.h>

static int __check(HMODULE hModule, const char* sName)
{
	if ( GetProcAddress(hModule, sName) != NULL ) return 1;
	printf("test_dll_platform_editor_exports missing: %s\n", sName);
	return 0;
}

static int __check_absent(HMODULE hModule, const char* sName)
{
	if ( GetProcAddress(hModule, sName) == NULL ) return 1;
	printf("test_dll_platform_editor_exports obsolete export: %s\n", sName);
	return 0;
}

int main(int argc, char** argv)
{
	static const char* arrNames[] = {
		"xgePlatformNativeHandle",
		"xuiWidgetIsAttachedToContext",
		"xuiWebViewGetType",
		"xuiWebViewCreate",
		"xuiWebViewInitializeAsync",
		"xuiWebViewFocus",
		"xuiWebViewNavigate",
		"xuiWebViewLoadHtml",
		"xuiWebViewSetZoomFactor",
		"xuiWebViewGetZoomFactor",
		"xuiWebViewMapLocalFolder",
		"xuiWebViewUnmapLocalFolder",
		"xuiDocumentSnapshotGetIdentity",
		"xuiDocumentSnapshotGetSourceLine",
		"xuiDocumentSnapshotGetBlockSyntax",
		"xuiDocumentSnapshotGetBreakSyntax",
		"xuiDocumentSnapshotGetQuotePrefix",
		"xuiDocumentSnapshotGetListContinuationIndent",
		"xuiDocumentSnapshotGetCodeIndent",
		"xuiDocumentSnapshotGetTableToken",
		"xuiDocumentWebProviderCreate",
		"xuiDocumentWebProviderRelease",
		"xuiDocumentWebProviderUpdate",
		"xuiDocumentWebProviderSetPalette",
		"xuiDocumentWebProviderGetStats",
		"xuiDocumentWebObjectMeasure",
		"xuiDocumentWebObjectDraw",
		"xuiDocumentHtmlInteractionCreate",
		"xuiDocumentHtmlInteractionUpdate",
		"xuiDocumentHtmlInteractionShow",
		"xuiDocumentHtmlInteractionGetWindow",
		"xuiDocumentHtmlInteractionGetWebView",
		"xuiDocumentHtmlInteractionRelease",
		"xuiWebViewGetState",
		"xuiWebViewGetNativeError",
		"xuiWebViewClose",
		"xgeDragDropCapsGet",
		"xgeDataObjectCreate",
		"xgeDataObjectSet",
		"xgeDataObjectSetProvider",
		"xgeDragEventCallbackSet",
		"xgeDragEventDispatch",
		"xgeDragBegin",
		"xgeDragCancel",
		"xgeImageInfoMemory",
		"xgeImageEncodePNG",
		"xgeImageEncodePNGEx",
		"xuiDataObjectCreate",
		"xuiResourceGetRegistryGeneration",
		"xuiDragBegin",
		"xuiDragAccept",
		"xuiDragExternalEvent",
		"xuiWidgetSetDropEnabled",
		"xuiCodeEditHitTestText",
		"xuiCodeEditGetTextOffsetRect",
		"xuiTextEditSetWordWrap",
		"xuiTextEditGetWordWrap",
		"xuiCodeEditSetWordWrap",
		"xuiCodeEditGetWordWrap",
		"xuiDocumentCreate",
		"xuiDocumentAcquireSnapshot",
		"xuiDocumentTxnRemoveFootnoteReference",
		"xuiDocumentTxnInsertImage",
		"xuiDocumentTxnUpdateImage",
		"xuiDocumentTxnSetTableColumnWidth",
		"xuiDocumentTxnPasteTableMatrix",
		"xuiDocumentTxnClearTableMatrix",
		"xuiDocumentTxnCreateListRange",
		"xuiDocumentTxnSetListStyleRange",
		"xuiDocumentTxnUnlistRange",
		"xuiDocumentTxnMoveBlockRange",
		"xuiDocumentTxnCopySubtree",
		"xuiDocumentTxnCopyRange",
		"xuiDocumentFragmentCreateRange",
		"xuiDocumentFragmentRetain",
		"xuiDocumentFragmentRelease",
		"xuiDocumentFragmentSerialize",
		"xuiDocumentFragmentExportHtml",
		"xuiDocumentFragmentImportHtml",
		"xuiDocumentFragmentDeserialize",
		"xuiDocumentTxnInsertFragment",
		"xuiDocumentTxnReplaceRangeWithFragment",
		"xuiDocumentSnapshotAnalyzeMarkdownConversion",
		"xuiDocumentSnapshotConvertToMarkdown",
		"xuiDocumentSnapshotConvertToMarkdownWithPolicy",
		"xuiDocumentSnapshotAnalyzeRichConversion",
		"xuiDocumentSnapshotConvertToRich",
		"xuiDocumentSnapshotCopyTableMatrix",
		"xuiDocumentSnapshotGetTableColumnWidth",
		"xuiDocumentImageResourceLoadMemory",
		"xuiDocumentImageResourceLoadFile",
		"xuiDocumentImageResourceLoadFileAsync",
		"xuiDocumentImageResourceLoadMemoryAsync",
		"xuiDocumentImageResourcePoll",
		"xuiDocumentImageResourceCancel",
		"xuiDocumentImageResourceRelease",
		"xuiDocumentImageResourceGetAsyncStats",
		"xuiDocumentEditorCreate",
		"xuiDocumentEditorSetupToolbar",
		"xuiDocumentEditorGetMenuWidget",
		"xuiDocumentEditorOpenMenu",
		"xuiDocumentEditorOpenFind",
		"xuiDocumentEditorOpenReplace",
		"xuiDocumentEditorGetFindWindow",
		"xuiDocumentEditorReplaceCurrent",
		"xuiDocumentEditorReplaceCurrentEx",
		"xuiDocumentEditorReplaceAllEx",
		"xuiDocumentEditorSyncToolbar",
		"xuiDocumentEditorExecuteToolbarItem",
		"xuiDocumentEditorInsertText",
		"xuiDocumentEditorAppendStreamSource",
		"xuiDocumentEditorInsertImage",
		"xuiDocumentEditorRemoveFootnoteReference",
		"xuiDocumentEditorUpdateImage",
		"xuiDocumentEditorUpdateObjectSource",
		"xuiDocumentEditorSetTextStyle",
		"xuiDocumentTxnClearFormatting",
		"xuiDocumentEditorSetTableColumnWidth",
		"xuiDocumentViewSetMode",
		"xuiDocumentRendererHitTestCell",
		"xuiDocumentRendererHitTaskMarker",
		"xuiDocumentRendererGetCellRect",
		"xuiDocumentRendererGetNodeRect",
		"xuiDocumentViewHitTestCell",
		"xuiDocumentViewGetCellRect",
		"xuiDocumentViewGetTableSelection",
		"xuiDocumentViewSetFindQuery",
		"xuiDocumentViewSetFindQueryEx",
		"xuiDocumentViewSetFindScope",
		"xuiDocumentViewGetFindScope",
		"xuiDocumentViewFindEx",
		"xuiDocumentSnapshotFindEx",
		"xuiDocumentTxnReplaceAllEx",
		"xuiDocumentViewClearFind",
		"xuiDocumentViewGetFindResultCount",
		"xuiDocumentViewGetFindResult",
		"xuiDocumentViewActivateFindResult",
		"xuiDocumentRendererInvalidateObjects",
		"xuiDocumentRendererInvalidateFonts",
		"xuiDocumentViewInvalidateObjects",
		"xuiDocumentViewInvalidateFonts",
		"xuiMessageListSetNodeDocument",
		"xuiMessageListGetNodeDocument",
		"xuiMessageListGetNodeDocumentRenderStats",
		"xuiMessageListInvalidateNodeDocumentObjects",
		"xuiMessageListInvalidateNodeDocumentFonts",
		"xuiMessageListHitNodeDocument",
		"xuiMessageListGetNodeDocumentSelection"
	};
	static const char* arrObsolete[] = {
		"xuiRichDocumentCreate", "xuiRichDocumentDeserialize",
		"xuiRichEditCreate", "xuiRichEditGetType",
		"xuiWebViewSetMessageHandler", "xuiWebViewPostMessageJson",
		"xuiWebViewEvalScriptAsync", "xuiWebViewCapturePngAsync",
		"xuiWebRequestRetain", "xuiWebRequestRelease",
		"xuiWebRequestGetState", "xuiWebRequestGetNativeError",
		"xuiWebRequestGetResultJson", "xuiWebRequestGetResultPng",
		"xuiWebRequestCancel"
	};
	HMODULE hModule;
	char sFullPath[MAX_PATH];
	const char* sPath = argc > 1 ? argv[1] : "build\\xge.dll";
	DWORD iPathLength;
	int i;

	iPathLength = GetFullPathNameA(sPath, MAX_PATH, sFullPath, NULL);
	if ( iPathLength == 0 || iPathLength >= MAX_PATH ) {
		printf("test_dll_platform_editor_exports invalid DLL path: %s\n", sPath);
		return 1;
	}
	hModule = LoadLibraryExA(sFullPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if ( hModule == NULL ) {
		printf("test_dll_platform_editor_exports failed to load %s: %lu\n",
			sFullPath, (unsigned long)GetLastError());
		return 1;
	}
	for ( i = 0; i < (int)(sizeof(arrNames) / sizeof(arrNames[0])); i++ ) {
		if ( !__check(hModule, arrNames[i]) ) {
			FreeLibrary(hModule);
			return 1;
		}
	}
	for ( i = 0; i < (int)(sizeof(arrObsolete) / sizeof(arrObsolete[0])); i++ ) {
		if ( !__check_absent(hModule, arrObsolete[i]) ) {
			FreeLibrary(hModule);
			return 1;
		}
	}
	FreeLibrary(hModule);
	printf("test_dll_platform_editor_exports passed\n");
	return 0;
}
