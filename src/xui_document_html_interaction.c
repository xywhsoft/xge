#include "../xui_config.h"
#if XUI_ENABLE_WEBVIEW
#include "xui_document_ui.h"
#include "xui_internal.h"
#include "xui_webview_internal.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct xui_doc_html_interaction_t {
    xui_document document;
    xui_doc_node_id node;
    xui_widget window, webview;
    char* html;
    size_t html_bytes;
    uint64_t seen_revision;
    int dirty;
    int focus_pending;
    int backend_error;
    uint32_t page_generation;
    xui_doc_html_open_link_proc on_open_link;
    void* user;
};

static int doc_html_interaction_read(xui_doc_html_interaction panel,
    char** html, size_t* bytes, uint64_t* revision)
{
    xui_document_snapshot snapshot = NULL;
    xui_doc_node_info_t node = {0};
    uint64_t required = 0;
    char* copy = NULL;
    int result;
    *html = NULL; *bytes = 0;
    result = xuiDocumentAcquireSnapshot(panel->document, &snapshot);
    if (result != XUI_OK) return result;
    *revision = xuiDocumentSnapshotGetRevision(snapshot);
    node.iSize = sizeof(node);
    result = xuiDocumentSnapshotGetNode(snapshot, panel->node, &node);
    if (result == XUI_OK && node.iKind != XUI_DOC_HTML)
        result = XUI_ERROR_INVALID_ARGUMENT;
    if (result == XUI_OK)
        result = xuiDocumentSnapshotCopyText(snapshot, panel->node,
            NULL, 0, &required);
    if (result == XUI_OK && required >= SIZE_MAX)
        result = XUI_DOC_ERROR_LIMIT;
    if (result == XUI_OK) {
        copy = malloc((size_t)required + 1);
        if (!copy) result = XUI_ERROR_OUT_OF_MEMORY;
    }
    if (result == XUI_OK)
        result = xuiDocumentSnapshotCopyText(snapshot, panel->node,
            copy, required + 1, &required);
    if (result == XUI_OK && memchr(copy, 0, (size_t)required))
        result = XUI_ERROR_INVALID_ARGUMENT;
    xuiDocumentSnapshotRelease(snapshot);
    if (result != XUI_OK) { free(copy); return result; }
    *html = copy; *bytes = (size_t)required;
    return XUI_OK;
}

