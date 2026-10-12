#include <windows.h>
#include <stdio.h>

static const char* const public_exports[] = {
    "xuiWebViewGetType", "xuiWebViewCreate", "xuiWebViewInitializeAsync",
    "xuiWebViewFocus", "xuiWebViewNavigate", "xuiWebViewLoadHtml",
    "xuiWebViewSetZoomFactor", "xuiWebViewGetZoomFactor",
    "xuiWebViewMapLocalFolder", "xuiWebViewUnmapLocalFolder",
    "xuiWebViewGetState", "xuiWebViewGetNativeError", "xuiWebViewClose",
    "xuiDocumentWebProviderCreate", "xuiDocumentWebProviderRelease",
    "xuiDocumentWebProviderSetPalette", "xuiDocumentWebProviderUpdate",
    "xuiDocumentWebProviderGetStats",
    "xuiDocumentWebObjectMeasure", "xuiDocumentWebObjectDraw"
};
static const char* const private_exports[] = {
    "xuiWebViewSetMessageHandler", "xuiWebViewBlockExternalResources",
    "xuiWebViewGetBlockedResourceCount",
    "xuiWebViewPostMessageJson",
    "xuiWebViewEvalScriptAsync", "xuiWebViewCapturePngAsync",
    "xuiWebRequestRetain", "xuiWebRequestRelease", "xuiWebRequestGetState",
    "xuiWebRequestGetNativeError", "xuiWebRequestGetResultJson",
    "xuiWebRequestGetResultPng", "xuiWebRequestCancel",
    "xuiWebViewTestBrowserProcessId"
};

int main(int argc, char** argv)
{
    HMODULE dll;
    unsigned i;
    if (argc != 2) return 2;
    dll = LoadLibraryA(argv[1]);
    if (!dll) {
        fprintf(stderr, "LoadLibrary failed: %lu\n", (unsigned long)GetLastError());
        return 1;
    }
    for (i = 0; i < sizeof(public_exports) / sizeof(*public_exports); i++) {
        if (!GetProcAddress(dll, public_exports[i])) {
            fprintf(stderr, "missing public export: %s\n", public_exports[i]);
            FreeLibrary(dll); return 1;
        }
    }
    for (i = 0; i < sizeof(private_exports) / sizeof(*private_exports); i++) {
        if (GetProcAddress(dll, private_exports[i])) {
            fprintf(stderr, "internal channel leaked from DLL: %s\n", private_exports[i]);
            FreeLibrary(dll); return 1;
        }
    }
    FreeLibrary(dll);
    printf("XUI DLL exports basic page hosting and Document provider; %u internal browser symbols are absent.\n",
        (unsigned)(sizeof(private_exports) / sizeof(*private_exports)));
    return 0;
}
