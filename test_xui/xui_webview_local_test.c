/* Real WebView2 virtual-host local HTML, CSS, JavaScript and SVG integration. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "xge.h"
#include "xui.h"
#include "src/xui_webview_internal.h"

static xui_context context;
static xui_widget root, webview;
static xui_web_request request, message_request, self_nav_request, untrusted_request;
static char folder[MAX_PATH], index_path[MAX_PATH], css_path[MAX_PATH];
static char script_path[MAX_PATH], image_path[MAX_PATH], other_path[MAX_PATH];
static int phase, frames, phase_start, navigated, messages, result = 1;

static void fail(const char* step)
{
    fprintf(stderr, "WebView local test failed: %s, phase=%d, native=0x%08x\n",
        step, phase, webview ? (unsigned)xuiWebViewGetNativeError(webview) : 0);
    result = 2;
    xgeQuit();
}

static int write_asset(char* path, const char* name, const char* text)
{
    HANDLE file;
    DWORD written = 0;
    size_t length = strlen(text);
    int count = snprintf(path, MAX_PATH, "%s\\%s", folder, name);
    if (count < 0 || count >= MAX_PATH || length > 0xffffffffu) return 0;
    file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return 0;
    if (!WriteFile(file, text, (DWORD)length, &written, NULL) ||
            written != (DWORD)length) {
        CloseHandle(file);
        return 0;
    }
    return CloseHandle(file) != 0;
}

static int create_assets(void)
{
    char path[MAX_PATH];
    DWORD absolute_length;
    int count = snprintf(path, sizeof(path), "build\\webview\\local-assets-%lu",
        (unsigned long)GetCurrentProcessId());
    if (count < 0 || count >= (int)sizeof(path)) return 0;
    absolute_length = GetFullPathNameA(path, MAX_PATH, folder, NULL);
    if (!absolute_length || absolute_length >= MAX_PATH ||
            !CreateDirectoryA(folder, NULL)) return 0;
    return write_asset(index_path, "index.html",
        "<!doctype html><title>Local assets</title>"
        "<link rel=stylesheet href=style.css><script src=app.js></script>"
        "<p id=probe>Local page</p><img id=icon src=icon.svg>") &&
        write_asset(css_path, "style.css", "#probe{color:rgb(9, 32, 77)}") &&
        write_asset(script_path, "app.js",
            "window.assetLoaded='local-js';"
            "chrome.webview.addEventListener('message',e=>{"
            "window.hostMessage=e.data;"
            "chrome.webview.postMessage({kind:'reply',echo:e.data.kind,text:e.data.text});"
            "});") &&
        write_asset(image_path, "icon.svg",
            "<svg xmlns='http://www.w3.org/2000/svg' width='7' height='5'>"
            "<rect width='7' height='5' fill='red'/></svg>") &&
        write_asset(other_path, "other.html",
            "<!doctype html><title>Untrusted origin</title>");
}

static void delete_assets(void)
{
    if (*index_path) DeleteFileA(index_path);
    if (*css_path) DeleteFileA(css_path);
    if (*script_path) DeleteFileA(script_path);
    if (*image_path) DeleteFileA(image_path);
    if (*other_path) DeleteFileA(other_path);
    if (*folder) RemoveDirectoryA(folder);
}

static void on_message(xui_widget widget, const char* source,
    const char* json, size_t bytes, void* user)
{
    (void)widget; (void)user;
    if (strcmp(source, "https://xui-assets.invalid/index.html") != 0 ||
            bytes != strlen(json)) { fail("message source/length"); return; }
    if (messages == 0 && strstr(json, "hello")) messages = 1;
    else if (messages == 1 && strstr(json, "reply") &&
            strstr(json, "host") && strstr(json, "echo")) messages = 2;
    else fail("message payload/order");
}

static void on_webview(xui_widget widget, int event, int32_t error, void* user)
{
    (void)user;
    if (event == XUI_WEBVIEW_EVENT_READY) {
        if (xuiWebViewMapLocalFolder(widget, "bad/host", folder,
                    XUI_WEB_LOCAL_ACCESS_SAME_ORIGIN) != XUI_ERROR_INVALID_ARGUMENT ||
                xuiWebViewMapLocalFolder(widget, "xui-assets.invalid", "relative",
                    XUI_WEB_LOCAL_ACCESS_SAME_ORIGIN) != XUI_ERROR_INVALID_ARGUMENT ||
                xuiWebViewMapLocalFolder(widget, "xui-assets.invalid", folder,
                    99) != XUI_ERROR_INVALID_ARGUMENT ||
                xuiWebViewMapLocalFolder(widget, "xui-assets.invalid", folder,
                    XUI_WEB_LOCAL_ACCESS_SAME_ORIGIN) != XUI_OK ||
                xuiWebViewMapLocalFolder(widget, "xui-untrusted.invalid", folder,
                    XUI_WEB_LOCAL_ACCESS_SAME_ORIGIN) != XUI_OK ||
                xuiWebViewSetMessageHandler(widget, "https://xui-assets.invalid/path",
                    on_message, NULL) != XUI_ERROR_INVALID_ARGUMENT ||
                xuiWebViewSetMessageHandler(widget, "https://xui-assets.invalid",
                    on_message, NULL) != XUI_OK ||
                xuiWebViewNavigate(widget,
                    "https://xui-assets.invalid/index.html") != XUI_OK)
            fail("map/navigate");
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
    if (!create_assets() ||
            xuiCreate(&context) != XUI_OK ||
            xuiWidgetCreate(context, &root) != XUI_OK ||
            xuiSetRootWidget(context, root) != XUI_OK ||
            xuiWidgetSetRect(root, (xui_rect_t){0, 0, 480, 320}) != XUI_OK ||
            xuiWebViewCreate(context, &webview, &desc) != XUI_OK ||
            xuiWidgetSetRect(webview, (xui_rect_t){20, 20, 400, 240}) != XUI_OK ||
            xuiWidgetAddChild(root, webview) != XUI_OK ||
            xuiWebViewInitializeAsync(webview) != XUI_OK) return 0;
    return 1;
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
        static const char script[] =
            "({title:document.title,js:window.assetLoaded,"
            "css:getComputedStyle(document.getElementById('probe')).color,"
            "image:document.getElementById('icon').naturalWidth})";
        if (xuiWebViewEvalScriptAsync(webview, script, sizeof(script) - 1,
                &request) != XUI_OK) {
            fail("inspect local assets"); return XGE_ERROR;
        }
        phase = 1;
    } else if (phase == 1 &&
            xuiWebRequestGetState(request) != XUI_WEB_REQUEST_PENDING) {
        const char* json = NULL;
        size_t bytes = 0;
        if (xuiWebRequestGetResultJson(request, &json, &bytes) != XUI_OK ||
                !json || bytes != strlen(json) ||
                !strstr(json, "Local assets") || !strstr(json, "local-js") ||
                !strstr(json, "rgb(9, 32, 77)") || !strstr(json, "\"image\":7") ||
                xuiWebViewEvalScriptAsync(webview,
                    "chrome.webview.postMessage({kind:'hello',text:'汉字😀'})",
                    strlen("chrome.webview.postMessage({kind:'hello',text:'汉字😀'})"),
                    &message_request) != XUI_OK) {
            fprintf(stderr, "local asset result: %s\n", json ? json : "(null)");
            fail("HTML/CSS/JS/SVG or page message"); return XGE_ERROR;
        }
        phase = 2;
    } else if (phase == 2 && messages >= 1 &&
            xuiWebRequestGetState(message_request) != XUI_WEB_REQUEST_PENDING) {
        static const char host_json[] = "{\"kind\":\"host\",\"text\":\"echo\"}";
        if (xuiWebRequestGetState(message_request) != XUI_WEB_REQUEST_COMPLETED ||
                xuiWebViewPostMessageJson(webview, host_json,
                    sizeof(host_json) - 1) != XUI_OK) {
            fail("host JSON message"); return XGE_ERROR;
        }
        phase = 3;
    } else if (phase == 3 && messages >= 2) {
        static const char self_nav[] =
            "location.href='https://xui-untrusted.invalid/other.html'";
        if (xuiWebViewEvalScriptAsync(webview, self_nav,
                sizeof(self_nav) - 1, &self_nav_request) != XUI_OK) {
            fail("page initiated navigation"); return XGE_ERROR;
        }
        phase = 4;
    } else if (phase == 4 && navigated >= 2) {
        static const char intruder[] = "chrome.webview.postMessage({kind:'intruder'})";
        if (xuiWebRequestGetState(self_nav_request) == XUI_WEB_REQUEST_PENDING ||
                xuiWebViewPostMessageJson(webview, "{}", 2) !=
                    XUI_ERROR_INVALID_STATE ||
                xuiWebViewEvalScriptAsync(webview, intruder,
                    sizeof(intruder) - 1, &untrusted_request) != XUI_OK) {
            fail("untrusted host send/source"); return XGE_ERROR;
        }
        phase_start = frames;
        phase = 5;
    } else if (phase == 5 && frames > phase_start + 30 &&
            xuiWebRequestGetState(untrusted_request) != XUI_WEB_REQUEST_PENDING) {
        if (messages != 2 ||
                xuiWebRequestGetState(untrusted_request) != XUI_WEB_REQUEST_COMPLETED ||
                xuiWebViewUnmapLocalFolder(webview, "xui-assets.invalid") != XUI_OK ||
                xuiWebViewUnmapLocalFolder(webview, "xui-untrusted.invalid") != XUI_OK ||
                xuiWebViewSetMessageHandler(webview, NULL, NULL, NULL) != XUI_OK ||
                xuiWebViewPostMessageJson(webview, "{}", 2) !=
                    XUI_ERROR_INVALID_STATE) {
            fail("untrusted reply/unmap/close bridge"); return XGE_ERROR;
        }
        puts("XUI WebView2 local assets and origin-checked JSON messages passed");
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
    desc.sTitle = "XUI WebView local resources";
    desc.iRunMode = XGE_RUN_GAME_LOOP;
    desc.iTargetFPS = 60;
    if (xgeInit(&desc) != XGE_OK) return 2;
    xgeRun(frame, NULL);
    if (request) xuiWebRequestRelease(request);
    if (message_request) xuiWebRequestRelease(message_request);
    if (self_nav_request) xuiWebRequestRelease(self_nav_request);
    if (untrusted_request) xuiWebRequestRelease(untrusted_request);
    if (context) xuiDestroy(context);
    xgeUnit();
    delete_assets();
    return result;
}
