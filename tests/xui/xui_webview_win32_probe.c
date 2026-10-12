/* Optional WebView2/MinGW integration probe. Build only with the WebView2 SDK. */
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "WebView2.h"
#ifdef XUI_WEBVIEW_PROBE_XGE
#include "xge.h"
#include "xui.h"
#endif

static HWND g_window;
static ICoreWebView2Controller* g_controller;
static ICoreWebView2* g_view;
static IStream* g_preview;
static EventRegistrationToken g_navigation_token;
static int g_has_navigation_token;
static int g_result = 1;
#ifdef XUI_WEBVIEW_PROBE_XGE
static xui_context g_context;
static xui_proxy_t g_proxy;
static xui_font g_font;
static xui_surface g_target;
static xui_widget g_native_editor;
static int g_xui_captured;
#endif

static void probe_quit(void)
{
#ifdef XUI_WEBVIEW_PROBE_XGE
    xgeQuit();
#else
    PostQuitMessage(0);
#endif
}

static void probe_fail(const char* stage, HRESULT hr)
{
    fprintf(stderr, "WebView2 %s failed: 0x%08lx\n", stage, (unsigned long)hr);
    g_result = 2;
    probe_quit();
}

#define PROBE_HANDLER_BASE(Type, Name, Identifier) \
    static HRESULT STDMETHODCALLTYPE Name##_query(Type* self, REFIID iid, void** out) \
    { \
        if (!out) return E_POINTER; \
        *out = NULL; \
        if (!IsEqualIID(iid, &IID_IUnknown) && !IsEqualIID(iid, &Identifier)) return E_NOINTERFACE; \
        *out = self; return S_OK; \
    } \
    static ULONG STDMETHODCALLTYPE Name##_add(Type* self) { (void)self; return 2; } \
    static ULONG STDMETHODCALLTYPE Name##_release(Type* self) { (void)self; return 1; }

PROBE_HANDLER_BASE(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler, environment,
    IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler)
PROBE_HANDLER_BASE(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler, controller,
    IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler)
PROBE_HANDLER_BASE(ICoreWebView2NavigationCompletedEventHandler, navigation,
    IID_ICoreWebView2NavigationCompletedEventHandler)
PROBE_HANDLER_BASE(ICoreWebView2ExecuteScriptCompletedHandler, script,
    IID_ICoreWebView2ExecuteScriptCompletedHandler)
PROBE_HANDLER_BASE(ICoreWebView2CapturePreviewCompletedHandler, capture,
    IID_ICoreWebView2CapturePreviewCompletedHandler)

static HRESULT STDMETHODCALLTYPE capture_invoke(ICoreWebView2CapturePreviewCompletedHandler* self,
    HRESULT error)
{
    static const unsigned char png_signature[8] = {137,80,78,71,13,10,26,10};
    STATSTG stat; LARGE_INTEGER start = {0}; unsigned char header[24], buffer[8192];
    ULONG read = 0; FILE* output; HRESULT hr; unsigned width, height;
    (void)self;
    if (FAILED(error) || !g_preview) { probe_fail("CapturePreview", error); return S_OK; }
    hr = IStream_Stat(g_preview, &stat, STATFLAG_NONAME);
    if (SUCCEEDED(hr)) hr = IStream_Seek(g_preview, start, STREAM_SEEK_SET, NULL);
    if (SUCCEEDED(hr)) hr = IStream_Read(g_preview, header, sizeof(header), &read);
    if (FAILED(hr) || read != sizeof(header) || stat.cbSize.QuadPart < 128 ||
        memcmp(header, png_signature, sizeof(png_signature))) {
        probe_fail("PNG bytes", hr); return S_OK;
    }
    width = ((unsigned)header[16] << 24) | ((unsigned)header[17] << 16) |
        ((unsigned)header[18] << 8) | header[19];
    height = ((unsigned)header[20] << 24) | ((unsigned)header[21] << 16) |
        ((unsigned)header[22] << 8) | header[23];
#ifdef XUI_WEBVIEW_PROBE_XGE
    if (!g_xui_captured) { probe_fail("XUI pixels", E_FAIL); return S_OK; }
    if (width != 320 || height != 480) { probe_fail("XGE PNG dimensions", E_FAIL); return S_OK; }
    output = fopen("artifacts/xui-webview-probe/capture-xge.png", "wb");
#else
    if (width < 640 || height < 400) { probe_fail("Win32 PNG dimensions", E_FAIL); return S_OK; }
    output = fopen("artifacts/xui-webview-probe/capture-win32.png", "wb");
#endif
    if (!output) { probe_fail("PNG file", E_FAIL); return S_OK; }
    IStream_Seek(g_preview, start, STREAM_SEEK_SET, NULL);
    for (;;) {
        read = 0;
        hr = IStream_Read(g_preview, buffer, sizeof(buffer), &read);
        if (FAILED(hr)) break;
        if (read && fwrite(buffer, 1, read, output) != read) { hr = E_FAIL; break; }
        if (hr == S_FALSE || !read) break;
    }
    fclose(output);
    if (FAILED(hr)) { probe_fail("PNG output", hr); return S_OK; }
    printf("WebView2 PNG preview: %u x %u, %llu bytes\n", width, height,
        (unsigned long long)stat.cbSize.QuadPart);
    puts("WebView2 C/MinGW host window, HTML navigation, DOM script and PNG preview passed");
    g_result = 0; probe_quit();
    return S_OK;
}
static ICoreWebView2CapturePreviewCompletedHandlerVtbl g_capture_vtable = {
    capture_query, capture_add, capture_release, capture_invoke
};
static ICoreWebView2CapturePreviewCompletedHandler g_capture_handler = {&g_capture_vtable};

