#include "xge_clock.h"
#include "xui_config.h"
#if XUI_ENABLE_WEBVIEW
/* Optional, application-owned static math/Mermaid/HTML renderer. The Document
 * remains authoritative; the one browser worker only produces cached pixels. */
#include "xui_document_ui.h"
#include "xui_internal.h"
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
    xui_owned_manager_t owned;
    xui_callback_owner_t invalidate_owner;
    xui_lifetime_link_t parent_watch,worker_watch;
    unsigned working;
    bool proxy_owned;
    xui_surface pending_surface,pending_crop;
    xui_context context;
    xui_widget parent, worker;
    xui_proxy proxy;
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

static void doc_web_message(xui_widget,const char*,const char*,size_t,void*);
static void doc_web_event(xui_widget,int,int32_t,void*);
static int doc_web_message_impl(xui_widget,const char*,const char*,size_t,void*);
static int doc_web_event_impl(xui_widget,int,int32_t,void*);
static void doc_web_worker_gone(void*);
static int doc_web_invalidate(xui_doc_web_provider);
static const xui_owned_manager_ops_t doc_web_ops;
static bool doc_web_business_alive(xui_doc_web_provider provider)
{
    return xuiInternalManagerAlive(&provider->owned) && !provider->context->bCloseRequested &&
        !provider->context->bDestroyPending && !provider->context->bDestroying;
}

static int doc_web_timed_out(xui_doc_web_provider provider,
    uint64_t started)
{
    /* The engine monotonic clock is microseconds; the public option is milliseconds. */
    return xgeClockMicroseconds() - started >= (uint64_t)provider->timeout_ms * 1000u;
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

/* A valid native Surface owner is consumed independently from its cleanup
 * result. An unconsumed preflight failure keeps the actual slot for retry. */
static int doc_web_surface_release(xui_doc_web_provider provider,xui_surface* slot)
{
    if(!*slot)return XUI_OK;
    if(provider->proxy_owned){
        bool consumed=false;
        int status=xuiSurfaceReleaseOwnerChecked(*slot,&consumed);
        if(consumed)*slot=NULL;
        return status;
    }
    provider->proxy->surfaceDestroy(provider->proxy,*slot);*slot=NULL;
    return xrtGetError() ? XUI_ERROR_CALLBACK_FAILED:XUI_OK;
}

static int doc_web_entry_clear(xui_doc_web_provider provider, doc_web_entry* entry)
{
    int status=doc_web_surface_release(provider,&entry->surface);
    if(entry->surface)return status==XUI_OK ? XUI_STATUS_DEFERRED:status;
    if (provider->cache_bytes >= entry->surface_bytes)
        provider->cache_bytes -= entry->surface_bytes;
    free(entry->source);
    memset(entry, 0, sizeof(*entry));
    return status;
}

static int doc_web_invalidate(xui_doc_web_provider provider)
{
    xui_doc_web_invalidate_proc callback=provider->on_invalidate;
    void* user=provider->user;
    if(!callback || !doc_web_business_alive(provider))return XUI_OK;
    xui_callback_snapshot_t snapshot={0};
    int status=xuiInternalManagerCallbackSnapshotBegin(&provider->owned,
        &provider->invalidate_owner,&snapshot,true);
    if(status!=XUI_OK)return status;
    callback(user);return xuiInternalCallbackSnapshotEnd(&snapshot,XUI_OK);
}

static void doc_web_message(xui_widget widget,const char* source,const char* json,size_t bytes,void* raw)
{
    xui_doc_web_provider provider=raw;xui_manager_scope_t scope={0};
    if(!provider || xuiInternalManagerScopeBegin(&provider->owned,&scope)!=XUI_OK)return;
    int status=doc_web_message_impl(widget,source,json,bytes,raw);
    if(status<0)xuiInternalContextRecordFailure(provider->context,status);
    (void)xuiInternalManagerScopeEnd(&scope,status);
}

static void doc_web_event(xui_widget widget,int event,int32_t error,void* raw)
{
    xui_doc_web_provider provider=raw;xui_manager_scope_t scope={0};
    if(!provider || xuiInternalManagerScopeBegin(&provider->owned,&scope)!=XUI_OK)return;
    int status=doc_web_event_impl(widget,event,error,raw);
    if(status<0)xuiInternalContextRecordFailure(provider->context,status);
    (void)xuiInternalManagerScopeEnd(&scope,status);
}

static int doc_web_prepare_error_card(xui_doc_web_provider provider,
    doc_web_entry* entry)
{
    xui_font font = xuiGetDefaultFont(provider->context);
    int status=xuiInternalFontAdmit(provider->context,font);
    if(status!=XUI_OK)return status;
    size_t bytes = strlen(entry->error_text), start = 0;
    float max_width;
    entry->height=0;entry->error_line_count=0;
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
            if(font && provider->proxy->textMeasure){
                status=provider->proxy->textMeasure(provider->proxy,
                    &(xui_text_item_t){.iSize=sizeof(xui_text_item_t),.pFont=font,.sText=candidate,.iTextSize=-1,.iFlags=XUI_TEXT_SHAPE_DEFAULT},&measured);
                if(status!=XUI_OK)goto end;
                if(measured.fX>max_width && fit>start)break;
            }
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
end: ;
    xerror* first=xrtTakeError();
    if(status==XUI_OK && first)status=XUI_ERROR_CALLBACK_FAILED;
    xui_cleanup_prefix_t prefix={0};xuiInternalCleanupPrefixBegin(provider->context,&prefix,status,first);
    xuiInternalContextCleanupOutcome(provider->context,xuiInternalFontRelease(provider->context,font),&status,&first);
    xuiInternalCleanupPrefixEnd(&prefix);
    xrtSetErrorTake(first);return status;
}

static int doc_web_active_finish(xui_doc_web_provider provider, int success)
{
    doc_web_entry* entry = provider->active;
    int result=XUI_OK;
    if (provider->capture) {
        xuiWebRequestRelease(provider->capture);
        provider->capture = NULL;
    }
    if (entry) {
        entry->state = success ? DOC_WEB_READY : DOC_WEB_FAILED;
        if (!success) {
            if (!entry->error_text[0])
                strcpy(entry->error_text, "Rendering failed");
            result=doc_web_prepare_error_card(provider,entry);
        }
    }
    provider->active = NULL;
    provider->stage = DOC_WEB_IDLE;
    provider->stage_frames = 0;
    return result!=XUI_OK ? result:doc_web_invalidate(provider);
}

static int doc_web_active_fail(xui_doc_web_provider provider,
    const char* reason)
{
    if (provider->active)
        (void)snprintf(provider->active->error_text,
            sizeof(provider->active->error_text), "%s", reason);
    return doc_web_active_finish(provider,0);
}

static int doc_web_fail_provider(xui_doc_web_provider provider,
    const char* reason, int retryable)
{
    size_t i;
    int result=XUI_OK;xerror* first=NULL;
    xui_cleanup_prefix_t prefix={0};
    uint64_t now = xgeClockMicroseconds();
    if (provider->failed) return XUI_OK;
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
        int status=doc_web_prepare_error_card(provider,entry);xerror* error=xrtTakeError();
        if(status!=XUI_OK || error){
            if(result==XUI_OK){
                result=status!=XUI_OK ? status:XUI_ERROR_CALLBACK_FAILED;first=error;
                xuiInternalCleanupPrefixBegin(provider->context,&prefix,result,first);
            }
            else xrtErrorFree(error);
        }
    }
    if(prefix.self)xuiInternalCleanupPrefixEnd(&prefix);
    xrtSetErrorTake(first);return result!=XUI_OK ? result:doc_web_invalidate(provider);
}

