#include <windows.h>

#include <stdio.h>

static int __check(HMODULE hModule, const char* sName)
{
	if ( GetProcAddress(hModule, sName) != NULL ) return 1;
	printf("test_dll_platform_editor_exports missing: %s\n", sName);
	return 0;
}

int main(void)
{
	static const char* arrNames[] = {
		"xgePlatformNativeHandle",
		"xgeDragDropCapsGet",
		"xgeDataObjectCreate",
		"xgeDataObjectSet",
		"xgeDataObjectSetProvider",
		"xgeDragEventCallbackSet",
		"xgeDragEventDispatch",
		"xgeDragBegin",
		"xgeDragCancel",
		"xuiDataObjectCreate",
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
		"xuiRichEditSetWordWrap",
		"xuiRichEditGetWordWrap"
	};
	HMODULE hModule;
	int i;

	hModule = LoadLibraryA("build\\xge.dll");
	if ( hModule == NULL ) {
		printf("test_dll_platform_editor_exports failed to load xge.dll: %lu\n",
			(unsigned long)GetLastError());
		return 1;
	}
	for ( i = 0; i < (int)(sizeof(arrNames) / sizeof(arrNames[0])); i++ ) {
		if ( !__check(hModule, arrNames[i]) ) {
			FreeLibrary(hModule);
			return 1;
		}
	}
	FreeLibrary(hModule);
	printf("test_dll_platform_editor_exports passed\n");
	return 0;
}
