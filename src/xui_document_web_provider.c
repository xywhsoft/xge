#include "../xui_config.h"
#if XUI_ENABLE_WEBVIEW
/* Optional, application-owned static math/Mermaid/HTML renderer. The Document
 * remains authoritative; the one browser worker only produces cached pixels. */
#include "../xui_document_ui.h"
#include "xui_webview_internal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOC_WEB_CACHE_ENTRIES 64
#define DOC_WEB_MAX_SOURCE 262144u
#define DOC_WEB_MAX_PNG (16u * 1024u * 1024u)
#define DOC_WEB_CACHE_BYTES (64u * 1024u * 1024u)
#define DOC_WEB_DEFAULT_TIMEOUT_MS 10000u
#define DOC_WEB_MAX_RESTARTS 3u
#define DOC_WEB_RESTART_DELAY_US UINT64_C(1000000)
#define DOC_WEB_HEALTHY_RESET_US UINT64_C(60000000)
#define DOC_WEB_ERROR_LINES 32
enum { DOC_WEB_QUEUED = 1, DOC_WEB_RENDERING, DOC_WEB_READY, DOC_WEB_FAILED };
enum { DOC_WEB_IDLE = 0, DOC_WEB_WAIT_RENDER, DOC_WEB_WAIT_PAINT, DOC_WEB_WAIT_CAPTURE };

typedef struct doc_web_entry {
    uint64_t document_id, revision, node_id, used, request_id, palette_generation;
    uint32_t kind, state;
    int drawn, display, worker_failure;
    float available, zoom, baseline;
    int width, height;
    char* source;
    size_t source_bytes;
    char error_text[256];
    uint16_t error_line_start[DOC_WEB_ERROR_LINES];
    uint16_t error_line_bytes[DOC_WEB_ERROR_LINES];
    uint8_t error_line_count;
    xui_surface surface;
    size_t surface_bytes;
} doc_web_entry;

struct xui_doc_web_provider_t {
    xui_context context;
    xui_widget parent, worker;
    xui_proxy_t proxy;
    char* assets_folder;
    char* user_data_folder;
    xui_doc_web_invalidate_proc on_invalidate;
    void* user;
    doc_web_entry entries[DOC_WEB_CACHE_ENTRIES];
    size_t entry_count, cache_bytes;
    uint64_t clock, next_request_id;
    uint64_t draw_calls;
    uint64_t palette_generation;
    uint32_t foreground, background;
    doc_web_entry* active;
    xui_web_request capture;
    int stage, stage_frames, page_ready, failed, recovering, restarting;
    uint32_t timeout_ms;
    uint32_t restart_attempts;
    uint64_t startup_started, stage_started, restart_at, healthy_since;
    char worker_error[128];
};

static int doc_web_timed_out(xui_doc_web_provider provider,
    uint64_t started)
{
    /* xrtClock counts microseconds; the public option is milliseconds. */
    return xrtClock() - started >= (uint64_t)provider->timeout_ms * 1000u;
}

static xstrview doc_web_view(const char* text, size_t bytes)
{
    xstrview view = {text, bytes}; return view;
}

static const xvalue* doc_web_field(const xvalue* value, const char* key)
{
    return xrtValueObjectGet(value, doc_web_view(key, strlen(key)));
}

static int doc_web_integer(const xvalue* value, int* out)
{
    int64 number;
    if (!xrtValueGetInt(value, &number) || number < 0 || number > 4096) return 0;
    *out = (int)number;
    return 1;
}

static int doc_web_u64(const xvalue* value, uint64_t* out)
{
    uint64 number;
    int64 integer;
    if (xrtValueGetUInt(value, &number)) { *out = number; return 1; }
    if (!xrtValueGetInt(value, &integer) || integer < 0) return 0;
    *out = (uint64_t)integer;
    return 1;
}

static int doc_web_number(const xvalue* value, float* out)
{
    double number;
    int64 integer;
    if (!xrtValueGetFloat(value, &number)) {
        if (!xrtValueGetInt(value, &integer)) return 0;
        number = (double)integer;
    }
    if (!isfinite(number) || number < 0 || number > 4096) return 0;
    *out = (float)number;
    return 1;
}

static int doc_web_string_equals(const xvalue* value, const char* literal)
{
    xstrview text;
    size_t bytes = strlen(literal);
    return xrtValueGetString(value, &text) && text.Size == bytes &&
        memcmp(text.Data, literal, bytes) == 0;
}

static void doc_web_set_error(doc_web_entry* entry, const xvalue* value)
{
    xstrview message;
    size_t i, bytes;
    if (!entry || !xrtValueGetString(value, &message)) return;
    bytes = message.Size < sizeof(entry->error_text) - 1 ?
        message.Size : sizeof(entry->error_text) - 1;
    if (bytes < message.Size)
        while (bytes && ((unsigned char)message.Data[bytes] & 0xc0u) == 0x80u)
            bytes--;
    for (i = 0; i < bytes; i++) {
        unsigned char ch = (unsigned char)message.Data[i];
        entry->error_text[i] = ch < 32 || ch == 127 ? ' ' : (char)ch;
    }
    entry->error_text[bytes] = 0;
}

