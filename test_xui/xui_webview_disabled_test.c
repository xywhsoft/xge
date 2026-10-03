#include <assert.h>
#include <stdio.h>
#include "xui.h"
#include "src/xui_webview_internal.h"

int main(void)
{
    xui_context context = NULL;
    xui_widget view = NULL;
    xui_web_request request = (xui_web_request)(uintptr_t)1;
    xui_webview_desc_t desc = {0};
    desc.iSize = sizeof(desc);
    assert(xuiCreate(&context) == XUI_OK);
    assert(xuiWebViewCreate(context, &view, &desc) == XUI_OK);
    assert(xuiWebViewGetState(view) == XUI_WEBVIEW_CREATED);
    assert(xuiWebViewInitializeAsync(view) == XUI_ERROR_UNSUPPORTED);
    assert(xuiWebViewGetState(view) == XUI_WEBVIEW_CREATED);
    {
        double zoom = 42.0;
        assert(xuiWebViewSetZoomFactor(view, 1.5) == XUI_ERROR_INVALID_STATE);
        assert(xuiWebViewGetZoomFactor(view, &zoom) == XUI_ERROR_INVALID_STATE &&
            zoom == 0.0);
    }
    assert(xuiWebViewEvalScriptAsync(view, "1", 1, &request) ==
        XUI_ERROR_INVALID_STATE && request == NULL);
    request = (xui_web_request)(uintptr_t)1;
    assert(xuiWebViewCapturePngAsync(view, &request) ==
        XUI_ERROR_INVALID_STATE && request == NULL);
    assert(xuiWebViewMapLocalFolder(view, "assets.invalid", "C:\\assets",
        XUI_WEB_LOCAL_ACCESS_SAME_ORIGIN) == XUI_ERROR_INVALID_STATE);
    assert(xuiWebViewUnmapLocalFolder(view, "assets.invalid") ==
        XUI_ERROR_INVALID_STATE);
    assert(xuiWebViewSetMessageHandler(view, NULL, NULL, NULL) ==
        XUI_ERROR_INVALID_STATE);
    assert(xuiWebViewPostMessageJson(view, "{}", 2) ==
        XUI_ERROR_INVALID_STATE);
    assert(xuiWebRequestGetState(NULL) == XUI_ERROR_INVALID_ARGUMENT);
    {
        const void* png = (const void*)(uintptr_t)1;
        size_t bytes = 123;
        assert(xuiWebRequestGetResultPng(NULL, &png, &bytes) ==
            XUI_ERROR_INVALID_ARGUMENT && !png && bytes == 0);
    }
    assert(xuiWebRequestCancel(NULL) == XUI_ERROR_INVALID_ARGUMENT);
    assert(xuiWebViewClose(view) == XUI_OK);
    assert(xuiWebViewGetState(view) == XUI_WEBVIEW_CLOSED);
    xuiWidgetDestroy(view);
    xuiDestroy(context);
    puts("XUI WebView disabled build: basic and internal API unsupported status passed");
    return 0;
}
