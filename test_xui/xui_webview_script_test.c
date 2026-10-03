/* Real WebView2 async script-request and lifetime integration. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "xge.h"
#include "xui.h"
#include "src/xui_webview_internal.h"

static xui_context context;
static xui_widget root, webview;
static xui_web_request first, null_value, capacity_probe, cancelled, navigation, second;
static int phase, frames, navigated, result = 1;

static void fail(const char* step)
{
    fprintf(stderr, "WebView script test failed: %s, phase=%d\n", step, phase);
    result = 2;
    xgeQuit();
}

static void on_webview(xui_widget widget, int event, int32_t error, void* user)
{
    static const char first_page[] = "<!doctype html><title>first</title>"
        "<script>window.marker='first';</script><p>First page</p>";
    (void)user;
    if (event == XUI_WEBVIEW_EVENT_READY) {
        if (xuiWebViewLoadHtml(widget, first_page, sizeof(first_page) - 1) != XUI_OK)
            fail("first page");
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
    if (xuiCreate(&context) != XUI_OK ||
            xuiWidgetCreate(context, &root) != XUI_OK ||
            xuiSetRootWidget(context, root) != XUI_OK ||
            xuiWidgetSetRect(root, (xui_rect_t){0, 0, 480, 320}) != XUI_OK ||
            xuiWebViewCreate(context, &webview, &desc) != XUI_OK ||
            xuiWidgetSetRect(webview, (xui_rect_t){20, 20, 400, 240}) != XUI_OK ||
            xuiWidgetAddChild(root, webview) != XUI_OK ||
            xuiWebViewInitializeAsync(webview) != XUI_OK) return 0;
    return 1;
}

static int result_contains(xui_web_request request, const char* needle)
{
    const char* json = NULL;
    size_t bytes = 0;
    return xuiWebRequestGetResultJson(request, &json, &bytes) == XUI_OK &&
        json && bytes == strlen(json) && strstr(json, needle) != NULL;
}

static int bounded_queue(void)
{
    xui_web_request pending[XUI_WEBVIEW_MAX_PENDING_SCRIPTS] = {0};
    xui_web_request overflow = (xui_web_request)(uintptr_t)1;
    int ok = 1;
    size_t i;
    for (i = 0; i < XUI_WEBVIEW_MAX_PENDING_SCRIPTS; i++) {
        if (xuiWebViewEvalScriptAsync(webview, "1", 1, &pending[i]) != XUI_OK) {
            ok = 0;
            break;
        }
    }
    if (ok && (xuiWebViewEvalScriptAsync(webview, "2", 1, &overflow) !=
            XUI_ERROR_LIMIT_EXCEEDED || overflow)) ok = 0;
    for (i = 0; i < XUI_WEBVIEW_MAX_PENDING_SCRIPTS; i++) {
        if (pending[i]) {
            if (xuiWebRequestCancel(pending[i]) != XUI_OK) ok = 0;
            xuiWebRequestRelease(pending[i]);
        }
    }
    overflow = (xui_web_request)(uintptr_t)1;
    if (ok && (xuiWebViewEvalScriptAsync(webview, "3", 1, &overflow) !=
            XUI_ERROR_LIMIT_EXCEEDED || overflow)) ok = 0;
    return ok;
}

static int frame(void* user)
{
    (void)user;
    if (!context && !setup()) { fail("setup"); return XGE_ERROR; }
    if (++frames > 600) { fail("timeout"); return XGE_ERROR; }
    if (xuiDispatchPendingEvents(context) != XUI_OK ||
            xuiLayout(context) != XUI_OK || xuiUpdate(context, .016f) != XUI_OK) {
        fail("layout/update"); return XGE_ERROR;
    }
    if (phase == 0 && navigated) {
        static const char script[] = "({answer:42,text:'汉字😀',page:window.marker})";
        static const char invalid_utf8[] = {(char)0xc3, (char)0x28};
        static const char embedded_nul[] = {'1', 0, '2'};
        xui_web_request invalid = (xui_web_request)(uintptr_t)1;
        if (xuiWebViewEvalScriptAsync(webview, invalid_utf8, sizeof(invalid_utf8),
                    &invalid) != XUI_ERROR_INVALID_ARGUMENT || invalid ||
                xuiWebViewEvalScriptAsync(webview, embedded_nul,
                    sizeof(embedded_nul), &invalid) != XUI_ERROR_INVALID_ARGUMENT ||
                invalid ||
                xuiWebViewEvalScriptAsync(webview, script, sizeof(script) - 1,
                    &first) != XUI_OK || !first ||
                xuiWebViewEvalScriptAsync(webview, "undefined", 9,
                    &null_value) != XUI_OK) {
            fail("script request/UTF-8 validation"); return XGE_ERROR;
        }
        phase = 1;
    } else if (phase == 1 &&
            xuiWebRequestGetState(first) != XUI_WEB_REQUEST_PENDING &&
            xuiWebRequestGetState(null_value) != XUI_WEB_REQUEST_PENDING) {
        if (xuiWebRequestGetState(first) != XUI_WEB_REQUEST_COMPLETED ||
                !result_contains(first, "42") || !result_contains(first, "first") ||
                !result_contains(first, "汉字") ||
                xuiWebRequestGetState(null_value) != XUI_WEB_REQUEST_COMPLETED ||
                !result_contains(null_value, "null") || !bounded_queue()) {
            fail("script result/inflight bound"); return XGE_ERROR;
        }
        phase = 11;
    } else if (phase == 11) {
        int error = xuiWebViewEvalScriptAsync(webview, "123", 3, &capacity_probe);
        if (error == XUI_OK) phase = 12;
        else if (error != XUI_ERROR_LIMIT_EXCEEDED || capacity_probe) {
            fail("inflight capacity recovery"); return XGE_ERROR;
        }
    } else if (phase == 12 &&
            xuiWebRequestGetState(capacity_probe) != XUI_WEB_REQUEST_PENDING) {
        if (xuiWebRequestGetState(capacity_probe) != XUI_WEB_REQUEST_COMPLETED ||
                !result_contains(capacity_probe, "123")) {
            fail("capacity recovery result"); return XGE_ERROR;
        }
        xuiWebRequestRelease(capacity_probe);
        capacity_probe = NULL;
        phase = 13;
    } else if (phase == 13) {
        static const char second_page[] = "<!doctype html><title>second</title>"
            "<script>window.marker='second';</script><p>Second page</p>";
        const char* json = NULL;
        size_t bytes = 123;
        xui_web_request abandoned = NULL;
        if (xuiWebViewEvalScriptAsync(webview, "123", 3, &cancelled) != XUI_OK ||
                xuiWebRequestCancel(cancelled) != XUI_OK ||
                xuiWebRequestGetState(cancelled) != XUI_WEB_REQUEST_CANCELLED ||
                xuiWebRequestGetResultJson(cancelled, &json, &bytes) !=
                    XUI_ERROR_INVALID_STATE || json || bytes ||
                xuiWebViewEvalScriptAsync(webview, "456", 3, &abandoned) != XUI_OK) {
            fail("script result/explicit cancellation"); return XGE_ERROR;
        }
        xuiWebRequestRelease(abandoned);
        if (xuiWebViewEvalScriptAsync(webview, "789", 3, &navigation) != XUI_OK ||
                xuiWebViewLoadHtml(webview, second_page, sizeof(second_page) - 1) !=
                    XUI_OK ||
                xuiWebRequestGetState(navigation) != XUI_WEB_REQUEST_CANCELLED) {
            fail("navigation cancellation"); return XGE_ERROR;
        }
        phase = 2;
    } else if (phase == 2 && navigated >= 2) {
        if (xuiWebViewEvalScriptAsync(webview, "window.marker", 13, &second) != XUI_OK)
        {
            fail("second page script"); return XGE_ERROR;
        }
        phase = 3;
    } else if (phase == 3 &&
            xuiWebRequestGetState(second) != XUI_WEB_REQUEST_PENDING) {
        xui_web_request rejected = (xui_web_request)(uintptr_t)1;
        if (xuiWebRequestGetState(second) != XUI_WEB_REQUEST_COMPLETED ||
                !result_contains(second, "second") ||
                xuiWebViewEvalScriptAsync(webview, "0", 1, &rejected) != XUI_OK ||
                !rejected) {
            fail("second page result"); return XGE_ERROR;
        }
        if (xuiWebViewClose(webview) != XUI_OK ||
                xuiWebRequestGetState(rejected) != XUI_WEB_REQUEST_CANCELLED ||
                !result_contains(first, "first") || !result_contains(second, "second")) {
            fail("close cancellation/retained results"); return XGE_ERROR;
        }
        xuiWebRequestRelease(rejected);
        rejected = (xui_web_request)(uintptr_t)1;
        if (xuiWebViewEvalScriptAsync(webview, "1", 1, &rejected) !=
                    XUI_ERROR_INVALID_STATE || rejected) {
            fail("closed request rejection"); return XGE_ERROR;
        }
        xuiWidgetDestroy(webview);
        webview = NULL;
        phase = 4;
        frames = 0;
    } else if (phase == 4 && frames > 60) {
        if (xuiWebRequestGetState(cancelled) != XUI_WEB_REQUEST_CANCELLED ||
                xuiWebRequestGetState(navigation) != XUI_WEB_REQUEST_CANCELLED ||
                !result_contains(first, "first") || !result_contains(second, "second")) {
            fail("late callback/request lifetime"); return XGE_ERROR;
        }
        puts("XUI WebView2 async script JSON, Unicode, cancellation and lifetime passed");
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
    desc.iWidth = 480;
    desc.iHeight = 320;
    desc.sTitle = "XUI WebView script integration";
    desc.iRunMode = XGE_RUN_GAME_LOOP;
    desc.iTargetFPS = 60;
    if (xgeInit(&desc) != XGE_OK) return 2;
    xgeRun(frame, NULL);
    if (first) xuiWebRequestRelease(first);
    if (null_value) xuiWebRequestRelease(null_value);
    if (capacity_probe) xuiWebRequestRelease(capacity_probe);
    if (cancelled) xuiWebRequestRelease(cancelled);
    if (navigation) xuiWebRequestRelease(navigation);
    if (second) xuiWebRequestRelease(second);
    if (context) xuiDestroy(context);
    xgeUnit();
    return result;
}