static void doc_web_entry_clear(xui_doc_web_provider provider, doc_web_entry* entry)
{
    if (entry->surface && provider->proxy.surfaceDestroy)
        provider->proxy.surfaceDestroy(&provider->proxy, entry->surface);
    if (provider->cache_bytes >= entry->surface_bytes)
        provider->cache_bytes -= entry->surface_bytes;
    free(entry->source);
    memset(entry, 0, sizeof(*entry));
}

static void doc_web_prepare_error_card(xui_doc_web_provider provider,
    doc_web_entry* entry)
{
    xui_font font = xuiGetDefaultFont(provider->context);
    size_t bytes = strlen(entry->error_text), start = 0;
    float max_width;
    entry->width = (int)fminf(4096.0f, fmaxf(1.0f,
        fminf(entry->available, fmaxf(240.0f, 480.0f * entry->zoom))));
    max_width = fmaxf(1.0f, (float)entry->width - 16.0f);
    while (start < bytes && entry->error_line_count < DOC_WEB_ERROR_LINES) {
        size_t cursor = start, fit = start, space = start, end;
        while (cursor < bytes) {
            char candidate[sizeof(entry->error_text)];
            xui_vec2_t measured = {0};
            size_t next = cursor + 1;
            while (next < bytes &&
                    ((unsigned char)entry->error_text[next] & 0xc0u) == 0x80u)
                next++;
            memcpy(candidate, entry->error_text + start, next - start);
            candidate[next - start] = 0;
            if (font && provider->proxy.textMeasure &&
                    provider->proxy.textMeasure(&provider->proxy, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=candidate, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT}, &measured) == XUI_OK &&
                    measured.fX > max_width && fit > start) break;
            if (entry->error_text[cursor] == ' ' && cursor > start)
                space = cursor;
            fit = next;
            cursor = next;
        }
        end = fit;
        if (fit < bytes && space > start && space < fit) end = space;
        entry->error_line_start[entry->error_line_count] = (uint16_t)start;
        entry->error_line_bytes[entry->error_line_count] =
            (uint16_t)(end - start);
        entry->error_line_count++;
        start = end;
        while (start < bytes && entry->error_text[start] == ' ') start++;
    }
    if (!entry->error_line_count) {
        entry->error_line_start[0] = 0;
        entry->error_line_bytes[0] = 0;
        entry->error_line_count = 1;
    }
    entry->height = 28 + 20 * entry->error_line_count;
    if (entry->height < 56) entry->height = 56;
    entry->baseline = (float)entry->height;
}

static void doc_web_active_finish(xui_doc_web_provider provider, int success)
{
    doc_web_entry* entry = provider->active;
    if (provider->capture) {
        xuiWebRequestRelease(provider->capture);
        provider->capture = NULL;
    }
    if (entry) {
        entry->state = success ? DOC_WEB_READY : DOC_WEB_FAILED;
        if (!success) {
            if (!entry->error_text[0])
                strcpy(entry->error_text, "Rendering failed");
            doc_web_prepare_error_card(provider, entry);
        }
    }
    provider->active = NULL;
    provider->stage = DOC_WEB_IDLE;
    provider->stage_frames = 0;
    if (provider->on_invalidate) provider->on_invalidate(provider->user);
}

static void doc_web_active_fail(xui_doc_web_provider provider,
    const char* reason)
{
    if (provider->active)
        (void)snprintf(provider->active->error_text,
            sizeof(provider->active->error_text), "%s", reason);
    doc_web_active_finish(provider, 0);
}

static void doc_web_fail_provider(xui_doc_web_provider provider,
    const char* reason, int retryable)
{
    size_t i;
    uint64_t now = xrtClock();
    if (provider->failed) return;
    if (retryable && provider->healthy_since &&
            now - provider->healthy_since >= DOC_WEB_HEALTHY_RESET_US)
        provider->restart_attempts = 0;
    provider->failed = 1;
    provider->recovering = 0;
    provider->page_ready = 0;
    provider->healthy_since = 0;
    provider->restart_at = retryable &&
        provider->restart_attempts < DOC_WEB_MAX_RESTARTS ?
        now + (DOC_WEB_RESTART_DELAY_US << provider->restart_attempts) : 0;
    (void)snprintf(provider->worker_error, sizeof(provider->worker_error),
        "%s", reason);
    if (provider->capture) {
        (void)xuiWebRequestCancel(provider->capture);
        xuiWebRequestRelease(provider->capture);
        provider->capture = NULL;
    }
    provider->active = NULL;
    provider->stage = DOC_WEB_IDLE;
    provider->stage_frames = 0;
    for (i = 0; i < provider->entry_count; i++) {
        doc_web_entry* entry = &provider->entries[i];
        if (entry->state != DOC_WEB_QUEUED &&
                entry->state != DOC_WEB_RENDERING) continue;
        entry->state = DOC_WEB_FAILED;
        entry->worker_failure = 1;
        strcpy(entry->error_text, provider->worker_error);
        doc_web_prepare_error_card(provider, entry);
    }
    if (provider->on_invalidate) provider->on_invalidate(provider->user);
}