static int doc_web_message_impl(xui_widget widget, const char* source,
    const char* json, size_t bytes, void* user)
{
    xui_doc_web_provider provider = user;
    xvalue* value;
    int outcome=XUI_OK;
    int id, width, height;
    uint64_t palette_generation;
    float baseline = 0;
    (void)source;
    if (!provider || widget != provider->worker || provider->failed ||
            bytes > 1024u * 1024u) return XUI_OK;
    value = xrtJsonParse(doc_web_view(json, bytes));
    if (!value) return XUI_OK;
    if (doc_web_string_equals(doc_web_field(value, "kind"), "ready")) {
        bool math = false, diagram = false, html = false;
        (void)xrtValueGetBool(doc_web_field(value, "katex"), &math);
        (void)xrtValueGetBool(doc_web_field(value, "mermaid"), &diagram);
        (void)xrtValueGetBool(doc_web_field(value, "dompurify"), &html);
        provider->page_ready = math && diagram && html;
        if (!provider->page_ready)
            outcome=doc_web_fail_provider(provider,
                "Document renderer assets are incomplete", 0);
        else {
            provider->healthy_since = xgeClockMicroseconds();
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
            outcome=doc_web_invalidate(provider);
        }
    } else if (provider->active && provider->stage == DOC_WEB_WAIT_RENDER &&
            doc_web_integer(doc_web_field(value, "id"), &id) &&
            (uint64_t)id == provider->active->request_id &&
            doc_web_u64(doc_web_field(value, "generation"),
                &palette_generation) &&
            palette_generation == provider->active->palette_generation) {
        if (doc_web_string_equals(doc_web_field(value, "kind"), "error")) {
            doc_web_set_error(provider->active, doc_web_field(value, "error"));
            outcome=doc_web_active_finish(provider,0);
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
            provider->stage_started = xgeClockMicroseconds();
        } else {
            outcome=doc_web_active_finish(provider,0);
        }
    }
    xrtValueRelease(value);return outcome;
}

