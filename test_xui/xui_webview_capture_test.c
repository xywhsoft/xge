/* Real WebView2 viewport PNG capture, cancellation and retained result. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "xge.h"
#include "xui.h"
#include "src/xui_webview_internal.h"

static xui_context context;
static xui_widget root, webview;
static xui_web_request first, zoomed, paint, hidden, clipped, navigation, closed;
static int phase, frames, phase_start, navigated, result = 1;

static void fail(const char* step)
{
    fprintf(stderr, "WebView capture test failed: %s, phase=%d, native=0x%08x\n",
        step, phase, webview ? (unsigned)xuiWebViewGetNativeError(webview) : 0);
    result = 2;
    xgeQuit();
}

static void on_webview(xui_widget widget, int event, int32_t error, void* user)
{
    static const char page[] = "<!doctype html><style>"
        "html,body{margin:0;width:100%;height:100%;background:#1159c9}"
        "#marker{position:absolute;left:0;top:0;width:30px;height:30px;"
        "background:#ef0000}"
        "</style><title>capture</title><div id='marker'></div>";
    (void)user;
    if (event == XUI_WEBVIEW_EVENT_READY) {
        xui_web_request early = (xui_web_request)(uintptr_t)1;
        if (xuiWebViewCapturePngAsync(widget, &early) != XUI_ERROR_INVALID_STATE ||
                early || xuiWebViewLoadHtml(widget, page, sizeof(page) - 1) != XUI_OK)
            fail("early capture/first page");
    } else if (event == XUI_WEBVIEW_EVENT_NAVIGATION_COMPLETE) {
        navigated++;
    } else if (event == XUI_WEBVIEW_EVENT_FAILED ||
            event == XUI_WEBVIEW_EVENT_NAVIGATION_FAILED) {
        fprintf(stderr, "WebView event error=0x%08x\n", (unsigned)error);
        fail("browser event");
    }
}

static int setup(void)
{
    xui_webview_desc_t desc = {0};
    desc.iSize = sizeof(desc);
    desc.onEvent = on_webview;
    return xuiCreate(&context) == XUI_OK &&
        xuiWidgetCreate(context, &root) == XUI_OK &&
        xuiSetRootWidget(context, root) == XUI_OK &&
        xuiWidgetSetRect(root, (xui_rect_t){0, 0, 320, 200}) == XUI_OK &&
        xuiWebViewCreate(context, &webview, &desc) == XUI_OK &&
        xuiWidgetSetRect(webview, (xui_rect_t){20, 20, 240, 120}) == XUI_OK &&
        xuiWidgetAddChild(root, webview) == XUI_OK &&
        xuiWebViewInitializeAsync(webview) == XUI_OK;
}

static int verify_png(xui_web_request request,
    unsigned char red, unsigned char green, unsigned char blue)
{
    const void* png = NULL;
    const char* json = (const char*)(uintptr_t)1;
    size_t bytes = 0, json_bytes = 99;
    xge_image_t image = {0};
    const unsigned char* pixel;
    int ok;
    if (xuiWebRequestGetResultJson(request, &json, &json_bytes) !=
            XUI_ERROR_INVALID_STATE || json || json_bytes ||
            xuiWebRequestGetResultPng(request, &png, &bytes) != XUI_OK ||
            !png || bytes < 24 || bytes > 64u * 1024u * 1024u ||
            xgeImageLoadMemory(&image, png, (int)bytes) != XGE_OK) return 0;
    pixel = (const unsigned char*)xgeImageGetPixels(&image) +
        ((size_t)image.iHeight / 2) * (size_t)image.iStride +
        ((size_t)image.iWidth / 2) * 4;
    ok = image.iWidth == 240 && image.iHeight == 120 &&
        pixel[0] == red && pixel[1] == green && pixel[2] == blue;
    if (!ok) fprintf(stderr, "PNG dimensions=%dx%d center=%u,%u,%u\n",
        image.iWidth, image.iHeight, pixel[0], pixel[1], pixel[2]);
    xgeImageFree(&image);
    return ok;
}

static int verify_zoom_marker(xui_web_request request, int expect_red)
{
    const void* png = NULL;
    size_t bytes = 0;
    xge_image_t image = {0};
    const unsigned char* pixel;
    int is_red, ok;
    if (xuiWebRequestGetResultPng(request, &png, &bytes) != XUI_OK ||
            !png || xgeImageLoadMemory(&image, png, (int)bytes) != XGE_OK)
        return 0;
    if (image.iWidth != 240 || image.iHeight != 120) {
        xgeImageFree(&image);
        return 0;
    }
    pixel = (const unsigned char*)xgeImageGetPixels(&image) +
        (size_t)10 * (size_t)image.iStride + (size_t)40 * 4u;
    is_red = pixel[0] > 180 && pixel[1] < 80 && pixel[2] < 80;
    ok = is_red == expect_red;
    if (!ok) fprintf(stderr, "zoom marker at 40,10=%u,%u,%u expected red=%d\n",
        pixel[0], pixel[1], pixel[2], expect_red);
    xgeImageFree(&image);
    return ok;
}

static int frame(void* user)
{
    (void)user;
    if (!context && !setup()) { fail("setup"); return XGE_ERROR; }
    if (++frames > 600) { fail("timeout"); return XGE_ERROR; }
    if (xuiDispatchPendingEvents(context) != XUI_OK ||
            xuiLayout(context) != XUI_OK ||
            xuiUpdate(context, .016f) != XUI_OK) {
        fail("layout/update"); return XGE_ERROR;
    }
    if (phase == 0 && navigated && frames > 20) {
        if (xuiWebViewSetZoomFactor(webview, 1.0) != XUI_OK ||
                xuiWebViewCapturePngAsync(webview, &first) != XUI_OK || !first) {
            fail("first capture"); return XGE_ERROR;
        }
        phase = 1;
    } else if (phase == 1 &&
            xuiWebRequestGetState(first) != XUI_WEB_REQUEST_PENDING) {
        if (xuiWebRequestGetState(first) != XUI_WEB_REQUEST_COMPLETED ||
                !verify_png(first, 0x11, 0x59, 0xc9) ||
                !verify_zoom_marker(first, 0) ||
                xuiWebViewSetZoomFactor(webview, 1.5) != XUI_OK) {
            fail("first pixels/page zoom setup"); return XGE_ERROR;
        }
        phase_start = frames;
        phase = 21;
    } else if (phase == 21 && frames > phase_start + 12) {
        if (xuiWebViewCapturePngAsync(webview, &zoomed) != XUI_OK) {
            fail("zoomed capture"); return XGE_ERROR;
        }
        phase = 22;
    } else if (phase == 22 &&
            xuiWebRequestGetState(zoomed) != XUI_WEB_REQUEST_PENDING) {
        if (xuiWebRequestGetState(zoomed) != XUI_WEB_REQUEST_COMPLETED ||
                !verify_png(zoomed, 0x11, 0x59, 0xc9) ||
                !verify_zoom_marker(zoomed, 1) ||
                xuiWebViewSetZoomFactor(webview, 1.0) != XUI_OK ||
                xuiWebViewCapturePngAsync(webview, &hidden) != XUI_OK ||
                xuiWidgetSetVisible(webview, 0) != XUI_OK) {
            fail("zoomed pixels/hidden cancellation setup"); return XGE_ERROR;
        }
        phase = 11;
    } else if (phase == 11) {
        static const char script[] = "document.body.style.background='#00aa33'";
        xui_web_request rejected = (xui_web_request)(uintptr_t)1;
        if (xuiWebRequestGetState(hidden) != XUI_WEB_REQUEST_CANCELLED ||
                xuiWebViewCapturePngAsync(webview, &rejected) !=
                    XUI_ERROR_INVALID_STATE || rejected ||
                xuiWebViewEvalScriptAsync(webview, script,
                    sizeof(script) - 1, &paint) != XUI_OK ||
                xuiWidgetSetRect(webview,
                    (xui_rect_t){319, 199, 240, 120}) != XUI_OK ||
                xuiWidgetSetVisible(webview, 1) != XUI_OK) {
            fail("hidden capture rejection/repaint"); return XGE_ERROR;
        }
        phase_start = frames;
        phase = 12;
    } else if (phase == 12 &&
            xuiWebRequestGetState(paint) != XUI_WEB_REQUEST_PENDING &&
            frames > phase_start + 12) {
        if (xuiWebRequestGetState(paint) != XUI_WEB_REQUEST_COMPLETED ||
                xuiWebViewCapturePngAsync(webview, &clipped) != XUI_OK) {
            fail("clipped viewport capture"); return XGE_ERROR;
        }
        phase = 13;
    } else if (phase == 13 &&
            xuiWebRequestGetState(clipped) != XUI_WEB_REQUEST_PENDING) {
        static const char second_page[] = "<!doctype html><title>second</title>"
            "<style>body{background:#ffffff}</style>";
        if (xuiWebRequestGetState(clipped) != XUI_WEB_REQUEST_COMPLETED ||
                !verify_png(clipped, 0x00, 0xaa, 0x33) ||
                xuiWidgetSetRect(webview,
                    (xui_rect_t){20, 20, 240, 120}) != XUI_OK ||
                xuiWebViewCapturePngAsync(webview, &navigation) != XUI_OK ||
                xuiWebViewLoadHtml(webview, second_page,
                    sizeof(second_page) - 1) != XUI_OK ||
                xuiWebRequestGetState(navigation) != XUI_WEB_REQUEST_CANCELLED) {
            fail("PNG pixels/navigation cancellation"); return XGE_ERROR;
        }
        phase = 2;
    } else if (phase == 2 && navigated >= 2) {
        if (xuiWebViewCapturePngAsync(webview, &closed) != XUI_OK ||
                xuiWebViewClose(webview) != XUI_OK ||
                xuiWebRequestGetState(closed) != XUI_WEB_REQUEST_CANCELLED ||
                !verify_png(first, 0x11, 0x59, 0xc9)) {
            fail("close cancellation/retained pixels"); return XGE_ERROR;
        }
        xuiWidgetDestroy(webview);
        webview = NULL;
        phase_start = frames;
        phase = 3;
    } else if (phase == 3 && frames > phase_start + 45) {
        if (xuiWebRequestGetState(navigation) != XUI_WEB_REQUEST_CANCELLED ||
                xuiWebRequestGetState(closed) != XUI_WEB_REQUEST_CANCELLED ||
                !verify_png(first, 0x11, 0x59, 0xc9) ||
                !verify_png(clipped, 0x00, 0xaa, 0x33)) {
            fail("late callback/result lifetime"); return XGE_ERROR;
        }
        puts("XUI WebView2 PNG viewport, visual page zoom, pixels, cancellation and lifetime passed");
        result = 0;
        xgeQuit();
    }
    if (xgeBegin() != XGE_OK) { fail("begin"); return XGE_ERROR; }
    xgeClear(0xffffffff);
    if (xgeEnd() != XGE_OK) { fail("end"); return XGE_ERROR; }
    return XGE_OK;
}

int main(void)
{
    xge_desc_t desc = {0};
    desc.iWidth = 320;
    desc.iHeight = 200;
    desc.sTitle = "XUI WebView PNG capture";
    desc.iRunMode = XGE_RUN_GAME_LOOP;
    desc.iTargetFPS = 60;
    if (xgeInit(&desc) != XGE_OK) return 2;
    xgeRun(frame, NULL);
    if (first) xuiWebRequestRelease(first);
    if (zoomed) xuiWebRequestRelease(zoomed);
    if (paint) xuiWebRequestRelease(paint);
    if (hidden) xuiWebRequestRelease(hidden);
    if (clipped) xuiWebRequestRelease(clipped);
    if (navigation) xuiWebRequestRelease(navigation);
    if (closed) xuiWebRequestRelease(closed);
    if (context) xuiDestroy(context);
    xgeUnit();
    return result;
}
