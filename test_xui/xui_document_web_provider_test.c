/* Real Markdown DocumentView using cached KaTeX, Mermaid and HTML browser pixels. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xge.h"
#include "xui_document_ui.h"
#include "src/xui_webview_internal.h"

static xui_context context;
static xui_proxy_t proxy;
static xui_surface target;
static xui_font font;
static xui_widget root, view, message_list;
static xui_document document;
static xui_doc_web_provider provider;
static int frames, invalidations, result = 1, phase;
static int first_graph_right;
static int message_invalidations, message_math_draws, message_diagram_draws;
static int message_html_draws;
static int error_heading_draws, error_detail_draws, error_source_draws;
static int mermaid_error_heading_draws, mermaid_error_detail_draws;
static int html_error_heading_draws, html_error_detail_draws;
static int worker_error_heading_draws, worker_error_detail_draws;
static int worker_future_heading_draws, worker_future_detail_draws;
static int worker_palette_heading_draws, worker_palette_detail_draws;
static int worker_timeout_heading_draws, worker_timeout_detail_draws;
static int worker_timeout_updates;
static int decode_error_heading_draws, decode_error_detail_draws;
static int decode_injections, decode_releases, inject_decode_error;
static int crop_error_heading_draws, crop_error_detail_draws;
static int crop_injections, crop_releases, inject_crop_error;
static int process_error_heading_draws, process_error_detail_draws;
static uint32_t killed_browser_pid, last_killed_browser_pid;
static int final_failure_updates;
static char isolated_profile[MAX_PATH * 2];
static xui_draw_text_proc original_draw_text;
static xui_surface_load_memory_proc original_surface_load_memory;
static xui_surface_create_proc original_surface_create;
static xui_surface_destroy_proc original_surface_destroy;
static xui_surface injected_surface, injected_crop_surface;
static uint32_t prior_ready, prior_drawn, prior_failed;
static unsigned char* first_pixels;
static const char initial_markdown[] =
    "# Browser-backed objects\n\n"
    "Inline $\\frac{a^2+b^2}{c}$ formula.\n\n"
    "```mermaid\n"
    "graph TD; A[Start]-->B[Finish]\n"
    "```\n";
static const char changed_markdown[] =
    "# Browser-backed objects\n\n"
    "Inline $\\sqrt{x^2+y^2}$ formula.\n\n"
    "```mermaid\n"
    "graph TD; A[Changed]-->B[Finish]\n"
    "```\n";
static const char html_markdown[] =
    "<div style=\"background:#d00000;color:#ffffff;padding:12px\">"
    "Rendered HTML</div>\n";
static const char invalid_math_markdown[] =
    "Invalid $\\definitely_not_a_command{x}$ formula.\n";
static const char recovery_math_markdown[] =
    "Recovered $\\sqrt{9}$ formula.\n";
static const char decode_recovery_markdown[] =
    "Recovered $\\sqrt{16}$ formula.\n";
static const char crop_recovery_markdown[] =
    "Recovered $\\sqrt{25}$ formula.\n";
static const char invalid_mermaid_markdown[] =
    "```mermaid\nnot-a-mermaid-diagram\n```\n";
static const char invalid_html_markdown[] =
    "<div style=\"height:5000px\">Too tall</div>\n";

static int inject_surface_load_memory(xui_proxy paint_proxy,
    xui_surface* out, const void* data, int bytes, uint32_t flags)
{
    int status = original_surface_load_memory(paint_proxy, out,
        data, bytes, flags);
    if (phase == 16 && inject_decode_error && status == XUI_OK &&
            out && *out) {
        injected_surface = *out;
        inject_decode_error = 0;
        decode_injections++;
        return XUI_ERROR_INVALID_STATE;
    }
    return status;
}

static int inject_surface_create(xui_proxy paint_proxy,
    xui_surface* out, const xui_surface_desc_t* desc)
{
    int status = original_surface_create(paint_proxy, out, desc);
    if (phase == 18 && inject_crop_error && status == XUI_OK &&
            out && *out && desc && desc->iWidth < 512 &&
            desc->iHeight < 350) {
        injected_crop_surface = *out;
        inject_crop_error = 0;
        crop_injections++;
        return XUI_ERROR_INVALID_STATE;
    }
    return status;
}

static void track_surface_destroy(xui_proxy paint_proxy, xui_surface surface)
{
    if (surface == injected_surface) {
        decode_releases++;
        injected_surface = NULL;
    }
    if (surface == injected_crop_surface) {
        crop_releases++;
        injected_crop_surface = NULL;
    }
    original_surface_destroy(paint_proxy, surface);
}

static int record_draw_text(xui_proxy paint_proxy, xui_draw_context draw, const xui_text_item_t* pTextItem, xui_rect_t bounds, uint32_t color, uint32_t flags)
{
    const char* text = pTextItem && pTextItem->iSize >= sizeof(*pTextItem) ? pTextItem->sText : NULL;

    if (phase == 8 && text) {
        if (strstr(text, "Formula error")) error_heading_draws++;
        if (strstr(text, "KaTeX parse error")) error_detail_draws++;
        if (strstr(text, "_not_a_command")) error_source_draws++;
    }
    if (phase == 10 && text) {
        if (strstr(text, "Mermaid error")) mermaid_error_heading_draws++;
        if (strstr(text, "diagram type") ||
                strstr(text, "not-a-mermaid-diagram"))
            mermaid_error_detail_draws++;
    }
    if (phase == 11 && text) {
        if (strstr(text, "HTML error")) html_error_heading_draws++;
        if (strstr(text, "size exceeds limit")) html_error_detail_draws++;
    }
    if (phase == 12 && text) {
        if (strstr(text, "HTML error")) worker_error_heading_draws++;
        if (strstr(text, "Document renderer")) worker_error_detail_draws++;
    }
    if (phase == 13 && text) {
        if (strstr(text, "Formula error")) worker_future_heading_draws++;
        if (strstr(text, "Document renderer")) worker_future_detail_draws++;
    }
    if (phase == 14 && text) {
        if (strstr(text, "Formula error")) worker_palette_heading_draws++;
        if (strstr(text, "Document renderer")) worker_palette_detail_draws++;
    }
    if (phase == 15 && text) {
        if (strstr(text, "Formula error")) worker_timeout_heading_draws++;
        if (strstr(text, "response timed")) worker_timeout_detail_draws++;
    }
    if (phase == 16 && text) {
        if (strstr(text, "Formula error")) decode_error_heading_draws++;
        if (strstr(text, "screenshot")) decode_error_detail_draws++;
    }
    if (phase == 18 && text) {
        if (strstr(text, "Formula error")) crop_error_heading_draws++;
        if (strstr(text, "crop")) crop_error_detail_draws++;
    }
    if (phase == 21 && text) {
        if (strstr(text, "Formula error")) process_error_heading_draws++;
        if (strstr(text, "Document renderer browser failed"))
            process_error_detail_draws++;
    }
    return original_draw_text(paint_proxy, draw, pTextItem, bounds, color, flags);
}

static int graph_right_edge(const unsigned char* pixels)
{
    int x, y, right = -1;
    for (y = 80; y < 250; y++) for (x = 5; x < 200; x++) {
        const unsigned char* pixel = pixels + ((size_t)y * 600u + (size_t)x) * 4u;
        if (pixel[0] < 230 || pixel[1] < 230 || pixel[2] < 230)
            if (x > right) right = x;
    }
    return right;
}

static int save_dark_target(const char* path, size_t* bright_out)
{
    size_t bright = 0;
    int x, y;
    unsigned char* pixels = malloc(600u * 400u * 4u);
    if (!pixels || !proxy.surfaceReadRGBA ||
            proxy.surfaceReadRGBA(&proxy, target, pixels, 600 * 4) != XUI_OK ||
            xgeImageSavePNG(path, 600, 400, pixels, 600 * 4) != XGE_OK) {
        free(pixels);
        return 0;
    }
    for (y = 40; y < 250; y++) for (x = 20; x < 200; x++) {
        const unsigned char* pixel = pixels +
            ((size_t)y * 600u + (size_t)x) * 4u;
        if (pixel[0] > 235 && pixel[1] > 235 && pixel[2] > 235)
            bright++;
    }
    x = bright >= 50 && bright <= 2000 &&
        pixels[((size_t)200 * 600u + 400u) * 4u] == 22 &&
        pixels[((size_t)200 * 600u + 400u) * 4u + 1] == 27 &&
        pixels[((size_t)200 * 600u + 400u) * 4u + 2] == 34;
    free(pixels);
    *bright_out = bright;
    return x;
}

static int save_html_target(const char* path, size_t* red_out)
{
    size_t red = 0;
    int x, y;
    unsigned char* pixels = malloc(600u * 400u * 4u);
    if (!pixels || !proxy.surfaceReadRGBA ||
            proxy.surfaceReadRGBA(&proxy, target, pixels, 600 * 4) != XUI_OK ||
            xgeImageSavePNG(path, 600, 400, pixels, 600 * 4) != XGE_OK) {
        free(pixels);
        return 0;
    }
    for (y = 10; y < 310; y++) for (x = 10; x < 510; x++) {
        const unsigned char* pixel = pixels +
            ((size_t)y * 600u + (size_t)x) * 4u;
        if (pixel[0] >= 150 && pixel[1] <= 100 && pixel[2] <= 100)
            red++;
    }
    free(pixels);
    *red_out = red;
    return red >= 100;
}

static int save_dark_error_target(const char* path, size_t* fill_out)
{
    size_t fill = 0;
    int x, y;
    unsigned char* pixels = malloc(600u * 400u * 4u);
    if (!pixels || !proxy.surfaceReadRGBA ||
            proxy.surfaceReadRGBA(&proxy, target, pixels, 600 * 4) != XUI_OK ||
            xgeImageSavePNG(path, 600, 400, pixels, 600 * 4) != XGE_OK) {
        free(pixels);
        return 0;
    }
    for (y = 10; y < 310; y++) for (x = 10; x < 510; x++) {
        const unsigned char* pixel = pixels +
            ((size_t)y * 600u + (size_t)x) * 4u;
        if (pixel[0] >= 50 && pixel[0] <= 75 &&
                pixel[1] >= 25 && pixel[1] <= 55 &&
                pixel[2] >= 30 && pixel[2] <= 65) fill++;
    }
    free(pixels);
    *fill_out = fill;
    return fill >= 100;
}

static void invalidate(void* user)
{
    (void)user;
    invalidations++;
    if (view) (void)xuiDocumentViewInvalidateObjects(view);
    if (message_list) {
        message_invalidations++;
        (void)xuiMessageListInvalidateNodeDocumentObjects(message_list, "answer");
    }
}

static int draw_message_object(xui_document_snapshot snapshot,
    xui_doc_node_id node, xui_proxy paint_proxy, xui_draw_context draw,
    xui_rect_t bounds, void* user)
{
    xui_doc_node_info_t info = {0};
    int status = xuiDocumentWebObjectDraw(snapshot, node, paint_proxy,
        draw, bounds, user);
    info.iSize = sizeof(info);
    if (status == XUI_OK &&
            xuiDocumentSnapshotGetNode(snapshot, node, &info) == XUI_OK) {
        if (info.iKind == XUI_DOC_MATH) message_math_draws++;
        if (info.iKind == XUI_DOC_DIAGRAM) message_diagram_draws++;
        if (info.iKind == XUI_DOC_HTML) message_html_draws++;
    }
    return status;
}

static int setup_message_list(void)
{
    xui_message_list_desc_t list_desc = {0};
    xui_message_document_desc_t binding = {0};
    xui_message_node_t node = {0};
    node.iSize = sizeof(node);
    node.sId = "answer";
    node.iType = XUI_MESSAGE_NODE_OTHER;
    node.sSender = "Agent";
    node.sText = "fallback";
    list_desc.iSize = sizeof(list_desc);
    list_desc.arrNodes = &node;
    list_desc.iNodeCount = 1;
    if (xuiMessageListCreate(context, &message_list, &list_desc) != XUI_OK ||
            xuiWidgetSetRect(message_list,
                (xui_rect_t){10, 10, 360, 360}) != XUI_OK ||
            xuiWidgetAddChild(root, message_list) != XUI_OK) return 0;
    binding.iSize = sizeof(binding);
    binding.pDocument = document;
    binding.tRenderer.iSize = sizeof(binding.tRenderer);
    binding.tRenderer.tFonts = (xui_doc_font_set_t){font,font,font,font,font};
    binding.tRenderer.onObjectMeasure = xuiDocumentWebObjectMeasure;
    binding.tRenderer.onObjectDraw = draw_message_object;
    binding.tRenderer.pUser = provider;
    return xuiMessageListSetNodeDocument(message_list, "answer", &binding) == XUI_OK;
}

static int replace_web_provider_with_timeout(const char* relative_folder,
    uint32_t timeout_ms)
{
    xui_doc_web_provider_desc_t desc = {0};
    char folder[MAX_PATH];
    DWORD length = GetFullPathNameA(relative_folder, MAX_PATH, folder, NULL);
    if (!length || length >= MAX_PATH) return 0;
    xuiWidgetDestroy(message_list);
    message_list = NULL;
    xuiDocumentWebProviderRelease(provider);
    provider = NULL;
    desc.iSize = sizeof(desc);
    desc.pContext = context;
    desc.pParent = root;
    desc.sAssetsFolder = folder;
    desc.sUserDataFolder = isolated_profile;
    desc.onInvalidate = invalidate;
    desc.iTimeoutMs = timeout_ms;
    return xuiDocumentWebProviderCreate(&desc, &provider) == XUI_OK &&
        setup_message_list();
}

static int replace_web_provider(const char* relative_folder)
{
    return replace_web_provider_with_timeout(relative_folder, 0);
}

static int setup_missing_assets_provider(void)
{
    return replace_web_provider("build\\webview\\missing-document-assets");
}

static int setup_stalled_assets_provider(void)
{
    static const char page[] = "<!doctype html><meta charset=\"utf-8\">"
        "<script>window.chrome.webview.postMessage({kind:'ready',"
        "katex:true,mermaid:true,dompurify:true});"
        "window.chrome.webview.addEventListener('message',()=>{});"
        "</script><body></body>";
    FILE* file;
    int wrote;
    if (!CreateDirectoryA("build\\webview\\stalled-document-assets",
                NULL) && GetLastError() != ERROR_ALREADY_EXISTS) return 0;
    file = fopen("build\\webview\\stalled-document-assets\\renderer.html",
        "wb");
    if (!file) return 0;
    wrote = fwrite(page, 1, sizeof(page) - 1, file) == sizeof(page) - 1;
    if (fclose(file) != 0 || !wrote) return 0;
    return replace_web_provider_with_timeout(
        "build\\webview\\stalled-document-assets", 1000);
}

static int setup_dark_view(void)
{
    xui_doc_view_desc_t desc = {0};
    desc.iSize = sizeof(desc);
    desc.pDocument = document;
    desc.tRenderer.iSize = sizeof(desc.tRenderer);
    desc.tRenderer.tFonts = (xui_doc_font_set_t){font,font,font,font,font};
    desc.tRenderer.iTextColor = XUI_COLOR_RGBA(240, 246, 252, 255);
    desc.tRenderer.onObjectMeasure = xuiDocumentWebObjectMeasure;
    desc.tRenderer.onObjectDraw = xuiDocumentWebObjectDraw;
    desc.tRenderer.pUser = provider;
    desc.iBackgroundColor = XUI_COLOR_RGBA(22, 27, 34, 255);
    return xuiDocumentViewCreate(context, &desc, &view) == XUI_OK &&
        xuiWidgetSetRect(view, (xui_rect_t){10, 10, 500, 300}) == XUI_OK &&
        xuiWidgetAddChild(root, view) == XUI_OK;
}

static int setup(void)
{
    xui_surface_desc_t surface = {0};
    xui_doc_desc_t doc_desc = {0};
    xui_doc_view_desc_t view_desc = {0};
    xui_doc_web_provider_desc_t provider_desc = {0};
    char folder[MAX_PATH];
    char temp[MAX_PATH];
    DWORD length = GetFullPathNameA("res\\xui_document_web", MAX_PATH,
        folder, NULL);
    DWORD temp_length = GetTempPathA(MAX_PATH, temp);
    int written;
    if (!length || length >= MAX_PATH || !temp_length ||
            temp_length >= MAX_PATH) return 0;
    written = snprintf(isolated_profile, sizeof(isolated_profile),
        "%sxui-document-web-provider-%lu-%lu", temp,
        (unsigned long)GetCurrentProcessId(), (unsigned long)GetTickCount());
    if (written <= 0 || (size_t)written >= sizeof(isolated_profile)) return 0;
    proxy = xuiProxyXge();
    original_draw_text = proxy.drawText;
    original_surface_load_memory = proxy.surfaceLoadMemory;
    original_surface_create = proxy.surfaceCreate;
    original_surface_destroy = proxy.surfaceDestroy;
    proxy.drawText = record_draw_text;
    proxy.surfaceLoadMemory = inject_surface_load_memory;
    proxy.surfaceCreate = inject_surface_create;
    proxy.surfaceDestroy = track_surface_destroy;
    if (xuiCreate(&context) != XUI_OK ||
            xuiSetProxy(context, &proxy) != XUI_OK ||
            proxy.fontLoadFile(&proxy, &font, "C:\\Windows\\Fonts\\segoeui.ttf",
                16, XUI_FONT_FORMAT_TTF) != XUI_OK ||
            xuiSetDefaultFont(context, font) != XUI_OK ||
            xuiInputViewport(context, 600, 400) != XUI_OK) return 0;
    surface.iKind = XUI_SURFACE_KIND_TEXTURE;
    surface.iFormat = XUI_SURFACE_FORMAT_RGBA8;
    surface.iWidth = 600; surface.iHeight = 400;
    surface.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
    if (proxy.surfaceCreate(&proxy, &target, &surface) != XUI_OK ||
            xuiWidgetCreate(context, &root) != XUI_OK ||
            xuiSetRootWidget(context, root) != XUI_OK ||
            xuiWidgetSetRect(root, (xui_rect_t){0, 0, 600, 400}) != XUI_OK)
        return 0;
    provider_desc.iSize = sizeof(provider_desc);
    provider_desc.pContext = context;
    provider_desc.pParent = root;
    provider_desc.sAssetsFolder = folder;
    provider_desc.sUserDataFolder = isolated_profile;
    provider_desc.onInvalidate = invalidate;
    if (xuiDocumentWebProviderCreate(&provider_desc, &provider) != XUI_OK)
        return 0;
    doc_desc.iSize = sizeof(doc_desc);
    doc_desc.iProfile = XUI_DOCUMENT_MARKDOWN;
    doc_desc.iMarkdownDialect = XUI_MD_EXTENDED;
    if (xuiDocumentCreate(&doc_desc, &document) != XUI_OK ||
            xuiDocumentLoadMarkdown(document, initial_markdown,
                sizeof(initial_markdown) - 1) != XUI_OK) return 0;
    view_desc.iSize = sizeof(view_desc);
    view_desc.pDocument = document;
    view_desc.tRenderer.iSize = sizeof(view_desc.tRenderer);
    view_desc.tRenderer.tFonts = (xui_doc_font_set_t){font,font,font,font,font};
    view_desc.tRenderer.onObjectMeasure = xuiDocumentWebObjectMeasure;
    view_desc.tRenderer.onObjectDraw = xuiDocumentWebObjectDraw;
    view_desc.tRenderer.pUser = provider;
    view_desc.iBackgroundColor = XUI_COLOR_WHITE;
    if (xuiDocumentViewCreate(context, &view_desc, &view) != XUI_OK ||
            xuiWidgetSetRect(view, (xui_rect_t){10, 10, 500, 300}) != XUI_OK ||
            xuiWidgetAddChild(root, view) != XUI_OK) return 0;
    return 1;
}

static xui_widget provider_worker_widget(void)
{
    xui_widget child, worker = NULL;
    for (child = xuiWidgetGetFirstChild(root); child;
            child = xuiWidgetGetNextSibling(child))
        if (xuiWidgetGetType(child) == xuiWebViewGetType(context)) {
            if (worker) return NULL;
            worker = child;
        }
    return worker;
}

static int provider_browser_pid(uint32_t* pid)
{
    xui_widget worker = provider_worker_widget();
    *pid = 0;
    return worker && xuiWebViewGetState(worker) == XUI_WEBVIEW_READY &&
        xuiWebViewTestBrowserProcessId(worker, pid) == XUI_OK &&
        *pid && *pid != GetCurrentProcessId();
}

static int terminate_provider_browser(uint32_t* pid)
{
    HANDLE browser;
    char image[MAX_PATH];
    DWORD image_size = sizeof(image);
    const char* basename;
    int terminated;
    if (!provider_browser_pid(pid)) return 0;
    browser = OpenProcess(PROCESS_TERMINATE |
        PROCESS_QUERY_LIMITED_INFORMATION, FALSE, *pid);
    if (!browser) return 0;
    if (!QueryFullProcessImageNameA(browser, 0, image, &image_size)) {
        CloseHandle(browser); return 0;
    }
    basename = strrchr(image, '\\');
    basename = basename ? basename + 1 : image;
    terminated = lstrcmpiA(basename, "msedgewebview2.exe") == 0 &&
        TerminateProcess(browser, 0xc0000005u);
    CloseHandle(browser);
    return terminated;
}

static int frame(void* user)
{
    xui_rect_i_t damage = {0, 0, 600, 400};
    xui_doc_web_provider_stats_t stats = {0};
    int error = XUI_OK;
    (void)user;
    if (!context && !setup()) { result = 2; xgeQuit(); return XGE_ERROR; }
    if (++frames > 2200) {
        stats.iSize = sizeof(stats);
        (void)xuiDocumentWebProviderGetStats(provider, &stats);
        fprintf(stderr, "Document web provider timed out: phase=%d "
            "invalidations=%d message-invalidations=%d math-draws=%d "
            "heading=%d detail=%d timeout-heading=%d timeout-detail=%d "
            "crop-heading=%d crop-detail=%d crop-injection=%d "
            "crop-release=%d queued=%u ready=%u failed=%u "
            "worker-failed=%u worker-error=%s\n",
            phase, invalidations, message_invalidations, message_math_draws,
            error_heading_draws, error_detail_draws,
            worker_timeout_heading_draws, worker_timeout_detail_draws,
            crop_error_heading_draws, crop_error_detail_draws,
            crop_injections, crop_releases,
            stats.iQueued, stats.iReady, stats.iFailed,
            stats.bWorkerFailed, stats.sWorkerError);
        result = 2; xgeQuit(); return XGE_ERROR;
    }
    if (phase == 15) worker_timeout_updates++;
    if (xgeBegin() != XGE_OK ||
            xuiDispatchPendingEvents(context) != XUI_OK ||
            xuiLayout(context) != XUI_OK ||
            xuiUpdate(context, .016f) != XUI_OK ||
            (error = xuiDocumentWebProviderUpdate(provider)) != XUI_OK ||
            proxy.surfaceClear(&proxy, target, XUI_COLOR_WHITE) != XUI_OK ||
            xuiRender(context, target, &damage, 1) != XUI_OK ||
            xgeEnd() != XGE_OK) {
        fprintf(stderr, "Document web provider frame failed: %d\n", error);
        result = 2; xgeQuit(); return XGE_ERROR;
    }
    stats.iSize = sizeof(stats);
    if (xuiDocumentWebProviderGetStats(provider, &stats) != XUI_OK) {
        result = 2; xgeQuit(); return XGE_ERROR;
    }
    if (phase < 12 && (stats.bWorkerFailed || stats.sWorkerError[0])) {
        fprintf(stderr, "Document web provider worker failed during object test\n");
        result = 2; xgeQuit(); return XGE_ERROR;
    }
    if (stats.iFailed && phase < 8) {
        fprintf(stderr, "Document web provider object failure: %u\n", stats.iFailed);
        result = 2; xgeQuit(); return XGE_ERROR;
    }
    if (phase == 0 && stats.iReady >= 2 && stats.iDrawn >= 2 &&
            invalidations >= 2) {
        xui_document_transaction transaction = NULL;
        xui_doc_txn_desc_t transaction_desc = {0};
        unsigned char* pixels = malloc(600u * 400u * 4u);
        if (!pixels || !proxy.surfaceReadRGBA ||
                proxy.surfaceReadRGBA(&proxy, target, pixels, 600 * 4) != XUI_OK ||
                xgeImageSavePNG("build/webview/xui_document_web_provider.png",
                    600, 400, pixels, 600 * 4) != XGE_OK) {
            free(pixels);
            fprintf(stderr, "Document web provider target readback failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        first_pixels = pixels;
        first_graph_right = graph_right_edge(pixels);
        prior_ready = stats.iReady;
        prior_drawn = stats.iDrawn;
        transaction_desc.iSize = sizeof(transaction_desc);
        transaction_desc.iDomain = XUI_DOC_SOURCE;
        if (xuiDocumentBeginTransaction(document, &transaction_desc,
                    &transaction) != XUI_OK ||
                xuiDocumentTxnReplaceSource(transaction, 0,
                    sizeof(initial_markdown) - 1, changed_markdown,
                    sizeof(changed_markdown) - 1) != XUI_OK ||
                xuiDocumentTxnCommit(transaction, NULL) != XUI_OK) {
            if (transaction) xuiDocumentTxnRelease(transaction);
            fprintf(stderr, "Document web provider source edit failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        xuiDocumentTxnRelease(transaction);
        phase = 1;
    } else if (phase == 1 && stats.iReady >= prior_ready + 2 &&
            stats.iDrawn >= prior_drawn + 2 && invalidations >= 4) {
        size_t changed = 0, i;
        int graph_right;
        unsigned char* pixels = malloc(600u * 400u * 4u);
        if (stats.iCacheBytes > 1024u * 1024u || !pixels ||
                !proxy.surfaceReadRGBA ||
                proxy.surfaceReadRGBA(&proxy, target, pixels, 600 * 4) != XUI_OK ||
                xgeImageSavePNG("build/webview/xui_document_web_provider_edited.png",
                    600, 400, pixels, 600 * 4) != XGE_OK) {
            free(pixels);
            fprintf(stderr, "Document web provider edited readback failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        for (i = 0; i < 600u * 400u * 4u; i++)
            changed += pixels[i] != first_pixels[i];
        graph_right = graph_right_edge(pixels);
        free(pixels);
        if (changed <= 100 || graph_right < first_graph_right + 20) {
            fprintf(stderr, "Document web provider edited image incomplete: "
                "changed=%zu graph-right=%d initial-right=%d\n", changed,
                graph_right, first_graph_right);
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI DocumentView KaTeX/Mermaid source edit rendered: ready=%u "
            "drawn=%u invalidations=%d changed-bytes=%zu cache=%llu\n",
            stats.iReady, stats.iDrawn, invalidations, changed,
            (unsigned long long)stats.iCacheBytes);
        xuiWidgetDestroy(view);
        view = NULL;
        if (!setup_message_list()) {
            fprintf(stderr, "Document web provider MessageList setup failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        phase = 2;
    } else if (phase == 2 && message_math_draws > 0 &&
            message_diagram_draws > 0 && message_invalidations >= 2) {
        unsigned char* pixels = malloc(600u * 400u * 4u);
        xui_rect_t message_rect = xuiMessageListGetNodeRect(message_list, 0);
        if (!pixels || message_rect.fH <= 140 ||
                proxy.surfaceReadRGBA(&proxy, target, pixels, 600 * 4) != XUI_OK ||
                xgeImageSavePNG("build/webview/xui_document_web_message.png",
                    600, 400, pixels, 600 * 4) != XGE_OK) {
            free(pixels);
            fprintf(stderr, "Document web provider MessageList readback failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        free(pixels);
        printf("XUI MessageList KaTeX/Mermaid rendered: math=%d diagram=%d "
            "invalidations=%d row-height=%.1f\n", message_math_draws,
            message_diagram_draws, message_invalidations,
            (double)message_rect.fH);
        xuiWidgetDestroy(message_list);
        message_list = NULL;
        if (!setup_dark_view() ||
                xuiDocumentWebProviderSetPalette(provider,
                    XUI_COLOR_RGBA(240, 246, 252, 255),
                    XUI_COLOR_RGBA(22, 27, 34, 255)) != XUI_OK ||
                xuiDocumentWebProviderGetStats(provider, &stats) != XUI_OK ||
                stats.iReady != 0 || stats.iCacheBytes != 0 ||
                stats.iPaletteGeneration != 2 ||
                xuiDocumentWebProviderSetPalette(provider,
                    XUI_COLOR_RGBA(240, 246, 252, 255),
                    XUI_COLOR_RGBA(22, 27, 34, 255)) != XUI_OK ||
                xuiDocumentWebProviderSetPalette(provider,
                    XUI_COLOR_RGBA(240, 246, 252, 128),
                    XUI_COLOR_RGBA(22, 27, 34, 255)) !=
                    XUI_ERROR_INVALID_ARGUMENT) {
            fprintf(stderr, "Document web provider palette reset failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        phase = 3;
    } else if (phase == 3 && stats.iReady >= 2 && stats.iDrawn >= 2) {
        size_t bright = 0;
        if (!save_dark_target("build/webview/xui_document_web_dark.png",
                    &bright)) {
            fprintf(stderr, "Document web provider dark pixels invalid: %zu\n", bright);
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI DocumentView dark palette rendered: ready=%u drawn=%u "
            "generation=%llu bright-pixels=%zu\n", stats.iReady,
            stats.iDrawn, (unsigned long long)stats.iPaletteGeneration, bright);
        if (xuiDocumentWebProviderSetPalette(provider,
                XUI_COLOR_RGBA(17, 17, 17, 255), XUI_COLOR_WHITE) != XUI_OK)
            { result = 2; xgeQuit(); return XGE_ERROR; }
        phase = 4;
    } else if (phase == 4 && stats.iQueued > 0 && stats.iReady == 0) {
        if (xuiDocumentWebProviderSetPalette(provider,
                XUI_COLOR_RGBA(240, 246, 252, 255),
                XUI_COLOR_RGBA(22, 27, 34, 255)) != XUI_OK ||
                xuiDocumentWebProviderGetStats(provider, &stats) != XUI_OK ||
                stats.iPaletteGeneration != 4 || stats.iCacheBytes != 0 ||
                stats.iReady != 0) {
            fprintf(stderr, "Document web provider in-flight palette reset failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        phase = 5;
    } else if (phase == 5 && stats.iReady >= 2 && stats.iDrawn >= 2) {
        size_t bright = 0;
        xui_document_transaction transaction = NULL;
        xui_doc_txn_desc_t transaction_desc = {0};
        if (!save_dark_target("build/webview/xui_document_web_dark_resumed.png",
                    &bright) || stats.iPaletteGeneration != 4) {
            fprintf(stderr, "Document web provider late palette result leaked\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI DocumentView palette switch during render: generation=%llu "
            "ready=%u drawn=%u bright-pixels=%zu\n",
            (unsigned long long)stats.iPaletteGeneration,
            stats.iReady, stats.iDrawn, bright);
        prior_ready = stats.iReady;
        prior_drawn = stats.iDrawn;
        transaction_desc.iSize = sizeof(transaction_desc);
        transaction_desc.iDomain = XUI_DOC_SOURCE;
        if (xuiDocumentBeginTransaction(document, &transaction_desc,
                    &transaction) != XUI_OK ||
                xuiDocumentTxnReplaceSource(transaction, 0,
                    sizeof(changed_markdown) - 1, html_markdown,
                    sizeof(html_markdown) - 1) != XUI_OK ||
                xuiDocumentTxnCommit(transaction, NULL) != XUI_OK) {
            if (transaction) xuiDocumentTxnRelease(transaction);
            fprintf(stderr, "Document web provider HTML source edit failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        xuiDocumentTxnRelease(transaction);
        phase = 6;
    } else if (phase == 6 && stats.iReady >= prior_ready + 1 &&
            stats.iDrawn >= prior_drawn + 1) {
        size_t red = 0;
        if (!save_html_target("build/webview/xui_document_web_html.png", &red)) {
            fprintf(stderr, "Document web provider HTML pixels invalid: %zu\n", red);
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI DocumentView HTML rendered: ready=%u drawn=%u "
            "red-pixels=%zu\n", stats.iReady, stats.iDrawn, red);
        xuiWidgetDestroy(view);
        view = NULL;
        message_invalidations = 0;
        if (!setup_message_list()) {
            fprintf(stderr, "Document web provider HTML MessageList setup failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        phase = 7;
    } else if (phase == 7 && message_html_draws > 0 &&
            message_invalidations > 0) {
        xui_document_transaction transaction = NULL;
        xui_doc_txn_desc_t transaction_desc = {0};
        xui_rect_t message_rect = xuiMessageListGetNodeRect(message_list, 0);
        if (message_rect.fH <= 40) {
            fprintf(stderr, "Document web provider HTML MessageList height invalid\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI MessageList HTML rendered: draws=%d invalidations=%d "
            "row-height=%.1f\n", message_html_draws,
            message_invalidations, (double)message_rect.fH);
        transaction_desc.iSize = sizeof(transaction_desc);
        transaction_desc.iDomain = XUI_DOC_SOURCE;
        if (xuiDocumentBeginTransaction(document, &transaction_desc,
                    &transaction) != XUI_OK ||
                xuiDocumentTxnReplaceSource(transaction, 0,
                    sizeof(html_markdown) - 1, invalid_math_markdown,
                    sizeof(invalid_math_markdown) - 1) != XUI_OK ||
                xuiDocumentTxnCommit(transaction, NULL) != XUI_OK) {
            if (transaction) xuiDocumentTxnRelease(transaction);
            fprintf(stderr, "Document web provider invalid math edit failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        xuiDocumentTxnRelease(transaction);
        message_math_draws = 0;
        message_invalidations = 0;
        phase = 8;
    } else if (phase == 8 && stats.iFailed >= 1 &&
            message_invalidations > 0 && message_math_draws > 0 &&
            error_heading_draws > 0 && error_detail_draws > 0 &&
            error_source_draws > 0) {
        size_t fill = 0;
        xui_rect_t message_rect = xuiMessageListGetNodeRect(message_list, 0);
        xui_document_transaction transaction = NULL;
        xui_doc_txn_desc_t transaction_desc = {0};
        if (message_rect.fH <= 120 ||
                !save_dark_error_target(
                    "build/webview/xui_document_web_error.png", &fill)) {
            fprintf(stderr, "Document web provider error card pixels invalid\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI MessageList invalid formula error shown: failed=%u "
            "draws=%d heading=%d detail=%d source=%d fill-pixels=%zu "
            "row-height=%.1f\n",
            stats.iFailed, message_math_draws, error_heading_draws,
            error_detail_draws, error_source_draws, fill,
            (double)message_rect.fH);
        prior_ready = stats.iReady;
        transaction_desc.iSize = sizeof(transaction_desc);
        transaction_desc.iDomain = XUI_DOC_SOURCE;
        if (xuiDocumentBeginTransaction(document, &transaction_desc,
                    &transaction) != XUI_OK ||
                xuiDocumentTxnReplaceSource(transaction, 0,
                    sizeof(invalid_math_markdown) - 1, recovery_math_markdown,
                    sizeof(recovery_math_markdown) - 1) != XUI_OK ||
                xuiDocumentTxnCommit(transaction, NULL) != XUI_OK) {
            if (transaction) xuiDocumentTxnRelease(transaction);
            fprintf(stderr, "Document web provider recovery edit failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        xuiDocumentTxnRelease(transaction);
        message_math_draws = 0;
        message_invalidations = 0;
        phase = 9;
    } else if (phase == 9 && stats.iReady >= prior_ready + 1 &&
            message_math_draws > 0 && message_invalidations > 0) {
        xui_document_transaction transaction = NULL;
        xui_doc_txn_desc_t transaction_desc = {0};
        printf("XUI MessageList recovered after invalid formula: ready=%u "
            "draws=%d invalidations=%d\n", stats.iReady,
            message_math_draws, message_invalidations);
        prior_failed = stats.iFailed;
        transaction_desc.iSize = sizeof(transaction_desc);
        transaction_desc.iDomain = XUI_DOC_SOURCE;
        if (xuiDocumentBeginTransaction(document, &transaction_desc,
                    &transaction) != XUI_OK ||
                xuiDocumentTxnReplaceSource(transaction, 0,
                    sizeof(recovery_math_markdown) - 1,
                    invalid_mermaid_markdown,
                    sizeof(invalid_mermaid_markdown) - 1) != XUI_OK ||
                xuiDocumentTxnCommit(transaction, NULL) != XUI_OK) {
            if (transaction) xuiDocumentTxnRelease(transaction);
            fprintf(stderr, "Document web provider invalid Mermaid edit failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        xuiDocumentTxnRelease(transaction);
        message_invalidations = 0;
        phase = 10;
    } else if (phase == 10 && stats.iFailed > prior_failed &&
            message_invalidations > 0 && mermaid_error_heading_draws > 0 &&
            mermaid_error_detail_draws > 0) {
        size_t fill = 0;
        xui_document_transaction transaction = NULL;
        xui_doc_txn_desc_t transaction_desc = {0};
        if (!save_dark_error_target(
                    "build/webview/xui_document_web_mermaid_error.png",
                    &fill)) {
            fprintf(stderr, "Document web provider Mermaid error pixels invalid\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI MessageList invalid Mermaid error shown: failed=%u "
            "heading=%d detail=%d fill-pixels=%zu\n", stats.iFailed,
            mermaid_error_heading_draws, mermaid_error_detail_draws, fill);
        prior_failed = stats.iFailed;
        transaction_desc.iSize = sizeof(transaction_desc);
        transaction_desc.iDomain = XUI_DOC_SOURCE;
        if (xuiDocumentBeginTransaction(document, &transaction_desc,
                    &transaction) != XUI_OK ||
                xuiDocumentTxnReplaceSource(transaction, 0,
                    sizeof(invalid_mermaid_markdown) - 1,
                    invalid_html_markdown,
                    sizeof(invalid_html_markdown) - 1) != XUI_OK ||
                xuiDocumentTxnCommit(transaction, NULL) != XUI_OK) {
            if (transaction) xuiDocumentTxnRelease(transaction);
            fprintf(stderr, "Document web provider invalid HTML edit failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        xuiDocumentTxnRelease(transaction);
        message_invalidations = 0;
        phase = 11;
    } else if (phase == 11 && stats.iFailed > prior_failed &&
            message_invalidations > 0 && html_error_heading_draws > 0 &&
            html_error_detail_draws > 0) {
        size_t fill = 0;
        if (!save_dark_error_target(
                    "build/webview/xui_document_web_html_error.png",
                    &fill)) {
            fprintf(stderr, "Document web provider HTML error pixels invalid\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI MessageList oversized HTML error shown: failed=%u "
            "heading=%d detail=%d fill-pixels=%zu\n", stats.iFailed,
            html_error_heading_draws, html_error_detail_draws, fill);
        message_invalidations = 0;
        if (!setup_missing_assets_provider()) {
            fprintf(stderr, "Document web provider missing assets setup failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        phase = 12;
    } else if (phase == 12 && stats.bWorkerFailed && stats.iFailed >= 1 &&
            !stats.bRestartPending && !stats.bRecovering &&
            stats.iRestartAttempts == 0 && stats.iQueued == 0 &&
            strstr(stats.sWorkerError,
                "assets cannot be mapped") &&
            message_invalidations > 0 && worker_error_heading_draws > 0 &&
            worker_error_detail_draws > 0) {
        size_t fill = 0;
        xui_document_transaction transaction = NULL;
        xui_doc_txn_desc_t transaction_desc = {0};
        if (!save_html_target("build/webview/xui_document_web_assets_error.png",
                    &fill)) {
            fprintf(stderr, "Document web provider missing assets pixels invalid\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI MessageList missing renderer assets shown: failed=%u "
            "heading=%d detail=%d pixels=%zu\n", stats.iFailed,
            worker_error_heading_draws, worker_error_detail_draws, fill);
        prior_failed = stats.iFailed;
        transaction_desc.iSize = sizeof(transaction_desc);
        transaction_desc.iDomain = XUI_DOC_SOURCE;
        if (xuiDocumentBeginTransaction(document, &transaction_desc,
                    &transaction) != XUI_OK ||
                xuiDocumentTxnReplaceSource(transaction, 0,
                    sizeof(invalid_html_markdown) - 1,
                    recovery_math_markdown,
                    sizeof(recovery_math_markdown) - 1) != XUI_OK ||
                xuiDocumentTxnCommit(transaction, NULL) != XUI_OK) {
            if (transaction) xuiDocumentTxnRelease(transaction);
            fprintf(stderr, "Document web provider future object edit failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        xuiDocumentTxnRelease(transaction);
        message_invalidations = 0;
        phase = 13;
    } else if (phase == 13 && stats.bWorkerFailed &&
            stats.iFailed > prior_failed && stats.iQueued == 0 &&
            message_math_draws > 0 && worker_future_heading_draws > 0 &&
            worker_future_detail_draws > 0) {
        size_t red = 0;
        if (!save_html_target(
                    "build/webview/xui_document_web_assets_future.png",
                    &red)) {
            fprintf(stderr, "Document web provider future error pixels invalid\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI MessageList future object after worker failure: failed=%u "
            "queued=%u heading=%d detail=%d pixels=%zu\n", stats.iFailed,
            stats.iQueued, worker_future_heading_draws,
            worker_future_detail_draws, red);
        message_invalidations = 0;
        if (xuiDocumentWebProviderSetPalette(provider,
                XUI_COLOR_RGBA(240, 246, 252, 255),
                XUI_COLOR_RGBA(22, 27, 34, 255)) != XUI_OK ||
                xuiDocumentWebProviderGetStats(provider, &stats) != XUI_OK ||
                !stats.bWorkerFailed || stats.iFailed != 0 ||
                stats.iPaletteGeneration != 2) {
            fprintf(stderr, "Document web provider failed palette reset invalid\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        phase = 14;
    } else if (phase == 14 && stats.bWorkerFailed &&
            stats.iPaletteGeneration == 2 && stats.iFailed >= 1 &&
            message_invalidations > 0 && worker_palette_heading_draws > 0 &&
            worker_palette_detail_draws > 0) {
        size_t fill = 0;
        if (!save_dark_error_target(
                    "build/webview/xui_document_web_assets_dark.png",
                    &fill)) {
            fprintf(stderr, "Document web provider failed palette pixels invalid\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI MessageList worker failure palette reset: failed=%u "
            "generation=%llu fill-pixels=%zu\n", stats.iFailed,
            (unsigned long long)stats.iPaletteGeneration, fill);
        if (!setup_stalled_assets_provider()) {
            fprintf(stderr, "Document web provider stalled assets setup failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        message_invalidations = 0;
        phase = 15;
    } else if (phase == 15 && stats.bWorkerFailed) {
        fprintf(stderr, "Document web provider stalled page became worker failure\n");
        result = 2; xgeQuit(); return XGE_ERROR;
    } else if (phase == 15 && stats.iFailed >= 1 &&
            stats.iQueued == 0 && !stats.sWorkerError[0] &&
            message_invalidations > 0 && worker_timeout_heading_draws > 0 &&
            worker_timeout_detail_draws > 0) {
        size_t red = 0;
        if (worker_timeout_updates >= 500) {
            fprintf(stderr, "Document web provider elapsed timeout used frame limit\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        if (!save_html_target("build/webview/xui_document_web_timeout.png",
                    &red)) {
            fprintf(stderr, "Document web provider timeout pixels invalid\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI MessageList stalled renderer response shown: "
            "failed=%u queued=%u heading=%d detail=%d pixels=%zu updates=%d\n",
            stats.iFailed, stats.iQueued, worker_timeout_heading_draws,
            worker_timeout_detail_draws, red, worker_timeout_updates);
        if (!replace_web_provider("res\\xui_document_web")) {
            fprintf(stderr, "Document web provider decode test setup failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        inject_decode_error = 1;
        message_invalidations = 0;
        phase = 16;
    } else if (phase == 16 && stats.bWorkerFailed) {
        fprintf(stderr, "Document web provider decode error became worker failure\n");
        result = 2; xgeQuit(); return XGE_ERROR;
    } else if (phase == 16 && stats.iFailed >= 1 &&
            stats.iQueued == 0 && decode_injections == 1 &&
            decode_releases == 1 && injected_surface == NULL &&
            message_invalidations > 0 && decode_error_heading_draws > 0 &&
            decode_error_detail_draws > 0) {
        size_t red = 0;
        xui_document_transaction transaction = NULL;
        xui_doc_txn_desc_t transaction_desc = {0};
        if (!save_html_target(
                    "build/webview/xui_document_web_decode_error.png",
                    &red)) {
            fprintf(stderr, "Document web provider decode error pixels invalid\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI MessageList screenshot decode error shown: "
            "failed=%u queued=%u injection=%d release=%d "
            "heading=%d detail=%d pixels=%zu\n",
            stats.iFailed, stats.iQueued, decode_injections,
            decode_releases, decode_error_heading_draws,
            decode_error_detail_draws, red);
        transaction_desc.iSize = sizeof(transaction_desc);
        transaction_desc.iDomain = XUI_DOC_SOURCE;
        if (xuiDocumentBeginTransaction(document, &transaction_desc,
                    &transaction) != XUI_OK ||
                xuiDocumentTxnReplaceSource(transaction, 0,
                    sizeof(recovery_math_markdown) - 1,
                    decode_recovery_markdown,
                    sizeof(decode_recovery_markdown) - 1) != XUI_OK ||
                xuiDocumentTxnCommit(transaction, NULL) != XUI_OK) {
            if (transaction) xuiDocumentTxnRelease(transaction);
            fprintf(stderr, "Document web provider decode recovery edit failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        xuiDocumentTxnRelease(transaction);
        message_math_draws = 0;
        message_invalidations = 0;
        phase = 17;
    } else if (phase == 17 && stats.iReady >= 1 &&
            stats.iDrawn >= 1 && stats.iFailed == 1 &&
            !stats.bWorkerFailed && message_math_draws > 0 &&
            message_invalidations > 0 && decode_injections == 1 &&
            decode_releases == 1 && injected_surface == NULL) {
        xui_document_transaction transaction = NULL;
        xui_doc_txn_desc_t transaction_desc = {0};
        printf("XUI MessageList recovered after screenshot decode error: "
            "ready=%u drawn=%u failed=%u math-draws=%d\n",
            stats.iReady, stats.iDrawn, stats.iFailed,
            message_math_draws);
        transaction_desc.iSize = sizeof(transaction_desc);
        transaction_desc.iDomain = XUI_DOC_SOURCE;
        if (xuiDocumentBeginTransaction(document, &transaction_desc,
                    &transaction) != XUI_OK ||
                xuiDocumentTxnReplaceSource(transaction, 0,
                    sizeof(decode_recovery_markdown) - 1,
                    crop_recovery_markdown,
                    sizeof(crop_recovery_markdown) - 1) != XUI_OK ||
                xuiDocumentTxnCommit(transaction, NULL) != XUI_OK) {
            if (transaction) xuiDocumentTxnRelease(transaction);
            fprintf(stderr, "Document web provider crop test edit failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        xuiDocumentTxnRelease(transaction);
        inject_crop_error = 1;
        message_invalidations = 0;
        phase = 18;
    } else if (phase == 18 && stats.bWorkerFailed) {
        fprintf(stderr, "Document web provider crop error became worker failure\n");
        result = 2; xgeQuit(); return XGE_ERROR;
    } else if (phase == 18 && stats.iFailed >= 2 &&
            stats.iQueued == 0 && crop_injections == 1 &&
            crop_releases == 1 && injected_crop_surface == NULL &&
            message_invalidations > 0 && crop_error_heading_draws > 0 &&
            crop_error_detail_draws > 0) {
        size_t red = 0;
        xui_document_transaction transaction = NULL;
        xui_doc_txn_desc_t transaction_desc = {0};
        if (!save_html_target(
                    "build/webview/xui_document_web_crop_error.png",
                    &red)) {
            fprintf(stderr, "Document web provider crop error pixels invalid\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI MessageList screenshot crop error shown: "
            "failed=%u queued=%u injection=%d release=%d "
            "heading=%d detail=%d pixels=%zu\n",
            stats.iFailed, stats.iQueued, crop_injections,
            crop_releases, crop_error_heading_draws,
            crop_error_detail_draws, red);
        transaction_desc.iSize = sizeof(transaction_desc);
        transaction_desc.iDomain = XUI_DOC_SOURCE;
        if (xuiDocumentBeginTransaction(document, &transaction_desc,
                    &transaction) != XUI_OK ||
                xuiDocumentTxnReplaceSource(transaction, 0,
                    sizeof(crop_recovery_markdown) - 1,
                    recovery_math_markdown,
                    sizeof(recovery_math_markdown) - 1) != XUI_OK ||
                xuiDocumentTxnCommit(transaction, NULL) != XUI_OK) {
            if (transaction) xuiDocumentTxnRelease(transaction);
            fprintf(stderr, "Document web provider crop recovery edit failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        xuiDocumentTxnRelease(transaction);
        message_math_draws = 0;
        message_invalidations = 0;
        phase = 19;
    } else if (phase == 19 && stats.iReady >= 2 &&
            stats.iDrawn >= 2 && stats.iFailed == 2 &&
            !stats.bWorkerFailed && message_math_draws > 0 &&
            message_invalidations > 0 && crop_injections == 1 &&
            crop_releases == 1 && injected_crop_surface == NULL) {
        printf("XUI MessageList recovered after screenshot crop error: "
            "ready=%u drawn=%u failed=%u math-draws=%d\n",
            stats.iReady, stats.iDrawn, stats.iFailed,
            message_math_draws);
        if (!replace_web_provider("res\\xui_document_web")) {
            fprintf(stderr, "Document web provider process failure setup failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        message_invalidations = 0;
        phase = 20;
    } else if (phase == 20 && stats.bWorkerFailed) {
        fprintf(stderr, "Document web provider failed before process injection\n");
        result = 2; xgeQuit(); return XGE_ERROR;
    } else if (phase == 20 && stats.iQueued >= 1) {
        xui_widget worker = provider_worker_widget();
        if (worker &&
                xuiWebViewGetState(worker) == XUI_WEBVIEW_INITIALIZING)
            return XGE_OK;
        if (!terminate_provider_browser(&killed_browser_pid)) {
            fprintf(stderr, "Document provider isolated browser termination failed\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        last_killed_browser_pid = killed_browser_pid;
        message_invalidations = 0;
        phase = 21;
    } else if (phase == 21 && stats.bWorkerFailed &&
            stats.iFailed >= 1 && stats.iQueued == 0 &&
            stats.bRestartPending && !stats.bRecovering &&
            stats.iRestartAttempts == 0 &&
            strstr(stats.sWorkerError, "browser failed") &&
            message_invalidations > 0 && process_error_heading_draws > 0 &&
            process_error_detail_draws > 0) {
        size_t red = 0;
        if (!save_html_target(
                    "build/webview/xui_document_web_process_failure.png",
                    &red)) {
            fprintf(stderr, "Document provider process error pixels invalid\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI MessageList browser process failure shown: "
            "failed=%u queued=%u heading=%d detail=%d pixels=%zu\n",
            stats.iFailed, stats.iQueued, process_error_heading_draws,
            process_error_detail_draws, red);
        message_math_draws = 0;
        message_invalidations = 0;
        phase = 22;
    } else if (phase == 22 && stats.iReady >= 1 && stats.iDrawn >= 1 &&
            stats.iFailed == 0 && !stats.bWorkerFailed &&
            !stats.bRestartPending && !stats.bRecovering &&
            stats.iRestartAttempts == 1 &&
            message_math_draws > 0 &&
            message_invalidations > 0) {
        uint32_t recovered_pid = 0;
        if (!provider_browser_pid(&recovered_pid) ||
                recovered_pid == killed_browser_pid) {
            fprintf(stderr, "Document provider restart process identity invalid\n");
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI MessageList browser process recovery rendered: "
            "ready=%u drawn=%u math-draws=%d old-pid=%lu new-pid=%lu\n",
            stats.iReady, stats.iDrawn, message_math_draws,
            (unsigned long)killed_browser_pid,
            (unsigned long)recovered_pid);
        phase = 23;
    } else if (phase == 23 || phase == 26 || phase == 29) {
        if (!terminate_provider_browser(&last_killed_browser_pid)) {
            fprintf(stderr, "Document provider repeated browser termination failed: phase=%d\n",
                phase);
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        phase++;
    } else if ((phase == 24 || phase == 27) &&
            stats.bWorkerFailed && stats.bRestartPending &&
            !stats.bRecovering && stats.iReady >= 1 && stats.iDrawn >= 1 &&
            stats.iRestartAttempts == (uint32_t)(phase == 24 ? 1 : 2)) {
        phase++;
    } else if ((phase == 25 || phase == 28) &&
            !stats.bWorkerFailed && !stats.bRestartPending &&
            !stats.bRecovering && stats.iReady >= 1 && stats.iDrawn >= 1 &&
            stats.iRestartAttempts == (uint32_t)(phase == 25 ? 2 : 3)) {
        uint32_t recovered_pid = 0;
        if (!provider_browser_pid(&recovered_pid) ||
                recovered_pid == last_killed_browser_pid) {
            fprintf(stderr, "Document provider repeated restart PID invalid: phase=%d\n",
                phase);
            result = 2; xgeQuit(); return XGE_ERROR;
        }
        printf("XUI MessageList browser process restart #%u: "
            "ready=%u drawn=%u old-pid=%lu new-pid=%lu\n",
            stats.iRestartAttempts, stats.iReady, stats.iDrawn,
            (unsigned long)last_killed_browser_pid,
            (unsigned long)recovered_pid);
        phase++;
    } else if (phase == 30 && stats.bWorkerFailed &&
            !stats.bRestartPending && !stats.bRecovering &&
            stats.iRestartAttempts == 3 && stats.iReady >= 1 &&
            stats.iDrawn >= 1) {
        if (++final_failure_updates >= 30) {
            printf("XUI MessageList browser restart budget exhausted: "
                "attempts=%u ready=%u drawn=%u\n",
                stats.iRestartAttempts, stats.iReady, stats.iDrawn);
            result = 0;
            xgeQuit();
        }
    }
    return XGE_OK;
}

int main(void)
{
    xge_desc_t desc = {0};
    desc.iWidth = 600; desc.iHeight = 400;
    desc.sTitle = "XUI Document browser provider integration";
    desc.iRunMode = XGE_RUN_GAME_LOOP; desc.iTargetFPS = 60;
    if (xgeInit(&desc) != XGE_OK) return 2;
    xgeRun(frame, NULL);
    if (provider) xuiDocumentWebProviderRelease(provider);
    if (context) xuiDestroy(context);
    if (document) xuiDocumentRelease(document);
    if (target && proxy.surfaceDestroy) proxy.surfaceDestroy(&proxy, target);
    if (font && proxy.fontDestroy) proxy.fontDestroy(&proxy, font);
    free(first_pixels);
    xgeUnit();
    return result;
}