static int doc_web_event_impl(xui_widget widget, int event, int32_t error, void* user)
{
    xui_doc_web_provider provider = user;
    (void)error;
    if (!provider || widget != provider->worker ||
            provider->restarting || provider->failed) return XUI_OK;
    if (event == XUI_WEBVIEW_EVENT_READY) {
        if (xuiWebViewMapLocalFolder(widget, "xui-document.invalid",
                    provider->assets_folder,
                    XUI_WEB_LOCAL_ACCESS_SAME_ORIGIN) != XUI_OK)
            return doc_web_fail_provider(provider,
                "Document renderer assets cannot be mapped", 0);
        else if (xuiWebViewSetMessageHandler(widget,
                    "https://xui-document.invalid", doc_web_message,
                    provider) != XUI_OK)
            return doc_web_fail_provider(provider,
                "Document renderer message channel is unavailable", 0);
        else if (xuiWebViewNavigate(widget,
                    "https://xui-document.invalid/renderer.html") != XUI_OK)
            return doc_web_fail_provider(provider,
                "Document renderer navigation failed", 0);
    } else if (event == XUI_WEBVIEW_EVENT_FAILED ||
            event == XUI_WEBVIEW_EVENT_NAVIGATION_FAILED ||
            event == XUI_WEBVIEW_EVENT_CLOSED) {
        return doc_web_fail_provider(provider,
            event == XUI_WEBVIEW_EVENT_FAILED ?
                "Document renderer browser failed" :
            event == XUI_WEBVIEW_EVENT_NAVIGATION_FAILED ?
                "Document renderer page failed to load" :
                "Document renderer browser closed",
            event != XUI_WEBVIEW_EVENT_NAVIGATION_FAILED);
    }
    return XUI_OK;
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
    if(oldest){
        int status=doc_web_entry_clear(provider,oldest);
        if(status<0)xuiInternalContextRecordFailure(provider->context,status);
        if(status!=XUI_OK || !xuiInternalManagerAlive(&provider->owned))return NULL;
    }
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
        int status=doc_web_entry_clear(provider,oldest);
        if(status<0)xuiInternalContextRecordFailure(provider->context,status);
        if(status!=XUI_OK || !xuiInternalManagerAlive(&provider->owned))return 0;
    }
    return 1;
}

/* CapturePreview returns the whole browser viewport. Keep its dimensions
 * stable for layout, then cache only the measured top-left content region. */
static int doc_web_crop_surface(xui_doc_web_provider provider,xui_surface_desc_t* desc,int width,int height)
{
    if(desc->iWidth==width && desc->iHeight==height)return XUI_OK;
    if(!provider->proxy->surfaceCreate || !provider->proxy->surfaceDrawTo)return XUI_ERROR_UNSUPPORTED;
    if(provider->pending_crop)return XUI_ERROR_BUSY;
    xui_surface_desc_t crop={.iKind=XUI_SURFACE_KIND_TEXTURE,.iFormat=XUI_SURFACE_FORMAT_RGBA8,.iWidth=width,.iHeight=height,
        .iFlags=XUI_SURFACE_USAGE_TARGET|XUI_SURFACE_ALPHA_PREMULTIPLIED};
    int status=provider->proxy->surfaceCreate(provider->proxy,&provider->pending_crop,&crop);
    if(status==XUI_OK && !provider->pending_crop)status=XUI_ERROR_BACKEND_FAILED;
    if(status==XUI_OK && provider->proxy_owned &&
        xuiSurfaceGetProxy(provider->pending_crop)!=provider->proxy)status=XUI_ERROR_INVALID_ARGUMENT;
    if(status==XUI_OK && !doc_web_business_alive(provider))status=XUI_ERROR_INVALID_STATE;
    if(status==XUI_OK){
        xui_rect_t bounds={0,0,(float)width,(float)height};
        status=provider->proxy->surfaceDrawTo(provider->proxy,provider->pending_crop,provider->pending_surface,bounds,bounds,XUI_COLOR_WHITE,0);
    }
    if(status==XUI_OK && !doc_web_business_alive(provider))status=XUI_ERROR_INVALID_STATE;
    if(status!=XUI_OK)return status;
    xerror* first=xrtTakeError();
    if(status==XUI_OK && first)status=XUI_ERROR_CALLBACK_FAILED;
    xui_cleanup_prefix_t prefix={0};xuiInternalCleanupPrefixBegin(provider->context,&prefix,status,first);
    xuiInternalContextCleanupOutcome(provider->context,doc_web_surface_release(provider,&provider->pending_surface),&status,&first);
    if(status==XUI_OK && provider->pending_surface)status=XUI_ERROR_BUSY;
    if(status==XUI_OK){provider->pending_surface=provider->pending_crop;provider->pending_crop=NULL;*desc=crop;}
    xuiInternalCleanupPrefixEnd(&prefix);xrtSetErrorTake(first);return status;
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
    xui_widget_type type=xuiWebViewGetType(provider->context);
    result=type ? xuiInternalWidgetCreatePart(provider->parent,type,&provider->worker,&desc):XUI_ERROR_OUT_OF_MEMORY;
    if (result == XUI_OK)
        result=xuiInternalWidgetInsertPart(provider->parent,provider->parent,provider->worker,NULL);
    if (result == XUI_OK)
        result=xuiWidgetLifetimeWatch(provider->worker,&provider->worker_watch,doc_web_worker_gone,provider);
    if (result == XUI_OK)
        result = xuiWidgetSetEnabled(provider->worker, 0);
    if (result == XUI_OK) doc_web_position_worker(provider, 512, 350);
    if (result == XUI_OK)
        result = xuiWebViewInitializeAsync(provider->worker);
    if (result != XUI_OK && provider->worker) {
        xerror* first=xrtTakeError();
        xui_cleanup_prefix_t prefix={0};
        xuiInternalCleanupPrefixBegin(provider->context,&prefix,result,first);
        provider->restarting = 1;
        xuiInternalContextCleanupOutcome(provider->context,
            xuiInternalWidgetDestroyPart(provider->parent,provider->worker),&result,&first);
        provider->restarting = 0;
        xuiInternalCleanupPrefixEnd(&prefix);xrtSetErrorTake(first);
    }
    return result;
}

