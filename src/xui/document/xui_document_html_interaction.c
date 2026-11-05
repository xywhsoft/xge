#include "xui_config.h"
#if XUI_ENABLE_WEBVIEW
#include "xui_document_ui.h"
#include "xui_internal.h"
#include "xui_webview_internal.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct xui_doc_html_interaction_t {
    xui_owned_manager_t owned;
    xui_context context;
    xui_callback_owner_t open_link_owner;
    xui_lifetime_link_t window_watch,webview_watch;
    unsigned working;
    xui_doc_html_open_link_view_proc on_open_link_view;
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

static const xui_owned_manager_ops_t doc_html_ops;
static bool doc_html_alive(xui_doc_html_interaction panel)
{
    if(panel->owned.creatorThread!=xrtThreadCurrentId() || !xuiInternalManagerAlive(&panel->owned))return false;
    xui_context context=panel->context;
    return !context->bCloseRequested && !context->bDestroyPending && !context->bDestroying;
}
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
    uint32_t next_generation=panel->page_generation+1;
    if(!next_generation)++next_generation;
    generation_bytes = snprintf(generation, sizeof(generation), "%u",
        (unsigned)next_generation);
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
    panel->page_generation=next_generation;
    result = xuiWebViewLoadHtml(panel->webview, page, length);
    free(page);
    if (result == XUI_OK && doc_html_alive(panel)) panel->dirty = 0;
    return result;
}

static int doc_html_interaction_advance(xui_doc_html_interaction panel)
{
    int result = doc_html_interaction_load(panel);
    if(!doc_html_alive(panel))return XUI_ERROR_INVALID_STATE;
    if (!xuiWindowIsOpen(panel->window)) panel->focus_pending = 0;
    if (result != XUI_OK || !panel->focus_pending ||
        xuiWebViewGetState(panel->webview) != XUI_WEBVIEW_READY)
        return result;
    result = xuiWebViewFocus(panel->webview);
    if (result == XUI_OK) panel->focus_pending = 0;
    return result == XUI_ERROR_INVALID_STATE ? XUI_OK : result;
}

static void doc_html_interaction_message(xui_widget webview,
    const char* source,const char* json,size_t bytes,void* raw)
{
    xui_doc_html_interaction panel=raw;xui_manager_scope_t scope={0};
    if(!panel || xuiInternalManagerScopeBegin(&panel->owned,&scope)!=XUI_OK)return;
    int status=XUI_OK;xvalue* root=NULL;char* copy=NULL;
    xui_callback_snapshot_t snapshot={0};
    xui_doc_html_open_link_proc callback=panel->on_open_link;
    xui_doc_html_open_link_view_proc view_callback=panel->on_open_link_view;
    void* user=panel->user;
    (void)webview;
    if((!callback && !view_callback) || !source || strcmp(source,"about:blank") ||
       !json || bytes>8192)goto done;
    root=xrtJsonParse((xstrview){json,bytes});
    if(!root){status=XUI_ERROR_INVALID_ARGUMENT;goto done;}
    xvalue* kind=xrtValueObjectGet(root,(xstrview){"kind",4});
    xvalue* url=xrtValueObjectGet(root,(xstrview){"url",3});
    xvalue* generation=xrtValueObjectGet(root,(xstrview){"generation",10});
    xstrview kind_text={0},url_text={0};uint64_t message_generation=0;int64_t signed_generation=0;
    if(!xrtValueGetString(kind,&kind_text) || kind_text.Size!=12 ||
       memcmp(kind_text.Data,"externalLink",12) || !xrtValueGetString(url,&url_text) ||
       url_text.Size>4096)goto done;
    if(xrtValueType(generation)==XVALUE_UINT){
        if(!xrtValueGetUInt(generation,&message_generation))goto done;
    }else{
        if(xrtValueType(generation)!=XVALUE_INT || !xrtValueGetInt(generation,&signed_generation) || signed_generation<0)goto done;
        message_generation=(uint64_t)signed_generation;
    }
    if(message_generation!=panel->page_generation)goto done;
    size_t scheme=url_text.Size>=8 && !memcmp(url_text.Data,"https://",8) ? 8:
        url_text.Size>=7 && !memcmp(url_text.Data,"http://",7) ? 7:0;
    if(!scheme || url_text.Size<=scheme)goto done;
    for(size_t i=0;i<url_text.Size;++i)
        if((unsigned char)url_text.Data[i]<32 || (unsigned char)url_text.Data[i]==127)goto done;
    if(callback){
        copy=malloc(url_text.Size+1);if(!copy){status=XUI_ERROR_OUT_OF_MEMORY;goto done;}
        memcpy(copy,url_text.Data,url_text.Size);copy[url_text.Size]=0;
    }
    status=xuiInternalManagerCallbackSnapshotBegin(&panel->owned,&panel->open_link_owner,&snapshot,true);
    if(status!=XUI_OK)goto done;
    if(view_callback)view_callback(&url_text,user);else callback(copy,user);
    /* The actual JSON owner and raw C copy both survive the final owner Drop:
     * its custom destructor is permitted to inspect callback-period inputs. */
    status=xuiInternalCallbackSnapshotEnd(&snapshot,XUI_OK);
done:
    free(copy);xrtValueRelease(root);
    if(status<0)xuiInternalContextRecordFailure(panel->context,status);
    (void)xuiInternalManagerScopeEnd(&scope,status);
}