static void doc_web_message(xui_widget widget, const char* source,
    const char* json, size_t bytes, void* user)
{
    xui_doc_web_provider provider = user;
    xvalue* value;
    int id, width, height;
    uint64_t palette_generation;
    float baseline = 0;
    (void)source;
    if (!provider || widget != provider->worker || provider->failed ||
            bytes > 1024u * 1024u) return;
    value = xrtJsonParse(doc_web_view(json, bytes));
    if (!value) return;
    if (doc_web_string_equals(doc_web_field(value, "kind"), "ready")) {
        bool math = false, diagram = false, html = false;
        (void)xrtValueGetBool(doc_web_field(value, "katex"), &math);
        (void)xrtValueGetBool(doc_web_field(value, "mermaid"), &diagram);
        (void)xrtValueGetBool(doc_web_field(value, "dompurify"), &html);
        provider->page_ready = math && diagram && html;
        if (!provider->page_ready)
            doc_web_fail_provider(provider,
                "Document renderer assets are incomplete", 0);
        else {
            provider->healthy_since = xrtClock();
        }
        if (provider->page_ready && provider->recovering) {
            size_t i;
            provider->recovering = 0;
            provider->worker_error[0] = 0;
            for (i = 0; i < provider->entry_count; i++) {
                doc_web_entry* entry = &provider->entries[i];
                if (entry->state != DOC_WEB_FAILED ||
                        !entry->worker_failure) continue;
                entry->state = DOC_WEB_QUEUED;
                entry->worker_failure = 0;
                entry->error_text[0] = 0;
                entry->error_line_count = 0;
                entry->width = entry->height = 0;
                entry->baseline = entry->display ? -1.0f : 0.0f;
            }
            if (provider->on_invalidate)
                provider->on_invalidate(provider->user);
        }
    } else if (provider->active && provider->stage == DOC_WEB_WAIT_RENDER &&
            doc_web_integer(doc_web_field(value, "id"), &id) &&
            (uint64_t)id == provider->active->request_id &&
            doc_web_u64(doc_web_field(value, "generation"),
                &palette_generation) &&
            palette_generation == provider->active->palette_generation) {
        if (doc_web_string_equals(doc_web_field(value, "kind"), "error")) {
            doc_web_set_error(provider->active, doc_web_field(value, "error"));
            doc_web_active_finish(provider, 0);
        } else if (doc_web_string_equals(doc_web_field(value, "kind"), "rendered") &&
                doc_web_integer(doc_web_field(value, "width"), &width) &&
                doc_web_integer(doc_web_field(value, "height"), &height) &&
                doc_web_number(doc_web_field(value, "baseline"), &baseline) &&
                width > 0 && height > 0 &&
                (uint64_t)width * (uint64_t)height <= 16000000u) {
            provider->active->width = width;
            provider->active->height = height;
            provider->active->baseline = baseline;
            provider->stage = DOC_WEB_WAIT_PAINT;
            provider->stage_frames = 0;
            provider->stage_started = xrtClock();
        } else {
            doc_web_active_finish(provider, 0);
        }
    }
    xrtValueRelease(value);
}

static void doc_web_event(xui_widget widget, int event, int32_t error, void* user)
{
    xui_doc_web_provider provider = user;
    (void)error;
    if (!provider || widget != provider->worker ||
            provider->restarting || provider->failed) return;
    if (event == XUI_WEBVIEW_EVENT_READY) {
        if (xuiWebViewMapLocalFolder(widget, "xui-document.invalid",
                    provider->assets_folder,
                    XUI_WEB_LOCAL_ACCESS_SAME_ORIGIN) != XUI_OK)
            doc_web_fail_provider(provider,
                "Document renderer assets cannot be mapped", 0);
        else if (xuiWebViewSetMessageHandler(widget,
                    "https://xui-document.invalid", doc_web_message,
                    provider) != XUI_OK)
            doc_web_fail_provider(provider,
                "Document renderer message channel is unavailable", 0);
        else if (xuiWebViewNavigate(widget,
                    "https://xui-document.invalid/renderer.html") != XUI_OK)
            doc_web_fail_provider(provider,
                "Document renderer navigation failed", 0);
    } else if (event == XUI_WEBVIEW_EVENT_FAILED ||
            event == XUI_WEBVIEW_EVENT_NAVIGATION_FAILED ||
            event == XUI_WEBVIEW_EVENT_CLOSED) {
        doc_web_fail_provider(provider,
            event == XUI_WEBVIEW_EVENT_FAILED ?
                "Document renderer browser failed" :
            event == XUI_WEBVIEW_EVENT_NAVIGATION_FAILED ?
                "Document renderer page failed to load" :
                "Document renderer browser closed",
            event != XUI_WEBVIEW_EVENT_NAVIGATION_FAILED);
    }
}

static void doc_web_position_worker(xui_doc_web_provider provider, int width, int height)
{
    xui_rect_t parent = xuiWidgetGetRect(provider->parent);
    xui_rect_t worker = {parent.fW > 0 ? parent.fW - 1 : 0,
        parent.fH > 0 ? parent.fH - 1 : 0, (float)width, (float)height};
    (void)xuiWidgetSetRect(provider->worker, worker);
}

static doc_web_entry* doc_web_find(xui_doc_web_provider provider,
    uint64_t document_id, uint64_t revision, uint64_t node_id,
    float available, float zoom)
{
    size_t i;
    for (i = 0; i < provider->entry_count; i++) {
        doc_web_entry* entry = &provider->entries[i];
        if (entry->state && entry->document_id == document_id &&
                entry->revision == revision && entry->node_id == node_id &&
                entry->palette_generation == provider->palette_generation &&
                entry->available == available && entry->zoom == zoom)
            return entry;
    }
    return NULL;
}

