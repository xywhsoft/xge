/* First-release browser hosting test: public APIs only, no script/message bridge. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "xge.h"
#include "xui.h"

static xui_context context;
static xui_widget root, view;
static HWND window, host;
static int result = 1, phase, navigations, closed, frames;

static void fail(const char* step)
{
    fprintf(stderr, "Basic WebView failed: %s (state=%d, HRESULT=%08x)\n", step,
        view ? xuiWebViewGetState(view) : -1, view ? (unsigned)xuiWebViewGetNativeError(view) : 0);
    result = 1; xgeQuit();
}
#define CHECK(c) do { if (!(c)) { fail(#c); return XGE_ERROR; } } while (0)

static int bounds_are(int x, int y, int width, int height)
{
    RECT rect;
    if (!host || !GetWindowRect(host, &rect)) return 0;
    MapWindowPoints(NULL, window, (POINT*)&rect, 2);
    return rect.left == x && rect.top == y && rect.right - rect.left == width && rect.bottom - rect.top == height;
}
static void event(xui_widget widget, int kind, int32_t error, void* user)
{
    static const char html[] = "<!doctype html><meta charset=utf-8><title>XUI basic browser</title><h1>Local HTML</h1><input autofocus value='editable'>";
    (void)user;
    if (kind == XUI_WEBVIEW_EVENT_READY) {
        if (xuiWebViewLoadHtml(widget, html, sizeof(html) - 1) != XUI_OK) fail("load HTML");
    } else if (kind == XUI_WEBVIEW_EVENT_NAVIGATION_COMPLETE) navigations++;
    else if (kind == XUI_WEBVIEW_EVENT_CLOSED) closed++;
    else if (kind == XUI_WEBVIEW_EVENT_FAILED || kind == XUI_WEBVIEW_EVENT_NAVIGATION_FAILED) {
        fprintf(stderr, "Browser event %d: %08x\n", kind, (unsigned)error); fail("async event");
    }
}
static int frame(void* user)
{
    (void)user;
    if (!context) {
        xui_proxy_t proxy = xuiProxyXge(); xui_webview_desc_t desc = {0}; char folder[MAX_PATH];
        DWORD length = GetTempPathA(MAX_PATH, folder);
        window = (HWND)xgePlatformNativeHandle();
        CHECK(window && length && length < MAX_PATH - 48);
        snprintf(folder + length, MAX_PATH - length, "xui-webview-basic-%lu", (unsigned long)GetCurrentProcessId());
        CHECK(xuiCreate(&context) == XUI_OK);
        CHECK(xuiSetProxy(context, &proxy) == XUI_OK);
        CHECK(xuiInputViewport(context, 640, 480) == XUI_OK);
        CHECK(xuiWidgetCreate(context, &root) == XUI_OK);
        CHECK(xuiSetRootWidget(context, root) == XUI_OK);
        CHECK(xuiWidgetSetRect(root, (xui_rect_t){0,0,640,480}) == XUI_OK);
        desc.iSize = sizeof(desc); desc.sUserDataFolder = folder; desc.onEvent = event;
        CHECK(xuiWebViewCreate(context, &view, &desc) == XUI_OK);
        CHECK(xuiWidgetSetRect(view, (xui_rect_t){20,30,300,200}) == XUI_OK);
        CHECK(xuiWidgetAddChild(root, view) == XUI_OK);
        CHECK(xuiWebViewInitializeAsync(view) == XUI_OK);
    }
    CHECK(++frames < 1800);
    CHECK(xuiDispatchPendingEvents(context) == XUI_OK);
    CHECK(xuiLayout(context) == XUI_OK && xuiUpdate(context, .016f) == XUI_OK);
    if (!phase && navigations) {
        double zoom = 0;
        host = FindWindowExW(window, NULL, L"Static", NULL);
        CHECK(navigations == 1 && xuiWebViewGetState(view) == XUI_WEBVIEW_READY);
        CHECK(bounds_are(20,30,300,200) && IsWindowVisible(host));
        CHECK(xuiWebViewSetZoomFactor(view, 1.5) == XUI_OK);
        CHECK(xuiWebViewGetZoomFactor(view, &zoom) == XUI_OK && zoom == 1.5);
        CHECK(xuiWebViewFocus(view) == XUI_OK);
        CHECK(xuiGetFocusWidget(context) == view);
        CHECK(xuiWidgetSetVisible(view, 0) == XUI_OK);
        phase = 1;
    } else if (phase == 1) {
        CHECK(!IsWindowVisible(host));
        CHECK(xuiWidgetSetRect(view, (xui_rect_t){40,50,280,160}) == XUI_OK);
        CHECK(xuiWidgetSetVisible(view, 1) == XUI_OK);
        phase = 2;
    } else if (phase == 2) {
        CHECK(IsWindowVisible(host) && bounds_are(40,50,280,160));
        CHECK(xuiWebViewNavigate(view, "about:blank") == XUI_OK);
        phase = 3;
    } else if (phase == 3 && navigations == 2) {
        CHECK(xuiWebViewClose(view) == XUI_OK);
        CHECK(closed == 1 && xuiWebViewGetState(view) == XUI_WEBVIEW_CLOSED && !IsWindow(host));
        CHECK(xuiWebViewClose(view) == XUI_OK && closed == 1);
        result = 0; xgeQuit();
    }
    CHECK(xgeBegin() == XGE_OK);
    xgeClear(0xffeef1f5);
    CHECK(xgeEnd() == XGE_OK);
    return XGE_OK;
}
int main(void)
{
    xge_desc_t desc = {0}; int run;
    desc.iWidth = 640; desc.iHeight = 480; desc.sTitle = "XUI basic WebView";
    desc.iRunMode = XGE_RUN_GAME_LOOP; desc.iTargetFPS = 60;
    if (xgeInit(&desc) != XGE_OK) return 2;
    run = xgeRun(frame, NULL);
    if (run != XGE_OK) result = 1;
    if (context) xuiDestroy(context);
    xgeUnit();
    if (!result) puts("Basic WebView: public HTML/navigation, focus, layout, visibility, zoom and close passed.");
    return result;
}