static int doc_web_restart_worker(xui_doc_web_provider provider)
{
    int result;
    provider->restarting = 1;
    if (provider->worker) {
        xui_widget worker=provider->worker;
        xuiLifetimeUnwatch(&provider->worker_watch);
        provider->worker = NULL;
        int status=xuiInternalWidgetDestroyPart(provider->parent,worker);
        if(status<0)xuiInternalContextRecordFailure(provider->context,status);
    }
    provider->restarting = 0;
    provider->failed = 0;
    provider->recovering = 1;
    provider->restart_at = 0;
    provider->restart_attempts++;
    provider->startup_started = xgeClockMicroseconds();
    result = doc_web_start_worker(provider);
    if (result != XUI_OK && !provider->failed)
        return doc_web_fail_provider(provider,
            "Document renderer restart failed", 1);
    return XUI_OK;
}

static void doc_web_worker_gone(void* raw)
{
    xui_doc_web_provider provider=raw;provider->worker=NULL;
    if(!provider->restarting && xuiInternalManagerAlive(&provider->owned))
        (void)xuiInternalManagerRequestClose(&provider->owned);
}
static void doc_web_parent_gone(void* raw)
{
    xui_doc_web_provider provider=raw;provider->parent=NULL;
    if(xuiInternalManagerAlive(&provider->owned))
        (void)xuiInternalManagerRequestClose(&provider->owned);
}
static bool doc_web_trace(const void* raw,xrtownershipvisitor visit,void* user)
{
    const struct xui_doc_web_provider_t* provider=raw;
    if(provider->working)return false;
    if(provider->proxy_owned && !visit(xuiProxyOwnership(provider->proxy),user))return false;
    for(size_t i=0;i<provider->entry_count;++i)
        if(provider->proxy_owned && provider->entries[i].surface &&
            !visit(xuiSurfaceOwnership(provider->entries[i].surface),user))return false;
    if(provider->proxy_owned && provider->pending_surface && !visit(xuiSurfaceOwnership(provider->pending_surface),user))return false;
    if(provider->proxy_owned && provider->pending_crop && !visit(xuiSurfaceOwnership(provider->pending_crop),user))return false;
    return !provider->invalidate_owner.tReference.Data || visit(provider->invalidate_owner.tReference,user);
}
static int doc_web_ready(const void* raw)
{
    const struct xui_doc_web_provider_t* provider=raw;
    return provider->working ? XUI_ERROR_BUSY:XUI_OK;
}
static int doc_web_prepare(void* raw,bool* complete)
{
    xui_doc_web_provider provider=raw;*complete=!provider->working;
    if(!*complete)return XUI_STATUS_DEFERRED;
    int status=XUI_OK;xerror* first=NULL;
    for(size_t i=0;i<provider->entry_count;++i){
        int next=doc_web_entry_clear(provider,&provider->entries[i]);
        if(next<0)xuiInternalContextCleanupOutcome(provider->context,next,&status,&first);
        if(provider->entries[i].surface)*complete=false;
    }
    int next=doc_web_surface_release(provider,&provider->pending_surface);
    if(next<0)xuiInternalContextCleanupOutcome(provider->context,next,&status,&first);
    next=doc_web_surface_release(provider,&provider->pending_crop);
    if(next<0)xuiInternalContextCleanupOutcome(provider->context,next,&status,&first);
    if(provider->pending_surface || provider->pending_crop)*complete=false;
    if(*complete){provider->entry_count=0;provider->active=NULL;}
    xrtSetErrorTake(first);return status==XUI_OK && !*complete ? XUI_STATUS_DEFERRED:status;
}
static void doc_web_close(void* raw)
{
    xui_doc_web_provider provider=raw;provider->restarting=1;
    xuiLifetimeUnwatch(&provider->parent_watch);
    if(provider->capture){(void)xuiWebRequestCancel(provider->capture);xuiWebRequestRelease(provider->capture);provider->capture=NULL;}
    if(provider->worker){
        xui_widget worker=provider->worker;
        xuiLifetimeUnwatch(&provider->worker_watch);provider->worker=NULL;
        /* Internal identity is installed before Init. Public destroy/reparent
         * cannot detach this worker from its actual native owner. */
        int status=xuiInternalWidgetDestroyPart((xui_widget)worker->pInternalOwner,worker);
        if(status<0)xuiInternalContextRecordFailure(provider->context,status);
    }
}
static int doc_web_finalize(void* raw)
{
    xui_doc_web_provider provider=raw;xui_context context=provider->context;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,XUI_OK);
    xuiInternalCleanupAdd(&receipt,xuiInternalCallbackOwnerDropOutcome(context,&provider->invalidate_owner,receipt.status,receipt.error));
    if(provider->proxy_owned)xuiInternalCleanupAdd(&receipt,xuiProxyReleaseChecked(provider->proxy));
    free(provider->user_data_folder);free(provider->assets_folder);free(provider);
    return xuiInternalCleanupEnd(context,&receipt);
}
static const xui_owned_manager_ops_t doc_web_ops={doc_web_trace,doc_web_ready,doc_web_prepare,doc_web_close,doc_web_finalize};
static void doc_web_construct_gone(void* raw){*(bool*)raw=false;}
static int doc_web_construct_end(xui_call_scope_t* scope,int status,
    xui_lifetime_link_t* watch,bool* alive,xui_doc_web_provider provider,xui_doc_web_provider* output)
{
    xerror* first=xrtTakeError();if(status==XUI_OK && first)status=XUI_ERROR_CALLBACK_FAILED;
    if(status==XUI_OK && (!*alive || provider->context->bCloseRequested))status=XUI_ERROR_INVALID_STATE;
    if(status!=XUI_OK && *alive){
        xui_cleanup_prefix_t prefix={0};xuiInternalCleanupPrefixBegin(provider->context,&prefix,status,first);
        (void)xuiInternalManagerRequestClose(&provider->owned);xrtClearError();
        xuiInternalCleanupPrefixEnd(&prefix);
    }
    status=xuiInternalCallScopeEndOutcome(scope,status,&first);xuiLifetimeUnwatch(watch);
    if(status==XUI_OK && !*alive)status=XUI_ERROR_INVALID_STATE;
    if(status==XUI_OK)*output=provider;
    xrtSetErrorTake(first);return status;
}
XUI_API int xuiDocumentWebProviderCreate(const xui_doc_web_provider_desc_t* desc,xui_doc_web_provider* out)
{
    if(out)*out=NULL;
    if(!out || !desc || desc->iSize<sizeof(*desc) || !desc->pContext || !desc->pParent ||
        !desc->sAssetsFolder || xuiWidgetGetContext(desc->pParent)!=desc->pContext)return XUI_ERROR_INVALID_ARGUMENT;
    xui_call_scope_t scope={0};int status=xuiCallScopeBegin(desc->pContext,&scope);if(status!=XUI_OK)return status;
    xui_doc_web_provider provider=calloc(1,sizeof(*provider));
    if(!provider){xerror* first=xrtTakeError();status=xuiInternalCallScopeEndOutcome(&scope,XUI_ERROR_OUT_OF_MEMORY,&first);xrtSetErrorTake(first);return status;}
    provider->context=desc->pContext;provider->parent=desc->pParent;
    status=xuiInternalManagerRegister(provider->context,&provider->owned,&doc_web_ops,provider);
    if(status!=XUI_OK){free(provider);xerror* first=xrtTakeError();status=xuiInternalCallScopeEndOutcome(&scope,status,&first);xrtSetErrorTake(first);return status;}
    bool alive=true;xui_lifetime_link_t watch={0};
    if(xuiInternalManagerWatch(&provider->owned,&watch,doc_web_construct_gone,&alive)!=XUI_OK)abort();
    provider->on_invalidate=desc->onInvalidate;provider->user=desc->pUser;
    provider->foreground=XUI_COLOR_RGBA(17,17,17,255);provider->background=XUI_COLOR_WHITE;
    provider->palette_generation=1;provider->timeout_ms=desc->iTimeoutMs ? desc->iTimeoutMs:DOC_WEB_DEFAULT_TIMEOUT_MS;
    provider->startup_started=xgeClockMicroseconds();
    provider->proxy=provider->context->pOwnedProxy ? provider->context->pOwnedProxy:xuiInternalContextGetProxy(provider->context);
    if(!provider->proxy || !provider->proxy->surfaceLoadMemory || !provider->proxy->surfaceDestroy || !provider->proxy->drawSurface)
        return doc_web_construct_end(&scope,XUI_ERROR_NOT_INITIALIZED,&watch,&alive,provider,out);
    if(provider->context->pOwnedProxy){
        status=xuiProxyAddRef(provider->proxy);
        if(status!=XUI_OK)return doc_web_construct_end(&scope,status,&watch,&alive,provider,out);
        provider->proxy_owned=true;
    }
    provider->assets_folder=malloc(strlen(desc->sAssetsFolder)+1);
    if(provider->assets_folder)strcpy(provider->assets_folder,desc->sAssetsFolder);
    if(desc->sUserDataFolder){provider->user_data_folder=malloc(strlen(desc->sUserDataFolder)+1);if(provider->user_data_folder)strcpy(provider->user_data_folder,desc->sUserDataFolder);}
    if(!provider->assets_folder || (desc->sUserDataFolder && !provider->user_data_folder))
        return doc_web_construct_end(&scope,XUI_ERROR_OUT_OF_MEMORY,&watch,&alive,provider,out);
    status=xuiWidgetLifetimeWatch(provider->parent,&provider->parent_watch,doc_web_parent_gone,provider);
    if(status==XUI_OK)status=doc_web_start_worker(provider);
    return doc_web_construct_end(&scope,status,&watch,&alive,provider,out);
}
XUI_API int xuiDocumentWebProviderReleaseOwnerChecked(xui_doc_web_provider provider,bool* consumed)
{
    if(!consumed)return XUI_ERROR_INVALID_ARGUMENT;
    *consumed=false;if(!provider)return XUI_ERROR_INVALID_ARGUMENT;
    if(provider->owned.pending)return XUI_OK;
    xui_manager_scope_t scope={0};int status=xuiInternalManagerScopeBegin(&provider->owned,&scope);if(status!=XUI_OK)return status;
    status=xuiInternalManagerRequestClose(&provider->owned);*consumed=status==XUI_OK;
    return xuiInternalManagerScopeEnd(&scope,status);
}
XUI_API int xuiDocumentWebProviderReleaseChecked(xui_doc_web_provider provider)
{bool consumed=false;return xuiDocumentWebProviderReleaseOwnerChecked(provider,&consumed);}
XUI_API void xuiDocumentWebProviderRelease(xui_doc_web_provider provider)
{if(provider)(void)xuiDocumentWebProviderReleaseChecked(provider);}
XUI_API xui_context xuiDocumentWebProviderGetContext(xui_doc_web_provider provider)
{return provider && xuiInternalManagerAlive(&provider->owned) ? provider->context:NULL;}
XUI_API int xuiDocumentWebProviderLifetimeWatch(xui_doc_web_provider provider,xui_lifetime_link_t* link,xui_lifetime_invalidate_proc invalidate,void* user)
{return provider ? xuiInternalManagerWatch(&provider->owned,link,invalidate,user):XUI_ERROR_INVALID_ARGUMENT;}
XUI_API int xuiDocumentWebProviderSetInvalidateOwned(xui_doc_web_provider provider,xui_doc_web_invalidate_proc callback,void* user,const xui_callback_owner_t* owner)
{
    if(!provider)return XUI_ERROR_INVALID_ARGUMENT;
    xui_manager_callback_update_t update={0};int status=xuiInternalManagerCallbackUpdateBegin(&provider->owned,&provider->invalidate_owner,callback!=NULL,owner,&update);
    if(status!=XUI_OK)return status;
    provider->on_invalidate=callback;provider->user=user;
    xuiInternalManagerCallbackUpdatePublish(&update);return xuiInternalManagerCallbackUpdateEnd(&update,XUI_OK);
}
XUI_API int xuiDocumentWebProviderSetInvalidate(xui_doc_web_provider provider,xui_doc_web_invalidate_proc callback,void* user)
{return xuiDocumentWebProviderSetInvalidateOwned(provider,callback,user,NULL);}

