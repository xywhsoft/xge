/* Real XGE window integration for the optional public XUI WebView widget. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xge.h"
#include "xui.h"
#include "src/xui_webview_internal.h"

static xui_context context;
static xui_widget root, clip_parent, webview, webview_second;
static xui_widget before_focus, after_focus, alternate_root;
static HWND window, host;
static int phase, frames, navigated, closed, reentrant_done, result = 1;
static int lifecycle_cycle, lifecycle_navigation, lifecycle_closed;
static int concurrent_first_navigation, concurrent_second_navigation;
static int concurrent_first_closed, concurrent_second_closed;
static int crash_navigated, crash_failed, crash_closed;
static int recovery_navigated, recovery_closed;
static int32_t crash_error;
static char crash_folder[MAX_PATH];
static int text_phase_start;
static xui_web_request focus_request, text_request;

static void fail(const char* step)
{
    fprintf(stderr, "WebView widget test failed: %s, state=%d, native=0x%08x\n",
        step, webview ? xuiWebViewGetState(webview) : -1,
        webview ? (unsigned)xuiWebViewGetNativeError(webview) : 0);
    result = 2;
    xgeQuit();
}

static int host_rect_is(int x, int y, int width, int height)
{
    RECT rect;
    if (!host || !IsWindow(host) || !GetWindowRect(host, &rect)) return 0;
    MapWindowPoints(NULL, window, (POINT*)&rect, 2);
    return rect.left == x && rect.top == y &&
        rect.right - rect.left == width && rect.bottom - rect.top == height;
}

static int browser_has_native_focus(void)
{
    HWND focused = GetFocus();
    return host && focused && (focused == host || IsChild(host, focused));
}

static int send_tab(int reverse)
{
    INPUT input[4] = {{0}};
    int count = 0;
    if (GetForegroundWindow() != window) return 0;
    if (reverse) {
        input[count].type = INPUT_KEYBOARD;
        input[count++].ki.wVk = VK_SHIFT;
    }
    input[count].type = INPUT_KEYBOARD;
    input[count++].ki.wVk = VK_TAB;
    input[count] = input[count - 1];
    input[count++].ki.dwFlags = KEYEVENTF_KEYUP;
    if (reverse) {
        input[count] = input[0];
        input[count++].ki.dwFlags = KEYEVENTF_KEYUP;
    }
    return SendInput((UINT)count, input, sizeof(INPUT)) == (UINT)count;
}

static int send_browser_click(void)
{
    POINT point = {400, 120};
    INPUT input[2] = {{0}};
    if (!ClientToScreen(window, &point) || !SetCursorPos(point.x, point.y)) return 0;
    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    return SendInput(2, input, sizeof(INPUT)) == 2;
}

static int send_unicode_text(void)
{
    static const WORD characters[] = {0x4e2d, 0x6587, 'A'};
    INPUT input[sizeof(characters) / sizeof(characters[0]) * 2] = {{0}};
    size_t i;
    if (GetForegroundWindow() != window || !browser_has_native_focus()) return 0;
    for (i = 0; i < sizeof(characters) / sizeof(characters[0]); i++) {
        input[i * 2].type = INPUT_KEYBOARD;
        input[i * 2].ki.wScan = characters[i];
        input[i * 2].ki.dwFlags = KEYEVENTF_UNICODE;
        input[i * 2 + 1] = input[i * 2];
        input[i * 2 + 1].ki.dwFlags |= KEYEVENTF_KEYUP;
    }
    return SendInput((UINT)(sizeof(input) / sizeof(input[0])), input,
        sizeof(INPUT)) == (UINT)(sizeof(input) / sizeof(input[0]));
}

static void on_webview(xui_widget widget, int event, int32_t error, void* user)
{
    static const char html[] = "<!doctype html><title>XUI widget</title>"
        "<h1>WebView widget</h1><textarea>input</textarea>";
    (void)user;
    if (event == XUI_WEBVIEW_EVENT_FAILED ||
            event == XUI_WEBVIEW_EVENT_NAVIGATION_FAILED) {
        fprintf(stderr, "WebView event %d: 0x%08x\n", event, (unsigned)error);
        fail("async event");
    } else if (event == XUI_WEBVIEW_EVENT_READY) {
        if (xuiWebViewLoadHtml(widget, html, sizeof(html) - 1) != XUI_OK)
            fail("LoadHtml");
    } else if (event == XUI_WEBVIEW_EVENT_NAVIGATION_COMPLETE) {
        navigated++;
    } else if (event == XUI_WEBVIEW_EVENT_CLOSED) {
        closed++;
    }
}

static void on_reentrant_webview(xui_widget widget, int event, int32_t error, void* user)
{
    (void)error; (void)user;
    if (event == XUI_WEBVIEW_EVENT_READY) {
        if (xuiWebViewClose(widget) != XUI_OK) {
            fail("reentrant close"); return;
        }
        xuiWidgetDestroy(widget);
        webview = NULL;
        reentrant_done = 1;
    } else if (event == XUI_WEBVIEW_EVENT_FAILED) {
        fail("reentrant initialization");
    }
}

static void on_concurrent_webview(xui_widget widget, int event, int32_t error,
    void* user)
{
    static const char first_html[] = "<!doctype html><title>first</title><p>first</p>";
    static const char second_html[] = "<!doctype html><title>second</title><p>second</p>";
    int first = widget == webview;
    (void)user;
    if (!first && widget != webview_second) {
        fail("concurrent callback widget"); return;
    }
    if (event == XUI_WEBVIEW_EVENT_FAILED ||
        event == XUI_WEBVIEW_EVENT_NAVIGATION_FAILED) {
        fprintf(stderr, "Concurrent WebView event %d: 0x%08x\n",
            event, (unsigned)error);
        fail("concurrent async event");
    } else if (event == XUI_WEBVIEW_EVENT_READY) {
        const char* html = first ? first_html : second_html;
        if (xuiWebViewLoadHtml(widget, html, strlen(html)) != XUI_OK)
            fail("concurrent LoadHtml");
    } else if (event == XUI_WEBVIEW_EVENT_NAVIGATION_COMPLETE) {
        if (first) concurrent_first_navigation++;
        else concurrent_second_navigation++;
    } else if (event == XUI_WEBVIEW_EVENT_CLOSED) {
        if (first) concurrent_first_closed++;
        else concurrent_second_closed++;
    }
}

static void on_crash_webview(xui_widget widget, int event, int32_t error,
    void* user)
{
    static const char html[] = "<!doctype html><title>isolated failure</title>";
    (void)user;
    if (event == XUI_WEBVIEW_EVENT_READY) {
        if (xuiWebViewLoadHtml(widget, html, sizeof(html) - 1) != XUI_OK)
            fail("isolated failure LoadHtml");
    } else if (event == XUI_WEBVIEW_EVENT_NAVIGATION_COMPLETE) {
        crash_navigated++;
    } else if (event == XUI_WEBVIEW_EVENT_FAILED) {
        crash_failed++;
        crash_error = error;
    } else if (event == XUI_WEBVIEW_EVENT_CLOSED) {
        crash_closed++;
    }
}

static void on_recovery_webview(xui_widget widget, int event, int32_t error,
    void* user)
{
    static const char html[] = "<!doctype html><title>recovered</title>";
    (void)user;
    if (event == XUI_WEBVIEW_EVENT_READY) {
        if (xuiWebViewLoadHtml(widget, html, sizeof(html) - 1) != XUI_OK)
            fail("recovery LoadHtml");
    } else if (event == XUI_WEBVIEW_EVENT_NAVIGATION_COMPLETE) {
        recovery_navigated++;
    } else if (event == XUI_WEBVIEW_EVENT_FAILED ||
               event == XUI_WEBVIEW_EVENT_NAVIGATION_FAILED) {
        fprintf(stderr, "Recovery WebView event %d: 0x%08x\n",
            event, (unsigned)error);
        fail("recovery async event");
    } else if (event == XUI_WEBVIEW_EVENT_CLOSED) {
        recovery_closed++;
    }
}

static int setup(void)
{
    xui_webview_desc_t desc = {0};
    char path[MAX_PATH];
    DWORD length;
    if (xuiCreate(&context) != XUI_OK ||
        xuiWidgetCreate(context, &root) != XUI_OK ||
        xuiSetRootWidget(context, root) != XUI_OK ||
        xuiWidgetSetRect(root, (xui_rect_t){0, 0, 640, 480}) != XUI_OK ||
        xuiWidgetCreate(context, &before_focus) != XUI_OK ||
        xuiWidgetSetRect(before_focus, (xui_rect_t){10, 10, 100, 30}) != XUI_OK ||
        xuiWidgetSetFocusable(before_focus, 1) != XUI_OK ||
        xuiWidgetSetTabIndex(before_focus, 1) != XUI_OK ||
        xuiWidgetAddChild(root, before_focus) != XUI_OK ||
        xuiWidgetCreate(context, &clip_parent) != XUI_OK ||
        xuiWidgetSetRect(clip_parent, (xui_rect_t){320, 40, 200, 300}) != XUI_OK ||
        xuiWidgetSetOverflow(clip_parent, XUI_OVERFLOW_HIDDEN) != XUI_OK ||
        xuiWidgetAddChild(root, clip_parent) != XUI_OK ||
        xuiWidgetCreate(context, &after_focus) != XUI_OK ||
        xuiWidgetSetRect(after_focus, (xui_rect_t){10, 60, 100, 30}) != XUI_OK ||
        xuiWidgetSetFocusable(after_focus, 1) != XUI_OK ||
        xuiWidgetSetTabIndex(after_focus, 3) != XUI_OK ||
        xuiWidgetAddChild(root, after_focus) != XUI_OK) return 0;
    length = GetTempPathA(MAX_PATH, path);
    if (!length || length > MAX_PATH - 32) return 0;
    strcat(path, "xui-webview-widget-test");
    desc.iSize = sizeof(desc);
    desc.sUserDataFolder = path;
    desc.onEvent = on_webview;
    if (xuiWebViewCreate(context, &webview, &desc) != XUI_OK ||
        xuiWidgetSetRect(webview, (xui_rect_t){-50, 10, 320, 200}) != XUI_OK ||
        xuiWidgetSetTabIndex(webview, 2) != XUI_OK ||
        xuiWidgetAddChild(clip_parent, webview) != XUI_OK ||
        xuiWebViewInitializeAsync(webview) != XUI_OK) return 0;
    return 1;
}

static int frame(void* user)
{
    (void)user;
    if (!window) {
        window = (HWND)xgePlatformNativeHandle();
        if (!window || !setup()) { fail("setup"); return XGE_ERROR; }
    }
    if (++frames > 900) { fail("timeout"); return XGE_ERROR; }
    if (xuiDispatchPendingEvents(context) != XUI_OK ||
            xuiLayout(context) != XUI_OK || xuiUpdate(context, .016f) != XUI_OK) {
        fail("XUI layout/update"); return XGE_ERROR;
    }
    if (phase == 0 && navigated) {
        xge_platform_runtime_t runtime = {0};
        RECT client = {0};
        double zoom = 0, initial_zoom = 0;
        if (xgePlatformRuntimeGet(&runtime) != XGE_OK ||
                !GetClientRect(window, &client)) {
            fail("DPI runtime/client geometry"); return XGE_ERROR;
        }
        printf("XUI WebView geometry: client=%ldx%ld window=%dx%d "
            "framebuffer=%dx%d dpi=%.3f\n",
            (long)client.right, (long)client.bottom,
            runtime.iWindowWidth, runtime.iWindowHeight,
            runtime.iFramebufferWidth, runtime.iFramebufferHeight,
            (double)runtime.fDpiScale);
        host = FindWindowExW(window, NULL, L"Static", NULL);
        if (xuiWebViewGetState(webview) != XUI_WEBVIEW_READY ||
            !host_rect_is(320, 50, 200, 200) || !IsWindowVisible(host)) {
            fail("initial clipped bounds"); return XGE_ERROR;
        }
        if (xuiWebViewGetZoomFactor(webview, &initial_zoom) != XUI_OK ||
                initial_zoom < 0.25 || initial_zoom > 5.0 ||
                xuiWebViewSetZoomFactor(webview, 1.5) != XUI_OK ||
                xuiWebViewGetZoomFactor(webview, &zoom) != XUI_OK || zoom != 1.5 ||
                !host_rect_is(320, 50, 200, 200) ||
                xuiWebViewSetZoomFactor(webview, 0.0) != XUI_ERROR_INVALID_ARGUMENT ||
                xuiWebViewSetZoomFactor(webview, 5.1) != XUI_ERROR_INVALID_ARGUMENT ||
                xuiWebViewGetZoomFactor(webview, NULL) != XUI_ERROR_INVALID_ARGUMENT ||
                xuiWebViewSetZoomFactor(webview, initial_zoom) != XUI_OK ||
                xuiSetVirtualDpi(context, 1.5f) != XUI_OK ||
                xuiLayout(context) != XUI_OK || xuiUpdate(context, .016f) != XUI_OK ||
                !host_rect_is(320, 50, 200, 200) ||
                xuiSetVirtualDpi(context, 1.0f) != XUI_OK) {
            fail("page zoom and virtual DPI geometry"); return XGE_ERROR;
        }
        SetFocus(window);
        if (xuiSetFocusWidget(context, before_focus) != XUI_OK ||
                xuiDispatchPendingEvents(context) != XUI_OK ||
                xuiFocusNext(context, 1) != XUI_OK ||
                xuiDispatchPendingEvents(context) != XUI_OK) {
            fail("XUI Tab into browser"); return XGE_ERROR;
        }
        phase = 10;
    } else if (phase == 10) {
        if (xuiGetFocusWidget(context) != webview || !browser_has_native_focus()) {
            fail("XUI/native browser focus"); return XGE_ERROR;
        }
        if (getenv("XUI_WEBVIEW_SYNTHETIC_TEXT")) {
            static const char script[] =
                "const t=document.querySelector('textarea');t.value='';t.focus();true";
            SetForegroundWindow(window);
            if (GetForegroundWindow() != window ||
                    xuiWebViewEvalScriptAsync(webview, script,
                        sizeof(script) - 1, &focus_request) != XUI_OK) {
                fail("synthetic text focus setup"); return XGE_ERROR;
            }
            phase = 50;
        } else if (xuiWidgetSetVisible(clip_parent, 0) != XUI_OK) {
            fail("hide parent"); return XGE_ERROR;
        } else {
            phase = 1;
        }
    } else if (phase == 50 &&
            xuiWebRequestGetState(focus_request) != XUI_WEB_REQUEST_PENDING) {
        if (xuiWebRequestGetState(focus_request) != XUI_WEB_REQUEST_COMPLETED ||
                !send_unicode_text()) {
            fail("synthetic Unicode text delivery"); return XGE_ERROR;
        }
        xuiWebRequestRelease(focus_request);
        focus_request = NULL;
        text_phase_start = frames;
        phase = 51;
    } else if (phase == 51 && frames > text_phase_start + 12) {
        static const char script[] =
            "document.querySelector('textarea').value==='\\u4e2d\\u6587A'";
        if (xuiWebViewEvalScriptAsync(webview, script,
                    sizeof(script) - 1, &text_request) != XUI_OK) {
            fail("synthetic Unicode text query"); return XGE_ERROR;
        }
        phase = 52;
    } else if (phase == 52 &&
            xuiWebRequestGetState(text_request) != XUI_WEB_REQUEST_PENDING) {
        const char* json = NULL;
        size_t bytes = 0;
        if (xuiWebRequestGetState(text_request) != XUI_WEB_REQUEST_COMPLETED ||
                xuiWebRequestGetResultJson(text_request, &json, &bytes) != XUI_OK ||
                bytes != 4 || memcmp(json, "true", 4) != 0) {
            fail("synthetic Unicode text value"); return XGE_ERROR;
        }
        xuiWebRequestRelease(text_request);
        text_request = NULL;
        puts("XUI WebView2 system-injected Unicode textarea input passed");
        if (xuiWidgetSetVisible(clip_parent, 0) != XUI_OK) {
            fail("hide parent after text"); return XGE_ERROR;
        }
        phase = 1;
    } else if (phase == 1) {
        if (IsWindowVisible(host) || browser_has_native_focus() ||
                xuiGetFocusWidget(context) == webview) {
            fail("hidden native host/focus"); return XGE_ERROR;
        }
        if (xuiWidgetSetVisible(clip_parent, 1) != XUI_OK) {
            fail("show parent"); return XGE_ERROR;
        }
        phase = 2;
    } else if (phase == 2) {
        if (!IsWindowVisible(host) || !host_rect_is(320, 50, 200, 200)) {
            fail("restored native host"); return XGE_ERROR;
        }
        if (xuiWebViewFocus(webview) != XUI_OK) {
            fail("restore browser focus"); return XGE_ERROR;
        }
        phase = 20;
    } else if (phase == 20) {
        if (xuiGetFocusWidget(context) != webview || !browser_has_native_focus()) {
            fail("restored browser focus"); return XGE_ERROR;
        }
        SetFocus(window);
        phase = 26;
    } else if (phase == 26) {
        if (browser_has_native_focus() || xuiGetFocusWidget(context) == webview) {
            fail("native focus exit"); return XGE_ERROR;
        }
        if (getenv("XUI_WEBVIEW_SYNTHETIC_POINTER")) {
            SetForegroundWindow(window);
            if (xuiSetFocusWidget(context, after_focus) != XUI_OK ||
                    xuiDispatchPendingEvents(context) != XUI_OK ||
                    GetForegroundWindow() != window || !send_browser_click()) {
                fail("synthetic browser click"); return XGE_ERROR;
            }
            phase = 36;
            frames = 0;
        } else if (xuiWebViewFocus(webview) != XUI_OK) {
            fail("native focus exit and restore"); return XGE_ERROR;
        } else {
            phase = 27;
        }
    } else if (phase == 36) {
        if (xuiGetFocusWidget(context) == webview && browser_has_native_focus()) {
            phase = 27;
        } else if (frames > 120) {
            fail("browser click to XUI focus"); return XGE_ERROR;
        }
    } else if (phase == 27) {
        if (xuiGetFocusWidget(context) != webview || !browser_has_native_focus()) {
            fail("native focus restore"); return XGE_ERROR;
        }
        if (xuiWidgetSetEnabled(clip_parent, 0) != XUI_OK) {
            fail("disable browser parent"); return XGE_ERROR;
        }
        phase = 28;
    } else if (phase == 28) {
        if (!IsWindowVisible(host) || IsWindowEnabled(host) ||
                browser_has_native_focus() || xuiGetFocusWidget(context) == webview ||
                xuiWebViewFocus(webview) != XUI_ERROR_INVALID_STATE ||
                xuiWidgetSetEnabled(clip_parent, 1) != XUI_OK) {
            fail("disabled browser input"); return XGE_ERROR;
        }
        phase = 29;
    } else if (phase == 29) {
        if (!IsWindowEnabled(host) || xuiWebViewFocus(webview) != XUI_OK) {
            fail("enabled browser input"); return XGE_ERROR;
        }
        phase = 37;
    } else if (phase == 37) {
        if (xuiGetFocusWidget(context) != webview || !browser_has_native_focus()) {
            fail("enabled browser focus"); return XGE_ERROR;
        }
        if (getenv("XUI_WEBVIEW_SYNTHETIC_TAB")) {
            SetForegroundWindow(window);
            if (GetForegroundWindow() != window ||
                    xuiWebViewFocus(webview) != XUI_OK || !send_tab(0)) {
                fail("synthetic Tab input"); return XGE_ERROR;
            }
            phase = 22;
            frames = 0;
            goto render;
        }
        if (xuiFocusNext(context, 1) != XUI_OK ||
                xuiDispatchPendingEvents(context) != XUI_OK) {
            fail("XUI Tab out of browser"); return XGE_ERROR;
        }
        phase = 21;
    } else if (phase == 22) {
        if (xuiGetFocusWidget(context) == after_focus) {
            if (browser_has_native_focus() ||
                    xuiFocusNext(context, 0) != XUI_OK ||
                    xuiDispatchPendingEvents(context) != XUI_OK) {
                fail("browser Tab native handoff"); return XGE_ERROR;
            }
            phase = 23;
        } else if (frames % 24 == 0 && frames < 120) {
            if (!send_tab(0)) { fail("repeat synthetic Tab"); return XGE_ERROR; }
        } else if (frames >= 120) {
            fail("browser Tab to XUI"); return XGE_ERROR;
        }
    } else if (phase == 23) {
        if (xuiGetFocusWidget(context) != webview || !browser_has_native_focus() ||
                !send_tab(1)) {
            fail("Shift+Tab input"); return XGE_ERROR;
        }
        phase = 24;
        frames = 0;
    } else if (phase == 24) {
        if (xuiGetFocusWidget(context) == before_focus) {
            if (browser_has_native_focus() ||
                    xuiSetFocusWidget(context, after_focus) != XUI_OK ||
                    xuiDispatchPendingEvents(context) != XUI_OK) {
                fail("browser Shift+Tab native handoff"); return XGE_ERROR;
            }
            phase = 21;
        } else if (frames % 24 == 0 && frames < 120) {
            if (!send_tab(1)) { fail("repeat Shift+Tab"); return XGE_ERROR; }
        } else if (frames >= 120) {
            fail("browser Shift+Tab to XUI"); return XGE_ERROR;
        }
    } else if (phase == 21) {
        if (xuiGetFocusWidget(context) != after_focus || browser_has_native_focus()) {
            fail("native/XUI focus handoff"); return XGE_ERROR;
        }
        if (xuiWidgetSetRect(clip_parent, (xui_rect_t){350, 60, 200, 300}) != XUI_OK) {
            fail("move parent"); return XGE_ERROR;
        }
        phase = 3;
    } else if (phase == 3) {
        if (!host_rect_is(350, 70, 200, 200)) {
            fail("moved clipped bounds"); return XGE_ERROR;
        }
        if (xuiWebViewFocus(webview) != XUI_OK ||
                xuiWidgetSetRect(clip_parent, (xui_rect_t){700, 60, 200, 300}) != XUI_OK) {
            fail("move focused browser outside window"); return XGE_ERROR;
        }
        phase = 30;
    } else if (phase == 30) {
        if (IsWindowVisible(host) || browser_has_native_focus() ||
                xuiGetFocusWidget(context) == webview) {
            fail("fully clipped browser focus"); return XGE_ERROR;
        }
        if (xuiWidgetSetRect(clip_parent, (xui_rect_t){350, 60, 200, 300}) != XUI_OK) {
            fail("restore clipped browser"); return XGE_ERROR;
        }
        phase = 31;
    } else if (phase == 31) {
        if (!IsWindowVisible(host) || !host_rect_is(350, 70, 200, 200) ||
                xuiWebViewFocus(webview) != XUI_OK ||
                xuiWidgetRemoveFromParent(webview) != XUI_OK ||
                IsWindowVisible(host) || xuiGetFocusWidget(context) == webview) {
            fail("detach native host"); return XGE_ERROR;
        }
        phase = 32;
    } else if (phase == 32) {
        if (xuiWidgetAddChild(clip_parent, webview) != XUI_OK) {
            fail("reattach native host"); return XGE_ERROR;
        }
        phase = 33;
    } else if (phase == 33) {
        if (!IsWindowVisible(host) || !host_rect_is(350, 70, 200, 200) ||
                xuiWebViewFocus(webview) != XUI_OK ||
                xuiWidgetCreate(context, &alternate_root) != XUI_OK ||
                xuiWidgetSetRect(alternate_root, (xui_rect_t){0, 0, 640, 480}) != XUI_OK ||
                xuiSetRootWidget(context, alternate_root) != XUI_OK ||
                IsWindowVisible(host) || xuiGetFocusWidget(context) == webview) {
            fail("root replacement native host"); return XGE_ERROR;
        }
        phase = 34;
    } else if (phase == 34) {
        if (xuiSetRootWidget(context, root) != XUI_OK) {
            fail("restore root"); return XGE_ERROR;
        }
        xuiWidgetDestroy(alternate_root);
        alternate_root = NULL;
        phase = 35;
    } else if (phase == 35) {
        if (!IsWindowVisible(host) || !host_rect_is(350, 70, 200, 200) ||
                xuiWebViewFocus(webview) != XUI_OK) {
            fail("restore browser after clipping"); return XGE_ERROR;
        }
        if (xuiWebViewClose(webview) != XUI_OK || closed != 1 || IsWindow(host) ||
            xuiWebViewGetState(webview) != XUI_WEBVIEW_CLOSED ||
            xuiGetFocusWidget(context) == webview) {
            fail("close"); return XGE_ERROR;
        }
        xuiWidgetDestroy(webview);
        webview = NULL;
        phase = 4;
    } else if (phase == 4) {
        xui_webview_desc_t desc = {0};
        desc.iSize = sizeof(desc);
        if (xuiWebViewCreate(context, &webview, &desc) != XUI_OK ||
            xuiWidgetSetRect(webview, (xui_rect_t){0, 0, 100, 100}) != XUI_OK ||
            xuiWidgetAddChild(root, webview) != XUI_OK ||
            xuiWebViewInitializeAsync(webview) != XUI_OK) {
            fail("create before cancellation"); return XGE_ERROR;
        }
        xuiWidgetDestroy(webview);
        webview = NULL;
        phase = 5;
        frames = 0;
    } else if (phase == 5 && frames > 90) {
        xui_webview_desc_t desc = {0};
        desc.iSize = sizeof(desc);
        desc.onEvent = on_reentrant_webview;
        if (xuiWebViewCreate(context, &webview, &desc) != XUI_OK ||
            xuiWidgetSetRect(webview, (xui_rect_t){40, 40, 120, 120}) != XUI_OK ||
            xuiWidgetAddChild(root, webview) != XUI_OK ||
            xuiWebViewInitializeAsync(webview) != XUI_OK) {
            fail("reentrant setup"); return XGE_ERROR;
        }
        phase = 6;
        frames = 0;
    } else if (phase == 6 && reentrant_done && frames > 30) {
        phase = 7;
        frames = 0;
    } else if (phase == 7 && lifecycle_cycle < 12) {
        xui_webview_desc_t desc = {0};
        if (FindWindowExW(window, NULL, L"Static", NULL)) {
            fail("orphan host before repeated lifecycle"); return XGE_ERROR;
        }
        desc.iSize = sizeof(desc);
        desc.onEvent = on_webview;
        lifecycle_navigation = navigated;
        lifecycle_closed = closed;
        if (xuiWebViewCreate(context, &webview, &desc) != XUI_OK ||
            xuiWidgetSetRect(webview, (xui_rect_t){40, 40, 120, 120}) != XUI_OK ||
            xuiWidgetAddChild(root, webview) != XUI_OK ||
            xuiWebViewInitializeAsync(webview) != XUI_OK) {
            fail("repeated lifecycle create"); return XGE_ERROR;
        }
        phase = 8;
        frames = 0;
    } else if (phase == 8 && navigated > lifecycle_navigation) {
        host = FindWindowExW(window, NULL, L"Static", NULL);
        if (navigated != lifecycle_navigation + 1 ||
            xuiWebViewGetState(webview) != XUI_WEBVIEW_READY ||
            !host || !IsWindow(host) ||
            xuiWebViewClose(webview) != XUI_OK ||
            closed != lifecycle_closed + 1 || IsWindow(host) ||
            xuiWebViewGetState(webview) != XUI_WEBVIEW_CLOSED) {
            fail("repeated lifecycle close"); return XGE_ERROR;
        }
        xuiWidgetDestroy(webview);
        webview = NULL;
        host = NULL;
        if (FindWindowExW(window, NULL, L"Static", NULL)) {
            fail("orphan host after repeated lifecycle"); return XGE_ERROR;
        }
        lifecycle_cycle++;
        phase = 7;
        frames = 0;
    } else if (phase == 7 && lifecycle_cycle == 12) {
        phase = 70;
        frames = 0;
    } else if (phase == 70) {
        xui_webview_desc_t desc = {0};
        if (FindWindowExW(window, NULL, L"Static", NULL)) {
            fail("orphan host before concurrent views"); return XGE_ERROR;
        }
        desc.iSize = sizeof(desc);
        desc.onEvent = on_concurrent_webview;
        if (xuiWebViewCreate(context, &webview, &desc) != XUI_OK ||
            xuiWidgetSetRect(webview, (xui_rect_t){30, 100, 220, 180}) != XUI_OK ||
            xuiWidgetAddChild(root, webview) != XUI_OK ||
            xuiWebViewInitializeAsync(webview) != XUI_OK ||
            xuiWebViewCreate(context, &webview_second, &desc) != XUI_OK ||
            xuiWidgetSetRect(webview_second, (xui_rect_t){330, 100, 220, 180}) != XUI_OK ||
            xuiWidgetAddChild(root, webview_second) != XUI_OK ||
            xuiWebViewInitializeAsync(webview_second) != XUI_OK) {
            fail("concurrent create"); return XGE_ERROR;
        }
        phase = 71;
        frames = 0;
    } else if (phase == 71 && concurrent_first_navigation &&
        concurrent_second_navigation) {
        static const char alive_html[] =
            "<!doctype html><title>still alive</title>";
        HWND first_host = FindWindowExW(window, NULL, L"Static", NULL);
        HWND second_host = first_host ?
            FindWindowExW(window, first_host, L"Static", NULL) : NULL;
        if (concurrent_first_navigation != 1 ||
            concurrent_second_navigation != 1 ||
            xuiWebViewGetState(webview) != XUI_WEBVIEW_READY ||
            xuiWebViewGetState(webview_second) != XUI_WEBVIEW_READY ||
            !first_host || !second_host ||
            FindWindowExW(window, second_host, L"Static", NULL) ||
            xuiWebViewClose(webview) != XUI_OK ||
            concurrent_first_closed != 1) {
            fail("concurrent ready and first close"); return XGE_ERROR;
        }
        xuiWidgetDestroy(webview);
        webview = NULL;
        host = FindWindowExW(window, NULL, L"Static", NULL);
        if (!host || !host_rect_is(330, 100, 220, 180) ||
            FindWindowExW(window, host, L"Static", NULL) ||
            xuiWebViewGetState(webview_second) != XUI_WEBVIEW_READY ||
            xuiWebViewLoadHtml(webview_second, alive_html,
                sizeof(alive_html) - 1) != XUI_OK) {
            fail("second view survives first close"); return XGE_ERROR;
        }
        phase = 72;
        frames = 0;
    } else if (phase == 72 && concurrent_second_navigation == 2) {
        if (xuiWebViewClose(webview_second) != XUI_OK ||
            concurrent_second_closed != 1 || IsWindow(host)) {
            fail("concurrent second close"); return XGE_ERROR;
        }
        xuiWidgetDestroy(webview_second);
        webview_second = NULL;
        host = NULL;
        if (FindWindowExW(window, NULL, L"Static", NULL)) {
            fail("orphan host after concurrent views"); return XGE_ERROR;
        }
        phase = 73;
        frames = 0;
    } else if (phase == 73) {
        xui_webview_desc_t desc = {0};
        char temp[MAX_PATH];
        DWORD bytes = GetTempPathA(MAX_PATH, temp);
        int written;
        if (!bytes || bytes >= MAX_PATH) {
            fail("isolated profile temp path"); return XGE_ERROR;
        }
        written = snprintf(crash_folder, sizeof(crash_folder),
            "%sxui-webview-process-failure-%lu", temp,
            (unsigned long)GetCurrentProcessId());
        if (written <= 0 || (size_t)written >= sizeof(crash_folder)) {
            fail("isolated profile path"); return XGE_ERROR;
        }
        desc.iSize = sizeof(desc);
        desc.sUserDataFolder = crash_folder;
        desc.onEvent = on_crash_webview;
        if (xuiWebViewCreate(context, &webview, &desc) != XUI_OK ||
            xuiWidgetSetRect(webview, (xui_rect_t){60, 120, 240, 160}) != XUI_OK ||
            xuiWidgetAddChild(root, webview) != XUI_OK ||
            xuiWebViewInitializeAsync(webview) != XUI_OK) {
            fail("isolated failure create"); return XGE_ERROR;
        }
        phase = 74;
        frames = 0;
    } else if (phase == 74 && crash_navigated) {
        uint32_t pid = 0;
        HANDLE process = NULL;
        char path[MAX_PATH];
        DWORD size = sizeof(path);
        const char* basename;
        if (crash_navigated != 1 ||
            xuiWebViewTestBrowserProcessId(webview, &pid) != XUI_OK ||
            pid == 0 || pid == GetCurrentProcessId()) {
            fail("isolated browser PID"); return XGE_ERROR;
        }
        process = OpenProcess(PROCESS_TERMINATE |
            PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if (!process || !QueryFullProcessImageNameA(process, 0, path, &size)) {
            if (process) CloseHandle(process);
            fail("isolated browser process identity"); return XGE_ERROR;
        }
        basename = strrchr(path, '\\');
        basename = basename ? basename + 1 : path;
        if (lstrcmpiA(basename, "msedgewebview2.exe") != 0 ||
            !TerminateProcess(process, 0xc0000005u)) {
            CloseHandle(process);
            fail("isolated browser termination"); return XGE_ERROR;
        }
        CloseHandle(process);
        phase = 75;
        frames = 0;
    } else if (phase == 75 && crash_failed) {
        if (crash_failed != 1 || crash_error != E_FAIL ||
            xuiWebViewGetState(webview) != XUI_WEBVIEW_FAILED ||
            xuiWebViewGetNativeError(webview) != E_FAIL ||
            crash_closed || FindWindowExW(window, NULL, L"Static", NULL)) {
            fail("browser failure publication"); return XGE_ERROR;
        }
        xuiWidgetDestroy(webview);
        webview = NULL;
        phase = 76;
        frames = 0;
    } else if (phase == 76) {
        xui_webview_desc_t desc = {0};
        desc.iSize = sizeof(desc);
        desc.sUserDataFolder = crash_folder;
        desc.onEvent = on_recovery_webview;
        if (xuiWebViewCreate(context, &webview, &desc) != XUI_OK ||
            xuiWidgetSetRect(webview, (xui_rect_t){60, 120, 240, 160}) != XUI_OK ||
            xuiWidgetAddChild(root, webview) != XUI_OK ||
            xuiWebViewInitializeAsync(webview) != XUI_OK) {
            fail("browser failure recovery create"); return XGE_ERROR;
        }
        phase = 77;
        frames = 0;
    } else if (phase == 77 && recovery_navigated) {
        host = FindWindowExW(window, NULL, L"Static", NULL);
        if (recovery_navigated != 1 ||
            xuiWebViewGetState(webview) != XUI_WEBVIEW_READY ||
            !host || !host_rect_is(60, 120, 240, 160) ||
            xuiWebViewClose(webview) != XUI_OK ||
            recovery_closed != 1 || IsWindow(host)) {
            fail("browser failure recovery navigation"); return XGE_ERROR;
        }
        xuiWidgetDestroy(webview);
        webview = NULL;
        puts("XUI WebView2 widget: focus/layout, 12 repeated lifecycles, two independent views and isolated browser-process failure/recreate passed");
        result = 0;
        xgeQuit();
    }
render:
    if (xgeBegin() != XGE_OK) { fail("XGE begin"); return XGE_ERROR; }
    xgeClear(0xffeef1f5);
    if (xgeEnd() != XGE_OK) { fail("XGE end"); return XGE_ERROR; }
    return XGE_OK;
}

int main(void)
{
    xge_desc_t desc = {0};
    desc.iWidth = 640;
    desc.iHeight = 480;
    desc.sTitle = "XUI WebView widget integration";
    desc.iRunMode = XGE_RUN_GAME_LOOP;
    desc.iTargetFPS = 60;
    if (xgeInit(&desc) != XGE_OK) return 2;
    xgeRun(frame, NULL);
    if (focus_request) xuiWebRequestRelease(focus_request);
    if (text_request) xuiWebRequestRelease(text_request);
    if (context) xuiDestroy(context);
    xgeUnit();
    return result;
}