static doc_web_entry* doc_web_allocate(xui_doc_web_provider provider)
{
    doc_web_entry* oldest = NULL;
    size_t i;
    if (provider->entry_count < DOC_WEB_CACHE_ENTRIES)
        return &provider->entries[provider->entry_count++];
    for (i = 0; i < provider->entry_count; i++) {
        doc_web_entry* entry = &provider->entries[i];
        if (entry == provider->active || entry->state == DOC_WEB_QUEUED ||
                entry->state == DOC_WEB_RENDERING) continue;
        if (!oldest || entry->used < oldest->used) oldest = entry;
    }
    if (oldest) doc_web_entry_clear(provider, oldest);
    return oldest;
}

static doc_web_entry* doc_web_next(xui_doc_web_provider provider)
{
    doc_web_entry* oldest = NULL;
    size_t i;
    for (i = 0; i < provider->entry_count; i++) {
        doc_web_entry* entry = &provider->entries[i];
        if (entry->state == DOC_WEB_QUEUED &&
                (!oldest || entry->used < oldest->used)) oldest = entry;
    }
    return oldest;
}

static int doc_web_reserve_surface(xui_doc_web_provider provider, size_t bytes)
{
    size_t i;
    if (bytes > DOC_WEB_CACHE_BYTES) return 0;
    while (provider->cache_bytes > DOC_WEB_CACHE_BYTES - bytes) {
        doc_web_entry* oldest = NULL;
        for (i = 0; i < provider->entry_count; i++) {
            doc_web_entry* entry = &provider->entries[i];
            if (entry == provider->active || !entry->surface) continue;
            if (!oldest || entry->used < oldest->used) oldest = entry;
        }
        if (!oldest) return 0;
        doc_web_entry_clear(provider, oldest);
    }
    return 1;
}

/* CapturePreview returns the whole browser viewport. Keep its dimensions
 * stable for layout, then cache only the measured top-left content region. */
static int doc_web_crop_surface(xui_doc_web_provider provider,
    xui_surface* surface, xui_surface_desc_t* desc, int width, int height)
{
    xui_surface cropped = NULL;
    xui_surface_desc_t crop_desc = {0};
    xui_rect_t bounds;
    if (desc->iWidth == width && desc->iHeight == height) return 1;
    if (!provider->proxy.surfaceCreate || !provider->proxy.surfaceDrawTo) return 0;
    crop_desc.iKind = XUI_SURFACE_KIND_TEXTURE;
    crop_desc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
    crop_desc.iWidth = width;
    crop_desc.iHeight = height;
    crop_desc.iFlags = XUI_SURFACE_USAGE_TARGET | XUI_SURFACE_ALPHA_PREMULTIPLIED;
    if (provider->proxy.surfaceCreate(&provider->proxy, &cropped, &crop_desc) != XUI_OK ||
            !cropped) {
        if (cropped) provider->proxy.surfaceDestroy(&provider->proxy, cropped);
        return 0;
    }
    bounds = (xui_rect_t){0, 0, (float)width, (float)height};
    if (provider->proxy.surfaceDrawTo(&provider->proxy, cropped, *surface,
                bounds, bounds, XUI_COLOR_WHITE, 0) != XUI_OK) {
        provider->proxy.surfaceDestroy(&provider->proxy, cropped);
        return 0;
    }
    provider->proxy.surfaceDestroy(&provider->proxy, *surface);
    *surface = cropped;
    *desc = crop_desc;
    return 1;
}

static int doc_web_send(xui_doc_web_provider provider, doc_web_entry* entry)
{
    xjsonwriteconfig config;
    xjsonwriter* writer;
    char* json = NULL;
    char background[8], foreground[8];
    size_t bytes = 0;
    int okay, result;
    xrtJsonWriteConfigInit(&config);
    config.MaxOutputBytes = 1024u * 1024u;
    writer = xrtJsonWriterCreate(&config);
    if (!writer) return XUI_ERROR_OUT_OF_MEMORY;
    (void)snprintf(background, sizeof(background), "#%02X%02X%02X",
        (provider->background >> 24) & 255u,
        (provider->background >> 16) & 255u,
        (provider->background >> 8) & 255u);
    (void)snprintf(foreground, sizeof(foreground), "#%02X%02X%02X",
        (provider->foreground >> 24) & 255u,
        (provider->foreground >> 16) & 255u,
        (provider->foreground >> 8) & 255u);
#define NAME(s) xrtJsonWriterName(writer, doc_web_view((s), sizeof(s) - 1))
#define STRING(s) xrtJsonWriterString(writer, doc_web_view((s), sizeof(s) - 1))
    entry->request_id = ++provider->next_request_id;
    if (entry->request_id > 4096) provider->next_request_id = entry->request_id = 1;
    okay = xrtJsonWriterObject(writer) && NAME("kind") && STRING("render") &&
        NAME("id") && xrtJsonWriterUInt(writer, entry->request_id) &&
        NAME("generation") && xrtJsonWriterUInt(writer, entry->palette_generation) &&
        NAME("type") && xrtJsonWriterString(writer,
            doc_web_view(entry->kind == XUI_DOC_MATH ? "math" :
                entry->kind == XUI_DOC_DIAGRAM ? "diagram" : "html",
                entry->kind == XUI_DOC_MATH ? 4 :
                entry->kind == XUI_DOC_DIAGRAM ? 7 : 4)) &&
        NAME("source") && xrtJsonWriterString(writer,
            doc_web_view(entry->source, entry->source_bytes)) &&
        NAME("display") && xrtJsonWriterBool(writer,
            entry->display != 0) &&
        NAME("available") && xrtJsonWriterFloat(writer, entry->available) &&
        NAME("background") && xrtJsonWriterString(writer,
            doc_web_view(background, 7)) &&
        NAME("foreground") && xrtJsonWriterString(writer,
            doc_web_view(foreground, 7)) &&
        NAME("fontSize") && xrtJsonWriterFloat(writer, 16.0 * entry->zoom) &&
        xrtJsonWriterEnd(writer) && xrtJsonWriterFinish(writer);
#undef NAME
#undef STRING
    if (okay) json = xrtJsonWriterTake(writer, &bytes);
    xrtJsonWriterFree(writer);
    if (!json) return XUI_ERROR_OUT_OF_MEMORY;
    result = xuiWebViewPostMessageJson(provider->worker, json, bytes);
    xrtFree(json);
    return result;
}