static int xuiDocumentWebProviderSetPalette_impl(xui_doc_web_provider provider,
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
    int cleanup=XUI_OK;xerror* first=NULL;
    for(i=0;i<provider->entry_count;++i){
        int next=doc_web_entry_clear(provider,&provider->entries[i]);
        if(next<0)xuiInternalContextCleanupOutcome(provider->context,next,&cleanup,&first);
        if(provider->entries[i].surface && cleanup==XUI_OK)cleanup=XUI_ERROR_BUSY;
    }
    xrtSetErrorTake(first);if(cleanup!=XUI_OK)return cleanup;
    if(!doc_web_business_alive(provider))return XUI_ERROR_INVALID_STATE;
    provider->entry_count = 0;
    provider->foreground = foreground;
    provider->background = background;
    provider->palette_generation++;
    if (!provider->palette_generation) provider->palette_generation = 1;
    return doc_web_invalidate(provider);
}

static int xuiDocumentWebProviderUpdate_impl(xui_doc_web_provider provider)
{
    int state, result;
    const void* png;
    size_t bytes, surface_bytes;
    xui_surface_desc_t surface_desc = {0};
    if (!provider) return XUI_ERROR_INVALID_ARGUMENT;
    if (provider->failed) {
        if (provider->restart_at && xgeClockMicroseconds() >= provider->restart_at)
            return doc_web_restart_worker(provider);
        return XUI_OK;
    }
    if (!provider->page_ready) {
        if (doc_web_timed_out(provider, provider->startup_started)) {
            return doc_web_fail_provider(provider,
                "Document renderer startup timed out",provider->recovering);
        }
        return XUI_OK;
    }
    if (provider->stage == DOC_WEB_WAIT_CAPTURE) {
        state = xuiWebRequestGetState(provider->capture);
        if (state == XUI_WEB_REQUEST_PENDING) {
            if (doc_web_timed_out(provider, provider->stage_started)) {
                (void)xuiWebRequestCancel(provider->capture);
                return doc_web_active_fail(provider,"Document renderer screenshot timed out");
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
        doc_web_entry* entry=provider->active;
        if(!entry)return XUI_ERROR_INVALID_STATE;
        if(provider->pending_surface || provider->pending_crop){
            int cleanup=XUI_OK;xerror* first=NULL;
            xuiInternalContextCleanupOutcome(provider->context,doc_web_surface_release(provider,&provider->pending_surface),&cleanup,&first);
            xuiInternalContextCleanupOutcome(provider->context,doc_web_surface_release(provider,&provider->pending_crop),&cleanup,&first);
            xrtSetErrorTake(first);if(cleanup!=XUI_OK)return cleanup;
            if(provider->pending_surface || provider->pending_crop)return XUI_ERROR_BUSY;
        }
        result=state==XUI_WEB_REQUEST_COMPLETED ? xuiWebRequestGetResultPng(provider->capture,&png,&bytes):XUI_ERROR_BACKEND_FAILED;
        if(result==XUI_OK && bytes>DOC_WEB_MAX_PNG)result=XUI_ERROR_LIMIT_EXCEEDED;
        if(result==XUI_OK)result=provider->proxy->surfaceLoadMemory(provider->proxy,&provider->pending_surface,png,(int)bytes,0);
        if(result==XUI_OK && provider->proxy_owned && provider->pending_surface &&
            xuiSurfaceGetProxy(provider->pending_surface)!=provider->proxy)result=XUI_ERROR_INVALID_ARGUMENT;
        if(result==XUI_OK && (!doc_web_business_alive(provider) || provider->active!=entry))result=XUI_ERROR_INVALID_STATE;
        if(result==XUI_OK){
            if(!provider->pending_surface || !provider->proxy->surfaceGetDesc)result=XUI_ERROR_BACKEND_FAILED;
            else result=provider->proxy->surfaceGetDesc(provider->proxy,provider->pending_surface,&surface_desc);
        }
        if(result==XUI_OK && (surface_desc.iWidth!=(entry->width>512 ? entry->width:512) ||
            surface_desc.iHeight!=(entry->height>350 ? entry->height:350)))result=XUI_ERROR_BACKEND_FAILED;
        if(result==XUI_OK && (!doc_web_business_alive(provider) || provider->active!=entry))result=XUI_ERROR_INVALID_STATE;
        if(result==XUI_OK)result=doc_web_crop_surface(provider,&surface_desc,entry->width,entry->height);
        if(result==XUI_OK && (!doc_web_business_alive(provider) || provider->active!=entry))result=XUI_ERROR_INVALID_STATE;
        if(result==XUI_OK){
            surface_bytes=(size_t)surface_desc.iWidth*(size_t)surface_desc.iHeight*4;
            if(!doc_web_reserve_surface(provider,surface_bytes))result=XUI_ERROR_LIMIT_EXCEEDED;
        }
        if(result==XUI_OK && (!doc_web_business_alive(provider) || provider->active!=entry))result=XUI_ERROR_INVALID_STATE;
        if(result!=XUI_OK){
            xerror* first=xrtTakeError();
            xui_cleanup_prefix_t prefix={0};xuiInternalCleanupPrefixBegin(provider->context,&prefix,result,first);
            xuiInternalContextCleanupOutcome(provider->context,doc_web_surface_release(provider,&provider->pending_surface),&result,&first);
            xuiInternalContextCleanupOutcome(provider->context,doc_web_surface_release(provider,&provider->pending_crop),&result,&first);
            if(xuiInternalManagerAlive(&provider->owned))doc_web_active_fail(provider,"Document renderer screenshot failed");
            xuiInternalCleanupPrefixEnd(&prefix);xrtClearError();xrtSetErrorTake(first);return result;
        }
        entry->surface=provider->pending_surface;provider->pending_surface=NULL;
        entry->surface_bytes=surface_bytes;provider->cache_bytes+=surface_bytes;
        result=doc_web_active_finish(provider,1);if(result!=XUI_OK)return result;
    }
    if (provider->stage == DOC_WEB_WAIT_PAINT) {
        if (doc_web_timed_out(provider, provider->stage_started)) {
            return doc_web_active_fail(provider,"Document renderer screenshot request timed out");
        }
        doc_web_position_worker(provider,
            provider->active->width > 512 ? provider->active->width : 512,
            provider->active->height > 350 ? provider->active->height : 350);
        if (++provider->stage_frames < 5) return XUI_OK;
        result = xuiWebViewCapturePngAsync(provider->worker, &provider->capture);
        if (result == XUI_ERROR_INVALID_STATE ||
                result == XUI_ERROR_LIMIT_EXCEEDED) return XUI_OK;
        if (result != XUI_OK) {
            return doc_web_active_fail(provider,"Document renderer screenshot request failed");
        }
        provider->stage = DOC_WEB_WAIT_CAPTURE;
        provider->stage_frames = 0;
        provider->stage_started = xgeClockMicroseconds();
        return XUI_OK;
    }
    if (provider->stage == DOC_WEB_WAIT_RENDER) {
        if (doc_web_timed_out(provider, provider->stage_started))
            return doc_web_active_fail(provider,"Document renderer response timed out");
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
            return doc_web_active_fail(provider,"Document renderer request failed");
        }
        provider->stage = DOC_WEB_WAIT_RENDER;
        provider->stage_started = xgeClockMicroseconds();
    }
    return XUI_OK;
}

static int xuiDocumentWebProviderGetStats_impl(xui_doc_web_provider provider,
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

static int xuiDocumentWebObjectMeasure_impl(xui_document_snapshot snapshot,
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
            if(entry->state==DOC_WEB_FAILED && entry->height<=0){
                int status=doc_web_prepare_error_card(provider,entry);if(status!=XUI_OK)return status;
            }
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
        int status=doc_web_prepare_error_card(provider,entry);if(status!=XUI_OK)return status;
        size->fX = (float)entry->width;
        size->fY = (float)entry->height;
        *baseline = entry->baseline;
        return XUI_OK;
    }
    return XUI_ERROR_UNSUPPORTED;
}

static int xuiDocumentWebObjectDraw_impl(xui_document_snapshot snapshot,
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
                result=xuiInternalFontAdmit(provider->context,font);
                if(result!=XUI_OK)return result;
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
                xerror* first=xrtTakeError();
                if(result==XUI_OK && first)result=XUI_ERROR_CALLBACK_FAILED;
                xui_cleanup_prefix_t prefix={0};xuiInternalCleanupPrefixBegin(provider->context,&prefix,result,first);
                xuiInternalContextCleanupOutcome(provider->context,xuiInternalFontRelease(provider->context,font),&result,&first);
                xuiInternalCleanupPrefixEnd(&prefix);
                xrtSetErrorTake(first);return result;
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

static int doc_web_business_begin(xui_doc_web_provider provider,xui_manager_scope_t* scope)
{
    if(!provider)return XUI_ERROR_INVALID_ARGUMENT;
    int status=xuiInternalManagerScopeBegin(&provider->owned,scope);if(status!=XUI_OK)return status;
    if(provider->working)return xuiInternalManagerScopeEnd(scope,XUI_ERROR_BUSY);
    provider->working=1;return XUI_OK;
}
static int doc_web_business_end(xui_doc_web_provider provider,xui_manager_scope_t* scope,int status)
{
    if(status==XUI_OK && !doc_web_business_alive(provider))status=XUI_ERROR_INVALID_STATE;
    provider->working=0;return xuiInternalManagerScopeEnd(scope,status);
}
XUI_API int xuiDocumentWebProviderSetPalette(xui_doc_web_provider provider,uint32_t foreground,uint32_t background)
{
    xui_manager_scope_t scope={0};int status=doc_web_business_begin(provider,&scope);if(status!=XUI_OK)return status;
    return doc_web_business_end(provider,&scope,xuiDocumentWebProviderSetPalette_impl(provider,foreground,background));
}
XUI_API int xuiDocumentWebProviderUpdate(xui_doc_web_provider provider)
{
    xui_manager_scope_t scope={0};int status=doc_web_business_begin(provider,&scope);if(status!=XUI_OK)return status;
    xui_web_request retained=provider->capture;if(retained)xuiWebRequestRetain(retained);
    status=xuiDocumentWebProviderUpdate_impl(provider);
    if(retained)xuiWebRequestRelease(retained);
    return doc_web_business_end(provider,&scope,status);
}
XUI_API int xuiDocumentWebProviderGetStats(xui_doc_web_provider provider,xui_doc_web_provider_stats_t* output)
{
    /* Pure copies do not dispatch or drain the Context at their return tail. */
    if(!provider || !doc_web_business_alive(provider))return XUI_ERROR_INVALID_STATE;
    if(provider->owned.creatorThread!=xrtThreadCurrentId())return XUI_ERROR_BUSY;
    return xuiDocumentWebProviderGetStats_impl(provider,output);
}
XUI_API int xuiDocumentWebObjectMeasure(xui_document_snapshot snapshot,xui_doc_node_id node,float width,float zoom,xui_vec2_t* size,float* baseline,void* raw)
{
    xui_doc_web_provider provider=raw;xui_manager_scope_t scope={0};int status=doc_web_business_begin(provider,&scope);if(status!=XUI_OK)return status;
    return doc_web_business_end(provider,&scope,xuiDocumentWebObjectMeasure_impl(snapshot,node,width,zoom,size,baseline,raw));
}
XUI_API int xuiDocumentWebObjectDraw(xui_document_snapshot snapshot,xui_doc_node_id node,xui_proxy proxy,xui_draw_context draw,xui_rect_t bounds,void* raw)
{
    xui_doc_web_provider provider=raw;xui_manager_scope_t scope={0};int status=doc_web_business_begin(provider,&scope);if(status!=XUI_OK)return status;
    return doc_web_business_end(provider,&scope,xuiDocumentWebObjectDraw_impl(snapshot,node,proxy,draw,bounds,raw));
}

#endif