static HRESULT STDMETHODCALLTYPE script_invoke(ICoreWebView2ExecuteScriptCompletedHandler* self,
    HRESULT error, LPCWSTR result)
{
    HRESULT hr;
    (void)self;
    if (FAILED(error) || !result || wcscmp(result, L"\"xui-w0:W0\"")) {
        probe_fail("DOM script", FAILED(error) ? error : E_FAIL);
    } else {
        hr = CreateStreamOnHGlobal(NULL, TRUE, &g_preview);
        if (SUCCEEDED(hr)) hr = ICoreWebView2_CapturePreview(g_view,
            COREWEBVIEW2_CAPTURE_PREVIEW_IMAGE_FORMAT_PNG, g_preview, &g_capture_handler);
        if (FAILED(hr)) probe_fail("start CapturePreview", hr);
    }
    return S_OK;
}
static ICoreWebView2ExecuteScriptCompletedHandlerVtbl g_script_vtable = {
    script_query, script_add, script_release, script_invoke
};
static ICoreWebView2ExecuteScriptCompletedHandler g_script_handler = {&g_script_vtable};

static HRESULT STDMETHODCALLTYPE navigation_invoke(ICoreWebView2NavigationCompletedEventHandler* self,
    ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args)
{
    BOOL success = FALSE; HRESULT hr;
    (void)self;
    hr = ICoreWebView2NavigationCompletedEventArgs_get_IsSuccess(args, &success);
    if (FAILED(hr) || !success) { probe_fail("navigation", FAILED(hr) ? hr : E_FAIL); return S_OK; }
    hr = ICoreWebView2_ExecuteScript(sender,
        L"document.title + ':' + document.querySelector('h1').textContent", &g_script_handler);
    if (FAILED(hr)) probe_fail("ExecuteScript", hr);
    return S_OK;
}
static ICoreWebView2NavigationCompletedEventHandlerVtbl g_navigation_vtable = {
    navigation_query, navigation_add, navigation_release, navigation_invoke
};
static ICoreWebView2NavigationCompletedEventHandler g_navigation_handler = {&g_navigation_vtable};

static HRESULT STDMETHODCALLTYPE controller_invoke(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* self,
    HRESULT error, ICoreWebView2Controller* controller)
{
    RECT bounds; HRESULT hr;
    (void)self;
    if (FAILED(error) || !controller) { probe_fail("controller", error); return S_OK; }
    g_controller = controller;
    ICoreWebView2Controller_AddRef(controller);
    GetClientRect(g_window, &bounds);
#ifdef XUI_WEBVIEW_PROBE_XGE
    bounds.left = 320;
#endif
    hr = ICoreWebView2Controller_put_Bounds(controller, bounds);
    if (SUCCEEDED(hr)) hr = ICoreWebView2Controller_get_CoreWebView2(controller, &g_view);
    if (SUCCEEDED(hr)) hr = ICoreWebView2_add_NavigationCompleted(g_view,
        &g_navigation_handler, &g_navigation_token);
    if (SUCCEEDED(hr)) g_has_navigation_token = 1;
    if (SUCCEEDED(hr)) hr = ICoreWebView2_NavigateToString(g_view,
        L"<!doctype html><title>xui-w0</title><h1>W0</h1><textarea>input</textarea>");
    if (FAILED(hr)) probe_fail("NavigateToString", hr);
    return S_OK;
}
static ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl g_controller_vtable = {
    controller_query, controller_add, controller_release, controller_invoke
};
static ICoreWebView2CreateCoreWebView2ControllerCompletedHandler g_controller_handler = {&g_controller_vtable};