static int doc_web_start_worker(xui_doc_web_provider provider)
{
    xui_webview_desc_t desc = {0};
    int result;
    desc.iSize = sizeof(desc);
    desc.sUserDataFolder = provider->user_data_folder;
    desc.onEvent = doc_web_event;
    desc.pUser = provider;
    result = xuiWebViewCreate(provider->context, &provider->worker, &desc);
    if (result == XUI_OK)
        result = xuiWidgetAddChild(provider->parent, provider->worker);
    if (result == XUI_OK)
        result = xuiWidgetSetEnabled(provider->worker, 0);
    if (result == XUI_OK) doc_web_position_worker(provider, 512, 350);
    if (result == XUI_OK)
        result = xuiWebViewInitializeAsync(provider->worker);
    if (result != XUI_OK && provider->worker) {
        provider->restarting = 1;
        (void)xuiWebViewClose(provider->worker);
        xuiWidgetDestroy(provider->worker);
        provider->worker = NULL;
        provider->restarting = 0;
    }
    return result;
}

static void doc_web_restart_worker(xui_doc_web_provider provider)
{
    int result;
    provider->restarting = 1;
    if (provider->worker) {
        (void)xuiWebViewClose(provider->worker);
        xuiWidgetDestroy(provider->worker);
        provider->worker = NULL;
    }
    provider->restarting = 0;
    provider->failed = 0;
    provider->recovering = 1;
    provider->restart_at = 0;
    provider->restart_attempts++;
    provider->startup_started = xrtClock();
    result = doc_web_start_worker(provider);
    if (result != XUI_OK && !provider->failed)
        doc_web_fail_provider(provider,
            "Document renderer restart failed", 1);
}

XUI_API int xuiDocumentWebProviderCreate(const xui_doc_web_provider_desc_t* desc,
    xui_doc_web_provider* out)
{
    xui_doc_web_provider provider;
    int result;
    if (out) *out = NULL;
    if (!out || !desc || desc->iSize < sizeof(*desc) || !desc->pContext ||
            !desc->pParent || !desc->sAssetsFolder ||
            xuiWidgetGetContext(desc->pParent) != desc->pContext)
        return XUI_ERROR_INVALID_ARGUMENT;
    provider = calloc(1, sizeof(*provider));
    if (!provider) return XUI_ERROR_OUT_OF_MEMORY;
    provider->context = desc->pContext;
    provider->parent = desc->pParent;
    provider->on_invalidate = desc->onInvalidate;
    provider->user = desc->pUser;
    provider->foreground = XUI_COLOR_RGBA(17, 17, 17, 255);
    provider->background = XUI_COLOR_WHITE;
    provider->palette_generation = 1;
    provider->timeout_ms = desc->iTimeoutMs ? desc->iTimeoutMs :
        DOC_WEB_DEFAULT_TIMEOUT_MS;
    provider->startup_started = xrtClock();
    provider->assets_folder = malloc(strlen(desc->sAssetsFolder) + 1);
    if (provider->assets_folder) strcpy(provider->assets_folder, desc->sAssetsFolder);
    if (desc->sUserDataFolder) {
        provider->user_data_folder = malloc(strlen(desc->sUserDataFolder) + 1);
        if (provider->user_data_folder)
            strcpy(provider->user_data_folder, desc->sUserDataFolder);
    }
    if (!provider->assets_folder ||
            (desc->sUserDataFolder && !provider->user_data_folder)) {
        free(provider->user_data_folder);
        free(provider->assets_folder);
        free(provider);
        return XUI_ERROR_OUT_OF_MEMORY;
    }
    if (xuiGetProxy(provider->context, &provider->proxy) != XUI_OK ||
            !provider->proxy.surfaceLoadMemory || !provider->proxy.surfaceDestroy ||
            !provider->proxy.drawSurface) {
        free(provider->user_data_folder);
        free(provider->assets_folder); free(provider);
        return XUI_ERROR_NOT_INITIALIZED;
    }
    result = doc_web_start_worker(provider);
    if (result != XUI_OK) {
        free(provider->user_data_folder);
        free(provider->assets_folder); free(provider);
        return result;
    }
    *out = provider;
    return XUI_OK;
}

