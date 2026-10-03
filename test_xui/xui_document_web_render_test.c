/* Offline KaTeX/Mermaid/sandboxed HTML rendering through XUI WebView. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xge.h"
#include "xui.h"
#include "src/xui_webview_internal.h"

static xui_context context;
static xui_widget root, webview;
static xui_web_request capture;
static xui_web_request html_probe;
static char folder[MAX_PATH];
static int phase, frames, phase_start, navigated, ready, rendered_id;
static int width, height, result = 1;

static void fail(const char* step)
{
    fprintf(stderr, "Document web render test failed: %s, phase=%d, native=0x%08x\n",
        step, phase, webview ? (unsigned)xuiWebViewGetNativeError(webview) : 0);
    result = 2;
    xgeQuit();
}

static int json_integer(const char* json, const char* key, int* value)
{
    char needle[40];
    char* end;
    long number;
    int count = snprintf(needle, sizeof(needle), "\"%s\":", key);
    const char* found;
    if (count <= 0 || count >= (int)sizeof(needle)) return 0;
    found = strstr(json, needle);
    if (!found) return 0;
    number = strtol(found + count, &end, 10);
    if (end == found + count || number < 0 || number > 4096) return 0;
    *value = (int)number;
    return 1;
}

static void on_message(xui_widget widget, const char* source,
    const char* json, size_t bytes, void* user)
{
    int id, measured_width, measured_height;
    (void)widget; (void)user;
    if (strcmp(source, "https://xui-document.invalid/renderer.html") != 0 ||
            bytes != strlen(json)) { fail("message source"); return; }
    if (strstr(json, "\"kind\":\"ready\"")) {
        if (!strstr(json, "\"katex\":true") ||
                !strstr(json, "\"mermaid\":true") ||
                !strstr(json, "\"dompurify\":true")) {
            fprintf(stderr, "renderer readiness: %s\n", json);
            fail("render libraries"); return;
        }
        ready = 1;
    } else if (strstr(json, "\"kind\":\"rendered\"")) {
        if (!json_integer(json, "id", &id) ||
                !json_integer(json, "width", &measured_width) ||
                !json_integer(json, "height", &measured_height) ||
                !measured_width || !measured_height) {
            fprintf(stderr, "measurement: %s\n", json);
            fail("measurement"); return;
        }
        rendered_id = id;
        width = measured_width;
        height = measured_height;
    } else {
        fprintf(stderr, "renderer message: %s\n", json);
        fail("renderer error/unexpected message");
    }
}

static void on_webview(xui_widget widget, int event, int32_t error, void* user)
{
    (void)user;
    if (event == XUI_WEBVIEW_EVENT_READY) {
        if (xuiWebViewMapLocalFolder(widget, "xui-document.invalid", folder,
                    XUI_WEB_LOCAL_ACCESS_SAME_ORIGIN) != XUI_OK ||
                xuiWebViewSetMessageHandler(widget,
                    "https://xui-document.invalid", on_message, NULL) != XUI_OK ||
                xuiWebViewNavigate(widget,
                    "https://xui-document.invalid/renderer.html") != XUI_OK)
            fail("map/bridge/navigate");
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
    DWORD length = GetFullPathNameA("res\\xui_document_web", MAX_PATH,
        folder, NULL);
    if (!length || length >= MAX_PATH) return 0;
    desc.iSize = sizeof(desc);
    desc.onEvent = on_webview;
    return xuiCreate(&context) == XUI_OK &&
        xuiWidgetCreate(context, &root) == XUI_OK &&
        xuiSetRootWidget(context, root) == XUI_OK &&
        xuiWidgetSetRect(root, (xui_rect_t){0, 0, 600, 400}) == XUI_OK &&
        xuiWebViewCreate(context, &webview, &desc) == XUI_OK &&
        xuiWidgetSetRect(webview, (xui_rect_t){20, 20, 500, 350}) == XUI_OK &&
        xuiWidgetAddChild(root, webview) == XUI_OK &&
        xuiWebViewInitializeAsync(webview) == XUI_OK;
}

static int check_capture(int expected_width, int expected_height)
{
    const void* png = NULL;
    size_t bytes = 0, pixels = 0;
    xge_image_t image = {0};
    int x, y;
    if (xuiWebRequestGetState(capture) != XUI_WEB_REQUEST_COMPLETED ||
            xuiWebRequestGetResultPng(capture, &png, &bytes) != XUI_OK ||
            !png || bytes > 64u * 1024u * 1024u ||
            xgeImageLoadMemory(&image, png, (int)bytes) != XGE_OK) return 0;
    for (y = 0; y < image.iHeight; y++) {
        const unsigned char* row = (const unsigned char*)image.pPixels +
            (size_t)y * (size_t)image.iStride;
        for (x = 0; x < image.iWidth; x++) {
            const unsigned char* pixel = row + (size_t)x * 4;
            if (pixel[0] < 180 && pixel[1] < 180 && pixel[2] < 180) pixels++;
        }
    }
    fprintf(stderr, "rendered PNG %dx%d, dark pixels=%zu\n",
        image.iWidth, image.iHeight, pixels);
    x = image.iWidth == expected_width && image.iHeight == expected_height &&
        pixels > 20;
    xgeImageFree(&image);
    return x;
}

static int frame(void* user)
{
    (void)user;
    if (!context && !setup()) { fail("setup"); return XGE_ERROR; }
    if (++frames > 900) { fail("timeout"); return XGE_ERROR; }
    if (xuiDispatchPendingEvents(context) != XUI_OK ||
            xuiLayout(context) != XUI_OK ||
            xuiUpdate(context, .016f) != XUI_OK) {
        fail("layout/update"); return XGE_ERROR;
    }
    if (phase == 0 && ready && navigated) {
        static const char math[] = "{\"kind\":\"render\",\"id\":1,\"type\":\"math\","
            "\"source\":\"\\\\frac{a^2+b^2}{c}\",\"display\":false}";
        if (xuiWebViewPostMessageJson(webview, math, sizeof(math) - 1) != XUI_OK) {
            fail("math request"); return XGE_ERROR;
        }
        phase = 1;
    } else if (phase == 1 && rendered_id == 1) {
        if (xuiWidgetSetRect(webview,
                (xui_rect_t){599, 399,
                    (float)(width > 500 ? width : 500),
                    (float)(height > 350 ? height : 350)}) != XUI_OK) {
            fail("math viewport"); return XGE_ERROR;
        }
        phase_start = frames;
        phase = 2;
    } else if (phase == 2 && frames > phase_start + 12) {
        if (xuiWebViewCapturePngAsync(webview, &capture) != XUI_OK) {
            fail("math capture"); return XGE_ERROR;
        }
        phase = 3;
    } else if (phase == 3 &&
            xuiWebRequestGetState(capture) != XUI_WEB_REQUEST_PENDING) {
        if (!check_capture(width > 500 ? width : 500,
                    height > 350 ? height : 350) ||
                xuiWidgetSetRect(webview,
                    (xui_rect_t){20, 20, 500, 350}) != XUI_OK) {
            fail("math visible pixels"); return XGE_ERROR;
        }
        xuiWebRequestRelease(capture); capture = NULL;
        phase_start = frames;
        phase = 4;
    } else if (phase == 4 && frames > phase_start + 3) {
        static const char diagram[] = "{\"kind\":\"render\",\"id\":2,"
            "\"type\":\"diagram\",\"source\":\"graph TD; A[Start]-->B[Finish]\"}";
        if (xuiWebViewPostMessageJson(webview, diagram,
                sizeof(diagram) - 1) != XUI_OK) {
            fail("Mermaid request"); return XGE_ERROR;
        }
        phase = 5;
    } else if (phase == 5 && rendered_id == 2) {
        if (xuiWidgetSetRect(webview,
                (xui_rect_t){599, 399,
                    (float)(width > 500 ? width : 500),
                    (float)(height > 350 ? height : 350)}) != XUI_OK) {
            fail("Mermaid viewport"); return XGE_ERROR;
        }
        phase_start = frames;
        phase = 6;
    } else if (phase == 6 && frames > phase_start + 12) {
        if (xuiWebViewCapturePngAsync(webview, &capture) != XUI_OK) {
            fail("Mermaid capture"); return XGE_ERROR;
        }
        phase = 7;
    } else if (phase == 7 &&
            xuiWebRequestGetState(capture) != XUI_WEB_REQUEST_PENDING) {
        if (!check_capture(width > 500 ? width : 500,
                    height > 350 ? height : 350)) {
            fail("Mermaid visible pixels"); return XGE_ERROR;
        }
        xuiWebRequestRelease(capture); capture = NULL;
        {
            static const char html[] = "{\"kind\":\"render\",\"id\":3,"
                "\"type\":\"html\",\"available\":320,"
                "\"source\":\"<div style='background:#e4e8ef;color:#14243a;padding:12px'>"
                "<h2>Safe HTML</h2><p>Styled browser layout.</p></div>"
                "<script>window.parent.__xuiAttack=1</script>\"}";
            if (xuiWebViewPostMessageJson(webview, html,
                    sizeof(html) - 1) != XUI_OK) {
                fail("HTML request"); return XGE_ERROR;
            }
        }
        phase = 8;
    } else if (phase == 8 && rendered_id == 3) {
        static const char probe[] =
            "(() => { const f=document.querySelector('#content iframe');"
            "const d=f&&f.contentDocument;return !!d&&"
            "d.body.textContent.includes('Safe HTML')&&"
            "d.querySelectorAll('script,iframe,form').length===0&&"
            "f.sandbox.contains('allow-same-origin')&&"
            "!f.sandbox.contains('allow-scripts')&&"
            "window.__xuiAttack===undefined&&"
            "d.defaultView.getComputedStyle(d.querySelector('div'))"
            ".backgroundColor==='rgb(228, 232, 239)' })()";
        if (width != 320 || height < 40 || height > 250 ||
                xuiWebViewEvalScriptAsync(webview, probe,
                    sizeof(probe) - 1, &html_probe) != XUI_OK) {
            fail("HTML measurement/probe"); return XGE_ERROR;
        }
        phase = 9;
    } else if (phase == 9 && html_probe &&
            xuiWebRequestGetState(html_probe) != XUI_WEB_REQUEST_PENDING) {
        const char* json = NULL; size_t bytes = 0;
        if (xuiWebRequestGetResultJson(html_probe, &json, &bytes) != XUI_OK ||
                bytes != 4 || memcmp(json, "true", 4) != 0 ||
                xuiWidgetSetRect(webview,
                    (xui_rect_t){599, 399, 500, 350}) != XUI_OK) {
            fail("HTML sandbox contents"); return XGE_ERROR;
        }
        xuiWebRequestRelease(html_probe); html_probe = NULL;
        phase_start = frames;
        phase = 10;
    } else if (phase == 10 && frames > phase_start + 12) {
        if (xuiWebViewCapturePngAsync(webview, &capture) != XUI_OK) {
            fail("HTML capture"); return XGE_ERROR;
        }
        phase = 11;
    } else if (phase == 11 &&
            xuiWebRequestGetState(capture) != XUI_WEB_REQUEST_PENDING) {
        if (!check_capture(500, 350)) {
            fail("HTML visible pixels"); return XGE_ERROR;
        }
        puts("XUI offline KaTeX/Mermaid/HTML measured rendering, HTML sandbox and PNG capture passed");
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
    desc.iWidth = 600;
    desc.iHeight = 400;
    desc.sTitle = "XUI Document offline browser renderer";
    desc.iRunMode = XGE_RUN_GAME_LOOP;
    desc.iTargetFPS = 60;
    if (xgeInit(&desc) != XGE_OK) return 2;
    xgeRun(frame, NULL);
    if (capture) xuiWebRequestRelease(capture);
    if (html_probe) xuiWebRequestRelease(html_probe);
    if (context) xuiDestroy(context);
    xgeUnit();
    return result;
}