static HRESULT STDMETHODCALLTYPE environment_invoke(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* self,
    HRESULT error, ICoreWebView2Environment* environment)
{
    HRESULT hr;
    (void)self;
    if (FAILED(error) || !environment) { probe_fail("environment", error); return S_OK; }
    hr = ICoreWebView2Environment_CreateCoreWebView2Controller(environment, g_window, &g_controller_handler);
    if (FAILED(hr)) probe_fail("CreateCoreWebView2Controller", hr);
    return S_OK;
}
static ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl g_environment_vtable = {
    environment_query, environment_add, environment_release, environment_invoke
};
static ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler g_environment_handler = {&g_environment_vtable};

static HRESULT probe_start(HWND window)
{
    wchar_t profile[MAX_PATH]; DWORD length;
    g_window = window;
    length = GetTempPathW(MAX_PATH, profile);
    if (!length || length > MAX_PATH - 18) return E_FAIL;
    wcscat(profile, L"xui-webview-w0");
    return CreateCoreWebView2EnvironmentWithOptions(NULL, profile, NULL, &g_environment_handler);
}

static void probe_close(void)
{
    if (g_view && g_has_navigation_token)
        ICoreWebView2_remove_NavigationCompleted(g_view, g_navigation_token);
    if (g_controller) ICoreWebView2Controller_Close(g_controller);
    if (g_view) ICoreWebView2_Release(g_view);
    if (g_controller) ICoreWebView2Controller_Release(g_controller);
    if (g_preview) IStream_Release(g_preview);
}

#ifdef XUI_WEBVIEW_PROBE_XGE
static int probe_setup_xui(void)
{
    xui_widget root, label; xui_label_desc_t caption = {0};
    xui_text_edit_desc_t edit = {0}; xui_surface_desc_t surface = {0};
    g_proxy = xuiProxyXge();
    if (xuiCreate(&g_context) != XUI_OK || xuiSetProxy(g_context, &g_proxy) != XUI_OK ||
        g_proxy.fontLoadFile(&g_proxy, &g_font, "C:\\Windows\\Fonts\\segoeui.ttf", 16, XUI_FONT_FORMAT_TTF) != XUI_OK ||
        xuiSetDefaultFont(g_context, g_font) != XUI_OK ||
        xuiInputViewport(g_context, 640, 480) != XUI_OK) return 0;
    surface.iKind = XUI_SURFACE_KIND_TEXTURE; surface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
    surface.iWidth = 640; surface.iHeight = 480;
    surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
    if (g_proxy.surfaceCreate(&g_proxy, &g_target, &surface) != XUI_OK ||
        xuiWidgetCreate(g_context, &root) != XUI_OK ||
        xuiSetRootWidget(g_context, root) != XUI_OK ||
        xuiWidgetSetRect(root, (xui_rect_t){0,0,640,480}) != XUI_OK) return 0;
    caption.iSize = sizeof(caption); caption.sText = "Native XUI editor + WebView2"; caption.pFont = g_font;
    if (xuiLabelCreate(g_context, &label, &caption) != XUI_OK ||
        xuiWidgetSetRect(label, (xui_rect_t){20,24,280,35}) != XUI_OK ||
        xuiWidgetAddChild(root, label) != XUI_OK) return 0;
    edit.iSize = sizeof(edit); edit.sText = "Native XUI text input"; edit.pFont = g_font;
    if (xuiTextEditCreate(g_context, &g_native_editor, &edit) != XUI_OK ||
        xuiWidgetSetRect(g_native_editor, (xui_rect_t){20,80,280,180}) != XUI_OK ||
        xuiWidgetAddChild(root, g_native_editor) != XUI_OK ||
        strcmp(xuiTextEditGetText(g_native_editor), "Native XUI text input")) return 0;
    return 1;
}