XUI_API void xuiDocumentWebProviderRelease(xui_doc_web_provider provider)
{
    size_t i;
    if (!provider) return;
    provider->on_invalidate = NULL;
    if (provider->worker) {
        (void)xuiWebViewClose(provider->worker);
        xuiWidgetDestroy(provider->worker);
    }
    if (provider->capture) xuiWebRequestRelease(provider->capture);
    for (i = 0; i < provider->entry_count; i++)
        doc_web_entry_clear(provider, &provider->entries[i]);
    free(provider->user_data_folder);
    free(provider->assets_folder);
    free(provider);
}

XUI_API int xuiDocumentWebProviderSetPalette(xui_doc_web_provider provider,
    uint32_t foreground, uint32_t background)
{
    size_t i;
    if (!provider || (foreground & 255u) != 255u ||
            (background & 255u) != 255u) return XUI_ERROR_INVALID_ARGUMENT;
    if (provider->foreground == foreground &&
            provider->background == background) return XUI_OK;
    if (provider->capture) {
        (void)xuiWebRequestCancel(provider->capture);
        xuiWebRequestRelease(provider->capture);
        provider->capture = NULL;
    }
    provider->active = NULL;
    provider->stage = DOC_WEB_IDLE;
    provider->stage_frames = 0;
    for (i = 0; i < provider->entry_count; i++)
        doc_web_entry_clear(provider, &provider->entries[i]);
    provider->entry_count = 0;
    provider->foreground = foreground;
    provider->background = background;
    provider->palette_generation++;
    if (!provider->palette_generation) provider->palette_generation = 1;
    if (provider->on_invalidate) provider->on_invalidate(provider->user);
    return XUI_OK;
}

XUI_API int xuiDocumentWebProviderUpdate(xui_doc_web_provider provider)
{
    int state, result;
    const void* png;
    size_t bytes, surface_bytes;
    xui_surface surface = NULL;
    xui_surface_desc_t surface_desc = {0};
    if (!provider) return XUI_ERROR_INVALID_ARGUMENT;
    if (provider->failed) {
        if (provider->restart_at && xrtClock() >= provider->restart_at)
            doc_web_restart_worker(provider);
        return XUI_OK;
    }
    if (!provider->page_ready) {
        if (doc_web_timed_out(provider, provider->startup_started)) {
            doc_web_fail_provider(provider,
                "Document renderer startup timed out",
                provider->recovering);
        }
        return XUI_OK;
    }
    if (provider->stage == DOC_WEB_WAIT_CAPTURE) {
        state = xuiWebRequestGetState(provider->capture);
        if (state == XUI_WEB_REQUEST_PENDING) {
            if (doc_web_timed_out(provider, provider->stage_started)) {
                (void)xuiWebRequestCancel(provider->capture);
                doc_web_active_fail(provider,
                    "Document renderer screenshot timed out");
            }
            return XUI_OK;
        }
        if (state == XUI_WEB_REQUEST_CANCELLED) {
            provider->active->state = DOC_WEB_QUEUED;
            xuiWebRequestRelease(provider->capture);
            provider->capture = NULL;
            provider->active = NULL;
            provider->stage = DOC_WEB_IDLE;
            return XUI_OK;
        }
        if (state != XUI_WEB_REQUEST_COMPLETED ||
                xuiWebRequestGetResultPng(provider->capture, &png, &bytes) != XUI_OK ||
                bytes > DOC_WEB_MAX_PNG ||
                provider->proxy.surfaceLoadMemory(&provider->proxy, &surface,
                    png, (int)bytes, 0) != XUI_OK) {
            if (surface)
                provider->proxy.surfaceDestroy(&provider->proxy, surface);
            doc_web_active_fail(provider,
                "Document renderer screenshot failed");
            return XUI_OK;
        }
        if (!surface || !provider->proxy.surfaceGetDesc ||
                provider->proxy.surfaceGetDesc(&provider->proxy, surface,
                    &surface_desc) != XUI_OK ||
                surface_desc.iWidth != (provider->active->width > 512 ?
                    provider->active->width : 512) ||
                surface_desc.iHeight != (provider->active->height > 350 ?
                    provider->active->height : 350)) {
            if (surface) provider->proxy.surfaceDestroy(&provider->proxy, surface);
            doc_web_active_fail(provider,
                "Document renderer screenshot dimensions are invalid");
            return XUI_OK;
        }
        if (!doc_web_crop_surface(provider, &surface, &surface_desc,
                    provider->active->width, provider->active->height)) {
            provider->proxy.surfaceDestroy(&provider->proxy, surface);
            doc_web_active_fail(provider,
                "Document renderer screenshot crop failed");
            return XUI_OK;
        }
        surface_bytes = (size_t)surface_desc.iWidth * (size_t)surface_desc.iHeight * 4;
        if (!doc_web_reserve_surface(provider, surface_bytes)) {
            provider->proxy.surfaceDestroy(&provider->proxy, surface);
            doc_web_active_fail(provider,
                "Document renderer screenshot cache is full");
            return XUI_OK;
        }
        provider->active->surface = surface;
        provider->active->surface_bytes = surface_bytes;
        provider->cache_bytes += surface_bytes;
        doc_web_active_finish(provider, 1);
    }
    if (provider->stage == DOC_WEB_WAIT_PAINT) {
        if (doc_web_timed_out(provider, provider->stage_started)) {
            doc_web_active_fail(provider,
                "Document renderer screenshot request timed out");
            return XUI_OK;
        }
        doc_web_position_worker(provider,
            provider->active->width > 512 ? provider->active->width : 512,
            provider->active->height > 350 ? provider->active->height : 350);
        if (++provider->stage_frames < 5) return XUI_OK;
        result = xuiWebViewCapturePngAsync(provider->worker, &provider->capture);
        if (result == XUI_ERROR_INVALID_STATE ||
                result == XUI_ERROR_LIMIT_EXCEEDED) return XUI_OK;
        if (result != XUI_OK) {
            doc_web_active_fail(provider,
                "Document renderer screenshot request failed");
            return XUI_OK;
        }
        provider->stage = DOC_WEB_WAIT_CAPTURE;
        provider->stage_frames = 0;
        provider->stage_started = xrtClock();
        return XUI_OK;
    }
    if (provider->stage == DOC_WEB_WAIT_RENDER) {
        if (doc_web_timed_out(provider, provider->stage_started))
            doc_web_active_fail(provider,
                "Document renderer response timed out");
        return XUI_OK;
    }
    if (!provider->active) {
        doc_web_entry* next = doc_web_next(provider);
        if (!next) return XUI_OK;
        provider->active = next;
        next->state = DOC_WEB_RENDERING;
        doc_web_position_worker(provider, 512, 350);
        result = doc_web_send(provider, next);
        if (result != XUI_OK) {
            doc_web_active_fail(provider,
                "Document renderer request failed");
            return XUI_OK;
        }
        provider->stage = DOC_WEB_WAIT_RENDER;
        provider->stage_started = xrtClock();
    }
    return XUI_OK;
}

