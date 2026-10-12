/* Real XGE window scroll-viewport integration for the optional WebView2 widget. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "xge.h"
#include "xui.h"

static xui_context context;
static xui_widget root, scroll_view, viewport, webview;
static HWND window, host;
static int phase, frames, ready, result = 1;

static void fail(const char* step)
{
    fprintf(stderr, "WebView scroll test failed: %s\n", step);
    result = 2;
    xgeQuit();
}

static int browser_has_native_focus(void)
{
    HWND focused = GetFocus();
    return host && focused && (focused == host || IsChild(host, focused));
}

static int host_matches_visible_intersection(void)
{
    xui_rect_t content = xuiWidgetGetWorldRect(webview);
    xui_rect_t clip = xuiWidgetGetWorldRect(viewport);
    RECT client, actual;
    int left, top, right, bottom;
    if (!host || !IsWindow(host) || !GetClientRect(window, &client)) return 0;
    left = content.fX > clip.fX ? content.fX : clip.fX;
    top = content.fY > clip.fY ? content.fY : clip.fY;
    right = content.fX + content.fW < clip.fX + clip.fW ?
        content.fX + content.fW : clip.fX + clip.fW;
    bottom = content.fY + content.fH < clip.fY + clip.fH ?
        content.fY + content.fH : clip.fY + clip.fH;
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right > client.right) right = client.right;
    if (bottom > client.bottom) bottom = client.bottom;
    if (right <= left || bottom <= top) {
        if (IsWindowVisible(host))
            fprintf(stderr, "scroll expected hidden: web=(%d,%d,%d,%d) clip=(%d,%d,%d,%d)\n",
                content.fX, content.fY, content.fW, content.fH,
                clip.fX, clip.fY, clip.fW, clip.fH);
        return !IsWindowVisible(host);
    }
    if (!IsWindowVisible(host) || !GetWindowRect(host, &actual)) return 0;
    MapWindowPoints(NULL, window, (POINT*)&actual, 2);
    if (actual.left != left || actual.top != top ||
            actual.right != right || actual.bottom != bottom) {
        fprintf(stderr, "scroll expected=(%d,%d,%d,%d) actual=(%ld,%ld,%ld,%ld) web=(%d,%d,%d,%d) clip=(%d,%d,%d,%d)\n",
            left, top, right, bottom, actual.left, actual.top, actual.right, actual.bottom,
            content.fX, content.fY, content.fW, content.fH,
            clip.fX, clip.fY, clip.fW, clip.fH);
        return 0;
    }
    return 1;
}

static void on_webview(xui_widget widget, int event, int32_t error, void* user)
{
    (void)widget; (void)error; (void)user;
    if (event == XUI_WEBVIEW_EVENT_READY) ready = 1;
    if (event == XUI_WEBVIEW_EVENT_FAILED) fail("initialization");
}

static int setup(void)
{
    xui_scroll_view_desc_t scroll = {0};
    xui_webview_desc_t browser = {0};
    scroll.iSize = sizeof(scroll);
    scroll.fViewportWidth = 200;
    scroll.fViewportHeight = 180;
    scroll.fContentWidth = 700;
    scroll.fContentHeight = 700;
    browser.iSize = sizeof(browser);
    browser.onEvent = on_webview;
    if (xuiCreate(&context) != XUI_OK ||
            xuiSetViewportSize(context, 640, 480) != XUI_OK ||
            xuiWidgetCreate(context, &root) != XUI_OK ||
            xuiSetRootWidget(context, root) != XUI_OK ||
            xuiWidgetSetLayoutType(root, XUI_LAYOUT_MANUAL) != XUI_OK ||
            xuiWidgetSetRect(root, (xui_rect_t){0, 0, 640, 480}) != XUI_OK ||
            xuiScrollViewCreate(context, &scroll_view, &scroll) != XUI_OK ||
            xuiWidgetSetRect(scroll_view, (xui_rect_t){100, 100, 200, 180}) != XUI_OK ||
            xuiWidgetAddChild(root, scroll_view) != XUI_OK ||
            xuiWebViewCreate(context, &webview, &browser) != XUI_OK ||
            xuiWidgetSetRect(webview, (xui_rect_t){50, 50, 320, 200}) != XUI_OK ||
            xuiWidgetAddChild(xuiScrollViewGetContentWidget(scroll_view), webview) != XUI_OK ||
            xuiWebViewInitializeAsync(webview) != XUI_OK) return 0;
    viewport = xuiScrollViewGetViewportWidget(scroll_view);
    return viewport != NULL;
}

static int frame(void* user)
{
    (void)user;
    if (!window) {
        window = (HWND)xgePlatformNativeHandle();
        if (!window || !setup()) { fail("setup"); return XGE_ERROR; }
    }
    if (++frames > 600) { fail("timeout"); return XGE_ERROR; }
    if (xuiDispatchPendingEvents(context) != XUI_OK ||
            xuiLayout(context) != XUI_OK || xuiUpdate(context, .016f) != XUI_OK) {
        fail("layout/update"); return XGE_ERROR;
    }
    if (phase == 0 && ready) {
        host = FindWindowExW(window, NULL, L"Static", NULL);
        if (!host_matches_visible_intersection() || !IsWindowVisible(host)) {
            fail("initial viewport"); return XGE_ERROR;
        }
        if (xuiWebViewFocus(webview) != XUI_OK) {
            fail("initial focus"); return XGE_ERROR;
        }
        if (xuiScrollViewSetOffset(scroll_view, 100, 100) != XUI_OK) {
            fail("set first scroll offset"); return XGE_ERROR;
        }
        phase = 1;
    } else if (phase == 1) {
        if (!host_matches_visible_intersection() || !IsWindowVisible(host) ||
                !browser_has_native_focus() ||
                xuiScrollViewSetOffset(scroll_view, 400, 400) != XUI_OK) {
            fail("partial scroll"); return XGE_ERROR;
        }
        phase = 2;
    } else if (phase == 2) {
        if (!host_matches_visible_intersection() || IsWindowVisible(host) ||
                browser_has_native_focus() ||
                xuiGetFocusWidget(context) == webview ||
                xuiScrollViewSetOffset(scroll_view, 0, 0) != XUI_OK) {
            fail("fully scrolled out"); return XGE_ERROR;
        }
        phase = 3;
    } else if (phase == 3) {
        if (!host_matches_visible_intersection() || !IsWindowVisible(host) ||
                xuiWebViewFocus(webview) != XUI_OK ||
                !browser_has_native_focus() ||
                xuiWebViewClose(webview) != XUI_OK) {
            fail("scroll restore/close"); return XGE_ERROR;
        }
        puts("XUI WebView2 scroll viewport and focus passed");
        result = 0;
        xgeQuit();
    }
    if (xgeBegin() != XGE_OK) { fail("begin"); return XGE_ERROR; }
    xgeClear(0xffeef1f5);
    if (xgeEnd() != XGE_OK) { fail("end"); return XGE_ERROR; }
    return XGE_OK;
}

int main(void)
{
    xge_desc_t desc = {0};
    desc.iWidth = 640;
    desc.iHeight = 480;
    desc.sTitle = "XUI WebView scroll integration";
    desc.iRunMode = XGE_RUN_GAME_LOOP;
    desc.iTargetFPS = 60;
    if (xgeInit(&desc) != XGE_OK) return 2;
    xgeRun(frame, NULL);
    if (context) xuiDestroy(context);
    xgeUnit();
    return result;
}