static int doc_html_interaction_load(xui_doc_html_interaction panel)
{
    static const char prefix[] = "<!doctype html><html><head><meta charset=\"utf-8\">"
        "<style>html,body{margin:0;width:100%;height:100%;overflow:hidden}"
        "iframe{display:block;width:100%;height:100%;border:0}</style></head>"
        "<body><iframe sandbox=\"allow-same-origin\" data-generation=\"";
    static const char middle[] = "\" srcdoc=\"";
    static const char suffix[] = "\"></iframe><script>"
        "(()=>{const f=document.querySelector('iframe');"
        "f.addEventListener('load',()=>{const d=f.contentDocument;if(!d)return;"
        "d.addEventListener('click',e=>{"
        "const a=e.target.closest?.('a[href]');if(!a)return;"
        "const raw=a.getAttribute('href');e.preventDefault();"
        "if(raw[0]==='#'){if(raw.length===1){f.contentWindow.scrollTo(0,0);return;}"
        "let id;try{id=decodeURIComponent(raw.slice(1));}catch(_){return;}"
        "const t=d.getElementById(id);if(t)t.scrollIntoView();return;}"
        "let u;try{u=new URL(a.href);}catch(_){return;}"
        "if(u.protocol!=='http:'&&u.protocol!=='https:')return;"
        "window.chrome?.webview?.postMessage({kind:'externalLink',"
        "url:u.href,generation:Number(f.dataset.generation)});"
        "});});})();"
        "</script></body></html>";
    char* page;
    char* cursor;
    char generation[16];
    int generation_bytes;
    size_t i, capacity, length;
    int result;
    if (!panel->dirty || !panel->webview ||
        xuiWebViewGetState(panel->webview) != XUI_WEBVIEW_READY)
        return XUI_OK;
    if (panel->html_bytes >
        (SIZE_MAX - sizeof(prefix) - sizeof(middle) - sizeof(suffix) -
            sizeof(generation)) / 6)
        return XUI_DOC_ERROR_LIMIT;
    if (++panel->page_generation == 0) ++panel->page_generation;
    generation_bytes = snprintf(generation, sizeof(generation), "%u",
        (unsigned)panel->page_generation);
    if (generation_bytes <= 0 ||
        (size_t)generation_bytes >= sizeof(generation))
        return XUI_ERROR_INVALID_STATE;
    capacity = sizeof(prefix) + sizeof(middle) + panel->html_bytes * 6 +
        sizeof(suffix) + (size_t)generation_bytes;
    page = malloc(capacity);
    if (!page) return XUI_ERROR_OUT_OF_MEMORY;
    cursor = page;
    memcpy(cursor, prefix, sizeof(prefix) - 1);
    cursor += sizeof(prefix) - 1;
    memcpy(cursor, generation, (size_t)generation_bytes);
    cursor += generation_bytes;
    memcpy(cursor, middle, sizeof(middle) - 1);
    cursor += sizeof(middle) - 1;
    for (i = 0; i < panel->html_bytes; i++) {
        const char* replacement = NULL;
        size_t bytes = 0;
        switch (panel->html[i]) {
        case '&': replacement = "&amp;"; bytes = 5; break;
        case '"': replacement = "&quot;"; bytes = 6; break;
        case '<': replacement = "&lt;"; bytes = 4; break;
        case '>': replacement = "&gt;"; bytes = 4; break;
        default: *cursor++ = panel->html[i]; break;
        }
        if (replacement) { memcpy(cursor, replacement, bytes); cursor += bytes; }
    }
    memcpy(cursor, suffix, sizeof(suffix) - 1);
    cursor += sizeof(suffix) - 1;
    *cursor = 0;
    length = (size_t)(cursor - page);
    result = xuiWebViewLoadHtml(panel->webview, page, length);
    free(page);
    if (result == XUI_OK) panel->dirty = 0;
    return result;
}

static int doc_html_interaction_advance(xui_doc_html_interaction panel)
{
    int result = doc_html_interaction_load(panel);
    if (result != XUI_OK || !panel->focus_pending ||
        xuiWebViewGetState(panel->webview) != XUI_WEBVIEW_READY)
        return result;
    result = xuiWebViewFocus(panel->webview);
    if (result == XUI_OK) panel->focus_pending = 0;
    return result == XUI_ERROR_INVALID_STATE ? XUI_OK : result;
}

static void doc_html_interaction_message(xui_widget webview,
    const char* source, const char* json, size_t bytes, void* user)
{
    xui_doc_html_interaction panel = user;
    xvalue* root;
    xvalue* kind;
    xvalue* url;
    xvalue* generation;
    xstrview kind_text, url_text;
    uint64_t message_generation = 0;
    int64_t signed_generation = 0;
    xui_doc_html_open_link_proc callback;
    void* callback_user;
    char* copy;
    size_t scheme, i;
    (void)webview;
    if (!panel || !panel->on_open_link || !source ||
        strcmp(source, "about:blank") != 0 || !json || bytes > 8192)
        return;
    root = xrtJsonParse((xstrview){json, bytes});
    if (!root) return;
    kind = xrtValueObjectGet(root, (xstrview){"kind", 4});
    url = xrtValueObjectGet(root, (xstrview){"url", 3});
    generation = xrtValueObjectGet(root, (xstrview){"generation", 10});
    if (!xrtValueGetString(kind, &kind_text) ||
        kind_text.Size != 12 ||
        memcmp(kind_text.Data, "externalLink", 12) != 0 ||
        !xrtValueGetString(url, &url_text) ||
        url_text.Size > 4096) goto done;
    if (!xrtValueGetUInt(generation, &message_generation) &&
        (!xrtValueGetInt(generation, &signed_generation) ||
            signed_generation < 0)) goto done;
    if (signed_generation > 0)
        message_generation = (uint64_t)signed_generation;
    if (message_generation != panel->page_generation) goto done;
    scheme = url_text.Size >= 8 &&
        memcmp(url_text.Data, "https://", 8) == 0 ? 8 :
        url_text.Size >= 7 &&
        memcmp(url_text.Data, "http://", 7) == 0 ? 7 : 0;
    if (!scheme || url_text.Size <= scheme) goto done;
    for (i = 0; i < url_text.Size; i++)
        if ((unsigned char)url_text.Data[i] < 32 ||
            (unsigned char)url_text.Data[i] == 127) goto done;
    copy = malloc(url_text.Size + 1);
    if (!copy) goto done;
    memcpy(copy, url_text.Data, url_text.Size);
    copy[url_text.Size] = 0;
    callback = panel->on_open_link;
    callback_user = panel->user;
    callback(copy, callback_user);
    free(copy);
done:
    xrtValueRelease(root);
}