XUI_API int xuiDocumentWebProviderGetStats(xui_doc_web_provider provider,
    xui_doc_web_provider_stats_t* out)
{
    size_t i;
    if (!provider || !out || out->iSize < sizeof(*out)) return XUI_ERROR_INVALID_ARGUMENT;
    out->iQueued = out->iReady = out->iFailed = out->iDrawn = 0;
    for (i = 0; i < provider->entry_count; i++) {
        switch (provider->entries[i].state) {
        case DOC_WEB_QUEUED: case DOC_WEB_RENDERING: out->iQueued++; break;
        case DOC_WEB_READY: out->iReady++; break;
        case DOC_WEB_FAILED: out->iFailed++; break;
        default: break;
        }
        if (provider->entries[i].state == DOC_WEB_READY && provider->entries[i].drawn)
            out->iDrawn++;
    }
    out->iDrawCalls = provider->draw_calls;
    out->iCacheBytes = provider->cache_bytes;
    out->iPaletteGeneration = provider->palette_generation;
    out->bWorkerFailed = provider->failed;
    memcpy(out->sWorkerError, provider->worker_error,
        sizeof(out->sWorkerError));
    out->bRestartPending = provider->restart_at != 0;
    out->bRecovering = provider->recovering;
    out->iRestartAttempts = provider->restart_attempts;
    return XUI_OK;
}

XUI_API int xuiDocumentWebObjectMeasure(xui_document_snapshot snapshot,
    xui_doc_node_id node, float width, float zoom, xui_vec2_t* size,
    float* baseline, void* user)
{
    xui_doc_web_provider provider = user;
    xui_doc_node_info_t info = {0};
    doc_web_entry* entry;
    uint64_t bytes = 0, document_id, revision;
    char* source;
    if (!snapshot || !provider || !size || !baseline) return XUI_ERROR_INVALID_ARGUMENT;
    info.iSize = sizeof(info);
    if (xuiDocumentSnapshotGetNode(snapshot, node, &info) != XUI_OK ||
            (info.iKind != XUI_DOC_MATH && info.iKind != XUI_DOC_DIAGRAM &&
                !(info.iKind == XUI_DOC_HTML &&
                    (info.tAttributes.iFlags & XUI_DOC_BLOCK))))
        return XUI_ERROR_UNSUPPORTED;
    if (!isfinite(width) || width <= 0 || !isfinite(zoom) || zoom <= 0)
        return XUI_ERROR_UNSUPPORTED;
    document_id = xuiDocumentSnapshotGetIdentity(snapshot);
    revision = xuiDocumentSnapshotGetRevision(snapshot);
    entry = doc_web_find(provider, document_id, revision, node, width, zoom);
    if (entry) {
        entry->used = ++provider->clock;
        if (entry->state == DOC_WEB_READY || entry->state == DOC_WEB_FAILED) {
            size->fX = (float)entry->width;
            size->fY = (float)entry->height;
            *baseline = entry->baseline;
            return XUI_OK;
        }
        return XUI_ERROR_UNSUPPORTED;
    }
    if (xuiDocumentSnapshotCopyText(snapshot, node, NULL, 0, &bytes) != XUI_OK ||
            !bytes || bytes > DOC_WEB_MAX_SOURCE) return XUI_ERROR_UNSUPPORTED;
    source = malloc((size_t)bytes + 1);
    if (!source) return XUI_ERROR_UNSUPPORTED;
    if (xuiDocumentSnapshotCopyText(snapshot, node, source,
            bytes + 1, &bytes) != XUI_OK) { free(source); return XUI_ERROR_UNSUPPORTED; }
    entry = doc_web_allocate(provider);
    if (!entry) { free(source); return XUI_ERROR_UNSUPPORTED; }
    entry->document_id = document_id;
    entry->revision = revision;
    entry->node_id = node;
    entry->kind = info.iKind;
    entry->palette_generation = provider->palette_generation;
    entry->state = DOC_WEB_QUEUED;
    entry->source = source;
    entry->source_bytes = (size_t)bytes;
    entry->available = width;
    entry->zoom = zoom;
    entry->display = info.iKind != XUI_DOC_MATH ||
        (info.tAttributes.iFlags & XUI_DOC_BLOCK) != 0;
    entry->baseline = entry->display ? -1.0f : 0.0f;
    entry->used = ++provider->clock;
    if (provider->failed) {
        entry->state = DOC_WEB_FAILED;
        entry->worker_failure = 1;
        strcpy(entry->error_text, provider->worker_error);
        doc_web_prepare_error_card(provider, entry);
        size->fX = (float)entry->width;
        size->fY = (float)entry->height;
        *baseline = entry->baseline;
        return XUI_OK;
    }
    return XUI_ERROR_UNSUPPORTED;
}