static int xge_frame(void* user)
{
    static int frames; HRESULT hr; xui_rect_i_t damage = {0,0,640,480};
    (void)user;
    if (!g_window) {
        HWND window = (HWND)xgePlatformNativeHandle();
        if (!window) { probe_fail("XGE HWND", E_FAIL); return XGE_ERROR; }
        if (!probe_setup_xui()) { probe_fail("XUI setup", E_FAIL); return XGE_ERROR; }
        hr = probe_start(window);
        if (FAILED(hr)) { probe_fail("XGE environment", hr); return XGE_ERROR; }
    }
    if (++frames > 1800) { probe_fail("XGE timeout", HRESULT_FROM_WIN32(ERROR_TIMEOUT)); return XGE_ERROR; }
    xuiProxyXgePumpInputRect(g_context, (xui_rect_t){0,0,320,480});
    xuiDispatchPendingEvents(g_context);
    if (xuiLayout(g_context) != XUI_OK || xuiUpdate(g_context, .016f) != XUI_OK) {
        probe_fail("XUI update", E_FAIL); return XGE_ERROR;
    }
    if (xgeBegin() != XGE_OK) { probe_fail("XGE begin", E_FAIL); return XGE_ERROR; }
    xgeClear(0xffedf0f5);
    if (g_proxy.surfaceClear(&g_proxy, g_target, XUI_COLOR_WHITE) != XUI_OK) {
        probe_fail("XUI clear", E_FAIL); return XGE_ERROR;
    }
    if (xuiRender(g_context, g_target, &damage, 1) != XUI_OK) {
        probe_fail("XUI render", E_FAIL); return XGE_ERROR;
    }
    if (g_proxy.surfaceDraw(&g_proxy, g_target, (xui_rect_t){0,0,640,480},
            (xui_rect_t){0,0,640,480}, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE) != XUI_OK) {
        probe_fail("XUI composite", E_FAIL); return XGE_ERROR;
    }
    if (xgeEnd() != XGE_OK) { probe_fail("XGE end", E_FAIL); return XGE_ERROR; }
    if (!g_xui_captured) {
        unsigned char* pixels = malloc(640u * 480u * 4u); unsigned dark = 0, x, y;
        if (!pixels) { probe_fail("XUI pixels allocation", E_OUTOFMEMORY); return XGE_ERROR; }
        if (g_proxy.surfaceReadRGBA(&g_proxy, g_target, pixels, 640 * 4) == XUI_OK) {
            for (y = 0; y < 480; y++) for (x = 0; x < 320; x++) {
                const unsigned char* p = pixels + ((size_t)y * 640 + x) * 4;
                if (p[0] < 100 && p[1] < 100 && p[2] < 100) dark++;
            }
            if (dark >= 100 && xgeImageSavePNG("artifacts/xui-webview-probe/capture-xui.png",
                    640, 480, pixels, 640 * 4) == XGE_OK) g_xui_captured = 1;
        }
        free(pixels);
        if (!g_xui_captured) { probe_fail("XUI pixels", E_FAIL); return XGE_ERROR; }
    }
    return XGE_OK;
}
#else
static LRESULT CALLBACK probe_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_SIZE && g_controller) {
        RECT bounds;
        GetClientRect(window, &bounds);
        ICoreWebView2Controller_put_Bounds(g_controller, bounds);
    } else if (message == WM_TIMER) {
        KillTimer(window, 1); probe_fail("timeout", HRESULT_FROM_WIN32(ERROR_TIMEOUT));
    } else if (message == WM_DESTROY) {
        PostQuitMessage(0);
    }
    return DefWindowProcW(window, message, wparam, lparam);
}
#endif

int main(void)
{
    HRESULT hr; LPWSTR version = NULL;
#ifdef XUI_WEBVIEW_PROBE_XGE
    xge_desc_t desc = {0};
#else
    WNDCLASSW cls = {0}; MSG message;
#endif
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) { probe_fail("STA", hr); return 2; }
    hr = GetAvailableCoreWebView2BrowserVersionString(NULL, &version);
    if (FAILED(hr)) { probe_fail("runtime", hr); CoUninitialize(); return 2; }
    wprintf(L"WebView2 Runtime: %ls\n", version); CoTaskMemFree(version);
#ifdef XUI_WEBVIEW_PROBE_XGE
    desc.iWidth = 640; desc.iHeight = 480; desc.sTitle = "XUI WebView2 XGE W0 probe";
    desc.iRunMode = XGE_RUN_GAME_LOOP; desc.iTargetFPS = 60;
    if (xgeInit(&desc) != XGE_OK) { CoUninitialize(); return 2; }
    xgeRun(xge_frame, NULL);
    probe_close();
    if (g_context) xuiDestroy(g_context);
    if (g_proxy.surfaceDestroy && g_target) g_proxy.surfaceDestroy(&g_proxy, g_target);
    if (g_proxy.fontDestroy && g_font) g_proxy.fontDestroy(&g_proxy, g_font);
    xgeUnit(); CoUninitialize();
    return g_result;
#else
    cls.lpfnWndProc = probe_window_proc; cls.hInstance = GetModuleHandleW(NULL);
    cls.lpszClassName = L"XuiWebViewW0Probe";
    if (!RegisterClassW(&cls)) { CoUninitialize(); return 2; }
    g_window = CreateWindowExW(0, cls.lpszClassName, L"XUI WebView2 W0 probe",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 660, 500,
        NULL, NULL, cls.hInstance, NULL);
    if (!g_window) { UnregisterClassW(cls.lpszClassName, cls.hInstance); CoUninitialize(); return 2; }
    ShowWindow(g_window, SW_SHOWNOACTIVATE);
    SetTimer(g_window, 1, 30000, NULL);
    hr = probe_start(g_window);
    if (FAILED(hr)) { probe_fail("start environment", hr); goto done; }
    while (GetMessageW(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message); DispatchMessageW(&message);
    }
done:
    probe_close();
    DestroyWindow(g_window);
    UnregisterClassW(cls.lpszClassName, cls.hInstance);
    CoUninitialize();
    return g_result;
#endif
}