static void doc_html_interaction_event(xui_widget webview, int event,
    int32_t native_error, void* user)
{
    xui_doc_html_interaction panel = user;
    (void)native_error;
    if (event == XUI_WEBVIEW_EVENT_READY) {
        panel->backend_error = xuiWebViewBlockExternalResources(webview);
        if (panel->backend_error == XUI_OK)
            panel->backend_error = xuiWebViewSetMessageHandler(webview,
                "about:blank", doc_html_interaction_message, panel);
        if (panel->backend_error == XUI_OK)
            panel->backend_error = doc_html_interaction_load(panel);
    }
}

XUI_API int xuiDocumentHtmlInteractionCreate(
    const xui_doc_html_interaction_desc_t* desc, xui_doc_html_interaction* out)
{
    xui_doc_html_interaction panel;
    xui_window_desc_t window_desc = {0};
    xui_webview_desc_t webview_desc = {0};
    xui_widget client;
    xui_rect_t bounds;
    xui_size_t viewport;
    int result;
    if (!out || !desc || desc->iSize < sizeof(*desc) ||
        !desc->pContext || !desc->pDocument || !desc->iNodeId)
        return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    if (!xuiGetRootWidget(desc->pContext))
        return XUI_ERROR_INVALID_ARGUMENT;
    viewport = xuiGetViewportSize(desc->pContext);
    if (viewport.iW <= 0 || viewport.iH <= 0)
        return XUI_ERROR_INVALID_STATE;
    panel = calloc(1, sizeof(*panel));
    if (!panel) return XUI_ERROR_OUT_OF_MEMORY;
    panel->document = desc->pDocument;
    panel->node = desc->iNodeId;
    panel->on_open_link = desc->onOpenLink;
    panel->user = desc->pUser;
    xuiDocumentRetain(panel->document);
    result = doc_html_interaction_read(panel, &panel->html,
        &panel->html_bytes, &panel->seen_revision);
    if (result != XUI_OK) goto fail;
    panel->dirty = 1;
    bounds = desc->tBounds;
    if (bounds.fW <= 0) bounds.fW = 640;
    if (bounds.fH <= 0) bounds.fH = 420;
    window_desc.iSize = sizeof(window_desc);
    window_desc.sTitle = desc->sTitle ? desc->sTitle : "HTML interaction";
    window_desc.pFont = xuiGetDefaultFont(desc->pContext);
    window_desc.bHideCollapse = window_desc.bHideMaximize = 1;
    window_desc.bNotResizable = 1;
    window_desc.fTitleBarHeight = 28;
    window_desc.fBorderWidth = 1;
    window_desc.fButtonSize = 18;
    result = xuiWindowCreate(desc->pContext, &panel->window, &window_desc);
    if (result != XUI_OK) goto fail;
    result = xuiWidgetSetRect(panel->window, bounds);
    if (result != XUI_OK) goto fail;
    client = xuiWindowGetClientWidget(panel->window);
    if (!client) { result = XUI_ERROR_INVALID_STATE; goto fail; }
    (void)xuiWidgetSetLayoutType(client, XUI_LAYOUT_MANUAL);
    (void)xuiWidgetSetFlowMode(client, XUI_FLOW_ABSOLUTE);
    (void)xuiWidgetSetPadding(client, (xui_thickness_t){0, 0, 0, 0});
    (void)xuiWidgetSetGap(client, 0);
    webview_desc.iSize = sizeof(webview_desc);
    webview_desc.onEvent = doc_html_interaction_event;
    webview_desc.pUser = panel;
    result = xuiWebViewCreate(desc->pContext, &panel->webview,
        &webview_desc);
    if (result != XUI_OK) goto fail;
    result = xuiWidgetSetRect(panel->webview,
        (xui_rect_t){0, 0, bounds.fW, bounds.fH - 29 > 1 ? bounds.fH - 29 : 1});
    if (result == XUI_OK)
        result = xuiWindowAddChild(panel->window, panel->webview);
    if (result == XUI_OK)
        result = xuiWebViewInitializeAsync(panel->webview);
    if (result != XUI_OK) goto fail;
    *out = panel;
    return XUI_OK;
fail:
    xuiDocumentHtmlInteractionRelease(panel);
    return result;
}