static void doc_html_interaction_event(xui_widget webview,int event,
    int32_t native_error,void* raw)
{
    xui_doc_html_interaction panel=raw;xui_manager_scope_t scope={0};(void)native_error;
    if(!panel || xuiInternalManagerScopeBegin(&panel->owned,&scope)!=XUI_OK)return;
    int status=XUI_OK;
    if(event==XUI_WEBVIEW_EVENT_READY){
        status=xuiWebViewBlockExternalResources(webview);
        if(status==XUI_OK && doc_html_alive(panel))status=xuiWebViewSetMessageHandler(webview,
            "about:blank",doc_html_interaction_message,panel);
        if(status==XUI_OK && doc_html_alive(panel))status=doc_html_interaction_load(panel);
        panel->backend_error=status;
    }
    if(status<0)xuiInternalContextRecordFailure(panel->context,status);
    (void)xuiInternalManagerScopeEnd(&scope,status);
}
static void doc_html_window_gone(void* raw)
{
    xui_doc_html_interaction panel=raw;panel->window=NULL;
    if(xuiInternalManagerAlive(&panel->owned))(void)xuiInternalManagerRequestClose(&panel->owned);
}
static void doc_html_webview_gone(void* raw)
{
    xui_doc_html_interaction panel=raw;panel->webview=NULL;
    if(xuiInternalManagerAlive(&panel->owned))(void)xuiInternalManagerRequestClose(&panel->owned);
}
static bool doc_html_trace(const void* raw,xrtownershipvisitor visit,void* user)
{
    const struct xui_doc_html_interaction_t* panel=raw;
    if(panel->working)return false;
    return !panel->open_link_owner.tReference.Data || visit(panel->open_link_owner.tReference,user);
}
static int doc_html_ready(const void* raw)
{return ((const struct xui_doc_html_interaction_t*)raw)->working ? XUI_ERROR_BUSY:XUI_OK;}
static int doc_html_prepare(void* raw,bool* complete)
{*complete=!((xui_doc_html_interaction)raw)->working;return *complete ? XUI_OK:XUI_STATUS_DEFERRED;}
static void doc_html_close(void* raw)
{
    xui_doc_html_interaction panel=raw;
    xuiLifetimeUnwatch(&panel->window_watch);xuiLifetimeUnwatch(&panel->webview_watch);
    if(panel->window){
        xui_widget window=panel->window;panel->window=NULL;panel->webview=NULL;
        int status=xuiWidgetDestroyChecked(window);
        if(status<0)xuiInternalContextRecordFailure(panel->context,status);
    }else if(panel->webview){
        xui_widget webview=panel->webview;panel->webview=NULL;
        int status=xuiInternalWidgetDestroyPart((xui_widget)webview->pInternalOwner,webview);
        if(status<0)xuiInternalContextRecordFailure(panel->context,status);
    }
}
static int doc_html_finalize(void* raw)
{
    xui_doc_html_interaction panel=raw;xui_context context=panel->context;
    xui_cleanup_receipt_t receipt={0};xuiInternalCleanupStart(&receipt,XUI_OK);
    xuiInternalCleanupAdd(&receipt,xuiInternalCallbackOwnerDropOutcome(context,&panel->open_link_owner,receipt.status,receipt.error));
    xuiDocumentRelease(panel->document);xuiInternalCleanupAdd(&receipt,XUI_OK);
    free(panel->html);free(panel);
    return xuiInternalCleanupEnd(context,&receipt);
}
static const xui_owned_manager_ops_t doc_html_ops={doc_html_trace,doc_html_ready,doc_html_prepare,doc_html_close,doc_html_finalize};
static void doc_html_construct_gone(void* raw){*(bool*)raw=false;}
static int doc_html_construct_end(xui_call_scope_t* scope,int status,xui_doc_html_interaction panel,
    xui_lifetime_link_t* watch,bool* alive,xui_doc_html_interaction* out)
{
    xerror* first=xrtTakeError();if(status==XUI_OK && first)status=XUI_ERROR_CALLBACK_FAILED;
    if(status==XUI_OK && (!*alive || !doc_html_alive(panel)))status=XUI_ERROR_INVALID_STATE;
    if(status!=XUI_OK && *alive){
        xui_cleanup_prefix_t prefix={0};xuiInternalCleanupPrefixBegin(panel->context,&prefix,status,first);
        (void)xuiInternalManagerRequestClose(&panel->owned);xrtClearError();xuiInternalCleanupPrefixEnd(&prefix);
    }
    status=xuiInternalCallScopeEndOutcome(scope,status,&first);xuiLifetimeUnwatch(watch);
    if(status==XUI_OK && !*alive)status=XUI_ERROR_INVALID_STATE;
    if(status==XUI_OK)*out=panel;
    xrtSetErrorTake(first);return status;
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
    if(out)*out=NULL;
    if (!out || !desc || desc->iSize < sizeof(*desc) ||
        !desc->pContext || !desc->pDocument || !desc->iNodeId)
        return XUI_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    if (!xuiGetRootWidget(desc->pContext))
        return XUI_ERROR_INVALID_ARGUMENT;
    viewport = xuiGetViewportSize(desc->pContext);
    if (viewport.iW <= 0 || viewport.iH <= 0)
        return XUI_ERROR_INVALID_STATE;
    xui_call_scope_t scope={0};result=xuiCallScopeBegin(desc->pContext,&scope);if(result!=XUI_OK)return result;
    panel=calloc(1,sizeof(*panel));
    if(!panel){xerror* first=xrtTakeError();result=xuiInternalCallScopeEndOutcome(&scope,XUI_ERROR_OUT_OF_MEMORY,&first);xrtSetErrorTake(first);return result;}
    panel->context=desc->pContext;
    result=xuiInternalManagerRegister(panel->context,&panel->owned,&doc_html_ops,panel);
    if(result!=XUI_OK){free(panel);xerror* first=xrtTakeError();result=xuiInternalCallScopeEndOutcome(&scope,result,&first);xrtSetErrorTake(first);return result;}
    bool alive=true;xui_lifetime_link_t watch={0};
    if(xuiInternalManagerWatch(&panel->owned,&watch,doc_html_construct_gone,&alive)!=XUI_OK)abort();
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
    result=xuiWidgetLifetimeWatch(panel->window,&panel->window_watch,doc_html_window_gone,panel);
    if(result!=XUI_OK)goto fail;
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
    xui_widget_type webview_type=xuiWebViewGetType(desc->pContext);
    result=webview_type ? xuiInternalWidgetCreatePart(panel->window,webview_type,&panel->webview,&webview_desc):XUI_ERROR_OUT_OF_MEMORY;
    if (result != XUI_OK) goto fail;
    result=xuiWidgetLifetimeWatch(panel->webview,&panel->webview_watch,doc_html_webview_gone,panel);
    if(result!=XUI_OK)goto fail;
    result = xuiWidgetSetRect(panel->webview,
        (xui_rect_t){0, 0, bounds.fW, bounds.fH - 29 > 1 ? bounds.fH - 29 : 1});
    if (result == XUI_OK)
        result=xuiInternalWidgetInsertPart(panel->window,client,panel->webview,NULL);
    if (result == XUI_OK)
        result = xuiWebViewInitializeAsync(panel->webview);
    if (result != XUI_OK) goto fail;
    return doc_html_construct_end(&scope,XUI_OK,panel,&watch,&alive,out);
fail:
    return doc_html_construct_end(&scope,result,panel,&watch,&alive,out);
}