XUI_API int xuiDocumentWebObjectDraw(xui_document_snapshot snapshot,
    xui_doc_node_id node, xui_proxy proxy, xui_draw_context draw,
    xui_rect_t bounds, void* user)
{
    xui_doc_web_provider provider = user;
    uint64_t document_id, revision;
    size_t i;
    if (!snapshot || !provider || !proxy || !proxy->drawSurface)
        return XUI_ERROR_UNSUPPORTED;
    document_id = xuiDocumentSnapshotGetIdentity(snapshot);
    revision = xuiDocumentSnapshotGetRevision(snapshot);
    for (i = 0; i < provider->entry_count; i++) {
        doc_web_entry* entry = &provider->entries[i];
        if ((entry->state == DOC_WEB_READY ||
                entry->state == DOC_WEB_FAILED) &&
                entry->document_id == document_id &&
                entry->revision == revision && entry->node_id == node &&
                entry->palette_generation == provider->palette_generation &&
                fabsf((float)entry->width - bounds.fW) <= 2.0f &&
                fabsf((float)entry->height - bounds.fH) <= 2.0f) {
            if (entry->state == DOC_WEB_FAILED) {
                const char* heading = entry->kind == XUI_DOC_MATH ?
                    "Formula error" : entry->kind == XUI_DOC_DIAGRAM ?
                    "Mermaid error" : "HTML error";
                uint32_t background = provider->background;
                int dark = 3 * (int)((background >> 24) & 255u) +
                    6 * (int)((background >> 16) & 255u) +
                    (int)((background >> 8) & 255u) < 1280;
                uint32_t fill = dark ? 0x3b2328ffu : 0xfff4f2ffu;
                uint32_t border = dark ? 0xffb4a9ffu : 0xb42318ffu;
                uint32_t ink = dark ? 0xffd0c8ffu : 0x912018ffu;
                xui_font font = xuiGetDefaultFont(provider->context);
                xui_rect_t label = {bounds.fX + 8, bounds.fY + 3,
                    fmaxf(0, bounds.fW - 16), 21};
                xui_rect_t detail = {bounds.fX + 8, bounds.fY + 24,
                    fmaxf(0, bounds.fW - 16), 20};
                size_t line;
                int result;
                if (!font || !proxy->drawRectFill || !proxy->drawRectStroke ||
                        !proxy->drawText) return XUI_ERROR_UNSUPPORTED;
                result = proxy->drawRectFill(proxy, draw, bounds, fill);
                if (result == XUI_OK) result = proxy->drawRectStroke(proxy,
                    draw, bounds, 1.0f, border);
                if (result == XUI_OK) result = proxy->drawText(proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=heading, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, label, ink, XUI_TEXT_CLIP);
                for (line = 0; result == XUI_OK &&
                        line < entry->error_line_count; line++) {
                    char text[sizeof(entry->error_text)];
                    size_t start = entry->error_line_start[line];
                    size_t bytes = entry->error_line_bytes[line];
                    memcpy(text, entry->error_text + start, bytes);
                    text[bytes] = 0;
                    detail.fY = bounds.fY + 24 + (float)(20 * line);
                    result = proxy->drawText(proxy, draw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=font, .sText=text, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, detail, ink, XUI_TEXT_CLIP);
                }
                if (result == XUI_OK) provider->draw_calls++;
                return result;
            }
            int result = proxy->drawSurface(proxy, draw, entry->surface,
                (xui_rect_t){0, 0, (float)entry->width, (float)entry->height},
                bounds, XUI_COLOR_WHITE, 0);
            if (result == XUI_OK) { provider->draw_calls++; entry->drawn = 1; }
            return result;
        }
    }
    return XUI_ERROR_UNSUPPORTED;
}

#endif