XUI_API int xuiDocumentHtmlInteractionUpdate(xui_doc_html_interaction panel)
{
    char* html;
    size_t bytes;
    uint64_t revision;
    int result;
    if (!panel || !xuiInternalWidgetIsValid(panel->window) ||
        !xuiInternalWidgetIsValid(panel->webview))
        return XUI_ERROR_INVALID_STATE;
    if (panel->backend_error != XUI_OK)
        return panel->backend_error;
    if (xuiWebViewGetState(panel->webview) == XUI_WEBVIEW_FAILED)
        return XUI_ERROR_BACKEND_FAILED;
    revision = xuiDocumentGetRevision(panel->document);
    if (revision == panel->seen_revision)
        return doc_html_interaction_advance(panel);
    result = doc_html_interaction_read(panel, &html, &bytes, &revision);
    if (result == XUI_ERROR_NOT_FOUND || result == XUI_ERROR_INVALID_ARGUMENT) {
        (void)xuiWindowSetOpen(panel->window, 0);
        return result;
    }
    if (result != XUI_OK) return result;
    panel->seen_revision = revision;
    if (bytes == panel->html_bytes &&
        !memcmp(html, panel->html, bytes)) {
        free(html);
        return doc_html_interaction_advance(panel);
    }
    free(panel->html);
    panel->html = html;
    panel->html_bytes = bytes;
    panel->dirty = 1;
    return doc_html_interaction_advance(panel);
}

XUI_API int xuiDocumentHtmlInteractionShow(xui_doc_html_interaction panel)
{
    int result = xuiDocumentHtmlInteractionUpdate(panel);
    if (result != XUI_OK) return result;
    result = xuiWindowSetOpen(panel->window, 1);
    if (result != XUI_OK) return result;
    panel->focus_pending = 1;
    return doc_html_interaction_advance(panel);
}

XUI_API xui_widget xuiDocumentHtmlInteractionGetWindow(xui_doc_html_interaction panel)
{
    return panel && xuiInternalWidgetIsValid(panel->window) ?
        panel->window : NULL;
}

XUI_API xui_widget xuiDocumentHtmlInteractionGetWebView(xui_doc_html_interaction panel)
{
    return panel && xuiInternalWidgetIsValid(panel->webview) ?
        panel->webview : NULL;
}

XUI_API void xuiDocumentHtmlInteractionRelease(xui_doc_html_interaction panel)
{
    if (!panel) return;
    if (panel->webview && xuiInternalWidgetIsValid(panel->webview))
        (void)xuiWebViewClose(panel->webview);
    if (panel->window && xuiInternalWidgetIsValid(panel->window))
        xuiWidgetDestroy(panel->window);
    else if (panel->webview && xuiInternalWidgetIsValid(panel->webview))
        xuiWidgetDestroy(panel->webview);
    xuiDocumentRelease(panel->document);
    free(panel->html);
    free(panel);
}

#endif