static int doc_html_update(xui_doc_html_interaction panel)
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

static int doc_html_show(xui_doc_html_interaction panel)
{
    int result = doc_html_update(panel);
    if (result != XUI_OK) return result;
    result = xuiWindowSetOpen(panel->window, 1);
    if (result != XUI_OK) return result;
    panel->focus_pending = 1;
    return doc_html_interaction_advance(panel);
}

static int doc_html_begin(xui_doc_html_interaction panel,xui_manager_scope_t* scope)
{
    if(!panel)return XUI_ERROR_INVALID_ARGUMENT;
    int status=xuiInternalManagerScopeBegin(&panel->owned,scope);if(status!=XUI_OK)return status;
    if(panel->working)return xuiInternalManagerScopeEnd(scope,XUI_ERROR_BUSY);
    panel->working=1;return XUI_OK;
}
static int doc_html_end(xui_doc_html_interaction panel,xui_manager_scope_t* scope,int status)
{
    if(status==XUI_OK && !doc_html_alive(panel))status=XUI_ERROR_INVALID_STATE;
    panel->working=0;return xuiInternalManagerScopeEnd(scope,status);
}
XUI_API int xuiDocumentHtmlInteractionUpdate(xui_doc_html_interaction panel)
{xui_manager_scope_t scope={0};int status=doc_html_begin(panel,&scope);if(status!=XUI_OK)return status;return doc_html_end(panel,&scope,doc_html_update(panel));}
XUI_API int xuiDocumentHtmlInteractionShow(xui_doc_html_interaction panel)
{xui_manager_scope_t scope={0};int status=doc_html_begin(panel,&scope);if(status!=XUI_OK)return status;return doc_html_end(panel,&scope,doc_html_show(panel));}
XUI_API xui_widget xuiDocumentHtmlInteractionGetWindow(xui_doc_html_interaction panel)
{return panel && doc_html_alive(panel) && xuiInternalWidgetIsValid(panel->window) ? panel->window:NULL;}
XUI_API xui_widget xuiDocumentHtmlInteractionGetWebView(xui_doc_html_interaction panel)
{return panel && doc_html_alive(panel) && xuiInternalWidgetIsValid(panel->webview) ? panel->webview:NULL;}
XUI_API xui_context xuiDocumentHtmlInteractionGetContext(xui_doc_html_interaction panel)
{return panel && doc_html_alive(panel) ? panel->context:NULL;}
XUI_API int xuiDocumentHtmlInteractionLifetimeWatch(xui_doc_html_interaction panel,xui_lifetime_link_t* link,xui_lifetime_invalidate_proc invalidate,void* user)
{return panel ? xuiInternalManagerWatch(&panel->owned,link,invalidate,user):XUI_ERROR_INVALID_ARGUMENT;}
XUI_API int xuiDocumentHtmlInteractionReleaseOwnerChecked(xui_doc_html_interaction panel,bool* consumed)
{
    if(!consumed)return XUI_ERROR_INVALID_ARGUMENT;
    *consumed=false;if(!panel)return XUI_ERROR_INVALID_ARGUMENT;
    if(panel->owned.pending)return XUI_OK;
    xui_manager_scope_t scope={0};int status=xuiInternalManagerScopeBegin(&panel->owned,&scope);if(status!=XUI_OK)return status;
    status=xuiInternalManagerRequestClose(&panel->owned);*consumed=status==XUI_OK;
    return xuiInternalManagerScopeEnd(&scope,status);
}
XUI_API int xuiDocumentHtmlInteractionReleaseChecked(xui_doc_html_interaction panel)
{bool consumed=false;return xuiDocumentHtmlInteractionReleaseOwnerChecked(panel,&consumed);}
XUI_API void xuiDocumentHtmlInteractionRelease(xui_doc_html_interaction panel)
{if(panel)(void)xuiDocumentHtmlInteractionReleaseChecked(panel);}
XUI_API int xuiDocumentHtmlInteractionSetOpenLinkOwned(xui_doc_html_interaction panel,
    xui_doc_html_open_link_view_proc callback,void* user,const xui_callback_owner_t* owner)
{
    if(!panel)return XUI_ERROR_INVALID_ARGUMENT;
    xui_manager_callback_update_t update={0};int status=xuiInternalManagerCallbackUpdateBegin(&panel->owned,&panel->open_link_owner,callback!=NULL,owner,&update);
    if(status!=XUI_OK)return status;
    panel->on_open_link=NULL;panel->on_open_link_view=callback;panel->user=user;
    xuiInternalManagerCallbackUpdatePublish(&update);return xuiInternalManagerCallbackUpdateEnd(&update,XUI_OK);
}
XUI_API int xuiDocumentHtmlInteractionSetOpenLink(xui_doc_html_interaction panel,xui_doc_html_open_link_proc callback,void* user)
{
    if(!panel)return XUI_ERROR_INVALID_ARGUMENT;
    xui_manager_callback_update_t update={0};int status=xuiInternalManagerCallbackUpdateBegin(&panel->owned,&panel->open_link_owner,callback!=NULL,NULL,&update);
    if(status!=XUI_OK)return status;
    panel->on_open_link=callback;panel->on_open_link_view=NULL;panel->user=user;
    xuiInternalManagerCallbackUpdatePublish(&update);return xuiInternalManagerCallbackUpdateEnd(&update,XUI_OK);
}
#endif
